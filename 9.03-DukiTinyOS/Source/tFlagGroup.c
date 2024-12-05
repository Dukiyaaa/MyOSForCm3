#include "tinyOS.h"

void tFlagGroupInit(tFlagGroup * flagGroup, uint32_t flags) {
	tEventInit(&flagGroup->event, tEventTypeFlagGroup);
	flagGroup->flags = flags;
}

static uint32_t tFlagGroupCheckAndConsume (tFlagGroup *flagGroup, uint32_t type, uint32_t *flags) {
	uint32_t srcFlags = *flags;                   // 目标标志位,传进来的是位置，即接下来的操作是对于flags为1的位上的
    uint32_t isSet = type & TFLAGGROUP_SET;       // 检查类型中是否含有设置操作标志 (TFLAGGROUP_SET)
    uint32_t isAll = type & TFLAGGROUP_ALL;       // 检查类型中是否要求所有标志匹配 (TFLAGGROUP_ALL)
    uint32_t isConsume = type & TFLAGGROUP_CONSUME; // 检查是否需要消费标志 (TFLAGGROUP_CONSUME)
    
	// 根据是否设置标志位选择计算哪些标志位匹配 置位(保持原来的)or清零
    uint32_t calFlags = isSet ? (flagGroup->flags & srcFlags) : (~flagGroup->flags & srcFlags);
	//1.对应位保持原来的位不变 2.对应位跟原本的取反
	// 检查标志位是否符合要求 isAll为1说明要求全部都一样 isAll为0说明只要有一位满足就行
    if (((isAll != 0) && (calFlags == srcFlags)) || ((isAll == 0) && (calFlags != 0))) {
        if (isConsume) {
            // 如果需要消费标志位
            if (isSet) {
                // 如果是置位操作，清除匹配的标志位
                flagGroup->flags &= ~srcFlags;
            } else {
                // 如果是清0操作，置位匹配的标志位
                flagGroup->flags |= srcFlags;
            }
        }
        
        // 返回计算后的标志位，并标志已成功处理
        *flags = calFlags;
        return tErrorNoError;
    }
    
    // 如果标志位不符合要求，返回资源不可用错误
    *flags = calFlags;
    return tErrorResourceUnavaliable;
}

uint32_t tFlagGroupWait(tFlagGroup * flagGroup, uint32_t waitType, uint32_t requestFlag, uint32_t *resultFlag, uint32_t waitTicks) {
    uint32_t result;   // 用于保存返回的结果
    uint32_t flags = requestFlag;  // 请求的标志位，初始化为请求的标志
    uint32_t status = tTaskEnterCritical();  // 进入临界区，禁止任务调度，以保证线程安全
    
    // 检查和消耗标志位
    result = tFlagGroupCheckAndConsume(flagGroup, waitType, &flags);
    
    // 如果检查和消耗失败
    if(result != tErrorNoError) {
        // 当前任务等待标志类型和事件标志
        curTask->waitFlagsType = waitType;
        curTask->eventFlags = requestFlag;
        
        // 当前任务进入等待状态，等待旗标组的事件，最多等待waitTicks个时钟周期
        tEventWait(&flagGroup->event, curTask, (void *)0, tEventTypeFlagGroup, waitTicks);
        
        // 退出临界区，允许任务调度
        tTaskExitCritical(status);
        
        // 调度任务，可能会切换任务
        tTaskSched();
        
			// 将当前任务的事件标志赋值给resultFlag  //无论如何resultFlag内都会是想要的flags，但是result不同
        *resultFlag = curTask->eventFlags;
        
        // 返回当前任务等待事件的结果
        result = curTask->waitEventResult;
    } else {
        // 如果标志检查和消耗成功，将结果标志返回给resultFlag
        *resultFlag = flags;
        
        // 退出临界区
        tTaskExitCritical(status);
    }
    
    return result;  // 返回操作结果
}


uint32_t tFlagGroupNoWaitGet(tFlagGroup * flagGroup, uint32_t waitType, uint32_t requestFlag, uint32_t *resultFlag, uint32_t waitTicks) {
	uint32_t flags = requestFlag;
	
	uint32_t status = tTaskEnterCritical();
	
	uint32_t result = tFlagGroupCheckAndConsume(flagGroup, waitType, &flags);
	
	tTaskExitCritical(status);
	
	*resultFlag = flags;
	
	return tErrorNoError;
}

void tFlagGroupNotify(tFlagGroup * flagGroup, uint8_t isSet, uint32_t flag) {
    tList * waitList;   // 等待列表，用于存储所有等待该标志的任务
    tNode * node;       // 用于遍历等待列表的节点
    uint32_t sched = 0; // 任务调度标志，标志是否需要调度任务
    uint32_t status = tTaskEnterCritical();  // 进入临界区，禁止任务调度，以保证线程安全
    
    // 根据isSet的值，设置或清除标志
    if(isSet) {
        // 设置标志
        flagGroup->flags |= flag;
    } else {
        // 清除标志
        flagGroup->flags &= ~flag;
    }
    
    // 获取等待列表，该列表存储了所有等待该标志的任务
    waitList = &flagGroup->event.waitList;

    // 遍历等待列表中的所有任务
    for(node = waitList->headNode.nextNode; node != &(waitList->headNode); node = node->nextNode) {
        uint32_t result;  
        tTask * task = tNodeParent(node, tTask, linkNode);  // 获取当前任务对象
        uint32_t flags = task->eventFlags;  // 当前任务的事件标志
        
        // 检查并消耗标志
        result = tFlagGroupCheckAndConsume(flagGroup, task->waitFlagsType, &flags);
        
        // 如果标志检查和消耗成功
        if(result == tErrorNoError) {
            // 更新任务的事件标志
            task->eventFlags = flags;
            
            // 唤醒任务，并标记任务为就绪状态
            tEventWakeUpTask(&flagGroup->event, task, (void *)0, tErrorNoError);
            
            // 标记需要调度任务
            sched = 1;
        }
    }

    // 如果有任务被唤醒，进行任务调度
    if(sched) {
        // 退出临界区，允许任务调度
        tTaskExitCritical(status);
        
        // 调度任务
        tTaskSched();
    } else {
        // 无任务需要调度，直接退出临界区
        tTaskExitCritical(status);
    }
}

uint32_t tFlagGroupDestory(tFlagGroup * flagGroup) {
    uint32_t status = tTaskEnterCritical();  // 进入临界区，保护共享资源

    uint32_t count = tEventRemoveAll(&flagGroup->event, (void *)0, tErrorDel);  // 删除所有等待任务
    tTaskExitCritical(status);  // 退出临界区

    if (count > 0) {  // 如果有任务被删除
        tTaskSched();  // 调度任务
    }

    return count;  // 返回删除的任务数量
}

void tFlagGroupGetInfo (tFlagGroup * flagGroup, tFlagGroupInfo * info) {
	    uint32_t status = tTaskEnterCritical();  // 进入临界区，保护共享资源

    info->taskCount = tEventWaitCount(&flagGroup->event);  // 获取等待任务数
    info->flags = flagGroup->flags;  // 标志位
    
    tTaskExitCritical(status);  // 退出临界区
}

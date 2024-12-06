#include "tinyOS.h"

#if TINYOS_ENABLE_FLAGGROUP == 1
/**
 * @brief 初始化标志组。
 * 
 * 该函数用于初始化标志组，并设置初始的标志值。
 * 
 * @param flagGroup  指向标志组的指针。
 * @param flags      初始标志位，定义了标志组的初始状态。
 */
void tFlagGroupInit(tFlagGroup *flagGroup, uint32_t flags) {
    tEventInit(&flagGroup->event, tEventTypeFlagGroup);  // 初始化事件对象，指定为标志组类型
    flagGroup->flags = flags;  // 设置初始的标志值
}

/**
 * @brief 检查并根据条件消费标志位。
 * 
 * 该函数检查标志组中的标志是否符合请求条件，如果符合，并且需要消费标志，则会更新标志组的状态。
 * 
 * @param flagGroup   指向标志组的指针。
 * @param type        检查类型，包含是否设置标志、是否全部匹配以及是否需要消费标志。
 * @param flags       请求标志位，在检查后返回实际的标志位。
 * 
 * @return uint32_t   返回操作结果，`tErrorNoError` 表示成功，`tErrorResourceUnavaliable` 表示资源不可用。
 */
static uint32_t tFlagGroupCheckAndConsume(tFlagGroup *flagGroup, uint32_t type, uint32_t *flags) {
    uint32_t srcFlags = *flags;  // 获取传入的标志位
    uint32_t isSet = type & TFLAGGROUP_SET;       // 检查类型是否包含设置标志操作
    uint32_t isAll = type & TFLAGGROUP_ALL;       // 检查是否要求所有标志匹配
    uint32_t isConsume = type & TFLAGGROUP_CONSUME; // 检查是否需要消费标志

    // 根据是否设置标志位选择计算哪些标志位匹配（保持原值或取反）
    uint32_t calFlags = isSet ? (flagGroup->flags & srcFlags) : (~flagGroup->flags & srcFlags);
    
    // 检查标志位是否符合要求
    if (((isAll != 0) && (calFlags == srcFlags)) || ((isAll == 0) && (calFlags != 0))) {
        if (isConsume) {
            // 如果需要消费标志位
            if (isSet) {
                // 设置操作：清除匹配的标志位
                flagGroup->flags &= ~srcFlags;
            } else {
                // 清除操作：置位匹配的标志位
                flagGroup->flags |= srcFlags;
            }
        }
        
        // 返回处理后的标志，并标记操作成功
        *flags = calFlags;
        return tErrorNoError;
    }

    // 如果标志位不符合要求，返回资源不可用
    *flags = calFlags;
    return tErrorResourceUnavaliable;
}

/**
 * @brief 等待标志组标志并阻塞当前任务。
 * 
 * 该函数会检查标志组是否满足指定条件，如果条件不满足，则将当前任务置为等待状态，直到条件满足或超时。
 * 
 * @param flagGroup   指向标志组的指针。
 * @param waitType    等待类型，定义了如何匹配标志。
 * @param requestFlag 请求的标志位。
 * @param resultFlag  用于返回操作结果的标志位。
 * @param waitTicks   等待时间（以滴答为单位），为0表示不等待。
 * 
 * @return uint32_t   返回操作结果，`tErrorNoError` 表示成功，`tErrorResourceUnavaliable` 表示资源不可用。
 */
uint32_t tFlagGroupWait(tFlagGroup *flagGroup, uint32_t waitType, uint32_t requestFlag, uint32_t *resultFlag, uint32_t waitTicks) {
    uint32_t result;   // 用于保存返回的结果
    uint32_t flags = requestFlag;  // 请求的标志位，初始化为请求的标志
    uint32_t status = tTaskEnterCritical();  // 进入临界区，禁止任务调度，以保证线程安全
    
    // 检查并消耗标志位
    result = tFlagGroupCheckAndConsume(flagGroup, waitType, &flags);
    
    // 如果检查和消耗失败
    if (result != tErrorNoError) {
        // 当前任务等待标志类型和事件标志
        curTask->waitFlagsType = waitType;
        curTask->eventFlags = requestFlag;
        
        // 当前任务进入等待状态，等待旗标组的事件，最多等待waitTicks个时钟周期
        tEventWait(&flagGroup->event, curTask, (void *)0, tEventTypeFlagGroup, waitTicks);
        
        // 退出临界区，允许任务调度
        tTaskExitCritical(status);
        
        // 调度任务，可能会切换任务
        tTaskSched();
        
        // 将当前任务的事件标志赋值给resultFlag
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

/**
 * @brief 不阻塞式获取标志组标志。
 * 
 * 该函数会检查标志组是否满足指定条件，如果满足则立即返回结果，否则不进行任何操作。
 * 
 * @param flagGroup   指向标志组的指针。
 * @param waitType    等待类型，定义了如何匹配标志。
 * @param requestFlag 请求的标志位。
 * @param resultFlag  用于返回操作结果的标志位。
 * @param waitTicks   等待时间（以滴答为单位），为0表示不等待。
 * 
 * @return uint32_t   返回操作结果，`tErrorNoError` 表示成功，`tErrorResourceUnavaliable` 表示资源不可用。
 */
uint32_t tFlagGroupNoWaitGet(tFlagGroup *flagGroup, uint32_t waitType, uint32_t requestFlag, uint32_t *resultFlag, uint32_t waitTicks) {
    uint32_t flags = requestFlag;  // 请求的标志位，初始化为请求的标志
    
    uint32_t status = tTaskEnterCritical();  // 进入临界区，禁止任务调度，以保证线程安全
    
    uint32_t result = tFlagGroupCheckAndConsume(flagGroup, waitType, &flags);
    
    tTaskExitCritical(status);  // 退出临界区
    
    *resultFlag = flags;  // 将操作结果赋值给resultFlag
    
    return tErrorNoError;  // 返回成功
}

/**
 * @brief 通知标志组标志状态变化。
 * 
 * 该函数会通知标志组某一标志被设置或清除，并唤醒所有等待该标志的任务。
 * 
 * @param flagGroup   指向标志组的指针。
 * @param isSet       标志操作类型，`1`表示设置标志，`0`表示清除标志。
 * @param flag        要设置或清除的标志位。
 */
void tFlagGroupNotify(tFlagGroup *flagGroup, uint8_t isSet, uint32_t flag) {
    tList *waitList;   // 等待列表，用于存储所有等待该标志的任务
    tNode *node;       // 用于遍历等待列表的节点
    uint32_t sched = 0; // 任务调度标志，标志是否需要调度任务
    uint32_t status = tTaskEnterCritical();  // 进入临界区，禁止任务调度，以保证线程安全
    
    // 根据isSet的值，设置或清除标志
    if (isSet) {
        // 设置标志
        flagGroup->flags |= flag;
    } else {
        // 清除标志
        flagGroup->flags &= ~flag;
    }
    
    // 获取等待列表，该列表存储了所有等待该标志的任务
    waitList = &flagGroup->event.waitList;

    // 遍历等待列表中的所有任务
    for (node = waitList->headNode.nextNode; node != &(waitList->headNode); node = node->nextNode) {
        uint32_t result;  
        tTask *task = tNodeParent(node, tTask, linkNode);  // 获取当前任务对象
        uint32_t flags = task->eventFlags;  // 当前任务的事件标志
        
        // 检查并消耗标志
        result = tFlagGroupCheckAndConsume(flagGroup, task->waitFlagsType, &flags);
        
        // 如果标志检查和消耗成功
        if (result == tErrorNoError) {
            // 更新任务的事件标志
            task->eventFlags = flags;
            
            // 唤醒任务，并标记任务为就绪状态
            tEventWakeUpTask(&flagGroup->event, task, (void *)0, tErrorNoError);
            
            // 标记需要调度任务
            sched = 1;
        }
    }

    // 如果有任务被唤醒，进行任务调度
    if (sched) {
        // 退出临界区，允许任务调度
        tTaskExitCritical(status);
        
        // 调度任务
        tTaskSched();
    } else {
        // 无任务需要调度，直接退出临界区
        tTaskExitCritical(status);
    }
}

/**
 * @brief 销毁标志组，删除所有等待任务。
 * 
 * 该函数会销毁标志组并删除所有在该标志组上等待的任务。
 * 
 * @param flagGroup   指向标志组的指针。
 * 
 * @return uint32_t   返回删除的任务数量。
 */
uint32_t tFlagGroupDestory(tFlagGroup *flagGroup) {
    uint32_t status = tTaskEnterCritical();  // 进入临界区，保护共享资源

    // 删除标志组上的所有等待任务，并返回删除的任务数量
    uint32_t count = tEventRemoveAll(&flagGroup->event, (void *)0, tErrorDel);  // 删除所有等待任务
    tTaskExitCritical(status);  // 退出临界区

    if (count > 0) {  // 如果有任务被删除
        // 如果有任务被删除，需要调度任务
        tTaskSched();  // 调度任务
    }

    return count;  // 返回删除的任务数量
}

/**
 * @brief 获取标志组的相关信息。
 * 
 * 该函数会获取标志组的当前信息，包括等待任务数量和标志位状态。
 * 
 * @param flagGroup   指向标志组的指针。
 * @param info        用于保存标志组信息的结构体指针。
 */
void tFlagGroupGetInfo(tFlagGroup *flagGroup, tFlagGroupInfo *info) {
    uint32_t status = tTaskEnterCritical();  // 进入临界区，保护共享资源

    // 获取等待任务的数量
    info->taskCount = tEventWaitCount(&flagGroup->event);  // 获取等待任务数
    // 获取当前标志组的标志位状态
    info->flags = flagGroup->flags;  // 获取标志位
    
    tTaskExitCritical(status);  // 退出临界区
}
#endif
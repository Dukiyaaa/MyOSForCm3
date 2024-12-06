#include "tinyOS.h"

/**
 * @brief 定义任务初始化函数
 * 
 * 该函数初始化任务的堆栈、优先级、状态等信息，并将任务准备就绪，以便加入任务调度队列。
 * 
 * @param tTask 任务控制块指针，用于保存任务信息。
 * @param entry 任务的入口函数。
 * @param param 任务的入口函数参数。
 * @param prio 任务的优先级。
 * @param tTaskStack 任务堆栈指针，用于保存任务的上下文信息。
 * 
 * @return void
 */
void tTaskInit(tTask *task, void(*entry)(void *), void * param, uint32_t prio, tTaskStack * stack, tTaskStack stackSize) {
	  //现在传入的是堆栈底
	  tTaskStack * stackTop;  //栈顶
	  task->stackBase = stack;
	  task->stackSize = stackSize;
	  memset(stack,0,stackSize);
	
	  //栈顶指针等于栈底+栈单元个数
	  stackTop = stack + stackSize / (sizeof(tTaskStack));
	
    // 设置xpsr寄存器，第24位置1进入Thumb模式
    *(--stackTop) = (unsigned long)(1 << 24); 

    // 设置PC寄存器（程序计数器），即任务入口地址
    *(--stackTop) = (unsigned long)entry;     

    // 设置LR寄存器（链接寄存器），即任务返回地址
    *(--stackTop) = (unsigned long)0xFFFFFFFD;      

    // 设置其他寄存器的初始值
    *(--stackTop) = (unsigned long)0x12;      // R12寄存器
    *(--stackTop) = (unsigned long)0x3;       // R3寄存器
    *(--stackTop) = (unsigned long)0x2;       // R2寄存器
    *(--stackTop) = (unsigned long)0x1;       // R1寄存器
    *(--stackTop) = (unsigned long)param;     // 任务的入口函数参数

    // 设置其他寄存器的初始值
    *(--stackTop) = (unsigned long)0x11;      // R11寄存器
    *(--stackTop) = (unsigned long)0x10;      // R10寄存器
    *(--stackTop) = (unsigned long)0x9;       // R9寄存器
    *(--stackTop) = (unsigned long)0x8;       // R8寄存器
    *(--stackTop) = (unsigned long)0x7;       // R7寄存器
    *(--stackTop) = (unsigned long)0x6;       // R6寄存器
    *(--stackTop) = (unsigned long)0x5;       // R5寄存器
    *(--stackTop) = (unsigned long)0x4;       // R4寄存器

    // 设置任务的堆栈指针
    task->stack = stackTop;

    // 初始化任务的延时、优先级、状态等字段
    task->delayTicks = 0;
    task->prio = prio;
    task->state = TINYOS_TASK_STATE_RDY;

    // 初始化时间片
    task->slice = TINYOS_SLICE_MAX; 

    // 初始化挂起计数
    task->suspendCount = 0;

    // 清除清理回调函数及其参数
    task->clean = (void (*)(void *))0;
    task->cleanParam = (void *)0;
    task->requestDeleteFlag = 0;

    // 初始化延时队列和优先级队列
    tNodeInit(&(task->delayNode)); // 延时队列
    tNodeInit(&(task->linkNode));  // 优先级队列

    // 将任务加入就绪队列
    tTaskSchedRdy(task);
#if TINYOS_ENABLE_HOOKS == 1
		tHooksTaskInit(task);
#endif

}

/**
 * @brief 挂起任务
 * 
 * 该函数将指定任务挂起，使其不再参与调度。
 * 
 * @param task 需要挂起的任务控制块。
 * 
 * @return void
 */
void tTaskSuspend(tTask * task) {
    uint32_t status = tTaskEnterCritical();
    
    // 如果任务没有处于延时状态，则进行挂起
    if (!(task->state & TINYOS_TASK_STATE_DELAYED)) {
        // 增加挂起计数
        if (++task->suspendCount <= 1) {
            // 设置任务为挂起状态
            task->state |= TINYOS_TASK_STATE_SUSPEND;
            // 从就绪队列移除任务
            tTaskSchedUnRdy(task);
            // 如果挂起的是当前任务，退出临界区并调度
            if (task == curTask) {
                tTaskExitCritical(status);
                tTaskSched();
            }
        }
    }
    
    tTaskExitCritical(status);
}

/**
 * @brief 唤醒任务
 * 
 * 该函数唤醒指定的挂起任务，使其重新进入就绪状态。
 * 
 * @param task 需要唤醒的任务控制块。
 * 
 * @return void
 */
void tTaskWakeUp(tTask * task) {
    uint32_t status = tTaskEnterCritical();
    
    // 如果任务处于挂起状态，则唤醒它
    if (task->state & TINYOS_TASK_STATE_SUSPEND) {
        // 减少挂起计数
        if (--task->suspendCount == 0) {
            // 清除挂起状态
            task->state &= ~(TINYOS_TASK_STATE_SUSPEND);
            // 将任务加入就绪队列
            tTaskSchedRdy(task);
            // 退出临界区并调度
            tTaskExitCritical(status);
            tTaskSched();
        }
    }
    
    tTaskExitCritical(status);
}

/**
 * @brief 设置任务删除回调函数
 * 
 * 设置任务删除时需要执行的清理函数及其参数。删除任务时会调用此回调函数。
 * 
 * @param task 任务控制块
 * @param clean 删除时执行的回调函数
 * @param param 回调函数的参数
 * 
 * @return void
 */
void tTaskSetCleanCallFunc(tTask * task, void (*clean) (void * param), void * param) {
    task->clean = clean;
    task->cleanParam = param;
}

/**
 * @brief 强制删除任务
 * 
 * 该函数会强制删除指定任务，包括从队列中移除，并执行清理函数。
 * 
 * @param task 任务控制块
 * 
 * @return void
 */
void tTaskForceDelete(tTask * task) {
    uint32_t status = tTaskEnterCritical();
    
    // 如果任务处于延时状态，则从延时队列中移除
    if (task->state & TINYOS_TASK_STATE_DELAYED) {
        tTimeTaskRemove(task);
    } else if (!(task->state & TINYOS_TASK_STATE_SUSPEND)) {
        // 如果任务没有被挂起，则从调度队列中移除
        tTaskSchedRemove(task);
    }
    
    // 执行任务的清理函数
    if (task->clean) {
        task->clean(task->cleanParam);
    }
    
    // 如果当前任务被删除，退出临界区并调度
    if (curTask == task) {
        tTaskExitCritical(status);
        tTaskSched();
    }
    
    tTaskExitCritical(status);
}

/**
 * @brief 请求删除任务
 * 
 * 该函数设置任务的删除标志位，表示该任务请求被删除。
 * 
 * @param task 任务控制块
 * 
 * @return void
 */
void tTaskRequestDelete(tTask * task) {
    uint32_t status = tTaskEnterCritical();
    
    // 设置删除标志位
    task->requestDeleteFlag = 1;
    
    tTaskExitCritical(status);
}

/**
 * @brief 查询当前任务是否请求删除
 * 
 * 该函数检查当前任务是否已经请求删除。
 * 
 * @return uint8_t 如果当前任务请求删除，返回1，否则返回0。
 */
uint8_t tTaskIsRequestedDeleted(void) {
    uint8_t delete;
    uint32_t status = tTaskEnterCritical();
    
    // 获取当前任务的删除标志
    delete = curTask->requestDeleteFlag;
    
    tTaskExitCritical(status);
    
    return delete;
}

/**
 * @brief 当前任务删除自身
 * 
 * 该函数会删除当前任务并执行清理工作。任务会从调度队列中移除并调用清理函数。
 * 
 * @return void
 */
void tTaskDeleteSelf(void) {
    uint32_t status = tTaskEnterCritical();
    
    // 从调度队列中移除当前任务
    tTaskSchedRemove(curTask);
    
    // 执行当前任务的清理函数
    if (curTask->clean) {
        curTask->clean(curTask->cleanParam);
    }
    
    tTaskExitCritical(status);
    tTaskSched();
}

/**
 * @brief 查询任务信息
 * 
 * 该函数返回任务的延时、优先级、时间片、状态等信息。
 * 
 * @param task 任务控制块
 * @param info 存储任务信息的结构体
 * 
 * @return void
 */
void tTaskGetInfo(tTask * task, tTaskInfo * info) {
    uint32_t status = tTaskEnterCritical();
    
    // 获取任务的相关信息
    info->delayTicks = task->delayTicks;
    info->prio = task->prio;
    info->slice = task->slice;
    info->state = task->state;
    info->suspendCount = task->suspendCount;
    
	  info->stackSize = task->stackSize;
	  info->stackFree = 0;
	  
	  tTaskStack *stackEnd = task->stackBase;
	  while((*stackEnd++) == 0 && (stackEnd <= task->stackBase + task->stackSize / sizeof(tTaskStack))) {
			info->stackFree++;
		}
	  info->stackFree *= sizeof(tTaskStack);//单位数转化为字节数
    tTaskExitCritical(status);
}

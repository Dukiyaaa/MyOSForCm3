#include "tinyOS.h"

/**
 * @brief 初始化互斥量
 * 
 * @param mutex 指向互斥量的指针
 * 
 * 该函数初始化互斥量，包括设置初始状态，锁定计数器，拥有者以及优先级等。
 */
void tMutexInit(tMutex * mutex) {
    tEventInit(&mutex->event,tEventTypeMutex);  // 初始化事件控制块，类型为互斥量
    mutex->lockedCount = (uint32_t)0;            // 锁定计数器初始为0
    mutex->owner = (tTask *)0;                   // 初始没有任务持有锁
    mutex->ownerOriginalPrio = TINYOS_PRIO_COUNT; // 初始化为最低优先级
}

/**
 * @brief 请求互斥量
 * 
 * @param mutex 指向互斥量的指针
 * @param waitTicks 等待时间（单位：tick），如果设置为0则表示不等待
 * 
 * 等待互斥量的获取，处理四种可能的情况：
 * 1. 互斥量未锁定，当前任务直接锁定；
 * 2. 当前任务已持有锁，进行嵌套锁定；
 * 3. 低优先级任务请求信号量，但高优先级任务正在锁定；
 * 4. 高优先级任务请求信号量，但低优先级任务正在锁定，为了避免优先级反转，将低优先级任务的优先级暂时升到与高优先级任务一致。
 * 
 * @return tErrorNoError 成功获取互斥量
 * @return tErrorResourceUnavaliable 获取互斥量失败
 */
uint32_t tMutexWait (tMutex * mutex, uint32_t waitTicks) {
    uint32_t status = tTaskEnterCritical();  // 进入临界区，保护共享资源

    // 1.互斥量未锁定，当前任务直接锁定
    if(mutex->lockedCount <= 0) {
        mutex->lockedCount++;        // 锁定计数器++
        mutex->owner = curTask;      // 当前任务成为拥有者
        mutex->ownerOriginalPrio = curTask->prio; // 记录当前任务的原优先级
        
        tTaskExitCritical(status);  // 退出临界区
        return tErrorNoError;       // 成功获取锁
    }

    // 2. 锁定该信号量的任务再次锁定(嵌套锁定)，锁定计数器++
    if(curTask == mutex->owner) {
        mutex->lockedCount++;       // 锁定计数器++
        tTaskExitCritical(status);  // 退出临界区
        return tErrorNoError;       // 成功获取锁
    }

    // 3. 低优先级任务请求信号量，但高优先级任务正在锁定
    // 4. 高优先级任务请求信号量，但低优先级任务正在锁定，为了避免优先级反转，将低优先级任务的优先级暂时升到与高优先级任务一致
    if(curTask->prio < mutex->owner->prio) {
        if(mutex->owner->state == TINYOS_TASK_STATE_RDY) {
            tTaskSchedUnRdy(mutex->owner);   // 将拥有锁的任务从就绪队列移除
            mutex->owner->prio = curTask->prio; // 提升拥有者优先级
            tTaskSchedRdy(mutex->owner);     // 将拥有者任务重新加入就绪队列
        } else {
            mutex->owner->prio = curTask->prio; // 任务不在就绪队列中，只需修改优先级
        }
    }

    // 请求互斥量，等待中
    tEventWait(&mutex->event, curTask, (void*)0, tEventTypeMutex, waitTicks);
    
    tTaskExitCritical(status);  // 退出临界区
    
    tTaskSched();  // 调度任务
    
    return curTask->waitEventResult; // 返回当前任务的等待结果
}

/**
 * @brief 请求互斥量，如果当前互斥量没有被锁定则直接获取
 * 
 * @param mutex 指向互斥量的指针
 * 
 * 如果互斥量未被锁定，则直接锁定，若已经锁定则立即返回错误。
 * 
 * @return tErrorNoError 成功获取互斥量
 * @return tErrorResourceUnavaliable 获取互斥量失败
 */
uint32_t tMutexNoWaitGet (tMutex * mutex) {
    uint32_t status = tTaskEnterCritical();  // 进入临界区，保护共享资源

    // 1. 互斥量未锁定，当前任务直接锁定
    if(mutex->lockedCount <= 0) {
        mutex->lockedCount++;        // 锁定计数器++
        mutex->owner = curTask;      // 当前任务成为拥有者
        mutex->ownerOriginalPrio = curTask->prio; // 记录当前任务的原优先级
        
        tTaskExitCritical(status);  // 退出临界区
        return tErrorNoError;       // 成功获取锁
    }

    // 2. 锁定该信号量的任务再次锁定(嵌套锁定)，锁定计数器++
    if(curTask == mutex->owner) {
        mutex->lockedCount++;       // 锁定计数器++
        tTaskExitCritical(status);  // 退出临界区
        return tErrorNoError;       // 成功获取锁
    }

    // 如果当前互斥量已被锁定，直接返回错误
    tTaskExitCritical(status);  
    return tErrorResourceUnavaliable;  // 获取失败
}


/**
 * @brief 释放互斥量
 * 
 * @param mutex 指向互斥量的指针
 * 
 * 释放互斥量时考虑以下情况：
 * 1. 没有任务锁定，则直接返回；
 * 2. 不是拥有者调用释放，认为是非法操作；
 * 3. 锁定计数器大于1，说明任务进行了嵌套锁定，只需减少计数器；
 * 4. 恢复优先级，检查优先级是否比当前任务高，若是则进行重调度。
 * 
 * @return tErrorNoError 成功释放互斥量
 * @return tErrorOwner 非法释放
 */
uint32_t tMutexNotify (tMutex * mutex) {
    uint32_t status = tTaskEnterCritical();  // 进入临界区，保护共享资源
    
    // 1. 没有任务锁定，则直接返回
    if (mutex->lockedCount <= 0) {
        tTaskExitCritical(status);  // 退出临界区
        return tErrorNoError;  // 没有任务锁定，直接返回
    }

    // 2. 不是拥有者调用释放，认为是非法操作
    if (mutex->owner != curTask) {
        tTaskExitCritical(status);  // 退出临界区
        return tErrorOwner;  // 当前任务不是锁的拥有者，返回非法操作错误
    }

    // 3. 锁定计数器大于1，说明任务进行了嵌套锁定，只需减少计数器
    if (--mutex->lockedCount > 0) {
        tTaskExitCritical(status);  // 退出临界区
        return tErrorNoError;  // 锁定计数器大于1，直接返回
    }

    // 4. 恢复优先级，检查优先级是否比当前任务高，若是则进行重调度
    if (curTask->prio != mutex->ownerOriginalPrio) {
        // 若当前任务在就绪队列中，则要先下车再上车
        if (curTask->state == TINYOS_TASK_STATE_RDY) {
            tTaskSchedUnRdy(mutex->owner);  // 从就绪队列移除
            curTask->prio = mutex->ownerOriginalPrio;  // 恢复原优先级
            tTaskSchedRdy(mutex->owner);  // 重新加入就绪队列
        } else {
            // 否则直接修改优先级
            curTask->prio = mutex->ownerOriginalPrio;
        }
    }

    // 如果有任务在等待，则唤醒任务并进行调度
    if (tEventWaitCount(&mutex->event) > 0) {
        tTask * task = tEventWakeUp(&mutex->event, (void *)0, tErrorNoError);  // 唤醒一个等待任务

        // 新任务锁定互斥量
        mutex->owner = task;
        mutex->ownerOriginalPrio = task->prio;
        mutex->lockedCount++;

        // 如果新任务优先级比当前任务低，进行任务调度
        if (task->prio < curTask->prio) {
            tTaskExitCritical(status);  // 退出临界区
            tTaskSched();  // 调度任务
        }
    }

    tTaskExitCritical(status);  // 退出临界区
    return tErrorNoError;  // 成功释放互斥量
}

/**
 * @brief 获取互斥量信息
 * 
 * @param mutex 指向互斥量的指针
 * @param info 指向互斥量信息结构体的指针
 * 
 * 获取互斥量的相关信息，包括等待的任务数、拥有者任务及其优先级等。
 */
void tMutexGetInfo (tMutex * mutex, tMutexInfo * info) {
    uint32_t status = tTaskEnterCritical();  // 进入临界区，保护共享资源

    // 获取互斥量相关信息
    info->taskCount = tEventWaitCount(&mutex->event);  // 获取等待任务数
    info->ownerPrio = mutex->ownerOriginalPrio;  // 获取拥有者的原始优先级
    
    // 判断是否有任务锁定该互斥量
    if (mutex->owner != (tTask *)0) {
        // 如果有任务锁定，则获取其继承的优先级
        info->inheritedPrio = mutex->owner->prio;
    } else {
        // 如果没有任务锁定，则设置为无效优先级
        info->inheritedPrio = TINYOS_PRIO_COUNT;
    }

    info->owner = mutex->owner;  // 获取当前拥有者任务
    info->lockedCount = mutex->lockedCount;  // 获取锁定计数

    tTaskExitCritical(status);  // 退出临界区
}

/**
 * @brief 销毁互斥量
 * 
 * @param mutex 指向互斥量的指针
 * 
 * 销毁互斥量时会恢复任务的优先级，并清空等待队列中的任务。
 * 
 * @return uint32_t 返回清除的等待任务数
 */
uint32_t tMutexDestroy (tMutex * mutex) {
    uint32_t count = 0;
    uint32_t status = tTaskEnterCritical();  // 进入临界区，保护共享资源

    // 判断是否有任务锁定该互斥量
    if (mutex->lockedCount > 0) {
        // 判断是否发生了优先级继承，如果有则恢复原优先级
        if (mutex->ownerOriginalPrio != mutex->owner->prio) {
            if (mutex->owner->state == TINYOS_TASK_STATE_RDY) {
                // 任务处于就绪状态时，更改任务在就绪表中的位置
                tTaskSchedUnRdy(mutex->owner);  // 从就绪队列移除
                mutex->owner->prio = mutex->ownerOriginalPrio;  // 恢复原优先级
                tTaskSchedRdy(mutex->owner);  // 重新加入就绪队列
            } else {
                // 其它状态下只需要恢复优先级
                mutex->owner->prio = mutex->ownerOriginalPrio;
            }
        }

        // 清空事件控制块中的任务
        count = tEventRemoveAll(&mutex->event, (void *)0, tErrorDel);

        // 清空过程中可能有任务就绪，执行一次调度
        if (count > 0) {
            tTaskSched();  // 调度任务
        }
    }

    tTaskExitCritical(status);  // 退出临界区
    return count;  // 返回清除的等待任务数
}

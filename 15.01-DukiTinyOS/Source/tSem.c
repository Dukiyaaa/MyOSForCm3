#include "tSem.h"
#include "tinyOS.h"

//条件编译
#if TINYOS_ENABLE_SEM == 1
/**
 * @brief 初始化信号量。
 * 
 * 该函数用于初始化信号量，设置信号量的初始计数和最大计数。如果最大计数为0，则仅使用初始计数。
 * 
 * @param sem         指向信号量结构体的指针。
 * @param startCount  信号量的初始计数。
 * @param maxCount    信号量的最大计数，0表示没有上限。
 * 
 * @return void
 */
void tSemInit(tSem * sem, uint32_t startCount, uint32_t maxCount) {
    tEventInit(&sem->event, tEventTypeSem);  // 初始化信号量的事件为信号量类型
    sem->maxCount = maxCount;  // 设置信号量的最大计数
    if (maxCount == 0) {
        sem->count = startCount;  // 如果没有最大计数，则信号量计数为初始计数
    } else {
        sem->count = startCount < maxCount ? startCount : maxCount;  // 确保计数不超过最大值
    }
}

/**
 * @brief 阻塞式请求信号量。
 * 
 * 该函数用于请求信号量，如果信号量的计数大于0，则直接分配信号量；否则，当前任务会被挂起，直到信号量可用或超时。
 * 
 * @param sem         指向信号量结构体的指针。
 * @param waitTicks   等待时间（以滴答为单位），如果为0则表示不等待。
 * 
 * @return uint32_t   返回操作结果，`tErrorNoError` 表示成功，`tErrorResourceUnavaliable` 表示资源不可用。
 */
uint32_t tSemWait(tSem * sem, uint32_t waitTicks) {
    uint32_t status = tTaskEnterCritical();  // 进入临界区，保护共享资源

    if (sem->count > 0) {  // 如果信号量有资源
        --sem->count;  // 资源计数减一，表示资源被分配出去
        tTaskExitCritical(status);  // 退出临界区
        return tErrorNoError;  // 返回成功
    } else {
        // 如果资源不足，当前任务请求信号量并进入等待状态
        tEventWait(&sem->event, curTask, (void*) 0, tEventTypeSem, waitTicks);  // 挂起当前任务，等待信号量可用
        tTaskExitCritical(status);  // 退出临界区

        tTaskSched();  // 调度任务

        return curTask->waitEventResult;  // 返回当前任务等待结果，可能是超时或成功获得信号量
    }
}

/**
 * @brief 无等待请求信号量。
 * 
 * 该函数尝试请求信号量，如果信号量的计数大于0，则直接分配信号量；如果信号量不可用，则直接返回错误，不进入等待状态。
 * 
 * @param sem         指向信号量结构体的指针。
 * 
 * @return uint32_t   返回操作结果，`tErrorNoError` 表示成功，`tErrorResourceUnavaliable` 表示资源不可用。
 */
uint32_t tSemNoWaitGet(tSem * sem) {
    uint32_t status = tTaskEnterCritical();  // 进入临界区，保护共享资源

    if (sem->count > 0) {  // 如果信号量有资源
        --sem->count;  // 资源计数减一，表示资源被分配出去
        tTaskExitCritical(status);  // 退出临界区
        return tErrorNoError;  // 返回成功
    } else {
        tTaskExitCritical(status);  // 退出临界区
        return tErrorResourceUnavaliable;  // 返回资源不可用错误
    }
}

/**
 * @brief 通知信号量释放资源。
 * 
 * 该函数用于释放一个信号量，如果有任务在等待该信号量，则唤醒一个等待的任务，并为其分配信号量；如果没有任务等待，则增加信号量计数。
 * 
 * @param sem         指向信号量结构体的指针。
 * 
 * @return void
 */
void tSemNotify(tSem * sem) {
    uint32_t status = tTaskEnterCritical();  // 进入临界区，保护共享资源

    if (tEventWaitCount(&sem->event) > 0) {  // 如果有任务在等待该信号量
        tTask * task = tEventWakeUp(&sem->event, (void *)0, tErrorNoError);  // 唤醒一个等待任务，并传递信号量
        if (task->prio < curTask->prio) {  // 如果唤醒的任务优先级更高，则调度任务
            tTaskExitCritical(status);  // 退出临界区
            tTaskSched();  // 调度任务
        }
    } else {
        ++sem->count;  // 如果没有任务在等待，增加信号量计数
        if ((sem->maxCount != 0) && (sem->count > sem->maxCount)) {  // 如果信号量计数超过最大值，则限制为最大值
            sem->count = sem->maxCount;
        }
    }

    tTaskExitCritical(status);  // 退出临界区
}

/**
 * @brief 查询信号量的当前状态。
 * 
 * 该函数用于查询信号量的当前计数、最大计数和等待的任务数量等信息。
 * 
 * @param sem         指向信号量结构体的指针。
 * @param info        用于存放信号量信息的结构体指针。
 * 
 * @return void
 */
void tSemGetInfo(tSem * sem, tSemInfo * info) {
    uint32_t status = tTaskEnterCritical();  // 进入临界区，保护共享资源

    info->count = sem->count;  // 获取当前信号量计数
    info->maxCount = sem->maxCount;  // 获取最大信号量计数
    info->taskCount = tEventWaitCount(&sem->event);  // 获取等待信号量的任务数量

    tTaskExitCritical(status);  // 退出临界区
}

/**
 * @brief 删除信号量并释放相关资源。
 * 
 * 该函数用于删除信号量并释放与之关联的资源。删除时，如果有任务在等待信号量，则会进行任务调度。
 * 
 * @param sem         指向信号量结构体的指针。
 * 
 * @return uint32_t   返回等待队列中的任务数量。
 */
uint32_t tSemDestory(tSem * sem) {
    uint32_t status = tTaskEnterCritical();  // 进入临界区，保护共享资源

    uint32_t count = tEventRemoveAll(&sem->event, (void *)0, tErrorDel);  // 删除所有等待任务
    sem->count = 0;  // 将信号量计数设置为0
    tTaskExitCritical(status);  // 退出临界区

    if (count > 0) {  // 如果有任务被删除
        tTaskSched();  // 调度任务
    }

    return count;  // 返回删除的任务数量
}
#endif

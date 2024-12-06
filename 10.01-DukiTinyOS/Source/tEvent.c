#include "tEvent.h"
#include "tinyOS.h"

/**
 * @brief 初始化事件对象。
 * 
 * 该函数用于初始化一个事件对象，并设置其类型。同时会初始化事件的等待任务链表。
 * 
 * @param event  指向事件结构体的指针。
 * @param type   事件类型，通常为信号量、邮箱等类型。
 * 
 * @return void
 */
void tEventInit(tEvent * event, tEventType type) {
    event->type = type;  // 设置事件的类型
    tListInit(&(event->waitList));  // 初始化事件等待任务链表
}

/**
 * @brief 当前任务等待事件。
 * 
 * 该函数将当前任务挂起，直到事件满足条件或超时。任务被加入到事件的等待队列中。
 * 
 * @param event    指向事件对象的指针。
 * @param task     指向当前任务结构体的指针。
 * @param msg      事件消息，可以是任意类型的数据，通常用于传递事件相关的信息。
 * @param state    当前任务的状态，任务将被设置为等待状态。
 * @param timeout  等待时间，如果为0则表示不等待。
 * 
 * @return void
 */
void tEventWait(tEvent * event, tTask * task, void * msg, uint32_t state, uint32_t timeout) {
    uint32_t status = tTaskEnterCritical();  // 进入临界区，保护共享资源

    task->state |= state;  // 设置任务的状态为等待状态
    task->waitEvent = event;  // 设置任务等待的事件
    task->eventMsg = msg;  // 存储事件消息
    task->waitEventResult = tErrorNoError;  // 初始设置事件结果为无错误

    tTaskSchedUnRdy(task);  // 将任务从就绪队列中移除，进入等待状态

    tListAddLast(&(event->waitList), &(task->linkNode));  // 将任务加入事件的等待队列

    if (timeout) {
        tTimeTaskWait(task, timeout);  // 如果设置了超时时间，将任务加入延时队列
    }

    tTaskExitCritical(status);  // 退出临界区
}

/**
 * @brief 唤醒等待事件的任务。
 * 
 * 该函数用于唤醒一个等待事件的任务，并设置任务的事件消息和结果。被唤醒的任务会恢复其原状态并重新加入就绪队列。
 * 
 * @param event   指向事件对象的指针。
 * @param msg     事件消息，传递给唤醒任务。
 * @param result  事件的结果，表示任务是否成功获取到事件。
 * 
 * @return tTask* 返回被唤醒的任务指针。如果没有任务等待事件，则返回NULL。
 */
tTask * tEventWakeUp(tEvent * event, void * msg, uint32_t result) {
    tNode * node;
    tTask * task = (tTask *)0;

    uint32_t status = tTaskEnterCritical();  // 进入临界区，保护共享资源

    // 从事件的等待队列中移除第一个任务
    if ((node = tListRemoveFirst(&event->waitList)) != (tNode *)0) {
        task = (tTask *)tNodeParent(node, tTask, linkNode);  // 获取等待的任务指针
        task->waitEvent = (tEvent *)0;  // 清除任务的等待事件
        task->eventMsg = msg;  // 设置任务的事件消息
        task->waitEventResult = result;  // 设置任务的等待结果
        task->state &= ~TINYOS_TASK_WAIT_MASK;  // 清除任务的等待状态

        if (task->delayTicks != 0) {
            tTimeTaskWakeUp(task);  // 如果任务被延时，唤醒任务
        }

        tTaskSchedRdy(task);  // 将任务加入就绪队列
    }

    tTaskExitCritical(status);  // 退出临界区
    return task;  // 返回被唤醒的任务指针
}

tTask * tEventWakeUpTask(tEvent * event, tTask * task, void * msg, uint32_t result) {
    uint32_t status = tTaskEnterCritical();  // 进入临界区，保护共享资源

		tListRemove(&event->waitList,&task->linkNode);
		task->waitEvent = (tEvent *)0;  // 清除任务的等待事件
		task->eventMsg = msg;  // 设置任务的事件消息
		task->waitEventResult = result;  // 设置任务的等待结果
		task->state &= ~TINYOS_TASK_WAIT_MASK;  // 清除任务的等待状态

		if (task->delayTicks != 0) {
				tTimeTaskWakeUp(task);  // 如果任务被延时，唤醒任务
		}

		tTaskSchedRdy(task);  // 将任务加入就绪队列

    tTaskExitCritical(status);  // 退出临界区
		
    return task;  // 返回被唤醒的任务指针
}

/**
 * @brief 从事件的等待队列中移除指定任务。
 * 
 * 该函数用于从事件的等待队列中移除指定任务，并设置该任务的事件消息和结果。任务将被设置为不再等待。
 * 
 * @param task    指向任务结构体的指针。
 * @param msg     事件消息，传递给任务。
 * @param result  事件的结果，表示任务的等待结果。
 * 
 * @return void
 */
void tEventRemoveTask(tTask * task, void * msg, uint32_t result) {
    uint32_t status = tTaskEnterCritical();  // 进入临界区，保护共享资源

    tListRemove(&task->waitEvent->waitList, &task->linkNode);  // 从事件的等待队列中移除任务
    task->waitEvent = (tEvent *)0;  // 清除任务的等待事件
    task->eventMsg = msg;  // 设置任务的事件消息
    task->waitEventResult = result;  // 设置任务的等待结果
    task->state &= ~TINYOS_TASK_WAIT_MASK;  // 清除任务的等待状态

    tTaskExitCritical(status);  // 退出临界区
}

/**
 * @brief 移除事件所有等待任务。
 * 
 * 该函数用于移除所有等待该事件的任务，并为这些任务设置事件消息和结果。所有被移除的任务会被加入到就绪队列。
 * 
 * @param event   指向事件对象的指针。
 * @param msg     事件消息，传递给所有任务。
 * @param result  事件的结果，表示任务的等待结果。
 * 
 * @return uint32_t 返回被移除的任务数量。
 */
uint32_t tEventRemoveAll(tEvent * event, void * msg, uint32_t result) {
    uint32_t count = 0;
    tNode * node;
    uint32_t status = tTaskEnterCritical();  // 进入临界区，保护共享资源

    count = tListCount(&event->waitList);  // 获取等待任务的数量

    // 移除事件等待队列中的所有任务
    while ((node = tListRemoveFirst(&event->waitList)) != (tNode *)0) {
        tTask * task = (tTask *)tNodeParent(node, tTask, linkNode);  // 获取任务指针
        task->waitEvent = (tEvent *)0;  // 清除任务的等待事件
        task->eventMsg = msg;  // 设置任务的事件消息
        task->waitEventResult = result;  // 设置任务的等待结果
        task->state &= ~TINYOS_TASK_WAIT_MASK;  // 清除任务的等待状态

        if (task->delayTicks != 0) {
            tTimeTaskWakeUp(task);  // 如果任务被延时，唤醒任务
        }

        tTaskSchedRdy(task);  // 将任务加入就绪队列
    }

    tTaskExitCritical(status);  // 退出临界区
    return count;  // 返回被移除的任务数量
}

/**
 * @brief 获取事件的等待任务数量。
 * 
 * 该函数返回当前等待该事件的任务数量。
 * 
 * @param event   指向事件对象的指针。
 * 
 * @return uint32_t 返回等待任务的数量。
 */
uint32_t tEventWaitCount(tEvent * event) {
    uint32_t count = 0;

    uint32_t status = tTaskEnterCritical();  // 进入临界区，保护共享资源

    count = tListCount(&event->waitList);  // 获取等待队列中任务的数量

    tTaskExitCritical(status);  // 退出临界区
    return count;  // 返回等待队列中的任务数量
}

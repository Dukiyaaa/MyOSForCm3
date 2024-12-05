#include "tinyOS.h"

/**
 * @brief 初始化邮箱。
 * 
 * 该函数用于初始化一个邮箱，将邮箱的事件类型设置为邮箱类型，并为其分配消息缓冲区、设置最大容量等。
 * 
 * @param mbox        指向要初始化的邮箱结构体的指针。
 * @param msgBuffer   指向消息缓冲区的指针，用于存储消息。
 * @param maxCount    邮箱能够容纳的最大消息数。
 * 
 * @return void
 */
void tMboxInit(tMbox * mbox, void ** msgBuffer, uint32_t maxCount) {
    tEventInit(&mbox->event, tEventTypeMbox);  // 初始化事件为邮箱类型

    mbox->msgBuffer = msgBuffer;    // 设置消息缓冲区
    mbox->maxCount = maxCount;      // 设置最大消息容量
    mbox->count = 0;                // 初始化当前消息数
    mbox->read = 0;                 // 初始化读取指针
    mbox->write = 0;                // 初始化写入指针
}

/**
 * @brief 阻塞式获取邮箱消息。
 * 
 * 该函数尝试从邮箱中获取一条消息。如果邮箱为空且没有等待的任务，则当前任务进入阻塞状态，直到有消息可用或超时。
 * 
 * @param mbox        指向邮箱结构体的指针。
 * @param msg         用于存放获取到的消息的指针。
 * @param waitTicks   等待的时间（以滴答为单位），如果为0则表示不等待。
 * 
 * @return uint32_t   返回操作结果，`tErrorNoError` 表示成功，`tErrorResourceUnavaliable` 表示邮箱为空。
 */
uint32_t tMboxWait(tMbox * mbox, void ** msg, uint32_t waitTicks) {
    uint32_t status = tTaskEnterCritical();   // 进入临界区，保护共享资源

    if (mbox->count > 0) {  // 如果邮箱中有消息
        --mbox->count;   // 消息数减一
        *msg = mbox->msgBuffer[mbox->read++];  // 获取消息并更新读取指针
        if (mbox->read >= mbox->maxCount) {    // 如果读取指针越界，则回绕
            mbox->read = 0;
        }
        tTaskExitCritical(status);  // 退出临界区
        return tErrorNoError;  // 返回成功
    } else {
        // 如果邮箱为空，任务等待邮箱事件
        tEventWait(&mbox->event, curTask, (void *)0, tEventTypeMbox, waitTicks);
        tTaskExitCritical(status);  // 退出临界区

        tTaskSched();  // 调度任务

        *msg = curTask->eventMsg;  // 获取当前任务的事件消息
        return curTask->waitEventResult;  // 返回等待结果
    }
}

/**
 * @brief 非阻塞式获取邮箱消息。
 * 
 * 该函数尝试从邮箱中获取一条消息。如果邮箱为空，则直接返回，不进入阻塞状态。
 * 
 * @param mbox        指向邮箱结构体的指针。
 * @param msg         用于存放获取到的消息的指针。
 * 
 * @return uint32_t   返回操作结果，`tErrorNoError` 表示成功，`tErrorResourceUnavaliable` 表示邮箱为空。
 */
uint32_t tMboxNoWaitGet(tMbox * mbox, void ** msg) {
    uint32_t status = tTaskEnterCritical();  // 进入临界区，保护共享资源

    if (mbox->count > 0) {  // 如果邮箱中有消息
        --mbox->count;  // 消息数减一
        *msg = mbox->msgBuffer[mbox->read++];  // 获取消息并更新读取指针
        if (mbox->read >= mbox->maxCount) {    // 如果读取指针越界，则回绕
            mbox->read = 0;
        }
        tTaskExitCritical(status);  // 退出临界区
        return tErrorNoError;  // 返回成功
    } else {
        tTaskExitCritical(status);  // 退出临界区
        return tErrorResourceUnavaliable;  // 返回邮箱为空的错误
    }
}

/**
 * @brief 通知邮箱中的等待任务并释放邮箱资源。
 * 
 * 如果邮箱中有任务在等待，则唤醒一个等待任务并传递消息。如果没有任务等待，则将消息插入邮箱。
 * 
 * @param mbox        指向邮箱结构体的指针。
 * @param msg         要发送的消息内容。
 * @param notifyOption   通知选项，决定将消息插入邮箱的头部还是尾部。
 * 
 * @return uint32_t   返回操作结果，`tErrorNoError` 表示成功，`tErrorResourceFull` 表示邮箱已满。
 */
uint32_t tMboxNotify(tMbox * mbox, void * msg, uint32_t notifyOption) {
    uint32_t status = tTaskEnterCritical();  // 进入临界区，保护共享资源

    // 如果有任务在等待邮箱
    if (tEventWaitCount(&mbox->event) > 0) {
        tTask * task = tEventWakeUp(&mbox->event, (void *)msg, tErrorNoError);  // 唤醒任务并传递消息
        if (task->prio < curTask->prio) {  // 如果唤醒的任务优先级更高，则进行任务调度
            tTaskExitCritical(status);  // 退出临界区
            tTaskSched();  // 调度任务
        }
    } else {
        // 如果没有任务等待，检查邮箱是否已满
        if (mbox->count >= mbox->maxCount) {
            tTaskExitCritical(status);  // 退出临界区
            return tErrorResourceFull;  // 返回邮箱已满错误
        }

        // 根据 notifyOption 决定是将消息插入邮箱的头部还是尾部
        if (notifyOption & tMboxSendFront) {
            if (mbox->read <= 0) {
                mbox->read = mbox->maxCount - 1;  // 如果读取指针为0，回绕到最后一个位置
            } else {
                --mbox->read;  // 向前移动读取指针
            }
            mbox->msgBuffer[mbox->read] = msg;  // 插入消息到头部
        } else {
            mbox->msgBuffer[mbox->write++] = msg;  // 插入消息到尾部并更新写入指针
            if (mbox->write >= mbox->maxCount) {  // 如果写入指针越界，则回绕
                mbox->write = 0;
            }
        }
        mbox->count++;  // 增加消息计数
    }

    tTaskExitCritical(status);  // 退出临界区
    return tErrorNoError;  // 返回成功
}

/**
 * @brief 清空邮箱中的所有消息。
 * 
 * 该函数会清空邮箱中的所有消息，仅在没有等待任务的情况下进行清空操作。
 * 
 * @param mbox        指向邮箱结构体的指针。
 * 
 * @return void
 */
void tMboxFlush(tMbox * mbox) {
    uint32_t status = tTaskEnterCritical();  // 进入临界区，保护共享资源

    if (tEventWaitCount(&mbox->event) == 0) {  // 如果没有任务等待
        mbox->count = 0;  // 清空消息计数
        mbox->read = 0;   // 重置读取指针
        mbox->write = 0;  // 重置写入指针
    }

    tTaskExitCritical(status);  // 退出临界区
}

/**
 * @brief 删除邮箱并释放相关资源。
 * 
 * 该函数会删除邮箱相关的任务列表，并释放相关资源。如果有任务在等待邮箱事件，会进行任务调度。
 * 
 * @param mbox        指向邮箱结构体的指针。
 * 
 * @return uint32_t   返回删除的任务数量。
 */
uint32_t tMboxDestory(tMbox * mbox) {
    uint32_t status = tTaskEnterCritical();  // 进入临界区，保护共享资源

    uint32_t count = tEventRemoveAll(&mbox->event, (void *)0, tErrorDel);  // 删除所有等待任务
    tTaskExitCritical(status);  // 退出临界区

    if (count > 0) {  // 如果有任务被删除
        tTaskSched();  // 调度任务
    }

    return count;  // 返回删除的任务数量
}

/**
 * @brief 获取邮箱的当前状态信息。
 * 
 * 该函数返回邮箱的当前状态，包括消息数量、最大容量和等待任务数量等信息。
 * 
 * @param mbox        指向邮箱结构体的指针。
 * @param info        指向用于存放邮箱信息的结构体指针。
 * 
 * @return void
 */
void tMboxGetInfo(tMbox * mbox, tMboxInfo * info) {
    uint32_t status = tTaskEnterCritical();  // 进入临界区，保护共享资源

    info->count = mbox->count;  // 获取当前消息数
    info->maxCount = mbox->maxCount;  // 获取邮箱的最大容量
    info->taskCount = tEventWaitCount(&mbox->event);  // 获取等待任务数

    tTaskExitCritical(status);  // 退出临界区
}

#include "tinyOS.h"

/**
 * @brief 初始化内存块。
 * 
 * 该函数用于初始化内存块，将内存块的相关信息进行设置，并将内存块划分为多个内存块单元。
 * 
 * @param memBlock   指向要初始化的内存块结构体的指针。
 * @param memStart   指向内存起始地址的指针。
 * @param blockSize  每个内存块的大小。
 * @param blockCnt   内存块的数量。
 * 
 * @return void
 */
void tMemBlockInit(tMemBlock * memBlock, uint8_t * memStart, uint32_t blockSize, uint32_t blockCnt) {
    uint8_t * memBlockStart = (uint8_t *) memStart;   // 初始化内存块起始地址
    uint8_t * memBlockEnd = (uint8_t *)(memBlockStart + blockCnt * blockSize);  // 计算内存块的结束地址
    
    if (blockSize < sizeof(tNode)) {  // 确保每个内存块的大小大于 tNode 类型的大小
        return;  // 如果内存块大小不够，直接返回
    }

    tEventInit(&memBlock->event, tEventTypeMemBlock);  // 初始化事件，设置为内存块事件类型
    
    memBlock->memStart = memBlockStart;  // 设置内存块起始地址
    memBlock->maxCount = blockCnt;  // 设置最大内存块数
    memBlock->blockSize = blockSize;  // 设置每个内存块的大小
    
    tListInit(&memBlock->blockList);  // 初始化内存块的链表
    
    while (memBlockStart < memBlockEnd) {  // 遍历每个内存块
        tNodeInit((tNode*)memBlockStart);  // 初始化当前内存块为链表节点
        tListAddLast(&memBlock->blockList, (tNode*)memBlockStart);  // 将当前节点添加到链表中
        
        memBlockStart += blockSize;  // 移动到下一个内存块的位置
    }
}

/**
 * @brief 阻塞式获取内存块。
 * 
 * 该函数尝试从内存块池中获取一个内存块。如果内存池中没有可用的内存块，则当前任务会阻塞，直到有内存块可用或者超时。
 * 
 * @param memBlock   指向内存块池的指针。
 * @param mem        指向接收内存块指针的指针。
 * @param waitTicks  等待时间（以滴答为单位），为0表示不等待。
 * 
 * @return uint32_t  返回操作结果，`tErrorNoError` 表示成功，`tErrorResourceUnavaliable` 表示资源不可用。
 */
uint32_t tMemBlockWait(tMemBlock * memBlock, uint8_t ** mem, uint32_t waitTicks) {
    uint32_t status = tTaskEnterCritical();   // 进入临界区，保护共享资源

    if (tListCount(&memBlock->blockList) > 0) {  // 如果有可用的内存块
        *mem = (uint8_t *) tListRemoveFirst(&memBlock->blockList);  // 从链表中移除并返回内存块
        tTaskExitCritical(status);  // 退出临界区
        return tErrorNoError;  // 返回成功
    } else {
        // 如果内存池为空，任务等待内存块事件
        tEventWait(&memBlock->event, curTask, (void *)0, tEventTypeMemBlock, waitTicks);
        tTaskExitCritical(status);  // 退出临界区

        tTaskSched();  // 调度任务

        *mem = (uint8_t *) curTask->eventMsg;  // 获取当前任务的事件消息（即分配的内存块）
        return curTask->waitEventResult;  // 返回等待的结果
    }
}

/**
 * @brief 非阻塞式获取内存块。
 * 
 * 该函数尝试从内存块池中获取一个内存块。如果内存池中没有可用的内存块，则直接返回，不进行阻塞。
 * 
 * @param memBlock   指向内存块池的指针。
 * @param mem        指向接收内存块指针的指针。
 * 
 * @return uint32_t  返回操作结果，`tErrorNoError` 表示成功，`tErrorResourceUnavaliable` 表示资源不可用。
 */
uint32_t tMemBlockNoWaitGet(tMemBlock * memBlock, uint8_t ** mem, uint32_t waitTicks) {
    uint32_t status = tTaskEnterCritical();  // 进入临界区，保护共享资源

    if (tListCount(&memBlock->blockList) > 0) {  // 如果有可用的内存块
        *mem = (uint8_t *) tListRemoveFirst(&memBlock->blockList);  // 从链表中移除并返回内存块
        tTaskExitCritical(status);  // 退出临界区
        return tErrorNoError;  // 返回成功
    } else {
        tTaskExitCritical(status);  // 退出临界区
        return tErrorResourceUnavaliable;  // 返回资源不可用
    }
}

/**
 * @brief 通知内存块池中的等待任务。
 * 
 * 如果有任务在等待内存块，则唤醒一个任务并分配内存块。如果没有任务等待，则将内存块插入池中。
 * 
 * @param memBlock   指向内存块池的指针。
 * @param mem        要分配的内存块。
 * 
 * @return uint32_t  返回操作结果，`tErrorNoError` 表示成功。
 */
uint32_t tMemBlockNotify(tMemBlock * memBlock, uint8_t *mem) {
    uint32_t status = tTaskEnterCritical();  // 进入临界区，保护共享资源

    if (tEventWaitCount(&memBlock->event) > 0) {  // 如果有任务在等待内存块
        tTask * task = tEventWakeUp(&memBlock->event, (void *)mem, tErrorNoError);  // 唤醒任务并传递内存块
        if (task->prio < curTask->prio) {  // 如果唤醒的任务优先级更高，则进行任务调度
            tTaskExitCritical(status);  // 退出临界区
            tTaskSched();  // 调度任务
        }
    } else {
        tListAddLast(&memBlock->blockList, (tNode*)mem);  // 如果没有任务等待，将内存块插入池中
    }

    tTaskExitCritical(status);  // 退出临界区
    return tErrorNoError;  // 返回成功
}

/**
 * @brief 获取内存块池的当前状态信息。
 * 
 * 该函数返回内存块池的当前状态，包括可用内存块数、最大内存块数和等待任务数等信息。
 * 
 * @param memBlock   指向内存块池的指针。
 * @param info       用于存放内存块池信息的结构体指针。
 * 
 * @return void
 */
void tMemBlockGetInfo(tMemBlock * memBlock, tMemBlockInfo * info) {
    uint32_t status = tTaskEnterCritical();  // 进入临界区，保护共享资源

    info->count = tListCount(&memBlock->blockList);  // 获取当前可用内存块数
    info->maxCount = memBlock->maxCount;  // 获取最大内存块数
    info->taskCount = tEventWaitCount(&memBlock->event);  // 获取等待任务数
    info->blockSize = memBlock->blockSize;  // 获取内存块大小
    
    tTaskExitCritical(status);  // 退出临界区
}

/**
 * @brief 销毁内存块池。
 * 
 * 该函数会销毁内存块池，并删除所有等待的任务。
 * 
 * @param memBlock   指向内存块池的指针。
 * 
 * @return uint32_t  返回被删除任务的数量。
 */
uint32_t tMemBlockDestory(tMemBlock * memBlock) {
    uint32_t status = tTaskEnterCritical();  // 进入临界区，保护共享资源

    uint32_t count = tEventRemoveAll(&memBlock->event, (void *)0, tErrorDel);  // 删除所有等待任务
    tTaskExitCritical(status);  // 退出临界区

    if (count > 0) {  // 如果有任务被删除
        tTaskSched();  // 调度任务
    }

    return count;  // 返回删除的任务数量
}

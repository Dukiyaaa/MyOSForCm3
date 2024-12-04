#include "tinyOS.h"

void tMemBlockInit(tMemBlock * memBlock, uint8_t * memStart, uint32_t blockSize, uint32_t blockCnt) {
	//确定该存储块的起始和终止地址
	uint8_t * memBlockStart = (uint8_t *) memStart;
	uint8_t * memBlockEnd = (uint8_t *)(memBlockStart + blockCnt * blockSize);
	
	if(blockSize < sizeof(tNode)) {
		return;
	}//存储块的大小需要比tNode类型大，因为其内需要存放链接指针
	
	tEventInit(&memBlock->event,tEventTypeMemBlock);//初始化事件块
	
	memBlock->memStart = memBlockStart;
	memBlock->maxCount = blockCnt; 
	memBlock->blockSize = blockSize;
	
	tListInit(&memBlock->blockList);
	
	while(memBlockStart < memBlockEnd) {
		//event等待队列的初始化不用初始化结点是因为其还不需要插入节点
		//而内存块本身的块链表则是需要初始化的，有多少个内存块就要插入多少个结点
		tNodeInit((tNode*)memBlockStart);
		tListAddLast(&memBlock->blockList, (tNode*)memBlockStart);
		
		memBlockStart += blockSize;//相当于blockSize的空间内，前一部分放的是tNode，后面一部分是该内存块的其余内容
	}
}

uint32_t tMemBlockWait(tMemBlock * memBlock, uint8_t ** mem, uint32_t waitTicks) {
    uint32_t status = tTaskEnterCritical();   // 进入临界区，保护共享资源

    if (tListCount(&memBlock->blockList) > 0) {  
        * mem = (uint8_t *) tListRemoveFirst(&memBlock->blockList);
			  //从内存列表中取出一块放入mem,注意取的是内存起点地址，所以是指针类型
        tTaskExitCritical(status);  // 退出临界区
        return tErrorNoError;  // 返回成功
    } else {
        // 如果邮箱为空，任务等待邮箱事件
        tEventWait(&memBlock->event, curTask, (void *)0, tEventTypeMemBlock, waitTicks);
        tTaskExitCritical(status);  // 退出临界区

        tTaskSched();  // 调度任务

        *mem = (uint8_t *)curTask->eventMsg;  // 获取当前任务的事件消息
        return curTask->waitEventResult;  // 返回等待结果
    }
}

uint32_t tMemBlockNoWaitGet(tMemBlock * memBlock, uint8_t ** mem, uint32_t waitTicks) {
    uint32_t status = tTaskEnterCritical();   // 进入临界区，保护共享资源

    if (tListCount(&memBlock->blockList) > 0) {  
        * mem = (uint8_t *) tListRemoveFirst(&memBlock->blockList);//从内存列表中取出一块放入mem
        tTaskExitCritical(status);  // 退出临界区
        return tErrorNoError;  // 返回成功
    } else {
        tTaskExitCritical(status);  // 退出临界区
        return tErrorResourceUnavaliable;  // 返回无资源消息
    }
}

uint32_t tMemBlockNotify(tMemBlock * memBlock, uint8_t *mem) {
    uint32_t status = tTaskEnterCritical();  // 进入临界区，保护共享资源

    if (tEventWaitCount(&memBlock->event) > 0) {
        tTask * task = tEventWakeUp(&memBlock->event, (void *)mem, tErrorNoError);  // 唤醒任务并传递消息
        if (task->prio < curTask->prio) {  // 如果唤醒的任务优先级更高，则进行任务调度
            tTaskExitCritical(status);  // 退出临界区
            tTaskSched();  // 调度任务
        }
    } else {
				tListAddLast(&memBlock->blockList,(tNode*)mem);
    }
    tTaskExitCritical(status);  // 退出临界区
    return tErrorNoError;  // 返回成功
}

void tMemBlockGetInfo(tMemBlock * memBlock, tMemBlockInfo * info) {
    uint32_t status = tTaskEnterCritical();  // 进入临界区，保护共享资源

    info->count = tListCount(&memBlock->blockList);  // 获取当前存储块数
    info->maxCount = memBlock->maxCount;  // 获取存储块的最大容量
    info->taskCount = tEventWaitCount(&memBlock->event);  // 获取等待任务数
    info->blockSize = memBlock->blockSize;
    tTaskExitCritical(status);  // 退出临界区
}

uint32_t tMemBlockDestory(tMemBlock * memBlock) {
    uint32_t status = tTaskEnterCritical();  // 进入临界区，保护共享资源

    uint32_t count = tEventRemoveAll(&memBlock->event, (void *)0, tErrorDel);  // 删除所有等待任务
    tTaskExitCritical(status);  // 退出临界区

	  //有个疑问 需不需要删除内存块
    if (count > 0) {  // 如果有任务被删除
        tTaskSched();  // 调度任务
    }

    return count;  // 返回删除的任务数量
}

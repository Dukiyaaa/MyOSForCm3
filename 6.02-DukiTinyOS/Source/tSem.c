#include "tSem.h"
#include "tinyOS.h"

//信号量初始化
void tSemInit(tSem * sem, uint32_t startCount, uint32_t maxCount) {
	tEventInit(&sem->event, tEventTypeSem);
	sem->maxCount = maxCount;
	if(maxCount == 0) {
		sem->count = startCount;
	}else {
		sem->count = startCount < maxCount ? startCount : maxCount;
	}
}

//有等待请求
uint32_t tSemWait(tSem * sem, uint32_t waitTicks) {
	uint32_t status = tTaskEnterCritical();
	
	if(sem->count > 0) {
		//如果有资源可以分配，那么该资源计数减一，表示资源被拿走了一个
		--sem->count;
		tTaskExitCritical(status);
		return tErrorNoError;
	}else {
		//资源不够用，那么请求该资源的任务就要加入该资源的事件等待队列
		tEventWait(&sem->event, curTask, (void*) 0, tEventTypeSem, waitTicks);//这个地方的state参数有点没懂
		tTaskExitCritical(status);
		
		tTaskSched();
		
		return curTask->waitEventResult;//返回对于该资源的等待结果，要么等到了要么超时
	}
}

//无等待请求
uint32_t tSemNoWaitGet(tSem * sem) {
	uint32_t status = tTaskEnterCritical();
	
	if(sem->count > 0) {
		//如果有资源可以分配，那么该资源计数减一，表示资源被拿走了一个
		--sem->count;
		tTaskExitCritical(status);
		return tErrorNoError;
	}else {
		tTaskExitCritical(status);	
		return tErrorResourceUnavaliable;
	}
}

//资源释放
void tSemNotify(tSem * sem) {
	uint32_t status = tTaskEnterCritical();
	
	if(tEventWaitCount(&sem->event) > 0) {
		//有任务在等这个资源，所以这个资源释放后直接拿给在等这个资源的任务用
		tTask * task = tEventWakeUp(&sem->event, (void *)0, tErrorNoError);
		if(task->prio < curTask->prio) {
			tTaskSched();
		}
	}else {
		++sem->count;
		if((sem->maxCount != 0) && (sem->count > sem->maxCount)) {
			sem->count = sem->maxCount;
		}
	}		
	tTaskExitCritical(status);	
}

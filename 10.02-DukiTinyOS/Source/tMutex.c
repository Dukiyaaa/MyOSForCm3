#include "tinyOS.h"

void tMutexInit(tMutex * mutex) {
	tEventInit(&mutex->event,tEventTypeMutex);
	mutex->lockedCount = (uint32_t)0;
	mutex->owner = (tTask *)0;
	mutex->ownerOriginalPrio = TINYOS_PRIO_COUNT;//初始化为最低
}

/*
等待分为四种情况：
1.互斥量未锁定，当前任务直接锁定
2.锁定该信号量的任务再次锁定(嵌套锁定),锁定计数器++
3.低优先级任务请求信号量，但高优先级任务正在锁定，则低优先级任务加入等待队列
4.高优先级任务请求信号量，但低优先级任务正在锁定，为了避免优先级反转，将低优先级任务的优先级暂时升到跟高优先级任务一致

*/
uint32_t tMutexWait (tMutex * mutex, uint32_t waitTicks) {
	uint32_t status = tTaskEnterCritical();
	
	//1.互斥量未锁定，当前任务直接锁定
	if(mutex->lockedCount <= 0) {
		
		mutex->lockedCount++;
		mutex->owner = curTask;
		mutex->ownerOriginalPrio = curTask->prio;
		
		tTaskExitCritical(status);
		return tErrorNoError;
	}
	
	//2.锁定该信号量的任务再次锁定(嵌套锁定),锁定计数器++
	if(curTask == mutex->owner) {
		
		mutex->lockedCount++;
		tTaskExitCritical(status);
		return tErrorNoError;
	}
	
	//3.低优先级任务请求信号量，但高优先级任务正在锁定，则低优先级任务加入等待队列
	//4.高优先级任务请求信号量，但低优先级任务正在锁定，为了避免优先级反转，将低优先级任务的优先级暂时升到跟高优先级任务一致
	if(curTask->prio < mutex->owner->prio) {
		//值小的优先级高 不要忘了
		if(mutex->owner->state == TINYOS_TASK_STATE_RDY) {
			tTaskSchedUnRdy(mutex->owner);
			mutex->owner->prio = curTask->prio;
			tTaskSchedRdy(mutex->owner);
		}	else {
			mutex->owner->prio = curTask->prio;
		}
	}
	
	tEventWait(&mutex->event, curTask, (void*)0, tEventTypeMutex,waitTicks);
	
	tTaskExitCritical(status);
	
	tTaskSched();
	
	return curTask->waitEventResult;
}

uint32_t tMutexNoWaitGet (tMutex * mutex) {
	uint32_t status = tTaskEnterCritical();
	
	//1.互斥量未锁定，当前任务直接锁定
	if(mutex->lockedCount <= 0) {
		
		mutex->lockedCount++;
		mutex->owner = curTask;
		mutex->ownerOriginalPrio = curTask->prio;
		
		tTaskExitCritical(status);
		return tErrorNoError;
	}
	
	//2.锁定该信号量的任务再次锁定(嵌套锁定),锁定计数器++
	if(curTask == mutex->owner) {
		
		mutex->lockedCount++;
		tTaskExitCritical(status);
		return tErrorNoError;
	}
	
	//不等了
	tTaskExitCritical(status);	
	return tErrorResourceUnavaliable;
}

/*
互斥量释放：
1.没有任务锁定，则直接离开
2.不是拥有者释放，认为是非法
3.还没有完全释放完，直接离开
4.恢复优先级，检查优先级是否比当前任务高，是的话需要进行重调度
*/

uint32_t tMutexNotify (tMutex * mutex) {
	uint32_t status = tTaskEnterCritical();
	//1.没有任务锁定，则直接离开
	if(mutex->lockedCount <= 0) {
		tTaskExitCritical(status);
		return tErrorNoError;
	}
	
	//2. 不是拥有者释放，认为是非法
	if (mutex->owner != curTask)
	{
			
			tTaskExitCritical(status);
			return tErrorOwner;
	}
	
	//3.还没有完全释放完，直接离开
	if(--mutex->lockedCount > 0) {
		tTaskExitCritical(status);
		return tErrorNoError;
	}
	
	//进行了优先级覆盖
	if(curTask->prio != mutex->ownerOriginalPrio) {
		//若当前任务在就绪队列中，则要先下车再上车
		if(curTask->state == TINYOS_TASK_STATE_RDY) {
			tTaskSchedUnRdy(mutex->owner);
			curTask->prio = mutex->ownerOriginalPrio;
			tTaskSchedRdy(mutex->owner);
		}else {
			//否则直接赋值就好
			curTask->prio = mutex->ownerOriginalPrio;
		}
	}
	
	//优先级调整完毕，开始考虑任务调度
	if(tEventWaitCount(&mutex->event) > 0) {
		//有任务在等待，则要把互斥量给它，并考虑调度
		tTask * task = tEventWakeUp(&mutex->event,(void *)0, tErrorNoError);
		
		//新任务锁定互斥量
		mutex->owner = task;
		mutex->ownerOriginalPrio = task->prio;
		mutex->lockedCount++;
		
		if(task->prio < curTask->prio) {
			tTaskExitCritical(status);
			tTaskSched();
		}
	}
	
	tTaskExitCritical(status);
	return tErrorNoError;
}

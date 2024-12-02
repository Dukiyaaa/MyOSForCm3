#include "tinyOS.h"

//定义任务初始化函数
void tTaskInit(tTask *tTask, void(*entry)(void *), void * param, uint32_t prio, tTaskStack * tTaskStack) {
	*(--tTaskStack) = (unsigned long)(1 << 24); //xpsr,第24位置1进入thumb模式
	*(--tTaskStack) = (unsigned long)entry;     //pc寄存器，即R15，设定为程序入口
	*(--tTaskStack) = (unsigned long)0x14;      //LR寄存器，即R14
	*(--tTaskStack) = (unsigned long)0x12;      //R12寄存器，R13寄存器是SP
	*(--tTaskStack) = (unsigned long)0x3;			  //R3
	*(--tTaskStack) = (unsigned long)0x2;       //R2
	*(--tTaskStack) = (unsigned long)0x1;       //R1
	*(--tTaskStack) = (unsigned long)param;     //参数
	
	*(--tTaskStack) = (unsigned long)0x11;      //R11
	*(--tTaskStack) = (unsigned long)0x10;      //R10
	*(--tTaskStack) = (unsigned long)0x9;       //R9
	*(--tTaskStack) = (unsigned long)0x8;       //R8
	*(--tTaskStack) = (unsigned long)0x7;       //R7
	*(--tTaskStack) = (unsigned long)0x6;       //R6
	*(--tTaskStack) = (unsigned long)0x5;       //R5
	*(--tTaskStack) = (unsigned long)0x4;       //R4
	tTask->stack = tTaskStack; 
	
	tTask->delayTicks = 0;
	tTask->prio = prio;
	tTask->state = TINYOS_TASK_STATE_RDY;
	
	tTask->slice = TINYOS_SLICE_MAX; //时间片初始化
	
	tTask->suspendCount = 0;
	
	tTask->clean = (void (*)(void *))0;
	tTask->cleanParam = (void *)0;
	tTask->requestDeleteFlag = 0;
		
	tNodeInit(&(tTask->delayNode)); //延时队列
	tNodeInit(&(tTask->linkNode));  //优先级队列
	
	tTaskSchedRdy(tTask);
}

//挂起函数
void tTaskSuspend(tTask * task) {
	uint32_t status = tTaskEnterCritical();
	
	if(!(task->state & TINYOS_TASK_STATE_DELAYED)) {
		if(++task->suspendCount <= 1) {
			task->state |= TINYOS_TASK_STATE_SUSPEND;
			tTaskSchedUnRdy(task);
			if(task == curTask) {
				tTaskExitCritical(status);
				tTaskSched();
			}
		}
	}
	
	tTaskExitCritical(status);
}

void tTaskWakeUp(tTask * task) {
	uint32_t status = tTaskEnterCritical();
	
	if(task->state & TINYOS_TASK_STATE_SUSPEND) {
		if(--task->suspendCount == 0) {
			task->state &= ~(TINYOS_TASK_STATE_SUSPEND);
			tTaskSchedRdy(task);
			tTaskExitCritical(status);
			tTaskSched();
		}
	}
	
	tTaskExitCritical(status);
}

//删除函数
//回调函数 对结构体删除字段进行赋值
void tTaskSetCleanCallFunc(tTask * task, void (*clean) (void * param), void * param) {
	task->clean = clean;
	task->cleanParam = param;
}
//强制删除
void tTaskForceDelete(tTask * task) {
	uint32_t status = tTaskEnterCritical();
	
	if(task->state & TINYOS_TASK_STATE_DELAYED) {
		tTimeTaskRemove(task);
	}else if(!(task->state & TINYOS_TASK_STATE_SUSPEND)) {
		tTaskSchedRemove(task);
	}
	
	//执行删除函数
	if(task->clean) {
		task->clean(task->cleanParam);
	}
	
	if(curTask == task) {
		tTaskExitCritical(status);
		tTaskSched();
	}
	
	tTaskExitCritical(status);
}

//请求删除的实现
//设置标志位
void tTaskRequestDelete(tTask * task) {
	uint32_t status = tTaskEnterCritical();
	
	task->requestDeleteFlag = 1;
	
	tTaskExitCritical(status);
}

//返回标志位
uint8_t tTaskIsRequestedDeleted(void) {
	uint8_t delete;
	uint32_t status = tTaskEnterCritical();
	
	delete = curTask->requestDeleteFlag;
	
	tTaskExitCritical(status);
	
	return delete;
}
//自己删除
void tTaskDeleteSelf(void) {
	uint32_t status = tTaskEnterCritical();
	
	tTaskSchedRemove(curTask);
	
	if(curTask->clean) {
		curTask->clean(curTask->cleanParam);
	}
	
	tTaskExitCritical(status);
	tTaskSched();
}

//任务查询
void tTaskGetInfo(tTask * task,tTaskInfo * info) {
	uint32_t status = tTaskEnterCritical();
	
	info->delayTicks = task->delayTicks;
	info->prio = task->prio;
	info->slice = task->slice;
	info->state = task->state;
	info->suspendCount = task->suspendCount;
	
	tTaskExitCritical(status);
}


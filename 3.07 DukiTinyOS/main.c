#include "tinyOS.h"
#include "ARMCM3.h"
#include "tLib.h"
#include "tConfig.h"

tTask * curTask;
tTask * nextTask;
tTask * idletask;


tBitMap taskPrioBitMap;//优先级位图
tList taskTable[TINYOS_PRIO_COUNT];//任务表

//调度锁计数器
uint8_t schedLockCounter; //八位，最大255

//延时队列
tList tTaskDelayedList;

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
	tTask->delayState = TINYOS_TASK_STATE_RDY;
	tNodeInit(&(tTask->delayNode)); //延时队列
	tNodeInit(&(tTask->linkNode));  //优先级队列
	tTask->slice = TINYOS_SLICE_MAX; //时间片初始化
  tListAddFirst(&(taskTable[prio]),&(tTask->linkNode));
	tBitMapSet(&taskPrioBitMap, prio);
	
}

//查找最高优先级的就绪任务
tTask * tTaskHighestReady(void) {
	uint32_t highestPrio = tBitMapGetFirstSet(&taskPrioBitMap); //利用优先级位图法找到最高优先级
	tNode *node = tListFirst(&(taskTable[highestPrio]));        //取出对应优先级就绪队列的第一个任务结点
	return tNodeParent(node, tTask, linkNode);                  //利用宏由任务结点反向推出任务结构体指针
}

//定时器初始化函数，定义**毫秒触发一次定时器中断
void tSetSysTickPeriod(uint32_t ms) {
	SysTick->LOAD = ms * SystemCoreClock / 1000 - 1; //SystemCoreClock = 12000000
	NVIC_SetPriority(SysTick_IRQn,(1 << __NVIC_PRIO_BITS) - 1);
	SysTick->VAL = 0;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | 
									SysTick_CTRL_TICKINT_Msk |
									SysTick_CTRL_ENABLE_Msk;
}

void tTaskSched(void) {
	tTask *tmpTask;
	uint32_t status = tTaskEnterCritical();
	if(schedLockCounter > 0) {
		tTaskExitCritical(status);
		return;
	}
	
	tmpTask = tTaskHighestReady();
	if(tmpTask != curTask) {
		nextTask = tmpTask;
		tTaskSwitch();
	}
	
	tTaskExitCritical(status);
}

//延时队列初始化
void tTaskDelayedInit(void) {
	tListInit(&(tTaskDelayedList));
}

//
void tTaskScheLockInit(void) {
	int i;
	uint32_t status = tTaskEnterCritical();
	schedLockCounter = 0;
	for(i = 0;i < TINYOS_PRIO_COUNT;i++) {
		tListInit(&(taskTable[i]));
	}
	tTaskExitCritical(status);
}

//调度失能
void tTaskSchedDisable(void) {
	uint32_t status = tTaskEnterCritical();
	if(schedLockCounter < 255) {
		schedLockCounter++;
	}
	tBitMapInit(&taskPrioBitMap);
	tTaskExitCritical(status);
}

//调度使能
void tTaskSchedEnable(void) {
	uint32_t status = tTaskEnterCritical();
	if(schedLockCounter > 0) {
		if(--schedLockCounter == 0) {
			//减到0，说明没有调度锁了，可以调度任务
			tTaskSched();
		}
	}
	tTaskExitCritical(status);
}

//操作系统延时函数
void tTaskDelay(uint32_t delay) {
	uint32_t status = tTaskEnterCritical();
	//插入延时队列
	tTimeTaskWait(curTask,delay);
	//移出就绪队列
	tTaskSchedUnRdy(curTask);
	tTaskExitCritical(status);
	tTaskSched();
}	

//插入就绪队列
void tTaskSchedRdy(tTask * task) {
	tListAddFirst(&(taskTable[task->prio]),&(task->linkNode));
	tBitMapSet(&taskPrioBitMap,task->prio);
}
//移出就绪队列
void tTaskSchedUnRdy(tTask * task) {
	tListRemove(&(taskTable[task->prio]),&(task->linkNode));
	if(tListCount(&taskTable[task->prio]) == 0) {
		tBitMapClear(&taskPrioBitMap,task->prio);
	}
}

//任务加入延时队列
void tTimeTaskWait(tTask * task,uint32_t ticks) {
	task->delayTicks = ticks;
	tListAddLast(&tTaskDelayedList,&(task->delayNode));
	task->delayState |= TYNYOS_TASK_STATE_DElAYED;
}

//任务移出延时队列
void tTimeTaskWakeUp(tTask * task) {
	tListRemove(&tTaskDelayedList,&(task->delayNode));
	task->delayState &= ~TYNYOS_TASK_STATE_DElAYED;
}


//定时器中断
void SysTick_Handler(void) {
	tTaskSystemTickHandler();
}

void tTaskSystemTickHandler(void) {
	tNode * node;
	uint32_t status = tTaskEnterCritical();
	for(node = tTaskDelayedList.headNode.nextNode;node != &(tTaskDelayedList.headNode);node = node->nextNode) {
		tTask * task = tNodeParent(node,tTask,delayNode);
		if(--task->delayTicks == 0) {
			tTimeTaskWakeUp(task);
			tTaskSchedRdy(task);
		}
	}
	
	if(--curTask->slice == 0) {
		if(tListCount(&(taskTable[curTask->prio])) > 0) {
			tListRemoveFirst(&(taskTable[curTask->prio]));
			tListAddLast(&(taskTable[curTask->prio]),&(curTask->linkNode));
			
			curTask->slice = TINYOS_SLICE_MAX;
		}
	}
	tTaskExitCritical(status);
	tTaskSched();
	
}

tTask Ttask1;
tTask Ttask2;
tTask Ttask3;
tTaskStack task1Stack[1024];
tTaskStack task2Stack[1024];
tTaskStack task3Stack[1024];

//定义空闲任务
tTask TtaskIdle;
tTaskStack taskIdleStack[1024];

void taskIdle(void *param) {
	while(1) {

	}
}
uint32_t shareCount;
//定义任务行为
int task1Flag;
uint32_t pos;

void task1Entry(void *param) {
	tSetSysTickPeriod(10);
	while(1) {
		task1Flag = 1;
		tTaskDelay(1);
		task1Flag = 0;
		tTaskDelay(1);
	}
}

void Delay(void) {
	int i = 0xff;
	while(--i > 0){}
}

int task2Flag;
void task2Entry(void *param) {
	while(1) {	
		task2Flag = 1;
		Delay();
		task2Flag = 0;
		Delay();
	}
}

int task3Flag;
void task3Entry(void *param) {
	while(1) {	
		task3Flag = 1;
		Delay();
		task3Flag = 0;
		Delay();
	}
}

int main() {	
	tTaskScheLockInit();
	tTaskDelayedInit();
	
	tTaskInit(&Ttask1, task1Entry, (void *)0x11111111, 0, &task1Stack[1024]);
	tTaskInit(&Ttask2, task2Entry, (void *)0x22222222, 1, &task2Stack[1024]);
	tTaskInit(&Ttask3, task3Entry, (void *)0x33333333, 1, &task3Stack[1024]);
	tTaskInit(&TtaskIdle, taskIdle, (void *)0, TINYOS_PRIO_COUNT - 1, &taskIdleStack[1024]); //最低优先级
	
	idletask = &TtaskIdle;
	nextTask = tTaskHighestReady();
	
	tTaskRunFirst();
	return 0;
}
#include "tinyOS.h"
#include "ARMCM3.h"
#include "tLib.h"
tTask * curTask;
tTask * nextTask;
tTask * idletask;
tTask * taskTable[2];

//调度锁计数器
uint8_t schedLockCounter; //八位，最大255

//定义任务初始化函数
void tTaskInit1(tTask *tTask,void(*entry)(void *),void * param,tTaskStack * tTaskStack) {
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
}

void tTaskInit2(tTask *tTask,void(*entry)(void *),void * param,tTaskStack * tTaskStack) {
	*(--tTaskStack) = (unsigned long)(1 << 24); //xpsr,第24位置1进入thumb模式
	*(--tTaskStack) = (unsigned long)entry;     //pc寄存器，即R15，设定为程序入口
	*(--tTaskStack) = (unsigned long)0x14+2;      //LR寄存器，即R14
	*(--tTaskStack) = (unsigned long)0x12+2;      //R12寄存器，R13寄存器是SP
	*(--tTaskStack) = (unsigned long)0x3+2;			  //R3
	*(--tTaskStack) = (unsigned long)0x2+2;       //R2
	*(--tTaskStack) = (unsigned long)0x1+2;       //R1
	*(--tTaskStack) = (unsigned long)param;     //参数
	
	*(--tTaskStack) = (unsigned long)0x11+2;      //R11
	*(--tTaskStack) = (unsigned long)0x10+2;      //R10
	*(--tTaskStack) = (unsigned long)0x9+2;       //R9
	*(--tTaskStack) = (unsigned long)0x8+2;       //R8
	*(--tTaskStack) = (unsigned long)0x7+2;       //R7
	*(--tTaskStack) = (unsigned long)0x6+2;       //R6
	*(--tTaskStack) = (unsigned long)0x5+2;       //R5
	*(--tTaskStack) = (unsigned long)0x4+2;       //R4
	tTask->stack = tTaskStack;    
	tTask->delayTicks = 0;
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
	uint32_t status = tTaskEnterCritical();
	if(schedLockCounter > 0) {
		tTaskExitCritical(status);
		return;
	}
	if(curTask == idletask) {
		//如果是空闲任务，则查找列表中delayticks为0的任务调度
		if(taskTable[0]->delayTicks == 0) {
			nextTask = taskTable[0];
		}else if(taskTable[1] == 0) {
			nextTask = taskTable[1];
		}else {
			tTaskExitCritical(status);
			return;
		}
	}else {
		if(curTask == taskTable[0]) {
			if(taskTable[1]->delayTicks == 0) {
				nextTask = taskTable[1];
			}else if(curTask->delayTicks != 0) {
				nextTask = idletask;
			}else {
				tTaskExitCritical(status);
				return;
			}
		}else if(curTask == taskTable[1]) {
			if(taskTable[0]->delayTicks == 0) {
				nextTask = taskTable[0];
			}else if(curTask->delayTicks != 0) {
				nextTask = idletask;
			}else {
				tTaskExitCritical(status);
				return;
			}
		}
	}
	tTaskExitCritical(status);
	tTaskSwitch();
	
}

void tTaskSystemTickHandler(void) {
	uint32_t status = tTaskEnterCritical();
	for(int i = 0;i < 2;i++) {
		if(taskTable[i]->delayTicks > 0) {
			taskTable[i]->delayTicks--;
		}
	}
	tTaskExitCritical(status);
	tTaskSched();
	
}

void tTaskScheLockInit(void) {
	uint32_t status = tTaskEnterCritical();
	schedLockCounter = 0;
	tTaskExitCritical(status);
}

void tTaskSchedDisable(void) {
	uint32_t status = tTaskEnterCritical();
	if(schedLockCounter < 255) {
		schedLockCounter++;
	}
	tTaskExitCritical(status);
}

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

void tTaskDelay(uint32_t delay) {
	uint32_t status = tTaskEnterCritical();
	curTask->delayTicks = delay;
	tTaskExitCritical(status);
	tTaskSched();
	
}	
//定时器中断
void SysTick_Handler(void) {
	tTaskSystemTickHandler();
}

void Delay(int time) {
	while(--time > 0);
}


tTask Ttask1;
tTask Ttask2;

tTaskStack task1Stack[1024];
tTaskStack task2Stack[1024];

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
	tBitMap bitMap;
	tBitMapInit(&bitMap);

	for(int i = tBitMapPosCount() - 1;i >= 0;i--) {
		tBitMapSet(&bitMap,i);
		pos = tBitMapGetFirstSet(&bitMap);
	}
	for(int i = 0;i < tBitMapPosCount();i++) {
		tBitMapClear(&bitMap,i);
		pos = tBitMapGetFirstSet(&bitMap);
	}
	
	while(1) {
		task1Flag = 1;
		tTaskDelay(1);
		task1Flag = 0;
		tTaskDelay(1);
	}
}
int task2Flag;
void task2Entry(void *param) {
	while(1) {	
		task2Flag = 1;
		tTaskDelay(1);
		task2Flag = 0;
		tTaskDelay(1);
	}
}

int main() {	
	tTaskScheLockInit();
	tTaskInit1(&Ttask1,task1Entry,(void *)0x11111111,&task1Stack[1024]);
	tTaskInit2(&Ttask2,task2Entry,(void *)0x22222222,&task2Stack[1024]);
	
	
	taskTable[0] = &Ttask1;
	taskTable[1] = &Ttask2;
	
	nextTask = taskTable[0];
	
	tTaskInit1(&TtaskIdle,taskIdle,(void *)0,&taskIdleStack[1024]);
	idletask = &TtaskIdle;
	
	
	tTaskRunFirst();
	return 0;
}
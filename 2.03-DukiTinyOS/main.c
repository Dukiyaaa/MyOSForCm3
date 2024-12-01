#include "tinyOS.h"
#include "ARMCM3.h"

tTask * curTask;
tTask * nextTask;
tTask * taskTable[2];

//定义任务初始化函数
void tTaskInit(tTask *tTask,void(*entry)(void *),void * param,tTaskStack * tTaskStack) {
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
}



void tTaskSched(void) {
	if(curTask == taskTable[0]) {
		nextTask = taskTable[1];
	}else {
		nextTask = taskTable[0];
	}
	tTaskSwitch();
}

//定时器初始化函数，定义**毫秒触发一次定时器中断
void tSetSysTickPeriod(uint32_t ms) {
	SysTick->LOAD = ms * SystemCoreClock / 1000 - 1;
	NVIC_SetPriority(SysTick_IRQn,(1 << __NVIC_PRIO_BITS) - 1);
	SysTick->VAL = 0;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | 
									SysTick_CTRL_TICKINT_Msk |
									SysTick_CTRL_ENABLE_Msk;
}

//定时器中断
void SysTick_Handler(void) {
	tTaskSched();
}

void Delay(int time) {
	while(--time > 0);
}


tTask Ttask1;
tTask Ttask2;

tTaskStack task1Stack[1024];
tTaskStack task2Stack[1024];


//定义任务行为
int task1Flag;
void task1Entry(void *param) {
	tSetSysTickPeriod(10);
	while(1) {
		task1Flag = 1;
		Delay(100);
		task1Flag = 0;
		Delay(100);
	}
}
int task2Flag;
void task2Entry(void *param) {
	while(1) {
		task2Flag = 1;
		Delay(100);
		task2Flag = 0;
		Delay(100);
	}
}

int main() {	
	tTaskInit(&Ttask1,task1Entry,(void *)0x11111111,&task1Stack[1024]);
	tTaskInit(&Ttask2,task2Entry,(void *)0x22222222,&task2Stack[1024]);
	
	taskTable[0] = &Ttask1;
	taskTable[1] = &Ttask2;
	
	nextTask = taskTable[0];
	
	tTaskRunFirst();
	return 0;
}
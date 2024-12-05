#include "tinyOS.h"

tTask Ttask1;
tTask Ttask2;
tTask Ttask3;
tTask Ttask4;

tTaskStack task1Stack[1024];
tTaskStack task2Stack[1024];
tTaskStack task3Stack[1024];
tTaskStack task4Stack[1024];

tFlagGroup flagGroup;
//定义任务行为
int task1Flag;
void task1Entry (void * param)
{		
	tSetSysTickPeriod(10);	
	tFlagGroupInit(&flagGroup,(uint32_t)0xff);
	for (;;)
	{		
		task1Flag = 0;
		tTaskDelay(1);
		task1Flag = 1;
		tTaskDelay(1);
		
		tFlagGroupNotify(&flagGroup, 0, 0x6);
	}
}

int task2Flag;
void task2Entry (void * param)
{		
	uint32_t resultFlags;
	for (;;)
	{					
		tFlagGroupWait(&flagGroup, TFLAGGROUP_CLEAR_ALL | TFLAGGROUP_CONSUME, 0x4, &resultFlags, 10);
		task2Flag = 0;
		tTaskDelay(1);
		task2Flag = 1;
		tTaskDelay(1);
	}
}

int task3Flag;
void task3Entry (void * param)
{
	for (;;)
	{			
		task3Flag = 0;
		tTaskDelay(1);
		task3Flag = 1;
		tTaskDelay(1);
	}
}
int task4Flag;
void task4Entry (void * param)
{	
	for (;;)
	{				
		task4Flag = 0;
		tTaskDelay(1);
		task4Flag = 1;
		tTaskDelay(1);
	}
}

void tInitApp(void) {
		tTaskInit(&Ttask1, task1Entry, (void *)0x11111111, 0, &task1Stack[1024]);
		tTaskInit(&Ttask2, task2Entry, (void *)0x22222222, 1, &task2Stack[1024]);
		tTaskInit(&Ttask3, task3Entry, (void *)0x33333333, 1, &task3Stack[1024]);
	  tTaskInit(&Ttask4, task4Entry, (void *)0x44444444, 1, &task4Stack[1024]);
}

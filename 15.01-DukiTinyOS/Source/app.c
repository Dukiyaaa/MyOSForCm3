#include "tinyOS.h"
#include "MySerial.h"

tTask Ttask1;
tTask Ttask2;
tTask Ttask3;
tTask Ttask4;

tTaskStack task1Stack[1024];
tTaskStack task2Stack[1024];
tTaskStack task3Stack[1024];
tTaskStack task4Stack[1024];

//定义任务行为
int task1Flag;
void task1Entry (void * param)
{
     for (;;)
    {
        printf("this is task1\n");
        tTaskDelay(10);
    }
}

int task2Flag;
void task2Entry (void * param)
{		
	for (;;)
	{					
		printf("this is task2\n");
		tTaskDelay(10);
	}
}

int task3Flag;
void task3Entry (void * param)
{
	for (;;)
	{			
		printf("this is task3\n");
		tTaskDelay(10);
	}
}
int task4Flag;
void task4Entry (void * param)
{	
	for (;;)
	{				
		printf("this is task4\n");
		tTaskDelay(10);
	}
}

void tInitApp(void) {
		tTaskInit(&Ttask1, task1Entry, (void *)0x11111111, 0, task1Stack,sizeof(task1Stack));
		tTaskInit(&Ttask2, task2Entry, (void *)0x22222222, 1, task2Stack,sizeof(task2Stack));
		tTaskInit(&Ttask3, task3Entry, (void *)0x33333333, 2, task3Stack,sizeof(task3Stack));
	  tTaskInit(&Ttask4, task4Entry, (void *)0x44444444, 3, task4Stack,sizeof(task4Stack));
}

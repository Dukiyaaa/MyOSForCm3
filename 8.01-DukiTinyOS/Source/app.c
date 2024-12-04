#include "tinyOS.h"

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
tMemBlock memBlock1;
uint8_t mem1[20][100];
typedef uint8_t (*tBlock)[100];
//tBlock 作为类型，实际上是一个指向 uint8_t[100] 数组的指针。换句话说，tBlock 可以指向一个包含 100 个 uint8_t 元素的数组

void task1Entry (void * param)
{		
	tSetSysTickPeriod(10);	
	tBlock block[20];
	tMemBlockInit(&memBlock1, (void *)mem1,100,20); //memBlock1有20个内存块，每个块的大小为100 字节（1 uint8_t = 8bit）
	for(int i = 0;i< 20;i++) {
		tMemBlockWait(&memBlock1,(uint8_t **)&block[i],0);
	}
	tTaskDelay(2);
	
	for(uint8_t i = 0;i < 20;i++) {
		memset(block[i], i, 100);//将 block[i] 所指向的内存区域的前 100 个字节设置为值 i。
		tMemBlockNotify(&memBlock1,(uint8_t *)block[i]);
		tTaskDelay(2);
	}
	for (;;)
	{		
		task1Flag = 0;
		tTaskDelay(1);
		task1Flag = 1;
		tTaskDelay(1);
	}
}

int task2Flag;
void task2Entry (void * param)
{		
	for (;;)
	{					
		tBlock block;
		tMemBlockWait(&memBlock1, (uint8_t **)&block,0);
		task2Flag = *(uint8_t*)block;
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

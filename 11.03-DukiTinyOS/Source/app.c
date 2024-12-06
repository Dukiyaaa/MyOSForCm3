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
tTimer timer1;
tTimer timer2;
tTimer timer3;
uint32_t bit1 = 0;
uint32_t bit2 = 0;
uint32_t bit3 = 0;
//定义行为函数
void timerFunc (void * arg)
{
    // 简单的将最低位取反，输出高低翻转的信号
    uint32_t * ptrBit = (uint32_t *)arg;
    *ptrBit ^= 0x1;
}

//timerFunc即为函数指针
void task1Entry (void * param)
{
    uint32_t stopped = 0;

    tSetSysTickPeriod(10);

    // 定时器1：100个tick后启动，以后每10个tick启动一次
    tTimerInit(&timer1, 100, 10, timerFunc, (void *)&bit1, TIMER_CONFIG_TYPE_HARD);
    tTimerStart(&timer1);

    // 定时器2：200个tick后启动，以后每20个tick启动一次
    tTimerInit(&timer2, 200, 20, timerFunc, (void *)&bit2, TIMER_CONFIG_TYPE_HARD);
    tTimerStart(&timer2);

    // 定时器1：300个tick后启动，启动之后关闭
    tTimerInit(&timer3, 300, 0, timerFunc, (void *)&bit3, TIMER_CONFIG_TYPE_HARD);
    tTimerStart(&timer3);
    for (;;)
    {
        task1Flag = 1;
        tTaskDelay(1);
        task1Flag = 0;
        tTaskDelay(1);

        // 200个tick后，手动关闭定时器1
        if (stopped == 0)
        {
            tTaskDelay(200);
            tTimerStop(&timer1);
            stopped = 1;
        }
    }
}

int task2Flag;
void task2Entry (void * param)
{		
	for (;;)
	{					
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

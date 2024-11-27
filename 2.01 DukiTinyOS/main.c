#include "tinyOS.h"
#define NVIC_INT_CTRL   0xE000ED04
#define NVIC_PENDSVSET  0x10000000
#define NVIC_SYSORI2    0xE000ED22
#define NVIC_PENDSV_PRI 0x000000FF

#define MEM32(addr)     *(volatile unsigned long*)(addr)
#define MEM8(addr)     *(volatile unsigned char*)(addr)

void triggerPendSVC(void) {
	MEM8(NVIC_SYSORI2) = NVIC_PENDSV_PRI;
	MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;
}

//定义任务初始化函数
void tTaskInit(tTask *tTask,void(*entry)(void *),void * param,tTaskStack * tTaskStack) {
	tTask->stack = tTaskStack;
}

typedef struct BlockType_t {
	unsigned long * stackPtr;
}BlockType_t;

BlockType_t *blockPtr;

void Delay(int time) {
	while(--time > 0);
}

int flag;
unsigned long stackBuffer[1024];
tTask Ttask1;
tTask Ttask2;

tTaskStack task1Stack[1024];
tTaskStack task2Stack[1024];

//定义任务行为
void task1(void *param) {
	while(1) {
		
	}
}

void task2(void *param) {
	while(1) {
		
	}
}

BlockType_t block;
int main() {
	block.stackPtr = &stackBuffer[1024];
	blockPtr = &block;
	
	tTaskInit(&Ttask1,task1,(void *)0x11111111,&task1Stack[1024]);
	tTaskInit(&Ttask2,task2,(void *)0x22222222,&task2Stack[1024]);
	
	for(;;) {
		flag = 0;
		Delay(200);
		flag = 1;
		Delay(200);
		triggerPendSVC();
	}
	return 0;
}
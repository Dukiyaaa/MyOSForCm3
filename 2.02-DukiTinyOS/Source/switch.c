#include "tinyOS.h"
#include "ARMCM3.h"
#define NVIC_INT_CTRL   0xE000ED04
#define NVIC_PENDSVSET  0x10000000
#define NVIC_SYSORI2    0xE000ED22
#define NVIC_PENDSV_PRI 0x000000FF

#define MEM32(addr)     *(volatile unsigned long*)(addr)
#define MEM8(addr)     *(volatile unsigned char*)(addr)

__asm void PendSV_Handler(void) {
	IMPORT curTask
	IMPORT nextTask
	
	MRS R0,PSP
	CBZ R0,PendSVHandler_nosave
	
	STMDB R0!,{R4-R11}
	
	LDR R1,=curTask
	LDR R1,[R1]
	
	STR R0,[R1]
	
PendSVHandler_nosave
	LDR R0,=curTask
	LDR R1,=nextTask
	LDR R2,[R1]
	STR R2,[R0]     //将curTask更新为nextTask
	
	LDR R0,[R2]     //R2本身已经是curTask，[R2]表示其中的第一个long数据，即堆栈指针
	LDMIA R0!,{R4-R11}
	
	MSR PSP,R0
	ORR LR,LR,#0X04
	BX LR
}

void tTaskRunFirst(void) {
	__set_PSP(0);
	
	MEM8(NVIC_SYSORI2) = NVIC_PENDSV_PRI;
	MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;
}

void tTaskSwitch(void) {
	MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;
}


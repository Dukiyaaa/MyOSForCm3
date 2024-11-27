__asm void PendSV_Handler(void) {
	IMPORT blockPtr
	
	LDR R0,=blockPtr
	LDR R0,[R0]			 //指针指向的结构体
	LDR R0,[R0]      //加载初始四字节内容，刚好是一个long，即结构体中的stackPtr
	
	//寄存器入栈
	STMDB R0!,{R4-R11}
	
	//更新栈指针
	LDR R1,=blockPtr
	LDR R1,[R1]
	STR R0,[R1]     //将存了寄存器的栈指针更新到R1
	
	//杂项操作
	ADD R4,R4,#1
	ADD R5,R5,#1
	
	//出栈
	LDMIA R0!,{R4-R11}
	
	BX LR
	
	
}
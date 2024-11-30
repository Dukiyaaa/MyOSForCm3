#include "tinyOS.h"
#include "ARMCM3.h"

//定时器初始化函数，定义**毫秒触发一次定时器中断
void tSetSysTickPeriod(uint32_t ms) {
	SysTick->LOAD = ms * SystemCoreClock / 1000 - 1; //SystemCoreClock = 12000000
	NVIC_SetPriority(SysTick_IRQn,(1 << __NVIC_PRIO_BITS) - 1);
	SysTick->VAL = 0;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | 
									SysTick_CTRL_TICKINT_Msk |
									SysTick_CTRL_ENABLE_Msk;
}

//定时器中断
void SysTick_Handler(void) {
	tTaskSystemTickHandler();
}

#include "tinyOS.h"
#include "ARMCM3.h"

/**
 * @brief 定时器初始化函数，设置定时器每隔指定毫秒触发一次中断。
 * 
 * 该函数通过配置系统定时器（SysTick），使其在指定的毫秒间隔后产生中断。定时器的触发周期是基于系统时钟频率（SystemCoreClock）和毫秒值计算的。
 * 
 * @param ms  需要设置的定时器周期，单位为毫秒。
 * 
 * @return void
 */
void tSetSysTickPeriod(uint32_t ms) {
    // 计算定时器加载值，确保定时器每隔指定毫秒触发一次中断
    // SystemCoreClock = 12000000 表示系统时钟频率为12 MHz
    // SysTick->LOAD 寄存器控制定时器的计数值，LOAD值 = ms * (SystemCoreClock / 1000) - 1
    SysTick->LOAD = ms * SystemCoreClock / 1000 - 1;

    // 设置SysTick中断的优先级。此处设置为最低优先级
    NVIC_SetPriority(SysTick_IRQn, (1 << __NVIC_PRIO_BITS) - 1);

    // 清除当前SysTick计数器值，确保从0开始计数
    SysTick->VAL = 0;

    // 启用SysTick定时器，并配置为使用系统时钟作为时钟源，同时使能SysTick中断
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |  // 选择系统时钟作为时钟源
                    SysTick_CTRL_TICKINT_Msk |    // 使能SysTick中断
                    SysTick_CTRL_ENABLE_Msk;     // 启用SysTick定时器
}

/**
 * @brief SysTick中断处理函数
 * 
 * 该函数在SysTick定时器中断触发时调用。它会调用系统时钟周期的处理函数，通常用于时间片轮转调度等任务。
 * 
 * @return void
 */
void SysTick_Handler(void) {
    tTaskSystemTickHandler();  // 调用任务调度处理函数，执行时间片轮转等操作
}

#include "Motor.h"
#include "tim.h"
#include "MySerial.h"

/**
 * @brief 初始化电机 PWM 输出
 * 
 * 此函数开启四个通道的 PWM 输出，用于驱动舵机或电机。
 * 在调用此函数前，需确保定时器（TIM3）已通过 HAL 库初始化。
 */
void Motor_Init(void) 
{
    // 开启 TIM3 的 4 个 PWM 通道
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
		Motor_SetPulse(1,0.05);
		HAL_Delay(1000);
		Motor_SetPulse(2,0.05);
		HAL_Delay(1000);
		Motor_SetPulse(3,0.05);
		HAL_Delay(1000);
		Motor_SetPulse(4,0.05);
		HAL_Delay(1000);
}

/**
 * @brief 设置指定通道的 PWM 占空比
 * 
 * @param channel PWM 通道号（1 ~ 4）
 * @param Pulse 占空比百分比（0.0 ~ 1.0），表示 PWM 高电平所占比例。
 *              - 0.0：完全低电平
 *              - 1.0：完全高电平
 *              - 其他值：高低电平按比例分配
 * 
 * 该函数会将占空比转换为定时器比较寄存器的值。
 */
void Motor_SetPulse(int channel, float Pulse)
{
    // 确保占空比在有效范围内（0.0 ~ 1.0）
    if (Pulse < 0.0f) Pulse = 0.0f;
    if (Pulse > 1.0f) Pulse = 1.0f;

    // 根据占空比计算计数值
    int duty = (int)(ARR_VAL * Pulse);  // ARR_VAL 是自动重装载值

    // 根据通道号设置对应的比较值
    switch(channel) {
        case 1: 
            __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_1, duty);
            break;
        case 2: 
            __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_2, duty);
            break;
        case 3: 
            __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_3, duty);
            break;
        case 4: 
            __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_4, duty);
            break;
        default:
            // 无效通道号，忽略设置
            break;
    }
}

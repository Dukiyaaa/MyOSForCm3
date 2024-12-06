#ifndef __MOTOR_H
#define __MOTOR_H
#include "main.h"
#define ARR_VAL 10000 // 定义自动重装载值（ARR）
void Motor_Init(void);// 初始化电机 PWM 模块
void Motor_SetPulse(int channel, float Pulse);// 设置 PWM 占空比

#endif

#ifndef ATTITUDE_SOLVER_H
#define ATTITUDE_SOLVER_H

#include <stdint.h>

//Yaw值线性回归矫正
#define K -0.00023825
#define B -0.00448912

// 输出的姿态角（全局变量）
extern volatile float Roll, Pitch, Yaw;

// 初始化函数
void AttitudeSolver_Init(float sample_frequency, float gain);

// 使用加速度计和陀螺仪进行更新
void AttitudeSolver_UpdateIMU(float gx, float gy, float gz, float ax, float ay, float az);

// 使用加速度计、陀螺仪和磁力计进行更新
void AttitudeSolver_Update(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz);

// 获取姿态角
void AttitudeSolver_GetEulerAngles(float *roll, float *pitch, float *yaw, int *count);

#endif // ATTITUDE_SOLVER_H

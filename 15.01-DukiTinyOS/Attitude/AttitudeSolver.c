#include "AttitudeSolver.h"
#include "MadgWick.h"
#include <math.h>

#define DEG_TO_RAD (3.14159265358979323846 / 180.0)
#define RAD_TO_DEG (180.0 / 3.14159265358979323846)
// 输出的姿态角（全局变量）


// 初始化函数
void AttitudeSolver_Init(float sample_frequency, float gain) {
    beta = gain;                 // 设置 Madgwick 算法增益
    q0 = 1.0f; q1 = 0.0f;        // 初始化四元数
    q2 = 0.0f; q3 = 0.0f;
}

// 使用加速度计和陀螺仪更新姿态
void AttitudeSolver_UpdateIMU(float gx, float gy, float gz, float ax, float ay, float az) {
    MadgwickAHRSupdateIMU(gx, gy, gz, ax, ay, az);
}

// 使用加速度计、陀螺仪和磁力计更新姿态
void AttitudeSolver_Update(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz) {
    MadgwickAHRSupdate(gx, gy, gz, ax, ay, az, mx, my, mz);
}

// 获取姿态角（欧拉角形式,单位为rad）
void AttitudeSolver_GetEulerAngles(float *roll, float *pitch, float *yaw, int *count) {
    *roll = atan2f(2.0f * (q0 * q1 + q2 * q3), 1.0f - 2.0f * (q1 * q1 + q2 * q2)) * RAD_TO_DEG;  // 转换为角度
    *pitch = asinf(2.0f * (q0 * q2 - q3 * q1)) * RAD_TO_DEG;
    *yaw = atan2f(2.0f * (q0 * q3 + q1 * q2), 1.0f - 2.0f * (q2 * q2 + q3 * q3)) * RAD_TO_DEG;
		*yaw = *yaw - (K * (*count) + B); // 线性回归矫正
	  *count = *count + 1;
	  
	  //转换回来
//		*roll *= DEG_TO_RAD;
//	  *pitch *= DEG_TO_RAD;
//	  *yaw *= DEG_TO_RAD;
}

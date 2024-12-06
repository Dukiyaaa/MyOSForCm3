#ifndef ANONYMITY_H
#define ANONYMITY_H

#include <stdint.h> // 包含标准整数类型
#include "main.h"
// 上位机通信函数声明
void SendToAno03(float Roll, float Pitch, float Yaw); // 发送姿态角数据
void SendToAno01(float ax, float ay, float az, float gx, float gy, float gz); // 发送六个轴数据
void SendToAno04(float q0,float q1,float q2,float q3); // 发送姿态四元数数据

#endif // ANONYMITY_H

//
// Created by 邓可 on 2024/10/16.
//

#include "MyMadgWick.h"
#include "stdio.h"
#include "MPU6050.h"
//定义全局变量用于存储上个时刻的姿态四元数
quaternion q_est_pre = {1.0f, 0.0f, 0.0f, 0.0f};  // 初始化为单位四元数
//定义全局变量用于存储当前时刻的姿态四元数
quaternion q_est_now = {1.0f, 0.0f, 0.0f, 0.0f};  // 初始化为单位四元数
quaternion q;
//extern float Roll, Pitch, Yaw;
//************************** 四元数部分 **************************

// 四元数加法
void quaternion_add(quaternion *q1, quaternion *q2, quaternion *q_res) {
    q_res->w = q1->w + q2->w;
    q_res->x = q1->x + q2->x;
    q_res->y = q1->y + q2->y;
    q_res->z = q1->z + q2->z;
}

// 四元数减法
void quaternion_sub(quaternion *q1, quaternion *q2, quaternion *q_res) {
    q_res->w = q1->w - q2->w;
    q_res->x = q1->x - q2->x;
    q_res->y = q1->y - q2->y;
    q_res->z = q1->z - q2->z;
}

// 四元数数乘
void quaternion_scalar(quaternion *q1, float num, quaternion *q_res) {
    q_res->w = q1->w * num;
    q_res->x = q1->x * num;
    q_res->y = q1->y * num;
    q_res->z = q1->z * num;
}

// 四元数乘法
void quaternion_mul(quaternion *q1, quaternion *q2, quaternion *q_res) {
    q_res->w = q1->w * q2->w - q1->x * q2->x - q1->y * q2->y - q1->z * q2->z;
    q_res->x = q1->w * q2->x + q1->x * q2->w + q1->y * q2->z - q1->z * q2->y;
    q_res->y = q1->w * q2->y - q1->x * q2->z + q1->y * q2->w + q1->z * q2->x;
    q_res->z = q1->w * q2->z + q1->x * q2->y - q1->y * q2->x + q1->z * q2->w;
}

// 四元数取模
float quaternion_norm(quaternion *q) {
    return sqrt(q->w * q->w + q->x * q->x + q->y * q->y + q->z * q->z);
}

// 快速求平方根倒数（Fast Inverse Square Root）
float fast_inverse_sqrt(float number) {
    const float threehalfs = 1.5F;

    union {
        float f;
        uint32_t i;
    } conv = { number };

    conv.i = 0x5f3759df - (conv.i >> 1); // 魔数常量
    conv.f *= (threehalfs - (number * 0.5F * conv.f * conv.f)); // 牛顿迭代一次
    return conv.f;
}

// 四元数归一化
// 取模可以参考 快速求平方根倒数算法
// 优化的四元数归一化
void quaternion_normalization(quaternion *q) {
    float norm_squared = q->w * q->w + q->x * q->x + q->y * q->y + q->z * q->z;
    if (norm_squared > 0.0f) {  // 防止除以0
        float inv_sqrt_norm = fast_inverse_sqrt(norm_squared);
        q->w *= inv_sqrt_norm;
        q->x *= inv_sqrt_norm;
        q->y *= inv_sqrt_norm;
        q->z *= inv_sqrt_norm;
    }
}
//*********************** 雅可比矩阵、误差函数、梯度计算 **********************

// 雅可比矩阵
void jacobi(float J[3][4], struct quaternion *q) {
    J[0][0] = -2.0f * (q->y);
    J[0][1] =  2.0f * (q->z);
    J[0][2] = -2.0f * (q->w);
    J[0][3] =  2.0f * (q->x);

    J[1][0] = 2.0f * (q->x);
    J[1][1] = 2.0f * (q->w);
    J[1][2] = 2.0f * (q->z);
    J[1][3] = 2.0f * (q->y);

    J[2][0] = 0.0f;
    J[2][1] = -4.0f * (q->x);
    J[2][2] = -4.0f * (q->y);
    J[2][3] = 0.0f;
}

// 误差函数
void Func(float F[3], struct quaternion *q, quaternion *q_acc) {
    F[0] = 2.0f * (q->x * q->z - q->w * q->y) - q_acc->x;
    F[1] = 2.0f * (q->y * q->z + q->w * q->x) - q_acc->y;
    F[2] = 2.0f * (0.5f - q->x * q->x - q->y * q->y) - q_acc->z;
}

// 求出梯度
void gra(quaternion *gradient, float J[3][4], float F[3]) {
    gradient->w = J[0][0] * F[0] + J[1][0] * F[1] + J[2][0] * F[2];
    gradient->x = J[0][1] * F[0] + J[1][1] * F[1] + J[2][1] * F[2];
    gradient->y = J[0][2] * F[0] + J[1][2] * F[1] + J[2][2] * F[2];
    gradient->z = J[0][3] * F[0] + J[1][3] * F[1] + J[2][3] * F[2];
}

//***************************** 融合函数 ****************************

void merge(float ax, float ay, float az, float wx, float wy, float wz) {
//		for (int i = 0; i < 500000; i++) { __asm__("nop"); } // 代替 HAL_Delay(10) 的延时
    q_est_pre = q_est_now;

    // 构造陀螺仪角速度四元数
    quaternion q_ang = {0, wx, wy, wz};
    // 构造加速度计加速度四元数
    quaternion q_acc = {0, ax, ay, az};
    // 加速度归一化
    quaternion_normalization(&q_acc);

    // 求出姿态变化率 0.5 * q(t-1) * w
    quaternion tmp1;
    quaternion_scalar(&q_est_pre, 0.5f, &tmp1);
    quaternion q_w_gra;
    quaternion_mul(&tmp1, &q_ang, &q_w_gra);
    // 求出雅可比矩阵
    float J[3][4];
    jacobi(J, &q_est_pre);

    // 求出误差函数
    float F[3];
    Func(F, &q_est_pre, &q_acc);

    // 求出梯度
    quaternion gradient;
    gra(&gradient, J, F);

    // 梯度归一化
    quaternion_normalization(&gradient);

    // q(t) = q(t-1) + q' * deltaT - B * 梯度归一化
    quaternion_scalar(&q_w_gra, deltaT, &tmp1);
    quaternion tmp2;
    quaternion_scalar(&gradient, B, &tmp2);
    quaternion_add(&q_est_pre, &tmp1, &q_est_now);
    quaternion_sub(&q_est_now, &tmp2, &q_est_now);
		
		q = q_est_now;
}


#include <math.h>

/**
 * @brief 将四元数转换为欧拉角
 * @param q 四元数结构体（输入）
 * @param roll 指向存储欧拉角Roll的指针（输出）
 * @param pitch 指向存储欧拉角Pitch的指针（输出）
 * @param yaw 指向存储欧拉角Yaw的指针（输出）
 */
void quaternion_to_euler(quaternion *q, float *roll, float *pitch, float *yaw) {
    // 提取四元数分量
    float qw = q->w;
    float qx = q->x;
    float qy = q->y;
    float qz = q->z;

    // 计算欧拉角（单位：弧度）
    *roll = atan2f(2.0f * (qw * qx + qy * qz), 1.0f - 2.0f * (qx * qx + qy * qy));  // Roll
    *pitch = asinf(2.0f * (qw * qy - qz * qx));                                     // Pitch
    *yaw = atan2f(2.0f * (qw * qz + qx * qy), 1.0f - 2.0f * (qy * qy + qz * qz));   // Yaw

    // 转换为角度（如果需要以度为单位）
    *roll *= (180.0f / PI);
    *pitch *= (180.0f / PI);
    *yaw *= (180.0f / PI);
}

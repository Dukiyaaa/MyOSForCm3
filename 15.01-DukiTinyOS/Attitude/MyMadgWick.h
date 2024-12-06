//
// Created by 邓可 on 2024/10/16.
//

#ifndef __MYMADGWICK_H
#define __MYMADGWICK_H
//
// Created by 邓可 on 2024/10/16.
//
#include "main.h"
#include <math.h>

//************************** 四元数部分 **************************
//四元数定义
typedef struct quaternion {
    float w;
    float x;
    float y;
    float z;
}quaternion;
// 声明全局变量
extern quaternion q_est_pre;
extern quaternion q_est_now;

//定义全局变量B和deltaT
#define PI 3.1415926535897932
//#define B sqrt(3.0f/4.0f) * 0.5f / 180 * PI //查手册发现噪声为0.05度每秒
#define B 0.01 //查手册发现噪声为0.05度每秒//隔一段时间roll和yaw就会加2.6
//Bq取0.2没有大偏移
#define deltaT 0.08f

void quaternion_add(quaternion *q1, quaternion *q2, quaternion *q_res) ;

void quaternion_sub(quaternion *q1, quaternion *q2, quaternion *q_res) ;

void quaternion_scalar(quaternion *q1, float num, quaternion *q_res) ;

void quaternion_mul(quaternion *q1, quaternion *q2, quaternion *q_res) ;

float quaternion_norm(quaternion *q) ;

float fast_inverse_sqrt(float number);

void quaternion_normalization(quaternion *q) ;

void jacobi(float J[3][4], struct quaternion *q) ;

void Func(float F[3], struct quaternion *q, quaternion *q_acc) ;

void gra(quaternion *gradient, float J[3][4], float F[3]);

void merge(float ax, float ay, float az, float wx, float wy, float wz);

void quaternion_to_euler(quaternion *q, float *roll, float *pitch, float *yaw);
#endif //UNTITLED_MADGWICK_H

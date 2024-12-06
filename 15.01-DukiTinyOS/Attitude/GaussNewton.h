#ifndef __GAUSSNEWTON_H
#define __GAUSSNEWTON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// 定义常量
#define M 6                          // 数据组数
#define g 1.0                        // 加速度标准值
#define ThreshodValue 0.000001       // 迭代停止的阈值

// 数据矩阵定义
extern double V[M][3];               // 实际输出
extern double K[3][3];               // 比例误差矩阵
extern double A[M][3];               // 理想输出
extern double B[3];                  // 零漂误差
extern double P[6];                  // 参数矩阵
extern double e[M];                  // 误差矩阵

// 函数声明
void printMatrix(double matrix[M][M], int n);  // 打印矩阵
void computeResiduals(double V[M][3], double e[M], double P[6]); // 计算残差
void computeJacobian(double A[M][3], double J[M][M], double P[M]); // 计算雅可比矩阵
int inverseMatrix(double matrix[M][M], double inverse[M][M], int n); // 计算矩阵逆
void gaussNewtonCalibration();  // 高斯牛顿校准主函数

#endif

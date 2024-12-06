#ifndef __MPU6050_H
#define __MPU6050_H

#include "main.h"
//倍率说明
#define DEG_TO_RAD (3.14159265358979323846 / 180.0)
#define RAD_TO_DEG (180.0 / 3.14159265358979323846)
#define ADC_16 65536
#define ACC_Range 4
#define GRO_Range 4000

//高斯牛顿迭代矫正参数
#define P0      1.001004
#define P1      1.006896
#define P2      0.987984
#define P3      0.038779
#define P4      -0.012867
#define P5      0.509085

//陀螺仪偏置矫正
#define GX_OFFSET -0.05
#define GY_OFFSET -0.01
#define GZ_OFFSET 0.03

//陀螺仪采样循环次数
#define CYCLE_COUNT 10


// MPU6050 I2C地址
#define MPU6050_I2C_ADDR      0x68  // 默认地址（7位地址），实际写操作时需要左移1位

// MPU6050 寄存器地址
#define MPU6050_SMPLRT_DIV    0x19
#define MPU6050_CONFIG        0x1A
#define MPU6050_GYRO_CONFIG   0x1B
#define MPU6050_ACCEL_CONFIG  0x1C

#define MPU6050_ACCEL_XOUT_H  0x3B
#define MPU6050_ACCEL_XOUT_L  0x3C
#define MPU6050_ACCEL_YOUT_H  0x3D
#define MPU6050_ACCEL_YOUT_L  0x3E
#define MPU6050_ACCEL_ZOUT_H  0x3F
#define MPU6050_ACCEL_ZOUT_L  0x40
#define MPU6050_TEMP_OUT_H    0x41
#define MPU6050_TEMP_OUT_L    0x42
#define MPU6050_GYRO_XOUT_H   0x43
#define MPU6050_GYRO_XOUT_L   0x44
#define MPU6050_GYRO_YOUT_H   0x45
#define MPU6050_GYRO_YOUT_L   0x46
#define MPU6050_GYRO_ZOUT_H   0x47
#define MPU6050_GYRO_ZOUT_L   0x48

#define MPU6050_PWR_MGMT_1    0x6B
#define MPU6050_PWR_MGMT_2    0x6C
#define MPU6050_WHO_AM_I      0x75

// MPU6050功能函数声明
uint8_t MPU6050_Init(void);                         // 初始化MPU6050
uint8_t MPU6050_WriteReg(uint8_t reg, uint8_t data); // 写寄存器
uint8_t MPU6050_ReadReg(uint8_t reg, uint8_t *data); // 读寄存器
uint8_t MPU6050_ReadData(uint8_t reg, uint8_t *buf, uint16_t len); // 批量读取数据
void MPU6050_GetAccelData(float *ax, float *ay, float *az);  // 读取加速度
void MPU6050_GetGyroData(float *gx, float *gy, float *gz);   // 读取陀螺仪
void MPU6050_GetGyroAveData(float *gx, float *gy, float *gz); //读取取均值的陀螺仪示数
void MPU6050_GetTemp(float *temp);                   // 读取温度

#endif

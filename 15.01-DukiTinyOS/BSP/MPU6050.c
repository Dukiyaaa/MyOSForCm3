#include "mpu6050.h"
#include "MyIIC.h" // 引入软件I2C库

/**
 * @brief 初始化MPU6050
 * @retval 0: 成功, 1: 失败
 */
uint8_t MPU6050_Init(void)
{
    I2C_Init(); // 初始化I2C
    if (MPU6050_WriteReg(MPU6050_PWR_MGMT_1, 0x00) != 0) // 解除休眠模式
        return 1;
    if (MPU6050_WriteReg(MPU6050_SMPLRT_DIV, 0x07) != 0) // 设置采样率
        return 1;
    if (MPU6050_WriteReg(MPU6050_CONFIG, 0x06) != 0)     // 配置低通滤波器
        return 1;
    if (MPU6050_WriteReg(MPU6050_GYRO_CONFIG, 0x18) != 0) // 配置陀螺仪量程 ±2000°/s
        return 1;
    if (MPU6050_WriteReg(MPU6050_ACCEL_CONFIG, 0x01) != 0) // 配置加速度计量程 ±4g
        return 1;
    return 0;
}

/**
 * @brief 向MPU6050寄存器写数据
 * @param reg 寄存器地址
 * @param data 写入的数据
 * @retval 0: 成功, 1: 失败
 */
uint8_t MPU6050_WriteReg(uint8_t reg, uint8_t data)
{
    return I2C_WriteReg(MPU6050_I2C_ADDR, reg, data);
}

/**
 * @brief 从MPU6050寄存器读取数据
 * @param reg 寄存器地址
 * @param data 读取到的数据
 * @retval 0: 成功, 1: 失败
 */
uint8_t MPU6050_ReadReg(uint8_t reg, uint8_t *data)
{
    return I2C_ReadReg(MPU6050_I2C_ADDR, reg, data);
}

/**
 * @brief 从MPU6050批量读取数据
 * @param reg 起始寄存器地址
 * @param buf 存储读取数据的缓冲区
 * @param len 读取数据的长度
 * @retval 0: 成功, 1: 失败
 */
uint8_t MPU6050_ReadData(uint8_t reg, uint8_t *buf, uint16_t len)
{
    I2C_Start();
    I2C_SendByte((MPU6050_I2C_ADDR << 1) | 0); // 发送设备地址+写指令
    if (I2C_WaitAck() != 0)
    {
        I2C_Stop();
        return 1;
    }

    I2C_SendByte(reg); // 发送起始寄存器地址
    if (I2C_WaitAck() != 0)
    {
        I2C_Stop();
        return 1;
    }

    I2C_Start();
    I2C_SendByte((MPU6050_I2C_ADDR << 1) | 1); // 发送设备地址+读指令
    if (I2C_WaitAck() != 0)
    {
        I2C_Stop();
        return 1;
    }

    for (uint16_t i = 0; i < len; i++)
    {
        buf[i] = I2C_ReceiveByte();
        if (i == (len - 1))
            I2C_SendNotAck(); // 最后一个字节发送NACK
        else
            I2C_SendAck();
    }
    I2C_Stop();
    return 0;
}

/**
 * @brief 获取并处理加速度数据，将原始ADC值转换为工程单位(g)
 * @param ax 指向X轴加速度数据的指针（浮点型，单位g）
 * @param ay 指向Y轴加速度数据的指针（浮点型，单位g）
 * @param az 指向Z轴加速度数据的指针（浮点型，单位g）
 */
void MPU6050_GetAccelData(float *ax, float *ay, float *az)
{
    uint8_t buf[6];
    int16_t raw_ax, raw_ay, raw_az;

    // 读取6字节的原始加速度数据
    MPU6050_ReadData(MPU6050_ACCEL_XOUT_H, buf, 6);

    // 合并高低字节为int16_t类型的原始数据
    raw_ax = (int16_t)((buf[0] << 8) | buf[1]);
    raw_ay = (int16_t)((buf[2] << 8) | buf[3]);
    raw_az = (int16_t)((buf[4] << 8) | buf[5]);

    // 转换为浮点数并存储到输出变量
    *ax = (float)raw_ax * ACC_Range / ADC_16;
    *ay = (float)raw_ay * ACC_Range / ADC_16;
    *az = (float)raw_az * ACC_Range / ADC_16;
	
		// 使用高斯牛顿计算得到的校准参数
    *ax = (*ax - P3) * P0;  // X轴校准：减去零偏误差，乘以比例误差
    *ay = (*ay - P4) * P1;  // Y轴校准
    *az = (*az - P5) * P2;  // Z轴校准
}

/**
 * @brief 获取并处理陀螺仪数据，将原始ADC值转换为工程单位(°/s)
 * @param gx 指向X轴角速度数据的指针（浮点型，单位°/s）
 * @param gy 指向Y轴角速度数据的指针（浮点型，单位°/s）
 * @param gz 指向Z轴角速度数据的指针（浮点型，单位°/s）
 */
void MPU6050_GetGyroData(float *gx, float *gy, float *gz)
{
    uint8_t buf[6];
    int16_t raw_gx, raw_gy, raw_gz;

    // 读取6字节的原始陀螺仪数据
    MPU6050_ReadData(MPU6050_GYRO_XOUT_H, buf, 6);

    // 合并高低字节为int16_t类型的原始数据
    raw_gx = (int16_t)((buf[0] << 8) | buf[1]);
    raw_gy = (int16_t)((buf[2] << 8) | buf[3]);
    raw_gz = (int16_t)((buf[4] << 8) | buf[5]);

    // 转换为浮点数并存储到输出变量
    *gx = (float)raw_gx * GRO_Range / ADC_16 * DEG_TO_RAD;
    *gy = (float)raw_gy * GRO_Range / ADC_16 * DEG_TO_RAD;
    *gz = (float)raw_gz * GRO_Range / ADC_16 * DEG_TO_RAD;
	
	  // 消去零偏误差
		*gx = (*gx) - GX_OFFSET;
		*gy = (*gy) - GY_OFFSET;
		*gz = (*gz) - GZ_OFFSET;
}

void MPU6050_GetGyroAveData(float *gx, float *gy, float *gz) {
		float tmp_gx, tmp_gy, tmp_gz;
		for(int i = 0;i < CYCLE_COUNT;i++) {
			MPU6050_GetGyroData(&tmp_gx, &tmp_gy, &tmp_gz);
			*gx += tmp_gx;
			*gy += tmp_gy;
			*gz += tmp_gz;
		}
		
		*gx = (*gx) / CYCLE_COUNT;
		*gy = (*gy) / CYCLE_COUNT;
		*gz = (*gz) / CYCLE_COUNT;
}
	
/**
 * @brief 获取温度数据
 * @param temp 存储温度值的指针（单位：摄氏度）
 */
void MPU6050_GetTemp(float *temp)
{
    uint8_t buf[2];
    MPU6050_ReadData(MPU6050_TEMP_OUT_H, buf, 2);
    int16_t raw_temp = (int16_t)((buf[0] << 8) | buf[1]);
    *temp = raw_temp / 340.0 + 36.53; // 将原始温度值转换为摄氏度
}

#ifndef __MYIIC_H
#define __MYIIC_H
#include "main.h"

// 用户配置部分（I2C引脚定义）
#define I2C_SCL_PIN        GPIO_PIN_4      // SCL: PC4
#define I2C_SDA_PIN        GPIO_PIN_5      // SDA: PC5
#define I2C_GPIO_PORT      GPIOC           // I2C引脚所在的端口
#define I2C_DELAY_TIME     5               // 软件I2C延时（单位：微秒）

// I2C操作函数声明
void I2C_Init(void);                       // 初始化I2C GPIO
void I2C_Start(void);                      // 发送起始信号
void I2C_Stop(void);                       // 发送停止信号
uint8_t I2C_WaitAck(void);                 // 等待从机应答信号
void I2C_SendAck(void);                    // 发送应答信号
void I2C_SendNotAck(void);                 // 发送非应答信号
void I2C_SendByte(uint8_t byte);           // 发送1字节数据
uint8_t I2C_ReceiveByte(void);             // 接收1字节数据
uint8_t I2C_ReadReg(uint8_t devAddr, uint8_t regAddr, uint8_t *data);//从指定设备地址的寄存器读取一个字节的数据
uint8_t I2C_WriteReg(uint8_t devAddr, uint8_t regAddr, uint8_t data);//向指定设备地址的寄存器写入一个字节的数据
#endif
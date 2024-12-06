#include "MyIIC.h"
#include "stm32f4xx_hal.h"

/**
 * @brief 延时函数，用于模拟I2C时序
 * @param time 延时的时间（单位：微秒）
 */
static void I2C_Delay(uint16_t time)
{
    for (uint32_t i = 0; i < time * 10; ++i)
        __NOP();
}

/**
 * @brief 初始化I2C的GPIO引脚
 * @param 无
 */
void I2C_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // 初始化SCL和SDA引脚为开漏输出模式
    GPIO_InitStruct.Pin = I2C_SCL_PIN | I2C_SDA_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD; // 开漏输出
    GPIO_InitStruct.Pull = GPIO_PULLUP;        // 上拉模式
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH; // 高速
    HAL_GPIO_Init(I2C_GPIO_PORT, &GPIO_InitStruct);

    // 默认将SCL和SDA置为高电平
    HAL_GPIO_WritePin(I2C_GPIO_PORT, I2C_SCL_PIN | I2C_SDA_PIN, GPIO_PIN_SET);
}

/**
 * @brief 设置SCL引脚电平
 * @param val 电平值（1：高，0：低）
 */
static void I2C_SetSCL(uint8_t val)
{
    if (val)
        HAL_GPIO_WritePin(I2C_GPIO_PORT, I2C_SCL_PIN, GPIO_PIN_SET);
    else
        HAL_GPIO_WritePin(I2C_GPIO_PORT, I2C_SCL_PIN, GPIO_PIN_RESET);
}

/**
 * @brief 设置SDA引脚电平
 * @param val 电平值（1：高，0：低）
 */
static void I2C_SetSDA(uint8_t val)
{
    if (val)
        HAL_GPIO_WritePin(I2C_GPIO_PORT, I2C_SDA_PIN, GPIO_PIN_SET);
    else
        HAL_GPIO_WritePin(I2C_GPIO_PORT, I2C_SDA_PIN, GPIO_PIN_RESET);
}

/**
 * @brief 读取SDA引脚电平
 * @return GPIO_PIN_SET 或 GPIO_PIN_RESET 0应答，1非应答
 */
static uint8_t I2C_ReadSDA(void)
{
    return HAL_GPIO_ReadPin(I2C_GPIO_PORT, I2C_SDA_PIN);
}

/**
 * @brief 发送I2C起始信号,在 SCL 高电平时，将 SDA 从低电平拉高。
 */
void I2C_Start(void)
{
		//要做到通信开始和结束后，SCL和SDA都拉高，中间则二者都低
    I2C_SetSDA(1);
    I2C_SetSCL(1);
    I2C_Delay(I2C_DELAY_TIME);
    I2C_SetSDA(0);
    I2C_Delay(I2C_DELAY_TIME);
    I2C_SetSCL(0);
    I2C_Delay(I2C_DELAY_TIME);
}

/**
 * @brief 发送I2C停止信号
 */
void I2C_Stop(void)
{
    I2C_SetSDA(0);
    I2C_SetSCL(1);
    I2C_Delay(I2C_DELAY_TIME);
    I2C_SetSDA(1);
    I2C_Delay(I2C_DELAY_TIME);
}

/**
 * @brief 等待从机应答信号
 * @return 1：未收到应答，0：收到应答
 */
uint8_t I2C_WaitAck(void)
{
    uint16_t timeout = 100;

    I2C_SetSDA(1); // 释放SDA
    I2C_SetSCL(1); // 拉高SCL
    I2C_Delay(I2C_DELAY_TIME);

    while (I2C_ReadSDA())
    {
        if (--timeout == 0)
        {
            I2C_Stop();
            return 1; // 超时未收到应答
        }
    }

    I2C_SetSCL(0);
    I2C_Delay(I2C_DELAY_TIME);
    return 0; // 收到应答
}

/**
 * @brief 发送应答信号
 */
void I2C_SendAck(void)
{
    I2C_SetSDA(0);//主机发送0，表示应答
    I2C_Delay(I2C_DELAY_TIME);
    I2C_SetSCL(1);//从机此时读取应答
    I2C_Delay(I2C_DELAY_TIME);
    I2C_SetSCL(0);//读取完毕
    I2C_Delay(I2C_DELAY_TIME);
}

/**
 * @brief 发送非应答信号
 */
void I2C_SendNotAck(void)
{
    I2C_SetSDA(1);//主机发送1，表示非应答
    I2C_Delay(I2C_DELAY_TIME);
    I2C_SetSCL(1);//从机此时读取应答
    I2C_Delay(I2C_DELAY_TIME);
    I2C_SetSCL(0);//读取完毕
    I2C_Delay(I2C_DELAY_TIME);
}

/**
 * @brief 发送1字节数据
 * @param byte 要发送的数据
 */
void I2C_SendByte(uint8_t byte)
{
    for (uint8_t i = 0; i < 8; ++i)
    {
        I2C_SetSCL(0);
        I2C_Delay(I2C_DELAY_TIME);
        I2C_SetSDA(byte & 0x80);
        byte <<= 1;
        I2C_Delay(I2C_DELAY_TIME);
        I2C_SetSCL(1);
        I2C_Delay(I2C_DELAY_TIME);
    }
    I2C_SetSCL(0);
}

/**
 * @brief 接收1字节数据
 * @return 接收到的数据
 */
uint8_t I2C_ReceiveByte(void)
{
    uint8_t byte = 0;

    I2C_SetSDA(1); // 释放SDA
    for (uint8_t i = 0; i < 8; ++i)
    {
        byte <<= 1;
        I2C_SetSCL(0);
        I2C_Delay(I2C_DELAY_TIME);
        I2C_SetSCL(1);
        I2C_Delay(I2C_DELAY_TIME);
        if (I2C_ReadSDA())
            byte |= 0x01;
    }
    I2C_SetSCL(0);
    return byte;
}

/**
 * @brief 从指定设备地址的寄存器读取一个字节的数据
 * @param devAddr I2C设备地址（7位地址，不包含读写位）
 * @param regAddr 寄存器地址
 * @param data 存储读取数据的变量指针
 * @retval 0: 成功, 1: 失败
 */
uint8_t I2C_ReadReg(uint8_t devAddr, uint8_t regAddr, uint8_t *data)
{
    I2C_Start();                              // 发送起始信号
    I2C_SendByte((devAddr << 1) | 0);         // 发送设备地址 + 写操作
    if (I2C_WaitAck() != 0)                   // 等待应答
    {
        I2C_Stop();                           // 停止信号
        return 1;                             // 操作失败
    }

    I2C_SendByte(regAddr);                    // 发送寄存器地址
    if (I2C_WaitAck() != 0)                   // 等待应答
    {
        I2C_Stop();                           // 停止信号
        return 1;                             // 操作失败
    }

    I2C_Start();                              // 重新启动信号
    I2C_SendByte((devAddr << 1) | 1);         // 发送设备地址 + 读操作
    if (I2C_WaitAck() != 0)                   // 等待应答
    {
        I2C_Stop();                           // 停止信号
        return 1;                             // 操作失败
    }

    *data = I2C_ReceiveByte();                // 接收一个字节数据
    I2C_SendNotAck();                         // 发送非应答信号
    I2C_Stop();                               // 发送停止信号

    return 0;                                 // 操作成功
}

/**
 * @brief 向指定设备地址的寄存器写入一个字节的数据
 * @param devAddr I2C设备地址（7位地址，不包含读写位）
 * @param regAddr 寄存器地址
 * @param data 写入的数据
 * @retval 0: 成功, 1: 失败
 */
uint8_t I2C_WriteReg(uint8_t devAddr, uint8_t regAddr, uint8_t data)
{
    I2C_Start();                              // 发送起始信号
    I2C_SendByte((devAddr << 1) | 0);         // 发送设备地址 + 写操作
    if (I2C_WaitAck() != 0)                   // 等待应答
    {
        I2C_Stop();                           // 停止信号
        return 1;                             // 操作失败
    }

    I2C_SendByte(regAddr);                    // 发送寄存器地址
    if (I2C_WaitAck() != 0)                   // 等待应答
    {
        I2C_Stop();                           // 停止信号
        return 1;                             // 操作失败
    }

    I2C_SendByte(data);                       // 发送数据
    if (I2C_WaitAck() != 0)                   // 等待应答
    {
        I2C_Stop();                           // 停止信号
        return 1;                             // 操作失败
    }

    I2C_Stop();                               // 发送停止信号
    return 0;                                 // 操作成功
}

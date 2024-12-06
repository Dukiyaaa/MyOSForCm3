#include "MySerial.h"
#include "usart.h"

// 定义一个数组用于存储接收到的数据
char data[30] = {0};
// 单字节数据存储
uint8_t dat;
// 数据指针，指向当前写入位置
uint pointer;

/**
 * @brief 串口初始化函数
 * 
 * 初始化串口接收中断，使能接收数据。
 * 
 * @param 无
 * @return 无
 */
void MySerial_Init(void)
{
    // 初始化UART1的接收中断，每次接收一个字节数据
    HAL_UART_Receive_IT(&huart1, &dat, 1);
}

/**
 * @brief UART接收中断回调函数
 * 
 * 在串口接收到数据后调用，处理接收到的字节数据。
 * 
 * @param huart 串口句柄指针
 * @return 无
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    // 将接收到的数据存储到data数组中，并移动指针
    data[pointer++] = dat;
    // 再次启动UART接收中断以接收后续数据
    HAL_UART_Receive_IT(&huart1, &dat, 1);	
}

/**
 * @brief 重定向标准输出函数
 * 
 * 用于实现printf函数的重定向，将标准输出数据通过串口发送。
 * 
 * @param ch  要发送的字符
 * @param f   文件指针(这里无实际作用)
 * @return 返回发送的字符
 */
int fputc(int ch, FILE *f)
{
    // 通过UART1发送数据，等待时间为最大
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xfff);
    return ch;
}

/**
 * @brief 串口接收数据处理函数
 * 
 * 检查是否有数据接收完成并稳定。如果接收完成调用数据处理函数。
 * 
 * @param 无
 * @return 无
 */
void MySerial_ReceiveData(void)
{
    // 检查指针是否有变化，判断是否接收到新数据
    if(pointer != 0) {
        int tmp = pointer;
        // 延迟1ms，等待数据接收稳定
        HAL_Delay(1);
        if(tmp == pointer) { // 如果延迟后指针未变，说明数据已完全接收
            UART_RX_PROC(); // 调用数据处理函数
        }
    }
}

/**
 * @brief 数据处理函数
 * 
 * 处理接收到的数据并通过串口打印到标准输出。
 * 
 * @param 无
 * @return 无
 */
void UART_RX_PROC(void)
{
    // 打印接收到的数据，并换行
    printf("%s\n", data);
}

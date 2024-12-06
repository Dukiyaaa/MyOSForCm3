#ifndef __MYSERIAL_H
#define __MYSERIAL_H
#include "main.h"
#include "stdio.h"

void MySerial_Init(void); // 初始化串口
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart); // 串口接收中断回调处理
int fputc(int ch, FILE *f); // 标准输出字符到串口
void MySerial_ReceiveData(void); // 接收串口数据
void UART_RX_PROC(void); // 串口接收数据处理逻辑
#endif

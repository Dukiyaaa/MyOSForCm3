#ifndef __RECEIVER_H
#define __RECEIVER_H

#include "main.h"

// 宏定义
#define CHANNEL_COUNT 4          // 通道数量
#define CHANNEL1_INDEX 0         // 通道 1 索引
#define CHANNEL2_INDEX 1         // 通道 2 索引
#define CHANNEL3_INDEX 2         // 通道 3 索引
#define CHANNEL4_INDEX 3         // 通道 4 索引
#define INVALID_CHANNEL 255      // 无效通道标志

#define MAX_MOTORVAL14 1750        // 14通道最大脉宽值
#define MIN_MOTORVAL14 1250        // 14通道最小脉宽值
#define MAX_MOTORVAL3 2000        // 3最大脉宽值
#define MIN_MOTORVAL3 1000        // 3最小脉宽值
#define MAX_MOTORVAL2 1750        // 2最大脉宽值
#define MIN_MOTORVAL2 1500        // 2最小脉宽值

#define SUB_MOTORVAL14 500        // 14通道脉宽范围的
#define SUB_MOTORVAL3 1000        // 3通道脉宽范围
#define SUB_MOTORVAL2 250        // 2通道脉宽范围

void Receiver_Init(void);        //初始化函数
float Receiver_GetMappedValue(uint32_t channelIndex); //返回映射值，方便外层调用
#endif // __RECEIVER_H

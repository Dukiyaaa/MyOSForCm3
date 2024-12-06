#ifndef __TTIMER_H
#define __TTIMER_H

#include "tEvent.h"
typedef enum _tTimerState {
	tTimerCreated,
	tTimerStarted,
	tTimerRunning,
	tTimerStopped,
	tTimerDestroyed,
}tTimerState;

typedef struct _tTimer
{
    // 链表结点
    tNode linkNode;

    // 初次启动延后的ticks数
    uint32_t startDelayTicks;

    // 周期定时时的周期tick数
    uint32_t durationTicks;

    // 当前定时递减计数值
    uint32_t delayTicks;

    // 定时回调函数
    void (*timerFunc) (void * arg);//
    //void (*timerFunc)(void *arg) 是一个 函数指针，
	  //表示 timerFunc 是一个指向函数的指针，函数的返回类型是 void，并且它接受一个 void * 类型的参数。
	
    // 传递给回调函数的参数
    void * arg;

    // 定时器配置参数
    uint32_t config;

    // 定时器状态
    tTimerState state;
}tTimer;

// 软硬定时器
#define TIMER_CONFIG_TYPE_HARD          (1 << 0)
#define TIMER_CONFIG_TYPE_SOFT          (0 << 0)

void tTimerInit (tTimer * timer, uint32_t delayTicks, uint32_t durationTicks,
                 void (*timerFunc) (void * arg), void * arg, uint32_t config);
#endif


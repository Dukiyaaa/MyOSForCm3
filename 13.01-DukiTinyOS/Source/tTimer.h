#ifndef TTIMER_H
#define	TTIMER_H

#include "tEvent.h"

typedef enum _tTimerState
{
	tTimerCreated,
	tTimerStarted,
	tTimerRunning,
	tTimerStopped,
	tTimerDestroyed
}tTimerState;

typedef struct _tTimer
{
	tNode linkNode;
	uint32_t startDelayTicks;
	uint32_t durationTicks;
	uint32_t delayTicks;
	void (*timerFunc) (void * arg);
	void * arg;
	uint32_t config;
	
	tTimerState state;
}tTimer;

// 软定时器状态信息
typedef struct _tTimerInfo
{
    // 初次启动延后的ticks数
    uint32_t startDelayTicks;

    // 周期定时时的周期tick数
    uint32_t durationTicks;

    // 定时回调函数
    void (*timerFunc) (void * arg);

    // 传递给回调函数的参数
    void * arg;

    // 定时器配置参数
    uint32_t config;

    // 定时器状态
    tTimerState state;
}tTimerInfo;

#define	TIMER_CONFIG_TYPE_HARD		(1 << 0)
#define	TIMER_CONFIG_TYPE_SOFT		(0 << 0)

void tTimerInit (tTimer * timer, uint32_t delayTicks, uint32_t durationTicks,
		void (*timerFunc) (void * arg), void * arg, uint32_t config);
void tTimerStart (tTimer * timer);
void tTimerStop (tTimer * timer);
void tTimerModuleTickNotify (void);
void tTimerModuleInit (void);
void tTimerDestroy (tTimer * timer);
void tTimerGetInfo (tTimer * timer, tTimerInfo * info);
void tTimerInitTask(void);
#endif

#ifndef TTIMER_H
#define TTIMER_H

#include "tEvent.h"

// 定义定时器的状态枚举类型
typedef enum _tTimerState
{
    tTimerCreated,    // 定时器已创建
    tTimerStarted,    // 定时器已启动
    tTimerRunning,    // 定时器正在运行
    tTimerStopped,    // 定时器已停止
    tTimerDestroyed   // 定时器已销毁
} tTimerState;

// 定义定时器结构体
typedef struct _tTimer
{
    tNode linkNode;       // 链表节点，用于将定时器添加到链表中
    uint32_t startDelayTicks; // 初始延迟的tick数
    uint32_t durationTicks;   // 定时器周期（定时器持续的tick数）
    uint32_t delayTicks;      // 当前延迟的tick数
    void (*timerFunc) (void * arg); // 定时器到期时的回调函数
    void * arg;               // 传递给回调函数的参数
    uint32_t config;          // 定时器配置，指定定时器类型（硬件或软件）
    tTimerState state;        // 定时器的当前状态
} tTimer;

// 软定时器状态信息结构体
typedef struct _tTimerInfo
{
    uint32_t startDelayTicks;   // 初次启动延迟的tick数
    uint32_t durationTicks;     // 周期定时器的周期tick数
    void (*timerFunc) (void * arg); // 定时回调函数
    void * arg;                 // 传递给回调函数的参数
    uint32_t config;            // 定时器配置参数
    tTimerState state;          // 定时器状态
} tTimerInfo;

// 定时器配置选项，定义硬件和软件定时器类型
#define TIMER_CONFIG_TYPE_HARD    (1 << 0)  // 硬件定时器
#define TIMER_CONFIG_TYPE_SOFT    (0 << 0)  // 软件定时器

// tTimerInit（timer：指向定时器对象的指针，delayTicks：初始延迟的tick数，durationTicks：定时器的周期tick数，timerFunc：定时器到期时调用的回调函数，arg：传递给回调函数的参数，config：定时器的配置选项）
// 初始化定时器对象
void tTimerInit (tTimer * timer, uint32_t delayTicks, uint32_t durationTicks,
        void (*timerFunc) (void * arg), void * arg, uint32_t config);

// tTimerStart（timer：指向定时器对象的指针）
// 启动定时器
void tTimerStart (tTimer * timer);

// tTimerStop（timer：指向定时器对象的指针）
// 停止定时器
void tTimerStop (tTimer * timer);

// tTimerModuleTickNotify（无参数）
// 每个系统滴答时调用，处理定时器事件
void tTimerModuleTickNotify (void);

// tTimerModuleInit（无参数）
// 初始化定时器模块，设置相关资源
void tTimerModuleInit (void);

// tTimerDestroy（timer：指向定时器对象的指针）
// 销毁定时器对象，停止定时器并释放资源
void tTimerDestroy (tTimer * timer);

// tTimerGetInfo（timer：指向定时器对象的指针，info：指向定时器信息结构体的指针）
// 获取定时器的相关信息
void tTimerGetInfo (tTimer * timer, tTimerInfo * info);

#endif // TTIMER_H

#include "tinyOS.h"

// 定义硬件定时器和软件定时器的链表
static tList tTimerHardList;   // 硬件定时器链表
static tList tTimerSoftList;   // 软件定时器链表

// 定义用于保护定时器操作的信号量
static tSem tTimerProtectSem;  // 定时器保护信号量
static tSem tTimerTickSem;     // 定时器滴答信号量

/**
 * @brief 初始化定时器对象
 * 
 * @param timer 指向定时器对象的指针
 * @param delayTicks 定时器启动前的初始延迟（以滴答为单位）
 * @param durationTicks 定时器的持续时间（以滴答为单位）
 * @param timerFunc 定时器到期时调用的回调函数
 * @param arg 传递给回调函数的参数
 * @param config 定时器的配置选项（例如，硬件或软件定时器类型）
 * 
 * 该函数使用提供的参数初始化定时器对象，设置定时器的初始状态，并确定其延迟和持续时间。
 */
void tTimerInit (tTimer * timer, uint32_t delayTicks, uint32_t durationTicks,
                void (*timerFunc) (void * arg), void * arg, uint32_t config)
{
    tNodeInit(&timer->linkNode);  // 初始化定时器的链表节点
    timer->startDelayTicks = delayTicks;    // 设置初始延迟
    timer->durationTicks = durationTicks;    // 设置持续时间
    timer->timerFunc = timerFunc;            // 设置回调函数
    timer->arg = arg;                        // 设置回调函数的参数
    timer->config = config;                  // 设置定时器配置
    
    if (delayTicks == 0)
    {
        timer->delayTicks = durationTicks;    // 如果无初始延迟，延迟设置为持续时间
    }
    else
    {
        timer->delayTicks = timer->startDelayTicks;  // 设置延迟为指定的初始延迟
    }
    
    timer->state = tTimerCreated;  // 设置定时器状态为已创建
}

/**
 * @brief 启动定时器
 * 
 * @param timer 指向定时器对象的指针
 * 
 * 该函数启动定时器，根据配置将其添加到硬件或软件定时器链表中，并处理定时器的不同状态。
 */
void tTimerStart (tTimer * timer)
{
    switch (timer->state)
    {
        case tTimerCreated:  // 定时器处于已创建状态
        case tTimerStopped:  // 定时器处于已停止状态
            // 设置延迟滴答数，如果有初始延迟则使用，否则使用持续时间
            timer->delayTicks = timer->startDelayTicks ? timer->startDelayTicks : timer->durationTicks;
            timer->state = tTimerStarted;  // 更改状态为已启动
        
            if (timer->config & TIMER_CONFIG_TYPE_HARD)  // 判断是否为硬件定时器
            {
                uint32_t status = tTaskEnterCritical();  // 进入临界区，保护共享资源
                tListAddFirst(&tTimerHardList, &timer->linkNode);  // 将定时器添加到硬件定时器链表的头部
                tTaskExitCritical(status);  // 退出临界区
            }
            else  // 软件定时器
            {
                tSemWait(&tTimerProtectSem, 0);  // 等待定时器保护信号量
                tListAddLast(&tTimerSoftList, &timer->linkNode);  // 将定时器添加到软件定时器链表的尾部
                tSemNotify(&tTimerProtectSem);  // 通知释放定时器保护信号量
            }
            break;
        default:
            // 其他状态下不执行任何操作
            break;
    }
}

/**
 * @brief 停止定时器
 * 
 * @param timer 指向定时器对象的指针
 * 
 * 该函数停止定时器，根据配置将其从硬件或软件定时器链表中移除，并处理定时器的不同状态。
 */
void tTimerStop (tTimer * timer)
{
    switch (timer->state)
    {
        case tTimerStarted:   // 定时器处于已启动状态
        case tTimerRunning:   // 定时器处于运行中状态
            if (timer->config & TIMER_CONFIG_TYPE_HARD)  // 判断是否为硬件定时器
            {
                uint32_t status = tTaskEnterCritical();  // 进入临界区，保护共享资源
                tListRemove(&tTimerHardList, &timer->linkNode);  // 从硬件定时器链表中移除定时器
                tTaskExitCritical(status);  // 退出临界区
            }
            else  // 软件定时器
            {
                tSemWait(&tTimerProtectSem, 0);  // 等待定时器保护信号量
                tListRemove(&tTimerSoftList, &timer->linkNode);  // 从软件定时器链表中移除定时器
                tSemNotify(&tTimerProtectSem);  // 通知释放定时器保护信号量
            }
            break;
        default:
            // 其他状态下不执行任何操作
            break;
    }
}

/**
 * @brief 调用定时器链表中的回调函数
 * 
 * @param timerList 指向定时器链表的指针
 * 
 * 该函数遍历定时器链表，检查每个定时器的延迟滴答数是否到期。如果到期，调用定时器的回调函数，
 * 并根据定时器的持续时间决定是否重新设置延迟或将定时器移除链表。
 */
static void tTimerCallFuncList (tList * timerList)
{
    tNode * node;
    for (node = timerList->headNode.nextNode; node != &(timerList->headNode); node = node->nextNode)
    {
        tTimer * timer = tNodeParent(node, tTimer, linkNode);  // 获取定时器对象
        if ((timer->delayTicks == 0) || (--timer->delayTicks == 0))  // 检查延迟是否到期
        {
            timer->state = tTimerRunning;  // 设置状态为运行中
            timer->timerFunc(timer->arg);  // 调用回调函数
            timer->state = tTimerStarted;  // 恢复状态为已启动
            
            if (timer->durationTicks > 0)
            {
                timer->delayTicks = timer->durationTicks;  // 重新设置延迟滴答数
            }
            else
            {
                tListRemove(timerList, &timer->linkNode);  // 从链表中移除定时器
                timer->state = tTimerStopped;  // 设置状态为已停止
            }
        }
    }
}

// 定义定时器任务及其栈空间
static tTask tTimeTask;  // 定时器任务
static tTaskStack tTimerTaskStack[TINYOS_TIMERTASK_STACK_SIZE];  // 定时器任务栈

/**
 * @brief 软件定时器任务函数
 * 
 * @param param 传递给任务的参数（未使用）
 * 
 * 该任务循环等待定时器滴答信号量，然后调用软件定时器链表中的回调函数。
 */
static void tTimerSoftTask (void * param)
{
    for (;;)
    {
        tSemWait(&tTimerTickSem, 0);  // 等待定时器滴答信号量
        
        tSemWait(&tTimerProtectSem, 0);  // 等待定时器保护信号量
        
        tTimerCallFuncList(&tTimerSoftList);  // 调用软件定时器链表中的回调函数
        
        tSemNotify(&tTimerProtectSem);  // 通知释放定时器保护信号量
    }
}

/**
 * @brief 定时器模块滴答通知函数
 * 
 * 该函数在每个系统滴答时调用，处理硬件定时器，并通知软件定时器任务进行处理。
 */
void tTimerModuleTickNotify (void)
{
    uint32_t status = tTaskEnterCritical();  // 进入临界区，保护共享资源
    
    tTimerCallFuncList(&tTimerHardList);  // 调用硬件定时器链表中的回调函数
    
    tTaskExitCritical(status);  // 退出临界区
    
    tSemNotify(&tTimerTickSem);  // 通知软件定时器任务
}

/**
 * @brief 初始化定时器模块
 * 
 * 该函数初始化定时器模块，包括初始化硬件和软件定时器链表、
 * 定时器保护信号量、定时器滴答信号量，并初始化定时器任务。
 */
void tTimerModuleInit (void)
{
    tListInit(&tTimerHardList);  // 初始化硬件定时器链表
    tListInit(&tTimerSoftList);  // 初始化软件定时器链表
    tSemInit(&tTimerProtectSem, 1, 1);  // 初始化定时器保护信号量，初始值为1
    tSemInit(&tTimerTickSem, 0, 0);     // 初始化定时器滴答信号量，初始值为0
    
#if TINYOS_TIMERTASK_PRIO >= (TINYOS_PRIO_COUNT - 1)
    #error "The priority of timer task must be greater than (TINYOS_PRIO_COUNT - 1)"
#endif
    // 初始化定时器任务，设置任务函数、参数、优先级及栈空间
    tTaskInit(&tTimeTask, tTimerSoftTask, (void *)0, TINYOS_TIMERTASK_PRIO, &tTimerTaskStack[TINYOS_TIMERTASK_STACK_SIZE]);
}

/**
 * @brief 销毁定时器
 * 
 * @param timer 指向定时器对象的指针
 * 
 * 该函数停止定时器并将其状态设置为已销毁。
 */
void tTimerDestroy (tTimer * timer)
{
    tTimerStop(timer);           // 停止定时器
    timer->state = tTimerDestroyed;  // 设置状态为已销毁
}

/**
 * @brief 获取定时器信息
 * 
 * @param timer 指向定时器对象的指针
 * @param info 指向定时器信息结构体的指针
 * 
 * 该函数获取定时器的相关信息，包括初始延迟、持续时间、回调函数、参数、配置选项及当前状态。
 */
void tTimerGetInfo (tTimer * timer, tTimerInfo * info)
{
    uint32_t status = tTaskEnterCritical();  // 进入临界区，保护共享资源

    // 获取定时器相关信息
    info->startDelayTicks = timer->startDelayTicks;      // 获取初始延迟
    info->durationTicks = timer->durationTicks;          // 获取持续时间
    info->timerFunc = timer->timerFunc;                  // 获取回调函数
    info->arg = timer->arg;                              // 获取回调函数的参数
    info->config = timer->config;                        // 获取定时器配置
    info->state = timer->state;                          // 获取定时器当前状态

    tTaskExitCritical(status);  // 退出临界区
}

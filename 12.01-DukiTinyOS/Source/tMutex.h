#ifndef __TMUTEX_H
#define __TMUTEX_H
#include "tEvent.h"

// 互斥量结构体，包含事件控制块、锁定计数器、拥有者及优先级信息
typedef struct _tMutex {
    tEvent event;  // 事件控制块
    uint32_t lockedCount;  // 锁定计数器
    tTask *owner;  // 当前互斥量拥有者
    uint32_t ownerOriginalPrio;  // 拥有者原始优先级
} tMutex;

// 互斥量信息结构体，包含任务数量、拥有者信息等
typedef struct _tMutexInfo {
    uint32_t taskCount;  // 等待的任务数量
    uint32_t ownerPrio;  // 拥有者优先级
    uint32_t inheritedPrio;  // 拥有者继承优先级
    tTask *owner;  // 当前互斥量拥有者任务
    uint32_t lockedCount;  // 锁定次数
} tMutexInfo;

/**
 * tMutexInit（mutex：互斥量指针） 
 * 初始化互斥量，设置事件、锁定计数器、拥有者等信息
 */
void tMutexInit(tMutex *mutex);

/**
 * tMutexWait（mutex：互斥量指针，waitTicks：等待时间） 
 * 请求互斥量，处理等待和优先级继承
 * 
 * @return 成功返回 tErrorNoError，失败返回 tErrorResourceUnavaliable
 */
uint32_t tMutexWait(tMutex *mutex, uint32_t waitTicks);

/**
 * tMutexNoWaitGet（mutex：互斥量指针） 
 * 请求互斥量，若已被锁定则立即返回失败
 * 
 * @return 成功返回 tErrorNoError，失败返回 tErrorResourceUnavaliable
 */
uint32_t tMutexNoWaitGet(tMutex *mutex);

/**
 * tMutexNotify（mutex：互斥量指针） 
 * 释放互斥量，恢复优先级并唤醒等待任务
 * 
 * @return 成功返回 tErrorNoError，失败返回 tErrorOwner（非法释放）
 */
uint32_t tMutexNotify(tMutex *mutex);

/**
 * tMutexGetInfo（mutex：互斥量指针，info：互斥量信息指针） 
 * 获取互斥量的相关信息，如任务数、优先级等
 */
void tMutexGetInfo(tMutex *mutex, tMutexInfo *info);

/**
 * tMutexDestroy（mutex：互斥量指针） 
 * 销毁互斥量，恢复优先级并清空等待队列
 * 
 * @return 返回清除的等待任务数
 */
uint32_t tMutexDestroy(tMutex *mutex);

#endif

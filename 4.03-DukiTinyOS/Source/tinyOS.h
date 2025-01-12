#ifndef __TINYOS_H
#define __TINYOS_H
#include <stdint.h>
#include "tLib.h"
#include "tconfig.h"
typedef  uint32_t tTaskStack;

#define TINYOS_TASK_STATE_RDY      0
#define TINYOS_TASK_STATE_DESTORYED  (1 << 1) //删除状态
#define TINYOS_TASK_STATE_DELAYED    (1 << 2)
#define TINYOS_TASK_STATE_SUSPEND    (1 << 3) //挂起状态

//任务结构体定义
typedef struct _tTask {
	tTaskStack* stack;
	uint32_t delayTicks;//添加软定时器
	uint32_t prio;//添加优先级字段
	tNode delayNode; //为了方便加入延时队列
	uint32_t state;//指示状态
	tNode linkNode;
	uint32_t slice;//时间片
	uint32_t suspendCount;//挂起计数器
	
	//删除部分字段
	void (*clean) (void * param);
	void * cleanParam;
	uint8_t requestDeleteFlag;
}tTask;

typedef struct _tTaskInfo {
	uint32_t delayTicks;
	uint32_t prio;
	uint32_t state;
	uint32_t slice;
	uint32_t suspendCount;
}tTaskInfo;

extern tTask * curTask;
extern tTask * nextTask;

void tTaskSwitch(void);
void tTaskRunFirst(void);
void tTaskInit(tTask *tTask, void(*entry)(void *), void * param, uint32_t prio, tTaskStack * tTaskStack);

uint32_t tTaskEnterCritical(void);
void tTaskExitCritical(uint32_t status);

void tTaskScheLockInit(void);
void tTaskSchedDisable(void);
void tTaskSchedEnable(void);
void tTaskSched(void);
void tTaskSchedRdy(tTask * task);
void tTaskSchedUnRdy(tTask * task);

void tTimeTaskWait(tTask * task,uint32_t ticks);
void tTimeTaskWakeUp(tTask * task);
void tTaskSystemTickHandler(void);

void tInitApp(void);

void tSetSysTickPeriod(uint32_t ms);

void tTaskDelay(uint32_t delay);

//挂起函数
void tTaskSuspend(tTask * task) ;
void tTaskWakeUp(tTask * task) ;

//两个队列的移出函数
void tTaskSchedRemove(tTask * task);
void tTimeTaskRemove(tTask * task);

//删除函数
void tTaskSetCleanCallFunc(tTask * task, void (*clean) (void * param), void * param);
void tTaskForceDelete(tTask * task);
void tTaskRequestDelete(tTask * task);
uint8_t tTaskIsRequestedDeleted(void);
void tTaskDeleteSelf(void);

//任务状态查询
void tTaskGetInfo(tTask * task,tTaskInfo * info);
#endif


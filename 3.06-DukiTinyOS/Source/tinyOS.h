#ifndef __TINYOS_H
#define __TINYOS_H
#include <stdint.h>
#include "tLib.h"
typedef  uint32_t tTaskStack;

#define TINYOS_TASK_STATE_RDY      0
#define TYNYOS_TASK_STATE_DElAYED  (1 << 1)

typedef struct _tTask {
	tTaskStack* stack;
	uint32_t delayTicks;//添加软定时器
	uint32_t prio;//添加优先级字段
	tNode delayNode; //为了方便加入延时队列
	uint32_t delayState;//指示延时状态
}tTask;

extern tTask * curTask;
extern tTask * nextTask;

void tTaskSwitch(void);
void tTaskRunFirst(void);

uint32_t tTaskEnterCritical(void);
void tTaskExitCritical(uint32_t status);

void tTaskScheLockInit(void);
void tTaskSchedDisable(void);
void tTaskSchedEnable(void);
void tTaskSched(void);
#endif
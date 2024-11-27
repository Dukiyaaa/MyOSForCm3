#ifndef __TINYOS_H
#define __TINYOS_H
#include <stdint.h>

typedef  uint32_t tTaskStack;

typedef struct _tTask {
	tTaskStack* stack;
	uint32_t delayTicks;//添加软定时器
	uint32_t prio;//添加优先级字段
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
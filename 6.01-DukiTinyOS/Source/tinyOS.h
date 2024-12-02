#ifndef __TINYOS_H
#define __TINYOS_H
#include <stdint.h>
#include "tLib.h"
#include "tconfig.h"
#include "tEvent.h"
#include "tTask.h"
#include "tSem.h"
//错误码
typedef enum _tError {
	tErrorNoError = 0,
	tErrorTimeOut = 1,
}tError;

extern tTask * curTask;
extern tTask * nextTask;

uint32_t tTaskEnterCritical(void);
void tTaskExitCritical(uint32_t status);

void tTaskSwitch(void);
void tTaskRunFirst(void);

void tTaskScheLockInit(void);
void tTaskSchedDisable(void);
void tTaskSchedEnable(void);
void tTaskSched(void);

void tTaskSchedRdy(tTask * task);
void tTaskSchedUnRdy(tTask * task);
void tTaskSchedRemove(tTask * task);

void tTimeTaskWait(tTask * task,uint32_t ticks);
void tTimeTaskWakeUp(tTask * task);
void tTimeTaskRemove(tTask * task);

void tTaskSystemTickHandler(void);
void tTaskDelay(uint32_t delay);
void tSetSysTickPeriod(uint32_t ms);

void tInitApp(void);

#endif


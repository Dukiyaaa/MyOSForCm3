#ifndef __TINYOS_H
#define __TINYOS_H
#include <stdint.h>
#include "tLib.h"
#include "tconfig.h"
#include "tEvent.h"
#include "tTask.h"
#include "tSem.h"
#include "tMbox.h"
#include "tMemBlock.h"
#include "tFlagGroup.h"
#include "tMutex.h"
#include "tTimer.h"
#include "tHooks.h"
#define TICKS_PER_SEC (1000 / TINYOS_SYSTICK_MS)
//错误码
typedef enum _tError {
	tErrorNoError = 0,
	tErrorTimeOut,
	tErrorResourceUnavaliable,
	tErrorDel,
	tErrorResourceFull,
	tErrorOwner,
}tError;

extern tTask * curTask;
extern tTask * nextTask;

uint32_t tTaskEnterCritical(void);
void tTaskExitCritical(uint32_t status);

tTask * tTaskHighestReady(void);
void tTaskDelayedInit(void);

void tTaskSwitch(void);
void tTaskRunFirst(void);

void tTaskSchedInit(void);
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

//返回CPU利用率
float tCpuUsageGet(void);

//启动函数
void tTinyOSInit(void);
void tTinyOSStart(void);
#endif


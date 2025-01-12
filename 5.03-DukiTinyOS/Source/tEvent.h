#ifndef __TEVENT_H
#define __TEVENT_H
#include "tLib.h"
#include "tTask.h"

//事件类型
typedef enum _tEventType {
	tEventTypeUnknow,
}tEventType;
//事件结构体的定义
typedef struct _tEvent {
	tEventType type;
	tList waitList;
}tEvent;

void tEventInit(tEvent * event,tEventType type);
void tEventWait(tEvent * event, tTask * task, void * msg, uint32_t state, uint32_t timeout);
tTask * tEventWakeUp(tEvent * event, void * msg, uint32_t result);
void  tEventRemoveTask(tTask * task, void * msg, uint32_t result);
uint32_t tEventRemoveAll(tEvent * event, void * msg, uint32_t result);
uint32_t tEventWaitCount(tEvent * event);
#endif





#ifndef __TEVENT_H
#define __TEVENT_H
#include "tLib.h"

typedef enum _tEventType {
	tEventTypeUnknow,
}tEventType;

typedef struct _tEvent {
	tEventType type;
	tList waitList;
}tEvent;

void tEventInit(tEvent * event,tEventType type);
#endif





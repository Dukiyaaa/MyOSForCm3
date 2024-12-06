#include "tinyOS.h"

void tMutexInit(tMutex * mutex) {
	tEventInit(&mutex->event,tEventTypeMutex);
	mutex->lockedCount = (uint32_t)0;
	mutex->owner = (tTask *)0;
	mutex->ownerOriginalPrio = TINYOS_PRIO_COUNT;//初始化为最低
}

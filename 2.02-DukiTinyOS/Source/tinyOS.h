#ifndef __TINYOS_H
#define __TINYOS_H
#include <stdint.h>

typedef  uint32_t tTaskStack;

typedef struct _tTask {
	tTaskStack* stack;
}tTask;

extern tTask * curTask;
extern tTask * nextTask;

void tTaskSwitch(void);
void tTaskRunFirst(void);
#endif
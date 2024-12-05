#ifndef __TFLAGGROUP_H
#define __TFLAGGROUP_H

#include "tEvent.h"

typedef struct _tFlagGroup {
	tEvent event;
	uint32_t flags;
}tFlagGroup;

void tFlagGroupInit(tFlagGroup * flagGroup, uint32_t flags);
#endif

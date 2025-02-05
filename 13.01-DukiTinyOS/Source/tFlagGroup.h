#ifndef __TFLAGGROUP_H
#define __TFLAGGROUP_H

#include "tEvent.h"

typedef struct _tFlagGroup {
	tEvent event;
	uint32_t flags;
}tFlagGroup;

typedef struct _tFlagGroupInfo {
	
	uint32_t flags;
	
	uint32_t taskCount;
}tFlagGroupInfo;

//设定标志位处理宏
#define TFLAGGROUP_CLEAR            (0x0 << 0)
#define TFLAGGROUP_SET              (0x1 << 0)
#define TFLAGGROUP_ANY              (0x0 << 1)
#define TFLAGGROUP_ALL              (0x1 << 1)

#define TFLAGGROUP_SET_ALL          (TFLAGGROUP_SET | TFLAGGROUP_ALL)    //全部位置1
#define TFLAGGROUP_SET_ANY          (TFLAGGROUP_SET | TFLAGGROUP_ANY)    //任一位置1
#define TFLAGGROUP_CLEAR_ALL        (TFLAGGROUP_CLEAR | TFLAGGROUP_ALL)  //全部位清0
#define TFLAGGROUP_CLEAR_ANY        (TFLAGGROUP_CLEAR | TFLAGGROUP_ANY)  //任一位清0

#define TFLAGGROUP_CONSUME          (0x1 << 7)

void tFlagGroupInit(tFlagGroup * flagGroup, uint32_t flags);

uint32_t tFlagGroupWait(tFlagGroup * flagGroup, uint32_t waitType, uint32_t requestFlag, uint32_t *resultFlag, uint32_t waitTicks);

uint32_t tFlagGroupNoWaitGet(tFlagGroup * flagGroup, uint32_t waitType, uint32_t requestFlag, uint32_t *resultFlag, uint32_t waitTicks);

void tFlagGroupNotify(tFlagGroup * flagGroup, uint8_t isSet, uint32_t flag);

uint32_t tFlagGroupDestory(tFlagGroup * flagGroup);

void tFlagGroupGetInfo (tFlagGroup * flagGroup, tFlagGroupInfo * info);
#endif

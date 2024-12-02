#ifndef __TMBOX_H
#define __TMBOX_H

#include "tEvent.h"

typedef struct _tMbox {
	tEvent event;
	uint32_t count;
	uint32_t read;
	uint32_t write;
	uint32_t maxCount;
	void ** msgBuffer;//消息缓冲队列
}tMbox;

void tMboxInit(tMbox * mbox,void ** msgBuffer,uint32_t maxCount);
#endif
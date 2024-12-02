#include "tinyOS.h"

void tMboxInit(tMbox * mbox,void ** msgBuffer,uint32_t maxCount) {
	tEventInit(&mbox->event,tEventTypeMbox);//初始化事件为邮箱类型
	
	mbox->msgBuffer = msgBuffer;
	mbox->maxCount = maxCount;
	mbox->count = 0;
	mbox->read = 0;
	mbox->write = 0;
}
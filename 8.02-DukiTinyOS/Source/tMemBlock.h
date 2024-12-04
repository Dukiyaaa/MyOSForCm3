#ifndef __TMEMBLOCK_H
#define __TMEMBLOCK_H
#include "tEvent.h"


typedef struct _tMemBlock {
	tEvent event;//事件控制块
	
	void * memStart;//存储块首地址
	
	uint32_t blockSize;//每个存储块的大小
	
	uint32_t maxCount;//存储块的最大数量
	
	tList blockList;
	
}tMemBlock;
void tMemBlockInit(tMemBlock * memBlock, uint8_t * memStart, uint32_t blockSize, uint32_t blockCnt);

uint32_t tMemBlockWait(tMemBlock * memBlock, uint8_t ** mem, uint32_t waitTicks);

uint32_t tMemBlockNoWaitGet(tMemBlock * memBlock, uint8_t ** mem, uint32_t waitTicks);

uint32_t tMemBlockNotify(tMemBlock * memBlock, uint8_t *mem);

#endif

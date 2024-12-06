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

typedef struct _tMemBlockInfo {
	uint32_t count;
	uint32_t maxCount;
	uint32_t blockSize;
	uint32_t taskCount;
}tMemBlockInfo;
void tMemBlockInit(tMemBlock * memBlock, uint8_t * memStart, uint32_t blockSize, uint32_t blockCnt);

uint32_t tMemBlockWait(tMemBlock * memBlock, uint8_t ** mem, uint32_t waitTicks);

uint32_t tMemBlockNoWaitGet(tMemBlock * memBlock, uint8_t ** mem, uint32_t waitTicks);

uint32_t tMemBlockNotify(tMemBlock * memBlock, uint8_t *mem);

void tMemBlockGetInfo(tMemBlock * memBlock, tMemBlockInfo * info);

uint32_t tMemBlockDestory(tMemBlock * memBlock);
#endif

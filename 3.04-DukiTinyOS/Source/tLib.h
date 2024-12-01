#ifndef __TLIB_H
#define __TLIB_H

#include <stdint.h>

typedef struct _tBitMap {
	uint32_t bitMap;
}tBitMap;

void tBitMapInit(tBitMap* bitMap);
uint32_t tBitMapPosCount(void);
void tBitMapSet(tBitMap* bitMap,uint32_t pos);//对应位置1
void tBitMapClear(tBitMap* bitMap,uint32_t pos);//对应位置0
uint32_t tBitMapGetFirstSet(tBitMap* bitMap);//获取第一个为1的位

#endif
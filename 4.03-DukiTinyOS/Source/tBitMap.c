#include "tLib.h"

void tBitMapInit(tBitMap* bitMap) {
	bitMap->bitMap = 0;
}
uint32_t tBitMapPosCount(void) {
	return 32;
}
void tBitMapSet(tBitMap* bitMap,uint32_t pos) {
	//对应位置1
	bitMap->bitMap |= 1 << pos;
}
void tBitMapClear(tBitMap* bitMap,uint32_t pos){
	//对应位置0 
	bitMap->bitMap &= ~(1 << pos);
}
uint32_t tBitMapGetFirstSet(tBitMap* bitMap) {
	//获取第一个为1的位
	static const uint8_t quickFindTable[] = {
		0xff, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, //0x00 to 0x0F
		4   , 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, //0x10 to 0x1F
		5   , 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, //0x20 to 0x2F
		4   , 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, //0x30 to 0x3F
		6   , 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, //0x40 to 0x4F
		4   , 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, //0x50 to 0x5F
		5   , 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, //0x60 to 0x6F
		4   , 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, //0x70 to 0x7F
		7   , 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, //0x80 to 0x8F
		4   , 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, //0x90 to 0x9F
		5   , 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, //0xA0 to 0xAF
		4   , 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, //0xB0 to 0xBF
		6   , 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, //0xC0 to 0xCF
		4   , 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, //0xD0 to 0xDF
		5   , 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, //0xE0 to 0xEF
		4   , 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0  //0xF0 to 0xFF
	};
	//分四段查找32位bitMap中第一个出现0的地方
	if(bitMap->bitMap & 0x000000ff) {
		return quickFindTable[bitMap->bitMap & 0xff];
	}else if(bitMap->bitMap & 0x0000ff00) {
		return quickFindTable[(bitMap->bitMap >> 8 ) & 0xff] + 8 ;
	}else if(bitMap->bitMap & 0x00ff0000) {
		return quickFindTable[(bitMap->bitMap >> 16) & 0xff] + 16;
	}else if(bitMap->bitMap & 0xff000000) {
		return quickFindTable[(bitMap->bitMap >> 24) & 0xff] + 24;
	}else {
		return tBitMapPosCount();
	}
	
}
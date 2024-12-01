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

//定义结点
typedef struct _tNode {
	struct _tNode * preNode;
	struct _tNode * nextNode;
}tNode;

typedef struct _tList {
	tNode headNode;
	uint32_t nodeCount;
}tList;

//链表相关函数
#define tNodeParent(node, parent, name) (parent *)((uint32_t)node - (uint32_t)&((parent *)0)->name) //?
void tNodeInit(tNode * node) ;
void tListInit(tList * list) ;
uint32_t tListCount(tList * list) ;
tNode * tListFirst(tList * list) ;
tNode * tListLast(tList * list) ;
tNode * tListPre(tList * list, tNode * node) ;
tNode * tListNext(tList * list, tNode * node) ;
void tListRemoveAll(tList * list) ;
void tListAddFirst(tList * list, tNode * node) ;
void tListAddLast(tList * list, tNode * node) ;
tNode * tListRemoveFirst(tList * list) ;
void tListInsertAfter(tList * list, tNode * nodeAfter, tNode * nodeToInsert) ;
void tListRemove(tList * list,tNode * node);
#endif


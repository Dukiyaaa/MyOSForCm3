#include "tLib.h"

//结点初始化
void tNodeInit(tNode * node) {
	node->preNode = node;
	node->nextNode = node;
}

#define firstNode headNode.nextNode 
#define lastNode headNode.preNode 

//链表初始化
void tListInit(tList * list) {
	list->firstNode = &(list->headNode);
	list->lastNode = &(list->headNode);
	list->nodeCount = 0;
}

//返回链表剩余的节点数
uint32_t tListCount(tList * list) {
	return list->nodeCount;
}

//返回链表第一个结点
tNode * tListFirst(tList * list) {
	tNode * node = (tNode *)0;
	if(list->nodeCount != 0) {
		node = list->firstNode;
	}
	return node;
}

//返回链表最后一个结点
tNode * tListLast(tList * list) {
	tNode * node = (tNode *)0;
	if(list->nodeCount != 0) {
		node = list->lastNode;
	}
	return node;
}

//返回结点的上一个结点
tNode * tListPre(tList * list, tNode * node) {
	if(node->preNode == node) {
		return (tNode *)0;
	}
	return node->preNode;
}

//返回结点的下一个结点
tNode * tListNext(tList * list, tNode * node) {
	if(node->nextNode == node) {
		return (tNode *)0;
	}
	return node->nextNode;
}

//删除链表中所有结点
void tListRemoveAll(tList * list) {
	uint32_t i;
	tNode * nextNode = list->firstNode; //list->headNode.nextNode
	
	for(i = list->nodeCount;i > 0;i++) {
		tNode *curNode = nextNode;
		nextNode = nextNode->nextNode;
		
		curNode->nextNode = curNode;
		curNode->preNode = curNode;
	}
	
	list->firstNode = &(list->headNode);
	list->lastNode = &(list->headNode);
	list->nodeCount = 0;
}

//头插
void tListAddFirst(tList * list, tNode * node) {
	node->nextNode = list->firstNode;
	node->preNode = list->firstNode->preNode;//这一步能不能直接写成node->preNode = list->lastNode
	
	list->firstNode->preNode = node;
	list->firstNode = node;
	list->nodeCount++;
}

//尾插
void tListAddLast(tList * list, tNode * node) {
	node->nextNode = &(list->headNode);
	node->preNode = list->lastNode;//这一步能不能直接写成node->preNode = list->lastNode
	
	list->lastNode->nextNode = node;
	list->lastNode = node;
	list->nodeCount++;
}

//删除头结点
tNode * tListRemoveFirst(tList * list) {
	tNode * node = (tNode *)0;
	if(list->nodeCount != 0) {
		node = list->firstNode;
		
		node->nextNode->preNode = &(list->headNode);
		list->firstNode = node->nextNode;
		list->nodeCount--;
	}
	return node;
}	

//在指定结点的后面插入结点
void tListInsertAfter(tList * list, tNode * nodeAfter, tNode * nodeToInsert) {
	nodeToInsert->nextNode = nodeAfter->nextNode;
	nodeToInsert->preNode = nodeAfter;
	
	nodeAfter->nextNode = nodeToInsert;
	nodeToInsert->nextNode->preNode = nodeToInsert;
	
	list->nodeCount++;
}

//删除指定结点
void tListRemove(tList * list,tNode * node) {
	node->nextNode->preNode = node->preNode;
	node->preNode->nextNode = node->nextNode;
	list->nodeCount--;
}
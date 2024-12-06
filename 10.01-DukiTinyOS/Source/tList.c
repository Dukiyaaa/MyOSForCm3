#include "tLib.h"

/**
 * @brief 初始化链表结点
 * 
 * 该函数初始化链表结点，使得每个结点的前后指针指向自己，表示这是一个单独的结点。
 * 
 * @param node 需要初始化的链表结点
 * 
 * @return void
 */
void tNodeInit(tNode * node) {
    node->preNode = node; // 前指针指向自己
    node->nextNode = node; // 后指针指向自己
}

#define firstNode headNode.nextNode 
#define lastNode headNode.preNode 

/**
 * @brief 初始化链表
 * 
 * 该函数初始化链表，将头结点的前后指针都指向头结点本身，并且节点计数设为0。
 * 
 * @param list 链表指针
 * 
 * @return void
 */
void tListInit(tList * list) {
    list->firstNode = &(list->headNode); // 设置链表的第一个结点
    list->lastNode = &(list->headNode);  // 设置链表的最后一个结点
    list->nodeCount = 0;                 // 设置链表的结点数为0
}

/**
 * @brief 获取链表的结点数量
 * 
 * 该函数返回链表中当前存在的结点数量。
 * 
 * @param list 链表指针
 * 
 * @return uint32_t 链表中结点的数量
 */
uint32_t tListCount(tList * list) {
    return list->nodeCount; // 返回链表中的结点数
}

/**
 * @brief 获取链表中的第一个结点
 * 
 * 该函数返回链表中的第一个结点。如果链表为空，返回NULL。
 * 
 * @param list 链表指针
 * 
 * @return tNode* 链表的第一个结点
 */
tNode * tListFirst(tList * list) {
    tNode * node = (tNode *)0;
    if (list->nodeCount != 0) {
        node = list->firstNode; // 如果链表不为空，返回第一个结点
    }
    return node;
}

/**
 * @brief 获取链表中的最后一个结点
 * 
 * 该函数返回链表中的最后一个结点。如果链表为空，返回NULL。
 * 
 * @param list 链表指针
 * 
 * @return tNode* 链表的最后一个结点
 */
tNode * tListLast(tList * list) {
    tNode * node = (tNode *)0;
    if (list->nodeCount != 0) {
        node = list->lastNode; // 如果链表不为空，返回最后一个结点
    }
    return node;
}

/**
 * @brief 获取指定结点的前一个结点
 * 
 * 该函数返回链表中指定结点的前一个结点。如果没有前一个结点，返回NULL。
 * 
 * @param list 链表指针
 * @param node 当前结点
 * 
 * @return tNode* 当前结点的前一个结点
 */
tNode * tListPre(tList * list, tNode * node) {
    if (node->preNode == node) { // 如果前一个结点是自己，表示链表只有一个结点
        return (tNode *)0;
    }
    return node->preNode; // 返回当前结点的前一个结点
}

/**
 * @brief 获取指定结点的下一个结点
 * 
 * 该函数返回链表中指定结点的下一个结点。如果没有下一个结点，返回NULL。
 * 
 * @param list 链表指针
 * @param node 当前结点
 * 
 * @return tNode* 当前结点的下一个结点
 */
tNode * tListNext(tList * list, tNode * node) {
    if (node->nextNode == node) { // 如果下一个结点是自己，表示链表只有一个结点
        return (tNode *)0;
    }
    return node->nextNode; // 返回当前结点的下一个结点
}

/**
 * @brief 删除链表中的所有结点
 * 
 * 该函数将链表中的所有结点移除，并且重置链表为初始化状态（节点数为0，头尾结点指向自己）。
 * 
 * @param list 链表指针
 * 
 * @return void
 */
void tListRemoveAll(tList * list) {
    uint32_t i;
    tNode * nextNode = list->firstNode; // 获取链表中的第一个结点
    
    for (i = list->nodeCount; i > 0; i--) {
        tNode * curNode = nextNode;
        nextNode = nextNode->nextNode; // 逐个遍历结点

        // 清除当前结点的前后指针
        curNode->nextNode = curNode;
        curNode->preNode = curNode;
    }
    
    list->firstNode = &(list->headNode); // 重置头结点的指针
    list->lastNode = &(list->headNode);  // 重置尾结点的指针
    list->nodeCount = 0;                 // 设置节点数为0
}

/**
 * @brief 在链表头部插入结点
 * 
 * 该函数将指定结点插入到链表的头部。
 * 
 * @param list 链表指针
 * @param node 要插入的结点
 * 
 * @return void
 */
void tListAddFirst(tList * list, tNode * node) {
    node->nextNode = list->firstNode; // 将当前结点的后指针指向链表的第一个结点
    node->preNode = list->firstNode->preNode; // 将当前结点的前指针指向链表的尾结点
    
    list->firstNode->preNode = node;  // 将链表的第一个结点的前指针指向当前结点
    list->firstNode = node;            // 将链表的第一个结点指向当前结点
    list->nodeCount++;                 // 增加节点计数
}

/**
 * @brief 在链表尾部插入结点
 * 
 * 该函数将指定结点插入到链表的尾部。
 * 
 * @param list 链表指针
 * @param node 要插入的结点
 * 
 * @return void
 */
void tListAddLast(tList * list, tNode * node) {
    node->nextNode = &(list->headNode);  // 将当前结点的后指针指向链表的头结点
    node->preNode = list->lastNode;      // 将当前结点的前指针指向链表的最后一个结点
    
    list->lastNode->nextNode = node;     // 将链表的最后一个结点的后指针指向当前结点
    list->lastNode = node;               // 将链表的最后一个结点指向当前结点
    list->nodeCount++;                   // 增加节点计数
}

/**
 * @brief 删除链表中的第一个结点
 * 
 * 该函数将链表中的第一个结点删除，并返回被删除的结点。
 * 
 * @param list 链表指针
 * 
 * @return tNode* 被删除的结点
 */
tNode * tListRemoveFirst(tList * list) {
    tNode * node = (tNode *)0;
    if (list->nodeCount != 0) {
        node = list->firstNode; // 获取链表中的第一个结点
        
        node->nextNode->preNode = &(list->headNode); // 更新链表中第二个结点的前指针
        list->firstNode = node->nextNode;            // 更新链表的第一个结点
        list->nodeCount--;                            // 减少结点计数
    }
    return node; // 返回被删除的结点
}

/**
 * @brief 在指定结点后插入新结点
 * 
 * 该函数将在链表中指定结点的后面插入新的结点。
 * 
 * @param list 链表指针
 * @param nodeAfter 插入位置的参考结点
 * @param nodeToInsert 要插入的结点
 * 
 * @return void
 */
void tListInsertAfter(tList * list, tNode * nodeAfter, tNode * nodeToInsert) {
    nodeToInsert->nextNode = nodeAfter->nextNode; // 将新结点的后指针指向原结点的后结点
    nodeToInsert->preNode = nodeAfter;            // 将新结点的前指针指向原结点
    
    nodeAfter->nextNode = nodeToInsert;           // 将原结点的后指针指向新结点
    nodeToInsert->nextNode->preNode = nodeToInsert; // 更新原结点后结点的前指针
    list->nodeCount++;                            // 增加结点计数
}

/**
 * @brief 删除指定结点
 * 
 * 该函数删除链表中的指定结点。
 * 
 * @param list 链表指针
 * @param node 要删除的结点
 * 
 * @return void
 */
void tListRemove(tList * list, tNode * node) {
    node->nextNode->preNode = node->preNode; // 将下一个结点的前指针指向当前结点的前结点
    node->preNode->nextNode = node->nextNode; // 将上一个结点的后指针指向当前结点的后结点
    list->nodeCount--;                         // 减少结点计数
}

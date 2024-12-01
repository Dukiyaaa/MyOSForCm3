#ifndef __TTASK_H
#define __TTASK_H

#define TINYOS_TASK_STATE_RDY      0
#define TINYOS_TASK_STATE_DESTORYED  (1 << 1) //删除状态
#define TINYOS_TASK_STATE_DELAYED    (1 << 2)
#define TINYOS_TASK_STATE_SUSPEND    (1 << 3) //挂起状态
#define TINYOS_TASK_WAIT_MASK        (0xff << 16)

struct _tEvent;
typedef uint32_t tTaskStack;

typedef struct _tTask {
	tTaskStack* stack;
	uint32_t delayTicks;//添加软定时器
	uint32_t prio;//添加优先级字段
	tNode delayNode; //为了方便加入延时队列
	uint32_t state;//指示状态
	tNode linkNode;
	uint32_t slice;//时间片
	uint32_t suspendCount;//挂起计数器
	
	//删除部分字段
	void (*clean) (void * param);
	void * cleanParam;
	uint8_t requestDeleteFlag;
	
	//事件控制块部分字段
	struct _tEvent * waitEvent;
	void * eventMsg;
	uint32_t waitEventResult;
}tTask;

typedef struct _tTaskInfo {
	uint32_t delayTicks;
	uint32_t prio;
	uint32_t state;
	uint32_t slice;
	uint32_t suspendCount;
}tTaskInfo;

void tTaskInit(tTask *tTask, void(*entry)(void *), void * param, uint32_t prio, tTaskStack * tTaskStack);

//挂起函数
void tTaskSuspend(tTask * task) ;
void tTaskWakeUp(tTask * task) ;
//删除函数
void tTaskSetCleanCallFunc(tTask * task, void (*clean) (void * param), void * param);
void tTaskForceDelete(tTask * task);
void tTaskRequestDelete(tTask * task);
uint8_t tTaskIsRequestedDeleted(void);
void tTaskDeleteSelf(void);
//任务状态查询
void tTaskGetInfo(tTask * task,tTaskInfo * info);
#endif




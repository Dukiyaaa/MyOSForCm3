#include "tinyOS.h"
#include "ARMCM3.h"
#include "tLib.h"
#include "tConfig.h"

tTask * curTask;
tTask * nextTask;
tTask * idletask;


tBitMap taskPrioBitMap;//优先级位图
tList taskTable[TINYOS_PRIO_COUNT];//任务表

//调度锁计数器
uint8_t schedLockCounter; //八位，最大255

//延时队列
tList tTaskDelayedList;

//空闲任务计数器，用于计算空闲代码块总count
uint32_t idleCount;
uint32_t idleMaxCount;

//统计时钟节拍发生次数
uint32_t tickCount;

static void cpuUsageSyncWithSysTick(void);
static void initCpuUsageState(void);
static void checkCpuUsage(void);

//查找最高优先级的就绪任务
tTask * tTaskHighestReady(void) {
	uint32_t highestPrio = tBitMapGetFirstSet(&taskPrioBitMap); //利用优先级位图法找到最高优先级
	tNode *node = tListFirst(&(taskTable[highestPrio]));        //取出对应优先级就绪队列的第一个任务结点
	return tNodeParent(node, tTask, linkNode);                  //利用宏由任务结点反向推出任务结构体指针
}

void tTaskSched(void) {
	tTask *tmpTask;
	uint32_t status = tTaskEnterCritical();
	if(schedLockCounter > 0) {
		tTaskExitCritical(status);
		return;
	}
	
	tmpTask = tTaskHighestReady();
	if(tmpTask != curTask) {
		nextTask = tmpTask;
		tTaskSwitch();//参照uc/osII的写法，在tTaskSched调用前退出临界区，在tTaskSwitch调用前不退
	}
	
	tTaskExitCritical(status);
}

//延时队列初始化
void tTaskDelayedInit(void) {
	tListInit(&(tTaskDelayedList));
}

//
void tTaskSchedInit(void) {
	int i;
	uint32_t status = tTaskEnterCritical();
	schedLockCounter = 0;
	for(i = 0;i < TINYOS_PRIO_COUNT;i++) {
		tListInit(&(taskTable[i]));
	}
	tTaskExitCritical(status);
}

//调度失能
void tTaskSchedDisable(void) {
	uint32_t status = tTaskEnterCritical();
	if(schedLockCounter < 255) {
		schedLockCounter++;
	}
	tBitMapInit(&taskPrioBitMap);
	tTaskExitCritical(status);
}

//调度使能
void tTaskSchedEnable(void) {
	uint32_t status = tTaskEnterCritical();
	if(schedLockCounter > 0) {
		if(--schedLockCounter == 0) {
			//减到0，说明没有调度锁了，可以调度任务
			tTaskSched();
		}
	}
	tTaskExitCritical(status);
}

//插入就绪队列
void tTaskSchedRdy(tTask * task) {
	tListAddFirst(&(taskTable[task->prio]),&(task->linkNode));
	tBitMapSet(&taskPrioBitMap,task->prio);
}
//将任务设置为非就绪状态
void tTaskSchedUnRdy(tTask * task) {
	tListRemove(&(taskTable[task->prio]),&(task->linkNode));
	if(tListCount(&taskTable[task->prio]) == 0) {
		tBitMapClear(&taskPrioBitMap,task->prio);
	}
}

//从优先级队列移出
void tTaskSchedRemove(tTask * task) {
	tListRemove(&(taskTable[task->prio]),&(task->linkNode));
	if(tListCount(&taskTable[task->prio]) == 0) {
		tBitMapClear(&taskPrioBitMap,task->prio);
	}
}

//任务加入延时队列
void tTimeTaskWait(tTask * task,uint32_t ticks) {
	task->delayTicks = ticks;
	tListAddLast(&tTaskDelayedList,&(task->delayNode));
	task->state |= TINYOS_TASK_STATE_DELAYED;
}

//任务取消延时状态
void tTimeTaskWakeUp(tTask * task) {
	tListRemove(&tTaskDelayedList,&(task->delayNode));
	task->state &= ~TINYOS_TASK_STATE_DELAYED;
}

//任务移出延时队列
void tTimeTaskRemove(tTask * task) {
	tListRemove(&tTaskDelayedList,&(task->delayNode));
}


//tickCount初始化
void tTimeTickInit(void) {
	tickCount = 0;
}

void tTaskSystemTickHandler(void) {
	tNode * node;
	uint32_t status = tTaskEnterCritical();
	for(node = tTaskDelayedList.headNode.nextNode;node != &(tTaskDelayedList.headNode);node = node->nextNode) {
		tTask * task = tNodeParent(node,tTask,delayNode);
		if(--task->delayTicks == 0) {
			if(task->waitEvent) {
				tEventRemoveTask(task,(void *)0,tErrorTimeOut);
			}
			tTimeTaskWakeUp(task);
			tTaskSchedRdy(task);
		}
	}
	
	if(--curTask->slice == 0) {
		if(tListCount(&(taskTable[curTask->prio])) > 0) {
			tListRemoveFirst(&(taskTable[curTask->prio]));
			tListAddLast(&(taskTable[curTask->prio]),&(curTask->linkNode));
			
			curTask->slice = TINYOS_SLICE_MAX;
		}
	}
	
	tickCount++;
	checkCpuUsage();//计算cpu使用率
	tTaskExitCritical(status);
	//此函数会先扫描硬件定时器列表，再通知软定时器可以扫描一次
	tTimerModuleTickNotify();
	tTaskSched();
}

static float cpuUsage;
//需要保证在一个节拍新开始时空闲计数器开始计数
uint32_t enableCpuUsageState;
static void initCpuUsageState(void) {
	enableCpuUsageState = 0;
	idleCount = 0;
	cpuUsage = 0.0f;
  idleMaxCount = 0;
}

static void checkCpuUsage(void) {
	if(enableCpuUsageState == 0) {
		enableCpuUsageState = 1;
		tickCount = 0;
		return;
	}
	
	if(tickCount == TICKS_PER_SEC) {
		idleMaxCount = idleCount;
		idleCount = 0;
		tTaskSchedEnable();
	}
	//之后每隔一秒钟计算一次占用率
	else if(tickCount % TICKS_PER_SEC == 0) {
		cpuUsage = (1 - idleCount / idleMaxCount) * 100;
		idleCount = 0;
	}
}

static void cpuUsageSyncWithSysTick(void) {
	while(enableCpuUsageState == 0) {
		;;
	}
}

float tCpuUsageGet(void) {
	float usage = 0;
	uint32_t status = tTaskEnterCritical();
	usage = cpuUsage;
	tTaskExitCritical(status);
	
	return usage;
}
//定义空闲任务
tTask TtaskIdle;
tTaskStack taskIdleStack[TINYOS_IDLETASK_STACK_SIZE];

void taskIdle(void *param) {
	tTaskSchedDisable();//关闭调度锁 防止空闲任务第一次运行时进行任务调度
	//用户任务初始化
	tInitApp();
  //软硬定时器任务初始化
	tTimerInitTask();
	//定时器节拍设定
	tSetSysTickPeriod(TINYOS_SYSTICK_MS);
	//进行时钟同步，保证在一个周期开始时开始计数
	cpuUsageSyncWithSysTick();
	while(1) {
		uint32_t status = tTaskEnterCritical();
		idleCount++;
		tTaskExitCritical(status);
	}
	
}

 int main() {	
	tTaskSchedInit();
	 
	tTaskDelayedInit();
	 
	tTimerModuleInit();
	 
	tTimeTickInit();//初始化tickcount = 0
	 
	initCpuUsageState();// 初始化cpu测量相关的变量
	 
	tTaskInit(&TtaskIdle, taskIdle, (void *)0, TINYOS_PRIO_COUNT - 1, taskIdleStack,TINYOS_IDLETASK_STACK_SIZE); //最低优先级
	
	nextTask = tTaskHighestReady();
	
	tTaskRunFirst();
	return 0;
}

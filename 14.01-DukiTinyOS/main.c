#include "tinyOS.h"
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

#if TINYOS_ENABLE_CPUUSAGE_STATE == 1
//空闲任务计数器，用于计算空闲代码块总count
uint32_t idleCount;
uint32_t idleMaxCount;

//统计时钟节拍发生次数
uint32_t tickCount;

#endif

#if TINYOS_ENABLE_CPUUSAGE_STATE == 1
static void initCpuUsageState (void);
static void checkCpuUsage (void);
static void cpuUsageSyncWithSysTick (void);
#endif

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
#if TINYOS_ENABLE_HOOKS == 1		
		tHooksTaskSwitch(curTask,nextTask);
#endif
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
  tBitMapInit(&taskPrioBitMap);
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

#if TINYOS_ENABLE_CPUUSAGE_STATE == 1
//tickCount初始化
void tTimeTickInit(void) {
	tickCount = 0;
}
#endif
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
	
	
#if TINYOS_ENABLE_CPUUSAGE_STATE == 1
	  // 节拍计数增加
    tickCount++;
    // 检查cpu使用率
    checkCpuUsage();
#endif
    // 退出临界区
    tTaskExitCritical(status); 
#if TINYOS_ENABLE_TIMER == 1
    // 通知定时器模块节拍事件
    tTimerModuleTickNotify();
#endif
#if TINYOS_ENABLE_HOOKS == 1
	  tHooksSysTick();
#endif
    // 这个过程中可能有任务延时完毕(delayTicks = 0)，进行一次调度。
    tTaskSched();
}

#if TINYOS_ENABLE_CPUUSAGE_STATE == 1
static float cpuUsage;
//需要保证在一个节拍新开始时空闲计数器开始计数
uint32_t enableCpuUsageState;
static void initCpuUsageState (void)
{
    idleCount = 0;
    idleMaxCount = 0;
    cpuUsage = 0;
    enableCpuUsageState = 0;
}

static void checkCpuUsage (void)
{
    // 与空闲任务的cpu统计同步
    if (enableCpuUsageState == 0)
    {
        enableCpuUsageState = 1;
        tickCount = 0;
        return;
    }

    if (tickCount == TICKS_PER_SEC)
    {
        // 统计最初1s内的最大计数值
        idleMaxCount = idleCount;
        idleCount = 0;

        // 计数完毕，开启调度器，允许切换到其它任务
        tTaskSchedEnable();
    }
    else if (tickCount % TICKS_PER_SEC == 0)
    {
        // 之后每隔1s统计一次，同时计算cpu利用率
        cpuUsage = 100 - (idleCount * 100.0 / idleMaxCount);
        idleCount = 0;
    }
}


static void cpuUsageSyncWithSysTick (void)
{
    // 等待与时钟节拍同步
    while (enableCpuUsageState == 0)
    {
        ;;
    }
}

float tCpuUsageGet (void)
{
    float usage = 0;

    uint32_t status = tTaskEnterCritical();
    usage = cpuUsage;
    tTaskExitCritical(status);

    return usage;
}

#endif
// 用于空闲任务的任务结构和堆栈空间
tTask tTaskIdle;
tTaskStack idleTaskEnv[TINYOS_IDLETASK_STACK_SIZE];

void idleTaskEntry (void * param) {
	
#if TINYOS_ENABLE_CPUUSAGE_STATE == 1
    // 禁止调度，防止后面在创建任务时切换到其它任务中去
    tTaskSchedDisable();
#endif
	
    // 初始化App相关配置
    tInitApp();
	
#if TINYOS_ENABLE_TIMER == 1
    // 初始化定时器任务
    tTimerInitTask();
#endif
	
    // 启动系统时钟节拍
    tSetSysTickPeriod(TINYOS_SYSTICK_MS);
	
#if TINYOS_ENABLE_CPUUSAGE_STATE == 1
    // 等待与时钟同步
    cpuUsageSyncWithSysTick();
#endif
	
    for (;;)
    {
#if TINYOS_ENABLE_CPUUSAGE_STATE == 1			
        uint32_t status = tTaskEnterCritical();
        idleCount++;
        tTaskExitCritical(status);
#endif
			
#if TINYOS_ENABLE_HOOKS == 1
			tHooksCpuIdle();
#endif
    }
}

int main () 
{
    // 优先初始化tinyOS的核心功能
    tTaskSchedInit();

    // 初始化延时队列
    tTaskDelayedInit();

#if TINYOS_ENABLE_TIMER == 1
    // 初始化定时器模块
    tTimerModuleInit();
#endif
	
    
#if TINYOS_ENABLE_CPUUSAGE_STATE == 1
	  // 初始化时钟计数器
    tTimeTickInit();
    // 初始化cpu统计
    initCpuUsageState();
#endif
	
    // 创建空闲任务
    tTaskInit(&tTaskIdle, idleTaskEntry, (void *)0, TINYOS_PRIO_COUNT - 1, idleTaskEnv, TINYOS_IDLETASK_STACK_SIZE);
    
    // 这里，不再指定先运行哪个任务，而是自动查找最高优先级的任务运行
    nextTask = tTaskHighestReady();

    // 切换到nextTask， 这个函数永远不会返回
    tTaskRunFirst();
    return 0;
}

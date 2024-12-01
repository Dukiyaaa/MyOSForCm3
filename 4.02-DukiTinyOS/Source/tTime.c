#include "tinyOS.h"

//操作系统延时函数
void tTaskDelay(uint32_t delay) {
	uint32_t status = tTaskEnterCritical();
	//插入延时队列
	tTimeTaskWait(curTask,delay);
	//移出就绪队列
	tTaskSchedUnRdy(curTask);
	tTaskExitCritical(status);
	tTaskSched();
}	


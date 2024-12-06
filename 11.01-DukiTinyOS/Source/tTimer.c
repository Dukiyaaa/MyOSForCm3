#include "tinyOS.h"

void tTimerInit (tTimer * timer, uint32_t delayTicks, uint32_t durationTicks,
                 void (*timerFunc) (void * arg), void * arg, uint32_t config)
{
		tNodeInit(&timer->linkNode);	
		timer->startDelayTicks = delayTicks;
		timer->durationTicks = durationTicks;
		timer->timerFunc = timerFunc;
		timer->arg = arg;
		timer->config = config;
	
	  //递减计数值赋值
		if(delayTicks == 0) {
			timer->delayTicks = durationTicks;
		}else {
			timer->delayTicks = timer->startDelayTicks;
		}
		
		timer->state = tTimerCreated;
}

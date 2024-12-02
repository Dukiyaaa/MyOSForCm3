#include "tinyOS.h"

void tMboxInit(tMbox * mbox,void ** msgBuffer,uint32_t maxCount) {
	tEventInit(&mbox->event,tEventTypeMbox);//初始化事件为邮箱类型
	
	mbox->msgBuffer = msgBuffer;
	mbox->maxCount = maxCount;
	mbox->count = 0;
	mbox->read = 0;
	mbox->write = 0;
}

//有等待申请
uint32_t tMboxWait(tMbox * mbox, void ** msg, uint32_t waitTicks) {
	uint32_t status = tTaskEnterCritical();
	
	if(mbox->count > 0) {
		--mbox->count;
		*msg = mbox->msgBuffer[mbox->read++];
		if(mbox->read >= mbox->maxCount) {
			mbox->read = 0;
		}
		tTaskExitCritical(status);
		return tErrorNoError;
	}else {
		tEventWait(&mbox->event,curTask,(void *)0,tEventTypeMbox, waitTicks);
		tTaskExitCritical(status);
		
		tTaskSched();
		
		*msg = curTask->eventMsg;
		return curTask->waitEventResult;
	}
}

//无等待申请
uint32_t tMboxNoWaitGet(tMbox * mbox, void ** msg) {
	uint32_t status = tTaskEnterCritical();
	
	if(mbox->count > 0) {
		--mbox->count;
		*msg = mbox->msgBuffer[mbox->read++];
		if(mbox->read >= mbox->maxCount) {
			mbox->read = 0;
		}
		tTaskExitCritical(status);
		return tErrorNoError;
	}else {
		tTaskExitCritical(status);	
		return tErrorResourceUnavaliable;
	}
}

//释放资源
uint32_t tMboxNotify(tMbox * mbox, void * msg, uint32_t notifyOption) {
	uint32_t status = tTaskEnterCritical();
	
	if(tEventWaitCount(&mbox->event) > 0) {
		tTask * task = tEventWakeUp(&mbox->event,(void *)msg,tErrorNoError);
		if(task->prio < curTask->prio) {
			tTaskSched();
		}
	}else {
		if(mbox->count >= mbox->maxCount) {
			tTaskExitCritical(status);	
			return tErrorResourceFull;
		}
		//依据notifyOption决定是插入消息头还是消息尾
		if(notifyOption & tMboxSendFront) {
			if(mbox->read <= 0) {
				mbox->read = mbox->maxCount - 1;
			}else {
				--mbox->read;
			}
			mbox->msgBuffer[mbox->read] = msg;
		}else {
			mbox->msgBuffer[mbox->write++] = msg;
			if(mbox->write >= mbox->maxCount) {//就因为这边一步导致现象不同
				mbox->write = 0;
			}
		}
		mbox->count++;
	}
	
	tTaskExitCritical(status);	
	return tErrorNoError;
}

//邮箱清空
void tMboxFlush(tMbox * mbox) {
	uint32_t status = tTaskEnterCritical();
	
	if(tEventWaitCount(&mbox->event) == 0) {
		//如果邮箱事件中等待的任务数不为0，说明邮件资源已经没有了，就不用清空
		mbox->count = 0;
		mbox->read = 0;
		mbox->write = 0;
	}
	
	tTaskExitCritical(status);	
}
//邮箱删除,跟信号量删除类似
uint32_t tMboxDestory(tMbox * mbox) {
	//删除任务列表
	uint32_t status = tTaskEnterCritical();
	
	uint32_t count = tEventRemoveAll(&mbox->event,(void *)0,tErrorDel);
	tTaskExitCritical(status);	
	
	if(count > 0) {
		tTaskSched();
	}
	
	return count;
}
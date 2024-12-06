#include "tinyOS.h"

/**
 * @brief 操作系统延时函数，用于使当前任务延迟指定的时间。
 * 
 * 该函数会将当前任务加入延时队列，并将任务从就绪队列中移除，直到指定的延时周期结束后再恢复任务。
 * 
 * @param delay 延时周期，单位为时钟滴答（ticks）。
 * 
 * @return void
 */
void tTaskDelay(uint32_t delay) {
    // 进入临界区，确保在修改任务状态和操作队列期间不受中断干扰
    uint32_t status = tTaskEnterCritical();

    // 将当前任务插入延时队列，等待指定的时间（delay）
    tTimeTaskWait(curTask, delay);

    // 将当前任务移出就绪队列，表示该任务不再处于就绪状态
    tTaskSchedUnRdy(curTask);

    // 退出临界区，恢复中断
    tTaskExitCritical(status);

    // 调用任务调度器，检查是否需要切换任务
    tTaskSched();
}

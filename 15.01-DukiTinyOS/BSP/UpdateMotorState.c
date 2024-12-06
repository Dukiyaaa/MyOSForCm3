#include "UpdateMotorState.h"
#include "Receiver.h"
#include "Motor.h"

void UpdateMotorState(void) {
	for(int i = 0;i < CHANNEL_COUNT;i++) {
		// 从接收机获取映射值 (0 ~ 1.0)
			float mappedValue = Receiver_GetMappedValue(i);

			// 将映射值转化为占空比 (0.05 ~ 1.0)
			float motorDuty = 0.05 * mappedValue + 0.05; // 映射到占空比范围

			// 更新电机状态
			Motor_SetPulse(i + 1, motorDuty); // 电机通道从1开始，+1偏移
	}
}
#ifndef __ATTITUDEPIDCONTROLLER_H
#define __ATTITUDEPIDCONTROLLER_H
#include "main.h"
#include "PID.h"

typedef struct _AttitudePIDController {
	float ax, ay, az, gx, gy, gz; //六个轴的数据
	float Roll, Pitch, Yaw; //三个角的数据
	int count;         //用于yaw线性回归矫正的计数器
	PIDCascade rollPID;
	PIDCascade pitchPID;
	PID yawPID;//三个角的pid
}AttitudePIDController;

void AttitudePIDController_Init(AttitudePIDController * attitudePIDController);

void AttitudePIDController_ProcData(AttitudePIDController * attitudePIDController);
#endif

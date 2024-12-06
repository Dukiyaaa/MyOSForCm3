#include "AttitudePIDController.h"
#include "MadgWick.h"
#include "AttitudeSolver.h"
#include "MPU6050.h"
#include "MySerial.h"

void AttitudePIDController_Init(AttitudePIDController * attitudePIDController) {
	//
	attitudePIDController->ax = 0;
	attitudePIDController->ay = 0;
	attitudePIDController->az = 0;
	attitudePIDController->gx = 0;
	attitudePIDController->gy = 0;
	attitudePIDController->gz = 0;
	attitudePIDController->Pitch = 0;
	attitudePIDController->Roll = 0;
	attitudePIDController->Yaw = 0;
	attitudePIDController->count = 0;
	
	AttitudeSolver_Init(50, 0.8f); // 采样频率50Hz，增益为0.8
	PIDAngleProc_Init(&attitudePIDController->rollPID,&attitudePIDController->pitchPID,&attitudePIDController->yawPID);
}

void AttitudePIDController_ProcData(AttitudePIDController * attitudePIDController) {
	//mpu6050处理数据
		MPU6050_GetAccelData(&attitudePIDController->ax, &attitudePIDController->ay, &attitudePIDController->az);
		MPU6050_GetGyroData(&attitudePIDController->gx, &attitudePIDController->gy, &attitudePIDController->gz);
	//madgwick处理数据
	  AttitudeSolver_UpdateIMU(attitudePIDController->gx, attitudePIDController->gy, attitudePIDController->gz,attitudePIDController->ax, attitudePIDController->ay, attitudePIDController->az);
	//转换为欧拉角
	 AttitudeSolver_GetEulerAngles(&attitudePIDController->Roll,&attitudePIDController->Pitch,&attitudePIDController->Yaw,&attitudePIDController->count);
	//注意：此时三个角的单位为°
//   printf("Accel: X=%.2fg Y=%.2fg Z=%.2fg\n", attitudePIDController->ax, attitudePIDController->ay, attitudePIDController->az);
//	 printf("Gyro: X=%.2fPI Y=%.2fPI Z=%.2fPI\n", attitudePIDController->gx, attitudePIDController->gy, attitudePIDController->gz);
//	 printf("Roll = %.2frad\tPitch = %.2frad\tYaw = %.2frad\n",attitudePIDController->Roll, attitudePIDController->Pitch, attitudePIDController->Yaw);
//	printf("Roll = %.2f°\tPitch = %.2f°\tYaw = %.2f°\n",attitudePIDController->Roll * RAD_TO_DEG, attitudePIDController->Pitch * RAD_TO_DEG, attitudePIDController->Yaw * RAD_TO_DEG);
	PIDAngleProc_Roll(&(attitudePIDController->rollPID),0.0f,attitudePIDController->Roll,attitudePIDController->gx);
	PIDAngleProc_Pitch(&(attitudePIDController->pitchPID),0.0f,attitudePIDController->Pitch,attitudePIDController->gx);
	PIDAngleProc_Yaw(&(attitudePIDController->yawPID),0.0f,attitudePIDController->gz);

	// 再打印 rollPID 的内外环数据
//	printf("外环目标值:%.2f\t外环反馈值:%.2f\t内环目标值:%.2f\t内环反馈值:%.2f\t内环输出值:%.2f\n",
//			0.0f, attitudePIDController->Roll,
//			attitudePIDController->rollPID.outer.output,
//			attitudePIDController->gx,
//			attitudePIDController->rollPID.inner.output);
}


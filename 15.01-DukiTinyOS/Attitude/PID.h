#ifndef __PID_H
#define __PID_H
#include "main.h"

//定义单层pid结构体
typedef struct _PID {
	float kp; //比例系数
	float ki; //积分系数
	float kd; //微分系数
	float error,lastError;//误差以及上次的误差
	float integral, maxIntegral; //积分、积分限幅
  float output, maxOutput; //输出、输出限幅
}PID;
//定义串级pid结构体
typedef struct _PIDCascade
{
    PID inner; //内环
    PID outer; //外环
    float output; //串级输出，等于inner.output
}PIDCascade;

void PID_Init(PID *pid, float kp, float ki, float kd, float maxIntegral, float maxOutput);
void PID_Calc(PID *pid, float reference, float feedback);
void PID_CascadeCalc(PIDCascade *pid, float outerRef, float outerFdb, float innerFdb);
void PID_Adjust(PID *pid, float kp, float ki, float kd);
void PIDAngleProc_Init(PIDCascade *rollPID,PIDCascade *pitchPID,PID *yawPID);
void PIDAngleProc_Roll(PIDCascade *rollPID,float rollRef, float rollFeedback, float rollRateFeedback);
void PIDAngleProc_Pitch(PIDCascade *pitchPID,float pitchRef, float pitchFeedback, float pitchRateFeedback);
void PIDAngleProc_Yaw(PID *yawPID, float yawRef, float yawFeedback);
#endif

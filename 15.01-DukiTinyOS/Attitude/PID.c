#include "PID.h"
#include "MySerial.h"

//初始化
void PID_Init(PID *pid, float kp, float ki, float kd, float maxIntegral, float maxOutput)
{
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->maxIntegral = maxIntegral;
    pid->maxOutput = maxOutput;
}

// 初始化每个角的PID控制器
void PIDAngleProc_Init(PIDCascade *rollPID,PIDCascade *pitchPID,PID *yawPID) {
    // 初始化Roll和Pitch的PID结构体（串级PID）
    PID_Init(&rollPID->outer, 1.0, 0.1, 0.01, 50.0, 100.0);  // Roll的PID参数
    PID_Init(&rollPID->inner, 0.8, 0.05, 0.005, 10.0, 100.0); // Roll的内环PID参数

    PID_Init(&pitchPID->outer, 1.0, 0.1, 0.01, 50.0, 100.0); // Pitch的PID参数
    PID_Init(&pitchPID->inner, 0.8, 0.05, 0.005, 10.0, 100.0); // Pitch的内环PID参数

    // 初始化Yaw的PID结构体（单环PID）
    PID_Init(yawPID, 1.0, 0.1, 0.01, 50.0, 100.0);   // Yaw的PID参数
}

// 增加动态调整PID参数的功能
void PID_Adjust(PID *pid, float kp, float ki, float kd) {
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
}
//进行一次pid计算
//参数为(pid结构体,目标值,反馈值)，计算结果放在pid结构体的output成员中
void PID_Calc(PID *pid, float reference, float feedback)
{
    //更新数据
    pid->lastError = pid->error; //将旧error存起来
    pid->error = reference - feedback; //计算新error
    //计算微分
    float dout = (pid->error - pid->lastError) * pid->kd;
    //计算比例
    float pout = pid->error * pid->kp;
    //计算积分
//    pid->integral += pid->error * pid->ki;
	  pid->integral = 0;
    //积分限幅
    if(pid->integral > pid->maxIntegral) pid->integral = pid->maxIntegral;
    else if(pid->integral < -pid->maxIntegral) pid->integral = -pid->maxIntegral;
    //计算输出
    pid->output = pout+dout + pid->integral;
    //输出限幅
    if(pid->output > pid->maxOutput) pid->output =   pid->maxOutput;
    else if(pid->output < -pid->maxOutput) pid->output = -pid->maxOutput;
}

//串级pid部分
//串级PID的计算函数
//参数(PID结构体,外环目标值,外环反馈值,内环反馈值)
void PID_CascadeCalc(PIDCascade *pid, float outerRef, float outerFdb, float innerFdb)
{
	  PID_Calc(&pid->outer, outerRef, outerFdb); //计算外环 输入为角度时，输出为角度取时间微分：即角速度
    PID_Calc(&pid->inner, pid->outer.output, innerFdb); //计算内环 输入为角速度时，输出为角速度取时间微分：即角加速度
		
    pid->output = pid->inner.output; //内环输出就是串级PID的输出
//	  printf("innder = %.2f,output = %.2f\n",pid->inner.output,pid->output);
}

// 处理Roll角度控制
//角度->角速度->角加速度
//参数(PID结构体,外环目标值(期望角度值),外环反馈值,内环反馈值)
void PIDAngleProc_Roll(PIDCascade *rollPID,float rollRef, float rollFeedback, float rollRateFeedback) {
    // 外环计算Roll角度的PID输出
    PID_CascadeCalc(rollPID, rollRef, rollFeedback, rollRateFeedback);
    // rollPID.output 包含了经过PID计算后的控制输出

//	  printf("外环目标值:%.2f\t外环反馈值:%.2f\t内环目标值:%.2f\t内环反馈值:%.2f\t内环输出值:%.2f\n",rollRef,rollFeedback,rollPID->outer.output,rollRateFeedback,rollPID->inner.output);
}

// 处理Pitch角度控制
//角度->角速度->角加速度
void PIDAngleProc_Pitch(PIDCascade *pitchPID,float pitchRef, float pitchFeedback, float pitchRateFeedback) {
    // 外环计算Pitch角度的PID输出
    PID_CascadeCalc(pitchPID, pitchRef, pitchFeedback, pitchRateFeedback);
    // pitchPID.output 包含了经过PID计算后的控制输出

//	  printf("外环目标值:%.2f\t外环反馈值:%.2f\t内环目标值:%.2f\t内环反馈值:%.2f\t内环输出值:%.2f\n",pitchRef,pitchFeedback,pitchPID->outer.output,pitchRateFeedback,pitchPID->inner.output);

}

// 处理Yaw角度控制 (单环PID)
//角速度->角加速度
//陀螺仪本身测得就是角速度，可以作为内环反馈值
void PIDAngleProc_Yaw(PID *yawPID, float yawRef, float yawFeedback) {
    // 计算Yaw角度的PID输出
    PID_Calc(yawPID, yawRef, yawFeedback);
    // yawPID.output 包含了经过PID计算后的控制输出

//	  printf("外环目标值:%.2f\t外环反馈值:%.2f\t外环输出值:%.2f\n",yawRef,yawFeedback,yawPID->output);

}
/*******************************************************
Copyright (C),E-EYE Tech. Co.,Ltd.
Author: WYF
Version:
Finish Date:
Description:
History: 
    1,Date:
      Author:
      Modification:
    2,
*******************************************************/
#ifndef _ERRORANDTIME_H_
#define _ERRORANDTIME_H_

//宏定义

//错误结构定义
typedef enum
{
    E_NO_ERR=0,                              //无错
    E_TTS_UART_OVERTIME,                     //TTS 串口超时
    E_TTS_UART_CMD_ERR,                      //错误帧
}E_ERROR;

//时钟结构
typedef struct
{
    u32 Counter;	                    //定时时间 （定时器为减定时）
    u32 Interval;	                    //重装载值
    u8  OverTimeF;                      //超时标志
}T_TIMER; 

//延时\定时器                        
typedef enum
{                 
TTS_POWERON_TIMER=0,//TTS上电延时
TTS_POWEROFF_TIMER, //TTS关电延时
TTS_UART_RECV_TIMER,//TTS串口接收定时
TTS_STATUS_TIMER,   //等待空闲定时   

SEARCH_FILE_TIMER,  //检索文件定时     

TIME_TEST_TIMER,
TIMER_SUM           //定时器数量
}E_CAM_TIME;

//时间任务结构
typedef struct{
    u8 TaskTimerState;       //task,1:valid;0:invalid
    u8 (*operate)(void);    //handle independtly funtion
}TIMES_TASK;

//时间任务ID定义
typedef enum 
{
    TIME_TTS=0,          
    TTS_SEND,
	TTS_UART_REC_TIMEOUT,
    MAX_TIMETASK_SUM,
}E_TIME_TASK;



//全局变量声明

//函数声明
void AddError(u8 ErrNo) ; //添加错误号
void TimerHandle(void);  //时钟处理函数
u8 IsOverTime(u8 Index); //判断是否超时
void StopTimer(u8 Index) ; //停止定时
u8 GetOverTimeStatus(u8 Index) ; //获得超时状态
void SetOnceTimer(u8 Index,u32 Value);//设置单次定时
void SetPeriodTimer(u8 Index,u32 CntValue, u32 ItvValue);
///FunctionalState TTSDelayTask(void) ; //周期任务,dxl
FunctionalState TTS_TimeTask(void) ;
void TTS_Init(void) ; //设备及全局变量初始化
void TimerTaskSch(void) ; //调度
void EnTimerTask(u8 TaskID) ; //启动任务
void DisTimerTask(u8 TaskID) ; //禁止任务

#endif

/**
  ******************************************************************************
  * @file    Record_Task.h
  * @author  
  * @version 
  * @date    
  * @brief  录音任务调度接口
  *               
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/

#ifndef __RECORD_TASK_H
#define __RECORD_TASK_H

/* Includes ------------------------------------------------------------------*/
#include "taskschedule.h"
#include "I2S_Record.h"

#ifdef __cplusplus
 extern "C" {
#endif 

//录音启动配置
typedef struct
{
  u8 StartFlag;            //启动录音标志,0-停止录音，1-正在录音
  u8 LoopFlag;             //暂时未启用，循环录音标志,0-未启用循环录音,1-启用循环录音
  u32 RecordTime;          //如果不是循环录音，则用该变量保存录音时长
  u32 RecordTimeSpace;     //已录音间隔,保存该间隔与RecordTime对比,好如果意外断电，也知道什么时候关闭它。
  u32 RecordID;            //多媒体ID,递增类型数据
  u32 RecordTimeMaxLen;    //录音最大长度  
  u16 resev;               //保留
}RecordDef;
   
#define RECORD_TASK_TIME    1                               //录音任务调度时间,50ms调度一次   
#define RECORD_TIME_TASK_TIME    5*SYSTICK_0p1SECOND        //定时录音关闭任务,  500ms
   
//任务处理函数 
FunctionalState RecordTask(void);

FunctionalState Record_TimeTask(void);

//初始化录音任务
void RecordTask_Init(void);

//启动任务
void RecordTask_Start(u32 AudioFreq,u16 maxTime);

//关闭任务
void RecordTask_Close(void);

//关闭任务-调度函数调用
FunctionalState Auto_RecordTask_Close(void);

//分割为5分钟一个文件
void RecordTask_Split(void);

//获取音频时间
u8  Audio_GetFileName(u8 *pBuffer);

//获取录音状态
u8 GetRecordFlag(void);

//获取已录音间隔
u32  GetRecordTime(void);

//写配置
void  RrcordWriteDef(void);

//读取配置
void  RrcordReadDef(void);

//启动定时任务
void  StartRecordTime(u32 RecordTime);

#ifdef __cplusplus
}
#endif   

#endif /*  */
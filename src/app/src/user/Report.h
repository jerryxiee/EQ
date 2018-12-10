#ifndef __REPORT_H
#define __REPORT_H

#include "stm32f2xx.h"

#define PRAM_REPORT_STRATEGY 1
#define PRAM_REPORT_SCHEME 2
#define PRAM_TEMP_TRACK_TIME 3
#define PRAM_TEMP_TRACK_COUNT 4
#define PRAM_EMERGENCY_REPORT_TIME 5
#define PRAM_EMERGENCY_REPORT_DISTANCE 6
#define PRAM_OVER_SPEED_REPORT_TIME 7
#define PRAM_OVER_TIME_REPORT_TIME 8
#define PRAM_ACC_ON_REPORT_TIME 9
#define PRAM_ACC_ON_REPORT_DISTANCE 10
#define PRAM_ACC_OFF_REPORT_TIME 11
#define PRAM_ACC_OFF_REPORT_DISTANCE 12
#define PRAM_UNLOGIN_REPORT_TIME 13
#define PRAM_UNLOGIN_REPORT_DISTANCE 14
#define PRAM_SECOND_LINK_ACC_ON_REPORT_TIME 15
#define PRAM_SECOND_LINK_ACC_OFF_REPORT_TIME 16
#define PRAM_SECOND_LINK_OPEN_FLAG 17

/**************************************************************************
//函数名：Report_TimeTask
//功能：实现位置汇报（包括实时跟踪、紧急报警、超速报警、超时报警、定时定距等位置汇报）
//输入：无
//输出：无
//返回值：始终是ENABLE
//备注：位置汇报定时任务，1秒钟进入1次，任务调度器需要调用此函数
***************************************************************************/
FunctionalState Report_TimeTask(void);
/**************************************************************************
//函数名：Report_UpdatePram
//功能：更新位置汇报所有变量
//输入：无
//输出：无
//返回值：无
//备注：主要是更新位置汇报模块所使用到的变量，运行任务调度之前需调用此函数
***************************************************************************/
void Report_UpdatePram(void);
/**************************************************************************
//函数名：Report_UpdateOnePram
//功能：更新某个参数
//输入：参数类型
//输出：该参数类型对应的变量
//返回值：无
//备注：串口或远程设置位置汇报相关参数时需要调用此函数
***************************************************************************/
void Report_UpdateOnePram(u8 type);
/**************************************************************************
//函数名：Report_CmdAck
//功能：位置上报应答处理
//输入：通道号
//输出：无
//返回值：无
//备注：收到位置上报的通用应答时需调用此函数
***************************************************************************/
void Report_CmdAck(u8 channel);
/**************************************************************************
//函数名：Report_UploadPositionInfo
//功能：上传一条位置信息
//输入：通道号
//输出：无
//返回值：0上传成功，1上传失败
//备注：当前的位置信息==位置基本信息+位置附加信息，位置信息最长为150字节
***************************************************************************/
u8 Report_UploadPositionInfo(u8 channel);
/**************************************************************************
//函数名：Report_GetPositionInfo
//功能：获取当前的位置信息
//输入：无
//输出：位置信息
//返回值：位置信息长度
//备注：当前的位置信息==位置基本信息+位置附加信息，位置信息最长为150字节
***************************************************************************/
u16 Report_GetPositionInfo(u8 *pBuffer);
/**************************************************************************
//函数名：Report_GetPositionBasicInfo
//功能：获取位置基本信息
//输入：无
//输出：位置基本信息
//返回值：位置基本信息长度
//备注：长度固定为28字节，位置基本信息：报警字、状态字、纬度、经度、高程、速度、方向、时间
***************************************************************************/
u16 Report_GetPositionBasicInfo(u8 *pBuffer);
/**************************************************************************
//函数名：Report_ClearReportTimeCount
//功能：上报时间计数变量清0
//输入：无
//输出：无
//返回值：无
//备注：两个连接的计数变量都清0
***************************************************************************/
void Report_ClearReportTimeCount(void);

#endif
#ifndef __SPEED_MONITOR_H
#define __SPEED_MONITOR_H

/************************文件包含***********************/
#include "stm32f2xx.h"
#include "stm32f2xx_conf.h"

typedef struct
{
    unsigned short GroupTime;               //组提示时间间隔(设定值)
    unsigned short GroupTimeCnt;            //组提示时间计数器
    unsigned short OnceInterval;            //单次提示时间间隔(设定值)
    unsigned char OnceNumber;               //提示单次重复播报的次数(设定值)
    unsigned char OnceCnt;                  //提示单次重复播报的计数器
}STPLAY_ATTRIB;//播报参数结构体

typedef struct
{
    unsigned long  ID;                  //区域或路段ID
    unsigned short GroupTime;           //组提示时间间隔(设定值)
    unsigned short OnceInterval;        //单次提示时间间隔(设定值)
    unsigned char OnceNumber;           //提示单次重复播报的次数(设定值)
    unsigned char Duration;             //持续时间设置值
    unsigned char SpeedLimit;           //速度限值
    char PlayVoice[30];                 //播报语音
}STSPEEDCHECK;//播报参数结构体
//////////////////////
#define STSPEEDCHECK_SIZE sizeof(STSPEEDCHECK)
//**************************宏定义************************
typedef enum
{
    E_SPEED_TYPE_GENERAL=0,      //限速类型:普通
    E_SPEED_TYPE_ROUND_AREA,     //限速类型:圆形区域
    E_SPEED_TYPE_RECT_AREA,      //限速类型:矩形区域
    E_SPEED_TYPE_POLYGON_AREA,   //限速类型:多边形区域
    E_SPEED_TYPE_ROUTE,          //限速类型:线路
    E_SPEED_TYPE_MAX
}E_REPORT_SPEED_TYPE;//限速类型
//********************自定义数据类型**************************

//***********************函数声明******************************
/*******************************************************************************
** 函数名称: SpeedMonitor_GetCurOverSpeedSubjoinInfo
** 功能描述: 取得当前超速附加信息包(包括附加信息ID、长度、附加信息体):即附加信息ID为0x11的所有内容
** 入口参数: 需要保存超速附加信息包的首地址
** 出口参数: 
** 返回参数: 超速附加信息包的总长度,0:表示未超速,
*******************************************************************************/
unsigned char SpeedMonitor_GetCurOverSpeedSubjoinInfo(unsigned char *data);
/*******************************************************************************
** 函数名称: SpeedMonitor_GetCurCarSpeedLimitSubjoinInfo
** 功能描述: 取得车辆限速值附加信息包(包括附加信息ID、长度、附加信息体) (索美要求 附加信息ID为0x6A的所有内容)
** 入口参数: 需要保存车辆限速值附加信息包的首地址
** 出口参数: 
** 返回参数: 车辆限速值附加信息包的总长度,
*******************************************************************************/
unsigned char SpeedMonitor_GetCurCarSpeedLimitSubjoinInfo(unsigned char *data);
/*************************************************************
** 函数名称: SpeedMonitor_TimeTask
** 功能描述: 速度监控的定时任务(1s调试一次)
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
FunctionalState SpeedMonitor_TimeTask(void);
/*************************************************************
** 函数名称: SpeedMonitor_UpdateNightParam
** 功能描述: 更新夜间行驶预警参数
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void SpeedMonitor_UpdateNightParam(void);
/*************************************************************
** 函数名称: SpeedMonitor_UpdateForbidParam
** 功能描述: 更新禁止行驶参数
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void SpeedMonitor_UpdateForbidParam(void);
/*************************************************************
** 函数名称: SpeedMonitor_UpDatePram
** 功能描述: 设置或修改超速相关变量后,需要更新超速模块相关变量(该模块内的所有参数)
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:SpeedMonitor_UpdateForbidParam,SpeedMonitor_UpdateNightParam
*************************************************************/
void SpeedMonitor_UpdatePram(void);
/*************************************************************
** 函数名称: SpeedMonitor_UpdateSpeedType
** 功能描述: 更新速度类型变量
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void SpeedMonitor_UpdateSpeedType(void);
/*************************************************************
** 函数名称: SpeedMonitor_UpdateMaxStopCarTime
** 功能描述: 更新最长停车时间
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void SpeedMonitor_UpdateMaxStopCarTime(void);
/*************************************************************
** 函数名称: SpeedMonitor_ParamInitialize
** 功能描述: 在开机时初始化
** 入口参数: 
** 出口参数: 
** 返回参数:  
** 全局变量: 无
** 调用模块:
*************************************************************/
void SpeedMonitor_ParamInitialize(void);
/*************************************************************
** 函数名称: SpeedMonitor_GetCurRunStatus
** 功能描述: 获取当前车辆的状态(行驶或停止)
** 入口参数: 
** 出口参数: 
** 返回参数: 1为行驶，0为停止
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned char SpeedMonitor_GetCurRunStatus(void);
/*************************************************************
** 函数名称: SpeedMonitor_GetCurSpeedType
** 功能描述: 获取速度类型
** 入口参数: 
** 出口参数: 
** 返回参数: 0为脉冲,1为GPS
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned char SpeedMonitor_GetCurSpeedType(void);
/*************************************************************
** 函数名称: SpeedMonitor_GetCurSpeed
** 功能描述: 获取瞬时速度(已考虑了速度类型，不需再判断速度类型了)
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned char SpeedMonitor_GetCurSpeed(void);
/*************************************************************
** 函数名称: SpeedMonitor_GetSpeedMaxLimitVal
** 功能描述: 获取限速值
** 入口参数: 
** 出口参数: 
** 返回参数: 限速值
** 全局变量: 无
** 调用模块:
*************************************************************/
u32 SpeedMonitor_GetSpeedMaxLimitVal(void);
/*************************************************************
** 函数名称: SpeedMonitor_GetEarlyAlarmDVal
** 功能描述: 获取速度预警差值
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned char SpeedMonitor_GetEarlyAlarmDVal(void);
/*************************************************************
** 函数名称: SpeedMonitor_GetCurAtNightFlag
** 功能描述: 取得是否为夜间时段内标志
** 入口参数: 
** 出口参数: 
** 返回参数: 1:为夜间时间段;0:非夜间时间段
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned char SpeedMonitor_GetCurAtNightFlag(void);
/*************************************************************
** 函数名称: SpeedMonitor_GetNeedCheckNightFlag
** 功能描述: 取得是否需要判断夜间时段标志
** 入口参数: 
** 出口参数: 
** 返回参数: 1:需要判断;0:不需要判断
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned char SpeedMonitor_GetNeedCheckNightFlag(void);
/*************************************************************
** 函数名称: SpeedMonitor_CheckPlayFlag
** 功能描述: 查检是否需要播报
** 入口参数: VaildFlag检查有效标志,stPlay播报参数结构体
** 出口参数: 
** 返回参数: 1:需要播报,0:不需要播报
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned char SpeedMonitor_CheckPlayFlag(unsigned char VaildFlag,STPLAY_ATTRIB *stPlay);
/*************************************************************
** 函数名称: SpeedMonitor_SetSpeedParamValid
** 功能描述: 设置某限速类型为有效
** 入口参数: type,限速类型,ID:区域ID或线路ID,speedLimit限速值,duration持续时间
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void SpeedMonitor_SetSpeedParamValid(unsigned char type,unsigned long ID,unsigned char speedLimit,unsigned char duration);
/*************************************************************
** 函数名称: SpeedMonitor_ClearSpeedParamValid
** 功能描述: 设置某限速类型为无效
** 入口参数: type,限速类型
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void SpeedMonitor_ClearSpeedParamValid(unsigned char type);
/*************************************************************
** 函数名称: SpeedMonitor_SetSpeedParamPlay
** 功能描述: 设置某限速类型的播报参数
** 入口参数: type,限速类型,GroupTime组提示时间间隔;OnceInterval单次提示时间间隔;OnceNumber提示单次重复播报的次数
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void SpeedMonitor_SetSpeedParamPlay(unsigned char type,unsigned short GroupTime,unsigned short OnceInterval,unsigned char OnceNumber);
/*************************************************************
** 函数名称: SpeedMonitor_SetSpeedParamPlayVoice
** 功能描述: 设置某限速类型的播报语音
** 入口参数: type,限速类型,playVoice超速播报语音
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void SpeedMonitor_SetSpeedParamPlayVoice(unsigned char type,char *playVoice);
/*************************************************************
** 函数名称: SpeedMonitor_GetCurMinSpeedType
** 功能描述: 获取当前的最小速度的类型
** 入口参数: 
** 出口参数: 
** 返回参数: 0,普通;1:圆形;2:矩形;3:多边形;4:线路
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned char SpeedMonitor_GetCurMinSpeedType(void);
/*************************************************************
** 函数名称: SpeedMonitor_GetNeedReportFlag
** 功能描述: 获取是否需要上报位置信息标志
** 入口参数: 
** 出口参数: 
** 返回参数: 0:不需要上报,1:需要上报
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned char SpeedMonitor_GetNeedReportFlag(void);
/*************************************************************
** 函数名称: SpeedMonitor_GetOverSpeedStatusForReport
** 功能描述: 获取是否根据超速状态来上报位置信息
** 入口参数: 
** 出口参数: 
** 返回参数: 0:不根据超速状态来上报, 1:根据超速状态来上报
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned char SpeedMonitor_GetOverSpeedStatusForReport(void);
/*************************************************************
** 函数名称: SpeedMonitor_GetReportFreqEvent
** 功能描述: 获取上报时间间隔的事件项
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned long SpeedMonitor_GetReportFreqEvent(void);
/*************************************************************
** 函数名称: SpeedMonitor_SetPreLimitSpeedParam
** 功能描述: 设置预警限速值(只有速度类型与当前限速类型一致时才有效)
** 入口参数: type速度类型,preSpeedLimit限速值,duration持续时间,saveflag:是否需要保存,0不需要保存,1:需要保存
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void SpeedMonitor_SetPreLimitSpeedParam(unsigned char type,unsigned char preSpeedLimit,unsigned char duration,unsigned char saveflag);
/*************************************************************
** 函数名称: SpeedMonitor_ReadSpeedRecordData
** 功能描述: 读取指定时间段、指定最大数据块速度记录数据
** 入口参数: 起始时间StartTime,结束时间EndTime,MaxBlock指定最大数据块
** 出口参数: pBuffer保存数据的首地址
** 返回参数: pBuffer字节长度
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned short SpeedMonitor_ReadSpeedRecordData(u8 *pBuffer, TIME_T StartTime, TIME_T EndTime, u16 MaxBlock);
#endif

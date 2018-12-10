/********************************************************************
//版权说明	:Shenzhen E-EYE Co. Ltd., Copyright 2009- All Rights Reserved.
//文件名称	:speedmonitor.c		
//功能		:实现速度监控功能
//版本号	:
//开发人	:dxl
//开发时间	:2010.4
//修改者	:
//修改时间	:
//修改简要说明	:
//备注		:
***********************************************************************/

//********************************头文件************************************
#include "include.h"
//********************************自定义数据类型****************************
//********************************宏定义************************************
#ifndef FRAM_SPEED_PLAY_ADDR
#define FRAM_SPEED_PLAY_ADDR    FRAM_TEXI_SERVICE_ADDR
#endif
/////////////////////////////////////////////
#define OVERSPEED_OVERHEAD_INFO_ID              0x11 //超速附加信息ID
#define CAR_SPEED_LIMIT_OVERHEAD_INFO_ID        0x6A //车辆限速值附加信息ID
#define CAR_SPEED_LIMIT_OVERHEAD_INFO_LEN       0x02;//车辆限速值附加信息长度
///////////////////////////////
#define ONE_SPEED_MONITOR_DATA_LEN              (REGISTER_SPEED_STEP_LEN-5)//4个字节的时间+1个字节的检验,每分钟的行驶数据长度

#define ONCE_MIN_PLAY_TIME                      5//单次播报最小间隔
#define ONCE_DEFAULT_PLAY_TIME                  10//单次播报默认10秒
//////////////////////////////////

//********************************全局变量**********************************

//********************************本地变量**********************************
typedef struct
{
    unsigned char speed;         //速度
    unsigned char status;        //状态
}STRUNRECORD;

typedef struct
{
    STPLAY_ATTRIB stPlay;                   //超速播报
    STPLAY_ATTRIB stPrePlay;                //预警播报
    /////////////////////////    
    unsigned long startStopTime;            //开始停车时间
    unsigned long OverstopTime;             //超时停车时间  
    ////////////////////////////////////
    unsigned short EarlyAlarmDVal;          //预警差值
    unsigned short overSpeedCnt;            //检查是否超速计数器 
    unsigned short speedDuration;           //超速持续时间设置值
    unsigned short preOverSpeedCnt;         //预警速度计数器
    unsigned short prespeedDuration;        //预超速持续时间设置值
    //////////////////////////////
    unsigned long ReportFreqEvent;          //上报时间间隔的事件项
    unsigned short ReportInterval;          //上报时间间隔
    unsigned short ReportTimeCnt;           //上报时间计数器
    ///////////////////////////////////////////////////////////////
    unsigned char curSpeedLimit;            //当前超速限值
    unsigned char curpreSpeedLimit;         //当前超速预警限值
    unsigned char curprespeedDuration;      //预超速持续时间设置值
    unsigned char SpeedType;                //速度类型,0为脉冲,1为GPS 
    unsigned char OverStopTimeFlag;         //超时停车上报标志
    unsigned char curSpeed;                 //当前速度
    unsigned char preSpeed;                 //前一次速度
    unsigned char runStatusCnt;             //行驶状态计数器
    unsigned char stopStatusCnt;            //停止状态计数器
    unsigned char runStatus;                //1为行驶，0为停驶
    unsigned char prerunStatus;             //前一次的行驶状态
    unsigned char needReportflag;           //是否需要上报
    unsigned char recordCnt;                //行驶记录暂存计数器
    STRUNRECORD stRecord[12];               //行驶记录暂存
}STSPEED_ATTRIB;

static STSPEED_ATTRIB s_stSpeedAttrib;

typedef struct
{
    STSPEEDCHECK stSpType[E_SPEED_TYPE_MAX]; //
    unsigned char speedLimit;               //当前超速值
    unsigned char nightspeedLimit;          //当前晚上超速值
    unsigned char valid;                    //有效标志
    unsigned char type;                     //当前使用的超速类型
    unsigned char playflag;                 //当前正在播报
}STSPEEDPARAM;

static STSPEEDPARAM s_stSpeedParam;

typedef struct
{
    STPLAY_ATTRIB stPlay;               //夜间行驶预警播报
    unsigned long earlyTime;            //夜间预警时间
    unsigned long startVal;             //夜间开始时间
    unsigned long endVal;               //夜间结束时间
    unsigned short control;             //夜间行驶时间控制域
    unsigned char percent;              //夜间速度百分比
    unsigned char needChecknight;       //需要检查晚上超时
}STSPEED_NIGHT;
static STSPEED_NIGHT s_stSpNight;

typedef struct
{
    STPLAY_ATTRIB stPlay;               //禁止行驶播报
    STPLAY_ATTRIB stPrePlay;            //禁止行驶预警播报
    unsigned long earlyTime;            //禁止行驶预警时间
    unsigned long startVal;             //禁止开始时间(相对开始时间)
    unsigned long endVal;               //禁止结束时间
    unsigned short control;             //禁止行驶时间控制域
    unsigned short runDuration;         //禁止行驶进入行驶状态的持续时间
    unsigned short runStatusCnt;        //禁止行驶行驶状态计数器
    unsigned short stopStatusCnt;       //禁止行驶停止状态计数器
    unsigned char runStatus;            //禁止行驶1为行驶，0为停驶
    unsigned char runSpeed;             //禁止行驶进入行驶状态的速度 
    unsigned char CheckVaild;           //是否需要检查
}STSPEED_FORBIDTIME;
static STSPEED_FORBIDTIME s_stSpForbid;
typedef struct
{
    unsigned char playSpeed;          //播报速度
    unsigned char atnightflag;        //0空,1白天,2夜间
    unsigned char reserve[6];
}STSPEED_PLAY;
static STSPEED_PLAY s_stSpeedPlay;

////////////////////////////////
typedef struct
{
    unsigned short head;                //头标志
    unsigned char saveflag;             //存储标志
    unsigned char count;                //计数器
    unsigned long startTime;            //开始时间
    STRUNRECORD stRecord[60];           //速度、状态
}STSPEEDSAVERECORD;
static STSPEEDSAVERECORD s_stSpSaveRecord;

#define  STSPEEDSAVERECORD_SIZE sizeof(STSPEEDSAVERECORD)
///////////////////////////
#define RUN_CHAGE_STATUS_TIME 10//改变状态持续的时间10s
///////////////////////////////////
typedef enum
{
    CAR_STATUS_EMPTY,   //空
    CAR_STATUS_STOP,    //停驶
    CAR_STATUS_RUN,     //行驶
    CAR_STATUS_MAX
}CAR_STATUS;//车辆行驶状态

enum 
{
    SPEED_TIMER_TASK,                       //
    SPEED_TIMER_CHECK,						// 
    SPEED_TIMER_PLAY,						// 
    SPEED_TIMER_TEST,						// 
    SPEED_TIMER_FORBID,						// 
    SPEED_TIMERS_MAX
}STSPEED_TIME;
static LZM_TIMER s_stSpeedTimer[SPEED_TIMERS_MAX];
//********************************函数声明**********************************
void SpeedMonitor_ClearPlayFlag(void);
void SpeedMonitor_UpdateCurSpeedLimitParam(void);
//********************************函数定义***********************************
/*******************************************************************************
** 函数名称: SpeedMonitor_GetCurOverSpeedSubjoinInfo
** 功能描述: 取得当前超速附加信息包(包括附加信息ID、长度、附加信息体) :即附加信息ID为0x11的所有内容
** 入口参数: 需要保存超速附加信息包的首地址
** 出口参数: 
** 返回参数: 超速附加信息包的总长度,0:表示未超速,
*******************************************************************************/
unsigned char SpeedMonitor_GetCurOverSpeedSubjoinInfo(unsigned char *data)
{
    unsigned char buffer[20]={0};
    unsigned char len;
    if(0==Io_ReadAlarmBit(ALARM_BIT_OVER_SPEED))return 0;
    len = 0;
    buffer[len++] = OVERSPEED_OVERHEAD_INFO_ID;
    if(E_SPEED_TYPE_GENERAL==s_stSpeedParam.type)
    {
        buffer[len++] = 1;//长度
        buffer[len++] = 0;//位置类型
    }
    else
    {
        buffer[len++] = 5;//长度
        buffer[len++] = s_stSpeedParam.type;//位置类型
        Public_ConvertLongToBuffer(s_stSpeedParam.stSpType[s_stSpeedParam.type].ID,&buffer[len]);        
        len += 4;
    }
    ///////////////
    memcpy(data,buffer,len);
    /////////////////
    return len;
}
/*******************************************************************************
** 函数名称: SpeedMonitor_GetCurCarSpeedLimitSubjoinInfo
** 功能描述: 取得车辆限速值附加信息包(包括附加信息ID、长度、附加信息体) (索美要求 附加信息ID为0x6A的所有内容)
** 入口参数: 需要保存车辆限速值附加信息包的首地址
** 出口参数: 
** 返回参数: 车辆限速值附加信息包的总长度,
*******************************************************************************/
unsigned char SpeedMonitor_GetCurCarSpeedLimitSubjoinInfo(unsigned char *data)
{
   
    return 0;
}
/*************************************************************
** 函数名称: SpeedMonitor_GetLastStopTime
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量:
** 调用模块:
*************************************************************/
unsigned long SpeedMonitor_GetLastStopTime(void)
{
    unsigned char buffer[10];
    TIME_T stTime;
    if(Public_ReadDataFromFRAM(FRAM_STOPING_TIME_ADDR,buffer,6))
    {
        if(Public_ConvertBCDToTime(&stTime,buffer))
        {
            return ConverseGmtime(&stTime);
        }
    }
    return 0;
}
/*************************************************************
** 函数名称: SpeedMonitor_OverSpeedPreAlarmVoice
** 功能描述: 播报预超速语音
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量:
** 调用模块:
*************************************************************/
void SpeedMonitor_OverSpeedPreAlarmVoice(void)
{
    if(!Public_PlayE2ParamAlarmVoice(E2_OVERSPEED_PREALARM_VOICE_ID))
    {
        Public_PlayTTSVoiceAlarmStr("请控制车速,谨慎驾驶");
    }
}
/*************************************************************
** 函数名称: SpeedMonitor_OverSpeedAlarmVoice
** 功能描述: 播报超速语音
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量:
** 调用模块:
*************************************************************/
void SpeedMonitor_OverSpeedAlarmVoice(void)
{
    Public_PlayTTSVoiceAlarmStr((unsigned char *)s_stSpeedParam.stSpType[s_stSpeedParam.type].PlayVoice);
}
/*************************************************************
** 函数名称: SpeedMonitor_NightPreAlarmVoice
** 功能描述: 夜间预警语音
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量:
** 调用模块:
*************************************************************/
void SpeedMonitor_NightPreAlarmVoice(void)
{
    if(!Public_PlayE2ParamAlarmVoice(E2_NIGHT_DRIVE_PREALARM_VOICE_ID))
    {
        Public_PlayTTSVoiceAlarmStr("即将进入夜间时段");
    }
}
/*************************************************************
** 函数名称: SpeedMonitor_ForbidPreAlarmVoice
** 功能描述: 夜间禁行预警语音
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量:
** 调用模块:
*************************************************************/
void SpeedMonitor_ForbidPreAlarmVoice(void)
{
    if(!Public_PlayE2ParamAlarmVoice(E2_FORBID_DRIVE_PREALARM_VOICE_ID))
    {
        Public_PlayTTSVoiceAlarmStr("请按规定时间停车休息");
    }
}
/*************************************************************
** 函数名称: SpeedMonitor_ForbidAlarmVoice
** 功能描述: 夜间禁行语音
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量:
** 调用模块:
*************************************************************/
void SpeedMonitor_ForbidAlarmVoice(void)
{
    if(!Public_PlayE2ParamAlarmVoice(E2_FORBID_DRIVE_ALARM_VOICE_ID))
    {
        Public_PlayTTSVoiceAlarmStr("请按规定时间停车休息");
    }    
}
/*************************************************************
** 函数名称: SpeedMonitor_UpdateNightSpeedLimit
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void SpeedMonitor_UpdateNightSpeedLimit(void)
{
    if(!SpeedMonitor_GetNeedCheckNightFlag())return;
    if(!SpeedMonitor_GetCurAtNightFlag())return;    
    s_stSpeedParam.nightspeedLimit = (unsigned char)(s_stSpeedParam.speedLimit*s_stSpNight.percent*0.01+0.5);
    s_stSpeedAttrib.curSpeedLimit = s_stSpeedParam.nightspeedLimit;
}
/*************************************************************
** 函数名称: SpeedMonitor_GetCurAtNightFlag
** 功能描述: 取得是否为夜间时段内标志
** 入口参数: 
** 出口参数: 
** 返回参数: 1:为夜间时间段;0:非夜间时间段
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned char SpeedMonitor_GetCurAtNightFlag(void)
{
    return (2 == s_stSpeedPlay.atnightflag)?1:0;
}
/*************************************************************
** 函数名称: SpeedMonitor_GetNeedCheckNightFlag
** 功能描述: 取得是否需要判断夜间时段标志
** 入口参数: 
** 出口参数: 
** 返回参数: 1:需要判断;0:不需要判断
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned char SpeedMonitor_GetNeedCheckNightFlag(void)
{
    return (s_stSpNight.needChecknight)?1:0;
}
/*************************************************************
** 函数名称: SpeedMonitor_ReadSpeedPlay
** 功能描述: 
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void SpeedMonitor_ReadSpeedPlay(void) 
{
    unsigned long timeVal,stopTime;    
    if(Public_ReadDataFromFRAM(FRAM_SPEED_PLAY_ADDR,(unsigned char *)&s_stSpeedPlay, sizeof(s_stSpeedPlay))) 
    {
        timeVal=RTC_GetCounter();
        stopTime = s_stSpeedAttrib.startStopTime;
        if(!SpeedMonitor_GetNeedCheckNightFlag()||(timeVal>stopTime+DAY_SECOND&&stopTime)
            ||(stopTime>timeVal+DAY_SECOND&&stopTime))
        {
            s_stSpeedPlay.atnightflag = 0;
        }
    }
    else
    {
        memset(&s_stSpeedPlay,0,sizeof(s_stSpeedPlay));
    }
}
/*************************************************************
** 函数名称: SpeedMonitor_SaveStopStartTime
** 功能描述: 
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void SpeedMonitor_SaveSpeedPlay(void) 
{
    Public_WriteDataToFRAM(FRAM_SPEED_PLAY_ADDR,(unsigned char *)&s_stSpeedPlay, sizeof(s_stSpeedPlay));
}
/*************************************************************
** 函数名称: SpeedMonitor_CheckPlayAndSaveNightFlag
** 功能描述: 判断并保存参数
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void SpeedMonitor_CheckPlayAndSaveNightFlag(unsigned char flag)
{
    if(flag==s_stSpeedPlay.atnightflag)return; 
    ////////////////////////////    
    if(SpeedMonitor_GetCurRunStatus()&&s_stSpeedPlay.atnightflag)
    {
        if(2==flag)//夜间
        {
            Public_PlayTTSVoiceAlarmStr("夜间行驶,请注意控制车速");
        }
        else //白天
        {
            Public_PlayTTSVoiceAlarmStr("白天行驶,请注意控制车速"); 
        }
        s_stSpeedParam.playflag = 2;
        LZM_PublicSetOnceTimer(&s_stSpeedTimer[SPEED_TIMER_TEST],PUBLICSECS(8),SpeedMonitor_ClearPlayFlag);
    }
    ///////////////////////////////
    s_stSpeedPlay.atnightflag = flag;
    SpeedMonitor_SaveSpeedPlay();
    SpeedMonitor_UpdateCurSpeedLimitParam();    
}
/*************************************************************
** 函数名称: SpeedMonitor_CheckIsAtNightFlag
** 功能描述: 判断是否属于夜间时间段
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void SpeedMonitor_CheckIsAtNightFlag(void)
{
    unsigned char flag;
    if(!SpeedMonitor_GetNeedCheckNightFlag())return;  
    ////////夜间时间段//////////
    if(Public_CheckCurTimeIsInTimeRange(s_stSpNight.control,s_stSpNight.startVal,s_stSpNight.endVal))
    {
        SpeedMonitor_CheckPlayAndSaveNightFlag(2);
    }
    else//非夜间时间段
    {
        flag = 0;
        SpeedMonitor_CheckPlayAndSaveNightFlag(1);
        /////////夜间预警时间段//////////////        
        if(Public_CheckCurTimeIsInEarlyAlarmTimeRange(s_stSpNight.control,s_stSpNight.earlyTime,s_stSpNight.startVal,s_stSpNight.endVal))
        {
            flag =1;
        }
        /////////////////////////////////
        if(SpeedMonitor_CheckPlayFlag(flag,&s_stSpNight.stPlay))
        {
            SpeedMonitor_NightPreAlarmVoice();
        }
    }    
}
/*************************************************************
** 函数名称: SpeedMonitor_SaveStopStartTime
** 功能描述: 保存停车开始时间
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void SpeedMonitor_SaveStopStartTime(void) 
{
    Public_WriteDataToFRAM(FRAM_STOP_TIME_ADDR,(unsigned char *)&s_stSpeedAttrib.startStopTime, 4);
}
/*************************************************************
** 函数名称: SpeedMonitor_ReadStopStartTime
** 功能描述: 读取停车开始时间
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void SpeedMonitor_ReadStopStartTime(void) 
{
    unsigned long val;
    if(Public_ReadDataFromFRAM(FRAM_STOP_TIME_ADDR,(unsigned char *)&val, 4)) 
    {
        s_stSpeedAttrib.startStopTime = val;
    }
    val = SpeedMonitor_GetLastStopTime();
    ////////////////////////////////////////////
    if(0==s_stSpeedAttrib.startStopTime||(s_stSpeedAttrib.startStopTime>val&&val))
    {
        s_stSpeedAttrib.startStopTime = val;
        SpeedMonitor_SaveStopStartTime();
    }
}
/*************************************************************
** 函数名称: SpeedMonitor_GetCurRunStatus
** 功能描述: 获取当前车辆的状态(行驶或停止)
** 入口参数: 
** 出口参数: 
** 返回参数: 1为行驶，0为停止
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned char SpeedMonitor_GetCurRunStatus(void)
{
    return(CAR_STATUS_RUN==s_stSpeedAttrib.runStatus)?1:0;
}
/*************************************************************
** 函数名称: SpeedMonitor_CheckRunStatus
** 功能描述: 判断当前是处于行驶状态还是停驶状态(1s调试一次)
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void SpeedMonitor_CheckRunStatus(unsigned char CurrentSpeed)
{
    //unsigned char ACC;	
    //ACC = Io_ReadStatusBit(STATUS_BIT_ACC);	
    if(CurrentSpeed > 0)//
    {
        s_stSpeedAttrib.stopStatusCnt = 0;//停止状态计数器清0
        if(s_stSpeedAttrib.runStatusCnt < RUN_CHAGE_STATUS_TIME)//是否持续够10s,不够时,不改变当前状态
        {
            s_stSpeedAttrib.runStatusCnt++;//行驶状态计数器++
        }
        else//已经持续10s,则设为行驶状态
        {
            s_stSpeedAttrib.runStatus = CAR_STATUS_RUN;
        }
    }
    else
    {
        s_stSpeedAttrib.runStatusCnt = 0;//行驶状态计数器清0
        if(s_stSpeedAttrib.stopStatusCnt < RUN_CHAGE_STATUS_TIME)//是否持续够10s,不够时,不改变当前状态
        {
            s_stSpeedAttrib.stopStatusCnt++;//停止状态计数器++
        }
        else//已经持续10s,则设为停止状态
        {
            s_stSpeedAttrib.runStatus = CAR_STATUS_STOP;
        }
    }
}
/*************************************************************
** 函数名称: SpeedMonitor_CheckStopRunStatus
** 功能描述: 判断当前是处于行驶状态还是停驶状态(1s调试一次)
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void SpeedMonitor_CheckForbidRunStatus(unsigned char CurrentSpeed)
{
    unsigned char ACC;
    ///////////////////////////
    ACC = Io_ReadStatusBit(STATUS_BIT_ACC);
    if((CurrentSpeed >=s_stSpForbid.runSpeed)&&(1 == ACC))//
    {
        s_stSpForbid.stopStatusCnt = 0;//停止状态计数器清0
        if(s_stSpForbid.runStatusCnt < s_stSpForbid.runDuration)//是否持续够10s,不够时,不改变当前状态
        {
            s_stSpForbid.runStatusCnt++;//行驶状态计数器++
        }
        else//已经持续10s,则设为行驶状态
        {
            s_stSpForbid.runStatus = CAR_STATUS_RUN;
        }
    }
    else//只要速度一低于设定值,就算停车???
    {
        s_stSpForbid.runStatusCnt = 0;//行驶状态计数器清0
        s_stSpForbid.runStatus = CAR_STATUS_STOP;
    }
}
/*************************************************************
** 函数名称: SpeedMonitor_ClearRunRecordData
** 功能描述: 
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void SpeedMonitor_ClearRunRecordData(void)
{
    s_stSpSaveRecord.count = 0;
    s_stSpSaveRecord.startTime = 0;
    memset(&s_stSpSaveRecord.stRecord,0xff,120);
}
/*************************************************************
** 函数名称: SpeedMonitor_SaveRecordDataToFlash
** 功能描述: 
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void SpeedMonitor_SaveRunRecordDataToFRAM(void)
{
    #if (ONE_SPEED_MONITOR_DATA_LEN != 6)
    s_stSpSaveRecord.head = 0xacbd;
    s_stSpSaveRecord.saveflag = 0x55;
    Public_WriteDataToFRAM(FRAM_SPEED_RUN_RECORD_ADDR,(unsigned char *)&s_stSpSaveRecord,STSPEEDSAVERECORD_SIZE);
    #endif
}
/*************************************************************
** 函数名称: SpeedMonitor_SaveRecordDataToFlash
** 功能描述: 
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void SpeedMonitor_ReadRunRecordData(void)
{
    #if (ONE_SPEED_MONITOR_DATA_LEN != 6)
    unsigned char len;
    len=Public_ReadDataFromFRAM(FRAM_SPEED_RUN_RECORD_ADDR,(unsigned char *)&s_stSpSaveRecord,STSPEEDSAVERECORD_SIZE);
    if((len != STSPEEDSAVERECORD_SIZE)
      ||(s_stSpSaveRecord.head != 0xacbd)
      ||(s_stSpSaveRecord.saveflag!=0x55))
    {
        SpeedMonitor_ClearRunRecordData();
    }
    #endif
}
/*************************************************************
** 函数名称: SpeedMonitor_SaveRecordDataToFlash
** 功能描述: 
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void SpeedMonitor_SaveRecordDataToFlash(void)
{
    #if (ONE_SPEED_MONITOR_DATA_LEN != 6)
    TIME_T stTime;
    unsigned char buffer[ONE_SPEED_MONITOR_DATA_LEN+1]={0};
    Gmtime(&stTime,s_stSpSaveRecord.startTime);
    Public_ConvertTimeToBCDEx(stTime,buffer);
    memcpy(&buffer[6],s_stSpSaveRecord.stRecord,120);
    if(ERROR ==Register_Write2(REGISTER_TYPE_SPEED,buffer,ONE_SPEED_MONITOR_DATA_LEN,s_stSpSaveRecord.startTime))
		{
		//flash出错了
        Register_EraseOneArea(REGISTER_TYPE_SPEED);
    }
    SpeedMonitor_ClearRunRecordData();
    #endif
}
/*************************************************************
** 函数名称: SpeedMonitor_SaveOneRecordData
** 功能描述: 
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void SpeedMonitor_SaveOneRecordData(STRUNRECORD stRecord,u32 curTime)
{
    #if (ONE_SPEED_MONITOR_DATA_LEN != 6)    
    unsigned char sec;
    sec = curTime%60;
    if(0==s_stSpSaveRecord.startTime)//
    {
        s_stSpSaveRecord.startTime = curTime-sec;
    }
    else
    if((curTime<s_stSpSaveRecord.startTime)||(curTime>=(s_stSpSaveRecord.startTime+60)))
    {
        SpeedMonitor_SaveRecordDataToFlash();
        s_stSpSaveRecord.startTime = curTime-sec;
    }
    /////////////////////
    s_stSpSaveRecord.stRecord[sec] = stRecord;
    s_stSpSaveRecord.count++;
    //////////////////////////////////
    SpeedMonitor_SaveRunRecordDataToFRAM();
    #else
    unsigned char buffer[ONE_SPEED_MONITOR_DATA_LEN]={0xff};
    buffer[0] = stRecord.speed;
    buffer[1] = stRecord.status;
    Register_Write2(REGISTER_TYPE_SPEED,buffer,ONE_SPEED_MONITOR_DATA_LEN,curTime);  
    #endif
}
/*************************************************************
** 函数名称: SpeedMonitor_SaveRunRecordData
** 功能描述: 保存行驶记录(以1s的时间间隔持续记录并存储车辆行驶状态数据。该行驶状态数据为：车辆在行驶过程中的实时时间、每秒钟间隔内对应的平均速度以及对应时间的状态信号)
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void SpeedMonitor_SavePackRecordDataToFRAM(unsigned char RecordNum, u32 curTime)
{
    unsigned char i;
    unsigned long timeVal;
    timeVal =curTime-RecordNum;
    for(i = 0; i<RecordNum; i++)
    {
        SpeedMonitor_SaveOneRecordData(s_stSpeedAttrib.stRecord[i],timeVal+i);
    }
}
/*************************************************************
** 函数名称: SpeedMonitor_SaveRecordDataToFlash
** 功能描述: 
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void SpeedMonitor_CheckRecordDataToSaveFlash(void)
{
    #if (ONE_SPEED_MONITOR_DATA_LEN != 6)
    unsigned long curTime;
    if(0==s_stSpSaveRecord.count||0 == s_stSpSaveRecord.startTime)return;
    curTime = RTC_GetCounter();
    if(s_stSpSaveRecord.startTime<curTime&&curTime<s_stSpSaveRecord.startTime+60)return;//一分钟之内不处理
    if(s_stSpeedAttrib.recordCnt)return;//有没有暂存的
    /////////////////////////////////
    SpeedMonitor_SaveRecordDataToFlash();
    /////////////////////////////////
    SpeedMonitor_SaveRunRecordDataToFRAM();
    #endif
}
/*************************************************************
** 函数名称: SpeedMonitor_SaveRunRecordData
** 功能描述: 保存行驶记录(以1s的时间间隔持续记录并存储车辆行驶状态数据。该行驶状态数据为：车辆在行驶过程中的实时时间、每秒钟间隔内对应的平均速度以及对应时间的状态信号)
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void SpeedMonitor_SaveRunRecordData(unsigned char CurrentSpeed)
{
    unsigned char saveflag;
    unsigned char temp;
    STRUNRECORD stRunRecord;
    saveflag = 0;    
    ///////////////////////
    stRunRecord.speed = CurrentSpeed;
    //////////////////////////
    temp = (Io_ReadExtCarStatus() & 0xff);
    temp <<= 3;//把低位往高移动3位
    stRunRecord.status = temp;
    //////添加自定义//////add by joneming//////
    /////Acc///////////
    if(Io_ReadStatusBit(STATUS_BIT_ACC))
    {
        PUBLIC_SETBIT(stRunRecord.status, 0);
    }
    /////////车门//////////////
    if(Io_ReadStatusBit(STATUS_BIT_DOOR1))
    {
        PUBLIC_SETBIT(stRunRecord.status, 1);
    }
    /////////车喇叭///////////////
    if(Io_ReadExtCarStatusBit(CAR_STATUS_BIT_HORN))
    {
        PUBLIC_SETBIT(stRunRecord.status, 2);
    }    
    ////////////////////////////
    if((CAR_STATUS_STOP==s_stSpeedAttrib.prerunStatus)&&(CAR_STATUS_RUN==s_stSpeedAttrib.runStatus))//由停驶到行驶
    {
        ///////需要转保/////////
        if(s_stSpeedAttrib.recordCnt)//
        {
            SpeedMonitor_SavePackRecordDataToFRAM(s_stSpeedAttrib.recordCnt,RTC_GetCounter());
        }
        s_stSpeedAttrib.recordCnt = 0;
        saveflag = 1;
    }
    else
    if((CAR_STATUS_RUN==s_stSpeedAttrib.prerunStatus)&&(CAR_STATUS_STOP==s_stSpeedAttrib.runStatus))//由行驶到停驶
    {
        ///////不需要保存////////////
        s_stSpeedAttrib.recordCnt = 0;
    }
    else
    if((CAR_STATUS_STOP==s_stSpeedAttrib.prerunStatus)&&(CAR_STATUS_STOP==s_stSpeedAttrib.runStatus))//停驶状态
    {
        if(0 == CurrentSpeed)//
        {
            s_stSpeedAttrib.recordCnt = 0;
        }
        else
        if(CurrentSpeed)////需要暂存
        {
            s_stSpeedAttrib.stRecord[s_stSpeedAttrib.recordCnt] = stRunRecord;
            s_stSpeedAttrib.recordCnt++;
        }
    }
    else
    if((CAR_STATUS_RUN==s_stSpeedAttrib.prerunStatus)&&(CAR_STATUS_RUN==s_stSpeedAttrib.runStatus))//行驶状态
    {
        if(0==CurrentSpeed&&0==s_stSpeedAttrib.preSpeed)////需要暂存
        {
            s_stSpeedAttrib.stRecord[s_stSpeedAttrib.recordCnt] = stRunRecord;
            s_stSpeedAttrib.recordCnt++;
        }
        else
        if(CurrentSpeed)//把暂存的东西保存到flash
        {
            if(s_stSpeedAttrib.recordCnt)///需要转保//////
            {
                SpeedMonitor_SavePackRecordDataToFRAM(s_stSpeedAttrib.recordCnt,RTC_GetCounter());
            }
            s_stSpeedAttrib.recordCnt =0;
            ///////////////////////////
            saveflag = 1;
        }
        else//第一个速度为0的需要保存
        if(0==CurrentSpeed&&s_stSpeedAttrib.preSpeed)////
        {
            s_stSpeedAttrib.recordCnt = 0;
            saveflag = 1;
        }
    }
    ///////////////////////////
    if(saveflag)//
    {
        SpeedMonitor_SaveOneRecordData(stRunRecord,RTC_GetCounter());
    }
    else
    {
        SpeedMonitor_CheckRecordDataToSaveFlash();
    }

    s_stSpeedAttrib.preSpeed = CurrentSpeed;
    ////////////////////////////
    s_stSpeedAttrib.prerunStatus=s_stSpeedAttrib.runStatus;
}
/*************************************************************
** 函数名称: SpeedMonitor_ReadSpeedRecordDataFromFram
** 功能描述: 从铁电里读取指定时间段速度记录
** 入口参数: 起始时间StartTime,结束时间EndTime
** 出口参数: pBuffer保存数据的首地址
** 返回参数: pBuffer字节长度
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned short SpeedMonitor_ReadSpeedRecordDataFromFram(unsigned char *pBuffer,TIME_T StartTime,TIME_T EndTime)
{
    #if (ONE_SPEED_MONITOR_DATA_LEN != 6)
    unsigned long startVal,endVal;  
    TIME_T stTime;
    if(0==s_stSpSaveRecord.count||0 == s_stSpSaveRecord.startTime)return 0;
    startVal = ConverseGmtime(&StartTime);
    endVal = ConverseGmtime(&EndTime);    
    if(0==Public_CheckTimeIsInRange(s_stSpSaveRecord.startTime,startVal,endVal))return 0;//不在范围内
    Gmtime(&stTime,s_stSpSaveRecord.startTime);
    Public_ConvertTimeToBCDEx(stTime,pBuffer);
    memcpy(&pBuffer[6],s_stSpSaveRecord.stRecord,120);  
    return 126;
    #else
    return 0;
    #endif
}
/*************************************************************
** 函数名称: SpeedMonitor_ReadSpeedRecordData
** 功能描述: 读取指定时间段、指定最大数据块速度记录数据
** 入口参数: 起始时间StartTime,结束时间EndTime,MaxBlock指定最大数据块
** 出口参数: pBuffer保存数据的首地址
** 返回参数: pBuffer字节长度
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned short SpeedMonitor_ReadSpeedRecordData(u8 *pBuffer, TIME_T StartTime, TIME_T EndTime, u16 MaxBlock)
{
    unsigned char *p;
    unsigned short len;
    unsigned short block;
    len=0;
    p=pBuffer;
    block = MaxBlock;
    len=SpeedMonitor_ReadSpeedRecordDataFromFram(p,StartTime,EndTime);
    if(len)
    {
        p += len;
        if(block)block--;
    }
    /////////////////
    len += Register_Read(REGISTER_TYPE_SPEED,p,StartTime,EndTime,block);
    return len;
}
/*********************************************************************
//函数名称	:SpeedMonitor_IsOverSpeed
//功能		:判断当前是否超速
//输入		:CurrentSpeed当前速度,MaxSpeed限速值,UpDelaySec:持续秒数
//输出		:
//返回		:1:已超速，0:未超速
//备注		:
*********************************************************************/
unsigned char SpeedMonitor_IsOverSpeed(u8 CurrentSpeed, u8 MaxSpeed, u32 UpDelaySec)
{
    unsigned char ACC = 0;
    //读ACC状态
    ACC = Io_ReadStatusBit(STATUS_BIT_ACC);
    if(CurrentSpeed >= MaxSpeed&&SpeedMonitor_GetCurRunStatus())
    {
        //速度计数加1
        if(0==s_stSpeedAttrib.overSpeedCnt&&UpDelaySec>1)
        {
            //SetEvTask(EV_REPORT);
            Report_UploadPositionInfo(CHANNEL_DATA_1);
            Report_UploadPositionInfo(CHANNEL_DATA_2);
        }
        //////////////////
        s_stSpeedAttrib.overSpeedCnt++;
        if((s_stSpeedAttrib.overSpeedCnt >= UpDelaySec)&&(0 != ACC))
        {
            s_stSpeedAttrib.overSpeedCnt = UpDelaySec;
            return 1;	//已超速
        }
    }
    else
    {
        //速度计数清0
        if(s_stSpeedAttrib.overSpeedCnt&&CurrentSpeed < MaxSpeed)
        {
           
        }
        s_stSpeedAttrib.overSpeedCnt = 0;
    }
    return 0;	//没超速
}
/*********************************************************************
//函数名称	:SpeedMonitor_IsPreAlarm
//功能		:判断当前是否预警
//输入		:CurrentSpeed当前速度,MaxSpeed限速值,UpDelaySec:持续秒数
//输出		:
//返回		:1:已报警，0:未报警
//备注		:
*********************************************************************/
unsigned char SpeedMonitor_IsPreAlarm(u8 CurrentSpeed, u8 MaxSpeed, u32 UpDelaySec)
{
    unsigned char ACC = 0;
    ///////////////////
    if((0==MaxSpeed)||(MaxSpeed>=s_stSpeedAttrib.curSpeedLimit))return 0;	//没检查
    //读ACC状态
    ACC = Io_ReadStatusBit(STATUS_BIT_ACC);
    /////////////////
    if(CurrentSpeed >= MaxSpeed&&SpeedMonitor_GetCurRunStatus())
    {
        //速度计数加1
        s_stSpeedAttrib.preOverSpeedCnt++;
        if((s_stSpeedAttrib.preOverSpeedCnt >= UpDelaySec)&&(0 != ACC))
        {
            s_stSpeedAttrib.preOverSpeedCnt = UpDelaySec;
            return 1;	//已超速
        }
    }
    else
    {
        //速度计数清0
        s_stSpeedAttrib.preOverSpeedCnt = 0;
    }
    ////////////////////
    return 0;	//没超速
}
/*********************************************************************
//函数名称	:SpeedMonitor_PreAlarm
//功能		:超速预警提醒
//输入		:CurrentSpeed当前速度,MaxSpeed限速值
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void SpeedMonitor_PreAlarm(u8 CurrentSpeed, u8 MaxSpeed)
{
    unsigned char flag; 
    ////////////////////////////
    if((1 == SpeedMonitor_IsPreAlarm(CurrentSpeed,MaxSpeed,s_stSpeedAttrib.curprespeedDuration))
     &&(0 == Io_ReadAlarmBit(ALARM_BIT_OVER_SPEED)))
    {
        flag =1;
        Io_WriteAlarmBit(ALARM_BIT_OVER_SPEED_PRE_ALARM,SET);
    }
    else
    {
        //清除预警标志
        flag = 0;
        Io_WriteAlarmBit(ALARM_BIT_OVER_SPEED_PRE_ALARM,RESET);
    }
    ////////////////////////////////////
    if(SpeedMonitor_CheckPlayFlag(flag,&s_stSpeedAttrib.stPrePlay))
    {
        if(2 != s_stSpeedParam.playflag&&3 != s_stSpeedParam.playflag)
        {
            s_stSpeedParam.playflag = 3;
            LZM_PublicSetOnceTimer(&s_stSpeedTimer[SPEED_TIMER_TEST],PUBLICSECS(8),SpeedMonitor_ClearPlayFlag);
            SpeedMonitor_OverSpeedPreAlarmVoice();
        }
    }
}
/*********************************************************************
//函数名称	:SpeedMonitor_Alarm
//功能		:超速报警
//输入		:CurrentSpeed当前速度,MaxSpeed限速值，UpDelaySec:持续秒数
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void SpeedMonitor_Alarm(u8 CurrentSpeed, u8 MaxSpeed, u32 UpDelaySec)
{
    if(1 == SpeedMonitor_IsOverSpeed(CurrentSpeed, MaxSpeed, UpDelaySec))	//需要超速上报
    {
        Io_WriteOverSpeedConrtolBit(OVER_SPEED_BIT_ORDINARY,SET);
        //Io_WriteAlarmBit(ALARM_BIT_OVER_SPEED,SET);
    }
    else//不需要超速上报
    {
        Io_WriteOverSpeedConrtolBit(OVER_SPEED_BIT_ORDINARY,RESET);
        //Io_WriteAlarmBit(ALARM_BIT_OVER_SPEED,RESET);
    }
}
/*************************************************************
** 函数名称: SpeedMonitor_AlarmTTS
** 功能描述: 超速语音播报
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void SpeedMonitor_AlarmTTS(void)
{
    if(SpeedMonitor_CheckPlayFlag(Io_ReadAlarmBit(ALARM_BIT_OVER_SPEED),&s_stSpeedAttrib.stPlay))
    {
        if(2 != s_stSpeedParam.playflag)
        {
            s_stSpeedParam.playflag = 2;
            LZM_PublicSetOnceTimer(&s_stSpeedTimer[SPEED_TIMER_TEST],PUBLICSECS(8),SpeedMonitor_ClearPlayFlag);
            SpeedMonitor_OverSpeedAlarmVoice();
        }
    }
}
/*************************************************************
** 函数名称: SpeedMonitor_UpdateSpeedType
** 功能描述: 更新速度类型变量
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void SpeedMonitor_UpdateSpeedType(void)
{
    u8 PramLen;
    u8 Buffer[5];
    PramLen = EepromPram_ReadPram(E2_SPEED_SELECT_ID, Buffer);
    if(PramLen)
    {
        s_stSpeedAttrib.SpeedType = (Buffer[0]&0x01);//dxl,2015.5.11,0x00:手动脉冲,0x01:手动GPS,0x02:自动脉冲,0x03:自动GPS
    }
    else
    {
        s_stSpeedAttrib.SpeedType = 1;//为GPS速度
    }   
}
/*************************************************************
** 函数名称: SpeedMonitor_UpdateMaxStopCarTime
** 功能描述: 更新最长停车时间
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void SpeedMonitor_UpdateMaxStopCarTime(void)
{
    u8 PramLen;
    u8 Buffer[5];
    //从EEPROM读取数据更新变量
    PramLen = EepromPram_ReadPram(E2_MAX_STOP_CAR_TIME_ID, Buffer);
    if(PramLen)
    {
        s_stSpeedAttrib.OverstopTime =Public_ConvertBufferToLong(Buffer);
    }
    else
    {
        s_stSpeedAttrib.OverstopTime = 0xffffffff;//        
    }
    /////////////////////////////
    if(0xffffffff==s_stSpeedAttrib.OverstopTime||0==s_stSpeedAttrib.OverstopTime)
    {
        Io_WriteAlarmBit(ALARM_BIT_STOP_OVER,RESET);
    }
}
/*************************************************************
** 函数名称: SpeedMonitor_UpdateNightParam
** 功能描述: 更新夜间行驶预警参数
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void SpeedMonitor_UpdateNightParam(void)
{
    u8 PramLen;
    u8 Buffer[30];
    //从EEPROM读取数据更新变量
    memset(&s_stSpNight,0,sizeof(s_stSpNight));
    /////////////夜间超速百分比///////////////////////
    PramLen = EepromPram_ReadPram(E2_NIGHT_OVERSPEED_PERCENT_ID, Buffer);    
    if(PramLen)
    {
        s_stSpNight.percent =Public_ConvertBufferToLong(Buffer);
        if(s_stSpNight.percent>=100) s_stSpNight.percent = 0;
    }
    ////////////////////////
    if(0==s_stSpNight.percent)return;
    /////////////////////////    
    PramLen = EepromPram_ReadPram(E2_OVERSPEED_NIGHT_TIME_ID, Buffer);
    if(PramLen)
    {
        s_stSpNight.control = Public_ConvertBufferToShort(Buffer);        
        s_stSpNight.startVal=Public_ConvertBCDTimeToLong(s_stSpNight.control,&Buffer[2]);
        if(s_stSpNight.startVal)
        {
            s_stSpNight.endVal=Public_ConvertBCDTimeToLong(s_stSpNight.control,&Buffer[8]);            
            if(0==s_stSpNight.endVal)
            {
                s_stSpNight.startVal = 0;
            }
        }        
    }    
    //////////////////////////
    if(s_stSpNight.percent&&Public_CheckTimeRangeIsValid(s_stSpNight.control,s_stSpNight.startVal,s_stSpNight.endVal))
    {
        s_stSpNight.needChecknight = 1;        
    }
    else
    {
        s_stSpNight.needChecknight =0;
        s_stSpNight.percent =0;
        return;
    }
    ////////////////////////
    PramLen = EepromPram_ReadPram(E2_NIGHT_DRIVE_PREALARM_TIME_ID, Buffer);//
    if(PramLen)
    {
        s_stSpNight.earlyTime=Public_ConvertBufferToLong(Buffer);
    }
    else
    {
        s_stSpNight.earlyTime = 10;//分钟
    }
    s_stSpNight.earlyTime *= 60;//秒
    ///////////////////////
    /////////////
    PramLen = EepromPram_ReadPram(E2_NIGHT_DRIVE_PREALARM_GPOUP_TIME_ID, Buffer);//
    if(PramLen)
    {
        s_stSpNight.stPlay.GroupTime=Public_ConvertBufferToLong(Buffer)*60;
    }
    else
    {
        s_stSpNight.stPlay.GroupTime = 300;//
    }
    PramLen = EepromPram_ReadPram(E2_NIGHT_DRIVE_PREALARM_NUMBER_ID, Buffer);//    
    if(PramLen)
    {
        s_stSpNight.stPlay.OnceNumber = Public_ConvertBufferToLong(Buffer);
    }
    else
    {
        s_stSpNight.stPlay.OnceNumber = 3;//
    }
}
/*************************************************************
** 函数名称: SpeedMonitor_UpdateForbidParam
** 功能描述: 更新禁止行驶参数
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void SpeedMonitor_UpdateForbidParam(void)
{
    u8 PramLen;
    u8 Buffer[30];
    //从EEPROM读取数据更新变量
    memset(&s_stSpForbid,0,sizeof(s_stSpForbid));
    ////////////////////////
    PramLen = EepromPram_ReadPram(E2_FORBID_DRIVE_NIGHT_TIME_ID, Buffer);
    if(PramLen)
    {
        s_stSpForbid.control = Public_ConvertBufferToShort(Buffer);
        s_stSpForbid.startVal = Public_ConvertBCDTimeToLong(s_stSpForbid.control,&Buffer[2]);
        if(s_stSpForbid.startVal)
        {
            s_stSpForbid.endVal = Public_ConvertBCDTimeToLong(s_stSpForbid.control,&Buffer[8]);            
            if(0==s_stSpForbid.endVal)
            {
                s_stSpForbid.startVal = 0;
            }
        }
    }
    //////////////////
    if(!Public_CheckTimeRangeIsValid(s_stSpForbid.control,s_stSpForbid.startVal,s_stSpForbid.endVal))
    {
        Io_WriteSuoMeiStatusBit(SUOMEI_BIT_28,RESET);
        return;
    }
    ////////////////////////
    PramLen = EepromPram_ReadPram(E2_FORBID_DRIVE_PREALARM_TIME_ID, Buffer);//
    if(PramLen)
    {
        s_stSpForbid.earlyTime=Public_ConvertBufferToLong(Buffer);
    }
    else
    {
        s_stSpForbid.earlyTime=30;//分钟
    }
    s_stSpForbid.earlyTime *=60;//秒   
    //////////////////////
    PramLen=EepromPram_ReadPram(E2_FORBID_DRIVE_PREALARM_GPOUP_TIME_ID, Buffer);    
    if(PramLen)////预警 单次提示时间间隔(设定值) 
    {
        s_stSpForbid.stPrePlay.GroupTime = Public_ConvertBufferToLong(Buffer)*60;
    }
    else
    {
        s_stSpForbid.stPrePlay.GroupTime =300;
    }
    PramLen=EepromPram_ReadPram(E2_FORBID_DRIVE_PREALARM_NUMBER_ID, Buffer);
    if(PramLen)//预警 提示单次重复播报的次数(设定值)  
    {
        s_stSpForbid.stPrePlay.OnceNumber = Public_ConvertBufferToLong(Buffer);
    }
    else
    {
        s_stSpForbid.stPrePlay.OnceNumber =3;
    }
    ///////////////
    PramLen = EepromPram_ReadPram(E2_FORBID_DRIVE_ALARM_GPOUP_TIME_ID, Buffer);//
    if(PramLen)
    {
        s_stSpForbid.stPlay.GroupTime=Public_ConvertBufferToLong(Buffer)*60;
    }
    else
    {
        s_stSpForbid.stPlay.GroupTime =300;
    }
    ////////////////
    PramLen = EepromPram_ReadPram(E2_FORBID_DRIVE_ALARM_NUMBER_ID, Buffer);//    
    if(PramLen)
    {
        s_stSpForbid.stPlay.OnceNumber = Public_ConvertBufferToLong(Buffer);
    }
    else
    {
        s_stSpForbid.stPlay.OnceNumber =3;
    }
    ///////////////////
    PramLen = EepromPram_ReadPram(E2_FORBID_DRIVE_RUN_SPEED_ID, Buffer);//
    if(PramLen)//停车判定速度
    {
        s_stSpForbid.runSpeed = Public_ConvertBufferToLong(Buffer);
    }
    else
    {
        s_stSpForbid.runSpeed =20;
    }
    ///////////////
    PramLen = EepromPram_ReadPram(E2_FORBID_DRIVE_RUN_SPEED_KEEPTIME_ID, Buffer);//
    if(PramLen)//停车判断持续运行时间
    {
        s_stSpForbid.runDuration = Public_ConvertBufferToLong(Buffer)*60;
    }
    else
    {
        s_stSpForbid.runDuration =300;
    }
    /////////////////
    s_stSpForbid.CheckVaild = 1;
}
/*************************************************************
** 函数名称: SpeedMonitor_UpDatePram
** 功能描述: 设置或修改超速相关变量后,需要更新超速模块相关变量(该模块内的所有参数)
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:SpeedMonitor_UpdateForbidParam,SpeedMonitor_UpdateNightParam
*************************************************************/
void SpeedMonitor_UpdatePram(void)
{
    unsigned long speedLimit,speedDuration;
    u8 PramLen;
    u8 Buffer[50];
    //从EEPROM读取数据更新变量
    SpeedMonitor_UpdateNightParam();
    //////////////////////////////
    SpeedMonitor_UpdateForbidParam();
    /////////////////////////
    SpeedMonitor_UpdateSpeedType();   
    ///////////////////////
    SpeedMonitor_UpdateMaxStopCarTime();
    ///////////////////////////////////
    SpeedMonitor_ReadStopStartTime();
    /////////////////////////
    SpeedMonitor_ReadSpeedPlay();
    ////////////////////
    //从EEPROM读取数据更新变量
    ///////////////////////////////////////    
    PramLen=EepromPram_ReadPram(E2_SPEED_EARLY_ALARM_DVALUE_ID, Buffer);
    if(PramLen)
    {
        s_stSpeedAttrib.EarlyAlarmDVal = (unsigned short)(Public_ConvertBufferToShort(Buffer)*0.1+0.5);//单位为1/10Km/h 
    }
    else
    {
        s_stSpeedAttrib.EarlyAlarmDVal = 5;//超速报警预警差值，单位为1/10Km/h 
    }
    /////////////////////
    PramLen = EepromPram_ReadPram(E2_MAX_SPEED_ID, Buffer);
    if(PramLen)
    {
        speedLimit =Public_ConvertBufferToLong(Buffer);
    }
    else
    {
        speedLimit = 100;//当最大限速为0时,表示不开启速度监控
    }
    PramLen=EepromPram_ReadPram(E2_OVER_SPEED_KEEP_TIME_ID, Buffer);
    if(PramLen)
    {
        speedDuration =Public_ConvertBufferToLong(Buffer);
    }
    else
    {
        speedDuration = 10;//默认情况是超速达10秒后才报警
    }
    //////////////////////////
    SpeedMonitor_SetSpeedParamValid(E_SPEED_TYPE_GENERAL,0,speedLimit,speedDuration);  
    //////////////超速提醒/////////////////////
    PramLen=EepromPram_ReadPram(E2_OVERSPEED_ALARM_NUMBER_ID, Buffer);
    if(PramLen)
    {
        s_stSpeedAttrib.stPlay.OnceNumber = Public_ConvertBufferToLong(Buffer);
    }
    else
    {
       s_stSpeedAttrib.stPlay.OnceNumber =3; 
    }
    PramLen=EepromPram_ReadPram(E2_OVERSPEED_ALARM_GPROUP_TIME_ID, Buffer);
    if(PramLen)
    {
        s_stSpeedAttrib.stPlay.GroupTime = Public_ConvertBufferToLong(Buffer)*60;
    }
    else
    {
       s_stSpeedAttrib.stPlay.GroupTime =300; 
    }
    PramLen=EepromPram_ReadPram(E2_OVERSPEED_ALARM_NUMBER_TIME_ID, Buffer);
    if(PramLen)
    {
        s_stSpeedAttrib.stPlay.OnceInterval = Public_ConvertBufferToLong(Buffer);
    }
    else
    {
       s_stSpeedAttrib.stPlay.OnceInterval =10; 
    }
    ////////////////////////
    PramLen=EepromPram_ReadPram(E2_OVERSPEED_ALARM_VOICE_ID, Buffer);
    if(PramLen)
    {
        Buffer[PramLen]=0;
        SpeedMonitor_SetSpeedParamPlayVoice(E_SPEED_TYPE_GENERAL,(char *)Buffer);
    }
    else
    {
        SpeedMonitor_SetSpeedParamPlayVoice(E_SPEED_TYPE_GENERAL,"您已超速,请减速行驶");
    }
    /////////////////////////////////////////////
    SpeedMonitor_SetSpeedParamPlay(E_SPEED_TYPE_GENERAL,s_stSpeedAttrib.stPlay.GroupTime,s_stSpeedAttrib.stPlay.OnceInterval,s_stSpeedAttrib.stPlay.OnceNumber); 
    
    /////////////超速预警提醒///////////////
    PramLen=EepromPram_ReadPram(E2_OVERSPEED_PREALARM_NUMBER_ID, Buffer);
    if(PramLen)//预超速 提示单次重复播报的次数(设定值)  
    {
        s_stSpeedAttrib.stPrePlay.OnceNumber = Public_ConvertBufferToLong(Buffer);
    }
    else
    {
       s_stSpeedAttrib.stPrePlay.OnceNumber =3; 
    }
    PramLen=EepromPram_ReadPram(E2_OVERSPEED_PREALARM_GPROUP_TIME_ID, Buffer);
    if(PramLen)//预超速 组提示时间间隔(设定值)  
    {
        s_stSpeedAttrib.stPrePlay.GroupTime = Public_ConvertBufferToLong(Buffer)*60;
    } 
    else
    {
       s_stSpeedAttrib.stPrePlay.GroupTime =300; 
    }
    
    PramLen=EepromPram_ReadPram(E2_OVERSPEED_PREALARM_NUMBER_TIME_ID, Buffer);
    if(PramLen)////预超速 单次提示时间间隔(设定值) 
    {
        s_stSpeedAttrib.stPrePlay.OnceInterval = Public_ConvertBufferToLong(Buffer);
    }
    else
    {
       s_stSpeedAttrib.stPrePlay.OnceInterval =10;
    }
    
    PramLen=EepromPram_ReadPram(E2_OVERSPEED_PREALARM_KEEPTIME_ID, Buffer);
    if(PramLen)//预超速持续时间设置值
    {
        s_stSpeedAttrib.prespeedDuration = Public_ConvertBufferToLong(Buffer);
    }
    else
    {
       s_stSpeedAttrib.prespeedDuration =3;
    }
    ///////////////////////////
    PramLen=EepromPram_ReadPram(E2_OVERSPEED_ALARM_REPORT_TIME_ID, Buffer);
    if(PramLen)//超速报警位置上报时间间隔,DWORD,单位秒,默认值0
    {
        s_stSpeedAttrib.ReportInterval = Public_ConvertBufferToLong(Buffer);
    }
    else
    {
       s_stSpeedAttrib.ReportInterval =0;
    }
    PramLen=EepromPram_ReadPram(E2_REPORT_FREQ_EVENT_SWITCH_ID, Buffer);
    if(PramLen)//用于指定产生事件后上报时间间隔的事件项 BIT0位＝1：超速报警
    {
        s_stSpeedAttrib.ReportFreqEvent = Public_ConvertBufferToLong(Buffer);
    }
    else
    {
       s_stSpeedAttrib.ReportFreqEvent = 0;
    }
    ///////////////////////////
    if(!PUBLIC_CHECKBIT(s_stSpeedAttrib.ReportFreqEvent,0))
    {
        s_stSpeedAttrib.ReportInterval = 0;
    }
    ////////////////////
    s_stSpeedAttrib.overSpeedCnt = 0;
    s_stSpeedAttrib.preOverSpeedCnt = 0;
    /////////////////////////////////
    SpeedMonitor_UpdateCurSpeedLimitParam();
}
/*************************************************************
** 函数名称: SpeedMonitor_GetCurSpeed
** 功能描述: 获取瞬时速度(已考虑了速度类型，不需再判断速度类型了)
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned char SpeedMonitor_GetCurSpeed(void)
{
    u8	CurrentSpeed;	
    //获取速度选择
    if(0 == SpeedMonitor_GetCurSpeedType())
    {
        CurrentSpeed = Pulse_GetSpeed();
    }
    else
    {
        CurrentSpeed = Gps_ReadSpeed();
    }    
    return CurrentSpeed;
}
/*************************************************************
** 函数名称: SpeedMonitor_GetSpeedMaxLimitVal
** 功能描述: 获取限速值
** 入口参数: 
** 出口参数: 
** 返回参数: 限速值
** 全局变量: 无
** 调用模块:
*************************************************************/
u32 SpeedMonitor_GetSpeedMaxLimitVal(void)
{
    return s_stSpeedAttrib.curSpeedLimit;
}
/*************************************************************
** 函数名称: SpeedMonitor_GetCurSpeedType
** 功能描述: 获取速度类型
** 入口参数: 
** 出口参数: 
** 返回参数: 0为脉冲,1为GPS
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned char SpeedMonitor_GetCurSpeedType(void)
{
    return (s_stSpeedAttrib.SpeedType)?1:0;
}
/*************************************************************
** 函数名称: SpeedMonitor_GetEarlyAlarmDVal
** 功能描述: 获取速度预警差值
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned char SpeedMonitor_GetEarlyAlarmDVal(void)
{
    return s_stSpeedAttrib.EarlyAlarmDVal;
}
/*************************************************************
** 函数名称: SpeedMonitor_GetReportFreqEvent
** 功能描述: 获取上报时间间隔的事件项
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned long SpeedMonitor_GetReportFreqEvent(void)
{
    return s_stSpeedAttrib.ReportFreqEvent;
}
/*************************************************************
** 函数名称: SpeedMonitor_CheckOverStopTime
** 功能描述: 判断是否超时停车
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void SpeedMonitor_CheckOverStopTime(void)
{
    unsigned long timeVal;
    unsigned char flag;
    if(0xffffffff==s_stSpeedAttrib.OverstopTime||0==s_stSpeedAttrib.OverstopTime)
    {
        return;
    }
    flag = 0;    
    if(TiredDrive_GetTimeAbnormalflag())
    {
        s_stSpeedAttrib.startStopTime =0;
        SpeedMonitor_SaveStopStartTime();
    }
    //////////////////////////
    if(CAR_STATUS_RUN==s_stSpeedAttrib.runStatus)//行驶
    {
        if(s_stSpeedAttrib.startStopTime)
        {
            s_stSpeedAttrib.startStopTime =0;
            SpeedMonitor_SaveStopStartTime();
        }
    }
    else
    if((CAR_STATUS_RUN==s_stSpeedAttrib.prerunStatus)&&(CAR_STATUS_STOP==s_stSpeedAttrib.runStatus))//由行驶到停驶
    {
        s_stSpeedAttrib.startStopTime = RTC_GetCounter()-RUN_CHAGE_STATUS_TIME;//
        SpeedMonitor_SaveStopStartTime();
    }
    else
    if(CAR_STATUS_STOP==s_stSpeedAttrib.runStatus)//停驶
    {
        timeVal =RTC_GetCounter();
        if(0==s_stSpeedAttrib.startStopTime)
        {
            s_stSpeedAttrib.startStopTime = timeVal;//
            SpeedMonitor_SaveStopStartTime();
        }
        else//如果开始停车时间比现在的时间还大
        if(s_stSpeedAttrib.startStopTime>=timeVal)
        {
            s_stSpeedAttrib.startStopTime = timeVal;//
            SpeedMonitor_SaveStopStartTime();
        }
        ////////////////////        
        if(timeVal>=s_stSpeedAttrib.startStopTime+s_stSpeedAttrib.OverstopTime)
        {
            flag =1;
        }
    }
    
    ///////////////////////////
    if(1==flag)
    {
        if(2 != s_stSpeedAttrib.OverStopTimeFlag)
        {
            s_stSpeedAttrib.OverStopTimeFlag = 2;
            Io_WriteAlarmBit(ALARM_BIT_STOP_OVER,SET);
        }
    }
    else
    {
        if(1 != s_stSpeedAttrib.OverStopTimeFlag)
        {
            s_stSpeedAttrib.OverStopTimeFlag = 1;
            Io_WriteAlarmBit(ALARM_BIT_STOP_OVER,RESET);
        }
    }
}
/*************************************************************
** 函数名称: SpeedMonitor_ClearPlayFlag
** 功能描述: 
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void SpeedMonitor_ClearPlayFlag(void) 
{
    s_stSpeedParam.playflag =0;
}
/*************************************************************
** 函数名称: SpeedMonitor_PlayCurSpeedLimitValue
** 功能描述: 
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void SpeedMonitor_PlayCurSpeedLimitValue(void) 
{
    if(!SpeedMonitor_GetCurRunStatus())
    { 
        s_stSpeedPlay.playSpeed = s_stSpeedAttrib.curSpeedLimit;
        SpeedMonitor_SaveSpeedPlay();
        return;
    }
    //////////////////////////////
    if(s_stSpeedPlay.playSpeed == s_stSpeedAttrib.curSpeedLimit)return;
    ///////////////////////////
    if((0==TTS_GetPlayBusyFlag())||(TTS_GetPlayBusyFlag()&&1==s_stSpeedParam.playflag))
    {
        unsigned char buffer[20];
        s_stSpeedParam.playflag = 1;       
        LZM_PublicSetOnceTimer(&s_stSpeedTimer[SPEED_TIMER_TEST],PUBLICSECS(8),SpeedMonitor_ClearPlayFlag);
        sprintf((char *)buffer,"当前限速%d公里",s_stSpeedAttrib.curSpeedLimit);       
        Public_PlayTTSVoiceStr(buffer);
        s_stSpeedPlay.playSpeed = s_stSpeedAttrib.curSpeedLimit;
        SpeedMonitor_SaveSpeedPlay();
    }
    else
    {
       LZM_PublicSetOnceTimer(&s_stSpeedTimer[SPEED_TIMER_PLAY],PUBLICSECS(1),SpeedMonitor_PlayCurSpeedLimitValue);
    }
}
/*************************************************************
** 函数名称: SpeedMonitor_UpdateCurSpeedLimitParam
** 功能描述: 
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void SpeedMonitor_UpdateCurSpeedLimitParam(void) 
{
    unsigned char type;
    type = s_stSpeedParam.type;    
    s_stSpeedAttrib.overSpeedCnt = 0;
    s_stSpeedAttrib.preOverSpeedCnt = 0;
    s_stSpeedAttrib.curSpeedLimit = s_stSpeedParam.speedLimit;
    s_stSpeedAttrib.stPlay.GroupTime = s_stSpeedParam.stSpType[type].GroupTime; 
    s_stSpeedAttrib.stPlay.OnceNumber = s_stSpeedParam.stSpType[type].OnceNumber;       
    s_stSpeedAttrib.stPlay.OnceInterval = s_stSpeedParam.stSpType[type].OnceInterval;    
    SpeedMonitor_UpdateNightSpeedLimit();
    //////////更新预警//////////////////    
    s_stSpeedAttrib.curprespeedDuration = s_stSpeedAttrib.prespeedDuration;
    if(s_stSpeedAttrib.EarlyAlarmDVal&&s_stSpeedAttrib.curSpeedLimit>s_stSpeedAttrib.EarlyAlarmDVal)
    {
        s_stSpeedAttrib.curpreSpeedLimit = s_stSpeedAttrib.curSpeedLimit-s_stSpeedAttrib.EarlyAlarmDVal;
    }
    else
    {
        s_stSpeedAttrib.curpreSpeedLimit = 0;
    }
    /////////////////
    if(Io_ReadAlarmMaskBit(ALARM_BIT_OVER_SPEED))return;//超速功能屏蔽了就不播报速度提示了
    LZM_PublicSetOnceTimer(&s_stSpeedTimer[SPEED_TIMER_PLAY],PUBLICSECS(4),SpeedMonitor_PlayCurSpeedLimitValue);
}
/*************************************************************
** 函数名称: SpeedMonitor_CheckSpeedLimitIndex
** 功能描述: 
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void SpeedMonitor_CheckSpeedLimitIndex(void) 
{
    unsigned char i;
    unsigned char speed,type;
    /////////////////////////
    type = 0;
    speed = s_stSpeedParam.stSpType[0].SpeedLimit;    
    for(i=1; i<E_SPEED_TYPE_MAX; i++)//不能算夜间,夜间的是在最小的基础上*百分比
    {
        if(PUBLIC_CHECKBIT(s_stSpeedParam.valid, i))
        {
            if(speed > s_stSpeedParam.stSpType[i].SpeedLimit)
            {
                type = i;
                speed = s_stSpeedParam.stSpType[i].SpeedLimit;
            }
        }
    }
    ////////////////////////////
    if((s_stSpeedParam.type != type)||(s_stSpeedParam.speedLimit != s_stSpeedParam.stSpType[type].SpeedLimit))
    {
        s_stSpeedParam.type = type;
        s_stSpeedParam.speedLimit =s_stSpeedParam.stSpType[type].SpeedLimit;
        SpeedMonitor_UpdateCurSpeedLimitParam();
    }
}
/*************************************************************
** 函数名称: SpeedMonitor_SetSpeedParamValid
** 功能描述: 设置某限速类型为有效
** 入口参数: type,限速类型,ID:区域ID或线路ID,speedLimit限速值,duration持续时间
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void SpeedMonitor_SetSpeedParamValid(unsigned char type,unsigned long ID,unsigned char speedLimit,unsigned char duration) 
{
    if(type>=E_SPEED_TYPE_MAX)return;   
    //////////////////
    s_stSpeedParam.stSpType[type].ID = ID;
    ///////////////////////
    if(E_SPEED_TYPE_GENERAL != type)//
    {
       s_stSpeedParam.stSpType[type].OnceNumber = s_stSpeedParam.stSpType[0].OnceNumber ;
       s_stSpeedParam.stSpType[type].GroupTime = s_stSpeedParam.stSpType[0].GroupTime;
       s_stSpeedParam.stSpType[type].OnceInterval = s_stSpeedParam.stSpType[0].OnceInterval;
       strcpy(s_stSpeedParam.stSpType[type].PlayVoice,s_stSpeedParam.stSpType[0].PlayVoice);
    }
    else
    if(PUBLIC_CHECKBIT(s_stSpeedParam.valid, type))//出区域不改限速时使用,==E_SPEED_TYPE_GENERAL且有效
    {
        unsigned char buffer[10];
        if(speedLimit != s_stSpeedParam.stSpType[E_SPEED_TYPE_GENERAL].SpeedLimit)             
        {
            Public_ConvertLongToBuffer(speedLimit,buffer);
            EepromPram_WritePram(E2_MAX_SPEED_ID,buffer,E2_MAX_SPEED_LEN);
        }
        //////////////////////////
        if(duration != s_stSpeedParam.stSpType[E_SPEED_TYPE_GENERAL].Duration)
        {
            Public_ConvertLongToBuffer(duration,buffer);
            EepromPram_WritePram(E2_OVER_SPEED_KEEP_TIME_ID,buffer,E2_OVER_SPEED_KEEP_TIME_LEN);
        }
    }
    ///////////////////
    if(PUBLIC_CHECKBIT(s_stSpeedParam.valid, type))//有效
    {
        if(s_stSpeedParam.stSpType[type].SpeedLimit == speedLimit)
        {
            s_stSpeedParam.stSpType[type].Duration = duration;
            if(s_stSpeedParam.type == type)
            {
                s_stSpeedAttrib.speedDuration = duration;
            }
            return;//限值不变
        }
    }
    else//
    {
        PUBLIC_SETBIT(s_stSpeedParam.valid, type); 
    }
    ////////////////////////////
    s_stSpeedParam.stSpType[type].Duration = duration;
    ///////////////////////////////
    s_stSpeedParam.stSpType[type].SpeedLimit = speedLimit;
    /////////////////////////////////////
    if(s_stSpeedParam.type == type)//当前更新的参数类型与最小值类型相同时
    {
        SpeedMonitor_CheckSpeedLimitIndex();
    }
    else//当前更新的参数类型与最小值类型不同时
    if(s_stSpeedParam.speedLimit >= speedLimit)
    {
        s_stSpeedParam.type = type;
        s_stSpeedParam.speedLimit = speedLimit;         
        SpeedMonitor_UpdateCurSpeedLimitParam();
    }    
}
/*************************************************************
** 函数名称: SpeedMonitor_ClearSpeedParamValid
** 功能描述: 设置某限速类型为无效
** 入口参数: type,限速类型
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void SpeedMonitor_ClearSpeedParamValid(unsigned char type) 
{
    if(type>=E_SPEED_TYPE_MAX)return;
    if(!PUBLIC_CHECKBIT(s_stSpeedParam.valid, type))return;
    PUBLIC_CLRBIT(s_stSpeedParam.valid, type);
    if(s_stSpeedParam.type == type)
    {
        SpeedMonitor_CheckSpeedLimitIndex();
    }
}
/*************************************************************
** 函数名称: SpeedMonitor_SetSpeedParamPlayVoice
** 功能描述: 设置某限速类型的播报语音
** 入口参数: type,限速类型,playVoice超速播报语音
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void SpeedMonitor_SetSpeedParamPlayVoice(unsigned char type,char *playVoice) 
{
    if(type>=E_SPEED_TYPE_MAX)return;
    if(!PUBLIC_CHECKBIT(s_stSpeedParam.valid, type))return;
    strcpy(s_stSpeedParam.stSpType[type].PlayVoice,playVoice);
}
/*************************************************************
** 函数名称: SpeedMonitor_SetSpeedParamPlay
** 功能描述: 设置某限速类型的播报参数
** 入口参数: type,限速类型,GroupTime组提示时间间隔;OnceInterval单次提示时间间隔;OnceNumber提示单次重复播报的次数
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void SpeedMonitor_SetSpeedParamPlay(unsigned char type,unsigned short GroupTime,unsigned short OnceInterval,unsigned char OnceNumber) 
{
    if(type>=E_SPEED_TYPE_MAX)return;
    if(!PUBLIC_CHECKBIT(s_stSpeedParam.valid, type))return;
    s_stSpeedParam.stSpType[type].GroupTime = GroupTime;
    s_stSpeedParam.stSpType[type].OnceNumber = OnceNumber;    
    s_stSpeedParam.stSpType[type].OnceInterval = OnceInterval;    
    if(type==s_stSpeedParam.type)//
    {
        s_stSpeedAttrib.stPlay.GroupTime = GroupTime;
        s_stSpeedAttrib.stPlay.OnceNumber = OnceNumber;        
        s_stSpeedAttrib.stPlay.OnceInterval = OnceInterval;
    }
}
/*************************************************************
** 函数名称: SpeedMonitor_SetPreLimitSpeedParam
** 功能描述: 设置预警限速值(只有速度类型与当前限速类型一致时才有效)
** 入口参数: type速度类型,preSpeedLimit限速值,duration持续时间,saveflag:是否需要保存,0不需要保存,1:需要保存
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void SpeedMonitor_SetPreLimitSpeedParam(unsigned char type,unsigned char preSpeedLimit,unsigned char duration,unsigned char saveflag)
{
   
}
/*************************************************************
** 函数名称: SpeedMonitor_CheckPlayFlag
** 功能描述: 查检是否需要播报
** 入口参数: VaildFlag检查有效标志,stPlay播报参数结构体
** 出口参数: 
** 返回参数: 1:需要播报,0:不需要播报
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned char SpeedMonitor_CheckPlayFlag(unsigned char VaildFlag,STPLAY_ATTRIB *stPlay)
{
    unsigned short temp,tmp;    
    if(VaildFlag&&stPlay->OnceNumber)
    {
        stPlay->GroupTimeCnt++;//组提示时间计数器
        if(stPlay->GroupTime)//组提示时间间隔
        {
            temp = stPlay->GroupTimeCnt%stPlay->GroupTime;//5分钟一组
        }
        else
        {
            temp = stPlay->GroupTimeCnt;
        }
        //////////////////////////////////////
        if(temp==1)//组提示时间间隔时间到
        {
            stPlay->OnceCnt = 0;//提示单次重复播报的计数器 
        }
        /////////////////////////
        tmp = stPlay->OnceInterval;
        if(0==tmp)//未设置
        {
            tmp=ONCE_DEFAULT_PLAY_TIME;
        }
        else
        if(tmp < ONCE_MIN_PLAY_TIME)
        {
            tmp = ONCE_MIN_PLAY_TIME;//单次提示时间间隔
        }
        if(1 ==(temp%tmp))//单次提示时间间隔
        {
            if(stPlay->OnceCnt<stPlay->OnceNumber)
            {
                stPlay->OnceCnt++;
                return 1;
            }
        }
    }
    else
    {
        stPlay->OnceCnt = 0;
        stPlay->GroupTimeCnt = 0;
    }
    return 0;
}
/*************************************************************
** 函数名称: SpeedMonitor_GetNeedReportFlag
** 功能描述: 是否需要上报位置信息
** 入口参数: 
** 出口参数: 
** 返回参数: 0:不需要上报,1:需要上报
** 全局变量: 无
** 调用模块:
*************************************************************/
void SpeedMonitor_CheckNeedReportFlag(void)
{
    unsigned short temp;
    s_stSpeedAttrib.needReportflag = 0;
    if(SpeedMonitor_GetOverSpeedStatusForReport())
    {
        s_stSpeedAttrib.ReportTimeCnt++;
        temp = s_stSpeedAttrib.ReportTimeCnt%s_stSpeedAttrib.ReportInterval;
        if(1 == temp)
        {
            s_stSpeedAttrib.needReportflag =1;//
        }
    }
    else
    {
       s_stSpeedAttrib.ReportTimeCnt =0;
    }
}
/*************************************************************
** 函数名称: SpeedMonitor_CheckOverSpeedAlarm
** 功能描述: 速度监控的定时任务(1s调试一次)
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void SpeedMonitor_CheckOverSpeedAlarm(void)
{
    //超速判断
    SpeedMonitor_Alarm(s_stSpeedAttrib.curSpeed, s_stSpeedAttrib.curSpeedLimit, s_stSpeedParam.stSpType[s_stSpeedParam.type].Duration);
    //超速语音提醒
    SpeedMonitor_AlarmTTS();
    //预警判断及语音提醒
    SpeedMonitor_PreAlarm(s_stSpeedAttrib.curSpeed,s_stSpeedAttrib.curpreSpeedLimit);    
    ///////////////////
    SpeedMonitor_CheckNeedReportFlag();
}
/*************************************************************
** 函数名称: SpeedMonitor_CheckIsAtForbidTime
** 功能描述: 判断是否属于禁止行驶时间段
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void SpeedMonitor_CheckIsAtForbidTime(void)
{
    unsigned char flag,flag1;    
    if(CAR_STATUS_RUN != s_stSpForbid.runStatus)
    {
        SpeedMonitor_CheckPlayFlag(0,&s_stSpForbid.stPlay);
        SpeedMonitor_CheckPlayFlag(0,&s_stSpForbid.stPrePlay);
        Io_WriteSuoMeiStatusBit(SUOMEI_BIT_28,RESET);
        return;
    }
    ////////////////////////
    flag = 0;
    flag1 = 0;
    if(Public_CheckCurTimeIsInTimeRange(s_stSpForbid.control,s_stSpForbid.startVal,s_stSpForbid.endVal))
    {
        flag1=1;
    }
    else//夜间预警时间段
    if(Public_CheckCurTimeIsInEarlyAlarmTimeRange(s_stSpForbid.control,s_stSpForbid.earlyTime,s_stSpForbid.startVal,s_stSpForbid.endVal))
    {
        flag =1;
    }
    /////////////////////////////////
    if(SpeedMonitor_CheckPlayFlag(flag,&s_stSpForbid.stPrePlay))
    {
        SpeedMonitor_ForbidPreAlarmVoice();
    }
    /////////////////////
    if(SpeedMonitor_CheckPlayFlag(flag1,&s_stSpForbid.stPlay))
    {
        SpeedMonitor_ForbidAlarmVoice();
    }
    //////////////////////
    if(flag1)//禁行时段行驶报警
    {
        Io_WriteSuoMeiStatusBit(SUOMEI_BIT_28,SET);
    }
    else
    {
        Io_WriteSuoMeiStatusBit(SUOMEI_BIT_28,RESET);
    }
}
/*************************************************************
** 函数名称: SpeedMonitor_CheckForbidAlarm
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void SpeedMonitor_CheckForbidAlarm(void)
{
    if(0==s_stSpForbid.CheckVaild)return;
    SpeedMonitor_CheckForbidRunStatus(s_stSpeedAttrib.curSpeed);
    SpeedMonitor_CheckIsAtForbidTime();
}

/*************************************************************
** 函数名称: SpeedMonitor_GetOverSpeedStatusForReport
** 功能描述: 获取是否根据超速状态来上报位置信息
** 入口参数: 
** 出口参数: 
** 返回参数: 0:不根据超速状态来上报, 1:根据超速状态来上报
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned char SpeedMonitor_GetOverSpeedStatusForReport(void)
{
    if(!PUBLIC_CHECKBIT(s_stSpeedAttrib.ReportFreqEvent,0))return 0;
    if(0 == s_stSpeedAttrib.ReportInterval)return 0;
    if(0 == Io_ReadAlarmBit(ALARM_BIT_OVER_SPEED))return 0;
    if(Io_ReadAlarmBit(ALARM_BIT_EMERGENCY))return 0;
    return 1;
}
/*************************************************************
** 函数名称: SpeedMonitor_GetNeedReportFlag
** 功能描述: 获取是否需要上报位置信息标志
** 入口参数: 
** 出口参数: 
** 返回参数: 0:不需要上报,1:需要上报
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned char SpeedMonitor_GetNeedReportFlag(void)
{
    if(0==s_stSpeedAttrib.needReportflag)return 0;
    s_stSpeedAttrib.needReportflag = 0;
    return 1;
    
}
/*************************************************************
** 函数名称: SpeedMonitor_GetCurMinSpeedType
** 功能描述: 获取当前的最小速度的类型
** 入口参数: 
** 出口参数: 
** 返回参数: 0,普通;1:圆形;2:矩形;3:多边形;4:线路
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned char SpeedMonitor_GetCurMinSpeedType(void)
{
    return s_stSpeedParam.type;
}
/*************************************************************
** 函数名称: SpeedMonitor_ScanTask
** 功能描述: 速度监控的定时任务(1s调试一次)
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void SpeedMonitor_ScanTask(void)
{
    u8 CurrentSpeed = 0;
    CurrentSpeed=SpeedMonitor_GetCurSpeed();     
    ///////////////////////////
    s_stSpeedAttrib.curSpeed =CurrentSpeed;
    SpeedMonitor_CheckRunStatus(CurrentSpeed);
    ///////////判断是否超时停车///////////////
    SpeedMonitor_CheckOverStopTime();
    ///////////行驶记录////////////////////
    if(Gps_GetRtcAdjSta()){
    SpeedMonitor_SaveRunRecordData(CurrentSpeed); 
    }
		/////////夜间判断///////////////////////
    SpeedMonitor_CheckIsAtNightFlag();
    ////////////////////////////////
    if(s_stSpForbid.CheckVaild)
    {
        LZM_PublicSetOnceTimer(&s_stSpeedTimer[SPEED_TIMER_FORBID],PUBLICSECS(0.4),SpeedMonitor_CheckForbidAlarm);
    }    
    //读超速报警屏蔽位状态
    if(Io_ReadAlarmMaskBit(ALARM_BIT_OVER_SPEED))
    {
        s_stSpeedAttrib.overSpeedCnt = 0;
        s_stSpeedAttrib.preOverSpeedCnt = 0;
        if(s_stSpeedAttrib.stPlay.GroupTimeCnt)
        {
            s_stSpeedAttrib.stPlay.GroupTimeCnt = 0;
            Io_WriteAlarmBit(ALARM_BIT_OVER_SPEED,RESET);
        }
        /////////////////////////
        if(s_stSpeedAttrib.stPrePlay.GroupTimeCnt)
        {
            s_stSpeedAttrib.stPrePlay.GroupTimeCnt = 0;
            Io_WriteAlarmBit(ALARM_BIT_OVER_SPEED_PRE_ALARM,RESET);
        }
        return;
    }
    ///////////////////////////
    LZM_PublicSetOnceTimer(&s_stSpeedTimer[SPEED_TIMER_CHECK],PUBLICSECS(0.2),SpeedMonitor_CheckOverSpeedAlarm);     
}
/*************************************************************
** 函数名称: SpeedMonitor_TimeTask
** 功能描述: 速度监控的定时任务(1s调试一次)
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void SpeedMonitor_StartScanTask(void)
{
    LZM_PublicSetCycTimer(&s_stSpeedTimer[SPEED_TIMER_TASK],PUBLICSECS(1),SpeedMonitor_ScanTask);    
}
/*************************************************************
** 函数名称: SpeedMonitor_CheckFlash
** 功能描述: 检查速度记录的flash区域 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void SpeedMonitor_CheckFlash(void)
{
      if(Register_CheckOneArea2(REGISTER_TYPE_SPEED))
      {
           LZM_PublicSetOnceTimer(&s_stSpeedTimer[SPEED_TIMER_TASK],PUBLICSECS(0.1),SpeedMonitor_StartScanTask); 
      }
   
}
/*************************************************************
** 函数名称: SpeedMonitor_ParamInitialize
** 功能描述: 在开机时初始化
** 入口参数: 
** 出口参数: 
** 返回参数:  
** 全局变量: 无
** 调用模块:
*************************************************************/
void SpeedMonitor_ParamInitialize(void)
{
    static unsigned char sucspeedfirstRun=0;
    if(0==sucspeedfirstRun)
    {
        sucspeedfirstRun = 1;
        LZM_PublicKillTimerAll(s_stSpeedTimer,SPEED_TIMERS_MAX);
        memset((unsigned char *)&s_stSpeedAttrib,0,sizeof(s_stSpeedAttrib));
        memset((unsigned char *)&s_stSpeedParam,0,sizeof(s_stSpeedParam));
        s_stSpeedParam.speedLimit = 255;
        SetTimerTask(TIME_SPEED_MONITOR, 1);        
        s_stSpeedAttrib.runStatus = CAR_STATUS_STOP;
        s_stSpeedAttrib.prerunStatus = CAR_STATUS_STOP;        
        SpeedMonitor_UpdatePram();
        SpeedMonitor_ReadRunRecordData();
        LZM_PublicSetCycTimer(&s_stSpeedTimer[SPEED_TIMER_TASK],1,SpeedMonitor_CheckFlash);    
    }
    else
    {
        SpeedMonitor_UpdatePram();
    }
}
/*************************************************************
** 函数名称: SpeedMonitor_TimeTask
** 功能描述: 速度监控的定时任务(1s调试一次)
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
FunctionalState SpeedMonitor_TimeTask(void)
{    
    LZM_PublicTimerHandler(s_stSpeedTimer,SPEED_TIMERS_MAX);
    return ENABLE;
}

/*******************************************************************************
 *                             end of module                                   *
 *******************************************************************************/

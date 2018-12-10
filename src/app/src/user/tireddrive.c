/*******************************************************************************
 * File Name:			tireddrivere.c 
 * Function Describe:	
 * Relate Module:		判断、存储、读取超时驾驶记录
 * Writer:				Joneming
 * Date:				2012-12-08
 * ReWriter:			
 * Date:
 *******************************************************************************/

//********************************头文件************************************
#include "include.h"
//********************************自定义数据类型****************************
//********************************宏定义************************************
/////////////////////////////////
#define SAVE_TIRED_DRIVE_INFO_SECTOR   (FLASH_TIRED_DRIVE_INFO_START_SECTOR)
#define SAVE_TIRED_DRIVE_INFO_ADDR    (SAVE_TIRED_DRIVE_INFO_SECTOR*FLASH_ONE_SECTOR_BYTES)//

#ifndef FRAM_TIRED_SAVE_INDEX_ADDR//驾驶员信息保存索引位置结构体(4个字节)
#define FRAM_TIRED_SAVE_INDEX_ADDR      FRAM_START_TIME_ADDR//超时驾驶记录开车时间的地址用来保存索引位置结构体(4个字节)
#endif
#ifndef FRAM_TIRED_STOP_INFO_ADDR////停车信息结构体(16个字节)
#define FRAM_TIRED_STOP_INFO_ADDR      FRAM_END_TIME_ADDR//停车信息结构体(16个字节)
#endif
///////////////////////////////////////
#define TIRED_DRIVER_INFO_NUM           5//最大驾驶员数
#define TIRED_DRIVER_INFO_SAVE_VAL      0xaa//驾驶员索引结构体保存标志值
////////////////////////////////////
#define RESEET_CURRENT_TIRED_DRIVER_TIME_VAL    15//(秒)时间异常阀值,需要进行异常处理

static u8 TiredDriveSaveFlg = 0;
//********************************全局变量**********************************

//********************************外部变量**********************************

//********************************本地变量**********************************
/////////////////////////////////////
typedef struct
{
    unsigned long starttime;            //开车时间
    unsigned long endtime;              //停车时间
    unsigned long nightstarttime;       //晚上开始驾驶时间
    unsigned char validflag;            //当前驾驶员计时标志
    unsigned char nightvalidFlag;       //晚上开始计时标志
    unsigned char overtimeflag;         //超时驾驶标志
    unsigned char preovertimeflag;      //超时预警标志        
    unsigned char startPosition[10];    //开车位置信息
    unsigned char endPosition[10];      //停车位置信息
    unsigned char driverLicence[20];    //驾驶证(只使用前面18字节)    
    unsigned long totalDriveTime;       //当天累计驾驶时间//unsigned char drivecode[4];       //驾驶员代码(未使用)
    unsigned long preReportTime;        //前一次上报时间
}STTIREDDRIVERINFO;//单个驾驶员结构体

typedef struct
{
    STTIREDDRIVERINFO driverinfo[TIRED_DRIVER_INFO_NUM+1];//最后一条为无登录时记录
    unsigned char loginindex;               //当前登录的驾驶员编号
    unsigned char currentindex;             //当前驾驶员保存索引    
    unsigned char driverloginflag;          //驾驶员是否登录标志    
    unsigned char bak[1];                   //保留 
    unsigned long clearTime;                //清除累计驾驶的日期//unsigned char bak[5];                 //保留
}STTIREDDRIVER;//驾驶员结构体

static STTIREDDRIVER s_stTiredDriver;
#define SIZE_STTIREDDRIVER sizeof(STTIREDDRIVER)

typedef struct
{
    unsigned short currentAddr;            //当前记录的首地址
    unsigned char  index;                  //索引
    unsigned char  savedflag;              //保存标志
}STTIREDSAVEINDEX;//驾驶员结构存储索引

static STTIREDSAVEINDEX s_stTiredSaveIndex;
#define SIZE_STTIREDSAVEINDEX sizeof(STTIREDSAVEINDEX)

typedef struct
{
    STPLAY_ATTRIB stPlay;                   //超时播报
    STPLAY_ATTRIB stPrePlay;                //预警播报
    ////////////////////////
    unsigned long restTimeVal;              //休息时间,单位秒
    unsigned long overTimeVal;              //超时驾驶时间,单位秒
    unsigned long nightoverTimeVal;         //晚上超时驾驶时间,单位秒    
    unsigned long nightrestTimeVal;         //晚上休息时间,单位秒
    unsigned long oneDayoverTimeVal;        //当天累计超时驾驶时间,单位秒
    unsigned long nightStartSec;            //晚上开始秒数
    unsigned long nightStartVal;            //晚上开始时间
    unsigned long nightEndVal;              //晚上结束时间
    unsigned long curOverTimeVal;           //当前驾驶员的超时驾驶时间,单位秒    
    unsigned long curTimeVal;               //当前时钟时间,单位秒   
    unsigned short nightctrl;               //时间控制域
    //////////////////////////
    unsigned short prePlayAlarmTime;        //预警提前时间  
    unsigned short PlayAlarmTime;           //播报报警时间
    /////////////////////////////
    unsigned short ReportInterval;          //上报时间间隔(秒)
    unsigned short AllowRunTime;            //超时后允许开车时间
    /////////////////////////////////
    unsigned short runDuration;             //进入行驶状态的持续时间
    unsigned short runStatusCnt;            //行驶状态计数器
    unsigned short stopStatusCnt;           //停止状态计数器
    unsigned char runStatus;                //1为行驶，0为停驶
    unsigned char runSpeed;                 //进入行驶状态的速度    
    ///////////////////////////// ////////////////////////////////////////////
    unsigned char atnightFlag;              //晚上标志
    unsigned char needChecknight;           //需要检查晚上超时
    unsigned char firstRunflag;             //首次进入判断
    unsigned char needsaveflag;             //驾驶员结构体是否需要保存 
    unsigned char prespeedVal;              //前一次速度
    unsigned char timeAbnormalFlag;         //时间异常标记
    unsigned char restPlayCnt;              //播放次数 
    unsigned char restTimeCnt;              //播放等待时间
    unsigned char needReportflag;           //是否需要上报    
}STTIRED_ATTRIB;//超时驾驶模块属性结构体

static STTIRED_ATTRIB s_stTiredAttrib;
static unsigned char s_ucTiredModeFirstRun=0;
//////////////////////////////
typedef struct
{
    unsigned long endtime;          //停车时间
    unsigned char endPosition[10];  //停车位置信息
    unsigned char index;            //保存驾驶员索引
    unsigned char bak[1];           //保留
}STSTOPINFO;//停车信息结构体
#define  STSTOPINFO_SIZE sizeof(STSTOPINFO)

typedef enum
{
    TIRED_CAR_STATUS_STOP,    //停驶
    TIRED_CAR_STATUS_RUN,     //行驶
    TIRED_CAR_STATUS_MAX
}E_T_CAR_STATUS;//车辆行驶状态

enum 
{
    TIRED_TIMER_TASK,                       //
    TIRED_TIMER_PLAY,						//
    TIRED_TIMERS_MAX
}STTIRED_TIME;
static LZM_TIMER s_stTiredTimer[TIRED_TIMERS_MAX];
/********************************函数声明**********************************/
/*************************************************************
** 函数名称: TiredDrive_CheckCurrentDriverStatus
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void TiredDrive_DisposeOneDriverInfoTimeIsAbnormal(unsigned char index);
void TiredDrive_CheckAndSaveTiredDriverInfo(void);
//********************************函数定义***********************************
/*************************************************************
** 函数名称: TiredDrive_SaveFramTimeToDriverInfo
** 功能描述: 
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void TiredDrive_ShowDebugInfo(unsigned char index,unsigned char *str)
{
    #if 0
    TIME_T stTime;
    Gmtime(&stTime,s_stTiredAttrib.curTimeVal);
    LOG_PR("\r\n %s index:%d  ct:%02d:%02d:%02d (%d);st:%d; eT:%d \r\n ",str,index,stTime.hour,stTime.min,stTime.sec,s_stTiredAttrib.curTimeVal,s_stTiredDriver.driverinfo[index].starttime,s_stTiredDriver.driverinfo[index].endtime);
    #endif
}
/*************************************************************
** 函数名称: TiredDrive_ClearAlltotalDriveTime
** 功能描述: 清除当天累计超时时间
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void TiredDrive_ClearAlltotalDriveTime(void)
{
    unsigned char index;
    unsigned long curTime;    
    for(index=0;index<=TIRED_DRIVER_INFO_NUM;index++)
    {
        s_stTiredDriver.driverinfo[index].totalDriveTime = 0;
    }
    /////////////////////
    curTime = s_stTiredAttrib.curTimeVal;
    s_stTiredDriver.clearTime = curTime-(curTime%DAY_SECOND);//清除累计驾驶的日期
    s_stTiredAttrib.needsaveflag = 1;
}
/*************************************************************
** 函数名称: TiredDrive_ClearAlltotalDriveTime
** 功能描述: 检查及清除当天累计超时时间
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void TiredDrive_CheckAndCleartotalDriveTime(void)
{
    if((s_stTiredAttrib.curTimeVal < s_stTiredDriver.clearTime)
     ||(s_stTiredAttrib.curTimeVal>=s_stTiredDriver.clearTime + DAY_SECOND))
    {
        TiredDrive_ClearAlltotalDriveTime();
    }    
}
/*************************************************************
** 函数名称: TiredDrive_GetCurtotalDriveTime
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned long TiredDrive_GetCurtotalDriveTime(unsigned char index)
{
    unsigned long val;
    if(0==s_stTiredDriver.driverinfo[index].validflag)return 0;
    val = s_stTiredDriver.driverinfo[index].starttime;
    if(s_stTiredDriver.driverinfo[index].starttime < s_stTiredDriver.clearTime)
    {
        val = s_stTiredDriver.clearTime;
    }
    ////////////////////
    if(s_stTiredDriver.driverinfo[index].endtime > val)
    {
        val = s_stTiredDriver.driverinfo[index].endtime - val;
    }
    else
    {
        val = 0;
    }
    return val;
}
/*************************************************************
** 函数名称: TiredDrive_SaveCurtotalDriveTime
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void TiredDrive_SaveCurtotalDriveTime(unsigned char index)
{
    if(0==s_stTiredDriver.driverinfo[index].validflag)return;
    if(s_stTiredDriver.driverinfo[index].endtime < s_stTiredDriver.clearTime)return;
    if(s_stTiredDriver.driverinfo[index].starttime > s_stTiredDriver.clearTime + DAY_SECOND)return;
    s_stTiredDriver.driverinfo[index].totalDriveTime += TiredDrive_GetCurtotalDriveTime(index);
}
/*************************************************************
** 函数名称: TiredDrive_CheckDriverRestStatus
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void TiredDrive_ResetCurDriverData(unsigned char index)
{
    TiredDrive_SaveCurtotalDriveTime(index);
    //////////////////////////////////
    s_stTiredDriver.driverinfo[index].validflag         = 0;
    s_stTiredDriver.driverinfo[index].starttime         = 0;
    s_stTiredDriver.driverinfo[index].endtime           = 0;
    s_stTiredDriver.driverinfo[index].overtimeflag      = 0; 
    s_stTiredDriver.driverinfo[index].preReportTime     = 0;
    s_stTiredDriver.driverinfo[index].nightvalidFlag    = 0;   
    s_stTiredDriver.driverinfo[index].nightstarttime    = 0; 
    s_stTiredDriver.driverinfo[index].preovertimeflag   = 0;    
}
/*************************************************************
** 函数名称: TiredDrive_GetCurRunStatus
** 功能描述: 获取超时驾驶当前车辆的状态(行驶或停止)
** 入口参数: 
** 出口参数: 
** 返回参数: 1为行驶，0为停止
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned char TiredDrive_GetCurRunStatus(void)
{
    if(0==s_stTiredAttrib.runSpeed)
    {
        return SpeedMonitor_GetCurRunStatus();
    }    
    else
    {
        return(TIRED_CAR_STATUS_RUN==s_stTiredAttrib.runStatus)?1:0;
    }
}
/*************************************************************
** 函数名称: TiredDrive_CheckRunStatus
** 功能描述: 判断当前是处于行驶状态还是停驶状态(1s调试一次)
** 入口参数: CurrentSpeed已根据s_stTiredAttrib.runSpeed转换
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void TiredDrive_CheckRunStatus(unsigned char CurrentSpeed)
{
    //unsigned char ACC;
    if(0==s_stTiredAttrib.runSpeed)return;
    ///////////////////////////
    //ACC = Io_ReadStatusBit(STATUS_BIT_ACC);	
    if(CurrentSpeed)//
    {
        s_stTiredAttrib.stopStatusCnt = 0;//停止状态计数器清0
        if(s_stTiredAttrib.runStatusCnt < s_stTiredAttrib.runDuration)//是否持续够10s,不够时,不改变当前状态
        {
            s_stTiredAttrib.runStatusCnt++;//行驶状态计数器++
        }
        else//已经持续10s,则设为行驶状态
        {
            s_stTiredAttrib.runStatus = TIRED_CAR_STATUS_RUN;
        }
    }
    else
    {
        s_stTiredAttrib.runStatusCnt = 0;//行驶状态计数器清0
        s_stTiredAttrib.runStatus = TIRED_CAR_STATUS_STOP;
    }
}
/*************************************************************
** 函数名称: TiredDrive_SaveFramTimeToDriverInfo
** 功能描述: 
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void TiredDrive_SaveFramTimeToDriverInfo(unsigned char index)
{
    unsigned len;
    STSTOPINFO stTmpStopinfo;
    if(0==s_stTiredDriver.driverinfo[index].validflag)return;
    len=Public_ReadDataFromFRAM(FRAM_TIRED_STOP_INFO_ADDR,(unsigned char *)&stTmpStopinfo,STSTOPINFO_SIZE);
    if(STSTOPINFO_SIZE==len)//新的
    {
        if(index==stTmpStopinfo.index) 
        {
            s_stTiredDriver.driverinfo[index].endtime = stTmpStopinfo.endtime;
            memcpy(s_stTiredDriver.driverinfo[index].endPosition,stTmpStopinfo.endPosition,10);
        }
    }
}
/*************************************************************
** 函数名称: TiredDrive_SaveDriverStopInfoToFRAM
** 功能描述: 
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void TiredDrive_SaveDriverStopInfoToFRAM(unsigned char index)
{
    STSTOPINFO stTmpStopinfo;
    if(0==s_stTiredDriver.driverinfo[index].validflag)return;
    stTmpStopinfo.index = index;
    stTmpStopinfo.endtime =s_stTiredDriver.driverinfo[index].endtime;
    memcpy(stTmpStopinfo.endPosition,s_stTiredDriver.driverinfo[index].endPosition,10);
    Public_WriteDataToFRAM(FRAM_TIRED_STOP_INFO_ADDR,(unsigned char *)&stTmpStopinfo,STSTOPINFO_SIZE);
}

/*************************************************************
** 函数名称: TiredDrive_SaveCurStopInfo
** 功能描述: 
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void TiredDrive_SaveCurStopInfo(unsigned char index,unsigned long currenttime)
{
    s_stTiredDriver.driverinfo[index].endtime = currenttime;
    Public_GetCurPositionInfoDataBlock(s_stTiredDriver.driverinfo[index].endPosition);
    TiredDrive_SaveDriverStopInfoToFRAM(index);
}
/*************************************************************
** 函数名称: TiredDrive_SaveCurStopInfo
** 功能描述: 
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void TiredDrive_SaveCurStartInfo(unsigned char index,unsigned long currenttime)
{
    s_stTiredDriver.driverinfo[index].starttime = currenttime;
    s_stTiredDriver.driverinfo[index].endtime   = currenttime;
    Public_GetCurPositionInfoDataBlock(s_stTiredDriver.driverinfo[index].startPosition);
}
/*************************************************************
** 函数名称: TiredDrive_SaveTiredSaveIndexToFRAM
** 功能描述: 
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void TiredDrive_SaveTiredSaveIndexToFRAM(void) 
{
    s_stTiredSaveIndex.savedflag =TIRED_DRIVER_INFO_SAVE_VAL;
    Public_WriteDataToFRAM(FRAM_TIRED_SAVE_INDEX_ADDR,(unsigned char *)&s_stTiredSaveIndex, SIZE_STTIREDSAVEINDEX);
}
/*************************************************************
** 函数名称: TiredDrive_SaveTiredDriverInfoToFlash
** 功能描述: 把驾驶员信息保存到flash
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void TiredDrive_SaveTiredDriverInfoToFlash(void) 
{
    unsigned char buffer[10];
    unsigned long val;
    s_stTiredSaveIndex.index++;
    s_stTiredSaveIndex.currentAddr=(s_stTiredSaveIndex.index-1)*SIZE_STTIREDDRIVER;    
    if(s_stTiredSaveIndex.currentAddr+SIZE_STTIREDDRIVER>FLASH_ONE_SECTOR_BYTES)
    {
        s_stTiredSaveIndex.index = 1;
        s_stTiredSaveIndex.currentAddr =0;
    }
    ////////检查是否为空////////////////////
    sFLASH_ReadBuffer((unsigned char *)&buffer,SAVE_TIRED_DRIVE_INFO_ADDR+s_stTiredSaveIndex.currentAddr,4);
    val=Public_ConvertBufferToLong(buffer);
    if(val != 0xffffffff)//保正可以存储
    {
        sFLASH_EraseSector(SAVE_TIRED_DRIVE_INFO_ADDR);
    }
    ////////////////////////////
    sFLASH_WriteBuffer((unsigned char *)&s_stTiredDriver,SAVE_TIRED_DRIVE_INFO_ADDR+s_stTiredSaveIndex.currentAddr,SIZE_STTIREDDRIVER);
    ///////////////////
    TiredDrive_SaveTiredSaveIndexToFRAM();
    ///////////////////////////////////////
    //TiredDrive_ShowDebugInfo(s_stTiredDriver.currentindex,"save");
}
/*************************************************************
** 函数名称: TiredDrive_TiredDriverInfoInit
** 功能描述: 超时驾驶相关驾驶员信息初始化
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void TiredDrive_TiredDriverInfoInit(void)
{
    s_stTiredSaveIndex.savedflag = TIRED_DRIVER_INFO_SAVE_VAL;
    s_stTiredSaveIndex.index = 0;
    s_stTiredSaveIndex.currentAddr = 0;
    memset(&s_stTiredDriver,0,SIZE_STTIREDDRIVER);
    ///////////////////////////////
    memset(&s_stTiredDriver.driverinfo[TIRED_DRIVER_INFO_NUM].driverLicence,0x30,18);
    s_stTiredAttrib.needsaveflag = 1;
    TiredDrive_CheckAndSaveTiredDriverInfo();
}
/*************************************************************
** 函数名称: TiredDrive_ReadTiredDriverInfoFromFlash
** 功能描述: 
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void TiredDrive_ReadTiredDriverInfoFromFlash(void) 
{
    unsigned char flag;
    /////////////////////////////
    flag = Public_ReadDataFromFRAM(FRAM_TIRED_SAVE_INDEX_ADDR,(unsigned char *)&s_stTiredSaveIndex, SIZE_STTIREDSAVEINDEX);
    if((s_stTiredSaveIndex.savedflag != TIRED_DRIVER_INFO_SAVE_VAL)||(s_stTiredSaveIndex.currentAddr > FLASH_ONE_SECTOR_BYTES)
        ||(s_stTiredSaveIndex.currentAddr%SIZE_STTIREDDRIVER != 0)||(0==flag))
    {
        TiredDrive_TiredDriverInfoInit();
    }    
    else
    {
        sFLASH_ReadBuffer((unsigned char *)&s_stTiredDriver,SAVE_TIRED_DRIVE_INFO_ADDR+s_stTiredSaveIndex.currentAddr,SIZE_STTIREDDRIVER);
        ///////////合法判断////////////
        if((s_stTiredDriver.loginindex>TIRED_DRIVER_INFO_NUM)||(s_stTiredDriver.currentindex>TIRED_DRIVER_INFO_NUM)||(s_stTiredDriver.driverloginflag>1)
         ||(s_stTiredDriver.driverloginflag&&(0==s_stTiredDriver.loginindex||s_stTiredDriver.loginindex!=s_stTiredDriver.currentindex+1))
         ||(0==s_stTiredDriver.driverloginflag&&(s_stTiredDriver.loginindex==s_stTiredDriver.currentindex+1||s_stTiredDriver.currentindex!=TIRED_DRIVER_INFO_NUM)))
        {
            memset(&s_stTiredDriver,0,SIZE_STTIREDDRIVER);
            /////////////////////////////
            memset(&s_stTiredDriver.driverinfo[TIRED_DRIVER_INFO_NUM].driverLicence,0x30,18);
        }
        else/////////
        {
            if((0==s_stTiredDriver.driverloginflag&&TIRED_DRIVER_INFO_NUM==s_stTiredDriver.currentindex)
              ||(s_stTiredDriver.driverloginflag&&s_stTiredDriver.loginindex==s_stTiredDriver.currentindex+1))
            TiredDrive_SaveFramTimeToDriverInfo(s_stTiredDriver.currentindex);
        }
    }
    //////////////////////////////
    s_stTiredAttrib.firstRunflag = 0;//
    if(s_stTiredDriver.driverloginflag)
    {
        Io_WriteSelfDefine2Bit(DEFINE_BIT_5,SET);
    }
}
/*************************************************************
** 函数名称: TiredDrive_CheckTimeAtNight
** 功能描述: 判断时间是否为晚上时间段
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned char TiredDrive_CheckTimeAtNight(unsigned long checkTime)
{
    unsigned char atnightFlag;
    if(!s_stTiredAttrib.needChecknight)return 0;//不需要检查晚上超时
    atnightFlag =Public_CheckTimeIsInTimeRange(s_stTiredAttrib.nightctrl,checkTime,s_stTiredAttrib.nightStartVal,s_stTiredAttrib.nightEndVal);    
    return atnightFlag;
}
/*************************************************************
** 函数名称: TiredDrive_ClearDriverInfo
** 功能描述: 清除存储驾驶员信息中的驾驶员结构体
** 入口参数: 驾驶员结构体索引号(从0开始)
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void TiredDrive_ClearDriverInfo(unsigned char index)
{
    TiredDrive_ResetCurDriverData(index);
    #if 0
    memset(&s_stTiredDriver.driverinfo[index],0,sizeof(STTIREDDRIVERINFO));
    if(TIRED_DRIVER_INFO_NUM == index)//
    {
        memset(&s_stTiredDriver.driverinfo[index].driverLicence,0x30,18);
    }
    #endif
}
/*************************************************************
** 函数名称: TiredDrive_GetDriverInfoOverTime
** 功能描述: 获得指定驾驶员的超时时间点
** 入口参数: index驾驶员索引号
** 出口参数: 
** 返回参数: 超时时间点
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned long TiredDrive_GetDriverInfoOverTime(unsigned char index)
{
    unsigned long overTimeVal,tmpVal1;
    if(1 != s_stTiredDriver.driverinfo[index].validflag)return 0;
    if(0 == s_stTiredDriver.driverinfo[index].starttime)return 0;
    if(0xffffffff == s_stTiredAttrib.overTimeVal)return 0;
    overTimeVal = s_stTiredDriver.driverinfo[index].starttime+s_stTiredAttrib.overTimeVal;
    if(s_stTiredDriver.driverinfo[index].nightvalidFlag&&s_stTiredAttrib.needChecknight)
    {
        if(TiredDrive_CheckTimeAtNight(s_stTiredDriver.driverinfo[index].nightstarttime+s_stTiredAttrib.nightoverTimeVal))
        {
            tmpVal1=s_stTiredDriver.driverinfo[index].nightstarttime+s_stTiredAttrib.nightoverTimeVal;
            if(tmpVal1 < overTimeVal)
            {
                overTimeVal = tmpVal1;
            }
        }
    }
    //////////////////////////////
    return overTimeVal;//最小值
}
/*************************************************************
** 函数名称: TiredDrive_CheckDriverIsTired
** 功能描述: 判断指定驾驶员是否已超时
** 入口参数: index驾驶员索引号
** 出口参数: 
** 返回参数: 1:已超时,0:未超时
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned char TiredDrive_CheckDriverIsTired(unsigned char index)
{
    unsigned long overtime;
    if(1==s_stTiredDriver.driverinfo[index].validflag)
    {
        overtime=TiredDrive_GetDriverInfoOverTime(index);
        if(overtime&&s_stTiredDriver.driverinfo[index].endtime>=overtime)
        {
            return 1;
        }
    }
    return 0;
}
/*************************************************************
** 函数名称: TiredDrive_GetOneTiredDriveFromFram
** 功能描述: 从铁电里读取一个超时驾驶记录(所有驾驶员)
** 入口参数: 
** 出口参数: 
** 返回参数:
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned char TiredDrive_GetOneTiredDriveFromFram(unsigned char *pBuffer,unsigned char index)
{
    unsigned char buffer1[20];
    unsigned short len;
    unsigned char temp;
    TIME_T tt;
    
    len = 0;
    temp =18;
    memcpy(&pBuffer[len],s_stTiredDriver.driverinfo[index].driverLicence,temp);
    len +=temp;
    //////////
    temp = 6;
    Gmtime(&tt, s_stTiredDriver.driverinfo[index].starttime);
    Public_ConvertTimeToBCDEx(tt,buffer1);
    memcpy(&pBuffer[len],buffer1,temp);
    len += temp;
    Gmtime(&tt, s_stTiredDriver.driverinfo[index].endtime);
    Public_ConvertTimeToBCDEx(tt,buffer1);
    memcpy(&pBuffer[len],buffer1,temp);
    len += temp;
    //////////////////////
    temp = 10;
    memcpy(&pBuffer[len],s_stTiredDriver.driverinfo[index].startPosition,temp);
    len +=temp;
    memcpy(&pBuffer[len],s_stTiredDriver.driverinfo[index].endPosition,temp);
    len +=temp;
    ////////////////////
    return ONE_TIRED_DRIVER_DATA_LEN;
}
/*************************************************************
** 函数名称: TiredDrive_ReadTiredDriveFromFram
** 功能描述: 从铁电里读取超时驾驶记录(所有驾驶员)
** 入口参数: 
** 出口参数: 保存数据的首地址
** 返回参数: pBuffer字节长度
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned short TiredDrive_ReadTiredDriveFromFram(unsigned char *pBuffer)
{
    unsigned char i;
    unsigned short len;
    unsigned char *p;
    p = pBuffer;
    //****************从铁电里查找*****************
    len =0;
    for(i=0; i<=TIRED_DRIVER_INFO_NUM;i++)
    {
        if(1==s_stTiredDriver.driverinfo[i].validflag)
        {
            if(TiredDrive_CheckDriverIsTired(i))
            {
                len += TiredDrive_GetOneTiredDriveFromFram(&p[len],i);
            }
        }
    }
    return len;        
}
/*********************************************************************
//函数名称	:TiredDrive_WriteTiredRecordToFlash
//功能		:存储一条超时驾驶记录到Flash
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void TiredDrive_WriteTiredRecordToFlash(unsigned char *pBuffer, unsigned char length,unsigned long endtime)
{
    Register_Write2(REGISTER_TYPE_OVER_TIME,pBuffer,length,endtime);
}
/*************************************************************
** 函数名称: TiredDrive_SaveOvertimeDriverRecordToFlash
** 功能描述: 保存超时驾驶记录到Flash
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void TiredDrive_SaveOvertimeDriverRecordToFlash(unsigned char index)
{
    unsigned char buffer[SAVE_ONE_TIRED_DRIVER_DATA_LEN+1]={0};
    if(index<=TIRED_DRIVER_INFO_NUM)//
    {
        TiredDrive_GetOneTiredDriveFromFram(buffer,index);        
        TiredDrive_WriteTiredRecordToFlash(buffer,SAVE_ONE_TIRED_DRIVER_DATA_LEN,s_stTiredDriver.driverinfo[index].endtime);
    }    
}
/*************************************************************
** 函数名称: TiredDrive_GetCurRestTimeVal
** 功能描述: 取得当前最小休息时间
** 入口参数: endtime:停车时间点
** 出口参数: 
** 返回参数: 最小休息时间
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned long TiredDrive_GetCurRestTimeVal(unsigned long endtime)
{
    static unsigned long sulEndtime=0;
    static unsigned long restTimeVal=0;    
    if(0==s_stTiredAttrib.needChecknight)return s_stTiredAttrib.restTimeVal;
    if(s_stTiredAttrib.nightrestTimeVal == s_stTiredAttrib.restTimeVal)return s_stTiredAttrib.restTimeVal;
    if(sulEndtime == endtime)return restTimeVal;
    if(Public_CheckTimeIsInTimeRange(s_stTiredAttrib.nightctrl,endtime,s_stTiredAttrib.nightStartVal,s_stTiredAttrib.nightEndVal))
    {
        restTimeVal = s_stTiredAttrib.nightrestTimeVal;
    }
    else
    {
        restTimeVal = s_stTiredAttrib.restTimeVal;
    }
    sulEndtime = endtime;
    return restTimeVal;
}
/*************************************************************
** 函数名称: TiredDrive_CheckDriverRestStatus
** 功能描述: 判断是否已达到最小休息时间
** 入口参数: currenttime:需要判断的时间点，index:驾驶员索引号
** 出口参数: 
** 返回参数: 1:已达到,0:未达到
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned char TiredDrive_CheckDriverRestStatus(unsigned long currenttime,unsigned char index)
{
    unsigned long overtime,restTimeVal;
    if((currenttime+10<s_stTiredDriver.driverinfo[index].endtime)||(s_stTiredDriver.driverinfo[index].endtime+10 < s_stTiredDriver.driverinfo[index].starttime))
    {
        return 1;//时间格式出错
    }
    else
    {
        restTimeVal = TiredDrive_GetCurRestTimeVal(s_stTiredDriver.driverinfo[index].endtime);
        if(currenttime>=s_stTiredDriver.driverinfo[index].endtime+restTimeVal)//超过休息时间
        {
            overtime = TiredDrive_GetDriverInfoOverTime(index);
            if(overtime&&s_stTiredDriver.driverinfo[index].endtime >= overtime)
            {
                ///////////保存超时驾驶记录//////////////////
                //因为RTC电池没电导致在GPS时间校准之前会出现时间错乱的问题，所以在没有GPS校时之前不存储行驶记录仪数据，这也是没有办法的办法....
								if(TiredDriveSaveFlg)
								{
                	TiredDrive_SaveOvertimeDriverRecordToFlash(index);
								}
            }
            return 1;
        }
    }
    return 0;
}
/*************************************************************
** 函数名称: TiredDrive_ReadTiredDriveFromFramEx
** 功能描述: 从铁电里读取指定时间段超时驾驶记录(所有驾驶员)
** 入口参数: 起始时间StartTime,结束时间EndTime
** 出口参数: pBuffer保存数据的首地址
** 返回参数: pBuffer字节长度
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned short TiredDrive_ReadTiredDriveFromFramEx(unsigned char *pBuffer,TIME_T StartTime,TIME_T EndTime)
{
    unsigned long startVal,endVal;
    unsigned char i;
    unsigned short len;
    unsigned char *p;
    p = pBuffer;
    startVal = ConverseGmtime(&StartTime);
    endVal = ConverseGmtime(&EndTime);
    //****************从铁电里查找*****************
    len =0;
    for(i=0; i<=TIRED_DRIVER_INFO_NUM;i++)
    {
        if(1==s_stTiredDriver.driverinfo[i].validflag)
        {
            if(TiredDrive_CheckDriverIsTired(i))
            {
                if((startVal<=s_stTiredDriver.driverinfo[i].starttime&&endVal>s_stTiredDriver.driverinfo[i].starttime)
                  ||(startVal<s_stTiredDriver.driverinfo[i].endtime&&endVal>=s_stTiredDriver.driverinfo[i].endtime)
                  ||(startVal>s_stTiredDriver.driverinfo[i].starttime&&endVal<s_stTiredDriver.driverinfo[i].endtime))
                {
                    len += TiredDrive_GetOneTiredDriveFromFram(&p[len],i);
                }
            }
        }
    }
    return len;        
}
/*********************************************************************
** 函数名称: TiredDrive_UpdatePram
** 功能描述: 设置超时驾驶参数时，初始化超时驾驶变量
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void TiredDrive_UpdatePram(void)
{
    unsigned char Buffer[50] = {0};
    unsigned char len;
    unsigned long tmpVal;
    ////////////////////////////  
    s_stTiredAttrib.firstRunflag =0;
    //////////////////
    len = EepromPram_ReadPram(E2_MAX_DRVIE_TIME_ID, Buffer);
    if(E2_MAX_DRVIE_TIME_LEN == len)
    {
        s_stTiredAttrib.overTimeVal=Public_ConvertBufferToLong(Buffer);
    }
    else
    {
        s_stTiredAttrib.overTimeVal = 14400;//出错,设置为默认值,240分钟
    }
    len = EepromPram_ReadPram(E2_MIN_RELAX_TIME_ID, Buffer);
    if(E2_MIN_RELAX_TIME_LEN == len)
    {
        s_stTiredAttrib.restTimeVal=Public_ConvertBufferToLong(Buffer);
    }
    else
    {
        s_stTiredAttrib.restTimeVal = 1200;//出错,设置为默认值,20分钟
    }
    /////当天累计驾驶时间门限//单位为秒//默认为0xffffffff,不开启该功能///////////
    len = EepromPram_ReadPram(E2_MAX_DRVIE_TIME_ONEDAY_ID, Buffer);
    if(E2_MAX_DRVIE_TIME_ONEDAY_LEN == len)
    {
        s_stTiredAttrib.oneDayoverTimeVal = Public_ConvertBufferToLong(Buffer);
    }
    else
    {
        s_stTiredAttrib.oneDayoverTimeVal = 0xffffffff;//
    }   
    /////////夜间超时驾驶门限////////////////////////
    s_stTiredAttrib.nightStartVal = 0;
    s_stTiredAttrib.nightEndVal = 0;    
    s_stTiredAttrib.needChecknight = 0;
    s_stTiredAttrib.nightrestTimeVal = 0;
    s_stTiredAttrib.nightoverTimeVal = 0;    
    len =EepromPram_ReadPram(E2_NIGHT_MAX_DRVIE_TIME_ID, Buffer);
    if(E2_NIGHT_MAX_DRVIE_TIME_ID_LEN== len)
    {
        tmpVal=Public_ConvertBufferToLong(Buffer);
        if((0xffffffff!=tmpVal)&&(tmpVal<s_stTiredAttrib.overTimeVal))//夜间超时驾驶的门限一定比白天的门限值小
        {
            s_stTiredAttrib.nightoverTimeVal = tmpVal;
        }
    }
    ////////////////////////////////
    len = EepromPram_ReadPram(E2_NIGHT_MIN_RELAX_TIME_ID, Buffer);
    if(E2_NIGHT_MIN_RELAX_TIME_ID_LEN == len)
    {
        s_stTiredAttrib.nightrestTimeVal=Public_ConvertBufferToLong(Buffer);
    }
    else
    {
        s_stTiredAttrib.nightrestTimeVal = 1200;//出错,设置为默认值,20分钟
    }
    ////////夜间时间范围,BCD码//////////////////////
    len =EepromPram_ReadPram(E2_TIRED_DRIVE_NIGHT_TIME_ID, Buffer);
    if(len)
    {
        s_stTiredAttrib.nightctrl = Public_ConvertBufferToShort(Buffer);        
        s_stTiredAttrib.nightStartVal=Public_ConvertBCDTimeToLong(s_stTiredAttrib.nightctrl,&Buffer[2]);
        if(s_stTiredAttrib.nightStartVal)
        {
            s_stTiredAttrib.nightStartSec = 0;
            s_stTiredAttrib.nightStartSec = s_stTiredAttrib.nightStartSec*24 + Public_BCD2HEX(Buffer[5]); //获得时
            s_stTiredAttrib.nightStartSec = s_stTiredAttrib.nightStartSec*60 + Public_BCD2HEX(Buffer[6]); //获得分
            s_stTiredAttrib.nightStartSec = s_stTiredAttrib.nightStartSec*60 + Public_BCD2HEX(Buffer[7]); //获得秒
            ////////////////////////////////////////////////////
            s_stTiredAttrib.nightEndVal=Public_ConvertBCDTimeToLong(s_stTiredAttrib.nightctrl,&Buffer[8]);            
            if(0==s_stTiredAttrib.nightEndVal)
            {
                s_stTiredAttrib.nightStartVal = 0;
            }
        }
    }    
    /////////////////////////////////
    if(s_stTiredAttrib.nightoverTimeVal&&Public_CheckTimeRangeIsValid(s_stTiredAttrib.nightctrl,s_stTiredAttrib.nightStartVal,s_stTiredAttrib.nightEndVal))
    {
        s_stTiredAttrib.needChecknight = 1;
    } 
    ///////疲劳驾驶预警差值/////////////////
    len =EepromPram_ReadPram(E2_TIRE_EARLY_ALARM_DVALUE_ID, Buffer);
    if(E2_TIRE_EARLY_ALARM_DVALUE_LEN== len)
    {
        s_stTiredAttrib.prePlayAlarmTime = Public_ConvertBufferToShort(Buffer);
    }
    else
    {
        s_stTiredAttrib.prePlayAlarmTime = 1800;
    }
    ///////////超时驾驶报警提示语音有效时间,
    len=EepromPram_ReadPram(E2_TIRED_DRIVE_ALARM_TIME_ID, Buffer);
    if(len)
    {
        s_stTiredAttrib.PlayAlarmTime = Public_ConvertBufferToLong(Buffer)*60;
    }
    else
    {
        s_stTiredAttrib.PlayAlarmTime = 1800;
    }
    ////////预警/////超时驾驶预警每组提示时间间隔////////////////// 
    len=EepromPram_ReadPram(E2_TIRED_DRIVE_PREALARM_GPOUP_TIME_ID, Buffer);
    if(len)
    {
        s_stTiredAttrib.stPrePlay.GroupTime = Public_ConvertBufferToLong(Buffer)*60;
    }
    else
    {
        s_stTiredAttrib.stPrePlay.GroupTime = 300;
    }
    
    len=EepromPram_ReadPram(E2_TIRED_DRIVE_PREALARM_NUMBER_ID, Buffer);
    if(len)
    {
        s_stTiredAttrib.stPrePlay.OnceNumber = Public_ConvertBufferToLong(Buffer);
    }
    else
    {
        s_stTiredAttrib.stPrePlay.OnceNumber = 3;
    }
    len=EepromPram_ReadPram(E2_TIRED_DRIVE_PREALARM_NUMBER_TIME_ID, Buffer);
    if(len)
    {
        s_stTiredAttrib.stPrePlay.OnceInterval = Public_ConvertBufferToLong(Buffer);
    }
    else
    {
        s_stTiredAttrib.stPrePlay.OnceInterval = 10;
    }
    ////////////超时////////////////////////////
    len=EepromPram_ReadPram(E2_TIRED_DRIVE_ALARM_GPOUP_TIME_ID, Buffer);
    if(len)
    {
        s_stTiredAttrib.stPlay.GroupTime = Public_ConvertBufferToLong(Buffer)*60;
    }
    else
    {
        s_stTiredAttrib.stPlay.GroupTime = 300;
    }
    len=EepromPram_ReadPram(E2_TIRED_DRIVE_ALARM_NUMBER_ID, Buffer);
    if(len)
    {
        s_stTiredAttrib.stPlay.OnceNumber = Public_ConvertBufferToLong(Buffer);
    }
    else
    {
        s_stTiredAttrib.stPlay.OnceNumber = 3;
    }    
    len=EepromPram_ReadPram(E2_TIRED_DRIVE_ALARM_NUMBER_TIME_ID, Buffer);
    if(len)
    {
        s_stTiredAttrib.stPlay.OnceInterval = Public_ConvertBufferToLong(Buffer);
    }
    else
    {
        s_stTiredAttrib.stPlay.OnceInterval = 10;
    }
    len=EepromPram_ReadPram(E2_TIRED_DRIVE_RUN_SPEED_ID, Buffer);
    if(len)
    {
        s_stTiredAttrib.runSpeed = Public_ConvertBufferToLong(Buffer);
    }
    else
    {
        s_stTiredAttrib.runSpeed = 0;
    }
    len=EepromPram_ReadPram(E2_TIRED_DRIVE_RUN_KEEPTIME_ID, Buffer);
    if(len)
    {
        s_stTiredAttrib.runDuration = Public_ConvertBufferToLong(Buffer);
    }
    else
    {
        s_stTiredAttrib.runDuration = 10;
    }
    //////超时驾驶报警位置上报时间间隔////////////////////////////////
    len=EepromPram_ReadPram(E2_TIRED_DRIVE_REPORT_TIME_ID,Buffer);
    if(len == E2_TIRED_DRIVE_REPORT_TIME_ID_LEN)
    {
        s_stTiredAttrib.ReportInterval = Public_ConvertBufferToLong(Buffer);
    }
    else
    {
        s_stTiredAttrib.ReportInterval = 0;
    }
    /////超时后允许开车时间/////////
    len = EepromPram_ReadPram(E2_TIRED_DRIVE_REPORT_OPEN_TIME_ID,Buffer);
    if(len == E2_TIRED_DRIVE_REPORT_OPEN_TIME_ID_LEN)
    {
        s_stTiredAttrib.AllowRunTime = Public_ConvertBufferToLong(Buffer);
    }
    else
    {
        s_stTiredAttrib.AllowRunTime = 1200;//20分钟
    }
    //////////////////////////////////////
    if(0xffffffff==s_stTiredAttrib.oneDayoverTimeVal)///不检查
    {
        Io_WriteAlarmBit(ALARM_BIT_DAY_OVER_DRIVE, RESET);
    }
}

/*************************************************************
** 函数名称: TiredDrive_Login
** 功能描述: 驾驶员登录时调用(初始化相关变量)
** 入口参数: 
** 出口参数: 无
** 返回参数: 1:登录成功,0:登录失败
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char TiredDrive_Login(void)
{
    unsigned char Buffer[21] = {0};
    unsigned char i,index,flag,loginsucceed;
    if(0==s_ucTiredModeFirstRun)//在登录之前一定要先初始化模块的参数
    {
        TiredDrive_ParamInitialize();
    }
    //读当前登录的驾驶证号码
    flag = 0;
    loginsucceed =0;
    EepromPram_ReadPram(E2_CURRENT_LICENSE_ID, Buffer);
    /////////保存前一次//////////////////////////
    TiredDrive_SaveFramTimeToDriverInfo(s_stTiredDriver.currentindex);
    /////////////////////////////////
    for(i=0; i< TIRED_DRIVER_INFO_NUM; i++)
    {
        if(1 != s_stTiredDriver.driverinfo[i].validflag)//如果不是正在计时,则清空
        {
            TiredDrive_ClearDriverInfo(i);
        }
        ////////////////
        //if(1 == s_stTiredDriver.driverinfo[i].validflag))//在计时有效中查找
        if(s_stTiredDriver.driverinfo[i].totalDriveTime||(1 == s_stTiredDriver.driverinfo[i].validflag))//找到资料
        {
            if(strncmp((char *)Buffer,(char *)&s_stTiredDriver.driverinfo[i].driverLicence[0],18)==0)
            {
                flag = 1;
                s_stTiredDriver.loginindex = i+1;
            }
        }
    } 
    /////////////////////////////
    if(0==flag)//没找到相同的驾驶员
    {
        for(i=0; i< TIRED_DRIVER_INFO_NUM; i++)
        { 
            //if(0 == s_stTiredDriver.driverinfo[i].validflag)
            if(0 == s_stTiredDriver.driverinfo[i].totalDriveTime&&0==s_stTiredDriver.driverinfo[i].validflag)
            {
                loginsucceed = 1;
                s_stTiredDriver.loginindex = i+1;//找到空位 
                memcpy((char *)s_stTiredDriver.driverinfo[i].driverLicence,(char *)Buffer,18);                
                //EepromPram_ReadPram(E2_CURRENT_DRIVER_ID, s_stTiredDriver.driverinfo[i].drivecode);
                break;
            }
        }
    }
    else
    if(1==flag)//找到相同的驾驶员资料
    {
        loginsucceed = 1;
        index = s_stTiredDriver.loginindex-1;
        TiredDrive_SaveDriverStopInfoToFRAM(index);
        /////////////////////////////////////
        //EepromPram_ReadPram(E2_CURRENT_DRIVER_ID, s_stTiredDriver.driverinfo[index].drivecode);
    }
    ////////////////////
    if(0==s_stTiredDriver.driverinfo[TIRED_DRIVER_INFO_NUM].validflag)
    TiredDrive_ClearDriverInfo(TIRED_DRIVER_INFO_NUM);
    ////////////////////////////////////
    memset(&s_stTiredDriver.driverinfo[TIRED_DRIVER_INFO_NUM].driverLicence,0x30,18);
    ///////////////////    
    s_stTiredAttrib.firstRunflag    = 0;
    s_stTiredDriver.driverloginflag = 1; 
    s_stTiredAttrib.needsaveflag    = 1;
    s_stTiredDriver.currentindex    = s_stTiredDriver.loginindex-1;
    ///////////////////////// 
    Io_WriteSelfDefine2Bit(DEFINE_BIT_5,SET);
    /////////////////////////
    return loginsucceed;
    //TiredDrive_ShowDebugInfo(s_stTiredDriver.loginindex-1,"login");
}
/*************************************************************
** 函数名称: TiredDrive_Logout
** 功能描述: :驾驶员签退时调用
** 入口参数: 
** 出口参数: 无
** 返回参数: 1:签退成功,0:签退失败
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char TiredDrive_Logout(void)
{
    ///////////////////////
    if(0==s_ucTiredModeFirstRun)//在登录之前一定要先初始化模块的参数
    {
        TiredDrive_ParamInitialize();
    }
    ////////////////////////
    TiredDrive_SaveFramTimeToDriverInfo(s_stTiredDriver.currentindex);   
    ////////////////////////////////
    TiredDrive_SaveDriverStopInfoToFRAM(TIRED_DRIVER_INFO_NUM);
    ///////////////////////////
    s_stTiredDriver.driverloginflag = 0;
    s_stTiredAttrib.firstRunflag    = 0;    
    s_stTiredAttrib.needsaveflag    = 1;
    /////////////////////
    Io_WriteSelfDefine2Bit(DEFINE_BIT_5,RESET);
    /////////////////
    return 1;
    ///////////////////////////////////////////
    //TiredDrive_ShowDebugInfo(TIRED_DRIVER_INFO_NUM,"Logout");    
}
/*************************************************************
** 函数名称: TiredDrive_GetCurrentRunOverTime
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void TiredDrive_GetCurrentRunOverTime(unsigned char index)
{
    if(0xffffffff == s_stTiredAttrib.overTimeVal)
    {
        s_stTiredAttrib.curOverTimeVal =0xffffffff;
    }
    else
    {
        s_stTiredAttrib.curOverTimeVal = TiredDrive_GetDriverInfoOverTime(index);//最小值
    }
    ////////////////////
    //TiredDrive_ShowDebugInfo(index,"Current");
}
/*************************************************************
** 函数名称: TiredDrive_CheckDriverRunStatus
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned char TiredDrive_CheckAndSaveNightstarttime(unsigned char index,unsigned long currenttime)
{
    if(!s_stTiredAttrib.needChecknight)return 0;
    if(!s_stTiredAttrib.atnightFlag)return 0;    
    if(s_stTiredDriver.driverinfo[index].nightvalidFlag)return 0;
    s_stTiredDriver.driverinfo[index].nightvalidFlag = 1;
    s_stTiredDriver.driverinfo[index].nightstarttime = currenttime;
    return 1;
}
/*************************************************************
** 函数名称: TiredDrive_OverTimeAlarmVoice
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void TiredDrive_RestTimePlayVoice(void)
{
    if(0==TTS_GetPlayBusyFlag()||(0==s_stTiredAttrib.restPlayCnt))
    {
        if((0==s_stTiredAttrib.restPlayCnt)
         ||(s_stTiredAttrib.restPlayCnt&&0==Io_ReadAlarmBit(ALARM_BIT_TIRED_DRIVE_PRE_ALARM)))
        {       
            s_stTiredAttrib.restPlayCnt++;
            s_stTiredAttrib.restTimeCnt = 0;
            Public_PlayTTSVoiceStr("您已休息完指定时间,可以继续行驶"); 
            if(s_stTiredAttrib.restPlayCnt<3)
            {                           
                LZM_PublicSetOnceTimer(&s_stTiredTimer[TIRED_TIMER_PLAY],PUBLICSECS(5),TiredDrive_RestTimePlayVoice); 
                return;
            }
        }
    }
    else  
    if(s_stTiredAttrib.restTimeCnt<60)
    {
        s_stTiredAttrib.restTimeCnt++;
        LZM_PublicSetOnceTimer(&s_stTiredTimer[TIRED_TIMER_PLAY],PUBLICSECS(1),TiredDrive_RestTimePlayVoice);             
        return;
    }
    //////////////////////////
    s_stTiredAttrib.restPlayCnt = 0;
    s_stTiredAttrib.restTimeCnt = 0;
}
/*************************************************************
** 函数名称: TiredDrive_CheckRestTimePlayVoice
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void TiredDrive_CheckRestTimePlayVoice(void)
{
    if(Io_ReadAlarmBit(ALARM_BIT_TIRED_DRIVE_PRE_ALARM)||Io_ReadAlarmBit(ALARM_BIT_TIRED_DRIVE))
    {
        s_stTiredAttrib.restPlayCnt = 0;
        s_stTiredAttrib.restTimeCnt = 0;
        TiredDrive_RestTimePlayVoice();
    }
}
/*************************************************************
** 函数名称: TiredDrive_ConvertSpeedAndRunStatus
** 功能描述: 转换速度并判断行驶状态,用于判断开车点、停车点
** 入口参数: 
** 出口参数: 
** 返回参数: 相对速度
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned char TiredDrive_ConvertSpeedAndRunStatus(void)
{
    unsigned char curSpeedVal;
    curSpeedVal = SpeedMonitor_GetCurSpeed();
    if(0==s_stTiredAttrib.runSpeed)return curSpeedVal;
    /////////////////////////////////
    if(curSpeedVal>=s_stTiredAttrib.runSpeed)
    {
        curSpeedVal = curSpeedVal+1-s_stTiredAttrib.runSpeed;
    }
    else
    {
        curSpeedVal = 0;
    }
    /////////////判断行驶状态/////////////////////
    TiredDrive_CheckRunStatus(curSpeedVal);
    ///////////////////////////////////
    return curSpeedVal;
}
/*************************************************************
** 函数名称: TiredDrive_CheckDriverRunStatus
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void TiredDrive_CheckDriverRunStatus(unsigned long currentTime,unsigned char index)
{
    static unsigned char sucRunStatusCnt = 0;//写行驶时间间隔,10s写一次  
    unsigned char RunStatus;
    unsigned char curSpeedVal;
    curSpeedVal = TiredDrive_ConvertSpeedAndRunStatus();   
    //////////////////////////////////////
    RunStatus = TiredDrive_GetCurRunStatus();
    /////////////////////////////////////    
    s_stTiredDriver.currentindex = index;
    if(1 == s_stTiredDriver.driverinfo[index].validflag)
    {
        if(0 == s_stTiredAttrib.firstRunflag)////掉电重启之后第一次运行//////////////
        {
            sucRunStatusCnt = 0;
            s_stTiredDriver.driverinfo[index].overtimeflag = 0; 
            if(TiredDrive_CheckDriverRestStatus(currentTime,index))//超过休息时间
            {
                TiredDrive_ResetCurDriverData(index);
                s_stTiredAttrib.needsaveflag = 1;//需要保存
            }
            else 
            if(TiredDrive_CheckDriverIsTired(index))
            {
                s_stTiredDriver.driverinfo[index].preovertimeflag = 0;
                s_stTiredDriver.driverinfo[index].overtimeflag = 1;
            }
            ////////////////////////////////////
            TiredDrive_GetCurrentRunOverTime(index);
        }
        else
        if(1 ==RunStatus)//行驶中
        {
            ///////////判断是否晚上计时有效///////////////////////////////////
            if(TiredDrive_CheckAndSaveNightstarttime(index,currentTime))
            {
                TiredDrive_GetCurrentRunOverTime(index);
                s_stTiredAttrib.needsaveflag = 1;//需要保存
            }
            ///////////////////////////////////////////////
            if(0 == s_stTiredDriver.driverinfo[index].overtimeflag)//未超时驾驶
            {
                if(currentTime >= s_stTiredAttrib.curOverTimeVal)//
                {
                    s_stTiredDriver.driverinfo[index].preovertimeflag = 0;
                    s_stTiredDriver.driverinfo[index].overtimeflag = 1;
                    s_stTiredAttrib.needsaveflag = 1;//需要保存
                }
                else
                if((currentTime+s_stTiredAttrib.prePlayAlarmTime)>=s_stTiredAttrib.curOverTimeVal)//
                {
                    if(0==s_stTiredDriver.driverinfo[index].preovertimeflag)
                    {
                        s_stTiredDriver.driverinfo[index].preovertimeflag = 1;
                        s_stTiredAttrib.needsaveflag = 1;//需要保存
                    }
                }
                else
                {
                    s_stTiredDriver.driverinfo[index].preovertimeflag = 0; 
                }
            }
            else////超时驾驶
            {
                sucRunStatusCnt +=2;//更新计数器加快
            }
            ////////////////
            sucRunStatusCnt++;
            //更新FRAM中的停车时间及停车位置,
            if(0 == curSpeedVal && s_stTiredAttrib.prespeedVal)////由速度不为零到为零
            {
                sucRunStatusCnt = 0;
                TiredDrive_SaveCurStopInfo(index,currentTime);
            }
            else
            if(sucRunStatusCnt >= 10&&curSpeedVal)//时间间隔清0
            {
                sucRunStatusCnt = 0; 
                TiredDrive_SaveCurStopInfo(index,currentTime);
            }
            else
            if(curSpeedVal)//当前有速度的时候才保存
            {
                s_stTiredDriver.driverinfo[index].endtime = currentTime;//实时更新停车时间
            }
        }
        else//停驶中
        {
            sucRunStatusCnt = 8;
            //////////////////////////////
            if(TiredDrive_CheckDriverRestStatus(currentTime,index))//超过休息时间
            {
                TiredDrive_CheckRestTimePlayVoice();
                TiredDrive_ResetCurDriverData(index);
                s_stTiredAttrib.needsaveflag = 1;//需要保存
            }
        }
    }
    else
    if(1 ==RunStatus)//行驶中,首次进入计时有效
    {
        s_stTiredDriver.driverinfo[index].validflag = 1;
        if(0==s_stTiredDriver.driverinfo[index].starttime)//在行驶过程中,直接换卡:没有出现速度为0到不为0的情况
        {
            TiredDrive_SaveCurStartInfo(index,currentTime);//实际开车时间            
        }
				//TiredDrive_DisposeOneDriverInfoTimeIsAbnormal(index);
        /////////////////////////////////        
        s_stTiredDriver.driverinfo[index].overtimeflag = 0;
        s_stTiredDriver.driverinfo[index].nightvalidFlag =0;
        s_stTiredDriver.driverinfo[index].nightstarttime = 0; 
        s_stTiredDriver.driverinfo[index].preovertimeflag = 0;
				s_stTiredDriver.driverinfo[index].starttime = currentTime;
        s_stTiredDriver.driverinfo[index].endtime = currentTime;
        TiredDrive_CheckAndSaveNightstarttime(index,s_stTiredDriver.driverinfo[index].starttime);       
        ////////////////// 
        TiredDrive_GetCurrentRunOverTime(index);
        //////////////////////////        
        sucRunStatusCnt =0;
				TiredDriveSaveFlg = 1;
        s_stTiredAttrib.needsaveflag = 1;//需要保存
    }
    else
    {
        if(0==curSpeedVal)
        {
            TiredDrive_ResetCurDriverData(index);
        }
        else
        if(0==s_stTiredAttrib.prespeedVal&&curSpeedVal)//由速度为零到不为零
        {
            TiredDrive_SaveCurStartInfo(index,currentTime);//实际开车时间            
        }              
    }
    /////////////////////
    s_stTiredAttrib.prespeedVal = curSpeedVal;    
    /////////////
}
/*************************************************************
** 函数名称: TiredDrive_CheckCurrentTimeAtNight
** 功能描述: 判断当前时间是否为晚上时间段
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void TiredDrive_CheckCurrentTimeAtNight(void)
{
    s_stTiredAttrib.atnightFlag = 0;
    if(!s_stTiredAttrib.needChecknight)return;//不需要检查晚上超时
    s_stTiredAttrib.atnightFlag=Public_CheckCurTimeIsInTimeRange(s_stTiredAttrib.nightctrl,s_stTiredAttrib.nightStartVal,s_stTiredAttrib.nightEndVal);   
}
/*************************************************************
** 函数名称: TiredDrive_CheckCurrentDriverStatus
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void TiredDrive_DisposeOneDriverInfoTimeIsAbnormal(unsigned char index)
{
    unsigned long currentTime,temp;
    //////////////////
    if(0==s_stTiredDriver.driverinfo[index].validflag)return;
    /////////////////
    currentTime = RTC_GetCounter();
    /////////////////////////
    temp = s_stTiredDriver.driverinfo[index].starttime+currentTime;
    if(temp > s_stTiredAttrib.curTimeVal)
    {
        s_stTiredDriver.driverinfo[index].starttime = temp-s_stTiredAttrib.curTimeVal;
    }
    else
    {
        s_stTiredDriver.driverinfo[index].starttime = 1;
    }
    //////////////////////
    temp = s_stTiredDriver.driverinfo[index].endtime+currentTime;
    if(temp > s_stTiredAttrib.curTimeVal)
    {
        s_stTiredDriver.driverinfo[index].endtime = temp-s_stTiredAttrib.curTimeVal;
    }
    else
    {
        s_stTiredDriver.driverinfo[index].endtime = 1;
    }
    //////////////////////  
    if(s_stTiredAttrib.needChecknight)//
    {
        if(TiredDrive_CheckTimeAtNight(s_stTiredDriver.driverinfo[index].starttime))
        {
            s_stTiredDriver.driverinfo[index].nightvalidFlag = 1;
            s_stTiredDriver.driverinfo[index].nightstarttime = s_stTiredDriver.driverinfo[index].starttime;
        }
        else
        if(TiredDrive_CheckTimeAtNight(s_stTiredDriver.driverinfo[index].endtime))
        {
            s_stTiredDriver.driverinfo[index].nightvalidFlag = 1;
            temp = s_stTiredDriver.driverinfo[index].endtime%DAY_SECOND;
            s_stTiredDriver.driverinfo[index].nightstarttime = s_stTiredDriver.driverinfo[index].endtime + s_stTiredAttrib.nightStartSec-temp;
        }
        else
        {
            s_stTiredDriver.driverinfo[index].nightvalidFlag = 0;
        }
    }
}
/*************************************************************
** 函数名称: TiredDrive_CheckCurrentDriverStatus
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void TiredDrive_DisposeCurrentTimeIsAbnormal(void)
{
    unsigned char i;
    TiredDrive_CheckCurrentTimeAtNight();
    /////////////////
    for(i=0; i<=TIRED_DRIVER_INFO_NUM; i++)
    {
        TiredDrive_DisposeOneDriverInfoTimeIsAbnormal(i);
    } 
    //////////////////////////
    TiredDrive_GetCurrentRunOverTime(s_stTiredDriver.currentindex);
}
/*************************************************************
** 函数名称: TiredDrive_CheckCurrentDriverStatus
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned long TiredDrive_CheckCurrentTimeIsAbnormal(void)
{
    unsigned long currentTime;
    currentTime = RTC_GetCounter(); 
    //////////////////////    
    s_stTiredAttrib.timeAbnormalFlag = 0;
    if((s_stTiredAttrib.curTimeVal>(currentTime+RESEET_CURRENT_TIRED_DRIVER_TIME_VAL))
      ||(s_stTiredAttrib.curTimeVal+RESEET_CURRENT_TIRED_DRIVER_TIME_VAL)< currentTime)//误差超过60s
    {
        TiredDrive_DisposeCurrentTimeIsAbnormal();
        s_stTiredAttrib.curTimeVal = currentTime;        
        //TiredDrive_ResetCurDriverData(s_stTiredDriver.currentindex);
        ////////////////////////////
        s_stTiredAttrib.needsaveflag = 1;//需要保存
        s_stTiredAttrib.timeAbnormalFlag = 1;
    }
    ///////////////////////////////
    s_stTiredAttrib.curTimeVal = currentTime;
    return s_stTiredAttrib.curTimeVal;
}
/*************************************************************
** 函数名称: TiredDrive_CheckCurrentDriverStatus
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void TiredDrive_CheckAndSaveDriver(unsigned char index,unsigned long currentTime)
{
    if(1 == s_stTiredDriver.driverinfo[index].validflag)
    {
        if(TiredDrive_CheckDriverRestStatus(currentTime,index))
        {
            TiredDrive_ClearDriverInfo(index);
            s_stTiredAttrib.needsaveflag =1;//需要保存
        }
    }
}
/*************************************************************
** 函数名称: TiredDrive_CheckCurrentDriverStatus
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void TiredDrive_CheckAndSetCurrentAlarm(unsigned char index)
{
    unsigned long timeVal;
    if(s_stTiredDriver.driverinfo[index].overtimeflag)
    {
        s_stTiredDriver.driverinfo[index].preovertimeflag = 0;
        Io_WriteAlarmBit(ALARM_BIT_TIRED_DRIVE_PRE_ALARM,RESET);
        Io_WriteAlarmBit(ALARM_BIT_TIRED_DRIVE,SET);
    }
    else
    {
        Io_WriteAlarmBit(ALARM_BIT_TIRED_DRIVE,RESET);
        /////////////预警////////////////////
        if(s_stTiredDriver.driverinfo[index].preovertimeflag)
        {
            Io_WriteAlarmBit(ALARM_BIT_TIRED_DRIVE_PRE_ALARM,SET);
        }
        else
        {
            Io_WriteAlarmBit(ALARM_BIT_TIRED_DRIVE_PRE_ALARM,RESET);
        }
    }
    /////////////////////////////////
    if(Io_ReadAlarmMaskBit(ALARM_BIT_DAY_OVER_DRIVE))//当天累计驾驶超时
    {
        Io_WriteAlarmBit(ALARM_BIT_DAY_OVER_DRIVE,RESET);
        return;
    }
    ////////////////////////////
    if(0xffffffff==s_stTiredAttrib.oneDayoverTimeVal)return;///不检查
    ////////////////////
    timeVal = TiredDrive_GetCurtotalDriveTime(index);    
    /////////////////////
    if(s_stTiredAttrib.oneDayoverTimeVal<(s_stTiredDriver.driverinfo[index].totalDriveTime+timeVal))
    {
        Io_WriteAlarmBit(ALARM_BIT_DAY_OVER_DRIVE,SET);
    }
    else
    {
        Io_WriteAlarmBit(ALARM_BIT_DAY_OVER_DRIVE,RESET);
    }    
}
/*************************************************************
** 函数名称: TiredDrive_CheckCurrentDriverStatus
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void TiredDrive_CheckCurrentDriverStatus(void)
{
    unsigned char i;
    unsigned long currentTime;
    currentTime = s_stTiredAttrib.curTimeVal;
    ////////////////////
    TiredDrive_CheckCurrentTimeAtNight();  
    /////////////////////////////////////////////////
    if(!TiredDrive_GetLoginFlag())//无人登录时
    {     
        TiredDrive_CheckDriverRunStatus(currentTime,TIRED_DRIVER_INFO_NUM);
    }
    else
    if(s_stTiredDriver.loginindex)//当前驾驶员登录的情况
    {
        TiredDrive_CheckDriverRunStatus(currentTime,s_stTiredDriver.loginindex-1);
    }
    ////////////////////////
    for(i=0; i< TIRED_DRIVER_INFO_NUM; i++)//判断其它驾驶员是否过了休息时间
    {
        if(i != s_stTiredDriver.currentindex)//不是当前的驾驶员
        {
            TiredDrive_CheckAndSaveDriver(i,currentTime);
        }        
    }
    ///////写当前超时驾驶状态标志/////////
    TiredDrive_CheckAndSetCurrentAlarm(s_stTiredDriver.currentindex);
    /////////////////////
    s_stTiredAttrib.firstRunflag = 1;
    //////////////////////////
}
/*************************************************************
** 函数名称: TiredDrive_OverTimePreAlarmVoice
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void TiredDrive_OverTimePreAlarmVoice(void)
{
    if(!Public_PlayE2ParamAlarmVoice(E2_TIRED_DRIVE_PREALARM_VOICE_ID))
    {
        Public_PlayTTSVoiceAlarmStr("请勿超时驾驶");
    }
}
/*************************************************************
** 函数名称: TiredDrive_OverTimeAlarmVoice
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void TiredDrive_OverTimeAlarmVoice(void)
{
    if(!Public_PlayE2ParamAlarmVoice(E2_TIRED_DRIVE_ALARM_VOICE_ID))
    {
        Public_PlayTTSVoiceAlarmStr("您已超时驾驶,请停车休息");
    }
}
/*************************************************************
** 函数名称: TiredDrive_PreOverTimeAlarm
** 功能描述: 超时驾驶预警提示
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void TiredDrive_PreOverTimeAlarm(void)
{
    unsigned char flag;
    /////////预警提示///////////////////////////////////
    if(Io_ReadAlarmBit(ALARM_BIT_TIRED_DRIVE_PRE_ALARM)&&TiredDrive_GetCurRunStatus())      
    {
        flag = 1;
    }
    else
    {
        flag = 0;
    }
    ///////////////////////////
    if(SpeedMonitor_CheckPlayFlag(flag,&s_stTiredAttrib.stPrePlay))
    {
        TiredDrive_OverTimePreAlarmVoice();
    }
}
/*************************************************************
** 函数名称: TiredDrive_OverTimeAlarm
** 功能描述: 超时驾驶提示
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void TiredDrive_OverTimeAlarm(void)
{
    unsigned long timeVal;
    //超时驾驶播报
    if(Io_ReadAlarmBit(ALARM_BIT_TIRED_DRIVE)&&TiredDrive_GetCurRunStatus())
    {
        s_stTiredAttrib.needReportflag = 0;
        timeVal = TiredDrive_GetCurOverDriveTime();
        if(0==timeVal)return;
        ////////////////////////////
        if(timeVal>=s_stTiredAttrib.AllowRunTime&&TiredDrive_GetTiredStatusForReport())
        {
            if(s_stTiredAttrib.curTimeVal>=s_stTiredDriver.driverinfo[s_stTiredDriver.currentindex].preReportTime+s_stTiredAttrib.ReportInterval)
            {
                s_stTiredDriver.driverinfo[s_stTiredDriver.currentindex].preReportTime = s_stTiredAttrib.curTimeVal;
                s_stTiredAttrib.needsaveflag    = 1;  
                s_stTiredAttrib.needReportflag  = 1;
            }
        }
        ///////////////超过提示时间///////////////////////
        if(timeVal>s_stTiredAttrib.PlayAlarmTime&&s_stTiredAttrib.PlayAlarmTime)return;
        ///////////未超过提示时间////////////////////////////
        if(SpeedMonitor_CheckPlayFlag(1,&s_stTiredAttrib.stPlay))
        {
            TiredDrive_OverTimeAlarmVoice();
        }
    }
    else
    {
        s_stTiredAttrib.needReportflag = 0;
        SpeedMonitor_CheckPlayFlag(0,&s_stTiredAttrib.stPlay);
    }
}
/*************************************************************
** 函数名称: TiredDrive_AlarmTTSAndBeep
** 功能描述: 超时驾驶模块提示
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void TiredDrive_AlarmTTSAndBeep(void)
{
    //////////预警提示///////////////
    TiredDrive_PreOverTimeAlarm();    
    ////////超时驾驶提示/////////////
    TiredDrive_OverTimeAlarm();
}
/*************************************************************
** 函数名称: TiredDrive_GetCurContDriveTimeSec
** 功能描述: 取得当前连续驾驶时间(单位秒)
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned long TiredDrive_GetCurContDriveTimeSec(void)
{
    unsigned long timeVal,temp;  
    if(!s_stTiredDriver.driverinfo[s_stTiredDriver.currentindex].validflag)return 0;
    timeVal = s_stTiredDriver.driverinfo[s_stTiredDriver.currentindex].endtime;
    temp = s_stTiredDriver.driverinfo[s_stTiredDriver.currentindex].starttime;
    if(timeVal>temp&&temp)
        return (timeVal-temp); 
    return 0;   
}
/*************************************************************
** 函数名称: TiredDrive_GetCurContDriveTime
** 功能描述: 取得当前连续驾驶时间(单位分钟)
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned long TiredDrive_GetCurContDriveTime(void)
{
    unsigned long timeVal;
    timeVal=TiredDrive_GetCurContDriveTimeSec();
    if(timeVal)
    {
        return (timeVal/60); 
    }
    return 0;   
}
/*************************************************************
** 函数名称: TiredDrive_GetCurOverDriveTime
** 功能描述: 取得当前超过疲劳驾驶设定值的多少秒(单位:秒)
** 入口参数: 
** 出口参数: 
** 返回参数: 0:表示未超时;非零:表示超过设定值多少秒
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned long TiredDrive_GetCurOverDriveTime(void)
{
    unsigned long timeVal;  
    if(!s_stTiredDriver.driverinfo[s_stTiredDriver.currentindex].validflag)return 0;
    if(!s_stTiredDriver.driverinfo[s_stTiredDriver.currentindex].overtimeflag)return 0;
    timeVal = s_stTiredAttrib.curTimeVal;
    if(timeVal >= s_stTiredAttrib.curOverTimeVal && s_stTiredAttrib.curOverTimeVal)
    {
        timeVal -=s_stTiredAttrib.curOverTimeVal;
        if(0==timeVal)
        {
            timeVal = 1;
            s_stTiredAttrib.needReportflag = 1;
        }
        return timeVal;
    }
    else
    {
        s_stTiredDriver.driverinfo[s_stTiredDriver.currentindex].overtimeflag = 0;
        return 0;
    }
}
/*************************************************************
** 函数名称: TiredDrive_GetLoginFlag
** 功能描述: 取得驾驶员登录标志
** 入口参数: 
** 出口参数: 
** 返回参数: 0:未登录,1已登录
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned char TiredDrive_GetLoginFlag(void)
{
    return (1==s_stTiredDriver.driverloginflag)?1:0;
}
/*************************************************************
** 函数名称: TiredDrive_GetTimeAbnormalflag
** 功能描述: 取得时间异常标志
** 入口参数: 
** 出口参数: 
** 返回参数: 0:未异常,1异常
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned char TiredDrive_GetTimeAbnormalflag(void)
{
    return (1==s_stTiredAttrib.timeAbnormalFlag)?1:0;
}
/*************************************************************
** 函数名称: TiredDrive_ReadTiredRecord
** 功能描述: 查找指定时间段的超时驾驶记录
** 入口参数: 起始时间StartTime,结束时间EndTime
** 出口参数: 
** 返回参数: pBuffer字节长度
** 全局变量: 无
** 调用模块: Register_Read
*************************************************************/
unsigned short TiredDrive_ReadTiredRecord(unsigned char *pBuffer, TIME_T StartTime, TIME_T EndTime)
{
    unsigned char *p;
    unsigned short len;
    unsigned char block;
    len=0;
    p=pBuffer;
    block = 0;
    len=TiredDrive_ReadTiredDriveFromFramEx(p,StartTime,EndTime);
    if(len)
    {
        p += len;
        block =len/ONE_TIRED_DRIVER_DATA_LEN;
    }
    block =12-block;
    /////////////////
    len += Register_Read(REGISTER_TYPE_OVER_TIME,p,StartTime,EndTime,block);
    return len;    
}
/*************************************************************
** 函数名称: TiredDrive_Read2daysTiredRecord
** 功能描述: 指定时间点开始两个日历天的超时驾驶记录
** 入口参数: 起始时间StartTime
** 出口参数: 
** 返回参数: pBuffer字节长度
** 全局变量: 无
** 调用模块: Register_Read
*************************************************************/
unsigned short TiredDrive_Read2daysTiredRecord(unsigned char *pBuffer, TIME_T StartTime)
{
    unsigned long TimeCount;
    unsigned short length;
    TIME_T EndTime;

    length = 0;
    //开始时间中时,分,秒清0
    StartTime.hour = 0;
    StartTime.min = 0;
    StartTime.sec = 0;

    //计数结束时间
    TimeCount = ConverseGmtime(&StartTime);
    TimeCount += 48*3600;
    Gmtime(&EndTime, TimeCount);
    //查找超时驾驶记录
    length = TiredDrive_ReadTiredRecord(pBuffer, StartTime,EndTime);
    if(length > 12*ONE_TIRED_DRIVER_DATA_LEN)
    {
        return 0;//查找出错
    }
    else
    {
        return length;
    }
}
/*************************************************************
** 函数名称: TiredDrive_GetCurdriverLicence
** 功能描述: 获取当前驾驶员的驾驶证号码(18个字节)
** 入口参数: 
** 出口参数: 保存数据的首地址
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned char TiredDrive_GetCurdriverLicence(unsigned char *buffer)
{
    memcpy(buffer,s_stTiredDriver.driverinfo[s_stTiredDriver.currentindex].driverLicence,18);
    ///////////////////////
    buffer[18]=0;
    return 18;
}
/*************************************************************
** 函数名称: TiredDrive_GetDriverLastRunTime
** 功能描述: 取得保存在电铁里的最后行驶时间(用于重启时读取)
** 入口参数: 
** 出口参数: 无
** 返回参数: 最后行驶时间
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned long TiredDrive_GetDriverLastRunTime(void)
{
    unsigned len;
    STSTOPINFO stTmpStopinfo;
    len=Public_ReadDataFromFRAM(FRAM_TIRED_STOP_INFO_ADDR,(unsigned char *)&stTmpStopinfo,STSTOPINFO_SIZE);
    if(STSTOPINFO_SIZE==len)//新的
    {
        return stTmpStopinfo.endtime;
    }
    return 0;
}
/*************************************************************
** 函数名称: TiredDrive_ClearAllDriverRunInfo
** 功能描述: 清空当前暂存在铁电中所有驾驶员的相关驾驶信息(已保存到flash的数据除外)
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void TiredDrive_ClearCurAllDriverRunInfo(void)
{
    unsigned char i;    
    for(i=0; i<=TIRED_DRIVER_INFO_NUM; i++)
    {
        TiredDrive_ResetCurDriverData(i);
    }
    /////////////////
    TiredDrive_ClearAlltotalDriveTime();
    //////需要保存/////////////////////////
    s_stTiredAttrib.needsaveflag = 1;
}
/*************************************************************
** 函数名称: TiredDrive_CheckAndSaveTiredDriverInfo
** 功能描述: 检查及保存驾驶员结构体
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void TiredDrive_CheckAndSaveTiredDriverInfo(void)
{
    if(s_stTiredAttrib.needsaveflag)//改变的驾驶员信息
    {
        s_stTiredAttrib.needsaveflag = 0;
        TiredDrive_SaveTiredDriverInfoToFlash();
    }//////////////
}
/*************************************************************
** 函数名称: TiredDrive_GetTiredStatusForReport
** 功能描述: 获取根据超时驾驶状态来上报位置信息
** 入口参数: 
** 出口参数: 
** 返回参数: 0:不根据超时驾驶状态来上报, 1:根据超时驾驶状态来上报
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned char TiredDrive_GetTiredStatusForReport(void)
{
    unsigned long val;    
    if(0 == s_stTiredAttrib.ReportInterval)return 0;
    if(0 == Io_ReadAlarmBit(ALARM_BIT_TIRED_DRIVE))return 0;
    if(Io_ReadAlarmBit(ALARM_BIT_EMERGENCY))return 0;
    val=SpeedMonitor_GetReportFreqEvent();
    if(!PUBLIC_CHECKBIT(val, 1))return 0;
    return 1;
}
/*************************************************************
** 函数名称: TiredDrive_GetNeedReportFlag
** 功能描述: 获取是否需要上报位置信息标志
** 入口参数: 
** 出口参数: 
** 返回参数: 0:不需要上报,1:需要上报
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned char TiredDrive_GetNeedReportFlag(void)
{
    if(0==s_stTiredAttrib.needReportflag)return 0;
    s_stTiredAttrib.needReportflag = 0;
    return 1;
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
void TiredDrive_ScanTask(void)
{
    TiredDrive_CheckCurrentTimeIsAbnormal();
    //////////////////
    if(Io_ReadAlarmMaskBit(ALARM_BIT_TIRED_DRIVE))//读超时驾驶报警屏蔽位状态
    {
        if(s_stTiredDriver.driverinfo[s_stTiredDriver.currentindex].validflag)
        {
            TiredDrive_ClearCurAllDriverRunInfo();
            Io_WriteAlarmBit(ALARM_BIT_TIRED_DRIVE,RESET);
            Io_WriteAlarmBit(ALARM_BIT_DAY_OVER_DRIVE,RESET);
            Io_WriteAlarmBit(ALARM_BIT_TIRED_DRIVE_PRE_ALARM,RESET);
        }
        return;
    }
    //计算连续驾驶时间,判断是否为超时驾驶
    TiredDrive_CheckCurrentDriverStatus();
    /////////超时驾驶语音播报及蜂鸣器/////////////
    TiredDrive_AlarmTTSAndBeep();  
    /////////////////////////
    TiredDrive_CheckAndCleartotalDriveTime();
    ///////////////////////////////////
    TiredDrive_CheckAndSaveTiredDriverInfo();  
}
/*************************************************************
** 函数名称: TiredDrive_StartScanTask
** 功能描述: 超时驾驶的定时任务
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void TiredDrive_StartScanTask(void)
{
  TiredDrive_ScanTask();
  LZM_PublicSetCycTimer(&s_stTiredTimer[TIRED_TIMER_TASK],PUBLICSECS(1),TiredDrive_ScanTask);    
}
/*************************************************************
** 函数名称: TiredDrive_ParamInitialize
** 功能描述: 超时驾驶参数初始化
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void TiredDrive_ParamInitialize(void) 
{
    if(0==s_ucTiredModeFirstRun)
    {
        s_ucTiredModeFirstRun = 1;
        LZM_PublicKillTimerAll(s_stTiredTimer,TIRED_TIMERS_MAX);
        memset(&s_stTiredAttrib,0,sizeof(s_stTiredAttrib));         
        SetTimerTask(TIME_TIRED_DRIVE, 5*SYSTICK_0p1SECOND);
        LZM_PublicSetOnceTimer(&s_stTiredTimer[TIRED_TIMER_TASK],PUBLICSECS(0.4),TiredDrive_StartScanTask); 
    }    
    ///////////////////////////////
    TiredDrive_UpdatePram();
    /////////////////////
    TiredDrive_ReadTiredDriverInfoFromFlash();    
    s_stTiredAttrib.curTimeVal = RTC_GetCounter();    
}
/*************************************************************
** 函数名称: TiredDrive_GetCurrentTime
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned long TiredDrive_GetCurrentTime(void)
{
    return s_stTiredAttrib.curTimeVal;
}
/*************************************************************
** 函数名称: TiredDrive_TimeTask
** 功能描述: 超时驾驶模块任务,1s调度一次
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
FunctionalState TiredDrive_TimeTask(void)
{
		
    LZM_PublicTimerHandler(s_stTiredTimer,TIRED_TIMERS_MAX);
    return ENABLE;     
}

/*******************************************************************************
 *                             end of module                                   *
 *******************************************************************************/


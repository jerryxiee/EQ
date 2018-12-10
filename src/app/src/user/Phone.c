/*******************************************************************************
 * File Name:			Phone.c 
 * Function Describe:	电话处理模块
 * Relate Module:		测试相关协议。
 * Writer:				Joneming
 * Date:				2014-08-29
 * ReWriter:			
 * Date:				
 *******************************************************************************/
#include "include.h"
/*******************************************************************************/
////////////////
#ifdef USE_PHONE
//////////////
enum 
{
    PH_TIMER_TASK,              //0 timer
    PH_TIMER_CHECK,             //1 timer
    PH_TIMER_TEST,              //2 timer
    PH_TIMER_CALL,              //3 timer
    PH_TIMERS_MAX
}E_PHONETIME;

static LZM_TIMER s_sPHTimer[PH_TIMERS_MAX];
/////////////////////////////////////
typedef enum 
{
    PHONE_STATUS_HANG_UP,    
    PHONE_STATUS_CALL_TALK,
    PHONE_STATUS_NEW_PHONE,
    PHONE_STATUS_MAX
}E_PHONE_STATUS;

typedef struct
{
    unsigned long curMonthTalkTime;//当月通话总时间
    unsigned long monthStartTime;//当月开始时间
    unsigned char saveFlag;//保存标志
    unsigned char bak[3];    //
    unsigned long nextMonthStartTime;//下个月的开始时间
    unsigned long eachMaxTalkTime; //每次最长通话时间
    unsigned long monthMaxTalkTime;//每月最长通话时间
    unsigned long curEachtime;//当前通话时间 
    unsigned char status;     //电话状态
    unsigned char saveCnt;    //
    unsigned char recallTime;  //重拨次数
    unsigned char tryCallCnt;  //
    unsigned char monitorFlag;//
    unsigned char specialFlag;      //
    unsigned char firstRunFlag;
}ST_PHONE_ATTRIB;

static ST_PHONE_ATTRIB s_stPhoneAttrib;
#define PHONE_SAVE_FLAG 0xAA
/****************************************************/
/*                                                  */
/* Local Definitions and macros                     */
/*                                                  */
/****************************************************/
/****************************************************/
/*                                                  */
/* Local Definitions and macros                     */
/*                                                  */
/****************************************************/
/*************************************************************
** 函数名称: Phone_SetPhoneSpecialFlag
** 功能描述: 设置电话号码特权标志
** 入口参数: 
** 出口参数: 无
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Phone_SetPhoneSpecialFlag(unsigned char value)
{
    s_stPhoneAttrib.specialFlag = value;
}
/*************************************************************
** 函数名称: Phone_GetPhoneSpecialFlag
** 功能描述: 获取电话号码特权标志
** 入口参数: 
** 出口参数: 无
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Phone_GetPhoneSpecialFlag(void)
{
    return s_stPhoneAttrib.specialFlag?1:0;
}
/*************************************************************OK
** 函数名称: Phone_GetCurMonthTotalSec
** 功能描述: 
** 入口参数:  
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned long Phone_GetCurMonthTotalSec(void)
{
    unsigned char MonTab[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};    
    TIME_T stTime;
    unsigned long result;
    RTC_GetCurTime(&stTime);
    result= MonTab[stTime.month];
    if(LeapYear(stTime.year)&&(stTime.month == 2))
        result++;
    result *= DAY_SECOND;
    return result;    
}
/*************************************************************OK
** 函数名称: Phone_GetCurMonthStartTime
** 功能描述: 
** 入口参数:  
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned long Phone_GetCurMonthStartTime(void)
{  
    TIME_T stTime;
    RTC_GetCurTime(&stTime);
    stTime.day  = 1;
    stTime.hour = 0;
    stTime.min  = 0;
    stTime.sec  = 0;
    return ConverseGmtime(&stTime);
}
/*************************************************************
** 函数名称: Phone_CheckMonthStartTime
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void Phone_GetCurNextMonthStartTime(void)
{
    s_stPhoneAttrib.nextMonthStartTime=Phone_GetCurMonthTotalSec()+s_stPhoneAttrib.monthStartTime;
}
/*************************************************************OK
** 函数名称: Phone_SaveMonthParam
** 功能描述: 更新参数
** 入口参数:  
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void Phone_SaveMonthParam(void)
{
    if(0==s_stPhoneAttrib.saveCnt)return;
    s_stPhoneAttrib.saveCnt = 0;
    s_stPhoneAttrib.saveFlag = PHONE_SAVE_FLAG;
    Public_WriteDataToFRAM(FRAM_PHONE_MONTH_PARAM_ADDR, (unsigned char *)&s_stPhoneAttrib,12);
}
/*************************************************************
** 函数名称: Phone_GetCurMonthStartTimeAndSave
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void Phone_GetCurMonthStartTimeAndSave(void)
{
    s_stPhoneAttrib.curMonthTalkTime = 0;
    s_stPhoneAttrib.monthStartTime = Phone_GetCurMonthStartTime();
    Phone_GetCurNextMonthStartTime();
    s_stPhoneAttrib.saveCnt = 1;
    Phone_SaveMonthParam();
}
/*************************************************************OK
** 函数名称: Phone_ReadMonthParam
** 功能描述: 更新参数
** 入口参数:  
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void Phone_ReadMonthParam(void)
{
    Public_ReadDataFromFRAM(FRAM_PHONE_MONTH_PARAM_ADDR, (unsigned char *)&s_stPhoneAttrib, 12);
    if(PHONE_SAVE_FLAG!=s_stPhoneAttrib.saveFlag)
    {
        Phone_GetCurMonthStartTimeAndSave();
    }
}

/*************************************************************
** 函数名称: Phone_GetEachMaxTalkTime
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned long Phone_GetEachMaxTalkTime(void)
{
    return s_stPhoneAttrib.eachMaxTalkTime;
}
/*************************************************************
** 函数名称: Phone_GetMonthMaxTalkTime
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned long Phone_GetMonthMaxTalkTime(void)
{
    return s_stPhoneAttrib.monthMaxTalkTime;
}
/*************************************************************
** 函数名称: Phone_GetCurMonthTalkTime
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned long Phone_GetCurMonthTalkTime(void)
{
    return s_stPhoneAttrib.curMonthTalkTime;
}
/*******************************************************************************
** 函数名称: Phone_DisposeHangUp
** 函数功能:  
** 入口参数:  
** 出口参数: 无
** 返回参数:  
** 全局变量: 无
** 调用模块: 无
*******************************************************************************/
void Phone_DisposeHangUp(void)
{
    communication_CallPhone("13800000000",PHONE_END);
}
/*************************************************************
** 函数名称: Phone_CheckMonthStartTime
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void Phone_CheckMonthStartTime(void)
{
    unsigned long curtime;
    curtime = RTC_GetCounter();
    if(curtime<s_stPhoneAttrib.monthStartTime||curtime>=s_stPhoneAttrib.nextMonthStartTime)
    {
        Phone_GetCurMonthStartTimeAndSave();
    }
}
/*************************************************************
** 函数名称: Phone_GetOverMonthTalkTimeFlag
** 功能描述: 获得超过当月通话时间标志
** 入口参数: 
** 出口参数: 
** 返回参数: 1:超过,0:未超过
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned char Phone_GetOverMonthTalkTimeFlag(void)
{
    return (s_stPhoneAttrib.curMonthTalkTime>=s_stPhoneAttrib.monthMaxTalkTime)?1:0;
}
/*************************************************************
** 函数名称: Phone_CheckMonthStartTime
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void Phone_CheckTalkTime(void)
{
    if(PHONE_STATUS_CALL_TALK!=s_stPhoneAttrib.status)
    {
        LZM_PublicKillTimer(&s_sPHTimer[PH_TIMER_CHECK]);
        Phone_SaveMonthParam();
        return;
    }
    s_stPhoneAttrib.saveCnt++;
    s_stPhoneAttrib.curEachtime++;
    s_stPhoneAttrib.curMonthTalkTime++;
    if(s_stPhoneAttrib.saveCnt>3)//3s更新一次FRAM
    {
        Phone_SaveMonthParam();
    }
    ////////////////////
    if(Phone_GetOverMonthTalkTimeFlag()||(s_stPhoneAttrib.curEachtime>=s_stPhoneAttrib.eachMaxTalkTime))
    {
        LZM_PublicSetOnceTimer(&s_sPHTimer[PH_TIMER_CHECK],LZM_AT_ONCE,Phone_DisposeHangUp);
        return;
    }
}
/*************************************************************
** 函数名称: Phone_StartCheckTalkTime
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void Phone_StartCheckTalkTime(void)
{
    if(Phone_GetOverMonthTalkTimeFlag())
    {
        LZM_PublicSetOnceTimer(&s_sPHTimer[PH_TIMER_CHECK],LZM_AT_ONCE,Phone_DisposeHangUp);
        return;
    }
    s_stPhoneAttrib.saveCnt = 0;
    s_stPhoneAttrib.curEachtime = 0;
    LZM_PublicSetCycTimer(&s_sPHTimer[PH_TIMER_CHECK],PUBLICSECS(1),Phone_CheckTalkTime);
}
/*******************************************************************************
* Function Name  : Phone_SetDialEvTask
* Description    : 电话回拨
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Phone_CallBackPhone(void)
{
    unsigned char ucTmp[30] = {0};	//号码存储
    unsigned char len = 0;
    unsigned char flag;
    len = EepromPram_ReadPram(E2_PHONE_CALLBACK_ID, ucTmp);
    if (len != 0) 
    {
        ucTmp[len] = '\0';
        EepromPram_ReadPram(E2_CALLBACK_FLAG_ID, &flag);
        if(0 == flag) 
        {
            if(communication_CallPhone(ucTmp, PHONE_NORMAL))
            {
                Phone_SetPhoneSpecialFlag(1);
                return;
            }
        }
        else
        {
            if(MODON_PHONE == Modem_Api_OnSta())//电话业务中
            {
                if(0==s_stPhoneAttrib.monitorFlag)
                {
                    LZM_PublicSetOnceTimer(&s_sPHTimer[PH_TIMER_CHECK],LZM_AT_ONCE,Phone_DisposeHangUp);
                    LZM_PublicSetOnceTimer(&s_sPHTimer[PH_TIMER_CALL],PUBLICSECS(2),Phone_CallBackPhone);
                }
                return;
            }
            else
            if(communication_CallPhone(ucTmp, PHONE_MONITOR))
            {
                Phone_SetPhoneSpecialFlag(1);
                s_stPhoneAttrib.tryCallCnt = 0;
                s_stPhoneAttrib.monitorFlag = 1;                
                if(s_stPhoneAttrib.recallTime)s_stPhoneAttrib.recallTime--;
                return;
            }
        }
        /////////////////
        if(s_stPhoneAttrib.tryCallCnt<10)
        {
            s_stPhoneAttrib.tryCallCnt++;
            LZM_PublicSetOnceTimer(&s_sPHTimer[PH_TIMER_CALL],PUBLICSECS(1),Phone_CallBackPhone);
        }
        else
        {
            s_stPhoneAttrib.tryCallCnt = 0;
            if(s_stPhoneAttrib.recallTime)
            {
                s_stPhoneAttrib.recallTime--;
                LZM_PublicSetOnceTimer(&s_sPHTimer[PH_TIMER_CALL],PUBLICSECS(30),Phone_CallBackPhone);
            }
        }
    }
}
/*******************************************************************************
* Function Name  : Phone_SetDialEvTask
* Description    : 电话回拨
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Phone_SetDialEvTask(void)
{
    unsigned char flag;
    EepromPram_ReadPram(E2_CALLBACK_FLAG_ID, &flag);
    if(0 == flag) 
    {
        if(PHONE_STATUS_CALL_TALK==s_stPhoneAttrib.status)return;//通话中,直接返回
        s_stPhoneAttrib.recallTime = 0;
    } 
    else 
    {
        if(MODON_PHONE == Modem_Api_OnSta())//通话中
        {
            if(s_stPhoneAttrib.monitorFlag)return;//监听中,直接返回
            LZM_PublicSetOnceTimer(&s_sPHTimer[PH_TIMER_CHECK],LZM_AT_ONCE,Phone_DisposeHangUp);
        }
        s_stPhoneAttrib.recallTime = 3;
    }
    s_stPhoneAttrib.tryCallCnt = 0;
    LZM_PublicSetOnceTimer(&s_sPHTimer[PH_TIMER_CALL],PUBLICSECS(3),Phone_CallBackPhone);
}
/*************************************************************
** 函数名称: Phone_DisposePhoneHangup
** 功能描述: 处理电话状态命令
** 入口参数: 
** 出口参数: 无
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Phone_DisposeHangUpStatus(void)
{
    s_stPhoneAttrib.monitorFlag = 0;
    if(PHONE_STATUS_HANG_UP != s_stPhoneAttrib.status)
    {
        LZM_PublicKillTimer(&s_sPHTimer[PH_TIMER_CHECK]);
        Phone_SaveMonthParam();
    }
    ///////////////////////
    if(s_stPhoneAttrib.recallTime)
    {
        LZM_PublicSetOnceTimer(&s_sPHTimer[PH_TIMER_CALL],PUBLICSECS(30),Phone_CallBackPhone);
    }
}
/*************************************************************
** 函数名称: Phone_DisposePhoneHangup
** 功能描述: 处理电话状态命令
** 入口参数: 
** 出口参数: 无
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Phone_DisposeCallTalkStatus(void)
{
    if(PHONE_STATUS_CALL_TALK != s_stPhoneAttrib.status)
    {
        if(s_stPhoneAttrib.monitorFlag)s_stPhoneAttrib.recallTime = 0;
        if(0==Phone_GetPhoneSpecialFlag())
        Phone_StartCheckTalkTime();
    }
}
/*************************************************************
** 函数名称: Phone_DisposePhoneStatus
** 功能描述: 处理电话状态命令
** 入口参数: phone:电话号码,status:电话状态：0：为挂断;1:接通;2:新来电;
** 出口参数: 无
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Phone_DisposePhoneStatus(unsigned char *phone,unsigned char status)
{
    switch(status)
    {
        case PHONE_STATUS_HANG_UP://挂机
            Phone_DisposeHangUpStatus();            
            break;
        case PHONE_STATUS_CALL_TALK://通话
            Phone_DisposeCallTalkStatus();
            break;
        case PHONE_STATUS_NEW_PHONE://新来电
            s_stPhoneAttrib.monitorFlag = 0;
            break;
    }
    s_stPhoneAttrib.status = status;    
}
/*************************************************************OK
** 函数名称: Phone_UpdateMaxPhoneTimeEach
** 功能描述: 更新参数
** 入口参数:  
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void Phone_UpdateMaxPhoneTimeEach(void)
{
    unsigned char buffer[10];
    if(EepromPram_ReadPram(E2_MAX_PHONE_TIME_EACH_ID,buffer))
    {
        s_stPhoneAttrib.eachMaxTalkTime = Public_ConvertBufferToLong(buffer);
    }
    else
    {
        s_stPhoneAttrib.eachMaxTalkTime = 0;
    }
}
/*************************************************************OK
** 函数名称: Phone_UpdateMaxPhoneTimeMonth
** 功能描述: 更新参数
** 入口参数:  
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void Phone_UpdateMaxPhoneTimeMonth(void)
{
    unsigned char buffer[10];
    if(EepromPram_ReadPram(E2_MAX_PHONE_TIME_MONTH_ID,buffer))
    {
        s_stPhoneAttrib.monthMaxTalkTime = Public_ConvertBufferToLong(buffer);
    }
    else
    {
        s_stPhoneAttrib.monthMaxTalkTime = 0;
    }
    ////////////////
    if(s_stPhoneAttrib.firstRunFlag)
    {
        Phone_GetCurMonthStartTimeAndSave();
    }
    s_stPhoneAttrib.firstRunFlag = 1;
}
/*************************************************************OK
** 函数名称: Phone_UpdatePram
** 功能描述: 更新参数
** 入口参数:  
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void Phone_UpdateParam(void)
{
    Phone_UpdateMaxPhoneTimeEach();
    //////////////
    Phone_UpdateMaxPhoneTimeMonth();
}
/*************************************************************OK
** 函数名称: Phone_ParamInitialize
** 功能描述: 初始化
** 入口参数:  
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void Phone_ParamInitialize(void)
{ 
    LZM_PublicKillTimerAll(s_sPHTimer,PH_TIMERS_MAX);
    memset(&s_stPhoneAttrib,0,sizeof(ST_PHONE_ATTRIB));
    Phone_ReadMonthParam();    
    Phone_UpdateParam(); 
    Phone_GetCurNextMonthStartTime();
    //SetTimerTask(TIME_PHONE, LZM_TIME_BASE);
    LZM_PublicSetCycTimer(&s_sPHTimer[PH_TIMER_TASK],PUBLICSECS(1),Phone_CheckMonthStartTime);
}

/*******************************************************************************
**  函数名称  : Phone_TimeTask
**  函数功能  : 时间任务调度接口函数
**  输    入  : 无				
**  输    出  : 任务调度状态:  使能或禁止
**  全局变量  : 
**  调用函数  : 
**  中断资源  : 
**  备    注  :
*******************************************************************************/
FunctionalState Phone_TimeTask(void) 
{
    LZM_PublicTimerHandler(s_sPHTimer,PH_TIMERS_MAX);
    return ENABLE;
}
#endif
/*******************************************************************************
 *                             end of module
 *******************************************************************************/


/*******************************************************************************
 * File Name:           OpenAccount.c 
 * Function Describe:   
 * Relate Module:       快速开户功能
 * Writer:              Joneming
 * Date:                2013-06-17 
 * ReWriter:            Joneming
 * Date:                //进行开户操作之前先把开户服务器设到相应的IP地址上,开户服务器的端口号设到相应的端口号地址上,
                        //开户流程:终端从主IP上下线,把开户服务器地址与主IP地址互换,终端再连接开户服务器,连接成功后发开户指令
                        //接收到解锁应答后,终端再恢复原来的主IP地址,之后再进行正常的上线
 *******************************************************************************/
#include "include.h"
////////////////////////////////
typedef enum
{
    OA_STATUS_SAVE_IP,   //保存IP地址
    OA_STATUS_UNLOCK,    //解锁,
    OA_STATUS_LOCK,      //锁定,
    OA_STATUS_VALID_FLAG,//有效标志
    OA_STATUS_MAX         //
}E_OA_STATUS;

enum 
{
    ACCOUNT_TIMER_TASK,//
    ACCOUNT_TIMER_SEND,//
    ACCOUNT_TIMERS_MAX
}T_STACCOUNTTIME;

static LZM_TIMER s_stAccountTimer[ACCOUNT_TIMERS_MAX];

static unsigned char s_ucfisrtRunFlag = 0;
static unsigned char s_ucOpenAccountStatus = 0;
/////////////////////////
void OpenAccount_RecoverOriginalIPAddress(void);
/*************************************************************
** 函数名称: OpenAccount_SaveStatus
** 功能描述: 保存快速开户锁定状态
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void OpenAccount_SaveStatus(void)
{
    PUBLIC_SETBIT(s_ucOpenAccountStatus, OA_STATUS_VALID_FLAG);    
    Public_WriteDataToFRAM(FRAM_QUICK_OPEN_ACCOUNT_ADDR, &s_ucOpenAccountStatus,FRAM_QUICK_OPEN_ACCOUNT_LEN);
}
/*************************************************************
** 函数名称: OpenAccount_ReadLockStatus
** 功能描述: 读取快速开户锁定状态
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void OpenAccount_ReadLockStatus(void)
{
    unsigned char max;
    unsigned char flag = 0;    
    if(Public_ReadDataFromFRAM(FRAM_QUICK_OPEN_ACCOUNT_ADDR, &flag,FRAM_QUICK_OPEN_ACCOUNT_LEN))
    {
        max = 0;
        PUBLIC_SETBIT(max, OA_STATUS_MAX);
        if(flag>=max)
        {
            flag = 0;
        }
        else
        if(!PUBLIC_CHECKBIT(flag,OA_STATUS_VALID_FLAG))
        {
            flag = 0;
        }
        else
        if((PUBLIC_CHECKBIT(flag,OA_STATUS_UNLOCK)&&PUBLIC_CHECKBIT(flag,OA_STATUS_LOCK))
         ||(!PUBLIC_CHECKBIT(flag,OA_STATUS_UNLOCK)&&!PUBLIC_CHECKBIT(flag,OA_STATUS_LOCK)))
        {
            flag = 0;
        }         
    }
    s_ucOpenAccountStatus = flag;
    ///////////////////
    if(0==s_ucOpenAccountStatus)//
    {
        s_ucOpenAccountStatus = 0;
        PUBLIC_SETBIT(s_ucOpenAccountStatus,OA_STATUS_UNLOCK);
        PUBLIC_CLRBIT(s_ucOpenAccountStatus,OA_STATUS_LOCK);
        OpenAccount_SaveStatus();
    }    
}
/*************************************************************
** 函数名称: OpenAccount_GetCurLockStatus
** 功能描述: 获得快速开户锁定状态
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char OpenAccount_GetCurLockStatus(void)
{
    if(PUBLIC_CHECKBIT(s_ucOpenAccountStatus,OA_STATUS_LOCK)&&0==PUBLIC_CHECKBIT(s_ucOpenAccountStatus,OA_STATUS_UNLOCK))return 1;
    return 0;
}
/*************************************************************
** 函数名称: OpenAccount_SetLockStatus
** 功能描述: 快速开户置为锁定状态
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void OpenAccount_SetLockStatus(void)
{
    PUBLIC_SETBIT(s_ucOpenAccountStatus,OA_STATUS_LOCK);
    PUBLIC_CLRBIT(s_ucOpenAccountStatus,OA_STATUS_UNLOCK);
    OpenAccount_SaveStatus();
}
/*************************************************************
** 函数名称: OpenAccount_ClearLockStatus
** 功能描述: 快速开户清除锁定状态
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void OpenAccount_ClearLockStatus(void)
{
    ClearTerminalAuthorizationCode(CHANNEL_DATA_1);
    ClearTerminalAuthorizationCode(CHANNEL_DATA_2);
    ///////////////////////
    PUBLIC_SETBIT(s_ucOpenAccountStatus,OA_STATUS_UNLOCK);
    PUBLIC_CLRBIT(s_ucOpenAccountStatus,OA_STATUS_LOCK);
    OpenAccount_SaveStatus();
    LZM_PublicKillTimer(&s_stAccountTimer[ACCOUNT_TIMER_SEND]);
    LZM_PublicKillTimer(&s_stAccountTimer[ACCOUNT_TIMER_TASK]);
    ///////////////////////////////////
    OpenAccount_RecoverOriginalIPAddress();
}
/*************************************************************
** 函数名称: OpenAccount_CheckOnlineAndSendQuickOpenAccount
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void OpenAccount_CheckOnlineAndSendQuickOpenAccount(void)
{
    u8 channel = CHANNEL_DATA_1;//为编译通过而添加
    if(1==communicatio_GetMudulState(COMMUNICATE_STATE))//
    {
        if(ACK_OK==RadioProtocol_OpenAccount(channel))//
        {
            LZM_PublicKillTimer(&s_stAccountTimer[ACCOUNT_TIMER_SEND]);
            LZM_PublicSetOnceTimer(&s_stAccountTimer[ACCOUNT_TIMER_TASK],PUBLICSECS(30),OpenAccount_CheckAndDisposeLockStatus);
        }
    }   
}
/*************************************************************
** 函数名称: OpenAccount_SetParamForLinkToFreightPlatform
** 功能描述: 连接到指定的货运平台(开户参数设置:备份IP设置为开户服务器,UDP设置为开户端口号)
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char OpenAccount_ChangeMainIPAndBackUpIP(void)
{
    unsigned char len;
    unsigned char buffer[30]={0};
    unsigned char IPBuff[30];           //IPBuff
    unsigned char IPBakBuff[30];         //IPBuff
    unsigned char TCPBuff[6];           //TCPBuff    
    unsigned char TCPBakBuff[6];         //TCPBuff
    unsigned char IPlen,IPBaklen,TCPlen,TCPBaklen;
    
    IPlen = EepromPram_ReadPram(E2_MAIN_SERVER_IP_ID,IPBuff);
    TCPlen = EepromPram_ReadPram(E2_MAIN_SERVER_TCP_PORT_ID,TCPBuff);
    IPBaklen = EepromPram_ReadPram(E2_OPEN_ACCOUNT_PARAM_IP_ID,IPBakBuff);
    TCPBaklen =EepromPram_ReadPram(E2_OPEN_ACCOUNT_PARAM_TCP_ID,TCPBakBuff);        
    if(0==IPlen||0==TCPlen||0==IPBaklen||0==TCPBaklen)//
    {
       return 1;
    }
    ////////////////把主IP地址写入暂存地址作备份//////////////////////////////
    EepromPram_WritePram(E2_OPEN_ACCOUNT_PARAM_IP_ID, IPBuff, IPlen);
    len=EepromPram_ReadPram(E2_OPEN_ACCOUNT_PARAM_IP_ID,buffer);
    if(Public_CheckArrayValIsEqual(buffer,IPBuff,len)||(len!=IPlen))
    {
        return 1;
    }
    ////////////把TCP写入暂存地址作备份////////////////
    EepromPram_WritePram(E2_OPEN_ACCOUNT_PARAM_TCP_ID,TCPBuff,TCPlen);
    len=EepromPram_ReadPram(E2_OPEN_ACCOUNT_PARAM_TCP_ID,buffer);
    if(Public_CheckArrayValIsEqual(buffer,TCPBuff,len)||(len!=TCPlen))
    {
        return 1;
    }
    ////////把开户服务器地址写入主IP///////////////
    EepromPram_WritePram(E2_MAIN_SERVER_IP_ID, IPBakBuff, IPBaklen);
    len=EepromPram_ReadPram(E2_MAIN_SERVER_IP_ID,buffer);
    if(Public_CheckArrayValIsEqual(buffer,IPBakBuff,len)||(len!=IPBaklen))
    {
        return 1;
    }
    ///////////把开户端口号地址写入TCP////////////
    EepromPram_WritePram(E2_MAIN_SERVER_TCP_PORT_ID, TCPBakBuff, TCPBaklen);
    len=EepromPram_ReadPram(E2_MAIN_SERVER_TCP_PORT_ID,buffer);
    if(Public_CheckArrayValIsEqual(buffer,TCPBakBuff,len)||(len!=TCPBaklen))
    {
        return 1;
    }
    return 0;
}
/*************************************************************
** 函数名称: OpenAccount_SetParamForLinkToFreightPlatform
** 功能描述: 连接到指定的货运平台(开户参数设置:备份IP设置为开户服务器,UDP设置为开户端口号)
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char OpenAccount_SetParamForLinkToFreightPlatform(void)
{
    unsigned char result;
    if(0==PUBLIC_CHECKBIT(s_ucOpenAccountStatus,OA_STATUS_SAVE_IP))
    {
        result=OpenAccount_ChangeMainIPAndBackUpIP();
        if(0==result)
        {
            PUBLIC_SETBIT(s_ucOpenAccountStatus,OA_STATUS_SAVE_IP);
            OpenAccount_SaveStatus();
            Communication_Close();
            LZM_PublicSetOnceTimer(&s_stAccountTimer[ACCOUNT_TIMER_SEND],PUBLICSECS(2),Communication_Open);
        }
        return result;
    }
    return 0;
}
/*************************************************************
** 函数名称: OpenAccount_LinkGovServer
** 功能描述: 连接到指定的货运平台
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void OpenAccount_LinkGovServer(void)
{
    //SetEvTask(EV_LINK_GOV_SERVER);    
    LZM_PublicSetCycTimer(&s_stAccountTimer[ACCOUNT_TIMER_SEND],PUBLICSECS(5),OpenAccount_CheckOnlineAndSendQuickOpenAccount); 
}
/*************************************************************
** 函数名称: OpenAccount_LinkToFreightPlatform
** 功能描述: 连接到指定的货运平台
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void OpenAccount_LinkToFreightPlatform(void)
{
    //Communication_Close();
    if(0==OpenAccount_SetParamForLinkToFreightPlatform())
    {
        LZM_PublicSetOnceTimer(&s_stAccountTimer[ACCOUNT_TIMER_TASK],PUBLICSECS(6),OpenAccount_LinkGovServer);
    }
    else
    {
        Public_ShowTextInfo("锁定设置出错", PUBLICSECS(3));
    }
    //LZM_PublicSetOnceTimer(&s_stAccountTimer[ACCOUNT_TIMER_SEND],PUBLICSECS(2),Communication_Open);
    /////////////////////////////    
}
/*************************************************************
** 函数名称: OpenAccount_CheckAndDisposeLockStatus
** 功能描述: 检查和处理快速开户锁定状态
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void OpenAccount_CheckAndDisposeLockStatus(void)
{
    if(0==s_ucfisrtRunFlag)
    {
        OpenAccount_ParameterInitialize();
    }
    ///////////////////////
    if(OpenAccount_GetCurLockStatus())//连到
    {
        Public_ShowTextInfo("快速开户中", PUBLICSECS(20));
        OpenAccount_LinkToFreightPlatform();
    }
    else
    {
        Public_ShowTextInfo("终端已解锁", PUBLICSECS(5));
    }
    ///////////////////////
}
/*************************************************************
** 函数名称: OpenAccount_CheckAndDisposeLockStatus
** 功能描述: 检查和处理快速开户锁定状态
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void OpenAccount_RecoverOriginalIPAddress(void)
{
    if(PUBLIC_CHECKBIT(s_ucOpenAccountStatus,OA_STATUS_SAVE_IP))
    {
        Communication_Close();
        ////////////////////////
        if(0==OpenAccount_ChangeMainIPAndBackUpIP())
        {
            PUBLIC_CLRBIT(s_ucOpenAccountStatus,OA_STATUS_SAVE_IP);
            OpenAccount_SaveStatus();
        }
        //////////////////////////////
        Communication_Open();
    }
}
/*************************************************************
** 函数名称: OpenAccount_DisposeRadioProtocol
** 功能描述: 快速开户协议解析
** 入口参数: pBuffer:数据首地址,BufferLen:数据长度
** 出口参数: 无
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void OpenAccount_DisposeRadioProtocol(unsigned char *pBuffer,unsigned short BufferLen)
{
    unsigned short ID;
    unsigned char result;
    ID=Public_ConvertBufferToShort(&pBuffer[2]);
    if(0x0110==ID)//快速开户
    {
        result = pBuffer[4];
        if(1 == result)
        {
            OpenAccount_SetLockStatus();
            Public_ShowTextInfo("终端解锁失败", PUBLICSECS(6));
            LZM_PublicKillTimer(&s_stAccountTimer[ACCOUNT_TIMER_TASK]);
            OpenAccount_RecoverOriginalIPAddress();  
        }
        else
        //if(0 == result||5 == result)//开户成功
        {
            OpenAccount_ClearLockStatus();
            Public_ShowTextInfo("终端解锁成功", PUBLICSECS(5));
            LZM_PublicSetOnceTimer(&s_stAccountTimer[ACCOUNT_TIMER_TASK],PUBLICSECS(4),ShowMainInterface);           
        }
    }
}
/*************************************************************
** 函数名称: OpenAccount_GetEncryptFlag
** 功能描述: 是否加密
** 入口参数: 
** 出口参数: 无
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char OpenAccount_GetEncryptFlag(void)
{
    unsigned char flag;
    if(!EepromPram_ReadPram(E2_TOPLIGHT_CONTROL_ID, &flag))
    {
        flag = 0;
    }
    return (1==flag)?1:0;
}
/*************************************************************
** 函数名称: OpenAccount_TimeTask
** 功能描述: 快速开户定时器处理函数
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
FunctionalState OpenAccount_TimeTask(void)
{
    LZM_PublicTimerHandler(s_stAccountTimer,ACCOUNT_TIMERS_MAX);
    return ENABLE;
}
/*************************************************************
** 函数名称: OpenAccountParameterInitialize
** 功能描述: 快速开户参数初始化
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void OpenAccount_ParameterInitialize(void) 
{
    LZM_PublicKillTimerAll(s_stAccountTimer,ACCOUNT_TIMERS_MAX);
    ////////////////////////////// 
    OpenAccount_ReadLockStatus();    
    if(OpenAccount_GetCurLockStatus())
    {
        LZM_PublicSetOnceTimer(&s_stAccountTimer[ACCOUNT_TIMER_TASK],PUBLICSECS(15),OpenAccount_CheckAndDisposeLockStatus);
    }
    /////////////////////////
    s_ucfisrtRunFlag = 1;
    SetTimerTask(TIME_ACCOUNT,2);
}
/*******************************************************************************
 *                             end of module
 *******************************************************************************/


/**
  ****************************************************************************** 
  * @file    modem_api.c
  * @author  miaoyahan
  * @version V1.0.0
  * @date    2013-08-25
  * @brief   
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#ifdef EYE_MODEM
#include "include.h"
#else
#include  <app_cfg.h>
#include  <includes.h>
#endif

/*
*********************************************************************************************************
*                                               DEFINES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES   本地变量
*********************************************************************************************************
*/
static u8  ApiPortLockFlg;//API接口上锁标志 0为空闲  1为上锁
static u8  ApiPortLockType;//API接口上锁类型类型 对应Modem_API_PORT枚举

static u8  SocOpenWait[MODEM_SOC_NUM];//打开连接等待标志 用于等待服务器响应 1表示正在等待 0为空闲
static u32 SocOpenOut[MODEM_SOC_NUM];//等待超时时间 单位时间为 一个调度
/*
*********************************************************************************************************
*                                          GLOBAL VARIABLES    全局变量
*********************************************************************************************************
*/
ST_MOD_API        modapi;//modem模块api结构体

CALL_SOC_FUNCTION     Modem_Call_Soc_Function[MODEM_SOC_NUM];//soc回调函数数组，用于存放数据接收函数
CALL_SET_IP_FUNCTION  Modem_Call_SocIpConf_Function[MODEM_SOC_NUM];//soc ip回调，用于配置socket申请后的IP信息

/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

/*
  * @brief  API接口上锁
  * @param  当前的API接口类型
  * @retval 0:空闲
  *         1:其他API接口正在加锁
  */ 
u8  Modem_Api_PortLock (Modem_API_PORT api_port) 
{
    if(ApiPortLockFlg)//已上锁
    {
        if(ApiPortLockType == api_port)
        {
            return 0;
        }
        else
        {
            return 1;
        }
    }
    modapi.index    = 0;
    ApiPortLockFlg  = 1;
    ApiPortLockType = api_port;
    return 0;
}
/*
  * @brief  API接口释放
  * @param  当前的API接口类型
  * @retval None
  */ 
void  Modem_Api_PortFree (Modem_API_PORT api_port) 
{
    if(ApiPortLockFlg)//已上锁
    {
        ApiPortLockFlg = 0;
    }
    modapi.index = 0;
}
/*
  * @brief  API接口状态
  * @param  None
  * @retval 0:空闲
  *         1:其他API接口正在加锁
  */ 
u8  Modem_Api_PortIs (Modem_API_PORT api_port) 
{
    if(ApiPortLockFlg)//已上锁
    {
        if(ApiPortLockType != api_port)
        {
            return 1;//其他API接口正在加锁
        }
    }
    return 0;//空闲
}
/*
  * @brief  modem初始化，主要初始化发送数据时的队列.
  * @param  None
  * @retval 返回初始化结果
  */ 
void  Modem_Api_Init (void) 
{      
#ifdef EYE_MODEM
    Net_First_Init();
    #ifdef MODEM_APP_SECOND_H
    Net_Second_Init();
    #endif
		#ifdef ICCARD_CERTIFICATE_SERVICE_H
		CertService_Init();
		#endif
#endif
}
/*
  * @brief  modem开始,开启通讯模块内核,一般在系统初始化时调用此函数
  * @param  None
  * @retval None
  */ 
void  Modem_Api_Start(void) 
{
    Modem_State_Switch(MODEM_BSP);
}
/*
  * @brief  modem复位
  * @param  None
  * @retval None
  */ 
void  Modem_Api_Rst (void) 
{
    Modem_State_Switch(MODEM_ERROR);
}
/*
  * @brief  modem关机
  * @param  delay :关机前延时时间用于向平台发送确认指令
  * @retval None
  */ 
void  Modem_Api_Off (u16 delay) 
{
    Modem_State_Delay(delay,MODON_OFF);
}
/*
  * @brief  modem开机
  * @param  None
  * @retval None
  */  
void  Modem_Api_On (void) 
{
    Modem_State_Switch(MODEM_INIT);
}


/*
  * @brief  短信数据接收回调函数
  * @param  type :短信类型
  *
  * @param  *p :指向接收的数据
  * @param  len:接收数据的长度
  * @retval None
  */
void  Modem_Api_CallSms(MODSMS_TYPE type, u8 *p, u16 len)
{
#ifdef EYE_MODEM
    Net_Other_SmsCall(type,p,len);
#endif
}
/*
  * @brief  ftp连接数据接收回调函数
  * @param  *p :指向接收的数据
  * @param  len:接收数据的长度,最大长度为512
  * @retval None
  */
void  Modem_Api_CallFtp(u8 *p, u16 len)
{
    #ifdef MODEM_DEBUG_H
    if(ModDbInfType == MODDB_BASE)
    {
        MODDB_PRTFBAS("modem->Ftp_Data = %d\r\n",modftp.fsize);               
    }
    #endif   
#ifdef EYE_MODEM
    Net_Other_FtpCall(p,len);
#endif
} 

/*
  * @brief  来电回调函数,新振铃来后回调一次
  * @param  *p :指向接收的数据
  * @param  len:接收数据的长度
  * @retval None
  */
void  Modem_Api_CallRing(u8 *p, u16 len)
{
}
/*
  * @brief  电话状态回调函数
  * @param  *pPhone:指向电话号码
  * @param  status: 0为挂断; 1:接通；2:新来电
  * @retval 新来电时返回:  0:直接拒接来电;  1:正在等待用户接听来电
  */
u8  Modem_Api_CallPhoneStatus(u8 *pPhone,u8 status)
{
    u8 ch=0;
#if EYE_MODEM
    ch = Public_DisposePhoneStatus(pPhone,status);//通知显示屏对方已挂断
#else
    ch = 1;
#endif
    return ch;
}
/*
  * @brief  应用程序查询modem的状态
  * @param  Soc号
  * @retval 返回modem所处的业务
  */
MODON_STA  Modem_Api_OnSta(void)
{
    return (modem.onsta);
}
/*
  * @brief  查询modem类型
  * @param  None
  * @retval 
            bit0，0：不支持GPRS通信，     1：支持GPRS通信；
            bit1，0：不支持CDMA通信，     1：支持CDMA通信；
            bit2，0：不支持TD-SCDMA通信， 1：支持TD-SCDMA通信；
            bit3，0：不支持WCDMA通信，    1：支持WCDMA通信；
            bit4，0：不支持CDMA2000通信， 1：支持CDMA2000通信。
            bit5，0：不支持TD-LTE通信，   1：支持TD-LTE通信；
            bit7，0：不支持其他通信方式， 1：支持其他通信方式。  
  */
u8  Modem_Api_Type(void)
{
    return Modem_State_Type(1);
}
/*
  * @brief  应用层调用CCID
  * @param  *p :指向ccid号码
  * @retval None
  */
void  Modem_Api_CCID(u8 *p)
{
    memcpy(p,modem.ccid,10);
} 


/*
  * @brief  应用程序查询指定soc链接状态
  * @param  Soc号
  * @retval 返回链接状态,在线返回真，否则返回假
  */
BOOL  Modem_Api_SocSta(u8 soc)
{
    return (Modem_State_SocSta(soc));
}
/**
  * @brief  发送数据到指定soc
  * @param  SocNum：soc号
  * @param  *data: 指向发送的数据
  * @param  len  : 发送数据的长度.
  * @retval 返回实际发送数据的长度，0表示失败.
  */
u16 Modem_Api_Socket_Send(u8 SocNum ,u8 *data, u16 len)
{
    u16 SendLen=0;

    if((modem.state == MODEM_ON)&&(Modem_State_SocSta(SocNum)))
    {
        if(ModLib_NodIns(&ModSocNode[SocNum],INS_ALL,data,len))
        {
            SendLen = len;
        }
    }

    return SendLen;
}
/*
  * @brief  设置是否使用域名连接，主要检查IP地址
  * @param  *pIp -> 指向IP地址
  * @retval None
  */
void  Modem_Api_SocDnsSet(u8 *pIp)
{
    u8 result;

    result = ModLib_ChkIp(pIp);

    if(result == 0)
    {
        modem.socdns = 0;//IP地址连接
    }
    else
    {
        modem.socdns = 1;//域名连接
    }
}

/*
  * @brief  打开指定soc连接.失败每隔1分钟重发一次，共5次
  *          注意!!!默认是使用IP连接，如果使用域名则需要在Call_IpConf函数中
  *          调用Modem_Api_SocDnsSet(1)来设置.
  * @param SocNum:要打开的soc号
  * @param  void(*Call_IpConf)( //函数用于配置soc连接
                    u8 *pMode,  //指向连接类型，tcp或udp
                    u8 *pAddr,  //指向ip地址
                    u8 *pPort,) //指向ip的端口号

   * @param    void(*Call_Soc)( //函数回传soc接收到的数据
                    u8 *pRec,   //指向接收到的数据
                    u16 Reclen) //接收数据的长度                                                   
  * @param  None
  * @retval 返回处理结果
  */
MODERR_TYPE Modem_Api_Socket_Open(u8 SocNum
                            ,void(*Call_IpConf)(u8 *pMode ,u8 *pAddr, u8 *pPort)
                            ,void(*Call_Soc)(u8 *pRec, u16 Reclen))
{  
    u8  Tab_Mode[5]={0};
    u8  Tab_Addr[30]={0};
    u8  Tab_Port[7]={0};

    if(SocNum >= MODEM_SOC_NUM)
    {
        return MOD_ER;//错误
    }

    if(modem.state != MODEM_ON)
    {
        return MOD_DOWN;//下线状态
    }
    
    if(modapi.wait != MODON_IDLE)
    {
        return MOD_DO;//modem忙
    }
    
    if(Modem_Api_PortIs(PORT_OPEN))
    {
        return MOD_DO;//其他API接口正在使用
    }

    if(modem.socopen != SocNum)//检查上次命令是否发生完毕
    {
        if(SocOpenWait[modem.socopen])
        {
            if((SocOpenWait[modem.socopen]&0x0f) != 0x0f)
            {
                return MOD_DO;//modem忙
            }
        }
    }
    
    if(SocOpenWait[SocNum] == 0)//单步归零
    {
        modapi.index = 0;
    }
    else if(SocOpenWait[SocNum] == 0x0f)//确保已打开命令成功
    {
        if(Modem_IpSock_STA[SocNum] == MODSOC_ONLN)//上线判断
        {
            modapi.index = 0;
            SocOpenWait[SocNum] = 0;  
            Modem_PrtfTo(MODPRTF_SOCOK); 
            return MOD_OK;//成功
        }
        else if(Modem_IpSock_STA[SocNum] == MODSOC_ERR)//错误判断
        {
            modapi.index = 0;
            SocOpenWait[SocNum] = 0;
            Modem_PrtfTo(MODPRTF_SOCOT);
            return MOD_ER;//错误
        }
        
        if(SocOpenOut[SocNum] < (MODEM_TSEC*60))//超时判断 1分钟超时时间
        {
            SocOpenOut[SocNum]++;
            return MOD_DO;//正忙
        }
        else
        {
            modapi.index = 0;
            SocOpenWait[SocNum] = 0;
            Modem_PrtfTo(MODPRTF_SOCOT);  
            return MOD_ER;//超时
        }
    }

    Modem_Api_PortLock(PORT_OPEN);

    switch (modapi.index)
    {
        case 0:
        {
            Modem_Call_Soc_Function[SocNum] = Call_Soc;
            Modem_Call_SocIpConf_Function[SocNum] = Call_IpConf;//保存soc 设置ip参数对应的回调函数
            Call_IpConf(Tab_Mode,Tab_Addr,Tab_Port);//获取ip地址和端口
            Modem_Api_SocDnsSet(Tab_Addr);
            SocOpenWait[SocNum] = 0x01;
            modem.socopen = SocNum;//要打开soc号
            modapi.index++;
        }
        break;
        case 1:
        {
            SocOpenWait[SocNum] |= 0x02;
            modapi.wait = MODON_DNSIP; 
            Modem_State_SwitchOn(MODON_DNSIP);//设置连接类型
            modapi.index++;
        }
        break;
        case 2:
        {     
            SocOpenWait[SocNum] |= 0x04;
            modapi.wait = MODON_IPOPEN; 
            Modem_State_SwitchOn(MODON_IPOPEN);//连接服务器
            modapi.index++;
        }
        break;
        case 3://打开连接模块响应成功
        {     
            SocOpenWait[SocNum] |= 0x08;
            SocOpenOut[SocNum]  = 0;
            Modem_Api_PortFree(PORT_OPEN);
        }
        break;
        default://打开连接失败
        {
            SocOpenWait[SocNum] = 0;
            Modem_Api_PortFree(PORT_OPEN);
            return MOD_ER;
        }
    } 
    return (MOD_IDLE);
}
/*
  * @brief  关闭指定Soc连接.
  * @param  Soc号
  * @retval 返回处理结果
  */
MODERR_TYPE  Modem_Api_Socket_Close(u8 SocNum)
{   

    if(SocNum >= MODEM_SOC_NUM)
    {
        return MOD_ER;//错误
    }
    if(Modem_Api_PortIs(PORT_CLOSE))
    {
        return MOD_DO;//其他API接口正在使用
    }
    if(modem.state != MODEM_ON)
    {
        return MOD_DOWN;//下线状态
    }
    if(modapi.wait != MODON_IDLE)
    {
        return MOD_DO;//modem忙
    }
    
    Modem_Api_PortLock(PORT_CLOSE);
    
    switch (modapi.index)
    {
        case 0:
        {
            modapi.index++;
            modem.socclose = SocNum;
            modapi.wait = MODON_IPCLOSE; 
            Modem_State_SwitchOn(MODON_IPCLOSE);
        }
        break;
        case 1://成功
        {     
            Modem_Api_PortFree(PORT_CLOSE);
            return MOD_OK;
        }
        default://失败
        {
            Modem_Api_PortFree(PORT_CLOSE);
            return MOD_ER;
        }

    }
    return (MOD_IDLE);
}
                                            
/*
  * @brief  从指定服务器下载文件，结束需调用Modem_Api_FtpIsOk判断文件下载是否完整
  * @param  *pIp:指向IP地址
  * @param  *pPort:指向端口
  * @param  *pUser:指向用户名
  * @param  *pPass:指向密码
  * @param  *pFpath:指向路径
  * @param  *pFname:指向文件名
  * @retval 返回处理结果
  */
MODERR_TYPE  Modem_Api_Ftp(u8 *pIp,u8 *pPort
                                             ,u8 *pUser,u8 *pPass
                                             ,u8 *pFpath,u8 *pFname)
{
    if(modem.state != MODEM_ON)
    {
        return MOD_DOWN;//下线状态
    }
    
    if(modapi.wait != MODON_IDLE)
    {
        return MOD_DO;//modem忙
    }
    
    if(Modem_Api_PortIs(PORT_FTP))
    {
        return MOD_DO;//其他接口已加锁
    }
    
    Modem_Api_PortLock(PORT_FTP);

    switch (modapi.index)
    {
        case 0://装载参数
        {
            modapi.index++;
            strcpy((char*)modftp.ip,(char*)pIp);       //ip地址
            strcpy((char*)modftp.port,(char*)pPort);   //端口
            strcpy((char*)modftp.user,(char*)pUser);   //用户名
            strcpy((char*)modftp.pass,(char*)pPass);   //密码
            strcpy((char*)modftp.fpath,(char*)pFpath); //文件路径
            strcpy((char*)modftp.fname,(char*)pFname); //文件名
            #ifdef MODEM_DEBUG_H
            if(ModDbInfType == MODDB_BASE)
            {
                MODDB_PRTFBAS("modem->Ftp_Ip   = %s\r\n",(char*)modftp.ip);
                MODDB_PRTFBAS("modem->Ftp_Port = %s\r\n",(char*)modftp.port);     
                MODDB_PRTFBAS("modem->Ftp_User = %s\r\n",(char*)modftp.user);
                MODDB_PRTFBAS("modem->Ftp_Pass = %s\r\n",(char*)modftp.pass);
                MODDB_PRTFBAS("modem->Ftp_Path = %s\r\n",(char*)modftp.fpath);
                MODDB_PRTFBAS("modem->Ftp_Name = %s\r\n\r\n",(char*)modftp.fname);                
            }
            #endif             
        }
        break;
        case 1://关闭所有链路
        {
            modapi.index++;
            modapi.wait = MODON_IPCLOSE; 
            modem.socclose = MODEM_SOC_NUM;
            Modem_State_SwitchOn(MODON_IPCLOSE);
        }
        break;
        case 2://进入ftp状态
        {
            modapi.index++;  
            modapi.wait = MODON_FTP; 
            Modem_State_SwitchOn(MODON_FTP);
        }
        break;    
        case 3://下载完毕
        {     
             Modem_Api_PortFree(PORT_FTP);
             return (MOD_OK);
        }
        default://关闭连接失败
        {
            #ifdef MODEM_DEBUG_H
            if(ModDbInfType == MODDB_BASE)
            {
                MODDB_PRTFBAS("modem->Ftp_Err Please Check Value!!!\r\n");             
            }
            #endif        
             Modem_Api_PortFree(PORT_FTP);
            return MOD_ER;
        }
    }   
    return (MOD_IDLE);
}
/*
  * @brief  ftp文件下载结果
  * @param  None
  * @retval 返回文件下载结果，0:空闲 1:正确 2:错误
  */
u8  Modem_Api_FtpResult(void)
{
    return modftp.fres;
}
/*
  * @brief  ftp下载文件的大小
  * @param  None
  * @retval 返回ftp下载文件的大小
  */
u32  Modem_Api_FtpSize(void)
{
    return modftp.fsize;
}
/*
  * @brief  向指定号码发送手机短信
  * @param  *pNum :指向手机号码,如果为NULL则使用接收号码
  * @param  *pData :指向短信数据内容
  * @param  len:短信数据的长度,一次只能发送(MODEM_SMS_SENDMAX)个字节，暂不支持短信拆包
  * @retval 返回处理结果
  */
BOOL  Modem_Api_SmsSend(MODSMS_TYPE Type,u8 *pNum,u8 *pData, u8 len)
{
    if(modsms.send)
    {
        return (__FALSE);
    }
    if(modem.state != MODEM_ON)
    {
        return (__FALSE);
    }
    
    return (Modem_State_Sms_Dt(Type,pNum,pData,len));

}
/*
  * @brief  拨打电话
  * @param  *pData :指向电话号码
  * @param  TypeSpeak:扬声器状态 1为打开扬声器  0为关闭
  * @retval 返回处理结果
  */
BOOL  Modem_Api_Phone_Atd(u8 *pData, u8 TypeSpeak)
{
    if(modem.state != MODEM_ON)
    {
        return (__FALSE);//下线状态
    }
    
    if(modapi.wait == MODON_IPOPEN)
    {
        Modem_State_Push();
        modphone.otherflg = 1;
    }
    else if(modapi.wait != MODON_IDLE)
    {
        return (__FALSE);//modem忙
    }

    
    if(modphone.state != MODPH_IDLE)
    {
        return (__FALSE);//modem正在处理电话业务
    }
          
    memset(modphone.num,0,sizeof(modphone.num));
    memcpy(modphone.num,pData,ModLib_AtLen(pData)); 
    strcat((char*)modphone.num,";"); 
    
    Modem_State_SwitchPhone(MODPH_ATD);
    modapi.wait = MODON_PHONE; 
    Modem_State_SwitchOn(MODON_PHONE);

    if(TypeSpeak == 0)
    {
        Modem_Bsp_SpkOff(BSP_SPK_TALK);
    }
    else
    {
        Modem_Bsp_SpkOn(BSP_SPK_TALK);
    }

    return (__TRUE);

}


/*
  * @brief  挂断电话
  * @param  None
  * @retval 返回处理结果
  */
BOOL  Modem_Api_Phone_Ath(void)
{
    if(modem.state != MODEM_ON)
    {
        return (__FALSE);//下线状态
    }

    memset(modphone.num,0,sizeof(modphone.num));
    Modem_State_SwitchPhone(MODPH_ATH);
    modapi.wait = MODON_PHONE; 
    Modem_State_SwitchOn(MODON_PHONE);

    if(modem.wait != MODWT_IDLE)
    {
        modem.wait = MODWT_IDLE;
    }    
    return (__TRUE);

}
/*
  * @brief  接听电话
  * @param  None
  * @retval 返回处理结果
  */
BOOL  Modem_Api_Phone_Ata(void)
{
    if(modem.state != MODEM_ON)
    {
        return (__FALSE);//下线状态
    }
    
    if(modapi.wait != MODON_PHONE)
    {
        return (__FALSE);//modem不在电话状态下
    }
    if(modphone.state != MODPH_RING)
    {
        return (__FALSE);//modem正在处理电话业务
    }

    Modem_State_SwitchPhone(MODPH_ATA);
    
    return (__TRUE);

}

/*
  * @brief  调节音量
  * @param  value :音量大小的数值
  * @retval 返回处理结果
  */
BOOL  Modem_Api_Phone_Clvl(u8 value)
{
    if(modem.state != MODEM_ON)
    {
        return (__FALSE);//下线状态
    }
    
    if(modapi.wait != MODON_PHONE)
    {
        return (__FALSE);//modem不在电话状态下
    }
    if(modphone.state != MODPH_IDLE)
    {
        return (__FALSE);//modem正在处理电话业务
    }
    if(value > 9)
    {
        value = 9;
    }
          
    modphone.clvl = value;
    Modem_State_SwitchPhone(MODPH_CLVL);
    
    return (__TRUE);

}
/*
  * @brief  获取当前音量
  * @param  None
  * @retval 返回当前音量
  */
u8    Modem_Api_Phone_ClvlGet(void)
{
    return modphone.clvl;
}
/*
  * @brief  设置音频通道
  * @param  channel :音频通道号
  * @retval 返回处理结果
  */
BOOL  Modem_Api_Phone_Audch(u8 channel)
{
    if(modem.state != MODEM_ON)
    {
        return (__FALSE);//下线状态
    }
    
    if(modapi.wait != MODON_PHONE)
    {
        return (__FALSE);//modem不在电话状态下
    }
    if(modphone.state != MODPH_IDLE)
    {
        return (__FALSE);//modem正在处理电话业务
    }
          
    modphone.audch= channel;
    Modem_State_SwitchPhone(MODPH_AUDCH);
    
    return (__TRUE);

}
/*
  * @brief  设置通话中按键
  * @param  *p :指向键值
  * @retval 返回处理结果
  */
BOOL  Modem_Api_Phone_Vts(u8 *p)
{
    if(modem.state != MODEM_ON)
    {
        return (__FALSE);//下线状态
    }
    
    if(modapi.wait != MODON_PHONE)
    {
        return (__FALSE);//modem不在电话状态下
    }
    if(modphone.state != MODPH_IDLE)
    {
        return (__FALSE);//modem正在处理电话业务
    }
          
    memset(modphone.vts,0,sizeof(modphone.vts));
    memcpy(modphone.vts,p,1);
    Modem_State_SwitchPhone(MODPH_VTS);
    
    return (__TRUE);

}
/*
  * @brief  查询模块是否支持TTS功能
  * @param  None
  * @retval 支持TTS返回1, 不支持返回0
  */
u8  Modem_Api_Tts_IsSpt(void)
{
    return Modem_State_Tts_Is(1);
}
/*
  * @brief  查询播报的状态
  * @param  None
  * @retval 正在播报返回1, 播报完毕返回0
  */
u8  Modem_Api_Tts_IsSta(void)
{
    return Modem_State_Tts_Is(0);
}
/*
  * @brief  查询TTS音量
  * @param  None
  * @retval 返回音量大小 大小 0--9 
  */
u8  Modem_Api_Tts_IsClvl(void)
{
    return modtts.clvl;
}
/*
  * @brief  调节TTS音量
  * @param  value:音量大小 0--9 
  * @retval 成功返回1, 失败返回0
  */
u8  Modem_Api_Tts_Clvl(u8 value)
{
    if(modtts.state != MODTTS_IDLE)
    {
        return 0;
    }
    if(value > 9)
    {
        value = 9;
    }
    modtts.clvl  = value;
    modtts.state = MODTTS_CLVL;
    modtts.send  = 1;
    return 1;
}
/*
  * @brief  停止播报
  * @param  None
  * @retval 成功返回1, 失败返回0
  */
u8  Modem_Api_Tts_Stop(void)
{
    if(modem.state < MODEM_ATI)
    {
        return 0;//不能播报
    }
    
    if(modtts.busy)//只有正在播报时才发送停止命令
    {
        modtts.busy = 0;
        modtts.state= MODTTS_STOP;
    }
    return 1;
}
/*
  * @brief  播报文本数据
  * @param  type : 0为立即播报,如果有正在播报的内容则会被打断；
  *                1为后续播报,如果有正在播报的内容则等待播完后再播报新的;
  * @param  *p :播报的数据
  * @param  lentts :播报数据的长度,如果参数为0则按字符串\0为结束
  * @retval 成功返回1, 失败返回0
  */
u8  Modem_Api_Tts_Play(u8 type,u8 *pTts, u8 lentts)
{
    if(modem.state < MODEM_ATI)
    {
        return 0;//不能播报
    }

    if(modem.onsta == MODON_PHONE)
    {
        return 0;//通话中禁止播报TTS
    }
    
    if(modem.onsta == MODON_FTP)
    {
        return 0;//FTP文件下载时禁止播报TTS
    }

    if(modtts.send)
    {
        return 0;//如果上次的数据还没发完则禁止新的数据播报
    }

    if(modtts.type)
    {
        return 0;//缓存中已经有一条等待播报的数据
    }

    if(Modem_State_Tts_Dt(type,pTts,lentts) == 0)
    {
        return 0;//非法数据
    }
    modtts.send = 1;
    
    return 1;
}





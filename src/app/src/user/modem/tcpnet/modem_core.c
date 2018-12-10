/*
********************************************************************************                                                           
*
* @file    modem_core.c
* @author  miaoyahan
* @version V1.0.0
* @date    2013-08-20
* @brief
*
********************************************************************************
*/

/*
********************************************************************************
*                               INCLUDES
********************************************************************************
*/
#ifdef EYE_MODEM
#include "include.h"
#else
#include  <app_cfg.h>
#include  <includes.h>
#endif

/*
********************************************************************************
*                               DEFIEN START
********************************************************************************
*/

/*
********************************************************************************
*                             LOCAL DEFINES
********************************************************************************
*/


/*
********************************************************************************
*                         LOCAL DATA TYPES
********************************************************************************
*/

/*
********************************************************************************
*                          LOCAL VARIABLES
********************************************************************************
*/
static u8  mbuf[MODEM_AT_SIZE];               
static u16 mlen; 

static u32 soc_rec_len;//接收的soc数据长度，用于调试观测，无实际应用

static u8  ZTE_IpStaNum;//查询指定soc连接状态0-5，中兴专用

static u8  FtpSectorBuf[520];//FTP扇区数据缓存，标准512个字节
static u16 FtpSectorBufLen;//FTP专用扇区数据焕春长度

/*
********************************************************************************
*                          GLOBAL VARIABLES
********************************************************************************
*/
MODEM_TYPE  Modem_Type;//模块类型

ST_MODAT    modat;     //AT命令处理
ST_MODEM    modem;     //modem全局结构体
ST_MODINF   modInf;    //modem运行的信息
ST_MODFTP   modftp;    //ftp业务全局结构体
ST_MODSMS   modsms;    //短信业务全局结构体
ST_MODHP    modphone;  //电话业务全局结构体
ST_MODTTS   modtts;    //tts业务全局结构体
SM_PARAM    modsmspdu; //PDU短信结构体

u8          ModSocBuf[MODEM_SOC_TX_SIZE*MODEM_SOC_NUM];
MODLIB_BUF  ModSocNdNum[MODEM_SOC_ND_SIZE*MODEM_SOC_NUM];
MODLIB_NOD  ModSocNode[MODEM_SOC_NUM];//soc节点数组结构体
MODSOC_STA  Modem_IpSock_STA[MODEM_SOC_NUM];//sock枚举数组，存放各个状态


u8  Modem_ActPinOnDly  = 2;//Modem激活引脚开机延时时间， 默认2s
u8  Modem_ActPinOffDly = 2;//Modem激活引脚关机延时时间，默认2s

u8 QuectelAtTtsFlag;//移远M35 TTS功能专用标志,用于进一步确认是否支持TTS功能

/*
********************************************************************************
*                          EXTERN VARIABLES
********************************************************************************
*/

/*
********************************************************************************
*                           FUNCTIONS EXTERN
********************************************************************************
*/


/*
********************************************************************************
*                              FUNCTIONS
********************************************************************************
*/
   
/*
  * @brief  modem标准打印函数
  * @param  None
  * @retval 返回结果
  */
void Modem_Printf (char *format, ...)
{
#ifdef MODEM_DEBUG_H
    va_list   v_args;
    char bufstr[MODDB_PRTF_SIZE + 1u];
    u16  len;

    va_start(v_args, format);
   (void)vsnprintf((char       *)&bufstr[0],
                   (u32         ) sizeof(bufstr),
                   (char const *) format,
                                  v_args);
    va_end(v_args);

    len = strlen(bufstr);
    
    if(ModDbInfType != MODDB_OFF)
    {
        Modem_Debug_UartTx((u8*)bufstr,len);
    }
#endif
}

/*
  * @brief  modem打印指定信息
  * @param  打印的类型
  * @retval 返回结果
  */
void Modem_PrtfTo (MODPRT_TYPE type)
{   
#ifdef MODEM_DEBUG_H
    u8  strbuf[sizeof(ModDbPrtf[0])+5]={0};
    u8  len = sizeof(ModDbPrtf[0]);

    if(ModDbInfType != MODDB_BASE)
    {
        return;
    }

    memcpy(strbuf,(u8*)&ModDbPrtf[type][0],len);
    strcat((char*)strbuf,"\r\n\r\n");
    Modem_Printf("%s",strbuf);
#endif    
}

/**
  * @brief  向串口发送AT命令
  * @param  *str: 指向发送modem的AT指令.
  * @retval None.
  */
static void send_cmd (u8 *str) 
{
    u8 len = strlen((char*)str);
    Modem_Uart_NodClr();                      //清空上条残留命令add jonening
    Modem_Uart_Send(str,len);

    if(modem.state > MODEM_OFF)//只在模块启动后累加
    {
        modem.atack++;//每发送一条AT命令加1，如果响应则清零0
    }
}
/**
  * @brief  状态机延时,单位为秒
  * @param  要延时的时间.
  * @retval None.
  */
void Modem_Delay (u8 delay) 
{
    if(delay != 0)
    {
       modem.delay= MODEM_TSEC*delay;
    }
    else
    {
        modem.delay= 1;//50ms
    }
}
/**
  * @brief  对比Modem返回的数据是否与想要的相同？.
  * @param  *pRecbuf: 指向将要做对比数据的缓存区.
  * @param  nRecLen: 将要做对比数据缓存区的长度.
  * @param  *Call_Ati: modem 信息的回调函数.
  * @retval 返回处理结果.
  */
static BOOL  Modem_StrCmp_Base(u8 *pRecbuf , u16 nRecLen)
{
    if(ModLib_AtCmp(mbuf,"ERROR") == 0)
    {   
        modem.delay = 0;
    }
    else if(ModLib_AtCmp(mbuf,modat.recbuf) == 0)
    {
        return (__TRUE);
    }
    return (__FALSE);
}
/**
  * @brief  对比Modem返回的数据是否与想要的相同？.
  * @param  *pRecbuf: 指向将要做对比数据的缓存区.
  * @param  nRecLen: 将要做对比数据缓存区的长度.
  * @param  *Call_Ati: modem 信息的回调函数.
  * @retval 返回处理结果.
  */
static BOOL  Modem_StrCmp_Ati(u8 *pRecbuf , u16 nRecLen)
{

    if(Modem_Type == MODEM_TYPE_IDE)
    {
        if(ModLib_AtCmp(pRecbuf,Modem_Quectel) == 0)//移远M系列模块  G网
        {
            Modem_Type         = MODEM_TYPE_Q;
            Modem_ActPinOnDly  = Modem_QActOnDly;
            Modem_ActPinOffDly = Modem_QActOffDly;
        }
        else if(ModLib_AtCmp(pRecbuf,Modem_ZTE) == 0)//中兴系列模块  C网
        {
            Modem_Type         = MODEM_TYPE_ZTE;
            Modem_ActPinOnDly  = Modem_ZTEActOnDly ;
            Modem_ActPinOffDly = Modem_ZTEActOffDly;      
        }
        
    }
    else if(Modem_Type == MODEM_TYPE_Q)
    {
        if(ModLib_AtCmp(pRecbuf,"Revision:") == 0)
        {
            if(ModLib_AtCmp(pRecbuf,"Revision: M35FA") == 0)
            {
                QuectelAtTtsFlag = 1;//支持TTS
            }
            return (__TRUE);
        }
    }
    else if(Modem_Type == MODEM_TYPE_ZTE)
    {
        if(ModLib_AtCmp(pRecbuf,"Revision:") == 0)
        {
            return (__TRUE);
        }
    }
    return (__FALSE);

}

/**
  * @brief  搜索基站分配的IP.
  * @param  *pRecbuf: 指向将要做对比数据的缓存区.
  * @param  nRecLen: 将要做对比数据缓存区的长度.
  * @param  *Call_LocIp: 获取本地IP回调函数.
  * @retval 返回处理结果.
  */
static BOOL  Modem_StrCmp_IpLoc(u8 *pRecbuf , u16 nRecLen,void(*Call_LocIp)(u8 *pCall_Data, u8 Call_Len))
{
    if(Modem_Type == MODEM_TYPE_Q)////////////移远
    {              
        if((*pRecbuf >= 0x30)&&(*pRecbuf <= 0x39))
        {
            Call_LocIp(pRecbuf,nRecLen-2);//提取基站分配的IP
            return (__TRUE);
        }
    }     
    else if(Modem_Type == MODEM_TYPE_ZTE)/////中兴
    {
        if(ModLib_AtCmp(pRecbuf,"+ZIPGETIP:") == 0)
        {
            Call_LocIp(pRecbuf+10,nRecLen-12);
            return (__TRUE);
        }
    } 

    return (__FALSE);
}
/**
  * @brief  对比Modem返回的数据是否与想要的相同？.
  * @param  *pRecbuf: 指向将要做对比数据的缓存区.
  * @param  nRecLen: 将要做对比数据缓存区的长度.
  * @retval 返回处理结果.
  */
static BOOL  Modem_StrCmp_Search(u8 *pRecbuf , u16 nRecLen)
{
    u8 *p;
    u8 addr;
    ////////////////////////////////////////////////////////////////提取信号质量 
    if(ModLib_AtCmp(pRecbuf,"+CSQ: ") == 0)///////////////////////////////+CSQ: 19,0 
    {
        p = pRecbuf + 6;
        modem.csq = atoi((char *)p);

        #ifdef MODEM_DEBUG_H
        if(ModDbInfType == MODDB_BASE)
        {
            MODDB_PRTFBAS("modem->CSQ = %d\r\n",modem.csq);
        }
        #endif
        
        if(modem.csq == 99)
        {
            modem.csq = 0;
        }
        if((modem.csq >= 10)||(modem.state == MODEM_ON))
        {
            return (__TRUE);
        }
    } 
    ///////////////////////////////////////////////////////////////////SIM卡状态
    else if(ModLib_AtCmp(pRecbuf,"+CPIN: READY\r") == 0) 
    { 
        #ifdef MODEM_DEBUG_H
        if(ModDbInfType == MODDB_BASE)
        {
            MODDB_PRTFBAS("modem->SIM = READY\r\n");
        }
        #endif    
        return (__TRUE);    
    }
    ////////////////////////////////////////////////////////////////////中心号码 
    else if(ModLib_AtCmp(pRecbuf,"+CSCA: ") == 0)//+CSCA: "+8613800755500",145
    {
           p = pRecbuf;
        addr = ModLib_AtAddr(p,'"')+1;
          p += addr;               //数据地址
        addr = ModLib_AtAddr(p,'"');//长度
        
        memset(modem.csca,0,sizeof(modem.csca));
        memcpy(modem.csca,p,addr);
        return (__TRUE); 
    }
    ////////////////////////////////////////////////////////////查询GPRS注册情况 
    else if(ModLib_AtCmp(pRecbuf,"+CGREG: ") == 0)//+CGREG: 0,1
    {
        p = pRecbuf+10;
        if((*p == '1')||(*p == '5'))
        {
            return (__TRUE);
        }
    } 
    ////////////////////////////////////////////////////////////查询网络注册情况    
    if(ModLib_AtCmp(pRecbuf,"+CREG:") == 0)
    {
        if(Modem_Type == MODEM_TYPE_Q)//移远 +CREG: 1,1
        {
            p = pRecbuf+9;
        } 
        else if(Modem_Type == MODEM_TYPE_ZTE)//中兴 +CREG:1,1
        {
            p = pRecbuf+8;
        }
        if((*p == '1')||(*p == '5'))
        {
            return (__TRUE);
        }
    }
    return (__FALSE);
}
/**
  * @brief  获取CCID号码
  * @param  *pRecbuf: 指向将要做对比数据的缓存区.
  * @param  nRecLen: 将要做对比数据缓存区的长度.
  * @retval 返回处理结果.
  */
static BOOL  Modem_StrCmp_Ccid(u8 *pRecbuf , u16 nRecLen)
{
  u8 len=0;
  u8 addr=0;
  u8 *p;
  
    if(nRecLen > 5)
    {
        if(Modem_Type == MODEM_TYPE_Q)////////////移远 89860008191249803743
        {              
            p    = pRecbuf;
            len = ModLib_AtLen(p);
        }
        else if(Modem_Type == MODEM_TYPE_ZTE)/////中兴 +GETICCID:0x89860310707551338748
        {
            p    = pRecbuf;
            addr = ModLib_AtAddr(p,'x')+1;
            p   += addr;
            len = ModLib_AtLen(p);
        }        
        memset(modem.ccid,0,sizeof(modem.ccid));
        ModLib_StrByte(p,modem.ccid,len);
        Modem_Port_Call_CCID(modem.ccid,len/2);
    }
    if(ModLib_AtCmp(pRecbuf,"OK") == 0)
    {
        if(ModLib_AtLen(modem.ccid))
        {
            return (__TRUE);
        }
    }
    return (__FALSE);
}
/**
  * @brief  对比Modem返回的数据是否与想要的相同？-移远模块.
  * @param  *pRecbuf: 指向将要做对比数据的缓存区.
  * @param  nRecLen: 将要做对比数据缓存区的长度.
  * @param  *Call_IpStat:(index : soc号
                          *pMode : tcp or ucp
                          *pAddr ：ip地址
                          *pPort ：端口号).
  * @retval 返回处理结果.
  */                             
static BOOL  Modem_StrCmp_IpSta_Q(u8 *pRecbuf , u16 nRecLen)
{
    u8  SockNum;
    u8  *pCmp = pRecbuf;
    u8  tmp;

    //+QISTAT: 0, "TCP", "121.15.212.251", 10002
    //+QISTAT: 1, "", "",
    
    if(ModLib_AtCmp(pCmp,"+QISTAT: ") == 0)
    {
        SockNum = *(pCmp+9)-0x30;//soc号

        if(SockNum < MODEM_SOC_NUM)
        {
            tmp = ModLib_AtLen(pCmp);
            if(tmp == 19)
            {
                Modem_IpSock_STA[SockNum] = MODSOC_IDLE;//该sock空闲
            }
            else if(tmp > 19)
            {
                pCmp += 13;
                if((ModLib_AtCmp(pCmp,"TCP") == 0)||(ModLib_AtCmp(pCmp,"UDP") == 0))
                {
                    if(Modem_IpSock_STA[SockNum] == MODSOC_IDLE)
                    {
                        Modem_IpSock_STA[SockNum] = MODSOC_ONLN;//该sock在线                  
                    }
                    #ifdef MODEM_DEBUG_H
                    if(ModDbInfType == MODDB_BASE)
                    {
                        if(Modem_IpSock_STA[SockNum] == MODSOC_ONLN)
                        {
                            MODDB_PRTFBAS("modem->OLN = %d\r\n",SockNum);
                        }
                    }
                    #endif                  
                }
            }
        }
        if(SockNum == 5)//最后一个soc返回结果
        {
            return (__TRUE);
        }
    }
    return (__FALSE);
}
/**
  * @brief  对比Modem返回的数据是否与想要的相同？-中兴MC8331模块.
  * @param  *pRecbuf: 指向将要做对比数据的缓存区.
  * @param  nRecLen: 将要做对比数据缓存区的长度.
  * @retval 返回处理结果.
  */                             
static BOOL  Modem_StrCmp_IpSta_ZTE(u8 *pRecbuf , u16 nRecLen)
{
    u8  *pCmp = pRecbuf+12;
    u8  flag = 0;

    //+ZIPSTATUS: CLOSED
    //+ZIPSTATUS: ESTABLISHED
    //+ZIPSTATUS: SYN_SENT 
    if(ModLib_AtCmp(pCmp,"CLOSED") == 0)
    {
        Modem_IpSock_STA[ZTE_IpStaNum] = MODSOC_IDLE;
        flag = 1;
    }
    else if(ModLib_AtCmp(pCmp,"ESTABLISHED") == 0)
    {
        if(Modem_IpSock_STA[ZTE_IpStaNum] == MODSOC_IDLE)
        {
            Modem_IpSock_STA[ZTE_IpStaNum] = MODSOC_ONLN;//该sock在线            
        }
        #ifdef MODEM_DEBUG_H
        if(ModDbInfType == MODDB_BASE)
        {
            if(Modem_IpSock_STA[ZTE_IpStaNum] == MODSOC_ONLN)
            {
                MODDB_PRTFBAS("modem->OLN = %d\r\n",ZTE_IpStaNum);
            }
        }
        #endif  
        flag = 1;
    }
    else if(ModLib_AtCmp(pCmp,"SYN_SENT") == 0)
    {
        Modem_IpSock_STA[ZTE_IpStaNum] = MODSOC_DOING;
        flag = 1;
    }
    
    if(flag)
    {
        if((++ZTE_IpStaNum) < MODEM_SOC_NUM)
        {
            modem.index--;
        }
        else 
        {
            ZTE_IpStaNum = 0;
        }
        return (__TRUE);
    }
    return (__FALSE);
}
/**
  * @brief  对比Modem返回的数据是否与想要的相同？-广和通 
  * @param  *pRecbuf: 指向将要做对比数据的缓存区.
  * @param  nRecLen: 将要做对比数据缓存区的长度.
  * @retval 返回处理结果.
  */                             
static BOOL  Modem_StrCmp_IpSta(u8 *pRecbuf , u16 nRecLen)
{
    u8 flag = 0;
    
    if(Modem_Type == MODEM_TYPE_Q)////////////移远
    {              
        if (Modem_StrCmp_IpSta_Q(pRecbuf,nRecLen)  == __TRUE)
        {   
            flag = 1; 
        }
    }                  
    else if(Modem_Type == MODEM_TYPE_ZTE)/////中兴
    {
        if(Modem_StrCmp_IpSta_ZTE(pRecbuf,nRecLen) == __TRUE)
        {
            flag = 1; 
        }    
    } 

    if(flag)
    {
        return (__TRUE); 
    }
    return (__FALSE);
}
/**
  * @brief  对比Modem返回的数据是否与想要的相同？-移远模块.
  * @param  *pRecbuf: 指向将要做对比数据的缓存区.
  * @param  nRecLen: 将要做对比数据缓存区的长度.
  * @retval 返回处理结果.
  */    
static BOOL  Modem_StrCmp_IpOpen(u8 *pRecbuf , u16 nRecLen)
{ 
    if(ModLib_AtCmp(pRecbuf,modat.recbuf) == 0)
    {
        return (__TRUE);
    }
    
    return (__FALSE);
}

/**
  * @brief  对比Modem返回的数据是否与想要的相同？-移远模块.
  * @param  *pRecbuf: 指向将要做对比数据的缓存区.
  * @param  *pLen   : 指向接收的数据长度.
  * @param  *pSoc   : 指向接收的SOC号码.
  * @retval 返回数据地址.
  */ 
u8*  Modem_StrCmp_RecData_Q(u8 *pRecbuf, u16 *pLen, u8 *pSoc)
{  
    u16 len;
    u8  soc;
    u8  *p;
    u8  temp;

    if(ModLib_AtCmp(pRecbuf,"+RECEIVE:") == 0)//+RECEIVE: 0, 20
    {
        p=pRecbuf;
        temp = ModLib_AtAddr(p,',');
        p += temp;

        soc = *(p-1)-0x30;          //soc号
        len = atoi((char*)(p+2));   //长度

        p=pRecbuf;
        temp = ModLib_AtLen(p)+2;
        p += temp;                  //数据地址

        *pSoc = soc;                //soc号
        *pLen = len;                //长度
        
        return p;                   //返回数据地址
    }
    return NULL;
}
/**
  * @brief  对比Modem返回的数据是否与想要的相同？-中兴模块.
  * @param  *pRecbuf: 指向将要做对比数据的缓存区.
  * @param  *pLen   : 指向接收的数据长度.
  * @param  *pSoc   : 指向接收的SOC号码.
  * @retval 返回数据地址.
  */                            
u8*  Modem_StrCmp_RecData_ZTE(u8 *pRecbuf, u16 *pLen, u8 *pSoc)
{
    u16 len;
    u8  soc;
    u8  *p;
    u8  temp;
    
    if(ModLib_AtCmp(pRecbuf,"+ZIPRECV:") == 0)//+ZIPRECV:0,24,
    {
        p=pRecbuf;
        temp = ModLib_AtAddr(p,',');
        p += temp;

        soc = *(p-1)-0x30;          //soc号
        len = atoi((char*)(p+1));   //长度

        p++;
        temp = ModLib_AtAddr(p,',')+1;
        p += temp;                  //数据地址
        
        *pSoc = soc;                //soc号
        *pLen = len;                //长度
        
        return p;                   //返回数据地址
    }
    return NULL;
}

/**
  * @brief  对比Modem返回的数据是否与想要的相同？.
  * @param  *pRecbuf: 指向将要做对比数据的缓存区.
  * @param  nRecLen: 将要做对比数据缓存区的长度.
  * @retval 返回处理结果.
  */                             
static BOOL  Modem_StrCmp_SendData(u8 *pRecbuf , u16 nRecLen)
{
    u8 type = 0;// 1:成功  2:失败 3:错误
    
    if(Modem_Type == MODEM_TYPE_Q)////////////移远
    {
        if(ModLib_AtCmp(pRecbuf,"SEND OK") == 0)
        {
            type = 1;
        }
        else if(ModLib_AtCmp(pRecbuf,"ERROR") == 0)
        {
            type = 3;
        }
    }
    else if(Modem_Type == MODEM_TYPE_ZTE)/////中兴
    {
        if(ModLib_AtCmp(pRecbuf,"+ZIPSEND: -") == 0)
        {
            type = 2;
        }
        else if(ModLib_AtCmp(pRecbuf,"+ZIPSEND:") == 0)
        {
            type = 1;
        }
    }

    if(type == 1)//成功
    {
        Modem_PrtfTo(MODPRTF_DATOK);
        return (__TRUE);
    }
    else if(type == 2)//失败
    {
        Modem_PrtfTo(MODPRTF_DATFA);
    }
    else if(type == 3)//错误
    {
        modem.delay = 0;
        modem.index = MODEM_ATERR;
        Modem_PrtfTo(MODPRTF_DATER);
    }
    return (__FALSE);
}


/**
  * @brief  对比Modem返回的数据是否与想要的相同？.
  * @param  *pRecbuf: 指向将要做对比数据的缓存区.
  * @param  nRecLen: 将要做对比数据缓存区的长度.
  * @retval 返回处理结果.
  */ 
static BOOL  Modem_StrCmp_Ftp_Q(u8 *pRecbuf , u16 nRecLen)
{   
    u16 addr;
    u8  tab[30];
    
    if(modftp.fend)
    {
        modftp.fend = 0;//下载结束
        modem.delay = 0;//立即处理

        addr = ModLib_AtCmpEx(pRecbuf,"\r\n+QFTPGET:",nRecLen);
        modftp.fsize += addr;
        if(addr)
        {
            Modem_Api_CallFtp(pRecbuf,addr);//最后一包
        }
        sprintf((char*)tab,"\r\n+QFTPGET:%d",modftp.fsize);
        if(ModLib_AtCmp(pRecbuf+addr,tab) == 0)
        {
            modem.wait  = MODWT_IDLE;
            modftp.fres = 1;//成功
            Modem_PrtfTo(MODPRTF_FTPOK);
        }
        else
        {
            modftp.fres = 2;//失败
            Modem_PrtfTo(MODPRTF_FTPFA);
        }

        return (__TRUE);
    }
    else if(modftp.busy)
    {
        if(modftp.fres == 0)
        {
            Modem_Api_CallFtp(pRecbuf,512);//整包都是数据
            modftp.fsize += 512;
            modem.delay = (120*MODEM_TSEC);//下载文件时停止一切处理
            return (__TRUE);
        }
    }
    else if(ModLib_AtCmp(pRecbuf,"CONNECT") == 0)
    {
        modftp.busy  = 1;
        modftp.fend  = 0;
        modftp.fres  = 0;
        modftp.fsize = 0;
        return (__TRUE);
    }

    return (__FALSE);
}
/**
  * @brief  对比Modem返回的数据是否与想要的相同？.
  * @param  *pRecbuf: 指向将要做对比数据的缓存区.
  * @param  nRecLen: 将要做对比数据缓存区的长度.
  * @retval 返回处理结果.
  */ 
static BOOL  Modem_StrCmp_Ftp_ZTE(u8 *pRecbuf , u16 nRecLen)
{   
    static u32 atlen;
    u32 reclen;
    u16 temp;
    u8  *p;

    u16  i;
    u16  sector_int;
    u16  sector_rem;
    
    if(ModLib_AtCmp(pRecbuf,"+ZFTPDATA:CLOSED") == 0)
    { 
        modftp.fend = 0;//下载结束
        modem.delay = 0;//立即处理
        Modem_Api_CallFtp(FtpSectorBuf,FtpSectorBufLen);//最后一包
        FtpSectorBufLen = 0;
        if((modftp.fsize)&&(modftp.fsize == atlen))
        {
            modem.wait  = MODWT_IDLE;
            modftp.fres = 1;//成功
            Modem_PrtfTo(MODPRTF_FTPOK);
        }
        else
        {
            modftp.fres = 2;//失败
           Modem_PrtfTo(MODPRTF_FTPFA);
        }
        return (__TRUE);
    }
    else if(ModLib_AtCmp(pRecbuf,"+ZFTPDATA:") == 0)//+ZFTPDATA:1024,1024\r\n
    {
        ////////////////////////////////////////////////////////////提取累计长度
        p = pRecbuf;
        temp = ModLib_AtAddr(p,':')+1;
        p += temp;  
        atlen = atoi((char*)p);
        
        ////////////////////////////////////////////////////////////////实际长度
        p = pRecbuf;
        temp = ModLib_AtLen(p)+2;
        reclen = nRecLen - temp - 2;
        
        //////////////////////////////////////////////////////////实际的累计长度
        modftp.fsize += reclen;
        
        ////////////////////////////////////////////////////////////调用回调函数
        p += temp;//移动到数据地址

        if(FtpSectorBufLen)
        {
            temp = 512-FtpSectorBufLen;
            if(temp > reclen)
            {
                temp = reclen;
            }
            memcpy(FtpSectorBuf+FtpSectorBufLen,p,temp);
            Modem_Api_CallFtp(FtpSectorBuf,512);
            p += temp;
            reclen -= temp;
        }        
        
        sector_int = reclen/512;
        sector_rem = reclen%512;

        for(i=0;i<sector_int;i++,p+=512)//处理整数
        {
            Modem_Api_CallFtp(p,512);
        }

        FtpSectorBufLen = sector_rem;//处理余数
        memcpy(FtpSectorBuf,p,sector_rem);

        modem.delay = (120*MODEM_TSEC);//下载文件时停止一切处理

        return (__TRUE);
        
    }

    return (__FALSE);
}
/**
  * @brief  监听新短信到来短信
  * @param  *pRecbuf: 指向将要做对比数据的缓存区.
  * @param  nRecLen: 将要做对比数据缓存区的长度.
  * @retval 返回处理结果.
  */ 
static BOOL  Modem_StrCmp_Sms(u8 *pRecbuf , u16 nRecLen)
{
    u16 len;
    u8  *p;
    u8  addr;
    static u8 state;
/*
    ////////////////////////////// PDU //////////////////////////////
    AT+CMGR=1

    +CMGR: 0,,24
    0891683108707515F0240D91685163515733F200003121110150412304E832BA0C

    OK
    
    /////////////////////////// 文本模式 ////////////////////////////
    AT+CMGR=1

    +CMGR: "REC UNREAD","+8615361575332","","2013/12/11 14:47:07+32"
    uir

    OK  
    /////////////////////////// 发送短信 ////////////////////////////
    AT+CMGS="1252015814451069"

    > 82D74E9A7FF0003A0020003300320034003300340035
    +CMGS: 57

    OK
    
*/

    if(state == 1)
    {
        state = 0;
        if(modsms.typr == SMS_PDU)//PDU格式进行转义
        {
            memset((u8*)&modsmspdu,0,sizeof(SM_PARAM));
            len = ModLib_PduDecode(pRecbuf,&modsmspdu);        //数据长度
            p   = modsmspdu.TP_UD;                         //数据地址
            strcpy((char*)modsms.num,(char*)modsmspdu.TPA);//复制号码
            if(modsmspdu.PDU_TP == GSM_8BIT)
            {
                modsms.typr = SMS_PDU8;
            }
            else if(modsmspdu.PDU_TP == GSM_7BIT)
            {
                modsms.typr = SMS_PDU7;
            }
            else
            {
                modsms.typr = SMS_ERR;
            }
            if(len == 0)
            {
                return (__TRUE); 
            }

        }
        else if(modsms.typr == SMS_TXT)//文本模式无需修改
        {
            *(pRecbuf+nRecLen-2)=0;
            len = nRecLen-2;
            p   = pRecbuf;        
        }
        #ifdef MODEM_DEBUG_H
        if(ModDbInfType == MODDB_BASE)
        {
            MODDB_PRTFBAS("modem-> SMS=%s, Type=%d, Rec_Len=%d\r\n",modsms.num,modsms.typr,len);
            MODDB_PRTFBAS("%s\r\n",p);
            MODDB_PRTFBAS("modem-> SMS End...\r\n\r\n");            
        }
        #endif 
        Modem_Api_CallSms(modsms.typr,p,len);
        return (__TRUE);  
    }
    else if(ModLib_AtCmp(pRecbuf,"OK\r") == 0)//发送成功判断
    {
        return (__TRUE); 
    }
    else if(ModLib_AtCmp(pRecbuf,"+CMGR: ") == 0)
    {
        state = 1;
        if(modsms.typr == SMS_TXT)//文本模式提取短信头信息
        {
            p    = pRecbuf;
            addr = ModLib_AtAddr(p,',')+2;
            p   += addr;
            pRecbuf = p;//号码地址
            addr = ModLib_AtAddr(p,',')-1;
            *(p+addr) = 0;
            strcpy((char*)modsms.num,(char*)pRecbuf);
        }
    }
    return (__FALSE);
}


/**
  * @brief  对比拨打电话后返回命令是否符合要求
  * @param  *pRecbuf: 指向将要做对比数据的缓存区.
  * @param  nRecLen: 将要做对比数据缓存区的长度.
  * @retval 返回处理结果.
  */ 
static BOOL  Modem_StrCmp_Atd(u8 *pRecbuf , u16 nRecLen)
{
    if(Modem_Type == MODEM_TYPE_Q)////////////移远
    {              
        if(ModLib_AtCmp(pRecbuf,"+COLP:") == 0)
        {
            return (__TRUE);
        }
    }  
    else if(Modem_Type == MODEM_TYPE_ZTE)/////中兴
    {
        if(ModLib_AtCmp(pRecbuf,"+ZCANS:") == 0)
        {
            return (__TRUE);
        }
    } 
    return (__FALSE);
}
/**
  * @brief  监听socket连接状态,主要是打开和被动关闭
  * @param  *pRecbuf: 指向将要做对比数据的缓存区.
  * @param  nRecLen: 将要做对比数据缓存区的长度.
  * @retval 返回处理结果.
  */ 
static BOOL  Modem_StrCmp_LisCon(u8 *pRecbuf , u16 nRecLen)
{
    u8  soc=0;
    u8  type=0;// 1:打开  2:连接失败 3:断开  
    
    if (Modem_Type == MODEM_TYPE_Q)//处理接收的函数
    {   
        if(ModLib_AtCmp(pRecbuf+1,", CONNECT OK") == 0)//0, CONNECT OK
        {
            soc = *pRecbuf - 0x30;
            type = 1;
        }
        else if(ModLib_AtCmp(pRecbuf+1,", CONNECT FAIL") == 0)//0, CONNECT FAIL
        {
            soc = *pRecbuf - 0x30;
            type = 2;
        }        
        else if(ModLib_AtCmp(pRecbuf+1,", CLOSED") == 0)//0, CLOSED
        {
            soc = *pRecbuf - 0x30;
            type = 3;
        }
    }
    else if(Modem_Type == MODEM_TYPE_ZTE)
    {
        if(ModLib_AtCmp(pRecbuf,"+ZTCPESTABLISHED:") == 0)//+ZTCPESTABLISHED:0
        {
            soc = (*(pRecbuf+17) - 0x30);
            type = 1;
        }
        else if(ModLib_AtCmp(pRecbuf,"+ZTCPCLOSED:") == 0)//+ZTCPCLOSED:0
        {
            soc = *(pRecbuf+12) - 0x30;
            type = 3;
            if(modem.onsta == MODON_IPCLOSE)
            {
                type = 0;//交给后面处理
            }
        }
    }
    
    if(type == 0)
    {
        return (__FALSE); 
    }

    if(type == 1)//主动打开
    {
        Modem_IpSock_STA[soc] = MODSOC_ONLN;      
    }
    else if(type == 2)//连接失败 
    {
        Modem_IpSock_STA[soc] = MODSOC_ERR;
    }
    else if(type == 3)//被动关闭
    {
        if(Modem_IpSock_STA[soc] == MODSOC_ONLN)
        {
            Modem_IpSock_STA[soc] = MODSOC_IDLE;//从上线到下线
            #ifdef MODEM_DEBUG_H
            if(ModDbInfType == MODDB_BASE)
            {
                MODDB_PRTFBAS("modem->Soc=%d Closed\r\n",soc);
            }
            #endif              
        }
        else
        {
            Modem_IpSock_STA[soc] = MODSOC_ERR;//打开连接时出现错误
        }       
    }
    return (__TRUE); 
}
/**
  * @brief  监听socket链路
  * @param  *pRecbuf: 指向将要做对比数据的缓存区.
  * @param  nRecLen: 将要做对比数据缓存区的长度.
  * @retval 返回处理结果.
  */ 
static BOOL  Modem_StrCmp_LisRec(u8 *pRecbuf , u16 nRecLen)
{
    u8  *p;
    u16 len;
    u8  soc;

    p   = pRecbuf;
    len = 0;
    
    if (Modem_Type == MODEM_TYPE_Q)//移远 
    {   
        p = Modem_StrCmp_RecData_Q(p,&len,&soc);
    }
    else if(Modem_Type == MODEM_TYPE_ZTE)//中兴
    {
        p = Modem_StrCmp_RecData_ZTE(p,&len,&soc);
    }
    if(len)
    {
        soc_rec_len += len;
        modInf.Tim_SocEr = 0;
        modInf.Num_SocRx[soc]++;            //接收条数
        Modem_Call_Soc_Function[soc](p,len);//调用应用程序
        #ifdef MODEM_DEBUG_H
        if(ModDbInfType == MODDB_BASE)
        {
            MODDB_PRTFBAS("modem-> Soc = %d   Rec_Len = %04d\r\n",soc,len);
            if(len > 200)
            {
                len = 200;
            }
            memset(mbuf+(MODEM_AT_SIZE-600),0,600);
            ModLib_ByteStrEx(mbuf+(MODEM_AT_SIZE-600),p,len);
            p = mbuf+(MODEM_AT_SIZE-600);
            MODDB_PRTFBAS("%s",p);
            MODDB_PRTFBAS("\r\n");
            Modem_PrtfTo(MODPRTF_RECOK);
        }
        #endif
        return (__TRUE);
    }
    return (__FALSE);
}
/**
  * @brief  监听FT命令
  * @param  *pRecbuf: 指向将要做对比数据的缓存区.
  * @param  nRecLen: 将要做对比数据缓存区的长度.
  * @retval 返回处理结果.
  */ 
static BOOL  Modem_StrCmp_LisFtp(u8 *pRecbuf , u16 nRecLen)
{
    u8 result = __FALSE;

    if(modftp.state == 0)
    {
        return result;
    }
    
    if (Modem_Type == MODEM_TYPE_Q)//移远 
    {   
        result = Modem_StrCmp_Ftp_Q(pRecbuf,nRecLen);
    }
    else if(Modem_Type == MODEM_TYPE_ZTE)//中兴
    {
        result = Modem_StrCmp_Ftp_ZTE(pRecbuf,nRecLen);
    }
    return result;
}
/**
  * @brief  监听modem数据类命令
  * @param  *pRecbuf: 指向将要做对比数据的缓存区.
  * @param  nRecLen: 将要做对比数据缓存区的长度.
  * @retval 返回处理结果.
  */ 
static BOOL  Modem_StrCmp_ListenData(u8 *pRecbuf , u16 nRecLen)
{
    u8 result = __FALSE;
    
    if((Modem_StrCmp_LisCon(pRecbuf,nRecLen)  == __TRUE))//socket状态监听
    {
        result = __TRUE;
    } 
    else if((Modem_StrCmp_LisRec(pRecbuf,nRecLen)  == __TRUE))
    {
        result = __TRUE;
    }
    return result; 
}
/**
  * @brief  监听新短信到来短信
  * @param  *pRecbuf: 指向将要做对比数据的缓存区.
  * @param  nRecLen: 将要做对比数据缓存区的长度.
  * @retval 返回处理结果.
  */ 
static BOOL  Modem_StrCmp_ListenSms(u8 *pRecbuf , u16 nRecLen)
{
    u8 addr;
    static u8 value;
    
    if(ModLib_AtCmp(pRecbuf,"+CMTI:") == 0)//+CMTI: "SM",2
    {
        addr = ModLib_AtAddr(pRecbuf,',')+1;
        pRecbuf += addr;
        value = atoi((char *)pRecbuf);//短信个数
        
        if(Modem_Type == MODEM_TYPE_ZTE)
        {
            modsms.typr = SMS_TXT;
        }
        else
        {
            modsms.rec = value;
            modsms.typr = SMS_PDU;
        }
        return (__TRUE);
    }
    else if(ModLib_AtCmp(pRecbuf,"SMS READY") == 0)//中兴专用
    {
        modsms.rec = 1;
        return (__TRUE);
    }
    return (__FALSE);
}
/**
  * @brief  监听来电
  * @param  *pRecbuf: 指向将要做对比数据的缓存区.
  * @param  nRecLen: 将要做对比数据缓存区的长度.
  * @retval 返回处理结果.
  */ 
static BOOL  Modem_StrCmp_ListenPhone(u8 *pRecbuf , u16 nRecLen)
{
    u16 i=0;
    u16 ch=0;
    
    if(ModLib_AtCmp(pRecbuf,"+CLIP:") == 0)
    {
     //+CLIP: "15361575332",161,"",,"",0  移远
     //+CLIP: "15361575332",161,,,"",0    广和通
     //+CLIP:"18938993721",129            中兴
        if(Modem_Type == MODEM_TYPE_Q)////////////移远
        {              
            ch = 8;
        }
        else if(Modem_Type == MODEM_TYPE_ZTE)/////中兴
        {
            ch = 7;
        } 
        for(i=ch;i<nRecLen;i++)
        {
            if(*(pRecbuf+i)=='"')
            {
                break;
            }
        }
        if(i != ch)
        {
            if(modphone.busy == 0)
            {
                memset(modphone.num,0,sizeof(modphone.num));
                memcpy(modphone.num,pRecbuf+ch,i-ch);
                Modem_Bsp_SpkOff(BSP_SPK_TTS);//关闭扬声器  
                Modem_Bsp_MicOff();
                Modem_State_Phone_Enter();                
            }
        }
        #ifdef MODEM_DEBUG_H
        if(ModDbInfType == MODDB_BASE)
        {
            MODDB_PRTFBAS("modem->RING:%s;\r\n",(char*)modphone.num);
        }
        #endif  
        return (__TRUE);
    }
 
    if(modapi.wait != MODON_PHONE)
    {
        return (__FALSE);
    }
    if((modphone.state == MODPH_RING)||(modphone.state == MODPH_IDLE)
      ||(modphone.state == MODPH_ATD)||(modphone.state == MODPH_VTS))
    {
        ch = 0;
        if(ModLib_AtCmp(pRecbuf,"NO CARRIER") == 0)
        {   
            ch = 1;
        }   
        else if(ModLib_AtCmp(pRecbuf,"NO DIALTONE") == 0)
        {   
            ch = 1;
        }         
        else if(ModLib_AtCmp(pRecbuf,"+ZCEND:") == 0)        
        {   
            ch = 1;
        }
        else if(ModLib_AtCmp(pRecbuf,"BUSY") == 0)         
        {   
            ch = 1;
        }         
        if(ch == 1)
        {
            #ifdef MODEM_DEBUG_H
            if(ModDbInfType == MODDB_BASE)
            {
                MODDB_PRTFBAS("modem->电话已断开!!!\r\n\r\n");
            }
            #endif        
            modem.delay   = 0;   
            modem.wait    = MODWT_IDLE; 
            Modem_State_Phone_Break();//退出电话模式  
        }
    }
    return (__FALSE);
}
/**
  * @brief  监听TTS是否播报完毕
  * @param  *pRecbuf: 指向将要做对比数据的缓存区.
  * @param  nRecLen: 将要做对比数据缓存区的长度.
  * @retval 返回处理结果.
  */ 
static BOOL  Modem_StrCmp_ListenTts(u8 *pRecbuf , u16 nRecLen)
{
    u8 cmptab[15]={0};
    u8 cmplen = 0;
    u8 len;
    
    if(Modem_Type == MODEM_TYPE_ZTE)/////中兴 
    {
        cmplen = ModLib_AtCmy(cmptab,"+ZTTS:0");        
    } 
    else if(Modem_Type == MODEM_TYPE_Q)/////移远
    {
        cmplen = ModLib_AtCmy(cmptab,"+QTTS:0");         
    }       
    if(ModLib_AtCmp(pRecbuf,"OK") == 0) 
    {
        modtts.cmdack = 0;
    }
    
    len = strlen((char *)cmptab);
    if(len)
    {
         if(strncmp((char *)pRecbuf,(char *)cmptab,cmplen) == 0)
        {
            modtts.busy = 0;
            #ifdef MODEM_DEBUG_H
            if(ModDbInfType == MODDB_BASE)
            {
                MODDB_PRTFBAS("modem->Tts Play End\r\n\r\n");
            }
            #endif             
            Modem_Bsp_SpkOff(BSP_SPK_TTS);//关闭扬声器  
            return (__TRUE);
        }   
    }
    return (__FALSE);
}

/**
  * @brief  监听模块开机
  * @param  *pRecbuf: 指向将要做对比数据的缓存区.
  * @param  nRecLen: 将要做对比数据缓存区的长度.
  * @retval 返回处理结果.
  */ 
static BOOL  Modem_StrCmp_ListenOn(u8 *pRecbuf , u16 nRecLen)
{
    u8 flag = 0;

    if(modem.state >= MODEM_ATI)//检查开机命令防止开机关机时序颠倒,只在开机前检查
    {
        return (__FALSE);
    }
    
    if(ModLib_AtCmp(pRecbuf,"RDY")==0)//发现移远开机指令
    {      
        Modem_Type         = MODEM_TYPE_Q;
        Modem_ActPinOnDly  = Modem_QActOnDly;
        Modem_ActPinOffDly = Modem_QActOffDly;
        flag = 1;  
    }
    else if(ModLib_AtCmp(pRecbuf,"+ZIND:8")==0)//发现中兴开机指令
    {
        Modem_Type         = MODEM_TYPE_ZTE;
        Modem_ActPinOnDly  = Modem_ZTEActOnDly;
        Modem_ActPinOffDly = Modem_ZTEActOffDly;
        flag = 1; 
    }
    
    if(flag)
    {
        Modem_State_Switch(MODEM_ATI);
        return (__TRUE);
    }

    return (__FALSE);
}

/**
  * @brief  监听模块异常
  * @param  *pRecbuf: 指向将要做对比数据的缓存区.
  * @param  nRecLen: 将要做对比数据缓存区的长度.
  * @retval 返回处理结果.
  */ 
static BOOL  Modem_StrCmp_ListenFalt(u8 *pRecbuf , u16 nRecLen)
{
    u8 flag = 0;

    if(modem.state < MODEM_ATI)//检查异常命令,只在开机后检查
    {
        return (__FALSE);
    }
    
    if(Modem_Type == MODEM_TYPE_Q)////////////移远
    {
        if (ModLib_AtCmp(pRecbuf,"Call Ready")==0)
        {
            if(modem.state > MODEM_ATI)
            {
                Modem_State_Switch(MODEM_SETAUD);//模块准备就绪 dxl,2015.5.13
            }
        }
        else if(ModLib_AtCmp(pRecbuf,"+CPIN: NOT INSERTED")==0)
        {
            Modem_State_Switch(MODEM_TTS);//未插卡转TTS
        } 
        else if(ModLib_AtCmp(pRecbuf,"+CPIN: NOT READY")==0)
        {
            flag = 1;//卡不在位,复位内核
        }
        else if(ModLib_AtCmp(pRecbuf,"+PDP DEACT")==0)
        {
            flag = 1;//模块异常,,复位内核
        }        
    }
    else if(Modem_Type == MODEM_TYPE_ZTE)/////中兴
    {
        if(ModLib_AtCmp(pRecbuf,"+ZIND:0")==0)//检查SIM是否在位
        {
            //卡不在位则复位缺省值modem内核
        }
    }
    if(flag)
    {
        Modem_State_Switch(MODEM_ERROR);//进入错误处理
    }
    return (__FALSE);
}

/**
  * @brief  向modem发送AT指令，包含发送类型超时时间、重发次数.
  * @param  type: 指令等待的状态.
  * @param  *pCmd: 指向要发送的AT指令.
  * @param  *pReply: 指向返回正确的AT指令.
  * @param  Outtime: 指令超时时间,单位s.
  * @param  Try: 指令重发次数.
  * @retval 返回处理结果.
  */
void  Modem_Send_cmd(MODWT_STA type,u8 *pCmd, u8 *pReply,u8 Outtime, u8 Try)
{
    memset(modat.sendbuf,0,sizeof(modat.sendbuf));//清零缓存
    memset(modat.recbuf,0,sizeof(modat.recbuf));  //清零缓存
    strcpy((char*)modat.sendbuf,(char*)pCmd);     //复制发送指令
    strcpy((char*)modat.recbuf,(char*)pReply);    //复制接收指令
    modat.outtime = Outtime*MODEM_TSEC;           //设置超时时间
    if(Try != NULL)
    {
        modat.trynum = Try;                       //设置重发次数
        modat.tryadd = 0;
    }
    modem.wait = type;                            //设置等待状态
    if(pCmd != NULL)
    {
        //Modem_Uart_NodClr();                      //清空上条残留命令 modify joneming
        send_cmd (pCmd);                          //发送指令
    }
    modem.delay = modat.outtime;                  //首次延时时间
}
/**
  * @brief  直接发送AT命令不判断结果
  * @param  *pCmd: 指向要发送的AT指令.
  * @retval none.
  */
void  Modem_Send_cmd_Only(u8 *pCmd)
{
    modem.delay = 0;
    modem.wait  = MODWT_NO; 
    modem.index++;
    send_cmd (pCmd);
}

/**
  * @brief  向modem发送基本AT指令，包含超时时间、重发次数.
  * @param  *pCmd: 指向要发送的AT指令.
  * @param  *pReply: 指向返回正确的AT指令.
  * @param  Outtime: 指令超时时间,单位s.
  * @param  Try: 指令重发次数.
  * @retval none.
  */
void  Modem_Send_cmd_Base(u8 *pCmd, u8 *pReply,u8 Outtime, u8 Try)
{
    Modem_Send_cmd(MODWT_CMD,pCmd,pReply,Outtime,Try);    
}
/**
  * @brief  向modem发送APN设置指令，包含超时时间、重发次数.
  * @param  Outtime: 指令超时时间,单位s.
  * @param  Try: 指令重发次数.
  * @retval None.
  */
void  Modem_Send_cmd_Apn(u8 Outtime, u8 Try)
{   
    u8  *pCmd,*pRec;   
    
    u8 Tab_r[20];
    u8 Tab_t[(sizeof(modem.apn)*3)+20];//G网 : AT+QICSGP=1,"CMNET","user","pass"   
                                       //C网 : AT+ZPIDPWD=card,card
    memset(Tab_t,0,sizeof(Tab_t));
    memset(Tab_r,0,sizeof(Tab_r));
    
    if(Modem_Type == MODEM_TYPE_Q)////////////移远
    {              
        sprintf((char*)Tab_t,"AT+QICSGP=1,\"%s\",\"%s\",\"%s\"\r",modem.apn,modem.user,modem.pass);
        strcpy((char*)Tab_r,"OK\r");
    }    
    else if(Modem_Type == MODEM_TYPE_ZTE)/////中兴
    {
        sprintf((char*)Tab_t,"AT+ZPIDPWD=%s,%s\r",modem.user,modem.pass);
        strcpy((char*)Tab_r,"OK\r");   
    } 

    pCmd = Tab_t;
    pRec = Tab_r;
    
    Modem_Send_cmd_Base(pCmd,pRec,Outtime,Try);
    #ifdef MODEM_DEBUG_H
    if(ModDbInfType == MODDB_BASE)
    {
        MODDB_PRTFBAS("modem->Apn  = %s\r\n",modem.apn);
        MODDB_PRTFBAS("modem->User = %s\r\n",modem.user);     
        MODDB_PRTFBAS("modem->Pass = %s\r\n\r\n",modem.pass);
    }
    #endif        

}
/**
  * @brief  向modem发送ATI命令，查询模块信息.
  * @param  Outtime: 指令超时时间,单位s.
  * @param  Try: 指令重发次数.
  * @retval None
  */
void  Modem_Send_cmd_Ati(u8 Outtime, u8 Try)
{
    Modem_Send_cmd(MODWT_ATI,"ATI\r",NULL,Outtime,Try);  
}
/**
  * @brief  向modem发送ATI命令，查询模块信息.广和通专用
  * @param  Outtime: 指令超时时间,单位s.
  * @param  Try: 指令重发次数.
  * @retval None
  */
void  Modem_Send_cmd_AtiFibocom(u8 Outtime, u8 Try)
{
    Modem_Send_cmd(MODWT_ATI,"ATI8\r",NULL,Outtime,Try);  
}
/**
  * @brief  向modem发送查询本地IP命令.
  * @param  *pCmd: 指向要发送的AT指令，
  * @param  Outtime: 指令超时时间,单位s.
  * @param  Try: 指令重发次数.
  * @retval 返回处理结果.
  */
void  Modem_Send_cmd_IpLoc(u8 *pCmd ,u8 Outtime, u8 Try)
{     
    Modem_Send_cmd(MODWT_LOCIP,pCmd,NULL,Outtime,Try);/*获取基站分配的IP指令处理*/
}



/**
  * @brief  向modem发送搜索类型的AT指令，包含信号质量、短信中心号码、网络注册情况,
            这些信息将被存储到与之对应的接口缓存区中。
  * @param  *pCmd: 指向要发送的AT指令，
            注意：仅支持CSQ、CREG、CGREG、CSCA,其他不支持！！！！！！！
  * @param  Outtime: 指令超时时间,单位s.
  * @param  Try: 指令重发次数.
  * @retval 返回处理结果.
  */
void  Modem_Send_cmd_Search(u8 *pCmd,u8 Outtime, u8 Try)
{
    Modem_Send_cmd(MODWT_SEARCH,pCmd,NULL,Outtime,Try);    
}


/**
  * @brief  查询CCID号码
  * @retval None
  */
void  Modem_Send_cmd_CCID(u8 *pCmd,u8 Outtime, u8 Try)
{
      Modem_Send_cmd(MODWT_CCID,pCmd,NULL,Outtime,Try);  
}
/**
  * @brief  向modem发送,查询当前连接状态指令
  * @param  *pCmd: 指向要发送的AT指令，
  * @param  Outtime: 指令超时时间,单位s.
  * @param  Try: 指令重发次数.
  * @param  nSoc: 查询的soc号，0xff表示全部.
  * @retval 返回处理结果.
  */
void  Modem_Send_cmd_IpSta(u8 Outtime, u8 Try)
{
    u8 *pCmd;
    u8  Tab[20];
    
    if(Modem_Type == MODEM_TYPE_Q)////////////移远
    {              
        pCmd = "AT+QISTAT\r";
    }                  
    else if(Modem_Type == MODEM_TYPE_ZTE)/////中兴
    {
        sprintf((char*)Tab,"AT+ZIPSTATUS=%d\r",ZTE_IpStaNum);
        pCmd = Tab;
    }

    Modem_Send_cmd(MODWT_IPSTAT,pCmd,NULL,Outtime,Try);
    
}
/**
  * @brief  打开指定链路
  * @param  index: 要打开的连接soc号
  * @param  Outtime: 指令超时时间,单位s.
  * @retval None
  */
void  Modem_Send_cmd_IpOpen(u8 index,u8 Outtime) 
{
    u8  *pCmd,*pRec;
    u8  Tab_Mode[5]={0};
    u8  Tab_Addr[30]={0};
    u8  Tab_Port[7]={0};
    u8  Tab_t[50]={0};
    u8  Tab_r[30]={0};
    u8  NodNum;
 
    ////////////////////////////////////////////////////检查是否已有链路成功打开
    NodNum = Modem_Uart_NodNum();
    for(;NodNum;NodNum--)
    {
        mlen = Modem_Uart_NodRet(mbuf);
        Modem_StrCmp_LisCon(mbuf,mlen);
    }
 
    Modem_Call_SocIpConf_Function[index](Tab_Mode,Tab_Addr,Tab_Port);//获取ip地址和端口
    if(Modem_Type == MODEM_TYPE_Q)////////////移远
    {
        //AT+QIOPEN=0,"TCP","121.15.212.251",8666
        sprintf((char*)Tab_t,"AT+QIOPEN=%d,\"%s\",\"%s\",%s\r",
                             index,Tab_Mode,Tab_Addr,Tab_Port);
    }                
    else if(Modem_Type == MODEM_TYPE_ZTE)/////中兴
    {
        if(strncmp((char *)Tab_Mode,"UDP",3) == 0)
        {
            sprintf((char*)Tab_t,"AT+ZIPSETUP=%d,%s,%s\r",
                                index,Tab_Addr,Tab_Port);
        }
        else
        {
            //AT+ZIPSETUP=0,121.15.212.251,8666
            //+ZTCPESTABLISHED:0
            sprintf((char*)Tab_t,"AT+ZIPSETUP=%d,%s,%s\r",
                                index,Tab_Addr,Tab_Port);
        }
    }
    strcpy((char*)Tab_r,"OK\r");
    pRec = Tab_r;
    pCmd = Tab_t;
    Modem_Send_cmd(MODWT_IPOPEN,pCmd,pRec,Outtime,1);//设置发送模式和重试次数
    #ifdef MODEM_DEBUG_H
    if(ModDbInfType == MODDB_BASE)
    {
        MODDB_PRTFBAS("modem->Soc = %d\r\n",index);
        MODDB_PRTFBAS("modem->IP  = %s\r\n",Tab_Addr);    
        MODDB_PRTFBAS("modem->Port= %s\r\n",Tab_Port);
        MODDB_PRTFBAS("modem->Mode= %s\r\n\r\n",Tab_Mode);
    }
    #endif   
   
}
/**
  * @brief  关闭指定soc链路
  * @param  index: 要打开的连接soc号
  * @param  Outtime: 指令超时时间,单位s.
  * @param  Try: 指令重发次数.
  * @retval None
  */
void  Modem_Send_cmd_IpClose(u8 index,u8 Outtime, u8 Try)
{
    u8  *pCmd,*pRec;
    u8  Tab_t[50]={0};
    u8  Tab_r[30]={0};
    
    if(Modem_Type == MODEM_TYPE_Q)////////////移远
    {              
        // 3, CLOSE OK
        sprintf((char*)Tab_t,"AT+QICLOSE=%d\r",index);
        sprintf((char*)Tab_r,"%d, CLOSE OK\r",index);
    }    
    else if(Modem_Type == MODEM_TYPE_ZTE)/////中兴
    {
        sprintf((char*)Tab_t,"AT+ZIPCLOSE=%d\r",index);
        sprintf((char*)Tab_r,"+ZTCPCLOSED:%d\r",index);//+ZTCPCLOSED:0
    } 

    pCmd = Tab_t;
    pRec = Tab_r;
    Modem_Send_cmd_Base(pCmd,pRec,Outtime,Try);
    #ifdef MODEM_DEBUG_H
    if(ModDbInfType == MODDB_BASE)
    {
        MODDB_PRTFBAS("modem->Soc=%d Close Do\r\n",index);
    }
    #endif       
}
/**
  * @brief  发送指定连接数据.
  * @param  SocNum : Soc号.
  * @param   len   : 指向要发送数据的长度.  
  * @param  Outtime: 指令超时时间,单位s
  * @retval None.
  */
void  Modem_Send_cmd_IpData(u8 SocNum, u16 len, u8 Outtime)
{
    u8  *pCmd;
    u8  Tab_t[30];  
    
    if(Modem_Type == MODEM_TYPE_Q)////////////移远
    {
        sprintf((char*)Tab_t,"AT+QISEND=%d,%d\r",SocNum,len);
    }
    else if(Modem_Type == MODEM_TYPE_ZTE)/////中兴
    {
        sprintf((char*)Tab_t,"AT+ZIPSEND=%d,%d\r",SocNum,len);
    }  
    pCmd = Tab_t;
    Modem_Send_cmd_Only(pCmd);//发送AT命令
    modInf.Num_SocTx[SocNum]++;//发送条数
}

/*
  * @brief  登录ftp服务器
  * @param  type: 0-> 用户名  (当中兴时为(IP、端口、用户名、密码))
  *               1-> 密码
  *               2-> IP、端口
  *
  * @param  Outtime: 指令超时时间 
  * @param  Try    : 指令重试次数
  * @retval None
  */
void  Modem_Send_cmd_FtpLogin(u8 type, u8 Outtime, u8 Try)
{
    u8  *pCmd,*pRec;
    u8  Tab_t[60]={0};
    u8  Tab_r[20]={0};
    
    if(Modem_Type == MODEM_TYPE_Q)//移远
    {
        if(type == 0)
        {
            sprintf((char*)Tab_t,"AT+QFTPUSER=\"%s\"\r",modftp.user);
            strcpy((char*)Tab_r,"OK\r");
        }
        else if(type == 1)
        {
            sprintf((char*)Tab_t,"AT+QFTPPASS=\"%s\"\r",modftp.pass);
            strcpy((char*)Tab_r,"OK\r");
        }
        else if(type == 2)
        {
            sprintf((char*)Tab_t,"AT+QFTPOPEN=\"%s\",%s\r",modftp.ip,modftp.port);
            strcpy((char*)Tab_r,"+QFTPOPEN:0\r");
        }
    }
    else if(Modem_Type == MODEM_TYPE_ZTE)//中兴
    {
        if(type == 0)
        {
            sprintf((char*)Tab_t,"AT+ZFTPLOGIN=%s,%s,%s,%s\r",
            modftp.ip,modftp.port,modftp.user,modftp.pass);
            strcpy((char*)Tab_r,"+ZFTPCTL:OPENED\r");
        }
    }
    modftp.state = 1;//登录
    pCmd = Tab_t;
    pRec = Tab_r;
    Modem_Send_cmd_Base(pCmd,pRec,Outtime,Try);
}
/*
  * @brief  ftp服务器下载文件
  * @param  type: 0-> 设置路径  (当中兴时为(路径、文件名))
  *               1-> 设置文件命令
  *
  * @param  Outtime: 指令超时时间 
  * @param  Try    : 指令重试次数
  * @retval None
  */
void  Modem_Send_cmd_FtpDown(u8 type, u8 Outtime, u8 Try)
{   
    u8  *pCmd,*pRec;
    u8  Tab_t[70]={0};
    u8  Tab_r[20]={0};
    
    if(Modem_Type == MODEM_TYPE_Q)//移远
    {
        if(type == 0)
        {
            sprintf((char*)Tab_t,"AT+QFTPPATH=\"%s\"\r",modftp.fpath);
            strcpy((char*)Tab_r,"+QFTPPATH:0\r");  
        }
        else if(type == 1)
        {
            sprintf((char*)Tab_t,"AT+QFTPGET=\"%s\"\r",modftp.fname);
            strcpy((char*)Tab_r,"OK\r");
        }
    }
    else if(Modem_Type == MODEM_TYPE_ZTE)//中兴
    {
        if(type == 0)
        {
            sprintf((char*)Tab_t,"AT+ZFTPDOWN=%s%s,0\r",modftp.fpath,modftp.fname);
            strcpy((char*)Tab_r,"OK\r");  
        }
    }
    modftp.state = 2;//空闲   
    pCmd = Tab_t;
    pRec = Tab_r;
    Modem_Send_cmd_Base(pCmd,pRec,Outtime,Try);
}    
/**
  * @brief  向modem发送短信处理指令,主要是基本指令
  * @param  *pCmd: 指向要发送的AT指令，
  * @param  Outtime: 指令超时时间,单位s.
  * @param  Try: 指令重发次数.
  * @retval 返回处理结果.
  */
void  Modem_Send_cmd_Sms(u8 type ,u8 Outtime, u8 Try)
{  
    u8 *pCmd,*pRec;
    u8  Tab_t[30];
    u8  Tab_r[5]={"OK\r"};
    u8  cmdonly=0;

    ////////////////////////////////////////////////////////////////////设置格式
    if(type == 0)
    {
        strcpy((char*)Tab_t,"AT+CMGF=1\r");//默认文本模式
        if((modsms.send)&&(modsms.typs== SMS_PDU))
        {
            strcpy((char*)Tab_t,"AT+CMGF=0\r");//PDU格式
        }
        else if((modsms.rec)&&(modsms.typr== SMS_PDU))
        {
            strcpy((char*)Tab_t,"AT+CMGF=0\r");//PDU格式
        }
    }
    //////////////////////////////////////////////////////////////////设置字符集
    else if(type == 1)
    {
        if(Modem_Type == MODEM_TYPE_Q)
        {              
            strcpy((char*)Tab_t,"AT+CSCS=\"GSM\"\r");//使用GSM字符集
        }
        else if(Modem_Type == MODEM_TYPE_ZTE)/////中兴
        {
            strcpy((char*)Tab_t,"AT+ZMSGL=1,2\r");//使用GSM字符集
        } 
    }
    ////////////////////////////////////////////////////////////////////发送信息
    else if(type == 2)
    {
        cmdonly = 1;
        if(modsms.typs == SMS_PDU)
        {
            sprintf((char*)Tab_t,"AT+CMGS=%d\r",modsms.pdulen);
        }
        else if(modsms.typs == SMS_TXT)
        {
            sprintf((char*)Tab_t,"AT+CMGS=\"%s\"\r",modsms.num);
        }
    }
    ////////////////////////////////////////////////////////////////////查询短信
    else if(type == 3)
    {
        strcpy((char*)Tab_t,"AT+CPMS?\r");//查询未读条数
    }    
    ////////////////////////////////////////////////////////////////////删除短信
    else if(type == 4)
    {
        strcpy((char*)Tab_t,"AT+CMGD=1,4\r");//查询未读条数
    }  
    ////////////////////////////////////////////////////////////////////////////
    pCmd = Tab_t;
    pRec = Tab_r; 
    if(cmdonly)
    {
        Modem_Send_cmd_Only(pCmd);
        return;
    }
    Modem_Send_cmd_Base(pCmd,pRec,Outtime,Try);

}
/**
  * @brief  发送TTS命令
  * @param  None.
  * @retval None.
  */
void  Modem_Tts_Cmd (u8 *pCmd)
{
    send_cmd(pCmd);
}
/**
  * @brief  Modem tts 处理
  * @param  None.
  * @retval None.
  */
void Modem_Tts_Run (void) 
{
    static u8 kongdebug;

    if(kongdebug)
    {  
#if 1
        kongdebug = 0;
        Modem_Api_Tts_Play(0,"1234深圳市伊爱高新技术开发有限公司",0);
        //Modem_Api_SmsSend(SMS_TXT,"15361575332","1234567",7);
#else
        MODERR_TYPE modemack;
        modemack = Modem_Api_Ftp("113.105.77.214","21","dbftp","0000",\
                                 "/","HB-EGS701GB_BZ-V1.01.32.3U.bin");//HB-EGS701GB_BZ-V1.01.32.3U.bin//ftptest.txt
        //modemack = Modem_Api_Socket_Close(9);
        if(modemack == MOD_OK)
        {                  
            kongdebug = 0;
        }
        else if(modemack == MOD_ER)
        {
            kongdebug = 0;
        }
#endif        
    }
    
    Modem_State_Tts_Pro();
}

/**
  * @brief  Modem character process event handler.
  * @param  None.
  * @retval None.
  */
void Modem_Process (void) 
{
    u8 return_flg=0;

    if((Modem_StrCmp_LisFtp(mbuf,mlen)  == __TRUE))
    {
        memset(mbuf,0,sizeof(mbuf));
        mlen = 0;
        return;
    }   
    /////////////////////////////////////////////////////////  
    if((Modem_StrCmp_ListenOn(mbuf,mlen)  == __TRUE))//监听模块开机
    {
        return_flg = 1;
    }
    /////////////////////////////////////////////////////////  
    if((Modem_StrCmp_ListenFalt(mbuf,mlen)  == __TRUE))//监听模块异常
    {
        return_flg = 1;
    }
    /////////////////////////////////////////////////////////  
    if((Modem_StrCmp_ListenSms(mbuf,mlen)  == __TRUE))//新短信监听
    {
        return_flg = 1;//新信息到达后暂不处理
    }
    /////////////////////////////////////////////////////////  
    if((Modem_StrCmp_ListenPhone(mbuf,mlen)  == __TRUE))//新来电监听
    {
        return_flg = 1;
    } 
    /////////////////////////////////////////////////////////  
    if((Modem_StrCmp_ListenTts(mbuf,mlen)  == __TRUE))//TTS播报完毕监听
    {
        return_flg = 1;
    }     
    /////////////////////////////////////////////////////////  
    if((Modem_StrCmp_ListenData(mbuf,mlen)  == __TRUE))//socket状态监听
    {
        return_flg = 1;
    }

    if(return_flg)
    {
        memset(mbuf,0,sizeof(mbuf));
        mlen = 0;
        return;
    }
    ////////////////////////  start wait_for  //////////////////////////////////    
    if(modem.wait == MODWT_CMD)/* 一般AT指令指令处理 */ 
    {
        if (Modem_StrCmp_Base(mbuf,mlen) == __TRUE)
        {   
            modem.wait  = MODWT_IDLE; 
            modem.delay = 0;
            modem.index++;
        }         
    }
    /////////////////////////////////////////////////////////
    else if(modem.wait == MODWT_SEARCH)/* 搜索指令处理*/
    {
        if (Modem_StrCmp_Search(mbuf,mlen) == __TRUE)
        {   
            modem.wait  = MODWT_IDLE; 
            modem.delay = 0;
            modem.index++;
        } 
    }
    /////////////////////////////////////////////////////////
    else if(modem.wait == MODWT_CCID)
    {
        if (Modem_StrCmp_Ccid(mbuf,mlen) == __TRUE)
        {   
            modem.wait  = MODWT_IDLE; 
            modem.delay = 0;
            modem.index++;
        } 
    }    
    /////////////////////////////////////////////////////////  
    else if (modem.wait == MODWT_ATI)/* 采集Modem信息,包括型号、软件版本等 */ 
    {                               
        if (Modem_StrCmp_Ati(mbuf,mlen)  == __TRUE)
        {   
            modem.wait = MODWT_IDLE; 
            modem.delay = 0;
            modem.index++;
        }
    } 
    /////////////////////////////////////////////////////////  
    else if (modem.wait == MODWT_LOCIP)/* 采集基站分配的IP */ 
    {
        if (Modem_StrCmp_IpLoc(mbuf,mlen,Modem_Port_Call_LocIp)  == __TRUE)
        {   
            modem.wait = MODWT_IDLE; 
            modem.delay = 0;
            modem.index++;
        }
    }   
    ////////////////////////////////////////////////////////////////////////////
    if(modem.state== MODEM_ON)/*在线状态下处理IP指令*/
    {
        //////////////////////////////////////////////////////////
        if (modem.wait == MODWT_IPSTAT)/* 查询当前连接状态 */
        {
            if(Modem_StrCmp_IpSta(mbuf,mlen)  == __TRUE)
            {
                modem.wait = MODWT_IDLE; 
                modem.delay = 0;
                modem.index++;  
            }
        }
        ////////////////////////////////////////////////////////////
        else if(modem.wait == MODWT_IPOPEN)/*打开指定IP连接*/
        {
            if((Modem_StrCmp_IpOpen(mbuf,mlen)  == __TRUE))
            {
                modem.wait = MODWT_IDLE; 
                modem.delay = 0;
                modem.index++;         
            }
        }
        ////////////////////////////////////////////////////////////
        else if ((modem.wait == MODWT_SEDATA)||(modem.wait == MODWT_NO))
        {
            if ((Modem_StrCmp_SendData(mbuf,mlen)  == __TRUE))
            {   
                modem.wait = MODWT_IDLE; 
                modem.delay = 0;
                modem.index++; 
            }      
        }
        ///////////////////////////////////////////////////////////        
        else if (modem.wait == MODWT_SMS)/* 短信处理 */  
        {     
            if (Modem_StrCmp_Sms(mbuf,mlen)  == __TRUE)
            {   
                modem.wait = MODWT_IDLE; 
                modem.delay = 0;
                modem.index++;
            }
        }
       ///////////////////////////////////////////////拨打电话处理，等待对方接通       
        if ((modem.onsta == MODON_PHONE)&&(modphone.state == MODPH_ATD))
        {     
            /* 'modem_run()' is waiting for modem reply */
            if (Modem_StrCmp_Atd(mbuf,mlen)  == __TRUE)
            {   
                modem.wait = MODWT_IDLE; 
                modem.delay = 0;
                modem.index++;
            }
        }
    }    
    ////////////////////////  end wait_for  ////////////////////////////////////
    memset(mbuf,0,sizeof(mbuf));
    mlen = 0;
  /* Modem not connected, return FALSE */
}


/**
  * @brief  modem接收处理，主要判断缓存区是否有要处理的数据
  * @param  None.
  * @retval None.
  */
void Modem_Rec_Handle(void)
{  
    if(Modem_Uart_NodNum())
    {
        for(mlen=1;mlen!=0;Modem_Process())
        {
            modem.atack = 0;
            mlen = Modem_Uart_NodRet(mbuf);
            
        }
    }
}

/*
  * @brief  modem指令重发处理
  * @param  None
  * @retval 返回重试原因
  */
static MODERR_TYPE  Modem_Cmd_Try(void)
{       
    if(modem.wait > MODWT_NO)
    {
        modat.tryadd++;

        ////////////////////////////////////////////////////////////////////////重试处理
        if(modat.tryadd < modat.trynum)
        {
            modem.delay = modat.outtime;//重置超时时间         
            send_cmd(modat.sendbuf);//重新发送数据
            return MOD_TMOT;
        }
        ////////////////////////////////////////////////////////////////////////重试失败
        else
        {
            modem.wait   = MODWT_IDLE;
            modapi.index = MODEM_ATERR;//通知API接口错误
            modem.index  = MODEM_ATERR;//执行指令序号设置为AT命令错误
        }
    }
    return MOD_IDLE;
}

/**
  * @brief  检查modem运行情况,每秒检查一次
  * @param  None.
  * @retval None.
  */
void Modem_Chk_Run(void)
{
    static u32 LastErrorTime = 0;
    static u8 secdly;
    static u8 ResetCount = 0;
    u8 i;
    u32 CurrentErrorTime;

    //////////////////////////////////////////////////////////////////////秒延时
    if((secdly++) < MODEM_TSEC)
    {
        return;
    }
    secdly = 0;
    
    ////////////////////////////////////////////////////////////////////内核计时
    modInf.Tim_Core ++;//内核

    ////////////////////////////////////////////////////////////////////链路异常    
    modInf.Tim_SocEr++;//时间
    if(modInf.Tim_SocEr > 1800)//1800 30分钟
    {
         modInf.Tim_SocEr = 0;
        //Modem_State_Switch(MODEM_ERROR);//进入错误处理
        Communication_Init();//dxl,2015.4.1,出错以后跟上电重启的处理流程一样，避免出现很长时间不上线的问题
        CurrentErrorTime = RTC_GetCounter();
        if((CurrentErrorTime - LastErrorTime) <= 3600)//一个小时内出现1次复位重启计数加1,连续达到10次以后重启主机
        {
            ResetCount++;
            if((ResetCount >= 10)&&(0==Io_ReadStatusBit(STATUS_BIT_ACC)))
            {
                ResetCount = 0;
                NVIC_SystemReset();
            }
        }
        else//有1次时间间隔大于1小时，则计数清0
        {
            ResetCount = 0;
        }
        LastErrorTime = CurrentErrorTime;
    }
    
    ///////////////////////////////////////////////////////////////modem在线计时
    if(modem.state == MODEM_ON) 
    {
        modInf.Tim_Onln++;//上线时间
    }
    else
    {
        modInf.Tim_Dwln++;//下线时间
    }
    //////////////////////////////////////////////////////////////socket在线计时
    for(i=0;i<MODEM_SOC_NUM;i++)
    {
        if(Modem_IpSock_STA[i] == MODSOC_ONLN)
        {
            modInf.Tim_SocOn[i]++;//上线时间
        }
        else
        {
            modInf.Tim_SocDw[i]++;//下线时间
        }
    }


}

/**
  * @brief  This is a main thread for MODEM Control module. It is called on every
            system timer timer tick to implement delays easy.
  * @param  None.
  * @param  None.
  * @param  None.
  * @retval None.
  */
void  Modem_Run (void)
{

    Modem_Uart_NodOut();               /*节点超时处理                         */
    
    Modem_Rec_Handle();                /*处理modem接收的数据                  */

    Modem_Tts_Run();                   /*TTS语音播报处理                      */ 

    Modem_Chk_Run();                   /*检查modem运行情况                    */
    
    #ifdef MODEM_DEBUG_H
    Modem_Debug_Run();                 /*modem调试处理函数                    */ 
    #endif

    if (modem.delay)                   /*延时处理                             */ 
    {
        if (--modem.delay) 
        {
            return;
        }
    }

    if(Modem_Cmd_Try() != MOD_IDLE)    /*modem指令重发处理                    */
    {
        return;
    }

    Modem_State_CheckRun();            /*Modem状态机检查                      */
    
    switch (modem.state)               /*状态机处理                           */ 
    {
        ////////////////////////////////////////////////////////////////模块空闲
        case MODEM_IDLE:
            {
                ;
            }
            break;
        //////////////////////////////////////////////////////////////硬件初始化
        case MODEM_BSP:
            {
                modem.state = MODEM_INIT;
                Modem_Type  = MODEM_TYPE_IDE;//系统启动默认空闲
                Modem_Uart_Init();
                Modem_Bsp_Init();
                #ifdef MODEM_DEBUG_H
                Modem_Debug_Init();
                #endif                
                Modem_PrtfTo(MODPRTF_BSP);
                Modem_PrtfTo(MODPRTF_STAR);
                #ifdef EYE_MODEM
                Modem_Bsp_SetPower(0);//关闭电源
                Modem_Delay(15);//15秒硬延时
                #endif
            }
            break;     
        //////////////////////////////////////////////////////////////数据初始化
        case MODEM_INIT:
            {
                Modem_State_Init();
            }         
            break;
        ////////////////////////////////////////////////////////////////关闭模块
        case MODEM_OFF:
            {
                Modem_State_Off(); 
            }
            break;
        ////////////////////////////////////////////////////////////////激活模块
        case MODEM_ACT:
            {          
                #if 0
                Modem_Debug_CtrOn();
                Modem_Bsp_SetPower(1);/*打开modem电源*/
                Modem_Bsp_SetActive(0);/*拉低激活引脚*/
                Modem_Bsp_MicOn();
                Modem_Bsp_SpkOn(BSP_SPK_TALK);
                #else
                Modem_State_Act(MODEM_ATI,MODEM_ERROR); 
                #endif
            }
            break;
        ////////////////////////////////////////////////////////////////查询属性
        case MODEM_ATI:
            {
                Modem_State_Ati(MODEM_SETAUD,MODEM_ERROR); 
            }
            break; 
        ////////////////////////////////////////////////////////////////设置音频          
        case MODEM_SETAUD:
            {
                Modem_State_Aud(MODEM_NETS,MODEM_ERROR);
            }
            break; 
        ////////////////////////////////////////////////////////////////查询网络             
         case MODEM_NETS:
            {
                Modem_State_NetSer(MODEM_SMS,MODEM_ERROR);
            }
            break;
        ////////////////////////////////////////////////////////////////短信处理  
        case MODEM_SMS:
            {
                Modem_State_Sms(MODEM_NETC,MODEM_ERROR);
            }
            break;                 
        ////////////////////////////////////////////////////////////////配置网络            
        case MODEM_NETC:
            {
                Modem_State_NetCfg(MODEM_DIAL,MODEM_ERROR);
            }
            break;  
        /////////////////////////////////////////////////////////////////PPP拨号
        case MODEM_DIAL:
            {
                Modem_State_Dial(MODEM_ON,MODEM_ERROR);
            }
            break; 
        /////////////////////////////////////////////////////////////////TTS处理            
        case MODEM_TTS:
            {                
                Modem_State_Tts(MODEM_ERROR,MODEM_ERROR);
            }
        break;    
        //////////////////////////////////////////////////////在线处理(长期在此)        
        case MODEM_ON:
            {                
                Modem_State_OnLine();
            }
        break;
        ////////////////////////////////////////////////////////////////模块异常
        case MODEM_ERROR:
            {
                Modem_State_Err(MODEM_OFF);
            }
            break;
        case MODEM_SYSRST:///////////////////////////////////////////////////系统复位
            {
                NVIC_SystemReset();//系统复位
            }
            break;            
    }
    ///////////////////////////////////////////////////////////////////////////

    /////////////////////////////////  end   //////////////////////////////////


}




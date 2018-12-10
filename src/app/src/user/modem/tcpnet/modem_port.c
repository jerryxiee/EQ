/**
  ******************************************************************************
  * @file    modem_prot.c 
  * @author  miaoyahan
  * @version V1.0.0
  * @date    2013-08-21
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
*                                       LOCAL GLOBAL VARIABLES   本地变量
*********************************************************************************************************
*/




/*
*********************************************************************************************************
*                                          GLOBAL VARIABLES    全局变量
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

/*
  * @brief  modem接口函数初始化,modem初始化时被调用
  * @param  None
  * @retval None
  */
void Modem_Port_Init(void)
{
#ifdef EYE_MODEM
    Public_SaveCurSIMICcidNumberToE2Param();
#endif
}

/**
  * @brief  modem信息回调函数.
  * @param  *p  :指向modem返回的信息数据.
  * @param  len :返回modem信息长度.
  * @retval None
  */
void Modem_Port_Call_Ati (u8 *p, u16 len) 
{

}

/**
  * @brief  查询基站分配的本地IP地址回调函数.
  * @param  *p  :指向本地IP地址数据.
  * @param  len :IP地址长度.
  * @retval None
  */
void Modem_Port_Call_LocIp (u8 *p, u8 len) 
{
}
/**
  * @brief  查询CCID号码回调函数.
  * @param  *p  :指向地址数据.
  * @param  len :IP地址长度.
  * @retval None
  */
void Modem_Port_Call_CCID (u8 *p, u8 len) 
{
#ifdef EYE_MODEM
    Public_SaveCurSIMICcidNumberToE2Param();
#endif
}
/**
  * @brief  查询指定sock连接回调函数.
  * @param  index  :连接序号
  * @param  *pMode :指向连接类型， “TCP”或“UDP”
  * @param  *pAddr :远程连接或接入IP 地址
  * @param  port   :远程连接或接入端口号 
  * @retval None
  */
void Modem_Port_Call_Ip_STAT (u8 *pIndex,u8 *pMode ,u8 *pAddr, u8 *pPort) 
{
#if 0
    printf("num  : %s\r\n",pIndex);
    printf("type : %s\r\n",pMode);
    printf("ip addr :%s\r\n",pAddr);
    printf("port : %s\r\n",pPort);
#endif
}
/**
  * @brief  读取休眠时间,主要用于CDMA模块
  * @param  None.
  * @retval None.
  */
void Modem_Port_ReadSleep (void) 
{

    modem.sleep = 0;//默认关闭休眠
    
#ifdef EYE_MODEM
    u8 tab[5];
    
    EepromPram_ReadPram(E2_CDMA_SLEEP_ID, tab);
    if(tab[0] == 1)
    {
        modem.sleep = 8;//8秒休眠时间
    }
#endif

}
/**
  * @brief  从存储器中读取语音参数,主要是初始化音频参数
  * @param  None.
  * @retval None.
  */
void Modem_Port_ReadAud (void) 
{ 
#ifdef EYE_MODEM
    u8 value=0;
    
    FRAM_BufferRead((u8*)&value,FRAM_TEL_VOLUME__LEN,FRAM_TEL_VOLUME_ADDR);
    if((value > 0)&&(value < 10))
    {
        modphone.clvl = value;
    }
    else
    {
        value = 6;//参数异常使用默认6级别 
        modphone.clvl = value;
        FRAM_BufferWrite(FRAM_TEL_VOLUME_ADDR,(u8*)&value,FRAM_TEL_VOLUME__LEN);
    }
    
    FRAM_BufferRead((u8*)&value,FRAM_TTS_VOLUME_LEN,FRAM_TTS_VOLUME_ADDR);
    if((value > 0)&&(value < 10))
    {
        modtts.clvl = value;
    }
    else
    {
        value = 4;//参数异常使用默认4级别   
        //modphone.clvl = value;  //dxl,2015.5.14  
        modtts.clvl = value;
        FRAM_BufferWrite(FRAM_TTS_VOLUME_ADDR,(u8*)&value,FRAM_TTS_VOLUME_LEN);
    }    
#else
    modphone.clvl = 6;//默认6级别
    modtts.clvl   = 4;
#endif    
}

/**
  * @brief  从存储器中读取apn
  * @param  None.
  * @retval None.
  */
void Modem_Port_ReadApn (void) 
{
#ifdef EYE_MODEM 
    u8 tab[30]={0};
    u8 len=0;
 
    memset(modem.apn,0,sizeof(modem.apn));  
    strcpy((char*)modem.apn,"cmnet");//G网默认公网

    len = EepromPram_ReadPram(E2_MAIN_SERVER_APN_ID, tab);
    len = strlen((char*)tab);
    if ((len >0)&&(len <= 20)) 
    {
        memset(modem.apn,0,sizeof(modem.apn));
        memcpy(modem.apn,tab,len);
    }
#endif
}

/**
  * @brief  从存储器中读取账号密码
  * @param  None.
  * @retval None.
  */
void Modem_Port_ReadUserPass (void) 
{
#ifdef EYE_MODEM  
    u8 tab[30]={0};
    u8 len=0;
    
    len = EepromPram_ReadPram(E2_MAIN_SERVER_DIAL_NAME_ID,tab);
    len = strlen((char*)tab);
    if ((len >0)&&(len <= 20)) 
    {
        memset(modem.user,0,sizeof(modem.user));
        memcpy(modem.user,tab,len);
    }
  
    len = EepromPram_ReadPram(E2_MAIN_SERVER_DIAL_PASSWORD_ID,tab);
    len = strlen((char*)tab);
    if ((len >0)&&(len <= 20)) 
    {
        memset(modem.pass,0,sizeof(modem.pass));
        memcpy(modem.pass,tab,len);
    }
#endif
}

/**
  * @brief  设置modeme的apn对外接口.
  * @param  *p  : 指向APN数据.
  * @param  len : APN的长度.
  * @retval 返回设置结果，成功返回1，否则返回0.
  */
u8 Modem_Port_Apn_Set (u8 *p, u8 len) 
{
    if(len < sizeof(modem.apn))
    {
        memcpy(modem.apn,p,len);
        return 1;
    } 
    return 0;
}
/**
  * @brief  查询modem信号质量对外接口.
  * @param  None.
  * @retval 返回Modem的信号质量.
  */
u8 Modem_Port_Csq (void) 
{
    return (modem.csq);
}


    


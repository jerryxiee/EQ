/**
  ******************************************************************************
  * @file    modem_api.h
  * @author  miaoyahan
  * @version V1.0.0
  * @date    2013-08-25 
  * @brief   
  ******************************************************************************
  * @attention
  ******************************************************************************
  */ 
#ifndef __MODEM_API_H
#define __MODEM_API_H

/*
*********************************************************************************************************
*                                               DEFINES
*********************************************************************************************************
*/

typedef enum
{
  MODEM_APP_IDLE = (0x00),
  MODEM_APP_STAT ,
  MODEM_APP_OPEN ,
  MODEM_APP_LISTEN,  
  MODEM_APP_CLOSE ,
  MODEM_APP_DOWN,
  MODEM_APP_SEND ,
  MODEM_APP_FTP ,
  MODEM_APP_DELAY,
  MODEM_APP_MODEMON , 
  MODEM_APP_SYSREST ,  
  MODEM_APP_ERR ,

} Modem_APP_STATE;//应用层应用状态机,主要用来维护一个连接


typedef enum
{
    PORT_OPEN=0,//Modem_Api_Socket_Open
    PORT_CLOSE,//Modem_Api_Socket_Close
    PORT_FTP,//Modem_Api_Ftp

} Modem_API_PORT;//API接口

typedef struct STRUCT_MODEM_API
{		   
    MODON_STA  wait;//等待结果
    u8  index;		//执行指令序号
}ST_MOD_API;//modem模块api结构体

typedef  void (*CALL_SOC_FUNCTION) (u8 *p, u16 len);//socket回调函数指针
typedef  void (*CALL_SET_IP_FUNCTION)(u8 *pMode ,u8 *pAddr, u8 *pPort);//配置socket申请后的IP信息

/*
*********************************************************************************************************
*                                          GLOBAL VARIABLES
*********************************************************************************************************
*/ 
extern ST_MOD_API   modapi;//modem模块api结构体
extern CALL_SOC_FUNCTION  Modem_Call_Soc_Function[];//soc回调函数数组
extern CALL_SET_IP_FUNCTION  Modem_Call_SocIpConf_Function[];//soc ip回调，用于配置socket申请后的IP信息


/*
*********************************************************************************************************
*                                            FUNCTION PROTOTYPES
*********************************************************************************************************
*/
void  Modem_Api_Start(void);
void  Modem_Api_Rst (void);
void  Modem_Api_Off (u16 delay);
void  Modem_Api_On (void);

void  Modem_Api_Init (void);
void  Modem_Api_CallFtp(u8 *p, u16 len);
void  Modem_Api_CCID(u8 *p);  
void  Modem_Api_CallSms(MODSMS_TYPE type, u8 *p, u16 len);
void  Modem_Api_CallRing(u8 *p, u16 len);
u8    Modem_Api_CallPhoneStatus(u8 *pPhone,u8 status);

BOOL  Modem_Api_SocSta(u8 soc);
BOOL  Modem_Api_Phone_Atd(u8 *pData, u8 TypeSpeak);
BOOL  Modem_Api_Phone_Ath(void);
BOOL  Modem_Api_Phone_Ata(void);
BOOL  Modem_Api_Phone_Clvl(u8 value);
BOOL  Modem_Api_Phone_Audch(u8 channel);
BOOL  Modem_Api_Phone_Vts(u8 *p);
u8    Modem_Api_Phone_ClvlGet(void);


BOOL  Modem_Api_SmsSend(MODSMS_TYPE Type,u8 *pNum,u8 *pData, u8 len);

u8  Modem_Api_Socket_SendIsBusy(u8 soc);
u16 Modem_Api_Socket_Send(u8 SocNum ,u8 *data, u16 len);
u32 Modem_Api_FtpSize(void);
u8  Modem_Api_FtpResult(void);


u8  Modem_Api_Tts_IsSta(void);
u8  Modem_Api_Tts_IsSpt(void);
u8  Modem_Api_Tts_IsClvl(void);
u8  Modem_Api_Tts_Clvl(u8 value);
u8  Modem_Api_Tts_Stop(void);
u8  Modem_Api_Tts_Play(u8 type,u8 *pTts, u8 lentts);

u8  Modem_Api_Type(void);
MODON_STA  Modem_Api_OnSta(void);

MODERR_TYPE  Modem_Api_Socket_Close(u8 SocNum);
MODERR_TYPE  Modem_Api_Socket_Open(u8 SocNum
                                            ,void(*Call_IpConf)(u8 *pMode ,u8 *pAddr, u8 *pPort)
                                            ,void(*Call_Soc)(u8 *pRec, u16 Reclen));

MODERR_TYPE  Modem_Api_Ftp(u8 *pIp,u8 *pPort
                                             ,u8 *pUser,u8 *pPass
                                             ,u8 *pFpath,u8 *pFname);  

                                          

void Eye_RunNet_Main(void);                                           
#endif

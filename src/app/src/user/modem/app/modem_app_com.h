/*
********************************************************************************
*
*                                        
*                          
*
*                        
*
* Filename      : modem_app_com.h
* Version       : V1.00
* Programmer(s) : miaoyahan
* @date         : 2014-10-24 
********************************************************************************
*/

/*
********************************************************************************
*                               MODULE
*
* Note(s) : 
********************************************************************************
*/

#ifndef MODEM_APP_COM_H
#define MODEM_APP_COM_H


/*
********************************************************************************
*                               INCLUDES
********************************************************************************
*/


/*
********************************************************************************
*                               DEFINES
********************************************************************************
*/
#ifndef true
#define true                       1
#endif

#ifndef false
#define false                      0
#endif

#ifndef bool
#define bool unsigned char
#endif

#define ICAUC_TIME_TICK            1*SYSTICK_0p1SECOND//连接维护任务执行时间

#define  UPDATE_START_SECTOR       FLASH_UPDATE_START_SECTOR
#define  UPDATE_BASE_ADDRESS       (UPDATE_START_SECTOR * SECTOR_SIZE)

#define  SECTOR_SIZE  		       0x1000ul //扇区字节4096	
#define  SPI_FLASH_PageSize        0x100	//文件页大小512字节



/*
********************************************************************************
*                                 ENUM
********************************************************************************
*/
typedef enum {
    CHANNEL_SMS    = 0x01,//短信通道,0值保留
    CHANNEL_DATA_1 = 0x02,//数据通道1
    CHANNEL_DATA_2 = 0x04,//数据通道2
    CHANNEL_DATA_3 = 0x08,//数据通道3 IC卡认证通道
}SND_TYPE;//发送数据的类型

typedef enum {
	PHONE_NORMAL = 0,			//正常拨号
	PHONE_MONITOR,				//监听
	PHONE_TALK,					//接听
	PHONE_END
}CALL_TYPE;//电话类型

typedef enum{
	COMMUNICATE_STATE = 0, 		//通信模块状态，0：GSM；1：GPRS
	SIGN_STRENTH,				//信号强度
}GPRSSTATE;

typedef enum {
    GSM_PHONE_IDLE = 0,   	    //空闲
    GSM_PHONE_CALL_IN,   	    //电话呼入
    GSM_PHONE_TALK,   		    //通话
}GSM_PHONE_STATUS;//电话状态

//函数返回状态值
typedef enum{
    ICAUC_RET_SUCCED = 0,               //返回成功
    ICAUC_RET_FAIL,                     //错误,函数参数自检失败
    ICAUC_RET_CONN_FAIL,                //错误,TCP连接未打开
    ICAUC_RET_OPENLNK_AUTH_FAIL,        //错误,打开连接失败,未获得鉴权。
    ICAUC_RET_OPENLNK_RE,               //错误,不能重复打开,上次连接正在执行。
    ICAUC_RET_SENDBUF_BUSY,             //错误,数据发送缓冲区忙,上次发送可能还未完成.
    ICAUC_RET_SENDBUF_NO_CMD,           //错误,模块不在待发送状态,可能在打电话或其他。
    
      
}ICAUC_RetType;

typedef enum
{
  DOWN_IPOP,//当前IP
  DOWN_IPMN,//主IP  
  DOWN_IPSW,//切换IP
  DOWN_IPTO,//指定IP

}APP_TYPE_DOWN;//下线重连类型

typedef enum
{
  VAL_IPMAIN=0,//主IP
  VAL_IPBACK,//备份IP
  VAL_IPTO,//指定IP
  VAL_HEART,//心跳 

}APP_TYPE_VAL;//参数类型

/*
********************************************************************************
*                                STRUCT
********************************************************************************
*/
typedef struct
{
    u8 status;
    u8 telephone[23];      //呼叫电话号码
}CALL_RECORD;//电话相关结构体



typedef struct
{
    SM_PARAM msgParam;	   // 信息参数设置	
}SMS_RECORD;//短信操作相关


typedef struct {
	u8	rec_chanel;		   // 1 : GPRS;0:短信

}UART2_RECEIVE;


typedef  void (*ICAUC_FUN_READ)(u8*,u16);  //定义接收函数指针
/*
********************************************************************************
*                          EXTERN  VALUE
********************************************************************************
*/
extern  CALL_RECORD     gPhone;         //通话相关
extern  SMS_RECORD      gSmsRecord;     //短信相关
extern  UART2_RECEIVE	uart2_recdat;   //串口2接收

/*
********************************************************************************
*                               FUNCTIONS
********************************************************************************
*/
void GSM_PWR_ON(void);
void GSM_PWR_OFF(void);


FunctionalState Communication_TimeTask(void);
FunctionalState Communication_CloseGprsTimeTask(void);
void Communication_TaskSet(u8 state);
void Communication_Init(void); 
u8   communication_CallPhone (u8 *pCode, CALL_TYPE type);
u16  Communication_SndMsg2Svr(u8 channel, u8 * srcdat, u16 srclen, u8 *phone);
u16  communicatio_GetMudulState(GPRSSTATE type);
u8   communicatio_GetSignalIntensity(void);
u8   communicatio_NetState(u8 channel);


void Communication_Close(void);
void Communication_Open(void);
void Communication_Close2(void);



void MOD_InitCRC(void);
void MOD_StopCRC(void);
u32  MOD_CalcCRC(u8 *pBuffer, u32 BufferLength);

void MOD_CloseMainIp(void);
void MOD_GetIccidBcdValue(u8 *Bcdbuff);


void SetTelVolumeIncr(void);
void SetTelVolumeDecr(void);
u8   GetTelVolume(void);

void SetModTtsVolumeIncr(void);
void SetModTtsVolumeDecr(void);
u8   GetModTtsVolume(void);

FunctionalState ICAUC_TimeTask(void);
ICAUC_RetType   ICAUC_OpenLnk(void);
ICAUC_RetType   ICAUC_SendBuff(u8* buff,u16 len);
void ICAUC_RegReadFun(ICAUC_FUN_READ pFun);
u8   ICAUC_GetLnkStatus(void);


void communication_CalTimer(void);


#endif


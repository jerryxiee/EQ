/*************************************************************************
* Copyright (c) 2013,深圳市伊爱高新技术开发有限公司
* All rights reserved.
* 文件名称 : Module_IC_Card_JTB.c
* 当前版本 : 1.0
* 开发者   : Shigle
* 修改时间 : 2013年10月17日
* 2013年10月17日为基线版本1.0, 后续功能如有增减或修改，将要求以下追加说明
* 1、本版同时支持雄帝或成为交通部规定的驾驶员卡
* 2、本次上电，所有存储和全局变量全清零，一次全新的IC卡操作，与历史操作无关 
* 修改时间 : 2017.6.5
* 修改者   : ZWB
* 支持广州二汽交运卡

1,		参数设置
主服务器APN:gzjtxx03.gd
监控中心服务器IP:10.190.10.110		port:9205  二汽专用卡必须配置这个，连我们平台是连接不上的
IC卡认证服务器IP:192.168.201.95	port:11000		主要用于交运卡认证
外设2类型:bit15设为1,即配置成0x8000	 二汽版本不配置也成

2,		串口辅助命令
使用串口助手发送"card_clr"可以清除读卡器信息,这时需要重新走认证流程
使用串口助手发送"card_ent_pwr_save"卡进入省电模式，没啥屌用
使用串口助手发送"card_esc_pwr_save"卡退出省电模式，没啥屌用

3,		功能
acc on状态下,1分钟间隔向读卡器发送链路探询,连续3次无应答置位"道路运输证 IC卡模块故障"报警位,收到应答报警解除

acc off状态下,读卡器进入省电模式,4分钟后关闭读卡器电源,acc on退出省电模式,打开读卡器电源

插入IC卡打卡上班签到,拔出IC卡下班签退.
如果IC卡插入状态下断电则下次上电并不签到(相当于驾驶员仍在上班),直到拔卡签退.有插入,有拔出才算一个完整的上下班过程

4,		终端固件版本号:11708

认证流程
1,上电指示---->命令类型(01)--->应答就对了，不重要;

2,插卡----->命令类型(40)--->状态位为0时，后面跟随64字节的认证信息,很重要;

3,0900上报认证服务器--->认证服务器返回认证信息--->后面跟随状态位(貌似不重要)+24字节(认证信息)，很重要;

4,认证结果(即状态位,说不重要是不可能的)为0x01时才是正确应答,这时终端向读卡器模块返回数据25字节(0x00+认证信息),必须是0x00才可以触发41命令读卡,很重要

5,拔卡，触发42命令，签退，没啥好说的

6,认证成功后下次插卡就不用走认证流程了，如果要完整测试的话，可以使用串口助手发送"card_clr"可以清除读卡器信息,这时需要重新走认证流程

7，刷卡后可以打电话?137 1083 8320?给王润昌，他后台可以看到验证是否成功



*************************************************************************/


//***************包含文件*****************
#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "include.h"

#define LEN_AUTH_REQUEST_CODE 64
#define LEN_AUTH_ACK_CODE	  24+1//认证状态位+认证信息
#define MAX_ICCARD_BUF	 256

extern TIME_T	CurTime;
extern u8	TerminalAuthorizationFlag;//终端鉴权标志,bit0:连接1;bit1:连接2;bit2~bit7保留;

unsigned char ICCard_SendCmdForReadModuleAttribute(u8	cmdType,u8 dataType);
unsigned char ICCard_SendCmdForReadElectronicCertificate(u8	cmdType,u8 dataType);
unsigned char ICCard_SendCmdForClearElectronicCertificate(u8	cmdType,u8 dataType);
void 					ICCard_Order_Process(void);
void 					ICCard_Process_CMD_0x41_Result_0x00( void );
void					ICCARD_Report_TerminalReadCardAttributeInfoAck(void);
void				 	ICCARD_Report_TerminalReadCardClrInbuiltInfoAck(void);
void 					ICCard_Send_CMD_To_Card_Reader_0x01( void );
void 					ICCard_Send_CMD_To_Card_Reader_0x02( void );
// 使用二汽交运卡时，读取认证服务器参数
void ICCARD_ReadCertSevicePara(void);

//模块认证返回属性信息  24bytes
#pragma pack(1)
typedef struct
{
  //u8	peripheralType;				//外设类型  0x0b 读卡模块
  u8	authPlatformCalStatus;//认证平台计算状态 00:正常 01:异常
  u8	authArea;							//认证区域 	01:从业人员资格证主应用 
  //				02:从业人员资格证主扩展应用
  //				03:道路运输证主扩展应用
  //				04:道路运输证主扩展应用
  u8	physicsCardNo[5];			//物理卡号
  u8	cardRandomCode[8];		//卡片随机码
  u8	cardAuthCode[8];			//卡片认证码
  u8	placeHolder;					//占位符
}STT_ICCARD_AUTH_BACK_INFO;
//卡模块属性
typedef struct
{
  u8	peripheralManufacturerNo[5];		//外设制造商编号
  u8	peripheralHardVersion[3];				//外设硬件版本号
  u8	peripheralSoftVersion[3];				//外设软件版本号
  u8	manufacturerNo[2];							//厂商编号  BCD
  u8	curFirmwareVersion[4];					//当前固件版本号
  u8	curHardVersion[4];							//当前硬件版本号
  u8	cardModuleMode;									//卡模块 1:单卡 2:双卡 3:三卡
  u8	upgradeFlag;										//升级标识
  u8	upgradeFirmwareVersion[4];			//升级固件版本号
  u8	curUpgradePackNo[2];						//当前升级包号
}STT_ICCARD_MODULE_ATTRIBUTE;
#pragma pack()
#define	ROAD_TRANSPORT_CERT		0x01	//道路运输证
#define	EMPLOYEE_CERT					0x02	//从业人员资格证

typedef enum peripheralCmdType
{
  POWER_ON_INDICATE 					= 0x01,			//上电指示
  LINK_INQUIRE 								= 0x02,			//链路探询
  SLAVE_POWER_CONTROL 				= 0x03,			//从机电源控制
  SLAVE_VERSION_QUERY					= 0x04,			//从机版本查询
  SLAVE_SELF_INSPECTION				= 0x05,			//从机自检
  SLAVE_FIRMWARE_UPDATE				= 0x06,			//从机固件更新
  IC_CARD_READ_ATTRIBUTE			= 0x07,			//读IC卡属性
  IC_CARD_AUTHENTICATION 			= 0x40,			//IC卡认证
  IC_CARD_READ 								= 0x41,			//IC卡读取
  IC_CARD_PULL 								= 0x42,			//IC拔出
  IC_CARD_INITIATIVE_READ 		= 0x43,			//主动触发读IC卡
  IC_CARD_READ_E_CERT					= 0xA1,			//读内置电子证书
  IC_CARD_CLR_E_CERT					= 0xA2,			//清除内置电子证书
  IC_CARD_SET_TIME						= 0xA4,			//设置时间
  IC_CARD_SPECIAL_ACK					= 0xAA,			//特殊应答,针对电源控制应答,从机自检等
  
  IC_CARD_CHECK_START					= 0xF0,			//自定义的,卡检测开始
  IC_CARD_CHECK_END						= 0xF1,			//自定义的,卡检测结束
}PERIPHERAL_CMD_Def;//外设命令类型
#define	SLAVE_FIRMWARE_DATA_LEN	516
typedef struct slaveFirmwareUpdate
{
  u8	packData[SLAVE_FIRMWARE_DATA_LEN];
}SLAVE_FIRMWARE_UPDATE_MSG_BODY;//从机模块固件更新消息体

static SLAVE_FIRMWARE_UPDATE_MSG_BODY	g_slaveFirmwarePackInfo;
typedef struct 
{
  u8 	cardLaunchStep;
  u8	lastCardLaunchStep;
  u16	delayCount;
}ICCARD_LAUNCH_CTRL;
//static ICCARD_LAUNCH_CTRL cardLaunchCtrl;

typedef struct
{
  u8	step:4;
  u8	sendStatus:1;
  u8	ackStatus:1;
  u8	hold:2;
  u8	cmdType;			//命令类型
  u8	dataType;			//数据类型
  u8	retryTimes;		//重试次数
  u16	counter;
  u8	E_Signature[8];//电子签名,即平台下发的随机码
  STT_ICCARD_MODULE_ATTRIBUTE	cardAttribute;
}TERMINAL_SEND_CTRL;//终端发出指令集

static TERMINAL_SEND_CTRL	gTerminalSendOrderCtrl;

typedef struct
{
  u8	cmdType;			//命令类型
  unsigned char	(*operate)(u8	cmdType,u8 dataType);
  void					(*response)(void);
}TERMINAL_SEND_ORDER;//终端发出指令集

const TERMINAL_SEND_ORDER gTerminalSendOrderTab[] =
{
  //POWER_ON_INDICATE,ICCard_Send_CMD_To_Card_Reader_0x01,NULL,//上电指示
  //LINK_INQUIRE,ICCard_Send_CMD_To_Card_Reader_0x02,NULL,			//链路探询
  IC_CARD_READ_ATTRIBUTE,ICCard_SendCmdForReadModuleAttribute,ICCARD_Report_TerminalReadCardAttributeInfoAck,
  //IC_CARD_READ_E_CERT,ICCard_SendCmdForReadElectronicCertificate,ICCard_CustomProtocolParse_Ack_0x0702,
  IC_CARD_CLR_E_CERT,ICCard_SendCmdForClearElectronicCertificate,ICCARD_Report_TerminalReadCardClrInbuiltInfoAck,
};

typedef struct tagSTT_ICCARD_CTRL_JTB
{
  BOOL Flag_Radio_Connected;
  BOOL Flag_Radio_Opened;
  BOOL Flag_Init;
  u16 Com_Buffer_Length;
  ST_PROTOCOL_EXDEVICE stt_exdev808;
  u16 stt_exdev808_len;
  u8	AuthRequestCode[LEN_AUTH_REQUEST_CODE];
  u8	AuthCode[LEN_AUTH_ACK_CODE];
  u8  Flag_AuthCode_Success;
  u8	SM_Stage;
  u8 	activeCardTimes; //自动触发40H指令重新进行认证,最多重发3次
  u16	SM_Counter;
  u16	LinkInquireCount;
}STT_ICCARD_CTRL_JTB;

typedef enum	
{
  AUTH_REQUEST_SUCCESS=0,				//认证请求成功
  AUTH_REQUEST_TERMINAL_OFFLINE,//终端不在线
  AUTH_REQUEST_TIMEOUT,					//认证超时
  AUTH_REQUEST_VERIFY						//确认收到信息
}STT_AUTH_REQUEST_RESULT;

//***************本地全局变量定义区*****************
STT_ICCARD_CTRL_JTB gICCardJTBCtrl;


//*************** 函数定义区 *****************

/********************************************************************
* 名称 : ICCard_Clear_Buf_COM
* 功能 : 清零无线通讯的缓冲区
* 说明   mode: 0: 清空所的的字节 1：仅清空当前长度的字节
********************************************************************/
void ICCard_Clear_Buf_COM( u8 mode )
{
  gICCardJTBCtrl.Com_Buffer_Length = 0 ;
}

/********************************************************************
* 名称 : ICCard_Process_CMD_0x40_Request_COM_Ack_Code
* 功能 : 将应答状态字 伴随其它数据一起打包 （ 终端=>雄帝IC卡读卡器 ）
* 说明 : 终端在接到读卡器命令0x40后，根据不同情况回应给 读卡器相应数据。
********************************************************************/
void ICCard_Process_CMD_0x40_Request_COM_Ack_Code( u8 AckCode)
{
  ICCard_PackAndSendData(IC_CARD_AUTHENTICATION,&AckCode,1);
}

/********************************************************************
* 名称 : ICCard_Process_CMD_0x40_Request_COM_Ack_Code
* 功能 : 将应答状态字 伴随其它数据一起打包 （ 终端=>雄帝IC卡读卡器 ）
* 说明 : 终端在接到读卡器命令0x40后，应答读卡器0x03:终端确认收到消息( IC卡认证请求读卡结果 = 0x01 ~ 0x04  时);
********************************************************************/
void ICCard_Process_CMD_0x40_Request_COM_Ack_Received( void )
{
  //IC卡认证请求应答 0x03:终端确认收到消息( IC卡认证请求读卡结果 = 0x01 ~ 0x04  时);
  ICCard_Process_CMD_0x40_Request_COM_Ack_Code(0x03);  //IC卡认证请求应答 0x03:终端确认收到消息( IC卡认证请求读卡结果 = 0x01 ~ 0x04  时);
}
/********************************************************************
* 名称 : ICCard_Process_CMD_0x40_Request_COM_Ack_Result
* 功能 : 
* 说明 : 终端在状态位为0x00时，向认证中心发送64字节卡片基本信息及认证信息，
*				 并根据不同情况向读卡模块返回1或25字节结果信息
********************************************************************/
void ICCard_Process_CMD_0x40_Request_COM_Ack_Result( u8 *pBuffer,u16 BufferLen )
{
  ICCard_PackAndSendData(IC_CARD_AUTHENTICATION,pBuffer,BufferLen);  
}

/********************************************************************
* 名称 : IC_Card_Terminal_Ack_Reader_Success
* 功能 : 终端=> 雄帝IC卡读卡器。 告之 终端已经成功接收了卡签到的卡片数据，或签退信息
* 说明 : 
********************************************************************/
void ICCard_Terminal_Ack_Reader_Success( u8 Cmd)
{
  //u8 SignInAckSuccessBuffer[9]  ={0x7E, 0x4D,0x00,0x01,0x01,0x00,0x0B,0x41,0x7E};
  //u8 SignOutAckSuccessBuffer[9] ={0x7E, 0x4E,0x00,0x01,0x01,0x00,0x0B,0x42,0x7E};    
  unsigned char TempBuf = 0;
  ICCard_PackAndSendData(Cmd,&TempBuf,0);
}

/********************************************************************
* 名称 : ICCard_Net_Process
* 功能 : IC卡认证中心，网络连接处理
* 说明 : 
********************************************************************/
void ICCard_Net_Process( void )
{
  /**************IC卡认证中心的网络连接 ********************/
  if(ICAUC_GetLnkStatus())
  { //查询IC卡认证中心GPRS连接是否已经建立成功/
    //已经建立连接
    //标志位0 表明: IC卡认证中心GPRS连接刚建立
    if( FALSE ==  gICCardJTBCtrl.Flag_Radio_Connected ) 
    {
      gICCardJTBCtrl.Flag_Radio_Connected = TRUE ; 
    }
    return ;              
  } 
  //尚未建立连接
  if( TRUE == gICCardJTBCtrl.Flag_Radio_Connected) 
  { //连接断开
    gICCardJTBCtrl.Flag_Radio_Connected = FALSE ; //清0 表明: IC卡认证中心GPRS连接未建立
    gICCardJTBCtrl.Flag_Radio_Opened = FALSE ;//如果还没有建立连接，则判断是否打开连接 
  }
  if( FALSE == gICCardJTBCtrl.Flag_Radio_Opened  )
  { //如果还没有建立连接，则判断是否打开连接
    
    if( ICAUC_RET_SUCCED == ICAUC_OpenLnk()  )
    {//开启IC卡认证中心的连接
      gICCardJTBCtrl.Flag_Radio_Opened= TRUE ;  //开启IC卡认证中心的连接成功后，打开
    }
  }
}
/********************************************************************
* 名称 : ICCard_Net_Rx
* 功能 : 回调函数, 拷贝IC卡认证中心网络发出的数据, 过滤提取有效的认证信息
* 说明 : 
********************************************************************/
void ICCard_Net_Rx( u8 * pInBuf, u16 InLength )
{
  gICCardJTBCtrl.AuthCode[0] = 1;
  gICCardJTBCtrl.Flag_AuthCode_Success = 1;
  if(InLength<14+LEN_AUTH_ACK_CODE)return;
  if((pInBuf[13]==0x0B)&&(pInBuf[14]==0x00))
  {
    memcpy(gICCardJTBCtrl.AuthCode,&pInBuf[14], LEN_AUTH_ACK_CODE);
  }
}

/********************************************************************
* 名称 : ICCard_WaitOrClrPlatAuthAnswer
* 功能 : IC卡等待或清除平台认证应答
* 说明   
********************************************************************/
void ICCard_WaitOrClrPlatAuthAnswer(void)
{
  gICCardJTBCtrl.Flag_AuthCode_Success = 0;
}
/********************************************************************
* 名称 : ICCard_VerifyPlatAuthAnswer
* 功能 : IC卡确认平台认证应答
* 说明   
********************************************************************/
void ICCard_VerifyPlatAuthAnswer(void)
{
  gICCardJTBCtrl.Flag_AuthCode_Success = 1;
}
/********************************************************************
* 名称 : ICCard_GetPlatAuthAnswer
* 功能 : IC卡获取平台认证应答
* 说明   
********************************************************************/
u8 ICCard_GetPlatAuthAnswer(void)
{
  return gICCardJTBCtrl.Flag_AuthCode_Success;
}
/**************************************************************************
//函数名：ICCard_AuthenticationAck
//功能：认证应答
//输入：无
//输出：无
//返回值：无
//备注：下一个状态为空闲
***************************************************************************/
void ICCard_AuthenticationAck(u8 *pBuffer , u16 BufferLen)
{
  memcpy(gICCardJTBCtrl.AuthCode,pBuffer, BufferLen);
  ICCard_VerifyPlatAuthAnswer();
}

/*************************************************************
** 函数名称: ICCard_DisposeRecvExdeviceJTB
** 功能描述: 数据解析
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void ICCard_DisposeRecvExdeviceJTB(ST_PROTOCOL_EXDEVICE stExdevice, unsigned short datalen)
{
  if(2 == gICCardJTBCtrl.SM_Stage)
  {
    gICCardJTBCtrl.SM_Stage = 0 ;
    gICCardJTBCtrl.SM_Counter = 0 ;
    ICCard_Clear_Buf_COM(0);
  }
  else
  {
    memcpy(&gICCardJTBCtrl.stt_exdev808,&stExdevice,sizeof(ST_PROTOCOL_EXDEVICE));
    gICCardJTBCtrl.Com_Buffer_Length = datalen;
    gICCardJTBCtrl.stt_exdev808_len  = datalen;
  }
}
/********************************************************************
* 名称 : ICCard_Process_CMD_0x01
* 功能 : IC卡行标外设命令处理:  0x01 从机上电指示应答
* 说明   终端->读卡器
********************************************************************/
void ICCard_Send_CMD_To_Card_Reader_0x01( void )
{
  ICCard_Terminal_Ack_Reader_Success( POWER_ON_INDICATE);
  gICCardJTBCtrl.SM_Stage = 5;
}
/********************************************************************
* 名称 : ICCard_Recieved_CMD_From_Card_Reader_0x01
* 功能 : IC卡行标外设命令处理:  0x01 从机上电指示应答
* 说明   读卡器->终端
********************************************************************/
void ICCard_Recieved_CMD_From_Card_Reader_0x01( void )
{
  
}

/********************************************************************
* 名称 : ICCard_Send_CMD_To_Card_Reader_0x02
* 功能 : IC卡行标外设命令处理:  0x02 外设链路探询应答
* 说明   终端->读卡器
********************************************************************/
void ICCard_Send_CMD_To_Card_Reader_0x02( void )
{
  ICCard_Terminal_Ack_Reader_Success( LINK_INQUIRE );
  gICCardJTBCtrl.SM_Stage = 5;
}
/********************************************************************
* 名称 : ICCard_Send_CMD_To_Card_Reader_0x02
* 功能 : IC卡行标外设命令处理:  0x02 外设链路探询应答
* 说明   读卡器->终端
********************************************************************/
void ICCard_Recieved_CMD_From_Card_Reader_0x02( void )
{
  
}

/********************************************************************
* 名称 : ICCard_Send_CMD_To_Terminal_0x03
* 功能 : IC卡行标外设命令处理:  0x03 从机电源控制
* 说明   终端发送命令给读卡器 controlMode 00:退出省电模式  01:进入省电模式
********************************************************************/
void ICCard_Send_CMD_To_Card_Reader_0x03( u8 controlMode )
{
  u8 TempData = controlMode;
  
  ICCard_PackAndSendData(SLAVE_POWER_CONTROL,&TempData,1);
}

/********************************************************************
* 名称 : ICCard_Send_CMD_To_Terminal_0x04
* 功能 : IC卡行标外设命令处理:  0x04 从机版本信息查询
* 说明   
********************************************************************/
void ICCard_Send_CMD_To_Card_Reader_0x04( void )
{	
  u8 TempData;
  
  ICCard_PackAndSendData(SLAVE_VERSION_QUERY,&TempData,0);
}

/********************************************************************
* 名称 : ICCard_Process_CMD_0x04
* 功能 : IC卡行标外设命令处理:  0x04 从机版本信息查询应答
* 说明  
********************************************************************/
void ICCard_Process_CMD_0x04( void )
{
  u8 slaveVerNo[2];	//从机版本号
  
  memcpy(slaveVerNo,gICCardJTBCtrl.stt_exdev808.Data,2);
  gICCardJTBCtrl.SM_Stage = 5;
}
/********************************************************************
* 名称 : ICCard_Send_CMD_To_Terminal_0x05
* 功能 : IC卡行标外设命令处理:  0x05 
* 说明   slaveType 从机自检类型
********************************************************************/
void ICCard_Send_CMD_To_Card_Reader_0x05( u8 slaveType )
{
  u8 TempData = slaveType;
  
  ICCard_PackAndSendData(SLAVE_SELF_INSPECTION,&TempData,1);
}

/********************************************************************
* 名称 : ICCard_Send_CMD_To_Terminal_0x06
* 功能 : IC卡行标外设命令处理:  0x06 
* 说明   更新从机固件
********************************************************************/
void ICCard_Send_CMD_To_Terminal_0x06( void )
{		
  ICCard_PackAndSendData(SLAVE_FIRMWARE_UPDATE,g_slaveFirmwarePackInfo.packData,SLAVE_FIRMWARE_DATA_LEN);
}

/********************************************************************
* 名称 : ICCard_Process_CMD_0x06
* 功能 : IC卡行标外设命令处理:  0x06 从机固件更新
* 说明  
********************************************************************/
void ICCard_Process_CMD_0x06( void )
{
  u16	packSerial;//包序号
  u8 ackResult;	//应答结果
  u8 *p = gICCardJTBCtrl.stt_exdev808.Data;
  
  packSerial = (*p++)<<8;
  packSerial |= *p++;
  ackResult = *p;
  
  if(0x00 == ackResult)
  {
    //操作成功
  }
  else
    if(0x01 == ackResult)
    {
      //非本固件程序
      
    }
    else
      if(0x02 == ackResult)
      {
        //重发 3次后终止本次升级
      }
      else
      {
        
      }	
}

/********************************************************************
* 名称 : ICCard_Process_CMD_0x07
* 功能 : IC卡行标外设命令处理:  0x07 读取卡模块综合属性信息
* 说明  
********************************************************************/
void ICCard_Process_CMD_0x07( void )
{
  memcpy( &gTerminalSendOrderCtrl.cardAttribute.peripheralManufacturerNo[0], \
    &gICCardJTBCtrl.stt_exdev808.Data[0], \
      sizeof(STT_ICCARD_MODULE_ATTRIBUTE) ); //读卡模块属性
  gTerminalSendOrderCtrl.ackStatus = 1;
}
/********************************************************************
* 名称 : ICCard_Process_CMD_0x40_request_0
* 功能 : IC卡行标外设命令处理:  0x40 IC卡认证请求
* 说明:  认证请求结果 状态位 0: IC卡读卡成功
********************************************************************/
void ICCard_Process_CMD_0x40_Request_0x00( void )
{
  memcpy( &gICCardJTBCtrl.AuthRequestCode[0], &gICCardJTBCtrl.stt_exdev808.Data[1], LEN_AUTH_REQUEST_CODE ); //读卡器 发送64字节卡片基本信息及认证信息
  //终端向读卡器返回应答数据
  ICCard_Terminal_Ack_Reader_Success( IC_CARD_AUTHENTICATION);
  gICCardJTBCtrl.SM_Stage = 1;
  gICCardJTBCtrl.SM_Counter = 0 ;
  if(0 == CertService_GetLinkFlg())//读卡成功后打开认证服务器链接通道
  {
    CertService_SetLinkFlg();
  }
  else if(CHANNEL_DATA_3 != (CHANNEL_DATA_3&TerminalAuthorizationFlag))
  {
    CertService_Open();
  }
}

/********************************************************************
* 名称 : ICCard_Process_CMD_0x40_request_1
* 功能 : IC卡行标外设命令处理:  0x40 IC卡认证请求
* 说明:  认证请求结果 状态位 1: IC卡未插入
********************************************************************/
void ICCard_Process_CMD_0x40_Request_0x01( void )
{
  //IC卡认证请求应答 0x02:终端透传认证中心超时无应答  
  Public_ShowTextInfo( "40IC卡认证请求失败原因1:IC卡未插入", 30 );
  ICCard_Process_CMD_0x40_Request_COM_Ack_Received();
  gICCardJTBCtrl.SM_Stage = 0;
  gICCardJTBCtrl.SM_Counter = 0 ;
}

/********************************************************************
* 名称 : ICCard_Process_CMD_0x40_request_2
* 功能 : IC卡行标外设命令处理:  0x40 IC卡认证请求
* 说明:  认证请求结果 状态位 2: IC卡读卡失败
********************************************************************/
void ICCard_Process_CMD_0x40_Request_0x02( void )
{
  Public_ShowTextInfo( "40IC卡认证请求失败原因2:IC卡读卡失败", 30 ); 
  ICCard_Process_CMD_0x40_Request_COM_Ack_Received();
  gICCardJTBCtrl.SM_Stage = 0;
  gICCardJTBCtrl.SM_Counter = 0 ;  
}

/********************************************************************
* 名称 : ICCard_Process_CMD_0x40_request_3
* 功能 : IC卡行标外设命令处理:  0x40 IC卡认证请求
* 说明:  认证请求结果 状态位 3: 非从业资格证IC卡
********************************************************************/
void ICCard_Process_CMD_0x40_Request_0x03( void )
{
  Public_ShowTextInfo( "40IC卡认证请求失败原因3:非从业资格证IC卡", 30 );
  ICCard_Process_CMD_0x40_Request_COM_Ack_Received();
  gICCardJTBCtrl.SM_Stage = 0;
  gICCardJTBCtrl.SM_Counter = 0 ;
  
}

/********************************************************************
* 名称 : ICCard_Process_CMD_0x40_request_4
* 功能 : IC卡行标外设命令处理:  0x40 IC卡认证请求
* 说明:  认证请求结果 状态位 4: //IC卡被锁定
********************************************************************/
void ICCard_Process_CMD_0x40_Request_0x04( void )
{
  Public_ShowTextInfo( "40IC卡认证请求失败原因4:IC卡被锁定", 30 );
  ICCard_Process_CMD_0x40_Request_COM_Ack_Received();
  gICCardJTBCtrl.SM_Stage = 0;
  gICCardJTBCtrl.SM_Counter = 0 ;
}

/********************************************************************
* 名称 : ICCard_Process_CMD_0x40_request_4
* 功能 : IC卡行标外设命令处理:  0x40 IC卡认证请求
* 说明:  认证请求结果 状态位 4: //未知的原因
********************************************************************/
void ICCard_Process_CMD_0x40_Request_0xXX( void )
{
  Public_ShowTextInfo( "40IC卡认证请求失败原因X:未知", 30 );
  gICCardJTBCtrl.SM_Stage = 0;
  gICCardJTBCtrl.SM_Counter = 0 ;
}
/********************************************************************
* 名称 : ICCard_Process_CMD_0x40
* 功能 : IC卡行标外设命令处理:  0x40 IC卡认证请求
* 说明  
********************************************************************/
void ICCard_Process_CMD_0x40( void )
{
  u8 result_readcard = 0 ;  
  
  result_readcard = gICCardJTBCtrl.stt_exdev808.Data[0] ;
  switch( result_readcard )
  {
  case 0x00: //IC卡读卡成功
    ICCard_Process_CMD_0x40_Request_0x00();
    break;
  case 0x01: //IC卡未插入
    ICCard_Process_CMD_0x40_Request_0x01();
    break;
  case 0x02: //IC卡读卡失败；
    ICCard_Process_CMD_0x40_Request_0x02();
    break;
  case 0x03: //非从业资格证IC卡
    ICCard_Process_CMD_0x40_Request_0x03();
    break;
  case 0x04: //IC卡被锁定
    ICCard_Process_CMD_0x40_Request_0x04();
    break;
  default:
    ICCard_Process_CMD_0x40_Request_0xXX();
    break;
  }
}
/********************************************************************
* 名称 : ICCard_Process_Repeat_0x40
* 功能 : 读卡模块重发40命令
* 说明  
********************************************************************/
void ICCard_Process_Repeat_0x40( void )
{ 
  ICCard_Terminal_Ack_Reader_Success(IC_CARD_AUTHENTICATION);
}

/********************************************************************
* 名称 : ICCard_Process_CMD_0x41
* 功能 : IC卡行标外设命令处理:  0x41 IC卡读取结果通知
* 说明:  IC卡读卡器发出的 读取结果通知 状态位 0: IC卡读卡成功 ,此时有后续数据
********************************************************************/
void ICCard_Process_CMD_0x41_Result_0x00( void )
{
  ICCard_Terminal_Ack_Reader_Success( IC_CARD_READ);
  ////////////////////
  ICCard_DisposeJTBCardDriverSignIn(&gICCardJTBCtrl.stt_exdev808.Data[1],gICCardJTBCtrl.stt_exdev808_len);
  /////////////////////
  
  gICCardJTBCtrl.SM_Stage = 0;
  gICCardJTBCtrl.SM_Counter = 0 ;
}

/********************************************************************
* 名称 : ICCard_Process_CMD_0x41
* 功能 : IC卡行标外设命令处理:  0x41 IC卡读取结果通知
* 说明:  IC卡读卡器发出的 读取结果通知 状态位 1: IC卡读卡失败 ,原因为卡片密钥认证未通过
********************************************************************/
void ICCard_Process_CMD_0x41_Result_0x01( void )
{
  Public_ShowTextInfo( "41读卡失败原因1:卡片密钥认证未通过", 30 );  
  ICCard_Terminal_Ack_Reader_Success( IC_CARD_READ);
  gICCardJTBCtrl.SM_Stage = 0;
  gICCardJTBCtrl.SM_Counter = 0 ;  
}

/********************************************************************
* 名称 : ICCard_Process_CMD_0x41
* 功能 : IC卡行标外设命令处理:  0x41 IC卡读取结果通知
* 说明:  IC卡读卡器发出的 读取结果通知 状态位 2: IC卡读卡失败 ,原因为卡片已被锁定
********************************************************************/
void ICCard_Process_CMD_0x41_Result_0x02( void )
{
  Public_ShowTextInfo( "41读卡失败原因2:卡片已被锁定", 30 );
  ICCard_Terminal_Ack_Reader_Success( IC_CARD_READ);
  gICCardJTBCtrl.SM_Stage = 0;
  gICCardJTBCtrl.SM_Counter = 0 ;
}

/********************************************************************
* 名称 : ICCard_Process_CMD_0x41
* 功能 : IC卡行标外设命令处理:  0x41 IC卡读取结果通知
* 说明:  IC卡读卡器发出的 读取结果通知 状态位 3: IC卡读卡失败 ,原因为卡片被拔出
********************************************************************/
void ICCard_Process_CMD_0x41_Result_0x03( void )
{
  Public_ShowTextInfo( "41读卡失败原因3:卡片被拔出", 30 );
  ICCard_Terminal_Ack_Reader_Success( IC_CARD_READ);
  gICCardJTBCtrl.SM_Stage = 0;
  gICCardJTBCtrl.SM_Counter = 0 ;    
}

/********************************************************************
* 名称 : ICCard_Process_CMD_0x41
* 功能 : IC卡行标外设命令处理:  0x41 IC卡读取结果通知
* 说明:  IC卡读卡器发出的 读取结果通知 状态位 4: IC卡读卡失败 ,原因为数据校检错误
********************************************************************/
void ICCard_Process_CMD_0x41_Result_0x04( void )
{
  Public_ShowTextInfo( "41读卡失败原因4:卡片数据校检错误", 30 );
  ICCard_Terminal_Ack_Reader_Success( IC_CARD_READ);
  gICCardJTBCtrl.SM_Stage = 0;
  gICCardJTBCtrl.SM_Counter = 0 ;    
}
/********************************************************************
* 名称 : ICCard_Process_CMD_0x41
* 功能 : IC卡行标外设命令处理:  0x41 IC卡读取结果通知
* 说明:  IC卡读卡器发出的 读取结果通知 状态位 4: IC卡读卡失败 ,原因为数据校检错误
********************************************************************/
void ICCard_Process_CMD_0x41_Result_0x05( void )
{
  Public_ShowTextInfo( "41读卡成功5:卡座上卡片非电子证件", 30 );
  ICCard_Terminal_Ack_Reader_Success( IC_CARD_READ);
  gICCardJTBCtrl.SM_Stage = 0;
  gICCardJTBCtrl.SM_Counter = 0 ;    
}
/********************************************************************
* 名称 : ICCard_Process_CMD_0x41
* 功能 : IC卡行标外设命令处理:  0x41 IC卡读取结果通知
* 说明:  IC卡读卡器发出的 读取结果通知 状态位 4: IC卡读卡失败 ,原因为数据校检错误
********************************************************************/
void ICCard_Process_CMD_0x41_Result_0x06( void )
{
  Public_ShowTextInfo( "41读卡成功6:卡座上卡片非从业人员资格证", 30 );
  ICCard_Terminal_Ack_Reader_Success( IC_CARD_READ);
  gICCardJTBCtrl.SM_Stage = 0;
  gICCardJTBCtrl.SM_Counter = 0 ;    
}
/********************************************************************
* 名称 : ICCard_Process_CMD_0x41
* 功能 : IC卡行标外设命令处理:  0x41 IC卡读取结果通知
* 说明:  IC卡读卡器发出的 读取结果通知 状态位 4: IC卡读卡失败 ,原因为数据校检错误
********************************************************************/
void ICCard_Process_CMD_0x41_Result_0x07( void )
{
  Public_ShowTextInfo( "41读卡成功7:天线异常", 30 );
  ICCard_Terminal_Ack_Reader_Success( IC_CARD_READ);
  gICCardJTBCtrl.SM_Stage = 0;
  gICCardJTBCtrl.SM_Counter = 0 ;    
}
/********************************************************************
* 名称 : ICCard_Process_CMD_0x41
* 功能 : IC卡行标外设命令处理:  0x41 IC卡读取结果通知
* 说明:  IC卡读卡器发出的 读取结果通知 状态位 其它: IC卡读卡失败 ,原因未知
********************************************************************/
void ICCard_Process_CMD_0x41_Result_0xXX( void )
{
  Public_ShowTextInfo( "41读卡失败原因X: 未知", 30 );
  ICCard_Terminal_Ack_Reader_Success( IC_CARD_READ);
  gICCardJTBCtrl.SM_Stage = 0;
  gICCardJTBCtrl.SM_Counter = 0 ;
}
/********************************************************************
* 名称 : ICCard_Process_CMD_0x41
* 功能 : IC卡行标外设命令处理:  0x41 IC卡读取结果通知
* 说明  	无论终端收到IC卡读取结果是否为0x00，都是用使用0x0702命令向归属平台发送证件身份信息
********************************************************************/
void ICCard_Process_CMD_0x41( void )
{
  u8 Result = 0 ;
  Result = gICCardJTBCtrl.stt_exdev808.Data[0] ;    
  switch(Result )
  {
  case 0x00: //IC卡读卡成功 ,从业人员资格证 此时有后续数据
  case 0x10: //IC卡读卡成功 ,道路运输证 此时有后续数据
  case 0x20: //IC卡读卡成功 , 此时有后续数据
    //Public_PlayTTSVoiceStr("IC卡读卡成功");
    ICCard_Process_CMD_0x41_Result_0x00();
    break;
  case 0x01: //IC卡读卡失败 ,原因为卡片密钥认证未通过
  case 0x11:
  case 0x21:
    Public_PlayTTSVoiceStr("IC卡密钥认证未通过");
    ICCard_Process_CMD_0x41_Result_0x01();
    break;
  case 0x02: //IC卡读卡失败 ,原因为卡片已被锁定
  case 0x12:
  case 0x22:
    Public_PlayTTSVoiceStr("IC卡已被锁定");
    ICCard_Process_CMD_0x41_Result_0x02();
    break;
  case 0x03: //IC卡读卡失败 ,原因为卡片被拔出
  case 0x13:
  case 0x23:
    Public_PlayTTSVoiceStr("IC卡被拔出");
    ICCard_Process_CMD_0x41_Result_0x03();
    break;
  case 0x04: //IC卡读卡失败 ,原因为数据校检错误
  case 0x14:
  case 0x24:
    Public_PlayTTSVoiceStr("IC卡数据校检错误");
    ICCard_Process_CMD_0x41_Result_0x04();
    break;
  case 0x05: //IC卡读卡失败 ,原因为卡片已被锁定
  case 0x15:
  case 0x25:
    Public_PlayTTSVoiceStr("卡座上卡片非电子证件");
    ICCard_Process_CMD_0x41_Result_0x05();
    break;
  case 0x06: //IC卡读卡失败 ,原因为卡片被拔出
  case 0x16:
  case 0x26:
    Public_PlayTTSVoiceStr("卡座上卡片非从业人员资格证");
    ICCard_Process_CMD_0x41_Result_0x06();
    break;
  case 0x07: //IC卡读卡失败 ,原因为数据校检错误
  case 0x17:
  case 0x27:
    Public_PlayTTSVoiceStr("天线异常");
    ICCard_Process_CMD_0x41_Result_0x07();
    break;
  default: //IC卡读卡失败 ,原因未知
    ICCard_Process_CMD_0x41_Result_0xXX();
    break;
  }
}

/********************************************************************
* 名称 : ICCard_Process_CMD_0x42
* 功能 : IC卡行标外设命令处理:  0x42 IC卡读取结果通知
* 说明:  IC卡读卡器发出的 读取结果通知 状态位 0: IC卡读卡成功 ,此时有后续数据
********************************************************************/
void ICCard_Process_CMD_0x42_Result_0x00( void )
{
  ////////////////////
  //ICCard_DisposeJTBCardDriverSignOut(&gICCardJTBCtrl.stt_exdev808.Data[1],gICCardJTBCtrl.stt_exdev808_len);
  /////////////////////
  ICCard_Terminal_Ack_Reader_Success( IC_CARD_PULL);
  ICCard_DisposeExternalCardDriverSignOut();    
  gICCardJTBCtrl.SM_Stage = 0;
  gICCardJTBCtrl.SM_Counter = 0 ;
}

/********************************************************************
* 名称 : ICCard_Process_CMD_0x42
* 功能 : IC卡行标外设命令处理:  0x42 IC卡读取结果通知
* 说明:  IC卡读卡器发出的 读取结果通知 状态位 1: IC卡读卡失败 ,原因为卡片密钥认证未通过
********************************************************************/
void ICCard_Process_CMD_0x42_Result_0x01( void )
{
  Public_ShowTextInfo( "42读卡失败原因1:卡片密钥认证未通过", 30 );            
  ICCard_Terminal_Ack_Reader_Success( IC_CARD_PULL);
  gICCardJTBCtrl.SM_Stage = 0;
  gICCardJTBCtrl.SM_Counter = 0 ;  
}

/********************************************************************
* 名称 : ICCard_Process_CMD_0x42
* 功能 : IC卡行标外设命令处理:  0x42 IC卡读取结果通知
* 说明:  IC卡读卡器发出的 读取结果通知 状态位 2: IC卡读卡失败 ,原因为卡片已被锁定
********************************************************************/
void ICCard_Process_CMD_0x42_Result_0x02( void )
{
  Public_ShowTextInfo( "42读卡失败原因2:卡片已被锁定", 30 );
  ICCard_Terminal_Ack_Reader_Success( IC_CARD_PULL);
  gICCardJTBCtrl.SM_Stage = 0;
  gICCardJTBCtrl.SM_Counter = 0 ;
}

/********************************************************************
* 名称 : ICCard_Process_CMD_0x42
* 功能 : IC卡行标外设命令处理:  0x42 IC卡读取结果通知
* 说明:  IC卡读卡器发出的 读取结果通知 状态位 3: IC卡读卡失败 ,原因为卡片被拔出
********************************************************************/
void ICCard_Process_CMD_0x42_Result_0x03( void )
{
  Public_ShowTextInfo( "42读卡失败原因3:卡片被拔出", 30 );
  ICCard_Terminal_Ack_Reader_Success( IC_CARD_PULL);
  gICCardJTBCtrl.SM_Stage = 0;
  gICCardJTBCtrl.SM_Counter = 0 ;    
}

/********************************************************************
* 名称 : ICCard_Process_CMD_0x42
* 功能 : IC卡行标外设命令处理:  0x41 IC卡读取结果通知
* 说明:  IC卡读卡器发出的 读取结果通知 状态位 4: IC卡读卡失败 ,原因为数据校检错误
********************************************************************/
void ICCard_Process_CMD_0x42_Result_0x04( void )
{
  Public_ShowTextInfo( "42读卡失败原因4:卡片数据校检错误", 30 );
  ICCard_Terminal_Ack_Reader_Success( IC_CARD_PULL);
  gICCardJTBCtrl.SM_Stage = 0;
  gICCardJTBCtrl.SM_Counter = 0 ;    
}
/********************************************************************
* 名称 : ICCard_Process_CMD_0x42
* 功能 : IC卡行标外设命令处理:  0x41 IC卡读取结果通知
* 说明:  IC卡读卡器发出的 读取结果通知 状态位 4: IC卡读卡失败 ,原因为数据校检错误
********************************************************************/
void ICCard_Process_CMD_0x42_Result_0x05( void )
{
  Public_ShowTextInfo( "42读卡成功5:卡座上卡片非电子证件", 30 );
  ICCard_Terminal_Ack_Reader_Success( IC_CARD_PULL);
  gICCardJTBCtrl.SM_Stage = 0;
  gICCardJTBCtrl.SM_Counter = 0 ;    
}
/********************************************************************
* 名称 : ICCard_Process_CMD_0x42
* 功能 : IC卡行标外设命令处理:  0x41 IC卡读取结果通知
* 说明:  IC卡读卡器发出的 读取结果通知 状态位 4: IC卡读卡失败 ,原因为数据校检错误
********************************************************************/
void ICCard_Process_CMD_0x42_Result_0x06( void )
{
  Public_ShowTextInfo( "42读卡成功6:卡座上卡片非从业人员资格证", 30 );
  ICCard_Terminal_Ack_Reader_Success( IC_CARD_PULL);
  gICCardJTBCtrl.SM_Stage = 0;
  gICCardJTBCtrl.SM_Counter = 0 ;    
}
/********************************************************************
* 名称 : ICCard_Process_CMD_0x42
* 功能 : IC卡行标外设命令处理:  0x41 IC卡读取结果通知
* 说明:  IC卡读卡器发出的 读取结果通知 状态位 4: IC卡读卡失败 ,原因为数据校检错误
********************************************************************/
void ICCard_Process_CMD_0x42_Result_0x07( void )
{
  Public_ShowTextInfo( "42读卡成功7:天线异常", 30 );
  ICCard_Terminal_Ack_Reader_Success( IC_CARD_PULL);
  gICCardJTBCtrl.SM_Stage = 0;
  gICCardJTBCtrl.SM_Counter = 0 ;    
}
/********************************************************************
* 名称 : ICCard_Process_CMD_0x42
* 功能 : IC卡行标外设命令处理:  0x42 IC卡读取结果通知
* 说明:  IC卡读卡器发出的 读取结果通知 状态位 其它: IC卡读卡失败 ,原因未知
********************************************************************/
void ICCard_Process_CMD_0x42_Result_0xXX( void )
{
  Public_ShowTextInfo( "42读卡失败原因X: 未知", 30 );
  ICCard_Terminal_Ack_Reader_Success( IC_CARD_PULL);
  gICCardJTBCtrl.SM_Stage = 0;
  gICCardJTBCtrl.SM_Counter = 0 ;
}
/********************************************************************
* 名称 : ICCard_Process_CMD_0x42
* 功能 : IC卡行标外设命令处理:  0x42 IC卡拔出通知
* 说明  
********************************************************************/
void ICCard_Process_CMD_0x42( void )
{ 
  //ICCard_DisposeExternalCardDriverSignOut();
  //ICCard_Terminal_Ack_Reader_Success( IC_CARD_PULL);
  u8 Result = 0 ;
  
  gICCardJTBCtrl.activeCardTimes = 0;//拔卡次数清零
  
  Result = gICCardJTBCtrl.stt_exdev808.Data[0] ;    
  switch(Result )
  {
  case 0x00: //IC卡读卡成功 ,此时有后续数据
  case 0x10:
  case 0x20:
    //Public_PlayTTSVoiceStr("IC卡读卡成功");
    ICCard_Process_CMD_0x42_Result_0x00();
    break;
  case 0x01: //IC卡读卡失败 ,原因为卡片密钥认证未通过
  case 0x11:
  case 0x21:
    Public_PlayTTSVoiceStr("IC卡密钥认证未通过");
    ICCard_Process_CMD_0x42_Result_0x01();
    break;
  case 0x02: //IC卡读卡失败 ,原因为卡片已被锁定
  case 0x12:
  case 0x22:
    Public_PlayTTSVoiceStr("IC卡已被锁定");
    ICCard_Process_CMD_0x42_Result_0x02();
    break;
  case 0x03: //IC卡读卡失败 ,原因为卡片被拔出
  case 0x13:
  case 0x23:
    Public_PlayTTSVoiceStr("IC卡被拔出");
    ICCard_Process_CMD_0x42_Result_0x03();
    break;
  case 0x04: //IC卡读卡失败 ,原因为数据校检错误
  case 0x14:
  case 0x24:
    Public_PlayTTSVoiceStr("IC卡数据校检错误");
    ICCard_Process_CMD_0x42_Result_0x04();
    break;
  case 0x05: //IC卡读卡失败 ,原因为卡片已被锁定
  case 0x15:
  case 0x25:
    Public_PlayTTSVoiceStr("卡座上卡片非电子证件");
    ICCard_Process_CMD_0x42_Result_0x05();
    break;
  case 0x06: //IC卡读卡失败 ,原因为卡片被拔出
  case 0x16:
  case 0x26:
    Public_PlayTTSVoiceStr("卡座上卡片非从业人员资格证");
    ICCard_Process_CMD_0x42_Result_0x06();
    break;
  case 0x07: //IC卡读卡失败 ,原因为数据校检错误
  case 0x17:
  case 0x27:
    Public_PlayTTSVoiceStr("天线异常");
    ICCard_Process_CMD_0x42_Result_0x07();
    break;		
  default: //IC卡读卡失败 ,原因未知
    ICCard_Process_CMD_0x42_Result_0xXX();
    break;
  }
}

/********************************************************************
* 名称 : ICCard_Process_CMD_0x43
* 功能 : IC卡行标外设命令处理:  0x43 主动触发读取IC卡
* 说明   cardType: 卡片类型  0x01: 道路运输证 0x02:  从业人员资格证
********************************************************************/
void ICCard_Process_CMD_0x43( u8 cardType )
{
  u8 TempData = cardType;
#if (ICCARD_JTD == ICCARD_SEL)
  {
    ICCard_PackAndSendData(IC_CARD_INITIATIVE_READ,&TempData,1);
  }
#else
  {
    ICCard_PackAndSendData(IC_CARD_INITIATIVE_READ,&TempData,0);
  }	
#endif
}
/********************************************************************
* 名称 : ICCard_Process_CMD_Ack_0x43
* 功能 : IC卡行标外设命令处理:  0x43 主动触发读取IC卡的响应
* 说明  
********************************************************************/
void ICCard_Process_CMD_Ack_0x43( void )
{
  ICCard_Terminal_Ack_Reader_Success( IC_CARD_INITIATIVE_READ);   
  // gICCardJTBCtrl.SM_Stage = 0;
  // gICCardJTBCtrl.SM_Counter = 0 ;
}

/********************************************************************
* 名称 : ICCard_Process_CMD_0xA1_Result_0x01
* 功能 : IC卡行标外设命令处理:  0xA1 IC卡读取结果通知
* 说明:  IC卡读卡器发出的 读取结果通知 状态位 : IC卡读卡失败 ,原因为无内置数据
********************************************************************/
void ICCard_Process_CMD_0xA1_Result_0x01( void )
{
  Public_ShowTextInfo( "A1读卡失败原因1:无内置数据", 30 );              
}
/********************************************************************
* 名称 : ICCard_Process_CMD_0xA1_Result_0x02
* 功能 : IC卡行标外设命令处理:  0xA1 IC卡读取结果通知
* 说明:  IC卡读卡器发出的 读取结果通知 状态位 : IC卡读卡失败 ,原因为卡片被拔出
********************************************************************/
void ICCard_Process_CMD_0xA1_Result_0x02( void )
{
  Public_ShowTextInfo( "A1读卡失败原因3:卡片被拔出", 30 );    
}
/********************************************************************
* 名称 : ICCard_Process_CMD_0xA1_Result_0xXX
* 功能 : IC卡行标外设命令处理:  0xA1 IC卡读取结果通知
* 说明:  IC卡读卡器发出的 读取结果通知 状态位 其它: IC卡读卡失败 ,原因未知
********************************************************************/
void ICCard_Process_CMD_0xA1_Result_0xXX( void )
{
  Public_ShowTextInfo( "A1读卡失败原因X: 未知", 30 );
}

/********************************************************************
* 名称 : ICCard_Process_CMD_0xA1
* 功能 : IC卡行标外设命令处理:  0xA1 读取(内置)电子证件信息
* 说明  
********************************************************************/
void ICCard_Process_CMD_0xA1( void )
{
  u8 result_readcard = 0 ;  
  
  result_readcard = gICCardJTBCtrl.stt_exdev808.Data[0] ;
  switch( result_readcard )
  {
  case 0x00: //主从业人员资格证读取成功,有后续数据
  case 0x10: //道路运输证读卡成功,有后续数据
  case 0x20: //副从业人员资格证读取成功,有后续数据
    gTerminalSendOrderCtrl.ackStatus = 1;
    //这里和41命令时一样的,暂不处理
    break;
  case 0x01: //IC卡读卡失败 ,无内置数据
  case 0x11: 
  case 0x21: 
    ICCard_Process_CMD_0xA1_Result_0x01();
    break;
  case 0x02: //IC卡读卡失败 ,原因为卡片被拔出
  case 0x12:
  case 0x22:
    ICCard_Process_CMD_0xA1_Result_0x02();
    break;
  default: //IC卡读卡失败 ,原因未知
    ICCard_Process_CMD_0xA1_Result_0xXX();
    break;
  }
}

/********************************************************************
* 名称 : ICCard_Process_CMD_0xA2
* 功能 : IC卡行标外设命令处理:  0xA1 清除(内置)电子证件信息
* 说明  
********************************************************************/
void ICCard_Process_CMD_0xA2( void )
{
  //没啥要做的
  //ICCard_SendCmdForClearElectronicCertificate(0xA2,0x00);//清除读卡器模块缓存,自动触发40指令
  gTerminalSendOrderCtrl.ackStatus = 1;
  //gICCardJTBCtrl.SM_Stage = 5;
  //gICCardJTBCtrl.SM_Counter = 0 ;
}
/********************************************************************
* 名称 : ICCard_Send_CMD_To_Terminal_0xA4
* 功能 : IC卡行标外设命令处理:  0xA4 设置GPS时间
* 说明   GPS->ICU
********************************************************************/
void ICCard_Send_CMD_To_Card_Reader_0xA4( void )
{	
  u8 TempData[7];
  
  TempData[0] = 20;
  memcpy(TempData+1,&CurTime.year,6);	
  ICCard_PackAndSendData(IC_CARD_SET_TIME,TempData,7);
}
/********************************************************************
* 名称 : ICCard_Process_CMD_0xA4
* 功能 : IC卡行标外设命令处理:  0xA4 
* 说明   从机获取GPS时间    ICU->GPS
********************************************************************/
void ICCard_Process_CMD_0xA4( void )
{
  gICCardJTBCtrl.SM_Stage = 6;
}

/********************************************************************
* 名称 : ICCard_Process_CMD_0xAA
* 功能 : IC卡行标外设命令处理:  0xAA 从机应答??
* 说明   从机电源控制应答,从机自检应答
********************************************************************/
void ICCard_Process_CMD_0xAA( void )
{
  u8 result_readcard = 0 ;	
  
  result_readcard = gICCardJTBCtrl.stt_exdev808.Data[0] ;
  if(0x01 == result_readcard)
  {
    //操作成功
    //cardLaunchCtrl.cardLaunchStep = IC_CARD_CHECK_START;
    //ICCard_Terminal_Ack_Reader_Success( IC_CARD_READ);
    
  }
  else
    if(0x02 == result_readcard)
    {
      //操作失败
      
    }
    else
    {
      
    }
  
}

/********************************************************************
* 名称 : SM_Stage_Process_0
* 功能 : IC卡认证状态机处理， 阶段0
* 说明  读卡器=>终端 IC卡请求认证
********************************************************************/
void ICCard_SM_Stage_Process_0( void )
{
  unsigned char ACC;    
  ACC=Io_ReadStatusBit(STATUS_BIT_ACC);
  if(ACC)
  {
    if(59*SECOND == gICCardJTBCtrl.LinkInquireCount++%(60*SECOND))
    {
      //周期性发送链路探寻指令
      ICCard_Terminal_Ack_Reader_Success( LINK_INQUIRE );
    }
    if(gICCardJTBCtrl.LinkInquireCount > 180*SECOND)
    {
      gICCardJTBCtrl.LinkInquireCount = 0;
      Io_WriteAlarmBit(ALARM_BIT_IC_ERROR,SET);
    }
  }
  if(gICCardJTBCtrl.Com_Buffer_Length < 1)return; 
  //////////////////////////
  switch(gICCardJTBCtrl.stt_exdev808.Cmd)
  {
    case	IC_CARD_AUTHENTICATION://IC卡认证请求
      ICCard_Process_CMD_0x40();
      break;
      case	IC_CARD_READ://IC卡认证成功后就不会再走流程了
        gICCardJTBCtrl.SM_Stage = 4;
        gICCardJTBCtrl.SM_Counter = 0 ;
        return;
        case	IC_CARD_PULL://IC卡拔出通知
          ICCard_Process_CMD_0x42();
          break;
          case	IC_CARD_INITIATIVE_READ://主动触发读IC卡响应
            ICCard_Process_CMD_Ack_0x43();
            break;
            case	POWER_ON_INDICATE://从机上电指示应答
              ICCard_Send_CMD_To_Card_Reader_0x01();//上电指示	
              break;
              case	LINK_INQUIRE://外设链路探寻
		gICCardJTBCtrl.LinkInquireCount = 0;
		//ICCard_Send_CMD_To_Card_Reader_0x02();//链路探询
		break;
		case	SLAVE_VERSION_QUERY://从机版本信息查询
                  ICCard_Process_CMD_0x04();
                  break;
                  //case	SLAVE_FIRMWARE_UPDATE://从机固件更新
                  //	ICCard_Process_CMD_0x06();
                  //	break;
                  case	IC_CARD_READ_ATTRIBUTE://读卡模块综合属性信息
                    ICCard_Process_CMD_0x07();
                    break;
                    case	IC_CARD_READ_E_CERT://读取(内置)电子证件信息
                      ICCard_Process_CMD_0xA1();
                      break;
                      case	IC_CARD_CLR_E_CERT://清除(内置)电子证件信息
                        ICCard_Process_CMD_0xA2();
                        break;
                        case	IC_CARD_SET_TIME:
                          ICCard_Send_CMD_To_Card_Reader_0xA4();
                          break;
                          case	IC_CARD_SPECIAL_ACK://从机电源控制应答 //从机自检应答
                            ICCard_Process_CMD_0xAA();
                            break;
  default:break;
  }
  ICCard_Clear_Buf_COM( 1);
  
}

/********************************************************************
* 名称 : SM_Stage_Process_1
* 功能 : IC卡认证状态机处理， 阶段1
* 说明   终端=>认证中心
********************************************************************/
void ICCard_SM_Stage_Process_1( void )
{
  
  if(CHANNEL_DATA_3 != (CHANNEL_DATA_3&TerminalAuthorizationFlag))
  {
    if( gICCardJTBCtrl.SM_Counter++ > TIMEOUT_REQUEST_LINK )
    {
      gICCardJTBCtrl.SM_Stage = 0;
      gICCardJTBCtrl.SM_Counter = 0 ;
      if(gICCardJTBCtrl.activeCardTimes++ > 3)
      {
        gICCardJTBCtrl.activeCardTimes = 0;
        //ICCard_SendCmdForClearElectronicCertificate(0xA2,0x00);//清除读卡器模块缓存,自动触发40指令
        ICCard_Clear_Buf_COM( 1);
				CertService_Close();//认证异常就关闭连接
      }
    }
    //认证等待过程中有插拔卡发生
    if((IC_CARD_READ == gICCardJTBCtrl.stt_exdev808.Cmd)||(IC_CARD_PULL == gICCardJTBCtrl.stt_exdev808.Cmd))
    {
      gICCardJTBCtrl.SM_Stage = 0;
      gICCardJTBCtrl.SM_Counter = 0 ;
      ICCard_Clear_Buf_COM( 1);
			CertService_Close();//认证异常就关闭连接
    }
    return;//等待认证服务器链接成功
  }
  //用透传上发(0x0900)指令打包卡片基本信息及认证信息
  if(ACK_OK == RadioProtocol_OriginalDataUpTrans(CHANNEL_DATA_3, 0x0B, gICCardJTBCtrl.AuthRequestCode, LEN_AUTH_REQUEST_CODE))
  {
    gICCardJTBCtrl.SM_Stage = 2;
    gICCardJTBCtrl.SM_Counter = 0 ;
    ICCard_WaitOrClrPlatAuthAnswer();//等待平台应答
  }
  else
  {
    Public_ShowTextInfo( "IC卡认证中心尚未连接", 30 );
    gICCardJTBCtrl.SM_Stage = 0;
    gICCardJTBCtrl.SM_Counter = 0 ;
    if(gICCardJTBCtrl.activeCardTimes++ > 3)
    {
      gICCardJTBCtrl.activeCardTimes = 0;
      //ICCard_SendCmdForClearElectronicCertificate(0xA2,0x00);//清除读卡器模块缓存,自动触发40指令
    }
  }
}


/********************************************************************
* 名称 : SM_Stage_Process_2
* 功能 : IC卡认证状态机处理， 阶段2
* 说明  认证中心=>终端
********************************************************************/
void ICCard_SM_Stage_Process_2( void )
{	
  if( gICCardJTBCtrl.SM_Counter++ > TIMEOUT_REQUEST_AUTH )
  {
    //标准35秒
    gICCardJTBCtrl.SM_Stage = 0;
    gICCardJTBCtrl.SM_Counter = 0 ;
    if(gICCardJTBCtrl.activeCardTimes++ > 3)
    {
      gICCardJTBCtrl.activeCardTimes = 0;
			CertService_Close();//认证异常就关闭连接
      //ICCard_SendCmdForClearElectronicCertificate(0xA2,0x00);//清除读卡器模块缓存,自动触发40指令
    }
    return ;
  }
	//认证等待过程中有插拔卡发生
  if((IC_CARD_READ == gICCardJTBCtrl.stt_exdev808.Cmd)||(IC_CARD_PULL == gICCardJTBCtrl.stt_exdev808.Cmd))
  {
    gICCardJTBCtrl.SM_Stage = 0;
    gICCardJTBCtrl.SM_Counter = 0 ;
    ICCard_Clear_Buf_COM( 1);
		CertService_Close();//认证异常就关闭连接
  }
  if(0 == ICCard_GetPlatAuthAnswer())return;//平台尚未响应
  CertService_Close();//认证成功后断开认证中心服务器连接
  ICCard_WaitOrClrPlatAuthAnswer();//平台响应后清除应答
  gICCardJTBCtrl.SM_Stage = 3;
  gICCardJTBCtrl.SM_Counter = 0 ;
}

/********************************************************************
* 名称 : SM_Stage_Process_3
* 功能 : IC卡认证状态机处理， 阶段3
* 说明   终端=>读卡器
********************************************************************/
void ICCard_SM_Stage_Process_3( void )
{ 
  u8 TempBuf[LEN_AUTH_ACK_CODE];
  
  memcpy(&TempBuf[0],gICCardJTBCtrl.AuthCode,LEN_AUTH_ACK_CODE ); 
  
  if(0x1 == TempBuf[0])
  {
    //向读卡器模块返回1或25字节信息
    Public_PlayTTSVoiceStr("IC卡认证请求成功");
    Public_ShowTextInfo( "IC卡认证请求成功", 30 );
    TempBuf[0] = 0;
    ICCard_Clear_Buf_COM(1);
    ICCard_Process_CMD_0x40_Request_COM_Ack_Result(TempBuf,LEN_AUTH_ACK_CODE );
    gICCardJTBCtrl.SM_Stage = 4;
    gICCardJTBCtrl.SM_Counter = 0 ; 	
  }
  else
  {
    Public_ShowTextInfo( "IC卡认证请求失败", 30 );
    ICCard_Process_CMD_0x40_Request_COM_Ack_Result(TempBuf,1 );
    gICCardJTBCtrl.SM_Stage = 0;
    gICCardJTBCtrl.SM_Counter = 0 ; 
  }
}

/********************************************************************
* 名称 : SM_Stage_Process_4
* 功能 : IC卡认证状态机处理， 阶段4
* 说明  读卡器=>终端
********************************************************************/
void ICCard_SM_Stage_Process_4( void )
{
  if( gICCardJTBCtrl.SM_Counter++ > (3*SECOND)  )
  {
    gICCardJTBCtrl.SM_Stage = 0;
    gICCardJTBCtrl.SM_Counter = 0 ;
    return;
  }
  ///////////////
  if( gICCardJTBCtrl.Com_Buffer_Length < 1)return; 
  /***** IC卡读卡器 外设命令处理 *****/
  if( 0x41 == gICCardJTBCtrl.stt_exdev808.Cmd)
  {        
    ICCard_Process_CMD_0x41();
    ICCard_Clear_Buf_COM(1);
  }
}

/********************************************************************
* 名称 : SM_Stage_Process_5
* 功能 : IC卡认证状态机处理， 阶段5
* 说明   处理终端与读卡器直接的一些中间状态
********************************************************************/
void ICCard_SM_Stage_Process_5( void )
{
  if( gICCardJTBCtrl.SM_Counter++ > SECOND  )
  {
    gICCardJTBCtrl.SM_Stage = 0;
    gICCardJTBCtrl.SM_Counter = 0 ;
    return;
  }
  
  if(gICCardJTBCtrl.Com_Buffer_Length < 1)return; 
  //////////////////////////
  
  switch(gICCardJTBCtrl.stt_exdev808.Cmd)
  {
    case	POWER_ON_INDICATE://从机上电指示应答
      case	LINK_INQUIRE://外设链路探寻
	case	SLAVE_VERSION_QUERY://从机版本信息查询
          case	IC_CARD_SET_TIME:
            ICCard_Send_CMD_To_Card_Reader_0x03(0);//退出省点模式
            gICCardJTBCtrl.SM_Stage = 0;
            gICCardJTBCtrl.SM_Counter = 0 ;
            break;
            case	IC_CARD_READ://IC卡认证成功后就不会再走流程了
              gICCardJTBCtrl.SM_Stage = 4;
              gICCardJTBCtrl.SM_Counter = 0 ;
              return;
              case	SLAVE_POWER_CONTROL://从机电源控制
                ICCard_Send_CMD_To_Card_Reader_0x03(0);//退出省点模式
                gICCardJTBCtrl.SM_Stage = 0;
                gICCardJTBCtrl.SM_Counter = 0 ;
                break;
                case	SLAVE_SELF_INSPECTION://从机自检
                  ICCard_Send_CMD_To_Card_Reader_0x05(0x0b);
                  gICCardJTBCtrl.SM_Stage = 0;
                  gICCardJTBCtrl.SM_Counter = 0 ;
                  break;
  default:
    break;
  }
  ICCard_Clear_Buf_COM( 1 );
}

/********************************************************************
* 名称 : ICCard_SM_Stage_Process
* 功能 : IC卡认证状态机处理
* 说明  
********************************************************************/
void ICCard_SM_Stage_Process( void )
{  
  switch( gICCardJTBCtrl.SM_Stage )
  {
  case 0:
    ICCard_SM_Stage_Process_0();
    break;		
  case 1:
    ICCard_SM_Stage_Process_1();
    break;		
  case 2:
    ICCard_SM_Stage_Process_2();
    break;
  case 3:
    ICCard_SM_Stage_Process_3();    
    break;
  case 4:    
    ICCard_SM_Stage_Process_4();    
    break;
  case 5:
    ICCard_SM_Stage_Process_5();    
    break;
  default:
    gICCardJTBCtrl.SM_Stage = 0;    
    break;
  }  
}
/********************************************************************
* 名称 : Task_IC_Card_JTB_Init
* 功能 : 交通部IC卡任务初始化
* 说明  
********************************************************************/
void Task_IC_Card_JTB_Init( void )
{
  if( !gICCardJTBCtrl.Flag_Init )
  {
    SetTimerTask(TIME_ICAUC,ICAUC_TIME_TICK);
    /* 清零整个结构体STT_ICCARD_CTRL_JTB变量 , 然后置位1， 表明初始化完成 */
    memset( ( u8 *)&gICCardJTBCtrl, 0 , sizeof( STT_ICCARD_CTRL_JTB ) );
    gICCardJTBCtrl.Flag_Init = TRUE;  
    /* 注册IC卡认证中心GPRS连接的 接收回调函数 */
    ICAUC_RegReadFun( ICCard_Net_Rx );    
    ///* 再次设置IC卡任务的周期 */      
#if (ICCARD_JTD == ICCARD_SEL)
    ICCARD_ReadCertSevicePara();
#endif
  } 
}

/********************************************************************
* 名称 : Task_IC_Card_JTB
* 功能 : 交通部IC卡任务
* 说明  
********************************************************************/
FunctionalState Task_IC_Card_JTB(void)
{
  Task_IC_Card_JTB_Init();
  ////////////////////
  ICCard_Net_Process();
  ///////////////
  ICCard_SM_Stage_Process();
  ICCard_Order_Process();
  /////////////
  return ENABLE;  
}
/******************************************************************************
**                            End Of File
******************************************************************************/
/********************************************************************
* 名称 : ICCard_SearchTerminalOrder
* 功能 : 查询指令集
* 说明	
********************************************************************/
u8 ICCard_SearchTerminalOrder(u8 cmdType)
{
  
  u8 i;
  
  for(i=0;i<sizeof(gTerminalSendOrderTab)/sizeof(TERMINAL_SEND_ORDER);i++)
  {
    if(cmdType == gTerminalSendOrderTab[i].cmdType)
    {
      break;
    }
  }
  if(i==sizeof(gTerminalSendOrderTab)/sizeof(TERMINAL_SEND_ORDER))
  {
    i = 0xff;
  }
  return i;
}
/********************************************************************
* 名称 : ICCard_CustomProtocolParse
* 功能 : 广州二汽解析自定义消息集入口
* 说明  
********************************************************************/
ProtocolACK ICCard_CustomProtocolParse(u8 *pBuffer,u16 BufferLen)
{
  u16 subID;//广州二汽子消息ID
  ProtocolACK ack = ACK_OK;
  
  subID 	= *(pBuffer+1)<<8;
  subID  |= *(pBuffer+2);
  
  if(0xc108 == subID)
  {
    gTerminalSendOrderCtrl.cmdType = IC_CARD_READ_ATTRIBUTE;
    gTerminalSendOrderCtrl.dataType = 0x00;//没影响
  }
  else if(0xc702 == subID)
  {
    gTerminalSendOrderCtrl.cmdType = IC_CARD_CLR_E_CERT;
    gTerminalSendOrderCtrl.dataType = *(pBuffer+3);
  }
  gTerminalSendOrderCtrl.sendStatus = 1;
  gTerminalSendOrderCtrl.ackStatus = 0;
  
  return ack;
}

/********************************************************************
* 名称 : ICCard_Order_Process
* 功能 : 广州二汽扩展协议入口
* 说明  
********************************************************************/
void ICCARD_CustomOrderInit(void)
{
  memset((void*)&gTerminalSendOrderCtrl,0,sizeof(TERMINAL_SEND_CTRL));
}
#if 0
void ICCard_Launch_Init(void)
{
  memset((void*)&cardLaunchCtrl.cardLaunchStep,0,sizeof(ICCARD_LAUNCH_CTRL));
  cardLaunchCtrl.cardLaunchStep = IC_CARD_CHECK_START;
}
//初始化读卡过程中的一些步骤,包括上电指示,链路探询,版本查询,电源控制,从机自检等
void ICCard_Launch_Process(void)
{
  switch(cardLaunchCtrl.cardLaunchStep)
  {
    case	IC_CARD_CHECK_START:
      cardLaunchCtrl.delayCount = 0;//保持一种空闲状态
      return;
      case	IC_CARD_CHECK_END:
        if(cardLaunchCtrl.delayCount++ > SECOND)//延时1s
        {
          cardLaunchCtrl.delayCount = 0;
          //ICCard_SendCmdForClearElectronicCertificate(0xA2,0x00);//清除读卡器模块缓存
          cardLaunchCtrl.cardLaunchStep = IC_CARD_CHECK_START;
        }return;
  case  POWER_ON_INDICATE:
    if(cardLaunchCtrl.delayCount++ > SECOND)//延时1s
    {
      cardLaunchCtrl.delayCount = 0;
      ICCard_Send_CMD_To_Card_Reader_0x01();//上电指示	
      cardLaunchCtrl.lastCardLaunchStep = cardLaunchCtrl.cardLaunchStep;
      cardLaunchCtrl.cardLaunchStep = 0xff;
    }break;
  case  LINK_INQUIRE:
    if(cardLaunchCtrl.delayCount++ > SECOND)//延时1s
    {
      cardLaunchCtrl.delayCount = 0;
      ICCard_Send_CMD_To_Card_Reader_0x02();//链路探询
      cardLaunchCtrl.lastCardLaunchStep = cardLaunchCtrl.cardLaunchStep;
      cardLaunchCtrl.cardLaunchStep = 0xff;
    }break;
  case  SLAVE_POWER_CONTROL:
    if(cardLaunchCtrl.delayCount++ > SECOND)//延时1s
    {
      ICCard_Send_CMD_To_Card_Reader_0x03(0);//退出省点模式
      cardLaunchCtrl.lastCardLaunchStep = cardLaunchCtrl.cardLaunchStep;
      cardLaunchCtrl.cardLaunchStep = 0xff;
    }break;
  case  SLAVE_VERSION_QUERY:
    ICCard_Send_CMD_To_Card_Reader_0x04();
    cardLaunchCtrl.lastCardLaunchStep = cardLaunchCtrl.cardLaunchStep;
    cardLaunchCtrl.cardLaunchStep = 0xff;
    break;
  case  SLAVE_SELF_INSPECTION:
    ICCard_Send_CMD_To_Card_Reader_0x05(0x0b);
    cardLaunchCtrl.lastCardLaunchStep = cardLaunchCtrl.cardLaunchStep;
    cardLaunchCtrl.cardLaunchStep = 0xff;
    break;
  default:
    if(cardLaunchCtrl.delayCount++ > 10*SECOND)//等一会
    {
      cardLaunchCtrl.delayCount = 0;
      cardLaunchCtrl.cardLaunchStep = cardLaunchCtrl.lastCardLaunchStep;
    }break;
  }
}
#endif
void ICCard_Order_Process(void)
{
  u8 i;
  static u8 initFlg = 0;
  
  if(0==initFlg)
  {
    initFlg = 1;
    ICCARD_CustomOrderInit();
    //ICCard_Launch_Init();
  }
  //ICCard_Launch_Process();
  i = ICCard_SearchTerminalOrder(gTerminalSendOrderCtrl.cmdType);
  if(0xff==i)
  {
    return;
  }
  switch(gTerminalSendOrderCtrl.step)
  {
    case	0:
      {
        //向读卡器模块发送指令
        gTerminalSendOrderTab[i].operate(gTerminalSendOrderCtrl.cmdType,gTerminalSendOrderCtrl.dataType);
        gTerminalSendOrderCtrl.step = 1;
        gTerminalSendOrderCtrl.sendStatus = 1;
      }break;
      case	1:
        {
          if(gTerminalSendOrderCtrl.counter++ > SECOND)//超时等待时间为1s
          {
            if(gTerminalSendOrderCtrl.retryTimes++ > 3)//重发3次
            {
              gTerminalSendOrderCtrl.retryTimes = 0;
              gTerminalSendOrderCtrl.step = 3;
            }
            else
            {
              gTerminalSendOrderCtrl.step = 0;
              gTerminalSendOrderCtrl.counter = 0;
            }
          }
          else if(gTerminalSendOrderCtrl.ackStatus)
          {
            gTerminalSendOrderCtrl.step = 2;
            gTerminalSendOrderCtrl.counter = 0;
          }
        }break;
        case	2:
          {
            //向平台上报数据
            gTerminalSendOrderTab[i].response();
            gTerminalSendOrderCtrl.step = 3;
          }break;
          case	3:
  default:
    {
      memset((void*)&gTerminalSendOrderCtrl, 
             0,
             sizeof(TERMINAL_SEND_CTRL)-sizeof(STT_ICCARD_MODULE_ATTRIBUTE)-8);
    }break;
  }
  
}
//被动上报 A1 收到平台8702命令
void ICCard_CustomProtocolParse_Rev_0x8702(u8 *pBuffer,u16 BufferLen)
{
  if(BufferLen<9)return;
  
  gTerminalSendOrderCtrl.cmdType = IC_CARD_READ_E_CERT;
  gTerminalSendOrderCtrl.dataType = *pBuffer;
  memcpy(&gTerminalSendOrderCtrl.E_Signature,pBuffer+1,8);	
  
  gTerminalSendOrderCtrl.sendStatus = 1;
  gTerminalSendOrderCtrl.ackStatus = 0;	
}
//被动上报 A1 响应8702命令  0702
void ICCard_CustomProtocolParse_Ack_0x0702(void)
{
  ICCard_DisposeJTBCardDriverSignPassivity(&gICCardJTBCtrl.stt_exdev808.Data[1],gICCardJTBCtrl.stt_exdev808_len);
  ICCard_Terminal_Ack_Reader_Success( IC_CARD_READ_E_CERT);
}
//终端发出指令集
//传递电子签名
void ICCard_Transmit_E_Signature(u8 *pBuffer)
{
  memcpy(pBuffer,gTerminalSendOrderCtrl.E_Signature,8);//卡片随机码
}

/********************************************************************
* 名称 : ICCard_SendCmdForReadModuleAttribute
* 功能 : 读卡模块综合属性
* 说明   终端=>卡 0x07
********************************************************************/
unsigned char ICCard_SendCmdForReadModuleAttribute(u8	cmdType,u8 dataType)
{  
  u8 TempData[2];
  dataType = dataType;//消除警告
  
  return ICCard_PackAndSendData(cmdType,TempData,0);
}
/********************************************************************
* 名称 : ICCard_SendCmdForReadElectronicCertificate
* 功能 : 读取内置电子证件信息
* 参数 : dataType 0x00:从业人员资格证(主)  0x10:道路运输证  0x20:道路运输证(从)
* 说明   终端=>卡 0xA1
********************************************************************/
unsigned char ICCard_SendCmdForReadElectronicCertificate(u8	cmdType,u8 dataType)
{  
  u8 TempData[9];
  
  TempData[0] = dataType;
  ICCard_Transmit_E_Signature(TempData+1);//卡片随机码
  
  return ICCard_PackAndSendData(cmdType,TempData,9);
}

/********************************************************************
* 名称 : ICCard_SendCmdForClearElectronicCertificate
* 功能 : 清除内置电子证件信息
* 参数 : dataType 0x00:从业人员资格证(主)  0x10:道路运输证  0x20:道路运输证(从)
* 说明   终端=>卡 0xA2
********************************************************************/
unsigned char ICCard_SendCmdForClearElectronicCertificate(u8	cmdType,u8 dataType)
{  
  u8 TempData;
  
  TempData = dataType;
  
  return ICCard_PackAndSendData(cmdType,&TempData,1);
}

extern u8 s_ucMsgFlashBuffer[];
/*********************************************************************
//函数名称	:ICCARD_Report_0x0900_0x3108
//功能		:数据上行透传
//输入		:pBuffer:指向数据（透传数据）的指针;BufferLen:当前透传数据包长度
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:    成功,返回ACK_OK;失败返回ACK_ERROR;
//备注		:广州二汽自定义消息集：0900/3108
*********************************************************************/
ProtocolACK ICCARD_Report_0x0900_0x3108(u8 channel, u8 Type, u8 *pBuffer, u16 BufferLen)
{
  unsigned char *buffer;
  u8 SmsPhone[20];
  u16 length;
  MESSAGE_HEAD head;
  if(BufferLen >= 1023)
  {
    return ACK_ERROR;
  }
  buffer =s_ucMsgFlashBuffer;//lzm,2014.9.24
  //指向首地址  
  buffer[0] = Type;
  buffer[1] = 0x31;//大端  高字节在前
  buffer[2] = 0x08;
  memcpy(buffer+3,pBuffer,BufferLen);
  length = BufferLen+3;
  //消息ID
  head.MessageID = 0x0900;
  //消息属性
  head.Attribute = (length & 0x3ff);
  
  if(0 != RadioProtocol_SendCmd(channel,head, buffer, length, SmsPhone))
  {
    return ACK_OK;
  }
  else
  {
    return ACK_ERROR;
  }
}

/********************************************************************
* 名称 : ICCARD_Report_CardAttributeInfoAck
* 功能 : 终端读卡模块属性应答 0900/3108
* 说明   广州二汽自定义消息集
********************************************************************/
void ICCARD_Report_TerminalReadCardAttributeInfoAck(void)
{
  u8 channel = CHANNEL_DATA_3;
  
  ICCARD_Report_0x0900_0x3108(channel, \
    0xF2, \
      gTerminalSendOrderCtrl.cardAttribute.peripheralManufacturerNo, \
        sizeof(STT_ICCARD_MODULE_ATTRIBUTE));
}

/*********************************************************************
//函数名称	:ICCARD_Report_0x0900_0x3702
//功能		:数据上行透传
//输入		:pBuffer:指向数据（透传数据）的指针;BufferLen:当前透传数据包长度
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:    成功,返回ACK_OK;失败返回ACK_ERROR;
//备注		:广州二汽自定义消息集：0900/3702
*********************************************************************/
ProtocolACK ICCARD_Report_0x0900_0x3702(u8 channel, u8 Type, u8 *pBuffer, u16 BufferLen)
{
  unsigned char *buffer;
  u8 SmsPhone[20];
  u16 length;
  MESSAGE_HEAD head;
  if(BufferLen >= 1023)
  {
    return ACK_ERROR;
  }
  buffer =s_ucMsgFlashBuffer;//lzm,2014.9.24
  //指向首地址  
  buffer[0] = Type;
  buffer[1] = 0x37;//大端  高字节在前
  buffer[2] = 0x02;
  memcpy(buffer+3,pBuffer,BufferLen);
  length = BufferLen+3;
  //消息ID
  head.MessageID = 0x0900;
  //消息属性
  head.Attribute = (length & 0x3ff);
  
  if(0 != RadioProtocol_SendCmd(channel,head, buffer, length, SmsPhone))
  {
    return ACK_OK;
  }
  else
  {
    return ACK_ERROR;
  }
}

/********************************************************************
* 名称 : ICCARD_Report_ClrCardInbuiltInfoAck
* 功能 : 终端读卡模块清除内置信息应答 0900/3108
* 说明   广州二汽自定义消息集
********************************************************************/
void ICCARD_Report_TerminalReadCardClrInbuiltInfoAck(void)
{
  u8 channel = CHANNEL_DATA_3;
  u8 temp[2];
  
  temp[0] = gTerminalSendOrderCtrl.dataType;
  temp[1] = 0;//因为没有返回结果,姑且认为成功吧
  ICCARD_Report_0x0900_0x3702(channel, \
    0xF2, \
      temp, \
        2);
  
}
// 使用二汽交运卡时，读取认证服务器参数
void ICCARD_ReadCertSevicePara(void)
{
  u8 buffer[20];
  u8 bufferLen;
  
  bufferLen= EepromPram_ReadPram(E2_MAIN_SERVER_APN_ID, buffer);
  if(0 == bufferLen){
    EepromPram_WritePram(E2_MAIN_SERVER_APN_ID, "gzjtxx03.gd",strlen("gzjtxx03.gd"));
  }
  bufferLen= EepromPram_ReadPram(E2_IC_MAIN_SERVER_IP_ID, buffer);
  if(0 == bufferLen){
    EepromPram_WritePram(E2_IC_MAIN_SERVER_IP_ID, "192.168.201.95",strlen("192.168.201.95"));
  }
  bufferLen= EepromPram_ReadPram(E2_IC_MAIN_SERVER_TCP_PORT_ID, buffer);
  if(E2_IC_MAIN_SERVER_TCP_PORT_LEN != bufferLen){
    Public_ConvertLongToBuffer(11000,buffer);
    EepromPram_WritePram(E2_IC_MAIN_SERVER_TCP_PORT_ID,buffer ,E2_IC_MAIN_SERVER_TCP_PORT_LEN);
  }
}

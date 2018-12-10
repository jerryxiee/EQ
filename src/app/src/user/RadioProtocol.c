//********************************头文件************************************
#include "stm32f2xx.h" 
#include "stm32f2xx_lib.h"
#include "Can_ErQi.h"
#include "include.h"


extern const u8 ProductID[];// 制造商ID Bytes:5
extern const u8 TerminalTypeID[]; // 终端型号ID Bytes:20
extern u8 Recorder_SendData(u8 *pData, u16 Len);
extern u16	MemoryUploadSendCount;//发送次数计数
extern u32	AccOnReportTime;//缺省汇报时间间隔
extern u16 NavigationReportAckSerialNum1;//定位精度位置信息上报应答流水号
extern u16 NavigationReportAckSerialNum2;//定位精度位置信息上报应答流水号
extern u8  NavigationReportEnableFlag;//定位精度位置信息上报使能标志,1为使能
extern u8  NavigationReportAddEnableFlag;//使能开关，1为使能
//extern char TerminalVersionSW[];//终端固件版本号 STRING  Bytes:1
extern u8	BlindReportFlag;//盲区上报标志，1为要求上报或正上报，0为不要求
extern u16     BlindAckSerialNum;//盲区应答流水号
extern u8	BlindReportFlag;//盲区上报标志，1为要求上报或正上报，0为不要求
extern s16	BlindReportSector;//盲区补报的扇区
extern s16	BlindReportStep;//盲区补报的步数
extern HISTORY_AREA_STATUS 	BlindStatus;
extern u8 	BlindDelayCount;//盲区延时计数,当计数达到20秒时发送下一个包或当前包重发1次（流水号会加1）
extern u16  SubpackageCurrentSerial;//分包补传当前包流水号
extern u16  SubpackageResendFirstSerial;//分包重传第1包的流水号
extern u16  SubpackageList[];//分包列表
extern u8  SubpacketUploadFlag;//分包上传标志,1为正在上传,0为不在上传
extern u8  SubpacketUploadType;//分包补传类型,1:行驶速度
// 2:位置信息记录;3:事故疑点记录;4:超时驾驶记录;5:驾驶人身份记录;6:外部供电记录;
// 7:参数修改记录;8:速度状态日志
extern u16 SubpackageTotalPacket;//分包的总包数
extern u8 GprsOneSendOverFlg;
extern u8 FirmwareVersion[];//固件版本号,固定为5字节
//const char TerminalVersionSW[] = "102";//终端固件版本号 STRING  Bytes:1
u8 TerminalVersionHW[] = "162"; //终端硬件版本号 STRING
u8    LawlessAccAlarmEnableFlag = 1;//非法点火报警使能标志，默认使能，收到人工确认报警后不使能
u8    LogoutFlag = 0;//终端注销标志

//static ProtocolACK RadioProtocol_ParamQueryAll(u8 *pBuffer, u16 BufferLen);
u8  Uart1DataDownTransFlag = 0;//串口1数据透传标志，
u16 Uart1DataDownTransCmd = 0;//串口1数据透传数据中的命令字
u8  UpdataResultNoteAckFlag = 0;//升级结果通知应答标志,1为收到平台的通用应答了
//*******************************应答函数*********************************
const u8	CmdOperationIndex[11] = { 0,  4,  13, 17, 22, 24, 25, 33, 36, 44, 45 };//Modify By Shigle 2013-04-01
const u8	IDLowByteMax[11] = {      3,  8,  3,  4,  1,  0,  7,  2,  7,  0,  0 };//Modify By Shigle 2013-04-01

ProtocolACK (* CmdOperate[MAX_PROTOCOL_FUN_LIST])(u8 channel, u8 *pBuffer, u16 BufferLen) = {
  
        
	NullOperate,				// 0,0x8000 ,空操作						
	RadioProtocol_ServerGeneralAck,		// 1,0x8001 , P10 平台通用应答,
    NullOperate,				// 2,0x8002 , P空操作//Add By Shigle 2013-04-01		
    RadioProtocol_UploadFromBreak_Request,	// 3,0x8003 , P10 补传分包传输请求,0x8003//Add By Shigle 2013-04-01


	RadioProtocol_TerminalRegisterAck,	// 4,0x8100 ; P11 终端注册应答,
	NullOperate,				// 5,0x8101 ; 空操作 , 
	NullOperate,				// 6,0x8102 ; 空操作 , 
	RadioProtocol_PramSet,			// 7,0x8103  ; P12 设置终端参数,
  	RadioProtocol_PramSearch,				// 10,0x8104 ; P20 空操作, //Add By Shigle 2013-04-01
	RadioProtocol_TerminalControl,		// 9,0x8105  ; P18 终端控制,
  	RadioProtocol_PramSearch,		// 8,0x8106  ; P18 查询终端参数, //Modify By Shigle 2013-04-01
	RadioProtocol_TerminalAttribute,	// 11,0x8107 ; P20 查询终端属性,//Add By Shigle 2013-04-01
  	RadioProtocol_DownloadUpdatePacket,	// 12,0x8108 ; P21 下发终端升纺包,//Add By Shigle 2013-04-01
        
        
	NullOperate,				// 13,0x8200 ; 空操作
	RadioProtocol_PositionSearch,		// 14,0x8201 ; P26 位置信息查询
	RadioProtocol_TempPositionControl,	// 15,0x8202 ; P27 临时位置跟踪控制
	RadioProtocol_AlarmConfirmManual,       // 16,0x8203 ; P27 人工确人报警  //Add By Shigle 2013-04-01



	RadioProtocol_TextMessageDownload,	// 17,0x8300 ; P28 文本信息下发, 
	RadioProtocol_EvenSet,			// 18,0x8301 ; P28 事件设置,
	RadioProtocol_QuestionDownload,		// 19,0x8302 ; P29 提问下发,
	RadioProtocol_InformationOrderSet,	// 20,0x8303 ; P30 信息点播菜单设置,
	RadioProtocol_InformationService,	// 21,0x8304 ; P31 信息服务,



	RadioProtocol_TelephoneCallback,	// 22,0x8400 ;P31 电话回拨,
	RadioProtocol_TelephoneBookSet,		// 23,0x8401 ;P31 设置电话本,


	RadioProtocol_CarControl,		// 24, 0x8500 ; P32 车辆控制,


	RadioProtocol_SetRoundArea,		// 25,0x8600 ; P33 设置圆形区域,
	RadioProtocol_DelRoundArea,		// 26,0x8601 ; P34 删除圆形区域,
	RadioProtocol_SetRectangleArea,		// 27,0x8602 ; P34 设置矩形区域,
	RadioProtocol_DelRectangleArea,		// 28,0x8603 ; P35 删除矩形区域,
	RadioProtocol_SetPolygonArea,		// 29,0x8604 ; P35 设置多边形区域,
	RadioProtocol_DelPolygonArea,		// 30,0x8605 ; P36 删除多边形区域,
	RadioProtocol_SetRoad,			// 31,0x8606 ; P36 设置路线,
	RadioProtocol_DelRoad,			// 32,0x8607 ; P37 删除路线,
	RadioProtocol_RecorderDataCollection,	// 33,0x8700 ; P38 行驶记录仪信息采集,
	RadioProtocol_RecorderPramSet,		// 34,0x8701 ; P38 行驶记录仪参数下传,
	RadioProtocol_DriverInformationUploadRequest, // 35,0x8702 ; P39 上报驾驶员身份信息请求,//Add By Shigle 2013-04-01
        
        
	RadioProtocol_MultiMediaUploadAck,	// 36,0x8800 ; P40 多媒体数据上传应答,
	RadioProtocol_CameraPhoto,		// 37,0x8801 ; P42 摄像头立即拍摄,
	RadioProtocol_MemoryIndex,		// 38,0x8802 ; P43 存储多媒体数据检索,
	RadioProtocol_MemoryUpload,		// 39,0x8803 ; P43 存储多媒体数据上传,
	RadioProtocol_StartSoundRecord,		// 40,0x8804 ; P44 开始录音,
	RadioProtocol_SingleMemoryIndex,	// 41,0x8805 ; P44 单条存储多媒体数据检索上传命令,
	NullOperate,				// 42,0x8806 ; 空操作,以备协议扩展用
	RadioProtocol_JiutongPhoto,		// 43,0x8807 ; 九通拍照，命令为0x8811，解析时做了转换

	RadioProtocol_OriginalDataDownTrans,	// 44,0x8900 ; P44 数据下行透传

	RadioProtocol_ServerRSA,		// 45,0x8A00 ; P45 平台公钥

};

#define UPLOAD_CMD_NUM  27
static u16 UploadCmdList[UPLOAD_CMD_NUM][4] = {//上行命令记录：命令,流水号,通道号
  {0x0001,0,0},//0,终端通用应答
  {0x0002,0,0},//1,终端心跳
  {0x0003,0,0},//2,终端注销
  {0x0100,0,0},//3,终端注册
  {0x0102,0,0},//4,终端鉴权
  {0x0104,0,0},//5,查询终端应答
  {0x0107,0,0},//6,查询终端属性应答
  {0x0108,0,0},//7,终端升级结果通知,需要具体的业务模块保存下行指令通道号
  {0x0110,0,0},//8,快速开户指令
  {0x0200,0,0},//9,位置汇报
  {0x0201,0,0},//10,位置信息查询应答
  {0x0301,0,0},//11,事件报告
  {0x0302,0,0},//12,提问应答,需要具体的业务模块保存下行指令通道号
  {0x0303,0,0},//13,信息点播/取消
  {0x0500,0,0},//14,车辆控制应答,需要具体的业务模块保存下行指令通道号
  {0x0700,0,0},//15,记录仪数据上传,需要具体的业务模块保存下行指令通道号
  {0x0701,0,0},//16,电子运单上报
  {0x0702,0,0},//17,驾驶员身份上报
  {0x0704,0,0},//18,定位数据批量上传
  {0x0705,0,0},//19,CAN总线数据上传
  {0x0800,0,0},//20,多媒体事件信息上传,需要具体的业务模块保存下行指令通道号
  {0x0801,0,0},//21,多媒体数据上传,需要具体的业务模块保存下行指令通道号
  {0x0802,0,0},//22,存储多媒体检索应答
  {0x0805,0,0},//23,摄像头立即拍摄命令应答
  {0x0900,0,0},//24,数据上行透传
  {0x0901,0,0},//25,数据压缩上报
  {0x0a00,0,0},//26,终端RSA公钥
};
#define DOWNLOAD_CMD_NUM  39
static u16 DownloadCmdList[DOWNLOAD_CMD_NUM][6] = {//下行命令记录：命令,流水号,通道号
  {0x8001,0,0},//0
  {0x8003,0,0},//1
  {0x8100,0,0},//2
  {0x8103,0,0},//3
  {0x8104,0,0},//4
  {0x8105,0,0},//5
  {0x8106,0,0},//6
  {0x8107,0,0},
  {0x8108,0,0},
  {0x8201,0,0},//9
  {0x8202,0,0},//10
  {0x8203,0,0},
  {0x8300,0,0},
  {0x8301,0,0},
  {0x8302,0,0},
  {0x8303,0,0},
  {0x8304,0,0},
  {0x8400,0,0},
  {0x8401,0,0},
  {0x8500,0,0},//19
  {0x8600,0,0},//20
  {0x8601,0,0},
  {0x8602,0,0},
  {0x8603,0,0},
  {0x8604,0,0},
  {0x8605,0,0},
  {0x8606,0,0},
  {0x8607,0,0},
  {0x8700,0,0},
  {0x8701,0,0},//29
  {0x8702,0,0},//30
  {0x8800,0,0},
  {0x8801,0,0},
  {0x8802,0,0},
  {0x8803,0,0},
  {0x8804,0,0},
  {0x8805,0,0},
  {0x8900,0,0},
  {0x8a00,0,0},//38
};



//********************外部变量*******************

//用于接收IC卡认证平台 透传下发的认证信息
//extern u8 gICCard_Radio_Buffer_Valid;
//extern u8 gICCard_Radio_Buffer[];

//extern u8 ICCard_SM ;
//extern u16 ICCard_SM_Timer;


//----------------------------------------------------
extern const u8 EepromPramLength[];
extern UART2_RECEIVE	uart2_recdat;	//串口2接收
extern TIME_TASK MyTimerTask[];
extern u8	ResetEnableFlag;//重启使能标志
extern u16	MultiSendMax;//多次发送计数的最大值
extern u16	MultiSendCount;//多次发送计数
extern u16	RecorderTotalList;//历史轨迹记录总条数
extern u16	MultiSendRemainder;//发送最后一包的余数
extern s16	RecorderSector;//历史轨迹读取的扇区
extern s16	RecorderStep;//历史轨迹读取的步数
extern u32	RecorderTimeCount;//历史轨迹读取的时间
extern HISTORY_AREA_STATUS 	HistoryCarStatus;
extern CAN_REC  CanData;
extern HISTORY_AREA_STATUS 	HistoryTrackStatus;
extern u16	CurPacketCount;//当前正在发送或取图的为第几包
extern u16	CameraRxBufferLen;//摄像头接收缓冲数据长度
extern u8 	CameraRxBuffer[];//接收缓冲
extern CAMERA_STRUCT 	Cam[];//摄像头结构体
extern u32     PhotoID;//多媒体ID
extern u8      MediaType;//多媒体类型，0图像，1音频，2视频
extern u8      MediaFormat;//多媒体格式编码,0JPEG,1TIF,2MP3,3WAV,4WMV
extern u8	     MediaRatio;//多媒体分辨率
extern u8      MediaChannelID;//多媒体通道ID
extern u8      MediaEvenCode;//多媒体事件编码
extern u16     MediaCommand;//多媒体命令  0:停止  N:张数  0XFFFF:录象 0XFFFE:一直拍照  BY WYF
extern u16     MediaPhotoSpace; //拍照间隔时间 BY WYF
extern u8	     MemoryUploadFileName[33];//存储多媒体数据上传的完整文件名
extern u8      MultiMediaSendFlag;//多媒体发送标志，0为不在发送，1为在发送
extern u8      MultiCenterLinkFlag;//多中心连接设置标准,1为已设置
extern u8	CurrentJpgFolder;//当前图像文件夹
extern HISTORY_AREA_STATUS 	NavigationRead;
//********************************自定义数据类型****************************

extern void MOD_GetIccidBcdValue(u8 *Bcdbuff);
extern void Area_ClearRectArea(void);
//********************************宏定义************************************

//********************************变量定义**********************************

//TERMINAL_UPDATA_PACKET gUpdatePacket;

//STRUCT_UPLOADFROMBREAK gUploadFromBreak; 
//********************主要使用的缓冲*******************
u8	RadioShareBuffer[RADIO_PROTOCOL_BUFFER_SIZE];//公共共享缓冲
u8	RadioProtocolRx1Buffer[RADIO_PROTOCOL_RX1_BUFFER_SIZE];//连接1接收缓冲
Queue  RadioProtocolRx1Q;//连接1接收队列
u8	RadioProtocolRx2Buffer[RADIO_PROTOCOL_RX2_BUFFER_SIZE];//连接2接收缓冲
Queue  RadioProtocolRx2Q;//连接2接收队列
pCirQueue RadioProtocolRx3Q;
u8	RadioProtocolRxSmsBuffer[RADIO_PROTOCOL_RX_SMS_BUFFER_SIZE];//短信接收缓冲
Queue  RadioProtocolRxSmsQ;//短信接收队列
static u8	RadioProtocolTxBuffer[RADIO_PROTOCOL_BUFFER_SIZE];//协议发送缓冲,一进入这个函数,忙标志置1;退出函数时,清0
static u8	RadioProtocolParseBuffer[RADIO_PROTOCOL_BUFFER_SIZE];//协议解析缓冲,在解析一条指令前需将数据放在该缓冲
u8	RadioProtocolRxBufferBusyFlag = 0;//RadioProtocolRxBuffer忙标志,0空闲,1为忙,忙时不能往该缓冲写数据
u8	CommonShareBufferBusyFlag = 0;//CommonShareBuffer忙标志,0空闲,1为忙,忙时不能往该缓冲写数据
u16	RadioProtocolRxLen = 0;//RadioProtocolRxBuffer接收数据长度
u16	CommonShareBufferLen = 0;//CommonShareBuffer数据长度
//********************模块自身使用的变量*******************
static MESSAGE_HEAD	RadioProtocolRxHead;//接收的二进制数据经解析后得到的帧头放在此结构体中
u8	TerminalAuthorizationFlag = 0;//终端鉴权标志,bit0:保留,bit1:连接1;bit2:连接2;bit3~bit7保留;
//每次终端鉴权成功后会相应位会置1,网络断开后相应位会清0
static	u8	BigLittleFlag = 0;//系统大端小端标志,0为小端
static  u8	TerminalAuthorizationCode[LOGIN_PASSWORD_LEN] = {0};//鉴权码,第1个字节存储鉴权码的长度
static u16	QuestionAckSerialNum = 0;//提问应答流水号

//********************外部模块调用的变量*******************
u16     DelayAckCommand = 0;//延时应答命令
u8	DelaySendChannel = 0;//延时发送通道号
u8	DelaySmsPhone[20];//延时发送短信号码
u16     CarControlAckSerialNum = 0;//车辆控制应答流水号
u16     CarControlAckCommandID = 0;//车辆控制应答命令ID
u8      TerminalPhoneNumber[6] = {0};//终端手机号
static u16 SmsChannelSerialNum = 0;//短信通道流水号
static u16 Data1ChannelSerialNum = 0;//数据通道连接1流水号
static u16 Data2ChannelSerialNum = 0;//数据通道连接2流水号
static u16 Data3ChannelSerialNum = 0;//数据通道连接3流水号
u8	RecorderProtocolFormatFlag = 0;//0:下发和上传的记录仪命令不包含帧头,1:下发和上传的记录仪命令包含帧头
u16	RecorderSerialNum;//行驶记录仪应答流水号
u8	RecorderCmd;//行驶记录仪命令
u8      MultiMediaIndexSendFlag = 0;//多媒体检索上传标志,1为需要检索上传,0为不需要
u8      RecordSendFlag = 0;//录音文件上传标志,1为上传
//********************************函数声明**********************************
//*****************第一部分：标准的平台下发指令**********************

/*********************************************************************
//函数名称	:RadioProtocol_ServerGeneralAck(u8 *pBuffer, u16 BufferLen)
//功能		:平台（服务器）通用应答
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:指令ID：0x8001
*********************************************************************/
ProtocolACK RadioProtocol_ServerGeneralAck(u8 channel, u8 *pBuffer, u16 BufferLen)
{
	u16 SerialNum;
	u16	ID;
	u8	Result;
	u8	*p;
	u8	flag;
	u8	HighByte;
	u8	LowByte;
	static u8   Count = 0;
    //static u16  count = 0;
        
	p = pBuffer;
	//判断长度是否正确
	if((5 != BufferLen))
	{
		return ACK_ERROR;
	}
    ///////快速开户///////////////////////
    OpenAccount_DisposeRadioProtocol(pBuffer,BufferLen);//add by joneming 20130619
    /////////////////////////////
	//读取应答流水号
	HighByte = *p++;
	LowByte = *p++;
	SerialNum = (HighByte << 8)|LowByte;
    if(0 == SerialNum)
    {
        //为编译不报错
    }
	
	//读取应答ID
	HighByte = *p++;
	LowByte = *p++;
	ID = (HighByte << 8)|LowByte;
     
    if(0x0200 == ID)
    {
        Report_CmdAck(channel);
    }
    //判断是否为盲区补报应答
    else if(0x0704 == ID)
    {
        Blind_ReportAck(channel);
    }
    //判断是否为鉴权应答
	else if(0x0102 == ID)
	{
		//读取结果
		Result = *p;
		if(0 == Result)
		{           
			//成功
			Count++;
			SetTerminalAuthorizationFlag(channel);
			#ifdef MODEM_EYE_D_H
			EyeNet_Back_Open();
			#endif
                        if(CHANNEL_DATA_1 == channel)
                        {
                                Public_ShowTextInfo("通道1鉴权成功",100);//dxl,2013.11.9
                        }
                        else if(CHANNEL_DATA_2 == channel)
                        {
                                Public_ShowTextInfo("通道2鉴权成功",100);//dxl,2013.11.9
                        }
			Blind_StartReport(channel);//开启盲区补报
            //SetEvTask(EV_REPORT);//鉴权成功后立马上报1条位置信息,dxl,2014.2.14
            Report_UploadPositionInfo(channel);
		}
		else
		{
			//清标志
			ClearTerminalAuthorizationFlag(channel);
			//显示鉴权失败
			//Lcd_DisplayString(2, 0, 0, (u8 *)RegisterInfo[7], 16);
                        if(CHANNEL_DATA_1 == channel)
                        {
                                Public_ShowTextInfo("通道1鉴权失败",100);//dxl,2013.11.9
                        }
                        else if(CHANNEL_DATA_2 == channel)
                        {
                                Public_ShowTextInfo("通道2鉴权失败",100);//dxl,2013.11.9
                        }
            if(CHANNEL_DATA_1 == channel)
            {
			    ClearTerminalAuthorizationCode(CHANNEL_DATA_1);
            }
            else if(CHANNEL_DATA_2 == channel)
            {
                ClearTerminalAuthorizationCode(CHANNEL_DATA_2);
            }
		}
		
	}
    else if(0x0003 == ID)//终端注销
    {
        if(CHANNEL_DATA_1 == channel)
        {
            Public_ShowTextInfo("通道1注销成功",100);//dxl,2013.11.9
        }
        else if(CHANNEL_DATA_2 == channel)
        {
            Public_ShowTextInfo("通道2注销成功",100);//dxl,2013.11.9
        }
        TerminalAuthorizationFlag = 0;
    }
    else if(0x0108 == ID)//终端升级结果通知
    {
        UpdataResultNoteAckFlag = 1;
    }
	else
	{
		
		if(Count >= 1)
		{
            //全部显示
            //SetMainFullDisplayFlag();
			//开启主界面显示
			//ClrDisplayControlFlag();
		}
		Count = 0;
		//读取结果
		Result = *p;

		if(0 == Result)
		{
			//成功
		}
		else if(1 == Result)
		{
			//失败
		}
		else if(2 == Result)
		{
			//消息有误
		}
		else if(3 == Result)
		{
			//不支持
		}
		else if(4 == Result)//报警处理确认
		{
			//清除所有需要平台确认的报警 
			if(SET == Io_ReadRecordConrtolBit(RECORD_BIT_EMERGENCY))
			{
				//关闭因紧急报警开启的录音
				Io_WriteRecordConrtolBit(RECORD_BIT_EMERGENCY, RESET);
			}
			//清楚所有需人工确认的报警
			//清除紧急报警
			Io_WriteAlarmBit(ALARM_BIT_EMERGENCY, RESET);
			//清除危险预警
			Io_WriteAlarmBit(ALARM_BIT_DANGER_PRE_ALARM, RESET);
			//清除进出区域报警
			Io_WriteAlarmBit(ALARM_BIT_IN_OUT_AREA, RESET);
			//清除进出线路报警
			Io_WriteAlarmBit(ALARM_BIT_IN_OUT_ROAD, RESET);
			//清除线路行驶过长或不足报警
			Io_WriteAlarmBit(ALARM_BIT_LOAD_OVER_DRIVE, RESET);
			
			//清除非法点火
			Io_WriteAlarmBit(ALARM_BIT_LAWLESS_ACC, RESET);
			//清除非法位移
			Io_WriteAlarmBit(ALARM_BIT_LAWLESS_MOVEMENT, RESET);
			//清除非法开车门
			Io_WriteAlarmBit(ALARM_BIT_LAWLESS_OPEN_DOOR, RESET);
			
			OUTPUT2_RESET();//output2为低电平
			flag = 0;
			FRAM_BufferWrite(FRAM_EMERGENCY_FLAG_ADDR, &flag, FRAM_EMERGENCY_FLAG_LEN);//清除铁电存储标志
		}
	}
	return ACK_OK;
}
/*********************************************************************
//函数名称	:RadioProtocol_TerminalRegisterAck(u8 *pBuffer, u16 BufferLen)
//功能		:终端注册应答
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:指令ID：0x8100
*********************************************************************/
ProtocolACK RadioProtocol_TerminalRegisterAck(u8 channel, u8 *pBuffer, u16 BufferLen)
{
	u8	Result;
	u8	*p;
	u8	i;
	u8	VerifyCode;
	u8	temp;
	u16	AckSerialNum;
	p = pBuffer;


	//判断长度是否正确
	if((BufferLen < 3)||(BufferLen >= 1024))
	{
		return ACK_ERROR;
	}
	
	//获取应答流水号
	AckSerialNum = 0;
	temp = *p++;
	AckSerialNum |= temp << 8;
	temp = *p++;
	AckSerialNum |= temp;
	
	//读取结果
	Result = *p++;	

	//判断结果
	if(0 == Result)
	{
		//读取鉴权码
		if((BufferLen-3) > (LOGIN_PASSWORD_LEN-2))
		{
            Public_ShowTextInfo("鉴权码长度超出",100);//dxl,2013.11.9
			return ACK_ERROR;
		}
		//写入鉴权码长度
		TerminalAuthorizationCode[0] = BufferLen-3;
		//拷贝鉴权码
		memcpy(TerminalAuthorizationCode+1,p,BufferLen-3);
		//写入校验字节,和校验
		VerifyCode = 0;
		for(i=0; i<BufferLen-3; i++)
		{
			VerifyCode += TerminalAuthorizationCode[1+i];
		}
		TerminalAuthorizationCode[1+BufferLen-3] = VerifyCode;
		
        if(CHANNEL_DATA_1 == channel)
        {
		    //保存鉴权码
		    if(0 == EepromPram_WritePram(E2_LOGINGPRS_PASSWORD_ID, p, BufferLen-3))
		    {
			    //如果出错在写一遍
			    if(0 == EepromPram_WritePram(E2_LOGINGPRS_PASSWORD_ID, p, BufferLen-3))
                {
                    Public_ShowTextInfo("通道1鉴权码保存失败",100);//dxl,2013.11.9
                }
                else
                {
                    SetLink1LoginAuthorizationState(LOGIN_SEND_AUTHORIZATION_CMD);
		            SetTimerTask(TIME_LINK1_LOGIN, SYSTICK_1SECOND);
                    Public_ShowTextInfo("通道1终端注册成功",100);//dxl,2013.11.9
                }
		    }
            else
            {
                SetLink1LoginAuthorizationState(LOGIN_SEND_AUTHORIZATION_CMD);
		        SetTimerTask(TIME_LINK1_LOGIN, SYSTICK_1SECOND);
                Public_ShowTextInfo("通道1终端注册成功",100);//dxl,2013.11.9
            }
        }
        else if(CHANNEL_DATA_2 == channel)
        {
            //保存鉴权码
		    if(0 == EepromPram_WritePram(E2_SECOND_LOGINGPRS_PASSWORD_ID, p, BufferLen-3))
		    {
			    //如果出错在写一遍
			    if(0 == EepromPram_WritePram(E2_SECOND_LOGINGPRS_PASSWORD_ID, p, BufferLen-3))
                {
                    Public_ShowTextInfo("通道2鉴权码保存失败",100);//dxl,2013.11.9
                }
                else
                {
                    SetLink2LoginAuthorizationState(LOGIN_SEND_AUTHORIZATION_CMD);
		            SetTimerTask(TIME_LINK2_LOGIN, SYSTICK_1SECOND);
                    Public_ShowTextInfo("通道2终端注册成功",100);//dxl,2013.11.9
                }
		    }
            else
            {
                 SetLink2LoginAuthorizationState(LOGIN_SEND_AUTHORIZATION_CMD);
		         SetTimerTask(TIME_LINK2_LOGIN, SYSTICK_1SECOND);
                 Public_ShowTextInfo("通道2终端注册成功",100);//dxl,2013.11.9
            }
        }
        else
        {
            Public_ShowTextInfo("终端注册应答通道错误",100);//dxl,2013.11.9
            RadioProtocol_TerminalLogout(channel);
        }
	}
	else if(1 == Result)
	{
            Public_ShowTextInfo("车辆已被注册",100);//dxl,2013.11.9
            RadioProtocol_TerminalLogout(channel);
	}
	else if(2 == Result)
	{
            Public_ShowTextInfo("数据库中无该车辆",100);//dxl,2013.11.9
            RadioProtocol_TerminalLogout(channel);
	}
	else if(3 == Result)
	{
            Public_ShowTextInfo("终端已被注册",100);//dxl,2013.11.9
            RadioProtocol_TerminalLogout(channel);
	}
	else if(4 == Result)
	{
            Public_ShowTextInfo("数据库中无该终端",100);//dxl,2013.11.9
            RadioProtocol_TerminalLogout(channel);
	}
	return ACK_OK;
}
/*********************************************************************
//函数名称	:RadioProtocol_PramSet(u8 *pBuffer, u16 BufferLen)
//功能		:设置终端参数
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:指令ID：0x8103
*********************************************************************/
ProtocolACK RadioProtocol_PramSet(u8 channel, u8 *pBuffer, u16 BufferLen)
{
	s16	TotalPramCount;//参数总数
	//s16	PacketPramCount;//包参数个数
	u8	*p;
	s16	length;
	u8 	AckResult = 0;
	
	p = pBuffer;
	
	//长度不允许超过1024
	if(BufferLen >= 1024)
	{
		//应答结果,消息有误
		AckResult = 0;
	}
	else
	{
		//获取长度
		length = BufferLen;

		//读取参数总数
		TotalPramCount = *p++;
		length--;
		
		if(0 == TotalPramCount)
		{
			return ACK_ERROR;
		}
		RadioProtocol_WritePram(channel,p, length);
		
		
		//应答结果
		AckResult = 0;
	}
	/*
         行标多中心检测时不能立马下线，需要发送3次鉴权，所以屏蔽下面代码,dxl
	if(1 == ResetFlag)
	{
		RadioProtocol_TerminalGeneralAck(AckResult);
		for(j=0; j<1000000; j++)//确保在短线前通用应答能回复到监控平台
		{
                                                    		
		}
                Communication_Close();
                Communication_Open();
	}
	*/
	return RadioProtocol_TerminalGeneralAck(channel,AckResult);
	
}
/*********************************************************************
//函数名称	:RadioProtocol_WritePram(channel,u8 *pBuffer, u16 BufferLen)
//功能		:写参数
//输入		:pBuffer：按ID+ID长度+内容的顺序存放需写入的字节序列，无参数总个数，只有字节序列长度
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:，有线串口设置参数和无线GPRS设置参数统一调用该函数，该函数被0x8103命令调用
*********************************************************************/
ProtocolACK RadioProtocol_WritePram(u8 channel,u8 *pBuffer, u16 BufferLen)
{
    return E2ParamApp_DisposeWriteParam(channel,pBuffer,BufferLen);
}
/*********************************************************************
//函数名称	:RadioProtocol_PramSearch(u8 *pBuffer, u16 BufferLen)
//功能		:查询终端参数
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:指令ID：0x8104
*********************************************************************/
/*
static ProtocolACK RadioProtocol_ParamQueryAll(u8 *pBuffer, u16 BufferLen)
{
  
	u32	PramID;
	u8	*p = NULL;
	//u8	*p1 = NULL;
	u8	*p2 = NULL;
	//u8	ReadPramCount;
	u8	PramLen;
	u8	AckPramCount;//包参数个数
	u8	Buffer[30];//参数长度必须小于30，否则要扩大缓冲
	u16	length;
	//s16	BufferLength;
	//u8	AckResult;
	u16	AckSerialNum;
	u8	SmsPhone[20];
	u8	channel = 0;
	MESSAGE_HEAD	head;


	//长度不允许超过1024
	//if((0 != BufferLen)||(1 == CommonShareBufferBusyFlag))
	//{
		//AckResult = 2;
		//return RadioProtocol_TerminalGeneralAck(AckResult);
	//}
	//获取应答流水号
	AckSerialNum = RadioProtocolRxHead.SerialNum;
	//应答流水号写入缓冲
	p = RadioShareBuffer;
	if(NULL == p)
	{
		return ACK_ERROR;
	}
	p2 = p;
	*p++ = (AckSerialNum & 0xff00) >> 8;
	*p++ = AckSerialNum & 0xff;
	*p++;//预留1个字节存储应答参数个数
	length = 0;
	AckPramCount = 0;
  
  
  
    //读取全部内定的参数
		//-----------------平台指定的参数---------------
		for(PramID=1; PramID<0x0110; PramID++)
		{
			//判断长度
			if(length >=1024)
			{
				break;
			}
            ///if((PramID == 0x002C)||(PramID == 0x002D))//模拟终端还没有加入这两项
            //{

            //}
            //else
            //{
			//读参数
			PramLen = EepromPram_ReadPram(PramID, Buffer);
      
			if((PramLen != 0)&&(PramLen < 30)){
        
				//写参数ID
				*p++ = 0;
				*p++ = 0;
				*p++ = (PramID & 0xff00) >> 8;
				*p++ = PramID & 0xff;
				//写参数长度
				*p++ = PramLen;
				//写参数内容
				memcpy(p,Buffer,PramLen);
				p += PramLen;
				AckPramCount++;
				length += (PramLen+1+4);
        
				if(length >= 1000){
					break;
				}
				
			}
            //}
		}	//for(PramID=1; PramID<0x0110; PramID++)
	
		//写入包参数个数
		*(p2+2) = AckPramCount;
		//长度加上应答流水号,应答参数个数
		length += 3;
		//消息头
		//消息ID
		head.MessageID = 0x0104;
		head.Attribute = length;
		//判断是否为短消息
		if(CHANNEL_SMS == uart2_recdat.rec_chanel)
		{
			memcpy(SmsPhone,gSmsRecord.msgParam.TPA,16);
			channel = CHANNEL_SMS;
		}
    
		if(0 == RadioProtocol_SendCmd(channel,head, p2, length, SmsPhone))
		{
			return ACK_OK;
		}
		else
		{
			return ACK_ERROR;
		}
}
*/
/*********************************************************************
//函数名称	:RadioProtocol_PramSearch(u8 *pBuffer, u16 BufferLen)
//功能		:查询终端参数
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:指令ID：0x8104
*********************************************************************/

ProtocolACK RadioProtocol_ParamQueryAll(u8 channel,u8 *pBuffer, u16 BufferLen)
{
  
	u32	PramID;
	u8	*p = NULL;
	u8	*p1 = NULL;
	u8	*p2 = NULL;
	u8	ReadPramCount;
	u8	PramLen;
	u8	AckPramCount;//包参数个数
	u8	Buffer[30];//参数长度必须小于30，否则要扩大缓冲
	u16	length;
	//s16	BufferLength;
	//u8	AckResult;
	u16	AckSerialNum;
	u8	SmsPhone[20];
	MESSAGE_HEAD	head;


	//长度不允许超过1024
	//if((0 != BufferLen)||(1 == CommonShareBufferBusyFlag))
	//{
		//AckResult = 2;
		//return RadioProtocol_TerminalGeneralAck(AckResult);
	//}
	//获取应答流水号
	AckSerialNum = RadioProtocolRxHead.SerialNum;
	//应答流水号写入缓冲
	p = RadioShareBuffer;
	if(NULL == p)
	{
		return ACK_ERROR;
	}
	p2 = p;
	*p++ = (AckSerialNum & 0xff00) >> 8;
	*p++ = AckSerialNum & 0xff;
	*p++;//预留1个字节存储应答参数个数
	length = 0;
	AckPramCount = 0;
	if(0 == BufferLen)//读取全部的参数
	{
		//-------------平台指定的参数-------------
		for(PramID=1; PramID<0x0110; PramID++)
		{
			//判断长度
			if(length >=1024)
			{
				break;
			}
            ///if((PramID == 0x002C)||(PramID == 0x002D))//模拟终端还没有加入这两项
            //{

            //}
            //else
            //{
			//读参数
			PramLen = EepromPram_ReadPram(PramID, Buffer);
      
			if((PramLen != 0)&&(PramLen < 30)){
        
				//写参数ID
				*p++ = 0;
				*p++ = 0;
				*p++ = (PramID & 0xff00) >> 8;
				*p++ = PramID & 0xff;
				//写参数长度
				*p++ = PramLen;
				//写参数内容
				memcpy(p,Buffer,PramLen);
				p += PramLen;
				AckPramCount++;
				length += (PramLen+1+4);
        
				if(length >= 1000){
					break;
				}
				
			}
            //}
		}	
		//-------------------自定义的参数--------------------
		for(PramID=E2_PRAM_BASE_CUSTOM_ID; PramID<E2_SELF_DEFINE_ID_MAX; PramID++)
		{
			if((PramID == E2_ALL_DRIVER_INFORMATION_ID)||(PramID == E2_UPDATA_URL_ID))
			{
				
			}
			else
			{
				//读参数
				PramLen = EepromPram_ReadPram(PramID, Buffer);
				if(PramLen >= 30)
				{
					PramID++;
				}
				else if((PramLen != 0)&&(PramLen < 30))
				{
					//写参数ID
					*p++ = 0;
					*p++ = 0;
					*p++ = (PramID & 0xff00) >> 8;
					*p++ = PramID & 0xff;
					//写参数长度
					*p++ = PramLen;
					//写参数内容
					memcpy(p,Buffer,PramLen);
					p += PramLen;
					AckPramCount++;
					length += (PramLen+1+4);
					if(length >= 1000)
					{
						break;
					}
				
				}
			}
		}	
		//写入包参数个数
		*(p2+2) = AckPramCount;
		//长度加上应答流水号,应答参数个数
		length += 3;
		//消息头
		//消息ID
		head.MessageID = 0x0104;
		head.Attribute = length;
	
		if(0 != RadioProtocol_SendCmd(channel,head, p2, length, SmsPhone))
		{
			return ACK_OK;
		}
		else
		{
			return ACK_ERROR;
		}
		
		
	}
	else//读取指定的参数
	{
		p1 = pBuffer;//第1个为需要查询的参数总个数
		ReadPramCount = *p1++;
         //-----------------平台指定的参数-----------------
		while(ReadPramCount != 0)
		{
			//判断长度
			if(length >=1024)
			{
				break;
			}
            ///if((PramID == 0x002C)||(PramID == 0x002D))//模拟终端还没有加入这两项
            //{

            //}
            //else
            //{
			PramID = 0;
			PramID |= *p1 << 24;
			p1++;
			PramID |= *p1 << 16;
			p1++;
			PramID |= *p1 << 8;
			p1++;
			PramID |= *p1;
			p1++;
			ReadPramCount--;
			//读参数
			//Shigle 2013-05-13
			if( PramID > 0x110  )
			{
				PramID = PramID - 0xD00;
			}
			
			PramLen = EepromPram_ReadPram(PramID, Buffer);
			if((PramLen != 0)&&(PramLen < 30))
			{
				//写参数ID
				*p++ = 0;
				*p++ = 0;
				*p++ = (PramID & 0xff00) >> 8;
				*p++ = PramID & 0xff;
				//写参数长度
				*p++ = PramLen;
				//写参数内容
				memcpy(p,Buffer,PramLen);
				p += PramLen;
				AckPramCount++;
				length += (PramLen+1+4);
				if(length >= 1000)
				{
					break;
				}
				
			}
            //}
		}	
			
		//写入包参数个数
		*(p2+2) = AckPramCount;
		//长度加上应答流水号,应答参数个数
		length += 3;
		//消息头
		//消息ID
		head.MessageID = 0x0104;
		head.Attribute = length;
	
		if(0 != RadioProtocol_SendCmd(channel,head, p2, length, SmsPhone))
		{
			return ACK_OK;
		}
		else
		{
			return ACK_ERROR;
		}
		
	}
}


ProtocolACK RadioProtocol_PramSearch(u8 channel, u8 *pBuffer, u16 BufferLen)
{
    u8	*p = NULL;
    u8	*p1 = NULL;
    u8	AckPramCount = 0;//包参数个数
    s16	length = 0;
    u16	AckSerialNum = 0;
    u8	SmsPhone[20];
    MESSAGE_HEAD	head;
    
    //获取应答流水号
    AckSerialNum = RadioProtocolRxHead.SerialNum;
    //应答流水号写入缓冲
    p = RadioShareBuffer;
    if(NULL == p)
    {
        return ACK_ERROR;
    }
    *p++ = (AckSerialNum & 0xff00) >> 8;
    *p++ = AckSerialNum & 0xff;
    *p++;//预留1个字节存储应答参数个数
    if(0 == BufferLen)//读取全部的参数
    {
        AckPramCount=E2ParamApp_DisposeReadAllParam(p,&length,1);
    }
    else//读取指定的参数
    {
        p1 = pBuffer;//第1个为需要查询的参数总个数
        p1++;//略过
        AckPramCount = RadioProtocol_ReadPram(p, &length, p1, BufferLen-1 , 1);
    }
    //写入包参数个数
    *(RadioShareBuffer+2) = AckPramCount;
    //长度加上应答流水号,应答参数个数
    length += 3;
    //消息头
    //消息ID
    head.MessageID = 0x0104;
    head.Attribute = length;
    
    if(0 != RadioProtocol_SendCmd(channel,head, RadioShareBuffer, length, SmsPhone))
    {
        return ACK_OK;
    }
    else
    {
        return ACK_ERROR;
    }
}

/*********************************************************************
//函数名称	:RadioProtocol_ReadPram
//功能		:读参数
//输入		:把需要读的参数ID号放入缓冲pIDBuffer中，每个ID四字节，高在前，IDBufferLen为4的整数倍
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:读取的参数总个数，*pDstBuffer按ID号，ID长度，ID内容的顺序存放，*DstBufferLen总的读取长度，不包括参数总个数字节
//备注		:ReadType:0为有线(串口)，每次只能读550字节；1为无线（GPRS），每次可读1000字节
*********************************************************************/
u8 RadioProtocol_ReadPram(u8 *pDstBuffer, s16 *DstBufferLen, u8 *pIDBuffer, s16 IDBufferLen,u8 ReadType)
{
    return E2ParamApp_DisposeReadParam(pDstBuffer,DstBufferLen,pIDBuffer,IDBufferLen,ReadType);
}


/*********************************************************************
//函数名称	:RadioProtocol_TerminalControl(u8 *pBuffer, u16 BufferLen)
//功能		:终端控制
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:指令ID：0x8105
*********************************************************************/
ProtocolACK RadioProtocol_TerminalControl(u8 channel, u8 *pBuffer, u16 BufferLen)
{
	u8	Command;
	u8	*p;
	u8	Count;
	s16	length;
	u32	i,j,k;
	u16	PramLen;
	u8	AckResult;
	u8	ControlLinkWord;
	u8	Buffer[200];
	u8	ErrorFlag = 0;
	//u8	ManufactureID[7];


	p = pBuffer;
	length = BufferLen;

	//读取命令字
	Command = *p++;
	length--;
	
	if(1 == Command)//无线升级
	{
		//参数依次为：URL地址;拨号点名称;拨号用户名;拨号密码;地址;TCP端口;
		//UDP端口;制造商ID;硬件版本;固件版本;连接到指定服务器时限

		i = 0;
		j = 0;
		Count = 0;
    
		while(length > 2)
		{
			if(';' == *(p+i))//找到了分割符
			{	
				Count++;
				PramLen = i-j;
				switch(Count)
				{
					
					case 1://URL地址
					{
						EepromPram_WritePram(E2_UPDATA_URL_ID, p+j, PramLen);
						EepromPram_ReadPram(E2_UPDATA_URL_ID, Buffer);
						for(k=0; k<PramLen; k++)
						{
							if(*(p+j+k) != Buffer[k])
							{
								ErrorFlag = 1;
								break;
							}
						}
						break;
					}
					case 2://拨号点名称
					{
						EepromPram_WritePram(E2_UPDATA_DIAL_NAME_ID, p+j+1, PramLen-1);
						EepromPram_ReadPram(E2_UPDATA_DIAL_NAME_ID, Buffer);
						for(k=0; k<PramLen-1; k++)
						{
							if(*(p+j+1+k) != Buffer[k])
							{
								ErrorFlag = 1;
								break;
							}
						}
						break;
					}
					case 3://拨号用户名
					{
						EepromPram_WritePram(E2_UPDATA_DIAL_USER_ID, p+j+1, PramLen-1);
						EepromPram_ReadPram(E2_UPDATA_DIAL_USER_ID, Buffer);
						for(k=0; k<PramLen-1; k++)
						{
							if(*(p+j+1+k) != Buffer[k])
							{
								ErrorFlag = 1;
								break;
							}
						}
						break;
					}
					case 4://拨号密码
					{
						EepromPram_WritePram(E2_UPDATA_DIAL_PASSWORD_ID, p+j+1, PramLen-1);
						EepromPram_ReadPram(E2_UPDATA_DIAL_PASSWORD_ID, Buffer);
						for(k=0; k<PramLen-1; k++)
						{
							if(*(p+j+1+k) != Buffer[k])
							{
								ErrorFlag = 1;
								break;
							}
						}
						break;
					}
					case 5://地址
					{
						EepromPram_WritePram(E2_UPDATA_IP_ID, p+j+1, PramLen-1);
						EepromPram_ReadPram(E2_UPDATA_IP_ID, Buffer);
						for(k=0; k<PramLen-1; k++)
						{
							if(*(p+j+1+k) != Buffer[k])
							{
								ErrorFlag = 1;
								break;
							}
						}
						break;
					}
					case 6://TCP端口
					{
						EepromPram_WritePram(E2_UPDATA_TCP_PORT_ID, p+j+1, PramLen-1);
						EepromPram_ReadPram(E2_UPDATA_TCP_PORT_ID, Buffer);
						for(k=0; k<PramLen-1; k++)
						{
							if(*(p+j+1+k) != Buffer[k])
							{
								ErrorFlag = 1;
								break;
							}
						}
						break;
					}
					case 7://UDP端口
					{
						EepromPram_WritePram(E2_UPDATA_UDP_PORT_ID, p+j+1, PramLen-1);
						EepromPram_ReadPram(E2_UPDATA_UDP_PORT_ID, Buffer);
						for(k=0; k<PramLen-1; k++)
						{
							if(*(p+j+1+k) != Buffer[k])
							{
								ErrorFlag = 1;
								break;
							}
						}
						break;
					}
					case 8://制造商ID
					{
						
						EepromPram_WritePram(E2_UPDATA_MANUFACTURE_ID, p+j+1, PramLen-1);
						EepromPram_ReadPram(E2_UPDATA_MANUFACTURE_ID, Buffer);
						
						for(k=0; k<PramLen-1; k++)
						{
							if(*(p+j+1+k) != Buffer[k])
							{
								ErrorFlag = 1;
								break;
							}
						}
						/*
						//读制造商ID
						EepromPram_ReadPram(E2_MANUFACTURE_ID, ManufactureID);
						
						for(k=0; k<E2_MANUFACTURE_LEN; k++)
						{
							if(ManufactureID[k] != Buffer[k])
							{
								ErrorFlag = 1;
								break;
							}
						}
						*/
						break;
					}
					case 9://硬件版本
					{
						EepromPram_WritePram(E2_UPDATA_HARDWARE_VERSION_ID, p+j+1, PramLen-1);
						EepromPram_ReadPram(E2_UPDATA_HARDWARE_VERSION_ID, Buffer);
						for(k=0; k<PramLen-1; k++)
						{
							if(*(p+j+1+k) != Buffer[k])
							{
								ErrorFlag = 1;
								break;
							}
						}
						break;
					}
					case 10://固件版本
					{
						EepromPram_WritePram(E2_UPDATA_FIRMWARE_VERSION_ID, p+j+1, PramLen-1);
						EepromPram_ReadPram(E2_UPDATA_FIRMWARE_VERSION_ID, Buffer);
						for(k=0; k<PramLen-1; k++)
						{
							if(*(p+j+1+k) != Buffer[k])
							{
								ErrorFlag = 1;
								break;
							}
						}
						break;
					}
					
					default :
						break;
				}
				j = i;
			}
			i++;
			length--;	
			if(10 == Count)
			{
				break;
			}
			
			
		}
		//连接到服务器时限
		EepromPram_WritePram(E2_UPDATA_TIME_ID, p+i, 2);
		EepromPram_ReadPram(E2_UPDATA_TIME_ID, Buffer);
		for(k=0; k<2; k++)
		{
			if(*(p+i+k) != Buffer[k])
			{
				ErrorFlag = 1;
				break;
			}
		}
		if(0 == ErrorFlag)//只有数据全部正确了才会开启FTP远程升级,并应答
		{
		    #ifdef EYE_MODEM
                if(Net_Other_FtpStart())
                {
                    AckResult = 0;//成功
                    Buffer[0] = 3;//升级标志 失败 3
                    FRAM_BufferWrite(FRAM_FIRMWARE_UPDATA_FLAG_ADDR,Buffer,1);                    
                }
                else
                {
                    AckResult = 1;//失败
                }
		    #else
    			if(MyTimerTask[TIME_DELAY_TRIG].TaskTimerState == DISABLE)
    		    	{
    				// 触发无线升级任务
    				//SetEvTask(EV_FTP_UPDATA);//2秒钟后触发
    				DelayAckCommand = 2;
    				//触发延时任务
        				SetTimerTask(TIME_DELAY_TRIG, 1);
    			}
    			AckResult = 0;
		    #endif

			return RadioProtocol_TerminalGeneralAck(channel,AckResult);
		}
	}
  
	else if(2 == Command)//控制终端连接指定服务器
	{
		//参数依次为：连接控制;监管平台鉴权码;拨号点名称;拨号用户名;拨号密码;地址;TCP端口;
		//UDP端口;连接到指定服务器时限
		//读取连接控制字
		ControlLinkWord = *p;
		if(0 == ControlLinkWord)//连接到指定服务器
		{
			i = 0;
			j = 0;
			Count = 0;
			while(length > 2)
			{
				if(';' == *(p+i))//找到了分割符
				{	
					Count++;
					PramLen = i-j;
					switch(Count)
					{
						case 1://连接控制
						{
							break;
						}
						case 2://监管平台鉴权码
						{
							EepromPram_WritePram(E2_UPDATA_MONITOR_SERVER_CODE_ID, p+j+1, PramLen-1);
							EepromPram_ReadPram(E2_UPDATA_MONITOR_SERVER_CODE_ID, Buffer);
							for(k=0; k<PramLen-1; k++)
							{
								if(*(p+j+1+k) != Buffer[k])
								{
									ErrorFlag = 1;
									break;
								}
							}
							break;
						}
						case 3://拨号点名称
						{
							EepromPram_WritePram(E2_UPDATA_DIAL_NAME_ID, p+j+1, PramLen-1);
							EepromPram_ReadPram(E2_UPDATA_DIAL_NAME_ID, Buffer);
							for(k=0; k<PramLen-1; k++)
							{
								if(*(p+j+1+k) != Buffer[k])
								{
									ErrorFlag = 1;
									break;
								}
							}
							break;
						}
						case 4://拨号用户名
						{
							EepromPram_WritePram(E2_UPDATA_DIAL_USER_ID, p+j+1, PramLen-1);
							EepromPram_ReadPram(E2_UPDATA_DIAL_USER_ID, Buffer);
							for(k=0; k<PramLen-1; k++)
							{
								if(*(p+j+1+k) != Buffer[k])
								{
									ErrorFlag = 1;
									break;
								}
							}
							break;
						}
						case 5://拨号密码
						{
							EepromPram_WritePram(E2_UPDATA_DIAL_PASSWORD_ID, p+j+1, PramLen-1);
							EepromPram_ReadPram(E2_UPDATA_DIAL_PASSWORD_ID, Buffer);
							for(k=0; k<PramLen-1; k++)
							{
								if(*(p+j+1+k) != Buffer[k])
								{
									ErrorFlag = 1;
									break;
								}
							}
							break;
						}
						case 6://地址
						{
							EepromPram_WritePram(E2_UPDATA_IP_ID, p+j+1, PramLen-1);
							EepromPram_ReadPram(E2_UPDATA_IP_ID, Buffer);
							for(k=0; k<PramLen-1; k++)
							{
								if(*(p+j+1+k) != Buffer[k])
								{
									ErrorFlag = 1;
									break;
								}
							}
							break;
						}
						case 7://TCP端口
						{
							EepromPram_WritePram(E2_UPDATA_TCP_PORT_ID, p+j+1, PramLen-1);
							EepromPram_ReadPram(E2_UPDATA_TCP_PORT_ID, Buffer);
							for(k=0; k<PramLen-1; k++)
							{
								if(*(p+j+1+k) != Buffer[k])
								{
									ErrorFlag = 1;
									break;
								}
							}
							break;
						}
						case 8://UDP端口
						{
							EepromPram_WritePram(E2_UPDATA_UDP_PORT_ID, p+j+1, PramLen-1);
							EepromPram_ReadPram(E2_UPDATA_UDP_PORT_ID, Buffer);
							for(k=0; k<PramLen-1; k++)
							{
								if(*(p+j+1+k) != Buffer[k])
								{
									ErrorFlag = 1;
									break;
								}
							}
							break;
						}
						default :
							break;
					}
					j = i;
				}
				i++;
				length--;
				if(Count == 8)
				{
					break;
				}
			}
			EepromPram_WritePram(E2_UPDATA_TIME_ID, p+i, 2);
			EepromPram_ReadPram(E2_UPDATA_TIME_ID, Buffer);
			for(k=0; k<2; k++)
			{
				if(*(p+i+k) != Buffer[k])
				{
					ErrorFlag = 1;
					break;
				}
			}
			if((0 == ErrorFlag)&&(MyTimerTask[TIME_DELAY_TRIG].TaskTimerState == DISABLE))
			{
				// 触发连接监管平台任务
				//SetEvTask(EV_LINK_GOV_SERVER);
				DelayAckCommand = 6;
				//触发延时任务
    				SetTimerTask(TIME_DELAY_TRIG, 1);
				AckResult = 0;
				return RadioProtocol_TerminalGeneralAck(channel,AckResult);
			}
		}
		else if((1 == ControlLinkWord)&&(MyTimerTask[TIME_DELAY_TRIG].TaskTimerState == DISABLE))
		{
		
			//触发无线通信模块,使其重新启动,连接默认的主服务器
			//MOD_StateSwitch(MOD_PWROFF);
			DelayAckCommand = 4;
			//触发延时任务
    			SetTimerTask(TIME_DELAY_TRIG, 1);
			AckResult = 0;
			return RadioProtocol_TerminalGeneralAck(channel,AckResult);
		}
	}
	else if(3 == Command)//终端关机
	{
                if(1 == Io_ReadStatusBit(STATUS_BIT_ACC))
                {
                          //下线，重启
                          DelayAckCommand = 7;
	                  //触发延时任务
    	                  SetTimerTask(TIME_DELAY_TRIG,3*SECOND);
                }
                else
                {
                          //下线，进入深度休眠
                          DelayAckCommand = 11;
	                  //触发延时任务
    	                  SetTimerTask(TIME_DELAY_TRIG,3*SECOND);
                }
		AckResult = 0;
		return RadioProtocol_TerminalGeneralAck(channel,AckResult);
              
	}
	else if(4 == Command)//终端复位
	{
		AckResult = 0;
		RadioProtocol_TerminalGeneralAck(channel,AckResult);
		  //下线，重启
                DelayAckCommand = 7;
	          //触发延时任务
    	        SetTimerTask(TIME_DELAY_TRIG,3*SECOND);
		//Lcd_DisplayString(1, 0, 0, "终端即将重启！  ", 16);
	}
	else if(5 == Command)//恢复出厂设置
	{
		EepromPram_DefaultSet();
		AckResult = 0;
		RadioProtocol_TerminalGeneralAck(channel,AckResult);
                 //下线，重启
                DelayAckCommand = 7;
	          //触发延时任务
    	        SetTimerTask(TIME_DELAY_TRIG,3*SECOND);
	}
	else if(6 == Command)//关闭数据通信
	{
    
		//指关闭GPRS
		AckResult = 0;
		RadioProtocol_TerminalGeneralAck(channel,AckResult);
#ifdef HUOYUN_DEBUG_OPEN
                DelayAckCommand = 9;//下一步是关闭通信模块任务,延时1小时后重启
                //触发延时任务
                SetTimerTask(TIME_DELAY_TRIG, 5*SECOND);
#endif
	}
	else if(7 == Command)//关闭所有无线通信
	{
		//指关系GPRS,短信
		AckResult = 0;
		RadioProtocol_TerminalGeneralAck(channel,AckResult);
#ifdef HUOYUN_DEBUG_OPEN
                DelayAckCommand = 9;//下一步是关闭通信模块任务,延时1小时后重启
                //触发延时任务
                SetTimerTask(TIME_DELAY_TRIG, 5*SECOND);
#endif
	}
	return ACK_OK;

}
/*********************************************************************
//函数名称	:RadioProtocol_PositionSearch(u8 *pBuffer, u16 BufferLen)
//功能		:位置信息查询
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:指令ID：0x8201
*********************************************************************/


ProtocolACK RadioProtocol_PositionSearch(u8 channel, u8 *pBuffer, u16 BufferLen)
{
	u16	length;
	u16	AckSerialNum;
	u8	Buffer[FLASH_BLIND_STEP_LEN+1];
	u8	SmsPhone[20];
	u8	PramLen;
	MESSAGE_HEAD	head;

	
	
	if(0 != BufferLen)
	{
		return ACK_ERROR;
	}
	length = 0;
	//添加应答流水号
	AckSerialNum = RadioProtocolRxHead.SerialNum;
	Buffer[0] = (AckSerialNum & 0xff00) >> 8;
	Buffer[1] = AckSerialNum & 0xff;
	length += 2;
	//获取位置信息,拷贝到Buffer中
	PramLen = Report_GetPositionInfo(Buffer+2);
	length += PramLen;
	//发送位置汇报信息
	//消息ID
	head.MessageID = 0x0201;
	head.Attribute = length;

	if(0 != RadioProtocol_SendCmd(channel,head, Buffer, length, SmsPhone))
	{
		return ACK_OK;
	}
	else
	{
                //添加到重发列表
                //RadioProtocol_AddCmdToResendList(RadioProtocolRxHead.SerialNum, head.MessageID, Buffer, length, 0);
		return ACK_ERROR;

	}
	
}
/*********************************************************************
//函数名称	:RadioProtocol_TempPositionControl(u8 *pBuffer, u16 BufferLen)
//功能		:临时位置跟踪控制
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:指令ID：0x8202
*********************************************************************/
ProtocolACK RadioProtocol_TempPositionControl(u8 channel, u8 *pBuffer, u16 BufferLen)
{

	u8	*p;
	u8	AckResult;
	u16	TimeSpace;//时间间隔
	u32	ValidTime;//有效期
	u8	Buffer[5];
	
	

	p = pBuffer;
	
	//if(6 != BufferLen),有误需屏蔽,当要停止跟踪控制时是2字节,HUOYUN_DEBUG_OPEN测试时有问题
	//{
		//AckResult = 2;
		//return RadioProtocol_TerminalGeneralAck(AckResult);
	//}
	
	//写入临时跟踪时间间隔
	TimeSpace = 0;
	TimeSpace |= *p << 8;
	Buffer[0] = *p;
	p++;
	TimeSpace |= *p;
	Buffer[1] = *p;
	p++;
	FRAM_BufferWrite(FRAM_TEMP_TRACK_SPACE_ADDR, Buffer, FRAM_TEMP_TRACK_SPACE_LEN);
	Report_UpdateOnePram(PRAM_TEMP_TRACK_TIME);
        if(6 == BufferLen)
        {
	        //写入临时跟踪有效期
	        ValidTime = 0;
	        ValidTime |= *p << 24;
	        p++;
	        ValidTime |= *p << 16;
	        p++;
	        ValidTime |= *p << 8;
	        p++;
	        ValidTime |= *p;
	        ValidTime = ValidTime/TimeSpace;
	        Buffer[0] = (ValidTime&0xff000000) >> 24;
	        Buffer[1] = (ValidTime&0xff0000) >> 16;
	        Buffer[2] = (ValidTime&0xff00) >> 8;
	        Buffer[3] = ValidTime&0xff;
	        FRAM_BufferWrite(FRAM_TEMP_TRACK_NUM_ADDR, Buffer, FRAM_TEMP_TRACK_NUM_LEN);
	        Report_UpdateOnePram(PRAM_TEMP_TRACK_COUNT);
        }
	
	AckResult = 0;
	return RadioProtocol_TerminalGeneralAck(channel,AckResult);
}
/*********************************************************************
//函数名称	:RadioProtocol_TextMessageDownload(u8 *pBuffer, u16 BufferLen)
//功能		:文本信息下发
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:指令ID：0x8300
*********************************************************************/
ProtocolACK RadioProtocol_TextMessageDownload(u8 channel, u8 *pBuffer, u16 BufferLen)
{
    unsigned short len;
    TIME_T stTime;
    u8 flag;
    u8 AckResult;
    //读取标志
    flag = *pBuffer;
    if(BufferLen >= 1024)
    {
        AckResult = 2;
    }
    else
    {
        len = 0;
        AckResult = 0;        
        ///////////////////////////
        if(PUBLIC_CHECKBIT(flag, 0))
        {
            strcpy((char *)&RadioShareBuffer[len],"紧急信息:");
            len = strlen((char *)RadioShareBuffer);
        }
        ////////////////////////////
        if(PUBLIC_CHECKBIT(flag, 5))
        {
            strcpy((char *)&RadioShareBuffer[len],"CAN故障码信息:");
            len = strlen((char *)RadioShareBuffer);
        }
        else
        {
            strcpy((char *)&RadioShareBuffer[len],"中心导航信息:");
            len = strlen((char *)RadioShareBuffer);
        }
        ////////////////////
        memcpy(&RadioShareBuffer[len],pBuffer+1,BufferLen-1);
        len +=BufferLen-1;
        RadioShareBuffer[len] = 0;//结尾标志
        CommonShareBufferLen = strlen((char *)RadioShareBuffer);
        //////接收时间//////////////////// ////////////       
        RTC_GetCurTime(&stTime);
        sprintf((char *)&RadioShareBuffer[CommonShareBufferLen],"(20%02d-%02d-%02d %02d:%02d:%02d)",stTime.year,stTime.month,stTime.day,stTime.hour,stTime.min,stTime.sec);
        CommonShareBufferLen = strlen((char *)RadioShareBuffer);
        ///////////////////////////////////
        CommonShareBufferBusyFlag = 1;//CommonShareBuffer忙标志
        if(PUBLIC_CHECKBIT(flag, 2))
        {
            DownloadGB2313_WriteToFlash(RadioShareBuffer,CommonShareBufferLen);
            SetEvTask(EV_SHOW_TEXT);
            #ifdef USE_NAVILCD
            NaviLcdShowPlatformText(RadioShareBuffer,CommonShareBufferLen);
            #endif
        }
        ////////////////////////
        if(PUBLIC_CHECKBIT(flag, 3))
        {
          /*dxl,2014.8.8根据黎工提交修改
            #ifdef TTS_M12
            PlayMulTTSVoiceBuff(pBuffer+1,BufferLen-1,M12_TTS_RE_NO);
            #else
            PlayMulTTSVoiceBuff(pBuffer+1,BufferLen-1,0);
            #endif
          */
            PlayMulTTSVoiceBuff(pBuffer+1,BufferLen-1,0);
 
        }
        ///////////////////
        if(PUBLIC_CHECKBIT(flag, 4))//平台需要修改
        {
            CommonShareBufferBusyFlag = 0;
        }

    }
    return RadioProtocol_TerminalGeneralAck(channel,AckResult);
}
/*********************************************************************
//函数名称	:RadioProtocol_EvenSet(u8 *pBuffer, u16 BufferLen)
//功能		:事件设置
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:指令ID：0x8301
*********************************************************************/
ProtocolACK RadioProtocol_EvenSet(u8 channel, u8 *pBuffer, u16 BufferLen)
{
	u8 AckResult = 0;
	
        AckResult=EventReport_DisposeRadioProtocol(pBuffer,BufferLen);
         //无实际动作，只有应答
        return RadioProtocol_TerminalGeneralAck(channel,AckResult);
}
/*********************************************************************
//函数名称	:RadioProtocol_QuestionDownload(u8 *pBuffer, u16 BufferLen)
//功能		:提问下发
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:指令ID：0x8302
*********************************************************************/
ProtocolACK RadioProtocol_QuestionDownload(u8 channel, u8 *pBuffer, u16 BufferLen)
{
    u8 AckResult = 0;
    QuestionAckSerialNum = RadioProtocolRxHead.SerialNum;//获取提问应答流水号  
    Question_DisposeRadioProtocol(pBuffer,BufferLen);
    return RadioProtocol_TerminalGeneralAck(channel,AckResult);
}
/*********************************************************************
//函数名称	:RadioProtocol_InformationOrderSet(u8 *pBuffer, u16 BufferLen)
//功能		:信息点播菜单设置
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:指令ID：0x8303
*********************************************************************/
ProtocolACK RadioProtocol_InformationOrderSet(u8 channel, u8 *pBuffer, u16 BufferLen)
{
	u8 AckResult;
    	
	//无实际动作，只有应答
	AckResult = InfoDemand_DisposeRadioProtocol(pBuffer,BufferLen);
	return RadioProtocol_TerminalGeneralAck(channel,AckResult);
}
/*********************************************************************
//函数名称	:RadioProtocol_InformationService(u8 *pBuffer, u16 BufferLen)
//功能		:信息服务
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:指令ID：0x8304
*********************************************************************/
ProtocolACK RadioProtocol_InformationService(u8 channel, u8 *pBuffer, u16 BufferLen)
{
    u8	AckResult;
    //////////////////////// 
    AckResult=InfoService_DisposeRadioProtocol(pBuffer,BufferLen);
    return RadioProtocol_TerminalGeneralAck(channel,AckResult);
}
/*********************************************************************
//函数名称	:RadioProtocol_TelephoneCallback(u8 *pBuffer, u16 BufferLen)
//功能		:电话回拨
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:指令ID：0x8400
*********************************************************************/
ProtocolACK RadioProtocol_TelephoneCallback(u8 channel, u8 *pBuffer, u16 BufferLen)
{
	u8	AckResult;
	u8	*p;
	u8	flag;
	s16	length;
	u8	Buffer[30];
	u8	PramLen;


	
	p = pBuffer;
	length = BufferLen;

	flag = *p++;
	length--;
	
	if((length < 0)||(length > 20))
	{
		AckResult = 2;
	}
	else
	{
		AckResult = 0;
		
		//读取回拨类型
		//PhoneCallBackType = flag;
	
		//把电话号码写入eeprom
		EepromPram_WritePram(E2_PHONE_CALLBACK_ID, p, length);
		
		//写入标志位
		EepromPram_WritePram(E2_CALLBACK_FLAG_ID, &flag, 1);
		
		//读电话号码
		PramLen = EepromPram_ReadPram(E2_PHONE_CALLBACK_ID, Buffer);
		
		//读 标志
		PramLen = EepromPram_ReadPram(E2_CALLBACK_FLAG_ID, Buffer);

                if(PramLen < 20)
                {

                }
	
		//触发电话回拨任务
		SetEvTask(EV_SEV_CALL);
                
                BdTestFlag = 6;
		BdTestCount1 = 0;
                BdTestCount2 = 0;
		
	}
	//应答
	return RadioProtocol_TerminalGeneralAck(channel,AckResult);
}
/*********************************************************************
//函数名称	:RadioProtocol_TelephoneBookSet(u8 *pBuffer, u16 BufferLen)
//功能		:设置电话本
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:指令ID：0x8401
*********************************************************************/
ProtocolACK RadioProtocol_TelephoneBookSet(u8 channel, u8 *pBuffer, u16 BufferLen)
{
	u8	AckResult = 0;
	
	AckResult=PhoneBook_DisposeRadioProtocol(pBuffer,BufferLen);
	return RadioProtocol_TerminalGeneralAck(channel,AckResult);
}
/*********************************************************************
//函数名称	:RadioProtocol_CarControl(u8 *pBuffer, u16 BufferLen)
//功能		:车辆控制
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:指令ID：0x8500
*********************************************************************/
ProtocolACK RadioProtocol_CarControl(u8 channel, u8 *pBuffer, u16 BufferLen) 
{
    u8 ControlFlag;
    u8 AckResult;
    ControlFlag = *pBuffer;
    if(PUBLIC_CHECKBIT(ControlFlag,6))
    {
        if(PUBLIC_CHECKBIT(ControlFlag,7))//断油
        {
            OilControl_OffOil();
        }
        else//通油
        {
            OilControl_OpenOil();
        }
    }
    else
    if(0x01 == (ControlFlag & 0x01))
    {
        //触发车门加锁任务
        //CtrlLock(1);
    }
    else
    {
        //触发车门解锁任务
        //CtrlLock(0);
    }
    //记下车辆控制应答流水号
    if(MyTimerTask[TIME_DELAY_TRIG].TaskTimerState == DISABLE)
    {
        CarControlAckSerialNum = RadioProtocolRxHead.SerialNum;
        DelayAckCommand = 0x0500;
        DelaySendChannel = uart2_recdat.rec_chanel;
        if(CHANNEL_SMS == DelaySendChannel)
        {
            memcpy(DelaySmsPhone,gSmsRecord.msgParam.TPA,16);
        }
        //触发延时任务
        SetTimerTask(TIME_DELAY_TRIG, 100);//2秒后应答,主要是为了确保锁车/解锁后的状态能采集到
        //只有应答
        AckResult = 0;
        return RadioProtocol_TerminalGeneralAck(channel,AckResult);
    }
    return ACK_OK;
}
/*********************************************************************
//函数名称	:RadioProtocol_SetRoundArea(u8 *pBuffer, u16 BufferLen)
//功能		:设置圆形区域
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:指令ID：0x8600
*********************************************************************/
ProtocolACK RadioProtocol_SetRoundArea(u8 channel, u8 *pBuffer, u16 BufferLen)
{
u8 AckResult;
    
//只有应答
AckResult = Area_SetRoundArea(pBuffer,BufferLen);
return RadioProtocol_TerminalGeneralAck(channel,AckResult);
}
/*********************************************************************
//函数名称	:RadioProtocol_DelRoundArea(u8 *pBuffer, u16 BufferLen)
//功能		:删除圆形区域
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:指令ID：0x8601
*********************************************************************/
ProtocolACK RadioProtocol_DelRoundArea(u8 channel, u8 *pBuffer, u16 BufferLen) //SSSS
{
u8 AckResult;
 
//只有应答
AckResult = Area_DelRoundArea(pBuffer,BufferLen);
return RadioProtocol_TerminalGeneralAck(channel,AckResult);
}
/*********************************************************************
//函数名称	:RadioProtocol_SetRectangleArea(u8 *pBuffer, u16 BufferLen)
//功能		:设置矩形区域
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:指令ID：0x8602
*********************************************************************/
ProtocolACK RadioProtocol_SetRectangleArea(u8 channel, u8 *pBuffer, u16 BufferLen)
{
  u8 AckResult;
  //u16 AreaAtribute;
  //u32 temp;
  //u8  Buffer[5];

/*
  //行标送检修改，显示测试项目编号
  AreaAtribute = (*(pBuffer+6) << 8)|*(pBuffer+7);
  if((AreaAtribute&0x8000)==0x8000)
  {
            BdTestFlag = 9;
            BdTestCount1 = 0;
            BdTestCount2 = 0;
            Area_ClearRectArea();//清除矩形区域,清除之前的区域,以免有两个及以上区域存在
            ClrRoute();//清除之前的路线
  }
  else if((AreaAtribute&0x4000)==0x4000)
  {
            BdTestFlag = 15;
            BdTestCount1 = 0;
            BdTestCount2 = 0;
            Report_EraseBlindArea();//清除盲区
            Area_ClearRectArea();//清除矩形区域
            ClrRoute();//清除之前的路线
            
            //设置ACC 上报时间间隔为1秒
            temp = 1;
	    Buffer[0] = (temp & 0xff000000) >> 24;
	    Buffer[1] = (temp & 0xff0000) >> 16;
	    Buffer[2] = (temp & 0xff00) >> 8;
	    Buffer[3] = temp;
	    EepromPram_WritePram(E2_ACCON_REPORT_TIME_ID, Buffer, E2_ACCON_REPORT_TIME_LEN);
            EepromPram_UpdateVariable(E2_ACCON_REPORT_TIME_ID);
            
            //关闭不相干的任务
        ClrTimerTask(TIME_TRACK_RECORD);
        ClrTimerTask(TIME_CAN_SEND);
        ClrTimerTask(TIME_CAMERA);
        ClrTimerTask(TIME_SPEED_MONITOR);
        ClrTimerTask(TIME_TIRED_DRIVE);
        ClrTimerTask(TIME_DOUBT_POINT);
       
      
        ClrTimerTask(TIME_RECORD);
       
        ClrTimerTask(TIME_PULSE_ADJUST);
        ClrTimerTask(TIME_IC_CARD);
        ClrTimerTask(TIME_ICAUC);
         ClrTimerTask(TIME_GPS_UPDATE);
         
        ClrTimerTask(TIME_BMA220);//22:碰撞报警
        ClrTimerTask(TIME_PULSE);//23:脉冲速度
        ClrTimerTask(TIME_USB);//25:USB
        ClrTimerTask(TIME_TTSDELAY);//26: TTS
        ClrTimerTask(TIME_SLEEP);//28:休眠
        ClrTimerTask(TIME_SPEED_LOG);//31:速度异常记录
        ClrTimerTask(TIME_POWER_ON_LOG);//32:供电记录
        ClrTimerTask(TIME_MEDIA);//39:删除多媒体文件
        ClrTimerTask(TIME_ROUTE);//41:线路任务
        ClrTimerTask(TIME_GNSS_REAL);  //53：北斗实时解析数据
        ClrTimerTask(TIME_GNSS_APP);//54:北斗发送数据到平台任务
            
  }
 */
  
//只有应答
AckResult = Area_SetRectArea(pBuffer,BufferLen);
return RadioProtocol_TerminalGeneralAck(channel,AckResult);
}
/*********************************************************************
//函数名称	:RadioProtocol_DelRectangleArea(u8 *pBuffer, u16 BufferLen)
//功能		:删除矩形区域
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:指令ID：0x8603
*********************************************************************/
ProtocolACK RadioProtocol_DelRectangleArea(u8 channel, u8 *pBuffer, u16 BufferLen) //SSSS
{
u8 AckResult;
//只有应答
AckResult = Area_DelRectArea(pBuffer,BufferLen);
return RadioProtocol_TerminalGeneralAck(channel,AckResult);
}
/*********************************************************************
//函数名称	:RadioProtocol_SetPolygonArea(u8 *pBuffer, u16 BufferLen)
//功能		:设置多边形区域
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:指令ID：0x8604
*********************************************************************/
ProtocolACK RadioProtocol_SetPolygonArea(u8 channel, u8 *pBuffer, u16 BufferLen)
{
    u8	AckResult;
    AckResult = Polygon_SetArea(pBuffer,BufferLen);
    return RadioProtocol_TerminalGeneralAck(channel,AckResult);
}
/*********************************************************************
//函数名称	:RadioProtocol_DelPolygonArea(u8 *pBuffer, u16 BufferLen)
//功能		:删除多边形区域
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:指令ID：0x8605
*********************************************************************/
ProtocolACK RadioProtocol_DelPolygonArea(u8 channel, u8 *pBuffer, u16 BufferLen)
{
    u8	AckResult;
    AckResult = Polygon_DeleteArea(pBuffer,BufferLen);
    return RadioProtocol_TerminalGeneralAck(channel,AckResult);
}
/*********************************************************************
//函数名称	:RadioProtocol_SetRoad(u8 *pBuffer, u16 BufferLen)
//功能		:设置路线
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:指令ID：0x8606
*********************************************************************/
ProtocolACK RadioProtocol_SetRoad(u8 channel, u8 *pBuffer, u16 BufferLen)
{
    u8	AckResult = 0;               //返回值
    u8 flag = 0;
    if(BufferLen==0)
    {
        return RadioProtocol_TerminalGeneralAck(channel,2);//消息有误
    }
    else
    if(PUBLIC_CHECKBIT(RadioProtocolRxHead.Attribute,13)&&((RadioProtocolRxHead.CurrentPackage > RadioProtocolRxHead.TotalPackage) || (RadioProtocolRxHead.CurrentPackage==0)))
    {
        return RadioProtocol_TerminalGeneralAck(channel,2);//消息有误
    }
    else
    if(PUBLIC_CHECKBIT(RadioProtocolRxHead.Attribute,13)&&(RadioProtocolRxHead.TotalPackage>=ROUTE_NODE_SUM))
    {
        return RadioProtocol_TerminalGeneralAck(channel,1);//失败
    }
    //////////////////////
    //从第一个包中获得线路信息    
    if(0==PUBLIC_CHECKBIT(RadioProtocolRxHead.Attribute, 13))
    {
        flag = 0;
    }
    else
    if(1==RadioProtocolRxHead.CurrentPackage)//第一包
    {
        flag = 1;
    }
    else
    {
        flag = 2;
    }
    AckResult = Route_DisposeSetRoad(flag,pBuffer,BufferLen);
    //应答
    BdTestFlag = 10;
    BdTestCount1 = 0;
    BdTestCount2 = 0;
    /////////////////////
    return RadioProtocol_TerminalGeneralAck(channel,AckResult);
}
/*********************************************************************
//函数名称	:RadioProtocol_DelRoad(u8 *pBuffer, u16 BufferLen)
//功能		:删除路线
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:指令ID：0x8607
*********************************************************************/
ProtocolACK RadioProtocol_DelRoad(u8 channel, u8 *pBuffer, u16 BufferLen)
{
    u8	AckResult;   
    if(BufferLen==0)
    {
        return FORMAT_ERROR;
    }           
    //只有应答
    AckResult = Route_DisposeDelRoad(pBuffer,BufferLen);
    return RadioProtocol_TerminalGeneralAck(channel,AckResult);
}
/*********************************************************************
//函数名称	:RadioProtocol_SetMuckDumpArea
//功能		:设置渣土倾倒区域(0x860a)
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:指令ID：0x860a
*********************************************************************/
ProtocolACK RadioProtocol_SetMuckDumpArea(u8 channel, u8 *pBuffer, u16 BufferLen)
{
    u8	AckResult=0;
    return RadioProtocol_TerminalGeneralAck(channel,AckResult);
}
/*********************************************************************
//函数名称	:RadioProtocol_DeleteMuckDumpArea
//功能		:删除渣土倾倒区域(0x860b)
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:指令ID：0x860b
*********************************************************************/
ProtocolACK RadioProtocol_DeleteMuckDumpArea(u8 channel, u8 *pBuffer, u16 BufferLen)
{
    u8 AckResult=0;
    return RadioProtocol_TerminalGeneralAck(channel,AckResult);
}
/*********************************************************************
//函数名称	:RadioProtocol_QueryMuckDumpArea(u8 *pBuffer, u16 BufferLen)
//功能		:查询渣土倾倒区域(0x860c)
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:指令ID：(0x860c)
*********************************************************************/
ProtocolACK RadioProtocol_QueryMuckDumpArea(u8 channel, u8 *pBuffer, u16 BufferLen)
{
    u16	length;
    u8	*Buffer;
    u8	SmsPhone[20];
    u8	PramLen = 0;
    MESSAGE_HEAD	head;

    length = 0;
    Buffer = RadioShareBuffer;    
    length += PramLen;
    //发送渣土倾倒区域信息
    //消息ID
    head.MessageID = 0x060c;
    head.Attribute = length;

    if(0 != RadioProtocol_SendCmd(channel,head, Buffer, length, SmsPhone))
    {
        return ACK_OK;
    }
    else
    {
        //添加到重发列表
        //RadioProtocol_AddCmdToResendList(RadioProtocolRxHead.SerialNum, head.MessageID, Buffer, length, 0);
        return ACK_ERROR;
    }
}
/*********************************************************************
//函数名称	:RadioProtocol_RecorderDataCollection(u8 *pBuffer, u16 BufferLen)
//功能		:行驶记录仪信息采集
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:指令ID：0x8700
*********************************************************************/
ProtocolACK RadioProtocol_RecorderDataCollection(u8 channel, u8 *pBuffer, u16 BufferLen)
{
 
    //读取应答流水号
    RecorderSerialNum = RadioProtocolRxHead.SerialNum;
    
    St_RecorderWire.Cmd = *pBuffer;//保存命令字
    St_RecorderWire.AckNum = RadioProtocolRxHead.SerialNum;//保存应答序号
    if((*(pBuffer+1) == 0xAA)&&(*(pBuffer+2) == 0x75))//平台下发的包含数据块
    {         
        if(St_RecorderWire.AddCnt == 0)
        {
            St_RecorderWire.DataBlock = 1;//置数据块标志
            St_RecorderWire.Len = (*(pBuffer+4)<<8)|(*(pBuffer+5));
            if(St_RecorderWire.Len)
            {
                Public_ConvertBCDToTime(&St_RecorderWire.Time_start,pBuffer+7);
                Public_ConvertBCDToTime(&St_RecorderWire.Time_end,pBuffer+13);
                St_RecorderWire.Max    = (*(pBuffer+19)<<8)|(*(pBuffer+20));
                St_RecorderWire.AddOverflg = 0;
                St_RecorderWire.MaxCnt = 0;
                St_RecorderWire.AllCnt = 
                RecorderWireSendNum(RecorderCmdToRegister(St_RecorderWire.Cmd),                            
                                                 St_RecorderWire.Time_start,
                                                 St_RecorderWire.Time_end,
                                                 &St_RecorderWire.AllRemainCnt);
                if(St_RecorderWire.AllCnt > St_RecorderWire.Max)
                {
                    St_RecorderWire.AllCnt = St_RecorderWire.Max;
                    St_RecorderWire.AddOverflg = 1;
                }
                St_RecorderWire.AddCnt = 1;
                #ifdef HUOYUN
                St_RecorderWire.HY_Cmd_SendNum = 1;
                St_RecorderWire.HY_Cmd_SendRemain = 0;
                #endif
            }
        }
        else
        {
            //正在传输数据包
        }
    }
    else//数据块无
    {
        St_RecorderWire.DataBlock = 0;//置数据块标志
        St_RecorderWire.Max = 1;
        St_RecorderWire.MaxCnt = 0;
        memset(&St_RecorderWire.Time_start,0,sizeof(TIME_T));
        memset(&St_RecorderWire.Time_end,0,sizeof(TIME_T));
        if(((St_RecorderWire.Cmd >= 0x10)&&(St_RecorderWire.Cmd <= 0x15))
            ||(St_RecorderWire.Cmd == 0x08)||(St_RecorderWire.Cmd == 0x09))
        {
            RTC_GetCurTime(&St_RecorderWire.Time_start);
            St_RecorderWire.Time_start.year--;
            RTC_GetCurTime(&St_RecorderWire.Time_end);
            St_RecorderWire.Time_end.year++;
        }    
        #if 0
        if(St_RecorderWire.Cmd <= 7)
        {
            St_RecorderWire.All_07 = 1;
            St_RecorderWire.Cmd = 0;
        }
        #endif
    }
    SetTimerTask(TIME_WIRELESS_RECORDER, SYSTICK_50ms);//触发无线记录仪定时任务
    return ACK_OK;
}



/*********************************************************************
//函数名称	:RadioProtocol_RecorderPramSet(u8 *pBuffer, u16 BufferLen)
//功能		:行驶记录仪参数下传
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:指令ID：0x8701
*********************************************************************/
/*
ProtocolACK RadioProtocol_RecorderPramSet(u8 *pBuffer, u16 BufferLen)
{
	//读取命令字
        RecorderCmd = *pBuffer;
		       
	if((*(pBuffer+1) == 0xAA)&&(*(pBuffer+2)) == 0x75)
	{
            RecorderProtocolFormatFlag = 1;
            
            memset(St_RecorderWire.Buf,0,sizeof(St_RecorderWire.Buf));
            St_RecorderWire.Cmd = *(pBuffer+3);
            St_RecorderWire.Len = (*(pBuffer+4)<<8)|(*(pBuffer+5));
            memcpy(St_RecorderWire.Buf,pBuffer+7,St_RecorderWire.Len);
            St_RecorderWire.AckNum = RadioProtocolRxHead.SerialNum;
            
            //触发无线记录仪定时任务
	    SetTimerTask(TIME_WIRELESS_RECORDER, 5*SYSTICK_0p1SECOND);
	}
	else
	{
	    RecorderProtocolFormatFlag = 0;
	}
	
        //读取应答流水号
	RecorderSerialNum = RadioProtocolRxHead.SerialNum;
	
	return ACK_OK;
}
*/
ProtocolACK RadioProtocol_RecorderPramSet(u8 channel, u8 *pBuffer, u16 BufferLen)
{
    //读取应答流水号
    RecorderSerialNum = RadioProtocolRxHead.SerialNum;
    
    St_RecorderWire.Cmd = *pBuffer;//保存命令字
    St_RecorderWire.AckNum = RadioProtocolRxHead.SerialNum;//保存应答序号
    if((*(pBuffer+1) == 0xAA)&&(*(pBuffer+2) == 0x75))//平台下发的包含数据块
    {
        St_RecorderWire.DataBlock = 1;//置数据块标志
        St_RecorderWire.Len = (*(pBuffer+4)<<8)|(*(pBuffer+5));        
        memcpy(St_RecorderWire.Buf,pBuffer+7,St_RecorderWire.Len);
    }
    else//数据块无
    {
        St_RecorderWire.DataBlock = 0;//置数据块标志
        St_RecorderWire.Max = 0;
        St_RecorderWire.Len = 0;
        memset(&St_RecorderWire.Time_start,0,sizeof(TIME_T));
        memset(&St_RecorderWire.Time_end,0,sizeof(TIME_T));
    }
    SetTimerTask(TIME_WIRELESS_RECORDER, SYSTICK_50ms);//触发无线记录仪定时任?
    return ACK_OK;
}



/*********************************************************************
//函数名称	:RadioProtocol_MultiMediaUploadAck(u8 *pBuffer, u16 BufferLen)
//功能		:多媒体数据上传应答
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:指令ID：0x8800
*********************************************************************/
ProtocolACK RadioProtocol_MultiMediaUploadAck(u8 channel, u8 *pBuffer, u16 BufferLen)
{
  u8 *p = NULL ;
  u8  HighByte ;
  u8  LowByte ;
  u8  i ;

  if(4 == BufferLen)//成功接收到完整的多媒体数据
  {
      g_sttSubPacketTransterCtrl.FirstSN = 0;
      g_sttSubPacketTransterCtrl.PacketIDCntTotal = 0 ;
      g_sttSubPacketTransterCtrl.PacketIDCntCursor = 0 ;
      for( i = 0 ; i < SUBPACKET_ID_LIST_SIZE_MEDIA ; i++ )
      {
          g_sttSubPacketTransterCtrl.PacketIDArray[i] = 0 ;
      }
      g_sttSubPacketTransterCtrl.MediaType = 0 ;
      return ACK_OK;
	}
	else//有丢包发生
	{
    
   if(DISABLE == MyTimerTask[TIME_SUBPACKAGE].TaskTimerState)//正在分包补传时不相应当前补传请求 
   {
      p = pBuffer ;
      
      //提出-原始消息流水号
      HighByte = *p++;
      LowByte = *p++;
      g_sttSubPacketTransterCtrl.FirstSN = (HighByte<<8)|LowByte;
      
      //提出-重传包总数
      g_sttSubPacketTransterCtrl.PacketIDCntTotal = *p++;
      if( g_sttSubPacketTransterCtrl.PacketIDCntTotal >= SUBPACKET_ID_LIST_SIZE_MEDIA )
      {
          g_sttSubPacketTransterCtrl.PacketIDCntTotal = SUBPACKET_ID_LIST_SIZE_MEDIA-1;
      }

      //提出-重传包ID列表
      for( i  = 0 ; i < g_sttSubPacketTransterCtrl.PacketIDCntTotal ; i++ )
      {
          HighByte = *p++;
          LowByte = *p++;
          g_sttSubPacketTransterCtrl.PacketIDArray[i] = (HighByte<<8)|LowByte;
      }
                
      g_sttSubPacketTransterCtrl.PacketIDCntCursor = 0 ;
      g_sttSubPacketTransterCtrl.ResendCnt = 0 ;
                
      if( 0 != g_sttSubPacketTransterCtrl.PacketIDCntTotal )
      {
          g_sttSubPacketTransterCtrl.MediaType = MEDIT_TYPE_PIC ;
          //开启分包补传任务
          SetTimerTask(TIME_SUBPACKAGE,2*SECOND);
      }
      //开启补包重传任务
   }
   return ACK_ERROR;
   
	}
}
/*********************************************************************
//函数名称	:RadioProtocol_CameraPhoto(u8 *pBuffer, u16 BufferLen)
//功能		:摄像头立即拍摄
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:指令ID：0x8801
*********************************************************************/
ProtocolACK RadioProtocol_CameraPhoto(u8 channel, u8 *pBuffer, u16 BufferLen)
{
	//u8	AckResult;
	u8	StoreFlag;
	u16	temp;
 
        // ClrTimerTask(TIME_SEND_MEDIA);//行标检测时暂停多媒体数据传输,dxl
        // memset(MemoryUploadFileName,0,33);
                
        //读取应答流水号
	RecorderSerialNum = RadioProtocolRxHead.SerialNum;
        //读取拍照命令（张数）
        temp  = (*(pBuffer+1)<<8)|*(pBuffer+2); 
        MediaCommand=temp;
        //MediaID++;//多媒体ID，
	//u8	MediaIDBuffer[5] = {0};
	//MediaIDBuffer[0] = PhotoID << 24;
	//MediaIDBuffer[1] = PhotoID << 16;
	//MediaIDBuffer[2] = PhotoID << 8;
	//MediaIDBuffer[3] = PhotoID;
	//FRAM_BufferWrite(FRAM_MEDIA_ID_ADDR,MediaIDBuffer,4);
        MediaType = 0;//多媒体类型，0图像，1音频，2视频
        MediaFormat = 0;//多媒体格式编码,0JPEG,1TIF,2MP3,3WAV,4WMV
        MediaChannelID = *pBuffer;//多媒体通道ID
        MediaPhotoSpace =(*(pBuffer+3)<<8)|*(pBuffer+4); 
        MediaEvenCode = 0;//多媒体事件编码
	StoreFlag = *(pBuffer+5);//保存标志
		

        //EepromPram_WritePram(E2_MEDIA_SAVE_FLAG_ID, &MediaSaveFlag, 1);
        temp=*(pBuffer+6); //图象分辨率
        if(temp>2 || temp==0)
        {
             temp=1;
        }
        MediaRatio=temp;      
	//触发拍照,参数顺序CameraID,PhotoNum,PhotoSpace,Resolution,StoreFlag, PhotoType,EventType
	
        if(0 == StoreFlag)
        {
            StoreFlag = 0x01;//bit0为1表示需要上传
        }
        else if(1 == StoreFlag)
        {
            StoreFlag = 0x02;//bit1为1表示需要保存
        }
		Camera_Photo(channel,MediaChannelID,MediaCommand,MediaPhotoSpace,MediaRatio,StoreFlag,0,0);//行标检测时使用这行即可,上一行Camera_Photo可去掉
        //BdTestFlag = 11;
        //BdTestCount1 = 0;
        //BdTestCount2 = 0;
        DelayAckCommand = 0x8801;
	//触发延时任务
    	SetTimerTask(TIME_DELAY_TRIG, 1);//2秒后发送拍照应答指令
	//AckResult = 0;
	//return RadioProtocol_TerminalGeneralAck(AckResult);通用应答不需要,需要应答0x0805
        return ACK_OK;
}
/*********************************************************************
//函数名称	:RadioProtocol_MemoryIndex(u8 *pBuffer, u16 BufferLen)
//功能		:存储多媒体数据检索
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:指令ID：0x8802
*********************************************************************/
ProtocolACK RadioProtocol_MemoryIndex(u8 channel, u8 *pBuffer, u16 BufferLen)
{
	u8	MediaType;//多媒体类型
	u8	ChannelID;//通道ID,0为检索全部通道
	u8	EvCode;//事件编码
	u8	HighBcd;//BCD码的高半字节
	u8	LowBcd;//BCD码的低半字节
	u8	*p;
    u16   length;
	TIME_T	StartTime;
	TIME_T	EndTime;
    
	MediaType = *pBuffer;
	ChannelID = *(pBuffer+1);
        EvCode = *(pBuffer+2);
	//**********为编译不产生警告而添加**********
	if(0 == MediaType)
	{
	
	}
	if(0 == ChannelID)
	{
	
	}
	if(0 == EvCode)
	{
	
	}
	p = pBuffer+3;
	//****************获取开始时间***********************
	HighBcd = ((*p) & 0xf0)>>4;
	LowBcd = ((*p) & 0x0f);
	p++;
	StartTime.year = HighBcd*10+LowBcd;
	HighBcd = ((*p) & 0xf0)>>4;
	LowBcd = ((*p) & 0x0f);
	p++;
	StartTime.month = HighBcd*10+LowBcd;
	HighBcd = ((*p) & 0xf0)>>4;
	LowBcd = ((*p) & 0x0f);
	p++;
	StartTime.day = HighBcd*10+LowBcd;
	HighBcd = ((*p) & 0xf0)>>4;
	LowBcd = ((*p) & 0x0f);
	p++;
	StartTime.hour = HighBcd*10+LowBcd;
	HighBcd = ((*p) & 0xf0)>>4;
	LowBcd = ((*p) & 0x0f);
	p++;
	StartTime.min = HighBcd*10+LowBcd;
	HighBcd = ((*p) & 0xf0)>>4;
	LowBcd = ((*p) & 0x0f);
	p++;
	StartTime.sec = HighBcd*10+LowBcd;
	
	//****************获取结束时间***********************
	HighBcd = ((*p) & 0xf0)>>4;
	LowBcd = ((*p) & 0x0f);
	p++;
	EndTime.year = HighBcd*10+LowBcd;
	HighBcd = ((*p) & 0xf0)>>4;
	LowBcd = ((*p) & 0x0f);
	p++;
	EndTime.month = HighBcd*10+LowBcd;
	HighBcd = ((*p) & 0xf0)>>4;
	LowBcd = ((*p) & 0x0f);
	p++;
	EndTime.day = HighBcd*10+LowBcd;
	HighBcd = ((*p) & 0xf0)>>4;
	LowBcd = ((*p) & 0x0f);
	p++;
	EndTime.hour = HighBcd*10+LowBcd;
	HighBcd = ((*p) & 0xf0)>>4;
	LowBcd = ((*p) & 0x0f);
	p++;
	EndTime.min = HighBcd*10+LowBcd;
	HighBcd = ((*p) & 0xf0)>>4;
	LowBcd = ((*p) & 0x0f);
	p++;
	EndTime.sec = HighBcd*10+LowBcd;
	
	length = MediaSearch_GetMediaIndexInformation(RadioShareBuffer+4, MediaType, &StartTime, &EndTime);
  
	//应答流水号
	RadioShareBuffer[0] = (RadioProtocolRxHead.SerialNum & 0xff00) >> 8;
	RadioShareBuffer[1] = RadioProtocolRxHead.SerialNum;
	//总项数
	RadioShareBuffer[2] = 0;
	RadioShareBuffer[3] = length/35;
	length += 4;
	return RadioProtocol_MemoryIndexAck(channel,RadioShareBuffer, length);
}
/*********************************************************************
//函数名称	:RadioProtocol_MemoryUpload(u8 *pBuffer, u16 BufferLen)
//功能		:存储多媒体数据上传
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:指令ID：0x8803
*********************************************************************/
ProtocolACK RadioProtocol_MemoryUpload(u8 channel, u8 *pBuffer, u16 BufferLen)
{
	
	u8	AckResult;
	
	//只有应答,先暂时调试通
	AckResult = 0;
	return RadioProtocol_TerminalGeneralAck(channel,AckResult);
	
}
/*********************************************************************
//函数名称	:RadioProtocol_StartSoundRecord(u8 *pBuffer, u16 BufferLen)
//功能		:开始录音
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:指令ID：0x8804
*********************************************************************/
ProtocolACK RadioProtocol_StartSoundRecord(u8 channel, u8 *pBuffer, u16 BufferLen)
{
	
  	u8	AckResult;
	u8	RecordCmd;//录音命令,1为开始录音,0为结束录音
	u16	RecordTime;//录音时间,0为一直录音
	u8	RecordSaveFlag;//录音保存标志,1为保存,0为实时上传
	u8	RecordSample;//录音采样频率,0:8K,1:11K,2:23K,3:32K
	
	RecordCmd = *pBuffer;
	RecordTime = *(pBuffer+1) << 8 | *(pBuffer+2);
	RecordSaveFlag = *(pBuffer+3);
	RecordSample = *(pBuffer+4);
        
        
         //ClrTimerTask(TIME_SEND_MEDIA);//行标测试时暂停存储多媒体数据传输,dxl
         //memset(MemoryUploadFileName,0,33);
         
	if(1 == RecordCmd)
	{
		//开启录音
		Io_WriteRecordConrtolBit(RECORD_BIT_TIME, SET);
		StartRecordTime(RecordTime); 
	}
	else if(0 == RecordCmd)
	{
		Io_WriteRecordConrtolBit(RECORD_BIT_TIME, RESET);//只停止平台自己开启的录音功能
	}
	//********以下是为了编译不提示警告*********
	if(0 == RecordTime)
	{
	
	}
	if(0 == RecordSaveFlag)
	{
		RecordSendFlag = 1;//录音实时上传标志
	}
	if(0 == RecordSample)
	{
	
	}
        //BdTestFlag = 12;//行标测试时使用
        //BdTestCount1 = 0;
        //BdTestCount2 = 0;
	//*********************end******************
	//只有应答,先暂时调试通
	AckResult = 0;
	return RadioProtocol_TerminalGeneralAck(channel,AckResult);
}
/*********************************************************************
//函数名称	:RadioProtocol_SingleMemoryIndex(u8 *pBuffer, u16 BufferLen)
//功能		:单条存储多媒体数据检索上传
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:指令ID：0x8805
*********************************************************************/
ProtocolACK RadioProtocol_SingleMemoryIndex(u8 channel, u8 *pBuffer, u16 BufferLen)
{
    	u8	AckResult;
  
	u32	MediaID;
	u8	DelFlag;
	u8	*p;
        
	u8	FileName[33];
	u8	length;
	//static u8 count = 0;
	
	
	p = pBuffer;
	//读取多媒体ID
	MediaID = 0;
	MediaID |= *p << 24;
	p++;
	MediaID |= *p << 16;
	p++;
	MediaID |= *p << 8;
	p++;
	MediaID |= *p;
	p++;
	//删除标志
	DelFlag = *p;
	if(DelFlag)//为了编译不报错
	{
	
	}
	
	///count++;
	//if(1 == count%2)
	//{
		//获得对应多媒体ID的文件名
		MediaSearch_GetMediaIDFileName(FileName,MEDIA_TYPE_JPG, MediaID);
	//}
	//else
	//{
		//获得对应多媒体ID的文件名
		//MediaSearch_GetMediaIDFileName(FileName,MEDIA_TYPE_WAV, MediaID);
		
	//}
	length = strlen((char const *)FileName);
	if(0 != length)
	{
		if(0 == strlen((char const *)MemoryUploadFileName))//正忙
		{
			strcpy((char *)MemoryUploadFileName,(char const *)FileName);
			SetTimerTask(TIME_SEND_MEDIA, 15*SYSTICK_0p1SECOND);
		}
		else//正忙
		{	
			
		}
	}
	else
	{
			
	}
  
        /*
        
        p = pBuffer;
	//读取多媒体ID
	MediaID = 0;
	MediaID |= *p << 24;
	p++;
	MediaID |= *p << 16;
	p++;
	MediaID |= *p << 8;
	p++;
	MediaID |= *p;
	p++;
	//删除标志
	DelFlag = *p;
	if(DelFlag)//为了编译不报错
	{
	
	}
        //if(MediaID == (PhotoID-1))
        //{
                MultiMediaIndexSendFlag = 1;
                //触发拍照,参数顺序CameraID,PhotoNum,PhotoSpace,Resolution,StoreFlag, PhotoType,EventType
	        Camera_Photo(1,1,20000,1,0,0,0);
        //}
	//通用应答
	//只有应答,先暂时调试通
        */
        BdTestFlag = 14;
        BdTestCount1 = 0;
        BdTestCount2 = 0;
	AckResult = 0;
	return RadioProtocol_TerminalGeneralAck(channel,AckResult);
	
}
/*********************************************************************
//函数名称	:RadioProtocol_JiutongPhoto(u8 *pBuffer, u16 BufferLen)
//功能		:摄像头立即拍摄(九通协议)
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:指令ID：0x8811
*********************************************************************/
ProtocolACK RadioProtocol_JiutongPhoto(u8 channel, u8 *pBuffer, u16 BufferLen)
{
	u8	AckResult;  
	u8	MediaChannelID;
        MediaChannelID = *pBuffer;//多媒体通道ID
    
	Camera_Photo(channel,MediaChannelID,1,0,1,0x01,1,0);//平台拍照1张作事件拍照处理
	
	AckResult = 0;
	return RadioProtocol_TerminalGeneralAck(channel,AckResult);
} 
/*********************************************************************
//函数名称	:RadioProtocol_OriginalDataDownTrans(u8 *pBuffer, u16 BufferLen)
//功能		:数据下行透传
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:指令ID：0x8900
*********************************************************************/
ProtocolACK RadioProtocol_OriginalDataDownTrans(u8 channel, u8 *pBuffer, u16 BufferLen)
{
    u8 AckResult;
    u16     length;
    u8 cmd;
    //只有应答,先暂时调试
    AckResult = 0;
    if ((BufferLen > 1023)||(BufferLen <= 1))
    {
        return ACK_ERROR;
    }

    u8 flag;
    flag=*pBuffer;
    //////////////////////////
    #ifdef USE_EIEXPAND_PROTOCOL
    if(EIEXPAND_PROTOCOL_TRAN_TYPE == flag)//伊爱扩展协议透传类型
    {
        EIExpand_RadioProtocolParse(pBuffer+1,BufferLen-1);
    }
    else
    #endif
    if(0x0B == flag) //IC卡认证中心下发的认证密钥信息
    {
        //memcpy( gICCard_Radio_Buffer, (u8 *)pBuffer, BufferLen);
        //gICCard_Radio_Buffer_Valid = 1; //置1 表明有效
        //       SetTimerTask( TIME_IC_CARD , SYSTICK_0p1SECOND );//0.1s	
        //拷贝认证请求应答数据1byte的状态位+ 24bytes认证信息
		ICCard_AuthenticationAck(pBuffer+1, BufferLen);
		return ACK_OK;
    }
		else if(0xf2 == flag)//广州二汽读卡模块属性
		{
			return ICCard_CustomProtocolParse(pBuffer+1, BufferLen-1);
		}
    else if((0x41 == flag)||(0x42 == flag))
    {
#ifdef HUOYUN_DEBUG_OPEN
            Recorder_SendData((u8 *)(pBuffer+1), (BufferLen-1));//从DB9口发送出来
#else
            //0x41串口1透传对应EGS701GB的32PIN485接口,0x42串口2透传对应EGS701GB的16PIN232接口,dxl,2014.3.14
            if(1 == *(pBuffer+1))//0x01表示透传的是久通油耗数据
            {
                Uart1DataDownTransFlag = 1;
                Uart1DataDownTransCmd = *(pBuffer+5);//取久通机电油耗传感器协议的命令字节
                JointechOiLCost_COM_Buf_Write(pBuffer+2,BufferLen-2);
            }
#endif
    }
    else if(0 == flag)//CAN模块数据透传
    {
            if(0x60 == *(pBuffer+1))
            {
                cmd = *(pBuffer+1);
                Can_ErQi_SendCmd(cmd,pBuffer+2,6);
            }
    }
    else 
    {
        //拷贝数据到中间缓冲
        length = 0;
        length |= *(pBuffer+5) << 8;
        length |= *(pBuffer+6);
        pBuffer++; //跳过类型
        memcpy(RadioShareBuffer,pBuffer, BufferLen-1);
        //触发CAN发送数据 2011.07.14 Wanj
        length = BufferLen-1;
        if(length > 100) {
        memcpy(CanData.data, RadioShareBuffer, 100); 
        length = 100;
        }else {
        memcpy(CanData.data, RadioShareBuffer, length);
        }
        CanData.len = length;
        CanData.snd = 0;
        SetTimerTask(TIME_CAN_SEND, SYSTICK_1SECOND);
    }
    return RadioProtocol_TerminalGeneralAck(channel,AckResult);
}

/*********************************************************************
//函数名称	:RadioProtocol_ServerRSA(u8 *pBuffer, u16 BufferLen)
//功能		:平台公钥
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:指令ID：0x8A00
*********************************************************************/
ProtocolACK RadioProtocol_ServerRSA(u8 channel, u8 *pBuffer, u16 BufferLen)
{
	u32	RSAServerE;
	//u8	RSAServerN[128];
	u8	temp;
	u8	*p;
	//u8	AckResult;
        u32     RSATerminalE;
        u8     RSATerminalN[128] = {1,2,3,4,5,6,7,8,9,10};

	p = pBuffer;


	//获取平台e值
	RSAServerE = 0;
	temp = *p++;
	RSAServerE |= temp << 24;
	temp = *p++;
	RSAServerE |= temp << 16;
	temp = *p++;
	RSAServerE |= temp << 8;
	temp = *p++;
	RSAServerE |= temp;
	//获取平台n值
	//memcpy(RSAServerN,p,128);

	//获取终端e值
        RSATerminalE = 1;

        return RadioProtocol_TerminalRSA(channel,RSATerminalE, RSATerminalN);
}
//*****************第二部分：标准的终端上发指令**********************
/*********************************************************************
//函数名称	:RadioProtocol_TerminalGeneralAck(u16 SerialNum, u16 MessageID, u8 Result)
//功能		:终端通用应答
//输入		:Result:应答结果,0为成功，1为失败，2消息有误，3不支持
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:成功,返回ACK_OK;失败返回ACK_ERROR;
//备注		:指令ID：0x0001
*********************************************************************/
ProtocolACK RadioProtocol_TerminalGeneralAck(u8 channel, u8 AckResult)
{

	u8	Buffer[5];
	u8	length;
	u8	SmsPhone[20];
	MESSAGE_HEAD	head;
	
	//消息ID
	head.MessageID = 0x0001;
	//消息属性
	length = 5;
	head.Attribute = length;
	//消息内容
	Buffer[0] = (RadioProtocolRxHead.SerialNum & 0xff00) >> 8;
	Buffer[1] = RadioProtocolRxHead.SerialNum;
	Buffer[2] = (RadioProtocolRxHead.MessageID & 0xff00) >> 8;
	Buffer[3] = RadioProtocolRxHead.MessageID;
	Buffer[4] = AckResult;
	
	if(0 != RadioProtocol_SendCmd(channel,head, Buffer, length, SmsPhone))
	{
		
	}
    else
	{
         //添加到重发列表
        //RadioProtocol_AddCmdToResendList(RadioProtocolRxHead.SerialNum, head.MessageID, Buffer, length, 0);
		return ACK_ERROR;
    }
    return ACK_OK;
}
/*********************************************************************
//函数名称	:RadioProtocol_TerminalRegister(void)
//功能		:终端注册
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:  成功,返回ACK_OK;失败返回ACK_ERROR;
//备注		:指令ID：0x0100
*********************************************************************/
ProtocolACK RadioProtocol_TerminalRegister(u8 channel)
{

	u8	*p;
	u8	*q;
	u8	Buffer[150];
	u8	Buffer2[30];
	u8	Buffer3[50];
	u8	PramLen;
	u8	SmsPhone[20];
	u16	length;
	u8	i;
	u8	BcdHigh;
	u8	BcdLow;
	u8	value;
	MESSAGE_HEAD	head;
	u32	ID;
	p = Buffer;
	length = 0;
	
	

	//读取省域ID
	PramLen = EepromPram_ReadPram(E2_CAR_PROVINCE_ID, Buffer2);
	if(2 == PramLen)
	{
		*p++ = Buffer2[0];
		*p++ = Buffer2[1];
		length += 2;
	}
	else
	{
		*p++ = 0;
		*p++ = 0;
		length += 2;
	}
	

	//读取市域ID
	PramLen = EepromPram_ReadPram(E2_CAR_CITY_ID, Buffer2);
	if(2 == PramLen)
	{
		*p++ = Buffer2[0];
		*p++ = Buffer2[1] & 0xff;
		length += 2;
	}
	else
	{
		*p++ = 0;
		*p++ = 0;
		length += 2;
	}

	//读取生产商ID
	PramLen = EepromPram_ReadPram(E2_MANUFACTURE_ID, Buffer2);
	if(5 == PramLen)
	{
		memcpy(p,Buffer2,5);
		p += 5;
		length += 5;
	}
	else
	{
		memcpy(p,"70108",5);
		p += 5;
		length += 5;
	}
	
	
	//读取终端型号
	for(i=0; i<20; i++)
	{
		Buffer2[i] = 0;
	}
#ifdef HUOYUN_DEBUG_OPEN
        Buffer2[0] = 'E';
        Buffer2[1] = 'G';
        Buffer2[2] = 'S';
        Buffer2[3] = '7';
        Buffer2[4] = '0';
        Buffer2[5] = '1';
        Buffer2[6] = 'G';
        Buffer2[7] = 'B';
#else
	if(E2_CAR_ONLY_NUM_ID_LEN == EepromPram_ReadPram(E2_CAR_ONLY_NUM_ID, Buffer3))//货运联调，终端型号补充字符只能是0x00，不能是空格，否则全国货运平台不认，提示数据库无此终端
	{
		for(i=0; i<16; i++)
		{
			if(Buffer3[7+i] < 128)//判断是否为ASCII码,前面7个字节为3C编号
			{
				Buffer2[i] = Buffer3[7+i];
			}
			else
			{
				break;
			}
		}
	}
	else
	{
		memcpy(Buffer2,TerminalTypeID,6);//如果没有设置过唯一性编号，产品型号默认为EGS701
		
	}
#endif
	memcpy(p,Buffer2,20);
	p += 20;
	length += 20;

	//读取终端ID
	PramLen = EepromPram_ReadPram(E2_DEVICE_ID, Buffer2);
	if(7 == PramLen)
	{
	
		//取终端ID后面7位作为登录注册的终端ID
		q = Buffer2;
		q += 3;
		value = *q++;
		BcdHigh = (value & 0xF0)>>4;
		BcdLow = value & 0x0F;
		Buffer3[0] = BcdLow + '0';
		value = *q++;
		BcdHigh = (value & 0xF0)>>4;
		BcdLow = value & 0x0F;
		Buffer3[1] = BcdHigh + '0';
		Buffer3[2] = BcdLow + '0';
		value = *q++;
		BcdHigh = (value & 0xF0)>>4;
		BcdLow = value & 0x0F;
		Buffer3[3] = BcdHigh + '0';
		Buffer3[4] = BcdLow + '0';
		value = *q++;
		BcdHigh = (value & 0xF0)>>4;
		BcdLow = value & 0x0F;
		Buffer3[5] = BcdHigh + '0';
		Buffer3[6] = BcdLow + '0';
		memcpy(p,Buffer3,7);
		p += 7;
		length += 7;
	}
	else
	{
		//读取备份的终端ID号
		PramLen = EepromPram_ReadPram(E2_DEVICE_BACKUP_ID, Buffer2);
		if(7 == PramLen)
		{
			//检查是之前hex码(2011.12.16之前10位ID号)还是BCD码(2011.12.16之后13位ID号)
			if((0 == Buffer2[0])&&(0 == Buffer2[1]))//对应V1.0的硬件版本
			{
			ID = 0;
			ID |= Buffer2[3] << 24;
			ID |= Buffer2[4] << 16;
			ID |= Buffer2[5] << 8;
			ID |= Buffer2[6];
			Buffer3[0] = ID%10000000/1000000+'0';
			Buffer3[1] = ID%1000000/100000+'0';
			Buffer3[2] = ID%100000/10000+'0';
			Buffer3[3] = ID%10000/1000+'0';
			Buffer3[4] = ID%1000/100+'0';
			Buffer3[5] = ID%100/10+'0';
			Buffer3[6] = ID%10+'0';
			memcpy(p,Buffer3,7);
			p += 7;
			length += 7;
			}
			else
			{
			//取终端ID后面7位作为登录注册的终端ID
			q = Buffer2;
			q += 3;
			value = *q++;
			BcdHigh = (value & 0xF0)>>4;
			BcdLow = value & 0x0F;
			Buffer3[0] = BcdLow + '0';
			value = *q++;
			BcdHigh = (value & 0xF0)>>4;
			BcdLow = value & 0x0F;
			Buffer3[1] = BcdHigh + '0';
			Buffer3[2] = BcdLow + '0';
			value = *q++;
			BcdHigh = (value & 0xF0)>>4;
			BcdLow = value & 0x0F;
			Buffer3[3] = BcdHigh + '0';
			Buffer3[4] = BcdLow + '0';
			value = *q++;
			BcdHigh = (value & 0xF0)>>4;
			BcdLow = value & 0x0F;
			Buffer3[5] = BcdHigh + '0';
			Buffer3[6] = BcdLow + '0';
			memcpy(p,Buffer3,7);
			p += 7;
			length += 7;
			}
		}
		else
		{
			//固定ID
			Buffer3[0] = '2';
			Buffer3[1] = '0';
			Buffer3[2] = '1';
			Buffer3[3] = '2';
			Buffer3[4] = '0';
			Buffer3[5] = '0';
			Buffer3[6] = '1';
			memcpy(p,Buffer3,7);
			p += 7;
			length += 7;
		}
	}
	
	//读取车牌颜色
	PramLen = EepromPram_ReadPram(E2_CAR_PLATE_COLOR_ID, Buffer2);
        if(1 == PramLen)
        {
                *p++ = Buffer2[0];
	        length++;

        }
        else
        {

                *p++ = 0;
                length++;
        }
	
#ifdef HUOYUN_DEBUG_OPEN
        if(0 == Buffer2[0])//读取VIN,用VIN码注册
        {
                PramLen = EepromPram_ReadPram(E2_CAR_IDENTIFICATION_CODE_ID, Buffer2);
                if((PramLen < 20)&&(0 != PramLen))
                {
                      memcpy(p,Buffer2,PramLen);
	              length += PramLen;
                }
                else
                {
                      memcpy(p,"VIN123456789",12);
	              length += 12;
                }
        }
        else
#endif
        {
	        //读取车牌,用车牌号码注册
	        PramLen = EepromPram_ReadPram(E2_CAR_PLATE_NUM_ID, Buffer2);
                if((PramLen < 20)&&(0 != PramLen))
                {
                      memcpy(p,Buffer2,PramLen);
	              length += PramLen;
                      //p += PramLen;
                      //*p = 0;
                      //length++;
                }
                else
                {
                      memcpy(p,"粤b012345",9);
	              length += 9;
                      // p += 9;
                      //*p = 0;
                      //length++;
                }
	}

	//消息ID
	head.MessageID = 0x0100;
	//消息属性
	head.Attribute = (length & 0x3ff);
	
	//读取注册短消息号码
	
	
	if(0 != RadioProtocol_SendCmd(channel,head, Buffer, length, SmsPhone))
	{
		return ACK_OK;
	}
	else
	{
		return ACK_ERROR;
	}
	
}
/*********************************************************************
//函数名称	:RadioProtocol_FirmwareUpdataResultNote(u8 *pBuffer, u16 BufferLen)
//功能		:远程固件升级结果通知
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:  成功,返回ACK_OK;失败返回ACK_ERROR;
//备注		:指令ID：0x0108
*********************************************************************/
ProtocolACK RadioProtocol_FirmwareUpdataResultNote(u8 channel, u8 *pBuffer, u16 BufferLen)
{
        
	u8	SmsPhone[20];
	MESSAGE_HEAD	head;
	
        if(2 != BufferLen)
        {
                return ACK_ERROR;
        }
	//消息ID
	head.MessageID = 0x0108;
	//消息属性
	head.Attribute = BufferLen;
	//消息内容
	if(0 != RadioProtocol_SendCmd( channel,head, pBuffer, BufferLen,SmsPhone))
	{
		
	}
        else
	{
                //添加到重发列表
                //RadioProtocol_AddCmdToResendList(RadioProtocolRxHead.SerialNum, head.MessageID, Buffer, length, 0);
		return ACK_ERROR;
        }
         return ACK_OK;
}
/*********************************************************************
//函数名称	:RadioProtocol_OpenAccount(void)
//功能		:终端开户
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:  成功,返回ACK_OK;失败返回ACK_ERROR;
//备注		:指令ID：0x0110
*********************************************************************/
/*行标北斗过检时使用该函数
ProtocolACK RadioProtocol_OpenAccount(void)
{
    u8  Buffer[100]={0};
    u8  temp[30];
    u8  temp1[10];
    u8  *p = NULL;
    u8  *q = NULL;
    s16 length;
    u8  PramLen;
    u8  BcdHigh;
    u8  BcdLow;
    u8  value;
    u8  i;  
    u8	SmsPhone[20];
    u8	channel = 0;
    MESSAGE_HEAD	head;

    p = Buffer;
    length = 0;

    //加密标识,未加密
    *p++ = 0;
    length++;
    //加密密钥,默认为0
    *p++ = 0;
    *p++ = 0;
    *p++ = 0;
    *p++ = 0;
    length += 4;
    //车主手机号
    //从eeprom中读取车主手机号
    PramLen =Public_ReadDataFromFRAM(FRAM_CAR_OWNER_PHONE_ADDR, temp,FRAM_CAR_OWNER_PHONE_LEN);//add by joneming 20130619
    if(FRAM_CAR_OWNER_PHONE_LEN == PramLen)
    {
        memcpy(p,temp,FRAM_CAR_OWNER_PHONE_LEN);
        p += FRAM_CAR_OWNER_PHONE_LEN;
        length += FRAM_CAR_OWNER_PHONE_LEN;
    }
    else
    {
        memcpy(p,"000000000000",FRAM_CAR_OWNER_PHONE_LEN);
        p += FRAM_CAR_OWNER_PHONE_LEN;
        length += FRAM_CAR_OWNER_PHONE_LEN;
    }
    //读取省域ID
    PramLen = EepromPram_ReadPram(E2_CAR_PROVINCE_ID, temp);
    if(E2_CAR_PROVINCE_ID_LEN == PramLen)
    {
        memcpy(p,temp,E2_CAR_CITY_ID_LEN);
        p += E2_CAR_CITY_ID_LEN;
        length += E2_CAR_CITY_ID_LEN;
    }
    else
    {
        *p++ = 0;
        *p++ = 0;
        length += 2;
    }

    //读取市域ID
    PramLen = EepromPram_ReadPram(E2_CAR_CITY_ID,temp);
    if(E2_CAR_CITY_ID_LEN == PramLen)
    {
        memcpy(p,temp,E2_CAR_CITY_ID_LEN);
        p += E2_CAR_CITY_ID_LEN;
        length += E2_CAR_CITY_ID_LEN;
    }
    else
    {
        *p++ = 0;
        *p++ = 0;
        length += 2;
    }

    //读取制造商ID
    PramLen = EepromPram_ReadPram(E2_MANUFACTURE_ID, temp);
    if(E2_MANUFACTURE_LEN == PramLen)
    {
        memcpy(p,temp,E2_MANUFACTURE_LEN);
        p += E2_MANUFACTURE_LEN;
        length += E2_MANUFACTURE_LEN;
    }
    else
    {
        memcpy(p,"70108",5);
        p += 5;
        length += 5;
    }
    //读取终端型号
    temp[0] = 'E';
    temp[1] = 'G';
    temp[2] = 'S';
    temp[3] = '7';
    temp[4] = '0';
    temp[5] = '1';
    temp[6] = 'B';
    for(i=0; i<13; i++)
    {
        temp[7+i] = 0;
    }
    memcpy(p,temp,20);
    p += 20;
    length += 20;
    //读取终端ID
    PramLen = EepromPram_ReadPram(E2_DEVICE_ID, temp);
    if(E2_DEVICE_ID_LEN == PramLen)
    {
        //取终端ID后面7位作为登录注册的终端ID
        q = temp;
        q += 3;
        value = *q++;
        BcdHigh = (value & 0xF0)>>4;
        BcdLow = value & 0x0F;
        temp1[0] = BcdLow + '0';
        value = *q++;
        BcdHigh = (value & 0xF0)>>4;
        BcdLow = value & 0x0F;
        temp1[1] = BcdHigh + '0';
        temp1[2] = BcdLow + '0';
        value = *q++;
        BcdHigh = (value & 0xF0)>>4;
        BcdLow = value & 0x0F;
        temp1[3] = BcdHigh + '0';
        temp1[4] = BcdLow + '0';
        value = *q++;
        BcdHigh = (value & 0xF0)>>4;
        BcdLow = value & 0x0F;
        temp1[5] = BcdHigh + '0';
        temp1[6] = BcdLow + '0';
        memcpy(p,temp1,7);
        p += 7;
        length += 7;

    }
    else
    {
        //读取备份的终端ID号
        PramLen = EepromPram_ReadPram(E2_DEVICE_BACKUP_ID, temp);
        if(7 == PramLen)
        {

            //取终端ID后面7位作为登录注册的终端ID
            q = temp;
            q += 3;
            value = *q++;
            BcdHigh = (value & 0xF0)>>4;
            BcdLow = value & 0x0F;
            temp1[0] = BcdLow + '0';
            value = *q++;
            BcdHigh = (value & 0xF0)>>4;
            BcdLow = value & 0x0F;
            temp1[1] = BcdHigh + '0';
            temp1[2] = BcdLow + '0';
            value = *q++;
            BcdHigh = (value & 0xF0)>>4;
            BcdLow = value & 0x0F;
            temp1[3] = BcdHigh + '0';
            temp1[4] = BcdLow + '0';
            value = *q++;
            BcdHigh = (value & 0xF0)>>4;
            BcdLow = value & 0x0F;
            temp1[5] = BcdHigh + '0';
            temp1[6] = BcdLow + '0';
            memcpy(p,temp1,7);
            p += 7;
            length += 7;

        }
        else
        {
            //固定ID
            temp1[0] = '2';
            temp1[1] = '0';
            temp1[2] = '1';
            temp1[3] = '2';
            temp1[4] = '0';
            temp1[5] = '0';
            temp1[6] = '1';
            memcpy(p,temp1,7);
            p += 7;
            length += 7;
        }
    }
    //读取车牌颜色
    PramLen = EepromPram_ReadPram(E2_CAR_PLATE_COLOR_ID, temp);
    if(1 == PramLen)
    {
        *p++ = temp[0];
        length++;
    }
    else
    {

        *p++ = 0;
        length++;
    }

    //读取车牌号码
    PramLen = EepromPram_ReadPram(E2_CAR_PLATE_NUM_ID, temp);
    if((PramLen < 10)&&(0 != PramLen))
    {
        memcpy(p,temp,PramLen);
        length += PramLen;
        p += PramLen; 
        for(i=0; i<9-PramLen; i++)//不足9位后补0x00
        {
        *p++ = 0;
        length++;
        }
    }
    else
    {
        memcpy(p,"粤B12345",8);
        length += 8;
        p += 8;
        *p++ = 0;
        length++;
    }
    //读取车辆VIN码
    PramLen = EepromPram_ReadPram(E2_CAR_IDENTIFICATION_CODE_ID, temp);
    if(0 != PramLen)
    {
        memcpy(p,temp,PramLen);
        length += PramLen;
        p += PramLen; 
        *p++ = 0;
        length++;
    }
    //消息ID
    head.MessageID = 0x0110;
    //消息属性
    head.Attribute = (length & 0x3ff);

    //读取注册短消息号码

    if(0 == RadioProtocol_SendCmd(channel,head, Buffer, length, SmsPhone))
    {
        return ACK_OK;
    }
    else
    {
        return ACK_ERROR;
    }

}
*/
//全国货运平台联调使用该函数，不知与行标北斗送检有无差异，能否兼容？？
ProtocolACK RadioProtocol_OpenAccount(u8 channel)
{
    u8  Buffer[100]={0};
    u8  temp[30];
    u8  temp1[20];
    s16 length;
    u8  PramLen; 
    u8	SmsPhone[20];
    u32 key;
    u8 cryptflag;
    MESSAGE_HEAD	head;

    key       = 0;
    length    = 0;    
    cryptflag = 0;
    if(OpenAccount_GetEncryptFlag())
    {
        PramLen =EepromPram_ReadPram(E2_MANUFACTURE_ID, temp);//制造商ID
        if(E2_MANUFACTURE_LEN == PramLen)
        {
            key = atoi((char *)temp);
            if(key)//
            cryptflag = 1;
        }
    }
    //加密标识,
    Buffer[length++] = cryptflag;
    //加密密钥,
    Public_ConvertLongToBuffer(key,&Buffer[length]);
    length += 4;
    //车主手机号
    //从eeprom中读取车主手机号
    PramLen =Public_ReadDataFromFRAM(FRAM_CAR_OWNER_PHONE_ADDR, temp,FRAM_CAR_OWNER_PHONE_LEN);//add by joneming 20130619
    if(FRAM_CAR_OWNER_PHONE_LEN == PramLen)
    {
        memcpy(&Buffer[length],temp,FRAM_CAR_OWNER_PHONE_LEN);
    }
    else
    {
        memcpy(&Buffer[length],"000000000000",FRAM_CAR_OWNER_PHONE_LEN);
    }
    length += FRAM_CAR_OWNER_PHONE_LEN;
    ///////////////////////
    //读取省域ID
    PramLen = EepromPram_ReadPram(E2_CAR_PROVINCE_ID, temp);
    if(E2_CAR_PROVINCE_ID_LEN == PramLen)
    {
        memcpy(&Buffer[length],temp,E2_CAR_PROVINCE_ID_LEN); 
    }    
    length += E2_CAR_PROVINCE_ID_LEN;
    ////////////////
    //读取市域ID
    PramLen = EepromPram_ReadPram(E2_CAR_CITY_ID,temp);
    if(E2_CAR_CITY_ID_LEN == PramLen)
    {
        memcpy(&Buffer[length],temp,E2_CAR_CITY_ID_LEN);
    }    
    length += E2_CAR_CITY_ID_LEN;
    /////////////
    //读取制造商ID
    PramLen = EepromPram_ReadPram(E2_MANUFACTURE_ID, temp);
    if(E2_MANUFACTURE_LEN == PramLen)
    {
        memcpy(&Buffer[length],temp,E2_MANUFACTURE_LEN);
    }
    else
    {
        memcpy(&Buffer[length],"70108",5);
    }
    length += E2_MANUFACTURE_LEN;
    //////////////////////
    //读取终端型号
    memset(temp,0,20);
    GetCurentProductModel((char *)temp);
    memcpy(&Buffer[length],temp,20);
    length += 20;
    //读取终端ID
    PramLen = EepromPram_ReadPram(E2_DEVICE_ID, temp);
    if(E2_DEVICE_ID_LEN == PramLen)//取终端ID后面7位作为登录注册的终端ID
    {
        Public_ConvertBcdToAsc(temp1,&temp[3],4);
    }
    else
    {
        //读取备份的终端ID号
        PramLen = EepromPram_ReadPram(E2_DEVICE_BACKUP_ID, temp);
        if(7 == PramLen)//取终端ID后面7位作为登录注册的终端ID
        { 
            Public_ConvertBcdToAsc(temp1,&temp[3],4);        
        }
        else//固定ID
        {
            memset(temp1,0,10);
            strcpy((char *)&temp1[1],"3100028");
        }
    }
    ///读取终端ID///////////////
    memcpy(&Buffer[length],&temp1[1],7);
    length += 7;
    ///////////////
    //读取车牌颜色
    PramLen = EepromPram_ReadPram(E2_CAR_PLATE_COLOR_ID, temp);
    if(1 != PramLen)
    {
        temp[0] = 0;
    }
    Buffer[length++] = temp[0];
    //读取车牌号码
    PramLen = EepromPram_ReadPram(E2_CAR_PLATE_NUM_ID, temp);
    if((PramLen < 10)&&(0 != PramLen))
    {
        memcpy(&Buffer[length],temp,PramLen);
    }
    else
    {
        memcpy(&Buffer[length],"粤B12345",8);
    }
    length += 9;
    //读取车辆VIN码
    PramLen = EepromPram_ReadPram(E2_CAR_IDENTIFICATION_CODE_ID, temp);
    if(0 != PramLen)
    {
        memcpy(&Buffer[length],temp,PramLen);
        length += PramLen;
        Buffer[length++] = 0;
    }
    ///////////////////
    if(cryptflag)
    Encrypt(key,&Buffer[5],length-5);
    //消息ID
    head.MessageID = 0x0110;
    //消息属性
    head.Attribute = (length & 0x3ff);

    //读取注册短消息号码

    if(0 != RadioProtocol_SendCmd(channel,head, Buffer, length, SmsPhone))
    {
        return ACK_OK;
    }
    else
    {
        return ACK_ERROR;
    }

}
/*********************************************************************
//函数名称	:RadioProtocol_TerminalHeartbeat()
//功能		:终端心跳
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:   成功,返回ACK_OK;失败返回ACK_ERROR;
//备注		:指令ID：0x0002
*********************************************************************/
ProtocolACK RadioProtocol_TerminalHeartbeat(u8 channel)
{
	u8	Buffer[3] = {0};
	u8	SmsPhone[20];
	MESSAGE_HEAD	head;

	//消息ID
	head.MessageID = 0x0002;
	//消息属性
	head.Attribute = 0;
	if(0 != RadioProtocol_SendCmd(channel,head, Buffer, 0, SmsPhone))
	{
		return ACK_OK;
	}
	else
	{
		return ACK_ERROR;
	}
}
/*********************************************************************
//函数名称	:RadioProtocol_TerminalLogout(void)
//功能		:终端注销
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:   成功,返回ACK_OK;失败返回ACK_ERROR;
//备注		:指令ID：0x0003
*********************************************************************/
ProtocolACK RadioProtocol_TerminalLogout(u8 channel)
{
	u8	Buffer[3] = {0};
	u8	SmsPhone[20];
	MESSAGE_HEAD	head;


        
#ifdef HUOYUN_DEBUG_OPEN
        if(0 != LogoutFlag)
        {
             return ACK_ERROR;
        }
#endif
 
        
	//消息ID
	head.MessageID = 0x0003;
	//消息属性
	head.Attribute = 0;
	//读注销短息号码
	
	if(0 != RadioProtocol_SendCmd(channel,head, Buffer, 0, SmsPhone))
	{
                LogoutFlag = 1;
		return ACK_OK;
	}
	else
	{
		return ACK_ERROR;
	}
}
/*********************************************************************
//函数名称	:RadioProtocol_TerminalQualify(void)
//功能		:终端鉴权
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:   成功,返回ACK_OK;失败返回ACK_ERROR;
//备注		:指令ID：0x0102
*********************************************************************/
ProtocolACK RadioProtocol_TerminalQualify(u8 channel)
{
	u8	Buffer[20] = {"abced"};
	u8	length;
	u8	SmsPhone[20];
	MESSAGE_HEAD	head;

	//获取鉴权码
    if(CHANNEL_DATA_1 == channel)
    {
	    length = EepromPram_ReadPram(E2_LOGINGPRS_PASSWORD_ID, Buffer);
    }
    else if(CHANNEL_DATA_2 == channel)
    {
        length = EepromPram_ReadPram(E2_SECOND_LOGINGPRS_PASSWORD_ID, Buffer);
    }
    else
    {
        return ACK_ERROR;
    }
	
	//消息ID
	head.MessageID = 0x0102;
	//消息属性
	head.Attribute = (length & 0x3ff);

	if(0 != RadioProtocol_SendCmd(channel,head, Buffer, length, SmsPhone))
	{
		return ACK_OK;
	}
	else
	{
		return ACK_ERROR;
	}
}
/*********************************************************************
//函数名称	:RadioProtocol_PostionInformationReport(u8 *pBuffer, u16 length)
//功能		:位置信息汇报
//输入		:pBuffer:指向数据（位置信息）指针;BufferLen:数据的长度;
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:    成功,返回ACK_OK;失败返回ACK_ERROR;
//备注		:指令ID：0x0200
*********************************************************************/
ProtocolACK  RadioProtocol_PostionInformationReport(u8 channel, u8 *pBuffer, u16 BufferLen)
{
	MESSAGE_HEAD	head;
	u8	SmsPhone[20];
	u16	length;
        //u8      byte;
        //u32     temp;
       // u8      ACC;

	/*行标检测时使用
	//判断长度
	if(1 == RecordSendFlag)//上传录音时也会上传位置信息
        {
        
        }
        else if((BufferLen >= 1024)||(0 != MultiMediaSendFlag))
	{
		return ACK_ERROR;
	}
	*/
	if(BufferLen >= 1024)
	{
		return ACK_ERROR;
	}
	length = BufferLen;
	//消息ID
	head.MessageID = 0x0200;
	//消息属性
	head.Attribute = (BufferLen & 0x3ff);
        //检查数据正确性
	/*
        //检查经纬度加密标志
        byte = *(pBuffer+7);
        if((byte&0x20) == 0x20)
        {
                return ACK_ERROR;
        }
        //检查海拔高度
        temp = 0;
        byte = *(pBuffer+16);
        temp |= byte << 8;
        byte = *(pBuffer+17);
        temp |= byte;
        if(temp > 10000)
        {
                return ACK_ERROR;
        }
        //检查速度
        temp = 0;
        byte = *(pBuffer+18);
        temp |= byte << 8;
        byte = *(pBuffer+19);
        temp |= byte;
        if(temp > 10000)
        {
                return ACK_ERROR;
        }
        //检查方向
        temp = 0;
        byte = *(pBuffer+20);
        temp |= byte << 8;
        byte = *(pBuffer+21);
        temp |= byte;
        if(temp > 359)
        {
                return ACK_ERROR;
        }
	*/
	if(0 != RadioProtocol_SendCmd(channel,head, pBuffer, length, SmsPhone))
	{
		return ACK_OK;
	}
	else
	{
		return ACK_ERROR;
	}
}
/*********************************************************************
//函数名称	:RadioProtocol_EvenReport(u8 EvenID)
//功能		:事件报告
//输入		:EvenID:事件ID
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:    成功,返回ACK_OK;失败返回ACK_ERROR;
//备注		:指令ID：0x0301
*********************************************************************/
ProtocolACK RadioProtocol_EvenReport(u8 channel, u8 EvenID)
{
	
	MESSAGE_HEAD	head;
	u16	length;
	u8	Buffer[2];
	u8	SmsPhone[20];
	//消息ID
	head.MessageID = 0x0301;
	//消息属性
	length =  1;
	head.Attribute = (length & 0x3ff);
	//消息内容
	Buffer[0] = EvenID;

	if(0 != RadioProtocol_SendCmd(channel,head, Buffer, length, SmsPhone))
	{
		return ACK_OK;
	}
	else
	{
		return ACK_ERROR;
	}
}
/*********************************************************************
//函数名称	:RadioProtocol_QuestionAck(u8 AnswerID)
//功能		:提问应答
//输入		:AnswerID:应答ID
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:  成功,返回ACK_OK;失败返回ACK_ERROR;
//备注		:指令ID：0x0302
*********************************************************************/
ProtocolACK RadioProtocol_QuestionAck(u8 channel, u8 AnswerID)
{
	
	MESSAGE_HEAD	head;
	u16	length;
	u8	Buffer[3];
	u8	SmsPhone[20];
	//消息ID
	head.MessageID = 0x0302;
	//消息属性
	length =  3;
	head.Attribute = (length & 0x3ff);
	//消息内容
	Buffer[0] = QuestionAckSerialNum >> 8;
	Buffer[1] = QuestionAckSerialNum;
	Buffer[2] = AnswerID;
	
	if(0 != RadioProtocol_SendCmd(channel,head, Buffer, length, SmsPhone))
	{
		return ACK_OK;
	}
	else
	{
		return ACK_ERROR;
	}
}
/*********************************************************************
//函数名称	:RadioProtocol_InforamtionOrder(u8 InformationType, u8 ControlFlg)
//功能		:信息点播
//输入		:InformationType:信息类型;ControlFlg控制标志
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:   成功,返回ACK_OK;失败返回ACK_ERROR;
//备注		:指令ID：0x0303
*********************************************************************/
ProtocolACK RadioProtocol_InforamtionOrder(u8 channel, u8 InformationType, u8 ControlFlg)
{
	
	MESSAGE_HEAD	head;
	u16	length;
	u8	Buffer[2];
	u8	SmsPhone[20];
	//消息ID
	head.MessageID = 0x0303;
	//消息属性
	length =  2;
	head.Attribute = (length & 0x3ff);
	//消息内容
	Buffer[0] = InformationType;
	Buffer[1] = ControlFlg;

	if(0 != RadioProtocol_SendCmd(channel,head, Buffer, length, SmsPhone))
	{
		return ACK_OK;
	}
	else
	{
		return ACK_ERROR;
	}
}
/*********************************************************************
//函数名称	:RadioProtocol_RecorderDataReport
//功能		:行驶记录仪数据上传
//输入		:pBuffer:指向数据（运单信息）指针;BufferLen:数据长度
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:   成功,返回ACK_OK;失败返回ACK_ERROR;
//备注		:指令ID：0x0700
*********************************************************************/
ProtocolACK RadioProtocol_RecorderDataReport(u8 channel, u8 *pBuffer, u16 BufferLen, u16 TotalPacket, u16 Packet)
{
        MESSAGE_HEAD	head;
	u8	SmsPhone[20];
	u16	length;
	
	length = BufferLen;
	//消息ID
	head.MessageID = 0x0700;
	//消息属性
        if(TotalPacket > 1)
        {

               head.Attribute = 0x2000;
               head.Attribute |= BufferLen;
               head.TotalPackage = TotalPacket;
               head.CurrentPackage = Packet;
               SubpackageTotalPacket = TotalPacket;
        }
        else
        {
              head.Attribute = (BufferLen&0x3ff);
        }
	if(0 != RadioProtocol_SendCmd(channel,head, pBuffer, length, SmsPhone))
	{
		return ACK_OK;
	}
	else
	{
		return ACK_ERROR;
	}
}
/*********************************************************************
//函数名称	:RadioProtocol_ElectricReceiptReport(u8 *pBuffer, u8 BufferLen)
//功能		:电子运单上报
//输入		:pBuffer:指向数据（运单信息）指针;BufferLen:数据长度
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:   成功,返回ACK_OK;失败返回ACK_ERROR;
//备注		:指令ID：0x0701
*********************************************************************/
ProtocolACK RadioProtocol_ElectricReceiptReport(u8 channel, u8 *pBuffer, u16 BufferLen)
{
        MESSAGE_HEAD	head;
	u8	SmsPhone[20];
	u16	length;
	
	length = BufferLen;
	//消息ID
	head.MessageID = 0x0701;
	//消息属性
	head.Attribute = (BufferLen & 0x3ff);

	if(0 != RadioProtocol_SendCmd(channel,head, pBuffer, length, SmsPhone))
	{
		return ACK_OK;
	}
	else
	{
		return ACK_ERROR;
	}
}
/*********************************************************************
//函数名称	:RadioProtocol_DriverInformationReport(u8 *pBuffer, u8 BufferLen)
//功能		:驾驶员身份信息上报
//输入		:pBuffer:指向数据（驾驶员信息）指针;BufferLen:数据长度
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:   成功,返回ACK_OK;失败返回ACK_ERROR;
//备注		:指令ID：0x0702
*********************************************************************/
ProtocolACK RadioProtocol_DriverInformationReport(u8 channel, u8 *pBuffer, u8 BufferLen)
{
	
	MESSAGE_HEAD	head;
	u8	SmsPhone[20];
	u16	length;
	
	length = BufferLen;
	//消息ID
	head.MessageID = 0x0702;
	//消息属性
	head.Attribute = (BufferLen & 0x3ff);

	if(0 != RadioProtocol_SendCmd(channel,head, pBuffer, length, SmsPhone))
	{
		return ACK_OK;
	}
	else
	{
		return ACK_ERROR;
	}
}
/*********************************************************************
//函数名称	:RadioProtocol_MultiMediaEvenReport(u8 channel, u32 DataID, u8 Type, u8 Format, u8 Even, u8 ChID)
//功能		:多媒体事件信息上传
//输入		:DataID:多媒体数据ID;Type:多媒体类型;Format:多媒体格式编码;Even:事件项编码;ChID:通道编码
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:   成功,返回ACK_OK;失败返回ACK_ERROR;
//备注		:指令ID：0x0800
*********************************************************************/
ProtocolACK RadioProtocol_MultiMediaEvenReport(u8 channel, u32 DataID, u8 Type, u8 Format, u8 Even, u8 ChID)
{
	
	MESSAGE_HEAD	head;
	u16	length;
	u8	Buffer[10];
	u8	*p;
	u8	temp;
	u8	SmsPhone[20];
	
	p = Buffer;

	//消息ID
	head.MessageID = 0x0800;
	//消息属性
	length = 8;
	head.Attribute = (length & 0x3ff);
	//消息内容
	temp = (DataID & 0xff000000) >> 24;
	*p++ = temp;
	temp = (DataID & 0xff0000) >> 16;
	*p++ = temp;
	temp = (DataID & 0xff00) >> 8;
	*p++ = temp;
	temp = DataID & 0xff;
	*p++ = temp;
	*p++ = Type;
	*p++ = Format;
	*p++ = Even;
	*p++ = ChID;
    
	if(0 != RadioProtocol_SendCmd(channel,head, Buffer, length, SmsPhone))
	{
		return ACK_OK;
	}
	else
	{
		return ACK_ERROR;
	}
}
/*********************************************************************
//函数名称	:RadioProtocol_MultiMediaDataReport(u8 *pBuffer, u16 BufferLen)
//功能		:多媒体数据上传
//输入		:pBuffer:指向数据（需要包含多媒体数据ID;多媒体类型;多媒体格式编码;事件项编码;通道编码;数据总包数;
//		:包ID;多媒体数据包）的指针;
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:  成功,返回ACK_OK;失败返回ACK_ERROR;
//备注		:  指令ID：0x0801
*********************************************************************/
ProtocolACK RadioProtocol_MultiMediaDataReport(u8 channel, u8 *pBuffer, u16 BufferLen, u16 TotalPacket, u16 Packet)
{
	
	MESSAGE_HEAD	head;
	u8	SmsPhone[20];
	u16	length;
	
	length = BufferLen;
	//消息ID
	head.MessageID = 0x0801;
	//消息属性
        if(TotalPacket != 0)
        {

               head.Attribute = 0x2000;
               head.Attribute |= BufferLen;
               head.TotalPackage = TotalPacket;
               head.CurrentPackage = Packet;
        }
        else
        {
              head.Attribute = (BufferLen&0x3ff);
        }
	
	if(0 != RadioProtocol_SendCmd(channel,head, pBuffer, length, SmsPhone))
	{
		return ACK_OK;
	}
	else
	{
		return ACK_ERROR;
	}
}
/*********************************************************************
//函数名称	:RadioProtocol_MemoryIndexAck(u8 *pBuffer, u16 BufferLen)
//功能		:存储多媒体检索应答
//输入		:pBuffer:指向数据（需要包含多媒体ID;多媒体类型;通道ID;事件项编码;位置信息汇报;
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:  成功,返回ACK_OK;失败返回ACK_ERROR;
//备注		:  指令ID：0x0802
*********************************************************************/
ProtocolACK RadioProtocol_MemoryIndexAck(u8 channel, u8 *pBuffer, u16 BufferLen)
{
	
	MESSAGE_HEAD	head;
	u8	SmsPhone[20];
	
	//消息ID
	head.MessageID = 0x0802;
	//消息属性
	head.Attribute = BufferLen;
        //判断是否为短消息
	if(0 != RadioProtocol_SendCmd(channel,head, pBuffer, BufferLen, SmsPhone))
	{
	    	return ACK_OK;
	}
	else
	{
	    	return ACK_ERROR;
	}	
}
/*********************************************************************
//函数名称	:RadioProtocol_PhotoAck(u8 *pBuffer, u16 BufferLen)
//功能		:摄像头立即拍照应答
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:  成功,返回ACK_OK;失败返回ACK_ERROR;
//备注		:  指令ID：0x0805
*********************************************************************/
ProtocolACK RadioProtocol_PhotoAck(u8 channel, u8 *pBuffer, u16 BufferLen)
{
        MESSAGE_HEAD	head;
	u8	SmsPhone[20];
	
	//消息ID
	head.MessageID = 0x0805;
	//消息属性
	head.Attribute = BufferLen;
   
	if(0 != RadioProtocol_SendCmd(channel,head, pBuffer, BufferLen, SmsPhone))
	{
	    	return ACK_OK;
	}
	else
	{
	    	return ACK_ERROR;
	}	
}
/*********************************************************************
//函数名称	:RadioProtocol_OriginalDataUpTrans(u8 Type, u32 TotalLen, u8 *pBuffer, u16 BufferLen)
//功能		:数据上行透传
//输入		:pBuffer:指向数据（透传数据）的指针;BufferLen:当前透传数据包长度
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:    成功,返回ACK_OK;失败返回ACK_ERROR;
//备注		:指令ID：0x0900
*********************************************************************/
ProtocolACK RadioProtocol_OriginalDataUpTrans(u8 channel, u8 Type, u8 *pBuffer, u16 BufferLen)
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
    memcpy(buffer+1,pBuffer,BufferLen);
    buffer[0] = Type;
    length = BufferLen+1;
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

/*********************************************************************
//函数名称	:RadioProtocol_DataCompressUpTrans(u8 *pBuffer, u16 BufferLen)
//功能		:数据压缩上传
//输入		:pBuffer:指向数据（压缩数据）的指针;BufferLen:压缩数据长度
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:    成功,返回ACK_OK;失败返回ACK_ERROR;
//备注		:指令ID：0x0901
*********************************************************************/
ProtocolACK RadioProtocol_DataCompressUpTrans(u8 channel, u8 *pBuffer, u16 BufferLen)
{
	
	u8	temp;
	u16	length;
	u8	SmsPhone[20];
	u8	*p = NULL;
	u8	*p1 = NULL;
	u8	Buffer[1024];
	
	p = Buffer;
	p1 = p;
	if(NULL == p)
	{
		return ACK_ERROR;
	}
	//压缩数据长度
	temp = (BufferLen & 0xff000000) >> 24;
	*p++ = temp;
	temp = (BufferLen & 0xff0000) >> 16;
	*p++ = temp;
	temp = (BufferLen & 0xff00) >> 8;
	*p++ = temp;
	temp = BufferLen & 0xff;
	*p++ = temp;

	memcpy(p,pBuffer,BufferLen);
	p += BufferLen;
	length = BufferLen+4;

	MESSAGE_HEAD	head;

	//消息ID
	head.MessageID = 0x0901;
	//消息属性
	head.Attribute = (length & 0x3ff);

	if(0 != RadioProtocol_SendCmd(channel,head, p1, length, SmsPhone))
	{
		return ACK_OK;
	}
	else
	{
		return ACK_ERROR;
	}
}
/*********************************************************************
//函数名称	:RadioProtocol_TerminalRSA(u32 RSAe, u32 RSAn)
//功能		:终端RSA公钥
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:  成功,返回ACK_OK;失败返回ACK_ERROR;
//备注		:指令ID：0x0A00
*********************************************************************/
ProtocolACK RadioProtocol_TerminalRSA(u8 channel, u32 RSAe, u8 *pRSAn)
{
	
	u8	temp;
	u16	length;
	u8	Buffer[140];
	u8	SmsPhone[20];
	//存储e值
	temp = (RSAe & 0xff000000) >> 24;
	Buffer[0] = temp;
	temp = (RSAe & 0xff0000) >> 16;
	Buffer[1] = temp;
	temp = (RSAe & 0xff00) >> 8;
	Buffer[2] = temp;
	temp = RSAe & 0xff;
	Buffer[3] = temp;
	//存储n值
	memcpy(Buffer+4,pRSAn,128);

	length = 132;

	MESSAGE_HEAD	head;

	//消息ID
	head.MessageID = 0x0a00;
	//消息属性
	head.Attribute = (length & 0x3ff);

	if(0 != RadioProtocol_SendCmd(channel,head, Buffer, length, SmsPhone))
	{
		return ACK_OK;
	}
	else
	{
		return ACK_ERROR;
	}
}
//*****************第三部分：自定义的平台下发指令**********************
//*****************第四部分：自定义的终端上发指令**********************
//*****************第五部分：对外接口函数**********************
/*********************************************************************
//函数名称	:RadioProtocol_UpdateTerminalPhoneNum(void)
//功能		:更新终端电话号码
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:上电初始化时需调用
*********************************************************************/
void RadioProtocol_UpdateTerminalPhoneNum(void)
{
    u8  	Buffer[13];
	u8	BufferLen;

	BufferLen = EepromPram_ReadPram(E2_DEVICE_PHONE_ID, Buffer);
    if(E2_DEVICE_PHONE_ID_LEN == BufferLen)
    {
        memcpy(TerminalPhoneNumber,Buffer,E2_DEVICE_PHONE_ID_LEN);
    }
    else
    {
		//读手机号码备份区域
		BufferLen = EepromPram_ReadPram(E2_DEVICE_PHONE_BACKUP_ID, Buffer);
		if(E2_DEVICE_PHONE_BACKUP_ID_LEN == BufferLen)
		{
			memcpy(TerminalPhoneNumber,Buffer,E2_DEVICE_PHONE_BACKUP_ID_LEN);
		}
		else
		{
                    /*dxl,2014.4.29屏蔽，改为如下
			        TerminalPhoneNumber[0] = 0x01;
              		TerminalPhoneNumber[1] = 0x37;
              		TerminalPhoneNumber[2] = 0x60;
             	 	TerminalPhoneNumber[3] = 0x25;
              		TerminalPhoneNumber[4] = 0x00;
              		TerminalPhoneNumber[5] = 0x00;
                    */
            //读终端ID
            BufferLen = EepromPram_ReadPram(E2_DEVICE_ID, Buffer);
            if(E2_DEVICE_ID_LEN == BufferLen)
            {
                Buffer[1] = Buffer[1]&0x0f;//强制高4位为0
                memcpy(TerminalPhoneNumber,Buffer+1,E2_DEVICE_PHONE_BACKUP_ID_LEN);//取终端ID号的后面11位
            }
            else
            {
                TerminalPhoneNumber[0] = 0x01;
              	TerminalPhoneNumber[1] = 0x37;
              	TerminalPhoneNumber[2] = 0x60;
             	TerminalPhoneNumber[3] = 0x25;
              	TerminalPhoneNumber[4] = 0x00;
              	TerminalPhoneNumber[5] = 0x00;
            }
            
		}

    }
}
/*********************************************************************
//函数名称	:RadioProtocol_GetRadioProtocolRxBufferBusyFlag(void)
//功能		:读接收缓冲忙标志
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:通信模块接收到数据后需调用此函数,判断是否能往协议接收缓冲拷贝数据
//		:若返回值为0表示空闲可以往协议接收缓冲，为1表示正忙，当前数据丢弃
*********************************************************************/
u8  RadioProtocol_GetRadioProtocolRxBufferBusyFlag(void)
{
	
	return RadioProtocolRxBufferBusyFlag;
}
/*********************************************************************
//函数名称	:RadioProtocol_SetRadioProtocolRxBufferBusyFlag
//功能		:置位RadioProtocolRxBufferBusyFlag
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:当往协议接收缓冲拷贝完数据后需置位协议接收缓冲忙标准,
//		:只有等协议接收缓冲里的数据都被解析完后该标准由系统自动清0
*********************************************************************/
void RadioProtocol_SetRadioProtocolRxBufferBusyFlag(void)
{
	//置位忙标志
	RadioProtocolRxBufferBusyFlag = 1;
}
/*********************************************************************
//函数名称	:RadioProtocol_AddRadioParseList(void)
//功能		:添加内容到无线解析列表
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:当往协议接收缓冲拷贝完数据后需调用此函数,将新接收到的数据
//		:（可能同时收到好几条命令,所以要用一个表列）登记进解析列表,
//		:然后触发解析任务
*********************************************************************/
void RadioProtocol_AddRadioParseList(void)
{
	
}

/*********************************************************************
//函数名称	:RadioProtocol_AddRadioParseList(void)
//功能		:添加内容到无线解析列表
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:当往协议接收缓冲拷贝完数据后需调用此函数,将新接收到的数据
//		:（可能同时收到好几条命令,所以要用一个表列）登记进解析列表,
//		:然后触发解析任务
*********************************************************************/
void RadioProtocol_UpdateRadioParseList(void)
{
	
	
}

/*********************************************************************
//函数名称	:RadioProtocol_AddRecvDataForParse
//功能		:添加数据
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:当往协议接收缓冲拷贝完数据后需调用此函数,将新接收到的数据
//		:（可能同时收到好几条命令,所以要用一个表列）登记进解析列表,
//		:然后触发解析任务
*********************************************************************/
void RadioProtocol_AddRecvDataForParse(unsigned char channel,unsigned char *recvData,unsigned short recvDataLen)
{
    if(CHANNEL_SMS == channel)//短信通道
    {
        QueueInBuffer(recvData, recvDataLen, RadioProtocolRxSmsQ);
    }
    else if(CHANNEL_DATA_1 == channel)//数据通道连接1
    {
        QueueInBuffer(recvData, recvDataLen, RadioProtocolRx1Q);
    }
    else if(CHANNEL_DATA_2 == channel)//数据通道连接2
    {
        QueueInBuffer(recvData, recvDataLen, RadioProtocolRx2Q);
    }
		else if(CHANNEL_DATA_3 == channel)//数据通道连接2
    {
        //QueueInBuffer(recvData, recvDataLen, RadioProtocolRx3Q);
				BufferEnCirQueue(recvData, recvDataLen, RadioProtocolRx3Q);
    }
}

/*********************************************************************
//函数名称	:RadioProtocol_ParseTimeTask(void)
//功能		:无线解析时间任务
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:0.5秒钟调度1次，轮流搜索短信、数据通道1、数据通道2这3个队列，
//          :每次取出一条数据进行解析。每个队列有10秒的超时时间，若10秒内
//          :还没有发现帧尾（0x7E），则丢弃这条数据帧
*********************************************************************/
FunctionalState RadioProtocolParse_TimeTask(void)
{
    static u8 ParseCh = CHANNEL_SMS;//上一次解析的通道
    static u8 ParseFlag = 0;//解析标志,0表示解析缓冲里没有数据,1表示找到帧头数据,还没有找到帧尾
    static u8 TimeCount = 0;
    static u16 length = 0;
    static Queue LastQ;
    
    Queue Q;
    u8  i;
    u8  Flag;//队列数据有效标志，1为有效，0为无效（已达到队列尾）
    u8  Data;
    
    u8  *p = NULL;
#ifdef ICCARD_CERTIFICATE_SERVICE_H  
		pCirQueue pQ = RadioProtocolRx3Q;
		static pCirQueue	LastpQ = NULL;
		static u8 ParseFlag1 = 0;

    if(0 == ParseFlag1)//正常情况
    {
      TimeCount = 0;

			if(Q_STATE_SUCCESS != IsCirQueueEmpty(pQ))
			{
				while(1)
				{
					if(Q_STATE_SUCCESS == DeCirQueue(&Data,pQ))//出队成功
          {
              if(0x7e == Data)
              {
                  if(0 == ParseFlag1)//帧开始
                  {
                      ParseFlag1 = 1;
                      length = 0;
                      p = RadioProtocolParseBuffer;
                      LastpQ = pQ;
                  }
                  else
                  {
                      if(length > 0)//帧结束
                      {
                          RadioProtocol_ProtocolParse(CHANNEL_DATA_3,RadioProtocolParseBuffer, length);
                          ParseFlag1 = 0;
                          break;
                      }
                  }
              }
              else
              {
                  *p++ = Data;
                  length++;
                  if(length >= RADIO_PROTOCOL_BUFFER_SIZE)
                  {
                      ParseFlag1 = 0;
                      break;
                  }
              }
          }
          else
          {
              break;
          }
				}
			}
    }
		else
		{
        pQ = LastpQ;
        TimeCount++;
        if(TimeCount > 20)//3秒延时
        {
            ParseFlag1 = 0;
        }
        while(1)
        {
            if(Q_STATE_SUCCESS == DeCirQueue(&Data,pQ))//出队成功
            {
                if(0x7e == Data)
                {
                     if(length > 0)//帧结束
                     {
                            RadioProtocol_ProtocolParse(CHANNEL_DATA_3,RadioProtocolParseBuffer, length);
                            ParseFlag1 = 0;
                            break;
                     }
                }
                else
                {
                    RadioProtocolParseBuffer[length] = Data;
                    length++;
                    if(length >= RADIO_PROTOCOL_BUFFER_SIZE)
                    {
                        ParseFlag1 = 0;
                        break;
                    }
                }
            }
            else
            {
                
                break;
            }
        }
    }
#endif		
    if(0 == ParseFlag)//正常情况
    {
        TimeCount = 0;
        for(i=1; i<4; i++)
        {
            if(CHANNEL_SMS == ParseCh)
            {
                Q = RadioProtocolRxSmsQ;
            }
            else if(CHANNEL_DATA_1 == ParseCh)
            {
                Q = RadioProtocolRx1Q;
            }
            else if(CHANNEL_DATA_2 == ParseCh)
            {
                Q = RadioProtocolRx2Q;
            }
            else
            {
                Q = RadioProtocolRxSmsQ;
                ParseCh = CHANNEL_SMS;
            }
            if(QueueIsEmpty(Q))//空
            {
                //切换通道
                ParseCh = RadioProtocol_SwitchChannel(ParseCh);
            }
            else//非空
            {
                break;
            }
        }
        if(4 == i)//3个缓冲队列均没有数据需要解析
        {
            
        }
        else//有需要解析的数据,提取协议帧
        {
            while(1)
            {
                Data = QueueFrontAndOut2(Q, &Flag);
                if(Flag)
                {
                    if(0x7e == Data)
                    {
                        if(0 == ParseFlag)//帧开始
                        {
                            ParseFlag = 1;
                            length = 0;
                            p = RadioProtocolParseBuffer;
                            LastQ = Q;
                        }
                        else
                        {
                            if(length > 0)//帧结束
                            {
                                RadioProtocol_ProtocolParse(ParseCh,RadioProtocolParseBuffer, length);
                                ParseFlag = 0;
                                //切换通道
                                ParseCh = RadioProtocol_SwitchChannel(ParseCh);
                                break;
                            }
                        }
                    }
                    else
                    {
                        *p++ = Data;
                        length++;
                        if(length >= RADIO_PROTOCOL_BUFFER_SIZE)
                        {
                            ParseFlag = 0;
                            ParseCh = CHANNEL_SMS;
                            break;
                        }
                    }
                }
                else
                {
                    break;
                }
            }
        }
    }
    else//几个数据包拼接成一帧数据
    {
        Q = LastQ;
        TimeCount++;
        if(TimeCount > 20)//3秒延时
        {
            ParseFlag = 0;
            ParseCh = CHANNEL_SMS;
        }
        while(1)
        {
            Data = QueueFrontAndOut2(Q, &Flag);
            if(Flag)
            {
                if(0x7e == Data)
                {
                     if(length > 0)//帧结束
                     {
                            RadioProtocol_ProtocolParse(ParseCh,RadioProtocolParseBuffer, length);
                            ParseFlag = 0;
                            //切换通道
                            ParseCh = RadioProtocol_SwitchChannel(ParseCh);
                            break;
                     }
                }
                else
                {
                    RadioProtocolParseBuffer[length] = Data;
                    length++;
                    if(length >= RADIO_PROTOCOL_BUFFER_SIZE)
                    {
                        ParseFlag = 0;
                        ParseCh = CHANNEL_SMS;
                        break;
                    }
                }
            }
            else
            {
                
                break;
            }
        }
    }

	return ENABLE;
}
/*********************************************************************
//函数名称	:RadioProtocol_SwitchChannel
//功能		:切换通道
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:CHANNEL_SMS,CHANNEL_DATA_1,CHANNEL_DATA_2三个通道来回切换
*********************************************************************/
u8 RadioProtocol_SwitchChannel(u8 channel)
{
    u8 NewChannel;
    
    if(CHANNEL_SMS == channel)
    {
        NewChannel = CHANNEL_DATA_1;
    }
    else if(CHANNEL_DATA_1 == channel)
    {
        NewChannel = CHANNEL_DATA_2;
    }
    else if(CHANNEL_DATA_2 == channel)
    {
        NewChannel = CHANNEL_SMS;
    }
    else
    {
        NewChannel = 0;
    }
    
    return NewChannel;
}
/*********************************************************************
//函数名称	:RadioProtocol_SendMessage(MESSAGE_HEAD	head, u8 *SrcData, u16 SrcLen, u8 SendCh)
//功能		:发送消息
//输入		:MESSAGE_HEAD	head:消息头;SrcData:指向消息体的指针;
//		    :SrcLen:消息体长度;SendCh:发送通道,可选值只能为CHANNEL_SMS,CHANNEL_DATA_1,CHANNEL_DATA_2,
//          :或者以上这三值的组合
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:bit0对应于CHANNEL_SMS通道,bit1对应于CHANNLE_DATA_1通道,bit2对应于CHANNLE_DATA_2通道,
//          :其它bit暂时保留,对应的位为1表示该通道发送成功。
//备注		:解析完一条指令后，通常需要应答，应答时需调用该函数；
//		:主动上发一条消息命令时也需要调用该函数。
*********************************************************************/
u8 RadioProtocol_SendCmd(u8 SendCh , MESSAGE_HEAD	head, u8 *SrcData, u16 SrcLen, u8 *pSmsPhone)
{
	u8	temp2[20];
	u8	temp;
	u8	VerifyCode;
	u8	*p;
    u8  flag = 0;
	u16	i;
	u16	HeadLen;
	u16	length;
	u16	SendLen;
    
    //*****************短信通道处理*******************
    if(CHANNEL_SMS == (SendCh&CHANNEL_SMS))
    {
        head.SerialNum = SmsChannelSerialNum;
        SmsChannelSerialNum++;

	    if(0x2000 == (0x2000&head.Attribute))//若是分包数据，流水号取分包序号
        {
            head.SerialNum = head.CurrentPackage;//行标检测时要求分包数据的流水号要连续
        }
	
        //终端手机号
	    memcpy(head.PhoneNum,TerminalPhoneNumber,6);
    
	    //MESSAGE_HEAD结构体变字节流
	    HeadLen = RadioProtocol_HeadtoBytes(temp2,head);
	    //计算校验码
	    VerifyCode = 0;
	    p = temp2;
	    for(i=0; i<HeadLen; i++)
	    {
		    temp = *p;
		    VerifyCode = VerifyCode ^ temp;
		    p++;
	    }
	    p = SrcData;
	    for(i=0; i<SrcLen; i++)
	    {
		    temp = *p;
		    VerifyCode = VerifyCode ^ temp;
		    p++;
	    }
	    //把消息头，消息体，校验码拷贝到中间缓冲
	    memcpy(RadioProtocolTxBuffer+1,temp2,HeadLen);
	    if(SrcLen > 0)
	    {
		    memcpy(RadioProtocolTxBuffer+1+HeadLen,SrcData,SrcLen);
	    }
	    RadioProtocolTxBuffer[1+HeadLen+SrcLen] = VerifyCode;
	    //转义
	    length = TransMean(RadioProtocolTxBuffer+1,1+HeadLen+SrcLen);
	    length++;
	    //补充一头一尾的标志位
	    RadioProtocolTxBuffer[0] = 0x7e;
	    RadioProtocolTxBuffer[length] = 0x7e;
	    length++;

	    //发送
	    SendLen = Communication_SndMsg2Svr(CHANNEL_SMS, RadioProtocolTxBuffer, length, pSmsPhone);
	    if(length == SendLen)
	    {
            flag = flag|CHANNEL_SMS;
	    }
    }
    //****************数据通道1处理*******************

    if(CHANNEL_DATA_1 == (SendCh&CHANNEL_DATA_1))
    {
        head.SerialNum = Data1ChannelSerialNum;
        Data1ChannelSerialNum++;
  
	    if(0x2000 == (0x2000&head.Attribute))//若是分包数据，流水号取分包序号
        {
            head.SerialNum = head.CurrentPackage;//行标检测时要求分包数据的流水号要连续
        }
    
	
	    if(CHANNEL_DATA_1 != (CHANNEL_DATA_1&TerminalAuthorizationFlag))//权限处理(只有相应的通道鉴权成功后才允许发其他指令)
	    {
		    if(0x0100 == head.MessageID)//注册
		    {
          
		    }
		    else if(0x0003 == head.MessageID)//终端注销
		    {
		
		    }
		    else if(0x0102 == head.MessageID)//鉴权
		    {

		    }
		    else if(0x0110 == head.MessageID)//立即开户
		    {
		
		    }
		    else
		    {
			    flag = flag|CHANNEL_DATA_1;
		    }
	    }
	
        if(CHANNEL_DATA_1 != (flag&CHANNEL_DATA_1))
        {
            //终端手机号
	        memcpy(head.PhoneNum,TerminalPhoneNumber,6);
    
	        //MESSAGE_HEAD结构体变字节流
	        HeadLen = RadioProtocol_HeadtoBytes(temp2,head);
	        //计算校验码
	        VerifyCode = 0;
	        p = temp2;
	        for(i=0; i<HeadLen; i++)
	        {
		        temp = *p;
		        VerifyCode = VerifyCode ^ temp;
		        p++;
	        }
	        p = SrcData;
	        for(i=0; i<SrcLen; i++)
	        {
		        temp = *p;
		        VerifyCode = VerifyCode ^ temp;
		        p++;
	        }
	        //把消息头，消息体，校验码拷贝到中间缓冲
	        memcpy(RadioProtocolTxBuffer+1,temp2,HeadLen);
	        if(SrcLen > 0)
	        {
		        memcpy(RadioProtocolTxBuffer+1+HeadLen,SrcData,SrcLen);
	        }
	        RadioProtocolTxBuffer[1+HeadLen+SrcLen] = VerifyCode;
	        //转义
	        length = TransMean(RadioProtocolTxBuffer+1,1+HeadLen+SrcLen);
	        length++;
	        //补充一头一尾的标志位
	        RadioProtocolTxBuffer[0] = 0x7e;
	        RadioProtocolTxBuffer[length] = 0x7e;
	        length++;

	        //发送
	        SendLen = Communication_SndMsg2Svr(CHANNEL_DATA_1, RadioProtocolTxBuffer, length, pSmsPhone);
	        if(length == SendLen)
	        {
                flag = flag|CHANNEL_DATA_1;
	        }
        }
    }
    //****************数据通道2处理*******************
    if(CHANNEL_DATA_2 == (SendCh&CHANNEL_DATA_2))
    {
        head.SerialNum = Data2ChannelSerialNum;
        Data2ChannelSerialNum++;

	    if(0x2000 == (0x2000&head.Attribute))//若是分包数据，流水号取分包序号
        {
            head.SerialNum = head.CurrentPackage;//行标检测时要求分包数据的流水号要连续
        }
        
	    if(CHANNEL_DATA_2 != (CHANNEL_DATA_2&TerminalAuthorizationFlag))//权限处理(只有相应的通道鉴权成功后才允许发其他指令)
	    {
		    if(0x0100 == head.MessageID)//注册
		    {
          
		    }
		    else if(0x0003 == head.MessageID)//终端注销
		    {
		
		    }
		    else if(0x0102 == head.MessageID)//鉴权
		    {

		    }
		    else if(0x0110 == head.MessageID)//立即开户
		    {
		
		    }
		    else
		    {
			    flag = flag|CHANNEL_DATA_2;
		    }
	    }
	    if(CHANNEL_DATA_2 != (flag&CHANNEL_DATA_2))
        {
            //终端手机号
	        memcpy(head.PhoneNum,TerminalPhoneNumber,6);
    
	        //MESSAGE_HEAD结构体变字节流
	        HeadLen = RadioProtocol_HeadtoBytes(temp2,head);
	        //计算校验码
	        VerifyCode = 0;
	        p = temp2;
	        for(i=0; i<HeadLen; i++)
	        {
		        temp = *p;
		        VerifyCode = VerifyCode ^ temp;
		        p++;
	        }
	        p = SrcData;
	        for(i=0; i<SrcLen; i++)
	        {
		        temp = *p;
		        VerifyCode = VerifyCode ^ temp;
		        p++;
	        }
	        //把消息头，消息体，校验码拷贝到中间缓冲
	        memcpy(RadioProtocolTxBuffer+1,temp2,HeadLen);
	        if(SrcLen > 0)
	        {
		        memcpy(RadioProtocolTxBuffer+1+HeadLen,SrcData,SrcLen);
	        }
	        RadioProtocolTxBuffer[1+HeadLen+SrcLen] = VerifyCode;
	        //转义
	        length = TransMean(RadioProtocolTxBuffer+1,1+HeadLen+SrcLen);
	        length++;
	        //补充一头一尾的标志位
	        RadioProtocolTxBuffer[0] = 0x7e;
	        RadioProtocolTxBuffer[length] = 0x7e;
	        length++;

	        //发送
	        SendLen = Communication_SndMsg2Svr(CHANNEL_DATA_2, RadioProtocolTxBuffer, length, pSmsPhone);
	        if(length == SendLen)
	        {
                flag = flag|CHANNEL_DATA_2;
	        }
        }
    }
		//****************数据通道3处理*******************
    if(CHANNEL_DATA_3 == (SendCh&CHANNEL_DATA_3))
		{
        head.SerialNum = Data3ChannelSerialNum;
        Data3ChannelSerialNum++;

	    if(0x2000 == (0x2000&head.Attribute))//若是分包数据，流水号取分包序号
        {
            head.SerialNum = head.CurrentPackage;//行标检测时要求分包数据的流水号要连续
        }
        
	    if(CHANNEL_DATA_3 != (CHANNEL_DATA_3&TerminalAuthorizationFlag))//权限处理(只有相应的通道鉴权成功后才允许发其他指令)
	    {		    
			flag = flag&(~CHANNEL_DATA_3);
	    }
		else
		{
		    flag = flag|CHANNEL_DATA_3;
		}
	    if(CHANNEL_DATA_3 == (flag&CHANNEL_DATA_3))
        {
            //终端手机号
	        memcpy(head.PhoneNum,TerminalPhoneNumber,6);
    
	        //MESSAGE_HEAD结构体变字节流
	        HeadLen = RadioProtocol_HeadtoBytes(temp2,head);
	        //计算校验码
	        VerifyCode = 0;
	        p = temp2;
	        for(i=0; i<HeadLen; i++)
	        {
		        temp = *p;
		        VerifyCode = VerifyCode ^ temp;
		        p++;
	        }
	        p = SrcData;
	        for(i=0; i<SrcLen; i++)
	        {
		        temp = *p;
		        VerifyCode = VerifyCode ^ temp;
		        p++;
	        }
	        //把消息头，消息体，校验码拷贝到中间缓冲
	        memcpy(RadioProtocolTxBuffer+1,temp2,HeadLen);
	        if(SrcLen > 0)
	        {
		        memcpy(RadioProtocolTxBuffer+1+HeadLen,SrcData,SrcLen);
	        }
	        //转义
			RadioProtocolTxBuffer[1+HeadLen+SrcLen] = VerifyCode;
	        length = TransMean(RadioProtocolTxBuffer+1,1+HeadLen+SrcLen);
			length++;
	        //补充一头一尾的标志位
	        RadioProtocolTxBuffer[0] = 0x7e;
	        RadioProtocolTxBuffer[length] = 0x7e;
	        length++;

	        //发送
	        SendLen = Communication_SndMsg2Svr(CHANNEL_DATA_3, RadioProtocolTxBuffer, length, pSmsPhone);
	        if(length == SendLen)
	        {
                flag = flag|CHANNEL_DATA_3;
	        }
        }
    }
    return flag;
}
/*********************************************************************
//函数名称	:RadioProtocolResend_TimeTask(void)
//功能		:命令重发
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:连续重发3次，完成后会自动关闭定时任务。
*********************************************************************/
FunctionalState RadioProtocolResend_TimeTask(void)
{
    return DISABLE;
}
//*****************第六部分：模块内部函数**********************
/*********************************************************************
//函数名称	:RadioProtocol_ProtocolParse(u8 *pBuffer, u16 BufferLen)
//功能		:协议解析,解析一条指令
//输入		:pBuffer:指向数据(待解析的数据)的指针;BufferLen:数据的长度
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:成功,返回SUCCESS;失败,返回ERROR
//备注		:平台下发的所有命令都是由该函数解析，该函数的输入参数
//		:是7E开头,7E结尾的一串字节序列
*********************************************************************/
ErrorStatus RadioProtocol_ProtocolParse(u8 channel, u8 *pBuffer, u16 BufferLen)
{
	u8	*p;
	static u8	HighByte;
	static u8	LowByte;
	static u8	IDHighByte;
	static u8	IDLowByte;
	static u8	Index;
	u16	length;
	u16	MessageLen;
	u8	offset;
	u8	VerifyCode;
	u16	i;
	
        
    if((channel == CHANNEL_SMS)||(channel == CHANNEL_DATA_1)||(channel == CHANNEL_DATA_2)||(channel == CHANNEL_DATA_3))
    {
    
    }
    else
    {
        return ERROR;
    }
	//心跳包计数清0
	Heartbeat_ClrHeartbeatCount(channel);
    
	//反转义
	length = unTransMean(pBuffer, BufferLen);

	//检查校验码
	VerifyCode = 0;
	for(i=0; i<length-1; i++)
	{
		VerifyCode = VerifyCode ^ pBuffer[i];
	}
	if(VerifyCode != pBuffer[length-1])//dxl,2013.11.30开启校验,防止垃圾短信
	{
		return ERROR;
	}
	//指向接收缓冲
	p = pBuffer;

	//读取消息ID，按照网络字节序解析，即高字节在前
	IDHighByte = *p++;
	IDLowByte = *p++;
	RadioProtocolRxHead.MessageID = (IDHighByte << 8)|IDLowByte;

	//读取消息体属性
	HighByte = *p++;
	LowByte = *p++;
	RadioProtocolRxHead.Attribute = (HighByte << 8)|LowByte;

	//消息长度
	MessageLen = RadioProtocolRxHead.Attribute & 0x03ff;

	//终端手机号
	memcpy(RadioProtocolRxHead.PhoneNum,p,6);
	p += 6;
        
        //判断手机号
       // if(0 != strncmp((char const *)RadioProtocolRxHead.PhoneNum,(char const *)TerminalPhoneNumber,6))//dxl,2013.11.30开启校验,防止垃圾短信
       // {
                 // return ERROR;
       // }
        
        
   //判断长度
    if(0x2000 == (RadioProtocolRxHead.Attribute&0x2000))//有分包项
    {
        length -= 16;//减消息头16字节
        length--;//减校验码1字节
    }
    else//无分包项
    {
        length -= 12;//减消息头12字节
        length--;//减校验码1字节
    }
    if(length != MessageLen)
    {
         return ERROR;
    }
        

	//消息流水号
	HighByte = *p++;
	LowByte = *p++;
	RadioProtocolRxHead.SerialNum = (HighByte << 8)|LowByte;

	//读取消息包封装项
	if(0x2000 == (RadioProtocolRxHead.Attribute & 0x2000))
	{
		HighByte = *p++;
		LowByte = *p++;
		RadioProtocolRxHead.TotalPackage = (HighByte << 8)|LowByte;
		HighByte = *p++;
		LowByte = *p++;
		RadioProtocolRxHead.CurrentPackage = (HighByte << 8)|LowByte;
		offset = 16;
	}
	else
	{
		RadioProtocolRxHead.TotalPackage = 0;
		RadioProtocolRxHead.CurrentPackage = 0;
		offset = 12;
	}
    
    RadioProtocol_UpdateDownloadCmdList(RadioProtocolRxHead.MessageID,RadioProtocolRxHead.SerialNum,channel);

   //命令变换（针对非标准命令）
    if((0x88==IDHighByte)&&(0x11==IDLowByte))
	{
		IDLowByte = 0x07;
	}
	//求索引号
	if(IDLowByte > IDLowByteMax[IDHighByte-0x80])
	{
		return ERROR;
	}
	else
	{
		Index = CmdOperationIndex[IDHighByte-0x80]+IDLowByte;
		if(Index >= MAX_PROTOCOL_FUN_LIST)
		{
			return ERROR;
		}
		else
		{
			//执行函数
			(CmdOperate[Index])(channel,pBuffer+offset, MessageLen);
		}
	}
	return SUCCESS;
}
/*********************************************************************
//函数名称	:RadioProtocol_GetDownloadCmdChannel(u16 DownloadCmd, u16 *SerialNum)
//功能		:获取下行命令的流水号，通道号
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:通道号
//备注		:只能获取最新的下行命令的流水号，通道号
*********************************************************************/
u8 RadioProtocol_GetDownloadCmdChannel(u16 DownloadCmd, u16 *SerialNum)
{
    u16 i;
    u8  channel = 0;
    
    for(i=0; i<DOWNLOAD_CMD_NUM; i++)
    {
        if(DownloadCmdList[i][0] == DownloadCmd)
        {
            *SerialNum = DownloadCmdList[i][1];
            channel = DownloadCmdList[i][2];
            break;
        }
    }
    return channel;
}
/*********************************************************************
//函数名称	:RadioProtocol_GetUploadCmdChannel(u16 UploadCmd, u16 *SerialNum)
//功能		:获取上行命令的流水号，通道号
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:通道号
//备注		:只能获取最新的下行命令的流水号，通道号
*********************************************************************/
u8 RadioProtocol_GetUploadCmdChannel(u16 UploadCmd, u16 *SerialNum)
{
    u16 i;
    u8 channel = 0;
    
    for(i=0; i<DOWNLOAD_CMD_NUM; i++)
    {
        if(UploadCmdList[i][0] == UploadCmd)
        {
            *SerialNum = UploadCmdList[i][1];
            channel = UploadCmdList[i][2];
            break;
        }
    }
    return channel;
}
/*********************************************************************
//函数名称	:RadioProtocol_UpdateDownloadCmdList(u16 Cmd, u16 SerialNum, u8 channel)
//功能		:更新下行命令列表
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:，将当前命令，流水号，通道号保存下来，以便后面使用
*********************************************************************/
ErrorStatus RadioProtocol_UpdateDownloadCmdList(u16 Cmd, u16 SerialNum, u8 channel)
{
    u16 i;
    
    for(i=0; i<DOWNLOAD_CMD_NUM; i++)
    {
        if(DownloadCmdList[i][0] == Cmd)
        {
            DownloadCmdList[i][1] = SerialNum;
            DownloadCmdList[i][2] = channel;
            return SUCCESS;
        }
    }
    return ERROR;
}
/*********************************************************************
//函数名称	:RadioProtocol_UpdateUploadCmdList(u16 Cmd, u16 SerialNum, u8 channel)
//功能		:更新上行命令列表
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:，将当前命令，流水号，通道号保存下来，以便后面使用
*********************************************************************/
ErrorStatus RadioProtocol_UpdateUploadCmdList(u16 Cmd, u16 SerialNum, u8 channel)
{
    u16 i;
    
    for(i=0; i<UPLOAD_CMD_NUM; i++)
    {
        if(UploadCmdList[i][0] == Cmd)
        {
            UploadCmdList[i][1] = SerialNum;
            UploadCmdList[i][2] = channel;
            return SUCCESS;
        }
    }
    return ERROR;
}
/*********************************************************************
//函数名称	:unTransMean
//功能		:实现反转义功能
//		:0x7e----->0x7d后跟一个0x02;0x7d----->0x7d后跟一个0x01
//输入		:解析前字节流，解析前字节流长度
//输出		:解析后字节流
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:转义解析后字节流长度
//备注		:协议解析函数需调用此函数，先进行反转义，然后再解析。
*********************************************************************/
u16 unTransMean(u8 *pBuffer, u16 BufferLen)
{
	u16	i;
	u16	NewLen;
	
	i = 0;
	NewLen = BufferLen;
	while(i < BufferLen)
	{
		if(0x7d == *pBuffer)
		{
			if(0x02 == *(pBuffer+1))
			{
				*pBuffer = 0x7e;
				NewLen--;
				i++;
				memmove(pBuffer+1,pBuffer+2,BufferLen-i);
			}
			else if(0x01 == *(pBuffer+1))
			{
				*pBuffer = 0x7d;
				NewLen--;
				i++;
				memmove(pBuffer+1,pBuffer+2,BufferLen-i);
			}
		}
		i++;
		pBuffer++;
	}
	return NewLen;
}
/*********************************************************************
//函数名称	:RadioProtocol_HeadtoBytes(u8 *pBuffer, MESSAGE_HEAD head)
//功能		:把消息头变成字节流
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:
*********************************************************************/
u8 RadioProtocol_HeadtoBytes(u8 *pBuffer, MESSAGE_HEAD head)
{
	u8	length;
	u16	Attribute;

	//主机字节序转换成网络字节序
	//消息ID
	head.MessageID = RadioProtocol_htons(head.MessageID);


	//消息体属性
	Attribute = head.Attribute;
	head.Attribute = RadioProtocol_htons(head.Attribute);
	//终端手机号
	//消息流水号
	head.SerialNum = RadioProtocol_htons(head.SerialNum);
	//判断有无封装项
	if(0x2000 == (Attribute & 0x2000))//有封装项
	{
		head.TotalPackage = RadioProtocol_htons(head.TotalPackage);
		head.CurrentPackage = RadioProtocol_htons(head.CurrentPackage);
		length = 16;
	}
	else//没有封装项
	{
		length = 12;
	}
	

	memcpy((char *)pBuffer, (char *)&head, length);
	return length;
}
/*********************************************************************
//函数名称	:TransMean
//功能		:实现打包时的转义处理功能
//输入		:转义前字节流，转义前字节流长度
//输出		:转义后字节流
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:转义后字节流长度
//备注		:0x7e----->0x7d后跟一个0x02;0x7d----->0x7d后跟一个0x01
*********************************************************************/
u16 TransMean(u8 *pBuffer, u16 BufferLen)
{
	u16	i;
	u16	NewLen;
	//u8	temp;
	i = 0;
	NewLen = BufferLen;
	while(i < BufferLen)
	{
		if(0x7e == *pBuffer)
		{
			*pBuffer = 0x7d;
			memmove(pBuffer+2,pBuffer+1,BufferLen-i);
			pBuffer++;
			*pBuffer = 0x02;
			NewLen++;
			
		}
		else if(0x7d == *pBuffer)
		{
			memmove(pBuffer+2,pBuffer+1,BufferLen-i);
			pBuffer++;
			*pBuffer = 0x01;
			NewLen++;
		}
		i++;
		pBuffer++;
	}
	return NewLen;
}
/*********************************************************************
//函数名称	:RadioProtocol_IsBigEndian(void)
//功能		:测试主机字节序是否为大端系统
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:1为大端，0为小端
//备注		:
*********************************************************************/
u8 RadioProtocol_IsBigEndian(void)
{
	u16	value = 1;
	if(1 == (*(u8 *)&value))
	{
		return 0;
	}
	else
	{
		return 1;
	}
}
/*********************************************************************
//函数名称	:RadioProtocol_ntohs(u16 value)
//功能		:网络字节序转主机字节序
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:
*********************************************************************/
u16 RadioProtocol_ntohs(u16 value)
{
	u8	flag;
	u8	byte;
	u8	*p = NULL;
	u16	ConvertValue;

	flag = BigLittleFlag;
	p = (u8 *)&value;

	if(1 == flag)
	{
		ConvertValue = value;
	}
	else //交换字节序
	{
		ConvertValue = 0;	
		byte = *p++;
		ConvertValue |= byte << 8;
		byte = *p++;
		ConvertValue |= byte;	
	}
	return ConvertValue;
	
}
/*********************************************************************
//函数名称	:RadioProtocol_ntoh(u32 value)
//功能		:网络字节序转主机字节序
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:
*********************************************************************/
u32 RadioProtocol_ntoh(u32 value)
{
	u8	flag;
	u8	byte;
	u32	ConvertValue;

	flag = BigLittleFlag;

	if(1 == flag)
	{
		ConvertValue = value;
	}
	else //交换字节序
	{
		ConvertValue = 0;
		//读取高字节
		byte = (value & 0xff000000) >> 24;
		ConvertValue |= byte;
		//读取次高字节
		byte = (value & 0xff0000) >> 16;
		ConvertValue |= (byte << 8);
		//读取中字节
		byte = (value & 0xff00) >> 8;
		ConvertValue |= (byte << 16);
		//读取中字节
		byte = value;
		ConvertValue |= (byte << 24);
		
	}
	return ConvertValue;
	
}
/*********************************************************************
//函数名称	:RadioProtocol_htons(u16 value)
//功能		:主机字节序转网络字节序
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:
*********************************************************************/
u16 RadioProtocol_htons(u16 value)
{
	u8	flag;
	u8	byte;
	u8	*p = NULL;
	u16	ConvertValue;

	flag = BigLittleFlag;
	p = (u8 *)&value;

	if(1 == flag)
	{
		ConvertValue = value;
	}
	else //交换字节序
	{
		ConvertValue = 0;	
		byte = *p++;
		ConvertValue |= byte << 8;
		byte = *p++;
		ConvertValue |= byte;	
	}
	return ConvertValue;
	
}
/*********************************************************************
//函数名称	:RadioProtocol_hton(u32 value)
//功能		:主机字节序转网络字节序
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:
*********************************************************************/
u32 RadioProtocol_hton(u32 value)
{
	u8	flag;
	u8	byte;
	u8	*p;
	u32	ConvertValue;

	flag = BigLittleFlag;
	p = (u8 *)&value;

	if(1 == flag)
	{
		ConvertValue = value;
	}
	else //交换字节序
	{
		ConvertValue = 0;	
		byte = *p++;
		ConvertValue |= byte << 24;
		byte = *p++;
		ConvertValue |= byte << 16;
		byte = *p++;
		ConvertValue |= byte << 8;
		byte = *p++;
		ConvertValue |= byte;	
	}
	return ConvertValue;
	
}
/*********************************************************************
//函数名称	:NullOperate(u8 *pBuffer, u16 BufferLen)
//功能		:空操作,不干任何事
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:成功,返回ACK_OK;失败返回ACK_ERROR;
//备注		:
*********************************************************************/
ProtocolACK NullOperate(u8 channel, u8 *pBuffer, u16 BufferLen)
{
	return ACK_OK;
}

u16 RadioProtocol_Packet_ICCard_AuthInfo( u16 MessageID, u8 ExdeviceCode , u8 *pBuffer, u16 BufferLen, u8 *pOutBuf )
{
u8 temp2[20];
u8 temp;
u8 VerifyCode;
u16 i;
u16 HeadLen;
u16 length;
	
MESSAGE_HEAD	head;
u8   *p = NULL;
u8   * SrcData;
  u16   SrcLen;
        
  static u16 MessageSerialID;
  
  u8  RadioProtocolMiddleBuffer[100];//当要发送大量数据时用RadioProtocolMiddleBuffer缓冲
  
  if( BufferLen > sizeof( RadioProtocolMiddleBuffer)-1 ){
    return 0;
  }

  RadioProtocolMiddleBuffer[0]= ExdeviceCode;
	memcpy( &RadioProtocolMiddleBuffer[1],pBuffer,BufferLen);
  
	SrcLen = BufferLen+1;
  
  
	//消息ID
	head.MessageID = MessageID;
  
	//消息属性
	head.Attribute = (SrcLen & 0x3ff);
           
  SrcData = (u8 *)RadioProtocolMiddleBuffer;

  //=================================================================

	//终端手机号
  
  memcpy(head.PhoneNum,TerminalPhoneNumber,6); 
  
	//消息流水号

	head.SerialNum = MessageSerialID;
  MessageSerialID++;
  
	//MESSAGE_HEAD结构体变字节流
  
	HeadLen = RadioProtocol_HeadtoBytes(temp2,head);
  
	//计算校验码
  
	VerifyCode = 0;
	p = temp2;
	for(i=0; i<HeadLen; i++)
	{
		temp = *p;
		VerifyCode = VerifyCode ^ temp;
		p++;
	}
	p = SrcData;
	for(i=0; i<SrcLen; i++)
	{
		temp = *p;
		VerifyCode = VerifyCode ^ temp;
		p++;
	}
  
	//把消息头，消息体，校验码拷贝到中间缓冲
  
	memcpy( RadioProtocolTxBuffer+1,temp2,HeadLen);
  
	if(SrcLen > 0)
	{
		memcpy(RadioProtocolTxBuffer+1+HeadLen,SrcData,SrcLen);
	}
        
	RadioProtocolTxBuffer[1+HeadLen+SrcLen] = VerifyCode;
  
	//转义

	length = TransMean(RadioProtocolTxBuffer+1,1+HeadLen+SrcLen);
	length++;
  
	//补充一头一尾的标志位
	RadioProtocolTxBuffer[0] = 0x7e;
	RadioProtocolTxBuffer[length] = 0x7e;
	length++;

  memcpy( pOutBuf, RadioProtocolTxBuffer, length );
  
  return length;
        
}
/*********************************************************************
//函数名称	:ProtocolACK RadioProtocol_UploadFromBreak_Request(u8 *pBuffer, u16 BufferLen)
//功能		:补传分包请求
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:成功,返回ACK_OK;失败返回ACK_ERROR;
//备注		:0x8003
*********************************************************************/
//*********************************************************************************/
static ProtocolACK RadioProtocol_UploadFromBreak_Request(u8 channel, u8 *pBuffer, u16 BufferLen)
{
        u8  *p = NULL;
        u8  HighByte;
        u8  LowByte;
        u8  ResendTotalNum;
        u8  i;
        
        if(DISABLE == MyTimerTask[TIME_SUBPACKAGE].TaskTimerState)//正在分包补传时不相应当前补传请求 
        {
                p = pBuffer;
                //原始消息流水号
                HighByte = *p++;
                LowByte = *p++;
                SubpackageResendFirstSerial = (HighByte<<8)|LowByte;
                //重传包总数
                ResendTotalNum = *p++;
                //重传包ID列表
                for(i=0; i<ResendTotalNum; i++)
                {
                      HighByte = *p++;
                      LowByte = *p++;
                      SubpackageList[i] = (HighByte<<8)|LowByte;
                
                }
                if(0 != ResendTotalNum)
                {
                      //开启分包补传任务
                      SetTimerTask(TIME_SUBPACKAGE,2*SECOND);
                }
        }
	return RadioProtocol_TerminalGeneralAck(channel,0);
}
/*********************************************************************
//函数名称	:RadioProtocol_DownloadUpdatePacket(u8 *pBuffer, u16 BufferLen)
//功能		:下发终端升级包
//备注		:指令ID：0x8108
*********************************************************************/
static ProtocolACK RadioProtocol_DownloadUpdatePacket(u8 channel, u8 *pBuffer, u16 BufferLen)
{
    return RadioProtocol_TerminalGeneralAck( channel,ACK_OK );
}
/*********************************************************************
//函数名称	:RadioProtocol_UpdataResultNote
//函数功能	:终端升级结果通知
//备注		:指令ID：0x0108
*********************************************************************/
ProtocolACK RadioProtocol_UpdataResultNote(u8 channel,u8 UpdataType,u8 AckResult)
{
  u8 Buffer[5];
  u8 length;
  u8 SmsPhone[20];
  MESSAGE_HEAD	head;
  
  
  if( AckResult > 2 )
  {
    return ACK_ERROR;    
  }
  
  
  //消息ID
  head.MessageID = 0x0108;

  //消息属性
  length = 2;
  head.Attribute = length;
    
  //消息内容
  Buffer[0] = UpdataType;//升级类型
  Buffer[1] = AckResult; //升级结果

  if(0 != RadioProtocol_SendCmd(channel,head, Buffer, length, SmsPhone))
  {
      return ACK_OK;
  }
  else
  {
      return ACK_ERROR;
  }
         
}

static ProtocolACK RadioProtocol_AlarmConfirmManual(u8 channel, u8 *pBuffer, u16 BufferLen)
{

  u8 *p =NULL;
  u8 AckResult;
  static u16 AlarmSerialNum;
  static u32 AlarmType;
  u8 flag = 0;  
        
  if( BufferLen != 6 )
  {     
      AckResult = 1;	          
  }
  else
  {


      p = pBuffer;

      /*** 需要人工确认的报警流水号, 0表示报警所有消息  ***/
      AlarmSerialNum = 0;
      AlarmSerialNum = *p++;
      AlarmSerialNum <<=8;
      AlarmSerialNum += *p++;
        
      /*** 人工确认报警类型  ***/
      
       AlarmType = 0;
       AlarmType += *p++;
        
       AlarmType <<= 8;
       AlarmType += *p++;
         
       AlarmType <<= 8;    
       AlarmType += *p++;
        
       AlarmType <<= 8;
       AlarmType += *p++;
       
       
       if( AlarmType & ( 1<< 0 ) ) {
        Io_WriteAlarmBit( ALARM_BIT_EMERGENCY , RESET );	//0	1：紧急报警,收到应答后清除
	flag = 0;
	FRAM_BufferWrite(FRAM_EMERGENCY_FLAG_ADDR, &flag, FRAM_EMERGENCY_FLAG_LEN);//清除铁电存储标志
       
       }
       
      if( AlarmType & ( 1<< 3 ) ) {
	      Io_WriteAlarmBit( ALARM_BIT_DANGER_PRE_ALARM , RESET );        //3	1：危险预警,收到应答后清零
       
       }
       
       
      if( AlarmType & ( 1<< 20 ) ) {
	      Io_WriteAlarmBit( ALARM_BIT_IN_OUT_AREA , RESET );		//20	1：进出区域,收到应答后清零
       
       }  
        
      if( AlarmType & ( 1<< 21 ) ) {
	      Io_WriteAlarmBit( ALARM_BIT_IN_OUT_ROAD , RESET );		//21	1：进出路线,收到应答后清除
       
       } 
        
        
      if( AlarmType & ( 1<< 22 ) ) {
	      Io_WriteAlarmBit( ALARM_BIT_LOAD_OVER_DRIVE , RESET );	        //22	1：路段行驶时间不足/过长,收到应答后清除
        
       }
        
        
      if( AlarmType & ( 1<< 27 ) ) {
        if(1 == Io_ReadAlarmBit(ALARM_BIT_LAWLESS_ACC))
        {
              LawlessAccAlarmEnableFlag = 0;
	      Io_WriteAlarmBit( ALARM_BIT_LAWLESS_ACC , RESET );		//27	1：车辆非法点火,收到应答后清零
              
        }
        
       }	
        
        
      if( AlarmType & ( 1<< 28 ) ) {
        Io_WriteAlarmBit( ALARM_BIT_LAWLESS_MOVEMENT , RESET );	        //28	1：车辆非法位移,收到应答后清零
       
       }
       
       
       AckResult = 0;	    

  }

  return RadioProtocol_TerminalGeneralAck(channel,AckResult);
}


/*********************************************************************
//函数名称  :RadioProtocol_DriverInformationUploadRequest(u8 channel, u8 *pBuffer, u16 BufferLen)
//功能      :上报驾驶员身份信息请求
//输入      :
//输出      :
//使用资源  :
//全局变量  :
//调用函数  :
//中断资源  :
//返回      :
//备注      :指令ID：0x8702
*********************************************************************/
static ProtocolACK RadioProtocol_DriverInformationUploadRequest(u8 channel, u8 *pBuffer, u16 BufferLen)
{
    ProtocolACK AckResult;
    u8 Buffer_Temp[148];
    u16 Length_Temp = 0;
  	/*两种方式
  	1,  可以直接用读卡获取的驾驶员身份信息除去电子签名
		2,	向读卡模块发送A1命令读取
  	*/
		//if(1 == ReadPeripheral2TypeBit(7))
		//{
		//	ICCard_CustomProtocolParse_Rev_0x8702(pBuffer,BufferLen);
		//}
    Length_Temp = ICCard_PackReportDriverInfo(pBuffer,Buffer_Temp);
 
    if( Length_Temp ){
      
      AckResult=RadioProtocol_DriverInformationReport(channel,Buffer_Temp, Length_Temp);

    } else {
      
      AckResult = ACK_ERROR ; 
    }
    return AckResult;
    

}



static ProtocolACK RadioProtocol_TerminalAttribute(u8 channel, u8 *pBuffer, u16 BufferLen)
{
  u8	*p;
	u8	*q;
	u8	Buffer[150];
	u8	Buffer2[30];
	u8	Buffer3[100];
	u8	PramLen;
	u8	SmsPhone[20];
	u16	length;
	u8	i,j;
	u8	BcdHigh;
	u8	BcdLow;
	u8	value;
        
        
	MESSAGE_HEAD	head;
	u32	ID;
	
	p = Buffer;
	length = 0;

  	//读取终端类型 Bytes:2 
  	if(E2_TERMINAL_TYPE_ID_LEN == EepromPram_ReadPram( E2_TERMINAL_TYPE_ID , Buffer2))
	{
	
	}
	else
	{
		//默认为适用于客运、危险品、货运，类型：一体机
		Buffer2[0] = 0;
		Buffer2[1] = 0x07;
	}
	memcpy(p,Buffer2,2);
	p += 2;
	length += 2;

  	//制造商ID Bytes:5
  	if( 0 == EepromPram_ReadPram( E2_MANUFACTURE_ID , Buffer2 ))
	{
  		for( i = 0 ; i < 5; i++ )
		{
      			Buffer2[i] = ProductID[i] ;
    		}
  	}
	 memcpy(p,Buffer2,5);
	p += 5;
	length += 5;
        
        
	//终端型号 Bytes:20
  	for(i=0; i<20; i++)
	{
		Buffer2[i] = 0;
	}
#ifdef HUOYUN_DEBUG_OPEN
        memcpy(Buffer2,"EGS701GB",8);//如果没有设置过唯一性编号，产品型号默认为EGS701
#else
	if(E2_CAR_ONLY_NUM_ID_LEN == EepromPram_ReadPram(E2_CAR_ONLY_NUM_ID, Buffer3))
	{
		for(i=0; i<16; i++)
		{
			if(Buffer[7+i] < 128)//判断是否为ASCII码,前面7个字节为3C编号
			{
				Buffer2[i] = Buffer3[7+i];
			}
			else
			{
				break;
			}
		}
	}
	else
	{
		memcpy(Buffer2,TerminalTypeID,6);//如果没有设置过唯一性编号，产品型号默认为EGS701
	}
#endif
	memcpy(p,Buffer2,20);
	p += 20;
	length += 20;
	//终端ID Bytes:7
	PramLen = EepromPram_ReadPram(E2_DEVICE_ID, Buffer2);
	if(7 == PramLen)
	{
		//检查是之前hex码(2011.12.16之前10位ID号)还是BCD码(2011.12.16之后13位ID号)
		if((0 == Buffer2[0])&&(0 == Buffer2[1]))//对应V1.0的硬件版本
		{
			ID = 0;
			ID |= Buffer2[3] << 24;
			ID |= Buffer2[4] << 16;
			ID |= Buffer2[5] << 8;
			ID |= Buffer2[6];
			Buffer3[0] = ID%10000000/1000000+'0';
			Buffer3[1] = ID%1000000/100000+'0';
			Buffer3[2] = ID%100000/10000+'0';
			Buffer3[3] = ID%10000/1000+'0';
			Buffer3[4] = ID%1000/100+'0';
			Buffer3[5] = ID%100/10+'0';
			Buffer3[6] = ID%10+'0';
			memcpy(p,Buffer3,7);
			p += 7;
			length += 7;
		}
		else
		{
		//取终端ID后面7位作为登录注册的终端ID
		q = Buffer2;
		q += 3;
		value = *q++;
		BcdHigh = (value & 0xF0)>>4;
		BcdLow = value & 0x0F;
		Buffer3[0] = BcdLow + '0';
		value = *q++;
		BcdHigh = (value & 0xF0)>>4;
		BcdLow = value & 0x0F;
		Buffer3[1] = BcdHigh + '0';
		Buffer3[2] = BcdLow + '0';
		value = *q++;
		BcdHigh = (value & 0xF0)>>4;
		BcdLow = value & 0x0F;
		Buffer3[3] = BcdHigh + '0';
		Buffer3[4] = BcdLow + '0';
		value = *q++;
		BcdHigh = (value & 0xF0)>>4;
		BcdLow = value & 0x0F;
		Buffer3[5] = BcdHigh + '0';
		Buffer3[6] = BcdLow + '0';
		memcpy(p,Buffer3,7);
		p += 7;
		length += 7;
		}
	}
	else
	{
		//读取备份的终端ID号
		PramLen = EepromPram_ReadPram(E2_DEVICE_BACKUP_ID, Buffer2);
		if(7 == PramLen)
		{
			//检查是之前hex码(2011.12.16之前10位ID号)还是BCD码(2011.12.16之后13位ID号)
			if((0 == Buffer2[0])&&(0 == Buffer2[1]))//对应V1.0的硬件版本
			{
			ID = 0;
			ID |= Buffer2[3] << 24;
			ID |= Buffer2[4] << 16;
			ID |= Buffer2[5] << 8;
			ID |= Buffer2[6];
			Buffer3[0] = ID%10000000/1000000+'0';
			Buffer3[1] = ID%1000000/100000+'0';
			Buffer3[2] = ID%100000/10000+'0';
			Buffer3[3] = ID%10000/1000+'0';
			Buffer3[4] = ID%1000/100+'0';
			Buffer3[5] = ID%100/10+'0';
			Buffer3[6] = ID%10+'0';
			memcpy(p,Buffer3,7);
			p += 7;
			length += 7;
			}
			else
			{
			//取终端ID后面7位作为登录注册的终端ID
			q = Buffer2;
			q += 3;
			value = *q++;
			BcdHigh = (value & 0xF0)>>4;
			BcdLow = value & 0x0F;
			Buffer3[0] = BcdLow + '0';
			value = *q++;
			BcdHigh = (value & 0xF0)>>4;
			BcdLow = value & 0x0F;
			Buffer3[1] = BcdHigh + '0';
			Buffer3[2] = BcdLow + '0';
			value = *q++;
			BcdHigh = (value & 0xF0)>>4;
			BcdLow = value & 0x0F;
			Buffer3[3] = BcdHigh + '0';
			Buffer3[4] = BcdLow + '0';
			value = *q++;
			BcdHigh = (value & 0xF0)>>4;
			BcdLow = value & 0x0F;
			Buffer3[5] = BcdHigh + '0';
			Buffer3[6] = BcdLow + '0';
			memcpy(p,Buffer3,7);
			p += 7;
			length += 7;
			}
		}
		else
		{
			//固定ID
			Buffer3[0] = '2';
			Buffer3[1] = '0';
			Buffer3[2] = '1';
			Buffer3[3] = '2';
			Buffer3[4] = '0';
			Buffer3[5] = '0';
			Buffer3[6] = '1';
			memcpy(p,Buffer3,7);
			p += 7;
			length += 7;
		}
	}
       //终端SIM卡ICCID BCD[10]  Bytes:10
        MOD_GetIccidBcdValue(&Buffer2[0] );
        memcpy(p,Buffer2,10 );
	p += 10 ;
	length += 10 ;
        
        //终端硬件版本长度 BYTE  Bytes:1
        j = strlen((char const *)TerminalVersionHW);
        *p++ = j;
        length++;
	//终端硬件版本
        for( i = 0 ; i < j; i++ )
        {
          Buffer2[i] = TerminalVersionHW[i] ; 
        }
        memcpy(p,Buffer2,j);//HUOYUN_DEBUG_OPEN货运平台调试前该行没有，为后来新增
        p += j;
	length += j;

       //终端固件版本长度 BYTE  Bytes:1
        j = strlen((char const *)FirmwareVersion);
	*p++ = j;
        length++;
        //终端固件版本号 STRING
 	for(i=0; i<j; i++)
	{
		Buffer2[i] = FirmwareVersion[i];
	}
        memcpy(p,Buffer2,j);
	p += j ;
	length += j;
        
        //GNSS模块属性
        if(E2_GPS_ATTRIBUTE_ID_LEN == EepromPram_ReadPram( E2_GPS_ATTRIBUTE_ID , Buffer2  ))
	{
		*p = Buffer2[0];
		p++;
    		length++;  
	}
	else
	{
		*p = 0x03;//没有设置过的化是支持GPS+北斗
		p++;
    		length++;  
	}
    
         
         //通信模块属性
    #ifdef EYE_MODEM
    *p = Modem_Api_Type();//CDMA通讯
		//*p = Buffer2[0];
		//p++;
    		//length++;  
	//}
    #else
	//{
#if (MOD_TYPE == CDMA_MODULE_MC8331)
	*p = 0x02;//CDMA通讯
#endif
	
#if (MOD_TYPE == GPRS_MODULE_M10)
	*p = 0x01;//GPRS通讯
#endif
    #endif
	p++;
    	length++;  
	
	//消息ID
	head.MessageID = 0x0107;
  	head.Attribute = (length & 0x3ff);
  
	if(0 != RadioProtocol_SendCmd(channel,head, Buffer, length, SmsPhone))
	{
		return ACK_OK;
	} 
	else 
	{
		return ACK_ERROR;

	} 

}



/*********************************************************************
//函数名称	:RadioProtocol_PostionInformation_BulkUpTrans(u8 *pBuffer, u16 length)
//功能		:定位数据批量上传
//输入		:pBuffer:指向数据（位置信息）指针;BufferLen:数据的长度;
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		: 成功,返回ACK_OK;失败返回ACK_ERROR;
//备注		: 指令ID：0x0704
//参见          : 页面 P45  <<道路运输车辆卫星定位系统北斗兼容车载终端通讯协议技术规范2013-01版>> 
*********************************************************************/
ProtocolACK  RadioProtocol_PostionInformation_BulkUpTrans(u8 channel,u8 *pBuffer, u16 BufferLen)
{
	MESSAGE_HEAD	head;
	u8	SmsPhone[20];
	u16	length;

	//判断长度
	if((BufferLen >= 1024)||(0 != MultiMediaSendFlag))
	{
		return ACK_ERROR;
	}
	
	length = BufferLen;
	//消息ID
	head.MessageID = 0x0704;
	//消息属性
	head.Attribute = (BufferLen & 0x3ff);
        
	if(0 != RadioProtocol_SendCmd(channel,head, pBuffer, length, SmsPhone))
	{
		return ACK_OK;
	}
	else
	{
		return ACK_ERROR;
	}
}

/*********************************************************************
//函数名称	:RadioProtocol_CAN_UpTrans(u8 *pBuffer, u16 length)
//功能		:CAN总线上传数据
//输入		:pBuffer:指向数据（位置信息）指针;BufferLen:数据的长度;
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		: 成功,返回ACK_OK;失败返回ACK_ERROR;
//备注		: 指令ID：0x0705
*********************************************************************/
ProtocolACK  RadioProtocol_CAN_UpTrans(u8 channel,u8 *pBuffer, u16 BufferLen)
{
	MESSAGE_HEAD	head;
	u8	SmsPhone[20];
	u16	length;

	//判断长度
	if((BufferLen >= 1024)||(0 != MultiMediaSendFlag))
	{
		return ACK_ERROR;
	}
	
	length = BufferLen;
	//消息ID
	head.MessageID = 0x0705;
	//消息属性
	head.Attribute = (BufferLen & 0x3ff);
        
	if(0 != RadioProtocol_SendCmd(channel,head, pBuffer, length, SmsPhone))
	{
		return ACK_OK;
	}
	else
	{
		return ACK_ERROR;
	}
}


  

u16 IC_Card_Radio_Heartbeat( u8 *pOutBuf )
{
  
	u8 temp2[20];
	u8 temp;
	u8 VerifyCode;
	u16 i;
	u16 HeadLen;
	u16 length;
	
	MESSAGE_HEAD	head;
  u8   *p = NULL;
 
        
  u8 PhoneTemp[6];
  static u16 MessageSerialID;

	//消息ID
	head.MessageID = 0x0002;
	//消息属性
	head.Attribute = (0 & 0x3ff);
           
 
//=================================================================
        
	//终端手机号
#if 1
        
  PhoneTemp[0] = 0x01;
  PhoneTemp[1] = 0x38;
  PhoneTemp[2] = 0x00;
  PhoneTemp[3] = 0x13;
  PhoneTemp[4] = 0x80;
  PhoneTemp[5] = 0x00;
  memcpy(head.PhoneNum,PhoneTemp,6);
  
#else
        
  memcpy(head.PhoneNum,TerminalPhoneNumber,6); 
        
#endif

	//消息流水号
	
	head.SerialNum = MessageSerialID;
  MessageSerialID++;
        
	//MESSAGE_HEAD结构体变字节流
	HeadLen = RadioProtocol_HeadtoBytes(temp2,head);
  
	//计算校验码
	VerifyCode = 0;
	p = temp2;
	for(i=0; i<HeadLen; i++)
	{
		temp = *p;
		VerifyCode = VerifyCode ^ temp;
		p++;
	}
	
	//把消息头，消息体，校验码拷贝到中间缓冲
	memcpy(RadioProtocolTxBuffer+1,temp2,HeadLen);
	
	RadioProtocolTxBuffer[1+HeadLen] = VerifyCode;
	//转义
#if 1
	length = TransMean(RadioProtocolTxBuffer+1,1+HeadLen+0);
#else        
  length = 1+HeadLen+SrcLen;
#endif 
	length++;
	//补充一头一尾的标志位
	RadioProtocolTxBuffer[0] = 0x7e;
	RadioProtocolTxBuffer[length] = 0x7e;
	length++;
   //CmdSendErrorCount = 0;
	//发送
//	SendLen = Communication_SndMsg2Svr(channel, RadioProtocolTxBuffer, length, SmsPhone);
        
  memcpy( pOutBuf, RadioProtocolTxBuffer, length );
  return length;
}




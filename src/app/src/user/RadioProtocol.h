#ifndef __RADIO_PROTOCOL_H
#define __RADIO_PROTOCOL_H

//*********文件包含************
#include "stm32f2xx.h"
#include "stm32f2xx_conf.h"
#include "rtc.h"
#include "main.h"

//********宏定义****************
#define	MAX_PROTOCOL_FUN_LIST		(43+5)//Modify By Shigle 2013-04-01

#define DEVICE_LOGIN_PASSWORD_LEN	20
#define LOGIN_PASSWORD_LEN		22
#define RADIO_PROTOCOL_RESEND_STEP_LEN	100
#define	RADIO_PROTOCOL_BUFFER_SIZE	1200
#define	RADIO_PROTOCOL_RX1_BUFFER_SIZE	1200
#define	RADIO_PROTOCOL_RX2_BUFFER_SIZE	1200
#define	RADIO_PROTOCOL_RX_SMS_BUFFER_SIZE	400
#define RADIOPROTOCOL_RESEND_BUFFER_SIZE  1100//只存储五条重发的指令,每一条100字节

//*****************自定义参数ID******************
#define SELF_DEFINE_PRAMID_BASE		0x0200
#define DIAL_POINT_NAME_PRAMID		(SELF_DEFINE_PRAMID_BASE+1)//拨号点名称参数ID,自定义
#define DIAL_USER_NAME_PRAMID		(SELF_DEFINE_PRAMID_BASE+2)//拨号用户名参数ID,自定义
#define DIAL_PASSWORD_PRAMID		(SELF_DEFINE_PRAMID_BASE+3)//拨号密码参数ID,自定义
#define WIRELESS_UPDATA_IP_PRAMID	(SELF_DEFINE_PRAMID_BASE+4)//地址参数ID,自定义
#define WIRELESS_UPDATA_PORT_PRAMID	(SELF_DEFINE_PRAMID_BASE+5)//端口参数ID,自定义
#define WIRELESS_UPDATA_HARDWARE_PRAMID	(SELF_DEFINE_PRAMID_BASE+6)//硬件版本参数ID,自定义
#define WIRELESS_UPDATA_SOFTWARE_PRAMID	(SELF_DEFINE_PRAMID_BASE+7)//固件版本参数ID,自定义
#define WIRELESS_UPDATA_URL_PRAMID	(SELF_DEFINE_PRAMID_BASE+8)//URL参数ID,自定义
#define WIRELESS_UPDATA_TIME_PRAMID	(SELF_DEFINE_PRAMID_BASE+9)//时限参数ID,自定义
//#define DIAL_POINT_NAME_PRAMID		(SELF_DEFINE_PRAMID_BASE+10)//拨号点名称参数ID,自定义
//#define DIAL_USER_NAME_PRAMID		(SELF_DEFINE_PRAMID_BASE+11)//拨号用户名参数ID,自定义
//#define DIAL_PASSWORD_PRAMID		(SELF_DEFINE_PRAMID_BASE+12)//拨号密码参数ID,自定义
#define CONTROL_CONNECT_IP_PRAMID	(SELF_DEFINE_PRAMID_BASE+13)//地址参数ID,自定义
#define CONTROL_CONNECT_PORT_PRAMID	(SELF_DEFINE_PRAMID_BASE+14)//端口参数ID,自定义
#define CONTROL_CONNECT_HARDWARE_PRAMID	(SELF_DEFINE_PRAMID_BASE+15)//硬件版本参数ID,自定义
#define CONTROL_CONNECT_SOFTWARE_PRAMID	(SELF_DEFINE_PRAMID_BASE+16)//固件版本参数ID,自定义
#define CONTROL_CONNECT_URL_PRAMID	(SELF_DEFINE_PRAMID_BASE+17)//URL参数ID,自定义
#define CONTROL_CONNECT_TIME_PRAMID	(SELF_DEFINE_PRAMID_BASE+18)//时限参数ID,自定义

//协议与软件监控中心应答类型
typedef enum {ACK_OK = 0, ACK_ERROR, FORMAT_ERROR, NOACK = 0xFF}ProtocolACK;



#define UPDATA_PACKET_SIZE_MAX 1024
typedef struct{
  
u8 Type;
u8 ManufacturerID[5];
u8 VersionLength;
u8 StrVersion[20];
u32 PacketLength;
u8  PacketBuffer[ UPDATA_PACKET_SIZE_MAX ];

}TERMINAL_UPDATA_PACKET;


/********P23 补传分包请求 结构体************/
typedef struct{
  
u16 OriginalMsgSerialNum;
u8  ReUploadPacketNum;
u16 ReUploadPacketIDList[2*255];

}STRUCT_UPLOADFROMBREAK; 
        
        

typedef struct{
	u16 	MessageID;//消息ID
	u16 	Attribute;//消息体属性
	u8 	PhoneNum[6];//手机号,BCD码
	u16 	SerialNum;//消息流水号
	u16 	TotalPackage;//消息总包数
	u16 	CurrentPackage;//当前包序号，从1开始
}MESSAGE_HEAD;

typedef struct{
	u16	List[20];//命令解析列表,分10组,没有命令时为0;触发任务前清0
	u8	ListCount;//待解析的协议指令数量
	u8	EnterCount;//进入协议解析任务的次数;触发任务前清0
}RADIO_PARSE_LIST;

//***********函数声明**************
//*****************第一部分：标准的平台下发指令**********************
//*******************************************************************
/*********************************************************************
//函数名称	:RadioProtocol_ServerGeneralAck(u8 *pBuffer, u16 BufferLen)
//功能		:平台（服务器）通用应答
//备注		:指令ID：0x8001
*********************************************************************/
ProtocolACK RadioProtocol_ServerGeneralAck(u8 channel, u8 *pBuffer, u16 BufferLen);
/*********************************************************************
//函数名称	:RadioProtocol_TerminalRegisterAck(u8 *pBuffer, u16 BufferLen)
//功能		:终端注册应答
//备注		:指令ID：0x8100
*********************************************************************/
ProtocolACK RadioProtocol_TerminalRegisterAck(u8 channel, u8 *pBuffer, u16 BufferLen);
/*********************************************************************
//函数名称	:RadioProtocol_PramSet(u8 *pBuffer, u16 BufferLen)
//功能		:设置终端参数
//备注		:指令ID：0x8103
*********************************************************************/
ProtocolACK RadioProtocol_PramSet(u8 channel, u8 *pBuffer, u16 BufferLen);
/*********************************************************************
//函数名称	:RadioProtocol_WritePram
//功能		:写参数
//备注		:无线写参数跟有线写参数公用此函数，该函数被0x8103命令调用
*********************************************************************/
ProtocolACK RadioProtocol_WritePram(u8 channel,u8 *pBuffer, u16 BufferLen);
/*********************************************************************
//函数名称	:RadioProtocol_PramSearch(u8 *pBuffer, u16 BufferLen)
//功能		:查询终端参数
//备注		:指令ID：0x8104
*********************************************************************/
ProtocolACK RadioProtocol_PramSearch(u8 channel, u8 *pBuffer, u16 BufferLen);
/*********************************************************************
//函数名称	:RadioProtocol_ReadPram
//功能		:读参数
//输入		:把需要读的参数ID号放入缓冲pIDBuffer中，每个ID四字节，高在前
//返回		:读取的参数总个数，*pDstBuffer按ID号，ID长度，ID内容的顺序存放，*DstBufferLen总的读取长度，不包括参数总个数字节
//备注		:ReadType:0为有线(串口)，每次只能读550字节；1为无线（GPRS），每次可读1000字节
*********************************************************************/
u8 RadioProtocol_ReadPram(u8 *pDstBuffer, s16 *DstBufferLen, u8 *pIDBuffer, s16 IDBufferLen,u8 ReadType);
/*********************************************************************
//函数名称	:RadioProtocol_TerminalControl(u8 *pBuffer, u16 BufferLen)
//功能		:终端控制
//备注		:指令ID：0x8105
*********************************************************************/
ProtocolACK RadioProtocol_TerminalControl(u8 channel, u8 *pBuffer, u16 BufferLen);
/*********************************************************************
//函数名称	:RadioProtocol_PositionSearch(u8 *pBuffer, u16 BufferLen)
//功能		:位置信息查询
//备注		:指令ID：0x8201
*********************************************************************/
ProtocolACK RadioProtocol_PositionSearch(u8 channel, u8 *pBuffer, u16 BufferLen);
/*********************************************************************
//函数名称	:RadioProtocol_TempPositionControl(u8 *pBuffer, u16 BufferLen)
//功能		:临时位置跟踪控制
//备注		:指令ID：0x8202
*********************************************************************/
ProtocolACK RadioProtocol_TempPositionControl(u8 channel, u8 *pBuffer, u16 BufferLen);
/*********************************************************************
//函数名称	:RadioProtocol_TextMessageDownload(u8 *pBuffer, u16 BufferLen)
//功能		:文本信息下发
//备注		:指令ID：0x8300
*********************************************************************/
ProtocolACK RadioProtocol_TextMessageDownload(u8 channel, u8 *pBuffer, u16 BufferLen);
/*********************************************************************
//函数名称	:RadioProtocol_EvenSet(u8 *pBuffer, u16 BufferLen)
//功能		:事件设置
//备注		:指令ID：0x8301
*********************************************************************/
ProtocolACK RadioProtocol_EvenSet(u8 channel, u8 *pBuffer, u16 BufferLen);
/*********************************************************************
//函数名称	:RadioProtocol_QuestionDownload(u8 *pBuffer, u16 BufferLen)
//功能		:提问下发
//备注		:指令ID：0x8302
*********************************************************************/
ProtocolACK RadioProtocol_QuestionDownload(u8 channel, u8 *pBuffer, u16 BufferLen);
/*********************************************************************
//函数名称	:RadioProtocol_InformationOrderSet(u8 *pBuffer, u16 BufferLen)
//功能		:信息点播菜单设置
//备注		:指令ID：0x8303
*********************************************************************/
ProtocolACK RadioProtocol_InformationOrderSet(u8 channel, u8 *pBuffer, u16 BufferLen);
/*********************************************************************
//函数名称	:RadioProtocol_InformationService(u8 *pBuffer, u16 BufferLen)
//功能		:信息服务
//备注		:指令ID：0x8304
*********************************************************************/
ProtocolACK RadioProtocol_InformationService(u8 channel, u8 *pBuffer, u16 BufferLen);
/*********************************************************************
//函数名称	:RadioProtocol_TelephoneCallback(u8 *pBuffer, u16 BufferLen)
//功能		:电话回拨
//备注		:指令ID：0x8400
*********************************************************************/
ProtocolACK RadioProtocol_TelephoneCallback(u8 channel, u8 *pBuffer, u16 BufferLen);
/*********************************************************************
//函数名称	:RadioProtocol_TelephoneBookSet(u8 *pBuffer, u16 BufferLen)
//功能		:设置电话本
//备注		:指令ID：0x8401
*********************************************************************/
ProtocolACK RadioProtocol_TelephoneBookSet(u8 channel, u8 *pBuffer, u16 BufferLen);
/*********************************************************************
//函数名称	:RadioProtocol_CarControl(u8 *pBuffer, u16 BufferLen)
//功能		:车辆控制
//备注		:指令ID：0x8500
*********************************************************************/
ProtocolACK RadioProtocol_CarControl(u8 channel, u8 *pBuffer, u16 BufferLen);
/*********************************************************************
//函数名称	:RadioProtocol_SetRoundArea(u8 *pBuffer, u16 BufferLen)
//功能		:设置圆形区域
//备注		:指令ID：0x8600
*********************************************************************/
ProtocolACK RadioProtocol_SetRoundArea(u8 channel, u8 *pBuffer, u16 BufferLen);
/*********************************************************************
//函数名称	:RadioProtocol_DelRoundArea(u8 *pBuffer, u16 BufferLen)
//功能		:删除圆形区域
//备注		:指令ID：0x8601
*********************************************************************/
ProtocolACK RadioProtocol_DelRoundArea(u8 channel, u8 *pBuffer, u16 BufferLen);
/*********************************************************************
//函数名称	:RadioProtocol_SetRectangleArea(u8 *pBuffer, u16 BufferLen)
//功能		:设置矩形区域
//备注		:指令ID：0x8602
*********************************************************************/
ProtocolACK RadioProtocol_SetRectangleArea(u8 channel, u8 *pBuffer, u16 BufferLen);
/*********************************************************************
//函数名称	:RadioProtocol_DelRectangleArea(u8 *pBuffer, u16 BufferLen)
//功能		:删除矩形区域
//备注		:指令ID：0x8603
*********************************************************************/
ProtocolACK RadioProtocol_DelRectangleArea(u8 channel, u8 *pBuffer, u16 BufferLen);
/*********************************************************************
//函数名称	:RadioProtocol_SetPolygonArea(u8 *pBuffer, u16 BufferLen)
//功能		:设置多边形区域
//备注		:指令ID：0x8604
*********************************************************************/
ProtocolACK RadioProtocol_SetPolygonArea(u8 channel, u8 *pBuffer, u16 BufferLen);
/*********************************************************************
//函数名称	:RadioProtocol_DelPolygonArea(u8 *pBuffer, u16 BufferLen)
//功能		:删除多边形区域
//备注		:指令ID：0x8605
*********************************************************************/
ProtocolACK RadioProtocol_DelPolygonArea(u8 channel, u8 *pBuffer, u16 BufferLen);
/*********************************************************************
//函数名称	:RadioProtocol_SetRoad(u8 *pBuffer, u16 BufferLen)
//功能		:设置路线
//备注		:指令ID：0x8606
*********************************************************************/
ProtocolACK RadioProtocol_SetRoad(u8 channel, u8 *pBuffer, u16 BufferLen);
/*********************************************************************
//函数名称	:RadioProtocol_DelRoad(u8 *pBuffer, u16 BufferLen)
//功能		:删除路线
//备注		:指令ID：0x8607
*********************************************************************/
ProtocolACK RadioProtocol_DelRoad(u8 channel, u8 *pBuffer, u16 BufferLen);

/*********************************************************************
//函数名称	:RadioProtocol_SetMuckDumpArea
//功能		:设置渣土倾倒区域(0x860a)
//备注		:指令ID：0x860a
*********************************************************************/
ProtocolACK RadioProtocol_SetMuckDumpArea(u8 channel, u8 *pBuffer, u16 BufferLen);
/*********************************************************************
//函数名称	:RadioProtocol_DeleteMuckDumpArea
//功能		:删除渣土倾倒区域(0x860b)
//备注		:指令ID：0x860b
*********************************************************************/
ProtocolACK RadioProtocol_DeleteMuckDumpArea(u8 channel, u8 *pBuffer, u16 BufferLen);
/*********************************************************************
//函数名称	:RadioProtocol_QueryMuckDumpArea(u8 *pBuffer, u16 BufferLen)
//功能		:删除渣土倾倒区域
//备注		:指令ID：0x860c
*********************************************************************/
ProtocolACK RadioProtocol_QueryMuckDumpArea(u8 channel, u8 *pBuffer, u16 BufferLen);
/*********************************************************************
//函数名称	:RadioProtocol_RecorderDataCollection(u8 *pBuffer, u16 BufferLen)
//功能		:行驶记录仪信息采集
//备注		:指令ID：0x8700
*********************************************************************/
ProtocolACK RadioProtocol_RecorderDataCollection(u8 channel, u8 *pBuffer, u16 BufferLen);
/*********************************************************************
//函数名称	:RadioProtocol_RecorderPramSet(u8 channel, u8 *pBuffer, u16 BufferLen)
//功能		:行驶记录仪参数下传
//备注		:指令ID：0x8701
*********************************************************************/
ProtocolACK RadioProtocol_RecorderPramSet(u8 channel, u8 *pBuffer, u16 BufferLen);
/*********************************************************************
//函数名称	:RadioProtocol_MultiMediaUploadAck(u8 *pBuffer, u16 BufferLen)
//功能		:多媒体数据上传应答
//备注		:指令ID：0x8800
*********************************************************************/
ProtocolACK RadioProtocol_MultiMediaUploadAck(u8 channel, u8 *pBuffer, u16 BufferLen);
/*********************************************************************
//函数名称	:RadioProtocol_CameraPhoto(u8 *pBuffer, u16 BufferLen)
//功能		:摄像头立即拍摄
//备注		:指令ID：0x8801
*********************************************************************/
ProtocolACK RadioProtocol_CameraPhoto(u8 channel, u8 *pBuffer, u16 BufferLen);
/*********************************************************************
//函数名称	:RadioProtocol_MemoryIndex(u8 *pBuffer, u16 BufferLen)
//功能		:存储多媒体数据检索
//备注		:指令ID：0x8802
*********************************************************************/
ProtocolACK RadioProtocol_MemoryIndex(u8 channel, u8 *pBuffer, u16 BufferLen);
/*********************************************************************
//函数名称	:RadioProtocol_MemoryUpload(u8 *pBuffer, u16 BufferLen)
//功能		:存储多媒体数据上传
//备注		:指令ID：0x8803
*********************************************************************/
ProtocolACK RadioProtocol_MemoryUpload(u8 channel, u8 *pBuffer, u16 BufferLen);
/*********************************************************************
//函数名称	:RadioProtocol_StartSoundRecord(u8 *pBuffer, u16 BufferLen)
//功能		:开始录音
//备注		:指令ID：0x8804
*********************************************************************/
ProtocolACK RadioProtocol_StartSoundRecord(u8 channel, u8 *pBuffer, u16 BufferLen);
/*********************************************************************
//函数名称	:RadioProtocol_SingleMemoryIndex(u8 *pBuffer, u16 BufferLen)
//功能		:单条存储多媒体数据检索上传
//备注		:指令ID：0x8805
*********************************************************************/
ProtocolACK RadioProtocol_SingleMemoryIndex(u8 channel, u8 *pBuffer, u16 BufferLen);
/*********************************************************************
//函数名称	:RadioProtocol_OriginalDataDownTrans(u8 *pBuffer, u16 BufferLen)
//功能		:数据下行透传
//备注		:指令ID：0x8900
*********************************************************************/
ProtocolACK RadioProtocol_OriginalDataDownTrans(u8 channel, u8 *pBuffer, u16 BufferLen);
/*********************************************************************
//函数名称	:RadioProtocol_ServerRSA(u8 *pBuffer, u16 BufferLen)
//功能		:平台公钥
//备注		:指令ID：0x8A00
*********************************************************************/
ProtocolACK RadioProtocol_ServerRSA(u8 channel, u8 *pBuffer, u16 BufferLen);

//*****************第二部分：标准的终端上发指令**********************
//*******************************************************************
/*********************************************************************
//函数名称	:RadioProtocol_TerminalGeneralAck(u16 SerialNum, u16 MessageID, u8 Result)
//功能		:终端通用应答
//备注		:指令ID：0x0001
*********************************************************************/
ProtocolACK RadioProtocol_TerminalGeneralAck(u8 channel, u8 AckResult);
/*********************************************************************
//函数名称	:RadioProtocol_TerminalHeartbeat(void)
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
ProtocolACK RadioProtocol_TerminalHeartbeat(u8 channel);
/*********************************************************************
//函数名称	:RadioProtocol_TerminalLogout(void)
//功能		:终端注销
//备注		:指令ID：0x0003
*********************************************************************/
ProtocolACK RadioProtocol_TerminalLogout(u8 channel);
/*********************************************************************
//函数名称	:RadioProtocol_TerminalRegister(void)
//功能		:终端注册
//备注		:指令ID：0x0100
*********************************************************************/
ProtocolACK RadioProtocol_TerminalRegister(u8 channel);
/*********************************************************************
//函数名称	:RadioProtocol_TerminalQualify(void)
//功能		:终端鉴权
//备注		:指令ID：0x0102
*********************************************************************/
ProtocolACK RadioProtocol_TerminalQualify(u8 channel);
/*********************************************************************
//函数名称	:DownloadUpdatePacket_Result
//函数功能	:终端升级结果通知
//备注		:指令ID：0x0108
*********************************************************************/
extern ProtocolACK DownloadUpdatePacket_Result( u8 channel, u8 UpdataType , u8 AckResult );
/*********************************************************************
//函数名称	:RadioProtocol_OpenAccount(void)
//功能		:终端开户
//返回		:  成功,返回ACK_OK;失败返回ACK_ERROR;
//备注		:指令ID：0x0110
*********************************************************************/
ProtocolACK RadioProtocol_OpenAccount(u8 channel);
/*********************************************************************
//函数名称	:RadioProtocol_FirmwareUpdataResultNote(u8 *pBuffer, u16 BufferLen)
//功能		:远程固件升级结果通知
//返回		:  成功,返回ACK_OK;失败返回ACK_ERROR;
//备注		:指令ID：0x0108
*********************************************************************/
ProtocolACK RadioProtocol_FirmwareUpdataResultNote(u8 channel, u8 *pBuffer, u16 BufferLen);
/*********************************************************************
//函数名称	:RadioProtocol_PostionInformationReport(u8 *pBuffer, u16 length)
//功能		:位置信息汇报
//备注		:指令ID：0x0200
*********************************************************************/
ProtocolACK  RadioProtocol_PostionInformationReport(u8 channel, u8 *pBuffer, u16 BufferLen);
/*********************************************************************
//函数名称	:RadioProtocol_EvenReport(u8 EvenID)
//功能		:事件报告
//备注		:指令ID：0x0301
*********************************************************************/
ProtocolACK RadioProtocol_EvenReport(u8 channel, u8 EvenID);
/*********************************************************************
//函数名称	:RadioProtocol_QuestionAck(u8 AnswerID)
//功能		:提问应答
//备注		:指令ID：0x0302
*********************************************************************/
ProtocolACK RadioProtocol_QuestionAck(u8 channel, u8 AnswerID);
/*********************************************************************
//函数名称	:RadioProtocol_InforamtionOrder(u8 InformationType, u8 ControlFlg)
//功能		:信息点播
//备注		:指令ID：0x0303
*********************************************************************/
ProtocolACK RadioProtocol_InforamtionOrder(u8 channel, u8 InformationType, u8 ControlFlg);
/*********************************************************************
//函数名称	:RadioProtocol_RecorderDataReport(u8 *pBuffer, u8 BufferLen)
//功能		:行驶记录仪数据上传
//备注		:指令ID：0x0700
*********************************************************************/
ProtocolACK RadioProtocol_RecorderDataReport(u8 channel, u8 *pBuffer, u16 BufferLen, u16 TotalPacket, u16 Packet);
/*********************************************************************
//函数名称	:RadioProtocol_ElectricReceiptReport(u8 *pBuffer, u8 BufferLen)
//功能		:电子运单上报
//备注		:指令ID：0x0701
*********************************************************************/
ProtocolACK RadioProtocol_ElectricReceiptReport(u8 channel, u8 *pBuffer, u16 BufferLen);
/*********************************************************************
//函数名称	:RadioProtocol_DriverInformationReport(u8 *pBuffer, u8 BufferLen)
//功能		:驾驶员身份信息上报
//备注		:指令ID：0x0702
*********************************************************************/
ProtocolACK RadioProtocol_DriverInformationReport(u8 channel, u8 *pBuffer, u8 BufferLen);
/*********************************************************************
//函数名称	:RadioProtocol_PostionInformation_BulkUpTrans(u8 *pBuffer, u16 length)
//功能		:定位数据批量上传
//输入		:pBuffer:指向数据（位置信息）指针;BufferLen:数据的长度;
//返回		: 成功,返回ACK_OK;失败返回ACK_ERROR;
//备注		: 指令ID：0x0704
//参见          : 页面 P45  <<道路运输车辆卫星定位系统北斗兼容车载终端通讯协议技术规范2013-01版>> 
*********************************************************************/
ProtocolACK  RadioProtocol_PostionInformation_BulkUpTrans(u8 channel, u8 *pBuffer, u16 BufferLen);
/*********************************************************************
//函数名称	:RadioProtocol_MultiMediaEvenReport(u32 DataID, u8 Type, u8 Format, u8 Even, u8 ChID)
//功能		:多媒体事件信息上传
//备注		:指令ID：0x0800
*********************************************************************/
ProtocolACK RadioProtocol_MultiMediaEvenReport(u8 channel, u32 DataID, u8 Type, u8 Format, u8 Even, u8 ChID);
/*********************************************************************
//函数名称	:RadioProtocol_MultiMediaDataReport(u8 *pBuffer, u16 BufferLen)
//功能		:多媒体数据上传
//备注		:  指令ID：0x0801
*********************************************************************/
ProtocolACK RadioProtocol_MultiMediaDataReport(u8 channel, u8 *pBuffer, u16 BufferLen, u16 TotalPacket, u16 Packet);
/*********************************************************************
//函数名称	:RadioProtocol_MemoryIndexAck(u8 *pBuffer, u16 BufferLen)
//功能		:存储多媒体检索应答
//输入		:pBuffer:指向数据（需要包含多媒体ID;多媒体类型;通道ID;事件项编码;位置信息汇报;
//备注		:  指令ID：0x0802
*********************************************************************/
ProtocolACK RadioProtocol_MemoryIndexAck(u8 channel, u8 *pBuffer, u16 BufferLen);
/*********************************************************************
//函数名称	:RadioProtocol_PhotoAck(u8 *pBuffer, u16 BufferLen)
//功能		:摄像头立即拍照应答
//返回		:  成功,返回ACK_OK;失败返回ACK_ERROR;
//备注		:  指令ID：0x0805
*********************************************************************/
ProtocolACK RadioProtocol_PhotoAck(u8 channel, u8 *pBuffer, u16 BufferLen);
/*********************************************************************
//函数名称	:RadioProtocol_JiutongPhoto(u8 *pBuffer, u16 BufferLen)
//功能		:摄像头立即拍摄(九通协议)
//备注		:指令ID：0x8811
*********************************************************************/
ProtocolACK RadioProtocol_JiutongPhoto(u8 channel, u8 *pBuffer, u16 BufferLen);
/*********************************************************************
//函数名称	:RadioProtocol_OriginalDataUpTrans(u8 Type, u32 TotalLen, u8 *pBuffer, u16 BufferLen)
//功能		:数据上行透传
//备注		:指令ID：0x0900
*********************************************************************/
ProtocolACK RadioProtocol_OriginalDataUpTrans(u8 channel, u8 Type, u8 *pBuffer, u16 BufferLen);
/*********************************************************************
//函数名称	:RadioProtocol_DataCompressUpTrans(u8 *pBuffer, u16 BufferLen)
//功能		:数据压缩上传
//备注		:指令ID：0x0901
*********************************************************************/
ProtocolACK RadioProtocol_DataCompressUpTrans(u8 channel, u8 *pBuffer, u16 BufferLen);
/*********************************************************************
//函数名称	:RadioProtocol_TerminalRSA(u32 RSAe, u32 RSAn)
//功能		:终端RSA公钥
//备注		:指令ID：0x0A00
*********************************************************************/
ProtocolACK RadioProtocol_TerminalRSA(u8 channel, u32 RSAe, u8 *pRSAn);

//*****************第三部分：自定义的平台下发指令**********************
//*********************************************************************


//*****************第四部分：自定义的终端上发指令**********************
//*********************************************************************

//*****************第五部分：对外接口函数******************************
//********************************************************************
/*********************************************************************
//函数名称	:RadioProtocol_UpdateTerminalPhoneNum(void)
//功能		:更新终端电话号码
//备注		:上电初始化时需调用
*********************************************************************/
void RadioProtocol_UpdateTerminalPhoneNum(void);
/*********************************************************************
//函数名称	:RadioProtocol_GetRadioProtocolRxBufferBusyFlag(void)
//功能		:读接收缓冲忙标志
//备注		:通信模块接收到数据后需调用此函数,判断是否能往协议接收缓冲RadioProtocolRxBuffer
//		:拷贝数据，若返回值为0表示空闲可以往协议接收缓冲，为1表示正忙，当前接收的数据丢弃
*********************************************************************/
u8  RadioProtocol_GetRadioProtocolRxBufferBusyFlag(void);
/*********************************************************************
//函数名称	:RadioProtocol_SetRadioProtocolRxBufferBusyFlag
//功能		:置位RadioProtocolRxBufferBusyFlag
//备注		:当往协议接收缓冲拷贝完数据后需置位协议接收缓冲忙标准,
//		:只有等协议接收缓冲里的数据都被解析完后该标准由系统自动清0
*********************************************************************/
void RadioProtocol_SetRadioProtocolRxBufferBusyFlag(void);
/*********************************************************************
//函数名称	:RadioProtocol_AddRadioParseList(void)
//功能		:添加内容到无线解析列表
//备注		:当往协议接收缓冲拷贝完数据后需调用此函数,将新接收到的数据
//		:（可能同时收到好几条命令,所以要用一个表列）登记进解析列表,
//		:然后开启无线解析定时任务
*********************************************************************/
void RadioProtocol_AddRadioParseList(void);
/*********************************************************************
//函数名称	:RadioProtocol_ParseTimeTask(void)
//功能		:无线解析时间任务
//备注		:1秒钟调度1次，不断查找解析列表，若列表中还有未解析的数据，
//		:则调用解析函数进行解析。若没有，则关闭该任务。
*********************************************************************/
FunctionalState RadioProtocolParse_TimeTask(void);
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
u8 RadioProtocol_SwitchChannel(u8 channel);
/*********************************************************************
//函数名称	:RadioProtocol_SendMessage(MESSAGE_HEAD	head, u8 *SrcData, u16 SrcLen, u8 SendCh)
//功能		:发送消息
//输入		:MESSAGE_HEAD	head:消息头;SrcData:指向消息体的指针;
//		    :SrcLen:消息体长度;SendCh:发送通道,可选值只能为CHANNEL_SMS,CHANNEL_DATA_1,CHANNEL_DATA_2,
//          :或者以上这三值的组合
//返回		:bit0对应于CHANNEL_SMS通道,bit1对应于CHANNLE_DATA_1通道,bit2对应于CHANNLE_DATA_2通道,
//          :其它bit暂时保留,对应的位为1表示该通道发送成功。
//备注		:解析完一条指令后，通常需要应答，应答时需调用该函数；
//		:主动上发一条消息命令时也需要调用该函数。
*********************************************************************/
u8 RadioProtocol_SendCmd(u8 SendCh , MESSAGE_HEAD	head, u8 *SrcData, u16 SrcLen, u8 *pSmsPhone);
/*********************************************************************
//函数名称	:RadioProtocolResend_TimeTask(void)
//功能		:命令重发定时任务
//备注		:1秒钟调度1次，直至发送成功，完成后会自动关闭。
*********************************************************************/
FunctionalState RadioProtocolResend_TimeTask(void);
/*********************************************************************
//函数名称	:RadioProtocol_PositionPacketSendTimeTask(void)
//功能		:位置信息打包上传任务，用于定位精度测试，行标检测修改
//备注		:0.5秒调度1次
*********************************************************************/
FunctionalState RadioProtocol_PositionPacketSendTimeTask(void);
//*****************第六部分：内部接口函数******************************
//********************************************************************
/*********************************************************************
//函数名称	:RadioProtocol_ProtocolParse(u8 channel, u8 *pBuffer, u16 BufferLen)
//功能		:协议解析,解析一条指令
//备注		:平台下发的所有命令都是由该函数解析，该函数的输入参数
//		:是7E开头,7E结尾,已做转义处理的一串字节序列
*********************************************************************/
ErrorStatus RadioProtocol_ProtocolParse(u8 channel, u8 *pBuffer, u16 BufferLen);
/*********************************************************************
//函数名称	:RadioProtocol_GetDownloadCmdChannel(u16 DownloadCmd, u16 *SerialNum, u8 *channel)
//功能		:获取下行命令的流水号，通道号
//返回		:0错误，1成功
//备注		:只能获取最新的下行命令的流水号，通道号
*********************************************************************/
u8 RadioProtocol_GetDownloadCmdChannel(u16 DownloadCmd, u16 *SerialNum);
/*********************************************************************
//函数名称	:RadioProtocol_GetUploadCmdChannel(u16 UploadCmd, u16 *SerialNum, u8 *channel)
//功能		:获取上行命令的流水号，通道号
//返回		:0错误，1成功
//备注		:只能获取最新的下行命令的流水号，通道号
*********************************************************************/
u8 RadioProtocol_GetUploadCmdChannel(u16 UploadCmd, u16 *SerialNum);
/*********************************************************************
//函数名称	:RadioProtocol_UpdateDownloadCmdList(u16 Cmd, u16 SerialNum, u8 channel)
//功能		:更新上行命令列表
//备注		:，将当前命令，流水号，通道号保存下来，以便后面使用
*********************************************************************/
ErrorStatus RadioProtocol_UpdateDownloadCmdList(u16 Cmd, u16 SerialNum, u8 channel);
/*********************************************************************
//函数名称	:RadioProtocol_UpdateUploadCmdList(u16 Cmd, u16 SerialNum, u8 channel)
//功能		:更新上行命令列表
//备注		:，将当前命令，流水号，通道号保存下来，以便后面使用
*********************************************************************/
ErrorStatus RadioProtocol_UpdateUploadCmdList(u16 Cmd, u16 SerialNum, u8 channel);
/*********************************************************************
//函数名称	:unTransMean
//功能		:实现反转义功能
//		:0x7e----->0x7d后跟一个0x02;0x7d----->0x7d后跟一个0x01
//输入		:解析前字节流，解析前字节流长度
//输出		:解析后字节流
//返回		:转义解析后字节流长度
//备注		:协议解析函数需调用此函数，先进行反转义，然后再解析。
*********************************************************************/
u16 unTransMean(u8 *pBuffer, u16 BufferLen);
/*********************************************************************
//函数名称	:RadioProtocol_HeadtoBytes(u8 *pBuffer, MESSAGE_HEAD head)
//功能		:把消息头变成字节流
//备注		:
*********************************************************************/
u8 RadioProtocol_HeadtoBytes(u8 *pBuffer, MESSAGE_HEAD head);
/*********************************************************************
//函数名称	:TransMean
//功能		:实现转义功能
//		:0x7e----->0x7d后跟一个0x02;0x7d----->0x7d后跟一个0x01
//输入		:转义前字节流，转义前字节流长度
//输出		:转义后字节流
//返回		:转义后字节流长度
//备注		:
*********************************************************************/
u16 TransMean(u8 *pBuffer, u16 BufferLen);
/*********************************************************************
//函数名称 :RadioProtocol_OriginalDataUpTransEx(u8 Type, u8 *pBuffer, u16 BufferLen)
//功能 :数据上行透传,带消息流水号
//备注 :
*********************************************************************/
ProtocolACK RadioProtocol_OriginalDataUpTransEx(u8 Type, u8 *pBuffer, u16 BufferLen);
/*********************************************************************
//函数名称	:RadioProtocol_IsBigEndian(void)
//功能		:测试主机字节序是否为大端系统
//备注		:
*********************************************************************/
u8 RadioProtocol_IsBigEndian(void);
/*********************************************************************
//函数名称	:RadioProtocol_ntohs(u16 value)
//功能		:网络字节序转主机字节序
//备注		:
*********************************************************************/
u16 RadioProtocol_ntohs(u16 value);
/*********************************************************************
//函数名称	:RadioProtocol_ntoh(u32 value)
//功能		:网络字节序转主机字节序
//备注		:
*********************************************************************/
u32 RadioProtocol_ntoh(u32 value);
/*********************************************************************
//函数名称	:RadioProtocol_htons(u16 value)
//功能		:主机字节序转网络字节序
//备注		:
*********************************************************************/
u16 RadioProtocol_htons(u16 value);
/*********************************************************************
//函数名称	:RadioProtocol_hton(u32 value)
//功能		:主机字节序转网络字节序
//备注		:
*********************************************************************/
u32 RadioProtocol_hton(u32 value);
/*********************************************************************
//函数名称	:NullOperate(u8 *pBuffer, u16 BufferLen)
//功能		:空操作,不干任何事
//备注		:
*********************************************************************/
ProtocolACK NullOperate(u8 channel,u8 *pBuffer, u16 BufferLen);

extern u16 RadioProtocol_Packet_ICCard_AuthInfo( u16 MessageID, u8 ExdeviceCode , u8 *pBuffer, u16 BufferLen, u8 *pOutBuf );
u16 IC_Card_Radio_Heartbeat( u8 *pOutBuf );
ProtocolACK RadioProtocol_UploadFromBreak_Request(u8 channel,u8 *pBuffer, u16 BufferLen);
ProtocolACK RadioProtocol_DownloadUpdatePacket(u8 channel, u8 *pBuffer, u16 BufferLen);
ProtocolACK RadioProtocol_AlarmConfirmManual(u8 channel, u8 *pBuffer, u16 BufferLen);
ProtocolACK RadioProtocol_DriverInformationUploadRequest(u8 channel, u8 *pBuffer, u16 BufferLen);
ProtocolACK RadioProtocol_TerminalAttribute(u8 channel,u8 *pBuffer, u16 BufferLen);
ProtocolACK RadioProtocol_UpdataResultNote(u8 channel,u8 UpdataType,u8 AckResult);
extern ProtocolACK  RadioProtocol_CAN_UpTrans(u8 channel,u8 *pBuffer, u16 BufferLen);
void RadioProtocol_AddRecvDataForParse(unsigned char ch,unsigned char *recvData,unsigned short recvDataLen);;

//****************外部使用变量声明*****************

#endif

/************************************************************************
//程序名称：Report.c
//功能：该模块实现定时上报、定距上报、定时+定距上报、实时跟踪功能，支持双连接的位置汇报。
//版本号：V0.1
//版权说明：版权属于深圳市伊爱高新技术开发有限公司
//开发人：董显林
//开发时间：2014.10
//版本记录：

//备注：版本记录需包含版本、修改人、修改时间、修改原因、修改简要说明
//V0.1：需要实现双连接的位置汇报，连接1按照行标标准进行位置汇报，连接2的TCP端口若设置为0表示关闭第2个连接，
//即也会关闭连接2的位置汇报，当为非0时表示开启连接2。当第2个连接开启后，第2个连接的ACC ON和ACC OFF上报时间间隔设置为非0，
//则表示开启第2个连接的特殊上报功能，否则按连接1的方式上报。
*************************************************************************/

/********************文件包含*************************/
#include <stdio.h>
#include "Report.h"
#include "spi_flashapi.h"
#include "EepromPram.h"
#include "Public.h"
#include "fm25c160.h"
#include "Blind.h"
#include "Gps_App.h"
#include "RadioProtocol.h"
#include "GPIOControl.h"
#include "Io.h"
#include "NaviLcd.h"
#include "SpeedMonitor.h"
#include "Pulse_App.h"
#include "MileMeter.h"
#include "JointechOiL_Protocol.h"
#include "LzmSys.h"
#include "OilWearCalib.h"
#include "AreaManage.h"
#include "Route.h"


/********************本地变量*************************/
static u32 Link1ReportTime = 0;//连接1位置汇报时间间隔，单位秒
static u32 Link1ReportTimeCount = 0;//连接1位置汇报时间计数
static u32 Link1LastReportDistance = 0;//连接1上一个上报点的距离，单位米
static u32 Link1ReportDistance = 0;//连接1下一个上报点的距离
static u8 Link1ReportAckFlag = 0;//连接1上一条位置信息应答标志，1表示有应答，0表示无应答
static u8 Link1ReportBuffer[FLASH_BLIND_STEP_LEN] = {0};//连接1位置上报缓冲
static u8 Link1ReportBufferLen = 0;//连接1位置上报缓冲内容长度
static u32 Link2AccOnReportTimeSpace = 0;//连接2 ACC ON位置汇报时间间隔，为0表示关闭该功能
static u32 Link2AccOffReportTimeSpace = 0;//连接2 ACC OFF位置汇报时间间隔，为0表示关闭该功能
static u8 Link2OpenFlag = 0;//通道2开启标志，1为开启，0为关闭
static u32 Link2ReportTime = 0;//连接2 位置汇报时间间隔，单位秒
static u32 Link2ReportTimeCount = 0;//连接2 位置汇报时间计数
static u8 Link2ReportAckFlag = 0;//上一条位置信息应答标志，1表示有应答，0表示无应答
static u8 Link2ReportBuffer[FLASH_BLIND_STEP_LEN] = {0};//位置上报缓冲
static u8 Link2ReportBufferLen = 0;//位置上报缓冲内容长度
static u32 Link2ReportDistanceSpace = 0;//连接2位置上报时间间隔，单位米
static u32 ReportStrategy = 0;//汇报策略
static u32 ReportScheme = 0;//汇报方案
static u32 EmergencyReportTimeSpace = 0;//紧急报警汇报时间间隔
static u32 EmergencyReportDistanceSpace = 0;//紧急报警汇报距离间隔
static u32 OverSpeedReportTimeSpace = 0;//超速报警汇报时间间隔
static u32 OverTimeReportTimeSpace = 0;//超时报警汇报时间间隔
static u32 AccOnReportTimeSpace = 0;//缺省汇报时间间隔
static u32 AccOnReportDistanceSpace = 0;//缺省汇报时间间隔
static u32 AccOffReportTimeSpace = 0;//休眠汇报时间间隔
static u32 AccOffReportDistanceSpace = 0;//休眠汇报时间间隔
static u32 UnloginReportTimeSpace = 0;//驾驶员未登录汇报时间间隔
static u32 UnloginReportDistanceSpace = 0;//驾驶员未登录汇报时间间隔
static u32 TempTrackTimeSpace = 0;//临时跟踪时间间隔
static u32 TempTrackCount = 0;//临时跟踪次数

/********************全局变量*************************/
u16 Link1LastReportSerialNum = 0;//连接1上一条位置上报的流水号
u16 Link2LastReportSerialNum = 0;//连接2上一条位置上报的流水号
u32 Link1ReportTimeSpace = 0;//同Link1ReportTime一样，用作外部访问
u32 Link1ReportDistanceSpace = 0;//连接1位置上报时间间隔，单位米
/********************外部变量*************************/
extern TIME_T CurTime;
extern u8	SpeedFlag;//速度类型,dxl,2015.5.11,0x00:手动脉冲,0x01:手动GPS,0x02:自动脉冲,0x03:自动GPS
/********************本地函数声明*********************/
static void Report_GetLink1NextReportPoint(u32 *NextReportTime, u32 *NextReportDistance);//获取下一个上报点
static u32 Report_GetCurDistance(void);//获取当前的累计行驶里程
static u16 Report_GetPositionAdditionalInfo(u8 *pBuffer);//获取位置附加信息
static u16 Report_GetPositionStdAdditionalInfo(u8 *pBuffer);//获取标准的位置附加信息
static u16 Report_GetPositionEiAdditionalInfo(u8 *pBuffer);//获取伊爱自定义的位置附加信息
static u8 Report_Link2ReportOpenFlag(void);//连接2特殊上报开启标志，1为开启，0为关闭
/********************函数定义*************************/

/**************************************************************************
//函数名：Report_TimeTask
//功能：实现位置汇报（包括实时跟踪、紧急报警、超速报警、超时报警、定时定距等位置汇报）
//输入：无
//输出：无
//返回值：始终是ENABLE
//备注：位置汇报定时任务，50ms进入1次，任务调度器需要调用此函数
***************************************************************************/
FunctionalState Report_TimeTask(void)
{
	u32 CurTimeCount = 0;
	u32 Link1CurDistance = 0;//当前位置点距离
	u8 Link1ReportFlag = 0;//位置上报标志，1表示要上报，0表示不用上报
	u8 Link2ReportFlag = 0;
	u8 Link2ReportStatus = 0;//第2个连接上报状态，0表示静止上报，1表示按连接1的方式上报，2表示按连接2的特殊方式上报
	u8 Ack = 0;
	u8 Acc = 0;
	u8 TempBuffer[5];
	static u8 Link1LastSendFlag = 0;
	static u8 Link2LastSendFlag = 0;
	//static u8 UpdateFlag = 0;
	//static GPS_STRUCT LastPosition;
	static u32 count = 0;
	static u32 LastTimeCount = 0;
	u8 ReportAddTimeCount;
	
	//if(0 == UpdateFlag)
	//{
	//	UpdateFlag = 1;
	//	Report_UpdatePram();
	//}

	count++;//dxl,2016.5.7增加
    if(count < 1200)//启动后前60秒不上报位置信息，因为这段时间是连网过程，肯定上报不上去，会作为盲区记录的，这样每次启动就会上来几条盲区，为避免这种情况发生加60秒延时
    {
			  return ENABLE;  
    }
		else if(count == 1200)
		{
			  //Gps_CopyPosition(&LastPosition);
		    	Report_UpdatePram();
			  LastTimeCount = RTC_GetCounter();
			  return ENABLE;
		}
		else
		{			 		
		    CurTimeCount = RTC_GetCounter();
		    if((CurTimeCount != LastTimeCount))
		    {
					ReportAddTimeCount = (CurTimeCount + 60 - LastTimeCount)%60;
        			LastTimeCount = CurTimeCount;  
		    }
		    else
		    {
		         return ENABLE;
		    }		
		}

		
		
    if(0 != Link2OpenFlag)
    {
        if(1==Report_Link2ReportOpenFlag())
        {
            Link2ReportStatus = 2;//已开启连接2的特殊汇报
        }
        else
        {
            Link2ReportStatus = 1;//已开启第2个连接，按连接1的方式汇报
        }
    }
    else
    {
        Link2ReportStatus = 0;//已关闭连接2
    }

	Link1ReportTimeCount += ReportAddTimeCount;
	
	Report_GetLink1NextReportPoint(&Link1ReportTime,&Link1ReportDistance);

	if(0 != Link1ReportTime)//Link1ReportTime等于0表示不开启定时上报
	{
		if(Link1ReportTimeCount >= Link1ReportTime)
		{
			Link1ReportFlag = 1;
			if(1 == Link2ReportStatus)
			{
				Link2ReportFlag = 1;
				Link2ReportTime = Link1ReportTime;
				
			}
			if((TempTrackTimeSpace > 0)&&(TempTrackCount > 0))
			{
				TempTrackCount--;
				Public_ConvertLongToBuffer(TempTrackCount,TempBuffer);
				FRAM_BufferWrite(FRAM_TEMP_TRACK_NUM_ADDR,TempBuffer,4);
				if(0 == TempTrackCount)
				{
					TempTrackTimeSpace = 0;
					Public_ConvertLongToBuffer(TempTrackTimeSpace,TempBuffer);
					FRAM_BufferWrite(FRAM_TEMP_TRACK_SPACE_ADDR,TempBuffer,4);
				}
			}
		}
	}

	if(0 != Link1ReportDistance)//Link1ReportDistance等于0表示不开启定距上报
	{
		Link1CurDistance = Report_GetCurDistance();
		if(Link1CurDistance >= Link1ReportDistance)
		{
			Link1ReportFlag = 1;
			if(1 == Link2ReportStatus)
			{
				Link2ReportFlag = 1;
				
			}
		}
	}

	if(1 == Link1ReportFlag)
	{
		if(1 == Link1LastSendFlag)
		{
			Link1LastSendFlag = 0;
			Link1LastReportDistance = Report_GetCurDistance();
			if(((0 == Link1ReportAckFlag)&&(Link1ReportTime >= 5))
                    		||((0 == Link1ReportAckFlag)&&(Link1ReportDistanceSpace >= 100)))//上一条位置信息汇报是否收到应答，1表示收到
			{
				//if(1 == Gps_ReadStatus())//只有导航才会上报，状态或报警变化也会上报当不在这个任务处理
				//{
					Blind_Save(CHANNEL_DATA_1,Link1ReportBuffer,Link1ReportBufferLen);
				//}
			}
		}

				
		Link1ReportBufferLen = Report_GetPositionInfo(Link1ReportBuffer);
		Ack = RadioProtocol_PostionInformationReport(CHANNEL_DATA_1,Link1ReportBuffer,Link1ReportBufferLen);
		RadioProtocol_GetUploadCmdChannel(0x0200,&Link1LastReportSerialNum);
		if(ACK_OK != Ack)
		{
			if(((Link1ReportTime < 5)&&(Link1ReportTime > 0))
                          ||((Link1ReportDistanceSpace < 100)&&(Link1ReportDistanceSpace > 0)))
			{
				//if(1 == Gps_ReadStatus())//只有导航才会上报，状态或报警变化也会上报当不在这个任务处理
				//{
					Blind_Save(CHANNEL_DATA_1,Link1ReportBuffer,Link1ReportBufferLen);
				//}
			}
				
		}
		Link1LastSendFlag = 1;
		Link1ReportTimeCount = 0;
		Link1LastReportDistance = Link1CurDistance;
		Link1ReportAckFlag = 0;	
		
	}

	if(2 == Link2ReportStatus)
	{
		Link2ReportTimeCount++;

		Acc = Io_ReadStatusBit(STATUS_BIT_ACC);
		if(1 == Acc)
		{
			Link2ReportTime = Link2AccOnReportTimeSpace;
		}
		else
		{
			Link2ReportTime = Link2AccOffReportTimeSpace;
		}

		if(0 != Link2ReportTime)//Link2ReportTime等于0表示不开启定时上报
		{
			if(Link2ReportTimeCount >= Link2ReportTime)
			{
				Link2ReportFlag = 1;
			}
		}
	}

	if(1 == Link2ReportFlag)
	{
		if(1 == Link2LastSendFlag)
		{
			Link2LastSendFlag = 0;
			if(((0 == Link2ReportAckFlag)&&(Link2ReportTime >= 5))
                    		||((0 == Link2ReportAckFlag)&&(Link2ReportDistanceSpace >= 100)))//上一条位置信息汇报是否收到应答，1表示收到
			{
				//if(1 == Gps_ReadStatus())
				//{
					Blind_Save(CHANNEL_DATA_2,Link2ReportBuffer,Link2ReportBufferLen);
				//}
			}
		}
		
		Link2ReportBufferLen = Report_GetPositionInfo(Link2ReportBuffer);
		Ack = RadioProtocol_PostionInformationReport(CHANNEL_DATA_2,Link2ReportBuffer,Link2ReportBufferLen);
		RadioProtocol_GetUploadCmdChannel(0x0200,&Link2LastReportSerialNum);
		if(ACK_OK != Ack)
		{
			if(((Link2ReportTime < 5)&&(Link2ReportTime > 0))||
				(Link2ReportDistanceSpace > 0)&&(Link2ReportDistanceSpace < 100))
			{
				
				//if(1 == Gps_ReadStatus())//只有导航才会上报，状态或报警变化也会上报当不在这个任务处理
				//{
					Blind_Save(CHANNEL_DATA_2,Link2ReportBuffer,Link2ReportBufferLen);
				//}
				
			}
		}
		Link2LastSendFlag = 1;	
		Link2ReportTimeCount = 0;
		Link2ReportAckFlag = 0;
		
	}

	return ENABLE;		
}
/**************************************************************************
//函数名：Report_UpdatePram
//功能：更新位置汇报所有变量
//输入：无
//输出：无
//返回值：无
//备注：主要是更新位置汇报模块所使用到的变量，运行任务调度之前需调用此函数
***************************************************************************/
void Report_UpdatePram(void)
{
	u8 i;
	for(i=PRAM_REPORT_STRATEGY; i<=PRAM_SECOND_LINK_OPEN_FLAG; i++)
	{
		Report_UpdateOnePram(i);
	}

}
/**************************************************************************
//函数名：Report_CmdAck
//功能：位置上报应答处理
//输入：通道号
//输出：无
//返回值：无
//备注：收到位置上报的通用应答时需调用此函数
***************************************************************************/
void Report_CmdAck(u8 channel)
{
	if(CHANNEL_DATA_1 == channel)
	{
		Link1ReportAckFlag = 1;
	}
	else if(CHANNEL_DATA_2 == channel)
	{
		Link2ReportAckFlag = 1;
	}
}
/**************************************************************************
//函数名：Report_UpdateOnePram
//功能：更新某个参数
//输入：参数类型
//输出：该参数类型对应的变量
//返回值：无
//备注：串口或远程设置位置汇报相关参数时需要调用此函数
***************************************************************************/
void Report_UpdateOnePram(u8 type)
{
	u8 Buffer[30];
	u8 BufferLen;
	u32 TcpPort;
	
	switch(type)
	{
	case PRAM_REPORT_STRATEGY://位置汇报策略
		{
			if(E2_POSITION_REPORT_STRATEGY_LEN == EepromPram_ReadPram(E2_POSITION_REPORT_STRATEGY_ID,Buffer))
			{
				ReportStrategy = Public_ConvertBufferToLong(Buffer);
			}
			break;
		}
	case PRAM_REPORT_SCHEME://位置汇报方案
		{
			if(E2_POSITION_REPORT_SCHEME_LEN == EepromPram_ReadPram(E2_POSITION_REPORT_SCHEME_ID,Buffer))
			{
				ReportScheme = Public_ConvertBufferToLong(Buffer);
			}
			break;
		}
	case PRAM_TEMP_TRACK_TIME://临时跟踪时间间隔
		{
			if(FRAM_TEMP_TRACK_SPACE_LEN == FRAM_BufferRead(Buffer, 2, FRAM_TEMP_TRACK_SPACE_ADDR))
			{
				TempTrackTimeSpace = Public_ConvertBufferToShort(Buffer);
			}
			break;
		}
	case PRAM_TEMP_TRACK_COUNT://临时跟踪次数
		{
			if(FRAM_TEMP_TRACK_NUM_LEN == FRAM_BufferRead(Buffer, 4, FRAM_TEMP_TRACK_NUM_ADDR))
			{
				TempTrackCount = Public_ConvertBufferToLong(Buffer);
			}
			break;
		}
	case PRAM_EMERGENCY_REPORT_TIME://紧急报警汇报时间间隔
		{
			if(E2_EMERGENCY_REPORT_TIME_LEN == EepromPram_ReadPram(E2_EMERGENCY_REPORT_TIME_ID,Buffer))
			{
				EmergencyReportTimeSpace = Public_ConvertBufferToLong(Buffer);
			}
			break;
		}
	case PRAM_EMERGENCY_REPORT_DISTANCE://紧急报警汇报距离间隔
		{
			if(E2_EMERGENCY_REPORT_DISTANCE_LEN == EepromPram_ReadPram(E2_EMERGENCY_REPORT_DISTANCE_ID,Buffer))
			{
				EmergencyReportDistanceSpace = Public_ConvertBufferToLong(Buffer);
			}
			break;
		}
	case PRAM_OVER_SPEED_REPORT_TIME://超速报警汇报时间间隔
		{
			if(E2_OVERSPEED_ALARM_REPORT_TIME_ID_LEN == EepromPram_ReadPram(E2_OVERSPEED_ALARM_REPORT_TIME_ID,Buffer))
			{
				OverSpeedReportTimeSpace = Public_ConvertBufferToLong(Buffer);
			}
			break;
		}
	case PRAM_OVER_TIME_REPORT_TIME://超时报警汇报时间间隔
		{
			if(E2_TIRED_DRIVE_REPORT_TIME_ID_LEN == EepromPram_ReadPram(E2_TIRED_DRIVE_REPORT_TIME_ID,Buffer))
			{
				OverTimeReportTimeSpace = Public_ConvertBufferToLong(Buffer);
			}
			break;
		}
	case PRAM_ACC_ON_REPORT_TIME://缺省汇报时间间隔
		{
			if(E2_ACCON_REPORT_TIME_LEN == EepromPram_ReadPram(E2_ACCON_REPORT_TIME_ID,Buffer))
			{
				AccOnReportTimeSpace = Public_ConvertBufferToLong(Buffer);
			}
			break;
		}
	case PRAM_ACC_ON_REPORT_DISTANCE://缺省汇报距离间隔
		{
			if(E2_ACCON_REPORT_DISTANCE_LEN == EepromPram_ReadPram(E2_ACCON_REPORT_DISTANCE_ID,Buffer))
			{
				AccOnReportDistanceSpace = Public_ConvertBufferToLong(Buffer);
			}
			break;
		}
	case PRAM_ACC_OFF_REPORT_TIME://休眠汇报时间间隔
		{
			if(E2_SLEEP_REPORT_TIME_LEN == EepromPram_ReadPram(E2_SLEEP_REPORT_TIME_ID,Buffer))
			{
				AccOffReportTimeSpace = Public_ConvertBufferToLong(Buffer);
			}
			break;
		}
	case PRAM_ACC_OFF_REPORT_DISTANCE://休眠汇报距离间隔
		{
			if(E2_SLEEP_REPORT_DISTANCE_LEN == EepromPram_ReadPram(E2_SLEEP_REPORT_DISTANCE_ID,Buffer))
			{
				AccOffReportDistanceSpace = Public_ConvertBufferToLong(Buffer);
			}
			break;
		}
	case PRAM_UNLOGIN_REPORT_TIME://驾驶员未登录汇报时间间隔
		{
			if(E2_DRIVER_UNLOGIN_REPORT_TIME_LEN == EepromPram_ReadPram(E2_DRIVER_UNLOGIN_REPORT_TIME_ID,Buffer))
			{
				UnloginReportTimeSpace = Public_ConvertBufferToLong(Buffer);
			}
			break;
		}
	case PRAM_UNLOGIN_REPORT_DISTANCE://驾驶员未登录汇报距离间隔
		{
			if(E2_DRIVER_UNLOGIN_REPORT_DISTANCE_LEN == EepromPram_ReadPram(E2_DRIVER_UNLOGIN_REPORT_DISTANCE_ID,Buffer))
			{
				UnloginReportDistanceSpace = Public_ConvertBufferToLong(Buffer);
			}
			break;
		}
	case PRAM_SECOND_LINK_ACC_ON_REPORT_TIME://连接2 ACC ON汇报时间间隔
		{
			if(E2_SECOND_ACC_ON_REPORT_TIME_ID_LEN == EepromPram_ReadPram(E2_SECOND_ACC_ON_REPORT_TIME_ID,Buffer))
			{
				Link2AccOnReportTimeSpace = Public_ConvertBufferToLong(Buffer);
			}
			
			break;
		}
	case PRAM_SECOND_LINK_ACC_OFF_REPORT_TIME://连接2 ACC OFF汇报时间间隔
		{
			
			if(E2_SECOND_ACC_OFF_REPORT_TIME_ID_LEN == EepromPram_ReadPram(E2_SECOND_ACC_OFF_REPORT_TIME_ID,Buffer))
			{
				Link2AccOffReportTimeSpace = Public_ConvertBufferToLong(Buffer);
			}
			
			break;
		}
	case PRAM_SECOND_LINK_OPEN_FLAG://连接2 开启标志
		{	
			BufferLen = EepromPram_ReadPram(E2_SECOND_MAIN_SERVER_IP_ID, Buffer);
			if(0 == BufferLen)
			{
				Link2OpenFlag = 0;
			}
			else
			{
				BufferLen = EepromPram_ReadPram(E2_SECOND_MAIN_SERVER_TCP_PORT_ID, Buffer);
				if(E2_SECOND_MAIN_SERVER_TCP_PORT_ID_LEN == BufferLen)
				{
					TcpPort = Public_ConvertBufferToLong(Buffer);
					if(0 == TcpPort)
					{
						Link2OpenFlag = 0;
					}
					else
					{
						Link2OpenFlag = 1;
					}
				}
				else
				{
					Link2OpenFlag = 0;
				}
		
			}
			
			break;
		}
	
		default:break;
	}
}
/**************************************************************************
//函数名：Report_UploadPositionInfo
//功能：上传一条位置信息
//输入：通道号
//输出：无
//返回值：0上传成功，1上传失败
//备注：当前的位置信息==位置基本信息+位置附加信息，位置信息最长为150字节
***************************************************************************/
u8 Report_UploadPositionInfo(u8 channel)
{
	u8 Buffer[FLASH_BLIND_STEP_LEN+1];
	u8 BufferLen;
	u8 flag1;
	u8 flag2;
	BufferLen = Report_GetPositionInfo(Buffer);

	if(CHANNEL_DATA_1 == channel)
	{

		if(ACK_OK == RadioProtocol_PostionInformationReport(channel,Buffer,BufferLen))
		{
			return 0;
		}
		else
		{
			Blind_Save(channel, Buffer, BufferLen);
			return 1;
		}
	}
	else if(CHANNEL_DATA_2 == channel)
	{
		if(1 == Link2OpenFlag)
		{
			if(ACK_OK == RadioProtocol_PostionInformationReport(channel,Buffer,BufferLen))
			{
				return 0;
			}
			else
			{
				Blind_Save(channel, Buffer, BufferLen);
				return 1;
			}
		}
		else
		{
			return 1;
		}
	}
	else if(0x06 == channel)
	{
		if(ACK_OK == RadioProtocol_PostionInformationReport(CHANNEL_DATA_1,Buffer,BufferLen))
		{
			flag1 = 0;
		}
		else
		{
			Blind_Save(CHANNEL_DATA_1, Buffer, BufferLen);
			flag1 = 1;
		}

		if(ACK_OK == RadioProtocol_PostionInformationReport(CHANNEL_DATA_2,Buffer,BufferLen))
		{
			flag2 = 0;
		}
		else
		{
			Blind_Save(CHANNEL_DATA_2, Buffer, BufferLen);
			flag2 = 1;
		}

		if((0 == flag1)&&(0 == flag2))
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}
	return 0;
}
/**************************************************************************
//函数名：Report_GetPositionInfo
//功能：获取当前的位置信息
//输入：无
//输出：位置信息
//返回值：位置信息长度
//备注：当前的位置信息==位置基本信息+位置附加信息，位置信息最长为150字节
***************************************************************************/
u16 Report_GetPositionInfo(u8 *pBuffer)
{
	u8 *p = NULL;
	u16 length;

	p = pBuffer;
	length = 0;

	length = Report_GetPositionBasicInfo(p);//获取位置基本信息
	p = p+length;

	length += Report_GetPositionAdditionalInfo(p);//获取位置附加信息

	return length;

}
/**************************************************************************
//函数名：Report_GetPositionBasicInfo
//功能：获取位置基本信息
//输入：无
//输出：位置基本信息
//返回值：位置基本信息长度
//备注：长度固定为28字节，位置基本信息：报警字、状态字、纬度、经度、高程、速度、方向、时间
***************************************************************************/
u16 Report_GetPositionBasicInfo(u8 *pBuffer)
{
	u8 *p = NULL;
	u32 temp;
	u16 temp2;
	GPS_STRUCT Position;
    TIME_T time;
	u32 timecount;
	p = pBuffer;
	Gps_CopygPosition(&Position);
	
	temp = Io_ReadAlarm();//报警字
	Public_ConvertLongToBuffer(temp,p);
	p += 4;

	temp = Io_ReadStatus();//状态字
	Public_ConvertLongToBuffer(temp,p);
	p += 4;

	//不导航经纬度置0   ?????
	//if(!Io_ReadStatusBit(STATUS_BIT_NAVIGATION))
	//{
	//	temp = 0ul;
	//}
	//else
	{
		temp = (Position.Latitue_FX*100UL+Position.Latitue_F*1000000)/60+(Position.Latitue_D*1000000UL);//纬度
	}
	Public_ConvertLongToBuffer(temp,p);
	p += 4;

	//if(!Io_ReadStatusBit(STATUS_BIT_NAVIGATION))
	//{
	//	temp = 0ul;
	//}
	//else
	{
		temp = (Position.Longitue_FX*100UL+Position.Longitue_F*1000000)/60+(Position.Longitue_D*1000000UL);//经度
	}
	Public_ConvertLongToBuffer(temp,p);
	p += 4;

	temp2 = Position.High+Position.HighOffset;//高程
	if((Position.High+Position.HighOffset)<0)temp2 = 0;//负海拔置0
	Public_ConvertShortToBuffer(temp2,p);
	p += 2;

        temp2 = 10*Gps_ReadSpeed();//速度
	Public_ConvertShortToBuffer(temp2,p);
	p += 2;

	temp2 = Position.Course;//方向
	Public_ConvertShortToBuffer(temp2,p);
	p += 2;
	
	time.year = Position.Year;
	time.month = Position.Month;
	time.day = Position.Date;
	time.hour = Position.Hour;
	time.min = Position.Minute;
	time.sec = Position.Second;
	timecount = ConverseGmtime(&time);
	timecount += 8*3600;
	timecount -= 3;//由于比省运管平台时间快了，这里减去3秒
	Gmtime(&time, timecount);

	//GPS定位和acc on时采用GPS时间
	if((1==Io_ReadStatusBit(STATUS_BIT_NAVIGATION))
			&&(1==Io_ReadStatusBit(STATUS_BIT_ACC)))
    {
        *p++ = Public_HEX2BCD(time.year);//GPS时间
        *p++ = Public_HEX2BCD(time.month);
        *p++ = Public_HEX2BCD(time.day);
        *p++ = Public_HEX2BCD(time.hour);      
        *p++ = Public_HEX2BCD(time.min);
        *p++ = Public_HEX2BCD(time.sec);
	}
	else
	{
		*p++ = Public_HEX2BCD(CurTime.year);//读取时间
		*p++ = Public_HEX2BCD(CurTime.month);
		*p++ = Public_HEX2BCD(CurTime.day);
		*p++ = Public_HEX2BCD(CurTime.hour);
		*p++ = Public_HEX2BCD(CurTime.min);
		*p++ = Public_HEX2BCD(CurTime.sec);
	}
	return 28;
	
}
/**************************************************************************
//函数名：Report_ClearReportTimeCount
//功能：上报时间计数变量清0
//输入：无
//输出：无
//返回值：无
//备注：两个连接的计数变量都清0
***************************************************************************/
void Report_ClearReportTimeCount(void)
{
	Link1ReportTimeCount = 0;
	Link2ReportTimeCount = 0;
}
/**************************************************************************
//函数名：Report_Link2ReportOpenFlag
//功能：连接2自定义汇报是否需要开启
//输入：无
//输出：1：需要开启；0：不需要开启
//返回值：无
//备注：
***************************************************************************/
static u8 Report_Link2ReportOpenFlag(void)
{
	if((TempTrackTimeSpace > 0)&&(TempTrackCount > 0))//临时跟踪
	{
		return 0;
	}
	else if(1 == Io_ReadAlarmBit(ALARM_BIT_EMERGENCY))//紧急报警
	{
		return 0;
	}
	else if((1 == Io_ReadAlarmBit(ALARM_BIT_OVER_SPEED))&&(0 != OverSpeedReportTimeSpace))//超速报警
	{
		return 0;
	}
	else if((1 == Io_ReadAlarmBit(ALARM_BIT_TIRED_DRIVE))&&(0 != OverTimeReportTimeSpace))//超时报警
	{
		return 0;
	}
	else if((0 != Link2AccOnReportTimeSpace)&&(0 != Link2AccOffReportTimeSpace))//连接2参数已设置
	{
		return 1;
	}
	else
	{
		return 0;
	}

	
}
/**************************************************************************
//函数名：Report_GetLink1NextReportPoint
//功能：获取连接1的下一个位置汇报点（汇报时间和汇报距离）
//输入：无
//输出：无
//返回值：无
//备注：
***************************************************************************/
static void Report_GetLink1NextReportPoint(u32 *NextReportTime, u32 *NextReportDistance)
{

	*NextReportTime = 0;
	*NextReportDistance = 0;

	if((TempTrackTimeSpace > 0)&&(TempTrackCount > 0))//临时跟踪
	{
		*NextReportTime = TempTrackTimeSpace;
		*NextReportDistance = 0;
		Link1ReportDistanceSpace = 0;
	}
	else
	{
		if(1 == Io_ReadAlarmBit(ALARM_BIT_EMERGENCY))//紧急报警
		{
			if(0 == ReportStrategy)//定时
			{
				*NextReportTime = EmergencyReportTimeSpace;
				*NextReportDistance = 0;
				Link1ReportDistanceSpace = 0;
			}
			else if(1 == ReportStrategy)//定距
			{
				*NextReportTime = 0;
				*NextReportDistance = EmergencyReportDistanceSpace+Link1LastReportDistance;
				Link1ReportDistanceSpace = EmergencyReportDistanceSpace;
			}
			else if(2 == ReportStrategy)//定时+定距
			{
				*NextReportTime = EmergencyReportTimeSpace;
				*NextReportDistance = EmergencyReportDistanceSpace+Link1LastReportDistance;
				Link1ReportDistanceSpace = EmergencyReportDistanceSpace;
			}
		}
		else if((1 == Io_ReadAlarmBit(ALARM_BIT_OVER_SPEED))&&(0 != OverSpeedReportTimeSpace))//超速报警
		{
			*NextReportTime = OverSpeedReportTimeSpace;
			*NextReportDistance = 0;
			Link1ReportDistanceSpace =  0;
		}
		else if((1 == Io_ReadAlarmBit(ALARM_BIT_TIRED_DRIVE))&&(0 != OverTimeReportTimeSpace))//超时报警
		{
			*NextReportTime = OverTimeReportTimeSpace;
			*NextReportDistance = 0;
			Link1ReportDistanceSpace = 0;
		}
		else 
		{
			if(0 == ReportScheme)//根据ACC状态
			{
				if(1 == Io_ReadStatusBit(STATUS_BIT_ACC))//ACC ON
				{
					if(0 == ReportStrategy)//定时
					{
						*NextReportTime = AccOnReportTimeSpace;
						*NextReportDistance = 0;
						Link1ReportDistanceSpace = 0;
					}
					else if(1 == ReportStrategy)//定距
					{
						*NextReportTime = 0;
						*NextReportDistance = AccOnReportDistanceSpace+Link1LastReportDistance;
						Link1ReportDistanceSpace = AccOnReportDistanceSpace;
					}
					else if(2 == ReportStrategy)//定时+定距
					{
						*NextReportTime = AccOnReportTimeSpace;
						*NextReportDistance = AccOnReportDistanceSpace+Link1LastReportDistance;
						Link1ReportDistanceSpace = AccOnReportDistanceSpace;
					}
					else
					{
						*NextReportTime = 0;
						*NextReportDistance = 0;
						Link1ReportDistanceSpace = 0;
					}
				}
				else//ACC OFF
				{
					if(0 == ReportStrategy)//定时
					{
						*NextReportTime = AccOffReportTimeSpace;
						*NextReportDistance = 0;
						Link1ReportDistanceSpace = 0;
					}
					else if(1 == ReportStrategy)//定距
					{
						*NextReportTime = 0;
						*NextReportDistance = AccOffReportDistanceSpace+Link1LastReportDistance;
						Link1ReportDistanceSpace = AccOffReportDistanceSpace;
					}
					else if(2 == ReportStrategy)//定时+定距
					{
						*NextReportTime = AccOffReportTimeSpace;
						*NextReportDistance = AccOffReportDistanceSpace+Link1LastReportDistance;
						Link1ReportDistanceSpace = AccOffReportDistanceSpace;
					}
					else
					{
						*NextReportTime = 0;
						*NextReportDistance = 0;
						Link1ReportDistanceSpace = 0;
					}
				}
			}
			else if(1 == ReportScheme)//根据ACC状态和登录状态，先判断登录状态，再根据ACC
			{
				if(0 == NaviLcdDriverLoginFlag())//驾驶员未登录
				{
					if(0 == ReportStrategy)//定时
					{
						*NextReportTime = UnloginReportTimeSpace;
						*NextReportDistance = 0;
						Link1ReportDistanceSpace = 0;
					}
					else if(1 == ReportStrategy)//定距
					{
						*NextReportTime = 0;
						*NextReportDistance = UnloginReportDistanceSpace+Link1LastReportDistance;
						Link1ReportDistanceSpace = UnloginReportDistanceSpace;
					}
					else if(2 == ReportStrategy)//定时+定距
					{
						*NextReportTime = UnloginReportTimeSpace;
						*NextReportDistance = UnloginReportDistanceSpace+Link1LastReportDistance;
						Link1ReportDistanceSpace = UnloginReportDistanceSpace;
					}
					else
					{
						*NextReportTime = 0;
						*NextReportDistance = 0;
						Link1ReportDistanceSpace = 0;
					}
				}
				else//驾驶员已登录
				{
					if(1 == Io_ReadStatusBit(STATUS_BIT_ACC))//ACC ON
					{
						if(0 == ReportStrategy)//定时
						{
							*NextReportTime = AccOnReportTimeSpace;
							*NextReportDistance = 0;
							Link1ReportDistanceSpace = 0;
						}
						else if(1 == ReportStrategy)//定距
						{
							*NextReportTime = 0;
							*NextReportDistance = AccOnReportDistanceSpace+Link1LastReportDistance;
							Link1ReportDistanceSpace = AccOnReportDistanceSpace;
						}
						else if(2 == ReportStrategy)//定时+定距
						{
							*NextReportTime = AccOnReportTimeSpace;
							*NextReportDistance = AccOnReportDistanceSpace+Link1LastReportDistance;
							Link1ReportDistanceSpace = AccOnReportDistanceSpace;
						}
						else
						{
							*NextReportTime = 0;
							*NextReportDistance = 0;
							Link1ReportDistanceSpace = 0;
						}
					}
					else//ACC OFF
					{
						if(0 == ReportStrategy)//定时
						{
							*NextReportTime = AccOffReportTimeSpace;
							*NextReportDistance = 0;
							Link1ReportDistanceSpace = 0;
						}
						else if(1 == ReportStrategy)//定距
						{
							*NextReportTime = 0;
							*NextReportDistance = AccOffReportDistanceSpace+Link1LastReportDistance;
							Link1ReportDistanceSpace = AccOffReportDistanceSpace;
						}
						else if(2 == ReportStrategy)//定时+定距
						{
							*NextReportTime = AccOffReportTimeSpace;
							*NextReportDistance = AccOffReportDistanceSpace+Link1LastReportDistance;
							Link1ReportDistanceSpace = AccOffReportDistanceSpace;
						}
						else
						{
							*NextReportTime = 0;
							*NextReportDistance = 0;
							Link1ReportDistanceSpace = 0;
						}
					}

				}
			}
			else
			{
				*NextReportTime = 0;
				*NextReportDistance = 0;
				Link1ReportDistanceSpace = 0;
			}
		}
	}
	Link2ReportDistanceSpace = Link1ReportDistanceSpace;
	Link1ReportTimeSpace = *NextReportTime;
}
/**************************************************************************
//函数名：Report_GetCurDistance
//功能：获取当前的距离（累计行驶里程）
//输入：无
//输出：无
//返回值：距离（累计行驶里程）
//备注：单位米
***************************************************************************/
static u32 Report_GetCurDistance(void)
{
	u32 CurMile;

	if(0 == SpeedMonitor_GetCurSpeedType())
	{
		CurMile = Pulse_GetTotalMile()*10;
	}
	else
	{
		CurMile = MileMeter_GetTotalMile()*10;
	}

	return CurMile;
}
/**************************************************************************
//函数名：Report_GetPositionAdditionalInfo
//功能：获取位置附加信息
//输入：无
//输出：位置附加信息
//返回值：位置附加信息长度
//备注：
***************************************************************************/
static u16 Report_GetPositionAdditionalInfo(u8 *pBuffer)
{
	u8 *p = NULL;
	u16 length;

	p = pBuffer;
	length = 0;

	length = Report_GetPositionStdAdditionalInfo(p);//获取标准的位置附加信息
	p = p+length;

	length += Report_GetPositionEiAdditionalInfo(p);//获取伊爱自定义的位置附加信息

	return length;	
}
/**************************************************************************
//函数名：Report_GetPositionStdAdditionalInfo
//功能：获取标准的位置附加信息
//输入：无
//输出：标准的位置附加信息
//返回值：标准的位置附加信息长度
//备注：附加信息ID:0x01~0x04,0x11~0x13,0x25,0x2a,0x2b,0x30,0x31
***************************************************************************/
static u16 Report_GetPositionStdAdditionalInfo(u8 *pBuffer)
{
	u8 *p = NULL;
	u16 length;
	u32 temp;
	u16 temp2;
	u16 temp3;
	u16 TempLen;

	p = pBuffer;
	length = 0;

	*p++ = 0x01;//里程，ID为0x01	
	*p++ = 4;
	temp = Public_GetCurTotalMile();
	Public_ConvertLongToBuffer(temp,p);
	p += 4;
	length += 6;

	TempLen = JointechOiLCost_GetSubjoinInfoCurOilVolume(p);//油量，ID为0x02，玖通油量，两者二选一
	p += TempLen;
	length += TempLen;
	TempLen = OilWear_GetSubjoinInfoCurOilVal(p);//油量，ID为0x02，博实结油量
	p += TempLen;
	length += TempLen;

	*p++ = 0x03;//行驶记录仪速度，ID为0x03，此处与标准要求不一样，做了处理
	*p++ = 0x02;//（按广州二汽需求处理：当脉冲速度不为0时使用脉冲速度，脉冲速度为0时使用GPS速度，这是之前的需求）
  
        if(0 == (SpeedFlag&0x01))//（按广州二汽需求处理：当选择为脉冲速度类型时，上传脉冲速度；当选择为GPS速度类型时，脉冲不为0时上传脉冲，为0时上传GPS速度，这是现在的需求）
	{
		temp2 = 10*Pulse_GetSpeed();
	}
	else//GPS速度 
	{
                if(0 == Pulse_GetSpeed())
                {
                    temp2 = 10*Gps_ReadSpeed();
                }
                else
                {
                    temp2 = 10*Pulse_GetSpeed();
                }
	}
	Public_ConvertShortToBuffer(temp2,p);
	p += 2;
	length += 4;

	//需要人工确认报警时间的ID，ID为0x04，此项暂无
	
	TempLen = SpeedMonitor_GetCurOverSpeedSubjoinInfo(p);//超速报警附加信息，ID为0x11
	p += TempLen;
	length += TempLen;

	TempLen = Area_GetInOutAreaAlarmSubjoinInfo(p);//进出区域报警附加信息，ID为0x12
	p += TempLen;
	length += TempLen;

	TempLen = Route_GetDriveAlarmInfoSubjoinInfo(p);//进出路线报警附加信息，ID为0x13
	p += TempLen;
	length += TempLen;

	
	*p++ = 0x25;//扩展车辆信号状态，ID为0x25	
	*p++ = 4;
	temp = Io_ReadExtCarStatus();
	Public_ConvertLongToBuffer(temp,p);
	p += 4;
	length += 6;

	*p++ = 0x2a;//IO状态，ID为0x2a	
	*p++ = 2;
	temp2 = Io_ReadIoStatus();
	Public_ConvertShortToBuffer(temp2,p);
	p += 2;
	length += 4;

	temp2 = Ad_GetValue(ADC_EXTERN1);
	temp3 = Ad_GetValue(ADC_EXTERN2);
	*p++ = 0x2b;//模拟量，ID为0x2b，高16位为模拟量2，低16位为模拟量1
	*p++ = 4;
	Public_ConvertShortToBuffer(temp3,p);
	p += 2;
	Public_ConvertShortToBuffer(temp2,p);
	p += 2;
	length += 6;

	*p++ = 0x30;//无线网络信号强度，ID为0x30	
	*p++ = 1;
	*p++ = communicatio_GetSignalIntensity();
	length += 3;

	*p++ = 0x31;//定位星数，ID为0x31	
	*p++ = 1;
	*p++ = Gps_ReadStaNum();
	length += 3;
	
	return length;
	
}
/**************************************************************************
//函数名：Report_GetPositionEiAdditionalInfo
//功能：获取伊爱自定义的位置附加信息
//输入：无
//输出：伊爱自定义的位置附加信息
//返回值：伊爱自定义的位置附加信息长度
//备注：
***************************************************************************/
static u16 Report_GetPositionEiAdditionalInfo(u8 *pBuffer)
{
	u8 length;
	u8 *p = NULL;
	u32 temp;
	u8 VerifyCode;
	u8 TempLen;

	length = 0;
	p = pBuffer;

	p += 2;//空开伊爱附加ID，标志符1字节，长度1字节

	*p++ = 0xE1;//自定义状态字2，ID为0xE1
	*p++ = 4;
	temp = Io_ReadSelfDefine2();
	Public_ConvertLongToBuffer(temp,p); 
	p += 4;
	length += 6;

	TempLen = CarLoad_GetCurWeightSubjoinInfo(p);//载重重量，ID为0xE2
	p += TempLen;
	length += TempLen;

	TempLen = JointechOiLCost_GetSubjoinInfoCurOilLevelPercent(p);//油位高度万分比，ID为0xE4
	p += TempLen;
	length += TempLen;

	TempLen = OilWear_GetSubjoinInfoCurOilResVal(p);//博实结油量电阻值，ID为0xE5
	p += TempLen;
	length += TempLen;

	TempLen = Temperature_PosEx(p);//温度采集器，ID为0xE8
	p += TempLen;
	length += TempLen;

        //zengliang add
	TempLen = Acceleration_PosEx(p);//急加速附加ID，ID为0xE9
	p += TempLen;
	length += TempLen;
        // add end 

	VerifyCode = Public_GetSumVerify(pBuffer+2,length);//校验和
	*p++ = VerifyCode;
	length++;

	*pBuffer = 0xF0;//伊爱扩展附加信息标志
	*(pBuffer+1) = length;

	return length+2;
	
}

































/************************************************************************
//�������ƣ�Report.c
//���ܣ���ģ��ʵ�ֶ�ʱ�ϱ��������ϱ�����ʱ+�����ϱ���ʵʱ���ٹ��ܣ�֧��˫���ӵ�λ�û㱨��
//�汾�ţ�V0.1
//��Ȩ˵������Ȩ�����������������¼����������޹�˾
//�����ˣ�������
//����ʱ�䣺2014.10
//�汾��¼��

//��ע���汾��¼������汾���޸��ˡ��޸�ʱ�䡢�޸�ԭ���޸ļ�Ҫ˵��
//V0.1����Ҫʵ��˫���ӵ�λ�û㱨������1�����б��׼����λ�û㱨������2��TCP�˿�������Ϊ0��ʾ�رյ�2�����ӣ�
//��Ҳ��ر�����2��λ�û㱨����Ϊ��0ʱ��ʾ��������2������2�����ӿ����󣬵�2�����ӵ�ACC ON��ACC OFF�ϱ�ʱ��������Ϊ��0��
//���ʾ������2�����ӵ������ϱ����ܣ���������1�ķ�ʽ�ϱ���
*************************************************************************/

/********************�ļ�����*************************/
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


/********************���ر���*************************/
static u32 Link1ReportTime = 0;//����1λ�û㱨ʱ��������λ��
static u32 Link1ReportTimeCount = 0;//����1λ�û㱨ʱ�����
static u32 Link1LastReportDistance = 0;//����1��һ���ϱ���ľ��룬��λ��
static u32 Link1ReportDistance = 0;//����1��һ���ϱ���ľ���
static u8 Link1ReportAckFlag = 0;//����1��һ��λ����ϢӦ���־��1��ʾ��Ӧ��0��ʾ��Ӧ��
static u8 Link1ReportBuffer[FLASH_BLIND_STEP_LEN] = {0};//����1λ���ϱ�����
static u8 Link1ReportBufferLen = 0;//����1λ���ϱ��������ݳ���
static u32 Link2AccOnReportTimeSpace = 0;//����2 ACC ONλ�û㱨ʱ������Ϊ0��ʾ�رոù���
static u32 Link2AccOffReportTimeSpace = 0;//����2 ACC OFFλ�û㱨ʱ������Ϊ0��ʾ�رոù���
static u8 Link2OpenFlag = 0;//ͨ��2������־��1Ϊ������0Ϊ�ر�
static u32 Link2ReportTime = 0;//����2 λ�û㱨ʱ��������λ��
static u32 Link2ReportTimeCount = 0;//����2 λ�û㱨ʱ�����
static u8 Link2ReportAckFlag = 0;//��һ��λ����ϢӦ���־��1��ʾ��Ӧ��0��ʾ��Ӧ��
static u8 Link2ReportBuffer[FLASH_BLIND_STEP_LEN] = {0};//λ���ϱ�����
static u8 Link2ReportBufferLen = 0;//λ���ϱ��������ݳ���
static u32 Link2ReportDistanceSpace = 0;//����2λ���ϱ�ʱ��������λ��
static u32 ReportStrategy = 0;//�㱨����
static u32 ReportScheme = 0;//�㱨����
static u32 EmergencyReportTimeSpace = 0;//���������㱨ʱ����
static u32 EmergencyReportDistanceSpace = 0;//���������㱨������
static u32 OverSpeedReportTimeSpace = 0;//���ٱ����㱨ʱ����
static u32 OverTimeReportTimeSpace = 0;//��ʱ�����㱨ʱ����
static u32 AccOnReportTimeSpace = 0;//ȱʡ�㱨ʱ����
static u32 AccOnReportDistanceSpace = 0;//ȱʡ�㱨ʱ����
static u32 AccOffReportTimeSpace = 0;//���߻㱨ʱ����
static u32 AccOffReportDistanceSpace = 0;//���߻㱨ʱ����
static u32 UnloginReportTimeSpace = 0;//��ʻԱδ��¼�㱨ʱ����
static u32 UnloginReportDistanceSpace = 0;//��ʻԱδ��¼�㱨ʱ����
static u32 TempTrackTimeSpace = 0;//��ʱ����ʱ����
static u32 TempTrackCount = 0;//��ʱ���ٴ���

/********************ȫ�ֱ���*************************/
u16 Link1LastReportSerialNum = 0;//����1��һ��λ���ϱ�����ˮ��
u16 Link2LastReportSerialNum = 0;//����2��һ��λ���ϱ�����ˮ��
u32 Link1ReportTimeSpace = 0;//ͬLink1ReportTimeһ���������ⲿ����
u32 Link1ReportDistanceSpace = 0;//����1λ���ϱ�ʱ��������λ��
/********************�ⲿ����*************************/
extern TIME_T CurTime;
extern u8	SpeedFlag;//�ٶ�����,dxl,2015.5.11,0x00:�ֶ�����,0x01:�ֶ�GPS,0x02:�Զ�����,0x03:�Զ�GPS
/********************���غ�������*********************/
static void Report_GetLink1NextReportPoint(u32 *NextReportTime, u32 *NextReportDistance);//��ȡ��һ���ϱ���
static u32 Report_GetCurDistance(void);//��ȡ��ǰ���ۼ���ʻ���
static u16 Report_GetPositionAdditionalInfo(u8 *pBuffer);//��ȡλ�ø�����Ϣ
static u16 Report_GetPositionStdAdditionalInfo(u8 *pBuffer);//��ȡ��׼��λ�ø�����Ϣ
static u16 Report_GetPositionEiAdditionalInfo(u8 *pBuffer);//��ȡ�����Զ����λ�ø�����Ϣ
static u8 Report_Link2ReportOpenFlag(void);//����2�����ϱ�������־��1Ϊ������0Ϊ�ر�
/********************��������*************************/

/**************************************************************************
//��������Report_TimeTask
//���ܣ�ʵ��λ�û㱨������ʵʱ���١��������������ٱ�������ʱ��������ʱ�����λ�û㱨��
//���룺��
//�������
//����ֵ��ʼ����ENABLE
//��ע��λ�û㱨��ʱ����50ms����1�Σ������������Ҫ���ô˺���
***************************************************************************/
FunctionalState Report_TimeTask(void)
{
	u32 CurTimeCount = 0;
	u32 Link1CurDistance = 0;//��ǰλ�õ����
	u8 Link1ReportFlag = 0;//λ���ϱ���־��1��ʾҪ�ϱ���0��ʾ�����ϱ�
	u8 Link2ReportFlag = 0;
	u8 Link2ReportStatus = 0;//��2�������ϱ�״̬��0��ʾ��ֹ�ϱ���1��ʾ������1�ķ�ʽ�ϱ���2��ʾ������2�����ⷽʽ�ϱ�
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

	count++;//dxl,2016.5.7����
    if(count < 1200)//������ǰ60�벻�ϱ�λ����Ϣ����Ϊ���ʱ�����������̣��϶��ϱ�����ȥ������Ϊä����¼�ģ�����ÿ�������ͻ���������ä����Ϊ�����������������60����ʱ
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
            Link2ReportStatus = 2;//�ѿ�������2������㱨
        }
        else
        {
            Link2ReportStatus = 1;//�ѿ�����2�����ӣ�������1�ķ�ʽ�㱨
        }
    }
    else
    {
        Link2ReportStatus = 0;//�ѹر�����2
    }

	Link1ReportTimeCount += ReportAddTimeCount;
	
	Report_GetLink1NextReportPoint(&Link1ReportTime,&Link1ReportDistance);

	if(0 != Link1ReportTime)//Link1ReportTime����0��ʾ��������ʱ�ϱ�
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

	if(0 != Link1ReportDistance)//Link1ReportDistance����0��ʾ�����������ϱ�
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
                    		||((0 == Link1ReportAckFlag)&&(Link1ReportDistanceSpace >= 100)))//��һ��λ����Ϣ�㱨�Ƿ��յ�Ӧ��1��ʾ�յ�
			{
				//if(1 == Gps_ReadStatus())//ֻ�е����Ż��ϱ���״̬�򱨾��仯Ҳ���ϱ����������������
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
				//if(1 == Gps_ReadStatus())//ֻ�е����Ż��ϱ���״̬�򱨾��仯Ҳ���ϱ����������������
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

		if(0 != Link2ReportTime)//Link2ReportTime����0��ʾ��������ʱ�ϱ�
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
                    		||((0 == Link2ReportAckFlag)&&(Link2ReportDistanceSpace >= 100)))//��һ��λ����Ϣ�㱨�Ƿ��յ�Ӧ��1��ʾ�յ�
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
				
				//if(1 == Gps_ReadStatus())//ֻ�е����Ż��ϱ���״̬�򱨾��仯Ҳ���ϱ����������������
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
//��������Report_UpdatePram
//���ܣ�����λ�û㱨���б���
//���룺��
//�������
//����ֵ����
//��ע����Ҫ�Ǹ���λ�û㱨ģ����ʹ�õ��ı����������������֮ǰ����ô˺���
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
//��������Report_CmdAck
//���ܣ�λ���ϱ�Ӧ����
//���룺ͨ����
//�������
//����ֵ����
//��ע���յ�λ���ϱ���ͨ��Ӧ��ʱ����ô˺���
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
//��������Report_UpdateOnePram
//���ܣ�����ĳ������
//���룺��������
//������ò������Ͷ�Ӧ�ı���
//����ֵ����
//��ע�����ڻ�Զ������λ�û㱨��ز���ʱ��Ҫ���ô˺���
***************************************************************************/
void Report_UpdateOnePram(u8 type)
{
	u8 Buffer[30];
	u8 BufferLen;
	u32 TcpPort;
	
	switch(type)
	{
	case PRAM_REPORT_STRATEGY://λ�û㱨����
		{
			if(E2_POSITION_REPORT_STRATEGY_LEN == EepromPram_ReadPram(E2_POSITION_REPORT_STRATEGY_ID,Buffer))
			{
				ReportStrategy = Public_ConvertBufferToLong(Buffer);
			}
			break;
		}
	case PRAM_REPORT_SCHEME://λ�û㱨����
		{
			if(E2_POSITION_REPORT_SCHEME_LEN == EepromPram_ReadPram(E2_POSITION_REPORT_SCHEME_ID,Buffer))
			{
				ReportScheme = Public_ConvertBufferToLong(Buffer);
			}
			break;
		}
	case PRAM_TEMP_TRACK_TIME://��ʱ����ʱ����
		{
			if(FRAM_TEMP_TRACK_SPACE_LEN == FRAM_BufferRead(Buffer, 2, FRAM_TEMP_TRACK_SPACE_ADDR))
			{
				TempTrackTimeSpace = Public_ConvertBufferToShort(Buffer);
			}
			break;
		}
	case PRAM_TEMP_TRACK_COUNT://��ʱ���ٴ���
		{
			if(FRAM_TEMP_TRACK_NUM_LEN == FRAM_BufferRead(Buffer, 4, FRAM_TEMP_TRACK_NUM_ADDR))
			{
				TempTrackCount = Public_ConvertBufferToLong(Buffer);
			}
			break;
		}
	case PRAM_EMERGENCY_REPORT_TIME://���������㱨ʱ����
		{
			if(E2_EMERGENCY_REPORT_TIME_LEN == EepromPram_ReadPram(E2_EMERGENCY_REPORT_TIME_ID,Buffer))
			{
				EmergencyReportTimeSpace = Public_ConvertBufferToLong(Buffer);
			}
			break;
		}
	case PRAM_EMERGENCY_REPORT_DISTANCE://���������㱨������
		{
			if(E2_EMERGENCY_REPORT_DISTANCE_LEN == EepromPram_ReadPram(E2_EMERGENCY_REPORT_DISTANCE_ID,Buffer))
			{
				EmergencyReportDistanceSpace = Public_ConvertBufferToLong(Buffer);
			}
			break;
		}
	case PRAM_OVER_SPEED_REPORT_TIME://���ٱ����㱨ʱ����
		{
			if(E2_OVERSPEED_ALARM_REPORT_TIME_ID_LEN == EepromPram_ReadPram(E2_OVERSPEED_ALARM_REPORT_TIME_ID,Buffer))
			{
				OverSpeedReportTimeSpace = Public_ConvertBufferToLong(Buffer);
			}
			break;
		}
	case PRAM_OVER_TIME_REPORT_TIME://��ʱ�����㱨ʱ����
		{
			if(E2_TIRED_DRIVE_REPORT_TIME_ID_LEN == EepromPram_ReadPram(E2_TIRED_DRIVE_REPORT_TIME_ID,Buffer))
			{
				OverTimeReportTimeSpace = Public_ConvertBufferToLong(Buffer);
			}
			break;
		}
	case PRAM_ACC_ON_REPORT_TIME://ȱʡ�㱨ʱ����
		{
			if(E2_ACCON_REPORT_TIME_LEN == EepromPram_ReadPram(E2_ACCON_REPORT_TIME_ID,Buffer))
			{
				AccOnReportTimeSpace = Public_ConvertBufferToLong(Buffer);
			}
			break;
		}
	case PRAM_ACC_ON_REPORT_DISTANCE://ȱʡ�㱨������
		{
			if(E2_ACCON_REPORT_DISTANCE_LEN == EepromPram_ReadPram(E2_ACCON_REPORT_DISTANCE_ID,Buffer))
			{
				AccOnReportDistanceSpace = Public_ConvertBufferToLong(Buffer);
			}
			break;
		}
	case PRAM_ACC_OFF_REPORT_TIME://���߻㱨ʱ����
		{
			if(E2_SLEEP_REPORT_TIME_LEN == EepromPram_ReadPram(E2_SLEEP_REPORT_TIME_ID,Buffer))
			{
				AccOffReportTimeSpace = Public_ConvertBufferToLong(Buffer);
			}
			break;
		}
	case PRAM_ACC_OFF_REPORT_DISTANCE://���߻㱨������
		{
			if(E2_SLEEP_REPORT_DISTANCE_LEN == EepromPram_ReadPram(E2_SLEEP_REPORT_DISTANCE_ID,Buffer))
			{
				AccOffReportDistanceSpace = Public_ConvertBufferToLong(Buffer);
			}
			break;
		}
	case PRAM_UNLOGIN_REPORT_TIME://��ʻԱδ��¼�㱨ʱ����
		{
			if(E2_DRIVER_UNLOGIN_REPORT_TIME_LEN == EepromPram_ReadPram(E2_DRIVER_UNLOGIN_REPORT_TIME_ID,Buffer))
			{
				UnloginReportTimeSpace = Public_ConvertBufferToLong(Buffer);
			}
			break;
		}
	case PRAM_UNLOGIN_REPORT_DISTANCE://��ʻԱδ��¼�㱨������
		{
			if(E2_DRIVER_UNLOGIN_REPORT_DISTANCE_LEN == EepromPram_ReadPram(E2_DRIVER_UNLOGIN_REPORT_DISTANCE_ID,Buffer))
			{
				UnloginReportDistanceSpace = Public_ConvertBufferToLong(Buffer);
			}
			break;
		}
	case PRAM_SECOND_LINK_ACC_ON_REPORT_TIME://����2 ACC ON�㱨ʱ����
		{
			if(E2_SECOND_ACC_ON_REPORT_TIME_ID_LEN == EepromPram_ReadPram(E2_SECOND_ACC_ON_REPORT_TIME_ID,Buffer))
			{
				Link2AccOnReportTimeSpace = Public_ConvertBufferToLong(Buffer);
			}
			
			break;
		}
	case PRAM_SECOND_LINK_ACC_OFF_REPORT_TIME://����2 ACC OFF�㱨ʱ����
		{
			
			if(E2_SECOND_ACC_OFF_REPORT_TIME_ID_LEN == EepromPram_ReadPram(E2_SECOND_ACC_OFF_REPORT_TIME_ID,Buffer))
			{
				Link2AccOffReportTimeSpace = Public_ConvertBufferToLong(Buffer);
			}
			
			break;
		}
	case PRAM_SECOND_LINK_OPEN_FLAG://����2 ������־
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
//��������Report_UploadPositionInfo
//���ܣ��ϴ�һ��λ����Ϣ
//���룺ͨ����
//�������
//����ֵ��0�ϴ��ɹ���1�ϴ�ʧ��
//��ע����ǰ��λ����Ϣ==λ�û�����Ϣ+λ�ø�����Ϣ��λ����Ϣ�Ϊ150�ֽ�
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
//��������Report_GetPositionInfo
//���ܣ���ȡ��ǰ��λ����Ϣ
//���룺��
//�����λ����Ϣ
//����ֵ��λ����Ϣ����
//��ע����ǰ��λ����Ϣ==λ�û�����Ϣ+λ�ø�����Ϣ��λ����Ϣ�Ϊ150�ֽ�
***************************************************************************/
u16 Report_GetPositionInfo(u8 *pBuffer)
{
	u8 *p = NULL;
	u16 length;

	p = pBuffer;
	length = 0;

	length = Report_GetPositionBasicInfo(p);//��ȡλ�û�����Ϣ
	p = p+length;

	length += Report_GetPositionAdditionalInfo(p);//��ȡλ�ø�����Ϣ

	return length;

}
/**************************************************************************
//��������Report_GetPositionBasicInfo
//���ܣ���ȡλ�û�����Ϣ
//���룺��
//�����λ�û�����Ϣ
//����ֵ��λ�û�����Ϣ����
//��ע�����ȹ̶�Ϊ28�ֽڣ�λ�û�����Ϣ�������֡�״̬�֡�γ�ȡ����ȡ��̡߳��ٶȡ�����ʱ��
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
	
	temp = Io_ReadAlarm();//������
	Public_ConvertLongToBuffer(temp,p);
	p += 4;

	temp = Io_ReadStatus();//״̬��
	Public_ConvertLongToBuffer(temp,p);
	p += 4;

	//��������γ����0   ?????
	//if(!Io_ReadStatusBit(STATUS_BIT_NAVIGATION))
	//{
	//	temp = 0ul;
	//}
	//else
	{
		temp = (Position.Latitue_FX*100UL+Position.Latitue_F*1000000)/60+(Position.Latitue_D*1000000UL);//γ��
	}
	Public_ConvertLongToBuffer(temp,p);
	p += 4;

	//if(!Io_ReadStatusBit(STATUS_BIT_NAVIGATION))
	//{
	//	temp = 0ul;
	//}
	//else
	{
		temp = (Position.Longitue_FX*100UL+Position.Longitue_F*1000000)/60+(Position.Longitue_D*1000000UL);//����
	}
	Public_ConvertLongToBuffer(temp,p);
	p += 4;

	temp2 = Position.High+Position.HighOffset;//�߳�
	if((Position.High+Position.HighOffset)<0)temp2 = 0;//��������0
	Public_ConvertShortToBuffer(temp2,p);
	p += 2;

        temp2 = 10*Gps_ReadSpeed();//�ٶ�
	Public_ConvertShortToBuffer(temp2,p);
	p += 2;

	temp2 = Position.Course;//����
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
	timecount -= 3;//���ڱ�ʡ�˹�ƽ̨ʱ����ˣ������ȥ3��
	Gmtime(&time, timecount);

	//GPS��λ��acc onʱ����GPSʱ��
	if((1==Io_ReadStatusBit(STATUS_BIT_NAVIGATION))
			&&(1==Io_ReadStatusBit(STATUS_BIT_ACC)))
    {
        *p++ = Public_HEX2BCD(time.year);//GPSʱ��
        *p++ = Public_HEX2BCD(time.month);
        *p++ = Public_HEX2BCD(time.day);
        *p++ = Public_HEX2BCD(time.hour);      
        *p++ = Public_HEX2BCD(time.min);
        *p++ = Public_HEX2BCD(time.sec);
	}
	else
	{
		*p++ = Public_HEX2BCD(CurTime.year);//��ȡʱ��
		*p++ = Public_HEX2BCD(CurTime.month);
		*p++ = Public_HEX2BCD(CurTime.day);
		*p++ = Public_HEX2BCD(CurTime.hour);
		*p++ = Public_HEX2BCD(CurTime.min);
		*p++ = Public_HEX2BCD(CurTime.sec);
	}
	return 28;
	
}
/**************************************************************************
//��������Report_ClearReportTimeCount
//���ܣ��ϱ�ʱ�����������0
//���룺��
//�������
//����ֵ����
//��ע���������ӵļ�����������0
***************************************************************************/
void Report_ClearReportTimeCount(void)
{
	Link1ReportTimeCount = 0;
	Link2ReportTimeCount = 0;
}
/**************************************************************************
//��������Report_Link2ReportOpenFlag
//���ܣ�����2�Զ���㱨�Ƿ���Ҫ����
//���룺��
//�����1����Ҫ������0������Ҫ����
//����ֵ����
//��ע��
***************************************************************************/
static u8 Report_Link2ReportOpenFlag(void)
{
	if((TempTrackTimeSpace > 0)&&(TempTrackCount > 0))//��ʱ����
	{
		return 0;
	}
	else if(1 == Io_ReadAlarmBit(ALARM_BIT_EMERGENCY))//��������
	{
		return 0;
	}
	else if((1 == Io_ReadAlarmBit(ALARM_BIT_OVER_SPEED))&&(0 != OverSpeedReportTimeSpace))//���ٱ���
	{
		return 0;
	}
	else if((1 == Io_ReadAlarmBit(ALARM_BIT_TIRED_DRIVE))&&(0 != OverTimeReportTimeSpace))//��ʱ����
	{
		return 0;
	}
	else if((0 != Link2AccOnReportTimeSpace)&&(0 != Link2AccOffReportTimeSpace))//����2����������
	{
		return 1;
	}
	else
	{
		return 0;
	}

	
}
/**************************************************************************
//��������Report_GetLink1NextReportPoint
//���ܣ���ȡ����1����һ��λ�û㱨�㣨�㱨ʱ��ͻ㱨���룩
//���룺��
//�������
//����ֵ����
//��ע��
***************************************************************************/
static void Report_GetLink1NextReportPoint(u32 *NextReportTime, u32 *NextReportDistance)
{

	*NextReportTime = 0;
	*NextReportDistance = 0;

	if((TempTrackTimeSpace > 0)&&(TempTrackCount > 0))//��ʱ����
	{
		*NextReportTime = TempTrackTimeSpace;
		*NextReportDistance = 0;
		Link1ReportDistanceSpace = 0;
	}
	else
	{
		if(1 == Io_ReadAlarmBit(ALARM_BIT_EMERGENCY))//��������
		{
			if(0 == ReportStrategy)//��ʱ
			{
				*NextReportTime = EmergencyReportTimeSpace;
				*NextReportDistance = 0;
				Link1ReportDistanceSpace = 0;
			}
			else if(1 == ReportStrategy)//����
			{
				*NextReportTime = 0;
				*NextReportDistance = EmergencyReportDistanceSpace+Link1LastReportDistance;
				Link1ReportDistanceSpace = EmergencyReportDistanceSpace;
			}
			else if(2 == ReportStrategy)//��ʱ+����
			{
				*NextReportTime = EmergencyReportTimeSpace;
				*NextReportDistance = EmergencyReportDistanceSpace+Link1LastReportDistance;
				Link1ReportDistanceSpace = EmergencyReportDistanceSpace;
			}
		}
		else if((1 == Io_ReadAlarmBit(ALARM_BIT_OVER_SPEED))&&(0 != OverSpeedReportTimeSpace))//���ٱ���
		{
			*NextReportTime = OverSpeedReportTimeSpace;
			*NextReportDistance = 0;
			Link1ReportDistanceSpace =  0;
		}
		else if((1 == Io_ReadAlarmBit(ALARM_BIT_TIRED_DRIVE))&&(0 != OverTimeReportTimeSpace))//��ʱ����
		{
			*NextReportTime = OverTimeReportTimeSpace;
			*NextReportDistance = 0;
			Link1ReportDistanceSpace = 0;
		}
		else 
		{
			if(0 == ReportScheme)//����ACC״̬
			{
				if(1 == Io_ReadStatusBit(STATUS_BIT_ACC))//ACC ON
				{
					if(0 == ReportStrategy)//��ʱ
					{
						*NextReportTime = AccOnReportTimeSpace;
						*NextReportDistance = 0;
						Link1ReportDistanceSpace = 0;
					}
					else if(1 == ReportStrategy)//����
					{
						*NextReportTime = 0;
						*NextReportDistance = AccOnReportDistanceSpace+Link1LastReportDistance;
						Link1ReportDistanceSpace = AccOnReportDistanceSpace;
					}
					else if(2 == ReportStrategy)//��ʱ+����
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
					if(0 == ReportStrategy)//��ʱ
					{
						*NextReportTime = AccOffReportTimeSpace;
						*NextReportDistance = 0;
						Link1ReportDistanceSpace = 0;
					}
					else if(1 == ReportStrategy)//����
					{
						*NextReportTime = 0;
						*NextReportDistance = AccOffReportDistanceSpace+Link1LastReportDistance;
						Link1ReportDistanceSpace = AccOffReportDistanceSpace;
					}
					else if(2 == ReportStrategy)//��ʱ+����
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
			else if(1 == ReportScheme)//����ACC״̬�͵�¼״̬�����жϵ�¼״̬���ٸ���ACC
			{
				if(0 == NaviLcdDriverLoginFlag())//��ʻԱδ��¼
				{
					if(0 == ReportStrategy)//��ʱ
					{
						*NextReportTime = UnloginReportTimeSpace;
						*NextReportDistance = 0;
						Link1ReportDistanceSpace = 0;
					}
					else if(1 == ReportStrategy)//����
					{
						*NextReportTime = 0;
						*NextReportDistance = UnloginReportDistanceSpace+Link1LastReportDistance;
						Link1ReportDistanceSpace = UnloginReportDistanceSpace;
					}
					else if(2 == ReportStrategy)//��ʱ+����
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
				else//��ʻԱ�ѵ�¼
				{
					if(1 == Io_ReadStatusBit(STATUS_BIT_ACC))//ACC ON
					{
						if(0 == ReportStrategy)//��ʱ
						{
							*NextReportTime = AccOnReportTimeSpace;
							*NextReportDistance = 0;
							Link1ReportDistanceSpace = 0;
						}
						else if(1 == ReportStrategy)//����
						{
							*NextReportTime = 0;
							*NextReportDistance = AccOnReportDistanceSpace+Link1LastReportDistance;
							Link1ReportDistanceSpace = AccOnReportDistanceSpace;
						}
						else if(2 == ReportStrategy)//��ʱ+����
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
						if(0 == ReportStrategy)//��ʱ
						{
							*NextReportTime = AccOffReportTimeSpace;
							*NextReportDistance = 0;
							Link1ReportDistanceSpace = 0;
						}
						else if(1 == ReportStrategy)//����
						{
							*NextReportTime = 0;
							*NextReportDistance = AccOffReportDistanceSpace+Link1LastReportDistance;
							Link1ReportDistanceSpace = AccOffReportDistanceSpace;
						}
						else if(2 == ReportStrategy)//��ʱ+����
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
//��������Report_GetCurDistance
//���ܣ���ȡ��ǰ�ľ��루�ۼ���ʻ��̣�
//���룺��
//�������
//����ֵ�����루�ۼ���ʻ��̣�
//��ע����λ��
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
//��������Report_GetPositionAdditionalInfo
//���ܣ���ȡλ�ø�����Ϣ
//���룺��
//�����λ�ø�����Ϣ
//����ֵ��λ�ø�����Ϣ����
//��ע��
***************************************************************************/
static u16 Report_GetPositionAdditionalInfo(u8 *pBuffer)
{
	u8 *p = NULL;
	u16 length;

	p = pBuffer;
	length = 0;

	length = Report_GetPositionStdAdditionalInfo(p);//��ȡ��׼��λ�ø�����Ϣ
	p = p+length;

	length += Report_GetPositionEiAdditionalInfo(p);//��ȡ�����Զ����λ�ø�����Ϣ

	return length;	
}
/**************************************************************************
//��������Report_GetPositionStdAdditionalInfo
//���ܣ���ȡ��׼��λ�ø�����Ϣ
//���룺��
//�������׼��λ�ø�����Ϣ
//����ֵ����׼��λ�ø�����Ϣ����
//��ע��������ϢID:0x01~0x04,0x11~0x13,0x25,0x2a,0x2b,0x30,0x31
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

	*p++ = 0x01;//��̣�IDΪ0x01	
	*p++ = 4;
	temp = Public_GetCurTotalMile();
	Public_ConvertLongToBuffer(temp,p);
	p += 4;
	length += 6;

	TempLen = JointechOiLCost_GetSubjoinInfoCurOilVolume(p);//������IDΪ0x02����ͨ���������߶�ѡһ
	p += TempLen;
	length += TempLen;
	TempLen = OilWear_GetSubjoinInfoCurOilVal(p);//������IDΪ0x02����ʵ������
	p += TempLen;
	length += TempLen;

	*p++ = 0x03;//��ʻ��¼���ٶȣ�IDΪ0x03���˴����׼Ҫ��һ�������˴���
	*p++ = 0x02;//�������ݶ����������������ٶȲ�Ϊ0ʱʹ�������ٶȣ������ٶ�Ϊ0ʱʹ��GPS�ٶȣ�����֮ǰ������
  
        if(0 == (SpeedFlag&0x01))//�������ݶ�����������ѡ��Ϊ�����ٶ�����ʱ���ϴ������ٶȣ���ѡ��ΪGPS�ٶ�����ʱ�����岻Ϊ0ʱ�ϴ����壬Ϊ0ʱ�ϴ�GPS�ٶȣ��������ڵ�����
	{
		temp2 = 10*Pulse_GetSpeed();
	}
	else//GPS�ٶ� 
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

	//��Ҫ�˹�ȷ�ϱ���ʱ���ID��IDΪ0x04����������
	
	TempLen = SpeedMonitor_GetCurOverSpeedSubjoinInfo(p);//���ٱ���������Ϣ��IDΪ0x11
	p += TempLen;
	length += TempLen;

	TempLen = Area_GetInOutAreaAlarmSubjoinInfo(p);//�������򱨾�������Ϣ��IDΪ0x12
	p += TempLen;
	length += TempLen;

	TempLen = Route_GetDriveAlarmInfoSubjoinInfo(p);//����·�߱���������Ϣ��IDΪ0x13
	p += TempLen;
	length += TempLen;

	
	*p++ = 0x25;//��չ�����ź�״̬��IDΪ0x25	
	*p++ = 4;
	temp = Io_ReadExtCarStatus();
	Public_ConvertLongToBuffer(temp,p);
	p += 4;
	length += 6;

	*p++ = 0x2a;//IO״̬��IDΪ0x2a	
	*p++ = 2;
	temp2 = Io_ReadIoStatus();
	Public_ConvertShortToBuffer(temp2,p);
	p += 2;
	length += 4;

	temp2 = Ad_GetValue(ADC_EXTERN1);
	temp3 = Ad_GetValue(ADC_EXTERN2);
	*p++ = 0x2b;//ģ������IDΪ0x2b����16λΪģ����2����16λΪģ����1
	*p++ = 4;
	Public_ConvertShortToBuffer(temp3,p);
	p += 2;
	Public_ConvertShortToBuffer(temp2,p);
	p += 2;
	length += 6;

	*p++ = 0x30;//���������ź�ǿ�ȣ�IDΪ0x30	
	*p++ = 1;
	*p++ = communicatio_GetSignalIntensity();
	length += 3;

	*p++ = 0x31;//��λ������IDΪ0x31	
	*p++ = 1;
	*p++ = Gps_ReadStaNum();
	length += 3;
	
	return length;
	
}
/**************************************************************************
//��������Report_GetPositionEiAdditionalInfo
//���ܣ���ȡ�����Զ����λ�ø�����Ϣ
//���룺��
//����������Զ����λ�ø�����Ϣ
//����ֵ�������Զ����λ�ø�����Ϣ����
//��ע��
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

	p += 2;//�տ���������ID����־��1�ֽڣ�����1�ֽ�

	*p++ = 0xE1;//�Զ���״̬��2��IDΪ0xE1
	*p++ = 4;
	temp = Io_ReadSelfDefine2();
	Public_ConvertLongToBuffer(temp,p); 
	p += 4;
	length += 6;

	TempLen = CarLoad_GetCurWeightSubjoinInfo(p);//����������IDΪ0xE2
	p += TempLen;
	length += TempLen;

	TempLen = JointechOiLCost_GetSubjoinInfoCurOilLevelPercent(p);//��λ�߶���ֱȣ�IDΪ0xE4
	p += TempLen;
	length += TempLen;

	TempLen = OilWear_GetSubjoinInfoCurOilResVal(p);//��ʵ����������ֵ��IDΪ0xE5
	p += TempLen;
	length += TempLen;

	TempLen = Temperature_PosEx(p);//�¶Ȳɼ�����IDΪ0xE8
	p += TempLen;
	length += TempLen;

        //zengliang add
	TempLen = Acceleration_PosEx(p);//�����ٸ���ID��IDΪ0xE9
	p += TempLen;
	length += TempLen;
        // add end 

	VerifyCode = Public_GetSumVerify(pBuffer+2,length);//У���
	*p++ = VerifyCode;
	length++;

	*pBuffer = 0xF0;//������չ������Ϣ��־
	*(pBuffer+1) = length;

	return length+2;
	
}

































/************************************************************************
//�������ƣ�Blind.c
//���ܣ���ģ��ʵ��ä���������ܡ�
//�汾�ţ�V0.1
//��Ȩ˵������Ȩ�����������������¼����������޹�˾
//�����ˣ�dxl
//����ʱ�䣺2014.10
//�汾��¼���汾��¼������汾���޸��ˡ��޸�ʱ�䡢�޸�ԭ���޸ļ�Ҫ˵��
//V0.1����Ҫʵ��˫���ӵ�ä������������1������2��ä���洢�����Ƕ����ġ��ֿ��ġ�
//�����ֻʹ��������1������2δ����������2 TCP�˿ڲ�������Ϊ0ʱ��ʾ����2����������
//�����洢����϶�Ϊһ��ÿ���ն����ߺ�����¼��һ�ο��Ƿ���Ҫ����������δ������
//���ݣ���ʼ������ÿ�ΰ�5��λ����Ϣ���������ֻ���յ�ƽ̨Ӧ���ŻᲹ����һ����
//����һֱ������ǰ������෢��10�Σ�ÿ��20�롣��10�κ���Ȼû��Ӧ��
//���ȴ�30���Ӻ����ط���ǰ����
*************************************************************************/

/********************�ļ�����*************************/
#include <stdio.h>
#include <string.h>

#include "Blind.h"
#include "modem_lib.h"
#include "GPIOControl.h"
#include "other.h"
#include "modem_app_com.h"
#include "spi_flashapi.h"
#include "rtc.h"
#include "Public.h"
#include "spi_flash.h"
#include "RadioProtocol.h"
#include "EepromPram.h"
#include "Gnss_app.h"

/********************���ر���*************************/
static BLIND_STRUCT Link1Blind;
static s16 Link1SearchSector;
static u16 Link1TimeCount = 0;
static u16 Link1WaitTimeCount = 0;

static BLIND_STRUCT Link2Blind;
static s16 Link2SearchSector;
static u16 Link2TimeCount = 0;
static u16 Link2WaitTimeCount = 0;
/********************ȫ�ֱ���*************************/
static u16 Link2OpenFlag = 0;//����2�Ƿ�����־��1Ϊ������0Ϊδ����

/********************�ⲿ����*************************/
extern u8 RadioShareBuffer[];
extern TIME_T CurTime;

/********************���غ�������*********************/
static void Blind_Link1Init(void);//����1������ʼ��
static void Blind_Link1Search(void);//����1����ȷ��ä����¼���ϱ��ĳ�ʼλ��
static void Blind_Link1SearchSector(s16 SearchSector);//���ĳ�������е�ä����Ϣ
static u8 Blind_Link1Report(void);//����1�ϱ�һ��ä������
static void Blind_Link1ReportAck(void);//����1ä������Ӧ����
static u8 Blind_GetLink1ReportFlag(void);//��ȡ����1ä���ϱ���־
static void Blind_StartLink1Report(void);//��������1ä������
static void Blind_Link1Erase(void);//��������1ä���洢����
static u8 Blind_Link1Save(u8 *pBuffer, u8 length, u8 attribute);//�洢һ������1��ä������
static void Blind_StartLink1Report(void);

static void Blind_Link2Init(void);//����2������ʼ��
static void Blind_Link2Search(void);//����2����ȷ��ä����¼���ϱ��ĳ�ʼλ��
static void Blind_Link2SearchSector(s16 SearchSector);//���ĳ�������е�ä����Ϣ
static u8 Blind_Link2Report(void);//����2�ϱ�һ��ä������
static void Blind_Link2ReportAck(void);//����2ä������Ӧ����
static u8 Blind_GetLink2ReportFlag(void);//��ȡ����2ä���ϱ���־
static void Blind_StartLink2Report(void);//��������2ä������
static void Blind_Link2Erase(void);//��������2ä���洢����
static u8 Blind_Link2Save(u8 *pBuffer, u8 length, u8 attribute);//�洢һ������2��ä������
static void Blind_StartLink2Report(void);

static void Blind_UpdateLink2OpenFlag(void);//���±���Link2OpenFlag
/********************��������*************************/
/**************************************************************************
//��������Blind_TimeTask
//���ܣ�ʵ��ä����������
//���룺��
//�������
//����ֵ����
//��ע��ä��������ʱ����50ms����1�Σ������������Ҫ���ô˺�����
//���ڽ���FTP����ʱ��Ҫ�رո�������Ϊä����¼��̼�Զ����������һ��洢��
//ע�����˭�ر��˸�����˭��Ҫ����򿪸������ϵ�����Ĭ���Ǵ򿪵�
***************************************************************************/
FunctionalState Blind_TimeTask(void)
{

	/**************����1ä������****************/
	if(BLIND_STATE_INIT == Link1Blind.State)//��ʼ��������ر���
	{
		Blind_Link1Init();//��ɺ��Զ�ת��BLIND_STATE_SEARCH
		
	}
	else if(BLIND_STATE_SEARCH == Link1Blind.State)//�𲽲���ȷ����ǰ��ä������λ��
	{
		Blind_Link1Search();//��ɺ��Զ�ת��BLIND_STATE_REPORT
	}
	else if(BLIND_STATE_REPORT == Link1Blind.State)//ä���ϱ�
	{
		Link1WaitTimeCount = 0;	
		//if(0x01 == (0x01&GetTerminalAuthorizationFlag()))//ֻ������1����ʱ�Ų���
		//{
			Link1TimeCount++;
			if(Link1TimeCount >= BLIND_REPORT_DELAY_TIME)//20��
			{
				Link1TimeCount = 0;	
				Link1Blind.ReportSendNum++;
				if((Link1Blind.ReportSendNum > 0)&&(Link1Blind.ReportSendNum <= 10))
				{
					if(0 != Link1Blind.LastReportSector[0])
					{
						Link1Blind.ReportSector = Link1Blind.LastReportSector[0];
						Link1Blind.ReportStep = Link1Blind.LastReportStep[0];
					}
					if(0 == Blind_Link1Report())//ä���ϱ�
					{
						Gnss_StartBlindReport();
						Link1Blind.State = BLIND_STATE_WAIT;//ת��ȴ�״̬
					}
				}
				else if(Link1Blind.ReportSendNum > 10)
				{
					Link1Blind.State = BLIND_STATE_WAIT;//ת��ȴ�״̬
				}
			}
		//}
	}
	else if(BLIND_STATE_WAIT == Link1Blind.State)//�ȴ�
	{
		Link1WaitTimeCount++;
		if(Link1WaitTimeCount >= 36000)//30����
		{
			Link1WaitTimeCount = 0;
			Link1Blind.State = BLIND_STATE_INIT;
		}
	}
	else//�쳣
	{
		Link1Blind.State = BLIND_STATE_INIT;
	}


	/**************����2ä������****************/
	if(0 == Link2OpenFlag)
	{
		return ENABLE;
	}

	if(BLIND_STATE_INIT == Link2Blind.State)//��ʼ��������ر���
	{
		Blind_Link2Init();//��ɺ��Զ�ת��BLIND_STATE_SEARCH
		
	}
	else if(BLIND_STATE_SEARCH == Link2Blind.State)//�𲽲���ȷ����ǰ��ä������λ��
	{
		Blind_Link2Search();//��ɺ��Զ�ת��BLIND_STATE_REPORT
	}
	else if(BLIND_STATE_REPORT == Link2Blind.State)//ä���ϱ�
	{
		Link2WaitTimeCount = 0;	
		//if(0x01 == (0x01&GetTerminalAuthorizationFlag()))//ֻ������1����ʱ�Ų���
		//{
			Link2TimeCount++;
			if(Link2TimeCount >= BLIND_REPORT_DELAY_TIME)//20��
			{
				Link2TimeCount = 0;	
				Link2Blind.ReportSendNum++;
				if((Link2Blind.ReportSendNum > 0)&&(Link2Blind.ReportSendNum <= 10))
				{
					if(0 != Link2Blind.LastReportSector[0])
					{
						Link2Blind.ReportSector = Link2Blind.LastReportSector[0];
						Link2Blind.ReportStep = Link2Blind.LastReportStep[0];
					}
					if(0 == Blind_Link2Report())//ä���ϱ�
					{
						Link2Blind.State = BLIND_STATE_WAIT;//ת��ȴ�״̬
					}
				}
				else if(Link2Blind.ReportSendNum > 10)
				{
					Link2Blind.State = BLIND_STATE_WAIT;//ת��ȴ�״̬
				}
			}
		//}
	}
	else if(BLIND_STATE_WAIT == Link2Blind.State)//�ȴ�
	{
		Link2WaitTimeCount++;
		if(Link2WaitTimeCount >= 36000)//30����
		{
			Link2WaitTimeCount = 0;
			Link2Blind.State = BLIND_STATE_INIT;
		}
	}
	else//�쳣
	{
		Link2Blind.State = BLIND_STATE_INIT;
	}

	

	return ENABLE;
}
/**************************************************************************
//��������Blind_Save
//���ܣ�����ĳͨ����һ��ä�����ݣ�λ�û㱨���ݣ�
//���룺ͨ���ţ�ä�����ݣ����ݳ���
//�������
//����ֵ��0��ʾ�ɹ���1��ʾʧ�ܣ���������ȷ�᷵��ʧ��
//��ע��ͨ����ֻ��ΪCHANNEL_SMS,CHANNEL_DATA_1,CHANNEL_DATA_2����֮һ
***************************************************************************/
u8 Blind_Save(u8 channel, u8 *pBuffer, u8 length)
{
	u8 flag = 0;
	u8 attribute;

	attribute = channel;

	if(CHANNEL_DATA_1 == channel)
	{
		flag = Blind_Link1Save(pBuffer, length, attribute);
		return flag;
	}
	else if(CHANNEL_DATA_2 == channel)
	{
		flag = Blind_Link2Save(pBuffer, length, attribute);
		return flag;
	}
	else
	{
		return 1;
	}
}
/**************************************************************************
//��������Blind_Erase
//���ܣ�����ĳͨ�������д洢����
//���룺ͨ���ţ�ֻ��ΪCHANNEL_SMS,CHANNEL_DATA_1,CHANNEL_DATA_2����֮һ
//�������
//����ֵ��0��ʾ�ɹ���1��ʾʧ�ܣ���������ȷ�᷵��ʧ��
//��ע��Ҫ�������е�ä�����ݣ�������Ϊ0xff��������Զ�������̼�֮ǰ���Ȳ���
//����ä��������Ϊä���洢������̼��洢������ͬһ��flash�ռ�
***************************************************************************/
u8 Blind_Erase(u8 channel)
{

	Link1Blind.State = BLIND_STATE_INIT;
	Link2Blind.State = BLIND_STATE_INIT;

	if(CHANNEL_DATA_1 == channel)
	{
		Blind_Link1Erase();
		return 0;
	}
	else if(CHANNEL_DATA_2 == channel)
	{
		Blind_Link2Erase();
		return 0;
	}
	else if(0xff == channel)
	{
		Blind_Link1Erase();
		Blind_Link2Erase();
		return 0;
	}
	else
	{
		return 1;
	}

	
	
}
/**************************************************************************
//��������Blind_StartReport
//���ܣ���ʼĳ��ͨ����ä������
//���룺ͨ����
//�������
//����ֵ����
//��ע��ͨ����ֻ��ΪCHANNEL_SMS,CHANNEL_DATA_1,CHANNEL_DATA_2����֮һ
***************************************************************************/
void Blind_StartReport(u8 channel)
{
	if(CHANNEL_DATA_1 == channel)
	{
		Blind_StartLink1Report();
	}
	else if(CHANNEL_DATA_2 == channel)
	{
		Blind_StartLink2Report();
	}
}
/**************************************************************************
//��������Blind_ReportAck
//���ܣ�ä������Ӧ����
//���룺ͨ���ţ�ֻ��ΪCHANNEL_SMS,CHANNEL_DATA_1,CHANNEL_DATA_2����֮һ
//�������
//����ֵ��0��ʾ�ɹ���1��ʾʧ�ܣ���������ȷ�᷵��ʧ��
//��ע���յ�ä������Ӧ�������ô˺���
***************************************************************************/
u8 Blind_ReportAck(u8 channel)
{
	if(CHANNEL_DATA_1 == channel)
	{
		Blind_Link1ReportAck();
		return 0;
	}
	else if(CHANNEL_DATA_2 == channel)
	{
		Blind_Link2ReportAck();
		return 0;
	}
	else
	{
		return 1;
	}	
}
/**************************************************************************
//��������Blind_GetLink2OpenFlag
//���ܣ���ȡ��2�����ӿ�����־
//���룺��
//�������
//����ֵ����2�����ӿ�����־
//��ע��Link2OpenFlagΪ1��ʾ�����˵�2�����ӣ�Ϊ0��ʾû�п���
***************************************************************************/
u8 Blind_GetLink2OpenFlag(void)
{
	return Link2OpenFlag;
}
/**************************************************************************
//��������Blind_Link1Init
//���ܣ�����1������ʼ��
//���룺��
//�������
//����ֵ����
//��ע��ä��������ʱ������Ҫ���ô˺���
***************************************************************************/
static void Blind_Link1Init(void)
{
	u8 i;

	//����2�����ӵ�TCP�˿ںţ��ж��Ƿ�����2�����ӣ�����������Ҫ��ִ洢����ÿ�θò���״̬�仯ʱ��Ҫ����flash
	//Ϊ�˼򵥵㣬�Ȳ����л��������洢�����ȷֳ����飬ǰһ����������1����һ����������2
	Blind_UpdateLink2OpenFlag();
	if(1 == Link2OpenFlag)//����������2
	{
		Link1Blind.StartSector = FLASH_BLIND_START_SECTOR;
		Link1Blind.EndSector = FLASH_BLIND_MIDDLE_SECTOR;
	}
	else
	{
		Link1Blind.StartSector = FLASH_BLIND_START_SECTOR;
		Link1Blind.EndSector = FLASH_BLIND_END_SECTOR;
	}
	Link1Blind.SectorStep = FLASH_ONE_SECTOR_BYTES/FLASH_BLIND_STEP_LEN;

	Link1Blind.OldestSector = Link1Blind.StartSector;
	Link1Blind.OldestStep = 0;
	Link1Blind.OldestTimeCount = 0;//������Сֵ
	Link1Blind.SaveSector = Link1Blind.StartSector;
	Link1Blind.SaveStep = 0;
	Link1Blind.SaveTimeCount = 0;//������Сֵ
	Link1Blind.SaveEnableFlag = 0;
	Link1Blind.ReportSendNum = 0;
	for(i=0; i<5; i++)
	{
		Link1Blind.LastReportSector[i] = 0;
		Link1Blind.LastReportStep[i] = 0;
	}
	Link1Blind.ErrorCount = 0;
	Link1Blind.State = BLIND_STATE_SEARCH;

	Link1TimeCount = 0;
	Link1WaitTimeCount = 0;	
	Link1SearchSector = Link1Blind.StartSector;

	
}
/**************************************************************************
//��������Blind_Link1Search
//���ܣ�����1����ȷ��ä����¼���ϱ��ĳ�ʼλ��
//���룺��
//�������
//����ֵ����
//��ע��ä��������ʱ������Ҫ���ô˺���
***************************************************************************/
static void Blind_Link1Search(void)
{
	if((Link1SearchSector >= Link1Blind.StartSector)&&(Link1SearchSector < Link1Blind.EndSector))
	{
		Blind_Link1SearchSector(Link1SearchSector);//��������1ĳ�������е�ÿ��ä����Ϣ
	}
	else
	{
		Link1Blind.State = BLIND_STATE_INIT;
		return ;
	}
	
	if(Link1Blind.ErrorCount > 3)
	{
		Link1Blind.ErrorCount = 0;
		Blind_Link1Erase();
		return ;
	}

	Link1SearchSector++;
	if(Link1SearchSector >= Link1Blind.EndSector)
	{
		if((Link1Blind.StartSector == Link1Blind.SaveSector)&&(0 == Link1Blind.SaveStep)&&(0 == Link1Blind.SaveTimeCount))
		{
			//ä������Ϊ��
		}
		else
		{
			Link1Blind.SaveStep++;
			if(Link1Blind.SaveStep >= Link1Blind.SectorStep)
			{
				Link1Blind.SaveStep = 0;
				Link1Blind.SaveSector++;
				if(Link1Blind.SaveSector >= Link1Blind.EndSector)
				{
					Link1Blind.SaveSector = Link1Blind.StartSector;
				}
			}
		}
		Link1Blind.ReportSector = Link1Blind.OldestSector;
		Link1Blind.ReportStep = Link1Blind.OldestStep;
		Link1Blind.SaveEnableFlag = 1;
		Link1Blind.State = BLIND_STATE_REPORT;
	}
}
/**************************************************************************
//��������Blind_Link1SearchSector
//���ܣ����ĳ�������е�ä����Ϣ
//���룺������
//�������ǰ�洢�Ͳ���ä����λ��
//����ֵ����
//��ע��
***************************************************************************/
static void Blind_Link1SearchSector(s16 SearchSector)
{
	s16 i,j;
	u32 Addr;
	u8 flag;
	u8 Buffer[FLASH_BLIND_STEP_LEN+1];
	u8 sum;
	TIME_T tt;
	u32 ttCount;

	Addr = SearchSector*FLASH_ONE_SECTOR_BYTES + Link1Blind.SectorStep*FLASH_BLIND_STEP_LEN + 1;
	sFLASH_ReadBuffer(&flag,Addr,1);//�����������Ƿ��Ѳ�����־
	if(0xaa == flag)
	{
		Addr = SearchSector*FLASH_ONE_SECTOR_BYTES + (Link1Blind.SectorStep-1)*FLASH_BLIND_STEP_LEN;
		sFLASH_ReadBuffer(Buffer,Addr,4);
		ttCount = Public_ConvertBufferToLong(Buffer);
		Gmtime(&tt,ttCount);
		if(SUCCESS == CheckTimeStruct(&tt))
		{
			if(ttCount > Link1Blind.OldestTimeCount)
			{
				Link1Blind.OldestSector = SearchSector;
				Link1Blind.OldestStep = Link1Blind.SectorStep-1;
				Link1Blind.OldestTimeCount = ttCount;
			}
		}
		return ;
	}

	Addr = SearchSector*FLASH_ONE_SECTOR_BYTES + (Link1Blind.SectorStep-1)*FLASH_BLIND_STEP_LEN;//�����������1��
	sFLASH_ReadBuffer(Buffer,Addr,4);
	ttCount = Public_ConvertBufferToLong(Buffer);
	if(0xffffffff != ttCount)
	{
		Gmtime(&tt,ttCount);
		if(SUCCESS == CheckTimeStruct(&tt))
		{
			sFLASH_ReadBuffer(Buffer,Addr,FLASH_BLIND_STEP_LEN);
			if(CHANNEL_DATA_1 != Buffer[4])//ͨ������ȷ
			{
				Link1Blind.ErrorCount++;
				if(Link1Blind.ErrorCount > 3)
				{
					Link1Blind.ErrorCount = 0;
					Blind_Erase(0xff);//��������ä���洢����
					return ;
				}	
			}
			sum = 0;
			for(j=0; j<Buffer[6]; j++)
			{
				sum += Buffer[j+7];
			}
			if((sum != Buffer[5])||(0 == Buffer[6]))
			{
				Link1Blind.ErrorCount++;
				if(Link1Blind.ErrorCount > 3)
				{
					Link1Blind.ErrorCount = 0;
					Blind_Erase(0xff);//��������ä���洢����
					return ;
				}
			}
			else
			{
				if((ttCount > Link1Blind.OldestTimeCount)&&(0xaa == Buffer[FLASH_BLIND_STEP_LEN-1]))
				{
					Link1Blind.OldestSector = SearchSector;
					Link1Blind.OldestStep = 0;
					Link1Blind.OldestTimeCount = ttCount;
				}

				if(ttCount > Link1Blind.SaveTimeCount)
				{
					Link1Blind.SaveSector = SearchSector;
					Link1Blind.SaveStep = Link1Blind.SectorStep-1;
					Link1Blind.SaveTimeCount = ttCount;
				}
			}
		}
		else
		{
			Link1Blind.ErrorCount++;
			if(Link1Blind.ErrorCount > 3)
			{
				Link1Blind.ErrorCount = 0;
				Blind_Erase(0xff);//��������ä���洢����
				return ;
			}
		}
	}
	else
	{
		Addr = SearchSector*FLASH_ONE_SECTOR_BYTES;//�������ĵ�1��
		sFLASH_ReadBuffer(Buffer,Addr,4);
		ttCount = Public_ConvertBufferToLong(Buffer);
		if(0xffffffff != ttCount)
		{
			for(i=0; i<Link1Blind.SectorStep; i++)
			{
				Addr = SearchSector*FLASH_ONE_SECTOR_BYTES+i*FLASH_BLIND_STEP_LEN;
				sFLASH_ReadBuffer(Buffer,Addr,FLASH_BLIND_STEP_LEN);
				ttCount = Public_ConvertBufferToLong(Buffer);
				if(0xffffffff == ttCount)
				{
					break;
				}
				Gmtime(&tt,ttCount);
				if(SUCCESS == CheckTimeStruct(&tt))
				{
					if(CHANNEL_DATA_1 != Buffer[4])//ͨ������ȷ
					{
						Link1Blind.ErrorCount++;
						if(Link1Blind.ErrorCount > 3)
						{
							Link1Blind.ErrorCount = 0;
							Blind_Erase(0xff);//��������ä���洢����
							return ;
						}	
					}
					sum = 0;
					for(j=0; j<Buffer[6]; j++)
					{
						sum += Buffer[j+7];
					}
					if((sum != Buffer[5])||(0 == Buffer[6]))
					{
						Link1Blind.ErrorCount++;
						if(Link1Blind.ErrorCount > 3)
						{
							Link1Blind.ErrorCount = 0;
							Blind_Erase(0xff);//��������ä���洢����
							return ;
						}
					}
					else
					{
						if((ttCount > Link1Blind.OldestTimeCount)&&(0xaa == Buffer[FLASH_BLIND_STEP_LEN-1]))
						{
							Link1Blind.OldestSector = SearchSector;
							Link1Blind.OldestStep = i;
							Link1Blind.OldestTimeCount = ttCount;
						}

						if(ttCount > Link1Blind.SaveTimeCount)
						{
							Link1Blind.SaveSector = SearchSector;
							Link1Blind.SaveStep = i;
							Link1Blind.SaveTimeCount = ttCount;
						}
					}
				}
				else
				{
					Link1Blind.ErrorCount++;
					if(Link1Blind.ErrorCount > 3)
					{
						Link1Blind.ErrorCount = 0;
						Blind_Erase(0xff);//��������ä���洢����
						return ;
					}
				}
			}
		}
	}

/*ÿ������ÿһ������飬̫�˷�ʱ�䣬�Ż����ֻ����1�������1��
	Link1Blind.ErrorCount = 0;
	for(i=0; i<Link1Blind.SectorStep; i++)
	{
		Addr = SearchSector*FLASH_ONE_SECTOR_BYTES + i*FLASH_BLIND_STEP_LEN;
		sFLASH_ReadBuffer(Buffer,Addr,4);
		ttCount = Public_ConvertBufferToLong(Buffer);
		if(0xffffffff != ttCount)
		{
			Gmtime(&tt,ttCount);
			if(SUCCESS == CheckTimeStruct(&tt))
			{
				sFLASH_ReadBuffer(Buffer,Addr,FLASH_BLIND_STEP_LEN);
				if(CHANNEL_DATA_1 != Buffer[4])//ͨ������ȷ
				{
					Link1Blind.ErrorCount++;
					if(Link1Blind.ErrorCount > 3)
					{
						Link1Blind.ErrorCount = 0;
						Blind_Erase(0xff);//��������ä���洢����
						break;
					}	
				}
				sum = 0;
				for(j=0; j<Buffer[6]; j++)
				{
					sum += Buffer[j+7];
				}
				if((sum != Buffer[5])||(0 == Buffer[6]))
				{
					Link1Blind.ErrorCount++;
				}
				else
				{
					if((ttCount > Link1Blind.OldestTimeCount)&&(0xaa == Buffer[FLASH_BLIND_STEP_LEN-1]))
					{
						Link1Blind.OldestSector = SearchSector;
						Link1Blind.OldestStep = i;
						Link1Blind.OldestTimeCount = ttCount;
					}

					if(ttCount > Link1Blind.SaveTimeCount)
					{
						Link1Blind.SaveSector = SearchSector;
						Link1Blind.SaveStep = i;
						Link1Blind.SaveTimeCount = ttCount;
					}
				}
			}
			else
			{
				Link1Blind.ErrorCount++;
			}
		}
	}
*/
	
}
/**************************************************************************
//��������Blind_Link1Report
//���ܣ�����1�ϱ�һ��ä������
//���룺��
//�������
//����ֵ����
//��ע��ä��������ʱ������Ҫ���ô˺���
***************************************************************************/
static u8 Blind_Link1Report(void)
{
	u32 Addr;
	u8 i;
	u8 count = 0;
	u8 flag;
	u8 Buffer[FLASH_BLIND_STEP_LEN+1];
	u8 BufferLen;
	u8 sum;
	u16 length = 3;//Ԥ��3���ֽ�


	for(count=0; count<5; )//ÿ������5��λ����Ϣ
	{
		if(0 == Blind_GetLink1ReportFlag())
		{
			break;
		}
		Addr = Link1Blind.ReportSector*FLASH_ONE_SECTOR_BYTES + Link1Blind.SectorStep*FLASH_BLIND_STEP_LEN + 1;
		sFLASH_ReadBuffer(&flag,Addr,1);//�����������Ƿ��Ѳ�����־
		if(0xaa == flag)//�Ѳ�����
		{
			Link1Blind.ReportSector++;
			Link1Blind.ReportStep = 0;
			if(Link1Blind.ReportSector >= Link1Blind.EndSector)
			{
				Link1Blind.ReportSector = Link1Blind.StartSector;
			}
		}
		else
		{
			for(;count<5;)
			{
				if(0 == Blind_GetLink1ReportFlag())
				{
					break;
				}
				Addr = Link1Blind.ReportSector*FLASH_ONE_SECTOR_BYTES + (Link1Blind.ReportStep+1)*FLASH_BLIND_STEP_LEN - 1;
				sFLASH_ReadBuffer(&flag,Addr,1);//�������Ƿ��Ѳ�����־
				if(0xaa == flag)//�Ѳ�����
				{
					Link1Blind.ReportStep++;
				}
				else
				{
					Addr = Link1Blind.ReportSector*FLASH_ONE_SECTOR_BYTES + Link1Blind.ReportStep*FLASH_BLIND_STEP_LEN;
					sFLASH_ReadBuffer(Buffer,Addr,FLASH_BLIND_STEP_LEN);
					BufferLen = Buffer[6];
					sum = 0;
					for(i=0; i<BufferLen; i++)
					{
						sum += Buffer[7+i];
					}
					if(Buffer[5] == sum)
					{
						RadioShareBuffer[length] = 0;//���ȸ��ֽ��0
						memcpy(RadioShareBuffer+length+1,Buffer+6,BufferLen+1);
						length += (BufferLen+2);
						Link1Blind.LastReportSector[count] = Link1Blind.ReportSector;
						Link1Blind.LastReportStep[count] = Link1Blind.ReportStep;
						count++;
					}
					Link1Blind.ReportStep++;
				}
				if(Link1Blind.ReportStep >= Link1Blind.SectorStep)
				{
					Link1Blind.ReportSector++;
					Link1Blind.ReportStep = 0;
					if(Link1Blind.ReportSector >= Link1Blind.EndSector)
					{
						Link1Blind.ReportSector = Link1Blind.StartSector;
					}
					break;
				}
			}
		}
	}
	
	if(0 == count)
	{
		return 0;
	}
	else
	{
		RadioShareBuffer[0] = 0;//�����ֽڱ�ʾ����
		RadioShareBuffer[1] = count;
		RadioShareBuffer[2] = 1;//0:������λ�������㱨;1:ä������
		RadioProtocol_PostionInformation_BulkUpTrans(CHANNEL_DATA_1,RadioShareBuffer,length);
	}

	return 1;
}
/**************************************************************************
//��������Blind_Link1ReportAck
//���ܣ�����1ä������Ӧ����
//���룺��
//�������
//����ֵ����
//��ע���յ�����1��ä������Ӧ��ʱ����ô˺���
***************************************************************************/
static void Blind_Link1ReportAck(void)
{
	u8 i;
	u8 flag;
	u32 Addr;
	s16 Sector;

	Link1Blind.ReportSendNum = 0;
	Link1TimeCount = BLIND_REPORT_DELAY_TIME-20;//1����ϱ���һ��

	Sector = Link1Blind.LastReportSector[0];

	for(i=0; i<5; i++)
	{
		if((Link1Blind.LastReportSector[i] >= Link1Blind.StartSector)
			&&(Link1Blind.LastReportSector[i] < Link1Blind.EndSector)
				&&(Link1Blind.LastReportStep[i] < Link1Blind.SectorStep))
		{
			Addr = Link1Blind.LastReportSector[i]*FLASH_ONE_SECTOR_BYTES+(Link1Blind.LastReportStep[i]+1)*FLASH_BLIND_STEP_LEN - 1;
			flag = 0xaa;
			sFLASH_WriteBuffer(&flag, Addr, 1);//��Ǹ���ä�����ϱ�

			if(Link1Blind.LastReportStep[i] >= (Link1Blind.SectorStep-1))
			{
				Addr = Sector*FLASH_ONE_SECTOR_BYTES+Link1Blind.SectorStep*FLASH_BLIND_STEP_LEN + 1;
				flag = 0xaa;
				sFLASH_WriteBuffer(&flag, Addr, 1);//��Ǹ�������ä�����ϱ�
			}
		}
		
	}

	for(i=0; i<5; i++)
	{
		Link1Blind.LastReportSector[i] = 0;
		Link1Blind.LastReportStep[i] = 0;
	}
	
		
}
/**************************************************************************
//��������Blind_GetLink1ReportFlag
//���ܣ���ȡ����1ä���ϱ���־
//���룺��
//�������
//����ֵ��ä���ϱ���־
//��ע��1��ʾ��Ҫ�ϱ���0��ʾ����Ҫ�ϱ����ն�ÿ��10���ӻ��Զ����һ�ο��Ƿ���Ҫ�ϱ�
***************************************************************************/
static u8 Blind_GetLink1ReportFlag(void)
{
	s16 Sector;
	s16 Step;

	Sector = Link1Blind.SaveSector;
	Step = 	Link1Blind.SaveStep;
/*
	if(0 != Link1Blind.SaveTimeCount)
	{
		Step--;
		if(Step < 0)
		{
			Step = Link1Blind.SectorStep-1;
			Sector--;
			if(Sector < Link1Blind.StartSector)
			{
				Sector = Link1Blind.EndSector - 1;
			}
		}
	}
	*/
	if((Link1Blind.ReportSector == Sector)&&(Link1Blind.ReportStep >= Step))
	{
		return 0;
	}
	else
	{
		return 1;
	}
}
/**************************************************************************
//��������Blind_StartLink1Report
//���ܣ���������1ä������
//���룺��
//�������
//����ֵ����
//��ע������ä��������ÿ������1��Ȩ�ɹ�����ô˺���
***************************************************************************/
static void Blind_StartLink1Report(void)
{
	//if(BLIND_STATE_WAIT == Link1Blind.State)
	//{
		Link1Blind.State = BLIND_STATE_INIT;
	//}
}
/**************************************************************************
//��������Blind_Link1Erase
//���ܣ���������1ä���洢����
//���룺��
//�������
//����ֵ����
//��ע����⵽ä����¼��λ����Ϣ����ʱ���������������50������ô˺�����
//������Զ��FTP������������ʱ����ô˺�������Ϊä����Զ�̹̼���������һ������
***************************************************************************/
static void Blind_Link1Erase(void)
{
	u16 i;
	u32 Addr;
	
	Link1Blind.SaveEnableFlag = 0;//��ֹ�洢
	Link1Blind.State = BLIND_STATE_INIT;

	for(i=Link1Blind.StartSector; i<Link1Blind.EndSector; i++)
	{
		Addr = i*FLASH_ONE_SECTOR_BYTES;
		sFLASH_EraseSector(Addr);
		IWDG_ReloadCounter();
	}	
}
/**************************************************************************
//��������Blind_Link1Save
//���ܣ��洢һ������1��ä������
//���룺һ��λ����Ϣ������
//�������
//����ֵ��0Ϊ�ɹ�����0Ϊʧ�ܣ����ȳ���ʱ�᷵��ʧ��
//��ע��һ��ä�����ݰ�����ʱ��4�ֽ�+����1�ֽڣ�Ԥ����+У���1�ֽ�+λ����Ϣ����1�ֽ�+λ����Ϣ�����ֽ�
***************************************************************************/
static u8 Blind_Link1Save(u8 *pBuffer, u8 length, u8 attribute)
{
	u8 Buffer[FLASH_BLIND_STEP_LEN+1] = {0};
	u8 i,j;
	u32 Addr;
	u32 TimeCount;
	u8 *p = NULL;
	u8 sum;
	s16 Sector;
	s16 Step;
	TIME_T tt;
	u32 ttCount;

	if((length > (FLASH_BLIND_STEP_LEN-9))||(length < 28)||(0 == Link1Blind.SaveEnableFlag))//ÿ����������ֽ����������ǣ����ϴ����
	{
		return 1;
	}
	
	if((Link1Blind.StartSector == Link1Blind.SaveSector)&&(0 == Link1Blind.SaveStep)&&(0 == Link1Blind.SaveTimeCount))//Ϊ��
	{
			
	}
	else//��Ϊ�գ��Ѵ�������
	{
		Sector = Link1Blind.SaveSector;
		Step = Link1Blind.SaveStep;
		Step--;//��ȡ�Ѵ洢�����µ���һ��ʱ��
		if(Step < 0)
		{
			Step = Link1Blind.SectorStep - 1;
			Sector--;
			if(Sector < Link1Blind.StartSector)
			{
				Sector = Link1Blind.EndSector - 1;
			}
		}
		Addr = Sector*FLASH_ONE_SECTOR_BYTES+Step*FLASH_BLIND_STEP_LEN;
		sFLASH_ReadBuffer(Buffer,Addr,4);
		ttCount = Public_ConvertBufferToLong(Buffer);
		Gmtime(&tt,ttCount);

		if(ERROR == CheckTimeStruct(&tt))//����Ѵ洢�����µ���һ��ʱ���Ƿ���ȷ
		{
			sFLASH_EraseSector(Addr);
			Link1Blind.SaveStep = 0;
			return 1;
		}

		if(ERROR == CheckTimeStruct(&CurTime))//��鵱ǰʱ���Ƿ���ȷ
		{
			return 1;
		}

		TimeCount = ConverseGmtime(&CurTime);//�Ƚϵ�ǰʱ�������¼�¼����һ��ʱ��
		if(TimeCount < ttCount)
		{
			return 1;
		}
		
	}
	
	Addr = Link1Blind.SaveSector*FLASH_ONE_SECTOR_BYTES+Link1Blind.SaveStep*FLASH_BLIND_STEP_LEN;//��鵱ǰ�洢����һ���ĵ�ַ�Ƿ�Ϊ��
	sFLASH_ReadBuffer(Buffer,Addr,5);
	for(i=0; i<5; i++)
	{
		if(0xff != Buffer[i])
		{
			sFLASH_EraseSector(Addr);
			Link1Blind.SaveStep = 0;
			Addr = Link1Blind.SaveSector*FLASH_ONE_SECTOR_BYTES;
			for(j=0; j<200; j++)
			{
				//�ȴ�һ���
			}
			break;
		}
	}

	sum = 0;//����У���
	for(i=0; i<length; i++)
	{
		sum += *(pBuffer+i);
	}

	
	memcpy(Buffer+7,pBuffer,length);//����λ����Ϣ������

	p = Buffer;//����7�ֽڵ�����
	*p++ = (TimeCount&0xff000000) >> 24;
	*p++ = (TimeCount&0xff0000) >> 16;
	*p++ = (TimeCount&0xff00) >> 8;
	*p++ = TimeCount&0xff;
	*p++ = attribute;
	*p++ = sum;
	*p++ = length;

	sFLASH_WriteBuffer(Buffer,Addr,length+7);//д��flash

	Link1Blind.SaveStep++;//����һ��
	if(Link1Blind.SaveStep >= Link1Blind.SectorStep)
	{
		Link1Blind.SaveSector++;
		Link1Blind.SaveStep = 0;
		if(Link1Blind.SaveSector >= Link1Blind.EndSector)
		{
			Link1Blind.SaveSector = Link1Blind.StartSector;
		}
		Addr = Link1Blind.SaveSector*FLASH_ONE_SECTOR_BYTES;
		sFLASH_EraseSector(Addr);
	}

	return 0;
	
}

//////////////////////////////////////////////////////////////////
/**************************************************************************
//��������Blind_Link2Init
//���ܣ�����2������ʼ��
//���룺��
//�������
//����ֵ����
//��ע��ä��������ʱ������Ҫ���ô˺���
***************************************************************************/
static void Blind_Link2Init(void)
{
	u8 i;

	//����2�����ӵ�TCP�˿ںţ��ж��Ƿ�����2�����ӣ�����������Ҫ��ִ洢����ÿ�θò���״̬�仯ʱ��Ҫ����flash
	//�����洢�����ȷֳ����飬ǰһ����������1����һ����������2
	Blind_UpdateLink2OpenFlag();
	Link2Blind.StartSector = FLASH_BLIND_MIDDLE_SECTOR;
	Link2Blind.EndSector = FLASH_BLIND_END_SECTOR;
	Link2Blind.SectorStep = FLASH_ONE_SECTOR_BYTES/FLASH_BLIND_STEP_LEN;
	Link2Blind.OldestSector = Link2Blind.StartSector;
	Link2Blind.OldestStep = 0;
	Link2Blind.OldestTimeCount = 0;//������Сֵ
	Link2Blind.SaveSector = Link2Blind.StartSector;
	Link2Blind.SaveStep = 0;
	Link2Blind.SaveTimeCount = 0;//������С��
	Link2Blind.SaveEnableFlag = 0;
	Link2Blind.ReportSendNum = 0;
	for(i=0; i<5; i++)
	{
		Link2Blind.LastReportSector[i] = 0;
		Link2Blind.LastReportStep[i] = 0;
	}
	Link2Blind.ErrorCount = 0;
	Link2Blind.State = BLIND_STATE_SEARCH;

	Link2TimeCount = 0;
	Link2WaitTimeCount = 0;	
	Link2SearchSector = Link2Blind.StartSector;

	
}
/**************************************************************************
//��������Blind_Link2Search
//���ܣ�����2����ȷ��ä����¼���ϱ��ĳ�ʼλ��
//���룺��
//�������
//����ֵ����
//��ע��ä��������ʱ������Ҫ���ô˺���
***************************************************************************/
static void Blind_Link2Search(void)
{
	if((Link2SearchSector >= Link2Blind.StartSector)&&(Link2SearchSector < Link2Blind.EndSector))
	{
		Blind_Link2SearchSector(Link2SearchSector);//��������1ĳ�������е�ÿ��ä����Ϣ
	}
	else
	{
		Link2Blind.State = BLIND_STATE_INIT;
		return ;
	}
	
	if(Link2Blind.ErrorCount > 3)
	{
		Link2Blind.ErrorCount = 0;
		Blind_Link2Erase();//��������1����ä��flash
		Link2Blind.State = BLIND_STATE_INIT;
		return ;
	}

	Link2SearchSector++;
	if(Link2SearchSector >= Link2Blind.EndSector)
	{
		if((Link2Blind.StartSector == Link2Blind.SaveSector)&&(0 == Link2Blind.SaveStep)&&(0 == Link2Blind.SaveTimeCount))
		{
			//ä������Ϊ��
		}
		else
		{
			Link2Blind.SaveStep++;
			if(Link2Blind.SaveStep >= Link2Blind.SectorStep)
			{
				Link2Blind.SaveStep = 0;
				Link2Blind.SaveSector++;
				if(Link2Blind.SaveSector >= Link2Blind.EndSector)
				{
					Link2Blind.SaveSector = Link2Blind.StartSector;
				}
			}
		}
		Link2Blind.ReportSector = Link2Blind.OldestSector;
		Link2Blind.ReportStep = Link2Blind.OldestStep;
		Link2Blind.SaveEnableFlag = 1;
		Link2Blind.State = BLIND_STATE_REPORT;
	}
}
/**************************************************************************
//��������Blind_Link2SearchSector
//���ܣ����ĳ�������е�ä����Ϣ
//���룺������
//�������ǰ�洢�Ͳ���ä����λ��
//����ֵ����
//��ע��
***************************************************************************/
static void Blind_Link2SearchSector(s16 SearchSector)
{
	s16 i,j;
	u32 Addr;
	u8 flag;
	u8 Buffer[FLASH_BLIND_STEP_LEN+1];
	u8 sum;
	TIME_T tt;
	u32 ttCount;

	Addr = SearchSector*FLASH_ONE_SECTOR_BYTES + Link2Blind.SectorStep*FLASH_BLIND_STEP_LEN + 1;
	sFLASH_ReadBuffer(&flag,Addr,1);//�����������Ƿ��Ѳ�����־
	if(0xaa == flag)
	{
		Addr = SearchSector*FLASH_ONE_SECTOR_BYTES + (Link2Blind.SectorStep-1)*FLASH_BLIND_STEP_LEN;
		sFLASH_ReadBuffer(Buffer,Addr,4);
		ttCount = Public_ConvertBufferToLong(Buffer);
		Gmtime(&tt,ttCount);
		if(SUCCESS == CheckTimeStruct(&tt))
		{
			if(ttCount > Link2Blind.OldestTimeCount)
			{
				Link2Blind.OldestSector = SearchSector;
				Link2Blind.OldestStep = Link2Blind.SectorStep-1;
				Link2Blind.OldestTimeCount = ttCount;
			}
		}
		return ;
	}

	Addr = SearchSector*FLASH_ONE_SECTOR_BYTES + (Link2Blind.SectorStep-1)*FLASH_BLIND_STEP_LEN;//���������1��
	sFLASH_ReadBuffer(Buffer,Addr,4);
	ttCount = Public_ConvertBufferToLong(Buffer);
	if(0xffffffff != ttCount)
	{
		Gmtime(&tt,ttCount);
		if(SUCCESS == CheckTimeStruct(&tt))
		{
			Addr = SearchSector*FLASH_ONE_SECTOR_BYTES + (Link2Blind.SectorStep-1)*FLASH_BLIND_STEP_LEN;
			sFLASH_ReadBuffer(Buffer,Addr,FLASH_BLIND_STEP_LEN);
			if(CHANNEL_DATA_2 != Buffer[4])//ͨ������ȷ
			{
				Link2Blind.ErrorCount++;
				if(Link2Blind.ErrorCount > 3)
				{
					Blind_Erase(0xff);//��������ä���洢����
					return;
				}
			}
			sum = 0;
			for(j=0; j<Buffer[6]; j++)
			{
				sum += Buffer[j+7];
			}
			if((sum != Buffer[5])||(0 == Buffer[6]))
			{
				Link2Blind.ErrorCount++;
				if(Link2Blind.ErrorCount > 3)
				{
					Blind_Erase(0xff);//��������ä���洢����
					return;
				}
			}
			else
			{
				if((ttCount > Link2Blind.OldestTimeCount)&&(0xaa == Buffer[FLASH_BLIND_STEP_LEN-1]))
				{
					Link2Blind.OldestSector = SearchSector;
					Link2Blind.OldestStep = 0;
					Link2Blind.OldestTimeCount = ttCount;
				}

				if(ttCount > Link2Blind.SaveTimeCount)
				{
					Link2Blind.SaveSector = SearchSector;
					Link2Blind.SaveStep = Link2Blind.SectorStep-1;
					Link2Blind.SaveTimeCount = ttCount;
				}
			}
		}
		else
		{
			Link2Blind.ErrorCount++;
			if(Link2Blind.ErrorCount > 3)
			{
				Blind_Erase(0xff);//��������ä���洢����
				return;
			}
		}
	}
	else
	{
		Addr = SearchSector*FLASH_ONE_SECTOR_BYTES;//��������1��
		sFLASH_ReadBuffer(Buffer,Addr,4);
		ttCount = Public_ConvertBufferToLong(Buffer);
		if(0xffffffff != ttCount)
		{
			for(i=0; i<Link2Blind.SectorStep; i++)
			{
				Addr = SearchSector*FLASH_ONE_SECTOR_BYTES + i*FLASH_BLIND_STEP_LEN;
				sFLASH_ReadBuffer(Buffer,Addr,FLASH_BLIND_STEP_LEN);
				ttCount = Public_ConvertBufferToLong(Buffer);
				if(0xffffffff != ttCount)
				{
					Gmtime(&tt,ttCount);
					if(SUCCESS == CheckTimeStruct(&tt))
					{
						if(CHANNEL_DATA_2 != Buffer[4])//ͨ������ȷ
						{
							Link2Blind.ErrorCount++;
							if(Link2Blind.ErrorCount > 3)
							{
								Blind_Erase(0xff);//��������ä���洢����
								return;
							}
						}
						sum = 0;
						for(j=0; j<Buffer[6]; j++)
						{
							sum += Buffer[j+7];
						}
						if((sum != Buffer[5])||(0 == Buffer[6]))
						{
							Link2Blind.ErrorCount++;
						}
						else
						{
							if((ttCount > Link2Blind.OldestTimeCount)&&(0xaa == Buffer[FLASH_BLIND_STEP_LEN-1]))
							{
								Link2Blind.OldestSector = SearchSector;
								Link2Blind.OldestStep = i;
								Link2Blind.OldestTimeCount = ttCount;
							}

							if(ttCount > Link2Blind.SaveTimeCount)
							{
								Link2Blind.SaveSector = SearchSector;
								Link2Blind.SaveStep = i;
								Link2Blind.SaveTimeCount = ttCount;
							}
						}
					}
					else
					{
						Link2Blind.ErrorCount++;
						if(Link2Blind.ErrorCount > 3)
						{
							Blind_Erase(0xff);//��������ä���洢����
							return;
						}
					}
				}
			}
		}		
		else
		{

		}
	}

/*
	Link2Blind.ErrorCount = 0;
	for(i=0; i<Link2Blind.SectorStep; i++)
	{
		Addr = SearchSector*FLASH_ONE_SECTOR_BYTES + i*FLASH_BLIND_STEP_LEN;
		sFLASH_ReadBuffer(Buffer,Addr,4);
		ttCount = Public_ConvertBufferToLong(Buffer);
		if(0xffffffff != ttCount)
		{
			Gmtime(&tt,ttCount);
			if(SUCCESS == CheckTimeStruct(&tt))
			{
				sFLASH_ReadBuffer(Buffer,Addr,FLASH_BLIND_STEP_LEN);
				if(CHANNEL_DATA_2 != Buffer[4])//ͨ������ȷ
				{
					Link2Blind.ErrorCount++;
					if(Link2Blind.ErrorCount > 3)
					{
						Blind_Erase(0xff);//��������ä���洢����
						break;
					}
				}
				sum = 0;
				for(j=0; j<Buffer[6]; j++)
				{
					sum += Buffer[j+7];
				}
				if((sum != Buffer[5])||(0 == Buffer[6]))
				{
					Link2Blind.ErrorCount++;
				}
				else
				{
					if((ttCount > Link2Blind.OldestTimeCount)&&(0xaa == Buffer[FLASH_BLIND_STEP_LEN-1]))
					{
						Link2Blind.OldestSector = SearchSector;
						Link2Blind.OldestStep = i;
						Link2Blind.OldestTimeCount = ttCount;
					}

					if(ttCount > Link2Blind.SaveTimeCount)
					{
						Link2Blind.SaveSector = SearchSector;
						Link2Blind.SaveStep = i;
						Link2Blind.SaveTimeCount = ttCount;
					}
				}
			}
			else
			{
				Link2Blind.ErrorCount++;
			}
		}
	}
*/
	
}
/**************************************************************************
//��������Blind_Link2Report
//���ܣ�����2�ϱ�һ��ä������
//���룺��
//�������
//����ֵ����
//��ע��ä��������ʱ������Ҫ���ô˺���
***************************************************************************/
static u8 Blind_Link2Report(void)
{
	u32 Addr;
	u8 i;
	u8 count = 0;
	u8 flag;
	u8 Buffer[FLASH_BLIND_STEP_LEN+1];
	u8 BufferLen;
	u8 sum;
	u16 length = 3;//Ԥ��3���ֽ�


	for(count=0; count<5; )//ÿ������5��λ����Ϣ
	{
		if(0 == Blind_GetLink2ReportFlag())
		{
			break;
		}
		Addr = Link2Blind.ReportSector*FLASH_ONE_SECTOR_BYTES + Link2Blind.SectorStep*FLASH_BLIND_STEP_LEN + 1;
		sFLASH_ReadBuffer(&flag,Addr,1);//�����������Ƿ��Ѳ�����־
		if(0xaa == flag)//�Ѳ�����
		{
			Link2Blind.ReportSector++;
			Link2Blind.ReportStep = 0;
			if(Link2Blind.ReportSector >= Link2Blind.EndSector)
			{
				Link2Blind.ReportSector = Link2Blind.StartSector;
			}
		}
		else
		{
			for(;count<5;)
			{
				if(0 == Blind_GetLink2ReportFlag())
				{
					break;
				}
				Addr = Link2Blind.ReportSector*FLASH_ONE_SECTOR_BYTES + (Link2Blind.ReportStep+1)*FLASH_BLIND_STEP_LEN - 1;
				sFLASH_ReadBuffer(&flag,Addr,1);//�������Ƿ��Ѳ�����־
				if(0xaa == flag)//�Ѳ�����
				{
					Link2Blind.ReportStep++;
				}
				else
				{
					Addr = Link2Blind.ReportSector*FLASH_ONE_SECTOR_BYTES + Link2Blind.ReportStep*FLASH_BLIND_STEP_LEN;
					sFLASH_ReadBuffer(Buffer,Addr,FLASH_BLIND_STEP_LEN);
					BufferLen = Buffer[6];
					sum = 0;
					for(i=0; i<BufferLen; i++)
					{
						sum += Buffer[7+i];
					}
					if(Buffer[5] == sum)
					{
						RadioShareBuffer[length] = 0;//���ȸ��ֽ��0
						memcpy(RadioShareBuffer+length+1,Buffer+6,BufferLen+1);
						length += (BufferLen+2);
						Link2Blind.LastReportSector[count] = Link2Blind.ReportSector;
						Link2Blind.LastReportStep[count] = Link2Blind.ReportStep;
						count++;
					}
					Link2Blind.ReportStep++;
				}
				if(Link2Blind.ReportStep >= Link2Blind.SectorStep)
				{
					Link2Blind.ReportSector++;
					Link2Blind.ReportStep = 0;
					if(Link2Blind.ReportSector >= Link2Blind.EndSector)
					{
						Link2Blind.ReportSector = Link2Blind.StartSector;
					}
					break;
				}
			}
		}
	}
	
	if(0 == count)
	{
		return 0;
	}
	else
	{
		for(i=count; i<5; i++)
		{
			Link2Blind.LastReportSector[i] = 0;
			Link2Blind.LastReportStep[i] = 0;
		}
		RadioShareBuffer[0] = 0;//�����ֽڱ�ʾ����
		RadioShareBuffer[1] = count;
		RadioShareBuffer[2] = 1;//0:������λ�������㱨;1:ä������
		RadioProtocol_PostionInformation_BulkUpTrans(CHANNEL_DATA_2,RadioShareBuffer,length);
	}

	return 1;
}
/**************************************************************************
//��������Blind_Link2ReportAck
//���ܣ�����2ä������Ӧ����
//���룺��
//�������
//����ֵ����
//��ע���յ�����2��ä������Ӧ��ʱ����ô˺���
***************************************************************************/
static void Blind_Link2ReportAck(void)
{
	u8 i;
	u8 flag;
	u32 Addr;
	s16 Sector;

	Link2Blind.ReportSendNum = 0;
	Link2TimeCount = BLIND_REPORT_DELAY_TIME-20;//1����ϱ���һ��

	Sector = Link2Blind.LastReportSector[0];

	for(i=0; i<5; i++)
	{
		if((Link2Blind.LastReportSector[i] >= Link2Blind.StartSector)
			&&(Link2Blind.LastReportSector[i] < Link2Blind.EndSector)
				&&(Link2Blind.LastReportStep[i] < Link2Blind.SectorStep))
		{
			Addr = Link2Blind.LastReportSector[i]*FLASH_ONE_SECTOR_BYTES+(Link2Blind.LastReportStep[i]+1)*FLASH_BLIND_STEP_LEN - 1;
			flag = 0xaa;
			sFLASH_WriteBuffer(&flag, Addr, 1);//��Ǹ���ä�����ϱ�

			if(Link2Blind.LastReportStep[i] >= (Link2Blind.SectorStep-1))
			{
				Addr = Sector*FLASH_ONE_SECTOR_BYTES+Link2Blind.SectorStep*FLASH_BLIND_STEP_LEN + 1;
				flag = 0xaa;
				sFLASH_WriteBuffer(&flag, Addr, 1);//��Ǹ�������ä�����ϱ�
			}
		}
		
	}

	for(i=0; i<5; i++)
	{
		Link2Blind.LastReportSector[i] = 0;
		Link2Blind.LastReportStep[i] = 0;
	}
	
		
}
/**************************************************************************
//��������Blind_GetLink2ReportFlag
//���ܣ���ȡ����2ä���ϱ���־
//���룺��
//�������
//����ֵ��ä���ϱ���־
//��ע��1��ʾ��Ҫ�ϱ���0��ʾ����Ҫ�ϱ����ն�ÿ��10���ӻ��Զ����һ�ο��Ƿ���Ҫ�ϱ�
***************************************************************************/
static u8 Blind_GetLink2ReportFlag(void)
{
	s16 Sector;
	s16 Step;

	Sector = Link2Blind.SaveSector;
	Step = 	Link2Blind.SaveStep;
/*
	if(0 != Link2Blind.SaveTimeCount)
	{
		Step--;
		if(Step < 0)
		{
			Step = Link2Blind.SectorStep-1;
			Sector--;
			if(Sector < Link2Blind.StartSector)
			{
				Sector = Link2Blind.EndSector - 1;
			}
		}
	}
*/
	if((Link2Blind.ReportSector == Sector)&&(Link2Blind.ReportStep >= Step))
	{
		return 0;
	}
	else
	{
		return 1;
	}
}
/**************************************************************************
//��������Blind_StartLink2Report
//���ܣ���������2ä������
//���룺��
//�������
//����ֵ����
//��ע������ä��������ÿ������1��Ȩ�ɹ�����ô˺���
***************************************************************************/
static void Blind_StartLink2Report(void)
{
	//if(BLIND_STATE_WAIT == Link1Blind.State)
	//{
		Link2Blind.State = BLIND_STATE_INIT;
	//}
}
/**************************************************************************
//��������Blind_Link2Erase
//���ܣ���������2ä���洢����
//���룺��
//�������
//����ֵ����
//��ע����⵽ä����¼��λ����Ϣ����ʱ���������������50������ô˺�����
//������Զ��FTP������������ʱ����ô˺�������Ϊä����Զ�̹̼���������һ������
***************************************************************************/
static void Blind_Link2Erase(void)
{
	u16 i;
	u32 Addr;
	
	if(1 == Link2OpenFlag)
	{
		Link2Blind.SaveEnableFlag = 0;//��ֹ�洢
		Link2Blind.State = BLIND_STATE_INIT;

		for(i=Link2Blind.StartSector; i<Link2Blind.EndSector; i++)
		{
			Addr = i*FLASH_ONE_SECTOR_BYTES;
			sFLASH_EraseSector(Addr);
			IWDG_ReloadCounter();
		}
	}	
}
/**************************************************************************
//��������Blind_Link2Save
//���ܣ��洢һ������2��ä������
//���룺һ��λ����Ϣ������
//�������
//����ֵ��0Ϊ�ɹ�����0Ϊʧ�ܣ����ȳ���ʱ�᷵��ʧ��
//��ע��һ��ä�����ݰ�����ʱ��4�ֽ�+����1�ֽڣ�Ԥ����+У���1�ֽ�+λ����Ϣ����1�ֽ�+λ����Ϣ�����ֽ�
***************************************************************************/
static u8 Blind_Link2Save(u8 *pBuffer, u8 length, u8 attribute)
{
	u8 Buffer[FLASH_BLIND_STEP_LEN+1] = {0};
	u8 i,j;
	u32 Addr;
	u32 TimeCount;
	u8 *p = NULL;
	u8 sum;
	s16 Sector;
	s16 Step;
	TIME_T tt;
	u32 ttCount;

	if((length > (FLASH_BLIND_STEP_LEN-9))||(length < 28)||(0 == Link2Blind.SaveEnableFlag))//ÿ����������ֽ����������ǣ����ϴ����
	{
		return 1;
	}
	
	if((Link2Blind.StartSector == Link2Blind.SaveSector)&&(0 == Link2Blind.SaveStep)&&(0 == Link2Blind.SaveTimeCount))//Ϊ��
	{
			
	}
	else//��Ϊ�գ��Ѵ�������
	{
		Sector = Link2Blind.SaveSector;
		Step = Link2Blind.SaveStep;
		Step--;//��ȡ�Ѵ洢�����µ���һ��ʱ��
		if(Step < 0)
		{
			Step = Link2Blind.SectorStep - 1;
			Sector--;
			if(Sector < Link2Blind.StartSector)
			{
				Sector = Link2Blind.EndSector - 1;
			}
		}
		Addr = Sector*FLASH_ONE_SECTOR_BYTES+Step*FLASH_BLIND_STEP_LEN;
		sFLASH_ReadBuffer(Buffer,Addr,4);
		ttCount = Public_ConvertBufferToLong(Buffer);
		Gmtime(&tt,ttCount);

		if(ERROR == CheckTimeStruct(&tt))//����Ѵ洢�����µ���һ��ʱ���Ƿ���ȷ
		{
			sFLASH_EraseSector(Addr);
			Link2Blind.SaveStep = 0;
			return 1;
		}

		if(ERROR == CheckTimeStruct(&CurTime))//��鵱ǰʱ���Ƿ���ȷ
		{
			return 1;
		}

		TimeCount = ConverseGmtime(&CurTime);//�Ƚϵ�ǰʱ�������¼�¼����һ��ʱ��
		if(TimeCount < ttCount)
		{
			return 1;
		}
		
	}
	
	Addr = Link2Blind.SaveSector*FLASH_ONE_SECTOR_BYTES+Link2Blind.SaveStep*FLASH_BLIND_STEP_LEN;//��鵱ǰ�洢����һ���ĵ�ַ�Ƿ�Ϊ��
	sFLASH_ReadBuffer(Buffer,Addr,5);
	for(i=0; i<5; i++)
	{
		if(0xff != Buffer[i])
		{
			sFLASH_EraseSector(Addr);
			Link2Blind.SaveStep = 0;
			Addr = Link2Blind.SaveSector*FLASH_ONE_SECTOR_BYTES;
			for(j=0; j<200; j++)
			{
				//�ȴ�һ���
			}
			break;
		}
	}

	sum = 0;//����У���
	for(i=0; i<length; i++)
	{
		sum += *(pBuffer+i);
	}

	
	memcpy(Buffer+7,pBuffer,length);//����λ����Ϣ������

	p = Buffer;//����7�ֽڵ�����
	*p++ = (TimeCount&0xff000000) >> 24;
	*p++ = (TimeCount&0xff0000) >> 16;
	*p++ = (TimeCount&0xff00) >> 8;
	*p++ = TimeCount&0xff;
	*p++ = attribute;
	*p++ = sum;
	*p++ = length;

	sFLASH_WriteBuffer(Buffer,Addr,length+7);//д��flash

	Link2Blind.SaveStep++;//����һ��
	if(Link2Blind.SaveStep >= Link2Blind.SectorStep)
	{
		Link2Blind.SaveSector++;
		Link2Blind.SaveStep = 0;
		if(Link2Blind.SaveSector >= Link2Blind.EndSector)
		{
			Link2Blind.SaveSector = Link2Blind.StartSector;
		}
		Addr = Link2Blind.SaveSector*FLASH_ONE_SECTOR_BYTES;
		sFLASH_EraseSector(Addr);
	}

	return 0;
	
}
/**************************************************************************
//��������Blind_UpdateLink2OpenFlag
//���ܣ����±���Link2OpenFlag
//���룺��
//�������
//����ֵ����
//��ע��Link2OpenFlagΪ1��ʾ�����˵�2������
***************************************************************************/
static void Blind_UpdateLink2OpenFlag(void)
{
	u8 Buffer[30];
	u8 BufferLen; 
	u32 TcpPort;

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
}



























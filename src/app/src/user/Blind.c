/************************************************************************
//程序名称：Blind.c
//功能：该模块实现盲区补报功能。
//版本号：V0.1
//版权说明：版权属于深圳市伊爱高新技术开发有限公司
//开发人：dxl
//开发时间：2014.10
//版本记录：版本记录需包含版本、修改人、修改时间、修改原因、修改简要说明
//V0.1：需要实现双连接的盲区补报。连接1和连接2的盲区存储区域是独立的、分开的。
//但如果只使用了连接1（连接2未开启，连接2 TCP端口参数设置为0时表示连接2不开启），
//两个存储区域合二为一。每次终端上线后会重新检测一次看是否需要补报，若有未补报的
//数据，则开始补报，每次把5条位置信息打包补报，只有收到平台应答后才会补报下一包，
//否则一直补报当前包，最多发送10次，每次20秒。若10次后仍然没有应答
//则会等待30分钟后再重发当前包。
*************************************************************************/

/********************文件包含*************************/
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

/********************本地变量*************************/
static BLIND_STRUCT Link1Blind;
static s16 Link1SearchSector;
static u16 Link1TimeCount = 0;
static u16 Link1WaitTimeCount = 0;

static BLIND_STRUCT Link2Blind;
static s16 Link2SearchSector;
static u16 Link2TimeCount = 0;
static u16 Link2WaitTimeCount = 0;
/********************全局变量*************************/
static u16 Link2OpenFlag = 0;//连接2是否开启标志，1为开启，0为未开启

/********************外部变量*************************/
extern u8 RadioShareBuffer[];
extern TIME_T CurTime;

/********************本地函数声明*********************/
static void Blind_Link1Init(void);//连接1变量初始化
static void Blind_Link1Search(void);//连接1查找确定盲区记录、上报的初始位置
static void Blind_Link1SearchSector(s16 SearchSector);//检查某个扇区中的盲区信息
static u8 Blind_Link1Report(void);//连接1上报一包盲区数据
static void Blind_Link1ReportAck(void);//连接1盲区补报应答处理
static u8 Blind_GetLink1ReportFlag(void);//获取连接1盲区上报标志
static void Blind_StartLink1Report(void);//开启连接1盲区补报
static void Blind_Link1Erase(void);//擦除连接1盲区存储区域
static u8 Blind_Link1Save(u8 *pBuffer, u8 length, u8 attribute);//存储一条连接1的盲区数据
static void Blind_StartLink1Report(void);

static void Blind_Link2Init(void);//连接2变量初始化
static void Blind_Link2Search(void);//连接2查找确定盲区记录、上报的初始位置
static void Blind_Link2SearchSector(s16 SearchSector);//检查某个扇区中的盲区信息
static u8 Blind_Link2Report(void);//连接2上报一包盲区数据
static void Blind_Link2ReportAck(void);//连接2盲区补报应答处理
static u8 Blind_GetLink2ReportFlag(void);//获取连接2盲区上报标志
static void Blind_StartLink2Report(void);//开启连接2盲区补报
static void Blind_Link2Erase(void);//擦除连接2盲区存储区域
static u8 Blind_Link2Save(u8 *pBuffer, u8 length, u8 attribute);//存储一条连接2的盲区数据
static void Blind_StartLink2Report(void);

static void Blind_UpdateLink2OpenFlag(void);//更新变量Link2OpenFlag
/********************函数定义*************************/
/**************************************************************************
//函数名：Blind_TimeTask
//功能：实现盲区补报功能
//输入：无
//输出：无
//返回值：无
//备注：盲区补报定时任务，50ms调度1次，任务调度器需要调用此函数。
//当在进行FTP升级时需要关闭该任务，因为盲区记录与固件远程升级共用一块存储区
//注意如果谁关闭了该任务，谁就要负责打开该任务，上电启动默认是打开的
***************************************************************************/
FunctionalState Blind_TimeTask(void)
{

	/**************连接1盲区补报****************/
	if(BLIND_STATE_INIT == Link1Blind.State)//初始化所有相关变量
	{
		Blind_Link1Init();//完成后自动转入BLIND_STATE_SEARCH
		
	}
	else if(BLIND_STATE_SEARCH == Link1Blind.State)//逐步查找确定当前的盲区补报位置
	{
		Blind_Link1Search();//完成后自动转入BLIND_STATE_REPORT
	}
	else if(BLIND_STATE_REPORT == Link1Blind.State)//盲区上报
	{
		Link1WaitTimeCount = 0;	
		//if(0x01 == (0x01&GetTerminalAuthorizationFlag()))//只有连接1在线时才补报
		//{
			Link1TimeCount++;
			if(Link1TimeCount >= BLIND_REPORT_DELAY_TIME)//20秒
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
					if(0 == Blind_Link1Report())//盲区上报
					{
						Gnss_StartBlindReport();
						Link1Blind.State = BLIND_STATE_WAIT;//转入等待状态
					}
				}
				else if(Link1Blind.ReportSendNum > 10)
				{
					Link1Blind.State = BLIND_STATE_WAIT;//转入等待状态
				}
			}
		//}
	}
	else if(BLIND_STATE_WAIT == Link1Blind.State)//等待
	{
		Link1WaitTimeCount++;
		if(Link1WaitTimeCount >= 36000)//30分钟
		{
			Link1WaitTimeCount = 0;
			Link1Blind.State = BLIND_STATE_INIT;
		}
	}
	else//异常
	{
		Link1Blind.State = BLIND_STATE_INIT;
	}


	/**************连接2盲区补报****************/
	if(0 == Link2OpenFlag)
	{
		return ENABLE;
	}

	if(BLIND_STATE_INIT == Link2Blind.State)//初始化所有相关变量
	{
		Blind_Link2Init();//完成后自动转入BLIND_STATE_SEARCH
		
	}
	else if(BLIND_STATE_SEARCH == Link2Blind.State)//逐步查找确定当前的盲区补报位置
	{
		Blind_Link2Search();//完成后自动转入BLIND_STATE_REPORT
	}
	else if(BLIND_STATE_REPORT == Link2Blind.State)//盲区上报
	{
		Link2WaitTimeCount = 0;	
		//if(0x01 == (0x01&GetTerminalAuthorizationFlag()))//只有连接1在线时才补报
		//{
			Link2TimeCount++;
			if(Link2TimeCount >= BLIND_REPORT_DELAY_TIME)//20秒
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
					if(0 == Blind_Link2Report())//盲区上报
					{
						Link2Blind.State = BLIND_STATE_WAIT;//转入等待状态
					}
				}
				else if(Link2Blind.ReportSendNum > 10)
				{
					Link2Blind.State = BLIND_STATE_WAIT;//转入等待状态
				}
			}
		//}
	}
	else if(BLIND_STATE_WAIT == Link2Blind.State)//等待
	{
		Link2WaitTimeCount++;
		if(Link2WaitTimeCount >= 36000)//30分钟
		{
			Link2WaitTimeCount = 0;
			Link2Blind.State = BLIND_STATE_INIT;
		}
	}
	else//异常
	{
		Link2Blind.State = BLIND_STATE_INIT;
	}

	

	return ENABLE;
}
/**************************************************************************
//函数名：Blind_Save
//功能：保存某通道的一条盲区数据（位置汇报数据）
//输入：通道号，盲区数据，数据长度
//输出：无
//返回值：0表示成功，1表示失败，参数不正确会返回失败
//备注：通道号只能为CHANNEL_SMS,CHANNEL_DATA_1,CHANNEL_DATA_2其中之一
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
//函数名：Blind_Erase
//功能：擦除某通道的所有存储扇区
//输入：通道号，只能为CHANNEL_SMS,CHANNEL_DATA_1,CHANNEL_DATA_2其中之一
//输出：无
//返回值：0表示成功，1表示失败，参数不正确会返回失败
//备注：要擦除所有的盲区数据，参数需为0xff，在下载远程升级固件之前需先擦除
//所有盲区区域，因为盲区存储区域与固件存储区域共用同一块flash空间
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
//函数名：Blind_StartReport
//功能：开始某个通道的盲区补报
//输入：通道号
//输出：无
//返回值：无
//备注：通道号只能为CHANNEL_SMS,CHANNEL_DATA_1,CHANNEL_DATA_2其中之一
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
//函数名：Blind_ReportAck
//功能：盲区补报应答处理
//输入：通道号，只能为CHANNEL_SMS,CHANNEL_DATA_1,CHANNEL_DATA_2其中之一
//输出：无
//返回值：0表示成功，1表示失败，参数不正确会返回失败
//备注：收到盲区补报应答后需调用此函数
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
//函数名：Blind_GetLink2OpenFlag
//功能：获取第2个连接开启标志
//输入：无
//输出：无
//返回值：第2个连接开启标志
//备注：Link2OpenFlag为1表示开启了第2个连接，为0表示没有开启
***************************************************************************/
u8 Blind_GetLink2OpenFlag(void)
{
	return Link2OpenFlag;
}
/**************************************************************************
//函数名：Blind_Link1Init
//功能：连接1变量初始化
//输入：无
//输出：无
//返回值：无
//备注：盲区补报定时任务需要调用此函数
***************************************************************************/
static void Blind_Link1Init(void)
{
	u8 i;

	//读第2个连接的TCP端口号，判断是否开启第2个连接，开启了则需要拆分存储区域，每次该参数状态变化时需要擦除flash
	//为了简单点，先不做切换，整个存储区域先分成两块，前一半属于连接1，后一半属于连接2
	Blind_UpdateLink2OpenFlag();
	if(1 == Link2OpenFlag)//开启了连接2
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
	Link1Blind.OldestTimeCount = 0;//赋给最小值
	Link1Blind.SaveSector = Link1Blind.StartSector;
	Link1Blind.SaveStep = 0;
	Link1Blind.SaveTimeCount = 0;//赋给最小值
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
//函数名：Blind_Link1Search
//功能：连接1查找确定盲区记录、上报的初始位置
//输入：无
//输出：无
//返回值：无
//备注：盲区补报定时任务需要调用此函数
***************************************************************************/
static void Blind_Link1Search(void)
{
	if((Link1SearchSector >= Link1Blind.StartSector)&&(Link1SearchSector < Link1Blind.EndSector))
	{
		Blind_Link1SearchSector(Link1SearchSector);//查找连接1某个扇区中的每条盲区信息
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
			//盲区区域为空
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
//函数名：Blind_Link1SearchSector
//功能：检查某个扇区中的盲区信息
//输入：扇区号
//输出：当前存储和补报盲区的位置
//返回值：无
//备注：
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
	sFLASH_ReadBuffer(&flag,Addr,1);//读整个扇区是否已补报标志
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

	Addr = SearchSector*FLASH_ONE_SECTOR_BYTES + (Link1Blind.SectorStep-1)*FLASH_BLIND_STEP_LEN;//读扇区的最后1步
	sFLASH_ReadBuffer(Buffer,Addr,4);
	ttCount = Public_ConvertBufferToLong(Buffer);
	if(0xffffffff != ttCount)
	{
		Gmtime(&tt,ttCount);
		if(SUCCESS == CheckTimeStruct(&tt))
		{
			sFLASH_ReadBuffer(Buffer,Addr,FLASH_BLIND_STEP_LEN);
			if(CHANNEL_DATA_1 != Buffer[4])//通道不正确
			{
				Link1Blind.ErrorCount++;
				if(Link1Blind.ErrorCount > 3)
				{
					Link1Blind.ErrorCount = 0;
					Blind_Erase(0xff);//擦除所有盲区存储区域
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
					Blind_Erase(0xff);//擦除所有盲区存储区域
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
				Blind_Erase(0xff);//擦除所有盲区存储区域
				return ;
			}
		}
	}
	else
	{
		Addr = SearchSector*FLASH_ONE_SECTOR_BYTES;//读扇区的第1步
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
					if(CHANNEL_DATA_1 != Buffer[4])//通道不正确
					{
						Link1Blind.ErrorCount++;
						if(Link1Blind.ErrorCount > 3)
						{
							Link1Blind.ErrorCount = 0;
							Blind_Erase(0xff);//擦除所有盲区存储区域
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
							Blind_Erase(0xff);//擦除所有盲区存储区域
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
						Blind_Erase(0xff);//擦除所有盲区存储区域
						return ;
					}
				}
			}
		}
	}

/*每个扇区每一步都检查，太浪费时间，优化后的只检查第1步和最后1步
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
				if(CHANNEL_DATA_1 != Buffer[4])//通道不正确
				{
					Link1Blind.ErrorCount++;
					if(Link1Blind.ErrorCount > 3)
					{
						Link1Blind.ErrorCount = 0;
						Blind_Erase(0xff);//擦除所有盲区存储区域
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
//函数名：Blind_Link1Report
//功能：连接1上报一包盲区数据
//输入：无
//输出：无
//返回值：无
//备注：盲区补报定时任务需要调用此函数
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
	u16 length = 3;//预留3个字节


	for(count=0; count<5; )//每包包含5条位置信息
	{
		if(0 == Blind_GetLink1ReportFlag())
		{
			break;
		}
		Addr = Link1Blind.ReportSector*FLASH_ONE_SECTOR_BYTES + Link1Blind.SectorStep*FLASH_BLIND_STEP_LEN + 1;
		sFLASH_ReadBuffer(&flag,Addr,1);//读整个扇区是否已补报标志
		if(0xaa == flag)//已补报过
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
				sFLASH_ReadBuffer(&flag,Addr,1);//读该条是否已补报标志
				if(0xaa == flag)//已补报过
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
						RadioShareBuffer[length] = 0;//长度高字节填补0
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
		RadioShareBuffer[0] = 0;//两个字节表示项数
		RadioShareBuffer[1] = count;
		RadioShareBuffer[2] = 1;//0:正常的位置批量汇报;1:盲区补报
		RadioProtocol_PostionInformation_BulkUpTrans(CHANNEL_DATA_1,RadioShareBuffer,length);
	}

	return 1;
}
/**************************************************************************
//函数名：Blind_Link1ReportAck
//功能：连接1盲区补报应答处理
//输入：无
//输出：无
//返回值：无
//备注：收到连接1的盲区补报应答时需调用此函数
***************************************************************************/
static void Blind_Link1ReportAck(void)
{
	u8 i;
	u8 flag;
	u32 Addr;
	s16 Sector;

	Link1Blind.ReportSendNum = 0;
	Link1TimeCount = BLIND_REPORT_DELAY_TIME-20;//1秒后上报下一包

	Sector = Link1Blind.LastReportSector[0];

	for(i=0; i<5; i++)
	{
		if((Link1Blind.LastReportSector[i] >= Link1Blind.StartSector)
			&&(Link1Blind.LastReportSector[i] < Link1Blind.EndSector)
				&&(Link1Blind.LastReportStep[i] < Link1Blind.SectorStep))
		{
			Addr = Link1Blind.LastReportSector[i]*FLASH_ONE_SECTOR_BYTES+(Link1Blind.LastReportStep[i]+1)*FLASH_BLIND_STEP_LEN - 1;
			flag = 0xaa;
			sFLASH_WriteBuffer(&flag, Addr, 1);//标记该条盲区已上报

			if(Link1Blind.LastReportStep[i] >= (Link1Blind.SectorStep-1))
			{
				Addr = Sector*FLASH_ONE_SECTOR_BYTES+Link1Blind.SectorStep*FLASH_BLIND_STEP_LEN + 1;
				flag = 0xaa;
				sFLASH_WriteBuffer(&flag, Addr, 1);//标记该扇区的盲区已上报
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
//函数名：Blind_GetLink1ReportFlag
//功能：获取连接1盲区上报标志
//输入：无
//输出：无
//返回值：盲区上报标志
//备注：1表示需要上报，0表示不需要上报，终端每隔10分钟会自动检测一次看是否需要上报
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
//函数名：Blind_StartLink1Report
//功能：开启连接1盲区补报
//输入：无
//输出：无
//返回值：无
//备注：开启盲区补报，每次连接1鉴权成功需调用此函数
***************************************************************************/
static void Blind_StartLink1Report(void)
{
	//if(BLIND_STATE_WAIT == Link1Blind.State)
	//{
		Link1Blind.State = BLIND_STATE_INIT;
	//}
}
/**************************************************************************
//函数名：Blind_Link1Erase
//功能：擦除连接1盲区存储区域
//输入：无
//输出：无
//返回值：无
//备注：检测到盲区记录的位置信息有误时（有误的条数大于50）会调用此函数，
//或者是远程FTP升级擦除区域时会调用此函数，因为盲区与远程固件升级共用一块区域
***************************************************************************/
static void Blind_Link1Erase(void)
{
	u16 i;
	u32 Addr;
	
	Link1Blind.SaveEnableFlag = 0;//禁止存储
	Link1Blind.State = BLIND_STATE_INIT;

	for(i=Link1Blind.StartSector; i<Link1Blind.EndSector; i++)
	{
		Addr = i*FLASH_ONE_SECTOR_BYTES;
		sFLASH_EraseSector(Addr);
		IWDG_ReloadCounter();
	}	
}
/**************************************************************************
//函数名：Blind_Link1Save
//功能：存储一条连接1的盲区数据
//输入：一条位置信息及长度
//输出：无
//返回值：0为成功，非0为失败，长度超出时会返回失败
//备注：一条盲区数据包括：时间4字节+属性1字节（预留）+校验和1字节+位置信息长度1字节+位置信息若干字节
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

	if((length > (FLASH_BLIND_STEP_LEN-9))||(length < 28)||(0 == Link1Blind.SaveEnableFlag))//每步最后两个字节用作错误标记，已上传标记
	{
		return 1;
	}
	
	if((Link1Blind.StartSector == Link1Blind.SaveSector)&&(0 == Link1Blind.SaveStep)&&(0 == Link1Blind.SaveTimeCount))//为空
	{
			
	}
	else//不为空，已存有数据
	{
		Sector = Link1Blind.SaveSector;
		Step = Link1Blind.SaveStep;
		Step--;//获取已存储的最新的那一步时间
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

		if(ERROR == CheckTimeStruct(&tt))//检查已存储的最新的那一步时间是否正确
		{
			sFLASH_EraseSector(Addr);
			Link1Blind.SaveStep = 0;
			return 1;
		}

		if(ERROR == CheckTimeStruct(&CurTime))//检查当前时间是否正确
		{
			return 1;
		}

		TimeCount = ConverseGmtime(&CurTime);//比较当前时间与最新记录的那一步时间
		if(TimeCount < ttCount)
		{
			return 1;
		}
		
	}
	
	Addr = Link1Blind.SaveSector*FLASH_ONE_SECTOR_BYTES+Link1Blind.SaveStep*FLASH_BLIND_STEP_LEN;//检查当前存储的那一步的地址是否为空
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
				//等待一会儿
			}
			break;
		}
	}

	sum = 0;//计算校验和
	for(i=0; i<length; i++)
	{
		sum += *(pBuffer+i);
	}

	
	memcpy(Buffer+7,pBuffer,length);//拷贝位置信息至缓冲

	p = Buffer;//补填7字节的数据
	*p++ = (TimeCount&0xff000000) >> 24;
	*p++ = (TimeCount&0xff0000) >> 16;
	*p++ = (TimeCount&0xff00) >> 8;
	*p++ = TimeCount&0xff;
	*p++ = attribute;
	*p++ = sum;
	*p++ = length;

	sFLASH_WriteBuffer(Buffer,Addr,length+7);//写入flash

	Link1Blind.SaveStep++;//下移一步
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
//函数名：Blind_Link2Init
//功能：连接2变量初始化
//输入：无
//输出：无
//返回值：无
//备注：盲区补报定时任务需要调用此函数
***************************************************************************/
static void Blind_Link2Init(void)
{
	u8 i;

	//读第2个连接的TCP端口号，判断是否开启第2个连接，开启了则需要拆分存储区域，每次该参数状态变化时需要擦除flash
	//整个存储区域先分成两块，前一半属于连接1，后一半属于连接2
	Blind_UpdateLink2OpenFlag();
	Link2Blind.StartSector = FLASH_BLIND_MIDDLE_SECTOR;
	Link2Blind.EndSector = FLASH_BLIND_END_SECTOR;
	Link2Blind.SectorStep = FLASH_ONE_SECTOR_BYTES/FLASH_BLIND_STEP_LEN;
	Link2Blind.OldestSector = Link2Blind.StartSector;
	Link2Blind.OldestStep = 0;
	Link2Blind.OldestTimeCount = 0;//赋给最小值
	Link2Blind.SaveSector = Link2Blind.StartSector;
	Link2Blind.SaveStep = 0;
	Link2Blind.SaveTimeCount = 0;//赋给最小给
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
//函数名：Blind_Link2Search
//功能：连接2查找确定盲区记录、上报的初始位置
//输入：无
//输出：无
//返回值：无
//备注：盲区补报定时任务需要调用此函数
***************************************************************************/
static void Blind_Link2Search(void)
{
	if((Link2SearchSector >= Link2Blind.StartSector)&&(Link2SearchSector < Link2Blind.EndSector))
	{
		Blind_Link2SearchSector(Link2SearchSector);//查找连接1某个扇区中的每条盲区信息
	}
	else
	{
		Link2Blind.State = BLIND_STATE_INIT;
		return ;
	}
	
	if(Link2Blind.ErrorCount > 3)
	{
		Link2Blind.ErrorCount = 0;
		Blind_Link2Erase();//擦除连接1整个盲区flash
		Link2Blind.State = BLIND_STATE_INIT;
		return ;
	}

	Link2SearchSector++;
	if(Link2SearchSector >= Link2Blind.EndSector)
	{
		if((Link2Blind.StartSector == Link2Blind.SaveSector)&&(0 == Link2Blind.SaveStep)&&(0 == Link2Blind.SaveTimeCount))
		{
			//盲区区域为空
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
//函数名：Blind_Link2SearchSector
//功能：检查某个扇区中的盲区信息
//输入：扇区号
//输出：当前存储和补报盲区的位置
//返回值：无
//备注：
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
	sFLASH_ReadBuffer(&flag,Addr,1);//读整个扇区是否已补报标志
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

	Addr = SearchSector*FLASH_ONE_SECTOR_BYTES + (Link2Blind.SectorStep-1)*FLASH_BLIND_STEP_LEN;//读扇区最后1步
	sFLASH_ReadBuffer(Buffer,Addr,4);
	ttCount = Public_ConvertBufferToLong(Buffer);
	if(0xffffffff != ttCount)
	{
		Gmtime(&tt,ttCount);
		if(SUCCESS == CheckTimeStruct(&tt))
		{
			Addr = SearchSector*FLASH_ONE_SECTOR_BYTES + (Link2Blind.SectorStep-1)*FLASH_BLIND_STEP_LEN;
			sFLASH_ReadBuffer(Buffer,Addr,FLASH_BLIND_STEP_LEN);
			if(CHANNEL_DATA_2 != Buffer[4])//通道不正确
			{
				Link2Blind.ErrorCount++;
				if(Link2Blind.ErrorCount > 3)
				{
					Blind_Erase(0xff);//擦除所有盲区存储区域
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
					Blind_Erase(0xff);//擦除所有盲区存储区域
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
				Blind_Erase(0xff);//擦除所有盲区存储区域
				return;
			}
		}
	}
	else
	{
		Addr = SearchSector*FLASH_ONE_SECTOR_BYTES;//读扇区第1步
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
						if(CHANNEL_DATA_2 != Buffer[4])//通道不正确
						{
							Link2Blind.ErrorCount++;
							if(Link2Blind.ErrorCount > 3)
							{
								Blind_Erase(0xff);//擦除所有盲区存储区域
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
							Blind_Erase(0xff);//擦除所有盲区存储区域
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
				if(CHANNEL_DATA_2 != Buffer[4])//通道不正确
				{
					Link2Blind.ErrorCount++;
					if(Link2Blind.ErrorCount > 3)
					{
						Blind_Erase(0xff);//擦除所有盲区存储区域
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
//函数名：Blind_Link2Report
//功能：连接2上报一包盲区数据
//输入：无
//输出：无
//返回值：无
//备注：盲区补报定时任务需要调用此函数
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
	u16 length = 3;//预留3个字节


	for(count=0; count<5; )//每包包含5条位置信息
	{
		if(0 == Blind_GetLink2ReportFlag())
		{
			break;
		}
		Addr = Link2Blind.ReportSector*FLASH_ONE_SECTOR_BYTES + Link2Blind.SectorStep*FLASH_BLIND_STEP_LEN + 1;
		sFLASH_ReadBuffer(&flag,Addr,1);//读整个扇区是否已补报标志
		if(0xaa == flag)//已补报过
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
				sFLASH_ReadBuffer(&flag,Addr,1);//读该条是否已补报标志
				if(0xaa == flag)//已补报过
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
						RadioShareBuffer[length] = 0;//长度高字节填补0
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
		RadioShareBuffer[0] = 0;//两个字节表示项数
		RadioShareBuffer[1] = count;
		RadioShareBuffer[2] = 1;//0:正常的位置批量汇报;1:盲区补报
		RadioProtocol_PostionInformation_BulkUpTrans(CHANNEL_DATA_2,RadioShareBuffer,length);
	}

	return 1;
}
/**************************************************************************
//函数名：Blind_Link2ReportAck
//功能：连接2盲区补报应答处理
//输入：无
//输出：无
//返回值：无
//备注：收到连接2的盲区补报应答时需调用此函数
***************************************************************************/
static void Blind_Link2ReportAck(void)
{
	u8 i;
	u8 flag;
	u32 Addr;
	s16 Sector;

	Link2Blind.ReportSendNum = 0;
	Link2TimeCount = BLIND_REPORT_DELAY_TIME-20;//1秒后上报下一包

	Sector = Link2Blind.LastReportSector[0];

	for(i=0; i<5; i++)
	{
		if((Link2Blind.LastReportSector[i] >= Link2Blind.StartSector)
			&&(Link2Blind.LastReportSector[i] < Link2Blind.EndSector)
				&&(Link2Blind.LastReportStep[i] < Link2Blind.SectorStep))
		{
			Addr = Link2Blind.LastReportSector[i]*FLASH_ONE_SECTOR_BYTES+(Link2Blind.LastReportStep[i]+1)*FLASH_BLIND_STEP_LEN - 1;
			flag = 0xaa;
			sFLASH_WriteBuffer(&flag, Addr, 1);//标记该条盲区已上报

			if(Link2Blind.LastReportStep[i] >= (Link2Blind.SectorStep-1))
			{
				Addr = Sector*FLASH_ONE_SECTOR_BYTES+Link2Blind.SectorStep*FLASH_BLIND_STEP_LEN + 1;
				flag = 0xaa;
				sFLASH_WriteBuffer(&flag, Addr, 1);//标记该扇区的盲区已上报
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
//函数名：Blind_GetLink2ReportFlag
//功能：获取连接2盲区上报标志
//输入：无
//输出：无
//返回值：盲区上报标志
//备注：1表示需要上报，0表示不需要上报，终端每隔10分钟会自动检测一次看是否需要上报
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
//函数名：Blind_StartLink2Report
//功能：开启连接2盲区补报
//输入：无
//输出：无
//返回值：无
//备注：开启盲区补报，每次连接1鉴权成功需调用此函数
***************************************************************************/
static void Blind_StartLink2Report(void)
{
	//if(BLIND_STATE_WAIT == Link1Blind.State)
	//{
		Link2Blind.State = BLIND_STATE_INIT;
	//}
}
/**************************************************************************
//函数名：Blind_Link2Erase
//功能：擦除连接2盲区存储区域
//输入：无
//输出：无
//返回值：无
//备注：检测到盲区记录的位置信息有误时（有误的条数大于50）会调用此函数，
//或者是远程FTP升级擦除区域时会调用此函数，因为盲区与远程固件升级共用一块区域
***************************************************************************/
static void Blind_Link2Erase(void)
{
	u16 i;
	u32 Addr;
	
	if(1 == Link2OpenFlag)
	{
		Link2Blind.SaveEnableFlag = 0;//禁止存储
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
//函数名：Blind_Link2Save
//功能：存储一条连接2的盲区数据
//输入：一条位置信息及长度
//输出：无
//返回值：0为成功，非0为失败，长度超出时会返回失败
//备注：一条盲区数据包括：时间4字节+属性1字节（预留）+校验和1字节+位置信息长度1字节+位置信息若干字节
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

	if((length > (FLASH_BLIND_STEP_LEN-9))||(length < 28)||(0 == Link2Blind.SaveEnableFlag))//每步最后两个字节用作错误标记，已上传标记
	{
		return 1;
	}
	
	if((Link2Blind.StartSector == Link2Blind.SaveSector)&&(0 == Link2Blind.SaveStep)&&(0 == Link2Blind.SaveTimeCount))//为空
	{
			
	}
	else//不为空，已存有数据
	{
		Sector = Link2Blind.SaveSector;
		Step = Link2Blind.SaveStep;
		Step--;//获取已存储的最新的那一步时间
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

		if(ERROR == CheckTimeStruct(&tt))//检查已存储的最新的那一步时间是否正确
		{
			sFLASH_EraseSector(Addr);
			Link2Blind.SaveStep = 0;
			return 1;
		}

		if(ERROR == CheckTimeStruct(&CurTime))//检查当前时间是否正确
		{
			return 1;
		}

		TimeCount = ConverseGmtime(&CurTime);//比较当前时间与最新记录的那一步时间
		if(TimeCount < ttCount)
		{
			return 1;
		}
		
	}
	
	Addr = Link2Blind.SaveSector*FLASH_ONE_SECTOR_BYTES+Link2Blind.SaveStep*FLASH_BLIND_STEP_LEN;//检查当前存储的那一步的地址是否为空
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
				//等待一会儿
			}
			break;
		}
	}

	sum = 0;//计算校验和
	for(i=0; i<length; i++)
	{
		sum += *(pBuffer+i);
	}

	
	memcpy(Buffer+7,pBuffer,length);//拷贝位置信息至缓冲

	p = Buffer;//补填7字节的数据
	*p++ = (TimeCount&0xff000000) >> 24;
	*p++ = (TimeCount&0xff0000) >> 16;
	*p++ = (TimeCount&0xff00) >> 8;
	*p++ = TimeCount&0xff;
	*p++ = attribute;
	*p++ = sum;
	*p++ = length;

	sFLASH_WriteBuffer(Buffer,Addr,length+7);//写入flash

	Link2Blind.SaveStep++;//下移一步
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
//函数名：Blind_UpdateLink2OpenFlag
//功能：更新变量Link2OpenFlag
//输入：无
//输出：无
//返回值：无
//备注：Link2OpenFlag为1表示开启了第2个连接
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



























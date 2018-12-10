/******************************************************************** 
//版权说明	:Shenzhen E-EYE Co. Ltd., Copyright 2009- All Rights Reserved.
//文件名称	:StatusAlarm.c		
//功能		:实现状态位变化或报警立即上报或立即拍照功能
//版本号	:V0.1
//开发人	:dxl
//开发时间	:2014.11
//修改者	:
//修改时间	:
//修改简要说明	:
//备注		:
***********************************************************************/
//***************包含文件*****************
#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"

#include "GPIOControl.h"
#include "Io.h"
#include "modem_lib.h"
#include "modem_app_com.h"
#include "Report.h"
#include "Blind.h"
#include "Other.h"
#include "RadioProtocol.h"
#include "EepromPram.h"
#include "taskschedule.h"
#include "SysTickApp.h"
#include "Camera_App.h"
#include "StatusAlarm.h"

/********************本地变量*************************/
/********************全局变量*************************/
/********************外部变量*************************/
extern u32	AlarmPhotoSwitch;//报警拍照开关,1为使能,0不使能
extern u32	AlarmPhotoStoreFlag;//报警拍摄存储标志,1为存储,0为不存储
extern u32	StatusPhotoSwitch;//状态变化拍照开关,1为使能,0不使能
extern u32	StatusPhotoStoreFlag;//状态变化拍照存储标志,1为存储,0为不存储
extern u32      PhotoID;//多媒体ID
extern u32	Status;//32位状态位,对应基本位置信息汇报里的状态位定义
extern u32	Alarm;//32位报警位,对应基本位置信息汇报里的报警位定义
/********************本地函数声明*********************/


/*********************************************************************
//函数名称	:StatusAlarm_TimeTask
//功能		:实现状态位变化或产生报警立即上报或拍照功能
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		: 50ms调度一次，事件类的拍照图片都是上传+存储
*********************************************************************/
FunctionalState StatusAlarm_TimeTask(void)
{
        static u32	LastStatus = 0;//上一次IO状态
	static u32	LastAlarm = 0;//上一次报警状态
	static u32	EmergencyDelayCount = 0;//紧急报警计数
	static u32	EnterCount = 0;
	u8	EmergencyFlag = 0;//当前紧急报警状态
	u8	PhoneCallBackType = 0;//电话回拨类型
	u8	Buffer[21];
	u8	PramLen = 0;
	u8 	i;
	u8	PhotoEnable = 0;//拍照使能
	u8	bit;//位移偏移
	u8	ReportFlag = 0;
	u8	PhotoNum = 1;//拍照张数
	u8	PhotoEventType = 0;//拍照事件类型
        u32	temp;
        
    //***************报警处理****************
	EmergencyFlag =  Io_ReadAlarmBit(ALARM_BIT_EMERGENCY);
	if(1 == EmergencyFlag)
	{
                //PhotoStoreFlag = AlarmPhotoStoreFlag&((1 << ALARM_BIT_EMERGENCY)>>ALARM_BIT_EMERGENCY);
                
		EmergencyDelayCount++;
		if(1 == EmergencyDelayCount)
		{
                        Report_UploadPositionInfo(CHANNEL_DATA_1|CHANNEL_DATA_2);//上报一条位置信息
                        ReportFlag = 1;
			Io_WriteRecordConrtolBit(RECORD_BIT_EMERGENCY, SET);//开启录音
		}
		else if(2 == EmergencyDelayCount)
		{ 
			if((0 == Io_ReadAlarmBit(ALARM_BIT_CAMERA_FAULT))
			   &&(0 != Camera_GetTotalNum())
		           &&((AlarmPhotoSwitch&(1 << ALARM_BIT_EMERGENCY))==(1 << ALARM_BIT_EMERGENCY))
			   &&(0 != GetTerminalAuthorizationFlag()))
			{
				if(1 == Blind_GetLink2OpenFlag())
			        {
				        RadioProtocol_MultiMediaEvenReport(CHANNEL_DATA_1|CHANNEL_DATA_2,PhotoID+1, 0, 0, 2, 0);//发送多媒体事件
			        }
			        else
			        {
				        RadioProtocol_MultiMediaEvenReport(CHANNEL_DATA_1,PhotoID+1, 0, 0, 2, 0);//发送多媒体事件
			        }
				
			}
		}

		else if(3 == EmergencyDelayCount)
		{
			//判断是否有摄像头在线
			if((0 == Io_ReadAlarmBit(ALARM_BIT_CAMERA_FAULT))
			   &&(0 != Camera_GetTotalNum())
			   &&((AlarmPhotoSwitch&(1 << ALARM_BIT_EMERGENCY))==(1 << ALARM_BIT_EMERGENCY))
		           &&(0 != GetTerminalAuthorizationFlag()))
			{
				
			        if(1 == Blind_GetLink2OpenFlag())
			        {
				        Camera_Photo(CHANNEL_DATA_1|CHANNEL_DATA_2, 0, 1, 10, 1, 0x03, 1, 0x02);
			        }
			        else
			        {
				        Camera_Photo(CHANNEL_DATA_1, 0, 1, 10, 1, 0x03, 1, 0x02);
			        }
			}
			else
			{
				PhoneCallBackType = 1;
				PramLen = EepromPram_ReadPram(E2_MONITOR_PHONE_ID, Buffer);//读取预设的监听电话号码
				if((PramLen>0)&&(PramLen<20))
				{	
					EepromPram_WritePram(E2_PHONE_CALLBACK_ID, Buffer, PramLen);//把电话号码写入eeprom
					EepromPram_WritePram(E2_CALLBACK_FLAG_ID, &PhoneCallBackType, 1);//写入标志位
					SetEvTask(EV_SEV_CALL);//触发电话回拨任务
				}
			}
			
		}
	}
	else
	{
		EmergencyDelayCount = 0;
	}
        
        PhotoEnable = 0;
	//报警触发事件拍照
	if(LastAlarm != Alarm)
	{
		temp = LastAlarm ^ Alarm;
		temp = temp & Alarm;
		bit = 0;
		for(i=0; i<32; i++)
		{
			if((1 << i) == (temp&(1 << i)))
			{
				bit = i;
				if((1 << bit) == (AlarmPhotoSwitch&(1 << bit)))
				{
					switch(bit)
					{
						case ALARM_BIT_EMERGENCY://紧急报警有专门处理流程,不在这触发
							{
								PhotoEnable = 0;//使能拍照
								break;
							}
						case ALARM_BIT_SIDE_TURN_PRE_ALARM://侧翻报警
						case ALARM_BIT_IMPACT_PRE_ALARM://碰撞报警
							{
								//PhotoStoreFlag = (AlarmPhotoStoreFlag&(1 << bit)) >> bit;
								PhotoNum = 1;
								PhotoEnable = 1;//使能拍照
								PhotoEventType = 3;
								break;
							}
						default :
							{
								//PhotoStoreFlag = (AlarmPhotoStoreFlag&(1 << bit)) >> bit;
								PhotoNum = 1;
								PhotoEnable = 1;//使能拍照
								PhotoEventType = bit+224;
								break;
							}
					}
				}
			}
			if(1 == PhotoEnable)
			{
				break;
			}
		}
		
	}
	
        //状态变化触发事件拍照
	if((LastStatus != Status)&&(0 == PhotoEnable))
	{
		temp = LastStatus ^ Status;
		bit = 0;
		for(i=0; i<32; i++)
		{
			if((1 << i) == (temp&(1 << i)))
			{
				bit = i;
				if((1 << bit) == (StatusPhotoSwitch&(1 << bit)))
				{
					switch(bit)
					{
						case STATUS_BIT_NAVIGATION://定位
						case STATUS_BIT_SOUTH://南纬
						case STATUS_BIT_EAST://东经
						case STATUS_BIT_ENCRYPT://经纬度保密
							{
								PhotoEnable = 0;//不使能拍照
								break;
							}
						case STATUS_BIT_DOOR1://门1
						case STATUS_BIT_DOOR2://门2
						case STATUS_BIT_DOOR3://门3
						case STATUS_BIT_DOOR4://门4
						case STATUS_BIT_DOOR5://门5
							{
								if((1 << bit) == (Status&(1 << bit)))//开门拍照
								{
									PhotoEnable = 1;//使能拍照
									//PhotoStoreFlag = (StatusPhotoStoreFlag&(1 << bit)) >> bit;
									PhotoNum = 1;
									PhotoEventType = 4;
								}
								break;
							}
						case STATUS_BIT_FULL_LOAD2://空重车
							{
								if((1 << bit) == (Status&(1 << bit)))//空转重拍照
								{
									PhotoEnable = 1;//使能拍照
									//PhotoStoreFlag = (StatusPhotoStoreFlag&(1 << bit)) >> bit;
									PhotoEventType = bit+192;
								}
								break;
							}
						default :
							{
								PhotoEnable = 1;//使能拍照
								//PhotoStoreFlag = (StatusPhotoStoreFlag&(1 << bit)) >> bit;
								PhotoNum = 1;
								PhotoEventType = bit+192;
								break;
							}
					}
				}
			}
			if(1 == PhotoEnable)
			{
				break;
			}
		}
		
	}
        EnterCount++;
	if((1 == PhotoEnable)&&(EnterCount >= 60*SECOND))//上电60秒以后才允许拍照
	{
		if(1 == Blind_GetLink2OpenFlag())
		{
			Camera_Photo(CHANNEL_DATA_1|CHANNEL_DATA_2, 0, PhotoNum, 10, 1, 0x03, 1, PhotoEventType);
		}
		else
		{
			Camera_Photo(CHANNEL_DATA_1, 0, PhotoNum, 10, 1, 0x03, 1, PhotoEventType);
		}
	}
        
        //状态变化立即上报
        if((LastStatus != Status)&&(0 == ReportFlag))
        {
                Report_UploadPositionInfo(CHANNEL_DATA_1|CHANNEL_DATA_2);//上报一条位置信息
                ReportFlag = 1;
        }
        //产生报警立即上报
        if((LastAlarm != Alarm)&&(0 == ReportFlag))
        {
                Report_UploadPositionInfo(CHANNEL_DATA_1|CHANNEL_DATA_2);//上报一条位置信息
                ReportFlag = 1;
        }
        LastAlarm = Alarm;
	LastStatus = Status;
        
        return ENABLE;
}
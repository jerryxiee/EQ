/************************************************************************
//程序名称：Photo.c
//功能：实现事件触发拍照，参数定时、定距拍照功能
//版本号：V0.1
//版权说明：版权属于深圳市伊爱高新技术开发有限公司
//开发人：dxl
//开发时间：2014.11
//版本记录：版本记录需包含版本、修改人、修改时间、修改原因、修改简要说明
//V0.1：通过调用Camera.c提供的拍照接口Camera_Photo实现事件触发拍照，参数定时、定距拍照功能
//拍照优先级从高到低分别为：平台下发指令拍照，事件触发拍照、定时参数拍照、
//定距参数拍照，只有在处理完高优先级的拍照请求后才会处理低优先级的拍照请求。
*************************************************************************/

/********************文件包含*************************/
#include <stdio.h>
#include <string.h>

#include "Photo.h"
#include "EepromPram.h"
#include "Public.h"
#include "GPIOControl.h"
#include "Io.h"
#include "modem_lib.h"
#include "modem_app_com.h"
#include "Blind.h"
#include "Camera_App.h"
#include "SpeedMonitor.h"
#include "Pulse_App.h"
#include "MileMeter.h"
#include "fm25c160.h"

/********************本地变量*************************/
static u32 AlarmPhotoSwitch = 0;//报警拍照开关，对应位为1表示使能
static u32 AlarmPhotoStoreFlag = 0;//报警拍照存储标志，对应位为1表示存储
static u32 StatusPhotoSwitch = 0;//状态变化拍照开关，对应位为1表示使能
static u32 StatusPhotoStoreFlag = 0;//状态变化拍照存储标志，对应位为1表示存储

static u32 LastAlarm = 0;//上一次报警字
static u32 LastStatus = 0;//上一次状态字
//static u32 LastExtCarStatus = 0;//上一次扩展车辆状态字
//static u16 LastIoStatus = 0;//上一次IO状态字
//static u32 LastSelfDefine2 = 0;//上一次自定义表2报警状态字

static u32 NextPhotoDistance = 0;//下一次拍照距离

static u32 TimePhotoControl = 0;//定时拍照控制参数
static u32 DistancePhotoControl = 0;//定距拍照控制参数

static u32 TimePhotoSpace = 0;//定时拍照间隔
static u32 DistancePhotoSpace = 0;//定距拍照间隔
static u8  TimePhotoChannel = 0;//定时拍照通道
static u8  DistancePhotoChannel = 0;//定距拍照通道

/********************全局变量*************************/

/********************外部变量*************************/
extern CAMERA_STRUCT Camera[];

/********************本地函数声明*********************/
static u8 Photo_AlarmChange(void);
static u8 Photo_StatusChange(void);
static u8 Photo_TimeArive(void);
static u8 Photo_DistanceArive(void);
static u32 Photo_GetCurDistance(void);

/********************函数定义*************************/
/**************************************************************************
//函数名：Photo_TimeTask
//功能：实现事件触发拍照，参数定时、定距拍照功能
//输入：无
//输出：无
//返回值：始终为ENABLE
//备注：拍照定时任务，1秒调度1次，任务调度器需要调用此函数。
//拍照优先级从高到低分别为：平台下发指令拍照，事件触发拍照、定时参数拍照、
//定距参数拍照，只有在处理完高优先级的拍照请求后才会处理低优先级的拍照请求。
//通过调用Camera.c提供的拍照接口Camera_Photo实现事件触发拍照，参数定时、定距拍照功能
//拍照事件类型：0为平台下发指令（此文件用不到这种类型），1为定时，2为抢劫（紧急报警）；
//3为碰撞或侧翻，4为开门，5为关门，6为车门由开变关，时速由<20km/h到超过20km/h，
//7为定距，其它保留
***************************************************************************/
FunctionalState Photo_TimeTask(void)
{

	static u8 UpdatePramFlag = 0;

	if(0 == UpdatePramFlag)
	{
		UpdatePramFlag = 1;
		Photo_UpdatePram();
	}

	if(0 != Photo_AlarmChange())//报警位置位事件拍照
	{
		
		return ENABLE;
	}

	if(0 != Photo_StatusChange())//状态变化事件拍照
	{
		return ENABLE;
	}

	if(0 != Photo_TimeArive())//参数定时拍照
	{
		return ENABLE;
	}

	if(0 != Photo_DistanceArive())//参数定距拍照
	{
		return ENABLE;
	}

	return ENABLE;
}
/**************************************************************************
//函数名：Photo_UpdatePram
//功能：更新拍照参数
//输入：无
//输出：无
//返回值：无
//备注：在设置定时拍照控制，定距拍照控制参数时需要调用此函数
***************************************************************************/
void Photo_UpdatePram(void)
{
	u8 Buffer[5];
	u8 flag = 0;
	u32 temp;

	if(FRAM_PARAMETER_TIME_PHOTO_CHANNEL_LEN == FRAM_BufferRead(Buffer, 1, FRAM_PARAMETER_TIME_PHOTO_CHANNEL_ADDR))
	{
		TimePhotoChannel = Buffer[0];
	}

	if(FRAM_PARAMETER_DISTANCE_PHOTO_CHANNEL_LEN == FRAM_BufferRead(Buffer, 1, FRAM_PARAMETER_DISTANCE_PHOTO_CHANNEL_ADDR))
	{
		DistancePhotoChannel = Buffer[0];
	}

	if(E2_TIME_PHOTO_CONTROL_LEN == EepromPram_ReadPram(E2_TIME_PHOTO_CONTROL_ID,Buffer))
	{
		TimePhotoControl = Public_ConvertBufferToLong(Buffer);
		temp = TimePhotoControl;
		flag = (temp&(1 << 16)) >> 16;
		if(1 == flag)
		{
			TimePhotoSpace = ((temp&0xfffe0000) >> 17)*60;
		}
		else
		{
			TimePhotoSpace = (temp&0xfffe0000) >> 17;
		}
		if(TimePhotoSpace < 5)
		{
			TimePhotoSpace = 5;
		}
	}

	if(E2_MILE_PHOTO_CONTROL_LEN == EepromPram_ReadPram(E2_MILE_PHOTO_CONTROL_ID,Buffer))
	{
		DistancePhotoControl = Public_ConvertBufferToLong(Buffer);
		temp = DistancePhotoControl;
		flag = (temp&(1 << 16)) >> 16;
		if(1 == flag)
		{
			DistancePhotoSpace = ((temp&0xfffe0000) >> 17)*1000;
		}
		else
		{
			DistancePhotoSpace = (temp&0xfffe0000) >> 17;
		}
		if(DistancePhotoSpace < 100)
		{
			DistancePhotoSpace = 100;
		}
	}

	if(E2_ALARM_PHOTO_SWITCH_LEN == EepromPram_ReadPram(E2_ALARM_PHOTO_SWITCH_ID,Buffer))
	{
		AlarmPhotoSwitch = Public_ConvertBufferToLong(Buffer);
	}
	
	if(E2_ALARM_PHOTO_STORE_FLAG_LEN == EepromPram_ReadPram(E2_ALARM_PHOTO_STORE_FLAG_ID,Buffer))
	{
		AlarmPhotoStoreFlag = Public_ConvertBufferToLong(Buffer);
	}

	if(E2_STATUS_PHOTO_SWITCH_ID_LEN == EepromPram_ReadPram(E2_STATUS_PHOTO_SWITCH_ID,Buffer))
	{
		StatusPhotoSwitch = Public_ConvertBufferToLong(Buffer);
	}
	
	if(E2_STATUS_PHOTO_STORE_FLAG_ID_LEN == EepromPram_ReadPram(E2_STATUS_PHOTO_STORE_FLAG_ID,Buffer))
	{
		StatusPhotoStoreFlag = Public_ConvertBufferToLong(Buffer);
	}
	
}
/**************************************************************************
//函数名：Photo_AlarmChange
//功能：实现报警位置位事件拍照功能
//输入：无
//输出：无
//返回值：0表示不需要拍照，非0表示需要拍照，含义为事件拍照类型
//备注：拍照事件类型：0为平台下发指令（此文件用不到这种类型），1为定时，2为抢劫（紧急报警）；
//3为碰撞或侧翻，4为开门，5为关门，6为车门由开变关，时速由<20km/h到超过20km/h，
//7为定距，其它保留
***************************************************************************/
static u8 Photo_AlarmChange(void)
{
	u8 flag = 0;
	u32 Alarm = 0;
	u32 temp = 0;
	u8 bit = 0;
	u8 i = 0;
	u8 PhotoEnable = 0;
	u8 PhotoEventType = 0;
	u8 PhotoStoreFlag = 0;
	u8 PhotoNum = 0;

	Alarm = Io_ReadAlarm();

	if(Alarm != LastAlarm)
	{
		temp = LastAlarm^Alarm;
		temp = temp&Alarm;
		bit = 32;
		for(i=0; i<32; i++)
		{
			if((1 << i) == (temp&(1 << i)))
			{
				if((1 << i) == (AlarmPhotoSwitch&(1 << i)))
				{
					bit = i;
					break;
				}
			}
		}
		if(bit < 32)
		{
			switch(bit)
			{
				case ALARM_BIT_EMERGENCY://紧急报警由专门的处理流程，不在这处理
				{
					PhotoEnable = 0;
					break;
				}
				case ALARM_BIT_SIDE_TURN_PRE_ALARM:
				case ALARM_BIT_IMPACT_PRE_ALARM:
				{
					PhotoEventType = 3;
					PhotoStoreFlag = (AlarmPhotoStoreFlag&(1 << bit)) >> bit;
					PhotoNum = 1;
					PhotoEnable = 1;
					break;
				}
				default:
				{
					PhotoEventType = bit+224;
					PhotoStoreFlag = (AlarmPhotoStoreFlag&(1 << bit)) >> bit;
					PhotoNum = 1;
					PhotoEnable = 1;
					break;
				}
			}
		}
		if(1 == PhotoEnable)
		{
			flag = 1;
			PhotoStoreFlag = 3;//报警拍照都需保存和上传
			if(1 == Blind_GetLink2OpenFlag())
			{
				Camera_Photo(CHANNEL_DATA_1|CHANNEL_DATA_2, 0, PhotoNum, 10, 1, PhotoStoreFlag, 1, PhotoEventType);
			}
			else
			{
				Camera_Photo(CHANNEL_DATA_1, 0, PhotoNum, 10, 1, PhotoStoreFlag, 1, PhotoEventType);
			}
		}
	}	

	LastAlarm = Alarm;

	return flag;
}
/**************************************************************************
//函数名：Photo_StatusChange
//功能：实现状态变化事件拍照功能
//输入：无
//输出：无
//返回值：0表示不需要拍照，非0表示需要拍照，含义为事件拍照类型
//备注：拍照事件类型：0为平台下发指令（此文件用不到这种类型），1为定时，2为抢劫（紧急报警）；
//3为碰撞或侧翻，4为开门，5为关门，6为车门由开变关，时速由<20km/h到超过20km/h，
//7为定距，其它保留
***************************************************************************/
static u8 Photo_StatusChange(void)
{
	u8 flag = 0;
	u32 Status = 0;
	u32 temp = 0;
	u8 bit = 0;
	u8 i = 0;
	u8 PhotoEnable = 0;
	u8 PhotoEventType = 0;
	u8 PhotoStoreFlag = 0;
	u8 PhotoNum = 0;

	Status = Io_ReadStatus();

	if(Status != LastStatus)
	{
		temp = LastStatus^Status;
		temp = temp&Status;
		bit = 32;
		for(i=0; i<32; i++)
		{
			if((1 << i) == (temp&(1 << i)))
			{
				if((1 << i) == (StatusPhotoSwitch&(1 << i)))
				{
					bit = i;
					break;
				}
			}
		}
		if(bit < 32)
		{
			switch(bit)
			{
				case STATUS_BIT_NAVIGATION:
				case STATUS_BIT_SOUTH:
				case STATUS_BIT_EAST:
				case STATUS_BIT_ENCRYPT:
				{
					break;
				}
				case STATUS_BIT_DOOR1:
				case STATUS_BIT_DOOR2:
				case STATUS_BIT_DOOR3:
				case STATUS_BIT_DOOR4:
				case STATUS_BIT_DOOR5:
				{
					if((1 << bit) == (Status&(1 << bit)))
					{
						PhotoEventType = 4;
						PhotoStoreFlag = (StatusPhotoStoreFlag&(1 << bit)) >> bit;
						PhotoNum = 1;
						PhotoEnable = 1;
					}
					break;
				}
				case STATUS_BIT_FULL_LOAD2:
				{
					if((1 << bit) == (Status&(1 << bit)))
					{
						PhotoEventType = bit+192;
						PhotoStoreFlag = (StatusPhotoStoreFlag&(1 << bit)) >> bit;
						PhotoNum = 1;
						PhotoEnable = 1;
					}	
					break;
				}
				default:
				{
					PhotoEventType = bit+192;
					PhotoStoreFlag = (StatusPhotoStoreFlag&(1 << bit)) >> bit;
					PhotoNum = 1;
					PhotoEnable = 1;
					break;
				}
			}
		}
		if(1 == PhotoEnable)
		{
			flag = 1;
		
			PhotoStoreFlag = 3;//状态变化拍照都需保存和上传
			if(1 == Blind_GetLink2OpenFlag())
			{
				Camera_Photo(CHANNEL_DATA_1|CHANNEL_DATA_2, 0, PhotoNum, 10, 1, PhotoStoreFlag, 1, PhotoEventType);
			}
			else
			{
				Camera_Photo(CHANNEL_DATA_1, 0, PhotoNum, 10, 1, PhotoStoreFlag, 1, PhotoEventType);
			}
		}
	}	

	LastStatus = Status;

	return flag;
}
/**************************************************************************
//函数名：Photo_TimeArive
//功能：实现参数定时拍照功能
//输入：无
//输出：无
//返回值：0表示不需要拍照，非0表示需要拍照
//备注：拍照事件类型：0为平台下发指令（此文件用不到这种类型），1为定时，2为抢劫（紧急报警）；
//3为碰撞或侧翻，4为开门，5为关门，6为车门由开变关，时速由<20km/h到超过20km/h，
//7为定距，其它保留
***************************************************************************/
static u8 Photo_TimeArive(void)
{
	u8 flag = 0;
	u8 i = 0;
	u32 temp = 0;
	u8 PhotoStoreFlag = 0;
	static u32 TimeCount = 0;

	TimeCount++;
	if((TimeCount >= TimePhotoSpace)&&(TimePhotoSpace > 0))
	{
		TimeCount = 0;
		for(i=1; i<=CAMERA_ID_MAX; i++)
		{
			if((Camera[i-1].TimePhotoNum > 0)&&(1 == Camera[i-1].OnOffFlag))
			{
				return 1;
			}
		}
		temp = TimePhotoControl;
		for(i=1; i<=CAMERA_ID_MAX; i++)
		{
			if((1 << (i-1)) == (temp&(1 << (i-1))))
			{
				flag = 1;
				PhotoStoreFlag = (temp&(1 << (i-1+8))) >> (i-1+8);
				if(0 == PhotoStoreFlag)
				{
					PhotoStoreFlag = 0x02;
				}
				else
				{
					PhotoStoreFlag = 0x01;
				}
				Camera_Photo(TimePhotoChannel, i, 1, 10, 1, PhotoStoreFlag, 1, 1);
			}
		}
	}

	return flag;
}
/**************************************************************************
//函数名：Photo_DistanceArive
//功能：实现参数定距拍照功能
//输入：无
//输出：无
//返回值：0表示不需要拍照，非0表示需要拍照
//备注：拍照事件类型：0为平台下发指令（此文件用不到这种类型），1为定时，2为抢劫（紧急报警）；
//3为碰撞或侧翻，4为开门，5为关门，6为车门由开变关，时速由<20km/h到超过20km/h，
//7为定距，其它保留
***************************************************************************/
static u8 Photo_DistanceArive(void)
{
	u8 flag = 0;
	u8 i = 0;
	u32 temp = 0;
	u8 PhotoStoreFlag = 0;
	u32 CurDistance = 0;

	CurDistance = Photo_GetCurDistance();

	if((CurDistance >= NextPhotoDistance)&&(DistancePhotoSpace > 0))
	{
		NextPhotoDistance = CurDistance+ DistancePhotoSpace;

		temp = DistancePhotoControl;
		for(i=1; i<=CAMERA_ID_MAX; i++)
		{
			if((1 << (i-1)) == (temp&(1 << (i-1))))
			{
				flag = 1;
				PhotoStoreFlag = (temp&(1 << (i-1+8))) >> (i-1+8);
				if(0 == PhotoStoreFlag)
				{
					PhotoStoreFlag = 0x02;
				}
				else
				{
					PhotoStoreFlag = 0x01;
				}
				
				Camera_Photo(DistancePhotoChannel, i, 1, 10, 1, PhotoStoreFlag, 1, 7);
				
			}
		}
	}

	return flag;
}
/**************************************************************************
//函数名：Photo_GetCurDistance
//功能：获取当前的距离（累计行驶里程）
//输入：无
//输出：无
//返回值：距离（累计行驶里程）
//备注：单位米
***************************************************************************/
static u32 Photo_GetCurDistance(void)
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







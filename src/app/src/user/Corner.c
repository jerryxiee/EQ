/********************************************************************
//版权说明	:Shenzhen E-EYE Co. Ltd., Copyright 2009- All Rights Reserved.
//文件名称	:Corner.c		
//功能		:拐点补传
//版本号	:
//开发人	:dxl
//开发时间	:2014.7
//修改者	:
//修改时间	:
//修改简要说明	:
//备注		:ACC ON，行驶，当前时刻角度-上一时刻角度 > 设定值，上报1条位置信息
***********************************************************************/ 
/*************************文件包含***********************/
#include "include.h"

/*************************宏定义***********************/

/*************************外部变量***********************/

/*************************全局变量***********************/

static u32  CornerAngleMax = 0;//拐点角度,标准是4字节
/*************************本地变量***********************/

/*************************函数定义***********************/
/*********************************************************************
//函数名称	:Corner_UpdataPram
//功能		:拐点模块更新变量
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
//		:
*********************************************************************/
void Corner_UpdataPram(void)
{
    u8	TmpData[4] = {0};
	u8	PramLen = 0;

	//读特征系数
	PramLen = EepromPram_ReadPram(E2_CORNER_REPORT_ID, TmpData);
	if(4 != PramLen)
	{
        CornerAngleMax = 5;
    }
    else
    {
        CornerAngleMax = 0;
		CornerAngleMax |= (TmpData[0] << 24);
		CornerAngleMax |= (TmpData[1] << 16);
		CornerAngleMax |= TmpData[2] << 8;
        CornerAngleMax |= TmpData[3];
    }
}
/*********************************************************************
//函数名称	:Corner_TimeTask
//功能		:拐点补传定时任务
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:1秒调用1次
*********************************************************************/
FunctionalState  Corner_TimeTask(void)
{
    u32 CornerAngle = 0;
    static u32 LastCornerAngle = 0;
    u8  Acc = 0;
    u8  RunFlag = 0;
    u8  Navigation = 0;
    
    Acc = Io_ReadStatusBit(STATUS_BIT_ACC);
    Navigation = Io_ReadStatusBit(STATUS_BIT_NAVIGATION);
    RunFlag = SpeedMonitor_GetCurRunStatus();
    CornerAngle = Gps_ReadCourse();
    
    if((1 == Acc)&&(1 == Navigation)&&(1 == RunFlag))
    {
        if(CornerAngle >= LastCornerAngle)
        {
            if((CornerAngle - LastCornerAngle) >= 180)
            {
                if((360+LastCornerAngle - CornerAngle) >= CornerAngleMax)
                {
                    //SetEvTask(EV_REPORT); //位置上报
                    Report_UploadPositionInfo(CHANNEL_DATA_1);
                    Report_UploadPositionInfo(CHANNEL_DATA_2);
                }
            }
            else
            {
                if((CornerAngle - LastCornerAngle) >= CornerAngleMax)
                {
                    //SetEvTask(EV_REPORT); //位置上报
                    Report_UploadPositionInfo(CHANNEL_DATA_1);
                    Report_UploadPositionInfo(CHANNEL_DATA_2);
                }
            }
           
        }
        else
        {
            if((LastCornerAngle - CornerAngle) >= 180)
            {
                if((360+CornerAngle - LastCornerAngle) >= CornerAngleMax)
                {
                    //SetEvTask(EV_REPORT); //位置上报
                    Report_UploadPositionInfo(CHANNEL_DATA_1);
                    Report_UploadPositionInfo(CHANNEL_DATA_2);
                }
            }  
            else
            {
                if((LastCornerAngle - CornerAngle) >= CornerAngleMax)
                {
                    //SetEvTask(EV_REPORT); //位置上报
                    Report_UploadPositionInfo(CHANNEL_DATA_1);
                    Report_UploadPositionInfo(CHANNEL_DATA_2);
                }
            }
        }
    }
    LastCornerAngle = CornerAngle;
    
    return ENABLE;
}
/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : stm32f10x_dma.h
* Author             : MCD Application Team
* Version            : V2.0.3
* Date               : 09/22/2008
* Description        : This file contains all the functions prototypes for the
*                      DMA firmware library.
********************************************************************************

*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TASKSCHEDULE_H
#define __TASKSCHEDULE_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f2xx.h"
//*****************事件任务**************************
typedef enum 
{
    	EV_SEND_CAN_DATA = 0,           // 0:发送CAN数据
      	EV_CANREC,                      // 1:CAN总线数据处理
    	EV_GPS_PARSE,                   // 2:GPS数据处理
	    EV_NULL,                        // 3:空
	    EV_GET_LOGIN_PASSWORD,          // 4: 获取鉴权码任务
	    EV_LINK_GOV_SERVER,             // 5: 切换到政府监管平台
	    EV_FTP_UPDATA,                  // 6: FTP无线升级
	    EV_CALLING,                     // 7: 一键通话
	    EV_SEV_CALL,                    // 8: 服务器回拔电话功能
	    EV_RING,                        // 9:响铃事件号   
    	EV_RING_OVER,                   // 10:停止响铃
	    EV_SHOW_TEXT,			        // 11:文本显示
    	MAX_EVENTASK

}EVENTTASK;

//******************时间任务***********************
typedef enum 
{
	TIME_COMMUNICATION = 0,         //0:无线通信,时间调度基准为20ms
    	TIME_RADIO_PARSE,               //1:无线协议解析
    	TIME_IO_MONITOR,                //2:IO 状态检测 
    	TIME_GPSREC,                    //3:GPS数据处理 
    	TIME_GPS_MILE,                   //4:GPS里程计算
    	TIME_POSITION,    	        //5:位置汇报 
	TIME_TRACK_RECORD,               //6:记录历史轨迹,开始是关闭的,在时间同步后打开 
	TIME_CAN_SEND,                  //7:CAN接收任务 Wangj 2011.07.14
	TIME_CAMERA,               //8:摄像头拍照 
	TIME_SEND_MEDIA,                //9:传输多媒体数据到平台
	TIME_UART5,              	//10:串口5定时解析任务 
	TIME_SPEED_MONITOR,              //11:超速监控 
	TIME_TIRED_DRIVE,	        //12:疲劳驾驶,开始是关闭的,在时间同步后打开
    	TIME_DOUBT_POINT,                //13:疑点数据,0.2s调度一次
	TIME_HEART_BEAT ,                //14:心跳包
	TIME_USART3,       	        //15:USART3串口3(摄像头拍照使用)定时接收任务
	TIME_PRINT,                     //16:打印任务,100ms调度一次,打印任务已放在whle(1)中 
    	TIME_WIRELESS_RECORDER,         //17:无线采集记录仪数据
 	TIME_WIRED_RECORDER,            //18:有线采集记录仪数据 
  	TIME_RADIO_RESEND,              //19:无线协议重发（国标）
 	TIME_DELAY_TRIG,                //20:延时触发
    	TIME_SYSRESET,                  //21:系统重启
	TIME_BMA220,			//22:碰撞报警
	TIME_PULSE,			//23:脉冲速度
	TIME_LINK1_LOGIN,			//24:连接1注册鉴权
	TIME_USB,			//25:USB
	TIME_TTSDELAY,         		//26: TTS
	TIME_AD,			//27:AD采样
	TIME_SLEEP,			//28:休眠
	TIME_KEY_BOARD,                     //29:评价器
    	TIME_ADVERT_LED,                //30:无，之前是广告屏
    	TIME_SPEED_LOG,                  //31:速度异常记录
    	TIME_POWER_ON_LOG,              //32:供电记录
    	TIME_COM485RESEND,              //33:485数据重发
	TIME_RECORD,                    //34:录音程序
	TIME_MENU,                     //35:LCD集成显示屏显示
 	TIME_TAXIMETER,                 //36:空，之前是计价器
	TIME_M12TTS_PLAY,		//37:M12 TTS
	TIME_AREA,			//38:区域
	TIME_MEDIA,			//39:删除多媒体文件
	TIME_RECORD_TIMER,              //40:定时录音关闭任务 
	TIME_ROUTE,			//41:线路任务
	TIME_PULSE_ADJUST,		//42:特征系数校准
	TIME_CAMERA_UPLOAD,		//43:上传事件类照片
	TIME_REPORT_SMS,                //44:上报一条短信位置信息
	TIME_OIL_CTRL,          	//45:断油路 
	TIME_USART6,			// USART6定时收发任务 (用于IC卡通讯)
	TIME_USART6_RESEND,        	//47: USART6定时重发    (用于IC卡通讯)
	TIME_IC_CARD,              	//48: IC卡各类事务
	TIME_ICAUC,                     //49:IC卡认证TCP连接任务 
        TIME_NVLCD,                 	//50: 导航屏 
	TIME_LOAD_CONTROLLER_UPDATA , //51. 分包升级任务
        TIME_MULTI_CONNECT,             //52: 多中心连接,北斗测试时使用
        TIME_GNSS_REAL,		        //53：北斗实时解析数据
        TIME_GNSS_APP,			//54:北斗发送数据到平台任务
        TIME_ACCOUNT,                   //55:快速开户
        TIME_CLOSE_GPRS,                //56:GPRS下线
        TIME_SUBPACKAGE,           	//57:分包补传
	TIME_PHOTO,			//58:无张数限制的定时定距拍照任务,
	TIME_SLEEPDEEP,            	//59:深度休眠
        TIME_POLYGON,            	//63:多边形
        TIME_JOINTECHOILCOST,           //64: 深圳久通油耗传感器任务
        TIME_UPDATA_RESULT_NOTE,        //67:发送升级结果通知
        TIME_CARLOAD,                   //68:载重
        TIME_CORNER,                    //69:拐点补传
        TIME_PUBLIC,                    //70:公共
        TIME_LINK2_LOGIN,	        //71:连接2注册鉴权
        TIME_BLIND,                     //72:盲区补报
        TIME_STATUS_ALARM,              //73:状态变化或产生报警立即上报或拍照
        TIME_TEMPERATURE,               //74:温度采集器接入
        TIME_CAN_ERQI,                  //75:二汽CAN接入
        TIME_ACCELERATION,              //76:急加速，急减速 zengliang add
        
        MAX_TIMETASK,   
}TIMERTASK;
/*------------------------ Execute task by timer--------------------------------
Max task is 32. when power on dev this task is init.
Include 
------------------------------------------------------------------------------*/
typedef struct{
  FunctionalState TaskTimerState;       //task valid,1:valid;0:invalid
  u32 TaskTimerCount;   		//variables task time count
  u32 TaskExTimer;      		//set fixed param by independently funtion module
  FunctionalState (* operate)();     //handle independtly funtion
}TIME_TASK;

typedef struct{
  FunctionalState evTaskOnState;       //task valid,1:valid;0:invalid
  void (*operate)();     //handle independtly funtion
}EVEN_TASK;

/* Private define ------------------------------------------------------------*/



/*******************************************************************************
* Function Name  : InitTimerTask
* Description    : Initialize Schedule Timer task .
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void InitTimerTask(void);

/*******************************************************************************
* Function Name  : TimerTaskScheduler
* Description    : Schedule Timer task .
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TimerTaskScheduler(void);

/*******************************************************************************
* Function Name  : SetTimerTask
* Description    : Setup timer task .
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SetTimerTask(TIMERTASK task, u32 time);

/*******************************************************************************
* Function Name  : ClrTimerTask
* Description    : Cancel timer task .
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ClrTimerTask(TIMERTASK task);

/*******************************************************************************
* Function Name  : InitEvTask
* Description    : Initialize Schedule Timer task .
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void InitEvTask(void);
/*******************************************************************************
* Function Name  : SetEvTask
* Description    : Setup even task .
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SetEvTask(u8 ev);
/*******************************************************************************
* Function Name  : EvTaskScheduler
* Description    : Schedule even task .
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EvTaskScheduler(void);
/*******************************************************************************
* Function Name  : SendCanData_EvTask
* Description    : 发送CAN数据
* Input          : None
* Output         : None
* Return         : None
* Note           :需要发送的CAN数据放在RadioProtocolMiddleBuffer,长度为RadioProtocolMiddleLen
*******************************************************************************/
void SendCanData_EvTask(void);
#endif

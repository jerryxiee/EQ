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
//*****************�¼�����**************************
typedef enum 
{
    	EV_SEND_CAN_DATA = 0,           // 0:����CAN����
      	EV_CANREC,                      // 1:CAN�������ݴ���
    	EV_GPS_PARSE,                   // 2:GPS���ݴ���
	    EV_NULL,                        // 3:��
	    EV_GET_LOGIN_PASSWORD,          // 4: ��ȡ��Ȩ������
	    EV_LINK_GOV_SERVER,             // 5: �л����������ƽ̨
	    EV_FTP_UPDATA,                  // 6: FTP��������
	    EV_CALLING,                     // 7: һ��ͨ��
	    EV_SEV_CALL,                    // 8: �������ذε绰����
	    EV_RING,                        // 9:�����¼���   
    	EV_RING_OVER,                   // 10:ֹͣ����
	    EV_SHOW_TEXT,			        // 11:�ı���ʾ
    	MAX_EVENTASK

}EVENTTASK;

//******************ʱ������***********************
typedef enum 
{
	TIME_COMMUNICATION = 0,         //0:����ͨ��,ʱ����Ȼ�׼Ϊ20ms
    	TIME_RADIO_PARSE,               //1:����Э�����
    	TIME_IO_MONITOR,                //2:IO ״̬��� 
    	TIME_GPSREC,                    //3:GPS���ݴ��� 
    	TIME_GPS_MILE,                   //4:GPS��̼���
    	TIME_POSITION,    	        //5:λ�û㱨 
	TIME_TRACK_RECORD,               //6:��¼��ʷ�켣,��ʼ�ǹرյ�,��ʱ��ͬ����� 
	TIME_CAN_SEND,                  //7:CAN�������� Wangj 2011.07.14
	TIME_CAMERA,               //8:����ͷ���� 
	TIME_SEND_MEDIA,                //9:�����ý�����ݵ�ƽ̨
	TIME_UART5,              	//10:����5��ʱ�������� 
	TIME_SPEED_MONITOR,              //11:���ټ�� 
	TIME_TIRED_DRIVE,	        //12:ƣ�ͼ�ʻ,��ʼ�ǹرյ�,��ʱ��ͬ�����
    	TIME_DOUBT_POINT,                //13:�ɵ�����,0.2s����һ��
	TIME_HEART_BEAT ,                //14:������
	TIME_USART3,       	        //15:USART3����3(����ͷ����ʹ��)��ʱ��������
	TIME_PRINT,                     //16:��ӡ����,100ms����һ��,��ӡ�����ѷ���whle(1)�� 
    	TIME_WIRELESS_RECORDER,         //17:���߲ɼ���¼������
 	TIME_WIRED_RECORDER,            //18:���߲ɼ���¼������ 
  	TIME_RADIO_RESEND,              //19:����Э���ط������꣩
 	TIME_DELAY_TRIG,                //20:��ʱ����
    	TIME_SYSRESET,                  //21:ϵͳ����
	TIME_BMA220,			//22:��ײ����
	TIME_PULSE,			//23:�����ٶ�
	TIME_LINK1_LOGIN,			//24:����1ע���Ȩ
	TIME_USB,			//25:USB
	TIME_TTSDELAY,         		//26: TTS
	TIME_AD,			//27:AD����
	TIME_SLEEP,			//28:����
	TIME_KEY_BOARD,                     //29:������
    	TIME_ADVERT_LED,                //30:�ޣ�֮ǰ�ǹ����
    	TIME_SPEED_LOG,                  //31:�ٶ��쳣��¼
    	TIME_POWER_ON_LOG,              //32:�����¼
    	TIME_COM485RESEND,              //33:485�����ط�
	TIME_RECORD,                    //34:¼������
	TIME_MENU,                     //35:LCD������ʾ����ʾ
 	TIME_TAXIMETER,                 //36:�գ�֮ǰ�ǼƼ���
	TIME_M12TTS_PLAY,		//37:M12 TTS
	TIME_AREA,			//38:����
	TIME_MEDIA,			//39:ɾ����ý���ļ�
	TIME_RECORD_TIMER,              //40:��ʱ¼���ر����� 
	TIME_ROUTE,			//41:��·����
	TIME_PULSE_ADJUST,		//42:����ϵ��У׼
	TIME_CAMERA_UPLOAD,		//43:�ϴ��¼�����Ƭ
	TIME_REPORT_SMS,                //44:�ϱ�һ������λ����Ϣ
	TIME_OIL_CTRL,          	//45:����· 
	TIME_USART6,			// USART6��ʱ�շ����� (����IC��ͨѶ)
	TIME_USART6_RESEND,        	//47: USART6��ʱ�ط�    (����IC��ͨѶ)
	TIME_IC_CARD,              	//48: IC����������
	TIME_ICAUC,                     //49:IC����֤TCP�������� 
        TIME_NVLCD,                 	//50: ������ 
	TIME_LOAD_CONTROLLER_UPDATA , //51. �ְ���������
        TIME_MULTI_CONNECT,             //52: ����������,��������ʱʹ��
        TIME_GNSS_REAL,		        //53������ʵʱ��������
        TIME_GNSS_APP,			//54:�����������ݵ�ƽ̨����
        TIME_ACCOUNT,                   //55:���ٿ���
        TIME_CLOSE_GPRS,                //56:GPRS����
        TIME_SUBPACKAGE,           	//57:�ְ�����
	TIME_PHOTO,			//58:���������ƵĶ�ʱ������������,
	TIME_SLEEPDEEP,            	//59:�������
        TIME_POLYGON,            	//63:�����
        TIME_JOINTECHOILCOST,           //64: ���ھ�ͨ�ͺĴ���������
        TIME_UPDATA_RESULT_NOTE,        //67:�����������֪ͨ
        TIME_CARLOAD,                   //68:����
        TIME_CORNER,                    //69:�յ㲹��
        TIME_PUBLIC,                    //70:����
        TIME_LINK2_LOGIN,	        //71:����2ע���Ȩ
        TIME_BLIND,                     //72:ä������
        TIME_STATUS_ALARM,              //73:״̬�仯��������������ϱ�������
        TIME_TEMPERATURE,               //74:�¶Ȳɼ�������
        TIME_CAN_ERQI,                  //75:����CAN����
        TIME_ACCELERATION,              //76:�����٣������� zengliang add
        
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
* Description    : ����CAN����
* Input          : None
* Output         : None
* Return         : None
* Note           :��Ҫ���͵�CAN���ݷ���RadioProtocolMiddleBuffer,����ΪRadioProtocolMiddleLen
*******************************************************************************/
void SendCanData_EvTask(void);
#endif

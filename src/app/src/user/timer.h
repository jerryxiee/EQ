/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : stm32f2xx_dma.h
* Author             : MCD Application Team
* Version            : V2.0.3
* Date               : 09/22/2008
* Description        : This file contains all the functions prototypes for the
*                      DMA firmware library.
********************************************************************************

*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TIMER_H
#define __TIMER_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f2xx.h"
#include "stm32f2xx_conf.h"
#include "systick.h"

#define MINUTE 	(SECOND*60)
/*******************************************************************************
* Function Name  : HardFaultException
* Description    : This function handles Hard Fault exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
u32 Timer_Val();
/*******************************************************************************
* Function Name  : Timer_ISR
* Description    : This function .
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Timer_ISR();
/*******************************************************************************
* Function Name  : Timer_GetClock
* Description    : This function .
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Timer_GetClock(u8 *clock);

/*******************************************************************************
* Function Name  : Timer_SetClock
* Description    : This function .
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Timer_SetClock(u8 *clock);


#define TIMR_ON         		( 1 )
#define TIMR_OFF        		( 0 )
#define TIMEER_1S_ENABLE       	( 1 )
#define TIMEER_10MS_ENABLE     	( 1 )

#define TIMER_WORDTYPE  		u16
#define TIMER_MODE_ONCE         0
#define TIMER_MODE_CONTINUE     1
typedef  ErrorStatus	(*PEFUNCTION)(void);
typedef  struct         
{
  u8 number:5;               //��ʱ�����
  u8 msg:1;
  u8 timeswitch:1;           //��ʱ������
  u8 timerMode:1;            //��ʱ��ģʽ   0:once   1:continue
  u8 timerPara;              //��ʱ������
  TIMER_WORDTYPE timeLen;       //��ʱ����
  TIMER_WORDTYPE timeCount;     //��ʱ����
  PEFUNCTION callback;          //�ص�����
}STR_TIMER;

typedef enum
{
    TIMER_1S_ID_gps = 0,
    TIMER_1S_ID_1,  
    TIMER_1S_ID_2,
    TIMER_1S_ID_3,
    TIMER_1S_ID_4,
    TIMER_1S_ID_5, 
    MAX_TIMENUM_1S
}TIMER_1S_ID;

typedef enum
{
    TIMER_10MS_ID_gps = 0,
    TIMER_10MS_ID_1,
    TIMER_10MS_ID_2,
    TIMER_10MS_ID_3,
    TIMER_10MS_ID_4,
    TIMER_10MS_ID_5,
    MAX_TIMENUM_10MS,
}TIMER_10MS_ID;

#define	PRESCALER_VAL		(8 - 1)		//Ԥ��ƵֵΪ15��Ƶ��Ϊ48/8=6M
#define T2CCR_VAL_10MS  	(60000)      //6MƵ����10ms��Ҫ��ʱ����


#if TIMEER_10MS_ENABLE
    void TimerManage10ms(void);
		void TimerManage10ms_Callback(void);
    u8 Timer10msStart(u8 timeNumber, TIMER_WORDTYPE timeLen, u8 mode, PEFUNCTION callBack);    
    u8 CloseTime10ms(u8 timeNumber);
    #define StartTime10ms(id, len, cb)      Timer10msStart((id), (len), TIMER_MODE_CONTINUE, (cb))
#endif

#if TIMEER_1S_ENABLE 
    void TimerManage1s(void);
		void TimerManage1s_Callback(void);
    u8 Timer1sStart(u8 timeNumber, TIMER_WORDTYPE timeLen, u8 mode, PEFUNCTION callBack);  
    u8 CloseTime1s(u8 timeNumber);
		#define StartTime1s(id, len, cb)      Timer1sStart((id), (len), TIMER_MODE_CONTINUE, (cb))
#endif  
void TimerInit(void);



#endif

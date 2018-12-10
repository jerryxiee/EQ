/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : timer.c
* Author             : MCD Application Team 
* Version            : V1.0.0
* Date               : 04/07/2009
* Description        : 定时器程序.
*                      This file provides template for all exceptions handler
*                      and peripherals interrupt service routine.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "include.h"
//*******************外部变量************************
extern u8 UART4_TxBuffer[];
extern u8 UART4_TxBuffer2[];
extern TIME_T CurrentTime;

//*******************全局变量************************
u32  TimerCount = 0;
u8 SecTimer;
u8 MyClock[6];
const u8 Month[] = {31,28,31,30,31,30,31,31,30,31,30,31};

//*******************函数声明*************************
void Ajust_Clock();

/*******************************************************************************
* Function Name  : Timer_Val
* Description    : This function .
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
u32 Timer_Val()
{
  return TimerCount; 
}

/*******************************************************************************
* Function Name  : Timer_ISR
* Description    : This function .
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Timer_ISR()
{
	TimerCount++;
    	communication_CalTimer();
}

/*******************************************************************************
* Function Name  : Timer_SetClock
* Description    : This function .
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Timer_SetClock(u8 *clock)
{
  u8 ch;
  
  ch = *clock++;
  MyClock[0] = ch & 0x3F;
  memcpy((u8*)&MyClock[1], clock, 5);
}

/*******************************************************************************
* Function Name  : Timer_GetClock
* Description    : This function .
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Timer_GetClock(u8 *clock)
{
  *clock &= 0xC0;
  *clock++ |= MyClock[0] & 0x3F;
  memcpy(clock, (u8*)&MyClock[1], 5);
}

/*******************************************************************************
* Function Name  : Ajust_Clock
* Description    : 内部时钟 .
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Ajust_Clock()
{
  u8 year,date;
	
	//秒
	MyClock[5]++;
	if(MyClock[5] > 59)
	{
		MyClock[5] = 0;//%= 60;
		MyClock[4]++;
		//分
		if(MyClock[4] > 59)
		{
			MyClock[4] = 0;//%= 60;
			MyClock[3]++;
			//时
			if(MyClock[3] > 23)
			{
				MyClock[3] = 0;//24;
				//日
				MyClock[2]++;
				date = Month[MyClock[1] - 1];
				//判断润年,判断是否2月份
				if(MyClock[1] == 2)
				{
					year = MyClock[0] & 0x03;
					if(year == 0)
						date++;// = 29;
				}
				
				if(MyClock[2] > date)
				{
					MyClock[2] = 1;
					//月	
					MyClock[1]++;
					if(MyClock[1] > 12)
					{
						MyClock[1] = 1;
						//年
						MyClock[0]++;
					}
				}
			}
		}
	}
}

/*******************************************************************************
模块名       : 软件定时器管理
文件名       : 
相关文件     : 
文件实现功能 : 
作者         : 张文波
版本         : Ver 1.0。
-------------------------------------------------------------------------------
备注         : 
-------------------------------------------------------------------------------
修改记录 : 
日 期        版本     修改人              修改内容
YYYY/MM/DD   X.Y      <作者或修改者名>    <修改内容>
===============================================================================
其他说明:   无。
*******************************************************************************/

static STR_TIMER gTimeInf10ms[MAX_TIMENUM_10MS];
static STR_TIMER gTimeInf1s[MAX_TIMENUM_1S];

/******************************************************************************
函数名      : InitTimer1(void)
函数实现功能: 初始化TIM1
输入        : 无
返回        : 无
作者		: 张文波
描述        : 10ms定时器
*******************************************************************************/
void InitTimer1(void)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_OCInitTypeDef  TIM_OCInitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;

	/* ---------------------------------------------------------------
	TIM2 Configuration: Output Compare Timing Mode:
	TIM2 counter clock = 48/8 = 6 MHz
	CC1 update rate = TIM2 counter clock / CCR1_Val = 1000 Hz
	--------------------------------------------------------------- */
  //T2CCR1_Val = T2CCR_VAL_10MS;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	
	TIM_TimeBaseStructure.TIM_Period = 65535;
	//将TIM2的时钟设为100K
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0x0000;
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
	
	TIM_PrescalerConfig(TIM1,PRESCALER_VAL,TIM_PSCReloadMode_Immediate);
	
	TIM_OCStructInit(&TIM_OCInitStructure);
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = T2CCR_VAL_10MS;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC1Init(TIM1, &TIM_OCInitStructure);
	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Disable);
  
  TIM_ITConfig(TIM1, TIM_IT_CC1, ENABLE);	
	TIM_Cmd(TIM1, ENABLE); 
  
  NVIC_InitStructure.NVIC_IRQChannel = TIM1_CC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  
  NVIC_Init(&NVIC_InitStructure); 
}
/******************************************************************************
函数名      : InitTimer5(void)
函数实现功能: 初始化TIM5
输入        : 无
返回        : 无
作者		: 张文波
描述        : 1s定时器
*******************************************************************************/
void InitTimer5(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
  
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
	TIM_TimeBaseStructure.TIM_Period = 10000;
	TIM_TimeBaseStructure.TIM_Prescaler = 48000000/10000ul-1;
	
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0x0000;
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);
	
	TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
	TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM5, ENABLE); 
  
	NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;	
	NVIC_Init(&NVIC_InitStructure); 
}

/******************************************************************************
函数名      : InitTimer7(void)
函数实现功能: 初始化TIM7
输入        : 无
返回        : 无
作者		: 张文波
描述        : 10ms定时器
*******************************************************************************/
void InitTimer7(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);
	/* ---------------------------------------------------------------
	TIM2 Configuration: Output Compare Timing Mode:
	TIM2 counter clock = 100 KHz
	CC1 update rate = TIM2 counter clock / CCR1_Val = 100 Hz
	--------------------------------------------------------------- */
	TIM_TimeBaseStructure.TIM_Period = 1000;
	//将TIM2的时钟设为100K
	TIM_TimeBaseStructure.TIM_Prescaler = 48000000/100000ul-1;
	
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0x0000;
	TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure);
	
	TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
  TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM7, ENABLE); 
  
  NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
  NVIC_Init(&NVIC_InitStructure); 
}
/******************************************************************************
函数名      : TimerInit(void)
函数实现功能: 初始化定时器
输入        : 无
返回        : 无
作者		: 张文波
描述        : 
*******************************************************************************/
void TimerInit(void)
{
	//InitTimer1();
	InitTimer5();
	InitTimer7();
#if TIMEER_10MS_ENABLE  
  memset(gTimeInf10ms,0,sizeof(gTimeInf10ms));
#endif	
#if TIMEER_1S_ENABLE  
  memset(gTimeInf1s,0,sizeof(gTimeInf1s));
#endif
}

#if TIMEER_10MS_ENABLE 
u8 CloseTime10ms(u8 timeNumber);
u8 RestartTime10ms(u8 timeNumber);
void TimerManage10ms(void);
/******************************************************************************
函数名		: Timer10msStart(u8 timeNumber, TIMER_WORDTYPE timeLen, u8 mode, PEFUNCTION callBack) 
函数实现功能: 开起基数为10MS的定时函数
输入		: timeNumber：定时器号  
TimeLen：定时时间长度  
mode:	模式选择  TIMER_MODE_ONCE:单次 TIMER_MODE_CONTINUE:循环
CallBack：回调函数
返回		: 0:成功1:失败
作者		: 张文波
描述		: 
*******************************************************************************/
u8 Timer10msStart(u8 timeNumber, TIMER_WORDTYPE timeLen, u8 mode, PEFUNCTION callBack)    
{
  if((timeNumber < MAX_TIMENUM_10MS)&&(timeLen > 0))
  {
    gTimeInf10ms[timeNumber].number = timeNumber;
    gTimeInf10ms[timeNumber].timeLen = timeLen;
    gTimeInf10ms[timeNumber].timeCount = 0;
    gTimeInf10ms[timeNumber].timerMode = mode;
    gTimeInf10ms[timeNumber].callback = callBack;
    gTimeInf10ms[timeNumber].timeswitch = TIMR_ON;
    return(0);
  }  
  return(1);
}
/*************************************************************
函数名: RestartTime10ms(u8 timeNumber)
函数实现功能:重置基数为10MS的定时函数
输入：定时器号
返回: 0：成功;1：失败
作者		: 张文波
描述		: 
**************************************************************/
u8 RestartTime10ms(u8 timeNumber)
{
  if(timeNumber < MAX_TIMENUM_10MS)
  {   
    gTimeInf10ms[timeNumber].timeswitch = TIMR_ON;
    gTimeInf10ms[timeNumber].timeCount = 0;
    return(0);
  }
  return(1);
}
/*************************************************************
函数名: CloseTime10ms(u8 timeNumber)
函数实现功能:关闭基数为10MS的定时函数
输入：定时器号
返回: 0：成功;1：失败
作者		: 张文波
描述		: 
**************************************************************/
u8 CloseTime10ms(u8 timeNumber)
{
  if(timeNumber < MAX_TIMENUM_10MS)
  {   
    gTimeInf10ms[timeNumber].timeswitch = TIMR_OFF;
    return(0);
  }
  return(1);
}
/*************************************************************
函数名: TimerManage10ms(void)
函数实现功能:管理基数为10MS的定时函数
输入：
返回: 
作者		: 张文波
描述		: 
**************************************************************/
void TimerManage10ms(void)
{
  u8 i;
  ErrorStatus err = ERROR;
  for(i=0;i<MAX_TIMENUM_10MS;i++)
  {
    if(gTimeInf10ms[i].timeswitch == TIMR_ON)
		{
      gTimeInf10ms[i].timeCount++;
      if(gTimeInf10ms[i].timeCount >= gTimeInf10ms[i].timeLen)
      {
        gTimeInf10ms[i].timeCount = 0;
				gTimeInf10ms[i].msg = 1;
        if(gTimeInf10ms[i].callback != NULL)
        {
          // TODO: Need check to protect more
          err = gTimeInf10ms[i].callback();
        }
        if((gTimeInf10ms[i].timerMode == TIMER_MODE_ONCE)&&(SUCCESS == err))
        {
        	gTimeInf10ms[i].timeswitch = TIMR_OFF;
        }
      }           
    }
  }
}
void TimerManage10ms_Callback(void)
{
	u8 i;
  ErrorStatus	err = ERROR;
  for(i=0;i<MAX_TIMENUM_10MS;i++)
  {
    if(gTimeInf10ms[i].msg)
    {
			gTimeInf10ms[i].msg = 0;
      if(gTimeInf10ms[i].callback != NULL)
      {
        // TODO: Need check to protect more
        err = gTimeInf10ms[i].callback();
      }
      if((gTimeInf10ms[i].timerMode == TIMER_MODE_ONCE)&&(SUCCESS == err))
      {
      	gTimeInf10ms[i].timeswitch = TIMR_OFF;
      }
    }           
  }
}

#endif

#if TIMEER_1S_ENABLE
u8 CloseTime1s(u8 timeNumber);
void TimerManage1s(void);
/******************************************************************************
函数名		: Timer1sStart(u8 timeNumber, TIMER_WORDTYPE timeLen, u8 mode, PEFUNCTION callBack) 
函数实现功能: 开起基数为1S的定时函数
输入		: timeNumber：定时器号  
TimeLen：定时时间长度  
mode:	模式选择  TIMER_MODE_ONCE:单次 TIMER_MODE_CONTINUE:循环
CallBack：回调函数
返回		: 0:成功1:失败
作者		: 张文波
描述		: 
*******************************************************************************/
u8 Timer1sStart(u8 timeNumber, TIMER_WORDTYPE timeLen, u8 mode, PEFUNCTION callBack)  
{
  if((timeNumber < MAX_TIMENUM_1S)&&(timeLen > 0))
  {
    gTimeInf1s[timeNumber].number = timeNumber;
    gTimeInf1s[timeNumber].timeLen = timeLen;
    gTimeInf1s[timeNumber].timeCount = 0;
    gTimeInf1s[timeNumber].timerMode = mode;
    gTimeInf1s[timeNumber].callback = callBack;
    gTimeInf1s[timeNumber].timeswitch = TIMR_ON;
    return(0);
  }
  return(1);
}

/*************************************************************
函数名: CloseTime1s(u8 timeNumber)
函数实现功能:关闭基数为1S的定时函数
输入：定时器号
返回: 0：成功;1：失败
作者		: 张文波
描述		: 
**************************************************************/
u8 CloseTime1s(u8 timeNumber)
{
  if(timeNumber < MAX_TIMENUM_1S)
  {   
    gTimeInf1s[timeNumber].timeswitch = TIMR_OFF;
    return(0);
  }
  return(1);
}
/*************************************************************
函数名: TimerManage1s(void)
函数实现功能:管理基数为1S的定时函数
输入：
返回: 
作者		: 张文波
描述		: 
**************************************************************/
void TimerManage1s(void)
{
  u8 i;
  ErrorStatus	err = ERROR;
  for(i=0;i<MAX_TIMENUM_1S;i++)
  {
    if(gTimeInf1s[i].timeswitch == TIMR_ON)
    {
      gTimeInf1s[i].timeCount++;
      if(gTimeInf1s[i].timeCount >= gTimeInf1s[i].timeLen)
      {
        gTimeInf1s[i].timeCount = 0;
				gTimeInf1s[i].msg = 1;
        if(gTimeInf1s[i].callback != NULL)
        {
          // TODO: Need check to protect more
          err = gTimeInf1s[i].callback();
        }
        if((gTimeInf1s[i].timerMode == TIMER_MODE_ONCE)&&(SUCCESS == err))
        {
        	gTimeInf1s[i].timeswitch = TIMR_OFF;
        }
      }           
    }
  }
}

void TimerManage1s_Callback(void)
{
	u8 i;
  ErrorStatus	err = ERROR;
  for(i=0;i<MAX_TIMENUM_1S;i++)
  {
    if(gTimeInf1s[i].msg)
    {
			gTimeInf1s[i].msg = 0;
      if(gTimeInf1s[i].callback != NULL)
      {
        // TODO: Need check to protect more
        err = gTimeInf1s[i].callback();
      }
      if((gTimeInf1s[i].timerMode == TIMER_MODE_ONCE)&&(SUCCESS == err))
      {
      	gTimeInf1s[i].timeswitch = TIMR_OFF;
      }
    }           
  }
}
#endif



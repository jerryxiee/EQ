/**
  ******************************************************************************
  * @file    SysTickApp.h
  * @author  
  * @version 
  * @date    
  * @brief   SySTick的应用，延迟函数和计时函数
  *          1.需要初始化调用SysTick_Config(SystemCoreClock / 1000);函数,1ms一次中断
  *          2.在sysTick中断中调用TimingDelay_Decrement()函数
  *
  *               
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SYSTICKAPP_H
#define __SYSTICKAPP_H

#ifdef __cplusplus
 extern "C" {
#endif
#include "stm32f2xx.h"

//***************以下宏定义是在96M滴答时钟下定义的***************
//#define	SYSTICK_1ms	1               //1ms
//#define SYSTICK_10ms	10              //10ms
//#define SYSTICK_20ms	20              //20ms
#define	SYSTICK_50ms	1              //50ms
#define	SYSTICK_100ms	2             //100ms
#define SYSTICK_0p1SECOND	2	//100ms

#define SYSTICK_1SECOND	20	////1s

#define SYSTICK_10SECOND	(10*SYSTICK_1SECOND)	//10s

#define SYSTICK_1MINUTE		(60*SYSTICK_1SECOND)	//1分钟

#define SYSTICK_10MINUTE	(600*SYSTICK_1SECOND)	//10分钟

#define SYSTICK_1HOUR		(3600*SYSTICK_1SECOND)	//1小时

#define	SECOND	SYSTICK_1SECOND
#define MINUTE 	(SECOND*60)
   
void Systick_Time_IRQ(void);

void TimingCurrent_Insr(void);
uint32_t Timer_Val(void);

void TimingDelay_Decrement(void);
void Delay(__IO uint32_t nTime);
   
void StartTimingCount(void);
uint32_t StopTimingCount(void);
void TimingCount_Insr(void);

void Timer_Delay_ms(u32 TimeOut);
void Timer_Delay_sec(u32 TimeOut);
void SpaceTime_Start(u32 *currTime);
u8 SpaceTime_Ms(u32 currTime,u32 TimeOut);
u8 SpaceTime_Sec(u32 currTime,u32 TimeOut);

#ifdef __cplusplus
}
#endif

#endif /* __SYSTICKAPP_H */
/**
  ******************************************************************************
  * @file    SysTickApp.h 
  * @author  
  * @version 
  * @date    
  * @brief   SySTick的应用，延迟函数和计时函数
  *
  *               
  ******************************************************************************
  */
#include "include.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static __IO uint32_t TimingDelay = 0;
static __IO uint32_t TimingCount = 0;    //计时程序
static __IO uint32_t TimingCurrent = 0;  //系统持续运行的时间累计(ms),可计算1193个小时数据
__IO uint8_t TimingCountFlag = 0;        //开始计时标志

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  系统定时器中断调用
  * @param  None
  * @retval None
  */
void Systick_Time_IRQ(void)
{
//  TimingCurrent_Insr();
  TimingCurrent++;                               //优化，省函数调用时间
  TimingDelay_Decrement();
  if(TimingCountFlag)
  {
    TimingCount_Insr();
  }

  communication_CalTimer();                   //计算 无线通讯 模块使用的超时应用
}

/**
  * @brief  系统运行时间统计,递增变量TimingCurrent.
  * @param  None
  * @retval None
  */
void TimingCurrent_Insr(void)
{
   TimingCurrent++;
}

/**
  * @brief  获取系统当前运行时间
  * @param  None
  * @retval None
  */
/*
uint32_t Timer_Val(void)
{
   return TimingCurrent;
}
*/

/**
  * @brief  插入一个延迟时间。
  * @param  nTime: 指定的延迟时间的长度，以毫秒为单位。
  * @retval None
  */
void Delay(__IO uint32_t nTime)
{ 
  TimingDelay = nTime;

  while(TimingDelay != 0);
}

/**
  * @brief  SysTick 中断处理程序,递减TimingDelay变量。
  * @param  None
  * @retval None
  */
void TimingDelay_Decrement(void)
{
  if (TimingDelay != 0x00)
  { 
    TimingDelay--;
  }
}
/**
  * @brief  开始计时。
  * @param  None
  * @retval None
  */
void StartTimingCount(void)
{
  TimingCount = 0;
  TimingCountFlag = 1;
}
/**
  * @brief  停止计时。
  * @param  None
  * @retval 返回累计毫秒数
  */
uint32_t StopTimingCount(void)
{
  TimingCountFlag = 0;
  return TimingCount;
}

/**
  * @brief  SysTick 中断处理程序,递增TimingCount变量。
  * @nots   TimingCount最大值为0xFFFFFFFF,所以计时的最大值为1193小时.
  * @param  None
  * @retval None
  */
void TimingCount_Insr(void)
{
  if (TimingCount != 0xFFFFFFFF)
  { 
    TimingCount++;
  }
}

/**
  * @brief  时间硬延迟 
  * @param  TimeOut:ms间隔
  * @retval None
  */
void Timer_Delay_ms(u32 TimeOut)
{
  u32 tmp = 0;
  tmp = TimingCurrent;
  while((TimingCurrent - tmp) < TimeOut)
  {
  }
}

/**
  * @brief  时间硬延迟 
  * @param  TimeOut:s间隔
  * @retval None
  */
void Timer_Delay_sec(u32 TimeOut)
{
  u32 tmp = 0;
  tmp = TimingCurrent;
  while((TimingCurrent - tmp) < (TimeOut*SYSTICK_1SECOND))
  {
  }
}

/**
  * @brief  启动间隔计算计算 
  * @nots   x最大值为0xFFFFFFFF,所以延迟的最大值为1193小时.
  * @param  None
  * @retval None
  */
void SpaceTime_Start(u32 *currTime)
{
  *currTime = TimingCurrent;
}

/**
  * @brief  计算延迟间隔 
  * @param  currTime:计算的开始时间
  * @param  TimeOut:计算间隔，单位毫秒
  * @retval 超过间隔返回1，否则返回0
  */
u8 SpaceTime_Ms(u32 currTime,u32 TimeOut)
{
  if((TimingCurrent - currTime) > TimeOut)
  {
    return 1;
  }else{
    return 0;
  }
}

/**
  * @brief  计算延迟间隔 
  * @param  currTime:计算的开始时间
  * @param  TimeOut:计算间隔，单位秒
  * @retval 超过间隔返回1，否则返回0
  */
u8 SpaceTime_Sec(u32 currTime,u32 TimeOut)
{
  if((TimingCurrent - currTime) > (TimeOut*SYSTICK_1SECOND))
  {
    return 1;
  }else{
    return 0;
  }
}
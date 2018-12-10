#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "ptr_stepmotor_ctrl.h"
#include "ptr_timer.h" 

u32 __20us; //时间单位是10微妙
u32 __1ms; //时间单位是10微妙
u32 __50ms;
static u32 __cnt;
static u32 __50mscnt;


/*************************************************************
** 定时器初始化
**************************************************************/
void ptr_timer_cfg(void)
{
  	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	
	NVIC_InitStructure.NVIC_IRQChannel = PTR_CTRL_TIMER_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  	NVIC_Init(&NVIC_InitStructure);
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PTR_CTRL_TIMER, ENABLE);
	
	TIM_DeInit(PTR_CTRL_TIMER);
	
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);

  	TIM_TimeBaseStructure.TIM_Period = 65535;
  	TIM_TimeBaseStructure.TIM_Prescaler = 0;
  	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  	TIM_TimeBaseInit(PTR_CTRL_TIMER, &TIM_TimeBaseStructure);

  	TIM_PrescalerConfig(PTR_CTRL_TIMER, 95, TIM_PSCReloadMode_Immediate); //系统时钟是96MHz，经过(95+1)分频后，得到1MHz计数时钟

	TIM_SetAutoreload(PTR_CTRL_TIMER, 20); //20us中断一次
	TIM_ARRPreloadConfig(PTR_CTRL_TIMER, ENABLE);

	TIM_ITConfig(PTR_CTRL_TIMER, TIM_IT_Update, ENABLE);

  	TIM_Cmd(PTR_CTRL_TIMER, DISABLE);
}

/******************************************************
** 启动打印机定时器
*******************************************************/
void start_ptr_timer(void)
{
	__cnt = 0;
	__20us = 0;
	__1ms = 0;
	__50ms = 0;
	__50mscnt = 0;
	TIM_Cmd(PTR_CTRL_TIMER, ENABLE);
	
}

/******************************************************
** 关闭打印机定时器
*******************************************************/
void close_ptr_timer(void)
{
	TIM_Cmd(PTR_CTRL_TIMER, DISABLE);
	__cnt = 0;
	__20us = 0;
	__1ms = 0;
	__50ms =0;
	__50mscnt = 0;
	PTR_PWR_OFF;
}


//-----20us中断一次---------
void ptr_timer_irq(void)
{
  	extern void ptr_heat_tick(void);
	extern void ptr_step_tick(void);
	extern void ptr_pwr_ctrl_tick(void);
	extern void heater_detect_tick(void);
	extern void _ptr_ctrl(void);
	__20us = 1;
	
	if(__cnt++ >= 50)
	{
		__1ms = 1;
		__cnt = 0;
		
		__50mscnt++;
		if(__50mscnt >= 50)
		{
			__50ms = 1;  
		}
	}

	if(__20us)
		ptr_heat_tick();

	if(__20us)
		ptr_step_tick();	
	
	
	if(__20us)
  	{
		ptr_pwr_ctrl_tick();
	}
	//-----------------------------------
	  
	if(__1ms)
	{  
		heater_detect_tick();
	}
	
	if(__20us)
		_ptr_ctrl();
	
	__1ms = 0;
	__50ms = 0;
}

void TIM6_DAC_IRQHandler(void)
{
	ptr_timer_irq();  
	TIM_ClearFlag(TIM6,TIM_FLAG_Update);
}
//---------------------------------------------------------


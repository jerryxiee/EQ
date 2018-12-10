#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "stm32f2xx_gpio.h"
#include "stm32f2xx_adc.h"

#include "tp_hardware.h"



static void ptr_timer_cfg(void);
static void ptr_adc_cfg(void);
static s16 temperature_factor(void);









	



//-------------------加热控制----------------------
static s8 heat_finished;
static s16 heat_time;  //单位20u秒
static u32 heat_tick;

/*******************************************************************
** 函数名：ptr_heat
** 功能描述：启动打印机加热，根据打印的点数pt_1_dots，计算加热时间
** 参数：pt_1_dots 打印的点数，即要显示在纸上的点数，最大是384点
********************************************************************/
void ptr_heat(u16 pt_1_dots)
{
	heat_finished = 0;
	heat_tick = 0;
	heat_time = 100;
	
	if(pt_1_dots > 0 )
	{
	  	if(pt_1_dots > 200)
		  	pt_1_dots = 200;
		
	  	heat_time += (pt_1_dots);// + temperature_factor(); // dxl

		PTR_HEAT_ON;
	}
}

//在定时中断函数中调用
static void ptr_heat_tick(void)
{
	if(heat_tick < heat_time + 100) 
	{
		heat_tick++;
		if(heat_tick == heat_time)
		{
			PTR_HEAT_OFF;
		}
		else if(heat_tick >= heat_time + 100)
		{
		  	PTR_HEAT_OFF;
		  	heat_finished = 1;
		}
	}
	else if(IS_PTR_HEAT_ON)
	{
	 	 PTR_HEAT_OFF;
		 heat_finished = 1;
	}
	
}

int ptr_heat_finished(void)
{
	return heat_finished;
}
//-----------------------------------------------------











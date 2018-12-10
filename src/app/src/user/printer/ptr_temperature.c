#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "ptr_timer.h"
#include "include.h" 
#include "ptr_temperature.h"

//打印机热敏电阻的阻值表
const u16 T_R_table[25] = 
{843/*对应-40℃*/, 623, 466, 352, 269, 208, 161, 124, 97, 76, 60,
47, 37, 30/*对应25℃*/, 24, 20, 16, 13, 11, 9, 7, 6, 5, 4, 3/*80℃*/};


static struct _T_ctrl T_ctrl={0,0,0,0,0,0,25};
/************************************************************
** 用ADC采集热敏电阻的电压，获得环境的温度。为了避免打印时加热
** 对热敏电阻的影响，所以要在打印停止30秒后。即打印机停止工作30
** 秒之后，如果再次启动打印机，那么在启动打印机之前采集环境温度，
** 根据采集到的温度，调节打印机的加热时间
*************************************************************/
#if 0
void ptr_adc_cfg(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);	
  
  GPIO_StructInit(&GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOF, &GPIO_InitStructure);

  ADC_RegularChannelConfig(ADC3, ADC_Channel_15, 1, ADC_SampleTime_15Cycles);
}
#endif

static s8 computer_temperature(u16 adc_value)
{
  	s8 temperature, tmp_t;
	u32 r;
	u32 tmp;
	u8 i;
				
	tmp = Ad_GetValue(ADC_PRINTF_THE);
							
	r = (tmp * 30) / (0xfff - tmp);  //算出热敏电阻的阻值
					
	for(i = 0; i < 25; i++)
	{
		if( r == T_R_table[i] )
		{
			temperature = -40 + i*5;
			break;
		}
		else if( (r < T_R_table[i]) && (r > T_R_table[i+1]) )
		{
			u16 tmp0 = T_R_table[i] - T_R_table[i+1];
							  
			if ( (T_R_table[i] - r ) > ( r - T_R_table[i+1]) )
			{  
				temperature = -40 + (i+1)*5;
								
				tmp_t = 5*(r - T_R_table[i+1]);
								
				temperature -= tmp_t / tmp0 ;
								
				if ( (tmp0 - tmp_t % tmp0) <= tmp_t)
				{
					temperature--;
				}
			}
			else
			{
				temperature = -40 + (i)*5;
								
				tmp_t = 5* (T_R_table[i] - r);
								
				temperature +=  tmp_t / tmp0;
								
				if ( (tmp0 - tmp_t % tmp0) <= tmp_t)
				{
					temperature++;
				}
			}  
		}
	}
	
	return temperature;
}

void init_temperature(void)
{
 	memset(&T_ctrl, 0, sizeof(struct _T_ctrl));
}

/***************************************************************
** 温度检测控制, 每50ms调用一次
****************************************************************/
void temperature_detect_tick(void)
{
  	static u32 oldtime = 0; 
	
	u32 _50ms = 0;
	
	struct _T_ctrl *t_ctrl = &T_ctrl;
		
	if((Timer_Val() - oldtime) > 0) //Timer_Val()的单位是50ms
	{
		_50ms = 50;
		oldtime = Timer_Val();
	}
	/*
	tick++;
	if(tick > 5000)
	{
	  	tick = 0;
	  	_50ms = 50;
	}
	*/
	
	if(t_ctrl->status)
	{
			t_ctrl->delay += _50ms;

			if(t_ctrl->delay >= 50) //每50毫秒启动采集一次
			{
			  	u32 tmp = 10000;
				
				t_ctrl->delay = 0;
				
			#if ADC_DMA_INCLUDE_EN	
				ADC_SoftwareStartConv(PTR_ADC);
				while(ADC_GetFlagStatus(PTR_ADC, ADC_FLAG_EOC) == RESET)
				{
				  if(tmp == 0)
						break;
				  tmp--;
				}
			#else 
				tmp = Ad_GetValue(ADC_PRINTF_THE);
			#endif
				
				if( (tmp > 0) && (tmp <= 0xFFF) )
				{
				  	t_ctrl->conv_sum += tmp;
					t_ctrl->cnt++;
				}
				else
				{
					t_ctrl->conv_sum += 0xFFF/2;  //热敏电阻在常温25度下，阻值是30K，所以采样值是0xFFF/2
					t_ctrl->cnt++;
				}
				
				if(t_ctrl->cnt >= 3)
				{
					t_ctrl->conv_sum /= t_ctrl->cnt;
					t_ctrl->status = 0;
					t_ctrl->cnt = 0;
				
				#if ADC_DMA_INCLUDE_EN	
					ADC_Cmd(PTR_ADC, DISABLE);
				#endif
					
					t_ctrl->temperature = computer_temperature(t_ctrl->conv_sum);
					
					t_ctrl->finished = 1;
					
					t_ctrl->finished_keep_time = 60 * 1000; //60秒之后,finished 状态消除
				}
			}
	}
	else if(t_ctrl->finished_keep_time)
	{
	 	t_ctrl->finished_keep_time -= _50ms;
		if(t_ctrl->finished_keep_time == 0)
		  t_ctrl->finished = 0;
	}
	
	_50ms = 0;
	  
}


/**********************************************
** 启动温度检测
***********************************************/
int start_T_datect(void)
{
  	struct _T_ctrl *t_ctrl = &T_ctrl;
	
	if( t_ctrl->finished )
	{
		return 0;  
	}
	if(t_ctrl->status)
	{
	    return 0;
	}
	
	t_ctrl->finished = 0;
  	t_ctrl->cnt = 0;
	t_ctrl->conv_sum = 0;
	t_ctrl->delay = 0;

#if ADC_DMA_INCLUDE_EN		
	ADC_Cmd(PTR_ADC, ENABLE);
#endif
	
	t_ctrl->status = 1;
	
	start_ptr_timer();
	
	return 1;
}

/**********************************************
** 强制启动温度检测
***********************************************/
int force_start_T_datect(void)
{
  	T_ctrl.finished = 0;
  	return start_T_datect();
}

/**********************************************
** 返回温度
***********************************************/
s16 get_temperature(void)
{
	if( T_ctrl.finished == 0)
	{
		return 0;  
	}  
	
	else
	    return T_ctrl.temperature;
}

/**********************************************
** 询问温度检测是否完成
***********************************************/
s8 is_T_detect_finished(void)
{
  	return T_ctrl.finished;
}

/**********************************************
** 询问温度检测是否完成
***********************************************/
int is_too_hot(void)
{
	if( T_ctrl.finished == 0)
	{
	  	u16 tmp;
	  	#if ADC_DMA_INCLUDE_EN	
			ADC_SoftwareStartConv(PTR_ADC);
			while(ADC_GetFlagStatus(PTR_ADC, ADC_FLAG_EOC) == RESET)
			{
				 if(tmp == 0)
					break;
				 tmp--;
			}
		#else 
			tmp = Ad_GetValue(ADC_PRINTF_THE);
		#endif
			
	  	if (computer_temperature(tmp) > 70)
		  	return 1;  
		
		return 0;  
	}  
	else if(T_ctrl.temperature > 70) //大于70度就认为是过热
	{
		return 1;  
	}
	return 0;
}




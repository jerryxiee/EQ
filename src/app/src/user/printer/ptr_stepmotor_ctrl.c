#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "stm32f2xx_gpio.h" 
#include "include.h"
#include "ptr_stepmotor_ctrl.h"


static struct _step_ctrl step_ctrl;

/*****************************************************************
** 硬件初始化
******************************************************************/
void ptr_io_init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	GPIO_StructInit(&GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;

	RCC_AHB1PeriphClockCmd(MBF_GPIO_RCC_APBPeriph, ENABLE);
  	GPIO_InitStructure.GPIO_Pin   = MBF_GPIO_Pin;
  	GPIO_Init(MBF_GPIO, &GPIO_InitStructure);
	MBF_0;
	
	RCC_AHB1PeriphClockCmd(MB_GPIO_RCC_APBPeriph, ENABLE);
  	GPIO_InitStructure.GPIO_Pin   = MB_GPIO_Pin;
  	GPIO_Init(MB_GPIO, &GPIO_InitStructure);
	MB_0;

	RCC_AHB1PeriphClockCmd(MAF_GPIO_RCC_APBPeriph, ENABLE);
  	GPIO_InitStructure.GPIO_Pin   = MAF_GPIO_Pin;
  	GPIO_Init(MAF_GPIO, &GPIO_InitStructure);
	MAF_0;
	
	RCC_APB2PeriphClockCmd(MA_GPIO_RCC_APBPeriph, ENABLE);
  	GPIO_InitStructure.GPIO_Pin   = MA_GPIO_Pin;
  	GPIO_Init(MA_GPIO, &GPIO_InitStructure);
    MA_0;
	
	RCC_AHB1PeriphClockCmd(STR_GPIO_RCC_APBPeriph, ENABLE);
  	GPIO_InitStructure.GPIO_Pin   = STR_GPIO_Pin;
  	GPIO_Init(STR_GPIO, &GPIO_InitStructure);

	RCC_AHB1PeriphClockCmd(PTR_PWR_GPIO_RCC_APBPeriph, ENABLE);
  	GPIO_InitStructure.GPIO_Pin   = PTR_PWR_GPIO_Pin;
  	GPIO_Init(PTR_PWR_GPIO, &GPIO_InitStructure);
	PTR_PWR_OFF;

	RCC_AHB1PeriphClockCmd(PTR_HEAT0_GPIO_RCC_APBPeriph, ENABLE);
  	GPIO_InitStructure.GPIO_Pin   = PTR_HEAT0_GPIO_Pin;
  	GPIO_Init(PTR_HEAT0_GPIO, &GPIO_InitStructure);
	PTR_HEAT_OFF;
    
    RCC_AHB1PeriphClockCmd(PTR_HEAT1_GPIO_RCC_APBPeriph, ENABLE);
  	GPIO_InitStructure.GPIO_Pin   = PTR_HEAT1_GPIO_Pin;
  	GPIO_Init(PTR_HEAT1_GPIO, &GPIO_InitStructure);
	PTR_HEAT_OFF;

	RCC_AHB1PeriphClockCmd(SCLK_GPIO_RCC_APBPeriph, ENABLE);
  	GPIO_InitStructure.GPIO_Pin   = SCLK_GPIO_Pin;
  	GPIO_Init(SCLK_GPIO, &GPIO_InitStructure);

	RCC_AHB1PeriphClockCmd(MOSI_GPIO_RCC_APBPeriph, ENABLE);
  	GPIO_InitStructure.GPIO_Pin   = MOSI_GPIO_Pin;
  	GPIO_Init(MOSI_GPIO, &GPIO_InitStructure);


	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	
	RCC_AHB1PeriphClockCmd(PTR_PAPER_GPIO_RCC_APBPeriph, ENABLE);
  	GPIO_InitStructure.GPIO_Pin   = PTR_PAPER_GPIO_Pin;
  	GPIO_Init(PTR_PAPER_GPIO, &GPIO_InitStructure);
}

/**********************************************************
** 每20us调用一次,使得IO口实处PWM波形，控制打印机电源打开
**********************************************************/
static u8 ptr_pwr = 0;
void ptr_pwr_ctrl_tick(void)
{
	if(print_version == 2) //低成本方案
	{
		if(ptr_pwr == 1)
		{
			PTR_PWR_ON; 
		}
		else
		{
			PTR_PWR_OFF;
		}
	}
	else
	{
		if(PTR_PWR_STATE)
			PTR_PWR_OFF;
		else
			PTR_PWR_ON; 
	}
}

void ptr_pwr_on(void)
{
	ptr_pwr =1; 	
	PTR_PWR_ON;
}

void ptr_pwr_off(void)
{
	ptr_pwr =0; 	
	PTR_PWR_OFF;
}

/**********************************************
** 函 数 名: ptr_step_into
** 功能描述: 使步进电机转动steps 步
***********************************************/
int ptr_step_into(u16 steps)
{
	step_ctrl.status = 1;
	step_ctrl.step = 0;
	step_ctrl.delay_flag = 0;
	step_ctrl.delay = 0;
	step_ctrl.delay_time = 0;
	
	step_ctrl.steps = steps;
	step_ctrl.finished = 0;
	
	return 1;
}

/**********************************************
检查步进电机是否完成转动
***********************************************/
int ptr_step_finished(void)
{
	return step_ctrl.finished;
}

#if 0
/***********************************************************
** 步进电机控制节拍，每20us调用一次(在定时中断函数中调用)
************************************************************/
int ptr_step_tick(void)
{
	
	if(step_ctrl.status)
	{
		if(step_ctrl.delay_flag == 0)
		{
			switch(step_ctrl.step)
			{
	           case 1:         
	                  MBF_1;
	                  MAF_0;
	                  MA_1;
					  MB_1;
					  step_ctrl.delay_flag = 1;
					  step_ctrl.delay = 0;
					  step_ctrl.delay_time = 25;//25
					  break;  
							  
	           case 0:
	                  MBF_1;
	                  MAF_1;
					  MB_1;
	                  MA_1;
					  step_ctrl.delay_flag = 1;
					  step_ctrl.delay = 0;
					  step_ctrl.delay_time = 105;//25
	                  break;  

	           case 3:
	                  MBF_0;
	                  MAF_1;
					  MB_1;
	                  MA_1;
					  step_ctrl.delay_flag = 1;
					  step_ctrl.delay = 0;
					  step_ctrl.delay_time = 25;//25
	                  break;  
	           case 2:
	                 MBF_0;
	                  MAF_0;
					  MB_1;
	                  MA_1;
					  step_ctrl.delay_flag = 1;
					  step_ctrl.delay = 0;
					  step_ctrl.delay_time = 105;//25
	                  break;   

			default:
			
					step_ctrl.step = 0;
			
					if( step_ctrl.steps ) 
						step_ctrl.steps--;

					if( step_ctrl.steps == 0) 
					{
						step_ctrl.status = 0;
					  	step_ctrl.delay_flag = 0;
					  	step_ctrl.delay = 0;
						step_ctrl.finished = 1;
					}
					
					break;
			}
		}
		else
		{
			step_ctrl.delay++;
			if(step_ctrl.delay > step_ctrl.delay_time)
			{
			  	if( step_ctrl.steps == 0) 
				{
				  	step_ctrl.status = 0;
				    step_ctrl.finished = 1;
				}
				else
				    step_ctrl.step++;
				step_ctrl.delay_flag = 0;
			}
		}
		
	}
	
	return 0;
}
#else
/***********************************************************
** 步进电机控制节拍，每20us调用一次(在定时中断函数中调用)
************************************************************/
int ptr_step_tick(void)
{
	
	if(step_ctrl.status)
	{
		if(step_ctrl.delay_flag == 0)
		{
		  	if(print_version == 2) //低成本方案
		 		step_ctrl.delay_time = 20;//16;//20;
			else
			  	step_ctrl.delay_time = 13;
			
			switch(step_ctrl.step)
			{
	           case 7:       //---A  
	                  MBF_1;
	                  MAF_0;
	                  MA_0;
					  MB_1;
					  step_ctrl.delay_flag = 1;
					  step_ctrl.delay = 0;
					  step_ctrl.delay_time = 0;//25
					  break;  
							  
	           case 6:
	                  MBF_1;  //---AB
	                  MAF_1;
					  MB_1;
	                  MA_1;
					  step_ctrl.delay_flag = 1;
					  step_ctrl.delay = 0;
					  //step_ctrl.delay_time = 1;
	                  break;  

	           case 5:
	                  MBF_0; //----B
	                  MAF_1;
					  MB_0;
	                  MA_1;
					  step_ctrl.delay_flag = 1;
					  step_ctrl.delay = 0;

	                  break;  
	           case 4:     //-------BC
	                 MBF_0;
	                  MAF_1;
					  MB_1;
	                  MA_1;
					  step_ctrl.delay_flag = 1;
					  step_ctrl.delay = 0;
					  
					  //step_ctrl.delay_time = 13;
	                  break;   
					  
				case 3:     //-------C
	                 MBF_0;
	                  MAF_0;
					  MB_1;
	                  MA_0;
					  step_ctrl.delay_flag = 1;
					  step_ctrl.delay = 0;
					  step_ctrl.delay_time = 0;//25
	                  break;  	  
			    case 2:     //-------CD
	                 MBF_0;
	                  MAF_0;
					  MB_1;
	                  MA_1;
					  step_ctrl.delay_flag = 1;
					  step_ctrl.delay = 0;
					  //step_ctrl.delay_time = 1;
	                  break; 
				 case 1:     //-------D
	                 MBF_0;
	                  MAF_0;
					  MB_0;
	                  MA_1;
					  step_ctrl.delay_flag = 1;
					  step_ctrl.delay = 0;

	                  break;  
				 case 0:     //-------DA
	                 MBF_1;
	                  MAF_0;
					  MB_1;
	                  MA_1;
					  step_ctrl.delay_flag = 1;
					  step_ctrl.delay = 0;
					  //step_ctrl.delay_time = 13;
	                  break;  
			default:
			
					step_ctrl.step = 0;
			
					if( step_ctrl.steps ) 
						step_ctrl.steps--;

					if( step_ctrl.steps == 0) 
					{
						step_ctrl.status = 0;
					  	step_ctrl.delay_flag = 0;
					  	step_ctrl.delay = 0;
						step_ctrl.finished = 1;
					}
					
					break;
			}
		}
		else
		{
			step_ctrl.delay++;
			if(step_ctrl.delay > step_ctrl.delay_time)
			{
			  	if(step_ctrl.steps)
				  	step_ctrl.steps--;
				
			  	if( step_ctrl.steps == 0) 
				{
				  	step_ctrl.status = 0;
				    step_ctrl.finished = 1;
				}
				else
				    step_ctrl.step++;
				step_ctrl.delay_flag = 0;
			}
		}
		
	}
	
	return 0;
}
#endif
/***************************************
**让步进电机从第step步开始转动一步
****************************************/
int ptr_step(u8 step, u8 steps)
{
  
	step_ctrl.delay_flag = 0;
	step_ctrl.delay = 0;
	step_ctrl.delay_time = 0;
	
	step_ctrl.steps = steps;
	step_ctrl.finished = 0;

  	step_ctrl.step = step;
	step_ctrl.status = 1;
	
	return 0;
}


static void _delay(u32 n)
{
	for(;n>0;n--);
}
/***************************************
** 模拟SPI口发送一个字节
****************************************/
static u8 ptr_port_send_byte(u8 _byte)
{
  	u8 i;
		
	for(i = 0; i < 8; i++ )
	{
		if(_byte & 0x80)
			MOSI_1;
		else
			MOSI_0;
		
		_byte <<= 1;

		SCLK_0;
		_delay(10);
		SCLK_1;
		_delay(10);
	}
	SCLK_0;
	
	return 1;
}

/***************************************
** 往打印机写入数据
****************************************/
void write_ptr(u8 *buf, u16 cnt)
{
	while(cnt>0)
	{
		ptr_port_send_byte(*buf++); 
		cnt--;
	}

	//锁存脉冲宽度至少100ns
	STR_1;
	_delay(10);
	STR_0;
	_delay(10);
	STR_1;
	
}


//探测打印机是否缺纸；如果有纸，就返回1
int tp_paper_detect(void)
{	
	if(PTR_PAPER_STATE)
			return 1;
	return 0;
}





//---------------------------------加热控制------------------------------------
struct _heat_ctrl
{
  	char status;
	u32 strobe_time[2];
	u8 step;
	s8 finished;
	s32 time;
	s32 tick;
};
struct _heat_ctrl heat_ctrl;

//获取缓冲区中1的位数
static s16 _1_bits_of(u8 *buf, u32 cnt)
{
  	u16 bits = 0;
	
	while(cnt-- > 0)
	{	
	  	u8 i;
	  	u8 ch = *buf++;
	  	
		for(i = 0; i < 8; i++)
		{
		  	if(ch & 1)
			   bits++;
		  	ch >>= 1;
		}
	}
	
	return bits;
}
//计算加热时间
u32 computer_heat_time(s8 temperature, u16 dots)
{
  	u16 cnt = dots;
	u16 tmp;
	u8 i;
  	u32 h_time;

	if(print_version == 2) //低成本方案
	{
	  	h_time = 120;
	  	//cnt/2 + cnt/4 + cnt/8 + cnt/16 + cnt/32 + (cnt/64)*30;		
		for(i = 0; i < 10; i++)
		{
			h_time += cnt/ (0x01<<i) ;
		}
		
		for(i = 0; i < 10; i++)
		{
			tmp = cnt/(64 + i*10);
			h_time += tmp * (5*i);
		}
	}
	else
	{
		h_time = 50;
		for(i = 0; i < 6; i++)
		{
			h_time += cnt / (0x02<<i) ;
		}
		
		for(i = 0; i < 8; i++)
		{
			tmp = cnt/(64 + i*16);
			h_time += tmp * (5*i);
		} 
	}
	
	if(temperature >= 40)
		h_time = (h_time * 100) / 130;
	else if(temperature <= 5)
		h_time = (h_time * 130) / 100;
				
	return h_time;
}

//根据打印机加热电路原理，将打印机加热头分为两个区，计算出每个区的加热点数
static void strobe_time(u8 *pt_buf, u16 cnt, s8 temperature, struct _heat_ctrl *heat_ctrl)
{
  	u16 strobe0_dots = 0;
	u16 strobe1_dots = 0;
	u16 i;
	
	for(i=0;i<cnt;i++)
	{
	  	if ( ( (i/8) & 0x01) == 0)
			strobe0_dots  += _1_bits_of(&pt_buf[i], 1);
		else
			strobe1_dots  += _1_bits_of(&pt_buf[i], 1);
	}
	
	heat_ctrl->strobe_time[0] = computer_heat_time(temperature, strobe0_dots);
	heat_ctrl->strobe_time[1] = computer_heat_time(temperature, strobe1_dots);
}


/*******************************************************************
** 函数名：ptr_heat
** 功能描述：启动打印机加热，根据打印的点数pt_1_dots，计算加热时间
** 参数：pt_1_dots 打印的点数，即要显示在纸上的点数，最大是384点
********************************************************************/
void ptr_heat(u8 *pt_buf, u16 cnt, s8 temperature)
{
	struct _heat_ctrl *h_ctrl = &heat_ctrl;
	
	strobe_time(pt_buf, cnt, temperature, h_ctrl);
	
	h_ctrl->status = 1;
	
	h_ctrl->step = 0;
	
	h_ctrl->finished = 0;
	
	h_ctrl->tick = 0;
}

/***************************************************
** 每20us调用一次(在定时中断函数中调用)
****************************************************/
void ptr_heat_tick(void)
{
  	struct _heat_ctrl *h_ctrl = &heat_ctrl;
	
	if(h_ctrl->status)
	{
		switch(h_ctrl->step) 
		{
		case 0:
		  	PTR_STROBE0_ON;
			h_ctrl->step++;
			h_ctrl->tick = 0;
			break;
		case 1:
		  	h_ctrl->tick++;
			if(h_ctrl->tick >= h_ctrl->strobe_time[0])
			{
				 PTR_STROBE0_OFF;
				 
				 PTR_STROBE1_ON;
				 h_ctrl->step++;
				 h_ctrl->tick = 0;
			}
			break;
		case 2:
		  	h_ctrl->tick++;
			if(h_ctrl->tick >= h_ctrl->strobe_time[1])
			{
				 PTR_STROBE1_OFF;
				 
				 h_ctrl->step++;
				 h_ctrl->tick = 0;
			}
			break;
		case 3:
		  	h_ctrl->tick++;
			if(h_ctrl->tick >= 50)
			{
				h_ctrl->finished = 1; 
				h_ctrl->status = 0;
			}
			break;
		default:
		  	PTR_HEAT_OFF;
		 	h_ctrl->finished = 1;
			h_ctrl->status = 0;
		}
	}
	else if(IS_PTR_HEAT_ON)
	{
	 	PTR_HEAT_OFF;
		h_ctrl->finished = 1;
		h_ctrl->status = 0;
	}
}


int ptr_heat_finished(void)
{
	return heat_ctrl.finished;
}
//-----------------------------------------------------

#if 0

#define REF_VOLTAGE  3300  //参考电压是3.3V
//打印机加热电压探测,返回电压值，单位mV
u32 get_ptr_vh(void)
{
    u32 tmp;
	
 	tmp = Ad_GetValue(ADC_PRINTF);//读取ADC采样值
	
	return 4 * tmp * 3300 / 0xfff;
}


//计算加热时间
s32 computer_heat_time(s8 temperature, u16 dots)
{
  	u64 e0, vh, tmp1, tmp2, tmp3;
	u64 t;
	
	if(temperature < 5)
	  	e0 = 0.0002  * 1000000; //
	else if(temperature > 40)
	    e0 = 0.00016  * 1000000; //
	else
	    e0 = 0.00018  * 1000000; //
	
	e0 *= dots;
	
  	vh = get_ptr_vh(); //单位是毫伏
	
	tmp1 = (vh - 300);
	tmp1 *= tmp1;
	
	tmp2 = 176*100 - 1170;
	tmp2 *= tmp2;
	
	t = (e0 * tmp2);
	tmp3 = (tmp1 * 176); //单位是us
	
	t = t / tmp3;
    return t;
}
#endif

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdarg.h"  

#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "stm32f2xx_gpio.h"
#include "Adc.h"
#include "timer.h"

#include "font.h"
#include "ptr_timer.h"
#include "ptr_temperature.h"
#include "ptr_stepmotor_ctrl.h"
#include "b_queue.h"
#include "printer.h"

#define PRINT_QUEUE_BUF_SIZE  2048
#define TP_BUF_SIZE  1500

u8 print_version;

static u8 buf1[PRINT_QUEUE_BUF_SIZE];  
static u8 buf2[TP_BUF_SIZE];  
static s_PRINTER Printer;
/*****************************************************
** 函数名: get_dot_image
** 功能描述: 获取字符的点阵码形
** 参数说明: code: 字符的编码(GBK码或ASC码)
             font: 字体
             dot_image: 存放读取到的点阵码形的缓冲区
******************************************************/
int get_dot_image_Flash(u16 code, u8 font, u8 *dot_image);

/**********************************************************
** 函 数 名: printer_init
** 功能描述: 初始化打印机
***********************************************************/
static void printer_init(s_PRINTER *ptr)
{
  	struct _print_p_map p_map;
	
	memset(ptr, 0, sizeof(s_PRINTER));

	ptr->status = PTR_IDLE;

	b_queue_init(&ptr->b_queue, buf1, PRINT_QUEUE_BUF_SIZE);

	ptr->pt_buf.data = buf2;
	ptr->pt_buf.size = TP_BUF_SIZE;

	p_map.line_width = PRINTER_LINE_DOTS;
	p_map.line_blank = 4;
	p_map.line_height = 24;
	p_map.print_type = _24x24;
	p_map.image_width = 0;
	p_map.image_height = 0;
	
	set_print_param(&p_map);
	  
	ptr->tp_heater_detect = &is_too_hot;
	ptr->tp_paper_detect = &tp_paper_detect;

	
}

static int __print(s_PRINTER *ptr, char *pt_data, u32 cnt)
{
  	e_PRINTER_STATUS ptr_status = printer_status();
	
	if ( (ptr_status == PTR_NO_PAPER) 
			 || (ptr_status == PTR_OVER_T))
		return 0;
	
	return in_b_queue(&ptr->b_queue, (u8*)pt_data, cnt);
}


/*******************************************
** 启动打印机
********************************************/
static int start_print(s_PRINTER *ptr)
{
	if(ptr == NULL)
		return 0;

	if(ptr->tp_heater_detect != NULL)
	{	
		if( ptr->tp_heater_detect() )
		{
			ptr->status = PTR_OVER_T;
			return 0;
		}
	}

	if(ptr->tp_paper_detect != NULL)
	{	
		if( ptr->tp_paper_detect() == 0)
		{
			ptr->status = PTR_NO_PAPER;
			return 0;
		}
	}
	
	ptr->status = PTR_BUSY;

	ptr->ctrler.status = 1;
	ptr->ctrler.step = 0;
	ptr->ctrler.line_p = 0;

	PTR_PWR_ON;

	start_ptr_timer();
	
	return 1;
}

/*******************************************
** 关闭打印机
********************************************/
static int stop_print(s_PRINTER *ptr)
{
	if(ptr == NULL)
		return 0;
	
	ptr->status = PTR_IDLE;

	ptr->ctrler.status = 0;
	ptr->ctrler.step = 0;
	ptr->ctrler.line_p = 0;

	PTR_PWR_OFF;

	MBF_0;
	MB_0;
    MAF_0;
    MA_0;     //关电机
	PTR_HEAT_OFF;
	
	close_ptr_timer();
	
	return 1;
}




//打印控制
static void ptr_ctrl(s_PRINTER *ptr)
{
	u8 *buf;
	s16 cnt;
	//s32 h_time;
	//s8 tmp;
	
	if(ptr == NULL)
			return;
	
	if(ptr->ctrler.status) 
	{
	  	if(!is_T_detect_finished()) //打印之前先探测温度
		{
	  		start_T_datect();
			return;
		}
		
		switch(ptr->ctrler.step)
		{
		case 0 :  
			ptr_step(0,2);
			ptr->ctrler.step++;
			break;
			
		case 1 :
			if( ptr_step_finished() ) //等电机走完一步
				ptr->ctrler.step++;
			break;
			
		case 2 :  
			ptr_step(2,2);
			ptr->ctrler.step++;
			break;
			
		case 3 :
			if( ptr_step_finished() )
				ptr->ctrler.step++;
			break;	
			
		case 4:
			buf = &ptr->pt_buf.data[ptr->ctrler.line_p++ * ptr->pt_buf.width/8];
			cnt = ptr->pt_buf.width/8;  
			write_ptr(buf, cnt);  //出数据
			
			ptr_heat(buf, cnt, get_temperature());
			
			ptr->ctrler.step++;
			break;
			
		case 5: 
			if(ptr_heat_finished()) //等待加热完成
				ptr->ctrler.step++;
			break;
			
		case 6 :  
			ptr_step(4,2);
			ptr->ctrler.step++;
			break;
			
		case 7 :
			if(ptr_step_finished())
				ptr->ctrler.step++;
			break;
			
		case 8 :  
			ptr_step(6,2);
			ptr->ctrler.step++;
			break;
			
		case 9 :
			if(ptr_step_finished())
				ptr->ctrler.step++;
			break;	
			
		case 10:
			if(ptr->ctrler.line_p >= ptr->pt_buf.height)
			{
				ptr->ctrler.step = 12;
			}
			buf = &ptr->pt_buf.data[ptr->ctrler.line_p++ * ptr->pt_buf.width/8];
			cnt = ptr->pt_buf.width/8;
			write_ptr( buf, cnt);
	
			ptr_heat(buf, cnt, get_temperature());
			
			ptr->ctrler.step++;
			
		  	break;
			
		case 11: 
			if( ptr_heat_finished() ) //等待加热完成
				ptr->ctrler.step++;
			break;
			
		case 12:
			if(ptr->ctrler.line_p >= ptr->pt_buf.height)
			{
			  ptr->ctrler.step = 14;
			}
			else
			{
				ptr->ctrler.step = 0;
			}
			break;
			
			
		default:

		  	ptr->status = PTR_IDLE;

			ptr->ctrler.status = 0;
			ptr->ctrler.step = 0;
			ptr->ctrler.line_p = 0;
		    MBF_0;
			MB_0;
			MAF_0;
			MA_0;     //关电机
			PTR_HEAT_OFF;
			ptr->ctrler.step = -1;
			
			break;
			
		}
	}
}



static int push_image(struct _image *image, struct _print_buf *pt_buf)
{	
	u16 width = image->width;
	u16 j, i;

	if( width % 8 )
	{
		//width += 8 - width%8; //调整为8的倍数，比如: 当width==15时，应该调整为16
		width = (width/8 + 1)*8;
	}

	
	for(j=0; j<image->height; j++)
	{
	    
		if ( j >= (pt_buf->height) )
		{
			break;
		}
	
		for(i=0; i<width/8; i++)
		{
			u32 offset;
			
			if ( (pt_buf->x_p + i) >= (pt_buf->width/8) )
			{
				break;;
			}
			
			offset = j * (pt_buf->width/8) + pt_buf->x_p + i;
			
			pt_buf->data[offset] = image->data[j*width/8 + i];
		}
   	}
	
	pt_buf->x_p += width/8;

	if(pt_buf->x_p >= pt_buf->width/8)
	{
		pt_buf->x_p = pt_buf->width/8;
		return -1;
	}
	return 1; 
}


/*******************************************
** 函数名: print_ctrl
** 功能描述: 打印控制
********************************************/
static u8 buf4[128];
static void print_ctrl(s_PRINTER *ptr)
{
	u8 tmp_buf[2];
	struct _image dot_image;

	if(printer_status() == PTR_BUSY)
		return;
	
	if(is_queue_enmty(&ptr->b_queue))
	{
	 	 stop_print(ptr);
	}

	dot_image.data = buf4;
	
	if(lock_b_queue(&ptr->b_queue))
	{
		char clr_flag = 0;
		char start_print_flag = 0;
		s8 font = ptr->p_map.print_type;

		
		ptr->pt_buf.x_p = 0;
		while(1)
		{	
			u16 code = tmp_buf[0];

			switch(font)
			{
			case _16x16:
				dot_image.width = 16;
				dot_image.height = 16;
				break;
			case _24x24:
				dot_image.width = 24;
				dot_image.height = 24;
				break;
			case _32x32:
				dot_image.width = 32;
				dot_image.height = 32;
				break;
			case _48x48:
				dot_image.width = 48;
				dot_image.height = 48;
				break;
			case _64x64:
				dot_image.width = 64;
				dot_image.height = 64;
				break;
			default:
				dot_image.width = 16;
				dot_image.height = 16;
				break;
			}
			
			if(clr_flag == 0)
			{
				//打印开始前，清空打印缓存
				memset(ptr->pt_buf.data, 0, ptr->pt_buf.size);
				ptr->pt_buf.x_p = 0;
				ptr->pt_buf.y_p = 0;
				clr_flag = 1;
			}
			
			if( out_b_queue(&ptr->b_queue, &tmp_buf[0], 1) == 0)
			{
				break;
			}
			code = tmp_buf[0];  
			if(code >= 0xA1) 
			{
				if(out_b_queue(&ptr->b_queue, &tmp_buf[1], 1))
				{
				  	code <<= 8;
					code += tmp_buf[1];
				}
			}

			if(code < 0x00ff) 
			{
				if(font == _24x24)
					dot_image.width = 16;
				else
					dot_image.width /= 2;
			}
			
			if( (ptr->pt_buf.x_p + dot_image.width/8) > (ptr->pt_buf.width/8) )
			    break;

			if(code == '\n') //遇到换行符
			{
				start_print_flag = 1;
				  break;
			}
			else if(code != '\r')
			{
				start_print_flag = 1;
                                
				get_dot_image_Flash(code, font, dot_image.data); //获取一个字的点阵码形
				//get_dot_image(code, font, dot_image.data); //获取一个字的点阵码形
				//image_conv(dot_image.data,dot_image.data);
				
				if( push_image(&dot_image, &ptr->pt_buf) == -1)//将点阵码形，放到打印缓存
				{
				  	while(1)
					{
				  		s8 flag = read_b_queue_fst_byte(&ptr->b_queue, &tmp_buf[0]);
						
						if(flag != 1 )
						   break;
						
						if(tmp_buf[0] == '\r') 
						{
						  	out_b_queue(&ptr->b_queue, &tmp_buf[0], 1);
						}
						else if(tmp_buf[0] == '\n') 
						{
							out_b_queue(&ptr->b_queue, &tmp_buf[0], 1);
							break;
						}
						else
						{
							break;  
						} 
					}
				
				  	break;
				}
			}
			
		}

		if(start_print_flag == 1)
		{
		  	if(printer_status() == PTR_IDLE)
			{
			  	ptr_io_init(); //2014.1.6 
				start_print(ptr); //启动打印
			}
		}
		
		
		unlock_b_queue(&ptr->b_queue);
	}			
		
}

//在打印的过程中3秒钟采集一次温度，如果过热，就停止打印机
//该函数放在定时器中断中调用，每毫秒调用一次
void heater_detect_tick(void)
{
	static u32 _tick = 0;
	
	_tick++;
	
	if(_tick > 3 * 1000)
	{
	  	_tick = 0;
		
		force_start_T_datect();
	}
	
	if( is_too_hot() )
	{
	    stop_print(&Printer);
	}
	
}




u8 is_print_finished(void)
{
 	if(is_queue_enmty(&Printer.b_queue))
	   return 1;
	 
	 return 0;
}

void ptr_pwr_ctrl(char on_off)
{
    if(on_off == 1)
	{
 		ptr_pwr_on();
	}
	else if(on_off == 0)
	{
	    ptr_pwr_off();
		stop_print(&Printer);
	}
}


/******************************************
** 函 数 名: printer_status
** 功能描述: 返回打印机当前的状态
** 返 回 值: PTR_IDLE: 打印机空闲
		     PTR_BUSY: 打印机忙  
			 PTR_NO_PAPER: 打印机缺纸
			 PTR_OVER_T:   超温度范围
*******************************************/
e_PRINTER_STATUS printer_status(void)
{
  	s_PRINTER *ptr = &Printer;
	
	if(ptr->tp_heater_detect != NULL)
	{	if( ptr->tp_heater_detect() )
			ptr->status = PTR_OVER_T;
		else if(ptr->status == PTR_OVER_T)
			ptr->status = PTR_IDLE;	  
	}

	if(ptr->tp_paper_detect != NULL)
	{	
	  	if( ptr->tp_paper_detect() == 0)
			ptr->status = PTR_NO_PAPER;
		else if(ptr->status == PTR_NO_PAPER)
			ptr->status = PTR_IDLE;	  
	}	
	
	return ptr->status;
}

/*******************************************
** 设置打印参数
********************************************/
char set_print_param(struct _print_p_map *p_map)
{
  	struct _printer *ptr = &Printer;
	
	if(p_map == NULL)
			return 0;
	
	if(p_map->line_width > 0)
		ptr->p_map.line_width = p_map->line_width;
	
	if(p_map->line_blank >= 0)
		ptr->p_map.line_blank = p_map->line_blank;
	
	if(p_map->print_type >= 0)
		ptr->p_map.print_type = p_map->print_type;
	
	switch(ptr->p_map.print_type)
	{
	case _16x16:
	  	ptr->p_map.line_height = 16;
		break;
	case _24x24:
		ptr->p_map.line_height = 24;
		break;
	case _32x32:
		ptr->p_map.line_height = 32;
		break;
	default:
		ptr->p_map.line_height = 16;
		break;
	}
	
	if(p_map->image_width > 0)
		ptr->p_map.image_width = p_map->image_width;
	if(p_map->image_height > 0)
		ptr->p_map.image_height = p_map->image_height;
	
	
	  
	ptr->pt_buf.width = ptr->p_map.line_width;
	ptr->pt_buf.height = ptr->p_map.line_height + ptr->p_map.line_blank;
	
	return 1;
}

/**********************************************************
** 函 数 名: fmt_print
** 功能描述: 打印格式化字符串
** 返    回: 返回打印的字节数,如果返回0，说明没有打印
***********************************************************/
static char buf3[512];
int fmt_print(const char *fmt,...)
{
	char *tmp_buf = buf3;
	
	va_list args;
	int n;
	
	va_start(args, fmt);
	n = vsprintf(tmp_buf, fmt, args);
	va_end(args);

	return __print(&Printer, tmp_buf, n);
}

/*******************************************
** 打印控制任务, 由系统调用
********************************************/
void print_ctrl_task(void)
{
  	static u8 ver1_cnt = 0;
	static u8 ver2_cnt = 0;
	static u32 oldtime = 0;
	
    if(print_version)
	{
		print_ctrl(&Printer);
		temperature_detect_tick();
	}
	
	else
	{
		u16 tmp;
		
		if((Timer_Val() - oldtime) > 10) //Timer_Val()的单位是50ms, 所以是0.5秒探测一次电压
		{
			oldtime = Timer_Val();
		  	tmp = Ad_GetValue(ADC_PRINTF);
			if(tmp > 1490 && tmp < 1800)//1.2V 和 1.4V的AD内码值
			{
			    ver2_cnt++;
				ver1_cnt = 0;
			}
			else
			{
				ver1_cnt++;
				ver2_cnt = 0;
			}
		
		
			if(ver1_cnt > 10)
			{
				 print_version = 1; 
			}
			else if(ver2_cnt > 10)
			{
				 print_version = 2;  
			}
		}
		  
	}
}


/*******************************************
** 打印机初始化函数, 由系统初始化的时候调用
********************************************/
void print_init(void)
{
	ptr_io_init(); 
	ptr_timer_cfg();
	printer_init(&Printer);
}


FunctionalState Print_TimeTask(void)
{
  	return ENABLE;
}


void _ptr_ctrl(void)
{
 	ptr_ctrl(&Printer); 
}

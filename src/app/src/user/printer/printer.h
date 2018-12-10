#ifndef _PRINTER_H
#define _PRINTER_H

#include "b_queue.h"

//------------------和硬件相关-------------------------------------------
#define PRINTER_LINE_DOTS 384   //打印机一行的点数
//-----------------------------------------------------------------------





//-------定义打印机状态---------
typedef enum 
{
	PTR_IDLE,  //打印机空闲
		
	PTR_BUSY,  
	
	PTR_NO_PAPER, 
	
	PTR_OVER_T,  //超温度范围

}e_PRINTER_STATUS;




struct _print_p_map
{
	s16 line_width;
	s16 line_height;
	s16 line_blank;
	s8 print_type;
	s16 image_width;
	s16 image_height;
};

struct _print_buf
{
	u8 *data; 	//thermal printer buffer
	u32 size;
	u16 width;
	u8 height;	

	u32 x_p;
	u32 y_p;
};



typedef struct _printer
{
	e_PRINTER_STATUS status;

	struct _b_queue b_queue;

	struct
	{
		s8 status;

		s8 line_p;
		
		s8 step;
	}ctrler;

    struct _print_p_map p_map;

	struct _print_buf pt_buf;

	//u8  (*tp_send_byte)(u8 _byte);
	int (*tp_heater_detect)(void);
	
	int  (*tp_paper_detect)(void);

	//int  (*tp_print)(void);

}s_PRINTER;


extern u8 print_version;


//--------------------------------输出函数------------------------------------

extern void print_init(void); //打印机初始化

extern int fmt_print(const char *fmt, ...); //打印格式化字符串

extern e_PRINTER_STATUS printer_status(void);//返回打印机当前的状态

extern char set_print_param(struct _print_p_map *p_map);//设置打印参数

extern void print_ctrl_task(void); //打印邋控制任务，循环调用

extern void ptr_pwr_ctrl(char on_off);

extern void ptr_pwr_ctrl(char on_off); //on_off=1，表示打开打印机电源；on_off=0表示关闭打印机电源

extern u8 is_print_finished(void); //打印完成返回1，否则返回0
//----------------------------------------------------------------------------

#endif


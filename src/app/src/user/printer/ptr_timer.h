#ifndef _PTR_TIMER_H
#define _PTR_TIMER_H

//-----------硬件相关定义---------------------------------
#define PTR_CTRL_TIMER  TIM6 //定义打印机控制专用的定时器
#define RCC_APB1Periph_PTR_CTRL_TIMER RCC_APB1Periph_TIM6
#define PTR_CTRL_TIMER_IRQn TIM6_DAC_IRQn
//--------------------------------------------------------

//----------输出变量---------------------
extern u32 __20us; //时间单位是10微妙
extern u32 __1ms; //时间单位是10微妙
//---------------------------------------


//----------输出函数---------------------
extern void ptr_timer_cfg(void);
extern void start_ptr_timer(void);
extern void close_ptr_timer(void);
//---------------------------------------

#endif


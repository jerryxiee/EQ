#ifndef _PTR_STEPMOTOR_CTRL_H
#define _PTR_STEPMOTOR_CTRL_H 

//------------------------定义打印机芯控制端口---------------------------------------------
#define MBF_GPIO_RCC_APBPeriph	 RCC_AHB1Periph_GPIOE
#define MBF_GPIO	 GPIOE
#define MBF_GPIO_Pin GPIO_Pin_2
#define MBF_1   GPIO_SetBits(MBF_GPIO, MBF_GPIO_Pin)
#define MBF_0   GPIO_ResetBits(MBF_GPIO, MBF_GPIO_Pin)

#define MB_GPIO_RCC_APBPeriph	 RCC_AHB1Periph_GPIOE
#define MB_GPIO	 GPIOE
#define MB_GPIO_Pin GPIO_Pin_3
#define MB_1   GPIO_SetBits(MB_GPIO, MB_GPIO_Pin)
#define MB_0   GPIO_ResetBits(MB_GPIO, MB_GPIO_Pin)

#define MAF_GPIO_RCC_APBPeriph	 RCC_AHB1Periph_GPIOE
#define MAF_GPIO	 GPIOE
#define MAF_GPIO_Pin GPIO_Pin_4
#define MAF_1   GPIO_SetBits(MAF_GPIO, MAF_GPIO_Pin)
#define MAF_0   GPIO_ResetBits(MAF_GPIO, MAF_GPIO_Pin)

#define MA_GPIO_RCC_APBPeriph	 RCC_AHB1Periph_GPIOE
#define MA_GPIO	 GPIOE
#define MA_GPIO_Pin GPIO_Pin_5
#define MA_1   GPIO_SetBits(MA_GPIO, MA_GPIO_Pin)
#define MA_0   GPIO_ResetBits(MA_GPIO, MA_GPIO_Pin)

#define STR_GPIO_RCC_APBPeriph	 RCC_AHB1Periph_GPIOB
#define STR_GPIO	 GPIOB
#define STR_GPIO_Pin GPIO_Pin_8
#define STR_0   GPIO_ResetBits(STR_GPIO, STR_GPIO_Pin)
#define STR_1   GPIO_SetBits(STR_GPIO, STR_GPIO_Pin)

//-----------打印机电源开关控制--------------
#define PTR_PWR_GPIO_RCC_APBPeriph	 RCC_AHB1Periph_GPIOB
#define PTR_PWR_GPIO	 GPIOB
#define PTR_PWR_GPIO_Pin GPIO_Pin_7
#define PTR_PWR_ON   GPIO_SetBits(PTR_PWR_GPIO, PTR_PWR_GPIO_Pin)
#define PTR_PWR_OFF   GPIO_ResetBits(PTR_PWR_GPIO, PTR_PWR_GPIO_Pin)
#define PTR_PWR_STATE   GPIO_ReadOutputDataBit(PTR_PWR_GPIO, PTR_PWR_GPIO_Pin)


//------------------加热控制----------------------
#define PTR_HEAT0_GPIO_RCC_APBPeriph	 RCC_AHB1Periph_GPIOG
#define PTR_HEAT0_GPIO	 GPIOG 
#define PTR_HEAT0_GPIO_Pin GPIO_Pin_11
#define PTR_STROBE0_ON  GPIO_SetBits(PTR_HEAT0_GPIO, GPIO_Pin_11);
#define PTR_STROBE0_OFF  GPIO_ResetBits(PTR_HEAT0_GPIO, GPIO_Pin_11);

#define PTR_HEAT1_GPIO_RCC_APBPeriph	 RCC_AHB1Periph_GPIOC
#define PTR_HEAT1_GPIO	 GPIOC 
#define PTR_HEAT1_GPIO_Pin GPIO_Pin_2
#define PTR_STROBE1_ON  GPIO_SetBits(PTR_HEAT1_GPIO, GPIO_Pin_2);
#define PTR_STROBE1_OFF  GPIO_ResetBits(PTR_HEAT1_GPIO, GPIO_Pin_2);

#define PTR_HEAT_OFF   GPIO_ResetBits(PTR_HEAT0_GPIO, GPIO_Pin_11);GPIO_ResetBits(PTR_HEAT1_GPIO, GPIO_Pin_2)
#define PTR_HEAT_ON    GPIO_SetBits(PTR_HEAT0_GPIO, GPIO_Pin_11);GPIO_SetBits(PTR_HEAT1_GPIO, GPIO_Pin_2)
#define IS_PTR_HEAT_ON (GPIO_ReadOutputDataBit(PTR_HEAT0_GPIO, GPIO_Pin_11)||GPIO_ReadOutputDataBit(PTR_HEAT1_GPIO, GPIO_Pin_2))




//-----------------打印纸探测端口-------------------------
#define PTR_PAPER_GPIO_RCC_APBPeriph	 RCC_AHB1Periph_GPIOC
#define PTR_PAPER_GPIO	 GPIOC 
#define PTR_PAPER_GPIO_Pin GPIO_Pin_3
#define PTR_PAPER_STATE GPIO_ReadInputDataBit(PTR_PAPER_GPIO, PTR_PAPER_GPIO_Pin)

//SPI接口
#define SCLK_GPIO_RCC_APBPeriph	 RCC_AHB1Periph_GPIOE
#define SCLK_GPIO	 GPIOE
#define SCLK_GPIO_Pin GPIO_Pin_0
#define SCLK_0   GPIO_ResetBits(SCLK_GPIO, SCLK_GPIO_Pin)
#define SCLK_1   GPIO_SetBits(SCLK_GPIO, SCLK_GPIO_Pin)

#define MOSI_GPIO_RCC_APBPeriph	 RCC_AHB1Periph_GPIOE
#define MOSI_GPIO	 GPIOE
#define MOSI_GPIO_Pin GPIO_Pin_1
#define MOSI_0   GPIO_ResetBits(MOSI_GPIO, MOSI_GPIO_Pin)
#define MOSI_1   GPIO_SetBits(MOSI_GPIO, MOSI_GPIO_Pin)


//-------------------------------------------------------------------------------------------------------



struct _step_ctrl
{
	s8 status;
	s8 step;
	s8 delay_flag;
	u32 delay;
	u32 delay_time;
	
	u16 steps;
	s8 finished;
	
};

//------------输出函数------------------
extern void ptr_io_init(void);
extern int tp_paper_detect(void);
extern void ptr_heat(u8 *pt_buf, u16 cnt, s8 temperature);
extern int ptr_heat_finished(void);
extern int ptr_step_into(u16 steps);
extern int ptr_step(u8 step, u8 steps);
extern int ptr_step_finished(void);
extern void write_ptr(u8 *buf, u16 cnt); 
extern void ptr_pwr_on(void);
extern void ptr_pwr_off(void);

//--------------------------------------

#endif


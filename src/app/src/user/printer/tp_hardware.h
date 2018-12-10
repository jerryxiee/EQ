  #ifndef _TP_HARDWARE_H
#define _TP_HARDWARE_H

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
#define STR_GPIO_Pin GPIO_Pin_9
#define STR_0   GPIO_ResetBits(STR_GPIO, STR_GPIO_Pin)
#define STR_1   GPIO_SetBits(STR_GPIO, STR_GPIO_Pin)

//-----------打印机电源开关控制--------------
#define PTR_PWR_GPIO_RCC_APBPeriph	 RCC_AHB1Periph_GPIOB
#define PTR_PWR_GPIO	 GPIOB
#define PTR_PWR_GPIO_Pin GPIO_Pin_8
#define PTR_PWR_ON   GPIO_SetBits(PTR_PWR_GPIO, PTR_PWR_GPIO_Pin)
#define PTR_PWR_OFF   GPIO_ResetBits(PTR_PWR_GPIO, PTR_PWR_GPIO_Pin)
#define PTR_PWR_STATE   GPIO_ReadOutputDataBit(PTR_PWR_GPIO, PTR_PWR_GPIO_Pin)


//------------------加热控制----------------------
#define PTR_HEAT_GPIO_RCC_APBPeriph	 RCC_AHB1Periph_GPIOG
#define PTR_HEAT_GPIO	 GPIOG 
#define PTR_HEAT_GPIO_Pin GPIO_Pin_11 | GPIO_Pin_15
#define PTR_HEAT_OFF   GPIO_ResetBits(PTR_HEAT_GPIO, GPIO_Pin_11);GPIO_ResetBits(PTR_HEAT_GPIO, GPIO_Pin_15)
#define PTR_HEAT_ON    GPIO_SetBits(PTR_HEAT_GPIO, GPIO_Pin_11);GPIO_SetBits(PTR_HEAT_GPIO, GPIO_Pin_15)
#define IS_PTR_HEAT_ON (GPIO_ReadOutputDataBit(PTR_HEAT_GPIO, GPIO_Pin_11)||GPIO_ReadOutputDataBit(PTR_HEAT_GPIO, GPIO_Pin_15))

//-----------------打印纸探测端口-------------------------
#define PTR_PAPER_GPIO_RCC_APBPeriph	 RCC_AHB1Periph_GPIOB
#define PTR_PAPER_GPIO	 GPIOB 
#define PTR_PAPER_GPIO_Pin GPIO_Pin_7
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




//------------------输出函数--------------------
extern void pt_hwd_init(void);

extern int tp_heater_detect(void);
extern int tp_paper_detect(void);
extern void ptr_heat(u16 pt_1_dots);
extern int ptr_heat_finished(void);
extern int ptr_step_into(u16 steps);
extern int ptr_step_finished(void);
extern void write_ptr(u8 *buf, u16 cnt);


extern void start_ptr_timer(void);
extern void close_ptr_timer(void);

#endif


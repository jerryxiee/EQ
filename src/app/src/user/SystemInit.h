#ifndef	__SYSTEM_INIT_H
#define	__SYSTEM_INIT_H

//*************文件包含***************

//*************宏定义****************
#define USER_PLL_M	8//PLL输入频率分频,分频后频率最好在1Mhz-2Mhz
#define USER_PLL_N   384//PLLCLK = 1Mhz*PLL_N,必须为192-432之间
#define USER_PLL_P	4//SYSCLK= PLLCLK/PLL_P,为96Mhz
#define USER_PLL_Q   8//USBCLK=48Mhz


#define RCC_USB_SW    RCC_AHB1Periph_GPIOA      /* GPIO的A组端口时钟 */
#define GPIO_USB_SW   GPIOA                       /* 选定A组端口*/ 
#define PIN_USB_SW    GPIO_Pin_8  
#define USB_SW_1()    GPIO_SetBits(GPIO_USB_SW,PIN_USB_SW)	 /* IO = 1 */
#define USB_SW_0()    GPIO_ResetBits(GPIO_USB_SW,PIN_USB_SW)	 /* IO = 0 */
#define USB_SW_READ() GPIO_ReadInputDataBit(GPIO_USB_SW,PIN_USB_SW)	/* 读IO口线状态 */

//************结构体类型******************
//*************变量声明**************
extern u8      IC_Reader_flag;
//*************函数声明**************
/*********************************************************************
//函数名称	:PowerOnUpdata
//功能		:上电更新变量
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:更新的变量有：IO、GPS、位置汇报、摄像头
*********************************************************************/
void PowerOnUpdata(void);
/*********************************************************************
//函数名称	:PowerOnPeripheralSelfCheck
//功能		:上电外设自检
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:自检的外设有：摄像头
*********************************************************************/
void PowerOnPeripheralSelfCheck(void);
/*********************************************************************
//函数名称	:MemoryChipSelfCheck
//功能		:上电存储芯片自检
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:自检的存储芯片有：FRAM、FLASH、EEPROM
*********************************************************************/
void MemoryChipSelfCheck(void);
/*********************************************************************
//函数名称	:PeripheralInit
//功能		:外设初始化
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:初始化的外设有：GPS、GPRS、FRAM、FLASH、EEPROM、摄像头
*********************************************************************/
void PeripheralInit(void);
/*******************************************************************************
* Function Name  : RCC_Configuration
* Description    : Configures the different system clocks.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RCC_Configuration(void);
/*******************************************************************************
* Function Name  : IDWG_Configuration
* Description    : Configures IDWG clock source and prescaler.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void IDWG_Configuration(void);
/*******************************************************************************
* Function Name  : NVIC_Configuration
* Description    : Configures the nested vectored interrupt controller.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void NVIC_Configuration(u32 Address);
/*******************************************************************************
* Function Name  : PWR_PVDInit
* Description    : 电源PVD初始化
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PWR_PVDInit(void);
/*******************************************************************************
* Function Name  : USB_SW_Init
* Description    : USB供电引脚初始化，该引脚用于硬件复位LCD屏
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void   USB_SW_Init(void);
#endif

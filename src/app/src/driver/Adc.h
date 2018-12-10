#ifndef	__AD_H
#define	__AD_H

//*************文件包含***************
#include "stm32f2xx.h"
//*************宏定义****************
//ADC配置和DMA配置
#define EVAL_ADC                           ADC3
#define ADC_CLOCK                          RCC_APB2Periph_ADC3
#define ADC_DR_ADDRESS                     (&(ADC3->DR))           //ADC数据地址

#define ADC_DMA_CLOCK                      RCC_AHB1Periph_DMA2
#define ADC_DMA_CHAN                       DMA_Channel_2
#define ADC_DMA_STREAM                     DMA2_Stream0

//ADC 硬件引脚配置
#define ADC_EXTERN1_PIN                     GPIO_Pin_1
#define ADC_EXTERN1_GPIO_PORT               GPIOC
#define ADC_EXTERN1_GPIO_CLK                RCC_AHB1Periph_GPIOC
#define ADC_EXTERN1_CHAN                    ADC_Channel_11

#define ADC_EXTERN2_PIN                     GPIO_Pin_0
#define ADC_EXTERN2_GPIO_PORT               GPIOC
#define ADC_EXTERN2_GPIO_CLK                RCC_AHB1Periph_GPIOC
#define ADC_EXTERN2_CHAN                    ADC_Channel_10

#define ADC_MAIN_POWER_PIN                  GPIO_Pin_10
#define ADC_MAIN_POWER_GPIO_PORT            GPIOF
#define ADC_MAIN_POWER_GPIO_CLK             RCC_AHB1Periph_GPIOF
#define ADC_MAIN_POWER_CHAN                 ADC_Channel_8

#define ADC_VBATIN_PIN                       GPIO_Pin_8
#define ADC_VBATIN_GPIO_PORT                 GPIOF
#define ADC_VBATIN_GPIO_CLK                  RCC_AHB1Periph_GPIOF
#define ADC_VBATIN_CHAN                      ADC_Channel_6

#define ADC_IO_OUT_PIN                       GPIO_Pin_9
#define ADC_IO_OUT_GPIO_PORT                 GPIOF
#define ADC_IO_OUT_GPIO_CLK                  RCC_AHB1Periph_GPIOF
#define ADC_IO_OUT_CHAN                      ADC_Channel_7

#define ADC_MIC_PIN                          GPIO_Pin_7
#define ADC_MIC_GPIO_PORT                    GPIOF
#define ADC_MIC_GPIO_CLK                     RCC_AHB1Periph_GPIOF
#define ADC_MIC_CHAN                         ADC_Channel_5

#define ADC_KEY_PIN                          GPIO_Pin_6
#define ADC_KEY_GPIO_PORT                    GPIOF
#define ADC_KEY_GPIO_CLK                     RCC_AHB1Periph_GPIOF
#define ADC_KEY_CHAN                         ADC_Channel_4

#define ADC_PRINTF_PIN                        GPIO_Pin_5
#define ADC_PRINTF_GPIO_PORT                  GPIOF
#define ADC_PRINTF_GPIO_CLK                   RCC_AHB1Periph_GPIOF
#define ADC_PRINTF_CHAN                       ADC_Channel_15

#define ADC_PRINTF_THE_PIN                     GPIO_Pin_4
#define ADC_PRINTF_THE_GPIO_PORT               GPIOF
#define ADC_PRINTF_THE_GPIO_CLK                RCC_AHB1Periph_GPIOF
#define ADC_PRINTF_THE_CHAN                    ADC_Channel_14

//************结构体类型******************
//采集各种电压定义
typedef enum {
  ADC_EXTERN1=0,  //外部电压1,PC1,ADC3_IN11
  ADC_EXTERN2,    //外部电压2,PC0,ADC3_IN10
  ADC_MAIN_POWER, //主电压,PF10,ADC3_IN8
  ADC_VBATIN,     //备电压采集,PF8,ADC3_IN6
  ADC_IO_OUT,     //IO输出短路检测,PF9,ADC3_IN7
  ADC_MIC,        //麦克风检测,PF7,ADC3_IN5
  ADC_KEY,        //键盘检测,PF6,ADC3_IN4,
  ADC_PRINTF,     //打印机检测,PF5,ADC3_IN15
  ADC_PRINTF_THE, //打印机温度检测,PF4,ADC3_IN14
  ADC_CHAN_MAX,   //ADC通道个数
}ADC_CHAN_TYPE;

//*************函数声明**************
/*********************************************************************
//函数名称	:Ad_Init
//功能		:AD初始化
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void Ad_Init(void);
/*********************************************************************
//函数名称	:ADC3_Config
//功能		:AD初始化
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
 void Ad_Config(void);
 /*********************************************************************
//函数名称	:Ad_Dma_Config
//功能		:与AD采样相关的DMA配置
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void Ad_Dma_Config(void);
/*********************************************************************
//函数名称	:Ad_Gpio_Config
//功能		:AD的IO初始化
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void Ad_Gpio_Config(u8 IoNum);
/*********************************************************************
//函数名称	:Ad_GetValue
//功能		:根据AD编号,获得当前AD值
//输入		:adNum AD编号,参考ADC_CHAN_TYPE枚举.
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:当前采集到的AD值.
//备注		:
*********************************************************************/
u16 Ad_GetValue(u8 adNum);

#endif

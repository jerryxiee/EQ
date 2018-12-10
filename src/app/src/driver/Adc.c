/******************************************************************** 
//版权说明	:Shenzhen E-EYE Co. Ltd., Copyright 2009- All Rights Reserved.
//文件名称	:Ad.c		
//功能		:AD信号采集
//版本号	:
//开发人	:dxl
//开发时间	:2012.6
//修改者	:
//修改时间	:
//修改简要说明	:
//备注		:1)
//              :采集值和实际值不准解决办法:
//              :a.直接使用电压表测量采集点的电压值。
//              :b.不进行平均采样，看直接采集值是多少。
//              :ADC_KEY相关键值:
//              :  ADC_key0=20K/(20K+10K)*4095=2730;
//              :  ADC_key1=10K/(10K+10K)*4095=2047;
//              :  ADC_key2=5.1K/(5.1K+10K)*4095=1383;
//              :  ADC_key3=2K/(2K+10K)*4095=682;
***********************************************************************/
//***************包含文件*****************
#include "HAL.h"

//****************宏定义****************


//***************本地变量******************
  __IO u16	ADCDualConvertedValue[ADC_CHAN_MAX];

//
//ADC采集因引脚配置,排列顺序必须ADC_CHAN_TYPE定义相同
//
const uint16_t ADC_IN_PIN[ADC_CHAN_MAX] = {
                                              ADC_EXTERN1_PIN,
                                              ADC_EXTERN2_PIN,
                                              ADC_MAIN_POWER_PIN,
                                              ADC_VBATIN_PIN,
                                              ADC_IO_OUT_PIN,
                                              ADC_MIC_PIN,
                                              ADC_KEY_PIN,
                                              ADC_PRINTF_PIN,
                                              ADC_PRINTF_THE_PIN,
                                           };
GPIO_TypeDef* ADC_IN_PORT[ADC_CHAN_MAX] = {
                                              ADC_EXTERN1_GPIO_PORT,
                                              ADC_EXTERN2_GPIO_PORT,
                                              ADC_MAIN_POWER_GPIO_PORT,
                                              ADC_VBATIN_GPIO_PORT,
                                              ADC_IO_OUT_GPIO_PORT,
                                              ADC_MIC_GPIO_PORT,
                                              ADC_KEY_GPIO_PORT,
                                              ADC_PRINTF_GPIO_PORT,
                                              ADC_PRINTF_THE_GPIO_PORT,
                                            };
const uint32_t ADC_IN_CLK[ADC_CHAN_MAX] = {
                                               ADC_EXTERN1_GPIO_CLK,
                                               ADC_EXTERN2_GPIO_CLK,
                                               ADC_MAIN_POWER_GPIO_CLK,
                                               ADC_VBATIN_GPIO_CLK,
                                               ADC_IO_OUT_GPIO_CLK,
                                               ADC_MIC_GPIO_CLK,                                               
                                               ADC_KEY_GPIO_CLK,
                                               ADC_PRINTF_GPIO_CLK,
                                               ADC_PRINTF_THE_GPIO_CLK,
                                           };
const uint8_t ADC_CHAN[ADC_CHAN_MAX] = {
                                               ADC_EXTERN1_CHAN,
                                               ADC_EXTERN2_CHAN,
                                               ADC_MAIN_POWER_CHAN,
                                               ADC_VBATIN_CHAN,
                                               ADC_IO_OUT_CHAN,
                                               ADC_MIC_CHAN,
                                               ADC_KEY_CHAN,
                                               ADC_PRINTF_CHAN,
                                               ADC_PRINTF_THE_CHAN,
                                       };
//***************函数定义******************
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
void Ad_Init(void)
{
        u8 i = 0;
	//使能时钟
	RCC_AHB1PeriphClockCmd(ADC_DMA_CLOCK, ENABLE);  //RCC_APB2Periph_ADC3   , ADC_CLOCK
  	RCC_APB2PeriphClockCmd(ADC_CLOCK, ENABLE);
	//DMA初始化
	Ad_Dma_Config();
	//AD相关的IO初始化
        for(i = 0 ; i < ADC_CHAN_MAX ; i++)
        {
	    Ad_Gpio_Config(i);
        }
	//ADC初始化
        Ad_Config();
}
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
void Ad_Config(void)
{
  u8 i = 0;
  ADC_InitTypeDef ADC_InitStructure;
  ADC_CommonInitTypeDef ADC_CommonInitStructure;
  memset(&ADC_InitStructure,0,sizeof(ADC_InitTypeDef));
  memset(&ADC_CommonInitStructure,0,sizeof(ADC_CommonInitTypeDef));
  
  /* ADC 通道初始化 */
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles; 
  ADC_CommonInit(&ADC_CommonInitStructure); 

  /* ADC 配置 ------------------------------------------------------*/
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None; 
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1; 
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfConversion = ADC_CHAN_MAX;
  ADC_Init(EVAL_ADC, &ADC_InitStructure);

  /* ADC采集通道顺序设置 */
  for(i = 0 ; i < ADC_CHAN_MAX ; i++)
  {
      ADC_RegularChannelConfig(EVAL_ADC, ADC_CHAN[i], (i+1), ADC_SampleTime_3Cycles);
  }
  
  /* 启用ADC的DMA模式 */
  ADC_DMARequestAfterLastTransferCmd(EVAL_ADC, ENABLE);
  
  /* 启用ADC的DMA模式 */
  ADC_DMACmd(EVAL_ADC, ENABLE);
  
  /* 使能 ADC */
  ADC_Cmd(EVAL_ADC, ENABLE);

  /* 启动ADC的软件转换 */ 
  ADC_SoftwareStartConv(EVAL_ADC);
}

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
void Ad_Dma_Config(void)
{
  	DMA_InitTypeDef DMA_InitStructure;
        memset(&DMA_InitStructure,0,sizeof(DMA_InitTypeDef));
        
  	DMA_InitStructure.DMA_Channel = ADC_DMA_CHAN;          
  	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&ADCDualConvertedValue;
  	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC_DR_ADDRESS;
  	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = ADC_CHAN_MAX;
  	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;         
  	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  	DMA_Init(ADC_DMA_STREAM, &DMA_InitStructure);

  	/* DMA2_Stream0 enable */
  	DMA_Cmd(ADC_DMA_STREAM, ENABLE);
}

/*********************************************************************
//函数名称	:Ad_Gpio_Config
//功能		:AD的IO初始化
//输入		:IoNum 引脚编号
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void Ad_Gpio_Config(u8 IoNum)
{
  	GPIO_InitTypeDef GPIO_InitStructure;
        memset(&GPIO_InitStructure,0,sizeof(GPIO_InitTypeDef));
        
         RCC_AHB1PeriphClockCmd(ADC_IN_CLK[IoNum], ENABLE);
	GPIO_InitStructure.GPIO_Pin = ADC_IN_PIN[IoNum];
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  	GPIO_Init(ADC_IN_PORT[IoNum], &GPIO_InitStructure);   
}
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
u16 Ad_GetValue(u8 adNum)
{
    u16 tmp = 0;
    tmp = ADCDualConvertedValue[adNum];
    return tmp;
}
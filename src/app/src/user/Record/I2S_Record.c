/**
  ******************************************************************************
  * @file    I2S_Record.c 
  * @author  
  * @version 
  * @date    
  * @brief   I2S和DMA配置
  *               
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "stm32f2xx_lib.h"  
#include "include.h"
#include "GPIOControl.h"
#include "I2S_Record.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern uint32_t AUDIO_SAMPLE_LEN ;
extern uint16_t AUDIO_SAMPLE[] ;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  初始放音接口 (I2S)
  * @note   使用本函数前，必须先配置好I2S时钟。   
  * @param  AudioFreq: 音频采样频率 
  * @retval None
  */
void I2S_Play_Init(uint32_t AudioFreq)
{
  I2S_InitTypeDef I2S_InitStructure;
  memset((void *)&I2S_InitStructure,0,sizeof(I2S_InitTypeDef)); 
  
  /* 使能I2S时钟 */
  RCC_APB1PeriphClockCmd(CODEC_I2S_CLK, ENABLE);

  /* 恢复I2S默认配置 */
  SPI_I2S_DeInit(CODEC_I2S);
  I2S_InitStructure.I2S_AudioFreq = AudioFreq;
  I2S_InitStructure.I2S_Standard = I2S_Standard_LSB;         //录音必须是LSB格式-由ak4554 IC决定
  I2S_InitStructure.I2S_DataFormat = I2S_DataFormat_16b;
  I2S_InitStructure.I2S_CPOL = I2S_CPOL_Low;
  I2S_InitStructure.I2S_Mode = I2S_Mode_MasterTx;            //I2S_Mode_MasterRx-主设备接收,I2S_Mode_MasterTx-主设备发送
  I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Enable;  //使能MCK引脚

  /* 根据初始参数初始I2S */
  I2S_Init(CODEC_I2S, &I2S_InitStructure);
  
  /* I2S使能DMA发送 */
  SPI_I2S_DMACmd(CODEC_I2S,SPI_I2S_DMAReq_Tx,ENABLE);
  
   /* DMA配置 */
  Audio_DMA_Play();
    
  /* 使能 SPI2/I2S2 外设 */
  I2S_Cmd(CODEC_I2S, ENABLE);
        
  /* 使能DMA发送*/
  DMA_Cmd(AUDIO_MAL_DMA_STREAM, ENABLE);
  
}

/**
  * @brief  将音频接口恢复到默认状态
  * @param  None
  * @retval None
  */
void Codec_AudioInterface_DeInit(void)
{
 
  /* 关闭DMA接口 */
  DMA_Cmd(AUDIO_MAL_DMA_STREAM, DISABLE);
  DMA_Cmd(AUDIO_REC_DMA_STREAM, DISABLE);
  RCC_AHB1PeriphClockCmd(AUDIO_MAL_DMA_CLOCK, DISABLE);
  RCC_AHB1PeriphClockCmd(AUDIO_REC_DMA_CLOCK, DISABLE);
  
  /* 关闭音频接口 */
  I2S_Cmd(CODEC_I2S, DISABLE);
  
  /* 恢复音频接口默认参数 */
  SPI_I2S_DeInit(CODEC_I2S);
  
  /* 关闭I2S时钟 */
  RCC_APB1PeriphClockCmd(CODEC_I2S_CLK, DISABLE); 
}

/**
  * @brief  初始录音接口 (I2S)
  * @note   使用本函数前，必须先配置好I2S时钟。   
  * @param  AudioFreq: 音频采样频率 
  * @retval None
  */
void I2S_Record_Init(uint32_t AudioFreq)
{
  I2S_InitTypeDef I2S_InitStructure;
  memset((void *)&I2S_InitStructure,0,sizeof(I2S_InitTypeDef));

  /* 使能I2S时钟 */
  RCC_APB1PeriphClockCmd(CODEC_I2S_CLK, ENABLE);

  /* 恢复I2S默认配置 */
  SPI_I2S_DeInit(CODEC_I2S);
  I2S_InitStructure.I2S_AudioFreq = AudioFreq;
  I2S_InitStructure.I2S_Standard = I2S_Standard_MSB;          //录音必须是MSB格式
  I2S_InitStructure.I2S_DataFormat = I2S_DataFormat_16b;
  I2S_InitStructure.I2S_CPOL = I2S_CPOL_Low;
  I2S_InitStructure.I2S_Mode = I2S_Mode_MasterRx;            //I2S_Mode_MasterRx-主设备接收,I2S_Mode_MasterTx-主设备发送
  I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Enable;  //使能MCK引脚

  /* 根据初始参数初始I2S */
  I2S_Init(CODEC_I2S, &I2S_InitStructure);
  
    /* I2S使能DMA接收 */
  SPI_I2S_DMACmd(CODEC_I2S,SPI_I2S_DMAReq_Rx,ENABLE);
  
  /* DMA配置 */
  Audio_DMA_REC();
  
  /* 使能 SPI2/I2S2 外设 */
  I2S_Cmd(CODEC_I2S, ENABLE);
  
  /* 使能DMA发送*/
  DMA_Cmd(AUDIO_REC_DMA_STREAM, ENABLE);  

}

/**
  * @brief  初始音频接口使用的IO口.
  * @param  None
  * @retval None
  */
void I2S_Audio_GPIO_Init(void)
{
   GPIO_InitTypeDef GPIO_InitStructure;
   memset((void *)&GPIO_InitStructure,0,sizeof(GPIO_InitTypeDef));
   
  /* Enable I2S and I2C GPIO clocks */
  RCC_AHB1PeriphClockCmd(CODEC_I2S_GPIO_CLOCK, ENABLE);

  /* CODEC_I2S pins configuration: WS, SCK and SD pins -----------------------------*/
  GPIO_InitStructure.GPIO_Pin = CODEC_I2S_WS_PIN | CODEC_I2S_SCK_PIN | CODEC_I2S_SD_PIN; 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(CODEC_I2S_GPIO, &GPIO_InitStructure);     
  /* Connect pins to I2S peripheral  */
  GPIO_PinAFConfig(CODEC_I2S_GPIO, CODEC_I2S_WS_PINSRC, CODEC_I2S_GPIO_AF);  
  GPIO_PinAFConfig(CODEC_I2S_GPIO, CODEC_I2S_SCK_PINSRC, CODEC_I2S_GPIO_AF);
  GPIO_PinAFConfig(CODEC_I2S_GPIO, CODEC_I2S_SD_PINSRC, CODEC_I2S_GPIO_AF);

  /* CODEC_I2S pins configuration: MCK pin */
  GPIO_InitStructure.GPIO_Pin = CODEC_I2S_MCK_PIN; 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(CODEC_I2S_MCK_GPIO, &GPIO_InitStructure);   
  /* Connect pins to I2S peripheral  */
  GPIO_PinAFConfig(CODEC_I2S_MCK_GPIO, CODEC_I2S_MCK_PINSRC, CODEC_I2S_GPIO_AF);   
}
/**
  * @brief  初始音频控制IO脚
  * @param  None
  * @retval None
  */
void I2S_Audio_Control_GPIO_Init(void)
{
  //放音
  GpioOutInit(I2S_PWDAN);
  GpioOutInit(I2S_AD_DA);
  GpioOutInit(V_COL);
  
  //录音
  GpioOutInit(I2S_PWADN);
  //GpioOutInit(I2S_MIC_AMP);dxl,由于原理图没有找到MIC_AMP所以屏蔽掉
  //GPIOControlInit(I2S_V_BASE);
  
}

/**
  * @brief  打开I2S中断.
  * @retval None
  */
void SPI2_NVIC_Config(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  memset((void *)&NVIC_InitStructure,0,sizeof(NVIC_InitTypeDef));   /** 局部变量设置,必须先清除为0.*/
  
  /* 启用USPI2中断*/
  NVIC_InitStructure.NVIC_IRQChannel = CODEC_I2S_IRQ;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

/**
  * @brief  音频播放初始化.
  * @param  None
  * @retval None
  */
void Audio_Play_Init(void)
{
  //I2S使能脚
  
  GpioOutOn(I2S_PWDAN);  
  GpioOutOff(I2S_PWADN);   
  
  GpioOutOn(I2S_AD_DA);    
  GpioOutOn(V_COL);
  
  //GpioOutOff(I2S_MIC_AMP);dxl,由于原理图没有找到MIC_AMP所以屏蔽掉
  //GPIOControlOff(I2S_V_BASE);   
  Io_WriteMicConrtolBit(MIC_BIT_RECORD, RESET);
  
  Codec_AudioInterface_DeInit();
//  SPI2_NVIC_Config();
  I2S_Audio_GPIO_Init();
  I2S_Play_Init(8000);           //8000,48000播放测试  
}

/**
  * @brief  音频录音初始化.
  * @param  None
  * @retval None
  */
void Audio_Record_Init(void)
{
  //I2S使能脚

  GpioOutOff(I2S_PWDAN);  
  GpioOutOn(I2S_PWADN);
  
  GpioOutOff(I2S_AD_DA); 
  
  //GPIOControlOn(I2S_MIC_AMP);dxl,因为原理图没有找到MIC_AMP所以屏蔽掉
  //GPIOControlOn(I2S_V_BASE);  
  Io_WriteMicConrtolBit(MIC_BIT_RECORD, SET);
     
  //GPIOControlOff(I2S_C_VOL);  //扬声器,因为电话和TTS语音都要使用，这里不能控制关闭。
  

  Codec_AudioInterface_DeInit();

  I2S_Audio_GPIO_Init();
  I2S_Record_Init(8000);          //8000,48000播放测试  
}
/**
  * @brief  音频录音关闭.
  * @param  None
  * @retval None
  */
void Audio_Record_Close(void)
{
  /*关闭麦克风控制IO脚*/
  //GPIOControlOff(I2S_MIC_AMP);           //与电话打开麦克风冲突,dxl,由于原理图没有找到MIC_AMP所以屏蔽掉
  //GPIOControlOff(I2S_V_BASE); 
  Io_WriteMicConrtolBit(MIC_BIT_RECORD, RESET);
  Codec_AudioInterface_DeInit();
}

/**
  * @brief  设置DMA方式播放音频流
  * @param  None
  * @retval None
  */
void Audio_DMA_Play(void)  //uint32_t Addr, uint32_t Size
{   
  DMA_InitTypeDef    DMA_InitStructure; 
  NVIC_InitTypeDef   NVIC_InitStructure; 
  memset((void *)&DMA_InitStructure,0,sizeof(DMA_InitTypeDef));
  memset((void *)&NVIC_InitStructure,0,sizeof(NVIC_InitTypeDef));
  
  /*配置DMA中断*/
  NVIC_InitStructure.NVIC_IRQChannel = AUDIO_MAL_DMA_IRQ;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* 使能DMA时钟 */
  RCC_AHB1PeriphClockCmd(AUDIO_MAL_DMA_CLOCK, ENABLE);
 
  /* USART1 发送_DMA通道 配置 */
  DMA_DeInit(AUDIO_MAL_DMA_STREAM);
  DMA_InitStructure.DMA_Channel = AUDIO_MAL_DMA_CHANNEL;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)AUDIO_DR_Base;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)AUDIO_SAMPLE;
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  DMA_InitStructure.DMA_BufferSize = DMA_MAX_SZE;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = AUDIO_MAL_DMA_PERIPH_DATA_SIZE;
  DMA_InitStructure.DMA_MemoryDataSize = AUDIO_MAL_DMA_MEM_DATA_SIZE;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
  DMA_Init(AUDIO_MAL_DMA_STREAM, &DMA_InitStructure);
  
  DMA_ITConfig(AUDIO_MAL_DMA_STREAM,DMA_IT_TC,ENABLE);         /** 设置传输完成中断*/
  
}

/**
  * @brief  设置DMA方式播放音频流
  * @param  None
  * @retval None
  */
void Audio_DMA_REC(void)
{   
  DMA_InitTypeDef    DMA_InitStructure; 
  NVIC_InitTypeDef   NVIC_InitStructure; 
  memset((void *)&DMA_InitStructure,0,sizeof(DMA_InitTypeDef));
  memset((void *)&NVIC_InitStructure,0,sizeof(NVIC_InitTypeDef));
  
  /*配置DMA中断*/
  NVIC_InitStructure.NVIC_IRQChannel = AUDIO_REC_DMA_IRQ;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* 使能DMA时钟 */
  RCC_AHB1PeriphClockCmd(AUDIO_REC_DMA_CLOCK, ENABLE);
 
  /* USART1 发送_DMA通道 配置 */
  DMA_DeInit(AUDIO_REC_DMA_STREAM);
  DMA_InitStructure.DMA_Channel = AUDIO_REC_DMA_CHANNEL;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)AUDIO_DR_Base;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)AUDIO_SAMPLE;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;                       //从外设到内存
  DMA_InitStructure.DMA_BufferSize = DMA_MAX_SZE;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = AUDIO_MAL_DMA_PERIPH_DATA_SIZE;
  DMA_InitStructure.DMA_MemoryDataSize = AUDIO_MAL_DMA_MEM_DATA_SIZE;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
  DMA_Init(AUDIO_REC_DMA_STREAM, &DMA_InitStructure);
  
  DMA_ITConfig(AUDIO_REC_DMA_STREAM,DMA_IT_TC,ENABLE);                          /** 设置传输完成中断*/
  
  DMA_ITConfig(AUDIO_REC_DMA_STREAM,DMA_IT_HT,ENABLE);                          /** 设置传输半完成中断*/

}
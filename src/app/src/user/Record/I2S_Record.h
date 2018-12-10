/**
  ******************************************************************************
  * @file    I2S_Record.h
  * @author  
  * @version 
  * @date    
  * @brief   I2S���ú�ʹ�ú���
  *          PB12->I2S_WS
  *          PB13->I2s_CK
  *          PB15->I2S_SD
  *          PC6 ->I2S_MCK
  *          MCKʱ�Ӽ���,�����8K������,OSC_IN=8M����:
  *          8m->/m(8)->1M->PLL2 VCO->*N(256)->/R(5)->51200KHZ
  *          8Khz= 51200KHz/(((16*2)*(2*I2SDIV)+ODD)*8)
  *          ͨ����ѯ�ֲ�:I2SDIV=12,ODD=1,ͨ�����湫ʽ��֤����Ϊ������
  *               
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/

#ifndef __I2S_RECORD_H
#define __I2S_RECORD_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f2xx_spi.h"

#ifdef __cplusplus
 extern "C" {
#endif 
 
/* I2S �������ö���  */
#define CODEC_I2S                      SPI2
#define CODEC_I2S_CLK                  RCC_APB1Periph_SPI2
#define CODEC_I2S_ADDRESS              0x4000380C
#define CODEC_I2S_GPIO_AF              GPIO_AF_SPI2
#define CODEC_I2S_IRQ                  SPI2_IRQn
#define CODEC_I2S_GPIO_CLOCK           (RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOB)
#define CODEC_I2S_WS_PIN               GPIO_Pin_9
#define CODEC_I2S_SCK_PIN              GPIO_Pin_13
#define CODEC_I2S_SD_PIN               GPIO_Pin_15
#define CODEC_I2S_MCK_PIN              GPIO_Pin_6
#define CODEC_I2S_WS_PINSRC            GPIO_PinSource9
#define CODEC_I2S_SCK_PINSRC           GPIO_PinSource13
#define CODEC_I2S_SD_PINSRC            GPIO_PinSource15
#define CODEC_I2S_MCK_PINSRC           GPIO_PinSource6
#define CODEC_I2S_GPIO                 GPIOB
#define CODEC_I2S_MCK_GPIO             GPIOC

 /* I2S DMA ������ */
#define AUDIO_DR_Base                 &(SPI2->DR)               //ȡ���ݵ�ַ
#define AUDIO_MAL_DMA_CLOCK            RCC_AHB1Periph_DMA1
#define AUDIO_MAL_DMA_STREAM           DMA1_Stream4             //ʹ��DMA1��4
#define AUDIO_MAL_DMA_CHANNEL          DMA_Channel_0            //ͨ��0��SPI2_TX
#define AUDIO_MAL_DMA_IRQ              DMA1_Stream4_IRQn        //ͨ��4�ж�
#define Audio_MAL_IRQHandler           DMA1_Stream4_IRQHandler  
#define AUDIO_MAL_DMA_FLAG_TC          DMA_FLAG_TCIF4           //��ɱ�־
#define AUDIO_MAL_DMA_FLAG_HT          DMA_FLAG_HTIF4           //����ɱ�־
#define AUDIO_MAL_DMA_FLAG_TE          DMA_FLAG_TEIF4           //�����־
#define AUDIO_MAL_DMA_FLAG_DME         DMA_FLAG_DMEIF4          //ֱ��ģʽ�µĴ����־
#define AUDIO_MAL_DMA_FLAG_FE          DMA_FLAG_FEIF4           //FIFOģʽ�µĴ����־

#define AUDIO_MAL_DMA_PERIPH_DATA_SIZE DMA_PeripheralDataSize_HalfWord  //�������ݿ��-16λ
#define AUDIO_MAL_DMA_MEM_DATA_SIZE    DMA_MemoryDataSize_HalfWord      //�洢�����ݿ��-16λ
#define DMA_MAX_SZE                    4000                            //DMA���ݴ�С(����ʹ����DMA��֧�ֵ����ֵ)-0xFFFF,16000/8000
   
#define AUDIO_REC_DMA_CLOCK            RCC_AHB1Periph_DMA1
#define AUDIO_REC_DMA_STREAM           DMA1_Stream3             //ʹ��DMA1��4
#define AUDIO_REC_DMA_CHANNEL          DMA_Channel_0            //ͨ��0��SPI2_TX
#define AUDIO_REC_DMA_IRQ              DMA1_Stream3_IRQn        //ͨ��4�ж�
#define Audio_REC_IRQHandler           DMA1_Stream3_IRQHandler  
#define AUDIO_REC_DMA_FLAG_TC          DMA_FLAG_TCIF3           //��ɱ�־
#define AUDIO_REC_DMA_FLAG_HT          DMA_FLAG_HTIF3           //����ɱ�־
#define AUDIO_REC_DMA_FLAG_TE          DMA_FLAG_TEIF3           //�����־
#define AUDIO_REC_DMA_FLAG_DME         DMA_FLAG_DMEIF3          //ֱ��ģʽ�µĴ����־
#define AUDIO_REC_DMA_FLAG_FE          DMA_FLAG_FEIF3           //FIFOģʽ�µĴ����־   

  
/*I2S CFGR�Ĵ�����ENλ */
#define I2S_ENABLE_MASK                 0x0400   

void I2S_Play_Init(uint32_t AudioFreq);
void Codec_AudioInterface_DeInit(void);
void I2S_Record_Init(uint32_t AudioFreq);
void I2S_Audio_GPIO_Init(void);
void I2S_Audio_Control_GPIO_Init(void);
void SPI2_NVIC_Config(void);
void Audio_Play_Init(void);
void Audio_Record_Init(void);
void Audio_Record_Close(void);
void Audio_DMA_Play(void);
void Audio_DMA_REC(void);

#ifdef __cplusplus
}
#endif   

#endif /*  */
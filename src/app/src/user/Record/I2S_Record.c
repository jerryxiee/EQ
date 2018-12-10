/**
  ******************************************************************************
  * @file    I2S_Record.c 
  * @author  
  * @version 
  * @date    
  * @brief   I2S��DMA����
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
  * @brief  ��ʼ�����ӿ� (I2S)
  * @note   ʹ�ñ�����ǰ�����������ú�I2Sʱ�ӡ�   
  * @param  AudioFreq: ��Ƶ����Ƶ�� 
  * @retval None
  */
void I2S_Play_Init(uint32_t AudioFreq)
{
  I2S_InitTypeDef I2S_InitStructure;
  memset((void *)&I2S_InitStructure,0,sizeof(I2S_InitTypeDef)); 
  
  /* ʹ��I2Sʱ�� */
  RCC_APB1PeriphClockCmd(CODEC_I2S_CLK, ENABLE);

  /* �ָ�I2SĬ������ */
  SPI_I2S_DeInit(CODEC_I2S);
  I2S_InitStructure.I2S_AudioFreq = AudioFreq;
  I2S_InitStructure.I2S_Standard = I2S_Standard_LSB;         //¼��������LSB��ʽ-��ak4554 IC����
  I2S_InitStructure.I2S_DataFormat = I2S_DataFormat_16b;
  I2S_InitStructure.I2S_CPOL = I2S_CPOL_Low;
  I2S_InitStructure.I2S_Mode = I2S_Mode_MasterTx;            //I2S_Mode_MasterRx-���豸����,I2S_Mode_MasterTx-���豸����
  I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Enable;  //ʹ��MCK����

  /* ���ݳ�ʼ������ʼI2S */
  I2S_Init(CODEC_I2S, &I2S_InitStructure);
  
  /* I2Sʹ��DMA���� */
  SPI_I2S_DMACmd(CODEC_I2S,SPI_I2S_DMAReq_Tx,ENABLE);
  
   /* DMA���� */
  Audio_DMA_Play();
    
  /* ʹ�� SPI2/I2S2 ���� */
  I2S_Cmd(CODEC_I2S, ENABLE);
        
  /* ʹ��DMA����*/
  DMA_Cmd(AUDIO_MAL_DMA_STREAM, ENABLE);
  
}

/**
  * @brief  ����Ƶ�ӿڻָ���Ĭ��״̬
  * @param  None
  * @retval None
  */
void Codec_AudioInterface_DeInit(void)
{
 
  /* �ر�DMA�ӿ� */
  DMA_Cmd(AUDIO_MAL_DMA_STREAM, DISABLE);
  DMA_Cmd(AUDIO_REC_DMA_STREAM, DISABLE);
  RCC_AHB1PeriphClockCmd(AUDIO_MAL_DMA_CLOCK, DISABLE);
  RCC_AHB1PeriphClockCmd(AUDIO_REC_DMA_CLOCK, DISABLE);
  
  /* �ر���Ƶ�ӿ� */
  I2S_Cmd(CODEC_I2S, DISABLE);
  
  /* �ָ���Ƶ�ӿ�Ĭ�ϲ��� */
  SPI_I2S_DeInit(CODEC_I2S);
  
  /* �ر�I2Sʱ�� */
  RCC_APB1PeriphClockCmd(CODEC_I2S_CLK, DISABLE); 
}

/**
  * @brief  ��ʼ¼���ӿ� (I2S)
  * @note   ʹ�ñ�����ǰ�����������ú�I2Sʱ�ӡ�   
  * @param  AudioFreq: ��Ƶ����Ƶ�� 
  * @retval None
  */
void I2S_Record_Init(uint32_t AudioFreq)
{
  I2S_InitTypeDef I2S_InitStructure;
  memset((void *)&I2S_InitStructure,0,sizeof(I2S_InitTypeDef));

  /* ʹ��I2Sʱ�� */
  RCC_APB1PeriphClockCmd(CODEC_I2S_CLK, ENABLE);

  /* �ָ�I2SĬ������ */
  SPI_I2S_DeInit(CODEC_I2S);
  I2S_InitStructure.I2S_AudioFreq = AudioFreq;
  I2S_InitStructure.I2S_Standard = I2S_Standard_MSB;          //¼��������MSB��ʽ
  I2S_InitStructure.I2S_DataFormat = I2S_DataFormat_16b;
  I2S_InitStructure.I2S_CPOL = I2S_CPOL_Low;
  I2S_InitStructure.I2S_Mode = I2S_Mode_MasterRx;            //I2S_Mode_MasterRx-���豸����,I2S_Mode_MasterTx-���豸����
  I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Enable;  //ʹ��MCK����

  /* ���ݳ�ʼ������ʼI2S */
  I2S_Init(CODEC_I2S, &I2S_InitStructure);
  
    /* I2Sʹ��DMA���� */
  SPI_I2S_DMACmd(CODEC_I2S,SPI_I2S_DMAReq_Rx,ENABLE);
  
  /* DMA���� */
  Audio_DMA_REC();
  
  /* ʹ�� SPI2/I2S2 ���� */
  I2S_Cmd(CODEC_I2S, ENABLE);
  
  /* ʹ��DMA����*/
  DMA_Cmd(AUDIO_REC_DMA_STREAM, ENABLE);  

}

/**
  * @brief  ��ʼ��Ƶ�ӿ�ʹ�õ�IO��.
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
  * @brief  ��ʼ��Ƶ����IO��
  * @param  None
  * @retval None
  */
void I2S_Audio_Control_GPIO_Init(void)
{
  //����
  GpioOutInit(I2S_PWDAN);
  GpioOutInit(I2S_AD_DA);
  GpioOutInit(V_COL);
  
  //¼��
  GpioOutInit(I2S_PWADN);
  //GpioOutInit(I2S_MIC_AMP);dxl,����ԭ��ͼû���ҵ�MIC_AMP�������ε�
  //GPIOControlInit(I2S_V_BASE);
  
}

/**
  * @brief  ��I2S�ж�.
  * @retval None
  */
void SPI2_NVIC_Config(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  memset((void *)&NVIC_InitStructure,0,sizeof(NVIC_InitTypeDef));   /** �ֲ���������,���������Ϊ0.*/
  
  /* ����USPI2�ж�*/
  NVIC_InitStructure.NVIC_IRQChannel = CODEC_I2S_IRQ;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

/**
  * @brief  ��Ƶ���ų�ʼ��.
  * @param  None
  * @retval None
  */
void Audio_Play_Init(void)
{
  //I2Sʹ�ܽ�
  
  GpioOutOn(I2S_PWDAN);  
  GpioOutOff(I2S_PWADN);   
  
  GpioOutOn(I2S_AD_DA);    
  GpioOutOn(V_COL);
  
  //GpioOutOff(I2S_MIC_AMP);dxl,����ԭ��ͼû���ҵ�MIC_AMP�������ε�
  //GPIOControlOff(I2S_V_BASE);   
  Io_WriteMicConrtolBit(MIC_BIT_RECORD, RESET);
  
  Codec_AudioInterface_DeInit();
//  SPI2_NVIC_Config();
  I2S_Audio_GPIO_Init();
  I2S_Play_Init(8000);           //8000,48000���Ų���  
}

/**
  * @brief  ��Ƶ¼����ʼ��.
  * @param  None
  * @retval None
  */
void Audio_Record_Init(void)
{
  //I2Sʹ�ܽ�

  GpioOutOff(I2S_PWDAN);  
  GpioOutOn(I2S_PWADN);
  
  GpioOutOff(I2S_AD_DA); 
  
  //GPIOControlOn(I2S_MIC_AMP);dxl,��Ϊԭ��ͼû���ҵ�MIC_AMP�������ε�
  //GPIOControlOn(I2S_V_BASE);  
  Io_WriteMicConrtolBit(MIC_BIT_RECORD, SET);
     
  //GPIOControlOff(I2S_C_VOL);  //������,��Ϊ�绰��TTS������Ҫʹ�ã����ﲻ�ܿ��ƹرա�
  

  Codec_AudioInterface_DeInit();

  I2S_Audio_GPIO_Init();
  I2S_Record_Init(8000);          //8000,48000���Ų���  
}
/**
  * @brief  ��Ƶ¼���ر�.
  * @param  None
  * @retval None
  */
void Audio_Record_Close(void)
{
  /*�ر���˷����IO��*/
  //GPIOControlOff(I2S_MIC_AMP);           //��绰����˷��ͻ,dxl,����ԭ��ͼû���ҵ�MIC_AMP�������ε�
  //GPIOControlOff(I2S_V_BASE); 
  Io_WriteMicConrtolBit(MIC_BIT_RECORD, RESET);
  Codec_AudioInterface_DeInit();
}

/**
  * @brief  ����DMA��ʽ������Ƶ��
  * @param  None
  * @retval None
  */
void Audio_DMA_Play(void)  //uint32_t Addr, uint32_t Size
{   
  DMA_InitTypeDef    DMA_InitStructure; 
  NVIC_InitTypeDef   NVIC_InitStructure; 
  memset((void *)&DMA_InitStructure,0,sizeof(DMA_InitTypeDef));
  memset((void *)&NVIC_InitStructure,0,sizeof(NVIC_InitTypeDef));
  
  /*����DMA�ж�*/
  NVIC_InitStructure.NVIC_IRQChannel = AUDIO_MAL_DMA_IRQ;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* ʹ��DMAʱ�� */
  RCC_AHB1PeriphClockCmd(AUDIO_MAL_DMA_CLOCK, ENABLE);
 
  /* USART1 ����_DMAͨ�� ���� */
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
  
  DMA_ITConfig(AUDIO_MAL_DMA_STREAM,DMA_IT_TC,ENABLE);         /** ���ô�������ж�*/
  
}

/**
  * @brief  ����DMA��ʽ������Ƶ��
  * @param  None
  * @retval None
  */
void Audio_DMA_REC(void)
{   
  DMA_InitTypeDef    DMA_InitStructure; 
  NVIC_InitTypeDef   NVIC_InitStructure; 
  memset((void *)&DMA_InitStructure,0,sizeof(DMA_InitTypeDef));
  memset((void *)&NVIC_InitStructure,0,sizeof(NVIC_InitTypeDef));
  
  /*����DMA�ж�*/
  NVIC_InitStructure.NVIC_IRQChannel = AUDIO_REC_DMA_IRQ;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* ʹ��DMAʱ�� */
  RCC_AHB1PeriphClockCmd(AUDIO_REC_DMA_CLOCK, ENABLE);
 
  /* USART1 ����_DMAͨ�� ���� */
  DMA_DeInit(AUDIO_REC_DMA_STREAM);
  DMA_InitStructure.DMA_Channel = AUDIO_REC_DMA_CHANNEL;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)AUDIO_DR_Base;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)AUDIO_SAMPLE;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;                       //�����赽�ڴ�
  DMA_InitStructure.DMA_BufferSize = DMA_MAX_SZE;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = AUDIO_MAL_DMA_PERIPH_DATA_SIZE;
  DMA_InitStructure.DMA_MemoryDataSize = AUDIO_MAL_DMA_MEM_DATA_SIZE;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
  DMA_Init(AUDIO_REC_DMA_STREAM, &DMA_InitStructure);
  
  DMA_ITConfig(AUDIO_REC_DMA_STREAM,DMA_IT_TC,ENABLE);                          /** ���ô�������ж�*/
  
  DMA_ITConfig(AUDIO_REC_DMA_STREAM,DMA_IT_HT,ENABLE);                          /** ���ô��������ж�*/

}
/**
  ******************************************************************************
  * @file    I2S_Record.h
  * @author  
  * @version 
  * @date    
  * @brief   I2S配置和使用函数
  *          PB12->I2S_WS
  *          PB13->I2s_CK
  *          PB15->I2S_SD
  *          PC6 ->I2S_MCK
  *               
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/

#ifndef __G711_H
#define __G711_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f2xx_spi.h"

#ifdef __cplusplus
 extern "C" {
#endif 

static int search(int val,short *table,int size);
unsigned char linear2alaw(int pcm_val);
int alaw2linear(unsigned char a_val);   
int PCM16ConverG711(u8 *buf,uint16_t *pcmBuf,uint32_t len);
int G711ToPCM16(uint16_t *pcmBuf,u8 *g7Buf,uint32_t len);
   
#ifdef __cplusplus
}
#endif   

#endif /*  */   
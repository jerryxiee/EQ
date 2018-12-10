/**
  ******************************************************************************
  * @file    modem_bsp.c 
  * @author  miaoyahan
  * @version V1.0.0
  * @date    2013-08-22
  * @brief   
  ******************************************************************************
  */
/*
********************************************************************************
*                               INCLUDES
********************************************************************************
*/
#ifdef EYE_MODEM
#include "include.h"
#else
#include  <app_cfg.h>
#include  <includes.h>
#endif

/*
********************************************************************************
*                            DEFINES
********************************************************************************
*/


/*
********************************************************************************
*                         LOCAL DATA TYPES
********************************************************************************
*/



/*
********************************************************************************
*                          LOCAL VARIABLES
********************************************************************************
*/


/*
********************************************************************************
*                          GLOBAL VARIABLES
********************************************************************************
*/


/*
********************************************************************************
*                           FUNCTIONS EXTERN
********************************************************************************
*/


/*
********************************************************************************
*                              FUNCTIONS
********************************************************************************
*/

/*
  * @brief  Modem硬件驱动初始化,包括IO口Power、Active、DTR、RING.
  * @param  None
  * @retval None
  */
void  Modem_Bsp_Init (void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    u8 i;
    
    /*!< Enable GPIO clocks */
    RCC_AHB1PeriphClockCmd(Modem_BSP_POWER_GPIO_CLK | Modem_BSP_ACTIVE_GPIO_CLK 
                           | Modem_BSP_DTR_GPIO_CLK | Modem_BSP_RING_GPIO_CLK
                           | Modem_BSP_MIC_GPIO_CLK | Modem_BSP_SPK_GPIO_CLK, ENABLE);
    
    /*!< Configure Modem  Power、ACTIVE、DTR、pin in output pushpull mode ************/  
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    
    GPIO_InitStructure.GPIO_Pin = Modem_BSP_POWER_PIN;////Power
    GPIO_Init(Modem_BSP_POWER_GPIO_PORT , &GPIO_InitStructure); 
    
    GPIO_InitStructure.GPIO_Pin = Modem_BSP_ACTIVE_PIN;////ACTIVE
    GPIO_Init(Modem_BSP_ACTIVE_GPIO_PORT , &GPIO_InitStructure); 
    
    GPIO_InitStructure.GPIO_Pin = Modem_BSP_DTR_PIN;////DTR
    GPIO_Init(Modem_BSP_DTR_GPIO_PORT , &GPIO_InitStructure);    

    GPIO_InitStructure.GPIO_Pin = Modem_BSP_MIC_PIN;////MIC
    GPIO_Init(Modem_BSP_MIC_GPIO_PORT , &GPIO_InitStructure);      

    GPIO_InitStructure.GPIO_Pin = Modem_BSP_SPK_PIN;////SPEAK
    GPIO_Init(Modem_BSP_SPK_GPIO_PORT , &GPIO_InitStructure);   

    /*!< Configure Modem  Ring pin in intput pushpull mode ********************/
    GPIO_InitStructure.GPIO_Pin   = Modem_BSP_RING_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_Init(Modem_BSP_RING_GPIO_PORT, &GPIO_InitStructure);


    for(i=0;i<MODEM_SOC_NUM;i++)
    {
        ModLib_NodInit(&ModSocNode[i],
                       &ModSocNdNum[i*MODEM_SOC_ND_SIZE],MODEM_SOC_ND_SIZE,
                       &ModSocBuf[i*MODEM_SOC_TX_SIZE],MODEM_SOC_TX_SIZE);
    }

}


/**
  * @brief  设置modem的电源,
        注意：经过实际检测：
                ->  M35 放电时间约为25秒，为保险起见建议断电30s;
  * @param  state : 0为关闭、否则打开
  * @retval None
  */

void Modem_Bsp_SetPower(u8 state)
{
    if (0 == state)
    { 
        /* 打开modem电源 */
        GPIO_ResetBits(Modem_BSP_POWER_GPIO_PORT, Modem_BSP_POWER_PIN);
    }
    else
    {
        /* 关闭modem电源 */
        GPIO_SetBits(Modem_BSP_POWER_GPIO_PORT, Modem_BSP_POWER_PIN);
    }
}                           


/**
  * @brief  设置modem激活引脚电平信号
  * @param  state : 0为拉低、否则拉高
  * @retval None
  */

void Modem_Bsp_SetActive(u8 state)
{
    if (0 == state)
    { 
        /* 拉高 激活引脚 电平*/
        GPIO_SetBits(Modem_BSP_ACTIVE_GPIO_PORT, Modem_BSP_ACTIVE_PIN);
    }
    else
    {
         /* 拉低 激活引脚 电平*/
        GPIO_ResetBits(Modem_BSP_ACTIVE_GPIO_PORT, Modem_BSP_ACTIVE_PIN);
    }
}

/**
  * @brief  设置modem DTR引脚电平信号
  * @param  state : 0为拉低、否则拉高
  * @retval None
  */

void Modem_Bsp_SetDTR(u8 state)
{
    if (0 == state)
    { 
        /* 拉低 DTR 电平*/
        GPIO_ResetBits(Modem_BSP_DTR_GPIO_PORT, Modem_BSP_DTR_PIN);
    }
    else
    {
        /* 拉高 DTR 电平 */
        GPIO_SetBits(Modem_BSP_DTR_GPIO_PORT, Modem_BSP_DTR_PIN);
    }
}
/**
  * @brief  读取modem RING引脚电平信号
  * @param  None
  * @retval 返回电平状态
  */
u8 Modem_Bsp_ReadRing(void)
{
    u8 bitstatus = 0;
    
    bitstatus = GPIO_ReadInputDataBit(Modem_BSP_RING_GPIO_PORT, Modem_BSP_RING_PIN);

    return (bitstatus);
}

/**
  * @brief  打开麦克风
  * @param  None
  * @retval None
  */

void  Modem_Bsp_MicOn(void)
{
#ifdef EYE_MODEM
    Io_WriteMicConrtolBit(MIC_BIT_TALK,SET);
#else
    GPIO_SetBits(Modem_BSP_MIC_GPIO_PORT, Modem_BSP_MIC_PIN);
#endif 

}

/**
  * @brief  关闭麦克风
  * @param  None
  * @retval None
  */

void  Modem_Bsp_MicOff(void)
{
#ifdef EYE_MODEM
    Io_WriteMicConrtolBit(MIC_BIT_TALK,RESET);
#else
    GPIO_ResetBits(Modem_BSP_MIC_GPIO_PORT, Modem_BSP_MIC_PIN);
#endif 
}
/**
  * @brief  打开扬声器
  * @param  BSP_SPK_TALK
  *         BSP_SPK_TTS
  *
  * @retval None
  */
void  Modem_Bsp_SpkOn(u8 type)
{
#ifdef EYE_MODEM
    Io_WriteSpeakerConrtolBit((SPEAKER_BIT)type,SET);
#else
    GPIO_SetBits(Modem_BSP_SPK_GPIO_PORT, Modem_BSP_SPK_PIN);
#endif 
}

/**
  * @brief  关闭扬声器
  * @param  type-> 0: 通话;
  *                1: TTS;
  * @retval None
  */
void  Modem_Bsp_SpkOff(u8 type)
{
#ifdef EYE_MODEM
    Io_WriteSpeakerConrtolBit((SPEAKER_BIT)type,RESET);
#else
    GPIO_ResetBits(Modem_BSP_SPK_GPIO_PORT, Modem_BSP_SPK_PIN);
#endif 
}
                           


/**
  ******************************************************************************
  * @file    modem_bsp.h
  * @author  miaoyahan
  * @version V1.0.0
  * @date    2013-08-21 
  * @brief   
  ******************************************************************************
  * @attention
  ******************************************************************************
  */ 
#ifndef __MODEM_BSP_H
#define __MODEM_BSP_H

/*
*********************************************************************************************************
*                                               DEFINES
*********************************************************************************************************
*/
#ifdef EYE_MODEM

#else
    typedef unsigned char    BIT;
    typedef unsigned char    BOOL;
#endif

#ifndef __TRUE
 #define __TRUE         1
#endif
#ifndef __FALSE
 #define __FALSE        0
#endif

/*******************************************************************************
*        模块信息: 上海移远 M系列模块  G网
*        开机    : 2s
*        关机    : 1s
*        备注    : 目前只有M12DE和M50支持TTS
*******************************************************************************/
//#define Modem_Quectel               "Revision: M"// Revision: M35,dxl,2015.5.13,之前模块型号和内部TTS的识别需要各发送ATI 1次,现在修改后1次就可以了
#define Modem_Quectel               "Quectel"
#define Modem_QActOnDly              (2)
#define Modem_QActOffDly             (1)

/*******************************************************************************
*        模块信息: 广和通 G510  G网
*        开机    : 2s
*        关机    : 1s
*        备注    : 无
*******************************************************************************/
#define Modem_FIBOCOM                 "G510_V0D"
#define Modem_FIBOCOMActOnDly         (2)
#define Modem_FIBOCOMActOffDly        (3)

/*******************************************************************************
*        模块信息: 中兴通讯 MC8332  C网
*        开机    : 2s
*        关机    : 2s
*        备注    : 无
*******************************************************************************/
//#define Modem_ZTE                    "Model: MC8332",2015.5.13
#define Modem_ZTE                    "Model: MC833"
#define Modem_ZTEActOnDly             (2)
#define Modem_ZTEActOffDly            (2)


/*******************************************************************************
*        模块扬声器控制
*******************************************************************************/
#define  BSP_SPK_TALK                 (0)//通话 
#define  BSP_SPK_TTS                  (1)//TTS
 
////////////////////////// IO Config Start /////////////////////////////////////
/*******************************************************************************
*
*   Definitions 适用于设备 -> HB EGS701 一体机
*
*******************************************************************************/
#ifdef EYE_EGS701
#define Modem_BSP_POWER_PIN                 GPIO_Pin_10
#define Modem_BSP_POWER_GPIO_PORT           GPIOC
#define Modem_BSP_POWER_GPIO_CLK            RCC_AHB1Periph_GPIOC

#define Modem_BSP_ACTIVE_PIN                GPIO_Pin_4
#define Modem_BSP_ACTIVE_GPIO_PORT          GPIOD
#define Modem_BSP_ACTIVE_GPIO_CLK           RCC_AHB1Periph_GPIOD

#define Modem_BSP_DTR_PIN                   GPIO_Pin_5
#define Modem_BSP_DTR_GPIO_PORT             GPIOG
#define Modem_BSP_DTR_GPIO_CLK              RCC_AHB1Periph_GPIOG

#define Modem_BSP_RING_PIN                  GPIO_Pin_13
#define Modem_BSP_RING_GPIO_PORT            GPIOG
#define Modem_BSP_RING_GPIO_CLK             RCC_AHB1Periph_GPIOG

#define Modem_BSP_MIC_PIN                  GPIO_Pin_5
#define Modem_BSP_MIC_GPIO_PORT            GPIOD
#define Modem_BSP_MIC_GPIO_CLK             RCC_AHB1Periph_GPIOD

#define Modem_BSP_SPK_PIN                  GPIO_Pin_7
#define Modem_BSP_SPK_GPIO_PORT            GPIOC
#define Modem_BSP_SPK_GPIO_CLK             RCC_AHB1Periph_GPIOC

/*******************************************************************************
*
*   Definitions 适用于设备 -> EGS702  分体机
*
*******************************************************************************/
#elif defined EYE_EGS702
#define Modem_BSP_POWER_PIN                 GPIO_Pin_10
#define Modem_BSP_POWER_GPIO_PORT           GPIOD
#define Modem_BSP_POWER_GPIO_CLK            RCC_AHB1Periph_GPIOD

#define Modem_BSP_ACTIVE_PIN                GPIO_Pin_14
#define Modem_BSP_ACTIVE_GPIO_PORT          GPIOE
#define Modem_BSP_ACTIVE_GPIO_CLK           RCC_AHB1Periph_GPIOE

#define Modem_BSP_DTR_PIN                   GPIO_Pin_5
#define Modem_BSP_DTR_GPIO_PORT             GPIOG
#define Modem_BSP_DTR_GPIO_CLK              RCC_AHB1Periph_GPIOG

#define Modem_BSP_RING_PIN                  GPIO_Pin_13
#define Modem_BSP_RING_GPIO_PORT            GPIOG
#define Modem_BSP_RING_GPIO_CLK             RCC_AHB1Periph_GPIOG

#define Modem_BSP_MIC_PIN                  GPIO_Pin_12
#define Modem_BSP_MIC_GPIO_PORT            GPIOF
#define Modem_BSP_MIC_GPIO_CLK             RCC_AHB1Periph_GPIOF

#define Modem_BSP_SPK_PIN                  GPIO_Pin_15
#define Modem_BSP_SPK_GPIO_PORT            GPIOF
#define Modem_BSP_SPK_GPIO_CLK             RCC_AHB1Periph_GPIOF

/*******************************************************************************
*
*   Definitions 适用于设备 -> 缺省设备,暂时默认 -> EGS701 一体机
*
*******************************************************************************/
#else
#define Modem_BSP_POWER_PIN                 GPIO_Pin_10
#define Modem_BSP_POWER_GPIO_PORT           GPIOC
#define Modem_BSP_POWER_GPIO_CLK            RCC_AHB1Periph_GPIOC

#define Modem_BSP_ACTIVE_PIN                GPIO_Pin_4
#define Modem_BSP_ACTIVE_GPIO_PORT          GPIOD
#define Modem_BSP_ACTIVE_GPIO_CLK           RCC_AHB1Periph_GPIOD

#define Modem_BSP_DTR_PIN                   GPIO_Pin_5
#define Modem_BSP_DTR_GPIO_PORT             GPIOG
#define Modem_BSP_DTR_GPIO_CLK              RCC_AHB1Periph_GPIOG

#define Modem_BSP_RING_PIN                  GPIO_Pin_13
#define Modem_BSP_RING_GPIO_PORT            GPIOG
#define Modem_BSP_RING_GPIO_CLK             RCC_AHB1Periph_GPIOG

#define Modem_BSP_MIC_PIN                  GPIO_Pin_5
#define Modem_BSP_MIC_GPIO_PORT            GPIOD
#define Modem_BSP_MIC_GPIO_CLK             RCC_AHB1Periph_GPIOD

#define Modem_BSP_SPK_PIN                  GPIO_Pin_7
#define Modem_BSP_SPK_GPIO_PORT            GPIOC
#define Modem_BSP_SPK_GPIO_CLK             RCC_AHB1Periph_GPIOC
#endif

////////////////////////// IO Config End ///////////////////////////////////////

/*
*********************************************************************************************************
*                                          GLOBAL VARIABLES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            FUNCTION PROTOTYPES
*********************************************************************************************************
*/
void  Modem_Bsp_Init (void);
void  Modem_Bsp_SetPower(uint8_t state);
void  Modem_Bsp_SetActive(uint8_t state);
void  Modem_Bsp_SetDTR(uint8_t state);
void  Modem_Bsp_SpkOn(u8 type);
void  Modem_Bsp_SpkOff(u8 type);
void  Modem_Bsp_MicOn(void);
void  Modem_Bsp_MicOff(void);

u8 Modem_Bsp_ReadRing(void);

#endif








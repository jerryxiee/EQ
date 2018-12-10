/**
  ******************************************************************************
  * @file    modem_uart.h 
  * @author  miaoyahan
  * @version V1.0.0
  * @date    2013-08-12 
  * @brief   modem串口操作，包括初始化串口号以及初始化相应中断
  ******************************************************************************
  * @attention
  ******************************************************************************
  */ 
#ifndef __MODEM_UART_H
#define __MODEM_UART_H

/*
********************************************************************************
*                               INCLUDES
********************************************************************************
*/
#include <stdio.h>
#include "stm32f2xx.h"
#include "modem_lib.h"
/*
********************************************************************************
*                               DEFINES
********************************************************************************
*/

/*******************************************************************************
*
*   Definitions 适用于设备 -> HB EGS701 一体机
*
*******************************************************************************/
#ifdef   EYE_EGS701
#define     Modem_UART_NUM             (5)
/*******************************************************************************
*
*   Definitions 适用于设备 -> EGS702  分体机
*
*******************************************************************************/
#elif    defined EYE_EGS702
#define     Modem_UART_NUM             (2)
/*******************************************************************************
*
*   Definitions 适用于设备 -> 缺省设备,暂时默认 -> EGS701 一体机
*
*******************************************************************************/
#else
#define     Modem_UART_NUM             (5)
#endif

#define     Modem_BaudRate             (115200)
#define     Modem_TX_FIFO_SIZE         (1024)
#define     Modem_NODE_AT_NUM          (50)
#define     Modem_NODE_BUF             (2048)

////////////////////////  start if /////////////////////////////////////////////
#if (Modem_UART_NUM == 2)//////////////串口2

#define     Modem_UART                  USART2
#define     Modem_UART_CLK              RCC_APB1Periph_USART2 
#define     Modem_UART_TX_PIN           GPIO_Pin_2
#define     Modem_UART_TX_GPIO_PORT     GPIOA
#define     Modem_UART_TX_GPIO_CLK      RCC_AHB1Periph_GPIOA
#define     Modem_UART_TX_SOURCE        GPIO_PinSource2
#define     Modem_UART_TX_AF            GPIO_AF_USART2
#define     Modem_UART_RX_PIN           GPIO_Pin_3
#define     Modem_UART_RX_GPIO_PORT     GPIOA
#define     Modem_UART_RX_GPIO_CLK      RCC_AHB1Periph_GPIOA
#define     Modem_UART_RX_SOURCE        GPIO_PinSource3
#define     Modem_UART_RX_AF            GPIO_AF_USART2
#define     Modem_UART_IRQn             USART2_IRQn
#define     Modem_USART_IRQHandler      USART2_IRQHandler  

///////////////////////////////////////////////////////////////////////////////
#elif (Modem_UART_NUM == 5)/////////////串口5

#define     Modem_UART                  UART5
#define     Modem_UART_CLK              RCC_APB1Periph_UART5 
#define     Modem_UART_TX_PIN           GPIO_Pin_12
#define     Modem_UART_TX_GPIO_PORT     GPIOC
#define     Modem_UART_TX_GPIO_CLK      RCC_AHB1Periph_GPIOC
#define     Modem_UART_TX_SOURCE        GPIO_PinSource12
#define     Modem_UART_TX_AF            GPIO_AF_UART5
#define     Modem_UART_RX_PIN           GPIO_Pin_2
#define     Modem_UART_RX_GPIO_PORT     GPIOD
#define     Modem_UART_RX_GPIO_CLK      RCC_AHB1Periph_GPIOD
#define     Modem_UART_RX_SOURCE        GPIO_PinSource2
#define     Modem_UART_RX_AF            GPIO_AF_UART5
#define     Modem_UART_IRQn             UART5_IRQn
#define     Modem_USART_IRQHandler      UART5_IRQHandler  

#endif

/*
********************************************************************************
*                                 ENUM
********************************************************************************
*/



/*
********************************************************************************
*                                STRUCT
********************************************************************************
*/

/*
********************************************************************************
*                            GLOBAL VARIABLES
********************************************************************************
*/


/*
********************************************************************************
*                               FUNCTIONS
********************************************************************************
*/
void Modem_Uart_Init (void);
u16  Modem_Uart_Send(u8* data, u16 len);
void Modem_Uart_NodRst(void);
u8   Modem_Uart_NodNum(void);
void Modem_Uart_NodClr(void);
u16  Modem_Uart_NodRet(u8 *pData);
void Modem_Uart_NodOut(void);

/*
********************************************************************************
*                               MODULE END
********************************************************************************
*/
#endif

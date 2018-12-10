/******************************************************************** 
//版权说明	:
//文件名称	:Usart_drive.c		
//功能		:USART串口配置
//版本号	:
//开发人	:yjb 
//开发时间	:2012.6 
//修改者	:
//修改时间	:
//修改简要说明	:
//备注		:
//
***********************************************************************/
//****************包含文件*****************
#include "stm32f2xx.h"
#include "HAL.h"
//****************宏定义********************

//****************外部变量*******************

//****************全局变量*******************
USART_TypeDef* COM_USART[COMn] = {EVAL_COM1,
                                  EVAL_COM2,
                                  EVAL_COM3,
                                  EVAL_COM4,
                                  EVAL_COM5,
                                  EVAL_COM6,
                                  }; 

GPIO_TypeDef* COM_TX_PORT[COMn] = {EVAL_COM1_TX_GPIO_PORT,
                                   EVAL_COM2_TX_GPIO_PORT,
                                   EVAL_COM3_TX_GPIO_PORT,
                                   EVAL_COM4_TX_GPIO_PORT,
                                   EVAL_COM5_TX_GPIO_PORT,
                                   EVAL_COM6_TX_GPIO_PORT,
                                  };
 
GPIO_TypeDef* COM_RX_PORT[COMn] = {EVAL_COM1_RX_GPIO_PORT,
                                   EVAL_COM2_RX_GPIO_PORT,
                                   EVAL_COM3_RX_GPIO_PORT,
                                   EVAL_COM4_RX_GPIO_PORT,
                                   EVAL_COM5_RX_GPIO_PORT,
                                   EVAL_COM6_RX_GPIO_PORT,
                                  };

const uint32_t COM_USART_CLK[COMn] = {EVAL_COM1_CLK,
                                      EVAL_COM2_CLK,
                                      EVAL_COM3_CLK,
                                      EVAL_COM4_CLK,
                                      EVAL_COM5_CLK,
                                      EVAL_COM6_CLK,
                                   };

const uint32_t COM_TX_PORT_CLK[COMn] = {EVAL_COM1_TX_GPIO_CLK,
                                        EVAL_COM2_TX_GPIO_CLK,
                                        EVAL_COM3_TX_GPIO_CLK,
                                        EVAL_COM4_TX_GPIO_CLK,
                                        EVAL_COM5_TX_GPIO_CLK,
                                        EVAL_COM6_TX_GPIO_CLK,
                                    };
 
const uint32_t COM_RX_PORT_CLK[COMn] = {EVAL_COM1_RX_GPIO_CLK,
                                        EVAL_COM2_RX_GPIO_CLK,
                                        EVAL_COM3_RX_GPIO_CLK,
                                        EVAL_COM4_RX_GPIO_CLK,
                                        EVAL_COM5_RX_GPIO_CLK,
                                        EVAL_COM6_RX_GPIO_CLK,
                                    };

const uint16_t COM_TX_PIN[COMn] = {EVAL_COM1_TX_PIN,
                                   EVAL_COM2_TX_PIN,
                                   EVAL_COM3_TX_PIN,
                                   EVAL_COM4_TX_PIN,
                                   EVAL_COM5_TX_PIN,
                                   EVAL_COM6_TX_PIN,
                                  };

const uint16_t COM_RX_PIN[COMn] = {EVAL_COM1_RX_PIN,
                                   EVAL_COM2_RX_PIN,
                                   EVAL_COM3_RX_PIN,
                                   EVAL_COM4_RX_PIN,
                                   EVAL_COM5_RX_PIN,
                                   EVAL_COM6_RX_PIN,
                                  };
 
const uint8_t COM_TX_PIN_SOURCE[COMn] = {EVAL_COM1_TX_SOURCE,
                                         EVAL_COM2_TX_SOURCE,
                                         EVAL_COM3_TX_SOURCE,
                                         EVAL_COM4_TX_SOURCE,
                                         EVAL_COM5_TX_SOURCE,
                                         EVAL_COM6_TX_SOURCE,
                                        };

const uint8_t COM_RX_PIN_SOURCE[COMn] = {EVAL_COM1_RX_SOURCE,
                                         EVAL_COM2_RX_SOURCE,
                                         EVAL_COM3_RX_SOURCE,
                                         EVAL_COM4_RX_SOURCE,
                                         EVAL_COM5_RX_SOURCE,
                                         EVAL_COM6_RX_SOURCE,
                                        };
 
const uint8_t COM_TX_AF[COMn] = {EVAL_COM1_TX_AF,
                                 EVAL_COM2_TX_AF,
                                 EVAL_COM3_TX_AF,
                                 EVAL_COM4_TX_AF,
                                 EVAL_COM5_TX_AF,
                                 EVAL_COM6_TX_AF,
                                };
 
const uint8_t COM_RX_AF[COMn] = {EVAL_COM1_RX_AF,
                                 EVAL_COM2_RX_AF,
                                 EVAL_COM3_RX_AF,
                                 EVAL_COM4_RX_AF,
                                 EVAL_COM5_RX_AF,
                                 EVAL_COM6_RX_AF,
                                };

const uint8_t COM_IAR[COMn] = {EVAL_COM1_IRQn,
                               EVAL_COM2_IRQn,
                               EVAL_COM3_IRQn,
                               EVAL_COM4_IRQn,
                               EVAL_COM5_IRQn,
                               EVAL_COM6_IRQn,
                               };
//****************函数声明*******************


//****************函数定义******************

/**
  * @brief  初始化COM口.
  * @param  COM: Specifies the COM port to be configured.
  *   This parameter can be one of following parameters:    
  *     @arg COM1
  *     @arg COM2  
  * @param  USART_InitStruct: pointer to a USART_InitTypeDef structure that
  *   contains the configuration information for the specified USART peripheral.
  * @retval None
  */
void COM_Init(COM_TypeDef COM, USART_InitTypeDef* USART_InitStruct)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* 使能GPIO时钟 */
  RCC_AHB1PeriphClockCmd(COM_TX_PORT_CLK[COM] | COM_RX_PORT_CLK[COM], ENABLE);

  /*使能USART时钟,Usart1和USART6在APB2上，Usart2, Usart3,Usart4,Usart5在APB1上*/
  if ((COM == COM1) || (COM == COM6))
  {
    RCC_APB2PeriphClockCmd(COM_USART_CLK[COM], ENABLE);
  }else{
    RCC_APB1PeriphClockCmd(COM_USART_CLK[COM], ENABLE);
  }

  /* 连接 PXx引脚到USART_Tx*/
  GPIO_PinAFConfig(COM_TX_PORT[COM], COM_TX_PIN_SOURCE[COM], COM_TX_AF[COM]);

  /* 连接 PXx引脚到USART_Rx*/
  GPIO_PinAFConfig(COM_RX_PORT[COM], COM_RX_PIN_SOURCE[COM], COM_RX_AF[COM]);

  /* Configure USART Tx as alternate function  */
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;

  GPIO_InitStructure.GPIO_Pin = COM_TX_PIN[COM];
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(COM_TX_PORT[COM], &GPIO_InitStructure);

  /* Configure USART Rx as alternate function  */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Pin = COM_RX_PIN[COM];
  GPIO_Init(COM_RX_PORT[COM], &GPIO_InitStructure);

  /* USART configuration */
  USART_Init(COM_USART[COM], USART_InitStruct);
    
  /* Enable USART */
  USART_Cmd(COM_USART[COM], ENABLE);
}

/**
  * @brief  打开串口中断.
  * @retval None
  */
void COM_NVIC_Config(COM_TypeDef COM)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  /* 启用USARTx中断*/
  NVIC_InitStructure.NVIC_IRQChannel = COM_IAR[COM];
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

/**
  * @brief  获取串口的中断编号.
  * @param  COM: 串口编号
  * @retval None
  */
u8 GetUsartIrq(COM_TypeDef COM)
{
  return COM_IAR[COM];
}

/**
  * @brief  获取指定状态位值.
  * @param  COM: 串口编号
  * @param  USART_IT: 需要获取的状态位
  * @retval 有效位返回SET,无效返回RESET.
  */
ITStatus GetUsartStatus(COM_TypeDef COM, uint16_t USART_IT)
{
    return USART_GetITStatus(COM_USART[COM], USART_IT);
}

/**
  * @brief  清除串口状态位.
  * @param  COM: 串口编号
  * @param  USART_IT: 待清除的状态位
  * @retval None
  */
void ClearUsartStatus(COM_TypeDef COM, uint16_t USART_IT)
{
    USART_ClearITPendingBit(COM_USART[COM], USART_IT); 
}

/**
  * @brief  获取串口数据.
  * @param  COM: 串口号
  * @retval 返回获取的数据
  */
u8 GetUsartReceiveData(COM_TypeDef COM)
{
    return (USART_ReceiveData(COM_USART[COM])  & 0xFF);
}
/**
  * @brief  设置中断是否启用和禁用.
  * @param  COM: 串口编号
  * @param  USART_IT: 中断类型
  * @param  NewState: 启用ENABLE,禁用DISABLE
  * @retval 返回获取的数据
  */
void SetUsartITConfig(COM_TypeDef COM, uint16_t USART_IT, FunctionalState NewState)
{
    USART_ITConfig(COM_USART[COM], USART_IT, NewState);
}

/**
  * @brief  设置中断是否启用和禁用.
  * @param  COM: 串口编号
  * @param  Data: 待发送数据
  * @retval None
  */
void UsartSendData(COM_TypeDef COM, u8 Data)
{
  USART_SendData(COM_USART[COM], Data);
}

/**
  * @brief  根据串口编号获取串口指针.
  * @param  COM: 串口编号
  * @retval 当前编号指向的串口指针
  */
USART_TypeDef* GetUsartPtr(COM_TypeDef COM)
{
    return COM_USART[COM];
}
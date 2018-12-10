/********************************************************************
//版权说明	:
//文件名称	:Can.c		
//功能		:CAN总线配置
//版本号	:
//开发人	:yjb
//开发时间	:2012.6
//修改者	:
//修改时间	:
//修改简要说明	:
//备注		:1)CAN配置,由于CAN_前缀与STM32库自定义的冲突,所以全部修改为CANBus。
//              :2)用户使用
//              :  GpioOutInit(CAN_CTR); 
//              :  GpioOutInit(CAN_CTR);       //打开CAN电源
//              :  CANBus_Init(CAN_2,500000); //初始化CAN，波特率可设置250KBps,500Kbps,800KBps,1000KBps,默认打开接收中断.
//              :  void CAN_IRQHandler(void);  //在CAN中断函数定义接收数据处理
//              :  
//              :3)管脚映射
//                 1.CAN1:
//                  CAN_RX->PD0
//                  CAN_TX->PD1
//                2.CAN2:
//                  CAN_RX->PB5
//                  CAN_TX->PB6
//
//
***********************************************************************/
#ifndef CAN_H
#define CAN_H

#ifdef __cplusplus
 extern "C" {
#endif 
   
//****************宏定义********************
//支持CAN总线个数编号    
typedef enum 
{
  CAN_1 = 0,
  CAN_2,
  CANMAX,
} CAN_NUM;

#define EVAL_CAN1                        CAN1
#define EVAL_CAN1_CLK                    RCC_APB1Periph_CAN1
#define EVAL_CAN1_TX_PIN                 GPIO_Pin_1
#define EVAL_CAN1_TX_GPIO_PORT           GPIOD
#define EVAL_CAN1_TX_GPIO_CLK            RCC_AHB1Periph_GPIOD
#define EVAL_CAN1_TX_SOURCE              GPIO_PinSource1
#define EVAL_CAN1_TX_AF                  GPIO_AF_CAN1
#define EVAL_CAN1_RX_PIN                 GPIO_Pin_0
#define EVAL_CAN1_RX_GPIO_PORT           GPIOD
#define EVAL_CAN1_RX_GPIO_CLK            RCC_AHB1Periph_GPIOD
#define EVAL_CAN1_RX_SOURCE              GPIO_PinSource0
#define EVAL_CAN1_RX_AF                  GPIO_AF_CAN1
#define EVAL_CAN1_IRQn                   CAN1_RX0_IRQn

#define EVAL_CAN2                        CAN2
#define EVAL_CAN2_CLK                    (RCC_APB1Periph_CAN1|RCC_APB1Periph_CAN2)
#define EVAL_CAN2_TX_PIN                 GPIO_Pin_6
#define EVAL_CAN2_TX_GPIO_PORT           GPIOB
#define EVAL_CAN2_TX_GPIO_CLK            RCC_AHB1Periph_GPIOB
#define EVAL_CAN2_TX_SOURCE              GPIO_PinSource6
#define EVAL_CAN2_TX_AF                  GPIO_AF_CAN2
#define EVAL_CAN2_RX_PIN                 GPIO_Pin_12
#define EVAL_CAN2_RX_GPIO_PORT           GPIOB
#define EVAL_CAN2_RX_GPIO_CLK            RCC_AHB1Periph_GPIOB
#define EVAL_CAN2_RX_SOURCE              GPIO_PinSource12
#define EVAL_CAN2_RX_AF                  GPIO_AF_CAN2
#define EVAL_CAN2_IRQn                   CAN2_RX0_IRQn

//****************结构定义*******************

//****************外部变量*******************

//****************函数声明*******************
void CANBus_Init(CAN_NUM CanNum, u32 BandRate);


#ifdef __cplusplus
}
#endif   

#endif /* CAN_H */
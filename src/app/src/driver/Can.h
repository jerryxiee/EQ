/********************************************************************
//��Ȩ˵��	:
//�ļ�����	:Can.c		
//����		:CAN��������
//�汾��	:
//������	:yjb
//����ʱ��	:2012.6
//�޸���	:
//�޸�ʱ��	:
//�޸ļ�Ҫ˵��	:
//��ע		:1)CAN����,����CAN_ǰ׺��STM32���Զ���ĳ�ͻ,����ȫ���޸�ΪCANBus��
//              :2)�û�ʹ��
//              :  GpioOutInit(CAN_CTR); 
//              :  GpioOutInit(CAN_CTR);       //��CAN��Դ
//              :  CANBus_Init(CAN_2,500000); //��ʼ��CAN�������ʿ�����250KBps,500Kbps,800KBps,1000KBps,Ĭ�ϴ򿪽����ж�.
//              :  void CAN_IRQHandler(void);  //��CAN�жϺ�������������ݴ���
//              :  
//              :3)�ܽ�ӳ��
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
   
//****************�궨��********************
//֧��CAN���߸������    
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

//****************�ṹ����*******************

//****************�ⲿ����*******************

//****************��������*******************
void CANBus_Init(CAN_NUM CanNum, u32 BandRate);


#ifdef __cplusplus
}
#endif   

#endif /* CAN_H */
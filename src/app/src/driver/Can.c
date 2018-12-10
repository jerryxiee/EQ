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
//****************�����ļ�*****************
#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "Can.h"
#include "HAL.h"

//****************�궨��********************

//****************�ⲿ����*******************

//****************ȫ�ֱ���*******************
CAN_TypeDef* EVAL_CAN[CANMAX] = {EVAL_CAN1,
                                 EVAL_CAN2,
                                  }; 

GPIO_TypeDef* CAN_TX_PORT[CANMAX] = {EVAL_CAN1_TX_GPIO_PORT,
                                     EVAL_CAN2_TX_GPIO_PORT,
                                  };
 
GPIO_TypeDef* CAN_RX_PORT[CANMAX] = {EVAL_CAN1_RX_GPIO_PORT,
                                     EVAL_CAN2_RX_GPIO_PORT,
                                  };

const uint32_t CAN_CLK[CANMAX] = {EVAL_CAN1_CLK,
                                  EVAL_CAN2_CLK,
                                   };

const uint32_t CAN_TX_PORT_CLK[CANMAX] = {EVAL_CAN1_TX_GPIO_CLK,
                                          EVAL_CAN2_TX_GPIO_CLK,
                                    };  
 
const uint32_t CAN_RX_PORT_CLK[CANMAX] = {EVAL_CAN1_RX_GPIO_CLK,
                                          EVAL_CAN2_RX_GPIO_CLK,
                                    };

const uint16_t CAN_TX_PIN[CANMAX] = {EVAL_CAN1_TX_PIN,
                                     EVAL_CAN2_TX_PIN,
                                  };

const uint16_t CAN_RX_PIN[CANMAX] = {EVAL_CAN1_RX_PIN,
                                   EVAL_CAN2_RX_PIN,
                                  };
 
const uint8_t CAN_TX_PIN_SOURCE[CANMAX] = {EVAL_CAN1_TX_SOURCE,
                                           EVAL_CAN2_TX_SOURCE,
                                        };

const uint8_t CAN_RX_PIN_SOURCE[CANMAX] = {EVAL_CAN1_RX_SOURCE,
                                           EVAL_CAN2_RX_SOURCE,
                                        };
 
const uint8_t CAN_TX_AF[CANMAX] = {EVAL_CAN1_TX_AF,
                                   EVAL_CAN2_TX_AF,
                                };
 
const uint8_t CAN_RX_AF[CANMAX] = {EVAL_CAN1_RX_AF,
                                   EVAL_CAN2_RX_AF,
                                };

const uint8_t CAN_IAR[CANMAX] = {EVAL_CAN1_IRQn,
                                 EVAL_CAN2_IRQn,
                               };
//****************��������*******************


//****************��������******************

/*********************************************************************
//��������	:CANBus_Init
//����		:��ʼ��CAN
//����		:CanNum        CAN���
//              :BandRateBuf   CAN������
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:1)Ĭ�ϴ�CAN�����ж�
*********************************************************************/
void CANBus_Init(CAN_NUM CanNum , u32 BandRate)
{
    NVIC_InitTypeDef  NVIC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    CAN_InitTypeDef        CAN_InitStructure;
    CAN_FilterInitTypeDef  CAN_FilterInitStructure;    
  
    //����CAN�ж�
    NVIC_InitStructure.NVIC_IRQChannel = CAN_IAR[CanNum];
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    //����GPIO
    //ʹ��ʱ��
    RCC_AHB1PeriphClockCmd(CAN_TX_PORT_CLK[CanNum]|CAN_RX_PORT_CLK[CanNum], ENABLE);
    //�ܽ�ӳ��
    GPIO_PinAFConfig(CAN_TX_PORT[CanNum], CAN_TX_PIN_SOURCE[CanNum], CAN_TX_AF[CanNum]);
    GPIO_PinAFConfig(CAN_RX_PORT[CanNum], CAN_RX_PIN_SOURCE[CanNum], CAN_RX_AF[CanNum]); 
    //IO����
    GPIO_InitStructure.GPIO_Pin = CAN_TX_PIN[CanNum];
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_Init(CAN_TX_PORT[CanNum], &GPIO_InitStructure); 
    
    GPIO_InitStructure.GPIO_Pin = CAN_RX_PIN[CanNum];
    GPIO_Init(CAN_RX_PORT[CanNum], &GPIO_InitStructure); 
    
    //CAN����
    RCC_APB1PeriphClockCmd(CAN_CLK[CanNum], ENABLE);
    CAN_DeInit(EVAL_CAN[CanNum]);                                 //�ָ�Ĭ��ֵ
    
    CAN_InitStructure.CAN_TTCM = DISABLE;                         //ʹ��/ʧ�� ʱ�䴥��ͨѶģʽ.0:ʱ�䴥��ͨ��ģʽ�ر�;
    CAN_InitStructure.CAN_ABOM = DISABLE;                         //ʹ��/ʧ�� �Զ����߹���.    1:һ����ص�128��11����������λ,�Զ��˳�����״̬;
    CAN_InitStructure.CAN_AWUM = ENABLE;                          //ʹ��/ʧ�� �Զ�����ģʽ.    1:Ӳ����⵽CAN����ʱ�Զ��뿪����ģʽ;
    CAN_InitStructure.CAN_NART = ENABLE;                          //ʹ��/ʧ�� ���Զ��ش���ģʽ.0:CANӲ������ʧ�ܺ��һֱ�ط�ֱ�����ͳɹ�;
    CAN_InitStructure.CAN_RFLM = DISABLE;                         //ʹ��/ʧ�� �ܽ���FIFO����ģʽ.0:����FIFO����,��һ������ı��Ľ�����ǰһ��;
    CAN_InitStructure.CAN_TXFP = ENABLE;                          //ʹ��/ʧ�� ����FIFO���ȼ�.    1:�ɷ��������˳��(ʱ���Ⱥ�˳��)���������ȼ�.
    CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;
    CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;      
    
    //RCC_ClocksStatus.PCLK1_Frequency ʱ��Ϊ24000000Hz
    CAN_InitStructure.CAN_BS2 = CAN_BS2_2tq;
    if(BandRate <= 500000L)         //BandRate <= 500KHz
    {
            CAN_InitStructure.CAN_BS1 = CAN_BS1_13tq;
            CAN_InitStructure.CAN_Prescaler = (24000000/BandRate)/16;
    } else if(BandRate <= 800000L){ //500KHz < BandRate <= 800KHz
            CAN_InitStructure.CAN_BS1 = CAN_BS1_7tq;
            CAN_InitStructure.CAN_Prescaler = (24000000/BandRate)/10;
    } else {                           //800KHz < BandRate
            CAN_InitStructure.CAN_BS1 = CAN_BS1_5tq;
            CAN_InitStructure.CAN_Prescaler = (24000000/BandRate)/8;
    }
    
    CAN_Init(EVAL_CAN[CanNum], &CAN_InitStructure);    
    if (CanNum == CAN_1)
    {
        CAN_FilterInitStructure.CAN_FilterNumber = 0;
    }else{
        CAN_FilterInitStructure.CAN_FilterNumber = 14;
    }
    CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
    CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
    CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;
    CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
    CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;
    CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
    CAN_FilterInitStructure.CAN_FilterFIFOAssignment = 0;
    CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
    CAN_FilterInit(&CAN_FilterInitStructure); 
    
    CAN_ITConfig(EVAL_CAN[CanNum], CAN_IT_FMP0, ENABLE);
}
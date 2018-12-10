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
//****************包含文件*****************
#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "Can.h"
#include "HAL.h"

//****************宏定义********************

//****************外部变量*******************

//****************全局变量*******************
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
//****************函数声明*******************


//****************函数定义******************

/*********************************************************************
//函数名称	:CANBus_Init
//功能		:初始化CAN
//输入		:CanNum        CAN编号
//              :BandRateBuf   CAN波特率
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:1)默认打开CAN接收中断
*********************************************************************/
void CANBus_Init(CAN_NUM CanNum , u32 BandRate)
{
    NVIC_InitTypeDef  NVIC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    CAN_InitTypeDef        CAN_InitStructure;
    CAN_FilterInitTypeDef  CAN_FilterInitStructure;    
  
    //配置CAN中断
    NVIC_InitStructure.NVIC_IRQChannel = CAN_IAR[CanNum];
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    //配置GPIO
    //使能时钟
    RCC_AHB1PeriphClockCmd(CAN_TX_PORT_CLK[CanNum]|CAN_RX_PORT_CLK[CanNum], ENABLE);
    //管脚映射
    GPIO_PinAFConfig(CAN_TX_PORT[CanNum], CAN_TX_PIN_SOURCE[CanNum], CAN_TX_AF[CanNum]);
    GPIO_PinAFConfig(CAN_RX_PORT[CanNum], CAN_RX_PIN_SOURCE[CanNum], CAN_RX_AF[CanNum]); 
    //IO配置
    GPIO_InitStructure.GPIO_Pin = CAN_TX_PIN[CanNum];
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_Init(CAN_TX_PORT[CanNum], &GPIO_InitStructure); 
    
    GPIO_InitStructure.GPIO_Pin = CAN_RX_PIN[CanNum];
    GPIO_Init(CAN_RX_PORT[CanNum], &GPIO_InitStructure); 
    
    //CAN配置
    RCC_APB1PeriphClockCmd(CAN_CLK[CanNum], ENABLE);
    CAN_DeInit(EVAL_CAN[CanNum]);                                 //恢复默认值
    
    CAN_InitStructure.CAN_TTCM = DISABLE;                         //使能/失能 时间触发通讯模式.0:时间触发通信模式关闭;
    CAN_InitStructure.CAN_ABOM = DISABLE;                         //使能/失能 自动离线管理.    1:一旦监控到128次11个连续隐形位,自动退出离线状态;
    CAN_InitStructure.CAN_AWUM = ENABLE;                          //使能/失能 自动唤醒模式.    1:硬件检测到CAN报文时自动离开休眠模式;
    CAN_InitStructure.CAN_NART = ENABLE;                          //使能/失能 非自动重传输模式.0:CAN硬件发送失败后会一直重发直到发送成功;
    CAN_InitStructure.CAN_RFLM = DISABLE;                         //使能/失能 能接收FIFO锁定模式.0:接收FIFO满了,下一条传入的报文将覆盖前一条;
    CAN_InitStructure.CAN_TXFP = ENABLE;                          //使能/失能 发送FIFO优先级.    1:由发送请求的顺序(时间先后顺序)来决定优先级.
    CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;
    CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;      
    
    //RCC_ClocksStatus.PCLK1_Frequency 时钟为24000000Hz
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
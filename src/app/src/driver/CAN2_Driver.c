/************************************************************************* 
* Copyright (c) 2013,�������������¼����������޹�˾
* All rights reserved.
* �ļ����� : CAN2_Driver.c
* ��ǰ�汾 : 1.0
* ������   : Shigle
* �޸�ʱ�� : 20113��6��10��
*************************************************************************/

/***  �����ļ�  ***/

#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "include.h"


/***  �ⲿ���ñ���  ***/

extern PARAMETER_CAN gCANParame[];
extern CAN_ISR_DATA   gCanIsrData[];



/***  ���� ��������  ***/

static void CAN2_Init_Interrupt( void);
static void CAN2_Init_GPIO( void );
static void CAN2_BaudRate_Set(u32 BandRateBuf);
static void CAN2_Init_Reg(void);
static void CAN2_Init_Task(void);


/********************************************************************
* ���� : CAN2_Init_Interrupt
* ���� : CAN2 �жϳ�ʼ������ʼʱ �����жϹر�
********************************************************************/
static void CAN2_Init_Interrupt( void)
{
  
  NVIC_InitTypeDef   NVIC_InitStructure;
  
  //----------�жϳ�ʼ�� ---------------------------------------
  /* Enable CAN2 RX0 interrupt IRQ channel */
  NVIC_InitStructure.NVIC_IRQChannel = CAN2_RX1_IRQn;		
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 8;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* Enable CAN2 TX interrupt IRQ channel */
  NVIC_InitStructure.NVIC_IRQChannel = CAN2_TX_IRQn;		
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 7;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  CAN_ITConfig(CAN2, CAN_IT_FMP1, DISABLE);//��CAN2�Ľ����ж�
  
}


/********************************************************************
* ���� : CAN2_Init_GPIO
* ���� : CAN2 GPIO�ڳ�ʼ������
********************************************************************/
static void CAN2_Init_GPIO( void )
{
 
  GPIO_InitTypeDef   GPIO_InitStructure;

  //--------- ʱ�ӳ�ʼ�� --------------------------
  //ʹ��/ʧ�� APB1,APB2����ʱ�� (CANʹ�ø��ù���,�������ù���ʱ��)

  /* Enable GPIO clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  //--------- ����IO�˿ڳ�ʼ�� ----------------------------

  /* CAN Periph clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN2, ENABLE);
  
  
//  GPIO_PinRemapConfig(GPIO_Remap_CAN,ENABLE); 
 // GPIO_PinRemapConfig(GPIO_Remap1_CAN1 , ENABLE);
  /* ��ӳ��CAN1��IO�� */
 // AFIO->MAPR |= GPIO_Remap2_CAN1;	//��ӳ�� Weite  ,dxl,2��û��AFIO

  /* Connect PB5 to CAN2 RX*/
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_CAN2);

  /* Connect PB6 to CAN2 TX*/
 // GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_CAN2);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource12 , GPIO_AF_CAN2);
  
  
  /* Configure CAN2 Tx as alternate function  */
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;

  
 //  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 ;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  /* Configure CAN2 Rx as alternate function  */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

}



/********************************************************************
* ���� : CAN2_BaudRate_Set
* ���� : CAN2 ͨ�Ų���������
* ���� :  BandRateBuf: CANͨ�Ų�����
********************************************************************/
static void CAN2_BaudRate_Set(u32 BandRateBuf)
{
  
  CAN_InitTypeDef        CAN_InitStructure;

  CAN_StructInit(&CAN_InitStructure);
 
  //------- CAN_MCR ------------
  CAN_InitStructure.CAN_TTCM = DISABLE; //ʹ��/ʧ�� ʱ�䴥��ͨѶģʽ.0:ʱ�䴥��ͨ��ģʽ�ر�;
  CAN_InitStructure.CAN_ABOM = DISABLE;  //ʹ��/ʧ�� �Զ����߹���.    1:һ����ص�128��11����������λ,�Զ��˳�����״̬;
  CAN_InitStructure.CAN_AWUM = ENABLE;  //ʹ��/ʧ�� �Զ�����ģʽ.    1:Ӳ����⵽CAN����ʱ�Զ��뿪����ģʽ;
  CAN_InitStructure.CAN_NART = ENABLE; //ʹ��/ʧ�� ���Զ��ش���ģʽ.0:CANӲ������ʧ�ܺ��һֱ�ط�ֱ�����ͳɹ�;
  CAN_InitStructure.CAN_RFLM = DISABLE; //ʹ��/ʧ�� �ܽ���FIFO����ģʽ.0:����FIFO����,��һ������ı��Ľ�����ǰһ��;
  CAN_InitStructure.CAN_TXFP = ENABLE;  //ʹ��/ʧ�� ����FIFO���ȼ�.    1:�ɷ��������˳��(ʱ���Ⱥ�˳��)���������ȼ�.

  //------- CAN_BTR ------------
  CAN_InitStructure.CAN_Mode =CAN_Mode_Normal ;//CANӲ������������ģʽ,CAN_Mode_LoopBack,CAN_Mode_Normal
  CAN_InitStructure.CAN_SJW  = CAN_SJW_1tq;     //����ͬ����Ծ���1��ʱ�䵥λ

	//CAN������: PCLK1 /  [(1 + CAN_BS1 + CAN_BS2) * CAN_Prescaler]
	//  Ki = (1 + CAN_BS1) / (1 + CAN_BS1 + CAN_BS2)
	//���⾡���ܵİѲ���������Ϊ CiA �Ƽ���ֵ��
	//                           Ki=75%     when ������ > 800K
	//                           Ki=80%     when ������ > 500K
	//                           Ki=87.5%   when ������ <= 500K
  CAN_InitStructure.CAN_BS1 = CAN_BS1_13tq;   //CAN_BS1_8tq;
  CAN_InitStructure.CAN_BS2 = CAN_BS2_2tq;    //CAN_BS2_7tq;
  CAN_InitStructure.CAN_Prescaler = 6;//(������:250K, PCLK1:24MHz, CAN_BS1:CAN_BS1_13tq, CAN_BS2:CAN_BS2_2tq, CAN_Prescaler:6)

//	CAN_InitStructure.CAN_BS2 = CAN_BS2_2tq;
//	if(BandRateBuf <= 500000L)         //BandRateBuf <= 500KHz
//	{
//		CAN_InitStructure.CAN_BS1 = CAN_BS1_13tq;
//		CAN_InitStructure.CAN_Prescaler = (RCC_ClocksStatus.PCLK1_Frequency/BandRateBuf)/16;
//	} else if(BandRateBuf <= 800000L){ //500KHz < BandRateBuf <= 800KHz
//		CAN_InitStructure.CAN_BS1 = CAN_BS1_7tq;
//		CAN_InitStructure.CAN_Prescaler = (RCC_ClocksStatus.PCLK1_Frequency/BandRateBuf)/10;
//	} else {                           //800KHz < BandRateBuf
//		CAN_InitStructure.CAN_BS1 = CAN_BS1_5tq;
//		CAN_InitStructure.CAN_Prescaler = (RCC_ClocksStatus.PCLK1_Frequency/BandRateBuf)/8;
//	}

  CAN_Init(CAN2, &CAN_InitStructure);
  
  
}

/********************************************************************
* ���� : CAN2_ID_Filter_Set
* ���� : CAN2 ����ID������
* ���� :  FilterNum: ����ID����ţ������ظ� �� ȡֵ��Χ ����1~13
           filterId: ����ID , ��28λΪ��Ч��IDλ�� ����λ����Ϊ0 
       filterMaskId: ��������λ�� 1: ��������  0:����
*******************************************************************
*/

void CAN2_ID_Filter_Set(u8 FilterNum, u32 filterId, u32 filterMaskId)
{
  
  CAN_FilterInitTypeDef  CAN_FilterInitStructure;
  CAN_FilterInitStructure.CAN_FilterNumber          = FilterNum+14;      //ָ���˴���ʼ���Ĺ����������ķ�Χ��14��27
//CAN_FilterInitStructure.CAN_FilterMode            = CAN_FilterMode_IdList; //CAN_FilterMode_IdMask;//����������ʼ��Ϊ��ʶ������λģʽ
  CAN_FilterInitStructure.CAN_FilterMode            = CAN_FilterMode_IdMask;//����������ʼ��Ϊ��ʶ������λģʽ
  CAN_FilterInitStructure.CAN_FilterScale           = CAN_FilterScale_32bit;//�����˹�����λ��
  
  CAN_FilterInitStructure.CAN_FilterIdHigh          = (((u32)filterId<<3)&0xFFFF0000)>>16;
  CAN_FilterInitStructure.CAN_FilterIdLow           = (((u32)filterId<<3)|0x0004)&0xFFFF;
  CAN_FilterInitStructure.CAN_FilterMaskIdHigh      = (((u32)filterMaskId<<3)&0xFFFF0000)>>16;
  CAN_FilterInitStructure.CAN_FilterMaskIdLow       = (((u32)filterMaskId<<3)|0x0007)&0xFFFF;
  
  CAN_FilterInitStructure.CAN_FilterFIFOAssignment  = CAN_FIFO1; //ͨ�����˲�����Ϣ����������FIFO0��
  CAN_FilterInitStructure.CAN_FilterActivation      = ENABLE;    //ʹ��/ʧ�� ������
  CAN_FilterInit(&CAN_FilterInitStructure);
  
  
}



/********************************************************************
* ���� : CAN2_Init_Reg
* ���� : CAN2 ��ؼĴ�����ʼ��
********************************************************************/
static void CAN2_Init_Reg(void)
{
  
  /* CAN Periph clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN2, ENABLE);

  CAN_DeInit(CAN2);
  
  CAN2_BaudRate_Set(250000L);//����CAN���ߵ� ������(250KHz).
  
  CAN2_ID_Filter_Set( 1, 0x00000000, 0x00000000);//���������е�

  
}



/********************************************************************
* ���� : UpdataOnePram_SamplePeriod_CAN2
* ���� : CAN����ͨ��2 �ɼ�ʱ��������ˢ��
* ˵�� : ����ID0x0102  CAN����ͨ��2 �ɼ�ʱ����(ms)��0 ��ʾ���ɼ� 
********************************************************************/
void UpdataOnePram_SamplePeriod_CAN2( void )
{
  
    u8 BufferTemp[6];

  if( E2_CAN2_GATHER_TIME_LEN == EepromPram_ReadPram( E2_CAN2_GATHER_TIME_ID , BufferTemp  ) )
  {
    
      gCANParame[iCAN2].CAN_SamplePeriodxMS = 0 ;   
      gCANParame[iCAN2].CAN_SamplePeriodxMS = BufferTemp[3];
      gCANParame[iCAN2].CAN_SamplePeriodxMS |= BufferTemp[2]<<8 ; 
      gCANParame[iCAN2].CAN_SamplePeriodxMS |= BufferTemp[1]<<16 ;
      gCANParame[iCAN2].CAN_SamplePeriodxMS |= BufferTemp[0]<<24 ;     

      
  } else {
    
       gCANParame[iCAN2].CAN_SamplePeriodxMS =20; //CAN����ͨ��1�ɼ�ʱ����(ms) , 0��ʾ���ɼ�   
    
  }
  
#if DEBUG_PRINT_CAN
  
     Print_RTC();
     LOG_PR( "UpdataOnePram_SamplePeriod_CAN2():  CAN2 Sample Period = %d (ms)   \r\n" , gCANParame[iCAN2].CAN_SamplePeriodxMS ); 

#endif   
    
  
  
  
}

/********************************************************************
* ���� : UpdataOnePram_UploadPeriod_CAN2
* ���� : CAN����ͨ��2 �ϴ�ʱ����(s)����ˢ��
* ˵�� : ����ID0x0103  CAN����ͨ��2 �ϴ�ʱ����(s)��0 ��ʾ���ϴ�
********************************************************************/
void UpdataOnePram_UploadPeriod_CAN2( void )
{
  u8 BufferTemp[4];

  if( E2_CAN2_UPLOAD_TIME_LEN == EepromPram_ReadPram(  E2_CAN2_UPLOAD_TIME_ID , BufferTemp   ) )
  {
    
      gCANParame[iCAN2].CAN_UploadPeriodxS = 0 ;
      gCANParame[iCAN2].CAN_UploadPeriodxS = BufferTemp[1];
      gCANParame[iCAN2].CAN_UploadPeriodxS |= BufferTemp[0]<<8 ;    
   
  
  } else {
    
      gCANParame[iCAN2].CAN_UploadPeriodxS = 30; //CAN����ͨ��1�ϴ�ʱ����(s), 0��ʾ���ϴ� 

  }
  
#if DEBUG_PRINT_CAN
  
     Print_RTC();
     LOG_PR( "UpdataOnePram_UploadPeriod_CAN2():  CAN2 Upload Period = %d (s)   \r\n" , gCANParame[iCAN2].CAN_UploadPeriodxS ); 

#endif   
   
  

}

/*******************************************************************
* ���� : CAN2_Init_Task
* ���� : CAN2 �����ʼ��
********************************************************************/
static void CAN2_Init_Task(void)
{

   /*****  CANȫ�ֱ�����ʼʱ��0  *****/
  memset( (u8 *)&gCANParame[iCAN2], 0, sizeof(PARAMETER_CAN ) );
  
//����ID0x0102//CAN ����ͨ��1 �ɼ�ʱ����(ms)��0 ��ʾ���ɼ� 

	UpdataOnePram_SamplePeriod_CAN2( );

//����ID0x0103//CAN ����ͨ��1 �ϴ�ʱ����(s)��0 ��ʾ���ϴ�
	UpdataOnePram_UploadPeriod_CAN2();
      
}


/********************************************************************
* ���� : Shigle_Set_Param_CAN2
* ���� : ����ʱ��ֱ���޸�CAN2 ������
********************************************************************/
void Shigle_Set_Param_CAN2( void )
{

  u8	Buffer[30]; 
  
  
//����ID0x0102//CAN ����ͨ��2 �ɼ�ʱ����(ms)��0 ��ʾ���ɼ�
//#define E2_CAN2_GATHER_TIME_LEN		        4

#if 0

    Buffer[0] = 0x00;
    Buffer[1] = 0x00;
    Buffer[2] = (200>>8 )&0xFF;  
    Buffer[3] = (200>>0 )&0xFF;
    EepromPram_WritePram(E2_CAN2_GATHER_TIME_ID,Buffer,4);
    
#endif
    

//����ID0x0103//CAN ����ͨ��2 �ϴ�ʱ����(s)��0 ��ʾ���ϴ�
//#define E2_CAN2_UPLOAD_TIME_LEN	2
#if 1
    Buffer[0] = (10>>8 )&0xFF;  
    Buffer[1] = (10>>0 )&0xFF;
    EepromPram_WritePram(E2_CAN2_UPLOAD_TIME_ID ,Buffer,2  );

#endif


}

/********************************************************************
* ���� : CAN2_Init
* ���� : CAN2 ��ʼ��
********************************************************************/
void CAN2_Init(void)
{
  
#if 0

  Shigle_Set_Param_CAN2();

#endif
  
  CAN2_Init_Task();
  
  CAN2_Init_GPIO();
  
  CAN2_Init_Reg();

  CAN2_Init_Interrupt();
  
}


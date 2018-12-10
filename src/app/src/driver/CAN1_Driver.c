/************************************************************************* 
* Copyright (c) 2013,�������������¼����������޹�˾
* All rights reserved.
* �ļ����� : CAN1_Driver.c
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
extern CAN_ISR_DATA  gCanIsrData[];
extern CAN_MASK_ID gFilterIDArray[];


/***  ���� ��������  ***/
static void CAN1_Init_Interrupt( void);
static void CAN1_Init_GPIO( void );
static void CAN1_BaudRate_Set(u32 BandRateBuf);
static void CAN1_Init_Reg(void);
static void CAN1_Init_Task(void);





/********************************************************************
* ���� : CAN1_Init_Interrupt
* ���� : CAN1 �жϳ�ʼ������ʼʱ �����жϹر�
********************************************************************/
static void CAN1_Init_Interrupt( void)
{
  
  NVIC_InitTypeDef   NVIC_InitStructure;
  
  //----------�жϳ�ʼ�� ---------------------------------------
  /* Enable CAN RX0 interrupt IRQ channel */
  
  NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn;		
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* Enable CAN TX interrupt IRQ channel */
  
  NVIC_InitStructure.NVIC_IRQChannel = CAN1_TX_IRQn;		
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
 //CAN_ITConfig(CAN1, CAN_IT_FMP0, DISABLE);//��ʼʱ�ر�CAN�Ľ����ж�
    CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);//��ʼʱ�ر�CAN�Ľ����ж�
}


/********************************************************************
* ���� : CAN1_Init_GPIO
* ���� : CAN1 GPIO�ڳ�ʼ������
********************************************************************/
static void CAN1_Init_GPIO( void )
{
  
  GPIO_InitTypeDef   GPIO_InitStructure;

  //--------- ʱ�ӳ�ʼ�� --------------------------
  
  /* Enable GPIO clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
  
  //--------- ����IO�˿ڳ�ʼ�� ----------------------------
  
  /* Connect PD0 to CAN1*/
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_CAN1);

  /* Connect PD1 to CAN1*/
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_CAN1);
	
  /* Configure USART Tx as alternate function  */
  
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
  
  /* Configure USART Rx as alternate function  */
  
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_Init(GPIOD, &GPIO_InitStructure);

  
}


/*
*******************************************************************
* ���� : CAN1_BaudRate_Set
* ���� : CAN1 ͨ�Ų���������
* ���� :  BandRateBuf: CANͨ�Ų�����
*******************************************************************
*/
static void CAN1_BaudRate_Set(u32 BandRateBuf)
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

  CAN_Init(CAN1, &CAN_InitStructure);
  
  
}


/********************************************************************
* ���� : CAN1_ID_Filter_Set
* ���� : CAN1 ����ID������
* ���� :  FilterNum: ����ID����ţ������ظ� �� ȡֵ��Χ ����1~13
           filterId: ����ID , ��28λΪ��Ч��IDλ�� ����λ����Ϊ0 
       filterMaskId: ��������λ�� 1: ��������  0:����
********************************************************************/
void CAN1_ID_Filter_Set(u8 FilterNum, u32 filterId, u32 filterMaskId)
{
  
  CAN_FilterInitTypeDef  CAN_FilterInitStructure;
  
  CAN_FilterInitStructure.CAN_FilterNumber          = FilterNum;      //ָ���˴���ʼ���Ĺ����������ķ�Χ��1��13
//CAN_FilterInitStructure.CAN_FilterMode            = CAN_FilterMode_IdList; //CAN_FilterMode_IdMask;//����������ʼ��Ϊ��ʶ������λģʽ
  CAN_FilterInitStructure.CAN_FilterMode            = CAN_FilterMode_IdMask;//����������ʼ��Ϊ��ʶ������λģʽ
  CAN_FilterInitStructure.CAN_FilterScale           = CAN_FilterScale_32bit;//�����˹�����λ��
  
  CAN_FilterInitStructure.CAN_FilterIdHigh          = (((u32)filterId<<3)&0xFFFF0000)>>16;
  CAN_FilterInitStructure.CAN_FilterIdLow           = (((u32)filterId<<3)|0x0004)&0xFFFF;
  CAN_FilterInitStructure.CAN_FilterMaskIdHigh      = (((u32)filterMaskId<<3)&0xFFFF0000)>>16;
  CAN_FilterInitStructure.CAN_FilterMaskIdLow       = (((u32)filterMaskId<<3)|0x0007)&0xFFFF;
  
  CAN_FilterInitStructure.CAN_FilterFIFOAssignment  = CAN_FIFO0; //ͨ�����˲�����Ϣ����������FIFO0��
  CAN_FilterInitStructure.CAN_FilterActivation      = ENABLE;    //ʹ��/ʧ�� ������
  CAN_FilterInit(&CAN_FilterInitStructure);

}


/********************************************************************
* ���� : CAN1_Init_Reg
* ���� : CAN1 ��ؼĴ�����ʼ��
********************************************************************/
static void CAN1_Init_Reg(void)
{

  /* CAN Periph clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
  
  CAN_DeInit(CAN1);
  
  CAN1_BaudRate_Set(250000L);//����CAN���ߵ� ������(250KHz).
  
  CAN1_ID_Filter_Set( 1, 0x00000000, 0x00000000);//���������е�
        
  //CanBus_SetIdentifie( 2, 0x18FFD117, 0x0000FFFF);
  
}






/********************************************************************
* ���� : UpdataOnePram_SamplePeriod_CAN1
* ���� : CAN����ͨ��1 �ɼ�ʱ��������ˢ��
* ˵�� : ����ID0x0100  CAN����ͨ��1 �ɼ�ʱ����(ms)��0 ��ʾ���ɼ� 
********************************************************************/
void UpdataOnePram_SamplePeriod_CAN1( void )
{

  u8 BufferTemp[6];
  if( E2_CAN1_GATHER_TIME_LEN == EepromPram_ReadPram( E2_CAN1_GATHER_TIME_ID , BufferTemp ) )
  {
      gCANParame[iCAN1].CAN_SamplePeriodxMS = 0 ;
      gCANParame[iCAN1].CAN_SamplePeriodxMS = BufferTemp[3];
      gCANParame[iCAN1].CAN_SamplePeriodxMS |= BufferTemp[2]<<8 ; 
      gCANParame[iCAN1].CAN_SamplePeriodxMS |= BufferTemp[1]<<16 ;
      gCANParame[iCAN1].CAN_SamplePeriodxMS |= BufferTemp[0]<<24 ;

  }else{
    
      gCANParame[iCAN1].CAN_SamplePeriodxMS =20; //CAN����ͨ��1�ɼ�ʱ����(ms) , 0��ʾ���ɼ�
  }
  
  	
	
#if DEBUG_PRINT_CAN
  
     Print_RTC();
     LOG_PR( "UpdataOnePram_SamplePeriod_CAN1():  CAN1 Sample Period = %d (ms) \r\n" , gCANParame[iCAN1].CAN_SamplePeriodxMS ); 

#endif   
  
  
  
  
}


/********************************************************************
* ���� : UpdataOnePram_UploadPeriod_CAN1
* ���� : CAN����ͨ��1 �ϴ�ʱ����(s)����ˢ��
* ˵�� : ����ID0x0101  CAN ����ͨ��1 �ϴ�ʱ����(s)��0 ��ʾ���ϴ�
********************************************************************/
void UpdataOnePram_UploadPeriod_CAN1( void )
{
  
  u8 BufferTemp[4];
  
  if( E2_CAN1_UPLOAD_TIME_LEN ==  EepromPram_ReadPram( E2_CAN1_UPLOAD_TIME_ID ,  BufferTemp ) )
  {
    
      gCANParame[iCAN1].CAN_UploadPeriodxS = 0;
      gCANParame[iCAN1].CAN_UploadPeriodxS = BufferTemp[1];
      gCANParame[iCAN1].CAN_UploadPeriodxS |= BufferTemp[0]<<8 ;
      
  } else {

      gCANParame[iCAN1].CAN_UploadPeriodxS = 30; //CAN����ͨ��1�ϴ�ʱ����(s), 0��ʾ���ϴ�
      
  }
  
  
#if DEBUG_PRINT_CAN
  
     Print_RTC();
     LOG_PR( "UpdataOnePram_UploadPeriod_CAN1():  CAN1 Upload Period = %d (s) \r\n" , gCANParame[iCAN1].CAN_UploadPeriodxS ); 

#endif   
  
  
  
}

/********************************************************************
* ���� : CAN1_Init_Task
* ���� : CAN1 �����ʼ��
********************************************************************/
static void CAN1_Init_Task(void)
{
  
   /*****  CANȫ�ֱ�����ʼʱ��0  *****/
  memset( (u8 *)&gCanIsrData[iCAN1], 0, sizeof(CAN_ISR_DATA) );
   /*****  CANȫ�ֱ�����ʼʱ��0  *****/
  memset( (u8 *)&gCANParame[iCAN1], 0, sizeof(PARAMETER_CAN ) );
  
//����ID0x0100//CAN ����ͨ��1 �ɼ�ʱ����(ms)��0 ��ʾ���ɼ� 
  
	UpdataOnePram_SamplePeriod_CAN1();
        
//����ID0x0101//CAN ����ͨ��1 �ϴ�ʱ����(s)��0 ��ʾ���ϴ�
	UpdataOnePram_UploadPeriod_CAN1();

 //����ID0x0110//CAN ����ID �����ɼ�����BYTE[8]
 
	//UpdataOnePram_Sample_Single_CAN1();
        
}



/********************************************************************
* ���� : Shigle_Set_Param_CAN1
* ���� : ����ʱ��ֱ���޸�CAN1 ������
********************************************************************/
void Shigle_Set_Param_CAN1( void )
{

  u8	Buffer[30]; 
  
  
//����ID0x0100//CAN ����ͨ��1 �ɼ�ʱ����(ms)��0 ��ʾ���ɼ�
//#define E2_CAN1_GATHER_TIME_LEN		        4

#if 1

    Buffer[0] = 0x00;
    Buffer[1] = 0x00;
    Buffer[2] = (200>>8 )&0xFF;  
    Buffer[3] = (200>>0 )&0xFF;
    EepromPram_WritePram(E2_CAN1_GATHER_TIME_ID,Buffer,4);
    
#endif
    

//����ID0x0101//CAN ����ͨ��1 �ϴ�ʱ����(s)��0 ��ʾ���ϴ�
    
//#define E2_CAN1_UPLOAD_TIME_LEN	2
#if 1
    Buffer[0] = (10>>8 )&0xFF;  
    Buffer[1] = (10>>0 )&0xFF;
    EepromPram_WritePram(E2_CAN1_UPLOAD_TIME_ID ,Buffer,2  );
    
#endif

}



/********************************************************************
* ���� : CAN1_Init
* ���� : CAN1 ��ʼ��
********************************************************************/
void CAN1_Init(void)
{

#if 0
 Shigle_Set_Param_CAN1();
 
#endif
 

  
  CAN1_Init_GPIO();
  
  CAN1_Init_Reg();

  CAN1_Init_Interrupt();
  
  CAN1_Init_Task();
  
}

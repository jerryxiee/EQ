/************************************************************************* 
* Copyright (c) 2013,深圳市伊爱高新技术开发有限公司
* All rights reserved.
* 文件名称 : CAN1_Driver.c
* 当前版本 : 1.0
* 开发者   : Shigle
* 修改时间 : 20113年6月10日
*************************************************************************/

/***  包含文件  ***/

#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "include.h"

/***  外部引用变量  ***/

extern PARAMETER_CAN gCANParame[];
extern CAN_ISR_DATA  gCanIsrData[];
extern CAN_MASK_ID gFilterIDArray[];


/***  本地 函数声明  ***/
static void CAN1_Init_Interrupt( void);
static void CAN1_Init_GPIO( void );
static void CAN1_BaudRate_Set(u32 BandRateBuf);
static void CAN1_Init_Reg(void);
static void CAN1_Init_Task(void);





/********************************************************************
* 名称 : CAN1_Init_Interrupt
* 功能 : CAN1 中断初始化，初始时 接收中断关闭
********************************************************************/
static void CAN1_Init_Interrupt( void)
{
  
  NVIC_InitTypeDef   NVIC_InitStructure;
  
  //----------中断初始化 ---------------------------------------
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
  
 //CAN_ITConfig(CAN1, CAN_IT_FMP0, DISABLE);//初始时关闭CAN的接收中断
    CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);//初始时关闭CAN的接收中断
}


/********************************************************************
* 名称 : CAN1_Init_GPIO
* 功能 : CAN1 GPIO口初始化设置
********************************************************************/
static void CAN1_Init_GPIO( void )
{
  
  GPIO_InitTypeDef   GPIO_InitStructure;

  //--------- 时钟初始化 --------------------------
  
  /* Enable GPIO clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
  
  //--------- 复用IO端口初始化 ----------------------------
  
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
* 名称 : CAN1_BaudRate_Set
* 功能 : CAN1 通信波特率设置
* 输入 :  BandRateBuf: CAN通信波特率
*******************************************************************
*/
static void CAN1_BaudRate_Set(u32 BandRateBuf)
{
  
  CAN_InitTypeDef        CAN_InitStructure;
  
  CAN_StructInit(&CAN_InitStructure);
  
  //------- CAN_MCR ------------
  CAN_InitStructure.CAN_TTCM = DISABLE; //使能/失能 时间触发通讯模式.0:时间触发通信模式关闭;
  CAN_InitStructure.CAN_ABOM = DISABLE;  //使能/失能 自动离线管理.    1:一旦监控到128次11个连续隐形位,自动退出离线状态;
  CAN_InitStructure.CAN_AWUM = ENABLE;  //使能/失能 自动唤醒模式.    1:硬件检测到CAN报文时自动离开休眠模式;
  CAN_InitStructure.CAN_NART = ENABLE; //使能/失能 非自动重传输模式.0:CAN硬件发送失败后会一直重发直到发送成功;
  CAN_InitStructure.CAN_RFLM = DISABLE; //使能/失能 能接收FIFO锁定模式.0:接收FIFO满了,下一条传入的报文将覆盖前一条;
  CAN_InitStructure.CAN_TXFP = ENABLE;  //使能/失能 发送FIFO优先级.    1:由发送请求的顺序(时间先后顺序)来决定优先级.

  //------- CAN_BTR ------------
  CAN_InitStructure.CAN_Mode =CAN_Mode_Normal ;//CAN硬件工作在正常模式,CAN_Mode_LoopBack,CAN_Mode_Normal
  CAN_InitStructure.CAN_SJW  = CAN_SJW_1tq;     //重新同步跳跃宽度1个时间单位

	//CAN波特率: PCLK1 /  [(1 + CAN_BS1 + CAN_BS2) * CAN_Prescaler]
	//  Ki = (1 + CAN_BS1) / (1 + CAN_BS1 + CAN_BS2)
	//另外尽可能的把采样点设置为 CiA 推荐的值：
	//                           Ki=75%     when 波特率 > 800K
	//                           Ki=80%     when 波特率 > 500K
	//                           Ki=87.5%   when 波特率 <= 500K
  CAN_InitStructure.CAN_BS1 = CAN_BS1_13tq;   //CAN_BS1_8tq;
  CAN_InitStructure.CAN_BS2 = CAN_BS2_2tq;    //CAN_BS2_7tq;
  CAN_InitStructure.CAN_Prescaler = 6;//(波特率:250K, PCLK1:24MHz, CAN_BS1:CAN_BS1_13tq, CAN_BS2:CAN_BS2_2tq, CAN_Prescaler:6)

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
* 名称 : CAN1_ID_Filter_Set
* 功能 : CAN1 过滤ID号设置
* 输入 :  FilterNum: 过滤ID的序号，不可重复 ， 取值范围 整数1~13
           filterId: 过滤ID , 低28位为有效的ID位， 其余位保留为0 
       filterMaskId: 过滤屏蔽位， 1: 表必须符合  0:随意
********************************************************************/
void CAN1_ID_Filter_Set(u8 FilterNum, u32 filterId, u32 filterMaskId)
{
  
  CAN_FilterInitTypeDef  CAN_FilterInitStructure;
  
  CAN_FilterInitStructure.CAN_FilterNumber          = FilterNum;      //指定了待初始化的过滤器，它的范围是1到13
//CAN_FilterInitStructure.CAN_FilterMode            = CAN_FilterMode_IdList; //CAN_FilterMode_IdMask;//过滤器被初始化为标识符屏蔽位模式
  CAN_FilterInitStructure.CAN_FilterMode            = CAN_FilterMode_IdMask;//过滤器被初始化为标识符屏蔽位模式
  CAN_FilterInitStructure.CAN_FilterScale           = CAN_FilterScale_32bit;//给出了过滤器位宽
  
  CAN_FilterInitStructure.CAN_FilterIdHigh          = (((u32)filterId<<3)&0xFFFF0000)>>16;
  CAN_FilterInitStructure.CAN_FilterIdLow           = (((u32)filterId<<3)|0x0004)&0xFFFF;
  CAN_FilterInitStructure.CAN_FilterMaskIdHigh      = (((u32)filterMaskId<<3)&0xFFFF0000)>>16;
  CAN_FilterInitStructure.CAN_FilterMaskIdLow       = (((u32)filterMaskId<<3)|0x0007)&0xFFFF;
  
  CAN_FilterInitStructure.CAN_FilterFIFOAssignment  = CAN_FIFO0; //通过此滤波器信息包将被放在FIFO0里
  CAN_FilterInitStructure.CAN_FilterActivation      = ENABLE;    //使能/失能 过滤器
  CAN_FilterInit(&CAN_FilterInitStructure);

}


/********************************************************************
* 名称 : CAN1_Init_Reg
* 功能 : CAN1 相关寄存器初始化
********************************************************************/
static void CAN1_Init_Reg(void)
{

  /* CAN Periph clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
  
  CAN_DeInit(CAN1);
  
  CAN1_BaudRate_Set(250000L);//设置CAN总线的 波特率(250KHz).
  
  CAN1_ID_Filter_Set( 1, 0x00000000, 0x00000000);//不屏蔽所有的
        
  //CanBus_SetIdentifie( 2, 0x18FFD117, 0x0000FFFF);
  
}






/********************************************************************
* 名称 : UpdataOnePram_SamplePeriod_CAN1
* 功能 : CAN总线通道1 采集时间间隔参数刷新
* 说明 : 参数ID0x0100  CAN总线通道1 采集时间间隔(ms)，0 表示不采集 
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
    
      gCANParame[iCAN1].CAN_SamplePeriodxMS =20; //CAN总线通道1采集时间间隔(ms) , 0表示不采集
  }
  
  	
	
#if DEBUG_PRINT_CAN
  
     Print_RTC();
     LOG_PR( "UpdataOnePram_SamplePeriod_CAN1():  CAN1 Sample Period = %d (ms) \r\n" , gCANParame[iCAN1].CAN_SamplePeriodxMS ); 

#endif   
  
  
  
  
}


/********************************************************************
* 名称 : UpdataOnePram_UploadPeriod_CAN1
* 功能 : CAN总线通道1 上传时间间隔(s)参数刷新
* 说明 : 参数ID0x0101  CAN 总线通道1 上传时间间隔(s)，0 表示不上传
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

      gCANParame[iCAN1].CAN_UploadPeriodxS = 30; //CAN总线通道1上传时间间隔(s), 0表示不上传
      
  }
  
  
#if DEBUG_PRINT_CAN
  
     Print_RTC();
     LOG_PR( "UpdataOnePram_UploadPeriod_CAN1():  CAN1 Upload Period = %d (s) \r\n" , gCANParame[iCAN1].CAN_UploadPeriodxS ); 

#endif   
  
  
  
}

/********************************************************************
* 名称 : CAN1_Init_Task
* 功能 : CAN1 任务初始化
********************************************************************/
static void CAN1_Init_Task(void)
{
  
   /*****  CAN全局变量初始时清0  *****/
  memset( (u8 *)&gCanIsrData[iCAN1], 0, sizeof(CAN_ISR_DATA) );
   /*****  CAN全局变量初始时清0  *****/
  memset( (u8 *)&gCANParame[iCAN1], 0, sizeof(PARAMETER_CAN ) );
  
//参数ID0x0100//CAN 总线通道1 采集时间间隔(ms)，0 表示不采集 
  
	UpdataOnePram_SamplePeriod_CAN1();
        
//参数ID0x0101//CAN 总线通道1 上传时间间隔(s)，0 表示不上传
	UpdataOnePram_UploadPeriod_CAN1();

 //参数ID0x0110//CAN 总线ID 单独采集设置BYTE[8]
 
	//UpdataOnePram_Sample_Single_CAN1();
        
}



/********************************************************************
* 名称 : Shigle_Set_Param_CAN1
* 功能 : 调试时，直接修改CAN1 参数项
********************************************************************/
void Shigle_Set_Param_CAN1( void )
{

  u8	Buffer[30]; 
  
  
//参数ID0x0100//CAN 总线通道1 采集时间间隔(ms)，0 表示不采集
//#define E2_CAN1_GATHER_TIME_LEN		        4

#if 1

    Buffer[0] = 0x00;
    Buffer[1] = 0x00;
    Buffer[2] = (200>>8 )&0xFF;  
    Buffer[3] = (200>>0 )&0xFF;
    EepromPram_WritePram(E2_CAN1_GATHER_TIME_ID,Buffer,4);
    
#endif
    

//参数ID0x0101//CAN 总线通道1 上传时间间隔(s)，0 表示不上传
    
//#define E2_CAN1_UPLOAD_TIME_LEN	2
#if 1
    Buffer[0] = (10>>8 )&0xFF;  
    Buffer[1] = (10>>0 )&0xFF;
    EepromPram_WritePram(E2_CAN1_UPLOAD_TIME_ID ,Buffer,2  );
    
#endif

}



/********************************************************************
* 名称 : CAN1_Init
* 功能 : CAN1 初始化
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

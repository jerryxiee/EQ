/*************************************************************************
* Copyright (c) 2013,�������������¼����������޹�˾
* All rights reserved.
* �ļ����� : Module_CAN.c
* ��ǰ�汾 : 1.0
* ������   : Shigle
* �޸�ʱ�� : 2013��6��10��

* 2013��9��18��Ϊ���߰汾1.0, �������������������޸ģ���Ҫ������׷��˵��
* 1����ʱֻ������CAN1�Ĺ��ܣ�CAN2 �Ĺ��������� ���Ҫ�����������ļ�Module_CAN.h���޸ĺ� #define CAN_CH_MAX 1 //CANͨ������� 1:ֻ����CAN1 2:ͬʱ����CAN1��CAN2
* 2�����ǵ��ڴ�ռ����ƣ�Ŀǰ�ϱ�CAN����ͬʱ����ϱ����ڻ����δ�ϴ�����Ч�������ܺ�CAN_MAX_RECE_NUM���ƣ���
* 3��������Ҫ����808-2012��Э��������ϴ���δ���������������ͻ�Э��
*************************************************************************/

/***  �����ļ�  ***/ 

#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "include.h"


/***  �ⲿ ���ú���  ***/

extern void RTC_GetCurTime(TIME_T *tt);


/***  ���� �����ȫ�ֱ���  ***/


CAN_REC  CanData;
PARAMETER_CAN   gCANParame[CAN_CH_MAX];
PARAMETER_CAN_SINGLE  gCANParameSingle;

u8  gIsCanOpen;

CAN_ISR_DATA    gCanIsrData[CAN_CH_MAX];
CAN_MASK_ID     gFilterIDArray[CAN_CH_MAX];

u16   gCANNum_Radio[CAN_CH_MAX];
u16    gCANNum_Rx[CAN_CH_MAX];


/********************************************************************
* ���� : CAN_PowerCtrl_Init_GPIO
* ���� : ��ʼ�� CAN�������Ӧ��IO��.
********************************************************************/

void CAN_PowerCtrl_Init_GPIO( void )
{
  
  GPIO_InitTypeDef   GPIO_InitStructure;
  
  //-----------------CAN��Դ����-----------------Weite
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
  
}



/********************************************************************
* ���� : DecodeCan_EvTask
* ���� : ����CAN���յ����ݰ�
********************************************************************/

void DecodeCan_EvTask()
{
  
}


/*
*******************************************************************
* ���� : Time_Now_Get_BCD
* ���� : ��ȡ��ǰ��RTCʱ��,ת����Э��Ҫ���BCD��ʽ
* ��� : ����BCDʱ���ָ��
*******************************************************************
*/
void Time_Now_Get_BCD( u8 * pTimer )
{
 
   TIME_T nowTime;
   RTC_GetCurTime(&nowTime);
   
   *(pTimer +0)=( ( nowTime.year/10 )<<4 )  + (( nowTime.year  %10)  &0x0F )            ; //��:00~99
   *(pTimer +1)=( ( nowTime.month/10)<<4 )  + (( nowTime.month %10)  &0x0F )            ; //��:1~12
   *(pTimer +2)=( ( nowTime.day/10  )<<4 )  + (( nowTime.day   %10)  &0x0F )            ; //��:1~31
   *(pTimer +3)=( ( nowTime.hour/10 )<<4 )  + (( nowTime.hour  %10)  &0x0F )            ; //ʱ:0~23
   *(pTimer +4)=( ( nowTime.min/10  )<<4 )  + (( nowTime.min   %10)  &0x0F )            ; //��:0~59
   *(pTimer +5)=( ( nowTime.sec/10  )<<4 )  + (( nowTime.sec   %10)  &0x0F )            ; //��:0~59
                
}


/*
*******************************************************************
* ���� : CAN_Isr_Rx
* ���� : �ɼ�CAN�����жϵ�����
* ���� :  * CANx  : STEM32 CAN�ṹ��ָ��
*******************************************************************
*/

void CAN_Isr_Rx( CAN_TypeDef* CANx )
{
  u8 NowTime[6];
  CanRxMsg  RxMessage;
  u8  num;
  u8  iCANCh ;
        
  if( CANx == CAN1 )
  {
    CAN_Receive( CANx, CAN_FIFO0, &RxMessage);
    iCANCh = iCAN1 ;
  }
  else
  {
    CAN_Receive( CANx, CAN_FIFO1, &RxMessage);
    iCANCh = iCAN2 ;
  }
  
  num = gCanIsrData[iCANCh].receIndex;
 
  Time_Now_Get_BCD(  NowTime );
  gCanIsrData[iCANCh].receBuffer[num].CAN_Time[0] = NowTime[3];	//ʱ:0~23
  gCanIsrData[iCANCh].receBuffer[num].CAN_Time[1] = NowTime[4];	//��:0~59
  gCanIsrData[iCANCh].receBuffer[num].CAN_Time[2] = NowTime[5];	//��:0~59

  
  if(RxMessage.IDE == CAN_ID_STD)
  {
    

    gCanIsrData[iCANCh].receBuffer[num].CAN_ID_BYTE_3 =0;
    gCanIsrData[iCANCh].receBuffer[num].CAN_ID_BYTE_2 = 0;
    gCanIsrData[iCANCh].receBuffer[num].CAN_ID_BYTE_1 = (RxMessage.StdId>>8)&0x07;
    gCanIsrData[iCANCh].receBuffer[num].CAN_ID_BYTE_0 = RxMessage.StdId;       

    memcpy((u8*)&gCanIsrData[iCANCh].receBuffer[num].CAN_ID_BYTE_0, (u8*)(&RxMessage.StdId), 4);
    
    gCanIsrData[iCANCh].receBuffer[num].CAN_Frame_Type = 0; //bit 30: ��ʾ֡����: 0:��׼֡ 1:��չ֡
  }
  else
  {
    gCanIsrData[iCANCh].receBuffer[num].CAN_ID_BYTE_3 =(RxMessage.ExtId>>24)&0x1F;
    gCanIsrData[iCANCh].receBuffer[num].CAN_ID_BYTE_2 = RxMessage.ExtId>>16;
    gCanIsrData[iCANCh].receBuffer[num].CAN_ID_BYTE_1 = RxMessage.ExtId>>8;
    gCanIsrData[iCANCh].receBuffer[num].CAN_ID_BYTE_0 = RxMessage.ExtId;    

 
  //  memcpy((u8*)&gCanIsrData[iCANCh].receBuffer[num].CAN_ID_BYTE_0, (u8*)(&RxMessage.ExtId), 4);
    gCanIsrData[iCANCh].receBuffer[num].CAN_Frame_Type = 1; //bit 30: ��ʾ֡����: 0:��׼֡ 1:��չ֡
    
  }
  
   gCanIsrData[iCANCh].receBuffer[num].CAN_Data_Original = 0; //bit 29: ��ʾ���ݲɼ���ʽ: 0:ԭʼ���� 1: �ɼ�����ļ���ֵ
        

   gCanIsrData[iCANCh].receBuffer[num].CAN_Channel = iCANCh; //bit31: ��ʾCANͨ����, 0: CAN1 1: CAN2
  
  
  //i = 4;
  //gCanIsrData.receBuffer[num][i] = ((RxMessage.IDE | RxMessage.RTR) << 4) | (RxMessage.DLC);
   //i++;
   memcpy((u8*)&gCanIsrData[iCANCh].receBuffer[num].CAN_Data[0], RxMessage.Data, RxMessage.DLC );
  
    //ѭ������
    gCANNum_Rx[iCANCh]++; //ͳ��CAN���յ����ݰ����� Shigle 2013-06-23
    
    gCanIsrData[iCANCh].receIndex++;
    if(gCanIsrData[iCANCh].receIndex >= CAN_MAX_RECE_NUM)
    {
        gCanIsrData[iCANCh].receIndex = 0;
    }
    
    //����δ������
    gCanIsrData[iCANCh].receNum++;
    if(gCanIsrData[iCANCh].receNum >= CAN_MAX_RECE_NUM)
    {
        gCanIsrData[iCANCh].receNum = CAN_MAX_RECE_NUM;
    }
    
#if 0
    if(gCanIsrData[iCANCh].receNum >= CAN_MAX_RECE_NUM)
    {
        gCanIsrData[iCANCh].receNum = CAN_MAX_RECE_NUM;
        gCanIsrData[iCANCh].UnreadIndex = gCanIsrData[iCANCh].receIndex;
    }
    else
    {
        gCanIsrData[iCANCh].UnreadIndex = 0;
    }
#endif
    
}


/*******************************************************************
* ���� : CAN1_Sample_Sw
* ���� : CAN ��������
* ���� : 
*       iCANChannel:    0: CAN1ͨ��   1: CAN2ͨ��
*            bSwCtr:    0: �ر�CAN�����ж�  1: ����CAN1�����ж�   
*******************************************************************/
void CAN_Sample_Sw( u8 iCANChannel , u8 bSwCtr )
{
  
  if( iCAN1 == iCANChannel ){
    
      if(  bSwCtr ){
        
            CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);//��CAN1�Ľ����ж�
        } else {
          
          CAN_ITConfig(CAN1, CAN_IT_FMP0, DISABLE);//�ر�CAN1�Ľ����ж� 
        } 
      
  } else if( iCAN2 == iCANChannel ) {

       if(  bSwCtr ){
         
            CAN_ITConfig(CAN2, CAN_IT_FMP1, ENABLE);//��CAN2�Ľ����ж�
            
        } else {
          
            CAN_ITConfig(CAN2, CAN_IT_FMP1, DISABLE);//�ر�CAN2�Ľ����ж�
        }    
     }
  
  
}
  

/*******************************************************************
* ���� : CAN_Packet_Single_ID
* ���� : ���CAN����
* ���� : 
*       iCANCh:    0: CAN1ͨ��   1: CAN2ͨ��
* ��� :
*        pdata:    ���հ����ݵ�ָ��
*******************************************************************/
u16 CAN_Packet_Single_ID( u8 iCANCh,  u8 *pdata )
{
  
  u8  readindex;
  u16 retlenght = 0 ;
  u8 Flag_First_Valid_CANPacket = 0 ;
  u16 CAN_Item_TotalNum = 0;
  u16 CAN_Item_TotalNum_ID0 = 0;
  u8  *dataAddr;
  u8  k ;

  //static u8 TempAA[15];
  
  assert_param((pdata != NULL));
  assert_param(( iCANCh < CAN_CH_MAX ));  

  dataAddr  =  pdata+2;//�������ֽ������ݰ� ��

  //���յ�CAN�������������
  
#if DEBUG_PRINT_CAN
  
      Print_RTC();
      LOG_PR( "CAN%d : receNum = %d ; readindex = %d ; " , iCANCh+1 , gCanIsrData[iCANCh].receNum, gCanIsrData[iCANCh].UnreadIndex  ); 
  //    LOG_PR_N( "\r\n");
#endif    
                
  
  Flag_First_Valid_CANPacket = 0;
  CAN_Item_TotalNum = gCanIsrData[iCANCh].receNum ;
  gCanIsrData[iCANCh].receNum = gCanIsrData[iCANCh].receNum - CAN_Item_TotalNum;

 
 //gCanIsrData[iCANCh].UnreadIndex =  ( gCanIsrData[iCANCh].UnreadIndex + CAN_Item_TotalNum )%CAN_MAX_RECE_NUM ;
  
  
  
  
  while( ( CAN_Item_TotalNum > 0 )&&( CAN_Item_TotalNum_ID0 < 50) )
  {
  //  for( iIDNum = 0; iIDNum < 2 ; iIDNum++ ){
    
      readindex = gCanIsrData[iCANCh].UnreadIndex; //��δ����������ʼ��ȡCAN���ݰ�����
    
      for( k = 0; k < MAX_NUM_FILTER_ID ; k++ ){
        
        if(
           ( !gCANParameSingle.CAN_Sample_Single[ k ].CAN_Sample_Interval )&&
           ( gCANParameSingle.CAN_Sample_Single[ k ].CAN_ID_BYTE_3 ==  gCanIsrData[0].receBuffer[readindex].CAN_ID_BYTE_0 )&& //����֡ID
           ( gCANParameSingle.CAN_Sample_Single[ k ].CAN_ID_BYTE_2 == gCanIsrData[0].receBuffer[readindex].CAN_ID_BYTE_1 )&&
           ( gCANParameSingle.CAN_Sample_Single[ k ].CAN_ID_BYTE_1 == gCanIsrData[0].receBuffer[readindex].CAN_ID_BYTE_2 )&&
           ( gCANParameSingle.CAN_Sample_Single[ k ].CAN_ID_BYTE_0 == gCanIsrData[0].receBuffer[readindex].CAN_ID_BYTE_3 )
        //   ( gCANParame[iIDNum].CAN_Sample_Single[ k ].CAN_Frame_Type == gCanIsrData[iCANCh].receBuffer[readindex].CAN_Frame_Type ) //����֡����
         ){
              break; //һ�������������׵ģ�������������ѭ��
          } 
      }
      
      if( k !=  MAX_NUM_FILTER_ID  ){
        
            gCanIsrData[iCANCh].UnreadIndex = ( gCanIsrData[iCANCh].UnreadIndex +1)%CAN_MAX_RECE_NUM ; ////δ��ȡ�������ƶ�һ��
            CAN_Item_TotalNum--;
          
      }else{
             
          //��һ��CAN���ݽ��յ�ʱ��
          if( ! Flag_First_Valid_CANPacket){
            
            Flag_First_Valid_CANPacket = 1;
            
            *dataAddr++ =   gCanIsrData[0].receBuffer[readindex].CAN_Time[0] ;
            *dataAddr++ =   gCanIsrData[0].receBuffer[readindex].CAN_Time[1] ;
            *dataAddr++ =   gCanIsrData[0].receBuffer[readindex].CAN_Time[2] ;
            *dataAddr++ =   gCanIsrData[0].receBuffer[readindex].CAN_Time[3] ; 
            *dataAddr++ =   gCanIsrData[0].receBuffer[readindex].CAN_Time[4] ;
            retlenght +=5;
          }
          
          memcpy( dataAddr, (u8*)&gCanIsrData[0].receBuffer[readindex], sizeof(CAN_ITEM)- 5 );//readindex, cpyLen,
     //     memcpy( TempAA, (u8*)&gCanIsrData[iCANCh].receBuffer[readindex], sizeof(CAN_ITEM)- 5 );//readindex, cpyLen,

          CAN_Item_TotalNum_ID0++ ; //���������++
          dataAddr += (sizeof(CAN_ITEM)-5); //�ֽڵ���
          retlenght += (sizeof(CAN_ITEM)- 5) ; //���ݳ�������
          gCANNum_Radio[iCANCh]++ ;  //ͳ�Ʒ��͵�CAN������
          
          gCanIsrData[iCANCh].UnreadIndex = (gCanIsrData[iCANCh].UnreadIndex+1)%CAN_MAX_RECE_NUM ; ////δ��ȡ�������ƶ�һ��
          CAN_Item_TotalNum--;
          
      }
      
  }
  
  gCanIsrData[iCANCh].receNum += CAN_Item_TotalNum ; //��δ����� ��Ŀ�� �����ۼ�
 // gCanIsrData[iCANCh].UnreadIndex = readindex ;
  
  
#if DEBUG_PRINT_CAN
  
   //   RTC_Print();
      LOG_PR( " CurItemNum = %d ; RadioSum = %d ; RxSum = %d ; " , CAN_Item_TotalNum_ID0,  gCANNum_Radio[iCANCh] ,  gCANNum_Rx[iCANCh]); 
      LOG_PR_N( "\r\n");
#endif    
               
  

  //���  ��������� ������
  if( CAN_Item_TotalNum_ID0){
    
        dataAddr  =  pdata;//���ص����ֽ������ݰ� ��
        *dataAddr++ = CAN_Item_TotalNum_ID0>>8;
        *dataAddr++ = CAN_Item_TotalNum_ID0&0x0FF ;
        retlenght +=2;

  } else {
    
       retlenght = 0 ;
  }
  
    return( retlenght );
        
}



/*******************************************************************
* ���� : ����ID0x0110~0x01FF  ����ID �����ɼ���������
* ���� : CAN ����ID �����ɼ�����
* ���� :  CANParmamerID : CAN����ID  Ŀǰֻ��ȡֵ0x0110�� 0x0111
*******************************************************************/
void UpdataOnePram_Sample_Single(  u32 CANParmamerID  )
{
  
  u8 BufferTemp[10];
  u32 Temp32;
//  u8 iCANChannel;
  u32 TempID ;
  u8 iItem;
  
  
#if 0 //����ʱֱ���޸Ĳ���
  
  u8 CANParam0[] ={ 0x00 , 0x00 , 0x00 , 0x00 , 0x58 , 0xFF , 0xD1 , 0x17 };
  EepromPram_WritePram( E2_CAN_SET_ONLY_GATHER_0_ID , CANParam0,  sizeof(CANParam0) );   
#endif
  
  
  
#if 0 //����ʱֱ���޸Ĳ���
  
//u8 CANParam0[] ={ 0x00 , 0x00 , 0x00 , 0x60 , 0x58 , 0xFF , 0xD1 , 0x17  };
  
  u8 CANParam1[] ={ 0x00 , 0x00 , 0x00 , 0x64 , 0xD8 , 0xFF , 0xD0 , 0x17 };

// EepromPram_WritePram(  E2_CAN_SET_ONLY_GATHER_0_ID , CANParam0,  sizeof(CANParam0) );  
  
  EepromPram_WritePram( E2_CAN_SET_ONLY_GATHER_1_ID ,  CANParam1,  sizeof(CANParam1) );  
   
#endif

  
  
    //����������
  if( 
      ( E2_CAN_SET_ONLY_GATHER_0_ID != CANParmamerID )&&
      ( E2_CAN_SET_ONLY_GATHER_1_ID != CANParmamerID )
     ) return;
  
  
    iItem = CANParmamerID - E2_CAN_SET_ONLY_GATHER_0_ID ;

  if(  E2_CAN_SET_ONLY_GATHER_LEN == EepromPram_ReadPram( CANParmamerID, BufferTemp  )    )
 // if( E2_CAN_SET_ONLY_GATHER_LEN ==  EepromPram_ReadPram(E2_CAN_SET_ONLY_GATHER_ID, (u8 *)&gCANParame[iCAN1].CAN_Sample_Single) )
  {
    
    
    
      //�ҳ�������ĸ�ͨ����CAN 
      //bit31: ��ʾCANͨ����, 0: CAN1 1: CAN2
#if 0
      if( BufferTemp[4]&0x80 ){
        
          iCANChannel = 1 ;
          
      } else {

          iCANChannel = 0 ;
      }
#endif
      
      
     //Bit28~Bit0 ��ʾCAN����ID
      
    gCANParameSingle.CAN_Sample_Single[ iItem ].CAN_ID_BYTE_0 = BufferTemp[4]&0x1F;
    gCANParameSingle.CAN_Sample_Single[ iItem ].CAN_ID_BYTE_1 = BufferTemp[5];
    gCANParameSingle.CAN_Sample_Single[ iItem ].CAN_ID_BYTE_2 = BufferTemp[6];
    gCANParameSingle.CAN_Sample_Single[ iItem ].CAN_ID_BYTE_3 = BufferTemp[7]; 
    
    
   //bit 29: ��ʾ���ݲɼ���ʽ: 0:ԭʼ���� 1: �ɼ�����ļ���ֵ 
    
    if(  BufferTemp[4]&0x20 ){
      
    gCANParameSingle.CAN_Sample_Single[ iItem ].CAN_Data_Original   = 1; 
    
  } else {
    
    gCANParameSingle.CAN_Sample_Single[ iItem ].CAN_Data_Original   = 0 ;
    
  }
  
  
  //bit 30: ��ʾ֡����: 0:��׼֡ 1:��չ֡
  
  if( BufferTemp[4]&0x40 ){
    
      gCANParameSingle.CAN_Sample_Single[ iItem ].CAN_Frame_Type      = 1 ;
      
  } else {
    
      gCANParameSingle.CAN_Sample_Single[ iItem ].CAN_Frame_Type     = 0 ;
  }

  
  //Bit63~Bit32 ��ʾ��ID�����ɼ�ʱ����(ms), 0 ��ʾ���ɼ� 
  
    Temp32  =  BufferTemp[3];
    gCANParameSingle.CAN_Sample_Single[ iItem ].CAN_Sample_Interval =  Temp32 ;
    
    Temp32  =  BufferTemp[2];
    Temp32 = Temp32<<8 ;
    gCANParameSingle.CAN_Sample_Single[ iItem ].CAN_Sample_Interval +=  Temp32 ;
    
    Temp32  =  BufferTemp[1];
    Temp32 = Temp32<<16 ;
    gCANParameSingle.CAN_Sample_Single[ iItem ].CAN_Sample_Interval +=  Temp32 ;

    Temp32  =  BufferTemp[0];
    Temp32 = Temp32<<24 ;
    gCANParameSingle.CAN_Sample_Single[ iItem ].CAN_Sample_Interval +=  Temp32 ;

    
    //���ݲɼ�ʱ���� ������Ӧͨ���� �����ж�
  //          CAN_Sample_Sw( 0 ,  1) ;  
            
#if 0
    if( gCANParame[iCANChannel].CAN_Sample_Single[ gCANParame[iCANChannel].CAN_Sample_Single_Num ].CAN_Sample_Interval ){
      
        CAN_Sample_Sw( iCANChannel ,  1) ;  
        
    } else {
      
        CAN_Sample_Sw( iCANChannel ,  0);
    }
#endif
    //===========================================================================
    gFilterIDArray[0].iMaskID = 1 ;
    
    TempID = gCANParameSingle.CAN_Sample_Single[ iItem ].CAN_ID_BYTE_0;
    TempID <<= 24;
    gFilterIDArray[0].MaskID = TempID;
    
    TempID = gCANParameSingle.CAN_Sample_Single[ iItem ].CAN_ID_BYTE_1;
    TempID <<= 16;
    gFilterIDArray[0].MaskID += TempID;
    
    TempID = gCANParameSingle.CAN_Sample_Single[ iItem ].CAN_ID_BYTE_2;
    TempID <<= 8;
    gFilterIDArray[0].MaskID += TempID;
    
    TempID = gCANParameSingle.CAN_Sample_Single[ iItem ].CAN_ID_BYTE_3;
    TempID <<= 0;
    gFilterIDArray[0].MaskID += TempID;

        
 //=======================================================================
#if 0   
    gCANParame[iCANChannel].CAN_Filter_ID_Index++ ;
    
    if( iCANChannel == 0){
      
  //    CAN1_ID_Filter_Set(  gCANParame[iCANChannel].CAN_Filter_ID_Index , gFilterIDArray[gCANParame[iCANChannel].CAN_Filter_ID_Index].MaskID , 0xFFFFFFFF );
      
    } else {
      
 //     CAN2_ID_Filter_Set(  gCANParame[iCANChannel].CAN_Filter_ID_Index , gFilterIDArray[gCANParame[iCANChannel].CAN_Filter_ID_Index].MaskID , 0xFFFFFFFF );
    }
      
#endif

    
 //=======================================================================   

  } else {
    
#if 0
        if( E2_CAN_SET_ONLY_GATHER_0_ID == CANParmamerID ){
          
              CAN1_ID_Filter_Set(  gCANParame[iCANChannel].CAN_Filter_ID_Index , 0x00000000, 0x00000000 );
              
        } else if ( E2_CAN_SET_ONLY_GATHER_1_ID == CANParmamerID ){
          
              CAN2_ID_Filter_Set(  gCANParame[iCANChannel].CAN_Filter_ID_Index , 0x00000000, 0x00000000 );
        }
#endif
        
    gCANParameSingle.CAN_Sample_Single[ iItem ].CAN_ID_BYTE_0 = 0;
    gCANParameSingle.CAN_Sample_Single[ iItem ].CAN_ID_BYTE_1 = 0;
    gCANParameSingle.CAN_Sample_Single[ iItem ].CAN_ID_BYTE_2 = 0;
    gCANParameSingle.CAN_Sample_Single[ iItem ].CAN_ID_BYTE_3 = 0;     // : 4;     //28�ֽ�CAN����ID
    
    gCANParameSingle.CAN_Sample_Single[ iItem ].CAN_Data_Original = 0; // :1; //bit 29: ��ʾ���ݲɼ���ʽ: 0:ԭʼ���� 1: �ɼ�����ļ���ֵ
    gCANParameSingle.CAN_Sample_Single[ iItem ].CAN_Frame_Type = 1; //: 1; //bit 30: ��ʾ֡����: 0:��׼֡ 1:��չ֡
    gCANParameSingle.CAN_Sample_Single[ iItem ].CAN_Channel =0 ; //: 1; //bit31: ��ʾCANͨ����, 0: CAN1 1: CAN2
    gCANParameSingle.CAN_Sample_Single[ iItem ].CAN_Sample_Interval = 15L*1000L ;

    }

  //======================��ӡ�����־ ===========================================
  
#if DEBUG_PRINT_CAN
  
    LOG_PR_N( "\r\n" );
  
    Print_RTC();
  
    LOG_PR( " Num = %d \r\n", gCANParameSingle.CAN_Sample_Single[ iItem ].CAN_Channel );
    
    LOG_PR_N( "====================================== \r\n" );

  //bit31: ��ʾCANͨ����, 0: CAN1 1: CAN2
    if( gCANParameSingle.CAN_Sample_Single[ iItem ].CAN_Channel ){
      
      LOG_PR_N( ">> CAN Channel: CAN2 \r\n" );
      
    } else {

      LOG_PR_N( ">> CAN Channel: CAN1 \r\n" );
    }
    
      
  //bit 30: ��ʾ֡����: 0:��׼֡ 1:��չ֡
      if( gCANParameSingle.CAN_Sample_Single[ iItem ].CAN_Frame_Type ){
        
        LOG_PR_N( ">> FrameType: Extend \r\n" );
        
      } else {
        
         LOG_PR_N( ">> FrameType: Standard \r\n" );
         
      }
      
    //bit 29: ��ʾ���ݲɼ���ʽ: 0:ԭʼ���� 1: �ɼ�����ļ���ֵ 
      
      if( gCANParameSingle.CAN_Sample_Single[ iItem ].CAN_Data_Original ){
        
       LOG_PR_N( ">> Data Sample Method: CalcValue\r\n" );
              
      } else {
        
        LOG_PR_N( ">> Data Sample Method: OriginalValue\r\n" );
        
      }
     

    LOG_PR( ">> Sample Internval = %d (ms)\r\n" ,  gCANParameSingle.CAN_Sample_Single[ iItem ].CAN_Sample_Interval  );

      
     LOG_PR( ">> ID(Hex) = %02x %02x %02x %02x\r\n" ,
            gCANParameSingle.CAN_Sample_Single[ iItem ].CAN_ID_BYTE_0 ,
            gCANParameSingle.CAN_Sample_Single[ iItem ].CAN_ID_BYTE_1 ,
            gCANParameSingle.CAN_Sample_Single[ iItem ].CAN_ID_BYTE_2 ,
            gCANParameSingle.CAN_Sample_Single[ iItem ].CAN_ID_BYTE_3
      );
     
     
LOG_PR_N( "\r\n" );
    
    
    

#endif   
   
  
   
    gCANParameSingle.CAN_Sample_Single_Num = (gCANParameSingle.CAN_Sample_Single_Num + 1 )%MAX_NUM_FILTER_ID ; 
   
  
  
}



/********************************************************************
* ���� : Can_TimeTask
* ���� : ��ʼ��ϵͳ����֮ CAN
********************************************************************/
void CAN_TimeTask_Init(void)
{
  
#if DEBUG_PRINT_CAN
  
     Print_RTC();
     LOG_PR_N( "CAN_TimeTask_Init() \r\n" ); 
     
#endif 

     
  CAN_PowerCtrl_Init_GPIO( );
  CAN_PWR_ON();	//��CAN��Դ 
  
  CAN1_Init();
  
  CAN2_Init();
  
//����ID0x0110,CAN ����ID �����ɼ�����BYTE[8] 
#if 0
  UpdataOnePram_Sample_Single(  E2_CAN_SET_ONLY_GATHER_0_ID  );
#endif
  
//����ID0x0111,CAN ����ID �����ɼ�����BYTE[8] 
#if 0
  UpdataOnePram_Sample_Single(  E2_CAN_SET_ONLY_GATHER_1_ID  );
#endif
  
  
}

/********************************************************************
* ���� : Can_TimeTask
* ���� : ϵͳ����֮ CAN
********************************************************************/
FunctionalState  Can_TimeTask(void)
{
  
//  static u16 UploadTimeCnt[2] = { 0, 0 };
 // static u16 SampleTimeCnt[2] = { 0, 0 };
  u8 PacktetBuf[1024*2];
  static u16 PacketLenght;
  u8 iCANCh = 0;;
  u8 channel = CHANNEL_DATA_1|CHANNEL_DATA_2;
    
  //  gCANParame[iCANCh].CAN_UploadPeriodxS  = 5;
  
 // for( iCANCh = 0; iCANCh < CAN_CH_MAX ; iCANCh++ ){
   iCANCh = 0 ;
   
   if( gCANParame[iCANCh].SampleTimeCnt++ > ( gCANParame[iCANCh].CAN_SamplePeriodxMS/50 )  ){
     
     gCANParame[iCANCh].FlagSampleTimeArrived = 1;
 //    FlagUplodTimeArrived
     
   }
   
   
   
   
   
  // gCANParame[iCANCh].CAN_SamplePeriodxMS/SYSTICK_50ms
     
     
     
    
  // gCANParame[0].CAN_UploadPeriodxS = 3;
    if( gCANParame[iCANCh].CAN_UploadPeriodxS )
    {
       gCANParame[iCANCh].UploadTimeCnt++;
      
    if(  ( gCANParame[iCANCh].UploadTimeCnt >= ( gCANParame[iCANCh].CAN_UploadPeriodxS*SYSTICK_1SECOND / (5*SYSTICK_0p1SECOND) ) ) ||
         ( gCanIsrData[iCANCh].receNum > (CAN_MAX_RECE_NUM-20) )
 
        )
      //  if(  UploadTimeCnt[iCANCh] >= ( 4 *SYSTICK_1SECOND / (5*SYSTICK_0p1SECOND) )   )
       // if(  UploadTimeCnt[iCANCh] >= ( 20*SYSTICK_1SECOND / (5*SYSTICK_0p1SECOND) )   )

      {
        
		//������0
		gCANParame[iCANCh].UploadTimeCnt = 0;
                
#if DEBUG_PRINT_CAN
     Print_RTC();
     LOG_PR( "CAN%d Upload Time Is Arrived" , iCANCh+1 ); 
     LOG_PR_N( "\r\n");     
#endif                  
                
                
                if(  gCanIsrData[iCANCh].receNum > 0)
                {
                  
                  //  PacketLenght = CAN_Create_Packet_Upload(  (u8 *) PacktetBuf  );
                    
                    // PacketLenght = CanBus_GetData(  iCANCh, (u8 *) PacktetBuf  );
                     
                    PacketLenght = CAN_Packet_Single_ID(  iCANCh, (u8 *) PacktetBuf  );
                    
                    
 #if DEBUG_PRINT_CAN
                if( PacketLenght) {
                        Print_RTC();
                        
                       
                        if(  communicatio_GetMudulState( COMMUNICATE_STATE ) ){
                          LOG_PR_N( "T->S OK ! ");  
                        } 
                        
                        LOG_PR_N( "CAN_Packet_Single_ID():  "); 
                        LOG_DB( "", (u8 *) PacktetBuf , PacketLenght );
                     //   LOG_DB( "", (u8 *) PacktetBuf , 2 );
                        LOG_PR_N( "\r\n");    
                 }
#endif                       
                    if( PacketLenght > 0)    
                    {
                        RadioProtocol_CAN_UpTrans( channel, (u8 *) PacktetBuf , PacketLenght );
                    }
                    
                }

        }

    }else {
      
        gCANParame[iCANCh].UploadTimeCnt = 0;
        
    }
   
    return ENABLE;
   
   
}
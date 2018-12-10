/*************************************************************************
* Copyright (c) 2013,�������������¼����������޹�˾
* All rights reserved.
* �ļ����� : Module_Sensor_MixerRotate.c
* ��ǰ�汾 : 1.1
* ������   : Shigle 
* �޸�ʱ�� : 2013��10��9��


* 2013��10��9��Ϊ���߰汾1.0, �������������������޸ģ���Ҫ������׷��˵��
* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
* 1������ת��������Ӳ�����ӣ�
    V+     ---------- 32PIN����ͷ�ӿڵĵ�Դ��- P17- ����
    V-     ---------  32PIN ����ͷ�ӿڵĵ�Դ�� - P18-  ����
    ת���� ---------- 32PIN�Ŀյ�  - P22- �̰��� - STM32:PE9
    ������ ---------- 32PIN�ĳ���2 - P9-  �ۺ��� - STM32:PF15
* 2�������߼���, �͵�ƽ������Ϊ��ת��Ϊ�ߵ�ƽ������Ϊ��ת
* 3��ת�ٷŴ�10X����λ: ת/����, ���磬���ص�����תת��ֵΪ52 ,������ǰ��⵽��Ͳת��Ϊ5.2ת/����
* 4��������60��δ���յ�ת�����ϵ��ⲿ�жϣ����ж���Ͳֹͣ���������Ӧ��λ��
* 5������ת�������Ĺ���: ACC OFF ֹͣ���磬 ACC ON ��������
* 2013��10��28����Ϊ�汾1.1
* 1���޸�Ϊ������ת��״̬�����仯ʱ��������һ��λ����Ϣ�ϱ���
* 2013��11��09����Ϊ�汾1.2
* 1���޸�Ϊ�ϱ�ƽ̨��״ֵ̬: 00:��ʾ ��Ч�� 11����ʾֹͣ
*************************************************************************/



//***************�����ļ�*****************
#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"


#include "include.h"


#ifdef ZHENGFANZHUAN

//*************** ȫ�ֱ��� ������ *****************

MIXER_CAR_OBJECT        gMixerCarObject;


/*******************************************************************************
**  ��������  :	MixerRotate_Exti_Init
**  ��������  : ���賵����ת������ �ⲿ�жϳ�ʼ��
*******************************************************************************/
void MixerRotate_Exti_Init(void)
{
  
  //   ת����----------32PIN�Ŀյ�    p22-�̰�-PE9
	  EXTI_InitTypeDef   EXTI_InitStructure;
  	GPIO_InitTypeDef   GPIO_InitStructure;
  	NVIC_InitTypeDef   NVIC_InitStructure;

  	/* Enable GPIOE clock */
  	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
  	/* Enable SYSCFG clock */
  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
  
  	/* Configure PE9 pin as input floating */
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_9;
  	GPIO_Init( GPIOE , &GPIO_InitStructure );

  	/* Connect EXTI Line2 to PE9 pin */
  	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource9);

  	/* Configure EXTI Line9 */
  	EXTI_InitStructure.EXTI_Line = EXTI_Line9;
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; //EXTI_Trigger_Rising_Falling;
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  	EXTI_Init( &EXTI_InitStructure );

  	/* Enable and set EXTI14 Interrupt to the lowest priority */
  	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	  NVIC_Init(&NVIC_InitStructure);
}



/*******************************************************************************
**  ��������  :	MixerRotate_IO_Polarity_Init
**  ��������  : ���賵����ת������ ���� ����ⲿ�ж�  ʹ��
*******************************************************************************/
void MixerRotate_Exti_Enable(void)
{
  
	  NVIC_InitTypeDef   NVIC_InitStructure;
    
	/* Enable and set EXTI9 Interrupt to the lowest priority */
  	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn ; //EXTI15_10_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	  NVIC_Init(&NVIC_InitStructure);
}

/*******************************************************************************
**  ��������  :	MixerRotate_IO_Polarity_Init
**  ��������  : ���賵����ת������ ���� ����ⲿ�ж�  ��ֹ
*******************************************************************************/
void MixerRotate_Exti_Disable(void)
{
  
	  NVIC_InitTypeDef   NVIC_InitStructure;
	/* Enable and set EXTI14 Interrupt to the lowest priority */
  	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn ; // EXTI15_10_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
	  NVIC_Init(&NVIC_InitStructure);
}


/*******************************************************************************
**  ��������  :	MixerRotate_Exti_Isr
**  ��������  : ���賵����ת������ ���� ����ⲿ�жϷ������
*******************************************************************************/
void MixerRotate_Exti_Isr(void)
{
  u32 nowtimer;
//�����ź���
    if(EXTI_GetITStatus(EXTI_Line9) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line9); //����ⲿ�жϱ�־
  
        if( gMixerCarObject.bMixterStateChange == 0)
        {
            gMixerCarObject.bMixterStateChange = 1;
        }
        
        if( gMixerCarObject.PulseStart ==0)
        {
            gMixerCarObject.PulseStart = 1;  
            gMixerCarObject.PulseCnt = 0;
            gMixerCarObject.RevSampleTime = Timer_Val();//�����𶯲�����ʱ��
        }
        else
        {
            gMixerCarObject.PulseCnt++;  //��������1
            
#if 1
            nowtimer = Timer_Val();//�����𶯲�����ʱ��
            gMixerCarObject.OneLoopTime = nowtimer - gMixerCarObject.RevSampleTime ;
            gMixerCarObject.RevSampleTime = nowtimer ; //�����𶯲�����ʱ��
            
            gMixerCarObject.Rev = 12000/( gMixerCarObject.OneLoopTime*gMixerCarObject.NumMagneticCylinder); 
            /*
            if(   (1200 - gMixerCarObject.Rev*gMixerCarObject.OneLoopTime*gMixerCarObject.NumMagneticCylinder)*2 > gMixerCarObject.OneLoopTime*gMixerCarObject.NumMagneticCylinder    )
            {
               gMixerCarObject.Rev++;
            }
          */
#else            
       
         //   if(gMixerCarObject.PulseCnt == 1)
             if(gMixerCarObject.PulseCnt%2 == 1)
            {
              
                gMixerCarObject.RevSampleTime = Timer_Val();//�����𶯲�����ʱ��
                
          //  }else if(gMixerCarObject.PulseCnt >1) {     //����Ͱһ�ܰ�װ��4���Ÿ֣�תһ�ܲ���4������
             }else if(gMixerCarObject.PulseCnt %2 == 0 ) {     //����Ͱһ�ܰ�װ��4���Ÿ֣�תһ�ܲ���4������
               
       //         gMixerCarObject.PulseCnt = 0;
               
                nowtimer = Timer_Val();//�����𶯲�����ʱ��
                gMixerCarObject.OneLoopTime = nowtimer - gMixerCarObject.RevSampleTime ;
                gMixerCarObject.Rev = 1200/( gMixerCarObject.OneLoopTime*gMixerCarObject.NumMagneticCylinder);      
                gMixerCarObject.RevSampleTime = nowtimer ; //�����𶯲�����ʱ��
            }
            
#endif           
            
            
         }      

    }
    
 
}


/*******************************************************************************
**  ��������  :	MixerRotate_IO_Polarity_Init
**  ��������  : ���賵����ת������ ���ԣ����򣩼��IO�ڳ�ʼ��
*******************************************************************************/
void MixerRotate_IO_Speed_Init(void)
{
  //ת����----------32PIN�Ŀյ�    p22-�̰�-PE9
	  EXTI_InitTypeDef   EXTI_InitStructure;
  	GPIO_InitTypeDef   GPIO_InitStructure;
  	NVIC_InitTypeDef   NVIC_InitStructure;

  	/* Enable GPIOE clock */ 
  	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
  	/* Enable SYSCFG clock */
  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
  
  	/* Configure PE9 pin as input floating */
    
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  	GPIO_Init(GPIOB, &GPIO_InitStructure);

  	/* Connect EXTI Line2 to PE9 pin */
  	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource9);

  	/* Configure EXTI Line9 */
  	EXTI_InitStructure.EXTI_Line = EXTI_Line9;
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling; //EXTI_Trigger_Rising_Falling;
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  	EXTI_Init(&EXTI_InitStructure);

  	/* Enable and set EXTI9 Interrupt to the lowest priority */
  	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn ; //EXTI15_10_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	  NVIC_Init(&NVIC_InitStructure);
    
    
    
}






/*******************************************************************************
**  ��������  :	MixerRotate_IO_Polarity_Init
**  ��������  : ���賵����ת������ ���ԣ����򣩼��IO�ڳ�ʼ��
*******************************************************************************/
void MixerRotate_IO_Polarity_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStructure;
  
  /* ʹ��GPIOʱ�� */
  
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
  
  //������----------32PIN�ĳ���2    PF15
  
  /* ���� ����ת������ ���Լ��� Ϊ����ģʽ*/
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15 ;
  GPIO_Init( GPIOF, &GPIO_InitStructure);
  
}
/*******************************************************************************
**  ��������  :	MixerRotate_RAM_Init
**  ��������  : ���賵����ת RAM ȫ�ֱ�����ʼ��
*******************************************************************************/
void MixerRotate_RAM_Init(void)
{

    gMixerCarObject.RevErrorFlag = 0 ; //GprsNetwork_GetMixerSensorFail();//��ʼ�����賵����ת�������޹���
    gMixerCarObject.Reserver = 0;
    
    gMixerCarObject.MixerFailTotal =0 ; //��ʼ��������賵����ת���������ϼ���
    gMixerCarObject.PulseCnt = 0 ;      //�������
    gMixerCarObject.PulseStart = 0 ;    //������ʼ����
    gMixerCarObject.updata = 0;          //�Ƿ����

    gMixerCarObject.bMixterStateChange= 0 ;
    gMixerCarObject.Mixer_Rotate_Polarity_last = 0;
    gMixerCarObject.Mixer_Rotate_Polarity_current= 0;

    
    gMixerCarObject.CarRunStopCounter = 0 ; //Shigle 2012-11-12  
    
    gMixerCarObject.RevSampleTime = Timer_Val();//�����𶯲�����ʱ��

    
    gMixerCarObject.Rev = 0;            //��ʼ�����賵����ת������ת��Ϊ0
    gMixerCarObject.RevState =  ROTATE_STATE_UNKNOWNED ; //��ʼ��Ϊδ֪״̬
    gMixerCarObject.NumMagneticCylinder = NUM_MAGNETIC_CYLINDER; //ÿ����Ͳ�Ÿ����� 

}




/*******************************************************************************
*  ��������  : MixerRotate_Task_Init
*  ��������  : ���賵��Ͳ�����ʼ��
*******************************************************************************/
void MixerRotate_Task_Init(void)
{
  
  
  MixerRotate_RAM_Init();
  
  MixerRotate_IO_Polarity_Init();

 // MixerRotate_IO_Speed_Init();

  MixerRotate_Exti_Init();
  
  MixerRotate_Exti_Enable();
 // MixerRotate_Exti_Disable();

}



/*******************************************************************************
*  ��������  : MixerRotate_Update_RotateState
*  ��������  : ��ý��賵��Ͳ��״̬: 00��ֹͣ 01:��ת 10:��ת 11��δ֪
*******************************************************************************/
void MixerRotate_Update_RotateState(void)
{
      
    static u8 cnt = 0 ;
    if( cnt++ > 1*SYSTICK_1SECOND ){
      
      cnt = 0 ;
      
    }else{
      
      return ;
      
    }

    switch( gMixerCarObject.RevState )
    {
      
        case 0://00:ֹͣ
#if 1
          Io_WriteSelfDefine2Bit( DEFINE_BIT_13 , SET );
          Io_WriteSelfDefine2Bit( DEFINE_BIT_14 , SET );
          
#else
          Io_WriteSelfDefine2Bit( DEFINE_BIT_13 , RESET );
          Io_WriteSelfDefine2Bit( DEFINE_BIT_14 , RESET );
#endif
          
          break;
          
        case 1: //01��ת
          Io_WriteSelfDefine2Bit( DEFINE_BIT_13 , SET );
          Io_WriteSelfDefine2Bit( DEFINE_BIT_14 , RESET ); 
          break;
          
        case 2: //10:��ת
          
          Io_WriteSelfDefine2Bit( DEFINE_BIT_13 , RESET );
          Io_WriteSelfDefine2Bit( DEFINE_BIT_14 , SET );
          break;
          
        case 3: //11:δ֪
          
#if 1
          Io_WriteSelfDefine2Bit( DEFINE_BIT_13 , RESET );
          Io_WriteSelfDefine2Bit( DEFINE_BIT_14 , RESET );          
#else

          Io_WriteSelfDefine2Bit( DEFINE_BIT_13 , SET );
          Io_WriteSelfDefine2Bit( DEFINE_BIT_14 , SET );          
#endif         
     
          break;
          
        default:
          
#if 1
          Io_WriteSelfDefine2Bit( DEFINE_BIT_13 , RESET );
          Io_WriteSelfDefine2Bit( DEFINE_BIT_14 , RESET );          
#else

          Io_WriteSelfDefine2Bit( DEFINE_BIT_13 , SET );
          Io_WriteSelfDefine2Bit( DEFINE_BIT_14 , SET );          
#endif  
          
          break;
          
    }
    

}
/*******************************************************************************
*  ��������  : Task_MixerRotate()
*  ��������  : ���賵��Ͳ��ʱ����
*******************************************************************************/
FunctionalState Task_MixerRotate(void)
{
  	u8	ACC;
    static u8 InitCnt = 0 ;
    static u8 RevState_Current = ROTATE_STATE_UNKNOWNED ; 
    
    


   //------------------------ACC OFF ------------------------------------  
    
    ACC = Io_ReadStatusBit(STATUS_BIT_ACC);
    if( 0 == ACC )
    {
      
      //ACC OFF �ر�ʱ��ֹͣ������ת����������
      //��ͨ�ͺĴ�����������ͷ���ýӿڣ����ͺĴ����������־,1Ϊ����,0Ϊδ����
      if(0 == JointechOiLCost_Online_Get())
      {
        CAMERA_POWER_OFF();//������ͷ��Դ
      }
       //CAMERA_POWER_OFF(); //���賵����ת������������ƿ� 0:ֹͣ 1:,dxl,2014.3.19
       
       gMixerCarObject.RevState = ROTATE_STATE_UNKNOWNED ;  //���賵ֹͣ״̬
       gMixerCarObject.Rev = 0;  //���賵ת�١�����

       gMixerCarObject.bMixterStateChange =0; //ת���ޱ仯
       gMixerCarObject.PulseStart = 0; //�����������λ���¿�ʼ 
       gMixerCarObject.PulseCnt = 0; //������� ����
       gMixerCarObject.Mixer_Rotate_Polarity_last = 0;
       gMixerCarObject.Mixer_Rotate_Polarity_current = 0 ;//������һ������״̬λ
       gMixerCarObject.MixerFailTotal = 0;
       gMixerCarObject.RevSampleTime = Timer_Val();//�����𶯲�����ʱ��

       MixerRotate_Update_RotateState();
       
       return ENABLE;
    }
    
    
    
    //------------------------ACC ON ------------------------------------    
    
    CAMERA_POWER_ON();
    
    
    

//======================����ת״̬�任===================
    if(  GPIO_ReadInputDataBit( GPIOF ,  GPIO_Pin_15 ) )
    {
        gMixerCarObject.Mixer_Rotate_Polarity_current = 1; //�͵�ƽ
        
    } else {
      
        gMixerCarObject.Mixer_Rotate_Polarity_current = 0; //�ߵ�ƽ������
    }
    
    
    
    if( InitCnt== 0   ){
      InitCnt = 1;
      
      if( gMixerCarObject.Mixer_Rotate_Polarity_current ) 
      {
        
          gMixerCarObject.RevState = 1;//�����ǰ����Ϊ�ߵ�ƽ������Ϊ��ת
          
      } else {
        
          gMixerCarObject.RevState = 2;//�����ǰ����Ϊ�͵�ƽ������Ϊ��ת
          
      }    

    }
    
    //------------------------------------------------------          
    if(  gMixerCarObject.bMixterStateChange ==0)
    {
        //���½��� �ӽ��賵��Ͳ״̬ת���� ����ת->ֹͣ �� ��ת->ֹͣ �ж�
        if( (gMixerCarObject.RevState== 1  )||(gMixerCarObject.RevState== 2 ) )
        {
          
            if(  gMixerCarObject.CarRunStopCounter++ > NUM_SENCOND_NOEXTI * SYSTICK_1SECOND   )
            {
              //���������10*15.625msû����⵽ ��Ͳ������Ϣ������Ϊ ��Ͳֹͣ
                gMixerCarObject.RevState = 0  ;
                gMixerCarObject.Rev = 0;
            }
            
        }

    } else if(  gMixerCarObject.bMixterStateChange < 4  ) {//��Ͳ���� �����źţ���������·�֧ 
    
          if( gMixerCarObject.RevState==0 )
         {
              gMixerCarObject.bMixterStateChange =0;
              gMixerCarObject.Mixer_Rotate_Polarity_last = gMixerCarObject.Mixer_Rotate_Polarity_current;
              
              if( gMixerCarObject.Mixer_Rotate_Polarity_current ) 
              {
                  gMixerCarObject.RevState = 1;//�����ǰ����Ϊ�ߵ�ƽ������Ϊ��ת
              }
              else
              {
                  gMixerCarObject.RevState = 2;//�����ǰ����Ϊ�͵�ƽ������Ϊ��ת
              }
              
         } else {
      
            if( gMixerCarObject.Mixer_Rotate_Polarity_current != gMixerCarObject.Mixer_Rotate_Polarity_last){
              
              //��Ͳ���� �����ź��б仯����������û�������ۼӵ�����
               gMixerCarObject.bMixterStateChange++;
            } else {
              
              //��Ͳ���� �����źŲ��б仯������������ 
                gMixerCarObject.bMixterStateChange =0; 
            }
            
         }
         
         //����Ͳ����������������������
          gMixerCarObject.MixerFailTotal = 0;
          gMixerCarObject.RevErrorFlag = 0;

          gMixerCarObject.CarRunStopCounter = 0; 
          
          
    }else{
      
           //��Ͳ���� �����ź��б仯�����������ۼӵ�����,����������, ����µ�ǰ����
           gMixerCarObject.bMixterStateChange = 0;
           
           //������������ת���Է����仯ʱ.....
           gMixerCarObject.PulseStart = 0; //�����������λ���¿�ʼ 
           gMixerCarObject.PulseCnt = 0; //������� ����
           gMixerCarObject.Rev = 0; //ת�����㡡
           
           gMixerCarObject.Mixer_Rotate_Polarity_last = gMixerCarObject.Mixer_Rotate_Polarity_current;//������һ������״̬λ
          
           gMixerCarObject.CarRunStopCounter = 0 ; //ˢ��ֹͣ��ʱ
           
           if( gMixerCarObject.Mixer_Rotate_Polarity_current ) 
           {
              gMixerCarObject.RevState = 1;//�����ǰ����Ϊ�ߵ�ƽ������Ϊ��ת
           }
           else
           {
              gMixerCarObject.RevState = 2;//�����ǰ����Ϊ�͵�ƽ������Ϊ��ת
           }
           
           //����Ͳ����������������������
            gMixerCarObject.MixerFailTotal = 0;
           
    }
    
    
    MixerRotate_Update_RotateState();
    
    
    if( RevState_Current != gMixerCarObject.RevState )
    {
        RevState_Current = gMixerCarObject.RevState ;
        //SetEvTask( EV_REPORT );
        Report_UploadPositionInfo(CHANNEL_DATA_1);
        Report_UploadPositionInfo(CHANNEL_DATA_2);
    }
    
    return ENABLE;
    
}



/*******************************************************************************
*  ��������  : MixerRotate_TimeTask()
*  ��������  : ���賵��Ͳ��ʱ����
*******************************************************************************/
FunctionalState MixerRotate_TimeTask(void)
{
  
  return  Task_MixerRotate();
}



/*******************************************************************************
*  ��������  : MixerRotate_Get_RotateState
*  ��������  : ��ý��賵��Ͳ��״̬: 00��ֹͣ 01:��ת 10:��ת 11��δ֪
*******************************************************************************/
u8 MixerRotate_Get_RotateState(void)
{
  
#if 1
  
    return  gMixerCarObject.RevState;
    
#else

    
    u8  retval =  ROTATE_STATE_UNKNOWNED ;
    switch( gMixerCarObject.RevState )
    {
      
        case 0:
          retval =  ROTATE_STATE_STOP      ;//00:ֹͣ
          break;
        case 1:
          retval =  ROTATE_STATE_POSITIVE  ;//10:��ת
          break;
        case 2:
          retval =  ROTATE_STATE_REVERSE   ;//01��ת
          break;
        case 3:
          retval =  ROTATE_STATE_UNKNOWNED ;//11:δ֪
          break;
        default:
           retval =  ROTATE_STATE_STOP      ;//00:ֹͣ
          break;
    }
    
    return  retval;
    
#endif
    
}



/*******************************************************************************
*  ��������  : MixerRotate_Get_RotateSpeed
*  ��������  : ��ý��賵��Ͳ��ת��
*******************************************************************************/
u8 MixerRotate_Get_RotateSpeed(void)
{

    if( (gMixerCarObject.RevState &0x03) == 0)
    {
         gMixerCarObject.Rev = 0;
    }
 
    if( gMixerCarObject.bMixterStateChange >1 )
    {
        gMixerCarObject.Rev = 0;
    }
    
    return  gMixerCarObject.Rev;

}



#endif

/******************************************************************************
**                            End Of File
******************************************************************************/


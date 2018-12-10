/********************************************************************
//��Ȩ˵��	:Shenzhen E-EYE Co. Ltd., Copyright 2009- All Rights Reserved.
//�ļ�����	:Sleep.c		
//����		:
//�汾��	:V0.1
//������	:dxl
//����ʱ��	:2012.6 
//�޸���	:
//�޸�ʱ��	:
//�޸ļ�Ҫ˵��	:
//��ע		:
***********************************************************************/

//********************************ͷ�ļ�************************************
#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "include.h"
//********************************�Զ�����������****************************

//********************************�궨��************************************

//********************************ȫ�ֱ���**********************************

//********************************�ⲿ����**********************************

//********************************���ر���**********************************
static u16	AccOnCount = 0;
static u16	AccOffCount = 0;
#ifdef HUOYUN_DEBUG_OPEN
u8       SleepFlag = 0;//���߱�־,1Ϊ��������,0Ϊ������,����ƽ̨Ҫ��ACC OFF��30���ӽ��������ϱ�
#endif
//********************************��������**********************************

/*********************************************************************
//��������	:Sleep_TimeTask(void)
//����		:���߶�ʱ����
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
*********************************************************************/
FunctionalState  Sleep_TimeTask(void)
{
        
	//u8	Buffer[REPORT_BUFFER_SIZE];//λ����Ϣ�洢����
	//u8	length;
        u8      Acc = 0;
        static u8      LastAcc = 0;
        
        Acc = Io_ReadStatusBit(STATUS_BIT_ACC);
        
        if(Acc != LastAcc)
        {
            //�������߻��˳����ߵĵ�һ��,�ϱ�һ����
            //�ϱ�λ����Ϣ                
	   // length = Report_GetPositionInformation(Buffer);
	   //RadioProtocol_PostionInformationReport(Buffer, length);//λ����Ϣ�㱨
            
        }
        
	if(1 == Acc)
	{
               // SleepCount = 0;
                //LoopCount = 0;
		//�����˷�
		//Io_WriteMicConrtolBit(MIC_BIT_TEST, SET);
		AccOffCount = 0;
		AccOnCount++;
		if(20 == AccOnCount)
		{
			CHARGE_ON(); 
			CAN_PWR_ON();
			
		}
		Io_WriteIoStatusBit(IO_STATUS_BIT_SLEEP, RESET);
               
#ifdef HUOYUN_DEBUG_OPEN
                 SleepFlag = 0;
#endif
	}
	else
	{
		AccOnCount = 0;
		AccOffCount++;
		if(20 == AccOffCount)
		{
			CHARGE_OFF();
			CAN_PWR_OFF();
                        //�ж�����2�����Ƿ�����ΪCANģ��
	                if(1 == ReadPeripheral2TypeBit(1))//bit1:1����CAN���ӣ�0:û�н�
	                {
		              NaviLcdPowerOff();
	                }
		}
                else if(1800 == AccOffCount)
                {
		        //�����˷�
		        //Io_WriteMicConrtolBit(MIC_BIT_TEST, RESET);
                        Io_WriteIoStatusBit(IO_STATUS_BIT_SLEEP, SET);
#ifdef HUOYUN_DEBUG_OPEN
                        SleepFlag = 1;
#endif
                }
                
           
                
	}

        LastAcc = Acc;
        #if	TACHOGRAPHS_19056_TEST
        Tachographs_Test_Speed_Reduce();
		#endif
        return ENABLE;
      
}
/*********************************************************************
//��������	:Sleep_Init(void)
//����		:���߳�ʼ��
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
*********************************************************************/
void Sleep_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
  	
	/* ʵ��GPIOʱ��*/
  	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);

  	/* ����PG10�� 50Mhz�������,������,���ڿ��Ʊ����� */
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;  //LED�ƿ��Ʋ��� �������� GPIO_PuPd_UP
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(GPIOG, &GPIO_InitStructure);
}





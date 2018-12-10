/********************************************************************
//��Ȩ˵��	:Shenzhen E-EYE Co. Ltd., Copyright 2009- All Rights Reserved.
//�ļ�����	:Pulse_App.c		
//����		:�����ٶȼ�⹦��
//�汾��	:
//������	:dxl 
//����ʱ��	:2012.6
//�޸���	:
//�޸�ʱ��	:
//�޸ļ�Ҫ˵��	:
//��ע		:
***********************************************************************/
/*************************�޸ļ�¼*************************/
/*************************�ļ�����***********************/
#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "include.h"
/*************************�궨��***********************/
#define	TIME_COUNT_ARRAY_SIZE	5
/*************************�ⲿ����***********************/
extern u8	SpeedFlag;//�ٶ�����,dxl,2015.5.11,0x00:�ֶ�����,0x01:�ֶ�GPS,0x02:�Զ�����,0x03:�Զ�GPS
extern vu32 	myPulseCnt;             //У׼����ʹ��
extern u8	TestStatus;//����״̬;E1H,�������������;E2H,��������ϵ��������;E3H,����ʵʱʱ��������
/*************************ȫ�ֱ���***********************/
u32	PulseTotalMile = 0;//�ۼ���ʻ��̣��������,��λ��0.01����
u32	FeatureCoef = 0;//��������ϵ��
/*************************���ر���***********************/
static u16	TimeCountMin = 0;//��ʱ������������Сֵ��С�ڸ�ֵ��Ϊ�Ǹ���
static u16	TimeCountArray[TIME_COUNT_ARRAY_SIZE+1] = {0};

#ifdef SPEED_PINGJUN
static u8	TimeCountArrayCount = 0;
#endif

u32 	ExtiCount = 0;//�ⲿ�жϽ����������
u32	PulseMileCount = 0;//�����������̣�����
u8	PulseSpeed = 0;//����˲ʱ�ٶȣ�km/h
u8	LastPulseSpeed = 0;//��һ�������ٶ�
u8	PulseMinuteSpeed = 0;//����ÿ����ƽ���ٶ�
/*********************************************************************
//��������	:Pulse_UpdataPram
//����		:����ģ����±���
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
//		:
*********************************************************************/
void Pulse_UpdataPram(void)
{
	u8	TmpData[4] = {0};
	u8	PramLen = 0;
	u32	temp;
	//������ϵ��
	PramLen = EepromPram_ReadPram(E2_CAR_FEATURE_COEF_ID, TmpData);
	if(3 != PramLen)
	{
			FeatureCoef = 7200;//Ĭ��Ϊ3600��ȡ�����ٶȴ�����ϵ��,dxl,2015.4.17���Ϊ7200
	}
	else
	{
			FeatureCoef = 0;
			FeatureCoef |= (TmpData[0]<<16);//��ȡ��λ
			FeatureCoef |= (TmpData[1] << 8);//���ֽ�
			FeatureCoef |= TmpData[2];//���ֽ�λ
	}
	//��ʱ��������С����ֵ
	temp = (3600*(u32)1000000)/(50*FeatureCoef*255);
	TimeCountMin = temp&0xffff;
	//���ۼ���ʻ���
	PramLen = FRAM_BufferRead(TmpData, FRAM_MILEAGE_LEN, FRAM_MILEAGE_ADDR);
	//���У���ֽ��Ƿ���ȷ
	if(FRAM_MILEAGE_LEN == PramLen)//У�����ȷ
	{
		PulseTotalMile = 0;
		PulseTotalMile |= TmpData[0] << 24;	//���ֽ�
		PulseTotalMile |= TmpData[1] << 16;	//�θ��ֽ�
		PulseTotalMile |= TmpData[2] << 8;		//���ֽ�
		PulseTotalMile |= TmpData[3];		//���ֽ�
	}
	//�������ϵ��ΪĬ��ֵ3600,���Զ���������ϵ��У׼����,dxl,2015.4.17���Ϊ7200
	if((3600 == FeatureCoef)||(7200 == FeatureCoef))
	{
		setAdjustFunction(ENABLE);//��������ϵ��У׼����
	}
	
}

/*********************************************************************
//��������	:Pulse_GetSpeed
//����		:��ȡ����˲ʱ�ٶ�,��λkm/h
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
//		:
*********************************************************************/
u8 Pulse_GetSpeed(void)
{
	#if(TACHOGRAPHS_19056_TEST)
    	return Tachographs_Test_Get_Speed();
	#else
		return PulseSpeed;
	#endif
}
/*********************************************************************
//��������	:Pulse_GetMinuteSpeed
//����		:��ȡ����ÿ����ƽ���ٶ�
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
//		:
*********************************************************************/
u8 Pulse_GetMinuteSpeed(void)
{
	return PulseMinuteSpeed;
}
/*********************************************************************
//��������	:Pulse_GetTotalMile
//����		:��ȡ���������
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:��λ��0.01����
//		:
*********************************************************************/
u32 Pulse_GetTotalMile(void)
{
	return PulseTotalMile;
}
/*********************************************************************
//��������	:Pulse_Time3Isr
//����		:��ʱ��3�ж�
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
//		:
*********************************************************************/
void Pulse_Tim3Isr(void)
{
	u8	i = 0;
	static u32	TestCount = 0;

	if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
  	{
		if(0xE3 == TestStatus)
		{
			TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
			//��ʱ��3������0
			TIM_SetCounter(TIM3, 0);
			
			TestCount++;
			if(0 == TestCount%2)
			{
				GPIO_SetBits(GPIOB, GPIO_Pin_4);
			}
			else
			{
				GPIO_ResetBits(GPIOB, GPIO_Pin_4);
			}
		}
		else if(0xE1 == TestStatus)
		{
			TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
			//��ʱ��3������0
			TIM_SetCounter(TIM3, 0);
			Recorder_MileageTest();//ÿ���ӷ���һ��Ӧ��֡
		}
		else
		{
    			TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
			//��ʱ��3������0
			TIM_SetCounter(TIM3, 0);
			//�رն�ʱ��3
			TIM_Cmd(TIM3, DISABLE);
			ExtiCount = 0;
			PulseSpeed = 0;
			LastPulseSpeed = 0;
			//��ʱ��4������0
			TIM_SetCounter(TIM4, 0);
			//�رն�ʱ��4
			TIM_Cmd(TIM4, DISABLE);
			for(i=0; i<TIME_COUNT_ARRAY_SIZE; i++)
			{
				TimeCountArray[i] = 0;
			}
			//TimeCountArrayCount = 0;
		}
		
	}
}
/*********************************************************************
//��������	:Pulse_Time4Isr
//����		:��ʱ��4�ж�
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:֮ǰ��0.2�����1��˲ʱ�ٶȣ��������ַ���ʵʱ���ͺ���������Ҫ�����
//		:���뵽�ⲿ�ж��м���˲ʱ�ٶȣ��ö�ʱ�����ڲ�����
*********************************************************************/
void Pulse_Tim4Isr(void)
{
	
	if(TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
  	{
    		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
		//��ʱ��4������0
		TIM_SetCounter(TIM4, 0);
		//�رն�ʱ��4
		TIM_Cmd(TIM4, DISABLE);
		
	}
}
/*********************************************************************
//��������	:Pulse_ExtiIsr
//����		:�ⲿ�ж�
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
//		:
*********************************************************************/
void Pulse_ExtiIsr(void)
{
	
	
	static u32	TestCount = 0;
	
	//��ȡ�ٶ����ͼ�ACC״̬
	//if((0 != SpeedFlag)||(0 == Io_ReadStatusBit(STATUS_BIT_ACC)))
	//{
		/* Clear the EXTI line 0 pending bit */
    		//EXTI_ClearITPendingBit(PULSE_EXTI_LINE);
		//return ;
	//}
	
	if(0xE2 == TestStatus)
	{
		TestCount++;
		if(0 == TestCount%2)
		{
			GPIO_SetBits(GPIOB, GPIO_Pin_4);
		}
		else
		{
			GPIO_ResetBits(GPIOB, GPIO_Pin_4);
		}
	}
	else
	{
		Pulse_MileAdd();
	}
	/* Clear the EXTI line 0 pending bit */
    	EXTI_ClearITPendingBit(PULSE_EXTI_LINE);
}
/*********************************************************************
//��������	:Pulse_MileAdd
//����		:����ۼ�
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
//		:
*********************************************************************/
void Pulse_MileAdd(void)
{
	u16	TimeCount = 0;
        
	u32	sum = 0;
	u16	temp;
        float   v = 0;
        u8      Acc = 0;
	
	static  u8	Count3 = 0;
        
    Acc = Io_ReadStatusBit(STATUS_BIT_ACC);//dxl,2013.11.21����ACC������
        
	ExtiCount++;
	PulseMileCount++;
	myPulseCnt++;          //У׼�������+1
	if(1 == ExtiCount)
	{
		//��ʱ��3������0
		TIM_SetCounter(TIM3, 0);
		//�򿪶�ʱ��3
		TIM_Cmd(TIM3, ENABLE);
		//�򿪶�ʱ��4
		TIM_Cmd(TIM4, ENABLE);
	}
	else 
	{
		//����������ֵ
		TimeCount = TIM_GetCounter(TIM3);
		if(TimeCount >= TimeCountMin)
		{
			//�������
			if((PulseMileCount > (FeatureCoef/50))&&(1 == Acc))//���ڵ���0.02����
			{
				PulseTotalMile += PulseMileCount*100/FeatureCoef;
				PulseMileCount = 0;
			}
#ifdef SPEED_PINGJUN
			TimeCountArrayCount++;
			if(TimeCountArrayCount >= TIME_COUNT_ARRAY_SIZE)
			{
				TimeCountArrayCount = 0;
			}
			TimeCountArray[TimeCountArrayCount] = TimeCount;
#else 
                        TimeCountArray[0] = TimeCount; 
                	sum = TimeCountArray[0];
#endif
#ifdef  SPEED_PINGJUN
        u8	i = 0;
	u16	CountMin = 0xffff;
	u16	CountMax = 0;
                sum = 0;
		for(i=0; i<TIME_COUNT_ARRAY_SIZE; i++)
		{
			if(TimeCountArray[i] > CountMax)
			{
				CountMax = TimeCountArray[i];
			}
			if(TimeCountArray[i] < CountMin)
			{
				CountMin = TimeCountArray[i];
			}
			sum += TimeCountArray[i];
                }
		sum -= CountMin;
		sum -= CountMax;
		sum = sum/3;
#endif
                if(0 == sum)
                {
                    return;
                }
                if(1 == Acc)
                {
		v = (3600.0*(u32)1000000.0)/((float)sum*50.0*(float)FeatureCoef);//��Ӧ50us������λ
                temp = (int)(v);
                if((v-temp)>=0.5)
                {
                    temp += 1;
                }
#ifdef SPEED_LVBO
		if(temp > LastPulseSpeed)
		{
			if((temp-LastPulseSpeed) >= 50)//��������50��Ϊ�Ǹ���
			{
				Count3++;
				if(Count3 >= 100)//���Ų�������100�Σ���Ϊ���쳣�������ͻ��
				{
					Count3 = 0;
					PulseSpeed = temp;
					LastPulseSpeed = temp;
				}
			}
			else
			{
				Count3 = 0;
				PulseSpeed = temp;
				LastPulseSpeed = temp;
			}
		}
		else
		{
			PulseSpeed = temp;
			LastPulseSpeed = temp;
		}
#else
                PulseSpeed = temp;
		LastPulseSpeed = temp;
#endif
                }
                else
                {
                        PulseSpeed = 0;
		        LastPulseSpeed = 0;
                }
                
                // Ϊ���벻��������
                if(0 == Count3)
                {
                
                }
                if(0 == LastPulseSpeed)
                {
                
                }
                        
                
		}
		else
		{
			PulseMileCount--;//�������
			myPulseCnt--;
		}
		//��ʱ��3������0
		TIM_SetCounter(TIM3, 0);
	}	
}
/*********************************************************************
//��������	:Pulse_TimeTask
//����		:�����ⶨʱ����
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:0.2�����1��
*********************************************************************/
FunctionalState  Pulse_TimeTask(void)
{
	


	u8	Speed1 = 0;//�����ٶ�
	u8	Speed2 = 0;//GPS�ٶ�
  u8  flag = 0;
	static	u16	Count4 = 0;
	static	u16	Count5 = 0;
	u8	TmpData[5];
	static  u16	Count1 = 0;
	static  u16	Count2 = 0;
	static  u16	Count3 = 0;
	static  u16	TotalPulseSpeed = 0;//˲ʱ�ٶ�֮��
	static  u32	LasePulseTotalMile = 0;//��һ���ۼ���ʻ��̣��������,��λ��0.01����
	
        if(0 == (SpeedFlag&0x01))//�����ٶ�
	{
	        //�����ۼ����
	        Count1++;
	        if(Count1 >= 5)//1��дһ�����
	        {
		        Count1 = 0;
		        //if(0 == Io_ReadAlarmBit(ALARM_BIT_VSS_FAULT))dxl,2013.11.21,������vbug����ѡ��GPS�ٶ�ʱ�����⣬�����ۼ���̶��ڼ�¼
                
		        //{
                        if(LasePulseTotalMile != PulseTotalMile)
                        {
			                TmpData[3] = PulseTotalMile;	//���ֽ�
			                TmpData[2] = PulseTotalMile>>8;	//���ֽ�
			                TmpData[1] = PulseTotalMile>>16;	//���ֽ�
			                TmpData[0] = PulseTotalMile>>24;	//���ֽ�
			                FRAM_BufferWrite(FRAM_MILEAGE_ADDR, TmpData, FRAM_MILEAGE_LEN); //�洢�����
                        }
		       // }
	        }
	        //����ÿ����ƽ���ٶ�
	        Count2++;
	        if(Count2 >= 5)
	        {
		        Count2 = 0;
		        Count3++;
		        TotalPulseSpeed += PulseSpeed;
		        if(Count3 >= 60)
		        {
			      Count3 = 0;
			      PulseMinuteSpeed = TotalPulseSpeed/60;
			      TotalPulseSpeed = 0;
		        }
	        }
      }
	//��λ�����ٶȴ���������
        //�б���ʱ���ο�ʼ
	Speed1 = Pulse_GetSpeed();
	Speed2 = Gps_ReadSpeed();
	if((1 == Io_ReadStatusBit(STATUS_BIT_ACC))
	   &&(1 == Io_ReadStatusBit(STATUS_BIT_NAVIGATION)))
		  // &&(0==SpeedMonitor_GetCurSpeedType()))
	{
		if(Speed2 > 15)
		{
			if(0 == Speed1)
			{
				Count5 = 0;
				Count4++;
				if(Count4 >= 300)//����1����
				{
					Count4 = 0;
					//��λ�ٶȴ���������
					Io_WriteAlarmBit(ALARM_BIT_VSS_FAULT, SET);
                    //�����ٶ�����Ϊgps�ٶ�����
                    if((0x03 != SpeedFlag)&&(0x02 == SpeedFlag))//dxl,2015.5.11,0x00:�ֶ�����,0x01:�ֶ�GPS,0x02:�Զ�����,0x03:�Զ�GPS
                    {
                        //֮ǰ�������л�Ϊgps
                        flag = 0x03;//dxl,2015.5.11
                        EepromPram_WritePram(E2_SPEED_SELECT_ID, &flag, 1);
                        SpeedFlagUpdatePram();
                    }
				}
			}
			else
			{
				Count4 = 0;
				Count5++;
				if(Count5 >= 300)//����1����
				{
					Count5 = 0;
					//����ٶȴ���������
					Io_WriteAlarmBit(ALARM_BIT_VSS_FAULT, RESET);
                    //�����ٶ�����Ϊ�����ٶ�����
                    if((0x02 != SpeedFlag)&&(0x03 == SpeedFlag))//dxl,2015.5.11
                    {
                        //֮ǰ��GPS�л�Ϊ����
                        flag = 0x02;//dxl,2015.5.11
                        EepromPram_WritePram(E2_SPEED_SELECT_ID, &flag, 1);
                        SpeedFlagUpdatePram();
                    }
				}
			}
		}
		else
		{
			Count4 = 0;
			Count5 = 0;
		}
	}
	else
	{
		Count4 = 0;
		Count5 = 0;
	}
	return ENABLE;
}
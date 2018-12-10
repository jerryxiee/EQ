/********************************************************************
//��Ȩ˵��	:Shenzhen E-EYE Co. Ltd., Copyright 2009- All Rights Reserved.
//�ļ�����	:Test.c		
//����		:ʵ����ʻ��¼�ǵļ춨���� 
//�汾��	:
//������	:dxl
//����ʱ��	:2013.3
//�޸���	:
//�޸�ʱ��	:
//�޸ļ�Ҫ˵��	:
//��ע		:
***********************************************************************/
//***************�����ļ�*****************
#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "include.h"

//****************�궨��****************
//***************��������***************
//*****************��������****************
u8	TestFlag = 0;//0:�˳��˼춨״̬;1:��ǰ���ڼ춨״̬,�յ��춨����E0ʱ��1,�յ��춨����E4ʱ��0
u8	TestStatus = 0;//����״̬;E1H,�������������;E2H,��������ϵ��������;E3H,����ʵʱʱ��������
//****************��������*****************
/*********************************************************************
//��������	:Test_GetFlag
//����		:��ü춨״ֵ̬
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:0:�˳��˼춨״̬;1:��ǰ���ڼ춨״̬
*********************************************************************/
u8 Test_GetFlag(void)
{
	return TestFlag;
}
/*********************************************************************
//��������	:Test_EnterE0
//����		:E0H,����򱣳ּ춨״̬
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:�յ��춨����E0ʱ���ô˺���
*********************************************************************/
void Test_EnterE0(void)
{
	//�ر�ԭ�����������ж�
	Pulse_DisableExtiIsr();
	
	TestFlag = 1;
}
/*********************************************************************
//��������	:Test_EnterE1
//����		:E1H,�������������
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:�յ��춨����E1ʱ���ô˺���
*********************************************************************/
void Test_EnterE1(void)
{
    	//�ر�ԭ�����������ж�
	Pulse_DisableExtiIsr();
	//��ʼ����������
	Test_DisableExtiIsr();
	Test_ExtiInit();
	
	TestStatus = 0xE1;
}
/*********************************************************************
//��������	:Test_EnterE2
//����		:E2H,��������ϵ��������
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:�յ��춨����E2ʱ���ô˺���
*********************************************************************/
void Test_EnterE2(void)
{
	//��ʼ����������
	Test_DisableExtiIsr();
    	//����ԭ����������Ϊ�������ش���
	Pulse_TestExtiInit();
	//��ʼ����������Ϊ���
	Test_GpioInit();
	
	TestStatus = 0xE2;
	
}
/*********************************************************************
//��������	:Test_EnterE3
//����		:E3H,����ʵʱʱ��������
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:�յ��춨����E3ʱ���ô˺���
*********************************************************************/
void Test_EnterE3(void)
{
	//��ʼ����������
	Test_DisableExtiIsr();
    	//�ر�ԭ�����������ж�
	Pulse_DisableExtiIsr();
	//��ʱʱ��Ϊ0.5s
	Test_Tim3Init();
	//��ʼ����������Ϊ���
	Test_GpioInit();
	
	TestStatus = 0xE3;
	
}
/*********************************************************************
//��������	:Test_EnterE4
//����		:E4H,�˳��춨״̬,�ָ�������״̬
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:�յ��춨����E4ʱ���ô˺���
*********************************************************************/
void Test_EnterE4(void)
{
	//������������������
	Pulse_Init();
	//�رղ�������
	Test_DisableExtiIsr();
	
	TestStatus = 0;
	TestFlag = 0;
}
/*********************************************************************
//��������	:Test_ExtiInit
//����		:�ⲿ�жϳ�ʼ��
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
void Test_ExtiInit(void)
{
	EXTI_InitTypeDef   EXTI_InitStructure;
  	GPIO_InitTypeDef   GPIO_InitStructure;
  	NVIC_InitTypeDef   NVIC_InitStructure;

  	/* Enable GPIOB clock */
  	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  	/* Enable SYSCFG clock */
  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
  
  	/* Configure PB14 pin as input floating */
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
  	GPIO_Init(GPIOB, &GPIO_InitStructure);

  	/* Connect EXTI Line14 to PB14 pin */
  	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource14);

  	/* Configure EXTI Line14 */
  	EXTI_InitStructure.EXTI_Line = EXTI_Line14;
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  	EXTI_Init(&EXTI_InitStructure);

  	/* Enable and set EXTI14 Interrupt to the lowest priority */
  	NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}
/*********************************************************************
//��������	:Test_EnableExtiIsr
//����		:ʹ�ܣ������ⲿ�ж�
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
*********************************************************************/
void Test_EnableExtiIsr(void)
{
	NVIC_InitTypeDef   NVIC_InitStructure;
	/* Enable and set EXTI15_10_IRQn Interrupt to the lowest priority */
  	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}
/*********************************************************************
//��������	:Pulse_DisableExtiIsr
//����		:ʹ�ܣ������ⲿ�ж�
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
*********************************************************************/
void Test_DisableExtiIsr(void)
{
	NVIC_InitTypeDef   NVIC_InitStructure;
	/* Enable and set EXTI15_10_IRQn Interrupt to the lowest priority */
  	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&NVIC_InitStructure);
}
/*********************************************************************
//��������	:Test_ExtiIsr
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
void Test_ExtiIsr(void)
{
	Pulse_MileAdd();
	/* Clear the EXTI line 0 pending bit */
    	EXTI_ClearITPendingBit(EXTI_Line14);
}
/*********************************************************************
//��������	:Test_GpioInit
//����		:�������ų�ʼ��ΪGPIO
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
void Test_GpioInit(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
  
  	/* ʵ��GPIOʱ��*/
  	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

  	/* ����GPIO�� 50Mhz�������,������ */
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;  
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(GPIOB, &GPIO_InitStructure);
}
/*********************************************************************
//��������	:Test_Tim3Init
//����		:��ʱ��3��ʼ��,�������ʵʱʱ�����ź�
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
void Test_Tim3Init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	
	u16	PrescalerValue;
	 /* TIM3 clock enable */
  	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

  	/* Enable the TIM3 gloabal Interrupt */
  	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  	NVIC_Init(&NVIC_InitStructure);

  	/* Compute the prescaler value */
  	PrescalerValue = 2399;

  	/* Time base configuration */
  	TIM_TimeBaseStructure.TIM_Period = 10000;//0.5s
  	TIM_TimeBaseStructure.TIM_Prescaler = 0;
  	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

  	TIM_SetCounter(TIM3, 0);
  
  	/* Prescaler configuration */
  	TIM_PrescalerConfig(TIM3, PrescalerValue, TIM_PSCReloadMode_Immediate);

  	/* TIM Interrupts enable */
  	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

  	/* TIM3 enable counter */
  	TIM_Cmd(TIM3, ENABLE);
}

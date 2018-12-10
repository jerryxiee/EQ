/********************************************************************
//版权说明	:Shenzhen E-EYE Co. Ltd., Copyright 2009- All Rights Reserved.
//文件名称	:Test.c		
//功能		:实现行驶记录仪的检定功能 
//版本号	:
//开发人	:dxl
//开发时间	:2013.3
//修改者	:
//修改时间	:
//修改简要说明	:
//备注		:
***********************************************************************/
//***************包含文件*****************
#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "include.h"

//****************宏定义****************
//***************常量定义***************
//*****************变量定义****************
u8	TestFlag = 0;//0:退出了检定状态;1:当前处于检定状态,收到检定命令E0时置1,收到检定命令E4时清0
u8	TestStatus = 0;//测试状态;E1H,进入里程误差测量;E2H,进入脉冲系数误差测量;E3H,进入实时时钟误差测量
//****************函数定义*****************
/*********************************************************************
//函数名称	:Test_GetFlag
//功能		:获得检定状态值
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:0:退出了检定状态;1:当前处于检定状态
*********************************************************************/
u8 Test_GetFlag(void)
{
	return TestFlag;
}
/*********************************************************************
//函数名称	:Test_EnterE0
//功能		:E0H,进入或保持检定状态
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:收到检定命令E0时调用此函数
*********************************************************************/
void Test_EnterE0(void)
{
	//关闭原来的脉冲检测中断
	Pulse_DisableExtiIsr();
	
	TestFlag = 1;
}
/*********************************************************************
//函数名称	:Test_EnterE1
//功能		:E1H,进入里程误差测量
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:收到检定命令E1时调用此函数
*********************************************************************/
void Test_EnterE1(void)
{
    	//关闭原来的脉冲检测中断
	Pulse_DisableExtiIsr();
	//初始化测试引脚
	Test_DisableExtiIsr();
	Test_ExtiInit();
	
	TestStatus = 0xE1;
}
/*********************************************************************
//函数名称	:Test_EnterE2
//功能		:E2H,进入脉冲系数误差测量
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:收到检定命令E2时调用此函数
*********************************************************************/
void Test_EnterE2(void)
{
	//初始化测试引脚
	Test_DisableExtiIsr();
    	//配置原来的脉冲线为正负边沿触发
	Pulse_TestExtiInit();
	//初始化测试引脚为输出
	Test_GpioInit();
	
	TestStatus = 0xE2;
	
}
/*********************************************************************
//函数名称	:Test_EnterE3
//功能		:E3H,进入实时时钟误差测量
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:收到检定命令E3时调用此函数
*********************************************************************/
void Test_EnterE3(void)
{
	//初始化测试引脚
	Test_DisableExtiIsr();
    	//关闭原来的脉冲检测中断
	Pulse_DisableExtiIsr();
	//定时时间为0.5s
	Test_Tim3Init();
	//初始化测试引脚为输出
	Test_GpioInit();
	
	TestStatus = 0xE3;
	
}
/*********************************************************************
//函数名称	:Test_EnterE4
//功能		:E4H,退出检定状态,恢复成正常状态
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:收到检定命令E4时调用此函数
*********************************************************************/
void Test_EnterE4(void)
{
	//返回至正常的脉冲检测
	Pulse_Init();
	//关闭测试引脚
	Test_DisableExtiIsr();
	
	TestStatus = 0;
	TestFlag = 0;
}
/*********************************************************************
//函数名称	:Test_ExtiInit
//功能		:外部中断初始化
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
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
//函数名称	:Test_EnableExtiIsr
//功能		:使能（开）外部中断
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
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
//函数名称	:Pulse_DisableExtiIsr
//功能		:使能（开）外部中断
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
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
//函数名称	:Test_ExtiIsr
//功能		:外部中断
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
//		:
*********************************************************************/
void Test_ExtiIsr(void)
{
	Pulse_MileAdd();
	/* Clear the EXTI line 0 pending bit */
    	EXTI_ClearITPendingBit(EXTI_Line14);
}
/*********************************************************************
//函数名称	:Test_GpioInit
//功能		:测试引脚初始化为GPIO
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
//		:
*********************************************************************/
void Test_GpioInit(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
  
  	/* 实用GPIO时钟*/
  	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

  	/* 配置GPIO脚 50Mhz推挽输出,弱上拉 */
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;  
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(GPIOB, &GPIO_InitStructure);
}
/*********************************************************************
//函数名称	:Test_Tim3Init
//功能		:定时器3初始化,用于输出实时时钟秒信号
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
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

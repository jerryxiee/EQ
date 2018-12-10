/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : stm32f10x_it.c
* Author             : MCD Application Team
* Version            : V2.0.3
* Date               : 09/22/2008
* Description        : Main Interrupt Service Routines.
*                      This file provides template for all exceptions handler
*                      and peripherals interrupt service routine.
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "include.h"
#include "usb_hcd_int.h"
//********************宏定义*****************
//*******************外部变量*****************

extern const u8 SystemFaultType[][17];

extern USB_OTG_CORE_HANDLE          USB_OTG_Core;
extern USBH_HOST                    USB_Host;
//录音程序
extern uint32_t AUDIO_SAMPLE_LEN ;
extern uint16_t AUDIO_SAMPLE[] ;
extern u8 REC_TO_G711_FLAG ;
//******************函数声明******************
/*******************************************************************************
* Function Name  : NMIException
* Description    : This function handles NMI exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void NMI_Handler(void)
{
	u32	i,j;
	while(1)
	{
		Lcd_DisplayString(1, 0, 0, " NMI Handler! ", strlen(" NMI Handler! "));
	  
	  	for(j=0; j<10; j++)
	  	{

                IWDG_ReloadCounter();//喂狗
	  		for(i=0; i<5000000; i++)
	  		{
	  
	  		}
	  	}
	  	NVIC_SystemReset();
	}
}

/*******************************************************************************
* Function Name  : HardFaultException
* Description    : This function handles Hard Fault exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void HardFault_Handler(void)
{
	u32	i;
	u8	j;
        
  /* Go to infinite loop when Hard Fault exception occurs */
        
  while (1)
  {
	  Lcd_DisplayString(1, 0, 0, " HardFault Handler! ", strlen(" HardFault Handler! "));
	  
	  for(j=0; j<10; j++)
	  {
                IWDG_ReloadCounter();//喂狗
	  	for(i=0; i<5000000; i++)
	  	{
	  
	  	}
	  }
	  NVIC_SystemReset();
  }
        
}

/*******************************************************************************
* Function Name  : MemManageException
* Description    : This function handles Memory Manage exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void MemManage_Handler(void)
{
	u32	i;
	u8	j;
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
	  Lcd_DisplayString(1, 0, 0, " MemManage Handler! ", strlen(" MemManage Handler! "));
	  for(j=0; j<10; j++)
	  {
                IWDG_ReloadCounter();//喂狗
	  	for(i=0; i<5000000; i++)
	  	{
	  
	  	}
	  }
	  ///NVIC_SETFAULTMASK();
	  NVIC_SystemReset();
  }
}

/*******************************************************************************
* Function Name  : BusFaultException
* Description    : This function handles Bus Fault exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void BusFault_Handler(void)
{
	u32 	i;
	u8	j;
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
	  Lcd_DisplayString(1, 0, 0, " BusFault Handler! ", strlen(" BusFault Handler! "));
	  for(j=0; j<10; j++)
	  {
                IWDG_ReloadCounter();//喂狗
	  	for(i=0; i<5000000; i++)
	  	{
	  
	  	}
	  }
	  //NVIC_SETFAULTMASK();
	  NVIC_SystemReset();
  }
}

/*******************************************************************************
* Function Name  : UsageFaultException
* Description    : This function handles Usage Fault exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UsageFault_Handler(void)
{
	u32	i;
	u8	j;
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
	  Lcd_DisplayString(1, 0, 0, " UsageFault Handler! ", strlen(" UsageFault Handler! "));
	  for(j=0; j<10; j++)
	  {

                IWDG_ReloadCounter();//喂狗
	  	for(i=0; i<5000000; i++)
	  	{
	  
	  	}
	  }
	  //NVIC_SETFAULTMASK();
	  NVIC_SystemReset();
  }
}

/*******************************************************************************
* Function Name  : DebugMonitor
* Description    : This function handles Debug Monitor exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DebugMon_Handler(void)
{
	while(1)
	{
	
	}
}

/*******************************************************************************
* Function Name  : SVCHandler
* Description    : This function handles SVCall exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SVC_Handler(void)
{
	while(1)
	{
	
	}
}

/*******************************************************************************
* Function Name  : PendSVC
* Description    : This function handles PendSVC exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PendSV_Handler(void)
{
	while(1)
	{
	
	}
	
}

/*******************************************************************************
* Function Name  : SysTickHandler
* Description    : This function handles SysTick Handler.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SysTick_Handler(void)
{
        static u8  count = 0;
        
        
        Timer_ISR();
        
        count++;
        if(count >= 10)//0.5秒更新1次时间
        {
                count = 0;
                ReadRtc();//只在这里更新rtc,其他地方不需要再更新了
        }
  	
}

/*******************************************************************************
* Function Name  : WWDG_IRQHandler
* Description    : This function handles WWDG interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void WWDG_IRQHandler(void)
{
	while(1)
	{
	
	}
}

/*******************************************************************************
* Function Name  : PVD_IRQHandler
* Description    : This function handles PVD interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PVD_IRQHandler(void)
{
	EXTI_ClearITPendingBit(EXTI_Line16);
	while(1)
	{

	}
	
}

/*******************************************************************************
* Function Name  : TAMPER_IRQHandler
* Description    : This function handles Tamper interrupt request. 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TAMPER_IRQHandler(void)
{
	while(1)
	{
	
	}
}

/*******************************************************************************
* Function Name  : RTC_IRQHandler
* Description    : This function handles RTC global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RTC_IRQHandler(void)
{
	while(1)
	{
	
	}
  
}

/**********************************************                                                                                                                                                                                                                                                                                                                      *********************************
* Function Name  : FLASH_IRQHandler
* Description    : This function handles Flash interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void FLASH_IRQHandler(void)
{
	while(1)
	{
	
	}
}

/*******************************************************************************
* Function Name  : RCC_IRQHandler
* Description    : This function handles RCC interrupt request. 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RCC_IRQHandler(void)
{
	while(1)
	{
	
	}
}

/*******************************************************************************
* Function Name  : EXTI0_IRQHandler
* Description    : This function handles External interrupt Line 0 request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI0_IRQHandler(void)
{
	while(1)
	{
	
	}
}

/*******************************************************************************
* Function Name  : EXTI1_IRQHandler
* Description    : This function handles External interrupt Line 1 request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI1_IRQHandler(void)
{
	
	if(EXTI_GetITStatus(EXTI_Line1) != RESET)
  	{
      		USB_Host.usr_cb->OverCurrentDetected();
      		EXTI_ClearITPendingBit(EXTI_Line1);
  	}
}

/*******************************************************************************
* Function Name  : EXTI2_IRQHandler
* Description    : This function handles External interrupt Line 2 request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI2_IRQHandler(void)
{
	while(1)
	{
	
	}
}

/*******************************************************************************
* Function Name  : EXTI3_IRQHandler
* Description    : This function handles External interrupt Line 3 request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI3_IRQHandler(void)
{
	while(1)
	{
	
	}
}

/*******************************************************************************
* Function Name  : EXTI4_IRQHandler
* Description    : This function handles External interrupt Line 4 request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI4_IRQHandler(void)
{
    while(1)
	{
	
	}
}

/*******************************************************************************
* Function Name  : DMA1_Channel1_IRQHandler
* Description    : This function handles DMA1 Channel 1 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel1_IRQHandler(void)
{
	while(1)
	{
	
	}
}

/*******************************************************************************
* Function Name  : DMA1_Channel2_IRQHandler
* Description    : This function handles DMA1 Channel 2 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel2_IRQHandler(void)
{
	while(1)
	{
	
	}
}

/*******************************************************************************
* Function Name  : DMA1_Channel3_IRQHandler
* Description    : This function handles DMA1 Channel 3 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel3_IRQHandler(void)
{
	while(1)
	{
	
	}
}

/*******************************************************************************
* Function Name  : DMA1_Channel4_IRQHandler
* Description    : This function handles DMA1 Channel 4 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel4_IRQHandler(void)
{
	while(1)
	{
	
	}
}

/*******************************************************************************
* Function Name  : DMA1_Channel5_IRQHandler
* Description    : This function handles DMA1 Channel 5 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel5_IRQHandler(void)
{
	while(1)
	{
	
	}
}

/*******************************************************************************
* Function Name  : DMA1_Channel6_IRQHandler
* Description    : This function handles DMA1 Channel 6 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel6_IRQHandler(void)
{
	while(1)
	{
	
	}
}

/*******************************************************************************
* Function Name  : DMA1_Channel7_IRQHandler
* Description    : This function handles DMA1 Channel 7 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel7_IRQHandler(void)
{
	while(1)
	{
	
	}
}

/*******************************************************************************
* Function Name  : ADC1_2_IRQHandler
* Description    : This function handles ADC1 and ADC2 global interrupts requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ADC1_2_IRQHandler(void)
{
	while(1)
	{
	
	}
}

/*******************************************************************************
* Function Name  : USB_HP_CAN_TX_IRQHandler
* Description    : This function handles USB High Priority or CAN TX interrupts 
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USB_HP_CAN1_TX_IRQHandler(void)
{
  while(1)
	{
	
	}
}

/*******************************************************************************
* Function Name  : USB_LP_CAN_RX0_IRQHandler
* Description    : This function handles USB Low Priority or CAN RX0 interrupts 
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USB_LP_CAN1_RX0_IRQHandler(void)
{
  while(1)
	{
	
	}
}

/*******************************************************************************
* Function Name  : EXTI9_5_IRQHandler
* Description    : This function handles External lines 9 to 5 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI9_5_IRQHandler(void)
{
   	SleepDeep_ACC_EXTI_IRQ();
}
/******************************************************************************
* Function Name  : RTC_WKUP_IRQHandler
* Description    : This function handles RTC_WKUP_IRQ interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RTC_WKUP_IRQHandler(void) 
{ 
    SleepDeep_RTC_WKUP_IRQ();
}

/*******************************************************************************
* Function Name  : TIM1_BRK_IRQHandler
* Description    : This function handles TIM1 Break interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM1_BRK_IRQHandler(void)
{
	while(1)
	{
	
	}
}

/*******************************************************************************
* Function Name  : TIM1_UP_IRQHandler
* Description    : This function handles TIM1 overflow and update interrupt 
*                  request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM1_UP_IRQHandler(void)
{
	while(1)
	{
	
	}
}

/*******************************************************************************
* Function Name  : TIM1_TRG_COM_IRQHandler
* Description    : This function handles TIM1 Trigger and commutation interrupts 
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM1_TRG_COM_IRQHandler(void)
{
	while(1)
	{
	
	}
}

/*******************************************************************************
* Function Name  : TIM1_CC_IRQHandler
* Description    : This function handles TIM1 capture compare interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM1_CC_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM1, TIM_IT_CC1) != RESET)
	{   
		TIM_ClearITPendingBit(TIM1, TIM_IT_CC1);
		//TimerManage10ms();
	}
}

/*******************************************************************************
* Function Name  : TIM2_IRQHandler
* Description    : This function handles TIM2 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM2_IRQHandler(void)
{
	USB_OTG_BSP_TimerIRQ();
}

/*******************************************************************************
* Function Name  : TIM3_IRQHandler
* Description    : This function handles TIM3 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM3_IRQHandler(void)
{
	Pulse_Tim3Isr();
}

/*******************************************************************************
* Function Name  : TIM4_IRQHandler
* Description    : This function handles TIM4 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM4_IRQHandler(void)
{
   	Pulse_Tim4Isr();
}
/*******************************************************************************
* Function Name  : TIM5_IRQHandler
* Description    : This function handles TIM5 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM5_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET)
	{   
		TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
		TimerManage1s();
	}
}
/*******************************************************************************
* Function Name  : TIM7_IRQHandler
* Description    : This function handles TIM7 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM7_IRQHandler(void)
{
  if(TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET)
  {   
    TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
    TimerManage10ms();
  }
}

/*******************************************************************************
* Function Name  : I2C1_EV_IRQHandler
* Description    : This function handles I2C1 Event interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void I2C1_EV_IRQHandler(void)
{
	while(1)
	{
	
	}
}

/*******************************************************************************
* Function Name  : I2C1_ER_IRQHandler
* Description    : This function handles I2C1 Error interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void I2C1_ER_IRQHandler(void)
{
	while(1)
	{
	
	}
}

/*******************************************************************************
* Function Name  : I2C2_EV_IRQHandler
* Description    : This function handles I2C2 Event interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void I2C2_EV_IRQHandler(void)
{
	while(1)
	{
	
	}
}

/*******************************************************************************
* Function Name  : I2C2_ER_IRQHandler
* Description    : This function handles I2C2 Error interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void I2C2_ER_IRQHandler(void)
{
	while(1)
	{
	
	}
}

/*******************************************************************************
* Function Name  : SPI1_IRQHandler
* Description    : This function handles SPI1 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI1_IRQHandler(void)
{
	while(1)
	{
	
	}
}

/*******************************************************************************
* Function Name  : SPI2_IRQHandler
* Description    : This function handles SPI2 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI2_IRQHandler(void)
{
	while(1)
	{
	
	}
}

/*******************************************************************************
* Function Name  : USART1_IRQHandler
* Description    : This function handles USART1 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USART1_IRQHandler(void)
{
    COMM_IRQHandler(COM1); 
}
/*******************************************************************************
* Function Name  : USART2_IRQHandler
* Description    : This function handles USART2 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USART2_IRQHandler(void)
{
    COMM_IRQHandler(COM2);
}

/*******************************************************************************
* Function Name  : USART3_IRQHandler
* Description    : This function handles USART3 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USART3_IRQHandler(void)
{
    Usart3_Isr();
}

/*******************************************************************************
* Function Name  : EXTI15_10_IRQHandler
* Description    : This function handles External lines 15 to 10 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI15_10_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line10))
    {
        Pulse_ExtiIsr();
    }
    if(EXTI_GetITStatus(EXTI_Line14))
    {
        Test_ExtiIsr();
    }
}

/*******************************************************************************
* Function Name  : RTCAlarm_IRQHandler
* Description    : This function handles RTC Alarm interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RTCAlarm_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : USBWakeUp_IRQHandler
* Description    : This function handles USB WakeUp interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USBWakeUp_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : TIM8_BRK_IRQHandler
* Description    : This function handles TIM8 Break interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM8_BRK_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : TIM8_UP_IRQHandler
* Description    : This function handles TIM8 overflow and update interrupt 
*                  request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM8_UP_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : TIM8_TRG_COM_IRQHandler
* Description    : This function handles TIM8 Trigger and commutation interrupts 
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM8_TRG_COM_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : TIM8_CC_IRQHandler
* Description    : This function handles TIM8 capture compare interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM8_CC_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : ADC3_IRQHandler
* Description    : This function handles ADC3 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ADC_IRQHandler(void)
{
	while(1)
	{
	
	}
}

/*******************************************************************************
* Function Name  : FSMC_IRQHandler
* Description    : This function handles FSMC global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void FSMC_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : SDIO_IRQHandler
* Description    : This function handles SDIO global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SDIO_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : SPI3_IRQHandler
* Description    : This function handles SPI3 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI3_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : UART4_IRQHandler
* Description    : This function handles UART4 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UART4_IRQHandler(void)
{
     	if(GetUsartStatus(COM4, USART_IT_RXNE) != RESET)
        {
               Gps_RxIsr();
        }  
	if(GetUsartStatus(COM4, USART_IT_TXE) != RESET)
	{
		Gps_TxIsr();
	}
	if (USART_GetFlagStatus(UART4, USART_FLAG_ORE) != RESET)//注意！不能使用if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)来判断
    	{
        	USART_ReceiveData(UART4);
    	}

	//USART_ClearITPendingBit(UART4, USART_IT_TC);
	//USART_ClearITPendingBit(UART4, USART_IT_RXNE);
	//USART_ClearITPendingBit(UART4, USART_IT_TXE);
}

/*******************************************************************************
* Function Name  : UART5_IRQHandler
* Description    : This function handles UART5 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/

/*******************************************************************************
* Function Name  : USART6_IRQHandler
* Description    : This function handles USART1 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USART6_IRQHandler(void)
{
    COMM_IRQHandler(COM6);
}
/*******************************************************************************
* Function Name  : TIM6_IRQHandler
* Description    : This function handles TIM6 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM6_IRQHandler(void)
{

}

/*******************************************************************************
* Function Name  : DMA2_Channel1_IRQHandler
* Description    : This function handles DMA2 Channel 1 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA2_Channel1_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : DMA2_Channel2_IRQHandler
* Description    : This function handles DMA2 Channel 2 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA2_Channel2_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : DMA2_Channel3_IRQHandler
* Description    : This function handles DMA2 Channel 3 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA2_Channel3_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : DMA2_Channel4_5_IRQHandler
* Description    : This function handles DMA2 Channel 4 and DMA2 Channel 5
*                  interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA2_Channel4_5_IRQHandler(void)
{
	
}


  
/**
  * @brief  OTG_FS_IRQHandler
  *          This function handles USB-On-The-Go FS global interrupt request.
  *          requests.
  * @param  None
  * @retval None
  */
#ifdef USE_USB_OTG_FS  
void OTG_FS_IRQHandler(void)
#else
void OTG_HS_IRQHandler(void)
#endif
{
  USBH_OTG_ISR_Handler(&USB_OTG_Core);
}


/**
  * @brief I2S DMA的接收完成中断.
  * @param  None
  * @retval None
  */
void DMA1_Stream3_IRQHandler(void)
{
	
   static uint32_t Audio_Play_LEN = 0;
   if(DMA_GetFlagStatus(AUDIO_REC_DMA_STREAM,AUDIO_REC_DMA_FLAG_HT) == SET)
   {
     REC_TO_G711_FLAG = 1;
     DMA_ClearFlag(AUDIO_REC_DMA_STREAM,AUDIO_REC_DMA_FLAG_HT);
   }
   if(DMA_GetFlagStatus(AUDIO_REC_DMA_STREAM,AUDIO_REC_DMA_FLAG_TC) == SET)
   {
     REC_TO_G711_FLAG = 2;
     DMA_ClearFlag(AUDIO_REC_DMA_STREAM,AUDIO_REC_DMA_FLAG_TC);                 //需要手动清除该标志
     DMA_Cmd(AUDIO_REC_DMA_STREAM, DISABLE);
     Audio_Play_LEN += DMA_MAX_SZE;
     if((Audio_Play_LEN+DMA_MAX_SZE) >= AUDIO_SAMPLE_LEN){                      //(Audio_Play_LEN+DMA_MAX_SZE) 怕最后一次越位,这里截取掉不足DMA_MAX_SZE部分
       Audio_Play_LEN = 0; 
     }
     DMA_MemoryTargetConfig(AUDIO_REC_DMA_STREAM,(uint32_t)&AUDIO_SAMPLE[Audio_Play_LEN],DMA_Memory_0);
     DMA_SetCurrDataCounter(AUDIO_REC_DMA_STREAM,DMA_MAX_SZE); 
     DMA_Cmd(AUDIO_REC_DMA_STREAM, ENABLE);   
   }
}




/*******************************************************************************
* Function Name  : USB_HP_CAN_TX_IRQHandler
* Description    : This function handles USB High Priority or CAN TX interrupts
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CAN1_TX_IRQHandler(void)	//Weite
{
	/*
	CAN1->TSR |= 0x000F0F0F;//清除中断标志位
	Can_Isr_Tx();
	*/
}

/*******************************************************************************
* Function Name  : USB_LP_CAN_RX0_IRQHandler
* Description    : This function handles USB Low Priority or CAN RX0 interrupts
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CAN1_RX0_IRQHandler(void)	//Weite 
{
	/*
	CAN1->RF0R |= 0x00000018;//清除中断标志位

	if(CAN_MessagePending(CAN1, CAN_FIFO0) != 0)
	{
		Can_Isr_Rx();
		//通知系统调度，收到CAN总线数据
		SetEvTask(EV_CANREC);
	}
	*/
  
  	CAN1->RF0R |= 0x00000018;//清除中断标志位

	if(CAN_MessagePending(CAN1, CAN_FIFO0) != 0)
	{
          
		CAN_Isr_Rx(CAN1);
		//通知系统调度，收到CAN总线数据
		// SetEvTask(EV_CANREC);
	}
        
        
}

/*******************************************************************************
* Function Name  : CAN_RX1_IRQHandler
* Description    : This function handles CAN RX1 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CAN1_RX1_IRQHandler(void)	//Weite
{
	CAN1->RF1R |= 0x00000018;//清除中断标志位
	CAN1->IER  &= 0xFFFFFF0F;//清除中断允许位
}

/*******************************************************************************
* Function Name  : CAN_SCE_IRQHandler
* Description    : This function handles CAN SCE interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CAN1_SCE_IRQHandler(void)	//Weite
{
	CAN1->IER  &= 0xFFFC7FFF;//清除中断允许位
}





/*******************************************************************************
* Function Name  : USB_HP_CAN_TX_IRQHandler
* Description    : This function handles USB High Priority or CAN TX interrupts
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CAN2_TX_IRQHandler(void)	//Shigle
{
	CAN2->TSR |= 0x000F0F0F;//清除中断标志位
	//Can2_Isr_Tx();
}

/*******************************************************************************
* Function Name  : USB_LP_CAN_RX0_IRQHandler
* Description    : This function handles USB Low Priority or CAN RX0 interrupts
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CAN2_RX0_IRQHandler(void)	//Shigle
{
	CAN2->RF0R |= 0x00000018;//清除中断标志位

	
}

/*******************************************************************************
* Function Name  : CAN_RX2_IRQHandler
* Description    : This function handles CAN RX1 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CAN2_RX1_IRQHandler(void)	//Shigle
{
	CAN2->RF1R |= 0x00000018;//清除中断标志位
        
        if(CAN_MessagePending(CAN2, CAN_FIFO1) != 0)
	{
          
		CAN_Isr_Rx(CAN2);
		//通知系统调度，收到CAN总线数据
		// SetEvTask(EV_CANREC);
	}
        
	//CAN2->IER  &= 0xFFFFFF0F;//清除中断允许位
}

/*******************************************************************************
* Function Name  : CAN_SCE_IRQHandler
* Description    : This function handles CAN SCE interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CAN2_SCE_IRQHandler(void)	//Shigle
{
	CAN2->IER  &= 0xFFFC7FFF;//清除中断允许位
}












/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/

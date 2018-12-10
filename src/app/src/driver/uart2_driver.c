/*******************(C)深圳市伊爱高新技术开发有限公司版权所有*******************

**  文件名称:   Isr.c
**  创建作者:   杨忠义
**  版本编号:   V1.1.0.0
**  创建日期:   20100113
**  功能说明:  	主要完成各中断处理程序
**  修改记录:

**  备    注:
*******************************************************************************/
#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "include.h"

extern  DATA_SEND   gSendData;
extern  MODULE_DAT  gGprsObject;
extern  u8 Recorder_SendData(u8 *pData, u16 Len);
extern  u8 PhotoPacketDataAlreadySendFlag;//图像分包数据已上传标准，1为已发送到通信模块
UART2_RECEIVE	uart2_recdat;				//串口2接收
UART2_SENDER	uart2_senddat;				//串口2发送

/********************************************************************
* 名称 : Uart2_RX_ISR
* 功能 : 串口2 接收数据
*
* 输入: 无
* 输出: 无
*
* 全局变量:
* 调用函数:
*
* 中断资源:  (没有,不写)
*
* 备注: (修改记录内容、时间)
********************************************************************/
void Uart2_RX_ISR(void)
{
	u16 next_pos = 0;
	u8  ch;

	ch = GetUsartReceiveData(COM5);
	next_pos =  uart2_recdat.iEnd + 1;
	if (next_pos >= UART2_REC_MAXLEN)
		next_pos = 0;
	
	uart2_recdat.uart2_recBuff[uart2_recdat.iEnd] = ch;
	uart2_recdat.iEnd = next_pos;

	uart2_recdat.uart2_timeout = 0;
#ifdef SEND_COMMUNICATION_DATA_TO_DB9
       Recorder_SendData(&ch, 1);
#endif
    Tts_RecData(ch);//dxl,2014.6.11增加内置TTS功能时同步增加这行
}

/********************************************************************
* 名称 : Uart2_TX_ISR
* 功能 : 串口2 发送数据
*
* 输入: 无
* 输出: 无
*
* 全局变量:
* 调用函数:
*
* 中断资源:  (没有,不写)
*
* 备注: (修改记录内容、时间)
********************************************************************/
u8 Uart2_TX_ISR(void)
{
	u8 ch,flag=1;
	u16 sndlen;
    static u8 LastCh = 0;//上一次发送的字符
	
	if(uart2_senddat.uart2_sendinglen < uart2_senddat.uart2_sendlen)
	{
		sndlen = uart2_senddat.uart2_sendinglen;
		ch = uart2_senddat.uart2_sendBuff[sndlen++];
		uart2_senddat.uart2_sendinglen = sndlen;
		UsartSendData(COM5, ch);
        if((0x08 == LastCh)&&(0x01 == ch))
        {
            PhotoPacketDataAlreadySendFlag = 1;  
        }
        LastCh = ch;
#ifdef SEND_COMMUNICATION_DATA_TO_DB9
        Recorder_SendData(&ch, 1);
#endif
	}else {
		flag = 0;
		uart2_senddat.uart2_sendlen = 0;
		uart2_senddat.uart2_sendinglen = 0;
                gSendData.inDatFlg = 0;
	}
	
	return(flag);
}

/*******************************************************************************
**  函数名称  : void Uart2_HwInit(void)
**  函数功能  : 硬件初始化函数，包括用到的GPIO 中断 USART初始化相关参数
**  输    入  : 无
**  输    出  : 无

**  全局变量 ：GPIO_InitStructure IO口初始化结构体 NVIC_InitStructure 中断向量设置结构体
               USART_InitStructure 串口初始化结构体

**  调用函数  : 调用以下STM32标准库函数：
                void USART_ITConfig(USART_TypeDef* USARTx, u16 USART_IT, FunctionalState NewState)
                void NVIC_Init(NVIC_InitTypeDef* NVIC_InitStruct)
                void GPIO_Init(GPIO_TypeDef* GPIOx, GPIO_InitTypeDef* GPIO_InitStruct)
                void USART_Cmd(USART_TypeDef* USARTx, FunctionalState NewState)
                void USART_ITConfig(USART_TypeDef* USARTx, u16 USART_IT, FunctionalState NewState)
**  中断资源  : 无
**  备    注  :
*******************************************************************************/
void Uart2_HwInit(int type)
{
    USART_InitTypeDef USART_InitStructure;
    
    memset(&USART_InitStructure,0,sizeof(USART_InitTypeDef));
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    
    COM_Init(COM5, &USART_InitStructure);  
    COM_NVIC_Config(COM5);                               //使能串口中断

    SetUsartITConfig(COM5,USART_IT_RXNE,ENABLE);        //使能串口接收中断
    
    //GpioOutInit(GPS_CTR);                               //初始GPS电源控制脚
	
    //GpioOutOff(GPS_CTR);                                //初始关闭GPS电源	
}

/*******************************************************************************
**  函数名称  : uart2driver_addsend
**  函数功能  : 增加待发送数据
**  输    入  : senddat:待发送数据
** 		length: 待发送数据长度
**  输    出  :

**  备    注  :
*******************************************************************************/
u16 uart2driver_addsend(const u8 *senddat, u16 length)
{
	u16 nowsendlen;
	
	nowsendlen = uart2_senddat.uart2_sendlen;
	
	if( UART2_SEND_MAXLEN - nowsendlen < length )
	{
		//发送缓冲不够用了
		return 0;
	}
	
	//USART_ITConfig(USART2, USART_IT_TXE, DISABLE);	//关闭发送中断	
	memcpy( uart2_senddat.uart2_sendBuff + nowsendlen, senddat, length );
	uart2_senddat.uart2_sendlen  += length;
	//uart2_sendinglen += length;
	//USART_ITConfig(USART2, USART_IT_TXE, ENABLE);//启动发送
	//if (0 == gGprsObject.startFlg) {
		//gGprsObject.startTmr = 4;			//100ms
        gGprsObject.startTmr = 1;//减少延时，确保1秒发送1包
		gGprsObject.startFlg = 1;
		GSM_DTR_EN();
	//}
	return length;
}
/*******************(C)�������������¼����������޹�˾��Ȩ����*******************

**  �ļ�����:   Isr.c
**  ��������:   ������
**  �汾���:   V1.1.0.0
**  ��������:   20100113
**  ����˵��:  	��Ҫ��ɸ��жϴ������
**  �޸ļ�¼:

**  ��    ע:
*******************************************************************************/
#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "include.h"

extern  DATA_SEND   gSendData;
extern  MODULE_DAT  gGprsObject;
extern  u8 Recorder_SendData(u8 *pData, u16 Len);
extern  u8 PhotoPacketDataAlreadySendFlag;//ͼ��ְ��������ϴ���׼��1Ϊ�ѷ��͵�ͨ��ģ��
UART2_RECEIVE	uart2_recdat;				//����2����
UART2_SENDER	uart2_senddat;				//����2����

/********************************************************************
* ���� : Uart2_RX_ISR
* ���� : ����2 ��������
*
* ����: ��
* ���: ��
*
* ȫ�ֱ���:
* ���ú���:
*
* �ж���Դ:  (û��,��д)
*
* ��ע: (�޸ļ�¼���ݡ�ʱ��)
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
    Tts_RecData(ch);//dxl,2014.6.11��������TTS����ʱͬ����������
}

/********************************************************************
* ���� : Uart2_TX_ISR
* ���� : ����2 ��������
*
* ����: ��
* ���: ��
*
* ȫ�ֱ���:
* ���ú���:
*
* �ж���Դ:  (û��,��д)
*
* ��ע: (�޸ļ�¼���ݡ�ʱ��)
********************************************************************/
u8 Uart2_TX_ISR(void)
{
	u8 ch,flag=1;
	u16 sndlen;
    static u8 LastCh = 0;//��һ�η��͵��ַ�
	
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
**  ��������  : void Uart2_HwInit(void)
**  ��������  : Ӳ����ʼ�������������õ���GPIO �ж� USART��ʼ����ز���
**  ��    ��  : ��
**  ��    ��  : ��

**  ȫ�ֱ��� ��GPIO_InitStructure IO�ڳ�ʼ���ṹ�� NVIC_InitStructure �ж��������ýṹ��
               USART_InitStructure ���ڳ�ʼ���ṹ��

**  ���ú���  : ��������STM32��׼�⺯����
                void USART_ITConfig(USART_TypeDef* USARTx, u16 USART_IT, FunctionalState NewState)
                void NVIC_Init(NVIC_InitTypeDef* NVIC_InitStruct)
                void GPIO_Init(GPIO_TypeDef* GPIOx, GPIO_InitTypeDef* GPIO_InitStruct)
                void USART_Cmd(USART_TypeDef* USARTx, FunctionalState NewState)
                void USART_ITConfig(USART_TypeDef* USARTx, u16 USART_IT, FunctionalState NewState)
**  �ж���Դ  : ��
**  ��    ע  :
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
    COM_NVIC_Config(COM5);                               //ʹ�ܴ����ж�

    SetUsartITConfig(COM5,USART_IT_RXNE,ENABLE);        //ʹ�ܴ��ڽ����ж�
    
    //GpioOutInit(GPS_CTR);                               //��ʼGPS��Դ���ƽ�
	
    //GpioOutOff(GPS_CTR);                                //��ʼ�ر�GPS��Դ	
}

/*******************************************************************************
**  ��������  : uart2driver_addsend
**  ��������  : ���Ӵ���������
**  ��    ��  : senddat:����������
** 		length: ���������ݳ���
**  ��    ��  :

**  ��    ע  :
*******************************************************************************/
u16 uart2driver_addsend(const u8 *senddat, u16 length)
{
	u16 nowsendlen;
	
	nowsendlen = uart2_senddat.uart2_sendlen;
	
	if( UART2_SEND_MAXLEN - nowsendlen < length )
	{
		//���ͻ��岻������
		return 0;
	}
	
	//USART_ITConfig(USART2, USART_IT_TXE, DISABLE);	//�رշ����ж�	
	memcpy( uart2_senddat.uart2_sendBuff + nowsendlen, senddat, length );
	uart2_senddat.uart2_sendlen  += length;
	//uart2_sendinglen += length;
	//USART_ITConfig(USART2, USART_IT_TXE, ENABLE);//��������
	//if (0 == gGprsObject.startFlg) {
		//gGprsObject.startTmr = 4;			//100ms
        gGprsObject.startTmr = 1;//������ʱ��ȷ��1�뷢��1��
		gGprsObject.startFlg = 1;
		GSM_DTR_EN();
	//}
	return length;
}
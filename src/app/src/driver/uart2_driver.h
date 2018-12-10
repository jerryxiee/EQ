/*******************(C)深圳市伊爱高新技术开发有限公司版权所有*******************

**  文件名称:   Isr.h
**  创建作者:   杨忠义
**  版本编号:   V1.1.0.0
**  创建日期:   20100113
**  功能说明:  	文件uart2_driver.c的头文件
**  修改记录:

**  备    注:
*******************************************************************************/
#ifndef _UART2_DRIVER_H_
#define _UART2_DRIVER_H_

#include "stm32f2xx.h"

//#define UART2_REC_MAXLEN	    1024	//接收缓冲大小
#define UART2_REC_MAXLEN	    2048	//接收缓冲大小
#define UART2_SEND_MAXLEN     	1400	//发送缓冲的大小

#define IsUart2RecvQueueNotEmpty() (uart2_recdat.uart2_reclen > 0)//(uart2_recdat.iEnd != uart2_recdat.iStart)

typedef struct {
	u8	rec_chanel;				//1:GPRS;0:短信
	u16 	uart2_timeout;				//接收超时
	u16  	uart2_reclen;	  			//串口接收长度
	u16     iStart;
	u16     iEnd;
	u8    	uart2_recBuff[UART2_REC_MAXLEN];	//接收缓冲区
}UART2_RECEIVE;

enum {
	UART_MODE = 0,	//PA2，PA3作为IO
	IO_MODE		
};

typedef struct {
	u16  	uart2_sendlen;  			//串口发送数据总长度
	u16  	uart2_sendinglen;  			//串口正在长度
	u8    	uart2_sendBuff[UART2_SEND_MAXLEN]; 	//发送缓冲区
}UART2_SENDER;

extern UART2_RECEIVE	uart2_recdat;				//串口2接收

extern UART2_SENDER	uart2_senddat;				//串口2发送

void Uart2_RX_ISR(void);
u8 Uart2_TX_ISR(void);
void Uart2_HwInit(int type);
u16 uart2driver_addsend(const u8 *senddat, u16 length);
/********************************************************************
* 名称 : Q228_hwInit
* 功能 : 初始化Q228模块的CPU资源
*
* 输入: 无
* 输出: 无
* 中断资源:  uart2收发中断
*
* 备注: (修改记录内容、时间)
********************************************************************/
void Q228_hwInit(void);
#endif

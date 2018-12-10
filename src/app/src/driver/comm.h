/*******************************************************************************
 * File Name:			comm.h 
 * Function Describe:	
 * Relate Module:		串口中断
 * Writer:				Joneming
 * Date:				2014-02-21
 * ReWriter:			
 * Date:				
 *******************************************************************************/

#ifndef __COMM__H_
#define __COMM__H_

#ifdef __cplusplus
 extern "C" {
#endif 

#include "Usart_drive.h"

//****************宏定义********************
#define COM1_MAX_LEN_TX   1024                                          //发送缓冲区长度
#define COM1_MAX_LEN_RX   1024                                          //接收缓冲区长度
///////////////////////////////////
#define COM2_MAX_LEN_TX   512                                          //发送缓冲区长度
#define COM2_MAX_LEN_RX   160                                          //接收缓冲区长度
//////////////////
#define COM3_MAX_LEN_TX   2                                          //发送缓冲区长度
#define COM3_MAX_LEN_RX   2                                          //接收缓冲区长度
/////////////////
#define COM4_MAX_LEN_TX   2                                          //发送缓冲区长度
#define COM4_MAX_LEN_RX   2                                          //接收缓冲区长度
///////////////////////////////////
#define COM5_MAX_LEN_TX   2                                          //发送缓冲区长度
#define COM5_MAX_LEN_RX   2                                          //接收缓冲区长度
///////////////////////
#define COM6_MAX_LEN_TX   256                                          //发送缓冲区长度
#define COM6_MAX_LEN_RX   128                                          //接收缓冲区长度

//****************外部变量*******************
//****************全局变量*******************
//结构类型
typedef struct{	
    unsigned short recvlen;         //串口接收数据长度,接收并处理完后清除,初始化清0	
    unsigned short resendlen;       //打包数据长度	
    unsigned char resendCnt;        //尝试重发发送次数
    unsigned char recvfreeCnt;      //接收空闲计数器	
    unsigned char *recvBuff;        //解析缓冲
    unsigned char *resendBuff;      //发送缓冲
}ST_COMM;
//****************函数声明******************* 
void COMM_putc(COM_TypeDef COM,int ch);
void COMM_putstr(COM_TypeDef COM,char* str);
/*************************************************************
** 函数名称: COMM_SetBaudRate
** 功能描述: 设置串口波特率
** 入口参数: COM串口,baud_rate:串口波特率
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/ 
void COMM_SetBaudRate(COM_TypeDef COM,unsigned long baud_rate);
/*************************************************************
** 函数名称: COMM_SetBaudRateEx
** 功能描述: 设置串口波特率(打印机专用)(9位,奇校验)
** 入口参数: COM串口,baud_rate:串口波特率
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/ 
void COMM_SetBaudRateEx(COM_TypeDef COM,unsigned long baud_rate);
/*************************************************************
** 函数名称: COMM_SetBaudRateInitialize
** 功能描述: 初始化串口并设置串口波特率
** 入口参数: COM串口,baud_rate:串口波特率
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: COM_Init
*************************************************************/ 
void COMM_SetBaudRateInitialize(COM_TypeDef COM,unsigned long baud_rate);
/*************************************************************
** 函数名称: COMM_Initialize
** 功能描述: 初始化串口,(包括收发队列初始化、串口使能及接收中断)
** 入口参数: COM串口,baud_rate:串口波特率
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/ 
void COMM_Initialize(COM_TypeDef COM,unsigned long baud_rate);
/*************************************************************
** 函数名称: COMM_CtrlEnableInitialize
** 功能描述: 串口收发方向控制初始化
** 入口参数: COM串口,IoNum:使能管脚
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/ 
void COMM_CtrlEnableInitialize(COM_TypeDef COM,void (*recvEnable)(void),void (*sendEnable)(void));
/*************************************************************
** 函数名称: COMM_SendData
** 功能描述: 发送数据
** 入口参数: COM:串口,buff数据首地址,len数据长度
** 出口参数: 
** 返回参数: 0:成功；1:失败,0xff:超出缓存长度
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned char COMM_SendData(COM_TypeDef COM,unsigned char *buff, unsigned short len);
/*************************************************************
** 函数名称: COMM_ReadData
** 功能描述: 读取缓冲区里面的数据
** 入口参数: COM:串口,buff数据缓冲区,len读取长度
** 出口参数: 
** 返回参数: 有数据则返回读取的字节数,无则返回0
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned short COMM_ReadData(COM_TypeDef COM,unsigned char *buff,unsigned short len);
/*************************************************************
** 函数名称: COMM_RxIsr
** 功能描述: 数据接收中断处理函数
** 入口参数: COM串口
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/ 
void COMM_RxIsr(COM_TypeDef COM);
/*************************************************************
** 函数名称: COMM_TxIsr
** 功能描述: 发送中断处理函数
** 入口参数: COM串口
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/ 
void COMM_TxIsr(COM_TypeDef COM);
/*************************************************************
** 函数名称: COMM_IRQHandler
** 功能描述: 串口中断
** 入口参数: COM:串口
** 出口参数: 无
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void COMM_IRQHandler(COM_TypeDef COM);
/*************************************************************
** 函数名称: COMM_GetSendBusy
** 功能描述: 获得当前串口发送忙标志
** 入口参数: 
** 出口参数: 无
** 返回参数: 1:串口忙,0串口空闲
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char COMM_GetSendBusy(COM_TypeDef COM);
#ifdef __cplusplus
}
#endif   

#endif /* __COMM__H_ */
/******************************************************************************
**                            End Of File
******************************************************************************/


/*******************************************************************************
 * File Name:			comm.c 
 * Function Describe:	
 * Relate Module:		串口中断
 * Writer:				Joneming
 * Date:				2014-02-21
 * ReWriter:			
 * Date:				
 *******************************************************************************/

//****************包含文件*****************
#include "hal.h"
#include "Comm.h"
#include "public.h"
////////////////////////
enum 
{
    COMM_ATTRIB_EMPTY=0,//
    COMM_ATTRIB_CTRL_VALID,//    
    COMM_ATTRIB_CTRL_SEND,//  
    COMM_ATTRIB_CTRL_RECV,// 
    COMM_ATTRIB_MAX
}E_COMMATTRIB;

typedef struct
{
    ST_QUEUE stRxQueue;//接收队列
    ST_QUEUE stTxQueue;//发送队列
    unsigned char sendBusy;//
    unsigned char ctrlValidFlag;//
    unsigned char ctrlStatus;//
    void (*recvEnable)(void);//
    void (*sendEnable)(void);//
}ST_COMM_ATTRIB;
////////////////////////////////
static ST_COMM_ATTRIB s_stCommAttrib[COM_MAX];
///////////////////////
static QueueData s_QDCOMRxData1[COM1_MAX_LEN_RX+1];
static QueueData s_QDCOMTxData1[COM1_MAX_LEN_TX+1];
///////////////////////////
static QueueData s_QDCOMRxData2[COM2_MAX_LEN_RX+1];
static QueueData s_QDCOMTxData2[COM2_MAX_LEN_TX+1];
////////////////////////////
static QueueData s_QDCOMRxData3[COM3_MAX_LEN_RX+1];
static QueueData s_QDCOMTxData3[COM3_MAX_LEN_TX+1];
///////////////////////////
static QueueData s_QDCOMRxData4[COM4_MAX_LEN_RX+1];
static QueueData s_QDCOMTxData4[COM4_MAX_LEN_TX+1];
/////////////////////////////
static QueueData s_QDCOMRxData5[COM5_MAX_LEN_RX+1];
static QueueData s_QDCOMTxData5[COM5_MAX_LEN_TX+1];
///////////////////////
static QueueData s_QDCOMRxData6[COM6_MAX_LEN_RX+1];
static QueueData s_QDCOMTxData6[COM6_MAX_LEN_TX+1];
////////////////

//****************函数声明*******************
/////////////////////接收队列////////////////////////////////////////////
//////////////入队列宏////////////////////////
#define Insert_COMRXQueue(COM,message)\
		LZM_QueueInsert(&s_stCommAttrib[COM].stRxQueue,message)
/////////////批量出队列宏////////////////////////	
#define BatchDelete_COMRXQueue(COM,dest,maxcount)\
        LZM_QueueDataBatchDelete(&s_stCommAttrib[COM].stRxQueue,dest,maxcount);
/////////////////////发送队列////////////////////////////////////////////
//////////////入队列宏////////////////////////
#define Insert_COMTXQueue(COM,message)\
		LZM_QueueInsert(&s_stCommAttrib[COM].stTxQueue,message)
/////////////批量入队列宏////////////////////////	
#define BatchInsert_COMTXQueue(COM,dest,datalen)\
        LZM_QueueDataBatchInsert(&s_stCommAttrib[COM].stTxQueue,dest,datalen);
/////////////出队列宏////////////////////////
#define Delete_COMTXQueue(COM)\
		LZM_QueueDelete(&s_stCommAttrib[COM].stTxQueue)
//////////////列队为空//////////////////////////////////
#define Empty_COMTXQueue(COM)\
		LZM_QueueEmpty(&s_stCommAttrib[COM].stTxQueue)
/////////////队列剩余空间////////////////////////
#define FreeSpace_COMTXQueue(COM)\
		LZM_QueueDataFreeSpace(&s_stCommAttrib[COM].stTxQueue)

//****************函数定义******************
/*********************************************************************
//函数名称	:Init_COMRXQueue
//功能		
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void COMM_QueueInit(COM_TypeDef COM)
{
    switch(COM)
    {
        case COM1:
            LZM_QueueDataInit(&s_stCommAttrib[COM].stTxQueue,s_QDCOMTxData1,COM1_MAX_LEN_TX);
            LZM_QueueDataInit(&s_stCommAttrib[COM].stRxQueue,s_QDCOMRxData1,COM1_MAX_LEN_RX);
            break;
        case COM2:
            LZM_QueueDataInit(&s_stCommAttrib[COM].stTxQueue,s_QDCOMTxData2,COM2_MAX_LEN_TX);
            LZM_QueueDataInit(&s_stCommAttrib[COM].stRxQueue,s_QDCOMRxData2,COM2_MAX_LEN_RX);
            break;
        case COM3:
            LZM_QueueDataInit(&s_stCommAttrib[COM].stTxQueue,s_QDCOMTxData3,COM3_MAX_LEN_TX);
            LZM_QueueDataInit(&s_stCommAttrib[COM].stRxQueue,s_QDCOMRxData3,COM3_MAX_LEN_RX);
            break;
        case COM4:
            LZM_QueueDataInit(&s_stCommAttrib[COM].stTxQueue,s_QDCOMTxData4,COM4_MAX_LEN_TX);
            LZM_QueueDataInit(&s_stCommAttrib[COM].stRxQueue,s_QDCOMRxData4,COM4_MAX_LEN_RX);
            break;
        case COM5:
            LZM_QueueDataInit(&s_stCommAttrib[COM].stTxQueue,s_QDCOMTxData5,COM5_MAX_LEN_TX);
            LZM_QueueDataInit(&s_stCommAttrib[COM].stRxQueue,s_QDCOMRxData5,COM5_MAX_LEN_RX);
            break;
        case COM6:
            LZM_QueueDataInit(&s_stCommAttrib[COM].stTxQueue,s_QDCOMTxData6,COM6_MAX_LEN_TX);
            LZM_QueueDataInit(&s_stCommAttrib[COM].stRxQueue,s_QDCOMRxData6,COM6_MAX_LEN_RX);
            break;
        default:
            break;
    }
}

/*************************************************************
** 函数名称: COMM_SetBaudRateEx
** 功能描述: 设置串口波特率(打印机专用)(9位,奇校验)
** 入口参数: COM串口,baud_rate:串口波特率
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/ 
void COMM_SetBaudRateEx(COM_TypeDef COM,unsigned long baud_rate)
{
    USART_InitTypeDef USART_InitStructure;
    //////////////////////////////
    if(COM>=COM_MAX)return;  
    memset(&USART_InitStructure,0,sizeof(USART_InitTypeDef));    
    USART_InitStructure.USART_BaudRate = baud_rate;                //通信波特路
    USART_InitStructure.USART_WordLength = USART_WordLength_9b;    //9位数据位
    USART_InitStructure.USART_StopBits = USART_StopBits_1;         //1位停止位
    USART_InitStructure.USART_Parity = USART_Parity_Odd ;          //奇校验
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(GetUsartPtr(COM), &USART_InitStructure);
}
/*************************************************************
** 函数名称: COMM_SetBaudRate
** 功能描述: 设置串口波特率
** 入口参数: COM串口,baud_rate:串口波特率
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:USART_Init
*************************************************************/ 
void COMM_SetBaudRate(COM_TypeDef COM,unsigned long baud_rate)
{
    USART_InitTypeDef USART_InitStructure;
    //////////////////////////////
    if(COM>=COM_MAX)return;
    memset(&USART_InitStructure,0,sizeof(USART_InitTypeDef));    
    USART_InitStructure.USART_BaudRate = baud_rate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;    
    USART_Init(GetUsartPtr(COM), &USART_InitStructure);
}
/*************************************************************
** 函数名称: COMM_SetBaudRateInitialize
** 功能描述: 设置串口波特率
** 入口参数: COM串口,baud_rate:串口波特率
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: COM_Init
*************************************************************/ 
void COMM_SetBaudRateInitialize(COM_TypeDef COM,unsigned long baud_rate)
{
    USART_InitTypeDef USART_InitStructure;
    //////////////////////////////
    if(COM>=COM_MAX)return;
    //default reset state
    USART_DeInit(GetUsartPtr(COM));
    //////////////////////
    memset(&USART_InitStructure,0,sizeof(USART_InitTypeDef));    
    USART_InitStructure.USART_BaudRate = baud_rate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;    
    COM_Init(COM, &USART_InitStructure);
    //打开串口中断.
    COM_NVIC_Config(COM);                                //使能串口中断
    /////使能串口接收中断//////////////////
    SetUsartITConfig(COM,USART_IT_RXNE,ENABLE);          //使能串口接收中断
}
/*************************************************************
** 函数名称: COMM_Initialize
** 功能描述: 初始化串口,(包括收发队列初始化、串口使能及接收中断)
** 入口参数: COM串口,baud_rate:串口波特率
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/ 
void COMM_Initialize(COM_TypeDef COM,unsigned long baud_rate)
{
    if(COM>=COM_MAX)return;
    ////////////////
    memset(&s_stCommAttrib[COM],0,sizeof(ST_COMM_ATTRIB));
    ////配置收发缓冲区/////////////
    COMM_QueueInit(COM);
    ////////////////////// 
    COMM_SetBaudRateInitialize(COM,baud_rate);
    ////////////////////
}

/*************************************************************OK
** 函数名称: Comm_CtrlEnableRecv
** 功能描述: 使能串口接收
** 入口参数: COM 串口
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/  
void Comm_CtrlEnableRecv(COM_TypeDef COM)
{
    if(COMM_ATTRIB_CTRL_VALID != s_stCommAttrib[COM].ctrlValidFlag)return;
    if(COMM_ATTRIB_CTRL_RECV == s_stCommAttrib[COM].ctrlStatus)return;
    s_stCommAttrib[COM].ctrlStatus = COMM_ATTRIB_CTRL_RECV;
    if(NULL!=s_stCommAttrib[COM].recvEnable)
    {
        s_stCommAttrib[COM].recvEnable();
    }
}
/*************************************************************OK
** 函数名称: Comm_CtrlEnableSend
** 功能描述: 使能串口发送
** 入口参数: COM 串口
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/  
void Comm_CtrlEnableSend(COM_TypeDef COM)
{
    if(COMM_ATTRIB_CTRL_VALID != s_stCommAttrib[COM].ctrlValidFlag)return;
    if(COMM_ATTRIB_CTRL_SEND == s_stCommAttrib[COM].ctrlStatus)return;
    s_stCommAttrib[COM].ctrlStatus = COMM_ATTRIB_CTRL_SEND;
    if(NULL != s_stCommAttrib[COM].sendEnable)
    {
        s_stCommAttrib[COM].sendEnable();
    }
}
/*************************************************************
** 函数名称: COMM_CtrlEnableInitialize
** 功能描述: 串口收发方向控制初始化
** 入口参数: COM串口,recvEnable:使能接收,sendEnable:使能发送
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void COMM_CtrlEnableInitialize(COM_TypeDef COM,void (*recvEnable)(void),void (*sendEnable)(void))
{
    s_stCommAttrib[COM].ctrlStatus      = COMM_ATTRIB_EMPTY;
    s_stCommAttrib[COM].ctrlValidFlag   = COMM_ATTRIB_CTRL_VALID;
    s_stCommAttrib[COM].recvEnable      = recvEnable;
    s_stCommAttrib[COM].sendEnable      = sendEnable;
    //////////////////////
    Comm_CtrlEnableRecv(COM);
}
/*************************************************************
** 函数名称: COMM_putc
** 功能描述: 发送一个字符
** 入口参数: COM串口,ch
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/ 
void COMM_putc(COM_TypeDef COM,int ch)
{
    while((GetUsartPtr(COM)->SR & USART_FLAG_TC) == RESET);
    GetUsartPtr(COM)->DR = (unsigned char)ch;
}

/*************************************************************
** 函数名称: COMM_putstr
** 功能描述: 发送一个字符串
** 入口参数: COM串口,str
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/ 
void COMM_putstr(COM_TypeDef COM,char* str)
{
    for(; *str; str++)
    {
        COMM_putc(COM,*str);
    }
}

/*************************************************************
** 函数名称: COMM_RxIsr
** 功能描述: 数据接收中断处理函数
** 入口参数: COM串口
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/ 
void COMM_RxIsr(COM_TypeDef COM)
{
    unsigned char tmp = 0;
    tmp = GetUsartReceiveData(COM);//取数据
    Insert_COMRXQueue(COM,tmp);
}
/*************************************************************
** 函数名称: COMM_TxIsr
** 功能描述: 发送中断处理函数
** 入口参数: COM串口
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/ 
void COMM_TxIsr(COM_TypeDef COM)
{
    if(!Empty_COMTXQueue(COM))    
    {
        UsartSendData(COM, Delete_COMTXQueue(COM));
    } 
    else 
    { //----- 数据发送完成 -----
        Comm_CtrlEnableRecv(COM);
        //////////////////
        s_stCommAttrib[COM].sendBusy = 0;
        SetUsartITConfig(COM,  USART_IT_TC, DISABLE);
    } 
}
/*************************************************************
** 函数名称: COMM_IRQHandler
** 功能描述: 串口中断
** 入口参数: COM:串口
** 出口参数: 无
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void COMM_IRQHandler(COM_TypeDef COM)
{
    if(GetUsartStatus(COM, USART_IT_RXNE) != RESET)
    {
        COMM_RxIsr(COM);
    }
    ////////////////////////////////////
    if(GetUsartStatus(COM, USART_IT_TC) != RESET)								          
    {
        COMM_TxIsr(COM);
    }
}
/*************************************************************
** 函数名称: COMM_SendData
** 功能描述: 发送数据
** 入口参数: COM:串口,buff数据首地址,len数据长度
** 出口参数: 
** 返回参数: 0:成功；1:失败,0xff:超出缓存长度
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned char COMM_SendData(COM_TypeDef COM,unsigned char *buff, unsigned short len)
{
    //检查串口是否正在发送数据
    if(Empty_COMTXQueue(COM))//没有发送数据
    {
        if(len > FreeSpace_COMTXQueue(COM))	
        {
            return 0xff;
        }
        else
        {
            Comm_CtrlEnableSend(COM);
            ////////////////////
            BatchInsert_COMTXQueue(COM,buff,len);
            s_stCommAttrib[COM].sendBusy = 1;
            SetUsartITConfig(COM, USART_IT_TC, ENABLE);
            return 0;
        }
    }
    else//正在发送数据
    {
        if(FreeSpace_COMTXQueue(COM)>len)
        {
            Comm_CtrlEnableSend(COM);
            BatchInsert_COMTXQueue(COM,buff,len);
            s_stCommAttrib[COM].sendBusy = 1;
            SetUsartITConfig(COM, USART_IT_TC, ENABLE);
            return 0;
        }
        else
        {
            return 1;
        }
    }
}
/*************************************************************
** 函数名称: COMM_ReadData
** 功能描述: 读取缓冲区里面的数据
** 入口参数: COM:串口,buff数据缓冲区,len读取长度
** 出口参数: 
** 返回参数: 有数据则返回读取的字节数,无则返回0
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned short COMM_ReadData(COM_TypeDef COM,unsigned char *buff,unsigned short len)
{
    return BatchDelete_COMRXQueue(COM,buff,len);
}
/*************************************************************
** 函数名称: COMM_GetSendBusy
** 功能描述: 获得当前串口发送忙标志
** 入口参数: 
** 出口参数: 无
** 返回参数: 1:串口忙,0串口空闲
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char COMM_GetSendBusy(COM_TypeDef COM)
{
    return (1==s_stCommAttrib[COM].sendBusy)?1:0;
}
/******************************************************************************
**                            End Of File
******************************************************************************/


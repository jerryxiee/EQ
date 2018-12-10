/**
  ******************************************************************************
  * @file    modem_uart.c
  * @author  miaoyahan 
  * @version V1.0.0
  * @date    2013-08-12
  * @brief   modem串口操作，包括初始化串口号以及初始化相应中断。
  ******************************************************************************
  */

/*
********************************************************************************
*                               INCLUDES
********************************************************************************
*/
#ifdef EYE_MODEM
#include "include.h"
#else
#include  <app_cfg.h>
#include  <includes.h>
#endif

/*
********************************************************************************
*                             LOCAL DEFINES
********************************************************************************
*/
#define MODEM_UART_ATSV_SIZE  (55)  //存放一条AT命令(受控缓存,不得修改!!!)

/*
********************************************************************************
*                          LOCAL VARIABLES
********************************************************************************
*/
static u8  Modem_Node_AtBuf[MODEM_UART_ATSV_SIZE];
static u8  Modem_Node_AtCnt;

static u16 Modem_Node_SocAll;//数据长度
static u16 Modem_Node_SocCnt;//数据计数器
static u16 Modem_Node_SocOut;//数据超时计数器 串口收到数据清零

static u16 Modem_Node_FtpDwn;//FTP下载标志 正在下载为1  空闲为0

static u8         Modem_Uart_RxBuf[MODEM_UART_RX_SIZE];//接收缓存
static MODLIB_BUF Modem_Uart_RxNdNum[MODEM_UART_NDRX_SIZE];//节点缓存
static MODLIB_NOD Modem_Uart_RxNode;//串口接收节点结构体

static u8         Modem_Uart_TxBuf[MODEM_UART_TX_SIZE];//发送缓存
static MODLIB_BUF Modem_Uart_TxNdNum[MODEM_UART_NDTX_SIZE];//节点缓存
static MODLIB_NOD Modem_Uart_TxNode;//串口发送节点结构体

static u8 Modem_Uart_TxBusy;//正在发送为1  空闲为0
/*
********************************************************************************
*                          GLOBAL VARIABLES
********************************************************************************
*/



/*
********************************************************************************
*                           FUNCTIONS EXTERN
********************************************************************************
*/


/*
********************************************************************************
*                              FUNCTIONS
********************************************************************************
*/
/*
  * @brief  数据接收计数器处理 
  * @param  data_end -> 当前结束标志状态 
  * @retval 返回结束标志结果
  */
u16  Modem_Uart_RecCnt(u16 data_end)
{
    u16 result = data_end;//默认无数据接收

    if(Modem_Type == MODEM_TYPE_Q)
    {
        if(Modem_Node_SocAll)
        {
            Modem_Node_SocCnt++;
            
            if(Modem_Node_SocAll == Modem_Node_SocCnt)
            {
                result = 0x0d0a;//收到完整数据
            }
            else
            {
                result = 0;//数据未收完
            }
        }
    }
    return result;
}

/*
  * @brief  数据接收指令处理 
  * @param  *pData : 指向AT命令地址
  *          len   : 当前接收数据的长度
  * @retval 标示未读标志 -> 0;
  *         返回继续存储 -> 1;
  */
u8  Modem_Uart_RecCmd(u8 *pData, u16 len)
{
    u8 *p = pData;
    u16 temp;
////////////////////////////////////////////////////////////////////////////////移远
    if(Modem_Type == MODEM_TYPE_Q)
    {
        if(Modem_Node_FtpDwn == 0)//非FTP下载中
        {
            if(ModLib_AtCmp(p,"+RECEIVE: ") == 0)//+RECEIVE: 0, 3
            {
                if(Modem_Node_SocAll == 0)
                {
                    p += 13;
                    Modem_Node_SocAll = atoi((char*)p);
                    Modem_Node_SocCnt = 0;
                    return 1;
                }
                Modem_Node_SocAll = 0;
                Modem_Node_SocCnt = 0;
            }
            ////////////////////////////////////////////////////////////////////
            else if(ModLib_AtCmp(p,"CONNECT") == 0)//开始下载文件
            {
                Modem_Node_FtpDwn = 1;
                Modem_Node_SocAll = 512;//512个字节一包
            }
        }
        else//FTP下载中
        {
            Modem_Node_SocCnt = 0;//清零计数器为下一包512个字节做准备
        }
    }
////////////////////////////////////////////////////////////////////////////////中兴
    else if(Modem_Type == MODEM_TYPE_ZTE)
    {
        if(ModLib_AtCmp(p,"+ZIPRECV:") == 0)//+ZIPRECV:0,4,1234
        {
            temp = ModLib_AtAddr(p,',')+1;
            p += temp;
            Modem_Node_SocAll = atoi((char*)p);//提取长度
            
            temp = ModLib_AtAddr(p,',') + temp + 1 + 2;//实际长度
            Modem_Node_SocCnt = len - temp;
            
            if((Modem_Node_SocCnt != Modem_Node_SocAll)
             &&(Modem_Node_SocCnt < Modem_Node_SocAll))
            {
                return 1;
            }
            Modem_Node_SocAll = 0;
            Modem_Node_SocCnt = 0;
        }
        else if(ModLib_AtCmp(p,"+ZFTPDATA:") == 0)//+ZFTPDATA:1024,1024\r\n +ZFTPDATA:CLOSED\r\n
        {
            if(ModLib_AtCmp(p,"+ZFTPDATA:CLOSED") == 0)
            {
                modftp.fend = 1;
            }
            else
            {
                temp = ModLib_AtAddr(p,',')+1;
                p += temp;
                Modem_Node_SocAll = atoi((char*)p);//提取长度
                p    = pData;
                temp = ModLib_AtLen(p)+2;
                if(len == temp)
                {
                    Modem_Node_SocCnt = 0;
                }
                else 
                {
                    Modem_Node_SocCnt = len - temp - 2;//实际长度
                }
                if((Modem_Node_SocCnt != Modem_Node_SocAll)
                 &&(Modem_Node_SocCnt < Modem_Node_SocAll))
                {
                    return 1;
                }
                Modem_Node_SocAll = 0;
                Modem_Node_SocCnt = 0;
            }
        }
    }
    
    return 0;
}

/*
  * @brief  节点复位
  * @param  None  
  * @retval None
  */
void  Modem_Uart_NodRst(void)
{
    memset((u8*)Modem_Node_AtBuf,0,sizeof(Modem_Node_AtBuf));
    Modem_Node_AtCnt  = 0;
    Modem_Node_SocAll = 0;
    Modem_Node_SocCnt = 0;
    Modem_Node_SocOut = 0;
    Modem_Node_FtpDwn = 0; 
    ModLib_NodRst(&Modem_Uart_RxNode); 
    ModLib_NodRst(&Modem_Uart_TxNode); 
}

/*
  * @brief  当前节点数
  * @param  None
  * @retval 返回节点数
  */
u8 Modem_Uart_NodNum(void)
{
    u8 len;
    
    len = ModLib_NodNum(&Modem_Uart_RxNode);//查询是否有命令需要处理

    return len;
}

/*
  * @brief  清除剩余节点
  * @param  None
  * @retval None
  */
void Modem_Uart_NodClr(void)
{
    if(ModLib_NodNum(&Modem_Uart_RxNode))
    {
        ModLib_NodRst(&Modem_Uart_RxNode);
    }
}
/*
  * @brief  从节点中取出数据
  * @param  None
  * @retval 返回数据长度
  */
u16 Modem_Uart_NodRet(u8 *pData)
{
    u16 len;

    len = ModLib_NodRet(&Modem_Uart_RxNode,pData);//获取数据
    if(len)
    {
        len -= 1;//截取1字节结束符
    }
    return len;
}
/*
  * @brief  处理节点内的数据
  * @param  ch:要插入节点的字节
  * @retval 返回接收到的字节
  */
u8 Modem_Uart_NodIns(u8 ch)
{
    static u16 AtEnd;//命令结束符 \r\n
    
    ////////////////////////////////////////////////////////////////////插入数据
    if(ModLib_NodIns(&Modem_Uart_RxNode,INS_HALF,&ch,1) == 0)
    {
        return 0;
    }
    ////////////////////////////////////////////////////////////////\r\n结束处理
    AtEnd = AtEnd <<8;
    AtEnd |= ch;

    //////////////////////////////////////////////////////////////保存一条AT命令
    if(Modem_Node_AtCnt < MODEM_UART_ATSV_SIZE)
    {
        Modem_Node_AtBuf[Modem_Node_AtCnt++] = ch;
    }

    //////////////////////////////////////////////////////////////数据计数器处理
    AtEnd = Modem_Uart_RecCnt(AtEnd);

    /////////////////////////////////////////////////////////检查0X0D,0X0A结束符
    if(AtEnd == 0x0d0a)
    {
        if(Modem_Uart_RecCmd(Modem_Node_AtBuf,ModLib_NodLen(&Modem_Uart_RxNode)))
        {
            return ch;
        }
        ch = 0x24;//结束符
        ModLib_NodIns(&Modem_Uart_RxNode,INS_ALL,&ch,1);//节点打包
        //////////////////////////////////////////////////////////清空AT命令缓存
        Modem_Node_AtCnt = 0;
        memset(Modem_Node_AtBuf,0,MODEM_UART_ATSV_SIZE);
    }
    return ch;
}

/*
  * @brief  节点节点超时处理
  * @param  None  
  * @retval None
  */
void  Modem_Uart_NodOut(void)
{
    u16 outtime = MODEM_TSEC*30;

    Modem_Node_SocOut++;
    
    if(Modem_Node_SocOut < outtime)
    {
        return;
    }
    Modem_Node_SocOut = 0;

    if(Modem_Node_SocAll)
    {
        Modem_Node_SocAll = Modem_Node_SocCnt+1;
        Modem_Uart_NodIns(0x55);
        
        if(Modem_Node_FtpDwn)
        {
            Modem_Node_FtpDwn = 0;//清零下载标志
            Modem_Node_SocAll = 0;
            Modem_Node_SocCnt = 0;
            modftp.fend = 1;//下载结束
        }
    }
}
    
/*
  * @brief  Modem硬件驱动初始化,包括串口.
  * @param  None
  * @retval None
  */
void  Modem_Uart_Init (void)
{  
    NVIC_InitTypeDef NVIC_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure; 

      /* Enable GPIO clock */
    RCC_AHB1PeriphClockCmd(Modem_UART_TX_GPIO_CLK | Modem_UART_RX_GPIO_CLK, ENABLE);

    /* Enable UART clock */
    RCC_APB1PeriphClockCmd(Modem_UART_CLK, ENABLE);
    
    USART_InitStructure.USART_BaudRate = Modem_BaudRate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    
    /* Connect PXx to USARTx_Tx*/
    GPIO_PinAFConfig(Modem_UART_TX_GPIO_PORT, Modem_UART_TX_SOURCE, Modem_UART_TX_AF);

    /* Connect PXx to USARTx_Rx*/
    GPIO_PinAFConfig(Modem_UART_RX_GPIO_PORT, Modem_UART_RX_SOURCE, Modem_UART_RX_AF);

    /* Configure USART Tx as alternate function  */
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;

    GPIO_InitStructure.GPIO_Pin = Modem_UART_TX_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(Modem_UART_TX_GPIO_PORT, &GPIO_InitStructure);

    /* Configure USART Rx as alternate function  */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Pin = Modem_UART_RX_PIN;
    GPIO_Init(Modem_UART_RX_GPIO_PORT, &GPIO_InitStructure);

    /* USART configuration */
    USART_Init(Modem_UART, &USART_InitStructure);

    /* Enable USART */
    USART_Cmd(Modem_UART, ENABLE);
    
    /* configure Modem_UART IT and Enable RxNE IT*/
    USART_ITConfig(Modem_UART, USART_IT_RXNE, ENABLE);

    /* Enable the Modem_UART Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = Modem_UART_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    ModLib_NodInit(&Modem_Uart_RxNode,Modem_Uart_RxNdNum,MODEM_UART_NDRX_SIZE,
                                    Modem_Uart_RxBuf,MODEM_UART_RX_SIZE);
                                    
    ModLib_NodInit(&Modem_Uart_TxNode,Modem_Uart_TxNdNum,MODEM_UART_NDTX_SIZE,
                                    Modem_Uart_TxBuf,MODEM_UART_TX_SIZE);     
}
/*
  * @brief  向串口发送指定长度数据
  * @param  None
  * @retval None
  */
u16 Modem_Uart_Send(u8* data,  u16 len)
{
    u16 SendLen;

    SendLen = ModLib_NodIns(&Modem_Uart_TxNode,INS_ALL,data,len);
    if(Modem_Uart_TxBusy == 0)
    {
        Modem_Uart_TxBusy = 1;
        USART_ITConfig(Modem_UART, USART_IT_TXE, ENABLE);
    }
  return SendLen;
}
/*
  * @brief  串口中断处理
  * @param  None
  * @retval None
  */
void Modem_USART_IRQHandler (void)
{
    u8 byte;
    u8 insfla = 0;
    ////////////////////////////////////////////////////////////////////////发送
    if (USART_GetITStatus(Modem_UART, USART_IT_TXE) == SET)
    {
        if(ModLib_NodRetEx(&Modem_Uart_TxNode,&byte,1))
        {
            USART_SendData(Modem_UART, byte);
            Modem_Node_SocOut = 0;
            #ifdef MODEM_DEBUG_H
            if((ModDbInfType == MODDB_CON)||(ModDbInfType == MODDB_ALL))
            {
                Modem_Debug_UartTx(&byte,1);
            }
            #endif            
        }
        else
        {
            USART_ITConfig(Modem_UART, USART_IT_TXE, DISABLE);
            Modem_Uart_TxBusy = 0;
        }
    }
    ////////////////////////////////////////////////////////////////////////接收    
    if(USART_GetITStatus(Modem_UART, USART_IT_RXNE) == SET)
    {
        insfla = 1;
    }
    ////////////////////////////////////////////////////////////////////////溢出        
    if(USART_GetFlagStatus(Modem_UART, USART_FLAG_ORE) != RESET)
	{
        insfla = 1;//进入影子,需取出数据防止死机
	}
    ////////////////////////////////////////////////////////////////////////插入
	if(insfla)
	{
        byte = USART_ReceiveData(Modem_UART);
        Modem_Uart_NodIns(byte);
        Modem_Node_SocOut = 0;
        #ifdef MODEM_DEBUG_H
        if((ModDbInfType == MODDB_CON)||(ModDbInfType == MODDB_ALL))
        {
            Modem_Debug_UartTx(&byte,1);
        }
        #endif  
	}
}



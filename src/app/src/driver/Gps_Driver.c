/******************************************************************** 
//版权说明	:Shenzhen E-EYE Co. Ltd., Copyright 2009- All Rights Reserved.
//文件名称	:Gps_Driver.c		
//功能		:GPS模块驱动
//版本号	:
//开发人	:dxl
//开发时间	:2011.8
//修改者	:
//修改时间	:
//修改简要说明	:
//备注		:1)GPS串口与TTS公用串口，所以要想发送有数据，必须置位TTS_P.
***********************************************************************/

//****************文件包含**************************
#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "include.h"

//****************宏定义********************

//***************变量定义********************
static stUartSend	Tx;
static u8  	GpsRxBuffer[GPS_RX_BUFFER_SIZE];//接收缓冲
static u8	GpsRxCount = 0;//接收计数
static u8	GpsRxFrameCount = 0;//接收的帧计数
u8	GprmcBuffer[GPRMC_BUFFER_SIZE];//存放GPRMC数据
u8	GprmcBufferBusyFlag = 0;//GprmcBuffer缓冲忙标志
u8	GpggaBuffer[GPGGA_BUFFER_SIZE];//存放GPGGA数据
u8	GpggaBufferBusyFlag = 0;//GpggaBuffer缓冲忙标志
u8	GpgsvBuffer[GPGSV_BUFFER_SIZE];//存放GPGGA数据
u8	GpgsvBufferBusyFlag = 0;//GpggaBuffer缓冲忙标志

u8 GPSSendBuff[GPS_SEND_BUFFER_SIZE];
extern u8 Recorder_SendData(u8 *pData, u16 Len);
extern u8	BlindReportFlag;//盲区上报标志，1为要求上报或正上报，0为不要求
//****************函数声明*******************

/*********************************************************************
//函数名称	:Gps_Init(void)
//功能		:GPS初始化
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void Gps_Init(void)
{
    USART_InitTypeDef USART_InitStructure;
    
    memset(&USART_InitStructure,0,sizeof(USART_InitTypeDef));
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    
    COM_Init(COM4, &USART_InitStructure);  
    COM_NVIC_Config(COM4);                               //使能串口中断
    
    SetUsartITConfig(COM4,USART_IT_RXNE,ENABLE);        //使能串口接收中断
    SetUsartITConfig(COM4,USART_IT_PE,DISABLE);        //不使能
    SetUsartITConfig(COM4,USART_IT_TXE,DISABLE);        //不使能
    SetUsartITConfig(COM4,USART_IT_TC,DISABLE);        //不使能
    SetUsartITConfig(COM4,USART_IT_IDLE,DISABLE);        //不使能
    SetUsartITConfig(COM4,USART_IT_LBD,DISABLE);        //不使能
    SetUsartITConfig(COM4,USART_IT_ERR,DISABLE);        //不使能
    
    GpioOutInit(GPS_CTR);                               //初始GPS电源控制脚
	
    GpioOutOff(GPS_CTR);                                //初始关闭GPS电源
      GpioOutOn(GPS_CTR);                                //初始开启GPS电源
    
    //配置发送缓冲区
    Tx.pSendBuffer = GPSSendBuff;
    Tx.SendBufferMaxLen = GPS_SEND_BUFFER_SIZE;
}
/*********************************************************************
//函数名称	:GPS_RxIsr(void)
//功能		:GPS接收中断
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void Gps_RxIsr(void)
{
	u8	temp;
	if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET)//GPS模块与CPU通信是串口4,接收寄存器是否有数据
	{
		if(GpsRxCount >= GPS_RX_BUFFER_SIZE)//防止写入到其它存储区域
		{
			GpsRxCount = 0;
		}
		temp = USART_ReceiveData(UART4);//取数据
		//----GNSS---
			Gnss_CommRxIsr(temp);//modify by joneming
		//-------------------------------	
#ifdef HUOYUN_DEBUG_OPEN
        //Recorder_SendData(&temp, 1);
#endif
		if('$' == temp)//起始符
		{
			GpsRxCount = 0;
			GpsRxBuffer[GpsRxCount] = temp; 
			GpsRxCount++;

				
		}
		else if(0x0a == temp)//结尾符
		{
			GpsRxBuffer[GpsRxCount] = temp; 
			GpsRxCount++;
                        //if(0 == BlindReportFlag)//当盲区补报时不发送定位信息,行标检测时使用，以便采集模拟场景的数据
                        //{
                                  //Recorder_SendData(GpsRxBuffer, GpsRxCount);
                        //}
			if((0 == strncmp("$GPRMC",(char const *)GpsRxBuffer,6))//GPS格式
			   ||(0 == strncmp("$GNRMC",(char const *)GpsRxBuffer,6))//GPS+BD格式
			   ||(0 == strncmp("$BDRMC",(char const *)GpsRxBuffer,6)))//BD格式
			{
				if(0 == GprmcBufferBusyFlag)
				{
					 if('P' == GpsRxBuffer[2])
			       		{
			       			Io_WriteStatusBit(STATUS_BIT_GPS, SET);
						Io_WriteStatusBit(STATUS_BIT_COMPASS, RESET);
			       		}
			       		else if('N' == GpsRxBuffer[2])
			       		{
			       			Io_WriteStatusBit(STATUS_BIT_GPS, SET);
						Io_WriteStatusBit(STATUS_BIT_COMPASS, SET);
						
			       		}
			       		else if('D' == GpsRxBuffer[2])
			       		{
			       			Io_WriteStatusBit(STATUS_BIT_GPS, RESET);
						Io_WriteStatusBit(STATUS_BIT_COMPASS, SET);
			       		}
					Io_WriteStatusBit(STATUS_BIT_GLONASS, RESET);
					Io_WriteStatusBit( STATUS_BIT_GALILEO, RESET);
					//拷贝数据
					memcpy(GprmcBuffer,GpsRxBuffer,GpsRxCount);
					//帧计数加1
					GpsRxFrameCount++;
			                //触发解析任务
			                //SetEvTask(EV_GPS_PARSE); //行标送检增加,dxl,2013.6.18
				}
			}
			else if((0 == strncmp("$GPGGA",(char const *)GpsRxBuffer,6))//GPS格式
				||(0 == strncmp("$GNGGA",(char const *)GpsRxBuffer,6))//GPS+BD格式
				||(0 == strncmp("$BDGGA",(char const *)GpsRxBuffer,6)))//BD格式
			{
				if(0 == GpggaBufferBusyFlag)
				{
					//拷贝数据
					memcpy(GpggaBuffer,GpsRxBuffer,GpsRxCount);
					//帧计数加1
					GpsRxFrameCount++;
				}
			}	
			else if((0 == strncmp("$GPGSV",(char const *)GpsRxBuffer,6))//GPS格式
				||(0 == strncmp("$GNGSV",(char const *)GpsRxBuffer,6))//GPS+BD格式
				||(0 == strncmp("$BDGSV",(char const *)GpsRxBuffer,6)))//BD格式
			{
				if(0 == GpgsvBufferBusyFlag)
				{
					//拷贝数据
					memcpy(GpgsvBuffer,GpsRxBuffer,GpsRxCount);
				}
			}
			//----增加GNSS TXT 文档---
					else if((0 == strncmp("$GPTXT",(char const *)GpsRxBuffer,6))//GPS格式
						||(0 == strncmp("$GNTXT",(char const *)GpsRxBuffer,6))//GPS+BD格式
						||(0 == strncmp("$BDTXT",(char const *)GpsRxBuffer,6)))//BD格式
						{
								//	g_GnssGroup.end=true;
						}	
			//----------------------------
			else if(0 == strncmp("$CFGSYS",(char const *)GpsRxBuffer,7))//GPS格式
			{
				if(temp == 0)
				{
					temp = 0;
				}
			}
		}
		else//中间字符
		{
			GpsRxBuffer[GpsRxCount] = temp; 
			GpsRxCount++;
		}

		if(GpsRxFrameCount >= 2)
		{
			//帧计数清0
			GpsRxFrameCount = 0;
			//触发解析任务
			SetEvTask(EV_GPS_PARSE); 
		}		
	}
	
}
/*********************************************************************
//函数名称	:GPS_TxIsr(void)
//功能		:GPS发送中断
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void Gps_TxIsr(void)
{
	u8	ch;

	if(Tx.SendCount < Tx.SendLen)//发送数据
	{
		
		if(Tx.SendCount < Tx.SendBufferMaxLen)
		{
			ch = Tx.pSendBuffer[Tx.SendCount];
			UsartSendData(COM4, ch);
			Tx.SendCount++;
		}
		else
		{
			Tx.SendCount = 0;
			Tx.SendLen = 0;
			SetUsartITConfig(COM4, USART_IT_TXE, DISABLE);
		}
	}
	else//计数清0，关闭串口
	{
		Tx.SendCount = 0;
		Tx.SendLen = 0;
		SetUsartITConfig(COM4,  USART_IT_TXE, DISABLE);
	}	
}
/*********************************************************************
//函数名称	:GPS_ReadGPRMC(u8 *pBuffer)
//功能		:读GPRMC原始数据
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
u8 Gps_ReadGPRMC(u8 *pBuffer)
{
	u8	i;
	u8	temp;
	
	for(i=0; i<GPRMC_BUFFER_SIZE; i++)
	{
		 temp = GprmcBuffer[i];
		 *(pBuffer+i) = temp;
		if(0x0a == temp)
		{
			return (i+1);
		}
	}
	return 100;
}
/*********************************************************************
//函数名称	:GPS_ReadGPGSV(u8 *pBuffer)
//功能		:读GPGSV原始数据
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
u8 Gps_ReadGPGSV(u8 *pBuffer)
{
	u8	i;
	u8	temp;
	
	for(i=0; i<GPGSV_BUFFER_SIZE; i++)
	{
		temp = GpgsvBuffer[i];
		*(pBuffer+i) = temp;
		if(0x0a == temp)
		{
			return (i+1);
		}
	}
	return 100;
}
/*********************************************************************
//函数名称	:GPS_SendData
//功能		:GPS发送数据
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:成功返回ACK_OK；失败返回ACK_ERROR
//备注		:
*********************************************************************/
ProtocolACK  Gps_SendData(u8 *pBuffer, u16 BufferLen)
{
	//检查串口是否正在发送数据
	if(0 == Tx.SendCount)//没有发送数据
	{
		if(BufferLen > Tx.SendBufferMaxLen)	
		{
			return ACK_ERROR;
		}
		else
		{
			memcpy(Tx.pSendBuffer,pBuffer,BufferLen);
			Tx.SendLen = BufferLen;
			SetUsartITConfig(COM4, USART_IT_TXE, ENABLE);
			return ACK_OK;
		}
	}
	else//正在发送数据
	{
		return ACK_ERROR;
	}
}
/*********************************************************************
//函数名称	:GPS_putc
//功能		:发送一个字符
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void GPS_putc(int ch)
{
  while((EVAL_COM4->SR & USART_FLAG_TC) == RESET);
  EVAL_COM4->DR = (u8)ch;
}

/*********************************************************************
//函数名称	:GPS_putstr
//功能		:发送一个字符串
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void GPS_putstr(char* str)
{
    for(; *str; str++)
    {
        GPS_putc(*str);
    }
}

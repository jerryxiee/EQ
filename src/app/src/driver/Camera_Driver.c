/******************************************************************** 
//版权说明	:Shenzhen E-EYE Co. Ltd., Copyright 2009- All Rights Reserved.
//文件名称	:Camera_Driver.c		
//功能		:摄像头拍照驱动
//版本号	:
//开发人	:dxl
//开发时间	:2012.3
//修改者	:
//修改时间	:
//修改简要说明	:
//备注		:摄像头与主机接口为485总线，用一个接口还共用了其他设备
***********************************************************************/
//***************包含文件*****************
#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "include.h"

//***************外部变量******************
extern u8 CameraStateAck[];//状态标志，0为初始值，1为应答正确，2为无应答，3为超时
extern u8 CameraDataBuffer[];//图像数据缓冲，存储的是摄像头图片数据，不包含摄像头协议的帧头，帧尾等内容
extern u16	CameraRxBufferLen;//摄像头接收缓冲数据长度
extern u16 CameraDataBufferLen;//图像数据长度
extern u8 CameraCmdAckBuffer[];//摄像头命令应答缓冲，存储摄像头应答指令，但不包括取图的应答，包含帧头、帧尾内容
extern u8 CameraCmdAckBufferLen;//命令应答长度
//extern u8 CameraDataBufferBusyFlag;//忙标志，1忙，0空闲
extern u8  Uart1DataDownTransFlag;//串口1数据透传标志，
extern u16 Uart1DataDownTransCmd;//串口1数据透传数据中的命令字
extern u8  DelayBuffer[100];//延时缓冲，可把要延时发送的数据拷贝到这里，注意最大长度限制
extern u8  DelayBufferLen;//延时缓冲数据长度
extern u16     DelayAckCommand;//延时应答命令
//***************本地变量******************
u8 	Usart3RxBuffer[USART3_RX_BUFFER_SIZE];//接收缓冲
static UART_SEND_STRUCT	Tx;//发送结构体
u8	Usart3BusyFlag = 0;//串口1忙标志，1为忙，0为空闲；忙时不能往外发送数据
static u32 	Usart3RxIdleCount = 0;
u16	Usart3RxCount = 0;
//****************全局变量*****************

//****************函数声明****************
/*********************************************************************
//函数名称	:Camera_Init
//功能		:摄像头硬件初始化
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void Camera_Init(void)
{
	GPIO_InitTypeDef 			GPIO_InitStructure;
#if 0
	COM3_Init(115200);                           //摄像头用串口3
        GpioOutInit(CAM_POWER);
        GpioOutInit(CAM_RD);
#else

	Usart3_Init();
        
#endif
        
	//*********摄像头电源控制引脚*********
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;       
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOF, &GPIO_InitStructure);
	//*************接收发送控制引脚**************
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;              
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOD, GPIO_Pin_10);//PD10为485读写控制线，默认情况下为接收
}
/*********************************************************************
//函数名称	:Camera_ClrRxBuffer
//功能		:串口6接收缓存清0
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void Usart3_ClrRxBuffer(void)
{
	u16	i;
	for(i=0; i<USART3_RX_BUFFER_SIZE; i++)
	{
		Usart3RxBuffer[i] = 0;
	}
}
/*********************************************************************
//函数名称	:Camera_Init
//功能		:串口6初始化
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void Usart3_Init(void)
{
  
#if 0
  
  	GPIO_InitTypeDef    GPIO_InitStructure;
  	USART_InitTypeDef   USART_InitStructure;
	NVIC_InitTypeDef 	NVIC_InitStructure;
  
  	/* Enable the USARTx Interrupt */
  	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  	NVIC_Init(&NVIC_InitStructure);

  	/* Enable GPIO clock */
  	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

    	/* Enable UART clock */
    	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

  	/* Connect PXx to USARTx_Tx*/
  	GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_USART3);

  	/* Connect PXx to USARTx_Rx*/
  	GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_USART3);

  	/* Configure USART Tx as alternate function  */
  	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;

  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(GPIOD, &GPIO_InitStructure);

  	/* Configure USART Rx as alternate function  */
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  	GPIO_Init(GPIOD, &GPIO_InitStructure);
  	//**********串口3初始化****************
  	USART_InitStructure.USART_BaudRate = 115200;               	//通信波特路115200
  	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  	USART_InitStructure.USART_StopBits = USART_StopBits_1;         //1位停止位
  	USART_InitStructure.USART_Parity = USART_Parity_No;//当使用电脑串口时,与字长度8位配套使用 
  	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  	USART_Init(USART3, &USART_InitStructure);         //配置串口3
        
        USART_Cmd(USART3, ENABLE);                        //使能USART3 
        
  	//使能接收中断
  	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);    //使能接收中断
        
#else
        
	GPIO_InitTypeDef 	GPIO_InitStructure;
	USART_InitTypeDef	USART_InitStructure;
	NVIC_InitTypeDef 	NVIC_InitStructure;
	
	/* Enable the USARTx Interrupt */
  	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  	NVIC_Init(&NVIC_InitStructure);

  	/* Enable GPIO clock */
  	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

    	/* Enable UART clock */
    	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

  	/* Connect PXx to USARTx_Tx*/
  	GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_USART3);

  	/* Connect PXx to USARTx_Rx*/
  	GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_USART3);

  	/* Configure USART Tx as alternate function  */
  	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;

  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(GPIOD, &GPIO_InitStructure);

  	/* Configure USART Rx as alternate function  */
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  	GPIO_Init(GPIOD, &GPIO_InitStructure);
  
//*************************UART3参数设置*****************************
	USART_InitStructure.USART_BaudRate = 115200;               //摄像头波特率为115200
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;   //8位数据位
	USART_InitStructure.USART_StopBits = USART_StopBits_1;        //1位停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;           //无校验
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART3, &USART_InitStructure);         //配置串口3

	USART_Cmd(USART3, ENABLE);                        //使能USART3
	
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);    //使能发送空中断和接收空中断        
        
#endif        
  	
}
/*********************************************************************
//函数名称	:Usart3_Isr
//功能		:串口3中断服务程序
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void Usart3_Isr(void)
{
  
 	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)//接收中断
	{
		Usart3_RxIsr();
	} 
  
	if(USART_GetITStatus(USART3, USART_IT_TXE) != RESET)//发送空中断
	//if(USART_GetITStatus(USART3, USART_IT_TC) != RESET)//发送完成中断
	{
		Usart3_TxIsr();
	}
        

	
}
/*********************************************************************
//函数名称	:Usart3_RxIsr
//功能		:串口3接收中断
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void Usart3_RxIsr(void)
{
    u8 ch;
    
	Usart3RxIdleCount = 0;
    
	ch = USART_ReceiveData(USART3);
	Usart3RxBuffer[Usart3RxCount] = ch;
	Usart3RxCount++;
	if(Usart3RxCount >= USART3_RX_BUFFER_SIZE)
	{
		Usart3RxCount = 0;
	}
}
/*********************************************************************
//函数名称	:Usart3_TxIsr
//功能		:串口6发送中断
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void Usart3_TxIsr(void)
{
	u8	ch;
	u32	i;
	if(Tx.SendCount < Tx.SendLen)//发送数据
	{
		
		if(Tx.SendCount < USART3_TX_BUFFER_SIZE)
		{
			ch = Tx.SendBuffer[Tx.SendCount];
			USART_SendData(USART3, ch);
			Tx.SendCount++;
		}
		else
		{
			Tx.SendCount = 0;
			Tx.SendLen = 0;
            USART_ITConfig(USART3, USART_IT_TXE, DISABLE);//发送空中断方案
			CAMERA_485RX_EN();//默认情况下485为接收
            for(i=0; i<100; i++)
            USART_ITConfig(USART3, USART_IT_RXNE, ENABLE); //使能接收中断
            //USART_ITConfig(USART3, USART_IT_TC, DISABLE);//发送完成中断方案
		}
	}
	else//计数清0，关闭串口
	{
		Tx.SendCount = 0;
		Tx.SendLen = 0;
        USART_ITConfig(USART3, USART_IT_TXE, DISABLE);//发送空中断方案
		CAMERA_485RX_EN();//默认情况下485为接收
        for(i=0; i<100; i++)
        ///USART_ITConfig(USART3, USART_IT_TC, DISABLE);//发送完成中断方案
        USART_ITConfig(USART3, USART_IT_RXNE, ENABLE); //使能接收中断
	}	
}
/*********************************************************************
//函数名称	:Usart3_SendData
//功能		:串口6发送数据
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:成功返回ACK_OK；失败返回ACK_ERROR
//备注		:
*********************************************************************/
ProtocolACK  Usart3_SendData(u8 *pBuffer, u16 BufferLen)
{
	u32 i;
	//检查串口是否正在发送数据
	if(0 == Tx.SendCount)//没有发送数据
	{
		if(BufferLen > USART3_TX_BUFFER_SIZE)	
		{
			return ACK_ERROR;
		}
		else
		{
			if(0 == Usart3BusyFlag)
			{
				Usart3BusyFlag = 1;//置位忙标志
				Usart3RxIdleCount = 0;
				memcpy(Tx.SendBuffer,pBuffer,BufferLen);
				Tx.SendLen = BufferLen;
                Tx.SendBuffer[BufferLen] = 0;//在最后一个字节增加一个0，保障最后一个字节能发送出去,dxl,2014.3.17
                Tx.SendLen++;
				CAMERA_485TX_EN();//设置485为发送
				for(i=0; i<100; i++)//延时，主要是为了稳定485控制电平
				{

				}
                USART_ITConfig(USART3, USART_IT_RXNE, DISABLE); //发送过程中禁止接收（因为485是半双工通信）
				USART_ITConfig(USART3, USART_IT_TXE, ENABLE);//发送空中断方案
				//USART_SendData(USART3,0xff);正常使用
                                //USART_SendData(USART3,0x00);
				//USART_ITConfig(USART3, USART_IT_TC, ENABLE);
                
				return ACK_OK;
			}
			else
			{
				return ACK_ERROR;
			}
		}
	}
	else//正在发送数据
	{
		return ACK_ERROR;
	}
}
/*********************************************************************
//函数名称	:Usart3_TimeTask
//功能		:串口3解析任务
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:100毫秒调度1次
*********************************************************************/
FunctionalState Usart3_TimeTask(void)
{
	u16 i;
	if(1 == Usart3BusyFlag)
	{
		Usart3RxIdleCount++;
        if(1 == Usart3_CameraProtocolParse())
        {
            Usart3RxIdleCount = 0;
            Usart3BusyFlag = 0;
			Usart3RxCount = 0;//接收计数清0
            for(i=0; i<USART3_RX_BUFFER_SIZE; i++)
            {
                Usart3RxBuffer[i] = 0;
            }
        }
        else
        {
		    if(Usart3RxIdleCount >= 5)//500ms内无应答强制清除忙标志
		    {
			    Usart3BusyFlag = 0;
			    Usart3RxCount = 0;//接收计数清0
                for(i=0; i<USART3_RX_BUFFER_SIZE; i++)
                {
                    Usart3RxBuffer[i] = 0;
                }
		    }
        }
	}
	else
	{
		Usart3RxIdleCount = 0;
	}
	return ENABLE;
}
/*********************************************************************
//函数名称	:Usart3_OilProtocolParse
//功能		:久通油耗协议解析
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
/*,dxl,2014.5.12屏蔽,油耗由32PIN 485接口改到16PIN 232接口
u8 Usart3_OilProtocolParse(void)
{
    u16 i;
    u16 j;
    u8  flag = 0;
    u8  HighByte;
    u8  LowByte;
    u8  sum;
    
    for(i=0; i<Usart3RxCount; i++)
	{
		if('@' == Usart3RxBuffer[i])
		{
				break;
		}
	}
    for(j=i; j<Usart3RxCount; j++)
	{
        if('#' == Usart3RxBuffer[j])
		{
            flag = 1;
		    break;
		}
	}
	if(1 == flag)
    {
        sum = 0;
        for(j=i+1;j<Usart3RxCount-3;j++)
        {
            sum += Usart3RxBuffer[j];
        }
        
        HighByte = Usart3RxBuffer[Usart3RxCount-3];
        if((HighByte >= '0')&&(HighByte <= '9'))
        {
            HighByte -= '0';
        }
        else if((HighByte >= 'A')&&(HighByte <= 'F'))
        {
            HighByte -= 55;
        }
        else if((HighByte >= 'a')&&(HighByte <= 'f'))
        {
            HighByte -= 87;
        }
        else
        {
            flag = 0;//非法字符
        }
            
        LowByte = Usart3RxBuffer[Usart3RxCount-2];
        if((LowByte >= '0')&&(LowByte <= '9'))
        {
            LowByte -= '0';
        }
        else if((LowByte >= 'A')&&(LowByte <= 'F'))
        {
            LowByte -= 55;
        }
        else if((LowByte >= 'a')&&(LowByte <= 'f'))
        {
            LowByte -= 87;
        }
        else
        {
            flag = 0;//非法字符
        }
            
        if((sum == ((HighByte<<4)|LowByte))&&(1 == flag))
        {
            //发送到平台
            if((Uart1DataDownTransCmd == *(Usart3RxBuffer+i+3))&&(1 == Uart1DataDownTransFlag))
            {
                Uart1DataDownTransFlag = 0;
                DelayBuffer[0] = 1;
                memcpy(DelayBuffer+1,Usart3RxBuffer+i,Usart3RxCount-i);
                DelayBufferLen = Usart3RxCount-i+1;
                DelayAckCommand = 13;//下一步把应答发送到平台
                SetTimerTask(TIME_DELAY_TRIG, 1*SECOND);//触发延时任务
            }
            JointechOiLCost_CopyBuf(Usart3RxBuffer+i,Usart3RxCount-i); 
            
        }
        else
        {
            flag = 0;
        }
    }
    return flag;
	
}
*/
/*********************************************************************
//函数名称	:Usart3_CameraProtocolParse
//功能		:摄像头协议解析
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:返回0表示不是摄像头协议，返回1表示是摄像头协议
//备注		:
*********************************************************************/
u8 Usart3_CameraProtocolParse(void)
{
	u16 i;
    u16 j;
    u16 length;
    u8  flag = 0;
    u8  sum;
	//解析数据
    for(i=0; i<Usart3RxCount; i++)
    {
        if((0x40 == Usart3RxBuffer[i])&&(0x40 == Usart3RxBuffer[i+1]))
        {
             break;
        }
    }
    if(Usart3RxCount != i)//找到摄像头指令的帧头
    {
         for(j=i; j<Usart3RxCount; j++)
         {
             if((0x0d == Usart3RxBuffer[j])&&(0x0a == Usart3RxBuffer[j+1]))
             {
                  j++;
                  flag = 1;//确认为摄像头指令
                  break;
              }
         }
     }
     if(1 == flag)//摄像头指令
     {
		if(0x64 == Usart3RxBuffer[i+2])//拍照应答
		{
			CameraCmdAckBufferLen = Usart3RxCount-i;
			memcpy(CameraCmdAckBuffer,Usart3RxBuffer+i,CameraCmdAckBufferLen);
            CameraStateAck[CAMERA_STATE_PHOTO] = 1;
		}
        else if(0x6a == Usart3RxBuffer[i+2])//测试指令
        {
            CameraCmdAckBufferLen = Usart3RxCount-i;
			memcpy(CameraCmdAckBuffer,Usart3RxBuffer+i,CameraCmdAckBufferLen);
            CameraStateAck[CAMERA_STATE_CHECK] = 1;
        }
		else if(0x63 == Usart3RxBuffer[i+2])//取图应答
		{
			//检查长度
			length = 0;
			length |= Usart3RxBuffer[i+6] << 8;
			length |= Usart3RxBuffer[i+5];
			if(length == Usart3RxCount-i-11)//长度正确
			{
                sum = 0;
                for(j=0; j<length; j++)
                {
                    sum += Usart3RxBuffer[i+8+j];
                }
                if(sum == Usart3RxBuffer[i+8+j])
                {
                    //if(0 == CameraDataBufferBusyFlag)
                    //{
				        CameraDataBufferLen = length;
				        memcpy(CameraDataBuffer,Usart3RxBuffer+i+8,CameraDataBufferLen);//只取图像数据
                        CameraStateAck[CAMERA_STATE_FETCH] = 1;
                    //}
                    //else
                    //{
                       // CameraStateAck[CAMERA_STATE_FETCH] = 2;
                       // flag = 0;
                   // }
                }
                else
                {
                    CameraStateAck[CAMERA_STATE_FETCH] = 2;
                    flag = 0;
                }
			}
            else
            {
                CameraStateAck[CAMERA_STATE_FETCH] = 2;
                flag = 0;
            }
		}
    }
    return flag;
}
/******************************************************************** 
//版权说明	:Shenzhen E-EYE Co. Ltd., Copyright 2009- All Rights Reserved.
//文件名称	:fm25c160.c
//功能		:铁电驱动
//版本号	:V0.1
//开发人	:dxl
//开发时间	:2010.03
//修改者	:
//修改时间	:
//修改简要说明	:
//备注		:
***********************************************************************/

//********************************头文件************************************
#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "include.h"

//********************************自定义数据类型****************************

//********************************宏定义************************************
//命令字节
#define	FRAM_WREN	0x06	//Write Enabled
#define FRAM_WRDI	0x04	//Write Disabled
#define FRAM_RDSR	0x05	//Read Status Register
#define FRAM_WRSR	0x01	//Write Status Register
#define FRAM_READ	0x03	//Read Data from Memory
#define FRAM_WRITE	0x02	//Write Data to Memory

#define FRAM_WEL_BIT	0x02	//bit1
#define FRAM_WPEN_BIT	0x80	//bit7
#define FRAM_BP1BP0_BIT	0x0C	//bit3,bit2


//********************************全局变量**********************************
//****************本文件没有用到任何全局变量**************

//********************************外部变量**********************************
#ifdef FRAM_SINGLE_TEST
extern u8 UART4_TxBuffer[];
#endif
//********************************本地变量**********************************
//****************本文件没有用到任何本地变量**************

//********************************函数声明**********************************

//********************************函数定义***********************************
/*********************************************************************
//函数名称	:FRAM_Init()
//功能		:铁电初始化
//输入		:
//		:
//		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void FRAM_Init(void)
{
        GpioOutInit(FR_CS);
        
        //片选初始化为高电平
        FRAM_CS_HIGH();
}
/*********************************************************************
//函数名称	:CheckFramChip(void)
//功能		:检查铁电芯片工作是否正常
//输入		:铁电地址的最后两个字节用于自检
//		:
//		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
ErrorStatus CheckFramChip(void)
{
	u8	ReadBuffer[3] ={0};
	u8	WriteBuffer[3] = {0};
	u16	Address;
	u8	temp;
	ErrorStatus	ErrorFlag = SUCCESS;
	
	//读状态寄存器
	temp = FRAM_ReadStatusRegister();
	if(0 != (FRAM_BP1BP0_BIT & temp))
	{
		return ERROR;
	}
	//清除写忙标志
	FRAM_WriteDisable();
	
	Address = FRAM_LAST_ADDR;
	WriteBuffer[0] = 0x5a;
	FRAM_BufferWrite(Address, WriteBuffer, 1);
	FRAM_BufferRead(ReadBuffer, 1, Address);
	if(ReadBuffer[0] == WriteBuffer[0])//预防铁电事先写了一个同样的值
	{
		WriteBuffer[0] = 0xa5;
		FRAM_BufferWrite(Address, WriteBuffer, 1);
		FRAM_BufferRead(ReadBuffer, 1, Address);
		if(ReadBuffer[0] == WriteBuffer[0])
		{
		
		}
		else
		{
			ErrorFlag = ERROR;
		}
	}
	else
	{
		ErrorFlag = ERROR;
	}
	return ErrorFlag;
	
}
/*********************************************************************
//函数名称	:FRAM_BufferWrite(u16 WriteAddr, u8 *pBuffer, u16 NumBytesToWrite)
//功能		:往铁电里写数据,在数据某尾加入校验字节
//输入		:WriteAddr，铁电地址
//		:pBuffer，数据缓冲
//		:NumBytesToWrite，写入的字节数
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void FRAM_BufferWrite(u16 WriteAddr, u8 *pBuffer, u16 NumBytesToWrite)
{
	u8	temp;
	u16	i;
	u8	*p;
	u8	sum = 0;
	static u8 count = 0;
	
	p = pBuffer;
        
	SPI_INTERFACE_INIT();
        
	//读状态寄存器
	temp = FRAM_ReadStatusRegister();
	
	if(FRAM_WEL_BIT == (temp &FRAM_WEL_BIT))//正在写，直接跳出
	{
		count++;
		if(count > 20)//连续20次写失败清除写标志
		{
			count = 0;
			FRAM_WriteDisable();
		}
		return ;
	}
	else
	{
		count = 0;
		//求校验和
		sum = 0;
		for(i=0; i<NumBytesToWrite; i++)
		{
			sum += *(pBuffer+i);
		}
		//写使能
		FRAM_WriteEnable();

		//片选拉低
		FRAM_CS_LOW();

		//发送写命令
		sFLASH_SendByte(FRAM_WRITE);

		//发送地址高字节
		temp = WriteAddr >> 8;
		sFLASH_SendByte(temp);

		//发送地址低字节
		temp = WriteAddr;
		sFLASH_SendByte(temp);

		//写数据内容
		for(i=0; i<NumBytesToWrite; i++)
		{
			temp = *p++;
			sFLASH_SendByte(temp);
		}
		
		//写校验字节
		sFLASH_SendByte(sum);
		
		//片选拉高
		FRAM_CS_HIGH();
	}
}
/*********************************************************************
//函数名称	:FRAM_BufferRead(u8 *pBuffer, u16 NumBytesToRead, u16 ReadAddr)
//功能		:从铁电里读数据
//输入		:ReadAddr，铁电地址
//		:pBuffer，目标缓冲
//		:NumBytesToRead，读出的字节数
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:实际读出的字节数
//备注		:
*********************************************************************/
u8 FRAM_BufferRead(u8 *pBuffer, u16 NumBytesToRead, u16 ReadAddr)
{
	u8 	temp;
	u16	i;
	u8	*p;
	u8	verify;
	u8	sum;
	u8	zeroCount = 0;
	
	p = pBuffer;
        
	SPI_INTERFACE_INIT();
        
	//片选拉低
	FRAM_CS_LOW();

	//发送读命令
	sFLASH_SendByte(FRAM_READ);

	//发送地址高字节
	temp = ReadAddr >> 8;
	sFLASH_SendByte(temp);

	//发送地址低字节
	temp = ReadAddr;
	sFLASH_SendByte(temp);

	//读存储器内容
	for(i=0; i<NumBytesToRead; i++)
	{
		*p++ = sFLASH_ReadByte();
	}
	
	//读校验字节
	verify = sFLASH_ReadByte();
	
	//片选拉高
	FRAM_CS_HIGH();
	
	//计算校验和
	sum = 0;
	for(i=0; i<NumBytesToRead; i++)
	{
		sum += *(pBuffer+i);
		if(0==sum)zeroCount++;
	}
	if(zeroCount == NumBytesToRead)return 0;//o shit all the data is zero
		
	//判断校验和是否一致
	if(verify == sum)
	{
		return NumBytesToRead;
	}
	else
	{
		return 0;
	}
	
}
/*********************************************************************
//函数名称	:FRAM_WriteEnable(void)
//功能		:铁电写使能
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void FRAM_WriteEnable(void)
{
	//拉低片选
	FRAM_CS_LOW();

	//发送写使能命令
	sFLASH_SendByte(FRAM_WREN);

	//拉高片选
	FRAM_CS_HIGH();
}
/*********************************************************************
//函数名称	:FRAM_WriteDisable(void)
//功能		:铁电写禁止
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void FRAM_WriteDisable(void)
{
	//拉低片选
	FRAM_CS_LOW();

	//发送不使能写命令
	sFLASH_SendByte(FRAM_WRDI);

	//拉高片选
	FRAM_CS_HIGH();
}
/*********************************************************************
//函数名称	:FRAM_WriteStatusRegister(u8 Byte)
//功能		:写铁电状态寄存器
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:/WP必须是高电平或无效电平,/WP仅仅是保护写铁电状态寄存器
*********************************************************************/
void FRAM_WriteStatusRegister(u8 Byte)
{

        SPI_INTERFACE_INIT();
         
	//拉低片选
 	FRAM_CS_LOW();

	//发送写状态寄存器命令
	sFLASH_SendByte(FRAM_WRSR);

	//发送所写字节
	sFLASH_SendByte(Byte);

	//片选拉高
	FRAM_CS_HIGH();
}
/*********************************************************************
//函数名称	:FRAM_ReadStatusRegister(void)
//功能		:读铁电状态寄存器
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:状态寄存器的值
//备注		:
*********************************************************************/
u8   FRAM_ReadStatusRegister(void)
{
	u8	temp;
        
        SPI_INTERFACE_INIT();
        
	//拉低片选
 	FRAM_CS_LOW();

	//发送读状态寄存器命令
	sFLASH_SendByte(FRAM_RDSR);

	//读状态寄存器
	temp = sFLASH_ReadByte();

	//片选拉高
	FRAM_CS_HIGH();

	//返回
	return temp;
}
/*********************************************************************
//函数名称	:FRAM_EraseChip(void)
//功能		:擦除整个铁电
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:状态寄存器的值
//备注		:
*********************************************************************/
void  FRAM_EraseChip(void)
{
	u16	i;
	u8	flag = 0;
        
        SPI_INTERFACE_INIT();
	for(i=0; i<FRAM_LAST_ADDR; i++)
	{
		FRAM_BufferWrite(i, &flag, 1);
	}
}


/************************************************************************* 
*
* Copyright (c) 2013,深圳市伊爱高新技术开发有限公司
* All rights reserved.
*
* 文件名称 : E2prom_25LC320A.c
* 功能     : E2prom_25LC320A_SPI的读写操作.
*
* 当前版本 : 1.0
* 开发者   : Senon Dou
* 修改时间 : 2013.07.26
*
* 历史版本 : 
* 开发者   : 
* 完成时间 : 
* 
* 备注 :    1 - 使用SPI1进行操作，注意EEPROM、FRAM、FLASH共用了SPI1，
                通过片选引脚选择进行操作。

            2 - 本程序中调用了spi_flash.c的部分函数，包括：
                uint8_t sFLASH_SendByte(uint8_t byte);
                uint8_t sFLASH_ReadByte(void);

            3 - SPI1的初始化在spi_flash.c中，本程序不再执行初始化操作。
*************************************************************************/
#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "include.h"

/********************************************************************
* 名称 : E2prom_InitIO
* 功能 : 25LC320A_SPI的GPIO初始化，包括25LC320A_CS、25LC320A_WP。
*
* 输入:   none
* 输出:   none
*
* 全局变量:   none
* 调用函数:  none
*
* 中断资源:  none
*
* 备注:   none
********************************************************************/
void E2prom_InitIO(void)
{ 
    GPIO_InitTypeDef  GPIO_InitStructure;
  
    /* 实用GPIO时钟*/
    RCC_AHB1PeriphClockCmd(E2PROM_25LC320A_CS_GPIO_CLK, ENABLE);
   
    /*!< Configure E2PROM 25LC320A CS pin in output pushpull mode ********************/
    GPIO_InitStructure.GPIO_Pin = E2PROM_25LC320A_CS_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(E2PROM_25LC320A_CS_GPIO_PORT, &GPIO_InitStructure);
  
    RCC_AHB1PeriphClockCmd(MEM_WP_GPIO_CLK, ENABLE);  
    /*!< Configure E2PROM 25LC320A wp pin in output pushpull mode ********************/
    GPIO_InitStructure.GPIO_Pin = MEM_WP_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(MEM_WP_GPIO_PORT, &GPIO_InitStructure);
  
    //sFLASH_LowLevel_Init();//初始化SPI1。由于EEPROM、FRAM、FLASH共用SPI1，这里默认SPI1已被初始化
  
    E2PROM_25LC320A_CS_HIGH();
    E2PROM_25LC320A_WP_HIGH();
   
}

/********************************************************************
* 名称 : E2prom_ReadByte
* 功能 : E2PROM_25LC320A读字节操作
*
* 输入: Address 读的起始地址
        *destin 读出数据存放地址指针
        bytenum 读的字节数
* 输出: none
*
* 全局变量:  none
* 调用函数: uint8_t sFLASH_SendByte(uint8_t byte);
            uint8_t sFLASH_ReadByte(void);
            u8 E2prom_ReadStatusRegister();
*
* 中断资源: none
*
* 备注: 
********************************************************************/
void E2prom_ReadByte(u16 Address, u8 *destin, u8 bytenum)
{
    u8 i;
    u8 temp;
    u8 count = 0;
    u8 *p;
    u8 addr;
  
    if(bytenum == 0)
        return;
    p = destin;
  
    //sFLASH_LowLevel_Init(); //初始化SPI1。由于EEPROM、FRAM、FLASH共用SPI1，这里默认SPI1已被初始化
    
    //检查写标志，等待写完成
    while(1)    
    {
        count++;
        //读状态寄存器
        temp = E2prom_ReadStatusRegister();
        if(E2PROM_WIP_BIT == (temp &E2PROM_WIP_BIT))//正在写进行中
	    {
            spi_Delay_uS(1000);//1ms，等待
        }
        else
        {
            break;  //跳出
        }
        if(count > 50) //最多等待50ms
        {
            return;  //返回
        }
    }
    
    E2PROM_25LC320A_CS_LOW();
      
    sFLASH_SendByte(READ);    //发送读命令
     
    addr = Address >> 8;
    sFLASH_SendByte(addr);  //发送地址高字节  
    addr = Address;
    sFLASH_SendByte(addr);  //发送地址低字节
     
    for(i = 0; i < bytenum; i++)
    {
        *p++ = sFLASH_ReadByte();  //读存储器内容
    }
    
    E2PROM_25LC320A_CS_HIGH();
  
}

/********************************************************************
* 名称 : E2prom_WriteByte
* 功能 : E2PROM_25LC320A写字节操作
*
* 输入: Address 写入的起始地址
        data    写入的数据
* 输出: SUCCESS 成功
        ERROR   失败
*
* 全局变量:  none
* 调用函数: uint8_t sFLASH_SendByte(uint8_t byte);
            uint8_t sFLASH_ReadByte(void);
            u8 E2prom_ReadStatusRegister();
*
* 中断资源: none
*
* 备注:  none
********************************************************************/
u8 E2prom_WriteByte(u16 Address, u8 data)
{
    u8 addr;
    u8 temp;
    u8 count = 0;

    //sFLASH_LowLevel_Init();//初始化SPI1。由于EEPROM、FRAM、FLASH共用SPI1，这里默认SPI1已被初始化
    
    //检查写标志，等待写完成
    while(1)    
    {
        count++;
        //读状态寄存器
        temp = E2prom_ReadStatusRegister();
        if(E2PROM_WIP_BIT == (temp &E2PROM_WIP_BIT))//正在写进行中
	    {
            spi_Delay_uS(1000);//1ms，等待
        }
        else
        {
            break;  //跳出
        }
        if(count > 50) //最多等待50ms
        {
            return ERROR;  //返回
        }
    }
    
    E2PROM_25LC320A_CS_LOW();        
    sFLASH_SendByte(WREN);    //写使能命令
    E2PROM_25LC320A_CS_HIGH();
    
    E2PROM_25LC320A_CS_LOW();
    
    sFLASH_SendByte(WRITE);    //发送写命令
    
    addr = Address >> 8;
    sFLASH_SendByte(addr);  //发送地址高字节
    addr = Address;
    sFLASH_SendByte(addr);  //发送地址低字节
    
    sFLASH_SendByte(data); //发送数据，发送完成后 写保护=RESET
  
    E2PROM_25LC320A_CS_HIGH();

    return SUCCESS;
}

/********************************************************************
* 名称 : E2prom_ReadStatusRegister(void)
* 功能 : 读状态寄存器
*
* 输入:   none
* 输出: 状态寄存器的值
*
* 全局变量:  none
* 调用函数: uint8_t sFLASH_SendByte(uint8_t byte);
            uint8_t sFLASH_ReadByte(void);
*
* 中断资源: none
*
* 备注:  none
********************************************************************/
u8 E2prom_ReadStatusRegister(void)
{
	u8	temp;
        
    sFLASH_LowLevel_Init();
        
	//拉低片选
 	E2PROM_25LC320A_CS_LOW();

	//发送读状态寄存器命令
	sFLASH_SendByte(RDSR);

	//读状态寄存器
	temp = sFLASH_ReadByte();

	//片选拉高
	E2PROM_25LC320A_CS_HIGH();

	//返回
	return temp;
}

/********************************************************************
* 名称 : CheckEepromChip(void)
* 功能 : 检查EEPROM芯片工作是否正常
*        eeprom地址的最后两个字节用来做自检用
* 输入:  none
* 输出:  none
*
* 全局变量: E2_LAST_ADDR  E2prom的最后两个字节的地址
* 调用函数: u8 E2prom_WriteByte(u16 Address, u8 data)
            void E2prom_ReadByte(u16 Address, u8 *destin, u8 bytenum)
*
* 中断资源: none
*
* 备注: none
********************************************************************/
ErrorStatus CheckEepromChip(void)
{
	u8	i;
	u8	ReadBuffer[2] = {0};
	u8	WriteBuffer[2] = {"OK"};
	u16	Address;
	
	Address = E2_LAST_ADDR;   //最后一个字节2*1024 - 2
	for(i=0; i<3; i++)
	{
		//往地址最后两个字节写数据
		E2prom_WriteByte(Address, WriteBuffer[0]);
		E2prom_WriteByte(Address+1, WriteBuffer[1]);
        
		//读出刚写入的字节数
		E2prom_ReadByte(Address,ReadBuffer,2);
		
		//比较写入的内容与读出的内容是否一致
		if((WriteBuffer[0] == ReadBuffer[0])&&(WriteBuffer[1] == ReadBuffer[1]))
		{
		    return SUCCESS;
		}
	}
	
	return ERROR;
}
/******************************************************************** 
//版权说明	:Shenzhen E-EYE Co. Ltd., Copyright 2009- All Rights Reserved.
//文件名称	:Lcd_Driver.c		
//功能		:液晶显示屏驱动程序
//版本号	:
//开发人	:dxl
//开发时间	:2010.11
//修改者	:
//修改时间	:
//修改简要说明	:
//备注		://使用的控制芯片是ST7565
***********************************************************************/
#include "include.h"

/*********************************************************************
//函数名称	:Lcd_DisplayString(u8 Line, u8 StartColumn, u8 DisplayModule, u8 *pBuffer, u16 length)
//功能		:显示一个字符串，该字符串可以是中英文混合
//输入		:Line：起始行，取值只能为1-4；StartColumm：起始列，取值为0-116；
//		:DisplayModule：显示模式，0为正常模式，1为反显；pBuffer，指向字符串的首字节；length，字符串长度
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
#ifndef NEW_LCD_DRIVER_GDI
void Lcd_DisplayString(u8 Line, u8 StartColumn, u8 DisplayModule, u8 *pBuffer, u16 length)	
{
	u8	*p;
	s8	LengthCount;
	u8	HighByte;
	u8	LowByte;
	u8	AsciiByte;
	u8	temp;
	u8	DispBuffer[33];
	s8	DispLine;
	u8	DispColumn;
	u8	num = 0;
	u32	Address;
	//u8	i;
	
	//初始化液晶显示屏IO口
	Lcd_IoInitial();
	/*
	for(i=0; i<200; i++)
	{
	
	}
	*/
	//检查行参数
	if((Line > 4)||(0 == Line))
	{
		return ;//行错误,行只能为1,2,3,4
	}
	//检查列参数
	if(StartColumn > (128-16))
	{
		return ;//列错误
	}
	//指向字符串头部,长度计数清0
	p = pBuffer;
	LengthCount = 0;
	DispLine = Line;
	DispColumn = StartColumn;
	
	//逐个显示直到字符串末尾
	for(;LengthCount < length;)
	{
		temp = *p;
		if(temp > 0x80)	//中文字符
		{
			//读取中文字符机内码
			HighByte = *p++;
			LowByte = *p++;
			if(0xFF == HighByte)//数据没有初始化过
			{
				HighByte = 0xA3;//显示为'@'
				LowByte = 0xC0;
			}
			//计算偏移地址
			Address = ((HighByte-0xa1)*94 + (LowByte-0xa1))*32;
			//读取字库
			Lcd_ReadOneChineseCharacterFromFlash(Address, DispBuffer);
			//显示一个汉字
			Lcd_DisplayOneChineseCharacter(DispLine, DispColumn, DisplayModule, DispBuffer);
			//列计数
			DispColumn += 16;
			//判断下一个字是汉字还是英文
			if( *p > 0x80)
			{
				num = 128 - 16;
			}
			else
			{
				num = 128 - 8;
			}
			//行计数
			if(DispColumn > num)//进入下一行
			{
				if(112 == num)//补一空格
				{
					//读取ASCII字符
					AsciiByte = ' ';
					//计算偏移地址
					Address = AsciiByte * 16;
					//读取字库
					Lcd_ReadOneEnglishCharacterFromFlash(Address, DispBuffer);
					//显示一个ASCII码
					Lcd_DisplayOneEnglishCharacter(DispLine, DispColumn, DisplayModule, DispBuffer);
				}
				DispColumn = 0;
				DispLine++;
				if(DispLine > 4)//回到第一行
				{
					return ;
					//DispLine = 1;
				}
				
			}
			//长度加2
			LengthCount += 2;
		}
		else //ASCII字符
		{
			//读取ASCII字符
			AsciiByte = *p++;
			//计算偏移地址
			Address = AsciiByte * 16;
			//读取字库
			Lcd_ReadOneEnglishCharacterFromFlash(Address, DispBuffer);
			//显示一个ASCII码
			Lcd_DisplayOneEnglishCharacter(DispLine, DispColumn, DisplayModule, DispBuffer);
			//列计数
			DispColumn += 8;
			//判断下一个字是汉字还是英文
			if( *p > 0x80)
			{
				num = 128 - 16;
			}
			else
			{
				num = 128 - 8;
			}
			//行计数
			if(DispColumn > num)//进入下一行
			{
				if(112 == num)//补一空格
				{
					//读取ASCII字符
					AsciiByte = ' ';
					//计算偏移地址
					Address = AsciiByte * 16;
					//读取字库
					Lcd_ReadOneEnglishCharacterFromFlash(Address, DispBuffer);
					//显示一个ASCII码
					Lcd_DisplayOneEnglishCharacter(DispLine, DispColumn, DisplayModule, DispBuffer);
				}
				DispColumn = 0;
				DispLine++;
				if(DispLine > 4)//回到第一行
				{
					return ;
					//DispLine = 1;
				}
			}
			//长度加1
			LengthCount++;
		}
	}		
}
#endif
/*********************************************************************
//函数名称	:Lcd_ReadOneEnglishCharacterFromFlash(u32 Address, u8 *pBuffer)
//功能		:从FLASH中读取一个英文字符的编码
//输入		:Address，偏移地址
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:读取的长度
//备注		:
*********************************************************************/
#ifdef	HZK16_IN_CHIP
u8 Lcd_ReadOneEnglishCharacterFromFlash(u32 Address, u8 *pBuffer)
{
	u8	i;
	u8	*p;
	p = pBuffer;
	for(i=0; i<16; i++)
	{
		*p++ = READ_ASCII_BYTE(Address+i);
	}
	return 16;
}
#else
u8 Lcd_ReadOneEnglishCharacterFromFlash(u32 Address, u8 *pBuffer)
{
	u32	Address1;
	Address1 = Address + 0x40000;
	sFLASH_ReadBuffer(pBuffer, Address1, 16);
	return 16;
}
#endif
/*********************************************************************
//函数名称	:Lcd_DisplayOneEnglishCharacter(u8 Line, u8 StartColumn, u8 DisplayModule, u8 *pBuffer)
//功能		:显示一个英文字符
//输入		:Line：起始行，取值只能为1-4；StartColumm：起始列，取值为0-124；
//		:DisplayModule：显示模式，0为正常模式，1为反显；pBuffer，指向英文编码8字节的首字节
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:0，正常；非0，异常
//备注		:
*********************************************************************/
u8 Lcd_DisplayOneEnglishCharacter(u8 Line, u8 StartColumn, u8 DisplayModule, u8 *pBuffer)
{

    #ifdef NEW_LCD_DRIVER_GDI
    Lcd_DisplayString(Line,StartColumn,DisplayModule,pBuffer,1);
    #else
	u8	HighHalfByte;
	u8	LowHalfByte;
	u8	i;
	u8	StartPage;
	u8	tempchar;
	u8	*p;
	
	//检查行参数
	if((Line > 4)||(0 == Line))
	{
		return 1;//行错误,行只能为1,2,3,4
	}
	//检查列参数
	if(StartColumn > (132-8))
	{
		return 2;//列错误
	}
	//指向首地址
	p = pBuffer;
	//计算页数
	if(1 == Line)
	{
		StartPage = 3;
	}
	else if(2 == Line)
	{
		StartPage = 1;
	}
	else if(3 == Line)
	{
		StartPage = 7;
	}
	else if(4 == Line)
	{
		StartPage = 5;
	}
	else
	{
		return 1;//
	}
	//提取列的高四位
	HighHalfByte = (StartColumn >> 4) & 0x0f;
	//提取列的低四位
	LowHalfByte = StartColumn & 0x0f;
	//设置起始列
	Lcd_WriteCom(LowHalfByte);
        Lcd_WriteCom(0x10 | HighHalfByte);
	//设置起始页
        Lcd_WriteCom(0xb0 | StartPage);
	//发送上半部
	for(i=0; i<8; i++)
	{
		tempchar = *p++;
		if(0 != DisplayModule)
		{
			tempchar = tempchar ^ 0xff;
		}
		Lcd_WriteData(tempchar);
	}
	//设置起始列
	Lcd_WriteCom(LowHalfByte);
        Lcd_WriteCom(0x10 | HighHalfByte);
	//设置起始页
        Lcd_WriteCom(0xb0 | (StartPage-1));
	//发送下半部
	for(i=0; i<8; i++)
	{
		tempchar = *p++;
		if(0 != DisplayModule)
		{
			tempchar = tempchar ^ 0xff;
		}
		Lcd_WriteData(tempchar);
	}
	#endif
	
	return 0;
}

/*********************************************************************
//函数名称	:ReadOneChineseCharacterFromFlash(u32	Address, u8 *pBuffer)
//功能		:从FLASH中读取一个汉字的编码
//输入		:Address，偏移地址
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:读取的长度
//备注		:
*********************************************************************/
#ifdef HZK16_IN_CHIP
u8 Lcd_ReadOneChineseCharacterFromFlash(u32	Address, u8 *pBuffer)
{
	u8	i;
	u8	*p;
	p = pBuffer;
	for(i=0; i<32; i++)
	{
		*p++ = READ_HZK16_BYTE(Address+i);
	}
	return 32;
}
#else
u8 Lcd_ReadOneChineseCharacterFromFlash(u32 Address, u8 *pBuffer)
{
	u32	Address1;
	Address1 = Address;
	sFLASH_ReadBuffer(pBuffer, Address1, 32);
	return 32;
}
#endif
/*********************************************************************
//函数名称	:Lcd_DisplayOneChineseCharacter(u8 Line, u8 StartColumn, u8 DisplayModule, u8 *pBuffer)
//功能		:显示一个汉字
//输入		:Line：起始行，取值只能为1-4；StartColumm：起始列，取值为0-116；
//		:DisplayModule：显示模式，0为正常模式，1为反显；pBuffer，指向汉字编码16字节的首字节
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:0，正常；非0，异常
//备注		:
*********************************************************************/
#ifndef NEW_LCD_DRIVER_GDI
u8 Lcd_DisplayOneChineseCharacter(u8 Line, u8 StartColumn, u8 DisplayModule, u8 *pBuffer)
{
	u8	HighHalfByte;
	u8	LowHalfByte;
	u8	i;
	u8	StartPage;
	u8	tempchar;
	u8	*p = NULL;
	
	//检查行参数
	if((Line > 4)||(0 == Line))
	{
		return 1;//行错误,行只能为1,2,3,4
	}
	//检查列参数
	if(StartColumn > (132-16))
	{
		return 2;//列错误
	}
	//指向首地址
	p = pBuffer;
	//计算页数
	if(1 == Line)
	{
		StartPage = 3;
	}
	else if(2 == Line)
	{
		StartPage = 1;
	}
	else if(3 == Line)
	{
		StartPage = 7;
	}
	else if(4 == Line)
	{
		StartPage = 5;
	}
	else
	{
		return 1;//行错误
	}
	//提取列的高四位
	HighHalfByte = (StartColumn >> 4) & 0x0f;
	//提取列的低四位
	LowHalfByte = StartColumn & 0x0f;
	//设置起始列
	Lcd_WriteCom(LowHalfByte);
        Lcd_WriteCom(0x10 | HighHalfByte);
	//设置起始页
        Lcd_WriteCom(0xb0 | StartPage);
	//发送上半部
	for(i=0; i<16; i++)
	{
		tempchar = *p++;
		if(0 != DisplayModule)
		{
			tempchar = tempchar ^ 0xff;
		}
		Lcd_WriteData(tempchar);
	}
	//设置起始列
	Lcd_WriteCom(LowHalfByte);
        Lcd_WriteCom(0x10 | HighHalfByte);
	//设置起始页
        Lcd_WriteCom(0xb0 | (StartPage-1));
	//发送下半部
	for(i=0; i<16; i++)
	{
		tempchar = *p++;
		if(0 != DisplayModule)
		{
			tempchar = tempchar ^ 0xff;
		}
		Lcd_WriteData(tempchar);
	}	
	return 0;
}
#endif
/*********************************************************************
//函数名称	:Lcd_InitialST7567(void)
//功能		:初始化液晶显示屏
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void Lcd_InitialST7567(void)
{   
   	Lcd_WriteCom(0xe2);  //0xE2=B1110 0010,软件复位
	LCD_Delay();

    	Lcd_WriteCom(0xa2);  //0xA2=B1010 0010,设置LCD偏压比,0-1/9
	LCD_Delay();
	
	Lcd_WriteCom(0xa0);  //0xA0=B1010 0000,显示地址增减,0-常规,从左到右
	LCD_Delay();
	
	Lcd_WriteCom(0xc0);  //0xC0=B11000000,行扫描顺序选择,0-普通顺序
	LCD_Delay();
        
	Lcd_WriteCom(0x27);  //0x27=B0010 0111,内部电压值电阻设置,(20h-27h) 27h ,改进型一体机的屏,正常使用的
	
	
	LCD_Delay();
        
	Lcd_WriteCom(0x81);  //0x81=B1000 0001,设置内部电阻微调,设置内部电阻微调，以设置液晶电压，此两个指令需紧接着使用
	LCD_Delay();
        
	Lcd_WriteCom(0x32);  //0x32=B0011 0010,设置电压值(0~63级),改进型一体机的屏,正常使用的
	
	LCD_Delay();
	
	Lcd_WriteCom(0x2f);  //0x2F=B0010 1111,电源控制集,选择内部电压供应操作模式。
	LCD_Delay();
	
    	Lcd_WriteCom(0x40);  //0x40=B0100 0000,显示初始行设置
	LCD_Delay();
	Lcd_WriteCom(0xb0);  //0xB0=B1011 0000,显示页地址，共4 位,页0
	LCD_Delay();
	Lcd_WriteCom(0xb7);  //0xB7=B10110111,显示页地址，共4 位,页7,总页数是0~7共八页
	LCD_Delay();
	
	Lcd_WriteCom(0x01);  //0x01=B0000 0001 设置列地址低4位,-1
	LCD_Delay();
	Lcd_WriteCom(0x10);  //0x01=B0001 0000 设置列地址高4位,-0,构成B0000 0001 =1列
	LCD_Delay();
	
	Lcd_WriteCom(0x00);  //0x00=B0000 0000 设置列地址低4位,-0
	LCD_Delay();
	Lcd_WriteCom(0x18);  //0x01=B0001 1000 设置列地址高4位,-1000,构成B1000 0000 = 128列
	LCD_Delay();
	Lcd_WriteCom(0xaf);  //0xAF=B1010 1111, 显示开
	LCD_Delay();
}
/*********************************************************************
//函数名称	:Lcd_BeepIoInit(void)
//功能		:蜂鸣器IO初始化
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void Lcd_BeepIoInit(void)
{
	GpioOutInit(BUZZER);
}
/*********************************************************************
//函数名称	:Lcd_IoInitial(void)
//功能		:液晶模块IO口初始化
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void Lcd_IoInitial(void)
{
	GpioOutInit(LCD_ON);
        GpioOutInit(LCD_CS);
        GpioOutInit(LCD_AO);
        GpioOutInit(LCD_SPI_SCK);
        GpioOutInit(LCD_SPI_MISO);
        GpioOutInit(LCD_SPI_MOSI);
        GpioOutOn(LCD_CS);              //取消片选
        GpioOutOn(LCD_SPI_MOSI);//dxl,2014.1.14增加
        GpioOutOn(LCD_SPI_SCK);//dxl,2014.1.14增加
        GpioOutOn(LCD_AO);//dxl,2014.1.14增加
}

/*********************************************************************
//函数名称	:Lcd_WriteCom(u8 com)
//功能		:写一个字节的命令
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
/*dxl,2014.1.9屏蔽,原来的驱动程序
void Lcd_WriteCom(u8 com)
{
    u8 i;

	LCD_CS_LOW();
	LCD_A0_LOW();
    for(i=0;i<8;i++)
		{
		  if(0x80 == (0x80&com))
		  {
		  	LCD_DA_HIGH();
		  }
		  else
		  {
		  	LCD_DA_LOW();
		  }
		  
          	LCD_SCK_LOW();
		LCD_SCK_HIGH();
		//LCD_SCK_HIGH();
		LCD_SCK_LOW();
		  com=com<<1;
		}
	LCD_A0_HIGH();
	LCD_CS_HIGH();
}
*/
void Lcd_WriteCom(u8 com)
{
    u8 i;

	LCD_CS_LOW();
        LCD_SCK_HIGH();

        for(i=0;i<8;i++)
	{
		if(0x80 == (0x80&com))
		{
		  	LCD_DA_HIGH();
		}
		else
		{
		  	LCD_DA_LOW();
		}
                if(4 == i)
                {
                        LCD_A0_LOW();
                }
                
          	LCD_SCK_LOW();
		LCD_SCK_HIGH();
		com=com<<1;
	}
	LCD_CS_HIGH();
}
/*********************************************************************
//函数名称	:Lcd_WriteData(u8 dat)
//功能		:写一个字节的数据
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
/*dxl,2014.1.9屏蔽,原来的驱动程序
void Lcd_WriteData(u8 dat)
{
    	u8 i;
		
	LCD_CS_LOW();
	LCD_A0_HIGH();
    	for(i=0;i<8;i++)
	{
		if(0x01 == (0x01&dat))
		{
		  	LCD_DA_HIGH();
		}
		else
		{
			LCD_DA_LOW();
		}
          	LCD_SCK_LOW();
		//LCD_SCK_HIGH();
		LCD_SCK_HIGH();
		LCD_SCK_LOW();
		dat=dat >> 1;
	}
	LCD_A0_LOW();
	LCD_CS_HIGH();
}
*/
void Lcd_WriteData(u8 dat)
{
    	u8 i;
		
	LCD_CS_LOW();
        LCD_SCK_HIGH();
    	for(i=0;i<8;i++)
	{
		if(0x01 == (0x01&dat))
		{
		  	LCD_DA_HIGH();
		}
		else
		{
			LCD_DA_LOW();
		}
                if(4 == i)
                {
                        LCD_A0_HIGH(); 
                }
          	LCD_SCK_LOW();
		LCD_SCK_HIGH();
		dat=dat >> 1;
	}
	LCD_CS_HIGH();
}
/*********************************************************************
//函数名称	:Lcd_DisplayClr(void)
//功能		:清除整个屏幕
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void Lcd_DisplayClr(void)
{
    #ifdef NEW_LCD_DRIVER_GDI
    LcdClearScreen();
    #else
	u8 i,j;

	for(i=0;i<8;i++)
       	{
         	Lcd_WriteCom(0x01);
        	Lcd_WriteCom(0x10);
         	Lcd_WriteCom(0xb0|i);

         	for(j=0;j<132;j++)
            	{
              		Lcd_WriteData(0x00);
            	}
       }
    #endif
}
/*********************************************************************
//函数名称	:Lcd_Initial(void)
//功能		:lcd模块初始化
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void Lcd_Initial(void)
{
	Lcd_BeepIoInit();
	Lcd_IoInitial();	
    
	Lcd_InitialST7567();
	Lcd_DisplayClr();	
    
}
/*********************************************************************
//函数名称	:ChangeGraphFormatToChineseFormat(u8 *pGraphBuffer, u8 *pChineseBuffer)
//功能		:将八厅局之前图形格式的编码转换成汉字编码
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:两个Buffer的长度均为32字节
*********************************************************************/
void ChangeGraphFormatToChineseFormat(u8 *pChineseBuffer, u8 *pGraphBuffer)
{
	u8	i;
	u8	j;
	u8	temp;
	u8	NewChar;
	u8	MoveBit = 0x01;
	u8	*pC;
	u8	*pG;
	
	pC = pChineseBuffer;
	
	//转换左上部
	for(i=0;i<8; i++)
	{
		NewChar = 0;
		pG = pGraphBuffer;
		for(j=0; j<8; j++)
		{
			//取8个图形字节的第(7-i)位
			temp = *pG++;
			temp &= (MoveBit << (7-i));
			temp = temp >> (7-i);
			NewChar |= (temp << j);
		}
		*pC++ = NewChar;
	}
		
	//转换右上部
	for(i=0;i<8; i++)
	{
		NewChar = 0;
		pG = pGraphBuffer+16;
		for(j=0; j<8; j++)
		{
			//取8个图形字节的第(7-i)位
			temp = *pG++;
			temp &= (MoveBit << (7-i));
			temp = temp >> (7-i);
			NewChar |= (temp << j);
		}
		*pC++ = NewChar;
	}
		
	//转换左下部
	for(i=0;i<8; i++)
	{
		NewChar = 0;
		pG = pGraphBuffer+8;
		for(j=0; j<8; j++)
		{
			//取8个图形字节的第(7-i)位
			temp = *pG++;
			temp &= (MoveBit << (7-i));
			temp = temp >> (7-i);
			NewChar |= (temp << j);
		}
		*pC++ = NewChar;
	}
		
	//转换右下部
	for(i=0;i<8; i++)
	{
		NewChar = 0;
		pG = pGraphBuffer+24;
		for(j=0; j<8; j++)
		{
			//取8个图形字节的第(7-i)位
			temp = *pG++;
			temp &= (MoveBit << (7-i));
			temp = temp >> (7-i);
			NewChar |= (temp << j);
		}
		*pC++ = NewChar;
	}
}
/*********************************************************************
//函数名称	:displayfull
//功能		:满屏显示,专用于测试
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void displayfull(void)
{
    u8 i,j;

    for(i=0;i<8;i++)
    {
         Lcd_WriteCom(0x01);
         Lcd_WriteCom(0x10);
         Lcd_WriteCom(0xb0|i);

         for(j=0;j<128;j++)
         {
            Lcd_WriteData(0xff);
         }
     }
}
/*********************************************************************
//函数名称	:LCD_Delay
//功能		:延迟函数
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void LCD_Delay(void)
{
    __IO uint32_t i = 0;
    for(i = 0xFF; i != 0; i--)
    {
    }
}
/*********************************************************************
//函数名称	:lcd_delay(u32 n)
//功能		:延迟函数
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:延时n个for语句时间
*********************************************************************/

static void lcd_delay(u32 n)
{
	for(;n>0;n--);
}


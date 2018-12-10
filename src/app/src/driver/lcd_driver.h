
#ifndef __LCD_DRIVER_H
#define __LCD_DRIVER_H

//*********宏定义****************
//*********宏定义****************
//#define HZK16_IN_CHIP	1

#define ASCII_BASE_ADDR	0x0803f7ff	//英文字符存储首地址
#define HZK16_BASE_ADDR	0x0803ffff	//汉字库存储首地址
#define READ_HZK16_BYTE(i) (*(u8 *)(HZK16_BASE_ADDR+(i)))
#define READ_ASCII_BYTE(i) (*(u8 *)(ASCII_BASE_ADDR+(i)))

#define	HZK16_END_ADDR	0x41000

//********A0--PC.8**************
#define LCD_A0_HIGH()	GpioOutOn(LCD_AO);lcd_delay(4)   //命令,数据选择线
#define LCD_A0_LOW()	GpioOutOff(LCD_AO);lcd_delay(4)
//********SCK--PB.13**************
#define LCD_SCK_HIGH()	GpioOutOn(LCD_SPI_SCK);lcd_delay(4) //delay(4),周期为1.42us,其中低电平为440ns,空闲时为高电平
#define LCD_SCK_LOW()	GpioOutOff(LCD_SPI_SCK);lcd_delay(4)
//********DA--PB.14**************
#define LCD_DA_HIGH()	GpioOutOn(LCD_SPI_MISO);lcd_delay(4)
#define LCD_DA_LOW()	GpioOutOff(LCD_SPI_MISO);lcd_delay(4)
//********CS--PE.8**************
#define LCD_CS_HIGH()	GpioOutOn(LCD_CS);lcd_delay(4)
#define LCD_CS_LOW()	GpioOutOff(LCD_CS);lcd_delay(4)

//*********函数声明**************
/*********************************************************************
//函数名称	:Lcd_Delayms(u32 n)
//功能		:延时程序
//备注		:
*********************************************************************/
void Lcd_Delayms(u32 n);
/*********************************************************************
//函数名称	:Lcd_DisplayClr(void)
//功能		:清除整个屏幕
//备注		:
*********************************************************************/
void Lcd_DisplayClr(void);
/*********************************************************************
//函数名称	:Lcd_WriteData(u8 dat)
//功能		:写一个字节的数据
//备注		:
*********************************************************************/
void Lcd_WriteData(u8 dat);
/*********************************************************************
//函数名称	:Lcd_WriteCom(u8 com)
//功能		:写一个字节的命令
//备注		:
*********************************************************************/
void Lcd_WriteCom(u8 com);
/*********************************************************************
//函数名称	:Lcd_IoInitial(void)
//功能		:液晶模块IO口初始化
//备注		:
*********************************************************************/
void Lcd_IoInitial(void);
/*********************************************************************
//函数名称	:Lcd_InitialST7567(void)
//功能		:初始化液晶显示屏

//备注		:
*********************************************************************/
void Lcd_InitialST7567(void);
/*********************************************************************
//函数名称	:Lcd_DisplayOneChineseCharacter(u8 Line, u8 StartColumn, u8 DisplayModule, u8 *pBuffer)
//功能		:显示一个汉字
//备注		:
*********************************************************************/
u8 Lcd_DisplayOneChineseCharacter(u8 Line, u8 StartColumn, u8 DisplayModule, u8 *pBuffer);
/*********************************************************************
//函数名称	:Lcd_DisplayOneChineseCharacter(u8 Line, u8 StartColumn, u8 DisplayModule, u8 *pBuffer)
//功能		:显示一个汉字
//备注		:
*********************************************************************/
u8 Lcd_DisplayOneChineseCharacter(u8 Line, u8 StartColumn, u8 DisplayModule, u8 *pBuffer);
/*********************************************************************
//函数名称	:ReadOneChineseCharacterFromFlash(u32	Address, u8 *pBuffer)
//功能		:从FLASH中读取一个汉字的编码
//备注		:
*********************************************************************/
u8 Lcd_ReadOneChineseCharacterFromFlash(u32	Address, u8 *pBuffer);
/*********************************************************************
//函数名称	:Lcd_DisplayOneEnglishCharacter(u8 Line, u8 StartColumn, u8 DisplayModule, u8 *pBuffer)
//功能		:显示一个英文字符
//备注		:
*********************************************************************/
u8 Lcd_DisplayOneEnglishCharacter(u8 Line, u8 StartColumn, u8 DisplayModule, u8 *pBuffer);
/*********************************************************************
//函数名称	:Lcd_ReadOneEnglishCharacterFromFlash(u32 Address, u8 *pBuffer)
//功能		:从FLASH中读取一个英文字符的编码
//备注		:
*********************************************************************/
u8 Lcd_ReadOneEnglishCharacterFromFlash(u32 Address, u8 *pBuffer);
/*********************************************************************
//函数名称	:Lcd_DisplayString(u8 Line, u8 StartColumn, u8 DisplayModule, u8 *pBuffer, u16 length)
//功能		:显示一个字符串，该字符串可以是中英文混合
//备注		:
*********************************************************************/
void Lcd_DisplayString(u8 Line, u8 StartColumn, u8 DisplayModule, u8 *pBuffer, u16 length);
/*********************************************************************
//函数名称	:Lcd_Initial(void)
//功能		:LCD模块初始化
//备注		:
*********************************************************************/
void Lcd_Initial(void);
/*********************************************************************
//函数名称	:Lcd_BeepIoInit(void)
//功能		:蜂鸣器IO初始化
//备注		:
*********************************************************************/
void Lcd_BeepIoInit(void);
/*********************************************************************
//函数名称	:ChangeGraphFormatToChineseFormat(u8 *pGraphBuffer, u8 *pChineseBuffer)
//功能		:将八厅局之前图形格式的编码转换成汉字编码
//备注		:两个Buffer的长度均为32字节
*********************************************************************/
void ChangeGraphFormatToChineseFormat(u8 *pChineseBuffer, u8 *pGraphBuffer);
/*********************************************************************
//函数名称	:Lcd_DisplayString(u8 Line, u8 StartColumn, u8 DisplayModule, u8 *pBuffer, u16 length)
//功能		:显示一个字符串，该字符串可以是中英文混合
//备注		:
*********************************************************************/
void Lcd_DisplayString(u8 Line, u8 StartColumn, u8 DisplayModule, u8 *pBuffer, u16 length);

void displayfull(void);

void LCD_Delay(void);

/*********************************************************************
//函数名称	:lcd_delay(u32 n)
//功能		:延迟函数
//备注		:延时n个for语句时间
*********************************************************************/
static void lcd_delay(u32 n);
#endif

/*******************************************************************************
 * File Name:			Gdi.h 
 * Function Describe:	132*64 LCD init and Display Module
 * Relate Module:		device.h,lcd.h
 * Explain:				modify from joneming's code
 * Writer:				joneming
 * Date:				2012-01-03
 * Rewriter:
 * Date:		
 *******************************************************************************/
 /*******************************************************************************
 * File: Gdi.c
 * Desc: 内存画图
 *******************************************************************************/
#ifndef _GDI_H_
#define _GDI_H_
//覆盖
#define OVER	0x00
#define OR		0x01
#define XOR		0x02
#define GRAY	0x03
#define SCR_WIDTH		132
#define SCR_HEIGHT		64

#define NEW_LCD_DRIVER_GDI
//lcd initialize,must be used before use this module 

void LcdClearScreen(void);
void LcdCopyScreen(void);
void LcdPasteScreen(void);

/********************************************************************************
 * Function:LcdShowPic
 * picture's width = width , height = height
 *******************************************************************************/
void DrawPic(
	unsigned char x,
	unsigned char y,
	unsigned char width,
	unsigned char height,
	const unsigned char *p_pucPic,
	unsigned char mode
);
void LcdPaintScreen
(
	unsigned char x1,
	unsigned char y1,
	unsigned char x2,
	unsigned char y2
);

void LcdClearArea
(
	unsigned char x1,
	unsigned char y1,
	unsigned char x2,
	unsigned char y2
);

void LcdCopyArea
(
	unsigned char x1,
	unsigned char y1,
	unsigned char width,
	unsigned char height,
	unsigned char x2,
	unsigned char y2
);

void LcdShowVerLine
(
	unsigned char x,
	unsigned char y1,
	unsigned char y2,
	unsigned char mode
);

void LcdShowHorLine
(
	unsigned char x1,
	unsigned char x2,
	unsigned char y,
	unsigned char mode
);

void LcdShowRect
(
	unsigned char x1,
	unsigned char y1,
	unsigned char x2,
	unsigned char y2,
	unsigned char mode
);

void LcdReverseBar
(
	unsigned char x1,
	unsigned char y1,
	unsigned char x2,
	unsigned char y2
);

void LcdShowArrow
(
	unsigned char x,
	unsigned char y,
	unsigned char type,
	unsigned char len,
	unsigned char mode
);

void LcdShowBorder(void);



void LcdShowPic
(
	unsigned char x,
	unsigned char y,
	unsigned char width,
	unsigned char height,
	const unsigned char *p_pucPic
);

void LcdShowWindow
(
	unsigned short x1,
	unsigned char y1,
	unsigned short x2,
	unsigned char y2
);
/******************************************************************************
 * Function:LcdShowBorder
 * show a border
 ******************************************************************************/
void LcdShowStr
(
	unsigned char x,
	unsigned char y,
	const char *string,
	unsigned char mode
);
/********************************************************************************
*Function:LcdShowPic
*picture's width = width , height = height
*******************************************************************************/
unsigned char LcdShowStrEx
(
	unsigned char x,
	unsigned char y,
	const char *string,
	unsigned char mode,
	unsigned short length
);
/********************************************************************************
*Function:LcdShowPic
*picture's width = width , height = height
*******************************************************************************/
void LcdShowPic(
	unsigned char x,
	unsigned char y,
	unsigned char width,
	unsigned char height,
	const unsigned char *p_pucPic
);
/******************************************************************************
 * Function:LcdShowButton
 * show a border
 ******************************************************************************/
void LcdShowButton(char *str,unsigned short x,unsigned char y,unsigned char mode);
/*************************************************************
** 函数名称: LcdShowCaption
** 功能描述: 居中显示字符串(只能显示一行字)
** 入口参数: str字符串首地址,y:显示数据的Y坐标(按64点阵算)
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void LcdShowCaption(char *str,unsigned char y);
/*************************************************************
** 函数名称: LcdShowCaptionEx
** 功能描述: 居中显示字符串(显示多行字)
** 入口参数: str字符串首地址,y:显示数据的Y坐标(按64点阵算)
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void LcdShowCaptionEx(char *str,unsigned char y);
/******************************************************************************
 * Function:LcdShowWaitPlease
 * show a border
 ******************************************************************************/
void LcdShowWaitPlease(unsigned short x,unsigned short y, char *str);
/*************************************************************
** 函数名称: LcdShowMsg
** 功能描述: 显示提示信息(只能显示一行字,不清屏),定时返回显示之前的界面
** 入口参数: msg字符串首地址,time:显示时间
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void LcdShowMsg(char *msg,unsigned long time);
/*************************************************************
** 函数名称: LcdShowMsgEx
** 功能描述: 显示提示信息(显示多行字,清屏),定时返回显示之前的界面
** 入口参数: msg字符串首地址,time:显示时间
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void LcdShowMsgEx(char *msg,unsigned long time);
/******************************************************************************
 * Function:Lcd_DisplayString
 * show a border
 ******************************************************************************/
void Lcd_DisplayString(unsigned char Line, unsigned char StartColumn, unsigned char DisplayModule, unsigned char *pBuffer, unsigned short length);
/******************************************************************************
 * Function:Lcd_DisplayOneChineseCharacter
 * show a border
 ******************************************************************************/
unsigned char Lcd_DisplayOneChineseCharacter(unsigned char Line, unsigned char StartColumn, unsigned char DisplayModule, unsigned char *pBuffer);
/******************************************************************************
 * Function:Lcd_DisplayNumber16
 * show a border
 ******************************************************************************/
void Lcd_DisplayNumber16(unsigned char Line, unsigned char StartColumn,unsigned char DisplayModule,unsigned char val);
/******************************************************************************
 * Function:Lcd_DisplayString
 * show a border
 ******************************************************************************/
void ClearLcdCopyFlag(void);
/******************************************************************************
 * Function:Lcd_DisplayString
 * show a border
 ******************************************************************************/
unsigned char GetLcdCopyFlag(void);
/******************************************************************************
 * Function:LcdShowOnePage
 * show a border
 ******************************************************************************/
unsigned char LcdShowOnePage(unsigned char *pBuffer);
#endif//_GDI_H_
/*******************************************************************************
 *                               End of File
 *******************************************************************************/

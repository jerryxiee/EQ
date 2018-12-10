/*******************************************************************************
 * File Name:			ICCardBase.h 
 * Function Describe:	
 * Relate Module:		IC卡模块
 * Writer:				Joneming
 * Date:				2014-07-09
 * ReWriter:			
 * Date:				
 *******************************************************************************/
#ifndef __ICCARD_BASE_H_
#define __ICCARD_BASE_H_


//*************文件包含***************
#include "stm32f2xx.h"

/***************   宏 定义区   ********************************/
#define ICCARD_EEYE         1 //Eye自主开发的IC卡
#define ICCARD_JTD          2 //交通部指定的IC卡  雄帝或成为
///////////////////////////////////////
//IC卡类数据内容用途类型
#define ICCARD_USAGE_DRIVER_19056_2012    0x0A
#define ICCARD_USAGE_DRIVER_808_2011      0x00
#define ICCARD_USAGE_EWAYBILL             0x01
#define ICCARD_USAGE_MANAGE               0x02
#define ICCARD_USAGE_UNDEFINE             0xFF

//行标808外设类型
#define EXDEVICE_TYPE_ICCARD                0x0B //道路运输证IC卡读卡器

/********************************************/
#define MAX_SIZE_BUFFER_IC_CARD  149
//-----------------------------------------------------
/***************   结构体 定义区   ********************************/

//++++++++  结构体: 行标808标准外设  通用 ++++++++++++++
#pragma pack(1)
typedef struct tagSTRUCT_PROTOCOL_EXDEVICE {
    /****  协议头 ***/
    //u8 Head;
    /****  校验码 从厂商编号到用户数据依次累加的累加和，取低８位 ***/
    u8 Verity;
    /****  版本号 ***/
    u8 PID[2];
    /****  厂商编号 ***/
    u8 VID[2];
    /****  外设类型编号 ***/
    u8 Type;
    /****  命令类型***/
    u8 Cmd;
    /****  用户数据 ***/
    u8 Data[ MAX_SIZE_BUFFER_IC_CARD];

}ST_PROTOCOL_EXDEVICE;

#pragma pack()
////////////////ICCARD_CS////////////////////////////
#define RCC_ICCARD_CS           RCC_AHB1Periph_GPIOD           /* GPIO的D组端口时钟 */
#define GPIO_ICCARD_CS          GPIOD                          /* 选定D组端口*/ 
#define PIN_ICCARD_CS           GPIO_Pin_5                     /* 选定端口7*/
#define ICCARD_CS_READ()        GPIO_ReadInputDataBit( GPIO_ICCARD_CS , PIN_ICCARD_CS )	/* 读IO口线状态 */

///// IC卡供电脚 IC-P  PC.9 //////
//-------------------------------------------------------------------------------------------
#define RCC_IC_CARD_POWER       RCC_AHB1Periph_GPIOC  /* GPIO端口时钟 */
#define GPIO_IC_CARD_POWER      GPIOC                 /* GPIO端口 */
#define PIN_IC_CARD_POWER       GPIO_Pin_9            /* GPIO引脚 */
//-------------------------------------------------------------------------------------------
#define IC_CARD_POWER_ON()      GPIO_SetBits(  GPIO_IC_CARD_POWER, PIN_IC_CARD_POWER ) 
#define IC_CARD_POWER_OFF()     GPIO_ResetBits(GPIO_IC_CARD_POWER, PIN_IC_CARD_POWER )


/***************   外部全局函数声明区   ********************************/
/********************************************************************
* 名称 : ICCardBase_VerifyCalcSum
* 功能 : 计算 数组的 校验(累加和)
********************************************************************/
u8 ICCardBase_VerifyCalcSum( u8 * buff, u16 length);
/********************************************************************
* 名称 : ICCardBase_VerifyCalcSum
* 功能 : 计算 数组的 校验(异或)
********************************************************************/
u16 ICCardBase_ExtractExdevice( u8 *pInBuf, u16 InLength, ST_PROTOCOL_EXDEVICE *pPacket    );
/*************************************************************
** 函数名称: ICCardBase_CreatPackData
** 功能描述: 数据打包
*************************************************************/
u16 ICCardBase_CreatPackData( u8 cmd,  u8 * pInBuf , u16 InLength , u8 * pOutBuf ,unsigned char response);
//-------------------------------------------------------------------------------------------
/*************************************************************
** 函数名称: ICCardBase_ParameterInitialize
** 功能描述: IC卡参数初始化
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void ICCardBase_ParameterInitialize(void);
/*************************************************************
** 函数名称: ICCardBase_GetReadFlag
** 功能描述: 终端是否正在读IC卡
** 入口参数: 
** 出口参数: 
** 返回参数: 1:正在读,0空闲
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char ICCardBase_GetReadFlag(void);
/*************************************************************
** 函数名称: ICCardBase_GetUseIntFlag
** 功能描述: 是否通过自身CPU通过I2C读IC卡
** 入口参数: 
** 出口参数: 
** 返回参数: 1:通过自身CPU通过I2C读IC卡,0:使用外部独立的IC卡模块
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char ICCardBase_GetUseIntFlag(void);
/*************************************************************
** 函数名称: ICCardBase_TimerTask
** 功能描述: 定时器处理函数
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void ICCardBase_TimerTask(void);
#endif
/******************************************************************************
**                            End Of File
******************************************************************************/
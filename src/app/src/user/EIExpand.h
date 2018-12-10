/*******************************************************************************
 * File Name:			EIExpand.c 
 * Function Describe:	EI扩展协议模块
 * Relate Module:		测试相关协议。
 * Writer:				Joneming
 * Date:				2013-07-16
 * ReWriter:			
 * Date:				
 *******************************************************************************/
#ifndef __EIEXPAND__H
#define __EIEXPAND__H
/*******************************************************************************/
#include "RadioProtocol.h"

#define USE_EIEXPAND_PROTOCOL 
///////////////////////////////////////////
#define EIEXPAND_PROTOCOL_TRAN_TYPE         0xFD//伊爱扩展协议透传类型

#define EIEXPAND_PROTOCOL_FUN_TAXI          0xF1//电召/出租车数据功能
////////////////////////////
#define EIEXPAND_PROTOCOL_FUN_LED           0xF2//LED功能
///////////////////////////
#define EIEXPAND_PROTOCOL_FUN_CARLOAD       0xF3//载重类功能
	
#define EIEXPAND_PROTOCOL_FUN_OBD           0xF5//OBD数据

#define EIEXPAND_PROTOCOL_FUN_TEMP          0xF8//温度采集功能

#define EIEXPAND_PROTOCOL_FUN_ACCELERATION  0xF7//急加速急减速

///////////////////////////////////////////
typedef enum
{
    EIEXPAND_CMD_ACCIDENT_REMIND = 0x0001,          //0x0001事故多发时段提醒
    EIEXPAND_CMD_MODIFY_AREA_VOICE,                 //0x0002修改区域语音
    EIEXPAND_CMD_OIL_WEAR_CALIB,                    //0x0003油耗标定
    EIEXPAND_CMD_SET_DRIVER_INFO,                   //0x0004设置驾驶员
    ///////////////////////////////////////
    EIEXPAND_CMD_MAX//
}E_EIEXPANDPROTOCOLCMD;
/****************************************************/
/*                                                  */
/* Local Definitions and macros                     */
/*                                                  */
/****************************************************/

/****************************************************/
/*                                                  */
/* Local Definitions and macros                     */
/*                                                  */
/****************************************************/
/*************************************************************
** 函数名称: EIExpand_DisposeFunctionProtocol
** 功能描述: 伊爱扩展协议处理功能协议
** 入口参数:  
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned char EIExpand_DisposeFunctionProtocol(unsigned short EICmd,unsigned char *pBuffer,unsigned short datalen);
/*************************************************************
** 函数名称: EIExpand_DisposeProtocol
** 功能描述: 伊爱扩展协议处理其它指令
** 入口参数:  
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned char EIExpand_DisposeProtocol(unsigned short EICmd,unsigned char *pBuffer,unsigned short datalen);
/*************************************************************
** 函数名称: EIExpand_RadioProtocolParse
** 功能描述: 伊爱扩展协议解释
** 入口参数: pBuffer伊爱扩展协议数据首地址, datalen伊爱扩展协议数据总长度
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned char EIExpand_RadioProtocolParse(unsigned char *pBuffer,unsigned short datalen);
/*************************************************************
** 函数名称: EIExpand_PotocolSendData
** 功能描述: 发送伊爱扩展协议到平台
** 入口参数: EICmd:子协议号,srcdat子协议消息体首地址,srclen子协议消息体长度;
** 出口参数: 
** 返回参数: 成功,返回ACK_OK;失败返回ACK_ERROR;
** 全局变量: 无
** 调用模块:
*************************************************************/
ProtocolACK EIExpand_PotocolSendData(unsigned short EICmd,unsigned char * srcdat,unsigned short srclen);
#endif//__EIEXPAND__H
/*******************************************************************************
 *                             end of module
 *******************************************************************************/


/*************************************************************************
* Copyright (c) 2013,???????????????
* All rights reserved.
* ???? : Module_IC_Card_JTB.h
* ???? : 1.0
* ???   : Shigle
* ???? : 20113?6?15?
*************************************************************************/

#ifndef __MODULE_IC_CARD_XIONGDI_H_
#define __MODULE_IC_CARD_XIONGDI_H_


//*************????***************
#include "stm32f2xx.h"

#define	IC_CARD_USE_XIONGDI						0		//??IC?
#define	IC_CARD_USE_GUANGZHOU_JY			1		//????IC?
#define	IC_CARD_MODE	IC_CARD_USE_GUANGZHOU_JY

#define IC_AUTHENTICATION_REQUEST_CODE  0x40  //IC?????
#define IC_AUTHENTICATION_RESULT_CODE   0x41  //?????IC???????
#define IC_SIGN_OUT_CODE                0x42  //??????
#define IC_ACTIVE_REQUEST_CODE          0x43  //??????IC?


#define IC_RESULT_REQUEST_SUCCESS     0x00
#define IC_RESULT_REQUEST_NOINSERT    0x01
#define IC_RESULT_REQUEST_FAIL        0x02
#define IC_RESULT_REQUEST_NOSUPPORT   0x03
#define IC_RESULT_REQUEST_LOCK        0x04



#define TIMEOUT_REQUEST_AUTH     						(35*SECOND)  //??35?
#define TIMEOUT_REQUEST_LINK     						(120*SECOND) //2分钟等待链接认证服务器
#define INITIATIVE_READ_CARD_WAIT_TIME     	(1200*SECOND)  //??????????20??

//////////////////////////
extern FunctionalState Task_IC_Card_JTB(void);
/*************************************************************
** ????: ICCard_DisposeRecvExdeviceJTB
** ????: ????
** ????: ?
** ????: ?
** ????: ?
** ????: 
** ????: 
*************************************************************/
void ICCard_DisposeRecvExdeviceJTB(ST_PROTOCOL_EXDEVICE stExdevice, unsigned short datalen);
/**************************************************************************
//???:ICCard_AuthenticationAck
//??:????
//??:?
//??:?
//???:?
//??:????????
***************************************************************************/
void ICCard_AuthenticationAck(u8 *pBuffer , u16 BufferLen);
/********************************************************************
* ?? : ICCard_CustomProtocolParse
* ?? : ??????????????
* ??  
********************************************************************/
ProtocolACK ICCard_CustomProtocolParse(u8 *pBuffer,u16 BufferLen);
//???? A1
void ICCard_CustomProtocolParse_Rev_0x8702(u8 *pBuffer,u16 BufferLen);
//???? A1 ??8702??  0702
void ICCard_CustomProtocolParse_Ack_0x0702(void);
//??????
void ICCard_Transmit_E_Signature(u8 *pBuffer);
/********************************************************************
* ?? : ICCard_SendCmdForClearElectronicCertificate
* ?? : ??????????
* ?? : dataType 0x00:???????(?)  0x10:?????  0x20:?????(?)
* ??   ??=>? 0xA2
********************************************************************/
unsigned char ICCard_SendCmdForClearElectronicCertificate(u8	cmdType,u8 dataType);
/********************************************************************
* 名称 : ICCard_Send_CMD_To_Terminal_0x03
* 功能 : IC卡行标外设命令处理:  0x03 从机电源控制
* 说明   终端发送命令给读卡器 controlMode 00:退出省电模式  01:进入省电模式
********************************************************************/
void ICCard_Send_CMD_To_Card_Reader_0x03( u8 controlMode );
#endif
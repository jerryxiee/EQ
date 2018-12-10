/**
  ******************************************************************************
  * @file    usbh_usr.h
  * @author  MCD Application Team
  * @version V2.1.0
  * @date    19-March-2012
  * @brief   Header file for usbh_usr.c
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USH_USR_H__
#define __USH_USR_H__

/* Includes ------------------------------------------------------------------*/
#include "ff.h"
#include "usbh_core.h"
#include "usb_conf.h"
#include <stdio.h>
#include "usbh_msc_core.h"

/** @addtogroup USBH_USER
  * @{
  */

/** @addtogroup USBH_MSC_DEMO_USER_CALLBACKS
  * @{
  */

/** @defgroup USBH_USR
  * @brief This file is the Header file for usbh_usr.c
  * @{
  */ 


/** @defgroup USBH_USR_Exported_Types
  * @{
  */ 


extern  USBH_Usr_cb_TypeDef USR_cb;



/**
  * @}
  */ 



/** @defgroup USBH_USR_Exported_Defines
  * @{
  */ 
/* State Machine for the USBH_USR_ApplicationState */
#define USH_USR_FS_INIT       0
#define USH_USR_FS_READLIST   1
#define USH_USR_FS_WRITEFILE  2
#define USH_USR_FS_DRAW       3
/**
  * @}
  */ 

/** @defgroup USBH_USR_Exported_Macros
  * @{
  */ 
/**
  * @}
  */ 

/** @defgroup USBH_USR_Exported_Variables
  * @{
  */ 
extern  uint8_t USBH_USR_ApplicationState ;
/**
  * @}
  */ 

/** @defgroup USBH_USR_Exported_FunctionsPrototype
  * @{
  */ 
void USBH_USR_ApplicationSelected(void);
void USBH_USR_Init(void);
void USBH_USR_DeInit(void);
void USBH_USR_DeviceAttached(void);
void USBH_USR_ResetDevice(void);
void USBH_USR_DeviceDisconnected (void);
void USBH_USR_OverCurrentDetected (void);
void USBH_USR_DeviceSpeedDetected(uint8_t DeviceSpeed); 
void USBH_USR_Device_DescAvailable(void *);
void USBH_USR_DeviceAddressAssigned(void);
void USBH_USR_Configuration_DescAvailable(USBH_CfgDesc_TypeDef * cfgDesc,
                                          USBH_InterfaceDesc_TypeDef *itfDesc,
                                          USBH_EpDesc_TypeDef *epDesc);
void USBH_USR_Manufacturer_String(void *);
void USBH_USR_Product_String(void *);
void USBH_USR_SerialNum_String(void *);
void USBH_USR_EnumerationDone(void);
USBH_USR_Status USBH_USR_UserInput(void);
void USBH_USR_DeInit(void);
void USBH_USR_DeviceNotSupported(void);
void USBH_USR_UnrecoveredError(void);
int USBH_USR_MSC_Application(void);
/*************************************************************
** 函数名称: USB_USR_CheckInsertUsbDisk
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned char USB_USR_CheckInsertUsbDisk(void);
/*************************************************************
** 函数名称: USB_USR_WriterDataToUsb
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void USB_USR_WriterDataToUsb(unsigned char folder,unsigned char folder2,unsigned char *buffer1,unsigned char *buffer2,unsigned char type);
/*******************************************************************************
* Function Name  : ff_NameConver
* Description    : 由于打开长文件名，调用ff_open函数时要把string转为16位数据
*            例如：f_open(&file,"12345.txt",FA_CREATE_ALWAYS | FA_WRITE)
*                  f_open(&file,ff_OpenConver("12345.txt"),FA_CREATE_ALWAYS | FA_WRITE)
* Input          : 指向要转换8位缓存
* Output         : None
* Return         : 返回8位文件名对应的16位数据
*******************************************************************************/ 
u16* ff_NameConver(u8 *pStr);
/*******************************************************************************
* Function Name  : ff_NameForward
* Description    : 由于打开长文件名，需要文件名转换时调用
* Input          : - > *pDst  : 指向要转换8位缓存
*                  - > *pSrc  : 指向16位文件名字符串数据
* Output         : None
* Return         : None
*******************************************************************************/ 
void ff_NameForward(u8 *pDst,u16 *pSrc);

/**
  * @}
  */ 

#endif /*__USH_USR_H__*/

/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/


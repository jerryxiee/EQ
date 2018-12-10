/*******************************************************************************
 * File Name:			usb_app.h 
 * Function Describe:	
 * Relate Module:		
 * Writer:				joneming
 * Date:				2012-05-15
 * Rewriter:
 * Date:		
 *******************************************************************************/
#ifndef __USB_APP_H_
#define __USB_APP_H_
///////////////////////////
#include "stm32f2xx.h"

//模块工作模式
typedef enum {
	USB_WORK_STATUS_EMPTY = 0,
	USB_WORK_STATUS_INSERT,  //插入U盘
	USB_WORK_STATUS_ERROR,  //安装U盘失败
	USB_WORK_STATUS_ENUM,    //枚举成功
	USB_WORK_STATUS_READLIST,//获得文件列表
	USB_WORK_STATUS_DEL,     //移除U盘	
	USB_WORK_STATUS_UPDATA,  //升级中
	USB_WORK_STATUS_UPDATA_FINISH,//升级完毕
	USB_WORK_STATUS_UPDATA_ERROR,  //升级出错
	USB_WORK_STATUS_PROTECTED,//U盘保护
	USB_WORK_STATUS_NO_U_DISK,//没有U盘
	USB_WORK_STATUS_READ_FILE_ERROR,//读文件出错
	USB_WORK_STATUS_START_SAVE,//正在保存数据
	USB_WORK_STATUS_SAVE_FINISH,//保存文件成功
	USB_WORK_STATUS_SAVE_ERROR,//保存文件出错
	USB_WORK_STATUS_MAX
}USBWORKSTATUS;
/////////////////////////////////////
/*************************************************************
** 函数名称: USB_ClearUsbUpdataFlag
** 功能描述: 清除U盘升级完毕标志
** 入口参数:  
** 出口参数: 
** 返回参数: 
*************************************************************/
void USB_ClearUsbUpdataFlag(void);
/*************************************************************
** 函数名称: USB_SetUsbUpdataFlag
** 功能描述: 置为U盘升级完毕标志
** 入口参数:  
** 出口参数: 
** 返回参数: 
*************************************************************/
void USB_SetUsbUpdataFlag(void);
/*************************************************************
** 函数名称: USB_GetReadUsbUpdataFlag
** 功能描述: 
** 入口参数:  
** 出口参数: 
** 返回参数: 1:为U盘升级完毕标志,0:其它
*************************************************************/
unsigned char USB_GetReadUsbUpdataFlag(void);
/*************************************************************
** 函数名称: USB_AppInitialize
** 功能描述: 
** 入口参数:  
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void USB_AppInitialize(void);
/*************************************************************
** 函数名称: Usb_FindFileAndSaveToUdisk
** 功能描述: 
** 入口参数: 	 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Usb_FindSDcardFileAndSaveToUdisk(unsigned char type,unsigned char *startTime,unsigned char *endTime);
/*************************************************************
** 函数名称: Usb_AppTimeTask
** 功能描述: 
** 入口参数: 	 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
FunctionalState Usb_AppTimeTask(void);
/*************************************************************
** 函数名称: Usb_Exti1Isr
** 功能描述: USB过流检测,在EXTI1_IRQHandler中断中调用
** 入口参数: 	 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Usb_Exti1Isr(void);
#endif//__USB_APP_H_
/*******************************************************************************
 *                             end of module
 *******************************************************************************/

/*******************************************************************************
 * File Name:			TaskMenu.h 
 * Function Describe:	
 * Relate Module:		
 * Writer:				joneming
 * Date:				2012-01-12
 * Rewriter:
 * Date:		
 *******************************************************************************/
#ifndef _TASKMENU_H_
#define _TASKMENU_H_
#include "include.h"

//#define SERVICE_MENU_JIUTONG  //若定义,则按九通要求,服务器参数不能通过显示屏设置
///////////////////////////////////////////////////
#define SYS_DEFAULT_PASSWORD    701000 //系统默认密码
/////(注意:超级密码请勿与系统默认密码一致///////////////
#define SUPER_PASSWORD          601703////(最大6位)超级密码,不管怎么改这个密码都可以进入
//(注意://系统默认密码,如果SHOW_INPUT_PASSWORD也为系统默认密码,则密码等于白设)////////
#define SHOW_INPUT_PASSWORD     0///(最大6位)在进入密码输入时,仅仅只是输入框显示的数字
//////////////////////////////////////////////////////////
extern u16 CommonShareBufferLen; 
extern u8 CommonShareBufferBusyFlag;
extern u8 RadioShareBuffer[];
extern unsigned char GB2312Buffer[];
/////////////////////////////////

typedef enum 
{
    MENU_TEST_REGISTER,//"终端注册"
    MENU_TEST_QUALIFY,//"终端鉴权"
    //MENU_TEST_LOGOUT,//"终端注销",
    MENU_TEST_SEND_POST,//"位置信息汇报"
    MENU_TEST_EVENT_REPORT,//"事件报告",//4
    MENU_TEST_SEND_MESSAGE,//"信息点播"
    MENU_TEST_CANCEL_MESSAGE,// "信息取消",//6
    MENU_TEST_SEND_E_WAYBILL,//"电子运单上报",//7
    MENU_TEST_DRIVER_INFO,//"驾驶员身份信息"   
    MENU_TEST_MULIT_EVENT,//"上发多媒体事件"
    MENU_TEST_SEND_DTU_DATA,//"数据上行透传"
    MENU_TEST_RESUME_FCT,//"恢复出厂未使用状态"
    MENU_TEST_MAX
}eSTMENUTEST;
//////////////////////////////////
///////////////////////////////
void ShowStartLogo(void);
void OpenLcdBackLight(void);
void FlashLcdBackLight(void);
void FlashLcdBackLightFinish(void);
void SysAutoRunCountReset(void);
void SysLcdBackLightCntReset(void);
void SysAutoRunToMainInfoTask(void);
unsigned char BrushAdministratorCardResponse(unsigned char *data);
///////////////////
void PublicSysParamterInit(void);
void ShowTextForMsgBusyFlag(unsigned char flag);
////////////////////////////
void ReflashToReturnMainInterface(unsigned long time);
void ShowMainInterface(void);
void SystemAutoShowText(unsigned short textlen,unsigned char *buffer);
void SystemAutoShowTextForTaxiLender(void);
////////////////////////////////////////////////////
void PublicConfirmAll(char *caption,void (*doit)(void),void (*Cancel)(void),unsigned long time,unsigned char select);
void PublicConfirm(char *caption,void (*doit)(void));
void PublicConfirmAutoExit(char *caption,void (*doit)(void),unsigned long time);
//////////////////////////////
LZM_RET TaskShowLogo(LZM_MESSAGE *Msg);
LZM_RET TaskMenuMain(LZM_MESSAGE *Msg);
LZM_RET TaskShowTextInfo(LZM_MESSAGE *Msg);
LZM_RET TaskShowMainInterface(LZM_MESSAGE *Msg);
LZM_RET TaskMenuPrint(LZM_MESSAGE *Msg);
/*************************************************************
** 函数名称: SystemAutoShowQuestionResponse
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void SystemAutoShowQuestionResponse(unsigned short datalen,unsigned char *buffer,unsigned char flag);
/*************************************************************
** 函数名称: GetShowMainInterfaceFlag
** 功能描述: 是否在主界面
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned char GetShowMainInterfaceFlag(void);
/*************************************************************OK
** 函数名称: Question_DisposeRadioProtocol
** 功能描述: 提问下发协议解析
** 入口参数: pBuffer:数据首地址,BufferLen:数据长度			 
** 出口参数: 
** 返回参数: :
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Question_DisposeRadioProtocol(u8 *pBuffer, u16 BufferLen);
/*************************************************************
** 函数名称: InfoService_DisposeRadioProtocol
** 功能描述: 点播信息操作协议解析
** 入口参数: pBuffer:数据首地址,BufferLen:数据长度
** 出口参数: 无
** 返回参数: 返回操作结果
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char InfoService_DisposeRadioProtocol(unsigned char *pBuffer,unsigned short BufferLen);
/*************************************************************
** 函数名称: GetCurentProductModel
** 功能描述: 获得当前终端型号
** 入口参数: 无
** 出口参数: name首地址
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void GetCurentProductModel(char *name);
////////////////////////////////
#endif//_TASKMENU_H_
/*******************************************************************************
 *                             end of module
 *******************************************************************************/

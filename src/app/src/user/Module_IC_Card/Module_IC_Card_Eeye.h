/*******************************************************************************
 * File Name:			ICCard.h 
 * Function Describe:	
 * Relate Module:		IC卡模块
 * Writer:				Joneming
 * Date:				2014-07-09
 * ReWriter:			
 * Date:				
 *******************************************************************************/
#ifndef __ICCARD_H_
#define __ICCARD_H_

/*************************************************************
** 函数名称: ICCard_TimeTask
** 功能描述: IC卡定时器处理函数
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
FunctionalState ICCard_TimeTask(void);
/*************************************************************
** 函数名称: ICCard_ParameterInitialize
** 功能描述: IC卡参数初始化
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void ICCard_ParamInit(void);
/*************************************************************
** 函数名称: ICCard_UpdataParamVoicePrompt
** 功能描述: IC卡更新驾驶员提醒标志
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void ICCard_UpdataParamVoicePrompt(void);
/*************************************************************
** 函数名称: ICCard_SendCmdForReadDriverInfo
** 功能描述: 主动读取IC卡内容指令
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
unsigned char ICCard_SendCmdForReadDriverInfo(void);
/*************************************************************
** 函数名称: ICCard_ProtocolParse
** 功能描述: 数据解析
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void ICCard_ProtocolParse(unsigned char * pInBuf, unsigned short InLength);
/*************************************************************OK
** 函数名称: ICCard_ReadRepairServiceDataForSendPC
** 功能描述: 检查发送盲区的签到、签到、营运数据
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void ICCard_ReadRepairServiceDataForSendPC(void);
/*************************************************************
** 函数名称: ICCard_PackAndSendData
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
unsigned char ICCard_PackAndSendData(unsigned char cmd,unsigned char *pBuffer,unsigned short datalen);
/*************************************************************
** 函数名称: ICCard_DisposeJTBCardDriverSignIn
** 功能描述: 处理雄帝或交通部规定的驾驶员卡
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void ICCard_DisposeJTBCardDriverSignIn(unsigned char *pBuffer, unsigned short datalen);
/*************************************************************
** 函数名称: ICCard_DisposeExternalDriverSignIn
** 功能描述: 处理外部显示屏驾驶员卡签到
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void ICCard_DisposeExternalDriverSignIn(unsigned char *pBuffer, unsigned short datalen);
/*************************************************************
** 函数名称: ICCard_DisposeExternalCardDriverSignOut
** 功能描述: 处理外部驾驶员卡签退
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void ICCard_DisposeExternalCardDriverSignOut(void);
/*************************************************************
** 函数名称: ICCard_GetExternalDriverInfoForSignIn
** 功能描述: 获取驾驶员代码及姓名
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
unsigned char ICCard_GetExternalDriverInfoForSignIn(unsigned char *pBuffer);
/*************************************************************
** 函数名称: ICCard_GetExternalDriverInfo
** 功能描述: 获取驾驶员姓名及驾驶证号码
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
unsigned char ICCard_GetExternalDriverInfo(unsigned char *pBuffer);
/*************************************************************
** 函数名称: ICCard_PackReportDriverInfo
** 功能描述: 打包当前登签信息
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
unsigned char ICCard_PackReportDriverInfo(unsigned char *pBuffer,unsigned char *pBuffer_Temp);
/*************************************************************
** 函数名称: ICCard_StartCheckCommTimeOut
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void ICCard_StartCheckCommTimeOut(void);
/*************************************************************
** 函数名称: ICCard_DisposeJTBCardDriverSignPassivity
** 功能描述: 广州二汽 被动应答电子证据信息
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void ICCard_DisposeJTBCardDriverSignPassivity(unsigned char *pBuffer, unsigned short datalen);
/*************************************************************
** 函数名称: ICCard_GetExternalDriverInfo
** 功能描述: 获取驾驶员姓名
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
unsigned char ICCard_GetExternalDriverName(unsigned char *pBuffer);
#endif

/*******************************************************************************
 *                             end of module
 *******************************************************************************/
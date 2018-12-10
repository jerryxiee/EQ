/*******************************************************************************
 * File Name:			NaviLcd.h 
 * Function Describe:	
 * Relate Module:		
 * Writer:				joneming
 * Date:				2012-05-15
 * Rewriter:
 * Date:		
 *******************************************************************************/
#ifndef __NAVILCD_H_
#define __NAVILCD_H_
///////////////////////////
#define USE_NAVILCD
//#define USE_NAVILCD_TTS
/*************************************************************
** 函数名称: NaviLcd_DisposePhoneStatus
** 功能描述: 导航屏处理电话状态命令
** 入口参数: phone:电话号码,status:电话状态：0：为挂断;1:接通；2:新来电
** 出口参数: 无
** 返回参数: 如果是电话呼入状态为新来电时,返回值:0:直接拒接来电;1:正在等待用户接听来电
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char NaviLcd_DisposePhoneStatus(unsigned char *phone,unsigned char status);
/*************************************************************
** 函数名称: NaviLcdSendCmdNewCallPhone
** 功能描述: 新来电命令
** 入口参数: 电话号码
** 出口参数: 无
** 返回参数: 等待接听标志:0:直接拒接来电;1:正在等待用户接听来电
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char NaviLcdSendCmdNewCallPhone(unsigned char *phone);
/*************************************************************
** 函数名称: NaviLcd_ParameterInitialize
** 功能描述: 参数初始化
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcd_ParameterInitialize(void);
/*************************************************************
** 函数名称: NaviLcd_TimerTask
** 功能描述: 内部定时器处理函数
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
FunctionalState NaviLcd_TimerTask(void);
/*************************************************************
** 函数名称: NaviLcdSendCmdNewText
** 功能描述: 文本显示
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcdSendCmdNewText(unsigned char display,unsigned char voice,unsigned char save,unsigned char *buffer,unsigned short datalen);
/*************************************************************
** 函数名称: NaviLcdShowPlatformText
** 功能描述: 文本/调度信息下发
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcdShowPlatformText(unsigned char *buffer,unsigned short datalen);
/*************************************************************
** 函数名称: NaviLcdShowTempText
** 功能描述: 显示临时信息
** 入口参数: 需要显示的临时信息
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcdShowTempText(unsigned char *string);
/*************************************************************
** 函数名称: NaviLcdSendCmdQuestionDisplay
** 功能描述: 提问下发(行标协议,不带问题ID)
** 入口参数: voice 是否播放,buffer源数据首地址,datalen源数据长度
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcdSendCmdQuestionDisplay(unsigned char voice,unsigned char *buffer,unsigned short datalen);
/*************************************************************
** 函数名称: NaviLcdSendCmdQuestionDisplayEx
** 功能描述: 提问下发(标准出租车协议,带问题ID)
** 入口参数: voice 是否播放,buffer源数据首地址,datalen源数据长度
** 出口参数: 
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcdSendCmdQuestionDisplayEx(unsigned char voice,unsigned char *buffer,unsigned short datalen);
/*************************************************************
** 函数名称: NaviLcdSendCmdNewCallTaxi
** 功能描述: 新电召信息命令
** 入口参数: voice 是否播放 0:播放,1:不播放,buffer源数据首地址,datalen源数据长度
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcdSendCmdNewCallTaxi(unsigned char voice,unsigned char *buffer,unsigned short datalen);
/*************************************************************
** 函数名称: NaviLcdSendCmdCallTaxiDetail
** 功能描述: 电召详细信息命令
** 入口参数: voice 是否播放,buffer源数据首地址,datalen源数据长度
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcdSendCmdCallTaxiDetail(unsigned char voice,unsigned char *buffer,unsigned short datalen);
/*************************************************************
** 函数名称: NaviLcdSendCmdCallTaxiPlatformCancel
** 功能描述: 平台取消电召命令
** 入口参数: buffer源数据首地址,datalen源数据长度
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcdSendCmdCallTaxiPlatformCancel(unsigned char *buffer,unsigned short datalen);
/*******************************************************************************
**  函数名称  : NaviLcd_OriginalDataDownTrans
**  函数功能  : 
**  输    入  : 无				
**  输    出  : 
**  全局变量  : 
**  调用函数  : 
**  中断资源  : 
**  备    注  :
*******************************************************************************/
void NaviLcd_OriginalDataDownTrans(u8 *pBuffer, u16 BufferLen);
/*************************************************************OK
** 函数名称: NaviLcdDriverLoginFlag
** 功能描述: 驾驶员登录标志
** 入口参数: 			 
** 出口参数: 为1已登录,为0未登录
** 返回参数: :
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char NaviLcdDriverLoginFlag(void);
/*************************************************************OK
** 函数名称: NaviLcd_RadioProtocolQuestionDownload
** 功能描述: 提问下发
** 入口参数: 			 
** 出口参数: 
** 返回参数: :
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcd_RadioProtocolQuestionDownload(u8 *pBuffer, u16 BufferLen);
/*******************************************************************************
** 函数名称: NaviLcd_CurrentPhoneVolUp
** 函数功能:  音量加加
** 入口参数:  
** 出口参数: 无
** 返回参数:  
** 全局变量: 无
** 调用模块: 无
*******************************************************************************/
void NaviLcd_CurrentPhoneVolUp(void);
/*******************************************************************************
** 函数名称: NaviLcd_CurrentPhoneVolDown
** 函数功能: 音量减小 
** 入口参数:  
** 出口参数: 无
** 返回参数:  
** 全局变量: 无
** 调用模块: 无
*******************************************************************************/
void NaviLcd_CurrentPhoneVolDown(void);
/*******************************************************************************
**  函数名称  : NaviLcd_OpenBrushCard
**  函数功能  : 开启刷卡功能  
**  输    入  : 无				
**  输    出  : 
**  全局变量  : 
**  调用函数  : 
**  中断资源  : 
**  备    注  :
*******************************************************************************/
void NaviLcd_OpenBrushCard(void);
/*******************************************************************************
**  函数名称  : NaviLcd_CloseBrushCard
**  函数功能  : 关闭刷卡功能  
**  输    入  : 无				
**  输    出  : 
**  全局变量  : 
**  调用函数  : 
**  中断资源  : 
**  备    注  :
*******************************************************************************/
void NaviLcd_CloseBrushCard(void);
/*******************************************************************************
**  函数名称  : NaviLcd_DisposeBrushCardDataForDriverInfo
**  函数功能  : 处理刷卡数据
**  输    入  : 无				
**  输    出  : 
**  全局变量  : 
**  调用函数  : 
**  中断资源  : 
**  备    注  :
*******************************************************************************/
void NaviLcd_DisposeBrushCardDataForDriverInfo(unsigned char *data,unsigned short datalen);
/*************************************************************
** 函数名称: NaviLcd_DriverLogout
** 功能描述: 
** 入口参数:  
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char NaviLcd_DriverLogout(void);
/*************************************************************OK
** 函数名称: GetCurrentDriverLoginRecordNumber
** 功能描述: 取得当前驾驶员记录号
** 入口参数: 			 
** 出口参数: 0未登录或不临时驾驶员,未0记录号
** 返回参数: :
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char GetCurrentDriverLoginRecordNumber(void);
/*************************************************************
** 函数名称: NaviLcd_AccStatusOff
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcd_AccStatusOff(void);
/*************************************************************
** 函数名称: NaviLcd_AccStatusOn
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcd_AccStatusOn(void);
/*************************************************************
** 函数名称: NaviLcdSendDataToWriteICcard
** 功能描述: 写卡指令
** 入口参数:  flag保留,type卡类型,buffer卡内容首地址,datalen卡内容长度
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcdSendDataToWriteICcard(unsigned char flag,unsigned char type,unsigned char *buffer,unsigned char datalen);
/*************************************************************
** 函数名称: NaviLcd_WriteDriverInfoToICcard
** 功能描述: 把相关驾驶员信息写到IC中
** 入口参数: recordNumber:驾驶员存储编号(从1开始)
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcd_WriteDriverInfoToICcard(unsigned char recordNumber);
/*************************************************************
** 函数名称: NaviLcdSendCmdModifyPassWord
** 功能描述: 修改密码
** 入口参数: buffer:密码的首地址,且长度4位的长整型数,高位在前
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcdSendCmdModifyPassWord(unsigned char *buffer);
/*************************************************************
** 函数名称: NaviLcdSendCmdReadICcardData
** 功能描述: 发命令去读IC卡内容
** 入口参数:  
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcdSendCmdReadICcardData(void);
/*************************************************************
** 函数名称: NaviLcd_DisposeGNSSCheckUp
** 功能描述:处理北斗送检导航屏指令
** 入口参数: cmd命令
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcd_DisposeGNSSCheckUp(unsigned char cmd);
/*************************************************************
** 函数名称: NaviLcd_SendCmdTerminalTransDataToICCard
** 功能描述: 终端数据透传给刷卡模块
** 入口参数: data需要发送的数据首地址,datalen:需要发送的数据长度
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcd_SendCmdTerminalTransDataToICCard(unsigned char *data,unsigned short datalen);
/*************************************************************
** 函数名称: NaviLcd_DisponseICCardTransDataToTerminal
** 功能描述: 刷卡模块透传给终端数据
** 入口参数: data需要发送的数据首地址,datalen:需要发送的数据长度
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcd_DisponseICCardTransDataToTerminal(unsigned char *data,unsigned short datalen);
/*************************************************************
** 函数名称: NaviLcd_DisponseCtrlUnitTransDataToTerminal
** 功能描述: 控制板透传给终端指令
** 入口参数: data需要发送的数据首地址,datalen:需要发送的数据长度
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcd_DisponseCtrlUnitTransDataToTerminal(unsigned char *data,unsigned short datalen);
/*************************************************************
** 函数名称: NaviLcd_SendCmdTerminalTransDataToCtrlUnit
** 功能描述: 终端透传给控制板指令
** 入口参数: ID类型ID,operate:操作
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcd_SendCmdTerminalTransDataToCtrlUnit(unsigned char ID,unsigned char operate);
/*******************************************************************************
** 函数名称: NaviLcd_SendCarLoadClearSensorResult
** 函数功能: 发送传感器清零结果
** 入口参数:  
** 出口参数: 无
** 返回参数:  
** 全局变量: 无
** 调用模块: 无
*******************************************************************************/
void NaviLcd_SendCarLoadClearSensorResult(void);
/*************************************************************
** 函数名称: NaviLcd_DisponseLoadSensorTransToLcd
** 功能描述: 载重传感器透传给Lcd指令
** 入口参数: data需要发送的数据首地址,datalen:需要发送的数据长度
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcd_DisponseLoadSensorTransToLcd(unsigned char *data,unsigned short datalen);
/*************************************************************
** 函数名称: NaviLcd_DisponseCtrlUnitTransDataToTerminal
** 功能描述: 一键导航
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcd_DisponseOneKeyNavigation(unsigned char *data,unsigned short datalen);
/*******************************************************************************
**  函数名称  : NaviLcd_DisposeTerminalCtrlCarLoadStatus
**  函数功能  : 车辆载货状态 
**  输    入  : status: 0：空载、1：半载、2：满载
**  输    出  : 
**  全局变量  : 
**  调用函数  : SetEvTask(EV_REPORT);
**  中断资源  : 
**  备    注  :
*******************************************************************************/
void NaviLcd_DisposeTerminalCtrlCarLoadStatus(unsigned char status);
/*************************************************************
** 函数名称: NaviLcdSendCmdInfoService
** 功能描述: 信息服务命令
** 入口参数: voice 是否播放,buffer源数据首地址,datalen源数据长度
** 出口参数: 
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcdSendCmdInfoService(unsigned char voice,unsigned char *buffer,unsigned short datalen);
/*************************************************************
** 函数名称: NaviLcdSendCmdCurrentDriverInfo
** 功能描述: 当前驾驶员身份信息命令
** 入口参数: loginFlag登签标志:(0:签到,1:签退); IDNumber证件号码;driverName驾驶员姓名
** 出口参数: 
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcdSendCmdCurrentDriverInfo(unsigned char loginFlag,char *IDNumber,char *driverName);
/*************************************************************
** 函数名称: NaviLcd_GetCurOnlineStatus
** 功能描述: 导航屏在不在线 
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char NaviLcd_GetCurOnlineStatus(void);
/*************************************************************OK
** 函数名称: NaviLcdPowerOn
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/  
void NaviLcdPowerOn(void);
/*************************************************************OK
** 函数名称: NaviLcdPowerOn
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/  
void NaviLcdPowerOff(void);
#endif//__COMMPJA_H_
/*******************************************************************************
 *                             end of module
 *******************************************************************************/

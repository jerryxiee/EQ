#ifndef	__OTHER_H
#define	__OTHER_H

//*************文件包含***************
#include "stm32f2xx.h"
#include "Io.h"
//*************宏定义****************
#define BEEP_ON_EV_ATTEMP		0x01//收到调度信息	
#define BEEP_ON_EV_SPEED		0x02//超速
#define BEEP_ON_EV_TIRED		0x04//疲劳驾驶
#define BEEP_ON_EV_LAND			0x08//未登录(蜂鸣器暂时不鸣叫)
#define	BEEP_ON_EV_SPEED_TIRED		0x06//超速和疲劳驾驶都有
#define BEEP_GPIO_CC                  			GPIOD
#define BEEP_GPIO_Pin                    		GPIO_Pin_3 


#define   PWR_24V()		GPIO_WriteBit(GPIOF, GPIO_Pin_3, Bit_SET)
#define   PWR_12V()		GPIO_WriteBit(GPIOF, GPIO_Pin_3, Bit_RESET)
//************结构体类型******************
//时间控制结构体
typedef  struct
{
	u32  MenuTime;				//记录进入当前菜单的时间
    u32  BeepTime;				//记录蜂鸣器开始鸣叫的时间
    u32  KeyTime;				//记录接收到当前按键的时间
    u8   BeepFlag;				//蜂鸣器状态标志,0为OFF; 1为ON
}TIME_CONTROL_STRUCT;
//注册鉴权任务状态
typedef enum {
  LOGIN_READ_AUTHORIZATION_CODE = 0,//读取鉴权码，每次连接上网络就是此状态
  LOGIN_SEND_LOGIN_CMD,//发送注册命令
  LOGIN_SEND_AUTHORIZATION_CMD,//发送鉴权命令
  LOGIN_CLOSE_GPRS,//断开网络
  LOGIN_DELAY,//延时
  LOGIN_RECONNECT_GPRS,//重连网络
  LOGIN_CLOSE_TASK,//关闭任务
} LOGIN_STATE;  
//*************函数声明**************
/*********************************************************************
//函数名称	:UpdatePeripheralType
//功能		:更新变量Peripheral1Type,Peripheral2Type
//备注		:1.当设置外设1类型或外设2类型参数时需要调用该接口
//              :2.上电启动初始化时需调用该接口 
*********************************************************************/
void UpdatePeripheralType(void);
/*********************************************************************
//函数名称	:ReadPeripheral1TypeBit
//功能		:读外设1类型的某一位
//输入		:第几位，取值为0~15
//返回		:1:表示该外设被启用了;0:表示该外设未被启用
//备注		:参数值大于15返回值为2，表示错误 
*********************************************************************/
u8 ReadPeripheral1TypeBit(u8 Bit);
/*********************************************************************
//函数名称	:ReadPeripheral2TypeBit
//功能		:读外设1类型的某一位
//输入		:第几位，取值为0~15 
//返回		:1:表示该外设被启用了;0:表示该外设未被启用
//备注		:参数值大于15返回值为2，表示错误 
*********************************************************************/
u8 ReadPeripheral2TypeBit(u8 Bit);
/*********************************************************************
//函数名称	:UpdataAccOffGpsControlFlag(void)
//功能		:更新变量AccOffGpsControlFlag
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void UpdataAccOffGpsControlFlag(void);
/*********************************************************************
//函数名称	:UpdataEvaluatorControlFlag(void)
//功能		:更新变量EvaluatorControlFlag
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void UpdataEvaluatorControlFlag(void);
/*********************************************************************
//函数名称	:UpdataTaximeterControlFlag(void)
//功能		:更新变量TaximeterControlFlag
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void UpdataTaximeterControlFlag(void);
/*********************************************************************
//函数名称	:UpdataToplightControlFlag(void)
//功能		:更新变量ToplightControlFlag
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void UpdataToplightControlFlag(void);
/*********************************************************************
//函数名称	:UpdataLedlightControlFlag(void)
//功能		:更新变量LedlightControlFlag
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void UpdataLedlightControlFlag(void);
/*********************************************************************
//函数名称	:EvRing
//功能		:开启电话响铃
//备注		:
*********************************************************************/
void EvRing(void);
/*********************************************************************
//函数名称	:EvRingOver
//功能		:关闭电话响铃
//备注		:
*********************************************************************/
void EvRingOver(void);
void SpeedFlagUpdatePram(void);
void SetBeepOnEvBit(u8 bit);
void SetCurCamID(u8 CurCamID);
FunctionalState Reset_TimeTask(void);  
/*******************************************************************************
* Function Name  : Link1Login_TimeTask
* Description    : 连接1登录任务
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
FunctionalState Link1Login_TimeTask(void);
/*******************************************************************************
* Function Name  : Link2Login_EvTask
* Description    : 连接1登录任务
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
FunctionalState Link2Login_TimeTask(void);
/*******************************************************************************
* Function Name  : SetLink1LoginAuthorizationState
* Description    : 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SetLink1LoginAuthorizationState(u8 State);
/*******************************************************************************
* Function Name  : SetLink2LoginAuthorizationState
* Description    : 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SetLink2LoginAuthorizationState(u8 State);
/*******************************************************************************
* Function Name  : EvGetAkeyTask
* Description    : 获取鉴权码--走SMS短信通道 .
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void GetAkey_EvTask(void);
/*******************************************************************************
* Function Name  : EvChangeServerTask
* Description    : 收到指令后切换到监管平台
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ChangeServer_EvTask(void);
/*******************************************************************************
* Function Name  : EvFtpUpdataTask
* Description    : 利用FTP下载文件
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void FtpUpdata_EvTask(void);
/*******************************************************************************
* Function Name  : EvDiallingTask
* Description    : 一键通话功能
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Dialling_EvTask(void);
/*******************************************************************************
* Function Name  : EvSevDialTask
* Description    : 电话回拨
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SevDial_EvTask(void);
/*********************************************************************
//函数名称	:Updata_SendResultNoteTimeTask
//功能		:发送升级结果通知到平台
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:默认是开启的,1秒调度1次
*********************************************************************/
FunctionalState  Updata_SendResultNoteTimeTask(void);
/*********************************************************************
//函数名称	:Reset_TimeTask
//功能		:系统定时重启
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:
*********************************************************************/
FunctionalState Reset_TimeTask(void);
/*******************************************************************************
* Function Name  : DelayTrigTimeTask(void)
* Description    : 延时触发
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
FunctionalState DelayTrigTimeTask(void);
/*********************************************************************
//函数名称	:GetTerminalAuthorizationFlag(void)
//功能		:获取鉴权标志的值
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:bit0:连接1;bit1:连接2;bit2~bit7:保留,相应位为1表示鉴权成功,否则为失败
*********************************************************************/
u8 GetTerminalAuthorizationFlag(void);
/*********************************************************************
//函数名称	:SetTerminalAuthorizationFlag(u8 channel)
//功能		:置位某个通道的鉴权标志
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:bit0:空缺或预留;bit1:表示CHANNEL_DATA_1，bit2:CHANNEL_DATA_2
//          :channel的值可以为以上两个通道值或者是它们的组合
*********************************************************************/
void SetTerminalAuthorizationFlag(u8 channel);
/*********************************************************************
//函数名称	:ClearTerminalAuthorizationFlag(u8 channel)
//功能		:清除某个通道的鉴权标志
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//备注		:bit0:空缺或预留;bit1:表示CHANNEL_DATA_1，bit2:CHANNEL_DATA_2
//          :channel的值可以为以上两个通道值或者是它们的组合
*********************************************************************/
void ClearTerminalAuthorizationFlag(u8 channel);
/*********************************************************************
//函数名称	:ClearTerminalAuthorizationCode(u8 channel)
//功能		:清除某个通道的鉴权码
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:bit0:空缺或预留;bit1:表示CHANNEL_DATA_1，bit2:CHANNEL_DATA_2
//          :channel的值可以为以上两个通道值或者是它们的组合
*********************************************************************/
void ClearTerminalAuthorizationCode(u8 channel);
/*******************************************************************************
* Function Name  : MultiCenterConnect_TimeTask(void)
* Description    : 多中心连接定时任务,收到设置备用服务器命令时开启,1秒钟调度1次
* Input          : None
* Output         : None
* Return         : 仅仅为行标北斗测试添加
*******************************************************************************/
FunctionalState MultiCenterConnect_TimeTask(void);
/*******************************************************************************
* Function Name  : Communication_Open
* Description    : 开启无线通信
* Input          : None
* Output         : None
* Return         : 行标北斗功能盲区补报测试时,出区域后需要调用这个函数
*******************************************************************************/
void Communication_Open(void);
/*******************************************************************************
* Function Name  : Communication_CloseGprsTimeTask(void)
* Description    : 用于使用前锁定一，主动关闭GPRS
* Input          : None
* Output         : None
* Return         : 仅仅为行标北斗测试添加
*******************************************************************************/
FunctionalState Communication_CloseGprsTimeTask(void);
/*******************************************************************************
* Function Name  : Communication_Close
* Description    : 关闭无线通信
* Input          : None
* Output         : None
* Return         : 行标北斗功能盲区补报测试时,进区域后需要调用这个函数
*******************************************************************************/
void Communication_Close(void);
void Communication_Open(void);
void Communication_Close2(void);
//为编译不报警告而添加，已实际各模块使用为准，在实际使用时该定义可以直接去掉 dxl
u8 SpeedMonitor_GetCurSpeedType(void);
void SpeedMonitor_UpdatePram(void);
void TiredDrive_UpdatePram(void);
void NaviLcdShowTempText(u8 *pBuffer);//在显示屏显示一行文本信息
/********************************************************************
* 名称 : Print_RTC
* 功能 : 截取当前的RTC时间,打印输出
********************************************************************/
void Print_RTC( void );
/********************************************************************
* 名称 : SetCostDownFlag
* 功能 : 设置CostDown版本标志位，该位为1表示PCB为降成本版本，为0表示之前的版本
********************************************************************/
void SetCostDownVersionFlag( void );
/********************************************************************
* 名称 : ClearCostDownFlag
* 功能 : 清除CostDown版本标志位，该位为1表示PCB为降成本版本，为0表示之前的版本
********************************************************************/
void ClearCostDownVersionFlag( void );
/********************************************************************
* 名称 : GetCostDownFlag
* 功能 : 读取CostDown版本标志位，该位为1表示PCB为降成本版本，为0表示之前的版本
********************************************************************/
u8 GetCostDownVersionFlag( void );
/********************************************************************
* 名称 : GansuOiL_OilProtocolCheck
* 功能 : 甘肃电信油耗协议检测,油耗传感器30秒发送一条数据给GPS终端
********************************************************************/
u8 GanSuOil_OilProtocolCheck(u8 *pBuffer, u8 length);
/********************************************************************
* 名称 : GanSuOiL_GetOnePacketData
* 功能 : 获取一包油耗数据，位置汇报时调用该函数
********************************************************************/
u8 GanSuOil_GetOnePacketData(u8 *pBuffer);
/********************************************************************
* 名称 : GanSuOiL_GetCurPacketLen
* 功能 : 获取当前油耗数据包长度
********************************************************************/
u8 GanSuOil_GetCurPacketLen(void);

/********************************************************************
* 名称 : Acceleration_DisposeRadioProtocol
* 功能 : 急加速急减速指令解析
********************************************************************/
void Acceleration_DisposeRadioProtocol(u8 cmd, u8 *pBuffer, u16 length);
/********************************************************************
* 名称 : Acceleration_Turn_DisposeRadioProtocol
* 功能 : 急转弯指令解析
********************************************************************/
void Acceleration_Turn_DisposeRadioProtocol(u8 cmd, u8 *pBuffer, u16 length);

extern u8      BdTestFlag;//北斗测试标志,对应于1.使用前锁定;2.自检;.....
extern u16     BdTestCount1;//北斗测试计数,每次更换类型BdTestFlag都会清0
extern u16     BdTestCount2;//北斗测试计数,每次更换类型BdTestFlag都会清0
extern u8	ResetEnableFlag;//重启使能标志
extern u8      Ftp_Updata_BusyFlg;//ftp 升级忙标志   0：空闲   1：升级中


#define	TACHOGRAPHS_19056_TEST	0	//行驶记录仪测试时设置为大于0的值，出货时置0
#if	TACHOGRAPHS_19056_TEST
void Tachographs_Test_Speed_Reduce(void);
u8 Tachographs_Test_Get_Speed(void);
#endif
#endif

/*******************************************************************************
 * File Name:			AreaManage.h 
 * Function Describe:	
 * Relate Module:		区域功能
 * Writer:				Joneming
 * Date:				2012-09-17
 * ReWriter:			
 * Date:				
 *******************************************************************************/
#ifndef __AREAMANAGE_H_
#define __AREAMANAGE_H_
///////////////////
typedef enum
{
    AREA_PROTOCOL_TYPE_EMPTY,       //协议出错
    AREA_PROTOCOL_TYPE_STANDARD,    //标准协议
    AREA_PROTOCOL_TYPE_JIUTONG,     //九通,14.15位为语音
    AREA_PROTOCOL_TYPE_SUOMEI,      //索美,8,9,10,11,12,13
    AREA_PROTOCOL_TYPE_MAX
}E_AREA_PROTOCOL_TYPE;
///////////////////////////////////
typedef enum
{
    AREA_STATUS_IN, 
    AREA_STATUS_OUT,
    AREA_STATUS_MAX
}E_AREA_STATUS;
//区域位类型
typedef enum{
    E_AREA_MOMENT=0,                //0时刻
    E_AREA_LIMIT_SPEED,             //1限速
    E_IN_SENDTO_DRIVER,             //2进区报警给司机
    E_IN_SENDTO_SERVER,             //3进区报警给平台
    E_OUT_SENDTO_DRIVER,            //4出区报警给司机
    E_OUT_SENDTO_SERVER,            //5出区报警给平台
    E_AREA_NORTH,                   //6北南
    E_AREA_EAST,                    //7东西
    E_AREA_PROHIBIT_OPENDOOR,       //8位:1:禁止开门
    E_AREA_ENTER_TURNOFF_GPRS = 14, //14位:1：进区域关闭通信模块
    E_AREA_ENTER_COLLECT_GNSS,      //15位:1：进区域采集GNSS详细定位数据 
}E_AREA_FLAG;

typedef enum{
    E_USER_IN_PLAY = 0,             //0进区域播语音提示
    E_USER_OUT_PLAY,                //1出区域播语音提示
    E_USER_SPEED_PHOTO = 8,         //8启动限速拍照
    E_USER_RETENTION,               //9区域滞留限制功能
    E_USER_PRE_LIMIT_SPEED,         //10限速预警提示,可以与限速分开使用
    E_USER_LEARN_ARAE,              //11场内学习区域
    E_USER_VALID_LEARN_ARAE,        //12有效学习区域
    E_USER_NO_CHANGE_LIMIT_SPEED,   //13出区域不更改限速值
}E_AREA_USER_FLAG;
//////////////////////////////
//点坐标结构
typedef struct
{
    unsigned long Lat; //纬度整型坐标
    unsigned long Long;//经度整型坐标
}T_GPS_POS;
/*************************************************************
** 函数名称: Area_ParameterInitialize
** 功能描述: 区域参数初始化
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Area_ParameterInitialize(void);  //区域初始化
/*************************************************************
** 函数名称: Area_EraseAllArea
** 功能描述: 清除区域
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Area_EraseAllArea(void) ;  //清空区域
/*************************************************************
** 函数名称: Area_EraseFlash
** 功能描述: 清空区域
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Area_EraseFlash(unsigned short Start,unsigned short End);//
/*************************************************************
** 函数名称: AreaManage_TimeTask
** 功能描述: 区域功能定时器处理函数
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
FunctionalState AreaManage_TimeTask(void) ; //区域管理
/*************************************************************
** 函数名称: Area_SetRectArea
** 功能描述: :设置矩形区域
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Area_SetRectArea(unsigned char *pBuffer, unsigned short BufferLen);
/*************************************************************
** 函数名称: Area_DelRectArea
** 功能描述: :删除矩形区域
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Area_DelRectArea(unsigned char *pBuffer, unsigned short BufferLen);
/*************************************************************
** 函数名称: Area_SetRoundArea
** 功能描述: :设置圆形区域
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Area_SetRoundArea(unsigned char *pBuffer, unsigned short BufferLen);
/*************************************************************
** 函数名称: Area_DelRoundArea
** 功能描述: :删除圆形区域
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Area_DelRoundArea(unsigned char *pBuffer, unsigned short BufferLen);
/*************************************************************
** 函数名称: Area_GetInOutAreaAlarmSubjoinInfo
** 功能描述: 获得进出区域报警附加信息包(包括附加信息ID、长度、附加信息体) :即附加信息ID为0x12的所有内容
** 入口参数: 需要保存进出区域报警附加信息包的首地址
** 出口参数: 
** 返回参数: 进出区域报警附加信息包的总长度,0:表示未进出区域
*************************************************************/
unsigned char Area_GetInOutAreaAlarmSubjoinInfo(unsigned char *data);
//-----------------------------------------------
//功能:计算点到点距离
//输入:指向两个定位点结构的指针
//输出:米
//备注:
//-----------------------------------------------
unsigned long CalDotDotMile(T_GPS_POS *gpsdataSrc, T_GPS_POS *gpsdataDest);
/********************************************************************
* 功能 : 计算点到线距离
* 输入: 指向三个定位点结构的指针
* 输出: 米
* 全局变量: 
*
* 备注: (修改记录内容、时间)
********************************************************************/
unsigned long CalDotLineMile(T_GPS_POS *gpsdataSrc, T_GPS_POS *gpsdataDest1,T_GPS_POS *gpsdataDest2);
//-----------------------------------------------
//功能:判断点是否在矩形区域内
//输入:3点坐标
//输出:0:在区域内  1:不在 0XFF:出错
//备注:
//-----------------------------------------------	
unsigned char ComparePos(T_GPS_POS *LeftUpPos,T_GPS_POS *RightDownPos, T_GPS_POS *CurPos);
/********************************************************************
* 功能 : 修改Flash里的数据
* 输入: 
* 输出: 
* 全局变量: 
*
* 备注: (修改记录内容、时间)
********************************************************************/
void Area_MdyDataInFlash(unsigned char *DataPtr,unsigned long addr,unsigned short len);
//-----------------------------------------------
//功能:清空区域
//输入:
//输出:
//备注:
//-----------------------------------------------
void Area_EraseFlash(unsigned short Start,unsigned short End);
//-----------------------------------------------
//功能:BCD格式时间,传成HEX
//输入:1,指向BCD格式时刻的指针，2，类型(0:全部 1:仅转化时分秒)
//输出:
//备注:
//-----------------------------------------------
unsigned long Area_ConverseBcdGmtime(unsigned char *Ptr, unsigned char Type);
//-----------------------------------------------
//功能:BCD格式时间,传成HEX
//输入:1,指向BCD格式时刻的指针，2，类型(0:全部 1:仅转化时分秒)
//输出:
//备注:
//-----------------------------------------------
unsigned long Area_GetConvertCurTime(unsigned char Type);
/*************************************************************
** 函数名称: Area_GetConverseBCDTimeType
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
unsigned char Area_GetConverseBCDTimeType(unsigned char *buffer);
/*************************************************************
** 函数名称: Area_DisposeEIExpandProtocol
** 功能描述: 处理区域伊爱扩展协议
** 入口参数: pBuffer:消息体内容首地址,BufferLen消息体内容长度
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Area_DisposeEIExpandProtocol(unsigned char *pBuffer, unsigned short BufferLen);
/*************************************************************
** 函数名称: Area_SetInOutAreaAlarmInfo
** 功能描述: 设置区域报警信息
** 入口参数: type:报警类型,AlarmID:报警ID,Orientation方向
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/  
void Area_SetInOutAreaAlarmInfo(unsigned char type,unsigned long AlarmID,unsigned char Orientation);
/*************************************************************
** 函数名称: Area_ClearInOutAreaAlarmBit
** 功能描述: 清除区域报警信息
** 入口参数: type区域类型
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/  
void Area_ClearInOutAreaAlarmBit(unsigned char type);
/*************************************************************
** 函数名称: Area_ClearOpenDoorAlarmBit
** 功能描述: 清除开门报警位
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/  
void Area_ClearOpenDoorAlarmBit(unsigned char type);
/*************************************************************
** 函数名称: Area_CheckOpenDoorAlarm
** 功能描述: 检查开门报警
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/  
void Area_CheckOpenDoorAlarm(unsigned char type);
/*************************************************************
** 函数名称: Area_GetCurSpeed
** 功能描述: 取得当前速度
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: SpeedMonitor_GetCurSpeed
*************************************************************/
unsigned char Area_GetCurSpeed(void);
/*************************************************************OK
** 函数名称: Area_GetCollectGnssFlag
** 功能描述: 获取当前Gnss是否需要采集标志
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/  
unsigned char Area_GetCollectGnssFlag(void);
/*************************************************************OK
** 函数名称: Area_GetCloseGPRSFlag
** 功能描述: 获取当前关闭通讯模块标志
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/  
unsigned char Area_GetCloseGPRSFlag(void);
/*************************************************************OK
** 函数名称: Area_GetCollectGnssFlag
** 功能描述: 获取当前是否在区域内标志
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/  
unsigned char Area_GetInAreaFlag(void);
/*************************************************************
** 函数名称: Area_GetRectAreaTotalNum
** 功能描述: 获得矩形区域总数
** 入口参数: 
** 出口参数: 
** 返回参数: 矩形区域总数
*************************************************************/ 
unsigned char Area_GetRectAreaTotalNum(void);
/*************************************************************
** 函数名称: Area_GetRoundAreaTotalNum
** 功能描述: 获得圆形区域总数
** 入口参数: 
** 出口参数: 
** 返回参数: 圆形区域总数
*************************************************************/ 
unsigned char Area_GetRoundAreaTotalNum(void);
#endif

/*******************************************************************************
 *                             end of module
 *******************************************************************************/
 
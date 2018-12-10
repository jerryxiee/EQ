
#ifndef _RECORDER_H
#define _RECORDER_H

#include<stddef.h>
#include<stdio.h>
#include<string.h>

/**************************采集数据命令字列表*************************************/
#define CMD_GET_PERFORM_VERSION                 0x00 /*采集记录仪执行标准版本    */
#define CMD_GET_DRIVER_INFORMATION              0x01 /*采集当前驾驶员信息        */
#define CMD_GET_SYS_RTC_TIME                    0x02 /*采集记录仪实时时间        */
#define CMD_GET_ADD_MILEAGE                     0x03 /*采集累计行驶里程          */
#define CMD_GET_IMPULSE_RATIO                   0x04 /*采集记录仪脉冲系数        */
#define CMD_GET_CAR_INFORMATION                 0x05 /*采集车辆信息              */
#define CMD_GET_STATUS_SIGNAL_CONFIG            0x06 /*采集记录仪状态信号配置信息*/
#define CMD_GET_UNIQUE_ID                       0x07 /*采集记录仪唯一性编号      */
#define CMD_GET_SPECIFY_RECORD_SPEED            0x08 /*采集指定的行驶速度记录    */
#define CMD_GET_SPECIFY_RECORD_POSITION         0x09 /*采集指定的位置信息记录    */
#define CMD_GET_SPECIFY_RECORD_ACCIDENT_DOUBT   0x10 /*采集指定的事故疑点记录    */
#define CMD_GET_SPECIFY_RECORD_OVERTIME_DRIVING 0x11 /*采集指定的超时驾驶记录    */
#define CMD_GET_SPECIFY_RECORD_DRIVER_IDENTITY  0x12 /*采集指定的驾驶人身份记录  */
#define CMD_GET_SPECIFY_RECORD_EXTERN_POWER     0x13 /*采集指定的外部供电记录    */
#define CMD_GET_SPECIFY_RECORD_CHANGE_VALUE     0x14 /*采集指定的参数修改记录    */
#define CMD_GET_SPECIFY_LOG_STATUS_SPEED        0x15 /*采集指定的速度状态日志    */
/*16H~~~~17H预留                                                                 */


/**************************设置参数命令字列表*************************************/
#define CMD_SET_CAR_INFORMATION                 0x82 /*设置车辆信息              */
#define CMD_SET_CAR_INIT_INSTALL_TIME           0x83 /*设置记录仪初次安装日期    */
#define CMD_SET_STATUS_SIGNAL_CONFIG            0x84 /*设置状态量配置信息        */
#define CMD_SET_RECORDER_TIME                   0xC2 /*设置记录仪时间            */
#define CMD_SET_IMPULSE_RATIO                   0xC3 /*设置记录仪脉冲系数        */
#define CMD_SET_INIT_MILEAGE                    0xC4 /*设置初始化里程            */
/*0C5H~~0CFH预留                                                                 */


/**************************鉴定命令字列表*****************************************/
#define CMD_IDENTIFY_INTO                       0xE0 /*进入或保持鉴定状态        */
#define CMD_IDENTIFY_MEASURE_MILEAGE            0xE1 /*进入里程误差测量          */
#define CMD_IDENTIFY_MEASURE_IMPULSE_RATIO      0xE2 /*进入脉冲系数误差测量      */
#define CMD_IDENTIFY_MEASURE_SYS_RTC_TIME       0xE3 /*进入实时时间误差测量      */
#define CMD_IDENTIFY_RETURN                     0xE4 /*返回正常工作状态          */
/*0E5~~0EFH预留                                                                  */

/**************************命令字名称列表*****************************************/
#define CMD_NAME_VERSION                 ("执行标准版本年号  ")              
#define CMD_NAME_DRIVER_MSG              ("当前驾驶人信息    ")
#define CMD_NAME_RTC                     ("实时时间          ")
#define CMD_NAME_MILEAGE                 ("累计行驶里程      ")
#define CMD_NAME_IMPULSE_RATIO           ("脉冲系数          ")
#define CMD_NAME_CAR_MSG                 ("车辆信息          ")
#define CMD_NAME_STATUS_MSG              ("状态信号配置信息  ")
#define CMD_NAME_ONLY_ID                 ("记录仪唯一性编号  ")
#define CMD_NAME_SPEED                   ("行驶速度记录      ")
#define CMD_NAME_POSITION                ("位置信息记录      ")
#define CMD_NAME_DOUBT                   ("事故疑点记录      ")
#define CMD_NAME_OVERTIME                ("超时驾驶记录      ")
#define CMD_NAME_DRIVER                  ("驾驶人身份记录    ")
#define CMD_NAME_POWER                   ("外部供电记录      ")
#define CMD_NAME_VALUE                   ("参数修改记录      ")
#define CMD_NAME_SPEED_STA               ("速度状态日志      ")


/**************************声明***************************************************/


typedef struct _RECORDER_QUEUE_RX {                  /*记录仪接收队列结构体      */
	u16  in;
	u16  out;
	u8  nState;
	u8  nChannel;                          /*接收通道 0：RS232通道   非0：GSM通道*/
	u8  nBuf[1024];
}RECORDER_QUEUE_RX;

typedef struct _RECORDER_QUEUE_TX {                  /*记录仪发送队列结构体      */
	u16 in;
	u16 out;
	u8  nState;
	u8  nBuf[1024];
}RECORDER_QUEUE_TX;

typedef  struct _ST_RECORDER_WIRE {                  /*记录仪无线采集数据结构体   */
    u8  DataBlock;      //数据块标志,0x03无AA 75,否则含有AA 75
	u16 AckNum;         //应答流水号,对应平台下发的
	u8  Cmd;            //命令字
	u16 Cmd808;         //平台下发的指令，用于判断错误类型
	TIME_T Time_start;  //开始时间
	TIME_T Time_end;    //结束时间
	u16 Max;            //平台下发的最大块数
	u16 MaxCnt;         //最大块数计数器
	u16 SendTotal;      //无线发送一次数据的最大数据块数
    u16 AllCnt;         //分包处理时的总包数
    u16 AllRemainCnt;   //分包处理时的总包数最后一包的余数
    u16 AddOverflg;     //如果最大数据块数比总块数小，则置位
    u16 AddCnt;         //分包处理时的当前包数
    u16 BlockNum;       //指定时间段内的总数据块数
    u8  All_07;         //一次性全部发送0-7数据标志，目前该标志只针对北京过检使用
    u16 Len;
	u8  Buf[1024];
	u8  PatchNum;//8003下发的重传包数
	u8  PatchCnt;//8003下发的重传包数计数
	u16 PatcBuf[50];//最大支持50个丢包
#ifdef HUOYUN
    u16 HY_Cmd_SendNum;//搜索的数据块数，用于发送定长900字节，货运平台检测专用
    u16 HY_Cmd_SendRemain;//不足900后的余数
#endif

}ST_RECORDER_WIRE;

typedef  struct _ST_RECORDER_HEAD {//记录仪数据头，用于协议组包                 
    u8  acknum808H;//808协议中的应答流水号
    u8  acknum808L;
    u8  cmd808;//808协议中的命令字    
    u8  head1;//55
    u8  head2;//7a    
    u8  cmd;//命令字
    u8  lenH;//数据块长度
    u8  lenL;
    u8  back;//备用字
    u8  DataBuf[1024];//数据块数据

}ST_RECORDER_HEAD;

typedef  struct _ST_RECORDER_FLASH {//记录仪数据头，用于协议组包                 
    u8  time[4];//32位时间
    u8  buf[250];//从flash中读取的数据
    u8  ver;//校验

}ST_RECORDER_FLASH;

typedef  struct  {    

    u16  start[1];       //开始字符
    u16  time_ymd[6];    //年月日
    u16  separator_1[1]; //分隔符
    u16  time_hm[4];     //时分
    u16  separator_2[1]; //分隔符
    u16  num[20];        //车牌号码
    u16  ftype[4];       //文件类型即后缀名, .VDR
    
}RECORDER_FILE_VDR_NAME;//记录仪数据USB文件名结构体

typedef  struct  {    

    u8  cmd;       //数据代码即记录仪命令字
    u8  name[18];  //名称
    u8  length[4]; //长度
    u8  buf[700];  //数据
    
}RECORDER_FILE_VDR_DATA;//记录仪数据USB文件名结构体

typedef  struct  {    

	u16	Sector  ; //扇区
	u16	Step    ; //步数
    u16	StepLen ; //步长 
    
}RECORDER_FLASH_ADDR;//记录仪数据在flash中存储的地址
/**************************外部引用********************************************/

extern ST_RECORDER_WIRE   St_RecorderWire;

/*******************************************************************************
* Function Name  : RecorderWired_TimeTask
* Description    : 记录仪事件处理,100ms调用一次
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
FunctionalState RecorderWired_TimeTask(void);
/*********************************************************************
//函数名称	:RecorderCom_WirelessTimeTask
//功能		:无线行驶记录仪定时任务
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
FunctionalState RecorderWireless_TimeTask(void);
/*******************************************************************************
* Function Name  : Recorder_USBHandle
* Description    : 行驶记录仪USB数据处理，主要用于设置标志位
* Input          : None            
* Output         : None
* Return         : None
*******************************************************************************/
void Recorder_USBHandle(void);
/*******************************************************************************
* Function Name  : Recorder_MileageTest
* Description    : 记录仪鉴定命令->里程误差,需每秒调用一次
* Input          : None            
* Output         : None
* Return         : None
*******************************************************************************/
void Recorder_MileageTest(void);
/*********************************************************************
//函数名称	:Recorder_GetCarInformation
//功能		:获取车辆信息，包括车辆VIN，车牌号码，车牌分类
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:命令字06H
*********************************************************************/
u8 Recorder_GetCarInformation(u8 *pBuffer);
/*******************************************************************************
* Function Name  : Recorder_Stop15MinuteSpeed
* Description    : 停车前15分钟速度，内容包括VIN、停车时间、停车前15分钟平均速度
* Input          : *p  : 指向15分钟平均速度信息打包数据              
* Output         : None
* Return         : 返回数据长度
*******************************************************************************/
u8 Recorder_Stop15MinuteSpeed(u8 *p);
/**
* @brief  根据开始时间结束时间计算时间段内总包数或提取指定数据包数据
* @param  tpye:查询类型
* @param  Start:开始时间
* @param  End:结束时间
* @param  PackNum:要查找的数据包号，只有pData != NULL时才有效
* @param  *pData:指向提取的数据地址；
* @retval 当pData==NULL时总数据包数，否则返回数据长度
*/
u16 RecorderDataPack(u8 Type, TIME_T Start, TIME_T End,u16 PackNum,u8 *pData);
/**
* @brief  根据开始时间结束时间计算时间段内总包数或提取指定数据包数据,USB专用
* @param  tpye:查询类型
* @param  Start:开始时间
* @param  End:结束时间
* @param  PackNum:要查找的数据包号，只有pData != NULL时才有效
* @param  *pData:指向提取的数据地址；
* @retval 当pData==NULL时总数据包数，否则返回数据长度
*/
u16 RecorderDataPackUsb(u8 Type, TIME_T Start, TIME_T End,u16 PackNum,u8 *pData);
/**
* @brief  记录仪命令转成FLASH存储命令
* @param  tpye:记录仪命令
* @retval 返回FLASH存储命令
*/
u8 RecorderCmdToRegister(u8 Type);
/*********************************************************************
//函数名称	:RecorderPatchGet
//功能		:无线行驶记录仪接收8003指令数据
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/

void RecorderPatchGet(u8 *pRec, u16 RecLen);
/*******************************************************************************
* Function Name  : Recorder_SendData
* Description    : 发送记录仪反馈数据
* Input          : - *pData ：指向要发送数据的地址
*                  - Len    :发送的数据的长度             
* Output         : None
* Return         : 成功返回真 否则返回假
*******************************************************************************/
u8 Recorder_SendData(u8 *pData, u16 Len);

/**
* @brief  GPRS发送数据的总包数
* @param  tpye:查询类型
* @param  Start:开始时间
* @param  End:结束时间
* @retval 总数据包数
*/
u16 RecorderWireSendNum(u8 Type, TIME_T Start, TIME_T End,u16 *pRemain);
u16 RecorderWireSend(u8 type,u16 Count,u8 *pData);
//参数修改，平台参数修改时调用
void Recorder_ParaSet(u8 cmd);

#endif




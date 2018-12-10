/*************************************************************************
* Copyright (c) 2013,深圳市伊爱高新技术开发有限公司
* All rights reserved.
* 文件名称 : Module_CAN.h
* 当前版本 : 1.0
* 开发者   : Shigle
* 修改时间 : 20113年6月10日
*************************************************************************/


#ifndef _MODULE_CAN_H_
#define _MODULE_CAN_H_


/*****  宏定义区   *****/



#define DEBUG_PRINT_CAN  0 //CAN任务日志打印


#define CAN_PWR_ON()    GPIO_WriteBit(GPIOD, GPIO_Pin_15, Bit_SET)
#define CAN_PWR_OFF()   GPIO_WriteBit(GPIOD, GPIO_Pin_15, Bit_RESET)


#define iCAN1   0
#define iCAN2   1
#define CAN_CH_MAX 1 //CAN通道最多数 1:只开启CAN1 2:同时开启CAN1和CAN2

#define  CAN_MAX_SEND_LEN    3// 35
#define  CAN_MAX_RECE_NUM    80 //20// 20 //接收缓冲最大不超过40条.


#define MAX_NUM_FILTER_ID  2 //每路CAN通道 过滤的ID参数个数




//---------- 以下为BASICCAN SJA1000的错误字 ----------
/*
#define     CAN_INTERFACE_0K    0                            //SJA1000接口正常
#define     CAN_BUS_0K          0                          //CAN总线工作正常
#define     CAN_INTERFACE_ERR   0XFF                         //SJA1000接口错误?
#define     CAN_QUITRESET_ERR   0XFD                         //SJA1000不能退出复位模式
#define     CAN_INITOBJECT_ERR  0XFC                         //初始化报文验收滤波器错
#define     CAN_INITBTR_ERR     0XFB                         //初始化总线定时器器错
#define     CAN_INITOUTCTL_ERR  0XFA                         //初始化输出控制错误
#define     CAN_INITSET_ERR     0XF9                         //初始化时钟分频控制器错
#define     CAN_BUS_ERR         0XF8                         //SJA1000发送数据错

#define  CAN_SEND_OK    0
#define  CAN_SEND_ERR   1


*/


/*****  结构体定义区   *****/

/*****  定义 CAN地过滤ID结构体   *****/

typedef struct {
	u8  iMaskID;
	u32 MaskID;
}CAN_MASK_ID;

#pragma pack(1)
typedef struct{
  u32 Can_ID;
  u8  Can_CtlCode;
  u8  CanChar[8];
}CAN_DATA;
#pragma pack()



typedef struct {
	u8 data[100];	//数据
	u8 snd;			//已发送
	u8 len;			//总长度
}CAN_REC;



/*****  定义 CAN参数结构体   *****/

#pragma pack(1)
typedef struct{
  u8  CAN_ID_BYTE_3;
  u8  CAN_ID_BYTE_2;
  u8  CAN_ID_BYTE_1;
  
  u8  CAN_ID_BYTE_0: 5;     //28字节CAN总线ID
  u8  CAN_Data_Original :1; //bit 29: 表示数据采集方式: 0:原始数据 1: 采集区间的计算值
  u8  CAN_Frame_Type: 1; //bit 30: 表示帧类型: 0:标准帧 1:扩展帧
  u8  CAN_Channel: 1; //bit31: 表示CAN通道号, 0: CAN1 1: CAN2
   
  u32 CAN_Sample_Interval;

}PARAMETER_0110;
#pragma pack()






typedef struct tagPARAMETER_CAN{
u16 CAN_UploadPeriodxS; //CAN总线通道1上传时间间隔(s), 0表示不上传
u16  UploadTimeCnt;
u8   FlagUplodTimeArrived;
u32 CAN_SamplePeriodxMS; //CAN总线通道1采集时间间隔(ms) , 0表示不采集
u16  SampleTimeCnt;
u8   FlagSampleTimeArrived;
u32 CAN_Filter_ID_Index;

}PARAMETER_CAN , * PPARAMETER_CAN;


typedef struct tagPARAMETER_CAN_SINGLE{
PARAMETER_0110  CAN_Sample_Single[ MAX_NUM_FILTER_ID ];
u8    CAN_Sample_Single_Num;
}PARAMETER_CAN_SINGLE , * PPARAMETER_CAN_SINGLE;


#pragma pack(1)
typedef struct tagCAN_ITEM{
   u8  CAN_ID_BYTE_3: 5;  //28字节CAN总线ID
  u8  CAN_Data_Original :1; //bit 29: 表示数据采集方式: 0:原始数据 1: 采集区间的计算值
   u8  CAN_Frame_Type: 1; //bit 30: 表示帧类型: 0:标准帧 1:扩展帧
 u8  CAN_Channel: 1; //bit31: 表示CAN通道号, 0: CAN1 1: CAN2

  
  u8  CAN_ID_BYTE_2;
  u8  CAN_ID_BYTE_1;
  u8  CAN_ID_BYTE_0;

  u8  CAN_Data[8];
  u8  CAN_Time[5];
  
}CAN_ITEM;
#pragma pack()




typedef struct
{
  
u8   receIndex;        //接收指标; 每次接收了一个完整的消息或者将消息取出 都会加1;
u8   UnreadIndex;      //接收到的数据但未读指示;
u8   receNum;          //接收数据包个数(单位:13的倍数)
u8   sendLen;         //发送的数据长度(单位:13的倍数)
u8   sendIndex;     //发送数据的索引(单位:13的倍数)
u8   sendBuffer[CAN_MAX_SEND_LEN][13];	//发送 到总线数据的缓冲
CAN_ITEM  receBuffer[CAN_MAX_RECE_NUM];

}CAN_ISR_DATA;






/*****  函数 声明区   *****/


/********************************************************************
* 名称 : DecodeCan_EvTask
* 功能 : 解析CAN接收的数据包
********************************************************************/
extern void DecodeCan_EvTask();

/********************************************************************
* 名称 : Time_Now_Get_BCD
* 功能 : 截取当前的RTC时间,转化成协议要求的BCD格式
* 输出 : 接收BCD时间的指针
********************************************************************/
extern void Time_Now_Get_BCD( u8 * pTimer );

/********************************************************************
* 名称 : CAN_Isr_Rx
* 功能 : 采集CAN接收中断的数据
* 输入 :  * CANx  : STEM32 CAN结构体指针
********************************************************************/

extern void CAN_Isr_Rx( CAN_TypeDef* CANx );

/*******************************************************************
* 名称 : CAN1_Sample_Sw
* 功能 : CAN 采样开关
* 输入 : 
*       iCANChannel:    0: CAN1通道   1: CAN2通道
*            bSwCtr:    0: 关闭CAN接收中断  1: 开启CAN1接收中断   
*******************************************************************/
extern void CAN_Sample_Sw( u8 iCANChannel , u8 bSwCtr );

/*******************************************************************
* 名称 : CAN_Packet_Single_ID
* 功能 : 打包CAN数据
* 输入 : 
*       iCANCh:    0: CAN1通道   1: CAN2通道
* 输出 :
*        pdata:    接收包数据的指针
*******************************************************************/
extern u16 CAN_Packet_Single_ID( u8 iCANCh,  u8 *pdata );


/*******************************************************************
* 名称 : 参数ID0x0110~0x01FF  总线ID 单独采集参数更新
* 功能 : CAN 总线ID 单独采集设置
* 输入 :  CANParmamerID : CAN参数ID  目前只能取值0x0110和 0x0111
*******************************************************************/
extern void UpdataOnePram_Sample_Single(  u32 CANParmamerID  );


/********************************************************************
* 名称 : Can_TimeTask
* 功能 : 初始化系统任务之 CAN
********************************************************************/
extern void CAN_TimeTask_Init(void);


/********************************************************************
* 名称 : Can_TimeTask
* 功能 : 系统任务之 CAN
********************************************************************/
extern FunctionalState  Can_TimeTask(void);

/*******************************************************************
* 名称 : 参数ID0x0110~0x01FF  总线ID 单独采集参数更新
* 功能 : CAN 总线ID 单独采集设置
* 输入 :  CANParmamerID : CAN参数ID  目前只能取值0x0110和 0x0111
*******************************************************************/
extern  void UpdataOnePram_Sample_Single(  u32 CANParmamerID  );





#endif

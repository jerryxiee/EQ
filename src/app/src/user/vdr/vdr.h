/*
********************************************************************************
*
*
* Filename      : vdr.h
* Version       : V1.00
* Programmer(s) : miaoyahan
* Time          : 20140722
********************************************************************************
*/

/*
********************************************************************************
*                               MODULE
*
* Note(s) : 
********************************************************************************
*/

#ifndef _VDR_H
#define _VDR_H


/*
********************************************************************************
*                               INCLUDES
********************************************************************************
*/
#include "include.h"

/*
********************************************************************************
*                            DEFINES VALUE
********************************************************************************
*/
#define   PRO_DEF_RETAINS        (0x12) //记录仪保留字节
#define   VDR_CCC_CODE           "C000116"
#define   VDR_PRO_MODEL          "EGS701          "

#define   VDR_MEM_READ           0
#define   VDR_MEM_WRITE          1

#define	  VDR_SECTOR_SIZE		 4096//扇区大小
#define	  VDR_PACKAGE_SIZE       1000//分包大小
#define	  VDR_STEP_SIZE          252//最大单步缓存
#define   VDR_WIRE_ADD_SIZE      300//注意类型为16BIT
/*
********************************************************************************
*                               DEFINES
********************************************************************************
*/


/*
********************************************************************************
*                                 ENUM
********************************************************************************
*/
typedef enum {
    VDR_COM=0, //串口通道
    VDR_WIRE_1,//数据通道1
    VDR_WIRE_2,//数据通道2
}VDR_CH;//VDR数据类型

/**
  * @brief  记录仪错误类型
  */
typedef enum
{
  VDR_ERR_NO=0,//正确返回 
  VDR_ERR_VF,  //校验错误
  VDR_ERR_CMD, //非法命令
  VDR_ERR_HEAD,//非记录仪命令
  
} VDR_ERR;

/**
  * @brief  记录仪存储参数
  */
typedef enum
{
  VDR_MEM_DVRINFOR  = (0x00),
  VDR_MEM_TIMEINIT  ,    
  VDR_MEM_MILEINIT  ,  
  VDR_MEM_MILEADD   ,
  VDR_MEM_PULSE     , 
  VDR_MEM_CARCODE   ,
  VDR_MEM_CARPLATE  ,
  VDR_MEM_CARTYPE   ,
  VDR_MEM_STASIGNAL ,
  VDR_MEM_ONLY      ,
  VDR_MEM_RTC       ,  
  
} VDR_MEM;

/*
********************************************************************************
*                                STRUCT
********************************************************************************
*/
typedef struct 		
{
    u8  cmd;   //命令字
    u16 len;   //长度
    u8  back;  //备用字
    u8  vef;   //校验
    u8  *pbuf; //数据

}VDR_HEAD;//记录仪数据头

typedef struct 		
{
    u32    cnt_start;    //32位开始时间
    u32    cnt_end;      //32位结束时间
    u8     bcd_start[6]; //bcd码开始时间
    u8     bcd_end[6];   //bcd码结束时间
    TIME_T t_start;      //时间结构体开始时间
    TIME_T t_end;        //时间结构体结束时间
    u16    maxblock;     //最大数据块数
    
}VDR_SERT;//记录仪时间检索结构体

typedef struct 		
{
    u8  cmdnum;  //命令字编号
    u32 timestar;//开始时间
    u32 timeend; //结束时间
    
}VDR_PACKINF;//分包信息结构体

typedef struct 		
{
    u32 addrstar;//开始地址
    u32 addrend; //结束地址
    u16 stepcnt; //单步计数器
    u16 all;     //总包数
    u16 cnt;     //当前报数
    u8  cmd;     //命令字
    u8  cyc;     //循环标志
    u16 len;     //长度
    u8  *pbuf;   //地址

}VDR_PACKSER;//分包检索结构体

typedef struct 		
{
    u8 state;  //无线采集状态机
    u8 busy;   //无线采集忙标志
    VDR_CH ch; //通道号
    
    VDR_PACKINF packageinf;//分包信息
    VDR_PACKSER packageser;//分包检索
    VDR_PACKSER packageadd;//分包补传

}VDR_WIRE;//无线采集结构体

typedef struct 		
{
    u8  rdy;  //就绪标志等待8003
    u8  flg;  //补传标志
    u16 ack;  //应答流水号
    u8  all;  //重传个数
    u8  cnt;  //当前个数    
    u16 buf[VDR_WIRE_ADD_SIZE];//重传列表

}VDR_WIREADD;//无线采集补传结构体

typedef  void (*VDR_FUC_SET)(u8 *pSrc);           //设置 
typedef  void (*VDR_FUC_TEST)(void);              //检定
typedef  void (*VDR_FUC_BASE)(VDR_HEAD *pHead);   //基本
typedef  u8   (*VDR_FUC_MEM)(u8 type, u8 *pSrc);  //存储处理 参数设置和读取

/*
********************************************************************************
*                          EXTERN FUNCTIONS
********************************************************************************
*/
extern const VDR_FUC_MEM  Vdr_Fuc_Mem[];
extern VDR_WIRE  Vdr_Wire;

extern const u16 VdrSecAddr[];
extern const u16 VdrStepLen[];
extern const u16 VdrUpLen[];
extern const u8  Vdr_Cmd_Base[];
extern const u8  Vdr_Cmd_Ser[];
extern const u8  Vdr_Cmd_Set[];
extern const u8  Vdr_Cmd_Test[];
extern const u8  Vdr_Cmd_All[];
extern u8  Vdr_BufData[];
extern u8  Vdr_BufSend[];

extern u16 Vdr_WireAck;

/*
********************************************************************************
*                               FUNCTIONS
********************************************************************************
*/
u8   Vdr_Wire_Handle(void);
void Vdr_Usb_Handle(void);
void Vdr_Data_Rec(VDR_CH ch, u8 type, u8 *pSrc, u16 SrcLen);
u8   Vdr_Wire_AddEven(VDR_CH ch, u8 *pSrc);

u8   Vdr_Cmd_To(u8 cmd);
u16  Vdr_Head_ToBuf(u8 type, VDR_HEAD head, u8 *pDst);
void Vdr_Time_Get(const u8 *pSrc, VDR_SERT *pTime);


void Vdr_Set_CarInfor(u8 *pSrc);
void Vdr_Set_TimeInit(u8 *pSrc);
void Vdr_Set_StaSignal(u8 *pSrc);
void Vdr_Set_Rtc(u8 *pSrc);
void Vdr_Set_Pulse(u8 *pSrc);
void Vdr_Set_MileInit(u8 *pSrc);
void Vdr_Set_Pro(u8 cmdnum, u8 *pSrc, VDR_HEAD *pHead);

void Vdr_Get_GbtVer(VDR_HEAD *pHead);
void Vdr_Get_DvrInfor(VDR_HEAD *pHead);
void Vdr_Get_Rtc(VDR_HEAD *pHead);
void Vdr_Get_Mileage(VDR_HEAD *pHead);
void Vdr_Get_Pulse(VDR_HEAD *pHead);
void Vdr_Get_CarInfor(VDR_HEAD *pHead);
void Vdr_Get_StaSignal(VDR_HEAD *pHead);
void Vdr_Get_OnlyNum(VDR_HEAD *pHead);
void Vdr_Get_Base(u8 cmdnum,VDR_HEAD *pHead);
void Vdr_Get_Ser(u8 cmdnum,VDR_SERT time, VDR_HEAD *pHead);
void Vdr_Get_PackageInf(VDR_PACKINF PackInf, VDR_PACKSER *pPackage);
void Vdr_Get_PackageSer(VDR_PACKSER *pPackage);


u8   Vdr_Mem_DvrInfor(u8 type, u8 *pSrc);
u8   Vdr_Mem_TimeInit(u8 type, u8 *pSrc);
u8   Vdr_Mem_MileInit(u8 type, u8 *pSrc);
u8   Vdr_Mem_MileAdd(u8 type, u8 *pSrc);
u8   Vdr_Mem_Pulse(u8 type, u8 *pSrc);
u8   Vdr_Mem_Carcode(u8 type, u8 *pSrc);
u8   Vdr_Mem_Carplate(u8 type, u8 *pSrc);
u8   Vdr_Mem_CarType(u8 type, u8 *pSrc);
u8   Vdr_Mem_StaSignal(u8 type, u8 *pSrc);
u8   Vdr_Mem_Only(u8 type, u8 *pSrc);
u8   Vdr_Mem_Rtc(u8 type, u8 *pSrc);

void Vdr_Test_Mode(void);
void Vdr_Test_Mile(void);
void Vdr_Test_Pulse(void);
void Vdr_Test_Rtc(void);
void Vdr_Test_Break(void);
void Vdr_Test_Pro(u8 cmdnum,VDR_HEAD *pHead);

void Vdr_Eye_Value(VDR_HEAD *pHeadRx,VDR_HEAD *pHeadTx);

FunctionalState RecorderWireless_TimeTask(void);
FunctionalState RecorderWired_TimeTask(void);
u8   Recorder_GetCarInformation(u8 *pBuffer);
u8   Recorder_Stop15MinuteSpeed(u8 *p);
void Recorder_USBHandle(void);
void Recorder_MileageTest(void);
/*
********************************************************************************
*                               MODULE END
********************************************************************************
*/

#endif





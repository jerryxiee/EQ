/*************************************************************************
* Copyright (c) 2013,深圳市伊爱高新技术开发有限公司
* All rights reserved.
* 文件名称 : CAN1_Driver.c
* 当前版本 : 1.0
* 开发者   : Shigle
* 修改时间 : 20113年6月10日
*************************************************************************/

#ifndef _CAN1_DRIVER_H_
#define _CAN1_DRIVER_H_










//==========================================================================
extern void CAN1_Init(void);

extern  void Can_Isr_Tx();

/********************************************************************
* 名称 : CAN1_ID_Filter_Set
* 功能 : CAN1 过滤ID号设置
* 输入 :  FilterNum: 过滤ID的序号，不可重复 ， 取值范围 整数1~13
           filterId: 过滤ID , 低28位为有效的ID位， 其余位保留为0 
       filterMaskId: 过滤屏蔽位， 1: 表必须符合  0:随意
********************************************************************/
extern void CAN1_ID_Filter_Set(u8 FilterNum, u32 filterId, u32 filterMaskId);


 //参数ID0x0100//CAN 总线通道1 采集时间间隔(ms)，0 表示不采集 

extern void UpdataOnePram_SamplePeriod_CAN1( void );


//参数ID0x0101//CAN 总线通道1 上传时间间隔(s)，0 表示不上传

extern void UpdataOnePram_UploadPeriod_CAN1( void );


//参数ID0x0110//CAN 总线ID 单独采集设置BYTE[8]
  

//extern void UpdataOnePram_Sample_Single_CAN1( void );




u8 CanBus_Send(u8 *dataAddr, u32 dataLen);
extern u8 CanBus_GetData( u8 iCANChannel , u8 *pdata );


//==========================================================================

#endif

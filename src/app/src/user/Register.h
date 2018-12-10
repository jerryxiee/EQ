#ifndef	__REGISTER_H
#define	__REGISTER_H

//*************文件包含***************
#include "stm32f2xx_gpio.h"
#include "stm32f2xx.h"
//*************宏定义****************
//////////////////////////////

///////////////////////////////////
//记录类型
#define REGISTER_TYPE_SPEED                 1	//行驶速度记录
#define REGISTER_TYPE_POSITION              2	//位置信息记录
#define REGISTER_TYPE_DOUBT                 3	//疑点数据记录
#define REGISTER_TYPE_OVER_TIME             4	//超时驾驶记录
#define REGISTER_TYPE_DRIVER                5	//驾驶人身份记录
#define REGISTER_TYPE_POWER_ON              6	//外部供电记录
#define REGISTER_TYPE_PRAMATER              7	//参数修改记录
#define REGISTER_TYPE_SPEED_STATUS          8	//速度状态记录
#define	REGISTER_TYPE_MAX                   9	
//采集数据长度限制
#ifdef USE_ONE_MIN_SAVE_SPEED_RECORD
#define	REGISTER_SPEED_COLLECT_LEN          126	//按条存储//modify by joneming	
#else
#define	REGISTER_SPEED_COLLECT_LEN          2	//按条存储
#endif
#define	REGISTER_POSITION_COLLECT_LEN       11
#define	REGISTER_DOUBT_COLLECT_LEN          234	//按块存储
#define	REGISTER_OVER_TIME_COLLECT_LEN      50
#define	REGISTER_DRIVER_COLLECT_LEN         25
#define	REGISTER_POWER_ON_COLLECT_LEN       7
#define	REGISTER_PRAMATER_COLLECT_LEN       7
#define	REGISTER_SPEED_STATUS_COLLECT_LEN   133
//采集数据最大块限制
#define	REGISTER_SPEED_MAX_BLOCK            7	//1000字节/块长度126字节
#define	REGISTER_POSITION_MAX_BLOCK         1	//1000字节/块长度666字节
#define	REGISTER_DOUBT_MAX_BLOCK            4	//1000字节/块长度234字节
#define	REGISTER_OVER_TIME_MAX_BLOCK        20	//1000字节/块长度50字节
#define	REGISTER_DRIVER_MAX_BLOCK           40	//1000字节/块长度25字节
#define	REGISTER_POWER_ON_MAX_BLOCK         142	//1000字节/块长度7字节
#define	REGISTER_PRAMATER_MAX_BLOCK         142	//1000字节/块长度7字节
#define	REGISTER_SPEED_STATUS_MAX_BLOCK	    7	//1000字节/块长度133字节
//记录步长
#ifdef USE_ONE_MIN_SAVE_SPEED_RECORD
#define	REGISTER_SPEED_STEP_LEN             144	//时间4+每分钟的行驶速度块126+预留13+校验1//modify by joneming	
#else
#define	REGISTER_SPEED_STEP_LEN             11	//时间4+行驶速度及状态2+预留4+校验1
#endif
#define	REGISTER_POSITION_STEP_LEN          24	//时间4+位置信息10+平均速度1+里程4+预留4+校验1
#define	REGISTER_DOUBT_STEP_LEN             251	//时间4+疑点数据234+预留12+校验1
#define	REGISTER_OVER_TIME_STEP_LEN         67	//时间4+超时驾驶50+预留12+校验1
#define	REGISTER_DRIVER_STEP_LEN            36	//时间4+驾驶人身份25+预留6+校验1
#define	REGISTER_POWER_ON_STEP_LEN          18	//时间4+外部供电7+预留6+校验1
#define	REGISTER_PRAMATER_STEP_LEN          18	//时间4+参数修改7+预留6+校验1
#define	REGISTER_SPEED_STATUS_STEP_LEN      144	//时间4+速度状态133+预留6+校验1
#define	REGISTER_MAX_STEP_LEN               252

//远程行驶记录仪数据传输时每包的条数
#ifdef USE_ONE_MIN_SAVE_SPEED_RECORD
#define	REGISTER_SPEED_PACKET_LIST          4	//每4条的行驶速度及状态数据打成一个包上传//modify by joneming	
#else
#define	REGISTER_SPEED_PACKET_LIST          240	//每4小时（每秒1条）的行驶速度及状态数据打成一个包上传
#endif
#define	REGISTER_POSITION_PACKET_LIST       60	//每1小时（每分钟1条）位置信息数据打成一个包上传
#define	REGISTER_DOUBT_PACKET_LIST          2	//每2条疑点数据打成一个包上传
#define	REGISTER_OVER_TIME_PACKET_LIST      10	//每10条超时驾驶数据打成一个包上传
#define	REGISTER_DRIVER_PACKET_LIST         20	//每20条驾驶人身份数据打成一个包上传
#define	REGISTER_POWER_ON_PACKET_LIST       100	//每100条外部供电数据打成一个包上传
#define	REGISTER_PRAMATER_PACKET_LIST       100	//每100条参数修改数据打成一个包上传
#define	REGISTER_SPEED_STATUS_PACKET_LIST   4	//每4条速度状态数据打成一个包上传

//行驶记录仪标准中每个块数据包含的条数
#ifdef USE_ONE_MIN_SAVE_SPEED_RECORD
#define	REGISTER_SPEED_BLOCK_LIST           1//modify by joneming
#else
#define	REGISTER_SPEED_BLOCK_LIST           60
#endif
#define	REGISTER_POSITION_BLOCK_LIST        60	
#define	REGISTER_DOUBT_BLOCK_LIST           1	
#define	REGISTER_OVER_TIME_BLOCK_LIST       1	
#define	REGISTER_DRIVER_BLOCK_LIST          1	
#define	REGISTER_POWER_ON_BLOCK_LIST        1	
#define	REGISTER_PRAMATER_BLOCK_LIST        1
#define	REGISTER_SPEED_STATUS_BLOCK_LIST    1	

//************结构体类型******************
typedef struct
{
    s16 StartSector;	//存储地址开始扇区
    s16 EndSector;	//存储地址结束扇区
    s16 StepLen;	//步长
    s16 SectorStep;	//单个扇区总步数
    s16 OldestSector;	//最老时间扇区
    s16 CurrentSector;	//当前时间扇区
    s16 CurrentStep;	//当前步数
    s8  LoopFlag;	//循环存储标志，0为不循环，1为循环	
}REGISTER_STRUCT;
//*************函数声明**************
//****************外部接口函数*****************
/*********************************************************************
//函数名称	:Register_CheckArea(void)
//功能		:检查所有存储区域，确定最老扇区，最新扇区，最新步数
//返回		:正确返回0，否则表示哪个类型的自检出错
//备注		:在上电运行while前需调用该函数，
//		:只有该函数先被调用后才能保证后续的读、写、擦除函数运行正确
*********************************************************************/
u8 Register_CheckArea(void);
/*********************************************************************
//函数名称	:Register_CheckOneArea(void)
//功能		:检查某一个存储区域
//返回		:正确返回SUCCESS，错误返回ERROR
*********************************************************************/
ErrorStatus Register_CheckOneArea(u8 Type);
/*********************************************************************
//函数名称	:Register_CheckOneArea2(void)
//功能		:检查某一个存储区域
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:返回结束标志，2为参数错误，1为该区域检查结束，0为未结束，
//备注		:只用于位置记录、速度记录；调用该函数的应用需要保证只有
//              :返回值为1时（表明自检结束）才能往该区域写数据
//              :REGISTER_TYPE_SPEED、REGISTER_TYPE_POSITION这两个类型的检查可调用此函数
*********************************************************************/
u8 Register_CheckOneArea2(u8 Type);
/*********************************************************************
//函数名称	:Register_Write(u8 Type,u8 *pBuffer, u8 length)
//功能		:记录一条某种类型的信息
//返回		:正确返回SUCCESS，错误返回ERROR
*********************************************************************/
ErrorStatus Register_Write(u8 Type,u8 *pBuffer, u16 length);
/*********************************************************************
//函数名称	:Register_Write2(u8 Type,u8 *pBuffer, u8 length, u32 Time)
//功能		:记录一条某种类型的信息 
//返回		:正确返回SUCCESS，错误返回ERROR
//备注		:Time为32位表示的时间，
//              :对于疲劳驾驶这个时间是疲劳驾驶结束时间；
//              :对于掉电需保存的数据这个时间是掉电前记录的最后时间
//              :其它情况通常这个时间是当前时间，调用RTC_GetCounter可获得当前时间。
*********************************************************************/
ErrorStatus Register_Write2(u8 Type,u8 *pBuffer, u16 length,u32 Time);
/*********************************************************************
//函数名称	:Register_Read(u8 Type,u8 *pBuffer, TIME_T StartTime, TIME_T EndTime, u16 MaxBlock)
//功能		:读取指定时间段，指定最大数据块的记录数据
//返回		:读取的数据长度
*********************************************************************/
u16 Register_Read(u8 Type,u8 *pBuffer, TIME_T StartTime, TIME_T EndTime, u16 MaxBlock);
/*********************************************************************
//函数名称	:Register_GetNewestTime(u8 Type)
//功能		:获取当前存储的最新步数的时间 
//返回		:4字节表示的时间
*********************************************************************/
u32 Register_GetNewestTime(u8 Type);
/*********************************************************************
//函数名称	:Register_GetOldestTime(u8 Type)
//功能		:获取当前存储的最老步数的时间 
//返回		:4字节表示的时间
*********************************************************************/
u32 Register_GetOldestTime(u8 Type);
/*********************************************************************
//函数名称	:Register_Search(u8 Type,u32 Time, s16 *Sector, s16 *Step)
//功能		:查找某一给定时间的存储位置
//返回		:实际查找得到的时间
//备注		:查找的结果放在Sector，Step中
*********************************************************************/
u32 Register_Search(u8 Type, u32 Time, s16 *Sector, s16 *Step);
/*********************************************************************
//函数名称	:Register_EraseRecorderData(void)
//功能		:擦除行驶记录仪所有的存储区域
//备注		:
*********************************************************************/
void Register_EraseRecorderData(void);
/*********************************************************************
//函数名称	:Register_EraseOneArea(u8 Type)
//功能		:擦除某个类型的存储区域
//备注		:
*********************************************************************/
void Register_EraseOneArea(u8 Type);
/*********************************************************************
//函数名称	:Register_GetSubpacketData
//功能		:获取分包数据
//备注		:该函数用于北斗功能测试，北斗功能测试要求上传所有的行驶
//              :记录仪数据，从最新时间开始上传。分包规则：每包最多512字节，
//              :（除位置信息是666字节外）PacketNum从1开始
*********************************************************************/
u16 Register_GetSubpacketData(u8 *pBuffer,u8 Type, u16 PacketNum);
/*********************************************************************
//函数名称	:Register_GetSubpacketStoreStep
//功能		:获取某个分包的存储步数
//备注		:该函数用于北斗功能测试，北斗功能测试要求上传所有的行驶
//              :记录仪数据，从最新时间开始上传。分包规则：每包最多512字节，
//              :（除位置信息是666字节外），最新时间分包序号为1
*********************************************************************/
void Register_GetSubpacketStoreStep(s16 *Sector, s16 *Step,u8 Type, u16 PacketNum);
/*********************************************************************
//函数名称	:Register_GetSubpacketTotalList
//功能		:获取分包总条数
//备注		:该函数用于北斗功能测试，北斗功能测试要求上传所有的行驶
//              :记录仪数据，从最新时间开始上传。分包规则：每包最多512字节，
//              :（除位置信息是666字节外），最新时间分包序号为1
*********************************************************************/
u16 Register_GetSubpacketTotalList(u8 Type);
//*****************内部接口函数**********************
/*********************************************************************
//函数名称	:Register_GetVerifySum(u8 *pBuffer, u16 length)
//功能		:获取输入字符串数据的校验和
//备注		:
*********************************************************************/
u8 Register_GetVerifySum(u8 *pBuffer, u16 length);
/*********************************************************************
//函数名称	:Register_CheckPram(u8 Type, u32 Time, u8 length)
//功能		:检查输入参数的合法性
//备注		:
*********************************************************************/
ErrorStatus Register_CheckPram(u8 Type, u32 Time, u16 length);
/*********************************************************************
//函数名称	:Register_BytesToInt(u8 *pBuffer)
//功能		:四个字节流转换成一个整形数据，大端模式，高字节在前
//备注		:
*********************************************************************/
u32 Register_BytesToInt(u8 *pBuffer);
/*********************************************************************
//函数名称	:Register_IntToBytes(u8 *pBuffer, u32 a)
//功能		:一个整形数据转换成四个字节流，大端模式，高字节在前
//备注		:
*********************************************************************/
u8 Register_IntToBytes(u8 *pBuffer, u32 a);
/*********************************************************************
//函数名称	:Register_BytesToShortInt(u8 *pBuffer)
//功能		:2个字节流转换成一个短整形数据，大端模式，高字节在前
//备注		:
*********************************************************************/
u16 Register_BytesToShortInt(u8 *pBuffer);
/*********************************************************************
//函数名称	:Register_ShortIntToBytes(u8 *pBuffer, u16 a)
//功能		:一个短整形数据转换成2个字节流，大端模式，高字节在前
//备注		:
*********************************************************************/
u8 Register_ShortIntToBytes(u8 *pBuffer, u16 a);


#endif

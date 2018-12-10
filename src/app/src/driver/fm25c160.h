#ifndef __FRAM_H
#define __FRAM_H

//*********************头文件********************************
#include "stm32f2xx.h"
#include "spi_flash.h"

//*********************自定义数据类型************************


//**********************宏定义********************************
//******************铁电空间分配******************
//#define 	FRAM_DOUBT_POINT_ADDR			0	//疑点数据,200字节,之前2003标准用的疑点数据地址
//#define 	FRAM_DOUBT_POINT_LEN			300	//疑点数据地址长度,加入校验字节

#define 	FRAM_SPEED_RUN_RECORD_ADDR          0//保存当前最新一分钟的行驶记录内容
#define 	FRAM_SPEED_RUN_RECORD_LEN           128//保存当前最新一分钟的行驶记录内容
#define 	FRAM_SPEED_PLAY_ADDR                130
#define 	FRAM_SPEED_PLAY_ADDR_LEN            8//保存当前播报结构体
#define     FRAM_PHONE_MONTH_PARAM_ADDR         139//每月通话参数
#define     FRAM_PHONE_MONTH_PARAM_LEN          12//每月通话参数长度
#define		FRAM_MILEAGE_REMAINDER_ADDR		310	//累计里程余数地址，现在是脉冲余数
#define		FRAM_MILEAGE_REMAINDER_LEN		2	//累计里程余数长度，现在是脉冲余数
#define		FRAM_CURRENT_DRIVER_MILEAGE_ADDR	313	//以驾驶里程，当前驾驶员里程
#define		FRAM_CURRENT_DRIVER_MILEAGE_LEN		3	//当前驾驶员里程存储长度
#define 	FRAM_START_TIME_ADDR            	317  	//开车时间
#define 	FRAM_START_TIME_LEN			4	//开车时间存储长度
#define 	FRAM_END_TIME_ADDR              	322  	//停车时间
#define 	FRAM_END_TIME_LEN			4	//停车时间存储长度
#define		FRAM_TIRED_START_TIME_ADDR		327  	//疲劳驾驶记录开车时间
#define		FRAM_TIRED_START_TIME_LEN		4	//疲劳驾驶开车时间存储长度
#define		FRAM_TIRED_END_TIME_ADDR		332  	//疲劳驾驶记录停车时间
#define		FRAM_TIRED_END_TIME_LEN			4	//疲劳驾驶停车时间存储长度
#define 	FRAM_TIRED_DRIVER_ADDR          	337  	//疲劳驾驶驾驶证号码后3个字节
#define 	FRAM_TIRED_DRIVER_LEN			3	//疲劳驾驶驾驶员代码存储长度
#define 	FRAM_TRIED_DRIVE_ADDR			341	//疲劳驾驶时间
#define 	FRAM_TRIED_DRIVE_LEN			4	//疲劳驾驶时间存储长度
#define		FRAM_STOP_MEAN_SPEED_ADDR		346  	//停车前15分钟车速
#define		FRAM_STOP_MEAN_SPEED_LEN		96	//停车前15分钟车速
#define 	FRAM_MEAN_SPEED_BASE_ADDR		FRAM_STOP_MEAN_SPEED_ADDR
#define 	FRAM_MEAN_SPEED_STEP_LEN		5
#define		FRAM_LAST_LOCATION_ADDR			450	//上一个有效定位点
#define 	FRAM_LAST_LOCATION_LEN			11	//上一个有效定位点
#define		FRAM_MILEAGE_ADDR			472	//总里程存储地址,带校验字节,
#define		FRAM_MILEAGE_LEN			4	//总里程存储长度,带校验字节
#define		FRAM_LAST_RUN_TIME_ADDR			477	//最后行驶的那个时刻,带校验字节
#define		FRAM_LAST_RUN_TIME_LEN			4	//最后行驶的那个时刻长度
#define		FRAM_15MIN_END_TIME_ADDR		482	//15分钟停车时间
#define         FRAM_15MIN_END_TIME_LEN			4	
#define		FRAM_MEDIA_ID_ADDR			487	//多媒体ID
#define		FRAM_MEDIA_ID_LEN			4
/* 老的拍照程序使用的宏定义，双连接版本后不再使用
#define		FRAM_CAM1_PHOTO_NUM_ADDR		492	//拍摄剩余张数
#define		FRAM_CAM1_PHOTO_NUM_LEN			2
#define		FRAM_CAM1_PHOTO_TIME_ADDR		495	//拍摄时间间隔
#define		FRAM_CAM1_PHOTO_TIME_LEN		2
#define		FRAM_CAM1_SAVE_FLAG_ADDR		498	//拍摄保存标志
#define		FRAM_CAM1_SAVE_FLAG_LEN			1
#define		FRAM_CAM1_RESOLUTION_ADDR		500	//拍摄分辨率
#define		FRAM_CAM1_RESOLUTION_LEN		1
#define		FRAM_CAM2_PHOTO_NUM_ADDR		502	//拍摄剩余张数
#define		FRAM_CAM2_PHOTO_NUM_LEN			2
#define		FRAM_CAM2_PHOTO_TIME_ADDR		505	//拍摄时间间隔
#define		FRAM_CAM2_PHOTO_TIME_LEN		2
#define		FRAM_CAM2_SAVE_FLAG_ADDR		508	//拍摄保存标志
#define		FRAM_CAM2_SAVE_FLAG_LEN			1
#define		FRAM_CAM2_RESOLUTION_ADDR		510	//拍摄分辨率
#define		FRAM_CAM2_RESOLUTION_LEN		1
#define		FRAM_CAM3_PHOTO_NUM_ADDR		512	//拍摄剩余张数
#define		FRAM_CAM3_PHOTO_NUM_LEN			2
#define		FRAM_CAM3_PHOTO_TIME_ADDR		515	//拍摄时间间隔
#define		FRAM_CAM3_PHOTO_TIME_LEN		2
#define		FRAM_CAM3_SAVE_FLAG_ADDR		518	//拍摄保存标志
#define		FRAM_CAM3_SAVE_FLAG_LEN			1
#define		FRAM_CAM3_RESOLUTION_ADDR		520	//拍摄分辨率
#define		FRAM_CAM3_RESOLUTION_LEN		1
#define		FRAM_CAM4_PHOTO_NUM_ADDR		522	//拍摄剩余张数
#define		FRAM_CAM4_PHOTO_NUM_LEN			2
#define		FRAM_CAM4_PHOTO_TIME_ADDR		525	//拍摄时间间隔
#define		FRAM_CAM4_PHOTO_TIME_LEN		2
#define		FRAM_CAM4_SAVE_FLAG_ADDR		528	//拍摄保存标志
#define		FRAM_CAM4_SAVE_FLAG_LEN			1
#define		FRAM_CAM4_RESOLUTION_ADDR		530	//拍摄分辨率
#define		FRAM_CAM4_RESOLUTION_LEN		1
//522-572为摄像头5-8保留
*/
#define		FRAM_CAM_TIME_PHOTO_NUM_ADDR		    492	//平台定时拍照剩余张数
#define		FRAM_CAM_TIME_PHOTO_NUM_LEN		    2//4路摄像头共占用12字节,每路3字节,从1到4,顺序排列
#define		FRAM_CAM_TIME_PHOTO_SPACE_ADDR	    504	//平台定时拍照时间间隔，单位秒
#define		FRAM_CAM_TIME_PHOTO_SPACE_LEN		    2//4路摄像头共占用12字节,每路3字节,从1到4,顺序排列
#define		FRAM_CAM_TIME_PHOTO_STORE_FLAG_ADDR	516	//平台定时拍照保存标志,bit0置1表示需要上传，bit1置1表示需要保存至SD卡
#define		FRAM_CAM_TIME_PHOTO_STORE_FLAG_LEN    1//4路摄像头共占用8字节,每路2字节,从1到4,顺序排列
#define		FRAM_CAM_TIME_PHOTO_RESOLUTION_ADDR	524	//平台定时拍照分辨率,0为320*240,1为640*480
#define		FRAM_CAM_TIME_PHOTO_RESOLUTION_LEN    1//4路摄像头共占用8字节,每路2字节,从1到4,顺序排列
#define		FRAM_CAM_TIME_PHOTO_EVENT_TYPE_ADDR	532	//平台定时拍照事件类型
#define		FRAM_CAM_TIME_PHOTO_EVENT_TYPE_LEN    1//4路摄像头共占用8字节,每路2字节,从1到4,顺序排列
#define		FRAM_CAM_TIME_PHOTO_CHANNEL_ADDR	    540	//平台定时拍照上传通道，只能为CHANNEL_DATA_1或者是CHANNEL_DATA_2
#define		FRAM_CAM_TIME_PHOTO_CHANNEL_LEN       1//4路摄像头共占用8字节,每路2字节,从1到4,顺序排列

#define		FRAM_CAM_EVENT_PHOTO_NUM_ADDR		    548	//事件拍照剩余张数
#define		FRAM_CAM_EVENT_PHOTO_NUM_LEN		    2//4路摄像头共占用12字节,每路3字节,从1到4,顺序排列
#define		FRAM_CAM_EVENT_PHOTO_SPACE_ADDR	        560	//事件拍照时间间隔，单位秒
#define		FRAM_CAM_EVENT_PHOTO_SPACE_LEN		    2//4路摄像头共占用12字节,每路3字节,从1到4,顺序排列
#define		FRAM_CAM_EVENT_PHOTO_STORE_FLAG_ADDR	572	//事件拍照保存标志,bit0置1表示需要上传，bit1置1表示需要保存至SD卡
#define		FRAM_CAM_EVENT_PHOTO_STORE_FLAG_LEN     1//4路摄像头共占用8字节,每路2字节,从1到4,顺序排列
#define		FRAM_CAM_EVENT_PHOTO_RESOLUTION_ADDR	580	//事件拍照分辨率,0为320*240,1为640*480
#define		FRAM_CAM_EVENT_PHOTO_RESOLUTION_LEN     1//4路摄像头共占用8字节,每路2字节,从1到4,顺序排列
#define		FRAM_CAM_EVENT_PHOTO_EVENT_TYPE_ADDR	588	//事件拍照事件类型
#define		FRAM_CAM_EVENT_PHOTO_EVENT_TYPE_LEN     1//4路摄像头共占用8字节,每路2字节,从1到4,顺序排列
#define		FRAM_CAM_EVENT_PHOTO_CHANNEL_ADDR	    596	//事件拍照上传通道，只能为CHANNEL_DATA_1或者是CHANNEL_DATA_2
#define		FRAM_CAM_EVENT_PHOTO_CHANNEL_LEN        1//4路摄像头共占用8字节,每路2字节,从1到4,顺序排列

#define         FRAM_PARAMETER_TIME_PHOTO_CHANNEL_ADDR  604//参数定时拍照通道
#define         FRAM_PARAMETER_TIME_PHOTO_CHANNEL_LEN   1
#define         FRAM_PARAMETER_DISTANCE_PHOTO_CHANNEL_ADDR  606//参数定距拍照通道
#define         FRAM_PARAMETER_DISTANCE_PHOTO_CHANNEL_LEN   1

#define         FRAM_PHOTO_ID_ADDR                      608
#define         FRAM_PHOTO_ID_LEN                       4


//下一个地址是613

#define		FRAM_TEMP_TRACK_NUM_ADDR		647	//临时跟踪剩余次数
#define		FRAM_TEMP_TRACK_NUM_LEN			4
#define		FRAM_TEMP_TRACK_SPACE_ADDR		652	//临时跟踪间隔
#define		FRAM_TEMP_TRACK_SPACE_LEN		2	
//#define		FRAM_BLIND_SECTOR_ADDR			591	//盲区汇报的扇区
//#define		FRAM_BLIND_SECTOR_LEN			2
//#define		FRAM_BLIND_STEP_ADDR			594	//盲区补报的步数
//#define		FRAM_BLIND_STEP_LEN			    2
#define		FRAM_EMERGENCY_FLAG_ADDR		655	//紧急报警上一次状态
#define		FRAM_EMERGENCY_FLAG_LEN			1
#define		FRAM_RECORD_FLAG_ADDR			657	//录音记录铁电区域
#define 	FRAM_RECORD_FLAG_LEN 			20
#define  	FRAM_TEXI_SERVICE_ADDR 			678 	//出租车业务
#define  	FRAM_TEXI_SERVICE_LEN 			24
#define		FRAM_RECORD_CONTROL_ADDR		703	//录音控制标志
#define		FRAM_RECORD_CONTROL_LEN			1
#define		FRAM_RECORD_CLOSE_ADDR			705	//关录音标志
#define		FRAM_RECORD_CLOSE_LEN			1
#define		FRAM_RECORD_OPEN_ADDR			707	//开录音标志
#define		FRAM_RECORD_OPEN_LEN			1
#define		FRAM_TTS_VOLUME_ADDR			709	//TTS音量
#define		FRAM_TTS_VOLUME_LEN			    1
#define		FRAM_TEL_VOLUME_ADDR			711	//电话音量
#define		FRAM_TEL_VOLUME__LEN			1
/*
#define		FRAM_CAM1_EVENT_PHOTO_NUM_ADDR		666	//事件触发拍摄剩余张数
#define		FRAM_CAM1_EVENT_PHOTO_NUM_LEN		2
#define		FRAM_CAM1_EVENT_PHOTO_TIME_ADDR		669	//拍摄时间间隔
#define		FRAM_CAM1_EVENT_PHOTO_TIME_LEN		2
#define		FRAM_CAM2_EVENT_PHOTO_NUM_ADDR		672	//事件触发拍摄剩余张数
#define		FRAM_CAM2_EVENT_PHOTO_NUM_LEN		2
#define		FRAM_CAM2_EVENT_PHOTO_TIME_ADDR		675	//拍摄时间间隔
#define		FRAM_CAM2_EVENT_PHOTO_TIME_LEN		2
#define		FRAM_CAM3_EVENT_PHOTO_NUM_ADDR		678	//事件触发拍摄剩余张数
#define		FRAM_CAM3_EVENT_PHOTO_NUM_LEN		2
#define		FRAM_CAM3_EVENT_PHOTO_TIME_ADDR		681	//拍摄时间间隔
#define		FRAM_CAM3_EVENT_PHOTO_TIME_LEN		2
#define		FRAM_CAM4_EVENT_PHOTO_NUM_ADDR		684	//事件触发拍摄剩余张数
#define		FRAM_CAM4_EVENT_PHOTO_NUM_LEN		2
#define		FRAM_CAM4_EVENT_PHOTO_TIME_ADDR		687	//拍摄时间间隔
#define		FRAM_CAM4_EVENT_PHOTO_TIME_LEN		2
//预留摄像头5~8
*/
#define FRAM_OIL_CTRL_ADDR                  		713 //断油路
#define FRAM_OIL_CTRL_ADDR_LEN         			1
#define FRAM_SPEED_LOG_TIME                             715   //速度日志信息
#define FRAM_SPEED_LOG_TIME_LEN                         7
#define FRAM_DOUBT_POINT_ADDR 				723  //疑点数据,2byte单步 + 7byte实时时间 + 450byte速度状态
#define FRAM_DOUBT_POINT_LEN 				459  //疑点数据地址长度,加入校验字节
#define FRAM_POWER_LOG_TIME 				1183 //主电工作时间--》记录为无备电使用
#define FRAM_POWER_LOG_TIME_LEN 			6    //记录时间长度为6
#define FRAM_DOUBT_POSITION_ADDR                        1190 //疑点采集时位置信息，每一秒写入一次
#define FRAM_DOUBT_POSITION_LEN                         10
#define FRAM_SPEED_STATUS_ADDR                          1201 //速度状态记录标志
#define FRAM_SPEED_STATUS_LEN                           1
#define FRAM_SPEED_STA_TIME_ADDR                        1203 //速度状态时间记录
#define FRAM_SPEED_STA_TIME_LEN                         6
#define	FRAM_STOP_TIME_ADDR				1210//当前
#define	FRAM_STOP_TIME_LEN				4
#define FRAM_CAR_OWNER_PHONE_ADDR                       1215//车主手机号,字符串ASCII码,dxl
#define FRAM_CAR_OWNER_PHONE_LEN                        12//写入或读取该项参数时长度需填写12,位数不足时后补0x00
#define FRAM_MAIN_DOMAIN_NAME_ADDR 			1228//主监控中心域名,字符串,ASCII码,为行标送检而添加,dxl
#define FRAM_MAIN_DOMAIN_NAME_LEN                       30//写入或读取该项参数时长度需填写30,位数不足时后补0x00
#define FRAM_BACKUP_DOMAIN_NAME_ADDR 		        1259//备份监控中心域名,字符串,ASCII码,为行标送检而添加,dxl
#define FRAM_BACKUP_DOMAIN_NAME_LEN                      30//写入或读取该项参数时长度需填写30,位数不足时后补0x00
#define FRAM_CAR_CARRY_STATUS_ADDR 			1290//车辆载货状态,1字节,dxl
#define FRAM_CAR_CARRY_STATUS_LEN                         1//写入或读取该参数时长度需为1
#define FRAM_QUICK_OPEN_ACCOUNT_ADDR                    1292//快速开户锁定状态存储地址
#define FRAM_QUICK_OPEN_ACCOUNT_LEN                       1//写入或读取该参数时长度需为1
#define FRAM_MULTI_CENTER_LINK_ADDR                     1294//多中心连接标志
#define FRAM_MULTI_CENTER_LINK_LEN                        1//多中心连接标志长度

#define FRAM_STOP_15MINUTE_SPEED_ADDR                   1296//停车前15分钟速度
#define FRAM_STOP_15MINUTE_SPEED_LEN                    62  // 1byte + 1byte = 单步 + 校验
                                                            // (2byte + 1byte + 1byte = 时分 + 速度 + 校验)*15
#define FRAM_STOPING_TIME_ADDR                          1359// 车辆停驶那一时刻的时间
#define FRAM_STOPING_TIME_LEN                           6   // 6byte  BCD年月日时分秒 

#define FRAM_FIRMWARE_UPDATA_FLAG_ADDR                  1366//升级任务触发标志，等于1表示触发了远程升级，等于2表示升级成功，
                                                            //等于3表示升级失败，等于4表示已发送过0x0108命令了，每次上电鉴权完成后需要读取该值，
#define FRAM_FIRMWARE_UPDATA_FLAG_LEN                   1
#define FRAM_GNSS_SAVE_STEP_ADDR                        1368	//Gnss盲区补报的步数
#define FRAM_GNSS_SAVE_STEP_LEN                         2

#define FRAM_HY_MILEAGE_ADDR                            1371//货运平台专用                         
#define FRAM_HY_MILEAGE_LEN                             4
#define FRAM_HY_UPDATA_ADDR                             1376//货运平台专用                         
#define FRAM_HY_UPDATA_LEN                              1
#define FRAM_FIRMWARE_UPDATA_TYPE_ADDR                  1378//远程升级类型,0为终端,9为载重控制器,目前只支持这两个
#define FRAM_FIRMWARE_UPDATA_TYPE_LEN                   1

#define FRAM_ROLL_OVER_INIT_ANGLE_ADDR                  1380//侧翻初始角度
#define FRAM_ROLL_OVER_INIT_ANGLE_LEN                   8

#define FRAM_OIL_WEAR_CALIB_ADDR                        1400//油耗校准//lzm,2014.9.24
 #define FRAM_OIL_WEAR_CALIB_ADDR_LEN                    104

//1505-1515在标准版中被使用了，尽量避开。
#define FRAM_ACCELERATION_ADDR                          1516
#define FRAM_ACCELERATION_LEN                           4//0-30,30-60,60-90,>90
#define FRAM_DECELERATION_ADDR                          1521
#define FRAM_DECELERATION_LEN                           2//0-30,>30
#define FRAM_TURN_ANGLE_ADDR                			1524
#define FRAM_TURN_ANGLE_LEN                 			2//急转弯角度阀

#define FRAM_INTERFACE_SWOTCH_ADDR                       1527
#define FRAM_INTERFACE_SWOTCH_LEN                        2

//下一个地址为1524

#define		FRAM_LAST_ADDR				0x07fc  //2043,FRAM最后两个字节用于芯片自检

//*******************铁电操作相关*****************
//#define FRAM_CS	  		GPIOC
//#define FRAM_Pin_CS	  	GPIO_Pin_4

#define FRAM_CS_HIGH()	GpioOutOn(FR_CS);spi_Delay_uS(60)
#define FRAM_CS_LOW()	GpioOutOff(FR_CS);spi_Delay_uS(60)

//***********************函数声明********************************
/*********************************************************************
//函数名称	:FRAM_Init()
//功能		:铁电初始化
//备注		:
*********************************************************************/
void FRAM_Init(void);
/*********************************************************************
//函数名称	:CheckFramChip(void)
//功能		:检查铁电芯片工作是否正常
//输入		:铁电地址的最后两个字节用于自检
//备注		:
*********************************************************************/
ErrorStatus CheckFramChip(void);
/*********************************************************************
//函数名称	:FRAM_BufferWrite2(u16 WriteAddr, u8 *pBuffer, u16 NumBytesToWrite)
//功能		:往铁电里写数据,在数据某尾加入校验字节
//输入		:WriteAddr，铁电地址
//		:pBuffer，数据缓冲
//		:NumBytesToWrite，写入的字节数
//备注		:
*********************************************************************/
void FRAM_BufferWrite(u16 WriteAddr, u8 *pBuffer, u16 NumBytesToWrite);
/*********************************************************************
//函数名称	:FRAM_BufferRead(u8 *pBuffer, u16 NumBytesToRead, u16 ReadAddr)
//功能		:从铁电里读数据
//输入		:ReadAddr，铁电地址
//		:pBuffer，目标缓冲
//		:NumBytesToRead，读出的字节数 
//返回		:实际读出的字节数
//备注		:
*********************************************************************/
u8 FRAM_BufferRead(u8 *pBuffer, u16 NumBytesToRead, u16 ReadAddr);
/*********************************************************************
//函数名称	:FRAM_WriteEnable(void)
//功能		:铁电写使能
//备注		:
*********************************************************************/
void FRAM_WriteEnable(void);
/*********************************************************************
//函数名称	:FRAM_WriteDisable(void)
//功能		:铁电写禁止
//备注		:
*********************************************************************/
void FRAM_WriteDisable(void);
/*********************************************************************
//函数名称	:FRAM_WriteStatusRegister(u8 Byte)
//功能		:写铁电状态寄存器
//备注		:/WP必须是高电平或无效电平,/WP仅仅是保护写铁电状态寄存器
*********************************************************************/
void FRAM_WriteStatusRegister(u8 Byte);
/*********************************************************************
//函数名称	:FRAM_ReadStatusRegister(void)
//功能		:读铁电状态寄存器 
//返回		:状态寄存器的值
//备注		:
*********************************************************************/
u8   FRAM_ReadStatusRegister(void);
/*********************************************************************
//函数名称	:FRAM_EraseChip(void)
//功能		:擦除整个铁电
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:状态寄存器的值
//备注		:
*********************************************************************/
void  FRAM_EraseChip(void);
#endif

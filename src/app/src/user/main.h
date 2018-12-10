
#ifndef __MAIN_H
#define __MAIN_H

//*********宏定义****************   
#define WATCHDOG_OPEN	1
#define SLEEP_OPEN	2
//#define DEBUG_OPEN	3
//#define HUOYUN_DEBUG_OPEN  1//货运平台联调启用该宏，正常发货该宏要屏蔽，不开启
//#define HUOYUN 1//行驶记录仪分包固定900字节需要开启该宏，正常发货该宏要屏蔽，不开启

#define LOG_TYPE 0   //打印输出调试信息,正常出货时应为0:0-忽略,1-输出

#define GPS_MODE_TYPE_UM330   //只能二选一
//#define GPS_MODE_TYPE_TD3017  


#define  BEEP_ON()  		GpioOutOn(BUZZER);BeepOnOffFlag = 1;BeepOnTime=Timer_Val()//实际应用使用
//#define  BEEP_ON()  		GpioOutOff(BUZZER);BeepOnOffFlag = 1;BeepOnTime=Timer_Val()//调试时静音

#define  ICCARD_SEL ICCARD_JTD //使用交通部指定IC卡
//#define  ICCARD_SEL  ICCARD_EEYE//使用伊爱的卡

#define USE_ONE_MIN_SAVE_SPEED_RECORD     //如果定义了，行驶速度记录则按一分钟一条的存储方式,从双连接版本开始一直开启该宏

#define	FIRMWARE_VERSION	 "11713"//特别注意从V1.01.09版本开始固件升级有分支校验,此处设置需与main.c中的固件版本相一致,main.c中的后5位与Ver:xxxxx项同

//版本号共5位数字，第1位表示机型EGS701；版本号第2位、第3表示分支版本号。奇数表示G网，偶数表示C网。版本号第4位、第5位表示子版本号，按流水排号。
//现在已使用的分支版本号为：01-02：表示通用版本；03-04：表示九通版本；05-06：表示索美版本；07-08表示窄温显示屏版本；09-10:正反转传感器版本；11-12渣土车版本；13-14简式项目,15-16甘肃电信
#define FIRMWARE_TIME		 "201809170024"//编译时间在HB-EGS701GBN_BZ-V1.11.02的201412271042版本上修改的第1版

#define Recorder_CCC_ID             "C000116"          /*7个字节*/
#define Recorder_Product_VER        "EGS701          " /*16个字节*/

extern const char ProductInfo[][17]; 
extern const char MANUFACTURER[];
extern const char PRODUCT_MODEL[];//含2个空格
extern const char PCB_VERSION[];//含1个空格
extern const char SOFTWARE_VERSION[];//与main.h中的FIRMWARE_VERSION保持一致，前面1位填充0,后5位一致。
//*********函数声明**************
#endif

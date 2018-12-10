
#ifndef __PW__LOG__
#define __PW__LOG__

#ifndef	_EXT_PW_
	#define	EXT_PW_LOG		extern
#else
	#define	EXT_PW_LOG
#endif

//电源记录偏移宏定义
#define		RECORD_PWER_LENGTH		7+6	//	7字节记录长度，6字节预留长度

#pragma pack(1)
typedef struct _POWER_RE_
{
		TIME_T	time;
		uchar 	status;
}_API_PWER_RE_;
#pragma pack()

//外部主电的欠压经验值
//（如果外部供电12V对应11。5，如果外部供电24V，对应21.5V）
//电压单位mV
#define		mV		*1
#define		MAIN_PW_UNDER_12V		11*1000mV	//11.5*1000mV		//11.5V 偏差-5%
#define		MAIN_PW_UNDER_24V		21*1000mV	//21.5*1000mV		//21.5V偏差 -5%

#define		REFRENCE_12V_OR_24V		18*1000mV		//判断主电供电是12V还是 24V
#define		ZERO_VOLTAGE_GND		0.6*1000mV		//浮-空空载，感应电压 必须小于0.6V

#define		MAIN_PW_RUN				1		//1：通电
#define		MAIN_PW_STOP				2		//2：断电	

//外部供电7字节+预留6字节（预留字节先填充0x00），共13字节
//供电类型供电时间（BCD码格式）	事件类型
//年 月 日	| 时 分 秒	|   1：通电，2：断电
// 3字节	  | 3字节	    |   1字节
FunctionalState CheckMainPowerRun_TimeTask(void);

//转换函数
void HexToBcd(uchar *p_bcd,uchar *p_hex,uchar length);

#endif

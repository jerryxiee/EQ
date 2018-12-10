//ADC采集函数，针对电压都是针对电压处理函数

#ifndef __ADC_VOLTAGE_API__
#define __ADC_VOLTAGE_API__

#ifndef	_EXT_ADC_VOL_
	#define	EXT_ADC_API		extern
#else
	#define	EXT_ADC_API
#endif

typedef unsigned int  uint;
typedef signed int  sint ;
typedef unsigned char  uchar;
typedef signed char  schar ;
/*
typedef enum
{
	false=0,
	true,	
}PUB_BOOL_;
*/
//针对9路的AD数据采集平滑处理后的数据
typedef struct _ADC_CHANNEL_DATA_
{
	uint ext_vol_1;		//外部输入电压1
	uint ext_vol_2;		//外部输入电压1
	uint main_vol;		//主电压1
	uint	battery_vol;	//电池电压
	uint short_vol;		//短路电压AD
	uint	mic_vol;		//麦克风电压
	uint	key_vol;		//键盘电压
	uint prt_vol;			//打印机电压
	uint	prt_temp_vol;	//打印机温度电压
	
}_ADC_API_;

typedef 	_ADC_API_		*P_ADC_API;

EXT_ADC_API	_ADC_API_	g_adc_api;

#define		MIC_INSERT_DEVICE		1100
#define		MIC_EXTRACT_DEVICE		500

//同类检测，算法的ADC采集
#define		APP_CHANNEL_36V_1			0	//外部输入1通道0-36V
#define		APP_CHANNEL_36V_2			1	//外部输入2通道0-36V
#define		APP_MAIN	_POWER				2	//主电源输入检测
#define		APP_BATTERY_POWER		3	//备份电池电压检测

#define		BASE_CURRENT_200MA		200

//系统调用函数，每隔20毫秒调用一次；采集9个通道的ADC值
//在进行平滑处理
FunctionalState Adc_CollectApp_TimeTask(void);
//外部接口函数
//外部接口ADC1的输出电压检测函数,mV为单位
uint Adc_ExternInputVoltage_1_Read(void);
//外部接口ADC2的输出电压检测函数,mV为单位
uint Adc_ExternInputVoltage_2_Read(void);
//获取主供电电压，mV为单位
uint Adc_MainPower_Readl(void);
//获取备份电池电压
uint Adc_BatteryPower_Readl(void);
//获取短路检测电压参考，电压电位mV
uint Adc_OutputDriver_Read(void);
//获取麦克风状态 1 插好，0没有插
uchar Adc_MicInput_Read(void);
//获取滤波后的键盘AD数值
uint	Adc_GetKeyValue(void);
//判断是否过载函数
//num必须大于0，小于6,输入，驱动输出数
//返回true 超载电流，false 正常工作状态.mV
uchar Adc_CheckDriverCurrentOver(uchar num);

#endif




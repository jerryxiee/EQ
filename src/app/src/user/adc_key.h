//AD键盘扫描函数
//AD键盘的硬件原理分析，根据不同的AD采样值，建立对应的AD键盘关系。
//每隔电阻的偏差值是+-5%
#ifndef __ADC_KEY_API__
#define __ADC_KEY_API__

#ifndef	_EXT_KEY_VOL_
	#define	EXT_KEY_API		extern
#else
	#define	EXT_KEY_API
#endif

EXT_KEY_API  uchar key_code;
//typedef struct __VIRTUAL_KEY_
//{
//	uchar value;
//		
//	
//}
//键盘值对用数据
typedef enum
{
	KEY_ERR0=0,		// 错误AD数据
	KEY_VALUE1,		// 1键按下
	KEY_VALUE2,		// 2键按下
	KEY_VALUE3,
	KEY_VALUE4,
	
	KEY_NONE=0XFF,	//无键盘按下
}_AD_KEY_VALUE;
//每隔电阻的偏差值是5%，所以要考虑波动范围5%
#define		BASE_KEY_R			10*1000
#define		R_KEY_1					20*1000
#define		R_KEY_2					10*1000
#define		R_KEY_3					5.1*1000
#define		R_KEY_4					2*1000

//NONE 无键按下
#define		KEY_NONE_MIN		3890
#define		KEY_NONE_MAX		4095
//理论值	2730
#define		KEY1_ADC_MIN			2637	 
#define		KEY1_ADC_MAX			2819	

#define		KEY1_ADC_VALUE		2730
//理论值	2047
#define		KEY2_ADC_MIN			1945
#define		KEY2_ADC_MAX			2149
//	1383
#define		KEY3_ADC_MIN			1293
#define		KEY3_ADC_MAX			1476
//	682
#define		KEY4_ADC_MIN			627
#define		KEY4_ADC_MAX			741

//感应电压超过0.5V将认为有键按下
//#define		FLOAT_INPUT_STATE_AD		500

//键盘扫描函数 定时100mS调用
FunctionalState KeyBoardAdc_TimeTask(void);
//返回具体键盘的虚拟键值
uchar KeyValue_Read(void);

#endif

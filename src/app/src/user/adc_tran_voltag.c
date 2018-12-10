//ADC采集函数，针对电压都是针对电压处理函数
#define		_EXT_ADC_VOL_ 
/*
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
*/
#include "include.h"
#include "Adc.h"

#define		COLL_CONST		1

/**********************************************************************************/
/*function	name: 			void Adc_CollectApp_TimeTask(void)		*/
/*input parameter:			none  */
/*output parameter:			none */
/*FUNCTION: 			系统调用函数，每隔20毫秒调用一次；采集9个通道的ADC值
在进行平滑处理
*/
/**********************************************************************************/
FunctionalState Adc_CollectApp_TimeTask(void)
{
	uchar k=0;
	static uchar i=0;
	static _ADC_API_	adc[COLL_CONST];
	
	//P_ADC_API	p_adc;
	//读取ADC数据
	adc[i].ext_vol_1=Ad_GetValue(ADC_EXTERN1);
	adc[i].ext_vol_2=Ad_GetValue(ADC_EXTERN2);
	adc[i].main_vol=Ad_GetValue(ADC_MAIN_POWER);
	adc[i].battery_vol=Ad_GetValue(ADC_VBATIN);
	adc[i].short_vol=Ad_GetValue(ADC_IO_OUT);
	adc[i].mic_vol=Ad_GetValue(ADC_MIC);
	adc[i].key_vol=Ad_GetValue(ADC_KEY);
	adc[i].prt_vol=Ad_GetValue(ADC_PRINTF);
	adc[i].prt_temp_vol=Ad_GetValue(ADC_PRINTF_THE);
	
	i++;
	//平滑处理函数
	if(i==COLL_CONST)
		{
			memset((uchar*)&g_adc_api,0,sizeof(g_adc_api));
			for(k=0;k<COLL_CONST;k++)
				{
					g_adc_api.ext_vol_1+=adc[k].ext_vol_1;
					g_adc_api.ext_vol_2+=adc[k].ext_vol_2;
					g_adc_api.main_vol+=adc[k].main_vol;
					g_adc_api.battery_vol+=adc[k].battery_vol;
					g_adc_api.short_vol+=adc[k].short_vol;
					g_adc_api.mic_vol+=adc[k].mic_vol;
					g_adc_api.key_vol+=adc[k].key_vol;
					g_adc_api.prt_vol+=adc[k].prt_vol;
					g_adc_api.prt_temp_vol+=adc[k].prt_temp_vol;
				}
				g_adc_api.ext_vol_1/=COLL_CONST;
				g_adc_api.ext_vol_2/=COLL_CONST;
				g_adc_api.main_vol/=COLL_CONST;
				g_adc_api.battery_vol/=COLL_CONST;
				g_adc_api.short_vol/=COLL_CONST;
				g_adc_api.mic_vol/=COLL_CONST;
				g_adc_api.key_vol/=COLL_CONST;
				g_adc_api.prt_vol/=COLL_CONST;
				g_adc_api.prt_temp_vol/=COLL_CONST;
		}
	
	i=i%COLL_CONST;
	
	
	return ENABLE;
}

/**********************************************************************************/
/*function	name: 			uint uint Adc_ExternInputVoltage_1_Read(void)		*/
/*input parameter:			none  */
/*output parameter:			none */
/*FUNCTION: 			输入的电压，计算公式：Vin*100=Vadc*330/4095*(R1+R2)/R2.；
以毫伏为单位。所以取得的数据都是在V的1000倍
Vin*1000=Vadc*3300/4095*(120K+15K)/15K.，分压超过满量程29.7V。
*/
/**********************************************************************************/
uint Adc_ExternInputVoltage_1_Read(void)
{
  uint tmp=0;
	//获取驱动函数的评法处理。
	tmp=g_adc_api.ext_vol_1;
	tmp=tmp*3300/4095*9;//135/15;
 return tmp;
}
//0-36V外部读取2通道
uint Adc_ExternInputVoltage_2_Read(void)
{
  uint tmp=0;
	//获取驱动函数的评法处理。
	tmp=g_adc_api.ext_vol_2;
	tmp=tmp*3300/4095*9;//135/15;
 return tmp;
}
//获取主供电电压，mV为单位
uint Adc_MainPower_Readl(void)
{
  uint tmp=0;
	//获取驱动函数的评法处理。
	tmp=g_adc_api.main_vol;
	tmp=tmp*3300/4095*9;//135/15;
 return tmp;
}
//获取备份电池电压
uint Adc_BatteryPower_Readl(void)
{
	  uint tmp=0;
	//获取驱动函数的评法处理。
	tmp=g_adc_api.battery_vol;
	tmp=tmp*3300/4095*2;
 return tmp;
}
//获取短路检测电压参考，电压电位mV
uint Adc_OutputDriver_Read(void)
{
	 uint tmp=0;
	 	tmp=g_adc_api.short_vol*3300/4095;
	 return tmp;
}
//判断是否过载函数
//num必须大于0，小于6
//返回true 超载电流，false 正常工作状态.mV
uchar Adc_CheckDriverCurrentOver(uchar num)
{
	//uchar tmp;
	uint vol,tmp_0;
	if(num==0)
		return false;
	else 
		if(num>6)
		return true;	
	vol=num*BASE_CURRENT_200MA*47/30;
	tmp_0=Adc_OutputDriver_Read();
	if(tmp_0>vol||tmp_0==vol)
		return true;
	else
		return false;
		
}
//获取麦克风状态 1 插好，0没有插
uchar  Adc_MicInput_Read(void)
{ 
    uchar tmp;
	//拔出麦克风
	if(g_adc_api.mic_vol<MIC_EXTRACT_DEVICE)
	tmp= false;
	//插入麦克风
	if(g_adc_api.mic_vol>MIC_INSERT_DEVICE)
	tmp= true;
      return tmp;  
}
//获取滤波后的键盘AD数值
uint	Adc_GetKeyValue(void)
{

	return	g_adc_api.key_vol;	
}

//获取打印机电压,单位mV
uint Adc_VoltagePrinterDriver_Read(void)
{
	  uint tmp=0;
	//获取驱动函数的评法处理。
	tmp=g_adc_api.prt_vol;
	tmp=tmp*3300/4095*4;
 return tmp;	
}
//获取打印机温度
uint Adc_PrinterTemprature_Read(void)
{
	
	return g_adc_api.prt_temp_vol;
}
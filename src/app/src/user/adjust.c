/********************************************************************
//版权说明	: Copyright (c) 2011,深圳市伊爱高新技术开发有限公司
//文件名称	: adjust.c		
//功能		: 脉冲自动校准
//              : 通过GPS速度与脉冲计数对照方式计算车辆系数
//              : 1.多次校准后，取平均值.
//版本号	: V1.0.0
//开发人	: wangj
//开发时间	: 
//修改者	: yjb
//修改时间	:
//修改简要说明	:
//备注		:1)校准一次耗时:40s*8=320s/60=5.3分钟,因为要等GPS有速度才测试，注意开始计时是GPS速度>3开始.
//              : 
***********************************************************************/

//****************包含文件*****************
#include "include.h"
#include "adjust.h"

//****************宏定义********************
#define		SAMPLE_CNT		8		  //采样计数

//****************外部变量*******************
extern u32	FeatureCoef;	                           //每公里脉冲数(PulseCounter.c 中定义)

//****************全局变量*******************
static vu8 ucSumCnt = 0;	//校准次数
static u8 adjFlg   = 0;		//允许自动校准标志,1-开始校准,0-停止校准
static u8 tmrCnt   = 0;         //任务执行次数,用于计时.
static u8 adjErrCount = 0;      //错误计数,如果连续发生5次错误则退出校准.

static T_GPS_POS prePoint;
static T_GPS_POS curPoint;
vu32 myPulseCnt = 0;             //校准计数

u32 gVehicleCoef[SAMPLE_CNT + 1];

static GPS_STRUCT  CurrGps;			//当前有效位置

//****************函数声明*******************

static void ClearAjustFlag(void);
static void setVehicleCoef(u32 AvgPulse);
static void ConverGpsPoing(T_GPS_POS *dstGps,GPS_STRUCT *srcGps);

//****************函数定义******************
/**
  * @brief  脉冲校准初始化。
  * @param  none
  * @retval None
  * @note   注意因为使用了FeatureCoef变量，必须放在Pulse_UpdataPram()函数后面执行。
  */
void Adjust_Init(void)
{
    if((FeatureCoef == 3600)||(FeatureCoef == 7200))   //如果系数=7200或3600,则开启自动自动校准.
    {
        setAdjustFunction(ENABLE);
    }
}
/**
  * @brief  初始化脉冲计数。
  * @param  none
  * @retval None
  */
void resetPulseCnt(void)
{
	myPulseCnt = 0;
}

/**
  * @brief  获取脉冲计数值。
  * @param  none
  * @retval 返回计数值
  */
u32 getPulseCnt(void)
{
    return myPulseCnt;
}

/**
  * @brief  脉冲计数累加。
  * @param  none
  * @retval None
  */
void incPulseCnt(void)
{
	myPulseCnt++;
}

/**
  * @brief  获取自动校准状态标志。
  * @param  none
  * @retval 1：正校准中
  *         0：不在校准状态
  */
u8 ADJ_getAdjFlg(void)
{
	return adjFlg;
}

/**
  * @brief  自动校调参数定时任务。
  * @param  none
  * @retval DISABLE：任务完成
  *         ENABLE ：继续执行
  */
FunctionalState CoeffAdust_TimeTask(void)
{
	u8  i;
	u32 ulAvgPulse;               //计算N次脉冲系数和值
	u32 pulseCnt = 0;
	u32 minMaxPulse;
 static u32 distance = 0;
	u32 disTmp = 0;
        u8  GpsStat = 0;               //GPS导航状态
        u8  GpsSpeed = 0;              //GPS速度 公里/小时
	
	if (0 == adjFlg)
        {
		return ENABLE;
        }
        GpsStat = Gps_ReadStatus();
        //LOG_PR("AdustTask:GpsStat=[%D] \r\n",GpsStat);
        if(GpsStat != 0 )
        {
           GpsSpeed = Gps_ReadSpeed();
	   Gps_CopygPosition(&CurrGps);   //获取GPS数据
           if(GpsSpeed < 5)               //一定要有GPS速度才能开始校准
           {
             ClearAjustFlag();
             return ENABLE;
           }
        }else{
           ClearAjustFlag();             //碰到不导航,需要清除重新计算.
           //LOG_PR_N("AdustTask:No GPS \r\n");
           return ENABLE;
        }
        //LOG_PR("GPS:GPS[%D],Speed[%D],A.%D%D.%04D,N.%D%D.%04D \r\n",GpsStat,GpsSpeed,CurrGps.Latitue_D,CurrGps.Latitue_F,CurrGps.Latitue_FX,CurrGps.Longitue_D,CurrGps.Longitue_F,CurrGps.Longitue_FX);
	if (0 == tmrCnt) {
		if (GpsStat != 0) {
			ConverGpsPoing(&prePoint, &CurrGps);                    //保存GPS开始点
                        //LOG_PR("1.prePoint:Lat=[%D],Lon=[%D]\r\n",(u32)(prePoint.LatDegree*1000000),(u32)(prePoint.LonDegree*1000000));
			resetPulseCnt();                                        //脉冲计数清零
			tmrCnt++;
			distance = 0;                                           //里程计数清零
		}
	} else {
		tmrCnt++;
		if (!(tmrCnt % 3))                                              // 3秒计算一次里程
                {
			if (GpsStat != 0) 
                        {
				ConverGpsPoing(&curPoint, &CurrGps);
                                //LOG_PR("2.curPoint:Lat=[%D],Lon=[%D]\r\n",(u32)(curPoint.LatDegree*1000000),(u32)(curPoint.LonDegree*1000000));
				disTmp = CalDotDotMile(&curPoint, &prePoint);
                                //LOG_PR("3.prePoint:Lat=[%D],Lon=[%D]\r\n",(u32)(prePoint.LatDegree*1000000),(u32)(prePoint.LonDegree*1000000));
                                //LOG_PR("distance=%D,disTmp=%Dm\r\n",distance,disTmp);
				if (disTmp > 0) 
                                {
					distance += disTmp;
					prePoint = curPoint;                    //保存上一个点坐标
				}
			}	
		}
		
		if (tmrCnt >= 40) {                                             //40秒后,
			if (GpsStat != 0) 
                        {
				ConverGpsPoing(&curPoint, &CurrGps);
				disTmp = CalDotDotMile(&curPoint, &prePoint);
				if (disTmp > 0) {
					distance += disTmp;
					prePoint = curPoint;
				}
                                //LOG_PR("distance=%Dm \r\n",distance);
			}
			
			if (distance>=167 && distance<=2000) {	                //时速15 ~ 120 KM/H
				pulseCnt = getPulseCnt();
                                //LOG_PR("pulseCnt=%D \r\n",pulseCnt);
				if (pulseCnt > 10)                              //速度>15Km/h,所以1秒内脉冲计数必须>10
                                {
					gVehicleCoef[ucSumCnt++] = (u32)(pulseCnt * 1000 / distance);  //脉冲个数/公里 =脉冲系数
					if (ucSumCnt == SAMPLE_CNT) {
						ulAvgPulse = 0;
						for (i=0; i<SAMPLE_CNT; i++) {
							ulAvgPulse += gVehicleCoef[i];	
						}
						
						for (i=0; i<SAMPLE_CNT; i++) {  //获取最大值
							if (0 == i) {
								minMaxPulse = gVehicleCoef[i];	
							} else {
								if (gVehicleCoef[i] > minMaxPulse)
                                                                {
									minMaxPulse = gVehicleCoef[i];
                                                                }
							}
						}
						ulAvgPulse -= minMaxPulse;      //减去最大值
						for (i=0; i<SAMPLE_CNT; i++) {
							if (0 == i) {
								minMaxPulse = gVehicleCoef[i];	
							} else {
								if (gVehicleCoef[i] < minMaxPulse)
									minMaxPulse = gVehicleCoef[i];
							}
						}
						ulAvgPulse -= minMaxPulse;      //减去最小值
						ulAvgPulse = ulAvgPulse / (SAMPLE_CNT - 2);  //计算平均值平保存
                                                //LOG_PR("ulAvgPulse=[%D]\r\n",ulAvgPulse);
                                                setVehicleCoef(ulAvgPulse);
						setAdjustFunction(DISABLE);                  //校准完成,则自动停止
					}else {	                                             //准备下一次采集
						tmrCnt = 0;
					}
				}else {		                                             //脉冲数无效 停止自动校调功能 
                                        if(adjErrCount++ > 4)                   //该错误连续发生5次，则退出校准. 
                                        {
					    setAdjustFunction(DISABLE);
                                        }else{
                                            tmrCnt = 0;
                                        }
				}
			}
		}
		if (tmrCnt > 60){
			tmrCnt = 0;
		}
	}
	return ENABLE;
}

/**
  * @brief  设置自动校调标志参数。
  * @param  sta 功能状态
  *              DISABLE 停止校准
  *              ENABLE  开启校准
  * @retval none
  */
void setAdjustFunction(FunctionalState sta)
{
	if ((sta != ENABLE)&&(sta != DISABLE))
        {
		return;
        }
	if (ENABLE == sta) {
		adjFlg   = 1;
		ucSumCnt = 0;
	} else {
		adjFlg   = 0;
	}
        ucSumCnt = 0;
	tmrCnt   = 0;
        adjErrCount = 0;
}

/**
  * @brief  清除所有状态。
  * @param  none
  * @retval none
  * @note   校准中间，碰到不导航的情况，清除状态.
  */
static void ClearAjustFlag(void)
{
    ucSumCnt = 0;
    tmrCnt   = 0;
}
/**
  * @brief  保存脉冲系数。
  * @param  AvgPulse 新的脉冲系数
  * @retval none
  */
static void setVehicleCoef(u32 AvgPulse)
{
        u8 tD2[4];
        u8 *pu32 = (u8*)&AvgPulse;  //特征系数
        tD2[2] = *pu32++;
        tD2[1] = *pu32++;  
        tD2[0] = *pu32++;
        EepromPram_WritePram(E2_CAR_FEATURE_COEF_ID,tD2,3);
        FeatureCoef = AvgPulse;
}
/**
  * @brief  转换GPS经纬度格式为整型。
  * @param  dstGps 转换后的点格式
  * @param  srcGps 原始GPS经纬度格式
  * @retval none
  */
static void ConverGpsPoing(T_GPS_POS *dstGps,GPS_STRUCT *srcGps)
{
    //扩大1000000倍
    dstGps->Lat = ((srcGps->Latitue_FX)*100UL+(srcGps->Latitue_F)*1000000UL)/60+(srcGps->Latitue_D)*1000000UL;
    //扩大1000000倍
    dstGps->Long = ((srcGps->Longitue_FX)*100UL+(srcGps->Longitue_F)*1000000UL)/60+(srcGps->Longitue_D)*1000000UL;
}

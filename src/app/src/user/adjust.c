/********************************************************************
//��Ȩ˵��	: Copyright (c) 2011,�������������¼����������޹�˾
//�ļ�����	: adjust.c		
//����		: �����Զ�У׼
//              : ͨ��GPS�ٶ�������������շ�ʽ���㳵��ϵ��
//              : 1.���У׼��ȡƽ��ֵ.
//�汾��	: V1.0.0
//������	: wangj
//����ʱ��	: 
//�޸���	: yjb
//�޸�ʱ��	:
//�޸ļ�Ҫ˵��	:
//��ע		:1)У׼һ�κ�ʱ:40s*8=320s/60=5.3����,��ΪҪ��GPS���ٶȲŲ��ԣ�ע�⿪ʼ��ʱ��GPS�ٶ�>3��ʼ.
//              : 
***********************************************************************/

//****************�����ļ�*****************
#include "include.h"
#include "adjust.h"

//****************�궨��********************
#define		SAMPLE_CNT		8		  //��������

//****************�ⲿ����*******************
extern u32	FeatureCoef;	                           //ÿ����������(PulseCounter.c �ж���)

//****************ȫ�ֱ���*******************
static vu8 ucSumCnt = 0;	//У׼����
static u8 adjFlg   = 0;		//�����Զ�У׼��־,1-��ʼУ׼,0-ֹͣУ׼
static u8 tmrCnt   = 0;         //����ִ�д���,���ڼ�ʱ.
static u8 adjErrCount = 0;      //�������,�����������5�δ������˳�У׼.

static T_GPS_POS prePoint;
static T_GPS_POS curPoint;
vu32 myPulseCnt = 0;             //У׼����

u32 gVehicleCoef[SAMPLE_CNT + 1];

static GPS_STRUCT  CurrGps;			//��ǰ��Чλ��

//****************��������*******************

static void ClearAjustFlag(void);
static void setVehicleCoef(u32 AvgPulse);
static void ConverGpsPoing(T_GPS_POS *dstGps,GPS_STRUCT *srcGps);

//****************��������******************
/**
  * @brief  ����У׼��ʼ����
  * @param  none
  * @retval None
  * @note   ע����Ϊʹ����FeatureCoef�������������Pulse_UpdataPram()��������ִ�С�
  */
void Adjust_Init(void)
{
    if((FeatureCoef == 3600)||(FeatureCoef == 7200))   //���ϵ��=7200��3600,�����Զ��Զ�У׼.
    {
        setAdjustFunction(ENABLE);
    }
}
/**
  * @brief  ��ʼ�����������
  * @param  none
  * @retval None
  */
void resetPulseCnt(void)
{
	myPulseCnt = 0;
}

/**
  * @brief  ��ȡ�������ֵ��
  * @param  none
  * @retval ���ؼ���ֵ
  */
u32 getPulseCnt(void)
{
    return myPulseCnt;
}

/**
  * @brief  ��������ۼӡ�
  * @param  none
  * @retval None
  */
void incPulseCnt(void)
{
	myPulseCnt++;
}

/**
  * @brief  ��ȡ�Զ�У׼״̬��־��
  * @param  none
  * @retval 1����У׼��
  *         0������У׼״̬
  */
u8 ADJ_getAdjFlg(void)
{
	return adjFlg;
}

/**
  * @brief  �Զ�У��������ʱ����
  * @param  none
  * @retval DISABLE���������
  *         ENABLE ������ִ��
  */
FunctionalState CoeffAdust_TimeTask(void)
{
	u8  i;
	u32 ulAvgPulse;               //����N������ϵ����ֵ
	u32 pulseCnt = 0;
	u32 minMaxPulse;
 static u32 distance = 0;
	u32 disTmp = 0;
        u8  GpsStat = 0;               //GPS����״̬
        u8  GpsSpeed = 0;              //GPS�ٶ� ����/Сʱ
	
	if (0 == adjFlg)
        {
		return ENABLE;
        }
        GpsStat = Gps_ReadStatus();
        //LOG_PR("AdustTask:GpsStat=[%D] \r\n",GpsStat);
        if(GpsStat != 0 )
        {
           GpsSpeed = Gps_ReadSpeed();
	   Gps_CopygPosition(&CurrGps);   //��ȡGPS����
           if(GpsSpeed < 5)               //һ��Ҫ��GPS�ٶȲ��ܿ�ʼУ׼
           {
             ClearAjustFlag();
             return ENABLE;
           }
        }else{
           ClearAjustFlag();             //����������,��Ҫ������¼���.
           //LOG_PR_N("AdustTask:No GPS \r\n");
           return ENABLE;
        }
        //LOG_PR("GPS:GPS[%D],Speed[%D],A.%D%D.%04D,N.%D%D.%04D \r\n",GpsStat,GpsSpeed,CurrGps.Latitue_D,CurrGps.Latitue_F,CurrGps.Latitue_FX,CurrGps.Longitue_D,CurrGps.Longitue_F,CurrGps.Longitue_FX);
	if (0 == tmrCnt) {
		if (GpsStat != 0) {
			ConverGpsPoing(&prePoint, &CurrGps);                    //����GPS��ʼ��
                        //LOG_PR("1.prePoint:Lat=[%D],Lon=[%D]\r\n",(u32)(prePoint.LatDegree*1000000),(u32)(prePoint.LonDegree*1000000));
			resetPulseCnt();                                        //�����������
			tmrCnt++;
			distance = 0;                                           //��̼�������
		}
	} else {
		tmrCnt++;
		if (!(tmrCnt % 3))                                              // 3�����һ�����
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
					prePoint = curPoint;                    //������һ��������
				}
			}	
		}
		
		if (tmrCnt >= 40) {                                             //40���,
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
			
			if (distance>=167 && distance<=2000) {	                //ʱ��15 ~ 120 KM/H
				pulseCnt = getPulseCnt();
                                //LOG_PR("pulseCnt=%D \r\n",pulseCnt);
				if (pulseCnt > 10)                              //�ٶ�>15Km/h,����1���������������>10
                                {
					gVehicleCoef[ucSumCnt++] = (u32)(pulseCnt * 1000 / distance);  //�������/���� =����ϵ��
					if (ucSumCnt == SAMPLE_CNT) {
						ulAvgPulse = 0;
						for (i=0; i<SAMPLE_CNT; i++) {
							ulAvgPulse += gVehicleCoef[i];	
						}
						
						for (i=0; i<SAMPLE_CNT; i++) {  //��ȡ���ֵ
							if (0 == i) {
								minMaxPulse = gVehicleCoef[i];	
							} else {
								if (gVehicleCoef[i] > minMaxPulse)
                                                                {
									minMaxPulse = gVehicleCoef[i];
                                                                }
							}
						}
						ulAvgPulse -= minMaxPulse;      //��ȥ���ֵ
						for (i=0; i<SAMPLE_CNT; i++) {
							if (0 == i) {
								minMaxPulse = gVehicleCoef[i];	
							} else {
								if (gVehicleCoef[i] < minMaxPulse)
									minMaxPulse = gVehicleCoef[i];
							}
						}
						ulAvgPulse -= minMaxPulse;      //��ȥ��Сֵ
						ulAvgPulse = ulAvgPulse / (SAMPLE_CNT - 2);  //����ƽ��ֵƽ����
                                                //LOG_PR("ulAvgPulse=[%D]\r\n",ulAvgPulse);
                                                setVehicleCoef(ulAvgPulse);
						setAdjustFunction(DISABLE);                  //У׼���,���Զ�ֹͣ
					}else {	                                             //׼����һ�βɼ�
						tmrCnt = 0;
					}
				}else {		                                             //��������Ч ֹͣ�Զ�У������ 
                                        if(adjErrCount++ > 4)                   //�ô�����������5�Σ����˳�У׼. 
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
  * @brief  �����Զ�У����־������
  * @param  sta ����״̬
  *              DISABLE ֹͣУ׼
  *              ENABLE  ����У׼
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
  * @brief  �������״̬��
  * @param  none
  * @retval none
  * @note   У׼�м䣬��������������������״̬.
  */
static void ClearAjustFlag(void)
{
    ucSumCnt = 0;
    tmrCnt   = 0;
}
/**
  * @brief  ��������ϵ����
  * @param  AvgPulse �µ�����ϵ��
  * @retval none
  */
static void setVehicleCoef(u32 AvgPulse)
{
        u8 tD2[4];
        u8 *pu32 = (u8*)&AvgPulse;  //����ϵ��
        tD2[2] = *pu32++;
        tD2[1] = *pu32++;  
        tD2[0] = *pu32++;
        EepromPram_WritePram(E2_CAR_FEATURE_COEF_ID,tD2,3);
        FeatureCoef = AvgPulse;
}
/**
  * @brief  ת��GPS��γ�ȸ�ʽΪ���͡�
  * @param  dstGps ת����ĵ��ʽ
  * @param  srcGps ԭʼGPS��γ�ȸ�ʽ
  * @retval none
  */
static void ConverGpsPoing(T_GPS_POS *dstGps,GPS_STRUCT *srcGps)
{
    //����1000000��
    dstGps->Lat = ((srcGps->Latitue_FX)*100UL+(srcGps->Latitue_F)*1000000UL)/60+(srcGps->Latitue_D)*1000000UL;
    //����1000000��
    dstGps->Long = ((srcGps->Longitue_FX)*100UL+(srcGps->Longitue_F)*1000000UL)/60+(srcGps->Longitue_D)*1000000UL;
}

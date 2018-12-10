/********************************************************************
//��Ȩ˵��	:Shenzhen E-EYE Co. Ltd., Copyright 2009- All Rights Reserved.
//�ļ�����	:SpeedLog.c		
//����		:ʵ���ٶ��쳣��¼����
//�汾��	:
//������	:
//����ʱ��	:
//�޸���	: 
//�޸�ʱ��	:
//�޸ļ�Ҫ˵��	:
//��ע		:
***********************************************************************/
//***************�����ļ�*****************
#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "include.h"
//****************�궨��****************
#define   DEBUG_SPEED_STATUS            0


//***************��������***************

//***************��������**************
u8  h_LogSpeedFlg;//��׼Ҫ��1����������һ�Σ�����Ϊ�˼�ⷽ������ÿ���ϵ���1��
u8  h_TTsFlg;     //�ٶ��쳣����������־

//***************��������***************


/*******************************************************************************
* Function Name  : SpeedLog_TTs
* Description    : ���ٶ��쳣������������
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SpeedLog_TTs(void)
{
    static u16  TimeCnt;
    static u8   SpeakNum;
    if((TimeCnt++) > 200)/*10s����һ��*/
    {
        TimeCnt = 0;
				//����Ҫ���ٶ��쳣ʱ����������
        //Public_PlayTTSVoiceAlarmStr("��ע�⣬�����ٶ��쳣");
        if((++SpeakNum) > 2)
        {
            SpeakNum = 0;
            h_TTsFlg = 0;
        }
    }

}
/*********************************************************************
//��������	:SpeedLog_TimeTask(void)
//����		:��¼�ٶ��쳣
//		:
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:50ms�����1��
*********************************************************************/
FunctionalState SpeedLog_TimeTask(void)
{

    static u8 Buffer1[REGISTER_SPEED_STATUS_STEP_LEN] = {0};
    static u8 Buffer2[REGISTER_SPEED_STATUS_STEP_LEN] = {0};   
    static u16 SpeedErrCnt = 0;
    static u16 SpeedOkCnt = 0;
    static u16 SpeedErrStep = 0;
    static u16 SpeedOkStep = 0;

    
    u8  GpsSpeed;
    u8  PulseSpeed;
    u8  Speed;
    u8  nData;
    
    u8  nTab[10];
    
    float val;
    TIME_T tt;
    u32 TimeCount;  

		//��ΪRTC���û�絼����GPSʱ��У׼֮ǰ�����ʱ����ҵ����⣬������û��GPSУʱ֮ǰ���洢��ʻ��¼�����ݣ���Ҳ��û�а취�İ취....
		if(!Gps_GetRtcAdjSta())
			return ENABLE;
    memset(nTab,0,sizeof(nTab));
    FRAM_BufferRead(nTab,7,FRAM_SPEED_STATUS_ADDR);
    
    RTC_GetCurTime(&tt);
    nData = (((nTab[3]&0xf0)>>4)*10) + (nTab[3]&0x0f); 
    
    if(h_TTsFlg)
    {
        SpeedLog_TTs();
    }
    if(nData == tt.day)
    {        
        return ENABLE;
    }
    else
    {
        if(h_LogSpeedFlg)
        {
            h_LogSpeedFlg = 0;
        }
    }
    
    if(1 == h_LogSpeedFlg)//�Ѽ�¼��һ���ٶ�״̬
    {
        return ENABLE;
    }
    
    //��ȡgps�ٶ�,���ο��ٶ�
    GpsSpeed = Gps_ReadSpeed();
    
    //��ȡ�����ٶ�
    PulseSpeed = Pulse_GetSpeed();
		
    //�жϲο��ٶ��Ƿ����40
#if (DEBUG_SPEED_STATUS)
    PulseSpeed = 110;
    if((PulseSpeed > 109)&&(PulseSpeed < 111))/*����110����*/
#else    
    if((Io_ReadStatusBit(STATUS_BIT_NAVIGATION))&&(GpsSpeed >= 40))
#endif    
    {
	if(PulseSpeed >= GpsSpeed)
	{
	    Speed = PulseSpeed - GpsSpeed;
	}
	else
	{  
	    Speed = GpsSpeed - PulseSpeed;
	}
	val = (float)Speed/(float)GpsSpeed;		

#if (DEBUG_SPEED_STATUS)
      val = 0.21;  
#endif	        
        if(val > 0.11)//�ٶ��쳣
        {
            SpeedOkCnt = 0;	
            SpeedOkStep = 0;   
	    SpeedErrCnt++;
                        
            if(SpeedErrCnt > 20)//1���Ӽ�¼һ��
	    {
	        SpeedErrCnt = 0;            
                
                if(SpeedErrStep < 60)     /*ֻ��¼��ʼʱ���1��������*/
                {
                    Buffer1[13+2*SpeedErrStep] = PulseSpeed;//ǰ��13���ֽ�Ԥ������
                    Buffer1[13+2*SpeedErrStep+1] = GpsSpeed;
		}                	
#if (DEBUG_SPEED_STATUS)    
                if(SpeedErrStep >= 5)
#else
                if(SpeedErrStep >= 299)            
#endif              
		{
		    SpeedErrStep = 0;
		    //����ǰʱ��
		    TimeCount = RTC_GetCounter();
		    //�ٶ�״̬
		    Buffer1[0] = 2;
		    //����ʱ��
		    Gmtime(&tt, TimeCount);
		    Buffer1[7] = Public_HEX2BCD(tt.year);
		    Buffer1[8] = Public_HEX2BCD(tt.month);
		    Buffer1[9] = Public_HEX2BCD(tt.day);
		    Buffer1[10] = Public_HEX2BCD(tt.hour);
		    Buffer1[11] = Public_HEX2BCD(tt.min);
		    Buffer1[12] = Public_HEX2BCD(tt.sec);                    
                    
                    //��ʼʱ��
		    TimeCount -= 300;
                    Gmtime(&tt, TimeCount);
                    Buffer1[1] = Public_HEX2BCD(tt.year);
                    Buffer1[2] = Public_HEX2BCD(tt.month);
                    Buffer1[3] = Public_HEX2BCD(tt.day);
                    Buffer1[4] = Public_HEX2BCD(tt.hour);
                    Buffer1[5] = Public_HEX2BCD(tt.min);
                    Buffer1[6] = Public_HEX2BCD(tt.sec);                                            	   
                    
                    memset(nTab,0,sizeof(nTab));
                    nTab[0] = 1;                   //�ٶ��쳣
                    memcpy(nTab+1,Buffer1+1,6);                                                       
                    FRAM_BufferWrite(FRAM_SPEED_STATUS_ADDR,nTab,7);
                    
                    //����ǰʱ��
		    TimeCount = RTC_GetCounter();
                    TimeCount--;//��1��    
                    //����д�뵽flash
                    h_LogSpeedFlg = 1;
                    
		    if(ERROR == Register_Write2(REGISTER_TYPE_SPEED_STATUS,Buffer1, REGISTER_SPEED_STATUS_STEP_LEN-5,TimeCount))
                    {
                        Register_Write2(REGISTER_TYPE_SPEED_STATUS,Buffer1, REGISTER_SPEED_STATUS_STEP_LEN-5,TimeCount);
                    }
                    h_TTsFlg = 1;
                    Public_PlayTTSVoiceAlarmStr("��ע�⣬�����ٶ��쳣");
                    //Public_ShowTextInfo("�ٶ��쳣������",100);
		}
                SpeedErrStep++;/*ÿ���1*/
            }
        }
	else//�ٶ�����
	{
	    SpeedErrCnt = 0;
	    SpeedErrStep = 0;
	    SpeedOkCnt++;
            if(SpeedOkCnt > 20)//1���Ӽ�¼һ��
	    {
                SpeedOkCnt = 0;
		if(SpeedOkCnt < 60)
                {
                    Buffer2[13+2*SpeedOkStep] = PulseSpeed;//ǰ��13���ֽ�Ԥ������
                    Buffer2[13+2*SpeedOkStep+1] = GpsSpeed;
                }
				  
#if (DEBUG_SPEED_STATUS)    
                if(SpeedOkStep >= 5)
#else
                if(SpeedOkStep >= 299)            
#endif
		{
		    SpeedOkStep = 0;
                    
                    //����ǰʱ��
		    TimeCount = RTC_GetCounter();
		    //�ٶ�״̬
		    Buffer2[0] = 1;
		    //����ʱ��
		    Gmtime(&tt, TimeCount);
		    Buffer2[7] = Public_HEX2BCD(tt.year);
		    Buffer2[8] = Public_HEX2BCD(tt.month);
		    Buffer2[9] = Public_HEX2BCD(tt.day);
		    Buffer2[10] = Public_HEX2BCD(tt.hour);
		    Buffer2[11] = Public_HEX2BCD(tt.min);
		    Buffer2[12] = Public_HEX2BCD(tt.sec);                    
                    
                    //��ʼʱ��
		    TimeCount -= 300;
                    Gmtime(&tt, TimeCount);
                    Buffer2[1] = Public_HEX2BCD(tt.year);
                    Buffer2[2] = Public_HEX2BCD(tt.month);
                    Buffer2[3] = Public_HEX2BCD(tt.day);
                    Buffer2[4] = Public_HEX2BCD(tt.hour);
                    Buffer2[5] = Public_HEX2BCD(tt.min);
                    Buffer2[6] = Public_HEX2BCD(tt.sec);    	                      

                    memset(nTab,0,sizeof(nTab));
                    nTab[0] = 0;                   //�ٶ�����
                    memcpy(nTab+1,Buffer2+1,6);                                                       
                    FRAM_BufferWrite(FRAM_SPEED_STATUS_ADDR,nTab,7);
                    
                    //����ǰʱ��
		    TimeCount = RTC_GetCounter();
                    TimeCount--;//��1��
		    //����д�뵽flash
                    h_LogSpeedFlg = 1;
		    if(ERROR ==Register_Write2(REGISTER_TYPE_SPEED_STATUS,Buffer2, REGISTER_SPEED_STATUS_STEP_LEN-5,TimeCount))
                    {
                        Register_Write2(REGISTER_TYPE_SPEED_STATUS,Buffer2, REGISTER_SPEED_STATUS_STEP_LEN-5,TimeCount);
                    }
                    //Public_ShowTextInfo("�ٶ�����",100);
		}
                SpeedOkStep++;/*ÿ���1*/
            }
        }
    }
    else
    {
	SpeedErrCnt = 0;	
	SpeedErrStep = 0;
        SpeedOkCnt = 0;	
	SpeedOkStep = 0;           		        		
    }
    return ENABLE;
}


/*******************************************************************************
* Function Name  : GetSpeedLogStatus
* Description    : ���ص�ǰ�ٶ�״̬
* Input          : None            
* Output         : None
* Return         : None
*******************************************************************************/
u8 GetSpeedLogStatus(void)
{
    u8 nTab[10];
    
    memset(nTab,0,sizeof(nTab));
    FRAM_BufferRead(nTab,7,FRAM_SPEED_STATUS_ADDR);
    
    if((nTab[0] != 0)&&(nTab[0] != 1))
    {
        nTab[0] = 0;
        FRAM_BufferWrite(FRAM_SPEED_STATUS_ADDR,nTab,1);
    }
      
    return nTab[0];
}

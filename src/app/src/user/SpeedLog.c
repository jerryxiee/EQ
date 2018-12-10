/********************************************************************
//版权说明	:Shenzhen E-EYE Co. Ltd., Copyright 2009- All Rights Reserved.
//文件名称	:SpeedLog.c		
//功能		:实现速度异常记录功能
//版本号	:
//开发人	:
//开发时间	:
//修改者	: 
//修改时间	:
//修改简要说明	:
//备注		:
***********************************************************************/
//***************包含文件*****************
#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "include.h"
//****************宏定义****************
#define   DEBUG_SPEED_STATUS            0


//***************常量定义***************

//***************变量定义**************
u8  h_LogSpeedFlg;//标准要求1个日历天检测一次，现在为了检测方便做成每次上电检测1次
u8  h_TTsFlg;     //速度异常语音播报标志

//***************函数定义***************


/*******************************************************************************
* Function Name  : SpeedLog_TTs
* Description    : 记速度异常语音播报函数
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SpeedLog_TTs(void)
{
    static u16  TimeCnt;
    static u8   SpeakNum;
    if((TimeCnt++) > 200)/*10s播报一次*/
    {
        TimeCnt = 0;
				//广州要求速度异常时不播报语音
        //Public_PlayTTSVoiceAlarmStr("请注意，车辆速度异常");
        if((++SpeakNum) > 2)
        {
            SpeakNum = 0;
            h_TTsFlg = 0;
        }
    }

}
/*********************************************************************
//函数名称	:SpeedLog_TimeTask(void)
//功能		:记录速度异常
//		:
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:50ms秒调度1次
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

		//因为RTC电池没电导致在GPS时间校准之前会出现时间错乱的问题，所以在没有GPS校时之前不存储行驶记录仪数据，这也是没有办法的办法....
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
    
    if(1 == h_LogSpeedFlg)//已记录了一次速度状态
    {
        return ENABLE;
    }
    
    //读取gps速度,即参考速度
    GpsSpeed = Gps_ReadSpeed();
    
    //读取脉冲速度
    PulseSpeed = Pulse_GetSpeed();
		
    //判断参考速度是否大于40
#if (DEBUG_SPEED_STATUS)
    PulseSpeed = 110;
    if((PulseSpeed > 109)&&(PulseSpeed < 111))/*恒速110测试*/
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
        if(val > 0.11)//速度异常
        {
            SpeedOkCnt = 0;	
            SpeedOkStep = 0;   
	    SpeedErrCnt++;
                        
            if(SpeedErrCnt > 20)//1秒钟记录一个
	    {
	        SpeedErrCnt = 0;            
                
                if(SpeedErrStep < 60)     /*只记录开始时间后1分钟数据*/
                {
                    Buffer1[13+2*SpeedErrStep] = PulseSpeed;//前面13个字节预留出来
                    Buffer1[13+2*SpeedErrStep+1] = GpsSpeed;
		}                	
#if (DEBUG_SPEED_STATUS)    
                if(SpeedErrStep >= 5)
#else
                if(SpeedErrStep >= 299)            
#endif              
		{
		    SpeedErrStep = 0;
		    //读当前时间
		    TimeCount = RTC_GetCounter();
		    //速度状态
		    Buffer1[0] = 2;
		    //结束时间
		    Gmtime(&tt, TimeCount);
		    Buffer1[7] = Public_HEX2BCD(tt.year);
		    Buffer1[8] = Public_HEX2BCD(tt.month);
		    Buffer1[9] = Public_HEX2BCD(tt.day);
		    Buffer1[10] = Public_HEX2BCD(tt.hour);
		    Buffer1[11] = Public_HEX2BCD(tt.min);
		    Buffer1[12] = Public_HEX2BCD(tt.sec);                    
                    
                    //开始时间
		    TimeCount -= 300;
                    Gmtime(&tt, TimeCount);
                    Buffer1[1] = Public_HEX2BCD(tt.year);
                    Buffer1[2] = Public_HEX2BCD(tt.month);
                    Buffer1[3] = Public_HEX2BCD(tt.day);
                    Buffer1[4] = Public_HEX2BCD(tt.hour);
                    Buffer1[5] = Public_HEX2BCD(tt.min);
                    Buffer1[6] = Public_HEX2BCD(tt.sec);                                            	   
                    
                    memset(nTab,0,sizeof(nTab));
                    nTab[0] = 1;                   //速度异常
                    memcpy(nTab+1,Buffer1+1,6);                                                       
                    FRAM_BufferWrite(FRAM_SPEED_STATUS_ADDR,nTab,7);
                    
                    //读当前时间
		    TimeCount = RTC_GetCounter();
                    TimeCount--;//减1秒    
                    //数据写入到flash
                    h_LogSpeedFlg = 1;
                    
		    if(ERROR == Register_Write2(REGISTER_TYPE_SPEED_STATUS,Buffer1, REGISTER_SPEED_STATUS_STEP_LEN-5,TimeCount))
                    {
                        Register_Write2(REGISTER_TYPE_SPEED_STATUS,Buffer1, REGISTER_SPEED_STATUS_STEP_LEN-5,TimeCount);
                    }
                    h_TTsFlg = 1;
                    Public_PlayTTSVoiceAlarmStr("请注意，车辆速度异常");
                    //Public_ShowTextInfo("速度异常！！！",100);
		}
                SpeedErrStep++;/*每秒加1*/
            }
        }
	else//速度正常
	{
	    SpeedErrCnt = 0;
	    SpeedErrStep = 0;
	    SpeedOkCnt++;
            if(SpeedOkCnt > 20)//1秒钟记录一个
	    {
                SpeedOkCnt = 0;
		if(SpeedOkCnt < 60)
                {
                    Buffer2[13+2*SpeedOkStep] = PulseSpeed;//前面13个字节预留出来
                    Buffer2[13+2*SpeedOkStep+1] = GpsSpeed;
                }
				  
#if (DEBUG_SPEED_STATUS)    
                if(SpeedOkStep >= 5)
#else
                if(SpeedOkStep >= 299)            
#endif
		{
		    SpeedOkStep = 0;
                    
                    //读当前时间
		    TimeCount = RTC_GetCounter();
		    //速度状态
		    Buffer2[0] = 1;
		    //结束时间
		    Gmtime(&tt, TimeCount);
		    Buffer2[7] = Public_HEX2BCD(tt.year);
		    Buffer2[8] = Public_HEX2BCD(tt.month);
		    Buffer2[9] = Public_HEX2BCD(tt.day);
		    Buffer2[10] = Public_HEX2BCD(tt.hour);
		    Buffer2[11] = Public_HEX2BCD(tt.min);
		    Buffer2[12] = Public_HEX2BCD(tt.sec);                    
                    
                    //开始时间
		    TimeCount -= 300;
                    Gmtime(&tt, TimeCount);
                    Buffer2[1] = Public_HEX2BCD(tt.year);
                    Buffer2[2] = Public_HEX2BCD(tt.month);
                    Buffer2[3] = Public_HEX2BCD(tt.day);
                    Buffer2[4] = Public_HEX2BCD(tt.hour);
                    Buffer2[5] = Public_HEX2BCD(tt.min);
                    Buffer2[6] = Public_HEX2BCD(tt.sec);    	                      

                    memset(nTab,0,sizeof(nTab));
                    nTab[0] = 0;                   //速度正常
                    memcpy(nTab+1,Buffer2+1,6);                                                       
                    FRAM_BufferWrite(FRAM_SPEED_STATUS_ADDR,nTab,7);
                    
                    //读当前时间
		    TimeCount = RTC_GetCounter();
                    TimeCount--;//减1秒
		    //数据写入到flash
                    h_LogSpeedFlg = 1;
		    if(ERROR ==Register_Write2(REGISTER_TYPE_SPEED_STATUS,Buffer2, REGISTER_SPEED_STATUS_STEP_LEN-5,TimeCount))
                    {
                        Register_Write2(REGISTER_TYPE_SPEED_STATUS,Buffer2, REGISTER_SPEED_STATUS_STEP_LEN-5,TimeCount);
                    }
                    //Public_ShowTextInfo("速度正常",100);
		}
                SpeedOkStep++;/*每秒加1*/
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
* Description    : 返回当前速度状态
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

/*******************************************************************************
*版权说明	:Shenzhen E-EYE Co. Ltd., Copyright 2009- All Rights Reserved.
*文件名称	:position.c		
*功能		:记录仪位置信息采集
*版本号	:
*开发人	:       :myh
*开发时间	:2013.3 
*修改者	:
*修改时间	:
*修改简要说明	:
*备注		:
*******************************************************************************/
#include "include.h"
#include "position.h"

extern u8 SpeedFlag;//dxl,2015.5.11,0x00:手动脉冲,0x01:手动GPS,0x02:自动脉冲,0x03:自动GPS

/*******************************************************************************
* Function Name  : Position_TimeTask
* Description    : 位置信息采集事件处理,每1min调用一次 .
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
FunctionalState Position_TimeTask(void)
{    
    u8 nTab[19];/*10个位置信息 + 1个平均速度 + 8保留位*/
    u8 nSpeed;
    u8 nStep;
    
    u32	CurrentTimeCount;
    u32	CurrentTimeCnt;
    
    static  u32	TimeCount = 0;
    static  u8  CheckFlg = 0;
		
    if(CheckFlg == 0)
    {
        if(Register_CheckOneArea2(REGISTER_TYPE_POSITION))
        {
            CheckFlg = 1;//结束
        }
        return ENABLE;
    }
    //因为RTC电池没电导致在GPS时间校准之前会出现时间错乱的问题，所以在没有GPS校时之前不存储行驶记录仪数据，这也是没有办法的办法....
		if(!Gps_GetRtcAdjSta())
			return ENABLE;
    CurrentTimeCount = Timer_Val();
    
    if((CurrentTimeCount - TimeCount) >= SYSTICK_1MINUTE)
    {
        TimeCount = Timer_Val();
        
        memset(nTab,0,sizeof(nTab));                       /*清空缓存区           */
        
        if(SpeedMonitor_GetCurRunStatus())
        {
            /*...单位分钟位置信息取该分钟范围内首个有效的位置信息，如该分钟范围内无有效的位置信息，则该分钟位置信息为 7FFFFFFFH...*/
            if(Io_ReadStatusBit(STATUS_BIT_NAVIGATION))
            {
                Public_GetCurPositionInfoDataBlock(nTab);          /*复制记录仪位置数据   */
            }
            else
            {
                memset(nTab,0xff,8);  
                nTab[0] = 0x7F;
                nTab[4] = 0x7F;          
            }
            if(0 == (SpeedFlag&0x01))//脉冲速度
            {
                nSpeed = Pulse_GetMinuteSpeed();
            }
            else//GPS速度
            {
                nSpeed = MileMeter_GetMinuteSpeed();                 /*1分钟平均速度        */
            }
            
            CurrentTimeCnt = RTC_GetCounter();
                       
            nTab[10] = nSpeed;
            if(ERROR == Register_Write2(REGISTER_TYPE_POSITION,nTab,19,CurrentTimeCnt))/*存储到flash*/                  
            {
							Register_EraseOneArea(REGISTER_TYPE_POSITION);
            }
            ////////////////////////////////////////////////////////////////////     
            memset(nTab,0,sizeof(nTab));//存储数据前清空数组 
            
            FRAM_BufferRead(&nStep, 1, FRAM_STOP_15MINUTE_SPEED_ADDR);//读取单步
            
            if(nStep > 14)//检查是否存满
            {
                nStep = 0;
            }           
            
            Public_ConvertNowTimeToBCDEx(nTab);//存储BCD码时间 0-5
            nTab[5] = nSpeed;//覆盖秒,记录速度            

            FRAM_BufferWrite(FRAM_STOP_15MINUTE_SPEED_ADDR+2+(nStep*4), nTab+3, 3);//从小时记录，速度覆盖秒
            nStep++;//增加单步
            FRAM_BufferWrite(FRAM_STOP_15MINUTE_SPEED_ADDR, &nStep, 1);//保存单步
        }
    }
    
    return ENABLE;
}




     
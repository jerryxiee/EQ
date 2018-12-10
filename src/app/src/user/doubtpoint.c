/*******************************************************************************
*版权说明	:Shenzhen E-EYE Co. Ltd., Copyright 2009- All Rights Reserved.
*文件名称	:doubtpoint.c		
*功能		:记录仪位疑点数据采集
*版本号	:
*开发人	:       :myh
*开发时间	:2013.3
*修改者	:
*修改时间	:
*修改简要说明	:
*备注		:
*******************************************************************************/
#include "include.h"
#include "doubtpoint.h"


/**********************************宏定义**************************************/ 
#define      Debug_DoubtPoint                    0    /*事故疑点模块调试开关*/


static u8   nCarSta;        /*车辆行驶状态                                */
static u8   nRunCnt;        /*车辆行驶计数器，即速度非0                   */
static u8   nStopCnt;       /*车辆停止计数器，即速度为0                   */

/*******************************************************************************
* Function Name  : DoubtPoint_GetSpeed
* Description    : 事故疑点模块获取速度
* Input          : None
* Output         : None
* Return         : 返回速度
*******************************************************************************/
u8 DoubtPoint_GetSpeed(void)
{
    
#if (Debug_DoubtPoint)
    static u8 nFctCnt;
    return ++nFctCnt;
#else
    return SpeedMonitor_GetCurSpeed();
#endif
}
/*******************************************************************************
* Function Name  : DoubtPoint_GetAccStatus
* Description    : 事故疑点模块获取ACC状态
* Input          : None
* Output         : None
* Return         : 返回ACC状态
*******************************************************************************/
u8 DoubtPoint_GetAccStatus(void)
{
    
#if (Debug_DoubtPoint)
    return 1;
#else
    return Io_ReadStatusBit(STATUS_BIT_ACC);
#endif
}
/*******************************************************************************
* Function Name  : DoubtPoint_Init
* Description    : 疑点数据初始化,主要完成断电钱疑点数据的存储.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DoubtPoint_Init(void)
{
    u16 nFmStep;
    u8  nFmBuff[470];
    u8  nTab[310];
    u8  nBuff[310];     /*30s速度和状态信号，共150组                  */
    u32	 CurrentTimeCount;
    TIME_T  h_Time;
    
    u16 i=0,j=0;  
    
    memset(nBuff,0,sizeof(nBuff));
            
    for(i=0,j=0;i<300;i+=2,j+=3)                               /*从铁电中读取数据              */
    {
        FRAM_BufferRead(nBuff+i,2,FRAM_DOUBT_POINT_ADDR+9+j);            
    }
    
    FRAM_BufferRead(nFmBuff, 1, FRAM_DOUBT_POINT_ADDR);        /*从铁电读取疑点单步            */                    
    FRAM_BufferRead(nFmBuff+2, 6, FRAM_DOUBT_POINT_ADDR+2);    /*从铁电读取疑点时间            */ 
    
    RTC_GetCurTime(&h_Time);
      
    h_Time.year  = (((nFmBuff[2]&0xf0)>>4)*10) + (nFmBuff[2]&0x0f);
    h_Time.month = (((nFmBuff[3]&0xf0)>>4)*10) + (nFmBuff[3]&0x0f);
    h_Time.day   = (((nFmBuff[4]&0xf0)>>4)*10) + (nFmBuff[4]&0x0f);
    h_Time.hour  = (((nFmBuff[5]&0xf0)>>4)*10) + (nFmBuff[5]&0x0f);
    h_Time.min   = (((nFmBuff[6]&0xf0)>>4)*10) + (nFmBuff[6]&0x0f);
    h_Time.sec   = (((nFmBuff[7]&0xf0)>>4)*10) + (nFmBuff[7]&0x0f);
      
      
    if(CheckTimeStruct(&h_Time) == SUCCESS)
    {
       
        nFmStep = nFmBuff[0]*2;
        
        memset(nTab,0,sizeof(nTab));
        
        nFmStep = nFmStep - 12;                                    /*根据检测中心测试需把数据提前1.2s*/
        
        for(i=0,j=nFmStep-1;i<300;i+=2,j-=2)                       /*倒序存储30s数据               */
        {
            if(j > 299)
            {
                j = 299;
            }
            nTab[i] = nBuff[j-1];
            nTab[i+1] = nBuff[j];
        } 
        memcpy(nBuff,nFmBuff,9);                                   /*nBuff暂存单步和时间           */
                         
        memset(nFmBuff,0,sizeof(nFmBuff));                         /*清零缓存区                    */
        memcpy(nFmBuff,nBuff+2,6);                                 /*向nFmBuff中写入时间           */
        nFmBuff[5] = nFmBuff[5] - 1;                               /*结束时间减1S*/
        
        if(EepromPram_ReadPram(E2_CURRENT_LICENSE_ID,nFmBuff+6)==0)/*复制机动车驾驶证号            */
        {                                                            
            memset(nFmBuff+6,0,18);                                /*读取失败填充0表示未知         */
        }
               
        memcpy(nFmBuff+24,nTab,200);                               /*存储包含停车前10s数据         */             
                   
        FRAM_BufferRead(nFmBuff+224, 10, FRAM_DOUBT_POSITION_ADDR);/*取结束时的位置信息            */        
        
        CurrentTimeCount = RTC_GetCounter();
        Register_Write2(REGISTER_TYPE_DOUBT,nFmBuff,246,CurrentTimeCount);
        
    }

    FRAM_BufferWrite(FRAM_DOUBT_POINT_ADDR,0,FRAM_DOUBT_POINT_LEN);
    FRAM_BufferRead(nFmBuff,460,FRAM_DOUBT_POINT_ADDR);
}
/*******************************************************************************
* Function Name  : DoubtPowerSave
* Description    : 设备主电源断开后,保存断电前20S数据
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DoubtPowerSave(void)
{
    u16 nFmStep;
    u8  nFmBuff[470];
    u8  nTab[310];
    u8  nBuff[310];     /*30s速度和状态信号，共150组                  */
    
    u16 i=0,j=0;  
    u32	 CurrentTimeCount;
    
    memset(nBuff,0,sizeof(nBuff));
            
    for(i=0,j=0;i<300;i+=2,j+=3)                               /*从铁电中读取数据              */
    {
        FRAM_BufferRead(nBuff+i,2,FRAM_DOUBT_POINT_ADDR+9+j);            
    }
    
    FRAM_BufferRead(nFmBuff, 1, FRAM_DOUBT_POINT_ADDR);        /*从铁电读取疑点单步            */                    
    FRAM_BufferRead(nFmBuff+2, 6, FRAM_DOUBT_POINT_ADDR+2);    /*从铁电读取疑点时间            */ 
       
    nFmStep = nFmBuff[0]*2;
    
    memset(nTab,0,sizeof(nTab));
    
    nFmStep = nFmStep - 16;                                    /*根据检测中心测试需把数据提前1.2s*/
    
    for(i=0,j=nFmStep-1;i<300;i+=2,j-=2)                       /*倒序存储30s数据               */
    {
        if(j > 299)
        {
            j = 299;
        }
        nTab[i] = nBuff[j-1];
        nTab[i+1] = nBuff[j];
    } 
    memcpy(nBuff,nFmBuff,9);                                   /*nBuff暂存单步和时间           */
                                                                     
    memset(nFmBuff,0,sizeof(nFmBuff));                         /*清零缓存区                    */
    memcpy(nFmBuff,nBuff+2,6);                                 /*向nFmBuff中写入时间           */
     
    if(EepromPram_ReadPram(E2_CURRENT_LICENSE_ID,nFmBuff+6)==0)/*复制机动车驾驶证号            */
    {                                                            
        memset(nFmBuff+6,0,18);                                /*读取失败填充0表示未知         */
    }
                             
    memcpy(nFmBuff+24,nTab,200);                               /*存储包含停车前10s数据         */             
    
    if(Io_ReadStatusBit(STATUS_BIT_NAVIGATION))
    {
        Public_GetCurPositionInfoDataBlock(nFmBuff+224);           /*取结束时的位置信息            */
    }
    
    CurrentTimeCount = RTC_GetCounter();
    if(ERROR == Register_Write2(REGISTER_TYPE_DOUBT,nFmBuff,246,CurrentTimeCount))
    {
			Register_EraseOneArea(REGISTER_TYPE_DOUBT);
    }
}
/*******************************************************************************
* Function Name  : DoubtNoChangeSave
* Description    : 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DoubtNoChangeSave(void)
{
    u16 nFmStep;
    u8  nFmBuff[470];
    u8  nTab[310];
    u8  nBuff[310];     /*30s速度和状态信号，共150组                  */
    u32	 CurrentTimeCount;
    u16 i=0,j=0;  
    
    memset(nBuff,0,sizeof(nBuff));
            
    for(i=0,j=0;i<300;i+=2,j+=3)                               /*从铁电中读取数据              */
    {
        FRAM_BufferRead(nBuff+i,2,FRAM_DOUBT_POINT_ADDR+9+j);            
    }
    
    FRAM_BufferRead(nFmBuff, 1, FRAM_DOUBT_POINT_ADDR);        /*从铁电读取疑点单步            */                    
    FRAM_BufferRead(nFmBuff+2, 6, FRAM_DOUBT_POINT_ADDR+2);    /*从铁电读取疑点时间            */ 
       
    nFmStep = nFmBuff[0]*2;
    
    memset(nTab,0,sizeof(nTab));
    
    for(i=0,j=nFmStep-1;i<300;i+=2,j-=2)                       /*倒序存储30s数据               */
    {
        if(j > 299)
        {
            j = 299;
        }
        nTab[i] = nBuff[j-1];
        nTab[i+1] = nBuff[j];
    } 
    memcpy(nBuff,nFmBuff,9);                                   /*nBuff暂存单步和时间           */
                                                                     
    memset(nFmBuff,0,sizeof(nFmBuff));                         /*清零缓存区                    */
    memcpy(nFmBuff,nBuff+2,6);                                 /*向nFmBuff中写入时间           */
     
    if(EepromPram_ReadPram(E2_CURRENT_LICENSE_ID,nFmBuff+6)==0)/*复制机动车驾驶证号            */
    {                                                            
        memset(nFmBuff+6,0,18);                                /*读取失败填充0表示未知         */
    }
                             
    memcpy(nFmBuff+24,nTab,200);                               /*存储包含停车前10s数据         */             

    Public_GetCurPositionInfoDataBlock(nFmBuff+224);           /*取结束时的位置信息            */
    
    CurrentTimeCount = RTC_GetCounter();
    Register_Write2(REGISTER_TYPE_DOUBT,nFmBuff,246,CurrentTimeCount);                                      
  
}
/*******************************************************************************
* Function Name  : DoubtPoint_FmControl
* Description    : 疑点数据初铁电控制
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DoubtPoint_FmControl()
{
    static u8   nBuff[300];     /*30s速度和状态信号，共150组                  */
    static u16  nBuffCnt;       /*缓存区计数器                                */
    static u16  FmAdrCnt;       /*铁电存储地址子增量                          */
    static u8   nSpeedBuf;      /*暂存一个速度用于滤除尖峰速度                */
    
    static u8   nSaveFlg;       /*记录疑点数据标志                            */
    
    u32	 CurrentTimeCount;
    TIME_T now_time;
    
    u8 nTabCmp[3];              /*对比缓存                                    */
    u8 nTabFlash[470];          /*组包缓存区，用于把数据存储到flash           */
    u8 nTab[310];               /*暂存缓存区,用于暂存20s疑点数据              */    
    u8 temp=0;
    u8 nCh;
    u16  i;
    u16  j;
    

    if(DoubtPoint_GetAccStatus())
    {
        temp = 0;
        temp = Io_ReadExtCarStatusBit(CAR_STATUS_BIT_BRAKE);                    /*制动            */
        temp = (temp<<1) | Io_ReadExtCarStatusBit(CAR_STATUS_BIT_LEFT_LIGHT);   /*左转            */
        temp = (temp<<1) | Io_ReadExtCarStatusBit(CAR_STATUS_BIT_RIGHT_LIGHT);  /*右转            */
        temp = (temp<<1) | Io_ReadExtCarStatusBit(CAR_STATUS_BIT_FAR_LIGHT);    /*远光            */
        temp = (temp<<1) | Io_ReadExtCarStatusBit(CAR_STATUS_BIT_NEAR_LIGHT);   /*近光            */ 
        temp = (temp<<2);                                                       /*D1-D2用户自定义 */
        temp = (temp<<1) | Io_ReadStatusBit(STATUS_BIT_ACC);                    /*D0用户自定义    */
        
        /***内存循环存储30s的速度和状态信号********************************************/       
        
        nTab[0] = DoubtPoint_GetSpeed();      /*行驶速度             */
        nTab[1] = temp;                           /*行驶状态信号         */
        
        if(nTab[0] > 0xfa)
        {
            nTab[0] = nSpeedBuf;
        }
        
        nSpeedBuf = nTab[0];
        //写入单步数
        nCh = FmAdrCnt/3;
        FRAM_BufferWrite(FRAM_DOUBT_POINT_ADDR, &nCh, 1);
        
        //写入速度&状态
        FRAM_BufferWrite(FRAM_DOUBT_POINT_ADDR+9+FmAdrCnt, nTab, 2);
                        
        //读出速度&状态
        FRAM_BufferRead(nTabCmp, 2, FRAM_DOUBT_POINT_ADDR+9+FmAdrCnt);
                        
        //比较读出的与写入的是否一样
        if(strncmp((char*)&nTab,(char*)&nTabCmp,2)!=0) 
        {
            //重写一遍
            FRAM_BufferWrite(FRAM_DOUBT_POINT_ADDR+9+FmAdrCnt, nTab, 2);
        }   
                                 
        FmAdrCnt += 3;                                   /*每0.2s加3              */
        nBuffCnt += 2;                                   /*每0.2s加2              */
        
        if(DoubtPoint_GetSpeed())  /*速度非0              */
        {
            nStopCnt = 0;         
            if(nRunCnt < 50)        /*连续10s速度非0       */
            {
                nRunCnt++;
            }
            else
            {
                nCarSta = 1;
            }
        }
        else                        /*速度为0              */
        {
    /*        if(!nCarSta)             速度没有连续10s非0  
            {
                nBuffCnt = 0;
                FmAdrCnt = 0;
            }*/
            nRunCnt = 0;           
            if(nStopCnt < 50)       /*速度连续10s为0       */
            {
                nStopCnt++;
            }
            else
            {
                if(nCarSta)
                {
                    nSaveFlg = 1;
                }             
                nCarSta = 0;
            }
        }
        
        if(nBuffCnt > 298)                               /*存满后覆盖以前的数据   */
        {
            nBuffCnt = 0;
        }
        if(FmAdrCnt > 447)/*每30s写一次时间*/
        {
            FmAdrCnt = 0;                     
        } 
    }
    if(nSaveFlg)/*把铁电中的数据存储到flash中*/
    {        
        nSaveFlg = 0;
        
        memset(nBuff,0,sizeof(nBuff));
        
        for(i=0,j=0;i<300;i+=2,j+=3)                                 /*去除铁电中校验位              */
        {
            FRAM_BufferRead(nBuff+i,2,FRAM_DOUBT_POINT_ADDR+9+j);           
        }
                                                
        for(i=0,j=nBuffCnt-1;i<300;i+=2,j-=2)                        /*倒序存储30s数据               */
        {
            if(j > 299)
            {
                j = 299;
            }
            nTab[i] = nBuff[j-1];
            nTab[i+1] = nBuff[j];
        }                      
        
        memset(nTabFlash,0,sizeof(nTabFlash));                       /*清零缓存区                    */
        
        CurrentTimeCount = RTC_GetCounter();                         /*获取实时时间                  */
        Gmtime(&now_time,CurrentTimeCount-10);                        /*减去停车时间                  */
        Public_ConvertTimeToBCDEx(now_time,nTabFlash);               /*转换为BCD码时间               */
        
        FRAM_BufferWrite(FRAM_STOPING_TIME_ADDR, nTabFlash, FRAM_STOPING_TIME_LEN);//保存停车时间
        
        if(EepromPram_ReadPram(E2_CURRENT_LICENSE_ID,nTabFlash+6)==0)/*复制机动车驾驶证号            */
        {                                                            
            memset(nTabFlash+6,0,18);                                /*读取失败填充0表示未知         */
        }
        
        memcpy(nTabFlash+24,nTab+100,200);                             /*存储不包含停车前10s数据      */        
          
        if(Io_ReadStatusBit(STATUS_BIT_NAVIGATION))
        {
            Public_GetCurPositionInfoDataBlock(nTabFlash+224);           /*取结束时的位置信息            */
        }
        
        Register_Write2(REGISTER_TYPE_DOUBT,nTabFlash,246,CurrentTimeCount);    
    
    }

}

/*******************************************************************************
* Function Name  : DoubtPoint_PositionNoChange
* Description    : 位置信息不变化，保存前20S数据
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void  DoubtPoint_PositionNoChange(void)
{
    GPS_STRUCT stTmpGps;
    
    static GPS_STRUCT stCmpTmpGps;
    static u8  SameCnt;
    
    u8 GpsOkFlg;
    
    Gps_CopygPosition(&stTmpGps);
    
    GpsOkFlg = Io_ReadStatusBit(STATUS_BIT_NAVIGATION);
    
    if(((GpsOkFlg==1)&&(SameCnt <= 110)&&(nCarSta)&&(stTmpGps.Latitue_D == stCmpTmpGps.Latitue_D)
    &&(stTmpGps.Latitue_F == stCmpTmpGps.Latitue_F))&&(stTmpGps.Latitue_FX == stCmpTmpGps.Latitue_FX))       
    { 
        SameCnt++;
        if(SameCnt > 100)
        {
            SameCnt = 0xaa; 
            DoubtNoChangeSave();/*从铁电读出数据写入FLASH*/
        }
    }
    else if(((!nCarSta)||(stTmpGps.Latitue_D != stCmpTmpGps.Latitue_D)
           ||(stTmpGps.Latitue_F != stCmpTmpGps.Latitue_F))||(stTmpGps.Latitue_FX != stCmpTmpGps.Latitue_FX))
    {   
        SameCnt = 0;
    }
    Gps_CopygPosition(&stCmpTmpGps);
    
}
/*******************************************************************************
* Function Name  : DoubtPoint_TimeTask
* Description    : 疑点数据采集事件处理,每0.2s调用一次.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
FunctionalState DoubtPoint_TimeTask(void)
{
    static u8   nDoubPointInit=0;
    static u32	TimeCount = 0;
    static u8   PowerFlg=0;
    u8 nTimeBuf[10];          /*组包缓存区，用于把数据存储到flash           */
    
    u32	 CurrentTimeCount;
    u8   nTab[30];
    u32  nMainPower=0;
    u32  nBatteryPower=0;
    
    u8   GpsOkFlg;    

		//因为RTC电池没电导致在GPS时间校准之前会出现时间错乱的问题，所以在没有GPS校时之前不存储行驶记录仪数据，这也是没有办法的办法....
		if(!Gps_GetRtcAdjSta())
			return ENABLE;
    nBatteryPower = Adc_BatteryPower_Readl();
    if((nDoubPointInit == 0)||(nDoubPointInit == 1))
    {
        if(nDoubPointInit == 0)
        {      
            nDoubPointInit++;
            return ENABLE;                /*第一次读取电池电压会失败，需读两次*/
        }
        else
        {
            nDoubPointInit = 0xaa;
            if(nBatteryPower < 3000)      /*只有在没接电池的情况下才用铁电读取*/
            {
                DoubtPoint_Init();      
            }
        }
    }
   
    CurrentTimeCount = Timer_Val();
    
    if((CurrentTimeCount - TimeCount) >= (2*SYSTICK_0p1SECOND))/*同步时间*/
    {
        TimeCount = Timer_Val();
 /***记录条件判断 ->每0.2秒向铁电写入时间*******************************************/       
        Public_ConvertNowTimeToBCDEx(nTimeBuf);       /*获取BCD码实时时间      */
        FRAM_BufferWrite(FRAM_DOUBT_POINT_ADDR+2, nTimeBuf, 6);
            
        FRAM_BufferRead(nTimeBuf+6, 6, FRAM_DOUBT_POINT_ADDR+2);
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             /*判断读出与写入是否相等 */
        if(strncmp((char*)&nTimeBuf,(char*)&nTimeBuf+6,6)!=0)      
        {
            FRAM_BufferWrite(FRAM_DOUBT_POINT_ADDR+2, nTimeBuf, 6);
        }           
/***记录条件判断 ->记录位置信息***************************************************/         
        memset(nTab,0,sizeof(nTab));
        
        GpsOkFlg = Io_ReadStatusBit(STATUS_BIT_NAVIGATION);
        
        if(GpsOkFlg)
        {
            Public_GetCurPositionInfoDataBlock(nTab);          /*  取结束时的位置信息  */   
        }
                            
        FRAM_BufferWrite(FRAM_DOUBT_POSITION_ADDR, nTab, 10);            
        FRAM_BufferRead(nTab+10, 10, FRAM_DOUBT_POSITION_ADDR);
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       /*判断读出与写入是否相等 */
        if(strncmp((char*)&nTab,(char*)&nTab+10,10)!=0)      
        {
            FRAM_BufferWrite(FRAM_DOUBT_POSITION_ADDR, nTab, 10);
        } 
       
/***记录条件判断 -> 行驶结束***************************************************/  

        DoubtPoint_FmControl();/*向铁电写入数据        */               
       
/***记录条件判断 -> 位置信息不变化***********************************************/
       DoubtPoint_PositionNoChange();  
       
/***记录条件判断 -> 外部供电断开***********************************************/ 
        nMainPower = Adc_MainPower_Readl();
        if((nMainPower > 1000)&&(nMainPower < 9000)&&(PowerFlg == 0))/*大于1V小于9V进入*/
        {       
            PowerFlg = 1;
            DoubtPowerSave();      /*从铁电读出数据写入FLASH*/
        }
        else if(nMainPower > 10000)
        {
            PowerFlg = 0;
        }
    }
    return ENABLE;  
}

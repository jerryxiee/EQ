/*******************************************************************************
*��Ȩ˵��	:Shenzhen E-EYE Co. Ltd., Copyright 2009- All Rights Reserved.
*�ļ�����	:doubtpoint.c		
*����		:��¼��λ�ɵ����ݲɼ�
*�汾��	:
*������	:       :myh
*����ʱ��	:2013.3
*�޸���	:
*�޸�ʱ��	:
*�޸ļ�Ҫ˵��	:
*��ע		:
*******************************************************************************/
#include "include.h"
#include "doubtpoint.h"


/**********************************�궨��**************************************/ 
#define      Debug_DoubtPoint                    0    /*�¹��ɵ�ģ����Կ���*/


static u8   nCarSta;        /*������ʻ״̬                                */
static u8   nRunCnt;        /*������ʻ�����������ٶȷ�0                   */
static u8   nStopCnt;       /*����ֹͣ�����������ٶ�Ϊ0                   */

/*******************************************************************************
* Function Name  : DoubtPoint_GetSpeed
* Description    : �¹��ɵ�ģ���ȡ�ٶ�
* Input          : None
* Output         : None
* Return         : �����ٶ�
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
* Description    : �¹��ɵ�ģ���ȡACC״̬
* Input          : None
* Output         : None
* Return         : ����ACC״̬
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
* Description    : �ɵ����ݳ�ʼ��,��Ҫ��ɶϵ�Ǯ�ɵ����ݵĴ洢.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DoubtPoint_Init(void)
{
    u16 nFmStep;
    u8  nFmBuff[470];
    u8  nTab[310];
    u8  nBuff[310];     /*30s�ٶȺ�״̬�źţ���150��                  */
    u32	 CurrentTimeCount;
    TIME_T  h_Time;
    
    u16 i=0,j=0;  
    
    memset(nBuff,0,sizeof(nBuff));
            
    for(i=0,j=0;i<300;i+=2,j+=3)                               /*�������ж�ȡ����              */
    {
        FRAM_BufferRead(nBuff+i,2,FRAM_DOUBT_POINT_ADDR+9+j);            
    }
    
    FRAM_BufferRead(nFmBuff, 1, FRAM_DOUBT_POINT_ADDR);        /*�������ȡ�ɵ㵥��            */                    
    FRAM_BufferRead(nFmBuff+2, 6, FRAM_DOUBT_POINT_ADDR+2);    /*�������ȡ�ɵ�ʱ��            */ 
    
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
        
        nFmStep = nFmStep - 12;                                    /*���ݼ�����Ĳ������������ǰ1.2s*/
        
        for(i=0,j=nFmStep-1;i<300;i+=2,j-=2)                       /*����洢30s����               */
        {
            if(j > 299)
            {
                j = 299;
            }
            nTab[i] = nBuff[j-1];
            nTab[i+1] = nBuff[j];
        } 
        memcpy(nBuff,nFmBuff,9);                                   /*nBuff�ݴ浥����ʱ��           */
                         
        memset(nFmBuff,0,sizeof(nFmBuff));                         /*���㻺����                    */
        memcpy(nFmBuff,nBuff+2,6);                                 /*��nFmBuff��д��ʱ��           */
        nFmBuff[5] = nFmBuff[5] - 1;                               /*����ʱ���1S*/
        
        if(EepromPram_ReadPram(E2_CURRENT_LICENSE_ID,nFmBuff+6)==0)/*���ƻ�������ʻ֤��            */
        {                                                            
            memset(nFmBuff+6,0,18);                                /*��ȡʧ�����0��ʾδ֪         */
        }
               
        memcpy(nFmBuff+24,nTab,200);                               /*�洢����ͣ��ǰ10s����         */             
                   
        FRAM_BufferRead(nFmBuff+224, 10, FRAM_DOUBT_POSITION_ADDR);/*ȡ����ʱ��λ����Ϣ            */        
        
        CurrentTimeCount = RTC_GetCounter();
        Register_Write2(REGISTER_TYPE_DOUBT,nFmBuff,246,CurrentTimeCount);
        
    }

    FRAM_BufferWrite(FRAM_DOUBT_POINT_ADDR,0,FRAM_DOUBT_POINT_LEN);
    FRAM_BufferRead(nFmBuff,460,FRAM_DOUBT_POINT_ADDR);
}
/*******************************************************************************
* Function Name  : DoubtPowerSave
* Description    : �豸����Դ�Ͽ���,����ϵ�ǰ20S����
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DoubtPowerSave(void)
{
    u16 nFmStep;
    u8  nFmBuff[470];
    u8  nTab[310];
    u8  nBuff[310];     /*30s�ٶȺ�״̬�źţ���150��                  */
    
    u16 i=0,j=0;  
    u32	 CurrentTimeCount;
    
    memset(nBuff,0,sizeof(nBuff));
            
    for(i=0,j=0;i<300;i+=2,j+=3)                               /*�������ж�ȡ����              */
    {
        FRAM_BufferRead(nBuff+i,2,FRAM_DOUBT_POINT_ADDR+9+j);            
    }
    
    FRAM_BufferRead(nFmBuff, 1, FRAM_DOUBT_POINT_ADDR);        /*�������ȡ�ɵ㵥��            */                    
    FRAM_BufferRead(nFmBuff+2, 6, FRAM_DOUBT_POINT_ADDR+2);    /*�������ȡ�ɵ�ʱ��            */ 
       
    nFmStep = nFmBuff[0]*2;
    
    memset(nTab,0,sizeof(nTab));
    
    nFmStep = nFmStep - 16;                                    /*���ݼ�����Ĳ������������ǰ1.2s*/
    
    for(i=0,j=nFmStep-1;i<300;i+=2,j-=2)                       /*����洢30s����               */
    {
        if(j > 299)
        {
            j = 299;
        }
        nTab[i] = nBuff[j-1];
        nTab[i+1] = nBuff[j];
    } 
    memcpy(nBuff,nFmBuff,9);                                   /*nBuff�ݴ浥����ʱ��           */
                                                                     
    memset(nFmBuff,0,sizeof(nFmBuff));                         /*���㻺����                    */
    memcpy(nFmBuff,nBuff+2,6);                                 /*��nFmBuff��д��ʱ��           */
     
    if(EepromPram_ReadPram(E2_CURRENT_LICENSE_ID,nFmBuff+6)==0)/*���ƻ�������ʻ֤��            */
    {                                                            
        memset(nFmBuff+6,0,18);                                /*��ȡʧ�����0��ʾδ֪         */
    }
                             
    memcpy(nFmBuff+24,nTab,200);                               /*�洢����ͣ��ǰ10s����         */             
    
    if(Io_ReadStatusBit(STATUS_BIT_NAVIGATION))
    {
        Public_GetCurPositionInfoDataBlock(nFmBuff+224);           /*ȡ����ʱ��λ����Ϣ            */
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
    u8  nBuff[310];     /*30s�ٶȺ�״̬�źţ���150��                  */
    u32	 CurrentTimeCount;
    u16 i=0,j=0;  
    
    memset(nBuff,0,sizeof(nBuff));
            
    for(i=0,j=0;i<300;i+=2,j+=3)                               /*�������ж�ȡ����              */
    {
        FRAM_BufferRead(nBuff+i,2,FRAM_DOUBT_POINT_ADDR+9+j);            
    }
    
    FRAM_BufferRead(nFmBuff, 1, FRAM_DOUBT_POINT_ADDR);        /*�������ȡ�ɵ㵥��            */                    
    FRAM_BufferRead(nFmBuff+2, 6, FRAM_DOUBT_POINT_ADDR+2);    /*�������ȡ�ɵ�ʱ��            */ 
       
    nFmStep = nFmBuff[0]*2;
    
    memset(nTab,0,sizeof(nTab));
    
    for(i=0,j=nFmStep-1;i<300;i+=2,j-=2)                       /*����洢30s����               */
    {
        if(j > 299)
        {
            j = 299;
        }
        nTab[i] = nBuff[j-1];
        nTab[i+1] = nBuff[j];
    } 
    memcpy(nBuff,nFmBuff,9);                                   /*nBuff�ݴ浥����ʱ��           */
                                                                     
    memset(nFmBuff,0,sizeof(nFmBuff));                         /*���㻺����                    */
    memcpy(nFmBuff,nBuff+2,6);                                 /*��nFmBuff��д��ʱ��           */
     
    if(EepromPram_ReadPram(E2_CURRENT_LICENSE_ID,nFmBuff+6)==0)/*���ƻ�������ʻ֤��            */
    {                                                            
        memset(nFmBuff+6,0,18);                                /*��ȡʧ�����0��ʾδ֪         */
    }
                             
    memcpy(nFmBuff+24,nTab,200);                               /*�洢����ͣ��ǰ10s����         */             

    Public_GetCurPositionInfoDataBlock(nFmBuff+224);           /*ȡ����ʱ��λ����Ϣ            */
    
    CurrentTimeCount = RTC_GetCounter();
    Register_Write2(REGISTER_TYPE_DOUBT,nFmBuff,246,CurrentTimeCount);                                      
  
}
/*******************************************************************************
* Function Name  : DoubtPoint_FmControl
* Description    : �ɵ����ݳ��������
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DoubtPoint_FmControl()
{
    static u8   nBuff[300];     /*30s�ٶȺ�״̬�źţ���150��                  */
    static u16  nBuffCnt;       /*������������                                */
    static u16  FmAdrCnt;       /*����洢��ַ������                          */
    static u8   nSpeedBuf;      /*�ݴ�һ���ٶ������˳�����ٶ�                */
    
    static u8   nSaveFlg;       /*��¼�ɵ����ݱ�־                            */
    
    u32	 CurrentTimeCount;
    TIME_T now_time;
    
    u8 nTabCmp[3];              /*�ԱȻ���                                    */
    u8 nTabFlash[470];          /*��������������ڰ����ݴ洢��flash           */
    u8 nTab[310];               /*�ݴ滺����,�����ݴ�20s�ɵ�����              */    
    u8 temp=0;
    u8 nCh;
    u16  i;
    u16  j;
    

    if(DoubtPoint_GetAccStatus())
    {
        temp = 0;
        temp = Io_ReadExtCarStatusBit(CAR_STATUS_BIT_BRAKE);                    /*�ƶ�            */
        temp = (temp<<1) | Io_ReadExtCarStatusBit(CAR_STATUS_BIT_LEFT_LIGHT);   /*��ת            */
        temp = (temp<<1) | Io_ReadExtCarStatusBit(CAR_STATUS_BIT_RIGHT_LIGHT);  /*��ת            */
        temp = (temp<<1) | Io_ReadExtCarStatusBit(CAR_STATUS_BIT_FAR_LIGHT);    /*Զ��            */
        temp = (temp<<1) | Io_ReadExtCarStatusBit(CAR_STATUS_BIT_NEAR_LIGHT);   /*����            */ 
        temp = (temp<<2);                                                       /*D1-D2�û��Զ��� */
        temp = (temp<<1) | Io_ReadStatusBit(STATUS_BIT_ACC);                    /*D0�û��Զ���    */
        
        /***�ڴ�ѭ���洢30s���ٶȺ�״̬�ź�********************************************/       
        
        nTab[0] = DoubtPoint_GetSpeed();      /*��ʻ�ٶ�             */
        nTab[1] = temp;                           /*��ʻ״̬�ź�         */
        
        if(nTab[0] > 0xfa)
        {
            nTab[0] = nSpeedBuf;
        }
        
        nSpeedBuf = nTab[0];
        //д�뵥����
        nCh = FmAdrCnt/3;
        FRAM_BufferWrite(FRAM_DOUBT_POINT_ADDR, &nCh, 1);
        
        //д���ٶ�&״̬
        FRAM_BufferWrite(FRAM_DOUBT_POINT_ADDR+9+FmAdrCnt, nTab, 2);
                        
        //�����ٶ�&״̬
        FRAM_BufferRead(nTabCmp, 2, FRAM_DOUBT_POINT_ADDR+9+FmAdrCnt);
                        
        //�Ƚ϶�������д����Ƿ�һ��
        if(strncmp((char*)&nTab,(char*)&nTabCmp,2)!=0) 
        {
            //��дһ��
            FRAM_BufferWrite(FRAM_DOUBT_POINT_ADDR+9+FmAdrCnt, nTab, 2);
        }   
                                 
        FmAdrCnt += 3;                                   /*ÿ0.2s��3              */
        nBuffCnt += 2;                                   /*ÿ0.2s��2              */
        
        if(DoubtPoint_GetSpeed())  /*�ٶȷ�0              */
        {
            nStopCnt = 0;         
            if(nRunCnt < 50)        /*����10s�ٶȷ�0       */
            {
                nRunCnt++;
            }
            else
            {
                nCarSta = 1;
            }
        }
        else                        /*�ٶ�Ϊ0              */
        {
    /*        if(!nCarSta)             �ٶ�û������10s��0  
            {
                nBuffCnt = 0;
                FmAdrCnt = 0;
            }*/
            nRunCnt = 0;           
            if(nStopCnt < 50)       /*�ٶ�����10sΪ0       */
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
        
        if(nBuffCnt > 298)                               /*�����󸲸���ǰ������   */
        {
            nBuffCnt = 0;
        }
        if(FmAdrCnt > 447)/*ÿ30sдһ��ʱ��*/
        {
            FmAdrCnt = 0;                     
        } 
    }
    if(nSaveFlg)/*�������е����ݴ洢��flash��*/
    {        
        nSaveFlg = 0;
        
        memset(nBuff,0,sizeof(nBuff));
        
        for(i=0,j=0;i<300;i+=2,j+=3)                                 /*ȥ��������У��λ              */
        {
            FRAM_BufferRead(nBuff+i,2,FRAM_DOUBT_POINT_ADDR+9+j);           
        }
                                                
        for(i=0,j=nBuffCnt-1;i<300;i+=2,j-=2)                        /*����洢30s����               */
        {
            if(j > 299)
            {
                j = 299;
            }
            nTab[i] = nBuff[j-1];
            nTab[i+1] = nBuff[j];
        }                      
        
        memset(nTabFlash,0,sizeof(nTabFlash));                       /*���㻺����                    */
        
        CurrentTimeCount = RTC_GetCounter();                         /*��ȡʵʱʱ��                  */
        Gmtime(&now_time,CurrentTimeCount-10);                        /*��ȥͣ��ʱ��                  */
        Public_ConvertTimeToBCDEx(now_time,nTabFlash);               /*ת��ΪBCD��ʱ��               */
        
        FRAM_BufferWrite(FRAM_STOPING_TIME_ADDR, nTabFlash, FRAM_STOPING_TIME_LEN);//����ͣ��ʱ��
        
        if(EepromPram_ReadPram(E2_CURRENT_LICENSE_ID,nTabFlash+6)==0)/*���ƻ�������ʻ֤��            */
        {                                                            
            memset(nTabFlash+6,0,18);                                /*��ȡʧ�����0��ʾδ֪         */
        }
        
        memcpy(nTabFlash+24,nTab+100,200);                             /*�洢������ͣ��ǰ10s����      */        
          
        if(Io_ReadStatusBit(STATUS_BIT_NAVIGATION))
        {
            Public_GetCurPositionInfoDataBlock(nTabFlash+224);           /*ȡ����ʱ��λ����Ϣ            */
        }
        
        Register_Write2(REGISTER_TYPE_DOUBT,nTabFlash,246,CurrentTimeCount);    
    
    }

}

/*******************************************************************************
* Function Name  : DoubtPoint_PositionNoChange
* Description    : λ����Ϣ���仯������ǰ20S����
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
            DoubtNoChangeSave();/*�������������д��FLASH*/
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
* Description    : �ɵ����ݲɼ��¼�����,ÿ0.2s����һ��.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
FunctionalState DoubtPoint_TimeTask(void)
{
    static u8   nDoubPointInit=0;
    static u32	TimeCount = 0;
    static u8   PowerFlg=0;
    u8 nTimeBuf[10];          /*��������������ڰ����ݴ洢��flash           */
    
    u32	 CurrentTimeCount;
    u8   nTab[30];
    u32  nMainPower=0;
    u32  nBatteryPower=0;
    
    u8   GpsOkFlg;    

		//��ΪRTC���û�絼����GPSʱ��У׼֮ǰ�����ʱ����ҵ����⣬������û��GPSУʱ֮ǰ���洢��ʻ��¼�����ݣ���Ҳ��û�а취�İ취....
		if(!Gps_GetRtcAdjSta())
			return ENABLE;
    nBatteryPower = Adc_BatteryPower_Readl();
    if((nDoubPointInit == 0)||(nDoubPointInit == 1))
    {
        if(nDoubPointInit == 0)
        {      
            nDoubPointInit++;
            return ENABLE;                /*��һ�ζ�ȡ��ص�ѹ��ʧ�ܣ��������*/
        }
        else
        {
            nDoubPointInit = 0xaa;
            if(nBatteryPower < 3000)      /*ֻ����û�ӵ�ص�����²��������ȡ*/
            {
                DoubtPoint_Init();      
            }
        }
    }
   
    CurrentTimeCount = Timer_Val();
    
    if((CurrentTimeCount - TimeCount) >= (2*SYSTICK_0p1SECOND))/*ͬ��ʱ��*/
    {
        TimeCount = Timer_Val();
 /***��¼�����ж� ->ÿ0.2��������д��ʱ��*******************************************/       
        Public_ConvertNowTimeToBCDEx(nTimeBuf);       /*��ȡBCD��ʵʱʱ��      */
        FRAM_BufferWrite(FRAM_DOUBT_POINT_ADDR+2, nTimeBuf, 6);
            
        FRAM_BufferRead(nTimeBuf+6, 6, FRAM_DOUBT_POINT_ADDR+2);
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             /*�ж϶�����д���Ƿ���� */
        if(strncmp((char*)&nTimeBuf,(char*)&nTimeBuf+6,6)!=0)      
        {
            FRAM_BufferWrite(FRAM_DOUBT_POINT_ADDR+2, nTimeBuf, 6);
        }           
/***��¼�����ж� ->��¼λ����Ϣ***************************************************/         
        memset(nTab,0,sizeof(nTab));
        
        GpsOkFlg = Io_ReadStatusBit(STATUS_BIT_NAVIGATION);
        
        if(GpsOkFlg)
        {
            Public_GetCurPositionInfoDataBlock(nTab);          /*  ȡ����ʱ��λ����Ϣ  */   
        }
                            
        FRAM_BufferWrite(FRAM_DOUBT_POSITION_ADDR, nTab, 10);            
        FRAM_BufferRead(nTab+10, 10, FRAM_DOUBT_POSITION_ADDR);
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       /*�ж϶�����д���Ƿ���� */
        if(strncmp((char*)&nTab,(char*)&nTab+10,10)!=0)      
        {
            FRAM_BufferWrite(FRAM_DOUBT_POSITION_ADDR, nTab, 10);
        } 
       
/***��¼�����ж� -> ��ʻ����***************************************************/  

        DoubtPoint_FmControl();/*������д������        */               
       
/***��¼�����ж� -> λ����Ϣ���仯***********************************************/
       DoubtPoint_PositionNoChange();  
       
/***��¼�����ж� -> �ⲿ����Ͽ�***********************************************/ 
        nMainPower = Adc_MainPower_Readl();
        if((nMainPower > 1000)&&(nMainPower < 9000)&&(PowerFlg == 0))/*����1VС��9V����*/
        {       
            PowerFlg = 1;
            DoubtPowerSave();      /*�������������д��FLASH*/
        }
        else if(nMainPower > 10000)
        {
            PowerFlg = 0;
        }
    }
    return ENABLE;  
}

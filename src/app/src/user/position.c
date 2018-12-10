/*******************************************************************************
*��Ȩ˵��	:Shenzhen E-EYE Co. Ltd., Copyright 2009- All Rights Reserved.
*�ļ�����	:position.c		
*����		:��¼��λ����Ϣ�ɼ�
*�汾��	:
*������	:       :myh
*����ʱ��	:2013.3 
*�޸���	:
*�޸�ʱ��	:
*�޸ļ�Ҫ˵��	:
*��ע		:
*******************************************************************************/
#include "include.h"
#include "position.h"

extern u8 SpeedFlag;//dxl,2015.5.11,0x00:�ֶ�����,0x01:�ֶ�GPS,0x02:�Զ�����,0x03:�Զ�GPS

/*******************************************************************************
* Function Name  : Position_TimeTask
* Description    : λ����Ϣ�ɼ��¼�����,ÿ1min����һ�� .
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
FunctionalState Position_TimeTask(void)
{    
    u8 nTab[19];/*10��λ����Ϣ + 1��ƽ���ٶ� + 8����λ*/
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
            CheckFlg = 1;//����
        }
        return ENABLE;
    }
    //��ΪRTC���û�絼����GPSʱ��У׼֮ǰ�����ʱ����ҵ����⣬������û��GPSУʱ֮ǰ���洢��ʻ��¼�����ݣ���Ҳ��û�а취�İ취....
		if(!Gps_GetRtcAdjSta())
			return ENABLE;
    CurrentTimeCount = Timer_Val();
    
    if((CurrentTimeCount - TimeCount) >= SYSTICK_1MINUTE)
    {
        TimeCount = Timer_Val();
        
        memset(nTab,0,sizeof(nTab));                       /*��ջ�����           */
        
        if(SpeedMonitor_GetCurRunStatus())
        {
            /*...��λ����λ����Ϣȡ�÷��ӷ�Χ���׸���Ч��λ����Ϣ����÷��ӷ�Χ������Ч��λ����Ϣ����÷���λ����ϢΪ 7FFFFFFFH...*/
            if(Io_ReadStatusBit(STATUS_BIT_NAVIGATION))
            {
                Public_GetCurPositionInfoDataBlock(nTab);          /*���Ƽ�¼��λ������   */
            }
            else
            {
                memset(nTab,0xff,8);  
                nTab[0] = 0x7F;
                nTab[4] = 0x7F;          
            }
            if(0 == (SpeedFlag&0x01))//�����ٶ�
            {
                nSpeed = Pulse_GetMinuteSpeed();
            }
            else//GPS�ٶ�
            {
                nSpeed = MileMeter_GetMinuteSpeed();                 /*1����ƽ���ٶ�        */
            }
            
            CurrentTimeCnt = RTC_GetCounter();
                       
            nTab[10] = nSpeed;
            if(ERROR == Register_Write2(REGISTER_TYPE_POSITION,nTab,19,CurrentTimeCnt))/*�洢��flash*/                  
            {
							Register_EraseOneArea(REGISTER_TYPE_POSITION);
            }
            ////////////////////////////////////////////////////////////////////     
            memset(nTab,0,sizeof(nTab));//�洢����ǰ������� 
            
            FRAM_BufferRead(&nStep, 1, FRAM_STOP_15MINUTE_SPEED_ADDR);//��ȡ����
            
            if(nStep > 14)//����Ƿ����
            {
                nStep = 0;
            }           
            
            Public_ConvertNowTimeToBCDEx(nTab);//�洢BCD��ʱ�� 0-5
            nTab[5] = nSpeed;//������,��¼�ٶ�            

            FRAM_BufferWrite(FRAM_STOP_15MINUTE_SPEED_ADDR+2+(nStep*4), nTab+3, 3);//��Сʱ��¼���ٶȸ�����
            nStep++;//���ӵ���
            FRAM_BufferWrite(FRAM_STOP_15MINUTE_SPEED_ADDR, &nStep, 1);//���浥��
        }
    }
    
    return ENABLE;
}




     
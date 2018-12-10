/******************************************************* 
Copyright (C),E-EYE Tech. Co.,Ltd.
Author: WYF
Version:
Finish Date:2012-6-12
Description:
*******************************************************/
#include "include.h"

//��������
void SetPeriodTimer(u8 Index,u32 CntValue, u32 ItvValue);//�������ڶ�ʱ
void StopTimer(u8 Index);//ֹͣ��ʱ

//��������
extern u8   ttsVolume;                          //����
u8 gErrorRpt[10]={0};   //��������
T_TIMER gTimers[TIMER_SUM]; //��ʱ������
//������
TIMES_TASK MyTTask[MAX_TIMETASK_SUM] = { 
    {0,NULL },   /*OptTTSSend*/     
};


/*******************************************************
Description:��Ӵ����
Input: ��������
Output:
Return: 
*******************************************************/
void AddError(u8 ErrNo)
{   
    static u8 gErrorCnt=0;         //�������,ÿ����һ�δ�������1,С��10        
    gErrorRpt[gErrorCnt] = ErrNo;    
    if(++gErrorCnt < 10)
    {
        ;
    }
    else
    {
        gErrorCnt=9;
    }
}

/************************************************************
��������: SetPeriodTimer()
��������: �������ڶ�ʱ
��ڲ���: Index:��ʱ�����
          Value:��ʱֵ
���ڲ���:
************************************************************/ 
void SetPeriodTimer(u8 Index,u32 CntValue, u32 ItvValue)
{
    gTimers[Index].Counter  = CntValue;
    gTimers[Index].Interval = ItvValue;
    gTimers[Index].OverTimeF=0;
}
/************************************************************
��������: SetOnceTimer()
��������: ���õ��ζ�ʱ
��ڲ���: Index:��ʱ�����
          Value:��ʱֵ
���ڲ���:
************************************************************/ 
void SetOnceTimer(u8 Index,u32 Value)
{
    gTimers[Index].Counter  = Value;
    gTimers[Index].Interval = 0;
    gTimers[Index].OverTimeF=0;
}
/************************************************************
��������: 
��������: ֹͣ��ʱ
��ڲ���: Index:��ʱ�����
          Value:��ʱֵ
���ڲ���:
************************************************************/ 
void StopTimer(u8 Index) 
{
    gTimers[Index].Counter=0;
    gTimers[Index].Interval=0;
    gTimers[Index].OverTimeF=0;
}
/************************************************************
��������: 
��������: ʱ�Ӵ�����
��ڲ���:
���ڲ���:
��ע:  ÿ��Tick(20ms)����һ��
************************************************************/ 
void TimerHandle(void)
{
    u8 i;
    for(i = 0; i < TIMER_SUM; i++)
    {
        if(gTimers[i].Counter)
        {
            gTimers[i].Counter--;
            if(gTimers[i].Counter==0)
            {
                gTimers[i].OverTimeF=1; 
                if(gTimers[i].Interval)
                {
                    gTimers[i].Counter=gTimers[i].Interval; 
                }  
            }
        }
    }
}

/************************************************************
��������: 
��������: �ж��Ƿ�ʱ
��ڲ���: Index:��ʱ�����
���ڲ���:
************************************************************/ 
u8 IsOverTime(u8 Index)
{
    if(gTimers[Index].OverTimeF)
    {
        gTimers[Index].OverTimeF=0;              
        return 1;    
    }
    else
    {
        return 0;
    }
}
/************************************************************
��������: 
��������: ��ó�ʱ״̬
��ڲ���: Index:��ʱ�����
���ڲ���:
************************************************************/ 
u8 GetOverTimeStatus(u8 Index) 
{
    if(gTimers[Index].OverTimeF)
    {   
        return 1;    
    }
    else
    {
        return 0;
    }
}

/************************************************************
��������: 
��������: ��ֹ����
��ڲ���: 
���ڲ���:
************************************************************/
void DisTimerTask(u8 TaskID) 
{
    MyTTask[TaskID].TaskTimerState = 0;
}

/************************************************************
��������: 
��������: ��������
��ڲ���: 
���ڲ���:
************************************************************/
void EnTimerTask(u8 TaskID)
{
    MyTTask[TaskID].TaskTimerState = 1;
}
/************************************************************
��������: 
��������: ����
��ڲ���: 
���ڲ���:
************************************************************/
void TimerTaskSch(void) 
{
    u8 i;
    
    for(i = 0; i < MAX_TIMETASK_SUM; i++)
    {
       if( !MyTTask[i].TaskTimerState )
            continue;   
       MyTTask[i].TaskTimerState = MyTTask[i].operate();      
    }
}





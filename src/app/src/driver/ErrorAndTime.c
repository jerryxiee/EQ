/******************************************************* 
Copyright (C),E-EYE Tech. Co.,Ltd.
Author: WYF
Version:
Finish Date:2012-6-12
Description:
*******************************************************/
#include "include.h"

//函数声明
void SetPeriodTimer(u8 Index,u32 CntValue, u32 ItvValue);//设置周期定时
void StopTimer(u8 Index);//停止定时

//变量定义
extern u8   ttsVolume;                          //音量
u8 gErrorRpt[10]={0};   //保存错误号
T_TIMER gTimers[TIMER_SUM]; //定时器数组
//任务定义
TIMES_TASK MyTTask[MAX_TIMETASK_SUM] = { 
    {0,NULL },   /*OptTTSSend*/     
};


/*******************************************************
Description:添加错误号
Input: 错误类型
Output:
Return: 
*******************************************************/
void AddError(u8 ErrNo)
{   
    static u8 gErrorCnt=0;         //错误计数,每发生一次错误增加1,小于10        
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
函数名称: SetPeriodTimer()
功能描述: 设置周期定时
入口参数: Index:定时器编号
          Value:定时值
出口参数:
************************************************************/ 
void SetPeriodTimer(u8 Index,u32 CntValue, u32 ItvValue)
{
    gTimers[Index].Counter  = CntValue;
    gTimers[Index].Interval = ItvValue;
    gTimers[Index].OverTimeF=0;
}
/************************************************************
函数名称: SetOnceTimer()
功能描述: 设置单次定时
入口参数: Index:定时器编号
          Value:定时值
出口参数:
************************************************************/ 
void SetOnceTimer(u8 Index,u32 Value)
{
    gTimers[Index].Counter  = Value;
    gTimers[Index].Interval = 0;
    gTimers[Index].OverTimeF=0;
}
/************************************************************
函数名称: 
功能描述: 停止定时
入口参数: Index:定时器编号
          Value:定时值
出口参数:
************************************************************/ 
void StopTimer(u8 Index) 
{
    gTimers[Index].Counter=0;
    gTimers[Index].Interval=0;
    gTimers[Index].OverTimeF=0;
}
/************************************************************
函数名称: 
功能描述: 时钟处理函数
入口参数:
出口参数:
备注:  每次Tick(20ms)调用一次
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
函数名称: 
功能描述: 判断是否超时
入口参数: Index:定时器编号
出口参数:
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
函数名称: 
功能描述: 获得超时状态
入口参数: Index:定时器编号
出口参数:
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
函数名称: 
功能描述: 禁止任务
入口参数: 
出口参数:
************************************************************/
void DisTimerTask(u8 TaskID) 
{
    MyTTask[TaskID].TaskTimerState = 0;
}

/************************************************************
函数名称: 
功能描述: 启动任务
入口参数: 
出口参数:
************************************************************/
void EnTimerTask(u8 TaskID)
{
    MyTTask[TaskID].TaskTimerState = 1;
}
/************************************************************
函数名称: 
功能描述: 调度
入口参数: 
出口参数:
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





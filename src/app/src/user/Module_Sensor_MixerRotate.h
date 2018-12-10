#ifndef __MODULE_SENSOR_MIXERROTATE_H_
#define __MODULE_SENSOR_MIXERROTATE_H_


#ifdef ZHENGFANZHUAN

//*************文件包含***************
#include "stm32f2xx.h"



/***************   宏 定义区   ********************************/



//正/反转状态
#define ROTATE_STATE_STOP       0 //00:停止
#define ROTATE_STATE_REVERSE    1 //01:反转
#define ROTATE_STATE_POSITIVE   2 //10:正转
#define ROTATE_STATE_UNKNOWNED  3 //11:未知



//一个滚筒上安装的磁缸数量
#define NUM_MAGNETIC_CYLINDER  2  //默认为 2


//当终端N 秒未接收到中断触发时，即认为滚筒停止
#define NUM_SENCOND_NOEXTI 60 //默认为60秒
/***************   结构体 定义区   ********************************/


typedef struct 
{
	u32  oldTime;    //最后更新的时间(单位:15.625ms)
}TIME_SYSTEM;



typedef struct
{
  
    u8  RevState ;    //正反转方向
    u8  RunState:1 ; //运行状态 0:工作  1：休眠
    u8  RevErrorFlag:1 ; //正反转传感器状态
    u8  Reserver:4 ; //正反转传感器状态  
    
    u8  bMixterStateChange ;
    u8  Mixer_Rotate_Polarity_last;
    u8  Mixer_Rotate_Polarity_current;
    
    
    u8  Rev;        //搅拌车转速 
    u8  PulseCnt;     //脉冲计数
    u8  PulseStart; //脉冲是始计数
    u8  updata; //是否更新
    
    u8  NumMagneticCylinder;

    


    u32   RevSampleTime;          //转速计时
    u32   CarRunStopCounter;
   
    u32     MixerFailTotal;
    u32     CarRunTimeTotal;
    u32     CarWorkTimeTotal;
    u32     OneLoopTime;                //搅拌桶转一圈花费的时间，单位15.625ms   
    
}MIXER_CAR_OBJECT;



/***************   外部全局变量声明区   ********************************/


/***************   外部全局函数声明区   ********************************/



/*******************************************************************************
*  函数名称  : u8 MixerRotate_Get_RotateState()
*  函数功能  : 获得搅拌车滚筒的状态: 00：停止 01:反转 10:正转 11：未知
*******************************************************************************/
extern u8 MixerRotate_Get_RotateState(void);


/*******************************************************************************
*  函数名称  : u8 MixerRotate_Get_RotateSpeed()
*  函数功能  : 获得搅拌车滚筒的转速
*******************************************************************************/
extern u8 MixerRotate_Get_RotateSpeed(void);


/*******************************************************************************
*  函数名称  : MixerRotate_Task_Init
*  函数功能  : 搅拌车滚筒任务初始化
*******************************************************************************/
extern void MixerRotate_Task_Init(void);


/*******************************************************************************
*  函数名称  : MixerRotate_TimeTask()
*  函数功能  : 搅拌车滚筒定时任务
*******************************************************************************/
extern FunctionalState MixerRotate_TimeTask(void);


/*******************************************************************************
**  函数名称  :	MixerRotate_Exti_Isr
**  函数功能  : 搅拌车正反转传感器 脉冲 检测外部中断服务程序
*******************************************************************************/
extern void MixerRotate_Exti_Isr(void);





#endif


#endif

/*******************************************************************************
 * File Name:           LzmSys.h
 * Function Describe:   application entry
 * Relate Module:
 * Writer:              joneming
 * Date:                2012-01-12
 *******************************************************************************/
#ifndef _LZMSYS_H_
#define _LZMSYS_H_
//类型定义
/****************************************************/
/*                                                  */
/* Included files                                   */
/*                                                  */
/****************************************************/
#include "stm32f2xx.h"
/****************************************************/
/*                                                  */
/* Definitions                                      */
/*                                                  */
/****************************************************/
#define _TASK(task) ((void (*)(void))(task)) 
//////////////消息操作宏//////////////////////
#define TaskMessage (LZM_GetSutMessage()->message)
#define TaskPara (LZM_GetSutMessage()->para)
#define pMessage LZM_GetSutMessage()
//类型定义													
///////////////定时器操作宏////////////////////////////
#define MENU_SECOND      SECOND//100ms                           
#define SECS(x)         (LZM_TIMER32)((x) * MENU_SECOND) 

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////													
													
/****************************************************/
/*                                                  */
/* Structures                                       */
/*                                                  */
/****************************************************/
//消息类型(消息数据由消息队列得到,消息数据类型长度应不小于队列数据类型长度)
typedef struct tagLZM_MESSAGE {
    unsigned short message;
    unsigned short para;
    unsigned short para1;
    unsigned short para2;
} LZM_MESSAGE, *PLZM_MESSAGE;

//时钟结构

//系统中的消息
enum TASK_MESSAGE
{    
    ///////////////////timer////////////////////////////
    SHOW_TIMER,							//1 timer
    FLASH_TIMER,						//2 timer
    OPERATE_TIMER,						//3 timer
    ///////以上系统也占用,请慎用//////////////////////////////////////
    REFRESH_TIMER,						//4 timer
    TEST_TIMER,							//5 timer
    MAX_TIMERS,
    ///////////////////message////////////////////////////
    //空消息
    EMPTY_MESSAGE,	                                    //在 LZM_GetMessage()中入当前任务消息
    ///中断消息    
    INT_RTC,		                                    //在 InterruptIrqRtc()中入中断队列
    //按键消息
    KEY_PRESS,		                                    //在 LZM_DefTaskProc()中入中断队列    
    //任务初始化消息
    TASK_START,		                                    //在 LZM_StartNewTask()中入系统队列
    //串口消息    
    //SYS_RECV0,		                                //在 LZM_GetMessage()中入当前任务消息(数据在InterruptIrqUART0中入串口接收数据队列)    
    MESSAGE_MAX		
};


/****************************************************/
/*                                                  */
/* Function declarations                            */
/*                                                  */
/****************************************************/
////////////////当前任务函数指针//////////////
extern LZM_RET (*LZM_TaskFunction)(LZM_MESSAGE *Msg);
////////////////前一任务函数指针//////////////
extern LZM_RET (*LZM_PreTaskFunction)(LZM_MESSAGE *Msg);

///////////////消息处理函数///////////////////
LZM_RET LZM_TaskEmpty(LZM_MESSAGE *Msg);
LZM_RET LZM_DefTaskProc(LZM_MESSAGE *Msg);
void LZM_TaskMenuStart(void);
LZM_MESSAGE *LZM_GetSutMessage(void);
void Insert_KEYQueue(unsigned char key);
void Insert_STARTQueue(unsigned char param);
void LZM_KeyHandler(void);
void LZM_TimerHandler(void);
//开始新任务(更新当前任务编号)	
void LZM_StartNewTask(LZM_RET (*function)(LZM_MESSAGE *Msg));
void LZM_RefuseNowTask(LZM_RET (*function)(LZM_MESSAGE *Msg));
void LZM_RefuseNowTaskEx(LZM_RET (*function)(LZM_MESSAGE *Msg));
void LZM_StartNewTaskEx(LZM_RET (*function)(LZM_MESSAGE *Msg));
//只刷新当前任务//
void LZM_FreshNowTask(void);

/////////时间任务//////////////////////
FunctionalState TaskMenuMessage_TimeTask(void);
////////////////////////////////
void LZM_Nothing(void);
void LZM_ReadyNewTask(void);
void LZM_ReturnPreTask(LZM_RET (*Task)(LZM_MESSAGE *Msg));
void LZM_ReturnOldTaskEx(void);
/////////////////////////////////////////////////////////////
void LZM_SetTimer(LZM_TIMER32 index,LZM_TIMER32 t);
void LZM_SetAlarm(LZM_TIMER32 index,LZM_TIMER32 t);
void LZM_KillTimer(LZM_TIMER32 index);
void LZM_SetOperateTimer(void (*function)(void),LZM_TIMER32 time);
LZM_TIMER32 LZM_TimerGetEnabled(LZM_TIMER32 index);

/////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#endif//_LZMSYS_H_
/*******************************************************************************
 *                            End of Module
 *******************************************************************************/

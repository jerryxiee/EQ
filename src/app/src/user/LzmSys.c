/*******************************************************************************
 * File Name:           LzmSys.c
 * Function Describe:   application entry
 * Relate Module:
 * Writer:              joneming 
 * Date:                2012-01-12
 *******************************************************************************/

/****************************************Copyright (c)**************************************************
**                               Guangzou ZLG-MCU Development Co.,LTD.
**                                      graduate school
**                                 http://www.zlgmcu.com
**
**--------------File Info-------------------------------------------------------------------------------
** File name:			main.c
** Last modified Date:  2004-09-16
** Last Version:		1.0
** Descriptions:		The main() function example template
**
**------------------------------------------------------------------------------------------------------
** Created by:			Chenmingji
** Created date:		2004-09-16
** Version:				1.0
** Descriptions:		The original version
**
**------------------------------------------------------------------------------------------------------
** Modified by:         joneming
** Modified date:		2009-01-17
** Version:			    
** Descriptions:		新的架构(单任务系统,该系统不能支持多任务并发操作，宏观串行地执行一个任务)
**						（3）跳转至用户主程序main执行，在main中完成： 
**							a.初试化各硬件设备；  
**							b.初始化各软件模块； 
**							c.进入死循环（无限循环），调用各模块的处理函数
********************************************************************************************************/
/****************************************************/
/*                                                  */
/* Included files                                   */
/*                                                  */
/****************************************************/ 
#include "include.h"
/****************************************************/
/*                                                  */
/* Local Definitions and macros                     */
/*                                                  */
/****************************************************/ 

/****************************************************/
/*                                                  */
/* Global variables                                 */
/*                                                  */
/****************************************************/
//当前执行任务函数指针
LZM_RET (*LZM_TaskFunction)(LZM_MESSAGE *Msg);
//前一执行任务函数指针	
LZM_RET (*LZM_PreTaskFunction)(LZM_MESSAGE *Msg);

void (*LZM_OperateFunction)(void);                          //
/****************************************************/
/*                                                  */
/* Local Variables                                  */
/*                                                  */
/****************************************************/
//消息结构
static LZM_MESSAGE s_sutMessage;	
//时钟列表
static LZM_TIMER  s_stLzmTimers[MAX_TIMERS];


/****************************************************/
/*                                                  */
/* Forward Declaration of local functions           */
/*                                                  */
/****************************************************/

/*****************************************************
*
*       Private  code
*
*****************************************************/

/*************************************************************OK
** 函数名称: LZM_DevInitHook()
** 功能描述: 端口及外部设备的初始化
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
static void LZM_DevInitHook(void)
{
    LZM_QueueInit();                                            //队列初始化
}

/************************************************************
** 函数名称: LZM_TaskInitHook()
** 功能描述: 队列初始化,任务函数表初始化,开始任务
** 入口参数:
** 出口参数:
************************************************************/ 
static void LZM_TaskInitHook(void)
{
	LZM_TaskFunction = LZM_TaskEmpty;                    //无当前任务
	LZM_PreTaskFunction = LZM_TaskEmpty;                 //无前一任务

    ////////////////////
    PublicSysParamterInit();
    /////////////////////////
    LZM_StartNewTask(TaskShowLogo);                    //开始任务	
}

/************************************************************
** 函数名称: LZM_GetMessage()
** 功能描述: 从消息队列获取消息
** 入口参数: 消息结构指针
** 出口参数:
************************************************************/ 
LZM_RET LZM_GetMessage(LZM_MESSAGE *Msg)
{
    if(!Empty_INTQueue())                                  //中断消息
    {
        Msg->message = Delete_INTQueue();
        return 1;
    }
    else
    if(!Empty_SYSQueue())                                  //系统消息
    {
        Msg->message = Delete_SYSQueue();
        if(Msg->message == KEY_PRESS)                      //按键消息
        {
            Msg->para = Delete_SYSQueue();
        }
        else 
        if(Msg->message == TASK_START)
        {
            Msg->para  = Delete_SYSQueue();
        }
        return 1;
    }	
    else
    Msg->message = EMPTY_MESSAGE;                       //空消息
    return 0;
}

/************************************************************
** 函数名称: LZM_SetTimer
** 功能描述: 重复定时
** 入口参数: index:定时器编号
             value:定时值
** 出口参数:
************************************************************/ 
void LZM_SetTimer(LZM_TIMER32 index,LZM_TIMER32 value)
{
    s_stLzmTimers[index].enabled  = TIMER_ENABLE;
    s_stLzmTimers[index].counter  = value+Timer_Val();
    s_stLzmTimers[index].interval = value;
    s_stLzmTimers[index].operate = NULL;
}
//	 
/************************************************************
** 函数名称: LZM_SetAlarm
** 功能描述: 单次定时
** 入口参数: index:定时器编号
             value:定时值
** 出口参数:
************************************************************/ 
void LZM_SetAlarm(LZM_TIMER32 index,LZM_TIMER32 value)
{
    s_stLzmTimers[index].enabled  = TIMER_ENABLE;
    s_stLzmTimers[index].counter  = value+Timer_Val();
    s_stLzmTimers[index].interval = ONCE;
    s_stLzmTimers[index].operate = NULL;
}
/************************************************************
** 函数名称: LZM_KillTimer()
** 功能描述: 关闭指定定时器
** 入口参数: index:定时器编号
** 出口参数:
************************************************************/ 
void LZM_KillTimer(LZM_TIMER32 index)
{
    s_stLzmTimers[index].enabled  = TIMER_DISENABLE;
}

/************************************************************
** 函数名称: LZM_TimerGetEnabled()
** 功能描述: 
** 入口参数: index:定时器编号
** 出口参数:
************************************************************/ 
LZM_TIMER32 LZM_TimerGetEnabled(LZM_TIMER32 index)
{
    return s_stLzmTimers[index].enabled;
}
/************************************************************
** 函数名称: LZM_TimerHandler()
** 功能描述: 应用程序时钟处理函数
** 入口参数:
** 出口参数:
************************************************************/ 
void LZM_TimerHandler(void)
{
    unsigned char i;
    unsigned long timerVal;
    timerVal = Timer_Val();
    for(i = 0; i < MAX_TIMERS; i++)
    {
        if(s_stLzmTimers[i].enabled == TIMER_ENABLE)
        {
            if(s_stLzmTimers[i].counter <= timerVal)
            {
                if(s_stLzmTimers[i].interval == ONCE)
                {
                    s_stLzmTimers[i].enabled = TIMER_DISENABLE;
                }
                else
                {
                    s_stLzmTimers[i].counter += s_stLzmTimers[i].interval;
                    if(s_stLzmTimers[i].counter < timerVal)
                        s_stLzmTimers[i].counter = timerVal;
                }
                Insert_INTQueue(i);
            }
        }
    }
}
/*****************************************************
*
*       Public  code
*****************************************************/
/************************************************************
** 函数名称: GetSutMessage()
** 功能描述: 获取消息结构
** 入口参数:
** 出口参数:
************************************************************/ 
LZM_MESSAGE *LZM_GetSutMessage(void) 
{
    return &s_sutMessage;
}

/************************************************************
** 函数名称: Insert_STARTQueue()
** 功能描述: 将按键值存入队列
** 入口参数:
** 出口参数:
************************************************************/ 
void Insert_STARTQueue(unsigned char param)
{
    Insert_SYSQueue(TASK_START); 
    Insert_SYSQueue(param);
}

/********************************************************************************
 * Function:KeyFormatValue
 * Describe:change Key's value to ASCII (Key 0 to Key 9)
 *******************************************************************************/
void KeyFormatValue(unsigned char *key)
{
    switch(*key)
    {
        case KEY_OK://确认
        case KEY_EXIT://返回
        case KEY_UP://上翻
        case KEY_DOWN://下翻
            *key = *key;            
            break;
        default:
            *key=0;
            break;
    }
}

/************************************************************
** 函数名称: Insert_KEYQueue()
** 功能描述: 将按键值存入队列
** 入口参数:
** 出口参数:
************************************************************/ 
void Insert_KEYQueue(unsigned char key)
{
    BEEP_ON();
    Insert_SYSQueue(KEY_PRESS); 
    Insert_SYSQueue(key); 
}
/************************************************************
** 函数名称: LZM_RefuseNowTask()
** 功能描述: 只更新当前任务
** 入口参数:param =0
** 出口参数:
************************************************************/ 
void LZM_RefuseNowTask(LZM_RET (*function)(LZM_MESSAGE *Msg))
{
    LZM_ReadyNewTask();    
    LZM_TaskFunction = function;
    Insert_STARTQueue(0);
}
/************************************************************
** 函数名称: LZM_RefuseNowTaskEx()
** 功能描述: 只更新当前任务
** 入口参数: param=0xff
** 出口参数:
************************************************************/ 
void LZM_RefuseNowTaskEx(LZM_RET (*function)(LZM_MESSAGE *Msg))
{
    LZM_ReadyNewTask();    
    LZM_TaskFunction = function;
    Insert_STARTQueue(0xff);
} 
/************************************************************
** 函数名称: LZM_StartNewTask()
** 功能描述: 启动新任务
** 入口参数:
** 出口参数:
************************************************************/ 
void LZM_StartNewTask(LZM_RET (*function)(LZM_MESSAGE *Msg))
{
    if(LZM_TaskFunction != function)
    LZM_PreTaskFunction = LZM_TaskFunction;
    LZM_RefuseNowTask(function);
}

/************************************************************
** 函数名称: LZM_FreshNowTask()
** 功能描述: 只刷新当前任务
** 入口参数:
** 出口参数:
************************************************************/ 
void LZM_FreshNowTask(void)
{
    LZM_ReadyNewTask();    
    Insert_STARTQueue(0);
}
/************************************************************
** 函数名称: LZM_StartNewTaskEx()
** 功能描述: param为0xff
** 入口参数:
** 出口参数:
************************************************************/ 
void LZM_StartNewTaskEx(LZM_RET (*function)(LZM_MESSAGE *Msg))
{
    LZM_RefuseNowTaskEx(function);
}

/************************************************************
** 函数名称: LZM_DispatchMessage()
** 功能描述: 当前任务处理消息
             向当前任务编号发送当前消息执行不同的任务函数 
** 入口参数:
** 出口参数:
************************************************************/ 
void LZM_DispatchMessage(LZM_MESSAGE *Msg)
{
    LZM_TaskFunction(Msg);
}  
/************************************************************
** 函数名称: LZM_DispatchMessage()
** 功能描述: 当前任务处理消息
             向当前任务编号发送当前消息执行不同的任务函数 
** 入口参数:
** 出口参数:
************************************************************/ 
void LZM_KeyHandler(void)
{
    static unsigned char s_ucCount;				//key press timer counter const
    static unsigned char s_ucKeyCount;			//key press timer counter
    //static unsigned char s_ucKeyTimes;			//Key times counter
    static unsigned char KeyCode;
    static unsigned char s_ucKeyValue=0;
    static unsigned short s_usTotalCnt=0;			//key press timer counter
    /////////////////////    
    KeyCode=KeyValue_Read();
    KeyFormatValue(&KeyCode);
    /////////    	
    /////////////////////////////
    if(!KeyCode)									//no key press
    {
        s_ucKeyValue = 0;//
        s_usTotalCnt = 0;
    }
    else										//key pressed
    {
        SysAutoRunCountReset();
        if(KeyCode != s_ucKeyValue)					//key counter reset
        {
            s_ucCount       = 10;						//first counter = 2
            s_ucKeyCount    = 0;
            s_usTotalCnt    = 0;        
            s_ucKeyValue    = KeyCode;            
            Insert_KEYQueue(s_ucKeyValue);
        }
        else									//key count
        {
            if(s_usTotalCnt>SYSTICK_10SECOND)//一直按着,超过10s
            {
                BEEP_ON();
                return;//
            }
            s_usTotalCnt++;
            s_ucKeyCount ++;
            if(s_ucKeyCount >= s_ucCount)
            {       
                s_ucCount = 3;				//other counter = 3
                s_ucKeyCount = 0;
                //if(s_ucKeyMode == 0)
                {
                    Insert_KEYQueue(s_ucKeyValue);
                }						//key up
            }
        }
    }
}
/************************************************************
** 函数名称: LZM_SetOperateTimer
** 功能描述: 
** 入口参数:
** 出口参数:
************************************************************/ 
void LZM_SetOperateTimer(void (*function)(void),LZM_TIMER32 time)
{
    LZM_OperateFunction=function;
    LZM_SetAlarm(OPERATE_TIMER, time);
}
/************************************************************
** 函数名称: LZM_DisposeOperateTimer
** 功能描述: 
** 入口参数:
** 出口参数:
************************************************************/ 
void LZM_DisposeOperateTimer(void)
{
    if(LZM_OperateFunction!=NULL)
    {
        LZM_OperateFunction();
        LZM_OperateFunction = NULL;
    }
}
/************************************************************
** 函数名称: LZM_DefTaskProc()
** 功能描述: 默认处理函数，包含时钟扫描、按键处理和串口数据收发
** 入口参数:
** 出口参数:
************************************************************/ 
LZM_RET LZM_DefTaskProc(LZM_MESSAGE *Msg)
{	
    switch(Msg->message)
    { 
        case OPERATE_TIMER:
            LZM_DisposeOperateTimer();
            break;
        case REFRESH_TIMER:
            break;
        case INT_RTC:		    
            break; 
        case SHOW_TIMER:
            Lcd_SetMainRedrawFlag();
            Lcd_ClearDisplayCtrlFlag();
            LcdPasteScreen();
            break;
        case FLASH_TIMER:
            FlashLcdBackLightFinish();
            break;
        case EMPTY_MESSAGE:      
        default:
            break;
    }
    return 0;
}

/*************************************************************
** 函数名称: LZM_TaskEmpty()
** 功能描述: 空任务
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET LZM_TaskEmpty(LZM_MESSAGE *Msg)
{
	switch(Msg->message)
	{
	    case TASK_START:
	        break;
	    case INT_RTC:
	        break;  
	    case KEY_PRESS:
	        break;              
	    default:
            LZM_DefTaskProc(Msg);
	}	
	return 0;
}

/************************************************************************************
*空函数.
*Button初始化时将其响应事件(按下/释放)设置成空操作.
************************************************************************************/
void LZM_Nothing(void)
{
    //Do Nothing
}
/*************************************************************OK
** 函数名称: ReadyNweTask()
** 功能描述: 新任务准备
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: Clear_SYSQueue();ClearLcdCopyFlag();
			 Clear_INTQueue();
*************************************************************/
void LZM_ReadyNewTask(void)
{
    unsigned char i,OpTime,flashTime;
    ClearLcdCopyFlag();
    LZM_KillTimer(SHOW_TIMER); 
    LZM_KillTimer(REFRESH_TIMER);    
    LZM_KillTimer(TEST_TIMER);
    /////////////////////////////
    //清除按键等消息//
    Clear_SYSQueue();
    ////清除定时器消息///
    OpTime =0;
    flashTime =0;
    while(!Empty_INTQueue())//中断消息
    {
        i = Delete_INTQueue();
        if(OPERATE_TIMER==i)
        {
            OpTime=1;
        }
        else
        if(FLASH_TIMER==i)
        {
            flashTime=1;
        }
    }
    //////////////////////
    if(OpTime)//
    {
        Insert_INTQueue(OPERATE_TIMER);
    }
    ////////////////////
    if(flashTime)//
    {
        Insert_INTQueue(FLASH_TIMER);
    }    
}
/*************************************************************OK
** 函数名称: LZM_ReturnPreTask()
** 功能描述: 只更新当前任务,不改变其父任务
** 入口参数: Task:要执行的任务
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
			 
** 调用模块: 
*************************************************************/
void LZM_ReturnPreTask(LZM_RET (*Task)(LZM_MESSAGE *Msg))
{
    LZM_RefuseNowTask(Task);
}
/*************************************************************OK
** 函数名称: LZM_ReturnOldTaskEx()
** 功能描述: 任务返回,返回前一次任务
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: :
			 :
** 调用模块: 
*************************************************************/
void LZM_ReturnOldTaskEx(void)
{
    LZM_RET (*LZM_TempTaskFunction)(LZM_MESSAGE *Msg);
    if(LZM_PreTaskFunction!=LZM_TaskFunction)
    {
        LZM_TempTaskFunction =LZM_PreTaskFunction;
        LZM_PreTaskFunction = LZM_TaskFunction;
    }
    else
    {
        LZM_TempTaskFunction = TaskShowMainInterface;
    }
    LZM_RefuseNowTaskEx(LZM_TempTaskFunction);
}

/************************************************************
** 函数名称: TaskMenuMessage_TimeTask()
** 功能描述: 获取消息，调用各任务模块的消息处理函数,20ms调度一次
** 入口参数:
** 出口参数:
************************************************************/ 
FunctionalState TaskMenuMessage_TimeTask(void)
{
    static unsigned char sucCnt=0;
    /////////////////
    sucCnt++;     
    //////////////////////////////
    LZM_GetMessage(pMessage);
    LZM_DispatchMessage(pMessage);    
    //////////////////////////////
    ///////////////////////
    LZM_KeyHandler();
    /////////////////////
    if(sucCnt%2==0)//100ms调试一次
    {
        ////菜单任务定时器//////////////////////////////
        LZM_TimerHandler();
    }
    ////////////////////////
    if(sucCnt>4)//200ms
    {
        sucCnt = 0;
        SysAutoRunToMainInfoTask();//自动
    }
    return ENABLE;
}
/*************************************************************OK
** 函数名称: LZM_TaskMenuStart()
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void LZM_TaskMenuStart(void)
{
	//初始化各硬件设备
    LZM_DevInitHook();
    //初始化各软件模块
    LZM_TaskInitHook();
    /////////////////////    
    SetTimerTask(TIME_MENU,1);//开启菜单任务
    /////////////////////
    //进入死循环（无限循环），调用各任务模块的消息处理函数
    TaskMenuMessage_TimeTask();
}

/*******************************************************************************
 *                            End of Module
 *******************************************************************************/

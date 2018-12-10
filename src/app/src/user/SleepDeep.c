/*******************************************************************************
 * File Name:			SleepDeep.c 
 * Function Describe:	深度休眠
 * Relate Module:		
 * Writer:				Joneming
 * Date:				2013-07-11
 * ReWriter:			
 * Date:
 ***************************************************************************
 *******************************************************************************/
#include "include.h"
////////////////////////////////////
#define SLEEPDEEP_VOLT_12V_MIN  110 //(0.1V)
#define SLEEPDEEP_VOLT_24V_MIN  220 //(0.1V)
#define SLEEPDEEP_VOLT_12V_MAX  146 //(0.1V)
#define SLEEPDEEP_VOLT_24V_MAX  292 //(0.1V)
#define WAKEUP_VOLT_12V  126 //(0.1V)
#define WAKEUP_VOLT_24V  252 //(0.1V)
///////////////////////////////////////
#define SLEEP_DEEP_MIN_STOP_TIME  240 //秒)
//////////////////////////
#define SLEEP_DEEP_ACC_GPIO_CLK         RCC_AHB1Periph_GPIOE
#define SLEEP_DEEP_ACC_PORT             GPIOE
#define SLEEP_DEEP_ACC_PIN              GPIO_Pin_7
#define SLEEP_DEEP_ACC_EXTI_GPIO        EXTI_PortSourceGPIOE
#define SLEEP_DEEP_ACC_GPIO_PINSOURCE   GPIO_PinSource7//注意，外部中断须与pin脚一致
#define SLEEP_DEEP_ACC_EXTI_LINE        EXTI_Line7//外部中断须与pin脚一致
#define SLEEP_DEEP_ACC_EXTI_IRQn        EXTI9_5_IRQn//外部中断须与pin脚一致
/////////////////////
/////////////////////////
enum 
{
    EXTI_TYPE_EMPTY,//   
    EXTI_TYPE_ACC,  // 
    EXTI_TYPE_RTC,  //
    EXTI_TYPE_OTHER,  // 
    EXTI_TYPE_BREAK,  //
    EXTI_TYPE_STOP,  // 
    EXTI_TYPE_MAX
}E_EXTITYPE;
////////////////////////
enum 
{
    SLEEP_TIMER_TASK,                        //   
    SLEEP_TIMER_TEST,						//
    SLEEP_TIMER_SEND,						//    
    SLEEP_TIMERS_MAX
}STDSLEEPTIME;

static LZM_TIMER s_stDSleepTimer[SLEEP_TIMERS_MAX];

typedef struct
{
    unsigned long StopTime;     //进入停机时间
    unsigned long AccOffTimeCnt;   //Acc关的时间
    unsigned short recoverVolt; //恢复电压值
    unsigned char stopFlag;     //停机标志
    unsigned char clearRestartFlag;//清除重启任务标志
    unsigned char CheckCnt;     //检查计数器
    unsigned char ExtiType;     //中断标志
    unsigned char VoltType;     //电压类型
}STSLEEP_ATTRIB; 
static STSLEEP_ATTRIB s_stSleepDAttrib;
//////////////////////////////////////////////////////
/*******************************************************************************
**  函数名称  : SleepDeep_ClearDeepSleep
**  函数功能  : 
**  输    入  : 无 
**  输    出  : 
 
**  全局变量  : 
**  调用函数  : 无
**  中断资源  : 无
**  备    注  :
*******************************************************************************/
void SleepDeep_ResetSCRSleepDeepBit(void)
{
    // Reset SLEEPDEEP bit of Cortex System Control Register //
    SCB->SCR &= (uint32_t)~((uint32_t)SCB_SCR_SLEEPDEEP_Msk);  
}
/*******************************************************************************
**  函数名称  : SleepDeep_SetAccExtiType
**  函数功能  : 
**  输    入  : 无 
**  输    出  : 
 
**  全局变量  : 
**  调用函数  : 无
**  中断资源  : 无
**  备    注  :
*******************************************************************************/
void SleepDeep_SetAccExtiType(void)
{
    s_stSleepDAttrib.ExtiType = EXTI_TYPE_ACC;
}
/*******************************************************************************
**  函数名称  : SleepDeep_SetAccExtiType
**  函数功能  : 
**  输    入  : 无 
**  输    出  :  
**  全局变量  : 
**  调用函数  : 无
**  中断资源  : 无
**  备    注  :
*******************************************************************************/
void SleepDeep_SetRtcExtiType(void)
{
    s_stSleepDAttrib.ExtiType = EXTI_TYPE_RTC;
}
/*******************************************************************************
**  函数名称  : SleepDeep_SetExtiTypeBreak
**  函数功能  : 
**  输    入  : 无 
**  输    出  :  
**  全局变量  : 
**  调用函数  : 无
**  中断资源  : 无
**  备    注  :
*******************************************************************************/
void SleepDeep_SetExtiTypeBreak(void)
{
    s_stSleepDAttrib.ExtiType = EXTI_TYPE_BREAK;
}
/*******************************************************************************
**  函数名称  : SleepDeep_PortCtrlAccExtiEnable(void)
**  函数功能  : 使能ACC1高电平上电检测中断
**  输    入  : 无 
**  输    出  : 无
 
**  全局变量  : 
**  调用函数  : 无
**  中断资源  : 无
**  备    注  :
*******************************************************************************/  
void SleepDeep_PortCtrlAccExtiEnable(void)
{
    EXTI_InitTypeDef    EXTI_InitStructure;
    // Connect EXTI Line to PE7 pin //
    SYSCFG_EXTILineConfig(SLEEP_DEEP_ACC_EXTI_GPIO, SLEEP_DEEP_ACC_GPIO_PINSOURCE);//设定为外部中断引脚

    EXTI_ClearITPendingBit(SLEEP_DEEP_ACC_EXTI_LINE);                 //清除外部中断标志    
    // Configure EXTI Line //
    EXTI_InitStructure.EXTI_Line = SLEEP_DEEP_ACC_EXTI_LINE;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;//上升沿触发模式
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
}
/*******************************************************************************
**  函数名称  : SleepDeep_PortCtrlAccExtiInit
**  函数功能  : 初始化高电平输入、开盖报警、防拆开关输入等为终端输入脚
**  输    入  : 无 
**  输    出  : 
 
**  全局变量  : 
**  调用函数  : 无
**  中断资源  : 无
**  备    注  :
*******************************************************************************/
void SleepDeep_PortCtrlAccExtiInit(void)
{
    GPIO_InitTypeDef   GPIO_InitStructure;
    NVIC_InitTypeDef   NVIC_InitStructure;

    // Enable GPIOE clock //
    RCC_AHB1PeriphClockCmd(SLEEP_DEEP_ACC_GPIO_CLK, ENABLE);

    // Enable SYSCFG clock //
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    // Configure PE7 pin as input floating //
    GPIO_InitStructure.GPIO_Pin = SLEEP_DEEP_ACC_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    //GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;    
    GPIO_Init(GPIOE, &GPIO_InitStructure);
    
    // Enable SYSCFG clock //
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    
    // Enable and set EXTI7 Interrupt to the lowest priority //
    NVIC_InitStructure.NVIC_IRQChannel = SLEEP_DEEP_ACC_EXTI_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    //////////////////////////
    SleepDeep_PortCtrlAccExtiEnable();
}
/*******************************************************************************
**  函数名称  : SleepDeep_PortCtrlAccExtiDisable
**  函数功能  : 禁止ACC1高电平检测中断发生
**  输    入  : 无 
**  输    出  : 无
 
**  全局变量  : 
**  调用函数  : 无
**  中断资源  : 无
**  备    注  :
*******************************************************************************/  
void SleepDeep_PortCtrlAccExtiDisable(void)
{
    EXTI_InitTypeDef    EXTI_InitStructure;

    SYSCFG_EXTILineConfig(SLEEP_DEEP_ACC_EXTI_GPIO, SLEEP_DEEP_ACC_GPIO_PINSOURCE); //设定Pe3为外部中断引脚
    ///////////////////////////
    EXTI_ClearITPendingBit(SLEEP_DEEP_ACC_EXTI_LINE);                 //清除外部中断标志
    ////////////////
    EXTI_InitStructure.EXTI_Line = SLEEP_DEEP_ACC_EXTI_LINE ;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; //外部中断模式
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;//EXTI_Trigger_Falling;         //上升沿触发模式
    EXTI_InitStructure.EXTI_LineCmd = DISABLE;
    EXTI_Init(&EXTI_InitStructure);  
}
/*******************************************************************************
**  函数名称  : SleepDeep_RTCWakeUpDisable
**  函数功能  : 
**  输    入  : 无 
**  输    出  : 无
 
**  全局变量  : 
**  调用函数  : 无
**  中断资源  : 无
**  备    注  :
*******************************************************************************/  
void SleepDeep_RTCWakeUpDisable(void)
{
    RTC_WakeUpCmd(DISABLE);
    RTC_ClearITPendingBit(RTC_IT_WUT);
    EXTI_ClearITPendingBit(EXTI_Line22);
}
/*******************************************************************************
**  函数名称  : SleepDeep_ConfigRTCWakeUp
**  函数功能  : 
**  输    入  : 无 
**  输    出  : 无
 
**  全局变量  : 
**  调用函数  : 无
**  中断资源  : 无
**  备    注  :
*******************************************************************************/  
void SleepDeep_ConfigRTCWakeUp(void)
{
    NVIC_InitTypeDef  NVIC_InitStructure;
    EXTI_InitTypeDef  EXTI_InitStructure;

    #if 0//不需要再配置
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

    // Allow access to RTC //
    PWR_BackupAccessCmd(ENABLE);
    // Enable the RTC Clock //
    RCC_LSEConfig(RCC_LSE_ON);

    // Wait till LSE is ready //  
    while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
    {
    }

    // Select the RTC Clock Source //
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
    RCC_RTCCLKCmd(ENABLE);

    // Wait for RTC APB registers synchronisation //
    RTC_WaitForSynchro();
    #endif
    ///////////////////////////////////////
    SleepDeep_RTCWakeUpDisable();
    
    RTC_WakeUpClockConfig(RTC_WakeUpClock_CK_SPRE_16bits);
    RTC_SetWakeUpCounter(0x09);

    // Enable the Wakeup Interrupt //
    RTC_ITConfig(RTC_IT_WUT, ENABLE);

    // Enable Wakeup Counter //
    RTC_WakeUpCmd(ENABLE);

    // Configure one bit for preemption priority //
    //NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

    // Enable the RTC Interrupt //
    NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // Exti configuration ////////    
    EXTI_ClearITPendingBit(EXTI_Line22);
    EXTI_InitStructure.EXTI_Line = EXTI_Line22;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    RTC_WaitForSynchro();
    RTC_ClearITPendingBit(RTC_IT_WUT);
    EXTI_ClearITPendingBit(EXTI_Line22);
}
/********************************************************************
* 名称 : SleepDeep_ResetWatchdog
* 功能 : 重新初始化看门狗的预分频寄存器和重载寄存器的值
*
* 输入 : 无
* 输出 : 无
*
* 备注: (修改记录内容、时间)
*        1.重新初始化看门狗的预分频寄存器和重载寄存器的值，使在STOP MODE模式下看门狗的喂狗时间加长。
********************************************************************/
void SleepDeep_ResetWatchdog(void)
{
    #ifdef WATCHDOG_OPEN
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);//允许修改 预分频寄存器 和 重装载寄存器;
    IWDG_SetPrescaler(IWDG_Prescaler_256);;//设置 "预分频系数" 为 256;//IWDG counter clock: 40KHz(LSI) / 256 = 0.15625 KHz
    IWDG_SetReload(4000);//独立看门狗 超时时间 T= 6.4ms *4000 = 25600ms
    IWDG_ReloadCounter();//清看门狗
    IWDG_Enable();//打开 独立看门狗  
    #endif
}
/*******************************************************************************
**  函数名称  : SleepDeep_ResetIoInit
**  函数功能  : 
**  输    入  : 无 
**  输    出  : 无 
**  全局变量  : 
**  调用函数  : 无
**  中断资源  : 无
**  备    注  :
*******************************************************************************/  
void SleepDeep_ResetIoInit(void)
{
    GPIO_InitTypeDef    GPIO_InitStructure;
    
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOC|RCC_AHB1Periph_GPIOD|RCC_AHB1Periph_GPIOE|RCC_AHB1Periph_GPIOF|RCC_AHB1Periph_GPIOG|RCC_AHB1Periph_GPIOH|RCC_AHB1Periph_GPIOI, ENABLE);

    //--------- IO端口初始化 ----------------------------
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN; 
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD; 
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; 
    GPIO_Init(GPIOA, &GPIO_InitStructure); 
    GPIO_Init(GPIOB, &GPIO_InitStructure); 
    GPIO_Init(GPIOC, &GPIO_InitStructure); 
    GPIO_Init(GPIOD, &GPIO_InitStructure); 
    GPIO_Init(GPIOE, &GPIO_InitStructure); 
    GPIO_Init(GPIOF, &GPIO_InitStructure); 
    GPIO_Init(GPIOG, &GPIO_InitStructure); 
    GPIO_Init(GPIOH, &GPIO_InitStructure); 
    GPIO_Init(GPIOI, &GPIO_InitStructure);    
}

/*******************************************************************************
**  函数名称  : SleepDeep_GetAccPortInValue
**  函数功能  : 
**  输    入  : 无 
**  输    出  : 无 
**  全局变量  : 
**  调用函数  : 无
**  中断资源  : 无
**  备    注  :
*******************************************************************************/  
unsigned char SleepDeep_GetAccPortInValue(void)
{
    return GPIO_ReadInputDataBit(SLEEP_DEEP_ACC_PORT,SLEEP_DEEP_ACC_PIN);
}
/*******************************************************************************
**  函数名称  : SleepDeep_CloseAllUserPower
**  函数功能  : 
**  输    入  : 无 
**  输    出  : 无
 
**  全局变量  : 
**  调用函数  : 无
**  中断资源  : 无
**  备    注  :
*******************************************************************************/  
void SleepDeep_CloseAllUserPower(void)
{
    GpioOutOff(IC_P);
    TTSDerv_CtrlTTSPower(0);
    GPS_POWER_OFF();      //关闭 GPS电源
    GSM_PWR_OFF();      //GPRS模块断电.
    CAMERA_POWER_OFF();
    //Comm485PowerOff(POWER_STATUS_ALL);
    SD_POWER_OFF();
    NaviLcd_AccStatusOff();//    
    LCD_LIGHT_OFF();
}
/*******************************************************************************
**  函数名称  : SleepDeep_OpenAllUserPower
**  函数功能  : 
**  输    入  : 无 
**  输    出  : 无
 
**  全局变量  : 
**  调用函数  : 无
**  中断资源  : 无
**  备    注  :
*******************************************************************************/  
void SleepDeep_OpenAllUserPower(void)
{
    GPS_POWER_ON();      //关闭 GPS电源
    GSM_PWR_ON();      //GPRS模块断电.
    CAMERA_POWER_ON();
    //Comm485PowerOn(POWER_STATUS_ALL);
    SD_POWER_ON();
    NaviLcd_AccStatusOn();//
}
/*************************************************************
** 函数名称: SleepDeep_GetCurPowerValue
** 功能描述: 取得当前主电压值
** 入口参数: 
** 出口参数: 
** 返回参数: 当前主电压值(0.1V)
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned short SleepDeep_GetCurPowerValue(void)
{
    unsigned long AdSum = 0;//AD转换值累计和
    unsigned short AdMin = 0xffff;//AD转换值最小值
    unsigned short AdMax = 0;//AD转换值最大值    
    unsigned short Ad;
    unsigned char Count = 0;//AD转换计数
    
    for(Count = 0; Count<10; Count++)
    {
        Ad = Ad_GetValue(ADC_MAIN_POWER); 
        if(AdMin > Ad)
        {
            AdMin = Ad;
        }
        if(AdMax < Ad)
        {
            AdMax = Ad;
        }
        AdSum += Ad;
    }
    if(Count >= 10)
    {
        //计算平均值
        AdSum -= AdMin;
        AdSum -= AdMax;
        AdSum = AdSum >> 3;
        //转成以0.1V为单位的值
        AdSum = AdSum*33*9/0xfff;        
    }
    return AdSum;
}
/*******************************************************************************
**  函数名称  : SleepDeep_DelayMs
**  函数功能  : 
**  输    入  : 无
**  输    出  : 
 
**  全局变量  : 
**  调用函数  : 无
**  中断资源  : 无
**  备    注  :
*******************************************************************************/
void SleepDeep_DelayMs(unsigned char num)
{
    unsigned char i;
    unsigned short j;
    for(i= num; i>0; i--)
        for(j=1000; j>0; j--);
}
/*******************************************************************************
**  函数名称  : SleepDeep_StopMode
**  函数功能  : 判断是否退出STOP MODE
**  输    入  : 无
**  输    出  :  
**  全局变量  : 
**  调用函数  : 无
**  中断资源  : 无
**  备    注  :
*******************************************************************************/
void SleepDeep_StopMode(void)
{
    unsigned char ucAccCnt;
    unsigned char ucCount,ucBackSleepCnt;

    ucAccCnt = 0;           //ACC有效计时 
    ucCount = 0;
    ucBackSleepCnt = 0;     //返回睡眠状态计数
    /////////////////////////////////
    s_stSleepDAttrib.CheckCnt = 0;
    //////////////////////    
    //Request to enter STOP mode with regulator in Low Power //
    PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
    //////////////////////////////
    while(1)
    {
        if(EXTI_TYPE_ACC == s_stSleepDAttrib.ExtiType)
        {
            for(ucCount =0; ucCount< 100 ; ucCount++)
            {
                IWDG_ReloadCounter();   //清 独立看门狗            
                if(SleepDeep_GetAccPortInValue())                  //检测ACC ON有效
                {
                    ucAccCnt ++;           //ACC ON持续有效次数
                    ucBackSleepCnt = 0;     //        
                }
                else
                {
                    ucAccCnt = 0;      
                    ucBackSleepCnt ++;  
                }
                ////////////////
                if(ucBackSleepCnt >= 50)    //条件不满足，返回休眠模式。
                {
                    ucCount = 0;
                    ucAccCnt = 0;              //ACC ON持续有效次数
                    ucBackSleepCnt = 0;                    
                    s_stSleepDAttrib.ExtiType = EXTI_TYPE_OTHER;//
                    break;
                }
                if((ucAccCnt >= 4)&&SleepDeep_GetAccPortInValue())//满足开盖大于30S，或ACC大于2S，退出休眠
                {
                    ucCount = 0;
                    ucAccCnt = 0;               //ACC ON持续有效次数 
                    ucBackSleepCnt = 0;                    
                    SleepDeep_PortCtrlAccExtiDisable();
                    SleepDeep_RTCWakeUpDisable();
                    s_stSleepDAttrib.ExtiType = EXTI_TYPE_BREAK;//退出休眠状态
                    return;                      //外部中断的情况下退出STOP MODE
                }
            }                             
        }
        else
        if(EXTI_TYPE_RTC == s_stSleepDAttrib.ExtiType)//
        {
            IWDG_ReloadCounter();   //清独立看门狗
            s_stSleepDAttrib.ExtiType = EXTI_TYPE_OTHER;// 
            s_stSleepDAttrib.CheckCnt++;
            if(s_stSleepDAttrib.CheckCnt>10)
            {
                s_stSleepDAttrib.CheckCnt = 0;
                SleepDeep_DelayMs(1);
                IWDG_ReloadCounter();   //清独立看门狗
                if(SleepDeep_GetCurPowerValue()>s_stSleepDAttrib.recoverVolt+2)
                {
                    s_stSleepDAttrib.ExtiType = EXTI_TYPE_BREAK;// 
                }
            }                       
        }
        else 
        if(EXTI_TYPE_BREAK == s_stSleepDAttrib.ExtiType)
        {
            return;
        }
        else
        if(EXTI_TYPE_OTHER == s_stSleepDAttrib.ExtiType)//
        {    
            s_stSleepDAttrib.ExtiType = EXTI_TYPE_STOP;
            PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
        }
    } 
}

/*******************************************************************************
**  函数名称  : fUsartIntoSleep
**  函数功能  : 判断是否退出STOP MODE
**  输    入  : 无
**  输    出  :  
**  全局变量  : 
**  调用函数  : 无
**  中断资源  : 无
**  备    注  :
*******************************************************************************/
void SleepDeep_UsartIntoSleep(void)
{
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_USART1,ENABLE);
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_USART1,DISABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, DISABLE);    
}

/*******************************************************************************
**  函数名称  : SleepDeep_EnterStopMode
**  函数功能  : 进入休眠模式
**  输    入  : 无
**  输    出  :  
**  全局变量  : 
**  调用函数  : 无
**  中断资源  : 无
**  备    注  :
*******************************************************************************/
void SleepDeep_EnterStopMode(void)
{
    s_stSleepDAttrib.AccOffTimeCnt =0;
    s_stSleepDAttrib.ExtiType =EXTI_TYPE_EMPTY;    
    SleepDeep_ResetSCRSleepDeepBit();
    ///////////////////////
    SleepDeep_CloseAllUserPower();
    /////////////////
    SleepDeep_ResetWatchdog();  //重新初始化看门狗的预分频寄存器和重载寄存器的值
    ///////////////////////
    SleepDeep_ResetIoInit();
    //////////////
    SleepDeep_UsartIntoSleep();
    ////////////////
    ///////////////////////////////////////
    PWR_FlashPowerDownCmd(DISABLE);

    IWDG_ReloadCounter();   //清 独立看门狗

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);//Enable PWR clock

    PWR_BackupAccessCmd(ENABLE);
    ////////////////
    SleepDeep_ConfigRTCWakeUp();
    ///////////////////////////////
    SleepDeep_PortCtrlAccExtiInit();   
    ////////////////////////
    SleepDeep_StopMode();

    PWR_FlashPowerDownCmd(ENABLE);
    ////////////////////////////////    
    SleepDeep_PortCtrlAccExtiDisable();
    SleepDeep_RTCWakeUpDisable();
    SleepDeep_ResetSCRSleepDeepBit();
    NVIC_SystemReset();
}
/*******************************************************************************
**  函数名称  : SleepDeep_TerminalShutdown
**  函数功能  : 处理终端关机命令,Acc开重启,Acc关进入深度休眠,
                只要当前电压再升高0.5v或者由Acc关变为Acc开即重新开机
**  输    入  : 无
**  输    出  :  
**  全局变量  : 
**  调用函数  : 无
**  中断资源  : 无
**  备    注  :
*******************************************************************************/
void SleepDeep_TerminalShutdown(void)
{
    if(Io_ReadStatusBit(STATUS_BIT_ACC))//
    {
        NVIC_SystemReset();
    }
    else
    {
        LcdClearScreen();
        s_stSleepDAttrib.recoverVolt = SleepDeep_GetCurPowerValue()+5;
        SleepDeep_EnterStopMode();
    }
}
/*******************************************************************************
**  函数名称  : SleepDeep_DisposeTerminalShutdown
**  函数功能  : 处理终端关机命令,Acc开延时2秒重启,Acc关延时2秒进入深度休眠,
                只要当前电压再升高0.5v或者由Acc关变为Acc开即重新开机
**  输    入  : 无
**  输    出  :  
**  全局变量  : 
**  调用函数  : 无
**  中断资源  : 无
**  备    注  :
*******************************************************************************/
void SleepDeep_DisposeTerminalShutdown(void)
{
    LZM_PublicSetOnceTimer(&s_stDSleepTimer[SLEEP_TIMER_TASK],PUBLICSECS(2),SleepDeep_TerminalShutdown);       
}
/*******************************************************************************
**  函数名称  : SleepDeep_ReportServerEnterStopMode
**  函数功能  : 深度休眠
**  输    入  : 无				
**  输    出  : 
**  全局变量  : 
**  调用函数  : 
**  中断资源  : 
**  备    注  :
*******************************************************************************/
void SleepDeep_ReportServerEnterStopMode(void)
{
    //SetEvTask(EV_REPORT);
    Report_UploadPositionInfo(CHANNEL_DATA_1);
    Report_UploadPositionInfo(CHANNEL_DATA_2);
    LZM_PublicSetOnceTimer(&s_stDSleepTimer[SLEEP_TIMER_SEND],PUBLICSECS(1.7),SleepDeep_ReportServerEnterStopMode);
}
/*******************************************************************************
**  函数名称  : SleepDeep_CheckForEnterStopMode
**  函数功能  : 深度休眠
**  输    入  : 无				
**  输    出  : 
**  全局变量  : 
**  调用函数  : 
**  中断资源  : 
**  备    注  :
*******************************************************************************/
void SleepDeep_CheckForEnterStopMode(void)
{
    unsigned char acc;
    ///////////////////////////////
    if(0xffffffff == s_stSleepDAttrib.StopTime)
    {
        return;
    }
    ////////////////////////////////
    acc = Io_ReadStatusBit(STATUS_BIT_ACC);
    if(acc)
    {
        s_stSleepDAttrib.AccOffTimeCnt = 0;
        s_stSleepDAttrib.stopFlag = 0;
        if(s_stSleepDAttrib.clearRestartFlag)
        {
            s_stSleepDAttrib.clearRestartFlag =0;
            SetTimerTask(TIME_SYSRESET,12*SYSTICK_1HOUR);
        }
    }
    else
    {
        if(0==s_stSleepDAttrib.clearRestartFlag)
        {
            s_stSleepDAttrib.clearRestartFlag = 1;
            ClrTimerTask(TIME_SYSRESET);
        }
        s_stSleepDAttrib.AccOffTimeCnt++;
        if(s_stSleepDAttrib.AccOffTimeCnt>=s_stSleepDAttrib.StopTime)
        {
            ////////////////////
            if(SleepDeep_GetCurPowerValue()<s_stSleepDAttrib.recoverVolt)
            {
                if(0==s_stSleepDAttrib.stopFlag)
                {
                    s_stSleepDAttrib.stopFlag = 1;
                    ////////////////////////////////////
                    Io_WriteIoStatusBit(IO_STATUS_BIT_DEEP_SLEEP,SET);
                    /////////////////////////////////
                    SleepDeep_ReportServerEnterStopMode();
                    ////////////////////////////////
                    Public_ShowTextInfo("深度休眠中", PUBLICSECS(10));
                    ////////////////////////////////
                    LZM_PublicSetCycTimer(&s_stDSleepTimer[SLEEP_TIMER_TEST],PUBLICSECS(6),SleepDeep_EnterStopMode);
                }
            }
        }
    }
}
/*******************************************************************************
**  函数名称  : SleepDeep_CheckCurrentVoltType
**  函数功能  : 检查当前是哪个电压系统
**  输    入  : 无				
**  输    出  : 
**  全局变量  : 
**  调用函数  : 
**  中断资源  : 
**  备    注  :
*******************************************************************************/
void SleepDeep_CheckCurrentVoltType(void)
{
    unsigned short val;
    val = SleepDeep_GetCurPowerValue();
    if(val<180)//12V系统
    {
        s_stSleepDAttrib.VoltType =0;
    }
    else
    {
        s_stSleepDAttrib.VoltType =1;
    }
}
/*******************************************************************************
**  函数名称  : SleepDeep_StartScanTask
**  函数功能  : 深度休眠
**  输    入  : 无				
**  输    出  : 
**  全局变量  : 
**  调用函数  : 
**  中断资源  : 
**  备    注  :
*******************************************************************************/
void SleepDeep_StartScanTask(void)
{
    SleepDeep_CheckCurrentVoltType();
    SleepDeep_UpdateParam();
    LZM_PublicSetCycTimer(&s_stDSleepTimer[SLEEP_TIMER_TASK],PUBLICSECS(1),SleepDeep_CheckForEnterStopMode); 
}

/*******************************************************************************
**  函数名称  : SleepDeep_UpdateParam
**  函数功能  : 深度休眠
**  输    入  : 无				
**  输    出  : 
**  全局变量  : 
**  调用函数  : 
**  中断资源  : 
**  备    注  :
*******************************************************************************/
void SleepDeep_UpdateParam(void)
{
    #ifdef E2_SHUTDOWN_MODE_POWER_ID
    unsigned char buffer[6];
    if(EepromPram_ReadPram(E2_SHUTDOWN_MODE_TIME_ID,buffer))
    {
        s_stSleepDAttrib.StopTime = Public_ConvertBufferToLong(buffer);
        if(0 == s_stSleepDAttrib.StopTime)
        {
          s_stSleepDAttrib.StopTime =0xffffffff;
        }
        else
        if(s_stSleepDAttrib.StopTime < SLEEP_DEEP_MIN_STOP_TIME)
        {
            s_stSleepDAttrib.StopTime = SLEEP_DEEP_MIN_STOP_TIME;
        }
    }
    else
    {
        s_stSleepDAttrib.StopTime =0xffffffff;
    }
    ////////////////////////////////////
    if(EepromPram_ReadPram(E2_SHUTDOWN_MODE_POWER_ID,buffer))
    {
        s_stSleepDAttrib.recoverVolt = Public_ConvertBufferToShort(buffer);
    }
    else
    {
        if(0==s_stSleepDAttrib.VoltType)//12V
        {
            s_stSleepDAttrib.recoverVolt = WAKEUP_VOLT_12V;
        }
        else
        {
            s_stSleepDAttrib.recoverVolt = WAKEUP_VOLT_24V;
        }
    }
    #else
    s_stSleepDAttrib.StopTime =0xffffffff;
    #endif
    //////////////////////////
    if(0==s_stSleepDAttrib.VoltType)//12V系统
    {
        if((s_stSleepDAttrib.recoverVolt > SLEEPDEEP_VOLT_12V_MAX)||(s_stSleepDAttrib.recoverVolt<SLEEPDEEP_VOLT_12V_MIN))//14.6
        {
            s_stSleepDAttrib.recoverVolt = WAKEUP_VOLT_12V;
        }
    }
    else
    {
        if((s_stSleepDAttrib.recoverVolt > (SLEEPDEEP_VOLT_24V_MAX))||(s_stSleepDAttrib.recoverVolt<SLEEPDEEP_VOLT_24V_MIN))//27.2
        {
            s_stSleepDAttrib.recoverVolt = WAKEUP_VOLT_24V;
        }
    }
   
}
/*******************************************************************************
**  函数名称  : SleepDeep_ParameterInitialize
**  函数功能  : 深度休眠初始化
**  输    入  : 无				
**  输    出  : 
**  全局变量  : 
**  调用函数  : 
**  中断资源  : 
**  备    注  :
*******************************************************************************/
void SleepDeep_ParameterInitialize(void)
{
    LZM_PublicKillTimerAll(s_stDSleepTimer,SLEEP_TIMERS_MAX);
    s_stSleepDAttrib.ExtiType =EXTI_TYPE_EMPTY;
    s_stSleepDAttrib.AccOffTimeCnt = 0;
    s_stSleepDAttrib.clearRestartFlag = 0;
    s_stSleepDAttrib.VoltType =0;//12v
    SleepDeep_PortCtrlAccExtiDisable();
    SleepDeep_RTCWakeUpDisable();    
    /////////////////////////////////////////    
    SetTimerTask(TIME_SLEEPDEEP, SYSTICK_0p1SECOND);
    LZM_PublicSetOnceTimer(&s_stDSleepTimer[SLEEP_TIMER_TASK],PUBLICSECS(10),SleepDeep_StartScanTask);
}

/*******************************************************************************
**  函数名称  : SleepDeep_RTC_WKUP_IRQ
**  函数功能  : 定时器中断处理
**  输    入  : 无				
**  输    出  : 
**  全局变量  : 
**  调用函数  : 
**  中断资源  : 
**  备    注  :
*******************************************************************************/
void SleepDeep_RTC_WKUP_IRQ(void)
{
    if(RTC_GetITStatus(RTC_IT_WUT) != RESET) 
    {
        RTC_ClearITPendingBit(RTC_IT_WUT);
        EXTI_ClearITPendingBit(EXTI_Line22);
        SleepDeep_SetRtcExtiType();
    }
}
/*******************************************************************************
**  函数名称  : SleepDeep_ACC_EXTI_IRQ
**  函数功能  : Acc中断处理
**  输    入  : 无				
**  输    出  : 
**  全局变量  : 
**  调用函数  : 
**  中断资源  : 
**  备    注  :
*******************************************************************************/
void SleepDeep_ACC_EXTI_IRQ(void)
{
    if(EXTI_GetITStatus(SLEEP_DEEP_ACC_EXTI_LINE) != RESET) 
    {
        SleepDeep_SetAccExtiType();
        EXTI_ClearITPendingBit(SLEEP_DEEP_ACC_EXTI_LINE);                 //清除外部中断标志
    }
}
/*************************************************************
** 函数名称: SleepDeep_TimeTask
** 功能描述: 定时器处理函数
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
FunctionalState SleepDeep_TimeTask(void)
{
    LZM_PublicTimerHandler(s_stDSleepTimer,SLEEP_TIMERS_MAX);
    return ENABLE;
}
/******************************************************************************
**                            End Of File
******************************************************************************/


/******************************************************************** 
//版权说明	:Shenzhen E-EYE Co. Ltd., Copyright 2009- All Rights Reserved.
//文件名称	:realtime.c		
//功能		:实现实时时钟功能
//版本号	:V0.1
//开发人	:dxl
//开发时间	:2009.12
//修改者	:
//修改时间	:
//修改简要说明	:
//备注		:
***********************************************************************/

//********************************头文件************************************
#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "include.h"
//********************************自定义数据类型****************************

//********************************宏定义************************************
#define RTCClockOutput_Enable   1
//#define RTC_CLOCK_SOURCE_LSI    1
//#define FIRST_DATA          0x32F2
#define FIRST_DATA          0x32F3
//********************************全局变量**********************************
u8	AsynchPrediv = 0;
u8	SynchPrediv = 0;
RTC_TimeTypeDef RTC_TimeStructure;
RTC_InitTypeDef RTC_InitStructure;
RTC_AlarmTypeDef  RTC_AlarmStructure;

//********************************外部变量**********************************


//********************************本地变量**********************************
static u32 CurTimeID = 0;//当前时间，用ID的形式表示
TIME_T	CurTime;//当前时间,用结构体形式表示
static unsigned char s_ucRTCWeekDay;

//********************************函数声明**********************************

/*********************************************************************
//函数名称	:InitRtc(void)
//功能		:初始化RTC模块
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void InitRtc(void)
{
    u8	flag = 0;
    RTC_TimeTypeDef RTC_TimeStruct;
    RTC_DateTypeDef RTC_DateStruct;

    /* Enable the PWR clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
    //RCC_APB1PeriphClockCmd(RCC_AHB1Periph_BKPSRAM, ENABLE);
    /* Allow access to RTC */
    PWR_BackupAccessCmd(ENABLE);
    /* Reset Backup Domain */
    ///RCC_BackupResetCmd(ENABLE);
    ///RCC_BackupResetCmd(DISABLE);

    if(RTC_ReadBackupRegister(RTC_BKP_DR0) != FIRST_DATA)
    {  
        // RTC configuration 
        RTC_Config();

        //Write to the first RTC Backup Data Register
        RTC_WriteBackupRegister(RTC_BKP_DR0, FIRST_DATA);

        // Configure the RTC data register and RTC prescaler 
        RTC_InitStructure.RTC_AsynchPrediv = AsynchPrediv;
        RTC_InitStructure.RTC_SynchPrediv = SynchPrediv;
        RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;

        // Check on RTC init 
        if(RTC_Init(&RTC_InitStructure) == ERROR)
        {
        	flag = 1;
        }

        // Set the RTC current date
        RTC_DateStruct.RTC_Month = 1;
        RTC_DateStruct.RTC_Date = 1;
        RTC_DateStruct.RTC_Year = 13;
        RTC_DateStruct.RTC_WeekDay = RTC_Weekday_Thursday;
        RTC_SetDate(RTC_Format_BIN, &RTC_DateStruct);

        //Set the RTC current time 
        RTC_TimeStruct.RTC_Hours = 0;
        RTC_TimeStruct.RTC_Minutes = 0;
        RTC_TimeStruct.RTC_Seconds = 0;
        RTC_TimeStruct.RTC_H12 = RTC_H12_AM;
        RTC_SetTime(RTC_Format_BIN, &RTC_TimeStruct);

    }
    else
    {

        // Enable PWR and BKP clocks 
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
        //RCC_APB1PeriphClockCmd(RCC_AHB1Periph_BKPSRAM, ENABLE);
        // Allow access to BKP Domain 
        PWR_BackupAccessCmd(ENABLE);

        // Clear reset flags
         RCC_ClearFlag();

        /* Clear the RTC Alarm Flag */
        RTC_ClearFlag(RTC_FLAG_ALRAF);

        /* Clear the EXTI Line 17 Pending bit (Connected internally to RTC Alarm) */
        EXTI_ClearITPendingBit(EXTI_Line17);

        /* Get the RTC current date */
        //RTC_GetDate(RTC_Format_BCD, &RTC_DateStruct);

        /* Get the RTC current Time */
        ///RTC_GetTime(RTC_Format_BCD, &RTC_TimeStruct);

        /* Check if the Power On Reset flag is set */
        if(RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)
        {
        	flag = 2;
        }
        /* Check if the Pin Reset flag is set */
        else if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)
        {
        	flag = 3;
        }

    }

    if(0 == flag)
    {
        ReadRtc();
    }

}
/*********************************************************************
//函数名称	:RTC_Config(void)
//功能		:设置RTC
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:    
//备注		:
*********************************************************************/
void RTC_Config(void)
{
	
  	// Enable the PWR clock 
  	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

  	/* Allow access to RTC */
  	PWR_BackupAccessCmd(ENABLE);
/*
// LSI used as RTC source clock 
// The RTC Clock may varies due to LSI frequency dispersion.    
  // Enable the LSI OSC 
  	RCC_LSICmd(ENABLE);

 	 // Wait till LSI is ready   
  	while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
  	{
  	}

  	// Select the RTC Clock Source 
  	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
	*/
	// Enable the LSE OSC 
  	RCC_LSEConfig(RCC_LSE_ON);

  	// Wait till LSE is ready   
  	while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
  	{
  	}

  	// Select the RTC Clock Source 
  	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
   
  	/* Enable the RTC Clock */
  	RCC_RTCCLKCmd(ENABLE);

  	/* Wait for RTC APB registers synchronisation */
  	RTC_WaitForSynchro();
	
	SynchPrediv = 0xFF;
  	AsynchPrediv = 0x7F;
	
}
/*********************************************************************
//函数名称	:SetRtc(TIME_T *tt)
//功能		:设置RTC计数器的值
//输入		:tt,时间
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void SetRtc(TIME_T *tt)
{
    RTC_TimeTypeDef RTC_TimeStruct;
    RTC_DateTypeDef RTC_DateStruct;

    RTC_DateStruct.RTC_Year = tt->year;
    RTC_DateStruct.RTC_Month = tt->month;
    RTC_DateStruct.RTC_Date = tt->day;

    RTC_DateStruct.RTC_WeekDay = RTC_CacWeekDay(tt->year,tt->month,tt->day);//add by joneming
    ///////////////////////////////////
    RTC_SetDate(RTC_Format_BIN, &RTC_DateStruct);

    RTC_TimeStruct.RTC_Hours = tt->hour;
    RTC_TimeStruct.RTC_Minutes = tt->min;
    RTC_TimeStruct.RTC_Seconds = tt->sec;
    if(RTC_TimeStruct.RTC_Hours >= 12)
    {
        RTC_TimeStruct.RTC_H12 = RTC_H12_PM;
    }
    else
    {
        RTC_TimeStruct.RTC_H12 = RTC_H12_AM;
    }
    RTC_SetTime(RTC_Format_BIN, &RTC_TimeStruct);

}
/*********************************************************************
//函数名称	:ReadRtc(void)
//功能		:读实时时钟,更新CurTime结构体变量
//输入		:
//输出		:读取的结果存放在结构体tt中
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:    
//备注		:该函数要求每秒钟调度一次,以便及时地更新CurrentTimeID值
*********************************************************************/
void ReadRtc(void)
{
    RTC_TimeTypeDef RTC_TimeStruct;
    RTC_DateTypeDef RTC_DateStruct;
    //////////////////////
    //注意要先读时间再读日期,否则在跨天的时候出现时间更新了,日期未更新的情况
    /////////////
    /* Get the RTC current Time */
    RTC_GetTime(RTC_Format_BIN, &RTC_TimeStruct);

    /* Get the RTC current date */
    RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct);

    CurTime.year = RTC_DateStruct.RTC_Year;
    CurTime.month = RTC_DateStruct.RTC_Month;
    CurTime.day = RTC_DateStruct.RTC_Date;
    CurTime.hour = RTC_TimeStruct.RTC_Hours;
    CurTime.min = RTC_TimeStruct.RTC_Minutes;
    CurTime.sec = RTC_TimeStruct.RTC_Seconds;
    CurTimeID = ConverseGmtime(&CurTime);
    ////////////////////////////
    s_ucRTCWeekDay = RTC_DateStruct.RTC_WeekDay;//add by joneming
}
/*********************************************************************
//函数名称	:ConverseGmtime
//功能		:把结构体类型时间转换成32位无符号的统一计数值
//输入		:TIME_T 结构体类型时间
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:32位无符号的统一计数值
//备注		:
*********************************************************************/
u32 ConverseGmtime(TIME_T *tt)
{
    s16 i;
    s16 year;
    u32 TotalDay;
    u32 TotalSec;

    TotalDay = 0;
    TotalSec = 0;
    year = tt->year;

    if((year >= START_YEAR)&&(year <= END_YEAR))	//判断年是否符合范围
    {
        for(i = START_YEAR; i < tt->year; i++)	//计算year年之前的天数
        {
            TotalDay += (365+LeapYear(i));
        }
        TotalDay += YearDay(tt->year, tt->month, tt->day);	//加上year当年的天数
        TotalSec = (TotalDay-1)*DAY_SECOND + tt->hour*HOUR_SECOND+ tt->min*60 + tt->sec;	//计算总的秒数
    }
    return TotalSec;
}

/*********************************************************************
//函数名称	:Gmtime
//功能		:把32位无符号的统一计数值转换成结构体类型时间
//输入		:指向TIME_T类型的指针，计数值
//输出		:指向TIME_T类型的指针
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void Gmtime(TIME_T *tt, u32 counter)
{
    s16 i;
    u32 sum ;
    u32 temp;
    u32 counter1 = 0;
    u8 flag;
    unsigned short jdays[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

    //////////////////
    if(counter > 0xBC191380)//超过开始年的一百年,则不再转换
    {
        tt->year =START_YEAR;
        tt->month = 1;
        tt->day = 1;
        tt->hour = 0;
        tt->min = 0;
        tt->sec= 0;
        return;//超过开始年的一百年,则不再转换
    }
    ///////////////////////
    sum = 0;
    temp = 0;    
    for(i=START_YEAR; sum<=counter&&i<END_YEAR; i++)   /* 计算年份 */
    {
        sum += (LeapYear(i)*DAY_SECOND + YEAR_SECOND);
        if(sum <= counter)
        {
            temp= sum;
        }
    }
    ///////////////////////////
    tt->year =i-1;

    flag=LeapYear(tt->year);
    counter1=counter-temp;
    /////////////////////////
    sum=counter1;
    counter1=counter1%DAY_SECOND;
    temp =(sum-counter1)/DAY_SECOND;
    temp++;//TotalSec = (TotalDay-1)*DAY_SECOND + tt->hour*HOUR_SECOND+ tt->min*60 + tt->sec;	//计算总的秒数
    for(i=12; i>0; i--)//
    {            
        if((temp>(jdays[i-1]+flag)&&i>2)||(temp>jdays[i-1]&&i<=2))            
        {
            tt->month=i;
            break;
        }
    }
    //////////////////////////////////////
    tt->day=temp-jdays[tt->month-1];
    if(tt->month>2)tt->day-=flag;

    tt->hour =counter1/HOUR_SECOND;
    ///////////////////////////
    counter1=counter1%HOUR_SECOND;
    /////////////////////
    tt->min =counter1/MIN_SECOND;

    tt->sec=counter1%MIN_SECOND;
    /////////////////////
}
/*********************************************************************
//函数名称	:RTC_GetCounter
//功能		:获取RTC时间（以TimeID形式）
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:       
//备注:
*********************************************************************/
u32  RTC_GetCounter(void)
{
    return CurTimeID;
}
/*********************************************************************
//函数名称	:RTC_GetCurTime
//功能		:获取当前时间
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:       
//备注:
*********************************************************************/
void RTC_GetCurTime(TIME_T *tt)
{
    tt->year = CurTime.year;
    tt->month = CurTime.month;
    tt->day = CurTime.day;
    tt->hour = CurTime.hour;
    tt->min = CurTime.min;
    tt->sec = CurTime.sec;
}
/*********************************************************************
//函数名称	:LeapYear
//功能		:判断是否为闰年
//输入		:年份
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:是闰年返回1，不是闰年返回0
//备注		:year的取值为0-99
*********************************************************************/
u8 LeapYear(u8 year)
{
	s16	year2;
	
	year2 = year + 2000;
	if(((year2%4 == 0)&&(year2%100 !=0))||(year2%400 == 0))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
/*********************************************************************
//函数名称	:YearDay
//功能		:输入年、月、日，计算是该年的第几天
//输入		:年、月、日
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:返回该年的第几天
//备注		:
*********************************************************************/
u16 YearDay(u8 yy,s8 mm,s8 dd)
{
    u16 TotalDay = 0;
    unsigned short jdays[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
    TotalDay += jdays[mm-1];
    TotalDay += dd;
    if(LeapYear(yy)&&(mm > 2))
        TotalDay++;
    return TotalDay;
}
/*********************************************************************
//函数名称	:CheckTimeStruct(TIME_T *tt)
//功能		:检查时间结构体内的值是否正确
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:正确，返回SUCCESS，错误返回ERROR
//备注		:
*********************************************************************/
ErrorStatus CheckTimeStruct(TIME_T *tt)
{
    //检查年、月、日、时、分、秒时间参数
    if((tt->year > END_YEAR)||(tt->year < START_YEAR)||
    (tt->month > 12)||(tt->month < 1)||
    (tt->day > 31)||(tt->day < 1)||
    (tt->hour > 23)||(tt->hour < 0)||
    (tt->min > 59)||(tt->min < 0)||
    (tt->sec > 59)||(tt->sec < 0))
    {
        return ERROR;
    }
    else
    {
        return SUCCESS;
    }
}
/*************************************************************
** 函数名称: RTC_CacWeekDay
** 功能描述: 计算指定日期是星期几
** 入口参数: year年(2000为起始):,moonth月,day日
** 出口参数: 
** 返回参数: 星期几,1～ 7表示星期一 ～ 星期天(与stm32一致)
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char RTC_CacWeekDay(unsigned char year,unsigned char moonth,unsigned char day)
{
    static unsigned char mm=0,dd=0,yy=0,w;
    unsigned short tmpyear;    
    unsigned long temp;
    if(yy==year&&mm==moonth&&dd==day)//不用每一次都计算
    {
        return w;
    }
    ///////////////////////
    yy=year;
    mm=moonth;
    dd=day;
    tmpyear=year+2000;
    ///////////////////
	if(moonth<3)
	{
		moonth+=12;
		tmpyear--;
	}
    /////////////////////
    temp=day+1+2*moonth+3*(moonth+1)/5+tmpyear+tmpyear/4-tmpyear/100+tmpyear/400;
    //////////////////////
	w =temp%7;
    if(0==w)w=7;
	return w;
}
/*************************************************************
** 函数名称: RTC_GetCurWeekDay
** 功能描述: 取得当前是期几
** 入口参数: 
** 出口参数: 
** 返回参数: 星期几,1～ 7表示星期一 ～ 星期天
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char RTC_GetCurWeekDay(void)
{ 
	return s_ucRTCWeekDay;
}
/*********************************************************************
//函数名称	:RTC_TimeIDAdjust
//功能		:时间ID调整,在TimeID的基础上往前减或往后加sec秒
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:       
//备注		:Action==0,为往后加,非0为往前减	
*********************************************************************/
/*
u32 RTC_TimeIDAdjust(u32 TimeID, u32 sec, s8 Action)
{
	TIME_T	tt;
	TIME_T	temp;
	u32	NewTimeID;
	u8 MonTab[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
	
	NewTimeID = TimeID;
	Gmtime(&tt, TimeID);
	temp.year = 0;//不能以年为单位进行调整
	temp.month = 0;//不能以月为单位进行调整
	temp.day = sec/86400;
	temp.hour = sec%86400/3600;
	temp.min = sec%3600/60;
	temp.sec = sec%60;
	
	if(1 == Action)//时间加,向后
	{
		//调整时间
		tt.sec += temp.sec;
		if(tt.sec >= 60)
		{
			tt.sec -= 60;
			tt.min++;
		}
		tt.min += temp.min;
		if(tt.min >= 60)
		{
			tt.min -= 60;
			tt.hour++;
		}
		tt.hour += temp.hour;
		if(tt.hour >= 24)
		{
			tt.hour -= 24;
			tt.day++;
		}
		tt.day += temp.day;
		if(tt.day > MonTab[tt.month])
		{
			tt.day -= MonTab[tt.month];
		}
		tt.month += temp.month;
		if(tt.month > 12)
		{
			tt.month -= 12;
			tt.year++;
		}
		tt.year += temp.year;
	}
	else if(-1 == Action)//时间减,往前
	{
		//调整时间
		tt.sec -= temp.sec;
		if(tt.sec < 0)
		{
			tt.sec += 60;
			tt.min--;
		}
		tt.min -= temp.min;
		if(tt.min < 0)
		{
			tt.min += 60;
			tt.hour--;
		}
		tt.hour -= temp.hour;
		if(tt.hour < 0)
		{
			tt.hour += 24;
			tt.day--;
		}
		tt.day -= temp.day;
		if(tt.day < 0)
		{
			tt.day += MonTab[tt.month];
			tt.month--;
		}
		tt.month += temp.month;
		if(tt.month < 0)
		{
			tt.month += 12;
			tt.year--;
		}
		tt.year += temp.year;
	}
	if(SUCCESS == CheckTimeStruct(&tt))
	{
		NewTimeID = ConverseGmtime(&tt);
	}
	return NewTimeID;
}
*/
/*********************************************************************
//函数名称	:ConverseGmtime
//功能		:把结构体类型时间转换成32位无符号的统一计数值
//输入		:TIME_T 结构体类型时间
//Bit31-26 Bit25-22 Bit21-17 Bit16-12 Bit11-6 Bit5-0
//年（2000+）  月      日       时      分       秒
//  
//
//  
//
//备注		:
*********************************************************************/
/*
u32 ConverseGmtime(TIME_T *tt)
{
	u32	TimeID = 0;
	
	if(SUCCESS == CheckTimeStruct(tt))
	{
		//转换成ID
		TimeID |= tt->sec;
		TimeID |= (tt->min) << 6;
		TimeID |= (tt->hour) << 12;
		TimeID |= (tt->day) << 17;
		TimeID |= (tt->month) << 22;
		TimeID |= (tt->year) << 26;
		//验证
		//Gmtime(&time, TimeID);
		
	}
	return TimeID;
}
*/
/*********************************************************************
//函数名称	:Gmtime
//功能		:把32位无符号的统一计数值转换成结构体类型时间
//输入		:指向TIME_T类型的指针，计数值
//输出		:指向TIME_T类型的指针
//Bit31-26 Bit25-22 Bit21-17 Bit16-12 Bit11-6 Bit5-0
//年（2000+）  月      日       时      分       秒
//  
//备注		:
*********************************************************************/
/*
void Gmtime(TIME_T *tt, u32 counter)
{
	//u32 TimeID = 0;
	TIME_T	time;
	
	//转换
	time.sec = counter&0x3f;
	time.min = (counter >> 6)&0x3f;
	time.hour = (counter >> 12)&0x1f;
	time.day = (counter >> 17)&0x1f;
	time.month = (counter >> 22)&0x0f;
	time.year = (counter >> 26)&0x3f;
	//验证
	//TimeID = ConverseGmtime(tt);
	
	//if(TimeID != counter)
	//{
		
	//}
	if(SUCCESS == CheckTimeStruct(&time))
	{
		tt->year = time.year;
		tt->month = time.month;
		tt->day = time.day;
		tt->hour = time.hour;
		tt->min = time.min;
		tt->sec = time.sec;
	}
	
}
*/
/*********************************************************************
//函数名称	:
//功能		:
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:       
//备注:
*********************************************************************/
void Num2NumStr(u32 Num, u8 *NumStr)
{
    u8 *p;
    u8 NumChar[10];
    u8 i;
    p = NumChar;

    if(0 == Num)
    {
        *NumStr = 0;
        return ;
    }

    *p = Num/1000000000 + 48;
    p++;
    *p = Num%1000000000/100000000 + 48;
    p++;
    *p = Num%100000000/10000000 + 48;
    p++;
    *p = Num%10000000/1000000 + 48;
    p++;
    *p = Num%1000000/100000 + 48;
    p++;
    *p = Num%100000/10000 + 48;
    p++;
    *p = Num%10000/1000 + 48;
    p++;
    *p = Num%1000/100 + 48;
    p++;
    *p = Num%100/10 + 48;
    p++;
    *p = Num%10 + 48;

    p = NumStr;
    i =0;
    while(NumChar[i] == '0')
    {
        i++;
    }
    if(10 == i)  //当Num值为0时
    {
        *p = '0';
        p++;
        *p = '0';
        p++;
        *p = '\0';
    }
    else if(9 == i) //当Num值小于10时
    {
        *p = '0';
        p++;
        *p = NumChar[i];
        p++;
        *p = '\0';
    }
    else //当Num值大于等于10时
    {
        for(; i<10; i++,p++)
        {
            *p = NumChar[i];
        }
        *p = '\0';
    }

}
/*********************************************************************
//函数名称	:YearDay
//功能		:输入年、月、日，计算是该年的第几天
//输入		:年、月、日
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:返回该年的第几天
//备注		:
*********************************************************************/
u16 YearDayEx(u8 yy,s8 mm,s8 dd)
{
    u8 i;
    u16 TotalDay = 0;
    u8 MonTab[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
    for(i = 1; i < mm; i++)
    {
        TotalDay += MonTab[i];
    }
    TotalDay += dd;
    if((LeapYear(yy) == LEAP_YEAR)&&(mm > 2))
        TotalDay++;
    return TotalDay;
}
/*********************************************************************
//函数名称	:Gmtime
//功能		:把32位无符号的统一计数值转换成结构体类型时间
//输入		:指向TIME_T类型的指针，计数值
//输出		:指向TIME_T类型的指针
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void GmtimeEx(TIME_T *tt, u32 counter)
{
    s16 i;
    s16 j;
    u32 sum ;
    u32 temp;
    u32 counter1 = 0;
    u8 MonTab[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};


    /* get the year */
    sum = 0;
    temp = 0;
    j = 0;
    for(i=START_YEAR; sum<=counter;i++)   /* 计算年份 */
    {
        j++;
        sum += (LeapYear(i)*DAY_SECOND + YEAR_SECOND);
        if(sum <= counter)
        {
            temp= sum;
        }
        if(j > (END_YEAR - START_YEAR))
        {
            j = 0;
            return ;
        }
    }
    tt->year = (i-1);  

    /* get the month */
    counter1= counter - temp;  /*计算年以后剩余时间*/
    sum = 0;
    temp = 0;
    for(i=1; sum<=counter1;i++)
    {
        sum += MonTab[i]*DAY_SECOND;
        if((2 == i)&&(LeapYear(tt->year)))
        {
            sum +=  DAY_SECOND;  
        }
        if(sum <= counter1)
        {
            temp = sum;
        }
    }
    tt->month = (i-1);

    /* get the day */
    counter1 = counter1 - temp; /*计算月后剩余的时间*/
    sum = 0;
    temp = 0;
    for(i=1; sum <= counter1; i++)
    {
        sum += DAY_SECOND;
        if(sum <= counter1)
        {
            temp = sum;
        }
    }
    tt->day = (i-1);

    /* get the hour */
    counter1 = counter1 - temp; /*计算日后剩余的时间*/
    sum = 0;
    temp = 0;
    for(i=0; sum <= counter1; i++)
    {
        sum += HOUR_SECOND;
        if(sum <= counter1)
        {
            temp = sum;
        }
    }
    tt->hour = (i-1);

    /* get the minute */
    counter1 = counter1 - temp; /*计算小时后剩余的时间 */
    sum = 0;
    temp = 0;
    for(i=0; sum <= counter1; i++)
    {
        sum += MIN_SECOND;
        if(sum <= counter1)
        {
            temp = sum;
        }
    }
    tt->min = (i-1);

    /* get the second */
    tt->sec = counter1 - temp;  /*计算分钟后剩余的时间 */

}


/*******************************************************************************
 *                             end of module
 *******************************************************************************/


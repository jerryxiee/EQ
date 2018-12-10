/******************************************************************** 
//��Ȩ˵��	:Shenzhen E-EYE Co. Ltd., Copyright 2009- All Rights Reserved.
//�ļ�����	:realtime.c		
//����		:ʵ��ʵʱʱ�ӹ���
//�汾��	:V0.1
//������	:dxl
//����ʱ��	:2009.12
//�޸���	:
//�޸�ʱ��	:
//�޸ļ�Ҫ˵��	:
//��ע		:
***********************************************************************/

//********************************ͷ�ļ�************************************
#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "include.h"
//********************************�Զ�����������****************************

//********************************�궨��************************************
#define RTCClockOutput_Enable   1
//#define RTC_CLOCK_SOURCE_LSI    1
//#define FIRST_DATA          0x32F2
#define FIRST_DATA          0x32F3
//********************************ȫ�ֱ���**********************************
u8	AsynchPrediv = 0;
u8	SynchPrediv = 0;
RTC_TimeTypeDef RTC_TimeStructure;
RTC_InitTypeDef RTC_InitStructure;
RTC_AlarmTypeDef  RTC_AlarmStructure;

//********************************�ⲿ����**********************************


//********************************���ر���**********************************
static u32 CurTimeID = 0;//��ǰʱ�䣬��ID����ʽ��ʾ
TIME_T	CurTime;//��ǰʱ��,�ýṹ����ʽ��ʾ
static unsigned char s_ucRTCWeekDay;

//********************************��������**********************************

/*********************************************************************
//��������	:InitRtc(void)
//����		:��ʼ��RTCģ��
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
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
//��������	:RTC_Config(void)
//����		:����RTC
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:    
//��ע		:
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
//��������	:SetRtc(TIME_T *tt)
//����		:����RTC��������ֵ
//����		:tt,ʱ��
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
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
//��������	:ReadRtc(void)
//����		:��ʵʱʱ��,����CurTime�ṹ�����
//����		:
//���		:��ȡ�Ľ������ڽṹ��tt��
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:    
//��ע		:�ú���Ҫ��ÿ���ӵ���һ��,�Ա㼰ʱ�ظ���CurrentTimeIDֵ
*********************************************************************/
void ReadRtc(void)
{
    RTC_TimeTypeDef RTC_TimeStruct;
    RTC_DateTypeDef RTC_DateStruct;
    //////////////////////
    //ע��Ҫ�ȶ�ʱ���ٶ�����,�����ڿ����ʱ�����ʱ�������,����δ���µ����
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
//��������	:ConverseGmtime
//����		:�ѽṹ������ʱ��ת����32λ�޷��ŵ�ͳһ����ֵ
//����		:TIME_T �ṹ������ʱ��
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:32λ�޷��ŵ�ͳһ����ֵ
//��ע		:
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

    if((year >= START_YEAR)&&(year <= END_YEAR))	//�ж����Ƿ���Ϸ�Χ
    {
        for(i = START_YEAR; i < tt->year; i++)	//����year��֮ǰ������
        {
            TotalDay += (365+LeapYear(i));
        }
        TotalDay += YearDay(tt->year, tt->month, tt->day);	//����year���������
        TotalSec = (TotalDay-1)*DAY_SECOND + tt->hour*HOUR_SECOND+ tt->min*60 + tt->sec;	//�����ܵ�����
    }
    return TotalSec;
}

/*********************************************************************
//��������	:Gmtime
//����		:��32λ�޷��ŵ�ͳһ����ֵת���ɽṹ������ʱ��
//����		:ָ��TIME_T���͵�ָ�룬����ֵ
//���		:ָ��TIME_T���͵�ָ��
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
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
    if(counter > 0xBC191380)//������ʼ���һ����,����ת��
    {
        tt->year =START_YEAR;
        tt->month = 1;
        tt->day = 1;
        tt->hour = 0;
        tt->min = 0;
        tt->sec= 0;
        return;//������ʼ���һ����,����ת��
    }
    ///////////////////////
    sum = 0;
    temp = 0;    
    for(i=START_YEAR; sum<=counter&&i<END_YEAR; i++)   /* ������� */
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
    temp++;//TotalSec = (TotalDay-1)*DAY_SECOND + tt->hour*HOUR_SECOND+ tt->min*60 + tt->sec;	//�����ܵ�����
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
//��������	:RTC_GetCounter
//����		:��ȡRTCʱ�䣨��TimeID��ʽ��
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:       
//��ע:
*********************************************************************/
u32  RTC_GetCounter(void)
{
    return CurTimeID;
}
/*********************************************************************
//��������	:RTC_GetCurTime
//����		:��ȡ��ǰʱ��
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:       
//��ע:
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
//��������	:LeapYear
//����		:�ж��Ƿ�Ϊ����
//����		:���
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:�����귵��1���������귵��0
//��ע		:year��ȡֵΪ0-99
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
//��������	:YearDay
//����		:�����ꡢ�¡��գ������Ǹ���ĵڼ���
//����		:�ꡢ�¡���
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:���ظ���ĵڼ���
//��ע		:
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
//��������	:CheckTimeStruct(TIME_T *tt)
//����		:���ʱ��ṹ���ڵ�ֵ�Ƿ���ȷ
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:��ȷ������SUCCESS�����󷵻�ERROR
//��ע		:
*********************************************************************/
ErrorStatus CheckTimeStruct(TIME_T *tt)
{
    //����ꡢ�¡��ա�ʱ���֡���ʱ�����
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
** ��������: RTC_CacWeekDay
** ��������: ����ָ�����������ڼ�
** ��ڲ���: year��(2000Ϊ��ʼ):,moonth��,day��
** ���ڲ���: 
** ���ز���: ���ڼ�,1�� 7��ʾ����һ �� ������(��stm32һ��)
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
unsigned char RTC_CacWeekDay(unsigned char year,unsigned char moonth,unsigned char day)
{
    static unsigned char mm=0,dd=0,yy=0,w;
    unsigned short tmpyear;    
    unsigned long temp;
    if(yy==year&&mm==moonth&&dd==day)//����ÿһ�ζ�����
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
** ��������: RTC_GetCurWeekDay
** ��������: ȡ�õ�ǰ���ڼ�
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: ���ڼ�,1�� 7��ʾ����һ �� ������
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
unsigned char RTC_GetCurWeekDay(void)
{ 
	return s_ucRTCWeekDay;
}
/*********************************************************************
//��������	:RTC_TimeIDAdjust
//����		:ʱ��ID����,��TimeID�Ļ�������ǰ���������sec��
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:       
//��ע		:Action==0,Ϊ�����,��0Ϊ��ǰ��	
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
	temp.year = 0;//��������Ϊ��λ���е���
	temp.month = 0;//��������Ϊ��λ���е���
	temp.day = sec/86400;
	temp.hour = sec%86400/3600;
	temp.min = sec%3600/60;
	temp.sec = sec%60;
	
	if(1 == Action)//ʱ���,���
	{
		//����ʱ��
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
	else if(-1 == Action)//ʱ���,��ǰ
	{
		//����ʱ��
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
//��������	:ConverseGmtime
//����		:�ѽṹ������ʱ��ת����32λ�޷��ŵ�ͳһ����ֵ
//����		:TIME_T �ṹ������ʱ��
//Bit31-26 Bit25-22 Bit21-17 Bit16-12 Bit11-6 Bit5-0
//�꣨2000+��  ��      ��       ʱ      ��       ��
//  
//
//  
//
//��ע		:
*********************************************************************/
/*
u32 ConverseGmtime(TIME_T *tt)
{
	u32	TimeID = 0;
	
	if(SUCCESS == CheckTimeStruct(tt))
	{
		//ת����ID
		TimeID |= tt->sec;
		TimeID |= (tt->min) << 6;
		TimeID |= (tt->hour) << 12;
		TimeID |= (tt->day) << 17;
		TimeID |= (tt->month) << 22;
		TimeID |= (tt->year) << 26;
		//��֤
		//Gmtime(&time, TimeID);
		
	}
	return TimeID;
}
*/
/*********************************************************************
//��������	:Gmtime
//����		:��32λ�޷��ŵ�ͳһ����ֵת���ɽṹ������ʱ��
//����		:ָ��TIME_T���͵�ָ�룬����ֵ
//���		:ָ��TIME_T���͵�ָ��
//Bit31-26 Bit25-22 Bit21-17 Bit16-12 Bit11-6 Bit5-0
//�꣨2000+��  ��      ��       ʱ      ��       ��
//  
//��ע		:
*********************************************************************/
/*
void Gmtime(TIME_T *tt, u32 counter)
{
	//u32 TimeID = 0;
	TIME_T	time;
	
	//ת��
	time.sec = counter&0x3f;
	time.min = (counter >> 6)&0x3f;
	time.hour = (counter >> 12)&0x1f;
	time.day = (counter >> 17)&0x1f;
	time.month = (counter >> 22)&0x0f;
	time.year = (counter >> 26)&0x3f;
	//��֤
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
//��������	:
//����		:
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:       
//��ע:
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
    if(10 == i)  //��NumֵΪ0ʱ
    {
        *p = '0';
        p++;
        *p = '0';
        p++;
        *p = '\0';
    }
    else if(9 == i) //��NumֵС��10ʱ
    {
        *p = '0';
        p++;
        *p = NumChar[i];
        p++;
        *p = '\0';
    }
    else //��Numֵ���ڵ���10ʱ
    {
        for(; i<10; i++,p++)
        {
            *p = NumChar[i];
        }
        *p = '\0';
    }

}
/*********************************************************************
//��������	:YearDay
//����		:�����ꡢ�¡��գ������Ǹ���ĵڼ���
//����		:�ꡢ�¡���
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:���ظ���ĵڼ���
//��ע		:
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
//��������	:Gmtime
//����		:��32λ�޷��ŵ�ͳһ����ֵת���ɽṹ������ʱ��
//����		:ָ��TIME_T���͵�ָ�룬����ֵ
//���		:ָ��TIME_T���͵�ָ��
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
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
    for(i=START_YEAR; sum<=counter;i++)   /* ������� */
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
    counter1= counter - temp;  /*�������Ժ�ʣ��ʱ��*/
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
    counter1 = counter1 - temp; /*�����º�ʣ���ʱ��*/
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
    counter1 = counter1 - temp; /*�����պ�ʣ���ʱ��*/
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
    counter1 = counter1 - temp; /*����Сʱ��ʣ���ʱ�� */
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
    tt->sec = counter1 - temp;  /*������Ӻ�ʣ���ʱ�� */

}


/*******************************************************************************
 *                             end of module
 *******************************************************************************/


#include "include.h"
#include "integer.h"   
#include "fattime.h"
//#include "rtc.h" //RPi

//获得时间
//User defined function to give a current time to fatfs module          */
//31-25: Year(0-127 org.1980), 24-21: Month(1-12), 20-16: Day(1-31) */                                                                                                                                                                                                                                          
//15-11: Hour(0-23), 10-5: Minute(0-59), 4-0: Second(0-29 *2) */                                                                                                                                                                                                                                                
DWORD get_fattime (void)
{
    TIME_T DayTime;
    u8 Temp;
    DWORD iDayTime;
    RTC_GetCurTime(&DayTime);
    iDayTime = DayTime.year+2000-1980;
    iDayTime = iDayTime<<25 ;
    Temp = DayTime.month;
    iDayTime |= (Temp<<21);
    Temp = DayTime.day;
    iDayTime |= (Temp<<16);
    Temp = DayTime.hour;
    iDayTime |= (Temp<<11);
    Temp = DayTime.min;
    iDayTime |= (Temp<<5);
    Temp = DayTime.sec;
    iDayTime |= (Temp>>1);
	return iDayTime;
}


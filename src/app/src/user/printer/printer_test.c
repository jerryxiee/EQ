#include "stdio.h"
#include "stdlib.h"
#include "string.h"  

#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "main.h"
#include "rtc.h"
#include "EepromPram.h"
#include "Register.h"
#include "tireddrive.h"
#include "RecTravel.h"
//#include "Recorder.h",dxl,2014.5.9
#include "printer.h"
//#include "log_pw_speed.h",dxl,2014.5.9
#include "SpeedLog.h"
#include "ptr_timer.h"
#include "ptr_stepmotor_ctrl.h"
#include "ptr_temperature.h"
#include "Module_IC_Card_Eeye.h"



extern void print_ctrl_task(void);

extern void print_init(void);

struct _time_str
{
	char year[5];
	char month[3];
	char day[3];
	char hour[3];
	char minute[3];
	char second[3];
};
static void conv_time_str(char *time_buf, struct _time_str *time_str);
static u8 bcd_to_dec(u8 bcd);
static u8 dectostr(u32 decnum,char *str,u8 _bit);
static char tmp_buf[600]; //


/****************************打印内容*********************************
机动车号牌号码：×××××××
机动车号牌分类：××××
机动车驾驶证号码：
××××××××××××××××××
速度状态：××
打印时间：
yy-mm-dd hh:mm:ss
2个日历天内超时驾驶记录：
记录1：
机动车驾驶证号码：
××××××××××××××××××
连续驾驶开始时间：
yyyy-mm-dd hh:mm:ss
连续驾驶结束时间：
yyyy-mm-dd hh:mm:ss
记录2：
机动车驾驶证号码：
××××××××××××××××××
连续驾驶开始时间：
yyyy-mm-dd hh:mm:ss
连续驾驶结束时间：
yyyy-mm-dd hh:mm:ss
..
签名：
——————————
**************************************************************/
void printer_task(void)
{
#if 1
  e_PRINTER_STATUS ptr_status;
    struct _time_str time_str;
	TIME_T cur_time, start_time;
	
	u16 j,record_num = 0;
	u32 i,seconds;
	
	if(print_version == 0 || printer_status() == PTR_BUSY)
	  	return;
	
	start_ptr_timer(); //开启打印机定时器，输出PWM波形，使打印机通电
	ptr_pwr_on();
	for(i=0;i<200000;i++); //通电一段时间，等电压稳定下来
	
	force_start_T_datect();
	
	ptr_status = printer_status();
	if ( (ptr_status == PTR_NO_PAPER) 
			 || (ptr_status == PTR_OVER_T))
	{
	  	close_ptr_timer(); //关闭打印机定时器
	  	return;
	}
        
    	i = EepromPram_ReadPram(E2_CAR_PLATE_NUM_ID, (u8*)tmp_buf);
	tmp_buf[i] = '\0';
 	fmt_print("机动车号牌号码: %s\n\r",tmp_buf);
        
	i = EepromPram_ReadPram(E2_CAR_TYPE_ID, (u8*)tmp_buf);
	tmp_buf[i] = '\0';
	fmt_print("机动车号牌分类: %s\n\r",tmp_buf);
        
	fmt_print("车辆VIN:\n\r");
	Recorder_GetCarInformation((u8*)tmp_buf);//获取车辆VIN
	tmp_buf[17] = '\0';
	fmt_print("   %s\n\r", tmp_buf);
	
        i = EepromPram_ReadPram(E2_ENGINE_NUM_ID, (u8*)tmp_buf);
	tmp_buf[i] = '\0';
 	fmt_print("车辆发动机号码: %s\n\r",tmp_buf);
        
	i = ICCard_GetExternalDriverName((u8*)tmp_buf);
	tmp_buf[i] = '\0';
 	fmt_print("驾驶员姓名: %s\n\r",tmp_buf);

	i = EepromPram_ReadPram(E2_CURRENT_LICENSE_ID, (u8*)tmp_buf);
        //i = ICCard_GetExternalDriverDriveID((u8*)tmp_buf);
	tmp_buf[i] = '\0';
	fmt_print("机动车驾驶证号码:\n\r");
	fmt_print("   %s\n\r",tmp_buf);
        
	i = EepromPram_ReadPram(E2_CURRENT_DRIVER_ID, (u8*)tmp_buf);
        seconds=(u32)tmp_buf[0]<<16|(u32)tmp_buf[1]<<8|tmp_buf[2];
        i =dectostr(seconds, tmp_buf, 3); 
	tmp_buf[i] = '\0';
 	fmt_print("驾驶员代码: %s\n\r",tmp_buf);
	

	//----暂时没有速度状态这项参数，这里固定为正常 !!!!!!!!!!!!!
	fmt_print("速度状态: "); 
	//if(0 == GetSpeedLogStatus())//先强制打印正常,dxl,2015.2.8
		fmt_print("正常\n\r");
	//else
	    //fmt_print("异常\n\r"); 
	//-----------------------------------------------------------
	
	RTC_GetCurTime(&cur_time);
	conv_time_str((char*)&cur_time, &time_str);
	fmt_print("打印时间:\n\r");
	fmt_print("   %s-%s-%s %s:%s:%s\n\r",\
    time_str.year,time_str.month,time_str.day,\
	time_str.hour,time_str.minute,time_str.second);

	i = Recorder_Stop15MinuteSpeed((u8*)tmp_buf);//获取停车时间和停车前15分钟速度

	fmt_print("停车时间:\n\r");
	tmp_buf[0] = bcd_to_dec(tmp_buf[0]);
	tmp_buf[1] = bcd_to_dec(tmp_buf[1]);
	tmp_buf[2] = bcd_to_dec(tmp_buf[2]);
	tmp_buf[3] = bcd_to_dec(tmp_buf[3]);
	tmp_buf[4] = bcd_to_dec(tmp_buf[4]);
	tmp_buf[5] = bcd_to_dec(tmp_buf[5]);
	conv_time_str(&tmp_buf[0], &time_str);
	fmt_print("   %s-%s-%s %s:%s:%s\n\r",\
	time_str.year,time_str.month,time_str.day,\
	time_str.hour,time_str.minute,time_str.second);
	
	
	fmt_print("停车前15分钟速度:\n\r");
	for(j=6; j < i; j+=3)
	{
	  	char min_str[3];
		char sec_str[3];
		
	  	tmp_buf[j] = bcd_to_dec(tmp_buf[j]);dectostr(tmp_buf[j], min_str, 2);
		tmp_buf[j+1] = bcd_to_dec(tmp_buf[j+1]);dectostr(tmp_buf[j+1],sec_str,2);
		fmt_print("   %s:%s   %dkm/h\n\r",min_str,sec_str,tmp_buf[j+2]);
	}

    seconds = (24 + cur_time.hour)*3600 + cur_time.min*60 + cur_time.sec; 
	seconds = ConverseGmtime(&cur_time) - seconds;
	Gmtime(&start_time, seconds);
	
	//超时驾驶是指连续驾驶超过4小时，所以两个日历天内最多只有12条
	//i = Register_Read(REGISTER_TYPE_OVER_TIME, (u8*)tmp_buf, start_time, cur_time, 12);
	i = TiredDrive_Read2daysTiredRecord((u8*)tmp_buf,start_time);

	record_num = i / REGISTER_OVER_TIME_COLLECT_LEN;
	
	if(record_num == 0)
	{
	 	fmt_print("2个日历天内超时驾驶记录: 无记录\n\r",i); 	
	}
	else
	{
		fmt_print("2个日历天内超时驾驶记录:\n\r");
		
		for(i = 0; i < record_num; i++)
		{
		  	u8 buf[20];
			char *pt = &tmp_buf[i * REGISTER_OVER_TIME_COLLECT_LEN];
			
			fmt_print("记录%d:\n\r",i+1);
		
			memcpy(buf, pt, 18);
			buf[18] = '\0';
			fmt_print("机动车驾驶证号码:\n\r");
			fmt_print("   %s\n\r",buf);
			
			fmt_print("连续驾驶开始时间:\n\r");  //注意：这里的时间是BCD码格式的
			pt[18] = bcd_to_dec(pt[18]);
			pt[19] = bcd_to_dec(pt[19]);
			pt[20] = bcd_to_dec(pt[20]);
			pt[21] = bcd_to_dec(pt[21]);
			pt[22] = bcd_to_dec(pt[22]);
			pt[23] = bcd_to_dec(pt[23]);
	    	conv_time_str(&pt[18], &time_str);
			fmt_print("   %s-%s-%s %s:%s:%s\n\r",\
    		time_str.year,time_str.month,time_str.day,\
			time_str.hour,time_str.minute,time_str.second);
			
			fmt_print("连续驾驶结束时间:\n\r");
			pt[24] = bcd_to_dec(pt[24]);
			pt[25] = bcd_to_dec(pt[25]);
			pt[26] = bcd_to_dec(pt[26]);
			pt[27] = bcd_to_dec(pt[27]);
			pt[28] = bcd_to_dec(pt[28]);
			pt[29] = bcd_to_dec(pt[29]);
	    	conv_time_str(&pt[24], &time_str);
			fmt_print("   %s-%s-%s %s:%s:%s\n\r",\
    		time_str.year,time_str.month,time_str.day,\
			time_str.hour,time_str.minute,time_str.second);
		}
	}

	fmt_print("\n\r");
	fmt_print("\n\r");
	fmt_print("签名:______\n\r");
	fmt_print("\n\r");
	fmt_print("........................\n\r");
	//fmt_print("------------------------\n\r");
	fmt_print("\n\r");
	fmt_print("\n\r");
#else	
	fmt_print("一一一一一一一一一一一一一一一一\n\r");
	fmt_print("一一一一一一一一一一一一一一一\n\r");
	fmt_print("一一一一一一一一一一一一一一\n\r");
	fmt_print("一一一一一一一一一一一一一\n\r");
	fmt_print("一一一一一一一一一一一一\n\r");
	fmt_print("一一一一一一一一一一一\n\r");
	fmt_print("一一一一一一一一一一\n\r");
	fmt_print("一一一一一一一一一\n\r");
	fmt_print("一一一一一一一一\n\r");
	fmt_print("一一一一一一一\n\r");
	fmt_print("一一一一一一\n\r");
	fmt_print("一一一一一\n\r");
	fmt_print("一一一一\n\r");
	fmt_print("一一一\n\r");
	fmt_print("一一\n\r");
	fmt_print("一\n\r");
	fmt_print("\n\r");
	fmt_print("\n\r");
#endif

}
/***********************************************
** 将BCD码转换为十进制数
** 如：0x16 => 16
************************************************/
static u8 bcd_to_dec(u8 bcd)	
{
	return ((bcd&0x0F)+(bcd>>4)*10);
}

/*********************************************************************
** 将十进制数转换为字符串
**********************************************************************/
static u8 dectostr(u32 decnum,char *str,u8 _bit)
{
   u8 pr;
   u8 i;
   u32 de;
   u8 flag=0;
   u32 temp;
   u8 bb, seg;
   

	//先判断decnum的有效位数
	temp = decnum;
	de=10;
	bb=1;
	for(i=0;i<10;i++)
	{
	 	if(temp/de>0)
			bb++;
		else
		 	break;
		de*=10;
	}
	seg=0;
   	if(_bit>bb)
	{
		for(i=0;i<(_bit-bb);i++)
		{
			str[seg]='0';
			seg++;	
		}
	}
	de=1000000000;
   	for(i=0;i<10;i++)
   	{
	 	pr=decnum/de%10;
	 	de/=10;
	 	if((pr==0)&&(flag==0)&&(de>0))
	 	continue;
	 	flag=1; 
	 	if(seg>128)return 0;
	 	str[seg]=pr+'0';
	 	seg++;
 	}
	str[seg]='\0'; //加上结束符
	return seg;
}

static void conv_time_str(char *time_buf, struct _time_str *time_str)
{
	u16 tmp;
	
	tmp = 2000 + time_buf[0];
    dectostr(tmp, time_str->year, 4); 
	
    dectostr(time_buf[1],time_str->month,2);
	
	dectostr(time_buf[2],time_str->day,2);
	
	dectostr(time_buf[3],time_str->hour,2);
	
	dectostr(time_buf[4],time_str->minute,2);
	
	dectostr(time_buf[5],time_str->second,2);
	
}




void test_printer(void)
{
  	//u32 tick =0;
	
  	char flag = 0;
	
  	print_init(); //打印机初始化
	
  	while(1)
  	{
    	if (GPIO_ReadInputDataBit(GPIOF, GPIO_Pin_14) == Bit_SET)
		{
			if(flag == 0)
			{
			  	//printer_task();
			 	flag = 1;
				
			}
    	}
		else
		{
       		if(flag==1)
			{
			  	printer_task();
				flag = 0;
			}
    	}
		
		print_ctrl_task();
	}
}

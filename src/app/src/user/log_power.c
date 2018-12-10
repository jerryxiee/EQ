#define	_EXT_PW_

#include "include.h" 

#define		RECORD_SEC_CONST	5
#define		OFFSET_VOL_CONST	300

//转换函数
void HexToBcd(uchar *p_bcd,uchar *p_hex,uchar length)
{
	uchar i=0,tmp;
	for(i=0;i<length;i++)
	{
		tmp=*p_hex++;
		*p_bcd++=(tmp/10)*16+tmp%10;
		
	}
	
}

//懂显林 --》大家好！记录仪数据记录部分基本上调试好，请在调用记录接口
//Register_Write(u8 Type,u8 *pBuffer, u16 length); 
//时注意记录的长度，即length，length请按如下约定填写，长度不对时会返回ERROR。
// 
//行驶速度及状态2字节+预留4字节（预留字节先填充0x00），共6字节，
//位置信息10字节+平均速度1字节+里程4字节+预留4字节（预留字节先填充0x00），共19字节
//疑点数据234字节+预留12字节（预留字节先填充0x00），共246字节
//超时驾驶50字节+预留12字节（预留字节先填充0x00），共62字节
//驾驶人身份25字节+预留6字节（预留字节先填充0x00），共31字节
//外部供电7字节+预留6字节（预留字节先填充0x00），共13字节
//参数修改7字节+预留6字节（预留字节先填充0x00），共13字节
//速度状态133字节+预留6字节（预留字节先填充0x00），共139字节

/**********************************************************************************/
/*function	name: 			void CheckMainPowerRun_TimeTask(void)		*/
/*input parameter:			none  */
/*output parameter:			none */
/*FUNCTION: 			//记录电源通断日志信息，调度任务200ms;实时检测，记录断电函数
存储记录格式：
供电类型供电时间（BCD码格式）	事件类型
年 月 日	　时 分 秒	1：通电，2：断电
3字节	3字节	1字节
*/
/**********************************************************************************/
FunctionalState CheckMainPowerRun_TimeTask(void)
{
	 static uint tmp0;
	static TIME_T	old_time;
	uchar buf[RECORD_PWER_LENGTH];
	_API_PWER_RE_	record_pwr;
	static uchar semaphore,flag=false;
	u32 new_sec;
	static u32 old_sec,old_time_0;
	
	//确认主电供电数据,获取主电压
	//time=
	//ReadRtc();
	RTC_GetCurTime(&record_pwr.time);
	new_sec=RTC_GetCounter();
	//上电自动记录一次，2013-03-27增加
	if(flag==false)
		{
					//无备电记录
					//去读铁电存储器，记录上次断电后，无备电的记录
					FRAM_BufferRead((uchar*)&old_time,sizeof(old_time),FRAM_POWER_LOG_TIME);
					//转换函数
					
					HexToBcd(buf,(uchar*)&old_time,sizeof(old_time));
					//record_pwr.status=MAIN_PW_STOP;
					buf[6]=MAIN_PW_STOP;
					//秒与时间结构转换
					old_sec=ConverseGmtime(&old_time);
					//写存储
					if(buf[0]!=0xff)//防止无初始化
						{
							Register_Write2( REGISTER_TYPE_POWER_ON,buf,RECORD_PWER_LENGTH,old_sec);	
//						 //LOG_PR("Time :old_sec=[%d]\r\n",old_sec);	
//						 
//    		
//              LOG_PR("FRAM_TIME:rtc=[%d-%d-%d %d:%d:%d]\r\n",\
//              	buf[0],buf[1],buf[2],
//              	buf[3],buf[4],buf[5]
//             );	
             						 					
						}
				

//              LOG_PR("CheckMainPowerRun_TimeTask:rtc=[%d-%d-%d %d:%d:%d--%d]\r\n",\
//              	old_time.time.year,old_time.time.month,old_time.time.day,
//              	old_time.time.hour,old_time.time.min,old_time.time.sec,
//              	old_time.status);					

//					//上电记录			
					flag=true;
//					record_pwr.status=MAIN_PW_RUN;
//				//	memcpy(buf,(uchar*)&record_pwr,sizeof(record_pwr));
//					HexToBcd(buf,(uchar*)&record_pwr,sizeof(record_pwr));
//					//写存储
//					Register_Write2( REGISTER_TYPE_POWER_ON,buf,RECORD_PWER_LENGTH);	
						
				old_time_0=new_sec;
					
		}
	else if(new_sec-old_time_0>2)
		{
	tmp0=Adc_MainPower_Readl();
	if(tmp0>REFRENCE_12V_OR_24V)
		{
			if(tmp0<MAIN_PW_UNDER_24V)
				{
					//获取当前时间函数
					//	record_pwr.time=
					
					if(semaphore==true)//false)
						{
							semaphore=false;//true;	
			
							record_pwr.status=MAIN_PW_STOP;
							//memcpy(buf,(uchar*)&record_pwr,sizeof(record_pwr));
							HexToBcd(buf,(uchar*)&record_pwr,sizeof(record_pwr));
							//写存储
							Register_Write2( REGISTER_TYPE_POWER_ON,buf,RECORD_PWER_LENGTH,new_sec);
//              LOG_PR("CheckMainPowerRun_TimeTask:rtc=[%d-%d-%d %d:%d:%d--%d]\r\n",\
//              	record_pwr.time.year,record_pwr.time.month,record_pwr.time.day,
//              	record_pwr.time.hour,record_pwr.time.min,record_pwr.time.sec,
//              	record_pwr.status);
						}
					
				}
			else if(tmp0>MAIN_PW_UNDER_24V+OFFSET_VOL_CONST)//主电工作
				if(semaphore==false)//true)
				{
					semaphore=true;//false;	
		
					record_pwr.status=MAIN_PW_RUN;
					//memcpy(buf,(uchar*)&record_pwr,sizeof(record_pwr));
					HexToBcd(buf,(uchar*)&record_pwr,sizeof(record_pwr));
					//写存储
					Register_Write2( REGISTER_TYPE_POWER_ON,buf,RECORD_PWER_LENGTH,new_sec);	
//					 LOG_PR("CheckMainPowerRun_TimeTask:rtc=[%d-%d-%d %d:%d:%d--%d]\r\n",
//              	record_pwr.time.year,record_pwr.time.month,record_pwr.time.day,
//              	record_pwr.time.hour,record_pwr.time.min,record_pwr.time.sec,
//              	record_pwr.status);				
				}
		}
	else 
		{
			if(tmp0<MAIN_PW_UNDER_12V)
				{
					//获取当前时间
					//record_pwr.time=
					//RTC_GetCurTime(&record_pwr.time);
					if(semaphore==true)//false)
						{
							semaphore=false;//true;	
							
							record_pwr.status=MAIN_PW_STOP;
							
							//memcpy(buf,(uchar*)&record_pwr,sizeof(record_pwr));
							HexToBcd(buf,(uchar*)&record_pwr,sizeof(record_pwr));
							//写存储
							Register_Write2( REGISTER_TYPE_POWER_ON,buf,RECORD_PWER_LENGTH,new_sec);
						}					
					
				}
			else if(tmp0>MAIN_PW_UNDER_12V+OFFSET_VOL_CONST)//主电工作
				if(semaphore==false)//true)
				{
					semaphore=true;//false;
					
					record_pwr.status=MAIN_PW_RUN;
					//memcpy(buf,(uchar*)&record_pwr,sizeof(record_pwr));
					HexToBcd(buf,(uchar*)&record_pwr,sizeof(record_pwr));
					//写存储
					Register_Write2( REGISTER_TYPE_POWER_ON,buf,RECORD_PWER_LENGTH,new_sec);
					
					
				}	
		}
		
	}
	//记录实时时间，每间隔5秒写一次时间到铁电存储器，2013-03-27-PM
	
	if(new_sec-old_sec>RECORD_SEC_CONST||new_sec-old_sec==RECORD_SEC_CONST)
		{
			old_sec=new_sec;
			//写的铁电存储器
			FRAM_BufferWrite(FRAM_POWER_LOG_TIME,(uchar*)&record_pwr,sizeof(record_pwr));
			
//		LOG_PR("Time :new_sec=[%d]\r\n",new_sec);	
			
		}	

	return	ENABLE;
}
#define	_EXT_PW_

#include "include.h" 

#define		RECORD_SEC_CONST	5
#define		OFFSET_VOL_CONST	300

//ת������
void HexToBcd(uchar *p_bcd,uchar *p_hex,uchar length)
{
	uchar i=0,tmp;
	for(i=0;i<length;i++)
	{
		tmp=*p_hex++;
		*p_bcd++=(tmp/10)*16+tmp%10;
		
	}
	
}

//������ --����Һã���¼�����ݼ�¼���ֻ����ϵ��Ժã����ڵ��ü�¼�ӿ�
//Register_Write(u8 Type,u8 *pBuffer, u16 length); 
//ʱע���¼�ĳ��ȣ���length��length�밴����Լ����д�����Ȳ���ʱ�᷵��ERROR��
// 
//��ʻ�ٶȼ�״̬2�ֽ�+Ԥ��4�ֽڣ�Ԥ���ֽ������0x00������6�ֽڣ�
//λ����Ϣ10�ֽ�+ƽ���ٶ�1�ֽ�+���4�ֽ�+Ԥ��4�ֽڣ�Ԥ���ֽ������0x00������19�ֽ�
//�ɵ�����234�ֽ�+Ԥ��12�ֽڣ�Ԥ���ֽ������0x00������246�ֽ�
//��ʱ��ʻ50�ֽ�+Ԥ��12�ֽڣ�Ԥ���ֽ������0x00������62�ֽ�
//��ʻ�����25�ֽ�+Ԥ��6�ֽڣ�Ԥ���ֽ������0x00������31�ֽ�
//�ⲿ����7�ֽ�+Ԥ��6�ֽڣ�Ԥ���ֽ������0x00������13�ֽ�
//�����޸�7�ֽ�+Ԥ��6�ֽڣ�Ԥ���ֽ������0x00������13�ֽ�
//�ٶ�״̬133�ֽ�+Ԥ��6�ֽڣ�Ԥ���ֽ������0x00������139�ֽ�

/**********************************************************************************/
/*function	name: 			void CheckMainPowerRun_TimeTask(void)		*/
/*input parameter:			none  */
/*output parameter:			none */
/*FUNCTION: 			//��¼��Դͨ����־��Ϣ����������200ms;ʵʱ��⣬��¼�ϵ纯��
�洢��¼��ʽ��
�������͹���ʱ�䣨BCD���ʽ��	�¼�����
�� �� ��	��ʱ �� ��	1��ͨ�磬2���ϵ�
3�ֽ�	3�ֽ�	1�ֽ�
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
	
	//ȷ�����繩������,��ȡ����ѹ
	//time=
	//ReadRtc();
	RTC_GetCurTime(&record_pwr.time);
	new_sec=RTC_GetCounter();
	//�ϵ��Զ���¼һ�Σ�2013-03-27����
	if(flag==false)
		{
					//�ޱ����¼
					//ȥ������洢������¼�ϴζϵ���ޱ���ļ�¼
					FRAM_BufferRead((uchar*)&old_time,sizeof(old_time),FRAM_POWER_LOG_TIME);
					//ת������
					
					HexToBcd(buf,(uchar*)&old_time,sizeof(old_time));
					//record_pwr.status=MAIN_PW_STOP;
					buf[6]=MAIN_PW_STOP;
					//����ʱ��ṹת��
					old_sec=ConverseGmtime(&old_time);
					//д�洢
					if(buf[0]!=0xff)//��ֹ�޳�ʼ��
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

//					//�ϵ��¼			
					flag=true;
//					record_pwr.status=MAIN_PW_RUN;
//				//	memcpy(buf,(uchar*)&record_pwr,sizeof(record_pwr));
//					HexToBcd(buf,(uchar*)&record_pwr,sizeof(record_pwr));
//					//д�洢
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
					//��ȡ��ǰʱ�亯��
					//	record_pwr.time=
					
					if(semaphore==true)//false)
						{
							semaphore=false;//true;	
			
							record_pwr.status=MAIN_PW_STOP;
							//memcpy(buf,(uchar*)&record_pwr,sizeof(record_pwr));
							HexToBcd(buf,(uchar*)&record_pwr,sizeof(record_pwr));
							//д�洢
							Register_Write2( REGISTER_TYPE_POWER_ON,buf,RECORD_PWER_LENGTH,new_sec);
//              LOG_PR("CheckMainPowerRun_TimeTask:rtc=[%d-%d-%d %d:%d:%d--%d]\r\n",\
//              	record_pwr.time.year,record_pwr.time.month,record_pwr.time.day,
//              	record_pwr.time.hour,record_pwr.time.min,record_pwr.time.sec,
//              	record_pwr.status);
						}
					
				}
			else if(tmp0>MAIN_PW_UNDER_24V+OFFSET_VOL_CONST)//���繤��
				if(semaphore==false)//true)
				{
					semaphore=true;//false;	
		
					record_pwr.status=MAIN_PW_RUN;
					//memcpy(buf,(uchar*)&record_pwr,sizeof(record_pwr));
					HexToBcd(buf,(uchar*)&record_pwr,sizeof(record_pwr));
					//д�洢
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
					//��ȡ��ǰʱ��
					//record_pwr.time=
					//RTC_GetCurTime(&record_pwr.time);
					if(semaphore==true)//false)
						{
							semaphore=false;//true;	
							
							record_pwr.status=MAIN_PW_STOP;
							
							//memcpy(buf,(uchar*)&record_pwr,sizeof(record_pwr));
							HexToBcd(buf,(uchar*)&record_pwr,sizeof(record_pwr));
							//д�洢
							Register_Write2( REGISTER_TYPE_POWER_ON,buf,RECORD_PWER_LENGTH,new_sec);
						}					
					
				}
			else if(tmp0>MAIN_PW_UNDER_12V+OFFSET_VOL_CONST)//���繤��
				if(semaphore==false)//true)
				{
					semaphore=true;//false;
					
					record_pwr.status=MAIN_PW_RUN;
					//memcpy(buf,(uchar*)&record_pwr,sizeof(record_pwr));
					HexToBcd(buf,(uchar*)&record_pwr,sizeof(record_pwr));
					//д�洢
					Register_Write2( REGISTER_TYPE_POWER_ON,buf,RECORD_PWER_LENGTH,new_sec);
					
					
				}	
		}
		
	}
	//��¼ʵʱʱ�䣬ÿ���5��дһ��ʱ�䵽����洢����2013-03-27-PM
	
	if(new_sec-old_sec>RECORD_SEC_CONST||new_sec-old_sec==RECORD_SEC_CONST)
		{
			old_sec=new_sec;
			//д������洢��
			FRAM_BufferWrite(FRAM_POWER_LOG_TIME,(uchar*)&record_pwr,sizeof(record_pwr));
			
//		LOG_PR("Time :new_sec=[%d]\r\n",new_sec);	
			
		}	

	return	ENABLE;
}
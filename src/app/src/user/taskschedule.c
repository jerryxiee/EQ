/********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS 
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "include.h"
/* extern function -----------------------------------------------------------*/
void EvNopHandleTask();
FunctionalState TimerNopHandleTask();

/* Private typedef -----------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
TIME_TASK MyTimerTask[MAX_TIMETASK] = {             
        {ENABLE, 0, 1, Communication_TimeTask},                 	// 0:����ͨ��,ʱ����Ȼ�׼Ϊ50ms,Communication_TimeTask,ʹ��
		{ENABLE, 0, SYSTICK_0p1SECOND, RadioProtocolParse_TimeTask},   // 1:����Э�����   
		{ENABLE, 0, 1,Io_TimeTask},    					// 2:IO ״̬���  Io_TimeTask,ʹ��
		{ENABLE, 0, 1*SYSTICK_1SECOND, Gps_TimeTask},  			// 3:GPS���ݴ���,ʹ��    
		{ENABLE,0,1*SYSTICK_1SECOND,GpsMile_TimeTask},		        // 4:GPS��̼���,xx:xx:00����  GpsMile_TimeTask,ʹ��
		{ENABLE,0,1,Report_TimeTask},  	                // 5:λ�û㱨   Report_TimeTask,ʹ��
		{ENABLE, 0, 1,Position_TimeTask}, 		// 6:��ʻ��¼��λ����Ϣ��¼,ʹ��,�������ܼ��ʱ�ر�
		//{ENABLE, 0, 1,Position_TimeTask}, 		                // 6:��ʻ��¼��λ����Ϣ��¼,50msдһ������,��360h����ʱʹ��
		{ENABLE, 0, 5*SYSTICK_0p1SECOND,Can_TimeTask},		        // 7:CAN�������� ,ʹ��   
		{ENABLE,1,1,Camera_TimeTask},				        // 8:����ͷ���� ,ʹ��           
		{DISABLE, 0, 1*SYSTICK_1SECOND, Media_UploadTimeTask},        	// 9:�����ý�����ݵ�ƽ̨  Media_UploadTimeTask
		{DISABLE,0,SYSTICK_1SECOND,TimerNopHandleTask},                 // 10:��
		{DISABLE, 0, 1*SYSTICK_1SECOND, SpeedMonitor_TimeTask},        	// 11:���ټ��, ʵ��ʹ��ʱxx:xx:00����,ʹ��,�������ܼ��ʱ�ر�
        {DISABLE, 0, 1*SYSTICK_1SECOND, TiredDrive_TimeTask},     	// 12:ƣ�ͼ�ʻ,ʵ��ʹ��ʱxx:xx:00��������IC��ǩ��ǩ��ģ�鿪��,�������ܼ��ʱ�ر�
        {ENABLE,0,SYSTICK_0p1SECOND,DoubtPoint_TimeTask},  	        // 13:�ɵ�����,ʹ��,�������ܼ��ʱ�ر�    
        {ENABLE,0,SYSTICK_1SECOND,HeartBeat_TimeTask},                	// 14:������   HeartBeat_TimeTask,����RTC,ʹ��
		{ENABLE,0,SYSTICK_0p1SECOND,Usart3_TimeTask},				        // 15:USART3����3��ʱ��������,����ͷ�õĴ˴���  Usart3_TimeTask,ʹ��
		{DISABLE,0,1,TimerNopHandleTask},             	                // 16:ԭ���Ǵ�ӡ�������ڷ���while(1)��
        {DISABLE, 0, 1*SYSTICK_1SECOND, RecorderWireless_TimeTask},     // 17:���߲ɼ���¼������
        {ENABLE, 0, SYSTICK_0p1SECOND, RecorderWired_TimeTask},    	// 18:���߲ɼ���¼������  ,ʹ��
		{DISABLE, 0, 1*SYSTICK_1SECOND, RadioProtocolResend_TimeTask}, 	// 19:����Э���ط�       
		{DISABLE, 0, 1, DelayTrigTimeTask},    	        		// 20:��ʱ����   DelayTrigTimeTask
		{ENABLE, 0, 1*SYSTICK_1SECOND, Reset_TimeTask},  // 21:24Сʱ��ϵͳ����,  Reset_TimeTask��ʵ��ʹ��ʱ����
        //{ENABLE, 12*SYSTICK_1HOUR, 12*SYSTICK_1HOUR, Reset_TimeTask},  // 21:24Сʱ��ϵͳ����,  Reset_TimeTask��ʵ��ʹ��ʱ����
		//{ENABLE,0,1,BMA250EApp_TimeTask},				// 22:��ײ�������,ʹ��,dxl,2014.4.16
		{ENABLE,0,5,BMA250_TimeTask},				// 22:��ײ�������,ʹ��,dxl,2014.4.16
		//{DISABLE,0,1,BMA220App_TimeTask},                             // 22:��ʱ���Է���I2C�ӿڻ��ǻ���볬ʱ�ȴ������
        {ENABLE,0,2*SYSTICK_0p1SECOND,Pulse_TimeTask},			// 23:��������ٶ�,ʵ��ʹ��ʱxx:xx:00����  ,ʹ��,�������ܼ��ʱ�ر�
		{DISABLE,0,10*SYSTICK_1SECOND,Link1Login_TimeTask},		// 24:����1��¼��Ȩ   Link1Login_TimeTask
		{ENABLE,0,1,Usb_AppTimeTask},					// 25:USB  Usb_AppTimeTask,ʹ��
		{ENABLE,0,1,TTS_TimeTask}, 					// 26:TTS�������� ,ʹ��,��Ҫע��GPSģ��̼�����ʱ���ܲ������� 
		{ENABLE,0,1,Adc_CollectApp_TimeTask},			        // 27:AD�ɼ�,ʹ�� 
		{ENABLE,0,SYSTICK_1SECOND,Sleep_TimeTask},			// 28:����  Sleep_TimeTask,ʹ��
		{ENABLE,1,1,KeyBoardAdc_TimeTask},                              // 29:����ɨ��,ʹ��
        {DISABLE,1,2,TimerNopHandleTask},                               // 30:ԭ���ǹ����������δʹ��
		{ENABLE,1,1,SpeedLog_TimeTask},               			// 31:�ٶ��쳣��¼,ʹ��,�������ܼ��ʱ�ر�
        {ENABLE,1,SYSTICK_1SECOND,CheckMainPowerRun_TimeTask},          // 32:�����¼,ʹ��,�������ܼ��ʱ�ر�
        {DISABLE,0,3*SYSTICK_1SECOND,TimerNopHandleTask},               // 33:ԭ����485�ط���������δʹ��
		{DISABLE,0,RECORD_TASK_TIME,RecordTask} ,			// 34:¼���ɼ�  RECORD_TASK_TIME,RecordTask,δ���,ʹ��
		{ENABLE,0,1,TaskMenuMessage_TimeTask},                          // 35:LCD������ʾ����ʾ,ʹ��
    	{DISABLE,1,SYSTICK_1SECOND,TimerNopHandleTask},                 // 36:��
		{DISABLE,0, 0,TimerNopHandleTask},		                // 37:ԭ����M12 TTS����  M12_TTS_TASK_TIME,M12TTSPlay_TimeTask,����δʹ��
		{ENABLE,0, SYSTICK_1SECOND,AreaManage_TimeTask}, 		// 38:���� ,ʹ�� 
		{ENABLE,0, SYSTICK_1SECOND,Media_TimeTask}, 			// 39:ɾ����ý���ļ�  ,ʹ��,SD��δ��������ݲ����
		{DISABLE,0, RECORD_TIME_TASK_TIME,Record_TimeTask},                                 // 40:��ʱ¼���ر�����   RECORD_TIME_TASK_TIME,Record_TimeTask,ʹ��,SD��δ��������ݲ����
		{ENABLE,0,3*SYSTICK_0p1SECOND,Route_TimeTask},                      // 41: ·�߼��  ROUTE_TASK_TIME,
		{ENABLE,0,1*SECOND,CoeffAdust_TimeTask},                        // 42:�����Զ�У׼  CoeffAdust_TimeTask,ʵ��ʹ��ʱ�ſ���,V1.01.03��ſ���
		{DISABLE,0,1*SECOND,TimerNopHandleTask},                     // 43:�ϴ��¼�����Ƭ , SD��δ��������ݲ���� 
	    {DISABLE, 0,1*SECOND,TimerNopHandleTask},       	        // 44:�ϱ�һ������λ����Ϣ  Report_TimeTask2,Ŀǰʹ��Ч������,�ݲ�����
		{DISABLE,0,1*SECOND,ControlOil_TimeTask},                        // 45:����·   ControlOil_TimeTask,δʵ�֣�ֻ�ڷ������ʵ����
		{ENABLE,0,1,CommICCard_TimeTask},	  			// 46: USART6��ʱ�շ����� (����IC��ͨѶ),Ĭ��ʹ���ǿ�����
        {DISABLE,0,3*SECOND,TimerNopHandleTask},  		// 47: USART6��ʱ�ط�    (����IC��ͨѶ)
		{DISABLE,0,  1  ,ICCard_TimeTask},			// 48: IC����������,Ĭ��ʹ���ǿ�����
		{DISABLE, 0, ICAUC_TIME_TICK, ICAUC_TimeTask},                   // 49:IC����֤TCP��������,Ĭ��ʹ���ǿ�����,��1���汾���������Ӳ����ƣ���˹ر�,dxl,2014.7.15
        //{DISABLE,0,3*SECOND,TimerIC_CARDHandleTask},	 	        // 48: IC����������,����Ӱ�����߹�����˹ر�  
		//{DISABLE, 0, ICAUC_TIME_TICK, ICAUC_TimeTask},                // 49:IC����֤TCP��������,����Ӱ�����߹�����˹ر�
        {DISABLE,0,SECOND,NaviLcd_TimerTask},                           // 50: ������ 
		//{ ENABLE , 0 , 2*SECOND , Update_TimeTask    }, //51. �ְ���������,
        { DISABLE , 0 , 1*SECOND , LoaderControllerUpdata_TimeTask}, //51. ��ʹ�ְܷ���������
        {DISABLE, 0, SECOND, MultiCenterConnect_TimeTask},              // 52:����������,�������ܲ���ʱʹ��
        {ENABLE, 0, 1, GnssRealData_TimeTask},	                        // 53������ʵʱ��������
		{ENABLE, 0, 5, Gnss_App_TimeTask},	                        // 54:�����������ݵ�ƽ̨����
        {DISABLE, 0, 1, OpenAccount_TimeTask},	                        // 55:���ٿ���
        {DISABLE, 0, SECOND, Communication_CloseGprsTimeTask},	        // 56:�ر�GPRS������ʹ��ǰ����һ
        {DISABLE, 0, 2*SECOND, Subpackage_UploadTimeTask},              // 57:�ְ���������
		{ENABLE,0,SECOND,Photo_TimeTask},				// 58:���������ƵĶ�ʱ������������,
		{DISABLE,0,2,SleepDeep_TimeTask},                      		// 59:�������
        {DISABLE,0,2,Polygon_TimeTask},                      		// 63:�����
        {ENABLE,0,1*SECOND,JointechOiLCost_TimeTask },                  //64: ���ھ�ͨ�ͺĴ���������};
        {ENABLE,0,1*SECOND,Updata_SendResultNoteTimeTask},          //67:�����������֪ͨ
        {DISABLE,0,1,CarLoad_TimeTask},                            //68:����
        {ENABLE,0,1*SECOND,Corner_TimeTask},                            //69:�յ㲹��
        {DISABLE,0,2,LZM_PublicTimerTask},                       //70:����
        {DISABLE,0,10*SYSTICK_1SECOND,Link2Login_TimeTask},		// 71:����2��¼��Ȩ   Link2Login_TimeTask
        {ENABLE,0,1,Blind_TimeTask},  	                // 72:ä������
        {ENABLE,0,1,StatusAlarm_TimeTask},  	                // 73:״̬�仯��������������ϱ�������
        {ENABLE,0,1,Temperature_TimeTask},                      //74:�¶Ȳɼ�������
        {ENABLE,0,1*SECOND,Can_ErQi_TimeTask},                      //75:����CAN����
        {ENABLE,0,1*SECOND,Acceleration_TimeTask},       //76:�����٣������� zengliang add   2016-11-10��SYSTICK_0p1SECOND��ΪSECOND 
};
EVEN_TASK MyEvTask[MAX_EVENTASK] = {                                 
                   {DISABLE, EvNopHandleTask},     		// 0:����CAN����SendCanData_EvTask
				   {DISABLE, EvNopHandleTask},        	        // 1:CAN�������ݴ��� ,DecodeCan_EvTask
                   {DISABLE, Gps_EvTask},       		// 2:GPS���ݴ��� Gps_EvTask
                   {DISABLE, EvNopHandleTask},       	        // 3:�ϱ�һ��λ����Ϣ 
				   {DISABLE, GetAkey_EvTask},			// 4: ��ȡ��Ȩ������ 
                   {DISABLE, ChangeServer_EvTask},		// 5: �л����������ƽ̨ 
				   {DISABLE, FtpUpdata_EvTask},			// 6: FTP�������� 
				   {DISABLE, Dialling_EvTask},			// 7: һ��ͨ�� 
				   {DISABLE, SevDial_EvTask},			// 8: �������ذε绰���� 
				   {DISABLE, EvRing},                           // 9:�������� 
                   {DISABLE, EvRingOver},                       // 10:��ֹ���� 
				   {DISABLE, Lcd_ShowText_EvTask},              // 11:��ʾ�ı���Ϣ  
			
                           
                       };
                                               
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/


/*******************************************************************************
* Function Name  : InitTimerTask
* Description    : Initialize Schedule Timer task .
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void InitTimerTask(void)
{
  //u8 i;
  /*
  for(i = 1; i < MAX_TIMETASK; i++)
  {
    MyTimerTask[i].TaskTimerState = DISABLE;
  }
  */
  //���ӳ�ʼ������,��������ִ�к���ʱ��  
}

/*******************************************************************************
* Function Name  : SetTimerTask
* Description    : Setup timer task .
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SetTimerTask(TIMERTASK task, u32 time)
{
  MyTimerTask[task].TaskTimerState = ENABLE;
  MyTimerTask[task].TaskExTimer = time;
  MyTimerTask[task].TaskTimerCount = time + Timer_Val();
}

/*******************************************************************************
* Function Name  : ClrTimerTask
* Description    : Cancel timer task .
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ClrTimerTask(TIMERTASK task)
{
  MyTimerTask[task].TaskTimerState = DISABLE;
}
/*******************************************************************************
* Function Name  : TimerTaskScheduler
* Description    : Schedule Timer task .
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TimerTaskScheduler(void)
{
  u8 i;
  u32 nowtimer;
  FunctionalState validflag;
  
  nowtimer = Timer_Val();
  for(i = 0; i < MAX_TIMETASK; i++)
  {
     if(MyTimerTask[i].TaskTimerState != ENABLE)
     {
         continue;
     }

    if(nowtimer >= MyTimerTask[i].TaskTimerCount)
    {
        MyTimerTask[i].TaskTimerCount = nowtimer + MyTimerTask[i].TaskExTimer;
        validflag = MyTimerTask[i].operate();
        MyTimerTask[i].TaskTimerState = validflag;
    }
  }
}


/*******************************************************************************
* Function Name  : InitEvTask
* Description    : Initialize Schedule Timer task .
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void InitEvTask(void)
{
  u8 i;
  
  for(i = 0; i < MAX_EVENTASK; i++)
  {
    MyEvTask[i].evTaskOnState = DISABLE;
  }
  //���ӳ�ʼ������
}

/*******************************************************************************
* Function Name  : SetEvTask
* Description    : Setup even task .
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SetEvTask(u8 ev)
{
  MyEvTask[ev].evTaskOnState = ENABLE;
}

/*******************************************************************************
* Function Name  : EvTaskScheduler
* Description    : Schedule even task .
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EvTaskScheduler(void)
{
  u8 i;

  for(i = 0; i < MAX_EVENTASK; i++)
  {
    if(MyEvTask[i].evTaskOnState != ENABLE)
      continue;
    MyEvTask[i].evTaskOnState = DISABLE;
    MyEvTask[i].operate();
   
  }
} 
/*******************************************************************************
* Function Name  : EvNopHandleTask
* Description    : ���¼����� .
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
FunctionalState TimerNopHandleTask()
{
  
  return DISABLE;
}

/*******************************************************************************
* Function Name  : EvNopHandleTask
* Description    : ���¼����� .
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EvNopHandleTask()
{

}
/*******************************************************************************
* Function Name  : SendCanData_EvTask
* Description    : ����CAN����
* Input          : None
* Output         : None
* Return         : None
* Note           :��Ҫ���͵�CAN���ݷ���RadioProtocolMiddleBuffer,����ΪRadioProtocolMiddleLen
*******************************************************************************/
void SendCanData_EvTask(void)
{
        //���÷���CAN���ݵĺ���
        
}
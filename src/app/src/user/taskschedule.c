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
        {ENABLE, 0, 1, Communication_TimeTask},                 	// 0:无线通信,时间调度基准为50ms,Communication_TimeTask,使能
		{ENABLE, 0, SYSTICK_0p1SECOND, RadioProtocolParse_TimeTask},   // 1:无线协议解析   
		{ENABLE, 0, 1,Io_TimeTask},    					// 2:IO 状态检测  Io_TimeTask,使能
		{ENABLE, 0, 1*SYSTICK_1SECOND, Gps_TimeTask},  			// 3:GPS数据处理,使能    
		{ENABLE,0,1*SYSTICK_1SECOND,GpsMile_TimeTask},		        // 4:GPS里程计算,xx:xx:00开启  GpsMile_TimeTask,使能
		{ENABLE,0,1,Report_TimeTask},  	                // 5:位置汇报   Report_TimeTask,使能
		{ENABLE, 0, 1,Position_TimeTask}, 		// 6:行驶记录仪位置信息记录,使能,北斗功能检测时关闭
		//{ENABLE, 0, 1,Position_TimeTask}, 		                // 6:行驶记录仪位置信息记录,50ms写一条数据,灌360h数据时使用
		{ENABLE, 0, 5*SYSTICK_0p1SECOND,Can_TimeTask},		        // 7:CAN接收任务 ,使能   
		{ENABLE,1,1,Camera_TimeTask},				        // 8:摄像头拍照 ,使能           
		{DISABLE, 0, 1*SYSTICK_1SECOND, Media_UploadTimeTask},        	// 9:传输多媒体数据到平台  Media_UploadTimeTask
		{DISABLE,0,SYSTICK_1SECOND,TimerNopHandleTask},                 // 10:空
		{DISABLE, 0, 1*SYSTICK_1SECOND, SpeedMonitor_TimeTask},        	// 11:超速监控, 实际使用时xx:xx:00开启,使能,北斗功能检测时关闭
        {DISABLE, 0, 1*SYSTICK_1SECOND, TiredDrive_TimeTask},     	// 12:疲劳驾驶,实际使用时xx:xx:00开启，由IC卡签到签退模块开启,北斗功能检测时关闭
        {ENABLE,0,SYSTICK_0p1SECOND,DoubtPoint_TimeTask},  	        // 13:疑点数据,使能,北斗功能检测时关闭    
        {ENABLE,0,SYSTICK_1SECOND,HeartBeat_TimeTask},                	// 14:心跳包   HeartBeat_TimeTask,更新RTC,使能
		{ENABLE,0,SYSTICK_0p1SECOND,Usart3_TimeTask},				        // 15:USART3串口3定时接收任务,摄像头用的此串口  Usart3_TimeTask,使能
		{DISABLE,0,1,TimerNopHandleTask},             	                // 16:原来是打印任务，现在放在while(1)中
        {DISABLE, 0, 1*SYSTICK_1SECOND, RecorderWireless_TimeTask},     // 17:无线采集记录仪数据
        {ENABLE, 0, SYSTICK_0p1SECOND, RecorderWired_TimeTask},    	// 18:有线采集记录仪数据  ,使能
		{DISABLE, 0, 1*SYSTICK_1SECOND, RadioProtocolResend_TimeTask}, 	// 19:无线协议重发       
		{DISABLE, 0, 1, DelayTrigTimeTask},    	        		// 20:延时触发   DelayTrigTimeTask
		{ENABLE, 0, 1*SYSTICK_1SECOND, Reset_TimeTask},  // 21:24小时后系统重启,  Reset_TimeTask，实际使用时开启
        //{ENABLE, 12*SYSTICK_1HOUR, 12*SYSTICK_1HOUR, Reset_TimeTask},  // 21:24小时后系统重启,  Reset_TimeTask，实际使用时开启
		//{ENABLE,0,1,BMA250EApp_TimeTask},				// 22:碰撞报警检测,使能,dxl,2014.4.16
		{ENABLE,0,5,BMA250_TimeTask},				// 22:碰撞报警检测,使能,dxl,2014.4.16
		//{DISABLE,0,1,BMA220App_TimeTask},                             // 22:有时调试发现I2C接口还是会进入超时等待的情况
        {ENABLE,0,2*SYSTICK_0p1SECOND,Pulse_TimeTask},			// 23:检测脉冲速度,实际使用时xx:xx:00开启  ,使能,北斗功能检测时关闭
		{DISABLE,0,10*SYSTICK_1SECOND,Link1Login_TimeTask},		// 24:连接1登录鉴权   Link1Login_TimeTask
		{ENABLE,0,1,Usb_AppTimeTask},					// 25:USB  Usb_AppTimeTask,使能
		{ENABLE,0,1,TTS_TimeTask}, 					// 26:TTS语音播报 ,使能,需要注意GPS模块固件升级时不能播报语音 
		{ENABLE,0,1,Adc_CollectApp_TimeTask},			        // 27:AD采集,使能 
		{ENABLE,0,SYSTICK_1SECOND,Sleep_TimeTask},			// 28:休眠  Sleep_TimeTask,使能
		{ENABLE,1,1,KeyBoardAdc_TimeTask},                              // 29:按键扫描,使能
        {DISABLE,1,2,TimerNopHandleTask},                               // 30:原来是广告屏，现在未使用
		{ENABLE,1,1,SpeedLog_TimeTask},               			// 31:速度异常记录,使能,北斗功能检测时关闭
        {ENABLE,1,SYSTICK_1SECOND,CheckMainPowerRun_TimeTask},          // 32:供电记录,使能,北斗功能检测时关闭
        {DISABLE,0,3*SYSTICK_1SECOND,TimerNopHandleTask},               // 33:原来是485重发任务，现在未使用
		{DISABLE,0,RECORD_TASK_TIME,RecordTask} ,			// 34:录音采集  RECORD_TASK_TIME,RecordTask,未完成,使能
		{ENABLE,0,1,TaskMenuMessage_TimeTask},                          // 35:LCD集成显示屏显示,使能
    	{DISABLE,1,SYSTICK_1SECOND,TimerNopHandleTask},                 // 36:空
		{DISABLE,0, 0,TimerNopHandleTask},		                // 37:原来是M12 TTS播报  M12_TTS_TASK_TIME,M12TTSPlay_TimeTask,现在未使用
		{ENABLE,0, SYSTICK_1SECOND,AreaManage_TimeTask}, 		// 38:区域 ,使能 
		{ENABLE,0, SYSTICK_1SECOND,Media_TimeTask}, 			// 39:删除多媒体文件  ,使能,SD卡未完成所有暂不添加
		{DISABLE,0, RECORD_TIME_TASK_TIME,Record_TimeTask},                                 // 40:定时录音关闭任务   RECORD_TIME_TASK_TIME,Record_TimeTask,使能,SD卡未完成所有暂不添加
		{ENABLE,0,3*SYSTICK_0p1SECOND,Route_TimeTask},                      // 41: 路线检查  ROUTE_TASK_TIME,
		{ENABLE,0,1*SECOND,CoeffAdust_TimeTask},                        // 42:脉冲自动校准  CoeffAdust_TimeTask,实际使用时才开启,V1.01.03版才开启
		{DISABLE,0,1*SECOND,TimerNopHandleTask},                     // 43:上传事件类照片 , SD卡未完成所有暂不添加 
	    {DISABLE, 0,1*SECOND,TimerNopHandleTask},       	        // 44:上报一条短信位置信息  Report_TimeTask2,目前使用效果不好,暂不开启
		{DISABLE,0,1*SECOND,ControlOil_TimeTask},                        // 45:断油路   ControlOil_TimeTask,未实现，只在分体机上实现了
		{ENABLE,0,1,CommICCard_TimeTask},	  			// 46: USART6定时收发任务 (用于IC卡通讯),默认使用是开启的
        {DISABLE,0,3*SECOND,TimerNopHandleTask},  		// 47: USART6定时重发    (用于IC卡通讯)
		{DISABLE,0,  1  ,ICCard_TimeTask},			// 48: IC卡各类事务,默认使用是开启的
		{DISABLE, 0, ICAUC_TIME_TICK, ICAUC_TimeTask},                   // 49:IC卡认证TCP连接任务,默认使用是开启的,第1个版本多中心连接不完善，因此关闭,dxl,2014.7.15
        //{DISABLE,0,3*SECOND,TimerIC_CARDHandleTask},	 	        // 48: IC卡各类事务,可能影响上线功能因此关闭  
		//{DISABLE, 0, ICAUC_TIME_TICK, ICAUC_TimeTask},                // 49:IC卡认证TCP连接任务,可能影响上线功能因此关闭
        {DISABLE,0,SECOND,NaviLcd_TimerTask},                           // 50: 导航屏 
		//{ ENABLE , 0 , 2*SECOND , Update_TimeTask    }, //51. 分包升级任务,
        { DISABLE , 0 , 1*SECOND , LoaderControllerUpdata_TimeTask}, //51. 不使能分包升级任务
        {DISABLE, 0, SECOND, MultiCenterConnect_TimeTask},              // 52:多中心连接,北斗功能测试时使用
        {ENABLE, 0, 1, GnssRealData_TimeTask},	                        // 53：北斗实时解析数据
		{ENABLE, 0, 5, Gnss_App_TimeTask},	                        // 54:北斗发送数据到平台任务
        {DISABLE, 0, 1, OpenAccount_TimeTask},	                        // 55:快速开户
        {DISABLE, 0, SECOND, Communication_CloseGprsTimeTask},	        // 56:关闭GPRS，用于使用前锁定一
        {DISABLE, 0, 2*SECOND, Subpackage_UploadTimeTask},              // 57:分包补传任务
		{ENABLE,0,SECOND,Photo_TimeTask},				// 58:无张数限制的定时定距拍照任务,
		{DISABLE,0,2,SleepDeep_TimeTask},                      		// 59:深度休眠
        {DISABLE,0,2,Polygon_TimeTask},                      		// 63:多边形
        {ENABLE,0,1*SECOND,JointechOiLCost_TimeTask },                  //64: 深圳久通油耗传感器任务};
        {ENABLE,0,1*SECOND,Updata_SendResultNoteTimeTask},          //67:发送升级结果通知
        {DISABLE,0,1,CarLoad_TimeTask},                            //68:载重
        {ENABLE,0,1*SECOND,Corner_TimeTask},                            //69:拐点补传
        {DISABLE,0,2,LZM_PublicTimerTask},                       //70:公共
        {DISABLE,0,10*SYSTICK_1SECOND,Link2Login_TimeTask},		// 71:连接2登录鉴权   Link2Login_TimeTask
        {ENABLE,0,1,Blind_TimeTask},  	                // 72:盲区补报
        {ENABLE,0,1,StatusAlarm_TimeTask},  	                // 73:状态变化或产生报警立即上报或拍照
        {ENABLE,0,1,Temperature_TimeTask},                      //74:温度采集器接入
        {ENABLE,0,1*SECOND,Can_ErQi_TimeTask},                      //75:二汽CAN接入
        {ENABLE,0,1*SECOND,Acceleration_TimeTask},       //76:急加速，急减速 zengliang add   2016-11-10由SYSTICK_0p1SECOND改为SECOND 
};
EVEN_TASK MyEvTask[MAX_EVENTASK] = {                                 
                   {DISABLE, EvNopHandleTask},     		// 0:发送CAN数据SendCanData_EvTask
				   {DISABLE, EvNopHandleTask},        	        // 1:CAN总线数据处理 ,DecodeCan_EvTask
                   {DISABLE, Gps_EvTask},       		// 2:GPS数据处理 Gps_EvTask
                   {DISABLE, EvNopHandleTask},       	        // 3:上报一条位置信息 
				   {DISABLE, GetAkey_EvTask},			// 4: 获取鉴权码任务 
                   {DISABLE, ChangeServer_EvTask},		// 5: 切换到政府监管平台 
				   {DISABLE, FtpUpdata_EvTask},			// 6: FTP无线升级 
				   {DISABLE, Dialling_EvTask},			// 7: 一键通话 
				   {DISABLE, SevDial_EvTask},			// 8: 服务器回拔电话功能 
				   {DISABLE, EvRing},                           // 9:来电响铃 
                   {DISABLE, EvRingOver},                       // 10:终止响铃 
				   {DISABLE, Lcd_ShowText_EvTask},              // 11:显示文本信息  
			
                           
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
  //增加初始化函数,并且增加执行函数时间  
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
  //增加初始化函数
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
* Description    : 空事件处理 .
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
* Description    : 空事件处理 .
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EvNopHandleTask()
{

}
/*******************************************************************************
* Function Name  : SendCanData_EvTask
* Description    : 发送CAN数据
* Input          : None
* Output         : None
* Return         : None
* Note           :需要发送的CAN数据放在RadioProtocolMiddleBuffer,长度为RadioProtocolMiddleLen
*******************************************************************************/
void SendCanData_EvTask(void)
{
        //调用发送CAN数据的函数
        
}
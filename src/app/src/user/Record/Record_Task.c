/**
  ******************************************************************************
  * @file    Record_Task.c 
  * @author  
  * @version V1.0.2
  * @date    
  * @brief   录音任务调度接口
  *          使用说明:
  *          1.使用GetRecordFlag()获取状态,如果为0则可以调用RecordTask_Start()开启录音.
  *
  *
  * 更新记录:
  *          2012-08-10:1.添加录音最大时间为可设置;
  *                     2.添加保存多媒体ID;录音初始化不关闭扬声器的;
  *                     3.录音关闭时关闭麦克风（注意与电话控制差异）；
  *                     4.使用新接口Io_WriteSpeakerConrtolBit()来开关麦克风。
  *          2012-08-13:1.修改在录音任务在调度初始化中，默认不开启。
  *                     2.在录音初始化任务中，如果不开启录音设置RECORD_TIME_LEN为一个超大值。
  *
  *               
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "stm32f2xx_lib.h"
#include "G711.h"
#include "wav.h"
#include "include.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
__IO u8 REC_TO_G711_FLAG = 0;
uint32_t G711_LEN = 0;          //最大值=0xFFFFFFFF/8000=536870s/60=8947min/60=149h

const uint32_t RECORD_TIME_MAXLEN = 1*60;  //60*SYSTICK_1SECOND=1min * 5=5min
uint32_t RECORD_TIME_LEN = 0;             //记录一次最大录音时长=0xFFFFFFFF/10 ,可能导致的溢出问题，因为设备24小时重启一次(所有数据清零),所以在此处经过计算并不会溢出。

u32 START_SPACE_TIME = 0;        //启动间隔配置

RecordDef RECORD_DEF;            //启动配置

//音频采集缓冲区
uint32_t AUDIO_SAMPLE_LEN = DMA_MAX_SZE;
uint16_t AUDIO_SAMPLE[DMA_MAX_SZE];
uint8_t AUDIO_REC_G711[DMA_MAX_SZE/2];

FATFS wav_fs;
//char *audioG7File="1:ag7_c.wav";     //保存G711编码文件

extern u16     DelayAckCommand;//延时应答命令,dxl,2013.5.29
extern u8      RecordSendFlag;//录音文件上传标志,1为上传
extern u8	SdOnOffFlag;//SD卡在线标志；1在线，0不在线,当sd卡初始化失败时该标志也会清0
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  时间任务调度执行函数  
  * @note   缓冲区值能保存500ms数据，所以必须保证500ms内能调度一次，否则会被后续数据覆盖.
  * @param  None
  * @retval DISABLE下次不执行，ENABLE继续执行
  */
FunctionalState RecordTask(void)
{
  if(0 == SdOnOffFlag)
  {
    return ENABLE;//如果sd卡不在,或发生了错误直接退出,dxl,2014.6.18
  }
  if(Timer_Val() > RECORD_TIME_LEN )   //如果超过最大录制时间5分钟，则自动重新生成一个文件
  {
    RecordTask_Split();
  }

  switch(REC_TO_G711_FLAG)
  {
    case 0:
      break;
    case 1:
      PCM16ConverG711(AUDIO_REC_G711,AUDIO_SAMPLE,(AUDIO_SAMPLE_LEN/2));
      //Wav_Write_Data((void *)AUDIO_REC_G711, AUDIO_SAMPLE_LEN/4);
      //G711_LEN += AUDIO_SAMPLE_LEN/4;
      REC_TO_G711_FLAG = 0;
      break;
    case 2:
      PCM16ConverG711(&AUDIO_REC_G711[AUDIO_SAMPLE_LEN/4],&AUDIO_SAMPLE[AUDIO_SAMPLE_LEN/2],(AUDIO_SAMPLE_LEN/2));
      //Wav_Write_Data((void *)&AUDIO_REC_G711[AUDIO_SAMPLE_LEN/4], AUDIO_SAMPLE_LEN/4);
      //G711_LEN += AUDIO_SAMPLE_LEN/4;
      Wav_Write_Data((void *)AUDIO_REC_G711, AUDIO_SAMPLE_LEN/2);   //必须一次写入4K,刚好一个块，(否则会进入缓冲区，断电后无法保存,也可能不是4K寻地址会导致失败。)
      G711_LEN += AUDIO_SAMPLE_LEN/2;
      REC_TO_G711_FLAG = 0;  
      break;
    default:
      break;
  }  
 
  return ENABLE;
}
/**
  * @brief  录音定时任务 
  * @note   如果设置了定时录音,则在本任务内判断。
  * @param  None
  * @retval DISABLE下次不执行，ENABLE继续执行
  */
FunctionalState Record_TimeTask(void)
{
  if(GetRecordTime() > RECORD_DEF.RecordTime)
  {
    Io_WriteRecordConrtolBit(RECORD_BIT_TIME, RESET);
    return DISABLE;
  }
  return ENABLE;
}

/**
  * @brief  录音任务初始化  
  * @param  None
  * @retval None
  */
void RecordTask_Init(void)
{
  I2S_Audio_Control_GPIO_Init();
  RrcordReadDef();
  ////LOG_PR("RecordTask_Init:StartFlag=[%D],RecordID=[%D]\r\n",RECORD_DEF.StartFlag,RECORD_DEF.RecordID);
  if(RECORD_DEF.RecordTimeMaxLen == 0)                           //单个文件录制最大值不能为0，所以当为0，使用默认设置值。
  {
    RECORD_DEF.RecordTimeMaxLen = RECORD_TIME_MAXLEN;            //设置默认最大值
  }    
  if(1 ==  RECORD_DEF.StartFlag)
  {
    RECORD_DEF.StartFlag = 0;
    if(RECORD_DEF.RecordTimeSpace > 0)
    {
      if(RECORD_DEF.RecordTime > RECORD_DEF.RecordTimeSpace)
      {
         RECORD_DEF.RecordTime -= RECORD_DEF.RecordTimeSpace;      //如果是定时录音,则要减去已录音的长度. 
      }else{
         RECORD_DEF.RecordTime = 0;
      }
      //LOG_PR("RecordTask_Init:RecordTime=[%D]\r\n",RECORD_DEF.RecordTime);
    }
    RecordTask_Start(8000,RECORD_DEF.RecordTimeMaxLen);
    if(RECORD_DEF.RecordTime > 0 )
    {
      StartRecordTime(RECORD_DEF.RecordTime);
    }
  }else{
    RECORD_TIME_LEN = 0xFFFFFFF;                                 //如果不在录音状态，设置一个超大值，否则在Task中进入Split().
  }
  if(RECORD_DEF.RecordID == 0)                                   //由于多媒体ID不能为0,所以当为0自动递增
  {
    RECORD_DEF.RecordID++;
  }
}
/**
  * @brief  开始执行录音任务  
  * @param  RecordTime:录制时间,0-循环录制,其他-录制时间(单位秒)
  * @param  AudioFreq:采样频率，默认为8000,这里暂时未启用
  * @param  maxTime:一个录音文件最长时间,单位秒
  * @retval None
  */
void RecordTask_Start(u32 AudioFreq,u16 maxTime)
{
  u8 audioFileName[25]; 
  //LOG_PR("RecordTask_Start:StartFlag=[%d]\r\n",RECORD_DEF.StartFlag);
  if(0 == RECORD_DEF.StartFlag)
  {
      memset(audioFileName,0,sizeof(audioFileName));         
      RECORD_DEF.StartFlag = 1;
      RECORD_DEF.RecordTimeMaxLen = maxTime; 
      RrcordWriteDef();                                          //重新保存录音时间
      Media_GetFileName(audioFileName,MEDIA_TYPE_WAV); 
      //LOG_PR("RecordTask_Start:audioFileName=[%s]\r\n",audioFileName);  
      Wav_Write_G711((char*)audioFileName,WAV_FILE_MODE);        //写wav文件头  
      Audio_Record_Init();
      SetTimerTask(TIME_RECORD,RECORD_TASK_TIME);                //启动设置调度任务
      G711_LEN = 0;
      REC_TO_G711_FLAG = 0;
      RECORD_TIME_LEN = Timer_Val() + (RECORD_DEF.RecordTimeMaxLen*SYSTICK_1SECOND);  //使用系统定时器，计时更准确
  }
}

/**
  * @brief  关闭任务,停止执行  - 外部执行
  * @param  None
  * @retval None
  */
void RecordTask_Close(void)
{
  //LOG_PR("RecordTask_Close:StartFlag=[%d]\r\n",RECORD_DEF.StartFlag);  
  if(1 == RECORD_DEF.StartFlag)
  {
    Audio_Record_Close();                                          //关闭I2S接口
    Wav_Close(G711_LEN,RECORD_DEF.RecordID);                       //接受完成关闭    
    //LOG_PR("RecordTask_Close:RecordID=[%d] \r\n",RECORD_DEF.RecordID);    
    Media_SaveLog(RECORD_DEF.RecordID++,MEDIA_TYPE_WAV);           //媒体ID保存成功     
    ClrTimerTask(TIME_RECORD);                                     //关闭时间调度任务
    G711_LEN = 0;
    REC_TO_G711_FLAG = 0;  
    RECORD_TIME_LEN = 0;   
    RECORD_DEF.StartFlag = 0;
    RECORD_DEF.RecordTime = 0;
    RECORD_DEF.RecordTimeSpace = 0;
    RrcordWriteDef();                                              //关闭保存配置
    if(1 == RecordSendFlag)
    {
    	DelayAckCommand  = 0x8804;//延时任务,行标送检添加,dxl
    	//触发延时任务
    	SetTimerTask(TIME_DELAY_TRIG, SYSTICK_0p1SECOND);//延时5个调度时间,确保刚录制的文件已保存好
    }
  }
}

/**
  * @brief  该函数暂时未使用,关闭任务,停止执行  -调度函数内部执行
  * @param  None
  * @retval None
  */
FunctionalState Auto_RecordTask_Close(void)
{ 
  //LOG_PR("Auto_RecordTask_Close:StartFlag=[%d]\r\n",RECORD_DEF.StartFlag);    
  if(1 == RECORD_DEF.StartFlag)
  {
    Codec_AudioInterface_DeInit();                      //关闭I2S接口
    Wav_Close(G711_LEN,RECORD_DEF.RecordID);            //接受完成关闭    
    //LOG_PR("Auto_RecordTask_Close:RecordID=[%d]\r\n",RECORD_DEF.RecordID);      
    Media_SaveLog(RECORD_DEF.RecordID++,MEDIA_TYPE_WAV);            //媒体ID保存成功     
    G711_LEN = 0;
    REC_TO_G711_FLAG = 0;  
    RECORD_TIME_LEN = 0;   
    RECORD_DEF.StartFlag = 0;
    RECORD_DEF.RecordTime = 0;
    RECORD_DEF.RecordTimeSpace = 0;  
    RrcordWriteDef();               //关闭保存配置
  }
  return DISABLE;                    //关闭时间调度任务
}

/**
  * @brief  如果录音打过5分钟,则需要分割为5分钟一个文件 
  * @param  None
  * @retval None
  */
void RecordTask_Split(void)
{
  u8 audioFileName[25];   
  //LOG_PR("RecordTask_Split:StartFlag=[%d]\r\n",RECORD_DEF.StartFlag);   
  if(1 == RECORD_DEF.StartFlag)
  {
    memset(audioFileName,0,sizeof(audioFileName));
    
    Wav_Close(G711_LEN,RECORD_DEF.RecordID);                  //接受完成关闭   
    //LOG_PR("RecordTask_Split:RecordID=[%d]\r\n",RECORD_DEF.RecordID);  
    Media_SaveLog(RECORD_DEF.RecordID++,MEDIA_TYPE_WAV);                    //媒体ID保存成功    
    RrcordWriteDef();                                         //关闭保存配置
    G711_LEN = 0;
    RECORD_TIME_LEN = Timer_Val() + (RECORD_DEF.RecordTimeMaxLen*SYSTICK_1SECOND);
    
    Media_GetFileName(audioFileName,MEDIA_TYPE_WAV);
    //LOG_PR("RecordTask_Split:audioFileName=[%s]\r\n",audioFileName);   
    Wav_Write_G711((char*)audioFileName,WAV_FILE_MODE);        //写wav文件头   
  }
}

/*********************************************************************
//函数名称	:Camera_GetFileName
//功能		:获取文件名
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
u8  Audio_GetFileName(u8 *pBuffer)
{
	TIME_T  tt;
	RTC_GetCurTime(&tt);
	//根目录路径
	*pBuffer = '1';
	*(pBuffer+1) = ':';
	*(pBuffer+2) = '/';
	//文件名
	*(pBuffer+3) = tt.year/10+0x30;
	*(pBuffer+4) = tt.year%10+0x30;
	*(pBuffer+5) = tt.month/10+0x30;
	*(pBuffer+6) = tt.month%10+0x30;
	*(pBuffer+7) = tt.day/10+0x30;
	*(pBuffer+8) = tt.day%10+0x30;
	*(pBuffer+9) = tt.hour/10+0x30;
	*(pBuffer+10) = tt.hour%10+0x30;
	*(pBuffer+11) = tt.min/10+0x30;
	*(pBuffer+12) = tt.min%10+0x30;
	*(pBuffer+13) = tt.sec/10+0x30;
	*(pBuffer+14) = tt.sec%10+0x30;
	//后缀
	*(pBuffer+15) = '.';
	*(pBuffer+16) = 'w';
	*(pBuffer+17) = 'a';
	*(pBuffer+18) = 'v';
	//结尾字符
	*(pBuffer+19) = 0;
		
	return 20;
}

/**
  * @brief  获取录音状态 
  * @param  None
  * @retval None
  */
u8 GetRecordFlag(void)
{
  return RECORD_DEF.StartFlag;
}


/*********************************************************************
//函数名称	:GetRtcTimeSec
//功能		:获得已录音时长,当在定时录音时需要启用
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
u32  GetRecordTime(void)
{
        u32     time = 0;
        u32 timeSpace = 0;

        time = Timer_Val();
        timeSpace = (time-START_SPACE_TIME)/SYSTICK_1SECOND;
        if(timeSpace > 1)                              //1s保存一次间隔值
        {
          RECORD_DEF.RecordTimeSpace += timeSpace;    //保存秒
          START_SPACE_TIME = time;                    //已运行1秒,重新保存开始值
          RrcordWriteDef();
        } 
	return RECORD_DEF.RecordTimeSpace;
}

/*********************************************************************
//函数名称	:RrcordWriteDef
//功能		:保存录音配置文件
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void  RrcordWriteDef(void)
{
       FRAM_BufferWrite(FRAM_RECORD_FLAG_ADDR,(u8*)&RECORD_DEF,FRAM_RECORD_FLAG_LEN);
}


/*********************************************************************
//函数名称	:RrcordReadDef
//功能		:读取录音配置文件
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void  RrcordReadDef(void)
{
       FRAM_BufferRead((u8*)&RECORD_DEF,FRAM_RECORD_FLAG_LEN,FRAM_RECORD_FLAG_ADDR);
}
/*********************************************************************
//函数名称	:StartRecordTime
//功能		:启动定时录音
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void  StartRecordTime(u32 RecordTime)
{
    if(0 == RecordTime )                                          //循环录音
    {
        //RECORD_DEF.LoopFlag = 0;
        RECORD_DEF.RecordTime = 0;
        RECORD_DEF.RecordTimeSpace = 0; 
        ClrTimerTask(TIME_RECORD_TIMER);
    }else{                                                        //定时录音
        //RECORD_DEF.LoopFlag = 1;
        if((0 < RecordTime) && (RecordTime < 10))                 //设置时间最小为10s,如果不是强制为10s
        {
          RecordTime = 10;
        }
        RECORD_DEF.RecordTime = RecordTime;       
        START_SPACE_TIME =  Timer_Val();                          //获取录音间隔的开始时间。
        RECORD_DEF.RecordTimeSpace = 0; 
        SetTimerTask(TIME_RECORD_TIMER,RECORD_TIME_TASK_TIME);
    }
    RrcordWriteDef();                                            //重新保存录音时间
}
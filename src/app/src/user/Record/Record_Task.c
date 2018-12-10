/**
  ******************************************************************************
  * @file    Record_Task.c 
  * @author  
  * @version V1.0.2
  * @date    
  * @brief   ¼��������Ƚӿ�
  *          ʹ��˵��:
  *          1.ʹ��GetRecordFlag()��ȡ״̬,���Ϊ0����Ե���RecordTask_Start()����¼��.
  *
  *
  * ���¼�¼:
  *          2012-08-10:1.���¼�����ʱ��Ϊ������;
  *                     2.��ӱ����ý��ID;¼����ʼ�����ر���������;
  *                     3.¼���ر�ʱ�ر���˷磨ע����绰���Ʋ��죩��
  *                     4.ʹ���½ӿ�Io_WriteSpeakerConrtolBit()��������˷硣
  *          2012-08-13:1.�޸���¼�������ڵ��ȳ�ʼ���У�Ĭ�ϲ�������
  *                     2.��¼����ʼ�������У����������¼������RECORD_TIME_LENΪһ������ֵ��
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
uint32_t G711_LEN = 0;          //���ֵ=0xFFFFFFFF/8000=536870s/60=8947min/60=149h

const uint32_t RECORD_TIME_MAXLEN = 1*60;  //60*SYSTICK_1SECOND=1min * 5=5min
uint32_t RECORD_TIME_LEN = 0;             //��¼һ�����¼��ʱ��=0xFFFFFFFF/10 ,���ܵ��µ�������⣬��Ϊ�豸24Сʱ����һ��(������������),�����ڴ˴��������㲢���������

u32 START_SPACE_TIME = 0;        //�����������

RecordDef RECORD_DEF;            //��������

//��Ƶ�ɼ�������
uint32_t AUDIO_SAMPLE_LEN = DMA_MAX_SZE;
uint16_t AUDIO_SAMPLE[DMA_MAX_SZE];
uint8_t AUDIO_REC_G711[DMA_MAX_SZE/2];

FATFS wav_fs;
//char *audioG7File="1:ag7_c.wav";     //����G711�����ļ�

extern u16     DelayAckCommand;//��ʱӦ������,dxl,2013.5.29
extern u8      RecordSendFlag;//¼���ļ��ϴ���־,1Ϊ�ϴ�
extern u8	SdOnOffFlag;//SD�����߱�־��1���ߣ�0������,��sd����ʼ��ʧ��ʱ�ñ�־Ҳ����0
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  ʱ���������ִ�к���  
  * @note   ������ֵ�ܱ���500ms���ݣ����Ա��뱣֤500ms���ܵ���һ�Σ�����ᱻ�������ݸ���.
  * @param  None
  * @retval DISABLE�´β�ִ�У�ENABLE����ִ��
  */
FunctionalState RecordTask(void)
{
  if(0 == SdOnOffFlag)
  {
    return ENABLE;//���sd������,�����˴���ֱ���˳�,dxl,2014.6.18
  }
  if(Timer_Val() > RECORD_TIME_LEN )   //����������¼��ʱ��5���ӣ����Զ���������һ���ļ�
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
      Wav_Write_Data((void *)AUDIO_REC_G711, AUDIO_SAMPLE_LEN/2);   //����һ��д��4K,�պ�һ���飬(�������뻺�������ϵ���޷�����,Ҳ���ܲ���4KѰ��ַ�ᵼ��ʧ�ܡ�)
      G711_LEN += AUDIO_SAMPLE_LEN/2;
      REC_TO_G711_FLAG = 0;  
      break;
    default:
      break;
  }  
 
  return ENABLE;
}
/**
  * @brief  ¼����ʱ���� 
  * @note   ��������˶�ʱ¼��,���ڱ��������жϡ�
  * @param  None
  * @retval DISABLE�´β�ִ�У�ENABLE����ִ��
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
  * @brief  ¼�������ʼ��  
  * @param  None
  * @retval None
  */
void RecordTask_Init(void)
{
  I2S_Audio_Control_GPIO_Init();
  RrcordReadDef();
  ////LOG_PR("RecordTask_Init:StartFlag=[%D],RecordID=[%D]\r\n",RECORD_DEF.StartFlag,RECORD_DEF.RecordID);
  if(RECORD_DEF.RecordTimeMaxLen == 0)                           //�����ļ�¼�����ֵ����Ϊ0�����Ե�Ϊ0��ʹ��Ĭ������ֵ��
  {
    RECORD_DEF.RecordTimeMaxLen = RECORD_TIME_MAXLEN;            //����Ĭ�����ֵ
  }    
  if(1 ==  RECORD_DEF.StartFlag)
  {
    RECORD_DEF.StartFlag = 0;
    if(RECORD_DEF.RecordTimeSpace > 0)
    {
      if(RECORD_DEF.RecordTime > RECORD_DEF.RecordTimeSpace)
      {
         RECORD_DEF.RecordTime -= RECORD_DEF.RecordTimeSpace;      //����Ƕ�ʱ¼��,��Ҫ��ȥ��¼���ĳ���. 
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
    RECORD_TIME_LEN = 0xFFFFFFF;                                 //�������¼��״̬������һ������ֵ��������Task�н���Split().
  }
  if(RECORD_DEF.RecordID == 0)                                   //���ڶ�ý��ID����Ϊ0,���Ե�Ϊ0�Զ�����
  {
    RECORD_DEF.RecordID++;
  }
}
/**
  * @brief  ��ʼִ��¼������  
  * @param  RecordTime:¼��ʱ��,0-ѭ��¼��,����-¼��ʱ��(��λ��)
  * @param  AudioFreq:����Ƶ�ʣ�Ĭ��Ϊ8000,������ʱδ����
  * @param  maxTime:һ��¼���ļ��ʱ��,��λ��
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
      RrcordWriteDef();                                          //���±���¼��ʱ��
      Media_GetFileName(audioFileName,MEDIA_TYPE_WAV); 
      //LOG_PR("RecordTask_Start:audioFileName=[%s]\r\n",audioFileName);  
      Wav_Write_G711((char*)audioFileName,WAV_FILE_MODE);        //дwav�ļ�ͷ  
      Audio_Record_Init();
      SetTimerTask(TIME_RECORD,RECORD_TASK_TIME);                //�������õ�������
      G711_LEN = 0;
      REC_TO_G711_FLAG = 0;
      RECORD_TIME_LEN = Timer_Val() + (RECORD_DEF.RecordTimeMaxLen*SYSTICK_1SECOND);  //ʹ��ϵͳ��ʱ������ʱ��׼ȷ
  }
}

/**
  * @brief  �ر�����,ִֹͣ��  - �ⲿִ��
  * @param  None
  * @retval None
  */
void RecordTask_Close(void)
{
  //LOG_PR("RecordTask_Close:StartFlag=[%d]\r\n",RECORD_DEF.StartFlag);  
  if(1 == RECORD_DEF.StartFlag)
  {
    Audio_Record_Close();                                          //�ر�I2S�ӿ�
    Wav_Close(G711_LEN,RECORD_DEF.RecordID);                       //������ɹر�    
    //LOG_PR("RecordTask_Close:RecordID=[%d] \r\n",RECORD_DEF.RecordID);    
    Media_SaveLog(RECORD_DEF.RecordID++,MEDIA_TYPE_WAV);           //ý��ID����ɹ�     
    ClrTimerTask(TIME_RECORD);                                     //�ر�ʱ���������
    G711_LEN = 0;
    REC_TO_G711_FLAG = 0;  
    RECORD_TIME_LEN = 0;   
    RECORD_DEF.StartFlag = 0;
    RECORD_DEF.RecordTime = 0;
    RECORD_DEF.RecordTimeSpace = 0;
    RrcordWriteDef();                                              //�رձ�������
    if(1 == RecordSendFlag)
    {
    	DelayAckCommand  = 0x8804;//��ʱ����,�б��ͼ����,dxl
    	//������ʱ����
    	SetTimerTask(TIME_DELAY_TRIG, SYSTICK_0p1SECOND);//��ʱ5������ʱ��,ȷ����¼�Ƶ��ļ��ѱ����
    }
  }
}

/**
  * @brief  �ú�����ʱδʹ��,�ر�����,ִֹͣ��  -���Ⱥ����ڲ�ִ��
  * @param  None
  * @retval None
  */
FunctionalState Auto_RecordTask_Close(void)
{ 
  //LOG_PR("Auto_RecordTask_Close:StartFlag=[%d]\r\n",RECORD_DEF.StartFlag);    
  if(1 == RECORD_DEF.StartFlag)
  {
    Codec_AudioInterface_DeInit();                      //�ر�I2S�ӿ�
    Wav_Close(G711_LEN,RECORD_DEF.RecordID);            //������ɹر�    
    //LOG_PR("Auto_RecordTask_Close:RecordID=[%d]\r\n",RECORD_DEF.RecordID);      
    Media_SaveLog(RECORD_DEF.RecordID++,MEDIA_TYPE_WAV);            //ý��ID����ɹ�     
    G711_LEN = 0;
    REC_TO_G711_FLAG = 0;  
    RECORD_TIME_LEN = 0;   
    RECORD_DEF.StartFlag = 0;
    RECORD_DEF.RecordTime = 0;
    RECORD_DEF.RecordTimeSpace = 0;  
    RrcordWriteDef();               //�رձ�������
  }
  return DISABLE;                    //�ر�ʱ���������
}

/**
  * @brief  ���¼�����5����,����Ҫ�ָ�Ϊ5����һ���ļ� 
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
    
    Wav_Close(G711_LEN,RECORD_DEF.RecordID);                  //������ɹر�   
    //LOG_PR("RecordTask_Split:RecordID=[%d]\r\n",RECORD_DEF.RecordID);  
    Media_SaveLog(RECORD_DEF.RecordID++,MEDIA_TYPE_WAV);                    //ý��ID����ɹ�    
    RrcordWriteDef();                                         //�رձ�������
    G711_LEN = 0;
    RECORD_TIME_LEN = Timer_Val() + (RECORD_DEF.RecordTimeMaxLen*SYSTICK_1SECOND);
    
    Media_GetFileName(audioFileName,MEDIA_TYPE_WAV);
    //LOG_PR("RecordTask_Split:audioFileName=[%s]\r\n",audioFileName);   
    Wav_Write_G711((char*)audioFileName,WAV_FILE_MODE);        //дwav�ļ�ͷ   
  }
}

/*********************************************************************
//��������	:Camera_GetFileName
//����		:��ȡ�ļ���
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
*********************************************************************/
u8  Audio_GetFileName(u8 *pBuffer)
{
	TIME_T  tt;
	RTC_GetCurTime(&tt);
	//��Ŀ¼·��
	*pBuffer = '1';
	*(pBuffer+1) = ':';
	*(pBuffer+2) = '/';
	//�ļ���
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
	//��׺
	*(pBuffer+15) = '.';
	*(pBuffer+16) = 'w';
	*(pBuffer+17) = 'a';
	*(pBuffer+18) = 'v';
	//��β�ַ�
	*(pBuffer+19) = 0;
		
	return 20;
}

/**
  * @brief  ��ȡ¼��״̬ 
  * @param  None
  * @retval None
  */
u8 GetRecordFlag(void)
{
  return RECORD_DEF.StartFlag;
}


/*********************************************************************
//��������	:GetRtcTimeSec
//����		:�����¼��ʱ��,���ڶ�ʱ¼��ʱ��Ҫ����
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
*********************************************************************/
u32  GetRecordTime(void)
{
        u32     time = 0;
        u32 timeSpace = 0;

        time = Timer_Val();
        timeSpace = (time-START_SPACE_TIME)/SYSTICK_1SECOND;
        if(timeSpace > 1)                              //1s����һ�μ��ֵ
        {
          RECORD_DEF.RecordTimeSpace += timeSpace;    //������
          START_SPACE_TIME = time;                    //������1��,���±��濪ʼֵ
          RrcordWriteDef();
        } 
	return RECORD_DEF.RecordTimeSpace;
}

/*********************************************************************
//��������	:RrcordWriteDef
//����		:����¼�������ļ�
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
*********************************************************************/
void  RrcordWriteDef(void)
{
       FRAM_BufferWrite(FRAM_RECORD_FLAG_ADDR,(u8*)&RECORD_DEF,FRAM_RECORD_FLAG_LEN);
}


/*********************************************************************
//��������	:RrcordReadDef
//����		:��ȡ¼�������ļ�
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
*********************************************************************/
void  RrcordReadDef(void)
{
       FRAM_BufferRead((u8*)&RECORD_DEF,FRAM_RECORD_FLAG_LEN,FRAM_RECORD_FLAG_ADDR);
}
/*********************************************************************
//��������	:StartRecordTime
//����		:������ʱ¼��
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
*********************************************************************/
void  StartRecordTime(u32 RecordTime)
{
    if(0 == RecordTime )                                          //ѭ��¼��
    {
        //RECORD_DEF.LoopFlag = 0;
        RECORD_DEF.RecordTime = 0;
        RECORD_DEF.RecordTimeSpace = 0; 
        ClrTimerTask(TIME_RECORD_TIMER);
    }else{                                                        //��ʱ¼��
        //RECORD_DEF.LoopFlag = 1;
        if((0 < RecordTime) && (RecordTime < 10))                 //����ʱ����СΪ10s,�������ǿ��Ϊ10s
        {
          RecordTime = 10;
        }
        RECORD_DEF.RecordTime = RecordTime;       
        START_SPACE_TIME =  Timer_Val();                          //��ȡ¼������Ŀ�ʼʱ�䡣
        RECORD_DEF.RecordTimeSpace = 0; 
        SetTimerTask(TIME_RECORD_TIMER,RECORD_TIME_TASK_TIME);
    }
    RrcordWriteDef();                                            //���±���¼��ʱ��
}
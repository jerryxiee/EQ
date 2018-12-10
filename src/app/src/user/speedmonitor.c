/********************************************************************
//��Ȩ˵��	:Shenzhen E-EYE Co. Ltd., Copyright 2009- All Rights Reserved.
//�ļ�����	:speedmonitor.c		
//����		:ʵ���ٶȼ�ع���
//�汾��	:
//������	:dxl
//����ʱ��	:2010.4
//�޸���	:
//�޸�ʱ��	:
//�޸ļ�Ҫ˵��	:
//��ע		:
***********************************************************************/

//********************************ͷ�ļ�************************************
#include "include.h"
//********************************�Զ�����������****************************
//********************************�궨��************************************
#ifndef FRAM_SPEED_PLAY_ADDR
#define FRAM_SPEED_PLAY_ADDR    FRAM_TEXI_SERVICE_ADDR
#endif
/////////////////////////////////////////////
#define OVERSPEED_OVERHEAD_INFO_ID              0x11 //���ٸ�����ϢID
#define CAR_SPEED_LIMIT_OVERHEAD_INFO_ID        0x6A //��������ֵ������ϢID
#define CAR_SPEED_LIMIT_OVERHEAD_INFO_LEN       0x02;//��������ֵ������Ϣ����
///////////////////////////////
#define ONE_SPEED_MONITOR_DATA_LEN              (REGISTER_SPEED_STEP_LEN-5)//4���ֽڵ�ʱ��+1���ֽڵļ���,ÿ���ӵ���ʻ���ݳ���

#define ONCE_MIN_PLAY_TIME                      5//���β�����С���
#define ONCE_DEFAULT_PLAY_TIME                  10//���β���Ĭ��10��
//////////////////////////////////

//********************************ȫ�ֱ���**********************************

//********************************���ر���**********************************
typedef struct
{
    unsigned char speed;         //�ٶ�
    unsigned char status;        //״̬
}STRUNRECORD;

typedef struct
{
    STPLAY_ATTRIB stPlay;                   //���ٲ���
    STPLAY_ATTRIB stPrePlay;                //Ԥ������
    /////////////////////////    
    unsigned long startStopTime;            //��ʼͣ��ʱ��
    unsigned long OverstopTime;             //��ʱͣ��ʱ��  
    ////////////////////////////////////
    unsigned short EarlyAlarmDVal;          //Ԥ����ֵ
    unsigned short overSpeedCnt;            //����Ƿ��ټ����� 
    unsigned short speedDuration;           //���ٳ���ʱ������ֵ
    unsigned short preOverSpeedCnt;         //Ԥ���ٶȼ�����
    unsigned short prespeedDuration;        //Ԥ���ٳ���ʱ������ֵ
    //////////////////////////////
    unsigned long ReportFreqEvent;          //�ϱ�ʱ�������¼���
    unsigned short ReportInterval;          //�ϱ�ʱ����
    unsigned short ReportTimeCnt;           //�ϱ�ʱ�������
    ///////////////////////////////////////////////////////////////
    unsigned char curSpeedLimit;            //��ǰ������ֵ
    unsigned char curpreSpeedLimit;         //��ǰ����Ԥ����ֵ
    unsigned char curprespeedDuration;      //Ԥ���ٳ���ʱ������ֵ
    unsigned char SpeedType;                //�ٶ�����,0Ϊ����,1ΪGPS 
    unsigned char OverStopTimeFlag;         //��ʱͣ���ϱ���־
    unsigned char curSpeed;                 //��ǰ�ٶ�
    unsigned char preSpeed;                 //ǰһ���ٶ�
    unsigned char runStatusCnt;             //��ʻ״̬������
    unsigned char stopStatusCnt;            //ֹͣ״̬������
    unsigned char runStatus;                //1Ϊ��ʻ��0Ϊͣʻ
    unsigned char prerunStatus;             //ǰһ�ε���ʻ״̬
    unsigned char needReportflag;           //�Ƿ���Ҫ�ϱ�
    unsigned char recordCnt;                //��ʻ��¼�ݴ������
    STRUNRECORD stRecord[12];               //��ʻ��¼�ݴ�
}STSPEED_ATTRIB;

static STSPEED_ATTRIB s_stSpeedAttrib;

typedef struct
{
    STSPEEDCHECK stSpType[E_SPEED_TYPE_MAX]; //
    unsigned char speedLimit;               //��ǰ����ֵ
    unsigned char nightspeedLimit;          //��ǰ���ϳ���ֵ
    unsigned char valid;                    //��Ч��־
    unsigned char type;                     //��ǰʹ�õĳ�������
    unsigned char playflag;                 //��ǰ���ڲ���
}STSPEEDPARAM;

static STSPEEDPARAM s_stSpeedParam;

typedef struct
{
    STPLAY_ATTRIB stPlay;               //ҹ����ʻԤ������
    unsigned long earlyTime;            //ҹ��Ԥ��ʱ��
    unsigned long startVal;             //ҹ�俪ʼʱ��
    unsigned long endVal;               //ҹ�����ʱ��
    unsigned short control;             //ҹ����ʻʱ�������
    unsigned char percent;              //ҹ���ٶȰٷֱ�
    unsigned char needChecknight;       //��Ҫ������ϳ�ʱ
}STSPEED_NIGHT;
static STSPEED_NIGHT s_stSpNight;

typedef struct
{
    STPLAY_ATTRIB stPlay;               //��ֹ��ʻ����
    STPLAY_ATTRIB stPrePlay;            //��ֹ��ʻԤ������
    unsigned long earlyTime;            //��ֹ��ʻԤ��ʱ��
    unsigned long startVal;             //��ֹ��ʼʱ��(��Կ�ʼʱ��)
    unsigned long endVal;               //��ֹ����ʱ��
    unsigned short control;             //��ֹ��ʻʱ�������
    unsigned short runDuration;         //��ֹ��ʻ������ʻ״̬�ĳ���ʱ��
    unsigned short runStatusCnt;        //��ֹ��ʻ��ʻ״̬������
    unsigned short stopStatusCnt;       //��ֹ��ʻֹͣ״̬������
    unsigned char runStatus;            //��ֹ��ʻ1Ϊ��ʻ��0Ϊͣʻ
    unsigned char runSpeed;             //��ֹ��ʻ������ʻ״̬���ٶ� 
    unsigned char CheckVaild;           //�Ƿ���Ҫ���
}STSPEED_FORBIDTIME;
static STSPEED_FORBIDTIME s_stSpForbid;
typedef struct
{
    unsigned char playSpeed;          //�����ٶ�
    unsigned char atnightflag;        //0��,1����,2ҹ��
    unsigned char reserve[6];
}STSPEED_PLAY;
static STSPEED_PLAY s_stSpeedPlay;

////////////////////////////////
typedef struct
{
    unsigned short head;                //ͷ��־
    unsigned char saveflag;             //�洢��־
    unsigned char count;                //������
    unsigned long startTime;            //��ʼʱ��
    STRUNRECORD stRecord[60];           //�ٶȡ�״̬
}STSPEEDSAVERECORD;
static STSPEEDSAVERECORD s_stSpSaveRecord;

#define  STSPEEDSAVERECORD_SIZE sizeof(STSPEEDSAVERECORD)
///////////////////////////
#define RUN_CHAGE_STATUS_TIME 10//�ı�״̬������ʱ��10s
///////////////////////////////////
typedef enum
{
    CAR_STATUS_EMPTY,   //��
    CAR_STATUS_STOP,    //ͣʻ
    CAR_STATUS_RUN,     //��ʻ
    CAR_STATUS_MAX
}CAR_STATUS;//������ʻ״̬

enum 
{
    SPEED_TIMER_TASK,                       //
    SPEED_TIMER_CHECK,						// 
    SPEED_TIMER_PLAY,						// 
    SPEED_TIMER_TEST,						// 
    SPEED_TIMER_FORBID,						// 
    SPEED_TIMERS_MAX
}STSPEED_TIME;
static LZM_TIMER s_stSpeedTimer[SPEED_TIMERS_MAX];
//********************************��������**********************************
void SpeedMonitor_ClearPlayFlag(void);
void SpeedMonitor_UpdateCurSpeedLimitParam(void);
//********************************��������***********************************
/*******************************************************************************
** ��������: SpeedMonitor_GetCurOverSpeedSubjoinInfo
** ��������: ȡ�õ�ǰ���ٸ�����Ϣ��(����������ϢID�����ȡ�������Ϣ��) :��������ϢIDΪ0x11����������
** ��ڲ���: ��Ҫ���泬�ٸ�����Ϣ�����׵�ַ
** ���ڲ���: 
** ���ز���: ���ٸ�����Ϣ�����ܳ���,0:��ʾδ����,
*******************************************************************************/
unsigned char SpeedMonitor_GetCurOverSpeedSubjoinInfo(unsigned char *data)
{
    unsigned char buffer[20]={0};
    unsigned char len;
    if(0==Io_ReadAlarmBit(ALARM_BIT_OVER_SPEED))return 0;
    len = 0;
    buffer[len++] = OVERSPEED_OVERHEAD_INFO_ID;
    if(E_SPEED_TYPE_GENERAL==s_stSpeedParam.type)
    {
        buffer[len++] = 1;//����
        buffer[len++] = 0;//λ������
    }
    else
    {
        buffer[len++] = 5;//����
        buffer[len++] = s_stSpeedParam.type;//λ������
        Public_ConvertLongToBuffer(s_stSpeedParam.stSpType[s_stSpeedParam.type].ID,&buffer[len]);        
        len += 4;
    }
    ///////////////
    memcpy(data,buffer,len);
    /////////////////
    return len;
}
/*******************************************************************************
** ��������: SpeedMonitor_GetCurCarSpeedLimitSubjoinInfo
** ��������: ȡ�ó�������ֵ������Ϣ��(����������ϢID�����ȡ�������Ϣ��) (����Ҫ�� ������ϢIDΪ0x6A����������)
** ��ڲ���: ��Ҫ���泵������ֵ������Ϣ�����׵�ַ
** ���ڲ���: 
** ���ز���: ��������ֵ������Ϣ�����ܳ���,
*******************************************************************************/
unsigned char SpeedMonitor_GetCurCarSpeedLimitSubjoinInfo(unsigned char *data)
{
   
    return 0;
}
/*************************************************************
** ��������: SpeedMonitor_GetLastStopTime
** ��������: 
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���:
** ����ģ��:
*************************************************************/
unsigned long SpeedMonitor_GetLastStopTime(void)
{
    unsigned char buffer[10];
    TIME_T stTime;
    if(Public_ReadDataFromFRAM(FRAM_STOPING_TIME_ADDR,buffer,6))
    {
        if(Public_ConvertBCDToTime(&stTime,buffer))
        {
            return ConverseGmtime(&stTime);
        }
    }
    return 0;
}
/*************************************************************
** ��������: SpeedMonitor_OverSpeedPreAlarmVoice
** ��������: ����Ԥ��������
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���:
** ����ģ��:
*************************************************************/
void SpeedMonitor_OverSpeedPreAlarmVoice(void)
{
    if(!Public_PlayE2ParamAlarmVoice(E2_OVERSPEED_PREALARM_VOICE_ID))
    {
        Public_PlayTTSVoiceAlarmStr("����Ƴ���,������ʻ");
    }
}
/*************************************************************
** ��������: SpeedMonitor_OverSpeedAlarmVoice
** ��������: ������������
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���:
** ����ģ��:
*************************************************************/
void SpeedMonitor_OverSpeedAlarmVoice(void)
{
    Public_PlayTTSVoiceAlarmStr((unsigned char *)s_stSpeedParam.stSpType[s_stSpeedParam.type].PlayVoice);
}
/*************************************************************
** ��������: SpeedMonitor_NightPreAlarmVoice
** ��������: ҹ��Ԥ������
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���:
** ����ģ��:
*************************************************************/
void SpeedMonitor_NightPreAlarmVoice(void)
{
    if(!Public_PlayE2ParamAlarmVoice(E2_NIGHT_DRIVE_PREALARM_VOICE_ID))
    {
        Public_PlayTTSVoiceAlarmStr("��������ҹ��ʱ��");
    }
}
/*************************************************************
** ��������: SpeedMonitor_ForbidPreAlarmVoice
** ��������: ҹ�����Ԥ������
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���:
** ����ģ��:
*************************************************************/
void SpeedMonitor_ForbidPreAlarmVoice(void)
{
    if(!Public_PlayE2ParamAlarmVoice(E2_FORBID_DRIVE_PREALARM_VOICE_ID))
    {
        Public_PlayTTSVoiceAlarmStr("�밴�涨ʱ��ͣ����Ϣ");
    }
}
/*************************************************************
** ��������: SpeedMonitor_ForbidAlarmVoice
** ��������: ҹ���������
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���:
** ����ģ��:
*************************************************************/
void SpeedMonitor_ForbidAlarmVoice(void)
{
    if(!Public_PlayE2ParamAlarmVoice(E2_FORBID_DRIVE_ALARM_VOICE_ID))
    {
        Public_PlayTTSVoiceAlarmStr("�밴�涨ʱ��ͣ����Ϣ");
    }    
}
/*************************************************************
** ��������: SpeedMonitor_UpdateNightSpeedLimit
** ��������: 
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
void SpeedMonitor_UpdateNightSpeedLimit(void)
{
    if(!SpeedMonitor_GetNeedCheckNightFlag())return;
    if(!SpeedMonitor_GetCurAtNightFlag())return;    
    s_stSpeedParam.nightspeedLimit = (unsigned char)(s_stSpeedParam.speedLimit*s_stSpNight.percent*0.01+0.5);
    s_stSpeedAttrib.curSpeedLimit = s_stSpeedParam.nightspeedLimit;
}
/*************************************************************
** ��������: SpeedMonitor_GetCurAtNightFlag
** ��������: ȡ���Ƿ�Ϊҹ��ʱ���ڱ�־
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 1:Ϊҹ��ʱ���;0:��ҹ��ʱ���
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
unsigned char SpeedMonitor_GetCurAtNightFlag(void)
{
    return (2 == s_stSpeedPlay.atnightflag)?1:0;
}
/*************************************************************
** ��������: SpeedMonitor_GetNeedCheckNightFlag
** ��������: ȡ���Ƿ���Ҫ�ж�ҹ��ʱ�α�־
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 1:��Ҫ�ж�;0:����Ҫ�ж�
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
unsigned char SpeedMonitor_GetNeedCheckNightFlag(void)
{
    return (s_stSpNight.needChecknight)?1:0;
}
/*************************************************************
** ��������: SpeedMonitor_ReadSpeedPlay
** ��������: 
** ��ڲ���: 
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void SpeedMonitor_ReadSpeedPlay(void) 
{
    unsigned long timeVal,stopTime;    
    if(Public_ReadDataFromFRAM(FRAM_SPEED_PLAY_ADDR,(unsigned char *)&s_stSpeedPlay, sizeof(s_stSpeedPlay))) 
    {
        timeVal=RTC_GetCounter();
        stopTime = s_stSpeedAttrib.startStopTime;
        if(!SpeedMonitor_GetNeedCheckNightFlag()||(timeVal>stopTime+DAY_SECOND&&stopTime)
            ||(stopTime>timeVal+DAY_SECOND&&stopTime))
        {
            s_stSpeedPlay.atnightflag = 0;
        }
    }
    else
    {
        memset(&s_stSpeedPlay,0,sizeof(s_stSpeedPlay));
    }
}
/*************************************************************
** ��������: SpeedMonitor_SaveStopStartTime
** ��������: 
** ��ڲ���: 
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void SpeedMonitor_SaveSpeedPlay(void) 
{
    Public_WriteDataToFRAM(FRAM_SPEED_PLAY_ADDR,(unsigned char *)&s_stSpeedPlay, sizeof(s_stSpeedPlay));
}
/*************************************************************
** ��������: SpeedMonitor_CheckPlayAndSaveNightFlag
** ��������: �жϲ��������
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
void SpeedMonitor_CheckPlayAndSaveNightFlag(unsigned char flag)
{
    if(flag==s_stSpeedPlay.atnightflag)return; 
    ////////////////////////////    
    if(SpeedMonitor_GetCurRunStatus()&&s_stSpeedPlay.atnightflag)
    {
        if(2==flag)//ҹ��
        {
            Public_PlayTTSVoiceAlarmStr("ҹ����ʻ,��ע����Ƴ���");
        }
        else //����
        {
            Public_PlayTTSVoiceAlarmStr("������ʻ,��ע����Ƴ���"); 
        }
        s_stSpeedParam.playflag = 2;
        LZM_PublicSetOnceTimer(&s_stSpeedTimer[SPEED_TIMER_TEST],PUBLICSECS(8),SpeedMonitor_ClearPlayFlag);
    }
    ///////////////////////////////
    s_stSpeedPlay.atnightflag = flag;
    SpeedMonitor_SaveSpeedPlay();
    SpeedMonitor_UpdateCurSpeedLimitParam();    
}
/*************************************************************
** ��������: SpeedMonitor_CheckIsAtNightFlag
** ��������: �ж��Ƿ�����ҹ��ʱ���
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
void SpeedMonitor_CheckIsAtNightFlag(void)
{
    unsigned char flag;
    if(!SpeedMonitor_GetNeedCheckNightFlag())return;  
    ////////ҹ��ʱ���//////////
    if(Public_CheckCurTimeIsInTimeRange(s_stSpNight.control,s_stSpNight.startVal,s_stSpNight.endVal))
    {
        SpeedMonitor_CheckPlayAndSaveNightFlag(2);
    }
    else//��ҹ��ʱ���
    {
        flag = 0;
        SpeedMonitor_CheckPlayAndSaveNightFlag(1);
        /////////ҹ��Ԥ��ʱ���//////////////        
        if(Public_CheckCurTimeIsInEarlyAlarmTimeRange(s_stSpNight.control,s_stSpNight.earlyTime,s_stSpNight.startVal,s_stSpNight.endVal))
        {
            flag =1;
        }
        /////////////////////////////////
        if(SpeedMonitor_CheckPlayFlag(flag,&s_stSpNight.stPlay))
        {
            SpeedMonitor_NightPreAlarmVoice();
        }
    }    
}
/*************************************************************
** ��������: SpeedMonitor_SaveStopStartTime
** ��������: ����ͣ����ʼʱ��
** ��ڲ���: 
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void SpeedMonitor_SaveStopStartTime(void) 
{
    Public_WriteDataToFRAM(FRAM_STOP_TIME_ADDR,(unsigned char *)&s_stSpeedAttrib.startStopTime, 4);
}
/*************************************************************
** ��������: SpeedMonitor_ReadStopStartTime
** ��������: ��ȡͣ����ʼʱ��
** ��ڲ���: 
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void SpeedMonitor_ReadStopStartTime(void) 
{
    unsigned long val;
    if(Public_ReadDataFromFRAM(FRAM_STOP_TIME_ADDR,(unsigned char *)&val, 4)) 
    {
        s_stSpeedAttrib.startStopTime = val;
    }
    val = SpeedMonitor_GetLastStopTime();
    ////////////////////////////////////////////
    if(0==s_stSpeedAttrib.startStopTime||(s_stSpeedAttrib.startStopTime>val&&val))
    {
        s_stSpeedAttrib.startStopTime = val;
        SpeedMonitor_SaveStopStartTime();
    }
}
/*************************************************************
** ��������: SpeedMonitor_GetCurRunStatus
** ��������: ��ȡ��ǰ������״̬(��ʻ��ֹͣ)
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 1Ϊ��ʻ��0Ϊֹͣ
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
unsigned char SpeedMonitor_GetCurRunStatus(void)
{
    return(CAR_STATUS_RUN==s_stSpeedAttrib.runStatus)?1:0;
}
/*************************************************************
** ��������: SpeedMonitor_CheckRunStatus
** ��������: �жϵ�ǰ�Ǵ�����ʻ״̬����ͣʻ״̬(1s����һ��)
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
void SpeedMonitor_CheckRunStatus(unsigned char CurrentSpeed)
{
    //unsigned char ACC;	
    //ACC = Io_ReadStatusBit(STATUS_BIT_ACC);	
    if(CurrentSpeed > 0)//
    {
        s_stSpeedAttrib.stopStatusCnt = 0;//ֹͣ״̬��������0
        if(s_stSpeedAttrib.runStatusCnt < RUN_CHAGE_STATUS_TIME)//�Ƿ������10s,����ʱ,���ı䵱ǰ״̬
        {
            s_stSpeedAttrib.runStatusCnt++;//��ʻ״̬������++
        }
        else//�Ѿ�����10s,����Ϊ��ʻ״̬
        {
            s_stSpeedAttrib.runStatus = CAR_STATUS_RUN;
        }
    }
    else
    {
        s_stSpeedAttrib.runStatusCnt = 0;//��ʻ״̬��������0
        if(s_stSpeedAttrib.stopStatusCnt < RUN_CHAGE_STATUS_TIME)//�Ƿ������10s,����ʱ,���ı䵱ǰ״̬
        {
            s_stSpeedAttrib.stopStatusCnt++;//ֹͣ״̬������++
        }
        else//�Ѿ�����10s,����Ϊֹͣ״̬
        {
            s_stSpeedAttrib.runStatus = CAR_STATUS_STOP;
        }
    }
}
/*************************************************************
** ��������: SpeedMonitor_CheckStopRunStatus
** ��������: �жϵ�ǰ�Ǵ�����ʻ״̬����ͣʻ״̬(1s����һ��)
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
void SpeedMonitor_CheckForbidRunStatus(unsigned char CurrentSpeed)
{
    unsigned char ACC;
    ///////////////////////////
    ACC = Io_ReadStatusBit(STATUS_BIT_ACC);
    if((CurrentSpeed >=s_stSpForbid.runSpeed)&&(1 == ACC))//
    {
        s_stSpForbid.stopStatusCnt = 0;//ֹͣ״̬��������0
        if(s_stSpForbid.runStatusCnt < s_stSpForbid.runDuration)//�Ƿ������10s,����ʱ,���ı䵱ǰ״̬
        {
            s_stSpForbid.runStatusCnt++;//��ʻ״̬������++
        }
        else//�Ѿ�����10s,����Ϊ��ʻ״̬
        {
            s_stSpForbid.runStatus = CAR_STATUS_RUN;
        }
    }
    else//ֻҪ�ٶ�һ�����趨ֵ,����ͣ��???
    {
        s_stSpForbid.runStatusCnt = 0;//��ʻ״̬��������0
        s_stSpForbid.runStatus = CAR_STATUS_STOP;
    }
}
/*************************************************************
** ��������: SpeedMonitor_ClearRunRecordData
** ��������: 
** ��ڲ���: 
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void SpeedMonitor_ClearRunRecordData(void)
{
    s_stSpSaveRecord.count = 0;
    s_stSpSaveRecord.startTime = 0;
    memset(&s_stSpSaveRecord.stRecord,0xff,120);
}
/*************************************************************
** ��������: SpeedMonitor_SaveRecordDataToFlash
** ��������: 
** ��ڲ���: 
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void SpeedMonitor_SaveRunRecordDataToFRAM(void)
{
    #if (ONE_SPEED_MONITOR_DATA_LEN != 6)
    s_stSpSaveRecord.head = 0xacbd;
    s_stSpSaveRecord.saveflag = 0x55;
    Public_WriteDataToFRAM(FRAM_SPEED_RUN_RECORD_ADDR,(unsigned char *)&s_stSpSaveRecord,STSPEEDSAVERECORD_SIZE);
    #endif
}
/*************************************************************
** ��������: SpeedMonitor_SaveRecordDataToFlash
** ��������: 
** ��ڲ���: 
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void SpeedMonitor_ReadRunRecordData(void)
{
    #if (ONE_SPEED_MONITOR_DATA_LEN != 6)
    unsigned char len;
    len=Public_ReadDataFromFRAM(FRAM_SPEED_RUN_RECORD_ADDR,(unsigned char *)&s_stSpSaveRecord,STSPEEDSAVERECORD_SIZE);
    if((len != STSPEEDSAVERECORD_SIZE)
      ||(s_stSpSaveRecord.head != 0xacbd)
      ||(s_stSpSaveRecord.saveflag!=0x55))
    {
        SpeedMonitor_ClearRunRecordData();
    }
    #endif
}
/*************************************************************
** ��������: SpeedMonitor_SaveRecordDataToFlash
** ��������: 
** ��ڲ���: 
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void SpeedMonitor_SaveRecordDataToFlash(void)
{
    #if (ONE_SPEED_MONITOR_DATA_LEN != 6)
    TIME_T stTime;
    unsigned char buffer[ONE_SPEED_MONITOR_DATA_LEN+1]={0};
    Gmtime(&stTime,s_stSpSaveRecord.startTime);
    Public_ConvertTimeToBCDEx(stTime,buffer);
    memcpy(&buffer[6],s_stSpSaveRecord.stRecord,120);
    if(ERROR ==Register_Write2(REGISTER_TYPE_SPEED,buffer,ONE_SPEED_MONITOR_DATA_LEN,s_stSpSaveRecord.startTime))
		{
		//flash������
        Register_EraseOneArea(REGISTER_TYPE_SPEED);
    }
    SpeedMonitor_ClearRunRecordData();
    #endif
}
/*************************************************************
** ��������: SpeedMonitor_SaveOneRecordData
** ��������: 
** ��ڲ���: 
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void SpeedMonitor_SaveOneRecordData(STRUNRECORD stRecord,u32 curTime)
{
    #if (ONE_SPEED_MONITOR_DATA_LEN != 6)    
    unsigned char sec;
    sec = curTime%60;
    if(0==s_stSpSaveRecord.startTime)//
    {
        s_stSpSaveRecord.startTime = curTime-sec;
    }
    else
    if((curTime<s_stSpSaveRecord.startTime)||(curTime>=(s_stSpSaveRecord.startTime+60)))
    {
        SpeedMonitor_SaveRecordDataToFlash();
        s_stSpSaveRecord.startTime = curTime-sec;
    }
    /////////////////////
    s_stSpSaveRecord.stRecord[sec] = stRecord;
    s_stSpSaveRecord.count++;
    //////////////////////////////////
    SpeedMonitor_SaveRunRecordDataToFRAM();
    #else
    unsigned char buffer[ONE_SPEED_MONITOR_DATA_LEN]={0xff};
    buffer[0] = stRecord.speed;
    buffer[1] = stRecord.status;
    Register_Write2(REGISTER_TYPE_SPEED,buffer,ONE_SPEED_MONITOR_DATA_LEN,curTime);  
    #endif
}
/*************************************************************
** ��������: SpeedMonitor_SaveRunRecordData
** ��������: ������ʻ��¼(��1s��ʱ����������¼���洢������ʻ״̬���ݡ�����ʻ״̬����Ϊ����������ʻ�����е�ʵʱʱ�䡢ÿ���Ӽ���ڶ�Ӧ��ƽ���ٶ��Լ���Ӧʱ���״̬�ź�)
** ��ڲ���: 
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void SpeedMonitor_SavePackRecordDataToFRAM(unsigned char RecordNum, u32 curTime)
{
    unsigned char i;
    unsigned long timeVal;
    timeVal =curTime-RecordNum;
    for(i = 0; i<RecordNum; i++)
    {
        SpeedMonitor_SaveOneRecordData(s_stSpeedAttrib.stRecord[i],timeVal+i);
    }
}
/*************************************************************
** ��������: SpeedMonitor_SaveRecordDataToFlash
** ��������: 
** ��ڲ���: 
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void SpeedMonitor_CheckRecordDataToSaveFlash(void)
{
    #if (ONE_SPEED_MONITOR_DATA_LEN != 6)
    unsigned long curTime;
    if(0==s_stSpSaveRecord.count||0 == s_stSpSaveRecord.startTime)return;
    curTime = RTC_GetCounter();
    if(s_stSpSaveRecord.startTime<curTime&&curTime<s_stSpSaveRecord.startTime+60)return;//һ����֮�ڲ�����
    if(s_stSpeedAttrib.recordCnt)return;//��û���ݴ��
    /////////////////////////////////
    SpeedMonitor_SaveRecordDataToFlash();
    /////////////////////////////////
    SpeedMonitor_SaveRunRecordDataToFRAM();
    #endif
}
/*************************************************************
** ��������: SpeedMonitor_SaveRunRecordData
** ��������: ������ʻ��¼(��1s��ʱ����������¼���洢������ʻ״̬���ݡ�����ʻ״̬����Ϊ����������ʻ�����е�ʵʱʱ�䡢ÿ���Ӽ���ڶ�Ӧ��ƽ���ٶ��Լ���Ӧʱ���״̬�ź�)
** ��ڲ���: 
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void SpeedMonitor_SaveRunRecordData(unsigned char CurrentSpeed)
{
    unsigned char saveflag;
    unsigned char temp;
    STRUNRECORD stRunRecord;
    saveflag = 0;    
    ///////////////////////
    stRunRecord.speed = CurrentSpeed;
    //////////////////////////
    temp = (Io_ReadExtCarStatus() & 0xff);
    temp <<= 3;//�ѵ�λ�����ƶ�3λ
    stRunRecord.status = temp;
    //////����Զ���//////add by joneming//////
    /////Acc///////////
    if(Io_ReadStatusBit(STATUS_BIT_ACC))
    {
        PUBLIC_SETBIT(stRunRecord.status, 0);
    }
    /////////����//////////////
    if(Io_ReadStatusBit(STATUS_BIT_DOOR1))
    {
        PUBLIC_SETBIT(stRunRecord.status, 1);
    }
    /////////������///////////////
    if(Io_ReadExtCarStatusBit(CAR_STATUS_BIT_HORN))
    {
        PUBLIC_SETBIT(stRunRecord.status, 2);
    }    
    ////////////////////////////
    if((CAR_STATUS_STOP==s_stSpeedAttrib.prerunStatus)&&(CAR_STATUS_RUN==s_stSpeedAttrib.runStatus))//��ͣʻ����ʻ
    {
        ///////��Ҫת��/////////
        if(s_stSpeedAttrib.recordCnt)//
        {
            SpeedMonitor_SavePackRecordDataToFRAM(s_stSpeedAttrib.recordCnt,RTC_GetCounter());
        }
        s_stSpeedAttrib.recordCnt = 0;
        saveflag = 1;
    }
    else
    if((CAR_STATUS_RUN==s_stSpeedAttrib.prerunStatus)&&(CAR_STATUS_STOP==s_stSpeedAttrib.runStatus))//����ʻ��ͣʻ
    {
        ///////����Ҫ����////////////
        s_stSpeedAttrib.recordCnt = 0;
    }
    else
    if((CAR_STATUS_STOP==s_stSpeedAttrib.prerunStatus)&&(CAR_STATUS_STOP==s_stSpeedAttrib.runStatus))//ͣʻ״̬
    {
        if(0 == CurrentSpeed)//
        {
            s_stSpeedAttrib.recordCnt = 0;
        }
        else
        if(CurrentSpeed)////��Ҫ�ݴ�
        {
            s_stSpeedAttrib.stRecord[s_stSpeedAttrib.recordCnt] = stRunRecord;
            s_stSpeedAttrib.recordCnt++;
        }
    }
    else
    if((CAR_STATUS_RUN==s_stSpeedAttrib.prerunStatus)&&(CAR_STATUS_RUN==s_stSpeedAttrib.runStatus))//��ʻ״̬
    {
        if(0==CurrentSpeed&&0==s_stSpeedAttrib.preSpeed)////��Ҫ�ݴ�
        {
            s_stSpeedAttrib.stRecord[s_stSpeedAttrib.recordCnt] = stRunRecord;
            s_stSpeedAttrib.recordCnt++;
        }
        else
        if(CurrentSpeed)//���ݴ�Ķ������浽flash
        {
            if(s_stSpeedAttrib.recordCnt)///��Ҫת��//////
            {
                SpeedMonitor_SavePackRecordDataToFRAM(s_stSpeedAttrib.recordCnt,RTC_GetCounter());
            }
            s_stSpeedAttrib.recordCnt =0;
            ///////////////////////////
            saveflag = 1;
        }
        else//��һ���ٶ�Ϊ0����Ҫ����
        if(0==CurrentSpeed&&s_stSpeedAttrib.preSpeed)////
        {
            s_stSpeedAttrib.recordCnt = 0;
            saveflag = 1;
        }
    }
    ///////////////////////////
    if(saveflag)//
    {
        SpeedMonitor_SaveOneRecordData(stRunRecord,RTC_GetCounter());
    }
    else
    {
        SpeedMonitor_CheckRecordDataToSaveFlash();
    }

    s_stSpeedAttrib.preSpeed = CurrentSpeed;
    ////////////////////////////
    s_stSpeedAttrib.prerunStatus=s_stSpeedAttrib.runStatus;
}
/*************************************************************
** ��������: SpeedMonitor_ReadSpeedRecordDataFromFram
** ��������: ���������ȡָ��ʱ����ٶȼ�¼
** ��ڲ���: ��ʼʱ��StartTime,����ʱ��EndTime
** ���ڲ���: pBuffer�������ݵ��׵�ַ
** ���ز���: pBuffer�ֽڳ���
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
unsigned short SpeedMonitor_ReadSpeedRecordDataFromFram(unsigned char *pBuffer,TIME_T StartTime,TIME_T EndTime)
{
    #if (ONE_SPEED_MONITOR_DATA_LEN != 6)
    unsigned long startVal,endVal;  
    TIME_T stTime;
    if(0==s_stSpSaveRecord.count||0 == s_stSpSaveRecord.startTime)return 0;
    startVal = ConverseGmtime(&StartTime);
    endVal = ConverseGmtime(&EndTime);    
    if(0==Public_CheckTimeIsInRange(s_stSpSaveRecord.startTime,startVal,endVal))return 0;//���ڷ�Χ��
    Gmtime(&stTime,s_stSpSaveRecord.startTime);
    Public_ConvertTimeToBCDEx(stTime,pBuffer);
    memcpy(&pBuffer[6],s_stSpSaveRecord.stRecord,120);  
    return 126;
    #else
    return 0;
    #endif
}
/*************************************************************
** ��������: SpeedMonitor_ReadSpeedRecordData
** ��������: ��ȡָ��ʱ��Ρ�ָ��������ݿ��ٶȼ�¼����
** ��ڲ���: ��ʼʱ��StartTime,����ʱ��EndTime,MaxBlockָ��������ݿ�
** ���ڲ���: pBuffer�������ݵ��׵�ַ
** ���ز���: pBuffer�ֽڳ���
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
unsigned short SpeedMonitor_ReadSpeedRecordData(u8 *pBuffer, TIME_T StartTime, TIME_T EndTime, u16 MaxBlock)
{
    unsigned char *p;
    unsigned short len;
    unsigned short block;
    len=0;
    p=pBuffer;
    block = MaxBlock;
    len=SpeedMonitor_ReadSpeedRecordDataFromFram(p,StartTime,EndTime);
    if(len)
    {
        p += len;
        if(block)block--;
    }
    /////////////////
    len += Register_Read(REGISTER_TYPE_SPEED,p,StartTime,EndTime,block);
    return len;
}
/*********************************************************************
//��������	:SpeedMonitor_IsOverSpeed
//����		:�жϵ�ǰ�Ƿ���
//����		:CurrentSpeed��ǰ�ٶ�,MaxSpeed����ֵ,UpDelaySec:��������
//���		:
//����		:1:�ѳ��٣�0:δ����
//��ע		:
*********************************************************************/
unsigned char SpeedMonitor_IsOverSpeed(u8 CurrentSpeed, u8 MaxSpeed, u32 UpDelaySec)
{
    unsigned char ACC = 0;
    //��ACC״̬
    ACC = Io_ReadStatusBit(STATUS_BIT_ACC);
    if(CurrentSpeed >= MaxSpeed&&SpeedMonitor_GetCurRunStatus())
    {
        //�ٶȼ�����1
        if(0==s_stSpeedAttrib.overSpeedCnt&&UpDelaySec>1)
        {
            //SetEvTask(EV_REPORT);
            Report_UploadPositionInfo(CHANNEL_DATA_1);
            Report_UploadPositionInfo(CHANNEL_DATA_2);
        }
        //////////////////
        s_stSpeedAttrib.overSpeedCnt++;
        if((s_stSpeedAttrib.overSpeedCnt >= UpDelaySec)&&(0 != ACC))
        {
            s_stSpeedAttrib.overSpeedCnt = UpDelaySec;
            return 1;	//�ѳ���
        }
    }
    else
    {
        //�ٶȼ�����0
        if(s_stSpeedAttrib.overSpeedCnt&&CurrentSpeed < MaxSpeed)
        {
           
        }
        s_stSpeedAttrib.overSpeedCnt = 0;
    }
    return 0;	//û����
}
/*********************************************************************
//��������	:SpeedMonitor_IsPreAlarm
//����		:�жϵ�ǰ�Ƿ�Ԥ��
//����		:CurrentSpeed��ǰ�ٶ�,MaxSpeed����ֵ,UpDelaySec:��������
//���		:
//����		:1:�ѱ�����0:δ����
//��ע		:
*********************************************************************/
unsigned char SpeedMonitor_IsPreAlarm(u8 CurrentSpeed, u8 MaxSpeed, u32 UpDelaySec)
{
    unsigned char ACC = 0;
    ///////////////////
    if((0==MaxSpeed)||(MaxSpeed>=s_stSpeedAttrib.curSpeedLimit))return 0;	//û���
    //��ACC״̬
    ACC = Io_ReadStatusBit(STATUS_BIT_ACC);
    /////////////////
    if(CurrentSpeed >= MaxSpeed&&SpeedMonitor_GetCurRunStatus())
    {
        //�ٶȼ�����1
        s_stSpeedAttrib.preOverSpeedCnt++;
        if((s_stSpeedAttrib.preOverSpeedCnt >= UpDelaySec)&&(0 != ACC))
        {
            s_stSpeedAttrib.preOverSpeedCnt = UpDelaySec;
            return 1;	//�ѳ���
        }
    }
    else
    {
        //�ٶȼ�����0
        s_stSpeedAttrib.preOverSpeedCnt = 0;
    }
    ////////////////////
    return 0;	//û����
}
/*********************************************************************
//��������	:SpeedMonitor_PreAlarm
//����		:����Ԥ������
//����		:CurrentSpeed��ǰ�ٶ�,MaxSpeed����ֵ
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
*********************************************************************/
void SpeedMonitor_PreAlarm(u8 CurrentSpeed, u8 MaxSpeed)
{
    unsigned char flag; 
    ////////////////////////////
    if((1 == SpeedMonitor_IsPreAlarm(CurrentSpeed,MaxSpeed,s_stSpeedAttrib.curprespeedDuration))
     &&(0 == Io_ReadAlarmBit(ALARM_BIT_OVER_SPEED)))
    {
        flag =1;
        Io_WriteAlarmBit(ALARM_BIT_OVER_SPEED_PRE_ALARM,SET);
    }
    else
    {
        //���Ԥ����־
        flag = 0;
        Io_WriteAlarmBit(ALARM_BIT_OVER_SPEED_PRE_ALARM,RESET);
    }
    ////////////////////////////////////
    if(SpeedMonitor_CheckPlayFlag(flag,&s_stSpeedAttrib.stPrePlay))
    {
        if(2 != s_stSpeedParam.playflag&&3 != s_stSpeedParam.playflag)
        {
            s_stSpeedParam.playflag = 3;
            LZM_PublicSetOnceTimer(&s_stSpeedTimer[SPEED_TIMER_TEST],PUBLICSECS(8),SpeedMonitor_ClearPlayFlag);
            SpeedMonitor_OverSpeedPreAlarmVoice();
        }
    }
}
/*********************************************************************
//��������	:SpeedMonitor_Alarm
//����		:���ٱ���
//����		:CurrentSpeed��ǰ�ٶ�,MaxSpeed����ֵ��UpDelaySec:��������
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
*********************************************************************/
void SpeedMonitor_Alarm(u8 CurrentSpeed, u8 MaxSpeed, u32 UpDelaySec)
{
    if(1 == SpeedMonitor_IsOverSpeed(CurrentSpeed, MaxSpeed, UpDelaySec))	//��Ҫ�����ϱ�
    {
        Io_WriteOverSpeedConrtolBit(OVER_SPEED_BIT_ORDINARY,SET);
        //Io_WriteAlarmBit(ALARM_BIT_OVER_SPEED,SET);
    }
    else//����Ҫ�����ϱ�
    {
        Io_WriteOverSpeedConrtolBit(OVER_SPEED_BIT_ORDINARY,RESET);
        //Io_WriteAlarmBit(ALARM_BIT_OVER_SPEED,RESET);
    }
}
/*************************************************************
** ��������: SpeedMonitor_AlarmTTS
** ��������: ������������
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
void SpeedMonitor_AlarmTTS(void)
{
    if(SpeedMonitor_CheckPlayFlag(Io_ReadAlarmBit(ALARM_BIT_OVER_SPEED),&s_stSpeedAttrib.stPlay))
    {
        if(2 != s_stSpeedParam.playflag)
        {
            s_stSpeedParam.playflag = 2;
            LZM_PublicSetOnceTimer(&s_stSpeedTimer[SPEED_TIMER_TEST],PUBLICSECS(8),SpeedMonitor_ClearPlayFlag);
            SpeedMonitor_OverSpeedAlarmVoice();
        }
    }
}
/*************************************************************
** ��������: SpeedMonitor_UpdateSpeedType
** ��������: �����ٶ����ͱ���
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
void SpeedMonitor_UpdateSpeedType(void)
{
    u8 PramLen;
    u8 Buffer[5];
    PramLen = EepromPram_ReadPram(E2_SPEED_SELECT_ID, Buffer);
    if(PramLen)
    {
        s_stSpeedAttrib.SpeedType = (Buffer[0]&0x01);//dxl,2015.5.11,0x00:�ֶ�����,0x01:�ֶ�GPS,0x02:�Զ�����,0x03:�Զ�GPS
    }
    else
    {
        s_stSpeedAttrib.SpeedType = 1;//ΪGPS�ٶ�
    }   
}
/*************************************************************
** ��������: SpeedMonitor_UpdateMaxStopCarTime
** ��������: �����ͣ��ʱ��
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
void SpeedMonitor_UpdateMaxStopCarTime(void)
{
    u8 PramLen;
    u8 Buffer[5];
    //��EEPROM��ȡ���ݸ��±���
    PramLen = EepromPram_ReadPram(E2_MAX_STOP_CAR_TIME_ID, Buffer);
    if(PramLen)
    {
        s_stSpeedAttrib.OverstopTime =Public_ConvertBufferToLong(Buffer);
    }
    else
    {
        s_stSpeedAttrib.OverstopTime = 0xffffffff;//        
    }
    /////////////////////////////
    if(0xffffffff==s_stSpeedAttrib.OverstopTime||0==s_stSpeedAttrib.OverstopTime)
    {
        Io_WriteAlarmBit(ALARM_BIT_STOP_OVER,RESET);
    }
}
/*************************************************************
** ��������: SpeedMonitor_UpdateNightParam
** ��������: ����ҹ����ʻԤ������
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
void SpeedMonitor_UpdateNightParam(void)
{
    u8 PramLen;
    u8 Buffer[30];
    //��EEPROM��ȡ���ݸ��±���
    memset(&s_stSpNight,0,sizeof(s_stSpNight));
    /////////////ҹ�䳬�ٰٷֱ�///////////////////////
    PramLen = EepromPram_ReadPram(E2_NIGHT_OVERSPEED_PERCENT_ID, Buffer);    
    if(PramLen)
    {
        s_stSpNight.percent =Public_ConvertBufferToLong(Buffer);
        if(s_stSpNight.percent>=100) s_stSpNight.percent = 0;
    }
    ////////////////////////
    if(0==s_stSpNight.percent)return;
    /////////////////////////    
    PramLen = EepromPram_ReadPram(E2_OVERSPEED_NIGHT_TIME_ID, Buffer);
    if(PramLen)
    {
        s_stSpNight.control = Public_ConvertBufferToShort(Buffer);        
        s_stSpNight.startVal=Public_ConvertBCDTimeToLong(s_stSpNight.control,&Buffer[2]);
        if(s_stSpNight.startVal)
        {
            s_stSpNight.endVal=Public_ConvertBCDTimeToLong(s_stSpNight.control,&Buffer[8]);            
            if(0==s_stSpNight.endVal)
            {
                s_stSpNight.startVal = 0;
            }
        }        
    }    
    //////////////////////////
    if(s_stSpNight.percent&&Public_CheckTimeRangeIsValid(s_stSpNight.control,s_stSpNight.startVal,s_stSpNight.endVal))
    {
        s_stSpNight.needChecknight = 1;        
    }
    else
    {
        s_stSpNight.needChecknight =0;
        s_stSpNight.percent =0;
        return;
    }
    ////////////////////////
    PramLen = EepromPram_ReadPram(E2_NIGHT_DRIVE_PREALARM_TIME_ID, Buffer);//
    if(PramLen)
    {
        s_stSpNight.earlyTime=Public_ConvertBufferToLong(Buffer);
    }
    else
    {
        s_stSpNight.earlyTime = 10;//����
    }
    s_stSpNight.earlyTime *= 60;//��
    ///////////////////////
    /////////////
    PramLen = EepromPram_ReadPram(E2_NIGHT_DRIVE_PREALARM_GPOUP_TIME_ID, Buffer);//
    if(PramLen)
    {
        s_stSpNight.stPlay.GroupTime=Public_ConvertBufferToLong(Buffer)*60;
    }
    else
    {
        s_stSpNight.stPlay.GroupTime = 300;//
    }
    PramLen = EepromPram_ReadPram(E2_NIGHT_DRIVE_PREALARM_NUMBER_ID, Buffer);//    
    if(PramLen)
    {
        s_stSpNight.stPlay.OnceNumber = Public_ConvertBufferToLong(Buffer);
    }
    else
    {
        s_stSpNight.stPlay.OnceNumber = 3;//
    }
}
/*************************************************************
** ��������: SpeedMonitor_UpdateForbidParam
** ��������: ���½�ֹ��ʻ����
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
void SpeedMonitor_UpdateForbidParam(void)
{
    u8 PramLen;
    u8 Buffer[30];
    //��EEPROM��ȡ���ݸ��±���
    memset(&s_stSpForbid,0,sizeof(s_stSpForbid));
    ////////////////////////
    PramLen = EepromPram_ReadPram(E2_FORBID_DRIVE_NIGHT_TIME_ID, Buffer);
    if(PramLen)
    {
        s_stSpForbid.control = Public_ConvertBufferToShort(Buffer);
        s_stSpForbid.startVal = Public_ConvertBCDTimeToLong(s_stSpForbid.control,&Buffer[2]);
        if(s_stSpForbid.startVal)
        {
            s_stSpForbid.endVal = Public_ConvertBCDTimeToLong(s_stSpForbid.control,&Buffer[8]);            
            if(0==s_stSpForbid.endVal)
            {
                s_stSpForbid.startVal = 0;
            }
        }
    }
    //////////////////
    if(!Public_CheckTimeRangeIsValid(s_stSpForbid.control,s_stSpForbid.startVal,s_stSpForbid.endVal))
    {
        Io_WriteSuoMeiStatusBit(SUOMEI_BIT_28,RESET);
        return;
    }
    ////////////////////////
    PramLen = EepromPram_ReadPram(E2_FORBID_DRIVE_PREALARM_TIME_ID, Buffer);//
    if(PramLen)
    {
        s_stSpForbid.earlyTime=Public_ConvertBufferToLong(Buffer);
    }
    else
    {
        s_stSpForbid.earlyTime=30;//����
    }
    s_stSpForbid.earlyTime *=60;//��   
    //////////////////////
    PramLen=EepromPram_ReadPram(E2_FORBID_DRIVE_PREALARM_GPOUP_TIME_ID, Buffer);    
    if(PramLen)////Ԥ�� ������ʾʱ����(�趨ֵ) 
    {
        s_stSpForbid.stPrePlay.GroupTime = Public_ConvertBufferToLong(Buffer)*60;
    }
    else
    {
        s_stSpForbid.stPrePlay.GroupTime =300;
    }
    PramLen=EepromPram_ReadPram(E2_FORBID_DRIVE_PREALARM_NUMBER_ID, Buffer);
    if(PramLen)//Ԥ�� ��ʾ�����ظ������Ĵ���(�趨ֵ)  
    {
        s_stSpForbid.stPrePlay.OnceNumber = Public_ConvertBufferToLong(Buffer);
    }
    else
    {
        s_stSpForbid.stPrePlay.OnceNumber =3;
    }
    ///////////////
    PramLen = EepromPram_ReadPram(E2_FORBID_DRIVE_ALARM_GPOUP_TIME_ID, Buffer);//
    if(PramLen)
    {
        s_stSpForbid.stPlay.GroupTime=Public_ConvertBufferToLong(Buffer)*60;
    }
    else
    {
        s_stSpForbid.stPlay.GroupTime =300;
    }
    ////////////////
    PramLen = EepromPram_ReadPram(E2_FORBID_DRIVE_ALARM_NUMBER_ID, Buffer);//    
    if(PramLen)
    {
        s_stSpForbid.stPlay.OnceNumber = Public_ConvertBufferToLong(Buffer);
    }
    else
    {
        s_stSpForbid.stPlay.OnceNumber =3;
    }
    ///////////////////
    PramLen = EepromPram_ReadPram(E2_FORBID_DRIVE_RUN_SPEED_ID, Buffer);//
    if(PramLen)//ͣ���ж��ٶ�
    {
        s_stSpForbid.runSpeed = Public_ConvertBufferToLong(Buffer);
    }
    else
    {
        s_stSpForbid.runSpeed =20;
    }
    ///////////////
    PramLen = EepromPram_ReadPram(E2_FORBID_DRIVE_RUN_SPEED_KEEPTIME_ID, Buffer);//
    if(PramLen)//ͣ���жϳ�������ʱ��
    {
        s_stSpForbid.runDuration = Public_ConvertBufferToLong(Buffer)*60;
    }
    else
    {
        s_stSpForbid.runDuration =300;
    }
    /////////////////
    s_stSpForbid.CheckVaild = 1;
}
/*************************************************************
** ��������: SpeedMonitor_UpDatePram
** ��������: ���û��޸ĳ�����ر�����,��Ҫ���³���ģ����ر���(��ģ���ڵ����в���)
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:SpeedMonitor_UpdateForbidParam,SpeedMonitor_UpdateNightParam
*************************************************************/
void SpeedMonitor_UpdatePram(void)
{
    unsigned long speedLimit,speedDuration;
    u8 PramLen;
    u8 Buffer[50];
    //��EEPROM��ȡ���ݸ��±���
    SpeedMonitor_UpdateNightParam();
    //////////////////////////////
    SpeedMonitor_UpdateForbidParam();
    /////////////////////////
    SpeedMonitor_UpdateSpeedType();   
    ///////////////////////
    SpeedMonitor_UpdateMaxStopCarTime();
    ///////////////////////////////////
    SpeedMonitor_ReadStopStartTime();
    /////////////////////////
    SpeedMonitor_ReadSpeedPlay();
    ////////////////////
    //��EEPROM��ȡ���ݸ��±���
    ///////////////////////////////////////    
    PramLen=EepromPram_ReadPram(E2_SPEED_EARLY_ALARM_DVALUE_ID, Buffer);
    if(PramLen)
    {
        s_stSpeedAttrib.EarlyAlarmDVal = (unsigned short)(Public_ConvertBufferToShort(Buffer)*0.1+0.5);//��λΪ1/10Km/h 
    }
    else
    {
        s_stSpeedAttrib.EarlyAlarmDVal = 5;//���ٱ���Ԥ����ֵ����λΪ1/10Km/h 
    }
    /////////////////////
    PramLen = EepromPram_ReadPram(E2_MAX_SPEED_ID, Buffer);
    if(PramLen)
    {
        speedLimit =Public_ConvertBufferToLong(Buffer);
    }
    else
    {
        speedLimit = 100;//���������Ϊ0ʱ,��ʾ�������ٶȼ��
    }
    PramLen=EepromPram_ReadPram(E2_OVER_SPEED_KEEP_TIME_ID, Buffer);
    if(PramLen)
    {
        speedDuration =Public_ConvertBufferToLong(Buffer);
    }
    else
    {
        speedDuration = 10;//Ĭ������ǳ��ٴ�10���ű���
    }
    //////////////////////////
    SpeedMonitor_SetSpeedParamValid(E_SPEED_TYPE_GENERAL,0,speedLimit,speedDuration);  
    //////////////��������/////////////////////
    PramLen=EepromPram_ReadPram(E2_OVERSPEED_ALARM_NUMBER_ID, Buffer);
    if(PramLen)
    {
        s_stSpeedAttrib.stPlay.OnceNumber = Public_ConvertBufferToLong(Buffer);
    }
    else
    {
       s_stSpeedAttrib.stPlay.OnceNumber =3; 
    }
    PramLen=EepromPram_ReadPram(E2_OVERSPEED_ALARM_GPROUP_TIME_ID, Buffer);
    if(PramLen)
    {
        s_stSpeedAttrib.stPlay.GroupTime = Public_ConvertBufferToLong(Buffer)*60;
    }
    else
    {
       s_stSpeedAttrib.stPlay.GroupTime =300; 
    }
    PramLen=EepromPram_ReadPram(E2_OVERSPEED_ALARM_NUMBER_TIME_ID, Buffer);
    if(PramLen)
    {
        s_stSpeedAttrib.stPlay.OnceInterval = Public_ConvertBufferToLong(Buffer);
    }
    else
    {
       s_stSpeedAttrib.stPlay.OnceInterval =10; 
    }
    ////////////////////////
    PramLen=EepromPram_ReadPram(E2_OVERSPEED_ALARM_VOICE_ID, Buffer);
    if(PramLen)
    {
        Buffer[PramLen]=0;
        SpeedMonitor_SetSpeedParamPlayVoice(E_SPEED_TYPE_GENERAL,(char *)Buffer);
    }
    else
    {
        SpeedMonitor_SetSpeedParamPlayVoice(E_SPEED_TYPE_GENERAL,"���ѳ���,�������ʻ");
    }
    /////////////////////////////////////////////
    SpeedMonitor_SetSpeedParamPlay(E_SPEED_TYPE_GENERAL,s_stSpeedAttrib.stPlay.GroupTime,s_stSpeedAttrib.stPlay.OnceInterval,s_stSpeedAttrib.stPlay.OnceNumber); 
    
    /////////////����Ԥ������///////////////
    PramLen=EepromPram_ReadPram(E2_OVERSPEED_PREALARM_NUMBER_ID, Buffer);
    if(PramLen)//Ԥ���� ��ʾ�����ظ������Ĵ���(�趨ֵ)  
    {
        s_stSpeedAttrib.stPrePlay.OnceNumber = Public_ConvertBufferToLong(Buffer);
    }
    else
    {
       s_stSpeedAttrib.stPrePlay.OnceNumber =3; 
    }
    PramLen=EepromPram_ReadPram(E2_OVERSPEED_PREALARM_GPROUP_TIME_ID, Buffer);
    if(PramLen)//Ԥ���� ����ʾʱ����(�趨ֵ)  
    {
        s_stSpeedAttrib.stPrePlay.GroupTime = Public_ConvertBufferToLong(Buffer)*60;
    } 
    else
    {
       s_stSpeedAttrib.stPrePlay.GroupTime =300; 
    }
    
    PramLen=EepromPram_ReadPram(E2_OVERSPEED_PREALARM_NUMBER_TIME_ID, Buffer);
    if(PramLen)////Ԥ���� ������ʾʱ����(�趨ֵ) 
    {
        s_stSpeedAttrib.stPrePlay.OnceInterval = Public_ConvertBufferToLong(Buffer);
    }
    else
    {
       s_stSpeedAttrib.stPrePlay.OnceInterval =10;
    }
    
    PramLen=EepromPram_ReadPram(E2_OVERSPEED_PREALARM_KEEPTIME_ID, Buffer);
    if(PramLen)//Ԥ���ٳ���ʱ������ֵ
    {
        s_stSpeedAttrib.prespeedDuration = Public_ConvertBufferToLong(Buffer);
    }
    else
    {
       s_stSpeedAttrib.prespeedDuration =3;
    }
    ///////////////////////////
    PramLen=EepromPram_ReadPram(E2_OVERSPEED_ALARM_REPORT_TIME_ID, Buffer);
    if(PramLen)//���ٱ���λ���ϱ�ʱ����,DWORD,��λ��,Ĭ��ֵ0
    {
        s_stSpeedAttrib.ReportInterval = Public_ConvertBufferToLong(Buffer);
    }
    else
    {
       s_stSpeedAttrib.ReportInterval =0;
    }
    PramLen=EepromPram_ReadPram(E2_REPORT_FREQ_EVENT_SWITCH_ID, Buffer);
    if(PramLen)//����ָ�������¼����ϱ�ʱ�������¼��� BIT0λ��1�����ٱ���
    {
        s_stSpeedAttrib.ReportFreqEvent = Public_ConvertBufferToLong(Buffer);
    }
    else
    {
       s_stSpeedAttrib.ReportFreqEvent = 0;
    }
    ///////////////////////////
    if(!PUBLIC_CHECKBIT(s_stSpeedAttrib.ReportFreqEvent,0))
    {
        s_stSpeedAttrib.ReportInterval = 0;
    }
    ////////////////////
    s_stSpeedAttrib.overSpeedCnt = 0;
    s_stSpeedAttrib.preOverSpeedCnt = 0;
    /////////////////////////////////
    SpeedMonitor_UpdateCurSpeedLimitParam();
}
/*************************************************************
** ��������: SpeedMonitor_GetCurSpeed
** ��������: ��ȡ˲ʱ�ٶ�(�ѿ������ٶ����ͣ��������ж��ٶ�������)
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
unsigned char SpeedMonitor_GetCurSpeed(void)
{
    u8	CurrentSpeed;	
    //��ȡ�ٶ�ѡ��
    if(0 == SpeedMonitor_GetCurSpeedType())
    {
        CurrentSpeed = Pulse_GetSpeed();
    }
    else
    {
        CurrentSpeed = Gps_ReadSpeed();
    }    
    return CurrentSpeed;
}
/*************************************************************
** ��������: SpeedMonitor_GetSpeedMaxLimitVal
** ��������: ��ȡ����ֵ
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: ����ֵ
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
u32 SpeedMonitor_GetSpeedMaxLimitVal(void)
{
    return s_stSpeedAttrib.curSpeedLimit;
}
/*************************************************************
** ��������: SpeedMonitor_GetCurSpeedType
** ��������: ��ȡ�ٶ�����
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 0Ϊ����,1ΪGPS
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
unsigned char SpeedMonitor_GetCurSpeedType(void)
{
    return (s_stSpeedAttrib.SpeedType)?1:0;
}
/*************************************************************
** ��������: SpeedMonitor_GetEarlyAlarmDVal
** ��������: ��ȡ�ٶ�Ԥ����ֵ
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
unsigned char SpeedMonitor_GetEarlyAlarmDVal(void)
{
    return s_stSpeedAttrib.EarlyAlarmDVal;
}
/*************************************************************
** ��������: SpeedMonitor_GetReportFreqEvent
** ��������: ��ȡ�ϱ�ʱ�������¼���
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
unsigned long SpeedMonitor_GetReportFreqEvent(void)
{
    return s_stSpeedAttrib.ReportFreqEvent;
}
/*************************************************************
** ��������: SpeedMonitor_CheckOverStopTime
** ��������: �ж��Ƿ�ʱͣ��
** ��ڲ���: 
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void SpeedMonitor_CheckOverStopTime(void)
{
    unsigned long timeVal;
    unsigned char flag;
    if(0xffffffff==s_stSpeedAttrib.OverstopTime||0==s_stSpeedAttrib.OverstopTime)
    {
        return;
    }
    flag = 0;    
    if(TiredDrive_GetTimeAbnormalflag())
    {
        s_stSpeedAttrib.startStopTime =0;
        SpeedMonitor_SaveStopStartTime();
    }
    //////////////////////////
    if(CAR_STATUS_RUN==s_stSpeedAttrib.runStatus)//��ʻ
    {
        if(s_stSpeedAttrib.startStopTime)
        {
            s_stSpeedAttrib.startStopTime =0;
            SpeedMonitor_SaveStopStartTime();
        }
    }
    else
    if((CAR_STATUS_RUN==s_stSpeedAttrib.prerunStatus)&&(CAR_STATUS_STOP==s_stSpeedAttrib.runStatus))//����ʻ��ͣʻ
    {
        s_stSpeedAttrib.startStopTime = RTC_GetCounter()-RUN_CHAGE_STATUS_TIME;//
        SpeedMonitor_SaveStopStartTime();
    }
    else
    if(CAR_STATUS_STOP==s_stSpeedAttrib.runStatus)//ͣʻ
    {
        timeVal =RTC_GetCounter();
        if(0==s_stSpeedAttrib.startStopTime)
        {
            s_stSpeedAttrib.startStopTime = timeVal;//
            SpeedMonitor_SaveStopStartTime();
        }
        else//�����ʼͣ��ʱ������ڵ�ʱ�仹��
        if(s_stSpeedAttrib.startStopTime>=timeVal)
        {
            s_stSpeedAttrib.startStopTime = timeVal;//
            SpeedMonitor_SaveStopStartTime();
        }
        ////////////////////        
        if(timeVal>=s_stSpeedAttrib.startStopTime+s_stSpeedAttrib.OverstopTime)
        {
            flag =1;
        }
    }
    
    ///////////////////////////
    if(1==flag)
    {
        if(2 != s_stSpeedAttrib.OverStopTimeFlag)
        {
            s_stSpeedAttrib.OverStopTimeFlag = 2;
            Io_WriteAlarmBit(ALARM_BIT_STOP_OVER,SET);
        }
    }
    else
    {
        if(1 != s_stSpeedAttrib.OverStopTimeFlag)
        {
            s_stSpeedAttrib.OverStopTimeFlag = 1;
            Io_WriteAlarmBit(ALARM_BIT_STOP_OVER,RESET);
        }
    }
}
/*************************************************************
** ��������: SpeedMonitor_ClearPlayFlag
** ��������: 
** ��ڲ���: 
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void SpeedMonitor_ClearPlayFlag(void) 
{
    s_stSpeedParam.playflag =0;
}
/*************************************************************
** ��������: SpeedMonitor_PlayCurSpeedLimitValue
** ��������: 
** ��ڲ���: 
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void SpeedMonitor_PlayCurSpeedLimitValue(void) 
{
    if(!SpeedMonitor_GetCurRunStatus())
    { 
        s_stSpeedPlay.playSpeed = s_stSpeedAttrib.curSpeedLimit;
        SpeedMonitor_SaveSpeedPlay();
        return;
    }
    //////////////////////////////
    if(s_stSpeedPlay.playSpeed == s_stSpeedAttrib.curSpeedLimit)return;
    ///////////////////////////
    if((0==TTS_GetPlayBusyFlag())||(TTS_GetPlayBusyFlag()&&1==s_stSpeedParam.playflag))
    {
        unsigned char buffer[20];
        s_stSpeedParam.playflag = 1;       
        LZM_PublicSetOnceTimer(&s_stSpeedTimer[SPEED_TIMER_TEST],PUBLICSECS(8),SpeedMonitor_ClearPlayFlag);
        sprintf((char *)buffer,"��ǰ����%d����",s_stSpeedAttrib.curSpeedLimit);       
        Public_PlayTTSVoiceStr(buffer);
        s_stSpeedPlay.playSpeed = s_stSpeedAttrib.curSpeedLimit;
        SpeedMonitor_SaveSpeedPlay();
    }
    else
    {
       LZM_PublicSetOnceTimer(&s_stSpeedTimer[SPEED_TIMER_PLAY],PUBLICSECS(1),SpeedMonitor_PlayCurSpeedLimitValue);
    }
}
/*************************************************************
** ��������: SpeedMonitor_UpdateCurSpeedLimitParam
** ��������: 
** ��ڲ���: 
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void SpeedMonitor_UpdateCurSpeedLimitParam(void) 
{
    unsigned char type;
    type = s_stSpeedParam.type;    
    s_stSpeedAttrib.overSpeedCnt = 0;
    s_stSpeedAttrib.preOverSpeedCnt = 0;
    s_stSpeedAttrib.curSpeedLimit = s_stSpeedParam.speedLimit;
    s_stSpeedAttrib.stPlay.GroupTime = s_stSpeedParam.stSpType[type].GroupTime; 
    s_stSpeedAttrib.stPlay.OnceNumber = s_stSpeedParam.stSpType[type].OnceNumber;       
    s_stSpeedAttrib.stPlay.OnceInterval = s_stSpeedParam.stSpType[type].OnceInterval;    
    SpeedMonitor_UpdateNightSpeedLimit();
    //////////����Ԥ��//////////////////    
    s_stSpeedAttrib.curprespeedDuration = s_stSpeedAttrib.prespeedDuration;
    if(s_stSpeedAttrib.EarlyAlarmDVal&&s_stSpeedAttrib.curSpeedLimit>s_stSpeedAttrib.EarlyAlarmDVal)
    {
        s_stSpeedAttrib.curpreSpeedLimit = s_stSpeedAttrib.curSpeedLimit-s_stSpeedAttrib.EarlyAlarmDVal;
    }
    else
    {
        s_stSpeedAttrib.curpreSpeedLimit = 0;
    }
    /////////////////
    if(Io_ReadAlarmMaskBit(ALARM_BIT_OVER_SPEED))return;//���ٹ��������˾Ͳ������ٶ���ʾ��
    LZM_PublicSetOnceTimer(&s_stSpeedTimer[SPEED_TIMER_PLAY],PUBLICSECS(4),SpeedMonitor_PlayCurSpeedLimitValue);
}
/*************************************************************
** ��������: SpeedMonitor_CheckSpeedLimitIndex
** ��������: 
** ��ڲ���: 
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void SpeedMonitor_CheckSpeedLimitIndex(void) 
{
    unsigned char i;
    unsigned char speed,type;
    /////////////////////////
    type = 0;
    speed = s_stSpeedParam.stSpType[0].SpeedLimit;    
    for(i=1; i<E_SPEED_TYPE_MAX; i++)//������ҹ��,ҹ���������С�Ļ�����*�ٷֱ�
    {
        if(PUBLIC_CHECKBIT(s_stSpeedParam.valid, i))
        {
            if(speed > s_stSpeedParam.stSpType[i].SpeedLimit)
            {
                type = i;
                speed = s_stSpeedParam.stSpType[i].SpeedLimit;
            }
        }
    }
    ////////////////////////////
    if((s_stSpeedParam.type != type)||(s_stSpeedParam.speedLimit != s_stSpeedParam.stSpType[type].SpeedLimit))
    {
        s_stSpeedParam.type = type;
        s_stSpeedParam.speedLimit =s_stSpeedParam.stSpType[type].SpeedLimit;
        SpeedMonitor_UpdateCurSpeedLimitParam();
    }
}
/*************************************************************
** ��������: SpeedMonitor_SetSpeedParamValid
** ��������: ����ĳ��������Ϊ��Ч
** ��ڲ���: type,��������,ID:����ID����·ID,speedLimit����ֵ,duration����ʱ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void SpeedMonitor_SetSpeedParamValid(unsigned char type,unsigned long ID,unsigned char speedLimit,unsigned char duration) 
{
    if(type>=E_SPEED_TYPE_MAX)return;   
    //////////////////
    s_stSpeedParam.stSpType[type].ID = ID;
    ///////////////////////
    if(E_SPEED_TYPE_GENERAL != type)//
    {
       s_stSpeedParam.stSpType[type].OnceNumber = s_stSpeedParam.stSpType[0].OnceNumber ;
       s_stSpeedParam.stSpType[type].GroupTime = s_stSpeedParam.stSpType[0].GroupTime;
       s_stSpeedParam.stSpType[type].OnceInterval = s_stSpeedParam.stSpType[0].OnceInterval;
       strcpy(s_stSpeedParam.stSpType[type].PlayVoice,s_stSpeedParam.stSpType[0].PlayVoice);
    }
    else
    if(PUBLIC_CHECKBIT(s_stSpeedParam.valid, type))//�����򲻸�����ʱʹ��,==E_SPEED_TYPE_GENERAL����Ч
    {
        unsigned char buffer[10];
        if(speedLimit != s_stSpeedParam.stSpType[E_SPEED_TYPE_GENERAL].SpeedLimit)             
        {
            Public_ConvertLongToBuffer(speedLimit,buffer);
            EepromPram_WritePram(E2_MAX_SPEED_ID,buffer,E2_MAX_SPEED_LEN);
        }
        //////////////////////////
        if(duration != s_stSpeedParam.stSpType[E_SPEED_TYPE_GENERAL].Duration)
        {
            Public_ConvertLongToBuffer(duration,buffer);
            EepromPram_WritePram(E2_OVER_SPEED_KEEP_TIME_ID,buffer,E2_OVER_SPEED_KEEP_TIME_LEN);
        }
    }
    ///////////////////
    if(PUBLIC_CHECKBIT(s_stSpeedParam.valid, type))//��Ч
    {
        if(s_stSpeedParam.stSpType[type].SpeedLimit == speedLimit)
        {
            s_stSpeedParam.stSpType[type].Duration = duration;
            if(s_stSpeedParam.type == type)
            {
                s_stSpeedAttrib.speedDuration = duration;
            }
            return;//��ֵ����
        }
    }
    else//
    {
        PUBLIC_SETBIT(s_stSpeedParam.valid, type); 
    }
    ////////////////////////////
    s_stSpeedParam.stSpType[type].Duration = duration;
    ///////////////////////////////
    s_stSpeedParam.stSpType[type].SpeedLimit = speedLimit;
    /////////////////////////////////////
    if(s_stSpeedParam.type == type)//��ǰ���µĲ�����������Сֵ������ͬʱ
    {
        SpeedMonitor_CheckSpeedLimitIndex();
    }
    else//��ǰ���µĲ�����������Сֵ���Ͳ�ͬʱ
    if(s_stSpeedParam.speedLimit >= speedLimit)
    {
        s_stSpeedParam.type = type;
        s_stSpeedParam.speedLimit = speedLimit;         
        SpeedMonitor_UpdateCurSpeedLimitParam();
    }    
}
/*************************************************************
** ��������: SpeedMonitor_ClearSpeedParamValid
** ��������: ����ĳ��������Ϊ��Ч
** ��ڲ���: type,��������
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void SpeedMonitor_ClearSpeedParamValid(unsigned char type) 
{
    if(type>=E_SPEED_TYPE_MAX)return;
    if(!PUBLIC_CHECKBIT(s_stSpeedParam.valid, type))return;
    PUBLIC_CLRBIT(s_stSpeedParam.valid, type);
    if(s_stSpeedParam.type == type)
    {
        SpeedMonitor_CheckSpeedLimitIndex();
    }
}
/*************************************************************
** ��������: SpeedMonitor_SetSpeedParamPlayVoice
** ��������: ����ĳ�������͵Ĳ�������
** ��ڲ���: type,��������,playVoice���ٲ�������
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void SpeedMonitor_SetSpeedParamPlayVoice(unsigned char type,char *playVoice) 
{
    if(type>=E_SPEED_TYPE_MAX)return;
    if(!PUBLIC_CHECKBIT(s_stSpeedParam.valid, type))return;
    strcpy(s_stSpeedParam.stSpType[type].PlayVoice,playVoice);
}
/*************************************************************
** ��������: SpeedMonitor_SetSpeedParamPlay
** ��������: ����ĳ�������͵Ĳ�������
** ��ڲ���: type,��������,GroupTime����ʾʱ����;OnceInterval������ʾʱ����;OnceNumber��ʾ�����ظ������Ĵ���
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void SpeedMonitor_SetSpeedParamPlay(unsigned char type,unsigned short GroupTime,unsigned short OnceInterval,unsigned char OnceNumber) 
{
    if(type>=E_SPEED_TYPE_MAX)return;
    if(!PUBLIC_CHECKBIT(s_stSpeedParam.valid, type))return;
    s_stSpeedParam.stSpType[type].GroupTime = GroupTime;
    s_stSpeedParam.stSpType[type].OnceNumber = OnceNumber;    
    s_stSpeedParam.stSpType[type].OnceInterval = OnceInterval;    
    if(type==s_stSpeedParam.type)//
    {
        s_stSpeedAttrib.stPlay.GroupTime = GroupTime;
        s_stSpeedAttrib.stPlay.OnceNumber = OnceNumber;        
        s_stSpeedAttrib.stPlay.OnceInterval = OnceInterval;
    }
}
/*************************************************************
** ��������: SpeedMonitor_SetPreLimitSpeedParam
** ��������: ����Ԥ������ֵ(ֻ���ٶ������뵱ǰ��������һ��ʱ����Ч)
** ��ڲ���: type�ٶ�����,preSpeedLimit����ֵ,duration����ʱ��,saveflag:�Ƿ���Ҫ����,0����Ҫ����,1:��Ҫ����
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void SpeedMonitor_SetPreLimitSpeedParam(unsigned char type,unsigned char preSpeedLimit,unsigned char duration,unsigned char saveflag)
{
   
}
/*************************************************************
** ��������: SpeedMonitor_CheckPlayFlag
** ��������: ����Ƿ���Ҫ����
** ��ڲ���: VaildFlag�����Ч��־,stPlay���������ṹ��
** ���ڲ���: 
** ���ز���: 1:��Ҫ����,0:����Ҫ����
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
unsigned char SpeedMonitor_CheckPlayFlag(unsigned char VaildFlag,STPLAY_ATTRIB *stPlay)
{
    unsigned short temp,tmp;    
    if(VaildFlag&&stPlay->OnceNumber)
    {
        stPlay->GroupTimeCnt++;//����ʾʱ�������
        if(stPlay->GroupTime)//����ʾʱ����
        {
            temp = stPlay->GroupTimeCnt%stPlay->GroupTime;//5����һ��
        }
        else
        {
            temp = stPlay->GroupTimeCnt;
        }
        //////////////////////////////////////
        if(temp==1)//����ʾʱ����ʱ�䵽
        {
            stPlay->OnceCnt = 0;//��ʾ�����ظ������ļ����� 
        }
        /////////////////////////
        tmp = stPlay->OnceInterval;
        if(0==tmp)//δ����
        {
            tmp=ONCE_DEFAULT_PLAY_TIME;
        }
        else
        if(tmp < ONCE_MIN_PLAY_TIME)
        {
            tmp = ONCE_MIN_PLAY_TIME;//������ʾʱ����
        }
        if(1 ==(temp%tmp))//������ʾʱ����
        {
            if(stPlay->OnceCnt<stPlay->OnceNumber)
            {
                stPlay->OnceCnt++;
                return 1;
            }
        }
    }
    else
    {
        stPlay->OnceCnt = 0;
        stPlay->GroupTimeCnt = 0;
    }
    return 0;
}
/*************************************************************
** ��������: SpeedMonitor_GetNeedReportFlag
** ��������: �Ƿ���Ҫ�ϱ�λ����Ϣ
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 0:����Ҫ�ϱ�,1:��Ҫ�ϱ�
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
void SpeedMonitor_CheckNeedReportFlag(void)
{
    unsigned short temp;
    s_stSpeedAttrib.needReportflag = 0;
    if(SpeedMonitor_GetOverSpeedStatusForReport())
    {
        s_stSpeedAttrib.ReportTimeCnt++;
        temp = s_stSpeedAttrib.ReportTimeCnt%s_stSpeedAttrib.ReportInterval;
        if(1 == temp)
        {
            s_stSpeedAttrib.needReportflag =1;//
        }
    }
    else
    {
       s_stSpeedAttrib.ReportTimeCnt =0;
    }
}
/*************************************************************
** ��������: SpeedMonitor_CheckOverSpeedAlarm
** ��������: �ٶȼ�صĶ�ʱ����(1s����һ��)
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
void SpeedMonitor_CheckOverSpeedAlarm(void)
{
    //�����ж�
    SpeedMonitor_Alarm(s_stSpeedAttrib.curSpeed, s_stSpeedAttrib.curSpeedLimit, s_stSpeedParam.stSpType[s_stSpeedParam.type].Duration);
    //������������
    SpeedMonitor_AlarmTTS();
    //Ԥ���жϼ���������
    SpeedMonitor_PreAlarm(s_stSpeedAttrib.curSpeed,s_stSpeedAttrib.curpreSpeedLimit);    
    ///////////////////
    SpeedMonitor_CheckNeedReportFlag();
}
/*************************************************************
** ��������: SpeedMonitor_CheckIsAtForbidTime
** ��������: �ж��Ƿ����ڽ�ֹ��ʻʱ���
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
void SpeedMonitor_CheckIsAtForbidTime(void)
{
    unsigned char flag,flag1;    
    if(CAR_STATUS_RUN != s_stSpForbid.runStatus)
    {
        SpeedMonitor_CheckPlayFlag(0,&s_stSpForbid.stPlay);
        SpeedMonitor_CheckPlayFlag(0,&s_stSpForbid.stPrePlay);
        Io_WriteSuoMeiStatusBit(SUOMEI_BIT_28,RESET);
        return;
    }
    ////////////////////////
    flag = 0;
    flag1 = 0;
    if(Public_CheckCurTimeIsInTimeRange(s_stSpForbid.control,s_stSpForbid.startVal,s_stSpForbid.endVal))
    {
        flag1=1;
    }
    else//ҹ��Ԥ��ʱ���
    if(Public_CheckCurTimeIsInEarlyAlarmTimeRange(s_stSpForbid.control,s_stSpForbid.earlyTime,s_stSpForbid.startVal,s_stSpForbid.endVal))
    {
        flag =1;
    }
    /////////////////////////////////
    if(SpeedMonitor_CheckPlayFlag(flag,&s_stSpForbid.stPrePlay))
    {
        SpeedMonitor_ForbidPreAlarmVoice();
    }
    /////////////////////
    if(SpeedMonitor_CheckPlayFlag(flag1,&s_stSpForbid.stPlay))
    {
        SpeedMonitor_ForbidAlarmVoice();
    }
    //////////////////////
    if(flag1)//����ʱ����ʻ����
    {
        Io_WriteSuoMeiStatusBit(SUOMEI_BIT_28,SET);
    }
    else
    {
        Io_WriteSuoMeiStatusBit(SUOMEI_BIT_28,RESET);
    }
}
/*************************************************************
** ��������: SpeedMonitor_CheckForbidAlarm
** ��������: 
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
void SpeedMonitor_CheckForbidAlarm(void)
{
    if(0==s_stSpForbid.CheckVaild)return;
    SpeedMonitor_CheckForbidRunStatus(s_stSpeedAttrib.curSpeed);
    SpeedMonitor_CheckIsAtForbidTime();
}

/*************************************************************
** ��������: SpeedMonitor_GetOverSpeedStatusForReport
** ��������: ��ȡ�Ƿ���ݳ���״̬���ϱ�λ����Ϣ
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 0:�����ݳ���״̬���ϱ�, 1:���ݳ���״̬���ϱ�
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
unsigned char SpeedMonitor_GetOverSpeedStatusForReport(void)
{
    if(!PUBLIC_CHECKBIT(s_stSpeedAttrib.ReportFreqEvent,0))return 0;
    if(0 == s_stSpeedAttrib.ReportInterval)return 0;
    if(0 == Io_ReadAlarmBit(ALARM_BIT_OVER_SPEED))return 0;
    if(Io_ReadAlarmBit(ALARM_BIT_EMERGENCY))return 0;
    return 1;
}
/*************************************************************
** ��������: SpeedMonitor_GetNeedReportFlag
** ��������: ��ȡ�Ƿ���Ҫ�ϱ�λ����Ϣ��־
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 0:����Ҫ�ϱ�,1:��Ҫ�ϱ�
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
unsigned char SpeedMonitor_GetNeedReportFlag(void)
{
    if(0==s_stSpeedAttrib.needReportflag)return 0;
    s_stSpeedAttrib.needReportflag = 0;
    return 1;
    
}
/*************************************************************
** ��������: SpeedMonitor_GetCurMinSpeedType
** ��������: ��ȡ��ǰ����С�ٶȵ�����
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 0,��ͨ;1:Բ��;2:����;3:�����;4:��·
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
unsigned char SpeedMonitor_GetCurMinSpeedType(void)
{
    return s_stSpeedParam.type;
}
/*************************************************************
** ��������: SpeedMonitor_ScanTask
** ��������: �ٶȼ�صĶ�ʱ����(1s����һ��)
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
void SpeedMonitor_ScanTask(void)
{
    u8 CurrentSpeed = 0;
    CurrentSpeed=SpeedMonitor_GetCurSpeed();     
    ///////////////////////////
    s_stSpeedAttrib.curSpeed =CurrentSpeed;
    SpeedMonitor_CheckRunStatus(CurrentSpeed);
    ///////////�ж��Ƿ�ʱͣ��///////////////
    SpeedMonitor_CheckOverStopTime();
    ///////////��ʻ��¼////////////////////
    if(Gps_GetRtcAdjSta()){
    SpeedMonitor_SaveRunRecordData(CurrentSpeed); 
    }
		/////////ҹ���ж�///////////////////////
    SpeedMonitor_CheckIsAtNightFlag();
    ////////////////////////////////
    if(s_stSpForbid.CheckVaild)
    {
        LZM_PublicSetOnceTimer(&s_stSpeedTimer[SPEED_TIMER_FORBID],PUBLICSECS(0.4),SpeedMonitor_CheckForbidAlarm);
    }    
    //�����ٱ�������λ״̬
    if(Io_ReadAlarmMaskBit(ALARM_BIT_OVER_SPEED))
    {
        s_stSpeedAttrib.overSpeedCnt = 0;
        s_stSpeedAttrib.preOverSpeedCnt = 0;
        if(s_stSpeedAttrib.stPlay.GroupTimeCnt)
        {
            s_stSpeedAttrib.stPlay.GroupTimeCnt = 0;
            Io_WriteAlarmBit(ALARM_BIT_OVER_SPEED,RESET);
        }
        /////////////////////////
        if(s_stSpeedAttrib.stPrePlay.GroupTimeCnt)
        {
            s_stSpeedAttrib.stPrePlay.GroupTimeCnt = 0;
            Io_WriteAlarmBit(ALARM_BIT_OVER_SPEED_PRE_ALARM,RESET);
        }
        return;
    }
    ///////////////////////////
    LZM_PublicSetOnceTimer(&s_stSpeedTimer[SPEED_TIMER_CHECK],PUBLICSECS(0.2),SpeedMonitor_CheckOverSpeedAlarm);     
}
/*************************************************************
** ��������: SpeedMonitor_TimeTask
** ��������: �ٶȼ�صĶ�ʱ����(1s����һ��)
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
void SpeedMonitor_StartScanTask(void)
{
    LZM_PublicSetCycTimer(&s_stSpeedTimer[SPEED_TIMER_TASK],PUBLICSECS(1),SpeedMonitor_ScanTask);    
}
/*************************************************************
** ��������: SpeedMonitor_CheckFlash
** ��������: ����ٶȼ�¼��flash���� 
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
void SpeedMonitor_CheckFlash(void)
{
      if(Register_CheckOneArea2(REGISTER_TYPE_SPEED))
      {
           LZM_PublicSetOnceTimer(&s_stSpeedTimer[SPEED_TIMER_TASK],PUBLICSECS(0.1),SpeedMonitor_StartScanTask); 
      }
   
}
/*************************************************************
** ��������: SpeedMonitor_ParamInitialize
** ��������: �ڿ���ʱ��ʼ��
** ��ڲ���: 
** ���ڲ���: 
** ���ز���:  
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
void SpeedMonitor_ParamInitialize(void)
{
    static unsigned char sucspeedfirstRun=0;
    if(0==sucspeedfirstRun)
    {
        sucspeedfirstRun = 1;
        LZM_PublicKillTimerAll(s_stSpeedTimer,SPEED_TIMERS_MAX);
        memset((unsigned char *)&s_stSpeedAttrib,0,sizeof(s_stSpeedAttrib));
        memset((unsigned char *)&s_stSpeedParam,0,sizeof(s_stSpeedParam));
        s_stSpeedParam.speedLimit = 255;
        SetTimerTask(TIME_SPEED_MONITOR, 1);        
        s_stSpeedAttrib.runStatus = CAR_STATUS_STOP;
        s_stSpeedAttrib.prerunStatus = CAR_STATUS_STOP;        
        SpeedMonitor_UpdatePram();
        SpeedMonitor_ReadRunRecordData();
        LZM_PublicSetCycTimer(&s_stSpeedTimer[SPEED_TIMER_TASK],1,SpeedMonitor_CheckFlash);    
    }
    else
    {
        SpeedMonitor_UpdatePram();
    }
}
/*************************************************************
** ��������: SpeedMonitor_TimeTask
** ��������: �ٶȼ�صĶ�ʱ����(1s����һ��)
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
FunctionalState SpeedMonitor_TimeTask(void)
{    
    LZM_PublicTimerHandler(s_stSpeedTimer,SPEED_TIMERS_MAX);
    return ENABLE;
}

/*******************************************************************************
 *                             end of module                                   *
 *******************************************************************************/

/*******************************************************************************
 * File Name:			tireddrivere.c 
 * Function Describe:	
 * Relate Module:		�жϡ��洢����ȡ��ʱ��ʻ��¼
 * Writer:				Joneming
 * Date:				2012-12-08
 * ReWriter:			
 * Date:
 *******************************************************************************/

//********************************ͷ�ļ�************************************
#include "include.h"
//********************************�Զ�����������****************************
//********************************�궨��************************************
/////////////////////////////////
#define SAVE_TIRED_DRIVE_INFO_SECTOR   (FLASH_TIRED_DRIVE_INFO_START_SECTOR)
#define SAVE_TIRED_DRIVE_INFO_ADDR    (SAVE_TIRED_DRIVE_INFO_SECTOR*FLASH_ONE_SECTOR_BYTES)//

#ifndef FRAM_TIRED_SAVE_INDEX_ADDR//��ʻԱ��Ϣ��������λ�ýṹ��(4���ֽ�)
#define FRAM_TIRED_SAVE_INDEX_ADDR      FRAM_START_TIME_ADDR//��ʱ��ʻ��¼����ʱ��ĵ�ַ������������λ�ýṹ��(4���ֽ�)
#endif
#ifndef FRAM_TIRED_STOP_INFO_ADDR////ͣ����Ϣ�ṹ��(16���ֽ�)
#define FRAM_TIRED_STOP_INFO_ADDR      FRAM_END_TIME_ADDR//ͣ����Ϣ�ṹ��(16���ֽ�)
#endif
///////////////////////////////////////
#define TIRED_DRIVER_INFO_NUM           5//����ʻԱ��
#define TIRED_DRIVER_INFO_SAVE_VAL      0xaa//��ʻԱ�����ṹ�屣���־ֵ
////////////////////////////////////
#define RESEET_CURRENT_TIRED_DRIVER_TIME_VAL    15//(��)ʱ���쳣��ֵ,��Ҫ�����쳣����

static u8 TiredDriveSaveFlg = 0;
//********************************ȫ�ֱ���**********************************

//********************************�ⲿ����**********************************

//********************************���ر���**********************************
/////////////////////////////////////
typedef struct
{
    unsigned long starttime;            //����ʱ��
    unsigned long endtime;              //ͣ��ʱ��
    unsigned long nightstarttime;       //���Ͽ�ʼ��ʻʱ��
    unsigned char validflag;            //��ǰ��ʻԱ��ʱ��־
    unsigned char nightvalidFlag;       //���Ͽ�ʼ��ʱ��־
    unsigned char overtimeflag;         //��ʱ��ʻ��־
    unsigned char preovertimeflag;      //��ʱԤ����־        
    unsigned char startPosition[10];    //����λ����Ϣ
    unsigned char endPosition[10];      //ͣ��λ����Ϣ
    unsigned char driverLicence[20];    //��ʻ֤(ֻʹ��ǰ��18�ֽ�)    
    unsigned long totalDriveTime;       //�����ۼƼ�ʻʱ��//unsigned char drivecode[4];       //��ʻԱ����(δʹ��)
    unsigned long preReportTime;        //ǰһ���ϱ�ʱ��
}STTIREDDRIVERINFO;//������ʻԱ�ṹ��

typedef struct
{
    STTIREDDRIVERINFO driverinfo[TIRED_DRIVER_INFO_NUM+1];//���һ��Ϊ�޵�¼ʱ��¼
    unsigned char loginindex;               //��ǰ��¼�ļ�ʻԱ���
    unsigned char currentindex;             //��ǰ��ʻԱ��������    
    unsigned char driverloginflag;          //��ʻԱ�Ƿ��¼��־    
    unsigned char bak[1];                   //���� 
    unsigned long clearTime;                //����ۼƼ�ʻ������//unsigned char bak[5];                 //����
}STTIREDDRIVER;//��ʻԱ�ṹ��

static STTIREDDRIVER s_stTiredDriver;
#define SIZE_STTIREDDRIVER sizeof(STTIREDDRIVER)

typedef struct
{
    unsigned short currentAddr;            //��ǰ��¼���׵�ַ
    unsigned char  index;                  //����
    unsigned char  savedflag;              //�����־
}STTIREDSAVEINDEX;//��ʻԱ�ṹ�洢����

static STTIREDSAVEINDEX s_stTiredSaveIndex;
#define SIZE_STTIREDSAVEINDEX sizeof(STTIREDSAVEINDEX)

typedef struct
{
    STPLAY_ATTRIB stPlay;                   //��ʱ����
    STPLAY_ATTRIB stPrePlay;                //Ԥ������
    ////////////////////////
    unsigned long restTimeVal;              //��Ϣʱ��,��λ��
    unsigned long overTimeVal;              //��ʱ��ʻʱ��,��λ��
    unsigned long nightoverTimeVal;         //���ϳ�ʱ��ʻʱ��,��λ��    
    unsigned long nightrestTimeVal;         //������Ϣʱ��,��λ��
    unsigned long oneDayoverTimeVal;        //�����ۼƳ�ʱ��ʻʱ��,��λ��
    unsigned long nightStartSec;            //���Ͽ�ʼ����
    unsigned long nightStartVal;            //���Ͽ�ʼʱ��
    unsigned long nightEndVal;              //���Ͻ���ʱ��
    unsigned long curOverTimeVal;           //��ǰ��ʻԱ�ĳ�ʱ��ʻʱ��,��λ��    
    unsigned long curTimeVal;               //��ǰʱ��ʱ��,��λ��   
    unsigned short nightctrl;               //ʱ�������
    //////////////////////////
    unsigned short prePlayAlarmTime;        //Ԥ����ǰʱ��  
    unsigned short PlayAlarmTime;           //��������ʱ��
    /////////////////////////////
    unsigned short ReportInterval;          //�ϱ�ʱ����(��)
    unsigned short AllowRunTime;            //��ʱ��������ʱ��
    /////////////////////////////////
    unsigned short runDuration;             //������ʻ״̬�ĳ���ʱ��
    unsigned short runStatusCnt;            //��ʻ״̬������
    unsigned short stopStatusCnt;           //ֹͣ״̬������
    unsigned char runStatus;                //1Ϊ��ʻ��0Ϊͣʻ
    unsigned char runSpeed;                 //������ʻ״̬���ٶ�    
    ///////////////////////////// ////////////////////////////////////////////
    unsigned char atnightFlag;              //���ϱ�־
    unsigned char needChecknight;           //��Ҫ������ϳ�ʱ
    unsigned char firstRunflag;             //�״ν����ж�
    unsigned char needsaveflag;             //��ʻԱ�ṹ���Ƿ���Ҫ���� 
    unsigned char prespeedVal;              //ǰһ���ٶ�
    unsigned char timeAbnormalFlag;         //ʱ���쳣���
    unsigned char restPlayCnt;              //���Ŵ��� 
    unsigned char restTimeCnt;              //���ŵȴ�ʱ��
    unsigned char needReportflag;           //�Ƿ���Ҫ�ϱ�    
}STTIRED_ATTRIB;//��ʱ��ʻģ�����Խṹ��

static STTIRED_ATTRIB s_stTiredAttrib;
static unsigned char s_ucTiredModeFirstRun=0;
//////////////////////////////
typedef struct
{
    unsigned long endtime;          //ͣ��ʱ��
    unsigned char endPosition[10];  //ͣ��λ����Ϣ
    unsigned char index;            //�����ʻԱ����
    unsigned char bak[1];           //����
}STSTOPINFO;//ͣ����Ϣ�ṹ��
#define  STSTOPINFO_SIZE sizeof(STSTOPINFO)

typedef enum
{
    TIRED_CAR_STATUS_STOP,    //ͣʻ
    TIRED_CAR_STATUS_RUN,     //��ʻ
    TIRED_CAR_STATUS_MAX
}E_T_CAR_STATUS;//������ʻ״̬

enum 
{
    TIRED_TIMER_TASK,                       //
    TIRED_TIMER_PLAY,						//
    TIRED_TIMERS_MAX
}STTIRED_TIME;
static LZM_TIMER s_stTiredTimer[TIRED_TIMERS_MAX];
/********************************��������**********************************/
/*************************************************************
** ��������: TiredDrive_CheckCurrentDriverStatus
** ��������: 
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
void TiredDrive_DisposeOneDriverInfoTimeIsAbnormal(unsigned char index);
void TiredDrive_CheckAndSaveTiredDriverInfo(void);
//********************************��������***********************************
/*************************************************************
** ��������: TiredDrive_SaveFramTimeToDriverInfo
** ��������: 
** ��ڲ���: 
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void TiredDrive_ShowDebugInfo(unsigned char index,unsigned char *str)
{
    #if 0
    TIME_T stTime;
    Gmtime(&stTime,s_stTiredAttrib.curTimeVal);
    LOG_PR("\r\n %s index:%d  ct:%02d:%02d:%02d (%d);st:%d; eT:%d \r\n ",str,index,stTime.hour,stTime.min,stTime.sec,s_stTiredAttrib.curTimeVal,s_stTiredDriver.driverinfo[index].starttime,s_stTiredDriver.driverinfo[index].endtime);
    #endif
}
/*************************************************************
** ��������: TiredDrive_ClearAlltotalDriveTime
** ��������: ��������ۼƳ�ʱʱ��
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
void TiredDrive_ClearAlltotalDriveTime(void)
{
    unsigned char index;
    unsigned long curTime;    
    for(index=0;index<=TIRED_DRIVER_INFO_NUM;index++)
    {
        s_stTiredDriver.driverinfo[index].totalDriveTime = 0;
    }
    /////////////////////
    curTime = s_stTiredAttrib.curTimeVal;
    s_stTiredDriver.clearTime = curTime-(curTime%DAY_SECOND);//����ۼƼ�ʻ������
    s_stTiredAttrib.needsaveflag = 1;
}
/*************************************************************
** ��������: TiredDrive_ClearAlltotalDriveTime
** ��������: ��鼰��������ۼƳ�ʱʱ��
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
void TiredDrive_CheckAndCleartotalDriveTime(void)
{
    if((s_stTiredAttrib.curTimeVal < s_stTiredDriver.clearTime)
     ||(s_stTiredAttrib.curTimeVal>=s_stTiredDriver.clearTime + DAY_SECOND))
    {
        TiredDrive_ClearAlltotalDriveTime();
    }    
}
/*************************************************************
** ��������: TiredDrive_GetCurtotalDriveTime
** ��������: 
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
unsigned long TiredDrive_GetCurtotalDriveTime(unsigned char index)
{
    unsigned long val;
    if(0==s_stTiredDriver.driverinfo[index].validflag)return 0;
    val = s_stTiredDriver.driverinfo[index].starttime;
    if(s_stTiredDriver.driverinfo[index].starttime < s_stTiredDriver.clearTime)
    {
        val = s_stTiredDriver.clearTime;
    }
    ////////////////////
    if(s_stTiredDriver.driverinfo[index].endtime > val)
    {
        val = s_stTiredDriver.driverinfo[index].endtime - val;
    }
    else
    {
        val = 0;
    }
    return val;
}
/*************************************************************
** ��������: TiredDrive_SaveCurtotalDriveTime
** ��������: 
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
void TiredDrive_SaveCurtotalDriveTime(unsigned char index)
{
    if(0==s_stTiredDriver.driverinfo[index].validflag)return;
    if(s_stTiredDriver.driverinfo[index].endtime < s_stTiredDriver.clearTime)return;
    if(s_stTiredDriver.driverinfo[index].starttime > s_stTiredDriver.clearTime + DAY_SECOND)return;
    s_stTiredDriver.driverinfo[index].totalDriveTime += TiredDrive_GetCurtotalDriveTime(index);
}
/*************************************************************
** ��������: TiredDrive_CheckDriverRestStatus
** ��������: 
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
void TiredDrive_ResetCurDriverData(unsigned char index)
{
    TiredDrive_SaveCurtotalDriveTime(index);
    //////////////////////////////////
    s_stTiredDriver.driverinfo[index].validflag         = 0;
    s_stTiredDriver.driverinfo[index].starttime         = 0;
    s_stTiredDriver.driverinfo[index].endtime           = 0;
    s_stTiredDriver.driverinfo[index].overtimeflag      = 0; 
    s_stTiredDriver.driverinfo[index].preReportTime     = 0;
    s_stTiredDriver.driverinfo[index].nightvalidFlag    = 0;   
    s_stTiredDriver.driverinfo[index].nightstarttime    = 0; 
    s_stTiredDriver.driverinfo[index].preovertimeflag   = 0;    
}
/*************************************************************
** ��������: TiredDrive_GetCurRunStatus
** ��������: ��ȡ��ʱ��ʻ��ǰ������״̬(��ʻ��ֹͣ)
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 1Ϊ��ʻ��0Ϊֹͣ
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
unsigned char TiredDrive_GetCurRunStatus(void)
{
    if(0==s_stTiredAttrib.runSpeed)
    {
        return SpeedMonitor_GetCurRunStatus();
    }    
    else
    {
        return(TIRED_CAR_STATUS_RUN==s_stTiredAttrib.runStatus)?1:0;
    }
}
/*************************************************************
** ��������: TiredDrive_CheckRunStatus
** ��������: �жϵ�ǰ�Ǵ�����ʻ״̬����ͣʻ״̬(1s����һ��)
** ��ڲ���: CurrentSpeed�Ѹ���s_stTiredAttrib.runSpeedת��
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
void TiredDrive_CheckRunStatus(unsigned char CurrentSpeed)
{
    //unsigned char ACC;
    if(0==s_stTiredAttrib.runSpeed)return;
    ///////////////////////////
    //ACC = Io_ReadStatusBit(STATUS_BIT_ACC);	
    if(CurrentSpeed)//
    {
        s_stTiredAttrib.stopStatusCnt = 0;//ֹͣ״̬��������0
        if(s_stTiredAttrib.runStatusCnt < s_stTiredAttrib.runDuration)//�Ƿ������10s,����ʱ,���ı䵱ǰ״̬
        {
            s_stTiredAttrib.runStatusCnt++;//��ʻ״̬������++
        }
        else//�Ѿ�����10s,����Ϊ��ʻ״̬
        {
            s_stTiredAttrib.runStatus = TIRED_CAR_STATUS_RUN;
        }
    }
    else
    {
        s_stTiredAttrib.runStatusCnt = 0;//��ʻ״̬��������0
        s_stTiredAttrib.runStatus = TIRED_CAR_STATUS_STOP;
    }
}
/*************************************************************
** ��������: TiredDrive_SaveFramTimeToDriverInfo
** ��������: 
** ��ڲ���: 
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void TiredDrive_SaveFramTimeToDriverInfo(unsigned char index)
{
    unsigned len;
    STSTOPINFO stTmpStopinfo;
    if(0==s_stTiredDriver.driverinfo[index].validflag)return;
    len=Public_ReadDataFromFRAM(FRAM_TIRED_STOP_INFO_ADDR,(unsigned char *)&stTmpStopinfo,STSTOPINFO_SIZE);
    if(STSTOPINFO_SIZE==len)//�µ�
    {
        if(index==stTmpStopinfo.index) 
        {
            s_stTiredDriver.driverinfo[index].endtime = stTmpStopinfo.endtime;
            memcpy(s_stTiredDriver.driverinfo[index].endPosition,stTmpStopinfo.endPosition,10);
        }
    }
}
/*************************************************************
** ��������: TiredDrive_SaveDriverStopInfoToFRAM
** ��������: 
** ��ڲ���: 
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void TiredDrive_SaveDriverStopInfoToFRAM(unsigned char index)
{
    STSTOPINFO stTmpStopinfo;
    if(0==s_stTiredDriver.driverinfo[index].validflag)return;
    stTmpStopinfo.index = index;
    stTmpStopinfo.endtime =s_stTiredDriver.driverinfo[index].endtime;
    memcpy(stTmpStopinfo.endPosition,s_stTiredDriver.driverinfo[index].endPosition,10);
    Public_WriteDataToFRAM(FRAM_TIRED_STOP_INFO_ADDR,(unsigned char *)&stTmpStopinfo,STSTOPINFO_SIZE);
}

/*************************************************************
** ��������: TiredDrive_SaveCurStopInfo
** ��������: 
** ��ڲ���: 
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void TiredDrive_SaveCurStopInfo(unsigned char index,unsigned long currenttime)
{
    s_stTiredDriver.driverinfo[index].endtime = currenttime;
    Public_GetCurPositionInfoDataBlock(s_stTiredDriver.driverinfo[index].endPosition);
    TiredDrive_SaveDriverStopInfoToFRAM(index);
}
/*************************************************************
** ��������: TiredDrive_SaveCurStopInfo
** ��������: 
** ��ڲ���: 
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void TiredDrive_SaveCurStartInfo(unsigned char index,unsigned long currenttime)
{
    s_stTiredDriver.driverinfo[index].starttime = currenttime;
    s_stTiredDriver.driverinfo[index].endtime   = currenttime;
    Public_GetCurPositionInfoDataBlock(s_stTiredDriver.driverinfo[index].startPosition);
}
/*************************************************************
** ��������: TiredDrive_SaveTiredSaveIndexToFRAM
** ��������: 
** ��ڲ���: 
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void TiredDrive_SaveTiredSaveIndexToFRAM(void) 
{
    s_stTiredSaveIndex.savedflag =TIRED_DRIVER_INFO_SAVE_VAL;
    Public_WriteDataToFRAM(FRAM_TIRED_SAVE_INDEX_ADDR,(unsigned char *)&s_stTiredSaveIndex, SIZE_STTIREDSAVEINDEX);
}
/*************************************************************
** ��������: TiredDrive_SaveTiredDriverInfoToFlash
** ��������: �Ѽ�ʻԱ��Ϣ���浽flash
** ��ڲ���: 
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void TiredDrive_SaveTiredDriverInfoToFlash(void) 
{
    unsigned char buffer[10];
    unsigned long val;
    s_stTiredSaveIndex.index++;
    s_stTiredSaveIndex.currentAddr=(s_stTiredSaveIndex.index-1)*SIZE_STTIREDDRIVER;    
    if(s_stTiredSaveIndex.currentAddr+SIZE_STTIREDDRIVER>FLASH_ONE_SECTOR_BYTES)
    {
        s_stTiredSaveIndex.index = 1;
        s_stTiredSaveIndex.currentAddr =0;
    }
    ////////����Ƿ�Ϊ��////////////////////
    sFLASH_ReadBuffer((unsigned char *)&buffer,SAVE_TIRED_DRIVE_INFO_ADDR+s_stTiredSaveIndex.currentAddr,4);
    val=Public_ConvertBufferToLong(buffer);
    if(val != 0xffffffff)//�������Դ洢
    {
        sFLASH_EraseSector(SAVE_TIRED_DRIVE_INFO_ADDR);
    }
    ////////////////////////////
    sFLASH_WriteBuffer((unsigned char *)&s_stTiredDriver,SAVE_TIRED_DRIVE_INFO_ADDR+s_stTiredSaveIndex.currentAddr,SIZE_STTIREDDRIVER);
    ///////////////////
    TiredDrive_SaveTiredSaveIndexToFRAM();
    ///////////////////////////////////////
    //TiredDrive_ShowDebugInfo(s_stTiredDriver.currentindex,"save");
}
/*************************************************************
** ��������: TiredDrive_TiredDriverInfoInit
** ��������: ��ʱ��ʻ��ؼ�ʻԱ��Ϣ��ʼ��
** ��ڲ���: 
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void TiredDrive_TiredDriverInfoInit(void)
{
    s_stTiredSaveIndex.savedflag = TIRED_DRIVER_INFO_SAVE_VAL;
    s_stTiredSaveIndex.index = 0;
    s_stTiredSaveIndex.currentAddr = 0;
    memset(&s_stTiredDriver,0,SIZE_STTIREDDRIVER);
    ///////////////////////////////
    memset(&s_stTiredDriver.driverinfo[TIRED_DRIVER_INFO_NUM].driverLicence,0x30,18);
    s_stTiredAttrib.needsaveflag = 1;
    TiredDrive_CheckAndSaveTiredDriverInfo();
}
/*************************************************************
** ��������: TiredDrive_ReadTiredDriverInfoFromFlash
** ��������: 
** ��ڲ���: 
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void TiredDrive_ReadTiredDriverInfoFromFlash(void) 
{
    unsigned char flag;
    /////////////////////////////
    flag = Public_ReadDataFromFRAM(FRAM_TIRED_SAVE_INDEX_ADDR,(unsigned char *)&s_stTiredSaveIndex, SIZE_STTIREDSAVEINDEX);
    if((s_stTiredSaveIndex.savedflag != TIRED_DRIVER_INFO_SAVE_VAL)||(s_stTiredSaveIndex.currentAddr > FLASH_ONE_SECTOR_BYTES)
        ||(s_stTiredSaveIndex.currentAddr%SIZE_STTIREDDRIVER != 0)||(0==flag))
    {
        TiredDrive_TiredDriverInfoInit();
    }    
    else
    {
        sFLASH_ReadBuffer((unsigned char *)&s_stTiredDriver,SAVE_TIRED_DRIVE_INFO_ADDR+s_stTiredSaveIndex.currentAddr,SIZE_STTIREDDRIVER);
        ///////////�Ϸ��ж�////////////
        if((s_stTiredDriver.loginindex>TIRED_DRIVER_INFO_NUM)||(s_stTiredDriver.currentindex>TIRED_DRIVER_INFO_NUM)||(s_stTiredDriver.driverloginflag>1)
         ||(s_stTiredDriver.driverloginflag&&(0==s_stTiredDriver.loginindex||s_stTiredDriver.loginindex!=s_stTiredDriver.currentindex+1))
         ||(0==s_stTiredDriver.driverloginflag&&(s_stTiredDriver.loginindex==s_stTiredDriver.currentindex+1||s_stTiredDriver.currentindex!=TIRED_DRIVER_INFO_NUM)))
        {
            memset(&s_stTiredDriver,0,SIZE_STTIREDDRIVER);
            /////////////////////////////
            memset(&s_stTiredDriver.driverinfo[TIRED_DRIVER_INFO_NUM].driverLicence,0x30,18);
        }
        else/////////
        {
            if((0==s_stTiredDriver.driverloginflag&&TIRED_DRIVER_INFO_NUM==s_stTiredDriver.currentindex)
              ||(s_stTiredDriver.driverloginflag&&s_stTiredDriver.loginindex==s_stTiredDriver.currentindex+1))
            TiredDrive_SaveFramTimeToDriverInfo(s_stTiredDriver.currentindex);
        }
    }
    //////////////////////////////
    s_stTiredAttrib.firstRunflag = 0;//
    if(s_stTiredDriver.driverloginflag)
    {
        Io_WriteSelfDefine2Bit(DEFINE_BIT_5,SET);
    }
}
/*************************************************************
** ��������: TiredDrive_CheckTimeAtNight
** ��������: �ж�ʱ���Ƿ�Ϊ����ʱ���
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
unsigned char TiredDrive_CheckTimeAtNight(unsigned long checkTime)
{
    unsigned char atnightFlag;
    if(!s_stTiredAttrib.needChecknight)return 0;//����Ҫ������ϳ�ʱ
    atnightFlag =Public_CheckTimeIsInTimeRange(s_stTiredAttrib.nightctrl,checkTime,s_stTiredAttrib.nightStartVal,s_stTiredAttrib.nightEndVal);    
    return atnightFlag;
}
/*************************************************************
** ��������: TiredDrive_ClearDriverInfo
** ��������: ����洢��ʻԱ��Ϣ�еļ�ʻԱ�ṹ��
** ��ڲ���: ��ʻԱ�ṹ��������(��0��ʼ)
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
void TiredDrive_ClearDriverInfo(unsigned char index)
{
    TiredDrive_ResetCurDriverData(index);
    #if 0
    memset(&s_stTiredDriver.driverinfo[index],0,sizeof(STTIREDDRIVERINFO));
    if(TIRED_DRIVER_INFO_NUM == index)//
    {
        memset(&s_stTiredDriver.driverinfo[index].driverLicence,0x30,18);
    }
    #endif
}
/*************************************************************
** ��������: TiredDrive_GetDriverInfoOverTime
** ��������: ���ָ����ʻԱ�ĳ�ʱʱ���
** ��ڲ���: index��ʻԱ������
** ���ڲ���: 
** ���ز���: ��ʱʱ���
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
unsigned long TiredDrive_GetDriverInfoOverTime(unsigned char index)
{
    unsigned long overTimeVal,tmpVal1;
    if(1 != s_stTiredDriver.driverinfo[index].validflag)return 0;
    if(0 == s_stTiredDriver.driverinfo[index].starttime)return 0;
    if(0xffffffff == s_stTiredAttrib.overTimeVal)return 0;
    overTimeVal = s_stTiredDriver.driverinfo[index].starttime+s_stTiredAttrib.overTimeVal;
    if(s_stTiredDriver.driverinfo[index].nightvalidFlag&&s_stTiredAttrib.needChecknight)
    {
        if(TiredDrive_CheckTimeAtNight(s_stTiredDriver.driverinfo[index].nightstarttime+s_stTiredAttrib.nightoverTimeVal))
        {
            tmpVal1=s_stTiredDriver.driverinfo[index].nightstarttime+s_stTiredAttrib.nightoverTimeVal;
            if(tmpVal1 < overTimeVal)
            {
                overTimeVal = tmpVal1;
            }
        }
    }
    //////////////////////////////
    return overTimeVal;//��Сֵ
}
/*************************************************************
** ��������: TiredDrive_CheckDriverIsTired
** ��������: �ж�ָ����ʻԱ�Ƿ��ѳ�ʱ
** ��ڲ���: index��ʻԱ������
** ���ڲ���: 
** ���ز���: 1:�ѳ�ʱ,0:δ��ʱ
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
unsigned char TiredDrive_CheckDriverIsTired(unsigned char index)
{
    unsigned long overtime;
    if(1==s_stTiredDriver.driverinfo[index].validflag)
    {
        overtime=TiredDrive_GetDriverInfoOverTime(index);
        if(overtime&&s_stTiredDriver.driverinfo[index].endtime>=overtime)
        {
            return 1;
        }
    }
    return 0;
}
/*************************************************************
** ��������: TiredDrive_GetOneTiredDriveFromFram
** ��������: ���������ȡһ����ʱ��ʻ��¼(���м�ʻԱ)
** ��ڲ���: 
** ���ڲ���: 
** ���ز���:
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
unsigned char TiredDrive_GetOneTiredDriveFromFram(unsigned char *pBuffer,unsigned char index)
{
    unsigned char buffer1[20];
    unsigned short len;
    unsigned char temp;
    TIME_T tt;
    
    len = 0;
    temp =18;
    memcpy(&pBuffer[len],s_stTiredDriver.driverinfo[index].driverLicence,temp);
    len +=temp;
    //////////
    temp = 6;
    Gmtime(&tt, s_stTiredDriver.driverinfo[index].starttime);
    Public_ConvertTimeToBCDEx(tt,buffer1);
    memcpy(&pBuffer[len],buffer1,temp);
    len += temp;
    Gmtime(&tt, s_stTiredDriver.driverinfo[index].endtime);
    Public_ConvertTimeToBCDEx(tt,buffer1);
    memcpy(&pBuffer[len],buffer1,temp);
    len += temp;
    //////////////////////
    temp = 10;
    memcpy(&pBuffer[len],s_stTiredDriver.driverinfo[index].startPosition,temp);
    len +=temp;
    memcpy(&pBuffer[len],s_stTiredDriver.driverinfo[index].endPosition,temp);
    len +=temp;
    ////////////////////
    return ONE_TIRED_DRIVER_DATA_LEN;
}
/*************************************************************
** ��������: TiredDrive_ReadTiredDriveFromFram
** ��������: ���������ȡ��ʱ��ʻ��¼(���м�ʻԱ)
** ��ڲ���: 
** ���ڲ���: �������ݵ��׵�ַ
** ���ز���: pBuffer�ֽڳ���
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
unsigned short TiredDrive_ReadTiredDriveFromFram(unsigned char *pBuffer)
{
    unsigned char i;
    unsigned short len;
    unsigned char *p;
    p = pBuffer;
    //****************�����������*****************
    len =0;
    for(i=0; i<=TIRED_DRIVER_INFO_NUM;i++)
    {
        if(1==s_stTiredDriver.driverinfo[i].validflag)
        {
            if(TiredDrive_CheckDriverIsTired(i))
            {
                len += TiredDrive_GetOneTiredDriveFromFram(&p[len],i);
            }
        }
    }
    return len;        
}
/*********************************************************************
//��������	:TiredDrive_WriteTiredRecordToFlash
//����		:�洢һ����ʱ��ʻ��¼��Flash
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
*********************************************************************/
void TiredDrive_WriteTiredRecordToFlash(unsigned char *pBuffer, unsigned char length,unsigned long endtime)
{
    Register_Write2(REGISTER_TYPE_OVER_TIME,pBuffer,length,endtime);
}
/*************************************************************
** ��������: TiredDrive_SaveOvertimeDriverRecordToFlash
** ��������: ���泬ʱ��ʻ��¼��Flash
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
void TiredDrive_SaveOvertimeDriverRecordToFlash(unsigned char index)
{
    unsigned char buffer[SAVE_ONE_TIRED_DRIVER_DATA_LEN+1]={0};
    if(index<=TIRED_DRIVER_INFO_NUM)//
    {
        TiredDrive_GetOneTiredDriveFromFram(buffer,index);        
        TiredDrive_WriteTiredRecordToFlash(buffer,SAVE_ONE_TIRED_DRIVER_DATA_LEN,s_stTiredDriver.driverinfo[index].endtime);
    }    
}
/*************************************************************
** ��������: TiredDrive_GetCurRestTimeVal
** ��������: ȡ�õ�ǰ��С��Ϣʱ��
** ��ڲ���: endtime:ͣ��ʱ���
** ���ڲ���: 
** ���ز���: ��С��Ϣʱ��
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
unsigned long TiredDrive_GetCurRestTimeVal(unsigned long endtime)
{
    static unsigned long sulEndtime=0;
    static unsigned long restTimeVal=0;    
    if(0==s_stTiredAttrib.needChecknight)return s_stTiredAttrib.restTimeVal;
    if(s_stTiredAttrib.nightrestTimeVal == s_stTiredAttrib.restTimeVal)return s_stTiredAttrib.restTimeVal;
    if(sulEndtime == endtime)return restTimeVal;
    if(Public_CheckTimeIsInTimeRange(s_stTiredAttrib.nightctrl,endtime,s_stTiredAttrib.nightStartVal,s_stTiredAttrib.nightEndVal))
    {
        restTimeVal = s_stTiredAttrib.nightrestTimeVal;
    }
    else
    {
        restTimeVal = s_stTiredAttrib.restTimeVal;
    }
    sulEndtime = endtime;
    return restTimeVal;
}
/*************************************************************
** ��������: TiredDrive_CheckDriverRestStatus
** ��������: �ж��Ƿ��Ѵﵽ��С��Ϣʱ��
** ��ڲ���: currenttime:��Ҫ�жϵ�ʱ��㣬index:��ʻԱ������
** ���ڲ���: 
** ���ز���: 1:�Ѵﵽ,0:δ�ﵽ
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
unsigned char TiredDrive_CheckDriverRestStatus(unsigned long currenttime,unsigned char index)
{
    unsigned long overtime,restTimeVal;
    if((currenttime+10<s_stTiredDriver.driverinfo[index].endtime)||(s_stTiredDriver.driverinfo[index].endtime+10 < s_stTiredDriver.driverinfo[index].starttime))
    {
        return 1;//ʱ���ʽ����
    }
    else
    {
        restTimeVal = TiredDrive_GetCurRestTimeVal(s_stTiredDriver.driverinfo[index].endtime);
        if(currenttime>=s_stTiredDriver.driverinfo[index].endtime+restTimeVal)//������Ϣʱ��
        {
            overtime = TiredDrive_GetDriverInfoOverTime(index);
            if(overtime&&s_stTiredDriver.driverinfo[index].endtime >= overtime)
            {
                ///////////���泬ʱ��ʻ��¼//////////////////
                //��ΪRTC���û�絼����GPSʱ��У׼֮ǰ�����ʱ����ҵ����⣬������û��GPSУʱ֮ǰ���洢��ʻ��¼�����ݣ���Ҳ��û�а취�İ취....
								if(TiredDriveSaveFlg)
								{
                	TiredDrive_SaveOvertimeDriverRecordToFlash(index);
								}
            }
            return 1;
        }
    }
    return 0;
}
/*************************************************************
** ��������: TiredDrive_ReadTiredDriveFromFramEx
** ��������: ���������ȡָ��ʱ��γ�ʱ��ʻ��¼(���м�ʻԱ)
** ��ڲ���: ��ʼʱ��StartTime,����ʱ��EndTime
** ���ڲ���: pBuffer�������ݵ��׵�ַ
** ���ز���: pBuffer�ֽڳ���
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
unsigned short TiredDrive_ReadTiredDriveFromFramEx(unsigned char *pBuffer,TIME_T StartTime,TIME_T EndTime)
{
    unsigned long startVal,endVal;
    unsigned char i;
    unsigned short len;
    unsigned char *p;
    p = pBuffer;
    startVal = ConverseGmtime(&StartTime);
    endVal = ConverseGmtime(&EndTime);
    //****************�����������*****************
    len =0;
    for(i=0; i<=TIRED_DRIVER_INFO_NUM;i++)
    {
        if(1==s_stTiredDriver.driverinfo[i].validflag)
        {
            if(TiredDrive_CheckDriverIsTired(i))
            {
                if((startVal<=s_stTiredDriver.driverinfo[i].starttime&&endVal>s_stTiredDriver.driverinfo[i].starttime)
                  ||(startVal<s_stTiredDriver.driverinfo[i].endtime&&endVal>=s_stTiredDriver.driverinfo[i].endtime)
                  ||(startVal>s_stTiredDriver.driverinfo[i].starttime&&endVal<s_stTiredDriver.driverinfo[i].endtime))
                {
                    len += TiredDrive_GetOneTiredDriveFromFram(&p[len],i);
                }
            }
        }
    }
    return len;        
}
/*********************************************************************
** ��������: TiredDrive_UpdatePram
** ��������: ���ó�ʱ��ʻ����ʱ����ʼ����ʱ��ʻ����
** ��ڲ���: 
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void TiredDrive_UpdatePram(void)
{
    unsigned char Buffer[50] = {0};
    unsigned char len;
    unsigned long tmpVal;
    ////////////////////////////  
    s_stTiredAttrib.firstRunflag =0;
    //////////////////
    len = EepromPram_ReadPram(E2_MAX_DRVIE_TIME_ID, Buffer);
    if(E2_MAX_DRVIE_TIME_LEN == len)
    {
        s_stTiredAttrib.overTimeVal=Public_ConvertBufferToLong(Buffer);
    }
    else
    {
        s_stTiredAttrib.overTimeVal = 14400;//����,����ΪĬ��ֵ,240����
    }
    len = EepromPram_ReadPram(E2_MIN_RELAX_TIME_ID, Buffer);
    if(E2_MIN_RELAX_TIME_LEN == len)
    {
        s_stTiredAttrib.restTimeVal=Public_ConvertBufferToLong(Buffer);
    }
    else
    {
        s_stTiredAttrib.restTimeVal = 1200;//����,����ΪĬ��ֵ,20����
    }
    /////�����ۼƼ�ʻʱ������//��λΪ��//Ĭ��Ϊ0xffffffff,�������ù���///////////
    len = EepromPram_ReadPram(E2_MAX_DRVIE_TIME_ONEDAY_ID, Buffer);
    if(E2_MAX_DRVIE_TIME_ONEDAY_LEN == len)
    {
        s_stTiredAttrib.oneDayoverTimeVal = Public_ConvertBufferToLong(Buffer);
    }
    else
    {
        s_stTiredAttrib.oneDayoverTimeVal = 0xffffffff;//
    }   
    /////////ҹ�䳬ʱ��ʻ����////////////////////////
    s_stTiredAttrib.nightStartVal = 0;
    s_stTiredAttrib.nightEndVal = 0;    
    s_stTiredAttrib.needChecknight = 0;
    s_stTiredAttrib.nightrestTimeVal = 0;
    s_stTiredAttrib.nightoverTimeVal = 0;    
    len =EepromPram_ReadPram(E2_NIGHT_MAX_DRVIE_TIME_ID, Buffer);
    if(E2_NIGHT_MAX_DRVIE_TIME_ID_LEN== len)
    {
        tmpVal=Public_ConvertBufferToLong(Buffer);
        if((0xffffffff!=tmpVal)&&(tmpVal<s_stTiredAttrib.overTimeVal))//ҹ�䳬ʱ��ʻ������һ���Ȱ��������ֵС
        {
            s_stTiredAttrib.nightoverTimeVal = tmpVal;
        }
    }
    ////////////////////////////////
    len = EepromPram_ReadPram(E2_NIGHT_MIN_RELAX_TIME_ID, Buffer);
    if(E2_NIGHT_MIN_RELAX_TIME_ID_LEN == len)
    {
        s_stTiredAttrib.nightrestTimeVal=Public_ConvertBufferToLong(Buffer);
    }
    else
    {
        s_stTiredAttrib.nightrestTimeVal = 1200;//����,����ΪĬ��ֵ,20����
    }
    ////////ҹ��ʱ�䷶Χ,BCD��//////////////////////
    len =EepromPram_ReadPram(E2_TIRED_DRIVE_NIGHT_TIME_ID, Buffer);
    if(len)
    {
        s_stTiredAttrib.nightctrl = Public_ConvertBufferToShort(Buffer);        
        s_stTiredAttrib.nightStartVal=Public_ConvertBCDTimeToLong(s_stTiredAttrib.nightctrl,&Buffer[2]);
        if(s_stTiredAttrib.nightStartVal)
        {
            s_stTiredAttrib.nightStartSec = 0;
            s_stTiredAttrib.nightStartSec = s_stTiredAttrib.nightStartSec*24 + Public_BCD2HEX(Buffer[5]); //���ʱ
            s_stTiredAttrib.nightStartSec = s_stTiredAttrib.nightStartSec*60 + Public_BCD2HEX(Buffer[6]); //��÷�
            s_stTiredAttrib.nightStartSec = s_stTiredAttrib.nightStartSec*60 + Public_BCD2HEX(Buffer[7]); //�����
            ////////////////////////////////////////////////////
            s_stTiredAttrib.nightEndVal=Public_ConvertBCDTimeToLong(s_stTiredAttrib.nightctrl,&Buffer[8]);            
            if(0==s_stTiredAttrib.nightEndVal)
            {
                s_stTiredAttrib.nightStartVal = 0;
            }
        }
    }    
    /////////////////////////////////
    if(s_stTiredAttrib.nightoverTimeVal&&Public_CheckTimeRangeIsValid(s_stTiredAttrib.nightctrl,s_stTiredAttrib.nightStartVal,s_stTiredAttrib.nightEndVal))
    {
        s_stTiredAttrib.needChecknight = 1;
    } 
    ///////ƣ�ͼ�ʻԤ����ֵ/////////////////
    len =EepromPram_ReadPram(E2_TIRE_EARLY_ALARM_DVALUE_ID, Buffer);
    if(E2_TIRE_EARLY_ALARM_DVALUE_LEN== len)
    {
        s_stTiredAttrib.prePlayAlarmTime = Public_ConvertBufferToShort(Buffer);
    }
    else
    {
        s_stTiredAttrib.prePlayAlarmTime = 1800;
    }
    ///////////��ʱ��ʻ������ʾ������Чʱ��,
    len=EepromPram_ReadPram(E2_TIRED_DRIVE_ALARM_TIME_ID, Buffer);
    if(len)
    {
        s_stTiredAttrib.PlayAlarmTime = Public_ConvertBufferToLong(Buffer)*60;
    }
    else
    {
        s_stTiredAttrib.PlayAlarmTime = 1800;
    }
    ////////Ԥ��/////��ʱ��ʻԤ��ÿ����ʾʱ����////////////////// 
    len=EepromPram_ReadPram(E2_TIRED_DRIVE_PREALARM_GPOUP_TIME_ID, Buffer);
    if(len)
    {
        s_stTiredAttrib.stPrePlay.GroupTime = Public_ConvertBufferToLong(Buffer)*60;
    }
    else
    {
        s_stTiredAttrib.stPrePlay.GroupTime = 300;
    }
    
    len=EepromPram_ReadPram(E2_TIRED_DRIVE_PREALARM_NUMBER_ID, Buffer);
    if(len)
    {
        s_stTiredAttrib.stPrePlay.OnceNumber = Public_ConvertBufferToLong(Buffer);
    }
    else
    {
        s_stTiredAttrib.stPrePlay.OnceNumber = 3;
    }
    len=EepromPram_ReadPram(E2_TIRED_DRIVE_PREALARM_NUMBER_TIME_ID, Buffer);
    if(len)
    {
        s_stTiredAttrib.stPrePlay.OnceInterval = Public_ConvertBufferToLong(Buffer);
    }
    else
    {
        s_stTiredAttrib.stPrePlay.OnceInterval = 10;
    }
    ////////////��ʱ////////////////////////////
    len=EepromPram_ReadPram(E2_TIRED_DRIVE_ALARM_GPOUP_TIME_ID, Buffer);
    if(len)
    {
        s_stTiredAttrib.stPlay.GroupTime = Public_ConvertBufferToLong(Buffer)*60;
    }
    else
    {
        s_stTiredAttrib.stPlay.GroupTime = 300;
    }
    len=EepromPram_ReadPram(E2_TIRED_DRIVE_ALARM_NUMBER_ID, Buffer);
    if(len)
    {
        s_stTiredAttrib.stPlay.OnceNumber = Public_ConvertBufferToLong(Buffer);
    }
    else
    {
        s_stTiredAttrib.stPlay.OnceNumber = 3;
    }    
    len=EepromPram_ReadPram(E2_TIRED_DRIVE_ALARM_NUMBER_TIME_ID, Buffer);
    if(len)
    {
        s_stTiredAttrib.stPlay.OnceInterval = Public_ConvertBufferToLong(Buffer);
    }
    else
    {
        s_stTiredAttrib.stPlay.OnceInterval = 10;
    }
    len=EepromPram_ReadPram(E2_TIRED_DRIVE_RUN_SPEED_ID, Buffer);
    if(len)
    {
        s_stTiredAttrib.runSpeed = Public_ConvertBufferToLong(Buffer);
    }
    else
    {
        s_stTiredAttrib.runSpeed = 0;
    }
    len=EepromPram_ReadPram(E2_TIRED_DRIVE_RUN_KEEPTIME_ID, Buffer);
    if(len)
    {
        s_stTiredAttrib.runDuration = Public_ConvertBufferToLong(Buffer);
    }
    else
    {
        s_stTiredAttrib.runDuration = 10;
    }
    //////��ʱ��ʻ����λ���ϱ�ʱ����////////////////////////////////
    len=EepromPram_ReadPram(E2_TIRED_DRIVE_REPORT_TIME_ID,Buffer);
    if(len == E2_TIRED_DRIVE_REPORT_TIME_ID_LEN)
    {
        s_stTiredAttrib.ReportInterval = Public_ConvertBufferToLong(Buffer);
    }
    else
    {
        s_stTiredAttrib.ReportInterval = 0;
    }
    /////��ʱ��������ʱ��/////////
    len = EepromPram_ReadPram(E2_TIRED_DRIVE_REPORT_OPEN_TIME_ID,Buffer);
    if(len == E2_TIRED_DRIVE_REPORT_OPEN_TIME_ID_LEN)
    {
        s_stTiredAttrib.AllowRunTime = Public_ConvertBufferToLong(Buffer);
    }
    else
    {
        s_stTiredAttrib.AllowRunTime = 1200;//20����
    }
    //////////////////////////////////////
    if(0xffffffff==s_stTiredAttrib.oneDayoverTimeVal)///�����
    {
        Io_WriteAlarmBit(ALARM_BIT_DAY_OVER_DRIVE, RESET);
    }
}

/*************************************************************
** ��������: TiredDrive_Login
** ��������: ��ʻԱ��¼ʱ����(��ʼ����ر���)
** ��ڲ���: 
** ���ڲ���: ��
** ���ز���: 1:��¼�ɹ�,0:��¼ʧ��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
unsigned char TiredDrive_Login(void)
{
    unsigned char Buffer[21] = {0};
    unsigned char i,index,flag,loginsucceed;
    if(0==s_ucTiredModeFirstRun)//�ڵ�¼֮ǰһ��Ҫ�ȳ�ʼ��ģ��Ĳ���
    {
        TiredDrive_ParamInitialize();
    }
    //����ǰ��¼�ļ�ʻ֤����
    flag = 0;
    loginsucceed =0;
    EepromPram_ReadPram(E2_CURRENT_LICENSE_ID, Buffer);
    /////////����ǰһ��//////////////////////////
    TiredDrive_SaveFramTimeToDriverInfo(s_stTiredDriver.currentindex);
    /////////////////////////////////
    for(i=0; i< TIRED_DRIVER_INFO_NUM; i++)
    {
        if(1 != s_stTiredDriver.driverinfo[i].validflag)//����������ڼ�ʱ,�����
        {
            TiredDrive_ClearDriverInfo(i);
        }
        ////////////////
        //if(1 == s_stTiredDriver.driverinfo[i].validflag))//�ڼ�ʱ��Ч�в���
        if(s_stTiredDriver.driverinfo[i].totalDriveTime||(1 == s_stTiredDriver.driverinfo[i].validflag))//�ҵ�����
        {
            if(strncmp((char *)Buffer,(char *)&s_stTiredDriver.driverinfo[i].driverLicence[0],18)==0)
            {
                flag = 1;
                s_stTiredDriver.loginindex = i+1;
            }
        }
    } 
    /////////////////////////////
    if(0==flag)//û�ҵ���ͬ�ļ�ʻԱ
    {
        for(i=0; i< TIRED_DRIVER_INFO_NUM; i++)
        { 
            //if(0 == s_stTiredDriver.driverinfo[i].validflag)
            if(0 == s_stTiredDriver.driverinfo[i].totalDriveTime&&0==s_stTiredDriver.driverinfo[i].validflag)
            {
                loginsucceed = 1;
                s_stTiredDriver.loginindex = i+1;//�ҵ���λ 
                memcpy((char *)s_stTiredDriver.driverinfo[i].driverLicence,(char *)Buffer,18);                
                //EepromPram_ReadPram(E2_CURRENT_DRIVER_ID, s_stTiredDriver.driverinfo[i].drivecode);
                break;
            }
        }
    }
    else
    if(1==flag)//�ҵ���ͬ�ļ�ʻԱ����
    {
        loginsucceed = 1;
        index = s_stTiredDriver.loginindex-1;
        TiredDrive_SaveDriverStopInfoToFRAM(index);
        /////////////////////////////////////
        //EepromPram_ReadPram(E2_CURRENT_DRIVER_ID, s_stTiredDriver.driverinfo[index].drivecode);
    }
    ////////////////////
    if(0==s_stTiredDriver.driverinfo[TIRED_DRIVER_INFO_NUM].validflag)
    TiredDrive_ClearDriverInfo(TIRED_DRIVER_INFO_NUM);
    ////////////////////////////////////
    memset(&s_stTiredDriver.driverinfo[TIRED_DRIVER_INFO_NUM].driverLicence,0x30,18);
    ///////////////////    
    s_stTiredAttrib.firstRunflag    = 0;
    s_stTiredDriver.driverloginflag = 1; 
    s_stTiredAttrib.needsaveflag    = 1;
    s_stTiredDriver.currentindex    = s_stTiredDriver.loginindex-1;
    ///////////////////////// 
    Io_WriteSelfDefine2Bit(DEFINE_BIT_5,SET);
    /////////////////////////
    return loginsucceed;
    //TiredDrive_ShowDebugInfo(s_stTiredDriver.loginindex-1,"login");
}
/*************************************************************
** ��������: TiredDrive_Logout
** ��������: :��ʻԱǩ��ʱ����
** ��ڲ���: 
** ���ڲ���: ��
** ���ز���: 1:ǩ�˳ɹ�,0:ǩ��ʧ��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
unsigned char TiredDrive_Logout(void)
{
    ///////////////////////
    if(0==s_ucTiredModeFirstRun)//�ڵ�¼֮ǰһ��Ҫ�ȳ�ʼ��ģ��Ĳ���
    {
        TiredDrive_ParamInitialize();
    }
    ////////////////////////
    TiredDrive_SaveFramTimeToDriverInfo(s_stTiredDriver.currentindex);   
    ////////////////////////////////
    TiredDrive_SaveDriverStopInfoToFRAM(TIRED_DRIVER_INFO_NUM);
    ///////////////////////////
    s_stTiredDriver.driverloginflag = 0;
    s_stTiredAttrib.firstRunflag    = 0;    
    s_stTiredAttrib.needsaveflag    = 1;
    /////////////////////
    Io_WriteSelfDefine2Bit(DEFINE_BIT_5,RESET);
    /////////////////
    return 1;
    ///////////////////////////////////////////
    //TiredDrive_ShowDebugInfo(TIRED_DRIVER_INFO_NUM,"Logout");    
}
/*************************************************************
** ��������: TiredDrive_GetCurrentRunOverTime
** ��������: 
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
void TiredDrive_GetCurrentRunOverTime(unsigned char index)
{
    if(0xffffffff == s_stTiredAttrib.overTimeVal)
    {
        s_stTiredAttrib.curOverTimeVal =0xffffffff;
    }
    else
    {
        s_stTiredAttrib.curOverTimeVal = TiredDrive_GetDriverInfoOverTime(index);//��Сֵ
    }
    ////////////////////
    //TiredDrive_ShowDebugInfo(index,"Current");
}
/*************************************************************
** ��������: TiredDrive_CheckDriverRunStatus
** ��������: 
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
unsigned char TiredDrive_CheckAndSaveNightstarttime(unsigned char index,unsigned long currenttime)
{
    if(!s_stTiredAttrib.needChecknight)return 0;
    if(!s_stTiredAttrib.atnightFlag)return 0;    
    if(s_stTiredDriver.driverinfo[index].nightvalidFlag)return 0;
    s_stTiredDriver.driverinfo[index].nightvalidFlag = 1;
    s_stTiredDriver.driverinfo[index].nightstarttime = currenttime;
    return 1;
}
/*************************************************************
** ��������: TiredDrive_OverTimeAlarmVoice
** ��������: 
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
void TiredDrive_RestTimePlayVoice(void)
{
    if(0==TTS_GetPlayBusyFlag()||(0==s_stTiredAttrib.restPlayCnt))
    {
        if((0==s_stTiredAttrib.restPlayCnt)
         ||(s_stTiredAttrib.restPlayCnt&&0==Io_ReadAlarmBit(ALARM_BIT_TIRED_DRIVE_PRE_ALARM)))
        {       
            s_stTiredAttrib.restPlayCnt++;
            s_stTiredAttrib.restTimeCnt = 0;
            Public_PlayTTSVoiceStr("������Ϣ��ָ��ʱ��,���Լ�����ʻ"); 
            if(s_stTiredAttrib.restPlayCnt<3)
            {                           
                LZM_PublicSetOnceTimer(&s_stTiredTimer[TIRED_TIMER_PLAY],PUBLICSECS(5),TiredDrive_RestTimePlayVoice); 
                return;
            }
        }
    }
    else  
    if(s_stTiredAttrib.restTimeCnt<60)
    {
        s_stTiredAttrib.restTimeCnt++;
        LZM_PublicSetOnceTimer(&s_stTiredTimer[TIRED_TIMER_PLAY],PUBLICSECS(1),TiredDrive_RestTimePlayVoice);             
        return;
    }
    //////////////////////////
    s_stTiredAttrib.restPlayCnt = 0;
    s_stTiredAttrib.restTimeCnt = 0;
}
/*************************************************************
** ��������: TiredDrive_CheckRestTimePlayVoice
** ��������: 
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
void TiredDrive_CheckRestTimePlayVoice(void)
{
    if(Io_ReadAlarmBit(ALARM_BIT_TIRED_DRIVE_PRE_ALARM)||Io_ReadAlarmBit(ALARM_BIT_TIRED_DRIVE))
    {
        s_stTiredAttrib.restPlayCnt = 0;
        s_stTiredAttrib.restTimeCnt = 0;
        TiredDrive_RestTimePlayVoice();
    }
}
/*************************************************************
** ��������: TiredDrive_ConvertSpeedAndRunStatus
** ��������: ת���ٶȲ��ж���ʻ״̬,�����жϿ����㡢ͣ����
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: ����ٶ�
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
unsigned char TiredDrive_ConvertSpeedAndRunStatus(void)
{
    unsigned char curSpeedVal;
    curSpeedVal = SpeedMonitor_GetCurSpeed();
    if(0==s_stTiredAttrib.runSpeed)return curSpeedVal;
    /////////////////////////////////
    if(curSpeedVal>=s_stTiredAttrib.runSpeed)
    {
        curSpeedVal = curSpeedVal+1-s_stTiredAttrib.runSpeed;
    }
    else
    {
        curSpeedVal = 0;
    }
    /////////////�ж���ʻ״̬/////////////////////
    TiredDrive_CheckRunStatus(curSpeedVal);
    ///////////////////////////////////
    return curSpeedVal;
}
/*************************************************************
** ��������: TiredDrive_CheckDriverRunStatus
** ��������: 
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
void TiredDrive_CheckDriverRunStatus(unsigned long currentTime,unsigned char index)
{
    static unsigned char sucRunStatusCnt = 0;//д��ʻʱ����,10sдһ��  
    unsigned char RunStatus;
    unsigned char curSpeedVal;
    curSpeedVal = TiredDrive_ConvertSpeedAndRunStatus();   
    //////////////////////////////////////
    RunStatus = TiredDrive_GetCurRunStatus();
    /////////////////////////////////////    
    s_stTiredDriver.currentindex = index;
    if(1 == s_stTiredDriver.driverinfo[index].validflag)
    {
        if(0 == s_stTiredAttrib.firstRunflag)////��������֮���һ������//////////////
        {
            sucRunStatusCnt = 0;
            s_stTiredDriver.driverinfo[index].overtimeflag = 0; 
            if(TiredDrive_CheckDriverRestStatus(currentTime,index))//������Ϣʱ��
            {
                TiredDrive_ResetCurDriverData(index);
                s_stTiredAttrib.needsaveflag = 1;//��Ҫ����
            }
            else 
            if(TiredDrive_CheckDriverIsTired(index))
            {
                s_stTiredDriver.driverinfo[index].preovertimeflag = 0;
                s_stTiredDriver.driverinfo[index].overtimeflag = 1;
            }
            ////////////////////////////////////
            TiredDrive_GetCurrentRunOverTime(index);
        }
        else
        if(1 ==RunStatus)//��ʻ��
        {
            ///////////�ж��Ƿ����ϼ�ʱ��Ч///////////////////////////////////
            if(TiredDrive_CheckAndSaveNightstarttime(index,currentTime))
            {
                TiredDrive_GetCurrentRunOverTime(index);
                s_stTiredAttrib.needsaveflag = 1;//��Ҫ����
            }
            ///////////////////////////////////////////////
            if(0 == s_stTiredDriver.driverinfo[index].overtimeflag)//δ��ʱ��ʻ
            {
                if(currentTime >= s_stTiredAttrib.curOverTimeVal)//
                {
                    s_stTiredDriver.driverinfo[index].preovertimeflag = 0;
                    s_stTiredDriver.driverinfo[index].overtimeflag = 1;
                    s_stTiredAttrib.needsaveflag = 1;//��Ҫ����
                }
                else
                if((currentTime+s_stTiredAttrib.prePlayAlarmTime)>=s_stTiredAttrib.curOverTimeVal)//
                {
                    if(0==s_stTiredDriver.driverinfo[index].preovertimeflag)
                    {
                        s_stTiredDriver.driverinfo[index].preovertimeflag = 1;
                        s_stTiredAttrib.needsaveflag = 1;//��Ҫ����
                    }
                }
                else
                {
                    s_stTiredDriver.driverinfo[index].preovertimeflag = 0; 
                }
            }
            else////��ʱ��ʻ
            {
                sucRunStatusCnt +=2;//���¼������ӿ�
            }
            ////////////////
            sucRunStatusCnt++;
            //����FRAM�е�ͣ��ʱ�估ͣ��λ��,
            if(0 == curSpeedVal && s_stTiredAttrib.prespeedVal)////���ٶȲ�Ϊ�㵽Ϊ��
            {
                sucRunStatusCnt = 0;
                TiredDrive_SaveCurStopInfo(index,currentTime);
            }
            else
            if(sucRunStatusCnt >= 10&&curSpeedVal)//ʱ������0
            {
                sucRunStatusCnt = 0; 
                TiredDrive_SaveCurStopInfo(index,currentTime);
            }
            else
            if(curSpeedVal)//��ǰ���ٶȵ�ʱ��ű���
            {
                s_stTiredDriver.driverinfo[index].endtime = currentTime;//ʵʱ����ͣ��ʱ��
            }
        }
        else//ͣʻ��
        {
            sucRunStatusCnt = 8;
            //////////////////////////////
            if(TiredDrive_CheckDriverRestStatus(currentTime,index))//������Ϣʱ��
            {
                TiredDrive_CheckRestTimePlayVoice();
                TiredDrive_ResetCurDriverData(index);
                s_stTiredAttrib.needsaveflag = 1;//��Ҫ����
            }
        }
    }
    else
    if(1 ==RunStatus)//��ʻ��,�״ν����ʱ��Ч
    {
        s_stTiredDriver.driverinfo[index].validflag = 1;
        if(0==s_stTiredDriver.driverinfo[index].starttime)//����ʻ������,ֱ�ӻ���:û�г����ٶ�Ϊ0����Ϊ0�����
        {
            TiredDrive_SaveCurStartInfo(index,currentTime);//ʵ�ʿ���ʱ��            
        }
				//TiredDrive_DisposeOneDriverInfoTimeIsAbnormal(index);
        /////////////////////////////////        
        s_stTiredDriver.driverinfo[index].overtimeflag = 0;
        s_stTiredDriver.driverinfo[index].nightvalidFlag =0;
        s_stTiredDriver.driverinfo[index].nightstarttime = 0; 
        s_stTiredDriver.driverinfo[index].preovertimeflag = 0;
				s_stTiredDriver.driverinfo[index].starttime = currentTime;
        s_stTiredDriver.driverinfo[index].endtime = currentTime;
        TiredDrive_CheckAndSaveNightstarttime(index,s_stTiredDriver.driverinfo[index].starttime);       
        ////////////////// 
        TiredDrive_GetCurrentRunOverTime(index);
        //////////////////////////        
        sucRunStatusCnt =0;
				TiredDriveSaveFlg = 1;
        s_stTiredAttrib.needsaveflag = 1;//��Ҫ����
    }
    else
    {
        if(0==curSpeedVal)
        {
            TiredDrive_ResetCurDriverData(index);
        }
        else
        if(0==s_stTiredAttrib.prespeedVal&&curSpeedVal)//���ٶ�Ϊ�㵽��Ϊ��
        {
            TiredDrive_SaveCurStartInfo(index,currentTime);//ʵ�ʿ���ʱ��            
        }              
    }
    /////////////////////
    s_stTiredAttrib.prespeedVal = curSpeedVal;    
    /////////////
}
/*************************************************************
** ��������: TiredDrive_CheckCurrentTimeAtNight
** ��������: �жϵ�ǰʱ���Ƿ�Ϊ����ʱ���
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
void TiredDrive_CheckCurrentTimeAtNight(void)
{
    s_stTiredAttrib.atnightFlag = 0;
    if(!s_stTiredAttrib.needChecknight)return;//����Ҫ������ϳ�ʱ
    s_stTiredAttrib.atnightFlag=Public_CheckCurTimeIsInTimeRange(s_stTiredAttrib.nightctrl,s_stTiredAttrib.nightStartVal,s_stTiredAttrib.nightEndVal);   
}
/*************************************************************
** ��������: TiredDrive_CheckCurrentDriverStatus
** ��������: 
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
void TiredDrive_DisposeOneDriverInfoTimeIsAbnormal(unsigned char index)
{
    unsigned long currentTime,temp;
    //////////////////
    if(0==s_stTiredDriver.driverinfo[index].validflag)return;
    /////////////////
    currentTime = RTC_GetCounter();
    /////////////////////////
    temp = s_stTiredDriver.driverinfo[index].starttime+currentTime;
    if(temp > s_stTiredAttrib.curTimeVal)
    {
        s_stTiredDriver.driverinfo[index].starttime = temp-s_stTiredAttrib.curTimeVal;
    }
    else
    {
        s_stTiredDriver.driverinfo[index].starttime = 1;
    }
    //////////////////////
    temp = s_stTiredDriver.driverinfo[index].endtime+currentTime;
    if(temp > s_stTiredAttrib.curTimeVal)
    {
        s_stTiredDriver.driverinfo[index].endtime = temp-s_stTiredAttrib.curTimeVal;
    }
    else
    {
        s_stTiredDriver.driverinfo[index].endtime = 1;
    }
    //////////////////////  
    if(s_stTiredAttrib.needChecknight)//
    {
        if(TiredDrive_CheckTimeAtNight(s_stTiredDriver.driverinfo[index].starttime))
        {
            s_stTiredDriver.driverinfo[index].nightvalidFlag = 1;
            s_stTiredDriver.driverinfo[index].nightstarttime = s_stTiredDriver.driverinfo[index].starttime;
        }
        else
        if(TiredDrive_CheckTimeAtNight(s_stTiredDriver.driverinfo[index].endtime))
        {
            s_stTiredDriver.driverinfo[index].nightvalidFlag = 1;
            temp = s_stTiredDriver.driverinfo[index].endtime%DAY_SECOND;
            s_stTiredDriver.driverinfo[index].nightstarttime = s_stTiredDriver.driverinfo[index].endtime + s_stTiredAttrib.nightStartSec-temp;
        }
        else
        {
            s_stTiredDriver.driverinfo[index].nightvalidFlag = 0;
        }
    }
}
/*************************************************************
** ��������: TiredDrive_CheckCurrentDriverStatus
** ��������: 
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
void TiredDrive_DisposeCurrentTimeIsAbnormal(void)
{
    unsigned char i;
    TiredDrive_CheckCurrentTimeAtNight();
    /////////////////
    for(i=0; i<=TIRED_DRIVER_INFO_NUM; i++)
    {
        TiredDrive_DisposeOneDriverInfoTimeIsAbnormal(i);
    } 
    //////////////////////////
    TiredDrive_GetCurrentRunOverTime(s_stTiredDriver.currentindex);
}
/*************************************************************
** ��������: TiredDrive_CheckCurrentDriverStatus
** ��������: 
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
unsigned long TiredDrive_CheckCurrentTimeIsAbnormal(void)
{
    unsigned long currentTime;
    currentTime = RTC_GetCounter(); 
    //////////////////////    
    s_stTiredAttrib.timeAbnormalFlag = 0;
    if((s_stTiredAttrib.curTimeVal>(currentTime+RESEET_CURRENT_TIRED_DRIVER_TIME_VAL))
      ||(s_stTiredAttrib.curTimeVal+RESEET_CURRENT_TIRED_DRIVER_TIME_VAL)< currentTime)//����60s
    {
        TiredDrive_DisposeCurrentTimeIsAbnormal();
        s_stTiredAttrib.curTimeVal = currentTime;        
        //TiredDrive_ResetCurDriverData(s_stTiredDriver.currentindex);
        ////////////////////////////
        s_stTiredAttrib.needsaveflag = 1;//��Ҫ����
        s_stTiredAttrib.timeAbnormalFlag = 1;
    }
    ///////////////////////////////
    s_stTiredAttrib.curTimeVal = currentTime;
    return s_stTiredAttrib.curTimeVal;
}
/*************************************************************
** ��������: TiredDrive_CheckCurrentDriverStatus
** ��������: 
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
void TiredDrive_CheckAndSaveDriver(unsigned char index,unsigned long currentTime)
{
    if(1 == s_stTiredDriver.driverinfo[index].validflag)
    {
        if(TiredDrive_CheckDriverRestStatus(currentTime,index))
        {
            TiredDrive_ClearDriverInfo(index);
            s_stTiredAttrib.needsaveflag =1;//��Ҫ����
        }
    }
}
/*************************************************************
** ��������: TiredDrive_CheckCurrentDriverStatus
** ��������: 
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
void TiredDrive_CheckAndSetCurrentAlarm(unsigned char index)
{
    unsigned long timeVal;
    if(s_stTiredDriver.driverinfo[index].overtimeflag)
    {
        s_stTiredDriver.driverinfo[index].preovertimeflag = 0;
        Io_WriteAlarmBit(ALARM_BIT_TIRED_DRIVE_PRE_ALARM,RESET);
        Io_WriteAlarmBit(ALARM_BIT_TIRED_DRIVE,SET);
    }
    else
    {
        Io_WriteAlarmBit(ALARM_BIT_TIRED_DRIVE,RESET);
        /////////////Ԥ��////////////////////
        if(s_stTiredDriver.driverinfo[index].preovertimeflag)
        {
            Io_WriteAlarmBit(ALARM_BIT_TIRED_DRIVE_PRE_ALARM,SET);
        }
        else
        {
            Io_WriteAlarmBit(ALARM_BIT_TIRED_DRIVE_PRE_ALARM,RESET);
        }
    }
    /////////////////////////////////
    if(Io_ReadAlarmMaskBit(ALARM_BIT_DAY_OVER_DRIVE))//�����ۼƼ�ʻ��ʱ
    {
        Io_WriteAlarmBit(ALARM_BIT_DAY_OVER_DRIVE,RESET);
        return;
    }
    ////////////////////////////
    if(0xffffffff==s_stTiredAttrib.oneDayoverTimeVal)return;///�����
    ////////////////////
    timeVal = TiredDrive_GetCurtotalDriveTime(index);    
    /////////////////////
    if(s_stTiredAttrib.oneDayoverTimeVal<(s_stTiredDriver.driverinfo[index].totalDriveTime+timeVal))
    {
        Io_WriteAlarmBit(ALARM_BIT_DAY_OVER_DRIVE,SET);
    }
    else
    {
        Io_WriteAlarmBit(ALARM_BIT_DAY_OVER_DRIVE,RESET);
    }    
}
/*************************************************************
** ��������: TiredDrive_CheckCurrentDriverStatus
** ��������: 
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
void TiredDrive_CheckCurrentDriverStatus(void)
{
    unsigned char i;
    unsigned long currentTime;
    currentTime = s_stTiredAttrib.curTimeVal;
    ////////////////////
    TiredDrive_CheckCurrentTimeAtNight();  
    /////////////////////////////////////////////////
    if(!TiredDrive_GetLoginFlag())//���˵�¼ʱ
    {     
        TiredDrive_CheckDriverRunStatus(currentTime,TIRED_DRIVER_INFO_NUM);
    }
    else
    if(s_stTiredDriver.loginindex)//��ǰ��ʻԱ��¼�����
    {
        TiredDrive_CheckDriverRunStatus(currentTime,s_stTiredDriver.loginindex-1);
    }
    ////////////////////////
    for(i=0; i< TIRED_DRIVER_INFO_NUM; i++)//�ж�������ʻԱ�Ƿ������Ϣʱ��
    {
        if(i != s_stTiredDriver.currentindex)//���ǵ�ǰ�ļ�ʻԱ
        {
            TiredDrive_CheckAndSaveDriver(i,currentTime);
        }        
    }
    ///////д��ǰ��ʱ��ʻ״̬��־/////////
    TiredDrive_CheckAndSetCurrentAlarm(s_stTiredDriver.currentindex);
    /////////////////////
    s_stTiredAttrib.firstRunflag = 1;
    //////////////////////////
}
/*************************************************************
** ��������: TiredDrive_OverTimePreAlarmVoice
** ��������: 
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
void TiredDrive_OverTimePreAlarmVoice(void)
{
    if(!Public_PlayE2ParamAlarmVoice(E2_TIRED_DRIVE_PREALARM_VOICE_ID))
    {
        Public_PlayTTSVoiceAlarmStr("����ʱ��ʻ");
    }
}
/*************************************************************
** ��������: TiredDrive_OverTimeAlarmVoice
** ��������: 
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
void TiredDrive_OverTimeAlarmVoice(void)
{
    if(!Public_PlayE2ParamAlarmVoice(E2_TIRED_DRIVE_ALARM_VOICE_ID))
    {
        Public_PlayTTSVoiceAlarmStr("���ѳ�ʱ��ʻ,��ͣ����Ϣ");
    }
}
/*************************************************************
** ��������: TiredDrive_PreOverTimeAlarm
** ��������: ��ʱ��ʻԤ����ʾ
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
void TiredDrive_PreOverTimeAlarm(void)
{
    unsigned char flag;
    /////////Ԥ����ʾ///////////////////////////////////
    if(Io_ReadAlarmBit(ALARM_BIT_TIRED_DRIVE_PRE_ALARM)&&TiredDrive_GetCurRunStatus())      
    {
        flag = 1;
    }
    else
    {
        flag = 0;
    }
    ///////////////////////////
    if(SpeedMonitor_CheckPlayFlag(flag,&s_stTiredAttrib.stPrePlay))
    {
        TiredDrive_OverTimePreAlarmVoice();
    }
}
/*************************************************************
** ��������: TiredDrive_OverTimeAlarm
** ��������: ��ʱ��ʻ��ʾ
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
void TiredDrive_OverTimeAlarm(void)
{
    unsigned long timeVal;
    //��ʱ��ʻ����
    if(Io_ReadAlarmBit(ALARM_BIT_TIRED_DRIVE)&&TiredDrive_GetCurRunStatus())
    {
        s_stTiredAttrib.needReportflag = 0;
        timeVal = TiredDrive_GetCurOverDriveTime();
        if(0==timeVal)return;
        ////////////////////////////
        if(timeVal>=s_stTiredAttrib.AllowRunTime&&TiredDrive_GetTiredStatusForReport())
        {
            if(s_stTiredAttrib.curTimeVal>=s_stTiredDriver.driverinfo[s_stTiredDriver.currentindex].preReportTime+s_stTiredAttrib.ReportInterval)
            {
                s_stTiredDriver.driverinfo[s_stTiredDriver.currentindex].preReportTime = s_stTiredAttrib.curTimeVal;
                s_stTiredAttrib.needsaveflag    = 1;  
                s_stTiredAttrib.needReportflag  = 1;
            }
        }
        ///////////////������ʾʱ��///////////////////////
        if(timeVal>s_stTiredAttrib.PlayAlarmTime&&s_stTiredAttrib.PlayAlarmTime)return;
        ///////////δ������ʾʱ��////////////////////////////
        if(SpeedMonitor_CheckPlayFlag(1,&s_stTiredAttrib.stPlay))
        {
            TiredDrive_OverTimeAlarmVoice();
        }
    }
    else
    {
        s_stTiredAttrib.needReportflag = 0;
        SpeedMonitor_CheckPlayFlag(0,&s_stTiredAttrib.stPlay);
    }
}
/*************************************************************
** ��������: TiredDrive_AlarmTTSAndBeep
** ��������: ��ʱ��ʻģ����ʾ
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
void TiredDrive_AlarmTTSAndBeep(void)
{
    //////////Ԥ����ʾ///////////////
    TiredDrive_PreOverTimeAlarm();    
    ////////��ʱ��ʻ��ʾ/////////////
    TiredDrive_OverTimeAlarm();
}
/*************************************************************
** ��������: TiredDrive_GetCurContDriveTimeSec
** ��������: ȡ�õ�ǰ������ʻʱ��(��λ��)
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
unsigned long TiredDrive_GetCurContDriveTimeSec(void)
{
    unsigned long timeVal,temp;  
    if(!s_stTiredDriver.driverinfo[s_stTiredDriver.currentindex].validflag)return 0;
    timeVal = s_stTiredDriver.driverinfo[s_stTiredDriver.currentindex].endtime;
    temp = s_stTiredDriver.driverinfo[s_stTiredDriver.currentindex].starttime;
    if(timeVal>temp&&temp)
        return (timeVal-temp); 
    return 0;   
}
/*************************************************************
** ��������: TiredDrive_GetCurContDriveTime
** ��������: ȡ�õ�ǰ������ʻʱ��(��λ����)
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
unsigned long TiredDrive_GetCurContDriveTime(void)
{
    unsigned long timeVal;
    timeVal=TiredDrive_GetCurContDriveTimeSec();
    if(timeVal)
    {
        return (timeVal/60); 
    }
    return 0;   
}
/*************************************************************
** ��������: TiredDrive_GetCurOverDriveTime
** ��������: ȡ�õ�ǰ����ƣ�ͼ�ʻ�趨ֵ�Ķ�����(��λ:��)
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 0:��ʾδ��ʱ;����:��ʾ�����趨ֵ������
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
unsigned long TiredDrive_GetCurOverDriveTime(void)
{
    unsigned long timeVal;  
    if(!s_stTiredDriver.driverinfo[s_stTiredDriver.currentindex].validflag)return 0;
    if(!s_stTiredDriver.driverinfo[s_stTiredDriver.currentindex].overtimeflag)return 0;
    timeVal = s_stTiredAttrib.curTimeVal;
    if(timeVal >= s_stTiredAttrib.curOverTimeVal && s_stTiredAttrib.curOverTimeVal)
    {
        timeVal -=s_stTiredAttrib.curOverTimeVal;
        if(0==timeVal)
        {
            timeVal = 1;
            s_stTiredAttrib.needReportflag = 1;
        }
        return timeVal;
    }
    else
    {
        s_stTiredDriver.driverinfo[s_stTiredDriver.currentindex].overtimeflag = 0;
        return 0;
    }
}
/*************************************************************
** ��������: TiredDrive_GetLoginFlag
** ��������: ȡ�ü�ʻԱ��¼��־
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 0:δ��¼,1�ѵ�¼
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
unsigned char TiredDrive_GetLoginFlag(void)
{
    return (1==s_stTiredDriver.driverloginflag)?1:0;
}
/*************************************************************
** ��������: TiredDrive_GetTimeAbnormalflag
** ��������: ȡ��ʱ���쳣��־
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 0:δ�쳣,1�쳣
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
unsigned char TiredDrive_GetTimeAbnormalflag(void)
{
    return (1==s_stTiredAttrib.timeAbnormalFlag)?1:0;
}
/*************************************************************
** ��������: TiredDrive_ReadTiredRecord
** ��������: ����ָ��ʱ��εĳ�ʱ��ʻ��¼
** ��ڲ���: ��ʼʱ��StartTime,����ʱ��EndTime
** ���ڲ���: 
** ���ز���: pBuffer�ֽڳ���
** ȫ�ֱ���: ��
** ����ģ��: Register_Read
*************************************************************/
unsigned short TiredDrive_ReadTiredRecord(unsigned char *pBuffer, TIME_T StartTime, TIME_T EndTime)
{
    unsigned char *p;
    unsigned short len;
    unsigned char block;
    len=0;
    p=pBuffer;
    block = 0;
    len=TiredDrive_ReadTiredDriveFromFramEx(p,StartTime,EndTime);
    if(len)
    {
        p += len;
        block =len/ONE_TIRED_DRIVER_DATA_LEN;
    }
    block =12-block;
    /////////////////
    len += Register_Read(REGISTER_TYPE_OVER_TIME,p,StartTime,EndTime,block);
    return len;    
}
/*************************************************************
** ��������: TiredDrive_Read2daysTiredRecord
** ��������: ָ��ʱ��㿪ʼ����������ĳ�ʱ��ʻ��¼
** ��ڲ���: ��ʼʱ��StartTime
** ���ڲ���: 
** ���ز���: pBuffer�ֽڳ���
** ȫ�ֱ���: ��
** ����ģ��: Register_Read
*************************************************************/
unsigned short TiredDrive_Read2daysTiredRecord(unsigned char *pBuffer, TIME_T StartTime)
{
    unsigned long TimeCount;
    unsigned short length;
    TIME_T EndTime;

    length = 0;
    //��ʼʱ����ʱ,��,����0
    StartTime.hour = 0;
    StartTime.min = 0;
    StartTime.sec = 0;

    //��������ʱ��
    TimeCount = ConverseGmtime(&StartTime);
    TimeCount += 48*3600;
    Gmtime(&EndTime, TimeCount);
    //���ҳ�ʱ��ʻ��¼
    length = TiredDrive_ReadTiredRecord(pBuffer, StartTime,EndTime);
    if(length > 12*ONE_TIRED_DRIVER_DATA_LEN)
    {
        return 0;//���ҳ���
    }
    else
    {
        return length;
    }
}
/*************************************************************
** ��������: TiredDrive_GetCurdriverLicence
** ��������: ��ȡ��ǰ��ʻԱ�ļ�ʻ֤����(18���ֽ�)
** ��ڲ���: 
** ���ڲ���: �������ݵ��׵�ַ
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
unsigned char TiredDrive_GetCurdriverLicence(unsigned char *buffer)
{
    memcpy(buffer,s_stTiredDriver.driverinfo[s_stTiredDriver.currentindex].driverLicence,18);
    ///////////////////////
    buffer[18]=0;
    return 18;
}
/*************************************************************
** ��������: TiredDrive_GetDriverLastRunTime
** ��������: ȡ�ñ����ڵ�����������ʻʱ��(��������ʱ��ȡ)
** ��ڲ���: 
** ���ڲ���: ��
** ���ز���: �����ʻʱ��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
unsigned long TiredDrive_GetDriverLastRunTime(void)
{
    unsigned len;
    STSTOPINFO stTmpStopinfo;
    len=Public_ReadDataFromFRAM(FRAM_TIRED_STOP_INFO_ADDR,(unsigned char *)&stTmpStopinfo,STSTOPINFO_SIZE);
    if(STSTOPINFO_SIZE==len)//�µ�
    {
        return stTmpStopinfo.endtime;
    }
    return 0;
}
/*************************************************************
** ��������: TiredDrive_ClearAllDriverRunInfo
** ��������: ��յ�ǰ�ݴ������������м�ʻԱ����ؼ�ʻ��Ϣ(�ѱ��浽flash�����ݳ���)
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
void TiredDrive_ClearCurAllDriverRunInfo(void)
{
    unsigned char i;    
    for(i=0; i<=TIRED_DRIVER_INFO_NUM; i++)
    {
        TiredDrive_ResetCurDriverData(i);
    }
    /////////////////
    TiredDrive_ClearAlltotalDriveTime();
    //////��Ҫ����/////////////////////////
    s_stTiredAttrib.needsaveflag = 1;
}
/*************************************************************
** ��������: TiredDrive_CheckAndSaveTiredDriverInfo
** ��������: ��鼰�����ʻԱ�ṹ��
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
void TiredDrive_CheckAndSaveTiredDriverInfo(void)
{
    if(s_stTiredAttrib.needsaveflag)//�ı�ļ�ʻԱ��Ϣ
    {
        s_stTiredAttrib.needsaveflag = 0;
        TiredDrive_SaveTiredDriverInfoToFlash();
    }//////////////
}
/*************************************************************
** ��������: TiredDrive_GetTiredStatusForReport
** ��������: ��ȡ���ݳ�ʱ��ʻ״̬���ϱ�λ����Ϣ
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 0:�����ݳ�ʱ��ʻ״̬���ϱ�, 1:���ݳ�ʱ��ʻ״̬���ϱ�
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
unsigned char TiredDrive_GetTiredStatusForReport(void)
{
    unsigned long val;    
    if(0 == s_stTiredAttrib.ReportInterval)return 0;
    if(0 == Io_ReadAlarmBit(ALARM_BIT_TIRED_DRIVE))return 0;
    if(Io_ReadAlarmBit(ALARM_BIT_EMERGENCY))return 0;
    val=SpeedMonitor_GetReportFreqEvent();
    if(!PUBLIC_CHECKBIT(val, 1))return 0;
    return 1;
}
/*************************************************************
** ��������: TiredDrive_GetNeedReportFlag
** ��������: ��ȡ�Ƿ���Ҫ�ϱ�λ����Ϣ��־
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 0:����Ҫ�ϱ�,1:��Ҫ�ϱ�
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
unsigned char TiredDrive_GetNeedReportFlag(void)
{
    if(0==s_stTiredAttrib.needReportflag)return 0;
    s_stTiredAttrib.needReportflag = 0;
    return 1;
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
void TiredDrive_ScanTask(void)
{
    TiredDrive_CheckCurrentTimeIsAbnormal();
    //////////////////
    if(Io_ReadAlarmMaskBit(ALARM_BIT_TIRED_DRIVE))//����ʱ��ʻ��������λ״̬
    {
        if(s_stTiredDriver.driverinfo[s_stTiredDriver.currentindex].validflag)
        {
            TiredDrive_ClearCurAllDriverRunInfo();
            Io_WriteAlarmBit(ALARM_BIT_TIRED_DRIVE,RESET);
            Io_WriteAlarmBit(ALARM_BIT_DAY_OVER_DRIVE,RESET);
            Io_WriteAlarmBit(ALARM_BIT_TIRED_DRIVE_PRE_ALARM,RESET);
        }
        return;
    }
    //����������ʻʱ��,�ж��Ƿ�Ϊ��ʱ��ʻ
    TiredDrive_CheckCurrentDriverStatus();
    /////////��ʱ��ʻ����������������/////////////
    TiredDrive_AlarmTTSAndBeep();  
    /////////////////////////
    TiredDrive_CheckAndCleartotalDriveTime();
    ///////////////////////////////////
    TiredDrive_CheckAndSaveTiredDriverInfo();  
}
/*************************************************************
** ��������: TiredDrive_StartScanTask
** ��������: ��ʱ��ʻ�Ķ�ʱ����
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
void TiredDrive_StartScanTask(void)
{
  TiredDrive_ScanTask();
  LZM_PublicSetCycTimer(&s_stTiredTimer[TIRED_TIMER_TASK],PUBLICSECS(1),TiredDrive_ScanTask);    
}
/*************************************************************
** ��������: TiredDrive_ParamInitialize
** ��������: ��ʱ��ʻ������ʼ��
** ��ڲ���: 
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void TiredDrive_ParamInitialize(void) 
{
    if(0==s_ucTiredModeFirstRun)
    {
        s_ucTiredModeFirstRun = 1;
        LZM_PublicKillTimerAll(s_stTiredTimer,TIRED_TIMERS_MAX);
        memset(&s_stTiredAttrib,0,sizeof(s_stTiredAttrib));         
        SetTimerTask(TIME_TIRED_DRIVE, 5*SYSTICK_0p1SECOND);
        LZM_PublicSetOnceTimer(&s_stTiredTimer[TIRED_TIMER_TASK],PUBLICSECS(0.4),TiredDrive_StartScanTask); 
    }    
    ///////////////////////////////
    TiredDrive_UpdatePram();
    /////////////////////
    TiredDrive_ReadTiredDriverInfoFromFlash();    
    s_stTiredAttrib.curTimeVal = RTC_GetCounter();    
}
/*************************************************************
** ��������: TiredDrive_GetCurrentTime
** ��������: 
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
unsigned long TiredDrive_GetCurrentTime(void)
{
    return s_stTiredAttrib.curTimeVal;
}
/*************************************************************
** ��������: TiredDrive_TimeTask
** ��������: ��ʱ��ʻģ������,1s����һ��
** ��ڲ���: 
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
FunctionalState TiredDrive_TimeTask(void)
{
		
    LZM_PublicTimerHandler(s_stTiredTimer,TIRED_TIMERS_MAX);
    return ENABLE;     
}

/*******************************************************************************
 *                             end of module                                   *
 *******************************************************************************/


/*******************************************************************************
 * File Name:			ICCard.h 
 * Function Describe:	
 * Relate Module:		IC��ģ��
 * Writer:				Joneming
 * Date:				2014-07-09
 * ReWriter:			
 * Date:				
 *******************************************************************************/
 /*******************************************
 //++++++++  �ṹ��: 2012���������ʻ��¼�� ��ʻ�����ʶ��IC�� ++++++++++++++

typedef struct tagSTT_CARD_VEHICLERECORDER_2012
{
    u8 Usage; /// Э��: ����;���� ��ʻԱ///    
    u8 Reserved[31]; ////    
    u8 DriverID[18];//��������ʻ֤����//    
    u8 ValidityPeriod_Year;///��������ʻ֤��Ч��//
    u8 ValidityPeriod_Month;///��������ʻ֤��Ч��//
    u8 ValidityPeriod_Day;///��������ʻ֤��Ч��//    
    u8 QualificationCertificateID[18];///��ҵ�ʸ�֤��///    
    u8 StandardExtension[56];///��׼��չԤ��///    
    u8 VerifyCode;//У����//
}STT_CARD_VEHICLERECORDER_2012;

//++++++++  �ṹ��: 2011�� ��·���䳵�����Ƕ�λϵͳ ��ʻԱ��ҵ�ʸ�֤  ++++++++++++++

typedef struct tagSTRUCT_IC_CARD_ROADTRANSPORT808_2011
{
    u8 Usage;// Э��: ����;���� ��ʻԱ //    
    u8 DriverNameLength;///��ʻԱ�������� ///    
    u8 DriverName[10];///��ʻԱ���� ///    
    u8 DriverID[20];///��ʻԱ���֤����  ����Ϊ20///    
    u8 QualificationCertificateID[40];///��ҵ�ʸ�֤�� ����Ϊ40///    
    u8 IssueOrganizationLength;///��֤�������Ƴ��� ///    
    u8 IssueOrganization[40];///��֤�������Ƴ���///    
    u8 DriverCodeLength;///��ʻԱ���볤�� ///    
    u8 DriverCode[18];///��ʻԱ����///    
    u8 ValidityPeriod_Year[2];///��������ʻ֤��Ч��///
    u8 ValidityPeriod_Month;///��������ʻ֤��Ч��///
    u8 ValidityPeriod_Day;///��������ʻ֤��Ч��///    
    u8 StandardExtension[56];///��׼��չԤ��///    
    u8 VerifyCode;///У����///
}STT_CARD_ROADTRANSPORT808_2011;

//++++++++  �ṹ��: 
typedef struct tagSTT_CARD_JTB
{
	u8 DriverNameLength;///��ʻԱ�������� n///	
	u8 DriverName[20];// ��ʻԱ���� //
	u8 QCID[20];//��ʻԱ���֤����  ����Ϊ20//	
	u8 IssueOrganizationLength;///��֤�������Ƴ���///	
	u8 IssueOrganization[40];///��֤�������Ƴ��� ///	
	u8 ValidityPeriod_Year[2];///��������ʻ֤��Ч��///
	u8 ValidityPeriod_Month;///��������ʻ֤��Ч��///
	u8 ValidityPeriod_Day;///��������ʻ֤��Ч��///	
}STT_CARD_JTB;
//++++++++  �ṹ��: 
typedef struct tagSTT_CARD_DRIVER_RECORD_ITEM
{
    u8 EventTime[6];  //�¼�����ʱ�䣬BCD�� ��-��-�� ʱ-��-��
    u8 DriverID[18];  //��������ʻ֤���� ASCII��
    u8 EventType; //�¼����ͣ� 0x01: ��¼  0x02:�˳� �� ����:Ԥ��  
}
******************************************/
////*****************************************//
//***************�����ļ�*****************
#include "include.h"
/////////////////////////////////
#define ICCARD_DATA_LEN_MIN                 9//10
#define ICCARD_DATA_LEN_MAX                 256
#define ICCARD_DATA_LEN_READ                128
/////////////////////////
#define ICCARD_PLAY_REMIND_GPOUP_TIME       300//300ÿ����ʾʱ����(��)
#define ICCARD_PLAY_REMIND_TOTAL_TIME       1800//1800������ʱ��(��)
#define ICCARD_PLAY_REMIND_ONCE_NUMBER      3//3ÿ����ʾ����
#define ICCARD_PLAY_REMIND_ONCE_TIME        12//ÿ����ʾʱ����(��)
///////////////////////////
typedef struct 
{
    u8 NameLen;
    u8 Name[20];//��ʻԱ
    u8 DriverID[20];//��ʻ֤����
    u8 QCID[40];//��ҵ�ʸ�֤��
    u8 OrganizationLen;
    u8 Organization[40];//��֤��������
    u8 ValidTime[4];//��������ʻ֤��Ч��YYYY-MM-dd
}ST_DRIVER_REPORT;
//////////////
#define SIZE_ST_DRIVER_REPORT sizeof(ST_DRIVER_REPORT)

typedef struct
{
    ST_DRIVER_REPORT stDriver;
    unsigned short playTimeCnt;    
    unsigned char signTime[6];
    unsigned char preDriverID[18];  //��������ʻ֤���� ASCII��
    unsigned char errCode;//
    unsigned char presignstatus;//
    unsigned char signstatus;//
    unsigned char resendCnt;
    unsigned char repairSendCnt; 
    unsigned char onlinestatus;
    unsigned char voicePrompt;
    unsigned char playflag;
    unsigned char firstcheckflag;
    unsigned char OnceNumber;//��ʾ�����ظ������Ĵ���(�趨ֵ)
    unsigned char OnceCnt;//��ʾ�����ظ������ļ�����    
}ST_ICCARD_ATTRIB;

static ST_ICCARD_ATTRIB s_stICCardAttrib;
/////////////////
#define ICCARD_DRIVER_SIGN_DATA_LEN_MAX        109//
typedef struct
{
    unsigned char len;//���ݳ���
    unsigned char data[ICCARD_DRIVER_SIGN_DATA_LEN_MAX];//����
}STDRIVERSIGN;

static STDRIVERSIGN s_stDriverSign;
/////////////////////
#define STDRIVERSIGN_SIZE sizeof(STDRIVERSIGN)

typedef struct
{
    unsigned char saveflag;         //�����־
    unsigned char recordCnt;        //��ǰ�����ǩ��¼������
    unsigned char sendrecordCnt;    //�ѷ��͵ĵ�ǩ����������
    unsigned char bak;
}ICCARD_SAVE_INDEX;

static ICCARD_SAVE_INDEX s_stSignSaveIndex;

#define SAVE_INDEX_FLAG 0xab

#define DRIVERSIGN_MAX_RECORD   40  //4k�Ŀռ�,ÿ��96�ֽ�
///////////////////////////////////////////////////////
#define DRIVER_SIGN_INFO_START_ADDR           (unsigned long)(FLASH_DRIVER_SIGN_INFO_START_SECTOR*FLASH_ONE_SECTOR_BYTES)//��ʼ��ַ
#define DRIVER_SIGN_INFO_DATALEN              (unsigned long)FLASH_ONE_SECTOR_BYTES
#define FRAM_ICCARD_INDEX_ADDR                 FRAM_LAST_RUN_TIME_ADDR

enum
{
    ICCARD_CMD_INSERT = 0x41,
    ICCARD_CMD_PULL_OUT,    
    ICCARD_CMD_MAX,
};

enum
{
    ICCARD_ONLINE_STASUS_EMPTY,
    ICCARD_ONLINE_STASUS_ON, 
    ICCARD_ONLINE_STASUS_OFF,
    ICCARD_ONLINE_STASUS_MAX,
};
enum
{
    DRIVER_SIGN_EMPTY,
    DRIVER_SIGN_IN,
    DRIVER_SIGN_OUT,
    DRIVER_SIGN_INITIATIVE,	//������ȡ��Ϣ0x43
    DRIVER_SIGN_PASSIVITY,	//�����ϱ���Ϣ0xA1
    DRIVER_SIGN_MAX,
};
enum
{
    ICCARD_ERR_NULL_DRIVER = 0,//
    ICCARD_ERR_NULL_EWAYBILL,//
    ICCARD_ERR_NULL_MANAGE,//
    ICCARD_ERR_NULL_MANAGE_EX,//
    ICCARD_ERR_CMD_UNKOWN,//δ֪����
    ICCARD_ERR_TYPE_OUT,  //�����ʹ���  
    ICCARD_ERR_PULL_OUT_LEN,//���ݳ��ȴ���  
    ICCARD_ERR_TYPE_INSERT, //�����ʹ��� 
    ICCARD_ERR_INSERT_NONSUPPORT,//����֧�ֻ���Ч��
    ICCARD_ERR_INSERT_LEN,//���ݳ��ȴ���
    ICCARD_ERR_INSERT_EMPTY,//�տ�
    ICCARD_ERR_INSERT_VERIFY,//����У�����
    ICCARD_ERR_INSERT_TYPE_UNKOWN,//�޷�ʶ��
    ICCARD_ERR_INSERT_DATA_FORMAT,//���ݸ�ʽ����    
    ICCARD_ERR_MAX,
};

enum 
{
    IC_TIMER_TASK,                  //0 timer
    IC_TIMER_TIMEOUT,               //1 timer
    IC_TIMER_CHECK,                 //2 timer
    IC_TIMER_TEST,                  //3 timer
    IC_TIMER_REPAIR,
    IC_TIMERS_MAX
}EICCARDTIME;

static LZM_TIMER s_stICTimer[IC_TIMERS_MAX];
/////////////////////////////////
void ICCard_ServiceDataRepairSignDataToPC(void);
/////////////////////////////////
/*************************************************************
** ��������: ICCard_ShowDebugInfo
** ��������: ���ݽ���
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
void ICCard_ShowDebugInfo(const char *StrPtr)
{
    //TIME_T nowTime;
    //RTC_GetCurTime(&nowTime);
    //LOG_PR( "%s: %02d:%02d:%02d% \r\n",StrPtr,nowTime.hour,nowTime.min,nowTime.sec );
}
/*************************************************************
** ��������: ICCard_PackAndSendData
** ��������: 
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
unsigned char ICCard_PackAndSendData(unsigned char cmd,unsigned char *pBuffer,unsigned short datalen)
{
    u8 buffer[160];
    u16 len;
    len=ICCardBase_CreatPackData(cmd,pBuffer,datalen,buffer,1);
    return CommICCard_SendData(buffer,len);
}
/*************************************************************OK
** ��������: ICCard_FlashIndexSave()
** ��������: ����������������
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void ICCard_FlashIndexSave(void) 
{
    s_stSignSaveIndex.saveflag =SAVE_INDEX_FLAG;
    Public_WriteDataToFRAM(FRAM_ICCARD_INDEX_ADDR,(unsigned char *)&s_stSignSaveIndex,sizeof(ICCARD_SAVE_INDEX));    
}
/*************************************************************OK
** ��������: ICCard_FlashIndexRead
** ��������: ��ȡ��ý��������������
** ��ڲ���: 			 
** ���ڲ���: 
** ���ز���: :
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void ICCard_FlashIndexRead(void) 
{
    Public_ReadDataFromFRAM(FRAM_ICCARD_INDEX_ADDR,(unsigned char *)&s_stSignSaveIndex,sizeof(ICCARD_SAVE_INDEX));
}
/*************************************************************OK
** ��������: ICCard_FlashFormat()
** ��������: Flash��ʽ��
** ��ڲ���: startAddr:��ʼ��ַ(��Ե�ַ);lenght:����
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void ICCard_FlashFormat(void) 
{
    sFLASH_EraseSector(DRIVER_SIGN_INFO_START_ADDR);
}
/*************************************************************OK
** ��������: ICCard_SaveDataToFlash()
** ��������: Flashд����
** ��ڲ���: s:���б����Դ����ָ��,startAddr:��ʼ��ַ(��Ե�ַ);lenght:����
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void ICCard_SaveDataToFlash(unsigned char *s,unsigned short startAddr,unsigned short lenght) 
{
    //ע��startAddrΪ��Ե�ַ
    unsigned long addr;
    addr = startAddr;
    addr += DRIVER_SIGN_INFO_START_ADDR;
    /////////////////////
    sFLASH_WriteBuffer(s,addr,lenght);
}
/*************************************************************OK
** ��������: ICCard_ReadDataFromFlash()
** ��������: Flash������
** ��ڲ���: s:����ָ�룬startAddr:��ʼ��ַ(��Ե�ַ);lenght:����
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void ICCard_ReadDataFromFlash(unsigned char *s,unsigned short startAddr,unsigned short lenght) 
{
    //ע��startAddrΪ��Ե�ַ
    unsigned long addr;
    addr = startAddr;
    addr += DRIVER_SIGN_INFO_START_ADDR;
    ///////////////
    sFLASH_ReadBuffer(s,addr,lenght);
}
/*************************************************************OK
** ��������: ICCard_FlashIndexInit()
** ��������: ��ǩ��¼��ʼ��
** ��ڲ���: 
** ���ڲ���: 
** ���ز���:
** ȫ�ֱ���: ��
** ����ģ��: ICCard_FlashIndexSave
*************************************************************/
void ICCard_SignFlashIndexInit(void)
{ 
    //�ȸ�ʽ��
    ICCard_FlashFormat();
    s_stSignSaveIndex.recordCnt =0;
    s_stSignSaveIndex.sendrecordCnt=0;
    //////////////////////////////////////
    ICCard_FlashIndexSave();
}
/*************************************************************OK
** ��������: ICCardSaveData
** ��������: 
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:           
*************************************************************/
void ICCard_CheckAndMoveSignData(void) //
{
    unsigned short len,startAddr,cnt;
    //////////////////////////////
    if(s_stSignSaveIndex.recordCnt+1>DRIVERSIGN_MAX_RECORD)
    {
        if(s_stSignSaveIndex.recordCnt == s_stSignSaveIndex.sendrecordCnt)
        {
            ICCard_SignFlashIndexInit();
        }
        else
        {
            cnt = s_stSignSaveIndex.sendrecordCnt;
            if(cnt < 3)cnt = 3;
            startAddr = cnt * STDRIVERSIGN_SIZE;
            len=(s_stSignSaveIndex.recordCnt-cnt);
            len*=STDRIVERSIGN_SIZE;
            ICCard_ReadDataFromFlash((unsigned char *)s_ucMsgFlashBuffer ,0,DRIVER_SIGN_INFO_DATALEN);
            ///////////////////////////////////////////////
            ICCard_FlashFormat();
            IWDG_ReloadCounter();
            /////////////////////////////////
            ICCard_SaveDataToFlash((unsigned char *)&s_ucMsgFlashBuffer[startAddr],0,len);            
            if(s_stSignSaveIndex.sendrecordCnt>cnt)s_stSignSaveIndex.sendrecordCnt -= cnt;
            else  s_stSignSaveIndex.sendrecordCnt =0;
            if(s_stSignSaveIndex.recordCnt>cnt)s_stSignSaveIndex.recordCnt -= cnt;
            else s_stSignSaveIndex.recordCnt =0;
            ICCard_FlashIndexSave();//�洢����
        }        
    }
}
/*************************************************************OK
** ��������: ICCardSaveData
** ��������: 
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:           
*************************************************************/
void ICCard_SaveSignData(STDRIVERSIGN stSign) //
{
    unsigned short startAddr;
    ////////////////////////// 
    ICCard_CheckAndMoveSignData();
    /////////////////////////////////
    startAddr = s_stSignSaveIndex.recordCnt * STDRIVERSIGN_SIZE;    
    ////////////////////////////////////////////////////////////
    ICCard_SaveDataToFlash((unsigned char *)&stSign,startAddr,STDRIVERSIGN_SIZE);
    //����������������////
    s_stSignSaveIndex.recordCnt++;    
    ICCard_FlashIndexSave();//�洢����
}
/*************************************************************
** ��������: Report_ResendPositionInformation
** ��������: λ�û㱨ʱ������
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void ICCard_SaveCurReportDriverSignRecord(void)
{
    if(s_stDriverSign.len&&s_stICCardAttrib.resendCnt)
    ICCard_SaveSignData(s_stDriverSign);
    s_stICCardAttrib.resendCnt = 0;
}
/*************************************************************OK
** ��������: ICCard_GetSignRecordData
** ��������: 
** ��ڲ���: index��¼��(��¼����1��ʼ)
** ���ڲ���: 
** ���ز���: 1��ʾ��������,0��ʾû����
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
void ICCard_GetSignRecordData(unsigned char index, STDRIVERSIGN *p)
{
   unsigned short startAddr;
   if(0==index)index = 1;
   startAddr =(index-1) * STDRIVERSIGN_SIZE;
   ////////////////////////////////
   ICCard_ReadDataFromFlash((unsigned char *)p ,startAddr,STDRIVERSIGN_SIZE);
   /////////////////////////////////////
}
/*************************************************************OK
** ��������: ICCard_FlashIndexSave()
** ��������: ����������������
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void ICCard_ServiceDataRepairSucceedForSaveSignIndex(void) 
{
    s_stSignSaveIndex.sendrecordCnt++;
    ICCard_FlashIndexSave();
}
/*************************************************************OK
** ��������: ICCard_ReadRepairServiceDataForSendPC
** ��������: ��鷢��ä����ǩ����ǩ����Ӫ������
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void ICCard_ReadRepairServiceDataForSendPC(void) 
{
    if(s_stSignSaveIndex.sendrecordCnt<s_stSignSaveIndex.recordCnt)
    {
        s_stICCardAttrib.repairSendCnt = 0;
        LZM_PublicSetOnceTimer(&s_stICTimer[IC_TIMER_REPAIR],PUBLICSECS(1),ICCard_ServiceDataRepairSignDataToPC);
    }
}
/*******************************************************************************
**  ��������  : ICCard_ServiceDataRepairToPC
**  ��������  : 
**  ��    ��  : ��				
**  ��    ��  : 
**  ȫ�ֱ���  : 
**  ���ú���  : 
**  �ж���Դ  : 
**  ��    ע  :
*******************************************************************************/
void ICCard_ServiceDataRepairSignDataToPC(void)
{ 
    ProtocolACK ACK_val;
    STDRIVERSIGN stDriverSign;///////////////
    u8 channel = CHANNEL_DATA_1|CHANNEL_DATA_2;
    if(0==communicatio_GetMudulState(COMMUNICATE_STATE))
    {
        s_stICCardAttrib.repairSendCnt = 0;
        return;
    }
    s_stICCardAttrib.repairSendCnt++;
    if(s_stICCardAttrib.repairSendCnt < 5)
    {
        ICCard_GetSignRecordData(s_stSignSaveIndex.sendrecordCnt+1,&stDriverSign);
        ACK_val=RadioProtocol_DriverInformationReport(channel,stDriverSign.data, stDriverSign.len);
        if(ACK_val == ACK_OK)//�ϴ���ƽ̨�ɹ�������һ��
        {
            s_stICCardAttrib.repairSendCnt = 0;
            ICCard_ServiceDataRepairSucceedForSaveSignIndex();
            ICCard_ReadRepairServiceDataForSendPC();
        }
        else//�ϴ���ƽ̨ʧ��
        {
            LZM_PublicSetOnceTimer(&s_stICTimer[IC_TIMER_REPAIR],PUBLICSECS(3),ICCard_ServiceDataRepairSignDataToPC);
        }
    }
    else
    {
        s_stICCardAttrib.repairSendCnt = 0;
    }
}
/*************************************************************
** ��������: ICCard_DriverSignSendDataToPlatform
** ��������: 
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void ICCard_DriverSignSendDataToPlatform(void) 
{
    unsigned char flag = 0;
    u8 channel = CHANNEL_DATA_1 | CHANNEL_DATA_2;
    
    if(0==s_stICCardAttrib.resendCnt)return;
    flag=communicatio_GetMudulState(COMMUNICATE_STATE);
	//if(Modem_IpSock_STA[2] == MODSOC_ONLN)//�����ж�
	//	flag = 1;
    //////////////////////�ȰѾɵĴ���ȥ
    if(s_stSignSaveIndex.sendrecordCnt<s_stSignSaveIndex.recordCnt||(flag==0))//
    {
        ICCard_SaveCurReportDriverSignRecord();//�ѵ�ǰ�ı�������
        if(flag)
        {
            ICCard_ReadRepairServiceDataForSendPC();
        }
        return;
    }
    /////////////////////////////////////
    if(ACK_OK == RadioProtocol_DriverInformationReport(channel,s_stDriverSign.data, s_stDriverSign.len))
    {
        ICCard_SaveCurReportDriverSignRecord();
        ICCard_ServiceDataRepairSucceedForSaveSignIndex();        
    }
    else//ʧ��
    {
        s_stICCardAttrib.resendCnt++;
        if(s_stICCardAttrib.resendCnt < 7)
        {
            LZM_PublicSetOnceTimer(&s_stICTimer[IC_TIMER_REPAIR],PUBLICSECS(0.3),ICCard_DriverSignSendDataToPlatform);            
        }
        else
        {
            ICCard_SaveCurReportDriverSignRecord();//�ѵ�ǰ�ı�������
        }
    }    
}
/*************************************************************
** ��������: ICCard_ReportDriverSignRecord
** ��������: ���ݽ���
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
void ICCard_PackAndSendReportDriverSignRecord(unsigned char status)
{
    unsigned char buffer[STDRIVERSIGN_SIZE+20];
    unsigned char len;
    if(s_stICCardAttrib.resendCnt)
    {
        LZM_PublicKillTimer(&s_stICTimer[IC_TIMER_REPAIR]);
        ICCard_SaveCurReportDriverSignRecord();
    }    
    len =0;
    buffer[len++] = status;
    memcpy(&buffer[len],s_stICCardAttrib.signTime,6);
    len +=6;    
    //if(DRIVER_SIGN_IN == status)
    {
        buffer[len++] = 0x00;//�������
        ////////////////////
        buffer[len++] = s_stICCardAttrib.stDriver.NameLen;
        memcpy(&buffer[len],s_stICCardAttrib.stDriver.Name,s_stICCardAttrib.stDriver.NameLen);        
        len += s_stICCardAttrib.stDriver.NameLen;
        //////////////////////////
        memcpy(&buffer[len],s_stICCardAttrib.stDriver.DriverID,20);
        len +=20;
        ////////////////
        buffer[len++] = s_stICCardAttrib.stDriver.OrganizationLen;
        memcpy(&buffer[len],s_stICCardAttrib.stDriver.Organization,s_stICCardAttrib.stDriver.OrganizationLen);
        len += s_stICCardAttrib.stDriver.OrganizationLen;
        //////////////////
        memcpy(&buffer[len],s_stICCardAttrib.stDriver.ValidTime,4);
        len += 4;
    } 
		if(DRIVER_SIGN_PASSIVITY == status)
		{
			ICCard_Transmit_E_Signature(&buffer[len]);//���ϵ���ǩ��
			len += 8;
		}
    memset(&s_stDriverSign,0,STDRIVERSIGN_SIZE);
    s_stDriverSign.len = len;
    if(s_stDriverSign.len>ICCARD_DRIVER_SIGN_DATA_LEN_MAX)
    {
        s_stDriverSign.len= ICCARD_DRIVER_SIGN_DATA_LEN_MAX;
    }
    memcpy(s_stDriverSign.data,buffer,s_stDriverSign.len);
    /////////////////////
    s_stICCardAttrib.resendCnt = 1;
    ICCard_DriverSignSendDataToPlatform();
}
/*************************************************************
** ��������: ICCard_SaveRegisterTypeDriver
** ��������: ���ݽ���
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
void ICCard_SaveRegisterTypeDriver(unsigned char status,unsigned long time)
{
    unsigned char len;
    unsigned char buffer[REGISTER_DRIVER_STEP_LEN];    
    memcpy(buffer,s_stICCardAttrib.signTime,6);
    len = 6;
    if(DRIVER_SIGN_IN==status)
    {
        memcpy(&buffer[len],s_stICCardAttrib.stDriver.DriverID,18);
        len+=18;
        buffer[len++] = status;
    }
    else//�Ȱ�ǰһ�ε�ǩ������ǩ��
    {
        memcpy(&buffer[len],s_stICCardAttrib.preDriverID,18);
        len+=18;
        buffer[len++] = status;
    }    
    Register_Write2( REGISTER_TYPE_DRIVER, buffer, (REGISTER_DRIVER_STEP_LEN-5),time);
}
/*************************************************************
** ��������: ICCard_CheckAndDisposeSignInAndOut
** ��������: ���ݽ���
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
void ICCard_SaveAndReportDriverSignRecord(unsigned char status,unsigned long time)
{
    if(DRIVER_SIGN_IN!=status&&DRIVER_SIGN_OUT!=status&&DRIVER_SIGN_PASSIVITY!=status)return;
    ICCard_SaveRegisterTypeDriver(status,time);
    ICCard_PackAndSendReportDriverSignRecord(status);
}
/*************************************************************
** ��������: ICCard_PackReportDriverInfo
** ��������: �����ǰ��ǩ��Ϣ
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
unsigned char ICCard_PackReportDriverInfo(unsigned char *pBuffer,unsigned char *pBuffer_Temp)
{
		u8	len = s_stDriverSign.len;
    if(len)
    {
#if (ICCARD_JTD == ICCARD_SEL)
		{	memcpy(pBuffer_Temp,s_stDriverSign.data,len);
			memcpy(pBuffer_Temp+len,pBuffer+1,8);//���ϵ���ǩ��
			*pBuffer_Temp = 0x04;//״̬λ �����ϱ�  04
			len += 8;
		}
#else
		{
			memcpy(pBuffer_Temp,s_stDriverSign.data,s_stDriverSign.len);
		}
#endif
    }
    return len;
}

/*************************************************************
** ��������: ICCard_ShowAndPlayAlarmStr
** ��������: ���ݽ���
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
void ICCard_ShowAndPlayStr(const u8 *StrPtr)
{
    Public_PlayTTSVoiceStr(StrPtr);
    Public_ShowTextInfo((char * )StrPtr,PUBLICSECS(6));
}
/*************************************************************
** ��������: ICCard_ShowAndPlayAlarmStr
** ��������: ���ݽ���
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
void ICCard_ShowAndPlayAlarmStr(const u8 *StrPtr)
{
    Public_PlayTTSVoiceAlarmStr(StrPtr);
    Public_ShowTextInfo((char * )StrPtr,PUBLICSECS(6));
}
/*************************************************************
** ��������: ICCard_ShowSignInDriverInformation
** ��������: ���ݽ���
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
void ICCard_ShowSignInDriverInfoAndPlay(void)
{
    char LCDDisplayBuf[50];
    unsigned char len;
    Public_PlayTTSVoiceStr("��ʻԱǩ��");
    strcpy(LCDDisplayBuf,"ǩ�� ��ʻ֤����:");
    len=strlen(LCDDisplayBuf);
    memcpy(&LCDDisplayBuf[len],s_stICCardAttrib.stDriver.DriverID,18);
    LCDDisplayBuf[18+len]= 0;
    Public_ShowTextInfo(LCDDisplayBuf,PUBLICSECS(6));
}
/*************************************************************
** ��������: ICCard_UpdataCurSignParamAndShow
** ��������: ���ݽ���
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
void ICCard_UpdataCurSignParamAndShow(void)
{
    EepromPram_WritePram(E2_CURRENT_LICENSE_ID,s_stICCardAttrib.stDriver.DriverID,18);
    s_stICCardAttrib.presignstatus = s_stICCardAttrib.signstatus;
    memcpy(s_stICCardAttrib.preDriverID,s_stICCardAttrib.stDriver.DriverID,18);
    if(DRIVER_SIGN_IN==s_stICCardAttrib.signstatus)
    {
        TiredDrive_Login();   
        ICCard_ShowSignInDriverInfoAndPlay();        
    }
    else
    if(DRIVER_SIGN_OUT==s_stICCardAttrib.signstatus)
    {
        TiredDrive_Logout();        
        ICCard_ShowAndPlayStr("��ʻԱǩ��");
    }
}
/*************************************************************
** ��������: ICCard_CheckAndDisposeSignInAndOut
** ��������: ���ݽ���
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
void ICCard_DisposeSignInAndOut(void)
{
    unsigned long time;
    time = RTC_GetCounter();
    Public_ConvertNowTimeToBCDEx(s_stICCardAttrib.signTime);
    ///////////////////
    if((DRIVER_SIGN_IN==s_stICCardAttrib.signstatus)&&(DRIVER_SIGN_IN==s_stICCardAttrib.presignstatus))
    {
        ICCard_SaveAndReportDriverSignRecord(DRIVER_SIGN_OUT,time);
        /////////////////////////////
        ICCard_SaveAndReportDriverSignRecord(s_stICCardAttrib.signstatus,time+1);
    }
    else
    {
        ICCard_SaveAndReportDriverSignRecord(s_stICCardAttrib.signstatus,time);
    } 
    //////////////////
    ICCard_UpdataCurSignParamAndShow();
}
/*************************************************************
** ��������: ICCard_CheckAndDisposeSignInAndOut
** ��������: ���ݽ���
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
void ICCard_CheckAndDisposeSignInAndOut(void)
{
    if(SpeedMonitor_GetCurRunStatus())
    {
        LZM_PublicSetOnceTimer(&s_stICTimer[IC_TIMER_TASK], PUBLICSECS(1), ICCard_CheckAndDisposeSignInAndOut);
    }
    else
    {
       ICCard_DisposeSignInAndOut(); 
    }
}
/*************************************************************
** ��������: ICCard_CheckAndDisposeInsertOrPullOutResult
** ��������: ���ݽ���
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
void ICCard_CheckAndDisposeInsertAndOutResult(void)
{
    switch(s_stICCardAttrib.errCode)
    {
        case ICCARD_ERR_NULL_EWAYBILL://
            ICCard_ShowAndPlayAlarmStr("�����˵����Ѳ���");
            break;
        case ICCARD_ERR_NULL_MANAGE://
            ICCard_ShowAndPlayAlarmStr("����Ա������Ч");
            break;
        case ICCARD_ERR_NULL_MANAGE_EX://
            ICCard_ShowAndPlayAlarmStr("��<�߼�����>�˵��²������Ա������Ч");            
            break;
        case ICCARD_ERR_CMD_UNKOWN://δ֪����
            break;
        case ICCARD_ERR_TYPE_OUT://�����ʹ���
            break;
        case ICCARD_ERR_PULL_OUT_LEN://���ݳ��ȴ���
            ICCard_ShowAndPlayAlarmStr("���ݳ��ȴ���");
            break;
        case ICCARD_ERR_TYPE_INSERT://�����ʹ���
            ICCard_ShowAndPlayAlarmStr("���ʹ���" );
            break;
        case ICCARD_ERR_INSERT_NONSUPPORT://����֧�ֻ���Ч��
            ICCard_ShowAndPlayAlarmStr("�����������Ч��" );
            break;
        case ICCARD_ERR_INSERT_LEN://���ݳ��ȴ���
            ICCard_ShowAndPlayAlarmStr("�����������Ч��" );
            break;
        case ICCARD_ERR_INSERT_EMPTY://�տ�
            ICCard_ShowAndPlayAlarmStr("�ÿ�Ϊ�տ�" );
            break;
        case ICCARD_ERR_INSERT_VERIFY://����У�����
            ICCard_ShowAndPlayAlarmStr("����У�����" );
            break;        
        case ICCARD_ERR_INSERT_TYPE_UNKOWN://�޷�ʶ��
            ICCard_ShowAndPlayAlarmStr("�޷�ʶ��" );
            break;
        case ICCARD_ERR_INSERT_DATA_FORMAT://���ݸ�ʽ����
            ICCard_ShowAndPlayAlarmStr("���ݸ�ʽ����" );
            break;
        case ICCARD_ERR_NULL_DRIVER:
            if((DRIVER_SIGN_OUT==s_stICCardAttrib.signstatus)&&(DRIVER_SIGN_OUT==s_stICCardAttrib.presignstatus))
            {
                //��ǩ��,����Ҫ����ʾ
                break;
            }
            else
            if((DRIVER_SIGN_IN==s_stICCardAttrib.signstatus)&&(DRIVER_SIGN_IN==s_stICCardAttrib.presignstatus))
            {
                if(0==Public_CheckArrayValIsEqual(s_stICCardAttrib.stDriver.DriverID, s_stICCardAttrib.preDriverID, 18))
                {
                    //ͬһ�ſ���ǩ��,����Ҫ����ʾ
                    break;
                }
            }
            ///////////////////////
            if(SpeedMonitor_GetCurRunStatus())
            {
                ICCard_ShowAndPlayAlarmStr("��ʻ�н�ֹ��ο�");
            }            
            ICCard_CheckAndDisposeSignInAndOut();
            break;
        default:
            break;
    }
}
/*************************************************************
** ��������: ICCard_DisposeExtractEwaybill
** ��������: 
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
unsigned char ICCard_VerifyAscllValue(const u8 * InBuf, u16 InLength)
{
    u8 ret = 0 ;
    for( u16 i = 0 ; i < InLength ; i++ )
    {
        if((InBuf[i]&&InBuf[i]<0x20)||(InBuf[i]>'~'))
        {
            ret  = 1;
            break;
        }
    }
    return ret ;
}
/*************************************************************
** ��������: ICCard_DisposeExtractEwaybill
** ��������: 
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
void ICCard_DisposeExtractEwaybill(unsigned char * pInBuf, unsigned short InLength)
{
    unsigned long tmplen;
    u8 channel = CHANNEL_DATA_1;//Ϊ����ͨ�������
    tmplen=Public_ConvertBufferToLong(pInBuf);
    if(tmplen>InLength)return;
    RadioProtocol_ElectricReceiptReport(channel,&pInBuf[4], tmplen);
}

/*************************************************************
** ��������: ICCard_DisposeExtractDriver808_2011
** ��������: �ӽ��յ��ֽ����������ݽ�����808Э���ʻԱ��Ӧ�Ľṹ����Ӧ������ 
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
unsigned char ICCard_DisposeExtractDriver808_2011( unsigned char * pInBuf, unsigned char InLength, ST_DRIVER_REPORT  * stDriver  )
{
    unsigned char i,temp;
    unsigned char len;
    unsigned char *buffer;
    memset(stDriver,0,SIZE_ST_DRIVER_REPORT);
    buffer=pInBuf;
    len = 0;
    //len++;/****  Э��: ����;���� ��ʻԱ ***/
    /*****************��ʻԱ�������� n  ,�10�ֽ�********************/
    stDriver->NameLen = buffer[len++];
    if(stDriver->NameLen > 20 )return 1;
    /*****************��ʻԱ����********************/
    for( i = 0 ; i <  stDriver->NameLen ; i++ )
    {
        stDriver->Name[i] = buffer[len++];
    }
    /*****************��ʻԱ���֤���� ����Ϊ20********************/
    for( i = 0 ; i <  20 ; i++ )
    {
        stDriver->DriverID[ i ] = buffer[len++];
    }
    ///////////////////
    if(ICCard_VerifyAscllValue(stDriver->DriverID,20)) return 2;//
    /*****************��ҵ�ʸ�֤�� ����Ϊ40********************/
    for( i = 0 ; i <  40 ; i++ )
    {
        stDriver->QCID[ i ] = buffer[len++];
    }
    ////////////////////
    if(ICCard_VerifyAscllValue(stDriver->QCID,40))return 4;//

    /*****************��֤�������Ƴ��� ,�30�ֽ� ********************/
    stDriver->OrganizationLen = buffer[len++];
    if( stDriver->OrganizationLen > 40 )return 5;//
    /*****************��֤�������Ƴ��� ********************/
    for( i= 0 ;  i <  stDriver->OrganizationLen ; i++ )
    {
        stDriver->Organization[ i ]= buffer[len++];
    }    
    /*****************��ʻԱ���볤�� ********************/
    temp = buffer[len++];
    /*****************��ʻԱ���� ********************/
    if(temp<20&&temp)//�ж��Ƿ���Ч
    {
        len +=  temp;//������ʻԱ����
        ///////////////////
        /*****************��������ʻ֤��Ч��*******************/
        for( i= 0 ;  i <  4 ; i++ )
        {
            stDriver->ValidTime[i]= buffer[len++];
        }
    }
    else
    {
        stDriver->ValidTime[0]=0x20;
        stDriver->ValidTime[1]=0x99;
        stDriver->ValidTime[2]=0x12;
        stDriver->ValidTime[3]=0x31;
    }
    return 0;
}
/*************************************************************
** ��������: ICCard_DisposeExtractDriverRecorder2012
** ��������: �ӽ��յ��ֽ���� 
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
unsigned char ICCard_DisposeExtractDriverRecorder2012( unsigned char * pInBuf, unsigned char InLength, ST_DRIVER_REPORT *stDriver)
{
    char strDriverName[]= "��ʻԱ"; //
    char strOrganization[]="��֤����";
    unsigned char i;
    unsigned char len;
    unsigned char *buffer;
    memset(stDriver,0,SIZE_ST_DRIVER_REPORT);
    buffer=pInBuf;
    len = 0;
    //len++;/****  Э��: ����;���� ��ʻԱ ***/
    len +=31;//�����ֽ�
    /*****************��ʻԱ���֤���� ����Ϊ20********************/
    for( i = 0 ; i <  18 ; i++ )
    {
        stDriver->DriverID[ i ] = buffer[len++];
    }
    ///////////////////
    if(ICCard_VerifyAscllValue(stDriver->DriverID,20)) return 2;//    
    /*****************��������ʻ֤��Ч��*******************/
    stDriver->ValidTime[0]=0x20;//��Ч�ڲ����YYYY
    for( i= 0 ;  i <  3 ; i++ )
    {
        stDriver->ValidTime[1+i]= buffer[len++];
    }    
    /*****************��ҵ�ʸ�֤�� ����Ϊ40********************/
    for( i = 0 ; i <  18 ; i++ )
    {
        stDriver->QCID[ i ] = buffer[len++];
    }
    ////////////////////
    if(ICCard_VerifyAscllValue(stDriver->QCID,40))return 4;//
    //////////��ӹ̶�����///////
    /*****************��ʻԱ�������� n  ,�10�ֽ�********************/
    stDriver->NameLen = sizeof(strDriverName)-1;//��ȥ������
    /*****************��ʻԱ����********************/
    strcpy((char *)stDriver->Name,strDriverName);
    /*****************��֤�������Ƴ��� ,�30�ֽ� ********************/    
    stDriver->OrganizationLen =sizeof(strOrganization)-1;
    /*****************��֤�������� ********************/
    strcpy((char *)stDriver->Organization,strOrganization);  
    ////////////////
    return 0;
}
/********************************************************************
** ��������: ICCard_DisposeExtractDriverJTB
** ��������: ��������ֽ������ݽ�ͨ����ʻԱ���ݸ�ʽ
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
unsigned char ICCard_DisposeExtractDriverJTB(u8 *pInBuf, u8 InLength,ST_DRIVER_REPORT *stDriver)
{
    unsigned char i;
    unsigned char len;
    unsigned char *buffer;
    memset(stDriver,0,SIZE_ST_DRIVER_REPORT);
    buffer=pInBuf;
    len = 0;
	/*****************��ʻԱ�������� n********************/
    stDriver->NameLen = buffer[len++] ;  
	/*****************��ʻԱ����********************/
    for( i = 0 ; i <  stDriver->NameLen ; i++ )
    {
		stDriver->Name[i] =  buffer[len++] ;  
    }    
	/*****************��ҵ�ʸ�֤�� ����Ϊ20********************/
    for( i = 0 ; i <  20 ; i++ )
    {
        stDriver->DriverID[i] = buffer[len++] ; 
        stDriver->QCID[i]     = stDriver->DriverID[i];  
    }
	/*****************��֤�������Ƴ��� ********************/
    stDriver->OrganizationLen = buffer[len++] ; 
	/*****************��֤�������Ƴ��� ********************/
    for( i= 0 ;  i <  stDriver->OrganizationLen ; i++ )
    {
        stDriver->Organization[ i ]= buffer[len++] ; 
    }	
	/*****************��������ʻ֤��Ч��*******************/
	
    for( i= 0 ;  i <  4 ; i++ )
    {
        stDriver->ValidTime[i]= buffer[len++] ; 
    }
	/*****************��׼��չԤ��********************/
    return 0;
}
/*************************************************************
** ��������: ICCard_DisposeExdeviceData
** ��������: ���ݽ���
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
unsigned char ICCard_DisposeExdeviceData(unsigned char * pInBuf, unsigned short InLength)
{
    ST_DRIVER_REPORT stTmpDriver;
    unsigned char result;
    unsigned char *buffer;
    unsigned char usage;
    buffer  = pInBuf;
    usage   = buffer[0];    
    switch(usage)
    {
        case ICCARD_USAGE_EWAYBILL:
            ICCard_DisposeExtractEwaybill(&buffer[1],InLength-1);
            result = ICCARD_ERR_NULL_EWAYBILL;
            break;
        case ICCARD_USAGE_MANAGE:
            if(BrushAdministratorCardResponse(&buffer[1]))
            {
                result = ICCARD_ERR_NULL_MANAGE;
            }
            else
            {
                result = ICCARD_ERR_NULL_MANAGE_EX;
            }
            break;
        case ICCARD_USAGE_DRIVER_808_2011:            
            if(0==ICCard_DisposeExtractDriver808_2011(&buffer[1],InLength-1,&stTmpDriver))
            {
                s_stICCardAttrib.signstatus = DRIVER_SIGN_IN;//ǩ��
                memcpy(&s_stICCardAttrib.stDriver,&stTmpDriver,SIZE_ST_DRIVER_REPORT);
            }
            else
            {
                result = ICCARD_ERR_INSERT_DATA_FORMAT;
            }
            break;
        case ICCARD_USAGE_UNDEFINE:
            result = ICCARD_ERR_INSERT_TYPE_UNKOWN;
            break;
        default:
            if(0==ICCard_DisposeExtractDriverRecorder2012(&buffer[1],InLength-1,&stTmpDriver))
            {
                s_stICCardAttrib.signstatus = DRIVER_SIGN_IN;//ǩ��
                memcpy(&s_stICCardAttrib.stDriver,&stTmpDriver,SIZE_ST_DRIVER_REPORT);
            }
            else
            {
                result = ICCARD_ERR_INSERT_DATA_FORMAT;
            }
            break;
    }
    return result;
}
/*************************************************************
** ��������: ICCard_ProtocolParseExdevice
** ��������: ���ݽ���
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
unsigned char ICCard_ProtocolParseExdevice(ST_PROTOCOL_EXDEVICE stExdevice, unsigned short datalen)
{
    unsigned char templen;
    unsigned char result;    
    result = ICCARD_ERR_NULL_DRIVER;
    ///////////////////////
    switch(stExdevice.Cmd)
    {
        case ICCARD_CMD_INSERT:
            templen = datalen - 8;           
            if(stExdevice.Data[0])//IC������������֧�ֵĿ�Ƭ
            {
                result = ICCARD_ERR_INSERT_NONSUPPORT;
            }
            else
            if(ICCARD_DATA_LEN_READ != templen)//
            {
                result = ICCARD_ERR_INSERT_LEN;
            }
            else
            if((0x00==stExdevice.Data[2])&&(0x00==stExdevice.Data[3]))//
            {
                result = ICCARD_ERR_INSERT_EMPTY;
            }
            else
            if((0xff==stExdevice.Data[2])&&(0xff==stExdevice.Data[3]))//
            {
                result = ICCARD_ERR_INSERT_EMPTY;
            }
            else
            {
                s_stICCardAttrib.firstcheckflag = 0;
                #ifdef ICCARD_VERIFY_ENABLE                    
                u8 verify = ICCardBase_VerifyCalcXor( &stExdevice.Data[1] , templen-1  ); 
                if(stExdevice.Data[templen] != verify )
                {
                    result = ICCARD_ERR_INSERT_VERIFY;
                    break;
                }
                #endif                    
                result = ICCard_DisposeExdeviceData(&stExdevice.Data[1],templen);
            }
            break;
       case ICCARD_CMD_PULL_OUT:   
            if(8 == datalen)//
            {
                if(1==s_stICCardAttrib.firstcheckflag)
                {
                    result = ICCARD_ERR_TYPE_OUT;
                }
                else
                {
                    s_stICCardAttrib.signstatus = DRIVER_SIGN_OUT;//ǩ��
                    memset(s_stICCardAttrib.stDriver.DriverID,0x30,18);
                }
            }
            else
            {
               result = ICCARD_ERR_PULL_OUT_LEN; 
            }
            break;
        default:
            result = ICCARD_ERR_CMD_UNKOWN;
            break;
    }
    return result;
}
/*************************************************************
** ��������: ICCard_DisposeExternalDriverSign
** ��������: �����۵ۻ�ͨ���涨�ļ�ʻԱ��
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
void ICCard_DisposeExternalDriverSign(void)
{
    s_stICCardAttrib.errCode = ICCARD_ERR_NULL_DRIVER;
    LZM_PublicSetOnceTimer(&s_stICTimer[IC_TIMER_TASK], LZM_AT_ONCE, ICCard_CheckAndDisposeInsertAndOutResult);
}
/*************************************************************
** ��������: ICCard_DisposeJTBCardDriverSignIn
** ��������: �����۵ۻ�ͨ���涨�ļ�ʻԱ��
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
void ICCard_DisposeJTBCardDriverSignIn(unsigned char *pBuffer, unsigned short datalen)
{
    ST_DRIVER_REPORT stTmpDriver;
    ICCard_DisposeExtractDriverJTB(pBuffer,datalen,&stTmpDriver);        
    memcpy(&s_stICCardAttrib.stDriver,&stTmpDriver,SIZE_ST_DRIVER_REPORT);
    s_stICCardAttrib.signstatus = DRIVER_SIGN_IN;//ǩ��
    s_stICCardAttrib.firstcheckflag = 0;
    ICCard_DisposeExternalDriverSign();
}
/*************************************************************
** ��������: ICCard_DisposeJTBCardDriverSignOut
** ��������: �����۵ۻ�ͨ���涨�ļ�ʻԱ��
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
void ICCard_DisposeJTBCardDriverSignOut(unsigned char *pBuffer, unsigned short datalen)
{
    ST_DRIVER_REPORT stTmpDriver;
    ICCard_DisposeExtractDriverJTB(pBuffer,datalen,&stTmpDriver);        
    memcpy(&s_stICCardAttrib.stDriver,&stTmpDriver,SIZE_ST_DRIVER_REPORT);
    s_stICCardAttrib.signstatus = DRIVER_SIGN_OUT;//ǩ��
    s_stICCardAttrib.firstcheckflag = 0;
    ICCard_DisposeExternalDriverSign();
}

/*************************************************************
** ��������: ICCard_DisposeJTBCardDriverSignPassivity
** ��������: ���ݶ��� ����Ӧ�����֤����Ϣ
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
void ICCard_DisposeJTBCardDriverSignPassivity(unsigned char *pBuffer, unsigned short datalen)
{
		ST_DRIVER_REPORT stTmpDriver;
		ICCard_DisposeExtractDriverJTB(pBuffer,datalen,&stTmpDriver); 			 
		memcpy(&s_stICCardAttrib.stDriver,&stTmpDriver,SIZE_ST_DRIVER_REPORT);
		s_stICCardAttrib.signstatus = DRIVER_SIGN_PASSIVITY;//����Ӧ��
		s_stICCardAttrib.firstcheckflag = 0;
		ICCard_DisposeExternalDriverSign();
}

/*************************************************************
** ��������: ICCard_DisposeExternalDriverSignIn
** ��������: �����ⲿ��ʾ����ʻԱ��ǩ��
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
void ICCard_DisposeExternalDriverSignIn(unsigned char *pBuffer, unsigned short datalen)
{
    ST_DRIVER_REPORT stTmpDriver;
    if(ICCard_DisposeExtractDriver808_2011(pBuffer,datalen,&stTmpDriver))return;
    memcpy(&s_stICCardAttrib.stDriver,&stTmpDriver,SIZE_ST_DRIVER_REPORT);
    s_stICCardAttrib.signstatus = DRIVER_SIGN_IN;//ǩ��
    s_stICCardAttrib.firstcheckflag = 0;   
    ICCard_DisposeExternalDriverSign();
}
/*************************************************************
** ��������: ICCard_DisposeExternalCardDriverSignOut
** ��������: �����ⲿ��ʻԱ��ǩ��
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
void ICCard_DisposeExternalCardDriverSignOut(void)
{
    if(DRIVER_SIGN_IN != s_stICCardAttrib.presignstatus)return;
    if(DRIVER_SIGN_OUT == s_stICCardAttrib.signstatus)return;    
    memset(s_stICCardAttrib.stDriver.DriverID,0x30,18);
    s_stICCardAttrib.signstatus = DRIVER_SIGN_OUT;//ǩ��
    ICCard_DisposeExternalDriverSign();
}
/*************************************************************
** ��������: ICCard_DisposeRecvExdevice
** ��������: ���ݽ���
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
void ICCard_DisposeRecvExdevice(ST_PROTOCOL_EXDEVICE stExdevice, unsigned short datalen)
{
    s_stICCardAttrib.errCode = ICCard_ProtocolParseExdevice(stExdevice,datalen);
    LZM_PublicSetOnceTimer(&s_stICTimer[IC_TIMER_TASK], LZM_AT_ONCE, ICCard_CheckAndDisposeInsertAndOutResult);
}
extern u8	SwipCardCh;//ˢ��ͨ��  0 ����������  1  ����IC��
/*************************************************************
** ��������: ICCard_ProtocolParse
** ��������: IC�����ݽ���
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
void ICCard_ProtocolParse(unsigned char * pInBuf, unsigned short InLength)
{
    ST_PROTOCOL_EXDEVICE stTmpExdevice;
    unsigned char tmpBuffer[ICCARD_DATA_LEN_MAX];
    unsigned short iPosition[3],datalen=0;
    unsigned short i;
    unsigned char *buffer;
    unsigned char flag;
    if(InLength<ICCARD_DATA_LEN_MIN)return;
    buffer=pInBuf;
    flag =0;     
    for(i = 0; i<InLength; i++)
    {
        if(0x7E==buffer[i])
        {
            iPosition[flag++] = i ;
            if(2==flag)
            {
                datalen = iPosition[1]-iPosition[0]-1;
								if(datalen==0)continue;//������������е�bug,����һ��
								memcpy(tmpBuffer, &buffer[iPosition[0]+1],datalen);
								flag = 0xff;
            }
			else
			if(3==flag)
			{
				datalen = iPosition[2]-iPosition[1]-1; 
				memcpy(tmpBuffer, &buffer[iPosition[1]+1],datalen);
				flag = 0xff;
			}
			if(datalen>ICCARD_DATA_LEN_MAX)return;
			if(0xff==flag)
            {
	            //memcpy(tmpBuffer, &buffer[iPosition[0]+1],datalen);
							memset(&stTmpExdevice,0x0,sizeof(ST_PROTOCOL_EXDEVICE));
	            datalen=ICCardBase_ExtractExdevice(tmpBuffer,datalen,&stTmpExdevice);
	            if(0==datalen)return;
	            ///////////////////////
	            s_stICCardAttrib.onlinestatus = ICCARD_ONLINE_STASUS_ON;
	            Io_WriteAlarmBit(ALARM_BIT_IC_ERROR,RESET);
	            LZM_PublicKillTimer(&s_stICTimer[IC_TIMER_TIMEOUT]);                
	            ////////////////////////////////
	            //#if (ICCARD_EEYE == ICCARD_SEL)
	            //ICCard_DisposeRecvExdevice(stTmpExdevice,datalen);
	            //#elif (ICCARD_JTD == ICCARD_SEL) 
	            //ICCard_DisposeRecvExdeviceJTB(stTmpExdevice,datalen);
	            //#endif  
				if(SwipCardCh)
				{
					ICCard_DisposeRecvExdevice(stTmpExdevice,datalen);
				}
				else
				{
					ICCard_DisposeRecvExdeviceJTB(stTmpExdevice,datalen);
				}
	            break;	     
            }
        }
    }
}
/*************************************************************
** ��������: ICCard_SendCmdForReadDriverInfo
** ��������: 
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
unsigned char ICCard_SendCmdForReadDriverInfo(void)
{  
    u8 TempData[2];
    return ICCard_PackAndSendData(0x43,TempData,0);
}
/*************************************************************
** ��������: ICCard_DisposeReadDriverInfoTimeOut
** ��������: 
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
void ICCard_DisposeReadDriverInfoTimeOut(void)
{
    ///////////////////////////////
    s_stICCardAttrib.firstcheckflag = 0;
    if(DRIVER_SIGN_IN != s_stICCardAttrib.presignstatus)return;
    if(s_stICCardAttrib.signstatus == DRIVER_SIGN_EMPTY)
    {
        s_stICCardAttrib.signstatus = DRIVER_SIGN_OUT;
        ICCard_DisposeSignInAndOut();
    }
}
/*************************************************************
** ��������: ICCard_DisposeReadDriverInfoTimeOut
** ��������: 
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
void ICCard_DisposeCommTimeOut(void)
{
    if(ICCardBase_GetUseIntFlag())return;
    s_stICCardAttrib.onlinestatus = ICCARD_ONLINE_STASUS_OFF;
    Io_WriteAlarmBit(ALARM_BIT_IC_ERROR,SET);
}
/*************************************************************
** ��������: ICCard_StartCheckCommTimeOut
** ��������: 
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
void ICCard_StartCheckCommTimeOut(void)
{
    LZM_PublicSetOnceTimer(&s_stICTimer[IC_TIMER_TIMEOUT],PUBLICSECS(4),ICCard_DisposeCommTimeOut);
}
/*************************************************************
** ��������: ICCard_StartCheckReadDriverInfoTimeOut
** ��������: 
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
void ICCard_StartCheckReadDriverInfoTimeOut(void)
{
    s_stICCardAttrib.firstcheckflag = 1;
    LZM_PublicSetOnceTimer(&s_stICTimer[IC_TIMER_TEST],PUBLICSECS(15),ICCard_DisposeReadDriverInfoTimeOut);
}
/*************************************************************
** ��������: ICCard_ReadDriverInfo
** ��������: 
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
void ICCard_ReadDriverInfo(void)
{
    if(ICCardBase_GetUseIntFlag())//ͨ������CPUͨ��I2C��IC��
    {
        ICCard_StartCheckReadDriverInfoTimeOut();
        return;
    }
    ////////////////////////
    if(0==ICCard_SendCmdForReadDriverInfo())
    {
        ICCard_StartCheckCommTimeOut();
        ICCard_StartCheckReadDriverInfoTimeOut();
    }
    else
    {
        LZM_PublicSetOnceTimer(&s_stICTimer[IC_TIMER_TEST],PUBLICSECS(2),ICCard_ReadDriverInfo);
    }
}
/*************************************************************OK
** ��������: ICCard_FlashIndexReadAndCheck
** ��������: ��ȡ������������������Ƿ����
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: 
             
             
*************************************************************/
void ICCard_FlashIndexReadAndCheck(void) //
{
    STDRIVERSIGN stDriverSign;
    ///////����������/////////////
    ICCard_FlashIndexRead();    
    /////////////////////////////////
    if((s_stSignSaveIndex.recordCnt>DRIVERSIGN_MAX_RECORD+1)
      ||(s_stSignSaveIndex.sendrecordCnt>s_stSignSaveIndex.recordCnt)
      ||(s_stSignSaveIndex.saveflag != SAVE_INDEX_FLAG))
    {
        ICCard_SignFlashIndexInit();
    }
    ///////////////////////////////
    ICCard_CheckAndMoveSignData();
    //////////////////////////////
    if(s_stSignSaveIndex.recordCnt)
    {
        ICCard_GetSignRecordData(s_stSignSaveIndex.recordCnt,&stDriverSign);
        if((stDriverSign.len>ICCARD_DRIVER_SIGN_DATA_LEN_MAX)
        ||((stDriverSign.data[0]!=DRIVER_SIGN_IN)&&(stDriverSign.data[0]!=DRIVER_SIGN_OUT))
        ||((stDriverSign.data[0]==DRIVER_SIGN_OUT)&&(stDriverSign.len!=7)))
        {
            ICCard_SignFlashIndexInit();
            return;
        }
        //////////////////////////
        memcpy(&s_stDriverSign,&stDriverSign,STDRIVERSIGN_SIZE);
    }
    /////////////////////////////
    if(s_stSignSaveIndex.recordCnt<DRIVERSIGN_MAX_RECORD)
    {
        ICCard_GetSignRecordData(s_stSignSaveIndex.recordCnt+1,&stDriverSign);
        if(stDriverSign.len !=0xff||stDriverSign.data[0]!=0xff)
        {
            ICCard_SignFlashIndexInit();
        }
    }
}
/*************************************************************
** ��������: ICCard_ReadE2ParamDriverInfo
** ��������: ������ʼ��
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void ICCard_ReadE2ParamDriverInfo(void) 
{
    unsigned char buffer[30];
    if(EepromPram_ReadPram(E2_CURRENT_LICENSE_ID, buffer))
    {
        memcpy(s_stICCardAttrib.preDriverID,buffer,18);
        memset(buffer,0x30,18);
        if(Public_CheckArrayValIsEqual(s_stICCardAttrib.preDriverID, buffer, 18))//������0x30
        {
            memset(buffer,0,18);
            if(Public_CheckArrayValIsEqual(s_stICCardAttrib.preDriverID, buffer, 18))//������0x00
            {
                s_stICCardAttrib.presignstatus = DRIVER_SIGN_IN;
                return;
            }
        }
    }
    /////////////////////////////
    s_stICCardAttrib.signstatus = DRIVER_SIGN_OUT;
    s_stICCardAttrib.presignstatus = DRIVER_SIGN_OUT;
    memset(s_stICCardAttrib.preDriverID,0x30,18);
}
/*************************************************************
** ��������: ICCard_CheckForVoicePrompt
** ��������: 
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
unsigned char ICCard_CheckForVoicePrompt(void)
{
    if(1!=s_stICCardAttrib.voicePrompt)return 0;
    if(TiredDrive_GetLoginFlag())return 0;
    if(TiredDrive_GetCurContDriveTimeSec()>=ICCARD_PLAY_REMIND_TOTAL_TIME)return 0;
    return 1;
}
/*************************************************************
** ��������: ICCard_PlayVoicePromptSignIn
** ��������: 
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void ICCard_PlayVoicePromptSignIn(void)
{
    ICCard_ShowAndPlayAlarmStr("��ͣ�������IC��");
}
/*************************************************************
** ��������: ICCard_CheckForVoicePromptSignIn
** ��������: 
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void ICCard_CheckForVoicePromptSignIn(void)
{
    unsigned char runStatus;
    if(0==ICCard_CheckForVoicePrompt())
    {
        s_stICCardAttrib.playTimeCnt = 0;
        return;
    }
    ////////////////////
    runStatus = SpeedMonitor_GetCurRunStatus();
    if(runStatus)
    {
        if(0==s_stICCardAttrib.playTimeCnt)
        {
            s_stICCardAttrib.playflag = 1;
            s_stICCardAttrib.OnceCnt  = ICCARD_PLAY_REMIND_ONCE_TIME;
            s_stICCardAttrib.OnceNumber = 0;            
        }
    }
    else
    if(0==s_stICCardAttrib.playTimeCnt)
    {
        return;
    }    
    ///////////////////////
    s_stICCardAttrib.playTimeCnt++;
    if(s_stICCardAttrib.playTimeCnt>=ICCARD_PLAY_REMIND_GPOUP_TIME)
    {
        s_stICCardAttrib.playTimeCnt = 0;
    }
    //////////////////////////////
    if(0==s_stICCardAttrib.playflag)return;
    ///////////////////
    s_stICCardAttrib.OnceCnt++;
    if(s_stICCardAttrib.OnceCnt>=ICCARD_PLAY_REMIND_ONCE_TIME&&runStatus)
    {
        s_stICCardAttrib.OnceCnt = 0;
        //////////////////
        ICCard_PlayVoicePromptSignIn();//������ʾ
        ///////////////////
        s_stICCardAttrib.OnceNumber++;        
        if(s_stICCardAttrib.OnceNumber>=ICCARD_PLAY_REMIND_ONCE_NUMBER)
        {
            s_stICCardAttrib.OnceNumber = 0;
            s_stICCardAttrib.playflag =0;
            s_stICCardAttrib.OnceCnt =0;
        }
    }
}
/*************************************************************
** ��������: ICCard_CheckForVoicePromptSignIn
** ��������: 
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void ICCard_UpdataParamVoicePrompt(void)
{
    unsigned char flag;
    LZM_PublicKillTimer(&s_stICTimer[IC_TIMER_CHECK]);
    if(EepromPram_ReadPram(E2_UNLOGIN_TTS_PLAY_ID, &flag))
    {
        s_stICCardAttrib.voicePrompt = flag;
        if(1==s_stICCardAttrib.voicePrompt)
        {
            LZM_PublicSetCycTimer(&s_stICTimer[IC_TIMER_CHECK],PUBLICSECS(1),ICCard_CheckForVoicePromptSignIn);
        }
    }
    else
    {
        s_stICCardAttrib.voicePrompt = 0;
    }
}
/*************************************************************
** ��������: ICCard_GetExternalDriverInfoForSignIn
** ��������: ��ȡ��ʻԱ���뼰����
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
unsigned char ICCard_GetExternalDriverInfoForSignIn(unsigned char *pBuffer)
{
    unsigned char len;
    if( s_stICCardAttrib.signstatus != DRIVER_SIGN_IN) return 0;
    len = 0;
    memcpy(&pBuffer[len],&s_stICCardAttrib.stDriver.DriverID[8],6);
    len +=6;
    pBuffer[len++]=s_stICCardAttrib.stDriver.NameLen;
    memcpy(&pBuffer[len],&s_stICCardAttrib.stDriver.Name,s_stICCardAttrib.stDriver.NameLen);
    len +=s_stICCardAttrib.stDriver.NameLen;
    return len;
}
/*************************************************************
** ��������: ICCard_GetExternalDriverInfo
** ��������: ��ȡ��ʻԱ��������ʻ֤����
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
unsigned char ICCard_GetExternalDriverInfo(unsigned char *pBuffer)
{
    unsigned char len;
    if( s_stICCardAttrib.signstatus != DRIVER_SIGN_IN) return 0;
    len = 0;
    memcpy(&pBuffer[len],&s_stICCardAttrib.stDriver.Name,s_stICCardAttrib.stDriver.NameLen);
    len +=s_stICCardAttrib.stDriver.NameLen;
    pBuffer[len++]=',';
    memcpy(&pBuffer[len],s_stICCardAttrib.stDriver.DriverID,18);
    len +=18;
    return len;
}
/*************************************************************
** ��������: ICCard_GetExternalDriverInfo
** ��������: ��ȡ��ʻԱ����
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
unsigned char ICCard_GetExternalDriverName(unsigned char *pBuffer)
{
        unsigned char len;
        
	if( s_stICCardAttrib.signstatus != DRIVER_SIGN_IN) return 0;
        len=s_stICCardAttrib.stDriver.NameLen;
        
	memcpy(pBuffer,&s_stICCardAttrib.stDriver.Name,len);
	return len;
}
/*************************************************************
** ��������: ICCard_GetExternalDriverInfo
** ��������: ��ȡ��ʻԱ��ʻ֤����
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
unsigned char ICCard_GetExternalDriverDriveID(unsigned char *pBuffer)
{
      u8 len;
	if( s_stICCardAttrib.signstatus != DRIVER_SIGN_IN) return 0;
        len=s_stICCardAttrib.stDriver.QCID[0];
	memcpy(pBuffer,&s_stICCardAttrib.stDriver.QCID[len+1],18);
	return 18;
}

/*************************************************************
** ��������: ICCard_TimeTask
** ��������: ��ʱ��������
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
FunctionalState ICCard_TimeTask(void)
{
    LZM_PublicTimerHandler(s_stICTimer,IC_TIMERS_MAX);
    ICCardBase_TimerTask();
    return ENABLE;
}
/*************************************************************
** ��������: ICCard_ParamInit
** ��������: ������ʼ��
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void ICCard_TimeTaskStart(void) 
{
    memset(&s_stICCardAttrib,0,sizeof(ST_ICCARD_ATTRIB));
    /////////////////////////
    ICCard_ReadE2ParamDriverInfo();
    //////////////////////////////
    ICCard_FlashIndexReadAndCheck();
    /////////////////////
    ICCard_UpdataParamVoicePrompt();
    ///////////////////////
    if(0==ICCardBase_GetUseIntFlag())
    {
        CommICCardInit();
    }
    //////////////////////////////////////////////////////////
    LZM_PublicSetOnceTimer(&s_stICTimer[IC_TIMER_TEST],PUBLICSECS(10),ICCard_ReadDriverInfo);
    ///////////////////
    LZM_PublicSetOnceTimer(&s_stICTimer[IC_TIMER_CHECK],PUBLICSECS(25),ICCard_UpdataParamVoicePrompt);
}
/*************************************************************
** ��������: ICCard_ParamInit
** ��������: ������ʼ��
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void ICCard_ParamInit(void) 
{
    LZM_PublicKillTimerAll(s_stICTimer,IC_TIMERS_MAX); 
    ////////////////////////
    memset(&s_stICCardAttrib,0,sizeof(ST_ICCARD_ATTRIB));
    /////////////////////////    
    ////////////////////////
    SetTimerTask(TIME_IC_CARD,LZM_TIME_BASE);
    //////////////////////////////////////////////////////////
    LZM_PublicSetOnceTimer(&s_stICTimer[IC_TIMER_TEST],LZM_AT_ONCE,ICCard_TimeTaskStart);
}
/******************************************************************************
**                            End Of File
******************************************************************************/

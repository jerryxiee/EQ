/*******************************************************************************
 * File Name:           OpenAccount.c 
 * Function Describe:   
 * Relate Module:       ���ٿ�������
 * Writer:              Joneming
 * Date:                2013-06-17 
 * ReWriter:            Joneming
 * Date:                //���п�������֮ǰ�Ȱѿ����������赽��Ӧ��IP��ַ��,�����������Ķ˿ں��赽��Ӧ�Ķ˿ںŵ�ַ��,
                        //��������:�ն˴���IP������,�ѿ�����������ַ����IP��ַ����,�ն������ӿ���������,���ӳɹ��󷢿���ָ��
                        //���յ�����Ӧ���,�ն��ٻָ�ԭ������IP��ַ,֮���ٽ�������������
 *******************************************************************************/
#include "include.h"
////////////////////////////////
typedef enum
{
    OA_STATUS_SAVE_IP,   //����IP��ַ
    OA_STATUS_UNLOCK,    //����,
    OA_STATUS_LOCK,      //����,
    OA_STATUS_VALID_FLAG,//��Ч��־
    OA_STATUS_MAX         //
}E_OA_STATUS;

enum 
{
    ACCOUNT_TIMER_TASK,//
    ACCOUNT_TIMER_SEND,//
    ACCOUNT_TIMERS_MAX
}T_STACCOUNTTIME;

static LZM_TIMER s_stAccountTimer[ACCOUNT_TIMERS_MAX];

static unsigned char s_ucfisrtRunFlag = 0;
static unsigned char s_ucOpenAccountStatus = 0;
/////////////////////////
void OpenAccount_RecoverOriginalIPAddress(void);
/*************************************************************
** ��������: OpenAccount_SaveStatus
** ��������: ������ٿ�������״̬
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void OpenAccount_SaveStatus(void)
{
    PUBLIC_SETBIT(s_ucOpenAccountStatus, OA_STATUS_VALID_FLAG);    
    Public_WriteDataToFRAM(FRAM_QUICK_OPEN_ACCOUNT_ADDR, &s_ucOpenAccountStatus,FRAM_QUICK_OPEN_ACCOUNT_LEN);
}
/*************************************************************
** ��������: OpenAccount_ReadLockStatus
** ��������: ��ȡ���ٿ�������״̬
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void OpenAccount_ReadLockStatus(void)
{
    unsigned char max;
    unsigned char flag = 0;    
    if(Public_ReadDataFromFRAM(FRAM_QUICK_OPEN_ACCOUNT_ADDR, &flag,FRAM_QUICK_OPEN_ACCOUNT_LEN))
    {
        max = 0;
        PUBLIC_SETBIT(max, OA_STATUS_MAX);
        if(flag>=max)
        {
            flag = 0;
        }
        else
        if(!PUBLIC_CHECKBIT(flag,OA_STATUS_VALID_FLAG))
        {
            flag = 0;
        }
        else
        if((PUBLIC_CHECKBIT(flag,OA_STATUS_UNLOCK)&&PUBLIC_CHECKBIT(flag,OA_STATUS_LOCK))
         ||(!PUBLIC_CHECKBIT(flag,OA_STATUS_UNLOCK)&&!PUBLIC_CHECKBIT(flag,OA_STATUS_LOCK)))
        {
            flag = 0;
        }         
    }
    s_ucOpenAccountStatus = flag;
    ///////////////////
    if(0==s_ucOpenAccountStatus)//
    {
        s_ucOpenAccountStatus = 0;
        PUBLIC_SETBIT(s_ucOpenAccountStatus,OA_STATUS_UNLOCK);
        PUBLIC_CLRBIT(s_ucOpenAccountStatus,OA_STATUS_LOCK);
        OpenAccount_SaveStatus();
    }    
}
/*************************************************************
** ��������: OpenAccount_GetCurLockStatus
** ��������: ��ÿ��ٿ�������״̬
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
unsigned char OpenAccount_GetCurLockStatus(void)
{
    if(PUBLIC_CHECKBIT(s_ucOpenAccountStatus,OA_STATUS_LOCK)&&0==PUBLIC_CHECKBIT(s_ucOpenAccountStatus,OA_STATUS_UNLOCK))return 1;
    return 0;
}
/*************************************************************
** ��������: OpenAccount_SetLockStatus
** ��������: ���ٿ�����Ϊ����״̬
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void OpenAccount_SetLockStatus(void)
{
    PUBLIC_SETBIT(s_ucOpenAccountStatus,OA_STATUS_LOCK);
    PUBLIC_CLRBIT(s_ucOpenAccountStatus,OA_STATUS_UNLOCK);
    OpenAccount_SaveStatus();
}
/*************************************************************
** ��������: OpenAccount_ClearLockStatus
** ��������: ���ٿ����������״̬
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void OpenAccount_ClearLockStatus(void)
{
    ClearTerminalAuthorizationCode(CHANNEL_DATA_1);
    ClearTerminalAuthorizationCode(CHANNEL_DATA_2);
    ///////////////////////
    PUBLIC_SETBIT(s_ucOpenAccountStatus,OA_STATUS_UNLOCK);
    PUBLIC_CLRBIT(s_ucOpenAccountStatus,OA_STATUS_LOCK);
    OpenAccount_SaveStatus();
    LZM_PublicKillTimer(&s_stAccountTimer[ACCOUNT_TIMER_SEND]);
    LZM_PublicKillTimer(&s_stAccountTimer[ACCOUNT_TIMER_TASK]);
    ///////////////////////////////////
    OpenAccount_RecoverOriginalIPAddress();
}
/*************************************************************
** ��������: OpenAccount_CheckOnlineAndSendQuickOpenAccount
** ��������: 
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void OpenAccount_CheckOnlineAndSendQuickOpenAccount(void)
{
    u8 channel = CHANNEL_DATA_1;//Ϊ����ͨ�������
    if(1==communicatio_GetMudulState(COMMUNICATE_STATE))//
    {
        if(ACK_OK==RadioProtocol_OpenAccount(channel))//
        {
            LZM_PublicKillTimer(&s_stAccountTimer[ACCOUNT_TIMER_SEND]);
            LZM_PublicSetOnceTimer(&s_stAccountTimer[ACCOUNT_TIMER_TASK],PUBLICSECS(30),OpenAccount_CheckAndDisposeLockStatus);
        }
    }   
}
/*************************************************************
** ��������: OpenAccount_SetParamForLinkToFreightPlatform
** ��������: ���ӵ�ָ���Ļ���ƽ̨(������������:����IP����Ϊ����������,UDP����Ϊ�����˿ں�)
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
unsigned char OpenAccount_ChangeMainIPAndBackUpIP(void)
{
    unsigned char len;
    unsigned char buffer[30]={0};
    unsigned char IPBuff[30];           //IPBuff
    unsigned char IPBakBuff[30];         //IPBuff
    unsigned char TCPBuff[6];           //TCPBuff    
    unsigned char TCPBakBuff[6];         //TCPBuff
    unsigned char IPlen,IPBaklen,TCPlen,TCPBaklen;
    
    IPlen = EepromPram_ReadPram(E2_MAIN_SERVER_IP_ID,IPBuff);
    TCPlen = EepromPram_ReadPram(E2_MAIN_SERVER_TCP_PORT_ID,TCPBuff);
    IPBaklen = EepromPram_ReadPram(E2_OPEN_ACCOUNT_PARAM_IP_ID,IPBakBuff);
    TCPBaklen =EepromPram_ReadPram(E2_OPEN_ACCOUNT_PARAM_TCP_ID,TCPBakBuff);        
    if(0==IPlen||0==TCPlen||0==IPBaklen||0==TCPBaklen)//
    {
       return 1;
    }
    ////////////////����IP��ַд���ݴ��ַ������//////////////////////////////
    EepromPram_WritePram(E2_OPEN_ACCOUNT_PARAM_IP_ID, IPBuff, IPlen);
    len=EepromPram_ReadPram(E2_OPEN_ACCOUNT_PARAM_IP_ID,buffer);
    if(Public_CheckArrayValIsEqual(buffer,IPBuff,len)||(len!=IPlen))
    {
        return 1;
    }
    ////////////��TCPд���ݴ��ַ������////////////////
    EepromPram_WritePram(E2_OPEN_ACCOUNT_PARAM_TCP_ID,TCPBuff,TCPlen);
    len=EepromPram_ReadPram(E2_OPEN_ACCOUNT_PARAM_TCP_ID,buffer);
    if(Public_CheckArrayValIsEqual(buffer,TCPBuff,len)||(len!=TCPlen))
    {
        return 1;
    }
    ////////�ѿ�����������ַд����IP///////////////
    EepromPram_WritePram(E2_MAIN_SERVER_IP_ID, IPBakBuff, IPBaklen);
    len=EepromPram_ReadPram(E2_MAIN_SERVER_IP_ID,buffer);
    if(Public_CheckArrayValIsEqual(buffer,IPBakBuff,len)||(len!=IPBaklen))
    {
        return 1;
    }
    ///////////�ѿ����˿ںŵ�ַд��TCP////////////
    EepromPram_WritePram(E2_MAIN_SERVER_TCP_PORT_ID, TCPBakBuff, TCPBaklen);
    len=EepromPram_ReadPram(E2_MAIN_SERVER_TCP_PORT_ID,buffer);
    if(Public_CheckArrayValIsEqual(buffer,TCPBakBuff,len)||(len!=TCPBaklen))
    {
        return 1;
    }
    return 0;
}
/*************************************************************
** ��������: OpenAccount_SetParamForLinkToFreightPlatform
** ��������: ���ӵ�ָ���Ļ���ƽ̨(������������:����IP����Ϊ����������,UDP����Ϊ�����˿ں�)
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
unsigned char OpenAccount_SetParamForLinkToFreightPlatform(void)
{
    unsigned char result;
    if(0==PUBLIC_CHECKBIT(s_ucOpenAccountStatus,OA_STATUS_SAVE_IP))
    {
        result=OpenAccount_ChangeMainIPAndBackUpIP();
        if(0==result)
        {
            PUBLIC_SETBIT(s_ucOpenAccountStatus,OA_STATUS_SAVE_IP);
            OpenAccount_SaveStatus();
            Communication_Close();
            LZM_PublicSetOnceTimer(&s_stAccountTimer[ACCOUNT_TIMER_SEND],PUBLICSECS(2),Communication_Open);
        }
        return result;
    }
    return 0;
}
/*************************************************************
** ��������: OpenAccount_LinkGovServer
** ��������: ���ӵ�ָ���Ļ���ƽ̨
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void OpenAccount_LinkGovServer(void)
{
    //SetEvTask(EV_LINK_GOV_SERVER);    
    LZM_PublicSetCycTimer(&s_stAccountTimer[ACCOUNT_TIMER_SEND],PUBLICSECS(5),OpenAccount_CheckOnlineAndSendQuickOpenAccount); 
}
/*************************************************************
** ��������: OpenAccount_LinkToFreightPlatform
** ��������: ���ӵ�ָ���Ļ���ƽ̨
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void OpenAccount_LinkToFreightPlatform(void)
{
    //Communication_Close();
    if(0==OpenAccount_SetParamForLinkToFreightPlatform())
    {
        LZM_PublicSetOnceTimer(&s_stAccountTimer[ACCOUNT_TIMER_TASK],PUBLICSECS(6),OpenAccount_LinkGovServer);
    }
    else
    {
        Public_ShowTextInfo("�������ó���", PUBLICSECS(3));
    }
    //LZM_PublicSetOnceTimer(&s_stAccountTimer[ACCOUNT_TIMER_SEND],PUBLICSECS(2),Communication_Open);
    /////////////////////////////    
}
/*************************************************************
** ��������: OpenAccount_CheckAndDisposeLockStatus
** ��������: ���ʹ�����ٿ�������״̬
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void OpenAccount_CheckAndDisposeLockStatus(void)
{
    if(0==s_ucfisrtRunFlag)
    {
        OpenAccount_ParameterInitialize();
    }
    ///////////////////////
    if(OpenAccount_GetCurLockStatus())//����
    {
        Public_ShowTextInfo("���ٿ�����", PUBLICSECS(20));
        OpenAccount_LinkToFreightPlatform();
    }
    else
    {
        Public_ShowTextInfo("�ն��ѽ���", PUBLICSECS(5));
    }
    ///////////////////////
}
/*************************************************************
** ��������: OpenAccount_CheckAndDisposeLockStatus
** ��������: ���ʹ�����ٿ�������״̬
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void OpenAccount_RecoverOriginalIPAddress(void)
{
    if(PUBLIC_CHECKBIT(s_ucOpenAccountStatus,OA_STATUS_SAVE_IP))
    {
        Communication_Close();
        ////////////////////////
        if(0==OpenAccount_ChangeMainIPAndBackUpIP())
        {
            PUBLIC_CLRBIT(s_ucOpenAccountStatus,OA_STATUS_SAVE_IP);
            OpenAccount_SaveStatus();
        }
        //////////////////////////////
        Communication_Open();
    }
}
/*************************************************************
** ��������: OpenAccount_DisposeRadioProtocol
** ��������: ���ٿ���Э�����
** ��ڲ���: pBuffer:�����׵�ַ,BufferLen:���ݳ���
** ���ڲ���: ��
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void OpenAccount_DisposeRadioProtocol(unsigned char *pBuffer,unsigned short BufferLen)
{
    unsigned short ID;
    unsigned char result;
    ID=Public_ConvertBufferToShort(&pBuffer[2]);
    if(0x0110==ID)//���ٿ���
    {
        result = pBuffer[4];
        if(1 == result)
        {
            OpenAccount_SetLockStatus();
            Public_ShowTextInfo("�ն˽���ʧ��", PUBLICSECS(6));
            LZM_PublicKillTimer(&s_stAccountTimer[ACCOUNT_TIMER_TASK]);
            OpenAccount_RecoverOriginalIPAddress();  
        }
        else
        //if(0 == result||5 == result)//�����ɹ�
        {
            OpenAccount_ClearLockStatus();
            Public_ShowTextInfo("�ն˽����ɹ�", PUBLICSECS(5));
            LZM_PublicSetOnceTimer(&s_stAccountTimer[ACCOUNT_TIMER_TASK],PUBLICSECS(4),ShowMainInterface);           
        }
    }
}
/*************************************************************
** ��������: OpenAccount_GetEncryptFlag
** ��������: �Ƿ����
** ��ڲ���: 
** ���ڲ���: ��
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
unsigned char OpenAccount_GetEncryptFlag(void)
{
    unsigned char flag;
    if(!EepromPram_ReadPram(E2_TOPLIGHT_CONTROL_ID, &flag))
    {
        flag = 0;
    }
    return (1==flag)?1:0;
}
/*************************************************************
** ��������: OpenAccount_TimeTask
** ��������: ���ٿ�����ʱ��������
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
FunctionalState OpenAccount_TimeTask(void)
{
    LZM_PublicTimerHandler(s_stAccountTimer,ACCOUNT_TIMERS_MAX);
    return ENABLE;
}
/*************************************************************
** ��������: OpenAccountParameterInitialize
** ��������: ���ٿ���������ʼ��
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void OpenAccount_ParameterInitialize(void) 
{
    LZM_PublicKillTimerAll(s_stAccountTimer,ACCOUNT_TIMERS_MAX);
    ////////////////////////////// 
    OpenAccount_ReadLockStatus();    
    if(OpenAccount_GetCurLockStatus())
    {
        LZM_PublicSetOnceTimer(&s_stAccountTimer[ACCOUNT_TIMER_TASK],PUBLICSECS(15),OpenAccount_CheckAndDisposeLockStatus);
    }
    /////////////////////////
    s_ucfisrtRunFlag = 1;
    SetTimerTask(TIME_ACCOUNT,2);
}
/*******************************************************************************
 *                             end of module
 *******************************************************************************/


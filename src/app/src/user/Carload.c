/*******************************************************************************
 * File Name:			CarLoad.c 
 * Function Describe:	����ģ��
 * Relate Module:		�������Э�顣
 * Writer:				Joneming
 * Date:				2013-05-20
 * ReWriter:			
 * Date:				
 *******************************************************************************/
#include "include.h"
/*******************************************************************************/
//////////cmd////////////////////////////
#define CARLOAD_COMM_CMD_QUERY_COM_WEIGHT           0x05	//�ۺ�����
#define CARLOAD_COMM_CMD_QUERY_SENSOR_ABNORMAL      0x07	//0x05
#define CARLOAD_COMM_CMD_SERVER_SENSOR_CLEAR        0x71	//ƽ̨����
#define CARLOAD_COMM_CMD_SENSOR_CLEAR               0x72	//�ն�����
#define CARLOAD_COMM_CMD_CAR_STATUS                 0xC1	//����״̬
#define CARLOAD_COMM_CMD_UPDATA                     0xC0	//����
///////////////////////////////////

#define CARLOAD_RADIO_QUERY_SENSOR_ABNORMAL         0x00 //��ѯ���ش���������
#define CARLOAD_RADIO_CLEAR_SENSOR_ABNORMAL         0x01 //�������ش���������
#define CARLOAD_RADIO_FREIGHT_INFORMATION           0x02 //�˻���Ϣ
#define CARLOAD_RADIO_DEVELOP_PORT                  0xF0 //�����ӿ�
///////////////////////////////////////////////////////////////
#define CARLOAD_RADIO_CMD_MAX                       0xFF //�������
////////////////////////////////////////////////
//////////////////////////////////
#define CARLOAD_RX_BUFFER                           256//���ջ�����
#define CARLOAD_TX_BUFFER                           256//���ͻ�����
////////////////////////
#define WEIGHT_OVERHEAD_INFO_ID                     0xE2 //������ϢID
#define WEIGHT_OVERHEAD_INFO_LEN                    4  //������Ϣ����

//#define WEIGHT_USER_STATUS_ID                       0xE1 //������ϢID
//#define WEIGHT_USER_STATUS_LEN                      4  //������Ϣ����

//#define DEFINE_BIT_19                  19 //���ر���λ
//#define DEFINE_BIT_25            25 //���������ϱ���λ
/*******************************************************************************/
/*******************************************************************************/
/////////////////////////////////
#define CARLOAD_USER_DATA_START         4
#define CARLOAD_USER_DATA_MIN_LEN       6
/////////////////////////
enum
{
    LEDPORT_BIT_LED     =0,     //����
    LEDPORT_BIT_TOPLIGHT,       //����
    LEDPORT_BIT_PJA,            //������
    LEDPORT_BIT_LOAD_SENSOR,    //���ش�����
    LEDPORT_BIT_MAX             //���ֵ
}E_32PINLEDPORT;//����32pin��LED�ӿڽӵ���������,
////////////////////
typedef enum
{
    COMM_RECV_OK,             //������ȷ
    COMM_RECV_ERR_START,      //��ͷ��־����
    COMM_RECV_ERR_TYPE,       //���Ͳ���
    COMM_RECV_ERR_LEN,        //���Ȳ���
    COMM_RECV_ERR_END,        //������־����
    COMM_RECV_ERR_CMD,        //�����
    COMM_RECV_ERR_VERIFY,     //У�鲻��
    COMM_RECV_MAX             //
}CARLOAD_RECV;

enum
{
    CARLOAD_CMD_TYPE_READ=0,      //
    CARLOAD_CMD_TYPE_WRITE,       //
    CARLOAD_CMD_TYPE_MAX          //���ֵ
}E_CARLOAD_CMD_TYPE;//
////////////////////

typedef enum
{
    CARLOAD_EMPTY_STATE,             //��
    CARLOAD_CHECK_STATE,             //�����·
    CARLOAD_MAX_STATE                //״̬��Ŀ
}CARLOAD_STATE;
////////////////
enum 
{
    CL_TIMER_TASK,                  //0 timer
    CL_TIMER_TIME_OUT,              //1 timer
    CL_TIMER_SEND_DATA,             //2 timer
    CL_TIMER_CAR_STATUS,            //3 timer
    CL_TIMERS_MAX
}CARLOADTIME;

static LZM_TIMER s_stCLTimer[CL_TIMERS_MAX];
/////////////////////////////////////

typedef struct
{
    unsigned long userAlarmMask; //�����Զ���״̬��
    unsigned long userAlarm;       //�Զ���״̬��
    unsigned long maxweight;        //�������
    unsigned long sensorAbnormal;   //�������쳣
    unsigned long clearresult;      //������������
    unsigned long totalWeight;      //�������    
    unsigned char status;           //״̬
    unsigned char clearCnt;         //�������������
    unsigned char CheckCnt;         //����ã�������
    unsigned char overloadflag;     //���ر�־
    unsigned char platformCmd;      //ƽָ̨��
    unsigned char recvCmd;          //��������
    unsigned char recvCmdEx;        //��������    
    unsigned char recvDataLen;      //�������ݳ���
    unsigned char freightInfo;      //������Ϣ
    unsigned char sendcnt;          //���ʹ���
    unsigned char onlinestatus;     //����״̬
    unsigned char lcdTransFlag;     //lcd͸����־
    unsigned char contrecvtimeoutCnt; //�������ճ�ʱ����
    unsigned char sendCmd;       //   
    unsigned char carStatusFlag;   //
    unsigned char loadSensorFlag;   // 
    unsigned char carStatusCnt;   //
    unsigned char parseData[CARLOAD_RX_BUFFER];
}T_CARLOAD_ATTRIB;

static T_CARLOAD_ATTRIB s_stCarLoadAttrib;

typedef struct
{
	unsigned char SendBuffer[CARLOAD_TX_BUFFER+1];
	unsigned short SendLen;   
	unsigned short SendCount;
}STSCARLOAD_TX;

static STSCARLOAD_TX s_stCarLoadTx;

/****************************************************/
/*                                                  */
/* Local Definitions and macros                     */
/*                                                  */
/****************************************************/
void CarLoad_ResendData(void);
unsigned char CarLoad_SendData(unsigned char *pBuffer, unsigned short BufferLen);
void Carload_SendCmdQueryTotalWeight(void);
void Carload_SendCmdQuerySensorAbnormal(void);
void Carload_SendCmdClearServerSensorAbnormal(void);
void CarLoad_SendDevelopPortResponse(void);
void CarLoad_SendClearSensorAbnormalResult(void);
void CarLoad_SendQuerySensorAbnormalResult(void);
void Carload_SendCmdCurCarStatus(void);
void CarLoad_DisposeLoadSensorTransToLcd(void);

/*************************************************************
** ��������: CarLoad_Get32PINLedPortInsertLoadSensorFlag
** ��������: �Ƿ�������ش�����
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
unsigned char CarLoad_Get32PINLedPortInsertLoadSensorFlag(void)
{
    return s_stCarLoadAttrib.loadSensorFlag;
}
/*************************************************************
** ��������: CarLoad_PactAndSendData
** ��������: �����ݽ��д��,������
** ��ڲ���: cmd:���CmdType:0�Ƕ�,1��д,data:���ݿ��׵�ַ,datalen:����		 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
ProtocolACK CarLoad_PactAndSendData(unsigned char cmd,unsigned char CmdType,unsigned char *data,unsigned short datalen)
{
    unsigned char result;
    unsigned short i;
    unsigned short len;
    unsigned short VerifyCode;  
    unsigned char TxBuffer[CARLOAD_TX_BUFFER];//����
    ///////////////////////
    if(s_stCarLoadAttrib.contrecvtimeoutCnt>=5)return ACK_ERROR;
    ////////////////////////////////
    if(datalen+CARLOAD_USER_DATA_MIN_LEN>CARLOAD_TX_BUFFER)return ACK_ERROR;    
    i=0;
    ///////////////////////
    TxBuffer[i++]=0x69; //ͬ����
    TxBuffer[i++]=0x01;//��ַ��
    ///////////////////////////
    TxBuffer[i++]=0x42;//��ַ��
    //////////////////////////////////
    if(CARLOAD_CMD_TYPE_WRITE==CmdType)
    {
        TxBuffer[i++] = cmd|0x80;
    }
    else
    {
        TxBuffer[i++] = cmd;
    }
    s_stCarLoadAttrib.sendCmd = TxBuffer[i-1];
    /////////////////////////////////////////////
    len = i;
    for(i=0;i<datalen;i++)TxBuffer[len+i]=data[i];
    /////////////////////////////////////////////
    len  +=datalen;
    VerifyCode=CRC16(TxBuffer,len);
    /////////////
    TxBuffer[len++]=(VerifyCode>>8)& 0xff;
    TxBuffer[len++]= VerifyCode& 0xff;  
    /////////////////////////////////
    ///////////////////////////
    s_stCarLoadAttrib.CheckCnt = 0;
    s_stCarLoadAttrib.lcdTransFlag = 0;
    ///////////////////
    if(CarLoad_Get32PINLedPortInsertLoadSensorFlag())
    {
        result=CarLoad_SendData(TxBuffer,len);
        if(result)return ACK_OK;
    }
    return ACK_ERROR;
}
/*************************************************************
** ��������: CarLoad_PactAndSendDataEx
** ��������: �����ݽ��д��,������
** ��ڲ���: cmd:���CmdType,data:���ݿ��׵�ַ,datalen:����		 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void CarLoad_PactAndSendDataEx(unsigned char cmd,unsigned char *data,unsigned short datalen)
{
    CarLoad_PactAndSendData(cmd,CARLOAD_CMD_TYPE_READ,data,datalen);
}
/*************************************************************
** ��������: CarLoad_ResendData
** ��������: 
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void CarLoad_ResendData(void)
{
    if(s_stCarLoadAttrib.contrecvtimeoutCnt>=5)
    {
        s_stCarLoadAttrib.CheckCnt = 0;
        s_stCarLoadAttrib.onlinestatus = 0;
        if(s_stCarLoadAttrib.carStatusFlag<2)s_stCarLoadAttrib.carStatusFlag=0xff;
        return;
    }
    //////////////////////////////
    s_stCarLoadAttrib.CheckCnt++;
    if(s_stCarLoadAttrib.CheckCnt < 5)
    {
        CarLoad_SendData(s_stCarLoadTx.SendBuffer,s_stCarLoadTx.SendLen);
    }
    else
    {
        s_stCarLoadAttrib.CheckCnt = 0;
        s_stCarLoadAttrib.onlinestatus = 0;
        if(s_stCarLoadAttrib.carStatusFlag<2)s_stCarLoadAttrib.carStatusFlag=0xff;
        LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_TIME_OUT],PUBLICSECS(30),Carload_SendCmdQueryTotalWeight);
    }
}
/*************************************************************
** ��������: CarLoad_SendData
** ��������: 
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
unsigned char CarLoad_SendData(unsigned char *pBuffer, unsigned short BufferLen)
{
    unsigned char result;
    s_stCarLoadTx.SendLen = BufferLen;
    if(BufferLen>CARLOAD_TX_BUFFER)return 0;
    memcpy(s_stCarLoadTx.SendBuffer,pBuffer,s_stCarLoadTx.SendLen);
    result=COM1_WriteBuff(pBuffer,BufferLen);
    if(result)
    {
        if(CARLOAD_COMM_CMD_QUERY_COM_WEIGHT == s_stCarLoadAttrib.sendCmd)
        {
            if(s_stCarLoadAttrib.contrecvtimeoutCnt<5)
            {
                s_stCarLoadAttrib.contrecvtimeoutCnt++;
            }
            else
            {
                return result;
            }
        }
    }
    if(0==LoadControllerUpdata_GetUpdataFlag())
    LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_TIME_OUT],PUBLICSECS(3),CarLoad_ResendData);
    return result;
}
/*************************************************************
** ��������: Carload_SendCmdQueryTotalWeight
** ��������: 
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void Carload_SendCmdQueryTotalWeight(void)
{
    unsigned char data[1];
    if(LoadControllerUpdata_GetUpdataFlag())return;
    CarLoad_PactAndSendDataEx(CARLOAD_COMM_CMD_QUERY_COM_WEIGHT,data,0);
    LZM_PublicKillTimer(&s_stCLTimer[CL_TIMER_CAR_STATUS]);
}

/*************************************************************
** ��������: Carload_SendCmdQuerySensorAbnormal
** ��������: 
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void Carload_SendCmdQuerySensorAbnormal(void)
{
    unsigned char data[1];
    CarLoad_PactAndSendDataEx(CARLOAD_COMM_CMD_QUERY_SENSOR_ABNORMAL,data,0);
}
/*************************************************************
** ��������: Carload_SendCmdClearServerSensorAbnormal
** ��������: ���������
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void Carload_SendCmdClearSensorAbnormal(void)
{
    unsigned char data[1];
    CarLoad_PactAndSendDataEx(CARLOAD_COMM_CMD_SENSOR_CLEAR,data,0);
}
/*************************************************************
** ��������: Carload_SendCmdClearServerSensorAbnormal
** ��������: ���������
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void Carload_SendCmdClearServerSensorAbnormal(void)
{
    unsigned char data[1];
    CarLoad_PactAndSendDataEx(CARLOAD_COMM_CMD_SERVER_SENSOR_CLEAR,data,0);
}
/*************************************************************
** ��������: Carload_SendCmdCurCarStatus
** ��������: 
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void Carload_SendCmdCurCarStatus(void)
{
    unsigned char data[10];
    unsigned char temp,len;
    unsigned short speed;
    if(LoadControllerUpdata_GetUpdataFlag())return;
    if(0==CarLoad_GetCurOnlineStatus())return;
    temp = 0;    
    //////////////////
    if(Io_ReadStatusBit(STATUS_BIT_ACC))
    PUBLIC_SETBIT(temp, 0);//acc
    //////////////////
    if(SpeedMonitor_GetCurRunStatus())//��ʻ״̬
    PUBLIC_SETBIT(temp, 1);//
    data[0] = temp;
    /////////////////////
    speed = SpeedMonitor_GetCurSpeed();
    speed *= 10;
    ///////////////////////
    Public_ConvertShortToBuffer(speed, &data[1]);
    Public_ConvertNowTimeToBCDEx(&data[3]);    
    if(0xff == s_stCarLoadAttrib.carStatusFlag)
    {
        s_stCarLoadAttrib.carStatusFlag = 0x55;
        len = 9;
    }
    else
    if(0x55 == s_stCarLoadAttrib.carStatusFlag)
    {
        s_stCarLoadAttrib.carStatusFlag = 0xff;
        len = 3;
    }
    else
    if(0x01 == s_stCarLoadAttrib.carStatusFlag)
    {
        len = 9;
    }
    else
    {
        len = 3;
    }
    CarLoad_PactAndSendDataEx(CARLOAD_COMM_CMD_CAR_STATUS,data,len);
}
/*******************************************************************************
**  ��������  : CarLoad_SetOverLoadAlarm
**  ��������  : ���ó��ر���λ
**  ��    ��  : ��				
**  ��    ��  : 
**  ȫ�ֱ���  : 
**  ���ú���  : 
**  �ж���Դ  : 
**  ��    ע  :
*******************************************************************************/
void CarLoad_SetOverLoadAlarm(void)
{
    Io_WriteSelfDefine2Bit(DEFINE_BIT_19,SET);
}
/*******************************************************************************
**  ��������  : CarLoad_ClearOverLoadAlarm
**  ��������  : ������ر���λ
**  ��    ��  : ��				
**  ��    ��  : 
**  ȫ�ֱ���  : 
**  ���ú���  : 
**  �ж���Դ  : 
**  ��    ע  :
*******************************************************************************/
void CarLoad_ClearOverLoadAlarm(void)
{
    Io_WriteSelfDefine2Bit(DEFINE_BIT_19,RESET);
}
/*******************************************************************************
**  ��������  : CarLoad_SetSensorAbnormalAlarm
**  ��������  : ���ô��������ϱ���λ
**  ��    ��  : ��				
**  ��    ��  : 
**  ȫ�ֱ���  : 
**  ���ú���  : 
**  �ж���Դ  : 
**  ��    ע  :
*******************************************************************************/
void CarLoad_SetSensorAbnormalAlarm(void)
{
    Io_WriteSelfDefine2Bit(DEFINE_BIT_25,SET); 
}
/*******************************************************************************
**  ��������  : CarLoad_ClearOverLoadAlarm
**  ��������  : ������������ϱ���λ
**  ��    ��  : ��				
**  ��    ��  : 
**  ȫ�ֱ���  : 
**  ���ú���  : 
**  �ж���Դ  : 
**  ��    ע  :
*******************************************************************************/
void CarLoad_ClearSensorAbnormalAlarm(void)
{
    s_stCarLoadAttrib.sensorAbnormal = 0;
    Io_WriteSelfDefine2Bit(DEFINE_BIT_25,RESET);
}
/*******************************************************************************
**  ��������  : CarLoad_CheckOverLoad
**  ��������  : ����Ƿ���
**  ��    ��  : ��				
**  ��    ��  : 
**  ȫ�ֱ���  : 
**  ���ú���  : 
**  �ж���Դ  : 
**  ��    ע  :
*******************************************************************************/
void CarLoad_CheckOverLoad(void)
{
    if(!PUBLIC_CHECKBIT(s_stCarLoadAttrib.totalWeight,31))
    {
        if(s_stCarLoadAttrib.totalWeight>=s_stCarLoadAttrib.maxweight) 
        {
            CarLoad_SetOverLoadAlarm();
            return;
        }
    }
    /////////////////////
    CarLoad_ClearOverLoadAlarm();   
    //////////////////////
}
/*******************************************************************************
**  ��������  : CarLoad_DebugTest
**  ��������  : 
**  ��    ��  : ��				
**  ��    ��  : 
**  ȫ�ֱ���  : 
**  ���ú���  : 
**  �ж���Դ  : 
**  ��    ע  :
*******************************************************************************/
void CarLoad_DebugTest(void)
{
    static unsigned long sulTmpCnt=0;
    sulTmpCnt++;
    if(!s_stCarLoadAttrib.totalWeight)
    s_stCarLoadAttrib.totalWeight =sulTmpCnt;
}
/*******************************************************************************
**  ��������  : CarLoad_CheckIsUpdataSystem
**  ��������  : 
**  ��    ��  : ��				
**  ��    ��  : 
**  ȫ�ֱ���  : 
**  ���ú���  : 
**  �ж���Դ  : 
**  ��    ע  :
*******************************************************************************/
void CarLoad_CheckIsUpdataSystem(void)
{
    if(LoadControllerUpdata_GetUpdataFlag())
    {
        LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_TASK],PUBLICSECS(10),CarLoad_CheckIsUpdataSystem);
    }
    else
    {
        LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_TASK],PUBLICSECS(3),Carload_SendCmdQueryTotalWeight);
    }
}
/*******************************************************************************
**  ��������  : CarLoad_DisposeRecvDataResponse
**  ��������  : 
**  ��    ��  : ��				
**  ��    ��  : 
**  ȫ�ֱ���  : 
**  ���ú���  : 
**  �ж���Դ  : 
**  ��    ע  :
*******************************************************************************/
void CarLoad_DisposeRecvDataResponse(void)
{
    unsigned char *pBuffer;
    unsigned char index;
    if(1==s_stCarLoadAttrib.lcdTransFlag)
    {
        CarLoad_DisposeLoadSensorTransToLcd();
        return;
    }
    ///////////////////////////
    pBuffer = &s_stCarLoadAttrib.parseData[CARLOAD_USER_DATA_START];
    switch(s_stCarLoadAttrib.recvCmd)
    {
        case CARLOAD_COMM_CMD_QUERY_COM_WEIGHT: 
            LZM_PublicKillTimer(&s_stCLTimer[CL_TIMER_CAR_STATUS]);
            //////////////////////////////////////////////
            s_stCarLoadAttrib.carStatusCnt = 0;
            index =0;
            s_stCarLoadAttrib.status = pBuffer[index++];
            if(PUBLIC_CHECKBIT(s_stCarLoadAttrib.status,2))//�������쳣
            {
                LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_TASK],PUBLICSECS(0.2),Carload_SendCmdQuerySensorAbnormal);
            }
            else///�������ȶ�
            if(PUBLIC_CHECKBIT(s_stCarLoadAttrib.status,0))////�������ȶ�
            {
                LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_TASK],PUBLICSECS(3),Carload_SendCmdQueryTotalWeight);
            }
            else///�����ȶ�
            {
                CarLoad_ClearSensorAbnormalAlarm();
                s_stCarLoadAttrib.totalWeight = Public_ConvertBufferToLong(&pBuffer[index]);
                CarLoad_CheckOverLoad();               
                LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_CAR_STATUS],PUBLICSECS(0.5),Carload_SendCmdCurCarStatus);
                LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_TASK],PUBLICSECS(6),Carload_SendCmdQueryTotalWeight);
            }
            break;
        case CARLOAD_COMM_CMD_QUERY_SENSOR_ABNORMAL:
            ////////////////////
            LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_CAR_STATUS],PUBLICSECS(0.5),Carload_SendCmdCurCarStatus);
            s_stCarLoadAttrib.sensorAbnormal = Public_ConvertBufferToLong(pBuffer);
            ///////////////////
            if(0 == s_stCarLoadAttrib.sensorAbnormal)//
            {
                CarLoad_ClearSensorAbnormalAlarm();
            }
            else
            {
                CarLoad_SetSensorAbnormalAlarm();
                CarLoad_ClearOverLoadAlarm();
            }
            ////////////////////////
            if(0==LZM_PublicGetTimerEnable(&s_stCLTimer[CL_TIMER_TASK]))
            {
                LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_TASK],PUBLICSECS(4),Carload_SendCmdQueryTotalWeight);
            }
            break;
        case CARLOAD_COMM_CMD_SERVER_SENSOR_CLEAR:
            LZM_PublicKillTimer(&s_stCLTimer[CL_TIMER_CAR_STATUS]);
            s_stCarLoadAttrib.clearresult = Public_ConvertBufferToLong(pBuffer);
            /////////////////////////////////////
            if(CARLOAD_RADIO_CLEAR_SENSOR_ABNORMAL == s_stCarLoadAttrib.platformCmd)///
            {
                s_stCarLoadAttrib.platformCmd = CARLOAD_RADIO_CMD_MAX;
                LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_SEND_DATA],PUBLICSECS(1.5),CarLoad_SendClearSensorAbnormalResult);
            }
            ////////////////////////////////////            
            s_stCarLoadAttrib.clearCnt = 0;
            LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_TASK],PUBLICSECS(1),Carload_SendCmdQueryTotalWeight);
            break;
        case CARLOAD_COMM_CMD_SENSOR_CLEAR:
            LZM_PublicKillTimer(&s_stCLTimer[CL_TIMER_CAR_STATUS]);
            s_stCarLoadAttrib.clearresult = Public_ConvertBufferToLong(pBuffer);
            /////////////////////////////////////
            LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_SEND_DATA],LZM_TIME_BASE,NaviLcd_SendCarLoadClearSensorResult);
            ////////////////////////////////////            
            s_stCarLoadAttrib.clearCnt = 0;
            LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_TASK],PUBLICSECS(1),Carload_SendCmdQueryTotalWeight);
            break;
        case CARLOAD_COMM_CMD_CAR_STATUS:
            if(0xff == s_stCarLoadAttrib.carStatusFlag)
            {
                s_stCarLoadAttrib.carStatusFlag = 0;
            }
            else
            if(0x55== s_stCarLoadAttrib.carStatusFlag)
            {
                s_stCarLoadAttrib.carStatusFlag = 1;
            }
            s_stCarLoadAttrib.carStatusCnt++;
            if(s_stCarLoadAttrib.carStatusCnt>4)
            {
                LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_TASK],PUBLICSECS(1),Carload_SendCmdQueryTotalWeight);
            }
            else
            LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_CAR_STATUS],PUBLICSECS(2.5),Carload_SendCmdCurCarStatus);
            break;
        case CARLOAD_COMM_CMD_UPDATA:
            LZM_PublicKillTimer(&s_stCLTimer[CL_TIMER_SEND_DATA]);
            LZM_PublicKillTimer(&s_stCLTimer[CL_TIMER_CAR_STATUS]);
            LoadControllerUpdata_DisposeUpdataResponse(pBuffer,s_stCarLoadAttrib.recvDataLen-CARLOAD_USER_DATA_MIN_LEN);
            LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_TASK],PUBLICSECS(10),CarLoad_CheckIsUpdataSystem);
            break;
        default:
            if(CARLOAD_RADIO_DEVELOP_PORT == s_stCarLoadAttrib.platformCmd)///
            {
                LZM_PublicKillTimer(&s_stCLTimer[CL_TIMER_CAR_STATUS]);
                LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_TASK],PUBLICSECS(3),Carload_SendCmdQueryTotalWeight);
                s_stCarLoadAttrib.platformCmd = CARLOAD_RADIO_CMD_MAX;
                LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_SEND_DATA],PUBLICSECS(1.5),CarLoad_SendDevelopPortResponse);
            }
            else
            {
                CarLoad_DisposeLoadSensorTransToLcd();
            }
            break;
    }
}
/*************************************************************
** ��������: CarLoad_CommProtocolParse
** ��������: ���ش�����Э�����
** ��ڲ���: ParseBuffer�����׵�ַ,usDataLenght���ݳ���
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
unsigned char CarLoad_CommProtocolParse(unsigned char *ParseBuffer,unsigned short usDataLenght)
{
    unsigned short datalen,i,crc,crc1;
    unsigned char startflag;
    unsigned char *pBuf;
    //////////////
    if(!CarLoad_Get32PINLedPortInsertLoadSensorFlag())return COMM_RECV_ERR_START;
    ///////////////////////////////////////
    if(usDataLenght<CARLOAD_USER_DATA_MIN_LEN)return COMM_RECV_ERR_START;
    datalen=usDataLenght;
    pBuf=ParseBuffer;
    ///////////////////////////////////////
    startflag=0;
    for(i=0; i<datalen; i++)
    {
        if(0x51== pBuf[i]&&0x01== pBuf[i+1])
        {
            pBuf=ParseBuffer+i;
            startflag =1;
            datalen -=i;
            break;
        }
    }    
    if(0 == startflag)return COMM_RECV_ERR_START;//��ͷ
    if(0xC2 != pBuf[2])return COMM_RECV_ERR_START;//��ͷ
    if(datalen < CARLOAD_USER_DATA_MIN_LEN)return COMM_RECV_ERR_START;
    crc=CRC16(pBuf,datalen-2);
    crc1=(pBuf[datalen-2]<<8)+pBuf[datalen-1];
    if(crc!=crc1)return COMM_RECV_ERR_VERIFY;
    s_stCarLoadAttrib.recvCmd = pBuf[3]; 
    s_stCarLoadAttrib.recvCmdEx =(pBuf[3]& 0x7f);
    s_stCarLoadAttrib.recvDataLen = datalen;
    s_stCarLoadAttrib.onlinestatus = 1;
    s_stCarLoadAttrib.contrecvtimeoutCnt = 0;
    memcpy(s_stCarLoadAttrib.parseData,pBuf,s_stCarLoadAttrib.recvDataLen);
    LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_TIME_OUT],LZM_AT_ONCE,CarLoad_DisposeRecvDataResponse);
    ///////////////////////////
    return COMM_RECV_OK;       
}
/*************************************************************
** ��������: Carload_GetCurSensorAbnormalValue
** ��������: ȡ�ô������쳣ֵ
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ÿһλ��Ӧһ·������������λΪ1��ʾ��·����������
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
unsigned long Carload_GetCurSensorAbnormalValue(void)
{
    return s_stCarLoadAttrib.sensorAbnormal;
}
/*************************************************************
** ��������: Carload_GetClearSensorResult
** ��������: ȡ�ô�����������
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ÿһλ��Ӧһ·������������λΪ1��ʾ��·����������
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
unsigned long Carload_GetClearSensorResult(void)
{
    return s_stCarLoadAttrib.clearresult;
}
/*************************************************************
** ��������: Carload_GetCurTotalWeight
** ��������: ȡ���������
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
unsigned long Carload_GetCurTotalWeight(void)
{
    return s_stCarLoadAttrib.totalWeight;
}
/*************************************************************
** ��������: Carload_GetSensorAbnormalFlag
** ��������: ȡ�ô������Ƿ��쳣��־
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: 0:����; 1:�쳣
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
unsigned char Carload_GetSensorAbnormalFlag(void)
{
    return PUBLIC_CHECKBIT(s_stCarLoadAttrib.status,2);
}
/*************************************************************
** ��������: CarLoad_GetCurOverLoadFlag
** ��������: ȡ���Ƿ��ر�־
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: 0:����; 1:����
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
unsigned char CarLoad_GetCurOverLoadFlag(void)
{
    return Io_ReadSelfDefine2Bit(DEFINE_BIT_19);
}
/*************************************************************OK
** ��������: CarLoad_UpdatePram
** ��������: ���²���
** ��ڲ���:  
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
void CarLoad_UpdatePram(void)
{
    unsigned short ledPort=0;
    unsigned char buffer[10];
    if(EepromPram_ReadPram(E2_LOAD_MAX_ID,buffer))
    {
        s_stCarLoadAttrib.maxweight = Public_ConvertBufferToLong(buffer);
    }
    else
    {
        s_stCarLoadAttrib.maxweight = 0xffffffff;
    }
    ////////////////////
    if(EepromPram_ReadPram(E2_LED_PORT_ID, buffer))
    {
        ledPort = Public_ConvertBufferToShort(buffer);
    }
    ////////////////////////
    s_stCarLoadAttrib.loadSensorFlag = PUBLIC_CHECKBIT_EX(ledPort,LEDPORT_BIT_LOAD_SENSOR);
}

/*************************************************************OK
** ��������: CarLoad_ParamInitialize
** ��������: ��ʼ��
** ��ڲ���:  
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
void CarLoad_ParamInitialize(void)
{ 
    LZM_PublicKillTimerAll(s_stCLTimer,CL_TIMERS_MAX);
    s_stCarLoadAttrib.userAlarm = 0;
    s_stCarLoadAttrib.onlinestatus =0;
    s_stCarLoadAttrib.lcdTransFlag=0;
    s_stCarLoadAttrib.contrecvtimeoutCnt = 0;
    s_stCarLoadAttrib.carStatusFlag=0xff;
    s_stCarLoadAttrib.platformCmd = CARLOAD_RADIO_CMD_MAX;
    s_stCarLoadAttrib.loadSensorFlag = 0;
    CarLoad_UpdatePram();    
    SetTimerTask(TIME_CARLOAD, LZM_TIME_BASE);    
    if(!CarLoad_Get32PINLedPortInsertLoadSensorFlag())return;
    LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_TASK],PUBLICSECS(30),Carload_SendCmdQueryTotalWeight);
}
/*******************************************************************************
** ��������: CarLoad_GetCurWeightSubjoinInfo
** ��������: ȡ�õ�ǰ����������Ϣ��(����������ϢID�����ȡ�������Ϣ��)
** ��ڲ���: ��Ҫ��������������Ϣ�����׵�ַ
** ���ڲ���: 
** ���ز���: ����������Ϣ�����ܳ���
*******************************************************************************/
unsigned char CarLoad_GetCurWeightSubjoinInfo(unsigned char *data)
{
    unsigned char buffer[10]={0};
    unsigned char i;
    if(!CarLoad_Get32PINLedPortInsertLoadSensorFlag())return 0;
    i = 0;
    buffer[i++] = WEIGHT_OVERHEAD_INFO_ID;
    buffer[i++] = WEIGHT_OVERHEAD_INFO_LEN;
    Public_ConvertLongToBuffer(s_stCarLoadAttrib.totalWeight,&buffer[i]);
    //////////////////////////
    i += WEIGHT_OVERHEAD_INFO_LEN;
    ///////////////
    memcpy(data,buffer,i);
    /////////////////
    return i;
}
/*************************************************************
** ��������: CarLoad_DisposeRadioProtocol
** ��������: ����͸��Э�����
** ��ڲ���: cmd����,pBuffer͸����Ϣ�׵�ַ��BufferLen͸����Ϣ����
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
void CarLoad_DisposeRadioProtocol(unsigned char cmd,unsigned char *pBuffer,unsigned short BufferLen)
{    
    switch(cmd)
    {
        case CARLOAD_RADIO_QUERY_SENSOR_ABNORMAL:
            LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_SEND_DATA],PUBLICSECS(1.5),CarLoad_SendQuerySensorAbnormalResult);
            break;
        case CARLOAD_RADIO_CLEAR_SENSOR_ABNORMAL:
            LZM_PublicKillTimer(&s_stCLTimer[CL_TIMER_CAR_STATUS]);
            s_stCarLoadAttrib.platformCmd = cmd;            
            Carload_SendCmdClearServerSensorAbnormal();
            LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_TASK],PUBLICSECS(8),Carload_SendCmdQueryTotalWeight);
            break;
        case CARLOAD_RADIO_DEVELOP_PORT:
            LZM_PublicKillTimer(&s_stCLTimer[CL_TIMER_CAR_STATUS]);
            s_stCarLoadAttrib.platformCmd = cmd;
            CarLoad_SendData(pBuffer,BufferLen);
            LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_TASK],PUBLICSECS(8),Carload_SendCmdQueryTotalWeight);
            break;
        default:
            break;
    }    
}
/*************************************************************
** ��������: CarLoad_OriginalDataUpTrans
** ��������: ���������ϴ�
** ��ڲ���: msgID��ϢID,pBuffer��Ϣ�����ݣ�BufferLen��Ϣ�峤��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/	
ProtocolACK CarLoad_OriginalDataUpTrans(unsigned char msgID,unsigned char *pBuffer,unsigned short BufferLen)
{
    unsigned short EIcmd;
    EIcmd = (EIEXPAND_PROTOCOL_FUN_CARLOAD<<8);
    EIcmd |= msgID;
    //////////////////////////
    return EIExpand_PotocolSendData(EIcmd,pBuffer,BufferLen);
}
/*************************************************************
** ��������: CarLoad_SendQuerySensorAbnormalResult
** ��������: ƽ̨��ѯ���������Ͻ��ϴ�
** ��ڲ���: 
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/	
void CarLoad_SendQuerySensorAbnormalResult(void)
{
    unsigned char data[10];
    Public_ConvertLongToBuffer(s_stCarLoadAttrib.sensorAbnormal, data);
    //////////////////////////
    if(ACK_OK != CarLoad_OriginalDataUpTrans(CARLOAD_RADIO_QUERY_SENSOR_ABNORMAL,data,4))
    {
        if(s_stCarLoadAttrib.sendcnt++<4)
        {
            LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_SEND_DATA],PUBLICSECS(0.2),CarLoad_SendQuerySensorAbnormalResult);
            return;
        }
    }
    s_stCarLoadAttrib.sendcnt = 0;
}
/*************************************************************
** ��������: CarLoad_SendClearSensorAbnormalResult
** ��������: ƽ̨������������Ͻ���ϴ�
** ��ڲ���: 
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/	
void CarLoad_SendClearSensorAbnormalResult(void)
{
    unsigned char data[10];
    Public_ConvertLongToBuffer(s_stCarLoadAttrib.clearresult, data);
    //////////////////////////
    if(ACK_OK != CarLoad_OriginalDataUpTrans(CARLOAD_RADIO_CLEAR_SENSOR_ABNORMAL,data,4))
    {
        if(s_stCarLoadAttrib.sendcnt++<4)
        {
            LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_SEND_DATA],PUBLICSECS(0.2),CarLoad_SendClearSensorAbnormalResult);
            return;
        }
    }
    s_stCarLoadAttrib.sendcnt = 0;
}
/*************************************************************
** ��������: CarLoad_SendFreightInformation
** ��������: �����ϴ��˻���Ϣ
** ��ڲ���: 
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/	
void CarLoad_SendFreightInformationToServiceEx(void)
{
    unsigned char data[2];
    data[0]=s_stCarLoadAttrib.freightInfo;
    if(ACK_OK !=CarLoad_OriginalDataUpTrans(CARLOAD_RADIO_FREIGHT_INFORMATION,data,1))
    {
        if(s_stCarLoadAttrib.sendcnt++<4)
        {
            LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_SEND_DATA],PUBLICSECS(0.2),CarLoad_SendFreightInformationToServiceEx);
            return;
        }
    }
    s_stCarLoadAttrib.sendcnt = 0;
}
/*************************************************************
** ��������: CarLoad_SendFreightInformation
** ��������: �����ϴ��˻���Ϣ
** ��ڲ���: 
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/	
void CarLoad_SendFreightInformationToService(unsigned char flag)
{
    s_stCarLoadAttrib.freightInfo=flag;
    CarLoad_SendFreightInformationToServiceEx();
}
/*************************************************************
** ��������: CarLoad_SendDevelopPortResponse
** ��������: ���ؿ�������Ӧ���ϴ�
** ��ڲ���: 
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/	
void CarLoad_SendDevelopPortResponse(void)
{
    if(ACK_OK !=CarLoad_OriginalDataUpTrans(CARLOAD_RADIO_DEVELOP_PORT,s_stCarLoadAttrib.parseData,s_stCarLoadAttrib.recvDataLen))
    {
        if(s_stCarLoadAttrib.sendcnt++<4)
        {
            LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_SEND_DATA],PUBLICSECS(0.2),CarLoad_SendDevelopPortResponse);
            return;
        }
    }
    s_stCarLoadAttrib.sendcnt = 0;
}

/*******************************************************************************
**  ��������  : CarLoad_TimeTask
**  ��������  : ʱ��������Ƚӿں���
**  ��    ��  : ��				
**  ��    ��  : �������״̬:  ʹ�ܻ��ֹ
**  ȫ�ֱ���  : 
**  ���ú���  : 
**  �ж���Դ  : 
**  ��    ע  :
*******************************************************************************/
FunctionalState CarLoad_TimeTask(void) 
{
    LZM_PublicTimerHandler(s_stCLTimer,CL_TIMERS_MAX);
    return ENABLE;
}
/*************************************************************OK
** ��������: CarLoad_GetCurOnlineStatus
** ��������: ���߱�־
** ��ڲ���:  
** ���ڲ���: 
** ���ز���: 1:����;0������
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
unsigned char CarLoad_GetCurOnlineStatus(void)
{ 
    return (1==s_stCarLoadAttrib.onlinestatus)?1:0;
}
/*************************************************************
** ��������: CarLoad_DisposeLcdTransToLoadSensor
** ��������: ����Lcd͸�������ش���������
** ��ڲ���: 
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/	
void CarLoad_DisposeLcdTransToLoadSensor(unsigned char *data,unsigned short len)
{
    if(!CarLoad_Get32PINLedPortInsertLoadSensorFlag())return;
    s_stCarLoadAttrib.lcdTransFlag = 1;
    CarLoad_SendData(data,len);
    LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_TASK],PUBLICSECS(8),Carload_SendCmdQueryTotalWeight);
}
/*************************************************************
** ��������: CarLoad_DisposeLoadSensorTransToLcd
** ��������: �������ش�����͸����Lcd����
** ��ڲ���: 
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/	
void CarLoad_DisposeLoadSensorTransToLcd(void)
{
    s_stCarLoadAttrib.lcdTransFlag = 0;
    NaviLcd_DisponseLoadSensorTransToLcd(s_stCarLoadAttrib.parseData,s_stCarLoadAttrib.recvDataLen);
    LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_TASK],PUBLICSECS(0.5),Carload_SendCmdQueryTotalWeight);
}

/*******************************************************************************
 *                             end of module
 *******************************************************************************/


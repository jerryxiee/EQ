/*******************************************************************************
 * File Name:			ICCardBase.h 
 * Function Describe:	
 * Relate Module:		IC��ģ��
 * Writer:				Joneming
 * Date:				2014-07-09
 * ReWriter:			
 * Date:				
 *******************************************************************************/

//***************�����ļ�*****************
#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "include.h"
/////////////////////////////////////
#define	RESULT_NULL             0xFF //0xFF IC����Ч���
#define	RESULT_SUCCESS          0x00 //0x00	IC�������ɹ�	������128�ֽڵĿ�������
#define	RESULT_ERROR_NOINSERT   0x01 //0x01	IC��δ����
#define	RESULT_ERROR_NOSUPPORT  0x02 //0x02 IC����Ч����֧�ֵĿ�Ƭ
#define	RESULT_ERROR_FOMAT      0x03 //0x03	IC�������ݸ�ʽ����
#define	RESULT_ERROR_VERIFY     0x04 //0x04	IC��������У�����
#define	RESULT_ERROR_PASSWORD   0x05 //0x05	IC ���������	Ԥ��, SLE4442����ʣ�����
////////////////////////////
enum
{
    ICCARD_STASUS_EMPTY,
    ICCARD_STASUS_IN,
    ICCARD_STASUS_OUT,
    ICCARD_STASUS_MAX,
};
//////////
enum
{
    ICCARD_VER_EMPTY,
    ICCARD_VER_USE_EXT,//ʹ���ⲿ������IC��ģ��
    ICCARD_VER_USE_INT,//ʹ������CPUͨ��I2C��IC��
    ICCARD_VER_MAX,
};
enum
{
    ICCARD_READ_EMPTY,
    ICCARD_READ_PWR_ON,
    ICCARD_READ_24XX,//
    ICCARD_READ_SL4442,//
    ICCARD_READ_MAX,
};
////////////////

typedef struct
{
    unsigned char PID[2];//�汾��
    unsigned char VID[2];//���̱��
    unsigned char prestatus;//
    unsigned char count;//
    unsigned char result;//
    unsigned char readstep;//
    unsigned char useIntFlag;//
}ST_ICCARD_BASE;
///////////////
static ST_ICCARD_BASE s_stICCardBase;
                                  //�汾��//���̴���
const unsigned char c_ucPID_VID[]={0xA0,0xA1,0xB0,0xB1};
//////////////////////
enum 
{
    ICB_TIMER_TASK,                  //0 timer
    ICB_TIMER_TEST,                  //1 timer
    ICB_TIMERS_MAX
}E_ICCARDBASETIME;

static LZM_TIMER s_stICBTimer[ICB_TIMERS_MAX];
/********************************************************************
* ���� : ICCardBase_VerifyCalcSum
* ���� : ���� ����� У��(�ۼӺ�)
********************************************************************/
u8 ICCardBase_VerifyCalcSum( u8 * buff, u16 length)
{     
    if((length == 0)||(buff == NULL))return 0;
    return Public_GetSumVerify(buff,length);
}

/********************************************************************
* ���� : ICCardBase_VerifyCalcSum
* ���� : ���� ����� У��(���)
********************************************************************/
u8 ICCardBase_VerifyCalcXor( const u8 * InBuf , u16 InLength)
{       
    if((InLength == 0)||(InBuf == NULL))return 0;
    return Public_GetXorVerify(InBuf,InLength);;
}

/********************************************************************
* ���� : ICCardBase_ExtractExdevice
* ���� : ���� ����� У���
********************************************************************/
u16 ICCardBase_ExtractExdevice( u8 *pInBuf, u16 InLength, ST_PROTOCOL_EXDEVICE *pPacket)
{
    u8 VerifyCode;
    u16 datalen;
    u8  *buffer;
    u16 Result = 0;    
    buffer = pInBuf;
    datalen = unTransMean(buffer, InLength);
    buffer = pInBuf+3;
    /****** ����Э��У����  *****/
    VerifyCode = ICCardBase_VerifyCalcSum( buffer, datalen-3);
    if( pInBuf[0]== VerifyCode )
    {
        if(datalen>sizeof(ST_PROTOCOL_EXDEVICE))return 0;
        memcpy((u8 *)pPacket,pInBuf,datalen);
        if(EXDEVICE_TYPE_ICCARD != pPacket->Type)return 0;
        memcpy(s_stICCardBase.PID,pPacket->PID,4);
        Result = datalen;
    }    
    return (Result);    
}
/*************************************************************
** ��������: ICCardBase_CreatPackData
** ��������: ���ݴ��
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
u16 ICCardBase_CreatPackData( u8 cmd,  u8 * pInBuf , u16 InLength , u8 * pOutBuf ,unsigned char response)
{
    ST_PROTOCOL_EXDEVICE * pPacket;
    u16 datalen;
    u8 * pTemp;    
    pTemp = pOutBuf+1;
    pPacket = (ST_PROTOCOL_EXDEVICE *)pTemp;
    //////�汾�š����̱��/////////////////////////
    if(response)//Ӧ��
    {
        memcpy(pPacket->PID,s_stICCardBase.PID,4);
    }
    else
    { 
        memcpy(pPacket->PID,c_ucPID_VID,4);
    }
    //////////////////
    pPacket->Type = EXDEVICE_TYPE_ICCARD;
    /****  ��������***/
    pPacket->Cmd = cmd;
    /****  �û����� ***/
    if(InLength)
    {      
        memcpy(pPacket->Data, pInBuf,  InLength ) ;    
    }    
    /****  У���� �ӳ��̱�ŵ��û����������ۼӵ��ۼӺͣ�ȡ�ͣ�λ ***/    
    pPacket->Verity = ICCardBase_VerifyCalcSum( (u8 *)&pPacket->VID[0], InLength+4 );//У���� ����+����+����)
    //////////////////
    datalen=InLength+7;
    ///////////////////////
    datalen = TransMean( pTemp, datalen );
    pOutBuf[0]= 0x7E;
    pOutBuf[datalen+1]= 0x7E;
    return (datalen+2);
}  
extern u8	SwipCardCh;//ˢ��ͨ��  0 ����������  1  ����IC��
/*************************************************************
** ��������: ICCardBase_PackDataToParse
** ��������: ���IC�����ݲ��������ݽ���
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
void ICCardBase_PackDataToParse( u8 cmd,  u8 * pIn , u16 InLength)
{
    unsigned short len;
    unsigned char buffer[160];
    len=ICCardBase_CreatPackData(cmd,pIn,InLength,buffer,0);
	SwipCardCh = 1;
    ICCard_ProtocolParse(buffer,len);
	SwipCardCh = 0;
}
/*************************************************************
** ��������: ICCard_PowerInit
** ��������: 
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
void ICCardBase_PowerInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    //================================================   
    /* �򿪹������� ,�øߵ�ƽ IC_P clock */
    RCC_AHB1PeriphClockCmd(RCC_IC_CARD_POWER, ENABLE); 
    GPIO_InitStructure.GPIO_Pin = PIN_IC_CARD_POWER;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init( GPIO_IC_CARD_POWER, &GPIO_InitStructure);
}
/*************************************************************
** ��������: ICCardBase_CSInit
** ��������: 
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
void ICCardBase_CSInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;  
    /******************************ICCardBase_CS--->PD5***********************************/    
    RCC_AHB1PeriphClockCmd(RCC_ICCARD_CS , ENABLE);	       /* ��GPIOʱ�� */
    GPIO_InitStructure.GPIO_Pin = PIN_ICCARD_CS  ;    
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;        /* ����ģʽ */     
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;  	         /* ���/���� */    
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;           /* ��© */    
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;             /* ���� */    
    GPIO_Init( GPIO_ICCARD_CS , &GPIO_InitStructure);
}
/*************************************************************
** ��������: ICCardBase_GetUseIntFlag
** ��������: �Ƿ�ͨ������CPUͨ��I2C��IC��
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 1:ͨ������CPUͨ��I2C��IC��,0:ʹ���ⲿ������IC��ģ��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
unsigned char ICCardBase_GetUseIntFlag(void)
{
    return (s_stICCardBase.useIntFlag == ICCARD_VER_USE_INT)?1:0;
}
/*************************************************************
** ��������: ICCardBase_GetReadFlag
** ��������: �ն��Ƿ����ڶ�IC��
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 1:���ڶ�,0����
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
unsigned char ICCardBase_GetReadFlag(void)
{
    return (s_stICCardBase.readstep)?1:0;
}
/*************************************************************
** ��������: ICCardBase_ReadICCardFinish
** ��������: 
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
void ICCardBase_ReadICCardFinish(void)
{
    s_stICCardBase.readstep = ICCARD_READ_EMPTY;
    ICCARD_POWER_OFF();
    LZM_PublicKillTimer(&s_stICBTimer[ICB_TIMER_TEST]);
}
/*************************************************************
** ��������: ICCardBase_ReadICCardSL4442
** ��������: 
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
void ICCardBase_ReadICCardSL4442(void)
{
    unsigned char buffer[140];
    unsigned short datalen;    
    if(0==ICCardBase_GetReadFlag())return;
    ////////////////////////
    datalen = 128;
    if(SL4442_Buffer_Read(&buffer[1],datalen))
    {
        datalen++;
        buffer[0] = RESULT_SUCCESS;        
        s_stICCardBase.result = RESULT_SUCCESS;
        if(buffer[1]==0xff&&buffer[2]==0xff&&buffer[3]==0xff)
        {
            s_stICCardBase.result = RESULT_NULL;
        }
    }
    else
    {
        datalen = 1;
        buffer[0] = RESULT_ERROR_NOSUPPORT;
        s_stICCardBase.result = RESULT_ERROR_NOSUPPORT;        
    }
    ///////////////////
    ICCardBase_ReadICCardFinish();
    ///////////////////
    ICCardBase_PackDataToParse(0x41,buffer,datalen);    
}
/*************************************************************
** ��������: ICCardBase_ReadICCard24XX
** ��������: 
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
void ICCardBase_ReadICCard24XX(void)
{
    unsigned char buffer[140];
    unsigned short datalen;
    if(0==ICCardBase_GetReadFlag())return;
    /////////////////
    datalen = 128;
    if(EEPROM_24XX_HL_Buf_Read(0, &buffer[1],datalen))
    {
        ICCardBase_ReadICCardFinish();
        s_stICCardBase.result = RESULT_SUCCESS;
        buffer[0] = s_stICCardBase.result;
        datalen++;
        ICCardBase_PackDataToParse(0x41,buffer,datalen);
    }
    else
    {
        s_stICCardBase.readstep = ICCARD_READ_SL4442;
    }
}
/*************************************************************
** ��������: ICCardBase_DisposeICCardStatusIn
** ��������: 
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
void ICCardBase_DisposeICCardStatusIn(void)
{
    LZM_PublicSetOnceTimer(&s_stICBTimer[ICB_TIMER_TEST],PUBLICSECS(0.1),ICCardBase_DisposeICCardStatusIn);
    switch(s_stICCardBase.readstep)
    {
        case ICCARD_READ_PWR_ON:
            ICCARD_POWER_ON();
            s_stICCardBase.result = RESULT_NULL;
            s_stICCardBase.readstep = ICCARD_READ_24XX;
            break;
        case ICCARD_READ_24XX:
            ICCardBase_ReadICCard24XX();
            break;
        case ICCARD_READ_SL4442:
            ICCardBase_ReadICCardSL4442();
            break;
        default:
            ICCardBase_ReadICCardFinish();
            break;
    }    
}
/*************************************************************
** ��������: Card_DisposeICCardStatusInsert
** ��������: 
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
void ICCardBase_DisposeICCardStatusOut(void)
{
    unsigned char data[2];
    ///////////////
    ICCardBase_ReadICCardFinish();
    ////////////////
    if(RESULT_SUCCESS!=s_stICCardBase.result)return;
    s_stICCardBase.result = RESULT_NULL;
    data[0] = 0;
    ICCardBase_PackDataToParse(0x42,data,1);
}
/*************************************************************
** ��������: ICCardBase_CheckStatusChange
** ��������: 
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
void ICCardBase_CheckStatusChange(void) 
{
    if(ICCARD_DETECT_READ())//�޿�
    {
        if(ICCARD_STASUS_OUT != s_stICCardBase.prestatus)
        {
            if(s_stICCardBase.count++ >5)
            {
                s_stICCardBase.count = 0;
                s_stICCardBase.prestatus = ICCARD_STASUS_OUT;                
                /////�ο�����///////////
                ICCardBase_DisposeICCardStatusOut();               
            }
        }
        else
        {
            s_stICCardBase.count = 0;
        }
    }
    else//�п�
    {
        if(ICCARD_STASUS_IN != s_stICCardBase.prestatus)//
        {
            if(s_stICCardBase.count++ >5)
            {
                s_stICCardBase.count = 0;
                s_stICCardBase.prestatus = ICCARD_STASUS_IN;                 
                /////�忨����///////////
                s_stICCardBase.readstep = ICCARD_READ_PWR_ON;
                ICCardBase_DisposeICCardStatusIn();
            }
        }
        else
        {
            s_stICCardBase.count = 0;
        }
    }
}

/*************************************************************
** ��������: ICCardBase_StartCheckStatus
** ��������: 
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
void ICCardBase_StartCheckStatus(void) 
{
    LZM_PublicSetCycTimer(&s_stICBTimer[ICB_TIMER_TASK],LZM_AT_ONCE,ICCardBase_CheckStatusChange);
}
/*************************************************************
** ��������: ICCardBase_ParameterInitialize
** ��������: IC��������ʼ��
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void ICCardBase_ParameterInitialize(void)
{
    ICCardBase_CSInit();   
    ////////////////
    LZM_PublicKillTimerAll(s_stICBTimer,ICB_TIMERS_MAX); 
    ////////////////
    memset(&s_stICCardBase,0,sizeof(s_stICCardBase));
    /////////////////////////
    memcpy(s_stICCardBase.PID,c_ucPID_VID,4);
    ////////////////////////
    ICCardBase_PowerInit(); 
    ////////////////////
    ICCard_ParamInit();
    /////////////////////
    s_stICCardBase.result = RESULT_NULL;
    s_stICCardBase.readstep = ICCARD_READ_EMPTY;
    s_stICCardBase.prestatus = ICCARD_STASUS_EMPTY;
    /////////////////////
    if(ICCARD_CS_READ())
    {
        ICCARD_M3_Init();
        SetCostDownVersionFlag();
        s_stICCardBase.useIntFlag = ICCARD_VER_USE_INT; 
        LZM_PublicSetOnceTimer(&s_stICBTimer[ICB_TIMER_TASK],PUBLICSECS(2),ICCardBase_StartCheckStatus);
    }
    else
    {
        CommICCardInit();
        ClearCostDownVersionFlag();
        s_stICCardBase.useIntFlag = ICCARD_VER_USE_EXT;
        /////////////////////
        ICCARD_POWER_ON();
    }
    /////////////////
    BMA250_ParamInitialize();
    ////////////////
    GpioOutInit(MIC_EN);//dxl,2014.5.27,�ָ�FD5ԭ���ĳ�ʼ����;����˷���ƣ�
    //////////////////////
    GpioOutOn(MIC_EN);//����˷�
}
/*************************************************************
** ��������: ICCardBase_TimerTask
** ��������: ��ʱ��������
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void ICCardBase_TimerTask(void)
{
    //#if (ICCARD_JTD == ICCARD_SEL) 
    Task_IC_Card_JTB();
    //#else 
    LZM_PublicTimerHandler(s_stICBTimer,ICB_TIMERS_MAX);
	//#endif
}
/******************************************************************************
**                            End Of File
******************************************************************************/


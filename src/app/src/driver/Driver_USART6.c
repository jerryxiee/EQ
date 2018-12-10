/*******************************************************************************
 * File Name:			CommICCard.c 
 * Function Describe:	
 * Relate Module:		�������Э�顣
 * Writer:				Joneming
 * Date:				2011-09-08
 * ReWriter:			
 * Date:				
 *******************************************************************************/
#include "include.h"

u8	SwipCardCh = 0;//ˢ��ͨ��  0 ����������  1  ����IC��
#if 0
#define COMICCARD_DEFAULT_BAUD  4800 //IC��������ģ�龧��Ϊ12Mʱ����,�����ͼ�����ʹ��
#else
#define COMICCARD_DEFAULT_BAUD  9600  //IC��������ģ�龧��Ϊ11.0596ʱ����
#endif
///////////////////////
#define PARAM_COM               COM6//����ʹ�õĴ���
//////////////////////////
#define COMICCARD_MAX_REC_LEN    256 //�������ݵĳ���
#define COMICCARD_MAX_SEND_LEN   COM6_MAX_LEN_TX //�������ݵĳ���
///////////////////////
static unsigned char s_ucCommrecvBuff[COMICCARD_MAX_REC_LEN+1];     //��������
static unsigned char s_ucCommresendBuff[COMICCARD_MAX_SEND_LEN+1];  //���ͻ���
/////////////////////////
static ST_COMM s_stCommICCard;
//////////////////////////
enum 
{
    CPARAM_TIMER_RESEND, //    
    CPARAM_TIMERS_MAX
}E_COM_PARAMTIME;
static LZM_TIMER s_stCParamTimer[CPARAM_TIMERS_MAX];
/****************************************************/
/*                                                  */
/* Local Definitions and macros                     */
/*                                                  */
/****************************************************/
/*************************************************************
** ��������: CommICCard_StartSendData
** ��������: ��������
** ��ڲ���: pBufferԴ����,BufferLen���ݳ���
** ���ڲ���: ��
** ���ز���: 0:�ɹ���1:ʧ��,0xff:�������泤��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
unsigned char CommICCard_StartSendData(unsigned char *pBuffer,unsigned short datalen)
{
#if (ICCARD_JTD == ICCARD_SEL)
	{
		return COMM_SendData(COM2,pBuffer,datalen);
	}
#else
	{
		return COMM_SendData(PARAM_COM,pBuffer,datalen);
	}
#endif
}
/************************************************************
** ��������: CommICCard_ResendData
** ��������: �ط�ʱ�䵽��ִ���ط�����
             
** ��ڲ���:
** ���ڲ���:
************************************************************/ 
void CommICCard_ResendData(void)
{
    if(0 == CommICCard_StartSendData(s_stCommICCard.resendBuff,s_stCommICCard.resendlen))//���ͳɹ�
    {
        s_stCommICCard.resendCnt = 0;
        return;
    }
    /////////////////////////
    if(s_stCommICCard.resendCnt)//�����ط�
    {
        s_stCommICCard.resendCnt--;
        LZM_PublicSetOnceTimer(&s_stCParamTimer[CPARAM_TIMER_RESEND], PUBLICSECS(0.15), CommICCard_ResendData); 
    }
    
}
/*************************************************************
** ��������: CommICCard_SendData
** ��������: ��������
** ��ڲ���: pBufferԴ����,BufferLen���ݳ���
** ���ڲ���: ��
** ���ز���: 0:�ɹ���1:�ȴ��ط�,2:ʧ��,0xff:�������泤��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
unsigned char CommICCard_SendData(unsigned char *pBuffer,unsigned short datalen)
{
    if(datalen>COMICCARD_MAX_SEND_LEN)return 0xff;//��������,ֱ�ӷ���
    //////////////////
    if(0 == CommICCard_StartSendData(pBuffer,datalen))return 0;//���ͳɹ�
    //////ʧ��//////////////////
    if(0==s_stCommICCard.resendCnt)
    {
        s_stCommICCard.resendlen = datalen;
        memcpy(&s_stCommICCard.resendBuff,pBuffer,s_stCommICCard.resendlen);
        s_stCommICCard.resendCnt = 4;//�ط��ʹ���
        LZM_PublicSetOnceTimer(&s_stCParamTimer[CPARAM_TIMER_RESEND], PUBLICSECS(0.15), CommICCard_ResendData); 
        return 1;//�ȴ��ط�
    }
    else
    if((s_stCommICCard.resendlen+datalen)<COMICCARD_MAX_SEND_LEN)
    {
        memcpy(&s_stCommICCard.resendBuff[s_stCommICCard.resendlen],pBuffer,datalen);
        s_stCommICCard.resendlen += datalen;
        return 1;//�ȴ��ط�
    }
    else//
    {
        return 2;//ʧ��
    }
}

/*************************************************************OK
** ��������: CommICCard_ProtocolParse
** ��������: Э�����
** ��ڲ���:  
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
void CommICCard_ProtocolParse(unsigned char *pBuffer,unsigned short datalen)
{
	SwipCardCh = 1;
    ICCard_ProtocolParse(pBuffer,datalen);
	SwipCardCh = 0;
}
/*************************************************************OK
** ��������: CommICCard_TimeTask
** ��������: ���ڽ������ݴ���ʱ������ӿ�
** ��ڲ���:  
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
**  �� ע  :50�������1��
*************************************************************/
void CommICCard_CheckReadData(void)
{
    unsigned short datalen;
    datalen=COMM_ReadData(PARAM_COM,&s_stCommICCard.recvBuff[s_stCommICCard.recvlen],COMICCARD_MAX_REC_LEN-s_stCommICCard.recvlen);
    ///////////////////////////////
    s_stCommICCard.recvlen += datalen;
    ///////////////////////////////////
    if(0 == datalen)////��������ݳ���Ϊ0//////////    
    {
        if(s_stCommICCard.recvlen)
        {
            s_stCommICCard.recvfreeCnt++;
            ////////////////////////////////
            if(s_stCommICCard.recvfreeCnt >1)//�������ε���
            {
                CommICCard_ProtocolParse(s_stCommICCard.recvBuff,s_stCommICCard.recvlen);
                s_stCommICCard.recvlen       = 0;
                s_stCommICCard.recvfreeCnt   = 0;//
            }
        }
    }
    else//////////////////
    if(s_stCommICCard.recvlen+10>COMICCARD_MAX_REC_LEN)//����������ʱ��
    {
        CommICCard_ProtocolParse(s_stCommICCard.recvBuff,s_stCommICCard.recvlen);
        s_stCommICCard.recvlen       = 0;
        s_stCommICCard.recvfreeCnt   = 0;
    }
    else//////////////////
    {
        s_stCommICCard.recvfreeCnt = 0;
    }
}
/*************************************************************OK
** ��������: CommICCard_TimeTask
** ��������: ���ڽ������ݴ���ʱ������ӿ�
** ��ڲ���:  
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
**  �� ע  :50�������1��
*************************************************************/
FunctionalState CommICCard_TimeTask(void)
{
//	#if (ICCARD_JTD == ICCARD_SEL)
//		return DISABLE;
//	#endif
    CommICCard_CheckReadData();
    ////////////////////
    LZM_PublicTimerHandler(s_stCParamTimer,CPARAM_TIMERS_MAX);    
    //////////////////////
    return ENABLE;
}
/*************************************************************OK
** ��������: CommICCardInit
** ��������: //���ڳ�ʼ��
** ��ڲ���:  
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��:
*************************************************************/
void CommICCardInit(void)
{
    LZM_PublicKillTimerAll(s_stCParamTimer,CPARAM_TIMERS_MAX); 
	
    //////////////////
    s_stCommICCard.recvlen       = 0;
    s_stCommICCard.resendlen     = 0;
    s_stCommICCard.resendCnt     = 0;
    s_stCommICCard.recvfreeCnt   = 0;
    s_stCommICCard.recvBuff      = s_ucCommrecvBuff;
    s_stCommICCard.resendBuff    = s_ucCommresendBuff;
    ////////////////////
    COMM_Initialize(PARAM_COM,COMICCARD_DEFAULT_BAUD);
    ////////////////////    
    SetTimerTask(TIME_USART6, LZM_TIME_BASE); 
    ////////////////////////    
}
/*******************************************************************************
 *                             end of module
 *******************************************************************************/


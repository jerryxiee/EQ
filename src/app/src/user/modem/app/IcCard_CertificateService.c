/*
********************************************************************************
*
*                                                              
*
* Filename      : modem_app_second.c
* Version       : V1.00
* Programmer(s) : miaoyahan
* @date         : 2014-10-23  
* 
*
********************************************************************************
*/

/*
********************************************************************************
*                               INCLUDES
********************************************************************************
*/
#ifdef EYE_MODEM
#include "include.h"
#else
#include  <app_cfg.h>
#include  <includes.h>
#endif

/*
********************************************************************************
*                               DEFIEN START
********************************************************************************
*/
#ifdef ICCARD_CERTIFICATE_SERVICE_H
/*
********************************************************************************
*                             LOCAL DEFINES
********************************************************************************
*/

#define  ICCARD_CERTIFICATE_SERVICE_SOC                         (2)//SOC��

extern u8	TerminalAuthorizationFlag;//�ն˼�Ȩ��־,bit0:����1;bit1:����2;bit2~bit7����;

/*
********************************************************************************
*                         LOCAL DATA TYPES
********************************************************************************
*/

/*
********************************************************************************
*                          LOCAL VARIABLES
********************************************************************************
*/
static Modem_APP_STATE CertService_State;    //״̬��
static APP_TYPE_DOWN   CertService_TypeDown; //��������
static SOC_IPVAL       CertService_AppointIp;//ָ��IP
static APP_TYPE_VAL    CertService_IpVal;    //����

static u8  CertService_ConSta;//����״̬ 0Ϊ����  1Ϊ����
static u8  CertService_ConTry;//�������Դ���
static u32 CertService_ConCnt;//��ǰ�������ͳ���������
static u32 CertService_ConVal;//��ǰ�������ͳ���ʱ��

static u32 CertService_HeartVal;//����ֵ
static u32 CertService_HeartCnt;//����������

static u8	CertService_Link_Flg = 0;
/*
********************************************************************************
*                          GLOBAL VARIABLES
********************************************************************************
*/

/*
********************************************************************************
*                          EXTERN VARIABLES
********************************************************************************
*/

/*
********************************************************************************
*                           FUNCTIONS EXTERN
********************************************************************************
*/


/*
********************************************************************************
*                              FUNCTIONS
********************************************************************************
*/
/**
  * @brief  �ӻ�ȡ����
  * @param  type: ��������
  * @retval ���ز����ṹ 1->�ɹ�  0->�Ƿ�����
  */
u8 CertService_ParaConfig(APP_TYPE_VAL type, void *pValue)
{
    u8  tab[50]={0};
    u8  len;
    u8  result;
    u32 id_ip;
    u32 id_port;
    u16 temp;

    SOC_IPVAL *pstip = pValue;
    u32 *pHeart = pValue;

    if(type == VAL_IPMAIN)//��IP
    {
         id_ip   = E2_IC_MAIN_SERVER_IP_ID;
         id_port = E2_IC_MAIN_SERVER_TCP_PORT_ID;
    }
    else if(type == VAL_IPBACK)//����IP
    {
         id_ip   = E2_IC_BACKUP_SERVER_IP_ID;
         id_port = E2_IC_MAIN_SERVER_TCP_PORT_ID;
    }
    else if(type == VAL_IPTO)//ָ��IP
    {
        *pstip = CertService_AppointIp;
        return 1;
    }
    else if(type == VAL_HEART)//����
    {
        if(EepromPram_ReadPram(E2_TERMINAL_HEARTBEAT_ID, tab) == 4)
        {
            *pHeart = (tab[0] << 24) + (tab[1] << 16) + (tab[2] << 8) + tab[3];
            return 1;//��������
        }
    }
		len = EepromPram_ReadPram(id_ip,tab);
    if((len > 0)&&(len < 30))
    {
        result = 0x0a;
        memcpy(pstip->ip,tab,len);
    }

		len = EepromPram_ReadPram(id_port,tab);
    if((len > 0)&&(len < 5))
    {
        result <<= 0x04;
        result |= 0x0a;
        temp = (tab[0] << 24) + (tab[1] << 16) + (tab[2] << 8) + tab[3];
        sprintf((char*)pstip->port,"%d",temp);
        if(temp == 0)
        {
            result = 0;//�ڶ����Ӷ˿�Ϊ0�ǲ���
        }
    }
    if(result == 0xaa)
    {
        strcpy((char*)pstip->mode,"TCP");//ģʽ
        return 1;//��������
    }
    return 0;//�����Ƿ�
}

/**
  * @brief  ���ӵ�¼����Ҫ�Ǵ���ע���Ȩ����
  * @param  type: ���� 
            state: ��һ��ִ�е�״̬
  * @retval None
  */
void CertService_Login(Modem_APP_STATE state)
{
    CertService_ConTry = 0;
    CertService_ConCnt = 0;
    CertService_State  = state;
	CertService_ConSta   = 1;//�����߱�־
	TerminalAuthorizationFlag |= CHANNEL_DATA_3;    //ǿ����λ��Ȩ�ɹ���־  
    //SetLink2LoginAuthorizationState(LOGIN_READ_AUTHORIZATION_CODE);
    //SetTimerTask(TIME_LINK2_LOGIN, SYSTICK_1SECOND);    
}
/**
  * @brief  �������ߣ���Ҫ����������־
  * @param  type: ��������
            state: ��һ��ִ�е�״̬
  * @retval None
  */
void CertService_Down(APP_TYPE_DOWN type, Modem_APP_STATE state)
{
    CertService_TypeDown = type;//ˢ������
    CertService_State    = state;//ˢ��״̬
    CertService_ConSta   = 0;//��������
    ClearTerminalAuthorizationFlag(CHANNEL_DATA_3);//��Ȩ��־��0
    
    //////////////////////////////////////////////////////////////////////�л�IP
    if(type == DOWN_IPSW)
    {
        if((++CertService_ConTry) > 2)
        {
            CertService_ConTry = 0;
            if(CertService_IpVal == VAL_IPMAIN)
            {
                CertService_ConVal = 10*MODEM_TMIN;
                CertService_IpVal = VAL_IPBACK;//��IP-����ʧ��ת��-����IP
            }
            else if(CertService_IpVal == VAL_IPBACK)
            {
                CertService_IpVal = VAL_IPMAIN;//����IP-����ʧ��ת��-��IP
            }
            else if(CertService_IpVal == VAL_IPTO)
            {
                CertService_IpVal = VAL_IPMAIN;//ָ��IP-����ʧ��ת��-��IP
            }            
        }  
    }
    //////////////////////////////////////////////////////////////////////��ǰIP
    else if(type == DOWN_IPOP)
    {
        ;//����
    }
    ////////////////////////////////////////////////////////////////////////��IP
    else if(type == DOWN_IPMN)
    {
        CertService_IpVal = VAL_IPMAIN;//��IP
    }   
    //////////////////////////////////////////////////////////////////////ָ��IP
    else if(type == DOWN_IPTO)
    {
        CertService_IpVal = VAL_IPTO;//ָ��IP
    }     
}

/**
  * @brief  ���ӳ�ʼ��
  * @param  None
  * @retval None
  */
void CertService_Init(void)
{
    SOC_IPVAL ip;
    u32 heartval;
    
    CertService_State  = MODEM_APP_IDLE;
    CertService_IpVal  = VAL_IPMAIN;//Ĭ��������
    CertService_ConVal = 0;//Ĭ��һֱ����
    CertService_ConSta = 0;//Ĭ������
    CertService_HeartVal = MODEM_TMIN*5;//Ĭ��5��������  ��֤ƽ̨���������

    //////////////////////////////////////////////////////////////////��Ȩ��־��
    ClearTerminalAuthorizationFlag(CHANNEL_DATA_3);

    //////////////////////////////////////////////////////////////////////IP����
    if(CertService_ParaConfig(CertService_IpVal,&ip) == 0)
    {
        return;//δ��������IP���������
    }
    CertService_State = MODEM_APP_OPEN;//������
    
    ////////////////////////////////////////////////////////////////////��������
    if(CertService_ParaConfig(VAL_HEART,&heartval))
    {
        CertService_HeartVal = heartval*3*MODEM_TSEC;// 3������ֵ��ʱ
    }
    if((CertService_HeartVal > (MODEM_TMIN*5))||(CertService_HeartVal < 30))//dxl,2015.4.21,������Сֵ���޶�,��ֹ����ʱ�������ô�����������
    {
        CertService_HeartVal = MODEM_TMIN*5;//ǿ��תΪ5����
    }

		CertService_HeartVal = 0;//��֤ƽ̨���������
}
/*
  * @brief  ���ӻص�����,ƽ̨�·�����ʱ������
  * @param  *p��ָ�������·�������
  * @param  len�����ݳ���
  * @retval None
  */
void CertService_Call(u8 *p,u16 len)
{
    CertService_ConSta   = 1;//�����߱�־
    CertService_HeartCnt = 0;//����������
    RadioProtocol_AddRecvDataForParse(CHANNEL_DATA_3,p,len);   
}
/*
  * @brief  ����IP����,����IPʱ������
  * @param  *pMode��ָ����������
  * @param  *pAddr��ָ��IP��ַ
  * @param  *pPort��ָ��˿ں�
  * @retval None
  */
void CertService_IP(u8 *pMode ,u8 *pAddr, u8 *pPort)
{ 
    SOC_IPVAL ip;

    memset((void*)&ip,0,sizeof(SOC_IPVAL));//��սṹ��
    
    if(CertService_ParaConfig(CertService_IpVal,&ip) == 0)
    {
        CertService_IpVal = VAL_IPMAIN;//�����Ƿ���ǿ��תΪ��������
        CertService_ParaConfig(CertService_IpVal,&ip);
    }
    strcpy((char*)pMode,(char*)ip.mode); //TCP��UDP
    strcpy((char*)pAddr,(char*)ip.ip);   //IP��ַ������
    strcpy((char*)pPort,(char*)ip.port); //�˿�
}


/*
  * @brief  ������
  * @param  None
  * @retval None
  */
void CertService_Open(void)
{
    if((CertService_State == MODEM_APP_OPEN))//���ڴ�����
    {
        return;
    }
    CertService_State = MODEM_APP_OPEN;
}
/*
  * @brief  �ر�����
  * @param  None
  * @retval None
  */
void CertService_Close(void)
{
    if(CertService_State != MODEM_APP_CLOSE)
    {
        CertService_State = MODEM_APP_CLOSE;
    }
	ClearTerminalAuthorizationFlag(CHANNEL_DATA_3);//��Ȩ��־��0
}
/*
  * @brief  ���ӵ�ָ��IP
  * @param  ip_value: ip����;
            time:��Ч��ֹʱ��
  * @retval None
  */
void CertService_ConTo(SOC_IPVAL ip_value, u16 time)
{
    CertService_AppointIp = ip_value;//IP����
    CertService_ConVal    = MODEM_TMIN*time;//�������ӵ�ʱ�� 
    CertService_State     = MODEM_APP_DOWN;//��ǰ����ʱ���ѵ�
    CertService_TypeDown  = DOWN_IPTO;//�������ӵ�ָ��IP  
}

/*
  * @brief  �������ݵ�����ƽ̨
  * @param  None
  * @retval �ɹ����س��ȣ�ʧ�ܷ���0
  */
u16  CertService_Send(u8 *p, u16 len)
{
    u16 sendlen=0;

    if(Modem_State_SocSta(ICCARD_CERTIFICATE_SERVICE_SOC))
    {
        sendlen = Modem_Api_Socket_Send(ICCARD_CERTIFICATE_SERVICE_SOC,p,len);
    }
   
    return sendlen;
}
/**
  * @brief  ���Ӽ���(���ڴ��ڸ�״̬)
  * @param  None
  * @retval None
  */
void CertService_Listen(void)
{
    ////////////////////////////////////////////////////////////////////��·���
    if(Modem_Api_SocSta(ICCARD_CERTIFICATE_SERVICE_SOC) == __FALSE)
    {
        CertService_Down(DOWN_IPOP,MODEM_APP_OPEN);
        return;
    } 
    //////////////////////////////////////////////////////////���ݻ�ָ�����Ӽ��
    if(CertService_ConVal)
    {
        if((CertService_ConCnt++) > CertService_ConVal)
        {
            CertService_ConCnt   = 0;
            CertService_ConVal   = 0;
            CertService_State    = MODEM_APP_DOWN;//�ر����� -> ����ʱ���ѵ�
            CertService_TypeDown = DOWN_IPMN;//�����л�����IP
        }
    }
    ////////////////////////////////////////////////////////////////������ʱ���
    if(CertService_HeartVal){
	    if((CertService_HeartCnt++) > CertService_HeartVal)
	    {
	        CertService_HeartCnt = 0;
	        CertService_State    = MODEM_APP_DOWN;//�ر����� -> ������ʱ
	        CertService_TypeDown = DOWN_IPOP;//�������´�(IP����)   
	    }
    }
}

/**
  * @brief  ����״̬
  * @param  None
  * @retval 0 -> ������
            1 -> ����
  */
u8 CertService_ConState(void)
{
    return CertService_ConSta;
}

/**
  * @brief  ���Ӽ��
  * @param  None
  * @retval None
  */
void CertService_Check(void)
{

}


/**
  * @brief  ����ר��
  * @param  None
  * @retval None
  */
void CertService_Debug(void)
{
    static u8 Second_debug;
    
    if(Second_debug)
    {
        Second_debug = 0;
    }
}
u8 CertService_GetLinkFlg(void)
{
	return CertService_Link_Flg;
}
void CertService_SetLinkFlg(void)
{
	CertService_Link_Flg = 1;
}
void CertService_ClrLinkFlg(void)
{
	CertService_Link_Flg = 0;
}

/**
  * @brief  ���Ӵ���
  * @param  None
  * @retval None
  */
void CertService_Run(void)
{

    MODERR_TYPE modemack;

    //CertService_Debug();

    //CertService_Check();
    if(0 == CertService_Link_Flg)return;//����IC��û���벻������֤������
    switch(CertService_State)
    {
        case MODEM_APP_IDLE:
            {
                ;
            }
        break; 
        case MODEM_APP_OPEN://����һ������
            {
                modemack = Modem_Api_Socket_Open(ICCARD_CERTIFICATE_SERVICE_SOC,
                                                 CertService_IP,
                                                 CertService_Call);
                if(modemack == MOD_OK)
                {
                    CertService_Login(MODEM_APP_LISTEN);//�ɹ�->�������
                }
                else if(modemack == MOD_ER)
                {
                    CertService_Down(DOWN_IPSW,MODEM_APP_OPEN);//ʧ��->�л�IP
                }
            }
        break;     
        case MODEM_APP_LISTEN://����һ������,���ڴ��ڸ�״̬
            {
                CertService_Listen();
            }
        break;        
        case MODEM_APP_CLOSE://�ر�һ������
            {   
                modemack = Modem_Api_Socket_Close(ICCARD_CERTIFICATE_SERVICE_SOC);
                if(modemack == MOD_OK)
                {
                    CertService_State = MODEM_APP_IDLE;//�ȴ�Ӧ�ô�
                }
                else if(modemack == MOD_ER)
                {
                    Modem_Api_Rst();//ģ���쳣->����ģ��
                }
            }
        break;        
        case MODEM_APP_DOWN://��������
            {
                modemack = Modem_Api_Socket_Close(ICCARD_CERTIFICATE_SERVICE_SOC);
                if(modemack == MOD_OK)
                {
                    CertService_Down(CertService_TypeDown,MODEM_APP_OPEN);
                }
                else if(modemack == MOD_ER)
                {
                    Modem_Api_Rst();//�ر�����ʧ�ܺ�����ͨѶģ��
                }
            }
        break; 
        default:
        break;

    }
}


/*
********************************************************************************
*                 DEFIEN END
********************************************************************************
*/
#endif 










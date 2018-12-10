/*************************************************************************
* Copyright (c) 2013,�������������¼����������޹�˾
* All rights reserved.
* �ļ����� : Module_IC_Card_JTB.c
* ��ǰ�汾 : 1.0
* ������   : Shigle
* �޸�ʱ�� : 2013��10��17��
* 2013��10��17��Ϊ���߰汾1.0, �������������������޸ģ���Ҫ������׷��˵��
* 1������ͬʱ֧���۵ۻ��Ϊ��ͨ���涨�ļ�ʻԱ��
* 2�������ϵ磬���д洢��ȫ�ֱ���ȫ���㣬һ��ȫ�µ�IC������������ʷ�����޹� 
* �޸�ʱ�� : 2017.6.5
* �޸���   : ZWB
* ֧�ֹ��ݶ������˿�

1,		��������
��������APN:gzjtxx03.gd
������ķ�����IP:10.190.10.110		port:9205  ����ר�ÿ��������������������ƽ̨�����Ӳ��ϵ�
IC����֤������IP:192.168.201.95	port:11000		��Ҫ���ڽ��˿���֤
����2����:bit15��Ϊ1,�����ó�0x8000	 �����汾������Ҳ��

2,		���ڸ�������
ʹ�ô������ַ���"card_clr"���������������Ϣ,��ʱ��Ҫ��������֤����
ʹ�ô������ַ���"card_ent_pwr_save"������ʡ��ģʽ��ûɶ����
ʹ�ô������ַ���"card_esc_pwr_save"���˳�ʡ��ģʽ��ûɶ����

3,		����
acc on״̬��,1���Ӽ���������������·̽ѯ,����3����Ӧ����λ"��·����֤ IC��ģ�����"����λ,�յ�Ӧ�𱨾����

acc off״̬��,����������ʡ��ģʽ,4���Ӻ�رն�������Դ,acc on�˳�ʡ��ģʽ,�򿪶�������Դ

����IC�����ϰ�ǩ��,�γ�IC���°�ǩ��.
���IC������״̬�¶ϵ����´��ϵ粢��ǩ��(�൱�ڼ�ʻԱ�����ϰ�),ֱ���ο�ǩ��.�в���,�аγ�����һ�����������°����

4,		�ն˹̼��汾��:11708

��֤����
1,�ϵ�ָʾ---->��������(01)--->Ӧ��Ͷ��ˣ�����Ҫ;

2,�忨----->��������(40)--->״̬λΪ0ʱ���������64�ֽڵ���֤��Ϣ,����Ҫ;

3,0900�ϱ���֤������--->��֤������������֤��Ϣ--->�������״̬λ(ò�Ʋ���Ҫ)+24�ֽ�(��֤��Ϣ)������Ҫ;

4,��֤���(��״̬λ,˵����Ҫ�ǲ����ܵ�)Ϊ0x01ʱ������ȷӦ��,��ʱ�ն��������ģ�鷵������25�ֽ�(0x00+��֤��Ϣ),������0x00�ſ��Դ���41�������,����Ҫ

5,�ο�������42���ǩ�ˣ�ûɶ��˵��

6,��֤�ɹ����´β忨�Ͳ�������֤�����ˣ����Ҫ�������ԵĻ�������ʹ�ô������ַ���"card_clr"���������������Ϣ,��ʱ��Ҫ��������֤����

7��ˢ������Դ�绰?137 1083 8320?�������������̨���Կ�����֤�Ƿ�ɹ�



*************************************************************************/


//***************�����ļ�*****************
#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "include.h"

#define LEN_AUTH_REQUEST_CODE 64
#define LEN_AUTH_ACK_CODE	  24+1//��֤״̬λ+��֤��Ϣ
#define MAX_ICCARD_BUF	 256

extern TIME_T	CurTime;
extern u8	TerminalAuthorizationFlag;//�ն˼�Ȩ��־,bit0:����1;bit1:����2;bit2~bit7����;

unsigned char ICCard_SendCmdForReadModuleAttribute(u8	cmdType,u8 dataType);
unsigned char ICCard_SendCmdForReadElectronicCertificate(u8	cmdType,u8 dataType);
unsigned char ICCard_SendCmdForClearElectronicCertificate(u8	cmdType,u8 dataType);
void 					ICCard_Order_Process(void);
void 					ICCard_Process_CMD_0x41_Result_0x00( void );
void					ICCARD_Report_TerminalReadCardAttributeInfoAck(void);
void				 	ICCARD_Report_TerminalReadCardClrInbuiltInfoAck(void);
void 					ICCard_Send_CMD_To_Card_Reader_0x01( void );
void 					ICCard_Send_CMD_To_Card_Reader_0x02( void );
// ʹ�ö������˿�ʱ����ȡ��֤����������
void ICCARD_ReadCertSevicePara(void);

//ģ����֤����������Ϣ  24bytes
#pragma pack(1)
typedef struct
{
  //u8	peripheralType;				//��������  0x0b ����ģ��
  u8	authPlatformCalStatus;//��֤ƽ̨����״̬ 00:���� 01:�쳣
  u8	authArea;							//��֤���� 	01:��ҵ��Ա�ʸ�֤��Ӧ�� 
  //				02:��ҵ��Ա�ʸ�֤����չӦ��
  //				03:��·����֤����չӦ��
  //				04:��·����֤����չӦ��
  u8	physicsCardNo[5];			//������
  u8	cardRandomCode[8];		//��Ƭ�����
  u8	cardAuthCode[8];			//��Ƭ��֤��
  u8	placeHolder;					//ռλ��
}STT_ICCARD_AUTH_BACK_INFO;
//��ģ������
typedef struct
{
  u8	peripheralManufacturerNo[5];		//���������̱��
  u8	peripheralHardVersion[3];				//����Ӳ���汾��
  u8	peripheralSoftVersion[3];				//��������汾��
  u8	manufacturerNo[2];							//���̱��  BCD
  u8	curFirmwareVersion[4];					//��ǰ�̼��汾��
  u8	curHardVersion[4];							//��ǰӲ���汾��
  u8	cardModuleMode;									//��ģ�� 1:���� 2:˫�� 3:����
  u8	upgradeFlag;										//������ʶ
  u8	upgradeFirmwareVersion[4];			//�����̼��汾��
  u8	curUpgradePackNo[2];						//��ǰ��������
}STT_ICCARD_MODULE_ATTRIBUTE;
#pragma pack()
#define	ROAD_TRANSPORT_CERT		0x01	//��·����֤
#define	EMPLOYEE_CERT					0x02	//��ҵ��Ա�ʸ�֤

typedef enum peripheralCmdType
{
  POWER_ON_INDICATE 					= 0x01,			//�ϵ�ָʾ
  LINK_INQUIRE 								= 0x02,			//��·̽ѯ
  SLAVE_POWER_CONTROL 				= 0x03,			//�ӻ���Դ����
  SLAVE_VERSION_QUERY					= 0x04,			//�ӻ��汾��ѯ
  SLAVE_SELF_INSPECTION				= 0x05,			//�ӻ��Լ�
  SLAVE_FIRMWARE_UPDATE				= 0x06,			//�ӻ��̼�����
  IC_CARD_READ_ATTRIBUTE			= 0x07,			//��IC������
  IC_CARD_AUTHENTICATION 			= 0x40,			//IC����֤
  IC_CARD_READ 								= 0x41,			//IC����ȡ
  IC_CARD_PULL 								= 0x42,			//IC�γ�
  IC_CARD_INITIATIVE_READ 		= 0x43,			//����������IC��
  IC_CARD_READ_E_CERT					= 0xA1,			//�����õ���֤��
  IC_CARD_CLR_E_CERT					= 0xA2,			//������õ���֤��
  IC_CARD_SET_TIME						= 0xA4,			//����ʱ��
  IC_CARD_SPECIAL_ACK					= 0xAA,			//����Ӧ��,��Ե�Դ����Ӧ��,�ӻ��Լ��
  
  IC_CARD_CHECK_START					= 0xF0,			//�Զ����,����⿪ʼ
  IC_CARD_CHECK_END						= 0xF1,			//�Զ����,��������
}PERIPHERAL_CMD_Def;//������������
#define	SLAVE_FIRMWARE_DATA_LEN	516
typedef struct slaveFirmwareUpdate
{
  u8	packData[SLAVE_FIRMWARE_DATA_LEN];
}SLAVE_FIRMWARE_UPDATE_MSG_BODY;//�ӻ�ģ��̼�������Ϣ��

static SLAVE_FIRMWARE_UPDATE_MSG_BODY	g_slaveFirmwarePackInfo;
typedef struct 
{
  u8 	cardLaunchStep;
  u8	lastCardLaunchStep;
  u16	delayCount;
}ICCARD_LAUNCH_CTRL;
//static ICCARD_LAUNCH_CTRL cardLaunchCtrl;

typedef struct
{
  u8	step:4;
  u8	sendStatus:1;
  u8	ackStatus:1;
  u8	hold:2;
  u8	cmdType;			//��������
  u8	dataType;			//��������
  u8	retryTimes;		//���Դ���
  u16	counter;
  u8	E_Signature[8];//����ǩ��,��ƽ̨�·��������
  STT_ICCARD_MODULE_ATTRIBUTE	cardAttribute;
}TERMINAL_SEND_CTRL;//�ն˷���ָ�

static TERMINAL_SEND_CTRL	gTerminalSendOrderCtrl;

typedef struct
{
  u8	cmdType;			//��������
  unsigned char	(*operate)(u8	cmdType,u8 dataType);
  void					(*response)(void);
}TERMINAL_SEND_ORDER;//�ն˷���ָ�

const TERMINAL_SEND_ORDER gTerminalSendOrderTab[] =
{
  //POWER_ON_INDICATE,ICCard_Send_CMD_To_Card_Reader_0x01,NULL,//�ϵ�ָʾ
  //LINK_INQUIRE,ICCard_Send_CMD_To_Card_Reader_0x02,NULL,			//��·̽ѯ
  IC_CARD_READ_ATTRIBUTE,ICCard_SendCmdForReadModuleAttribute,ICCARD_Report_TerminalReadCardAttributeInfoAck,
  //IC_CARD_READ_E_CERT,ICCard_SendCmdForReadElectronicCertificate,ICCard_CustomProtocolParse_Ack_0x0702,
  IC_CARD_CLR_E_CERT,ICCard_SendCmdForClearElectronicCertificate,ICCARD_Report_TerminalReadCardClrInbuiltInfoAck,
};

typedef struct tagSTT_ICCARD_CTRL_JTB
{
  BOOL Flag_Radio_Connected;
  BOOL Flag_Radio_Opened;
  BOOL Flag_Init;
  u16 Com_Buffer_Length;
  ST_PROTOCOL_EXDEVICE stt_exdev808;
  u16 stt_exdev808_len;
  u8	AuthRequestCode[LEN_AUTH_REQUEST_CODE];
  u8	AuthCode[LEN_AUTH_ACK_CODE];
  u8  Flag_AuthCode_Success;
  u8	SM_Stage;
  u8 	activeCardTimes; //�Զ�����40Hָ�����½�����֤,����ط�3��
  u16	SM_Counter;
  u16	LinkInquireCount;
}STT_ICCARD_CTRL_JTB;

typedef enum	
{
  AUTH_REQUEST_SUCCESS=0,				//��֤����ɹ�
  AUTH_REQUEST_TERMINAL_OFFLINE,//�ն˲�����
  AUTH_REQUEST_TIMEOUT,					//��֤��ʱ
  AUTH_REQUEST_VERIFY						//ȷ���յ���Ϣ
}STT_AUTH_REQUEST_RESULT;

//***************����ȫ�ֱ���������*****************
STT_ICCARD_CTRL_JTB gICCardJTBCtrl;


//*************** ���������� *****************

/********************************************************************
* ���� : ICCard_Clear_Buf_COM
* ���� : ��������ͨѶ�Ļ�����
* ˵��   mode: 0: ������ĵ��ֽ� 1������յ�ǰ���ȵ��ֽ�
********************************************************************/
void ICCard_Clear_Buf_COM( u8 mode )
{
  gICCardJTBCtrl.Com_Buffer_Length = 0 ;
}

/********************************************************************
* ���� : ICCard_Process_CMD_0x40_Request_COM_Ack_Code
* ���� : ��Ӧ��״̬�� ������������һ���� �� �ն�=>�۵�IC�������� ��
* ˵�� : �ն��ڽӵ�����������0x40�󣬸��ݲ�ͬ�����Ӧ�� ��������Ӧ���ݡ�
********************************************************************/
void ICCard_Process_CMD_0x40_Request_COM_Ack_Code( u8 AckCode)
{
  ICCard_PackAndSendData(IC_CARD_AUTHENTICATION,&AckCode,1);
}

/********************************************************************
* ���� : ICCard_Process_CMD_0x40_Request_COM_Ack_Code
* ���� : ��Ӧ��״̬�� ������������һ���� �� �ն�=>�۵�IC�������� ��
* ˵�� : �ն��ڽӵ�����������0x40��Ӧ�������0x03:�ն�ȷ���յ���Ϣ( IC����֤���������� = 0x01 ~ 0x04  ʱ);
********************************************************************/
void ICCard_Process_CMD_0x40_Request_COM_Ack_Received( void )
{
  //IC����֤����Ӧ�� 0x03:�ն�ȷ���յ���Ϣ( IC����֤���������� = 0x01 ~ 0x04  ʱ);
  ICCard_Process_CMD_0x40_Request_COM_Ack_Code(0x03);  //IC����֤����Ӧ�� 0x03:�ն�ȷ���յ���Ϣ( IC����֤���������� = 0x01 ~ 0x04  ʱ);
}
/********************************************************************
* ���� : ICCard_Process_CMD_0x40_Request_COM_Ack_Result
* ���� : 
* ˵�� : �ն���״̬λΪ0x00ʱ������֤���ķ���64�ֽڿ�Ƭ������Ϣ����֤��Ϣ��
*				 �����ݲ�ͬ��������ģ�鷵��1��25�ֽڽ����Ϣ
********************************************************************/
void ICCard_Process_CMD_0x40_Request_COM_Ack_Result( u8 *pBuffer,u16 BufferLen )
{
  ICCard_PackAndSendData(IC_CARD_AUTHENTICATION,pBuffer,BufferLen);  
}

/********************************************************************
* ���� : IC_Card_Terminal_Ack_Reader_Success
* ���� : �ն�=> �۵�IC���������� ��֮ �ն��Ѿ��ɹ������˿�ǩ���Ŀ�Ƭ���ݣ���ǩ����Ϣ
* ˵�� : 
********************************************************************/
void ICCard_Terminal_Ack_Reader_Success( u8 Cmd)
{
  //u8 SignInAckSuccessBuffer[9]  ={0x7E, 0x4D,0x00,0x01,0x01,0x00,0x0B,0x41,0x7E};
  //u8 SignOutAckSuccessBuffer[9] ={0x7E, 0x4E,0x00,0x01,0x01,0x00,0x0B,0x42,0x7E};    
  unsigned char TempBuf = 0;
  ICCard_PackAndSendData(Cmd,&TempBuf,0);
}

/********************************************************************
* ���� : ICCard_Net_Process
* ���� : IC����֤���ģ��������Ӵ���
* ˵�� : 
********************************************************************/
void ICCard_Net_Process( void )
{
  /**************IC����֤���ĵ��������� ********************/
  if(ICAUC_GetLnkStatus())
  { //��ѯIC����֤����GPRS�����Ƿ��Ѿ������ɹ�/
    //�Ѿ���������
    //��־λ0 ����: IC����֤����GPRS���Ӹս���
    if( FALSE ==  gICCardJTBCtrl.Flag_Radio_Connected ) 
    {
      gICCardJTBCtrl.Flag_Radio_Connected = TRUE ; 
    }
    return ;              
  } 
  //��δ��������
  if( TRUE == gICCardJTBCtrl.Flag_Radio_Connected) 
  { //���ӶϿ�
    gICCardJTBCtrl.Flag_Radio_Connected = FALSE ; //��0 ����: IC����֤����GPRS����δ����
    gICCardJTBCtrl.Flag_Radio_Opened = FALSE ;//�����û�н������ӣ����ж��Ƿ������ 
  }
  if( FALSE == gICCardJTBCtrl.Flag_Radio_Opened  )
  { //�����û�н������ӣ����ж��Ƿ������
    
    if( ICAUC_RET_SUCCED == ICAUC_OpenLnk()  )
    {//����IC����֤���ĵ�����
      gICCardJTBCtrl.Flag_Radio_Opened= TRUE ;  //����IC����֤���ĵ����ӳɹ��󣬴�
    }
  }
}
/********************************************************************
* ���� : ICCard_Net_Rx
* ���� : �ص�����, ����IC����֤�������緢��������, ������ȡ��Ч����֤��Ϣ
* ˵�� : 
********************************************************************/
void ICCard_Net_Rx( u8 * pInBuf, u16 InLength )
{
  gICCardJTBCtrl.AuthCode[0] = 1;
  gICCardJTBCtrl.Flag_AuthCode_Success = 1;
  if(InLength<14+LEN_AUTH_ACK_CODE)return;
  if((pInBuf[13]==0x0B)&&(pInBuf[14]==0x00))
  {
    memcpy(gICCardJTBCtrl.AuthCode,&pInBuf[14], LEN_AUTH_ACK_CODE);
  }
}

/********************************************************************
* ���� : ICCard_WaitOrClrPlatAuthAnswer
* ���� : IC���ȴ������ƽ̨��֤Ӧ��
* ˵��   
********************************************************************/
void ICCard_WaitOrClrPlatAuthAnswer(void)
{
  gICCardJTBCtrl.Flag_AuthCode_Success = 0;
}
/********************************************************************
* ���� : ICCard_VerifyPlatAuthAnswer
* ���� : IC��ȷ��ƽ̨��֤Ӧ��
* ˵��   
********************************************************************/
void ICCard_VerifyPlatAuthAnswer(void)
{
  gICCardJTBCtrl.Flag_AuthCode_Success = 1;
}
/********************************************************************
* ���� : ICCard_GetPlatAuthAnswer
* ���� : IC����ȡƽ̨��֤Ӧ��
* ˵��   
********************************************************************/
u8 ICCard_GetPlatAuthAnswer(void)
{
  return gICCardJTBCtrl.Flag_AuthCode_Success;
}
/**************************************************************************
//��������ICCard_AuthenticationAck
//���ܣ���֤Ӧ��
//���룺��
//�������
//����ֵ����
//��ע����һ��״̬Ϊ����
***************************************************************************/
void ICCard_AuthenticationAck(u8 *pBuffer , u16 BufferLen)
{
  memcpy(gICCardJTBCtrl.AuthCode,pBuffer, BufferLen);
  ICCard_VerifyPlatAuthAnswer();
}

/*************************************************************
** ��������: ICCard_DisposeRecvExdeviceJTB
** ��������: ���ݽ���
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/
void ICCard_DisposeRecvExdeviceJTB(ST_PROTOCOL_EXDEVICE stExdevice, unsigned short datalen)
{
  if(2 == gICCardJTBCtrl.SM_Stage)
  {
    gICCardJTBCtrl.SM_Stage = 0 ;
    gICCardJTBCtrl.SM_Counter = 0 ;
    ICCard_Clear_Buf_COM(0);
  }
  else
  {
    memcpy(&gICCardJTBCtrl.stt_exdev808,&stExdevice,sizeof(ST_PROTOCOL_EXDEVICE));
    gICCardJTBCtrl.Com_Buffer_Length = datalen;
    gICCardJTBCtrl.stt_exdev808_len  = datalen;
  }
}
/********************************************************************
* ���� : ICCard_Process_CMD_0x01
* ���� : IC���б����������:  0x01 �ӻ��ϵ�ָʾӦ��
* ˵��   �ն�->������
********************************************************************/
void ICCard_Send_CMD_To_Card_Reader_0x01( void )
{
  ICCard_Terminal_Ack_Reader_Success( POWER_ON_INDICATE);
  gICCardJTBCtrl.SM_Stage = 5;
}
/********************************************************************
* ���� : ICCard_Recieved_CMD_From_Card_Reader_0x01
* ���� : IC���б����������:  0x01 �ӻ��ϵ�ָʾӦ��
* ˵��   ������->�ն�
********************************************************************/
void ICCard_Recieved_CMD_From_Card_Reader_0x01( void )
{
  
}

/********************************************************************
* ���� : ICCard_Send_CMD_To_Card_Reader_0x02
* ���� : IC���б����������:  0x02 ������·̽ѯӦ��
* ˵��   �ն�->������
********************************************************************/
void ICCard_Send_CMD_To_Card_Reader_0x02( void )
{
  ICCard_Terminal_Ack_Reader_Success( LINK_INQUIRE );
  gICCardJTBCtrl.SM_Stage = 5;
}
/********************************************************************
* ���� : ICCard_Send_CMD_To_Card_Reader_0x02
* ���� : IC���б����������:  0x02 ������·̽ѯӦ��
* ˵��   ������->�ն�
********************************************************************/
void ICCard_Recieved_CMD_From_Card_Reader_0x02( void )
{
  
}

/********************************************************************
* ���� : ICCard_Send_CMD_To_Terminal_0x03
* ���� : IC���б����������:  0x03 �ӻ���Դ����
* ˵��   �ն˷�������������� controlMode 00:�˳�ʡ��ģʽ  01:����ʡ��ģʽ
********************************************************************/
void ICCard_Send_CMD_To_Card_Reader_0x03( u8 controlMode )
{
  u8 TempData = controlMode;
  
  ICCard_PackAndSendData(SLAVE_POWER_CONTROL,&TempData,1);
}

/********************************************************************
* ���� : ICCard_Send_CMD_To_Terminal_0x04
* ���� : IC���б����������:  0x04 �ӻ��汾��Ϣ��ѯ
* ˵��   
********************************************************************/
void ICCard_Send_CMD_To_Card_Reader_0x04( void )
{	
  u8 TempData;
  
  ICCard_PackAndSendData(SLAVE_VERSION_QUERY,&TempData,0);
}

/********************************************************************
* ���� : ICCard_Process_CMD_0x04
* ���� : IC���б����������:  0x04 �ӻ��汾��Ϣ��ѯӦ��
* ˵��  
********************************************************************/
void ICCard_Process_CMD_0x04( void )
{
  u8 slaveVerNo[2];	//�ӻ��汾��
  
  memcpy(slaveVerNo,gICCardJTBCtrl.stt_exdev808.Data,2);
  gICCardJTBCtrl.SM_Stage = 5;
}
/********************************************************************
* ���� : ICCard_Send_CMD_To_Terminal_0x05
* ���� : IC���б����������:  0x05 
* ˵��   slaveType �ӻ��Լ�����
********************************************************************/
void ICCard_Send_CMD_To_Card_Reader_0x05( u8 slaveType )
{
  u8 TempData = slaveType;
  
  ICCard_PackAndSendData(SLAVE_SELF_INSPECTION,&TempData,1);
}

/********************************************************************
* ���� : ICCard_Send_CMD_To_Terminal_0x06
* ���� : IC���б����������:  0x06 
* ˵��   ���´ӻ��̼�
********************************************************************/
void ICCard_Send_CMD_To_Terminal_0x06( void )
{		
  ICCard_PackAndSendData(SLAVE_FIRMWARE_UPDATE,g_slaveFirmwarePackInfo.packData,SLAVE_FIRMWARE_DATA_LEN);
}

/********************************************************************
* ���� : ICCard_Process_CMD_0x06
* ���� : IC���б����������:  0x06 �ӻ��̼�����
* ˵��  
********************************************************************/
void ICCard_Process_CMD_0x06( void )
{
  u16	packSerial;//�����
  u8 ackResult;	//Ӧ����
  u8 *p = gICCardJTBCtrl.stt_exdev808.Data;
  
  packSerial = (*p++)<<8;
  packSerial |= *p++;
  ackResult = *p;
  
  if(0x00 == ackResult)
  {
    //�����ɹ�
  }
  else
    if(0x01 == ackResult)
    {
      //�Ǳ��̼�����
      
    }
    else
      if(0x02 == ackResult)
      {
        //�ط� 3�κ���ֹ��������
      }
      else
      {
        
      }	
}

/********************************************************************
* ���� : ICCard_Process_CMD_0x07
* ���� : IC���б����������:  0x07 ��ȡ��ģ���ۺ�������Ϣ
* ˵��  
********************************************************************/
void ICCard_Process_CMD_0x07( void )
{
  memcpy( &gTerminalSendOrderCtrl.cardAttribute.peripheralManufacturerNo[0], \
    &gICCardJTBCtrl.stt_exdev808.Data[0], \
      sizeof(STT_ICCARD_MODULE_ATTRIBUTE) ); //����ģ������
  gTerminalSendOrderCtrl.ackStatus = 1;
}
/********************************************************************
* ���� : ICCard_Process_CMD_0x40_request_0
* ���� : IC���б����������:  0x40 IC����֤����
* ˵��:  ��֤������ ״̬λ 0: IC�������ɹ�
********************************************************************/
void ICCard_Process_CMD_0x40_Request_0x00( void )
{
  memcpy( &gICCardJTBCtrl.AuthRequestCode[0], &gICCardJTBCtrl.stt_exdev808.Data[1], LEN_AUTH_REQUEST_CODE ); //������ ����64�ֽڿ�Ƭ������Ϣ����֤��Ϣ
  //�ն������������Ӧ������
  ICCard_Terminal_Ack_Reader_Success( IC_CARD_AUTHENTICATION);
  gICCardJTBCtrl.SM_Stage = 1;
  gICCardJTBCtrl.SM_Counter = 0 ;
  if(0 == CertService_GetLinkFlg())//�����ɹ������֤����������ͨ��
  {
    CertService_SetLinkFlg();
  }
  else if(CHANNEL_DATA_3 != (CHANNEL_DATA_3&TerminalAuthorizationFlag))
  {
    CertService_Open();
  }
}

/********************************************************************
* ���� : ICCard_Process_CMD_0x40_request_1
* ���� : IC���б����������:  0x40 IC����֤����
* ˵��:  ��֤������ ״̬λ 1: IC��δ����
********************************************************************/
void ICCard_Process_CMD_0x40_Request_0x01( void )
{
  //IC����֤����Ӧ�� 0x02:�ն�͸����֤���ĳ�ʱ��Ӧ��  
  Public_ShowTextInfo( "40IC����֤����ʧ��ԭ��1:IC��δ����", 30 );
  ICCard_Process_CMD_0x40_Request_COM_Ack_Received();
  gICCardJTBCtrl.SM_Stage = 0;
  gICCardJTBCtrl.SM_Counter = 0 ;
}

/********************************************************************
* ���� : ICCard_Process_CMD_0x40_request_2
* ���� : IC���б����������:  0x40 IC����֤����
* ˵��:  ��֤������ ״̬λ 2: IC������ʧ��
********************************************************************/
void ICCard_Process_CMD_0x40_Request_0x02( void )
{
  Public_ShowTextInfo( "40IC����֤����ʧ��ԭ��2:IC������ʧ��", 30 ); 
  ICCard_Process_CMD_0x40_Request_COM_Ack_Received();
  gICCardJTBCtrl.SM_Stage = 0;
  gICCardJTBCtrl.SM_Counter = 0 ;  
}

/********************************************************************
* ���� : ICCard_Process_CMD_0x40_request_3
* ���� : IC���б����������:  0x40 IC����֤����
* ˵��:  ��֤������ ״̬λ 3: �Ǵ�ҵ�ʸ�֤IC��
********************************************************************/
void ICCard_Process_CMD_0x40_Request_0x03( void )
{
  Public_ShowTextInfo( "40IC����֤����ʧ��ԭ��3:�Ǵ�ҵ�ʸ�֤IC��", 30 );
  ICCard_Process_CMD_0x40_Request_COM_Ack_Received();
  gICCardJTBCtrl.SM_Stage = 0;
  gICCardJTBCtrl.SM_Counter = 0 ;
  
}

/********************************************************************
* ���� : ICCard_Process_CMD_0x40_request_4
* ���� : IC���б����������:  0x40 IC����֤����
* ˵��:  ��֤������ ״̬λ 4: //IC��������
********************************************************************/
void ICCard_Process_CMD_0x40_Request_0x04( void )
{
  Public_ShowTextInfo( "40IC����֤����ʧ��ԭ��4:IC��������", 30 );
  ICCard_Process_CMD_0x40_Request_COM_Ack_Received();
  gICCardJTBCtrl.SM_Stage = 0;
  gICCardJTBCtrl.SM_Counter = 0 ;
}

/********************************************************************
* ���� : ICCard_Process_CMD_0x40_request_4
* ���� : IC���б����������:  0x40 IC����֤����
* ˵��:  ��֤������ ״̬λ 4: //δ֪��ԭ��
********************************************************************/
void ICCard_Process_CMD_0x40_Request_0xXX( void )
{
  Public_ShowTextInfo( "40IC����֤����ʧ��ԭ��X:δ֪", 30 );
  gICCardJTBCtrl.SM_Stage = 0;
  gICCardJTBCtrl.SM_Counter = 0 ;
}
/********************************************************************
* ���� : ICCard_Process_CMD_0x40
* ���� : IC���б����������:  0x40 IC����֤����
* ˵��  
********************************************************************/
void ICCard_Process_CMD_0x40( void )
{
  u8 result_readcard = 0 ;  
  
  result_readcard = gICCardJTBCtrl.stt_exdev808.Data[0] ;
  switch( result_readcard )
  {
  case 0x00: //IC�������ɹ�
    ICCard_Process_CMD_0x40_Request_0x00();
    break;
  case 0x01: //IC��δ����
    ICCard_Process_CMD_0x40_Request_0x01();
    break;
  case 0x02: //IC������ʧ�ܣ�
    ICCard_Process_CMD_0x40_Request_0x02();
    break;
  case 0x03: //�Ǵ�ҵ�ʸ�֤IC��
    ICCard_Process_CMD_0x40_Request_0x03();
    break;
  case 0x04: //IC��������
    ICCard_Process_CMD_0x40_Request_0x04();
    break;
  default:
    ICCard_Process_CMD_0x40_Request_0xXX();
    break;
  }
}
/********************************************************************
* ���� : ICCard_Process_Repeat_0x40
* ���� : ����ģ���ط�40����
* ˵��  
********************************************************************/
void ICCard_Process_Repeat_0x40( void )
{ 
  ICCard_Terminal_Ack_Reader_Success(IC_CARD_AUTHENTICATION);
}

/********************************************************************
* ���� : ICCard_Process_CMD_0x41
* ���� : IC���б����������:  0x41 IC����ȡ���֪ͨ
* ˵��:  IC�������������� ��ȡ���֪ͨ ״̬λ 0: IC�������ɹ� ,��ʱ�к�������
********************************************************************/
void ICCard_Process_CMD_0x41_Result_0x00( void )
{
  ICCard_Terminal_Ack_Reader_Success( IC_CARD_READ);
  ////////////////////
  ICCard_DisposeJTBCardDriverSignIn(&gICCardJTBCtrl.stt_exdev808.Data[1],gICCardJTBCtrl.stt_exdev808_len);
  /////////////////////
  
  gICCardJTBCtrl.SM_Stage = 0;
  gICCardJTBCtrl.SM_Counter = 0 ;
}

/********************************************************************
* ���� : ICCard_Process_CMD_0x41
* ���� : IC���б����������:  0x41 IC����ȡ���֪ͨ
* ˵��:  IC�������������� ��ȡ���֪ͨ ״̬λ 1: IC������ʧ�� ,ԭ��Ϊ��Ƭ��Կ��֤δͨ��
********************************************************************/
void ICCard_Process_CMD_0x41_Result_0x01( void )
{
  Public_ShowTextInfo( "41����ʧ��ԭ��1:��Ƭ��Կ��֤δͨ��", 30 );  
  ICCard_Terminal_Ack_Reader_Success( IC_CARD_READ);
  gICCardJTBCtrl.SM_Stage = 0;
  gICCardJTBCtrl.SM_Counter = 0 ;  
}

/********************************************************************
* ���� : ICCard_Process_CMD_0x41
* ���� : IC���б����������:  0x41 IC����ȡ���֪ͨ
* ˵��:  IC�������������� ��ȡ���֪ͨ ״̬λ 2: IC������ʧ�� ,ԭ��Ϊ��Ƭ�ѱ�����
********************************************************************/
void ICCard_Process_CMD_0x41_Result_0x02( void )
{
  Public_ShowTextInfo( "41����ʧ��ԭ��2:��Ƭ�ѱ�����", 30 );
  ICCard_Terminal_Ack_Reader_Success( IC_CARD_READ);
  gICCardJTBCtrl.SM_Stage = 0;
  gICCardJTBCtrl.SM_Counter = 0 ;
}

/********************************************************************
* ���� : ICCard_Process_CMD_0x41
* ���� : IC���б����������:  0x41 IC����ȡ���֪ͨ
* ˵��:  IC�������������� ��ȡ���֪ͨ ״̬λ 3: IC������ʧ�� ,ԭ��Ϊ��Ƭ���γ�
********************************************************************/
void ICCard_Process_CMD_0x41_Result_0x03( void )
{
  Public_ShowTextInfo( "41����ʧ��ԭ��3:��Ƭ���γ�", 30 );
  ICCard_Terminal_Ack_Reader_Success( IC_CARD_READ);
  gICCardJTBCtrl.SM_Stage = 0;
  gICCardJTBCtrl.SM_Counter = 0 ;    
}

/********************************************************************
* ���� : ICCard_Process_CMD_0x41
* ���� : IC���б����������:  0x41 IC����ȡ���֪ͨ
* ˵��:  IC�������������� ��ȡ���֪ͨ ״̬λ 4: IC������ʧ�� ,ԭ��Ϊ����У�����
********************************************************************/
void ICCard_Process_CMD_0x41_Result_0x04( void )
{
  Public_ShowTextInfo( "41����ʧ��ԭ��4:��Ƭ����У�����", 30 );
  ICCard_Terminal_Ack_Reader_Success( IC_CARD_READ);
  gICCardJTBCtrl.SM_Stage = 0;
  gICCardJTBCtrl.SM_Counter = 0 ;    
}
/********************************************************************
* ���� : ICCard_Process_CMD_0x41
* ���� : IC���б����������:  0x41 IC����ȡ���֪ͨ
* ˵��:  IC�������������� ��ȡ���֪ͨ ״̬λ 4: IC������ʧ�� ,ԭ��Ϊ����У�����
********************************************************************/
void ICCard_Process_CMD_0x41_Result_0x05( void )
{
  Public_ShowTextInfo( "41�����ɹ�5:�����Ͽ�Ƭ�ǵ���֤��", 30 );
  ICCard_Terminal_Ack_Reader_Success( IC_CARD_READ);
  gICCardJTBCtrl.SM_Stage = 0;
  gICCardJTBCtrl.SM_Counter = 0 ;    
}
/********************************************************************
* ���� : ICCard_Process_CMD_0x41
* ���� : IC���б����������:  0x41 IC����ȡ���֪ͨ
* ˵��:  IC�������������� ��ȡ���֪ͨ ״̬λ 4: IC������ʧ�� ,ԭ��Ϊ����У�����
********************************************************************/
void ICCard_Process_CMD_0x41_Result_0x06( void )
{
  Public_ShowTextInfo( "41�����ɹ�6:�����Ͽ�Ƭ�Ǵ�ҵ��Ա�ʸ�֤", 30 );
  ICCard_Terminal_Ack_Reader_Success( IC_CARD_READ);
  gICCardJTBCtrl.SM_Stage = 0;
  gICCardJTBCtrl.SM_Counter = 0 ;    
}
/********************************************************************
* ���� : ICCard_Process_CMD_0x41
* ���� : IC���б����������:  0x41 IC����ȡ���֪ͨ
* ˵��:  IC�������������� ��ȡ���֪ͨ ״̬λ 4: IC������ʧ�� ,ԭ��Ϊ����У�����
********************************************************************/
void ICCard_Process_CMD_0x41_Result_0x07( void )
{
  Public_ShowTextInfo( "41�����ɹ�7:�����쳣", 30 );
  ICCard_Terminal_Ack_Reader_Success( IC_CARD_READ);
  gICCardJTBCtrl.SM_Stage = 0;
  gICCardJTBCtrl.SM_Counter = 0 ;    
}
/********************************************************************
* ���� : ICCard_Process_CMD_0x41
* ���� : IC���б����������:  0x41 IC����ȡ���֪ͨ
* ˵��:  IC�������������� ��ȡ���֪ͨ ״̬λ ����: IC������ʧ�� ,ԭ��δ֪
********************************************************************/
void ICCard_Process_CMD_0x41_Result_0xXX( void )
{
  Public_ShowTextInfo( "41����ʧ��ԭ��X: δ֪", 30 );
  ICCard_Terminal_Ack_Reader_Success( IC_CARD_READ);
  gICCardJTBCtrl.SM_Stage = 0;
  gICCardJTBCtrl.SM_Counter = 0 ;
}
/********************************************************************
* ���� : ICCard_Process_CMD_0x41
* ���� : IC���б����������:  0x41 IC����ȡ���֪ͨ
* ˵��  	�����ն��յ�IC����ȡ����Ƿ�Ϊ0x00��������ʹ��0x0702���������ƽ̨����֤�������Ϣ
********************************************************************/
void ICCard_Process_CMD_0x41( void )
{
  u8 Result = 0 ;
  Result = gICCardJTBCtrl.stt_exdev808.Data[0] ;    
  switch(Result )
  {
  case 0x00: //IC�������ɹ� ,��ҵ��Ա�ʸ�֤ ��ʱ�к�������
  case 0x10: //IC�������ɹ� ,��·����֤ ��ʱ�к�������
  case 0x20: //IC�������ɹ� , ��ʱ�к�������
    //Public_PlayTTSVoiceStr("IC�������ɹ�");
    ICCard_Process_CMD_0x41_Result_0x00();
    break;
  case 0x01: //IC������ʧ�� ,ԭ��Ϊ��Ƭ��Կ��֤δͨ��
  case 0x11:
  case 0x21:
    Public_PlayTTSVoiceStr("IC����Կ��֤δͨ��");
    ICCard_Process_CMD_0x41_Result_0x01();
    break;
  case 0x02: //IC������ʧ�� ,ԭ��Ϊ��Ƭ�ѱ�����
  case 0x12:
  case 0x22:
    Public_PlayTTSVoiceStr("IC���ѱ�����");
    ICCard_Process_CMD_0x41_Result_0x02();
    break;
  case 0x03: //IC������ʧ�� ,ԭ��Ϊ��Ƭ���γ�
  case 0x13:
  case 0x23:
    Public_PlayTTSVoiceStr("IC�����γ�");
    ICCard_Process_CMD_0x41_Result_0x03();
    break;
  case 0x04: //IC������ʧ�� ,ԭ��Ϊ����У�����
  case 0x14:
  case 0x24:
    Public_PlayTTSVoiceStr("IC������У�����");
    ICCard_Process_CMD_0x41_Result_0x04();
    break;
  case 0x05: //IC������ʧ�� ,ԭ��Ϊ��Ƭ�ѱ�����
  case 0x15:
  case 0x25:
    Public_PlayTTSVoiceStr("�����Ͽ�Ƭ�ǵ���֤��");
    ICCard_Process_CMD_0x41_Result_0x05();
    break;
  case 0x06: //IC������ʧ�� ,ԭ��Ϊ��Ƭ���γ�
  case 0x16:
  case 0x26:
    Public_PlayTTSVoiceStr("�����Ͽ�Ƭ�Ǵ�ҵ��Ա�ʸ�֤");
    ICCard_Process_CMD_0x41_Result_0x06();
    break;
  case 0x07: //IC������ʧ�� ,ԭ��Ϊ����У�����
  case 0x17:
  case 0x27:
    Public_PlayTTSVoiceStr("�����쳣");
    ICCard_Process_CMD_0x41_Result_0x07();
    break;
  default: //IC������ʧ�� ,ԭ��δ֪
    ICCard_Process_CMD_0x41_Result_0xXX();
    break;
  }
}

/********************************************************************
* ���� : ICCard_Process_CMD_0x42
* ���� : IC���б����������:  0x42 IC����ȡ���֪ͨ
* ˵��:  IC�������������� ��ȡ���֪ͨ ״̬λ 0: IC�������ɹ� ,��ʱ�к�������
********************************************************************/
void ICCard_Process_CMD_0x42_Result_0x00( void )
{
  ////////////////////
  //ICCard_DisposeJTBCardDriverSignOut(&gICCardJTBCtrl.stt_exdev808.Data[1],gICCardJTBCtrl.stt_exdev808_len);
  /////////////////////
  ICCard_Terminal_Ack_Reader_Success( IC_CARD_PULL);
  ICCard_DisposeExternalCardDriverSignOut();    
  gICCardJTBCtrl.SM_Stage = 0;
  gICCardJTBCtrl.SM_Counter = 0 ;
}

/********************************************************************
* ���� : ICCard_Process_CMD_0x42
* ���� : IC���б����������:  0x42 IC����ȡ���֪ͨ
* ˵��:  IC�������������� ��ȡ���֪ͨ ״̬λ 1: IC������ʧ�� ,ԭ��Ϊ��Ƭ��Կ��֤δͨ��
********************************************************************/
void ICCard_Process_CMD_0x42_Result_0x01( void )
{
  Public_ShowTextInfo( "42����ʧ��ԭ��1:��Ƭ��Կ��֤δͨ��", 30 );            
  ICCard_Terminal_Ack_Reader_Success( IC_CARD_PULL);
  gICCardJTBCtrl.SM_Stage = 0;
  gICCardJTBCtrl.SM_Counter = 0 ;  
}

/********************************************************************
* ���� : ICCard_Process_CMD_0x42
* ���� : IC���б����������:  0x42 IC����ȡ���֪ͨ
* ˵��:  IC�������������� ��ȡ���֪ͨ ״̬λ 2: IC������ʧ�� ,ԭ��Ϊ��Ƭ�ѱ�����
********************************************************************/
void ICCard_Process_CMD_0x42_Result_0x02( void )
{
  Public_ShowTextInfo( "42����ʧ��ԭ��2:��Ƭ�ѱ�����", 30 );
  ICCard_Terminal_Ack_Reader_Success( IC_CARD_PULL);
  gICCardJTBCtrl.SM_Stage = 0;
  gICCardJTBCtrl.SM_Counter = 0 ;
}

/********************************************************************
* ���� : ICCard_Process_CMD_0x42
* ���� : IC���б����������:  0x42 IC����ȡ���֪ͨ
* ˵��:  IC�������������� ��ȡ���֪ͨ ״̬λ 3: IC������ʧ�� ,ԭ��Ϊ��Ƭ���γ�
********************************************************************/
void ICCard_Process_CMD_0x42_Result_0x03( void )
{
  Public_ShowTextInfo( "42����ʧ��ԭ��3:��Ƭ���γ�", 30 );
  ICCard_Terminal_Ack_Reader_Success( IC_CARD_PULL);
  gICCardJTBCtrl.SM_Stage = 0;
  gICCardJTBCtrl.SM_Counter = 0 ;    
}

/********************************************************************
* ���� : ICCard_Process_CMD_0x42
* ���� : IC���б����������:  0x41 IC����ȡ���֪ͨ
* ˵��:  IC�������������� ��ȡ���֪ͨ ״̬λ 4: IC������ʧ�� ,ԭ��Ϊ����У�����
********************************************************************/
void ICCard_Process_CMD_0x42_Result_0x04( void )
{
  Public_ShowTextInfo( "42����ʧ��ԭ��4:��Ƭ����У�����", 30 );
  ICCard_Terminal_Ack_Reader_Success( IC_CARD_PULL);
  gICCardJTBCtrl.SM_Stage = 0;
  gICCardJTBCtrl.SM_Counter = 0 ;    
}
/********************************************************************
* ���� : ICCard_Process_CMD_0x42
* ���� : IC���б����������:  0x41 IC����ȡ���֪ͨ
* ˵��:  IC�������������� ��ȡ���֪ͨ ״̬λ 4: IC������ʧ�� ,ԭ��Ϊ����У�����
********************************************************************/
void ICCard_Process_CMD_0x42_Result_0x05( void )
{
  Public_ShowTextInfo( "42�����ɹ�5:�����Ͽ�Ƭ�ǵ���֤��", 30 );
  ICCard_Terminal_Ack_Reader_Success( IC_CARD_PULL);
  gICCardJTBCtrl.SM_Stage = 0;
  gICCardJTBCtrl.SM_Counter = 0 ;    
}
/********************************************************************
* ���� : ICCard_Process_CMD_0x42
* ���� : IC���б����������:  0x41 IC����ȡ���֪ͨ
* ˵��:  IC�������������� ��ȡ���֪ͨ ״̬λ 4: IC������ʧ�� ,ԭ��Ϊ����У�����
********************************************************************/
void ICCard_Process_CMD_0x42_Result_0x06( void )
{
  Public_ShowTextInfo( "42�����ɹ�6:�����Ͽ�Ƭ�Ǵ�ҵ��Ա�ʸ�֤", 30 );
  ICCard_Terminal_Ack_Reader_Success( IC_CARD_PULL);
  gICCardJTBCtrl.SM_Stage = 0;
  gICCardJTBCtrl.SM_Counter = 0 ;    
}
/********************************************************************
* ���� : ICCard_Process_CMD_0x42
* ���� : IC���б����������:  0x41 IC����ȡ���֪ͨ
* ˵��:  IC�������������� ��ȡ���֪ͨ ״̬λ 4: IC������ʧ�� ,ԭ��Ϊ����У�����
********************************************************************/
void ICCard_Process_CMD_0x42_Result_0x07( void )
{
  Public_ShowTextInfo( "42�����ɹ�7:�����쳣", 30 );
  ICCard_Terminal_Ack_Reader_Success( IC_CARD_PULL);
  gICCardJTBCtrl.SM_Stage = 0;
  gICCardJTBCtrl.SM_Counter = 0 ;    
}
/********************************************************************
* ���� : ICCard_Process_CMD_0x42
* ���� : IC���б����������:  0x42 IC����ȡ���֪ͨ
* ˵��:  IC�������������� ��ȡ���֪ͨ ״̬λ ����: IC������ʧ�� ,ԭ��δ֪
********************************************************************/
void ICCard_Process_CMD_0x42_Result_0xXX( void )
{
  Public_ShowTextInfo( "42����ʧ��ԭ��X: δ֪", 30 );
  ICCard_Terminal_Ack_Reader_Success( IC_CARD_PULL);
  gICCardJTBCtrl.SM_Stage = 0;
  gICCardJTBCtrl.SM_Counter = 0 ;
}
/********************************************************************
* ���� : ICCard_Process_CMD_0x42
* ���� : IC���б����������:  0x42 IC���γ�֪ͨ
* ˵��  
********************************************************************/
void ICCard_Process_CMD_0x42( void )
{ 
  //ICCard_DisposeExternalCardDriverSignOut();
  //ICCard_Terminal_Ack_Reader_Success( IC_CARD_PULL);
  u8 Result = 0 ;
  
  gICCardJTBCtrl.activeCardTimes = 0;//�ο���������
  
  Result = gICCardJTBCtrl.stt_exdev808.Data[0] ;    
  switch(Result )
  {
  case 0x00: //IC�������ɹ� ,��ʱ�к�������
  case 0x10:
  case 0x20:
    //Public_PlayTTSVoiceStr("IC�������ɹ�");
    ICCard_Process_CMD_0x42_Result_0x00();
    break;
  case 0x01: //IC������ʧ�� ,ԭ��Ϊ��Ƭ��Կ��֤δͨ��
  case 0x11:
  case 0x21:
    Public_PlayTTSVoiceStr("IC����Կ��֤δͨ��");
    ICCard_Process_CMD_0x42_Result_0x01();
    break;
  case 0x02: //IC������ʧ�� ,ԭ��Ϊ��Ƭ�ѱ�����
  case 0x12:
  case 0x22:
    Public_PlayTTSVoiceStr("IC���ѱ�����");
    ICCard_Process_CMD_0x42_Result_0x02();
    break;
  case 0x03: //IC������ʧ�� ,ԭ��Ϊ��Ƭ���γ�
  case 0x13:
  case 0x23:
    Public_PlayTTSVoiceStr("IC�����γ�");
    ICCard_Process_CMD_0x42_Result_0x03();
    break;
  case 0x04: //IC������ʧ�� ,ԭ��Ϊ����У�����
  case 0x14:
  case 0x24:
    Public_PlayTTSVoiceStr("IC������У�����");
    ICCard_Process_CMD_0x42_Result_0x04();
    break;
  case 0x05: //IC������ʧ�� ,ԭ��Ϊ��Ƭ�ѱ�����
  case 0x15:
  case 0x25:
    Public_PlayTTSVoiceStr("�����Ͽ�Ƭ�ǵ���֤��");
    ICCard_Process_CMD_0x42_Result_0x05();
    break;
  case 0x06: //IC������ʧ�� ,ԭ��Ϊ��Ƭ���γ�
  case 0x16:
  case 0x26:
    Public_PlayTTSVoiceStr("�����Ͽ�Ƭ�Ǵ�ҵ��Ա�ʸ�֤");
    ICCard_Process_CMD_0x42_Result_0x06();
    break;
  case 0x07: //IC������ʧ�� ,ԭ��Ϊ����У�����
  case 0x17:
  case 0x27:
    Public_PlayTTSVoiceStr("�����쳣");
    ICCard_Process_CMD_0x42_Result_0x07();
    break;		
  default: //IC������ʧ�� ,ԭ��δ֪
    ICCard_Process_CMD_0x42_Result_0xXX();
    break;
  }
}

/********************************************************************
* ���� : ICCard_Process_CMD_0x43
* ���� : IC���б����������:  0x43 ����������ȡIC��
* ˵��   cardType: ��Ƭ����  0x01: ��·����֤ 0x02:  ��ҵ��Ա�ʸ�֤
********************************************************************/
void ICCard_Process_CMD_0x43( u8 cardType )
{
  u8 TempData = cardType;
#if (ICCARD_JTD == ICCARD_SEL)
  {
    ICCard_PackAndSendData(IC_CARD_INITIATIVE_READ,&TempData,1);
  }
#else
  {
    ICCard_PackAndSendData(IC_CARD_INITIATIVE_READ,&TempData,0);
  }	
#endif
}
/********************************************************************
* ���� : ICCard_Process_CMD_Ack_0x43
* ���� : IC���б����������:  0x43 ����������ȡIC������Ӧ
* ˵��  
********************************************************************/
void ICCard_Process_CMD_Ack_0x43( void )
{
  ICCard_Terminal_Ack_Reader_Success( IC_CARD_INITIATIVE_READ);   
  // gICCardJTBCtrl.SM_Stage = 0;
  // gICCardJTBCtrl.SM_Counter = 0 ;
}

/********************************************************************
* ���� : ICCard_Process_CMD_0xA1_Result_0x01
* ���� : IC���б����������:  0xA1 IC����ȡ���֪ͨ
* ˵��:  IC�������������� ��ȡ���֪ͨ ״̬λ : IC������ʧ�� ,ԭ��Ϊ����������
********************************************************************/
void ICCard_Process_CMD_0xA1_Result_0x01( void )
{
  Public_ShowTextInfo( "A1����ʧ��ԭ��1:����������", 30 );              
}
/********************************************************************
* ���� : ICCard_Process_CMD_0xA1_Result_0x02
* ���� : IC���б����������:  0xA1 IC����ȡ���֪ͨ
* ˵��:  IC�������������� ��ȡ���֪ͨ ״̬λ : IC������ʧ�� ,ԭ��Ϊ��Ƭ���γ�
********************************************************************/
void ICCard_Process_CMD_0xA1_Result_0x02( void )
{
  Public_ShowTextInfo( "A1����ʧ��ԭ��3:��Ƭ���γ�", 30 );    
}
/********************************************************************
* ���� : ICCard_Process_CMD_0xA1_Result_0xXX
* ���� : IC���б����������:  0xA1 IC����ȡ���֪ͨ
* ˵��:  IC�������������� ��ȡ���֪ͨ ״̬λ ����: IC������ʧ�� ,ԭ��δ֪
********************************************************************/
void ICCard_Process_CMD_0xA1_Result_0xXX( void )
{
  Public_ShowTextInfo( "A1����ʧ��ԭ��X: δ֪", 30 );
}

/********************************************************************
* ���� : ICCard_Process_CMD_0xA1
* ���� : IC���б����������:  0xA1 ��ȡ(����)����֤����Ϣ
* ˵��  
********************************************************************/
void ICCard_Process_CMD_0xA1( void )
{
  u8 result_readcard = 0 ;  
  
  result_readcard = gICCardJTBCtrl.stt_exdev808.Data[0] ;
  switch( result_readcard )
  {
  case 0x00: //����ҵ��Ա�ʸ�֤��ȡ�ɹ�,�к�������
  case 0x10: //��·����֤�����ɹ�,�к�������
  case 0x20: //����ҵ��Ա�ʸ�֤��ȡ�ɹ�,�к�������
    gTerminalSendOrderCtrl.ackStatus = 1;
    //�����41����ʱһ����,�ݲ�����
    break;
  case 0x01: //IC������ʧ�� ,����������
  case 0x11: 
  case 0x21: 
    ICCard_Process_CMD_0xA1_Result_0x01();
    break;
  case 0x02: //IC������ʧ�� ,ԭ��Ϊ��Ƭ���γ�
  case 0x12:
  case 0x22:
    ICCard_Process_CMD_0xA1_Result_0x02();
    break;
  default: //IC������ʧ�� ,ԭ��δ֪
    ICCard_Process_CMD_0xA1_Result_0xXX();
    break;
  }
}

/********************************************************************
* ���� : ICCard_Process_CMD_0xA2
* ���� : IC���б����������:  0xA1 ���(����)����֤����Ϣ
* ˵��  
********************************************************************/
void ICCard_Process_CMD_0xA2( void )
{
  //ûɶҪ����
  //ICCard_SendCmdForClearElectronicCertificate(0xA2,0x00);//���������ģ�黺��,�Զ�����40ָ��
  gTerminalSendOrderCtrl.ackStatus = 1;
  //gICCardJTBCtrl.SM_Stage = 5;
  //gICCardJTBCtrl.SM_Counter = 0 ;
}
/********************************************************************
* ���� : ICCard_Send_CMD_To_Terminal_0xA4
* ���� : IC���б����������:  0xA4 ����GPSʱ��
* ˵��   GPS->ICU
********************************************************************/
void ICCard_Send_CMD_To_Card_Reader_0xA4( void )
{	
  u8 TempData[7];
  
  TempData[0] = 20;
  memcpy(TempData+1,&CurTime.year,6);	
  ICCard_PackAndSendData(IC_CARD_SET_TIME,TempData,7);
}
/********************************************************************
* ���� : ICCard_Process_CMD_0xA4
* ���� : IC���б����������:  0xA4 
* ˵��   �ӻ���ȡGPSʱ��    ICU->GPS
********************************************************************/
void ICCard_Process_CMD_0xA4( void )
{
  gICCardJTBCtrl.SM_Stage = 6;
}

/********************************************************************
* ���� : ICCard_Process_CMD_0xAA
* ���� : IC���б����������:  0xAA �ӻ�Ӧ��??
* ˵��   �ӻ���Դ����Ӧ��,�ӻ��Լ�Ӧ��
********************************************************************/
void ICCard_Process_CMD_0xAA( void )
{
  u8 result_readcard = 0 ;	
  
  result_readcard = gICCardJTBCtrl.stt_exdev808.Data[0] ;
  if(0x01 == result_readcard)
  {
    //�����ɹ�
    //cardLaunchCtrl.cardLaunchStep = IC_CARD_CHECK_START;
    //ICCard_Terminal_Ack_Reader_Success( IC_CARD_READ);
    
  }
  else
    if(0x02 == result_readcard)
    {
      //����ʧ��
      
    }
    else
    {
      
    }
  
}

/********************************************************************
* ���� : SM_Stage_Process_0
* ���� : IC����֤״̬������ �׶�0
* ˵��  ������=>�ն� IC��������֤
********************************************************************/
void ICCard_SM_Stage_Process_0( void )
{
  unsigned char ACC;    
  ACC=Io_ReadStatusBit(STATUS_BIT_ACC);
  if(ACC)
  {
    if(59*SECOND == gICCardJTBCtrl.LinkInquireCount++%(60*SECOND))
    {
      //�����Է�����·̽Ѱָ��
      ICCard_Terminal_Ack_Reader_Success( LINK_INQUIRE );
    }
    if(gICCardJTBCtrl.LinkInquireCount > 180*SECOND)
    {
      gICCardJTBCtrl.LinkInquireCount = 0;
      Io_WriteAlarmBit(ALARM_BIT_IC_ERROR,SET);
    }
  }
  if(gICCardJTBCtrl.Com_Buffer_Length < 1)return; 
  //////////////////////////
  switch(gICCardJTBCtrl.stt_exdev808.Cmd)
  {
    case	IC_CARD_AUTHENTICATION://IC����֤����
      ICCard_Process_CMD_0x40();
      break;
      case	IC_CARD_READ://IC����֤�ɹ���Ͳ�������������
        gICCardJTBCtrl.SM_Stage = 4;
        gICCardJTBCtrl.SM_Counter = 0 ;
        return;
        case	IC_CARD_PULL://IC���γ�֪ͨ
          ICCard_Process_CMD_0x42();
          break;
          case	IC_CARD_INITIATIVE_READ://����������IC����Ӧ
            ICCard_Process_CMD_Ack_0x43();
            break;
            case	POWER_ON_INDICATE://�ӻ��ϵ�ָʾӦ��
              ICCard_Send_CMD_To_Card_Reader_0x01();//�ϵ�ָʾ	
              break;
              case	LINK_INQUIRE://������·̽Ѱ
		gICCardJTBCtrl.LinkInquireCount = 0;
		//ICCard_Send_CMD_To_Card_Reader_0x02();//��·̽ѯ
		break;
		case	SLAVE_VERSION_QUERY://�ӻ��汾��Ϣ��ѯ
                  ICCard_Process_CMD_0x04();
                  break;
                  //case	SLAVE_FIRMWARE_UPDATE://�ӻ��̼�����
                  //	ICCard_Process_CMD_0x06();
                  //	break;
                  case	IC_CARD_READ_ATTRIBUTE://����ģ���ۺ�������Ϣ
                    ICCard_Process_CMD_0x07();
                    break;
                    case	IC_CARD_READ_E_CERT://��ȡ(����)����֤����Ϣ
                      ICCard_Process_CMD_0xA1();
                      break;
                      case	IC_CARD_CLR_E_CERT://���(����)����֤����Ϣ
                        ICCard_Process_CMD_0xA2();
                        break;
                        case	IC_CARD_SET_TIME:
                          ICCard_Send_CMD_To_Card_Reader_0xA4();
                          break;
                          case	IC_CARD_SPECIAL_ACK://�ӻ���Դ����Ӧ�� //�ӻ��Լ�Ӧ��
                            ICCard_Process_CMD_0xAA();
                            break;
  default:break;
  }
  ICCard_Clear_Buf_COM( 1);
  
}

/********************************************************************
* ���� : SM_Stage_Process_1
* ���� : IC����֤״̬������ �׶�1
* ˵��   �ն�=>��֤����
********************************************************************/
void ICCard_SM_Stage_Process_1( void )
{
  
  if(CHANNEL_DATA_3 != (CHANNEL_DATA_3&TerminalAuthorizationFlag))
  {
    if( gICCardJTBCtrl.SM_Counter++ > TIMEOUT_REQUEST_LINK )
    {
      gICCardJTBCtrl.SM_Stage = 0;
      gICCardJTBCtrl.SM_Counter = 0 ;
      if(gICCardJTBCtrl.activeCardTimes++ > 3)
      {
        gICCardJTBCtrl.activeCardTimes = 0;
        //ICCard_SendCmdForClearElectronicCertificate(0xA2,0x00);//���������ģ�黺��,�Զ�����40ָ��
        ICCard_Clear_Buf_COM( 1);
				CertService_Close();//��֤�쳣�͹ر�����
      }
    }
    //��֤�ȴ��������в�ο�����
    if((IC_CARD_READ == gICCardJTBCtrl.stt_exdev808.Cmd)||(IC_CARD_PULL == gICCardJTBCtrl.stt_exdev808.Cmd))
    {
      gICCardJTBCtrl.SM_Stage = 0;
      gICCardJTBCtrl.SM_Counter = 0 ;
      ICCard_Clear_Buf_COM( 1);
			CertService_Close();//��֤�쳣�͹ر�����
    }
    return;//�ȴ���֤���������ӳɹ�
  }
  //��͸���Ϸ�(0x0900)ָ������Ƭ������Ϣ����֤��Ϣ
  if(ACK_OK == RadioProtocol_OriginalDataUpTrans(CHANNEL_DATA_3, 0x0B, gICCardJTBCtrl.AuthRequestCode, LEN_AUTH_REQUEST_CODE))
  {
    gICCardJTBCtrl.SM_Stage = 2;
    gICCardJTBCtrl.SM_Counter = 0 ;
    ICCard_WaitOrClrPlatAuthAnswer();//�ȴ�ƽ̨Ӧ��
  }
  else
  {
    Public_ShowTextInfo( "IC����֤������δ����", 30 );
    gICCardJTBCtrl.SM_Stage = 0;
    gICCardJTBCtrl.SM_Counter = 0 ;
    if(gICCardJTBCtrl.activeCardTimes++ > 3)
    {
      gICCardJTBCtrl.activeCardTimes = 0;
      //ICCard_SendCmdForClearElectronicCertificate(0xA2,0x00);//���������ģ�黺��,�Զ�����40ָ��
    }
  }
}


/********************************************************************
* ���� : SM_Stage_Process_2
* ���� : IC����֤״̬������ �׶�2
* ˵��  ��֤����=>�ն�
********************************************************************/
void ICCard_SM_Stage_Process_2( void )
{	
  if( gICCardJTBCtrl.SM_Counter++ > TIMEOUT_REQUEST_AUTH )
  {
    //��׼35��
    gICCardJTBCtrl.SM_Stage = 0;
    gICCardJTBCtrl.SM_Counter = 0 ;
    if(gICCardJTBCtrl.activeCardTimes++ > 3)
    {
      gICCardJTBCtrl.activeCardTimes = 0;
			CertService_Close();//��֤�쳣�͹ر�����
      //ICCard_SendCmdForClearElectronicCertificate(0xA2,0x00);//���������ģ�黺��,�Զ�����40ָ��
    }
    return ;
  }
	//��֤�ȴ��������в�ο�����
  if((IC_CARD_READ == gICCardJTBCtrl.stt_exdev808.Cmd)||(IC_CARD_PULL == gICCardJTBCtrl.stt_exdev808.Cmd))
  {
    gICCardJTBCtrl.SM_Stage = 0;
    gICCardJTBCtrl.SM_Counter = 0 ;
    ICCard_Clear_Buf_COM( 1);
		CertService_Close();//��֤�쳣�͹ر�����
  }
  if(0 == ICCard_GetPlatAuthAnswer())return;//ƽ̨��δ��Ӧ
  CertService_Close();//��֤�ɹ���Ͽ���֤���ķ���������
  ICCard_WaitOrClrPlatAuthAnswer();//ƽ̨��Ӧ�����Ӧ��
  gICCardJTBCtrl.SM_Stage = 3;
  gICCardJTBCtrl.SM_Counter = 0 ;
}

/********************************************************************
* ���� : SM_Stage_Process_3
* ���� : IC����֤״̬������ �׶�3
* ˵��   �ն�=>������
********************************************************************/
void ICCard_SM_Stage_Process_3( void )
{ 
  u8 TempBuf[LEN_AUTH_ACK_CODE];
  
  memcpy(&TempBuf[0],gICCardJTBCtrl.AuthCode,LEN_AUTH_ACK_CODE ); 
  
  if(0x1 == TempBuf[0])
  {
    //�������ģ�鷵��1��25�ֽ���Ϣ
    Public_PlayTTSVoiceStr("IC����֤����ɹ�");
    Public_ShowTextInfo( "IC����֤����ɹ�", 30 );
    TempBuf[0] = 0;
    ICCard_Clear_Buf_COM(1);
    ICCard_Process_CMD_0x40_Request_COM_Ack_Result(TempBuf,LEN_AUTH_ACK_CODE );
    gICCardJTBCtrl.SM_Stage = 4;
    gICCardJTBCtrl.SM_Counter = 0 ; 	
  }
  else
  {
    Public_ShowTextInfo( "IC����֤����ʧ��", 30 );
    ICCard_Process_CMD_0x40_Request_COM_Ack_Result(TempBuf,1 );
    gICCardJTBCtrl.SM_Stage = 0;
    gICCardJTBCtrl.SM_Counter = 0 ; 
  }
}

/********************************************************************
* ���� : SM_Stage_Process_4
* ���� : IC����֤״̬������ �׶�4
* ˵��  ������=>�ն�
********************************************************************/
void ICCard_SM_Stage_Process_4( void )
{
  if( gICCardJTBCtrl.SM_Counter++ > (3*SECOND)  )
  {
    gICCardJTBCtrl.SM_Stage = 0;
    gICCardJTBCtrl.SM_Counter = 0 ;
    return;
  }
  ///////////////
  if( gICCardJTBCtrl.Com_Buffer_Length < 1)return; 
  /***** IC�������� ��������� *****/
  if( 0x41 == gICCardJTBCtrl.stt_exdev808.Cmd)
  {        
    ICCard_Process_CMD_0x41();
    ICCard_Clear_Buf_COM(1);
  }
}

/********************************************************************
* ���� : SM_Stage_Process_5
* ���� : IC����֤״̬������ �׶�5
* ˵��   �����ն��������ֱ�ӵ�һЩ�м�״̬
********************************************************************/
void ICCard_SM_Stage_Process_5( void )
{
  if( gICCardJTBCtrl.SM_Counter++ > SECOND  )
  {
    gICCardJTBCtrl.SM_Stage = 0;
    gICCardJTBCtrl.SM_Counter = 0 ;
    return;
  }
  
  if(gICCardJTBCtrl.Com_Buffer_Length < 1)return; 
  //////////////////////////
  
  switch(gICCardJTBCtrl.stt_exdev808.Cmd)
  {
    case	POWER_ON_INDICATE://�ӻ��ϵ�ָʾӦ��
      case	LINK_INQUIRE://������·̽Ѱ
	case	SLAVE_VERSION_QUERY://�ӻ��汾��Ϣ��ѯ
          case	IC_CARD_SET_TIME:
            ICCard_Send_CMD_To_Card_Reader_0x03(0);//�˳�ʡ��ģʽ
            gICCardJTBCtrl.SM_Stage = 0;
            gICCardJTBCtrl.SM_Counter = 0 ;
            break;
            case	IC_CARD_READ://IC����֤�ɹ���Ͳ�������������
              gICCardJTBCtrl.SM_Stage = 4;
              gICCardJTBCtrl.SM_Counter = 0 ;
              return;
              case	SLAVE_POWER_CONTROL://�ӻ���Դ����
                ICCard_Send_CMD_To_Card_Reader_0x03(0);//�˳�ʡ��ģʽ
                gICCardJTBCtrl.SM_Stage = 0;
                gICCardJTBCtrl.SM_Counter = 0 ;
                break;
                case	SLAVE_SELF_INSPECTION://�ӻ��Լ�
                  ICCard_Send_CMD_To_Card_Reader_0x05(0x0b);
                  gICCardJTBCtrl.SM_Stage = 0;
                  gICCardJTBCtrl.SM_Counter = 0 ;
                  break;
  default:
    break;
  }
  ICCard_Clear_Buf_COM( 1 );
}

/********************************************************************
* ���� : ICCard_SM_Stage_Process
* ���� : IC����֤״̬������
* ˵��  
********************************************************************/
void ICCard_SM_Stage_Process( void )
{  
  switch( gICCardJTBCtrl.SM_Stage )
  {
  case 0:
    ICCard_SM_Stage_Process_0();
    break;		
  case 1:
    ICCard_SM_Stage_Process_1();
    break;		
  case 2:
    ICCard_SM_Stage_Process_2();
    break;
  case 3:
    ICCard_SM_Stage_Process_3();    
    break;
  case 4:    
    ICCard_SM_Stage_Process_4();    
    break;
  case 5:
    ICCard_SM_Stage_Process_5();    
    break;
  default:
    gICCardJTBCtrl.SM_Stage = 0;    
    break;
  }  
}
/********************************************************************
* ���� : Task_IC_Card_JTB_Init
* ���� : ��ͨ��IC�������ʼ��
* ˵��  
********************************************************************/
void Task_IC_Card_JTB_Init( void )
{
  if( !gICCardJTBCtrl.Flag_Init )
  {
    SetTimerTask(TIME_ICAUC,ICAUC_TIME_TICK);
    /* ���������ṹ��STT_ICCARD_CTRL_JTB���� , Ȼ����λ1�� ������ʼ����� */
    memset( ( u8 *)&gICCardJTBCtrl, 0 , sizeof( STT_ICCARD_CTRL_JTB ) );
    gICCardJTBCtrl.Flag_Init = TRUE;  
    /* ע��IC����֤����GPRS���ӵ� ���ջص����� */
    ICAUC_RegReadFun( ICCard_Net_Rx );    
    ///* �ٴ�����IC����������� */      
#if (ICCARD_JTD == ICCARD_SEL)
    ICCARD_ReadCertSevicePara();
#endif
  } 
}

/********************************************************************
* ���� : Task_IC_Card_JTB
* ���� : ��ͨ��IC������
* ˵��  
********************************************************************/
FunctionalState Task_IC_Card_JTB(void)
{
  Task_IC_Card_JTB_Init();
  ////////////////////
  ICCard_Net_Process();
  ///////////////
  ICCard_SM_Stage_Process();
  ICCard_Order_Process();
  /////////////
  return ENABLE;  
}
/******************************************************************************
**                            End Of File
******************************************************************************/
/********************************************************************
* ���� : ICCard_SearchTerminalOrder
* ���� : ��ѯָ�
* ˵��	
********************************************************************/
u8 ICCard_SearchTerminalOrder(u8 cmdType)
{
  
  u8 i;
  
  for(i=0;i<sizeof(gTerminalSendOrderTab)/sizeof(TERMINAL_SEND_ORDER);i++)
  {
    if(cmdType == gTerminalSendOrderTab[i].cmdType)
    {
      break;
    }
  }
  if(i==sizeof(gTerminalSendOrderTab)/sizeof(TERMINAL_SEND_ORDER))
  {
    i = 0xff;
  }
  return i;
}
/********************************************************************
* ���� : ICCard_CustomProtocolParse
* ���� : ���ݶ��������Զ�����Ϣ�����
* ˵��  
********************************************************************/
ProtocolACK ICCard_CustomProtocolParse(u8 *pBuffer,u16 BufferLen)
{
  u16 subID;//���ݶ�������ϢID
  ProtocolACK ack = ACK_OK;
  
  subID 	= *(pBuffer+1)<<8;
  subID  |= *(pBuffer+2);
  
  if(0xc108 == subID)
  {
    gTerminalSendOrderCtrl.cmdType = IC_CARD_READ_ATTRIBUTE;
    gTerminalSendOrderCtrl.dataType = 0x00;//ûӰ��
  }
  else if(0xc702 == subID)
  {
    gTerminalSendOrderCtrl.cmdType = IC_CARD_CLR_E_CERT;
    gTerminalSendOrderCtrl.dataType = *(pBuffer+3);
  }
  gTerminalSendOrderCtrl.sendStatus = 1;
  gTerminalSendOrderCtrl.ackStatus = 0;
  
  return ack;
}

/********************************************************************
* ���� : ICCard_Order_Process
* ���� : ���ݶ�����չЭ�����
* ˵��  
********************************************************************/
void ICCARD_CustomOrderInit(void)
{
  memset((void*)&gTerminalSendOrderCtrl,0,sizeof(TERMINAL_SEND_CTRL));
}
#if 0
void ICCard_Launch_Init(void)
{
  memset((void*)&cardLaunchCtrl.cardLaunchStep,0,sizeof(ICCARD_LAUNCH_CTRL));
  cardLaunchCtrl.cardLaunchStep = IC_CARD_CHECK_START;
}
//��ʼ�����������е�һЩ����,�����ϵ�ָʾ,��·̽ѯ,�汾��ѯ,��Դ����,�ӻ��Լ��
void ICCard_Launch_Process(void)
{
  switch(cardLaunchCtrl.cardLaunchStep)
  {
    case	IC_CARD_CHECK_START:
      cardLaunchCtrl.delayCount = 0;//����һ�ֿ���״̬
      return;
      case	IC_CARD_CHECK_END:
        if(cardLaunchCtrl.delayCount++ > SECOND)//��ʱ1s
        {
          cardLaunchCtrl.delayCount = 0;
          //ICCard_SendCmdForClearElectronicCertificate(0xA2,0x00);//���������ģ�黺��
          cardLaunchCtrl.cardLaunchStep = IC_CARD_CHECK_START;
        }return;
  case  POWER_ON_INDICATE:
    if(cardLaunchCtrl.delayCount++ > SECOND)//��ʱ1s
    {
      cardLaunchCtrl.delayCount = 0;
      ICCard_Send_CMD_To_Card_Reader_0x01();//�ϵ�ָʾ	
      cardLaunchCtrl.lastCardLaunchStep = cardLaunchCtrl.cardLaunchStep;
      cardLaunchCtrl.cardLaunchStep = 0xff;
    }break;
  case  LINK_INQUIRE:
    if(cardLaunchCtrl.delayCount++ > SECOND)//��ʱ1s
    {
      cardLaunchCtrl.delayCount = 0;
      ICCard_Send_CMD_To_Card_Reader_0x02();//��·̽ѯ
      cardLaunchCtrl.lastCardLaunchStep = cardLaunchCtrl.cardLaunchStep;
      cardLaunchCtrl.cardLaunchStep = 0xff;
    }break;
  case  SLAVE_POWER_CONTROL:
    if(cardLaunchCtrl.delayCount++ > SECOND)//��ʱ1s
    {
      ICCard_Send_CMD_To_Card_Reader_0x03(0);//�˳�ʡ��ģʽ
      cardLaunchCtrl.lastCardLaunchStep = cardLaunchCtrl.cardLaunchStep;
      cardLaunchCtrl.cardLaunchStep = 0xff;
    }break;
  case  SLAVE_VERSION_QUERY:
    ICCard_Send_CMD_To_Card_Reader_0x04();
    cardLaunchCtrl.lastCardLaunchStep = cardLaunchCtrl.cardLaunchStep;
    cardLaunchCtrl.cardLaunchStep = 0xff;
    break;
  case  SLAVE_SELF_INSPECTION:
    ICCard_Send_CMD_To_Card_Reader_0x05(0x0b);
    cardLaunchCtrl.lastCardLaunchStep = cardLaunchCtrl.cardLaunchStep;
    cardLaunchCtrl.cardLaunchStep = 0xff;
    break;
  default:
    if(cardLaunchCtrl.delayCount++ > 10*SECOND)//��һ��
    {
      cardLaunchCtrl.delayCount = 0;
      cardLaunchCtrl.cardLaunchStep = cardLaunchCtrl.lastCardLaunchStep;
    }break;
  }
}
#endif
void ICCard_Order_Process(void)
{
  u8 i;
  static u8 initFlg = 0;
  
  if(0==initFlg)
  {
    initFlg = 1;
    ICCARD_CustomOrderInit();
    //ICCard_Launch_Init();
  }
  //ICCard_Launch_Process();
  i = ICCard_SearchTerminalOrder(gTerminalSendOrderCtrl.cmdType);
  if(0xff==i)
  {
    return;
  }
  switch(gTerminalSendOrderCtrl.step)
  {
    case	0:
      {
        //�������ģ�鷢��ָ��
        gTerminalSendOrderTab[i].operate(gTerminalSendOrderCtrl.cmdType,gTerminalSendOrderCtrl.dataType);
        gTerminalSendOrderCtrl.step = 1;
        gTerminalSendOrderCtrl.sendStatus = 1;
      }break;
      case	1:
        {
          if(gTerminalSendOrderCtrl.counter++ > SECOND)//��ʱ�ȴ�ʱ��Ϊ1s
          {
            if(gTerminalSendOrderCtrl.retryTimes++ > 3)//�ط�3��
            {
              gTerminalSendOrderCtrl.retryTimes = 0;
              gTerminalSendOrderCtrl.step = 3;
            }
            else
            {
              gTerminalSendOrderCtrl.step = 0;
              gTerminalSendOrderCtrl.counter = 0;
            }
          }
          else if(gTerminalSendOrderCtrl.ackStatus)
          {
            gTerminalSendOrderCtrl.step = 2;
            gTerminalSendOrderCtrl.counter = 0;
          }
        }break;
        case	2:
          {
            //��ƽ̨�ϱ�����
            gTerminalSendOrderTab[i].response();
            gTerminalSendOrderCtrl.step = 3;
          }break;
          case	3:
  default:
    {
      memset((void*)&gTerminalSendOrderCtrl, 
             0,
             sizeof(TERMINAL_SEND_CTRL)-sizeof(STT_ICCARD_MODULE_ATTRIBUTE)-8);
    }break;
  }
  
}
//�����ϱ� A1 �յ�ƽ̨8702����
void ICCard_CustomProtocolParse_Rev_0x8702(u8 *pBuffer,u16 BufferLen)
{
  if(BufferLen<9)return;
  
  gTerminalSendOrderCtrl.cmdType = IC_CARD_READ_E_CERT;
  gTerminalSendOrderCtrl.dataType = *pBuffer;
  memcpy(&gTerminalSendOrderCtrl.E_Signature,pBuffer+1,8);	
  
  gTerminalSendOrderCtrl.sendStatus = 1;
  gTerminalSendOrderCtrl.ackStatus = 0;	
}
//�����ϱ� A1 ��Ӧ8702����  0702
void ICCard_CustomProtocolParse_Ack_0x0702(void)
{
  ICCard_DisposeJTBCardDriverSignPassivity(&gICCardJTBCtrl.stt_exdev808.Data[1],gICCardJTBCtrl.stt_exdev808_len);
  ICCard_Terminal_Ack_Reader_Success( IC_CARD_READ_E_CERT);
}
//�ն˷���ָ�
//���ݵ���ǩ��
void ICCard_Transmit_E_Signature(u8 *pBuffer)
{
  memcpy(pBuffer,gTerminalSendOrderCtrl.E_Signature,8);//��Ƭ�����
}

/********************************************************************
* ���� : ICCard_SendCmdForReadModuleAttribute
* ���� : ����ģ���ۺ�����
* ˵��   �ն�=>�� 0x07
********************************************************************/
unsigned char ICCard_SendCmdForReadModuleAttribute(u8	cmdType,u8 dataType)
{  
  u8 TempData[2];
  dataType = dataType;//��������
  
  return ICCard_PackAndSendData(cmdType,TempData,0);
}
/********************************************************************
* ���� : ICCard_SendCmdForReadElectronicCertificate
* ���� : ��ȡ���õ���֤����Ϣ
* ���� : dataType 0x00:��ҵ��Ա�ʸ�֤(��)  0x10:��·����֤  0x20:��·����֤(��)
* ˵��   �ն�=>�� 0xA1
********************************************************************/
unsigned char ICCard_SendCmdForReadElectronicCertificate(u8	cmdType,u8 dataType)
{  
  u8 TempData[9];
  
  TempData[0] = dataType;
  ICCard_Transmit_E_Signature(TempData+1);//��Ƭ�����
  
  return ICCard_PackAndSendData(cmdType,TempData,9);
}

/********************************************************************
* ���� : ICCard_SendCmdForClearElectronicCertificate
* ���� : ������õ���֤����Ϣ
* ���� : dataType 0x00:��ҵ��Ա�ʸ�֤(��)  0x10:��·����֤  0x20:��·����֤(��)
* ˵��   �ն�=>�� 0xA2
********************************************************************/
unsigned char ICCard_SendCmdForClearElectronicCertificate(u8	cmdType,u8 dataType)
{  
  u8 TempData;
  
  TempData = dataType;
  
  return ICCard_PackAndSendData(cmdType,&TempData,1);
}

extern u8 s_ucMsgFlashBuffer[];
/*********************************************************************
//��������	:ICCARD_Report_0x0900_0x3108
//����		:��������͸��
//����		:pBuffer:ָ�����ݣ�͸�����ݣ���ָ��;BufferLen:��ǰ͸�����ݰ�����
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:
//���ú���	:
//�ж���Դ	:
//����		:    �ɹ�,����ACK_OK;ʧ�ܷ���ACK_ERROR;
//��ע		:���ݶ����Զ�����Ϣ����0900/3108
*********************************************************************/
ProtocolACK ICCARD_Report_0x0900_0x3108(u8 channel, u8 Type, u8 *pBuffer, u16 BufferLen)
{
  unsigned char *buffer;
  u8 SmsPhone[20];
  u16 length;
  MESSAGE_HEAD head;
  if(BufferLen >= 1023)
  {
    return ACK_ERROR;
  }
  buffer =s_ucMsgFlashBuffer;//lzm,2014.9.24
  //ָ���׵�ַ  
  buffer[0] = Type;
  buffer[1] = 0x31;//���  ���ֽ���ǰ
  buffer[2] = 0x08;
  memcpy(buffer+3,pBuffer,BufferLen);
  length = BufferLen+3;
  //��ϢID
  head.MessageID = 0x0900;
  //��Ϣ����
  head.Attribute = (length & 0x3ff);
  
  if(0 != RadioProtocol_SendCmd(channel,head, buffer, length, SmsPhone))
  {
    return ACK_OK;
  }
  else
  {
    return ACK_ERROR;
  }
}

/********************************************************************
* ���� : ICCARD_Report_CardAttributeInfoAck
* ���� : �ն˶���ģ������Ӧ�� 0900/3108
* ˵��   ���ݶ����Զ�����Ϣ��
********************************************************************/
void ICCARD_Report_TerminalReadCardAttributeInfoAck(void)
{
  u8 channel = CHANNEL_DATA_3;
  
  ICCARD_Report_0x0900_0x3108(channel, \
    0xF2, \
      gTerminalSendOrderCtrl.cardAttribute.peripheralManufacturerNo, \
        sizeof(STT_ICCARD_MODULE_ATTRIBUTE));
}

/*********************************************************************
//��������	:ICCARD_Report_0x0900_0x3702
//����		:��������͸��
//����		:pBuffer:ָ�����ݣ�͸�����ݣ���ָ��;BufferLen:��ǰ͸�����ݰ�����
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:
//���ú���	:
//�ж���Դ	:
//����		:    �ɹ�,����ACK_OK;ʧ�ܷ���ACK_ERROR;
//��ע		:���ݶ����Զ�����Ϣ����0900/3702
*********************************************************************/
ProtocolACK ICCARD_Report_0x0900_0x3702(u8 channel, u8 Type, u8 *pBuffer, u16 BufferLen)
{
  unsigned char *buffer;
  u8 SmsPhone[20];
  u16 length;
  MESSAGE_HEAD head;
  if(BufferLen >= 1023)
  {
    return ACK_ERROR;
  }
  buffer =s_ucMsgFlashBuffer;//lzm,2014.9.24
  //ָ���׵�ַ  
  buffer[0] = Type;
  buffer[1] = 0x37;//���  ���ֽ���ǰ
  buffer[2] = 0x02;
  memcpy(buffer+3,pBuffer,BufferLen);
  length = BufferLen+3;
  //��ϢID
  head.MessageID = 0x0900;
  //��Ϣ����
  head.Attribute = (length & 0x3ff);
  
  if(0 != RadioProtocol_SendCmd(channel,head, buffer, length, SmsPhone))
  {
    return ACK_OK;
  }
  else
  {
    return ACK_ERROR;
  }
}

/********************************************************************
* ���� : ICCARD_Report_ClrCardInbuiltInfoAck
* ���� : �ն˶���ģ�����������ϢӦ�� 0900/3108
* ˵��   ���ݶ����Զ�����Ϣ��
********************************************************************/
void ICCARD_Report_TerminalReadCardClrInbuiltInfoAck(void)
{
  u8 channel = CHANNEL_DATA_3;
  u8 temp[2];
  
  temp[0] = gTerminalSendOrderCtrl.dataType;
  temp[1] = 0;//��Ϊû�з��ؽ��,������Ϊ�ɹ���
  ICCARD_Report_0x0900_0x3702(channel, \
    0xF2, \
      temp, \
        2);
  
}
// ʹ�ö������˿�ʱ����ȡ��֤����������
void ICCARD_ReadCertSevicePara(void)
{
  u8 buffer[20];
  u8 bufferLen;
  
  bufferLen= EepromPram_ReadPram(E2_MAIN_SERVER_APN_ID, buffer);
  if(0 == bufferLen){
    EepromPram_WritePram(E2_MAIN_SERVER_APN_ID, "gzjtxx03.gd",strlen("gzjtxx03.gd"));
  }
  bufferLen= EepromPram_ReadPram(E2_IC_MAIN_SERVER_IP_ID, buffer);
  if(0 == bufferLen){
    EepromPram_WritePram(E2_IC_MAIN_SERVER_IP_ID, "192.168.201.95",strlen("192.168.201.95"));
  }
  bufferLen= EepromPram_ReadPram(E2_IC_MAIN_SERVER_TCP_PORT_ID, buffer);
  if(E2_IC_MAIN_SERVER_TCP_PORT_LEN != bufferLen){
    Public_ConvertLongToBuffer(11000,buffer);
    EepromPram_WritePram(E2_IC_MAIN_SERVER_TCP_PORT_ID,buffer ,E2_IC_MAIN_SERVER_TCP_PORT_LEN);
  }
}

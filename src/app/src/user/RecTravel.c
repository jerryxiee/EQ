/*******************************************************************************
*��Ȩ˵��	:Shenzhen E-EYE Co. Ltd., Copyright 2009- All Rights Reserved.
*�ļ�����	:RecTravel.c		
*����		:��¼�����ݲɼ�
*�汾��	:
*������	:       :myh
*����ʱ��	:2013.3
*�޸���	:
*�޸�ʱ��	:
*�޸ļ�Ҫ˵��	:
*��ע		:
*******************************************************************************/
#include "include.h"
#include "RecTravel.h"

/******************���Կ���****************************************************/
#define   REDEBUG_USB             0  /*USB����,��Ҫ��U��д�����Э���ģ������*/
#define   REUSB_TO_FLASH          0 /*ͨ��USB�ļ���FLASH��д������           */ 
#define   PRO_DEF_RETAINS        (0x00) /*��¼�Ǳ����ֽ�,Ĭ��0x00*/

/******************���ر���****************************************************/
u8 const h_BufGb2312[100]=
{"��������������������ǭ���ƵἪ����ԥ��¤������������ɸӹ�³�����¸۰�̨���պ����������Ϲ�ɾ�"};
u16 const h_BufUnicode[50]=
{
  0x4EAC,0x6E58,0x6D25,0x9102,0x6CAA,0x7CA4,0x6E1D,0x743C,0x5180,0x5DDD,0x664B,0x8D35,0x9ED4,0x8FBD,0x4E91,
  0x6EC7,0x5409,0x9655,0x79E6,0x8C6B,0x7518,0x9647,0x82CF,0x9752,0x6D59,0x7696,0x85CF,0x95FD,0x8499,0x8D63,
  0x6842,0x9C81,0x5B81,0x9ED1,0x65B0,0x6E2F,0x6FB3,0x53F0,0x519B,0x7A7A,0x6D77,0x5317,0x6C88,0x5170,0x6D4E,
  0x5357,0x5E7F,0x6210,0x8B66
};
u8 const Recorder_Cmd_Num[28]=
{
0x0,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x10,0x11,0x12,0x13,0x14,0x15,//16��
0x82,0x83,0x84,0xc2,0xc3,0xc4,//6��
0xe0,0xe1,0xe2,0xe3,0xe4,0xe5 //6��
};
u16 const Recorder_Cmd_OneNum[8]=//һ�����ݿ����õ��ֽ���
{
  126,  //�ٶ� 126 
  666,  //λ�� 666 
  234,  //�ɵ� 234 
  50,   //��ʱ 50  
  25,   //��� 25 
  7,    //���� 7  
  7,    //���� 7  
  133   //��״ 133
};
u8 const Recorder_Cmd_PackNum[8]=//GPRS�ְ�����
{
  5,  //�ٶ� 126 * 5
  1,  //λ�� 666 * 1
  3,  //�ɵ� 234 * 3
  13, //��ʱ 50  * 13
  25, //��� 25  * 25
  100,//���� 7   * 100
  100,//���� 7   * 100
  5   //��״ 133 * 5
};
u16 const Recorder_Cmd_PackSize[8]=//GPRS�ְ���С
{
  630, //�ٶ� 126 * 5
  666, //λ�� 666 * 1
  702, //�ɵ� 234 * 3
  650, //��ʱ 50  * 13
  625, //��� 25  * 25
  700, //���� 7   * 100
  700, //���� 7   * 100
  665  //��״ 133 * 5
};
u8 const Vdr_Cmd07_Name[16][18]=
{
    "ִ�б�׼�汾���  ",            
    "��ǰ��ʻ����Ϣ    ",
    "ʵʱʱ��          ",
    "�ۼ���ʻ���      ",
    "����ϵ��          ",
    "������Ϣ          ",
    "״̬�ź�������Ϣ  ",
    "��¼��Ψһ�Ա��  ",
    "��ʻ�ٶȼ�¼      ",
    "λ����Ϣ��¼      ",
    "�¹��ɵ��¼      ",
    "��ʱ��ʻ��¼      ",
    "��ʻ����ݼ�¼    ",
    "�ⲿ�����¼      ",
    "�����޸ļ�¼      ",
    "�ٶ�״̬��־      "    
};

  
/******************�ⲿ����****************************************************/
extern u8	RecorderProtocolFormatFlag;
extern const u8 EepromPramLength[];
extern  u16     DelayAckCommand;      /*��ʱӦ������                          */
extern FIL file;
extern REGISTER_STRUCT Register[REGISTER_TYPE_MAX];

extern u16  SubpackageCurrentSerial;//�ְ�������ǰ����ˮ��
extern u16  SubpackageResendFirstSerial;//�ְ��ش���1������ˮ��
extern u8  SubpacketUploadFlag;//�ְ��ϴ���־,1Ϊ�����ϴ�,0Ϊ�����ϴ�


const u8 GB_T2012_Ver[2] ={0x12,0x00};/*��¼��ִ�б�׼�汾                    */ 

RECORDER_QUEUE_RX  St_RecorderQueueRx; 
RECORDER_QUEUE_TX  St_RecorderQueueTx;

ST_RECORDER_WIRE    St_RecorderWire;
ST_RECORDER_HEAD    RecorderHead;//��¼��Э��ͷ�ṹ��
ST_RECORDER_FLASH   RecorderFlash;//��¼�Ǵ�flash�ж�ȡ���ݽṹ��
RECORDER_FLASH_ADDR RecorderFlashAddr;//��¼��������flash�д洢�ĵ�ַ

//static u16  PackAll;//ָ��ʱ����������ܰ���
//static u16  PackCnt;//ָ��ʱ����������ܰ���������
//static u16  OnePackCnt;//һ�����ݰ���OnePackCnt�����ݿ��������Ҫ�����ٶȺ�λ�÷��
static u16  DataBlockWatch;//ָ��ʱ������������ݿ��������ڵ��Թ۲죬��ʵ������
//static u16  DataBlockSize;//һ�����ݿ��Ӧ�����ݳ���

static u16  DataBlockAll;//ָ��ʱ����ڵ��ܵ����ݿ���
static u16  DataBlockSpeedPos;//ָ��ʱ�����1�����ٶ����ݻ�1Сʱλ����Ϣ����


static u8 WireSendVer;



static RECORDER_FILE_VDR_NAME vdr_fname;//VDR�ļ���
static RECORDER_FILE_VDR_DATA vdr_fdata;//VDR�ļ�����
static u8  vdr_ver;//VDR�ļ�У��
static u16 vdr_Written;//VDR�ļ�ϵͳ���泤��

u32	   SearchTimeCnt = 0;//32λ����ʱ��
TIME_T SearchTime;//ʱ��ṹ������ʱ��
TIME_T SpeedPosTime;//����λ����Ϣ���ٶ��ж��Ƿ��֡���Сʱ


static u8 SerOutTmrCnt;//���ڳ�ʱ������
static u8 SerBusyFlag;//����æ��־  1Ϊæ 0Ϊ����


extern u16* ff_NameConver(u8 *pStr);

/**
* @brief  �������У��
* @param  *p:ָ��У������ݣ�
* @param  len:У�����ݵĳ��ȣ�
* @retval None
*/
void  RecorderXorVer(u8 *pVer,u8 *p,u16 len)
{ 
    u16  i;
    
    for(i=0;i<len;i++)
    {
        *pVer ^= *(p+i);
    }
}

/**
* @brief  ��ָ�����ݽ��е�����
* @param  *p:ָ��������ݣ�
* @param  len:���ݳ���
* @retval ����ת��������ɹ����س��ȣ����򷵻�0
*/
u16  RecorderDataBack(u8 *p,u16 len)
{ 
    u8  buf[300]={0};
    u16 LenCnt=len-1;
    u16 LenAdd=0;
    
    if(len > sizeof(buf))
    {
        return 0;
    }
    
    for(;LenCnt != 0;)
    {
        buf[LenCnt] = *(p+LenAdd);
        LenCnt--;
        LenAdd++;
    }    
    buf[0] = *(p+LenAdd);
    memcpy(p,buf,len);//���Ƶ���������
    return LenCnt;
    
}
/**
* @brief  ���BCD��ʱ��
* @param  *p:ָ��������ݣ�
* @param  len:���ݳ���
* @retval ����Ϊ��ȷ���������
*/
u8  RecorderCheckBCDTime(u8 *p)
{
    u8 timetab[6];

    memcpy(timetab,p,6);
    if((timetab[5]&0x0f)>9)
    {
        *(p+5) = 0;
        timetab[5]= 0;
        return 0;
    }
    return 1;
}
/**
* @brief  ����¼��ָ���Ƿ�Ϸ�
* @param  cmd:��ʻ��¼��ָ��
* @retval ����Ϊ��ȷ���������
*/
u8  RecorderCheckCmd(u8 cmd)
{
    u8 i;
    for(i=0;i<sizeof(Recorder_Cmd_Num);i++)
    {
        if(Recorder_Cmd_Num[i] == cmd)
        {
            break;
        }
    }
    if(i == sizeof(Recorder_Cmd_Num))
    {
        return 0;
    }
    else
    {
        return 1;
    }
}
/**
* @brief  ���ɼ�¼�Ǵ���ָ������
* @param  type:Ӧ�����ͣ�0Ϊ�ɼ���������  ��0Ϊ���ô�������
* @param  *p:ָ������
* @retval �������ݳ���
*/
u8 RecorderCreateErrCmd(u8 type,u8 *p)
{
    u8 i;
    u8 verify;

    if(type == 0)                    //�ɼ�ָ����� 
    {
        verify = 0;
        *p=0x55;
        *(p+1)=0x7a;  
        *(p+2)=0xfa; 
        *(p+3)=PRO_DEF_RETAINS;  
        
        for(i=0; i<4; i++)
        {
            verify ^= *(p+i);
        }
	
        *(p+4) = verify;
    }
    else                         //����ָ�����
    {
        verify = 0;
        *p=0x55;
        *(p+1)=0x7a;  
        *(p+2)=0xfb; 
        *(p+3)=PRO_DEF_RETAINS;  
        
        for(i=0; i<4; i++)
        {
            verify ^= *(p+i);
        }
	
        *(p+4) = verify;            
    }
    return 5;

}
/*******************************************************************************
* Function Name  : Recorder_Stop15MinuteSpeed
* Description    : ͣ��ǰ15�����ٶȣ����ݰ���VIN��ͣ��ʱ�䡢ͣ��ǰ15����ƽ���ٶ�
* Input          : *p  : ָ��15����ƽ���ٶ���Ϣ�������              
* Output         : None
* Return         : �������ݳ���
*******************************************************************************/
u8 Recorder_Stop15MinuteSpeed(u8 *p)
{
    u8 Tab[70];
    u8 *pSend;
    u8 TimeBuf[6];//ʱ�仺����
    
    u8 step;
    u8 i;
    
    u32	TimeCnt;
    TIME_T  Time;

    
    pSend = p;
    memset(Tab,0,sizeof(Tab));
   
    ////////////////////////////////////////////////////////////////////////////  
    FRAM_BufferRead(pSend,FRAM_STOPING_TIME_LEN ,FRAM_STOPING_TIME_ADDR);//ͣ��ʱ��
    
    if(((*(pSend) == 0)&&(*(pSend+1)== 0)&&(*(pSend+2) == 0))||(SpeedMonitor_GetCurRunStatus()))
    {
        Public_ConvertNowTimeToBCDEx(pSend);//�洢BCD��ʱ�� 0-5
    }
    
    //////////////////////////////////////////////////////////////////////////// 
    FRAM_BufferRead(Tab,FRAM_STOP_15MINUTE_SPEED_LEN, FRAM_STOP_15MINUTE_SPEED_ADDR);//�ٶ�
    
    step = Tab[0];//��ǰ����
    
    for(i=0;i<15;i++)
    {
        if(step == 0)
        {
            step = 14;
        }
        else
        {
            step--;
        }
        
        if(i == 0)
        {     
            memcpy(pSend+6,Tab+(2+(step*4)),3);//����洢  
            if((*(pSend+6)==0)&&(*(pSend+7)==0))  
            {
                *(pSend+6) = *(pSend+3);
                *(pSend+7) = *(pSend+4);
                *(pSend+8) = 0;
            }   
        }     
        else
        {    
            if((TimeBuf[3] == Tab[2+(step*4)])&&(TimeBuf[4] == Tab[3+(step*4)]))
            {
                memcpy(pSend+6+(i*3),Tab+(2+(step*4)),3);//����洢
            }
            else
            {
                step++;
                memcpy(pSend+6+(i*3),TimeBuf+3,3);//����洢
            }
        }
        
        TimeBuf[0] = 0x13;
        TimeBuf[1] = 0x01;
        TimeBuf[2] = 0x01;
        TimeBuf[5] = 0x00;
        memcpy(TimeBuf+3,pSend+6+(i*3),2);//��ȡ���δ洢ʱ��,ʱ��              
        Public_ConvertBCDToTime(&Time,TimeBuf);//תʱ���ʽ
        TimeCnt = ConverseGmtime(&Time) - 60;
        Gmtime(&Time, TimeCnt); 
        Public_ConvertTimeToBCDEx(Time,TimeBuf);//�·���Ӧ��¼��ʱ��
        
    }
    
    return 51;
    
}
/*********************************************************************
//��������	:Recorder_SetCarInformation
//����		:���ó�����Ϣ����������VIN�����ƺ��롢���Ʒ���
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:������82H
*********************************************************************/
ProtocolACK Recorder_SetCarInformation(u8 *pBuffer, u8 BufferLen)
{
	u8	Buffer[20];
	u8	PramLen;
	u8	i;

	if(41 != BufferLen)
	{
		return ACK_ERROR;
	}
	//����ʶ�����17�ֽ�
	EepromPram_WritePram(E2_CAR_IDENTIFICATION_CODE_ID, pBuffer, 17);
	PramLen = EepromPram_ReadPram(E2_CAR_IDENTIFICATION_CODE_ID, Buffer);
	if(17 != PramLen)
	{
		return ACK_ERROR;
	}
	for(i=0; i<17; i++)
	{
		if(*(pBuffer+i) != Buffer[i])
		{
			return ACK_ERROR;
		}
	}
	pBuffer += 17;			
	//���ƺ���12�ֽ�
	EepromPram_WritePram(E2_CAR_PLATE_NUM_ID, pBuffer, 12);
	PramLen = EepromPram_ReadPram(E2_CAR_PLATE_NUM_ID, Buffer);
	if(12 != PramLen)
	{
		return ACK_ERROR;
	}
	for(i=0; i<12; i++)
	{
		if(*(pBuffer+i) != Buffer[i])
		{
			return ACK_ERROR;
		}
	}
	pBuffer +=12;			
	//���Ʒ���12�ֽ�
	EepromPram_WritePram(E2_CAR_TYPE_ID, pBuffer, 12);
	PramLen = EepromPram_ReadPram(E2_CAR_TYPE_ID, Buffer);
	if(12 != PramLen)
	{
		return ACK_ERROR;
	}
	for(i=0; i<12; i++)
	{
		if(*(pBuffer+i) != Buffer[i])
		{
			return ACK_ERROR;
		}
	}
	return ACK_OK;
	
}
/*********************************************************************
//��������	:Recorder_SetRtc
//����		:����ʵʱʱ��
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:������C2H
*********************************************************************/
ProtocolACK Recorder_SetRtc(u8 *pBuffer, u8 BufferLen)
{
	TIME_T	Rtc;

	if(6 != BufferLen)
	{
		return ACK_ERROR;
	} 
    if(Public_ConvertBCDToTime(&Rtc,pBuffer))
	{
		SetRtc(&Rtc);
        return ACK_OK;
	}
	else
	{
		return ACK_ERROR;
	}
        
}

/*********************************************************************
//��������	:Recorder_SetCarFeatureCoeff
//����		:���ó�������ϵ��
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:������C3H
*********************************************************************/
ProtocolACK Recorder_SetCarFeatureCoeff(u8 *pBuffer, u8 BufferLen)
{
	u8	Buffer[5];
	u8	PramLen;
        u8      nTab[5];
        
	if(8 != BufferLen)
	{
		return ACK_ERROR;
	}
	
        nTab[0] = 0;
        nTab[1] = *(pBuffer+6);
        nTab[2] = *(pBuffer+7);
        
        EepromPram_WritePram(E2_CAR_FEATURE_COEF_ID, nTab, 3);
	PramLen = EepromPram_ReadPram(E2_CAR_FEATURE_COEF_ID, Buffer);
	if(3 != PramLen)
	{
	    return ACK_ERROR;
	}
        if((Buffer[1]==*(pBuffer+6))&&(Buffer[2]==*(pBuffer+7)))
        {
            return ACK_OK;    
        }
        return ACK_ERROR;
        
	//PulseCounter_ChangePramNotice();
	//���³�������ϵ��
	//PulsePerKm_UpDatePram();
	
}
/*********************************************************************
//��������	:Recorder_GetCarInformation
//����		:��ȡ������Ϣ����������VIN�����ƺ��룬���Ʒ���
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:������06H
*********************************************************************/
u8 Recorder_GetCarInformation(u8 *pBuffer)
{
	u8	Buffer[45];
	u8	BufferLen;
	u8	i;

	//������VIN
	BufferLen = EepromPram_ReadPram(E2_CAR_IDENTIFICATION_CODE_ID, Buffer);
	if(BufferLen > 17)//ǿ�Ƶ���17
	{
		BufferLen = 17;
	}
	else if(BufferLen < 17)//����������0
	{
		for(i=BufferLen; i<17; i++)
		{
			Buffer[i] = 0;
		}
	}
	for(i=0; i<BufferLen; i++)//0ֵת��0
	{
		if(0 == Buffer[i])
		{
			Buffer[i]  = 0;
		}
	}
	memcpy(pBuffer,Buffer,17);
	pBuffer += 17;

	//�����ƺ���
	BufferLen = EepromPram_ReadPram(E2_CAR_PLATE_NUM_ID, Buffer);
	if(BufferLen > 12)//ǿ�Ƶ���12
	{
		BufferLen = 12;
	}
	else if(BufferLen < 12)//����������0
	{
		for(i=BufferLen; i<12; i++)
		{
			Buffer[i] = 0;
		}
	}
	for(i=0; i<BufferLen; i++)//0ֵת��0
	{
		if(0 == Buffer[i])
		{
			Buffer[i]  = 0;
		}
	}
	memcpy(pBuffer,Buffer,12);
	pBuffer += 12;

	//�����Ʒ���
	BufferLen = EepromPram_ReadPram(E2_CAR_TYPE_ID, Buffer);
	if(BufferLen > 12)//ǿ�Ƶ���12
	{
		BufferLen = 12;
	}
	else if(BufferLen < 12)//����������0
	{
		for(i=BufferLen; i<12; i++)
		{
			Buffer[i] = 0;
		}
	}
	for(i=0; i<BufferLen; i++)//0ֵת��0
	{
		if(0 == Buffer[i])
		{
			Buffer[i]  = 0;
		}
	}
	memcpy(pBuffer,Buffer,12);
	pBuffer += 12;

	return 41;
}


/*******************************************************************************
* Function Name  : Recorder_SendData
* Description    : ���ͼ�¼�Ƿ�������
* Input          : - *pData ��ָ��Ҫ�������ݵĵ�ַ
*                  - Len    :���͵����ݵĳ���             
* Output         : None
* Return         : �ɹ������� ���򷵻ؼ�
*******************************************************************************/
u8 Recorder_SendData(u8 *pData, u16 Len)
{
    u16 temp;
    
    if(St_RecorderQueueRx.nChannel)                            
    {
       	                                                   /*Ԥ��GSMģ�鷢��  */
    }
    else                                                   /*RS232ͨ��        */
    {
        temp = St_RecorderQueueTx.in;                      /*�ݴ���г���     */
        temp += Len;                                       /*�����ۼ�         */
        
        if(temp >1024)                                     /*�ж��Ƿ����     */
        {
            return 0;
        }
        
        temp = St_RecorderQueueTx.in;
        
        St_RecorderQueueTx.in += Len;                      /*���ݳ����ۼ�     */

        memcpy(St_RecorderQueueTx.nBuf+temp,pData,Len);    /*����ѹ�����     */
   	  
    }
    
    return 1;
}

/*******************************************************************************
* Function Name  : Recorder_QueueTxEmpty
* Description    : ��鷢�Ͷ����Ƿ�Ϊ��
* Input          : None             
* Output         : None
* Return         : ���пշ�����   ���򷵻ؼ�
*******************************************************************************/
u8 Recorder_QueueTxEmpty(void)
{
    if(St_RecorderQueueTx.in != 0)
    {
        return 1;
    }
    return 0;
}

/*******************************************************************************
* Function Name  : Recorder_SendOutTime
* Description    : ��ʱ��������
* Input          : None             
* Output         : None
* Return         : None
*******************************************************************************/
void Recorder_SendOutTime(void)
{
    if((St_RecorderQueueTx.out != St_RecorderQueueTx.in)
    &&(St_RecorderQueueTx.out < St_RecorderQueueTx.in))
    {
	COM1_WriteBuff(St_RecorderQueueTx.nBuf,St_RecorderQueueTx.in);		
        St_RecorderQueueTx.out = 0;
	St_RecorderQueueTx.in  = 0;		
    }    
     
}

/*******************************************************************************
* Function Name  : Recorder_BCD_HEX
* Description    : ת��ָ���ֽ�BCD�뵽HEX
* Input          : - *pData  : ָ��Ҫת����BCD��
*                  - Length  : ת��BCD���ֽ���                
* Output         : None
* Return         : - ����ת���õ�����
*******************************************************************************/
u32 Recorder_BCD_HEX(u8 *pData,u8 Length)
{
    u32 temp=0;
    
    if(Length == 1)
    {
        temp = (*pData/16)*10 + (*pData & 0x0f);
    }
    else if(Length == 2)
    {
        temp = (*pData/16)*1000   + (*pData & 0x0f)*100 
             + (*(pData+1)/16)*10 + (*(pData+1) & 0x0f);
    }
    else if(Length == 3)
    {
        temp = (*pData/16)*100000   + (*pData & 0x0f)*10000 
             + (*(pData+1)/16)*1000 + (*(pData+1) & 0x0f)*100 
             + (*(pData+2)/16)*10   + (*(pData+2) & 0x0f);
    }
    else if(Length == 4)
    {
        temp = (*pData/16)*10000000   + (*pData & 0x0f)*1000000 
             + (*(pData+1)/16)*100000 + (*(pData+1) & 0x0f)*10000   
             + (*(pData+2)/16)*1000   + (*(pData+2) & 0x0f)*100 
             + (*(pData+3)/16)*10     + (*(pData+3) & 0x0f);
     
    }
    else
    {
        temp = 0;
    }
	
    return temp;

}

/*******************************************************************************
* Function Name  : Recorder_BCD_HEX
* Description    : 32λ16����ת����BCD��,��Ҫ����ʮ�������λ��16�������λ��ϵ
*                  ����ת��
* Input          : - x  : Ҫת����HEX             
* Output         : None
* Return         : - ����ת���õ�����
*******************************************************************************/
u32 Recorder_HEX_BCD(u32 x)
{
    u32 y = 0;
    u32 z = 10000000;
    u32 h = 0x10000000;

    while (z)
    {
        while (x >= z)
        {
            x = x-z;
            y = y+h;
        }
        z = z/10;
        h = h>>4;
    }
    return y;
}

/*******************************************************************************
* Function Name  : Recorder_GetGBT2012Ver
* Description    : ��ȡ��¼��ִ�б�׼�汾
* Input          : - *pData  : ָ��洢���ݵĻ�����               
* Output         : None
* Return         : �洢���ݵĳ���
*******************************************************************************/
u8 Recorder_GetGBT2012Ver(u8 *pData)
{
    memcpy(pData,GB_T2012_Ver,sizeof(GB_T2012_Ver));
    return sizeof(GB_T2012_Ver);
}

/*******************************************************************************
* Function Name  : Recorder_GetDriverInformation
* Description    : ��ȡ������ʻ����Ϣ
* Input          : - *pData  : ָ��洢���ݵĻ�����               
* Output         : None
* Return         : �洢���ݵĳ���
*******************************************************************************/
u8 Recorder_GetDriverInformation(u8 *pData)
{
    
#if	TACHOGRAPHS_19056_TEST
	memcpy(pData,"XXXXXXXXXXXXXXXXXX",18);
#else
	u8 nLen;
    u8 nTab[35];
    u8 i;
	
    memset(nTab,0,sizeof(nTab));
    nLen = EepromPram_ReadPram(E2_CURRENT_LICENSE_ID,nTab);
    
    if(nLen < 18)
    {
        for(i=nLen;i<18;i++)
        {
            nTab[i] = 0x00;
        }
    }
    else if(nLen > 18)
    {
        nLen = 18;
    }

    for(i=0;i<nLen;i++)
    {
    	//ĳЩ���֤������������֡�X = 0X58��
        if((nTab[i]==0xff)||(nTab[i]<0x30)||(nTab[i]>0x58))
        {
            memset(nTab,0,sizeof(nTab));
            break;
        }
    }

    if(TiredDrive_GetLoginFlag()==0)                /*������������������    */
    {
        memset(nTab,0,sizeof(nTab));
    }
	
    memcpy(pData,nTab,18);	
#endif

    return 18;
}
/*******************************************************************************
* Function Name  : Recorder_GetRtc
* Description    : ��ȡʵʱʱ��
* Input          : - *pData  : ָ��洢���ݵĻ�����               
* Output         : None
* Return         : �洢���ݵĳ���
*******************************************************************************/
u8 Recorder_GetRtc(u8 *pData)
{
    u8 nTab[10];

    Public_ConvertNowTimeToBCDEx(nTab);
    memcpy(pData,nTab,6);
    
    return 6;
}

/*******************************************************************************
* Function Name  : Recorder_GetMileage
* Description    : ��ȡ�ۼ���ʻ���
* Input          : - *pData  : ָ��洢���ݵĻ�����               
* Output         : None
* Return         : �洢���ݵĳ���
*******************************************************************************/
/*
#define E2_CAR_INIT_MILE_ID       0x023d
#define E2_CAR_INIT_MILE_ADDR			(EEPROM_PRAM_BASE_ADDR+630)	
#define E2_CAR_INIT_MILE_LEN			4		
*/
u8 Recorder_GetMileage(u8 *pData)
{
    u8 nTab[30];
    u8 nLen;
    u8 timebuf[6];
#ifndef HUOYUN
    u32 temp_bcd;
    u32 temp_hex;
#endif
    u32 timecnt;
    TIME_T t_time;

 
    memset(nTab,0,sizeof(nTab));                            /*��ջ�����      */
   
    Public_ConvertNowTimeToBCDEx(nTab);                     /*����ʵʱʱ��    */  

    nLen = EepromPram_ReadPram(E2_INSTALL_TIME_ID,timebuf);  /*���ΰ�װʱ��    */
    if(nLen == E2_INSTALL_TIME_ID_LEN)
    {
        timecnt = timebuf[0];
        timecnt = (timecnt<<8) | timebuf[1];
        timecnt = (timecnt<<8) | timebuf[2];
        timecnt = (timecnt<<8) | timebuf[3];
        Gmtime(&t_time, timecnt); 
        Public_ConvertTimeToBCDEx(t_time,nTab+6);
    }
    else
    {
        memset(nTab+6,0,6);
    }

    nLen = EepromPram_ReadPram(E2_CAR_INIT_MILE_ID,nTab+12);/*��ʼ�����      */
    if(nLen != E2_CAR_INIT_MILE_ID_LEN)
    {      
        memset(nTab+12,0,4);
    }

#ifdef HUOYUN
    FRAM_BufferRead(nTab+16,4,FRAM_HY_MILEAGE_ADDR);//�ۼ����
#else
    temp_hex = nTab[12];
    temp_hex = (temp_hex<<8) | nTab[13];
    temp_hex = (temp_hex<<8) | nTab[14];
    temp_hex = (temp_hex<<8) | nTab[15];

    temp_bcd = Recorder_HEX_BCD(temp_hex);
    
    nTab[12] = temp_bcd>>24;
    nTab[13] = (temp_bcd>>16)&0x0000ff;
    nTab[14] = (temp_bcd>>8) &0x0000ff;
    nTab[15] = temp_bcd & 0x000000ff;
   
    temp_hex = Public_GetCurTotalMile();                    /*�ۼ���ʻ���   */
    temp_bcd = Recorder_HEX_BCD(temp_hex);
    
    nTab[16] = temp_bcd>>24;
    nTab[17] = (temp_bcd>>16)&0x0000ff;
    nTab[18] = (temp_bcd>>8) &0x0000ff;
    nTab[19] = temp_bcd & 0x000000ff; 
#endif	  
    memcpy(pData,nTab,20);
    
    return 20;
}
/*******************************************************************************
* Function Name  : Recorder_GetCarFeatureCoeff
* Description    : ��ȡ����ϵ��
* Input          : - *pData  : ָ��洢���ݵĻ�����               
* Output         : None
* Return         : �洢���ݵĳ���
*******************************************************************************/
u8 Recorder_GetCarFeatureCoeff(u8 *pData)
{
    u8 nTab[15];
    u8 nBuff[5];
    u8 nLen;
	
	
    memset(nTab,0,sizeof(nTab));                              /*��ջ�����    */    
    
    Public_ConvertNowTimeToBCDEx(nTab);                       /*����ʵʱʱ��  */ 
    
    nLen = EepromPram_ReadPram(E2_CAR_FEATURE_COEF_ID,nBuff);/*����ϵ��      */
    if(nLen != E2_CAR_FEATURE_COEF_LEN)
    {
        memset(nTab+6,0,2);
    }
    nTab[6] = nBuff[1];
    nTab[7] = nBuff[2];
    
    memcpy(pData,nTab,8);
	
    return 8;
}
/*******************************************************************************
* Function Name  : Recorder_GetOnlyNum
* Description    : ��ȡΨһ�Ա��
* Input          : - *pData  : ָ��洢���ݵĻ�����               
* Output         : None
* Return         : �洢���ݵĳ���
*******************************************************************************/
/*
#define E2_CAR_ONLY_NUM_ID       0x023e
#define E2_CAR_ONLY_NUM_ADDR		(EEPROM_PRAM_BASE_ADDR+630)	
#define E2_CAR_ONLY_NUM_LEN			35	
*/
u8 Recorder_GetOnlyNum(u8 *pData)
{
    u8 nTab[40];
    u8 nLen;
    u8 i;
    
    memset(nTab,0,sizeof(nTab));                          /*��ջ�����        */

    nLen = EepromPram_ReadPram(E2_CAR_ONLY_NUM_ID,nTab);  /*���ΰ�װʱ��      */
    if(nLen != E2_CAR_ONLY_NUM_ID_LEN)
    {         
        memset(nTab,0,sizeof(nTab));
    }
    //ʱ�仺�� 23 - 25����
    for(i=23;i<=25;i++)
    {
        if(((nTab[i]&0x0f)>9)||(nTab[i]>>4)>9)
        {
            nTab[23] = 0x13;
            nTab[24] = 0x04;
            nTab[25] = 0x18;
            break;
        }
    }
    memcpy(pData,nTab,sizeof(nTab));
    
    return 35;
}
/*******************************************************************************
* Function Name  : Recorder_SetInstallTime
* Description    : ���ó��ΰ�װ����
* Input          : - *pData  : ָ��д��洢��������
*                  - Len     : д��洢�����ݵĳ���              
* Output         : None
* Return         : д��ɹ�����ACK_OK�� ���򷵻�ACK_ERROR
*******************************************************************************/
ProtocolACK Recorder_SetInstallTime(u8 *pData,u8 Len)
{
    u8 nTab[6];
    u8 nLen;
    TIME_T time;
    u32    timecnt;
    
    if(Len != 6)
    {
        return ACK_ERROR;     
    }
    
    memcpy(nTab,pData,Len);

    time.year  = (nTab[0]>>4)*10 + (nTab[0]&0x0f);
    time.month = (nTab[1]>>4)*10 + (nTab[1]&0x0f);
    time.day   = (nTab[2]>>4)*10 + (nTab[2]&0x0f);
    time.hour  = (nTab[3]>>4)*10 + (nTab[3]&0x0f);
    time.min   = (nTab[4]>>4)*10 + (nTab[4]&0x0f);
    time.sec   = (nTab[5]>>4)*10 + (nTab[5]&0x0f);
    
    timecnt = ConverseGmtime(&time);

    nTab[0] =  timecnt>>24;             
    nTab[1] =  (timecnt>>16)&0xff; 
    nTab[2] =  (timecnt>>8)&0xff; 
    nTab[3] =   timecnt&0xff; 
    nTab[4] = 0; 
    nTab[5] = 0;   
    
    EepromPram_WritePram(E2_INSTALL_TIME_ID, nTab, 4);     /*ֻ�洢������ʱ   */

    nLen = EepromPram_ReadPram(E2_INSTALL_TIME_ID, nTab);
    if(4 != nLen)
    {
        return ACK_ERROR;
    }
    
    return ACK_OK;
}
/*******************************************************************************
* Function Name  : Recorder_SetInitMileage
* Description    : ���ó�ʼ�����
* Input          : - *pData  : ָ��д��洢��������
*                  - Len     : д��洢�����ݵĳ���              
* Output         : None
* Return         : д��ɹ�����ACK_OK�� ���򷵻�ACK_ERROR
*******************************************************************************/
ProtocolACK Recorder_SetInitMileage(u8 *pData,u8 Len)
{
    u8  nTab[25]={0};

#ifdef HUOYUN
    memcpy(nTab,pData,Len);
    Recorder_SetRtc(nTab, 6);
    Recorder_SetInstallTime(nTab+6,6);
    EepromPram_WritePram(E2_CAR_INIT_MILE_ID, nTab+12, 4);//��ʼ�����
    FRAM_BufferWrite(FRAM_HY_MILEAGE_ADDR,nTab+16,4);//�ۼ����

#else
    u32 nData;
    u8  nLen;
    if(Len > 4)
    {
        return ACK_ERROR;     
    }
    
    memcpy(nTab,pData,Len);
    
    nData = Recorder_BCD_HEX(nTab,Len);                       /*BCD��ת��ΪHEX*/
    
    nTab[0] = nData>>24;
    nTab[1] = (nData>>16)&0x0000ff;
    nTab[2] = (nData>>8) &0x0000ff;
    nTab[3] = nData & 0x000000ff;
    
    EepromPram_WritePram(E2_CAR_INIT_MILE_ID, nTab, 4);
    
    memset(nTab,0,sizeof(nTab));
    nLen = EepromPram_ReadPram(E2_CAR_INIT_MILE_ID, nTab);
    if(4 != nLen)
    {
        return ACK_ERROR;
    }
#endif
    return ACK_OK;
}

/*******************************************************************************
* Function Name  : Recorder_SetOnlyNum
* Description    : ����Ψһ�Ա��
* Input          : - *pData  : ָ��д��洢��������
*                  - Len     : д��洢�����ݵĳ���              
* Output         : None
* Return         : д��ɹ�����ACK_OK�� ���򷵻�ACK_ERROR
*******************************************************************************/
ProtocolACK Recorder_SetOnlyNum(u8 *pData,u8 Len)
{
    u8 nTab[35];
    u8 nLen;
    
    if(Len != 35)
    {
        return ACK_ERROR;     
    }
    
    memcpy(nTab,pData,Len);
    
    EepromPram_WritePram(E2_CAR_ONLY_NUM_ID, nTab, 35);  

    nLen = EepromPram_ReadPram(E2_CAR_ONLY_NUM_ID, nTab);
    if(35 != nLen)
    {
        return ACK_ERROR;
    }
    
    return ACK_OK;
}

/*******************************************************************************
* Function Name  : Recorder_MileageTest
* Description    : ��¼�Ǽ�������->������,��ÿ�����һ��
* Input          : None            
* Output         : None
* Return         : None
*******************************************************************************/
void Recorder_MileageTest(void)
{

    u8 nTab[55];
    
    u8 nBuff[5];
 
    u16 temp;
    u32 temp_hex;
    
    u8  nData;
    u8  i;
    u8  verify;
    
    memset(nTab,0,sizeof(nTab));                                              /*��ջ�����      */           
    
    nTab[0] = 0x55;
    nTab[1] = 0x7a;
    nTab[2] = 0xe1;
    nTab[3] = 0;
    nTab[4] = 44;
    nTab[5] = PRO_DEF_RETAINS;
    
    Recorder_GetOnlyNum(nTab+6);                                              /*��ȡΨһ���    */
  
    EepromPram_ReadPram(E2_CAR_FEATURE_COEF_ID,nBuff);                        /*����ϵ��        */

    nTab[35+6] = nBuff[1];
    nTab[36+6] = nBuff[2];
    
    temp = SpeedMonitor_GetCurSpeed();                                        /*��ȡʵʱ�ٶ�    */
    temp = temp * 10;
    nTab[37+6] = temp>>8;
    nTab[38+6] = temp&0xff00;
      
    temp_hex = Pulse_GetTotalMile();                                          /*�ۼ���ʻ���    */    
    nTab[39+6] = temp_hex>>24;
    nTab[40+6] = (temp_hex>>16)&0x0000ff;
    nTab[41+6] = (temp_hex>>8) &0x0000ff;
    nTab[42+6] = temp_hex & 0x000000ff; 
        
    nData = Io_ReadExtCarStatus();                                            /*�ƶ�            */
    nData = (nData<<1) | Io_ReadExtCarStatusBit(CAR_STATUS_BIT_LEFT_LIGHT);   /*��ת            */
    nData = (nData<<1) | Io_ReadExtCarStatusBit(CAR_STATUS_BIT_RIGHT_LIGHT);  /*��ת            */
    nData = (nData<<1) | Io_ReadExtCarStatusBit(CAR_STATUS_BIT_FAR_LIGHT);    /*Զ��            */
    nData = (nData<<1) | Io_ReadExtCarStatusBit(CAR_STATUS_BIT_NEAR_LIGHT);   /*����            */ 
    nData = (nData<<3);                                                       /*D0-D2�û��Զ��� */    
       
    nTab[43+6] = nData;     
        
    verify = 0;    
    for(i=0; i<(44+6); i++)
    {
         verify ^= nTab[i];
    }
    nTab[50] = verify;
    
    if(Recorder_QueueTxEmpty()==0)                                            /*�ж϶����Ƿ�Ϊ�� */
    {
        Recorder_SendData(nTab,51);
    }
  
}

/*******************************************************************************
* Function Name  : Recorder_Error_Cmd
* Description    : �򴮿ڷ������ݴ���ʱ��Ӧ��֡����
*                  �ʹ���֡����
* Input          : - Cmd  : ��ʻ��¼��������              
* Output         : None
* Return         : None
*******************************************************************************/
void Recorder_Error_Cmd(u8 Cmd)
{
    u8 verify=0;
    u8 i;
    u8 Buffer[5];
   
    if(Cmd < 0x80)                     /*�����������Ӧ��    */
    {
        verify = 0;
        Buffer[0] = 0x55;
        Buffer[1] = 0x7a;
        Buffer[2] = 0xfa;
        Buffer[3] = 0x00;
	
        for(i=0; i<4; i++)
        {
            verify ^= Buffer[i];
        }
	
        Buffer[4] = verify;
    }
    else                                                /*�´��������Ӧ��    */
    {
        verify = 0;
        Buffer[0] = 0x55;
        Buffer[1] = 0x7a;
        Buffer[2] = 0xfb;
        Buffer[3] = 0x00;
        
        for(i=0; i<4; i++)
        {
            verify ^= Buffer[i];      
        }
        
        Buffer[4] = verify;
              
    }
    if(Recorder_QueueTxEmpty()==0)                      /*�ж϶����Ƿ�Ϊ��    */
    {
        Recorder_SendData(Buffer,5);
    }
}

/*******************************************************************************
* Function Name  : Recorder_CheckVerify
* Description    : �����ʻ��¼�ǽ��յĴ�������У���Ƿ���ȷ,����ȷ���򴮿ڷ�
*                  �ʹ���֡����
* Input          : - *pData  : ָ��У�������
*                  - Length  : У�����ݳ���                
* Output         : None
* Return         : - ����У����ȷ�����棬���򷵻ؼ�
*******************************************************************************/
u8 Recorder_CheckVerify(u8 *pData,u16 Length)
{   
    u8  verify = 0;
    u16  i;
    u8 cmd;
    static u8 nDelay;
    
    for(i=0; i<Length-1; i++)                         /*У�������ֽ�          */
    {
        verify ^= *(pData+i);
    }
    
    if(verify != *(pData+Length-1))                   /*�ж�У���ֽ��Ƿ����  */
    {
        if((*(pData+0)==0xaa)&&(*(pData+1)==0x75))
        {
            nDelay++;
            if(nDelay < 2)                            /*�ȴ�ʣ�µ����ݰ�     */
            {
                nDelay++;  
                return 0;
            }
        }
        
        nDelay = 0;
        *(pData+0) = 0;                               /*����֡ͷ              */
        *(pData+1) = 0;
        
        for(i=0;i<512;i++)                            /*����������е�֡ͷ    */
        {
            if((*(pData+i)==0xaa)&&(*(pData+i+1)==0x75))
            {
                break;
            }
        }        
        
        cmd = *(pData+2);
        if(i>500)                                     /*����������֡ͷ�����  */
        {
            memset((u8*)&St_RecorderQueueRx,0,sizeof(RECORDER_QUEUE_RX));
        }
        if((*(pData+0)==0xaa)&&(*(pData+1)==0x75))
        {
            Recorder_Error_Cmd(cmd);                      /*�򴮿ڷ��ʹ���֡����  */
        }
  	  
        return 0;
    }
    nDelay = 0;
    return 1;
}

/*******************************************************************************
* Function Name  : Vdr_SerOutTime
* Description    : ��¼�Ǵ���һ֡���ݳ�ʱ�ж�
* Input          : - *pData  : ָ�򴮿�
*                  - Length  : ���Դ������ݵĳ��� 
* Output         : None
* Return         : - �յ�һ֡��������ȷ���ݷ����棬���򷵻ؼ�
*******************************************************************************/

u8 Recorder_SerOutTime(u8 *p,u16 len)
{
    u8  *pBuf  = St_RecorderQueueRx.nBuf;
    u16 BufLen = St_RecorderQueueRx.in;

    if(len == 0)
    {
        if(St_RecorderQueueRx.in)
        {
            if((SerOutTmrCnt++)>2)
            {
                SerOutTmrCnt = 0;//��ʱ
                SerBusyFlag  = 1;//æ
                return 1;
            }
        }
        return 0;//����������
    }
    
    SerOutTmrCnt = 0;//�����������峬ʱ������
    
    if(SerBusyFlag)
    {
        return 0;//�ϴε����ݻ�û�������
    }

    if((BufLen + len) < sizeof(St_RecorderQueueRx.nBuf))//�ж����
    {
        memcpy(pBuf+BufLen,p,len);
        St_RecorderQueueRx.in = St_RecorderQueueRx.in + len;
    }
    return 0;
}

/*******************************************************************************
* Function Name  : Recorder_Analyse
* Description    : ��ʻ��¼�����ݷ���
* Input          : - *pData  : ָ�����������
*                  - Length  : �������ݳ���  
* Output         : None
* Return         : - �յ�һ֡��������ȷ���ݷ����棬���򷵻ؼ�
*******************************************************************************/
u8 Recorder_Analyse(u8 *p,u16 Len)
{
    u8 *pRec = p;
    u16 temp;
    
    temp = ((*(pRec+3)>>8)|(*(pRec+4)))+7;
     
    if(Recorder_CheckVerify(pRec,temp) == 0)         /*�ж�У��               */
    {
        return 0;                                    /*У����󷵻ؼ�         */
    }
    
    return 1;
}
/*******************************************************************************
* Function Name  : Recorder_ReadStatus
* Description    : ��ȡ״̬�ź�
* Input          : *p��ָ�����ݵ�ַ        
* Output         : None
* Return         : �������ݳ���
*******************************************************************************/
u8 Recorder_ReadStatus(u8 *p)
{
    u8 addr=0;

    sFLASH_ReadBuffer(p,FLASH_STATUS_SET_SECTOR,80);

    if(strncmp((char *)(p+70),"�ƶ�",4) != 0)
    {
        memcpy(p+addr,"ACC       ",10);//DO
        addr = addr + 10;
        memcpy(p+addr,"��ɲ      ",10);//D1
        addr = addr + 10;
        memcpy(p+addr,"����      ",10);//D2
        addr = addr + 10;  
        memcpy(p+addr,"����      ",10);//D3
        addr = addr + 10;
        memcpy(p+addr,"Զ��      ",10);//D4
        addr = addr + 10;
        memcpy(p+addr,"��ת��    ",10);//D5
        addr = addr + 10;
        memcpy(p+addr,"��ת��    ",10);//D6
        addr = addr + 10;
        memcpy(p+addr,"�ƶ�      ",10);//D7        
    }

    return 80;
}
/*******************************************************************************
* Function Name  : Recorder_SetStatus
* Description    : ����״̬�ź�
* Input          : *p��ָ�����ݵ�ַ
*                 len�����ݳ���           
* Output         : None
* Return         : None
*******************************************************************************/
void Recorder_SetStatus(u8 *p, u8 len)
{
	sFLASH_EraseSector(FLASH_STATUS_SET_SECTOR);
	sFLASH_WriteBuffer(p,FLASH_STATUS_SET_SECTOR,len);
}
/**
* @brief  ���ü�¼�ǲ���
* @param  tpye:��ѯ����
* @param  *pdata:ָ�����õ�����
* @param  RecLen:ָ���������ݳ���
* @retval ���ؽ��
*/ 
u8 RecorderDataSet(u8 tpye,u8 *pRec, u16 RecLen)
{
    u8 result = 0;

    if(RecLen == 0)
    {
        return result;
    }
    switch(tpye)
    {
        case CMD_SET_CAR_INFORMATION:                /*���ó�����Ϣ           */
            {        
                if(ACK_OK == Recorder_SetCarInformation(pRec, RecLen))
                {
                    result = 1;
                }
    		}			
            break;
        case CMD_SET_CAR_INIT_INSTALL_TIME:          /*���ó��ΰ�װ����       */
            {
                if(ACK_OK == Recorder_SetInstallTime(pRec,RecLen))
                {
                    result = 1;
                }
            }
            break; 
        case CMD_SET_STATUS_SIGNAL_CONFIG:           /*����״̬��������Ϣ     */
            {      
                Recorder_SetStatus(pRec,RecLen);
                result = 1;
            }
            break; 
        case CMD_SET_RECORDER_TIME:                  /*���ü�¼��ʱ��         */
            {     
                if(ACK_OK == Recorder_SetRtc(pRec, RecLen))
                {
                    result = 1;
                }
            }    
            break; 
        case CMD_SET_IMPULSE_RATIO:                  /*���ü�¼������ϵ��     */
			{		
                if(ACK_OK == Recorder_SetCarFeatureCoeff(pRec, RecLen))
                {
                    result = 1;
                }
            }   
            break; 
        case CMD_SET_INIT_MILEAGE:                   /*���ó�ʼ�����         */
            {
                if(ACK_OK == Recorder_SetInitMileage(pRec,RecLen))
                {
                    result = 1;
                }
            }
            break; 
        case 0xc5:                                   /*����Ψһ�Ա��         */
            {
                if(ACK_OK == Recorder_SetOnlyNum(pRec,RecLen))
                {
                    result = 1;
                }
            }
            break;       
    }
    return result;
}
//�����޸ģ�ƽ̨�����޸�ʱ����
void Recorder_ParaSet(u8 cmd)
{
	u8  nBuff[20];
	if((cmd > 0x80)&&(cmd <= 0xcf))
	{                  
	    memset(nBuff,0,sizeof(nBuff));                  /*��ջ�����          */             
	    Public_ConvertNowTimeToBCDEx(nBuff);            /*��ȡBCD��ʵʱʱ��   */
	    nBuff[6] = cmd;                                 /*�洢����������      */
	    Register_Write(REGISTER_TYPE_PRAMATER,nBuff,13);/*����flash��         */
	}
}
/**
* @brief  ��ѯ��ʻ��¼��0-7������
* @param  tpye:��ѯ����
* @param  *pdata:ָ����ȡ������
* @retval �������ݸ���
*/ 
u16 RecorderData0_7(u8 tpye,u8 *pdata)
{
    u16 len=0;
    
    switch(tpye)
    {
        case CMD_GET_PERFORM_VERSION:                /*�ɼ���¼�ǰ汾         */
            {      
                len = Recorder_GetGBT2012Ver(pdata);               
            }
            break;
        case CMD_GET_DRIVER_INFORMATION:             /*�ɼ���ʻԱ��Ϣ         */
            {      
                len = Recorder_GetDriverInformation(pdata);      
            }
            break;
        case CMD_GET_SYS_RTC_TIME:                   /*�ɼ���¼��ʵʱʱ��     */
            {   
               len = Recorder_GetRtc(pdata);
            }
            break;
        case CMD_GET_ADD_MILEAGE:                    /*�ɼ��ۼ���ʻ���       */
            {
                len = Recorder_GetMileage(pdata);
            }
            break;
        case CMD_GET_IMPULSE_RATIO:                  /*�ɼ���¼������ϵ��     */
            { 
                len = Recorder_GetCarFeatureCoeff(pdata);
            }
            break;
        case CMD_GET_CAR_INFORMATION:                /*�ɼ�������Ϣ           */
            {   
                len = Recorder_GetCarInformation(pdata);
            }
            break;
        case CMD_GET_STATUS_SIGNAL_CONFIG:           /*�ɼ�״̬�ź�����       */
            {          
                Public_ConvertNowTimeToBCDEx(pdata);  /*��ȡBCD��ʵʱʱ��      */
                *(pdata+6) = 1;
                Recorder_ReadStatus(pdata+7);
                len = 87;
            }
            break;
        case CMD_GET_UNIQUE_ID:                      /*�ɼ�ΨһID             */
            {
                len =  Recorder_GetOnlyNum(pdata);
            }
            break;
        default:
        {
           len = 0; 
        }
        break;
    }
    return (len);
}
/*******************************************************************************
* Function Name  : Recorder_Handle
* Description    : ������ʻ��¼�����ݴ���
* Input          : None              
* Output         : None
* Return         : None
*******************************************************************************/
void Recorder_Handle(void)
{
    
    u8  cmd=0;                                       /*��ʻ��¼��������       */   
    u8  flag=0;                                      /*�������ݱ�־           */
    s16 nLen=0;                                      /*�����ݳ���             */
    u8  verify;                                      /*��������У���ֽ�       */
    u16 length;
    u16 i;
    
    u8	*pSend;                                      /*��������ָ��           */
    u8  *pRec;                                       /*��������ָ��           */
    
    u8  nTab[1024];
    u8  nBuff[20];

    TIME_T nStartTime,nEndTime;
//    u32    nStartTimeCnt;
    u32    nEndTimeCnt;
    u16    nMaxBlock;
    
    pSend = nTab;                                    /*��ʼ���������ݵ�ַ     */
    pRec  = St_RecorderQueueRx.nBuf;                 /*��ʼ���������ݵ�ַ     */
               
    cmd  = *(pRec+2);                                /*����������             */   
    length = (*(pRec+3) << 8)|*(pRec+4);             /*�������ݳ���           */
    
    switch(cmd)
    {
    	
/*******�ɼ�����������*********************************************************/
        case CMD_GET_PERFORM_VERSION:                /*�ɼ���¼�ǰ汾         */
                      
            nLen = Recorder_GetGBT2012Ver(pSend+6);
            flag = 1;
            
            break;
        case CMD_GET_DRIVER_INFORMATION:             /*�ɼ���ʻԱ��Ϣ         */
            
            nLen = Recorder_GetDriverInformation(pSend+6);
            flag = 1;            
            
            break;
        case CMD_GET_SYS_RTC_TIME:                   /*�ɼ���¼��ʵʱʱ��     */

            nLen = Recorder_GetRtc(pSend+6);
            flag = 1;

            break;
        case CMD_GET_ADD_MILEAGE:                    /*�ɼ��ۼ���ʻ���       */

            nLen = Recorder_GetMileage(pSend+6);
            flag = 1;
            
            break;
        case CMD_GET_IMPULSE_RATIO:                  /*�ɼ���¼������ϵ��     */
            
            nLen = Recorder_GetCarFeatureCoeff(pSend+6);
            flag = 1;
            
            break;
        case CMD_GET_CAR_INFORMATION:                /*�ɼ�������Ϣ           */
            
            nLen = Recorder_GetCarInformation(pSend+6);
            flag = 1;

            break;
        case CMD_GET_STATUS_SIGNAL_CONFIG:           /*�ɼ�״̬�ź�����       */
             
             Public_ConvertNowTimeToBCDEx(pSend+6);  /*��ȡBCD��ʵʱʱ��      */
             *(pSend+12) = 1;
             Recorder_ReadStatus(pSend+13);
             nLen = 87;
             flag = 1;
            
            break;
        case CMD_GET_UNIQUE_ID:                      /*�ɼ�ΨһID             */
            
            nLen = Recorder_GetOnlyNum(pSend+6);
            flag = 1;
            
            break;
        case CMD_GET_SPECIFY_RECORD_SPEED:                       /*ָ������ʻ�ٶȼ�¼          */
                     
                Public_ConvertBCDToTime(&nStartTime,pRec+6);     /*��ʼʱ��                    */
                Public_ConvertBCDToTime(&nEndTime,pRec+12);      /*����ʱ��                    */                           
                
                nEndTimeCnt = ConverseGmtime(&nEndTime);
                nEndTimeCnt++;
                Gmtime(&nEndTime, nEndTimeCnt);              
                
                nMaxBlock = (*(pRec+18)<<8) | *(pRec+19);        /*����                      */
                //nLen = Register_Read(REGISTER_TYPE_SPEED,pSend+6,nStartTime,nEndTime,nMaxBlock);
                nLen = SpeedMonitor_ReadSpeedRecordData(pSend+6,nStartTime,nEndTime,nMaxBlock);
                flag = 1;
            
            break;
        case CMD_GET_SPECIFY_RECORD_POSITION:                    /*ָ����λ����Ϣ��¼          */
                
                Public_ConvertBCDToTime(&nStartTime,pRec+6);     /*��ʼʱ��                    */
                Public_ConvertBCDToTime(&nEndTime,pRec+12);      /*����ʱ��                    */  
                
                nEndTimeCnt = ConverseGmtime(&nEndTime);
                nEndTimeCnt++;
                Gmtime(&nEndTime, nEndTimeCnt);  
                
                nMaxBlock = (*(pRec+18)<<8) | *(pRec+19);        /*����                      */                               
                nLen = Register_Read(REGISTER_TYPE_POSITION,pSend+6,nStartTime,nEndTime,nMaxBlock);
                flag = 1;         
            
             break;
        case CMD_GET_SPECIFY_RECORD_ACCIDENT_DOUBT:              /*ָ�����¹��ɵ��¼          */
                
                Public_ConvertBCDToTime(&nStartTime,pRec+6);     /*��ʼʱ��                    */
                Public_ConvertBCDToTime(&nEndTime,pRec+12);      /*����ʱ��                    */      
                nMaxBlock = (*(pRec+18)<<8) | *(pRec+19);        /*����                      */
                //nMaxBlock = 1;
				nLen = Register_Read(REGISTER_TYPE_DOUBT,pSend+6,nStartTime,nEndTime,nMaxBlock);
                flag = 1;
                                              
            break;
        case CMD_GET_SPECIFY_RECORD_OVERTIME_DRIVING:            /*ָ���ĳ�ʱ��ʻ��¼          */
                
                Public_ConvertBCDToTime(&nStartTime,pRec+6);     /*��ʼʱ��                    */
                Public_ConvertBCDToTime(&nEndTime,pRec+12);      /*����ʱ��                    */      
                nMaxBlock = (*(pRec+18)<<8) | *(pRec+19);        /*����                      */
                nLen = TiredDrive_ReadTiredRecord(pSend+6,nStartTime,nEndTime);
                flag = 1;                            

            break;
        case CMD_GET_SPECIFY_RECORD_DRIVER_IDENTITY:             /*ָ���ļ�ʻԱ��ݼ�¼        */
                
                Public_ConvertBCDToTime(&nStartTime,pRec+6);     /*��ʼʱ��                    */
                Public_ConvertBCDToTime(&nEndTime,pRec+12);      /*����ʱ��                    */      
                nMaxBlock = (*(pRec+18)<<8) | *(pRec+19);        /*����                      */
                nLen = Register_Read(REGISTER_TYPE_DRIVER,pSend+6,nStartTime,nEndTime,nMaxBlock);
                flag = 1;
                
            break;
        case CMD_GET_SPECIFY_RECORD_EXTERN_POWER:                /*ָ�����ⲿ�����¼          */
                
                Public_ConvertBCDToTime(&nStartTime,pRec+6);     /*��ʼʱ��                    */
                Public_ConvertBCDToTime(&nEndTime,pRec+12);      /*����ʱ��                    */      
                nMaxBlock = (*(pRec+18)<<8) | *(pRec+19);        /*����                      */
                nLen = Register_Read(REGISTER_TYPE_POWER_ON,pSend+6,nStartTime,nEndTime,nMaxBlock);
                flag = 1;
                
            break;
        case CMD_GET_SPECIFY_RECORD_CHANGE_VALUE:                /*ָ���Ĳ����޸ļ�¼          */
                
                Public_ConvertBCDToTime(&nStartTime,pRec+6);     /*��ʼʱ��                    */
                Public_ConvertBCDToTime(&nEndTime,pRec+12);      /*����ʱ��                    */       
                nMaxBlock = (*(pRec+18)<<8) | *(pRec+19);        /*����                      */
                nLen = Register_Read(REGISTER_TYPE_PRAMATER,pSend+6,nStartTime,nEndTime,nMaxBlock);
                flag = 1;
                
            break;
        case CMD_GET_SPECIFY_LOG_STATUS_SPEED:                   /*ָ�����ٶ�״̬��־          */
                
                Public_ConvertBCDToTime(&nStartTime,pRec+6);     /*��ʼʱ��                    */
                Public_ConvertBCDToTime(&nEndTime,pRec+12);      /*����ʱ��                    */
                
                nMaxBlock = (*(pRec+18)<<8) | *(pRec+19);        /*����                      */
                nLen = Register_Read(REGISTER_TYPE_SPEED_STATUS,pSend+6,nStartTime,nEndTime,nMaxBlock);
                flag = 1;             

            break;    

/*******���ò���������*********************************************************/ 
        case CMD_SET_CAR_INFORMATION:                /*���ó�����Ϣ           */
                
            /*���������г���ʶ����롢���ƺ��롢����*/
            if(ACK_OK == Recorder_SetCarInformation(pRec+6, length))
            {
                flag = 1;
            }
					
            break;
        case CMD_SET_CAR_INIT_INSTALL_TIME:          /*���ó��ΰ�װ����       */
            
            if(ACK_OK == Recorder_SetInstallTime(pRec+6,length))
            {
                flag = 1;
            }
            
            break; 
        case CMD_SET_STATUS_SIGNAL_CONFIG:           /*����״̬��������Ϣ     */
                  
            Recorder_SetStatus(pRec+6,length);
            flag = 1;
            
            break; 
        case CMD_SET_RECORDER_TIME:                  /*���ü�¼��ʱ��         */
                
            if(ACK_OK == Recorder_SetRtc(pRec+6, length))
            {
                flag = 1;
            }
                
            break; 
        case CMD_SET_IMPULSE_RATIO:                  /*���ü�¼������ϵ��     */
					
            if(ACK_OK == Recorder_SetCarFeatureCoeff(pRec+6, length))
            {
                flag = 1;
            }
                
            break; 
        case CMD_SET_INIT_MILEAGE:                   /*���ó�ʼ�����         */
            /*AA 75 C4 00 04 00 11 22 33 44 5B*/
            if(ACK_OK == Recorder_SetInitMileage(pRec+6,length))
            {
                flag = 1;
            }
            
            break; 
        case 0xc5:                                   /*����Ψһ�Ա��         */
            
            if(ACK_OK == Recorder_SetOnlyNum(pRec+6,length))
            {
                flag = 1;
            }
            
            break;           
/**************************�����������б�**************************************/
            
        case CMD_IDENTIFY_INTO:                   /*����򱣳ּ���״̬        */
            Public_ShowTextInfo("�������״̬",100);  
            Public_PlayTTSVoiceAlarmStr("�������״̬");
            Test_EnterE0();
            flag = 1;
            
            break;                       
          
        case CMD_IDENTIFY_MEASURE_MILEAGE:        /*�������������          */
                       
            Recorder_MileageTest();
            Test_EnterE1();           
                
            break;  
          
        case CMD_IDENTIFY_MEASURE_IMPULSE_RATIO:  /*��������ϵ��������      */

            Test_EnterE2();
            flag = 1;
            
            break;            
            
        case CMD_IDENTIFY_MEASURE_SYS_RTC_TIME:   /*����ʵʱʱ��������      */

            Test_EnterE3();
            flag = 1;

            break; 
        case CMD_IDENTIFY_RETURN:                 /*������������״̬          */
           Public_ShowTextInfo("��������״̬",100);  
           Public_PlayTTSVoiceAlarmStr("��������״̬");
           Test_EnterE4(); 
           flag = 1;
            
            break; 
 
 /*******����******************************************************************/            
 #if REDEBUG_USB  
       case 0xaa:
         
            sFLASH_EraseBulk();
            flag = 1;
            
        break;
 #endif       
 /*******�����Զ���************************************************************/  
        case 0xd0:                                   /*���ò����������Զ���   */
	    if(0 == *(pRec+6))                       /*������                 */
	    {
		*pSend = 0x55;
		*(pSend+1) = 0x7a;
		*(pSend+2) = cmd;
		//nLen = Recorder_ReadPram(pSend+7, pRec+7, length-1); dxl,2013.10.21���ߺ��������ò���ʹ��ͳһ�Ľӿں���
		RadioProtocol_ReadPram(pSend+7, &nLen,pRec+7, length-1,0);
		nLen++;                              /*���϶���־             */
		*(pSend+3) = nLen >> 8;              /*����                   */
		*(pSend+4) = nLen;
		*(pSend+5) = PRO_DEF_RETAINS;
		*(pSend+6) = 0;                      /*����־                 */
		if(nLen > 0)
		{
		    verify = 0;
		    for(i=0; i<nLen+6; i++)
		    {
		        verify ^= *(pSend+i);
		    }
		}
		*(pSend+6+nLen) = verify;
		Recorder_SendData(pSend, nLen+7);
	    }
	    else if(1 == *(pRec+6))                  /*д����                 */
	    {
                //if(ACK_OK == Recorder_WritePram(pRec+7, length-1))  dxl,2013.10.21���ߺ��������ò���ʹ��ͳһ�Ľӿں���
		if(ACK_OK == RadioProtocol_WritePram(0,pRec+7, length-1))
		{
		    nLen = 1;                    
                    *pSend = 0x55;
		    *(pSend+1) = 0x7a;
                    *(pSend+2) = cmd;
                    *(pSend+3) = nLen >> 8;
                    *(pSend+4) = nLen;
                    *(pSend+5) = PRO_DEF_RETAINS;
                    *(pSend+6) = 1;
		    
                    verify = 0;
                    for(i=0; i<nLen+6; i++)
		    {
			verify ^= *(pSend+i);
	            }
                    *(pSend+nLen+6) = verify;
                    Recorder_SendData(pSend,nLen+7);    /*���ͷ�������        */
		}
	    }
	    break;  
   
 /*******�Ƿ�������************************************************************/            
        default:
          
            break;  	
    }
    
/***�����޸ļ�¼***************************************************************/    
    if((cmd > 0x80)&&(cmd <= 0xcf))
    {                  
        memset(nBuff,0,sizeof(nBuff));                  /*��ջ�����          */             
        Public_ConvertNowTimeToBCDEx(nBuff);            /*��ȡBCD��ʵʱʱ��   */
        nBuff[6] = cmd;                                 /*�洢����������      */
        Register_Write(REGISTER_TYPE_PRAMATER,nBuff,13);/*����flash��         */
    }
    
/***����Ƿ������ݷ���*********************************************************/    
    
    if(flag != 0)                                       
    {
        flag = 0; 
        
        if(cmd > 0x80)
        {
            nLen = 0;
        }
	
        *pSend = 0x55;
        *(pSend+1) = 0x7a;
        *(pSend+2) = cmd;
        *(pSend+3) = nLen >> 8;
        *(pSend+4) = nLen;
        *(pSend+5) = PRO_DEF_RETAINS;        
        
        verify = 0;    
        for(i=0; i<(nLen+6); i++)
        {
            verify ^= *(pSend+i);
        }
        *(pSend+nLen+6) = verify;
      
        Recorder_SendData(pSend,nLen+7);                /*���ͷ�������        */
    }
    
    St_RecorderQueueRx.in -= (*(pRec+4)+7);             /*���г��ȼ���һ֡    */
    memset(St_RecorderQueueRx.nBuf,0,(*(pRec+4)+7));    /*����һ֡����        */
}

/*******************************************************************************
* Function Name  : Recorder_USBRead_Vdr
* Description    : ��U����д���¼������
* Input          : None            
* Output         : None
* Return         : None
*******************************************************************************/
void Recorder_USBRead_Vdr(void)
{
    FIL file;
    u8	Buffer[513];
    u8	Buffer1[25];
    u32	BufferLen=0; 
    u32 flen=0;
    
    u8  DataTab[800];
    u32 DataLen=0;
    u32 DataStep;
    u32 i,k;
    u32	 CurrentTimeCount,TimeCountCheck;
    TIME_T tt,tt_check;
    //kong
    LCD_LIGHT_ON();
    Public_ShowTextInfo("��ʼ������",100); 
    if(FR_OK == f_open(&file,ff_NameConver("123.VDR"),FA_READ))
    {
        
      memset(Buffer,0,sizeof(Buffer));
      f_read(&file,Buffer,512,&BufferLen);
      
      flen = 0;                                 /*���ݿ���*/
      f_lseek(&file,flen);
      memset(Buffer1,0,sizeof(Buffer1));
      f_read(&file,Buffer1,2,&BufferLen);     
      
      flen = 2;                                 
      f_lseek(&file,flen);
/******************************************************************************/      
      f_read(&file,Buffer1,23,&BufferLen);     /*ִ����� 00H*/       
      if(Buffer1[0] == 0x00)
      {
          DataLen = (Buffer1[21]*256)+Buffer1[22];
          f_lseek(&file,flen+23);
          f_read(&file,DataTab,DataLen,&BufferLen);  
          flen = flen+23+DataLen;                   
          f_lseek(&file,flen);
      }
      else
      {
          f_lseek(&file,flen);
      }
/******************************************************************************/      
      f_read(&file,Buffer1,23,&BufferLen);     /*��ʻԱ��Ϣ 01H*/
      if(Buffer1[0] == 0x01)
      {
          DataLen = (Buffer1[21]*256)+Buffer1[22];
          f_lseek(&file,flen+23);
          f_read(&file,DataTab,DataLen,&BufferLen);
          flen = flen+23+DataLen;                    
          f_lseek(&file,flen);
      }
      else
      {
          f_lseek(&file,flen);
      }
/******************************************************************************/      
      f_read(&file,Buffer1,23,&BufferLen);    /*ʵʱʱ�� 02H*/
      if(Buffer1[0] == 0x02)
      {
          DataLen = (Buffer1[21]*256)+Buffer1[22];
          f_lseek(&file,flen+23);
          f_read(&file,DataTab,DataLen,&BufferLen);
          flen = flen+23+DataLen;                    
          f_lseek(&file,flen);
      }
      else
      {
          f_lseek(&file,flen);
      }
/******************************************************************************/       
      f_read(&file,Buffer1,23,&BufferLen);       /*�ۼ���� 03H*/
      if(Buffer1[0] == 0x03)
      {
          DataLen = (Buffer1[21]*256)+Buffer1[22];
          f_lseek(&file,flen+23);
          f_read(&file,DataTab,DataLen,&BufferLen);
          //Recorder_SetInitMileage(DataTab,DataLen);    
          flen = flen+23+DataLen;                    
          f_lseek(&file,flen);
      }
      else
      {
          f_lseek(&file,flen);
      }
/******************************************************************************/      
      f_read(&file,Buffer1,23,&BufferLen);     /*����ϵ�� 04H*/
      if(Buffer1[0] == 0x04)
      {
          DataLen = (Buffer1[21]*256)+Buffer1[22];
          f_lseek(&file,flen+23);
          f_read(&file,DataTab,DataLen,&BufferLen);
          //Recorder_SetCarFeatureCoeff(DataTab, DataLen);         
          flen = flen+23+DataLen;                    
          f_lseek(&file,flen);
      }
      else
      {
          f_lseek(&file,flen);
      }      
/******************************************************************************/      
      f_read(&file,Buffer1,23,&BufferLen);     /*������Ϣ 05H*/
      if(Buffer1[0] == 0x05)
      {
          DataLen = (Buffer1[21]*256)+Buffer1[22];
          f_lseek(&file,flen+23);
          f_read(&file,DataTab,DataLen,&BufferLen);
          //Recorder_SetCarInformation(DataTab, DataLen);     
          flen = flen+23+DataLen;                    
          f_lseek(&file,flen);
      }
      else
      {
          f_lseek(&file,flen);
      }
/******************************************************************************/      
      f_read(&file,Buffer1,23,&BufferLen);    /*״̬������Ϣ 06H*/
      if(Buffer1[0] == 0x06)
      {
          DataLen = (Buffer1[21]*256)+Buffer1[22];
          f_lseek(&file,flen+23);
          f_read(&file,DataTab,DataLen,&BufferLen);      
          //sFLASH_EraseSector(FLASH_STATUS_SET_SECTOR);
          //sFLASH_WriteBuffer(DataTab+7,FLASH_STATUS_SET_SECTOR,DataLen-7);      
          flen = flen+23+DataLen;                    
          f_lseek(&file,flen);
      }
      else
      {
          f_lseek(&file,flen);
      }
/******************************************************************************/      
      f_read(&file,Buffer1,23,&BufferLen);     /*Ψһ�Ա�� 07H*/
      if(Buffer1[0] == 0x07)
      {
          DataLen = (Buffer1[21]*256)+Buffer1[22];
          f_lseek(&file,flen+23);
          f_read(&file,DataTab,DataLen,&BufferLen);
          flen = flen+23+DataLen;                    
          f_lseek(&file,flen);
      }
      else
      {
          f_lseek(&file,flen);
      }
/******************************************************************************/        
      Public_ShowTextInfo("�ٶ�����",100); 
      f_read(&file,Buffer1,23,&BufferLen);/*��ʻ�ٶ� 08H*/
      if(Buffer1[0] == 0x08)
      {
          Register_EraseOneArea(REGISTER_TYPE_SPEED);
          
          DataLen = (((Buffer1[19]<<24)|(Buffer1[20]<<16))|(Buffer1[21]<<8))|Buffer1[22];     
          
          DataStep = DataLen/126;/*�޸ĳ��� 1*/
          
          for(i=DataStep-1;i!=0XFFFFFFFF;i--)
          //for(i=0;i<DataStep;i++)  
          {
              f_lseek(&file,flen+23+(126*i));/*�޸ĳ��� 2*/
              memset(DataTab,0,sizeof(DataTab));
              f_read(&file,DataTab,126,&BufferLen); /*�޸ĳ��� 3*/
              
              tt.year = ((DataTab[0]>>4)*10)+(DataTab[0]&0x0f);
              tt.month= ((DataTab[1]>>4)*10)+(DataTab[1]&0x0f);
              tt.day = ((DataTab[2]>>4)*10)+(DataTab[2]&0x0f);
              tt.hour =((DataTab[3]>>4)*10)+(DataTab[3]&0x0f);
              tt.min = ((DataTab[4]>>4)*10)+(DataTab[4]&0x0f);
              tt.sec = ((DataTab[5]>>4)*10)+(DataTab[5]&0x0f);          
              
              CurrentTimeCount = ConverseGmtime(&tt);
              #if (REGISTER_SPEED_COLLECT_LEN==2)
              for(k=0;k<120;k=k+2)
              {
                  memset(Buffer1,0,sizeof(Buffer1));
                  memcpy(Buffer1,DataTab+6+k,2);
                  if(ERROR == Register_Write2(REGISTER_TYPE_SPEED,Buffer1,6,CurrentTimeCount+(k/2)))/*�޸ĳ��� 4 */
                  {
                      break;
                  }         
              }
              #elif (REGISTER_SPEED_COLLECT_LEN==126)
              if(ERROR == Register_Write2(REGISTER_TYPE_SPEED,DataTab,REGISTER_SPEED_STATUS_STEP_LEN-5,CurrentTimeCount))/*�޸ĳ��� 4 */
              {
                  break;
              }  
              #endif
          }
          flen = flen+23+DataLen;                    
          f_lseek(&file,flen);
      }
      else
      {
          f_lseek(&file,flen);
      }
/******************************************************************************/       
      Public_ShowTextInfo("λ����Ϣ����",100); 
      f_read(&file,Buffer1,23,&BufferLen);/*λ����Ϣ 09H*/
      if(Buffer1[0] == 0x09)
      {      
          Register_EraseOneArea(REGISTER_TYPE_POSITION);
          DataLen = (((Buffer1[19]<<24)|(Buffer1[20]<<16))|(Buffer1[21]<<8))|Buffer1[22];
          DataStep = DataLen/666;/*�޸ĳ��� 1*/
          
          for(i=DataStep-1;i!=0XFFFFFFFF;i--)
          //for(i=0;i<DataStep;i++)
          {
              f_lseek(&file,flen+23+(666*i));/*�޸ĳ��� 2*/
              memset(DataTab,0,sizeof(DataTab));
              f_read(&file,DataTab,666,&BufferLen); /*�޸ĳ��� 3*/
              
              tt.year = ((DataTab[0]>>4)*10)+(DataTab[0]&0x0f);
              tt.month= ((DataTab[1]>>4)*10)+(DataTab[1]&0x0f);
              tt.day = ((DataTab[2]>>4)*10)+(DataTab[2]&0x0f);
              tt.hour =((DataTab[3]>>4)*10)+(DataTab[3]&0x0f);
              tt.min = ((DataTab[4]>>4)*10)+(DataTab[4]&0x0f);
              tt.sec = ((DataTab[5]>>4)*10)+(DataTab[5]&0x0f);          
              
              CurrentTimeCount = ConverseGmtime(&tt);
              
              for(k=0;k<660;k=k+11)
              {
                  memset(Buffer1,0,sizeof(Buffer1));
                  memcpy(Buffer1,DataTab+6+k,11);
                  TimeCountCheck = CurrentTimeCount + ((k/11)*60);
                  Gmtime(&tt_check, TimeCountCheck); 
                  if(ERROR == Register_Write2(REGISTER_TYPE_POSITION,Buffer1,
                                              REGISTER_POSITION_STEP_LEN-5,
                                              TimeCountCheck))/*�޸ĳ��� 4*/
                  {
                      break;
                  }
              }
          } 
          flen = flen+23+DataLen;                    
          f_lseek(&file,flen);
      }
      else
      {
          f_lseek(&file,flen);
      }      
/******************************************************************************/        
      Public_ShowTextInfo("�¹��ɵ�����",100); 
      f_read(&file,Buffer1,23,&BufferLen);/*�¹��ɵ� 10H*/
      if(Buffer1[0] == 0x10)
      {       
          Register_EraseOneArea(REGISTER_TYPE_DOUBT);
          DataLen = (((Buffer1[19]<<24)|(Buffer1[20]<<16))|(Buffer1[21]<<8))|Buffer1[22];
          DataStep = DataLen/234;
          
          for(i=DataStep-1;i!=0XFFFFFFFF;i--)
          //for(i=0;i<DataStep;i++)
          {
              f_lseek(&file,flen+23+(234*i));
              memset(DataTab,0,sizeof(DataTab));
              f_read(&file,DataTab,234,&BufferLen); 
              
              tt.year = ((DataTab[0]>>4)*10)+(DataTab[0]&0x0f);
              tt.month= ((DataTab[1]>>4)*10)+(DataTab[1]&0x0f);
              tt.day = ((DataTab[2]>>4)*10)+(DataTab[2]&0x0f);
              tt.hour =((DataTab[3]>>4)*10)+(DataTab[3]&0x0f);
              tt.min = ((DataTab[4]>>4)*10)+(DataTab[4]&0x0f);
              tt.sec = ((DataTab[5]>>4)*10)+(DataTab[5]&0x0f);          
              
              CurrentTimeCount = ConverseGmtime(&tt);
              if(ERROR == Register_Write2(REGISTER_TYPE_DOUBT,DataTab,REGISTER_DOUBT_STEP_LEN-5,CurrentTimeCount))
              {
                 break;
              }
          }    
          flen = flen+23+DataLen;                    
          f_lseek(&file,flen);
      }
      else
      {
          f_lseek(&file,flen);
      }          
      
/******************************************************************************/       
      Public_ShowTextInfo("��ʱ��ʻ����",100); 
      f_read(&file,Buffer1,23,&BufferLen);/*��ʱ��ʻ 11H*/
      if(Buffer1[0] == 0x11)
      {       
          Register_EraseOneArea(REGISTER_TYPE_OVER_TIME);
          DataLen = (((Buffer1[19]<<24)|(Buffer1[20]<<16))|(Buffer1[21]<<8))|Buffer1[22];
          DataStep = DataLen/50;/*�޸ĳ��� 1*/
          
          //for(i=DataStep-1;i!=0XFFFFFFFF;i--)
          for(i=0;i<DataStep;i++)
          {
              f_lseek(&file,flen+23+(50*i));/*�޸ĳ��� 2*/
              memset(DataTab,0,sizeof(DataTab));
              f_read(&file,DataTab,50,&BufferLen); /*�޸ĳ��� 3*/
              
              tt.year = ((DataTab[0+24]>>4)*10)+(DataTab[0+24]&0x0f);
              tt.month= ((DataTab[1+24]>>4)*10)+(DataTab[1+24]&0x0f);
              tt.day = ((DataTab[2+24]>>4)*10)+(DataTab[2+24]&0x0f);
              tt.hour =((DataTab[3+24]>>4)*10)+(DataTab[3+24]&0x0f);
              tt.min = ((DataTab[4+24]>>4)*10)+(DataTab[4+24]&0x0f);
              tt.sec = ((DataTab[5+24]>>4)*10)+(DataTab[5+24]&0x0f);          
              
              CurrentTimeCount = ConverseGmtime(&tt);
              if(ERROR == Register_Write2(REGISTER_TYPE_OVER_TIME,DataTab,REGISTER_OVER_TIME_STEP_LEN-5,CurrentTimeCount))/*�޸ĳ��� 4*/
              {
                  break;
              }
          } 
          flen = flen+23+DataLen;                    
          f_lseek(&file,flen);
      }
      else
      {
          f_lseek(&file,flen);
      }
      
 /******************************************************************************/        
      Public_ShowTextInfo("��ʻԱ�������",100); 
      f_read(&file,Buffer1,23,&BufferLen);/*��ʻԱ��� 12H*/
      if(Buffer1[0] == 0x12)
      {
          Register_EraseOneArea(REGISTER_TYPE_DRIVER);
          DataLen = (((Buffer1[19]<<24)|(Buffer1[20]<<16))|(Buffer1[21]<<8))|Buffer1[22];
          DataStep = DataLen/25;/*�޸ĳ��� 1*/
          
          for(i=DataStep-1;i!=0XFFFFFFFF;i--)
          //for(i=0;i<DataStep;i++)
          {
              f_lseek(&file,flen+23+(25*i));/*�޸ĳ��� 2*/
              memset(DataTab,0,sizeof(DataTab));
              f_read(&file,DataTab,25,&BufferLen); /*�޸ĳ��� 3*/
              
              tt.year = ((DataTab[0]>>4)*10)+(DataTab[0]&0x0f);
              tt.month= ((DataTab[1]>>4)*10)+(DataTab[1]&0x0f);
              tt.day = ((DataTab[2]>>4)*10)+(DataTab[2]&0x0f);
              tt.hour =((DataTab[3]>>4)*10)+(DataTab[3]&0x0f);
              tt.min = ((DataTab[4]>>4)*10)+(DataTab[4]&0x0f);
              tt.sec = ((DataTab[5]>>4)*10)+(DataTab[5]&0x0f);          
              
              CurrentTimeCount = ConverseGmtime(&tt);
              if(ERROR == Register_Write2(REGISTER_TYPE_DRIVER,DataTab,REGISTER_DRIVER_STEP_LEN-5,CurrentTimeCount))/*�޸ĳ��� 4*/
              {
                  break;
              }
          }        
          flen = flen+23+DataLen;                    
          f_lseek(&file,flen);
      }
      else
      {
          f_lseek(&file,flen);
      }
      
/******************************************************************************/       
      Public_ShowTextInfo("�ⲿ��������",100); 
      f_read(&file,Buffer1,23,&BufferLen);/*�ⲿ���� 13H*/
      if(Buffer1[0] == 0x13)
      {      
         Register_EraseOneArea(REGISTER_TYPE_POWER_ON);
         DataLen = (((Buffer1[19]<<24)|(Buffer1[20]<<16))|(Buffer1[21]<<8))|Buffer1[22];
         DataStep = DataLen/7;/*�޸ĳ��� 1*/
          
          for(i=DataStep-1;i!=0XFFFFFFFF;i--)
          //for(i=0;i<DataStep;i++)
          {
              f_lseek(&file,flen+23+(7*i));/*�޸ĳ��� 2*/
              memset(DataTab,0,sizeof(DataTab));
              f_read(&file,DataTab,7,&BufferLen); /*�޸ĳ��� 3*/
              
              tt.year = ((DataTab[0]>>4)*10)+(DataTab[0]&0x0f);
              tt.month= ((DataTab[1]>>4)*10)+(DataTab[1]&0x0f);
              tt.day = ((DataTab[2]>>4)*10)+(DataTab[2]&0x0f);
              tt.hour =((DataTab[3]>>4)*10)+(DataTab[3]&0x0f);
              tt.min = ((DataTab[4]>>4)*10)+(DataTab[4]&0x0f);
              tt.sec = ((DataTab[5]>>4)*10)+(DataTab[5]&0x0f);          
              
              CurrentTimeCount = ConverseGmtime(&tt);
              if(ERROR == Register_Write2(REGISTER_TYPE_POWER_ON,DataTab,REGISTER_POWER_ON_STEP_LEN-5,CurrentTimeCount))/*�޸ĳ��� 4*/
              {
                  break;
              }
          }
          flen = flen+23+DataLen;                    
          f_lseek(&file,flen);
      }
      else
      {
          f_lseek(&file,flen);
      }
/******************************************************************************/       
      Public_ShowTextInfo("�����޸�����",100); 
      f_read(&file,Buffer1,23,&BufferLen);/*�����޸� 14H*/
      if(Buffer1[0] == 0x14)
      {        
          Register_EraseOneArea(REGISTER_TYPE_PRAMATER);
          DataLen = (((Buffer1[19]<<24)|(Buffer1[20]<<16))|(Buffer1[21]<<8))|Buffer1[22];
          DataStep = DataLen/7;/*�޸ĳ��� 1*/
          
          for(i=DataStep-1;i!=0XFFFFFFFF;i--)
          //for(i=0;i<DataStep;i++)
          {
              f_lseek(&file,flen+23+(7*i));/*�޸ĳ��� 2*/
              memset(DataTab,0,sizeof(DataTab));
              f_read(&file,DataTab,7,&BufferLen); /*�޸ĳ��� 3*/
              
              tt.year = ((DataTab[0]>>4)*10)+(DataTab[0]&0x0f);
              tt.month= ((DataTab[1]>>4)*10)+(DataTab[1]&0x0f);
              tt.day = ((DataTab[2]>>4)*10)+(DataTab[2]&0x0f);
              tt.hour =((DataTab[3]>>4)*10)+(DataTab[3]&0x0f);
              tt.min = ((DataTab[4]>>4)*10)+(DataTab[4]&0x0f);
              tt.sec = ((DataTab[5]>>4)*10)+(DataTab[5]&0x0f);          
              
              CurrentTimeCount = ConverseGmtime(&tt);
              if(ERROR == Register_Write2(REGISTER_TYPE_PRAMATER,DataTab,REGISTER_PRAMATER_STEP_LEN-5,CurrentTimeCount))/*�޸ĳ��� 4*/
              {
                  break;
              }
          }
          flen = flen+23+DataLen;                    
          f_lseek(&file,flen);
      }
      else
      {
          f_lseek(&file,flen);
      }
/******************************************************************************/       
      Public_ShowTextInfo("�ٶ�״̬��־����",100); 
      f_read(&file,Buffer1,23,&BufferLen);/*�ٶ�״̬��־ 15H*/
      if(Buffer1[0] == 0x15)
      {       
          Register_EraseOneArea(REGISTER_TYPE_SPEED_STATUS);
          DataLen = (((Buffer1[19]<<24)|(Buffer1[20]<<16))|(Buffer1[21]<<8))|Buffer1[22];
          DataStep = DataLen/133;/*�޸ĳ��� 1*/    
          for(i=DataStep-1;i!=0XFFFFFFFF;i--)
          //for(i=0;i<DataStep;i++)
          {
              f_lseek(&file,flen+23+(133*i));/*�޸ĳ��� 2*/
              memset(DataTab,0,sizeof(DataTab));
              f_read(&file,DataTab,133,&BufferLen); /*�޸ĳ��� 3*/
              
              tt.year = ((DataTab[0+7]>>4)*10)+(DataTab[0+7]&0x0f);
              tt.month= ((DataTab[1+7]>>4)*10)+(DataTab[1+7]&0x0f);
              tt.day = ((DataTab[2+7]>>4)*10)+(DataTab[2+7]&0x0f);
              tt.hour =((DataTab[3+7]>>4)*10)+(DataTab[3+7]&0x0f);
              tt.min = ((DataTab[4+7]>>4)*10)+(DataTab[4+7]&0x0f);
              tt.sec = ((DataTab[5+7]>>4)*10)+(DataTab[5+7]&0x0f);          
              
              CurrentTimeCount = ConverseGmtime(&tt);
              if(ERROR == Register_Write2(REGISTER_TYPE_SPEED_STATUS,DataTab,REGISTER_SPEED_STATUS_STEP_LEN-5,CurrentTimeCount))/*�޸ĳ��� 4*/
              {
                  break;
              }
          }
      }
      else
      {
          f_lseek(&file,flen);
      }      

    }
    f_close(&file);
    LCD_LIGHT_ON();
    Public_ShowTextInfo("����",100); 
    
    
}



/*******************************************************************************
* Function Name  : Recorder_Vdr_Name
* Description    : VDR�ļ���
* Input          : None            
* Output         : None
* Return         : None
*******************************************************************************/
void Recorder_Vdr_Name(void)
{
    u8  tab[50]={0};
    u16 i,j;
    u8  len;
   
    Public_ConvertNowTimeToBCDEx((u8*)&tab);
    vdr_fname.start[0] = 'D';
    vdr_fname.time_ymd[0] = (tab[0]>>4)   + 0x30;
    vdr_fname.time_ymd[1] = (tab[0]&0x0f) + 0x30;
    vdr_fname.time_ymd[2] = (tab[1]>>4)   + 0x30;
    vdr_fname.time_ymd[3] = (tab[1]&0x0f) + 0x30;
    vdr_fname.time_ymd[4] = (tab[2]>>4)   + 0x30;
    vdr_fname.time_ymd[5] = (tab[2]&0x0f) + 0x30;
    vdr_fname.separator_1[0] = '_';
    vdr_fname.time_hm[0] = (tab[3]>>4)   + 0x30;
    vdr_fname.time_hm[1] = (tab[3]&0x0f) + 0x30;
    vdr_fname.time_hm[2] = (tab[4]>>4)   + 0x30;
    vdr_fname.time_hm[3] = (tab[4]&0x0f) + 0x30;  
    vdr_fname.separator_2[0] = '_';
 
    memset(tab,0,sizeof(tab));
 
    len = EepromPram_ReadPram(E2_CAR_PLATE_NUM_ID, tab);
    
    len = strlen((char*)tab);
    
    for(i=0;i<len;i++)
    {
        if((tab[i]!=0)||(tab[i]!=0xff))
        {
            break;
        }
    }
    if(i != len)
    {
        for(i=0;i<98;i+=2)                               
        {
            if((tab[0]==h_BufGb2312[i])&&(tab[1]==h_BufGb2312[i+1]))
            {
                vdr_fname.num[0]  = h_BufUnicode[i/2];
                break;
            }
        }
        if(i < 98)              
        {
            for(i=1,j=2;j<len;i++,j++)
            {
                vdr_fname.num[i] = tab[j];
            }
            vdr_fname.num[i++] = '.';
            vdr_fname.num[i++] = 'V';
            vdr_fname.num[i++] = 'D';   
            vdr_fname.num[i++] = 'R'; 
        } 
        else
        {
            vdr_fname.num[0] = 0x672A; //δ
            vdr_fname.num[1] = 0x77E5; //֪
            vdr_fname.num[2] = 0x8F66; //��
            vdr_fname.num[3] = 0x724C; //��  
            vdr_fname.num[4] = '.';
            vdr_fname.num[5] = 'V';
            vdr_fname.num[6] = 'D';   
            vdr_fname.num[7] = 'R'; 
        }
    }
}


/*******************************************************************************
* Function Name  : ��VDR�ļ���д������
* Description    : ��ʻ��¼��USB���ݴ���
* Input          : None            
* Output         : None
* Return         : None
*******************************************************************************/
void Recorder_VdrFileWite(u8 *p, u16 len)
{
    f_write (&file, p, len, (void *)&vdr_Written); 
    RecorderXorVer(&vdr_ver,p,len);
}

/*******************************************************************************
* Function Name  : ��VDR�ļ���д��0-7����
* Description    : ��ʻ��¼��USB���ݴ���
* Input          : None            
* Output         : None
* Return         : None
*******************************************************************************/
void Recorder_VdrCmd_07(u8 block)
{
    u8  bufsave[5]={0};
    u16 bufsavelen;
    u16 i=0;

    bufsave[0] = 0;
    bufsave[1] = block+8;
    Recorder_VdrFileWite(bufsave,2);
        
    for(i=0;i<8;i++)
    {
        memset((u8*)&vdr_fdata,0,sizeof(RECORDER_FILE_VDR_DATA));
        vdr_fdata.cmd = i;
        memcpy(vdr_fdata.name,Vdr_Cmd07_Name[i],18);         
        bufsavelen = RecorderData0_7(i,vdr_fdata.buf);
        vdr_fdata.length[2] = bufsavelen>>8;
        vdr_fdata.length[3] = bufsavelen&0x00ff;  
        Recorder_VdrFileWite((u8*)&vdr_fdata,bufsavelen+23);
    }
}
/*******************************************************************************
* Function Name  : ������VDR�ļ���д������ݳ���
* Description    : ��ʻ��¼��USB���ݴ���
* Input          : None            
* Output         : None
* Return         : None
*******************************************************************************/
u32 Recorder_VdrCmd_DataLen(u8 Cmd,u16 allcnt,TIME_T Time_start, TIME_T Time_end)
{
    u32 bufsavelen;
    u8  PackSizeNum;
    if(Cmd == 0x08)
    {
        PackSizeNum = 0;
    }
    else if(Cmd == 0x09)
    {
        PackSizeNum = 1;
    }
    else if(Cmd == 0x10)
    {
        PackSizeNum = 2;
    }
    else if(Cmd == 0x11)
    {
        PackSizeNum = 3;
    }
    else if(Cmd == 0x12)
    {
        PackSizeNum = 4;
    }    
    else if(Cmd == 0x13)
    {
        PackSizeNum = 5;
    }
    else if(Cmd == 0x14)
    {
        PackSizeNum = 6;
    }
    else if(Cmd == 0x15)
    {
        PackSizeNum = 7;
    }
    if(allcnt != 0)
    {
        bufsavelen = Recorder_Cmd_OneNum[PackSizeNum] * allcnt;
    }
    else
    {
        bufsavelen = 0;
    }
    
    return bufsavelen;
}

/*******************************************************************************
* Function Name  : Recorder_USBStartRead
* Description    : ��ʼ����USB����
* Input          : None            
* Output         : None
* Return         : None
*******************************************************************************/
void Recorder_USBStartRead(u8 Type)
{
	u16	SearchSector  = Register[Type].CurrentSector;      //��ǰ����
	u16	SearchStep    = Register[Type].CurrentStep;        //��ǰ����
    u16	SearchStepLen = Register[Type].StepLen;            //���� 

    RecorderFlashAddr.Sector = SearchSector;      //��ǰ����
	RecorderFlashAddr.Step   = SearchStep;        //��ǰ����
    RecorderFlashAddr.StepLen= SearchStepLen;     //����   
    DataBlockAll             = 0; 
}
/*******************************************************************************
* Function Name  : ��VDR�ļ���д�����ʱ���������
* Description    : ��ʻ��¼��USB���ݴ���
* Input          : None            
* Output         : None
* Return         : None
*******************************************************************************/
void Recorder_VdrCmd_Time(u8 Cmd)
{
	TIME_T Time_start;  //��ʼʱ��
	TIME_T Time_end;    //����ʱ��
	u32  allcnt=0;
	u32  addcnt=1;
	u32  bufsavelen=0;
	u8   namecnt;
    u8   tab_lcd[50]={0};//����LCD��ʾ���ݵ�������
    
    RTC_GetCurTime(&Time_start);
    Time_start.year--;
    RTC_GetCurTime(&Time_end);
    Time_end.year++;

    allcnt = RecorderDataPack(RecorderCmdToRegister(Cmd),Time_start,Time_end,0,NULL);
    if(Cmd == 0x08)
    {
        Public_ShowTextInfo("�ɼ��ٶ�����",100); 
        bufsavelen = Recorder_VdrCmd_DataLen(Cmd,allcnt,Time_start,Time_end);
        namecnt = Cmd;
    }
    else if(Cmd == 0x09)
    {
        Public_ShowTextInfo("�ɼ�λ����Ϣ����",100);     
        bufsavelen = Recorder_VdrCmd_DataLen(Cmd,allcnt,Time_start,Time_end);
        namecnt = Cmd;
    }
    else if(Cmd == 0x10)
    {
        Public_ShowTextInfo("�ɼ��¹��ɵ�����",100);  
        bufsavelen = Recorder_VdrCmd_DataLen(Cmd,allcnt,Time_start,Time_end);
        namecnt = 10;
    }
    else if(Cmd == 0x11)
    {
        Public_ShowTextInfo("�ɼ���ʱ��ʻ����",100);
        bufsavelen = Recorder_VdrCmd_DataLen(Cmd,allcnt,Time_start,Time_end);
        namecnt = 11;
    }
    else if(Cmd == 0x12)
    {
        Public_ShowTextInfo("�ɼ���ʻԱ�������",100);
        bufsavelen = Recorder_VdrCmd_DataLen(Cmd,allcnt,Time_start,Time_end);
        namecnt = 12;
    }    
    else if(Cmd == 0x13)
    {
        Public_ShowTextInfo("�ɼ��ⲿ��������",100);    
        bufsavelen = Recorder_VdrCmd_DataLen(Cmd,allcnt,Time_start,Time_end);
        namecnt = 13;
    }
    else if(Cmd == 0x14)
    {
        Public_ShowTextInfo("�ɼ������޸�����",100);    
        bufsavelen = Recorder_VdrCmd_DataLen(Cmd,allcnt,Time_start,Time_end);
        namecnt = 14;
    }
    else if(Cmd == 0x15)
    {
        Public_ShowTextInfo("�ɼ��ٶ�״̬��־����",100);        
        bufsavelen = Recorder_VdrCmd_DataLen(Cmd,allcnt,Time_start,Time_end);
        namecnt = 15;
    }
    else
    {
        return;
    }
    memset((u8*)&vdr_fdata,0,sizeof(RECORDER_FILE_VDR_DATA));
    vdr_fdata.cmd = Cmd;
    memcpy(vdr_fdata.name,Vdr_Cmd07_Name[namecnt],18);
    vdr_fdata.length[0] = bufsavelen>>24;
    vdr_fdata.length[1] = (bufsavelen>>16)&0xff;     
    vdr_fdata.length[2] = (bufsavelen>>8)&0xff;
    vdr_fdata.length[3] = bufsavelen&0xff;  
    Recorder_VdrFileWite((u8*)&vdr_fdata,23);
    Recorder_USBStartRead(RecorderCmdToRegister(Cmd));
    while(1)
    {  
        if(allcnt == 0)
        {
            break;
        }
        bufsavelen = RecorderDataPackUsb(RecorderCmdToRegister(Cmd),
                                Time_start,Time_end,addcnt,vdr_fdata.buf);
        Recorder_VdrFileWite((u8*)&vdr_fdata.buf,(u16)bufsavelen);
        memset(tab_lcd,0,sizeof(tab_lcd));
        sprintf((char*)tab_lcd,"�ܰ���=%d,��ǰ����=%d",allcnt,addcnt);
        Public_ShowTextInfo((char*)tab_lcd,100); 
        IWDG_ReloadCounter();
        if(addcnt >= allcnt)
        {
            break;
        }
        else
        {
            addcnt++;
        } 
    }

}

/*******************************************************************************
* Function Name  : Recorder_USBHandle
* Description    : ��ʻ��¼��USB���ݴ���
* Input          : None            
* Output         : None
* Return         : None
*******************************************************************************/
void Recorder_USBHandle(void)
{
    u8 i;
    u8 tab[10]={0};
    u32	BufferLen=0; 
    u8  writeflg=0;
    if(f_open(&file,(WCHAR*)ff_NameConver("VDR.txt"),FA_READ | FA_WRITE) == FR_OK)
    {
        f_read(&file,tab,5,&BufferLen);
        if(strncmp((char *)tab,"write",5) == 0)
        {
            memset(tab,0,sizeof(tab));
            memcpy(tab,"ok",2);
            f_lseek(&file,0);
            f_write (&file, tab, 2, (void *)&BufferLen); 
            writeflg = 1;
        }
    }
    f_close(&file);
    if(writeflg)
    {
        Recorder_USBRead_Vdr();
        return ;
    }

    
    E2prom_ReadByte(E2_PROGRAM_UPDATA_REQUEST_ID_ADDR,tab, 2);
    if(tab[0] == 0xaa)
    {
        return ;
    }
    
    LCD_LIGHT_ON();

    Recorder_Vdr_Name();//vdr�ļ������
    IWDG_ReloadCounter();
    if(f_open(&file,(WCHAR*)&vdr_fname,FA_CREATE_ALWAYS | FA_WRITE) == FR_OK)
    {      
        Public_ShowTextInfo("��ʼ��������",100); 
        Recorder_VdrCmd_07(8);
        i=8;
        for(;i<sizeof(Recorder_Cmd_Num);i++)
        {
            Recorder_VdrCmd_Time(Recorder_Cmd_Num[i]);
        }  
        f_write (&file, &vdr_ver, 1, (void *)&vdr_Written); 
        f_close(&file);
        Public_ShowTextInfo("���ݵ������",100); 
    }
    else
    {
        f_close(&file);
        Public_ShowTextInfo("U�̲�ʶ��!!",100); 
    }
}

/**
* @brief  ����Ƿ���Ҫȫ������0-7ָ������
* @param  None
* @retval ���ؽ����0Ϊ����Ҫ����0Ϊ��Ҫ
*/ 
u8 Recorder_CheckAll07(void)
{
    if((St_RecorderWire.All_07)&&(St_RecorderWire.Cmd<7))
    {
        St_RecorderWire.AckNum++;
        St_RecorderWire.Cmd++;
        return 1;
    }
    else
    {
        St_RecorderWire.All_07 = 0;
        return 0;
    }
}
/**
* @brief  ����Ƿ�������ͷְ�����
* @param  None
* @retval ���ؽ����0Ϊ����Ҫ����0Ϊ��Ҫ
*/ 
u8 Recorder_CheckPackage(void)
{
    if((St_RecorderWire.AllCnt != 0)&&(St_RecorderWire.AddCnt != St_RecorderWire.AllCnt))
    {
        St_RecorderWire.AddCnt++;
        SubpackageCurrentSerial++;//��ˮ���ۼ�
#if 0
        if(SubpackageCurrentSerial == 10)
        {
            SubpackageCurrentSerial = 15;
        }
#endif      
        return 1;
    }
    else
    {
        St_RecorderWire.AddCnt = 0;
        St_RecorderWire.AllCnt = 0;
        SubpackageCurrentSerial = 0;//������ˮ��
        SubpacketUploadFlag = 0;//������ˮ�ű�־
        
        return 0;
    }
}
/**
* @brief  ��¼������ת��FLASH�洢����
* @param  tpye:��¼������
* @retval ����FLASH�洢����
*/
u8 RecorderCmdToRegister(u8 Type)
{
    u8 RegType=0;
    
    switch(Type)
    {
        case CMD_GET_SPECIFY_RECORD_SPEED:
        {
            RegType = REGISTER_TYPE_SPEED;
        }
        break;
        case CMD_GET_SPECIFY_RECORD_POSITION:
        {
            RegType = REGISTER_TYPE_POSITION;
        }
        break;
        case CMD_GET_SPECIFY_RECORD_ACCIDENT_DOUBT:
        {
            RegType = REGISTER_TYPE_DOUBT;
        }
        break;
        case CMD_GET_SPECIFY_RECORD_OVERTIME_DRIVING:
        {
            RegType = REGISTER_TYPE_OVER_TIME;
        }
        break;
        case CMD_GET_SPECIFY_RECORD_DRIVER_IDENTITY:
        {
            RegType = REGISTER_TYPE_DRIVER;
        }
        break;
        case CMD_GET_SPECIFY_RECORD_EXTERN_POWER:
        {
            RegType = REGISTER_TYPE_POWER_ON;
        }
        break;
        case CMD_GET_SPECIFY_RECORD_CHANGE_VALUE:
        {
            RegType = REGISTER_TYPE_PRAMATER;
        }
        break;
        case CMD_GET_SPECIFY_LOG_STATUS_SPEED:
        {
            RegType = REGISTER_TYPE_SPEED_STATUS;
        }
        break;        
    }
    return (RegType);
    
}

/**
* @brief  �ٶȷְ�����
* @param  *src:ָ��Դ����ָ��
* @param  *pBlockAll:ָ�����ݿ������
* @param  PackNum:Ҫ��ȡ�����ݰ���
* @retval �������ݰ��ĳ���
*/
u16 RecorderSpeedPack(u8 *src,u16 PackNum,u16 *pBlockAll)
{
    u8 *dest=src;

    if(SearchTime.min != SpeedPosTime.min)//����ӷְ�
    {
        if(dest != NULL)
        {
            #ifdef USE_ONE_MIN_SAVE_SPEED_RECORD
            memcpy(dest,RecorderFlash.buf,126);
            #else
            memset(dest,0xff,126);
            *dest++ = Public_HEX2BCD(SearchTime.year);
            *dest++ = Public_HEX2BCD(SearchTime.month);
            *dest++ = Public_HEX2BCD(SearchTime.day);
            *dest++ = Public_HEX2BCD(SearchTime.hour);
            *dest++ = Public_HEX2BCD(SearchTime.min);
            *dest++ = 0x00;
            memcpy(dest+(SearchTime.sec*2),RecorderFlash.buf,2);
            #endif
        }
        DataBlockSpeedPos = 1;
    }
    else
    {
        if(dest != NULL)
        {
            memcpy(dest+6+(SearchTime.sec*2),RecorderFlash.buf,2);
        }
        DataBlockSpeedPos++;
    }
    if(SearchTime.sec == 0)
    {
        (*pBlockAll)++;
        DataBlockSpeedPos = 0;
        return 126;
    }    
    SpeedPosTime = SearchTime;
    return 0;
    
}

/**
* @brief  λ����Ϣ�ְ�����
* @param  *src:ָ��Դ����ָ��
* @param  PackNum:Ҫ��ȡ�����ݰ���
* @param  *pLen:ָ�򷵻����ݰ��ĳ���
* @retval ���ش���������ָ��
*/
u16 RecorderPositionPack(u8 *src,u16 PackNum,u16 *pBlockAll)
{
    u8 *dest=src;
    
    if(SearchTime.hour != SpeedPosTime.hour)//����ӷְ�
    {
        if(dest != NULL)
        {
            memset(dest,0xff,666);
            *dest++ = Public_HEX2BCD(SearchTime.year);
            *dest++ = Public_HEX2BCD(SearchTime.month);
            *dest++ = Public_HEX2BCD(SearchTime.day);
            *dest++ = Public_HEX2BCD(SearchTime.hour);
            *dest++ = 0x00;
            *dest++ = 0x00;
            memcpy(dest+(SearchTime.min*11),RecorderFlash.buf,11);
        }
        DataBlockSpeedPos = 1;
    }
    else
    {
        if(dest != NULL)
        {
            memcpy(dest+6+(SearchTime.min*11),RecorderFlash.buf,11);
        }
        DataBlockSpeedPos++;
    }
    if(SearchTime.min == 0)
    {
        (*pBlockAll)++;
        DataBlockSpeedPos = 0;
        return 666;
    }    
    SpeedPosTime = SearchTime;
    return 0;
}

/**
* @brief  ���������ݿ������ܰ���
* @param  tpye:��ѯ����
* @param  *pBlock:ָ�����ݿ���
* @param  *pPack:ָ�����ݿ���
* @retval �������ݳ���
*/
u16 RecorderTotal(u8 Type,u16 PackNum,u8 *pData)
{
    u16 len=0;
    u8  *p=pData;
    
    if(Type == REGISTER_TYPE_SPEED)
    {

    }
    else if(Type == REGISTER_TYPE_POSITION)
    {
        
    }
    else
    {
        DataBlockAll = DataBlockAll + 1;//���ݿ��1
    }
    switch(Type)
    {
        case REGISTER_TYPE_SPEED:
        {
            len = RecorderSpeedPack(p,PackNum,&DataBlockAll);
        }
        break;
        case REGISTER_TYPE_POSITION:
        {
            len = RecorderPositionPack(p,PackNum,&DataBlockAll);
        }
        break;
        case REGISTER_TYPE_DOUBT:
        {
            if(pData != NULL)
            {
                len = Recorder_Cmd_OneNum[2];
                memcpy(p,RecorderFlash.buf,len);
            } 
        }
        break;
        case REGISTER_TYPE_OVER_TIME:
        {
            if(pData != NULL)
            {
                len = Recorder_Cmd_OneNum[3];
                memcpy(p,RecorderFlash.buf,len);
            } 
        }
        break;
        case REGISTER_TYPE_DRIVER:
        {
            if(pData != NULL)
            {
                len = Recorder_Cmd_OneNum[4];
                memcpy(p,RecorderFlash.buf,len);
            } 
        }
        break;
        case REGISTER_TYPE_POWER_ON:
        {
            if(pData != NULL)
            {
                len = Recorder_Cmd_OneNum[5];
                memcpy(p,RecorderFlash.buf,len);
            }
        }
        break;
        case REGISTER_TYPE_PRAMATER:
        {
            if(pData != NULL)
            {
                len = Recorder_Cmd_OneNum[6];
                memcpy(p,RecorderFlash.buf,len);
            }
        }
        break;
        case REGISTER_TYPE_SPEED_STATUS:
        {
            if(pData != NULL)
            {
                len = Recorder_Cmd_OneNum[7];
                memcpy(p,RecorderFlash.buf,len);
            }
        }
        break;        
    }
    if((p != NULL)&&(PackNum == DataBlockAll))  
    {
        return len;
    }
    return 0;
}

/**
* @brief  ���ݿ�ʼʱ�����ʱ�����ʱ������ܰ�������ȡָ�����ݰ�����
* @param  tpye:��ѯ����
* @param  Start:��ʼʱ��
* @param  End:����ʱ��
* @param  PackNum:Ҫ���ҵ����ݰ��ţ�ֻ��pData != NULLʱ����Ч
* @param  *pData:ָ����ȡ�����ݵ�ַ��
* @retval ��pData==NULLʱ�����ݰ��������򷵻����ݳ���
*/
u16 RecorderDataPack(u8 Type, TIME_T Start, TIME_T End,u16 PackNum,u8 *pData)
{
    u16 len_return    = 0;                                 //�������ص����ݳ���
    u32 StartCnt      = 0;                                 //32λ��ʼʱ��
    u32 EndCnt        = 0;                                 //32λ����ʱ��                    
    
	u16	SearchSector  = Register[Type].CurrentSector;      //��ǰ����
	u16	SearchStep    = Register[Type].CurrentStep;        //��ǰ����
    u16	SearchStepLen = Register[Type].StepLen;            //���� 

	u32	SearchAddress = SearchSector*FLASH_ONE_SECTOR_BYTES//��ǰ������ַ 
	                    + SearchStep*SearchStepLen;                   

    u32 AddrStart     = Register[Type].StartSector * FLASH_ONE_SECTOR_BYTES;
    
    u8  *p;//8λָ��,��Ҫ���ڴ��ݴ������ݵ�ַ
    
    StartCnt = ConverseGmtime(&Start);
    EndCnt   = ConverseGmtime(&End);
	
	if(StartCnt >= EndCnt)//�ж�ʱ���Ⱥ��ϵ
	{
		return 0;//���ش���
	}	
                 
    p = (u8*)&RecorderFlash;
    DataBlockWatch    = 0;
    DataBlockAll      = 0;
    SearchTimeCnt     = 0;
    memset((u8*)&SpeedPosTime,0,sizeof(TIME_T));
    memset((u8*)&SearchTime,0,sizeof(TIME_T));
    for(;SearchAddress >= AddrStart;)//ѭ������
    {     
    	if(SearchStep == 0)
    	{       		
            if(SearchSector == Register[Type].OldestSector)
            {
                break;//�ҵ���������
            }
    		SearchSector--;
    		if(SearchSector < Register[Type].StartSector)
    		{
    			SearchSector = Register[Type].EndSector;
    		}
            SearchStep = Register[Type].SectorStep;
    	} 
        SearchStep--;
        SearchAddress = SearchSector * FLASH_ONE_SECTOR_BYTES
                        + SearchStep * SearchStepLen; 
     
        sFLASH_ReadBuffer(p,SearchAddress,SearchStepLen);//��ȡʱ��+����+У��	
      	SearchTimeCnt = Register_BytesToInt(RecorderFlash.time);
      	Gmtime(&SearchTime, SearchTimeCnt);
        DataBlockWatch++;
        if((SearchTimeCnt <= EndCnt)&&(SearchTimeCnt >= StartCnt))//С�ڽ���ʱ���Ҵ��ڿ�ʼʱ�䣬����������
        {        
            len_return = RecorderTotal(Type,PackNum,pData);
            if(len_return != 0)
            {                   
                return len_return;//�������ݳ���
            }
        }
        IWDG_ReloadCounter();
    }
    if(DataBlockSpeedPos)
    {
        DataBlockSpeedPos = 0;
        DataBlockAll++;
        if((pData != NULL)&&(PackNum == DataBlockAll))
        {
            if(Type == REGISTER_TYPE_SPEED)
            {
                return 126;
            }
            else if(Type == REGISTER_TYPE_POSITION)
            {
                return 666;
            }
        }
    }
    return (DataBlockAll);//�����ܰ���
}

/**
* @brief  ���ݿ�ʼʱ�����ʱ�����ʱ������ܰ�������ȡָ�����ݰ�����,USBר��
* @param  tpye:��ѯ����
* @param  Start:��ʼʱ��
* @param  End:����ʱ��
* @param  PackNum:Ҫ���ҵ����ݰ��ţ�ֻ��pData != NULLʱ����Ч
* @param  *pData:ָ����ȡ�����ݵ�ַ��
* @retval ��pData==NULLʱ�����ݰ��������򷵻����ݳ���
*/
u16 RecorderDataPackUsb(u8 Type, TIME_T Start, TIME_T End,u16 PackNum,u8 *pData)
{
    u16 len_return    = 0;                                 //�������ص����ݳ���
    u32 StartCnt      = 0;                                 //32λ��ʼʱ��
    u32 EndCnt        = 0;                                 //32λ����ʱ��                    

	u16	SearchSector  = RecorderFlashAddr.Sector;      //��ǰ����
	u16	SearchStep    = RecorderFlashAddr.Step;        //��ǰ����
    u16	SearchStepLen = RecorderFlashAddr.StepLen;     //����     

	u32	SearchAddress = SearchSector*FLASH_ONE_SECTOR_BYTES//��ǰ������ַ 
	                    + SearchStep*SearchStepLen;                   

    u32 AddrStart     = Register[Type].StartSector * FLASH_ONE_SECTOR_BYTES;
    
    u8  *p;//8λָ��,��Ҫ���ڴ��ݴ������ݵ�ַ
    
    StartCnt = ConverseGmtime(&Start);
    EndCnt   = ConverseGmtime(&End);
	
	if(StartCnt >= EndCnt)//�ж�ʱ���Ⱥ��ϵ
	{
		return 0;//���ش���
	}	
                 
    p = (u8*)&RecorderFlash;
    DataBlockWatch    = 0;
    SearchTimeCnt     = 0;
    memset((u8*)&SpeedPosTime,0,sizeof(TIME_T));
    memset((u8*)&SearchTime,0,sizeof(TIME_T));
    for(;SearchAddress >= AddrStart;)//ѭ������
    {     
    	if(SearchStep == 0)
    	{       		
            if(SearchSector == Register[Type].OldestSector)
            {
                break;//�ҵ���������
            }
    		SearchSector--;
    		if(SearchSector < Register[Type].StartSector)
    		{
    			SearchSector = Register[Type].EndSector;
    		}
            SearchStep = Register[Type].SectorStep;
    	} 
        SearchStep--;   
        SearchAddress = SearchSector * FLASH_ONE_SECTOR_BYTES
                        + SearchStep * SearchStepLen; 

        RecorderFlashAddr.Sector = SearchSector;
        RecorderFlashAddr.Step   = SearchStep;
        RecorderFlashAddr.StepLen= SearchStepLen;
     
        sFLASH_ReadBuffer(p,SearchAddress,SearchStepLen);//��ȡʱ��+����+У��	
      	SearchTimeCnt = Register_BytesToInt(RecorderFlash.time);
      	Gmtime(&SearchTime, SearchTimeCnt);
        DataBlockWatch++;
        if((SearchTimeCnt <= EndCnt)&&(SearchTimeCnt >= StartCnt))//С�ڽ���ʱ���Ҵ��ڿ�ʼʱ�䣬����������
        {        
            len_return = RecorderTotal(Type,PackNum,pData);
            if(len_return != 0)
            {                   
                return len_return;//�������ݳ���
            }
        }
        IWDG_ReloadCounter();
    }
    if(DataBlockSpeedPos)
    {
        DataBlockSpeedPos = 0;
        DataBlockAll++;
        if((pData != NULL)&&(PackNum == DataBlockAll))
        {
            if(Type == REGISTER_TYPE_SPEED)
            {
                return 126;
            }
            else if(Type == REGISTER_TYPE_POSITION)
            {
                return 666;
            }
        }
    }
    return (DataBlockAll);//�����ܰ���
}
/**
* @brief  GPRS�������ݵ��ܰ���
* @param  tpye:��ѯ����
* @param  Start:��ʼʱ��
* @param  End:����ʱ��
* @param  *pRemain:ָ��ְ�������
* @retval �����ݰ���
*/
u16 RecorderWireSendNum(u8 Type, TIME_T Start, TIME_T End,u16 *pRemain)
{
    u16 returnpack;
    u16 allpack;
    u16 remainpack;    
    u8  cmdpacknum=0;

    allpack = RecorderDataPack(Type,Start,End,0,NULL);
    cmdpacknum = Type - 1;     
#ifdef HUOYUN
    u32  datalen = Recorder_Cmd_OneNum[cmdpacknum]*allpack;//�����ݿ��ֽ���
    returnpack = datalen/900;
    remainpack = datalen%900;
    if(remainpack)
    {
        returnpack++;
        *pRemain = remainpack;
    }    
#else
    returnpack = allpack/Recorder_Cmd_PackNum[cmdpacknum];
    remainpack = allpack%Recorder_Cmd_PackNum[cmdpacknum];
    if(remainpack)
    {
        returnpack++;
        *pRemain = remainpack;
    }
#endif
    
    return returnpack; 
}

/**
* @brief  ͨ���ܰ����͵�ǰ������������
* @param  tpye:��ѯ����
* @param  TotalPackage:�ܰ���
* @param  CurrentPackage:��ǰ����
* @param  *pData:ָ��ǰ����������
* @retval �������ݳ���
*/
u16 RecorderWireSend(u8 type,u16 Count,u8 *pData) 
{
    TIME_T start = St_RecorderWire.Time_start;
    TIME_T end   = St_RecorderWire.Time_end;

#ifndef   HUOYUN 
    u16 i;
    u16 searchcnt;
    u16 ForPack; 
#endif   
    u16 Len=0;//���ݿ鳤��
    u8  *pSend=pData;
    u8  cmdpacknum=0;
 
    switch(type)
    {
        case 0x08:
        {
            cmdpacknum = 0;
        }
        break;
        case 0x09:
        {
            cmdpacknum = 1;
        }
        break;
        case 0x10:
        {
            cmdpacknum = 2;
        }
        break;
        case 0x11:
        {
            cmdpacknum = 3;
        }
        break;
        case 0x12:
        {
            cmdpacknum = 4;
        }
        break;
        case 0x13:
        {
            cmdpacknum = 5;
        }
        break;
        case 0x14:
        {
            cmdpacknum = 6;
        }
        break;  
        case 0x15:
        {
            cmdpacknum = 7;
        }
        break;        
    }

#ifdef HUOYUN
    u16 HY_addr;
    u16 HY_remain=St_RecorderWire.HY_Cmd_SendRemain;
    if(HY_remain)
    {
        if(HY_remain > Recorder_Cmd_OneNum[cmdpacknum])
        {
            RecorderDataPack(RecorderCmdToRegister(type),start,end,
                             St_RecorderWire.HY_Cmd_SendNum-1,pSend);
            HY_remain = HY_remain%Recorder_Cmd_OneNum[cmdpacknum];
            Len = HY_remain;
            HY_addr = Recorder_Cmd_OneNum[cmdpacknum] - Len;
            memmove(pSend,pSend+HY_addr,Len);//��������     
            RecorderDataPack(RecorderCmdToRegister(type),start,end,
                             St_RecorderWire.HY_Cmd_SendNum,pSend+Len);
            Len = Len + Recorder_Cmd_OneNum[cmdpacknum];   
            St_RecorderWire.HY_Cmd_SendNum++;
            if(St_RecorderWire.AllCnt == Count)
            {
                St_RecorderWire.HY_Cmd_SendRemain = 0;
                return Len;
            }                 
        }
        else
        {
            RecorderDataPack(RecorderCmdToRegister(type),start,end,St_RecorderWire.HY_Cmd_SendNum,pSend);
            Len = St_RecorderWire.HY_Cmd_SendRemain;
            HY_addr = Recorder_Cmd_OneNum[cmdpacknum] - St_RecorderWire.HY_Cmd_SendRemain;
            memmove(pSend,pSend+HY_addr,Len);//��������  
            St_RecorderWire.HY_Cmd_SendNum++;
            if(St_RecorderWire.AllCnt == Count)
            {
                St_RecorderWire.HY_Cmd_SendRemain = 0;
                return Len;
            }
        }
      

    }
    for(;;)
    {
        RecorderDataPack(RecorderCmdToRegister(type),start,end,St_RecorderWire.HY_Cmd_SendNum,pSend+Len);
        Len = Len + Recorder_Cmd_OneNum[cmdpacknum];
        if((St_RecorderWire.AllCnt == 1)&&(St_RecorderWire.AllRemainCnt == Len))
        {
            return Len;
        }
        if(Len > 900)
        {          
            St_RecorderWire.HY_Cmd_SendRemain = (Len%900);
            if(Count == 1)
            {
                St_RecorderWire.HY_Cmd_SendRemain=St_RecorderWire.HY_Cmd_SendRemain+9;
            }
            Len = Len - St_RecorderWire.HY_Cmd_SendRemain;
            return Len;
        }
        else
        {
            St_RecorderWire.HY_Cmd_SendNum++;
        }
    }  
#else 
    if((St_RecorderWire.AllCnt == Count)&&(St_RecorderWire.AllRemainCnt != 0)
        &&(St_RecorderWire.AddOverflg == 0))
    {
        ForPack = St_RecorderWire.AllRemainCnt;
    }
    else
    {
        ForPack = Recorder_Cmd_PackNum[cmdpacknum];
    }
    searchcnt = Recorder_Cmd_PackNum[cmdpacknum]*(Count-1);
    for(i=1;i<=ForPack;i++)
    {
        RecorderDataPack(RecorderCmdToRegister(type),start,end,searchcnt+i,pSend+Len);
        Len = Len + Recorder_Cmd_OneNum[cmdpacknum];
    }    
    return Len;
#endif
}

/*********************************************************************
//��������	:RecorderCom_WirelessTimeTask
//����		:������ʻ��¼�Ƕ�ʱ����
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
*********************************************************************/
FunctionalState RecorderWireless_TimeTask(void)
{

    MESSAGE_HEAD	head;
    u16 Datalen=0;
    u8	SmsPhone[20]={0};
    u8  *pData808=NULL;//808Э�����ݷ���ָ��
    u8 channel = CHANNEL_DATA_1;
    u16 SerialNum = 0;
    
    channel = RadioProtocol_GetDownloadCmdChannel(0x8700, &SerialNum);
    
    memset(&head,0,sizeof(MESSAGE_HEAD));                    //��սṹ��
    memset(&RecorderHead,0,sizeof(ST_RECORDER_HEAD));
    
    RecorderHead.cmd808     = St_RecorderWire.Cmd;            //������
    RecorderHead.acknum808H = St_RecorderWire.AckNum>>8;      //Ӧ����ˮ��
    RecorderHead.acknum808L = St_RecorderWire.AckNum&0x00ff;
    RecorderHead.head1      = 0x55;                           //֡ͷ
    RecorderHead.head2      = 0x7a;                           //֡ͷ 
    RecorderHead.cmd        = St_RecorderWire.Cmd;            //������
    RecorderHead.back       = PRO_DEF_RETAINS;                //������       

    head.MessageID          =  0x0700;//��¼�������ϴ�����  
    
    if(RecorderCheckCmd(St_RecorderWire.Cmd))
    {
        if(St_RecorderWire.Cmd <= 0x07)                           //�޲�������
        {
            Datalen = RecorderData0_7(St_RecorderWire.Cmd,RecorderHead.DataBuf);
            RecorderHead.lenH = Datalen>>8;
            RecorderHead.lenL = Datalen&0x00ff;
        }
        else if(((St_RecorderWire.Cmd >= 0x82)&&(St_RecorderWire.Cmd <= 0x84))  
               ||((St_RecorderWire.Cmd >= 0xC2)&&(St_RecorderWire.Cmd <= 0xC4))) 
        {
            RecorderDataSet(St_RecorderWire.Cmd,St_RecorderWire.Buf, St_RecorderWire.Len);//��������
        }
        else//�в�������,�����п�ʼʱ�䡢����ʱ�䡢����������
        {
            if(St_RecorderWire.AllCnt)
            {
                Datalen = RecorderWireSend(St_RecorderWire.Cmd,
                                           St_RecorderWire.AddCnt,
                                           RecorderHead.DataBuf);   

#ifdef HUOYUN
                if((St_RecorderWire.AddCnt == 1)&&(St_RecorderWire.AllCnt>1))
                {
                    RecorderHead.lenH  = ((900*St_RecorderWire.AllCnt)+
                                         St_RecorderWire.AllRemainCnt)>>8;
                    RecorderHead.lenL  = ((900*St_RecorderWire.AllCnt)+
                                         St_RecorderWire.AllRemainCnt)&0xff;
                } 
                else
                {
                    RecorderHead.lenH = Datalen>>8;
                    RecorderHead.lenL = Datalen&0x00ff;
                }
#else       
                RecorderHead.lenH = Datalen>>8;
                RecorderHead.lenL = Datalen&0x00ff;
#endif            
            }
        }
        if((St_RecorderWire.AllCnt == 1)&&(Datalen < 900))
        {
            WireSendVer = 0;
            RecorderXorVer(&WireSendVer,(u8*)&RecorderHead.head1,Datalen+6);//������С��600�򲻽��зְ�
            RecorderHead.DataBuf[Datalen] = WireSendVer;
        }
        else
        {
            head.TotalPackage   =  St_RecorderWire.AllCnt;//�ܰ���   
            head.CurrentPackage =  St_RecorderWire.AddCnt;//��ǰ����
        }
    }
    else
    {
        Datalen = 0;
        head.TotalPackage = 0;
        head.CurrentPackage = 0;
    }
    if(head.TotalPackage)
    {
#ifdef HUOYUN
        if(head.CurrentPackage == 1)//��һ��������Ҫ����: ������ + Ӧ����ˮ��
        {
            WireSendVer           = 0;     
            SubpacketUploadFlag   = 1;//������ˮ�ŵ�������
            SubpackageCurrentSerial = 1;//��ˮ�Ŵ�1��ʼ        
            pData808              = (u8*)&RecorderHead.head1;
            RecorderXorVer(&WireSendVer,pData808,Datalen+6); //У��
            if(St_RecorderWire.AllCnt == St_RecorderWire.AddCnt)
            {
                *(pData808+Datalen+6)    = WireSendVer;
                Datalen++;
            }
            head.Attribute         = (Datalen+9)|0x2000;
            pData808               = (u8*)&RecorderHead;         
        }
        else
        { 
            if(St_RecorderWire.AllCnt == St_RecorderWire.AddCnt)
            {
                pData808               = (u8*)&RecorderHead.DataBuf;
                RecorderXorVer(&WireSendVer,pData808,Datalen);
                *(pData808+Datalen)    = WireSendVer;
                head.Attribute         = (Datalen+1)|0x2000;               
            }
            else
            {
                pData808               = (u8*)&RecorderHead.DataBuf;
                RecorderXorVer(&WireSendVer,pData808,Datalen);
                head.Attribute         = Datalen|0x2000;
            }
        }
#else
        if(head.CurrentPackage == 1)//��һ��������Ҫ����: ������ + Ӧ����ˮ��
        {
            WireSendVer           = 0;     
            SubpacketUploadFlag   = 1;//������ˮ�ŵ�������
            SubpackageCurrentSerial = 1;//��ˮ�Ŵ�1��ʼ
            
            pData808              = (u8*)&RecorderHead.head1;
            RecorderXorVer(&WireSendVer,pData808,Datalen+6); //У��
            *(pData808+Datalen+6)    = WireSendVer;
            head.Attribute         = (Datalen+10)|0x2000;
            pData808               = (u8*)&RecorderHead;         
        }
        else
        { 
            WireSendVer = 0;  
            pData808               = (u8*)&RecorderHead.head1;
            RecorderXorVer(&WireSendVer,pData808,Datalen+6);
            *(pData808+Datalen+6)    = WireSendVer;
            head.Attribute         = (Datalen+7)|0x2000;
        }
#endif
    }
    else
    {
        if(RecorderCheckCmd(St_RecorderWire.Cmd))
        {
            head.Attribute = Datalen+10;//�����зְ�����
            pData808 = (u8*)&RecorderHead;
        }
        else
        {
            if(St_RecorderWire.Cmd808 == 0x8700)
            {
                head.Attribute = RecorderCreateErrCmd(0,(u8*)&RecorderHead.head1)+3;//�ɼ�����
            }
            else
            {
                head.Attribute = RecorderCreateErrCmd(1,(u8*)&RecorderHead.head1)+3;//���ô���
            }
            pData808 = (u8*)&RecorderHead;
        }
    }
    
    if(0 != RadioProtocol_SendCmd(channel,head,pData808,(head.Attribute&0X03FF),SmsPhone)) 
    {           
        if(Recorder_CheckAll07()||Recorder_CheckPackage())
        {
            return ENABLE;
        }
        else
        {
            return DISABLE;
        }
    }
    else
    {    
        SetTimerTask(TIME_WIRELESS_RECORDER, 30*SYSTICK_100ms);//�������߼�¼�Ƕ�ʱ����      
        return ENABLE;
    }

}

/*******************************************************************************
* Function Name  : RecorderWired_TimeTask
* Description    : ��¼���¼�����,100ms����һ��
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
FunctionalState RecorderWired_TimeTask(void)
{
    u16 nRecLen = 0;
    u8  nRecTab[512];

    ////////////////////////////////////////////////////////////////////////////����
    nRecLen = COM1_ReadBuff(nRecTab,512);  
    
    #ifdef EYE_MODEM
    Modem_Debug_UartRx(nRecTab,nRecLen);
    #endif

    ////////////////////////////////////////////////////////////////////////////����
    Recorder_SendOutTime();//���ʹ��������
    
    ////////////////////////////////////////////////////////////////////////////һ֡����
    if(Recorder_SerOutTime(nRecTab,nRecLen) == 0)
    {
        return ENABLE;
    }
    
    ////////////////////////////////////////////////////////////////////////////���ݴ���
    ///// ���յ�����һ֡���� -> St_RecorderQueueRx.nBuf
    ///// ..............���� -> St_RecorderQueueRx.in

    ///// ����Э�鴦��
    CarLoad_CommProtocolParse(St_RecorderQueueRx.nBuf,St_RecorderQueueRx.in);

    /////��¼��Э�鴦��,��Э����ŵ����
    if(Recorder_Analyse(St_RecorderQueueRx.nBuf,St_RecorderQueueRx.in))
    {
         Recorder_Handle();
    }

    ////////////////////////////////////////////////////////////////////////////���㻺��
    SerBusyFlag           = 0;//�������
    St_RecorderQueueRx.in = 0;//����������
    
    return ENABLE;
}




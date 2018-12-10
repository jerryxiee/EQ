/************************************************************************
//�������ƣ�Can_ErQi.c
//���ܣ���ģ��ʵ�ֹ��ݶ���CAN�����ϴ����ܡ�
//�汾�ţ�V0.1
//��Ȩ˵������Ȩ�����������������¼����������޹�˾
//�����ˣ�dxl
//����ʱ�䣺2014.12
//��ע��CAN���ߵ����ӹ�ϵ�ǳ���CAN�����Ƚӵ���3����CAN���ӣ������ۻ�ŷ�Ƽѣ���Ȼ���ٽ�������CAN���ӣ�������EGS701GB�նˡ�
//ֻ������CAN������EGS701GB�������Ǵ������ӣ�������������CAN���ߣ�����CAN������EGS701GB���ӵĴ���ΪUSART2
//����CAN���ӵ������ǣ�1.��CAN���ݽ��б�Ҫ���˲����˳�����Ҫ��CAN ID��������ͬ��CAN ID��
//2.�ѵ�3����CAN����ת����������ͳһ��ʽ������ʹ��ͬһ̨�ն˿��Խ����ҵ�CAN���ӣ�
//3.���з��͵�CAN���ݷֳ����࣬һ����ģ��������һ������������ģ�����Ƕ�ʱ���͸��նˣ��������Ǳ仯�˾ͷ��͸��նˡ�
//�汾��¼���汾��¼������汾���޸��ˡ��޸�ʱ�䡢�޸�ԭ���޸ļ�Ҫ˵��
//V0.1 ��ʼ�汾  ������
//V0.2 ԭ�����յ�һ֡CAN���ӷ������ݾ�͸��һ֡��ƽ̨�����ڸ�Ϊ��ʱ��������֡��ƽ̨����ʱʱ��Ϊ10�롣������Ϊ��������Ҫ����Ҫ�ǵ����������꣩
*************************************************************************/

/********************�ļ�����*************************/
#include <stdio.h>
#include <string.h>

#include "Can_ErQi.h"
#include "Usart2.h"
#include "GPIOControl.h"
#include "modem_lib.h"
#include "modem_app_com.h"
#include "Report.h"
#include "RadioProtocol.h"
#include "other.h"
#include "EepromPram.h"
#include "Public.h"
#include "NaviLcd.h"


/********************���ر���*************************/
static u8 CanErQiUploadBuffer[CAN_ERQI_UPLOAD_BUFFER_SIZE];
static u8 CanErQiCmdSendBuffer[CAN_ERQI_CMD_BUFFER_SIZE];
static u8 CanErQiSetRtcAckFlag = 0;//����RTCӦ���־��1Ϊ�յ�CANģ��Ӧ��
static u8 CanErQiDigitalBuffer[CAN_ERQI_UPLOAD_BUFFER_SIZE];//�洢CANģ�鷢�͵�OF03��������
static u8 CanErQiAnalogBuffer[CAN_ERQI_UPLOAD_BUFFER_SIZE];//�洢CANģ�鷢�͵�OF04��������
static u16 CanErQiDigitalBufferLen = 0;
static u16 CanErQiAnalogBufferLen = 0;
/********************ȫ�ֱ���*************************/


/********************�ⲿ����*************************/


/********************���غ�������*********************/
static void Can_ErQi_ParseCmd(u8 *pBuffer, u16 BufferLen);
static void Can_ErQi_Upload(u16 CanId,u8 *pBuffer, u16 BufferLen);

/********************��������*************************/
/**************************************************************************
//��������Can_ErQi_Init
//���ܣ�CAN�����ʼ��
//���룺��
//�������
//����ֵ����
//��ע����
***************************************************************************/
void Can_ErQi_Init(void)
{
	u8 Buffer[5];
	u8 BufferLen;
	u16 temp;

	BufferLen = EepromPram_ReadPram(E2_POS_PORT_ID,Buffer);
	if(E2_POS_PORT_ID_LEN == BufferLen)
	{
		temp = Public_ConvertBufferToShort(Buffer);
	}
	if(0x02 == (temp&0x02))
	{
		NaviLcdPowerOn();
	}
}
/**************************************************************************
//��������Can_ErQi_TimeTask
//���ܣ�CAN��ʱ����
//���룺��
//�������
//����ֵ����
//��ע��ʵ�ֶ�CAN����Уʱ��ÿ���ն���������1�ε������CAN����Уʱ������ʱ��1��1��
***************************************************************************/
FunctionalState Can_ErQi_TimeTask(void)
{
	static u8 TimingCount = 0;
        static u8 UploadCount = 0;
	u8 flag = 0;
	u8 Buffer[10] = {0};
	u16 year;
	TIME_T time;


	flag = Io_ReadStatusBit(STATUS_BIT_NAVIGATION);//�ϵ絼����Уʱ1��
	if(1 == flag)
	{
		TimingCount++;
		if(TimingCount > 3)
		{
			TimingCount = 0;
			if(0 == CanErQiSetRtcAckFlag)
			{
				RTC_GetCurTime(&time);
				year = 2000+time.year;
				Buffer[0] = year&0xff;
				Buffer[1] = (year&0xff00) >> 8;
				Buffer[2] = time.month;
				Buffer[3] = time.day;
				Buffer[4] = time.hour;
				Buffer[5] = time.min;
				Buffer[6] = time.sec;
				Can_ErQi_SendCmd(0x61,Buffer,7);
			}
			else
			{
                          
			}
		}
	}
	else
	{
		TimingCount = 0;
	}
        
        //��ʱ��ƽ̨����CAN���ݣ�0F03��OF04��
        UploadCount++;
        if(UploadCount == 10)//����OF03��ƽ̨
        {
                if(CanErQiDigitalBufferLen > 0)
                {
                        Can_ErQi_Upload(0x0F03,CanErQiDigitalBuffer,CanErQiDigitalBufferLen);
                        CanErQiDigitalBufferLen = 0;
                }
        }
        else if(UploadCount >= 11)//����OF04��ƽ̨
        {
                if(CanErQiAnalogBufferLen > 0)
                {
                        Can_ErQi_Upload(0x0F04,CanErQiAnalogBuffer,CanErQiAnalogBufferLen);
                        CanErQiAnalogBufferLen = 0;
                }
                UploadCount = 0;
        }
	return ENABLE;
}
/**************************************************************************
//��������Can_ErQi_Parse
//���ܣ�CAN����Э�����
//���룺��
//�������
//����ֵ����
//��ע����
***************************************************************************/
void Can_ErQi_Parse(u8 *pBuffer, u16 BufferLen)
{
	u8 *p = NULL;
	u16 i = 0;
	u16 j = 0;
	u16 k = 0;
	u16 length = 0;
	u8 VerifyCode = 0;

	if(NULL != p)
	{
		return ;
	}

	p = pBuffer;
	for(i=0; i<BufferLen-1; i++)
	{
		if((0x55 == *(p+i))&&(0xaa == *(p+i+1)))
		{
			j = i;
			length = (*(p+j+2) << 8)|(*(p+j+3));
			i += (length+4);
			if((length < CAN_ERQI_UPLOAD_BUFFER_SIZE)&&(length < BufferLen))
			{
				p = pBuffer+j+2;
				VerifyCode = 0;
				for(k=0; k<length+1; k++)
				{
					VerifyCode += *(p+k);
				}
				if(VerifyCode == *(p+k))
				{
					Can_ErQi_ParseCmd(pBuffer+j+4,length);
				}
			}
		}
	}
}
/**************************************************************************
//��������Can_ErQi_ParseCmd
//���ܣ�CAN�����������
//���룺pBuffer:ָ��������+�������׵�ַ��BufferLen:������+�����ĳ���
//�������
//����ֵ����
//��ע����
***************************************************************************/
static void Can_ErQi_ParseCmd(u8 *pBuffer, u16 BufferLen)
{
	u8 Command;
	u8 Buffer[1] = {0};
	
	Command = *pBuffer;
	switch(Command)
	{
		case 0x40://��ȡCANģ��汾��Ϣ�������CANӦ����Ϣ
		{
			break;
		}
		case 0x60://�������ã������CANӦ����Ϣ
		{
			break;
		}
		case 0x61://����CANģ��RTC
		{
			CanErQiSetRtcAckFlag = 1;
			break;
		}
		case 0x6E://��ȡCANģ�����б�
		{
			//Can_ErQi_Upload(0x0F04,pBuffer,BufferLen);�յ�һ�������ϴ���ƽ̨
                        if(BufferLen < CAN_ERQI_UPLOAD_BUFFER_SIZE)
                        {
                                memcpy(CanErQiAnalogBuffer,pBuffer,BufferLen);
                                CanErQiAnalogBufferLen = BufferLen;
                        }
                        
			break;
		}
		case 0x6F://��ȡCAN�������б�
		{
			//Can_ErQi_Upload(0x0F03,pBuffer,BufferLen);�յ�һ�������ϴ���ƽ̨
                        if(BufferLen < CAN_ERQI_UPLOAD_BUFFER_SIZE)
                        {
                                memcpy(CanErQiDigitalBuffer,pBuffer,BufferLen);
                                CanErQiDigitalBufferLen = BufferLen;
                        }
                  
			Can_ErQi_SendCmd(0xA5, Buffer, 1);//�յ�������������Ӧ��CANģ��
			break;
		}
		default:break;
	}
}
/**************************************************************************
//��������Can_ErQi_SendCmd
//���ܣ��������CAN����
//���룺Cmd:�����֣�pBuffer:ָ��������������ֺ���������ݣ��׵�ַ��BufferLen:�������� 
//�������
//����ֵ��0:�ɹ���1:ʧ�ܣ�2:���ȴ���
//��ע����
***************************************************************************/
u8 Can_ErQi_SendCmd(u8 Cmd, u8 *pBuffer, u16 BufferLen)
{
	u16 i;
	u8 VerifyCode = 0;
	u16 length;
	
	if(BufferLen > (CAN_ERQI_CMD_BUFFER_SIZE-3))
	{
		return 2;
	}
	length = BufferLen+2;
	CanErQiCmdSendBuffer[0] = 0x55;
	CanErQiCmdSendBuffer[1] = 0xaa;
	CanErQiCmdSendBuffer[2] = (length&0xff00) >> 8;
	CanErQiCmdSendBuffer[3] = length&0xff;//���Ȱ�������+����+У��
	CanErQiCmdSendBuffer[4] = Cmd;
	memcpy(CanErQiCmdSendBuffer+5,pBuffer,BufferLen);
	VerifyCode = 0;
	for(i=0; i<BufferLen+3; i++)//У�������ݳ���+����+����
	{
		VerifyCode += CanErQiCmdSendBuffer[2+i];
	}
	CanErQiCmdSendBuffer[5+BufferLen] = VerifyCode;
	
	if(0 == COM2_WriteBuff(CanErQiCmdSendBuffer,BufferLen+6))
	{
		return 0;
	}
	else
	{
		return 1;
	}
		
}
/**************************************************************************
//��������Can_ErQi_Upload
//���ܣ�����CAN���ݵ�ƽ̨
//���룺pBuffer:ָ��������+�������׵�ַ��BuffeLen:������+������ɵĻ��峤��
//�������
//����ֵ����
//��ע��CanId:CANģ���Զ�����ϢID��0x0F03Ϊ�������б�0x0F04Ϊģ�����б�
//0x0F05Ϊ������0x0F06ΪRTC��������û�����ϴ�
***************************************************************************/
static void Can_ErQi_Upload(u16 CanId,u8 *pBuffer, u16 BufferLen)
{
	u8 Buffer[40];
	u8 length;
	//TIME_T time;
	//u16 temp;
	u8 channel = CHANNEL_DATA_1|CHANNEL_DATA_2;

	switch(CanId)
	{
		case 0x0F03:
		{
			if(*pBuffer == 0x6F)
			{
				CanErQiUploadBuffer[0] = 0x0f;
				CanErQiUploadBuffer[1] = 0x03;
				length = Report_GetPositionBasicInfo(Buffer);//����λ����Ϣ�̶�Ϊ28�ֽ�
				memcpy(CanErQiUploadBuffer+2,Buffer,length);
				//RTC_GetCurTime(&time);
				//temp = 2000;
				//temp += time.year;
				length += 2;//ǰ��2�ֽ�
				CanErQiUploadBuffer[length++] = '0';//ԭЭ����ASCII[7]��Ӧ�������⣬�������'0'
				CanErQiUploadBuffer[length++] = '0';
				CanErQiUploadBuffer[length++] = '0';
				CanErQiUploadBuffer[length++] = '0';
				CanErQiUploadBuffer[length++] = '0';
				CanErQiUploadBuffer[length++] = '0';
				CanErQiUploadBuffer[length++] = '0';
				memcpy(CanErQiUploadBuffer+length,pBuffer,BufferLen);
				length += BufferLen;
				RadioProtocol_OriginalDataUpTrans(channel,0xF1,CanErQiUploadBuffer,length);
			}
			break;
		}
		case 0x0F04:
		{
			if(*pBuffer == 0x6E)
			{
				CanErQiUploadBuffer[0] = 0x0f;
				CanErQiUploadBuffer[1] = 0x04;
				length = Report_GetPositionBasicInfo(Buffer);//����λ����Ϣ�̶�Ϊ28�ֽ�
				memcpy(CanErQiUploadBuffer+2,Buffer,length);
				//RTC_GetCurTime(&time);
				//temp = 2000;
				//temp += time.year;
				length += 2;//ǰ��2�ֽ�
				CanErQiUploadBuffer[length++] = '0';//ԭЭ����ASCII[7]��Ӧ�������⣬�������'0'
				CanErQiUploadBuffer[length++] = '0';
				CanErQiUploadBuffer[length++] = '0';
				CanErQiUploadBuffer[length++] = '0';
				CanErQiUploadBuffer[length++] = '0';
				CanErQiUploadBuffer[length++] = '0';
				CanErQiUploadBuffer[length++] = '0';
				memcpy(CanErQiUploadBuffer+length,pBuffer,BufferLen);
				length += BufferLen;
				RadioProtocol_OriginalDataUpTrans(channel,0xF1,CanErQiUploadBuffer,length);
			}
			break;
		}
		case 0x0F05://�ݲ�֧��
		{
			break;
		}
		case 0x0F06://�ݲ�֧��
		{
			break;
		}
		default:break;
	}
}






















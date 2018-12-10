/********************************************************************
//��Ȩ˵��	:Shenzhen E-EYE Co. Ltd., Copyright 2009- All Rights Reserved.
//�ļ�����	:JTReport.c		
//����		:ʵ�־�ͨ�������ϱ�����
//�汾��	:
//������	:dxl
//����ʱ��	:2013.11.28 
//�޸���	:
//�޸�ʱ��	:
//�޸ļ�Ҫ˵��	:
//��ע		:���趨���������о�������ǿ�������Ϊ��������״̬��
//                ��������趨���������о�������ǿ�����������Ϊ���쳣��״̬��
//                �����쳣����5���ӻ㱨1��λ����Ϣ��ÿ�������ͨ����ID0xe4
//                1���㵹��Ҫ�ϱ�һ������ͨ����ID0xe3��λ����Ϣ
***********************************************************************/
//***************�����ļ�*****************
#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "include.h"
//***************���ر���******************
static u32 JTIoStatus = 0;//��ͨ״̬λ��
//****************ȫ�ֱ���*****************

//***************��������********************
/*********************************************************************
//��������	:JTReport_TimeTask
//����		:��ͨλ�û㱨����ͨ������Ӧ�ã�
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
*********************************************************************/
FunctionalState JTReport_TimeTask(void)
{
      u8  Status1 = 0;//���״̬
      u8  Status2 = 0;//����״̬
     // u8  Status1Error = 0;//��Ǵ��������ϱ���
     // u8  Status2Error = 0;//�������������ϱ���
      u8  Buffer[FLASH_BLIND_STEP_LEN];
      u8  BufferLen = 0;
      u8  InOutAreaFlag = 0;//�ڻ��������־,1Ϊ���㵹������
      static TIME_T StartTime;//�㵹��ʼʱ��
      static TIME_T EndTime;//�㵹����ʱ��
      static u8 InOutAreaFlag1 = 0;//�㵹��ʼ�Ƿ��������ڵı�־
      static u8  InOutAreaFlag2 = 0;//�㵹�����Ƿ��������ڵı�־
      static u8  LastStatus1 = 0;//��һ�����״̬
      static u8  LastStatus2 = 0;//��һ�ξ���״̬
      static u8  SendFlag1 = 0;//���ͱ�־
      static u8  SendFlag2 = 0;//���ͱ�־
      static u8  Status3 = 0;//�㵹״̬
      static u16 AlarmCount = 0;//��ǻ�����쳣��������
      u8 channel = CHANNEL_DATA_1;//Ϊ����ͨ�������
       
      //�����״̬
      Status1 = Io_ReadSelfDefine2Bit(DEFINE_BIT_9);
      //Status1Error = Io_ReadSelfDefine2Bit(DEFINE_BIT_29);
      //������״̬
      Status2 = Io_ReadSelfDefine2Bit(DEFINE_BIT_10);
      //Status2Error = Io_ReadSelfDefine2Bit(DEFINE_BIT_30);
      //���Ƿ����㵹��
      InOutAreaFlag = MuckDump_GetCurMuckDumpAreaID();
      //�㵹��ʼʱ�̵�,�㵹����ʱ�̵�
      //if((1 == Status1)&&(0 == Status1Error))//��Ǵ��ڿ�������Ǽ�⴫�����޹��ϵ�״̬�£�����������ˮƽλ�õ���������Ϊ���㵹��ʼ�㣬�ɾ������ص�����ˮƽλ��Ϊ�㵹������
     // {
             if((0 == LastStatus2)&&(1 == Status2))//�㵹��ʼ
             {
                      //����ǰʱ��
                      RTC_GetCurTime(&StartTime);
                      //���Ƿ��������ڵ�״̬
                      InOutAreaFlag1 = InOutAreaFlag;
             }
             else if((1 == LastStatus2)&&(0 == Status2))//�㵹����
             {
                      
                      //����ǰʱ��
                      RTC_GetCurTime(&EndTime);
                      //���Ƿ��������ڵ�״̬
                      InOutAreaFlag2 = InOutAreaFlag;
                      
                      if((InOutAreaFlag1)&&(InOutAreaFlag2))//��ʼ�ͽ���ʱ�̶��������ڲ���Ϊ�������㵹������Ϊ�쳣�㵹
                      {
                              Status3 = 0;
                      }
                      else
                      {
                              Status3 = 1;
                      }
                      
                      SendFlag1 = 1;
             }
     // }
      if(1 == SendFlag1)
      {
              //��ȡλ����Ϣ
              BufferLen = Report_GetPositionInfo(Buffer);
             //���Ӿ�ͨ��λ�ø���ID E3
              Buffer[BufferLen++] = 0xE3;//����ID
              Buffer[BufferLen++] = 13;//����ID����
              //����ID����----�㵹״̬
              Buffer[BufferLen++] = Status3;
              //����ID����----�㵹��ʼʱ��
              Buffer[BufferLen++] = Public_HEX2BCD(StartTime.year);
              Buffer[BufferLen++] = Public_HEX2BCD(StartTime.month);
              Buffer[BufferLen++] = Public_HEX2BCD(StartTime.day);
              Buffer[BufferLen++] = Public_HEX2BCD(StartTime.hour);
              Buffer[BufferLen++] = Public_HEX2BCD(StartTime.min);
              Buffer[BufferLen++] = Public_HEX2BCD(StartTime.sec);
              //����ID����----�㵹����ʱ��
              Buffer[BufferLen++] = Public_HEX2BCD(EndTime.year);
              Buffer[BufferLen++] = Public_HEX2BCD(EndTime.month);
              Buffer[BufferLen++] = Public_HEX2BCD(EndTime.day);
              Buffer[BufferLen++] = Public_HEX2BCD(EndTime.hour);
              Buffer[BufferLen++] = Public_HEX2BCD(EndTime.min);
              Buffer[BufferLen++] = Public_HEX2BCD(EndTime.sec);
              //����λ����Ϣ��ƽ̨
              if(ACK_OK == RadioProtocol_PostionInformationReport(channel,Buffer, BufferLen))
	      {
		      SendFlag1 = 0;
	      }
     }
  
      //�㵹״̬���㵹��ʼʱ�̵���㵹����ʱ�̵㶼����������Ϊ���������㵹��������Ϊ���쳣�㵹
      //���״̬�����״̬����ָ�������ڴ�ʱ��Ϊ���쳣���쳣�����5���ӻ㱨1��
     // if((1 == Status1)&&(0 == Status1Error)&&(0 == InOutAreaFlag))
     if((1 == Status1)&&(0 == InOutAreaFlag))
      {
              AlarmCount++;
              if(1 == AlarmCount)
              {
                      SendFlag2 = 1;
              }
              else if(AlarmCount >= 300)
              {
                    AlarmCount = 0;
              }
              JTReport_WriteIoStatusBit(0, SET);
      }
     // else if((1 == Status2)&&(0 == Status2Error)&&(0 == InOutAreaFlag))
      else if((1 == Status2)&&(0 == InOutAreaFlag))
      {
              AlarmCount++;
              if(1 == AlarmCount)
              {
                      SendFlag2 = 1;
              }
              else if(AlarmCount >= 300)
              {
                    AlarmCount = 0;
              }
              JTReport_WriteIoStatusBit(1, SET);
      }
     // else if((0 == Status1)&&(1 == LastStatus1)&&(0 == Status1Error))//����ɿ����أ���������ж���Ϊ�˷�ֹ�����쳣ͬʱ����ʱ���������ʱ�ܿ��ٿ���
      else if((0 == Status1)&&(1 == LastStatus1))
      {
              AlarmCount = 0;
              JTReport_WriteIoStatusBit(0, RESET);
      }
     // else if((0 == Status2)&&(1 == LastStatus2)&&(0 == Status2Error))//�����ɿ�����
      else if((0 == Status2)&&(1 == LastStatus2))//�����ɿ�����
      {
              AlarmCount = 0;
              JTReport_WriteIoStatusBit(1, RESET);
      }
      else
      {
              AlarmCount = 0;
              JTReport_WriteIoStatusBit(0, RESET);
              JTReport_WriteIoStatusBit(1, RESET);
      }
  
      if(1 == SendFlag2)
      {
             //��ȡλ����Ϣ
              BufferLen = Report_GetPositionInfo(Buffer);
             //���Ӿ�ͨ��λ�ø���ID E4
              Buffer[BufferLen++] = 0xE4;//����ID
              Buffer[BufferLen++] = 4;//����ID����
              //����ID����----�㵹��ʼʱ��
              Buffer[BufferLen++] = (JTIoStatus&0xff000000)>>24;
              Buffer[BufferLen++] = (JTIoStatus&0xff0000)>>16;
              Buffer[BufferLen++] = (JTIoStatus&0xff00)>>8;
              Buffer[BufferLen++] = JTIoStatus&0xff;

              //����λ����Ϣ��ƽ̨
              if(ACK_OK == RadioProtocol_PostionInformationReport(channel,Buffer, BufferLen))
	      {
		      SendFlag2 = 0;
	      }
      }
      
      LastStatus1 = Status1;
      LastStatus2 = Status2;
      
  
      return ENABLE;
}
/*********************************************************************
//��������	:JTReport_WriteIoStatusBit
//����		:д��ͨ�Զ����״̬λ��
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:����ID E4���õ�
*********************************************************************/
void JTReport_WriteIoStatusBit(u8 bit, FlagStatus state)
{
	if(bit < 32)
	{
		if(SET==state)
		{
			JTIoStatus |= 1<<bit;
		}
		else if(RESET==state)
		{
			JTIoStatus &= ~(1<<bit);
		}
	
	}
}
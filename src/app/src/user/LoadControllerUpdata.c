/********************************************************************
//��Ȩ˵��	:Shenzhen E-EYE Co. Ltd., Copyright 2009- All Rights Reserved.
//�ļ�����	:LoadControllerUpdata.c		
//����		:���ؿ������̼�����
//�汾��	:
//������	:dxl 
//����ʱ��	:2014.6
//�޸���	:
//�޸�ʱ��	:
//�޸ļ�Ҫ˵��	:
//��ע		:���ؿ�������EGS701GB��DB9���ڶԽ�
***********************************************************************/
/*************************�ļ�����***********************/
#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "include.h"
/*************************�ⲿ����***********************/
/*************************ȫ�ֱ���***********************/


/*************************���ر���***********************/
static u8 LoadControllerCmdAckFlag = 0;//���ؿ���������Ӧ���־,1Ϊ���յ�Ӧ��,0Ϊδ�յ�
static u32 UpdataFileSize = 0;
static u8  UpdataBuffer[LOAD_CONTROLLER_TX_BUFFER_SIZE] = {0};
static u32 UpdataCurrentPacket = 0;//������ǰ�����,��1��ʼ
static u32 UpdataTotalPacket = 0;//�����ܰ���
static u8  UpdataFileAdditionalInformation[23] = {0};//�����ļ�������Ϣ,22�ֽ�����,2�ֽڹ�˾��ʶ,8�ֽڲ�Ʒ��ʶ,2�ֽ�PCB�汾,6�ֽڹ̼��汾,4�ֽ�CRCУ����
static u8 LoaderControllerUpdataFlag = 0;//���ؿ�����������־,1Ϊ��������,0Ϊ��������,��������ʱ�벻Ҫ���ӿڷ�������ָ��,ֻ��������ָ��
/*************************��������***********************/
/*********************************************************************
//��������	:LoaderControllerUpdata_TimeTask(void)
//����		:���ؿ�������������
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:���ն������������ļ�����ж��Ƿ�Ϊ���ؿ������̼���������������������0.3�����1��,
*********************************************************************/
FunctionalState  LoaderControllerUpdata_TimeTask(void)
{
    static u8 UpdataState = 0;//��ǰ����״̬
    static u8 NoLoaderControllerAckCount = 0;//��Ӧ�����,����5�������ؿ�����Ӧ��,��ر�����
    static u8 LoaderControllerSendErrorCount = 0;//���ʹ������������30�η��Ͳ��ɹ�,��ر�����
    static u16 TimeCount = 0;//ʱ�����,����������,�������˰�Сʱ��û�н���,��ǿ�ƽ�������
    static u8 UpdataResultFlag = 0;//���������־,0Ϊʧ��,1Ϊ�ɹ���
    static u8 PacketTimeDelay = 0;//����ʱ,2�뻹û�յ�Ӧ�𽫻��ط���һ��,����ط�5��,�����Ȼû��Ӧ��������ʧ��
    static u16 LastestPacketLen = 0;//���һ���ĳ���
    
    u8  Buffer[6] = {0};
    u8  sum = 0;
    u8  i = 0;
    u32 Address = 0;
    u16 length = 0;
    u8  PramLen = 0;
    
    TimeCount++;//0.3������1
    if(TimeCount >= 3000)//15���Ӻ󲻹������ɹ���񶼹ر�����
    {
         TimeCount = 0;
         UpdataState = 4;
    }
    if(0 == UpdataState)//��һ�׶Σ���ȡ������Ϣ
    {
        E2prom_ReadByte(E2_PROGRAM_UPDATA_LENGTH_ID_ADDR, Buffer, 5);
        for(i=0; i<4; i++)
        {
            sum += Buffer[i];
        }
        if(sum == Buffer[4])
        {
            //��ȡ����
            UpdataFileSize = 0;
            UpdataFileSize |= Buffer[0] << 24;
            UpdataFileSize |= Buffer[1] << 16;
            UpdataFileSize |= Buffer[2] << 8;
            UpdataFileSize |= Buffer[3];
            UpdataCurrentPacket = 1;
            UpdataTotalPacket = UpdataFileSize/UPDATA_PACKET_SIZE;//ÿһ��200�ֽ�
            if(0 != UpdataFileSize%UPDATA_PACKET_SIZE)
            {
                UpdataTotalPacket++;
                LastestPacketLen = UpdataFileSize%UPDATA_PACKET_SIZE;
            }
            else
            {
                LastestPacketLen = UPDATA_PACKET_SIZE;
            }
            //��ȡ������Ϣ
            Address = UPDATE_BASE_ADDRESS+UpdataFileSize-22;
            sFLASH_ReadBuffer(UpdataFileAdditionalInformation, Address, 22);
            if((UpdataFileAdditionalInformation[0] == 'E')&&(UpdataFileAdditionalInformation[1] == 'I'))
            {
                UpdataState = 1;
                LoaderControllerUpdataFlag = 1;
            }
            else
            {
                UpdataState = 4;//�������������ƽ̨
            }
              
        }
        else
        {
            UpdataState = 4;//�������������ƽ̨
        }
    }
    else if(1 == UpdataState)//�ڶ��׶Σ����������������ؿ�����
    {
        if((1 == UpdataCurrentPacket)&&(UpdataTotalPacket >= UpdataCurrentPacket))
        {
              length = 0;
              //�ܰ���
              UpdataBuffer[0] = (UpdataTotalPacket&0xff00) >> 8;
              UpdataBuffer[1] = UpdataTotalPacket&0xff;
              //�����,��1��ʼ
              UpdataBuffer[2] = (UpdataCurrentPacket&0xff00) >> 8;
              UpdataBuffer[3] = UpdataCurrentPacket&0xff;
              //��������,�̶�Ϊ0x09
              UpdataBuffer[4] = 0x09;
              //������ID
              PramLen = EepromPram_ReadPram(E2_MANUFACTURE_ID, UpdataBuffer+5);
              if(E2_MANUFACTURE_LEN== PramLen)
              {
              
              }
              else
              {
                    UpdataBuffer[5] = '7';
                    UpdataBuffer[6] = '0';
                    UpdataBuffer[7] = '1';
                    UpdataBuffer[8] = '0';
                    UpdataBuffer[9] = '8';
              }
              //�汾�ų���,�̶�Ϊ0
              UpdataBuffer[10] = 6;
              //�汾��
              memcpy(UpdataBuffer+11,UpdataFileAdditionalInformation+12,6);
              //���������ݳ���,�����˸�����Ϣ
              UpdataBuffer[17] = (UpdataFileSize&0xff000000) >> 24;
              UpdataBuffer[18] = (UpdataFileSize&0xff0000) >> 16;
              UpdataBuffer[19] = (UpdataFileSize&0xff00) >> 8;
              UpdataBuffer[20] = UpdataFileSize&0xff;
              length += 21;
              Address = UPDATE_BASE_ADDRESS+(UpdataCurrentPacket-1)*UPDATA_PACKET_SIZE;
              sFLASH_ReadBuffer(UpdataBuffer+length, Address, UPDATA_PACKET_SIZE);
              length += UPDATA_PACKET_SIZE;
              if(ACK_OK == CarLoad_PactAndSendData(0x40,1,UpdataBuffer,length))
              {
                    UpdataState = 2;
                    PacketTimeDelay = 0;
                    LoaderControllerSendErrorCount = 0;
              }
              else
              {
                    LoaderControllerSendErrorCount++;
                    if(LoaderControllerSendErrorCount >= 30)
                    {
                        UpdataResultFlag = 0;//����ʧ��
                        UpdataState = 3;//�������������ƽ̨
                    }
              }
              
        }
        else if((UpdataCurrentPacket > 1)&&(UpdataTotalPacket >= UpdataCurrentPacket))
        {
              length = 0;
              //�ܰ���
              UpdataBuffer[0] = (UpdataTotalPacket&0xff00) >> 8;
              UpdataBuffer[1] = UpdataTotalPacket&0xff;
              //�����,��1��ʼ
              UpdataBuffer[2] = (UpdataCurrentPacket&0xff00) >> 8;
              UpdataBuffer[3] = UpdataCurrentPacket&0xff;
              length += 4;
              Address = UPDATE_BASE_ADDRESS+(UpdataCurrentPacket-1)*UPDATA_PACKET_SIZE;
              sFLASH_ReadBuffer(UpdataBuffer+length, Address, UPDATA_PACKET_SIZE);
              if(UpdataCurrentPacket == UpdataTotalPacket)
              {
                    length += LastestPacketLen;
              }
              else
              {
                    length += UPDATA_PACKET_SIZE;
              }
              if(ACK_OK == CarLoad_PactAndSendData(0x40,1,UpdataBuffer,length))
              {
                    UpdataState = 2;
                    PacketTimeDelay = 0;
                    LoaderControllerSendErrorCount = 0;
              }
              else
              {
                    LoaderControllerSendErrorCount++;
                    if(LoaderControllerSendErrorCount >= 30)
                    {
                        UpdataResultFlag = 0;//����ʧ��
                        UpdataState = 3;//�������������ƽ̨
                    }
              }
        }
        else
        {

            UpdataResultFlag = 0;//����ʧ��
            UpdataState = 3;//�������������ƽ̨
        }
    }
    else if(2 == UpdataState)//�����׶Σ�������ؿ�����Ӧ��
    {
         if(1 == LoadControllerUpdata_GetCmdAckFlag())
         {
              UpdataState = 1;
              UpdataCurrentPacket++;//��һ��
              LoadControllerUpdata_ClrCmdAckFlag();
              NoLoaderControllerAckCount = 0;
              if(UpdataCurrentPacket > UpdataTotalPacket)
              {
                    UpdataTotalPacket = 0;
                    UpdataResultFlag = 1;//�����ɹ�
                    UpdataState = 3;//�������������ƽ̨
              }
         }
         else
         {
            PacketTimeDelay++;
            if(PacketTimeDelay >= 34)//10����ʱʱ�䵽,�ط�
            {
                UpdataState = 1;
                PacketTimeDelay = 0;
                NoLoaderControllerAckCount++;
                if(NoLoaderControllerAckCount >= 3)
                {
                    NoLoaderControllerAckCount = 0;
                    UpdataResultFlag = 0;//����ʧ��
                    UpdataState = 3;//�������������ƽ̨
                }
            }
         }
    }
    else if(3 == UpdataState)//���Ľ׶Σ������������֪ͨ��־,
    {
        if(1 == UpdataResultFlag)
        {
            Buffer[0] = 2;
        }
        else
        {
            Buffer[0] = 3;
        }
        FRAM_BufferWrite(FRAM_FIRMWARE_UPDATA_FLAG_ADDR,Buffer,1);
        NVIC_SystemReset();//������Ҫ��������Ϊ��������洢����ä���洢���ǹ��õģ�Ϊ��ȫ��������󲻹��Ƿ�ɹ�������������
    }
    else//����,�ر�����
    {
         UpdataFileSize = 0;
         UpdataCurrentPacket = 0;
         UpdataTotalPacket = 0;
         UpdataState = 0;
         PacketTimeDelay = 0;
         TimeCount = 0;
         LoaderControllerUpdataFlag = 0;
         Buffer[0] = 3;
         FRAM_BufferWrite(FRAM_FIRMWARE_UPDATA_FLAG_ADDR,Buffer,1);
         NVIC_SystemReset();//������Ҫ��������Ϊ��������洢����ä���洢���ǹ��õģ�Ϊ��ȫ��������󲻹��Ƿ�ɹ�������������
        
    }
    return ENABLE;
}
/*********************************************************************
//��������	:LoadControllerUpdata_SendData
//����		:�������ݸ����ؿ�����
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:���ؿ�������EGS701GBͨ��DB9��������
//		:CmdType:0�Ƕ�,1��д
*********************************************************************/
/*
ProtocolACK LoadControllerUpdata_SendData(u8 Cmd,u8 CmdType,u8 *pData,u16 DataLen)
{
    u8  Buffer[LOAD_CONTROLLER_TX_BUFFER_SIZE] = {0};
    u16 BufferLen = 0;
    u16 CrcCode = 0;
    
    if(DataLen > (LOAD_CONTROLLER_TX_BUFFER_SIZE-6))//���ȳ���
    {
        return ACK_ERROR;
    }
    //ͬ����
    Buffer[0] = 0x69;
    //��ַ��
    Buffer[1] = 0x01;
    //������
    Buffer[2] = 0x42;
    //������
    if(0 == CmdType)//0�Ƕ�
    {
        Buffer[3] = Cmd;
    }
    else//1��д
    {
        Buffer[3] = Cmd | 0x80;
    }
    //������
    memcpy(Buffer+4,pData,DataLen);
    
    //У����
    CrcCode = CRC16(Buffer, DataLen+4);
    Buffer[4+DataLen] = (CrcCode&0xff00) >> 8;
    Buffer[5+DataLen] = CrcCode&0x00ff;
    
    //����
    BufferLen = DataLen+6;
    
    if(1 == Recorder_SendData(Buffer, BufferLen))//Ŀǰ����1��ʾ�ɹ�,ע����ڿ��ܻ��
    {
       return ACK_OK;
    }
    else
    {
        return ACK_ERROR;
    }
    
}
*/
/*********************************************************************
//��������	:LoadControllerUpdata_ProtocolParse
//����		:�����ؿ��������͹��������ݽ��н���
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:���ؿ�������EGS701GBͨ��DB9��������
//		:
*********************************************************************/
/*
ProtocolACK LoadControllerUpdata_ProtocolParse(u8 *pData,u16 DataLen)
{
    u8  *p = NULL;
    u16 i = 0;
    u16 j = 0;
    u16 CrcCode = 0;
    u16 CrcHi = 0;
    u16 CrcLow = 0;
    
    
    //������ݵ���ȷ��
    if((NULL == p)&&(DataLen >= 6))
    {
        p = pData;
        for(i=0; i<DataLen; i++)
        {
            if((0x51 == *(p+i))&&(0x01 == *(p+i+1)))//���ͬ����͵�ַ��
            {
                j = i;
                break;
            }
        }
        if(DataLen == i)//û���ҵ�֡ͷ
        {
            return ACK_ERROR;
        }
        else
        {
            if(j > 0)
            {
                p += j;//ָ��ָ��֡ͷ
            }
            CrcCode = CRC16(p, DataLen-j-2);
            CrcHi = (CrcCode&0xff00) >> 8;
            CrcLow = CrcCode&0xff;
            if((CrcHi == *(p+DataLen-2))&&(CrcLow == *(p+DataLen-1)))
            {
               if((*(p+3) == 0xC0)&&(*(p+4) == 0x09)&&(*(p+5) == 0x00))//��Զ������ָ��Ӧ���һظ��ɹ�
               {
                    //У��ͨ��,��λӦ���־
                    LoadControllerUpdata_SetCmdAckFlag();
               }
            }
            else
            {
                return ACK_ERROR;
            }
        }
    }
    else
    {
        return ACK_ERROR;
    }
    return ACK_OK;
}
*/
/*********************************************************************
//��������	:LoadControllerUpdata_DisposeUpdataResponse
//����		:������������Ӧ��(У����ͨ��)
//����		:pBuffer:Ӧ���������׵�ַ,Ӧ�������򳤶�
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
*********************************************************************/
void LoadControllerUpdata_DisposeUpdataResponse(unsigned char *pBuffer,unsigned short datalen)
{
    //��������
    if(0==pBuffer[1])//�������
    LoadControllerUpdata_SetCmdAckFlag();
}
/*********************************************************************
//��������	:LoadControllerUpdata_GetUpdataFlag
//����		:��ȡ���ؿ�����������־
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:1:��ʾ����������,0��ʾû������
//��ע		:��������ʱ�벻Ҫ��������ָ�������������š�
//		:
*********************************************************************/
u8 LoadControllerUpdata_GetUpdataFlag(void)
{
    return LoaderControllerUpdataFlag;
}
/*********************************************************************
//��������	:LoadControllerUpdata_GetCmdAckFlag
//����		:��ȡ���ؿ���������Ӧ���־
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
//		:
*********************************************************************/
u8 LoadControllerUpdata_GetCmdAckFlag(void)
{
    return LoadControllerCmdAckFlag;
}
/*********************************************************************
//��������	:LoadControllerUpdata_SetCmdAckFlag
//����		:��λ���ؿ���������Ӧ���־
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
//		:
*********************************************************************/
void LoadControllerUpdata_SetCmdAckFlag(void)
{
    LoadControllerCmdAckFlag = 1;
}
/*********************************************************************
//��������	:LoadControllerUpdata_ClrCmdAckFlag
//����		:������ؿ���������Ӧ���־
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
//		:
*********************************************************************/
void LoadControllerUpdata_ClrCmdAckFlag(void)
{
    LoadControllerCmdAckFlag = 0;
}



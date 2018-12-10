/*
********************************************************************************
*
*
* Filename      : vdr_pub.c
* Version       : V1.00
* Programmer(s) : miaoyahan
* Time          : 20140829
********************************************************************************
*/

/*
********************************************************************************
*                               INCLUDES
********************************************************************************
*/
#include "vdr.h"
/*
********************************************************************************
*                             LOCAL DEFINES
********************************************************************************
*/

/*
********************************************************************************
*                         LOCAL DATA TYPES
********************************************************************************
*/

/*
********************************************************************************
*                          CONST VARIABLES
********************************************************************************
*/

/*
********************************************************************************
*                          LOCAL VARIABLES
********************************************************************************
*/
static u8  SerOutTmrCnt;//���ڳ�ʱ������
static u8  SerBusyFlag;//����æ��־  1Ϊæ 0Ϊ����
static u8  SerRxBuf[1024];//���ڽ��ջ���
static u16 SerRxLen;//���ڽ��ճ���

/*
********************************************************************************
*                         EXTERN GLOBAL VARIABLES
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
	VDR_HEAD head;

    Vdr_Get_CarInfor(&head);
    memcpy(pBuffer,head.pbuf,head.len);

	return 41;
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

    VDR_HEAD head;
    
    memset(nTab,0,sizeof(nTab));                                              /*��ջ�����      */           
    
    nTab[0] = 0x55;
    nTab[1] = 0x7a;
    nTab[2] = 0xe1;
    nTab[3] = 0;
    nTab[4] = 44;
    nTab[5] = PRO_DEF_RETAINS;
    
    Vdr_Get_OnlyNum(&head);
    memcpy(nTab+6,head.pbuf,head.len);
  
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
    
    COM1_WriteBuff(nTab,51);	
  
}

/**
  * @brief  ��¼��U�̵�������
  * @param  None
  * @retval None
  */
void Recorder_USBHandle(void)
{
    u8 buf[5];
    
    //��ȡ������־
    E2prom_ReadByte(E2_PROGRAM_UPDATA_REQUEST_ID_ADDR,buf,5);
    if((buf[0]==0xaa)&&(buf[1]==0xaa))
    {
        return;//��������ʱ����������
    }
    Vdr_Usb_Handle();
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
    if(Vdr_Wire_Handle())
    {
        return ENABLE;
    }

    return DISABLE;
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
    if(len == 0)
    {
        if(SerRxLen)
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

    if((SerRxLen + len) < sizeof(SerRxBuf))//�ж����
    {
        memcpy(SerRxBuf+SerRxLen,p,len);
        SerRxLen += len;
    }
    return 0;
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
    
    ////////////////////////////////////////////////////////////////////////////һ֡����
    if(Recorder_SerOutTime(nRecTab,nRecLen) == 0)
    {
        return ENABLE;
    }
    
    ////////////////////////////////////////////////////////////////////////////���ݴ���
    ///// ���յ�����һ֡���� -> SerRxBuf
    ///// ..............���� -> SerRxLen

    ///// ����Э�鴦��
    CarLoad_CommProtocolParse(SerRxBuf,SerRxLen);

    /////��¼��Э�鴦��,��Э����ŵ����
    Vdr_Data_Rec(VDR_COM,0,SerRxBuf,SerRxLen);

    memset(SerRxBuf,0,SerRxLen);
    SerBusyFlag = 0;
    SerRxLen    = 0;
    
    return ENABLE;
}


/*
********************************************************************************
*                               END
********************************************************************************
*/
  

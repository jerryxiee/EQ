/*
********************************************************************************
*
*
* Filename      : vdr.c
* Version       : V1.00
* Programmer(s) : miaoyahan
* Time          : 20140722
********************************************************************************
*/

/*
********************************************************************************
*                               INCLUDES
********************************************************************************
*/
#include "Vdr.h"

/*
********************************************************************************
*                             LOCAL DEFINES
********************************************************************************
*/
#define VDR_CMD_ERR_GET  0xfa
#define VDR_CMD_ERR_SET  0xfb
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
const u8 Vdr_Cmd_Base[]=//�����ɼ�����
{0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07};
const u8 Vdr_Cmd_Ser[]=//�����ɼ�����
{0x08,0x09,0x10,0x11,0x12,0x13,0x14,0x15};
const u8 Vdr_Cmd_Set[]=//��������
{0x82,0x83,0x84,0xc2,0xc3,0xc4};
const u8 Vdr_Cmd_Test[]=//�춨����
{0xe0,0xe1,0xe2,0xe3,0xe4,0xe5};

const u8 Vdr_Cmd_All[]=//��������
{0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,//0  - 7  �ɼ�
 0x08,0x09,0x10,0x11,0x12,0x13,0x14,0x15,//8  - 15 �ɼ�
 0x82,0x83,0x84,0xc2,0xc3,0xc4,          //16 - 21 ����
 0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,          //22 - 27 �춨
 0xd0,                                   //28      ����
};

/*
********************************************************************************
*                          LOCAL VARIABLES
********************************************************************************
*/
static VDR_WIRE    Vdr_Wire;//���߲ɼ�
static VDR_WIREADD Vdr_WireAdd;//���߲���
/*
********************************************************************************
*                          GLOBAL VARIABLES
********************************************************************************
*/
u8  Vdr_BufData[1024];//���ݻ���
u8  Vdr_BufSend[1200];//���ͻ���
u16 Vdr_WireAck;//���߲ɼ�Ӧ����ˮ��    

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
* @brief  �������У��
* @param   pVef  -> У����
           pSrc  -> Դ����
           SrcLen-> Դ����
* @retval None
*/
void Vdr_XorVef(u8 *pVef,const u8 *pSrc, u16 SrcLen)
{ 
    u16 i;
    
    for(i=0;i<SrcLen;i++)
    {
        *pVef ^= *pSrc++;
    }
}
/**
  * @brief  ��ȡ����ʱ��ͷ����
  * @param  None
  * @retval None
  */
void Vdr_Time_Get(const u8 *pSrc, VDR_SERT *pTime)
{
    u32 timecnt;

    //������ݺϷ���
    memset(pTime,0,sizeof(VDR_SERT));//��սṹ��

    memcpy(pTime->bcd_start,pSrc,6);//��ʼʱ��
    pSrc += 6;
    
    memcpy(pTime->bcd_end,pSrc,6);//����ʱ��
    pSrc += 6;

    pTime->maxblock = *pSrc++;
    pTime->maxblock = (pTime->maxblock<<8)|(*pSrc++);//���ݿ��� 

    Public_ConvertBCDToTime(&pTime->t_start,
                             pTime->bcd_start);
                            
    Public_ConvertBCDToTime(&pTime->t_end, 
                             pTime->bcd_end);    

    timecnt = ConverseGmtime(&pTime->t_start);
    pTime->cnt_start = timecnt;
    
    timecnt = ConverseGmtime(&pTime->t_end)+1;
    pTime->cnt_end= timecnt; 

}


/**
  * @brief  ��ȡ��¼��֡ͷ����
  * @param  None
  * @retval None
  */
void Vdr_Head_Get(const u8 *pSrc, VDR_HEAD *pHead)
{
    pSrc += 2;                              //55 7a
    pHead->cmd  = *pSrc++;                  //����
    pHead->len  = *pSrc++; 
    pHead->len  = (pHead->len<<8)|(*pSrc++);//����
    pHead->back = *pSrc++;                  //������
    pHead->pbuf = (u8*)pSrc;                //���� 
}

/**
  * @brief  �Ѽ�¼�ǽṹ��ת�ɻ����ֽ���
  * @param  type: 0->ͷ+����+У��  
                  1->ͷ+���� 
                  2->������      
  * @retval None
  */
u16 Vdr_Head_ToBuf(u8 type, VDR_HEAD head, u8 *pDst)
{   
    u8  *pVef=pDst;
    u8  verf=0;    
    u16 len=0;
    
    if((head.cmd == VDR_CMD_ERR_GET)||(head.cmd == VDR_CMD_ERR_SET))
    {
         len    = 4;
        *pDst++ = 0x55;
        *pDst++ = 0x7a;
        *pDst++ = head.cmd;
        *pDst++ = PRO_DEF_RETAINS;
    }
    else
    {
        if(type != 2)//������ʱ������֡ͷ
        {
             len    = 6;
            *pDst++ = 0x55;
            *pDst++ = 0x7a;
            *pDst++ = head.cmd;
            *pDst++ = head.len >> 8;
            *pDst++ = head.len & 0xff;
            *pDst++ = PRO_DEF_RETAINS;
        }
        memcpy(pDst,head.pbuf,head.len);
        pDst += head.len;
        len  += head.len;
    }

    if(type == 0)//�������ݲż���У��
    {
        Vdr_XorVef(&verf,pVef,len);
        *pDst++ = verf;
        len++;
    }

    return len;
}
/**
  * @brief  ��¼������ת��ַ
  * @param  cmd->����
  * @retval ���ص�ַ
  */
u8 Vdr_Cmd_To(u8 cmd)
{
    u8 i;
    
    for(i=0;i<sizeof(Vdr_Cmd_All);i++)
    {
        if(Vdr_Cmd_All[i] == cmd)
        {
            return i;
        }
    }
    return 0;//�Ƿ�
}

/**
  * @brief  ����¼��������
  * @param  cmd->����
  * @retval �Ϸ�����1  �Ƿ�����0
  */
u8 Vdr_Cmd_Chk(u8 cmd)
{
    u8 i;
    
    for(i=0;i<sizeof(Vdr_Cmd_All);i++)
    {
        if(Vdr_Cmd_All[i] == cmd)
        {
            return 1;//�Ϸ�
        }
    }
    return 0;//�Ƿ�
}
/**
  * @brief  ����������������͡���������
  * @param  cmd->��¼������
            pType->���� 0:����ָ�� 1:�ɼ�ָ�� 2:���� 3:���� 4:����
            pAddr->��Ӧ�����ַ
  * @retval ���ؽ�� �Ϸ�����1���Ƿ�����0 
  */
u8 Vdr_Cmd_Cnv(u8 cmd, u8 *pType, u8 *pAddr)
{
    u8 i;

    for(i=0;i<sizeof(Vdr_Cmd_All);i++)
    {
        if(Vdr_Cmd_All[i] == cmd)
        {
            cmd = i;
            break;
        }
    }
    if(i == sizeof(Vdr_Cmd_All))
    {
        return 0;//�Ƿ�ָ��
    }
    
    if(cmd < 8)
    {
        *pType = 0;//����ָ��
        *pAddr = cmd;
    }
    else if(cmd < 16)
    {
        *pType = 1;//�ɼ�ָ��
        *pAddr = cmd-8;            
    }
    else if(cmd < 22)
    {
        *pType = 2;//��������
        *pAddr = cmd-16;
    }
    else if(cmd < 28)
    {
        *pType = 3;//����ָ��
        *pAddr = cmd-22;              
    } 
    else if(cmd == 28)
    {
        *pType = 4;//����
        *pAddr = 0;  
    }
    
    return 1;//�Ϸ�ָ��
}
/**
  * @brief  �������߲ɼ�����(����ר��)
  * @param  None
  * @retval None
  */
void Vdr_Wire_Even(VDR_CH ch, VDR_HEAD head)
{
    VDR_SERT    serch_time;//����ʱ��
    TIME_T      now_time;  //��ǰʱ��
    VDR_PACKINF packageinf;//�ְ���Ϣ

    if(Vdr_Wire.busy)
    {
        if(Vdr_Wire.ch == ch)
        {
            return;//��ͬͨ���ظ��·�����
        }
    }
    Vdr_Wire.ch   = ch;//����ͨ��
    Vdr_Wire.busy = 1;//��æ��־
    
    if(head.len == 0)//ȫ������
    {
        RTC_GetCurTime(&now_time);
        now_time.year -= 1;
        packageinf.timestar = ConverseGmtime(&now_time);//��ʼʱ��
        now_time.year += 2;
        packageinf.timeend = ConverseGmtime(&now_time);//����ʱ��
    }
    else//�ɼ�ָ��ʱ�������
    {
        Vdr_Time_Get(head.pbuf,&serch_time);
        packageinf.timestar = serch_time.cnt_start;
        packageinf.timeend  = serch_time.cnt_end;
    }
    
    packageinf.cmdnum = Vdr_Cmd_To(head.cmd);//�����ֱ��
    
    Vdr_Wire.packageinf = packageinf;//���Ʒְ���Ϣ
    SetTimerTask(TIME_WIRELESS_RECORDER, SYSTICK_1SECOND);//�������߼�¼�Ƕ�ʱ���� 
}
/**
  * @brief  �������߲�������(����ר��)
  * @param  pSrc->ָ��Դ����
  * @retval ���ؽ��, �ϴ��������ݷ���1, �����ݷ���0
  */
u8 Vdr_Wire_AddEven(VDR_CH ch, u8 *pSrc)
{
    u8 i = 0;
    u8 high,low;

    if(Vdr_Wire.ch != ch)
    {
        Vdr_WireAdd.rdy = 0;
        return 0;//ͨ����һ��
    }
    if(Vdr_WireAdd.rdy == 0)
    {
        return 0;//�Ǽ�¼��8003
    }
    
    high = *pSrc++;
    low  = *pSrc++;
    Vdr_WireAdd.ack = (high <<8)| low;//��ˮ��
    Vdr_WireAdd.all = *pSrc++;        //�ش�����
    Vdr_WireAdd.cnt = 0;

    for(i=0;i<Vdr_WireAdd.all;i++)    //�ش��б�
    {
        high = *pSrc++;
        low  = *pSrc++;    
        Vdr_WireAdd.buf[i] = (high <<8)| low;     
    }
    if(Vdr_WireAdd.all)
    {
        Vdr_Wire.busy   = 1;//��æ��־
        Vdr_WireAdd.flg = 1;//�ò�����־
        SetTimerTask(TIME_WIRELESS_RECORDER, SYSTICK_1SECOND);//�������߼�¼�Ƕ�ʱ���� 
        return 1;//��ʼ��������
    }
    Vdr_WireAdd.rdy = 0;
    return 0;//�������
}
/**
  * @brief  �������߲�������(����ר��)
  * @param  None
  * @retval None
  */
u8 Vdr_Wire_AddGet(VDR_PACKSER *pPackage)
{

    VDR_PACKSER Package = Vdr_Wire.packageadd;
    u16 i;
    
    if(Vdr_WireAdd.cnt >= Vdr_WireAdd.all)
    {
        Vdr_WireAdd.flg = 0;
        return 0;//�������
    }
    
    for(i=1;i<=Package.all;i++)
    {
        Vdr_Get_PackageSer(&Package);
        if(Package.cnt == Vdr_WireAdd.buf[Vdr_WireAdd.cnt])
        {
            *pPackage = Package;
            Vdr_WireAdd.cnt++;  
            return 1;
        }
    }
    return 0;
}
/**
  * @brief  �������ݰ�����(����ר��)
  * @param  None
  * @retval None
  */
u16 Vdr_Wire_Package(VDR_CH ch,VDR_PACKSER *pPackage)
{
    MESSAGE_HEAD msg_head;
    VDR_HEAD head;
    u16 addr;
    u16 len;
    u8  wirech;

    //////////////////////////////////////////////////////////////////////��һ��
    addr = 0;
    if(pPackage->cnt == 1)
    {
        Vdr_BufSend[addr++] = Vdr_WireAck>>8;
        Vdr_BufSend[addr++] = Vdr_WireAck&0xff;
        Vdr_BufSend[addr++] = pPackage->cmd;
    } 
    ////////////////////////////////////////////////////////////////��¼��Э��ͷ
    head.cmd = pPackage->cmd;
    head.len = pPackage->len;
    head.pbuf= pPackage->pbuf;
    len = Vdr_Head_ToBuf(0,head,Vdr_BufSend+addr);
    len += addr;

    msg_head.TotalPackage   = pPackage->all;
    msg_head.CurrentPackage = pPackage->cnt;    
    msg_head.MessageID      = 0x0700;
    msg_head.SerialNum      = Vdr_WireAck; 
    msg_head.Attribute      = len;
    if(pPackage->all > 1)
    {
        msg_head.Attribute |= 0x2000;//�ְ��ϴ�
    }    

    //////////////////////////////////////////////////////////////////////ͨ����
    wirech = CHANNEL_DATA_1;
    if(ch == VDR_WIRE_2)
    {
        wirech = CHANNEL_DATA_2;
    }
    #if 0
    if((pPackage->cnt == 2)&&(!Vdr_WireAdd.flg))//ǿ�ƶ���
    {
        if(pPackage->cnt >= pPackage->all)
        {
            pPackage->all = 0;
            pPackage->cnt = 0;
            return 0;
        }      
        return 1;
    }
    #endif
    RadioProtocol_SendCmd(wirech,msg_head,Vdr_BufSend,len,NULL);
    ////////////////////////////////////////////////////////////////////���һ��    
    if(pPackage->cnt >= pPackage->all)
    {
        pPackage->all = 0;
        pPackage->cnt = 0;
        return 0;
    }      
    return 1;
}

/**
  * @brief  ��¼�����ߴ���
  * @param  None
  * @retval ���ؽ��
  */
u8 Vdr_Wire_Handle(void)
{
    VDR_PACKSER *pPackage = &Vdr_Wire.packageser;

    if(Vdr_Wire.busy == 0)
    {
        Vdr_WireAdd.flg = 0;
        Vdr_WireAdd.rdy = 0;
        return 0;
    }
    else if(Vdr_Wire.busy == 1)
    {
        Vdr_Wire.busy++;
        Vdr_Wire.state = 0;//������������
        if(Vdr_WireAdd.flg)
        {
            Vdr_Wire.state = 2;//���벹��
        }
    }
    
    switch(Vdr_Wire.state)
    {
        case 0://��ȡ�ְ���Ϣ
        {
            Vdr_Get_PackageInf(Vdr_Wire.packageinf,pPackage);
            Vdr_Wire.packageadd = *pPackage;
            Vdr_Wire.state++;
            break;
        }
        case 1://����ְ�����
        {
            Vdr_Get_PackageSer(pPackage);                //��ȡ����
            if(Vdr_Wire_Package(Vdr_Wire.ch,pPackage)==0)//����ƽ̨
            {
                Vdr_Wire.state  = 0xaa;//����
                Vdr_WireAdd.rdy = 1;   //�ȴ�8003
            }
            break;
        }
        case 2://�����ְ�����
        {
            if(Vdr_Wire_AddGet(pPackage) == 0)//��ȡ��������
            {
                Vdr_Wire.state=0xaa;//����
                break;
            }
            Vdr_Wire_Package(Vdr_Wire.ch,pPackage);
            break;
        }
        default:
        {
            Vdr_Wire.busy=0;//����
            return 0;//ֹͣ
        } 
    }
    return 1;//����
}
/**
  * @brief  ������Դ��ڵ�����
  * @param  None
  * @retval ���ش�������
  */
VDR_ERR Vdr_Data_Chk(VDR_CH ch, u8 *pSrc, u16 SrcLen)
{
    VDR_ERR err=VDR_ERR_NO;//��ȷ
    u8 vef=0;
    u8 *pVef=pSrc;
    u8 cmd;

    if(ch == VDR_COM)
    {
        ////////////////////////////////////////////////////////////////���֡ͷ
        if((*pSrc++ != 0xaa)||(*pSrc++ != 0x75))
        {
            return VDR_ERR_HEAD;//�Ǽ�¼������
        }
        cmd = *pSrc++;//������
        ////////////////////////////////////////////////////////////////���У��
        Vdr_XorVef(&vef,pVef,SrcLen-1);
        pVef += SrcLen-1;
        if(vef != *pVef)
        {
            return VDR_ERR_VF;//У�����
        }
    }
    else
    {
        cmd = *pSrc++;//������
    }
    ////////////////////////////////////////////////////////////////////���ָ��
    if(Vdr_Cmd_Chk(cmd) == 0)
    {
        err = VDR_ERR_CMD;//ָ�����
    }
    
    return err;
}


/**
  * @brief  Ӧ�����Դ��ڵ�����,������Ч���ݵ�����
  * @param  None
  * @retval ��ȷ���� 0
  *         ���󷵻� ��0
  */
void Vdr_Data_Send(VDR_CH ch, VDR_HEAD head)
{
    MESSAGE_HEAD msg_head;
    u16 len;
    u16 addr;
    u8  wirech;

    if(ch == VDR_COM)
    {
        len = Vdr_Head_ToBuf(0,head,Vdr_BufSend);
        COM1_WriteBuff(Vdr_BufSend,len);
    }
    else
    {
        addr = 0;
        Vdr_BufSend[addr++] = Vdr_WireAck>>8;
        Vdr_BufSend[addr++] = Vdr_WireAck&0xff;

        Vdr_BufSend[addr++] = head.cmd;
        len = Vdr_Head_ToBuf(0,head,Vdr_BufSend+addr);//תΪ�ֽ���
        len += addr;    
        
        msg_head.MessageID = 0x0700;//��¼�������ϴ�����  
        msg_head.SerialNum = Vdr_WireAck; 
        msg_head.Attribute = len;  

        wirech = CHANNEL_DATA_1; 
        if(ch == VDR_WIRE_2)
        {
            wirech = CHANNEL_DATA_2;
        }
        RadioProtocol_SendCmd(wirech,msg_head,Vdr_BufSend,len,NULL);
    }
}

/**
  * @brief  �������Դ��ڻ����������
  * @param  None
  * @retval ���ش�����
  */
u8 Vdr_Data_Pro(VDR_CH ch, VDR_HEAD head, VDR_HEAD *pHead)
{
    VDR_SERT  serch_time;
    u8  addr;
    u8  type;
    
    ////////////////////////////////////////////////////////////////////ת������
    if(Vdr_Cmd_Cnv(head.cmd,&type,&addr) ==0 )
    {
        return 0;//�Ƿ�����
    }
    
    ////////////////////////////////////////////////////////////////////�ɼ�����
    switch(type)
    {
        case 0://����ָ��
        {
            Vdr_Get_Base(addr,pHead);
            Vdr_Wire.busy = 0;//ֹͣ���緢��
            break;
        }
        case 1://�ɼ�ָ��
        {
            if(ch == VDR_COM)//����
            {
                Vdr_Time_Get(head.pbuf,&serch_time);
                Vdr_Get_Ser(addr,serch_time,pHead);
            }
            else//���߲ɼ�
            {   
                Vdr_Wire_Even(ch,head);
                return 0;
            }
            break;
        }
        case 2://����ָ��
        {
            Vdr_Set_Pro(addr,head.pbuf,pHead);
            break;
        }
        case 3://����ָ��
        {
            Vdr_Test_Pro(addr,pHead);
            break;
        } 
        case 4://����ָ��
        {
            Vdr_Eye_Value(&head,pHead);
            break;
        }        
    }
    return 1;
}

/**
  * @brief  ��¼�ǽ��մ���
  * @param  ch     -> ͨ����
            type   -> 0:�ɼ� 1:����
            pSrc   -> Դ����
            SrcLen -> Դ����
  * @retval None
  */
void Vdr_Data_Rec(VDR_CH ch, u8 type, u8 *pSrc, u16 SrcLen)
{
    VDR_HEAD HeadRec,HeadSend;
    VDR_ERR  err;

    ////////////////////////////////////////////////////////////////////�������
    err = Vdr_Data_Chk(ch,pSrc,SrcLen);
    if(err != VDR_ERR_NO)
    {
        if(err == VDR_ERR_CMD)
        {
            HeadSend.cmd = VDR_CMD_ERR_GET;      //�ɼ�����
            if(type)
            {
                HeadSend.cmd = VDR_CMD_ERR_SET;  //���ô���
            }
            Vdr_Data_Send(ch,HeadSend);
        }
        return;                                
    }

    ////////////////////////////////////////////////////////////////////��ȡ����
    if(SrcLen < 6)//������ͷ
    {
        HeadRec.cmd = *pSrc++;
        HeadRec.len = 0;
    }
    else//������ + ��¼����������
    {
        if(ch != VDR_COM)
        {
            pSrc++;
        }
        Vdr_Head_Get(pSrc,&HeadRec);
    }
    
    ////////////////////////////////////////////////////////////////////��������
    HeadSend.pbuf = Vdr_BufData;
    if(Vdr_Data_Pro(ch,HeadRec,&HeadSend))
    {
        Vdr_Data_Send(ch,HeadSend);//��������
    }
}

/*
********************************************************************************
*                               END
********************************************************************************
*/




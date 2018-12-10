/*
********************************************************************************
*
*
* Filename      : vdr_get.c
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

#ifdef USE_ONE_MIN_SAVE_SPEED_RECORD
#define VDR_SPEED_START_SECTOR	     588//��ʻ�ٶȼ�¼
#define VDR_SPEED_END_SECTOR	     692//416K byte
#else
#define	VDR_SPEED_START_SECTOR		 224//��ʻ�ٶȼ�¼
#define VDR_SPEED_END_SECTOR		 692//1868K byte
#endif

#define	VDR_POSITION_START_SECTOR	 693//λ����Ϣ��¼
#define VDR_POSITION_END_SECTOR		 864//512K byte

#define	VDR_DOUBT_START_SECTOR		 865//�ɵ����ݼ�¼
#define VDR_DOUBT_END_SECTOR		 875//32K byte

#define	VDR_OVER_START_SECTOR		 876//��ʱ��ʻ��¼
#define VDR_OVER_END_SECTOR		     880//12K byte

#define	VDR_DRIVER_START_SECTOR		 881//��ʻ����ݼ�¼
#define VDR_DRIVER_END_SECTOR		 885//12K byte

#define	VDR_POWER_START_SECTOR		 886//�����¼
#define VDR_POWER_END_SECTOR		 889//8K byte

#define	VDR_VALUE_START_SECTOR		 890//�����޸ļ�¼
#define VDR_VALUE_END_SECTOR		 893//8K byte

#define	VDR_SPDLOG_START_SECTOR      894//�ٶ�״̬��¼
#define VDR_SPDLOG_END_SECTOR	     897//8K byte


#ifdef USE_ONE_MIN_SAVE_SPEED_RECORD
#define	VDR_SPEED_STEP_LEN           144 //ʱ��4+ÿ���ӵ���ʻ�ٶȿ�126+Ԥ��13+У��1	
#else
#define	VDR_SPEED_STEP_LEN           11	 //ʱ��4+��ʻ�ٶȼ�״̬2+Ԥ��4+У��1
#endif

#define	VDR_POSITION_STEP_LEN        24	 //ʱ��4+λ����Ϣ10+ƽ���ٶ�1+���4+Ԥ��4+У��1
#define	VDR_DOUBT_STEP_LEN           251 //ʱ��4+�ɵ�����234+Ԥ��12+У��1
#define	VDR_OVER_STEP_LEN            67  //ʱ��4+��ʱ��ʻ50+Ԥ��12+У��1
#define	VDR_DRIVER_STEP_LEN          36  //ʱ��4+��ʻ�����25+Ԥ��6+У��1
#define	VDR_POWER_STEP_LEN           18  //ʱ��4+�ⲿ����7+Ԥ��6+У��1
#define	VDR_VALUE_STEP_LEN           18  //ʱ��4+�����޸�7+Ԥ��6+У��1
#define	VDR_SPDLOG_STEP_LEN          144 //ʱ��4+�ٶ�״̬133+Ԥ��6+У��1


#define	VDR_SPEED_UP_LEN             126	 
#define	VDR_POSITION_UP_LEN          666	 
#define	VDR_DOUBT_UP_LEN             234 
#define	VDR_OVER_UP_LEN              50  
#define	VDR_DRIVER_UP_LEN            25  
#define	VDR_POWER_UP_LEN             7 
#define	VDR_VALUE_UP_LEN             7
#define	VDR_SPDLOG_UP_LEN            133


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
const u16 VdrSecAddr[]=//������ַ
{
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
VDR_SPEED_START_SECTOR,
VDR_SPEED_END_SECTOR,
VDR_POSITION_START_SECTOR,
VDR_POSITION_END_SECTOR,
VDR_DOUBT_START_SECTOR,
VDR_DOUBT_END_SECTOR,
VDR_OVER_START_SECTOR,
VDR_OVER_END_SECTOR,
VDR_DRIVER_START_SECTOR,
VDR_DRIVER_END_SECTOR,
VDR_POWER_START_SECTOR,
VDR_POWER_END_SECTOR,
VDR_VALUE_START_SECTOR,
VDR_VALUE_END_SECTOR,
VDR_SPDLOG_START_SECTOR,
VDR_SPDLOG_END_SECTOR,
};
const u16 VdrStepLen[]=//��������
{
0,0,0,0,0,0,0,0,
VDR_SPEED_STEP_LEN,
VDR_POSITION_STEP_LEN,
VDR_DOUBT_STEP_LEN,
VDR_OVER_STEP_LEN,
VDR_DRIVER_STEP_LEN,
VDR_POWER_STEP_LEN,
VDR_VALUE_STEP_LEN,
VDR_SPDLOG_STEP_LEN,
};
const u16 VdrUpLen[]=//�ϴ�����
{
0,0,0,0,0,0,0,0,
VDR_SPEED_UP_LEN, 
VDR_POSITION_UP_LEN, 
VDR_DOUBT_UP_LEN,
VDR_OVER_UP_LEN,
VDR_DRIVER_UP_LEN, 
VDR_POWER_UP_LEN,
VDR_VALUE_UP_LEN,
VDR_SPDLOG_UP_LEN,
};

const VDR_FUC_BASE Vdr_Fuc_Base[]=//�ɼ�������Ϣ
{
 Vdr_Get_GbtVer,
 Vdr_Get_DvrInfor,
 Vdr_Get_Rtc,
 Vdr_Get_Mileage,
 Vdr_Get_Pulse,
 Vdr_Get_CarInfor,
 Vdr_Get_StaSignal,
 Vdr_Get_OnlyNum,
};

/*
********************************************************************************
*                          LOCAL VARIABLES
********************************************************************************
*/
static u8  Vdr_PackBuf[VDR_PACKAGE_SIZE];//�ְ�����

/*
********************************************************************************
*                          GLOBAL VARIABLES
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
  * @brief  ��¼��Э��ִ�����
  * @param  None
  * @retval None
  */
void Vdr_Get_GbtVer(VDR_HEAD *pHead)
{
    pHead->pbuf[0] = 0x12;
    pHead->pbuf[1] = 0x00;
    pHead->len     = 2;

}
/**
  * @brief  ��ʻԱ��Ϣ
  * @param  None
  * @retval None
  */
void Vdr_Get_DvrInfor(VDR_HEAD *pHead)
{
    pHead->len = 18;
    memset(pHead->pbuf,0,pHead->len);
    Vdr_Fuc_Mem[VDR_MEM_DVRINFOR](VDR_MEM_READ,pHead->pbuf);
}
/**
  * @brief  ʵʱʱ��
  * @param  None
  * @retval None
  */
void Vdr_Get_Rtc(VDR_HEAD *pHead)
{
    pHead->len = 6;
    memset(pHead->pbuf,0,pHead->len);
    Vdr_Fuc_Mem[VDR_MEM_RTC](VDR_MEM_READ,pHead->pbuf);
}
/**
  * @brief  �ۼ����
  * @param  None
  * @retval None
  */
void Vdr_Get_Mileage(VDR_HEAD *pHead)
{
    u8 *pDst=pHead->pbuf;

    pHead->len = 20;
    memset(pHead->pbuf,0,pHead->len);

    Vdr_Fuc_Mem[VDR_MEM_RTC](VDR_MEM_READ,pDst);//ʵʱʱ��
    pDst += 6;
    
    Vdr_Fuc_Mem[VDR_MEM_TIMEINIT](VDR_MEM_READ,pDst);//����ʱ��
    pDst += 6;
    
    Vdr_Fuc_Mem[VDR_MEM_MILEINIT](VDR_MEM_READ,pDst);//��ʼ���
    pDst += 4;

    Vdr_Fuc_Mem[VDR_MEM_MILEADD](VDR_MEM_READ,pDst);//�ۼ����
    pDst += 4;
}
/**
  * @brief  ����ϵ��
  * @param  None
  * @retval None
  */
void Vdr_Get_Pulse(VDR_HEAD *pHead)
{
    u8 *pDst=pHead->pbuf;

    pHead->len = 8;
    
    memset(pHead->pbuf,0,pHead->len);
    
    Vdr_Fuc_Mem[VDR_MEM_RTC](VDR_MEM_READ,pDst);//��ǰʱ��
    pDst += 6;
    
    Vdr_Fuc_Mem[VDR_MEM_PULSE](VDR_MEM_READ,pDst);//����ϵ��
    pDst += 2;
}
/**
  * @brief  ������Ϣ
  * @param  None
  * @retval None
  */
void Vdr_Get_CarInfor(VDR_HEAD *pHead)
{
    u8 *pDst=pHead->pbuf;
    
    pHead->len = 41;
    
    memset(pHead->pbuf,0,pHead->len);
    
    Vdr_Fuc_Mem[VDR_MEM_CARCODE](VDR_MEM_READ,pDst);  
    pDst += 17;

    Vdr_Fuc_Mem[VDR_MEM_CARPLATE](VDR_MEM_READ,pDst); 
    pDst += 12;    

    Vdr_Fuc_Mem[VDR_MEM_CARTYPE](VDR_MEM_READ,pDst); 
    pDst += 12;
}
/**
  * @brief  ״̬�ź�
  * @param  None
  * @retval None
  */
void Vdr_Get_StaSignal(VDR_HEAD *pHead)
{
    u8  *pDst=pHead->pbuf;

    pHead->len = 87;
    
    memset(pHead->pbuf,0,pHead->len);
    
    Vdr_Fuc_Mem[VDR_MEM_RTC](VDR_MEM_READ,pDst);      //��ǰʱ��
     pDst  += 6;

    *pDst++ = 1;                                      //״̬����

    Vdr_Fuc_Mem[VDR_MEM_STASIGNAL](VDR_MEM_READ,pDst);//״̬��Ϣ
}
/**
  * @brief  Ψһ���
  * @param  None
  * @retval None
  */
void Vdr_Get_OnlyNum(VDR_HEAD *pHead)
{
    u8  *pDst=pHead->pbuf;
    
    pHead->len = 35;
    
    memset(pHead->pbuf,0,pHead->len);
    
    Vdr_Fuc_Mem[VDR_MEM_ONLY](VDR_MEM_READ,pDst);
}
/**
  * @brief  ���������ַ��ȡ������Ϣ
  * @param  None
  * @retval None
  */
void Vdr_Get_Base(u8 cmdnum,VDR_HEAD *pHead)
{
    pHead->cmd = Vdr_Cmd_Base[cmdnum];//������
    Vdr_Fuc_Base[cmdnum](pHead);      //����
}

/**
  * @brief  ���������ַ��ȡ��¼��Ϣ
  * @param  None
  * @retval None
  */
void Vdr_Get_Ser(u8 cmdnum,VDR_SERT time, VDR_HEAD *pHead)
{
    pHead->cmd = Vdr_Cmd_Ser[cmdnum];                    //������

    pHead->len = Register_Read(cmdnum+1,                 //��������
                               pHead->pbuf,
                               time.t_start,
                               time.t_end,
                               time.maxblock);
}


/**
  * @brief  ��ȡ���ݰ���Ϣ
  * @param  None
  * @retval None
  */
void Vdr_Get_PackageInf(VDR_PACKINF PackInf, VDR_PACKSER *pPackage)
{
    u8  cmdnum   = PackInf.cmdnum;//������
    u8  *p_data  = Vdr_BufData;//��¼�ǹ�������
     
    u32 addrstar = VdrSecAddr[cmdnum*2]*VDR_SECTOR_SIZE;//��ʼ��ַ
    u32 addrend  = VdrSecAddr[cmdnum*2+1]*VDR_SECTOR_SIZE;//������ַ
    u8  steplen  = VdrStepLen[cmdnum];//��������
    u16 upnum    = VDR_PACKAGE_SIZE/VdrUpLen[cmdnum];
    u16 upcnt    = 0;

    u32 stepnum  = VDR_SECTOR_SIZE/steplen;//������������ ��Ч����
    u16 steprem  = VDR_SECTOR_SIZE%steplen;//������������ ��Ч���� ��������������ַ

    u32 forend   = (VdrSecAddr[cmdnum*2+1]-VdrSecAddr[cmdnum*2])*(VDR_SECTOR_SIZE/steplen);//�����ڵ��ܲ���
    u16 stepcnt  = 0;//�������ڵ��������������ж�����

    u32 addr     = 0;      
    u8  fwrflg   = 0;
    u8  datflg   = 0;    
    u8  wrtflg   = 0;
    u8  starflg  = 0;    
    u8  errcnt   = 0;

    u32 steptime;//�洢����ÿ��������32λʱ��
    u32 steptimelast;//�ϸ�ʱ��
    u32 i;

    TIME_T time_t;
    TIME_T timeback_t;  
    u8     timeis1,timeis2;

    memset((u8*)pPackage,0,sizeof(VDR_PACKSER));//��սṹ��
    pPackage->cmd  = Vdr_Cmd_All[cmdnum];//����
    pPackage->pbuf = Vdr_PackBuf;//�����ַ
    
    steptime     = 0;
    steptimelast = 0;
    addr         = addrstar; 
    
    timeback_t.year = 0x55;//��ʼֵ
    
    for(i=0; i<forend; i++)//�������е�������
    {  
        sFLASH_ReadBuffer(p_data,addr,steplen);//��ȡ����

        steptime  = (p_data[0]<<24)|(p_data[1]<<16)|(p_data[2]<<8)|p_data[3];//��ȡ����ʱ��
        Gmtime(&time_t,steptime);//ת��ʱ��

        if(datflg == 0)//������ʼ��ַ
        {
            if(steptime != 0xffffffff)
            {
                if(steptimelast == 0xffffffff)
                {
                    i       = 0;//���±���         
                    datflg  = 1;//�ҵ���ʼ��ַ
                    wrtflg  = 1;//д���ļ�
                    pPackage->cyc = 0;//ѭ��
                    pPackage->addrstar = addr;//��ʼ��ַ
                }
                fwrflg = 1;              
            }
            steptimelast = steptime;//ˢ���ϴ�ʱ��

            if(((i+1) == forend)&&(fwrflg == 1))
            {
                i = 0;//δѭ���洢 ���±���  
                if((errcnt++) > 1)
                {
                    break;//��������Ƿ�
                }                
            }
        }
        else if(steptime != 0xffffffff)//��Ч����
        {
            if(steptime > PackInf.timeend)
            {
                if(upcnt)
                {
                    pPackage->all++;
                }
                break;//�ҵ�������ַ�˳�ѭ��
            }
            else
            {
                wrtflg = 1;
            }
        }
        else//����
        {
            if(upcnt)
            {
                pPackage->all++;
            }
            break;//�ҵ�������ַ�˳�ѭ��
        }
        ////////////////////////////////////////////////////////////////////////
        if((wrtflg)&&(steptime >= PackInf.timestar))
        {
            if(starflg == 0)
            {
                starflg = 1;
                pPackage->addrstar = addr;//��ʼ��ַ
            }
            else
            {
                pPackage->addrend = addr;//������ַ      
            }            
            if(cmdnum == 8)//�ٶ� �������
            {
                timeis1 = time_t.min;
                timeis2 = timeback_t.min;  
                #ifdef USE_ONE_MIN_SAVE_SPEED_RECORD
                cmdnum = 0x55;
                #endif
            }
            else if(cmdnum == 9)//λ�� Сʱ���
            {
                timeis1 = time_t.hour;
                timeis2 = timeback_t.hour;               
            }
            if((cmdnum == 8)||(cmdnum == 9))
            {
                if((timeis1 != timeis2)&&(timeback_t.year != 0x55))
                {
                    if(++upcnt >= upnum)//����
                    {
                        upcnt = 0;
                        pPackage->all++;//������
                    }
                }
                timeback_t = time_t;//ˢ��ʱ��
            }
            else
            {    
                if(++upcnt >= upnum)//����
                {
                    upcnt = 0;
                    pPackage->all++;
                }         
            }               
        }
        wrtflg = 0;
        ////////////////////////////////////////////////////////////////////////
        addr += steplen;//ָ����һ������
        if((++stepcnt) >= stepnum)
        {
            stepcnt = 0;
            addr += steprem;//����������Ч����
        }
        if(addr >= addrend)
        {
            addr = addrstar;//����������ͷ������
            pPackage->cyc = 1;//ѭ��
        }
        IWDG_ReloadCounter();//ι��
    }
}
/**
  * @brief  ���ݰ�����
  * @param  None
  * @retval None
  */
void Vdr_Get_PackageSer(VDR_PACKSER *pPackage)
{
    u8  cmdnum   = Vdr_Cmd_To(pPackage->cmd);//������
    u8  *p_send  = pPackage->pbuf;
    
    u32 addrstar = VdrSecAddr[cmdnum*2]*VDR_SECTOR_SIZE;//��ʼ��ַ
    u32 addrend  = VdrSecAddr[cmdnum*2+1]*VDR_SECTOR_SIZE;//������ַ
    u8  steplen  = VdrStepLen[cmdnum];//��������
    u16 upnum    = VDR_PACKAGE_SIZE/VdrUpLen[cmdnum];
    u16 upcnt    = 0;

    u32 stepnum  = VDR_SECTOR_SIZE/steplen;//������������ ��Ч����
    u16 steprem  = VDR_SECTOR_SIZE%steplen;//������������ ��Ч���� ��������������ַ

    u32 addr     = pPackage->addrstar;  
    u32 steptime = 0;//�洢����ÿ��������32λʱ��

    u32 backaddr = 0;      
    u16 backstep = 0;
    
    TIME_T time_t;
    TIME_T timeback_t;
    u8     timesv;
    u8     timesz;   
    u8     timeis1,timeis2;     

    u8  buf[VDR_STEP_SIZE];
    u8  spdmin;//�ٶȰ����Ӵ洢��־

    if(pPackage->all == 0)
    {
        return ;//�޷������
    }
    
    memset(p_send,0xff,VdrUpLen[cmdnum]*upnum);//��仺��
    timeback_t.year = 0x55;//��ʼֵ
    pPackage->len   = 0;
    
    for(;;)//�������е�������
    {  
        if(pPackage->addrstar > pPackage->addrend)
        {      
            if(pPackage->cyc == 0)
            {
                if((pPackage->cnt < pPackage->all)||(upcnt))
                {
                    pPackage->cnt++;
                }
                break;//�˳�
            }
        }
        sFLASH_ReadBuffer(buf,addr,steplen);//��ȡ����

        backaddr = addr;
        backstep = pPackage->stepcnt;

        steptime  = (buf[0]<<24)|(buf[1]<<16)|(buf[2]<<8)|buf[3];//��ȡ����ʱ��
        Gmtime(&time_t,steptime);//ת��ʱ��

        ////////////////////////////////////////////////////////////////////////
        addr += steplen;//ָ����һ������
        if((++pPackage->stepcnt) >= stepnum)
        {
            pPackage->stepcnt = 0;
            addr += steprem;//����������Ч����
        }
        if(addr >= addrend)
        {
            addr = addrstar;//����������ͷ������
        }
        pPackage->addrstar = addr;
        
        ////////////////////////////////////////////////////////////////////////
        if(cmdnum == 8)//�ٶ� �������
        {
            timesz  = 2;        
            timesv  = time_t.sec;
            timeis1 = time_t.min;
            timeis2 = timeback_t.min; 
            spdmin  = 0;
            #ifdef USE_ONE_MIN_SAVE_SPEED_RECORD
            spdmin  = 1;
            #endif            
        }
        else if(cmdnum == 9)//λ�� Сʱ���
        {
            timesz  = 11;          
            timesv  = time_t.min;
            timeis1 = time_t.hour;
            timeis2 = timeback_t.hour;               
        }
        if(((cmdnum == 8)||(cmdnum == 9))&&(spdmin == 0))
        {                
            ////////////////////////////////////////////////////////////////////
            if((timeis1 != timeis2)&&(timeback_t.year != 0x55))
            {
                Public_ConvertTimeToBCDEx(timeback_t,p_send);
                p_send[5] = 0;//������ 
                if(timesz == 11)
                {
                    p_send[4] = 0;//������ 
                }
                pPackage->len += VdrUpLen[cmdnum];//�����ۼ�
                if(++upcnt >= upnum)//����
                {
                    upcnt = 0;
                    pPackage->cnt++;//������
                    pPackage->addrstar = backaddr;
                    pPackage->stepcnt  = backstep;                    
                    break;
                }
                p_send += VdrUpLen[cmdnum];//ת����һ��  
            }
            memcpy(p_send+6+(timesv*timesz),buf+4,timesz);//��������
            timeback_t = time_t;//ˢ��ʱ��
        }
        else
        {
            pPackage->len += VdrUpLen[cmdnum];//�����ۼ�            
            memcpy(p_send,buf+4,VdrUpLen[cmdnum]);          
            if(++upcnt >= upnum)//����
            {
                upcnt = 0;
                pPackage->cnt++;//������
                break;
            }
            p_send += VdrUpLen[cmdnum];//ת����һ��             
        }
        IWDG_ReloadCounter();//ι��
    }
    if((pPackage->cyc)&&(pPackage->addrstar < pPackage->addrend))
    {
        pPackage->cyc = 0;
    }
}












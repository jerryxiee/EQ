/*
********************************************************************************
*
*
* Filename      : vdr_mem.c
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
const VDR_FUC_MEM Vdr_Fuc_Mem[]=//��¼�Ǵ洢����
{
 Vdr_Mem_DvrInfor,
 Vdr_Mem_TimeInit, 
 Vdr_Mem_MileInit,
 Vdr_Mem_MileAdd,
 Vdr_Mem_Pulse,
 Vdr_Mem_Carcode,
 Vdr_Mem_Carplate,
 Vdr_Mem_CarType,
 Vdr_Mem_StaSignal,
 Vdr_Mem_Only,
 Vdr_Mem_Rtc,
};

const u8 Vdr_Code_StaSignal[][10]=//��������
{
 "�Զ���    ",
 "�Զ���    ",
 "�Զ���    ",
 "����      ",
 "Զ��      ",
 "��ת��    ",
 "��ת��    ",
 "�ƶ�      ",
};

/*
********************************************************************************
*                          LOCAL VARIABLES
********************************************************************************
*/


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
  * @brief  �Ӵ洢���ж�������
  * @param  type->��������
            pSrc->Դ����
  * @retval �������ݳ���
  */
u8   Vdr_Mem_E2promRd(VDR_MEM type, u8 *pSrc)
{
    u8  lenok;//Ӧ���صĳ���
    u8  lenrd;//ʵ�ʵĳ���

    switch(type)
    {
        case VDR_MEM_DVRINFOR://��ʻԱ��Ϣ
            {
                lenrd = EepromPram_ReadPram(E2_CURRENT_LICENSE_ID,pSrc);
                lenok = lenrd;
                break;
            }
        case VDR_MEM_TIMEINIT://���ΰ�װʱ��
            {
                lenrd = EepromPram_ReadPram(E2_INSTALL_TIME_ID,pSrc);
                lenok = E2_INSTALL_TIME_ID_LEN;                
                break;
            }
        case VDR_MEM_MILEINIT://��ʼ�����
            {
                lenrd = EepromPram_ReadPram(E2_CAR_INIT_MILE_ID,pSrc);
                lenok = E2_CAR_INIT_MILE_ID_LEN;
                break;
            }
        case VDR_MEM_MILEADD://�ۼ����
            {
                *pSrc++ =  Public_GetCurTotalMile()>>24; 
                *pSrc++ = (Public_GetCurTotalMile()>>16)&0xff; 
                *pSrc++ = (Public_GetCurTotalMile()>>8)&0xff; 
                *pSrc++ =  Public_GetCurTotalMile()&0xff; 
                lenrd = 4; 
                lenok = 4;
                break;
            }  
        case VDR_MEM_PULSE://����ϵ��
            {
                lenrd = EepromPram_ReadPram(E2_CAR_FEATURE_COEF_ID,pSrc);
                lenok = E2_CAR_FEATURE_COEF_LEN;
                break;
            }
        case VDR_MEM_CARCODE://����ʶ����
            {
                lenrd = EepromPram_ReadPram(E2_CAR_IDENTIFICATION_CODE_ID,pSrc);
                lenok = lenrd;
                break;
            }
        case VDR_MEM_CARPLATE://���ƺ�
            {
                lenrd = EepromPram_ReadPram(E2_CAR_PLATE_NUM_ID,pSrc);
                lenok = lenrd;
                break;
            }
        case VDR_MEM_CARTYPE://��������
            {
                lenrd = EepromPram_ReadPram(E2_CAR_TYPE_ID,pSrc);
                lenok = lenrd;
                break;
            } 
        case VDR_MEM_STASIGNAL://״̬�ź�
            {
                lenrd = 80;
                lenok = lenrd;
                sFLASH_ReadBuffer(pSrc,FLASH_STATUS_SET_SECTOR,80);
                break;
            }
        case VDR_MEM_ONLY://Ψһ�Ա��
            {
                lenrd = EepromPram_ReadPram(E2_CAR_ONLY_NUM_ID,pSrc);
                lenok = E2_CAR_ONLY_NUM_ID_LEN;
                break;
            }          
        default:
            {
                lenrd = 0;
                break;
            }
        
    }
    if(lenok != lenrd)//�жϳ����Ƿ���ȷ
    {
        lenrd = 0;
    }
    return lenrd;
}
/**
  * @brief  ��洢����д������
  * @param  type->��������
            pSrc->Դ����
  * @retval �������ݳ���
  */
u8   Vdr_Mem_E2promWr(VDR_MEM type, u8 *pSrc)
{
    u8  lenok;//Ӧ���صĳ���
    u8  lenrd;//ʵ�ʵĳ���

    switch(type)
    {
        case VDR_MEM_TIMEINIT://���ΰ�װʱ��
            {
                lenok = E2_INSTALL_TIME_ID_LEN;
                lenrd = EepromPram_WritePram(E2_INSTALL_TIME_ID, pSrc, lenok);                
                break;
            }
        case VDR_MEM_MILEINIT://��ʼ�����
            {
                lenok = E2_CAR_INIT_MILE_ID_LEN;
                lenrd = EepromPram_WritePram(E2_CAR_INIT_MILE_ID, pSrc, lenok);                  
                break;
            }
        case VDR_MEM_PULSE://����ϵ��
            {
                lenok = E2_CAR_FEATURE_COEF_LEN;
                lenrd = EepromPram_WritePram(E2_CAR_FEATURE_COEF_ID, pSrc, lenok);                 
                break;
            }
        case VDR_MEM_CARCODE://����ʶ����
            {
                lenok = 17;
                lenrd = EepromPram_WritePram(E2_CAR_IDENTIFICATION_CODE_ID,pSrc,lenok);                 
                break;
            }
        case VDR_MEM_CARPLATE://���ƺ�
            {
                lenok = strlen((char*)pSrc);
                lenrd = EepromPram_WritePram(E2_CAR_PLATE_NUM_ID, pSrc, lenok);                      
                break;
            }
        case VDR_MEM_CARTYPE://��������
            {
                lenok = strlen((char*)pSrc);
                lenrd = EepromPram_WritePram(E2_CAR_TYPE_ID, pSrc, lenok);                  
                break;
            } 
        case VDR_MEM_STASIGNAL://״̬�ź�
            {
                lenok = 80;
                lenrd = lenok;
                sFLASH_EraseSector(FLASH_STATUS_SET_SECTOR);
	            sFLASH_WriteBuffer(pSrc,FLASH_STATUS_SET_SECTOR,lenok);
                break;
            }
        case VDR_MEM_ONLY://Ψһ�Ա��
            {
                lenok = E2_CAR_ONLY_NUM_ID_LEN;
                lenrd = EepromPram_WritePram(E2_CAR_ONLY_NUM_ID, pSrc, lenok);                 
                break;
            }
        default:
            {
                lenrd = 0;
                break;
            }
        
    }
    if(lenok != lenrd)//�жϳ����Ƿ���ȷ
    {
        lenrd = 0;
    }
    return lenrd;
}
/**
  * @brief  ��ʻԱ��Ϣ(18Byte)
  * @param  ��д���� ��->VDR_MEM_READ 
  *                  д->VDR_MEM_WRITE
  * @retval �������ݳ���
  */ 
u8 Vdr_Mem_DvrInfor(u8 type, u8 *pSrc)
{
    //////////////////////////////////////////////////////////////////////д����
    if(type == VDR_MEM_WRITE)
    {    
        Vdr_Mem_E2promWr(VDR_MEM_DVRINFOR,pSrc);      
    }
    //////////////////////////////////////////////////////////////////////������    
    else
    {
        if(Vdr_Mem_E2promRd(VDR_MEM_DVRINFOR,pSrc)==0)
        {
            //////////////////////////////////////////////////////////////ȱʡֵ
            memset(pSrc,0,18);//Ĭ������
        }
    }
    return 18;
}
/**
  * @brief  ���ΰ�װʱ��(6Byte)
  * @param  ��д���� ��->VDR_MEM_READ 
  *                  д->VDR_MEM_WRITE
  * @retval �������ݳ���
  */ 
u8 Vdr_Mem_TimeInit(u8 type, u8 *pSrc)
{
    TIME_T time;
    u8  buf[5];
    u32 temp;    
    
    //////////////////////////////////////////////////////////////////////д����
    if(type == VDR_MEM_WRITE)
    {
        Public_ConvertBCDToTimeEx(&time,pSrc);
        temp = ConverseGmtime(&time);
        buf[0] = (temp>>24)&0xff;
        buf[1] = (temp>>16)&0xff;
        buf[2] = (temp>>8) &0xff;  
        buf[3] =  temp&0xff;      
        Vdr_Mem_E2promWr(VDR_MEM_TIMEINIT,buf);  
    }
    //////////////////////////////////////////////////////////////////////������     
    else
    {   
        if(Vdr_Mem_E2promRd(VDR_MEM_TIMEINIT,buf)==0)
        {
            //////////////////////////////////////////////////////////////ȱʡֵ
            memset(buf,0,4);
        }
        temp  = buf[0]<<24;
        temp |= buf[1]<<16;   
        temp |= buf[2]<<8; 
        temp |= buf[3]; 
        Gmtime(&time,temp);
        Public_ConvertTimeToBCDEx(time,pSrc);
    }
    return 6;
}
/**
  * @brief  ��ʼ�����(4Byte)
  * @param  ��д���� ��->VDR_MEM_READ 
  *                  д->VDR_MEM_WRITE
  * @retval �������ݳ���
  */ 
u8 Vdr_Mem_MileInit(u8 type, u8 *pSrc)
{
    u8  buf[5];
    u32 temp;
    
    //////////////////////////////////////////////////////////////////////д����
    if(type == VDR_MEM_WRITE)
    {
        temp = Public_ConvertBCDToLong(pSrc);
        buf[0] = (temp>>24)&0xff;
        buf[1] = (temp>>16)&0xff;
        buf[2] = (temp>>8) &0xff;  
        buf[3] =  temp&0xff;         
        Vdr_Mem_E2promWr(VDR_MEM_MILEINIT,buf);
    }
    //////////////////////////////////////////////////////////////////////������     
    else
    {
        if(Vdr_Mem_E2promRd(VDR_MEM_MILEINIT,buf)==0)
        {
            //////////////////////////////////////////////////////////////ȱʡֵ
            memset(buf,0,4);//Ĭ������
        }
        temp  = buf[0]<<24;
        temp |= buf[1]<<16;   
        temp |= buf[2]<<8; 
        temp |= buf[3]; 
        Public_ConvertLongToBCD(temp,pSrc);
    }
    return 4;
}
/**
  * @brief  �ۼ����(4Byte)
  * @param  ��д���� ��->VDR_MEM_READ 
  *                  д->VDR_MEM_WRITE
  * @retval �������ݳ���
  */ 
u8 Vdr_Mem_MileAdd(u8 type, u8 *pSrc)
{
    u8  buf[5];
    u32 temp;

    //////////////////////////////////////////////////////////////////////д����
    if(type == VDR_MEM_WRITE)
    {
        temp = Public_ConvertBCDToLong(pSrc);
        buf[0] = (temp>>24)&0xff;
        buf[1] = (temp>>16)&0xff;
        buf[2] = (temp>>8) &0xff;  
        buf[3] =  temp&0xff;  
        Vdr_Mem_E2promWr(VDR_MEM_MILEADD,buf);
    }
    //////////////////////////////////////////////////////////////////////������     
    else
    {
        if(Vdr_Mem_E2promRd(VDR_MEM_MILEADD,buf)==0)
        {
            //////////////////////////////////////////////////////////////ȱʡֵ
            memset(buf,0,4);//Ĭ������
        }
        temp  = buf[0]<<24;
        temp |= buf[1]<<16;   
        temp |= buf[2]<<8; 
        temp |= buf[3]; 
        Public_ConvertLongToBCD(temp,pSrc);
    }
    return 4;
}
/**
  * @brief  ����ϵ��(2Byte)
  * @param  ��д���� ��->VDR_MEM_READ 
  *                  д->VDR_MEM_WRITE
  * @retval �������ݳ���
  */ 
u8 Vdr_Mem_Pulse(u8 type, u8 *pSrc)
{
    u8 buf[5];
    
    //////////////////////////////////////////////////////////////////////д����
    if(type == VDR_MEM_WRITE)
    {
        buf[0]=0;
        memcpy(buf+1,pSrc,2);
        Vdr_Mem_E2promWr(VDR_MEM_PULSE,buf);
    }
    //////////////////////////////////////////////////////////////////////������     
    else
    {
        if(Vdr_Mem_E2promRd(VDR_MEM_PULSE,buf)==0)
        {
            //////////////////////////////////////////////////////////////ȱʡֵ
            *pSrc++ = 0x0e;//Ĭ������ 3600
            *pSrc++ = 0x10;
        }
        else
        {
            memcpy(pSrc,buf+1,2);
        }
    }
    return 2;
}
/**
  * @brief  ����ʶ���(17Byte)
  * @param  ��д���� ��->VDR_MEM_READ 
  *                  д->VDR_MEM_WRITE
  * @retval �������ݳ���
  */ 
u8 Vdr_Mem_Carcode(u8 type, u8 *pSrc)
{
    //////////////////////////////////////////////////////////////////////д����
    if(type == VDR_MEM_WRITE)
    {
        Vdr_Mem_E2promWr(VDR_MEM_CARCODE,pSrc);
    }
    //////////////////////////////////////////////////////////////////////������     
    else
    {
        if(Vdr_Mem_E2promRd(VDR_MEM_CARCODE,pSrc)==0)
        {
            //////////////////////////////////////////////////////////////ȱʡֵ
            memcpy(pSrc,"88888888888888888",17);//Ĭ������
        }
    }
    return 17;
}
/**
  * @brief  ���ƺ���(12Byte ��3������)
  * @param  ��д���� ��->VDR_MEM_READ 
  *                  д->VDR_MEM_WRITE
  * @retval �������ݳ���
  */ 
u8 Vdr_Mem_Carplate(u8 type, u8 *pSrc)
{
    
    //////////////////////////////////////////////////////////////////////д����
    if(type == VDR_MEM_WRITE)
    {
        Vdr_Mem_E2promWr(VDR_MEM_CARPLATE,pSrc);
    }
    //////////////////////////////////////////////////////////////////////������     
    else
    {
        if(Vdr_Mem_E2promRd(VDR_MEM_CARPLATE,pSrc) == 0)
        {
            //////////////////////////////////////////////////////////////ȱʡֵ
            memset(pSrc,0,12);
            memcpy(pSrc,"��B888168",9);
        }
    }
    return 12;
}
/**
  * @brief  ��������(12Byte ��4������)
  * @param  ��д���� ��->VDR_MEM_READ 
  *                  д->VDR_MEM_WRITE
  * @retval �������ݳ���
  */ 
u8 Vdr_Mem_CarType(u8 type, u8 *pSrc)
{
    //////////////////////////////////////////////////////////////////////д����    
    if(type == VDR_MEM_WRITE)
    {
        Vdr_Mem_E2promWr(VDR_MEM_CARTYPE,pSrc);
    }
    //////////////////////////////////////////////////////////////////////������     
    else
    {
        if(Vdr_Mem_E2promRd(VDR_MEM_CARTYPE,pSrc)==0)
        {
            //////////////////////////////////////////////////////////////ȱʡֵ
            memset(pSrc,0,12);
            memcpy(pSrc,"����С��",8);
        }
    }
    return 12;
}

/**
  * @brief  �ź�״̬(80Byte)
  * @param  ��д���� ��->VDR_MEM_READ 
  *                  д->VDR_MEM_WRITE
  * @retval �������ݳ���
  */  
u8 Vdr_Mem_StaSignal(u8 type, u8 *pSrc)
{
    //////////////////////////////////////////////////////////////////////д����
    if(type == VDR_MEM_WRITE)
    {   
        Vdr_Mem_E2promWr(VDR_MEM_STASIGNAL,pSrc);
    }
    //////////////////////////////////////////////////////////////////////������
    else
    {
        if(Vdr_Mem_E2promRd(VDR_MEM_STASIGNAL,pSrc) == 0)
        {
            //////////////////////////////////////////////////////////////ȱʡֵ
            memcpy(pSrc,Vdr_Code_StaSignal,80);
        }
    }
    return 80;
}

/**
  * @brief  Ψһ�Ա��(35Byte)
  *
        CCC��֤���� :1 -7
        ��֤��Ʒ�ź�:8 -23
        ����������  :24-26
        ������ˮ��  :27-30
        ����        :31-35
  *                
  * @param  ��д���� ��->VDR_MEM_READ 
  *                  д->VDR_MEM_WRITE
  * @retval �������ݳ���
  */  
u8 Vdr_Mem_Only(u8 type, u8 *pSrc)
{
    u8  *pBuf=pSrc;

    //////////////////////////////////////////////////////////////////////д����
    if(type == VDR_MEM_WRITE)
    {  
        Vdr_Mem_E2promWr(VDR_MEM_ONLY,pSrc);
    }
    //////////////////////////////////////////////////////////////////////������    
    else
    {
        if(Vdr_Mem_E2promRd(VDR_MEM_ONLY,pSrc)==0)
        {
            //////////////////////////////////////////////////////////////ȱʡֵ
            memcpy(pBuf,VDR_CCC_CODE,7);
            pBuf += 7;
            memcpy(pBuf,VDR_PRO_MODEL,16);
            pBuf += 16;
            pBuf[0]=0x14;
            pBuf[1]=0x07;
            pBuf[2]=0x01;
            pBuf += 3;
            pBuf[0]=0x00;
            pBuf[1]=0x00;
            pBuf[2]=0x55;
            pBuf[3]=0x5a;    
            pBuf += 4;
            memset(pBuf,0,5);     
            pBuf += 5;
        }
    }
    return 35;
}
/**
  * @brief  ʵʱʱ��(3Byte 6λBCD��)
  * @param  ��д���� ��->VDR_MEM_READ 
  *                  д->VDR_MEM_WRITE
  * @retval �������ݳ���
  */
u8 Vdr_Mem_Rtc(u8 type, u8 *pSrc)
{
    TIME_T time_t;
    
    //////////////////////////////////////////////////////////////////////ȱʡֵ

    
    //////////////////////////////////////////////////////////////////////д����
    if(type == VDR_MEM_WRITE)
    {
        if(Public_ConvertBCDToTime(&time_t,pSrc))
    	{
    		SetRtc(&time_t);
    	}
    }
    //////////////////////////////////////////////////////////////////////������   
    else
    {
        Public_ConvertNowTimeToBCDEx(pSrc);
    }
    return 6;
}



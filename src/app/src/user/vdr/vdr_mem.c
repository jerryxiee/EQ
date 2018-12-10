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
const VDR_FUC_MEM Vdr_Fuc_Mem[]=//记录仪存储参数
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

const u8 Vdr_Code_StaSignal[][10]=//所有命令
{
 "自定义    ",
 "自定义    ",
 "自定义    ",
 "近光      ",
 "远光      ",
 "右转向    ",
 "左转向    ",
 "制动      ",
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
  * @brief  从存储器中读出数据
  * @param  type->数据类型
            pSrc->源数据
  * @retval 返回数据长度
  */
u8   Vdr_Mem_E2promRd(VDR_MEM type, u8 *pSrc)
{
    u8  lenok;//应返回的长度
    u8  lenrd;//实际的长度

    switch(type)
    {
        case VDR_MEM_DVRINFOR://驾驶员信息
            {
                lenrd = EepromPram_ReadPram(E2_CURRENT_LICENSE_ID,pSrc);
                lenok = lenrd;
                break;
            }
        case VDR_MEM_TIMEINIT://初次安装时间
            {
                lenrd = EepromPram_ReadPram(E2_INSTALL_TIME_ID,pSrc);
                lenok = E2_INSTALL_TIME_ID_LEN;                
                break;
            }
        case VDR_MEM_MILEINIT://初始化里程
            {
                lenrd = EepromPram_ReadPram(E2_CAR_INIT_MILE_ID,pSrc);
                lenok = E2_CAR_INIT_MILE_ID_LEN;
                break;
            }
        case VDR_MEM_MILEADD://累计里程
            {
                *pSrc++ =  Public_GetCurTotalMile()>>24; 
                *pSrc++ = (Public_GetCurTotalMile()>>16)&0xff; 
                *pSrc++ = (Public_GetCurTotalMile()>>8)&0xff; 
                *pSrc++ =  Public_GetCurTotalMile()&0xff; 
                lenrd = 4; 
                lenok = 4;
                break;
            }  
        case VDR_MEM_PULSE://脉冲系数
            {
                lenrd = EepromPram_ReadPram(E2_CAR_FEATURE_COEF_ID,pSrc);
                lenok = E2_CAR_FEATURE_COEF_LEN;
                break;
            }
        case VDR_MEM_CARCODE://车辆识别码
            {
                lenrd = EepromPram_ReadPram(E2_CAR_IDENTIFICATION_CODE_ID,pSrc);
                lenok = lenrd;
                break;
            }
        case VDR_MEM_CARPLATE://车牌号
            {
                lenrd = EepromPram_ReadPram(E2_CAR_PLATE_NUM_ID,pSrc);
                lenok = lenrd;
                break;
            }
        case VDR_MEM_CARTYPE://车辆类型
            {
                lenrd = EepromPram_ReadPram(E2_CAR_TYPE_ID,pSrc);
                lenok = lenrd;
                break;
            } 
        case VDR_MEM_STASIGNAL://状态信号
            {
                lenrd = 80;
                lenok = lenrd;
                sFLASH_ReadBuffer(pSrc,FLASH_STATUS_SET_SECTOR,80);
                break;
            }
        case VDR_MEM_ONLY://唯一性编号
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
    if(lenok != lenrd)//判断长度是否正确
    {
        lenrd = 0;
    }
    return lenrd;
}
/**
  * @brief  向存储器中写入数据
  * @param  type->数据类型
            pSrc->源数据
  * @retval 返回数据长度
  */
u8   Vdr_Mem_E2promWr(VDR_MEM type, u8 *pSrc)
{
    u8  lenok;//应返回的长度
    u8  lenrd;//实际的长度

    switch(type)
    {
        case VDR_MEM_TIMEINIT://初次安装时间
            {
                lenok = E2_INSTALL_TIME_ID_LEN;
                lenrd = EepromPram_WritePram(E2_INSTALL_TIME_ID, pSrc, lenok);                
                break;
            }
        case VDR_MEM_MILEINIT://初始化里程
            {
                lenok = E2_CAR_INIT_MILE_ID_LEN;
                lenrd = EepromPram_WritePram(E2_CAR_INIT_MILE_ID, pSrc, lenok);                  
                break;
            }
        case VDR_MEM_PULSE://脉冲系数
            {
                lenok = E2_CAR_FEATURE_COEF_LEN;
                lenrd = EepromPram_WritePram(E2_CAR_FEATURE_COEF_ID, pSrc, lenok);                 
                break;
            }
        case VDR_MEM_CARCODE://车辆识别码
            {
                lenok = 17;
                lenrd = EepromPram_WritePram(E2_CAR_IDENTIFICATION_CODE_ID,pSrc,lenok);                 
                break;
            }
        case VDR_MEM_CARPLATE://车牌号
            {
                lenok = strlen((char*)pSrc);
                lenrd = EepromPram_WritePram(E2_CAR_PLATE_NUM_ID, pSrc, lenok);                      
                break;
            }
        case VDR_MEM_CARTYPE://车辆类型
            {
                lenok = strlen((char*)pSrc);
                lenrd = EepromPram_WritePram(E2_CAR_TYPE_ID, pSrc, lenok);                  
                break;
            } 
        case VDR_MEM_STASIGNAL://状态信号
            {
                lenok = 80;
                lenrd = lenok;
                sFLASH_EraseSector(FLASH_STATUS_SET_SECTOR);
	            sFLASH_WriteBuffer(pSrc,FLASH_STATUS_SET_SECTOR,lenok);
                break;
            }
        case VDR_MEM_ONLY://唯一性编号
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
    if(lenok != lenrd)//判断长度是否正确
    {
        lenrd = 0;
    }
    return lenrd;
}
/**
  * @brief  驾驶员信息(18Byte)
  * @param  读写类型 读->VDR_MEM_READ 
  *                  写->VDR_MEM_WRITE
  * @retval 返回数据长度
  */ 
u8 Vdr_Mem_DvrInfor(u8 type, u8 *pSrc)
{
    //////////////////////////////////////////////////////////////////////写数据
    if(type == VDR_MEM_WRITE)
    {    
        Vdr_Mem_E2promWr(VDR_MEM_DVRINFOR,pSrc);      
    }
    //////////////////////////////////////////////////////////////////////读数据    
    else
    {
        if(Vdr_Mem_E2promRd(VDR_MEM_DVRINFOR,pSrc)==0)
        {
            //////////////////////////////////////////////////////////////缺省值
            memset(pSrc,0,18);//默认数据
        }
    }
    return 18;
}
/**
  * @brief  初次安装时间(6Byte)
  * @param  读写类型 读->VDR_MEM_READ 
  *                  写->VDR_MEM_WRITE
  * @retval 返回数据长度
  */ 
u8 Vdr_Mem_TimeInit(u8 type, u8 *pSrc)
{
    TIME_T time;
    u8  buf[5];
    u32 temp;    
    
    //////////////////////////////////////////////////////////////////////写数据
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
    //////////////////////////////////////////////////////////////////////读数据     
    else
    {   
        if(Vdr_Mem_E2promRd(VDR_MEM_TIMEINIT,buf)==0)
        {
            //////////////////////////////////////////////////////////////缺省值
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
  * @brief  初始化里程(4Byte)
  * @param  读写类型 读->VDR_MEM_READ 
  *                  写->VDR_MEM_WRITE
  * @retval 返回数据长度
  */ 
u8 Vdr_Mem_MileInit(u8 type, u8 *pSrc)
{
    u8  buf[5];
    u32 temp;
    
    //////////////////////////////////////////////////////////////////////写数据
    if(type == VDR_MEM_WRITE)
    {
        temp = Public_ConvertBCDToLong(pSrc);
        buf[0] = (temp>>24)&0xff;
        buf[1] = (temp>>16)&0xff;
        buf[2] = (temp>>8) &0xff;  
        buf[3] =  temp&0xff;         
        Vdr_Mem_E2promWr(VDR_MEM_MILEINIT,buf);
    }
    //////////////////////////////////////////////////////////////////////读数据     
    else
    {
        if(Vdr_Mem_E2promRd(VDR_MEM_MILEINIT,buf)==0)
        {
            //////////////////////////////////////////////////////////////缺省值
            memset(buf,0,4);//默认数据
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
  * @brief  累计里程(4Byte)
  * @param  读写类型 读->VDR_MEM_READ 
  *                  写->VDR_MEM_WRITE
  * @retval 返回数据长度
  */ 
u8 Vdr_Mem_MileAdd(u8 type, u8 *pSrc)
{
    u8  buf[5];
    u32 temp;

    //////////////////////////////////////////////////////////////////////写数据
    if(type == VDR_MEM_WRITE)
    {
        temp = Public_ConvertBCDToLong(pSrc);
        buf[0] = (temp>>24)&0xff;
        buf[1] = (temp>>16)&0xff;
        buf[2] = (temp>>8) &0xff;  
        buf[3] =  temp&0xff;  
        Vdr_Mem_E2promWr(VDR_MEM_MILEADD,buf);
    }
    //////////////////////////////////////////////////////////////////////读数据     
    else
    {
        if(Vdr_Mem_E2promRd(VDR_MEM_MILEADD,buf)==0)
        {
            //////////////////////////////////////////////////////////////缺省值
            memset(buf,0,4);//默认数据
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
  * @brief  脉冲系数(2Byte)
  * @param  读写类型 读->VDR_MEM_READ 
  *                  写->VDR_MEM_WRITE
  * @retval 返回数据长度
  */ 
u8 Vdr_Mem_Pulse(u8 type, u8 *pSrc)
{
    u8 buf[5];
    
    //////////////////////////////////////////////////////////////////////写数据
    if(type == VDR_MEM_WRITE)
    {
        buf[0]=0;
        memcpy(buf+1,pSrc,2);
        Vdr_Mem_E2promWr(VDR_MEM_PULSE,buf);
    }
    //////////////////////////////////////////////////////////////////////读数据     
    else
    {
        if(Vdr_Mem_E2promRd(VDR_MEM_PULSE,buf)==0)
        {
            //////////////////////////////////////////////////////////////缺省值
            *pSrc++ = 0x0e;//默认数据 3600
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
  * @brief  车牌识别号(17Byte)
  * @param  读写类型 读->VDR_MEM_READ 
  *                  写->VDR_MEM_WRITE
  * @retval 返回数据长度
  */ 
u8 Vdr_Mem_Carcode(u8 type, u8 *pSrc)
{
    //////////////////////////////////////////////////////////////////////写数据
    if(type == VDR_MEM_WRITE)
    {
        Vdr_Mem_E2promWr(VDR_MEM_CARCODE,pSrc);
    }
    //////////////////////////////////////////////////////////////////////读数据     
    else
    {
        if(Vdr_Mem_E2promRd(VDR_MEM_CARCODE,pSrc)==0)
        {
            //////////////////////////////////////////////////////////////缺省值
            memcpy(pSrc,"88888888888888888",17);//默认数据
        }
    }
    return 17;
}
/**
  * @brief  车牌号码(12Byte 后3个备用)
  * @param  读写类型 读->VDR_MEM_READ 
  *                  写->VDR_MEM_WRITE
  * @retval 返回数据长度
  */ 
u8 Vdr_Mem_Carplate(u8 type, u8 *pSrc)
{
    
    //////////////////////////////////////////////////////////////////////写数据
    if(type == VDR_MEM_WRITE)
    {
        Vdr_Mem_E2promWr(VDR_MEM_CARPLATE,pSrc);
    }
    //////////////////////////////////////////////////////////////////////读数据     
    else
    {
        if(Vdr_Mem_E2promRd(VDR_MEM_CARPLATE,pSrc) == 0)
        {
            //////////////////////////////////////////////////////////////缺省值
            memset(pSrc,0,12);
            memcpy(pSrc,"粤B888168",9);
        }
    }
    return 12;
}
/**
  * @brief  车辆类型(12Byte 后4个备用)
  * @param  读写类型 读->VDR_MEM_READ 
  *                  写->VDR_MEM_WRITE
  * @retval 返回数据长度
  */ 
u8 Vdr_Mem_CarType(u8 type, u8 *pSrc)
{
    //////////////////////////////////////////////////////////////////////写数据    
    if(type == VDR_MEM_WRITE)
    {
        Vdr_Mem_E2promWr(VDR_MEM_CARTYPE,pSrc);
    }
    //////////////////////////////////////////////////////////////////////读数据     
    else
    {
        if(Vdr_Mem_E2promRd(VDR_MEM_CARTYPE,pSrc)==0)
        {
            //////////////////////////////////////////////////////////////缺省值
            memset(pSrc,0,12);
            memcpy(pSrc,"机动小车",8);
        }
    }
    return 12;
}

/**
  * @brief  信号状态(80Byte)
  * @param  读写类型 读->VDR_MEM_READ 
  *                  写->VDR_MEM_WRITE
  * @retval 返回数据长度
  */  
u8 Vdr_Mem_StaSignal(u8 type, u8 *pSrc)
{
    //////////////////////////////////////////////////////////////////////写数据
    if(type == VDR_MEM_WRITE)
    {   
        Vdr_Mem_E2promWr(VDR_MEM_STASIGNAL,pSrc);
    }
    //////////////////////////////////////////////////////////////////////读数据
    else
    {
        if(Vdr_Mem_E2promRd(VDR_MEM_STASIGNAL,pSrc) == 0)
        {
            //////////////////////////////////////////////////////////////缺省值
            memcpy(pSrc,Vdr_Code_StaSignal,80);
        }
    }
    return 80;
}

/**
  * @brief  唯一性编号(35Byte)
  *
        CCC认证代码 :1 -7
        认证产品信号:8 -23
        生产年月日  :24-26
        生产流水号  :27-30
        备用        :31-35
  *                
  * @param  读写类型 读->VDR_MEM_READ 
  *                  写->VDR_MEM_WRITE
  * @retval 返回数据长度
  */  
u8 Vdr_Mem_Only(u8 type, u8 *pSrc)
{
    u8  *pBuf=pSrc;

    //////////////////////////////////////////////////////////////////////写数据
    if(type == VDR_MEM_WRITE)
    {  
        Vdr_Mem_E2promWr(VDR_MEM_ONLY,pSrc);
    }
    //////////////////////////////////////////////////////////////////////读数据    
    else
    {
        if(Vdr_Mem_E2promRd(VDR_MEM_ONLY,pSrc)==0)
        {
            //////////////////////////////////////////////////////////////缺省值
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
  * @brief  实时时间(3Byte 6位BCD码)
  * @param  读写类型 读->VDR_MEM_READ 
  *                  写->VDR_MEM_WRITE
  * @retval 返回数据长度
  */
u8 Vdr_Mem_Rtc(u8 type, u8 *pSrc)
{
    TIME_T time_t;
    
    //////////////////////////////////////////////////////////////////////缺省值

    
    //////////////////////////////////////////////////////////////////////写数据
    if(type == VDR_MEM_WRITE)
    {
        if(Public_ConvertBCDToTime(&time_t,pSrc))
    	{
    		SetRtc(&time_t);
    	}
    }
    //////////////////////////////////////////////////////////////////////读数据   
    else
    {
        Public_ConvertNowTimeToBCDEx(pSrc);
    }
    return 6;
}



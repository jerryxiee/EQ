/*
********************************************************************************
*
*
* Filename      : vdr_set.c
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
const VDR_FUC_SET Vdr_Fuc_Set[]=//设置记录信息
{
 Vdr_Set_CarInfor,
 Vdr_Set_TimeInit,
 Vdr_Set_StaSignal,
 Vdr_Set_Rtc,
 Vdr_Set_Pulse,
 Vdr_Set_MileInit,
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
  * @brief  设置车辆信息
  * @param  None
  * @retval None
  */
void Vdr_Set_CarInfor(u8 *pSrc)
{
    Vdr_Fuc_Mem[VDR_MEM_CARCODE](VDR_MEM_WRITE,pSrc);//车辆识别号
    pSrc += 17;

    Vdr_Fuc_Mem[VDR_MEM_CARPLATE](VDR_MEM_WRITE,pSrc);//机动车牌号 后3个字节备用
    pSrc += 12;

    Vdr_Fuc_Mem[VDR_MEM_CARTYPE](VDR_MEM_WRITE,pSrc);//机动车牌分类 后4个字节备用
    pSrc += 12;
}
/**
  * @brief  初次安装时间
  * @param  None
  * @retval None
  */
void Vdr_Set_TimeInit(u8 *pSrc)
{
    Vdr_Fuc_Mem[VDR_MEM_TIMEINIT](VDR_MEM_WRITE,pSrc);
}
/**
  * @brief  状态量
  * @param  None
  * @retval None
  */
void Vdr_Set_StaSignal(u8 *pSrc)
{
    Vdr_Fuc_Mem[VDR_MEM_STASIGNAL](VDR_MEM_WRITE,pSrc);
}
/**
  * @brief  设置记录仪时间
  * @param  None
  * @retval None
  */
void Vdr_Set_Rtc(u8 *pSrc)
{
    Vdr_Fuc_Mem[VDR_MEM_RTC](VDR_MEM_WRITE,pSrc);
}
/**
  * @brief  设置脉冲系数
  * @param  None
  * @retval None
  */
void Vdr_Set_Pulse(u8 *pSrc)
{
    Vdr_Fuc_Mem[VDR_MEM_RTC](VDR_MEM_WRITE,pSrc);//实时时间
    pSrc += 6;
    
    Vdr_Fuc_Mem[VDR_MEM_PULSE](VDR_MEM_WRITE,pSrc);//脉冲系数
}
/**
  * @brief  设置初始化里程
  * @param  None
  * @retval None
  */
void Vdr_Set_MileInit(u8 *pSrc)
{
    Vdr_Fuc_Mem[VDR_MEM_MILEINIT](VDR_MEM_WRITE,pSrc);
}
/**
  * @brief  参数修改设置
  * @param  None
  * @retval None
  */
void Vdr_Set_Value(u8 cmdnum)
{
    u8 tab[20];
    u32 tiemcnt;
    
    memset(tab,0,sizeof(tab)); 
    Public_ConvertNowTimeToBCDEx(tab);
    tab[6] = Vdr_Cmd_Set[cmdnum];

    tiemcnt = Timer_Val();
    Register_Write2(REGISTER_TYPE_POSITION,tab,13,tiemcnt);
}
/**
  * @brief  
  * @param  None
  * @retval None
  */
void Vdr_Set_Pro(u8 cmdnum, u8 *pSrc, VDR_HEAD *pHead)
{
    Vdr_Fuc_Set[cmdnum](pSrc);       //参数设置
    
    pHead->cmd = Vdr_Cmd_Set[cmdnum];//命令字
    pHead->len = 0;                  //长度  
    
    Vdr_Set_Value(cmdnum);           //参数修改记录
}

/**
  * @brief  伊爱参数读取、设置
  * @param  None
  * @retval None
  */
void Vdr_Eye_Value(VDR_HEAD *pHeadRx,VDR_HEAD *pHeadTx)
{
    s16 DstLen;

    pHeadTx->cmd     = pHeadRx->cmd;    //复制命令
    pHeadTx->pbuf[0] = pHeadRx->pbuf[0];//复制类型
    pHeadRx->len    -= 1;
    
    if(pHeadRx->pbuf[0] == 0)//读取
    {
        RadioProtocol_ReadPram(pHeadTx->pbuf+1,&DstLen,
                               pHeadRx->pbuf+1,pHeadRx->len,0);
        DstLen++;
    }
    else//设置
    {
        RadioProtocol_WritePram(0,pHeadRx->pbuf+1,pHeadRx->len);
        DstLen = 1;
    }
    
    pHeadTx->len = DstLen;//长度
}







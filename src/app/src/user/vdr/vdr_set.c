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
const VDR_FUC_SET Vdr_Fuc_Set[]=//���ü�¼��Ϣ
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
  * @brief  ���ó�����Ϣ
  * @param  None
  * @retval None
  */
void Vdr_Set_CarInfor(u8 *pSrc)
{
    Vdr_Fuc_Mem[VDR_MEM_CARCODE](VDR_MEM_WRITE,pSrc);//����ʶ���
    pSrc += 17;

    Vdr_Fuc_Mem[VDR_MEM_CARPLATE](VDR_MEM_WRITE,pSrc);//�������ƺ� ��3���ֽڱ���
    pSrc += 12;

    Vdr_Fuc_Mem[VDR_MEM_CARTYPE](VDR_MEM_WRITE,pSrc);//�������Ʒ��� ��4���ֽڱ���
    pSrc += 12;
}
/**
  * @brief  ���ΰ�װʱ��
  * @param  None
  * @retval None
  */
void Vdr_Set_TimeInit(u8 *pSrc)
{
    Vdr_Fuc_Mem[VDR_MEM_TIMEINIT](VDR_MEM_WRITE,pSrc);
}
/**
  * @brief  ״̬��
  * @param  None
  * @retval None
  */
void Vdr_Set_StaSignal(u8 *pSrc)
{
    Vdr_Fuc_Mem[VDR_MEM_STASIGNAL](VDR_MEM_WRITE,pSrc);
}
/**
  * @brief  ���ü�¼��ʱ��
  * @param  None
  * @retval None
  */
void Vdr_Set_Rtc(u8 *pSrc)
{
    Vdr_Fuc_Mem[VDR_MEM_RTC](VDR_MEM_WRITE,pSrc);
}
/**
  * @brief  ��������ϵ��
  * @param  None
  * @retval None
  */
void Vdr_Set_Pulse(u8 *pSrc)
{
    Vdr_Fuc_Mem[VDR_MEM_RTC](VDR_MEM_WRITE,pSrc);//ʵʱʱ��
    pSrc += 6;
    
    Vdr_Fuc_Mem[VDR_MEM_PULSE](VDR_MEM_WRITE,pSrc);//����ϵ��
}
/**
  * @brief  ���ó�ʼ�����
  * @param  None
  * @retval None
  */
void Vdr_Set_MileInit(u8 *pSrc)
{
    Vdr_Fuc_Mem[VDR_MEM_MILEINIT](VDR_MEM_WRITE,pSrc);
}
/**
  * @brief  �����޸�����
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
    Vdr_Fuc_Set[cmdnum](pSrc);       //��������
    
    pHead->cmd = Vdr_Cmd_Set[cmdnum];//������
    pHead->len = 0;                  //����  
    
    Vdr_Set_Value(cmdnum);           //�����޸ļ�¼
}

/**
  * @brief  ����������ȡ������
  * @param  None
  * @retval None
  */
void Vdr_Eye_Value(VDR_HEAD *pHeadRx,VDR_HEAD *pHeadTx)
{
    s16 DstLen;

    pHeadTx->cmd     = pHeadRx->cmd;    //��������
    pHeadTx->pbuf[0] = pHeadRx->pbuf[0];//��������
    pHeadRx->len    -= 1;
    
    if(pHeadRx->pbuf[0] == 0)//��ȡ
    {
        RadioProtocol_ReadPram(pHeadTx->pbuf+1,&DstLen,
                               pHeadRx->pbuf+1,pHeadRx->len,0);
        DstLen++;
    }
    else//����
    {
        RadioProtocol_WritePram(0,pHeadRx->pbuf+1,pHeadRx->len);
        DstLen = 1;
    }
    
    pHeadTx->len = DstLen;//����
}







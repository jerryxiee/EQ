/*
********************************************************************************
*
*                                                              
*
* Filename      : JTOther.c
* Version       : V1.00
* Programmer(s) : miaoyahan
* @date         : 2014-11-08  
* @note         : ��ͨ����,���ͨ���ر���
* @ver -> V1.00 : ->����������ⲿ������,��Ч��ر�����;
              
*
********************************************************************************
*/

/*
********************************************************************************
*                               INCLUDES
********************************************************************************
*/
#ifdef EYE_MODEM
#include "include.h"
#endif

/*
********************************************************************************
*                               DEFIEN START
********************************************************************************
*/

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
*                          LOCAL VARIABLES
********************************************************************************
*/
// 0:���� 1:Զ�� 2:��ת 3:��ת 4:ɲ�� 5:ACC 6:��ɲ 7:���� 8:�ٶ�
static u8  JT_IoSta[9];
static u8  JT_IoScn[9];
static u8  JT_IoTm;//IO���ʱ��

const u8 JT_IoStaHz[][10]=
{
"����:��Ч ",
"Զ��:��Ч ",
"��ת:��Ч ",
"��ת:��Ч ",
"ɲ��:��Ч ",
"ACC :��Ч ",
"��ɲ:��Ч ",
"����:��Ч ",
"�ٶ�:��Ч ",
};

/*
********************************************************************************
*                          GLOBAL VARIABLES
********************************************************************************
*/

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
  * @brief  ��ʼ��
  * @param  None
  * @retval NOne
  */
void JTOTher_Init(void)
{
    static u8 InitFlg;
    
    if(InitFlg)
    {
        return;
    }
    InitFlg = 1;    
    
}
/**
  * @brief  IO���,ÿ�������һ��
  * @param  None
  * @retval NOne
  */
void JTOTher_IoSense(void)
{
    u8 iocnt=0;
    u8 spdtype=0;

    if(SpeedMonitor_GetCurSpeedType() == 0)
    {
        spdtype = 1;
    }

    if(++JT_IoTm < 2)
    {
        return;
    }

    JT_IoScn[iocnt++] = Io_ReadExtCarStatusBit(CAR_STATUS_BIT_NEAR_LIGHT);//����
    JT_IoScn[iocnt++] = Io_ReadExtCarStatusBit(CAR_STATUS_BIT_FAR_LIGHT);//Զ��
    JT_IoScn[iocnt++] = Io_ReadExtCarStatusBit(CAR_STATUS_BIT_RIGHT_LIGHT);//��ת
    JT_IoScn[iocnt++] = Io_ReadExtCarStatusBit(CAR_STATUS_BIT_LEFT_LIGHT);//��ת 
    JT_IoScn[iocnt++] = Io_ReadExtCarStatusBit(CAR_STATUS_BIT_BRAKE);//�ƶ� 
    JT_IoScn[iocnt++] = Io_ReadStatusBit(STATUS_BIT_ACC);//ACC
    JT_IoScn[iocnt++] = 0;//��ɲ    
    JT_IoScn[iocnt++] = Io_ReadStatusBit(STATUS_BIT_DOOR1);//����
    JT_IoScn[iocnt++] = spdtype;//�ٶ�

    for(iocnt=0;iocnt<sizeof(JT_IoSta);iocnt++)
    {
        if(JT_IoSta[iocnt] == 0)
        {
            JT_IoSta[iocnt] = JT_IoScn[iocnt];
        }
    }
}

/**
  * @brief  ����IO״̬,����9�ֽڵ�״̬
  *         0:���� 1:Զ�� 2:��ת 3:��ת 4:ɲ�� 5:ACC 6:��ɲ 7:���� 8:�ٶ�
  * @param  None
  * @retval NOne
  */
void JTOTher_IoSta(u8 *pData)
{
    memcpy(pData,JT_IoSta,sizeof(JT_IoSta));
}
/**
  * @brief  ����IO״̬,������ʽ����,��90���ֽ�
  * @param  None
  * @retval ���س���
  */
u8 JTOTher_IoStaHz(u8 *pData)
{
    u8 i;
    u8 ionum = sizeof(JT_IoSta);
    u8 hzlen = sizeof(JT_IoStaHz[0]);
    
    for(i=0;i<ionum;i++)
    {
        memcpy(pData+i*hzlen,(u8*)&JT_IoStaHz[i][0],hzlen);
        if(JT_IoSta[i])
        {
            memcpy(pData+i*hzlen+5,"��Ч",4);
        }
    }

    return 90;
}

/**
  * @brief  λ����Ϣ�еı�׼״̬,��8λ��ͨר��
  * @param  None
  * @retval ����ˢ�º��״̬
  */
u32 JTOTher_IoReport(u32 sta)
{
    u32 jtio=0;
    u8  spdtype=0;

    if(SpeedMonitor_GetCurSpeedType() == 0)
    {
        spdtype = 1;
    }

    jtio = (jtio<<1) | Io_ReadExtCarStatusBit(CAR_STATUS_BIT_BRAKE); 
    jtio = (jtio<<1) | Io_ReadExtCarStatusBit(CAR_STATUS_BIT_LEFT_LIGHT); 
    jtio = (jtio<<1) | Io_ReadExtCarStatusBit(CAR_STATUS_BIT_RIGHT_LIGHT);  
    jtio = (jtio<<1) | Io_ReadExtCarStatusBit(CAR_STATUS_BIT_FAR_LIGHT);
    jtio = (jtio<<1) | Io_ReadExtCarStatusBit(CAR_STATUS_BIT_NEAR_LIGHT); 
    jtio = (jtio<<1) | spdtype;//���忪�� 
    jtio = (jtio<<1) | 0;//����  
    jtio = (jtio<<1) | 0;//����
    jtio = (jtio<<1) | 0;//��ɲ

    sta |= (jtio<<23);

    return sta;
}
/**
  * @brief  ��ͨ��������,ÿ�����һ��
  * @param  None
  * @retval ��������״̬
  */
FunctionalState JTOTher_TimeTask(void)
{
    JTOTher_Init();

    JTOTher_IoSense();//IO���

    return ENABLE;
}

/*
********************************************************************************
*                 DEFIEN END
********************************************************************************
*/ 

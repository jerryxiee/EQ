/*******************************************************************************
 * File Name:			ICCardBase.h 
 * Function Describe:	
 * Relate Module:		IC��ģ��
 * Writer:				Joneming
 * Date:				2014-07-09
 * ReWriter:			
 * Date:				
 *******************************************************************************/
#ifndef __ICCARD_BASE_H_
#define __ICCARD_BASE_H_


//*************�ļ�����***************
#include "stm32f2xx.h"

/***************   �� ������   ********************************/
#define ICCARD_EEYE         1 //Eye����������IC��
#define ICCARD_JTD          2 //��ͨ��ָ����IC��  �۵ۻ��Ϊ
///////////////////////////////////////
//IC��������������;����
#define ICCARD_USAGE_DRIVER_19056_2012    0x0A
#define ICCARD_USAGE_DRIVER_808_2011      0x00
#define ICCARD_USAGE_EWAYBILL             0x01
#define ICCARD_USAGE_MANAGE               0x02
#define ICCARD_USAGE_UNDEFINE             0xFF

//�б�808��������
#define EXDEVICE_TYPE_ICCARD                0x0B //��·����֤IC��������

/********************************************/
#define MAX_SIZE_BUFFER_IC_CARD  149
//-----------------------------------------------------
/***************   �ṹ�� ������   ********************************/

//++++++++  �ṹ��: �б�808��׼����  ͨ�� ++++++++++++++
#pragma pack(1)
typedef struct tagSTRUCT_PROTOCOL_EXDEVICE {
    /****  Э��ͷ ***/
    //u8 Head;
    /****  У���� �ӳ��̱�ŵ��û����������ۼӵ��ۼӺͣ�ȡ�ͣ�λ ***/
    u8 Verity;
    /****  �汾�� ***/
    u8 PID[2];
    /****  ���̱�� ***/
    u8 VID[2];
    /****  �������ͱ�� ***/
    u8 Type;
    /****  ��������***/
    u8 Cmd;
    /****  �û����� ***/
    u8 Data[ MAX_SIZE_BUFFER_IC_CARD];

}ST_PROTOCOL_EXDEVICE;

#pragma pack()
////////////////ICCARD_CS////////////////////////////
#define RCC_ICCARD_CS           RCC_AHB1Periph_GPIOD           /* GPIO��D��˿�ʱ�� */
#define GPIO_ICCARD_CS          GPIOD                          /* ѡ��D��˿�*/ 
#define PIN_ICCARD_CS           GPIO_Pin_5                     /* ѡ���˿�7*/
#define ICCARD_CS_READ()        GPIO_ReadInputDataBit( GPIO_ICCARD_CS , PIN_ICCARD_CS )	/* ��IO����״̬ */

///// IC������� IC-P  PC.9 //////
//-------------------------------------------------------------------------------------------
#define RCC_IC_CARD_POWER       RCC_AHB1Periph_GPIOC  /* GPIO�˿�ʱ�� */
#define GPIO_IC_CARD_POWER      GPIOC                 /* GPIO�˿� */
#define PIN_IC_CARD_POWER       GPIO_Pin_9            /* GPIO���� */
//-------------------------------------------------------------------------------------------
#define IC_CARD_POWER_ON()      GPIO_SetBits(  GPIO_IC_CARD_POWER, PIN_IC_CARD_POWER ) 
#define IC_CARD_POWER_OFF()     GPIO_ResetBits(GPIO_IC_CARD_POWER, PIN_IC_CARD_POWER )


/***************   �ⲿȫ�ֺ���������   ********************************/
/********************************************************************
* ���� : ICCardBase_VerifyCalcSum
* ���� : ���� ����� У��(�ۼӺ�)
********************************************************************/
u8 ICCardBase_VerifyCalcSum( u8 * buff, u16 length);
/********************************************************************
* ���� : ICCardBase_VerifyCalcSum
* ���� : ���� ����� У��(���)
********************************************************************/
u16 ICCardBase_ExtractExdevice( u8 *pInBuf, u16 InLength, ST_PROTOCOL_EXDEVICE *pPacket    );
/*************************************************************
** ��������: ICCardBase_CreatPackData
** ��������: ���ݴ��
*************************************************************/
u16 ICCardBase_CreatPackData( u8 cmd,  u8 * pInBuf , u16 InLength , u8 * pOutBuf ,unsigned char response);
//-------------------------------------------------------------------------------------------
/*************************************************************
** ��������: ICCardBase_ParameterInitialize
** ��������: IC��������ʼ��
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void ICCardBase_ParameterInitialize(void);
/*************************************************************
** ��������: ICCardBase_GetReadFlag
** ��������: �ն��Ƿ����ڶ�IC��
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 1:���ڶ�,0����
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
unsigned char ICCardBase_GetReadFlag(void);
/*************************************************************
** ��������: ICCardBase_GetUseIntFlag
** ��������: �Ƿ�ͨ������CPUͨ��I2C��IC��
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 1:ͨ������CPUͨ��I2C��IC��,0:ʹ���ⲿ������IC��ģ��
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
unsigned char ICCardBase_GetUseIntFlag(void);
/*************************************************************
** ��������: ICCardBase_TimerTask
** ��������: ��ʱ��������
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void ICCardBase_TimerTask(void);
#endif
/******************************************************************************
**                            End Of File
******************************************************************************/
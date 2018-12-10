/*
********************************************************************************
*
*
* Filename      : vdr.h
* Version       : V1.00
* Programmer(s) : miaoyahan
* Time          : 20140722
********************************************************************************
*/

/*
********************************************************************************
*                               MODULE
*
* Note(s) : 
********************************************************************************
*/

#ifndef _VDR_H
#define _VDR_H


/*
********************************************************************************
*                               INCLUDES
********************************************************************************
*/
#include "include.h"

/*
********************************************************************************
*                            DEFINES VALUE
********************************************************************************
*/
#define   PRO_DEF_RETAINS        (0x12) //��¼�Ǳ����ֽ�
#define   VDR_CCC_CODE           "C000116"
#define   VDR_PRO_MODEL          "EGS701          "

#define   VDR_MEM_READ           0
#define   VDR_MEM_WRITE          1

#define	  VDR_SECTOR_SIZE		 4096//������С
#define	  VDR_PACKAGE_SIZE       1000//�ְ���С
#define	  VDR_STEP_SIZE          252//��󵥲�����
#define   VDR_WIRE_ADD_SIZE      300//ע������Ϊ16BIT
/*
********************************************************************************
*                               DEFINES
********************************************************************************
*/


/*
********************************************************************************
*                                 ENUM
********************************************************************************
*/
typedef enum {
    VDR_COM=0, //����ͨ��
    VDR_WIRE_1,//����ͨ��1
    VDR_WIRE_2,//����ͨ��2
}VDR_CH;//VDR��������

/**
  * @brief  ��¼�Ǵ�������
  */
typedef enum
{
  VDR_ERR_NO=0,//��ȷ���� 
  VDR_ERR_VF,  //У�����
  VDR_ERR_CMD, //�Ƿ�����
  VDR_ERR_HEAD,//�Ǽ�¼������
  
} VDR_ERR;

/**
  * @brief  ��¼�Ǵ洢����
  */
typedef enum
{
  VDR_MEM_DVRINFOR  = (0x00),
  VDR_MEM_TIMEINIT  ,    
  VDR_MEM_MILEINIT  ,  
  VDR_MEM_MILEADD   ,
  VDR_MEM_PULSE     , 
  VDR_MEM_CARCODE   ,
  VDR_MEM_CARPLATE  ,
  VDR_MEM_CARTYPE   ,
  VDR_MEM_STASIGNAL ,
  VDR_MEM_ONLY      ,
  VDR_MEM_RTC       ,  
  
} VDR_MEM;

/*
********************************************************************************
*                                STRUCT
********************************************************************************
*/
typedef struct 		
{
    u8  cmd;   //������
    u16 len;   //����
    u8  back;  //������
    u8  vef;   //У��
    u8  *pbuf; //����

}VDR_HEAD;//��¼������ͷ

typedef struct 		
{
    u32    cnt_start;    //32λ��ʼʱ��
    u32    cnt_end;      //32λ����ʱ��
    u8     bcd_start[6]; //bcd�뿪ʼʱ��
    u8     bcd_end[6];   //bcd�����ʱ��
    TIME_T t_start;      //ʱ��ṹ�忪ʼʱ��
    TIME_T t_end;        //ʱ��ṹ�����ʱ��
    u16    maxblock;     //������ݿ���
    
}VDR_SERT;//��¼��ʱ������ṹ��

typedef struct 		
{
    u8  cmdnum;  //�����ֱ��
    u32 timestar;//��ʼʱ��
    u32 timeend; //����ʱ��
    
}VDR_PACKINF;//�ְ���Ϣ�ṹ��

typedef struct 		
{
    u32 addrstar;//��ʼ��ַ
    u32 addrend; //������ַ
    u16 stepcnt; //����������
    u16 all;     //�ܰ���
    u16 cnt;     //��ǰ����
    u8  cmd;     //������
    u8  cyc;     //ѭ����־
    u16 len;     //����
    u8  *pbuf;   //��ַ

}VDR_PACKSER;//�ְ������ṹ��

typedef struct 		
{
    u8 state;  //���߲ɼ�״̬��
    u8 busy;   //���߲ɼ�æ��־
    VDR_CH ch; //ͨ����
    
    VDR_PACKINF packageinf;//�ְ���Ϣ
    VDR_PACKSER packageser;//�ְ�����
    VDR_PACKSER packageadd;//�ְ�����

}VDR_WIRE;//���߲ɼ��ṹ��

typedef struct 		
{
    u8  rdy;  //������־�ȴ�8003
    u8  flg;  //������־
    u16 ack;  //Ӧ����ˮ��
    u8  all;  //�ش�����
    u8  cnt;  //��ǰ����    
    u16 buf[VDR_WIRE_ADD_SIZE];//�ش��б�

}VDR_WIREADD;//���߲ɼ������ṹ��

typedef  void (*VDR_FUC_SET)(u8 *pSrc);           //���� 
typedef  void (*VDR_FUC_TEST)(void);              //�춨
typedef  void (*VDR_FUC_BASE)(VDR_HEAD *pHead);   //����
typedef  u8   (*VDR_FUC_MEM)(u8 type, u8 *pSrc);  //�洢���� �������úͶ�ȡ

/*
********************************************************************************
*                          EXTERN FUNCTIONS
********************************************************************************
*/
extern const VDR_FUC_MEM  Vdr_Fuc_Mem[];
extern VDR_WIRE  Vdr_Wire;

extern const u16 VdrSecAddr[];
extern const u16 VdrStepLen[];
extern const u16 VdrUpLen[];
extern const u8  Vdr_Cmd_Base[];
extern const u8  Vdr_Cmd_Ser[];
extern const u8  Vdr_Cmd_Set[];
extern const u8  Vdr_Cmd_Test[];
extern const u8  Vdr_Cmd_All[];
extern u8  Vdr_BufData[];
extern u8  Vdr_BufSend[];

extern u16 Vdr_WireAck;

/*
********************************************************************************
*                               FUNCTIONS
********************************************************************************
*/
u8   Vdr_Wire_Handle(void);
void Vdr_Usb_Handle(void);
void Vdr_Data_Rec(VDR_CH ch, u8 type, u8 *pSrc, u16 SrcLen);
u8   Vdr_Wire_AddEven(VDR_CH ch, u8 *pSrc);

u8   Vdr_Cmd_To(u8 cmd);
u16  Vdr_Head_ToBuf(u8 type, VDR_HEAD head, u8 *pDst);
void Vdr_Time_Get(const u8 *pSrc, VDR_SERT *pTime);


void Vdr_Set_CarInfor(u8 *pSrc);
void Vdr_Set_TimeInit(u8 *pSrc);
void Vdr_Set_StaSignal(u8 *pSrc);
void Vdr_Set_Rtc(u8 *pSrc);
void Vdr_Set_Pulse(u8 *pSrc);
void Vdr_Set_MileInit(u8 *pSrc);
void Vdr_Set_Pro(u8 cmdnum, u8 *pSrc, VDR_HEAD *pHead);

void Vdr_Get_GbtVer(VDR_HEAD *pHead);
void Vdr_Get_DvrInfor(VDR_HEAD *pHead);
void Vdr_Get_Rtc(VDR_HEAD *pHead);
void Vdr_Get_Mileage(VDR_HEAD *pHead);
void Vdr_Get_Pulse(VDR_HEAD *pHead);
void Vdr_Get_CarInfor(VDR_HEAD *pHead);
void Vdr_Get_StaSignal(VDR_HEAD *pHead);
void Vdr_Get_OnlyNum(VDR_HEAD *pHead);
void Vdr_Get_Base(u8 cmdnum,VDR_HEAD *pHead);
void Vdr_Get_Ser(u8 cmdnum,VDR_SERT time, VDR_HEAD *pHead);
void Vdr_Get_PackageInf(VDR_PACKINF PackInf, VDR_PACKSER *pPackage);
void Vdr_Get_PackageSer(VDR_PACKSER *pPackage);


u8   Vdr_Mem_DvrInfor(u8 type, u8 *pSrc);
u8   Vdr_Mem_TimeInit(u8 type, u8 *pSrc);
u8   Vdr_Mem_MileInit(u8 type, u8 *pSrc);
u8   Vdr_Mem_MileAdd(u8 type, u8 *pSrc);
u8   Vdr_Mem_Pulse(u8 type, u8 *pSrc);
u8   Vdr_Mem_Carcode(u8 type, u8 *pSrc);
u8   Vdr_Mem_Carplate(u8 type, u8 *pSrc);
u8   Vdr_Mem_CarType(u8 type, u8 *pSrc);
u8   Vdr_Mem_StaSignal(u8 type, u8 *pSrc);
u8   Vdr_Mem_Only(u8 type, u8 *pSrc);
u8   Vdr_Mem_Rtc(u8 type, u8 *pSrc);

void Vdr_Test_Mode(void);
void Vdr_Test_Mile(void);
void Vdr_Test_Pulse(void);
void Vdr_Test_Rtc(void);
void Vdr_Test_Break(void);
void Vdr_Test_Pro(u8 cmdnum,VDR_HEAD *pHead);

void Vdr_Eye_Value(VDR_HEAD *pHeadRx,VDR_HEAD *pHeadTx);

FunctionalState RecorderWireless_TimeTask(void);
FunctionalState RecorderWired_TimeTask(void);
u8   Recorder_GetCarInformation(u8 *pBuffer);
u8   Recorder_Stop15MinuteSpeed(u8 *p);
void Recorder_USBHandle(void);
void Recorder_MileageTest(void);
/*
********************************************************************************
*                               MODULE END
********************************************************************************
*/

#endif





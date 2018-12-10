/**
  ******************************************************************************
  * @file    Record_Task.h
  * @author  
  * @version 
  * @date    
  * @brief  ¼��������Ƚӿ�
  *               
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/

#ifndef __RECORD_TASK_H
#define __RECORD_TASK_H

/* Includes ------------------------------------------------------------------*/
#include "taskschedule.h"
#include "I2S_Record.h"

#ifdef __cplusplus
 extern "C" {
#endif 

//¼����������
typedef struct
{
  u8 StartFlag;            //����¼����־,0-ֹͣ¼����1-����¼��
  u8 LoopFlag;             //��ʱδ���ã�ѭ��¼����־,0-δ����ѭ��¼��,1-����ѭ��¼��
  u32 RecordTime;          //�������ѭ��¼�������øñ�������¼��ʱ��
  u32 RecordTimeSpace;     //��¼�����,����ü����RecordTime�Ա�,���������ϵ磬Ҳ֪��ʲôʱ��ر�����
  u32 RecordID;            //��ý��ID,������������
  u32 RecordTimeMaxLen;    //¼����󳤶�  
  u16 resev;               //����
}RecordDef;
   
#define RECORD_TASK_TIME    1                               //¼���������ʱ��,50ms����һ��   
#define RECORD_TIME_TASK_TIME    5*SYSTICK_0p1SECOND        //��ʱ¼���ر�����,  500ms
   
//�������� 
FunctionalState RecordTask(void);

FunctionalState Record_TimeTask(void);

//��ʼ��¼������
void RecordTask_Init(void);

//��������
void RecordTask_Start(u32 AudioFreq,u16 maxTime);

//�ر�����
void RecordTask_Close(void);

//�ر�����-���Ⱥ�������
FunctionalState Auto_RecordTask_Close(void);

//�ָ�Ϊ5����һ���ļ�
void RecordTask_Split(void);

//��ȡ��Ƶʱ��
u8  Audio_GetFileName(u8 *pBuffer);

//��ȡ¼��״̬
u8 GetRecordFlag(void);

//��ȡ��¼�����
u32  GetRecordTime(void);

//д����
void  RrcordWriteDef(void);

//��ȡ����
void  RrcordReadDef(void);

//������ʱ����
void  StartRecordTime(u32 RecordTime);

#ifdef __cplusplus
}
#endif   

#endif /*  */
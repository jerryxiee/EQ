/********************************************************************
//��Ȩ˵��	:
//�ļ�����	:Usart1.c		
//����		:USART1��������,����Debug�����
//�汾��	:
//������	:yjb
//����ʱ��	:2012.6
//�޸���	:
//�޸�ʱ��	:
//�޸ļ�Ҫ˵��	:
//��ע		:
//
***********************************************************************/
#ifndef _USART1_H
#define _USART1_H

#ifdef __cplusplus
 extern "C" {
#endif 

//****************�궨��********************

//****************�ⲿ����*******************   

//****************ȫ�ֱ���*******************

//****************��������*******************     
void COM1_Init(u32 baud_rate);
void COM1_putc(int ch);
void COM1_putstr(char* str);
u8  COM1_WriteBuff(u8 *buff, u16 len);
u16 COM1_ReadBuff(u8 *buff,u16 len);
void COM1_RxIsr(void);
void COM1_TxIsr(void);

#ifdef __cplusplus
}
#endif   

#endif /* _USART1_H */
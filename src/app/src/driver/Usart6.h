/********************************************************************
//版权说明	:
//文件名称	:Usart6.c		
//功能		:USART6串口配置,用于IC读写
//版本号	:
//开发人	:yjb
//开发时间	:2012.6
//修改者	:
//修改时间	:
//修改简要说明	:
//备注		:
//
***********************************************************************/
#ifndef _USART6_H
#define _USART6_H

#ifdef __cplusplus
 extern "C" {
#endif 

//****************宏定义********************
#define COM6_TX_MAX_LEN   512                                          //接收缓冲区长度
#define COM6_RX_MAX_LEN   512                                          //接收缓冲区长度

   
//****************外部变量*******************   

//****************全局变量*******************

//****************函数声明*******************     
void COM6_Init(u32 baud_rate);
void COM6_putc(int ch);
void COM6_putstr(char* str);
u8  COM6_WriteBuff(u8 *buff, u16 len);
u16 COM6_ReadBuff(u8 *buff,u16 len);
void COM6_RxIsr(void);
void COM6_TxIsr(void);

#ifdef __cplusplus
}
#endif   

#endif /* _USART6_H */
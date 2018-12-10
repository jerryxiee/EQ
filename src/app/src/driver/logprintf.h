/********************************************************************
//版权说明	:
//文件名称	:logprintf.h	
//功能		:日志打印函数
//版本号	:
//开发人	:yjb
//开发时间	:2012.6
//修改者	:
//修改时间	:
//修改简要说明	:
//备注		:
***********************************************************************/

#ifndef __LOGPRINTF_H
#define __LOGPRINTF_H

#ifdef __cplusplus
 extern "C" {
#endif 
#include    <stm32f2xx.h>
   
//****************宏定义********************

   
#if(LOG_TYPE == 0)   
#define LOG_PR(fmt, args ...)
#define LOG_PR_N(fmt)   
#define LOG_DB(desc,info,ilen) 
#define LOG_DB2(desc,dlen,info,ilen)  
#define LOG_B(info,ilen)   
#endif

#if(LOG_TYPE == 1)   
#define LOG_PR(fmt,...) printu(fmt, __VA_ARGS__)                                     //可带参数输出
#define LOG_PR_N(fmt)   printu(fmt)                                                  //不可带参数输出
#define LOG_DB(desc,info,ilen) printuHex((u8*)desc,strlen((char*)desc),info,ilen)    //描述信息为字符串,将数据转换为16进制字符写日志
#define LOG_DB2(desc,dlen,info,ilen) printuHex((u8*)desc,dlen,info,ilen)             //描述信息为一文字数据块,将数据转换为16进制字符写日志
#define LOG_B(info,ilen) printuHex((u8*)((void*)0),0,info,ilen)                      //将数据转换为16进制字符写日志
#endif
   
//****************结构定义*******************

//****************外部变量*******************

//****************函数声明*******************

//****************函数定义******************
void LOG_SetUsart(COM_TypeDef COM);
void printu(char* ctrl, ...);
void printuHex(u8 *desc, u16 dlen,u8* buf,u16 len);

#ifdef __cplusplus
}
#endif   

#endif /* __LOGPRINTF_H */
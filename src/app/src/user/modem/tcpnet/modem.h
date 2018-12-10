/*
********************************************************************************
*
*
* Filename      : modem.h
* Version       : V1.00
* Programmer(s) : miaoyahan 
* Time          : 20140710
********************************************************************************
*/
#ifndef  MODEM_H_
#define  MODEM_H_

/*
********************************************************************************
*                            DEFINES VALUE
********************************************************************************
*/
#define MODEM_TSEC           (20)  //秒延时配置
#define MODEM_TMIN           (1200)//分延时配置
#define MODEM_ATERR          (0XAA)//modem返回指令错误       
#define MODEM_VER            ("->  Ver:1.0     <-\r\n")//版本

/*
********************************************************************************
*                            DEFINES CONFIG
********************************************************************************
*/
////////////////////////////////////////////////////////////////串口相关内存说明
/*
  模块立即执行的数据,为直接跟
  模块交互的缓存;缓存大小主要
  取决于"MODEM_SOC_TX_SIZE"大
  小,但一定要稍微大些,预留AT
  命令所需的空间(AT命令一般需
  要50个字节即可,
  例如:MODEM_SOC_TX_SIZE = 1024
  则MODEM_UART_TX_SIZE = 1074).
*/
#define MODEM_UART_TX_SIZE   (1074)
/*
  在"MODEM_UART_TX_SIZE"缓存存储
  的数据条数即需要模块处理的AT命
  令条数(当调用Modem_Uart_Send
  函数时条数加1).
*/
#define MODEM_UART_NDTX_SIZE (10)
/*
  模块立即返回的数据,为直接跟
  模块交互的缓存,缓存大小主要
  取决于模块一次性能吐多少数据
  ,一般情况下按照FTP升级来算,
  跟模块特性也有关系(比如中兴
  模块在FTP升级时一次性能吐
  1024个字节,如果一个调度没能
  来的急处理完这1024则会被滞留
  在该缓存,为了保险起见故开辟
  2048).
*/
#define MODEM_UART_RX_SIZE   (2048)
/*
  在"MODEM_UART_RX_SIZE"缓存存储
  的数据条数即需要模块处理的AT命
  令条数(当串口收到\r\n时条数加1).
*/
#define MODEM_UART_NDRX_SIZE (50)
/*
  当侦测到模块有数据返回时,会从
  "MODEM_UART_RX_SIZE"内存出取出
  数据并拷贝到该缓存中,在该缓存
  中进行AT命令的遍历,并执行动作,
  执行完毕后会把内存销毁;
*/
#define MODEM_AT_SIZE        (1500)

/////////////////////////////////////////////////////////////////SOC连接相关内存
/*
  全部连接占用的内存为: MEM_SIZE     = ONE_SOC_SIZE * MODEM_SOC_NUM;
  
  一个连接占用的内存为: ONE_SOC_SIZE = (MODEM_SOC_TX_SIZE * MODEM_SOC_ND_SIZE 
                                      *sizeof(MODLIB_BUF) * sizeof(MODLIB_NOD));

  MODEM_SOC_NUM为: 系统支持的连接个数.
*/
#define MODEM_SOC_NUM        (3)
/*
  单个连接发送的最大数据长度,
  注意!!!必须 < MODEM_UART_TX_SIZE
*/
#define MODEM_SOC_TX_SIZE    (1024)
/*
  在"MODEM_SOC_TX_SIZE"内存中
  暂存的数据条数(当调用
  Modem_Api_Socket_Send函数时
  条数加1).
*/
#define MODEM_SOC_ND_SIZE    (200)

/////////////////////////////////////////////////////////////////////SMS相关内存
/*
  一次最多发送短信数据的长度,
  PDU格式时还有协议头
*/
#define MODEM_SMS_SENDMAX    (400)

/////////////////////////////////////////////////////////////////////TTS相关内存
/*
  存放TTS播报的数据.
*/
#define MODTTS_SIZE          (250)//TTS缓存


///////////////////////////////////////////////////////////////////DEBUG相关内存
/*
  modem调试总开关,打开后将分
  配响应调试资源.
*/
#define MODEM_DEBUG

/*
  Modem_Printf函数打印时所需
  的内存(注意!!!该内存使用的
  是栈).
*/
#define MODDB_PRTF_SIZE      (1024)//printf栈内存

/*
********************************************************************************
*                               INCLUDES
********************************************************************************
*/
#include "modem_lib.h"
#include "modem_bsp.h"
#include "modem_uart.h"
#include "modem_core.h"
#include "modem_port.h"
#include "modem_state.h"
#include "modem_api.h"
#ifdef MODEM_DEBUG
#include "modem_debug.h"
#endif
#ifdef EYE_MODEM
#include "modem_app_com.h"
#include "modem_app_first.h"
#include "modem_app_other.h"
#ifdef   EYE_EGS701      
#include "modem_app_second.h"//只有701才打开第二联接
#include "IcCard_CertificateService.h"
#endif
#endif


/*
********************************************************************************
*                               MODULE END
********************************************************************************
*/

#endif






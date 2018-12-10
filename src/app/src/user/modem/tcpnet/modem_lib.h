/**
  ******************************************************************************
  * @file    modem_lib.h 
  * @author  miaoyahan
  * @version V1.0.0
  * @date    2014-10-13
  * @brief   
  ******************************************************************************
  */

/*
********************************************************************************
*                               MODULE
*
* Note(s) : 一般用到的常用算法
********************************************************************************
*/
#ifndef  MODEM_LIB
#define  MODEM_LIB

/*
********************************************************************************
*                               INCLUDES
********************************************************************************
*/
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
/*
********************************************************************************
*                            DEFINES VALUE
********************************************************************************
*/


/*
********************************************************************************
*                               DEFINES
********************************************************************************
*/
// 用户信息编码方式
#define GSM_7BIT		0
#define GSM_ASCII		1
#define GSM_8BIT		4
#define GSM_UCS2		8
/*
********************************************************************************
*                                 ENUM
********************************************************************************
*/
typedef enum 
{
    INS_ALL=0,//插入整包数据
    INS_HALF, //插入半包数据
    
}MODNOD_TYPE;//错误类型


/*
********************************************************************************
*                                STRUCT
********************************************************************************
*/

//////////////////////////////////////////////////////////////////////缓存结构体
typedef struct  
{
    u16 len;   //数据长度
}MODLIB_BUF;//缓存结构体

//////////////////////////////////////////////////////////////////////AT命令节点
typedef struct  
{ 
    u8  max;    //最大节点数
    u16 num;    //节点数 
    u8  head;   //数据头    
    u8  tail;   //数据尾
    u8  lock;   //加锁   
    u16  pLen;  //数据长度   
    u16  pSize; //数据缓存大小  
    u8  *pBase; //数据缓存基地址
    u8  *pHead; //数据缓存头   
    u8  *pTail; //数据缓存尾      
    MODLIB_BUF *pNode;//节点缓存地址
}MODLIB_NOD;//AT命令节点




typedef struct
{
  uint32_t head;
  uint32_t tail;
  uint32_t size;
  uint32_t len;
  uint8_t * baseAddr;
  union
  {
    uint8_t lock;
    uint8_t set:1;
  }spin;
}fifo_TypeDef;//队列结构体


// 短消息参数结构，编码/解码共用
// 其中，字符串以'\0'结尾
typedef struct {
    u8 SCA[16];            // 短消息服务中心号码(SMSC地址)
    u8 TPA[16];            // 目标号码或回复号码(TP-DA或TP-RA)
    u8 TP_PID;             // 用户信息协议标识(TP-PID)
    u8 TP_DCS;             // 用户信息编码方式(TP-DCS)
    u8 TP_FO;			   //
    u8 TP_SCTS[16];        // 服务时间戳字符串(TP_SCTS), 接收时用到
    u8 TP_UD[160];         // 原始用户信息(编码前或解码后的TP-UD)
    u8 PDU_TP;             // PDU编码类型
    u8 PDU_LN;             // PDU编码长度
    u8 udMultiFlg:1;	   // 拆分标专
    u8 udMultiCnt:7;	   // 分包数
    u8 udMultiIndex;	   // 发送序号
    u8 udLen;			   // TP_UD长度
    short index;		   // 短消息序号，在读取时用到
}SM_PARAM;


/*
********************************************************************************
*                          DEFINES FUNCTIONS
********************************************************************************
*/


/*
********************************************************************************
*                               FUNCTIONS
********************************************************************************
*/

void ModLib_NodInit(MODLIB_NOD *node,MODLIB_BUF *BufNode, u8 NodeMax,
                                                 u8 *BufData,u16 DataSize);
u8   ModLib_NodIns(MODLIB_NOD *node, MODNOD_TYPE type, u8 *data, u16 len);
u16  ModLib_NodRet(MODLIB_NOD *node, u8 *data);
u16  ModLib_NodRetEx(MODLIB_NOD *node, u8 *data, u16 len);
u8   ModLib_NodNum(MODLIB_NOD *node);
u16  ModLib_NodLen(MODLIB_NOD *node);
void ModLib_NodRst(MODLIB_NOD *node);

int ModLib_ByteStr(u8* pDst, const u8* pSrc, int nSrcLength);
int ModLib_ByteStrEx(u8* pDst, const u8* pSrc, int nSrcLength);
int ModLib_StrByte(const u8* pSrc, u8* pDst, int nSrcLength);
int ModLib_ByteExc(const u8* pSrc, u8* pDst, int nSrcLength);
int ModLib_PduEncode( SM_PARAM* pSrc, u8* pDst);
int ModLib_PduDecode(const u8* pSrc, SM_PARAM*   pDst);
int ModLib_7BitEncode(const char* pSrc, unsigned char* pDst, int nSrcLength);
u16 ModLib_7BitDecode(const u8* pSrc, u8* pDst, u16 nSrcLength);


u16  ModLib_AtCmp(u8 *pdst, u8 *psrc);
u16  ModLib_AtCmy(u8 *pdst, u8 *psrc);
u16  ModLib_AtLen(u8 *psrc);
u16  ModLib_AtAddr(u8 *psrc, char cmpchar);
u16  ModLib_AtCmpEx(u8 *pdst, u8 *psrc, u16 dstlen);



u8   ModLib_ChkIp(u8 *pData);

/*
********************************************************************************
*                               MODULE END
********************************************************************************
*/

#endif



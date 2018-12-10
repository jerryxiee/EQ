/**
  ******************************************************************************
  * @file    modem_lib.c 
  * @author  miaoyahan
  * @version V1.0.0
  * @date    2014-10-13
  * @brief   
  ******************************************************************************
  */
/*
********************************************************************************
*                               INCLUDES
********************************************************************************
*/
#ifdef EYE_MODEM
#include "include.h"
#else
#include  <app_cfg.h>
#include  <includes.h>
#endif

/*
********************************************************************************
*                             LOCAL DEFINES
********************************************************************************
*/
#define min(x,y)  ((x < y)? x : y)
#define max(x,y)  ((x > y)? x : y)

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
  * @brief  初始化节点数据结构
  * @param  *node     : 节点数据结构
  * @param  *BufNode  : 指向节点缓存首地址
  * @param   NodeMax  : 节点缓存大小
  * @param  *BufData  : 指向节点存储缓存首地址
  * @param   DataSize : 节点数据缓存大小
  * @retval Node
  */
void ModLib_NodInit(MODLIB_NOD *node,MODLIB_BUF *BufNode, u8 NodeMax,
                                                    u8 *BufData,u16 DataSize)
{
    memset((MODLIB_NOD*)node,0,sizeof(MODLIB_NOD));//销毁结构体
    memset((MODLIB_BUF*)BufNode,0,NodeMax*sizeof(MODLIB_BUF));
    node->pNode = BufNode;//节点缓存地址
    node->max   = NodeMax;//最大节点个数   
    node->pBase = BufData;//数据缓存地址    
    node->pSize = DataSize;//数据缓存大小
    node->pHead = node->pTail = BufData;
}
/**
  * @brief  指定长度数据插入节点结构
  * @param  *node : 节点数据结构
  * @param   type : INS_ALL->节点打包  INS_HALF->仅插入数据不打包
  * @param  *data : 指向插入数据的首地址
  * @param   len  : 插入数据的大小
  * @retval 成功返回->1  失败返回->0
  */
u8  ModLib_NodIns(MODLIB_NOD *node, MODNOD_TYPE type, u8 *data, u16 len)
{
    MODLIB_BUF *pNode;
    u32 addrmax;
    u32 addrdata;
    
    if((node == NULL)||(node->lock))
    {
        return 0;//非法指针或加锁
    }
    if((node->num+1) >= node->max)
    {
        return 0;//节点缓存已满
    }
    if(len == 0)
    {
        return 0;//无效数据长度
    }
    pNode = node->pNode + node->tail;//节点地址

    if((node->pLen+len) > node->pSize)
    {
        if(pNode->len)//强制封包
        {
            node->tail += 1;//节点尾加1
            node->tail %= node->max;
            node->num  += 1;//节点个数加1
        }
        return 0;//数据缓存已满        
    }
    pNode->len += len;//数据长度
    node->pLen += len;//累计长度
    
    addrmax  = (u32)node->pBase + node->pSize;
    addrdata = (u32)node->pTail+ len;
    if((addrdata/addrmax)&&(addrdata%addrmax))
    {
        len -= (addrdata%addrmax);
        memcpy(node->pTail,data,len);
        data += len;
        len = addrdata%addrmax;
        node->pTail = node->pBase;
    }
    memcpy(node->pTail,data,len);//复制数据
    node->pTail+= len;//移动指针
    
    if(type==0)//封包
    {
        node->tail += 1;//节点尾加1
        node->tail %= node->max;
        node->num  += 1;//节点个数加1
    }
    return 1;
}
/**
  * @brief  从节点数据中取出当前节点内的全部数据
  * @param  *node : 节点数据结构
  * @param  *data : 指向节点内的数据
  * @retval 返回数据长度
  */
u16 ModLib_NodRet(MODLIB_NOD *node, u8 *data)
{
    MODLIB_BUF *pNode;
    u32 addrmax;
    u32 addrdata;
    u16 len;

    if((node == NULL)||(data == NULL)||(node->lock))
    {
        return 0;//非法指针或加锁
    }
    if(node->num == 0)
    {
        return 0;//无节点
    }

    pNode = node->pNode + node->head;//节点地址
    len   = pNode->len;

    addrmax  = (u32)node->pBase + node->pSize;
    addrdata = (u32)node->pHead + len;
    if((addrdata/addrmax)&&(addrdata%addrmax))
    {
        len -= (addrdata%addrmax);
        memcpy(data,node->pHead,len);
        memset(node->pHead,0,len);   
        data += len;
        len = addrdata%addrmax;
        node->pHead = node->pBase;
    }
    memcpy(data,node->pHead,len);//复制数据
    memset(node->pHead,0,len);   //清空数据
    node->pHead += len;          //移动指针

    node->head += 1;         //节点头加1
    node->head %= node->max;

    node->num  -= 1;         //节点个数减1
    node->pLen = max(node->pLen, pNode->len);
    node->pLen -= pNode->len;//累计长度

    len = pNode->len;//暂存
    pNode->len = 0;  //销毁长度
    
    return len;

}
/**
  * @brief  从节点中取出指定长度数据
  * @param  *node : 节点数据结构
  * @param  *data : 指向节点内的数据
  * @param  len   : 要取出的数据长度
  * @retval 返回实际取出长度
  */
u16 ModLib_NodRetEx(MODLIB_NOD *node, u8 *data, u16 len)
{
    MODLIB_BUF *pNode;
    u32 addrmax;
    u32 addrdata;

    if((node == NULL)||(data == NULL)||(node->lock))
    {
        return 0;//非法指针或加锁
    }
    if(node->num == 0)
    {
        return 0;//无节点
    }
    if(len == 0)
    {
        return 0;//无效数据长度
    }
    pNode = node->pNode + node->head;//节点地址
    len = min(len,(u16)pNode->len); //提取剩余长度

    addrmax  = (u32)node->pBase + node->pSize;
    addrdata = (u32)node->pHead + len;
    if((addrdata/addrmax)&&(addrdata%addrmax))
    {
        len -= (addrdata%addrmax);
        memcpy(data,node->pHead,len);
        data += len;
        len = addrdata%addrmax;
        node->pHead = node->pBase;
    }
    memcpy(data,node->pHead,len);//复制数据
    node->pHead += len;          //移动指针
    pNode->len  -= len;          //数据长度递减
    node->pLen   = max(node->pLen, len);
    node->pLen  -= len;          //累计长度递减
    
    if(pNode->len == 0)
    {
        node->head  += 1;         //节点头加1
        node->head  %= node->max;
        node->num   -= 1;         //节点个数减1  
    }
    return len;

}


/**
  * @brief  当前节点个数
  * @param  *node : 节点数据结构
  * @retval 返回数据长度
  */
u8 ModLib_NodNum(MODLIB_NOD *node)
{
    return node->num;
}
/**
  * @brief  当前节点中的数据长度
  * @param  *node : 节点数据结构
  * @retval 返回数据长度
  */
u16 ModLib_NodLen(MODLIB_NOD *node)
{
    MODLIB_BUF *pNode;
    
    pNode = node->pNode + node->tail;//节点地址
    
    return pNode->len;
}
/**
  * @brief  复位节点
  * @param  *node : 节点数据结构
  * @retval 返回数据长度
  */
void ModLib_NodRst(MODLIB_NOD *node)
{
    node->lock = 1;//上锁
    memset((MODLIB_BUF*)node->pNode,0,node->max*sizeof(MODLIB_BUF));
    node->num  = 0;//归零节点个数
    node->pLen = 0;//归零数据长度
    node->head = node->tail  = 0;//归零节点头尾
    node->pHead= node->pTail = node->pBase;;//数据地址回归
    node->lock = 0;//释放
}
/**
  * @brief  比较两条命令是否一致
  * @param  *pdst: 指向目标数据地址
  * @param  *psrc: 指向源数据地址
  * @retval 返回对比差值 0表示相等
  */
u16  ModLib_AtCmp(u8 *pdst, u8 *psrc)
{
    u16  len=0;
    
    len = strlen((char*)psrc);

    len = strncmp((char *)pdst,(char *)psrc,len);
    
    return len;
}
/**
  * @brief  从缓存末地址搜索指定命令
  * @param  *pdst: 指向目标数据地址
  * @param  *psrc: 指向源数据地址
  * @param  dstlen: 目标缓存的长度 
  * @param  *prelen: 回传命令首地址到\r\n的长度
  * @retval 返回命令的坐标 0表示失败
  */
u16  ModLib_AtCmpEx(u8 *pdst, u8 *psrc, u16 dstlen)
{
    u16 len;
    u16 i;

    len = strlen((char*)psrc);

    for(i=dstlen; i!=0; i--)
    {
        if(strncmp((char *)(pdst+i),(char *)psrc,len) == 0)
        {
            break;
        }
    }
    return i;
}

/**
  * @brief  把命令放入指定缓存
  * @param  *pdst: 目标缓存
  * @param  *psrc: 源数据
  * @retval 返回复制的长度
  */
u16  ModLib_AtCmy(u8 *pdst, u8 *psrc)
{
    u16  len=0;

    len = strlen((char*)psrc);

    memcpy(pdst,psrc,len);
    *(pdst+len)=0;
    
    return len;
}
/**
  * @brief  计算缓存中命令的长度 以0x0d 或者 0x0a 或者 0x0 结束符
  * @param  *psrc: 源数据
  * @retval 返回命令长度
  */
u16  ModLib_AtLen(u8 *psrc)
{
    u16 i;
    u8  tmp;

    for(i=0;;i++)
    {
        tmp = *psrc++;
        if((tmp == 0x0d)||(tmp == 0x0a)||(tmp == 0x0))
        {
            break;
        }
    }
    return i;
}

/**
  * @brief  在缓存中查找指定字符并返回坐标
  * @param  *psrc: 源数据
  * @param  *pchar:要查找的字符 
  * @retval 返回当前字符在缓存中的坐标
  */
u16  ModLib_AtAddr(u8 *psrc, char cmpchar)
{
    u16 i;
    u16 len;
    
    len = strlen((char*)psrc);

    for(i=0;i<len;i++)
    {
        if(*(psrc+i) == cmpchar)
        {
            break;
        }
    }
    return i;
}
/*
  * @brief  检查IP类型和合法性
  * @param  指向IP地址
  * @retval 0: IP
  *         1: 域名
  *         2: 非法
  */
u8   ModLib_ChkIp(u8 *pData)
{
    u8 *p = pData;
    u8 i;
    u8 len;

    len = strlen((char*)p);
    
    for(i=0;i < len;i++)
    {
        if((*(p+i) > 0x39)||(*(p+i) < 0x30))
        {
            if(*(p+i) != '.')
            {
                break;
            }
        }
    }
    if(len == i)
    {
        return 0;//IP地址连接
    }
    return 1;//域名连接
}

/********************************************************************
* 名称 :  ModLib_StrByte
* 功能 : 可打印字符串转换为字节数据
* 输入 :  nSrcLength - 源字符串长度
                  pSrc - 源字符串指针
* 输出 :  pDst - 目标字符串指针
* 返回 : 目标字符串长度
* 全局变量:
* 调用函数:
* 中断资源:  (没有,不写)
* 备注: "C832" --> {0xC8, 0x32}
********************************************************************/
int ModLib_StrByte(const u8* pSrc, u8* pDst, int nSrcLength)
{
	int i;	
	if ((pSrc == NULL)||(pDst == NULL))
		return 0;

	for (i=0; i<nSrcLength; i += 2) {
        if ((*pSrc >= '0')&&(*pSrc <= '9'))// 输出高4位
            *pDst = (*pSrc - '0') << 4;
        else
            *pDst = (*pSrc - 'A' + 10) << 4;
		
        pSrc++;
        if ((*pSrc>='0') && (*pSrc<='9')) // 输出低4位
            *pDst |= *pSrc - '0';
        else
            *pDst |= *pSrc - 'A' + 10;
		
        pSrc++;
        pDst++;
    }

    return (nSrcLength / 2);
}

/********************************************************************
* 名称 :  ModLib_ByteStr
* 功能 : 字节数据转换为可打印字符串
* 输入 :  nSrcLength - 源字符串长度
                  pSrc - 源字符串指针
* 输出 :  pDst - 目标字符串指针
* 返回 : 目标字符串长度
* 全局变量:
* 调用函数:
* 中断资源:  (没有,不写)
* 备注: {0xC8, 0x32} --> "C832"
********************************************************************/
int ModLib_ByteStr(u8* pDst, const u8* pSrc, int nSrcLength)
{
    const u8 tab[]="0123456789ABCDEF";  
	int i;

	if ((pSrc == NULL)||(pDst == NULL))
		return 0;

    for (i=0; i<nSrcLength; i++) {
        *pDst++ = tab[*pSrc >> 4];      // 输出高4位
        *pDst++ = tab[*pSrc & 0x0f];    // 输出低4位
        pSrc++;
    }

    *pDst = '\0';
    return (nSrcLength * 2);
}

/********************************************************************
* 名称 :  ModLib_ByteStrEx
* 功能 : 字节数据转换为可打印字符串,并插入空格
* 输入 :  nSrcLength - 源字符串长度
                  pSrc - 源字符串指针
* 输出 :  pDst - 目标字符串指针
* 返回 : 目标字符串长度
* 全局变量:
* 调用函数:
* 中断资源:  (没有,不写)
* 备注: {0xC8, 0x32} --> "C8 32"
********************************************************************/
int ModLib_ByteStrEx(u8* pDst, const u8* pSrc, int nSrcLength)
{
    const u8 tab[]="0123456789ABCDEF";  
	int i;

	if ((pSrc == NULL)||(pDst == NULL))
		return 0;

    for (i=0; i<nSrcLength; i++) {
        *pDst++ = tab[*pSrc >> 4];      // 输出高4位
        *pDst++ = tab[*pSrc & 0x0f];    // 输出低4位
        *pDst++ = ' ';                  //插入空格
        pSrc++;
    }

    *pDst = '\0';
    return (nSrcLength * 3);
}

/********************************************************************
* 名称 :  ModLib_ByteExc
* 功能 :  两两颠倒的字符串转换为正常顺序的字符串
* 输入 :  nSrcLength - 源字符串长度
                  pSrc - 源字符串指针
* 输出 :  pDst - 目标字符串指针
* 返回 : 目标字符串长度
* 全局变量:
* 调用函数:
* 中断资源:  (没有,不写)
* 备注: "683158812764F8" --> "8613851872468" 
********************************************************************/
int ModLib_ByteExc(const u8* pSrc, u8* pDst, int nSrcLength)
{
    int nDstLength, i;								
    u8 ch;									

	if ((pSrc == NULL)||(pDst == NULL))
		return 0;
	
    nDstLength = nSrcLength;

    for (i=0; i<nSrcLength; i+=2) { // 两两颠倒
        ch = *pSrc++;							
        *pDst++ = *pSrc++;						
        *pDst++ = ch;						
    }

    if (*(pDst - 1) == 'F') {
        pDst--;
        nDstLength--;				// 目标字符串长度减1
    }

    *pDst = '\0';

    return nDstLength;
}

/********************************************************************
* 名称 : ModLib_PduEncode
* 功能 :PDU编码，用于编制、发送短消息
* 输入 : pSrc - 源PDU参数指针
* 输出 : pDst - 目标PDU串指针
* 返回 : 目标PDU串长度
* 全局变量:
* 调用函数:
* 中断资源:  (没有,不写)
* 备注:  只发8 bit 编码
********************************************************************/
int ModLib_PduEncode( SM_PARAM* pSrc, u8* pDst)
{
	int nLength;// 内部用的串长度
	int nDstLength;				
	u8  ucbuf[20]={0};									
	u8  *ptr=NULL;
    u8  format_sca[25]={0};//格式化后的短信中心号码
    u8  format_fo_mr[5]={0};       
    u8  format_tpa[25]={0};//格式化后的发送号码
    u8  format_data_type[10]={0};//格式化后的短信内容类型
    u8  format_data[300]={0};//格式化后的短信内容

    //////////////////////////////////////////////////////////格式化短信中心号码
	ptr = (u8 *)pSrc->SCA;
    nLength = strlen((char *)ptr);
    nLength = ModLib_ByteExc(ptr,format_sca,nLength);  
    ucbuf[0] = (nLength/2)+1;
	ucbuf[1] = 0x91;
    ModLib_ByteStr(format_sca, ucbuf, 2); 
    nLength = strlen((char *)ptr);
    nLength = ModLib_ByteExc(ptr,format_sca+4,nLength); 
    
    ///////////////////////////////////////////////////////////FO and MR 
    memcpy(format_fo_mr,"1100",4);
    pSrc->PDU_LN = 4;
    
    ////////////////////////////////////////////////////////////格式化发短信号码
	ptr = (u8 *)pSrc->TPA;
    nLength = strlen((char *)ptr);
    ucbuf[0] = nLength;
	ucbuf[1] = 0xa1;
    ModLib_ByteStr(format_tpa, ucbuf, 2);  
	nLength = ModLib_ByteExc(ptr, format_tpa+4, nLength);	
    pSrc->PDU_LN += (nLength + 4);
   
    //////////////////////////////////////////////////////////格式化短信内容类型
	ucbuf[0] = 0x00;    
	ucbuf[1] = GSM_8BIT;
	ucbuf[2] = 167;
    nLength = ModLib_ByteStr(format_data_type, ucbuf, 3);	
    pSrc->PDU_LN += nLength;
   
    ////////////////////////////////////////////////////////////格式化发短信内容
    ptr = (u8 *)pSrc->TP_UD;
    nLength = pSrc->udLen;
    ucbuf[0] = nLength;
    ModLib_ByteStr(format_data, ucbuf, 1);
    nLength = ModLib_ByteStr(format_data+2, ptr, nLength);
    pSrc->PDU_LN += (nLength + 2);
    
    pSrc->PDU_LN /= 2;
    
    ////////////////////////////////////////////////////////////////数据传递外部
    ptr = format_sca;//中心号码
    nLength = strlen((char *)ptr);
    memcpy(pDst,ptr,nLength);
    nDstLength = nLength;    

    ptr = format_fo_mr;//fo mr
    nLength = strlen((char *)ptr);
    memcpy(pDst+nDstLength,ptr,nLength);  
    nDstLength = nDstLength + nLength;

    ptr = format_tpa;//发送号码
    nLength = strlen((char *)ptr);
    memcpy(pDst+nDstLength,ptr,nLength);  
    nDstLength = nDstLength + nLength;  

    ptr = format_data_type;//数据类型
    nLength = strlen((char *)ptr);
    memcpy(pDst+nDstLength,ptr,nLength);
    nDstLength = nDstLength + nLength;

    ptr = format_data;//数据内容
    nLength = strlen((char *)ptr);
    memcpy(pDst+nDstLength,ptr,nLength);
    nDstLength = nDstLength + nLength;
    
	return nDstLength;
}

/********************************************************************
* 名称 : ModLib_PduDecode
* 功能 :  PDU解码，用于接收、阅读短消息
* 输入 : pSrc - 源PDU参数指针
* 输出 : pDst - 目标PDU串指针
* 返回 :用户信息串长度
* 全局变量:
* 调用函数:
* 中断资源:  (没有,不写)
* 备注: 判断如果不是8bit.返回错
********************************************************************/
int ModLib_PduDecode(const u8* pSrc, SM_PARAM*   pDst)
{
	u16 nDstLength;								
	u8  tmp;							
	u8  ucbuf[256];	
	u8  dcs;
	
	ModLib_StrByte(pSrc, &tmp, 2);		        //   SMSC地址信息段		
	tmp = (tmp - 1) * 2;						
	pSrc += 4;									
	ModLib_ByteExc(pSrc, pDst->SCA, tmp);	//   转换SMSC号码到目标PDU串
	pSrc += tmp;								
	
	ModLib_StrByte(pSrc, &tmp, 2);		        //   TPDU段基本参数		
	pSrc += 2;									
	
	ModLib_StrByte(pSrc, &tmp, 2);	            //   发送号码			
	if (tmp & 1)   
	{
		tmp   +=   1;							//   调整奇偶性
	}
	pSrc += 4;									
	ModLib_ByteExc(pSrc, pDst->TPA, tmp);	//   取TP-RA号码
	pSrc += tmp;								

	ModLib_StrByte(pSrc, (unsigned char*)&pDst->TP_PID, 2);	//   取协议标识(TP-PID)
	pSrc += 2;	 
	ModLib_StrByte(pSrc, &tmp, 2);	            //   取编码方式(TP-DCS)
    dcs = tmp;//复制编码方式

	pDst->TP_DCS = tmp;
	pSrc += 2;	 
	ModLib_ByteExc(pSrc, pDst->TP_SCTS, 14);//   服务时间戳字符串(TP_SCTS)
	pSrc += 14;	
	ModLib_StrByte(pSrc, &tmp, 2);				 //   用户信息长度(TP-UDL)
	pSrc += 2;

	if (dcs == GSM_8BIT)                         
	{
	    pDst->PDU_TP = GSM_8BIT;
        memset(ucbuf,0,sizeof(ucbuf));
    	nDstLength = ModLib_StrByte(pSrc, ucbuf, tmp * 2);//格式转换
    	memcpy(pDst->TP_UD, ucbuf, nDstLength);
	}
	else if(dcs == GSM_7BIT)
	{
        tmp = tmp - (tmp/8);
	    pDst->PDU_TP  = GSM_7BIT;
        memset(ucbuf,0,sizeof(ucbuf));
	    nDstLength = ModLib_StrByte(pSrc, ucbuf, tmp * 2);//格式转换
        nDstLength = ModLib_7BitDecode(ucbuf,pDst->TP_UD,nDstLength);
	}//31 D9 8C 56 B3 DD 70 F3
	else
	{
	    pDst->PDU_TP  = 0xaa;//未知编码
        nDstLength = 0;
	}
	
	return   nDstLength;
}

/**
  * @brief  7-bit编码
  * @param  pSrc: 源字符串指针
  *         pDst: 目标编码串指针
  *         nSrcLength: 源字符串长度
  * @retval 返回: 目标编码串长度
  */
int ModLib_7BitEncode(const char* pSrc, unsigned char* pDst, int nSrcLength)
{
    int nSrc;        // 源字符串的计数值
    int nDst;        // 目标编码串的计数值
    int nChar;       // 当前正在处理的组内字符字节的序号，范围是0-7
    unsigned char nLeft;    // 上一字节残余的数据
    
    // 计数值初始化
    nSrc = 0;
    nDst = 0;
    
    // 将源串每8个字节分为一组，压缩成7个字节
    // 循环该处理过程，直至源串被处理完
    // 如果分组不到8字节，也能正确处理
    while(nSrc<nSrcLength)
    {
        // 取源字符串的计数值的最低3位
        nChar = nSrc & 7;
    
        // 处理源串的每个字节
        if(nChar == 0)
        {
            // 组内第一个字节，只是保存起来，待处理下一个字节时使用
            nLeft = *pSrc;
        }
        else
        {
            // 组内其它字节，将其右边部分与残余数据相加，得到一个目标编码字节
            *pDst = (*pSrc << (8-nChar)) | nLeft;
    
            // 将该字节剩下的左边部分，作为残余数据保存起来
            nLeft = *pSrc >> nChar;
            // 修改目标串的指针和计数值 
            pDst++;
            nDst++; 
        } 
        
        // 修改源串的指针和计数值
        pSrc++; 
        nSrc++;
    }
    
    // 返回目标串长度
    return nDst; 
}
/**
  * @brief  7-bit解码
  * @param  pSrc: 源字符串指针
  *         pDst: 目标编码串指针
  *         nSrcLength: 源字符串长度
  * @retval 返回: 目标编码串长度
  */
u16 ModLib_7BitDecode(const u8* pSrc, u8* pDst, u16 nSrcLength)
{
    u16 nSrc;        // 源字符串的计数值
    u16 nDst;        // 目标解码串的计数值
    u16 nByte;       // 当前正在处理的组内字节的序号，范围是0-6
    u8 nLeft;    // 上一字节残余的数据
    
    // 计数值初始化
    nSrc = 0;
    nDst = 0;
    
    // 组内字节序号和残余数据初始化
    nByte = 0;
    nLeft = 0;
    
    // 将源数据每7个字节分为一组，解压缩成8个字节
    // 循环该处理过程，直至源数据被处理完
    // 如果分组不到7字节，也能正确处理
    while(nSrc<nSrcLength)
    {
        // 将源字节右边部分与残余数据相加，去掉最高位，得到一个目标解码字节
        *pDst = ((*pSrc << nByte) | nLeft) & 0x7f;
        // 将该字节剩下的左边部分，作为残余数据保存起来
        nLeft = *pSrc >> (7-nByte);
    
        // 修改目标串的指针和计数值
        pDst++;
        nDst++;
    
        // 修改字节计数值
        nByte++;
    
        // 到了一组的最后一个字节
        if(nByte == 7)
        {
            // 额外得到一个目标解码字节
            *pDst = nLeft;
    
            // 修改目标串的指针和计数值
            pDst++;
            nDst++;
    
            // 组内字节序号和残余数据初始化
            nByte = 0;
            nLeft = 0;
        }
    
        // 修改源串的指针和计数值
        pSrc++;
        nSrc++;
    }
    
    *pDst = 0;
    
    // 返回目标串长度
    return nDst;
}



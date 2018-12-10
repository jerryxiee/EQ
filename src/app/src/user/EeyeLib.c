/*************************************************************************
*
* Copyright (c) 2008,深圳市伊爱高新技术开发有限公司
* All rights reserved.
*
* 文件名称 : EeyeLib.c
* 功能     : E-EYE 库函数
*
* 当前版本 : 1.0
* 开发者   : zhulin
* 修改时间 : 2008年4月15日
*
* 历史版本 : 
* 开发者   : 
* 完成时间 : 
* 
* 备注 :
*************************************************************************/ 
//#pragma SRC
#include    "EeyeLib.h"


const u8 ASC[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
/********************************************************************
* 名称 : CharCopy
* 功能 : 字符串拷贝
*
* 输入: strDest : 目的 字符串 
*       strSrc  : 源   字符串
* 输出: len : 拷贝的长度
*
* 全局变量: 
* 调用函数:
*
* 中断资源:  (没有,不写)
*
* 备注: (修改记录内容、时间)
********************************************************************/
u8 CharCopy(const u8 *strSrc, u8  *strDest)
{
	u8  i,j;

	j = 0;
	do
	{
		i = *strSrc++;
		*strDest++ = i;
		j++;
	}while(i != 0);
	j--;
	return(j);
}

/********************************************************************
* 名称 : CharCmp
* 功能 : 比较2字串，返回0，相等；返回1，不等。
*
* 输入: strDest : 目的 字符串 
*       strSrc  : 源   字符串
* 输出: len : 拷贝的长度
*
* 全局变量: 
* 调用函数:
*
* 中断资源:  (没有,不写)
*
* 备注: (修改记录内容、时间)
********************************************************************/
u8 CharCmp(u8  * buff, const u8 * source, u8 length)
{
	u8  dat,dat0;

	do
	{
		dat = *buff++;
		dat0 = *source++;
		length--;
		if(dat != dat0)
			return(1);
	}while(length != 0);
	
	return(0);
}
/********************************************************************
* 名称 : ClearMem
* 功能 : 复位 存储器区域值
*
* 输入: ptray : 开始地址
*       len   : 长度
* 输出: 无
*
* 全局变量: 
* 调用函数:
*
* 中断资源:  (没有,不写)
*
* 备注: (修改记录内容、时间)
********************************************************************/
void ClearMem(u8 *ptray, u8 len)
{
	if(len == 0)
		return;
	do{
		*ptray++ = 0;
		len--;
	}while(len != 0);
}


/********************************************************************
* 名称 : CopyLenChar
* 功能 : 拷贝 字符串strSrc前N个字符到字符串strDest里
*
* 输入: ptray : 开始地址
*       len   : 长度
* 输出: 无
*
* 全局变量: 
* 调用函数:
*
* 中断资源:  (没有,不写)
*
* 备注: (修改记录内容、时间)
********************************************************************/
/*
void CopyLenChar(u8  *strDest, u8 * strSrc, u8 length)
{
	u8  i;

	do
	{
		i = *strSrc++;
		*strDest++ = i;
		length--;
	}while(length != 0);
	//return(j);
}
*/
/********************************************************************
* 名称 : CalTel
* 功能 : 计算电话号码个数，"**,**"
*
* 输入: teldat :电话号码
*       length :列表数据长度
* 输出: relen  :有效电话号码列表长度,该长度包含了引号及逗号,","
*		telcount:电话号码个数，0无电话，>0电话个数
* 全局变量: 
* 调用函数:
*
* 中断资源:  (没有,不写)
*
* 备注: (修改记录内容、时间)
********************************************************************/
u8 CheckTelFormat(u8  * teldat, u8 length, u8  * relen)
{
  u8  chtemp,i,telcount;
  
  chtemp = *teldat++;
  if(chtemp != '"')
    return 0;			//不合法，返回0个电话
  
  telcount = 0;
  length--;
  i = 1;
  do{
    chtemp = *teldat++;
    if(chtemp == ',' || chtemp == '"' )
      telcount++;
    //else if(chtemp < 0x30 || chtemp > 0x39)
    //	return 0;

    length--;
    i++;
  }while(chtemp != '"' && length != 0);
  
  if(chtemp != '"')
    return 0;
  
//	if(relen != 0)
  *relen = i;
          
  return(telcount);
}
/********************************************************************
* 名称 : StrLenCompare
* 功能 : 比较 字符串buff 和 字符串source 前 length 个字符
*
* 输入: buff   : 比较的字符串0
*       source : 比较的字符串1
*       length : 比较的长度
* 输出: state  : 0:两个字符串相等;  非0:两个字符串不等;
*
* 全局变量: 
* 调用函数:
*
* 中断资源:  (没有,不写)
*
* 备注: (修改记录内容、时间)
********************************************************************/
/*
u8 StrLenCompare(u8  * buff, u8 * source, u8 length)
{
	u8  dat,dat0;

	do
	{
		dat = *buff++;
		dat0 = *source++;
		length--;
		if(dat != dat0)
			return(1);
	}while(length != 0);
	
	return(0);
}
*/

/********************************************************************
* 名称 : FindStr
* 功能 : 在一个字串中查找相符的字串
*
* 输入: buff   : 
*       source : 
*       length : 
* 输出: state  : 
*
* 全局变量: 
* 调用函数:
*
* 中断资源:  (没有,不写)
*
* 备注: (修改记录内容、时间)
********************************************************************/
/*
//在一个字串中查找相符的字串
u8 FindStr(u8  * buff, u8 * source, u8 length)
{
	u8  dat,dat0;

	do
	{
		dat = *buff++;
		dat0 = *source++;
		length--;
		if(dat != dat0)
			return(1);
	}while(length != 0);
	
	return(0);
}
*/

/********************************************************************
* 名称 : ConvertHex2Asc
* 功能 : 将1个8bit数据 转换为 2个ASCII码
*
* 输入: source : 要转换的 8bit数组 
*       destin : 转换后的ASCII码数组
*       length : 转换的 8bit数据长度
* 输出: count  : 
*
* 全局变量: 
* 调用函数:
*
* 中断资源:  (没有,不写)
*
* 备注: (修改记录内容、时间)
********************************************************************/
u32 ConvertHex2Asc(u8 * source, u8 * destin, u32 length)
{
	u8  temp, datbak;//,i;
	u32  count;
	
	if(length == 0) {
		return(0);
	}
	
	count = 0;
	do{
		temp = 	*source++;
		//一个字节数据，拆分2个字符发送
		datbak = temp & 0xF0;		//high 4 bit
		datbak >>= 4;
		
		*destin++ = ASC[datbak];
		
		temp &= 0x0F;				//low 4 bit
		*destin++ = ASC[temp];
		count+=2;	
		length--;	
	}while(length != 0);

	return(count);	
}

/********************************************************************
* 名称 : Convert2Asc2Hex
* 功能 : 将2个字节的ASCII码 合成 1个HEX字节
*
* 输入: source : 要转换的 ASCII码数组
*       destin : 转换后的 HEX数组 
*       length : 转换的 HEX码数组长度
* 输出: 1:合法字符；0：非法字符
*
* 全局变量: 
* 调用函数:
*
* 中断资源:  (没有,不写)
*
* 备注: (修改记录内容、时间)
********************************************************************/
u8 Convert2Asc2Hex(u8  * source, u8 * destin, u32 length)
{
	u8  temp,temp0;
	u32  count;
	
	count = 0;
	
	do{
		////2个字节ASC码的字符，合成一个HEX字节
		temp0 = *source++;
		temp = *source++;
		if(temp0 > 0x60 && temp0 < 0x67)
			temp0 -= 0x57;
		else if(temp0 > 0x40 && temp0 < 0x47)
			temp0 -= 0x37;
		else if(temp0 >= 0x30 && temp0 <= 0x39)
			temp0 &= 0x0F;
		else return 1;
		temp0 <<= 4;
		
		if(temp > 0x60 && temp < 0x67)
			temp -= 0x57;	
		else if(temp > 0x40 && temp < 0x47)
			temp -= 0x37;
		else if(temp >= 0x30 && temp <= 0x39)
			temp &= 0x0F;
		else return 1;
		
		temp |= temp0;
		
		*destin++ = temp;
		count++;
	}while(count != length);
	
	return 1;
}

/********************************************************************
* 名称 : ConvertDec2Hex
* 功能 : 将 2个字节的10进制数字 转换为 16进制数据
*
* 输入: ch1 : 10进制数据的 10位
*       ch2 : 10进制数据的 个位
* 输出: hexData : 16进制数据
*
* 全局变量: 
* 调用函数:
*
* 中断资源:  (没有,不写)
*
* 备注: (修改记录内容、时间)
********************************************************************/
u8 ConvertDec2Hex(u8 ch1, u8 ch2)
{
	u8  chtemp;
	
	chtemp = ch1;
	chtemp &= 0x0F;
	chtemp = (chtemp << 3) + (chtemp << 1);		//b=Hi*10
	chtemp += (ch2 & 0x0F);						//b+=Lo
	
	return(chtemp);
}


/********************************************************************
* 名称 : CalCheckSum
* 功能 : 计算 数组的 校验和(异或)
*
* 输入: buff   : 要校验的数组
*       length : 需要校验的数据长度
* 输出: result : 0: 表示校验码正确;  非0:错误
*
* 全局变量: 
* 调用函数:
*
* 中断资源:  (没有,不写)
*
* 备注: (修改记录内容、时间)
********************************************************************/
u8 CalCheckSum(u8 * buff, u32 length)
{
	u8  checksum,dattemp;
	
	checksum = 0;
	do{
		dattemp = *buff++;
		checksum ^= dattemp;
		length--;
	}while(length != 0);
	
	return(checksum);
}

/********************************************************************
* 名称 : ConvertAsc2Asc
* 功能 : 解析移位超作
*
* 输入: source : 
*       destin :
*       length : 
* 输出: 无
*
* 全局变量: 
* 调用函数:
*
* 中断资源:  (没有,不写)
*
* 备注: (修改记录内容、时间)
********************************************************************/
void ConvertAsc2Asc(u8  * source, u8  * destin, u8 length)
{
	u8  count,temp,i,datbak;
	
	count = 0;
	
	if(length == 0) {
		return;
	}
	
	do{
		datbak = 0;
		for(i = 0; i < 7; i++)
		{	
			////2个字节ASC码的字符，合成一个HEX字节
			Convert2Asc2Hex(source, &temp, 1);
			source += 2;	
					
			*(destin + count) = ((temp << i) & 0x7F) | datbak;
			datbak = temp >> (7 - i);
			count++;
			
			if(i == 6) 
			{
				*(destin + count) = temp >> 1;
				count++;
			}
			
			if(count >= length) 
				break;
		}
	
	}while(count < length);
}



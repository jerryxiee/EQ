/******************************************************************** 
//版权说明	:
//文件名称	:logprintf.c		
//功能		:日志打印函数
//版本号	:
//开发人	:yjb
//开发时间	:2012.6
//修改者	:
//修改时间	:
//修改简要说明	:
//备注		:1.发送格式化字符串
//               
***********************************************************************/
//****************包含文件*****************
#include <string.h>
#include <ctype.h>        //isdigit()判断字符是否为数字,
#include <stdarg.h>       //定一可变参数
#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "Usart_drive.h"
#include "logprintf.h"

//****************宏定义********************

//****************外部变量*******************

//****************全局变量*******************
USART_TypeDef* pLogUSART = NULL;   //Log输出串口指针

static int do_padding;
static int left_flag;
static int len;
static int num1;
static int num2;
static char pad_character;

//****************函数声明*******************
void LOG_putc(int ch);
void LOG_putstr(char* str);

//****************函数定义******************
/**
  * @brief  设置日志输出串口
  * @param  None
  * @retval None
  */
void LOG_SetUsart(COM_TypeDef COM)
{
   pLogUSART = GetUsartPtr(COM);
}

/**
  * @brief  发送一个字符
  * @param  None
  * @retval None
  */
void LOG_putc(int ch)
{
    if(pLogUSART != NULL)
    {
        while((pLogUSART->SR & USART_FLAG_TC) == RESET);
        pLogUSART->DR = (u8)ch;
    }
}

/**
  * @brief  发送一个字符串
  * @param  None
  * @retval None
  */
void LOG_putstr(char* str)
{
  for(; *str; str++)
  {
      LOG_putc(*str);
  }
}


/**
  * @brief  发送一个数据缓冲区
  * @param  None
  * @retval None
  */
static void padding(const int l_flag)
{
	int i;
	
	if(do_padding && l_flag && (len < num1))
	{
		for(i=len; i<num1; i++)
		{
	  	  LOG_putc(pad_character);
		}
	}
}

/**
  * @brief  输出字符串
  * @param  None
  * @retval None
  */
static void outs(char* lp)
{
	/* 填充字符在左边*/
	len = strlen( lp);
	padding(!left_flag);
	
	/* 输出字符串，num2为最大可输出长度，例如%.5s,则num2=5 */
	while(*lp && num2--)
	{
		LOG_putc(*lp++);
	}
	
	/* 填充字符在在右边 */
	len = strlen(lp);
	padding(left_flag);
}
 /*
  * @brief  将数字(10进制或16进制数)转换为字符串
  * @param  num  : 数字
  * @param  negative  : 是否为负数,是则为1,不是为0
  * @param  base : 需要输出的字符类型,10=10进制数,16=16进制数
  * @retval None
  */
static void reoutnum(unsigned long num, unsigned int negative, const long base) 
{
	char* cp;
	char outbuf[32];
	const char digits[] = "0123456789ABCDEF";
	
	/* Build number (backwards) in outbuf             */
	cp = outbuf;
	do
	{
		*cp++ = digits[(int)(num % base)];
	} 
	while((num /= base) > 0);

	if(negative)
	{
	  *cp++ = '-';
	}

	*cp-- = 0;
	
	/* 转换后的数字移动到缓冲区，并添加需要填充的部分 */
	len = strlen(outbuf);
	padding(!left_flag);
	while(cp >= outbuf)
	{
		LOG_putc( *cp--);
	}
	padding( left_flag);
}

 /*
  * @brief  将数字转换为字符输出
  * @param  num  : 数字
  * @param  base : 需要输出的字符类型,10=10进制数,16=16进制数
  * @param  sign : 是否有符号,1-有符号数,0-是无符号数
  * @retval None
  */
static void outnum(long num, const long base, unsigned char sign /* 1-signed, 0-unsigned */)
{
	unsigned int negative;

	if((num < 0) && sign ) 
	{
		negative=1;
	        num = -num;
	}
	else
	{
		negative=0;
	}
	
	reoutnum(num, negative, base);  
}

 /*
  * @brief  将浮点数数字转换为字符输出
  * @param  num  : 数字(浮点数*1000)
  * @param  base : 需要输出的字符类型,10=10进制数,16=16进制数
  * @param  sign : 是否有符号,1-有符号数,0-是无符号数
  * @retval None
  */
static void outfloat(long num, unsigned char sign /* 1-signed, 0-unsigned */)
{
	unsigned int negative;              //1-负数,0-正数
	char* cp;
	char outbuf[32];
	const char digits[] = "0123456789";
        long base =10;                      //10进制数
        long tmp = 0;                       //临时保存数
        int tmpNum = 0;
	if((num < 0) && sign )              //如果是有符号数，且num<0则说明是负数
	{
		negative=1;
	        num = -num;
	}
	else
	{
		negative=0;
	}
	cp = outbuf;
        tmp = num%1000;                   //取小数部分
	do
	{
		*cp++ = digits[(int)(tmp % base)];
	} 
	while((tmp /= base) > 0); 
        
        *cp++ = '.';                      //加小数点
        tmp = num/1000;                    //缩小1000倍
	do
	{
                tmpNum = (int)(tmp % base);
		*cp++ = digits[tmpNum];
	} 
	while((tmp /= base) > 0);         
        
	if(negative)                      //如果是要加负号.
	{
	  *cp++ = '-';
	}
        
	*cp-- = 0;                        //设置字符结束符号
	
	/* 转换后的数字移动到缓冲区，并添加需要填充的部分 */
	len = strlen(outbuf);
	padding(!left_flag);
	while(cp >= outbuf)
	{
		LOG_putc( *cp--);
	}
	padding( left_flag);
}
 /*
  * @brief  将字符转换为数字
  * @param  linep  : 包含数字的字符串,转换完后并将指针移动到数字部分后面。
  * @retval None
  */
static int getnum(char** linep)
{
	int n = 0;
	char* cp;
	
	cp = *linep;
	while(isdigit(*cp))     //isdigit()判断字符是否为数字
	{
		n = n*10 + ((*cp++) - '0');
	}

	*linep = cp;
	return n;
}

/**
  * @brief  格式化输出数据
  *         %c 与%C等同，输出字符
  *         %s 与%S等同,输出字符串
  *         %d 输出有符号数字
  *         %D 输出无符号数字
  *         %ld 或lD 输出长整型数字
  *         %x  输出16进制数-有符号数
  *         %X  输出16进制数-无符号数
  *         %-04 如果参数输出字符不够4个，则在右边填充0,例如:("%-04d" ,12)
  *         %04  如果参数输出字符不够4个，则在左边填充0
  *         %.5s 限制字符串最大输出长度为5
  *         %f   float*1000,在输入，自动输出小数。
  *         %-010f -说明在后面补位,0要补0,10是不足10位补0
  * @param  None
  * @retval None
  */
void printu(char* ctrl, ...)
{
	int long_flag;
	int dot_flag;
	char ch;
	
	va_list argp;
	va_start(argp, ctrl);
	
	for(; *ctrl; ctrl++)
	{
	  /* 在缓冲区中发现格式化字符串'%',则替换参数*/
	  if(*ctrl != '%')
	  {
	  	LOG_putc(*ctrl);
		continue;
	  }
	
	  /* 初始化所有标志 */
	  dot_flag   = 0;
	  long_flag  = 0;
	  left_flag  = 0;
	  do_padding = 0;
	  pad_character = ' ';
	  num2=32767;

try_next:

                ch = *(++ctrl);		//获取%后的字符，例如:d,s,x
		if(isdigit(ch))         //如果字符是数字,则说明格式如:%2d
		{
			if(dot_flag)
			{
				num2 = getnum(&ctrl);     //如果%后面带数字参数,则是
			}
			else
			{
				if(ch == '0')
				{
				 	pad_character = '0';
				}
				
				num1 = getnum(&ctrl);
				do_padding = 1;
			}
			
			ctrl--;
			goto try_next;
		}
      
		switch (tolower(ch))           //将ch字符转换为小写形式
		{
		case '%':
			LOG_putc( '%');
			continue;
		
		case '-':
			left_flag = 1;
			break;
		
		case '.':
			dot_flag = 1;
			break;
		
		case 'l':
			long_flag = 1;
			break;
		
		case 'd':
			if(long_flag == 1)
			{
				if(ch == 'D')
					outnum(va_arg(argp, unsigned long), 10L , 0);
				else
					outnum(va_arg(argp, long), 10L, 1);
			}
			else
			{
				if(ch == 'D')
					outnum(va_arg(argp, unsigned int), 10L, 0);
				else
					outnum(va_arg(argp, int), 10L, 1);      //va_arg(argp, int) 获取参数值
			}
			continue;
			
		case 'x':
			if(long_flag == 1)
			{
				if(ch == 'X')
					outnum(va_arg(argp, unsigned long), 16L, 0);
				else
					outnum(va_arg(argp, long), 16L, 1);
			}
			else 
			{
				if(ch == 'X')
					outnum(va_arg(argp, unsigned int), 16L, 0);
				else
					outnum( va_arg(argp, int), 16L,1);
			}
			continue;
		case 'f':
			if(long_flag == 1)
			{
				if(ch == 'F')
					outfloat(va_arg(argp, unsigned long), 0);
				else
					outfloat(va_arg(argp, long), 1);
			}
			else 
			{
				if(ch == 'F')
					outfloat(va_arg(argp, unsigned int), 0);
				else
					outfloat(va_arg(argp, int),1);
			}
			continue;                        
		
		case 's':
			outs( va_arg( argp, char*));
			continue;
		
		case 'c':
			LOG_putc( va_arg( argp, int));
			continue;
		
		default:
			continue;
		}
	
		goto try_next;
	}

	va_end(argp);
}
/**
  * @brief  用16进制数据输出缓冲区
  *
  * @param  None
  * @retval None
  */
void printuHex(u8 *desc, u16 dlen,u8* buf,u16 len)
{
  int i = 0;
  for(i = 0 ; i < dlen ; i++)
  {
    printu("%C",desc[i]);
  }
  printu("0x");
  for(i = 0 ; i < len ; i++)
  {
    printu("%02X ",buf[i]);
  }
  printu("\r\n");
}
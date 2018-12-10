/********************************************************************************
 * File Name:			ESpin.c 
 * Function Describe:	
 * Relate Module:		lcd.c,lcd.h,Key.c,Key.h
 * Explain:
 * Writer:				joneming
 * Date:				
 * Rewriter:			joneming
 * Date:				2012-01-06
 *******************************************************************************/ 
#include "include.h"

/*************************************************************OK
** 函数名称: ESpinPoint()
** 功能描述: ESpin是否包含有小数点
** 入口参数: p:
** 出口参数: 无
** 返回参数: 1：有  0：没有
** 全局变量: 
** 调用模块: 
*************************************************************/ 
static unsigned char ESpinPoint(ST_ESPIN *p)
{
    unsigned char i;	
	unsigned char retval;	

	retval = 0;
    for(i=0; i<p->width; i++)
	{
        if(p->buffer[i]=='.')
		{
		    retval = 1;
			break;
		}
	}
	return(retval); 
}


/********************************************************************************
 * Function:SpinInit
 * initialize spin struct
 *******************************************************************************/
void ESpinInit(
	ST_ESPIN *p,
	unsigned short x,
	unsigned char y,
	unsigned char width,
	unsigned long def
)
{
	char format[ESPIN_BUFFER_MAX_LEN];
	unsigned char w;

	w =  width;
	p->x=x;
	p->y=y;
	p->focus=0;
	p->handle=def;
	if(w > ESPIN_BUFFER_MAX_LEN-1)
	    w = ESPIN_BUFFER_MAX_LEN-1;
	p->width=w;
	p->min=0;
	p->max=99999999;
	//p->border=1;//默认有框
	p->border=0;//无框
	p->loop=1;//循环
	p->type=0;
	sprintf(format,"%%0%ulu",p->width);
	sprintf(p->buffer,format,p->handle);
	p->buffer[w]=0;
}
#if 0
/********************************************************************************
 * Function:SpinInit
 * initialize spin struct
 *******************************************************************************/
void ESpinInitHex(
	ST_ESPIN *p,
	unsigned short x,
	unsigned char y,
	unsigned char width,
	unsigned long def
)
{
	char format[12];
	unsigned char w;

	w =  width;
	p->x=x;
	p->y=y;
	p->focus=0;
	p->handle=def;
	if(w > 11)
	    w = 11;
	p->width=w;
	p->min=0;
	p->max=99999999;
	//p->border=1;//默认有框
	p->border=0;//无框
	p->loop=1;//循环
	p->type=1;//十六进制
	sprintf(format,"%%0%ulx",p->width);
	sprintf(p->buffer,format,p->handle);
	p->buffer[w]=0;
}
#endif
/********************************************************************************
 * Function:SpinInit
 * initialize spin struct
 *******************************************************************************/
void ESpinInitLimit(
	ST_ESPIN *p,
	unsigned short x,
	unsigned char y,
	unsigned char width,
	unsigned long def,
	long min,
	long max
)
{
	char format[ESPIN_BUFFER_MAX_LEN];
	unsigned char w;

	w =  width;
	p->x=x;
	p->y=y;
	p->focus=0;
	p->handle=def;
	if(w > ESPIN_BUFFER_MAX_LEN-1)
	    w = ESPIN_BUFFER_MAX_LEN-1;
	p->width=w;
	p->min=min;
	p->max=max;
	p->border=0;//无框
	p->loop=1;//循环
	p->type=0;
	sprintf(format,"%%0%ulu",p->width);
	sprintf(p->buffer,format,p->handle);
	p->buffer[w]=0;
}
/********************************************************************************
 * Function:SpinSetFocus
 *******************************************************************************/
void ESpinSetFocus(ST_ESPIN *p)
{
	if(p->focus==0)
	{
		p->focus = 1;
	}
	ESpinShow(p);
}
/********************************************************************************
 * Function:SpinLoseFocus
 *******************************************************************************/
void ESpinLoseFocus(ST_ESPIN *p)
{
	if(!p->focus)return;
	p->focus = 0;            
	ESpinShow(p);
}

/********************************************************************************
 * Function:SpinShow
 * show spin
 *******************************************************************************/
void ESpinShow(ST_ESPIN *p)
{
    unsigned char x;
    char buf[ESPIN_BUFFER_MAX_LEN];

    strcpy(buf,p->buffer);
    buf[p->width]=0;
    strcpy(p->buffer,buf);

    LcdClearArea(p->x,p->y-1,p->x+(p->width)*8-1,p->y+15);  
    if(2 == p->type)//密码
    {
        memset(buf,'*',ESPIN_BUFFER_MAX_LEN);
        buf[p->width]=0;
        LcdShowStr(p->x,p->y,buf,0); 
    }
    else
    {
        LcdShowStr(p->x,p->y,p->buffer,0);
    }
    if(p->focus==1)
    {
        x=p->x;
        LcdReverseBar(x,p->y,x+7,p->y+15);
    }
    else if(p->focus>1) 
    {
        x=(p->focus-1)*8+p->x;
        LcdReverseBar(x-1,p->y,x+7,p->y+15);
    }
    if(p->border)
        LcdShowRect(p->x-1,p->y-1,p->x+p->width*8,p->y+16,0x00);
}
/********************************************************************************
 * Function:SpinShow
 * show spin
 *******************************************************************************/
void ESpinShowEx(ST_ESPIN *p)
{
    unsigned char x;
    char buf[ESPIN_BUFFER_MAX_LEN];

    strcpy(buf,p->buffer);
    buf[p->width]=0;
    strcpy(p->buffer,buf);

    LcdClearArea(p->x,p->y-1,p->x+(p->width)*8-1,p->y+15);
    if(2 == p->type)//密码
    {
        memset(buf,'*',ESPIN_BUFFER_MAX_LEN);
        buf[p->focus-1] = p->buffer[p->focus-1]; 
        buf[p->width] = 0;               
        LcdShowStr(p->x,p->y,buf,0); 
    }
    else
    {
        LcdShowStr(p->x,p->y,p->buffer,0);
    }
    if(p->focus==1)
    {
        x=p->x;
        LcdReverseBar(x,p->y,x+7,p->y+15);
    }
    else if(p->focus>1) 
    {
        x=(p->focus-1)*8+p->x;
        LcdReverseBar(x-1,p->y,x+7,p->y+15);
    }
    if(p->border)
        LcdShowRect(p->x-1,p->y-1,p->x+p->width*8,p->y+16,0x00);
}

/********************************************************************************
 * Function:SpinResponse
 * 左右移位 上下修改 
 * 已经是第1位时再左移则返回1 已经是最后一位时再右移位则返回2 其他否则返回0 
 *******************************************************************************/
unsigned char ESpinResponse(ST_ESPIN *p,unsigned char key)
{
    unsigned char temp;
    unsigned long ibuffer;
    float fbuffer;
    char  tempbuffer;
    if(!p->focus)return 0;
    switch(key)
    {   
        case KEY_UP:
            if(p->buffer[p->focus-1] !=' ')
            {
                tempbuffer = p->buffer[p->focus-1];
                /////////////////////////
                temp=p->buffer[p->focus-1]-0x30;
                ////////////////////////
                if(temp<9)temp++;
                else temp=0;
                p->buffer[p->focus-1]=temp+0x30;
                if(p->type==0)
                {
                    if(ESpinPoint(p))
                    {
                        fbuffer = atof(p->buffer);
                        ibuffer = (unsigned long)floor(fbuffer);
                    }
                    else
                    {
                        ibuffer = (unsigned long)atof(p->buffer);
                    }

                    if((ibuffer > p->max) ||  (ibuffer < p->min))
                    {
                        p->buffer[p->focus-1] = tempbuffer;
                    } 
                }
                ESpinShowEx(p);
            }
            break;
        case KEY_DOWN:            
            if(p->focus>=p->width)
            {
                if(p->loop==0) return 1;//不循环
                else
                    p->focus = 1;//
            }
            else
            {
                p->focus++;                
            }
            if(p->buffer[p->focus-1]=='.')p->focus++;
            ESpinShowEx(p);
            break;
        default:
            return 0;
    }
    return 0;

}

/********************************************************************************
 *                              End of focus
 *******************************************************************************/


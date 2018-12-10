/********************************************************************************
 * File Name:			Spin.c 
 * Function Describe:	
 * Relate Module:		lcd.c,lcd.h,Key.c,Key.h
 * Explain:
 * Writer:				joneming 
 * Date:				 
 * Rewriter:			joneming 
 * Date:		        2012-01-06 
 *******************************************************************************/
#include "include.h"


/********************************************************************************
 * Function:SpinInit
 * initialize spin struct
 *******************************************************************************/
void SpinInit(
	ST_SPIN *p,
	unsigned short x,
	unsigned char y,
	char **item,
	char *unit
)
{
	unsigned char i,len;

	p->x = x;
	p->y = y;
	p->item = item;
	p->itemnum = 0;
	i = 0;
	p->width = 0;
	p->unit = unit;
	for(;;)
	{
		if(strlen(p->item[i]) == 0)break;		
		len=strlen(p->item[i])+strlen(p->unit);
		if(len > p->width)
			p->width = len;
		i++;
		p->itemnum ++;
	}
    
	p->top = 0;
	p->bottom = p->itemnum - 1;
	if(p->handle < p->top)p->handle = p->top;
	if(p->handle > p->bottom)p->handle = p->bottom;
	p->focus = 0;
	p->disable = 0;
	p->keyflag = 0;
	p->type = 0;
	p->step = 1;
}

/********************************************************************************
 * Function:SpinInitData
 * initialize spin struct to data mode
 *******************************************************************************/
void SpinInitData(
	ST_SPIN *p,
	unsigned short x,
	unsigned char y,
	unsigned char width,
	long min,
	long max,
	long step,
	char *unit
)
{
	p->x = x;
	p->y = y;
	p->width = width;
	p->top = min;
	p->bottom = max;
	p->unit = unit;
	p->step = step;
	if(p->handle < p->top || p->handle > p->bottom)
		p->handle = p->top;
	p->focus = 0;
	p->disable = 0;
	p->keyflag = 0;
	p->type = 1;
}
#if 0
/********************************************************************************
Function:SpinInitFloatData
initialize spin struct to data mode
*******************************************************************************/
void SpinInitFloatData(
	ST_SPIN *p,
	unsigned short x,
	unsigned char y,
	unsigned char width,
	long min,
	long max,
	long step,
	unsigned char dot,
	char *unit
)
{
	p->x = x;
	p->y = y;
	//p->width = width;
    p->width = width+1;//modify by joneming 1位是小数点
	p->top = min;
	p->bottom = max;
	p->unit = unit;
	p->step = step;
	if(p->handle < p->top || p->handle > p->bottom)
		p->handle = p->top;
	p->focus = 0;
	p->disable = 0;
	p->keyflag = 0;
	p->type = 2;
	p->dot=dot;
}
/********************************************************************************
 * Function:SpinSetPage
 *******************************************************************************/
void SpinSetRange(ST_SPIN *p,short top,short bottom)
{
	p->top = top;
	p->bottom = bottom;
	if(p->type == 0)
	{
		if(p->top >= p->itemnum)p->top = p->itemnum - 1;
		if(p->bottom >= p->itemnum)p->bottom = p->itemnum - 1;
		if(p->bottom < p->top)p->bottom = p->top;
	}
	if(p->handle < p->top)p->handle = p->top;
	if(p->handle > p->bottom)p->handle = p->bottom;
}
#endif
/********************************************************************************
 * Function:SpinSetFocus
 *******************************************************************************/
void SpinSetFocus(ST_SPIN *p)
{
	if(p->disable)return;
	if(p->focus)return;
	p->focus = 1;

	LcdReverseBar(p->x,p->y,p->x+p->width*8-1,p->y+15);
}
/********************************************************************************
 * Function:SpinLoseFocus
 *******************************************************************************/
void SpinLoseFocus(ST_SPIN *p)
{
	if(p->disable)return;
	if(!p->focus)return;
	p->focus = 0;
	
	LcdReverseBar(p->x,p->y,p->x+p->width*8-1,p->y+15);
}

/********************************************************************************
 * Function:SpinShow
 * show spin
 *******************************************************************************/
void SpinShow(ST_SPIN *p)
{
    unsigned short x;
    unsigned char i;
    char format[10];
    char buffer[20];
    float fv;
    float flen;

    LcdClearArea(p->x,p->y-1,p->x+(p->width)*8-1,p->y+15);                                 

    if(p->type == 0)
    {
        strcpy(buffer,p->item[p->handle]);
        strcat(buffer,p->unit);
        LcdShowStr(p->x,p->y,buffer,0x00);

        if(p->disable)return;
    }
    else
    {
        sprintf(buffer,"%ld",p->bottom);//Modify by joneming
        flen=strlen(buffer);
        if(p->type == 1)
        {
            sprintf(format,"%%0%ulu",(unsigned short)flen);//modify by joneming
            sprintf(buffer,format,p->handle);
        }
        else if(p->type == 2)
        {
            fv=1.0;
            for(i=0; i<p->dot; i++)fv*=10.0;
            flen=flen+1+p->dot*0.1;
            sprintf(format,"%%0%3.1ff",flen);//modify by joneming
            sprintf(buffer,format,p->handle/fv);//Modify joneming
        }        
        if(!strlen(p->unit))
        {
            x=p->x+3;
        }
        else
        {
            x=p->x;
            strcat(buffer,p->unit);	
        }			
        LcdShowStr(x,p->y,buffer,0x00);
    }

    if(p->focus)LcdReverseBar(p->x,p->y,p->x+p->width*8-1,p->y+15);
    LcdShowRect(p->x-2,p->y-2,p->x+p->width*8+1,p->y+16+1,0x00);
}

/********************************************************************************
 * Function:SpinGetHandle
 * get spin's handle
 *******************************************************************************/
long SpinGetHandle(ST_SPIN *p)
{
    return p->handle;
}

/********************************************************************************
 * Function:SpinResponse
 * spin response
 *******************************************************************************/
unsigned char SpinResponse(ST_SPIN *p,unsigned char key)
{
    unsigned char task;
    if(p->disable)return 0;
    if(!p->focus)return 0;
    task = 0xFF;
    switch(key)
    {
        case KEY_UP:
            if(p->keyflag)task = 1;
            else
            task = 0;
            break;
        case KEY_DOWN:
            if(p->keyflag)task = 3;
            else
            task = 1;
            break;
        case KEY_BACK:
            return 1;
        case KEY_ENTER:
            return 2;
        default:
            break;
    }
    switch(task)
    {
        case 0:
            LcdReverseBar(p->x,p->y,p->x+p->width*8-1,p->y+15);
            if((p->handle - p->step) < p->top)p->handle = p->bottom;
            else p->handle -= p->step;
            SpinShow(p);
            break;
        case 1: 
            LcdReverseBar(p->x,p->y,p->x+p->width*8-1,p->y+15);			
            if((p->handle + p->step) > p->bottom)p->handle = p->top;
            else p->handle += p->step;
            SpinShow(p);
            break;
        case 2:
            return 3;
        case 3:
            return 4;
    }
    return 0; 
}
/********************************************************************************
 *                              End of focus
 *******************************************************************************/


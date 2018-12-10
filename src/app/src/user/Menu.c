/********************************************************************************
 * File Name:			menu.c 
 * Function Describe:	system menu module
 * Relate Module:		lcd.c,lcd.h,key.h,menu.h
 * Explain:
 * Writer:				joneming 
 * Date:				2012-01-06	
 * Rewriter:			joneming
 * Date:				2012-01-06		
 *******************************************************************************/
#include "include.h"

//define system menu parameter , lie on lcd screen's size and designer define
/////////////////////////////////////////////////////////////////////////////////
#define MENU_TOPX		0					//menu display start position x
#define MENU_TOPY		1					//menu display start position y
#define MENU_BOTTOMX	128					//menu display end position x
#define MENU_BOTTOMY	62					//menu display end position y
#define MENU_ITEM_ROWS	17					//menu item's  number of rows
#define MENU_ITEM_SPACE	1					//menu item's  space between
/******************************************************************************
 * MENU_SCR_ITEMS = (MENU_BOTTOMY - MENU_TOPY - MENU_ITEM_SPACE- 1);
 * MENU_SCR_ITEMS /= (MENU_ITEM_SPACE + MENU_ITEM_ROWS)
 *****************************************************************************/
#define MENU_SCR_ITEMS	(MENU_BOTTOMY - MENU_TOPY - MENU_ITEM_SPACE- 1)/(MENU_ITEM_SPACE + MENU_ITEM_ROWS)

//declare function only use in this module
void MenuShowItem(struct SUT_MENU *,unsigned char handle);
void MenuShowBar(struct SUT_MENU *p_psutMenu);
void MenuReShow(struct SUT_MENU *p_psutMenu,unsigned char page);
unsigned char MenuModify(struct SUT_MENU *p_psutMenu, unsigned char flag);
/********************************************************************************
 * Function:MenuInit
 * initialize menu structs
 *******************************************************************************/
void MenuInit(struct SUT_MENU *p, const char **item)
{
	unsigned char i;
	p->item = item;
	i = 0;
	for(;;)
	{
		if(strlen(p->item[i]) == 0)break;
		i ++;
	}
	p->itemnum = i;
	//check handle insure not overflow
	if((p->handle >= p->itemnum) || (p->page >= p->itemnum))
	{
		p->handle = 0;
		p->page = 0;
	}
	//check handle and page without line and overflow
	MenuModify(p,1);
}

/********************************************************************************
 * Function:MenuShow
 * display menu
 *******************************************************************************/
void MenuShow(struct SUT_MENU *p)
{
	unsigned char i,y;
    LcdClearScreen();
	if(p->itemnum < MENU_SCR_ITEMS)
		y = p->itemnum;
	else y = MENU_SCR_ITEMS;

	y *= (MENU_ITEM_ROWS + MENU_ITEM_SPACE);
	y += MENU_TOPY + MENU_ITEM_SPACE + 1;

	LcdShowRect(MENU_TOPX,MENU_TOPY,MENU_BOTTOMX,MENU_TOPY,0);
	LcdShowRect(MENU_TOPX,y,MENU_BOTTOMX,y,0);

	for(i = p->page ; i < p->itemnum ; i ++)
	{
		if((i - p->page) >= MENU_SCR_ITEMS)break;
		MenuShowItem(p,i);
	}
	MenuShowBar(p);   
    
}

/********************************************************************************
 * Function:MenuShowItem
 * display menu item
 *******************************************************************************/
void MenuShowItem(struct SUT_MENU *p,unsigned char handle)
{
	unsigned char y;

	y = handle - p->page;
	y *= MENU_ITEM_ROWS + MENU_ITEM_SPACE;
	y += MENU_TOPY + MENU_ITEM_SPACE + 1;
	if(p->item[handle][0] == '-')
		LcdShowRect(MENU_TOPX+2,y + MENU_ITEM_ROWS / 2 + 1 ,MENU_BOTTOMX-2,y + MENU_ITEM_ROWS / 2 + 1,0x10);
	else LcdShowStr(MENU_TOPX+12,y,p->item[handle],0x00);
}

/********************************************************************************
 * Function:MenuShowBar
 * display a bar 
 *******************************************************************************/
void MenuShowBar(struct SUT_MENU *p)
{
	unsigned char bary;

	bary = p->handle - p->page;
	bary *= (MENU_ITEM_ROWS + MENU_ITEM_SPACE);
	bary += MENU_TOPY + MENU_ITEM_SPACE + 1;
	//LcdReverseBar(MENU_TOPX+8,bary,MENU_BOTTOMX-8,bary + MENU_ITEM_ROWS - 1);
	#if 0
    if(!p->handle)
    {
        LcdShowArrow(MENU_TOPX+5,bary+8,4,5,0x01);

    }
    else if(p->handle==(p->itemnum-1))
    {
        LcdShowArrow(MENU_TOPX+5,bary+4,3,5,0x01);
    }
    else
    #endif
    {
    	LcdShowArrow(MENU_TOPX+4,bary+2,3,4,0x01);
    	LcdShowArrow(MENU_TOPX+4,bary+10,4,4,0x01);
    }
}

/********************************************************************************
 * Function:MenuResponse
 *******************************************************************************/
void MenuResponse(struct SUT_MENU *p,unsigned char key)
{
	unsigned char temp;
	switch(key)
	{
		case KEY_UP:
			if(p->handle==0)
            {
              p->handle = p->itemnum-1;
              MenuModify(p,0);
              MenuShow(p);
              break;
            }
			MenuShowBar(p);
			p->handle --;                       
			temp = p->page;
			if(MenuModify(p,0))MenuReShow(p,temp);
			MenuShowBar(p);
			break;
		case KEY_DOWN:
			if((p->handle + 1)>= p->itemnum)
            {
              p->handle=0;
              p->page=0;
              MenuModify(p,1);
              MenuShow(p);
              break;
            }
			MenuShowBar(p);
			p->handle ++;
			temp = p->page;
			if(MenuModify(p,1))MenuReShow(p,temp);
			MenuShowBar(p);
			break;
		//case KEY_BACK:
			//break;
		//case KEY_ENTER:
			//break;
		default:
			break;
	}
}

/********************************************************************************
 * Function:MenuGetHandle
 * get menu 's  handle
 *******************************************************************************/
unsigned char MenuGetHandle(struct SUT_MENU *p)
{
	unsigned char handle,i;

	handle = p->handle;
	for(i = 0 ; i <= p->handle ; i ++)
	{
		if(p->item[i][0] == '-')handle --;
	}
	return handle;
}

/********************************************************************************
 * Function:MenuReShow,para page is originally page
 * Re draw menu when page change
 *******************************************************************************/
void MenuReShow(struct SUT_MENU *p,unsigned char page)
{
	unsigned char start,end,row,num,h;
	unsigned char flag;

	//temp = page;
	if(p->page < page)					//move to down
	{
		row = page - p->page;
		start = 0;
 		end = MENU_SCR_ITEMS - row - 1;
		flag = 0;
		h = p->page;
	}
	else								//move to up
	{
		row = p->page - page;
		start = row;
		end = MENU_SCR_ITEMS - 1;
		flag = 1;
		h = p->page + MENU_SCR_ITEMS - row;
	}    
	num = row;
	start *= (MENU_ITEM_ROWS + MENU_ITEM_SPACE);
	start += MENU_TOPY + MENU_ITEM_SPACE;
	end *= (MENU_ITEM_ROWS + MENU_ITEM_SPACE);
	end += MENU_TOPY + MENU_ITEM_SPACE;
	end += MENU_ITEM_ROWS;
	row *= (MENU_ITEM_ROWS + MENU_ITEM_SPACE);
    
//	LcdCopy(MENU_TOPX+8,start,MENU_BOTTOMX-8,end);
	if(!flag)
	{
//		LcdPaste(MENU_TOPX+8,start+row,MENU_BOTTOMX-8,end+row);
		LcdCopyArea(MENU_TOPX+1,start,MENU_BOTTOMX-MENU_TOPX-2,end-start,MENU_TOPX+1,start+row);
		LcdClearArea(MENU_TOPX+1,start,MENU_BOTTOMX-1,start+row);
	}
	else
	{
//		LcdPaste(MENU_TOPX+8,start-row,MENU_BOTTOMX-8,end-row);
		LcdCopyArea(MENU_TOPX+1,start,MENU_BOTTOMX-MENU_TOPX-2,end-start,MENU_TOPX+1,start-row);
		LcdClearArea(MENU_TOPX+1,end-row,MENU_BOTTOMX-1,end);
	}

	for(row = 0 ; row < num ; row ++)
	{
		if(h >= p->itemnum)break;
		MenuShowItem(p,h);
		h ++;
	}
}

/********************************************************************************
 * Function:MenuModify
 * Modify menu struct handle,page's value
 * index = menu index,flag = 0 or 1 figure handle - or +
 * return 1 = page changed , 0 = no changed
 *******************************************************************************/
unsigned char MenuModify(struct SUT_MENU *p, unsigned char flag)
{
	unsigned char temp;

	temp = p->handle;
	if(flag)								//when handle ++
	{
		while(temp < p->itemnum)
		{
			if(p->item[temp][0] != '-')break;
			temp ++;
		}
		if(temp != p->handle)				//handle have no change
		{
			if(temp >= p->itemnum)temp = p->itemnum - 1;
			p->handle = temp;
		}
	}
	else									//when handle --
	{
		while(temp > 0)
		{
			if(p->item[temp][0] != '-')break;
			temp --;
		}
		if(temp != p->handle)				//handle have no change
		{
			if(temp >= p->itemnum)temp = 0;
			p->handle = temp;
		}
	}
	//check page change
	if(p->page > p->handle)
	{
		p->page = p->handle;
		return 1;
	}
	temp = p->handle - p->page;
	if(temp >= MENU_SCR_ITEMS)
	{
		p->page = p->handle - (MENU_SCR_ITEMS - 1);
		//check page without line
		temp = p->page;
		while(temp < p->itemnum)
		{
			if(p->item[temp][0] != '-')break;
			temp ++;
		}
		if(temp >= p->itemnum)temp = p->handle;
		p->page = temp;
		return 1;
	}
	return 0;
}

/********************************************************************************
 *                            End of Module
 *******************************************************************************/


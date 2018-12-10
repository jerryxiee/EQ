/********************************************************************************
 * File Name:			menu.h 
 * Function Describe:	menu.c head module
 * Relate Module:		menu.c
 * Writer:				joneming
 * Date:				2012-01-12
 * Rewriter:
 * Date:		
 *******************************************************************************/
#define ENTER	1
#define BACK	0
struct SUT_MENU
{
	const char **item;					//menu items
	unsigned char itemnum;					//item number
	unsigned char handle;					//current item
	unsigned char page;						//menu page(first item of page)
};
//initialize menu structs and variable
void MenuInit(struct SUT_MENU *, const char **);						
//show menu
void MenuShow(struct SUT_MENU *);
//response key
void MenuResponse(struct SUT_MENU *p,unsigned char key);
//get menu 's handle
unsigned char MenuGetHandle(struct SUT_MENU *);
/********************************************************************************
 *                            End of Module
 *******************************************************************************/


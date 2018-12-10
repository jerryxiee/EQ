/********************************************************************************
 * File Name:			Spin.h 
 * Function Describe:	Spin.c declare module
 * Relate Module:		No File
 * Writer:				joneming
 * Date:				2012-01-12
 * Rewriter:			
 * Date:		        
 *******************************************************************************/
 
#ifndef __SPIN_H 
#define __SPIN_H  
 
typedef struct 
{
	char **item;					//spin item
	unsigned short x;						//position x
	unsigned char y;						//position y
	unsigned char width;					//max string width
	unsigned char itemnum;					//item number 
	long top;
	long bottom;
	long handle;
	long step;
	unsigned char focus;					//input focus
	unsigned char disable;
	unsigned char keyflag;
	char *unit;
	unsigned char type;						//data/string spin   
	unsigned char dot;
}ST_SPIN;
//initialize struct
void SpinInit
(
	ST_SPIN *,
	unsigned short x,
	unsigned char y,
	char **item,
	char *unit
);

void SpinInitData
(
	ST_SPIN *p,
	unsigned short x,
	unsigned char y,
	unsigned char width,
	long min,
	long max,
	long step,
	char *unit
);

void SpinInitFloatData
(
	ST_SPIN *p,
	unsigned short x,
	unsigned char y,
	unsigned char width,
	long min,
	long max,
	long step,
	unsigned char dot,
	char *unit
);

void SpinSetRange
(
	ST_SPIN *,
	short top,
	short bottom
);

//switch spin focus
void SpinSetFocus(ST_SPIN *);
void SpinLoseFocus(ST_SPIN *);
//show spin
void SpinShow(ST_SPIN *);
//get spin 's handle
long SpinGetHandle(ST_SPIN *);
//response key
unsigned char SpinResponse(ST_SPIN *,unsigned char key);

#endif
/*********************************************************************************************************
**                            End Of File
********************************************************************************************************/


/********************************************************************************
 * File Name:			ESpin.h 
 * Function Describe:	ESpin.c declare module
 * Relate Module:		No File
 * Writer:				joneming
 * Date:				2012-01-12	
 *******************************************************************************/
 
#ifndef __ESPIN_H 
#define __ESPIN_H  
/////////////////
#define ESPIN_BUFFER_MAX_LEN 14
////////////////////
typedef struct
{
	unsigned short x;						//position x
	unsigned char y;						//position y
	unsigned char width;				//max string width
	unsigned long handle;
	unsigned long min;                  //最小值限制        
	unsigned long max;                  //最大值限制
	unsigned char focus;				//input focus 0表示无，1表示第1位 2表示第2位。。。
	unsigned char border;				//框标志 1表示有框 0表示无框
	unsigned char loop;                 //1:循环，0不循环
	unsigned char type;                 //0,1:字符串标志,2:密码
	char buffer[ESPIN_BUFFER_MAX_LEN];
}ST_ESPIN;
//initialize struct
void ESpinInit
(
	ST_ESPIN *p,
	unsigned short x,
	unsigned char y,
	unsigned char width,
	unsigned long def
);
void ESpinInitHex
(
	ST_ESPIN *p,
	unsigned short x,
	unsigned char y,
	unsigned char width,
	unsigned long def
);
void ESpinInitLimit(
	ST_ESPIN *p,
	unsigned short x,
	unsigned char y,
	unsigned char width,
	unsigned long def,
	long min,
	long max
);
//switch spin focus
void ESpinSetFocus(ST_ESPIN *);
void ESpinLoseFocus(ST_ESPIN *);
//show spin
void ESpinShow(ST_ESPIN *);
//response key
unsigned char ESpinResponse(ST_ESPIN *,unsigned char key);
#endif
/*********************************************************************************************************
**                            End Of File
********************************************************************************************************/


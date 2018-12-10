/********************************************************************************
 * File Name:			public.h 
 * Function Describe:	public.h declare module
 * Relate Module:		No File
 * Writer:				Joneming
 * Date:				  2011-09-08
 * Rewriter: 			
 * Date:				
 *******************************************************************************/
#ifndef __LZMPUBLIC__H__
#define __LZMPUBLIC__H__

//*********************头文件********************************
#include "stm32f2xx.h"
#include "stm32f2xx_conf.h"
#include "rtc.h"
/****************************************************/
/*                                                  */
/* Included files                                   */
/*                                                  */
/****************************************************/

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif


/****************************************************/
/*                                                  */
/* Structures                                       */
/*                                                  */
/****************************************************/

#define PUBLIC_CHECKBIT(X,N) ((X) & (1ul << (N)))	   //测试某一位
#define PUBLIC_SETBIT(X,N) (X)=((X) | (1ul<<(N)))       //设置某一位
#define PUBLIC_CLRBIT(X,N) (X)=((X) & (~(1ul<<(N))))    //清除某一位
////////////////////////////////////
#define PUBLIC_CHECKBIT_EX(X,N) (PUBLIC_CHECKBIT(X,N)?1:0)	   //测试某一位,返回值只有0和1
//////////////////////////////////////
#define USE_SYS_QUEUE		1//是否使用系统队列: 0:不使用  
//////////////队列参数定义//////////////////////
typedef unsigned char  QueueData;  //队列的数据类型
typedef unsigned short  Qu16;      //队列操作参数类型
#define CIRCULAR_QUEUE	0		//环形队列
#define QUEUE_TYPE		CIRCULAR_QUEUE//使用的队列类型
//队列类型
#if(QUEUE_TYPE == CIRCULAR_QUEUE)
typedef struct
{
  //////////////////////////////////////////////////////
    Qu16 datalen;				//队列长度
    Qu16 front;					//队列头
    Qu16 rear;					//队列尾
    Qu16 count;					//队列数据个数
    QueueData *data;		//队列数据	
//////////////////////////////////////////////////////
}ST_QUEUE;
#endif

//////////////////////////////////////////////////
#define USE_STACK    0   //是否使用栈 0:不使用  1:使用

typedef unsigned short  StackData;  //栈的数据类型
typedef unsigned char  SDus8;       //栈的数据类型
#define STACKMAXNUM 			12	//为了对齐数据

typedef struct
{
	StackData data[STACKMAXNUM];    //栈数据
	SDus8 top;					    //栈顶
    SDus8 bottom;					//栈底
	SDus8 count;					//栈数据个数
	SDus8 pad;					    //留用
}ST_STACK;
/////////////////栈处理函数/////////////////
void LZM_StackClear(ST_STACK *ps);
StackData LZM_StackEmpty(ST_STACK *ps);
StackData LZM_StackFull(ST_STACK *ps);
StackData LZM_StackTop(ST_STACK *ps); 
StackData LZM_StackBottom(ST_STACK *ps);
void LZM_StackPush(ST_STACK *ps,StackData sdata);
///////////////////////////////////////////
//////////////////////////////////////////
//定时器参数类型定义///////////////////////////
typedef unsigned char   LZM_RET;                     //任务的返回参数类型//
typedef unsigned short  LZM_TIMER16;                 //定时器参数类型//
typedef unsigned long   LZM_TIMER32;                 //定时器参数类型//

/////////////定时器定义宏//////////////////////
#define TIMER_TRIGGER	0                           //定时器产生消息值（定时器为减定时，减到 TIMER_TRIGGER 触发消息）	
#define ONCE			TIMER_TRIGGER               //单次定时标志值（定时器为减定时，减到 TIMER_TRIGGER 触发消息）	
#define TIMER_ENABLE	0xBA                        //开启定时器//最好为特殊值,即使没有初始化,一般也不会是该值	
#define TIMER_DISENABLE	0x10		                //关闭定时器
//时钟结构
typedef struct sttimer
{
    LZM_TIMER32 counter;	                        //定时时间（定时器为减定时）
    LZM_TIMER32 interval;	                        //重装载值
    LZM_RET     enabled;	                        //时钟开启标志 0:关闭  1:开启    
    void (*operate)(void);                          //handle independtly funtion
}LZM_TIMER;

////////////////////////////////////
#define LZM_AT_ONCE         1//最快执行
#define LZM_TIME_BASE       1//仅仅只是判断定时器是否已经到了的频率
/////////////////////////////////////////////
#define PUBLICSECS(x)	(LZM_TIMER32)((x) * SYSTICK_1SECOND)  //
/////////////////////

/****************************************************/
/*                                                  */
/* Definitions                                      */
/*                                                  */
/****************************************************/
/****************************************************/
/*                                                  */
/* Function declarations                            */
/*                                                  */
/****************************************************/
/////////////////队列处理函数/////////////////
/************************************************************
** 函数名称: LZM_QueueClear()initialize the queue
** 功能描述: 清除队列（初始化） queue is initialized to being empty 
** 入口参数: pq points to a queue 
** 出口参数:
************************************************************/ 
void LZM_QueueClear(ST_QUEUE *pq);
/************************************************************
** 函数名称: LZM_QueueFull()
** 功能描述: 检查队列是否满
** 入口参数:
** 出口参数:
************************************************************/ 
QueueData LZM_QueueFull(ST_QUEUE *pq);
/************************************************************
** 函数名称: LZM_QueueInsert()
** 功能描述: 将数据插入队列
** 入口参数:
** 出口参数:
************************************************************/ 
void LZM_QueueInsert(ST_QUEUE *pq,QueueData ucdata);
/************************************************************
** 函数名称: LZM_QueueDataBatchInsert
** 功能描述: 数据批量入队
** 入口参数: ST_QUEUE:批量入队队列,dest:目标地址,datalen:数据长度
** 出口参数: 入队的数据长度,
************************************************************/ 
Qu16 LZM_QueueDataBatchInsert(ST_QUEUE *pq,unsigned char *dest,unsigned short datalen);
/************************************************************
** 函数名称: LZM_QueueDelete()
** 功能描述: 从队列取出数据
** 入口参数:
** 出口参数:
************************************************************/ 
QueueData LZM_QueueDelete(ST_QUEUE *pq);
/************************************************************
** 函数名称: LZM_QueueDataBatchDelete
** 功能描述: 数据批量出队
** 入口参数: ST_QUEUE:批量出队队列,dest:目标地址,maxcount:最长长度,防止dest溢出
** 出口参数: 出队的数据长度,
************************************************************/ 
Qu16 LZM_QueueDataBatchDelete(ST_QUEUE *pq,unsigned char *dest,unsigned short maxcount);
/************************************************************
** 函数名称: LZM_QueueEmpty()
** 功能描述: 检查队列是否为空
** 入口参数:
** 出口参数: 0:   非空
			 非0：空
************************************************************/ 
QueueData LZM_QueueEmpty(ST_QUEUE *pq);
//////////队列保放数据的数组空间初始化/////////////////////////////////////
/************************************************************
** 函数名称: LZM_QueueClear()initialize the queue
** 功能描述: 清除队列（初始化） queue is initialized to being empty 
** 入口参数: pq points to a queue 
** 出口参数:
************************************************************/ 
void LZM_QueueDataInit(ST_QUEUE *pq,QueueData *data,int dataLen);
//////////////////
/************************************************************
** 函数名称: LZM_QueueDataCount
** 功能描述: 获取队列当前数据总长度
** 入口参数:
** 出口参数:
************************************************************/ 
Qu16 LZM_QueueDataCount(ST_QUEUE *pq);
/////////////////////////
/************************************************************
** 函数名称: LZM_QueueDataFreeSpace
** 功能描述: 获取队列当前数据剩余空间
** 入口参数:
** 出口参数:
************************************************************/ 
Qu16 LZM_QueueDataFreeSpace(ST_QUEUE *pq);
//////////////队列初始化//////////////////////
void LZM_QueueInit(void);

#if (USE_SYS_QUEUE==1)
extern ST_QUEUE g_sutSysQueue;		                    //系统消息队列(主要用于各任务的切换)
//////////////队列清除宏//////////////////////
#define Clear_SYSQueue()\
		LZM_QueueClear(&g_sutSysQueue)
//////////////入队列宏////////////////////////
#define Insert_SYSQueue(message)\
		LZM_QueueInsert(&g_sutSysQueue,message)
/////////////出队列宏////////////////////////
#define Delete_SYSQueue()\
		LZM_QueueDelete(&g_sutSysQueue)
//////////////判断队列是否为空宏/////	
#define Empty_SYSQueue()\
		LZM_QueueEmpty(&g_sutSysQueue)

/////////////IntQueue//////////////////////////
extern ST_QUEUE g_sutIntQueue;		                    //中断消息队列(主要用于中断的切换)
//////////////队列清除宏//////////////////////
#define Clear_INTQueue()\
		LZM_QueueClear(&g_sutIntQueue)
//////////////入队列宏////////////////////////
#define Insert_INTQueue(message)\
		LZM_QueueInsert(&g_sutIntQueue,message)
/////////////出队列宏////////////////////////
#define Delete_INTQueue()\
		LZM_QueueDelete(&g_sutIntQueue)
//////////////判断队列是否为空宏/////	
#define Empty_INTQueue()\
		LZM_QueueEmpty(&g_sutIntQueue)
#endif


/*************************************************************
 *                        定时器相关函数
 *************************************************************/

/************************************************************
** 函数名称: LZM_PublicSetCycTimer
** 功能描述: 重复(循环)定时
** 入口参数: stTimer:定时器变量
             value:定时值
             function:需要执行的函数
** 出口参数:
************************************************************/ 
void LZM_PublicSetCycTimer(LZM_TIMER *stTimer,LZM_TIMER32 value,void (*function)(void));
/************************************************************
** 函数名称: LZM_PublicSetOnceTimer
** 功能描述: 单次定时
** 入口参数: stTimer:定时器变量
             value:定时值
             function:需要执行的函数
** 出口参数:
************************************************************/ 
void LZM_PublicSetOnceTimer(LZM_TIMER *stTimer,LZM_TIMER32 value,void (*function)(void));
/************************************************************
** 函数名称: LZM_PublicKillTimer
** 功能描述: 关闭指定定时器
** 入口参数: stTimer:定时器变量
** 出口参数:
************************************************************/ 
void LZM_PublicKillTimer(LZM_TIMER *stTimer);
/************************************************************
** 函数名称: LZM_PublicGetTimerEnable
** 功能描述: 指定定时器是否有效
** 入口参数: stTimer:定时器指针
** 出口参数:
************************************************************/ 
LZM_RET LZM_PublicGetTimerEnable(LZM_TIMER *stTimer);
/************************************************************
** 函数名称: LZM_PublicKillTimerAll
** 功能描述: 关闭所有指定定时器
** 入口参数: stTimes:定时器变量
             maxtimes:定时器最大项
** 出口参数:
************************************************************/ 
void LZM_PublicKillTimerAll(LZM_TIMER *stTimers,unsigned char maxtimes);
/************************************************************
** 函数名称: LZM_PublicTimerHandler
** 功能描述: 应用程序时钟处理函数
** 入口参数: stTimers:定时器变量
             maxtimes:定时器最大项
** 出口参数:
************************************************************/ 
void LZM_PublicTimerHandler(LZM_TIMER *stTimers,unsigned char maxtimes);
/*************************************************************
** 函数名称: LZM_PublicTimerTask
** 功能描述: 公共定时器处理函数(对时间要求不是很高的且执行占用的时间不会很长)
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
FunctionalState LZM_PublicTimerTask(void);

//////////////////////////////////////////////////////////////
/*************************************************************
 ********以上为定时器相关函数,请勿修改****
 *************************************************************/
////////////////////////////////////////////////////////////////

/*************************************************************
** 函数名称: Public_ConvertStrToASC
** 功能描述: 把字符串的每一位(0~F)用一个Asc码表示
** 入口参数: des目标数据的首地址,src源数据的首地址,len需要转换的源数据长度
** 出口参数: des目标数据的首地址
** 返回参数: 无
** 全局变量: 
** 调用模块: 注意空间分配
*************************************************************/	
void Public_ConvertStrToASC(char * des,const char * src, unsigned short len);
/*************************************************************
** 函数名称: Public_ConvertAscToVal
** 功能描述: 把字符'0'~'F'转成0~15的数值
** 入口参数: val:需要转换的数据
** 出口参数: 无
** 返回参数: =0xff转换出错,其它则为转换结果
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned char Public_ConvertAscToVal(unsigned char val);
/*************************************************************OK
** 函数名称: Public_GetSumVerify
** 功能描述: 对数据块累加求和，只取低字节
** 入口参数: Pdata数据块首地址,datalen长度		 
** 出口参数: 
** 返回参数: 校验码
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Public_GetSumVerify(const unsigned char *Pdata,unsigned short datalen);
/*************************************************************
** 函数名称: Public_GetXorVerify
** 功能描述: 对数据块进行异或，只取低字节
** 入口参数: Pdata数据块首地址,datalen长度		 
** 出口参数: 
** 返回参数: 校验码
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Public_GetXorVerify(const unsigned char *Pdata,unsigned short datalen);
/*************************************************************
** 函数名称: Public_ConvertLongToBuffer
** 功能描述: 把unsigned long转成4位unsigned char,高位在前
** 入口参数: value:需要转换的数据
** 出口参数: buf:转换结果的首地址
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void Public_ConvertLongToBuffer(unsigned long value,unsigned char *buf);
/*************************************************************
** 函数名称: Public_ConvertBufferToLong
** 功能描述: 把4位unsigned char,高位在前的数据转成unsigned long;
             (与Public_ConvertLongToBuffer功能相反)
** 入口参数: buf:需要转换的数据的首地址
** 出口参数: 
** 返回参数: 转换结果
** 全局变量: 
** 调用模块: 
*************************************************************/
unsigned long Public_ConvertBufferToLong(unsigned char *buf);
/*************************************************************
** 函数名称: Public_ConvertBufferToShort
** 功能描述: 把2位unsigned char,高位在前的数据转成unsigned short;
** 入口参数: buf:需要转换的数据的首地址
** 出口参数: 
** 返回参数: 转换结果
** 全局变量: 
** 调用模块: 
*************************************************************/
unsigned short Public_ConvertBufferToShort(unsigned char *buf);
/*************************************************************
** 函数名称: Public_ConvertShortToBuffer
** 功能描述: 把unsigned short转成2位unsigned char,高位在前
** 入口参数: value:需要转换的数据
** 出口参数: buf:转换结果的首地址
** 返回参数: 
** 全局变量: 
** 调用模块: 
*************************************************************/	
void Public_ConvertShortToBuffer(unsigned short value,unsigned char *buf);
/*************************************************************
** 函数名称: Public_ConvertAscToBCD
** 功能描述: 把asciii码转成BCD码,(把两个字节的asciii码转成一个字节的BCD码)
** 入口参数: destin目标数据的首地址,source源数据的首地址,len需要转换的源数据长度
** 出口参数: destin目标数据的首地址
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void Public_ConvertAscToBCD(unsigned char * destin,const unsigned char* source, unsigned char len);
/*************************************************************
** 函数名称: Public_ConvertBcdToAsc
** 功能描述: 将BCD码字符串每一个字节转换成它的AscII码的字符串
** 入口参数: pAsc转换之后的数据首地址,pBcd需要转换的BCD数据首地址，BcdLen:需要转换的数据长度
** 出口参数: pAsc转换之后的数据首地址
** 返回参数: 无
** 全局变量: 
** 调用模块: 注意空间分配
*************************************************************/	
void Public_ConvertBcdToAsc(unsigned char *pAsc, const unsigned char *pBcd, unsigned char BcdLen);
/*************************************************************
** 函数名称: Public_ConvertBcdToValue
** 功能描述: 将BCD码字符串转换成它的值的字符串
** 入口参数: pValue转换之后的数据首地址,pBcd需要转换的BCD数据首地址，BcdLen:需要转换的数据长度
** 出口参数: pValue转换之后的数据首地址
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void Public_ConvertBcdToValue(unsigned char *pValue,const unsigned char *pBcd, unsigned char BcdLen);
/*************************************************************
** 函数名称: Public_JudgeBCD
** 功能描述: 测试数据是否为BCD码	  
** 入口参数: sPtr:需要测试的数据指针,Len:数据长度
** 出口参数: 无
** 返回参数: 转换结果
** 全局变量: 
** 调用模块: 
*************************************************************/
u8 Public_JudgeBCD(u8 *sPtr, u8 Len);
/*************************************************************
** 函数名称: Public_ConvertTimeToBCD
** 功能描述: 把时间转成BCD码(7位)
** 入口参数: stTime:需要转换的时间结构体,保存BCD码的首地址
** 出口参数: data:转换之后的数据首地址
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void Public_ConvertTimeToBCD(TIME_T stTime,unsigned char * data);
/*************************************************************
** 函数名称: Public_ConvertTimeToBCDEx
** 功能描述: 把时间转成BCD码(6位)
** 入口参数: stTime:需要转换的时间结构体,保存BCD码的首地址
** 出口参数: data:转换之后的数据首地址
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void Public_ConvertTimeToBCDEx(TIME_T stTime,unsigned char * data);
/*************************************************************
** 函数名称: Public_ConvertNowTimeToBCD
** 功能描述: 把当前时间转成BCD码(7位)
** 入口参数: 保存BCD码的首地址
** 出口参数: data:转换之后的数据首地址
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void Public_ConvertNowTimeToBCD(unsigned char * data);
/*************************************************************
** 函数名称: Public_ConvertNowTimeToBCDEx
** 功能描述: 把当前时间转成BCD码(6位)
** 入口参数: 保存BCD码的首地址
** 出口参数: data:转换之后的数据首地址
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void Public_ConvertNowTimeToBCDEx(unsigned char * data);
/*************************************************************
** 函数名称: Public_CheckTimeStruct
** 功能描述: 检查时间结构体内的值是否正确
** 入口参数: 检查时间结构体的首地址
** 出口参数: 
** 返回参数: 正确，返回1，错误返回0
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned char Public_CheckTimeStruct(TIME_T *tt);
/*************************************************************
** 函数名称: Public_ConvertBCDToTime
** 功能描述: 把BCD码(6位)转成时间格式
** 入口参数: BCD码的首地址
** 出口参数: stTime:转换之后的结果(转换成功才赋值)
** 返回参数: 1转换成功,0出错
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned char Public_ConvertBCDToTime(TIME_T *stTime,unsigned char * data);
/*************************************************************
** 函数名称: Public_ConvertBCDToTimeEx
** 功能描述: 把BCD码(6位)转成时间格式
** 入口参数: BCD码的首地址
** 出口参数: stTime:转换之后的结果,不判断结果
** 返回参数:  
** 全局变量: 
** 调用模块: 
*************************************************************/	
void Public_ConvertBCDToTimeEx(TIME_T *stTime,unsigned char * data);
/*************************************************************
** 函数名称: Public_HEX2BCD
** 功能描述: Byte码转换为BCD码    
** 入口参数: val:需要转换的数据
** 出口参数: 无
** 返回参数: 转换结果
** 全局变量: 
** 调用模块: 
*************************************************************/
unsigned char Public_HEX2BCD(unsigned char val);    //B码转换为BCD码
/*************************************************************
** 函数名称: Public_BCD2HEX
** 功能描述: BCD转换为Byte        
** 入口参数: val:需要转换的数据
** 出口参数: 无
** 返回参数: 转换结果
** 全局变量: 
** 调用模块: 
*************************************************************/
unsigned char Public_BCD2HEX(unsigned char val);    //BCD转换为Byte
/*************************************************************
** 函数名称: Public_CheckStrIsBCD
** 功能描述: 判断字符是否为BCD码，若是,则返回相应的数值,若不是:则返回0xff
** 入口参数: val:需要转换的数据
** 出口参数: 无
** 返回参数: =0xff转换出错,其它则为转换结果
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned char Public_CheckStrIsBCD(unsigned char val);
/*************************************************************
** 函数名称: Public_ConvertTwoAscToOneChar
** 功能描述: 把两个字符合成数值把字符'0'~'F'转成0~15的数值
** 入口参数: src:两个字符的首地址
** 出口参数: result:转换结果
** 返回参数: 0:转换出错,1转换成功
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned char Public_ConvertTwoAscToOneChar(const unsigned char *src,unsigned char *result);
/*************************************************************
** 函数名称: Public_ConvertShortToBCD
** 功能描述: 把short类数据转成BCD(2位BCD,不足的前面补0)
** 入口参数: val数据不大于9999,buffer首地址
** 出口参数: buffer转换结果的首地址
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/		
void Public_ConvertShortToBCD(unsigned short val,unsigned char *buffer);
/*************************************************************
** 函数名称: Public_ConvertShortToBCDEx
** 功能描述: 把short类数据转成BCD(3位BCD,不足的前面补0)
** 入口参数: val数据,buffer转换结果的首地址
** 出口参数: buffer转换结果的首地址
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void Public_ConvertShortToBCDEx(unsigned short val,unsigned char *buffer);
/*************************************************************
** 函数名称: Public_ConvertLongToBCD
** 功能描述: 把long类数据转成BCD(4位BCD,不足的前面补0)
** 入口参数: val数据不大于99999999,buffer转换结果的首地址
** 出口参数: buffer转换结果的首地址
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void Public_ConvertLongToBCD(unsigned long val,unsigned char *buffer);
/*************************************************************
** 函数名称: Public_ConvertLongToBCDEx
** 功能描述: 把long类数据转成BCD(5位BCD,不足的前面补0)
** 入口参数: val数据,buffer转换结果的首地址
** 出口参数: buffer转换结果的首地址
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void Public_ConvertLongToBCDEx(unsigned long val,unsigned char *buffer);
/*************************************************************
** 函数名称: Public_ConvertBCDToLong
** 功能描述: 把4位BCD转成long类数据
** 入口参数: buffer需要转换数据的首地址
** 出口参数: 转换结果
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
unsigned long Public_ConvertBCDToLong(const unsigned char *buffer);
/*************************************************************
** 函数名称: Public_CheckArrayValIsEqual
** 功能描述: 判断两个数组是否相等,
** 入口参数: array需要判断的数组1首地址,array1需要判断的数组2首地址,len需要判断的长度
** 出口参数: 无
** 返回参数: 0相等;1不相等 
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned char Public_CheckArrayValIsEqual(const unsigned char *array,const unsigned char *array1,unsigned short len);
/*************************************************************
** 函数名称: Public_WriteDataToFRAM
** 功能描述: 往铁电里写数据,在数据某尾加入校验字节
** 入口参数: address:铁电地址;data:数据首地址;datalen:写入的字节数
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: FRAM_BufferWrite
*************************************************************/
void Public_WriteDataToFRAM(unsigned short address,const unsigned char *data, unsigned short datalen);
/*************************************************************
** 函数名称: Public_ReadDataFromFRAM
** 功能描述: 从铁电里读数据
** 入口参数: address:铁电地址;data:目标数据首地址;datalen:读出的字节数
** 出口参数: data
** 返回参数: 实际读出的字节数
** 全局变量: 无
** 调用模块: FRAM_BufferRead
*************************************************************/
unsigned short Public_ReadDataFromFRAM(unsigned short address,unsigned char *data, unsigned short datalen);
/*************************************************************
** 函数名称: Public_GetCurTotalMile
** 功能描述: 获取总里程(已考虑了速度类型，不需再判断速度类型了)
** 入口参数: 
** 出口参数: 
** 返回参数: 单位是0.1公里
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned long Public_GetCurTotalMile(void);
/*************************************************************
** 函数名称: Public_Mymemcpy
** 功能描述: src的数据拷到des中,n为拷贝长度,flag为方向(1:倒序,0:顺序)
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void *Public_Mymemcpy(void *des,  const void *src, unsigned short len, unsigned char flag);
/*************************************************************OK
** 函数名称: Public_GetCurBasicPositionInfo
** 功能描述: 获得当前基本位置信息(上报格式)
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: Report_GetBasicPositionInformation
*************************************************************/
void Public_GetCurBasicPositionInfo(unsigned char *data);
/*************************************************************
** 函数名称: Public_GetCurPositionInfoDataBlock
** 功能描述: 取得当前位置信息(纬度、经度、高度:表A.20 位置信息数据块格式)
** 入口参数: 返回数据的首地址
** 出口参数: 返回数据的首地址
** 返回参数: 数据长度
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Public_GetCurPositionInfoDataBlock(unsigned char *buffer);
/*************************************************************
** 函数名称: Public_PlayTTSVoiceStr
** 功能描述: 分段播放语音,普通信息
** 入口参数: 需要播放的语音首地址
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Public_PlayTTSVoiceStr(const u8 *StrPtr);
/*************************************************************
** 函数名称: Public_PlayTTSVoiceAlarmStr
** 功能描述: 分段播放语音,报警信息
** 入口参数: 需要播放的语音首地址
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Public_PlayTTSVoiceAlarmStr(const u8 *StrPtr);
/*************************************************************
** 函数名称: Public_PlayE2ParamAlarmVoice
** 功能描述: 播放保存在E2参数中的报警信息语音
** 入口参数: 参数ID
** 出口参数: 
** 返回参数: 0:语音读取出错,1语音读取正确
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Public_PlayE2ParamAlarmVoice(unsigned short ParamID);
/*************************************************************OK
** 函数名称: Public_ShowTextInfo
** 功能描述: 即时显示文本信息(显示多行字,清屏),定时返回显示之前的界面 
** 入口参数: msg字符串首地址,time:显示时间
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: LcdShowMsgEx
*************************************************************/
void Public_ShowTextInfo(char *msg,unsigned long timeVal);
/*************************************************************
** 函数名称: Public_ShowSaveDataProBar
** 功能描述: 显示存储进度
** 入口参数: 百分比(从0开始,1.0结束显示,取值:0.0~1.0)
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Public_ShowSaveDataProBar(float persent);
/*******************************************************************************
**  函数名称  : Public_SetCarLoadStatus
**  函数功能  : 车辆载货状态 
**  输    入  : status: 0：空载、1：半载、2：满载
**  输    出  : 
**  全局变量  : 
**  调用函数  : SetEvTask(EV_REPORT);Io_WriteStatusBit
**  中断资源  : 
**  备    注  :
*******************************************************************************/
void Public_SetCarLoadStatus(unsigned char status);
/*******************************************************************************
**  函数名称  : Public_QuickOpenAccount
**  函数功能  : 快速开户
**  输    入  : 
**  输    出  : 
**  全局变量  : 
**  调用函数  : 
**  中断资源  : 
**  备    注  :
*******************************************************************************/
void Public_QuickOpenAccount(void);
/*************************************************************
** 函数名称: Public_ConvertTimeToLong
** 功能描述: 把时间结构体根据控制域转成long型
** 入口参数: ctrl:周期控制域(详见索美格式),stTime需要转换的时间结构体
** 出口参数: 
** 返回参数: 转换结果(非0,为了Public_ConvertBCDTimeToLong能区分出错) 
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned long Public_ConvertTimeToLong(unsigned short ctrl,TIME_T stTime);
/*************************************************************
** 函数名称: Public_ConvertBCDTimeToLong
** 功能描述: 把BCD码(6位)根据控制域转成long型
** 入口参数: ctrl:周期控制域(详见索美格式),data需要转换的BCD码首地址
** 出口参数: 
** 返回参数: 0:表示转换出错或不需要转换,非0:正常的转换结果 
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned long Public_ConvertBCDTimeToLong(unsigned short ctrl,unsigned char *data);
/*************************************************************
** 函数名称: Public_CheckTimeRangeIsValid
** 功能描述: 判断指定时间范围是否有效
** 入口参数: ctrl:周期控制域(详见索美格式),startTime范围开始时间点(经过Public_ConvertTimeToLong转换的值),endTime范围结束时间点(经过Public_ConvertTimeToLong转换的值)
** 出口参数: 
** 返回参数: 1:有效，0:无效
** 全局变量: 无
** 调用模块: 
*************************************************************/
unsigned char Public_CheckTimeRangeIsValid(unsigned short ctrl,unsigned long startTime,unsigned long endTime);
/*************************************************************
** 函数名称: Public_CheckTimeIsInRange
** 功能描述: 判断指定时间是否在指定范围内
** 入口参数: curTime指定时间,startTime开始时间,endTime结束时间
** 出口参数: 
** 返回参数: 1:在范围内，0:不在范围内 
** 全局变量: 无
** 调用模块: 
*************************************************************/
unsigned char Public_CheckTimeIsInRange(unsigned long curTime,unsigned long startTime,unsigned long endTime);
/*************************************************************
** 函数名称: Public_CheckCurTimeIsInTimeRange
** 功能描述: 判断当前时间是否在指定时间范围内
** 入口参数: ctrl:周期控制域(详见索美格式),startTime范围开始时间点(经过Public_ConvertTimeToLong转换的值),endTime范围结束时间点(经过Public_ConvertTimeToLong转换的值)
** 出口参数: 
** 返回参数: 1:在范围内，0:不在范围内 
** 全局变量: 无
** 调用模块: Public_ConvertTimeToLong,RTC_GetCurWeekDay
*************************************************************/
unsigned char Public_CheckCurTimeIsInTimeRange(unsigned short ctrl,unsigned long startTime,unsigned long endTime);
/*************************************************************
** 函数名称: Public_CheckTimeIsInTimeRange
** 功能描述: 判断指定时间是否在指定时间范围内
** 入口参数: ctrl:周期控制域(详见索美格式),checkTime需要判断的时间点(四个字节的时间),startTime(经过Public_ConvertTimeToLong转换的值),endTime范围结束时间点(经过Public_ConvertTimeToLong转换的值)
** 返回参数: 1:在范围内，0:不在范围内 
** 全局变量: 无
** 调用模块: Public_ConvertTimeToLong,RTC_CacWeekDay
*************************************************************/
unsigned char Public_CheckTimeIsInTimeRange(unsigned short ctrl,unsigned long checkTime,unsigned long startTime,unsigned long endTime);
/*************************************************************
** 函数名称: Public_CheckCurTimeIsInEarlyAlarmTimeRange
** 功能描述: 判断当前时间是否在指定时间的提前范围内
** 入口参数: ctrl:周期控制域(详见索美格式),earlyTime提前时间,startTime范围开始时间点(经过Public_ConvertTimeToLong转换的值),endTime范围结束时间点(经过Public_ConvertTimeToLong转换的值)
** 出口参数: 
** 返回参数: 1:在范围内，0:不在范围内 
** 全局变量: 无
** 调用模块: Public_CheckTimeIsInTimeRange
*************************************************************/
unsigned char Public_CheckCurTimeIsInEarlyAlarmTimeRange(unsigned short ctrl,unsigned long earlyTime,unsigned long startTime,unsigned long endTime);
/*******************************************************************************
**  函数名称  : Public_ShowValue
**  函数功能  : 显示数值
**  输    入  : x,y,format格式,value数值
**  输    出  : 
**  全局变量  : 
**  调用函数  : 
**  中断资源  : 
**  备    注  :
*******************************************************************************/
void Public_ShowValue(unsigned char x,unsigned char y,char *format ,unsigned long value);
/*************************************************************
** 函数名称: Public_CheckIsPhoneServerRunStatus
** 功能描述: 判断是否为电话服务的行驶状态
** 入口参数: 
** 出口参数: 
** 返回参数: 1为电话服务的行驶状态,0:非电话服务的行驶状态
** 全局变量: 
** 调用模块: 
*************************************************************/
unsigned char Public_CheckIsPhoneServerRunStatus(void);
/*************************************************************
** 函数名称: Public_CheckPhoneIsSpecialNumber
** 功能描述: 判断是否为特殊号码
** 入口参数: 
** 出口参数: 
** 返回参数: 0:非特殊号码,非0:为特殊号码,
** 全局变量: 
** 调用模块: 
*************************************************************/
unsigned char Public_CheckPhoneIsSpecialNumber(unsigned char *phone);
/*************************************************************
** 函数名称: Public_CheckPhoneInformation
** 功能描述: 检查电话号码,已根据通话时长判断电话簿以外的号码,(监听电话号码、热线电话号码不受任何限制)
** 入口参数: phone：需要检查的电话号码首地址； 
** 出口参数: name：返回的联系人姓名
** 返回参数: 0：禁止呼入/呼出；1: 呼入；2: 呼出；3：呼入/呼出
** 全局变量: 无
** 调用模块: CheckPhoneInformation,
*************************************************************/
unsigned char Public_CheckPhoneInformation(unsigned char *phone,unsigned char *name);
/*************************************************************
** 函数名称: Public_DisposePhoneStatus
** 功能描述: 处理电话状态命令
** 入口参数: phone:电话号码,status:电话状态：0：为挂断;1:接通；2:新来电
** 出口参数: 无
** 返回参数: 如果是电话呼入状态为新来电时,返回值:0:直接拒接来电;1:正在等待用户接听来电
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Public_DisposePhoneStatus(unsigned char *phone,unsigned char status);
/*******************************************************************************
**  函数名称  : Public_LcdShowTime
**  函数功能  : 显示时间
**  输    入  : y坐标,stTime时间结构体
**  输    出  : 
**  全局变量  : 
**  调用函数  : 
**  中断资源  : 
**  备    注  :
*******************************************************************************/
void Public_LcdShowTime(unsigned char y,TIME_T stTime);
/*************************************************************
** 函数名称: Public_ParameterInitialize
** 功能描述: 参数初始化
** 入口参数: 
** 出口参数: 无
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Public_ParameterInitialize(void);
/************************************************************
** 函数名称: Public_SetEIExpandTimer
** 功能描述: 
** 入口参数:
** 出口参数:
************************************************************/ 
void Public_SetEIExpandTimer(void (*function)(void),LZM_TIMER32 time);
/************************************************************
** 函数名称: Public_SetTestTimer
** 功能描述: 
** 入口参数:
** 出口参数:
************************************************************/ 
void Public_SetTestTimer(void (*function)(void),LZM_TIMER32 time);
/************************************************************
** 函数名称: Public_SetOperateTimer
** 功能描述: 
** 入口参数:
** 出口参数:
************************************************************/ 
void Public_SetOperateTimer(void (*function)(void),LZM_TIMER32 time);
/************************************************************
** 函数名称: Public_SetTaskTimer
** 功能描述: 
** 入口参数:
** 出口参数:
************************************************************/ 
void Public_SetTaskTimer(void (*function)(void),LZM_TIMER32 time);
/*************************************************************
** 函数名称: Public_CheckSystemLogoAndVerSion
** 功能描述: 检验终端产品名称、产品版本号是否正确(2字节公司标志,8字节型号,2字节PCB版本,6字节版本号,4字节校验码)
** 入口参数: 附加信息首地址,crc
** 出口参数: 无
** 返回参数:  1:检验正确,0:检验出错
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned char Public_CheckSystemLogoAndVerSion(unsigned char *pBuffer,unsigned long *checkCRC);
/*************************************************************
** 函数名称: Public_CheckTerminalAndPeripheralVerify
** 功能描述: 检验终端、外设产品名称、产品版本号是否正确(2字节公司标志,8字节型号,2字节PCB版本,6字节版本号,4字节校验码)
** 入口参数: 附加信息首地址,checkCRC返回值,type升级类型返回值
** 出口参数: 无
** 返回参数: 1:检验正确,0:检验出错
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned char Public_CheckTerminalAndPeripheralVerify(unsigned char *pBuffer,unsigned long *checkCRC,unsigned char *type);
/*************************************************************
** 函数名称: ShowStartLogo
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void Public_SaveCurSIMICcidNumberToE2Param(void);
//////////////////////////////////////////////////
/****************************************************/
/*                                                  */
/* Structures                                       */
/*                                                  */
/****************************************************/
////////////////////////////////////////////////////////
#ifndef PublicConvertValueToBuffer
/////////////////////////////////
#define PublicConvertStrToASC(des,src,len)   Public_ConvertStrToASC(des,src,len)
#define PublicGetSumVerify(Pdata,datalen)   Public_GetSumVerify(Pdata,datalen)
#define PublicGetXorVerify(Pdata,datalen)   Public_GetXorVerify(Pdata,datalen)
#define PublicConvertValueToBuffer(value,buf) Public_ConvertLongToBuffer(value,buf)
#define PublicConvertBufferToLong(buf)   Public_ConvertBufferToLong(buf)
#define PublicConvertAscToBCD(destin,source, len)   Public_ConvertAscToBCD(destin,source, len)
#define PublicConvertNowTimeToBCD(data)   Public_ConvertNowTimeToBCD(data)
#define CheckStrIsBCD(val)   Public_CheckStrIsBCD(val)
#define PublicConvertAscToVal(val)  Public_ConvertAscToVal(val)
#define PublicConvertTwoAscToOneChar(val,result)  Public_ConvertTwoAscToOneChar(val,result)
#define HEX2BCD(val)   Public_HEX2BCD(val)
#define BCD2HEX(val)   Public_BCD2HEX(val)
////////////////////
#endif
/********************************************************************************
 *                            End of File
 *******************************************************************************/
#endif//__LZMPUBLIC__H__
/********************************************************************************
 *                            End of File
 *******************************************************************************/

/********************************************************************************
 * File Name:			public.c
 * Function Describe:	
 * Relate Module:	public.h
 * Writer:				Joneming 
 * Date:				  2013-01-18
 * Rewriter: 			
 * Date:				
 *******************************************************************************/
/****************************************************/
/*                                                  */
/* Included files                                   */
/*                                                  */
/****************************************************/
#include "include.h"
/****************************************************/
/*                                                  */
/* Local Definitions and macros                     */
/*                                                  */
/****************************************************/
enum 
{
    PUBLIC_TIMER_EIEXPAND,					//
    PUBLIC_TIMER_TASK,                      //  
    PUBLIC_TIMER_TEST,						//  
    PUBLIC_TIMER_OPERATE,					//      
    PUBLIC_TIMERS_MAX
}STPUBLICTIME;
static LZM_TIMER s_stPUBLICTimer[PUBLIC_TIMERS_MAX];
///////////////////////////
static unsigned char s_ucNewCallPhoneFlag = 0;//新来电标志
//////////////////////////////////
#if (USE_SYS_QUEUE == 1) 
#define QUEUE_DATALEN_SYS     40  //队列数据长度
ST_QUEUE g_sutSysQueue;
static QueueData s_QDSysData[QUEUE_DATALEN_SYS];
//中断消息队列(主要用于中断的切换,队列中的值为中断类型，在系统中抽象为消息)
#define QUEUE_DATALEN_INT       40
ST_QUEUE g_sutIntQueue;
static QueueData s_QDIntData[QUEUE_DATALEN_INT];
#endif
/************************************************************
** 函数名称: LZM_QueueClear()initialize the queue
** 功能描述: 清除队列（初始化） queue is initialized to being empty 
** 入口参数: pq points to a queue 
** 出口参数:
************************************************************/ 
void LZM_QueueClear(ST_QUEUE *pq)
{
    #if(QUEUE_TYPE == CIRCULAR_QUEUE)
    pq->front   = 0;
    pq->rear    = 0;
    pq->count   = 0; 
    #endif
}

/************************************************************
** 函数名称: LZM_QueueEmpty()
** 功能描述: 检查队列是否为空(如果队列满了，怎么办。。。。。)
** 入口参数:
** 出口参数: 0:   非空
			 非0：空
************************************************************/ 
QueueData LZM_QueueEmpty(ST_QUEUE *pq)
{
    #if(QUEUE_TYPE == CIRCULAR_QUEUE)
    if((pq->front==pq->rear)||(0 == pq->count))
    {
        pq->count = 0;
        return 1;
    }
    return 0;
    #endif
}

/************************************************************
** 函数名称: LZM_QueueFull()
** 功能描述: 检查队列是否满
** 入口参数:
** 出口参数:
************************************************************/ 
QueueData LZM_QueueFull(ST_QUEUE *pq)
{
    #if(QUEUE_TYPE == CIRCULAR_QUEUE)
    return (pq->count == pq->datalen);
    #endif
}
/************************************************************
** 函数名称: LZM_QueueInsert()
** 功能描述: 将数据插入队列
** 入口参数:
** 出口参数:
************************************************************/ 
void LZM_QueueInsert(ST_QUEUE *pq,QueueData qdata)
{
    #if(QUEUE_TYPE == CIRCULAR_QUEUE)
    if(!LZM_QueueFull(pq))
    {  
        pq->data[pq->rear++] = qdata;
        if(pq->rear >= pq->datalen)
            pq->rear = 0;
        pq->count++;   
    }
    #endif
}

/************************************************************
** 函数名称: LZM_QueueDataBatchInsert
** 功能描述: 数据批量入队
** 入口参数: ST_QUEUE:批量入队队列,dest:目标地址,datalen:数据长度
** 出口参数: 入队的数据长度,
************************************************************/ 
Qu16 LZM_QueueDataBatchInsert(ST_QUEUE *pq,unsigned char *dest,unsigned short datalen)
{
    #if(QUEUE_TYPE == CIRCULAR_QUEUE)
    Qu16 count,count1;
    if(!LZM_QueueFull(pq))
    {
        if(pq->count + datalen > pq->datalen)
        {
            count = pq->datalen - pq->count;
        }
        else
        {
            count = datalen;
        }
        ////////////////////////////
        if(pq->rear + count > pq->datalen)
        {
            count1 =pq->datalen-pq->rear;
            memcpy(&pq->data[pq->rear],dest,count1);
            memcpy(&pq->data[0],&dest[count1],count-count1);
            pq->rear = count-count1;
        }
        else
        {
            memcpy(&pq->data[pq->rear],dest,count);
            pq->rear +=count;
        }
        //////////////////
        pq->count +=count;
        //////////////////////
        return count;
    }
    return 0;
    #endif
}
/************************************************************
** 函数名称: LZM_QueueDelete()
** 功能描述: 从队列取出数据
** 入口参数:
** 出口参数:
************************************************************/ 
QueueData LZM_QueueDelete(ST_QUEUE *pq)
{   
    #if(QUEUE_TYPE == CIRCULAR_QUEUE)
    QueueData data;
    data = 0;
    if(!LZM_QueueEmpty(pq))   
    {   
        ///////////////////////////////
        data = pq->data[pq->front];
        /////////////////////////
        if(pq->count)pq->count--;
        pq->front++;
        if(pq->front >= pq->datalen)
            pq->front = 0;
    } 
    ////////////////////////
    return data;
    #endif
}

/************************************************************
** 函数名称: LZM_QueueDataBatchDelete
** 功能描述: 数据批量出队
** 入口参数: ST_QUEUE:批量出队队列,dest:目标地址,maxcount:最长长度,防止dest溢出
** 出口参数: 出队的数据长度,
************************************************************/ 
Qu16 LZM_QueueDataBatchDelete(ST_QUEUE *pq,unsigned char *dest,unsigned short maxcount)
{
    #if(QUEUE_TYPE == CIRCULAR_QUEUE)
    Qu16 count,count1;
    if(!LZM_QueueEmpty(pq))
    {
        count = pq->count;
        /////////////////////////
        if(count>maxcount)count = maxcount;
        /////////////////////////
        pq->count -= count;
        if(pq->front + count > pq->datalen)
        {
            count1 = pq->datalen - pq->front;
            memcpy(dest,&pq->data[pq->front],count1);
            pq->front = count-count1;
            memcpy(&dest[count1],&pq->data[0],pq->front);
        }
        else
        {
            memcpy(dest,&pq->data[pq->front],count);
            pq->front += count;
        }
        //////////////////////
        return count;
    }
    return 0;
    #endif
}
/************************************************************
** 函数名称: LZM_QueueBackUpParam
** 功能描述: 备份相关队列操作参数
** 入口参数: src源队列,bak暂存队列
** 出口参数:
************************************************************/ 
void LZM_QueueBackUpParam(ST_QUEUE *src,ST_QUEUE *bak)
{
    #if(QUEUE_TYPE == CIRCULAR_QUEUE)
    bak->datalen    = src->datalen;
    bak->count      = src->count; 
    bak->front      = src->front;
    bak->rear       = src->rear;    
    #endif
}
/************************************************************
** 函数名称: LZM_QueueRecoveryParam
** 功能描述: 恢复相关队列操作参数(注意:队列实际数据根据实际情况,如果没有覆盖到备份前的数据,都可恢复)
** 入口参数: des目标队列,bak暂存队列
** 出口参数:
************************************************************/ 
void LZM_QueueRecoveryParam(ST_QUEUE *des,ST_QUEUE *bak)
{
    #if(QUEUE_TYPE == CIRCULAR_QUEUE)
    des->datalen    = bak->datalen;
    des->count      = bak->count; 
    des->front      = bak->front;
    des->rear       = bak->rear;    
    #endif
}
/************************************************************
** 函数名称: LZM_QueueDataCount
** 功能描述: 获取队列当前数据总长度
** 入口参数:
** 出口参数:
************************************************************/ 
Qu16 LZM_QueueDataCount(ST_QUEUE *pq)
{
    #if(QUEUE_TYPE == CIRCULAR_QUEUE)
    return   pq->count; 
    #endif
}

/************************************************************
** 函数名称: LZM_QueueDataFreeSpace
** 功能描述: 获取队列当前数据剩余空间
** 入口参数:
** 出口参数:
************************************************************/ 
Qu16 LZM_QueueDataFreeSpace(ST_QUEUE *pq)
{
    #if(QUEUE_TYPE == CIRCULAR_QUEUE)
    return   (pq->datalen - pq->count); 
    #endif
}
/************************************************************
** 函数名称: LZM_QueueClear()initialize the queue
** 功能描述: 清除队列（初始化） queue is initialized to being empty 
** 入口参数: pq points to a queue 
** 出口参数:
************************************************************/ 
void LZM_QueueDataInit(ST_QUEUE *pq,QueueData *data,int dataLen)
{
    #if(QUEUE_TYPE == CIRCULAR_QUEUE)
    pq->data= data;
    pq->datalen = dataLen;
    LZM_QueueClear(pq);
    #endif
}

/************************************************************
** 函数名称: LZM_QueueInit()
** 功能描述: 
** 入口参数:
** 出口参数:
************************************************************/ 
void LZM_QueueInit(void)
{
    #if(QUEUE_TYPE == CIRCULAR_QUEUE)
    #if (USE_SYS_QUEUE==1)
    LZM_QueueDataInit(&g_sutSysQueue,s_QDSysData,QUEUE_DATALEN_SYS);
    LZM_QueueDataInit(&g_sutIntQueue,s_QDIntData,QUEUE_DATALEN_INT);
    #endif
    #endif
}

#if(USE_STACK == 1)
/************************************************************
** 函数名称: LZM_StackClear()initialize the stack
** 功能描述: 清除栈（初始化） stack is initialized to being empty 
** 入口参数: ps points to a stack 
** 出口参数:
************************************************************/ 
void LZM_StackClear(ST_STACK *ps)
{
    ps->top = 0;    
    ps->count = 0;
    ps->bottom = 0;
}

/************************************************************
** 函数名称: LZM_StackEmpty()
** 功能描述: 检查栈是否为空(如果队列满了，怎么办。。。。。)
** 入口参数:
** 出口参数: 0:   非空
			 非0：空
************************************************************/ 
StackData LZM_StackEmpty(ST_STACK *ps)
{
    return(0 == ps->count); 
}


/************************************************************
** 函数名称: LZM_StackFull()
** 功能描述: 检查栈是否满
** 入口参数:
** 出口参数: 0:   非满
			 非0：满
************************************************************/ 
StackData LZM_StackFull(ST_STACK *ps)
{
    return (ps->count >= (STACKMAXNUM - 1));
}	  

/************************************************************
** 函数名称: LZM_StackTop
** 功能描述: 将数据从栈顶出栈
** 入口参数:
** 出口参数:
************************************************************/ 
StackData LZM_StackTop(ST_STACK *ps)
{
    StackData data;
    data = 0;
    if(!LZM_StackEmpty(ps))   
    {
        if(ps->top == 0)
            ps->top = STACKMAXNUM;
        ps->top--;
        ps->count--;
        data = ps->data[ps->top];
    }
    return data;
}

/************************************************************
** 函数名称: LZM_StackBottom()
** 功能描述: 将数据从栈底出栈
** 入口参数:
** 出口参数:
************************************************************/ 
StackData LZM_StackBottom(ST_STACK *ps)
{
    StackData data;
    data =0;
    if(!LZM_StackEmpty(ps))   
    { 
        data = ps->data[ps->bottom];
        ps->bottom++;
        if(ps->bottom == STACKMAXNUM)
            ps->bottom = 0;
        ps->count--;
    }
    /////////////////////
    return data;
}
/************************************************************
** 函数名称: LZM_StackPush()
** 功能描述: 将数据入栈
** 入口参数:
** 出口参数:
************************************************************/ 
void LZM_StackPush(ST_STACK *ps,StackData sdata)
{
    if(!LZM_StackFull(ps))   
    { 
        ps->data[ps->top++] = sdata;  
        if(ps->top >= STACKMAXNUM) 
            ps->top = 0;   
        ps->count++;
    }
}
#endif

/*****************************************************
*
*       Public  code
*
*****************************************************/

/************************************************************
** 函数名称: LZM_PublicSetCycTimer
** 功能描述: 重复(循环)定时
** 入口参数: stTimer:定时器指针变量
             value:定时值
             function:需要执行的函数
** 出口参数:
************************************************************/ 
void LZM_PublicSetCycTimer(LZM_TIMER *stTimer,LZM_TIMER32 value,void (*function)(void))
{
    stTimer->enabled  = TIMER_ENABLE;
    stTimer->counter  = value+Timer_Val();
    stTimer->interval = value;
    stTimer->operate = function;
}
/************************************************************
** 函数名称: LZM_PublicSetOnceTimer
** 功能描述: 单次定时
** 入口参数: stTimer:定时器指针变量
             value:定时值
             function:需要执行的函数
** 出口参数:
************************************************************/ 
void LZM_PublicSetOnceTimer(LZM_TIMER *stTimer,LZM_TIMER32 value,void (*function)(void))
{
    stTimer->enabled  = TIMER_ENABLE;
    stTimer->counter  = value+Timer_Val();
    stTimer->interval = ONCE;
    stTimer->operate = function;
}
/************************************************************
** 函数名称: LZM_PublicKillTimer
** 功能描述: 关闭指定定时器
** 入口参数: stTimer:定时器指针
** 出口参数:
************************************************************/ 
void LZM_PublicKillTimer(LZM_TIMER *stTimer)
{
    stTimer->enabled  = TIMER_DISENABLE;
}
/************************************************************
** 函数名称: LZM_PublicGetTimerEnable
** 功能描述: 指定定时器是否有效
** 入口参数: stTimer:定时器指针
** 出口参数:
************************************************************/ 
LZM_RET LZM_PublicGetTimerEnable(LZM_TIMER *stTimer)
{
    return (TIMER_ENABLE == stTimer->enabled)? 1 : 0;
}
/************************************************************
** 函数名称: LZM_PublicKillTimerAll
** 功能描述: 关闭所有指定定时器
** 入口参数: stTimes:定时器变量
             maxtimes:定时器最大项
** 出口参数:
************************************************************/ 
void LZM_PublicKillTimerAll(LZM_TIMER *stTimers,unsigned char maxtimes)
{
    unsigned char i;
    for(i = 0; i < maxtimes; i++)
    {
        stTimers[i].enabled = TIMER_DISENABLE;
    }
}
/************************************************************
** 函数名称: LZM_PublicTimerHandler
** 功能描述: 应用程序定时器处理函数
** 入口参数: stTimes:定时器变量
             maxtimes:定时器最大项
** 出口参数:
************************************************************/ 
void LZM_PublicTimerHandler(LZM_TIMER *stTimers,unsigned char maxtimes)
{
    unsigned char i;
    unsigned long timerVal;
    timerVal=Timer_Val();
    for(i = 0; i < maxtimes; i++)
    {
        if(TIMER_ENABLE == stTimers[i].enabled)
        {
            if(stTimers[i].counter <= timerVal)//
            {
                if(ONCE == stTimers[i].interval)//单次
                {
                    stTimers[i].enabled = TIMER_DISENABLE;
                }
                else//重复定时器
                {
                    stTimers[i].counter += stTimers[i].interval;
                    if(stTimers[i].counter< timerVal)
                        stTimers[i].counter = timerVal;
                }
                //可以在该函数体中重复定义该定时器
                if(NULL != stTimers[i].operate)
                stTimers[i].operate();
            }
        }
    }
}

/*************************************************************
** 函数名称: Public_ConvertStrToASC
** 功能描述: 把字符串的每一个字节用两个16进制数表示(与平台对应)
** 入口参数: des目标数据的首地址,src源数据的首地址,len需要转换的源数据长度
** 出口参数: des目标数据的首地址
** 返回参数: 无
** 全局变量: 
** 调用模块: 注意空间分配
*************************************************************/	
void Public_ConvertStrToASC(char * des,const char * src, unsigned short len)
{
    unsigned short i;
    for(i = 0; i < len; i++)
    {
        sprintf(&des[2*i],"%2X",src[i]);
    }
}
/*************************************************************
** 函数名称: Public_ConvertAscToVal
** 功能描述: 把字符'0'~'F'转成0~15的数值
** 入口参数: val:需要转换的数据
** 出口参数: 无
** 返回参数: =0xff转换出错,其它则为转换结果
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned char Public_ConvertAscToVal(unsigned char val)
{
    unsigned char temp;
    temp = 0xff;
    if(val>=0x30&&val<=0x39)
    {
        temp = val-0x30;
    }
    else
    if(val>=0x41&&val<=0x46)
    {
        temp = val-0x41+10;
    }
    return temp;
}
/*************************************************************OK
** 函数名称: Public_GetSumVerify
** 功能描述: 对数据块累加求和，只取低字节
** 入口参数: Pdata数据块首地址,datalen长度		 
** 出口参数: 
** 返回参数: 校验码
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Public_GetSumVerify(const unsigned char *Pdata,unsigned short datalen)
{
    //对数据块累加求和，只取低字节
    unsigned char sum = 0;
    unsigned short  i;
    IWDG_ReloadCounter();//喂狗
    for(i = 0; i < datalen; i++)
    {
        sum += Pdata[i];
    }
    IWDG_ReloadCounter();//喂狗
    return sum;	  
}
/*************************************************************
** 函数名称: Public_GetXorVerify
** 功能描述: 对数据块进行异或，只取低字节
** 入口参数: Pdata数据块首地址,datalen长度		 
** 出口参数: 
** 返回参数: 校验码
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Public_GetXorVerify(const unsigned char *Pdata,unsigned short datalen)
{
    //对数据块进行异或，只取低字节
    unsigned char Verify ;
    unsigned short  i;
    //////////////////
    Verify= Pdata[0];
    IWDG_ReloadCounter();//喂狗
    for(i = 1; i < datalen; i++)
    {
        Verify ^= Pdata[i];
    }
    IWDG_ReloadCounter();//喂狗
    return Verify;	  
}

/*************************************************************
** 函数名称: Public_ConvertAscToBCD
** 功能描述: 把asciii码转成BCD码,(把两个字节的asciii码转成一个字节的BCD码)
** 入口参数: destin目标数据的首地址,source源数据的首地址,len需要转换的源数据长度
** 出口参数: destin目标数据的首地址
** 返回参数: 无
** 全局变量: destin使用时注意destin的空间分配
** 调用模块: 
*************************************************************/	
void Public_ConvertAscToBCD(unsigned char * destin,const unsigned char* source, unsigned char len)
{
    unsigned char i;
    unsigned char chtemp,chhigh,chlow;
    for(i = 0; i < len; i++)
    {
        chtemp = *source++;
        chhigh = chtemp & 0x0F;
        chtemp = *source++;
        chlow = chtemp & 0x0F;
        chhigh <<= 4; 
        chhigh |= chlow;
        *destin++ = chhigh;
    }
}
/*************************************************************
** 函数名称: Public_ConvertBcdToAsc
** 功能描述: 将BCD码字符串每一个字节转换成它的AscII码的字符串
** 入口参数: pAsc转换之后的数据首地址,pBcd需要转换的BCD数据首地址，BcdLen:需要转换的数据长度
** 出口参数: pAsc转换之后的数据首地址
** 返回参数: 无
** 全局变量: 
** 调用模块: 注意空间分配
*************************************************************/	
void Public_ConvertBcdToAsc(unsigned char *pAsc, const unsigned char *pBcd, unsigned char BcdLen)
{
    unsigned char	i;
    for(i=0; i<BcdLen; i++)
    {
        *(pAsc+2*i) = (*(pBcd+i) >> 4)+0x30;
        *(pAsc+2*i+1) = (*(pBcd+i) & 0x0f)+0x30;
    }
}
/*************************************************************
** 函数名称: Public_ConvertBcdToValue
** 功能描述: 将BCD码字符串转换成它的值的字符串
** 入口参数: pValue转换之后的数据首地址,pBcd需要转换的BCD数据首地址，BcdLen:需要转换的数据长度
** 出口参数: pValue转换之后的数据首地址
** 返回参数: 无
** 全局变量: 
** 调用模块: 注意空间分配
*************************************************************/	
void Public_ConvertBcdToValue(unsigned char *pValue, const unsigned char *pBcd, unsigned char BcdLen)
{
    unsigned char	i;
    for(i=0; i<BcdLen; i++)
    {
        *(pValue+i) = (*(pBcd+i) >> 4)*10+(*(pBcd+i) & 0x0f);
    }
}
/*************************************************************
** 函数名称: Public_ConvertTimeToBCD
** 功能描述: 把时间转成BCD码(7位)
** 入口参数: stTime:需要转换的时间结构体,保存BCD码的首地址
** 出口参数: data:转换之后的数据首地址
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void Public_ConvertTimeToBCD(TIME_T stTime,unsigned char * data)
{
    unsigned char buffer[20];
    unsigned char buffer1[20];
    sprintf((char *)buffer,"20%02d%02d%02d%02d%02d%02d",stTime.year,stTime.month,stTime.day,stTime.hour,stTime.min,stTime.sec);
    Public_ConvertAscToBCD(buffer1,buffer,strlen((char *)buffer));
    memcpy(data,buffer1,7);
}
/*************************************************************
** 函数名称: Public_ConvertTimeToBCDEx
** 功能描述: 把时间转成BCD码(6位)
** 入口参数: stTime:需要转换的时间结构体,保存BCD码的首地址
** 出口参数: data:转换之后的数据首地址
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void Public_ConvertTimeToBCDEx(TIME_T stTime,unsigned char * data)
{
    unsigned char buffer[20];
    unsigned char buffer1[20];
    sprintf((char *)buffer,"%02d%02d%02d%02d%02d%02d",stTime.year,stTime.month,stTime.day,stTime.hour,stTime.min,stTime.sec);
    Public_ConvertAscToBCD(buffer1,buffer,strlen((char *)buffer));
    memcpy(data,buffer1,6);
}
/*************************************************************
** 函数名称: Public_ConvertNowTimeToBCD
** 功能描述: 把当前时间转成BCD码(7位)
** 入口参数: 保存BCD码的首地址
** 出口参数: data:转换之后的数据首地址
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void Public_ConvertNowTimeToBCD(unsigned char * data)
{
    TIME_T now_time;
    RTC_GetCurTime(&now_time);
    Public_ConvertTimeToBCD(now_time,data);
}
/*************************************************************
** 函数名称: Public_ConvertNowTimeToBCDEx
** 功能描述: 把当前时间转成BCD码(6位)
** 入口参数: 保存BCD码的首地址
** 出口参数: data:转换之后的数据首地址
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void Public_ConvertNowTimeToBCDEx(unsigned char * data)
{
    TIME_T now_time;
    RTC_GetCurTime(&now_time);
    Public_ConvertTimeToBCDEx(now_time,data);
}
/*************************************************************
** 函数名称: Public_CheckTimeStruct
** 功能描述: 检查时间结构体内的值是否正确
** 入口参数: 检查时间结构体的首地址
** 出口参数: 
** 返回参数: 正确，返回1，错误返回0
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned char Public_CheckTimeStruct(TIME_T *tt)
{
    //检查年、月、日、时、分、秒时间参数
    if((tt->year > END_YEAR)||(tt->year < 0)||
    (tt->month > 12)||(tt->month < 1)||
    (tt->day > 31)||(tt->day < 1)||
    (tt->hour > 23)||(tt->hour < 0)||
    (tt->min > 59)||(tt->min < 0)||
    (tt->sec > 59)||(tt->sec < 0))
    {
        return 0;
    }
    else
    {
        return 1;
    }
}
/*************************************************************
** 函数名称: Public_ConvertBCDToTime
** 功能描述: 把BCD码(6位)转成时间格式
** 入口参数: BCD码的首地址
** 出口参数: stTime:转换之后的结果(转换成功才赋值)
** 返回参数: 1转换成功,0出错
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned char Public_ConvertBCDToTime(TIME_T *stTime,unsigned char * data)
{
    unsigned char buffer[20];
    Public_ConvertBcdToValue(buffer,data,6);
    if(Public_CheckTimeStruct((TIME_T *)buffer))
    {
        memcpy(stTime, buffer,6);
        return 1;
    }
    return 0;
}
/*************************************************************
** 函数名称: Public_ConvertBCDToTimeEx
** 功能描述: 把BCD码(6位)转成时间格式
** 入口参数: BCD码的首地址
** 出口参数: stTime:转换之后的结果,不判断结果
** 返回参数:  
** 全局变量: 
** 调用模块: 
*************************************************************/	
void Public_ConvertBCDToTimeEx(TIME_T *stTime,unsigned char * data)
{
    unsigned char buffer[20];
    Public_ConvertBcdToValue(buffer,data,6);
    memcpy(stTime, buffer,6);
}
/*************************************************************
** 函数名称: Public_ConvertLongToBuffer
** 功能描述: 把unsigned long转成4位unsigned char,高位在前
** 入口参数: value:需要转换的数据
** 出口参数: buf:转换结果的首地址
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void Public_ConvertLongToBuffer(unsigned long value,unsigned char *buf)
{
    unsigned char i;
    for(i= 0; i<4; i++)
    {
        buf[3-i]=value>>(i<<3);
    }
}
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
unsigned long Public_ConvertBufferToLong(unsigned char *buf)
{
    unsigned long value;
    value = 0;
    value |= buf[0]<<24;
    value |= buf[1]<<16;
    value |= buf[2]<<8;
    value |= buf[3];
    return value;
}
/*************************************************************
** 函数名称: Public_ConvertBufferToShort
** 功能描述: 把2位unsigned char,高位在前的数据转成unsigned short;
** 入口参数: buf:需要转换的数据的首地址
** 出口参数: 
** 返回参数: 转换结果
** 全局变量: 
** 调用模块: 
*************************************************************/
unsigned short Public_ConvertBufferToShort(unsigned char *buf)
{
    unsigned short value;
    value = 0;
    value |= buf[0]<<8;
    value |= buf[1];
    return value;
}
/*************************************************************
** 函数名称: Public_ConvertShortToBuffer
** 功能描述: 把unsigned short转成2位unsigned char,高位在前
** 入口参数: value:需要转换的数据
** 出口参数: buf:转换结果的首地址
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void Public_ConvertShortToBuffer(unsigned short value,unsigned char *buf)
{
    buf[0] = value>>8;
    buf[1] = value&0xff;
}
/*************************************************************
** 函数名称: Public_BCD2HEX
** 功能描述: BCD转换为Byte        
** 入口参数: val:需要转换的数据
** 出口参数: 无
** 返回参数: 转换结果
** 全局变量: 
** 调用模块: 
*************************************************************/
unsigned char Public_BCD2HEX(unsigned char val)    //BCD转换为Byte
{
    unsigned char temp;
    temp=val&0x0f;
    val>>=4;
    val&=0x0f;
    val*=10;
    temp+=val;
   
    return temp;
}
/*************************************************************
** 函数名称: Public_JudgeBCD
** 功能描述: 测试数据是否为BCD码      
** 入口参数: sPtr:需要测试的数据指针,Len:数据长度
** 出口参数: 无
** 返回参数: 转换结果
** 全局变量: 
** 调用模块: 
*************************************************************/
u8 Public_JudgeBCD(u8 *sPtr, u8 Len)
{
	u8 Temp;

	while (Len--)
	{
	    Temp = *sPtr++;
	 if (((Temp >> 4  ) >= 10) || ((Temp & 0x0F) >= 10))
	    {
	        return 0;
	    }
	}

	return 1;
}

/*************************************************************
** 函数名称: Public_HEX2BCD
** 功能描述: Byte码转换为BCD码    
** 入口参数: val:需要转换的数据
** 出口参数: 无
** 返回参数: 转换结果
** 全局变量: 
** 调用模块: 
*************************************************************/
unsigned char Public_HEX2BCD(unsigned char val)    //Byte码转换为BCD码
{
    unsigned char i,j,k;
    i=val/10;
    j=val%10;
    k=j+(i<<4);
    return k;
}
/*************************************************************
** 函数名称: Public_ConvertTwoAscToOneChar
** 功能描述: 把两个字符合成数值(把字符'0'~'F'转成0~15的数值)
** 入口参数: src:两个字符的首地址
** 出口参数: result:转换结果
** 返回参数: 0:转换出错,1转换成功
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned char Public_ConvertTwoAscToOneChar(const unsigned char *val,unsigned char *result)
{
    unsigned char temp,temp1;
    temp1=Public_ConvertAscToVal(val[0]);
    if(temp1==0xff)return 0;
    temp=Public_ConvertAscToVal(val[1]);
    if(temp==0xff)return 0;
    /////////////////
    temp1=temp1*16+temp;
    *result=temp1;
    return 1;
}
/*************************************************************
** 函数名称: Public_CheckStrIsBCD
** 功能描述: 判断字符是否为BCD码，若是,则返回相应的数值,若不是:则返回0xff
** 入口参数: val:需要转换的数据
** 出口参数: 无
** 返回参数: =0xff转换出错,其它则为转换结果
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned char Public_CheckStrIsBCD(unsigned char val)
{
    unsigned char temp,temp1;
    temp=val/16;
    temp1=val%16;
    if(temp>9||temp1>9)
    {
        return 0xff;
    }
    else
    {
        return (temp*10+temp1);
    }
}
/*************************************************************
** 函数名称: Public_ConvertShortToBCD
** 功能描述: 把short类数据转成BCD(2位BCD,不足的前面补0)
** 入口参数: val数据不大于9999,buffer转换结果的首地址
** 出口参数: buffer转换结果的首地址
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void Public_ConvertShortToBCD(unsigned short val,unsigned char *buffer)
{
    unsigned char temp[20],temp1[20];  
    if(val>9999) val = 9999;
    sprintf((char *)temp,"%04d",val);
    Public_ConvertAscToBCD(temp1,temp,4);
    memcpy(buffer,temp1,2);
}
/*************************************************************
** 函数名称: Public_ConvertShortToBCDEx
** 功能描述: 把short类数据转成BCD(3位BCD,不足的前面补0)
** 入口参数: val数据,buffer转换结果的首地址
** 出口参数: buffer转换结果的首地址
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void Public_ConvertShortToBCDEx(unsigned short val,unsigned char *buffer)
{
    unsigned char temp[20],temp1[20];  
    sprintf((char *)temp,"%06d",val);
    Public_ConvertAscToBCD(temp1,temp,6);
    memcpy(buffer,temp1,3);
}
/*************************************************************
** 函数名称: Public_ConvertBCDToShort
** 功能描述: 把2位BCD转成Short类数据
** 入口参数: buffer首地址
** 出口参数: 无
** 返回参数: 转换之后的数值
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned short Public_ConvertBCDToShort(unsigned char *buffer)
{
    unsigned char data[20];
    unsigned short val;
    Public_ConvertBcdToValue(buffer,data,4);
    val=100ul*data[0]+data[1];
    return val;
}
/*************************************************************
** 函数名称: Public_ConvertLongToBCD
** 功能描述: 把long类数据转成BCD(4位BCD,不足的前面补0)
** 入口参数: val数据不大于99999999,buffer转换结果的首地址
** 出口参数: buffer转换结果的首地址
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void Public_ConvertLongToBCD(unsigned long val,unsigned char *buffer)
{
    unsigned char temp[20],temp1[20];
    if(val>99999999) val = 99999999;
    sprintf((char *)temp,"%08d",val);
    Public_ConvertAscToBCD(temp1,temp,8);
    memcpy(buffer,temp1,4);
}
/*************************************************************
** 函数名称: Public_ConvertLongToBCDEx
** 功能描述: 把long类数据转成BCD(5位BCD,不足的前面补0)
** 入口参数: val数据,buffer首地址
** 出口参数: buffer转换结果的首地址
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void Public_ConvertLongToBCDEx(unsigned long val,unsigned char *buffer)
{
    unsigned char temp[20],temp1[20];
    sprintf((char *)temp,"%010d",val);
    Public_ConvertAscToBCD(temp1,temp,10);
    memcpy(buffer,temp1,5);
}
/*************************************************************
** 函数名称: Public_ConvertBCDToLong
** 功能描述: 把4位BCD转成long类数据
** 入口参数: buffer需要转换数据的首地址
** 出口参数: 转换结果
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
unsigned long Public_ConvertBCDToLong(const unsigned char *buffer)
{
    unsigned char data[20];
    unsigned long val;
    Public_ConvertBcdToValue(data,buffer,4);
    val=1000000ul*data[0]+10000ul*data[1]+100ul*data[2]+data[3];
    return val;
}
/*************************************************************
** 函数名称: Public_CheckArrayValIsEqual
** 功能描述: 判断两个数组是否相等,
** 入口参数: array需要判断的数组1首地址,array1需要判断的数组2首地址,len需要判断的长度
** 出口参数: 无
** 返回参数: 0相等,1不相等;
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned char Public_CheckArrayValIsEqual(const unsigned char *array,const unsigned char *array1,unsigned short len)
{
    unsigned short i;
    for(i =0; i <len; i++)
    {
        if(array[i]!=array1[i])return 1;
    }
    return 0;
}

/*************************************************************
** 函数名称: Public_PlayTTSVoiceAlarmStr
** 功能描述: 分段播放语音,报警信息
** 入口参数: 需要播放的语音首地址
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
/* dxl,2014.8.8 按照黎工提交修改
void Public_PlayTTSVoiceAlarmStr(const u8 *StrPtr)
{
    if(Modem_Api_Tts_IsSpt())//dxl,2014.6.11增加内置tts时修改
    {
        Modem_Api_Tts_Play(0,(u8 *)StrPtr,0);
    }
    else
    {
        PlayMulTTSVoiceAlarmStr((u8 *)StrPtr);
    }
}
*/
void Public_PlayTTSVoiceAlarmStr(const u8 *StrPtr)
{
    PlayMulTTSVoiceAlarmStr((u8 *)StrPtr);
}
/*************************************************************
** 函数名称: Public_PlayE2ParamAlarmVoice
** 功能描述: 播放保存在E2参数中的报警信息语音(存储的语音不超过50个字节)
** 入口参数: 参数ID
** 出口参数: 
** 返回参数: 0:语音读取出错,1语音读取正确
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Public_PlayE2ParamAlarmVoice(unsigned short ParamID)
{
    unsigned char len;
    unsigned char buffer[51];
    len=EepromPram_ReadPram(ParamID, buffer);
    if(len)
    {
        buffer[len]=0;
        Public_PlayTTSVoiceAlarmStr(buffer);
        return 1;
    }
    return 0;
}
/*************************************************************
** 函数名称: Public_PlayTTSVoiceStr
** 功能描述: 分段播放语音,普通信息
** 入口参数: 需要播放的语音首地址
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
/* dxl,2014.8.8按照黎工提交的修改
void Public_PlayTTSVoiceStr(const u8 *StrPtr)
{
    if(Modem_Api_Tts_IsSpt())//dxl,2014.6.11增加内置tts时修改
    {
        Modem_Api_Tts_Play(0,(u8 *)StrPtr,0);
    }
    else
    {
        PlayMulTTSVoiceStr((u8 *)StrPtr);
    }
}
*/
void Public_PlayTTSVoiceStr(const u8 *StrPtr)
{
    PlayMulTTSVoiceStr((u8 *)StrPtr);
}
/*************************************************************
** 函数名称: Public_WriteDataToFRAM
** 功能描述: 往铁电里写数据,在数据某尾加入校验字节
** 入口参数: address:铁电地址;data:数据首地址;datalen:写入的字节数
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: FRAM_BufferWrite
*************************************************************/
void Public_WriteDataToFRAM(unsigned short address,const unsigned char *data, unsigned short datalen)
{
    FRAM_BufferWrite(address,(unsigned char *)data,datalen);
}
/*************************************************************
** 函数名称: Public_ReadDataFromFRAM
** 功能描述: 从铁电里读数据
** 入口参数: address:铁电地址;data:目标数据首地址;datalen:读出的字节数
** 出口参数: data
** 返回参数: 实际读出的字节数
** 全局变量: 无
** 调用模块: FRAM_BufferRead
*************************************************************/
unsigned short Public_ReadDataFromFRAM(unsigned short address,unsigned char *data, unsigned short datalen)
{
    return FRAM_BufferRead(data,datalen,address);
}
/*************************************************************OK
** 函数名称: Public_GetCurBasicPositionInfo
** 功能描述: 获得当前基本位置信息(上报格式)
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: Report_GetBasicPositionInformation
*************************************************************/
void Public_GetCurBasicPositionInfo(unsigned char *data)
{
    Report_GetPositionBasicInfo(data); //加载位置信息
}
/*************************************************************
** 函数名称: Public_GetCurTotalMile
** 功能描述: 获取总里程(已考虑了速度类型，不需再判断速度类型了)
** 入口参数: 
** 出口参数: 
** 返回参数: 单位是0.1公里
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned long Public_GetCurTotalMile(void)
{
    unsigned long CurrentMile;
    if(SpeedMonitor_GetCurSpeedType())
    {
        CurrentMile = MileMeter_GetTotalMile();        
    }
    else
    {
        CurrentMile = Pulse_GetTotalMile();
    }
    CurrentMile /=10;
    return CurrentMile;
}

/*************************************************************
** 函数名称: Public_Mymemcpy
** 功能描述: src的数据拷到des中,n为拷贝长度,flag为方向(1:倒序,0:顺序)
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void *Public_Mymemcpy(void *des,  const void *src, unsigned short len, unsigned char flag)
{
    //copy char src[len] to des[len] in any order //
    char *su1 = (char *)des;
    const char *su2 = (const char *)src;
    unsigned short i;
    IWDG_ReloadCounter();//喂狗
    if(flag==0)
    {
        for (; 0 < len; ++su1, ++su2, --len)
            *su1 = *su2;
    }
    else
    {
        for(i=0 ;i<len; i++)
        {
            *(su1+len-1-i) = *(su2+i);
        }	
    }
    IWDG_ReloadCounter();//喂狗
    return (des);
}
/*************************************************************
** 函数名称: Public_GetCurPositionInfoDataBlock
** 功能描述: 取得当前位置信息(纬度、经度、高度:表A.20 位置信息数据块格式)
** 入口参数: 返回数据的首地址
** 出口参数: 返回数据的首地址
** 返回参数: 数据长度
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Public_GetCurPositionInfoDataBlock(unsigned char *buffer)
{
    unsigned char len;
    signed long slVal;
    GPS_STRUCT stTmpGps;
    Gps_CopygPosition(&stTmpGps);
    //////////////////////////////
    len = 0;
    //////////////////////////
    slVal=stTmpGps.Longitue_D*600000ul+stTmpGps.Longitue_F*10000ul+stTmpGps.Longitue_FX;
    if(!stTmpGps.East)slVal *= -1;
    buffer[len++] = slVal>>24;
    buffer[len++] = slVal>>16;
    buffer[len++] = slVal>>8;
    buffer[len++] = slVal;
    /////////////////
    slVal=stTmpGps.Latitue_D*600000ul+stTmpGps.Latitue_F*10000ul+stTmpGps.Latitue_FX;
    if(!stTmpGps.North)slVal *= -1;   
    buffer[len++] = slVal>>24;
    buffer[len++] = slVal>>16;
    buffer[len++] = slVal>>8;
    buffer[len++] = slVal;
    
    buffer[len++] = stTmpGps.High>>8;
    buffer[len++] = stTmpGps.High; 
    return len;
}
/*************************************************************OK
** 函数名称: Public_ShowTextInfo
** 功能描述: 即时显示文本信息(显示多行字,清屏),定时返回显示之前的界面 
** 入口参数: msg字符串首地址,time:显示时间
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: LcdShowMsgEx
*************************************************************/
void Public_ShowTextInfo(char *msg,unsigned long timeVal)
{
    #ifdef NEW_LCD_DRIVER_GDI
    LcdShowMsgEx(msg,timeVal);
    #else
    #ifdef USE_NAVILCD
    NaviLcdShowTempText((unsigned char*)msg);
    #endif
    #endif
}
/*************************************************************
** 函数名称: Public_ShowSaveDataProBar
** 功能描述: 显示存储进度
** 入口参数: 百分比(从0开始,1.0结束显示,取值:0.0~1.0)
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Public_ShowSaveDataProBar(float persent)
{
    #ifdef NEW_LCD_DRIVER_GDI
    char buffer[20];
    unsigned char val;
    static unsigned char x0,x;
    if(persent==0.0)
    {
        x0=x=12;
        LcdCopyScreen();
        LcdClearScreen();
        LcdShowCaption("数据转存中", 2);
        LcdShowWindow(10, 22, 115, 31);
        LcdShowWindow(12, 24, 112, 29);
        LcdShowStr(48, 36, "  0%", 0);
        return;
    }
    ////////////////////////////
    val =(unsigned char)(persent*100.0);
    if(val > 100)val=100;
    x= 12 + val;
    LcdShowHorLine(x0+1, x+1, 26, 0);  
    LcdShowHorLine(x0+1, x+1, 27, 0);    
    x0=x;
    sprintf(buffer,"%3d",val);    
    LcdShowStr(48, 36, buffer, 0);
    if(val>=100)LcdPasteScreen();
    #endif
}

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
void Public_SetCarLoadStatus(unsigned char status)
{
    if(0==status)
    {
        Io_WriteStatusBit(STATUS_BIT_FULL_LOAD1,RESET);
        Io_WriteStatusBit(STATUS_BIT_FULL_LOAD2,RESET);
    }
    else
    if(1==status)
    {
        Io_WriteStatusBit(STATUS_BIT_FULL_LOAD1,SET);
        Io_WriteStatusBit(STATUS_BIT_FULL_LOAD2,RESET);
    }
    else
    if(2==status)
    {
        Io_WriteStatusBit(STATUS_BIT_FULL_LOAD1,SET);
        Io_WriteStatusBit(STATUS_BIT_FULL_LOAD2,SET);
    }
    //SetEvTask(EV_REPORT);
    Report_UploadPositionInfo(CHANNEL_DATA_1);
    Report_UploadPositionInfo(CHANNEL_DATA_2);
}
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
void Public_QuickOpenAccount(void)
{
    ///请在这里添加快速开户实际发送函数/////////    
    #ifdef USE_QUICK_OPEN_ACCOUNT
    OpenAccount_CheckAndDisposeLockStatus();
    #endif
}
/*************************************************************
** 函数名称: Public_ConvertTimeToLong
** 功能描述: 把时间结构体根据控制域转成long型
** 入口参数: ctrl:周期控制域(详见索美格式),stTime需要转换的时间结构体
** 出口参数: 
** 返回参数: 转换结果(0:周期控制域为0,不起作用;非0,起作用) 
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned long Public_ConvertTimeToLong(unsigned short ctrl,TIME_T stTime)
{
    unsigned long Temp=0;
    ///////////////////////
    if(0==ctrl)return 0;////
    ///////////////////////////
    if(PUBLIC_CHECKBIT(ctrl, 0))//日期时间范围
    {
        Temp = stTime.year; //获得年
        Temp = Temp*12 + stTime.month; //获得月
        Temp = Temp*31 + stTime.day; //获得日
        Temp = Temp*24 + stTime.hour; //获得时
        Temp = Temp*60 + stTime.min; //获得分
        Temp = Temp*60 + stTime.sec; //获得秒
    }
    else
    if(PUBLIC_CHECKBIT(ctrl, 1))//日期范围
    {
        Temp = stTime.year; //获得年
        Temp = Temp*12 + stTime.month; //获得月
        Temp = Temp*31 + stTime.day; //获得日
    }
    else
    if(PUBLIC_CHECKBIT(ctrl, 2))//时间范围
    {
        Temp = Temp*24 + stTime.hour; //获得时
        Temp = Temp*60 + stTime.min; //获得分
        Temp = Temp*60 + stTime.sec; //获得秒
    }
    else
    if(PUBLIC_CHECKBIT(ctrl, 3))//日月范围
    {
        Temp = Temp*12 + stTime.month; //获得月
        Temp = Temp*31 + stTime.day; //获得日
    }
    else
    if(PUBLIC_CHECKBIT(ctrl, 4))//保留
    {
    }
    else
    if(PUBLIC_CHECKBIT(ctrl, 5))//日
    {
        Temp = Temp*31 + stTime.day; //获得日
    }
    else
    if(PUBLIC_CHECKBIT(ctrl, 6))//月
    {
        Temp = Temp*12 + stTime.month; //获得月
    }
    else
    if(PUBLIC_CHECKBIT(ctrl, 7))//年
    {
        Temp = stTime.year; //获得年
    }
    /////////////////
    Temp++;//保证返回结果不再为0
    ///////////////////
    return Temp;
}
/*************************************************************
** 函数名称: Public_ConvertBCDTimeToLong
** 功能描述: 把BCD码(6位)根据控制域转成long型
** 入口参数: ctrl:周期控制域(详见索美格式),data需要转换的BCD码首地址
** 出口参数: 
** 返回参数: 0:表示转换出错或不需要转换,非0:正常的转换结果 
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned long Public_ConvertBCDTimeToLong(unsigned short ctrl,unsigned char *data)
{
    TIME_T stTime;
    if(0==ctrl)return 0;
    Public_ConvertBCDToTimeEx(&stTime,data);
    return Public_ConvertTimeToLong(ctrl,stTime);//返回结果非0
}
/*************************************************************
** 函数名称: Public_CheckTimeRangeIsValid
** 功能描述: 判断指定时间范围是否有效
** 入口参数: ctrl:周期控制域(详见索美格式),startTime范围开始时间点(经过Public_ConvertTimeToLong转换的值),endTime范围结束时间点(经过Public_ConvertTimeToLong转换的值)
** 出口参数: 
** 返回参数: 1:有效，0:无效
** 全局变量: 无
** 调用模块: 
*************************************************************/
unsigned char Public_CheckTimeRangeIsValid(unsigned short ctrl,unsigned long startTime,unsigned long endTime)
{
    if((0==startTime)||(0==endTime))return 0;
    if(startTime==endTime)
    {
        if(PUBLIC_CHECKBIT(ctrl, 0)||PUBLIC_CHECKBIT(ctrl, 2))//日期时间范围
        return 0;
    }
    return 1;
}
/*************************************************************
** 函数名称: Public_ConvertWeekDay
** 功能描述: 转换星期
** 入口参数: flag转换标志,0:某星期有效时,夜间时间段范围为某星期早上和晚上,分两个时间段,不连续
                          1:某星期有效时,夜间时间段范围为某星期晚上及第二天的早上,为一个连续的时间段
             week星期,startTime范围开始时间点(经过Public_ConvertTimeToLong转换的值),endTime范围结束时间点(经过Public_ConvertTimeToLong转换的值)
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 
*************************************************************/
unsigned char Public_ConvertWeekDay(unsigned char flag,unsigned char week,TIME_T stTime,unsigned long startTime,unsigned long endTime)
{
    unsigned char result;
    result=week;
    /////根据标志采用哪一种方案/////////////
    if(0==flag)return result;////某星期有效时,夜间时间段范围为某星期早上和晚上,分两个时间段,不连续
    /////////////////
    if(startTime>endTime&&stTime.hour<12)//某星期有效时,夜间时间段范围为某星期晚上及第二天的早上,为一个连续的时间段
    {
        if(1==result)
        {
            result=7;
        }
        else
        {
           result=week-1;
        }
    }
    return result;
}
/*************************************************************
** 函数名称: Public_CheckTimeIsInRange
** 功能描述: 判断指定时间是否在指定范围内
** 入口参数: curTime指定时间,startTime开始时间,endTime结束时间
** 出口参数: 
** 返回参数: 1:在范围内，0:不在范围内 
** 全局变量: 无
** 调用模块: 
*************************************************************/
unsigned char Public_CheckTimeIsInRange(unsigned long curTime,unsigned long startTime,unsigned long endTime)
{
    if(startTime<endTime)//小于
    {
        if(startTime<=curTime&&curTime<=endTime)return 1;
    }
    else//大于
    if(startTime>endTime)//
    {
        if(startTime<=curTime||curTime<=endTime)return 1;
    }
    else
    if(startTime==endTime)//等于
    {
        if(startTime==curTime)return 1;
    }
    return 0;
}
/*************************************************************
** 函数名称: Public_CheckCurTimeIsInTimeRange
** 功能描述: 判断当前时间是否在指定时间范围内
** 入口参数: ctrl:周期控制域(详见索美格式),startTime范围开始时间点(经过Public_ConvertTimeToLong转换的值),endTime范围结束时间点(经过Public_ConvertTimeToLong转换的值)
** 出口参数: 
** 返回参数: 1:在范围内，0:不在范围内 
** 全局变量: 无
** 调用模块: Public_ConvertTimeToLong,RTC_GetCurWeekDay
*************************************************************/
unsigned char Public_CheckCurTimeIsInTimeRange(unsigned short ctrl,unsigned long startTime,unsigned long endTime)
{
    unsigned char temp,week;
    unsigned long curTime;
    TIME_T stTime;
    if(!Public_CheckTimeRangeIsValid(ctrl,startTime,endTime))return 0;
    RTC_GetCurTime(&stTime);
    temp=(ctrl & 0xff00)>>8;
    if(temp)//需要检查星期
    {
        week = RTC_GetCurWeekDay();
        week = Public_ConvertWeekDay(0,week,stTime,startTime,endTime);
        if(!PUBLIC_CHECKBIT(temp, week-1))return 0;//
    }
    ////////////////////////////    
    curTime=Public_ConvertTimeToLong(ctrl,stTime);
    return Public_CheckTimeIsInRange(curTime,startTime,endTime);
}
/*************************************************************
** 函数名称: Public_CheckTimeIsInTimeRange
** 功能描述: 判断指定时间是否在指定时间范围内
** 入口参数: ctrl:周期控制域(详见索美格式),checkTime需要判断的时间点(四个字节的时间),startTime(经过Public_ConvertTimeToLong转换的值),endTime范围结束时间点(经过Public_ConvertTimeToLong转换的值)
** 返回参数: 1:在范围内，0:不在范围内 
** 全局变量: 无
** 调用模块: Public_ConvertTimeToLong,RTC_CacWeekDay
*************************************************************/
unsigned char Public_CheckTimeIsInTimeRange(unsigned short ctrl,unsigned long checkTime,unsigned long startTime,unsigned long endTime)
{
    unsigned char temp,week;
    unsigned long curTime;
    TIME_T stTime;    
    if(!Public_CheckTimeRangeIsValid(ctrl,startTime,endTime))return 0;
    //////////////////////////
    Gmtime(&stTime,checkTime);
    /////////////////////////
    temp=(ctrl & 0xff00)>>8;
    if(temp)//需要检查星期
    {
        week = RTC_CacWeekDay(stTime.year,stTime.month,stTime.day);
        week = Public_ConvertWeekDay(0,week,stTime,startTime,endTime);
        if(!PUBLIC_CHECKBIT(temp, week-1))return 0;//
    }
    ////////////////////////////
    curTime=Public_ConvertTimeToLong(ctrl,stTime);
    /////////////////////////
    return Public_CheckTimeIsInRange(curTime,startTime,endTime);
}
/*************************************************************
** 函数名称: Public_CheckCurTimeIsInEarlyAlarmTimeRange
** 功能描述: 判断当前时间是否在指定时间的提前范围内
** 入口参数: ctrl:周期控制域(详见索美格式),earlyTime提前时间,startTime范围开始时间点(经过Public_ConvertTimeToLong转换的值),endTime范围结束时间点(经过Public_ConvertTimeToLong转换的值)
** 出口参数: 
** 返回参数: 1:在范围内，0:不在范围内 
** 全局变量: 无
** 调用模块: Public_CheckTimeIsInTimeRange
*************************************************************/
unsigned char Public_CheckCurTimeIsInEarlyAlarmTimeRange(unsigned short ctrl,unsigned long earlyTime,unsigned long startTime,unsigned long endTime)
{
    unsigned long checktime;
    checktime=RTC_GetCounter();
    checktime += earlyTime;
    return Public_CheckTimeIsInTimeRange(ctrl,checktime,startTime,endTime);
}
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
void Public_ShowValue(unsigned char x,unsigned char y,char *format ,unsigned long value)
{
    #ifdef NEW_LCD_DRIVER_GDI
    char buffer[30];
    sprintf(buffer,format,value);
    LcdShowStr(x,y, buffer, 0);
    #endif
}
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
void Public_LcdShowTime(unsigned char y,TIME_T stTime)
{
    #ifdef NEW_LCD_DRIVER_GDI
    char buffer[20];
    sprintf(buffer,"%02d-",stTime.year);
    LcdShowStr(0,y,buffer,0);
    sprintf(buffer,"%02d-",stTime.month);
    LcdShowStr(23,y,buffer,0);
    sprintf(buffer,"%02d",stTime.day);
    LcdShowStr(46,y,buffer,0);
    sprintf(buffer,"%02d:%02d:%02d",stTime.hour,stTime.min,stTime.sec);
    LcdShowStr(67,y,buffer,0);
    #endif
}
/*************************************************************
** 函数名称: Public_CheckIsPhoneServerRunStatus
** 功能描述: 判断是否为电话服务的行驶状态
** 入口参数: 
** 出口参数: 
** 返回参数: 1为电话服务的行驶状态,0:非电话服务的行驶状态
** 全局变量: 
** 调用模块: 
*************************************************************/
unsigned char Public_CheckIsPhoneServerRunStatus(void)
{
    #ifdef E2_RUN_FORBID_CALL_SWITCH_ID 
    unsigned char flag;
    if(EepromPram_ReadPram(E2_RUN_FORBID_CALL_SWITCH_ID,&flag))
    {
        if(1==flag)
        {
            if((Gps_ReadSpeed()>5)||(Pulse_GetSpeed()>5))
            {
                return 1;//
            }
        }
    }
    #else
    #if (MOD_TYPE == CDMA_MODULE_MC8331) 
    if((Gps_ReadSpeed()>5)||(Pulse_GetSpeed()>5))
    {
        return 1;//
    }
    #endif
    #endif
    return 0;
}
/*************************************************************
** 函数名称: Public_CheckPhoneIsSpecialNumber
** 功能描述: 判断是否为特殊号码
** 入口参数: 
** 出口参数: 
** 返回参数: 0:非特殊号码,非0:为特殊号码,
** 全局变量: 
** 调用模块: 
*************************************************************/
unsigned char Public_CheckPhoneIsSpecialNumber(unsigned char *phone)
{
    unsigned char buffer[30];
    unsigned char len;
    if(!strlen((char *)phone))return 0;
    len = EepromPram_ReadPram(E2_MONITOR_SERVER_PHONE_ID,buffer);
    if(len)
    {
        buffer[len]='\0';
        if(!strcmp((char *)phone,(char *)buffer))
        {
            return 1;//呼入/呼出
        }
    }
    /////监听电话号码////////////
    len = EepromPram_ReadPram(E2_MONITOR_PHONE_ID,buffer);
    if(len)
    {
        buffer[len]='\0';
        if(!strcmp((char *)phone,(char *)buffer))
        {
            return 2;//呼入/呼出
        }
    }
    /////////////////////
    len = EepromPram_ReadPram(E2_TERMINAL_RESET_PHONE_ID,buffer);//终端复位电话号码
    if(len)
    {
        buffer[len]='\0';
        if(!strcmp((char *)phone,(char *)buffer))
        {
            return 3;
        }
    }
    len = EepromPram_ReadPram(E2_TERMINAL_DEFAULT_SET_PHONE_ID,buffer);//恢复出厂设置电话号码
    if(len)
    {
        buffer[len]='\0';
        if(!strcmp((char *)phone,(char *)buffer))
        {
            return 4;
        }
    }
    return 0;
}
/*************************************************************
** 函数名称: Public_CheckPhoneInformation
** 功能描述: 检查电话号码,已根据通话时长判断电话簿以外的号码,(监听电话号码、热线电话号码不受任何限制)
** 入口参数: phone：需要检查的电话号码首地址； 
** 出口参数: name：返回的联系人姓名
** 返回参数: 0:禁止呼入/呼出；1: 呼入；2: 呼出；3：呼入/呼出 
** 全局变量: 无
** 调用模块: CheckPhoneInformation,
*************************************************************/
unsigned char Public_CheckPhoneInformation(unsigned char *phone,unsigned char *name)
{
    unsigned char flag;
    flag=Public_CheckPhoneIsSpecialNumber(phone);
    #ifdef USE_PHONE
    Phone_SetPhoneSpecialFlag(flag);
    #endif
    if(1 == flag)//监控平台电话号码
    {
        strcpy((char *)name,"客服中心");
        return 3;//呼入/呼出
    }
    else
    if(2 == flag)//监听电话
    {
        strcpy((char *)name,"监听电话");
        return 3;//呼入/呼出
    }
    else
    if(3 == flag)//是终端复位电话号码
    {
        strcpy((char *)name,"复位电话");
        if(s_ucNewCallPhoneFlag)
        Public_SetOperateTimer(NVIC_SystemReset,PUBLICSECS(3));
        return 3;//呼入/呼出
    }
    else
    if(4 == flag)//恢复出厂设置电话号码
    {
        strcpy((char *)name,"恢复出厂");
        if(s_ucNewCallPhoneFlag)
        Public_SetOperateTimer(EepromPram_DefaultSet,PUBLICSECS(2));
        return 3;//呼入/呼出
    }
    ///////检查是否属于行驶中禁止通话////////////
    if(Public_CheckIsPhoneServerRunStatus())
    {
        return 0;//
    }
    //////检查通话时长///////////////////
    #ifdef USE_PHONE 
    /////////每次通话时长////////////
    if(0==Phone_GetEachMaxTalkTime())
    {
        return 0;//禁止呼入/呼出
    }
    ////////////每月通话时长/////////////// 
    if(Phone_GetOverMonthTalkTimeFlag())//获得超过当月通话时间标志
    {
        return 0;//禁止呼入/呼出
    }
    #else
    unsigned char buffer[30];
    /////////每次通话时长////////////
    if(EepromPram_ReadPram(E2_MAX_PHONE_TIME_EACH_ID,buffer))
    {
        if(0==Public_ConvertBufferToLong(buffer))
        {
            return 0;//禁止呼入/呼出
        }
    }
    ////////////每月通话时长/////////////// 
    if(EepromPram_ReadPram(E2_MAX_PHONE_TIME_MONTH_ID,buffer))
    {
        if(0==Public_ConvertBufferToLong(buffer))
        {
            return 0;//禁止呼入/呼出
        }
    }
    #endif
    ///////在没有电话簿的情况下允许通话/////////////////
    if(0==Message_GetPhoneBookTotalNum())
    {
        flag = 3;
        strcpy((char *)name,"未知");
        return flag;//呼入/呼出 
    }
    ////////由电话簿属性决定////////////////
    return CheckPhoneInformation(phone,name);//0：没有该电话号码的资料；1: 呼入；2: 呼出；3：呼入/呼出
}
/*************************************************************
** 函数名称: Public_DisposePhoneStatus
** 功能描述: 处理电话状态命令
** 入口参数: phone:电话号码,status:电话状态：0：为挂断;1:接通;2:新来电;
** 出口参数: 无
** 返回参数: 如果是电话呼入状态为新来电时,返回值:0:直接拒接来电;1:正在等待用户接听来电
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Public_DisposePhoneStatus(unsigned char *phone,unsigned char status)
{
    unsigned char result;
    result = 0;
    s_ucNewCallPhoneFlag = 0;
    if(2 == status)s_ucNewCallPhoneFlag = 1;
    #ifdef USE_PHONE_BOOK
    #ifdef USE_NAVILCD
    if(NaviLcd_GetCurOnlineStatus())
    result = NaviLcd_DisposePhoneStatus(phone,status);
    #endif
    result = PhoneBook_DisposePhoneStatus(phone,status);
    #else
    #ifdef USE_NAVILCD
    result = NaviLcd_DisposePhoneStatus(phone,status); 
    #endif 
    #endif
    #ifdef USE_PHONE
    Phone_DisposePhoneStatus(phone,status);
    #endif
    s_ucNewCallPhoneFlag = 0;
    return result;
}
/*************************************************************
** 函数名称: ShowStartLogo
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void Public_SaveCurSIMICcidNumberToE2Param(void)
{
    unsigned char data[20]={0}; 
    MOD_GetIccidBcdValue(data);
    EepromPram_WritePram(E2_SIM_ICCID_NUMBER_ID,data,E2_SIM_ICCID_NUMBER_ID_LEN);
}
/*************************************************************
** 函数名称: Public_CheckSystemLogoAndVerSion
** 功能描述: 检验终端产品名称、产品版本号是否正确(2字节公司标志,8字节型号,2字节PCB版本,6字节版本号,4字节校验码)
** 入口参数: 附加信息首地址,crc
** 出口参数: 无
** 返回参数:  1:检验正确,0:检验出错
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned char Public_CheckSystemLogoAndVerSion(unsigned char *pBuffer,unsigned long *checkCRC)
{
    char readLogoModel[11];  
    char readVersion[7]; 
    unsigned char len;
    ///////////2字节公司标志,8字节型号,2字节PCB版本,6字节版本号,4字节校验码 
    ///////////////////
    memcpy(readLogoModel,pBuffer,10);//EIEGS702
    memcpy(readVersion,pBuffer+12,6);//021101  
    /////////////
    *checkCRC = Public_ConvertBufferToLong(&pBuffer[18]); 
    /////////////////////////////////
    #ifdef __VERSION_H_//EGS702
    char curLogoModel[11];  
    char curVersion[7];    
    ///////////////////
    len=Version_GetEquipmentCompany(curLogoModel);
    Version_GetProductSeriesName(&curLogoModel[len]);
    Version_GetEquipmentSoftVer(curVersion);
    //比较公司LOGO和产品型号
    if(0 != strncmp(readLogoModel,curLogoModel,8))
    {
        return 0;//        
    }
    //////////////////
    if(0 != strncmp(readVersion,curVersion,4))
    {
        if(0 != strncmp(readVersion,"0220",4))return 0;
    }
    #else//EGS701
    if(0 != strncmp(readLogoModel+2,PRODUCT_MODEL+14,6))
    {
        return 0;//        
    }
    #endif
    ////////////////////
    //写升级类型,dxl,2014.6.14
    #ifdef FRAM_FIRMWARE_UPDATA_TYPE_ADDR
    len = 0;
    FRAM_BufferWrite(FRAM_FIRMWARE_UPDATA_TYPE_ADDR, &len, FRAM_FIRMWARE_UPDATA_TYPE_LEN);
    #endif
    return 1;
}
/*************************************************************
** 函数名称: Public_CheckPeripheralUpdataType
** 功能描述: 检验外设产品名称是否正确(2字节公司标志,8字节型号,2字节PCB版本,6字节版本号,4字节校验码
** 入口参数: 附加信息首地址,crc
** 出口参数: 无
** 返回参数: 1:检验正确,0:检验出错
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned char Public_CheckPeripheralUpdataType(unsigned char *pBuffer,unsigned char *type)
{
    #ifndef __VERSION_H_
    if(0 == strncmp((char const *)pBuffer+2,"EICZ    ",4))//检测产品型号dxl,2014.6.14
    {
        *type = 0x09;
        #ifdef FRAM_FIRMWARE_UPDATA_TYPE_ADDR
        FRAM_BufferWrite(FRAM_FIRMWARE_UPDATA_TYPE_ADDR, type, FRAM_FIRMWARE_UPDATA_TYPE_LEN);
        #endif
        return 1;
    }
    #endif    
    return 0;
}
/*************************************************************
** 函数名称: Public_CheckTerminalAndPeripheralVerify
** 功能描述: 检验终端、外设产品名称、产品版本号是否正确(2字节公司标志,8字节型号,2字节PCB版本,6字节版本号,4字节校验码)
** 入口参数: 附加信息首地址,checkCRC返回值,type升级类型返回值
** 出口参数: 无
** 返回参数: 1:检验正确,0:检验出错
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned char Public_CheckTerminalAndPeripheralVerify(unsigned char *pBuffer,unsigned long *checkCRC,unsigned char *type)
{    
    *type = 0;
    ///////是否是终端升级///////////////////
    if(Public_CheckSystemLogoAndVerSion(pBuffer,checkCRC))
    {
        return 1;
    }
    ///////是否是外设升级///////////////////
    if(Public_CheckPeripheralUpdataType(pBuffer,type))
    {
        return 1;
    }
    ///////////////////
    return 0;
}
/*************************************************************
** 函数名称: Public_ParameterInitialize
** 功能描述: 参数初始化
** 入口参数: 
** 出口参数: 无
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Public_ParameterInitialize(void)
{
    LZM_PublicKillTimerAll(s_stPUBLICTimer,PUBLIC_TIMERS_MAX);  
    ///////////////////////
    SetTimerTask(TIME_PUBLIC,LZM_TIME_BASE);
    ////////////////////////
    //Public_SetOperateTimer(Public_SaveCurSIMICcidNumberToE2Param,PUBLICSECS(50));
    //////////////////////////  
    #ifdef USE_PHONE
    Phone_ParamInitialize();
    #endif
    ///////////////////////
}
/************************************************************
** 函数名称: Public_SetOperateTimer
** 功能描述: 
** 入口参数:
** 出口参数:
************************************************************/ 
void Public_SetOperateTimer(void (*function)(void),LZM_TIMER32 time)
{
    LZM_PublicSetOnceTimer(&s_stPUBLICTimer[PUBLIC_TIMER_OPERATE],time,function);
}
/************************************************************
** 函数名称: Public_SetEIExpandTimer
** 功能描述: 
** 入口参数:
** 出口参数:
************************************************************/ 
void Public_SetEIExpandTimer(void (*function)(void),LZM_TIMER32 time)
{
    LZM_PublicSetOnceTimer(&s_stPUBLICTimer[PUBLIC_TIMER_EIEXPAND],time,function);
}
/************************************************************
** 函数名称: Public_SetTestTimer
** 功能描述: 
** 入口参数:
** 出口参数:
************************************************************/ 
void Public_SetTestTimer(void (*function)(void),LZM_TIMER32 time)
{
    LZM_PublicSetOnceTimer(&s_stPUBLICTimer[PUBLIC_TIMER_TEST],time,function);
}

/************************************************************
** 函数名称: Public_SetTaskTimer
** 功能描述: 
** 入口参数:
** 出口参数:
************************************************************/ 
void Public_SetTaskTimer(void (*function)(void),LZM_TIMER32 time)
{
    LZM_PublicSetOnceTimer(&s_stPUBLICTimer[PUBLIC_TIMER_TASK],time,function);
}
/*************************************************************
** 函数名称: LZM_PublicTimerTask
** 功能描述: 公共定时器处理函数(对时间要求不是很高的且执行占用的时间不会很长)
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
FunctionalState LZM_PublicTimerTask(void)
{
    LZM_PublicTimerHandler(s_stPUBLICTimer,PUBLIC_TIMERS_MAX);
    #ifdef USE_PHONE
    Phone_TimeTask();
    #endif
    return ENABLE;
}

/********************************************************************************
 *                            End of File
 *******************************************************************************/

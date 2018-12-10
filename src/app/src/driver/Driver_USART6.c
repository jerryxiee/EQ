/*******************************************************************************
 * File Name:			CommICCard.c 
 * Function Describe:	
 * Relate Module:		测试相关协议。
 * Writer:				Joneming
 * Date:				2011-09-08
 * ReWriter:			
 * Date:				
 *******************************************************************************/
#include "include.h"

u8	SwipCardCh = 0;//刷卡通道  0 二汽读卡器  1  伊爱IC卡
#if 0
#define COMICCARD_DEFAULT_BAUD  4800 //IC卡读卡器模块晶振为12M时采用,无锡送检样机使用
#else
#define COMICCARD_DEFAULT_BAUD  9600  //IC卡读卡器模块晶振为11.0596时采用
#endif
///////////////////////
#define PARAM_COM               COM6//参数使用的串口
//////////////////////////
#define COMICCARD_MAX_REC_LEN    256 //接收数据的长度
#define COMICCARD_MAX_SEND_LEN   COM6_MAX_LEN_TX //发送数据的长度
///////////////////////
static unsigned char s_ucCommrecvBuff[COMICCARD_MAX_REC_LEN+1];     //解析缓冲
static unsigned char s_ucCommresendBuff[COMICCARD_MAX_SEND_LEN+1];  //发送缓冲
/////////////////////////
static ST_COMM s_stCommICCard;
//////////////////////////
enum 
{
    CPARAM_TIMER_RESEND, //    
    CPARAM_TIMERS_MAX
}E_COM_PARAMTIME;
static LZM_TIMER s_stCParamTimer[CPARAM_TIMERS_MAX];
/****************************************************/
/*                                                  */
/* Local Definitions and macros                     */
/*                                                  */
/****************************************************/
/*************************************************************
** 函数名称: CommICCard_StartSendData
** 功能描述: 发送数据
** 入口参数: pBuffer源数据,BufferLen数据长度
** 出口参数: 无
** 返回参数: 0:成功；1:失败,0xff:超出缓存长度
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char CommICCard_StartSendData(unsigned char *pBuffer,unsigned short datalen)
{
#if (ICCARD_JTD == ICCARD_SEL)
	{
		return COMM_SendData(COM2,pBuffer,datalen);
	}
#else
	{
		return COMM_SendData(PARAM_COM,pBuffer,datalen);
	}
#endif
}
/************************************************************
** 函数名称: CommICCard_ResendData
** 功能描述: 重发时间到，执行重发处理
             
** 入口参数:
** 出口参数:
************************************************************/ 
void CommICCard_ResendData(void)
{
    if(0 == CommICCard_StartSendData(s_stCommICCard.resendBuff,s_stCommICCard.resendlen))//发送成功
    {
        s_stCommICCard.resendCnt = 0;
        return;
    }
    /////////////////////////
    if(s_stCommICCard.resendCnt)//开启重发
    {
        s_stCommICCard.resendCnt--;
        LZM_PublicSetOnceTimer(&s_stCParamTimer[CPARAM_TIMER_RESEND], PUBLICSECS(0.15), CommICCard_ResendData); 
    }
    
}
/*************************************************************
** 函数名称: CommICCard_SendData
** 功能描述: 发送数据
** 入口参数: pBuffer源数据,BufferLen数据长度
** 出口参数: 无
** 返回参数: 0:成功；1:等待重发,2:失败,0xff:超出缓存长度
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char CommICCard_SendData(unsigned char *pBuffer,unsigned short datalen)
{
    if(datalen>COMICCARD_MAX_SEND_LEN)return 0xff;//超过缓存,直接返回
    //////////////////
    if(0 == CommICCard_StartSendData(pBuffer,datalen))return 0;//发送成功
    //////失败//////////////////
    if(0==s_stCommICCard.resendCnt)
    {
        s_stCommICCard.resendlen = datalen;
        memcpy(&s_stCommICCard.resendBuff,pBuffer,s_stCommICCard.resendlen);
        s_stCommICCard.resendCnt = 4;//重发送次数
        LZM_PublicSetOnceTimer(&s_stCParamTimer[CPARAM_TIMER_RESEND], PUBLICSECS(0.15), CommICCard_ResendData); 
        return 1;//等待重发
    }
    else
    if((s_stCommICCard.resendlen+datalen)<COMICCARD_MAX_SEND_LEN)
    {
        memcpy(&s_stCommICCard.resendBuff[s_stCommICCard.resendlen],pBuffer,datalen);
        s_stCommICCard.resendlen += datalen;
        return 1;//等待重发
    }
    else//
    {
        return 2;//失败
    }
}

/*************************************************************OK
** 函数名称: CommICCard_ProtocolParse
** 功能描述: 协议解释
** 入口参数:  
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void CommICCard_ProtocolParse(unsigned char *pBuffer,unsigned short datalen)
{
	SwipCardCh = 1;
    ICCard_ProtocolParse(pBuffer,datalen);
	SwipCardCh = 0;
}
/*************************************************************OK
** 函数名称: CommICCard_TimeTask
** 功能描述: 串口接收数据处理时间任务接口
** 入口参数:  
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
**  备 注  :50毫秒调度1次
*************************************************************/
void CommICCard_CheckReadData(void)
{
    unsigned short datalen;
    datalen=COMM_ReadData(PARAM_COM,&s_stCommICCard.recvBuff[s_stCommICCard.recvlen],COMICCARD_MAX_REC_LEN-s_stCommICCard.recvlen);
    ///////////////////////////////
    s_stCommICCard.recvlen += datalen;
    ///////////////////////////////////
    if(0 == datalen)////处理的数据长度为0//////////    
    {
        if(s_stCommICCard.recvlen)
        {
            s_stCommICCard.recvfreeCnt++;
            ////////////////////////////////
            if(s_stCommICCard.recvfreeCnt >1)//空闲两次调度
            {
                CommICCard_ProtocolParse(s_stCommICCard.recvBuff,s_stCommICCard.recvlen);
                s_stCommICCard.recvlen       = 0;
                s_stCommICCard.recvfreeCnt   = 0;//
            }
        }
    }
    else//////////////////
    if(s_stCommICCard.recvlen+10>COMICCARD_MAX_REC_LEN)//缓存已满的时候
    {
        CommICCard_ProtocolParse(s_stCommICCard.recvBuff,s_stCommICCard.recvlen);
        s_stCommICCard.recvlen       = 0;
        s_stCommICCard.recvfreeCnt   = 0;
    }
    else//////////////////
    {
        s_stCommICCard.recvfreeCnt = 0;
    }
}
/*************************************************************OK
** 函数名称: CommICCard_TimeTask
** 功能描述: 串口接收数据处理时间任务接口
** 入口参数:  
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
**  备 注  :50毫秒调度1次
*************************************************************/
FunctionalState CommICCard_TimeTask(void)
{
//	#if (ICCARD_JTD == ICCARD_SEL)
//		return DISABLE;
//	#endif
    CommICCard_CheckReadData();
    ////////////////////
    LZM_PublicTimerHandler(s_stCParamTimer,CPARAM_TIMERS_MAX);    
    //////////////////////
    return ENABLE;
}
/*************************************************************OK
** 函数名称: CommICCardInit
** 功能描述: //串口初始化
** 入口参数:  
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void CommICCardInit(void)
{
    LZM_PublicKillTimerAll(s_stCParamTimer,CPARAM_TIMERS_MAX); 
	
    //////////////////
    s_stCommICCard.recvlen       = 0;
    s_stCommICCard.resendlen     = 0;
    s_stCommICCard.resendCnt     = 0;
    s_stCommICCard.recvfreeCnt   = 0;
    s_stCommICCard.recvBuff      = s_ucCommrecvBuff;
    s_stCommICCard.resendBuff    = s_ucCommresendBuff;
    ////////////////////
    COMM_Initialize(PARAM_COM,COMICCARD_DEFAULT_BAUD);
    ////////////////////    
    SetTimerTask(TIME_USART6, LZM_TIME_BASE); 
    ////////////////////////    
}
/*******************************************************************************
 *                             end of module
 *******************************************************************************/


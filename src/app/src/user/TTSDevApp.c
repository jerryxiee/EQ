/*******************************************************
Copyright (C),E-EYE Tech. Co.,Ltd.
Author: WYF
Version:V1.0
Finish Date:2012-6-12
Description:TTS协议解析
*******************************************************/
#include "include.h"

#define S_OFF_SPK        0         //功放控制
#define S_ON_SPK         1
#define S_OFF_TTS_POWER  0         //TTS POWER 控制
#define S_ON_TTS_POWER   1 

#define VOICE_DATA_LEN        512 //能够播放的字节长度
//宏定义
#define TTS_VOICE_SIZE      180                          //每条语音字节数
//结构类型
#pragma pack(1)
//TTS 协议结构
typedef struct
{
    u8 Head;     //0XFD
    u8 Len[2];   //高字节在前，低字节在后
    u8 Cmd;      //指令 0X01:语音合成 0X88:PowerDown 0X22:IDLE 0XFF:WakeUp 
    u8 FontStyle;//汉字编码格式 0X00:GB2313 0X01:GBK 0X02:BIG5 0X03:UNICODE
    u8 VoiceData[TTS_VOICE_SIZE]; //指向字符串的指针
    u16 PageLen; //整条协议总字节数
}ST_TTS_PRO;
#pragma pack()
//////////////////////////////
#define TTS_MAX_SEND_LEN sizeof(ST_TTS_PRO)   //发送语音数据的长度
//////////////////////////////////
//结构类型
typedef struct
{   
    u16 voiceDataLen;   //语音数据总长度
    u16 sendDataLen;    //当前语音数据发送计数
    u8 status;          //TTS播放状态
    u8 curSendLen;      //当前单次发送实际长度
    u8 oneMaxLen;       //单次最大长度
    u8 oneMinLen;       //单次最小长度
    u8 busy;            //TTS忙,在多段语音发送时,TTS被占用的整体忙标志 
    u8 extTtsFlag;      //支持外部Tts
    u8 checkCnt;        //
    u8 checkRDYCnt;     //
    u8 *sendPtr;    //指向移交串口前的待发数据   
}ST_TTS_ATTRIB;
////////////////////////////
static ST_TTS_ATTRIB s_stTtsAttirb;//
static unsigned char s_ucTtsPlayVoiceData[VOICE_DATA_LEN+1]; //语音数据池   
//////////////////
typedef enum{E_VOICE_INIT=0,E_VOICE_LOAD, E_VOICE_PLAY, E_VOICE_END, E_VOICE_MAX}E_TTS_STATUS;

//可选的提示音 ：
//短信提示音   报警提示音
//sounda       sounde
//soundb       soundf
//soundc       soundg
//soundd
#define USE_INFO_SOUND        "soundb"        //用户信息提示音
#define ALARM_INFO_SOUND      "soundc"        //报警信息提示音

//播放类型
enum TTS_PLAY_TYPE
{
    TTS_PLAY_TYPE_USE = 0,                    //普通信息
    TTS_PLAY_TYPE_ALARM,                      //报警信息
};

enum 
{
    TTS_USE_EMPTY = 0,//没检测
    TTS_USE_EXT,    //外部TTS
    TTS_USE_INT,    //内部TTS
    TTS_USE_MAX
};
//变量定义


u8   ttsVolume = 8;                          //音量
u8   ttsValue[9]={0,1,3,5,7,9,11,13,16};       //TTS音量共8级,!-8,0是静音使用。

void IntoNextVoiceStatus(u8 Val) ; //状态转换
static unsigned char TTSApp_PackVoiceStr(u8 *ptr,u8 datalen) ;   //打包语音
static u8 TTSApp_PlayMulTTSVoiceStrbase(u8 *StrPtr,u8 type);

void TTSApp_StatusVoiceInit(void);
void TTSApp_StatusVoiceLoad(void);
void TTSApp_StatusVoicePlay(void);
void TTSApp_StatusVoiceEnd(void);
void TTSApp_TimeStatusTask(void);
void TTSApp_CheckExtTtsTimeOut(void);
////////////////////////////////////
enum 
{
    TTS_TIMER_TASK,                     //  
    TTS_TIMER_POWER,                //    
    TTS_TIMERS_MAX
}STTTSTIME;

static LZM_TIMER s_stTTSTimer[TTS_TIMERS_MAX];
/********************************************************************
* 功  能:TTSApp_TTSPowerOff
* 输  入:停止当前正在播报的任务
* 输  出:
* 全局变量:
* 备注: 
********************************************************************/
void TTSApp_TurnOffPower(void) 
{
    s_stTtsAttirb.sendDataLen=0;
    s_stTtsAttirb.curSendLen = 0;
    s_stTtsAttirb.voiceDataLen = 0;
    IntoNextVoiceStatus(E_VOICE_MAX);
    LZM_PublicKillTimer(&s_stTTSTimer[TTS_TIMER_POWER]);
    s_stTtsAttirb.busy=0;            //关闭TTS全局忙标志    
    #ifndef EYE_MODEM
    if (gPhone.status == GSM_PHONE_IDLE)                          //dxl  
    #endif
    Io_WriteSpeakerConrtolBit(SPEAKER_BIT_TTS, RESET);  
    TTSDerv_CtrlTTSPower(S_OFF_TTS_POWER);    //关闭TTS电源  
    ClrTimerTask(TIME_TTSDELAY);
}
/********************************************************************
* 功能:TTSApp_TTSPowerOff
* 输入:
* 输出:
* 全局变量:
*
* 备注: 
********************************************************************/
void TTSApp_TurnOnPower(void) 
{
    TTSDerv_CtrlTTSPower(S_ON_TTS_POWER);            //打开TTS电源
    s_stTtsAttirb.sendDataLen = 0;
    Io_WriteSpeakerConrtolBit(SPEAKER_BIT_TTS, SET);
    TTSDev_ResetTTS();
    SetTimerTask(TIME_TTSDELAY,1);
}
/********************************************************************
* 功能:打包语音
* 输入:指向语音字节的指针
* 输出:
* 全局变量:
*
* 备注: 不可由外部函数调用.
********************************************************************/
static unsigned char TTSApp_CheckPackVoiceStr(void) 
{
    ST_TTS_PRO stTTSApp;
    u8 vlen = 0;
    if(GpioInGetState(TTS_RDY))
    {
        memset(&stTTSApp.VoiceData[0],0,TTS_VOICE_SIZE);                  //清空缓冲区
        sprintf((char*)&(stTTSApp.VoiceData[0]),"[v%d]",0);
        vlen = strlen((char*)&(stTTSApp.VoiceData[0]));
        memcpy((u8 *)&(stTTSApp.VoiceData[vlen]),"...",3);  
        vlen = 3 + vlen;          //重新计算总长度
        stTTSApp.Head=0xFD;  //加头
        stTTSApp.Cmd = 1;    //语音合成
        stTTSApp.FontStyle=0; //GB2313
        stTTSApp.Len[0]=((2+vlen)>>8) & 0xFF;
        stTTSApp.Len[1]=(2+vlen) & 0xFF;
        vlen+=5; 
        stTTSApp.PageLen = vlen;
        LZM_PublicSetOnceTimer(&s_stTTSTimer[TTS_TIMER_POWER],PUBLICSECS(8),TTSApp_TurnOffPower); 
        return TTSDerv_SendUartData((u8 *)&stTTSApp,stTTSApp.PageLen); //把语音拷到发送区域,启动发送
    }
    return 0;
}
/********************************************************************
* 功  能:TTSApp_TTSPowerOff
* 输  入:停止当前正在播报的任务
* 输  出:
* 全局变量:
* 备注: 
********************************************************************/
static unsigned char TTSApp_PlayExtTtsFlag(void)
{
    return (s_stTtsAttirb.extTtsFlag==TTS_USE_EXT)?1:0;
}
/********************************************************************
* 功能:打包语音
* 输入:指向语音字节的指针
* 输出:
* 全局变量:
*
* 备注: 不可由外部函数调用.
********************************************************************/
static unsigned char TTSApp_PackVoiceStr(u8 *ptr,u8 datalen) 
{
    ST_TTS_PRO stTTSApp;
    u32 Len = 0;
    u8 vlen = 0;
    u8 time = 0; 
    char *pvstrd ="[v%d]";       //音量 yan add
    if(0==datalen)return 1;
    if(GpioInGetState(TTS_RDY))
    {
        //打包协议
        Len = datalen;
        if(Len > TTS_VOICE_SIZE-5)Len = TTS_VOICE_SIZE-5;
        memset(&stTTSApp.VoiceData[0],0,TTS_VOICE_SIZE);                  //清空缓冲区
        sprintf((char*)&(stTTSApp.VoiceData[0]),pvstrd,ttsValue[ttsVolume]);
        vlen = strlen((char*)&(stTTSApp.VoiceData[0]));
        memcpy((u8 *)&(stTTSApp.VoiceData[vlen]),ptr,Len);  
        Len = Len + vlen;          //重新计算总长度
        stTTSApp.Head=0xFD;  //加头
        stTTSApp.Cmd = 1;    //语音合成
        stTTSApp.FontStyle=0; //GB2313
        stTTSApp.Len[0]=((2+Len)>>8) & 0xFF;
        stTTSApp.Len[1]=(2+Len) & 0xFF;
        Len+=5;
        if(Len > TTS_MAX_SEND_LEN)
            Len=TTS_MAX_SEND_LEN;
        stTTSApp.PageLen = Len;
        time = (stTTSApp.PageLen/5) + 6;
        LZM_PublicSetOnceTimer(&s_stTTSTimer[TTS_TIMER_POWER],PUBLICSECS(time),TTSApp_TurnOffPower); 
        return TTSDerv_SendUartData((u8 *)&stTTSApp,stTTSApp.PageLen); //把语音拷到发送区域,启动发送
    }
    /////////
    return 0;
}
/*************************************************************
** 函数名称: PlayMulTTSVoiceBuffExternal
** 功能描述: 使用外部TTS播报内容
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
u8 PlayMulTTSVoiceBuffExternal(u8 *buff,u16 len,unsigned char type)              //播放语音处理
{   
    u16 Time;
    u8 psoundlen = 0;
    char *psound =USE_INFO_SOUND" ";              //需要增加空格,普通信息提示音.
    char *psoundAlarm =ALARM_INFO_SOUND" ";       //需要增加空格,报警提示音.
    
    //LOG_PR("TTSApp_PlayMulTTSVoiceStrbase:[%s] \r\n",buff);
    
    #ifdef EYE_MODEM
    if((buff == NULL)||MODON_PHONE == Modem_Api_OnSta()) 
    #else
    if((buff == NULL)||(gPhone.status == GSM_PHONE_TALK))
    #endif
    {
        return 0;
    }  
    #ifndef EYE_MODEM
    if (gPhone.status == GSM_PHONE_IDLE)                          //dxl  
	    Io_WriteSpeakerConrtolBit(SPEAKER_BIT_TTS, RESET);
    #endif
    ///////////
    TTSApp_TurnOnPower(); //
    //////////
    if(type == TTS_PLAY_TYPE_USE)
    {
        strcpy((char *)&s_ucTtsPlayVoiceData[0],(char const *)psound);       //yan add- 普通信息
        psoundlen = strlen(psound);
    }else{
        strcpy((char *)&s_ucTtsPlayVoiceData[0],(char const *)psoundAlarm);  //yan add- 报警信息
        psoundlen = strlen(psoundAlarm);        
    }
    
    s_stTtsAttirb.voiceDataLen = len;
    if((s_stTtsAttirb.voiceDataLen+psoundlen) >VOICE_DATA_LEN)
    {
        s_stTtsAttirb.voiceDataLen = VOICE_DATA_LEN-psoundlen;
    }   

    strncpy((char *)&s_ucTtsPlayVoiceData[psoundlen] ,(char const *)buff,s_stTtsAttirb.voiceDataLen);
    s_stTtsAttirb.voiceDataLen += psoundlen;
      
    s_ucTtsPlayVoiceData[s_stTtsAttirb.voiceDataLen]='\0';
    s_stTtsAttirb.busy = 1;
    IntoNextVoiceStatus(E_VOICE_INIT);
    LZM_PublicSetOnceTimer(&s_stTTSTimer[TTS_TIMER_TASK],LZM_AT_ONCE,TTSApp_TimeStatusTask);
    ///////////////////////////
    Time=s_stTtsAttirb.voiceDataLen/5 ; //5个字节1s 
    if(s_stTtsAttirb.voiceDataLen%5)Time++;
    Time += 10;   //多给10s超时
    LZM_PublicSetOnceTimer(&s_stTTSTimer[TTS_TIMER_POWER],PUBLICSECS(Time),TTSApp_TurnOffPower);
    return 1; //成功
}
/*************************************************************
** 函数名称: PlayMulTTSVoiceBuffInternal
** 功能描述: 使用内部TTS播报内容
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
u8 PlayMulTTSVoiceBuffInternal(u8 *buff,u16 len)              //播放语音处理
{
    #ifdef EYE_MODEM
    if(MODON_PHONE == Modem_Api_OnSta())return 0;  
    /////////////////////////
    if(2==Modem_Api_Type())////中兴模块最大只支持140个字节
    {
        s_stTtsAttirb.oneMaxLen = 100;
        s_stTtsAttirb.oneMinLen = 64;
    }
    else
    {
        s_stTtsAttirb.oneMaxLen = 226;
        s_stTtsAttirb.oneMinLen = 180;
    }
    ////////////////////////
    if(len>s_stTtsAttirb.oneMaxLen)
    {
        s_stTtsAttirb.sendDataLen = 0;                   //清除计数
        s_stTtsAttirb.voiceDataLen = len;
        if(s_stTtsAttirb.voiceDataLen >VOICE_DATA_LEN)
        {
            s_stTtsAttirb.voiceDataLen = VOICE_DATA_LEN;
        } 
        memcpy(s_ucTtsPlayVoiceData,buff,s_stTtsAttirb.voiceDataLen);
        ///////////////////////
        s_stTtsAttirb.busy = 1;
        IntoNextVoiceStatus(E_VOICE_LOAD);
        LZM_PublicSetOnceTimer(&s_stTTSTimer[TTS_TIMER_TASK],LZM_AT_ONCE,TTSApp_TimeStatusTask);
        Io_WriteSpeakerConrtolBit(SPEAKER_BIT_TTS, RESET);
        SetTimerTask(TIME_TTSDELAY,1);
        len=s_stTtsAttirb.voiceDataLen/5 ; //5个字节1s 
        if(s_stTtsAttirb.voiceDataLen%5)len++;
        len += 10;   //多给10s超时
        LZM_PublicSetOnceTimer(&s_stTTSTimer[TTS_TIMER_POWER],PUBLICSECS(len),TTSApp_TurnOffPower);        
    }
    else
    {
        TTSApp_TurnOffPower(); //
        return Modem_Api_Tts_Play(0,buff,len);
    }
    #endif
    return 1;
}

//-----------------------------------------------------------------------------------
//功能:分段播放语音,普通信息
//输入参数：StrPtr 字符串
//          type    普通信息或报警信息
//返回：
//-----------------------------------------------------------------------------------
u8 PlayMulTTSVoiceBuff(u8 *buff,u16 len,unsigned char type)              //播放语音处理
{
    if(0==len)return 0; 
    if(TTS_USE_EMPTY==s_stTtsAttirb.extTtsFlag)return 0;
    if(TTSApp_PlayExtTtsFlag())
    {
        s_stTtsAttirb.oneMaxLen = 172;
        s_stTtsAttirb.oneMinLen = 128;
        return PlayMulTTSVoiceBuffExternal(buff,len,type);
    }
    #ifdef EYE_MODEM
    else
    if(Modem_Api_Tts_IsSpt())//dxl,2014.6.11增加内置tts时修改
    {
       return PlayMulTTSVoiceBuffInternal(buff,len); 
    }
    #endif
    return 0;
}
/*************************************************************
** 函数名称: TTSApp_PlayMulTTSVoiceStrbase
** 功能描述: 
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
static u8 TTSApp_PlayMulTTSVoiceStrbase(u8 *StrPtr,u8 type) 
{   
    return PlayMulTTSVoiceBuff(StrPtr,strlen((char const *)StrPtr),type);
}
/*************************************************************
** 函数名称: TTSApp_StatusVoiceInit
** 功能描述: 
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void TTSApp_StatusVoiceInit(void)
{
    s_stTtsAttirb.sendDataLen = 0;                   //清除计数
    Io_WriteSpeakerConrtolBit(SPEAKER_BIT_TTS, SET);
    IntoNextVoiceStatus(E_VOICE_LOAD);
    LZM_PublicSetOnceTimer(&s_stTTSTimer[TTS_TIMER_TASK],PUBLICSECS(0.7),TTSApp_TimeStatusTask);
}
/*************************************************************
** 函数名称: TTSApp_StatusVoiceInit
** 功能描述: 
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void TTSApp_PackVoiceStrToSend(void)
{
    unsigned char flag;
    if(TTSApp_PlayExtTtsFlag())
    {
        flag=TTSApp_PackVoiceStr(s_stTtsAttirb.sendPtr,s_stTtsAttirb.curSendLen);    //打包语音        
    }
    #ifdef EYE_MODEM 
    else
    if(Modem_Api_Tts_IsSpt())//dxl,2014.6.11增加内置tts时修改
    {
        flag=Modem_Api_Tts_Play(0,s_stTtsAttirb.sendPtr,s_stTtsAttirb.curSendLen);
        flag = 1;
    }    
    #endif    
    ///////////////////
    if(flag||(0==s_stTtsAttirb.curSendLen))
    {
        IntoNextVoiceStatus(E_VOICE_PLAY);
        LZM_PublicSetOnceTimer(&s_stTTSTimer[TTS_TIMER_TASK],PUBLICSECS(4),TTSApp_TimeStatusTask); 
    }
    else
    {
        LZM_PublicSetOnceTimer(&s_stTTSTimer[TTS_TIMER_TASK],PUBLICSECS(0.1),TTSApp_PackVoiceStrToSend); 
    }
}
/*************************************************************
** 函数名称: TTSApp_StatusVoiceInit
** 功能描述: 
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void TTSApp_StatusVoiceLoad(void)
{
    u16 len;
    u8 chr,temp;
    ////////////////
    s_stTtsAttirb.curSendLen = 0;
    //////////////////////
    if(s_stTtsAttirb.sendDataLen<s_stTtsAttirb.voiceDataLen)
    {
        s_stTtsAttirb.sendPtr=&s_ucTtsPlayVoiceData[s_stTtsAttirb.sendDataLen];
        ////////////////////
        if(0==s_stTtsAttirb.oneMaxLen)//
        {
            s_stTtsAttirb.oneMaxLen = 100;
            s_stTtsAttirb.oneMinLen = 64;
        }
        len=s_stTtsAttirb.voiceDataLen-s_stTtsAttirb.sendDataLen;
        if(len>s_stTtsAttirb.oneMaxLen)len = s_stTtsAttirb.oneMaxLen;
        for( ; s_stTtsAttirb.curSendLen<len; ) //
        {
            if(s_stTtsAttirb.curSendLen<s_stTtsAttirb.oneMinLen)
            {   //最少收50个可读语音
                chr = s_stTtsAttirb.sendPtr[s_stTtsAttirb.curSendLen++];
                if(chr >=0x80)
                {
                    s_stTtsAttirb.curSendLen++;
                }
            }
            else
            {
                temp = s_stTtsAttirb.curSendLen;
                chr  = s_stTtsAttirb.sendPtr[s_stTtsAttirb.curSendLen++];
                if(chr >=0x80)
                {
                    s_stTtsAttirb.curSendLen++;
                    if((0==strncmp((char *)&s_stTtsAttirb.sendPtr[temp],"，",2)) 
                     ||(0==strncmp((char *)&s_stTtsAttirb.sendPtr[temp],"、",2))
                     ||(0==strncmp((char *)&s_stTtsAttirb.sendPtr[temp],"。",2)))
                    {
                        break;
                    }                    
                }
                else
                {  //找到','或者'.'为止
                    if(chr==',' || chr=='.')
                    {
                        break;
                    }
                }
            }
        }
        ///////////////////
        s_stTtsAttirb.sendDataLen +=s_stTtsAttirb.curSendLen; 
        if(s_stTtsAttirb.sendDataLen>s_stTtsAttirb.voiceDataLen)//最后一个汉字不全,播报时会出错
        {
            if(s_stTtsAttirb.curSendLen>2)
            {
                s_stTtsAttirb.curSendLen-=2;
            }
            else
            {
                s_stTtsAttirb.curSendLen =0;
            }
        }
        if(s_stTtsAttirb.curSendLen)
        {
            TTSApp_PackVoiceStrToSend();
            return;
        }
    }
    //////////////////
    IntoNextVoiceStatus(E_VOICE_END);
    LZM_PublicSetOnceTimer(&s_stTTSTimer[TTS_TIMER_TASK],LZM_AT_ONCE,TTSApp_TimeStatusTask);
}
/*************************************************************
** 函数名称: TTSApp_StatusVoiceInit
** 功能描述: 
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void TTSApp_StatusVoicePlay(void)
{
    unsigned char flag;
    if(TTSApp_PlayExtTtsFlag())
    {
        flag=GpioInGetState(TTS_RDY);
        flag^=1;
    }    
    #ifdef EYE_MODEM 
    else
    if(Modem_Api_Tts_IsSpt())
    {
        flag=Modem_Api_Tts_IsSta();
    }    
    #endif    
    ///////////////////////
    if(0==flag)                 //该段播放完成,准备装载下一段
    {
        IntoNextVoiceStatus(E_VOICE_LOAD);
        LZM_PublicSetOnceTimer(&s_stTTSTimer[TTS_TIMER_TASK],LZM_AT_ONCE,TTSApp_TimeStatusTask);
    }
    else
    {
        LZM_PublicSetOnceTimer(&s_stTTSTimer[TTS_TIMER_TASK],PUBLICSECS(0.1),TTSApp_TimeStatusTask);
    }
}
/*************************************************************
** 函数名称: TTSApp_StatusVoiceEnd
** 功能描述: 
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void TTSApp_StatusVoiceEnd(void)
{
    IntoNextVoiceStatus(E_VOICE_MAX);
    LZM_PublicSetOnceTimer(&s_stTTSTimer[TTS_TIMER_POWER],LZM_AT_ONCE,TTSApp_TurnOffPower); 
}
/*******************************************************************************
**  函数名称  : CommPJA_TimeStatusTask
**  函数功能  : 评价器模块时间任务调度接口函数
**  输    入  : 无				
**  输    出  : 任务调度状态:  使能或禁止
**  全局变量  : 
**  调用函数  : 
**  中断资源  : 
**  备    注  :
*******************************************************************************/
void TTSApp_TimeStatusTask(void) 
{
    #ifdef EYE_MODEM
    if(0==s_stTtsAttirb.busy||MODON_PHONE == Modem_Api_OnSta())
    #else
    if(0==s_stTtsAttirb.busy||gPhone.status != GSM_PHONE_IDLE)
    #endif
    {
        TTSApp_TurnOffPower();
        return;
    }
    switch(s_stTtsAttirb.status)
    {
        case E_VOICE_INIT:
            TTSApp_StatusVoiceInit();
            break;
        case E_VOICE_LOAD:
            TTSApp_StatusVoiceLoad();
            break;
        case E_VOICE_PLAY:
            TTSApp_StatusVoicePlay();
            break;
        case E_VOICE_END:
            TTSApp_StatusVoiceEnd();
            break;
        default:
            TTSApp_StatusVoiceEnd();
            break;
    }
}


/********************************************************************
* 功能:状态转换
* 输入:状态量
* 输出:
* 全局变量:
*
* 备注:
********************************************************************/
void IntoNextVoiceStatus(u8 Val) 
{
	s_stTtsAttirb.status = Val;
}
//-----------------------------------------------------------------------------------
//功能:分段播放语音,普通信息
//输入参数：字符串
//返回：
//-----------------------------------------------------------------------------------
u8 PlayMulTTSVoiceStr(u8 *StrPtr) 
{   
    return TTSApp_PlayMulTTSVoiceStrbase(StrPtr,TTS_PLAY_TYPE_USE);
}

//-----------------------------------------------------------------------------------
//功能:分段播放语音,报警信息
//输入参数：字符串
//返回：
//-----------------------------------------------------------------------------------
u8 PlayMulTTSVoiceAlarmStr(u8 *StrPtr) 
{   
    return TTSApp_PlayMulTTSVoiceStrbase(StrPtr,TTS_PLAY_TYPE_ALARM);
}

//-----------------------------------------------------------------------------------
//功能:音量递增
//输入参数：
//返回：
//----------------------------------------------------------------------------------- 
void SetTtsVolumeIncr(void)
{
    if(TTSApp_PlayExtTtsFlag())
    {
        if(ttsVolume < 8) 
        {
            ttsVolume++;         //音量最大值是16
            FRAM_BufferWrite(FRAM_TTS_VOLUME_ADDR,(u8*)&ttsVolume,FRAM_TTS_VOLUME_LEN);
        }
    }
    #ifdef EYE_MODEM 
    else
    if(Modem_Api_Tts_IsSpt())//dxl,2014.6.11增加内置tts时修改
    {
        SetModTtsVolumeIncr();
    }    
    #endif
    
}
//-----------------------------------------------------------------------------------
//功能:音量递减
//输入参数：
//返回：
//----------------------------------------------------------------------------------- 
void SetTtsVolumeDecr(void)
{
    if(TTSApp_PlayExtTtsFlag())
    {
        if(ttsVolume > 1)
        {
            ttsVolume--;         //音量最小指是0,静音
            FRAM_BufferWrite(FRAM_TTS_VOLUME_ADDR,(u8*)&ttsVolume,FRAM_TTS_VOLUME_LEN);
        }
    }
    #ifdef EYE_MODEM 
    else
    if(Modem_Api_Tts_IsSpt())//dxl,2014.6.11增加内置tts时修改
    {
        SetModTtsVolumeDecr();
    }    
    #endif    
}

//-----------------------------------------------------------------------------------
//功能:获取TTS当前音量
//输入参数：
//返回：
//----------------------------------------------------------------------------------- 
u8 GetTtsVolume(void)
{
    if(TTSApp_PlayExtTtsFlag())
    {
        return ttsVolume;
    }
    #ifdef EYE_MODEM 
    else
    if(Modem_Api_Tts_IsSpt())//dxl,2014.6.11增加内置tts时修改
    {
        return GetModTtsVolume();
    }    
    #endif 
    return 0;
}
/********************************************************************
* 名称 : TtsVolumeInit
* 功能 : TTS音量初始化
* 输入 : 无
* 输出 : 无
* 全局变量:
* 调用函数:
* 中断资源:  (没有,不写)
* 备注:
********************************************************************/
void TtsVolumeInit(void)
{
    FRAM_BufferRead((u8*)&ttsVolume,FRAM_TTS_VOLUME_LEN,FRAM_TTS_VOLUME_ADDR);
    if((ttsVolume == 0) || (ttsVolume >8))
    {
        ttsVolume = 5;
    }    
}
/********************************************************************
* 名   称 : TTS_GetPlayBusyFlag
* 功   能 : 取得播报忙标志
* 输   入 : 无
* 输   出 : 正在播报返回1, 播报完毕返回0
* 全局变量:
* 调用函数:
* 中断资源:  
* 备注:
********************************************************************/
unsigned char TTS_GetPlayBusyFlag(void)
{
    if(TTSApp_PlayExtTtsFlag())
    {
        return TtsPowerFlag;
    }
    #ifdef EYE_MODEM
    else
    if(Modem_Api_Tts_IsSpt())//dxl,2014.6.11增加内置tts时修改
    {
        return Modem_Api_Tts_IsSta();
    }
    #endif
    return 0;    
}
/************************************************************
函数名称: TTSApp_CheckExtTtsFlag
功能描述: 设备及全局变量初始化
入口参数: 
出口参数:
************************************************************/
void TTSApp_CheckRDYFlag(void)
{
    s_stTtsAttirb.checkRDYCnt++;
    if(0==GpioInGetState(TTS_RDY))
    {
        s_stTtsAttirb.checkCnt =0;
        s_stTtsAttirb.checkRDYCnt = 0;
        s_stTtsAttirb.extTtsFlag = TTS_USE_EXT;
        TTSApp_TurnOffPower();
    }
    else   
    if(s_stTtsAttirb.checkRDYCnt>50)
    {
        TTSApp_CheckExtTtsTimeOut();
    }
    else
    {
       LZM_PublicSetOnceTimer(&s_stTTSTimer[TTS_TIMER_TASK],PUBLICSECS(0.1),TTSApp_CheckRDYFlag);
    }
    
}
/************************************************************
函数名称: TTSApp_CheckExtTtsFlag
功能描述: 设备及全局变量初始化
入口参数: 
出口参数:
************************************************************/
void TTSApp_CheckExtTtsFlag(void)
{
    unsigned char result;
    s_stTtsAttirb.checkCnt++;
    result=TTSApp_CheckPackVoiceStr();
    if(result)
    {
        s_stTtsAttirb.checkRDYCnt = 0;
        LZM_PublicSetOnceTimer(&s_stTTSTimer[TTS_TIMER_TASK],1,TTSApp_CheckRDYFlag); 
    }
    else
    {
        TTSApp_CheckExtTtsTimeOut();
    }    
}
/************************************************************
函数名称: TTSApp_CheckExtTtsFlag
功能描述: 设备及全局变量初始化
入口参数: 
出口参数:
************************************************************/
void TTSApp_CheckExtTtsTimeOut(void)
{
    s_stTtsAttirb.checkRDYCnt = 0;
    if(s_stTtsAttirb.checkCnt<3)
    {
        LZM_PublicSetOnceTimer(&s_stTTSTimer[TTS_TIMER_TASK],PUBLICSECS(0.5),TTSApp_CheckExtTtsFlag); 
    }
    else
    {
        s_stTtsAttirb.checkCnt =0;
        s_stTtsAttirb.checkRDYCnt = 0;
        s_stTtsAttirb.extTtsFlag = TTS_USE_INT;
        TTSApp_TurnOffPower();
    }
}
/************************************************************
函数名称: 
功能描述: 设备及全局变量初始化
入口参数: 
出口参数:
************************************************************/
void TTS_Init(void)
{
    memset(&s_stTtsAttirb,0,sizeof(s_stTtsAttirb));
    TtsVolumeInit();
    //设备初始化
    TTSDev_UartInit();             //TTS串口初始化 
    //全局变量初始化
    TTSApp_TurnOffPower();    
    /////////////
    s_stTtsAttirb.oneMaxLen = 172;
    s_stTtsAttirb.oneMinLen = 120;
    ///////////////////
    Io_WriteSpeakerConrtolBit(SPEAKER_BIT_TTS, RESET); 
    LZM_PublicKillTimerAll(s_stTTSTimer,TTS_TIMERS_MAX);
    /////////////////////////
    TTSApp_TurnOnPower();
    ///////////////////
    LZM_PublicSetOnceTimer(&s_stTTSTimer[TTS_TIMER_TASK],PUBLICSECS(1),TTSApp_CheckExtTtsFlag);    
    ///////////////////
    LZM_PublicSetOnceTimer(&s_stTTSTimer[TTS_TIMER_POWER],PUBLICSECS(25),TTSApp_TurnOffPower);
}
/************************************************************
函数名称: 
功能描述: 周期任务
入口参数: 
出口参数:
备注: 每20ms调用一次
************************************************************/ 
FunctionalState TTS_TimeTask(void)
{
    LZM_PublicTimerHandler(s_stTTSTimer,TTS_TIMERS_MAX);
    return ENABLE;
}
/******************************************************************************
**                            End Of File
******************************************************************************/


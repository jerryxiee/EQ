/*******************************************************
Copyright (C),E-EYE Tech. Co.,Ltd.
Author: WYF
Version:V1.0
Finish Date:
Description:TTS协议解析
History: 
    1,Date:
      Author:
      Modification:
    2,
    
备注：
    1,最多发送语音长度500BYTES
    2,单次最多发送语音长度180BYTES
    3,发送思路,先准备数据,再打开电源,等待稳定后发送
    
*******************************************************/
#ifndef _TTSDEVAPP_H_
#define _TTSDEVAPP_H_

u8 PlayMulTTSVoiceStr(u8 *StrPtr) ;      //分段播放语音
u8 PlayMulTTSVoiceAlarmStr(u8 *StrPtr);

void SetTtsVolumeIncr(void);
void SetTtsVolumeDecr(void);
u8 GetTtsVolume(void);
void TtsVolumeInit(void);
//-----------------------------------------------------------------------------------
//功能:分段播放语音
//输入参数：buff 字符串,len长度
//          type    普通信息或报警信息
//返回：
//-----------------------------------------------------------------------------------
u8 PlayMulTTSVoiceBuff(u8 *buff,u16 len,unsigned char type);              //播放语音处理
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
unsigned char TTS_GetPlayBusyFlag(void);
/************************************************************
函数名称: 
功能描述: 周期任务
入口参数: 
出口参数:
备注: 每20ms调用一次
************************************************************/ 
FunctionalState TTS_TimeTask(void);
/************************************************************
函数名称: 
功能描述: 设备及全局变量初始化
入口参数: 
出口参数:
************************************************************/
void TTS_Init(void);

#endif

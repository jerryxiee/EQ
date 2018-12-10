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
#ifndef _TTSDEV_H_
#define _TTSDEV_H_

extern u8  TtsPowerFlag;//dxl,TTS电源供电标志,0为断电,1为供电,播报语音前请先检查该标志是否为0,不为0表示正在播报

/*******************************************************
Description:串口进行配置
Input:
Output:
Return:
*******************************************************/
void TTSDev_UartInit(void);
/*******************************************************
Description:把语音拷到发送区域,启动发送   
Input: 发送数据地址,发送长度
Output:
Return: 0:可以发送 1:发送失败
*******************************************************/
u8 TTSDerv_SendUartData(u8 *Ptr,u16 Len);
/*******************************************************
Description:TTS电源控制
Input: S_OFF_TTS_POWER \ S_ON_TTS_POWER
Output:
Return: 0:可以发送 1:发送失败
*******************************************************/
void TTSDerv_CtrlTTSPower(u8 Val);
//-----------------------------------------------
//功能:复位TTS
//输入:
//输出:
//备注:
//-----------------------------------------------
void TTSDev_ResetTTS(void);
#endif

/*******************************************************
Copyright (C),E-EYE Tech. Co.,Ltd.
Author: WYF
Version:V1.0
Finish Date:2012-6-12
Description:TTS驱动
*******************************************************/ 
#include "include.h"

u8  TtsPowerFlag = 0;//dxl,TTS电源供电标志,0为断电,1为供电,播报语音前请先检查该标志是否为0,不为0表示正在播报
/*******************************************************
Description:串口进行配置
Input:
Output:
Return:
*******************************************************/
void TTSDev_UartInit(void) 
{
    GpioInInit(TTS_RDY); 
    GpioOutInit(TTS_P);      //与GPS发送串口公用,TTS_P拉高则，GPS发送串口失效.
}      
/*******************************************************
Description:把语音拷到发送区域,启动发送   
Input: 发送数据地址,发送长度
Output:
Return: 0:可以发送 1:发送失败
*******************************************************/
u8 TTSDerv_SendUartData(u8 *Ptr,u16 Len)
{
    Gps_SendData(Ptr,Len);
    return 1;
}
/*******************************************************
Description:TTS电源控制
Input: S_OFF_TTS_POWER \ S_ON_TTS_POWER
Output:
Return: 0:可以发送 1:发送失败
*******************************************************/
void TTSDerv_CtrlTTSPower(u8 Val) 
{
    if(Val)
    {
        TtsPowerFlag =1;
        GpioOutOn(TTS_P);
    }
    else
    {
        TtsPowerFlag =0;
        GpioOutOff(TTS_P);
    }
}
//-----------------------------------------------
//功能:复位TTS
//输入:
//输出:
//备注:
//-----------------------------------------------
void TTSDev_ResetTTS(void) 
{
    vu16 i;
    GpioOutOff(TTS_P);       //复位TTS
    for(i=0; i<0XFFFF; i++); //Delay 
    GpioOutOn(TTS_P);
}
/******************************************************************************
**                            End Of File
******************************************************************************/
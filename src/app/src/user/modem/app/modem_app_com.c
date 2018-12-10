/*
********************************************************************************
*
*                                                              
*
* Filename      : modem_app_com.c
* Version       : V1.00
* Programmer(s) : miaoyahan
* @date         : 2014-10-23  
* 
*
********************************************************************************
*/

/*
********************************************************************************
*                               INCLUDES
********************************************************************************
*/
#ifdef EYE_MODEM
#include "include.h"
#else
#include  <app_cfg.h>
#include  <includes.h>
#endif

/*
********************************************************************************
*                               DEFIEN START
********************************************************************************
*/
#ifdef MODEM_APP_COM_H

/*
********************************************************************************
*                             LOCAL DEFINES
********************************************************************************
*/


/*
********************************************************************************
*                         LOCAL DATA TYPES
********************************************************************************
*/


/*
********************************************************************************
*                          LOCAL VARIABLES
********************************************************************************
*/
static u8 communication_tasksta = 1;//通讯模块任务状态 0为关闭任务 1为打开任务

/*
********************************************************************************
*                          GLOBAL VARIABLES
********************************************************************************
*/
CALL_RECORD     gPhone;             //通话相关
SMS_RECORD      gSmsRecord;         //短信相关
UART2_RECEIVE	uart2_recdat;	    //串口2接收

/******************************************************************************///EGS701
#ifdef   EYE_EGS701
    u8 BlindSaveEnableFlag = 0;//盲区记录使能标志,1为使能
/******************************************************************************///EGS702
#elif    defined EYE_EGS702
    extern u8 BlindSaveEnableFlag;
/******************************************************************************///缺省
#else
    u8 BlindSaveEnableFlag = 0;
#endif

/*
********************************************************************************
*                          EXTERN VARIABLES
********************************************************************************
*/

/*
********************************************************************************
*                           FUNCTIONS EXTERN
********************************************************************************
*/


/*
********************************************************************************
*                              FUNCTIONS
********************************************************************************
*/
/**
  * @brief  打开通讯模块电源
  * @param  None.
  * @retval None.
  */
#ifndef   EYE_EGS702  
void GSM_PWR_ON(void)
{

}
#endif
/**
  * @brief  关闭通讯模块电源
  * @param  None.
  * @retval None.
  */
void GSM_PWR_OFF(void)
{

}

/**
  * @brief  通讯模块处理任务
  * @param  None.
  * @retval 返回任务运行情况.
  */
FunctionalState Communication_TimeTask(void)
{
#ifdef EYE_MODEM
    Modem_Run();  
    Net_First_Run();
    Net_Other_Run();
    #ifdef EYE_EGS701
    Net_Second_Run();
    #endif
	#ifdef ICCARD_CERTIFICATE_SERVICE_H
	CertService_Run();
	#endif
#endif    

    if(communication_tasksta == 0)
    {
        return DISABLE;
    }
    return ENABLE;
}
/**
  * @brief  通讯模块任务设置
  * @param  state 0 -> 关闭任务;
  *               1 -> 打开任务;
  * @retval None.
  */
void Communication_TaskSet(u8 state)
{
    if(state == 0)
    {
        communication_tasksta = 0;
    }
    else
    {
        communication_tasksta = 1;
    }
}
/**
  * @brief  通讯模块初始化
  * @param  None.
  * @retval None.
  */
void Communication_Init(void)
{
#ifdef EYE_MODEM
    Modem_Api_Start();
#endif
}
/**
  * @brief  拨打电话回调函数
  * @param  *pCode :指向电话号码
  * @param  type   :电话类型
  *                 PHONE_NORMAL  : 正常通话 
  *                 PHONE_MONITOR : 监听 
  * @retval 成功返回1  失败返回0
  */
u8 communication_CallPhone(u8 *pCode, CALL_TYPE type)
{
#ifdef EYE_MODEM
    if (type > PHONE_END)
    {
        return 0;
    }    
    if(type == PHONE_NORMAL)////////////////////////////////////////////拨打电话
    {
        if(Modem_Api_Phone_Atd(pCode,1))
        {
            
            return 1;
        }
    }
    else if(type == PHONE_MONITOR)//////////////////////////////////////监听电话
    {
        if(Modem_Api_Phone_Atd(pCode,0))
        {
            
            return 1;
        }
    }
    else if(type == PHONE_TALK)/////////////////////////////////////////////接听
    {
        if(Modem_Api_Phone_Ata())
        {
            return 1;
        }
    }
    else if(type == PHONE_END)//////////////////////////////////////////////挂断
    {
        if(Modem_Api_Phone_Ath())
        {
            return 1;
        }
    }
#endif    
	return  0;
}

/**
  * @brief  获取信号强度
  * @param  None.
  * @retval 返回信号强度.
  */
u8 communicatio_GetSignalIntensity(void)
{
#ifdef EYE_MODEM
    return Modem_Port_Csq();
#else
    return 0;
#endif
}
/**
  * @brief  发送缓冲数据到中心
  * @param    channel:数据通道
  * @param    srcdat :指向发送数据
  * @param    srclen :发送数据长度
  * @retval 返回发送结果
  *                              0 :失败  
  *                     等于srclen :成功
  */
u16 Communication_SndMsg2Svr(u8 channel, u8 * srcdat, u16 srclen, u8 *phone)
{
    u16 len = 0;
    u8  tab[MODEM_SMS_SENDMAX];
#ifdef EYE_MODEM
    ////////////////////////////////////////////////////////////////////短信通道	
    if(channel == CHANNEL_SMS)
    {
        if(Modem_Type == MODEM_TYPE_ZTE)/////中兴 强制转为文本模式
        {
            if((srclen*2) > MODEM_SMS_SENDMAX)
            {
                return 0;
            }
            len  = ModLib_ByteStr(tab,srcdat,srclen);
            Modem_Api_SmsSend(SMS_TXT,NULL,tab,len);
            return len;
        } 
        len = srclen;
        Modem_Api_SmsSend(SMS_PDU,NULL,srcdat,srclen);
    }
    ////////////////////////////////////////////////////////////////////第二通道	
    #ifdef MODEM_APP_SECOND_H
    else if(channel == CHANNEL_DATA_2)
    {
       	len = Net_Second_Send(srcdat,srclen);
		}
		#endif
		#ifdef ICCARD_CERTIFICATE_SERVICE_H
		else if(channel == CHANNEL_DATA_3)
    {
       	len = CertService_Send(srcdat,srclen);
		}
		#endif
    ////////////////////////////////////////////////////////////////////第一通道
    else
    {
        len = Net_First_Send(srcdat,srclen);
    }
#endif    
    return len;
}

/**
  * @brief  读取模块状态
  * @param  type :读取的数据类型
  *              COMMUNICATE_STATE :通讯模块状态
  *              SIGN_STRENTH      :信号质量
  * @retval 返回类型信息
  */
u16 communicatio_GetMudulState(GPRSSTATE type)
{
	u8 ch=0;

	if(COMMUNICATE_STATE == type)
	{
	#if (ICCARD_JTD == ICCARD_SEL)
		ch = Net_First_ConSta();
		ch |= Net_Second_ConSta();
	#else
       ch = Net_First_ConSta();
		//	 ch = CertService_ConState();
	#endif
	} 
	else if(SIGN_STRENTH == type) 
	{
        ch = Modem_Port_Csq();
	}
 
    return (ch);
}
/**
  * @brief  读取连接状态
  * @param  channel : 通道号
  * @retval 返回类型信息
  */
u8 communicatio_NetState(u8 channel)
{
	u8 ch=0;
	
    #ifdef MODEM_APP_SECOND_H
	if(channel == CHANNEL_DATA_2) 
	{
       ch = Net_Second_ConSta();
	} 
    else
    #endif
	{
        ch = Net_First_ConSta();
	}
  
    return (ch);
}
/*******************************************************************************
* Function Name  : Communication_CloseGprsTimeTask(void)
* Description    : 用于使用前锁定一，主动关闭GPRS
* Input          : None
* Output         : None
* Return         : 仅仅为行标北斗测试添加
*******************************************************************************/
FunctionalState Communication_CloseGprsTimeTask(void)
{
#ifdef EYE_MODEM
      static u8   count = 0;
      count++;
      
      if(count < 10)//modify by joneming 20130624
      {
              MOD_CloseMainIp();//模块主动断开连接
      }
      else if(10 == count)
      {
              Communication_Close2();
      }
      else if(11 == count)
      {
              count = 0;
              return DISABLE;
      }
#endif      
      return ENABLE;
}

/*******************************************************************************
* Function Name  : Communication_Open
* Description    : 开启无线通信
* Input          : None
* Output         : None
* Return         : 行标北斗功能盲区补报测试时,出区域后需要调用这个函数
*******************************************************************************/
void Communication_Open(void)
{
#ifdef EYE_MODEM
    Net_First_Open();
    BlindSaveEnableFlag = 0;
#endif    
}
/*******************************************************************************
* Function Name  : Communication_Close
* Description    : 关闭无线通信
* Input          : None
* Output         : None
* Return         : 行标北斗功能盲区补报测试时,进区域后需要调用这个函数
*******************************************************************************/
void Communication_Close(void)
{
#ifdef EYE_MODEM
    Net_First_Close();  
    BlindSaveEnableFlag = 1;
#endif    
}
/*******************************************************************************
* Function Name  : Communication_Close2
* Description    : 关闭无线通信
* Input          : None
* Output         : None
* Return         : 行标北斗锁定一功能测试时,需要调用这个函数
*******************************************************************************/
void Communication_Close2(void)
{     
#ifdef EYE_MODEM
    Net_First_Close(); 
    BlindSaveEnableFlag = 0;
#endif    
}

/**
  * @brief  关闭主通道的ip连接
  * @param  None.
  * @retval None.
  */
void MOD_CloseMainIp(void)
{
#ifdef EYE_MODEM
    Net_First_Close(); 
    BlindSaveEnableFlag = 0;
#endif  
}
/**
  * @brief  获取终端SIM卡ICCID号
  * @param  *Bcdbuff :指向被保存的缓存.
  * @retval None.
  */
void MOD_GetIccidBcdValue(u8 *Bcdbuff)
{
#ifdef EYE_MODEM
    Modem_Api_CCID(Bcdbuff);
#endif    
}
/**
  * @brief  增大音量
  * @param  None.
  * @retval None.
  */
void SetTelVolumeIncr(void)
{
#ifdef EYE_MODEM
    u8 value=0;
    FRAM_BufferRead((u8*)&value,FRAM_TEL_VOLUME__LEN,FRAM_TEL_VOLUME_ADDR);

    if(value < 8)
    {
        value++;
        if(Modem_Api_Phone_Clvl(value))
        {
            FRAM_BufferWrite(FRAM_TEL_VOLUME_ADDR,(u8*)&value,FRAM_TEL_VOLUME__LEN);  
        }
    }
#endif   
}
/**
  * @brief  减小音量
  * @param  None.
  * @retval None.
  */
void SetTelVolumeDecr(void)
{
#ifdef EYE_MODEM
    u8 value=0;
    FRAM_BufferRead((u8*)&value,FRAM_TEL_VOLUME__LEN,FRAM_TEL_VOLUME_ADDR);
    
    if(value > 1)
    {
        value--;
        if(Modem_Api_Phone_Clvl(value))
        {
            FRAM_BufferWrite(FRAM_TEL_VOLUME_ADDR,(u8*)&value,FRAM_TEL_VOLUME__LEN);
        }      
    }
#endif    
}
/**
  * @brief  获取当前电话音量
  * @param  None.
  * @retval None.
  */
u8 GetTelVolume(void)
{
#ifdef EYE_MODEM
    u8 value;
    
    value = Modem_Api_Phone_ClvlGet();

    return value;
#else
    return 0;
#endif  
}
/**
  * @brief  增大音量
  * @param  None.
  * @retval None.
  */
void SetModTtsVolumeIncr(void)
{
#ifdef EYE_MODEM
    u8 value=0;
    FRAM_BufferRead((u8*)&value,FRAM_TTS_VOLUME_LEN,FRAM_TTS_VOLUME_ADDR);

    if(value < 8)
    {
        value++;
        if(Modem_Api_Tts_Clvl(value))
        {
            FRAM_BufferWrite(FRAM_TTS_VOLUME_ADDR,(u8*)&value,FRAM_TTS_VOLUME_LEN);  
        }
    }
#endif   
}
/**
  * @brief  减小音量
  * @param  None.
  * @retval None.
  */
void SetModTtsVolumeDecr(void)
{
#ifdef EYE_MODEM
    u8 value=0;
    FRAM_BufferRead((u8*)&value,FRAM_TTS_VOLUME_LEN,FRAM_TTS_VOLUME_ADDR);
    
    if(value > 1)
    {
        value--;
        if(Modem_Api_Tts_Clvl(value))
        {
            FRAM_BufferWrite(FRAM_TTS_VOLUME_ADDR,(u8*)&value,FRAM_TTS_VOLUME_LEN);
        }      
    }
#endif    
}
/**
  * @brief  获取当前TTS音量
  * @param  None.
  * @retval None.
  */
u8 GetModTtsVolume(void)
{
#ifdef EYE_MODEM
    u8 value;
    
    value = Modem_Api_Tts_IsClvl();

    return value;
#else
    return 0;
#endif  
}
/**
  * @brief  IC卡认证中心连接任务
  * @param  None.
  * @retval 返回任务状态.
  */
FunctionalState ICAUC_TimeTask(void)
{   

    return ENABLE;
}

/**
  * @brief  建立于IC卡认证中心建立连接,IC认证中心IP地址从配置参数中读取。
  * @param  None.
  * @retval 成功返回ICAUC_RET_SUCCED,否则返回错误码.
  */
ICAUC_RetType ICAUC_OpenLnk(void)
{
    return ICAUC_RET_SUCCED;
}
/**
  * @brief  向IC卡认证中心发送数据
  * @param  None.
  * @retval 成功返回ICAUC_RET_SUCCED,否则返回错误码.
  */
ICAUC_RetType ICAUC_SendBuff(u8* buff,u16 len)
{

	return ICAUC_RET_SENDBUF_NO_CMD;    
}
/**
  * @brief  注册一个读取函数
  * @param  pFun :数据读取函数定义.
  * @retval None
  */
void ICAUC_RegReadFun(ICAUC_FUN_READ pFun)
{

}


/*********************************************************************
//函数名称	:ICAUC_RegReadFun
//功能		:获取连接状态
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		: 连接成功返回1,失败返回0
//备注		:
*********************************************************************/
/**
  * @brief  获取连接状态
  * @param  None
  * @retval None
  */
u8 ICAUC_GetLnkStatus(void)
{
    return 0;
}


void communication_CalTimer(void)
{

}
/********************************************************************
* 名称    :  MOD_InitCRC
* 功能    :  FTP服务需启动CRC校验
* 输入    :  无
* 输出    :  无
* 返回    :  bool
* 调用函数:
* 日期    :2011.09.22 Wangj
* 说明    ://dxl,2012.6.1
********************************************************************/
void MOD_InitCRC(void)
{
	/* Enable CRC Clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC, ENABLE);
	/* Reset CRC generator */
  	CRC->CR = CRC_CR_RESET;
}
/********************************************************************
* 名称    :  MOD_StopCRC
* 功能    :  停止CRC时钟
* 输入    :  无
* 输出    :  无
* 返回    :  bool
* 调用函数:
* 日期    :2011.09.22 Wangj
* 说明    :dxl,2012.6.1
********************************************************************/
void MOD_StopCRC(void)
{
	/* Disable CRC Clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC, DISABLE);
}
/********************************************************************
* 名称    :  MOD_CalcCRC
* 功能    :  通过STM32硬件CRC计算校验和
* 输入    :  *pBuffer 待计算的数据
              BufferLength 待计算数据长度
* 输出    :  无
* 返回    :  CRC值
* 调用函数:
* 日期    :2011.09.22 Wangj
* 说明    :
********************************************************************/
u32 MOD_CalcCRC(u8 *pBuffer, u32 BufferLength)
{
	u32  i = 0;
	u32  ulTmp;
	
	if (pBuffer != NULL) {
		for(i = 0; i < BufferLength; i++) {
    		ulTmp = *((uint8_t*)pBuffer + i);
			CRC->DR = ulTmp;			
		}
	}
		
	return(CRC->DR);
}

/*
********************************************************************************
*                               DEFIEN END
********************************************************************************
*/
#endif
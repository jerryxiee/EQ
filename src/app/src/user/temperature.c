/*
********************************************************************************
*
* Filename      : temperature.c
* Version       : V1.00
* Programmer(s) : miaoyahan
* @date         : 2014-12-4  
* 
********************************************************************************
*/


/*
********************************************************************************
*                               INCLUDES
********************************************************************************
*/
#include "include.h"

/*
********************************************************************************
*                               DEFIEN START
********************************************************************************
*/
#ifdef TEMPERATURE_H

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
static u8  Temp_16PinFlg;//16PIN接口接入温度传感器标志  1为有效
static u8  Temp_ValBuf[50];
static u16 Temp_SendNum=0;//流水号
static u16 Temp_ErrCnt;//错误计数器
static u16 Temp_ErrFlg;//错误标志 0:正常  1:异常
/*
********************************************************************************
*                          GLOBAL VARIABLES
********************************************************************************
*/

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
  * @brief  位置信息附加信息 0xe8
  * @param  指向数据地址
  * @retval 返回数据长度
  */
u16  Temperature_PosEx(u8 *pDst)
{
    u8 len;
    len = Temp_ValBuf[0];
    if((len == 0)||(Temp_ErrFlg))
    {
        return 0;
    }
    *pDst++ = 0xE8;//附加信息ID
    *pDst++ = len;//附加长度
    memcpy(pDst,Temp_ValBuf+1,len);
    return (len+2);
}
/**
  * @brief  温度初始化
  * @param  None
  * @retval None
  */
void Temperature_Init(void)
{
    static u8 flag;
    u8 tab[5];

    if(flag)
    {
        return;//已初始化完毕
    }
    flag = 1;

    if(EepromPram_ReadPram(E2_POS_PORT_ID, tab))
    {
        if(tab[1]&0x20)// bit5: 16PIN 接入为温度传感器
        {
            Temp_16PinFlg = 1;//
            NaviLcdPowerOn();
        }
    }
}
/**
  * @brief  接收外设返回的温度数据
  * @param  None
  * @retval None
  */
void Temperature_ComRec(u8 *pSrc, u16 len)
{
    TEMP_DATA  tempData;
    
    u8  *pVer=pSrc;
    u8   verf=0;
    u8   i;

    if(Temp_16PinFlg == 0)
    {
        return;//未接入传感器
    }
    if(*pSrc++ != 0xff)
    {
        return;//标示符错误
    }    
    if(*pSrc++ != 0x55)
    {
        return;//标示符错误
    }
    if(*pSrc++ != 0xaa)
    {
        return;//标示符错误
    }

    len -= 4;
    pVer = pSrc;
    for(i=3;i<len;i++)
    {
        verf ^= *pVer++;
    }
    
    if(verf != *pVer)
    {
        return;//校验出错
    }

    tempData.len  = *pSrc++;//长度
    tempData.len  = (tempData.len<<8)|*pSrc++;
    tempData.len -= 4;

    if(*pSrc++ != 0xf0)//设备类型  
    {
        return;//非温度采集器
    }
    
    pSrc++;//厂商标示
    
    Temp_ErrCnt=0;//归零
    Temp_ErrFlg=0;//正常
    Io_WriteSelfDefine2Bit(DEFINE_BIT_15,RESET);//温度传感器正常
    
    tempData.cmd  = *pSrc++;//命令字
    tempData.cmd  = (tempData.cmd<<8)|*pSrc++;

    memcpy(tempData.buf,pSrc,tempData.len);//数据

    if((tempData.cmd == 0x0110)||(tempData.cmd == 0x0111))//温度数据
    {
        Temp_ValBuf[0]=tempData.len-2;//传感器个数
        memcpy(Temp_ValBuf+1,tempData.buf+2,Temp_ValBuf[0]);//校验正确刷新温度值        
    }
    else if(tempData.cmd == 0x0112)//参数读写
    {
        Temperature_8900Send(&tempData);//透传到平台
    }
}
/**
  * @brief  向外设发送获取温度命令
  * @param  None
  * @retval None
  */
void Temperature_ComSend(TEMP_DATA *pTemp)
{
    u8  send_buf[50];
    u8  send_len;
    u8  verf;
    u8  i;

    send_len = 0;
    send_buf[send_len++]=0xff;
    send_buf[send_len++]=0x55;    
    send_buf[send_len++]=0xaa;  
    send_buf[send_len++]=0;                 
    send_buf[send_len++]=pTemp->len+4;      //长度
    send_buf[send_len++]=0xf0;              //设备类型  
    send_buf[send_len++]=0x00;              //厂商标识符
    send_buf[send_len++]=pTemp->cmd>>8;     //命令字
    send_buf[send_len++]=pTemp->cmd&0xff;

    memcpy(send_buf+send_len,pTemp->buf, pTemp->len);
    send_len += pTemp->len;

    for(i=3,verf=0;i<send_len;i++)
    {
        verf ^= send_buf[i];
    }
    send_buf[send_len++]=verf;               //校验
    send_buf[send_len++]=0x55;
    send_buf[send_len++]=0xaa;
    send_buf[send_len++]=0xff;

    Temp_SendNum++;                          //累计
    
    COM2_WriteBuff(send_buf,send_len);       //发送
    if(Temp_ErrCnt++ >= 3)
    {
        Temp_ErrFlg=1;//异常
        Io_WriteSelfDefine2Bit(DEFINE_BIT_15,SET);//温度传感器故障
    }
}
/**
  * @brief  接收平台下发透传命令
  * @param  None
  * @retval None
  */
void Temperature_DisposeRadioProtocol(u8 cmd, u8 *pSrc, u16 len)
{
    TEMP_DATA  tempData;

    if(cmd != 0x00)
    {
        return;//指令错误
    }

    tempData.cmd = 0x0112;
    tempData.len = len;
    memcpy(tempData.buf,pSrc,len);
    Temperature_ComSend(&tempData);//转发至温度采集器
}
/**
  * @brief  向平台发送透传命令
  * @param  None
  * @retval None
  */
void Temperature_8900Send(TEMP_DATA *pTemp)
{
    EIExpand_PotocolSendData(0xf8f0,pTemp->buf,pTemp->len);
}

/**
  * @brief  温度处理任务
  * @param  None
  * @retval None
  */
FunctionalState Temperature_TimeTask(void)
{
    static u16 timecnt;
    TEMP_DATA  tempData;

    //////////////////////////////////////////////////////////////////////初始化
    Temperature_Init();
    if(Temp_16PinFlg == 0)
    {
        return DISABLE;//无温度外设，关闭任务
    }
    
    //////////////////////////////////////////////////////////////////5S获取温度
    if(timecnt++ > (SYSTICK_1SECOND*5))
    {
        timecnt = 0;
        tempData.cmd    = 0x0111;
        tempData.buf[0] = Temp_SendNum>>8;
        tempData.buf[1] = Temp_SendNum&0xff;
        tempData.len    = 2;
        Temperature_ComSend(&tempData);
    }
    
    return ENABLE;
}

/*
********************************************************************************
*                 DEFIEN END
********************************************************************************
*/
#endif 

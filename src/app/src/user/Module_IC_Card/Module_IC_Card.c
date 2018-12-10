/*******************************************************************************
 * File Name:			ICCardBase.h 
 * Function Describe:	
 * Relate Module:		IC卡模块
 * Writer:				Joneming
 * Date:				2014-07-09
 * ReWriter:			
 * Date:				
 *******************************************************************************/

//***************包含文件*****************
#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "include.h"
/////////////////////////////////////
#define	RESULT_NULL             0xFF //0xFF IC卡无效结果
#define	RESULT_SUCCESS          0x00 //0x00	IC卡读卡成功	后续带128字节的卡内数据
#define	RESULT_ERROR_NOINSERT   0x01 //0x01	IC卡未插入
#define	RESULT_ERROR_NOSUPPORT  0x02 //0x02 IC卡无效卡或不支持的卡片
#define	RESULT_ERROR_FOMAT      0x03 //0x03	IC卡内数据格式错误
#define	RESULT_ERROR_VERIFY     0x04 //0x04	IC卡内数据校验错误
#define	RESULT_ERROR_PASSWORD   0x05 //0x05	IC 卡密码错误	预留, SLE4442返回剩余记数
////////////////////////////
enum
{
    ICCARD_STASUS_EMPTY,
    ICCARD_STASUS_IN,
    ICCARD_STASUS_OUT,
    ICCARD_STASUS_MAX,
};
//////////
enum
{
    ICCARD_VER_EMPTY,
    ICCARD_VER_USE_EXT,//使用外部独立的IC卡模块
    ICCARD_VER_USE_INT,//使用自身CPU通过I2C读IC卡
    ICCARD_VER_MAX,
};
enum
{
    ICCARD_READ_EMPTY,
    ICCARD_READ_PWR_ON,
    ICCARD_READ_24XX,//
    ICCARD_READ_SL4442,//
    ICCARD_READ_MAX,
};
////////////////

typedef struct
{
    unsigned char PID[2];//版本号
    unsigned char VID[2];//厂商编号
    unsigned char prestatus;//
    unsigned char count;//
    unsigned char result;//
    unsigned char readstep;//
    unsigned char useIntFlag;//
}ST_ICCARD_BASE;
///////////////
static ST_ICCARD_BASE s_stICCardBase;
                                  //版本号//厂商代码
const unsigned char c_ucPID_VID[]={0xA0,0xA1,0xB0,0xB1};
//////////////////////
enum 
{
    ICB_TIMER_TASK,                  //0 timer
    ICB_TIMER_TEST,                  //1 timer
    ICB_TIMERS_MAX
}E_ICCARDBASETIME;

static LZM_TIMER s_stICBTimer[ICB_TIMERS_MAX];
/********************************************************************
* 名称 : ICCardBase_VerifyCalcSum
* 功能 : 计算 数组的 校验(累加和)
********************************************************************/
u8 ICCardBase_VerifyCalcSum( u8 * buff, u16 length)
{     
    if((length == 0)||(buff == NULL))return 0;
    return Public_GetSumVerify(buff,length);
}

/********************************************************************
* 名称 : ICCardBase_VerifyCalcSum
* 功能 : 计算 数组的 校验(异或)
********************************************************************/
u8 ICCardBase_VerifyCalcXor( const u8 * InBuf , u16 InLength)
{       
    if((InLength == 0)||(InBuf == NULL))return 0;
    return Public_GetXorVerify(InBuf,InLength);;
}

/********************************************************************
* 名称 : ICCardBase_ExtractExdevice
* 功能 : 计算 数组的 校验和
********************************************************************/
u16 ICCardBase_ExtractExdevice( u8 *pInBuf, u16 InLength, ST_PROTOCOL_EXDEVICE *pPacket)
{
    u8 VerifyCode;
    u16 datalen;
    u8  *buffer;
    u16 Result = 0;    
    buffer = pInBuf;
    datalen = unTransMean(buffer, InLength);
    buffer = pInBuf+3;
    /****** 计算协议校验字  *****/
    VerifyCode = ICCardBase_VerifyCalcSum( buffer, datalen-3);
    if( pInBuf[0]== VerifyCode )
    {
        if(datalen>sizeof(ST_PROTOCOL_EXDEVICE))return 0;
        memcpy((u8 *)pPacket,pInBuf,datalen);
        if(EXDEVICE_TYPE_ICCARD != pPacket->Type)return 0;
        memcpy(s_stICCardBase.PID,pPacket->PID,4);
        Result = datalen;
    }    
    return (Result);    
}
/*************************************************************
** 函数名称: ICCardBase_CreatPackData
** 功能描述: 数据打包
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
u16 ICCardBase_CreatPackData( u8 cmd,  u8 * pInBuf , u16 InLength , u8 * pOutBuf ,unsigned char response)
{
    ST_PROTOCOL_EXDEVICE * pPacket;
    u16 datalen;
    u8 * pTemp;    
    pTemp = pOutBuf+1;
    pPacket = (ST_PROTOCOL_EXDEVICE *)pTemp;
    //////版本号、厂商编号/////////////////////////
    if(response)//应答
    {
        memcpy(pPacket->PID,s_stICCardBase.PID,4);
    }
    else
    { 
        memcpy(pPacket->PID,c_ucPID_VID,4);
    }
    //////////////////
    pPacket->Type = EXDEVICE_TYPE_ICCARD;
    /****  命令类型***/
    pPacket->Cmd = cmd;
    /****  用户数据 ***/
    if(InLength)
    {      
        memcpy(pPacket->Data, pInBuf,  InLength ) ;    
    }    
    /****  校验码 从厂商编号到用户数据依次累加的累加和，取低８位 ***/    
    pPacket->Verity = ICCardBase_VerifyCalcSum( (u8 *)&pPacket->VID[0], InLength+4 );//校验域（ 长度+命令+数据)
    //////////////////
    datalen=InLength+7;
    ///////////////////////
    datalen = TransMean( pTemp, datalen );
    pOutBuf[0]= 0x7E;
    pOutBuf[datalen+1]= 0x7E;
    return (datalen+2);
}  
extern u8	SwipCardCh;//刷卡通道  0 二汽读卡器  1  伊爱IC卡
/*************************************************************
** 函数名称: ICCardBase_PackDataToParse
** 功能描述: 打包IC卡数据并触发数据解析
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void ICCardBase_PackDataToParse( u8 cmd,  u8 * pIn , u16 InLength)
{
    unsigned short len;
    unsigned char buffer[160];
    len=ICCardBase_CreatPackData(cmd,pIn,InLength,buffer,0);
	SwipCardCh = 1;
    ICCard_ProtocolParse(buffer,len);
	SwipCardCh = 0;
}
/*************************************************************
** 函数名称: ICCard_PowerInit
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void ICCardBase_PowerInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    //================================================   
    /* 打开供电引脚 ,置高电平 IC_P clock */
    RCC_AHB1PeriphClockCmd(RCC_IC_CARD_POWER, ENABLE); 
    GPIO_InitStructure.GPIO_Pin = PIN_IC_CARD_POWER;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init( GPIO_IC_CARD_POWER, &GPIO_InitStructure);
}
/*************************************************************
** 函数名称: ICCardBase_CSInit
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void ICCardBase_CSInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;  
    /******************************ICCardBase_CS--->PD5***********************************/    
    RCC_AHB1PeriphClockCmd(RCC_ICCARD_CS , ENABLE);	       /* 打开GPIO时钟 */
    GPIO_InitStructure.GPIO_Pin = PIN_ICCARD_CS  ;    
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;        /* 快速模式 */     
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;  	         /* 输出/输入 */    
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;           /* 开漏 */    
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;             /* 上拉 */    
    GPIO_Init( GPIO_ICCARD_CS , &GPIO_InitStructure);
}
/*************************************************************
** 函数名称: ICCardBase_GetUseIntFlag
** 功能描述: 是否通过自身CPU通过I2C读IC卡
** 入口参数: 
** 出口参数: 
** 返回参数: 1:通过自身CPU通过I2C读IC卡,0:使用外部独立的IC卡模块
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char ICCardBase_GetUseIntFlag(void)
{
    return (s_stICCardBase.useIntFlag == ICCARD_VER_USE_INT)?1:0;
}
/*************************************************************
** 函数名称: ICCardBase_GetReadFlag
** 功能描述: 终端是否正在读IC卡
** 入口参数: 
** 出口参数: 
** 返回参数: 1:正在读,0空闲
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char ICCardBase_GetReadFlag(void)
{
    return (s_stICCardBase.readstep)?1:0;
}
/*************************************************************
** 函数名称: ICCardBase_ReadICCardFinish
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void ICCardBase_ReadICCardFinish(void)
{
    s_stICCardBase.readstep = ICCARD_READ_EMPTY;
    ICCARD_POWER_OFF();
    LZM_PublicKillTimer(&s_stICBTimer[ICB_TIMER_TEST]);
}
/*************************************************************
** 函数名称: ICCardBase_ReadICCardSL4442
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void ICCardBase_ReadICCardSL4442(void)
{
    unsigned char buffer[140];
    unsigned short datalen;    
    if(0==ICCardBase_GetReadFlag())return;
    ////////////////////////
    datalen = 128;
    if(SL4442_Buffer_Read(&buffer[1],datalen))
    {
        datalen++;
        buffer[0] = RESULT_SUCCESS;        
        s_stICCardBase.result = RESULT_SUCCESS;
        if(buffer[1]==0xff&&buffer[2]==0xff&&buffer[3]==0xff)
        {
            s_stICCardBase.result = RESULT_NULL;
        }
    }
    else
    {
        datalen = 1;
        buffer[0] = RESULT_ERROR_NOSUPPORT;
        s_stICCardBase.result = RESULT_ERROR_NOSUPPORT;        
    }
    ///////////////////
    ICCardBase_ReadICCardFinish();
    ///////////////////
    ICCardBase_PackDataToParse(0x41,buffer,datalen);    
}
/*************************************************************
** 函数名称: ICCardBase_ReadICCard24XX
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void ICCardBase_ReadICCard24XX(void)
{
    unsigned char buffer[140];
    unsigned short datalen;
    if(0==ICCardBase_GetReadFlag())return;
    /////////////////
    datalen = 128;
    if(EEPROM_24XX_HL_Buf_Read(0, &buffer[1],datalen))
    {
        ICCardBase_ReadICCardFinish();
        s_stICCardBase.result = RESULT_SUCCESS;
        buffer[0] = s_stICCardBase.result;
        datalen++;
        ICCardBase_PackDataToParse(0x41,buffer,datalen);
    }
    else
    {
        s_stICCardBase.readstep = ICCARD_READ_SL4442;
    }
}
/*************************************************************
** 函数名称: ICCardBase_DisposeICCardStatusIn
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void ICCardBase_DisposeICCardStatusIn(void)
{
    LZM_PublicSetOnceTimer(&s_stICBTimer[ICB_TIMER_TEST],PUBLICSECS(0.1),ICCardBase_DisposeICCardStatusIn);
    switch(s_stICCardBase.readstep)
    {
        case ICCARD_READ_PWR_ON:
            ICCARD_POWER_ON();
            s_stICCardBase.result = RESULT_NULL;
            s_stICCardBase.readstep = ICCARD_READ_24XX;
            break;
        case ICCARD_READ_24XX:
            ICCardBase_ReadICCard24XX();
            break;
        case ICCARD_READ_SL4442:
            ICCardBase_ReadICCardSL4442();
            break;
        default:
            ICCardBase_ReadICCardFinish();
            break;
    }    
}
/*************************************************************
** 函数名称: Card_DisposeICCardStatusInsert
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void ICCardBase_DisposeICCardStatusOut(void)
{
    unsigned char data[2];
    ///////////////
    ICCardBase_ReadICCardFinish();
    ////////////////
    if(RESULT_SUCCESS!=s_stICCardBase.result)return;
    s_stICCardBase.result = RESULT_NULL;
    data[0] = 0;
    ICCardBase_PackDataToParse(0x42,data,1);
}
/*************************************************************
** 函数名称: ICCardBase_CheckStatusChange
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void ICCardBase_CheckStatusChange(void) 
{
    if(ICCARD_DETECT_READ())//无卡
    {
        if(ICCARD_STASUS_OUT != s_stICCardBase.prestatus)
        {
            if(s_stICCardBase.count++ >5)
            {
                s_stICCardBase.count = 0;
                s_stICCardBase.prestatus = ICCARD_STASUS_OUT;                
                /////拔卡处理///////////
                ICCardBase_DisposeICCardStatusOut();               
            }
        }
        else
        {
            s_stICCardBase.count = 0;
        }
    }
    else//有卡
    {
        if(ICCARD_STASUS_IN != s_stICCardBase.prestatus)//
        {
            if(s_stICCardBase.count++ >5)
            {
                s_stICCardBase.count = 0;
                s_stICCardBase.prestatus = ICCARD_STASUS_IN;                 
                /////插卡处理///////////
                s_stICCardBase.readstep = ICCARD_READ_PWR_ON;
                ICCardBase_DisposeICCardStatusIn();
            }
        }
        else
        {
            s_stICCardBase.count = 0;
        }
    }
}

/*************************************************************
** 函数名称: ICCardBase_StartCheckStatus
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void ICCardBase_StartCheckStatus(void) 
{
    LZM_PublicSetCycTimer(&s_stICBTimer[ICB_TIMER_TASK],LZM_AT_ONCE,ICCardBase_CheckStatusChange);
}
/*************************************************************
** 函数名称: ICCardBase_ParameterInitialize
** 功能描述: IC卡参数初始化
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void ICCardBase_ParameterInitialize(void)
{
    ICCardBase_CSInit();   
    ////////////////
    LZM_PublicKillTimerAll(s_stICBTimer,ICB_TIMERS_MAX); 
    ////////////////
    memset(&s_stICCardBase,0,sizeof(s_stICCardBase));
    /////////////////////////
    memcpy(s_stICCardBase.PID,c_ucPID_VID,4);
    ////////////////////////
    ICCardBase_PowerInit(); 
    ////////////////////
    ICCard_ParamInit();
    /////////////////////
    s_stICCardBase.result = RESULT_NULL;
    s_stICCardBase.readstep = ICCARD_READ_EMPTY;
    s_stICCardBase.prestatus = ICCARD_STASUS_EMPTY;
    /////////////////////
    if(ICCARD_CS_READ())
    {
        ICCARD_M3_Init();
        SetCostDownVersionFlag();
        s_stICCardBase.useIntFlag = ICCARD_VER_USE_INT; 
        LZM_PublicSetOnceTimer(&s_stICBTimer[ICB_TIMER_TASK],PUBLICSECS(2),ICCardBase_StartCheckStatus);
    }
    else
    {
        CommICCardInit();
        ClearCostDownVersionFlag();
        s_stICCardBase.useIntFlag = ICCARD_VER_USE_EXT;
        /////////////////////
        ICCARD_POWER_ON();
    }
    /////////////////
    BMA250_ParamInitialize();
    ////////////////
    GpioOutInit(MIC_EN);//dxl,2014.5.27,恢复FD5原来的初始化用途（麦克风控制）
    //////////////////////
    GpioOutOn(MIC_EN);//打开麦克风
}
/*************************************************************
** 函数名称: ICCardBase_TimerTask
** 功能描述: 定时器处理函数
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void ICCardBase_TimerTask(void)
{
    //#if (ICCARD_JTD == ICCARD_SEL) 
    Task_IC_Card_JTB();
    //#else 
    LZM_PublicTimerHandler(s_stICBTimer,ICB_TIMERS_MAX);
	//#endif
}
/******************************************************************************
**                            End Of File
******************************************************************************/


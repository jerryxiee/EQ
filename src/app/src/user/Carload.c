/*******************************************************************************
 * File Name:			CarLoad.c 
 * Function Describe:	载重模块
 * Relate Module:		测试相关协议。
 * Writer:				Joneming
 * Date:				2013-05-20
 * ReWriter:			
 * Date:				
 *******************************************************************************/
#include "include.h"
/*******************************************************************************/
//////////cmd////////////////////////////
#define CARLOAD_COMM_CMD_QUERY_COM_WEIGHT           0x05	//综合重量
#define CARLOAD_COMM_CMD_QUERY_SENSOR_ABNORMAL      0x07	//0x05
#define CARLOAD_COMM_CMD_SERVER_SENSOR_CLEAR        0x71	//平台清零
#define CARLOAD_COMM_CMD_SENSOR_CLEAR               0x72	//终端清零
#define CARLOAD_COMM_CMD_CAR_STATUS                 0xC1	//车辆状态
#define CARLOAD_COMM_CMD_UPDATA                     0xC0	//升级
///////////////////////////////////

#define CARLOAD_RADIO_QUERY_SENSOR_ABNORMAL         0x00 //查询载重传感器故障
#define CARLOAD_RADIO_CLEAR_SENSOR_ABNORMAL         0x01 //清零载重传感器故障
#define CARLOAD_RADIO_FREIGHT_INFORMATION           0x02 //运货信息
#define CARLOAD_RADIO_DEVELOP_PORT                  0xF0 //开发接口
///////////////////////////////////////////////////////////////
#define CARLOAD_RADIO_CMD_MAX                       0xFF //最大命令
////////////////////////////////////////////////
//////////////////////////////////
#define CARLOAD_RX_BUFFER                           256//接收缓冲区
#define CARLOAD_TX_BUFFER                           256//发送缓冲区
////////////////////////
#define WEIGHT_OVERHEAD_INFO_ID                     0xE2 //附加信息ID
#define WEIGHT_OVERHEAD_INFO_LEN                    4  //附加信息长度

//#define WEIGHT_USER_STATUS_ID                       0xE1 //附加信息ID
//#define WEIGHT_USER_STATUS_LEN                      4  //附加信息长度

//#define DEFINE_BIT_19                  19 //超载报警位
//#define DEFINE_BIT_25            25 //传感器故障报警位
/*******************************************************************************/
/*******************************************************************************/
/////////////////////////////////
#define CARLOAD_USER_DATA_START         4
#define CARLOAD_USER_DATA_MIN_LEN       6
/////////////////////////
enum
{
    LEDPORT_BIT_LED     =0,     //条屏
    LEDPORT_BIT_TOPLIGHT,       //顶灯
    LEDPORT_BIT_PJA,            //评价器
    LEDPORT_BIT_LOAD_SENSOR,    //载重传感器
    LEDPORT_BIT_MAX             //最大值
}E_32PINLEDPORT;//表明32pin的LED接口接的外设类型,
////////////////////
typedef enum
{
    COMM_RECV_OK,             //接收正确
    COMM_RECV_ERR_START,      //开头标志不对
    COMM_RECV_ERR_TYPE,       //类型不对
    COMM_RECV_ERR_LEN,        //长度不对
    COMM_RECV_ERR_END,        //结束标志不对
    COMM_RECV_ERR_CMD,        //命令不对
    COMM_RECV_ERR_VERIFY,     //校验不对
    COMM_RECV_MAX             //
}CARLOAD_RECV;

enum
{
    CARLOAD_CMD_TYPE_READ=0,      //
    CARLOAD_CMD_TYPE_WRITE,       //
    CARLOAD_CMD_TYPE_MAX          //最大值
}E_CARLOAD_CMD_TYPE;//
////////////////////

typedef enum
{
    CARLOAD_EMPTY_STATE,             //空
    CARLOAD_CHECK_STATE,             //检测线路
    CARLOAD_MAX_STATE                //状态数目
}CARLOAD_STATE;
////////////////
enum 
{
    CL_TIMER_TASK,                  //0 timer
    CL_TIMER_TIME_OUT,              //1 timer
    CL_TIMER_SEND_DATA,             //2 timer
    CL_TIMER_CAR_STATUS,            //3 timer
    CL_TIMERS_MAX
}CARLOADTIME;

static LZM_TIMER s_stCLTimer[CL_TIMERS_MAX];
/////////////////////////////////////

typedef struct
{
    unsigned long userAlarmMask; //屏蔽自定义状态表
    unsigned long userAlarm;       //自定义状态表
    unsigned long maxweight;        //最大重量
    unsigned long sensorAbnormal;   //传感器异常
    unsigned long clearresult;      //传感器清零结果
    unsigned long totalWeight;      //最大重量    
    unsigned char status;           //状态
    unsigned char clearCnt;         //传感器清零次数
    unsigned char CheckCnt;         //检测用，计数器
    unsigned char overloadflag;     //超载标志
    unsigned char platformCmd;      //平台指令
    unsigned char recvCmd;          //接收命令
    unsigned char recvCmdEx;        //接收命令    
    unsigned char recvDataLen;      //接收数据长度
    unsigned char freightInfo;      //货运信息
    unsigned char sendcnt;          //发送次数
    unsigned char onlinestatus;     //在线状态
    unsigned char lcdTransFlag;     //lcd透传标志
    unsigned char contrecvtimeoutCnt; //连续接收超时次数
    unsigned char sendCmd;       //   
    unsigned char carStatusFlag;   //
    unsigned char loadSensorFlag;   // 
    unsigned char carStatusCnt;   //
    unsigned char parseData[CARLOAD_RX_BUFFER];
}T_CARLOAD_ATTRIB;

static T_CARLOAD_ATTRIB s_stCarLoadAttrib;

typedef struct
{
	unsigned char SendBuffer[CARLOAD_TX_BUFFER+1];
	unsigned short SendLen;   
	unsigned short SendCount;
}STSCARLOAD_TX;

static STSCARLOAD_TX s_stCarLoadTx;

/****************************************************/
/*                                                  */
/* Local Definitions and macros                     */
/*                                                  */
/****************************************************/
void CarLoad_ResendData(void);
unsigned char CarLoad_SendData(unsigned char *pBuffer, unsigned short BufferLen);
void Carload_SendCmdQueryTotalWeight(void);
void Carload_SendCmdQuerySensorAbnormal(void);
void Carload_SendCmdClearServerSensorAbnormal(void);
void CarLoad_SendDevelopPortResponse(void);
void CarLoad_SendClearSensorAbnormalResult(void);
void CarLoad_SendQuerySensorAbnormalResult(void);
void Carload_SendCmdCurCarStatus(void);
void CarLoad_DisposeLoadSensorTransToLcd(void);

/*************************************************************
** 函数名称: CarLoad_Get32PINLedPortInsertLoadSensorFlag
** 功能描述: 是否插入载重传感器
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char CarLoad_Get32PINLedPortInsertLoadSensorFlag(void)
{
    return s_stCarLoadAttrib.loadSensorFlag;
}
/*************************************************************
** 函数名称: CarLoad_PactAndSendData
** 功能描述: 对数据进行打包,并发送
** 入口参数: cmd:命令，CmdType:0是读,1是写,data:数据块首地址,datalen:长度		 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
ProtocolACK CarLoad_PactAndSendData(unsigned char cmd,unsigned char CmdType,unsigned char *data,unsigned short datalen)
{
    unsigned char result;
    unsigned short i;
    unsigned short len;
    unsigned short VerifyCode;  
    unsigned char TxBuffer[CARLOAD_TX_BUFFER];//发送
    ///////////////////////
    if(s_stCarLoadAttrib.contrecvtimeoutCnt>=5)return ACK_ERROR;
    ////////////////////////////////
    if(datalen+CARLOAD_USER_DATA_MIN_LEN>CARLOAD_TX_BUFFER)return ACK_ERROR;    
    i=0;
    ///////////////////////
    TxBuffer[i++]=0x69; //同步域
    TxBuffer[i++]=0x01;//地址域
    ///////////////////////////
    TxBuffer[i++]=0x42;//地址域
    //////////////////////////////////
    if(CARLOAD_CMD_TYPE_WRITE==CmdType)
    {
        TxBuffer[i++] = cmd|0x80;
    }
    else
    {
        TxBuffer[i++] = cmd;
    }
    s_stCarLoadAttrib.sendCmd = TxBuffer[i-1];
    /////////////////////////////////////////////
    len = i;
    for(i=0;i<datalen;i++)TxBuffer[len+i]=data[i];
    /////////////////////////////////////////////
    len  +=datalen;
    VerifyCode=CRC16(TxBuffer,len);
    /////////////
    TxBuffer[len++]=(VerifyCode>>8)& 0xff;
    TxBuffer[len++]= VerifyCode& 0xff;  
    /////////////////////////////////
    ///////////////////////////
    s_stCarLoadAttrib.CheckCnt = 0;
    s_stCarLoadAttrib.lcdTransFlag = 0;
    ///////////////////
    if(CarLoad_Get32PINLedPortInsertLoadSensorFlag())
    {
        result=CarLoad_SendData(TxBuffer,len);
        if(result)return ACK_OK;
    }
    return ACK_ERROR;
}
/*************************************************************
** 函数名称: CarLoad_PactAndSendDataEx
** 功能描述: 对数据进行打包,并发送
** 入口参数: cmd:命令，CmdType,data:数据块首地址,datalen:长度		 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void CarLoad_PactAndSendDataEx(unsigned char cmd,unsigned char *data,unsigned short datalen)
{
    CarLoad_PactAndSendData(cmd,CARLOAD_CMD_TYPE_READ,data,datalen);
}
/*************************************************************
** 函数名称: CarLoad_ResendData
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void CarLoad_ResendData(void)
{
    if(s_stCarLoadAttrib.contrecvtimeoutCnt>=5)
    {
        s_stCarLoadAttrib.CheckCnt = 0;
        s_stCarLoadAttrib.onlinestatus = 0;
        if(s_stCarLoadAttrib.carStatusFlag<2)s_stCarLoadAttrib.carStatusFlag=0xff;
        return;
    }
    //////////////////////////////
    s_stCarLoadAttrib.CheckCnt++;
    if(s_stCarLoadAttrib.CheckCnt < 5)
    {
        CarLoad_SendData(s_stCarLoadTx.SendBuffer,s_stCarLoadTx.SendLen);
    }
    else
    {
        s_stCarLoadAttrib.CheckCnt = 0;
        s_stCarLoadAttrib.onlinestatus = 0;
        if(s_stCarLoadAttrib.carStatusFlag<2)s_stCarLoadAttrib.carStatusFlag=0xff;
        LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_TIME_OUT],PUBLICSECS(30),Carload_SendCmdQueryTotalWeight);
    }
}
/*************************************************************
** 函数名称: CarLoad_SendData
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char CarLoad_SendData(unsigned char *pBuffer, unsigned short BufferLen)
{
    unsigned char result;
    s_stCarLoadTx.SendLen = BufferLen;
    if(BufferLen>CARLOAD_TX_BUFFER)return 0;
    memcpy(s_stCarLoadTx.SendBuffer,pBuffer,s_stCarLoadTx.SendLen);
    result=COM1_WriteBuff(pBuffer,BufferLen);
    if(result)
    {
        if(CARLOAD_COMM_CMD_QUERY_COM_WEIGHT == s_stCarLoadAttrib.sendCmd)
        {
            if(s_stCarLoadAttrib.contrecvtimeoutCnt<5)
            {
                s_stCarLoadAttrib.contrecvtimeoutCnt++;
            }
            else
            {
                return result;
            }
        }
    }
    if(0==LoadControllerUpdata_GetUpdataFlag())
    LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_TIME_OUT],PUBLICSECS(3),CarLoad_ResendData);
    return result;
}
/*************************************************************
** 函数名称: Carload_SendCmdQueryTotalWeight
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Carload_SendCmdQueryTotalWeight(void)
{
    unsigned char data[1];
    if(LoadControllerUpdata_GetUpdataFlag())return;
    CarLoad_PactAndSendDataEx(CARLOAD_COMM_CMD_QUERY_COM_WEIGHT,data,0);
    LZM_PublicKillTimer(&s_stCLTimer[CL_TIMER_CAR_STATUS]);
}

/*************************************************************
** 函数名称: Carload_SendCmdQuerySensorAbnormal
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Carload_SendCmdQuerySensorAbnormal(void)
{
    unsigned char data[1];
    CarLoad_PactAndSendDataEx(CARLOAD_COMM_CMD_QUERY_SENSOR_ABNORMAL,data,0);
}
/*************************************************************
** 函数名称: Carload_SendCmdClearServerSensorAbnormal
** 功能描述: 清除传感器
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Carload_SendCmdClearSensorAbnormal(void)
{
    unsigned char data[1];
    CarLoad_PactAndSendDataEx(CARLOAD_COMM_CMD_SENSOR_CLEAR,data,0);
}
/*************************************************************
** 函数名称: Carload_SendCmdClearServerSensorAbnormal
** 功能描述: 清除传感器
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Carload_SendCmdClearServerSensorAbnormal(void)
{
    unsigned char data[1];
    CarLoad_PactAndSendDataEx(CARLOAD_COMM_CMD_SERVER_SENSOR_CLEAR,data,0);
}
/*************************************************************
** 函数名称: Carload_SendCmdCurCarStatus
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Carload_SendCmdCurCarStatus(void)
{
    unsigned char data[10];
    unsigned char temp,len;
    unsigned short speed;
    if(LoadControllerUpdata_GetUpdataFlag())return;
    if(0==CarLoad_GetCurOnlineStatus())return;
    temp = 0;    
    //////////////////
    if(Io_ReadStatusBit(STATUS_BIT_ACC))
    PUBLIC_SETBIT(temp, 0);//acc
    //////////////////
    if(SpeedMonitor_GetCurRunStatus())//行驶状态
    PUBLIC_SETBIT(temp, 1);//
    data[0] = temp;
    /////////////////////
    speed = SpeedMonitor_GetCurSpeed();
    speed *= 10;
    ///////////////////////
    Public_ConvertShortToBuffer(speed, &data[1]);
    Public_ConvertNowTimeToBCDEx(&data[3]);    
    if(0xff == s_stCarLoadAttrib.carStatusFlag)
    {
        s_stCarLoadAttrib.carStatusFlag = 0x55;
        len = 9;
    }
    else
    if(0x55 == s_stCarLoadAttrib.carStatusFlag)
    {
        s_stCarLoadAttrib.carStatusFlag = 0xff;
        len = 3;
    }
    else
    if(0x01 == s_stCarLoadAttrib.carStatusFlag)
    {
        len = 9;
    }
    else
    {
        len = 3;
    }
    CarLoad_PactAndSendDataEx(CARLOAD_COMM_CMD_CAR_STATUS,data,len);
}
/*******************************************************************************
**  函数名称  : CarLoad_SetOverLoadAlarm
**  函数功能  : 设置超载报警位
**  输    入  : 无				
**  输    出  : 
**  全局变量  : 
**  调用函数  : 
**  中断资源  : 
**  备    注  :
*******************************************************************************/
void CarLoad_SetOverLoadAlarm(void)
{
    Io_WriteSelfDefine2Bit(DEFINE_BIT_19,SET);
}
/*******************************************************************************
**  函数名称  : CarLoad_ClearOverLoadAlarm
**  函数功能  : 清除超载报警位
**  输    入  : 无				
**  输    出  : 
**  全局变量  : 
**  调用函数  : 
**  中断资源  : 
**  备    注  :
*******************************************************************************/
void CarLoad_ClearOverLoadAlarm(void)
{
    Io_WriteSelfDefine2Bit(DEFINE_BIT_19,RESET);
}
/*******************************************************************************
**  函数名称  : CarLoad_SetSensorAbnormalAlarm
**  函数功能  : 设置传感器故障报警位
**  输    入  : 无				
**  输    出  : 
**  全局变量  : 
**  调用函数  : 
**  中断资源  : 
**  备    注  :
*******************************************************************************/
void CarLoad_SetSensorAbnormalAlarm(void)
{
    Io_WriteSelfDefine2Bit(DEFINE_BIT_25,SET); 
}
/*******************************************************************************
**  函数名称  : CarLoad_ClearOverLoadAlarm
**  函数功能  : 清除传感器故障报警位
**  输    入  : 无				
**  输    出  : 
**  全局变量  : 
**  调用函数  : 
**  中断资源  : 
**  备    注  :
*******************************************************************************/
void CarLoad_ClearSensorAbnormalAlarm(void)
{
    s_stCarLoadAttrib.sensorAbnormal = 0;
    Io_WriteSelfDefine2Bit(DEFINE_BIT_25,RESET);
}
/*******************************************************************************
**  函数名称  : CarLoad_CheckOverLoad
**  函数功能  : 检查是否超载
**  输    入  : 无				
**  输    出  : 
**  全局变量  : 
**  调用函数  : 
**  中断资源  : 
**  备    注  :
*******************************************************************************/
void CarLoad_CheckOverLoad(void)
{
    if(!PUBLIC_CHECKBIT(s_stCarLoadAttrib.totalWeight,31))
    {
        if(s_stCarLoadAttrib.totalWeight>=s_stCarLoadAttrib.maxweight) 
        {
            CarLoad_SetOverLoadAlarm();
            return;
        }
    }
    /////////////////////
    CarLoad_ClearOverLoadAlarm();   
    //////////////////////
}
/*******************************************************************************
**  函数名称  : CarLoad_DebugTest
**  函数功能  : 
**  输    入  : 无				
**  输    出  : 
**  全局变量  : 
**  调用函数  : 
**  中断资源  : 
**  备    注  :
*******************************************************************************/
void CarLoad_DebugTest(void)
{
    static unsigned long sulTmpCnt=0;
    sulTmpCnt++;
    if(!s_stCarLoadAttrib.totalWeight)
    s_stCarLoadAttrib.totalWeight =sulTmpCnt;
}
/*******************************************************************************
**  函数名称  : CarLoad_CheckIsUpdataSystem
**  函数功能  : 
**  输    入  : 无				
**  输    出  : 
**  全局变量  : 
**  调用函数  : 
**  中断资源  : 
**  备    注  :
*******************************************************************************/
void CarLoad_CheckIsUpdataSystem(void)
{
    if(LoadControllerUpdata_GetUpdataFlag())
    {
        LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_TASK],PUBLICSECS(10),CarLoad_CheckIsUpdataSystem);
    }
    else
    {
        LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_TASK],PUBLICSECS(3),Carload_SendCmdQueryTotalWeight);
    }
}
/*******************************************************************************
**  函数名称  : CarLoad_DisposeRecvDataResponse
**  函数功能  : 
**  输    入  : 无				
**  输    出  : 
**  全局变量  : 
**  调用函数  : 
**  中断资源  : 
**  备    注  :
*******************************************************************************/
void CarLoad_DisposeRecvDataResponse(void)
{
    unsigned char *pBuffer;
    unsigned char index;
    if(1==s_stCarLoadAttrib.lcdTransFlag)
    {
        CarLoad_DisposeLoadSensorTransToLcd();
        return;
    }
    ///////////////////////////
    pBuffer = &s_stCarLoadAttrib.parseData[CARLOAD_USER_DATA_START];
    switch(s_stCarLoadAttrib.recvCmd)
    {
        case CARLOAD_COMM_CMD_QUERY_COM_WEIGHT: 
            LZM_PublicKillTimer(&s_stCLTimer[CL_TIMER_CAR_STATUS]);
            //////////////////////////////////////////////
            s_stCarLoadAttrib.carStatusCnt = 0;
            index =0;
            s_stCarLoadAttrib.status = pBuffer[index++];
            if(PUBLIC_CHECKBIT(s_stCarLoadAttrib.status,2))//传感器异常
            {
                LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_TASK],PUBLICSECS(0.2),Carload_SendCmdQuerySensorAbnormal);
            }
            else///重量不稳定
            if(PUBLIC_CHECKBIT(s_stCarLoadAttrib.status,0))////重量不稳定
            {
                LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_TASK],PUBLICSECS(3),Carload_SendCmdQueryTotalWeight);
            }
            else///重量稳定
            {
                CarLoad_ClearSensorAbnormalAlarm();
                s_stCarLoadAttrib.totalWeight = Public_ConvertBufferToLong(&pBuffer[index]);
                CarLoad_CheckOverLoad();               
                LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_CAR_STATUS],PUBLICSECS(0.5),Carload_SendCmdCurCarStatus);
                LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_TASK],PUBLICSECS(6),Carload_SendCmdQueryTotalWeight);
            }
            break;
        case CARLOAD_COMM_CMD_QUERY_SENSOR_ABNORMAL:
            ////////////////////
            LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_CAR_STATUS],PUBLICSECS(0.5),Carload_SendCmdCurCarStatus);
            s_stCarLoadAttrib.sensorAbnormal = Public_ConvertBufferToLong(pBuffer);
            ///////////////////
            if(0 == s_stCarLoadAttrib.sensorAbnormal)//
            {
                CarLoad_ClearSensorAbnormalAlarm();
            }
            else
            {
                CarLoad_SetSensorAbnormalAlarm();
                CarLoad_ClearOverLoadAlarm();
            }
            ////////////////////////
            if(0==LZM_PublicGetTimerEnable(&s_stCLTimer[CL_TIMER_TASK]))
            {
                LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_TASK],PUBLICSECS(4),Carload_SendCmdQueryTotalWeight);
            }
            break;
        case CARLOAD_COMM_CMD_SERVER_SENSOR_CLEAR:
            LZM_PublicKillTimer(&s_stCLTimer[CL_TIMER_CAR_STATUS]);
            s_stCarLoadAttrib.clearresult = Public_ConvertBufferToLong(pBuffer);
            /////////////////////////////////////
            if(CARLOAD_RADIO_CLEAR_SENSOR_ABNORMAL == s_stCarLoadAttrib.platformCmd)///
            {
                s_stCarLoadAttrib.platformCmd = CARLOAD_RADIO_CMD_MAX;
                LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_SEND_DATA],PUBLICSECS(1.5),CarLoad_SendClearSensorAbnormalResult);
            }
            ////////////////////////////////////            
            s_stCarLoadAttrib.clearCnt = 0;
            LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_TASK],PUBLICSECS(1),Carload_SendCmdQueryTotalWeight);
            break;
        case CARLOAD_COMM_CMD_SENSOR_CLEAR:
            LZM_PublicKillTimer(&s_stCLTimer[CL_TIMER_CAR_STATUS]);
            s_stCarLoadAttrib.clearresult = Public_ConvertBufferToLong(pBuffer);
            /////////////////////////////////////
            LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_SEND_DATA],LZM_TIME_BASE,NaviLcd_SendCarLoadClearSensorResult);
            ////////////////////////////////////            
            s_stCarLoadAttrib.clearCnt = 0;
            LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_TASK],PUBLICSECS(1),Carload_SendCmdQueryTotalWeight);
            break;
        case CARLOAD_COMM_CMD_CAR_STATUS:
            if(0xff == s_stCarLoadAttrib.carStatusFlag)
            {
                s_stCarLoadAttrib.carStatusFlag = 0;
            }
            else
            if(0x55== s_stCarLoadAttrib.carStatusFlag)
            {
                s_stCarLoadAttrib.carStatusFlag = 1;
            }
            s_stCarLoadAttrib.carStatusCnt++;
            if(s_stCarLoadAttrib.carStatusCnt>4)
            {
                LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_TASK],PUBLICSECS(1),Carload_SendCmdQueryTotalWeight);
            }
            else
            LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_CAR_STATUS],PUBLICSECS(2.5),Carload_SendCmdCurCarStatus);
            break;
        case CARLOAD_COMM_CMD_UPDATA:
            LZM_PublicKillTimer(&s_stCLTimer[CL_TIMER_SEND_DATA]);
            LZM_PublicKillTimer(&s_stCLTimer[CL_TIMER_CAR_STATUS]);
            LoadControllerUpdata_DisposeUpdataResponse(pBuffer,s_stCarLoadAttrib.recvDataLen-CARLOAD_USER_DATA_MIN_LEN);
            LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_TASK],PUBLICSECS(10),CarLoad_CheckIsUpdataSystem);
            break;
        default:
            if(CARLOAD_RADIO_DEVELOP_PORT == s_stCarLoadAttrib.platformCmd)///
            {
                LZM_PublicKillTimer(&s_stCLTimer[CL_TIMER_CAR_STATUS]);
                LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_TASK],PUBLICSECS(3),Carload_SendCmdQueryTotalWeight);
                s_stCarLoadAttrib.platformCmd = CARLOAD_RADIO_CMD_MAX;
                LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_SEND_DATA],PUBLICSECS(1.5),CarLoad_SendDevelopPortResponse);
            }
            else
            {
                CarLoad_DisposeLoadSensorTransToLcd();
            }
            break;
    }
}
/*************************************************************
** 函数名称: CarLoad_CommProtocolParse
** 功能描述: 载重传感器协议解释
** 入口参数: ParseBuffer数据首地址,usDataLenght数据长度
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned char CarLoad_CommProtocolParse(unsigned char *ParseBuffer,unsigned short usDataLenght)
{
    unsigned short datalen,i,crc,crc1;
    unsigned char startflag;
    unsigned char *pBuf;
    //////////////
    if(!CarLoad_Get32PINLedPortInsertLoadSensorFlag())return COMM_RECV_ERR_START;
    ///////////////////////////////////////
    if(usDataLenght<CARLOAD_USER_DATA_MIN_LEN)return COMM_RECV_ERR_START;
    datalen=usDataLenght;
    pBuf=ParseBuffer;
    ///////////////////////////////////////
    startflag=0;
    for(i=0; i<datalen; i++)
    {
        if(0x51== pBuf[i]&&0x01== pBuf[i+1])
        {
            pBuf=ParseBuffer+i;
            startflag =1;
            datalen -=i;
            break;
        }
    }    
    if(0 == startflag)return COMM_RECV_ERR_START;//开头
    if(0xC2 != pBuf[2])return COMM_RECV_ERR_START;//开头
    if(datalen < CARLOAD_USER_DATA_MIN_LEN)return COMM_RECV_ERR_START;
    crc=CRC16(pBuf,datalen-2);
    crc1=(pBuf[datalen-2]<<8)+pBuf[datalen-1];
    if(crc!=crc1)return COMM_RECV_ERR_VERIFY;
    s_stCarLoadAttrib.recvCmd = pBuf[3]; 
    s_stCarLoadAttrib.recvCmdEx =(pBuf[3]& 0x7f);
    s_stCarLoadAttrib.recvDataLen = datalen;
    s_stCarLoadAttrib.onlinestatus = 1;
    s_stCarLoadAttrib.contrecvtimeoutCnt = 0;
    memcpy(s_stCarLoadAttrib.parseData,pBuf,s_stCarLoadAttrib.recvDataLen);
    LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_TIME_OUT],LZM_AT_ONCE,CarLoad_DisposeRecvDataResponse);
    ///////////////////////////
    return COMM_RECV_OK;       
}
/*************************************************************
** 函数名称: Carload_GetCurSensorAbnormalValue
** 功能描述: 取得传感器异常值
** 入口参数: 无
** 出口参数: 无
** 返回参数: 每一位对应一路传感器，当该位为1表示该路传感器故障
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned long Carload_GetCurSensorAbnormalValue(void)
{
    return s_stCarLoadAttrib.sensorAbnormal;
}
/*************************************************************
** 函数名称: Carload_GetClearSensorResult
** 功能描述: 取得传感器清零结果
** 入口参数: 无
** 出口参数: 无
** 返回参数: 每一位对应一路传感器，当该位为1表示该路传感器故障
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned long Carload_GetClearSensorResult(void)
{
    return s_stCarLoadAttrib.clearresult;
}
/*************************************************************
** 函数名称: Carload_GetCurTotalWeight
** 功能描述: 取得最大重量
** 入口参数: 无
** 出口参数: 无
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned long Carload_GetCurTotalWeight(void)
{
    return s_stCarLoadAttrib.totalWeight;
}
/*************************************************************
** 函数名称: Carload_GetSensorAbnormalFlag
** 功能描述: 取得传感器是否异常标志
** 入口参数: 无
** 出口参数: 无
** 返回参数: 0:正常; 1:异常
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Carload_GetSensorAbnormalFlag(void)
{
    return PUBLIC_CHECKBIT(s_stCarLoadAttrib.status,2);
}
/*************************************************************
** 函数名称: CarLoad_GetCurOverLoadFlag
** 功能描述: 取得是否超载标志
** 入口参数: 无
** 出口参数: 无
** 返回参数: 0:正常; 1:超载
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char CarLoad_GetCurOverLoadFlag(void)
{
    return Io_ReadSelfDefine2Bit(DEFINE_BIT_19);
}
/*************************************************************OK
** 函数名称: CarLoad_UpdatePram
** 功能描述: 更新参数
** 入口参数:  
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void CarLoad_UpdatePram(void)
{
    unsigned short ledPort=0;
    unsigned char buffer[10];
    if(EepromPram_ReadPram(E2_LOAD_MAX_ID,buffer))
    {
        s_stCarLoadAttrib.maxweight = Public_ConvertBufferToLong(buffer);
    }
    else
    {
        s_stCarLoadAttrib.maxweight = 0xffffffff;
    }
    ////////////////////
    if(EepromPram_ReadPram(E2_LED_PORT_ID, buffer))
    {
        ledPort = Public_ConvertBufferToShort(buffer);
    }
    ////////////////////////
    s_stCarLoadAttrib.loadSensorFlag = PUBLIC_CHECKBIT_EX(ledPort,LEDPORT_BIT_LOAD_SENSOR);
}

/*************************************************************OK
** 函数名称: CarLoad_ParamInitialize
** 功能描述: 初始化
** 入口参数:  
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void CarLoad_ParamInitialize(void)
{ 
    LZM_PublicKillTimerAll(s_stCLTimer,CL_TIMERS_MAX);
    s_stCarLoadAttrib.userAlarm = 0;
    s_stCarLoadAttrib.onlinestatus =0;
    s_stCarLoadAttrib.lcdTransFlag=0;
    s_stCarLoadAttrib.contrecvtimeoutCnt = 0;
    s_stCarLoadAttrib.carStatusFlag=0xff;
    s_stCarLoadAttrib.platformCmd = CARLOAD_RADIO_CMD_MAX;
    s_stCarLoadAttrib.loadSensorFlag = 0;
    CarLoad_UpdatePram();    
    SetTimerTask(TIME_CARLOAD, LZM_TIME_BASE);    
    if(!CarLoad_Get32PINLedPortInsertLoadSensorFlag())return;
    LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_TASK],PUBLICSECS(30),Carload_SendCmdQueryTotalWeight);
}
/*******************************************************************************
** 函数名称: CarLoad_GetCurWeightSubjoinInfo
** 功能描述: 取得当前重量附加信息包(包括附加信息ID、长度、附加信息体)
** 入口参数: 需要保存重量附加信息包的首地址
** 出口参数: 
** 返回参数: 重量附加信息包的总长度
*******************************************************************************/
unsigned char CarLoad_GetCurWeightSubjoinInfo(unsigned char *data)
{
    unsigned char buffer[10]={0};
    unsigned char i;
    if(!CarLoad_Get32PINLedPortInsertLoadSensorFlag())return 0;
    i = 0;
    buffer[i++] = WEIGHT_OVERHEAD_INFO_ID;
    buffer[i++] = WEIGHT_OVERHEAD_INFO_LEN;
    Public_ConvertLongToBuffer(s_stCarLoadAttrib.totalWeight,&buffer[i]);
    //////////////////////////
    i += WEIGHT_OVERHEAD_INFO_LEN;
    ///////////////
    memcpy(data,buffer,i);
    /////////////////
    return i;
}
/*************************************************************
** 函数名称: CarLoad_DisposeRadioProtocol
** 功能描述: 载重透传协议解析
** 入口参数: cmd命令,pBuffer透传消息首地址，BufferLen透传消息长度
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void CarLoad_DisposeRadioProtocol(unsigned char cmd,unsigned char *pBuffer,unsigned short BufferLen)
{    
    switch(cmd)
    {
        case CARLOAD_RADIO_QUERY_SENSOR_ABNORMAL:
            LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_SEND_DATA],PUBLICSECS(1.5),CarLoad_SendQuerySensorAbnormalResult);
            break;
        case CARLOAD_RADIO_CLEAR_SENSOR_ABNORMAL:
            LZM_PublicKillTimer(&s_stCLTimer[CL_TIMER_CAR_STATUS]);
            s_stCarLoadAttrib.platformCmd = cmd;            
            Carload_SendCmdClearServerSensorAbnormal();
            LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_TASK],PUBLICSECS(8),Carload_SendCmdQueryTotalWeight);
            break;
        case CARLOAD_RADIO_DEVELOP_PORT:
            LZM_PublicKillTimer(&s_stCLTimer[CL_TIMER_CAR_STATUS]);
            s_stCarLoadAttrib.platformCmd = cmd;
            CarLoad_SendData(pBuffer,BufferLen);
            LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_TASK],PUBLICSECS(8),Carload_SendCmdQueryTotalWeight);
            break;
        default:
            break;
    }    
}
/*************************************************************
** 函数名称: CarLoad_OriginalDataUpTrans
** 功能描述: 载重数据上传
** 入口参数: msgID消息ID,pBuffer消息体内容，BufferLen消息体长度
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
ProtocolACK CarLoad_OriginalDataUpTrans(unsigned char msgID,unsigned char *pBuffer,unsigned short BufferLen)
{
    unsigned short EIcmd;
    EIcmd = (EIEXPAND_PROTOCOL_FUN_CARLOAD<<8);
    EIcmd |= msgID;
    //////////////////////////
    return EIExpand_PotocolSendData(EIcmd,pBuffer,BufferLen);
}
/*************************************************************
** 函数名称: CarLoad_SendQuerySensorAbnormalResult
** 功能描述: 平台查询传感器故障结上传
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void CarLoad_SendQuerySensorAbnormalResult(void)
{
    unsigned char data[10];
    Public_ConvertLongToBuffer(s_stCarLoadAttrib.sensorAbnormal, data);
    //////////////////////////
    if(ACK_OK != CarLoad_OriginalDataUpTrans(CARLOAD_RADIO_QUERY_SENSOR_ABNORMAL,data,4))
    {
        if(s_stCarLoadAttrib.sendcnt++<4)
        {
            LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_SEND_DATA],PUBLICSECS(0.2),CarLoad_SendQuerySensorAbnormalResult);
            return;
        }
    }
    s_stCarLoadAttrib.sendcnt = 0;
}
/*************************************************************
** 函数名称: CarLoad_SendClearSensorAbnormalResult
** 功能描述: 平台清除传感器故障结果上传
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void CarLoad_SendClearSensorAbnormalResult(void)
{
    unsigned char data[10];
    Public_ConvertLongToBuffer(s_stCarLoadAttrib.clearresult, data);
    //////////////////////////
    if(ACK_OK != CarLoad_OriginalDataUpTrans(CARLOAD_RADIO_CLEAR_SENSOR_ABNORMAL,data,4))
    {
        if(s_stCarLoadAttrib.sendcnt++<4)
        {
            LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_SEND_DATA],PUBLICSECS(0.2),CarLoad_SendClearSensorAbnormalResult);
            return;
        }
    }
    s_stCarLoadAttrib.sendcnt = 0;
}
/*************************************************************
** 函数名称: CarLoad_SendFreightInformation
** 功能描述: 载重上传运货信息
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void CarLoad_SendFreightInformationToServiceEx(void)
{
    unsigned char data[2];
    data[0]=s_stCarLoadAttrib.freightInfo;
    if(ACK_OK !=CarLoad_OriginalDataUpTrans(CARLOAD_RADIO_FREIGHT_INFORMATION,data,1))
    {
        if(s_stCarLoadAttrib.sendcnt++<4)
        {
            LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_SEND_DATA],PUBLICSECS(0.2),CarLoad_SendFreightInformationToServiceEx);
            return;
        }
    }
    s_stCarLoadAttrib.sendcnt = 0;
}
/*************************************************************
** 函数名称: CarLoad_SendFreightInformation
** 功能描述: 载重上传运货信息
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void CarLoad_SendFreightInformationToService(unsigned char flag)
{
    s_stCarLoadAttrib.freightInfo=flag;
    CarLoad_SendFreightInformationToServiceEx();
}
/*************************************************************
** 函数名称: CarLoad_SendDevelopPortResponse
** 功能描述: 载重开发数据应答上传
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void CarLoad_SendDevelopPortResponse(void)
{
    if(ACK_OK !=CarLoad_OriginalDataUpTrans(CARLOAD_RADIO_DEVELOP_PORT,s_stCarLoadAttrib.parseData,s_stCarLoadAttrib.recvDataLen))
    {
        if(s_stCarLoadAttrib.sendcnt++<4)
        {
            LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_SEND_DATA],PUBLICSECS(0.2),CarLoad_SendDevelopPortResponse);
            return;
        }
    }
    s_stCarLoadAttrib.sendcnt = 0;
}

/*******************************************************************************
**  函数名称  : CarLoad_TimeTask
**  函数功能  : 时间任务调度接口函数
**  输    入  : 无				
**  输    出  : 任务调度状态:  使能或禁止
**  全局变量  : 
**  调用函数  : 
**  中断资源  : 
**  备    注  :
*******************************************************************************/
FunctionalState CarLoad_TimeTask(void) 
{
    LZM_PublicTimerHandler(s_stCLTimer,CL_TIMERS_MAX);
    return ENABLE;
}
/*************************************************************OK
** 函数名称: CarLoad_GetCurOnlineStatus
** 功能描述: 在线标志
** 入口参数:  
** 出口参数: 
** 返回参数: 1:在线;0不在线
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned char CarLoad_GetCurOnlineStatus(void)
{ 
    return (1==s_stCarLoadAttrib.onlinestatus)?1:0;
}
/*************************************************************
** 函数名称: CarLoad_DisposeLcdTransToLoadSensor
** 功能描述: 处理Lcd透传给载重传感器数据
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void CarLoad_DisposeLcdTransToLoadSensor(unsigned char *data,unsigned short len)
{
    if(!CarLoad_Get32PINLedPortInsertLoadSensorFlag())return;
    s_stCarLoadAttrib.lcdTransFlag = 1;
    CarLoad_SendData(data,len);
    LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_TASK],PUBLICSECS(8),Carload_SendCmdQueryTotalWeight);
}
/*************************************************************
** 函数名称: CarLoad_DisposeLoadSensorTransToLcd
** 功能描述: 处理载重传感器透传给Lcd数据
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void CarLoad_DisposeLoadSensorTransToLcd(void)
{
    s_stCarLoadAttrib.lcdTransFlag = 0;
    NaviLcd_DisponseLoadSensorTransToLcd(s_stCarLoadAttrib.parseData,s_stCarLoadAttrib.recvDataLen);
    LZM_PublicSetOnceTimer(&s_stCLTimer[CL_TIMER_TASK],PUBLICSECS(0.5),Carload_SendCmdQueryTotalWeight);
}

/*******************************************************************************
 *                             end of module
 *******************************************************************************/


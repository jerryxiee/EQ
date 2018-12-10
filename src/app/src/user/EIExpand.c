/*******************************************************************************
 * File Name:			EIExpand.c 
 * Function Describe:	EI扩展协议模块
 * Relate Module:		测试相关协议。
 * Writer:				Joneming
 * Date:				2013-07-16
 * ReWriter:			
 * Date:				
 *******************************************************************************/
#include "include.h"
/*******************************************************************************/
////////////////////////////////////////////////////////////////
#define EIEXPAND_PROTOCOL_UDID          0x01//伊爱扩展协议中的识别码(固定值,0x01)
#define EIEXPAND_PROTOCOL_VERSION       0x01//伊爱扩展协议中的版本号(当前版本0x01)
///////////////////////////////////////////////
#define EIEXPAND_PROTOCOL_HEAD_LEN      0x06//伊爱扩展协议头长度
////////////////////////////////////////////
//////////////////////////////////////
#define EIEXPAND_PROTOCOL_FUN_CMD       0xF000//按功能分类命令
///////////////////////////////////////
/*******************************************************************************/
/*******************************************************************************/
#define PROTOCOL_CONTENT_MAX_LEN        900	//最多可发送
//************伊爱扩展协议帧格式****************
typedef struct
{
    unsigned char UDID;         //识别码
    unsigned char Version;      //版本号
    unsigned short EICmd;       //子协议号
    unsigned short ProLen;      //子协议消息体长度
    unsigned char ProContent[PROTOCOL_CONTENT_MAX_LEN];//子协议消息体内容
}STEIEXPRO_FRAME; 

typedef struct
{
    unsigned short datalen;      //长度
    unsigned char resendCnt;      //
    unsigned char buffer[PROTOCOL_CONTENT_MAX_LEN+EIEXPAND_PROTOCOL_HEAD_LEN];//子协议消息体内容
}STEIEXPRO_TX; 

static STEIEXPRO_TX s_stEIExpro_Tx;
/****************************************************/
/*                                                  */
/* Local Definitions and macros                     */
/*                                                  */
/****************************************************/
/*************************************************************
** 函数名称: EIExpand_DisposeFunctionProtocol
** 功能描述: 伊爱扩展协议处理功能协议
** 入口参数:  
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned char EIExpand_DisposeFunctionProtocol(unsigned short EICmd,unsigned char *pBuffer,unsigned short datalen)
{
    unsigned char AckResult=0;
    unsigned char fun;
    unsigned char cmd;
    fun = EICmd>>8;
    cmd = EICmd;
    switch(fun)
    {
        case EIEXPAND_PROTOCOL_FUN_TAXI:
            {
                #ifdef TEXI_SERVICE                
                unsigned short msgID;
                msgID =*(pBuffer+0)<<8;
                msgID |=*(pBuffer+1);
                TexiService_SetSendType(1);
                TexiService_DisposeRadioProtocol(msgID,pBuffer+2,datalen-2);                
                #endif
            }
            break;
        case EIEXPAND_PROTOCOL_FUN_LED:
            {
                #ifdef TEXI_SERVICE
                CommLed_SetSendType(0);
                TexiService_ComLedTransmitDataToLed(pBuffer,datalen,RadioProtocol_GetCurRecvSerialNum());                
                #endif
            }
            break;
        case EIEXPAND_PROTOCOL_FUN_CARLOAD:
            #ifdef __CARLOAD_H
            CarLoad_DisposeRadioProtocol(cmd,pBuffer,datalen);
            #endif
            break; 
        case EIEXPAND_PROTOCOL_FUN_TEMP:
            #ifdef TEMPERATURE_H
            Temperature_DisposeRadioProtocol(cmd,pBuffer,datalen);
            #endif
            break;
		case EIEXPAND_PROTOCOL_FUN_OBD:
			#ifdef ACCELERATION_H
			Acceleration_Turn_DisposeRadioProtocol(cmd,pBuffer,datalen);
            #endif
			break;  
        case EIEXPAND_PROTOCOL_FUN_ACCELERATION:
            #ifdef ACCELERATION_H
            Acceleration_DisposeRadioProtocol(cmd,pBuffer,datalen);
            #endif
            break;
        default:
            break;
    }
    ////////////////
    cmd = cmd ;
    return AckResult;
}
/*************************************************************
** 函数名称: EIExpand_DisposeProtocol
** 功能描述: 伊爱扩展协议处理其它指令
** 入口参数:  
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned char EIExpand_DisposeProtocol(unsigned short EICmd,unsigned char *pBuffer,unsigned short datalen)
{
    unsigned char AckResult=0; 
    switch(EICmd)
    {
        case EIEXPAND_CMD_ACCIDENT_REMIND:
            #ifdef __ACCIDENT_H_
            AckResult=Accident_DisposeEIExpandProtocol(pBuffer,datalen);
            #endif
            break;
        case EIEXPAND_CMD_MODIFY_AREA_VOICE:
            #ifdef __AREAMANAGE_H_
            AckResult=Area_DisposeEIExpandProtocol(pBuffer,datalen);
            #endif
            break;
        case EIEXPAND_CMD_OIL_WEAR_CALIB://油耗标定
            #ifdef __OILWEARCALIB_H
            OilWear_DisposeEIExpandProtocol(pBuffer,datalen);
            #endif
            break;
        case EIEXPAND_CMD_SET_DRIVER_INFO://设置驾驶员
            #ifdef EI_MSG_SET_DRIVER_INFO 
            AckResult=Messege_DisposeEIExpandProtocolForDriverInfo(pBuffer,datalen);
            #endif
            break;
        default:
            break;
    }
    return AckResult;
}
/*************************************************************
** 函数名称: EIExpand_RadioProtocolParse
** 功能描述: 伊爱扩展协议解释
** 入口参数: pBuffer伊爱扩展协议数据首地址, datalen伊爱扩展协议数据总长度
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned char EIExpand_RadioProtocolParse(unsigned char *pBuffer,unsigned short datalen)
{
    unsigned char AckResult=0;
    unsigned short len;
    unsigned char *p;    
    STEIEXPRO_FRAME stEIProFrame ={0};     
    if(datalen < EIEXPAND_PROTOCOL_HEAD_LEN)return 2;//长度过小
    if(datalen>PROTOCOL_CONTENT_MAX_LEN)return 2;//长度过大
    p = pBuffer;
    len = 0;
    stEIProFrame.UDID = p[len++];
    stEIProFrame.Version = p[len++];
    stEIProFrame.EICmd = p[len++] << 8;
    stEIProFrame.EICmd |= p[len++];
    stEIProFrame.ProLen = p[len++] << 8;
    stEIProFrame.ProLen |= p[len++];
    if(EIEXPAND_PROTOCOL_UDID != stEIProFrame.UDID)return 2;//识别码不对、直接返回
    if(EIEXPAND_PROTOCOL_VERSION != stEIProFrame.Version)return 2;//版本号不对、直接返回
    if(stEIProFrame.ProLen +len != datalen)return 2;//长度不对、直接返回
    if(stEIProFrame.ProLen)////////////
    {
       memcpy((unsigned char *)&stEIProFrame.ProContent,&p[len],stEIProFrame.ProLen);
    }    
    //////////////////////////
    if(stEIProFrame.EICmd>=EIEXPAND_PROTOCOL_FUN_CMD)
    {
        AckResult = EIExpand_DisposeFunctionProtocol(stEIProFrame.EICmd,stEIProFrame.ProContent,stEIProFrame.ProLen);
    }
    else
    {
       AckResult = EIExpand_DisposeProtocol(stEIProFrame.EICmd,stEIProFrame.ProContent,stEIProFrame.ProLen);
    }
    return AckResult;
}

/*************************************************************
** 函数名称: EIExpand_PotocolPackData
** 功能描述: 按
** 入口参数: srcdat子协议内容首地址,cmd:子协议号,srclen子协议内容长度;
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned short EIExpand_PotocolPackData(unsigned char *des,unsigned short EICmd,const unsigned char * srcdat,  unsigned short srclen)
{
    unsigned short len;
    unsigned char *p;
    p = des;
    len = 0;    
    p[len++] = EIEXPAND_PROTOCOL_UDID;
    p[len++] = EIEXPAND_PROTOCOL_VERSION;
    p[len++] = EICmd >>8;
    p[len++] = EICmd & 0xff;
    p[len++] = srclen >>8;
    p[len++] = srclen & 0xff;   
    ////////////////////
    if(srclen)
    {
        memcpy(&p[len],srcdat,srclen);
    }
    len += srclen;
    return len;
}
/*************************************************************
** 函数名称: EIExpand_PotocolResendData
** 功能描述: 发送伊爱扩展协议到平台
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void EIExpand_PotocolResendData(void)
{
    ProtocolACK ack;
    u8 channel = CHANNEL_DATA_1|CHANNEL_DATA_2;
    if(0==s_stEIExpro_Tx.resendCnt)return;
    if(s_stEIExpro_Tx.resendCnt++<4)
    {
        ack=RadioProtocol_OriginalDataUpTrans(channel,EIEXPAND_PROTOCOL_TRAN_TYPE,s_stEIExpro_Tx.buffer,s_stEIExpro_Tx.datalen);
        if(ACK_OK != ack)
        {
            Public_SetEIExpandTimer(EIExpand_PotocolResendData,PUBLICSECS(2.5));
            return;
        }
    }
    ////////////////
    s_stEIExpro_Tx.resendCnt=0;
}
/*************************************************************
** 函数名称: EIExpand_PotocolSendData
** 功能描述: 发送伊爱扩展协议到平台
** 入口参数: EICmd:子协议号,srcdat子协议消息体首地址,srclen子协议消息体长度;
** 出口参数: 
** 返回参数: 成功,返回ACK_OK;失败返回ACK_ERROR;
** 全局变量: 无
** 调用模块:
*************************************************************/
ProtocolACK EIExpand_PotocolSendData(unsigned short EICmd,unsigned char * srcdat,unsigned short srclen)
{
    ProtocolACK ack;
    s_stEIExpro_Tx.resendCnt = 0;
    u8 channel = CHANNEL_DATA_1|CHANNEL_DATA_2;
    s_stEIExpro_Tx.datalen = EIExpand_PotocolPackData(s_stEIExpro_Tx.buffer,EICmd,srcdat,srclen);
    ack=RadioProtocol_OriginalDataUpTrans(channel,EIEXPAND_PROTOCOL_TRAN_TYPE,s_stEIExpro_Tx.buffer,s_stEIExpro_Tx.datalen);
    if(ACK_OK==ack)
    {
        return ACK_OK;
    }
    else
    {
        s_stEIExpro_Tx.resendCnt = 1;
        Public_SetEIExpandTimer(EIExpand_PotocolResendData,PUBLICSECS(2.5));
        return ack;
    }
}
/*******************************************************************************
 *                             end of module
 *******************************************************************************/


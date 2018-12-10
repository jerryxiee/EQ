/************************************************************************
//程序名称：Can_ErQi.c
//功能：该模块实现广州二汽CAN数据上传功能。
//版本号：V0.1
//版权说明：版权属于深圳市伊爱高新技术开发有限公司
//开发人：dxl
//开发时间：2014.12
//备注：CAN总线的连接关系是车辆CAN总线先接到第3方的CAN盒子（如威帝或欧科佳），然后再接入伊爱CAN盒子，最后接入EGS701GB终端。
//只有伊爱CAN盒子与EGS701GB的连接是串口连接，其它的连接是CAN总线；伊爱CAN盒子与EGS701GB连接的串口为USART2
//伊爱CAN盒子的作用是：1.对CAN数据进行必要的滤波，滤除不必要的CAN ID或数据相同的CAN ID；
//2.把第3方的CAN数据转换成伊爱的统一格式，这样使得同一台终端可以接入多家的CAN盒子；
//3.所有发送的CAN数据分成两类，一类是模拟量，另一类是数字量。模拟量是定时发送给终端，数字量是变化了就发送给终端。
//版本记录：版本记录需包含版本、修改人、修改时间、修改原因、修改简要说明
//V0.1 初始版本  董显林
//V0.2 原来是收到一帧CAN盒子发的数据就透传一帧到平台，现在改为定时发送数据帧到平台，定时时间为10秒。该需求为广州伊爱要求（主要是担心流量超标）
*************************************************************************/

/********************文件包含*************************/
#include <stdio.h>
#include <string.h>

#include "Can_ErQi.h"
#include "Usart2.h"
#include "GPIOControl.h"
#include "modem_lib.h"
#include "modem_app_com.h"
#include "Report.h"
#include "RadioProtocol.h"
#include "other.h"
#include "EepromPram.h"
#include "Public.h"
#include "NaviLcd.h"


/********************本地变量*************************/
static u8 CanErQiUploadBuffer[CAN_ERQI_UPLOAD_BUFFER_SIZE];
static u8 CanErQiCmdSendBuffer[CAN_ERQI_CMD_BUFFER_SIZE];
static u8 CanErQiSetRtcAckFlag = 0;//设置RTC应答标志，1为收到CAN模块应答
static u8 CanErQiDigitalBuffer[CAN_ERQI_UPLOAD_BUFFER_SIZE];//存储CAN模块发送的OF03类型数据
static u8 CanErQiAnalogBuffer[CAN_ERQI_UPLOAD_BUFFER_SIZE];//存储CAN模块发送的OF04类型数据
static u16 CanErQiDigitalBufferLen = 0;
static u16 CanErQiAnalogBufferLen = 0;
/********************全局变量*************************/


/********************外部变量*************************/


/********************本地函数声明*********************/
static void Can_ErQi_ParseCmd(u8 *pBuffer, u16 BufferLen);
static void Can_ErQi_Upload(u16 CanId,u8 *pBuffer, u16 BufferLen);

/********************函数定义*************************/
/**************************************************************************
//函数名：Can_ErQi_Init
//功能：CAN接入初始化
//输入：无
//输出：无
//返回值：无
//备注：无
***************************************************************************/
void Can_ErQi_Init(void)
{
	u8 Buffer[5];
	u8 BufferLen;
	u16 temp;

	BufferLen = EepromPram_ReadPram(E2_POS_PORT_ID,Buffer);
	if(E2_POS_PORT_ID_LEN == BufferLen)
	{
		temp = Public_ConvertBufferToShort(Buffer);
	}
	if(0x02 == (temp&0x02))
	{
		NaviLcdPowerOn();
	}
}
/**************************************************************************
//函数名：Can_ErQi_TimeTask
//功能：CAN定时任务
//输入：无
//输出：无
//返回值：无
//备注：实现对CAN盒子校时，每次终端重启，第1次导航后对CAN盒子校时，调度时间1秒1次
***************************************************************************/
FunctionalState Can_ErQi_TimeTask(void)
{
	static u8 TimingCount = 0;
        static u8 UploadCount = 0;
	u8 flag = 0;
	u8 Buffer[10] = {0};
	u16 year;
	TIME_T time;


	flag = Io_ReadStatusBit(STATUS_BIT_NAVIGATION);//上电导航后校时1次
	if(1 == flag)
	{
		TimingCount++;
		if(TimingCount > 3)
		{
			TimingCount = 0;
			if(0 == CanErQiSetRtcAckFlag)
			{
				RTC_GetCurTime(&time);
				year = 2000+time.year;
				Buffer[0] = year&0xff;
				Buffer[1] = (year&0xff00) >> 8;
				Buffer[2] = time.month;
				Buffer[3] = time.day;
				Buffer[4] = time.hour;
				Buffer[5] = time.min;
				Buffer[6] = time.sec;
				Can_ErQi_SendCmd(0x61,Buffer,7);
			}
			else
			{
                          
			}
		}
	}
	else
	{
		TimingCount = 0;
	}
        
        //定时向平台发送CAN数据（0F03，OF04）
        UploadCount++;
        if(UploadCount == 10)//发送OF03到平台
        {
                if(CanErQiDigitalBufferLen > 0)
                {
                        Can_ErQi_Upload(0x0F03,CanErQiDigitalBuffer,CanErQiDigitalBufferLen);
                        CanErQiDigitalBufferLen = 0;
                }
        }
        else if(UploadCount >= 11)//发送OF04到平台
        {
                if(CanErQiAnalogBufferLen > 0)
                {
                        Can_ErQi_Upload(0x0F04,CanErQiAnalogBuffer,CanErQiAnalogBufferLen);
                        CanErQiAnalogBufferLen = 0;
                }
                UploadCount = 0;
        }
	return ENABLE;
}
/**************************************************************************
//函数名：Can_ErQi_Parse
//功能：CAN盒子协议解析
//输入：无
//输出：无
//返回值：无
//备注：无
***************************************************************************/
void Can_ErQi_Parse(u8 *pBuffer, u16 BufferLen)
{
	u8 *p = NULL;
	u16 i = 0;
	u16 j = 0;
	u16 k = 0;
	u16 length = 0;
	u8 VerifyCode = 0;

	if(NULL != p)
	{
		return ;
	}

	p = pBuffer;
	for(i=0; i<BufferLen-1; i++)
	{
		if((0x55 == *(p+i))&&(0xaa == *(p+i+1)))
		{
			j = i;
			length = (*(p+j+2) << 8)|(*(p+j+3));
			i += (length+4);
			if((length < CAN_ERQI_UPLOAD_BUFFER_SIZE)&&(length < BufferLen))
			{
				p = pBuffer+j+2;
				VerifyCode = 0;
				for(k=0; k<length+1; k++)
				{
					VerifyCode += *(p+k);
				}
				if(VerifyCode == *(p+k))
				{
					Can_ErQi_ParseCmd(pBuffer+j+4,length);
				}
			}
		}
	}
}
/**************************************************************************
//函数名：Can_ErQi_ParseCmd
//功能：CAN盒子命令解析
//输入：pBuffer:指向命令字+参数的首地址；BufferLen:命令字+参数的长度
//输出：无
//返回值：无
//备注：无
***************************************************************************/
static void Can_ErQi_ParseCmd(u8 *pBuffer, u16 BufferLen)
{
	u8 Command;
	u8 Buffer[1] = {0};
	
	Command = *pBuffer;
	switch(Command)
	{
		case 0x40://读取CAN模块版本信息，不理会CAN应答信息
		{
			break;
		}
		case 0x60://参数设置，不理会CAN应答信息
		{
			break;
		}
		case 0x61://更新CAN模块RTC
		{
			CanErQiSetRtcAckFlag = 1;
			break;
		}
		case 0x6E://获取CAN模拟量列表
		{
			//Can_ErQi_Upload(0x0F04,pBuffer,BufferLen);收到一条数据上传给平台
                        if(BufferLen < CAN_ERQI_UPLOAD_BUFFER_SIZE)
                        {
                                memcpy(CanErQiAnalogBuffer,pBuffer,BufferLen);
                                CanErQiAnalogBufferLen = BufferLen;
                        }
                        
			break;
		}
		case 0x6F://获取CAN数字量列表
		{
			//Can_ErQi_Upload(0x0F03,pBuffer,BufferLen);收到一条数据上传给平台
                        if(BufferLen < CAN_ERQI_UPLOAD_BUFFER_SIZE)
                        {
                                memcpy(CanErQiDigitalBuffer,pBuffer,BufferLen);
                                CanErQiDigitalBufferLen = BufferLen;
                        }
                  
			Can_ErQi_SendCmd(0xA5, Buffer, 1);//收到开关量数据需应答CAN模块
			break;
		}
		default:break;
	}
}
/**************************************************************************
//函数名：Can_ErQi_SendCmd
//功能：发送命令到CAN盒子
//输入：Cmd:命令字；pBuffer:指向参数（即命令字后面跟的数据）首地址；BufferLen:参数长度 
//输出：无
//返回值：0:成功；1:失败；2:长度错误
//备注：无
***************************************************************************/
u8 Can_ErQi_SendCmd(u8 Cmd, u8 *pBuffer, u16 BufferLen)
{
	u16 i;
	u8 VerifyCode = 0;
	u16 length;
	
	if(BufferLen > (CAN_ERQI_CMD_BUFFER_SIZE-3))
	{
		return 2;
	}
	length = BufferLen+2;
	CanErQiCmdSendBuffer[0] = 0x55;
	CanErQiCmdSendBuffer[1] = 0xaa;
	CanErQiCmdSendBuffer[2] = (length&0xff00) >> 8;
	CanErQiCmdSendBuffer[3] = length&0xff;//长度包括命令+参数+校验
	CanErQiCmdSendBuffer[4] = Cmd;
	memcpy(CanErQiCmdSendBuffer+5,pBuffer,BufferLen);
	VerifyCode = 0;
	for(i=0; i<BufferLen+3; i++)//校验是数据长度+命令+参数
	{
		VerifyCode += CanErQiCmdSendBuffer[2+i];
	}
	CanErQiCmdSendBuffer[5+BufferLen] = VerifyCode;
	
	if(0 == COM2_WriteBuff(CanErQiCmdSendBuffer,BufferLen+6))
	{
		return 0;
	}
	else
	{
		return 1;
	}
		
}
/**************************************************************************
//函数名：Can_ErQi_Upload
//功能：发送CAN数据到平台
//输入：pBuffer:指向命令字+参数的首地址；BuffeLen:命令字+参数组成的缓冲长度
//输出：无
//返回值：无
//备注：CanId:CAN模块自定义消息ID，0x0F03为开关量列表，0x0F04为模拟量列表
//0x0F05为参数，0x0F06为RTC，这两个没有做上传
***************************************************************************/
static void Can_ErQi_Upload(u16 CanId,u8 *pBuffer, u16 BufferLen)
{
	u8 Buffer[40];
	u8 length;
	//TIME_T time;
	//u16 temp;
	u8 channel = CHANNEL_DATA_1|CHANNEL_DATA_2;

	switch(CanId)
	{
		case 0x0F03:
		{
			if(*pBuffer == 0x6F)
			{
				CanErQiUploadBuffer[0] = 0x0f;
				CanErQiUploadBuffer[1] = 0x03;
				length = Report_GetPositionBasicInfo(Buffer);//基本位置信息固定为28字节
				memcpy(CanErQiUploadBuffer+2,Buffer,length);
				//RTC_GetCurTime(&time);
				//temp = 2000;
				//temp += time.year;
				length += 2;//前面2字节
				CanErQiUploadBuffer[length++] = '0';//原协议是ASCII[7]，应该有问题，现在填充'0'
				CanErQiUploadBuffer[length++] = '0';
				CanErQiUploadBuffer[length++] = '0';
				CanErQiUploadBuffer[length++] = '0';
				CanErQiUploadBuffer[length++] = '0';
				CanErQiUploadBuffer[length++] = '0';
				CanErQiUploadBuffer[length++] = '0';
				memcpy(CanErQiUploadBuffer+length,pBuffer,BufferLen);
				length += BufferLen;
				RadioProtocol_OriginalDataUpTrans(channel,0xF1,CanErQiUploadBuffer,length);
			}
			break;
		}
		case 0x0F04:
		{
			if(*pBuffer == 0x6E)
			{
				CanErQiUploadBuffer[0] = 0x0f;
				CanErQiUploadBuffer[1] = 0x04;
				length = Report_GetPositionBasicInfo(Buffer);//基本位置信息固定为28字节
				memcpy(CanErQiUploadBuffer+2,Buffer,length);
				//RTC_GetCurTime(&time);
				//temp = 2000;
				//temp += time.year;
				length += 2;//前面2字节
				CanErQiUploadBuffer[length++] = '0';//原协议是ASCII[7]，应该有问题，现在填充'0'
				CanErQiUploadBuffer[length++] = '0';
				CanErQiUploadBuffer[length++] = '0';
				CanErQiUploadBuffer[length++] = '0';
				CanErQiUploadBuffer[length++] = '0';
				CanErQiUploadBuffer[length++] = '0';
				CanErQiUploadBuffer[length++] = '0';
				memcpy(CanErQiUploadBuffer+length,pBuffer,BufferLen);
				length += BufferLen;
				RadioProtocol_OriginalDataUpTrans(channel,0xF1,CanErQiUploadBuffer,length);
			}
			break;
		}
		case 0x0F05://暂不支持
		{
			break;
		}
		case 0x0F06://暂不支持
		{
			break;
		}
		default:break;
	}
}






















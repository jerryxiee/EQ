/********************************************************************
//版权说明	:Shenzhen E-EYE Co. Ltd., Copyright 2009- All Rights Reserved.
//文件名称	:LoadControllerUpdata.c		
//功能		:载重控制器固件升级
//版本号	:
//开发人	:dxl 
//开发时间	:2014.6
//修改者	:
//修改时间	:
//修改简要说明	:
//备注		:载重控制器与EGS701GB的DB9串口对接
***********************************************************************/
/*************************文件包含***********************/
#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "include.h"
/*************************外部变量***********************/
/*************************全局变量***********************/


/*************************本地变量***********************/
static u8 LoadControllerCmdAckFlag = 0;//载重控制器命令应答标志,1为已收到应答,0为未收到
static u32 UpdataFileSize = 0;
static u8  UpdataBuffer[LOAD_CONTROLLER_TX_BUFFER_SIZE] = {0};
static u32 UpdataCurrentPacket = 0;//升级当前包序号,从1开始
static u32 UpdataTotalPacket = 0;//升级总包数
static u8  UpdataFileAdditionalInformation[23] = {0};//升级文件附加信息,22字节内容,2字节公司标识,8字节产品标识,2字节PCB版本,6字节固件版本,4字节CRC校验码
static u8 LoaderControllerUpdataFlag = 0;//载重控制器升级标志,1为正在升级,0为不在升级,正在升级时请不要往接口发送其它指令,只发送升级指令
/*************************函数定义***********************/
/*********************************************************************
//函数名称	:LoaderControllerUpdata_TimeTask(void)
//功能		:载重控制器升级任务
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:当终端下载完升级文件后会判断是否为载重控制器固件升级，若是则开启该任务，0.3秒调度1次,
*********************************************************************/
FunctionalState  LoaderControllerUpdata_TimeTask(void)
{
    static u8 UpdataState = 0;//当前升级状态
    static u8 NoLoaderControllerAckCount = 0;//无应答计数,连续5次无载重控制器应答,则关闭任务
    static u8 LoaderControllerSendErrorCount = 0;//发送错误计数，连续30次发送不成功,则关闭任务
    static u16 TimeCount = 0;//时间计数,进入该任务后,若持续了半小时还没有结束,则强制结束任务
    static u8 UpdataResultFlag = 0;//升级结果标志,0为失败,1为成功。
    static u8 PacketTimeDelay = 0;//包延时,2秒还没收到应答将会重发上一包,最多重发5次,如果仍然没有应答则升级失败
    static u16 LastestPacketLen = 0;//最后一包的长度
    
    u8  Buffer[6] = {0};
    u8  sum = 0;
    u8  i = 0;
    u32 Address = 0;
    u16 length = 0;
    u8  PramLen = 0;
    
    TimeCount++;//0.3秒增加1
    if(TimeCount >= 3000)//15分钟后不管升级成功与否都关闭任务
    {
         TimeCount = 0;
         UpdataState = 4;
    }
    if(0 == UpdataState)//第一阶段：获取升级信息
    {
        E2prom_ReadByte(E2_PROGRAM_UPDATA_LENGTH_ID_ADDR, Buffer, 5);
        for(i=0; i<4; i++)
        {
            sum += Buffer[i];
        }
        if(sum == Buffer[4])
        {
            //读取长度
            UpdataFileSize = 0;
            UpdataFileSize |= Buffer[0] << 24;
            UpdataFileSize |= Buffer[1] << 16;
            UpdataFileSize |= Buffer[2] << 8;
            UpdataFileSize |= Buffer[3];
            UpdataCurrentPacket = 1;
            UpdataTotalPacket = UpdataFileSize/UPDATA_PACKET_SIZE;//每一包200字节
            if(0 != UpdataFileSize%UPDATA_PACKET_SIZE)
            {
                UpdataTotalPacket++;
                LastestPacketLen = UpdataFileSize%UPDATA_PACKET_SIZE;
            }
            else
            {
                LastestPacketLen = UPDATA_PACKET_SIZE;
            }
            //读取附加信息
            Address = UPDATE_BASE_ADDRESS+UpdataFileSize-22;
            sFLASH_ReadBuffer(UpdataFileAdditionalInformation, Address, 22);
            if((UpdataFileAdditionalInformation[0] == 'E')&&(UpdataFileAdditionalInformation[1] == 'I'))
            {
                UpdataState = 1;
                LoaderControllerUpdataFlag = 1;
            }
            else
            {
                UpdataState = 4;//发送升级结果到平台
            }
              
        }
        else
        {
            UpdataState = 4;//发送升级结果到平台
        }
    }
    else if(1 == UpdataState)//第二阶段：发送升级包到载重控制器
    {
        if((1 == UpdataCurrentPacket)&&(UpdataTotalPacket >= UpdataCurrentPacket))
        {
              length = 0;
              //总包数
              UpdataBuffer[0] = (UpdataTotalPacket&0xff00) >> 8;
              UpdataBuffer[1] = UpdataTotalPacket&0xff;
              //包序号,从1开始
              UpdataBuffer[2] = (UpdataCurrentPacket&0xff00) >> 8;
              UpdataBuffer[3] = UpdataCurrentPacket&0xff;
              //升级类型,固定为0x09
              UpdataBuffer[4] = 0x09;
              //制造商ID
              PramLen = EepromPram_ReadPram(E2_MANUFACTURE_ID, UpdataBuffer+5);
              if(E2_MANUFACTURE_LEN== PramLen)
              {
              
              }
              else
              {
                    UpdataBuffer[5] = '7';
                    UpdataBuffer[6] = '0';
                    UpdataBuffer[7] = '1';
                    UpdataBuffer[8] = '0';
                    UpdataBuffer[9] = '8';
              }
              //版本号长度,固定为0
              UpdataBuffer[10] = 6;
              //版本号
              memcpy(UpdataBuffer+11,UpdataFileAdditionalInformation+12,6);
              //升级包数据长度,包含了附加信息
              UpdataBuffer[17] = (UpdataFileSize&0xff000000) >> 24;
              UpdataBuffer[18] = (UpdataFileSize&0xff0000) >> 16;
              UpdataBuffer[19] = (UpdataFileSize&0xff00) >> 8;
              UpdataBuffer[20] = UpdataFileSize&0xff;
              length += 21;
              Address = UPDATE_BASE_ADDRESS+(UpdataCurrentPacket-1)*UPDATA_PACKET_SIZE;
              sFLASH_ReadBuffer(UpdataBuffer+length, Address, UPDATA_PACKET_SIZE);
              length += UPDATA_PACKET_SIZE;
              if(ACK_OK == CarLoad_PactAndSendData(0x40,1,UpdataBuffer,length))
              {
                    UpdataState = 2;
                    PacketTimeDelay = 0;
                    LoaderControllerSendErrorCount = 0;
              }
              else
              {
                    LoaderControllerSendErrorCount++;
                    if(LoaderControllerSendErrorCount >= 30)
                    {
                        UpdataResultFlag = 0;//升级失败
                        UpdataState = 3;//发送升级结果到平台
                    }
              }
              
        }
        else if((UpdataCurrentPacket > 1)&&(UpdataTotalPacket >= UpdataCurrentPacket))
        {
              length = 0;
              //总包数
              UpdataBuffer[0] = (UpdataTotalPacket&0xff00) >> 8;
              UpdataBuffer[1] = UpdataTotalPacket&0xff;
              //包序号,从1开始
              UpdataBuffer[2] = (UpdataCurrentPacket&0xff00) >> 8;
              UpdataBuffer[3] = UpdataCurrentPacket&0xff;
              length += 4;
              Address = UPDATE_BASE_ADDRESS+(UpdataCurrentPacket-1)*UPDATA_PACKET_SIZE;
              sFLASH_ReadBuffer(UpdataBuffer+length, Address, UPDATA_PACKET_SIZE);
              if(UpdataCurrentPacket == UpdataTotalPacket)
              {
                    length += LastestPacketLen;
              }
              else
              {
                    length += UPDATA_PACKET_SIZE;
              }
              if(ACK_OK == CarLoad_PactAndSendData(0x40,1,UpdataBuffer,length))
              {
                    UpdataState = 2;
                    PacketTimeDelay = 0;
                    LoaderControllerSendErrorCount = 0;
              }
              else
              {
                    LoaderControllerSendErrorCount++;
                    if(LoaderControllerSendErrorCount >= 30)
                    {
                        UpdataResultFlag = 0;//升级失败
                        UpdataState = 3;//发送升级结果到平台
                    }
              }
        }
        else
        {

            UpdataResultFlag = 0;//升级失败
            UpdataState = 3;//发送升级结果到平台
        }
    }
    else if(2 == UpdataState)//第三阶段：检查载重控制器应答
    {
         if(1 == LoadControllerUpdata_GetCmdAckFlag())
         {
              UpdataState = 1;
              UpdataCurrentPacket++;//下一包
              LoadControllerUpdata_ClrCmdAckFlag();
              NoLoaderControllerAckCount = 0;
              if(UpdataCurrentPacket > UpdataTotalPacket)
              {
                    UpdataTotalPacket = 0;
                    UpdataResultFlag = 1;//升级成功
                    UpdataState = 3;//发送升级结果到平台
              }
         }
         else
         {
            PacketTimeDelay++;
            if(PacketTimeDelay >= 34)//10秒延时时间到,重发
            {
                UpdataState = 1;
                PacketTimeDelay = 0;
                NoLoaderControllerAckCount++;
                if(NoLoaderControllerAckCount >= 3)
                {
                    NoLoaderControllerAckCount = 0;
                    UpdataResultFlag = 0;//升级失败
                    UpdataState = 3;//发送升级结果到平台
                }
            }
         }
    }
    else if(3 == UpdataState)//第四阶段：保存升级结果通知标志,
    {
        if(1 == UpdataResultFlag)
        {
            Buffer[0] = 2;
        }
        else
        {
            Buffer[0] = 3;
        }
        FRAM_BufferWrite(FRAM_FIRMWARE_UPDATA_FLAG_ADDR,Buffer,1);
        NVIC_SystemReset();//升级后都要重启，因为升级程序存储区与盲区存储区是共用的，为安全起见升级后不管是否成功，都必须重启
    }
    else//出错,关闭任务
    {
         UpdataFileSize = 0;
         UpdataCurrentPacket = 0;
         UpdataTotalPacket = 0;
         UpdataState = 0;
         PacketTimeDelay = 0;
         TimeCount = 0;
         LoaderControllerUpdataFlag = 0;
         Buffer[0] = 3;
         FRAM_BufferWrite(FRAM_FIRMWARE_UPDATA_FLAG_ADDR,Buffer,1);
         NVIC_SystemReset();//升级后都要重启，因为升级程序存储区与盲区存储区是共用的，为安全起见升级后不管是否成功，都必须重启
        
    }
    return ENABLE;
}
/*********************************************************************
//函数名称	:LoadControllerUpdata_SendData
//功能		:发送数据给载重控制器
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:载重控制器与EGS701GB通过DB9串口相连
//		:CmdType:0是读,1是写
*********************************************************************/
/*
ProtocolACK LoadControllerUpdata_SendData(u8 Cmd,u8 CmdType,u8 *pData,u16 DataLen)
{
    u8  Buffer[LOAD_CONTROLLER_TX_BUFFER_SIZE] = {0};
    u16 BufferLen = 0;
    u16 CrcCode = 0;
    
    if(DataLen > (LOAD_CONTROLLER_TX_BUFFER_SIZE-6))//长度超出
    {
        return ACK_ERROR;
    }
    //同步域
    Buffer[0] = 0x69;
    //地址域
    Buffer[1] = 0x01;
    //控制域
    Buffer[2] = 0x42;
    //命令域
    if(0 == CmdType)//0是读
    {
        Buffer[3] = Cmd;
    }
    else//1是写
    {
        Buffer[3] = Cmd | 0x80;
    }
    //数据域
    memcpy(Buffer+4,pData,DataLen);
    
    //校验域
    CrcCode = CRC16(Buffer, DataLen+4);
    Buffer[4+DataLen] = (CrcCode&0xff00) >> 8;
    Buffer[5+DataLen] = CrcCode&0x00ff;
    
    //发送
    BufferLen = DataLen+6;
    
    if(1 == Recorder_SendData(Buffer, BufferLen))//目前返回1表示成功,注意后期可能会改
    {
       return ACK_OK;
    }
    else
    {
        return ACK_ERROR;
    }
    
}
*/
/*********************************************************************
//函数名称	:LoadControllerUpdata_ProtocolParse
//功能		:对载重控制器发送过来的数据进行解析
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:载重控制器与EGS701GB通过DB9串口相连
//		:
*********************************************************************/
/*
ProtocolACK LoadControllerUpdata_ProtocolParse(u8 *pData,u16 DataLen)
{
    u8  *p = NULL;
    u16 i = 0;
    u16 j = 0;
    u16 CrcCode = 0;
    u16 CrcHi = 0;
    u16 CrcLow = 0;
    
    
    //检查数据的正确性
    if((NULL == p)&&(DataLen >= 6))
    {
        p = pData;
        for(i=0; i<DataLen; i++)
        {
            if((0x51 == *(p+i))&&(0x01 == *(p+i+1)))//检查同步域和地址域
            {
                j = i;
                break;
            }
        }
        if(DataLen == i)//没有找到帧头
        {
            return ACK_ERROR;
        }
        else
        {
            if(j > 0)
            {
                p += j;//指针指向帧头
            }
            CrcCode = CRC16(p, DataLen-j-2);
            CrcHi = (CrcCode&0xff00) >> 8;
            CrcLow = CrcCode&0xff;
            if((CrcHi == *(p+DataLen-2))&&(CrcLow == *(p+DataLen-1)))
            {
               if((*(p+3) == 0xC0)&&(*(p+4) == 0x09)&&(*(p+5) == 0x00))//是远程升级指令应答且回复成功
               {
                    //校验通过,置位应答标志
                    LoadControllerUpdata_SetCmdAckFlag();
               }
            }
            else
            {
                return ACK_ERROR;
            }
        }
    }
    else
    {
        return ACK_ERROR;
    }
    return ACK_OK;
}
*/
/*********************************************************************
//函数名称	:LoadControllerUpdata_DisposeUpdataResponse
//功能		:处理升级命令应答(校验已通过)
//输入		:pBuffer:应答数据域首地址,应答数据域长度
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void LoadControllerUpdata_DisposeUpdataResponse(unsigned char *pBuffer,unsigned short datalen)
{
    //升级类型
    if(0==pBuffer[1])//升级结果
    LoadControllerUpdata_SetCmdAckFlag();
}
/*********************************************************************
//函数名称	:LoadControllerUpdata_GetUpdataFlag
//功能		:获取载重控制器升级标志
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:1:表示正在升级中,0表示没在升级
//备注		:正在升级时请不要发送其它指令，以免对升级干扰。
//		:
*********************************************************************/
u8 LoadControllerUpdata_GetUpdataFlag(void)
{
    return LoaderControllerUpdataFlag;
}
/*********************************************************************
//函数名称	:LoadControllerUpdata_GetCmdAckFlag
//功能		:获取载重控制器命令应答标志
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
//		:
*********************************************************************/
u8 LoadControllerUpdata_GetCmdAckFlag(void)
{
    return LoadControllerCmdAckFlag;
}
/*********************************************************************
//函数名称	:LoadControllerUpdata_SetCmdAckFlag
//功能		:置位载重控制器命令应答标志
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
//		:
*********************************************************************/
void LoadControllerUpdata_SetCmdAckFlag(void)
{
    LoadControllerCmdAckFlag = 1;
}
/*********************************************************************
//函数名称	:LoadControllerUpdata_ClrCmdAckFlag
//功能		:清除载重控制器命令应答标志
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
//		:
*********************************************************************/
void LoadControllerUpdata_ClrCmdAckFlag(void)
{
    LoadControllerCmdAckFlag = 0;
}



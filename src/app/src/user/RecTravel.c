/*******************************************************************************
*版权说明	:Shenzhen E-EYE Co. Ltd., Copyright 2009- All Rights Reserved.
*文件名称	:RecTravel.c		
*功能		:记录仪数据采集
*版本号	:
*开发人	:       :myh
*开发时间	:2013.3
*修改者	:
*修改时间	:
*修改简要说明	:
*备注		:
*******************************************************************************/
#include "include.h"
#include "RecTravel.h"

/******************调试开关****************************************************/
#define   REDEBUG_USB             0  /*USB调试,主要向U盘写入符合协议的模拟数据*/
#define   REUSB_TO_FLASH          0 /*通过USB文件向FLASH中写入数据           */ 
#define   PRO_DEF_RETAINS        (0x00) /*记录仪保留字节,默认0x00*/

/******************本地变量****************************************************/
u8 const h_BufGb2312[100]=
{"京湘津鄂沪粤渝琼冀川晋贵黔辽云滇吉陕秦豫甘陇苏青浙皖藏闽蒙赣桂鲁宁黑新港澳台军空海北沈兰济南广成警"};
u16 const h_BufUnicode[50]=
{
  0x4EAC,0x6E58,0x6D25,0x9102,0x6CAA,0x7CA4,0x6E1D,0x743C,0x5180,0x5DDD,0x664B,0x8D35,0x9ED4,0x8FBD,0x4E91,
  0x6EC7,0x5409,0x9655,0x79E6,0x8C6B,0x7518,0x9647,0x82CF,0x9752,0x6D59,0x7696,0x85CF,0x95FD,0x8499,0x8D63,
  0x6842,0x9C81,0x5B81,0x9ED1,0x65B0,0x6E2F,0x6FB3,0x53F0,0x519B,0x7A7A,0x6D77,0x5317,0x6C88,0x5170,0x6D4E,
  0x5357,0x5E7F,0x6210,0x8B66
};
u8 const Recorder_Cmd_Num[28]=
{
0x0,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x10,0x11,0x12,0x13,0x14,0x15,//16个
0x82,0x83,0x84,0xc2,0xc3,0xc4,//6个
0xe0,0xe1,0xe2,0xe3,0xe4,0xe5 //6个
};
u16 const Recorder_Cmd_OneNum[8]=//一个数据块数用的字节数
{
  126,  //速度 126 
  666,  //位置 666 
  234,  //疑点 234 
  50,   //超时 50  
  25,   //身份 25 
  7,    //供电 7  
  7,    //参数 7  
  133   //速状 133
};
u8 const Recorder_Cmd_PackNum[8]=//GPRS分包个数
{
  5,  //速度 126 * 5
  1,  //位置 666 * 1
  3,  //疑点 234 * 3
  13, //超时 50  * 13
  25, //身份 25  * 25
  100,//供电 7   * 100
  100,//参数 7   * 100
  5   //速状 133 * 5
};
u16 const Recorder_Cmd_PackSize[8]=//GPRS分包大小
{
  630, //速度 126 * 5
  666, //位置 666 * 1
  702, //疑点 234 * 3
  650, //超时 50  * 13
  625, //身份 25  * 25
  700, //供电 7   * 100
  700, //参数 7   * 100
  665  //速状 133 * 5
};
u8 const Vdr_Cmd07_Name[16][18]=
{
    "执行标准版本年号  ",            
    "当前驾驶人信息    ",
    "实时时间          ",
    "累计行驶里程      ",
    "脉冲系数          ",
    "车辆信息          ",
    "状态信号配置信息  ",
    "记录仪唯一性编号  ",
    "行驶速度记录      ",
    "位置信息记录      ",
    "事故疑点记录      ",
    "超时驾驶记录      ",
    "驾驶人身份记录    ",
    "外部供电记录      ",
    "参数修改记录      ",
    "速度状态日志      "    
};

  
/******************外部变量****************************************************/
extern u8	RecorderProtocolFormatFlag;
extern const u8 EepromPramLength[];
extern  u16     DelayAckCommand;      /*延时应答命令                          */
extern FIL file;
extern REGISTER_STRUCT Register[REGISTER_TYPE_MAX];

extern u16  SubpackageCurrentSerial;//分包补传当前包流水号
extern u16  SubpackageResendFirstSerial;//分包重传第1包的流水号
extern u8  SubpacketUploadFlag;//分包上传标志,1为正在上传,0为不在上传


const u8 GB_T2012_Ver[2] ={0x12,0x00};/*记录仪执行标准版本                    */ 

RECORDER_QUEUE_RX  St_RecorderQueueRx; 
RECORDER_QUEUE_TX  St_RecorderQueueTx;

ST_RECORDER_WIRE    St_RecorderWire;
ST_RECORDER_HEAD    RecorderHead;//记录仪协议头结构体
ST_RECORDER_FLASH   RecorderFlash;//记录仪从flash中读取数据结构体
RECORDER_FLASH_ADDR RecorderFlashAddr;//记录仪数据在flash中存储的地址

//static u16  PackAll;//指定时间段内数据总包数
//static u16  PackCnt;//指定时间段内数据总包数计数器
//static u16  OnePackCnt;//一个数据包由OnePackCnt个数据块决定，主要用于速度和位置封包
static u16  DataBlockWatch;//指定时间段内整个数据块数，用于调试观察，无实际作用
//static u16  DataBlockSize;//一个数据块对应的数据长度

static u16  DataBlockAll;//指定时间段内的总的数据块数
static u16  DataBlockSpeedPos;//指定时间段内1分钟速度数据或1小时位置信息数据


static u8 WireSendVer;



static RECORDER_FILE_VDR_NAME vdr_fname;//VDR文件名
static RECORDER_FILE_VDR_DATA vdr_fdata;//VDR文件数据
static u8  vdr_ver;//VDR文件校验
static u16 vdr_Written;//VDR文件系统跟随长度

u32	   SearchTimeCnt = 0;//32位搜索时间
TIME_T SearchTime;//时间结构体搜索时间
TIME_T SpeedPosTime;//用于位置信息、速度判断是否跨分、跨小时


static u8 SerOutTmrCnt;//串口超时计数器
static u8 SerBusyFlag;//串口忙标志  1为忙 0为空闲


extern u16* ff_NameConver(u8 *pStr);

/**
* @brief  计算异或校验
* @param  *p:指向校验的数据；
* @param  len:校验数据的长度；
* @retval None
*/
void  RecorderXorVer(u8 *pVer,u8 *p,u16 len)
{ 
    u16  i;
    
    for(i=0;i<len;i++)
    {
        *pVer ^= *(p+i);
    }
}

/**
* @brief  对指定数据进行倒序处理
* @param  *p:指向倒序的数据；
* @param  len:数据长度
* @retval 返回转换结果，成功返回长度，否则返回0
*/
u16  RecorderDataBack(u8 *p,u16 len)
{ 
    u8  buf[300]={0};
    u16 LenCnt=len-1;
    u16 LenAdd=0;
    
    if(len > sizeof(buf))
    {
        return 0;
    }
    
    for(;LenCnt != 0;)
    {
        buf[LenCnt] = *(p+LenAdd);
        LenCnt--;
        LenAdd++;
    }    
    buf[0] = *(p+LenAdd);
    memcpy(p,buf,len);//复制倒序后的数据
    return LenCnt;
    
}
/**
* @brief  检查BCD码时间
* @param  *p:指向倒序的数据；
* @param  len:数据长度
* @retval 非零为正确，否则错误
*/
u8  RecorderCheckBCDTime(u8 *p)
{
    u8 timetab[6];

    memcpy(timetab,p,6);
    if((timetab[5]&0x0f)>9)
    {
        *(p+5) = 0;
        timetab[5]= 0;
        return 0;
    }
    return 1;
}
/**
* @brief  检查记录仪指令是否合法
* @param  cmd:行驶记录仪指令
* @retval 非零为正确，否则错误
*/
u8  RecorderCheckCmd(u8 cmd)
{
    u8 i;
    for(i=0;i<sizeof(Recorder_Cmd_Num);i++)
    {
        if(Recorder_Cmd_Num[i] == cmd)
        {
            break;
        }
    }
    if(i == sizeof(Recorder_Cmd_Num))
    {
        return 0;
    }
    else
    {
        return 1;
    }
}
/**
* @brief  生成记录仪错误指令数据
* @param  type:应答类型，0为采集错误类型  非0为设置错误类型
* @param  *p:指向数据
* @retval 返回数据长度
*/
u8 RecorderCreateErrCmd(u8 type,u8 *p)
{
    u8 i;
    u8 verify;

    if(type == 0)                    //采集指令错误 
    {
        verify = 0;
        *p=0x55;
        *(p+1)=0x7a;  
        *(p+2)=0xfa; 
        *(p+3)=PRO_DEF_RETAINS;  
        
        for(i=0; i<4; i++)
        {
            verify ^= *(p+i);
        }
	
        *(p+4) = verify;
    }
    else                         //设置指令错误
    {
        verify = 0;
        *p=0x55;
        *(p+1)=0x7a;  
        *(p+2)=0xfb; 
        *(p+3)=PRO_DEF_RETAINS;  
        
        for(i=0; i<4; i++)
        {
            verify ^= *(p+i);
        }
	
        *(p+4) = verify;            
    }
    return 5;

}
/*******************************************************************************
* Function Name  : Recorder_Stop15MinuteSpeed
* Description    : 停车前15分钟速度，内容包括VIN、停车时间、停车前15分钟平均速度
* Input          : *p  : 指向15分钟平均速度信息打包数据              
* Output         : None
* Return         : 返回数据长度
*******************************************************************************/
u8 Recorder_Stop15MinuteSpeed(u8 *p)
{
    u8 Tab[70];
    u8 *pSend;
    u8 TimeBuf[6];//时间缓存区
    
    u8 step;
    u8 i;
    
    u32	TimeCnt;
    TIME_T  Time;

    
    pSend = p;
    memset(Tab,0,sizeof(Tab));
   
    ////////////////////////////////////////////////////////////////////////////  
    FRAM_BufferRead(pSend,FRAM_STOPING_TIME_LEN ,FRAM_STOPING_TIME_ADDR);//停车时间
    
    if(((*(pSend) == 0)&&(*(pSend+1)== 0)&&(*(pSend+2) == 0))||(SpeedMonitor_GetCurRunStatus()))
    {
        Public_ConvertNowTimeToBCDEx(pSend);//存储BCD码时间 0-5
    }
    
    //////////////////////////////////////////////////////////////////////////// 
    FRAM_BufferRead(Tab,FRAM_STOP_15MINUTE_SPEED_LEN, FRAM_STOP_15MINUTE_SPEED_ADDR);//速度
    
    step = Tab[0];//当前步数
    
    for(i=0;i<15;i++)
    {
        if(step == 0)
        {
            step = 14;
        }
        else
        {
            step--;
        }
        
        if(i == 0)
        {     
            memcpy(pSend+6,Tab+(2+(step*4)),3);//倒序存储  
            if((*(pSend+6)==0)&&(*(pSend+7)==0))  
            {
                *(pSend+6) = *(pSend+3);
                *(pSend+7) = *(pSend+4);
                *(pSend+8) = 0;
            }   
        }     
        else
        {    
            if((TimeBuf[3] == Tab[2+(step*4)])&&(TimeBuf[4] == Tab[3+(step*4)]))
            {
                memcpy(pSend+6+(i*3),Tab+(2+(step*4)),3);//倒序存储
            }
            else
            {
                step++;
                memcpy(pSend+6+(i*3),TimeBuf+3,3);//倒序存储
            }
        }
        
        TimeBuf[0] = 0x13;
        TimeBuf[1] = 0x01;
        TimeBuf[2] = 0x01;
        TimeBuf[5] = 0x00;
        memcpy(TimeBuf+3,pSend+6+(i*3),2);//读取本次存储时间,时分              
        Public_ConvertBCDToTime(&Time,TimeBuf);//转时间格式
        TimeCnt = ConverseGmtime(&Time) - 60;
        Gmtime(&Time, TimeCnt); 
        Public_ConvertTimeToBCDEx(Time,TimeBuf);//下分钟应记录的时间
        
    }
    
    return 51;
    
}
/*********************************************************************
//函数名称	:Recorder_SetCarInformation
//功能		:设置车辆信息，包括车辆VIN、车牌号码、车牌分类
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:命令字82H
*********************************************************************/
ProtocolACK Recorder_SetCarInformation(u8 *pBuffer, u8 BufferLen)
{
	u8	Buffer[20];
	u8	PramLen;
	u8	i;

	if(41 != BufferLen)
	{
		return ACK_ERROR;
	}
	//车辆识别代码17字节
	EepromPram_WritePram(E2_CAR_IDENTIFICATION_CODE_ID, pBuffer, 17);
	PramLen = EepromPram_ReadPram(E2_CAR_IDENTIFICATION_CODE_ID, Buffer);
	if(17 != PramLen)
	{
		return ACK_ERROR;
	}
	for(i=0; i<17; i++)
	{
		if(*(pBuffer+i) != Buffer[i])
		{
			return ACK_ERROR;
		}
	}
	pBuffer += 17;			
	//车牌号码12字节
	EepromPram_WritePram(E2_CAR_PLATE_NUM_ID, pBuffer, 12);
	PramLen = EepromPram_ReadPram(E2_CAR_PLATE_NUM_ID, Buffer);
	if(12 != PramLen)
	{
		return ACK_ERROR;
	}
	for(i=0; i<12; i++)
	{
		if(*(pBuffer+i) != Buffer[i])
		{
			return ACK_ERROR;
		}
	}
	pBuffer +=12;			
	//车牌分类12字节
	EepromPram_WritePram(E2_CAR_TYPE_ID, pBuffer, 12);
	PramLen = EepromPram_ReadPram(E2_CAR_TYPE_ID, Buffer);
	if(12 != PramLen)
	{
		return ACK_ERROR;
	}
	for(i=0; i<12; i++)
	{
		if(*(pBuffer+i) != Buffer[i])
		{
			return ACK_ERROR;
		}
	}
	return ACK_OK;
	
}
/*********************************************************************
//函数名称	:Recorder_SetRtc
//功能		:设置实时时钟
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:命令字C2H
*********************************************************************/
ProtocolACK Recorder_SetRtc(u8 *pBuffer, u8 BufferLen)
{
	TIME_T	Rtc;

	if(6 != BufferLen)
	{
		return ACK_ERROR;
	} 
    if(Public_ConvertBCDToTime(&Rtc,pBuffer))
	{
		SetRtc(&Rtc);
        return ACK_OK;
	}
	else
	{
		return ACK_ERROR;
	}
        
}

/*********************************************************************
//函数名称	:Recorder_SetCarFeatureCoeff
//功能		:设置车辆特征系数
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:命令字C3H
*********************************************************************/
ProtocolACK Recorder_SetCarFeatureCoeff(u8 *pBuffer, u8 BufferLen)
{
	u8	Buffer[5];
	u8	PramLen;
        u8      nTab[5];
        
	if(8 != BufferLen)
	{
		return ACK_ERROR;
	}
	
        nTab[0] = 0;
        nTab[1] = *(pBuffer+6);
        nTab[2] = *(pBuffer+7);
        
        EepromPram_WritePram(E2_CAR_FEATURE_COEF_ID, nTab, 3);
	PramLen = EepromPram_ReadPram(E2_CAR_FEATURE_COEF_ID, Buffer);
	if(3 != PramLen)
	{
	    return ACK_ERROR;
	}
        if((Buffer[1]==*(pBuffer+6))&&(Buffer[2]==*(pBuffer+7)))
        {
            return ACK_OK;    
        }
        return ACK_ERROR;
        
	//PulseCounter_ChangePramNotice();
	//更新车辆特征系数
	//PulsePerKm_UpDatePram();
	
}
/*********************************************************************
//函数名称	:Recorder_GetCarInformation
//功能		:获取车辆信息，包括车辆VIN，车牌号码，车牌分类
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:命令字06H
*********************************************************************/
u8 Recorder_GetCarInformation(u8 *pBuffer)
{
	u8	Buffer[45];
	u8	BufferLen;
	u8	i;

	//读车辆VIN
	BufferLen = EepromPram_ReadPram(E2_CAR_IDENTIFICATION_CODE_ID, Buffer);
	if(BufferLen > 17)//强制等于17
	{
		BufferLen = 17;
	}
	else if(BufferLen < 17)//不够，补齐0
	{
		for(i=BufferLen; i<17; i++)
		{
			Buffer[i] = 0;
		}
	}
	for(i=0; i<BufferLen; i++)//0值转成0
	{
		if(0 == Buffer[i])
		{
			Buffer[i]  = 0;
		}
	}
	memcpy(pBuffer,Buffer,17);
	pBuffer += 17;

	//读车牌号码
	BufferLen = EepromPram_ReadPram(E2_CAR_PLATE_NUM_ID, Buffer);
	if(BufferLen > 12)//强制等于12
	{
		BufferLen = 12;
	}
	else if(BufferLen < 12)//不够，补齐0
	{
		for(i=BufferLen; i<12; i++)
		{
			Buffer[i] = 0;
		}
	}
	for(i=0; i<BufferLen; i++)//0值转成0
	{
		if(0 == Buffer[i])
		{
			Buffer[i]  = 0;
		}
	}
	memcpy(pBuffer,Buffer,12);
	pBuffer += 12;

	//读车牌分类
	BufferLen = EepromPram_ReadPram(E2_CAR_TYPE_ID, Buffer);
	if(BufferLen > 12)//强制等于12
	{
		BufferLen = 12;
	}
	else if(BufferLen < 12)//不够，补齐0
	{
		for(i=BufferLen; i<12; i++)
		{
			Buffer[i] = 0;
		}
	}
	for(i=0; i<BufferLen; i++)//0值转成0
	{
		if(0 == Buffer[i])
		{
			Buffer[i]  = 0;
		}
	}
	memcpy(pBuffer,Buffer,12);
	pBuffer += 12;

	return 41;
}


/*******************************************************************************
* Function Name  : Recorder_SendData
* Description    : 发送记录仪反馈数据
* Input          : - *pData ：指向要发送数据的地址
*                  - Len    :发送的数据的长度             
* Output         : None
* Return         : 成功返回真 否则返回假
*******************************************************************************/
u8 Recorder_SendData(u8 *pData, u16 Len)
{
    u16 temp;
    
    if(St_RecorderQueueRx.nChannel)                            
    {
       	                                                   /*预留GSM模块发送  */
    }
    else                                                   /*RS232通道        */
    {
        temp = St_RecorderQueueTx.in;                      /*暂存队列长度     */
        temp += Len;                                       /*长度累加         */
        
        if(temp >1024)                                     /*判断是否溢出     */
        {
            return 0;
        }
        
        temp = St_RecorderQueueTx.in;
        
        St_RecorderQueueTx.in += Len;                      /*数据长度累加     */

        memcpy(St_RecorderQueueTx.nBuf+temp,pData,Len);    /*数据压入队列     */
   	  
    }
    
    return 1;
}

/*******************************************************************************
* Function Name  : Recorder_QueueTxEmpty
* Description    : 检查发送队列是否为空
* Input          : None             
* Output         : None
* Return         : 队列空返回真   否则返回假
*******************************************************************************/
u8 Recorder_QueueTxEmpty(void)
{
    if(St_RecorderQueueTx.in != 0)
    {
        return 1;
    }
    return 0;
}

/*******************************************************************************
* Function Name  : Recorder_SendOutTime
* Description    : 超时发送数据
* Input          : None             
* Output         : None
* Return         : None
*******************************************************************************/
void Recorder_SendOutTime(void)
{
    if((St_RecorderQueueTx.out != St_RecorderQueueTx.in)
    &&(St_RecorderQueueTx.out < St_RecorderQueueTx.in))
    {
	COM1_WriteBuff(St_RecorderQueueTx.nBuf,St_RecorderQueueTx.in);		
        St_RecorderQueueTx.out = 0;
	St_RecorderQueueTx.in  = 0;		
    }    
     
}

/*******************************************************************************
* Function Name  : Recorder_BCD_HEX
* Description    : 转换指定字节BCD码到HEX
* Input          : - *pData  : 指向将要转换的BCD码
*                  - Length  : 转化BCD的字节数                
* Output         : None
* Return         : - 返回转换好的数据
*******************************************************************************/
u32 Recorder_BCD_HEX(u8 *pData,u8 Length)
{
    u32 temp=0;
    
    if(Length == 1)
    {
        temp = (*pData/16)*10 + (*pData & 0x0f);
    }
    else if(Length == 2)
    {
        temp = (*pData/16)*1000   + (*pData & 0x0f)*100 
             + (*(pData+1)/16)*10 + (*(pData+1) & 0x0f);
    }
    else if(Length == 3)
    {
        temp = (*pData/16)*100000   + (*pData & 0x0f)*10000 
             + (*(pData+1)/16)*1000 + (*(pData+1) & 0x0f)*100 
             + (*(pData+2)/16)*10   + (*(pData+2) & 0x0f);
    }
    else if(Length == 4)
    {
        temp = (*pData/16)*10000000   + (*pData & 0x0f)*1000000 
             + (*(pData+1)/16)*100000 + (*(pData+1) & 0x0f)*10000   
             + (*(pData+2)/16)*1000   + (*(pData+2) & 0x0f)*100 
             + (*(pData+3)/16)*10     + (*(pData+3) & 0x0f);
     
    }
    else
    {
        temp = 0;
    }
	
    return temp;

}

/*******************************************************************************
* Function Name  : Recorder_BCD_HEX
* Description    : 32位16进制转换成BCD码,主要利用十进制最高位与16进制最高位关系
*                  进行转换
* Input          : - x  : 要转换的HEX             
* Output         : None
* Return         : - 返回转换好的数据
*******************************************************************************/
u32 Recorder_HEX_BCD(u32 x)
{
    u32 y = 0;
    u32 z = 10000000;
    u32 h = 0x10000000;

    while (z)
    {
        while (x >= z)
        {
            x = x-z;
            y = y+h;
        }
        z = z/10;
        h = h>>4;
    }
    return y;
}

/*******************************************************************************
* Function Name  : Recorder_GetGBT2012Ver
* Description    : 读取记录仪执行标准版本
* Input          : - *pData  : 指向存储数据的缓存区               
* Output         : None
* Return         : 存储数据的长度
*******************************************************************************/
u8 Recorder_GetGBT2012Ver(u8 *pData)
{
    memcpy(pData,GB_T2012_Ver,sizeof(GB_T2012_Ver));
    return sizeof(GB_T2012_Ver);
}

/*******************************************************************************
* Function Name  : Recorder_GetDriverInformation
* Description    : 读取车辆驾驶人信息
* Input          : - *pData  : 指向存储数据的缓存区               
* Output         : None
* Return         : 存储数据的长度
*******************************************************************************/
u8 Recorder_GetDriverInformation(u8 *pData)
{
    
#if	TACHOGRAPHS_19056_TEST
	memcpy(pData,"XXXXXXXXXXXXXXXXXX",18);
#else
	u8 nLen;
    u8 nTab[35];
    u8 i;
	
    memset(nTab,0,sizeof(nTab));
    nLen = EepromPram_ReadPram(E2_CURRENT_LICENSE_ID,nTab);
    
    if(nLen < 18)
    {
        for(i=nLen;i<18;i++)
        {
            nTab[i] = 0x00;
        }
    }
    else if(nLen > 18)
    {
        nLen = 18;
    }

    for(i=0;i<nLen;i++)
    {
    	//某些身份证号码包含非数字‘X = 0X58’
        if((nTab[i]==0xff)||(nTab[i]<0x30)||(nTab[i]>0x58))
        {
            memset(nTab,0,sizeof(nTab));
            break;
        }
    }

    if(TiredDrive_GetLoginFlag()==0)                /*如果卡不在则清空数据    */
    {
        memset(nTab,0,sizeof(nTab));
    }
	
    memcpy(pData,nTab,18);	
#endif

    return 18;
}
/*******************************************************************************
* Function Name  : Recorder_GetRtc
* Description    : 读取实时时间
* Input          : - *pData  : 指向存储数据的缓存区               
* Output         : None
* Return         : 存储数据的长度
*******************************************************************************/
u8 Recorder_GetRtc(u8 *pData)
{
    u8 nTab[10];

    Public_ConvertNowTimeToBCDEx(nTab);
    memcpy(pData,nTab,6);
    
    return 6;
}

/*******************************************************************************
* Function Name  : Recorder_GetMileage
* Description    : 读取累加行驶里程
* Input          : - *pData  : 指向存储数据的缓存区               
* Output         : None
* Return         : 存储数据的长度
*******************************************************************************/
/*
#define E2_CAR_INIT_MILE_ID       0x023d
#define E2_CAR_INIT_MILE_ADDR			(EEPROM_PRAM_BASE_ADDR+630)	
#define E2_CAR_INIT_MILE_LEN			4		
*/
u8 Recorder_GetMileage(u8 *pData)
{
    u8 nTab[30];
    u8 nLen;
    u8 timebuf[6];
#ifndef HUOYUN
    u32 temp_bcd;
    u32 temp_hex;
#endif
    u32 timecnt;
    TIME_T t_time;

 
    memset(nTab,0,sizeof(nTab));                            /*清空缓存区      */
   
    Public_ConvertNowTimeToBCDEx(nTab);                     /*复制实时时间    */  

    nLen = EepromPram_ReadPram(E2_INSTALL_TIME_ID,timebuf);  /*初次安装时间    */
    if(nLen == E2_INSTALL_TIME_ID_LEN)
    {
        timecnt = timebuf[0];
        timecnt = (timecnt<<8) | timebuf[1];
        timecnt = (timecnt<<8) | timebuf[2];
        timecnt = (timecnt<<8) | timebuf[3];
        Gmtime(&t_time, timecnt); 
        Public_ConvertTimeToBCDEx(t_time,nTab+6);
    }
    else
    {
        memset(nTab+6,0,6);
    }

    nLen = EepromPram_ReadPram(E2_CAR_INIT_MILE_ID,nTab+12);/*初始化里程      */
    if(nLen != E2_CAR_INIT_MILE_ID_LEN)
    {      
        memset(nTab+12,0,4);
    }

#ifdef HUOYUN
    FRAM_BufferRead(nTab+16,4,FRAM_HY_MILEAGE_ADDR);//累计里程
#else
    temp_hex = nTab[12];
    temp_hex = (temp_hex<<8) | nTab[13];
    temp_hex = (temp_hex<<8) | nTab[14];
    temp_hex = (temp_hex<<8) | nTab[15];

    temp_bcd = Recorder_HEX_BCD(temp_hex);
    
    nTab[12] = temp_bcd>>24;
    nTab[13] = (temp_bcd>>16)&0x0000ff;
    nTab[14] = (temp_bcd>>8) &0x0000ff;
    nTab[15] = temp_bcd & 0x000000ff;
   
    temp_hex = Public_GetCurTotalMile();                    /*累计行驶里程   */
    temp_bcd = Recorder_HEX_BCD(temp_hex);
    
    nTab[16] = temp_bcd>>24;
    nTab[17] = (temp_bcd>>16)&0x0000ff;
    nTab[18] = (temp_bcd>>8) &0x0000ff;
    nTab[19] = temp_bcd & 0x000000ff; 
#endif	  
    memcpy(pData,nTab,20);
    
    return 20;
}
/*******************************************************************************
* Function Name  : Recorder_GetCarFeatureCoeff
* Description    : 读取脉冲系数
* Input          : - *pData  : 指向存储数据的缓存区               
* Output         : None
* Return         : 存储数据的长度
*******************************************************************************/
u8 Recorder_GetCarFeatureCoeff(u8 *pData)
{
    u8 nTab[15];
    u8 nBuff[5];
    u8 nLen;
	
	
    memset(nTab,0,sizeof(nTab));                              /*清空缓存区    */    
    
    Public_ConvertNowTimeToBCDEx(nTab);                       /*复制实时时间  */ 
    
    nLen = EepromPram_ReadPram(E2_CAR_FEATURE_COEF_ID,nBuff);/*脉冲系数      */
    if(nLen != E2_CAR_FEATURE_COEF_LEN)
    {
        memset(nTab+6,0,2);
    }
    nTab[6] = nBuff[1];
    nTab[7] = nBuff[2];
    
    memcpy(pData,nTab,8);
	
    return 8;
}
/*******************************************************************************
* Function Name  : Recorder_GetOnlyNum
* Description    : 读取唯一性编号
* Input          : - *pData  : 指向存储数据的缓存区               
* Output         : None
* Return         : 存储数据的长度
*******************************************************************************/
/*
#define E2_CAR_ONLY_NUM_ID       0x023e
#define E2_CAR_ONLY_NUM_ADDR		(EEPROM_PRAM_BASE_ADDR+630)	
#define E2_CAR_ONLY_NUM_LEN			35	
*/
u8 Recorder_GetOnlyNum(u8 *pData)
{
    u8 nTab[40];
    u8 nLen;
    u8 i;
    
    memset(nTab,0,sizeof(nTab));                          /*清空缓存区        */

    nLen = EepromPram_ReadPram(E2_CAR_ONLY_NUM_ID,nTab);  /*初次安装时间      */
    if(nLen != E2_CAR_ONLY_NUM_ID_LEN)
    {         
        memset(nTab,0,sizeof(nTab));
    }
    //时间缓存 23 - 25三个
    for(i=23;i<=25;i++)
    {
        if(((nTab[i]&0x0f)>9)||(nTab[i]>>4)>9)
        {
            nTab[23] = 0x13;
            nTab[24] = 0x04;
            nTab[25] = 0x18;
            break;
        }
    }
    memcpy(pData,nTab,sizeof(nTab));
    
    return 35;
}
/*******************************************************************************
* Function Name  : Recorder_SetInstallTime
* Description    : 设置初次安装日期
* Input          : - *pData  : 指向写入存储器的数据
*                  - Len     : 写入存储器数据的长度              
* Output         : None
* Return         : 写入成功返回ACK_OK， 否则返回ACK_ERROR
*******************************************************************************/
ProtocolACK Recorder_SetInstallTime(u8 *pData,u8 Len)
{
    u8 nTab[6];
    u8 nLen;
    TIME_T time;
    u32    timecnt;
    
    if(Len != 6)
    {
        return ACK_ERROR;     
    }
    
    memcpy(nTab,pData,Len);

    time.year  = (nTab[0]>>4)*10 + (nTab[0]&0x0f);
    time.month = (nTab[1]>>4)*10 + (nTab[1]&0x0f);
    time.day   = (nTab[2]>>4)*10 + (nTab[2]&0x0f);
    time.hour  = (nTab[3]>>4)*10 + (nTab[3]&0x0f);
    time.min   = (nTab[4]>>4)*10 + (nTab[4]&0x0f);
    time.sec   = (nTab[5]>>4)*10 + (nTab[5]&0x0f);
    
    timecnt = ConverseGmtime(&time);

    nTab[0] =  timecnt>>24;             
    nTab[1] =  (timecnt>>16)&0xff; 
    nTab[2] =  (timecnt>>8)&0xff; 
    nTab[3] =   timecnt&0xff; 
    nTab[4] = 0; 
    nTab[5] = 0;   
    
    EepromPram_WritePram(E2_INSTALL_TIME_ID, nTab, 4);     /*只存储年月日时   */

    nLen = EepromPram_ReadPram(E2_INSTALL_TIME_ID, nTab);
    if(4 != nLen)
    {
        return ACK_ERROR;
    }
    
    return ACK_OK;
}
/*******************************************************************************
* Function Name  : Recorder_SetInitMileage
* Description    : 设置初始化里程
* Input          : - *pData  : 指向写入存储器的数据
*                  - Len     : 写入存储器数据的长度              
* Output         : None
* Return         : 写入成功返回ACK_OK， 否则返回ACK_ERROR
*******************************************************************************/
ProtocolACK Recorder_SetInitMileage(u8 *pData,u8 Len)
{
    u8  nTab[25]={0};

#ifdef HUOYUN
    memcpy(nTab,pData,Len);
    Recorder_SetRtc(nTab, 6);
    Recorder_SetInstallTime(nTab+6,6);
    EepromPram_WritePram(E2_CAR_INIT_MILE_ID, nTab+12, 4);//初始化里程
    FRAM_BufferWrite(FRAM_HY_MILEAGE_ADDR,nTab+16,4);//累计里程

#else
    u32 nData;
    u8  nLen;
    if(Len > 4)
    {
        return ACK_ERROR;     
    }
    
    memcpy(nTab,pData,Len);
    
    nData = Recorder_BCD_HEX(nTab,Len);                       /*BCD码转换为HEX*/
    
    nTab[0] = nData>>24;
    nTab[1] = (nData>>16)&0x0000ff;
    nTab[2] = (nData>>8) &0x0000ff;
    nTab[3] = nData & 0x000000ff;
    
    EepromPram_WritePram(E2_CAR_INIT_MILE_ID, nTab, 4);
    
    memset(nTab,0,sizeof(nTab));
    nLen = EepromPram_ReadPram(E2_CAR_INIT_MILE_ID, nTab);
    if(4 != nLen)
    {
        return ACK_ERROR;
    }
#endif
    return ACK_OK;
}

/*******************************************************************************
* Function Name  : Recorder_SetOnlyNum
* Description    : 设置唯一性编号
* Input          : - *pData  : 指向写入存储器的数据
*                  - Len     : 写入存储器数据的长度              
* Output         : None
* Return         : 写入成功返回ACK_OK， 否则返回ACK_ERROR
*******************************************************************************/
ProtocolACK Recorder_SetOnlyNum(u8 *pData,u8 Len)
{
    u8 nTab[35];
    u8 nLen;
    
    if(Len != 35)
    {
        return ACK_ERROR;     
    }
    
    memcpy(nTab,pData,Len);
    
    EepromPram_WritePram(E2_CAR_ONLY_NUM_ID, nTab, 35);  

    nLen = EepromPram_ReadPram(E2_CAR_ONLY_NUM_ID, nTab);
    if(35 != nLen)
    {
        return ACK_ERROR;
    }
    
    return ACK_OK;
}

/*******************************************************************************
* Function Name  : Recorder_MileageTest
* Description    : 记录仪鉴定命令->里程误差,需每秒调用一次
* Input          : None            
* Output         : None
* Return         : None
*******************************************************************************/
void Recorder_MileageTest(void)
{

    u8 nTab[55];
    
    u8 nBuff[5];
 
    u16 temp;
    u32 temp_hex;
    
    u8  nData;
    u8  i;
    u8  verify;
    
    memset(nTab,0,sizeof(nTab));                                              /*清空缓存区      */           
    
    nTab[0] = 0x55;
    nTab[1] = 0x7a;
    nTab[2] = 0xe1;
    nTab[3] = 0;
    nTab[4] = 44;
    nTab[5] = PRO_DEF_RETAINS;
    
    Recorder_GetOnlyNum(nTab+6);                                              /*获取唯一编号    */
  
    EepromPram_ReadPram(E2_CAR_FEATURE_COEF_ID,nBuff);                        /*脉冲系数        */

    nTab[35+6] = nBuff[1];
    nTab[36+6] = nBuff[2];
    
    temp = SpeedMonitor_GetCurSpeed();                                        /*获取实时速度    */
    temp = temp * 10;
    nTab[37+6] = temp>>8;
    nTab[38+6] = temp&0xff00;
      
    temp_hex = Pulse_GetTotalMile();                                          /*累计行驶里程    */    
    nTab[39+6] = temp_hex>>24;
    nTab[40+6] = (temp_hex>>16)&0x0000ff;
    nTab[41+6] = (temp_hex>>8) &0x0000ff;
    nTab[42+6] = temp_hex & 0x000000ff; 
        
    nData = Io_ReadExtCarStatus();                                            /*制动            */
    nData = (nData<<1) | Io_ReadExtCarStatusBit(CAR_STATUS_BIT_LEFT_LIGHT);   /*左转            */
    nData = (nData<<1) | Io_ReadExtCarStatusBit(CAR_STATUS_BIT_RIGHT_LIGHT);  /*右转            */
    nData = (nData<<1) | Io_ReadExtCarStatusBit(CAR_STATUS_BIT_FAR_LIGHT);    /*远光            */
    nData = (nData<<1) | Io_ReadExtCarStatusBit(CAR_STATUS_BIT_NEAR_LIGHT);   /*近光            */ 
    nData = (nData<<3);                                                       /*D0-D2用户自定义 */    
       
    nTab[43+6] = nData;     
        
    verify = 0;    
    for(i=0; i<(44+6); i++)
    {
         verify ^= nTab[i];
    }
    nTab[50] = verify;
    
    if(Recorder_QueueTxEmpty()==0)                                            /*判断队列是否为空 */
    {
        Recorder_SendData(nTab,51);
    }
  
}

/*******************************************************************************
* Function Name  : Recorder_Error_Cmd
* Description    : 向串口发送数据错误时的应答帧数据
*                  送错误帧数据
* Input          : - Cmd  : 行驶记录仪命令字              
* Output         : None
* Return         : None
*******************************************************************************/
void Recorder_Error_Cmd(u8 Cmd)
{
    u8 verify=0;
    u8 i;
    u8 Buffer[5];
   
    if(Cmd < 0x80)                     /*上载命令出错应答    */
    {
        verify = 0;
        Buffer[0] = 0x55;
        Buffer[1] = 0x7a;
        Buffer[2] = 0xfa;
        Buffer[3] = 0x00;
	
        for(i=0; i<4; i++)
        {
            verify ^= Buffer[i];
        }
	
        Buffer[4] = verify;
    }
    else                                                /*下传命令出错应答    */
    {
        verify = 0;
        Buffer[0] = 0x55;
        Buffer[1] = 0x7a;
        Buffer[2] = 0xfb;
        Buffer[3] = 0x00;
        
        for(i=0; i<4; i++)
        {
            verify ^= Buffer[i];      
        }
        
        Buffer[4] = verify;
              
    }
    if(Recorder_QueueTxEmpty()==0)                      /*判断队列是否为空    */
    {
        Recorder_SendData(Buffer,5);
    }
}

/*******************************************************************************
* Function Name  : Recorder_CheckVerify
* Description    : 检查行驶记录仪接收的串口数据校验是否正确,不正确则向串口发
*                  送错误帧数据
* Input          : - *pData  : 指向校验的数据
*                  - Length  : 校验数据长度                
* Output         : None
* Return         : - 数据校验正确返回真，否则返回假
*******************************************************************************/
u8 Recorder_CheckVerify(u8 *pData,u16 Length)
{   
    u8  verify = 0;
    u16  i;
    u8 cmd;
    static u8 nDelay;
    
    for(i=0; i<Length-1; i++)                         /*校验数据字节          */
    {
        verify ^= *(pData+i);
    }
    
    if(verify != *(pData+Length-1))                   /*判断校验字节是否相等  */
    {
        if((*(pData+0)==0xaa)&&(*(pData+1)==0x75))
        {
            nDelay++;
            if(nDelay < 2)                            /*等待剩下的数据包     */
            {
                nDelay++;  
                return 0;
            }
        }
        
        nDelay = 0;
        *(pData+0) = 0;                               /*销毁帧头              */
        *(pData+1) = 0;
        
        for(i=0;i<512;i++)                            /*检查整个队列的帧头    */
        {
            if((*(pData+i)==0xaa)&&(*(pData+i+1)==0x75))
            {
                break;
            }
        }        
        
        cmd = *(pData+2);
        if(i>500)                                     /*整个队列无帧头则清空  */
        {
            memset((u8*)&St_RecorderQueueRx,0,sizeof(RECORDER_QUEUE_RX));
        }
        if((*(pData+0)==0xaa)&&(*(pData+1)==0x75))
        {
            Recorder_Error_Cmd(cmd);                      /*向串口发送错误帧数据  */
        }
  	  
        return 0;
    }
    nDelay = 0;
    return 1;
}

/*******************************************************************************
* Function Name  : Vdr_SerOutTime
* Description    : 记录仪串口一帧数据超时判断
* Input          : - *pData  : 指向串口
*                  - Length  : 来自串口数据的长度 
* Output         : None
* Return         : - 收到一帧完整且正确数据返回真，否则返回假
*******************************************************************************/

u8 Recorder_SerOutTime(u8 *p,u16 len)
{
    u8  *pBuf  = St_RecorderQueueRx.nBuf;
    u16 BufLen = St_RecorderQueueRx.in;

    if(len == 0)
    {
        if(St_RecorderQueueRx.in)
        {
            if((SerOutTmrCnt++)>2)
            {
                SerOutTmrCnt = 0;//超时
                SerBusyFlag  = 1;//忙
                return 1;
            }
        }
        return 0;//串口无数据
    }
    
    SerOutTmrCnt = 0;//串口有数据清超时计数器
    
    if(SerBusyFlag)
    {
        return 0;//上次的数据还没处理完毕
    }

    if((BufLen + len) < sizeof(St_RecorderQueueRx.nBuf))//判断溢出
    {
        memcpy(pBuf+BufLen,p,len);
        St_RecorderQueueRx.in = St_RecorderQueueRx.in + len;
    }
    return 0;
}

/*******************************************************************************
* Function Name  : Recorder_Analyse
* Description    : 行驶记录仪数据分析
* Input          : - *pData  : 指向分析的数据
*                  - Length  : 分析数据长度  
* Output         : None
* Return         : - 收到一帧完整且正确数据返回真，否则返回假
*******************************************************************************/
u8 Recorder_Analyse(u8 *p,u16 Len)
{
    u8 *pRec = p;
    u16 temp;
    
    temp = ((*(pRec+3)>>8)|(*(pRec+4)))+7;
     
    if(Recorder_CheckVerify(pRec,temp) == 0)         /*判断校验               */
    {
        return 0;                                    /*校验错误返回假         */
    }
    
    return 1;
}
/*******************************************************************************
* Function Name  : Recorder_ReadStatus
* Description    : 读取状态信号
* Input          : *p：指向数据地址        
* Output         : None
* Return         : 返回数据长度
*******************************************************************************/
u8 Recorder_ReadStatus(u8 *p)
{
    u8 addr=0;

    sFLASH_ReadBuffer(p,FLASH_STATUS_SET_SECTOR,80);

    if(strncmp((char *)(p+70),"制动",4) != 0)
    {
        memcpy(p+addr,"ACC       ",10);//DO
        addr = addr + 10;
        memcpy(p+addr,"手刹      ",10);//D1
        addr = addr + 10;
        memcpy(p+addr,"车灯      ",10);//D2
        addr = addr + 10;  
        memcpy(p+addr,"近光      ",10);//D3
        addr = addr + 10;
        memcpy(p+addr,"远光      ",10);//D4
        addr = addr + 10;
        memcpy(p+addr,"右转向    ",10);//D5
        addr = addr + 10;
        memcpy(p+addr,"左转向    ",10);//D6
        addr = addr + 10;
        memcpy(p+addr,"制动      ",10);//D7        
    }

    return 80;
}
/*******************************************************************************
* Function Name  : Recorder_SetStatus
* Description    : 设置状态信号
* Input          : *p：指向数据地址
*                 len：数据长度           
* Output         : None
* Return         : None
*******************************************************************************/
void Recorder_SetStatus(u8 *p, u8 len)
{
	sFLASH_EraseSector(FLASH_STATUS_SET_SECTOR);
	sFLASH_WriteBuffer(p,FLASH_STATUS_SET_SECTOR,len);
}
/**
* @brief  设置记录仪参数
* @param  tpye:查询类型
* @param  *pdata:指向设置的数据
* @param  RecLen:指向设置数据长度
* @retval 返回结果
*/ 
u8 RecorderDataSet(u8 tpye,u8 *pRec, u16 RecLen)
{
    u8 result = 0;

    if(RecLen == 0)
    {
        return result;
    }
    switch(tpye)
    {
        case CMD_SET_CAR_INFORMATION:                /*设置车辆信息           */
            {        
                if(ACK_OK == Recorder_SetCarInformation(pRec, RecLen))
                {
                    result = 1;
                }
    		}			
            break;
        case CMD_SET_CAR_INIT_INSTALL_TIME:          /*设置初次安装日期       */
            {
                if(ACK_OK == Recorder_SetInstallTime(pRec,RecLen))
                {
                    result = 1;
                }
            }
            break; 
        case CMD_SET_STATUS_SIGNAL_CONFIG:           /*设置状态量配置信息     */
            {      
                Recorder_SetStatus(pRec,RecLen);
                result = 1;
            }
            break; 
        case CMD_SET_RECORDER_TIME:                  /*设置记录仪时间         */
            {     
                if(ACK_OK == Recorder_SetRtc(pRec, RecLen))
                {
                    result = 1;
                }
            }    
            break; 
        case CMD_SET_IMPULSE_RATIO:                  /*设置记录仪脉冲系数     */
			{		
                if(ACK_OK == Recorder_SetCarFeatureCoeff(pRec, RecLen))
                {
                    result = 1;
                }
            }   
            break; 
        case CMD_SET_INIT_MILEAGE:                   /*设置初始化里程         */
            {
                if(ACK_OK == Recorder_SetInitMileage(pRec,RecLen))
                {
                    result = 1;
                }
            }
            break; 
        case 0xc5:                                   /*设置唯一性编号         */
            {
                if(ACK_OK == Recorder_SetOnlyNum(pRec,RecLen))
                {
                    result = 1;
                }
            }
            break;       
    }
    return result;
}
//参数修改，平台参数修改时调用
void Recorder_ParaSet(u8 cmd)
{
	u8  nBuff[20];
	if((cmd > 0x80)&&(cmd <= 0xcf))
	{                  
	    memset(nBuff,0,sizeof(nBuff));                  /*清空缓存区          */             
	    Public_ConvertNowTimeToBCDEx(nBuff);            /*获取BCD码实时时间   */
	    nBuff[6] = cmd;                                 /*存储设置命令字      */
	    Register_Write(REGISTER_TYPE_PRAMATER,nBuff,13);/*存入flash中         */
	}
}
/**
* @brief  查询行驶记录仪0-7的数据
* @param  tpye:查询类型
* @param  *pdata:指向索取的数据
* @retval 返回数据个数
*/ 
u16 RecorderData0_7(u8 tpye,u8 *pdata)
{
    u16 len=0;
    
    switch(tpye)
    {
        case CMD_GET_PERFORM_VERSION:                /*采集记录仪版本         */
            {      
                len = Recorder_GetGBT2012Ver(pdata);               
            }
            break;
        case CMD_GET_DRIVER_INFORMATION:             /*采集驾驶员信息         */
            {      
                len = Recorder_GetDriverInformation(pdata);      
            }
            break;
        case CMD_GET_SYS_RTC_TIME:                   /*采集记录仪实时时间     */
            {   
               len = Recorder_GetRtc(pdata);
            }
            break;
        case CMD_GET_ADD_MILEAGE:                    /*采集累计行驶里程       */
            {
                len = Recorder_GetMileage(pdata);
            }
            break;
        case CMD_GET_IMPULSE_RATIO:                  /*采集记录仪脉冲系数     */
            { 
                len = Recorder_GetCarFeatureCoeff(pdata);
            }
            break;
        case CMD_GET_CAR_INFORMATION:                /*采集车辆信息           */
            {   
                len = Recorder_GetCarInformation(pdata);
            }
            break;
        case CMD_GET_STATUS_SIGNAL_CONFIG:           /*采集状态信号配置       */
            {          
                Public_ConvertNowTimeToBCDEx(pdata);  /*获取BCD码实时时间      */
                *(pdata+6) = 1;
                Recorder_ReadStatus(pdata+7);
                len = 87;
            }
            break;
        case CMD_GET_UNIQUE_ID:                      /*采集唯一ID             */
            {
                len =  Recorder_GetOnlyNum(pdata);
            }
            break;
        default:
        {
           len = 0; 
        }
        break;
    }
    return (len);
}
/*******************************************************************************
* Function Name  : Recorder_Handle
* Description    : 处理行驶记录仪数据处理
* Input          : None              
* Output         : None
* Return         : None
*******************************************************************************/
void Recorder_Handle(void)
{
    
    u8  cmd=0;                                       /*行驶记录仪命令字       */   
    u8  flag=0;                                      /*反馈数据标志           */
    s16 nLen=0;                                      /*反馈据长度             */
    u8  verify;                                      /*反馈数据校验字节       */
    u16 length;
    u16 i;
    
    u8	*pSend;                                      /*反馈数据指针           */
    u8  *pRec;                                       /*接收数据指针           */
    
    u8  nTab[1024];
    u8  nBuff[20];

    TIME_T nStartTime,nEndTime;
//    u32    nStartTimeCnt;
    u32    nEndTimeCnt;
    u16    nMaxBlock;
    
    pSend = nTab;                                    /*初始化发送数据地址     */
    pRec  = St_RecorderQueueRx.nBuf;                 /*初始化接收数据地址     */
               
    cmd  = *(pRec+2);                                /*复制命令字             */   
    length = (*(pRec+3) << 8)|*(pRec+4);             /*复制数据长度           */
    
    switch(cmd)
    {
    	
/*******采集数据命令字*********************************************************/
        case CMD_GET_PERFORM_VERSION:                /*采集记录仪版本         */
                      
            nLen = Recorder_GetGBT2012Ver(pSend+6);
            flag = 1;
            
            break;
        case CMD_GET_DRIVER_INFORMATION:             /*采集驾驶员信息         */
            
            nLen = Recorder_GetDriverInformation(pSend+6);
            flag = 1;            
            
            break;
        case CMD_GET_SYS_RTC_TIME:                   /*采集记录仪实时时间     */

            nLen = Recorder_GetRtc(pSend+6);
            flag = 1;

            break;
        case CMD_GET_ADD_MILEAGE:                    /*采集累计行驶里程       */

            nLen = Recorder_GetMileage(pSend+6);
            flag = 1;
            
            break;
        case CMD_GET_IMPULSE_RATIO:                  /*采集记录仪脉冲系数     */
            
            nLen = Recorder_GetCarFeatureCoeff(pSend+6);
            flag = 1;
            
            break;
        case CMD_GET_CAR_INFORMATION:                /*采集车辆信息           */
            
            nLen = Recorder_GetCarInformation(pSend+6);
            flag = 1;

            break;
        case CMD_GET_STATUS_SIGNAL_CONFIG:           /*采集状态信号配置       */
             
             Public_ConvertNowTimeToBCDEx(pSend+6);  /*获取BCD码实时时间      */
             *(pSend+12) = 1;
             Recorder_ReadStatus(pSend+13);
             nLen = 87;
             flag = 1;
            
            break;
        case CMD_GET_UNIQUE_ID:                      /*采集唯一ID             */
            
            nLen = Recorder_GetOnlyNum(pSend+6);
            flag = 1;
            
            break;
        case CMD_GET_SPECIFY_RECORD_SPEED:                       /*指定的行驶速度记录          */
                     
                Public_ConvertBCDToTime(&nStartTime,pRec+6);     /*开始时间                    */
                Public_ConvertBCDToTime(&nEndTime,pRec+12);      /*结束时间                    */                           
                
                nEndTimeCnt = ConverseGmtime(&nEndTime);
                nEndTimeCnt++;
                Gmtime(&nEndTime, nEndTimeCnt);              
                
                nMaxBlock = (*(pRec+18)<<8) | *(pRec+19);        /*最大块                      */
                //nLen = Register_Read(REGISTER_TYPE_SPEED,pSend+6,nStartTime,nEndTime,nMaxBlock);
                nLen = SpeedMonitor_ReadSpeedRecordData(pSend+6,nStartTime,nEndTime,nMaxBlock);
                flag = 1;
            
            break;
        case CMD_GET_SPECIFY_RECORD_POSITION:                    /*指定的位置信息记录          */
                
                Public_ConvertBCDToTime(&nStartTime,pRec+6);     /*开始时间                    */
                Public_ConvertBCDToTime(&nEndTime,pRec+12);      /*结束时间                    */  
                
                nEndTimeCnt = ConverseGmtime(&nEndTime);
                nEndTimeCnt++;
                Gmtime(&nEndTime, nEndTimeCnt);  
                
                nMaxBlock = (*(pRec+18)<<8) | *(pRec+19);        /*最大块                      */                               
                nLen = Register_Read(REGISTER_TYPE_POSITION,pSend+6,nStartTime,nEndTime,nMaxBlock);
                flag = 1;         
            
             break;
        case CMD_GET_SPECIFY_RECORD_ACCIDENT_DOUBT:              /*指定的事故疑点记录          */
                
                Public_ConvertBCDToTime(&nStartTime,pRec+6);     /*开始时间                    */
                Public_ConvertBCDToTime(&nEndTime,pRec+12);      /*结束时间                    */      
                nMaxBlock = (*(pRec+18)<<8) | *(pRec+19);        /*最大块                      */
                //nMaxBlock = 1;
				nLen = Register_Read(REGISTER_TYPE_DOUBT,pSend+6,nStartTime,nEndTime,nMaxBlock);
                flag = 1;
                                              
            break;
        case CMD_GET_SPECIFY_RECORD_OVERTIME_DRIVING:            /*指定的超时驾驶记录          */
                
                Public_ConvertBCDToTime(&nStartTime,pRec+6);     /*开始时间                    */
                Public_ConvertBCDToTime(&nEndTime,pRec+12);      /*结束时间                    */      
                nMaxBlock = (*(pRec+18)<<8) | *(pRec+19);        /*最大块                      */
                nLen = TiredDrive_ReadTiredRecord(pSend+6,nStartTime,nEndTime);
                flag = 1;                            

            break;
        case CMD_GET_SPECIFY_RECORD_DRIVER_IDENTITY:             /*指定的驾驶员身份记录        */
                
                Public_ConvertBCDToTime(&nStartTime,pRec+6);     /*开始时间                    */
                Public_ConvertBCDToTime(&nEndTime,pRec+12);      /*结束时间                    */      
                nMaxBlock = (*(pRec+18)<<8) | *(pRec+19);        /*最大块                      */
                nLen = Register_Read(REGISTER_TYPE_DRIVER,pSend+6,nStartTime,nEndTime,nMaxBlock);
                flag = 1;
                
            break;
        case CMD_GET_SPECIFY_RECORD_EXTERN_POWER:                /*指定的外部供电记录          */
                
                Public_ConvertBCDToTime(&nStartTime,pRec+6);     /*开始时间                    */
                Public_ConvertBCDToTime(&nEndTime,pRec+12);      /*结束时间                    */      
                nMaxBlock = (*(pRec+18)<<8) | *(pRec+19);        /*最大块                      */
                nLen = Register_Read(REGISTER_TYPE_POWER_ON,pSend+6,nStartTime,nEndTime,nMaxBlock);
                flag = 1;
                
            break;
        case CMD_GET_SPECIFY_RECORD_CHANGE_VALUE:                /*指定的参数修改记录          */
                
                Public_ConvertBCDToTime(&nStartTime,pRec+6);     /*开始时间                    */
                Public_ConvertBCDToTime(&nEndTime,pRec+12);      /*结束时间                    */       
                nMaxBlock = (*(pRec+18)<<8) | *(pRec+19);        /*最大块                      */
                nLen = Register_Read(REGISTER_TYPE_PRAMATER,pSend+6,nStartTime,nEndTime,nMaxBlock);
                flag = 1;
                
            break;
        case CMD_GET_SPECIFY_LOG_STATUS_SPEED:                   /*指定的速度状态日志          */
                
                Public_ConvertBCDToTime(&nStartTime,pRec+6);     /*开始时间                    */
                Public_ConvertBCDToTime(&nEndTime,pRec+12);      /*结束时间                    */
                
                nMaxBlock = (*(pRec+18)<<8) | *(pRec+19);        /*最大块                      */
                nLen = Register_Read(REGISTER_TYPE_SPEED_STATUS,pSend+6,nStartTime,nEndTime,nMaxBlock);
                flag = 1;             

            break;    

/*******设置参数命令字*********************************************************/ 
        case CMD_SET_CAR_INFORMATION:                /*设置车辆信息           */
                
            /*设置内容有车辆识别代码、车牌号码、分类*/
            if(ACK_OK == Recorder_SetCarInformation(pRec+6, length))
            {
                flag = 1;
            }
					
            break;
        case CMD_SET_CAR_INIT_INSTALL_TIME:          /*设置初次安装日期       */
            
            if(ACK_OK == Recorder_SetInstallTime(pRec+6,length))
            {
                flag = 1;
            }
            
            break; 
        case CMD_SET_STATUS_SIGNAL_CONFIG:           /*设置状态量配置信息     */
                  
            Recorder_SetStatus(pRec+6,length);
            flag = 1;
            
            break; 
        case CMD_SET_RECORDER_TIME:                  /*设置记录仪时间         */
                
            if(ACK_OK == Recorder_SetRtc(pRec+6, length))
            {
                flag = 1;
            }
                
            break; 
        case CMD_SET_IMPULSE_RATIO:                  /*设置记录仪脉冲系数     */
					
            if(ACK_OK == Recorder_SetCarFeatureCoeff(pRec+6, length))
            {
                flag = 1;
            }
                
            break; 
        case CMD_SET_INIT_MILEAGE:                   /*设置初始化里程         */
            /*AA 75 C4 00 04 00 11 22 33 44 5B*/
            if(ACK_OK == Recorder_SetInitMileage(pRec+6,length))
            {
                flag = 1;
            }
            
            break; 
        case 0xc5:                                   /*设置唯一性编号         */
            
            if(ACK_OK == Recorder_SetOnlyNum(pRec+6,length))
            {
                flag = 1;
            }
            
            break;           
/**************************鉴定命令字列表**************************************/
            
        case CMD_IDENTIFY_INTO:                   /*进入或保持鉴定状态        */
            Public_ShowTextInfo("进入鉴定状态",100);  
            Public_PlayTTSVoiceAlarmStr("进入鉴定状态");
            Test_EnterE0();
            flag = 1;
            
            break;                       
          
        case CMD_IDENTIFY_MEASURE_MILEAGE:        /*进入里程误差测量          */
                       
            Recorder_MileageTest();
            Test_EnterE1();           
                
            break;  
          
        case CMD_IDENTIFY_MEASURE_IMPULSE_RATIO:  /*进入脉冲系数误差测量      */

            Test_EnterE2();
            flag = 1;
            
            break;            
            
        case CMD_IDENTIFY_MEASURE_SYS_RTC_TIME:   /*进入实时时间误差测量      */

            Test_EnterE3();
            flag = 1;

            break; 
        case CMD_IDENTIFY_RETURN:                 /*返回正常工作状态          */
           Public_ShowTextInfo("返回正常状态",100);  
           Public_PlayTTSVoiceAlarmStr("返回正常状态");
           Test_EnterE4(); 
           flag = 1;
            
            break; 
 
 /*******调试******************************************************************/            
 #if REDEBUG_USB  
       case 0xaa:
         
            sFLASH_EraseBulk();
            flag = 1;
            
        break;
 #endif       
 /*******伊爱自定义************************************************************/  
        case 0xd0:                                   /*设置参数，伊爱自定义   */
	    if(0 == *(pRec+6))                       /*读参数                 */
	    {
		*pSend = 0x55;
		*(pSend+1) = 0x7a;
		*(pSend+2) = cmd;
		//nLen = Recorder_ReadPram(pSend+7, pRec+7, length-1); dxl,2013.10.21有线和无线设置参数使用统一的接口函数
		RadioProtocol_ReadPram(pSend+7, &nLen,pRec+7, length-1,0);
		nLen++;                              /*加上读标志             */
		*(pSend+3) = nLen >> 8;              /*长度                   */
		*(pSend+4) = nLen;
		*(pSend+5) = PRO_DEF_RETAINS;
		*(pSend+6) = 0;                      /*读标志                 */
		if(nLen > 0)
		{
		    verify = 0;
		    for(i=0; i<nLen+6; i++)
		    {
		        verify ^= *(pSend+i);
		    }
		}
		*(pSend+6+nLen) = verify;
		Recorder_SendData(pSend, nLen+7);
	    }
	    else if(1 == *(pRec+6))                  /*写参数                 */
	    {
                //if(ACK_OK == Recorder_WritePram(pRec+7, length-1))  dxl,2013.10.21有线和无线设置参数使用统一的接口函数
		if(ACK_OK == RadioProtocol_WritePram(0,pRec+7, length-1))
		{
		    nLen = 1;                    
                    *pSend = 0x55;
		    *(pSend+1) = 0x7a;
                    *(pSend+2) = cmd;
                    *(pSend+3) = nLen >> 8;
                    *(pSend+4) = nLen;
                    *(pSend+5) = PRO_DEF_RETAINS;
                    *(pSend+6) = 1;
		    
                    verify = 0;
                    for(i=0; i<nLen+6; i++)
		    {
			verify ^= *(pSend+i);
	            }
                    *(pSend+nLen+6) = verify;
                    Recorder_SendData(pSend,nLen+7);    /*发送反馈数据        */
		}
	    }
	    break;  
   
 /*******非法命令字************************************************************/            
        default:
          
            break;  	
    }
    
/***参数修改记录***************************************************************/    
    if((cmd > 0x80)&&(cmd <= 0xcf))
    {                  
        memset(nBuff,0,sizeof(nBuff));                  /*清空缓存区          */             
        Public_ConvertNowTimeToBCDEx(nBuff);            /*获取BCD码实时时间   */
        nBuff[6] = cmd;                                 /*存储设置命令字      */
        Register_Write(REGISTER_TYPE_PRAMATER,nBuff,13);/*存入flash中         */
    }
    
/***检查是否有数据发送*********************************************************/    
    
    if(flag != 0)                                       
    {
        flag = 0; 
        
        if(cmd > 0x80)
        {
            nLen = 0;
        }
	
        *pSend = 0x55;
        *(pSend+1) = 0x7a;
        *(pSend+2) = cmd;
        *(pSend+3) = nLen >> 8;
        *(pSend+4) = nLen;
        *(pSend+5) = PRO_DEF_RETAINS;        
        
        verify = 0;    
        for(i=0; i<(nLen+6); i++)
        {
            verify ^= *(pSend+i);
        }
        *(pSend+nLen+6) = verify;
      
        Recorder_SendData(pSend,nLen+7);                /*发送反馈数据        */
    }
    
    St_RecorderQueueRx.in -= (*(pRec+4)+7);             /*队列长度减少一帧    */
    memset(St_RecorderQueueRx.nBuf,0,(*(pRec+4)+7));    /*销毁一帧数据        */
}

/*******************************************************************************
* Function Name  : Recorder_USBRead_Vdr
* Description    : 向U盘中写入记录仪数据
* Input          : None            
* Output         : None
* Return         : None
*******************************************************************************/
void Recorder_USBRead_Vdr(void)
{
    FIL file;
    u8	Buffer[513];
    u8	Buffer1[25];
    u32	BufferLen=0; 
    u32 flen=0;
    
    u8  DataTab[800];
    u32 DataLen=0;
    u32 DataStep;
    u32 i,k;
    u32	 CurrentTimeCount,TimeCountCheck;
    TIME_T tt,tt_check;
    //kong
    LCD_LIGHT_ON();
    Public_ShowTextInfo("开始灌数据",100); 
    if(FR_OK == f_open(&file,ff_NameConver("123.VDR"),FA_READ))
    {
        
      memset(Buffer,0,sizeof(Buffer));
      f_read(&file,Buffer,512,&BufferLen);
      
      flen = 0;                                 /*数据块数*/
      f_lseek(&file,flen);
      memset(Buffer1,0,sizeof(Buffer1));
      f_read(&file,Buffer1,2,&BufferLen);     
      
      flen = 2;                                 
      f_lseek(&file,flen);
/******************************************************************************/      
      f_read(&file,Buffer1,23,&BufferLen);     /*执行年号 00H*/       
      if(Buffer1[0] == 0x00)
      {
          DataLen = (Buffer1[21]*256)+Buffer1[22];
          f_lseek(&file,flen+23);
          f_read(&file,DataTab,DataLen,&BufferLen);  
          flen = flen+23+DataLen;                   
          f_lseek(&file,flen);
      }
      else
      {
          f_lseek(&file,flen);
      }
/******************************************************************************/      
      f_read(&file,Buffer1,23,&BufferLen);     /*驾驶员信息 01H*/
      if(Buffer1[0] == 0x01)
      {
          DataLen = (Buffer1[21]*256)+Buffer1[22];
          f_lseek(&file,flen+23);
          f_read(&file,DataTab,DataLen,&BufferLen);
          flen = flen+23+DataLen;                    
          f_lseek(&file,flen);
      }
      else
      {
          f_lseek(&file,flen);
      }
/******************************************************************************/      
      f_read(&file,Buffer1,23,&BufferLen);    /*实时时间 02H*/
      if(Buffer1[0] == 0x02)
      {
          DataLen = (Buffer1[21]*256)+Buffer1[22];
          f_lseek(&file,flen+23);
          f_read(&file,DataTab,DataLen,&BufferLen);
          flen = flen+23+DataLen;                    
          f_lseek(&file,flen);
      }
      else
      {
          f_lseek(&file,flen);
      }
/******************************************************************************/       
      f_read(&file,Buffer1,23,&BufferLen);       /*累加里程 03H*/
      if(Buffer1[0] == 0x03)
      {
          DataLen = (Buffer1[21]*256)+Buffer1[22];
          f_lseek(&file,flen+23);
          f_read(&file,DataTab,DataLen,&BufferLen);
          //Recorder_SetInitMileage(DataTab,DataLen);    
          flen = flen+23+DataLen;                    
          f_lseek(&file,flen);
      }
      else
      {
          f_lseek(&file,flen);
      }
/******************************************************************************/      
      f_read(&file,Buffer1,23,&BufferLen);     /*脉冲系数 04H*/
      if(Buffer1[0] == 0x04)
      {
          DataLen = (Buffer1[21]*256)+Buffer1[22];
          f_lseek(&file,flen+23);
          f_read(&file,DataTab,DataLen,&BufferLen);
          //Recorder_SetCarFeatureCoeff(DataTab, DataLen);         
          flen = flen+23+DataLen;                    
          f_lseek(&file,flen);
      }
      else
      {
          f_lseek(&file,flen);
      }      
/******************************************************************************/      
      f_read(&file,Buffer1,23,&BufferLen);     /*车辆信息 05H*/
      if(Buffer1[0] == 0x05)
      {
          DataLen = (Buffer1[21]*256)+Buffer1[22];
          f_lseek(&file,flen+23);
          f_read(&file,DataTab,DataLen,&BufferLen);
          //Recorder_SetCarInformation(DataTab, DataLen);     
          flen = flen+23+DataLen;                    
          f_lseek(&file,flen);
      }
      else
      {
          f_lseek(&file,flen);
      }
/******************************************************************************/      
      f_read(&file,Buffer1,23,&BufferLen);    /*状态配置信息 06H*/
      if(Buffer1[0] == 0x06)
      {
          DataLen = (Buffer1[21]*256)+Buffer1[22];
          f_lseek(&file,flen+23);
          f_read(&file,DataTab,DataLen,&BufferLen);      
          //sFLASH_EraseSector(FLASH_STATUS_SET_SECTOR);
          //sFLASH_WriteBuffer(DataTab+7,FLASH_STATUS_SET_SECTOR,DataLen-7);      
          flen = flen+23+DataLen;                    
          f_lseek(&file,flen);
      }
      else
      {
          f_lseek(&file,flen);
      }
/******************************************************************************/      
      f_read(&file,Buffer1,23,&BufferLen);     /*唯一性编号 07H*/
      if(Buffer1[0] == 0x07)
      {
          DataLen = (Buffer1[21]*256)+Buffer1[22];
          f_lseek(&file,flen+23);
          f_read(&file,DataTab,DataLen,&BufferLen);
          flen = flen+23+DataLen;                    
          f_lseek(&file,flen);
      }
      else
      {
          f_lseek(&file,flen);
      }
/******************************************************************************/        
      Public_ShowTextInfo("速度数据",100); 
      f_read(&file,Buffer1,23,&BufferLen);/*行驶速度 08H*/
      if(Buffer1[0] == 0x08)
      {
          Register_EraseOneArea(REGISTER_TYPE_SPEED);
          
          DataLen = (((Buffer1[19]<<24)|(Buffer1[20]<<16))|(Buffer1[21]<<8))|Buffer1[22];     
          
          DataStep = DataLen/126;/*修改长度 1*/
          
          for(i=DataStep-1;i!=0XFFFFFFFF;i--)
          //for(i=0;i<DataStep;i++)  
          {
              f_lseek(&file,flen+23+(126*i));/*修改长度 2*/
              memset(DataTab,0,sizeof(DataTab));
              f_read(&file,DataTab,126,&BufferLen); /*修改长度 3*/
              
              tt.year = ((DataTab[0]>>4)*10)+(DataTab[0]&0x0f);
              tt.month= ((DataTab[1]>>4)*10)+(DataTab[1]&0x0f);
              tt.day = ((DataTab[2]>>4)*10)+(DataTab[2]&0x0f);
              tt.hour =((DataTab[3]>>4)*10)+(DataTab[3]&0x0f);
              tt.min = ((DataTab[4]>>4)*10)+(DataTab[4]&0x0f);
              tt.sec = ((DataTab[5]>>4)*10)+(DataTab[5]&0x0f);          
              
              CurrentTimeCount = ConverseGmtime(&tt);
              #if (REGISTER_SPEED_COLLECT_LEN==2)
              for(k=0;k<120;k=k+2)
              {
                  memset(Buffer1,0,sizeof(Buffer1));
                  memcpy(Buffer1,DataTab+6+k,2);
                  if(ERROR == Register_Write2(REGISTER_TYPE_SPEED,Buffer1,6,CurrentTimeCount+(k/2)))/*修改长度 4 */
                  {
                      break;
                  }         
              }
              #elif (REGISTER_SPEED_COLLECT_LEN==126)
              if(ERROR == Register_Write2(REGISTER_TYPE_SPEED,DataTab,REGISTER_SPEED_STATUS_STEP_LEN-5,CurrentTimeCount))/*修改长度 4 */
              {
                  break;
              }  
              #endif
          }
          flen = flen+23+DataLen;                    
          f_lseek(&file,flen);
      }
      else
      {
          f_lseek(&file,flen);
      }
/******************************************************************************/       
      Public_ShowTextInfo("位置信息数据",100); 
      f_read(&file,Buffer1,23,&BufferLen);/*位置信息 09H*/
      if(Buffer1[0] == 0x09)
      {      
          Register_EraseOneArea(REGISTER_TYPE_POSITION);
          DataLen = (((Buffer1[19]<<24)|(Buffer1[20]<<16))|(Buffer1[21]<<8))|Buffer1[22];
          DataStep = DataLen/666;/*修改长度 1*/
          
          for(i=DataStep-1;i!=0XFFFFFFFF;i--)
          //for(i=0;i<DataStep;i++)
          {
              f_lseek(&file,flen+23+(666*i));/*修改长度 2*/
              memset(DataTab,0,sizeof(DataTab));
              f_read(&file,DataTab,666,&BufferLen); /*修改长度 3*/
              
              tt.year = ((DataTab[0]>>4)*10)+(DataTab[0]&0x0f);
              tt.month= ((DataTab[1]>>4)*10)+(DataTab[1]&0x0f);
              tt.day = ((DataTab[2]>>4)*10)+(DataTab[2]&0x0f);
              tt.hour =((DataTab[3]>>4)*10)+(DataTab[3]&0x0f);
              tt.min = ((DataTab[4]>>4)*10)+(DataTab[4]&0x0f);
              tt.sec = ((DataTab[5]>>4)*10)+(DataTab[5]&0x0f);          
              
              CurrentTimeCount = ConverseGmtime(&tt);
              
              for(k=0;k<660;k=k+11)
              {
                  memset(Buffer1,0,sizeof(Buffer1));
                  memcpy(Buffer1,DataTab+6+k,11);
                  TimeCountCheck = CurrentTimeCount + ((k/11)*60);
                  Gmtime(&tt_check, TimeCountCheck); 
                  if(ERROR == Register_Write2(REGISTER_TYPE_POSITION,Buffer1,
                                              REGISTER_POSITION_STEP_LEN-5,
                                              TimeCountCheck))/*修改长度 4*/
                  {
                      break;
                  }
              }
          } 
          flen = flen+23+DataLen;                    
          f_lseek(&file,flen);
      }
      else
      {
          f_lseek(&file,flen);
      }      
/******************************************************************************/        
      Public_ShowTextInfo("事故疑点数据",100); 
      f_read(&file,Buffer1,23,&BufferLen);/*事故疑点 10H*/
      if(Buffer1[0] == 0x10)
      {       
          Register_EraseOneArea(REGISTER_TYPE_DOUBT);
          DataLen = (((Buffer1[19]<<24)|(Buffer1[20]<<16))|(Buffer1[21]<<8))|Buffer1[22];
          DataStep = DataLen/234;
          
          for(i=DataStep-1;i!=0XFFFFFFFF;i--)
          //for(i=0;i<DataStep;i++)
          {
              f_lseek(&file,flen+23+(234*i));
              memset(DataTab,0,sizeof(DataTab));
              f_read(&file,DataTab,234,&BufferLen); 
              
              tt.year = ((DataTab[0]>>4)*10)+(DataTab[0]&0x0f);
              tt.month= ((DataTab[1]>>4)*10)+(DataTab[1]&0x0f);
              tt.day = ((DataTab[2]>>4)*10)+(DataTab[2]&0x0f);
              tt.hour =((DataTab[3]>>4)*10)+(DataTab[3]&0x0f);
              tt.min = ((DataTab[4]>>4)*10)+(DataTab[4]&0x0f);
              tt.sec = ((DataTab[5]>>4)*10)+(DataTab[5]&0x0f);          
              
              CurrentTimeCount = ConverseGmtime(&tt);
              if(ERROR == Register_Write2(REGISTER_TYPE_DOUBT,DataTab,REGISTER_DOUBT_STEP_LEN-5,CurrentTimeCount))
              {
                 break;
              }
          }    
          flen = flen+23+DataLen;                    
          f_lseek(&file,flen);
      }
      else
      {
          f_lseek(&file,flen);
      }          
      
/******************************************************************************/       
      Public_ShowTextInfo("超时驾驶数据",100); 
      f_read(&file,Buffer1,23,&BufferLen);/*超时驾驶 11H*/
      if(Buffer1[0] == 0x11)
      {       
          Register_EraseOneArea(REGISTER_TYPE_OVER_TIME);
          DataLen = (((Buffer1[19]<<24)|(Buffer1[20]<<16))|(Buffer1[21]<<8))|Buffer1[22];
          DataStep = DataLen/50;/*修改长度 1*/
          
          //for(i=DataStep-1;i!=0XFFFFFFFF;i--)
          for(i=0;i<DataStep;i++)
          {
              f_lseek(&file,flen+23+(50*i));/*修改长度 2*/
              memset(DataTab,0,sizeof(DataTab));
              f_read(&file,DataTab,50,&BufferLen); /*修改长度 3*/
              
              tt.year = ((DataTab[0+24]>>4)*10)+(DataTab[0+24]&0x0f);
              tt.month= ((DataTab[1+24]>>4)*10)+(DataTab[1+24]&0x0f);
              tt.day = ((DataTab[2+24]>>4)*10)+(DataTab[2+24]&0x0f);
              tt.hour =((DataTab[3+24]>>4)*10)+(DataTab[3+24]&0x0f);
              tt.min = ((DataTab[4+24]>>4)*10)+(DataTab[4+24]&0x0f);
              tt.sec = ((DataTab[5+24]>>4)*10)+(DataTab[5+24]&0x0f);          
              
              CurrentTimeCount = ConverseGmtime(&tt);
              if(ERROR == Register_Write2(REGISTER_TYPE_OVER_TIME,DataTab,REGISTER_OVER_TIME_STEP_LEN-5,CurrentTimeCount))/*修改长度 4*/
              {
                  break;
              }
          } 
          flen = flen+23+DataLen;                    
          f_lseek(&file,flen);
      }
      else
      {
          f_lseek(&file,flen);
      }
      
 /******************************************************************************/        
      Public_ShowTextInfo("驾驶员身份数据",100); 
      f_read(&file,Buffer1,23,&BufferLen);/*驾驶员身份 12H*/
      if(Buffer1[0] == 0x12)
      {
          Register_EraseOneArea(REGISTER_TYPE_DRIVER);
          DataLen = (((Buffer1[19]<<24)|(Buffer1[20]<<16))|(Buffer1[21]<<8))|Buffer1[22];
          DataStep = DataLen/25;/*修改长度 1*/
          
          for(i=DataStep-1;i!=0XFFFFFFFF;i--)
          //for(i=0;i<DataStep;i++)
          {
              f_lseek(&file,flen+23+(25*i));/*修改长度 2*/
              memset(DataTab,0,sizeof(DataTab));
              f_read(&file,DataTab,25,&BufferLen); /*修改长度 3*/
              
              tt.year = ((DataTab[0]>>4)*10)+(DataTab[0]&0x0f);
              tt.month= ((DataTab[1]>>4)*10)+(DataTab[1]&0x0f);
              tt.day = ((DataTab[2]>>4)*10)+(DataTab[2]&0x0f);
              tt.hour =((DataTab[3]>>4)*10)+(DataTab[3]&0x0f);
              tt.min = ((DataTab[4]>>4)*10)+(DataTab[4]&0x0f);
              tt.sec = ((DataTab[5]>>4)*10)+(DataTab[5]&0x0f);          
              
              CurrentTimeCount = ConverseGmtime(&tt);
              if(ERROR == Register_Write2(REGISTER_TYPE_DRIVER,DataTab,REGISTER_DRIVER_STEP_LEN-5,CurrentTimeCount))/*修改长度 4*/
              {
                  break;
              }
          }        
          flen = flen+23+DataLen;                    
          f_lseek(&file,flen);
      }
      else
      {
          f_lseek(&file,flen);
      }
      
/******************************************************************************/       
      Public_ShowTextInfo("外部供电数据",100); 
      f_read(&file,Buffer1,23,&BufferLen);/*外部供电 13H*/
      if(Buffer1[0] == 0x13)
      {      
         Register_EraseOneArea(REGISTER_TYPE_POWER_ON);
         DataLen = (((Buffer1[19]<<24)|(Buffer1[20]<<16))|(Buffer1[21]<<8))|Buffer1[22];
         DataStep = DataLen/7;/*修改长度 1*/
          
          for(i=DataStep-1;i!=0XFFFFFFFF;i--)
          //for(i=0;i<DataStep;i++)
          {
              f_lseek(&file,flen+23+(7*i));/*修改长度 2*/
              memset(DataTab,0,sizeof(DataTab));
              f_read(&file,DataTab,7,&BufferLen); /*修改长度 3*/
              
              tt.year = ((DataTab[0]>>4)*10)+(DataTab[0]&0x0f);
              tt.month= ((DataTab[1]>>4)*10)+(DataTab[1]&0x0f);
              tt.day = ((DataTab[2]>>4)*10)+(DataTab[2]&0x0f);
              tt.hour =((DataTab[3]>>4)*10)+(DataTab[3]&0x0f);
              tt.min = ((DataTab[4]>>4)*10)+(DataTab[4]&0x0f);
              tt.sec = ((DataTab[5]>>4)*10)+(DataTab[5]&0x0f);          
              
              CurrentTimeCount = ConverseGmtime(&tt);
              if(ERROR == Register_Write2(REGISTER_TYPE_POWER_ON,DataTab,REGISTER_POWER_ON_STEP_LEN-5,CurrentTimeCount))/*修改长度 4*/
              {
                  break;
              }
          }
          flen = flen+23+DataLen;                    
          f_lseek(&file,flen);
      }
      else
      {
          f_lseek(&file,flen);
      }
/******************************************************************************/       
      Public_ShowTextInfo("参数修改数据",100); 
      f_read(&file,Buffer1,23,&BufferLen);/*参数修改 14H*/
      if(Buffer1[0] == 0x14)
      {        
          Register_EraseOneArea(REGISTER_TYPE_PRAMATER);
          DataLen = (((Buffer1[19]<<24)|(Buffer1[20]<<16))|(Buffer1[21]<<8))|Buffer1[22];
          DataStep = DataLen/7;/*修改长度 1*/
          
          for(i=DataStep-1;i!=0XFFFFFFFF;i--)
          //for(i=0;i<DataStep;i++)
          {
              f_lseek(&file,flen+23+(7*i));/*修改长度 2*/
              memset(DataTab,0,sizeof(DataTab));
              f_read(&file,DataTab,7,&BufferLen); /*修改长度 3*/
              
              tt.year = ((DataTab[0]>>4)*10)+(DataTab[0]&0x0f);
              tt.month= ((DataTab[1]>>4)*10)+(DataTab[1]&0x0f);
              tt.day = ((DataTab[2]>>4)*10)+(DataTab[2]&0x0f);
              tt.hour =((DataTab[3]>>4)*10)+(DataTab[3]&0x0f);
              tt.min = ((DataTab[4]>>4)*10)+(DataTab[4]&0x0f);
              tt.sec = ((DataTab[5]>>4)*10)+(DataTab[5]&0x0f);          
              
              CurrentTimeCount = ConverseGmtime(&tt);
              if(ERROR == Register_Write2(REGISTER_TYPE_PRAMATER,DataTab,REGISTER_PRAMATER_STEP_LEN-5,CurrentTimeCount))/*修改长度 4*/
              {
                  break;
              }
          }
          flen = flen+23+DataLen;                    
          f_lseek(&file,flen);
      }
      else
      {
          f_lseek(&file,flen);
      }
/******************************************************************************/       
      Public_ShowTextInfo("速度状态日志数据",100); 
      f_read(&file,Buffer1,23,&BufferLen);/*速度状态日志 15H*/
      if(Buffer1[0] == 0x15)
      {       
          Register_EraseOneArea(REGISTER_TYPE_SPEED_STATUS);
          DataLen = (((Buffer1[19]<<24)|(Buffer1[20]<<16))|(Buffer1[21]<<8))|Buffer1[22];
          DataStep = DataLen/133;/*修改长度 1*/    
          for(i=DataStep-1;i!=0XFFFFFFFF;i--)
          //for(i=0;i<DataStep;i++)
          {
              f_lseek(&file,flen+23+(133*i));/*修改长度 2*/
              memset(DataTab,0,sizeof(DataTab));
              f_read(&file,DataTab,133,&BufferLen); /*修改长度 3*/
              
              tt.year = ((DataTab[0+7]>>4)*10)+(DataTab[0+7]&0x0f);
              tt.month= ((DataTab[1+7]>>4)*10)+(DataTab[1+7]&0x0f);
              tt.day = ((DataTab[2+7]>>4)*10)+(DataTab[2+7]&0x0f);
              tt.hour =((DataTab[3+7]>>4)*10)+(DataTab[3+7]&0x0f);
              tt.min = ((DataTab[4+7]>>4)*10)+(DataTab[4+7]&0x0f);
              tt.sec = ((DataTab[5+7]>>4)*10)+(DataTab[5+7]&0x0f);          
              
              CurrentTimeCount = ConverseGmtime(&tt);
              if(ERROR == Register_Write2(REGISTER_TYPE_SPEED_STATUS,DataTab,REGISTER_SPEED_STATUS_STEP_LEN-5,CurrentTimeCount))/*修改长度 4*/
              {
                  break;
              }
          }
      }
      else
      {
          f_lseek(&file,flen);
      }      

    }
    f_close(&file);
    LCD_LIGHT_ON();
    Public_ShowTextInfo("结束",100); 
    
    
}



/*******************************************************************************
* Function Name  : Recorder_Vdr_Name
* Description    : VDR文件名
* Input          : None            
* Output         : None
* Return         : None
*******************************************************************************/
void Recorder_Vdr_Name(void)
{
    u8  tab[50]={0};
    u16 i,j;
    u8  len;
   
    Public_ConvertNowTimeToBCDEx((u8*)&tab);
    vdr_fname.start[0] = 'D';
    vdr_fname.time_ymd[0] = (tab[0]>>4)   + 0x30;
    vdr_fname.time_ymd[1] = (tab[0]&0x0f) + 0x30;
    vdr_fname.time_ymd[2] = (tab[1]>>4)   + 0x30;
    vdr_fname.time_ymd[3] = (tab[1]&0x0f) + 0x30;
    vdr_fname.time_ymd[4] = (tab[2]>>4)   + 0x30;
    vdr_fname.time_ymd[5] = (tab[2]&0x0f) + 0x30;
    vdr_fname.separator_1[0] = '_';
    vdr_fname.time_hm[0] = (tab[3]>>4)   + 0x30;
    vdr_fname.time_hm[1] = (tab[3]&0x0f) + 0x30;
    vdr_fname.time_hm[2] = (tab[4]>>4)   + 0x30;
    vdr_fname.time_hm[3] = (tab[4]&0x0f) + 0x30;  
    vdr_fname.separator_2[0] = '_';
 
    memset(tab,0,sizeof(tab));
 
    len = EepromPram_ReadPram(E2_CAR_PLATE_NUM_ID, tab);
    
    len = strlen((char*)tab);
    
    for(i=0;i<len;i++)
    {
        if((tab[i]!=0)||(tab[i]!=0xff))
        {
            break;
        }
    }
    if(i != len)
    {
        for(i=0;i<98;i+=2)                               
        {
            if((tab[0]==h_BufGb2312[i])&&(tab[1]==h_BufGb2312[i+1]))
            {
                vdr_fname.num[0]  = h_BufUnicode[i/2];
                break;
            }
        }
        if(i < 98)              
        {
            for(i=1,j=2;j<len;i++,j++)
            {
                vdr_fname.num[i] = tab[j];
            }
            vdr_fname.num[i++] = '.';
            vdr_fname.num[i++] = 'V';
            vdr_fname.num[i++] = 'D';   
            vdr_fname.num[i++] = 'R'; 
        } 
        else
        {
            vdr_fname.num[0] = 0x672A; //未
            vdr_fname.num[1] = 0x77E5; //知
            vdr_fname.num[2] = 0x8F66; //车
            vdr_fname.num[3] = 0x724C; //牌  
            vdr_fname.num[4] = '.';
            vdr_fname.num[5] = 'V';
            vdr_fname.num[6] = 'D';   
            vdr_fname.num[7] = 'R'; 
        }
    }
}


/*******************************************************************************
* Function Name  : 向VDR文件中写入数据
* Description    : 行驶记录仪USB数据处理
* Input          : None            
* Output         : None
* Return         : None
*******************************************************************************/
void Recorder_VdrFileWite(u8 *p, u16 len)
{
    f_write (&file, p, len, (void *)&vdr_Written); 
    RecorderXorVer(&vdr_ver,p,len);
}

/*******************************************************************************
* Function Name  : 向VDR文件中写入0-7数据
* Description    : 行驶记录仪USB数据处理
* Input          : None            
* Output         : None
* Return         : None
*******************************************************************************/
void Recorder_VdrCmd_07(u8 block)
{
    u8  bufsave[5]={0};
    u16 bufsavelen;
    u16 i=0;

    bufsave[0] = 0;
    bufsave[1] = block+8;
    Recorder_VdrFileWite(bufsave,2);
        
    for(i=0;i<8;i++)
    {
        memset((u8*)&vdr_fdata,0,sizeof(RECORDER_FILE_VDR_DATA));
        vdr_fdata.cmd = i;
        memcpy(vdr_fdata.name,Vdr_Cmd07_Name[i],18);         
        bufsavelen = RecorderData0_7(i,vdr_fdata.buf);
        vdr_fdata.length[2] = bufsavelen>>8;
        vdr_fdata.length[3] = bufsavelen&0x00ff;  
        Recorder_VdrFileWite((u8*)&vdr_fdata,bufsavelen+23);
    }
}
/*******************************************************************************
* Function Name  : 计算向VDR文件中写入的数据长度
* Description    : 行驶记录仪USB数据处理
* Input          : None            
* Output         : None
* Return         : None
*******************************************************************************/
u32 Recorder_VdrCmd_DataLen(u8 Cmd,u16 allcnt,TIME_T Time_start, TIME_T Time_end)
{
    u32 bufsavelen;
    u8  PackSizeNum;
    if(Cmd == 0x08)
    {
        PackSizeNum = 0;
    }
    else if(Cmd == 0x09)
    {
        PackSizeNum = 1;
    }
    else if(Cmd == 0x10)
    {
        PackSizeNum = 2;
    }
    else if(Cmd == 0x11)
    {
        PackSizeNum = 3;
    }
    else if(Cmd == 0x12)
    {
        PackSizeNum = 4;
    }    
    else if(Cmd == 0x13)
    {
        PackSizeNum = 5;
    }
    else if(Cmd == 0x14)
    {
        PackSizeNum = 6;
    }
    else if(Cmd == 0x15)
    {
        PackSizeNum = 7;
    }
    if(allcnt != 0)
    {
        bufsavelen = Recorder_Cmd_OneNum[PackSizeNum] * allcnt;
    }
    else
    {
        bufsavelen = 0;
    }
    
    return bufsavelen;
}

/*******************************************************************************
* Function Name  : Recorder_USBStartRead
* Description    : 开始导出USB数据
* Input          : None            
* Output         : None
* Return         : None
*******************************************************************************/
void Recorder_USBStartRead(u8 Type)
{
	u16	SearchSector  = Register[Type].CurrentSector;      //当前扇区
	u16	SearchStep    = Register[Type].CurrentStep;        //当前步数
    u16	SearchStepLen = Register[Type].StepLen;            //步长 

    RecorderFlashAddr.Sector = SearchSector;      //当前扇区
	RecorderFlashAddr.Step   = SearchStep;        //当前步数
    RecorderFlashAddr.StepLen= SearchStepLen;     //步长   
    DataBlockAll             = 0; 
}
/*******************************************************************************
* Function Name  : 向VDR文件中写入带有时间参数数据
* Description    : 行驶记录仪USB数据处理
* Input          : None            
* Output         : None
* Return         : None
*******************************************************************************/
void Recorder_VdrCmd_Time(u8 Cmd)
{
	TIME_T Time_start;  //开始时间
	TIME_T Time_end;    //结束时间
	u32  allcnt=0;
	u32  addcnt=1;
	u32  bufsavelen=0;
	u8   namecnt;
    u8   tab_lcd[50]={0};//用于LCD显示数据导出进度
    
    RTC_GetCurTime(&Time_start);
    Time_start.year--;
    RTC_GetCurTime(&Time_end);
    Time_end.year++;

    allcnt = RecorderDataPack(RecorderCmdToRegister(Cmd),Time_start,Time_end,0,NULL);
    if(Cmd == 0x08)
    {
        Public_ShowTextInfo("采集速度数据",100); 
        bufsavelen = Recorder_VdrCmd_DataLen(Cmd,allcnt,Time_start,Time_end);
        namecnt = Cmd;
    }
    else if(Cmd == 0x09)
    {
        Public_ShowTextInfo("采集位置信息数据",100);     
        bufsavelen = Recorder_VdrCmd_DataLen(Cmd,allcnt,Time_start,Time_end);
        namecnt = Cmd;
    }
    else if(Cmd == 0x10)
    {
        Public_ShowTextInfo("采集事故疑点数据",100);  
        bufsavelen = Recorder_VdrCmd_DataLen(Cmd,allcnt,Time_start,Time_end);
        namecnt = 10;
    }
    else if(Cmd == 0x11)
    {
        Public_ShowTextInfo("采集超时驾驶数据",100);
        bufsavelen = Recorder_VdrCmd_DataLen(Cmd,allcnt,Time_start,Time_end);
        namecnt = 11;
    }
    else if(Cmd == 0x12)
    {
        Public_ShowTextInfo("采集驾驶员身份数据",100);
        bufsavelen = Recorder_VdrCmd_DataLen(Cmd,allcnt,Time_start,Time_end);
        namecnt = 12;
    }    
    else if(Cmd == 0x13)
    {
        Public_ShowTextInfo("采集外部供电数据",100);    
        bufsavelen = Recorder_VdrCmd_DataLen(Cmd,allcnt,Time_start,Time_end);
        namecnt = 13;
    }
    else if(Cmd == 0x14)
    {
        Public_ShowTextInfo("采集参数修改数据",100);    
        bufsavelen = Recorder_VdrCmd_DataLen(Cmd,allcnt,Time_start,Time_end);
        namecnt = 14;
    }
    else if(Cmd == 0x15)
    {
        Public_ShowTextInfo("采集速度状态日志数据",100);        
        bufsavelen = Recorder_VdrCmd_DataLen(Cmd,allcnt,Time_start,Time_end);
        namecnt = 15;
    }
    else
    {
        return;
    }
    memset((u8*)&vdr_fdata,0,sizeof(RECORDER_FILE_VDR_DATA));
    vdr_fdata.cmd = Cmd;
    memcpy(vdr_fdata.name,Vdr_Cmd07_Name[namecnt],18);
    vdr_fdata.length[0] = bufsavelen>>24;
    vdr_fdata.length[1] = (bufsavelen>>16)&0xff;     
    vdr_fdata.length[2] = (bufsavelen>>8)&0xff;
    vdr_fdata.length[3] = bufsavelen&0xff;  
    Recorder_VdrFileWite((u8*)&vdr_fdata,23);
    Recorder_USBStartRead(RecorderCmdToRegister(Cmd));
    while(1)
    {  
        if(allcnt == 0)
        {
            break;
        }
        bufsavelen = RecorderDataPackUsb(RecorderCmdToRegister(Cmd),
                                Time_start,Time_end,addcnt,vdr_fdata.buf);
        Recorder_VdrFileWite((u8*)&vdr_fdata.buf,(u16)bufsavelen);
        memset(tab_lcd,0,sizeof(tab_lcd));
        sprintf((char*)tab_lcd,"总包数=%d,当前包数=%d",allcnt,addcnt);
        Public_ShowTextInfo((char*)tab_lcd,100); 
        IWDG_ReloadCounter();
        if(addcnt >= allcnt)
        {
            break;
        }
        else
        {
            addcnt++;
        } 
    }

}

/*******************************************************************************
* Function Name  : Recorder_USBHandle
* Description    : 行驶记录仪USB数据处理
* Input          : None            
* Output         : None
* Return         : None
*******************************************************************************/
void Recorder_USBHandle(void)
{
    u8 i;
    u8 tab[10]={0};
    u32	BufferLen=0; 
    u8  writeflg=0;
    if(f_open(&file,(WCHAR*)ff_NameConver("VDR.txt"),FA_READ | FA_WRITE) == FR_OK)
    {
        f_read(&file,tab,5,&BufferLen);
        if(strncmp((char *)tab,"write",5) == 0)
        {
            memset(tab,0,sizeof(tab));
            memcpy(tab,"ok",2);
            f_lseek(&file,0);
            f_write (&file, tab, 2, (void *)&BufferLen); 
            writeflg = 1;
        }
    }
    f_close(&file);
    if(writeflg)
    {
        Recorder_USBRead_Vdr();
        return ;
    }

    
    E2prom_ReadByte(E2_PROGRAM_UPDATA_REQUEST_ID_ADDR,tab, 2);
    if(tab[0] == 0xaa)
    {
        return ;
    }
    
    LCD_LIGHT_ON();

    Recorder_Vdr_Name();//vdr文件名组包
    IWDG_ReloadCounter();
    if(f_open(&file,(WCHAR*)&vdr_fname,FA_CREATE_ALWAYS | FA_WRITE) == FR_OK)
    {      
        Public_ShowTextInfo("开始导出数据",100); 
        Recorder_VdrCmd_07(8);
        i=8;
        for(;i<sizeof(Recorder_Cmd_Num);i++)
        {
            Recorder_VdrCmd_Time(Recorder_Cmd_Num[i]);
        }  
        f_write (&file, &vdr_ver, 1, (void *)&vdr_Written); 
        f_close(&file);
        Public_ShowTextInfo("数据导出完毕",100); 
    }
    else
    {
        f_close(&file);
        Public_ShowTextInfo("U盘不识别!!",100); 
    }
}

/**
* @brief  检查是否需要全部发送0-7指令数据
* @param  None
* @retval 返回结果，0为不需要，非0为需要
*/ 
u8 Recorder_CheckAll07(void)
{
    if((St_RecorderWire.All_07)&&(St_RecorderWire.Cmd<7))
    {
        St_RecorderWire.AckNum++;
        St_RecorderWire.Cmd++;
        return 1;
    }
    else
    {
        St_RecorderWire.All_07 = 0;
        return 0;
    }
}
/**
* @brief  检查是否继续发送分包数据
* @param  None
* @retval 返回结果，0为不需要，非0为需要
*/ 
u8 Recorder_CheckPackage(void)
{
    if((St_RecorderWire.AllCnt != 0)&&(St_RecorderWire.AddCnt != St_RecorderWire.AllCnt))
    {
        St_RecorderWire.AddCnt++;
        SubpackageCurrentSerial++;//流水号累加
#if 0
        if(SubpackageCurrentSerial == 10)
        {
            SubpackageCurrentSerial = 15;
        }
#endif      
        return 1;
    }
    else
    {
        St_RecorderWire.AddCnt = 0;
        St_RecorderWire.AllCnt = 0;
        SubpackageCurrentSerial = 0;//清零流水号
        SubpacketUploadFlag = 0;//清零流水号标志
        
        return 0;
    }
}
/**
* @brief  记录仪命令转成FLASH存储命令
* @param  tpye:记录仪命令
* @retval 返回FLASH存储命令
*/
u8 RecorderCmdToRegister(u8 Type)
{
    u8 RegType=0;
    
    switch(Type)
    {
        case CMD_GET_SPECIFY_RECORD_SPEED:
        {
            RegType = REGISTER_TYPE_SPEED;
        }
        break;
        case CMD_GET_SPECIFY_RECORD_POSITION:
        {
            RegType = REGISTER_TYPE_POSITION;
        }
        break;
        case CMD_GET_SPECIFY_RECORD_ACCIDENT_DOUBT:
        {
            RegType = REGISTER_TYPE_DOUBT;
        }
        break;
        case CMD_GET_SPECIFY_RECORD_OVERTIME_DRIVING:
        {
            RegType = REGISTER_TYPE_OVER_TIME;
        }
        break;
        case CMD_GET_SPECIFY_RECORD_DRIVER_IDENTITY:
        {
            RegType = REGISTER_TYPE_DRIVER;
        }
        break;
        case CMD_GET_SPECIFY_RECORD_EXTERN_POWER:
        {
            RegType = REGISTER_TYPE_POWER_ON;
        }
        break;
        case CMD_GET_SPECIFY_RECORD_CHANGE_VALUE:
        {
            RegType = REGISTER_TYPE_PRAMATER;
        }
        break;
        case CMD_GET_SPECIFY_LOG_STATUS_SPEED:
        {
            RegType = REGISTER_TYPE_SPEED_STATUS;
        }
        break;        
    }
    return (RegType);
    
}

/**
* @brief  速度分包处理
* @param  *src:指向源数据指针
* @param  *pBlockAll:指向数据块计数器
* @param  PackNum:要提取包数据包号
* @retval 返回数据包的长度
*/
u16 RecorderSpeedPack(u8 *src,u16 PackNum,u16 *pBlockAll)
{
    u8 *dest=src;

    if(SearchTime.min != SpeedPosTime.min)//跨分钟分包
    {
        if(dest != NULL)
        {
            #ifdef USE_ONE_MIN_SAVE_SPEED_RECORD
            memcpy(dest,RecorderFlash.buf,126);
            #else
            memset(dest,0xff,126);
            *dest++ = Public_HEX2BCD(SearchTime.year);
            *dest++ = Public_HEX2BCD(SearchTime.month);
            *dest++ = Public_HEX2BCD(SearchTime.day);
            *dest++ = Public_HEX2BCD(SearchTime.hour);
            *dest++ = Public_HEX2BCD(SearchTime.min);
            *dest++ = 0x00;
            memcpy(dest+(SearchTime.sec*2),RecorderFlash.buf,2);
            #endif
        }
        DataBlockSpeedPos = 1;
    }
    else
    {
        if(dest != NULL)
        {
            memcpy(dest+6+(SearchTime.sec*2),RecorderFlash.buf,2);
        }
        DataBlockSpeedPos++;
    }
    if(SearchTime.sec == 0)
    {
        (*pBlockAll)++;
        DataBlockSpeedPos = 0;
        return 126;
    }    
    SpeedPosTime = SearchTime;
    return 0;
    
}

/**
* @brief  位置信息分包处理
* @param  *src:指向源数据指针
* @param  PackNum:要提取包数据包号
* @param  *pLen:指向返回数据包的长度
* @retval 返回处理后的数据指针
*/
u16 RecorderPositionPack(u8 *src,u16 PackNum,u16 *pBlockAll)
{
    u8 *dest=src;
    
    if(SearchTime.hour != SpeedPosTime.hour)//跨分钟分包
    {
        if(dest != NULL)
        {
            memset(dest,0xff,666);
            *dest++ = Public_HEX2BCD(SearchTime.year);
            *dest++ = Public_HEX2BCD(SearchTime.month);
            *dest++ = Public_HEX2BCD(SearchTime.day);
            *dest++ = Public_HEX2BCD(SearchTime.hour);
            *dest++ = 0x00;
            *dest++ = 0x00;
            memcpy(dest+(SearchTime.min*11),RecorderFlash.buf,11);
        }
        DataBlockSpeedPos = 1;
    }
    else
    {
        if(dest != NULL)
        {
            memcpy(dest+6+(SearchTime.min*11),RecorderFlash.buf,11);
        }
        DataBlockSpeedPos++;
    }
    if(SearchTime.min == 0)
    {
        (*pBlockAll)++;
        DataBlockSpeedPos = 0;
        return 666;
    }    
    SpeedPosTime = SearchTime;
    return 0;
}

/**
* @brief  根据总数据块计算出总包数
* @param  tpye:查询类型
* @param  *pBlock:指向数据块数
* @param  *pPack:指向数据块数
* @retval 返回数据长度
*/
u16 RecorderTotal(u8 Type,u16 PackNum,u8 *pData)
{
    u16 len=0;
    u8  *p=pData;
    
    if(Type == REGISTER_TYPE_SPEED)
    {

    }
    else if(Type == REGISTER_TYPE_POSITION)
    {
        
    }
    else
    {
        DataBlockAll = DataBlockAll + 1;//数据块加1
    }
    switch(Type)
    {
        case REGISTER_TYPE_SPEED:
        {
            len = RecorderSpeedPack(p,PackNum,&DataBlockAll);
        }
        break;
        case REGISTER_TYPE_POSITION:
        {
            len = RecorderPositionPack(p,PackNum,&DataBlockAll);
        }
        break;
        case REGISTER_TYPE_DOUBT:
        {
            if(pData != NULL)
            {
                len = Recorder_Cmd_OneNum[2];
                memcpy(p,RecorderFlash.buf,len);
            } 
        }
        break;
        case REGISTER_TYPE_OVER_TIME:
        {
            if(pData != NULL)
            {
                len = Recorder_Cmd_OneNum[3];
                memcpy(p,RecorderFlash.buf,len);
            } 
        }
        break;
        case REGISTER_TYPE_DRIVER:
        {
            if(pData != NULL)
            {
                len = Recorder_Cmd_OneNum[4];
                memcpy(p,RecorderFlash.buf,len);
            } 
        }
        break;
        case REGISTER_TYPE_POWER_ON:
        {
            if(pData != NULL)
            {
                len = Recorder_Cmd_OneNum[5];
                memcpy(p,RecorderFlash.buf,len);
            }
        }
        break;
        case REGISTER_TYPE_PRAMATER:
        {
            if(pData != NULL)
            {
                len = Recorder_Cmd_OneNum[6];
                memcpy(p,RecorderFlash.buf,len);
            }
        }
        break;
        case REGISTER_TYPE_SPEED_STATUS:
        {
            if(pData != NULL)
            {
                len = Recorder_Cmd_OneNum[7];
                memcpy(p,RecorderFlash.buf,len);
            }
        }
        break;        
    }
    if((p != NULL)&&(PackNum == DataBlockAll))  
    {
        return len;
    }
    return 0;
}

/**
* @brief  根据开始时间结束时间计算时间段内总包数或提取指定数据包数据
* @param  tpye:查询类型
* @param  Start:开始时间
* @param  End:结束时间
* @param  PackNum:要查找的数据包号，只有pData != NULL时才有效
* @param  *pData:指向提取的数据地址；
* @retval 当pData==NULL时总数据包数，否则返回数据长度
*/
u16 RecorderDataPack(u8 Type, TIME_T Start, TIME_T End,u16 PackNum,u8 *pData)
{
    u16 len_return    = 0;                                 //函数返回的数据长度
    u32 StartCnt      = 0;                                 //32位开始时间
    u32 EndCnt        = 0;                                 //32位结束时间                    
    
	u16	SearchSector  = Register[Type].CurrentSector;      //当前扇区
	u16	SearchStep    = Register[Type].CurrentStep;        //当前步数
    u16	SearchStepLen = Register[Type].StepLen;            //步长 

	u32	SearchAddress = SearchSector*FLASH_ONE_SECTOR_BYTES//当前搜索地址 
	                    + SearchStep*SearchStepLen;                   

    u32 AddrStart     = Register[Type].StartSector * FLASH_ONE_SECTOR_BYTES;
    
    u8  *p;//8位指针,主要用于传递处理数据地址
    
    StartCnt = ConverseGmtime(&Start);
    EndCnt   = ConverseGmtime(&End);
	
	if(StartCnt >= EndCnt)//判断时间先后关系
	{
		return 0;//返回错误
	}	
                 
    p = (u8*)&RecorderFlash;
    DataBlockWatch    = 0;
    DataBlockAll      = 0;
    SearchTimeCnt     = 0;
    memset((u8*)&SpeedPosTime,0,sizeof(TIME_T));
    memset((u8*)&SearchTime,0,sizeof(TIME_T));
    for(;SearchAddress >= AddrStart;)//循环条件
    {     
    	if(SearchStep == 0)
    	{       		
            if(SearchSector == Register[Type].OldestSector)
            {
                break;//找到最老扇区
            }
    		SearchSector--;
    		if(SearchSector < Register[Type].StartSector)
    		{
    			SearchSector = Register[Type].EndSector;
    		}
            SearchStep = Register[Type].SectorStep;
    	} 
        SearchStep--;
        SearchAddress = SearchSector * FLASH_ONE_SECTOR_BYTES
                        + SearchStep * SearchStepLen; 
     
        sFLASH_ReadBuffer(p,SearchAddress,SearchStepLen);//读取时间+数据+校验	
      	SearchTimeCnt = Register_BytesToInt(RecorderFlash.time);
      	Gmtime(&SearchTime, SearchTimeCnt);
        DataBlockWatch++;
        if((SearchTimeCnt <= EndCnt)&&(SearchTimeCnt >= StartCnt))//小于结束时间且大于开始时间，在数据区内
        {        
            len_return = RecorderTotal(Type,PackNum,pData);
            if(len_return != 0)
            {                   
                return len_return;//返回数据长度
            }
        }
        IWDG_ReloadCounter();
    }
    if(DataBlockSpeedPos)
    {
        DataBlockSpeedPos = 0;
        DataBlockAll++;
        if((pData != NULL)&&(PackNum == DataBlockAll))
        {
            if(Type == REGISTER_TYPE_SPEED)
            {
                return 126;
            }
            else if(Type == REGISTER_TYPE_POSITION)
            {
                return 666;
            }
        }
    }
    return (DataBlockAll);//返回总包数
}

/**
* @brief  根据开始时间结束时间计算时间段内总包数或提取指定数据包数据,USB专用
* @param  tpye:查询类型
* @param  Start:开始时间
* @param  End:结束时间
* @param  PackNum:要查找的数据包号，只有pData != NULL时才有效
* @param  *pData:指向提取的数据地址；
* @retval 当pData==NULL时总数据包数，否则返回数据长度
*/
u16 RecorderDataPackUsb(u8 Type, TIME_T Start, TIME_T End,u16 PackNum,u8 *pData)
{
    u16 len_return    = 0;                                 //函数返回的数据长度
    u32 StartCnt      = 0;                                 //32位开始时间
    u32 EndCnt        = 0;                                 //32位结束时间                    

	u16	SearchSector  = RecorderFlashAddr.Sector;      //当前扇区
	u16	SearchStep    = RecorderFlashAddr.Step;        //当前步数
    u16	SearchStepLen = RecorderFlashAddr.StepLen;     //步长     

	u32	SearchAddress = SearchSector*FLASH_ONE_SECTOR_BYTES//当前搜索地址 
	                    + SearchStep*SearchStepLen;                   

    u32 AddrStart     = Register[Type].StartSector * FLASH_ONE_SECTOR_BYTES;
    
    u8  *p;//8位指针,主要用于传递处理数据地址
    
    StartCnt = ConverseGmtime(&Start);
    EndCnt   = ConverseGmtime(&End);
	
	if(StartCnt >= EndCnt)//判断时间先后关系
	{
		return 0;//返回错误
	}	
                 
    p = (u8*)&RecorderFlash;
    DataBlockWatch    = 0;
    SearchTimeCnt     = 0;
    memset((u8*)&SpeedPosTime,0,sizeof(TIME_T));
    memset((u8*)&SearchTime,0,sizeof(TIME_T));
    for(;SearchAddress >= AddrStart;)//循环条件
    {     
    	if(SearchStep == 0)
    	{       		
            if(SearchSector == Register[Type].OldestSector)
            {
                break;//找到最老扇区
            }
    		SearchSector--;
    		if(SearchSector < Register[Type].StartSector)
    		{
    			SearchSector = Register[Type].EndSector;
    		}
            SearchStep = Register[Type].SectorStep;
    	} 
        SearchStep--;   
        SearchAddress = SearchSector * FLASH_ONE_SECTOR_BYTES
                        + SearchStep * SearchStepLen; 

        RecorderFlashAddr.Sector = SearchSector;
        RecorderFlashAddr.Step   = SearchStep;
        RecorderFlashAddr.StepLen= SearchStepLen;
     
        sFLASH_ReadBuffer(p,SearchAddress,SearchStepLen);//读取时间+数据+校验	
      	SearchTimeCnt = Register_BytesToInt(RecorderFlash.time);
      	Gmtime(&SearchTime, SearchTimeCnt);
        DataBlockWatch++;
        if((SearchTimeCnt <= EndCnt)&&(SearchTimeCnt >= StartCnt))//小于结束时间且大于开始时间，在数据区内
        {        
            len_return = RecorderTotal(Type,PackNum,pData);
            if(len_return != 0)
            {                   
                return len_return;//返回数据长度
            }
        }
        IWDG_ReloadCounter();
    }
    if(DataBlockSpeedPos)
    {
        DataBlockSpeedPos = 0;
        DataBlockAll++;
        if((pData != NULL)&&(PackNum == DataBlockAll))
        {
            if(Type == REGISTER_TYPE_SPEED)
            {
                return 126;
            }
            else if(Type == REGISTER_TYPE_POSITION)
            {
                return 666;
            }
        }
    }
    return (DataBlockAll);//返回总包数
}
/**
* @brief  GPRS发送数据的总包数
* @param  tpye:查询类型
* @param  Start:开始时间
* @param  End:结束时间
* @param  *pRemain:指向分包的余数
* @retval 总数据包数
*/
u16 RecorderWireSendNum(u8 Type, TIME_T Start, TIME_T End,u16 *pRemain)
{
    u16 returnpack;
    u16 allpack;
    u16 remainpack;    
    u8  cmdpacknum=0;

    allpack = RecorderDataPack(Type,Start,End,0,NULL);
    cmdpacknum = Type - 1;     
#ifdef HUOYUN
    u32  datalen = Recorder_Cmd_OneNum[cmdpacknum]*allpack;//总数据块字节数
    returnpack = datalen/900;
    remainpack = datalen%900;
    if(remainpack)
    {
        returnpack++;
        *pRemain = remainpack;
    }    
#else
    returnpack = allpack/Recorder_Cmd_PackNum[cmdpacknum];
    remainpack = allpack%Recorder_Cmd_PackNum[cmdpacknum];
    if(remainpack)
    {
        returnpack++;
        *pRemain = remainpack;
    }
#endif
    
    return returnpack; 
}

/**
* @brief  通过总包数和当前包数返回数据
* @param  tpye:查询类型
* @param  TotalPackage:总包数
* @param  CurrentPackage:当前包数
* @param  *pData:指向当前包数的数据
* @retval 返回数据长度
*/
u16 RecorderWireSend(u8 type,u16 Count,u8 *pData) 
{
    TIME_T start = St_RecorderWire.Time_start;
    TIME_T end   = St_RecorderWire.Time_end;

#ifndef   HUOYUN 
    u16 i;
    u16 searchcnt;
    u16 ForPack; 
#endif   
    u16 Len=0;//数据块长度
    u8  *pSend=pData;
    u8  cmdpacknum=0;
 
    switch(type)
    {
        case 0x08:
        {
            cmdpacknum = 0;
        }
        break;
        case 0x09:
        {
            cmdpacknum = 1;
        }
        break;
        case 0x10:
        {
            cmdpacknum = 2;
        }
        break;
        case 0x11:
        {
            cmdpacknum = 3;
        }
        break;
        case 0x12:
        {
            cmdpacknum = 4;
        }
        break;
        case 0x13:
        {
            cmdpacknum = 5;
        }
        break;
        case 0x14:
        {
            cmdpacknum = 6;
        }
        break;  
        case 0x15:
        {
            cmdpacknum = 7;
        }
        break;        
    }

#ifdef HUOYUN
    u16 HY_addr;
    u16 HY_remain=St_RecorderWire.HY_Cmd_SendRemain;
    if(HY_remain)
    {
        if(HY_remain > Recorder_Cmd_OneNum[cmdpacknum])
        {
            RecorderDataPack(RecorderCmdToRegister(type),start,end,
                             St_RecorderWire.HY_Cmd_SendNum-1,pSend);
            HY_remain = HY_remain%Recorder_Cmd_OneNum[cmdpacknum];
            Len = HY_remain;
            HY_addr = Recorder_Cmd_OneNum[cmdpacknum] - Len;
            memmove(pSend,pSend+HY_addr,Len);//搬移数据     
            RecorderDataPack(RecorderCmdToRegister(type),start,end,
                             St_RecorderWire.HY_Cmd_SendNum,pSend+Len);
            Len = Len + Recorder_Cmd_OneNum[cmdpacknum];   
            St_RecorderWire.HY_Cmd_SendNum++;
            if(St_RecorderWire.AllCnt == Count)
            {
                St_RecorderWire.HY_Cmd_SendRemain = 0;
                return Len;
            }                 
        }
        else
        {
            RecorderDataPack(RecorderCmdToRegister(type),start,end,St_RecorderWire.HY_Cmd_SendNum,pSend);
            Len = St_RecorderWire.HY_Cmd_SendRemain;
            HY_addr = Recorder_Cmd_OneNum[cmdpacknum] - St_RecorderWire.HY_Cmd_SendRemain;
            memmove(pSend,pSend+HY_addr,Len);//搬移数据  
            St_RecorderWire.HY_Cmd_SendNum++;
            if(St_RecorderWire.AllCnt == Count)
            {
                St_RecorderWire.HY_Cmd_SendRemain = 0;
                return Len;
            }
        }
      

    }
    for(;;)
    {
        RecorderDataPack(RecorderCmdToRegister(type),start,end,St_RecorderWire.HY_Cmd_SendNum,pSend+Len);
        Len = Len + Recorder_Cmd_OneNum[cmdpacknum];
        if((St_RecorderWire.AllCnt == 1)&&(St_RecorderWire.AllRemainCnt == Len))
        {
            return Len;
        }
        if(Len > 900)
        {          
            St_RecorderWire.HY_Cmd_SendRemain = (Len%900);
            if(Count == 1)
            {
                St_RecorderWire.HY_Cmd_SendRemain=St_RecorderWire.HY_Cmd_SendRemain+9;
            }
            Len = Len - St_RecorderWire.HY_Cmd_SendRemain;
            return Len;
        }
        else
        {
            St_RecorderWire.HY_Cmd_SendNum++;
        }
    }  
#else 
    if((St_RecorderWire.AllCnt == Count)&&(St_RecorderWire.AllRemainCnt != 0)
        &&(St_RecorderWire.AddOverflg == 0))
    {
        ForPack = St_RecorderWire.AllRemainCnt;
    }
    else
    {
        ForPack = Recorder_Cmd_PackNum[cmdpacknum];
    }
    searchcnt = Recorder_Cmd_PackNum[cmdpacknum]*(Count-1);
    for(i=1;i<=ForPack;i++)
    {
        RecorderDataPack(RecorderCmdToRegister(type),start,end,searchcnt+i,pSend+Len);
        Len = Len + Recorder_Cmd_OneNum[cmdpacknum];
    }    
    return Len;
#endif
}

/*********************************************************************
//函数名称	:RecorderCom_WirelessTimeTask
//功能		:无线行驶记录仪定时任务
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
FunctionalState RecorderWireless_TimeTask(void)
{

    MESSAGE_HEAD	head;
    u16 Datalen=0;
    u8	SmsPhone[20]={0};
    u8  *pData808=NULL;//808协议数据发送指针
    u8 channel = CHANNEL_DATA_1;
    u16 SerialNum = 0;
    
    channel = RadioProtocol_GetDownloadCmdChannel(0x8700, &SerialNum);
    
    memset(&head,0,sizeof(MESSAGE_HEAD));                    //清空结构体
    memset(&RecorderHead,0,sizeof(ST_RECORDER_HEAD));
    
    RecorderHead.cmd808     = St_RecorderWire.Cmd;            //命令字
    RecorderHead.acknum808H = St_RecorderWire.AckNum>>8;      //应答流水号
    RecorderHead.acknum808L = St_RecorderWire.AckNum&0x00ff;
    RecorderHead.head1      = 0x55;                           //帧头
    RecorderHead.head2      = 0x7a;                           //帧头 
    RecorderHead.cmd        = St_RecorderWire.Cmd;            //命令字
    RecorderHead.back       = PRO_DEF_RETAINS;                //备用字       

    head.MessageID          =  0x0700;//记录仪数据上传命令  
    
    if(RecorderCheckCmd(St_RecorderWire.Cmd))
    {
        if(St_RecorderWire.Cmd <= 0x07)                           //无参数命令
        {
            Datalen = RecorderData0_7(St_RecorderWire.Cmd,RecorderHead.DataBuf);
            RecorderHead.lenH = Datalen>>8;
            RecorderHead.lenL = Datalen&0x00ff;
        }
        else if(((St_RecorderWire.Cmd >= 0x82)&&(St_RecorderWire.Cmd <= 0x84))  
               ||((St_RecorderWire.Cmd >= 0xC2)&&(St_RecorderWire.Cmd <= 0xC4))) 
        {
            RecorderDataSet(St_RecorderWire.Cmd,St_RecorderWire.Buf, St_RecorderWire.Len);//参数设置
        }
        else//有参数命令,即带有开始时间、结束时间、最大块数限制
        {
            if(St_RecorderWire.AllCnt)
            {
                Datalen = RecorderWireSend(St_RecorderWire.Cmd,
                                           St_RecorderWire.AddCnt,
                                           RecorderHead.DataBuf);   

#ifdef HUOYUN
                if((St_RecorderWire.AddCnt == 1)&&(St_RecorderWire.AllCnt>1))
                {
                    RecorderHead.lenH  = ((900*St_RecorderWire.AllCnt)+
                                         St_RecorderWire.AllRemainCnt)>>8;
                    RecorderHead.lenL  = ((900*St_RecorderWire.AllCnt)+
                                         St_RecorderWire.AllRemainCnt)&0xff;
                } 
                else
                {
                    RecorderHead.lenH = Datalen>>8;
                    RecorderHead.lenL = Datalen&0x00ff;
                }
#else       
                RecorderHead.lenH = Datalen>>8;
                RecorderHead.lenL = Datalen&0x00ff;
#endif            
            }
        }
        if((St_RecorderWire.AllCnt == 1)&&(Datalen < 900))
        {
            WireSendVer = 0;
            RecorderXorVer(&WireSendVer,(u8*)&RecorderHead.head1,Datalen+6);//数据区小于600则不进行分包
            RecorderHead.DataBuf[Datalen] = WireSendVer;
        }
        else
        {
            head.TotalPackage   =  St_RecorderWire.AllCnt;//总包数   
            head.CurrentPackage =  St_RecorderWire.AddCnt;//当前包数
        }
    }
    else
    {
        Datalen = 0;
        head.TotalPackage = 0;
        head.CurrentPackage = 0;
    }
    if(head.TotalPackage)
    {
#ifdef HUOYUN
        if(head.CurrentPackage == 1)//第一包数据需要带上: 命令字 + 应答流水号
        {
            WireSendVer           = 0;     
            SubpacketUploadFlag   = 1;//发送流水号单独控制
            SubpackageCurrentSerial = 1;//流水号从1开始        
            pData808              = (u8*)&RecorderHead.head1;
            RecorderXorVer(&WireSendVer,pData808,Datalen+6); //校验
            if(St_RecorderWire.AllCnt == St_RecorderWire.AddCnt)
            {
                *(pData808+Datalen+6)    = WireSendVer;
                Datalen++;
            }
            head.Attribute         = (Datalen+9)|0x2000;
            pData808               = (u8*)&RecorderHead;         
        }
        else
        { 
            if(St_RecorderWire.AllCnt == St_RecorderWire.AddCnt)
            {
                pData808               = (u8*)&RecorderHead.DataBuf;
                RecorderXorVer(&WireSendVer,pData808,Datalen);
                *(pData808+Datalen)    = WireSendVer;
                head.Attribute         = (Datalen+1)|0x2000;               
            }
            else
            {
                pData808               = (u8*)&RecorderHead.DataBuf;
                RecorderXorVer(&WireSendVer,pData808,Datalen);
                head.Attribute         = Datalen|0x2000;
            }
        }
#else
        if(head.CurrentPackage == 1)//第一包数据需要带上: 命令字 + 应答流水号
        {
            WireSendVer           = 0;     
            SubpacketUploadFlag   = 1;//发送流水号单独控制
            SubpackageCurrentSerial = 1;//流水号从1开始
            
            pData808              = (u8*)&RecorderHead.head1;
            RecorderXorVer(&WireSendVer,pData808,Datalen+6); //校验
            *(pData808+Datalen+6)    = WireSendVer;
            head.Attribute         = (Datalen+10)|0x2000;
            pData808               = (u8*)&RecorderHead;         
        }
        else
        { 
            WireSendVer = 0;  
            pData808               = (u8*)&RecorderHead.head1;
            RecorderXorVer(&WireSendVer,pData808,Datalen+6);
            *(pData808+Datalen+6)    = WireSendVer;
            head.Attribute         = (Datalen+7)|0x2000;
        }
#endif
    }
    else
    {
        if(RecorderCheckCmd(St_RecorderWire.Cmd))
        {
            head.Attribute = Datalen+10;//不进行分包补传
            pData808 = (u8*)&RecorderHead;
        }
        else
        {
            if(St_RecorderWire.Cmd808 == 0x8700)
            {
                head.Attribute = RecorderCreateErrCmd(0,(u8*)&RecorderHead.head1)+3;//采集错误
            }
            else
            {
                head.Attribute = RecorderCreateErrCmd(1,(u8*)&RecorderHead.head1)+3;//设置错误
            }
            pData808 = (u8*)&RecorderHead;
        }
    }
    
    if(0 != RadioProtocol_SendCmd(channel,head,pData808,(head.Attribute&0X03FF),SmsPhone)) 
    {           
        if(Recorder_CheckAll07()||Recorder_CheckPackage())
        {
            return ENABLE;
        }
        else
        {
            return DISABLE;
        }
    }
    else
    {    
        SetTimerTask(TIME_WIRELESS_RECORDER, 30*SYSTICK_100ms);//触发无线记录仪定时任务      
        return ENABLE;
    }

}

/*******************************************************************************
* Function Name  : RecorderWired_TimeTask
* Description    : 记录仪事件处理,100ms调用一次
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
FunctionalState RecorderWired_TimeTask(void)
{
    u16 nRecLen = 0;
    u8  nRecTab[512];

    ////////////////////////////////////////////////////////////////////////////接收
    nRecLen = COM1_ReadBuff(nRecTab,512);  
    
    #ifdef EYE_MODEM
    Modem_Debug_UartRx(nRecTab,nRecLen);
    #endif

    ////////////////////////////////////////////////////////////////////////////发送
    Recorder_SendOutTime();//发送处理的数据
    
    ////////////////////////////////////////////////////////////////////////////一帧数据
    if(Recorder_SerOutTime(nRecTab,nRecLen) == 0)
    {
        return ENABLE;
    }
    
    ////////////////////////////////////////////////////////////////////////////数据处理
    ///// 接收到完整一帧数据 -> St_RecorderQueueRx.nBuf
    ///// ..............长度 -> St_RecorderQueueRx.in

    ///// 其他协议处理
    CarLoad_CommProtocolParse(St_RecorderQueueRx.nBuf,St_RecorderQueueRx.in);

    /////记录仪协议处理,该协议需放到最后
    if(Recorder_Analyse(St_RecorderQueueRx.nBuf,St_RecorderQueueRx.in))
    {
         Recorder_Handle();
    }

    ////////////////////////////////////////////////////////////////////////////清零缓存
    SerBusyFlag           = 0;//处理完毕
    St_RecorderQueueRx.in = 0;//计数器清零
    
    return ENABLE;
}




/********************************************************************
//版权说明	:Shenzhen E-EYE Co. Ltd., Copyright 2009- All Rights Reserved.
//文件名称	:Other.c		
//功能		:杂散的函数
//版本号	:
//开发人	:dxl
//开发时间	:2012.4 
//修改者	:
//修改时间	:
//修改简要说明	:
//备注		:
***********************************************************************/
//***************包含文件*****************
#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "include.h"

extern u16     DelayAckCommand;//延时应答命令
extern u8	DelaySendChannel;//延时发送通道号
extern u8	DelaySmsPhone[];//延时发送短信号码
extern u8      MultiMediaSendFlag;//多媒体发送标志，0为不在发送，1为在发送
extern u16     CarControlAckSerialNum;//车辆控制应答流水号
extern  TIME_TASK MyTimerTask[];
extern u32     PhotoID;//图像多媒体ID
extern u8      MediaType;//多媒体类型，0图像，1音频，2视频
extern u8      MediaFormat;//多媒体格式编码,0JPEG,1TIF,2MP3,3WAV,4WMV
extern u8	MediaRatio;//多媒体分辨率
extern u8      MediaChannelID;//多媒体通道ID
extern u8      MediaEvenCode;//多媒体事件编码
extern u16     MediaCommand;//多媒体命令  0:停止  N:张数  0XFFFF:录象 0XFFFE:一直拍照  BY WYF
extern u16     MediaPhotoSpace; //拍照间隔时间 BY WYF
extern u8	MemoryUploadFileName[];//存储多媒体数据上传的完整文件名
extern u8	CurrentWavFolder;//当前录音文件夹
extern CAMERA_STRUCT 	Camera[];//摄像头结构体
extern u16	RecorderSerialNum;//行驶记录仪应答流水号
extern u32 TestBlindSaveCount;
extern u8      RecordSendFlag;//录音文件上传标志,1为上传
extern u8 OneHourDelayTimeCountEnableFlag;
extern u32 OneHourDelayTimeCount;
extern u8  UpdataResultNoteAckFlag;//升级结果通知应答标志,1为收到平台的通用应答了
extern u8	TerminalAuthorizationFlag;//终端鉴权标志,bit0:连接1;bit1:连接2;bit2~bit7保留;
//每次终端鉴权成功后会相应位会置1,网络断开后相应位会清0
u8	ResetEnableFlag = 0;//重启使能标志
u8	Link1LoginCount = 0;//注册命令发送次数，每次30秒，3次不成功，需断开网络，10分钟后重连
u8	Link1AuthorizationCount = 0;//鉴权命令发送次数，每次30秒，10次不成功，擦除鉴权码，重走注册流程
u8	Link1LoginAuthorizationState = 0;//读取鉴权码，发送注册命令，发送鉴权命令，断开网络，延时、重连网络、关闭任务几个状态
u8	Link1LoginAuthorizationLastState = 0;//上一次状态
u16	Link1LoginAuthorizationDelayTime = 0;//注册鉴权延时时间
u8	Link2LoginCount = 0;//注册命令发送次数，每次30秒，3次不成功，需断开网络，10分钟后重连
u8	Link2AuthorizationCount = 0;//鉴权命令发送次数，每次30秒，10次不成功，擦除鉴权码，重走注册流程
u8	Link2LoginAuthorizationState = 0;//读取鉴权码，发送注册命令，发送鉴权命令，断开网络，延时、重连网络、关闭任务几个状态
u8	Link2LoginAuthorizationLastState = 0;//上一次状态
u16	Link2LoginAuthorizationDelayTime = 0;//注册鉴权延时时间
static u16	Link1LoginAuthorizationDelayCount = 0;//延时计数，1秒钟计数1
static u16	Link2LoginAuthorizationDelayCount = 0;//延时计数，1秒钟计数1
u8   Link1ManualCloseFlag = 0;//连接1手动关闭标志,1为关闭,0为开启
u8   Link2ManualCloseFlag = 0;//连接2手动关闭标志,1为关闭,0为开启
//***************本地变量******************
static u8	BeepOnEv = 0;//蜂鸣器鸣叫的事件,bit0,收到调度信息;bit1,超速;bit2,疲劳驾驶;bit3,未登录
static u8   CostDownVersionFlag = 0;//dxl,2014.4.29,降成本标志，1为降成本版本，0为降成本之前的版本，该标志涉及显示屏，打印机，USB，麦克风的相关控制
//****************全局变量*****************
u8      Ftp_Updata_BusyFlg;//ftp 升级忙标志   0：空闲   1：升级中 myh 130808
u8	SpeedFlag = 0;//速度类型,dxl,2015.5.11,0x00:手动脉冲,0x01:手动GPS,0x02:自动脉冲,0x03:自动GPS
u8	AccOffGpsControlFlag = 0;//ACC OFF时GPS模块控制标志,0为关,1为开
u8	EvaluatorControlFlag = 0;//评价器功能开关,0为关,1为开
u8	TaximeterControlFlag = 0;//计价器功能开关,0为关,1为开
u8	ToplightControlFlag = 0;//顶灯功能开关,0为关,1为开
u8	LedlightControlFlag = 0;//LED广告功能开关,0为关,1为开
u8      MultiCenterLinkFlag = 0;//多中心连接标准,1为已设置要求连接备份服务器
u8      BdTestFlag = 0;//北斗测试标志,对应于1.使用前锁定;2.自检;.....
u16     BdTestCount1 = 0;//北斗测试计数,每次更换类型BdTestFlag都会清0
u16     BdTestCount2 = 0;//北斗测试计数,每次更换类型BdTestFlag都会清0
u8  DelayBuffer[100];//延时缓冲，可把要延时发送的数据拷贝到这里，注意最大长度限制
u8  DelayBufferLen;//延时缓冲数据长度
u8  GanSuOilBuffer[100];//接收甘肃电信所接油耗传感器的数据缓冲
u8  GanSuOilBufferLen = 0;
u8  GanSuOilBufferBusyFlag = 0;//忙标志,1为忙,表示禁止对其写入数据,0为闲,表示可以写入数据
static u16 Peripheral1Type = 0;//外设1类型，对应参数ID E2_PRAM_BASE_CUSTOM_ID+0x80，即E2_LED_PORT_ID
static u16 Peripheral2Type = 0;//外设2类型，对应参数ID E2_PRAM_BASE_CUSTOM_ID+0x81，即E2_POS_PORT_ID
static u8 AccelerationAckBuffer[20];
static u8 AccelerationAckBufferLen;
//****************宏定义******************
/*********************************************************************
//函数名称	:UpdatePeripheralType
//功能		:更新变量Peripheral1Type,Peripheral2Type
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:1.当设置外设1类型或外设2类型参数时需要调用该接口
//              :2.上电启动初始化时需调用该接口，已在
*********************************************************************/
void UpdatePeripheralType(void)
{
        u8	Buffer[3];
	u8	BufferLen;
        
	BufferLen = EepromPram_ReadPram(E2_LED_PORT_ID, Buffer);
    	if(E2_LED_PORT_ID_LEN == BufferLen)
    	{
                Peripheral1Type = Public_ConvertBufferToShort(Buffer); 
    	}
        else
        {
                Peripheral1Type = 0;
        }
        
        BufferLen = EepromPram_ReadPram(E2_POS_PORT_ID, Buffer);
    	if(E2_POS_PORT_ID_LEN == BufferLen)
    	{
                Peripheral2Type = Public_ConvertBufferToShort(Buffer); 
    	}
        else
        {
                Peripheral2Type = 0;
        }     
}
/*********************************************************************
//函数名称	:ReadPeripheral1TypeBit
//功能		:读外设1类型的某一位
//输入		:第几位，取值为0~15
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:1:表示该外设被启用了;0:表示该外设未被启用
//备注		:参数值大于15返回值为2，表示错误 
*********************************************************************/
u8 ReadPeripheral1TypeBit(u8 Bit)
{
        if(Bit > 15)
        {
                return 2;
        }
        
        return PUBLIC_CHECKBIT_EX(Peripheral1Type,Bit);
}
/*********************************************************************
//函数名称	:ReadPeripheral2TypeBit
//功能		:读外设1类型的某一位
//输入		:第几位，取值为0~15
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:1:表示该外设被启用了;0:表示该外设未被启用
//备注		:参数值大于15返回值为2，表示错误 
*********************************************************************/
u8 ReadPeripheral2TypeBit(u8 Bit)
{
        if(Bit > 15)
        {
                return 2;
        }
        
        return PUBLIC_CHECKBIT_EX(Peripheral2Type,Bit);
}
/*********************************************************************
//函数名称	:UpdataAccOffGpsControlFlag(void)
//功能		:更新变量AccOffGpsControlFlag
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void UpdataAccOffGpsControlFlag(void)
{
	    u8	Buffer[4];
	    u8	PramLen;
	    PramLen = EepromPram_ReadPram(E2_GPS_CONTROL_ID, Buffer);
    	if(E2_GPS_CONTROL_LEN == PramLen)
    	{
		    if(0xff == Buffer[0])//没有设置过参数,默认为关
		    {
			    Buffer[0] = 0;
		    }
    		AccOffGpsControlFlag = Buffer[0];
    	}
        else
        {
            AccOffGpsControlFlag = 0;
        }
}
/*********************************************************************
//函数名称	:UpdataEvaluatorControlFlag(void)
//功能		:更新变量EvaluatorControlFlag
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void UpdataEvaluatorControlFlag(void)
{
	u8	Buffer[4];
	u8	PramLen;
	PramLen = EepromPram_ReadPram(E2_USB_UPDATA_FLAG_ID, Buffer);
    	if(E2_USB_UPDATA_FLAG_ID_LEN == PramLen)
    	{
    		EvaluatorControlFlag = Buffer[0];
    	}
}
/*********************************************************************
//函数名称	:UpdataTaximeterControlFlag(void)
//功能		:更新变量TaximeterControlFlag
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void UpdataTaximeterControlFlag(void)
{
	u8	Buffer[4];
	u8	PramLen;
	PramLen = EepromPram_ReadPram(E2_TAXIMETER_CONTROL_ID, Buffer);
    	if(E2_TAXIMETER_CONTROL_LEN == PramLen)
    	{
    		TaximeterControlFlag = Buffer[0];
    	}
}
/*********************************************************************
//函数名称	:UpdataToplightControlFlag(void)
//功能		:更新变量ToplightControlFlag
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void UpdataToplightControlFlag(void)
{
	u8	Buffer[4];
	u8	PramLen;
	PramLen = EepromPram_ReadPram(E2_TOPLIGHT_CONTROL_ID, Buffer);
    	if(E2_TOPLIGHT_CONTROL_ID_LEN == PramLen)
    	{
    		ToplightControlFlag = Buffer[0];
    	}
}
/*********************************************************************
//函数名称	:UpdataLedlightControlFlag(void)
//功能		:更新变量LedlightControlFlag
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void UpdataLedlightControlFlag(void)
{
	u8	Buffer[4];
	u8	PramLen;
	PramLen = EepromPram_ReadPram(E2_LEDLIGHT_CONTROL_ID, Buffer);
    	if(E2_LEDLIGHT_CONTROL_ID_LEN == PramLen)
    	{
    		LedlightControlFlag = Buffer[0];
    	}
}
//**************函数定义*******************
/*********************************************************************
//函数名称	:EvRing
//功能		:开启电话响铃
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void EvRing(void)
{
  /*dxl,2014.8.8根据黎工提交修改
#ifdef TTS_M12
		M12TTSPlayStr("您有新来电!",M12_TTS_RE_NO);
#else
    PlayMulTTSVoiceStr("您有新来电!");
#endif
  */
  Public_PlayTTSVoiceStr("您有新来电!");
}
  
  
/*********************************************************************
//函数名称	:EvRingOver
//功能		:关闭电话响铃
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void EvRingOver(void)
{
#ifdef TTS_M12
		//M12TTSPlayStr(" ",M12_TTS_RE_NO);
#else
    //PlayMulTTSVoiceStr(" "); 
#endif
}
/*********************************************************************
//函数名称	:SpeedFlagUpdatePram(void)
//功能		:更新SpeedFlag变量
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void SpeedFlagUpdatePram(void)
{
	u8	PramLen;
	//u8	Buffer[1] = {1};
    u8  i;
	
    for(i=0; i<100; i++)
    {
        //延时一会儿
    }
	//读取速度选择值
	PramLen = EepromPram_ReadPram(E2_SPEED_SELECT_ID, &SpeedFlag);
        if(1 == PramLen)
        {
	
	      //更新总里程数
	      if(0 == (SpeedFlag&0x01))//dxl,2015.5.11
	      {
			Pulse_UpdataPram();
	      }
	      else
	      {
		  	GpsMile_UpdatePram();
		  	MileMeter_ClearOldPoint();
	      }
        }
        else
        {
		//设置为GPS速度
               	SpeedFlag = 0x03;//为GPS速度,//dxl,2015.5.11
               	GpsMile_UpdatePram();
	       	MileMeter_ClearOldPoint();
        }
        SpeedMonitor_UpdateSpeedType();
}
/*********************************************************************
//函数名称	:SetBeepOnEvBit(u8 bit)
//功能		:设置变量BeepOnEv中的某一位
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void SetBeepOnEvBit(u8 bit)
{
	u8	i;
	u8	temp;
	u8	count;
	
	temp = bit;
	count = 0;
	for(i=0; i<8; i++)
	{
		if(0x01 == ((temp >> i)&0x01))
		{
			count++;
		}
	}
	
	if(1 == count)
	{
		BeepOnEv |= bit;
	}
}
/*******************************************************************************
* Function Name  : SetLink1LoginAuthorizationState
* Description    : 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SetLink1LoginAuthorizationState(u8 State)
{
    Link1LoginCount = 0;
	Link1AuthorizationCount = 0;
	Link1LoginAuthorizationLastState = 0;
	Link1LoginAuthorizationState = State;
        
}
/*******************************************************************************
* Function Name  : SetLink2LoginAuthorizationState
* Description    : 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SetLink2LoginAuthorizationState(u8 State)
{
    Link2LoginCount = 0;
	Link2AuthorizationCount = 0;
	Link2LoginAuthorizationLastState = 0;
	Link2LoginAuthorizationState = State;
        
}
/*******************************************************************************
* Function Name  : EvGetAkeyTask
* Description    : 获取鉴权码--走SMS短信通道 .
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void GetAkey_EvTask(void)
{	
    u8 channel = CHANNEL_DATA_1;//为编译通过而添加
	//发送获取鉴权码短信
	RadioProtocol_TerminalRegister(channel);
}
/*******************************************************************************
* Function Name  : EvChangeServerTask
* Description    : 收到指令后切换到监管平台
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ChangeServer_EvTask(void)
{
	/*
	//判断平台参数数是否正确
	if (MOD_GetNetPara(&gNet, THI_IP) == true){
		gNet.lnkParam.bits.lnkIndex = THI_IP;  //监管平台
		gNet.thirdLnkCnt = 3; //最多连接3次 
		MOD_StateSwitch(MOD_DOWNGPRS);
	}
	*/
}
/*******************************************************************************
* Function Name  : EvFtpUpdataTask
* Description    : 利用FTP下载文件
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void FtpUpdata_EvTask(void)
{

}
/*******************************************************************************
* Function Name  : EvDiallingTask
* Description    : 一键通话功能
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Dialling_EvTask(void)
{

}
/*******************************************************************************
* Function Name  : EvSevDialTask
* Description    : 电话回拨
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SevDial_EvTask(void)
{
    #ifdef USE_PHONE
    Phone_SetDialEvTask();
    #else
	u8 ucTmp[30] = {0};	//号码存储
	u8 ucFlg = 0;			//通话标志
	u8 len = 0;
	
	len = EepromPram_ReadPram(E2_PHONE_CALLBACK_ID, ucTmp);
	if (len != 0) {
		ucTmp[len] = '\0';
		EepromPram_ReadPram(E2_CALLBACK_FLAG_ID, &ucFlg);
		if (0 == ucFlg) {
			if (0 == communication_CallPhone(ucTmp, PHONE_NORMAL))
				//当前状态不允许通话，延时等待
				SetEvTask(EV_SEV_CALL);//communication_CallPhone(ucTmp, PHONE_NORMAL);
		} else {
			if (0 == communication_CallPhone(ucTmp, PHONE_MONITOR))
				//当前状态不允许通话，延时等待
				SetEvTask(EV_SEV_CALL);//communication_CallPhone(ucTmp, PHONE_NORMAL);
		}
	}
    #endif
}
/*********************************************************************
//函数名称	:Updata_SendResultNoteTimeTask
//功能		:发送升级结果通知到平台
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:默认是开启的,1秒调度1次
*********************************************************************/
FunctionalState  Updata_SendResultNoteTimeTask(void)
{
    static u8 State = 0;
    static u8 AckResult = 0;
    static u8 UpdataType = 0;
    static u8 count1 = 0;
    static u8 count2 = 0;
    
    u8  channel = CHANNEL_DATA_1;
    u8  Buffer[3] = {0};
    u8  BufferLen = 0;
    
    if(0 == State)//读取结果信息
    {
        //读升级结果
        BufferLen = FRAM_BufferRead(Buffer, FRAM_FIRMWARE_UPDATA_FLAG_LEN, FRAM_FIRMWARE_UPDATA_FLAG_ADDR);
        if(0 == BufferLen)
        {
            return DISABLE;
        }
        else if((1 == Buffer[0])||(3 == Buffer[0]))//1:表示平台下发过远程升级指令,2:升级成功,3:升级失败
        {
            AckResult = 1;
        }
        else if(2 == Buffer[0])
        {
            AckResult = 0;
        } 
        else
        {
            return DISABLE;
        }
        //读取升级类型
        BufferLen = FRAM_BufferRead(Buffer, FRAM_FIRMWARE_UPDATA_TYPE_LEN, FRAM_FIRMWARE_UPDATA_TYPE_ADDR);
        if(0 == BufferLen)
        {
            return DISABLE;
        }
        else if((0 == Buffer[0])||(0x09 == Buffer[0]))
        {
                  State = 1;//继续下一步
                  count1 = 0;
                  count2 = 0;
                  UpdataType = Buffer[0];
        }
        else
        {
            return DISABLE;
        }
  
    }
    else if(1 == State)//发送升级失败结果通知给平台
    {
         //读取通道号,当前默认为CHANNEL_DATA_1
         RadioProtocol_UpdataResultNote(channel,UpdataType,AckResult);
         State = 2;
    }
    else if(2 == State)//检测应答
    {
        if(1 == UpdataResultNoteAckFlag)
        {
            UpdataResultNoteAckFlag = 0;
            State = 0;
            Buffer[0] = 0;
            FRAM_BufferWrite(FRAM_FIRMWARE_UPDATA_FLAG_ADDR,Buffer,1);//写入铁电下次就不会发送结果通知了
            return DISABLE;
        }
        else
        {
            count1++;
            if(count1 >= 10)//连续发送3次没有应答的话就不发送了
            {
                count1 = 0;
                State = 1;
                count2++;
                if(count2 > 3)
                {
                    count2 = 0;
                    Buffer[0] = 0;
                    FRAM_BufferWrite(FRAM_FIRMWARE_UPDATA_FLAG_ADDR,Buffer,1);//写入铁电下次就不会发送结果通知了
                    return DISABLE;
                }
            }
                       
        }
    }
    return ENABLE;
}
/*********************************************************************
//函数名称	:Reset_TimeTask
//功能		:系统定时重启
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:
*********************************************************************/
/*
FunctionalState Reset_TimeTask(void)
{
	u8 ACC;
	
	if(1 == ResetEnableFlag)
	{
		NVIC_SystemReset();
	}
	//读ACC状态
	ACC = Io_ReadStatusBit(STATUS_BIT_ACC);
	if(0 == ACC)	//关闭
	{
		NVIC_SystemReset();
		return DISABLE;
	}
	else	//开启
	{
		SetTimerTask(TIME_SYSRESET, 60*SECOND);
		return ENABLE;
	}
}
*/
FunctionalState Reset_TimeTask(void)
{
	u8 flag;
        u8 speed;
	static u32 count = 0;
        
        count++;//1秒加1
        if(count >= (24*3600))//满24小时
        {
               // flag = Report_GetBlindStatusFlag();//读盲区标志,1为盲区,0为在线,判断盲区的方法是：成功发送1条位置信息开始计时，
                                                  //在3倍的上报时间间隔内没有收到任何平台下发的指令认为是盲区,一旦读取到盲区标志置为1位置汇报信息直接存入flash,不上报了
                 flag = communicatio_GetMudulState(COMMUNICATE_STATE);                              //需不需要限定上报的时间间隔必须大于某个值,如3秒以上
                if(1 == flag)
                {
                        speed = Gps_ReadSpeed();//读gps速度,用gps速度通用一些
                        if(0 == speed)
                        {
                                NVIC_SystemReset();//只有满足时间满24小时,处于盲区状态,速度为0三个条件时才重启
                        }
                }
                else
                {
                        count = 0;
                }
        }
	if(1 == ResetEnableFlag)//在某些特殊情况下会要求立即重启,可以通过置位ResetEnableFlag来实现
	{
		NVIC_SystemReset();
	}
        
	return ENABLE;
}
/*******************************************************************************
* Function Name  : DelayTrigTimeTask(void)
* Description    : 延时触发
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
FunctionalState DelayTrigTimeTask(void)
{
        u8      flag;
        u16     length;
        u8      Buffer[FLASH_BLIND_STEP_LEN+1];
        u8      Len;
	    u8	Dir[15];
        UINT      Byte;
        u8	FileName[_MAX_LFN];
        u32     MediaID;
        FIL	file;
        MESSAGE_HEAD head;
        u16 SerialNum;
        u8 i;
        u8 channel = CHANNEL_DATA_1|CHANNEL_DATA_2;
        
        if(0 == DelayAckCommand)
        {
                return DISABLE;
        }

        flag = 0;
        switch(DelayAckCommand)
        {
                case 0:
                {
                        break;
                }
                case 0x0800://上发多媒体事件
                {
                        RadioProtocol_MultiMediaEvenReport(channel,PhotoID+1, MediaType, MediaFormat, MediaEvenCode, MediaChannelID);
                        break;
                }
                case 0x0900://数据上行透传,透传伊爱自定义扩展协议
                {
                         RadioProtocol_OriginalDataUpTrans(channel,0xFD,AccelerationAckBuffer,AccelerationAckBufferLen);
                         break;
                }
                case 0x0500://0x0500车辆控制
                {
                        //应答流水号
                        length = 0;
                        Buffer[0] = (CarControlAckSerialNum & 0xff00) >> 8;
                        Buffer[1] = CarControlAckSerialNum & 0xff;
                        length += 2;
                        //获取位置信息,拷贝到Buffer中
	                    Len = Report_GetPositionInfo(Buffer+2);
                        //if(PramLen > 28)
                        //{
                               //PramLen = 28;
                        //}
	                    length += Len;
                        //发送位置汇报信息
	                //消息ID
	                    head.MessageID = 0x0500;
	                    head.Attribute = length;
                        //发送
			            if(0 == RadioProtocol_SendCmd(DelaySendChannel,head, Buffer, length, DelaySmsPhone))
			            {
				            flag = 1;
			            }
                        break;
                }
		case 0x0003://0x0003,终端注销
		{
			//TerminalAuthorizationFlag = 0;dxl,2014.11.26收到注销应答后才把鉴权标志清0
			break;
		}
        case 2:
        {
			 Blind_Erase(0xff);//升级前擦除一次盲区flash myh 130808
             ClrTimerTask(TIME_POSITION);//升级过程中关闭位置信息上报及盲区 myh 130808，原因是防止对同一块flash区域写
             ClrTimerTask(TIME_BLIND);
             Ftp_Updata_BusyFlg = 1;//置升级忙标志 myh 130808
             // 触发无线升级任务
			 SetEvTask(EV_FTP_UPDATA);
             break;
        }
		case 4:
		{
			//切换为断电状态，之后通讯模块内部会重新启动,连接默认的主服务器
			break;
		}
		case 5:
		{
			//重启
			 NVIC_SystemReset();
			break;
		}
		case 6:
                {
			// 触发连接监管平台任务
			SetEvTask(EV_LINK_GOV_SERVER);
                        break;
                }
                case 7:
                {
                        MOD_CloseMainIp();//模块主动断开连接
                        DelayAckCommand = 5;//下一步重启
                        flag = 1;
                        break;
                }
                case 8:
                {
                         //关Communication_TimeTask任务
                        //ClrTimerTask(TIME_COMMUNICATION);实际应用时不关闭任务
                        //先关闭模块电源
                        GSM_PWR_OFF(); 
                        break;
                }
                case 9:
                {
                        MOD_CloseMainIp();//模块主动断开连接
                        DelayAckCommand = 10;//下一步是关闭通信模块任务
                        //触发延时任务
                        SetTimerTask(TIME_DELAY_TRIG, 5*SECOND);
                        TerminalAuthorizationFlag = 0;
                        flag = 1;
                        break;
                }
                case 10:
                {
                         //关Communication_TimeTask任务
                        //ClrTimerTask(TIME_COMMUNICATION);实际应用时不关闭任务
                        //先关闭模块电源
                        GSM_PWR_OFF(); 
#ifdef HUOYUN_DEBUG_OPEN
                        OneHourDelayTimeCountEnableFlag = 1;//时间计数使能,在心跳任务里计数，计数达到1小时后重连
                        OneHourDelayTimeCount = 0;
#else
                          //10分钟后重启
                        ResetEnableFlag = 1;
                        SetTimerTask(TIME_SYSRESET, SYSTICK_10MINUTE);
#endif
                      
                        
                     
                        break;
                }
                case 11:
                {
                        MOD_CloseMainIp();//模块主动断开连接
                        DelayAckCommand = 12;//下一步是关机
                        //触发延时任务
                        SetTimerTask(TIME_DELAY_TRIG, 5*SECOND);
                        TerminalAuthorizationFlag = 0;
                        flag = 1;
                        break;
                }
                case 12:
                {
                        SleepDeep_DisposeTerminalShutdown();//ACC ON重启
                        break;
                }
                case 13:
                {
                          if(ACK_OK == RadioProtocol_OriginalDataUpTrans(channel,0x41, DelayBuffer, DelayBufferLen))
                          {
                          
                          }
                          else
                          {
                                flag = 1;
                          }
                          break;
                }
                case 0x8801://接收到立即拍照指令后发送拍照应答指令,只应答1张图片
                {
                        //应答流水号
                        Buffer[0] = (RecorderSerialNum&0xff00)>>8;
                        Buffer[1] = RecorderSerialNum;
                        channel = RadioProtocol_GetDownloadCmdChannel(0x8801, &SerialNum);
                        if(0 == MediaChannelID)
                        {
                            for(i=1; i<=CAMERA_ID_MAX; i++)
                            {
                                if(1 == Camera[i-1].OnOffFlag)
                                {
                                    break;
                                }
                            }
                            if(i == (CAMERA_ID_MAX+1))
                            {
                                Buffer[2] = 2;//通道不支持
                                //多媒体ID个数
                                Buffer[3] = 0;//默认为1
                                Buffer[4] = 1;
                                //多媒体ID
                                Buffer[5] = (PhotoID&0xff000000) >> 24;
                                Buffer[6] = (PhotoID&0xff0000) >> 16;
                                Buffer[7] = (PhotoID&0xff00) >> 8;
                                Buffer[8] = PhotoID&0xff;
                                RadioProtocol_PhotoAck(channel,Buffer,9);
                            }
                            else
                            {
                                Buffer[2] = 0;//成功
                                //多媒体ID个数
                                Buffer[3] = 0;//默认为1
                                Buffer[4] = 1;
                                //多媒体ID
                                Buffer[5] = (PhotoID&0xff000000) >> 24;
                                Buffer[6] = (PhotoID&0xff0000) >> 16;
                                Buffer[7] = (PhotoID&0xff00) >> 8;
                                Buffer[8] = PhotoID&0xff;
                                RadioProtocol_PhotoAck(channel,Buffer,9);
                            }
                              
                        }
                        else
                        {
                            if(1 == Camera[MediaChannelID-1].OnOffFlag)
                            {
                                Buffer[2] = 0;//成功
                                //多媒体ID个数
                                Buffer[3] = 0;//默认为1
                                Buffer[4] = 1;
                                //多媒体ID
                                Buffer[5] = (PhotoID&0xff000000) >> 24;
                                Buffer[6] = (PhotoID&0xff0000) >> 16;
                                Buffer[7] = (PhotoID&0xff00) >> 8;
                                Buffer[8] = PhotoID&0xff;
                                RadioProtocol_PhotoAck(channel,Buffer,9);   
                            }
                            else
                            {
                                Buffer[2] = 2;//通道不支持
                                //多媒体ID个数
                                Buffer[3] = 0;//默认为1
                                Buffer[4] = 1;
                                //多媒体ID
                                Buffer[5] = (PhotoID&0xff000000) >> 24;
                                Buffer[6] = (PhotoID&0xff0000) >> 16;
                                Buffer[7] = (PhotoID&0xff00) >> 8;
                                Buffer[8] = PhotoID&0xff;
                                RadioProtocol_PhotoAck(channel,Buffer,9);
                            }
                        }
                       
                        break;
                }
                case 0x8804://录音完成后把最近的一段录音(1分钟内)上传到监控中心,该功能仅仅为行标北斗测试添加
                {
                        //检测是否正处于上传状态
	                if((0 == MultiMediaSendFlag)&&(0 == strlen((char const *)MemoryUploadFileName)))
	                {
		                strcpy((char *)Dir,"1:/WAV");
	                        if(CurrentWavFolder < 10)
	                        {
		                        Dir[6] = CurrentWavFolder+0x30;
		                        Dir[7] = 0;
	                        }
	                        else
	                        {
		                        Dir[6] = CurrentWavFolder/10+0x30;
		                        Dir[7] = CurrentWavFolder%10+0x30;
		                        Dir[8] = 0;
	                        }
	                        //写日志文档log.bin
	                        strcpy((char *)FileName,(char const *)Dir);
	                        length = strlen((char const *)FileName);
	                        memcpy(FileName+length,"/log.bin",8);
	                        FileName[length+8] = 0;
                                Buffer[0] = 0;
                                MediaID = 0;
	                        if(FR_OK == f_open(&file,ff_NameConver(FileName),FA_READ))
	                        {
                                        if(file.fsize >= 9)
                                        {
		                                f_lseek(&file,file.fsize-9);
		                                if(FR_OK == f_read (&file, Buffer, 9, &Byte))
                                                {
                                                        MediaID = 0;
                                                        MediaID |= Buffer[4] << 24;
                                                        MediaID |= Buffer[5] << 16;
                                                        MediaID |= Buffer[6] << 8;
                                                        MediaID |= Buffer[7];
                                                }
                                        }
		                        f_close(&file);
	                        }
                                if(0 != MediaID)
                                {
                                        Buffer[0] = 0;
                                        MediaSearch_GetMediaIDFileName(Buffer,MEDIA_TYPE_WAV, MediaID);
                                        if(strlen((char const *)Buffer))
                                        {
                                                strcpy((char *)MemoryUploadFileName,(char const *)Buffer);
                                                //开启多媒体数据上传任务
				                SetTimerTask(TIME_SEND_MEDIA, 10*SYSTICK_0p1SECOND);
                                        }
                                }
                        }
                }
                default :break;
        }

        if(0 == flag)
        {
                DelayAckCommand  = 0;
                return DISABLE;
        }
        else
        {
                return ENABLE;
        }
}
/*********************************************************************
//函数名称	:GetTerminalAuthorizationFlag(void)
//功能		:获取鉴权标志的值
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:bit0:保留;bit1:CHANNEL_DATA_1;bit2:CHANNEL_DATA_2;bit3~bit7:保留,
/           :相应位为1表示对应通道鉴权成功,否则为失败
*********************************************************************/
u8 GetTerminalAuthorizationFlag(void)
{
	return 	TerminalAuthorizationFlag;
}
/*********************************************************************
//函数名称	:SetTerminalAuthorizationFlag(void)
//功能		:置位某个通道的鉴权标志
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:bit0:空缺或预留;bit1:表示CHANNEL_DATA_1，bit2:CHANNEL_DATA_2
//          :channel的值可以为以上两个通道值或者是它们的组合
*********************************************************************/
void SetTerminalAuthorizationFlag(u8 channel)
{
    if(CHANNEL_DATA_1 == (channel&CHANNEL_DATA_1))
    {
        TerminalAuthorizationFlag = TerminalAuthorizationFlag | CHANNEL_DATA_1;
    }
    
    if(CHANNEL_DATA_2 == (channel&CHANNEL_DATA_2))
    {
        TerminalAuthorizationFlag = TerminalAuthorizationFlag | CHANNEL_DATA_2;
    }
	
}
/*********************************************************************
//函数名称	:ClearTerminalAuthorizationFlag(u8 channel)
//功能		:清除某个通道的鉴权标志
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:bit0:空缺或预留;bit1:表示CHANNEL_DATA_1，bit2:CHANNEL_DATA_2
//          :channel的值可以为以上两个通道值或者是它们的组合
*********************************************************************/
void ClearTerminalAuthorizationFlag(u8 channel)
{
    if(CHANNEL_DATA_1 == (channel&CHANNEL_DATA_1))
    {
        TerminalAuthorizationFlag &= ~CHANNEL_DATA_1;
    }
    if(CHANNEL_DATA_2 == (channel&CHANNEL_DATA_2))
    {
        TerminalAuthorizationFlag &= ~CHANNEL_DATA_2;
    }
		if(CHANNEL_DATA_3 == (channel&CHANNEL_DATA_3))
    {
        TerminalAuthorizationFlag &= ~CHANNEL_DATA_3;
    }
}
/*********************************************************************
//函数名称	:ClearTerminalAuthorizationCode(u8 channel)
//功能		:清除某个通道的鉴权码
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:bit0:空缺或预留;bit1:表示CHANNEL_DATA_1，bit2:CHANNEL_DATA_2
//          :channel的值可以为以上两个通道值或者是它们的组合
*********************************************************************/
void ClearTerminalAuthorizationCode(u8 channel)
{
    u8  i;
    
    if(CHANNEL_DATA_1 == (channel&CHANNEL_DATA_1))
    {
         for(i=0; i<20; i++)
         {
            E2prom_WriteByte(E2_LOGINGPRS_PASSWORD_ID_ADDR+i,0xff);
         }
    }
   if(CHANNEL_DATA_2 == (channel&CHANNEL_DATA_2))
    {
         for(i=0; i<20; i++)
         {
            E2prom_WriteByte(E2_SECOND_LOGINGPRS_PASSWORD_ID_ADDR+i,0xff);
         }
    }
}
/*******************************************************************************
* Function Name  : MultiCenterConnect_TimeTask(void)
* Description    : 多中心连接定时任务,收到设置备用服务器命令时开启,1秒钟调度1次
* Input          : None
* Output         : None
* Return         : 仅仅为行标北斗测试添加
*******************************************************************************/
FunctionalState MultiCenterConnect_TimeTask(void)
{
	//行标检测时使用
	/*
        static u8 count = 0;
        u32 temp;
        
        u8  Buffer[30];
        u8  PramLen;
        
        count++;
        if(1 == count)//关闭位置信息上报功能和心跳包功能,置位多媒体发送标志
        {
                //心跳包设置3分钟,防止终端主动下线（2.5个心跳包时间没收到数据下线）
                temp = 180;
	        Buffer[0] = (temp & 0xff000000) >> 24;
	        Buffer[1] = (temp & 0xff0000) >> 16;
	        Buffer[2] = (temp & 0xff00) >> 8;
	        Buffer[3] = temp;
	        if(0 == EepromPram_WritePram(E2_TERMINAL_HEARTBEAT_ID, Buffer, E2_TERMINAL_HEARTBEAT_LEN))
	        {
		        EepromPram_WritePram(E2_TERMINAL_HEARTBEAT_ID, Buffer, E2_TERMINAL_HEARTBEAT_LEN);
	        }
                EepromPram_UpdateVariable(E2_TERMINAL_HEARTBEAT_ID);
                //ClrTimerTask(TIME_POSITION);,多中心连接时是需要关闭，为测试盲区不关闭定时上报
                ClrTimerTask(TIME_HEART_BEAT);
                MultiMediaSendFlag = 1;//防止有位置信息上传
        }
        else if(32 == count)//发送第1次鉴权命令
        {
                if(ACK_OK == RadioProtocol_TerminalQualify())
                {
                        BdTestCount1++;
                }
                else
                {
                        count--;//再发一遍
                }
        }
        else if(53 == count)//发送第2次鉴权命令
        {
                if(ACK_OK == RadioProtocol_TerminalQualify())
                {
                        BdTestCount1++;
                }
                else
                {
                        count--;//再发一遍
                }
        }
        else if(93 == count)//发送第3次鉴权命令
        {
                if(ACK_OK == RadioProtocol_TerminalQualify())
                {
                        BdTestCount1++;
                }
                else
                {
                        count--;//再发一遍
                }
        }
        else if(100 == count)//把主ip跟备份ip设置一样,重新启动连接备份服务器
        {
                //心跳包设置回30秒,这是检测平台开始进入该任务前设置的值
                temp = 180;//行标检测修改
	        Buffer[0] = (temp & 0xff000000) >> 24;
	        Buffer[1] = (temp & 0xff0000) >> 16;
	        Buffer[2] = (temp & 0xff00) >> 8;
	        Buffer[3] = temp;
	        if(0 == EepromPram_WritePram(E2_TERMINAL_HEARTBEAT_ID, Buffer, E2_TERMINAL_HEARTBEAT_LEN))
	        {
		        EepromPram_WritePram(E2_TERMINAL_HEARTBEAT_ID, Buffer, E2_TERMINAL_HEARTBEAT_LEN);
	        }
                EepromPram_UpdateVariable(E2_TERMINAL_HEARTBEAT_ID);
                //读取备份ip
                PramLen = EepromPram_ReadPram(E2_BACKUP_SERVER_IP_ID, Buffer);
                //写主ip
                EepromPram_WritePram(E2_MAIN_SERVER_IP_ID, Buffer,PramLen);
                //重新启动
                //NVIC_SystemReset();
                count = 0;
                MultiMediaSendFlag = 0;
                Communication_Close();
                Communication_Open();
                SetTimerTask(TIME_POSITION, SYSTICK_1SECOND);
                SetTimerTask(TIME_HEART_BEAT, SYSTICK_1SECOND);
                MultiCenterLinkFlag = 0;
                FRAM_BufferWrite(FRAM_MULTI_CENTER_LINK_ADDR, &MultiCenterLinkFlag, 1);
                return DISABLE;
        } 
        return ENABLE;
	*/
	return DISABLE;
}
/*******************************************************************************
* Function Name  : RadioProtocolStartRecord(void)
* Description    : 下发录音开始指令,录20秒后上传至监控中心
* Input          : None
* Output         : None
* Return         : 仅仅为行标北斗测试添加
*******************************************************************************/
void RadioProtocolStartRecord(void)
{

}
//为编译不报警告而添加，已实际各模块使用为准，在实际使用时该定义可以直接去掉 dxl
void TexiService_PlayTTSVoiceStr(u8 *StrPtr)
{

}
/********************************************************************
* 名称 : Print_RTC
* 功能 : 截取当前的RTC时间,打印输出
********************************************************************/
void Print_RTC( void )
{
  
  TIME_T nowTime;
  RTC_GetCurTime(&nowTime);
  LOG_PR( "%02d:%02d:%02d%t ;", nowTime.hour,nowTime.min,nowTime.sec );
 
}
/********************************************************************
* 名称 : SetCostDownVersionFlag
* 功能 : 设置CostDown版本标志位，该位为1表示PCB为降成本版本，为0表示之前的版本
********************************************************************/
void SetCostDownVersionFlag( void )
{
   CostDownVersionFlag = 1; 
}
/********************************************************************
* 名称 : ClearCostDownVersionFlag
* 功能 : 清除CostDown版本标志位，该位为1表示PCB为降成本版本，为0表示之前的版本
********************************************************************/
void ClearCostDownVersionFlag( void )
{
   CostDownVersionFlag = 0; 
}
/********************************************************************
* 名称 : GetCostDownVersionFlag
* 功能 : 读取CostDown版本标志位，该位为1表示PCB为降成本版本，为0表示之前的版本
********************************************************************/
u8 GetCostDownVersionFlag( void )
{
   return CostDownVersionFlag; 
}
/********************************************************************
* 名称 : GansuOiL_OilProtocolCheck
* 功能 : 甘肃电信油耗协议检测,油耗传感器30秒发送一条数据给GPS终端
********************************************************************/
u8 GanSuOil_OilProtocolCheck(u8 *pBuffer, u8 length)
{
   
    if(((length > 70)||(1 == GanSuOilBufferBusyFlag))&&(length > 2))
    {
        return 0;
    }
    if(('S'==*pBuffer)
       &&('t' == *(pBuffer+1))
         &&('E' == *(pBuffer+length-2))
           &&('d' == *(pBuffer+length-1)))
    {
        memcpy(GanSuOilBuffer,pBuffer,length);  
        GanSuOilBufferLen = length;
    }
    return length;
}
/********************************************************************
* 名称 : GanSuOiL_GetOnePacketData
* 功能 : 获取一包油耗数据，位置汇报时调用该函数
********************************************************************/
u8 GanSuOil_GetOnePacketData(u8 *pBuffer)
{
    if((GanSuOilBufferLen > 70)&&(0 == GanSuOilBufferLen))//长度超出
    {
        return 0;
    }
  
    memcpy(pBuffer,GanSuOilBuffer,GanSuOilBufferLen);  
    return GanSuOilBufferLen;
}
/********************************************************************
* 名称 : GanSuOiL_GetCurPacketLen
* 功能 : 获取当前油耗数据包长度
********************************************************************/
u8 GanSuOil_GetCurPacketLen(void)
{
    return GanSuOilBufferLen;
}
/********************************************************************
* 名称 : Acceleration_DisposeRadioProtocol
* 功能 : 急加速急减速指令解析
********************************************************************/
void Acceleration_DisposeRadioProtocol(u8 cmd, u8 *pBuffer, u16 length)
{
		u8 *p = NULL;
		
		if(0x02 == cmd)//急加速
		{
				p = pBuffer;
				if((7 == length)&&(1 == *(p+2)))//写
				{
						AccelerationAckBuffer[0] = 0x01;
						AccelerationAckBuffer[1] = 0x01;
						AccelerationAckBuffer[2] = 0xF7;
						AccelerationAckBuffer[3] = 0x02;
						AccelerationAckBuffer[4] = 0x00;					
						AccelerationAckBuffer[5] = 0x03;					
						memcpy(AccelerationAckBuffer+6,p,3);
						AccelerationAckBufferLen = 9;
						FRAM_BufferWrite(FRAM_ACCELERATION_ADDR,p+3,4);
						Set_Emer_Threshold(p+3,0);//zengliang add 2016-11-21
					
					CleanInitFlag();
				}
				else if((3 == length)&&(0 == *(p+2)))//读
				{
						AccelerationAckBuffer[0] = 0x01;
						AccelerationAckBuffer[1] = 0x01;
						AccelerationAckBuffer[2] = 0xF7;
						AccelerationAckBuffer[3] = 0x02;
						AccelerationAckBuffer[4] = 0x00;
						AccelerationAckBuffer[5] = 0x07;	
						memcpy(AccelerationAckBuffer+6,p,3);		
						FRAM_BufferRead(AccelerationAckBuffer+9,4,FRAM_ACCELERATION_ADDR);
						AccelerationAckBufferLen = 13;
				}
				DelayAckCommand = 0x0900;
				SetTimerTask(TIME_DELAY_TRIG,SECOND);
		}
		else if(0x03 == cmd)//急减速
		{
				p = pBuffer;
				if((5 == length)&&(1 == *(p+2)))//写
				{
						AccelerationAckBuffer[0] = 0x01;
						AccelerationAckBuffer[1] = 0x01;
						AccelerationAckBuffer[2] = 0xF7;
						AccelerationAckBuffer[3] = 0x03;
						AccelerationAckBuffer[4] = 0;
						AccelerationAckBuffer[5] = 0x03;	
						memcpy(AccelerationAckBuffer+6,p,3);
						AccelerationAckBufferLen = 9;
						FRAM_BufferWrite(FRAM_DECELERATION_ADDR,p+3,2);
						Set_Emer_Threshold(p+3,1);//zengliang add 2016-11-21
					
					CleanInitFlag();
				}
				else if((3 == length)&&(0 == *(p+2)))//读
				{
						AccelerationAckBuffer[0] = 0x01;
						AccelerationAckBuffer[1] = 0x01;
						AccelerationAckBuffer[2] = 0xF7;
						AccelerationAckBuffer[3] = 0x03;
						AccelerationAckBuffer[4] = 0;
						AccelerationAckBuffer[5] = 0x05;	
						memcpy(AccelerationAckBuffer+6,p,3);		
						FRAM_BufferRead(AccelerationAckBuffer+9,2,FRAM_DECELERATION_ADDR);
						AccelerationAckBufferLen = 11;
				}
				DelayAckCommand = 0x0900;
				SetTimerTask(TIME_DELAY_TRIG,SECOND);
		}
}

void Acceleration_Turn_DisposeRadioProtocol(u8 cmd, u8 *pBuffer, u16 length)
{
		u8 *p = NULL;
		u16 TurnSerilialNum=0;
		if(0x04 == cmd)//急转弯相关参数
		{
				p = pBuffer;
				TurnSerilialNum |= *(p+1) << 8;
				TurnSerilialNum |= *(p+2);
			
				AccelerationAckBuffer[0]=0x01;
				AccelerationAckBuffer[1]=0x01;
				AccelerationAckBuffer[2]=0xF5;
				AccelerationAckBuffer[3]=0x04;	
				AccelerationAckBuffer[4]=0;
				AccelerationAckBuffer[5]=6; 
				AccelerationAckBuffer[6] = 0x14;
			//流水号
				AccelerationAckBuffer[7] = *(p+1) << 8;
				AccelerationAckBuffer[8] = *(p+2);
			//读写标志	
			AccelerationAckBuffer[9] = *(p+3);
			if(0x14 == *p)//急转弯
				{
						if((0 == *(p+3))&&(4 == length))//读急转弯角度
						{
							FRAM_BufferRead(AccelerationAckBuffer+10,2,FRAM_TURN_ANGLE_ADDR);
						}
						else if((1 == *(p+3))&&(6 == length))//写
						{
							FRAM_BufferWrite(FRAM_TURN_ANGLE_ADDR,p+4,2);
							memcpy(AccelerationAckBuffer+10,p+4,2);
							Set_Emer_Threshold(p+4,2);
							
							CleanInitFlag();
						}
				}
				AccelerationAckBufferLen = 12;
				DelayAckCommand = 0x0900;
				SetTimerTask(TIME_DELAY_TRIG,SECOND);
		}
}


/*******************************************************************************
* Function Name  : Link1Login_TimeTask
* Description    : 连接1登录
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
FunctionalState  Link1Login_TimeTask(void)
{
	u8  ucTmp[30];
        static u8   errorcount = 0;
        
        Modem_Printf("Link1ManualCloseFlag = %d\r\n",Link1ManualCloseFlag);
        Modem_Printf("Link1LoginAuthorizationState = %d\r\n",Link1LoginAuthorizationState);
    
	if(CHANNEL_DATA_1 == (CHANNEL_DATA_1&GetTerminalAuthorizationFlag()))
	{
		Link1LoginAuthorizationState = LOGIN_CLOSE_TASK;
	}
	
	switch(Link1LoginAuthorizationState)
	{
	    case LOGIN_READ_AUTHORIZATION_CODE://读取鉴权码
	        {
                Link1LoginCount = 0;
                errorcount = 0;
			    Link1AuthorizationCount = 0;
			    Link1LoginAuthorizationLastState = 0;
                Link1LoginAuthorizationDelayCount = 0;
			    if(0 == EepromPram_ReadPram(E2_LOGINGPRS_PASSWORD_ID, ucTmp))//失败
			    {
				    Link1LoginAuthorizationState = LOGIN_SEND_LOGIN_CMD;
			    }
			    else//成功
			    {
				    Link1LoginAuthorizationState = LOGIN_SEND_AUTHORIZATION_CMD;
			    }
			    break;
		    }
	    case LOGIN_SEND_LOGIN_CMD://发送注册命令
		    {
			    if(ACK_OK == RadioProtocol_TerminalRegister(CHANNEL_DATA_1))
                {
                    Net_Other_Printf(PRTF_REGISTER);
			        Link1LoginAuthorizationDelayCount = 0;
			        Link1LoginAuthorizationDelayTime = 30;//注册鉴权延时时间
			        Link1LoginAuthorizationLastState = LOGIN_SEND_LOGIN_CMD;//上一次状态
			        Link1LoginCount++;
			        if(Link1LoginCount >= 3)
			        {
				        Link1LoginAuthorizationState = LOGIN_CLOSE_GPRS;
			        }
			        else
			        {
				        Link1LoginAuthorizationState = LOGIN_DELAY;
			        }
                    errorcount = 0;
                }
                else
                {
                    errorcount++;
                    if(errorcount >= 200)
                    {
                        Link1LoginAuthorizationState = LOGIN_CLOSE_TASK;
                    }
                }
			    break;
		    }
	    case LOGIN_SEND_AUTHORIZATION_CMD://发送鉴权命令
		    {
			    if(ACK_OK == RadioProtocol_TerminalQualify(CHANNEL_DATA_1))
                {
                    Net_Other_Printf(PRTF_AUTHORIZATION);
			        Link1LoginAuthorizationDelayCount = 0;
                    Link1LoginAuthorizationDelayTime = 30;//注册鉴权延时时间,给1秒做网络延时
			        Link1LoginAuthorizationLastState = LOGIN_SEND_AUTHORIZATION_CMD;//上一次状态
			        Link1AuthorizationCount++;
                    if(Link1AuthorizationCount >= 3)
			        {
				        Link1LoginAuthorizationState = LOGIN_SEND_LOGIN_CMD;
			        }
			        else
			        {
				        Link1LoginAuthorizationState = LOGIN_DELAY;
			        }
                    errorcount = 0;
                }
                else
                {
                    errorcount++;
                    if(errorcount >= 200)
                    {
                         Link1LoginAuthorizationState = LOGIN_CLOSE_TASK;
                    }
                }
			    break;
		    }
	    case LOGIN_CLOSE_GPRS://断开网络
		    {
                Net_First_Close();
                Link1ManualCloseFlag = 1;
                ClearTerminalAuthorizationFlag(CHANNEL_DATA_1);//鉴权标志清0
                ClearTerminalAuthorizationCode(CHANNEL_DATA_1);
                Link1LoginAuthorizationDelayCount = 0;
                Link1LoginAuthorizationDelayTime = 10*60;//注册鉴权延时时间，真服务器延时10分钟
                Link1LoginAuthorizationLastState = LOGIN_RECONNECT_GPRS;
                Link1LoginAuthorizationState = LOGIN_DELAY;	
    			break;
			    
		    }
	    case LOGIN_DELAY://延时
		    {
			    Link1LoginAuthorizationDelayCount++;
			    if(Link1LoginAuthorizationDelayCount >= Link1LoginAuthorizationDelayTime)
			    {
				    Link1LoginAuthorizationDelayCount = 0;
				    Link1LoginAuthorizationState = Link1LoginAuthorizationLastState;
			    }
			    break;
		    }
	    case LOGIN_RECONNECT_GPRS://重连网络
		    {
                Link1LoginCount = 0;
                errorcount = 0;
                Link1AuthorizationCount = 0;
                Link1LoginAuthorizationLastState = 0;
                Link1LoginAuthorizationState = 0;
                Link1LoginAuthorizationDelayCount = 0; 
                Net_First_Open();
                Link1ManualCloseFlag = 0;
    			Link1LoginAuthorizationState = LOGIN_CLOSE_TASK;
    			break;
		    }
	    case LOGIN_CLOSE_TASK://关闭任务
		    {
                if(1 == Link1ManualCloseFlag)
                {
                    Net_First_Open();
                    Link1ManualCloseFlag = 0;
                }
                Link1LoginCount = 0;
                errorcount = 0;
                Link1AuthorizationCount = 0;
                Link1LoginAuthorizationLastState = 0;
                Link1LoginAuthorizationState = 0;
                Link1LoginAuthorizationDelayCount = 0; 
                return DISABLE;
		    }
	    default:
            {
                Link1LoginAuthorizationState = LOGIN_CLOSE_TASK;
            }
		break;
	}
        
       
        
	return ENABLE;
	
}
/*******************************************************************************
* Function Name  : Link2Login_TimeTask
* Description    : 连接2登录
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
FunctionalState  Link2Login_TimeTask(void)
{
	u8  ucTmp[30];

        static u8   errorcount = 0;
        
        Modem_Printf("Link2ManualCloseFlag = %d\r\n",Link2ManualCloseFlag);
        Modem_Printf("Link2LoginAuthorizationState = %d\r\n",Link2LoginAuthorizationState);
	
	if(CHANNEL_DATA_2 == (CHANNEL_DATA_2&GetTerminalAuthorizationFlag()))
	{
		Link2LoginAuthorizationState = LOGIN_CLOSE_TASK;
	}
	
	switch(Link2LoginAuthorizationState)
	{
	    case LOGIN_READ_AUTHORIZATION_CODE://读取鉴权码
	        {
                Link2LoginCount = 0;
                errorcount = 0;
			    Link2AuthorizationCount = 0;
			    Link2LoginAuthorizationLastState = 0;
                Link2LoginAuthorizationDelayCount = 0;
			    if(0 == EepromPram_ReadPram(E2_SECOND_LOGINGPRS_PASSWORD_ID, ucTmp))//需要修改,第2个链接的鉴权码还没定义
			    {
				    Link2LoginAuthorizationState = LOGIN_SEND_LOGIN_CMD;
			    }
			    else//成功
			    {
				    Link2LoginAuthorizationState = LOGIN_SEND_AUTHORIZATION_CMD;
			    }
			    break;
		    }
	    case LOGIN_SEND_LOGIN_CMD://发送注册命令
		    {
			    if(ACK_OK == RadioProtocol_TerminalRegister(CHANNEL_DATA_2))
                {
                    Net_Other_Printf(PRTF_REGISTER);
			        Link2LoginAuthorizationDelayCount = 0;
			        Link2LoginAuthorizationDelayTime = 30;//注册鉴权延时时间
			        Link2LoginAuthorizationLastState = LOGIN_SEND_LOGIN_CMD;//上一次状态
			        Link2LoginCount++;
			        if(Link2LoginCount >= 3)
			        {
				        Link2LoginAuthorizationState = LOGIN_CLOSE_GPRS;
			        }
			        else
			        {
				        Link2LoginAuthorizationState = LOGIN_DELAY;
			        }
                    errorcount = 0;
                }
                else
                {
                    errorcount++;
                    if(errorcount >= 200)
                    {
                        Link2LoginAuthorizationState = LOGIN_CLOSE_TASK;
                    }
                }
			    break;
		    }
	    case LOGIN_SEND_AUTHORIZATION_CMD://发送鉴权命令
		    {
			    if(ACK_OK == RadioProtocol_TerminalQualify(CHANNEL_DATA_2))
                {
                    Net_Other_Printf(PRTF_AUTHORIZATION);
			        Link2LoginAuthorizationDelayCount = 0;
                    Link2LoginAuthorizationDelayTime = 30;//注册鉴权延时时间,给1秒做网络延时
			        Link2LoginAuthorizationLastState = LOGIN_SEND_AUTHORIZATION_CMD;//上一次状态
			        Link2AuthorizationCount++;
                    if(Link2AuthorizationCount >= 3)
			        {
				        Link2LoginAuthorizationState = LOGIN_SEND_LOGIN_CMD;
			        }
			        else
			        {
				        Link2LoginAuthorizationState = LOGIN_DELAY;
			        }
                    errorcount = 0;
                }
                else
                {
                    errorcount++;
                    if(errorcount >= 200)
                    {
                         Link2LoginAuthorizationState = LOGIN_CLOSE_TASK;
                    }
                }
			    break;
		    }
	    case LOGIN_CLOSE_GPRS://断开网络
		    {
                Net_Second_Close();
                Link2ManualCloseFlag = 1;
                ClearTerminalAuthorizationFlag(CHANNEL_DATA_2);//鉴权标志清0
                ClearTerminalAuthorizationCode(CHANNEL_DATA_2);
                Link2LoginAuthorizationDelayCount = 0;
                Link2LoginAuthorizationDelayTime = 10*60;//注册鉴权延时时间，真服务器延时10分钟
                Link2LoginAuthorizationLastState = LOGIN_RECONNECT_GPRS;
                Link2LoginAuthorizationState = LOGIN_DELAY;	
    			break;
			    
		    }
	    case LOGIN_DELAY://延时
		    {
			    Link2LoginAuthorizationDelayCount++;
			    if(Link2LoginAuthorizationDelayCount >= Link2LoginAuthorizationDelayTime)
			    {
				    Link2LoginAuthorizationDelayCount = 0;
				    Link2LoginAuthorizationState = Link2LoginAuthorizationLastState;
			    }
			    break;
		    }
	    case LOGIN_RECONNECT_GPRS://重连网络
		    {
                Link2LoginCount = 0;
                errorcount = 0;
                Link2AuthorizationCount = 0;
                Link2LoginAuthorizationLastState = 0;
                Link2LoginAuthorizationState = 0;
                Link2LoginAuthorizationDelayCount = 0; 
                Net_Second_Open();
                Link2ManualCloseFlag = 0;
    			Link2LoginAuthorizationState = LOGIN_CLOSE_TASK;
    			break;
		    }
	    case LOGIN_CLOSE_TASK://关闭任务
		    {
                if(1 == Link2ManualCloseFlag)
                {
                    Net_Second_Open();//退出任务之前若已手动关闭连接，需要重新打开
                    Link2ManualCloseFlag = 0;
                }
                Link2LoginCount = 0;
                errorcount = 0;
                Link2AuthorizationCount = 0;
                Link2LoginAuthorizationLastState = 0;
                Link2LoginAuthorizationState = 0;
                Link2LoginAuthorizationDelayCount = 0; 
                return DISABLE;
		    }
	    default:
            {
                Link2LoginAuthorizationState = LOGIN_CLOSE_TASK;
            }
		break;
	}
	return ENABLE;
	
}
#if	TACHOGRAPHS_19056_TEST
static u8 simulationSpeed = 100;
static const  u8	simulationSpeedRate = 20;
void Tachographs_Test_Speed_Reduce(void)
{	
	static u8 flag = 0;
	if(!flag)
	{
		simulationSpeed -= simulationSpeedRate;
		if(0 == simulationSpeed)flag = !flag;
	}
	else
	{
		simulationSpeed += simulationSpeedRate;
		if(simulationSpeed >= 100)flag = !flag;
	}
}
u8 Tachographs_Test_Get_Speed(void)
{	
	return simulationSpeed;
}
#endif


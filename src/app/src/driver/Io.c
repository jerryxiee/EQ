/******************************************************************** 
//版权说明	:Shenzhen E-EYE Co. Ltd., Copyright 2009- All Rights Reserved.
//文件名称	:Io.c		
//功能		:IO及AD采集
//版本号	:
//开发人	:dxl
//开发时间	:2012.3
//修改者	:
//修改时间	:
//修改简要说明	:
//备注		:
***********************************************************************/
//***************包含文件*****************
#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "include.h"
//****************宏定义****************
//***************变量定义***************
// 0.0：ACC 关；1： ACC 开;检测时间1秒
// 1.0：未定位；1：定位
// 2.0：北纬；1：南纬
// 3.0：东经；1：西经
// 4.0：运营状态；1：停运状态
// 5.0：经纬度未经保密插件加密；1：经纬度已经保密插件加密
// 6.保留
// 7.保留
// 9.[9:8]表示：00空车,01半载;10保留;11满载
// 10.0：车辆油路正常；1：车辆油路断开
// 11.0：车辆电路正常；1：车辆电路断开
// 12.0：车门解锁；1：车门加锁
// 13.0：门1 关；1：门1 开（前门）;检测时间1秒
// 14.0：门2 关；1：门2 开（中门）;检测时间1秒
// 15.0：门3 关；1：门3 开（后门）;检测时间1秒
// 16.0：0：门4 关；1：门4 开（驾驶席门）;检测时间1秒
// 17.0：门5 关；1：门5 开（自定义）;检测时间1秒
// 18.0：未使用GPS 卫星进行定位；1：使用GPS 卫星进行定位
// 19.0：未使用北斗卫星进行定位；1：使用北斗卫星进行定位
// 20.0：未使用GLONASS 卫星进行定位；1：使用GLONASS 卫星进行定位
// 21.0：未使用Galileo 卫星进行定位；1：使用Galileo 卫星进行定位
//22-31 保留
const u8 DetectStatusHighDelay[STATUS_BIT_MAX] = {
    20,0,0,0,0,0,0,0,       
    0,0,0,0,0,20,20,20,	        
    20,20,0,0,0,0,0,0,     
    0,0,0,0,0,0,0,0,      
};
const u8 DetectStatusLowDelay[STATUS_BIT_MAX] = {
    20,0,0,0,0,0,0,0,       
    0,0,0,0,0,20,20,20,	        
    20,20,0,0,0,0,0,0,     
    0,0,0,0,0,0,0,0,      
};
// 0	近光灯;高电平检测时间0.15秒，低电平检测1秒
// 1	远光灯;高电平检测时间0.15秒，低电平检测1秒
// 2	右转向灯;检测时间0.15秒，低电平检测1秒
// 3	左转向灯;检测时间0.15秒，低电平检测1秒
// 4	制动（刹车）;检测时间0.15秒
// 5	倒档;检测时间1秒
// 6	雾灯;检测时间0.15秒，低电平检测1秒
// 7	示廊灯;检测时间0.15秒，低电平检测1秒
// 8	喇叭;检测时间0.15秒
// 9	空调;检测时间1秒
// 10	空挡
// 11	缓速器工作
// 12	ABS工作
// 13	加热器工作				
// 14	离合器工作
// 15	15-31保留
const u8 DetectExtCarStatusHighDelay[CAR_STATUS_BIT_MAX] = {
    3,3,3,3,3,20,3,3,		
    3,20,0,0,0,0,0,0,		
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
};
const u8 DetectExtCarStatusLowDelay[CAR_STATUS_BIT_MAX] = {
    20,20,20,20,20,20,20,20,		
    20,20,0,0,0,0,0,0,		
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
};
// 0	1：紧急报警,收到应答后清除;检测时间2秒
// 1	1：超速,标志维持至报警条件解除
// 2	1：疲劳驾驶,标志维持至报警条件解除
// 3	1：危险预警,收到应答后清零
// 4	1：GNSS模块故障,标志维持至报警条件解除
// 5	1：GNSS天线断开,标志维持至报警条件解除;检测时间1秒
// 6	1：GNSS天线短路,标志维持至报警条件解除;检测时间1秒
// 7	1：主电欠压,标志维持至报警条件解除;检测时间15秒
// 8	1：主电掉电,标志维持至报警条件解除;检测时间15秒
// 9	1：LCD显示屏故障,标志维持至报警条件解除
// 10	1：TTS故障,标志维持至报警条件解除
// 11	1：摄像头故障,标志维持至报警条件解除
// 12    1：道路运输证IC 卡模块故障,标志维持至报警条件解除
// 13    1：超速预警,标志维持至报警条件解除
// 14    1：疲劳驾驶预警,标志维持至报警条件解除
// 15    保留
// 16    保留
// 17	保留
// 18	1：当天累计驾驶超时,标志维持至报警条件解除
// 19	1：超时停车,标志维持至报警条件解除
// 20	1：进出区域,收到应答后清零
// 21	1：进出路线,收到应答后清除
// 22	1：路段行驶时间不足/过长,收到应答后清除
// 23	1：路线偏离报警,标志维持至报警条件解除
// 24	1：车辆VSS 故障,标志维持至报警条件解除
// 25	1：车辆油量异常,标志维持至报警条件解除
// 26	1：车辆被盗(通过车辆防盗器),标志维持至报警条件解除
// 27	1：车辆非法点火,收到应答后清零
// 28	1：车辆非法位移,收到应答后清零
// 29    1：碰撞预警,标志维持至报警条件解除
// 30    1：侧翻预警,标志维持至报警条件解除
// 31    1：非法开门报警（终端未设置区域时，不判断非法开门）,收到应答后清零
const u8 DetectAlarmHighDelay[ALARM_BIT_MAX] = {
    40,0,0,0,0,20,20,0,		    
    0,0,0,0,0,0,0,0,          
    0,0,0,0,0,0,0,0,	    
    0,0,0,0,0,0,0,0,	
};
const u8 DetectAlarmLowDelay[ALARM_BIT_MAX] = {
    40,0,0,0,0,20,20,0,		    
    0,0,0,0,0,0,0,0,          
    0,0,0,0,0,0,0,0,	    
    0,0,0,0,0,0,0,0,	
};

u32	Status = 0;//32位状态位,对应基本位置信息汇报里的状态位定义
u32	Alarm = 0;//32位报警位,对应基本位置信息汇报里的报警位定义
//********************报警及状态位变量************
static u32	ExtCarStatus = 0;//扩展的车辆状态位,对应附加位置信息汇报里的扩展车辆信号状态定义
static u32	IoStatus = 0;//IO状态位,对应附加位置信息汇报里的扩展IO状态定义
static u32	SuoMeiStatus = 0;//索美状态位，附加ID为0xEF,
static u32	SelfDefine2 = 0;//自定义表2，附加ID0xE1
static u32	SelfDefine2MaskWord = 0;//自定义表2屏蔽字
static u32      SelfDefine2HighValid = 0;//自定义2高有效标志，对应位为1表示高有效，0为低有效
static u32      KeyAlarm = 0;//关键报警，当相应位设置为关键报警时，该报警不受报警屏蔽字控制，产生该报警后需发送SMS及拍照上传
static u8	MicControl = 0;//麦克风控制字
//********************报警及状态位对应的外部IO口高低有效定义************
static u32	AlarmHighValid = 0xffffffff;  //高电压有效状态标志，每个位与Alarm对应，1为高有效，0为低有效
static u32	StatusHighValid = 0x0;        //高电压有效状态标志，每个位与IoStatus对应，1为高有效，0为低有效
static u32	ExtCarStatusHighValid = 0x0;  //高电压有效状态标志，每个位与ExtCarStatus对应，1为高有效，0为低有效

//********************共口控制位************
static u8	SpeakerControl = 0;//杨声器控制字,有2个地方使用了:通话,tts语音播报
static u8	OverSpeedControl = 0;//超速控制字，有3个地方使用了:常规超速,区域超速,线路超速
static u8	RecordControl = 0;//录音控制字,有3个地方使用了:定时或循环录音,紧急报警录音,空车转重车录音

//********************报警屏蔽字***************
u32	AlarmMaskWord = 0;//报警屏蔽字,设置对应位后该报警位将不再报警,相当于该报警功能没有了
u8	MicBaseOnOffFlag = 0;//麦克偏置电压ON/OFF标志位1,1为ON，0为OFF
//********************报警及状态变化拍照控制*************
u32	AlarmPhotoSwitch = 0;//报警拍照开关,1为使能,0不使能
u32	AlarmPhotoStoreFlag = 0;//报警拍摄存储标志,1为存储,0为不存储
u32	StatusPhotoSwitch = 0;//状态变化拍照开关,1为使能,0不使能
u32	StatusPhotoStoreFlag = 0;//状态变化拍照存储标志,1为存储,0为不存储
//*********************蜂鸣器控制*****************
u32	BeepOnTime = 0;
u32	BeepOffTime = 0;
u8	BeepOnOffFlag = 0;//蜂鸣器开关标志

//*********************通信模块，定位模块指示灯控制************
u32	GprsLedOnTime = 0;
u32	GprsLedOffTime = 0;
u32	GpsLedOnTime = 0;
u32	GpsLedOffTime = 0;
u8	GprsLedOnOffFlag = 0;//蜂鸣器开关标志
u8	GpsLedOnOffFlag = 0;//蜂鸣器开关标志
u8	GprsLedState = 0;//GPRS LED灯状态,0：不在线熄灭,1：在线闪烁,2：故障常亮
u8	GpsLedState = 0;//GPS LED灯状态,0：不导航熄灭,1：导航闪烁,2：故障常亮

static u8	LastOneKeyStatus = 0;//上一次一键通话状态,1为有动作，按了键
static u8	DoorSelectFlag = 0;//车门电平选择标志,0为低有效,1为高有效
//***************12/24V电源自适应**************
static u16	PowerLowMaxValue = 0;//代表0V
static u16	PowerShutValue = 50;//代表5.0V
u8	CarVoltageType = 0;//车辆电平类型:0为12V,1为24V
//****************外部变量*****************

extern u8	GpsOnOffFlag;//GPS开关标志,0为GPS关,1为GPS开
extern CAMERA_STRUCT Camera[];
extern u32     PhotoID;//图像多媒体ID

//****************函数定义*****************
//**********************第一部分：初始化****************
/*********************************************************************
//函数名称	:Io_Init(void)
//功能		:IO初始化
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void Io_Init(void)
{
	    GpioInInit(DLIGHTI);
        GpioInInit(LLIGHTI);
        GpioInInit(BREAK);
        GpioInInit(ACC);
        GpioInInit(DOORI_N);
        GpioInInit(DOORI_P);
        GpioInInit(DOORI2);
        GpioOutInit(LAMP);
        GpioOutInit(LOCK_OUT);
        GpioOutInit(UNLOCK_OUT);
        GpioInInit(XLIGHTI);
        GpioInInit(RLIGHTI);
        GpioInInit(BKHI);
        GpioInInit(SWITCHI);  
        GpioInInit(HORNI); 
        GpioInInit(VIRBRATIONI); 
        GpioOutInit(COIL); 
        GpioOutInit(HORN_OUT); 
        
        GpioInInit(DETCT1I); 
        GpioInInit(DETCT2I); 
        
        GpioOutInit(V_COL);
	
        GpioOutInit(BUZZER);
	    GpioInInit(MIC_EN);//dxl,2014.9.30原来是配置为输出，后来由于MIC_IN同时也用于区分降成本版本，因此需配置为输入，
                            //在检测完降成本版本后函数会自动配置为输出。
	    GpioOutInit(SD_POWER);//dxl,2013.5.16
        
        GpioOutInit(I2S_AD_DA);//dxl,2013.5.23
        GpioOutInit(I2S_PWDAN);//dxl,2013.5.23
        GpioOutInit(I2S_PWADN);//dxl,2013.5.23
        
        GpioOutInit(PWR_TYPE);//dxl,2013.5.26
	
	    GpioInInit(SD_DETECT);//dxl,2013.8.16
        
	    Io_InitStatusHighValid();        //初始化状态高有效电平
	    Io_InitExtCarStatusHighValid();  //初始化扩展车辆状态高有效电平
	    Io_InitAlarmHighValid();         //初始化报警高有效电平
        Io_InitDefine2StatusHighValid();//初始化自定义状态字2高有效电平
	
}
/*********************************************************************
//函数名称	:Io_InitStatusHighValid(void)
//功能		:初始化高电平有效标志
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void Io_InitStatusHighValid(void)
{
	StatusHighValid = 0;
	Io_WriteStatusHighValidBit(STATUS_BIT_ACC, SET);//ACC高有效
	Io_WriteStatusHighValidBit(STATUS_BIT_DOOR1, SET);//前门高有效,门1
	Io_WriteStatusHighValidBit(STATUS_BIT_DOOR2, SET);//中门高有效,门2
	//Io_WriteStatusHighValidBit(DEF_STATUS_BIT_LITTLE_LIGHT, SET);//小灯高有效
	//Io_WriteStatusHighValidBit(DEF_STATUS_BIT_NEAR_LIGHT, SET);//近光灯高有效
	//Io_WriteStatusHighValidBit(DEF_STATUS_BIT_GAS_PRESS, SET);//机油压力高有效
	//Io_WriteStatusHighValidBit(DEF_STATUS_BIT_BRAKE, SET);//刹车高有效
	//Io_WriteStatusHighValidBit(DEF_STATUS_BIT_HORN, SET);//车喇叭高有效
	//Io_WriteStatusHighValidBit(DEF_STATUS_BIT_BIG_LIGHT, SET);//大灯高有效
	//Io_WriteStatusHighValidBit(DEF_STATUS_BIT_LEFT_LIGHT, SET);//左灯高有效
	//Io_WriteStatusHighValidBit(DEF_STATUS_BIT_RIGHT_LIGHT, SET);//右灯高有效
	//Io_WriteStatusHighValidBit(DEF_STATUS_BIT_FRONT_DOOR, SET);//前门高有效
	//Io_WriteStatusHighValidBit(DEF_STATUS_BIT_MIDDLE_DOOR, SET);//中门高有效
	//Io_WriteStatusHighValidBit(DEF_STATUS_BIT_AIR_CONDITION, SET);//空调高有效
	//Io_WriteStatusHighValidBit(DEF_STATUS_BIT_ENGINE, SET);//发动机高有效
	//Io_WriteStatusHighValidBit(DEF_STATUS_BIT_EXTG1, SET);//外部输入1
	//Io_WriteStatusHighValidBit(DEF_STATUS_BIT_EXTG2, SET);//外部输入2
}
/*********************************************************************
//函数名称	:Io_InitExtCarStatusHighValid(void)
//功能		:初始化高电平有效标志
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void Io_InitExtCarStatusHighValid(void)
{
	Io_WriteExtCarStatusHighValidBit(CAR_STATUS_BIT_SIDE_LIGHT, SET);    //小灯高有效,示廊灯
	Io_WriteExtCarStatusHighValidBit(CAR_STATUS_BIT_NEAR_LIGHT, SET);    //近光灯高有效
	Io_WriteExtCarStatusHighValidBit(CAR_STATUS_BIT_BRAKE, SET);         //刹车高有效
	Io_WriteExtCarStatusHighValidBit(CAR_STATUS_BIT_HORN, SET);          //车喇叭高有效
	Io_WriteExtCarStatusHighValidBit(CAR_STATUS_BIT_FAR_LIGHT, SET);     //大灯高有效,远光灯
	Io_WriteExtCarStatusHighValidBit(CAR_STATUS_BIT_LEFT_LIGHT, SET);    //左灯高有效
	Io_WriteExtCarStatusHighValidBit(CAR_STATUS_BIT_RIGHT_LIGHT, SET);   //右灯高有效
	Io_WriteExtCarStatusHighValidBit(CAR_STATUS_BIT_AIR_CONDITION, SET); //空调高有效
}
/*********************************************************************
//函数名称	:Io_InitDefine2StatusHighValid(void)
//功能		:初始化高电平有效标志
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void Io_InitDefine2StatusHighValid(void)
{

}
/*********************************************************************
//函数名称	:Io_InitAlarmHighValid(void)
//功能		:初始化高电平有效标志
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void Io_InitAlarmHighValid(void)
{
	AlarmHighValid = 0xffffffff;
	Io_WriteAlarmHighValidBit(ALARM_BIT_EMERGENCY, RESET);//紧急报警低有效
	//Io_WriteAlarmHighValidBit(ALARM_BIT_ANT_SHUT, RESET);//天线开路低有效
	Io_WriteAlarmHighValidBit(ALARM_BIT_ANT_SHORT, RESET);//天线短路低有效
}
/*********************************************************************
//函数名称	:Io_UpdataPram(void)
//功能		:更新变量
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void Io_UpdataPram(void)
{       
	u8	Buffer[5];
	u8	BufferLen;
        
        //读关键报警字
        BufferLen = EepromPram_ReadPram(E2_KEY_FLAG_ID,Buffer);
	if(BufferLen == E2_KEY_FLAG_LEN)
	{
		KeyAlarm = 0;
		KeyAlarm |= Buffer[0] << 24;
		KeyAlarm |= Buffer[1] << 16;
		KeyAlarm |= Buffer[2] << 8;
		KeyAlarm |= Buffer[3];
	}
	//读报警屏蔽字
	BufferLen = EepromPram_ReadPram(E2_ALARM_MASK_ID,Buffer);
	if(BufferLen == E2_ALARM_MASK_LEN)
	{
		AlarmMaskWord = 0;
		AlarmMaskWord |= Buffer[0] << 24;
		AlarmMaskWord |= Buffer[1] << 16;
		AlarmMaskWord |= Buffer[2] << 8;
		AlarmMaskWord |= Buffer[3];
	}
	//读紧急报警拍照开关
	BufferLen = EepromPram_ReadPram(E2_ALARM_PHOTO_SWITCH_ID,Buffer);
	if(BufferLen == E2_ALARM_PHOTO_SWITCH_LEN)
	{
		AlarmPhotoSwitch = 0;
		AlarmPhotoSwitch |= Buffer[0] << 24;
		AlarmPhotoSwitch |= Buffer[1] << 16;
		AlarmPhotoSwitch |= Buffer[2] << 8;
		AlarmPhotoSwitch |= Buffer[3];
	}
	//读报警拍照存储标志
	BufferLen = EepromPram_ReadPram(E2_ALARM_PHOTO_STORE_FLAG_ID,Buffer);
	if(BufferLen == E2_ALARM_PHOTO_STORE_FLAG_LEN)
	{
		AlarmPhotoStoreFlag = 0;
		AlarmPhotoStoreFlag |= Buffer[0] << 24;
		AlarmPhotoStoreFlag |= Buffer[1] << 16;
		AlarmPhotoStoreFlag |= Buffer[2] << 8;
		AlarmPhotoStoreFlag |= Buffer[3];
	}
	//状态变化拍照开关
	BufferLen = EepromPram_ReadPram(E2_STATUS_PHOTO_SWITCH_ID,Buffer);
	if(BufferLen == E2_STATUS_PHOTO_SWITCH_ID_LEN)
	{
		StatusPhotoSwitch = 0;
		StatusPhotoSwitch |= Buffer[0] << 24;
		StatusPhotoSwitch |= Buffer[1] << 16;
		StatusPhotoSwitch |= Buffer[2] << 8;
		StatusPhotoSwitch |= Buffer[3];
	}
	//读报警拍照存储标志
	BufferLen = EepromPram_ReadPram(E2_STATUS_PHOTO_STORE_FLAG_ID,Buffer);
	if(BufferLen == E2_STATUS_PHOTO_STORE_FLAG_ID_LEN)
	{
		StatusPhotoStoreFlag = 0;
		StatusPhotoStoreFlag |= Buffer[0] << 24;
		StatusPhotoStoreFlag |= Buffer[1] << 16;
		StatusPhotoStoreFlag |= Buffer[2] << 8;
		StatusPhotoStoreFlag |= Buffer[3];
	}
	//读上一次紧急报警状态
	BufferLen = FRAM_BufferRead(Buffer, FRAM_EMERGENCY_FLAG_LEN, FRAM_EMERGENCY_FLAG_ADDR);
	if(1 == BufferLen)
	{
		if(1 == Buffer[0])
		{
			Io_WriteAlarmBit(ALARM_BIT_EMERGENCY, SET);
			
		}
		else
		{
			Io_WriteAlarmBit(ALARM_BIT_EMERGENCY, RESET);
		}
	}
	//读车门电平配置
	BufferLen = EepromPram_ReadPram(E2_DOOR_SET_ID,Buffer);
	if(E2_DOOR_SET_LEN == BufferLen)
	{
		DoorSelectFlag = Buffer[0];
	}
        //DoorSelectFlag = 1;//行标北斗检测时添加,车门1高电平有效
	//读录音控制标志
	if(FRAM_RECORD_CONTROL_LEN != FRAM_BufferRead(&RecordControl, FRAM_RECORD_CONTROL_LEN, FRAM_RECORD_CONTROL_ADDR))
	{
		RecordControl = 0;//异常时默认为0
	}
	//读取定位模式
	BufferLen = EepromPram_ReadPram(E2_GPS_SET_MODE_ID,Buffer);
	if(E2_GPS_SET_MODE_LEN == BufferLen)//bit0:GPS定位;bit1:北斗定位;bit2:GLONASS定位;bit3:Galileo定位
	{
		if(0x01 == (0x01&Buffer[0]))
		{
			Io_WriteStatusBit(STATUS_BIT_GPS, SET);
		}
		else
		{
			Io_WriteStatusBit(STATUS_BIT_GPS, RESET);
		}
		
		if(0x02 == (0x02&Buffer[0]))
		{
			Io_WriteStatusBit(STATUS_BIT_COMPASS, SET);
		}
		else
		{
			Io_WriteStatusBit(STATUS_BIT_COMPASS, RESET);
		}
		
		if(0x04 == (0x04&Buffer[0]))
		{
			Io_WriteStatusBit( STATUS_BIT_GLONASS, SET);
		}
		else
		{
			Io_WriteStatusBit( STATUS_BIT_GLONASS, RESET);
		}
		
		if(0x08 == (0x08&Buffer[0]))
		{
			Io_WriteStatusBit(STATUS_BIT_GALILEO, SET);
		}
		else
		{
			Io_WriteStatusBit(STATUS_BIT_GALILEO, RESET);
		}
		
	}
}
//**********************第二部分：外部调用****************
/*********************************************************************
//函数名称	:Io_TimeTask
//功能		:IO及报警位检测任务
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		: 50ms调度一次
*********************************************************************/
FunctionalState Io_TimeTask(void)
{
	u8	OneKeyStatus;
        u32	CurrentTime = 0;
	u8	Acc;
	static u8 BeepCount = 0;

	Io_DetectExtCarStatus(BREAK,CAR_STATUS_BIT_BRAKE);//检测刹车
        Io_DetectStatus(ACC, STATUS_BIT_ACC); //检测ACC
        
        Acc = Io_ReadStatusBit(STATUS_BIT_ACC);

  
	Io_DetectExtCarStatus(BKHI,CAR_STATUS_BIT_AIR_CONDITION);         //检测空调
        if((1 == GpsOnOffFlag)&&(1 == Acc))//GPS是打开的
	{
		Io_DetectAlarm(GPS_OC, ALARM_BIT_ANT_SHUT);//检测GPS天线开路
		Io_DetectAlarm(GPS_SC, ALARM_BIT_ANT_SHORT);//检测GPS天线短路
			
	}
	else//ACC OFF时不检测天线，天线状态将维持之前的
	{
		//Io_WriteAlarmBit(ALARM_BIT_ANT_SHUT, RESET);
		//Io_WriteAlarmBit(ALARM_BIT_ANT_SHORT, RESET);
	}
	Io_DetectAlarm(SWITCHI, ALARM_BIT_EMERGENCY);//检测紧急报警
	
        Io_DetectExtCarStatus(DLIGHTI,CAR_STATUS_BIT_FAR_LIGHT);    //检测远光灯，大灯，连续检测3次有效
   
        Io_DetectExtCarStatus(XLIGHTI,CAR_STATUS_BIT_NEAR_LIGHT);//,原来近光灯检测是在16Pin上，现在改为32PIN的小灯处

        Io_DetectExtCarStatus(DETCT1I,CAR_STATUS_BIT_SIDE_LIGHT); //,示廊灯改为由16PIN上的近光灯检测,注意与原来的小灯检测需同步修改
	Io_DetectExtCarStatus(LLIGHTI,CAR_STATUS_BIT_LEFT_LIGHT);    //检测左转灯
                
	Io_DetectExtCarStatus(RLIGHTI,CAR_STATUS_BIT_RIGHT_LIGHT);   //检测右转灯
	Io_DetectExtCarStatus(HORNI,CAR_STATUS_BIT_HORN);            //检测车喇叭,修改长沙渣土车演示版本时屏蔽
            
        Io_DetectStatus(DOORI2, STATUS_BIT_DOOR2);                   //检测中车门（车门2）,修改长沙渣土车演示版本时屏蔽
    
	Io_DetectFrontDoor();                                        //检测前门,前门需要单独检测，不能调用通用函数
	OneKeyStatus = Io_DetectPhoneKey();                          //检测一键通话
	if((0 == OneKeyStatus)&&(1 == LastOneKeyStatus))             //按下键后已松开
	{
		SetEvTask(EV_CALLING);     //触发一键通话	
	}
	LastOneKeyStatus = OneKeyStatus;
	Io_DetectMainPower();//检测主电是否正常
	//Io_DetectBackupPower();//检测备电
        
        //广州二汽要求超速报警后蜂鸣器要一直响
        if(1 == Io_ReadAlarmBit(ALARM_BIT_OVER_SPEED))
        {
                BeepCount++;
                if(BeepCount >= (4*SYSTICK_0p1SECOND))
                {
                       BeepCount = 0;
                       BEEP_ON();
                }
        }
        
        //***********蜂鸣器*************
	CurrentTime = Timer_Val();        
	if(1 == BeepOnOffFlag)                                         //BEEP_ON()会置位标志 BeepOnOffFlag
	{
		if((CurrentTime - BeepOnTime) >= (1*SYSTICK_0p1SECOND))
		{
			BEEP_OFF();
		}
	}
     
	return ENABLE;
}
/*********************************************************************
//函数名称	:Io_ReadStatus
//功能		:读标准状态字
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
u32 Io_ReadStatus(void)
{
	return Status;
}
/*********************************************************************
//函数名称	:Io_ReadStatusBit(u8 index)
//功能		:读标准状态位
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
u8 Io_ReadStatusBit(STATUS_BIT bit)
{
	u8	state = 0;

	if(bit < 32)
	{
		state = (Status & (1<< bit))>> bit;
	}
	return state;
}
/*********************************************************************
//函数名称	:Io_WriteStatusBit
//功能		:写标准状态位
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:注意请不要在中断函数内调用此函数
*********************************************************************/
void Io_WriteStatusBit(STATUS_BIT bit, FlagStatus state)
{
	if(bit < 32)
	{
		if(SET==state)
		{
			Status |= 1<<bit;
		}
		else if(RESET==state)
		{
			Status &= ~(1<<bit);
		}
	
	}
}
/*********************************************************************
//函数名称	:Io_ReadExtCarStatus
//功能		:读扩展车辆状态字
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
u32 Io_ReadExtCarStatus(void)
{
	return ExtCarStatus;
}
/*********************************************************************
//函数名称	:Io_ReadExtCarStatusBit(u8 index)
//功能		:读扩展车辆状态位
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
u8 Io_ReadExtCarStatusBit(CAR_STATUS_BIT bit)
{
	u8	state = 0;

	if(bit < 32)
	{
		state = (ExtCarStatus & (1<< bit))>> bit;
	}
	return state;
}
/*********************************************************************
//函数名称	:Io_WriteExtCarStatusBit
//功能		:写扩展车辆状态位
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:注意请不要在中断函数内调用此函数
*********************************************************************/
void Io_WriteExtCarStatusBit(CAR_STATUS_BIT bit, FlagStatus state)
{
	if(bit < 32)
	{
		if(SET==state)
		{
			ExtCarStatus |= 1<<bit;
		}
		else if(RESET==state)
		{
			ExtCarStatus &= ~(1<<bit);
		}
	
	}
}
/*********************************************************************
//函数名称	:Io_ReadIoStatus
//功能		:读IO状态字
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
u16 Io_ReadIoStatus(void)
{
	return IoStatus;
}
/*********************************************************************
//函数名称	:Io_ReadIoStatusBit(u8 index)
//功能		:读IO状态位
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
u8 Io_ReadIoStatusBit(IO_STATUS_BIT bit)
{
	u8	state = 0;

	if(bit < 16)
	{
		state = (IoStatus & (1<< bit))>> bit;
	}
	return state;
}
/*********************************************************************
//函数名称	:Io_WriteIoStatusBit
//功能		:写IO状态位
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:注意请不要在中断函数内调用此函数
*********************************************************************/
void Io_WriteIoStatusBit(IO_STATUS_BIT bit, FlagStatus state)
{
	if(bit < 16)
	{
		if(SET==state)
		{
			IoStatus |= 1<<bit;
		}
		else if(RESET==state)
		{
			IoStatus &= ~(1<<bit);
		}
	
	}
}
/*********************************************************************
//函数名称	:Io_ReadSuoMeiStatus
//功能		:读SuoMei状态字
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
u32 Io_ReadSuoMeiStatus(void)
{
	return SuoMeiStatus;
}
/*********************************************************************
//函数名称	:Io_ReadSuoMeiStatusBit(u8 index)
//功能		:读SuoMei状态位
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
u8 Io_ReadSuoMeiStatusBit(SUOMEI_STATUS_BIT bit)
{
	u8	state = 0;

	if(bit < 32)
	{
		state = (SuoMeiStatus & (1<< bit))>> bit;
	}
	return state;
}
/*********************************************************************
//函数名称	:Io_WriteSuoMeiStatusBit
//功能		:写SuoMei状态位
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:注意请不要在中断函数内调用此函数
*********************************************************************/
void Io_WriteSuoMeiStatusBit(SUOMEI_STATUS_BIT bit, FlagStatus state)
{
	if(bit < 32)
	{
		if(SET==state)
		{
			SuoMeiStatus |= 1<<bit;
		}
		else if(RESET==state)
		{
			SuoMeiStatus &= ~(1<<bit);
		}
	
	}
}

/*********************************************************************
//函数名称	:Io_ReadAlarmMaskBit(u8 index)
//功能		:读报警屏蔽位
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:关键报警位对应为1，直接返回0；关键报警位对应位为0，返回屏蔽报警位
*********************************************************************/
u8 Io_ReadAlarmMaskBit(ALARM_BIT bit)
{
	u8	tmp = 0;

	if(bit < 32)
	{
                if((KeyAlarm& (1<< bit))==(1 << bit))
                {
                      tmp = 0;
                }
                else
                {
		      tmp = (AlarmMaskWord& (1<< bit))>> bit;
                }
	}
	return tmp;
}
/*********************************************************************
//函数名称	:Io_ReadAlarm
//功能		:读报警字
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
u32 Io_ReadAlarm(void)
{
#ifdef HUOYUN_DEBUG_OPEN
	return (Alarm&((~AlarmMaskWord)|KeyAlarm));
#else
        return (Alarm&(~AlarmMaskWord));
#endif
}
/*********************************************************************
//函数名称	:Io_ReadKeyAlarm
//功能		:读关键报警字
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
u32 Io_ReadKeyAlarm(void)
{
	return KeyAlarm;
}
/*********************************************************************
//函数名称	:Io_ReadSelfDefine2
//功能		:读自定义状态表2
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
u32 Io_ReadSelfDefine2(void)
{
	return (SelfDefine2&(~SelfDefine2MaskWord));
}
/*********************************************************************
//函数名称	:Io_ReadSelfDefine2Bit(u8 index)
//功能		:读自定义表2的位
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
u8 Io_ReadSelfDefine2Bit(DEFINE_BIT bit)
{
	u8	tmp = 0;

	if(bit < 32)
	{
		tmp = ((SelfDefine2&(~SelfDefine2MaskWord)) & (1<< bit))>> bit;
	}
	return tmp;
}
/*********************************************************************
//函数名称	:Io_ReadAlarmBit(u8 index)
//功能		:读报警位
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
u8 Io_ReadAlarmBit(ALARM_BIT bit)
{
	u8	tmp = 0;

	if(bit < 32)
	{
#ifdef HUOYUN_DEBUG_OPEN
		tmp = ((Alarm&((~AlarmMaskWord)|KeyAlarm)) & (1<< bit))>> bit;
#else
                tmp = ((Alarm&(~AlarmMaskWord)) & (1<< bit))>> bit;
#endif
	}
	return tmp;
}
/*********************************************************************
//函数名称	:Io_ReadKeyAlarmBit(ALARM_BIT bit)
//功能		:读关键报警的某一个位
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
u8 Io_ReadKeyAlarmBit(ALARM_BIT bit)
{
	u8	tmp = 0;

	if(bit < 32)
	{
		tmp = (KeyAlarm & (1<< bit))>> bit;
	}
	return tmp;
}
/*********************************************************************
//函数名称	:Io_WriteAlarmBit
//功能		:写某一报警位
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:注意请不要在中断函数内调用此函数
*********************************************************************/
void Io_WriteAlarmBit(ALARM_BIT bit, FlagStatus state)
{
	if(bit < 32)
	{
		
		if(SET==state)
		{
			Alarm |= 1<<bit;
		}
		else if(RESET==state)
		{
			Alarm &= ~(1<<bit);
		}
	}
}
/*********************************************************************
//函数名称	:Io_WriteKeyAlarmBit
//功能		:写关键报警的某一报警位
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:注意请不要在中断函数内调用此函数
*********************************************************************/
void Io_WriteKeyAlarmBit(ALARM_BIT bit, FlagStatus state)
{
	if(bit < 32)
	{
		
		if(SET==state)
		{
			KeyAlarm |= 1<<bit;
		}
		else if(RESET==state)
		{
			KeyAlarm &= ~(1<<bit);
		}
	}
}
/*********************************************************************
//函数名称	:Io_WriteSelfDefine2Bit
//功能		:写自定义表2的位
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:注意请不要在中断函数内调用此函数
*********************************************************************/
void Io_WriteSelfDefine2Bit(DEFINE_BIT bit, FlagStatus state)
{
	if(bit < 32)
	{
		
		if(SET==state)
		{
			SelfDefine2 |= 1<<bit;
		}
		else if(RESET==state)
		{
			SelfDefine2 &= ~(1<<bit);
		}
	}
}
/*********************************************************************
//函数名称	:Io_ReadOverSpeedConrtolBit
//功能		:读超速控制位
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:注意请不要在中断函数内调用此函数
*********************************************************************/
FlagStatus Io_ReadOverSpeedConrtolBit(OVER_SPEED_BIT bit)
{
	
	
	if(bit < OVER_SPEED_BIT_MAX)
	{
		if((1 << bit) == ((1 << bit)&OverSpeedControl))
		{
			return SET;
		}
		else
		{
			return RESET;
		}
	}
	else//错误
	{
		return RESET;
	}
}
/*********************************************************************
//函数名称	:Io_WriteOverSpeedConrtolBit
//功能		:写超速控制位
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:注意请不要在中断函数内调用此函数
*********************************************************************/
void Io_WriteOverSpeedConrtolBit(OVER_SPEED_BIT bit, FlagStatus state)
{
	if(bit < OVER_SPEED_BIT_MAX)
	{
		
		if(SET==state)
		{
			OverSpeedControl |= 1<<bit;
		}
		else if(RESET==state)
		{
			OverSpeedControl &= ~(1<<bit);
		}
	}
	if(0 == OverSpeedControl)
	{
		//清除超速标志
		Io_WriteAlarmBit(ALARM_BIT_OVER_SPEED, RESET);
	}
	else
	{
		//置位超速标志
		Io_WriteAlarmBit(ALARM_BIT_OVER_SPEED, SET);
	}
}
/*********************************************************************
//函数名称	:Io_ReadRecordConrtol
//功能		:读录音控制字节
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:注意请不要在中断函数内调用此函数
*********************************************************************/
u8 Io_ReadRecordConrtol(void)
{
	return RecordControl;
}
/*********************************************************************
//函数名称	:Io_WriteRecordConrtolBit
//功能		:写录音控制位
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:注意请不要在中断函数内调用此函数
*********************************************************************/
void Io_WriteRecordConrtolBit(RECORD_BIT bit, FlagStatus state)
{
	u8	CloseFlag;
	u8	OpenFlag;
	
	//u8	TestFlag1;
	//u8	TestFlag2;
	//u8	TestFlag3;
	
	if(bit < RECORD_BIT_MAX)
	{
		
		if(SET==state)
		{
			RecordControl |= 1<<bit;
			OpenFlag = (1 << bit);
		}
		else if(RESET==state)
		{
			RecordControl &= ~(1<<bit);
			CloseFlag = ~(1 << bit);
		}
		if(0 == RecordControl)
		{
			//关闭录音
			RecordTask_Close();
			//关标志写入铁电
			FRAM_BufferWrite(FRAM_RECORD_CLOSE_ADDR, &CloseFlag, FRAM_RECORD_CLOSE_LEN);
		}
		else
		{
			if(SET == state)
			{
				//开启录音
				RecordTask_Start(8000,60);
				//开标志写入铁电
				FRAM_BufferWrite(FRAM_RECORD_OPEN_ADDR, &OpenFlag, FRAM_RECORD_OPEN_LEN);
			}
		}
		//标志写入铁电
		FRAM_BufferWrite(FRAM_RECORD_CONTROL_ADDR, &RecordControl, FRAM_RECORD_CONTROL_LEN);
		//读写入的值
		//FRAM_BufferRead(&TestFlag1, FRAM_RECORD_CONTROL_LEN, FRAM_RECORD_CONTROL_ADDR);
		//FRAM_BufferRead(&TestFlag2, FRAM_RECORD_CLOSE_LEN, FRAM_RECORD_CLOSE_ADDR);
		//FRAM_BufferRead(&TestFlag3, FRAM_RECORD_OPEN_LEN, FRAM_RECORD_OPEN_ADDR);
	}
	
}
/*********************************************************************
//函数名称	:Io_ReadRecordConrtolBit
//功能		:读录音控制位
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:注意请不要在中断函数内调用此函数
*********************************************************************/
FlagStatus Io_ReadRecordConrtolBit(RECORD_BIT bit)
{
	
	
	if(bit < RECORD_BIT_MAX)
	{
		if((1 << bit) == ((1 << bit)&RecordControl))
		{
			return SET;
		}
		else
		{
			return RESET;
		}
	}
	else//错误
	{
		return RESET;
	}
}
/*********************************************************************
//函数名称	:Io_WriteMicConrtolBit
//功能		:写某一麦克控制位
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:注意请不要在中断函数内调用此函数
*********************************************************************/
void Io_WriteMicConrtolBit(MIC_BIT bit, FlagStatus Status)
{
	if(bit < MIC_BIT_MAX)
	{
		
		if(SET==Status)
		{
			MicControl |= 1<<bit;
		}
		else if(RESET==Status)
		{
			MicControl &= ~(1<<bit);
		}
	}
	if(0 == MicControl)
	{
		MIC_BASE_OFF();//关闭
	}
	else
	{
		MIC_BASE_ON();//打开
	}
}
/*********************************************************************
//函数名称	:Io_WriteSpeakerConrtolBit
//功能		:写某一扬声器控制位
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:注意请不要在中断函数内调用此函数
*********************************************************************/
void Io_WriteSpeakerConrtolBit(SPEAKER_BIT bit, FlagStatus state)
{
	if(bit < SPEAKER_BIT_MAX)
	{
		
		if(SET==state)
		{
			SpeakerControl |= 1<<bit;
		}
		else if(RESET==state)
		{
			SpeakerControl &= ~(1<<bit);
		}
	}
	if(0 == SpeakerControl)
	{
		SPEAKER_OFF();//关闭
	}
	else
	{
		SPEAKER_ON();//打开
	}
	
	
}

//********************第三部分：内部函数*******************
/*********************************************************************
//函数名称	:Io_DetectStatus
//功能		:检测属于状态位的IO输入
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void Io_DetectStatus(GPIO_OUT_IN_NUM IoNum, u8 index)
{
	u8	state;
	static u8	HighCount[32] = {0};
	static u8	LowCount[32] = {0};

	state = GpioInGetState(IoNum);
	if(1 == state)
	{
		HighCount[index]++;
		if(HighCount[index] >=DetectStatusHighDelay[index])
		{
			HighCount[index] = DetectStatusHighDelay[index];
		}
		LowCount[index] = 0;
	}
	else
	{
		HighCount[index] = 0;
		LowCount[index]++;
		if(LowCount[index] >=DetectStatusLowDelay[index])
		{
			LowCount[index] = DetectStatusLowDelay[index];
		}

	}
	if(HighCount[index] >=DetectStatusHighDelay[index])
	{
		state = (StatusHighValid & (1<< index))>> index;
  		if(1==state)
		{
			Status |= 1<<index;
		}
		else
		{
			Status &= ~(1<<index);
		}
	}
	else if(LowCount[index] >=DetectStatusLowDelay[index])
	{
		state = (StatusHighValid & (1<< index))>> index;
  		if(0==state)
		{
			Status |= 1<<index;
		}
		else
		{
			Status &= ~(1<<index);
		}

	}

}
/*********************************************************************
//函数名称	:Io_DetectExtCarStatus
//功能		:检测属于扩展车辆状态字的IO输入
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void Io_DetectExtCarStatus(GPIO_OUT_IN_NUM IoNum,u8 index)
{
	u8	state;
	static u8	HighCount[32] = {0};
	static u8	LowCount[32] = {0};

	state = GpioInGetState(IoNum);
	if(1 == state)
	{
		HighCount[index]++;
		if(HighCount[index] >=DetectExtCarStatusHighDelay[index])
		{
			HighCount[index] = DetectExtCarStatusHighDelay[index];
		}
		LowCount[index] = 0;
	}
	else
	{
		HighCount[index] = 0;
		LowCount[index]++;
		if(LowCount[index] >=DetectExtCarStatusLowDelay[index])
		{
			LowCount[index] = DetectExtCarStatusLowDelay[index];
		}

	}
	if(HighCount[index] >=DetectExtCarStatusHighDelay[index])
	{
		state = (ExtCarStatusHighValid & (1<< index))>> index;
  		if(1==state)
		{
			ExtCarStatus |= 1<<index;
		}
		else
		{
			ExtCarStatus &= ~(1<<index);
		}
	}
	else if(LowCount[index] >=DetectExtCarStatusLowDelay[index])
	{
		state = (ExtCarStatusHighValid & (1<< index))>> index;
  		if(0==state)
		{
			ExtCarStatus |= 1<<index;
		}
		else
		{
			ExtCarStatus &= ~(1<<index);
		}

	}

}

/*********************************************************************
//函数名称	:Io_DetectDefine2Status
//功能		:检测属于自定义状态字的IO输入
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void Io_DetectDefine2Status(GPIO_OUT_IN_NUM IoNum,u8 index)
{
	u8	state;
	static u8	HighCount[32] = {0};
	static u8	LowCount[32] = {0};

	state = GpioInGetState(IoNum);
	if(1 == state)
	{
		HighCount[index]++;
		if(HighCount[index] >=3)
		{
			HighCount[index] = 3;
		}
		LowCount[index] = 0;
	}
	else
	{
		HighCount[index] = 0;
		LowCount[index]++;
		if(LowCount[index] >=3)
		{
			LowCount[index] = 3;
		}

	}
	if(HighCount[index] >=3)
	{
		state = (SelfDefine2HighValid & (1<< index))>> index;
  		if(1==state)
		{
			SelfDefine2 |= 1<<index;
		}
		else
		{
			SelfDefine2 &= ~(1<<index);
		}
	}
	else if(LowCount[index] >=3)
	{
		state = (SelfDefine2HighValid & (1<< index))>> index;
  		if(0==state)
		{
			SelfDefine2 |= 1<<index;
		}
		else
		{
			SelfDefine2 &= ~(1<<index);
		}

	}

}

/*********************************************************************
//函数名称	:Io_DetectAlarm
//功能		:检测报警IO口
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void Io_DetectAlarm(GPIO_OUT_IN_NUM IoNum, u8 index)
{
	u8	state;
	u8	flag;
	static u8	HighCount[32] = {0};
	static u8	LowCount[32] = {0};

	state = GpioInGetState(IoNum);
	if(1 == state)
	{
		HighCount[index]++;
		if(HighCount[index] >=DetectAlarmHighDelay[index])
		{
			HighCount[index] = DetectAlarmHighDelay[index];
		}
		LowCount[index] = 0;
	}
	else
	{
		HighCount[index] = 0;
		LowCount[index]++;
		if(LowCount[index] >=DetectAlarmLowDelay[index])
		{
			LowCount[index] = DetectAlarmLowDelay[index];
		}

	}
	if(HighCount[index] >=DetectAlarmHighDelay[index])
	{
		state = (AlarmHighValid & (1<< index))>> index;
  		if(1==state)
		{
			Alarm |= 1<<index;
		}
		else
		{
			if(ALARM_BIT_EMERGENCY == index)//紧急报警状态由监控中心清除
			{
				
			}
			else
			{
				Alarm &= ~(1<<index);
			}
		}
	}
	else if(LowCount[index] >=DetectAlarmLowDelay[index])
	{
		state = (AlarmHighValid & (1<< index))>> index;
  		if(0==state)
		{
			Alarm |= 1<<index;
			if(ALARM_BIT_EMERGENCY == index)//紧急报警状态由监控中心清除
			{
				flag = 1;
				FRAM_BufferWrite(FRAM_EMERGENCY_FLAG_ADDR, &flag, FRAM_EMERGENCY_FLAG_LEN);
			}
			
		}
		else
		{
			if(ALARM_BIT_EMERGENCY == index)//紧急报警状态由监控中心清除
			{
				
			}
			else
			{
				Alarm &= ~(1<<index);
			}
		}

	}

}
/*********************************************************************
//函数名称	:Io_DetectFrontDoor
//功能		:检测前门
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:120ms调度一次
*********************************************************************/
void Io_DetectFrontDoor(void)
{
	u8	state;
	static u8	HighCountP = 0;
	static u8	LowCountP = 0;
	static u8	HighCountN = 0;
	static u8	LowCountN = 0;
	
	if(1 == DoorSelectFlag)//高电平有效
	{
		//检测DOOR+
		state = GpioInGetState (DOORI_P);
		if(1 == state)
		{
			HighCountP++;
			if(HighCountP >=3)
			{
				HighCountP = 3;
			}
			LowCountP = 0;
		}
		else
		{
			HighCountP = 0;
			LowCountP++;
			if(LowCountP >=3)
			{
				LowCountP = 3;
			}
		}
		if(HighCountP >=3)
		{
			Status |= 1<<STATUS_BIT_DOOR1;
		}
		else if(LowCountP >=3)
		{
			Status &= ~(1<<STATUS_BIT_DOOR1);
		}
	}
	else if(0 == DoorSelectFlag)
	{
		//检测DOOR-
		state = GpioInGetState (DOORI_N);
		if(1 == state)
		{
			HighCountN++;
			if(HighCountN >=3)
			{
				HighCountN = 3;
			}
			LowCountN = 0;
		}
		else
		{
			HighCountN = 0;
			LowCountN++;
			if(LowCountN >=3)
			{
				LowCountN = 3;
			}

		}
		if(HighCountN >=3)
		{
			Status &= ~(1<<STATUS_BIT_DOOR1);
		}
		else if(LowCountN >=3)
		{	
			Status |= 1<<STATUS_BIT_DOOR1;
		}
	
	}
	else
	{
		Status &= ~(1<<STATUS_BIT_DOOR1);//车门选择错误,车门始终为关
	}
	
		
}
/*********************************************************************
//函数名称	:Io_DetectPhoneKey
//功能		:检测一键通话键
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:低电平有效
*********************************************************************/
u8 Io_DetectPhoneKey(void)
{
	u8	state;

	static u8	HighCount = 0;
	static u8	LowCount = 0;
	
	
	state = GpioInGetState (VIRBRATIONI);
	if(1 == state)
	{
		HighCount++;
		if(HighCount >=3)
		{
			HighCount = 3;
		}
		LowCount = 0;
	}
	else
	{
		HighCount = 0;
		LowCount++;
		if(LowCount >=3)
		{
			LowCount = 3;
		}

	}
	if(LowCount >=3)
	{
		return 1;
	}
	else
	{
		return 0;
	}
	
}
/*********************************************************************
//函数名称	:Io_DetectMainPower
//功能		:检测主电电压
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:100ms调度一次
*********************************************************************/
void Io_DetectMainPower(void)
{
	static u8	LowAlarmCount = 0;
	static u8	NoLowAlarmCount = 0;
	static u8	ShutAlarmCount = 0;
	static u8	NoShutAlarmCount = 0;
	static u32	AdSum = 0;//AD转换值累计和
	static u16	AdMin = 0xffff;//AD转换值最小值
	static u16	AdMax = 0;//AD转换值最大值
	static u8	AdCount = 0;//AD转换计数
	u16	Ad;
        u8     AlarmCountMax;
        u8     noAlarmCountMax;
        u8     AdCountMax;

        AlarmCountMax = 15;//功能规范要求是15秒，后面统一改成这个
        noAlarmCountMax = 15;
        //AlarmCountMax = 4;//实际应用时为30秒,10秒,福建电信测试均为5秒
        //noAlarmCountMax = 4;
        //AlarmCountMax = 3;//北斗检测时使用
        //noAlarmCountMax = 1;
        AdCountMax = 10;
        
	AdCount++;
	Ad = Ad_GetValue(ADC_MAIN_POWER); 
	if(AdMin > Ad)
	{
		AdMin = Ad;
	}
	if(AdMax < Ad)
	{
		AdMax = Ad;
	}
	AdSum += Ad;
	if(AdCount >=  AdCountMax)
	{
		//计算平均值
		AdSum -= AdMin;
		AdSum -= AdMax;
		AdSum = AdSum >> 3;
		//转成以0.1V为单位的值
		AdSum = AdSum*33*9/0xfff;
		//置位或清除欠压报警
		if((AdSum < PowerLowMaxValue)&&(AdSum > PowerShutValue))
		{
			LowAlarmCount++;
			if(LowAlarmCount >=AlarmCountMax)//实际应用取值
			{
			        LowAlarmCount = AlarmCountMax;
			}
			NoLowAlarmCount = 0;
		}
		else
		{
			LowAlarmCount = 0;
			NoLowAlarmCount++;
			if(NoLowAlarmCount >=noAlarmCountMax)//实际应用取值
			{
				NoLowAlarmCount = noAlarmCountMax;
			}
		}
		if(LowAlarmCount >=AlarmCountMax)
		{
			Io_WriteAlarmBit(ALARM_BIT_POWER_LOW, SET);//主电欠压置位
			Io_WriteAlarmBit(ALARM_BIT_POWER_SHUT, RESET);//主电掉电清除
		}
		else if(NoLowAlarmCount >=noAlarmCountMax)
		{
			Io_WriteAlarmBit(ALARM_BIT_POWER_LOW, RESET);//主电欠压清除
		}
		//置位或清除断电报警
		if(AdSum < PowerShutValue)
		{
			ShutAlarmCount++;
			if(ShutAlarmCount >=AlarmCountMax)
			{
				ShutAlarmCount = AlarmCountMax;
			}
			NoShutAlarmCount = 0;
		}
		else
		{
			ShutAlarmCount = 0;
			NoShutAlarmCount++;
			if(NoShutAlarmCount >=noAlarmCountMax)
			{
				NoShutAlarmCount = noAlarmCountMax;
			}
		}
		if(ShutAlarmCount >=AlarmCountMax)
		{
			Io_WriteAlarmBit(ALARM_BIT_POWER_SHUT, SET);//主电掉电置位
			Io_WriteAlarmBit(ALARM_BIT_POWER_LOW, RESET);//主电欠压清除
		}
		else if(NoShutAlarmCount >=noAlarmCountMax)
		{
			Io_WriteAlarmBit(ALARM_BIT_POWER_SHUT, RESET);//主电掉电清除
		}
		//变量初始化
		AdCount = 0;
		AdMin = 0xffff;
		AdMax = 0;
		AdSum = 0;	
	}
	
}
/*********************************************************************
//函数名称	:Io_DetectLoad
//功能		:检测空重车状态，空重车传感器故障
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:只有渣土车版本才有,用ADC1检测,100ms调度一次
//              :空车：3.2V~4.5V
//              :重车：2.0V~3.2V
//              :传感器短路：0V~2.0V
//              :传感器开路：4.5V~
*********************************************************************/
void Io_DetectLoad(void)
{
	static u8	AlarmCount = 0;
        static u8	UnAlarmCount = 0;
        static u8       StatusCount = 0;
        static u8       UnStatusCount = 0;
	static u32	AdSum = 0;//AD转换值累计和
	static u16	AdMin = 0xffff;//AD转换值最小值
	static u16	AdMax = 0;//AD转换值最大值
	static u8	AdCount = 0;//AD转换计数
        
       // static u16      LoadSensorEmptyMinValue = 32;//空载最小值
       // static u16      LoadSensorEmptyMaxValue = 45;//空载最大值
	u16	Ad;
        u8     AlarmCountMax = 3;
        u8     UnAlarmCountMax = 3;
        u8     AdCountMax = 10;
        u8     StatusCountMax = 3;
        u8     UnStatusCountMax = 3;
        
	AdCount++;
	Ad = Ad_GetValue(ADC_EXTERN1); 
	if(AdMin > Ad)
	{
		AdMin = Ad;
	}
	if(AdMax < Ad)
	{
		AdMax = Ad;
	}
	AdSum += Ad;
	if(AdCount >=  AdCountMax)
	{
		//计算平均值
		AdSum -= AdMin;
		AdSum -= AdMax;
		AdSum = AdSum >> 3;
		//转成以0.1V为单位的值
		AdSum = AdSum*33*9/0xfff;
		//置位或清除空重车传感器故障
		if((AdSum < LOAD_SENSOR_SHORT_VALUE)||(AdSum > LOAD_SENSOR_SHUT_VALUE))
		{
			AlarmCount++;
			if(AlarmCount >=AlarmCountMax)//实际应用取值
			{
			        AlarmCount = AlarmCountMax;
			}
			UnAlarmCount = 0;
		}
		else
		{
		        AlarmCount = 0;
			UnAlarmCount++;
			if(UnAlarmCount >= UnAlarmCountMax)//实际应用取值
			{
				UnAlarmCount = UnAlarmCountMax;
			}
                        
                              //置位或清除空重车状态
		        if((AdSum > LOAD_SENSOR_FULL_MIN_VALUE)&&(AdSum < LOAD_SENSOR_FULL_MAX_VALUE))
		        {
			      StatusCount++;
			      if(StatusCount >=StatusCountMax)//实际应用取值
			      {
			              StatusCount = StatusCountMax;
			      }
			      UnStatusCount = 0;
		        }
		        else
		        {
		              StatusCount = 0;
			      UnStatusCount++;
			      if(UnStatusCount >=UnStatusCountMax)//实际应用取值
			      {
				    UnStatusCount = UnStatusCountMax;
			      }
		        }
		        if(StatusCount ==StatusCountMax)
		        {
			      //置位空重车
                              Io_WriteStatusBit(STATUS_BIT_FULL_LOAD1,SET);
                              Io_WriteStatusBit(STATUS_BIT_FULL_LOAD2,SET);
                        
		        }
		        else if(UnStatusCount == UnStatusCountMax)
		        {
			    //清除空重车
                            Io_WriteStatusBit(STATUS_BIT_FULL_LOAD1,RESET);
                            Io_WriteStatusBit(STATUS_BIT_FULL_LOAD2,RESET);
		        }
		}
		if(AlarmCount ==AlarmCountMax)
		{
			//置位空重车传感器故障
                        Io_WriteSelfDefine2Bit(DEFINE_BIT_25, SET);
                        
		}
		else if(UnAlarmCount == UnAlarmCountMax)
		{
			//清除空重车传感器故障
                    Io_WriteSelfDefine2Bit(DEFINE_BIT_25, RESET);
		}
                
          
	
		//变量初始化
		AdCount = 0;
		AdMin = 0xffff;
		AdMax = 0;
		AdSum = 0;	
	}
	
}

/*********************************************************************
//函数名称	:Io_PowerOnDetectMainPower
//功能		:上电检测主电电压,判断是12V供电还是24V供电
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void Io_PowerOnDetectMainPower(void)
{
	
	u32	AdSum = 0;//AD转换值累计和
	u16	AdMin = 0xffff;//AD转换值最小值
	u16	AdMax = 0;//AD转换值最大值
	u8	Count = 0;//AD转换计数
	u16	Ad;

	for(Count = 0; Count<10; Count++)
	{
		Ad = Ad_GetValue(ADC_MAIN_POWER); 
		if(AdMin > Ad)
		{
			AdMin = Ad;
		}
		if(AdMax < Ad)
		{
			AdMax = Ad;
		}
		AdSum += Ad;
	}
	if(Count >= 10)
	{
		//计算平均值
		AdSum -= AdMin;
		AdSum -= AdMax;
		AdSum = AdSum >> 3;
		//转成以0.1V为单位的值
		AdSum = AdSum*33*9/0xfff;
		//if(AdSum >= 180)//大于等于18V认为是24V系统,否则是12V系统,dxl,2014.1.15屏蔽,把180V改成21V
        //if(AdSum > 210)//,dxl,2014.1.15
        if(AdSum > 190)//,dxl,2014.6.24,用21V作为24V系统的判断值会有bug,改为19V
		{
			CarVoltageType = 1;
			//PowerLowMinValue = 210;//欠压值范围
			PowerLowMaxValue = 220;
			PWR_24V();
		}
		else
		{
			CarVoltageType = 0;
			//PowerLowMinValue = 105;//欠压值范围
			PowerLowMaxValue = 110;
			PWR_12V();
		}
		
	}
	
}
/*********************************************************************
//函数名称	:Io_WriteStatusHighValidBit
//功能		:写StatusHighValid某一状态位
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:注意请不要在中断函数内调用此函数
*********************************************************************/
void Io_WriteStatusHighValidBit(STATUS_BIT bit, FlagStatus state)
{
	if(bit < 32)
	{
		if(SET==state)
		{
			StatusHighValid |= 1<<bit;
		}
		else if(RESET==state)
		{
			StatusHighValid &= ~(1<<bit);
		}
	
	}
}
/*********************************************************************
//函数名称	:Io_WriteExtCarStatusHighValidBit
//功能		:写ExtCarStatusHighValid某一状态位
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:注意请不要在中断函数内调用此函数
*********************************************************************/
void Io_WriteExtCarStatusHighValidBit(CAR_STATUS_BIT bit, FlagStatus state)
{
	if(bit < 32)
	{
		if(SET==state)
		{
			ExtCarStatusHighValid |= 1<<bit;
		}
		else if(RESET==state)
		{
			ExtCarStatusHighValid &= ~(1<<bit);
		}
	
	}
}
/*********************************************************************
//函数名称	:Io_WriteSelfDefine2HighValidBit
//功能		:写SelfDefine2HighValid某一状态位
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:注意请不要在中断函数内调用此函数
*********************************************************************/
void Io_WriteSelfDefine2HighValidBit(DEFINE_BIT bit, FlagStatus state)
{
	if(bit < 32)
	{
		if(SET==state)
		{
			SelfDefine2HighValid |= 1<<bit;
		}
		else if(RESET==state)
		{
			SelfDefine2HighValid &= ~(1<<bit);
		}
	
	}
}
/*********************************************************************
//函数名称	:Io_WriteAlarmHighValidBit
//功能		:写AlarmHighValid某一状态位
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:注意请不要在中断函数内调用此函数
*********************************************************************/
void Io_WriteAlarmHighValidBit(ALARM_BIT bit, FlagStatus state)
{
	if(bit < 32)
	{
		if(SET==state)
		{
			AlarmHighValid |= 1<<bit;
		}
		else if(RESET==state)
		{
			AlarmHighValid &= ~(1<<bit);
		}
	
	}
}
/*********************************************************************
//函数名称	:Io_GetCarVoltageType
//功能		:获取车辆电压类型
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
u8 Io_GetCarVoltageType(void)
{
	return CarVoltageType;
}
/*********************************************************************
//函数名称	:Io_ClearAlarmWord
//功能		:清除报警字各位
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void Io_ClearAlarmWord(void)
{
    u8 flag = 0;
	Alarm = 0;
    FRAM_BufferWrite(FRAM_EMERGENCY_FLAG_ADDR, &flag, FRAM_EMERGENCY_FLAG_LEN);
}
/*********************************************************************
//函数名称	:Io_ClearStatusWord
//功能		:清除状态字各位
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void Io_ClearStatusWord(void)
{
    Status = 0;
}
/*********************************************************************
//函数名称	:Io_ClearExtCarStatusWord
//功能		:清除扩展车辆状态字各位
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void Io_ClearExtCarStatusWord(void)
{
    ExtCarStatus = 0;
}
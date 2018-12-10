/*******************************************************************************
 * File Name:			NaviLcd.c 
 * Function Describe:	
 * Relate Module:		导航屏
 * Writer:				Joneming
 * Date:				2012-05-18
 * ReWriter:			
 * Date:				
 *******************************************************************************/
#include "include.h"

/*******************************************************************************/
#ifdef USE_NAVILCD
//1161789980

//#define DEBUG_TEST_NAVILCD 

extern u8 RadioShareBuffer[];
/*******************************************************************************/
/////////////////////////////////

/***************************************************************************************************
*  起始位	包长度	设备类型	厂商标识	命令字	数据区	校验码	 结束位

*  2 byte	2 byte	1 byte	     1 byte	    2 byte	 n byte	 1 byte	  2byte
******************************************************************************
******************************************************************************
*（1）	命令包长度定义为协议体中从设备类型到数据区这部分数据的字节数，
*       第1字节为命令包长度的高字节，第2字节为命令包长度的低字节，命令
*       包的长度为=高字节*256+低字节
*  3、 校验码：校验码定义为对协议体中从包长度到数据区这部分数据的异或运算
******************************************************************************
******************************************************************************

******************************************************************************************************/

/////////////////////////////////
/////////////设备控制指令//////////////////////////////////////////
#define NAVILCD_CMD_QUERY                       0x0000	//查询状态
#define NAVILCD_CMD_RESET                       0x0001	//设备复位指令
#define NAVILCD_CMD_SET_BAUDRATE                0x0002	//LCD通讯波特率设置
#define NAVILCD_CMD_DISPLAY                     0x0003	//LCD屏的开启/关闭
#define NAVILCD_CMD_CALI_TIME                   0x0004	//校时
#define NAVILCD_CMD_SET_LCD_PARAM               0x0005	//LCD设备参数设置指令
#define NAVILCD_CMD_HEART_BEAT                  0x0006	//心跳命令
#define NAVILCD_CMD_SYSTEM_UPDATE               0x00FF	//固件升级指令
/////////////////控制/设置命令/////////////////////////////////////////
#define NAVILCD_CMD_TERMINAL_CONTROL            0x0101	//控制终端命令
#define NAVILCD_CMD_TERMINAL_SET_PARAM          0x0102	//设置参数操作命令
#define NAVILCD_CMD_TERMINAL_QUERY_PARAM        0x0103	//参数查询命令
#define NAVILCD_CMD_QUERY_STATUS_ALARM          0x0104	//查询状态报警信息命令
#define NAVILCD_CMD_QUERY_WORK_STATUS           0x0105	//查询工作状态命令
#define NAVILCD_CMD_MODIFY_PASSWORD             0x0106	//修改密码命令
#define NAVILCD_CMD_REGISTER_OUT_CITY           0x0107	//出城登记
#define NAVILCD_CMD_EXPORT_U_DISK               0x0108	//U盘数据导出
#define NAVILCD_CMD_CARLOAD_WEIGHT              0x0109	//载重指令
#define NAVILCD_CMD_SET_PJA_VALID               0x0110	//评价有效
#define NAVILCD_CMD_PJA_RESULT                  0x0111	//评价结果
///////////////电话/////////////////////////
#define NAVILCD_CMD_DIAL_PHONE                  0x0201	//拨号命令
#define NAVILCD_CMD_NEW_CALL_PHONE              0x0202	//新来电命令
#define NAVILCD_CMD_LCD_KEY_VALUE               0x0203	//LCD发送按键值到终端命令
#define NAVILCD_CMD_TERMINAL_KEY_VALUE          0x0204	//终端发送按键值到LCD命令
#define NAVILCD_CMD_QUERY_PHONE_BOOK            0x0205	//查询终端电话簿命令
#define NAVILCD_CMD_ADJUST_VOLUME               0x0206	//调节音量
//////////////////信息服务///////////////////////////////////
#define NAVILCD_CMD_TEXT_DISPLAY                0x0301	//文本信息下发命令
#define NAVILCD_CMD_QUESTION_DISPLAY            0x0302	//提问下发命令
#define NAVILCD_CMD_QUESTION_RESPONSE           0x0303	//发送提问答案命令
#define NAVILCD_CMD_QUERY_FIX_TEXT              0x0304	//查询固定文本信息命令
#define NAVILCD_CMD_SEND_FIX_TEXT               0x0305	//发送固定文本信息命令
#define NAVILCD_CMD_SEND_TICKLE_INFO            0x0306	//发送反馈信息命令
#define NAVILCD_CMD_INFO_SERVICE                0x0307	//信息服务命令
/////////////////电召服务/////////////////////////////////////
#define NAVILCD_CMD_CALL_TAXI_NEW               0x0401	//新电召信息命令
#define NAVILCD_CMD_CALL_TAXI_VIE               0x0402	//抢标命令
#define NAVILCD_CMD_CALL_TAXI_DETAIL            0x0403	//电召详细信息命令
#define NAVILCD_CMD_CALL_TAXI_FINISH            0x0404	//电召完成汇报命令
#define NAVILCD_CMD_CALL_TAXI_CANCEL_TERMINAL   0x0405	//终端请求取消电召命令
#define NAVILCD_CMD_CALL_TAXI_CANCEL_PLATFORM   0x0406	//平台取消电召命令
//////////////////驾驶员信息////////////////////////////////////
#define NAVILCD_CMD_DRIVER_SIGN                 0x0501	//驾驶员签到/签退命令
#define NAVILCD_CMD_DRIVER_ADD                  0x0502	//增加/修改驾驶员信息命令
#define NAVILCD_CMD_DRIVER_RECORD               0x0503	//驾驶记录查询命令
#define NAVILCD_CMD_DRIVER_REPORT               0x0504	//驾驶员有关业务上报命令
#define NAVILCD_CMD_DRIVER_QUERY_ALL            0x0505	//查询所有驾驶员信息命令
#define NAVILCD_CMD_DRIVER_ADD_RESULT           0x0506	//增加/修改驾驶员信息结果答应命令
#define NAVILCD_CMD_DRIVER_MODIFY               0x0507	//修改当前驾驶员工号/密码命令
#define NAVILCD_CMD_DRIVER_DEL_ALL              0x0508	//删除所有驾驶员命令
#define NAVILCD_CMD_DRIVER_BRUSH_CARD           0x0509	//显示屏刷驾驶员卡命令
#define NAVILCD_CMD_WRITE_IC_CARD               0x050A	//写卡命令
#define NAVILCD_CMD_READ_IC_CARD                0x050B	//读卡命令
#define NAVILCD_CMD_CURRENT_DRIVER_INFO         0x050C	//当前驾驶员身份信息命令
///////////////////信息透传///////////////////////////////////
#define NAVILCD_CMD_PLATFORM_TRANS_DOWN         0x0601	//平台数据透传给LCD命令
#define NAVILCD_CMD_LCD_TRANS_UP                0x0602	//LCD数据透传给平台命令
////////////////////////////////
#define NAVILCD_CMD_TERMINAL_TRANS_TO_ICCARD    0x0603	//终端数据透传给刷卡模块
#define NAVILCD_CMD_ICCARD_TRANS_TO_TERMINAL    0x0604	//刷卡模块数据透传给终端命令
///////////////////////////////////////////////////////
#define NAVILCD_CMD_TERMINAL_TRANS_TO_CTRL_UNIT 0x0605	//终端透传给控制板指令
#define NAVILCD_CMD_CTRL_UNIT_TRANS_TO_TERMINAL 0x0606	//终端透传给控制板指令
/////////////////////////////////////////////////////
#define NAVILCD_CMD_LCD_TRANS_TO_LOAD_SENSOR    0x0607	//LCD透传给载重传感器指令
#define NAVILCD_CMD_LOAD_SENSOR_TRANS_TO_LCD    0x0608	//载重传感器给透传LCD指令
////////////////////导航//////////////////////////////////
////////////////////导航//////////////////////////////////
#define NAVILCD_CMD_NAVIGATION                  0x0701	//导航命令
#define NAVILCD_CMD_ONE_KEY_NAVIGATION          0x0702	//一键导航命令
//////////////////////////////////////////////////////
/////////以下属于顺丰扩展命令////////////////////////////////
#define NAVILCD_CMD_SF_EXPRESS_LCD_TRANS_UP         0x0801	//顺丰显示屏透传指令
#define NAVILCD_CMD_SF_EXPRESS_LCD_TRANS_RESPONSE   0x0802	//顺丰显示屏透传应答指令
#define NAVILCD_CMD_SF_EXPRESS_SET_LCD_PARAM        0x0803	//设置显示屏参数
#define NAVILCD_CMD_SF_EXPRESS_QUERY_LCD_PARAM      0x0804	//查询显示屏参数
#define NAVILCD_CMD_SF_EXPRESS_BRUSH_CARD_DATA      0x0805	//刷顺丰卡数据命令
/////////////////////////////////////////////////////////
#define NAVILCD_CMD_GNSS_CHECK_UP                   0xFE01	//送检协议指令

#define NAVILCD_USER_DATA_START 8

const unsigned char c_ucNaviLcdTag[2]={0x55,0xaa};    //标识

#define NAVILCD_FACTORY_NO        0x03//0x03   //厂商编号
#define EQUIP_TYPE_NAVILCD_NO     0x09         //设备类型(LCD)


typedef enum
{
    NV_RECV_OK,             //接收正确
    NV_RECV_ERR_START,      //开头标志不对
    NV_RECV_ERR_TYPE,       //类型不对
    NV_RECV_ERR_LEN,        //长度不对
    NV_RECV_ERR_END,        //结束标志不对
    NV_RECV_ERR_VERIFY,     //校验不对
    NV_RECV_MAX             //
}NAVILCDRECV;

/////////////////////////////////////
typedef struct
{
    unsigned short recvCmd;         //收到的命令
    unsigned short recvDatalen;     //接收数据区长度
    unsigned char onlinestaus;      //是否在线
    unsigned char CheckCnt;         //检测用，计数器
    unsigned char ProcessStep;      //进程 
    unsigned char RecvAnswerFlag;   //收到应答
    unsigned char AccStatus;        //Acc状态
    unsigned char newphone;         //
    unsigned char logout;           //
    unsigned short AccOffCnt;           //Acc关的计数器
    unsigned char LcdPowerOn;           //lcd电源开关
    unsigned char AccTurnPrompt;        //提示驾驶员标志
    unsigned char VoicePrompt;          //声音提示标志
}
T_NAVILCD_ATTRIB;


static T_NAVILCD_ATTRIB s_stNaviLcdAttrib;

typedef enum
{
    NAVILCD_EMPTY_STATE,          //空
    NAVILCD_QUARY_STATE,          //
    NAVILCD_EQUIP_OPEN_STATE,     //
    NAVILCD_DRIVER_SIGN_OUT,      //
    NAVILCD_SERVICE_REPAIR,       //
    NAVILCD_MAX_STATE             //
}E_NAVILCD_STATE;

typedef enum
{
    TERMINAL_CTRL_EMPTY,
    TERMINAL_CTRL_REGISTER,         //终端注册
    TERMINAL_CTRL_CONFIRM_ID,       //终端鉴权
    TERMINAL_CTRL_LOGOUT,           //终端注销
    TERMINAL_CTRL_RESET,            //终端复位
    TERMINAL_CTRL_FEATURE,          //开启特征系数校准
    TERMINAL_CTRL_DEFAULT_SET,      //恢复出厂参数设置
    TERMINAL_CTRL_QUICK_OPEN_ACCOUNT,//快速开户
    TERMINAL_CTRL_CAR_LOAD_STATUS,   //车辆载货状态
    TERMINAL_CTRL_MAX       //
}E_NAVILCD_TERMINALCTRL;

typedef enum
{
    NAVILCD_KEYVALUE_EMPTY,                     //空
    NAVILCD_KEYVALUE_HOT_LINE  =0x0A,           //拨打热线电话
    NAVILCD_KEYVALUE_ANSWER_CALL,               //接听电话
    NAVILCD_KEYVALUE_HANG_UP,                   //挂断
    NAVILCD_KEYVALUE_HF,                        //免提
    NAVILCD_KEYVALUE_REDIAL,                    //重拨
    NAVILCD_KEYVALUE_DEL,                       //删除
    NAVILCD_KEYVALUE_VOL_ADD,                   //音量+
    NAVILCD_KEYVALUE_VOL_DEC,                   //音量-
    NAVILCD_KEYVALUE_ASTERISK='*',              //*键 
    NAVILCD_KEYVALUE_POUND='#',                 //#键
    NAVILCD_KEYVALUE_KEY0   =0x30,              //0键
    NAVILCD_KEYVALUE_KEY1,                      //1键
    NAVILCD_KEYVALUE_KEY2,                      //2键
    NAVILCD_KEYVALUE_KEY3,                      //3键
    NAVILCD_KEYVALUE_KEY4,                      //4键
    NAVILCD_KEYVALUE_KEY5,                      //5键
    NAVILCD_KEYVALUE_KEY6,                      //6键
    NAVILCD_KEYVALUE_KEY7,                      //7键
    NAVILCD_KEYVALUE_KEY8,                      //8键
    NAVILCD_KEYVALUE_KEY9,                      //9键
    NAVILCD_KEYVALUE_MAX       //
}E_NAVILCD_KEYVALUE;

typedef enum
{
    FIXTEXT_TYPE_PHRASE,            //固定短语
    FIXTEXT_TYPE_EVENT,             //事件报告
    FIXTEXT_TYPE_INFO,              //信息点播
    FIXTEXT_TYPE_MAX                //
}E_NV_FIXTEXTTYPE;

typedef enum
{
    DRIVER_SPEED_RECORD,     //超速记录
    DRIVER_TIRED_RECORD,     //疲劳驾驶记录
    DRIVER_STOP_RECORD,      //停车打印
    DRIVER_INFO_RECORD,      //驾驶员信息
    DRIVER_RECORD_MAX       //
}E_NAVILCD_DRIVER;

typedef enum
{
    TERMINAL_PARAM_EMPTY,               //0空
    TERMINAL_PARAM_MAIN_APN,            //1主服务器 APN
    TERMINAL_PARAM_RADIO_USER,          //2主服务器无线通信拨号用户名
    TERMINAL_PARAM_RADIO_PASSWORD,      //3主服务器无线通信拨号密码
    TERMINAL_PARAM_MAIN_IP,             //4主服务器地址,IP 或域名
    TERMINAL_PARAM_BAK_APN,             //5备份服务器 APN，无线通信拨号访问点
    TERMINAL_PARAM_BAK_RADIO_USER,      //6备份服务器无线通信拨号用户名
    TERMINAL_PARAM_BAK_RADIO_PASSWORD,  //7备份服务器无线通信拨号密码
    TERMINAL_PARAM_BAK_IP,              //8备份服务器地址,IP 或域名
    TERMINAL_PARAM_TCP,                 //9服务器 TCP 端口 
    TERMINAL_PARAM_UDP,                 //10服务器 UDP 端口
    TERMINAL_PARAM_SPEED_ALARM,         //11最高速度，单位为公里每小时（km/h）
    TERMINAL_PARAM_TOTAL_MILE,          //12车辆里程表读数，1/10km 
    TERMINAL_PARAM_CAR_PLATE,           //13公安交通管理部门颁发的机动车号牌
    TERMINAL_PARAM_CAR_COLOR,           //14车牌颜色，按照 JT/T415-2006 的 5.4.12
    TERMINAL_PARAM_PHONE_NUMBER,        //15终端手机号
    TERMINAL_PARAM_CAR_VIN,             //16车辆识别码
    TERMINAL_PARAM_CAR_PLATE_TYPE,      //17车牌分类
    TERMINAL_PARAM_SPEED_SENSOR,        //18速度传感器系数
    TERMINAL_PARAM_FEATURE_COEF,        //19车辆特征系数
    TERMINAL_PARAM_SPEED_SELECT,        //20速度选择开关
    TERMINAL_PARAM_SET_BACKLIGHT,       //21背光选择开关
    TERMINAL_PARAM_DOOR_LEVEL,          //22车门正负设置
    TERMINAL_PARAM_CDMA_SLEEP,          //23CDMA休眠开关
    TERMINAL_PARAM_CAR_HORN_LEVEL,      //24车喇叭电平选择
    TERMINAL_PARAM_TERMINAL_ID,         //25终端ID(LCD只可查询，不可写入)
    TERMINAL_PARAM_PASSWORD,            //26密码
    TERMINAL_PARAM_WEIGHT,              //27重量
    TERMINAL_PARAM_OWNER_PHONE,         //28车主联系手机号
    TERMINAL_PARAM_CAR_PROVINCE_ID,     //29省域ID
    TERMINAL_PARAM_CAR_CITY_ID,         //30市县域ID
    TERMINAL_PARAM_MAIN_DOMAIN_NAME,    //31主监控中心域名
    TERMINAL_PARAM_BACKUP_DOMAIN_NAME,  //32备份监控中心域名
    TERMINAL_PARAM_MANUFACTURE_ID,      //33制造商ID
    TERMINAL_PARAM_MAX       //
}E_NAVILCD_TERMINALPARAM;


////////////////////////////////////////
typedef enum
{
    NV_GNSS_CHECK_EMPTY,            //空
    NV_GNSS_CHECK_TALK_PHONE,       //拔打电话
    NV_GNSS_CHECK_ANSWER_PHONE,     //接听电话
    NV_GNSS_CHECK_HANG_UP_PHONE,    //挂断电话
    NV_GNSS_CHECK_START_RECORD,     //开始录音
    NV_GNSS_CHECK_STOP_RECORD,      //停止录音    
    NV_GNSS_CHECK_FIX_PHRASE,       //发送固定短语 
    NV_GNSS_CHECK_RTU_REGISTER,     //终端注册
    NV_GNSS_CHECK_RTU_QUALIFY,      //终端鉴权
    NV_GNSS_CHECK_POSITION_INFO,    //位置信息汇报
    NV_GNSS_CHECK_EVENT_REPORT,     //事件报告    
    NV_GNSS_CHECK_INFO_DEMAND,      //信息点播   
    NV_GNSS_CHECK_CANCEL_IOD,       //取消信息点播
    NV_GNSS_CHECK_E_WAYBILL,        //电子运单上报
    NV_GNSS_CHECK_DRIVER_INFO,      //驾驶员身份信息
    NV_GNSS_CHECK_MULIT_EVENT,      //上发多媒体事件
    NV_GNSS_CHECK_DTU_DATA,         //数据上行透传    
    NV_GNSS_CHECK_RESERVE1,         //保留操作1   
    NV_GNSS_CHECK_RESERVE2,         //保留操作2
    NV_GNSS_CHECK_RESERVE3,         //保留操作3
    NV_GNSS_CHECK_MAX       //
}E_NAVILCD_GNSS_CHECK;//北斗送检指令宏定义


enum 
{
    NV_TIMER_CHECK_ACC,					//
    NV_TIMER_RESEND,					// 
    NV_TIMER_SEND_END,                  //
    NV_TIMER_BUSY_TIMEOUT,              //
    NV_TIMER_TASK,						//
    NV_TIMER_TEST,						//
    NV_TIMER_KEY,    
    NV_TIMER_STATUS,					//
    NV_TIMER_DRIVER_ADD,				//
    NV_TIMERS_MAX
}E_NAVILCDTIME;

static LZM_TIMER s_stNVTimer[NV_TIMERS_MAX];

typedef enum
{
    LCD_SF_PROTOCOL_EMPTY,                      //空  
    LCD_SF_PROTOCOL_DRIVE_LOG =0x37,              //行车日志
    LCD_SF_PROTOCOL_BRUSH_CARD_LOGIN =0x39,       //上下班打卡产
    LCD_SF_PROTOCOL_MAX       //
}E_LCDSFPROTOCOL;


#define COMNAVILCD_MAX_REC_LEN     512 //接收数据的长度
#define COMNAVILCD_MAX_SEND_LEN    1024 //发送数据的长度

#define COMNAVILCD_MIN_LEN  11
//结构类型
typedef struct{	
	unsigned short recvlen;          //串口接收数据长度,接收并处理完后清除,初始化清0	
	unsigned short datalen;         //打包数据长度
	unsigned short disposelen;      //已处理数据长度	
	unsigned char sendBusy;         //串口的发送状态 false:空闲;  true:忙  (发送前判断,发送前置位,数据发送完成清除,初始化清0)//3个定时周期自动清
	unsigned char resendCnt;        //尝试重发发送次数
	unsigned char recvtimeoutCnt;   //接收超时次数
	unsigned char contrecvtimeoutCnt; //连续接收超时次数
	unsigned char conttryCnt;       //连续尝试
	unsigned char responseFlag;     //是否需要应答标志	
    unsigned char recvfreeCnt;      //接收空闲计数器    
	unsigned char recvBuff[COMNAVILCD_MAX_REC_LEN+1];     //解析缓冲
}ST_COMNAVILCD;
////////////////////////////////
static unsigned char s_ucNaviLcdTxBuffer[COMNAVILCD_MAX_SEND_LEN+1];//发送
static unsigned char s_ucNaviLcdParseBuffer[COMNAVILCD_MAX_REC_LEN+1];//解析之后缓冲
////////////////////////////////
//函数声明
void GetNaviLcdCommRxData(void) ; //接收数据
void PutNaviLcdCommTxData(void) ;//发送数据数据
void NaviLcdCommSendEnd(void);//发送结束
unsigned char IsNaviLcdCommBusy(void);//判断串口是否繁忙
unsigned char MayNaviLcdCommSend(void); //判断是否满足发送条件
unsigned char NaviLcdProtocolParse(unsigned char *ParseBuffer,unsigned short usDataLenght);
unsigned char Public_GetXorVerify(const unsigned char *Pdata,unsigned short DataLength);
void NaviLcd_DisposeRecvDataResponse(void);
unsigned char NaviLcdCommResendData(unsigned char *Ptr,unsigned short Len);
unsigned char NaviLcd_CheckDailPhone(unsigned char *phone);
void NaviLcd_DisposeHangUpPhoneStatus(void);
void NaviLcd_BrushCardDataDebugTest(void);
void NaviLcd_SetTerminalPassWordToLcd(void);
void NaviLcd_DisposePJAResult(void);
////////////////////////////////
void NaviLcd_SetCurrentStatus(unsigned char Status);

const char * c_apcRegisterInfo[] = 
{
    "注册成功!",//0
    "车辆已被注册!",//1
    "数据库无该车辆!",//2
    "终端已被注册!",//3
    "数据库无该终端!",//4
    "鉴权码长度超出!",//5
    "鉴权成功!",//6
    "鉴权失败!",//7
};


static char *c_apcCarPlakeTypeItems[]=
{
    "大型汽车牌号",
    "挂车牌号",
    "小型汽车牌号",
    "其它牌号",
    ""
};
//静态变量定义
static ST_COMNAVILCD s_stNaviLcdComm;  //串口变量

//结构类型
typedef struct{	
	unsigned char datalen;         //
	unsigned char Keybuff[32];     //
}STNVKEY;

static STNVKEY s_stNVKey;

//////////////////////////////////////////////
enum 
{
    NV_DRIVERLOG_STATUS_EMPTY =0x55,//
    NV_DRIVERLOG_STATUS_LOGOUT,     //
    NV_DRIVERLOG_STATUS_LOGIN,      //
    NV_DRIVERLOG_STATUS_MAX
}DRIVERLOGSTATUS;

enum 
{
    NV_CMD_FLAG_EMPTY,      //空
    NV_CMD_FLAG_BUSINESS,   //业务上报 
    NV_CMD_FLAG_LOGIN,      //刷卡签到  
    NV_CMD_FLAG_LOGOUT,     //刷卡签退 
    NV_CMD_FLAG_ADD,        //添加/修改驾驶员    
    NV_CMD_FLAG_MAX
}DRIVERLOGFLAG;

typedef struct
{  
     unsigned char statusFlag;   //状态标志
     unsigned char recordNumber; //记录号
     unsigned char cmdFlag;      //指令标志
     unsigned char recordindex;   //添加/修改的顺序号
     unsigned char resutltindex;  //顺序号
     unsigned char result;   //结果标志
     unsigned char drivercode[6]; //工号
     unsigned char password[6];   //密码
     unsigned char loginTime[6];//登录时间
}STDRIVERLOG;

static STDRIVERLOG s_stDriverLog;

///////////////////////////
const char *c_apcFixPhrase[] = 
{
    "塞车",
    "故障",
    "事故",
    "抛锚",
    "请指示加油站",
    "停车修理",
    "车辆加油", 
    "已发车",
    "已到达",
    "大雾雷雨",
    "医疗求助",
    "迷路,请指示",
    "车内有危险",
    "违章",
	""
};
#define FIX_PHRASE_ITEMS 14

void NaviLcdSaveDriverLogin(unsigned char recordnumber);
void NaviLcdSaveDriverLogout(void);
void NaviLcdSendCmdQueryStatus(void);
void NaviLcd_DebugToWriteICcard(void);

/*************************************************************OK
** 函数名称: NaviLcdPowerIoInit
** 功能描述: 允许发送
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/  
void NaviLcdPowerIoInit(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
}

/*************************************************************OK
** 函数名称: NaviLcdPowerOn
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/  
void NaviLcdPowerOn(void)
{
    GPIO_SetBits(GPIOE, GPIO_Pin_11);//
}
/*************************************************************OK
** 函数名称: NaviLcdPowerOn
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/  
void NaviLcdPowerOff(void)
{
    if(1 == ReadPeripheral2TypeBit(5))//bit5:接了温度传感器，ACC OFF时不关闭电源
    {
    
    }
    //else if(1 == ReadPeripheral2TypeBit(15))//bit15:接了调度屏，ACC OFF时会关闭电源
    //{
    //        GPIO_ResetBits(GPIOE, GPIO_Pin_11);//
    //}
    else if(1 == ReadPeripheral2TypeBit(1))//bit1:接了CAN盒子，ACC OFF时会关闭电源
    {
            GPIO_ResetBits(GPIOE, GPIO_Pin_11);//
    }
	
#if (ICCARD_JTD == ICCARD_SEL)
	GPIO_ResetBits(GPIOE, GPIO_Pin_11);//
#endif
}

/*************************************************************OK
** 函数名称: NaviLcdPowerOn
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/  
void NaviLcdDelayms(unsigned char ms)
{
    unsigned char i,j;
    for(i=ms; i>0;i--)
    {
        for(j=200; j>0;j--);
    }
}

/************************************************************
** 函数名称: NaviLcdCommBusyTimeOut
** 功能描述: 
             
** 入口参数:
** 出口参数:
************************************************************/ 
void NaviLcdCommBusyTimeOut(void)
{
    s_stNaviLcdComm.sendBusy = 0;
}
/************************************************************
** 函数名称: NaviLcdCommResponeTimeout
** 功能描述: 没有答应则重发三次
             
** 入口参数:
** 出口参数:
************************************************************/ 
void NaviLcdCommResponeTimeout(void)
{
    s_stNaviLcdComm.recvtimeoutCnt++;
    s_stNaviLcdComm.contrecvtimeoutCnt++;
    if(s_stNaviLcdComm.contrecvtimeoutCnt > 4)
    {
        LZM_PublicKillTimer(&s_stNVTimer[NV_TIMER_STATUS]);
        s_stNaviLcdComm.sendBusy = 0;
        s_stNaviLcdComm.recvtimeoutCnt = 0;
        s_stNaviLcdComm.conttryCnt++;
        s_stNaviLcdAttrib.onlinestaus = 0;
        if(s_stNaviLcdComm.conttryCnt<2)
        {
            #ifndef DEBUG_TEST_NAVILCD
            //if(1 == ReadPeripheral2TypeBit(15))//bit15:调度屏,1为接入了,0为没接,dxl,2014.12.30
			#if (ICCARD_JTD == ICCARD_SEL)
			{
                NaviLcdPowerOff();
            }
			#endif
            #endif
            LZM_PublicSetOnceTimer(&s_stNVTimer[NV_TIMER_STATUS],PUBLICSECS(5),NaviLcdPowerOn); 
            s_stNaviLcdComm.contrecvtimeoutCnt = 0;
            LZM_PublicSetOnceTimer(&s_stNVTimer[NV_TIMER_TASK],PUBLICSECS(15),NaviLcdSendCmdQueryStatus); 
        }
        else
        {
            Io_WriteAlarmBit(ALARM_BIT_LCD_FAULT,SET);            
            LZM_PublicSetOnceTimer(&s_stNVTimer[NV_TIMER_TASK],PUBLICSECS(30),NaviLcdSendCmdQueryStatus); 
        }
    }
    else
    if(s_stNaviLcdComm.recvtimeoutCnt < 3)//////////////////
    {
        NaviLcdCommResendData(s_ucNaviLcdTxBuffer,s_stNaviLcdComm.datalen);
    }
    else////超时//////////////////
    {
        s_stNaviLcdComm.sendBusy = 0;
        s_stNaviLcdComm.recvtimeoutCnt = 0;
        LZM_PublicKillTimer(&s_stNVTimer[NV_TIMER_STATUS]);
        LZM_PublicSetOnceTimer(&s_stNVTimer[NV_TIMER_TASK],PUBLICSECS(6),NaviLcdSendCmdQueryStatus);
    }
}
/************************************************************
** 函数名称: NaviLcdCommSendEnd
** 功能描述: 发送完毕
             
** 入口参数:
** 出口参数:
************************************************************/ 
void NaviLcdCommSendEnd(void)
{  
    if(s_stNaviLcdComm.responseFlag)//需要判断接收超时
    {
        LZM_PublicSetOnceTimer(&s_stNVTimer[NV_TIMER_TASK],PUBLICSECS(1.8),NaviLcdCommResponeTimeout);
        LZM_PublicSetOnceTimer(&s_stNVTimer[NV_TIMER_BUSY_TIMEOUT], PUBLICSECS(0.8), NaviLcdCommBusyTimeOut);
    }
    else
    {
        s_stNaviLcdComm.sendBusy = 0;
        s_stNaviLcdComm.recvtimeoutCnt = 0;
    }
}
/************************************************************
** 函数名称: IsNaviLcdCommBusy
** 功能描述: 取得当前串口状态
             
** 入口参数:
** 出口参数:
************************************************************/ 
unsigned char IsNaviLcdCommBusy(void)
{
    return COM2_GetSendBusy();
}
/************************************************************
** 函数名称: NaviLcdCommStartToSendData
** 功能描述: 
             
** 入口参数:
** 出口参数:
************************************************************/ 
unsigned char NaviLcdCommStartSendData(unsigned char *pBuffer,unsigned short datalen)
{     
    unsigned char result;
    result=COM2_WriteBuff(pBuffer,datalen);
    LZM_PublicSetOnceTimer(&s_stNVTimer[NV_TIMER_SEND_END],LZM_AT_ONCE,NaviLcdCommSendEnd);    
    return result;
    //USART_ITConfig(USART1, USART_IT_TC, ENABLE); //启动发送
}

/************************************************************
** 函数名称: NaviLcdCommBusyForResendData
** 功能描述: 重发时间到，执行重发处理
             
** 入口参数:
** 出口参数:
************************************************************/ 
void NaviLcdCommBusyForResendData(void)
{
    if(0==NaviLcdCommStartSendData(s_ucNaviLcdTxBuffer,s_stNaviLcdComm.datalen))//不忙,直接发送
    {
        s_stNaviLcdComm.resendCnt = 0;
    }
    ///////////
    if(s_stNaviLcdComm.resendCnt)//开启重发
    {
        s_stNaviLcdComm.resendCnt--;
        LZM_PublicSetOnceTimer(&s_stNVTimer[NV_TIMER_RESEND], PUBLICSECS(0.15), NaviLcdCommBusyForResendData);  
    }
}
/************************************************************
** 函数名称: NaviLcdCommStartToSendData
** 功能描述: 
             
** 入口参数:
** 出口参数:
************************************************************/ 
void NaviLcdCommClearResponse(void)
{
    s_stNaviLcdComm.responseFlag = 0;
}
/************************************************************
** 函数名称: NaviLcdCommStartToSendData
** 功能描述: 
             
** 入口参数:
** 出口参数:
************************************************************/ 
unsigned char NaviLcdCommSendData(unsigned char *pBuffer,unsigned short datalen)
{
    if(datalen>COMNAVILCD_MAX_SEND_LEN)return 0xff;
    s_stNaviLcdComm.recvtimeoutCnt = 0;
    s_stNaviLcdComm.responseFlag = 1;
    if(0==NaviLcdCommStartSendData(pBuffer,datalen))return 0;//发送成功 
    #if 1
    s_stNaviLcdComm.datalen = datalen;
    memcpy(s_ucNaviLcdTxBuffer,pBuffer,s_stNaviLcdComm.datalen);
    s_stNaviLcdComm.resendCnt = 3;//重发送次数
    LZM_PublicSetOnceTimer(&s_stNVTimer[NV_TIMER_RESEND], PUBLICSECS(0.15), NaviLcdCommBusyForResendData); 
    return 1;//等待重发
    #else
    if(0==s_stNaviLcdComm.resendCnt)
    {
        s_stNaviLcdComm.datalen = datalen;
        memcpy(s_ucNaviLcdTxBuffer,pBuffer,s_stNaviLcdComm.datalen);
        s_stNaviLcdComm.resendCnt = 3;//重发送次数
        LZM_PublicSetOnceTimer(&s_stNVTimer[NV_TIMER_RESEND], PUBLICSECS(0.15), NaviLcdCommBusyForResendData); 
        return 1;//等待重发
    }
    else//串口忙,发送失败
    if((s_stNaviLcdComm.datalen+datalen)<COMNAVILCD_MAX_SEND_LEN)
    {
        memcpy(&s_ucNaviLcdTxBuffer[s_stNaviLcdComm.datalen],pBuffer,datalen);
        s_stNaviLcdComm.datalen += datalen;
        return 1;//等待重发
    }  
    else//
    {
        return 2;//失败
    } 
    #endif    
}
/************************************************************
** 函数名称: NaviLcdCommResendData
** 功能描述: 
             
** 入口参数:
** 出口参数:
************************************************************/ 
unsigned char NaviLcdCommResendData(unsigned char *pBuffer,unsigned short datalen)
{
    if(datalen>COMNAVILCD_MAX_SEND_LEN)return 0xff;
    if(0==NaviLcdCommStartSendData(pBuffer,datalen))return 0;//发送成功 
    #if 1
    s_stNaviLcdComm.datalen = datalen;
    memcpy(s_ucNaviLcdTxBuffer,pBuffer,s_stNaviLcdComm.datalen);
    s_stNaviLcdComm.resendCnt = 3;//重发送次数
    LZM_PublicSetOnceTimer(&s_stNVTimer[NV_TIMER_RESEND], PUBLICSECS(0.15), NaviLcdCommBusyForResendData); 
    return 1;//等待重发
    #else
    if(0==s_stNaviLcdComm.resendCnt)
    {
        s_stNaviLcdComm.datalen = datalen;
        memcpy(s_ucNaviLcdTxBuffer,pBuffer,s_stNaviLcdComm.datalen);
        s_stNaviLcdComm.resendCnt = 3;//重发送次数
        LZM_PublicSetOnceTimer(&s_stNVTimer[NV_TIMER_RESEND], PUBLICSECS(0.15), NaviLcdCommBusyForResendData); 
        return 1;//等待重发
    }
    else//串口忙,发送失败
    if((s_stNaviLcdComm.datalen+datalen)<COMNAVILCD_MAX_SEND_LEN)
    {
        memcpy(&s_ucNaviLcdTxBuffer[s_stNaviLcdComm.datalen],pBuffer,datalen);
        s_stNaviLcdComm.datalen += datalen;
        return 1;//等待重发
    }  
    else//
    {
        return 2;//失败
    } 
    #endif
}
/*************************************************************
** 函数名称: PactSendCmdForNaviLcd
** 功能描述: 对数据进行打包,并发送
** 入口参数: cmd:命令，data:数据块首地址,datalen:长度		 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void PactSendCmdForNaviLcd(unsigned short cmd,unsigned char *data,unsigned short datalen)
{
    unsigned short i;
    unsigned short len;
    unsigned char VerifyCode;    
    i=0;
    s_ucNaviLcdTxBuffer[i++]=0xff;
    ///////////////////////
    s_ucNaviLcdTxBuffer[i++]=c_ucNaviLcdTag[0];
    s_ucNaviLcdTxBuffer[i++]=c_ucNaviLcdTag[1];
    //////////////////////////////////
    len=datalen+4;
    ///////////////////////////////////
    s_ucNaviLcdTxBuffer[i++] = (len>>8);
    //////////////////////////////////
    s_ucNaviLcdTxBuffer[i++] = (len & 0xff);
    //////////////////////////////////
    s_ucNaviLcdTxBuffer[i++] = EQUIP_TYPE_NAVILCD_NO;
    
    //s_ucNaviLcdTxBuffer[i++] = NAVILCD_FACTORY_NO;
    s_ucNaviLcdTxBuffer[i++] = 0x00;
    ///////////////////////
    s_ucNaviLcdTxBuffer[i++] = (cmd>>8);
    //////////////////////////////////
    s_ucNaviLcdTxBuffer[i++] = (cmd & 0xff);
    /////////////////////////////////////////////
    len = i;
    for(i=0;i<datalen;i++)s_ucNaviLcdTxBuffer[len+i]=data[i];
    /////////////////////////////////////////////
    len  +=datalen;
    VerifyCode=Public_GetXorVerify(&s_ucNaviLcdTxBuffer[3],len-3);
    /////////////
    s_ucNaviLcdTxBuffer[len++]=VerifyCode;
    //////////////////////
    s_ucNaviLcdTxBuffer[len++] = c_ucNaviLcdTag[0];
    s_ucNaviLcdTxBuffer[len++] = c_ucNaviLcdTag[1];
    /////////////////////////////////
    s_ucNaviLcdTxBuffer[len++] = 0xff;
    ///////////////////////////
    s_stNaviLcdAttrib.RecvAnswerFlag = 0;
    s_stNaviLcdComm.datalen = len;
    ///////////////////
    NaviLcdCommSendData(s_ucNaviLcdTxBuffer,len);
}
/*************************************************************
** 函数名称: PactSendCmdForNaviLcd
** 功能描述: 对数据进行打包,并发送
** 入口参数: cmd:命令，data:数据块首地址,datalen:长度		 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void PactSendCmdForNaviLcdEx(unsigned short cmd,unsigned char *data,unsigned short datalen)
{
    unsigned short i;
    unsigned short len;
    unsigned char VerifyCode; 
    unsigned char TxBuffer[COMNAVILCD_MAX_SEND_LEN];
    i=0;
    TxBuffer[i++]=0xff;
    ///////////////////////
    TxBuffer[i++]=c_ucNaviLcdTag[0];
    TxBuffer[i++]=c_ucNaviLcdTag[1];
    //////////////////////////////////
    len=datalen+4;
    ///////////////////////////////////
    TxBuffer[i++] = (len>>8);
    //////////////////////////////////
    TxBuffer[i++] = (len & 0xff);
    //////////////////////////////////
    TxBuffer[i++] = EQUIP_TYPE_NAVILCD_NO;
    
    //TxBuffer[i++] = NAVILCD_FACTORY_NO;
    TxBuffer[i++] = 0x00;
    ///////////////////////
    TxBuffer[i++] = (cmd>>8);
    //////////////////////////////////
    TxBuffer[i++] = (cmd & 0xff);
    /////////////////////////////////////////////
    len = i;
    for(i=0;i<datalen;i++)TxBuffer[len+i]=data[i];
    /////////////////////////////////////////////
    len  +=datalen;
    VerifyCode=Public_GetXorVerify(&TxBuffer[3],len-3);
    /////////////
    TxBuffer[len++]=VerifyCode;
    //////////////////////
    TxBuffer[len++] = c_ucNaviLcdTag[0];
    TxBuffer[len++] = c_ucNaviLcdTag[1];
    /////////////////////////////////
    TxBuffer[len++] = 0xff;
    ///////////////////////////
    NaviLcdCommSendData(TxBuffer,len);
    s_stNaviLcdComm.responseFlag = 0;
    ///////////////////
    //if(!IsNaviLcdCommBusy())//不忙,直接发送
    //{
    //    NaviLcdCommStartToSendData(TxBuffer,len);
    //}
}
/************************************************************
** 函数名称: NaviLcdGetStartPoint
** 功能描述: 查找数据开始标志的位置
             
** 入口参数:
** 出口参数:
************************************************************/ 
unsigned short NaviLcdGetStartPoint(unsigned char *buffer,unsigned short datalen)
{
    unsigned short i;
    for(i=0; i< datalen-2; i++)
    {
        if(*(buffer+i)==0x55 && *(buffer+i+1)==0xAA&& *(buffer+i+2) != 0xFF)
        {
            return i;
        }
    }
    ///////////////////
    return 0xffff;
}
/************************************************************
** 函数名称: NaviLcdParseResultForRecvLen
** 功能描述: 
             
** 入口参数:
** 出口参数:
************************************************************/ 
void NaviLcdParseResultForRecvLen(unsigned char Resultflag)
{
    unsigned short datalen;
    switch(Resultflag)
    {
        case NV_RECV_ERR_START://找不到开头
            s_stNaviLcdComm.recvlen = 0;
            s_stNaviLcdComm.disposelen = 0;
            break;
        case NV_RECV_ERR_TYPE:
        case NV_RECV_ERR_LEN:
        case NV_RECV_ERR_END:
            //////////找到新的开始标志////////////////////////////
            s_stNaviLcdComm.disposelen +=2;//跳过老的开始标志
            datalen=NaviLcdGetStartPoint(&s_stNaviLcdComm.recvBuff[s_stNaviLcdComm.disposelen],s_stNaviLcdComm.recvlen-s_stNaviLcdComm.disposelen);
            if(0xffff == datalen)//没有找到新的开始标志///////
            {
                s_stNaviLcdComm.recvlen = 0;
            }
            else
            {
                s_stNaviLcdComm.disposelen +=datalen;
                if(s_stNaviLcdComm.recvlen>s_stNaviLcdComm.disposelen)
                {
                    s_stNaviLcdComm.recvlen -= s_stNaviLcdComm.disposelen;
                }
                else
                {
                    s_stNaviLcdComm.recvlen = 0;
                }
            }
            break;
        case NV_RECV_ERR_VERIFY:
        case NV_RECV_OK:
            if(s_stNaviLcdComm.recvlen>s_stNaviLcdComm.disposelen)
            {
                s_stNaviLcdComm.recvlen -= s_stNaviLcdComm.disposelen;
            }
            else
            {
                s_stNaviLcdComm.recvlen =0;
            }
            break;
    }
    /////////////////////////////////
    /////////////////////////////////////////////////
    if(s_stNaviLcdComm.recvlen < COMNAVILCD_MIN_LEN)//达不到最小处理数据的长度
    {
        s_stNaviLcdComm.recvlen = 0;
    }
    else/////////////////////////////////
    {
        memmove((char *)&s_stNaviLcdComm.recvBuff[0], (char *)&s_stNaviLcdComm.recvBuff[s_stNaviLcdComm.disposelen], s_stNaviLcdComm.recvlen);
    }
}

/*************************************************************
** 函数名称: ProtocolParseForNaviLcd
** 功能描述: 协议解释
** 入口参数:  
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned char NaviLcdProtocolParse(unsigned char *ParseBuffer,unsigned short usDataLenght)
{
    unsigned char VerifyCode;
    unsigned short datalen,cmd;
    unsigned char *buffer;
    ///////////重新复位///////////////
    s_stNaviLcdComm.disposelen = 0;
    //////////////////////////
    datalen=NaviLcdGetStartPoint(ParseBuffer,usDataLenght);
    if(0xffff != datalen)//找到开始标志
    {
        s_stNaviLcdComm.disposelen = datalen;
        buffer = ParseBuffer+datalen;
    }
    ///////////////////////////////////////////////
    if(c_ucNaviLcdTag[0] != buffer[0]||c_ucNaviLcdTag[1] != buffer[1])  return NV_RECV_ERR_START;
    ////外设类型编号///不属于本机类型------out--------------
    if(EQUIP_TYPE_NAVILCD_NO != buffer[4])  return NV_RECV_ERR_TYPE;
    //////////////////////////////////////
    datalen=(buffer[2]<<8)+buffer[3];
    if(usDataLenght<datalen+7) return NV_RECV_ERR_LEN;//长度不对////
    ////////////////////////////////////
    //////////////结束标志不对////////////////////
    if(c_ucNaviLcdTag[0] != buffer[datalen+5]||c_ucNaviLcdTag[1] != buffer[datalen+6])  return NV_RECV_ERR_END;
    /////////命令不对//////////
    //cmd=(buffer[6]<<8)+buffer[7];
    //////////////////////////////
    //if(s_stNaviLcdAttrib.recvCmd != cmd)return 0;//主动
    //////////////////////////
    /////////////////////////////////////////////////////////////
    VerifyCode=Public_GetXorVerify(&buffer[2],datalen+2);
    /////////////////////////////////////////////////////////////
    //////////////////////////////////////
    s_stNaviLcdComm.disposelen += datalen+7;
    ///////////////////////////////////
    s_stNaviLcdComm.contrecvtimeoutCnt = 0;
    s_stNaviLcdComm.conttryCnt =0;
    Io_WriteAlarmBit(ALARM_BIT_LCD_FAULT,RESET);
    /////////////////////////////////////////
    if(VerifyCode==buffer[datalen+4])//校验码
    {
        s_stNaviLcdAttrib.RecvAnswerFlag = 1;
        ///////////////////////////
        cmd=(buffer[6]<<8)+buffer[7];
        s_stNaviLcdAttrib.recvCmd =cmd;
        s_stNaviLcdAttrib.recvDatalen = datalen-4;
        /////////////////////////////////
        s_stNaviLcdAttrib.onlinestaus = 1;
        ///////////把数据区保存下来/////////////////////
        memcpy((char *)&s_ucNaviLcdParseBuffer[0], (char *)&buffer[NAVILCD_USER_DATA_START], s_stNaviLcdAttrib.recvDatalen);
        LZM_PublicSetOnceTimer(&s_stNVTimer[NV_TIMER_TASK],LZM_AT_ONCE,NaviLcd_DisposeRecvDataResponse);
        ///////////////接收正确,清掉接收超时/////////////////////////////
        LZM_PublicKillTimer(&s_stNVTimer[NV_TIMER_BUSY_TIMEOUT]);  
        s_stNaviLcdComm.sendBusy = 0;
        /////////////////////////////////////////
        return NV_RECV_OK;
    }    
    return NV_RECV_ERR_VERIFY;
}
/************************************************************
** 函数名称: NaviLcd_DisposeCommProtocolParse
** 功能描述: 定时处理接收缓存里的串口数据
             
** 入口参数:
** 出口参数:
************************************************************/ 
void NaviLcd_DisposeCommProtocolParse(unsigned char * pInBuf, unsigned short InLength)
{
    //在添加外部共用串口的相关解析函数
    #ifndef DEBUG_TEST_NAVILCD
    ICCard_ProtocolParse(pInBuf,InLength);
    #endif
    ////////////////////
    OilWear_UartDataProtocolParse(pInBuf,  InLength);
    /////////////////
    JointechOiLCost_OilProtocolCheck(pInBuf, InLength);
    //////////////////
    Temperature_ComRec(pInBuf, InLength);
    ///////////////////
    Can_ErQi_Parse(pInBuf, InLength);
}
/************************************************************
** 函数名称: NaviLcdAutoToDisposeRecvData
** 功能描述: 定时处理接收缓存里的串口数据
             
** 入口参数:
** 出口参数:
************************************************************/ 
void NaviLcdAutoToDisposeRecvData(void)
{
    unsigned char flag;
    unsigned short datalen;
    datalen=COM2_ReadBuff(&s_stNaviLcdComm.recvBuff[s_stNaviLcdComm.recvlen],COMNAVILCD_MAX_REC_LEN-s_stNaviLcdComm.recvlen);
    ///////////////////////////////
    s_stNaviLcdComm.recvlen += datalen;
    ///////////////////////////////////
    if(0 == datalen)////处理的数据长度为0//////////    
    {
        if(s_stNaviLcdComm.recvlen)
        {
            /////////////////////////////////
            s_stNaviLcdComm.recvfreeCnt++;
            ////////////////////////////////
            if(s_stNaviLcdComm.recvfreeCnt >1)//空闲两次调度
            {
                s_stNaviLcdComm.recvfreeCnt = 0;//
                ///////////////////////////////////////
                NaviLcd_DisposeCommProtocolParse(s_stNaviLcdComm.recvBuff,s_stNaviLcdComm.recvlen);                
                ///////////////////////
                if(s_stNaviLcdComm.recvlen < COMNAVILCD_MIN_LEN)
                {
                    s_stNaviLcdComm.recvlen = 0;
                }
                else
                {
                    flag=NaviLcdProtocolParse(s_stNaviLcdComm.recvBuff,s_stNaviLcdComm.recvlen);
                    NaviLcdParseResultForRecvLen(flag);
                }
            }
        }
    }
    else//////////////////
    if(s_stNaviLcdComm.recvlen+10>COMNAVILCD_MAX_REC_LEN)//缓存已满的时候
    {
        ///////////////////////////////////////
        s_stNaviLcdComm.recvfreeCnt = 0;
        flag=NaviLcdProtocolParse(s_stNaviLcdComm.recvBuff,s_stNaviLcdComm.recvlen);
        ///////////////////////////////////////
        NaviLcd_DisposeCommProtocolParse(s_stNaviLcdComm.recvBuff,s_stNaviLcdComm.recvlen);        
        ///////////////////////
        NaviLcdParseResultForRecvLen(flag);
    }
    else//////////////////
    {
        s_stNaviLcdComm.recvfreeCnt = 0;
    }
}
/*************************************************************
** 函数名称: NaviLcd_GetCurOnlineStatus
** 功能描述: 导航屏在不在线 
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char NaviLcd_GetCurOnlineStatus(void)
{
    return (1==s_stNaviLcdAttrib.onlinestaus)?1:0;
}
/*************************************************************
** 函数名称: NaviLcdSendCmdQueryStatus
** 功能描述:  查询状态指令
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcdSendCmdQueryStatus(void)
{
    unsigned char data[20];    
    Public_ConvertNowTimeToBCD(data);
    PactSendCmdForNaviLcd(NAVILCD_CMD_QUERY,data,7);    
    //PactSendCmdForNaviLcdEx(NAVILCD_CMD_QUERY,data,7);
}

/*************************************************************
** 函数名称: NaviLcdSendCmdResetLcd
** 功能描述: 系统复位
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcdSendCmdResetLcd(void)
{
    unsigned char data[1];
    PactSendCmdForNaviLcd(NAVILCD_CMD_RESET,data,0);
}

/*************************************************************
** 函数名称: NaviLcdSendCmdSetBaudrate
** 功能描述: LCD通讯波特率设置
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcdSendCmdSetBaudrate(unsigned char baudrate)
{
    unsigned char data[2];
    data[0] = baudrate;
    PactSendCmdForNaviLcd(NAVILCD_CMD_SET_BAUDRATE,data,1);
}

/*************************************************************
** 函数名称: NaviLcdSendCmdDislpay
** 功能描述: LCD屏的开启/关闭
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcdSendCmdDislpay(unsigned char OnOff)
{
    unsigned char data[2];
    data[0] = OnOff;
    PactSendCmdForNaviLcd(NAVILCD_CMD_DISPLAY,data,1);
}

/*************************************************************
** 函数名称: NaviLcdSendCmdCaliTime
** 功能描述: LCD校时指令
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcdSendCmdCaliTime(void)
{
    unsigned char data[20];    
    Public_ConvertNowTimeToBCD(data);
    PactSendCmdForNaviLcd(NAVILCD_CMD_CALI_TIME,data,7);
}

/*************************************************************
** 函数名称: NaviLcdSendCmdSetLcdParam
** 功能描述: LCD设备参数设置指令
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcdSendCmdSetLcdParam(unsigned char *data,unsigned short len)
{
    PactSendCmdForNaviLcd(NAVILCD_CMD_SET_LCD_PARAM,data,len);
}

/*************************************************************
** 函数名称: NaviLcdSendCmdHeartBeat
** 功能描述: 心跳命令
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcdSendCmdHeartBeat(void)
{
    unsigned char buffer[30];
    unsigned char data[20];  
    Public_ConvertNowTimeToBCD(data);
    
    Public_GetCurBasicPositionInfo(buffer);
    memcpy(&data[7],&buffer[8],8); //经、纬度  
    PactSendCmdForNaviLcdEx(NAVILCD_CMD_HEART_BEAT,data,15);
}

/*************************************************************
** 函数名称: NaviLcdSendCmdGeneralAck
** 功能描述: 终端发给Lcd的通用应答
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcdSendCmdGeneralAck(unsigned short cmd,unsigned char result)
{
    PactSendCmdForNaviLcdEx(cmd,&result,1);
    s_stNaviLcdComm.responseFlag = 0;
}
/*************************************************************
** 函数名称: NaviLcdSendCmdQueryParameter
** 功能描述: 参数查询命令
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcdSendCmdQueryParameter(unsigned char *buffer,unsigned short datalen)
{
    PactSendCmdForNaviLcdEx(NAVILCD_CMD_TERMINAL_QUERY_PARAM,buffer,datalen);
}

/*************************************************************
** 函数名称: NaviLcdSendCmdModifyPassWord
** 功能描述: 修改密码
** 入口参数: buffer:密码的首地址,且长度4位的长整型数,高位在前
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcdSendCmdModifyPassWord(unsigned char *buffer)
{
    PactSendCmdForNaviLcd(NAVILCD_CMD_MODIFY_PASSWORD,buffer,4);
}
/*************************************************************
** 函数名称: NaviLcdSendCmdNewCallPhone
** 功能描述: 新来电命令
** 入口参数: 电话号码
** 出口参数: 无
** 返回参数: 等待接听标志:0:直接拒接来电;1:正在等待用户接听来电
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char NaviLcdSendCmdNewCallPhone(unsigned char *phone)
{
    unsigned char name[24];
    unsigned char data[60];
    unsigned char len,len1,flag;
    //////////////////////////////
    len=strlen((char *)phone);
    data[0] = len;
    memcpy(&data[1],phone,len);
    len++;
    ////////////////////////
    flag=Public_CheckPhoneInformation(phone,name);//0：没有该电话号码的资料；1: 呼入；2: 呼出；3：呼入/呼出
    if(2 == flag||0==flag)//电话簿存在号码,但属于呼出
    {
        communication_CallPhone("13800000000",PHONE_END);
        //直接拒接电话/////////////
        return 0;
    }
    else//电话簿存在号码,属于可呼入
    if(flag)
    {        
        len1 = strlen((char *)name);
        data[len++] = len1;
        memcpy(&data[len],name,len1);
        len += len1;
    }
    /////////////////////////////
    PactSendCmdForNaviLcd(NAVILCD_CMD_NEW_CALL_PHONE,data,len);
    s_stNaviLcdAttrib.newphone =1;
    return 1;
}

/*************************************************************
** 函数名称: NaviLcdSendCmdQueryPhoneBooks
** 功能描述: 查询电话簿
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcdSendCmdQueryPhoneBooks(unsigned char *buffer,unsigned short datalen)
{
    PactSendCmdForNaviLcdEx(NAVILCD_CMD_QUERY_PHONE_BOOK,buffer,datalen);
}

/*************************************************************
** 函数名称: NaviLcdSendCmdNewText
** 功能描述: 文本显示
** 入口参数: display:显示标志:0:显示;1: 不显示; voice语音播读标志:0: 播读;1: 不播读;save存储标志:0: 存储1: 不存储
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcdSendCmdNewText(unsigned char display,unsigned char voice,unsigned char save,unsigned char *buffer,unsigned short datalen)
{
    unsigned char data[1024];
    unsigned short len;
    /////////////////////////////
    len = 0;
    data[len++] = display;
    data[len++] = voice;
    data[len++] = save;
    memcpy((char *)&data[len],buffer,datalen);
    len += datalen;
    PactSendCmdForNaviLcd(NAVILCD_CMD_TEXT_DISPLAY,data,len);
}

/*************************************************************
** 函数名称: NaviLcdShowPlatformText
** 功能描述: 文本/调度信息下发
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcdShowPlatformText(unsigned char *buffer,unsigned short datalen)
{
    if(NaviLcd_GetCurOnlineStatus())
    NaviLcdSendCmdNewText(0,1,0,buffer,datalen);
}
/*************************************************************
** 函数名称: NaviLcdShowTempText
** 功能描述: 显示临时信息
** 入口参数: 需要显示的临时信息
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcdShowTempText(unsigned char *string)
{
    NaviLcdSendCmdNewText(0,1,1,string,strlen((char*)string));
}
/*************************************************************
** 函数名称: NaviLcdSendCmdQuestionDisplayEx
** 功能描述: 提问下发(标准出租车协议,带问题ID)
** 入口参数: voice 是否播放,buffer源数据首地址,datalen源数据长度
** 出口参数: 
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcdSendCmdQuestionDisplayEx(unsigned char voice,unsigned char *buffer,unsigned short datalen)
{
    unsigned char data[1024];
    unsigned short len;
    /////////////////////////////
    len = 0;
    data[len++] = voice;
    memcpy((char *)&data[len],buffer,datalen);
    len += datalen;
    PactSendCmdForNaviLcd(NAVILCD_CMD_QUESTION_DISPLAY,data,len);
}

/*************************************************************
** 函数名称: NaviLcdSendCmdQuestionDisplay
** 功能描述: 提问下发(行标协议,不带问题ID)
** 入口参数: voice 是否播放,buffer源数据首地址,datalen源数据长度
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcdSendCmdQuestionDisplay(unsigned char voice,unsigned char *buffer,unsigned short datalen)
{
    static unsigned long sulID=0; 
    unsigned char data[1024];
    unsigned short len,templen,tmplen1;
    ///////////////////
    /////////////////////////////
    sulID++;
    len = 0;    
    data[len++] = voice;
    data[len++] = 0;
    Public_ConvertLongToBuffer(sulID, &data[len]);
    len +=4;
    templen = 0;
    tmplen1 =buffer[templen++];
    memcpy((char *)&data[len],&buffer[templen],tmplen1);
    templen +=tmplen1;
    len +=tmplen1;
    data[len++]='\0';
    while(templen<datalen)
    {
        //答案ID
        data[len++]=buffer[templen++];
        //答案长度
        tmplen1=(buffer[templen++]<<8);
        tmplen1 +=buffer[templen++];
        ////答案内容////////////////////
        memcpy((char *)&data[len],&buffer[templen],tmplen1);
        templen += tmplen1;
        len  += tmplen1;
        data[len++]='\0';
    }
    /////////////////////////
    PactSendCmdForNaviLcd(NAVILCD_CMD_QUESTION_DISPLAY,data,len);
}
/*************************************************************
** 函数名称: NaviLcdSendCmdQueryFixText
** 功能描述: 查询固定文本
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcdSendCmdQueryFixText(unsigned char *buffer,unsigned short datalen)
{
    PactSendCmdForNaviLcdEx(NAVILCD_CMD_QUERY_FIX_TEXT,buffer,datalen);
}

/*************************************************************
** 函数名称: NaviLcdSendCmdInfoService
** 功能描述: 信息服务命令
** 入口参数: voice 是否播放,buffer源数据首地址,datalen源数据长度
** 出口参数: 
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcdSendCmdInfoService(unsigned char voice,unsigned char *buffer,unsigned short datalen)
{
    unsigned char data[1024];
    unsigned short len;
    /////////////////////////////    
    len = 0;
    data[len++] = 0;    //保留
    data[len++] = voice;//播放标志
    memcpy((char *)&data[len],buffer,datalen);
    len += datalen;
    PactSendCmdForNaviLcd(NAVILCD_CMD_INFO_SERVICE,data,len);
}
/*************************************************************
** 函数名称: NaviLcdSendCmdCurrentDriverInfo
** 功能描述: 当前驾驶员身份信息命令
** 入口参数: loginFlag登签标志:(0:签到,1:签退); IDNumber证件号码;driverName驾驶员姓名
** 出口参数: 
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcdSendCmdCurrentDriverInfo(unsigned char loginFlag,char *IDNumber,char *driverName)
{
    unsigned char data[60];
    unsigned short len,temp;
    /////////////////////////////
    len = 0;
    data[len++] = 0;    //保留
    data[len++] = loginFlag;//登签标志
    if(0==loginFlag)
    {
        temp=strlen(IDNumber);
        data[len++] = temp;//
        memcpy(&data[len],IDNumber,temp);
        len +=temp;
        temp=strlen(driverName);
        data[len++] = temp;//
        if(temp)//
        {
            memcpy(&data[len],driverName,temp);
            len +=temp;
        }
    }
    PactSendCmdForNaviLcd(NAVILCD_CMD_CURRENT_DRIVER_INFO,data,len);
}

/*************************************************************
** 函数名称: NaviLcdSendCmdNewCallTaxi
** 功能描述: 新电召信息命令
** 入口参数: voice 是否播放 0:播放,1:不播放,buffer源数据首地址,datalen源数据长度
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcdSendCmdNewCallTaxi(unsigned char voice,unsigned char *buffer,unsigned short datalen)
{
    unsigned char data[1024];
    unsigned short len;
    /////////////////////////////
    if(!NaviLcd_GetCurOnlineStatus())return;
    len = 0;
    data[len++] = voice;
    memcpy((char *)&data[len],buffer,datalen);
    len += datalen;
    PactSendCmdForNaviLcd(NAVILCD_CMD_CALL_TAXI_NEW,data,len);
    //////////////////////////////////////////////
}

/*************************************************************
** 函数名称: NaviLcdSendCmdCallTaxiDetail
** 功能描述: 电召详细信息命令
** 入口参数: voice 是否播放,buffer源数据首地址,datalen源数据长度
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcdSendCmdCallTaxiDetail(unsigned char voice,unsigned char *buffer,unsigned short datalen)
{
    unsigned char data[1024];
    unsigned short len;
    /////////////////////////////
    if(!NaviLcd_GetCurOnlineStatus())return;
    len = 0;
    data[len++] = voice;
    memcpy((char *)&data[len],buffer,datalen);
    len += datalen;
    PactSendCmdForNaviLcd(NAVILCD_CMD_CALL_TAXI_DETAIL,data,len);
}

/*************************************************************
** 函数名称: NaviLcdSendCmdCallTaxiPlatformCancel
** 功能描述: 平台取消电召命令
** 入口参数: buffer源数据首地址,datalen源数据长度
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcdSendCmdCallTaxiPlatformCancel(unsigned char *buffer,unsigned short datalen)
{
    PactSendCmdForNaviLcd(NAVILCD_CMD_CALL_TAXI_CANCEL_PLATFORM,buffer,datalen);
}
/************************************************************
** 函数名称: NaviLcdParamInit
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
************************************************************/ 
void NaviLcdCommParamInit(void)
{
    s_stNVKey.datalen = 0; 
    memset(&s_stNaviLcdComm,0,13);
#if (ICCARD_JTD == ICCARD_SEL)
    {
    	COM2_Init(115200);
		}
#else
		{
			COM2_Init(9600);
		}
#endif
}

/*************************************************************
** 函数名称: NaviLcd_DisposeCallTaxiVie
** 功能描述: 
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char NaviLcd_DisposeCallTaxiVie(void)
{
  /*//joneming 2013.1.15
    ProtocolACK rev_ack;  
    ///////////////////////////
    if(s_stNaviLcdAttrib.recvDatalen<5)return 2;
    ////////////////////////
    if(s_ucNaviLcdParseBuffer[4]==0)//抢标
    {
        if(TexiService_CheckCurrentVieServiceID())
        {
            rev_ack=TexiService_SendTerminalDriverGlomLender();
            if(ACK_OK==rev_ack)
            {
                Public_PlayTTSVoiceStr("抢标已发送");
                return 0;
            }
            else
            {
              Public_PlayTTSVoiceStr("发送失败");
            }
        }
        else
        {
          Public_PlayTTSVoiceStr("业务已失效");
        }
        return 2;
    }
    else
    {
        SaveTexiStatusFlagForCancel();
        return 0;
    }  
*/
return 0;//joneming 2013.1.15  
}
/*************************************************************
** 函数名称: NaviLcd_DisposeCallTaxiFinish
** 功能描述: 
** 入口参数:  
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char NaviLcd_DisposeCallTaxiFinish(void)
{
  /*
    ProtocolACK rev_ack;
    if(s_stNaviLcdAttrib.recvDatalen<5)return 2;
    if(TexiService_CheckCurrentServiceID())
    {
        if(GetTexiServiceReportFlag())
        {
            Public_PlayTTSVoiceStr("业务已完成");
            return 2; 
        }
        rev_ack=SendTexiServiceFinishRepor(s_ucNaviLcdParseBuffer[4]);
        if(ACK_OK==rev_ack)
        {
            Public_PlayTTSVoiceStr("完成订单已发送");
            return 0;
        }
        else
        {
          Public_PlayTTSVoiceStr("发送失败");
        }
    }
    else
    {
        Public_PlayTTSVoiceStr("业务已过期");
    }
  */
    return 2;    
}
/*************************************************************
** 函数名称: NaviLcd_DisposeTerminalCancelCallTaxi
** 功能描述: 
** 入口参数:  
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char NaviLcd_DisposeTerminalCancelCallTaxi(void)
{
  /*
    ProtocolACK rev_ack;
    if(s_stNaviLcdAttrib.recvDatalen<5)return 2;
    if(TexiService_CheckCurrentServiceID())
    {
        if(GetTexiServiceReportFlag())
        {
            Public_PlayTTSVoiceStr("业务已完成");
            return 2; 
        }
        rev_ack=SendTexiServiceTerminalCancel(s_ucNaviLcdParseBuffer[4]);
        if(ACK_OK==rev_ack)
        {
            Public_PlayTTSVoiceStr("取消订单已发送");
            return 0;
        }
        else
        {
          Public_PlayTTSVoiceStr("发送失败");
        }
    }
    else
    {
        Public_PlayTTSVoiceStr("业务已过期");
    }
  */
    return 2;    
}
/*************************************************************
** 函数名称: NaviLcd_DriverLogin
** 功能描述: 
** 入口参数:  
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char NaviLcd_DriverLogin(unsigned char *drivercode,unsigned char *password)
{
    unsigned char recordnum,flag;
    flag= CheckDriverInfoForLogin(drivercode,password,&recordnum);
    if(flag)return flag;
    if(NaviLcdDriverLoginFlag())
    {
        if(s_stDriverLog.recordNumber==recordnum)
        {
            Public_PlayTTSVoiceStr("该驾驶员已登录");
            return 3;
        }
    }
    //写驾驶员代码到eeprom
    SaveCurrentDriverParameter(recordnum);
    //清除疲劳驾驶信息
    TiredDrive_Login();
    //清除驾驶员里程数
    NaviLcdSaveDriverLogin(recordnum);
    return 0;
}
/*************************************************************
** 函数名称: NaviLcd_SaveLogout
** 功能描述: :
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcd_LogoutSaveParam(void)
{	
    //unsigned char driver[7] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}; 
    ///////////////////
    //设置当前驾驶员代码为0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
    //EepromPram_WritePram(E2_CURRENT_DRIVER_ID, driver, E2_CURRENT_DRIVER_ID_LEN);
}
/*************************************************************
** 函数名称: NaviLcd_DriverLogout
** 功能描述: 
** 入口参数:  
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char NaviLcd_DriverLogout(void)
{   
    NaviLcd_LogoutSaveParam();
    ////////////////////
    //驾驶员签退时调用
    TiredDrive_Logout();
    //登录标志清
    NaviLcdSaveDriverLogout();
    return 0;
}
/*************************************************************
** 函数名称: NaviLcd_DisposeDriverSign
** 功能描述: 
** 入口参数:  
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcdSendDriverSignResult(unsigned char *data,unsigned char datalen)
{
    PactSendCmdForNaviLcdEx(NAVILCD_CMD_DRIVER_SIGN,data,datalen);
    s_stNaviLcdComm.responseFlag =0;
}
/*************************************************************
** 函数名称: NaviLcd_DisposeDriverSign
** 功能描述: 
** 入口参数:  
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcd_DisposeDriverSign(void)
{
    unsigned char data[40];
    unsigned char type,len;
    type=s_ucNaviLcdParseBuffer[0];
    if(0 == type)//签到
    {
        len=NaviLcd_DriverLogin(&s_ucNaviLcdParseBuffer[1],&s_ucNaviLcdParseBuffer[7]);
        if(len==0)
        {
            data[0] = type;
            data[1] = 0;
            len=GetDriverInfo(s_stDriverLog.recordNumber,&data[2],2);  
            len +=2;
            NaviLcdSendDriverSignResult(data,len);
        }
        else
        {
            data[0] = type;
            data[1] = len;
            NaviLcdSendDriverSignResult(data,2);
        }
    }
    else
    if(1 == type)//签退
    {
        /////////////////////////
        if(NaviLcdDriverLoginFlag()==0)
        {
            Public_PlayTTSVoiceStr("未登录或已签退");
            data[0] = type;
            data[1] = 1;
            NaviLcdSendDriverSignResult(data,2);
        }
        else
        {
            data[0] = type;
            data[1] = 2;
            NaviLcdSendDriverSignResult(data,2);
        }
    }  
}

/*************************************************************
** 函数名称: NaviLcd_DisposeDriverSign
** 功能描述: 
** 入口参数:  
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcd_DisposeQueryDriverInfo(void)
{
    unsigned char type;
    unsigned char data[1024];
    unsigned short len;
    type = s_ucNaviLcdParseBuffer[0];
    if(0 == type)//签到
    {
        s_stDriverLog.cmdFlag =NV_CMD_FLAG_LOGIN;
        NaviLcd_OpenBrushCard();
        len=GetDriverInfoAll(data,2);    
    }
    else
    if(1 == type)//签退
    {
        s_stDriverLog.cmdFlag =NV_CMD_FLAG_LOGOUT;
        NaviLcd_OpenBrushCard();
        len = 1;
        data[0] =0;
    }
    else
    if(2 == type)//添加/修改驾驶员
    {
        s_stDriverLog.cmdFlag =NV_CMD_FLAG_ADD;
        len=GetDriverInfoAll(data,2);
    }
    ////////////////////
    if(len==0)//无记录时
    {
        data[len++]=0;
    }
    /////////////////////////
    PactSendCmdForNaviLcdEx(NAVILCD_CMD_DRIVER_QUERY_ALL,data,len);
    s_stNaviLcdComm.responseFlag =0;
}
/*************************************************************
** 函数名称: NaviLcd_DisposeDriverInfoAddInfo
** 功能描述: 
** 入口参数:  
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcdSendDriverInfoAdd(unsigned char result,unsigned char time)
{
    unsigned char buffer[10];
    buffer[0] = result;
    if(result == 0x02)
    {
        buffer[1] =time;
    }
    else
    {
         buffer[1] =0;
    }
    PactSendCmdForNaviLcd(NAVILCD_CMD_DRIVER_ADD,buffer,2);
}

/*************************************************************
** 函数名称: NaviLcd_DisposeDriverInfoAddInfo
** 功能描述: 
** 入口参数:  
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcdSendDriverInfoAddOrModifyInfoResult(unsigned char *data,unsigned char datalen)
{
    PactSendCmdForNaviLcd(NAVILCD_CMD_DRIVER_ADD_RESULT,data,datalen);
}
/*************************************************************
** 函数名称: NaviLcd_DisposeDriverInfoAddInfo
** 功能描述: 
** 入口参数:  
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcd_DisposeDriverInfoAddInfo(void)
{
    unsigned char buffer[120];
    unsigned char datalen,temp;
    temp =s_stDriverLog.result;
    if(temp)
    {
        buffer[0] = 1;//失败
        buffer[1] = s_stDriverLog.resutltindex;//失败原因
        NaviLcdSendDriverInfoAddOrModifyInfoResult(buffer,2);
    }
    else
    {      
        buffer[0] = 0;//成功
        datalen=GetDriverInfo(s_stDriverLog.resutltindex,&buffer[1],2);
        datalen++;
        NaviLcdSendDriverInfoAddOrModifyInfoResult(buffer,datalen);
    } 
    ////////////////////////
    s_stDriverLog.recordindex = 0xAA;
}
/*************************************************************
** 函数名称: NaviLcd_DisposeDriverInfoAddInfo
** 功能描述: 
** 入口参数:  
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcd_DisposeDriverInfoAdd(void)
{
    NaviLcd_OpenBrushCard();
    s_stDriverLog.cmdFlag = NV_CMD_FLAG_ADD;
    s_stDriverLog.recordindex = s_ucNaviLcdParseBuffer[0];
    memcpy(s_stDriverLog.drivercode,&s_ucNaviLcdParseBuffer[1],6);
    memcpy(s_stDriverLog.password,&s_ucNaviLcdParseBuffer[7],6);
    NaviLcdSendDriverInfoAdd(2,30);
    s_stDriverLog.result =1;//
    s_stDriverLog.resutltindex =0;
    LZM_PublicSetOnceTimer(&s_stNVTimer[NV_TIMER_DRIVER_ADD],PUBLICSECS(30),NaviLcd_DisposeDriverInfoAddInfo); 
    s_stNaviLcdComm.responseFlag = 0;
}
/*************************************************************
** 函数名称: NaviLcd_GetDriverSpeedRecord
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned short NaviLcd_GetDriverSpeedRecord(unsigned char *buffer)
{
    strcpy((char *)buffer,"无记录");
    return strlen((char *)buffer);
}
/*************************************************************
** 函数名称: NaviLcd_GetTiredDriveRecord
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned short NaviLcd_GetTiredDriveRecord(unsigned char *buffer)
{
    unsigned char data[100];
    unsigned short len,i,maxcount,index;
    unsigned long TimeCount;
    TIME_T	CurrentTime,stTime;
    
    //读当前时间
    RTC_GetCurTime(&CurrentTime);
    CurrentTime.hour = 0;
    CurrentTime.min = 0;
    CurrentTime.sec = 0;
    TimeCount = ConverseGmtime(&CurrentTime);
    if(TimeCount>DAY_SECOND)
    {
       TimeCount -= DAY_SECOND;
    }
    else
    {
      TimeCount =0;
    }
    Gmtime(&CurrentTime, TimeCount);
    //读当前驾驶员信息
    len = TiredDrive_Read2daysTiredRecord(RadioShareBuffer,CurrentTime);//modify by joneming 20130312
    ////////////////////////
    //显示超时驾驶记录
    if(0 == len)//没有一条记录,显示"无记录"
    {
        strcpy((char *)buffer,"无记录");
    }
    else
    {
        ///////////////////
        maxcount = len/ONE_TIRED_DRIVER_DATA_LEN;
        len=0;
        for(i = 0; i< maxcount; i++)
        {
            index =ONE_TIRED_DRIVER_DATA_LEN*i;	
            sprintf((char *)&buffer[len],"记录%d:",i+1);
            len=strlen((char *)buffer);
            strcpy((char *)&buffer[len]," 驾驶证号码:");
            len=strlen((char *)buffer);
            memcpy(&buffer[len],&RadioShareBuffer[index],18);
            len+=18;
            index += 18;
            memcpy(data,&RadioShareBuffer[index],12);
            Public_ConvertBCDToTime(&stTime,data);            
            sprintf((char *)&buffer[len]," 开始时间:20%02d-%02d-%02d %02d:%02d:%02d ",stTime.year,stTime.month,stTime.day,stTime.hour,stTime.min,stTime.sec);
            Public_ConvertBCDToTime(&stTime,&data[6]);
            len = strlen((char *)buffer);
            sprintf((char *)&buffer[len],"结束时间:20%02d-%02d-%02d %02d:%02d:%02d ",stTime.year,stTime.month,stTime.day,stTime.hour,stTime.min,stTime.sec);
            len = strlen((char *)buffer);
        }
    }
    len = strlen((char *)buffer);
    return len;    
}

/*************************************************************
** 函数名称: NaviLcd_StartPrintStopRecord
** 功能描述: 
** 入口参数:  
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcd_StartPrintStopRecord(void)
{
    LZM_StartNewTask(TaskMenuPrint);//打印
    //SetTimerTask(TIME_PRINT, SYSTICK_0p1SECOND);
}
/*************************************************************
** 函数名称: NaviLcd_GetDriverInfo
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned short NaviLcd_GetDriverInfo(unsigned char *buffer)
{
    unsigned short len;
    if(0 == NaviLcdDriverLoginFlag())//没登陆
    {
        strcpy((char *)buffer,"未登录!");
        len = strlen((char *)buffer);
        return len;
    }
    ////////////////////////////
    len=ICCard_GetExternalDriverInfo(buffer);
    //////////////////////////////
    return len;    
}
/*************************************************************
** 函数名称: NaviLcd_DisposeDriverSign
** 功能描述: 
** 入口参数:  
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcd_DisposeBrushCardForDriverInfo(unsigned char type ,unsigned char *data,unsigned char datalen)
{
    if(0 == type)//签到
    {
        s_stDriverLog.cmdFlag =NV_CMD_FLAG_LOGIN;  
    }
    else
    if(1 == type)//签退
    {
        s_stDriverLog.cmdFlag =NV_CMD_FLAG_LOGOUT;
    }
    else
    if(2 == type)//添加/修改驾驶员
    {
        s_stDriverLog.cmdFlag =NV_CMD_FLAG_ADD;
        LZM_PublicKillTimer(&s_stNVTimer[NV_TIMER_DRIVER_ADD]);        
    }
    else
    if(3 == type)   //业务上报 
    {
        s_stDriverLog.cmdFlag =NV_CMD_FLAG_BUSINESS;
    }
    NaviLcd_DisposeBrushCardDataForDriverInfo(data,datalen);
}
/*************************************************************
** 函数名称: NaviLcd_DisposeBrushCard
** 功能描述: 
** 入口参数:  
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcd_DisposeBrushCard(void)
{
    unsigned char type;
    unsigned char flag;
    type = s_ucNaviLcdParseBuffer[0];
    flag = s_ucNaviLcdParseBuffer[1];    
    if(flag==0)
    {
        LZM_PublicKillTimer(&s_stNVTimer[NV_TIMER_DRIVER_ADD]);
        NaviLcd_DisposeBrushCardForDriverInfo(type,&s_ucNaviLcdParseBuffer[2],s_stNaviLcdAttrib.recvDatalen-2);
    }    
}
/*************************************************************
** 函数名称: NaviLcdSendDataToWriteICcard
** 功能描述: 写卡指令
** 入口参数:  flag保留,type卡类型,buffer卡内容首地址,datalen卡内容长度
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcdSendDataToWriteICcard(unsigned char flag,unsigned char type,unsigned char *buffer,unsigned char datalen)
{
    unsigned char data[256];
    data[0] =flag;//保留
    data[1] =type;//卡类型
    memcpy(&data[2],buffer,datalen);
    PactSendCmdForNaviLcd(NAVILCD_CMD_WRITE_IC_CARD,data,datalen+2);
}
/*************************************************************
** 函数名称: NaviLcdSendCmdReadICcardData
** 功能描述: 读IC卡内容
** 入口参数:  
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcdSendCmdReadICcardData(void)
{
    unsigned char data[1];
    data[0] =1;//保留
    PactSendCmdForNaviLcd(NAVILCD_CMD_READ_IC_CARD,data,1);
}
/*************************************************************
** 函数名称: NaviLcdSendCmdReadICcardData
** 功能描述: 读IC卡内容
** 入口参数:  
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcd_DisposeReadICcardData(void)
{
    unsigned char flag;
    unsigned short datalen;
    unsigned char len;
    datalen = 0;
    flag = s_ucNaviLcdParseBuffer[0];//类型
    if(flag==0)
    {     
        len = s_ucNaviLcdParseBuffer[datalen+1];
        datalen++;//驾驶员姓名长度
        datalen += len; //驾驶员姓名 
        /////////////////
        datalen += 20;//驾驶员身份证编码
        datalen += 40;//从业资格证编码
        //////////////////
        len = s_ucNaviLcdParseBuffer[datalen+1];//
        datalen++;//发证机构长度
        datalen += len;//发证机构
        //memcpy(buffer,&s_ucNaviLcdParseBuffer[1],datalen);
    }
}
/*************************************************************
** 函数名称: NaviLcdSendWriteDataICcard
** 功能描述: 
** 入口参数:  
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcd_DisposeWriteICcardResult(unsigned char result)
{
    if(0 == result)
    {
        Public_PlayTTSVoiceStr("写卡成功");
    }
    else
    if(2 == result)
    {
        Public_PlayTTSVoiceStr("无卡，操作失败");
    }
    else
    {
        Public_PlayTTSVoiceStr("写卡失败");
    }
}
/*************************************************************
** 函数名称: NaviLcd_DisposeDriverRecord
** 功能描述: 
** 入口参数:  
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcd_DisposeDriverRecord(void)
{
    unsigned char buffer[512];
    unsigned short datalen;
    unsigned char type;
    type = s_ucNaviLcdParseBuffer[0];
    switch(type)
    {
        case DRIVER_SPEED_RECORD:
            datalen = NaviLcd_GetDriverSpeedRecord(buffer);
            break;
        case DRIVER_TIRED_RECORD:
            datalen = NaviLcd_GetTiredDriveRecord(buffer);
            break;
        case DRIVER_STOP_RECORD:
            ///////开启打印功能////////////
            datalen = 0;
            NaviLcd_StartPrintStopRecord();
            break;
        case DRIVER_INFO_RECORD:
            datalen = NaviLcd_GetDriverInfo(buffer);
            break;
    }
    PactSendCmdForNaviLcdEx(NAVILCD_CMD_DRIVER_RECORD,buffer,datalen);
}

/*************************************************************
** 函数名称: NaviLcd_DisposeDriverRecord
** 功能描述: 
** 入口参数:  
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char NaviLcd_DisposeDriverBusinessReport(void)
{
  /*
    unsigned char data[512];
    unsigned char datalen,type;
    ProtocolACK rev_ack;
    type = s_ucNaviLcdParseBuffer[0];
    datalen =0;
    if(type==0)
    {
        rev_ack = RadioProtocol_ElectricReceiptReport(data,datalen);
    }
    else
    if(type==1)
    {
        if(NaviLcdDriverLoginFlag())
        {
            datalen=GetDriverInfo(s_stDriverLog.recordNumber,data,0);
        }
        rev_ack = RadioProtocol_DriverInformationReport(data,datalen);
    }
    if(ACK_OK==rev_ack)return 0;
  */
    return 2; 
}
///////////////////////////////////////////////////////////
extern TIME_TASK MyTimerTask[];
extern u16 DelayAckCommand;
/*******************************************************************************
**  函数名称  : NaviLcd_DisposeTerminalCtrlQuickOpenAccount
**  函数功能  : 快速开户
**  输    入  : 无				
**  输    出  : 
**  全局变量  : 
**  调用函数  : 
**  中断资源  : 
**  备    注  :
*******************************************************************************/
void NaviLcd_DisposeTerminalCtrlQuickOpenAccount(void)
{
    Public_QuickOpenAccount();
}
/*******************************************************************************
**  函数名称  : NaviLcd_DisposeTerminalCtrlCarLoadStatus
**  函数功能  : 车辆载货状态 
**  输    入  : status: 0：空载、1：半载、2：满载
**  输    出  : 
**  全局变量  : 
**  调用函数  : SetEvTask(EV_REPORT);
**  中断资源  : 
**  备    注  :
*******************************************************************************/
void NaviLcd_DisposeTerminalCtrlCarLoadStatus(unsigned char status)
{
    Public_SetCarLoadStatus(status);
    Public_WriteDataToFRAM(FRAM_CAR_CARRY_STATUS_ADDR, &status, 1);
}
/*******************************************************************************
**  函数名称  : NaviLcd_DisposeTerminalControl
**  函数功能  : 
**  输    入  : 无				
**  输    出  : 
**  全局变量  : 
**  调用函数  : 
**  中断资源  : 
**  备    注  :
*******************************************************************************/
unsigned char NaviLcd_DisposeTerminalControl(void)
{
    unsigned char control;    
    ProtocolACK ACK_val;
    u8 channel = CHANNEL_DATA_1|CHANNEL_DATA_2;
    control=s_ucNaviLcdParseBuffer[0];
    ACK_val = ACK_OK;
    switch(control)
    {
        case TERMINAL_CTRL_REGISTER://终端注册
            ACK_val = RadioProtocol_TerminalRegister(channel);
            break;
        case TERMINAL_CTRL_CONFIRM_ID://终端鉴权
            ACK_val=RadioProtocol_TerminalQualify(CHANNEL_DATA_1);//只能一个一个通道发，因为通道不一样鉴权码也不一样      
            ACK_val=RadioProtocol_TerminalQualify(CHANNEL_DATA_2);
            break;
        case TERMINAL_CTRL_LOGOUT://终端注销
            ACK_val= RadioProtocol_TerminalLogout(channel);
            if(ACK_OK ==ACK_val)
            {
                s_stNaviLcdAttrib.logout =1;
                ClearTerminalAuthorizationFlag(0xff);
                if(MyTimerTask[TIME_DELAY_TRIG].TaskTimerState == DISABLE)
                {
                    DelayAckCommand = 0x0003;                    
                    //触发延时任务
                    SetTimerTask(TIME_DELAY_TRIG, 2*SYSTICK_0p1SECOND);
                    //擦除鉴权码
                    ClearTerminalAuthorizationCode(CHANNEL_DATA_1);
                    ClearTerminalAuthorizationCode(CHANNEL_DATA_2);
                }
            }
            break;
        case TERMINAL_CTRL_RESET://终端复位
            LZM_PublicSetOnceTimer(&s_stNVTimer[NV_TIMER_TEST],PUBLICSECS(2),NVIC_SystemReset);
            break;
        case TERMINAL_CTRL_FEATURE://开启特征系数校准
            if(!ADJ_getAdjFlg())setAdjustFunction(ENABLE);
            break;
        case TERMINAL_CTRL_DEFAULT_SET://恢复出厂参数设置
            EepromPram_DefaultSet();
            break;
        case TERMINAL_CTRL_QUICK_OPEN_ACCOUNT://快速开户
            NaviLcd_DisposeTerminalCtrlQuickOpenAccount();
            break;
        case TERMINAL_CTRL_CAR_LOAD_STATUS://车辆载货状态
            NaviLcd_DisposeTerminalCtrlCarLoadStatus(s_ucNaviLcdParseBuffer[1]);
            break;
    }
    if(ACK_OK == ACK_val)
    {
        return 0;
    }
    else
    {
        return 2;
    }
}

/*******************************************************************************
** 函数名称: NaviLcd_DisposeQueryStatusAlarm
** 函数功能:  
** 入口参数:  
** 出口参数: 无
** 返回参数:  
** 全局变量: 无
** 调用模块: 无
*******************************************************************************/
void NaviLcd_DisposeQueryStatusAlarm(void)
{ 
    unsigned char buf[30];
    unsigned long temp[3];
    if(1==s_ucNaviLcdParseBuffer[0])
    {
    temp[0]=Io_ReadAlarm();
    Public_ConvertLongToBuffer(temp[0],buf);     
    temp[1]=Io_ReadStatus();
    Public_ConvertLongToBuffer(temp[1],&buf[4]);
    temp[2]=Io_ReadExtCarStatus();
    Public_ConvertLongToBuffer(temp[2],&buf[4]);
    }
    else
    {
        temp[0]=Io_ReadAlarm();
        Public_ConvertLongToBuffer(temp[0],buf);     
        temp[1]=Io_ReadStatus();        
        temp[1] = (temp[1] & 0x1fff);
        if(Io_ReadExtCarStatusBit(CAR_STATUS_BIT_NEAR_LIGHT))
        {
            PUBLIC_SETBIT(temp[1], 18);
        }
        if(Io_ReadExtCarStatusBit(CAR_STATUS_BIT_BRAKE))
        {
            PUBLIC_SETBIT(temp[1], 20);
        }
        if(Io_ReadExtCarStatusBit(CAR_STATUS_BIT_HORN))
        {
            PUBLIC_SETBIT(temp[1], 21);
        }        
        if(Io_ReadStatusBit(STATUS_BIT_DOOR2))
        {
            PUBLIC_SETBIT(temp[1], 25);
        }
        if(Io_ReadExtCarStatusBit(CAR_STATUS_BIT_FAR_LIGHT))
        {
            PUBLIC_SETBIT(temp[1], 26);
        }
        if(Io_ReadExtCarStatusBit(CAR_STATUS_BIT_LEFT_LIGHT))
        {
            PUBLIC_SETBIT(temp[1], 27);
        }
        if(Io_ReadExtCarStatusBit(CAR_STATUS_BIT_RIGHT_LIGHT))
        {
            PUBLIC_SETBIT(temp[1], 28);
        }
        if(Io_ReadStatusBit(STATUS_BIT_DOOR1))
        {
            PUBLIC_SETBIT(temp[1], 29);
        }
        if(Io_ReadExtCarStatusBit(CAR_STATUS_BIT_AIR_CONDITION))
        {
            PUBLIC_SETBIT(temp[1], 30);
        }
        if(Io_ReadExtCarStatusBit(CAR_STATUS_BIT_SIDE_LIGHT))
        {
            PUBLIC_SETBIT(temp[1], 31);
        }
        Public_ConvertLongToBuffer(temp[1],&buf[4]);
        temp[2] = 0;
        Public_ConvertLongToBuffer(temp[2],&buf[8]);
    }
    PactSendCmdForNaviLcdEx(NAVILCD_CMD_QUERY_STATUS_ALARM,buf,12);
}

/*******************************************************************************
** 函数名称: NaviLcd_DisposeQueryWorkStatus
** 函数功能:  
** 入口参数:  
** 出口参数: 无
** 返回参数:  
** 全局变量: 无
** 调用模块: 无
*******************************************************************************/
void NaviLcd_DisposeQueryWorkStatus(void)
{ 
    GPS_STRUCT stTmpGPS;
    unsigned char buffer[30]={0};
    unsigned char buf[30]={0};
    unsigned char temp1; 
    unsigned short temp;
    //////////////////////
    //当前速度
    temp1 = SpeedMonitor_GetCurSpeed();
    ////////////////
    buf[0] = temp1;//速度   
    buf[1] = communicatio_GetMudulState(COMMUNICATE_STATE)?1:0;//连通标志
    ///////////////////////////////////////
    temp1 =communicatio_GetMudulState(SIGN_STRENTH);//网络信号量    
    if(temp1 !=0)//无信号
    {
        temp1 = temp1/6 + 1;
        if(temp1 > 5)
        {
            temp1 = 5;//最大信号强度为5级
        }
    }
    //////////////////////////////
    buf[2] = 0;
    //buf[2] = (Gps_GetCurrentStarNumEx()<<4);    
    buf[2] |= temp1;//网络信号量
    ////////////////////////////////////
    /////////////////////////////////
    buf[3] = Io_ReadStatusBit(STATUS_BIT_NAVIGATION);//定位标志
    ////////////////////////////////////
    temp = Gps_ReadCourse();//方向
    temp1= temp*2/3;
    buf[4] = buf[3]?temp1:0;//方向
    ////////////////////////////
    ////////////////////
    Public_GetCurBasicPositionInfo(buffer);
    memcpy(&buf[5],&buffer[8],8); //纬度\经度
    buf[13] = Io_ReadStatusBit(STATUS_BIT_SOUTH);
    buf[14] = Io_ReadStatusBit(STATUS_BIT_EAST);
    ////////////////////
    /////报警位//////////
    buf[15] = 0;
    /////超速报警位//////////
    temp =Io_ReadAlarmBit(ALARM_BIT_OVER_SPEED);    
    if(temp)
    PUBLIC_SETBIT(buf[15],0);
    /////疲劳驾驶//////////
    temp =Io_ReadAlarmBit(ALARM_BIT_TIRED_DRIVE);    
    if(temp)
    PUBLIC_SETBIT(buf[15],1);
    //////终端注销//////////////////////////
    if(1 == s_stNaviLcdAttrib.logout)
    PUBLIC_SETBIT(buf[15],2);
    //////传感器故障//////////////////////////
    #ifdef USE_SYS_CAR_LOAD
    if(Carload_GetSensorAbnormalFlag())
    PUBLIC_SETBIT(buf[15],3);
    if(CarLoad_GetCurOverLoadFlag())
    PUBLIC_SETBIT(buf[15],5);
    #endif
    //////////////////////
    #ifdef USE_QUICK_OPEN_ACCOUNT    
    if(OpenAccount_GetCurLockStatus())
    PUBLIC_SETBIT(buf[15],4);
    #endif
     /////GPS卫星颗数//////////
    Gps_CopygPosition(&stTmpGPS);
    buf[16] = stTmpGPS.SatNum;
    ///////北斗卫星颗数///////////////////////////////
    buf[17] = 0;
    //////////////////
    #ifdef USE_SYS_CAR_LOAD
    Public_ConvertLongToBuffer(Carload_GetCurTotalWeight(), &buf[18]);
    #endif
    /////////////////////
    PactSendCmdForNaviLcdEx(NAVILCD_CMD_QUERY_WORK_STATUS,buf,24);
}
/*******************************************************************************
** 函数名称: NaviLcd_SendWorkStatus
** 函数功能:  
** 入口参数:  
** 出口参数: 无
** 返回参数:  
** 全局变量: 无
** 调用模块: 无
*******************************************************************************/
void NaviLcd_SendWorkStatus(void)
{
    static unsigned char sucCnt=0;
    static unsigned char sucCnt1=0;
    sucCnt++;
    if(!IsNaviLcdCommBusy())//不忙,直接发送
    {
        LZM_PublicSetOnceTimer(&s_stNVTimer[NV_TIMER_STATUS],PUBLICSECS(2),NaviLcd_SendWorkStatus);        
        sucCnt =0;
        sucCnt1++;
        if(sucCnt1>=15)//30s当心跳包
        {
           sucCnt1 =0;
           NaviLcdSendCmdQueryStatus();
        }
        else
        {
            NaviLcd_DisposeQueryWorkStatus();
        }        
    }    else

    if(sucCnt<4)//////////////////
    {
        LZM_PublicSetOnceTimer(&s_stNVTimer[NV_TIMER_STATUS],PUBLICSECS(0.2),NaviLcd_SendWorkStatus);
    }
    else
    {
        LZM_PublicSetOnceTimer(&s_stNVTimer[NV_TIMER_STATUS],PUBLICSECS(1.2),NaviLcd_SendWorkStatus);
    }
}

/*******************************************************************************
** 函数名称: NaviLcd_DisposeOutCityRegister
** 函数功能: 出城登记 
** 入口参数:  
** 出口参数: 无
** 返回参数:  
** 全局变量: 无
** 调用模块: 无
*******************************************************************************/
unsigned char NaviLcd_DisposeOutCityRegister(void)
{
  /*//joneming 2013.1.15
    unsigned char buffer[1];
    ProtocolACK rev_ack;
    ////////在这调用出城登记函数///////////////////////
    rev_ack = RadioProtocol_OutRegister(buffer,0);
    ///////////////////
    if(ACK_OK==rev_ack)return 0;//发送成功
  */
    return 0xff;////发送失败
}
/*******************************************************************************
** 函数名称: NaviLcd_DisposeOutCityRegister
** 函数功能:  
** 入口参数:  
** 出口参数: 无
** 返回参数:  
** 全局变量: 无
** 调用模块: 无
*******************************************************************************/
void NaviLcd_DisposeExportDataToUsbDisk(void)
{
    Usb_FindSDcardFileAndSaveToUdisk(s_ucNaviLcdParseBuffer[0],&s_ucNaviLcdParseBuffer[1],&s_ucNaviLcdParseBuffer[7]);
}
/*******************************************************************************
** 函数名称: NaviLcd_SendCarLoadClearSensorResult
** 函数功能: 发送传感器清零结果
** 入口参数:  
** 出口参数: 无
** 返回参数:  
** 全局变量: 无
** 调用模块: 无
*******************************************************************************/
void NaviLcd_SendCarLoadClearSensorResult(void)
{
    #ifdef USE_SYS_CAR_LOAD    
    unsigned char buffer[10]={0};
    unsigned char len;
    len =0;
    buffer[len++] = 1;//类型
    Public_ConvertLongToBuffer(Carload_GetClearSensorResult(),&buffer[len]);
    len +=4;
    //////////////////
    PactSendCmdForNaviLcdEx(NAVILCD_CMD_CARLOAD_WEIGHT,buffer,len);
    #endif
}
/*******************************************************************************
** 函数名称: NaviLcd_DisposeCarLoadWeight
** 函数功能:  
** 入口参数:  
** 出口参数: 无
** 返回参数:  
** 全局变量: 无
** 调用模块: 无
*******************************************************************************/
void NaviLcd_DisposeCarLoadWeight(void)
{
    #ifdef USE_SYS_CAR_LOAD
    unsigned char buffer[10]={0};
    unsigned char type,len;
    type=s_ucNaviLcdParseBuffer[0];
    len =0;
    buffer[len++] = type;    
    if(0==type)//查询传感器故障
    {
        if(!CarLoad_GetCurOnlineStatus())return;//
        Public_ConvertLongToBuffer(Carload_GetCurSensorAbnormalValue(),&buffer[len]);
        len +=4;
    }
    else
    if(1==type)//清空传感器故障
    {
        Carload_SendCmdClearSensorAbnormal();
        buffer[len++] = 0;        
        return;
    } 
    else
    if(2==type)//运货信息
    {
        CarLoad_SendFreightInformationToService(s_ucNaviLcdParseBuffer[1]);
        buffer[len++] =0;
    } 
    //////////////////
    PactSendCmdForNaviLcdEx(NAVILCD_CMD_CARLOAD_WEIGHT,buffer,len);
    #endif
}
/*******************************************************************************
** 函数名称: NaviLcd_CheckDataIsIPFormat
** 函数功能: 判断数据是否为IP地址格式
** 入口参数: data数据首地址,datalen数据长度
** 返回参数: 1是IP地址格式,0不是IP地址格式格式
** 全局变量: 无
** 调用模块: 无
*******************************************************************************/
unsigned char NaviLcd_CheckDataIsIPFormat(unsigned char *data,unsigned char datalen)
{
    unsigned char i;
    for(i=0; i<datalen; i++)
    {
        if((data[i]<0x30||data[i]>0x39)&&data[i]!='.'&&data[i]!=0)
        {
            return 0;
        }
    }
    return 1;
}
/*******************************************************************************
** 函数名称: NaviLcd_QueryOrSaveTerminalParamter
** 函数功能: 参数查询命令
** 入口参数: ID:参数ID,	datalen参数长度;buffer:参数缓存,type:类型,0:设置;1:查询;
** 出口参数: 无
** 返回参数: 当前参数打包之后的长度
** 全局变量: 无
** 调用模块: 无
*******************************************************************************/
unsigned char NaviLcd_QueryOrSaveTerminalParamter(unsigned char ID,unsigned char datalen,unsigned char *buffer,unsigned char type)
{
    unsigned long val;
    unsigned char len;   
    unsigned char buf[40]={0};
    switch(ID)
    {
        case TERMINAL_PARAM_MAIN_APN://主服务器 APN
            if(type)
            {
                len=EepromPram_ReadPram(E2_MAIN_SERVER_APN_ID, buffer);
            }
            else
            {
                EepromPram_WritePram(E2_MAIN_SERVER_APN_ID, buffer,datalen);
                EepromPram_ReadPram(E2_MAIN_SERVER_APN_ID, buf);
                if(strncmp((char*)buf,(char*)buffer,datalen)==0)
                {
                    EepromPram_UpdateVariable(E2_MAIN_SERVER_APN_ID);
                }
            }
            break;
        case TERMINAL_PARAM_RADIO_USER://主服务器无线通信拨号用户名
            if(type)
            {
                len=EepromPram_ReadPram(E2_MAIN_SERVER_DIAL_NAME_ID, buffer);
            }
            else
            {
                EepromPram_WritePram(E2_MAIN_SERVER_DIAL_NAME_ID, buffer,datalen);
                EepromPram_ReadPram(E2_MAIN_SERVER_DIAL_NAME_ID, buf);
                if(strncmp((char*)buf,(char*)buffer,datalen)==0)
                {
                    EepromPram_UpdateVariable(E2_MAIN_SERVER_DIAL_NAME_ID);
                }
            }            
            break;
        case TERMINAL_PARAM_RADIO_PASSWORD://主服务器无线通信拨号密码
            if(type)
            {
                len=EepromPram_ReadPram(E2_MAIN_SERVER_DIAL_PASSWORD_ID, buffer);
            }
            else
            {
                EepromPram_WritePram(E2_MAIN_SERVER_DIAL_PASSWORD_ID, buffer,datalen);
                EepromPram_ReadPram(E2_MAIN_SERVER_DIAL_PASSWORD_ID, buf);
                if(strncmp((char*)buf,(char*)buffer,datalen)==0)
                {
                    EepromPram_UpdateVariable(E2_MAIN_SERVER_DIAL_PASSWORD_ID);
                }
            }  
            break;        
        case TERMINAL_PARAM_BAK_APN://备份服务器 APN，无线通信拨号访问点
            if(type)
            {
                len=EepromPram_ReadPram(E2_BACKUP_SERVER_APN_ID, buffer);
            }
            else
            {
                EepromPram_WritePram(E2_BACKUP_SERVER_APN_ID, buffer,datalen);
                EepromPram_ReadPram(E2_BACKUP_SERVER_APN_ID, buf);
                if(strncmp((char*)buf,(char*)buffer,datalen)==0)
                {
                    EepromPram_UpdateVariable(E2_BACKUP_SERVER_APN_ID);
                }
            }
            break;
        case TERMINAL_PARAM_BAK_RADIO_USER://备份服务器无线通信拨号用户名  
            if(type)
            {
                len=EepromPram_ReadPram(E2_BACKUP_SERVER_DIAL_NAME_ID, buffer);
            }
            else
            {
                EepromPram_WritePram(E2_BACKUP_SERVER_DIAL_NAME_ID, buffer,datalen);
                EepromPram_ReadPram(E2_BACKUP_SERVER_DIAL_NAME_ID, buf);
                if(strncmp((char*)buf,(char*)buffer,datalen)==0)
                {
                    EepromPram_UpdateVariable(E2_BACKUP_SERVER_DIAL_NAME_ID);
                }
            }
            break;
        case TERMINAL_PARAM_BAK_RADIO_PASSWORD://备份服务器无线通信拨号密码
            if(type)
            {
                len=EepromPram_ReadPram(E2_BACKUP_SERVER_DIAL_PASSWORD_ID, buffer);
            }
            else
            {
                EepromPram_WritePram(E2_BACKUP_SERVER_DIAL_PASSWORD_ID, buffer,datalen);
                EepromPram_ReadPram(E2_BACKUP_SERVER_DIAL_PASSWORD_ID, buf);
                if(strncmp((char*)buf,(char*)buffer,datalen)==0)
                {
                    EepromPram_UpdateVariable(E2_BACKUP_SERVER_DIAL_PASSWORD_ID);
                }
            }
            break;        
        case TERMINAL_PARAM_TCP://服务器 TCP 端口 
            if(type)
            {
                len=EepromPram_ReadPram(E2_MAIN_SERVER_TCP_PORT_ID, buffer);
            }
            else
            {
                EepromPram_WritePram(E2_MAIN_SERVER_TCP_PORT_ID, buffer,datalen);
                EepromPram_ReadPram(E2_MAIN_SERVER_TCP_PORT_ID, buf);
                if(strncmp((char*)buf,(char*)buffer,datalen)==0)
                {
                    EepromPram_UpdateVariable(E2_MAIN_SERVER_TCP_PORT_ID);
                }
            }
            break;
        case TERMINAL_PARAM_UDP://服务器 UDP 端口
            if(type)
            {
                len=EepromPram_ReadPram(E2_MAIN_SERVER_UDP_PORT_ID, buffer);
            }
            else
            {
                EepromPram_WritePram(E2_MAIN_SERVER_UDP_PORT_ID, buffer,datalen);
                EepromPram_ReadPram(E2_MAIN_SERVER_UDP_PORT_ID, buf);
                if(strncmp((char*)buf,(char*)buffer,datalen)==0)
                {
                    EepromPram_UpdateVariable(E2_MAIN_SERVER_UDP_PORT_ID);
                }
            }
            break;
        case TERMINAL_PARAM_SPEED_ALARM://最高速度，单位为公里每小时（km/h）
            if(type)
            {
                len=EepromPram_ReadPram(E2_MAX_SPEED_ID, buffer);
            }
            else
            {
                EepromPram_WritePram(E2_MAX_SPEED_ID, buffer,datalen);
                EepromPram_ReadPram(E2_MAX_SPEED_ID, buf);
                if(strncmp((char*)buf,(char*)buffer,datalen)==0)
                {
                    EepromPram_UpdateVariable(E2_MAX_SPEED_ID);
                }
            }
            break;
        case TERMINAL_PARAM_TOTAL_MILE://车辆里程表读数，1/10km 
            if(type)
            {
                len=E2ParamApp_ReadSpecialParam(E2_CAR_TOTAL_MILE_ID,buffer);
                //len=EepromPram_ReadPram(E2_CAR_TOTAL_MILE_ID, buffer);
            }
            else
            {
                
                EepromPram_WritePram(E2_CAR_TOTAL_MILE_ID, buffer,datalen);
                EepromPram_ReadPram(E2_CAR_TOTAL_MILE_ID, buf);
                if(strncmp((char*)buf,(char*)buffer,datalen)==0)
                {
                    EepromPram_UpdateVariable(E2_CAR_TOTAL_MILE_ID);
                }
            }
            break;
        case TERMINAL_PARAM_CAR_PLATE://公安交通管理部门颁发的机动车号牌
            if(type)
            {
                len=EepromPram_ReadPram(E2_CAR_PLATE_NUM_ID, buffer);
            }
            else
            {
                EepromPram_WritePram(E2_CAR_PLATE_NUM_ID, buffer,datalen);
                EepromPram_ReadPram(E2_CAR_PLATE_NUM_ID, buf);
                if(strncmp((char*)buf,(char*)buffer,datalen)==0)
                {
                    EepromPram_UpdateVariable(E2_CAR_PLATE_NUM_ID);
                }
            }
            break;
        case TERMINAL_PARAM_CAR_COLOR://车牌颜色，按照 JT/T415-2006 的 5.4.12
            if(type)
            {
                len=EepromPram_ReadPram(E2_CAR_PLATE_COLOR_ID, buffer);
            }
            else
            {
                
                EepromPram_WritePram(E2_CAR_PLATE_COLOR_ID, buffer,datalen);
                EepromPram_ReadPram(E2_CAR_PLATE_COLOR_ID, buf);
                if(strncmp((char*)buf,(char*)buffer,datalen)==0)
                {
                    EepromPram_UpdateVariable(E2_CAR_PLATE_COLOR_ID);
                }                
            }
            break;
        case TERMINAL_PARAM_PHONE_NUMBER://终端手机号
            if(type)
            {
                len=EepromPram_ReadPram(E2_DEVICE_PHONE_ID, buffer);
            }
            else
            {
                EepromPram_WritePram(E2_DEVICE_PHONE_ID, buffer,datalen);
                EepromPram_ReadPram(E2_DEVICE_PHONE_ID, buf);
                if(strncmp((char*)buf,(char*)buffer,datalen)==0)
                {
                    EepromPram_UpdateVariable(E2_DEVICE_PHONE_ID);
                }
            }
            break;
        case TERMINAL_PARAM_CAR_VIN://车辆识别码
            if(type)
            {
                len=EepromPram_ReadPram(E2_CAR_IDENTIFICATION_CODE_ID, buffer);
            }
            else
            {
                
                EepromPram_WritePram(E2_CAR_IDENTIFICATION_CODE_ID, buffer,datalen);
                EepromPram_ReadPram(E2_CAR_IDENTIFICATION_CODE_ID, buf);
                if(strncmp((char*)buf,(char*)buffer,datalen)==0)
                {
                    EepromPram_UpdateVariable(E2_CAR_IDENTIFICATION_CODE_ID);
                }
            }
            break;
        case TERMINAL_PARAM_CAR_PLATE_TYPE://车牌分类 
            if(type)
            {
                buffer[0]=1;
                len=EepromPram_ReadPram(E2_CAR_TYPE_ID, buf);
                if(len)
                {
                    unsigned char i;
                    for(i=0;i<4;i++)
                    {
                        if(strncmp((char*)buf,(char*)c_apcCarPlakeTypeItems[i],4)==0)
                        {
                            buffer[0]=i+1;
                            break;
                        }
                    }
                }
                len=1;
            }
            else
            {
                val =buffer[0]-1;
                len=strlen(c_apcCarPlakeTypeItems[val]);
                EepromPram_WritePram(E2_CAR_TYPE_ID, (unsigned char*)c_apcCarPlakeTypeItems[val],len);
                EepromPram_ReadPram(E2_CAR_TYPE_ID, buf);
                if(strncmp((char*)buf,(char*)c_apcCarPlakeTypeItems[val],len)==0)
                {
                    EepromPram_UpdateVariable(E2_CAR_TYPE_ID);
                }
            }            
            break;
        case TERMINAL_PARAM_SPEED_SENSOR://速度传感器系数
            if(type)
            {
                len=EepromPram_ReadPram(E2_SPEED_SENSOR_COEF_ID, buffer);
            }
            else
            {
                EepromPram_WritePram(E2_SPEED_SENSOR_COEF_ID, buffer,datalen);
                EepromPram_ReadPram(E2_SPEED_SENSOR_COEF_ID, buf);
                if(strncmp((char*)buf,(char*)buffer,datalen)==0)
                {
                    EepromPram_UpdateVariable(E2_SPEED_SENSOR_COEF_ID);
                }
            }
            break;
        case TERMINAL_PARAM_FEATURE_COEF://车辆特征系数
            if(type)
            {
                len=EepromPram_ReadPram(E2_CAR_FEATURE_COEF_ID, buffer);
            }
            else
            {
                EepromPram_WritePram(E2_CAR_FEATURE_COEF_ID, buffer,datalen);
                EepromPram_ReadPram(E2_CAR_FEATURE_COEF_ID, buf);
                if(strncmp((char*)buf,(char*)buffer,datalen)==0)
                {
                    EepromPram_UpdateVariable(E2_CAR_FEATURE_COEF_ID);
                }
            }
            break;
        case TERMINAL_PARAM_SPEED_SELECT://速度选择开关
            if(type)
            {
                len=EepromPram_ReadPram(E2_SPEED_SELECT_ID, buffer);
            }
            else
            {
                EepromPram_WritePram(E2_SPEED_SELECT_ID, buffer,datalen);
                EepromPram_ReadPram(E2_SPEED_SELECT_ID, buf);
                if(strncmp((char*)buf,(char*)buffer,datalen)==0)
                {
                    EepromPram_UpdateVariable(E2_SPEED_SELECT_ID);
                }
            }
            break;
        case TERMINAL_PARAM_SET_BACKLIGHT://背光选择开关            
            if(type)
            {
                len=EepromPram_ReadPram(E2_BACKLIGHT_SELECT_ID, buffer);
            }
            else
            {
                EepromPram_WritePram(E2_BACKLIGHT_SELECT_ID, buffer,datalen);
                EepromPram_ReadPram(E2_BACKLIGHT_SELECT_ID, buf);
                if(strncmp((char*)buf,(char*)buffer,datalen)==0)
                {
                    EepromPram_UpdateVariable(E2_BACKLIGHT_SELECT_ID);
                }
            }            
            break;
        case TERMINAL_PARAM_DOOR_LEVEL://车门正负设置            
            if(type)
            {
                len=EepromPram_ReadPram(E2_DOOR_SET_ID, buffer);
            }
            else
            {
                EepromPram_WritePram(E2_DOOR_SET_ID, buffer,datalen);
                EepromPram_ReadPram(E2_DOOR_SET_ID, buf);
                if(strncmp((char*)buf,(char*)buffer,datalen)==0)
                {
                    EepromPram_UpdateVariable(E2_DOOR_SET_ID);
                }
            }
            break;
        case TERMINAL_PARAM_CDMA_SLEEP://CDMA休眠开关 
            if(type)
            {
                len=EepromPram_ReadPram(E2_CDMA_SLEEP_ID, buffer);
            }
            else
            {
                EepromPram_WritePram(E2_CDMA_SLEEP_ID, buffer,datalen);
                EepromPram_ReadPram(E2_CDMA_SLEEP_ID, buf);
                if(strncmp((char*)buf,(char*)buffer,datalen)==0)
                {
                    EepromPram_UpdateVariable(E2_CDMA_SLEEP_ID);
                }
            }
            break;
        case TERMINAL_PARAM_CAR_HORN_LEVEL://车喇叭电平选择 
            if(type)
            {
                len=EepromPram_ReadPram(E2_HORN_SET_ID, buffer);
            }
            else
            {
                EepromPram_WritePram(E2_HORN_SET_ID, buffer,datalen);
                EepromPram_ReadPram(E2_HORN_SET_ID, buf);
                if(strncmp((char*)buf,(char*)buffer,datalen)==0)
                {
                    EepromPram_UpdateVariable(E2_HORN_SET_ID);
                }
            } 
            break;
        case TERMINAL_PARAM_TERMINAL_ID://终端ID(LCD只可查询，不可写入)
            if(type)
            {
                len=EepromPram_ReadPram(E2_DEVICE_ID, buffer);
            }            
            else
            {
                #if 0 //不可写入
                EepromPram_WritePram(E2_DEVICE_ID, buffer,datalen);
                EepromPram_ReadPram(E2_DEVICE_ID, buf);
                if(strncmp((char*)buf,(char*)buffer,datalen)==0)
                {
                    EepromPram_UpdateVariable(E2_DEVICE_ID);
                }
                #endif
            }
            break;
        case TERMINAL_PARAM_PASSWORD:
            if(type)
            {
                len=EepromPram_ReadPram(E2_PASSWORD_ID, buffer);
            }
            else
            {
                EepromPram_WritePram(E2_PASSWORD_ID, buffer,datalen);
                EepromPram_ReadPram(E2_PASSWORD_ID, buf);
                if(strncmp((char*)buf,(char*)buffer,datalen)==0)
                {
                    EepromPram_UpdateVariable(E2_PASSWORD_ID);
                }
            }
            break;
       #ifdef USE_SYS_CAR_LOAD
        case TERMINAL_PARAM_WEIGHT:
            if(type)
            {
                len=EepromPram_ReadPram(E2_LOAD_MAX_ID, buffer);
            }
            else
            {
                EepromPram_WritePram(E2_LOAD_MAX_ID, buffer,datalen);
                EepromPram_ReadPram(E2_LOAD_MAX_ID, buf);
                if(strncmp((char*)buf,(char*)buffer,datalen)==0)
                {
                    EepromPram_UpdateVariable(E2_LOAD_MAX_ID);
                }
            }
            break;
       #endif
       case TERMINAL_PARAM_OWNER_PHONE:
            if(type)
            {
                len=Public_ReadDataFromFRAM(FRAM_CAR_OWNER_PHONE_ADDR, buffer,FRAM_CAR_OWNER_PHONE_LEN);
            }
            else
            {
                memset(buf,0,FRAM_CAR_OWNER_PHONE_LEN+1);
                memcpy(buf,buffer,datalen);
                Public_WriteDataToFRAM(FRAM_CAR_OWNER_PHONE_ADDR, buf,FRAM_CAR_OWNER_PHONE_LEN);                
            }
            break;
       case TERMINAL_PARAM_CAR_PROVINCE_ID:
            if(type)
            {
                len=EepromPram_ReadPram(E2_CAR_PROVINCE_ID, buffer);
            }
            else
            {
                EepromPram_WritePram(E2_CAR_PROVINCE_ID, buffer,datalen);
                EepromPram_ReadPram(E2_CAR_PROVINCE_ID, buf);
                if(strncmp((char*)buf,(char*)buffer,datalen)==0)
                {
                    EepromPram_UpdateVariable(E2_CAR_PROVINCE_ID);
                }
            }
            break;
       case TERMINAL_PARAM_CAR_CITY_ID:
            if(type)
            {
                len=EepromPram_ReadPram(E2_CAR_CITY_ID, buffer);
            }
            else
            {
                EepromPram_WritePram(E2_CAR_CITY_ID, buffer,datalen);
                EepromPram_ReadPram(E2_CAR_CITY_ID, buf);
                if(strncmp((char*)buf,(char*)buffer,datalen)==0)
                {
                    EepromPram_UpdateVariable(E2_CAR_CITY_ID);
                }
            }
            break;        
        case TERMINAL_PARAM_MAIN_IP://主服务器地址,IP 或域名
            #if 1
            if(type)
            {
                len=EepromPram_ReadPram(E2_MAIN_SERVER_IP_ID, buffer);
            }
            else
            {
                EepromPram_WritePram(E2_MAIN_SERVER_IP_ID, buffer,datalen);
                EepromPram_ReadPram(E2_MAIN_SERVER_IP_ID, buf);
                if(strncmp((char*)buf,(char*)buffer,datalen)==0)
                {
                    EepromPram_UpdateVariable(E2_MAIN_SERVER_IP_ID);
                }
            }
            break;
            #endif
        case TERMINAL_PARAM_MAIN_DOMAIN_NAME:
            if(type)
            {
                len=Public_ReadDataFromFRAM(FRAM_MAIN_DOMAIN_NAME_ADDR, buffer,FRAM_MAIN_DOMAIN_NAME_LEN);
            }
            else
            {
                memset(buf,0,FRAM_MAIN_DOMAIN_NAME_LEN+1);
                memcpy(buf,buffer,datalen);
                Public_WriteDataToFRAM(FRAM_MAIN_DOMAIN_NAME_ADDR, buf,FRAM_MAIN_DOMAIN_NAME_LEN);
                if(NaviLcd_CheckDataIsIPFormat(buffer,datalen))
                {
                    EepromPram_WritePram(E2_MAIN_SERVER_IP_ID, buffer,datalen);
                    EepromPram_ReadPram(E2_MAIN_SERVER_IP_ID, buf);
                    if(strncmp((char*)buf,(char*)buffer,datalen)==0)
                    {
                        EepromPram_UpdateVariable(E2_MAIN_SERVER_IP_ID);
                    }
                }                
            }
            break;        
        case TERMINAL_PARAM_BAK_IP://备份服务器地址,IP 或域名
            #if 1
            if(type)
            {
                len=EepromPram_ReadPram(E2_BACKUP_SERVER_IP_ID, buffer);
            }
            else
            {
                EepromPram_WritePram(E2_BACKUP_SERVER_IP_ID, buffer,datalen);
                EepromPram_ReadPram(E2_BACKUP_SERVER_IP_ID, buf);
                if(strncmp((char*)buf,(char*)buffer,datalen)==0)
                {
                    EepromPram_UpdateVariable(E2_BACKUP_SERVER_IP_ID);
                }
            }
            break;
            #endif
        case TERMINAL_PARAM_BACKUP_DOMAIN_NAME:
            if(type)
            {
                len=Public_ReadDataFromFRAM(FRAM_BACKUP_DOMAIN_NAME_ADDR, buffer,FRAM_BACKUP_DOMAIN_NAME_LEN);
            }
            else
            {
                memset(buf,0,FRAM_BACKUP_DOMAIN_NAME_LEN+1);
                memcpy(buf,buffer,datalen);
                Public_WriteDataToFRAM(FRAM_BACKUP_DOMAIN_NAME_ADDR, buf,FRAM_BACKUP_DOMAIN_NAME_LEN);
                if(NaviLcd_CheckDataIsIPFormat(buffer,datalen))
                {
                    EepromPram_WritePram(E2_BACKUP_SERVER_IP_ID, buffer,datalen);
                    EepromPram_ReadPram(E2_BACKUP_SERVER_IP_ID, buf);
                    if(strncmp((char*)buf,(char*)buffer,datalen)==0)
                    {
                        EepromPram_UpdateVariable(E2_BACKUP_SERVER_IP_ID);
                    }
                }
            }
            break;
       case TERMINAL_PARAM_MANUFACTURE_ID:
            if(type)
            {
                len=EepromPram_ReadPram(E2_MANUFACTURE_ID, buffer);
            }
            else
            {
                EepromPram_WritePram(E2_MANUFACTURE_ID, buffer,datalen);
                EepromPram_ReadPram(E2_MANUFACTURE_ID, buf);
                if(strncmp((char*)buf,(char*)buffer,datalen)==0)
                {
                    EepromPram_UpdateVariable(E2_MANUFACTURE_ID);
                }
            }
            break;
            
    }
    return len;
}
/*******************************************************************************
** 函数名称: NaviLcd_DisposeSetTerminalParamter
** 函数功能:  
** 入口参数:  
** 出口参数: 无
** 返回参数:  
** 全局变量: 无
** 调用模块: 无
*******************************************************************************/
void NaviLcd_DisposeSetTerminalParamter(void)
{
    unsigned char i,num,ID,len;
    unsigned short index;
    unsigned char data[40];
    index = 0;
    num = s_ucNaviLcdParseBuffer[index++];    
    for(i=0 ; i<num ; i++)
    {
        ID = s_ucNaviLcdParseBuffer[index++];
        len = s_ucNaviLcdParseBuffer[index++]; 
        if(index +len > s_stNaviLcdAttrib.recvDatalen)break;
        memcpy(data,&s_ucNaviLcdParseBuffer[index],len);
        index +=len;
        ///////////////////////////
        if(len)
        NaviLcd_QueryOrSaveTerminalParamter(ID,len,data,0);
    }
}
/*******************************************************************************
** 函数名称: NaviLcd_DisposeQueryTerminalParamter
** 函数功能:  
** 入口参数:  
** 出口参数: 无
** 返回参数:  
** 全局变量: 无
** 调用模块: 无
*******************************************************************************/
void NaviLcd_DisposeQueryTerminalParamter(void)
{
    unsigned char i,num,ID,datalen;
    unsigned short len;
    unsigned char data[1024];
    unsigned char buffer[30];
    
    num = s_ucNaviLcdParseBuffer[0];    
    len = 0;
    /////////////////
    data[len++] = num;
    ////////////////
    for(i=0 ; i<num ; i++)
    {
        ID = s_ucNaviLcdParseBuffer[i+1];
        datalen=NaviLcd_QueryOrSaveTerminalParamter(ID,0,buffer,1);
        data[len++]=ID;
        data[len++]=datalen;        
        memcpy(&data[len],buffer,datalen);
        len+=datalen;
    }
    /////////////////////
    NaviLcdSendCmdQueryParameter(data,len);
}
/*******************************************************************************
** 函数名称: NaviLcd_RecvKeyTimeOut
** 函数功能:  
** 入口参数:  
** 出口参数: 无
** 返回参数:  
** 全局变量: 无
** 调用模块: 无
*******************************************************************************/
void NaviLcd_RecvKeyTimeOut(void)
{
    s_stNVKey.datalen = 0;
    s_stNaviLcdAttrib.newphone = 0;
}
/*******************************************************************************
** 函数名称: NaviLcd_CurrentPhoneVolUp
** 函数功能:  音量加加
** 入口参数:  
** 出口参数: 无
** 返回参数:  
** 全局变量: 无
** 调用模块: 无
*******************************************************************************/
void NaviLcd_CurrentPhoneVolUp(void)
{
    SetTelVolumeIncr();
}
/*******************************************************************************
** 函数名称: NaviLcd_CurrentPhoneVolDown
** 函数功能: 音量减小 
** 入口参数:  
** 出口参数: 无
** 返回参数:  
** 全局变量: 无
** 调用模块: 无
*******************************************************************************/
void NaviLcd_CurrentPhoneVolDown(void)
{
    SetTelVolumeDecr();
}
/*******************************************************************************
** 函数名称: NaviLcd_CurrentPhoneVolUp
** 函数功能:  音量加加
** 入口参数:  
** 出口参数: 无
** 返回参数:  
** 全局变量: 无
** 调用模块: 无
*******************************************************************************/
void NaviLcd_CurrentTTSVolUp(void)
{
    SetTtsVolumeIncr();    
}
/*******************************************************************************
** 函数名称: NaviLcd_CurrentPhoneVolDown
** 函数功能: 音量减小 
** 入口参数:  
** 出口参数: 无
** 返回参数:  
** 全局变量: 无
** 调用模块: 无
*******************************************************************************/
void NaviLcd_CurrentTTSVolDown(void)
{
    SetTtsVolumeDecr();
}
/*******************************************************************************
** 函数名称: NaviLcd_DisposeKeyValue
** 函数功能:  
** 入口参数:  
** 出口参数: 无
** 返回参数:  
** 全局变量: 无
** 调用模块: 无
*******************************************************************************/
void NaviLcd_HangUpPhone(void)
{
    s_stNaviLcdAttrib.newphone = 0;
    communication_CallPhone("13800000000",PHONE_END);
}
/*******************************************************************************
** 函数名称: NaviLcd_DisposeKeyValue
** 函数功能:  
** 入口参数:  
** 出口参数: 无
** 返回参数:  
** 全局变量: 无
** 调用模块: 无
*******************************************************************************/
unsigned char NaviLcd_DisposeKeyValue(unsigned char value)
{
    unsigned char result;
    result = 0;
    switch(value)
    {
        case NAVILCD_KEYVALUE_HOT_LINE://拨打热线电话 
            s_stNVKey.datalen = EepromPram_ReadPram(E2_MONITOR_SERVER_PHONE_ID, s_stNVKey.Keybuff);
            s_stNVKey.Keybuff[s_stNVKey.datalen]='\0';
            if(s_stNVKey.datalen)
            result=communication_CallPhone(s_stNVKey.Keybuff,PHONE_NORMAL);
            if(result)result = 0;
            else result = 0x02;
            s_stNVKey.datalen = 0;
            break;
        case NAVILCD_KEYVALUE_ANSWER_CALL://接听电话
            result=communication_CallPhone("0",PHONE_TALK);
            if(result)
            {
                s_stNaviLcdAttrib.newphone = 2; 
                LZM_PublicSetOnceTimer(&s_stNVTimer[NV_TIMER_KEY],PUBLICSECS(5),NaviLcd_RecvKeyTimeOut);
                result = 0;
            }
            else result = 0x02;
            break;
        case NAVILCD_KEYVALUE_HANG_UP://挂断
            NaviLcd_RecvKeyTimeOut();
            if(0==communication_CallPhone("13800000000",PHONE_END))
            {
                LZM_PublicSetOnceTimer(&s_stNVTimer[NV_TIMER_KEY],PUBLICSECS(3),NaviLcd_HangUpPhone);
            }           
            break;
        case NAVILCD_KEYVALUE_HF://免提            
            break;
        case NAVILCD_KEYVALUE_REDIAL://重拨
            s_stNVKey.datalen = 0;
            result=NaviLcd_CheckDailPhone(s_stNVKey.Keybuff);
            break;
        case NAVILCD_KEYVALUE_DEL://删除
            if(s_stNVKey.datalen)s_stNVKey.datalen--;
            LZM_PublicSetOnceTimer(&s_stNVTimer[NV_TIMER_KEY],PUBLICSECS(3),NaviLcd_RecvKeyTimeOut);
            break;
        case NAVILCD_KEYVALUE_VOL_ADD://音量+
            NaviLcd_CurrentPhoneVolUp();
            break;
        case NAVILCD_KEYVALUE_VOL_DEC://音量-
            NaviLcd_CurrentPhoneVolDown();
            break;
        case NAVILCD_KEYVALUE_ASTERISK://*键 
        case NAVILCD_KEYVALUE_POUND://#键
        case NAVILCD_KEYVALUE_KEY0://数字键:0~9
        case NAVILCD_KEYVALUE_KEY1:
        case NAVILCD_KEYVALUE_KEY2:
        case NAVILCD_KEYVALUE_KEY3:
        case NAVILCD_KEYVALUE_KEY4:
        case NAVILCD_KEYVALUE_KEY5:
        case NAVILCD_KEYVALUE_KEY6:
        case NAVILCD_KEYVALUE_KEY7:
        case NAVILCD_KEYVALUE_KEY8:
        case NAVILCD_KEYVALUE_KEY9:
            #ifdef EYE_MODEM
            Modem_Api_Phone_Vts(&value);
            #endif
            break;
    }
    return result;
}
/*******************************************************************************
** 函数名称: NaviLcdActiveSendKeyValue
** 函数功能:  
** 入口参数:  
** 出口参数: 无
** 返回参数:  
** 全局变量: 无
** 调用模块: 无
*******************************************************************************/
void NaviLcdActiveSendKeyValue(unsigned char value)
{
    PactSendCmdForNaviLcd(NAVILCD_CMD_TERMINAL_KEY_VALUE,&value,1);
/*******************************************************************************
** 函数名称: NaviLcdSendSFExpressBrushCardData
** 函数功能: 发送顺丰刷卡数据内容到导航屏 
** 入口参数:  
** 出口参数: 无
** 返回参数:  
** 全局变量: 无
** 调用模块: 无
*******************************************************************************/
}
/*******************************************************************************
** 函数名称: NaviLcd_DisposeQueryPhoneBooks
** 函数功能:  
** 入口参数:  
** 出口参数: 无
** 返回参数:  
** 全局变量: 无
** 调用模块: 无
*******************************************************************************/
void NaviLcd_DisposeQueryPhoneBooks(unsigned char type)
{
    unsigned char buffer[1024];
    unsigned short datalen;
    //////////////////////////
    datalen = GetPhoneBook(type,buffer);
    NaviLcdSendCmdQueryPhoneBooks(buffer,datalen);
}
/*******************************************************************************
** 函数名称: NaviLcd_DisposeQueryPhoneBooks
** 函数功能:  
** 入口参数:  
** 出口参数: 无
** 返回参数:  
** 全局变量: 无
** 调用模块: 无
*******************************************************************************/
void NaviLcd_DisposeAdjustVolume(void)
{
    unsigned char buffer[2]; 
    unsigned char type,flag,operate;
    unsigned char val;
    type=s_ucNaviLcdParseBuffer[0];//
    flag=s_ucNaviLcdParseBuffer[1];//
    operate=s_ucNaviLcdParseBuffer[2];//
    //////////////////////////
    if(0==type)//通话音量
    {
        if(1==flag)//1:设置
        {
            if(0==operate)// 0：音量加,1：音量减
            {
                NaviLcd_CurrentPhoneVolUp();
            }
            else
            if(1==operate)
            {
                NaviLcd_CurrentPhoneVolDown();
            }
        }
        //////当前音量级别///////////////////
        val= GetTelVolume();
    }
    else
    if(1==type)//TTS音量
    {
        if(1==flag)
        {
            if(0==operate)
            {
                NaviLcd_CurrentTTSVolUp();
            }
            else
            if(1==operate)
            {
                NaviLcd_CurrentTTSVolDown();
            }
        }
        //////////当前音量级别///////////////////////////////////
        val= GetTtsVolume();
    }
    /////////////////
    buffer[0]= 0;
    buffer[1]= val;
    
    PactSendCmdForNaviLcdEx(NAVILCD_CMD_ADJUST_VOLUME,buffer,2);
}
/*************************************************************
** 函数名称: NaviLcd_DisposeQustionResponse
** 功能描述: 发送提问答案命令
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char NaviLcd_DisposeQustionResponse(void)
{
    ProtocolACK val_ack;
    unsigned char ID;
    u8 channel = CHANNEL_DATA_1;
    u16 SerialNum;
    //////////////////////////
    ID=s_ucNaviLcdParseBuffer[4];//
    
    channel = RadioProtocol_GetDownloadCmdChannel(0x8302, &SerialNum);
    /////////发送提问答案到平台///////////////////
    val_ack=RadioProtocol_QuestionAck(channel,ID);
    if(val_ack==ACK_OK)

    {
        return 0;
    }
    else
    {
        return 2;
    }
}
/*************************************************************
** 函数名称: NaviLcd_GetFixPhrase
** 功能描述: 
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned short NaviLcd_GetFixPhrase(unsigned char *buffer)
{
    unsigned char i,length;
    unsigned short index;
    ///////////////////////
    index = 0;    
    buffer[index++] = FIX_PHRASE_ITEMS;
    ////////////////////
    for(i = 0; i < FIX_PHRASE_ITEMS; i++)
    {
        ///////////////////////////////
        buffer[index++] = i+1;
        /////////////////////
        length =strlen((char *)c_apcFixPhrase[i]);
        buffer[index++] = length;
        memcpy(&buffer[index],c_apcFixPhrase[i],length);
        index +=length;
    }
    /////////////////
    return index;
}

/*************************************************************
** 函数名称: NaviLcd_DisposeQueryFixText
** 功能描述: 查询固定文本信息命令
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcd_DisposeQueryFixText(unsigned char type)
{
    unsigned char buffer[1024];
    unsigned short datalen;
    datalen = 0;
    buffer[0] = type;//
    if(FIXTEXT_TYPE_PHRASE == type)//固定短语
    {
        datalen=NaviLcd_GetFixPhrase(&buffer[1]);
        datalen++;
    }
    else
    if(FIXTEXT_TYPE_EVENT == type)////事件报告
    {       
        datalen=GetEventReport(&buffer[1]);//joneming 2013.1.15
        datalen++;
    }
    else
    if(FIXTEXT_TYPE_INFO == type)//信息点播
    {
        datalen = GetInfoDemand(&buffer[1]);//joneming 2013.1.15
        datalen++;
    }
    ///////////////////////////////
    NaviLcdSendCmdQueryFixText(buffer,datalen);
    ////////////////////////
}

/*************************************************************
** 函数名称: NaviLcd_DisposeSendTickleInfo
** 功能描述: 发送反馈信息命令
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char NaviLcd_DisposeSendTickleInfo(void)
{
    unsigned char i;
    unsigned short len;
    unsigned char phone[30];
    ProtocolACK val_ack;
    u8 channel = CHANNEL_DATA_1|CHANNEL_DATA_2;
    /////////////////////////
    if(s_ucNaviLcdParseBuffer[0]=='#'&& s_ucNaviLcdParseBuffer[1]=='#')//发到服务中心
    {
        /////////发送反馈信息到平台///////////////////
        val_ack=RadioProtocol_OriginalDataUpTrans(channel,4,(unsigned char *)&s_ucNaviLcdParseBuffer[2],s_stNaviLcdAttrib.recvDatalen-2);
        if(ACK_OK == val_ack)
        {
            return 0;
        }
    }
    else
    if(s_ucNaviLcdParseBuffer[0]!='#')//发到服务中心 
    {
        /////////发送反馈信息到平台///////////////////
        val_ack=RadioProtocol_OriginalDataUpTrans(channel,4,s_ucNaviLcdParseBuffer,s_stNaviLcdAttrib.recvDatalen);
        if(ACK_OK == val_ack)
        {
            return 0;
        }
    }
    else
    if(s_stNaviLcdAttrib.recvDatalen > 10)
    {
        for(i=1; i<s_stNaviLcdAttrib.recvDatalen;i++)
        {
            if(s_ucNaviLcdParseBuffer[i]=='#')
            {
                if(i<10||i>20)return 2;
                break;
            }
        }
        ///////////////////////////
        memcpy(phone,&s_ucNaviLcdParseBuffer[1],i-1);
        phone[i-1]='\0';
        i++;
        len=Communication_SndMsg2Svr(channel,&s_ucNaviLcdParseBuffer[i],s_stNaviLcdAttrib.recvDatalen-i,phone);
        if(len==s_stNaviLcdAttrib.recvDatalen-i)
        {
            return 0;
        }
    }
    
    return 2;
}

/*************************************************************
** 函数名称: NaviLcd_DisposeSendFixText
** 功能描述: 发送固定文本信息命令
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char NaviLcd_DisposeSendFixText(void)
{
    unsigned char buffer[50];
    unsigned char type,ID,status,len;
    ProtocolACK val_ack;
    u8 channel = CHANNEL_DATA_1|CHANNEL_DATA_2;
    ////////////////////////
    type = s_ucNaviLcdParseBuffer[0];
    ID = s_ucNaviLcdParseBuffer[1];
    status =s_ucNaviLcdParseBuffer[2];    
    if(FIXTEXT_TYPE_PHRASE == type)//固定短语
    {
        if(ID)ID--;
        len=strlen(c_apcFixPhrase[ID]);   
        memcpy(buffer,c_apcFixPhrase[ID],len);
        val_ack=RadioProtocol_OriginalDataUpTrans(channel,4,buffer,len);
    }
    else
    if(FIXTEXT_TYPE_EVENT == type)////事件报告
    {
        /////////发送事件报告到平台///////////////////
        val_ack=RadioProtocol_EvenReport(channel,ID);
        ///////////////////////////////
    }
    else
    if(FIXTEXT_TYPE_INFO == type)//信息点播
    {
        val_ack=RadioProtocol_InforamtionOrder(channel,ID,status);
        if(ACK_OK==val_ack)
        {
            //////////更改信息点播状态//////////////
            InfoDemandChangeStatus(ID,status);
            return 0;
        }      
    }
    //////////////////////
    if(ACK_OK==val_ack)
    {
        return 0;
    }
    else
    {
        return 2;
    }
}
/*******************************************************************************
** 函数名称: NaviLcd_CheckDailPhone
** 函数功能: 判断拨号是否允许
** 入口参数: 
** 出口参数: 无
** 返回参数: 0:拨号成功,0x02禁止呼出
** 全局变量: 无
** 调用模块: 拨号函数
*******************************************************************************/
unsigned char NaviLcd_CheckDailPhone(unsigned char *phone)
{
    unsigned char name[24];
    unsigned char flag;    
    flag=Public_CheckPhoneInformation(phone,name);//0：没有该电话号码的资料；1: 呼入；2: 呼出；3：呼入/呼出//joneming 2013.1.15
    if(1 == flag||0==flag)//存在，但属性只呼入
    {
        Public_PlayTTSVoiceStr("该号码禁止呼出!");//joneming 2013.1.15
        return 0x02;
    }
    else
    {
        /////////////////////////////
        strcpy((char *)s_stNVKey.Keybuff,(char *)phone);
        ///拨号函数/1是成功;0:失败
        flag=communication_CallPhone(phone,PHONE_NORMAL);
        if(flag)return 0;
        else return 0xFF;
    } 
}
/*******************************************************************************
**  函数名称  : NaviLcd_OriginalDataDownTrans
**  函数功能  : 
**  输    入  : 无				
**  输    出  : 
**  全局变量  : 
**  调用函数  : 
**  中断资源  : 
**  备    注  :
*******************************************************************************/
void NaviLcd_OriginalDataDownTrans(u8 *pBuffer, u16 BufferLen)
{
    PactSendCmdForNaviLcd(NAVILCD_CMD_PLATFORM_TRANS_DOWN,pBuffer,BufferLen);
}
/*******************************************************************************
**  函数名称  : NaviLcd_DisposeOriginalDataTrans
**  函数功能  : 
**  输    入  : 无				
**  输    出  : 
**  全局变量  : 
**  调用函数  : 
**  中断资源  : 
**  备    注  :
*******************************************************************************/
unsigned char NaviLcd_DisposeOriginalDataTrans(u8 *pBuffer, u16 BufferLen)
{
    ProtocolACK val_ack;
    u8 channel = CHANNEL_DATA_1|CHANNEL_DATA_2;
    val_ack = RadioProtocol_OriginalDataUpTrans(channel,pBuffer[0],pBuffer+1,BufferLen-1);
    if(ACK_OK==val_ack)
    {
        return 0;
    }
    else
    {
        return 2;
    }
}

/*************************************************************
** 函数名称: NaviLcd_DisponseLoadSensorTransToLcd
** 功能描述: Lcd透传给载重传感器指令
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcd_DisponseLcdTransToLoadSensor(void)
{
    #ifdef USE_SYS_CAR_LOAD
    CarLoad_DisposeLcdTransToLoadSensor(s_ucNaviLcdParseBuffer,s_stNaviLcdAttrib.recvDatalen);
    #endif
}
/*******************************************************************************
**  函数名称  : NaviLcd_DisposeCmdResponse
**  函数功能  : 
**  输    入  : 无				
**  输    出  : 
**  全局变量  : 
**  调用函数  : 
**  中断资源  : 
**  备    注  :
*******************************************************************************/
void NaviLcd_DisposeRecvDataResponse(void)
{
    static unsigned char sucSetPasswordflag=0;
    unsigned char result;
    unsigned short cmd;
    result = 0;
    ///////////////////////
    cmd = s_stNaviLcdAttrib.recvCmd;
    ///////////////////
    if(s_stNVTimer[NV_TIMER_STATUS].enabled != TIMER_ENABLE)
    {
        LZM_PublicSetOnceTimer(&s_stNVTimer[NV_TIMER_STATUS],PUBLICSECS(2),NaviLcd_SendWorkStatus);
    }    
    switch(cmd)
    {
        case NAVILCD_CMD_QUERY:
            if(0==sucSetPasswordflag)
            {
                sucSetPasswordflag = 1;
                NaviLcd_SetTerminalPassWordToLcd();
            }
            return ;//不需要使用通用应答回复
            //break;
         case NAVILCD_CMD_RESET:
            return ;//不需要使用通用应答回复
            //break;
        case NAVILCD_CMD_SET_BAUDRATE:
            return ;//不需要使用通用应答回复
            //break;
        case NAVILCD_CMD_DISPLAY:
            return ;//不需要使用通用应答回复
            //break;
        case NAVILCD_CMD_CALI_TIME:
            return ;//不需要使用通用应答回复
            //break;
        case NAVILCD_CMD_SET_LCD_PARAM:
            return ;//不需要使用通用应答回复
            //break;
        case NAVILCD_CMD_HEART_BEAT:
            NaviLcdSendCmdHeartBeat();
            s_stNaviLcdComm.responseFlag = 0;
            return ;//不需要使用通用应答回复
            //break;
        case NAVILCD_CMD_SYSTEM_UPDATE: 
            return ;//不需要使用通用应答回复
            //break;
        ////////////控制/设置命令/////////////////////////
        case NAVILCD_CMD_TERMINAL_CONTROL: 
            result=NaviLcd_DisposeTerminalControl();
            break;
        case NAVILCD_CMD_TERMINAL_SET_PARAM:
            NaviLcd_DisposeSetTerminalParamter();
            break;
        case NAVILCD_CMD_TERMINAL_QUERY_PARAM:
            NaviLcd_DisposeQueryTerminalParamter();
            s_stNaviLcdComm.responseFlag = 0;
            return;
            //break;
        case NAVILCD_CMD_QUERY_STATUS_ALARM:            
            NaviLcd_DisposeQueryStatusAlarm(); 
            s_stNaviLcdComm.responseFlag = 0;
            return;
        case NAVILCD_CMD_QUERY_WORK_STATUS:
            if(s_stNaviLcdAttrib.recvDatalen==0)
            NaviLcd_DisposeQueryWorkStatus();
            s_stNaviLcdComm.responseFlag = 0;
            return;
        case NAVILCD_CMD_MODIFY_PASSWORD:
            return;
        case NAVILCD_CMD_REGISTER_OUT_CITY:
            result=NaviLcd_DisposeOutCityRegister();
            break;
        case NAVILCD_CMD_EXPORT_U_DISK:
            NaviLcdSendCmdGeneralAck(s_stNaviLcdAttrib.recvCmd,0);
            NaviLcdDelayms(25);
            NaviLcd_DisposeExportDataToUsbDisk();
            return;
        case NAVILCD_CMD_CARLOAD_WEIGHT:
             NaviLcd_DisposeCarLoadWeight();
            return;
        case NAVILCD_CMD_SET_PJA_VALID:
            #ifdef __COMMPJA_H_
            CommPJA_PlayPromptStr();
            #endif
            return;
        case NAVILCD_CMD_PJA_RESULT:
            NaviLcd_DisposePJAResult();
            break;
        ////////////电话/////////////////////////////
        case NAVILCD_CMD_DIAL_PHONE:            
            s_ucNaviLcdParseBuffer[s_stNaviLcdAttrib.recvDatalen]='\0';
            #ifdef USE_PHONE_BOOK
            result=PhoneBook_CheckPhoneCallResult(s_ucNaviLcdParseBuffer,NULL);
            if(result)
            {
                result= 0x02;
            }
            #else
            result=NaviLcd_CheckDailPhone(s_ucNaviLcdParseBuffer);            
            #endif
            if(0 == result)
            {
                strcpy((char *)s_stNVKey.Keybuff,(char *)s_ucNaviLcdParseBuffer);
                s_stNaviLcdAttrib.newphone = 1;
                LZM_PublicSetOnceTimer(&s_stNVTimer[NV_TIMER_TEST],PUBLICSECS(70),NaviLcd_DisposeHangUpPhoneStatus);
            }
            break;
        case NAVILCD_CMD_NEW_CALL_PHONE:
            return ;//不需要使用通用应答回复
            //break;
        case NAVILCD_CMD_LCD_KEY_VALUE:
            result=NaviLcd_DisposeKeyValue(s_ucNaviLcdParseBuffer[0]);
            break;
        case NAVILCD_CMD_TERMINAL_KEY_VALUE:            
            return ;//不需要使用通用应答回复
        case NAVILCD_CMD_QUERY_PHONE_BOOK:
            NaviLcd_DisposeQueryPhoneBooks(s_ucNaviLcdParseBuffer[0]);
            s_stNaviLcdComm.responseFlag = 0;
            return ;//不需要使用通用应答回复
        case NAVILCD_CMD_ADJUST_VOLUME:
            NaviLcd_DisposeAdjustVolume();
            s_stNaviLcdComm.responseFlag = 0;
            return;
        //////////////信息服务//////////////////////
        case NAVILCD_CMD_TEXT_DISPLAY:
            return ;//不需要使用通用应答回复
            //break;
        case NAVILCD_CMD_QUESTION_DISPLAY:
            return ;//不需要使用通用应答回复
            //break;
        case NAVILCD_CMD_QUESTION_RESPONSE: 
            result=NaviLcd_DisposeQustionResponse();
            break;
        case NAVILCD_CMD_QUERY_FIX_TEXT:
            NaviLcd_DisposeQueryFixText(s_ucNaviLcdParseBuffer[0]);   
            s_stNaviLcdComm.responseFlag = 0;
            return;//不需要使用通用应答回复
            //break;
        case NAVILCD_CMD_SEND_FIX_TEXT: 
            result = NaviLcd_DisposeSendFixText();
            break;
        case NAVILCD_CMD_SEND_TICKLE_INFO:
            result = NaviLcd_DisposeSendTickleInfo();
            break;
        case NAVILCD_CMD_INFO_SERVICE:
             return ;//不需要使用通用应答回复
        //////////////电召服务//////////////////////
        case NAVILCD_CMD_CALL_TAXI_NEW:
            return ;//不需要使用通用应答回复
            //break;
        case NAVILCD_CMD_CALL_TAXI_VIE://抢标命令
            result = NaviLcd_DisposeCallTaxiVie();
            break;
        case NAVILCD_CMD_CALL_TAXI_DETAIL:
            return ;//不需要使用通用应答回复
            //break;
        case NAVILCD_CMD_CALL_TAXI_FINISH:
            result = NaviLcd_DisposeCallTaxiFinish();
            break;
        case NAVILCD_CMD_CALL_TAXI_CANCEL_TERMINAL:
            result = NaviLcd_DisposeTerminalCancelCallTaxi();
            break;
        case NAVILCD_CMD_CALL_TAXI_CANCEL_PLATFORM:
            return ;//不需要使用通用应答回复
            //break;
        /////////////驾驶员信息///////////////////////
        case NAVILCD_CMD_DRIVER_SIGN:
            NaviLcd_DisposeDriverSign();
            s_stNaviLcdComm.responseFlag = 0;
            return;
        case NAVILCD_CMD_DRIVER_ADD:
            NaviLcd_DisposeDriverInfoAdd();
            return;
        case NAVILCD_CMD_DRIVER_RECORD:
            NaviLcd_DisposeDriverRecord();
            s_stNaviLcdComm.responseFlag = 0;
            return;
        case NAVILCD_CMD_DRIVER_REPORT:
            break;
        case NAVILCD_CMD_DRIVER_QUERY_ALL:
            NaviLcd_DisposeQueryDriverInfo(); 
            s_stNaviLcdComm.responseFlag = 0;
            return;
        case NAVILCD_CMD_DRIVER_ADD_RESULT:            
            return;
        case NAVILCD_CMD_DRIVER_MODIFY://
            if(NaviLcdDriverLoginFlag())
            {
                //result = DriverInfoModifyDriverCodeAndPassword(s_stDriverLog.recordNumber,&s_ucNaviLcdParseBuffer[0],&s_ucNaviLcdParseBuffer[6]);               
            }
            else
            {
                Public_PlayTTSVoiceStr("请先登录再修改");
                result = 0x03;
            }
            break;
        case NAVILCD_CMD_DRIVER_DEL_ALL://
            DriverInfoDeleteAll();
            break;
        case NAVILCD_CMD_DRIVER_BRUSH_CARD: 
            NaviLcdSendCmdGeneralAck(s_stNaviLcdAttrib.recvCmd,0);
            NaviLcdDelayms(20);
            NaviLcd_DisposeBrushCard();
            return;
        case NAVILCD_CMD_WRITE_IC_CARD:
            NaviLcd_DisposeWriteICcardResult(s_ucNaviLcdParseBuffer[0]);
            return;
        case NAVILCD_CMD_READ_IC_CARD:
            NaviLcd_DisposeReadICcardData();
            return;
        case NAVILCD_CMD_CURRENT_DRIVER_INFO:
            return;
        ///////////////信息透传///////////////////////////////
        case NAVILCD_CMD_PLATFORM_TRANS_DOWN:
            return;
        case NAVILCD_CMD_LCD_TRANS_UP:
            result=NaviLcd_DisposeOriginalDataTrans(s_ucNaviLcdParseBuffer,s_stNaviLcdAttrib.recvDatalen);
            break;
        case NAVILCD_CMD_TERMINAL_TRANS_TO_ICCARD://终端数据透传给刷卡模块
            return;
         case NAVILCD_CMD_ICCARD_TRANS_TO_TERMINAL://刷卡模块数据透传给终端命令
            NaviLcd_DisponseICCardTransDataToTerminal(s_ucNaviLcdParseBuffer,s_stNaviLcdAttrib.recvDatalen);
            break;
        case NAVILCD_CMD_TERMINAL_TRANS_TO_CTRL_UNIT://终端数据透传给刷卡模块
            return;
         case NAVILCD_CMD_CTRL_UNIT_TRANS_TO_TERMINAL://刷卡模块数据透传给终端命令
            NaviLcd_DisponseCtrlUnitTransDataToTerminal(s_ucNaviLcdParseBuffer,s_stNaviLcdAttrib.recvDatalen);
            break;
        case NAVILCD_CMD_LCD_TRANS_TO_LOAD_SENSOR://LCD透传给载重传感器指令
            NaviLcd_DisponseLcdTransToLoadSensor();
            break;
        case NAVILCD_CMD_LOAD_SENSOR_TRANS_TO_LCD://载重传感器给透传LCD指令
            return;
        ///////////////导航/////////////////////
        case NAVILCD_CMD_NAVIGATION:
            return;
        case NAVILCD_CMD_ONE_KEY_NAVIGATION:
            NaviLcd_DisponseOneKeyNavigation(s_ucNaviLcdParseBuffer,s_stNaviLcdAttrib.recvDatalen);
            break;
        case NAVILCD_CMD_GNSS_CHECK_UP:
            NaviLcd_DisposeGNSSCheckUp(s_ucNaviLcdParseBuffer[0]);
            break;
        default:
            break;
    }
    NaviLcdSendCmdGeneralAck(cmd,result);
}
/*************************************************************OK
** 函数名称: NaviLcdSaveDriverLogin
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcdSaveDriverLogin(unsigned char recordnumber) 
{
    s_stDriverLog.statusFlag = NV_DRIVERLOG_STATUS_LOGIN;
    s_stDriverLog.recordNumber =recordnumber;
}
/*************************************************************OK
** 函数名称: NaviLcdSaveDriverLogin
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcdSaveDriverLogout(void) 
{
    s_stDriverLog.statusFlag = NV_DRIVERLOG_STATUS_LOGOUT;
    s_stDriverLog.recordNumber = 0;
}
/*************************************************************OK
** 函数名称: NaviLcdDriverLoginFlag
** 功能描述: 驾驶员登录标志
** 入口参数: 			 
** 出口参数: 为1已登录,为0未登录
** 返回参数: :
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char NaviLcdDriverLoginFlag(void)
{
    //return (s_stDriverLog.statusFlag == NV_DRIVERLOG_STATUS_LOGIN)?1:0;
    return TiredDrive_GetLoginFlag();
}
/*************************************************************OK
** 函数名称: GetCurrentDriverLoginRecordNumber
** 功能描述: 取得当前驾驶员记录号
** 入口参数: 			 
** 出口参数: 0未登录或不临时驾驶员,未0记录号
** 返回参数: :
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char GetCurrentDriverLoginRecordNumber(void)
{
    if(NaviLcdDriverLoginFlag())
    {
        return s_stDriverLog.recordNumber;
    }
    ////////////////////////
    return 0;
}
/*************************************************************OK
** 函数名称: CheckTiredDriverAutoLog
** 功能描述: 检查驾驶员是否登录，已登录，则使用上次的
** 入口参数: 			 
** 出口参数: 
** 返回参数: :
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void CheckTiredDriverAutoLog(void)
{
    unsigned char drivercode[8] = {0};
    unsigned char num;
    unsigned long driverID;
    EepromPram_ReadPram(E2_CURRENT_DRIVER_ID, drivercode);
    if(drivercode[0] == 0xFF && drivercode[1] == 0xFF && drivercode[2] == 0xFF)//如果是无人登录的话每次上电都默认为0
    {
        NaviLcd_DriverLogout();
    }
    else 
    {
        driverID = 0;
        driverID |= drivercode[0]<<16;
        driverID |= drivercode[1]<<8;
        driverID |= drivercode[2];
        sprintf((char *)drivercode,"%06d",driverID);
        //////////////////////////
        num=GetDriverInfoRecordNumber(drivercode);
        SaveCurrentDriverParameter(num);
        ////////////////////////////
        TiredDrive_Login();
        /////////////////////
        NaviLcdSaveDriverLogin(num);
    }
}
/*************************************************************OK
** 函数名称: NaviLcd_RadioProtocolQuestionDownload
** 功能描述: 提问下发
** 入口参数: 			 
** 出口参数: 
** 返回参数: :
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcd_RadioProtocolQuestionDownload(u8 *pBuffer, u16 BufferLen)
{
    #ifndef USE_NAVILCD_TTS
    u8      QuestionFlag;//标志
    u8      QuestionLen;//问题内容长度
    u8      AnswerID;//答案ID
    u16      AnswerLen;//答案长度
    u8      *p;
    u16     length;
    s16     TotalLength;
    u8      HighByte;
    u8      LowByte;
    u8      Buffer[20];
    u8      PramLen;
    u8	flag = 0;//错误标志    
    TotalLength = BufferLen;

    p = pBuffer;
    TotalLength -= 2;
    QuestionFlag = *p++;//提问下发标志

    if(0 == QuestionFlag)
    {

    }
    //问题内容长度
    QuestionLen = *p++;
    length = 0;

    memcpy(RadioShareBuffer+length,"问题：",6);
    length += 6;
    memcpy(RadioShareBuffer+length,p,QuestionLen);
    length += QuestionLen;
    p += QuestionLen;
    TotalLength -= QuestionLen;
    memcpy(RadioShareBuffer+length,"答案：",6);
    length += 6;
    while(TotalLength > 0)
    {
        //答案ID
        AnswerID = *p++;
        TotalLength--;

        sprintf((char *)Buffer," %d、",AnswerID);
        PramLen = strlen((const char *)Buffer);
        if((PramLen+length) >= RADIO_PROTOCOL_BUFFER_SIZE)
        {
            flag = 1;
            break;
        }
        memcpy(RadioShareBuffer+length,Buffer,PramLen);
        length += PramLen;
        //答案长度
        AnswerLen = 0;
        HighByte = *p++;
        LowByte = *p++;
        AnswerLen = (HighByte << 8)|LowByte;
        TotalLength -= 2;
        //答案内容
        if((AnswerLen+length) >= RADIO_PROTOCOL_BUFFER_SIZE)
        {
            flag = 1;
            break;
        }
        memcpy(RadioShareBuffer+length,p,AnswerLen);
        length += AnswerLen;
        TotalLength -= AnswerLen;
        p += AnswerLen;
    }
    if(0 == flag)
    {
        RadioShareBuffer[length] ='\0';
        Public_PlayTTSVoiceStr(RadioShareBuffer);
        NaviLcdSendCmdQuestionDisplay(1,pBuffer+1,BufferLen-1);
        
    }
    #else
    NaviLcdSendCmdQuestionDisplay(0,pBuffer+1,BufferLen-1);
    #endif
}
/*******************************************************************************
**  函数名称  : NaviLcd_OpenBrushCard
**  函数功能  : 开启刷卡功能  
**  输    入  : 无				
**  输    出  : 
**  全局变量  : 
**  调用函数  : 
**  中断资源  : 
**  备    注  :
*******************************************************************************/
void NaviLcd_OpenBrushCard(void)
{
  
}
/*******************************************************************************
**  函数名称  : NaviLcd_CloseBrushCard
**  函数功能  : 关闭刷卡功能  
**  输    入  : 无				
**  输    出  : 
**  全局变量  : 
**  调用函数  : 
**  中断资源  : 
**  备    注  :
*******************************************************************************/
void NaviLcd_CloseBrushCard(void)
{
  
}
/*******************************************************************************
**  函数名称  : NaviLcd_DisposeBrushCardDataForDriverInfo
**  函数功能  : 处理刷卡数据
**  输    入  : 无				
**  输    出  : 
**  全局变量  : 
**  调用函数  : 
**  中断资源  : 
**  备    注  :
*******************************************************************************/
void NaviLcd_DisposeBrushCardDataForDriverInfo(unsigned char *data,unsigned short datalen)
{
    unsigned char len;
    unsigned char buffer[30];
    if(NV_CMD_FLAG_ADD==s_stDriverLog.cmdFlag)//刷卡添加/修改驾驶员资料
    {
        #if 0
        if(0xAA != s_stDriverLog.recordindex)
        {
            s_stDriverLog.resutltindex = s_stDriverLog.recordindex;
            s_stDriverLog.result = DriverInfoAddOrModify(&s_stDriverLog.resutltindex,s_stDriverLog.drivercode,s_stDriverLog.password,data,datalen);
        }
        else
        {
            s_stDriverLog.resutltindex = 5;
            s_stDriverLog.result = 1; 
            Public_PlayTTSVoiceStr("请先输入工号密码"); 
        }
        NaviLcd_DisposeDriverInfoAddInfo();
        #endif
    }
    else
    if(NV_CMD_FLAG_LOGIN==s_stDriverLog.cmdFlag)//刷卡签到
    {
        len = 0;
        buffer[len++] = 0;//签到/签退标志
        buffer[len++] = 0;//成功
        ICCard_DisposeExternalDriverSignIn(data,datalen);
        len += ICCard_GetExternalDriverInfoForSignIn(&buffer[len]);
        if(len<3)
        {
            buffer[1] = 1;//失败
        }
        NaviLcdSendDriverSignResult(buffer,len);
    }
    else
    if(NV_CMD_FLAG_LOGOUT==s_stDriverLog.cmdFlag)//刷卡签退
    {
        buffer[0]= 1;//签到/签退标志
        if(NaviLcdDriverLoginFlag())
        {
            ICCard_DisposeExternalCardDriverSignOut();            
        }
        //////////////////
        buffer[1]= 0;//成功
        NaviLcdSendDriverSignResult(buffer,2);
    }
    else
    if(NV_CMD_FLAG_BUSINESS==s_stDriverLog.cmdFlag)//业务上报
    {
        //RadioProtocol_DriverInformationReport(data,len);//
    }
}

/*************************************************************
** 函数名称: NaviLcd_DisposePhoneStatus
** 功能描述: 导航屏处理电话状态命令
** 入口参数: phone:电话号码,status:电话状态：0：为挂断;1:接通；2:新来电
** 出口参数: 无
** 返回参数: 如果是电话呼入状态为新来电时,返回值:0:直接拒接来电;1:正在等待用户接听来电
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char NaviLcd_DisposePhoneStatus(unsigned char *phone,unsigned char status)
{
    unsigned char result;
    result =0;
    switch(status)
    {
        case 0://挂机
            s_stNaviLcdAttrib.newphone = 0;
            NaviLcdActiveSendKeyValue(NAVILCD_KEYVALUE_HANG_UP);            
            break;
        case 1://通话
            s_stNaviLcdAttrib.newphone = 2;
            LZM_PublicSetOnceTimer(&s_stNVTimer[NV_TIMER_TEST],PUBLICSECS(5),NaviLcd_RecvKeyTimeOut);
            NaviLcdActiveSendKeyValue(NAVILCD_KEYVALUE_ANSWER_CALL);
            break;
        case 2://新来电
            if(!s_stNaviLcdAttrib.newphone)
            result=NaviLcdSendCmdNewCallPhone(phone);
            break;
    }
    return result;
}
/*************************************************************
** 函数名称: NaviLcd_DisposeHangUpPhoneStatus
** 功能描述: 主动打电话，超时处理
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcd_DisposeHangUpPhoneStatus(void)
{
    if(1== s_stNaviLcdAttrib.newphone)
    NaviLcd_HangUpPhone();
}
/*************************************************************
** 函数名称: NaviLcd_TimerTask
** 功能描述: 内部定时器处理函数
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
FunctionalState NaviLcd_TimerTask(void)
{
    /////////////////////
    NaviLcdAutoToDisposeRecvData();//处理串口缓存数据
    ////////////////////////////////////
    LZM_PublicTimerHandler(s_stNVTimer,NV_TIMERS_MAX);
    ///////////////////////////////////
    return ENABLE;
}
/*************************************************************
** 函数名称: NaviLcd_AccStatusOff
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcd_AccStatusOff(void)
{
    unsigned char i;
    for(i = NV_TIMER_RESEND; i < NV_TIMERS_MAX; i++)
    {
        LZM_PublicKillTimer(&s_stNVTimer[i]);
    }
    //if(1 == ReadPeripheral2TypeBit(15))//bit15:调度屏,1为接入了,0为没接,dxl,2014.12.30
    
#if (ICCARD_JTD == ICCARD_SEL)
    {
        NaviLcdPowerOff();
    }
#endif
}
/*************************************************************
** 函数名称: NaviLcd_AccStatusOn
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcd_AccStatusOn(void)
{
    NaviLcdCommParamInit();
    NaviLcdPowerOn();
    LZM_PublicSetOnceTimer(&s_stNVTimer[NV_TIMER_TEST],PUBLICSECS(15),NaviLcdSendCmdQueryStatus);    
}
/*************************************************************
** 函数名称: NaviLcd_AccStatusOn
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcd_AccStatusTurnPromptDriver(void)
{
    #if 0
    unsigned char ACC;
    if(1 != s_stNaviLcdAttrib.VoicePrompt)return;
    if(0==s_stNaviLcdAttrib.AccTurnPrompt)return;
    ////////////////////////////////
    s_stNaviLcdAttrib.AccTurnPrompt =0;
    /////////////////////////////////
    if(0== s_stNaviLcdAttrib.onlinestaus)return;
    //////////////////////////////////////
    ACC = Io_ReadStatusBit(STATUS_BIT_ACC);    
    if(0==ACC)//下班签退
    {
        if(NaviLcdDriverLoginFlag())
        {
            #ifdef TexiService_TaximeterUseMode
            if(1 == TexiService_TaximeterUseMode())
            {
                Public_PlayTTSVoiceStr("请先按计价器IC键,再刷司机卡下班签退"); 
            }
            else
            #endif
            {
                Public_PlayTTSVoiceStr("请刷司机卡下班签退");
            }
        }
    }
    else//上班签到
    {
        if(!NaviLcdDriverLoginFlag())
        {
            #ifdef TexiService_TaximeterUseMode
            if(1==TexiService_TaximeterUseMode())
            {
                Public_PlayTTSVoiceStr("请先刷司机卡,再按计价器IC键上班签到");
            }
            else
            #endif
            {
                Public_PlayTTSVoiceStr("请刷司机卡上班签到");
            }
        }
    }
    #endif
}

/*************************************************************
** 函数名称: NaviLcd_CheckACCStatus
** 功能描述: 检查ACC当前状态
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcd_CheckACCStatus(void)
{
    unsigned char ACC;    
    ACC=Io_ReadStatusBit(STATUS_BIT_ACC);
    if(ACC)//acc开
    {
        if(s_stNaviLcdAttrib.LcdPowerOn != 1)
        {
            s_stNaviLcdAttrib.LcdPowerOn =1;
            NaviLcd_AccStatusOn();
			ICCard_Send_CMD_To_Card_Reader_0x03(0);//退出省点模式    
        }
        s_stNaviLcdAttrib.AccOffCnt = 0;
    }
    else//acc关
    {
        ////////////////////////
        if(s_stNaviLcdAttrib.LcdPowerOn != 0)
        {
            if(s_stNaviLcdAttrib.AccOffCnt<65)//65*4
            {
                s_stNaviLcdAttrib.AccOffCnt++;
            }
            else
            {
                s_stNaviLcdAttrib.LcdPowerOn =0;
                s_stNaviLcdAttrib.AccOffCnt = 0;
                NaviLcd_AccStatusOff();
				ICCard_Send_CMD_To_Card_Reader_0x03(1);//进入省点模式    
            }
        }
    }
}

/*************************************************************
** 函数名称: NaviLcd_CheckACCStatus
** 功能描述: 检查ACC当前状态
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcd_StartCheckACCStatus(void)
{
    NaviLcd_CheckACCStatus();
    LZM_PublicSetCycTimer(&s_stNVTimer[NV_TIMER_CHECK_ACC],PUBLICSECS(4),NaviLcd_CheckACCStatus);
}
/*************************************************************
** 函数名称: NaviLcd_ParameterInitialize
** 功能描述: 参数初始化
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcd_ParameterInitialize(void)
{  
    
    LZM_PublicKillTimerAll(s_stNVTimer,NV_TIMERS_MAX);
    memset(&s_stDriverLog,0,sizeof(STDRIVERLOG));
    s_stDriverLog.statusFlag = NV_DRIVERLOG_STATUS_EMPTY;
    s_stDriverLog.cmdFlag =NV_CMD_FLAG_EMPTY; 
    s_stNaviLcdAttrib.newphone = 0;
    s_stNaviLcdAttrib.logout =0;    
    s_stDriverLog.recordindex = 0xAA;
    s_stNaviLcdAttrib.LcdPowerOn = 0;
    s_stNaviLcdAttrib.AccOffCnt = 0;
    s_stNaviLcdAttrib.onlinestaus = 0;
    #ifdef E2_UNLOGIN_TTS_PLAY_ID
    if(!EepromPram_ReadPram(E2_UNLOGIN_TTS_PLAY_ID, &s_stNaviLcdAttrib.VoicePrompt))
    #endif
    {
        s_stNaviLcdAttrib.VoicePrompt = 0;
    }
    NaviLcdCommParamInit();    
    NaviLcdPowerIoInit();
    //if(1 == ReadPeripheral2TypeBit(15))//bit15:调度屏,1为接入了,0为没接,dxl,2014.12.30
    
#if (ICCARD_JTD == ICCARD_SEL)
    {
        NaviLcdPowerOff();
    }
#endif
    SetTimerTask(TIME_NVLCD, LZM_TIME_BASE);
    /////////////////////////
    //以下加入外设类型判断，若没有使用外接调度屏，则直接返回,dxl,2014.12.26
    //if(0 == ReadPeripheral2TypeBit(15))//外设2类型的bit15为调度屏,1:接了;0:没接
#if (ICCARD_JTD != ICCARD_SEL)
	{
        return ;
    }
#endif
    /////// dxl,2014.12.26 end//////////
    NaviLcd_CheckACCStatus();   
    //////////////////////
    LZM_PublicSetOnceTimer(&s_stNVTimer[NV_TIMER_CHECK_ACC],PUBLICSECS(1),NaviLcd_StartCheckACCStatus);
}

/*************************************************************
** 函数名称: NaviLcd_WriteDriverInfoToICcard
** 功能描述: 把相关驾驶员信息写到IC中
** 入口参数: recordNumber:驾驶员存储编号(从1开始)
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcd_WriteDriverInfoToICcard(unsigned char recordNumber)
{
    unsigned char buffer[200];
    unsigned char datalen;
    memset(buffer,0,200);
    datalen=GetDriverInfo(recordNumber,buffer,0);
    if(0==datalen)return ; //无相应记录
    NaviLcdSendDataToWriteICcard(0,0,buffer,datalen);
}
/*************************************************************
** 函数名称: NaviLcd_DebugToWriteICcard
** 功能描述:
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcd_DebugToWriteICcard(void)
{
    NaviLcd_WriteDriverInfoToICcard(1);
}
/*************************************************************
** 函数名称: NaviLcd_SetTerminalPassWordToLcd
** 功能描述:
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcd_SetTerminalPassWordToLcd(void)
{
    unsigned char buffer[5];
    if(!EepromPram_ReadPram(E2_PASSWORD_ID,buffer))
    {
        Public_ConvertLongToBuffer(701000,buffer);
        EepromPram_WritePram(E2_PASSWORD_ID,buffer,E2_PASSWORD_ID_LEN);
    }
    ////////////////////////////////////
    NaviLcdSendCmdModifyPassWord(buffer);
}
/*************************************************************
** 函数名称: NaviLcd_SendCmdTerminalTransDataToICCard
** 功能描述: 终端数据透传给刷卡模块
** 入口参数: data需要发送的数据首地址,datalen:需要发送的数据长度
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcd_SendCmdTerminalTransDataToICCard(unsigned char *data,unsigned short datalen)
{
    PactSendCmdForNaviLcd(NAVILCD_CMD_TERMINAL_TRANS_TO_ICCARD,data,datalen);
}
/*************************************************************
** 函数名称: NaviLcd_DisponseICCardTransDataToTerminal
** 功能描述: 刷卡模块透传给终端数据
** 入口参数: data需要发送的数据首地址,datalen:需要发送的数据长度
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcd_DisponseICCardTransDataToTerminal(unsigned char *data,unsigned short datalen)
{
    ///////////////////
}
/*************************************************************
** 函数名称: NaviLcd_SendCmdDataToCtrlUnit
** 功能描述: 终端透传给控制板指令
** 入口参数: data需要发送的数据首地址,datalen:需要发送的数据长度
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcd_SendCmdDataToCtrlUnit(unsigned char *data,unsigned short datalen)
{
    PactSendCmdForNaviLcd(NAVILCD_CMD_TERMINAL_TRANS_TO_CTRL_UNIT,data,datalen);
}
/*************************************************************
** 函数名称: NaviLcd_SendCmdTerminalTransDataToCtrlUnit
** 功能描述: 终端透传给控制板指令
** 入口参数: ID类型ID,operate:操作
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcd_SendCmdTerminalTransDataToCtrlUnit(unsigned char ID,unsigned char operate)
{
    unsigned char len;
    unsigned char data[4];
    len =0;
    data[len++] = ID;
    data[len++] = operate;
    data[len++] = 0;//保留
    NaviLcd_SendCmdDataToCtrlUnit(data,len);
}
/*************************************************************
** 函数名称: NaviLcd_DisponseCtrlUnitTransDataToTerminal
** 功能描述: 控制板透传给终端指令
** 入口参数: data需要发送的数据首地址,datalen:需要发送的数据长度
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcd_DisponseCtrlUnitTransDataToTerminal(unsigned char *data,unsigned short datalen)
{
    ///////////////////
}
/*************************************************************
** 函数名称: NaviLcd_DisponseLoadSensorTransToLcd
** 功能描述: 载重传感器透传给Lcd指令
** 入口参数: data需要发送的数据首地址,datalen:需要发送的数据长度
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcd_DisponseLoadSensorTransToLcd(unsigned char *data,unsigned short datalen)
{
    PactSendCmdForNaviLcd(NAVILCD_CMD_LOAD_SENSOR_TRANS_TO_LCD,data,datalen);
}

/*************************************************************
** 函数名称: NaviLcd_DisponseCtrlUnitTransDataToTerminal
** 功能描述: 一键导航
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcd_DisponseOneKeyNavigation(unsigned char *data,unsigned short datalen)
{
    ///////////////////
}

void NaviLcd_SetCurrentPJAValid(void)
{
	 #ifdef __COMMPJA_H_
    unsigned char data[2]={0};
    if(!NaviLcd_GetCurOnlineStatus())return;
    PactSendCmdForNaviLcd(NAVILCD_CMD_SET_PJA_VALID,data,1);
	#endif
}
void NaviLcd_DisposePJAResult(void)
{
	 #ifdef __COMMPJA_H_
    unsigned char result;
    result=s_ucNaviLcdParseBuffer[1]; 
    CommPJA_SetCurrentPjaResult(result);    
	#endif
}
//////////////////////////////
/*************************************************************
** 函数名称: NaviLcd_DisposeGNSSCheckUp
** 功能描述:处理北斗送检导航屏指令
** 入口参数: cmd命令
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void NaviLcd_DisposeGNSSCheckUp(unsigned char cmd)
{
    u8 Buffer[100];
    u16 length;
     u8 channel = CHANNEL_DATA_1|CHANNEL_DATA_2;
    switch(cmd)
    {
        case NV_GNSS_CHECK_TALK_PHONE:      //拨打电话 
            ///////请填入需要拨打的电话//////////
            communication_CallPhone("13800138000",PHONE_NORMAL);
            break;
        case NV_GNSS_CHECK_ANSWER_PHONE:    //接听电话
            communication_CallPhone("0",PHONE_TALK);
            break;
        case NV_GNSS_CHECK_HANG_UP_PHONE:    //挂断电话
            communication_CallPhone("13800000000",PHONE_END);
            break;
        case NV_GNSS_CHECK_START_RECORD:     //开始录音      
            break;
        case NV_GNSS_CHECK_STOP_RECORD:      //停止录音    
            break;
        case NV_GNSS_CHECK_FIX_PHRASE:       //发送固定短语
            break;
        case NV_GNSS_CHECK_RTU_REGISTER:     //终端注册
            RadioProtocol_TerminalRegister(channel);  
            break;
        case NV_GNSS_CHECK_RTU_QUALIFY:      //终端鉴权
            RadioProtocol_TerminalQualify(CHANNEL_DATA_1);//只能一个一个通道发，因为通道不一样鉴权码也不一样      
            RadioProtocol_TerminalQualify(CHANNEL_DATA_2);
            break;
        case NV_GNSS_CHECK_POSITION_INFO:    //位置信息汇报
            Public_GetCurBasicPositionInfo(Buffer);
            length = 28;
            RadioProtocol_PostionInformationReport(channel,Buffer,length); 
            break;
        case NV_GNSS_CHECK_EVENT_REPORT:     //事件报告 
            RadioProtocol_EvenReport(channel,0x01);
            break;
        case NV_GNSS_CHECK_INFO_DEMAND:      //信息点播   
            RadioProtocol_InforamtionOrder(channel,1, 1);
            break;
        case NV_GNSS_CHECK_CANCEL_IOD:       //取消信息点播
            RadioProtocol_InforamtionOrder(channel,1, 0);
            break;
        case NV_GNSS_CHECK_E_WAYBILL:        //电子运单上报 
            memcpy(Buffer+4,"电子运单",8);
            Buffer[0] = 0;
            Buffer[1] = 0;
            Buffer[2] = 0;
            Buffer[3] = 8;
            RadioProtocol_ElectricReceiptReport(channel,Buffer, 12);
            break;
        case NV_GNSS_CHECK_DRIVER_INFO:      //驾驶员身份信息      
            break;
        case NV_GNSS_CHECK_MULIT_EVENT:      //上发多媒体事件
            RadioProtocol_MultiMediaEvenReport(channel,0x01, 0, 0, 1, 1);
            break;
        case NV_GNSS_CHECK_DTU_DATA:         //数据上行透传 
            RadioProtocol_OriginalDataUpTrans(channel,1, Buffer, 10);
            break;
        case NV_GNSS_CHECK_RESERVE1:         //保留操作1  
            break;
        case NV_GNSS_CHECK_RESERVE2:         //保留操作2
            break;
        case NV_GNSS_CHECK_RESERVE3:         //保留操作3  
            break;        
    }
}
#endif
/******************************************************************************
**                            End Of File
******************************************************************************/

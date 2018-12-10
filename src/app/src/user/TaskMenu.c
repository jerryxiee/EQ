/*******************************************************************************
* File Name:			TaskMenu.h 
* Function Describe:	 
* Relate Module:		
* Writer:				joneming
* Date:				2012-01-5
* Rewriter:
* Date:
*******************************************************************************/
/****************************************************/
/*                                                  */
/* Included files                                   */
/*                                                  */
/****************************************************/ 
#include "include.h"
#include<time.h>
/////////////////////////////////////////////////
static void (*TaskTo)(void);
static void (*s_fptrDoit)(void);
static void (*s_fptrCancel)(void);
LZM_RET (*PasswordPreTaskFunction)(LZM_MESSAGE *Msg);//正确响应密码输入任务之前的任务
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

ST_ESPIN s_stESpin[4];
ST_SPIN s_stSpin[4];

#define AUTORUN_MAIN_TIME 900//3min//60*5
#define LCD_TIMEOUT 300//1min=60*5
#define REFLASH_TIME 6//0.3
#define REINIT_LCD_TIME 1200//s 20分钟

static u8 MenuSwitchFlg =0;
/*************************************************************
** 函数名称: TaskMainInterfaceSelection
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
** 作者    :fanqinghai
*************************************************************/  
LZM_RET TaskMainInterfaceSelection(LZM_MESSAGE *Msg);

///////////////////////////////////////////////////////////////////////////////
typedef struct
{
  unsigned long SysPassword;
  unsigned long InPassword;
  unsigned long timeVal;
  unsigned short LcdTimeOutCnt;
  unsigned short AutoRunCnt;
  unsigned short textlen;
  unsigned char FlashBacklightFlag;//
  unsigned char BacklightFlag;
  unsigned char ControlMaxId;
  unsigned char ControlId;
  unsigned char index;
  unsigned char select;
  unsigned char step;
  unsigned char answerNum;    
  unsigned char status[4];    //
  unsigned char ID[4];        //
  char ShowBuffer[4][17];    
}STTASKMENU_ATTRIB;

STTASKMENU_ATTRIB s_stTaskAttrib;

typedef enum 
{
  MENUMAIN_DRIVE_RECORD,
  MENUMAIN_INFO_SERVER,
#ifdef USE_PHONE_BOOK
MENUMAIN_PHONE_SERVER,
#endif
MENUMAIN_DRIVE_MILE,
MENUMAIN_PRODUCT_INFO,
MENUMAIN_SIM_NUM,  
MENUMAIN_TTS,
MENUMAIN_SYSTEM,
MENUMAIN_MAX
}eSTMENUMAIN;

////////////////////////////////////////////////
struct SUT_MENU s_sutMenuMain;
const char *s_apcMenuMain[]= 
{
  "行驶记录",
  "信息服务",
#ifdef USE_PHONE_BOOK
"电话服务",
#endif
"驾驶里程",    
"产品信息",
"SIM卡信息", 
"TTS音量", 
"系统维护",
""
};

////////////////////////////////////////////////
typedef enum 
{
  MENU_INFO_TEXT_INFO,        //"文本信息",
  MENU_INFO_QUESTION_RESPONSE,//"提问应答",
  MENU_INFO_INFO_SERVICE,     //"点播信息",    
  MENU_INFO_EVENT_REPORT,     //"事件报告",
  MENU_INFO_RESERVE_SERVICE,  //"信息点播",    
  MENU_INFO_CAR_LOAD_STATUS,  //"车辆载货状态",
  MENU_INFO_MAX
}eSTMENUINFO;
struct SUT_MENU s_sutMenuInfoServer;
const char *s_apcMenuInfoServer[]= 
{
  "文本信息",
  "提问应答",    
  "点播的信息",
  "事件报告", 
  "信息点播",    
  "车辆载货状态",
  ""
};

typedef enum 
{
  MENUDRIVERECORD_CAR_INFO,
  MENUDRIVERECORD_DRIVER_INFO,
  MENUDRIVERECORD_TIREDDRIVE_RECORD,
  MENUDRIVERECORD_15MIN_SPEED,
  MENUDRIVERECORD_MAX
}eSTMENUDRIVERECORD;
struct SUT_MENU s_sutMenuDriveRecord;
const char *s_apcMenuDriveRecord[]= 
{
  "车辆信息",
  "驾驶员信息",
  "超时驾驶记录", 
  "15分钟速度",
  ""
};

//_____________________________________
struct SUT_MENU s_sutMenuSystemTest;
const char *s_apcMenuSystemTest[] = 
{
  "终端注册",         //0
  "终端鉴权",         //1
  //"终端注销",         //2
  "位置信息汇报",     //3
  "事件报告",         //4
  "信息点播",         //5
  "信息取消",         //6
  "电子运单上报",     //7
  "驾驶员身份信息",   //8
  "上发多媒体事件",   //9
  "数据上行透传",     //10
  "出厂未使用状态",
  ""
};

struct SUT_MENU s_sutMenuShortMsg;
const char *s_apcMenuShortMsg[] = 
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

struct SUT_MENU s_sutMenuSystem;
const char *s_apcMenuSystem[] = 
{
  "终端注册",         //0
  "终端鉴权",         //1
  "设置背光",         //    
  "高级设置",         //
  "主界面选择", 
  ""
};
typedef enum 
{
  MENU_SYSTEM_REGISTER,         //"终端注册"
  MENU_SYSTEM_QUALIFY,          //"终端鉴权"
  MENU_SYSTEM_BACKLIGHT,        //"设置背光"
  MENU_SYSTEM_ADVANCED,         //"高级设置"
  MENU_SYSTEM_MAIN_SELECT,      //"主界面选择"
  //MENU_SYSTEM_BASE_SET,         //"耕深基准设置"
  MENU_SYSTEM_MAX,              //最大界面
}eSTMENUSYSTEM;

struct SUT_MENU s_sutMenuMainInterfaceSet;
const char *s_apcMenuMainInterfaceSet[] = 
{
  "常规主界面",
  //"温度主界面",    
  "速度主界面",
  //"载重主界面",
  //"耕深主界面",
  //"转速主界面",         //0
  ""
};
typedef enum 
{
  MENU_INTERFACE_CONVENTION,           //常规界面
  //MENU_INTERFACE_TEMPERATURE,          //温度界面
  MENU_INTERFACE_SPEED,                //速度界面
  //MENU_INTERFACE_LOAD,                 //载重界面
  //MENU_INTERFACE_DEEP,                 //深耕界面
  //MENU_INTERFACE_TURNSPEED,            //转速界面
  MENU_INTERFACE_MAX,                  //最大界面
}eSTMENUSELECT;

struct SUT_MENU s_sutMenuSystemAdvanced;
const char *s_apcMenuSystemAdvanced[] = 
{
  "车辆信息设置",
  "服务器设置",    
  "安装调试",
  "终端参数设置",
  "开户加密设置",
  "使用前锁定一",
  "使用前锁定二", 
#ifdef CALIB_OIL_WEAR
"油量标定",
#endif
""
};

typedef enum 
{
  MENU_SYS_CAR_INFO,          //"车辆信息"
  MENU_SYS_SERVER_INFO,       //"服务器参数"
  MENU_SYS_DEBUG,             //"安装调试"
  MENU_SYS_TERMINAL_PARAM,    //"终端参数"
  MENU_SYS_ENCRYPT,
  MENU_SYS_LOCK_ONE,
  MENU_SYS_LOCK_TWO,    
#ifdef CALIB_OIL_WEAR
MENU_SYS_OIL_DEMARCATE,     //"油量标定"
#endif
MENU_SYS_MAX
}eSTMENUSYS;

struct SUT_MENU s_sutMenuSystemCarInfo;
const char *s_apcMenuSystemCarInfo[] = 
{
  "设置车牌号码",
  "设置车辆VIN",    
  "设置车牌颜色",
  "设置车牌分类",
  "设置初始里程",
  "设置脉冲系数",
  "设置省域ID",
  "设置市域ID",
  "车主手机号",
  "车辆发动机号",
  ""
};

typedef enum 
{
  MENU_CAR_SET_PLATE_NUM,     //"设置车牌号码"
  MENU_CAR_SET_VIN_NUM,       //"设置车辆VIN",
  MENU_CAR_SET_PLATE_COLOR,   //"设置车牌颜色"
  MENU_CAR_SET_PLATE_TYPE,    //"设置车牌分类"
  MENU_CAR_SET_TOTAL_MILE,    //"设置初始量程", 
  MENU_CAR_SET_FEATUER_COEF,  //"设置脉冲系数",
  MENU_CAR_SET_PROVINCE_ID,   //"设置省域ID",
  MENU_CAR_SET_CITY_ID,       //"设置市域ID",
  MENU_CAR_SET_OWNER_PHONE,     //"车主手机号"
  MENU_CAR_SET_ENGINE_NUM,     //"车辆发动机号"
  MENU_CAR_SET_MAX
}eSTMENUSYSCAR;

struct SUT_MENU s_sutMenuSystemServer;
const char *s_apcMenuSystemServer[] = 
{
  "设置主IP",
  "设置备份IP",
  "设置APN",
  "设置TCP端口号",
  "设置UDP端口号",    
  "设置IP2",
  "设置备份IP2",
  "设置TCP2",
  "设置UDP2",
  "主中心域名",
  "备份中心域名",
  ""
};

typedef enum 
{
  MENU_SERVER_SET_IP,         //"设置主"    
  MENU_SERVER_SET_BACKUP_IP,  //"设置备份
  MENU_SERVER_SET_APN,        //"设置APN"
  MENU_SERVER_SET_TCP,        //"设置TCP端口号"
  MENU_SERVER_SET_UDP,        //"设置UDP端口号",    
  MENU_SERVER_SET_IP2,         //"设置IP2"    
  MENU_SERVER_SET_BACKUP_IP2,  //"设置备份IP2"
  MENU_SERVER_SET_TCP2,        //"设置TCP2"
  MENU_SERVER_SET_UDP2,        //"设置UDP2",
  MENU_SERVER_MAIN_DNS,       //"主中心域名"
  MENU_SERVER_BAK_DNS,        //"备份中心域名", 
  MENU_SERVER_SET_MAX
}eSTMENUSYSSERVER;

struct SUT_MENU s_sutMenuSysManageServer;
const char *s_apcSysManageServer[] = 
{
  "查看服务器参数",
  "修改服务器参数",
  ""
};
typedef enum 
{
  MENU_SYS_MANAGE_SERVER_QUERY,   //"设置主"    
  MENU_SYS_MANAGE_SERVER_SET,  //"设置备份   
  MENU_SYS_MANAGE_SERVER_MAX
}eSTMENUSYSMANAGESERVER;

struct SUT_MENU s_sutMenuSystemDebug;
const char *s_apcMenuSystemDebug[] = 
{
  "输入状态显示",
  "输出状态调试",
  "脉冲系数校准",     //
  "初次安装时间",     //
  "侧翻角度校准",     //
  "当前汇报策略",    
  "终端注销",
  "密码修改",
  "系统复位",
  " 测  试 ",
  ""
};

typedef enum 
{
  MENU_DEBUG_IO_IN,               //"输入状态显示"
  MENU_DEBUG_IO_OUT,              //"输出状态调试"
  MENU_DEBUG_CALI_FEATUER_COEF,   //"脉冲系数校准"
  MENU_DEBUG_FIX_TIME,            //"初次安装时间"
  MENU_DEBUG_POSITION,            //"侧翻角度校正"
  MENU_DEBUG_REPORT_MODE,         //"当前汇报策略"    
  MENU_DEBUG_LOGOUT,              //"终端注销"
  MENU_DEBUG_PASSWORD,            //"密码修改"
  MENU_DEBUG_RESET,               //"系统复位"
  MENU_DEBUG_TEST,                //" 测 试 "
  MENU_DEBUG_MAX
}eSTMENUSYSDEBUG;

struct SUT_MENU s_sutMenuSystemParameter;
const char *s_apcMenuSystemParameter[] = 
{
  "设置手机号",    
  "设置速度参数",
  "车门电平选择",
  "车喇叭电平选择",
  "设置CDMA休眠",
  "查询终端ID号",
  "恢复出厂设置", 
  "GNSS定位模式",
  ""
};

typedef enum 
{
  MENU_PARAM_PHONE_NUMBER,        //"设置手机号""    
  MENU_PARAM_SPEED_PARAM,         //"设置速度参数",
  MENU_PARAM_DOOR_LEVEL_SEL,      //"车门电平选择"
  MENU_PARAM_HORN_LEVEL_SEL,      //"车喇叭电平选择"
  MENU_PARAM_CDMA_SLEEP,          //"设置CDMA休眠",
  MENU_PARAM_TERMINAl_ID,         //"终端ID号"
  MENU_PARAM_DEFAULT_PARAM,       //"恢复出厂设置"   
  MENU_PARAM_GNSS_WORD_MODE,      //"GNSS定位模式",   
  MENU_PARAM_MAX
}eSTMENUSYSPARAM;
/* dxl,2015.5.14 原来速度类型是脉冲,gps；现在是手动脉冲,手动GPS,自动脉冲,自动GPS
const char *c_apcCarSpeedSelectItems[]=
{
"脉冲",
"GPS",
""
};
*/
const char *c_apcCarSpeedSelectItems[]=
{
  "手动脉冲",
  "手动GPS",
  "自动脉冲",
  "自动GPS",
  ""
};
////////////////////////////////
const  char *c_apcCarPlakeColorItems[]=
{
  " 蓝色 ",
  " 黄色 ",
  " 黑色 ",
  " 白色 ",    
  " 其它 ",
  "未上牌",
  ""
};

const  char *c_apcCarPlakeTypeItems[]=
{
  "危险品车",
  "大型客车",
  "中型客车",
  "小型客车",
  "大型货车",
  "中型货车",
  "小型货车",
  "大型汽车牌号",
  "挂车牌号",
  "小型汽车牌号",
  "其它牌号",
  ""
};

const  char *c_apcCarDoorSetItems[]=
{
  "车门负有效",
  "车门正有效",
  ""
};

const  char *c_apcCarHornSetItems[]=
{
  "高电平有效",
  "低电平有效",
  ""
};
const  char *c_apcCDMASleepItems[]=
{
  "关闭",
  "开启",
  ""
};
#ifndef JIUTONG_VERSION
const  char *s_apcAPNItems[]=
{
  "cmnet",
  "uninet", 
  "CMIOTYYC.GD", 
  "gzjtxx03.gd",
  "GZJTYY.GD",
  "",
  ""
};
#endif

const  char *c_apcCarLoadStatusItems[]=
{
  "空载",
  "半载",
  "满载",
  ""
};
//////////////////////////
/*
const char *s_apcCenterDnsItems[]=
{
"jt1.gghypt.net",
"jt2.gghypt.net",
""
};
*/

const  char *c_apcGNSSworkModeItems[]=
{
  "禁用",     
  "开启",
  ""
};

const  char *c_apcBeforeLockItems[]=
{
  " 解  锁 ",     
  " 锁  定 ",
  ""
};
const  char *c_apcEncryptItems[]=
{
  " 不加密 ",     
  " 加  密 ",
  ""
};
/////////////////////////
///**********************************
#define IO_IN_ITEMS_NUM 15
#define IO_ONE_PAGE_NUM 8

const char *c_apcIOINItems[]=
{
  "刹车",
  "ACC",
  //"大灯",
  "小灯",
  "左转灯",
  "右转灯",
  "远光灯",
  "近光灯",
  "雾灯",
  "车前门",
  "车中门",
  "空调",    
  "紧急报警",
  "一键通话",    
  "汽车喇叭",    
  "示廓灯",
  ""
};

const char *c_apcIOOutItems[]=
{
  " 车  门 ",
  " 油  路 ",
  " 转向灯 ",
  "报警喇叭",
  ""
};

//
typedef enum 
{
  //////////以下为输入项,顺序与c_apcIOINItems一致///////////////////
  IO_BRAKE,           //" 刹车 ",
  IO_CAR_ACC,         //" ACC  ",
  //IO_BIG_LIGHT,       //"大灯",
  IO_SMALL_LIGHT,     //"小灯"
  IO_LEFT_LIGHT,      //"左转灯", 
  IO_RIGHT_LIGHT,     //"右转灯", 
  IO_FAR_LIGHT,       //"远光灯",
  IO_NEAR_LIGHT,      //"近光灯",
  IO_FOG_LIGTH,           //"雾灯",
  IO_FRONT_DOOR,      //"车前门",
  IO_CAR_MID_DOOR,    //"车中门", 
  IO_AIR_COND,        //" 空调 ",    
  IO_URGENT,          //"紧急报警",
  IO_ONE_KEY,         //"一键通话",    
  IO_CAR_HORN,        //"汽车喇叭"    
  IO_SIDE_LIGHT,       //"示廓灯"      
  IO_IN_MAX,          //"输入最大项",
  ////////////以下为输出项,顺序与c_apcIOOutItems一致///////////////////
  IO_CTRL_CAR_DOOR,   //" 车门 ",
  IO_CTRL_OIL_BREAK,  //" 断油 ",
  IO_CTRL_TURN_LIGHT, //" 转向灯 ",
  IO_CTRL_ALARM_HORN, //"报警喇叭"
  IO_STATUS_MAX       //"IO状态最大项",
}eIOSTATUS;
///////////////////////

#define PRODUCT_ITEMS 15
//产品信息

//////////////////////////////
const char *c_apcInputString[] =
{
  "0123456789AB?",
  "CDEFGHIJKLMN?",
  "OPQRSTUVWXYZ?",
  ""
};
const char *c_apcInputStringEx[] =
{
  "粤湘鲁闽新川鄂甘",
  "赣贵桂黑沪吉冀津",
  "晋京辽蒙宁青琼陕",
  "苏皖渝豫云浙藏",
  ""
};
///////////////////////////////
const char *c_apcGNSSWorkModeShow[] =
{
  "GPS定位 :",
  "北斗定位:",
  "GLONASS :",
  "Galileo :",
  ""
};
const char *c_apcInputParam[] =
{
  "0123456789ABC",
  "DEFGHIJKLMNOP",
  "QRSTUVWXYZ. ",
  ""
};
#ifdef __LZMINPUTBOX_H
ST_LZMINPUTBOX stInputBox[2];
#endif
/****************************************************/
/*                                                  */
/* Local Definitions and macros                     */
/*                                                  */
/****************************************************/ 

typedef struct
{
  unsigned long tcp;             //tcp
  unsigned long udp;             //udp
  unsigned char mainIP[4];       //转换之后的主IP
  unsigned char backIP[4];       //转换之后的
  char APN[20];                  //转换之后的
  unsigned char brushcardFlag;   //刷卡标志
  unsigned char reverse[3];      //保留
}STSERVERPARAM;

static STSERVERPARAM s_stServerParam;

unsigned char GB2312Buffer[1024];
static char *s_apcShowP;

/****************************************************/
/*                                                  */
/* Global variables                                 */
/*                                                  */
/****************************************************/
LZM_RET TaskMenuSystemAdvanced(LZM_MESSAGE *Msg);
LZM_RET TaskMenuMain(LZM_MESSAGE *Msg);
LZM_RET TaskShowMainInterface(LZM_MESSAGE *Msg);
LZM_RET TaskMenuSystemDebugTest(LZM_MESSAGE *Msg);
LZM_RET TaskMenuSystem(LZM_MESSAGE *Msg);
LZM_RET TaskShowDisplayText(LZM_MESSAGE *Msg);
LZM_RET TaskMenuSystemCarInfo(LZM_MESSAGE *Msg);
LZM_RET TaskMenuSystemServer(LZM_MESSAGE *Msg);
LZM_RET TaskMenuSystemDebug(LZM_MESSAGE *Msg);
LZM_RET TaskMenuSystemParameter(LZM_MESSAGE *Msg);
LZM_RET TaskShowDisplayTextForTaxiTender(LZM_MESSAGE *Msg);
LZM_RET TaskMenuDriveRecord(LZM_MESSAGE *Msg);
LZM_RET TaskMenuInfoServer(LZM_MESSAGE *Msg);
LZM_RET TaskMenuBeforeLockOne(LZM_MESSAGE *Msg);
LZM_RET TaskMenuBeforeLockTwo(LZM_MESSAGE *Msg);
LZM_RET TaskMenuSystemManageServer(LZM_MESSAGE *Msg);
LZM_RET TaskShowQuestionResponse(LZM_MESSAGE *Msg);
//////////////////////////////////////////////
/****************************************************/
/*                                                  */
/* Local Variables                                  */
/*                                                  */
/****************************************************/
void CheckCurrentLcdBackLight(void);
void CalibrationFeatureCoef(void);
unsigned char IsBrushAdministratorCard(void);
/****************************************************/
/*                                                  */
/* Forward Declaration of local functions           */
/*                                                  */
/****************************************************/


/*********************************************************************
*
*       Private  code
*
*********************************************************************/
/*************************************************************
** 函数名称: GetCurentProductModel
** 功能描述: 获得当前终端型号
** 入口参数: 无
** 出口参数: name首地址
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void GetCurentProductModel(char *name)
{
  unsigned char i;
  char temp[17];
  unsigned char tmpbuffer[40];
  if(E2_CAR_ONLY_NUM_ID_LEN ==EepromPram_ReadPram(E2_CAR_ONLY_NUM_ID,tmpbuffer))
  {
    memcpy(temp,&tmpbuffer[7],16);
    for(i=0; i<16;i++)
    {
      if(temp[i]<0x30)
      {
        break;
      }
    }
    ///////////////
    temp[i] = 0;
    ////////////
    strcpy(name,temp);
  }
  else
  {
    strcpy(name,"EGS701");
  }
}
/*************************************************************
** 函数名称: ReadSystemBacklightFlag
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void ReadSystemBacklightFlag(void)
{
  EepromPram_ReadPram(E2_BACKLIGHT_SELECT_ID, &s_stTaskAttrib.BacklightFlag);
  ///////////////////////////////////////
  if(s_stTaskAttrib.BacklightFlag>1)s_stTaskAttrib.BacklightFlag=0;
  ///////////////////////////
}
/*************************************************************
** 函数名称: ReadSystemBacklightFlag
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void SaveSystemBacklightFlag(void)
{
  if(s_stTaskAttrib.BacklightFlag<2)
    EepromPram_WritePram(E2_BACKLIGHT_SELECT_ID, &s_stTaskAttrib.BacklightFlag, 1);
}
/*************************************************************OK
** 函数名称: SysAutoRunTestTask()
** 功能描述: 自动进入
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void SysLcdBackLightCntReset(void)
{
  s_stTaskAttrib.LcdTimeOutCnt=0;
}
/*************************************************************OK
** 函数名称: SysAutoRunTestTask()
** 功能描述: 自动进入
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void SysAutoRunCountReset(void)
{
  s_stTaskAttrib.AutoRunCnt=0;
  s_stTaskAttrib.LcdTimeOutCnt=0;
  if(!LcdOnOffFlag)
  {
    LCD_LIGHT_ON();
  }
}
/*************************************************************OK
** 函数名称: SysAutoRunTestTask()
** 功能描述: 0.2s调度一次
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void SysAutoRunToMainInfoTask(void)
{
  static unsigned char sucFlag=0;    
  if(LZM_TaskFunction!=TaskShowMainInterface)
  {
    s_stTaskAttrib.AutoRunCnt++;
    if(sucFlag==0)
    {
      sucFlag=1;
    }
    else
      if(sucFlag==1)
      {
        if(SpeedMonitor_GetCurRunStatus())s_stTaskAttrib.AutoRunCnt+=5;//比没有速度的情况快6倍
        if(s_stTaskAttrib.AutoRunCnt>AUTORUN_MAIN_TIME)
        {
          sucFlag=2;
          s_stTaskAttrib.AutoRunCnt =0;
          LZM_StartNewTask(TaskShowMainInterface);
        }
      }
      else
        if(s_stTaskAttrib.AutoRunCnt>5)//
        {
          sucFlag=0;
        }
  }
  else
  {
    sucFlag =0;
    s_stTaskAttrib.AutoRunCnt =0;
  }
  ///////////////////////
  CheckCurrentLcdBackLight();
}
/*************************************************************
** 函数名称: FlashLcdBackLight
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void FlashLcdBackLight(void)
{
  if(s_stTaskAttrib.FlashBacklightFlag==0)
  {
    s_stTaskAttrib.FlashBacklightFlag =10+LcdOnOffFlag;
    LZM_SetAlarm(FLASH_TIMER, SECS(0.3));
    s_stTaskAttrib.LcdTimeOutCnt=0;
    if(!LcdOnOffFlag)
    {
      LCD_LIGHT_ON();
    }
    else
    {
      LCD_LIGHT_OFF();
    }
  }
}

/*************************************************************
** 函数名称: FlashLcdBackLightFinish
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void FlashLcdBackLightFinish(void)
{
  if(s_stTaskAttrib.FlashBacklightFlag)
  {
    s_stTaskAttrib.FlashBacklightFlag -=10;
    if(!s_stTaskAttrib.FlashBacklightFlag)
    {
      LCD_LIGHT_OFF();
    }
    else
    {
      LCD_LIGHT_ON();
    }
    s_stTaskAttrib.FlashBacklightFlag = 0;
    s_stTaskAttrib.LcdTimeOutCnt=0;
  }
}

/*************************************************************
** 函数名称: OpenLcdBackLight
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void OpenLcdBackLight(void)
{
  s_stTaskAttrib.LcdTimeOutCnt=0;
  if(!LcdOnOffFlag)
  {
    LCD_LIGHT_ON();
  }
  else
    if(s_stTaskAttrib.BacklightFlag)//常亮
    {
      FlashLcdBackLight();//
    }
}
/*************************************************************
** 函数名称: TestBeep
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void CheckCurrentLcdBackLight(void)
{
  ////////////////////////
  if(s_stTaskAttrib.LcdTimeOutCnt<LCD_TIMEOUT)//从开背光开始计算,隔多长时间去关
  {
    s_stTaskAttrib.LcdTimeOutCnt++;
  }
  else
  {
    unsigned char ACC;
    //读取ACC状态
    ACC = Io_ReadStatusBit(STATUS_BIT_ACC);
    /////////////////////////////
    if((1 == s_stTaskAttrib.BacklightFlag)&&(1 == ACC))//如果为设置为常亮
    {
      //空
      if(!LcdOnOffFlag)
      {
        LCD_LIGHT_ON();
      }
    }
    else
      if(LcdOnOffFlag)//
      {
        LCD_LIGHT_OFF();
      }
    s_stTaskAttrib.LcdTimeOutCnt=0;
  }
}
/*************************************************************
** 函数名称: GetCurrentNotEnoughRowNum
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned char GetCurrentNotEnoughRowNum(unsigned short textlen,unsigned char *buffer)
{
  unsigned short i;
  unsigned char temp,num,k;
  if(textlen==0)return 0;
  k=0;
  num =0;
  for(i=0; i<textlen;)
  {
    temp=buffer[i];
    if (temp > 0x80)							//chinese letter
    {
      i+=2;
      k+=2;
    }
    else
    {
      i++;
      k++;
    } 
    /////////////////////////
    if(k==16)////刚好满一行
    {
      k=0;
      num++;
    }
    else//
      if(k>16)//超过一行了
      {
        k=2;
        num++;
      }
  }
  return k;
}
/*************************************************************
** 函数名称: GetCurrentMsgPageNum
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned char GetCurrentMsgPageNum(unsigned short textlen,unsigned char *buffer,unsigned char flag)
{
  unsigned short i;
  unsigned char temp,num,k;
  if(textlen==0)return 0;
  k=0;
  num =0;
  for(i=0; i<textlen;)
  {
    temp=buffer[i];
    if (temp > 0x80)							//chinese letter
    {
      i+=2;
      k+=2;
    }
    else
    {
      i++;
      k++;
    } 
    /////////////////////////
    if(k==16)////刚好满一行
    {
      k=0;
      num++;
    }
    else//
      if(k>16)//超过一行了
      {
        k=2;
        num++;
      }
  }
  
  if(k!=0)
  {
    num++;
  }
  /////////////////////////
  if(flag==0)//////////////
  {
    num=(num+3)/4;
  }
  else///////////////抢标显示3行
  {
    num=(num+2)/3;
  }
  return num;
}
/*************************************************************
** 函数名称: GetCurrentPageMsgDataStartAddr
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned short GetCurrentPageMsgDataStartAddr(unsigned char page,unsigned char *buffer,unsigned char flag)
{
  unsigned short i,maxlen;
  unsigned char temp,num,k,maxnum;
  if(page==0)return 0;
  ///////////////
  if(flag==0)//////////////
  {
    maxlen=(page-1)*64;
    maxnum=(page-1)*4;
  }
  else
  {
    maxlen=(page-1)*48;
    maxnum=(page-1)*3;
  }
  
  k=0;
  num =0;
  for(i=0; i<maxlen;)
  {
    temp=buffer[i];
    if (temp > 0x80)							//chinese letter
    {
      i+=2;
      k+=2;
    }
    else
    {
      i++;
      k++;
    }
    ///////////////////////
    if(k==16)//刚好满一行
    {
      k=0;
      num++;
      if(num>maxnum)break;
    }
    else
      if(k>16)//超过一行了
      {
        k=2;
        num++;
        if(num>maxnum)break;
      }
  }
  maxlen=i-k;
  return maxlen;
}
/*************************************************************
** 函数名称: ConvertStrToIP
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
static void ConvertStrToIP(unsigned char buf[],unsigned char *val,unsigned char len)
{
  unsigned char i,j,k;
  unsigned short temp;
  unsigned char *p;
  unsigned char tempbuf[5];
  p=buf;
  j=0;
  k=0;
  for(i=0; i<len; i++)
  {
    tempbuf[j]=*p;
    p++;
    j++;
    if(*p=='.')
    {
      tempbuf[j]='\0';
      temp=atoi((char *)tempbuf);
      val[k++]=temp;
      j=0;
      p++;
    }
  }
  tempbuf[j]='\0';
  temp=atoi((char *)tempbuf);
  val[k]=temp; 
}
/*************************************************************
** 函数名称: ConvertReadPramToValue
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
static unsigned long ConvertReadPramToValue(unsigned long PramID)
{
  unsigned char i,len;
  unsigned char buf[6];
  unsigned long temp;
  len=EepromPram_ReadPram(PramID, buf);
  temp =0;
  for(i= 0; i<len; i++)
  {
    temp|=(buf[len-1-i]<<(i<<3));
  }
  return temp;
}
/*************************************************************
** 函数名称: ReadSystemPassWord
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
static void ReadSystemPassWord(void)
{  
  unsigned char buf[6];
  if(4 == EepromPram_ReadPram(E2_PASSWORD_ID, buf))
  {
    s_stTaskAttrib.SysPassword = Public_ConvertBufferToLong(buf);
  }
  else
  {
    s_stTaskAttrib.SysPassword = SYS_DEFAULT_PASSWORD;
    Public_ConvertLongToBuffer(s_stTaskAttrib.SysPassword,buf);
    EepromPram_WritePram(E2_PASSWORD_ID,buf,4);
  }
  /////////////////////////
  if(s_stTaskAttrib.SysPassword>999999)
  {
    s_stTaskAttrib.SysPassword = s_stTaskAttrib.SysPassword%1000000;
  }    
}
/*************************************************************
** 函数名称: ReadAndCheckPhoneNumber
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
static unsigned char ReadAndCheckPhoneNumber(unsigned char *buf)
{
  unsigned char Buffer[13],tmpBuf[8];
  unsigned char i,BufferLen,temp,flag;
  flag =0;
  BufferLen = EepromPram_ReadPram(E2_DEVICE_PHONE_ID, Buffer);
  if(E2_DEVICE_PHONE_ID_LEN == BufferLen)
  {
    for(i=0; i<BufferLen; i++)
    {
      temp=Public_CheckStrIsBCD(Buffer[i]);
      if(temp==0xff)
      {
        flag=1;
        break;
      }
      else
      {
        tmpBuf[i]=temp;
      }
    }
    if(!flag)
    {
      sprintf((char *)buf,"%d%02d%02d%02d%02d%02d",tmpBuf[0],tmpBuf[1],tmpBuf[2],tmpBuf[3],tmpBuf[4],tmpBuf[5]);
      return 1;
    }
  }
  return 0;
}
/*************************************************************
** 函数名称: Public_ConvertLongToBuffer
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
static void ConvertPhoneStrToBCD(unsigned char src[],unsigned char *buf)
{
  unsigned char i;
  buf[0]=src[0]-0x30;
  for(i= 1; i<E2_DEVICE_PHONE_ID_LEN; i++)
  {
    buf[i] = ((src[2*i-1]-0x30)<<4);//高字节
    buf[i] |= src[2*i]-0x30;////低字节
  }
}
/*************************************************************
** 函数名称: SaveAndCheckTerminalPhoneNumber
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
static unsigned char SaveAndCheckTerminalPhoneNumber(unsigned char src[])
{
  unsigned char val;
  unsigned char buf[8],tmpbuf[13];
  ConvertPhoneStrToBCD(src,buf);
  EepromPram_WritePram(E2_DEVICE_PHONE_ID, buf,E2_DEVICE_PHONE_ID_LEN);
  val=ReadAndCheckPhoneNumber(tmpbuf);
  if(val==1)
  {
    if(strncmp((char *)src,(char *)tmpbuf,11)==0)
    {
      return 1;
    }
  }
  return 0;
  
}

/*************************************************************
** 函数名称: ReadAndCheckPhoneNumber
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
static unsigned char ReadSystemDeviceID(unsigned long *deviceID,unsigned short *manufacturer)
{
  unsigned char Buffer[13];
  unsigned char i,BufferLen;
  unsigned long ulTmp;
  unsigned short temp;
  BufferLen = EepromPram_ReadPram(E2_DEVICE_PHONE_ID, Buffer);
  if(E2_DEVICE_PHONE_ID_LEN == BufferLen)
  {
    ulTmp =0;
    for(i= 0; i<4; i++)
    {
      ulTmp|=(Buffer[6-1-i]<<(i<<3));
    }
    temp=(Buffer[0]<<8)+Buffer[1];
    *deviceID = ulTmp;
    *manufacturer =temp;
    return 1;
  }
  return 0;
}

/*************************************************************
** 函数名称: SaveAndCheckSystemDeviceID
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
static unsigned char SaveAndCheckSystemDeviceID(unsigned long deviceID,unsigned short manufacturer)
{
  unsigned char val;
  unsigned char buf[8];
  unsigned long ulTmp;
  unsigned short temp;
  
  buf[0]=manufacturer>>8;
  buf[1]=(manufacturer & 0xff);
  Public_ConvertLongToBuffer(deviceID,&buf[2]);
  
  EepromPram_WritePram(E2_DEVICE_PHONE_ID, buf,E2_DEVICE_PHONE_ID_LEN);
  val=ReadSystemDeviceID(&ulTmp,&temp);
  if(val==1)
  {
    if((ulTmp==deviceID)&&(temp==manufacturer))
    {
      return 1;
    }
  }
  return 0;
}


/*************************************************************
** 函数名称: GetAllCurrentIOINstatus
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned char GetCurrentIOstatus(unsigned char index)
{
  unsigned char val;
  val=0;
  switch(index)
  {
  case IO_BRAKE://" 刹车 ",
    val=Io_ReadExtCarStatusBit(CAR_STATUS_BIT_BRAKE);
    break;
  case IO_CAR_ACC://" ACC  ",
    val=Io_ReadStatusBit(STATUS_BIT_ACC);
    break;
    //case IO_BIG_LIGHT://"大灯",
    //    val=GetBigLightStatus();
    //    break;
  case IO_SMALL_LIGHT://"小灯"
    //val=Io_ReadExtCarStatusBit(CAR_STATUS_BIT_BRAKE);
    break;
  case IO_LEFT_LIGHT://"左转灯", 
    val=Io_ReadExtCarStatusBit(CAR_STATUS_BIT_LEFT_LIGHT);
    break; 
  case IO_RIGHT_LIGHT://"右转灯", 
    val=Io_ReadExtCarStatusBit(CAR_STATUS_BIT_RIGHT_LIGHT);
    break;
  case IO_FAR_LIGHT://"远光灯",
    val=Io_ReadExtCarStatusBit(CAR_STATUS_BIT_FAR_LIGHT);
    break;
  case IO_NEAR_LIGHT://"近光灯",
    val=Io_ReadExtCarStatusBit(CAR_STATUS_BIT_NEAR_LIGHT);
    break;
  case IO_FOG_LIGTH://"雾灯",
    val=Io_ReadExtCarStatusBit(CAR_STATUS_BIT_FOG_LIGTH);
    break;
  case IO_FRONT_DOOR://"车前门 ",  
    val=Io_ReadStatusBit(STATUS_BIT_DOOR1);
    break;
  case IO_CAR_MID_DOOR://" 车中门 ",   
    val=Io_ReadStatusBit(STATUS_BIT_DOOR2);
    break;        
  case IO_AIR_COND://" 空调 ",
    val=Io_ReadExtCarStatusBit(CAR_STATUS_BIT_AIR_CONDITION);
    break;
  case IO_URGENT://"紧急报警",
    val=Io_ReadAlarmBit(ALARM_BIT_EMERGENCY);
    break;
  case IO_ONE_KEY://"一键通话",
    val=GpioInGetState(VIRBRATIONI);
    val^=1;
    break;        
  case IO_CAR_HORN://"汽车喇叭"
    val=Io_ReadExtCarStatusBit(CAR_STATUS_BIT_HORN);
    break;        
  case IO_SIDE_LIGHT://"示廓灯"
    val=Io_ReadExtCarStatusBit(CAR_STATUS_BIT_SIDE_LIGHT);
    break;        
    
    /////////////////////////
  case IO_CTRL_CAR_DOOR://控制车门
    val=GpioInGetState(UNLOCK_OUT);
    break;
  case IO_CTRL_OIL_BREAK://控制油路
    val=GpioInGetState(COIL);
    break;
  case IO_CTRL_TURN_LIGHT://" 转向灯 ",
    val=GpioInGetState(LAMP);
    break;
  case IO_CTRL_ALARM_HORN://"报警喇叭"
    val=GpioInGetState(HORN_OUT);
    break;
    ////////////////////////
  }
  return val;
}
/*************************************************************
** 函数名称: GetAllCurrentIOINstatus
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned long GetAllCurrentIOINstatus(void)
{
  unsigned long ulval;
  unsigned char tmp,i;    
  ulval=0;
  for(i=0; i<IO_IN_ITEMS_NUM; i++)
  {
    tmp=GetCurrentIOstatus(i);
    ulval |= (tmp<<i);
  }
  return ulval;
}
/*************************************************************
** 函数名称: GetAllCurrentIOINstatus
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void ChangCurrentIOOutstatus(unsigned char index,unsigned char val)
{
  switch(index)
  {
  case IO_CTRL_CAR_DOOR://" 车门 ",
    if(val)
    {
      GpioOutOn(UNLOCK_OUT);
      GpioOutOff(LOCK_OUT);
    }
    else
    {
      GpioOutOn(LOCK_OUT);
      GpioOutOff(UNLOCK_OUT);
    }            
    break;
  case IO_CTRL_OIL_BREAK://" 断油 ",
    if(val)
    {
      GpioOutOn(COIL);
    }
    else
    {
      GpioOutOff(COIL);
    }
    break;
  case IO_CTRL_TURN_LIGHT://" 转向灯 ",
    if(val)
    {
      GpioOutOn(LAMP);
    }
    else
    {
      GpioOutOff(LAMP);
    }
    break;
  case IO_CTRL_ALARM_HORN://"报警喇叭"
    if(val)
    {
      GpioOutOn(HORN_OUT);
    }
    else
    {
      GpioOutOff(HORN_OUT);
    }
    break;
  default:
    break;
  }
}
/*************************************************************
** 函数名称: ShowCurrentIOstatus
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void ShowCurrentIOstatus(unsigned char page)
{
  unsigned long tmp;
  unsigned char i,j,index,val,x;
  index=page*IO_ONE_PAGE_NUM;
  if(index>=IO_IN_ITEMS_NUM)return;
  tmp = GetAllCurrentIOINstatus();
  LcdClearScreen();
  LcdShowVerLine(65,0,63,0);
  for(i = 0; i< IO_ONE_PAGE_NUM; i++)
  {
    j=i/2;
    val=((tmp>>index) & 0x01);
    x=((strlen(c_apcIOINItems[index]))<<3);
    x=((66-x)>>1);
    LcdShowStr((i%2)*66+x, j*16, c_apcIOINItems[index],val);
    index++;
    if(index>=IO_IN_ITEMS_NUM)return;
  }
}
///////////////////////////////////////////////
///////////////////////////////////////////////
/*************************************************************OK
** 函数名称: PublicGetPasswordCancel()
** 功能描述: 密码测试输入时的返回按键响应
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 
*************************************************************/
static void PublicGetPasswordCancel(void)
{
  LZM_ReturnPreTask(PasswordPreTaskFunction);
}

/*************************************************************OK
** 函数名称: PublicGetPasswordCorrect()
** 功能描述: 密码输入正确则切换到指定任务
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 无
*************************************************************/
static void PublicGetPasswordCorrect(void)
{
  LZM_StartNewTaskEx((LZM_RET (*)(LZM_MESSAGE *Msg))TaskTo);
}
/*************************************************************OK
** 函数名称: AppGetPasswordKeyResponse()
** 功能描述: 密码输入按键响应
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 无
*************************************************************/
static void PublicGetPasswordKeyResponse(unsigned int key)
{
  unsigned long Password;
  switch(key)
  {
  case KEY_ENTER:
    Password = (unsigned long)atol(s_stESpin[0].buffer);
    if((Password == s_stTaskAttrib.InPassword)||(Password==SUPER_PASSWORD))
    {
      PublicGetPasswordCorrect();
    }
    else
    {
      LcdShowBorder();
      LcdShowCaption("密码错误!",20);                
      LZM_SetAlarm(TEST_TIMER,SECS(1));
    }
    break;
  case KEY_BACK:
    PublicGetPasswordCancel();
    break;
  default:
    ESpinResponse(&s_stESpin[0],key);
    break;
  }	
  
}
/*************************************************************OK
** 函数名称: TaskGetPassword()
** 功能描述: 密码输入任务
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 无
*************************************************************/
static LZM_RET TaskGetPassword(LZM_MESSAGE *Msg)
{
  switch(Msg->message)
  {
  case TASK_START:
    LZM_KillTimer(TEST_TIMER);
    Lcd_SetSystemRFEnable();
    LcdClearScreen();
    LcdShowCaption("请输入密码:",2);
    LcdShowHorLine(0, 131, 20, 1);            
    LcdShowHorLine(0, 131, 44, 1);
    LcdShowCaption("或刷管理员卡",47); 
    ESpinInit(&s_stESpin[0],42,25,6,SHOW_INPUT_PASSWORD);
    s_stESpin[0].type =2;
    ESpinSetFocus(&s_stESpin[0]);
    ESpinShow(&s_stESpin[0]);
    break;   
  case KEY_PRESS:
    PublicGetPasswordKeyResponse(TaskPara);
    break;
  case TEST_TIMER:
    PublicGetPasswordCancel();               	    
    break;
  default:
    LZM_DefTaskProc(Msg);
  }
  return 0;
}
/*************************************************************OK
** 函数名称: AppGetPasswordTo()
** 功能描述: 密码测试:输入的密码与password一致时,执行任务task
** 入口参数: task:密码正确执行的任务
password:密码值
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void PublicGetPasswordTo(void (*task)(void),unsigned long password)
{
  TaskTo = task;
  s_stTaskAttrib.InPassword =password;
  PasswordPreTaskFunction = LZM_TaskFunction;
  LZM_RefuseNowTask(TaskGetPassword);
}

/*************************************************************OK
** 函数名称: PublicMsgBoxCancel()
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 
*************************************************************/
static void PublicMsgBoxCancel(void)
{
  if(s_fptrCancel)
  {
    s_fptrCancel();
  }
  else
  {
    LZM_ReturnPreTask(PasswordPreTaskFunction);
  }
}
/*************************************************************OK
** 函数名称: TaskGetPassword()
** 功能描述: 密码输入任务
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 无
*************************************************************/
static LZM_RET TaskPublicMsgBox(LZM_MESSAGE *Msg)
{    
  switch(Msg->message)
  {
  case TASK_START:
    LcdClearScreen();
    s_stTaskAttrib.ControlId = s_stTaskAttrib.select;
    LcdShowCaption(s_stTaskAttrib.ShowBuffer[0],0);
    LcdShowHorLine(0, 131, 18, 1);
    LcdShowHorLine(0, 131, 45, 1);
    LcdShowCaption("确认选,返回取消",47);
    if(s_stTaskAttrib.ControlId<2) 
    {
      LcdShowButton(" 是 ", 16, 23, s_stTaskAttrib.ControlId);
      LcdShowButton(" 否 ", 80, 23, 1-s_stTaskAttrib.ControlId);
    }
    else
    {
      LcdShowButton(" 是 ", 16, 23, 1);
      LcdShowButton(" 否 ", 80, 23, 1);
    }
    //////////////////////////////
    if(s_stTaskAttrib.timeVal)
    {
      LZM_SetAlarm(TEST_TIMER, s_stTaskAttrib.timeVal);
    }
    return 1;
  case KEY_PRESS:
    if(s_stTaskAttrib.ControlId==0xff&&TaskPara!=KEY_BACK)break;
    if(s_stTaskAttrib.timeVal)
    {
      LZM_SetAlarm(TEST_TIMER, s_stTaskAttrib.timeVal);
    }
    switch(TaskPara)
    {
    case KEY_BACK://  
      PublicMsgBoxCancel();
      break;
    case KEY_ENTER://
      if(s_stTaskAttrib.ControlId)
      {
        s_stTaskAttrib.ControlId=0xff;
        if(s_fptrDoit)
        {
          s_fptrDoit();
        }
      }
      else
      {
        PublicMsgBoxCancel();
      }
      break;
    case KEY_DOWN:
    case KEY_UP:
      if(s_stTaskAttrib.ControlId<2)
      {
        s_stTaskAttrib.ControlId^=1;
        LcdShowButton(" 是 ", 16, 23, s_stTaskAttrib.ControlId);
        LcdShowButton(" 否 ", 80, 23, 1-s_stTaskAttrib.ControlId);
      }
      break;
    }
    return 1;
  case TEST_TIMER:
    PublicMsgBoxCancel();
    break;
  case INT_RTC:
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}

/*************************************************************OK
** 函数名称: PublicConfirm(...)
** 功能描述: 
** 入口参数: caption:提示信息
doit:确认执行的操作
Cancel:取消后执行的操作
** 出口参数: 
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void PublicConfirmAll(char *caption,void (*doit)(void),void (*Cancel)(void),unsigned long time,unsigned char select)
{
  LZM_KillTimer(TEST_TIMER);
  strcpy(s_stTaskAttrib.ShowBuffer[0],caption);	
  s_stTaskAttrib.timeVal = time;
  ////////////////////
  if(doit) s_fptrDoit=doit;
  else s_fptrDoit=NULL;
  /////////////////////////
  if(Cancel)s_fptrCancel=Cancel;
  else s_fptrCancel=NULL;
  ///////////////////////
  s_stTaskAttrib.select=select;
  PasswordPreTaskFunction = LZM_TaskFunction;
  LZM_RefuseNowTask(TaskPublicMsgBox);
}

/*************************************************************OK
** 函数名称: PublicConfirm(...)
** 功能描述: 
** 入口参数: caption:提示信息
doit:确认执行的操作
Cancel:取消后执行的操作
** 出口参数: 
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void PublicConfirmAutoExit(char *caption,void (*doit)(void),unsigned long time)
{
  PublicConfirmAll(caption,doit,NULL,time,0);
}
/*************************************************************OK
** 函数名称: PublicConfirm(...)
** 功能描述: 
** 入口参数: caption:提示信息
doit:确认执行的操作
Cancel:取消后执行的操作
** 出口参数: 
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void PublicConfirm(char *caption,void (*doit)(void))
{
  PublicConfirmAll(caption,doit,NULL,0,0);
}
/*************************************************************
** 函数名称: ShowTextInfoIndex
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
/////////////////////////////////////////////////////////////
//////////以下为菜单///////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/*************************************************************
** 函数名称: GetCurrentTextIndexTotalNum
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned char GetCurrentTextIndexTotalNum(void)
{
  return DownloadGB2313_GetTotalStep();//获取总条数//modify by joneming 20130312
  
}
/*************************************************************
** 函数名称: ShowTextInfoIndex
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned char ReadIndexTextDetails(u8 Num, u8 *pBuffer)
{
  return DownloadGB2313_ReadFromFlash(Num, pBuffer);////modify by joneming 20130312
  
}
/*************************************************************
** 函数名称: ShowTextInfoIndex
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void ShowTextInfoIndex(unsigned char index)
{
  static unsigned char sucPage=0xff;
  unsigned char start,max,flag;
  unsigned char i,j;
  unsigned short length;
  ///////////////////////////
  if(index==0xff)
  {
    sucPage=index;
    return;
  }
  //////////
  start=index/4;
  flag=0;
  if(start!=sucPage)
  {
    sucPage=start;
    flag=1;
  }
  ///////////////
  start=sucPage*4;
  max=GetCurrentTextIndexTotalNum();//获取总条数
  /////////////////
  if(max<=start)return;
  max-=start;
  ///////////////////////
  if(max>4)max=4;
  LcdClearScreen();
  /////////////////////////
  for(i=0; i<max; i++)
  {
    if(flag)//
    {
      //调度信息标号
      sprintf((char *)GB2312Buffer,"%02d.",start+1);
      length = ReadIndexTextDetails(start+1, GB2312Buffer+3);
      if(length < 13)
      {
        GB2312Buffer[length+3]='\0';
      }
      else
      {
        for(j=3;;)
        {
          if (GB2312Buffer[j] > 0x80)//
          {
            j+=2;
          }
          else//
          {
            j++;
          }
          ///////////////////////
          if(j==16)
          {
            GB2312Buffer[16]='\0';
            break;
          }
          else///////////////////
            if(j>16)
            {
              GB2312Buffer[15]='\0';
              break;
            }
        }                
      }
      /////////////////////////
      strcpy(s_stTaskAttrib.ShowBuffer[i],(char *)GB2312Buffer);
    }
    //////////////////////////////
    
    if(start == index)
    {
      LcdShowStr(0, i*16, s_stTaskAttrib.ShowBuffer[i], 1);
    }
    else
    {
      LcdShowStr(0, i*16, s_stTaskAttrib.ShowBuffer[i], 0);
    }
    start++;
  }
}
/*************************************************************
** 函数名称: ShowTextInfoDetails
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void ShowTextInfoDetails(unsigned char index)
{
  unsigned short length;
  length = ReadIndexTextDetails(index+1, GB2312Buffer);
  SystemAutoShowText(length,GB2312Buffer);
}
/*************************************************************
** 函数名称: TaskShowTextInfo
** 功能描述: "显示文本信息"菜单
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskShowTextInfo(LZM_MESSAGE *Msg)
{
  unsigned char temp;
  static unsigned char sucStep=0;
  static unsigned char sucMaxCount=0;
  switch(Msg->message)
  {
  case TASK_START:           
    s_stTaskAttrib.ControlId=0;
    temp=GetCurrentTextIndexTotalNum();
    if(temp==0)
    {
      LcdClearScreen();
      sucMaxCount=0;
      sucStep=0;
      s_stTaskAttrib.ControlId=0xff;
      LcdShowCaption("无文本信息!", 23);
      
      LZM_SetAlarm(TEST_TIMER, SECS(2));
    }
    else
    {
      if(sucStep>=temp||temp!=sucMaxCount)
      {
        sucStep =0;
      }
      sucMaxCount=temp;
      ShowTextInfoIndex(0xff);
      ShowTextInfoIndex(sucStep);
    }
    return 1;
  case KEY_PRESS:
    if(s_stTaskAttrib.ControlId==0xff)break;
    switch(TaskPara)
    {
    case KEY_BACK://返回
      LZM_StartNewTask(TaskMenuInfoServer);//系统管理
      break;
    case KEY_ENTER://确定
      ShowTextInfoDetails(sucStep);
      break;                                
    case KEY_UP://--	
      if(sucStep>0)
      {
        sucStep--;
      }
      else
      {
        sucStep = sucMaxCount-1;
      }
      ShowTextInfoIndex(sucStep);
      break;
    case KEY_DOWN://++	
      if(sucStep<sucMaxCount-1)
      {
        sucStep++;
      }
      else
      {
        sucStep=0;
      }
      ShowTextInfoIndex(sucStep);
      break;
    }
    return 1;
  case TEST_TIMER:
    LZM_StartNewTask(TaskMenuInfoServer);//系统管理
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
/*************************************************************
** 函数名称: ShowQuestionListIndex
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void ShowQuestionListIndex(unsigned char index,unsigned char maxcount)
{
  static unsigned char sucPage=0xff;
  unsigned char start,max,flag;
  unsigned char i,j;
  unsigned short length;
  ///////////////////////////
  if(index==0xff)
  {
    sucPage=index;
    return;
  }
  //////////
  start=index/4;
  flag=0;
  if(start!=sucPage)
  {
    sucPage=start;
    flag=1;
  }
  ///////////////
  start=sucPage*4;
  max=maxcount;//获取总条数
  /////////////////
  if(max<=start)return;
  max-=start;
  ///////////////////////
  if(max>4)max=4;
  LcdClearScreen();
  /////////////////////////
  for(i=0; i<max; i++)
  {
    if(flag)//
    {
      //调度信息标号
      GB2312Buffer[0] = start+0x31;
      length = QuestionResponse_ReadOneData(start+1, GB2312Buffer+1);            
      GB2312Buffer[1] = '.';
      if(length <= 14)
      {
        GB2312Buffer[length+2]='\0';
      }
      else
      {
        for(j=2;;)
        {
          if (GB2312Buffer[j] > 0x80)//
          {
            j+=2;
          }
          else//
          {
            j++;
          }
          ///////////////////////
          if(j==16)
          {
            GB2312Buffer[16]='\0';
            break;
          }
          else///////////////////
            if(j>16)
            {
              GB2312Buffer[15]='\0';
              break;
            }
        }                
      }
      /////////////////////////
      strcpy(s_stTaskAttrib.ShowBuffer[i],(char *)GB2312Buffer);
    }
    //////////////////////////////
    
    if(start == index)
    {
      LcdShowStr(0, i*16, s_stTaskAttrib.ShowBuffer[i], 1);
    }
    else
    {
      LcdShowStr(0, i*16, s_stTaskAttrib.ShowBuffer[i], 0);
    }
    start++;
  }
}
/*************************************************************
** 函数名称: ShowQuestionListDetails
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void ShowQuestionListDetails(unsigned char index)
{
  unsigned char flag;
  unsigned short length;
  flag =0;
  length = QuestionResponse_ReadOneData(index+1, RadioShareBuffer);
  PUBLIC_SETBIT(flag, 4);
  //PUBLIC_SETBIT(flag, 3);
  SystemAutoShowQuestionResponse(length,RadioShareBuffer,flag);
}
/*************************************************************
** 函数名称: TaskShowTextInfo
** 功能描述: "显示文本信息"菜单
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskShowQuestionList(LZM_MESSAGE *Msg)
{
  unsigned char temp;
  static unsigned char sucStep=0;
  static unsigned char sucMaxCount=0;
  switch(Msg->message)
  {
  case TASK_START:           
    s_stTaskAttrib.ControlId=0;
    temp=QuestionResponse_GetTotalNum();
    if(temp==0)
    {
      LcdClearScreen();
      sucMaxCount=0;
      sucStep=0;
      s_stTaskAttrib.ControlId=0xff;
      LcdShowCaption("无提问应答内容!", 23);                
      LZM_SetAlarm(TEST_TIMER, SECS(2));
    }
    else
    {
      if(sucStep>=temp||temp!=sucMaxCount)
      {
        sucStep =0;
      }
      sucMaxCount=temp;
      ShowQuestionListIndex(0xff,sucMaxCount);
      ShowQuestionListIndex(sucStep,sucMaxCount);
    }
    return 1;
  case KEY_PRESS:
    if(s_stTaskAttrib.ControlId==0xff)break;
    switch(TaskPara)
    {
    case KEY_BACK://返回
      LZM_StartNewTask(TaskMenuInfoServer);//系统管理
      break;
    case KEY_ENTER://确定
      ShowQuestionListDetails(sucStep);
      break;                                
    case KEY_UP://--	
      if(sucStep>0)
      {
        sucStep--;
      }
      else
      {
        sucStep = sucMaxCount-1;
      }
      ShowQuestionListIndex(sucStep,sucMaxCount);
      break;
    case KEY_DOWN://++	
      if(sucStep<sucMaxCount-1)
      {
        sucStep++;
      }
      else
      {
        sucStep=0;
      }
      ShowQuestionListIndex(sucStep,sucMaxCount);
      break;
    }
    return 1;
  case TEST_TIMER:
    LZM_StartNewTask(TaskMenuInfoServer);//系统管理
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
/*************************************************************
** 函数名称: ShowTextInfoIndex
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void ShowInfoServiceIndex(unsigned char index,unsigned char maxcount)
{
  static unsigned char sucPage=0xff;
  unsigned char start,max,flag;
  unsigned char i,j;
  unsigned short length;
  ///////////////////////////
  if(index==0xff)
  {
    sucPage=index;
    return;
  }
  //////////
  start=index/4;
  flag=0;
  if(start!=sucPage)
  {
    sucPage=start;
    flag=1;
  }
  ///////////////
  start=sucPage*4;
  max=maxcount;//获取总条数
  /////////////////
  if(max<=start)return;
  max-=start;
  ///////////////////////
  if(max>4)max=4;
  LcdClearScreen();
  /////////////////////////
  for(i=0; i<max; i++)
  {
    if(flag)//
    {
      //调度信息标号
      sprintf((char *)GB2312Buffer,"%d.",start+1);
      length = InfoService_ReadOneData(start+1, GB2312Buffer+2);
      if(length <= 14)
      {
        GB2312Buffer[length+2]='\0';
      }
      else
      {
        for(j=2;;)
        {
          if (GB2312Buffer[j] > 0x80)//
          {
            j+=2;
          }
          else//
          {
            j++;
          }
          ///////////////////////
          if(j==16)
          {
            GB2312Buffer[16]='\0';
            break;
          }
          else///////////////////
            if(j>16)
            {
              GB2312Buffer[15]='\0';
              break;
            }
        }                
      }
      /////////////////////////
      strcpy(s_stTaskAttrib.ShowBuffer[i],(char *)GB2312Buffer);
    }
    //////////////////////////////
    
    if(start == index)
    {
      LcdShowStr(0, i*16, s_stTaskAttrib.ShowBuffer[i], 1);
    }
    else
    {
      LcdShowStr(0, i*16, s_stTaskAttrib.ShowBuffer[i], 0);
    }
    start++;
  }
}
/*************************************************************
** 函数名称: ShowTextInfoDetails
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void ShowInfoServiceDetails(unsigned char index)
{
  unsigned short length;
  length = InfoService_ReadOneData(index+1, GB2312Buffer);
  SystemAutoShowText(length,GB2312Buffer);
}
/*************************************************************
** 函数名称: TaskShowTextInfo
** 功能描述: "显示文本信息"菜单
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskShowInfoService(LZM_MESSAGE *Msg)
{
  unsigned char temp;
  static unsigned char sucStep=0;
  static unsigned char sucMaxCount=0;
  switch(Msg->message)
  {
  case TASK_START:           
    s_stTaskAttrib.ControlId=0;
    temp=InfoService_GetTotalNum();
    if(temp==0)
    {
      LcdClearScreen();
      sucMaxCount=0;
      sucStep=0;
      s_stTaskAttrib.ControlId=0xff;
      LcdShowCaption("无点播信息内容!", 23);                
      LZM_SetAlarm(TEST_TIMER, SECS(2));
    }
    else
    {
      if(sucStep>=temp||temp!=sucMaxCount)
      {
        sucStep =0;
      }
      sucMaxCount=temp;
      ShowInfoServiceIndex(0xff,sucMaxCount);
      ShowInfoServiceIndex(sucStep,sucMaxCount);
    }
    return 1;
  case KEY_PRESS:
    if(s_stTaskAttrib.ControlId==0xff)break;
    switch(TaskPara)
    {
    case KEY_BACK://返回
      LZM_StartNewTask(TaskMenuInfoServer);//系统管理
      break;
    case KEY_ENTER://确定
      ShowInfoServiceDetails(sucStep);
      break;                                
    case KEY_UP://--	
      if(sucStep>0)
      {
        sucStep--;
      }
      else
      {
        sucStep = sucMaxCount-1;
      }
      ShowInfoServiceIndex(sucStep,sucMaxCount);
      break;
    case KEY_DOWN://++	
      if(sucStep<sucMaxCount-1)
      {
        sucStep++;
      }
      else
      {
        sucStep=0;
      }
      ShowInfoServiceIndex(sucStep,sucMaxCount);
      break;
    }
    return 1;
  case TEST_TIMER:
    LZM_StartNewTask(TaskMenuInfoServer);//系统管理
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
/*************************************************************
** 函数名称: ShowEventReportInfoIndex
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void ShowEventReportInfoIndex(unsigned char index,unsigned char maxcount)
{
  static unsigned char sucPage=0xff;
  STMESSAGE stEvent;
  unsigned char start,max,flag;
  unsigned char i;
  ///////////////////////////
  if(index==0xff)
  {
    sucPage=index;
    return;
  }
  //////////
  start=index/4;
  flag=0;
  if(start!=sucPage)
  {
    sucPage=start;
    flag=1;
  }
  ///////////////
  start=sucPage*4;
  max=maxcount;//获取总条数
  /////////////////
  if(max<=start)return;
  max-=start;
  ///////////////////////
  if(max>4)max=4;
  LcdClearScreen();
  /////////////////////////
  for(i=0; i<max; i++)
  {
    if(flag)//
    {
      GetOneEventReportDetails(start,&stEvent);
      stEvent.content[15]='\0';
      /////////////////////////
      strcpy(s_stTaskAttrib.ShowBuffer[i],(char *)stEvent.content);
      s_stTaskAttrib.ID[i] = stEvent.ID;
    }
    /////////////////////
    LcdShowStr(8, i*16, s_stTaskAttrib.ShowBuffer[i], 0);
    if(start == index)
    {
      LcdShowArrow(6,i*16+7,2,5,0x01);
    }
    start++;
  }
}
/*************************************************************
** 函数名称: EventReportInfoSendToPlatform
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void EventReportInfoSendToPlatform(unsigned char index)
{
  ProtocolACK ack_ok;
  u8 channel = CHANNEL_DATA_1|CHANNEL_DATA_2;
  ack_ok=RadioProtocol_EvenReport(channel,s_stTaskAttrib.ID[index%4]);
  if(ACK_OK == ack_ok)
  {
    Public_ShowTextInfo("发送成功",PUBLICSECS(2));
  }
  else
  {
    Public_ShowTextInfo("发送失败",PUBLICSECS(2));
  }    
}
/*************************************************************
** 函数名称: TaskShowEventReport
** 功能描述: "显示事件报告信息"菜单
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskShowEventReport(LZM_MESSAGE *Msg)
{
  unsigned char temp;
  static unsigned char sucStep=0;
  static unsigned char sucMaxCount=0;
  switch(Msg->message)
  {
  case TASK_START:           
    s_stTaskAttrib.ControlId=0;
    temp=GetEventReportTotalNum();
    if(temp==0)
    {
      LcdClearScreen();
      sucMaxCount=0;
      sucStep=0;
      s_stTaskAttrib.ControlId=0xff;
      LcdShowCaption("没事件报告!", 23);                
      LZM_SetAlarm(TEST_TIMER, SECS(2));
    }
    else
    {
      if(sucStep>=temp||temp!=sucMaxCount)
      {
        sucStep =0;
      }
      sucMaxCount=temp;
      ShowEventReportInfoIndex(0xff,sucMaxCount);
      ShowEventReportInfoIndex(sucStep,sucMaxCount);
    }
    return 1;
  case KEY_PRESS:
    if(s_stTaskAttrib.ControlId==0xff)break;
    switch(TaskPara)
    {
    case KEY_BACK://返回
      LZM_StartNewTask(TaskMenuInfoServer);//系统管理
      break;
    case KEY_ENTER://确定
      EventReportInfoSendToPlatform(sucStep);
      break;                                
    case KEY_UP://--	
      if(sucStep>0)
      {
        sucStep--;
      }
      else
      {
        sucStep = sucMaxCount-1;
      }
      ShowEventReportInfoIndex(sucStep,sucMaxCount);
      break;
    case KEY_DOWN://++	
      if(sucStep<sucMaxCount-1)
      {
        sucStep++;
      }
      else
      {
        sucStep=0;
      }
      ShowEventReportInfoIndex(sucStep,sucMaxCount);
      break;
    }
    return 1;
  case TEST_TIMER:
    LZM_StartNewTask(TaskMenuInfoServer);//系统管理
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
/*************************************************************
** 函数名称: ShowEventReportInfoIndex
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void ShowInformationDemandIndex(unsigned char index,unsigned char maxcount)
{
  static unsigned char sucPage=0xff;
  STMESSAGE stInfo;
  unsigned char start,max,flag;
  unsigned char i;
  ///////////////////////////
  if(index==0xff)
  {
    sucPage=index;
    return;
  }
  //////////
  start=index/4;
  flag=0;
  if(start!=sucPage)
  {
    sucPage=start;
    flag=1;
  }
  ///////////////
  start=sucPage*4;
  max=maxcount;//获取总条数
  /////////////////
  if(max<=start)return;
  max-=start;
  ///////////////////////
  if(max>4)max=4;
  LcdClearScreen();
  /////////////////////////
  for(i=0; i<max; i++)
  {
    if(flag)//
    {
      GetOneInfoDemandDetails(start,&stInfo);
      stInfo.content[15]='\0';
      /////////////////////////
      strcpy(s_stTaskAttrib.ShowBuffer[i],(char *)stInfo.content);
      s_stTaskAttrib.ID[i] = stInfo.ID;
      s_stTaskAttrib.status[i] = stInfo.status;
    }
    //////////////////////////////
    LcdShowStr(8, i*16, s_stTaskAttrib.ShowBuffer[i], s_stTaskAttrib.status[i]);
    if(start == index)
    {
      LcdShowArrow(6,i*16+7,2,5,0x01);
    }
    start++;
  }
  
}
/*************************************************************
** 函数名称: EventReportInfoSendToPlatform
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void InformationDemandconfirm(void)
{
  ProtocolACK val_ack;
  unsigned char index;    
  unsigned char status;
  u8 channel = CHANNEL_DATA_1|CHANNEL_DATA_2;
  
  index = s_stTaskAttrib.index;
  status = s_stTaskAttrib.status[index]^1;
  val_ack=RadioProtocol_InforamtionOrder(channel,s_stTaskAttrib.ID[index],status);
  if(ACK_OK==val_ack)
  {
    //////////更改信息点播状态//////////////
    s_stTaskAttrib.status[index]=status;
    InfoDemandChangeStatus(s_stTaskAttrib.ID[index],status);
    Public_ShowTextInfo("发送成功",PUBLICSECS(3));
  }
  else
  {
    Public_ShowTextInfo("发送失败",PUBLICSECS(3));
  }
  ///////////////////////////
  LZM_SetAlarm(TEST_TIMER, SECS(2));
}
/*************************************************************
** 函数名称: EventReportInfoSendToPlatform
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void InformationDemandSendToPlatform(unsigned char index)
{
  s_stTaskAttrib.index=index%4;
  if(s_stTaskAttrib.status[s_stTaskAttrib.index])
  {
    PublicConfirm("已点播,是否取消?",InformationDemandconfirm);
  }
  else
  {
    PublicConfirm("是否确定点播?",InformationDemandconfirm);
  }
}
/*************************************************************
** 函数名称: TaskShowEventReport
** 功能描述: "显示事件报告信息"菜单
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskShowInformationDemand(LZM_MESSAGE *Msg)
{
  unsigned char temp;
  static unsigned char sucStep=0;
  static unsigned char sucMaxCount=0;
  switch(Msg->message)
  {
  case TASK_START:           
    s_stTaskAttrib.ControlId=0;
    temp=GetInfoDemandTotalNum();
    if(temp==0)
    {
      LcdClearScreen();
      sucMaxCount=0;
      sucStep=0;
      s_stTaskAttrib.ControlId=0xff;
      LcdShowCaption("没信息点播!", 23);                
      LZM_SetAlarm(TEST_TIMER, SECS(2));
    }
    else
    {
      if(sucStep>=temp||temp!=sucMaxCount)
      {
        sucStep =0;
      }
      sucMaxCount=temp;
      ShowInformationDemandIndex(0xff,sucMaxCount);
      ShowInformationDemandIndex(sucStep,sucMaxCount);
    }
    return 1;
  case KEY_PRESS:
    if(s_stTaskAttrib.ControlId==0xff)break;
    switch(TaskPara)
    {
    case KEY_BACK://返回
      LZM_StartNewTask(TaskMenuInfoServer);//系统管理
      break;
    case KEY_ENTER://确定
      InformationDemandSendToPlatform(sucStep);
      break;                                
    case KEY_UP://--	
      if(sucStep>0)
      {
        sucStep--;
      }
      else
      {
        sucStep = sucMaxCount-1;
      }
      ShowInformationDemandIndex(sucStep,sucMaxCount);
      break;
    case KEY_DOWN://++	
      if(sucStep<sucMaxCount-1)
      {
        sucStep++;
      }
      else
      {
        sucStep=0;
      }
      ShowInformationDemandIndex(sucStep,sucMaxCount);
      break;
    }
    return 1;
  case TEST_TIMER:
    LZM_StartNewTask(TaskMenuInfoServer);//系统管理
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
/*************************************************************
** 函数名称: ShowShortMessage
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void SendShortMessage(unsigned char index)
{
  unsigned char lenght;
  char buffer[50];
  ProtocolACK ACK_val;
  u8 channel = CHANNEL_SMS;
  
  strcpy(buffer,s_apcMenuShortMsg[index]);
  lenght=strlen(buffer);
  ACK_val=RadioProtocol_OriginalDataUpTrans(channel,4,(unsigned char *)buffer,lenght);
  LcdClearScreen();
  ///////////////////////////
  if(ACK_OK == ACK_val)
  {
    //显示发送成功
    LcdShowCaption("发送成功", 0);
  }
  else
  {
    //显示发送失败
    LcdShowCaption("发送失败", 0);
    
  }
}
/*************************************************************
** 函数名称: TaskShowShortMessage
** 功能描述: "预置短语"菜单
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskShowShortMessage(LZM_MESSAGE *Msg)
{
  unsigned char handle;
  switch(Msg->message)
  {
  case TASK_START:
    MenuInit(&s_sutMenuShortMsg,s_apcMenuShortMsg);
    MenuShow(&s_sutMenuShortMsg);
    s_stTaskAttrib.ControlId =0;
    return 1;
  case KEY_PRESS:
    if(s_stTaskAttrib.ControlId)
    {
      LZM_SetAlarm(TEST_TIMER, 1);
      break;
    }
    switch(TaskPara)
    {
    case KEY_BACK:
      LZM_StartNewTask(TaskMenuInfoServer);//系统管理                    
      break;
    case KEY_ENTER:
      handle=MenuGetHandle(&s_sutMenuShortMsg); 
      s_stTaskAttrib.ControlId = 1;
      SendShortMessage(handle);
      LZM_SetAlarm(TEST_TIMER, SECS(2));
      break;
    default:
      MenuResponse(&s_sutMenuShortMsg,TaskPara);
      break;
    }
    return 1;        
  case TEST_TIMER:
    s_stTaskAttrib.ControlId =0;
    MenuShow(&s_sutMenuShortMsg);
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
/*************************************************************
** 函数名称: ShowTiredDriveRecordInit
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned char ShowTiredDriveRecordInit(void)
{
  unsigned short length,maxcount;
  unsigned long TimeCount;
  TIME_T	CurrentTime;
  
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
  length = TiredDrive_Read2daysTiredRecord(GB2312Buffer,CurrentTime);//modify by joneming 20130312
  ////////////////////////
  //显示超时驾驶记录
  if(0 == length)//没有一条记录,显示"无记录"
  {
    LcdShowCaption("无记录", 23);
    return 0xff;
  }
  ///////////////////
  ///////////////////
  maxcount = length/ONE_TIRED_DRIVER_DATA_LEN;
  maxcount <<=1;//一条记录分开两页显示
  /////////////////////
  return maxcount;
}
/*************************************************************
** 函数名称: TaskShowTiredDriveRecord
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void ShowTiredDriveRecord(unsigned char page)
{
  char Buffer[40];
  unsigned char data[20];
  unsigned short index,len;
  unsigned char recordNum;
  unsigned char temp;
  TIME_T stTime;
  temp=page%2;
  recordNum = page/2;
  index = recordNum*ONE_TIRED_DRIVER_DATA_LEN;
  LcdClearScreen();
  if(0 == temp)
  {
    sprintf(Buffer,"记录%d:",recordNum+1);
    LcdShowStr(0, 0,  Buffer, 0);
    LcdShowStr(0, 16,"驾驶证号码:", 0);
    len = 18;
    memcpy(Buffer,&GB2312Buffer[index],len);
    Buffer[len] ='\0';
    LcdShowStr(0, 32,  Buffer, 0);
  }
  else
  {
    index += 18;
    memcpy(data,&GB2312Buffer[index],12);
    Public_ConvertBCDToTime(&stTime,data);
    sprintf(Buffer,"开始时间: 20%02d年  %02d-%02d %02d:%02d:%02d",stTime.year,stTime.month,stTime.day,stTime.hour,stTime.min,stTime.sec);
    LcdShowStr(0, 0,  Buffer, 0);
    //第三行显示"结束时间"
    Public_ConvertBCDToTime(&stTime,&data[6]);      
    sprintf(Buffer,"结束时间: 20%02d年  %02d-%02d %02d:%02d:%02d",stTime.year,stTime.month,stTime.day,stTime.hour,stTime.min,stTime.sec);
    LcdShowStr(0, 32,  Buffer, 0);
  }    
}
/*************************************************************
** 函数名称: TaskShowTiredDriveRecord
** 功能描述: "超时驾驶记录"菜单
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskShowTiredDriveRecord(LZM_MESSAGE *Msg)
{
  switch(Msg->message)
  {
  case TASK_START:
    LcdClearScreen();
    s_stTaskAttrib.ControlId=0;
    s_stTaskAttrib.ControlMaxId=ShowTiredDriveRecordInit();
    if(s_stTaskAttrib.ControlMaxId==0xff)
    {
      s_stTaskAttrib.ControlId=0xff;                
      LZM_SetAlarm(TEST_TIMER, SECS(2));
    }
    else
    {
      ShowTiredDriveRecord(s_stTaskAttrib.ControlId);
    }
    return 1;
  case KEY_PRESS:
    if(s_stTaskAttrib.ControlId==0xff)break;
    switch(TaskPara)
    {
    case KEY_BACK://返回
      LZM_StartNewTask(TaskMenuDriveRecord);//系统管理
      break;
    case KEY_ENTER://确定	
      //LZM_StartNewTask(TaskMenuMain);//系统管理
      break;                                
    case KEY_UP://登录	
      if(s_stTaskAttrib.ControlId>0)
      {
        s_stTaskAttrib.ControlId--;
        ShowTiredDriveRecord(s_stTaskAttrib.ControlId);
      }
      break;
    case KEY_DOWN://++	
      if(s_stTaskAttrib.ControlId<s_stTaskAttrib.ControlMaxId-1)
      {
        s_stTaskAttrib.ControlId++;
        ShowTiredDriveRecord(s_stTaskAttrib.ControlId);
      }
      break;
    }
    return 1;
  case INT_RTC:
    break;
  case TEST_TIMER:
    LZM_StartNewTask(TaskMenuDriveRecord);//系统管理
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
/*************************************************************
** 函数名称: ShowTiredDriveRecordInit
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned char Show15MinSpeedInit(void)
{
  unsigned char maxcount;
  unsigned char length;
  //读取15分钟速度
  length = Recorder_Stop15MinuteSpeed(GB2312Buffer);
  ////////////////////////
  maxcount = (length-6+2)/3+1;
  //////////////////////////
  return maxcount;
}
/*************************************************************
** 函数名称: TaskShowTiredDriveRecord
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void Show15MinSpeed(unsigned char page)
{
  TIME_T stEndTime;
  unsigned char i,y,index,start;
  char buffer[40];
  //显示15分钟停车速度
  LcdClearScreen();
  if(0 == page)
  {
    Public_ConvertBCDToTime(&stEndTime,&GB2312Buffer[0]);
    sprintf(buffer,"停车时间:20%02d年",stEndTime.year);
    LcdShowStr(0, 0, buffer, 0);
    sprintf(buffer,"%02d-%02d %02d:%02d:%02d",stEndTime.month,stEndTime.day,stEndTime.hour,stEndTime.min,stEndTime.sec);
    LcdShowCaption(buffer, 16);
    y=32;
    index=0;
    
  }
  else
  {
    y = 0;
    index=(page-1)*4+2;
  }  
  ///////////
  start = index*3+6;
  //////////////////
  for(i=0; i<4; i++)
  {               
    sprintf(buffer,"%02d:%02d %3dkm/h",Public_CheckStrIsBCD(GB2312Buffer[start+0]),Public_CheckStrIsBCD(GB2312Buffer[start+1]),GB2312Buffer[start+2]);
    LcdShowCaption(buffer, y);
    index++;
    start +=3;
    y+=16;
    if(index>=s_stTaskAttrib.ControlMaxId-1)
    {
      break;
    }
    if(y>64)
    {
      break;
    }
  }
}
/*************************************************************
** 函数名称: TaskShow15MinSpeed
** 功能描述: "15分钟速度"菜单
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskShow15MinSpeed(LZM_MESSAGE *Msg)
{
  switch(Msg->message)
  {
  case TASK_START:
    LcdClearScreen();
    s_stTaskAttrib.ControlId = 0;
    s_stTaskAttrib.ControlMaxId = Show15MinSpeedInit();
    if(s_stTaskAttrib.ControlMaxId==0xff)
    {
      s_stTaskAttrib.ControlId = 0xff;
      LZM_SetAlarm(TEST_TIMER, SECS(2));
    }
    else
    {
      Show15MinSpeed(s_stTaskAttrib.ControlId);
    }
    return 1;
  case KEY_PRESS:
    if(s_stTaskAttrib.ControlId==0xff)break;
    switch(TaskPara)
    {
    case KEY_BACK://打印
      LZM_StartNewTask(TaskMenuDriveRecord);//系统管理
      break;
    case KEY_ENTER://菜单
      break;                                
    case KEY_UP://登录	
      if(s_stTaskAttrib.ControlId>0)
      {
        s_stTaskAttrib.ControlId--;
        Show15MinSpeed(s_stTaskAttrib.ControlId);
      }
      break;
    case KEY_DOWN://++	
      if(s_stTaskAttrib.ControlId*4<s_stTaskAttrib.ControlMaxId-1)
      {
        s_stTaskAttrib.ControlId++;
        Show15MinSpeed(s_stTaskAttrib.ControlId);
      }
      break;
    }
    return 1;
  case INT_RTC:
    break;
  case TEST_TIMER:
    LZM_StartNewTask(TaskMenuDriveRecord);//系统管理
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
/*************************************************************
** 函数名称: ShowDriveMile
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void ShowDriveMile(void)
{
  unsigned long Mileage;
  char buffer[17];
  Mileage = Public_GetCurTotalMile();   //读取当前总里程    
  LcdShowCaption("总里程", 1);
  LcdShowHorLine(0, 131, 20, 1);
  sprintf(buffer,"%.1f公里",Mileage*0.1);
  LcdShowCaption(buffer,25); 
  LcdShowHorLine(0, 131, 45, 1);
  LcdShowCaption("按<返回>退出", 47);
}
/*************************************************************
** 函数名称: TaskShowDriveMile
** 功能描述: "驾驶里程"菜单
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskShowDriveMile(LZM_MESSAGE *Msg)
{
  switch(Msg->message)
  {
  case TASK_START:
    LcdClearScreen();
    ShowDriveMile();
    return 1;
  case KEY_PRESS:
    switch(TaskPara)
    {
    case KEY_BACK://返回
      LZM_StartNewTask(TaskMenuMain);//系统管理
      break;
    case KEY_ENTER://确定
      //LZM_StartNewTask(TaskMenuMain);//系统管理
      break;                                
    case KEY_UP://--
      break;
    case KEY_DOWN://++
      break;
    }
    return 1;
  case INT_RTC:
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
/*************************************************************
** 函数名称: ShowDriveMile
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void ShowCarInfo(unsigned char page)
{
  const char *c_apcExtCarStatusItems[]=
  {
    "近光灯",
    "远光灯",
    "右转向灯",
    "左转向灯",
    "制动",
    ""
  };
  unsigned char Buffer[24];
  unsigned char i;
  unsigned char temp;
  unsigned char SpeedSensorCoeff;
  unsigned long ulTmp;
  LcdClearScreen();
  //显示
  if(0 == page)
  {
    //第一行显示"车牌号码",最多12字节        
    LcdShowStr(0, 0, "车牌号:", 0);
    //第二行显示具体的车牌号码
    temp = EepromPram_ReadPram(E2_CAR_PLATE_NUM_ID, Buffer);
    for(i=0; i<temp; i++)
    {
      if(0 != Buffer[i])
      {
        break;
      }
    }
    Buffer[temp]='\0';        
    //这一行剩余空间显示空格
    LcdShowStr(60, 0, (char *)Buffer+i, 0);	
    //第三行显示"车辆VIN",最多17字节
    LcdShowStr(0, 16, "车牌颜色:", 0);
    temp = EepromPram_ReadPram(E2_CAR_PLATE_COLOR_ID, Buffer);
    if(temp)
    {
      temp=Buffer[0];
      if(9==temp)
      {
        LcdShowStr(80, 16, c_apcCarPlakeColorItems[4], 0);    
      }
      else
        if(temp&&temp<5)
        {
          LcdShowStr(80, 16, c_apcCarPlakeColorItems[temp-1], 0);                
        }
        else
          if(0==temp)
          {
            LcdShowStr(80, 16, c_apcCarPlakeColorItems[5], 0);   
          }
    }
    
    LcdShowStr(0, 32, "车牌分类: ", 0);
    //第四行显示具体的VIN
    temp=EepromPram_ReadPram(E2_CAR_TYPE_ID, Buffer);
    Buffer[temp]='\0';
    LcdShowStr(0, 48, (char *)Buffer, 0);
  }
  else
    if(1 == page)
    {
      LcdShowStr(0, 0, "车辆VIN码:", 0);        
      i=EepromPram_ReadPram(E2_CAR_IDENTIFICATION_CODE_ID,(unsigned char*)Buffer);
      if(i&&Buffer[0]>=0x30&&Buffer[0]<0x80)
      {
        Buffer[17]='\0';
        LcdShowStr(0, 16,(char *)Buffer, 0);
      }
    }
    else
      if(2 == page)
      {
        //第一行显示"脉冲系数"
        LcdShowStr(0, 0, "脉冲系数:", 0);
        //第二行显示具体的脉冲系数 
        EepromPram_ReadPram(E2_SPEED_SENSOR_COEF_ID, &SpeedSensorCoeff);
        temp = EepromPram_ReadPram(E2_CAR_FEATURE_COEF_ID, Buffer);
        ulTmp = 0;
        for(i = 0; i < temp; i++)
        {
          ulTmp <<= 8;
          ulTmp |= Buffer[i];
        }
        ///////////////////
        ulTmp = ulTmp;
        sprintf((char *)Buffer,"%d转/公里",ulTmp);
        LcdShowStr(0, 16,(char *)Buffer, 0);
        //第三行显示"报警车速"
        LcdShowStr(0, 32, "报警车速:", 0);
        //第四行显示具体的报警车速
        temp = SpeedMonitor_GetSpeedMaxLimitVal();//modify by joneming 20130312 
        sprintf((char *)Buffer,"%dkm/h",temp);
        LcdShowStr(0, 48,(char *)Buffer, 0);
      }
      else
      {
        LcdShowCaption("当前状态信号值:",0);
        temp = 0;
        for(i=0; i<5;i++)
        {
          if(Io_ReadExtCarStatusBit((CAR_STATUS_BIT)i))
          {
            LcdShowStr(0+64*(temp%2), 16+16*(temp/2),c_apcExtCarStatusItems[i], 0);
            temp++;
          }
        }
      }
}
/*************************************************************
** 函数名称: TaskShowCarInfo
** 功能描述: "车辆信息"菜单
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskShowCarInfo(LZM_MESSAGE *Msg)
{
  switch(Msg->message)
  {
  case TASK_START:
    s_stTaskAttrib.ControlId=0;
    s_stTaskAttrib.ControlMaxId=3;
    ShowCarInfo(s_stTaskAttrib.ControlId);
    return 1;
  case KEY_PRESS:
    switch(TaskPara)
    {
    case KEY_BACK://返回
      LZM_StartNewTask(TaskMenuDriveRecord);//系统管理
      break; 
    case KEY_ENTER://确定	
      //LZM_StartNewTask(TaskMenuMain);//系统管理
      break;                               
    case KEY_UP://--	
      if(s_stTaskAttrib.ControlId>0)
      {
        s_stTaskAttrib.ControlId--;
        ShowCarInfo(s_stTaskAttrib.ControlId);
      }
      break;
    case KEY_DOWN://++	
      if(s_stTaskAttrib.ControlId<s_stTaskAttrib.ControlMaxId)
      {
        s_stTaskAttrib.ControlId++;
        ShowCarInfo(s_stTaskAttrib.ControlId);
      }
      break;
    }
    return 1;
  case INT_RTC:
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}

/*************************************************************
** 函数名称: ShowDriveMile
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void ShowDriverInfo(void)
{
  unsigned char buffer[30];
  //////////////////////
  if(0 == TiredDrive_GetLoginFlag())//没登陆
  {
    LcdShowCaption("没有登录!", 23);
    return;
  }
  /////////////////////////////
  TiredDrive_GetCurdriverLicence(buffer);
  LcdShowCaption("驾驶证号码:",0);
  LcdShowHorLine(0, 131, 17, 1);
  LcdShowStr(0,20,(char *)buffer,0);
#if 0
  len =EepromPram_ReadPram(E2_ALL_DRIVER_INFORMATION_ID, buffer);
  if(len) //如果读取成功
  {
    if(0x10==buffer[0])//汽车行驶记录仪
    {
      STRUCT_IC_CARD_DRIVER stTmpICDriver;
      /*** EEPROM内北斗新行标IC卡数据域格式定义的完整的字节流提取出从业资格证IC卡数据到gCardRoadTransport 相应的域 ***/       
      ICCard_Pack_Extract_VehicleRecorder_Driver ((STRUCT_PROTOCOL_IC_CARD *)buffer,len , &stTmpICDriver);
      len =0;
      memcpy(&buffer[len],stTmpICDriver.IC_Card_DriverID,18);
      len += 18;
      buffer[len++]=' ';
      temp[0]=Public_CheckStrIsBCD(stTmpICDriver.IC_Card_ValidityPeriod_Year);
      temp[1]=Public_CheckStrIsBCD(stTmpICDriver.IC_Card_ValidityPeriod_Month);
      temp[2]=Public_CheckStrIsBCD(stTmpICDriver.IC_Card_ValidityPeriod_Day);
      sprintf((char *)&buffer[len],"%02d-%2d-%2d",temp[0],temp[1],temp[2]);
      //////////////////////////
      LcdShowStr(0,0,(char *)buffer,0);
    }
    else
      if(0x20==buffer[0])//IC卡从业资格证卡
      {
        STRUCT_IC_CARD_ROADTRANSPORT stTmpICard;
        /*** EEPROM内北斗新行标IC卡数据域格式定义的完整的字节流提取出从业资格证IC卡数据到gCardRoadTransport 相应的域 ***/       
        ICCard_Pack_Extract_RoadTransportCard(buffer, len, &stTmpICard);
        len =0;
        memcpy(&buffer[len],stTmpICard.IC_Card_DriverName,stTmpICard.IC_Card_DriverNameLength);
        len += stTmpICard.IC_Card_DriverNameLength;
        buffer[len++]=' ';
        memcpy(&buffer[len],stTmpICard.IC_Card_QualificationCertificateID,18);
        len += 18;
        buffer[len++]=' ';
        memcpy(&buffer[len],stTmpICard.IC_Card_IssueOrganization,stTmpICard.IC_Card_IssueOrganizationLength);
        len += stTmpICard.IC_Card_IssueOrganizationLength;
        //////////////////////
        if(len > 53)len = 53;
        //////////////////////
        buffer[len++]=' ';
        ////////////////////////////
        temp[0]=Public_CheckStrIsBCD(stTmpICard.IC_Card_ValidityPeriod_Year[0]);
        temp[1]=Public_CheckStrIsBCD(stTmpICard.IC_Card_ValidityPeriod_Year[1]);
        temp[2]=Public_CheckStrIsBCD(stTmpICard.IC_Card_ValidityPeriod_Month);
        temp[3]=Public_CheckStrIsBCD(stTmpICard.IC_Card_ValidityPeriod_Day);
        sprintf((char *)&buffer[len],"%02d%02d-%2d-%2d",temp[0],temp[1],temp[2],temp[3]);
        //////////////////////////
        LcdShowStr(0,0,(char *)buffer,0);
      }
      else
      {
        LcdShowCaption("数据未定义!", 23);
      }
  }
  else
  {
    LcdShowCaption("数据出错!", 23);
  } 
#endif
}
/*************************************************************
** 函数名称: TaskShowDriverInfo
** 功能描述: "驾驶员信息"菜单
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskShowDriverInfo(LZM_MESSAGE *Msg)
{
  switch(Msg->message)
  {
  case TASK_START:
    LcdClearScreen();
    ShowDriverInfo();
    return 1;
  case KEY_PRESS:
    switch(TaskPara)
    {
    case KEY_BACK://返回
      LZM_StartNewTask(TaskMenuDriveRecord);//系统管理
      break; 
    case KEY_ENTER://确定	
      //LZM_StartNewTask(TaskMenuMain);//系统管理
      break;                               
    case KEY_UP://--
      break;
    case KEY_DOWN://++
      break;
    }
    return 1;
  case INT_RTC:
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
/*************************************************************
** 函数名称: ShowProductInfo
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void ShowProductInfo(unsigned char page)
{
  unsigned char i,index,max;
  char Buffer[20];
  index=page*4;
  LcdClearScreen();
  //显示
  max=0;
  if(PRODUCT_ITEMS>index)
  {
    max=(PRODUCT_ITEMS-index);
  }
  //////////////
  if(max>4)max=4;
  /////////////
  for(i=0; i<max; i++)
  {
    if(3==index)
    {
      char name[20];
      GetCurentProductModel(name);
      LcdShowStr(0, i*16,name, 0);
      ///////////////////
      index++;
    }
#ifdef __DATE__
    else
      if(7==index)
      {
        LcdShowStr(67,  32,__TIME__, 0);
        LcdShowStr(0, i*16,__DATE__, 0);
        ///////////////////
        index++;
      }
#endif
      else
      {
        if(4 == index)//dxl,2014.10.28,软件版本V1.02或V1.03自动根据网络制式调整
        {
          memcpy(Buffer,ProductInfo[4],9);
          Buffer[9] = 0;
          if(0x01 == (0x01&Modem_Api_Type()))
          {
            strcat(Buffer,"V1.03");
          }
          else if(0x02 == (0x02&Modem_Api_Type()))
          {
            strcat(Buffer,"V1.02"); 
          }
          LcdShowStr(0, i*16, Buffer, 0);
          index++;
        }
        else
        {
          LcdShowStr(0, i*16, ProductInfo[index++], 0);//modify by joneming 20130312 
        }
      }
  }
}
/*************************************************************
** 函数名称: TaskShowProductInfo
** 功能描述: "产品信息"菜单
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskShowProductInfo(LZM_MESSAGE *Msg)
{
  switch(Msg->message)
  {
  case TASK_START:
    s_stTaskAttrib.ControlId=0;
    s_stTaskAttrib.ControlMaxId=PRODUCT_ITEMS;
    ShowProductInfo(s_stTaskAttrib.ControlId);
    return 1;
  case KEY_PRESS:
    switch(TaskPara)
    {
    case KEY_BACK://返回
      LZM_StartNewTask(TaskMenuMain);//系统管理
      break;
    case KEY_ENTER://确定	
      //LZM_StartNewTask(TaskMenuMain);//系统管理
      break;               
    case KEY_UP://--	
      if(s_stTaskAttrib.ControlId>0)
      {
        s_stTaskAttrib.ControlId--;
        ShowProductInfo(s_stTaskAttrib.ControlId);
      }
      break;
    case KEY_DOWN://++	
      if((s_stTaskAttrib.ControlId+1)*4<s_stTaskAttrib.ControlMaxId)
      {
        s_stTaskAttrib.ControlId++;
        ShowProductInfo(s_stTaskAttrib.ControlId);
      }
      break;
    }
    return 1;
  case INT_RTC:
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
/*************************************************************
** 函数名称: ShowSIMCardNumber
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void ShowSIMCardNumber(void)
{
  unsigned char buffer[30],data[20];    
  LcdClearScreen();
  LcdShowCaption("SIM卡ICCID号",0); 
  LcdShowHorLine(0, 131, 17, 1);
  MOD_GetIccidBcdValue(data);
  Public_ConvertBcdToAsc(buffer,data,10);
  buffer[20]='\0';
  LcdShowStr(0,20,(char *)buffer,0);
}
/*************************************************************
** 函数名称: TaskShowSIMCardNumber
** 功能描述: "SIM卡号"菜单
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskShowSIMCardNumber(LZM_MESSAGE *Msg)
{
  switch(Msg->message)
  {
  case TASK_START:            
    ShowSIMCardNumber();
    LZM_SetAlarm(TEST_TIMER,SECS(10));
    return 1;
  case KEY_PRESS:
    switch(TaskPara)
    {
    case KEY_BACK://返回
      LZM_StartNewTask(TaskMenuMain);//系统管理
      break; 
    case KEY_ENTER://确定
      break;                               
    case KEY_UP://--
      break;
    case KEY_DOWN://++
      break;
    }
    return 1;
  case INT_RTC:
    break;
  case TEST_TIMER:
    LZM_StartNewTask(TaskMenuMain);//系统管理
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
/*************************************************************
** 函数名称: TaskShowMainInterface
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void LcdShowTtsVolume(unsigned char Volume)
{
  unsigned char i,x,xx,y,yy,width,hight;
  width = 10;
  hight = 5;
  y = 62;
  x = 6; 
  xx = x + width;
  yy = y - hight;
  LcdClearArea(0,19,130,63);
  for(i=0; i< 8; i++)
  {
    if(i<Volume)
    {
      LcdReverseBar(x,yy,xx,y);
    }
    else
    {
      LcdShowRect(x,yy,xx,y,0);
    }
    x = xx + 5 ;
    xx = x + width;
    yy = yy - hight;
  }
}
/*************************************************************
** 函数名称: TaskShowMainInterface
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void AdjustAndShowTTsVolume(unsigned char flag)
{
  if(1==flag)
  {
    SetTtsVolumeIncr();
  }
  else
    if(2==flag)
    {
      SetTtsVolumeDecr();
    } 
  ////////////////////
  if(flag)
    LZM_SetAlarm(TEST_TIMER, PUBLICSECS(0.3));
  Public_ShowValue(104, 0,"v:%d", GetTtsVolume());
  LcdShowTtsVolume(GetTtsVolume());
}
/*************************************************************
** 函数名称: TaskShowSIMCardNumber
** 功能描述: "SIM卡号"菜单
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskShowAdjustTtsVolume(LZM_MESSAGE *Msg)
{
  switch(Msg->message)
  {
  case TASK_START:            
    LcdClearScreen();
    LcdShowStr(10,0,"调节TTS音量",0); 
    LcdShowHorLine(0, 131, 17, 1);
    AdjustAndShowTTsVolume(0);
    return 1;
  case KEY_PRESS:
    switch(TaskPara)
    {
    case KEY_BACK://返回
      LZM_StartNewTask(TaskMenuMain);//系统管理
      break; 
    case KEY_ENTER://确定
      break;                               
    case KEY_UP://--
      AdjustAndShowTTsVolume(1);
      break;
    case KEY_DOWN://++
      AdjustAndShowTTsVolume(2);
      break;
    }
    return 1;
  case INT_RTC:
    break;
  case TEST_TIMER:
    Public_PlayTTSVoiceStr("调节音量");
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
/*************************************************************
** 函数名称: TaskLcdBacklight
** 功能描述: "背光控制"菜单
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskLcdBacklight(LZM_MESSAGE *Msg)
{
  static char *DispBuffer[]={"默认","常亮",""};	//电池电压类型
  unsigned char handle;
  switch(Msg->message)
  {
  case TASK_START:
    LcdClearScreen();           
    LcdShowCaption("背光控制",0); 
    LcdShowStr(2, 23, "背光选择:", 0);  
    LcdShowHorLine(0, 131, 18, 1);
    LcdShowHorLine(0, 131, 45, 1);
    LcdShowCaption("按<确认>保存",47);
    SpinInit(&s_stSpin[0],80,23,DispBuffer,"");
    s_stSpin[0].handle=s_stTaskAttrib.BacklightFlag;
    SpinShow(&s_stSpin[0]);
    SpinSetFocus(&s_stSpin[0]);
    s_stTaskAttrib.ControlId=0;
    return 1;
  case KEY_PRESS:
    if(s_stTaskAttrib.ControlId==0xff)
    {
      LZM_StartNewTask(TaskMenuSystem);//系统管理
      break;
    }
    switch(TaskPara)
    {
    case KEY_BACK://返回
      LZM_StartNewTask(TaskMenuSystem);//系统管理
      break;
    case KEY_ENTER://确定
      handle=SpinGetHandle(&s_stSpin[0]);
      if(handle!=s_stTaskAttrib.BacklightFlag)
      {
        s_stTaskAttrib.BacklightFlag=handle;
        LcdClearArea(0,19,131,44);
        LcdShowCaption("正在保存..", 23);                        
        SaveSystemBacklightFlag();
        LcdClearArea(0,19,131,44);                         
        LcdShowCaption(s_stTaskAttrib.BacklightFlag?"已改为常亮设置":"已改为默认设置", 23); 
        LZM_SetAlarm(TEST_TIMER, SECS(1));
      }
      else
      {
        LZM_SetAlarm(TEST_TIMER, SECS(0.5));
      }
      s_stTaskAttrib.ControlId=0xff; 
      break;                                
    default:                    
      SpinResponse(&s_stSpin[0],TaskPara);  
      break;
    }
    return 1;
  case TEST_TIMER:
    LZM_StartNewTask(TaskMenuSystem);//系统管理
    break;
  case INT_RTC:
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
/*************************************************************
** 函数名称: ShowDriveMile
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void ShowQueryServerParameter(unsigned char page)
{
  unsigned char buf[30];
  unsigned long val;
  LcdClearScreen();
  //显示
  if(0 == page)
  {
    LcdShowStr(0,0,"主IP:",0);
    val=EepromPram_ReadPram(E2_MAIN_SERVER_IP_ID, buf);
    buf[val]='\0';
    LcdShowStr(0,16,(char *)buf,0);        
    LcdShowStr(0,32,"备份IP:",0);  
    val=EepromPram_ReadPram(E2_BACKUP_SERVER_IP_ID, buf);
    buf[val]='\0';
    LcdShowStr(0,48,(char *)buf,0);
  }
  else
    if(1 == page)
    {
      LcdShowStr(0,0,"APN:",0);
      val=EepromPram_ReadPram(E2_MAIN_SERVER_APN_ID, buf);
      buf[val]='\0';
      LcdShowStr(0,16,(char *)buf,0);
      LcdShowStr(0,32,"备份APN:",0);
      val=EepromPram_ReadPram(E2_BACKUP_SERVER_APN_ID, buf);
      buf[val]='\0';
      LcdShowStr(0,48,(char *)buf,0);
    }
    else
      if(2 == page)
      {
        LcdShowStr(0,0,"TCP端口号:",0);         
        val=EepromPram_ReadPram(E2_MAIN_SERVER_TCP_PORT_ID, buf);
        if(val)
        {
          val=ConvertReadPramToValue(E2_MAIN_SERVER_TCP_PORT_ID);
          sprintf((char *)buf,"%d",val);
          LcdShowStr(0,16,(char *)buf,0);
        }
        LcdShowStr(0,32,"UDP端口号:",0);
        val=EepromPram_ReadPram(E2_MAIN_SERVER_UDP_PORT_ID, buf);
        if(val)
        {
          val=ConvertReadPramToValue(E2_MAIN_SERVER_UDP_PORT_ID);
          sprintf((char *)buf,"%d",val);
          LcdShowStr(0,48,(char *)buf,0); 
        }
        
      }
      else
        if(3 == page)
        {
          LcdShowStr(0,0,"主中心域名",0);
          val=Public_ReadDataFromFRAM(FRAM_MAIN_DOMAIN_NAME_ADDR, buf,FRAM_MAIN_DOMAIN_NAME_LEN);
          buf[val]= 0;
          LcdShowStr(0, 16, (char *)buf, 0);
          LcdShowStr(0,32,"备份中心域名",0);
          val=Public_ReadDataFromFRAM(FRAM_BACKUP_DOMAIN_NAME_ADDR, buf,FRAM_BACKUP_DOMAIN_NAME_LEN);
          buf[val]= 0;
          LcdShowStr(0, 48, (char *)buf, 0);
        }
        else
          if(4 == page)
          {
            LcdShowStr(0,0,"IP2:",0);
            val=EepromPram_ReadPram(E2_SECOND_MAIN_SERVER_IP_ID, buf);
            buf[val]='\0';
            LcdShowStr(0,16,(char *)buf,0);        
            LcdShowStr(0,32,"备份IP2:",0);  
            val=EepromPram_ReadPram(E2_SECOND_BACKUP_SERVER_IP_ID, buf);
            buf[val]='\0';
            LcdShowStr(0,48,(char *)buf,0);
          }
          else
            if(5 == page)
            {
              LcdShowStr(0,0,"TCP2:",0);
              val=EepromPram_ReadPram(E2_SECOND_MAIN_SERVER_TCP_PORT_ID, buf);
              if(val)
              {
                val=ConvertReadPramToValue(E2_SECOND_MAIN_SERVER_TCP_PORT_ID);
                sprintf((char *)buf,"%d",val);
                LcdShowStr(0,16,(char *)buf,0);
              }
              LcdShowStr(0,32,"UDP2:",0);
              val=EepromPram_ReadPram(E2_SECOND_MAIN_SERVER_UDP_PORT_ID, buf);
              if(val)
              {
                val=ConvertReadPramToValue(E2_SECOND_MAIN_SERVER_UDP_PORT_ID);
                sprintf((char *)buf,"%d",val);
                LcdShowStr(0,48,(char *)buf,0); 
              }        
            }
}
/*************************************************************
** 函数名称: TaskSystemServerSetIP
** 功能描述: "设置主IP"菜单
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskSystemQueryServerParameter(LZM_MESSAGE *Msg)
{
  switch(Msg->message)
  {
  case TASK_START: 
    s_stTaskAttrib.ControlId=0;  
    s_stTaskAttrib.ControlMaxId = 5;
    ShowQueryServerParameter(s_stTaskAttrib.ControlId);
    return 1;
  case KEY_PRESS: 
    switch(TaskPara)
    {
    case KEY_BACK://返回
      LZM_StartNewTask(TaskMenuSystemManageServer);
      break;
    case KEY_ENTER://确定                    
      break;
    case KEY_UP://--	
      if(s_stTaskAttrib.ControlId>0)
      {
        s_stTaskAttrib.ControlId--;
        ShowQueryServerParameter(s_stTaskAttrib.ControlId);
      }
      break;
    case KEY_DOWN://++	
      if(s_stTaskAttrib.ControlId<s_stTaskAttrib.ControlMaxId)
      {
        s_stTaskAttrib.ControlId++;
        ShowQueryServerParameter(s_stTaskAttrib.ControlId);
      }
      break;
    default: 
      break;
    }
    return 1;
  case INT_RTC:
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
/*************************************************************
** 函数名称: TaskSystemServerSetIP
** 功能描述: "设置主IP"菜单
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskSystemServerSetIP(LZM_MESSAGE *Msg)
{
  unsigned char val,buf[16];
  unsigned char i,tmpbuf[16];
  unsigned char temp[4];
  unsigned short tmp;
  switch(Msg->message)
  {
  case TASK_START:            
    val=EepromPram_ReadPram(E2_MAIN_SERVER_IP_ID, buf);
    LcdClearScreen();
    s_stTaskAttrib.ControlId=0;  
    s_stTaskAttrib.ControlMaxId =3;
    LcdShowStr(0,0,"当前主IP:",0);                
    buf[val]='\0';
    LcdShowStr(0,16,(char *)buf,0);
    LcdShowStr(0,32,"确认修改为:",0);
    if(!IsBrushAdministratorCard())
    {
      ConvertStrToIP(buf,s_stServerParam.mainIP,val);
    }
    for(i=0; i<=s_stTaskAttrib.ControlMaxId; i++)
    {
      ESpinInit(&s_stESpin[i], 3+i*33, 48, 3, s_stServerParam.mainIP[i]);
      s_stESpin[i].loop =0;
      ESpinShow(&s_stESpin[i]);
      if(i<s_stTaskAttrib.ControlMaxId)LcdShowStr(29+i*33, 48, ".", 0); 
    }            
    ESpinSetFocus(&s_stESpin[s_stTaskAttrib.ControlId]);
    
    return 1;
  case KEY_PRESS: 
    if(s_stTaskAttrib.ControlId==0xff)
    {
      LZM_SetAlarm(TEST_TIMER, 1);
      break;
    }
    switch(TaskPara)
    {
    case KEY_BACK://返回
      LZM_StartNewTask(TaskMenuSystemServer);//系统管理
      break;
    case KEY_ENTER://确定
      for( i=0; i<=s_stTaskAttrib.ControlMaxId; i++)
      {
        tmp=atoi(s_stESpin[i].buffer);
        if(tmp>255)
        {
          LcdCopyScreen();
          LcdClearScreen();
          LcdShowCaption("格式错误", 24);
          LcdPasteScreen();
          ESpinLoseFocus(&s_stESpin[s_stTaskAttrib.ControlId]);
          s_stTaskAttrib.ControlId=i;
          ESpinSetFocus(&s_stESpin[s_stTaskAttrib.ControlId]);
          return 0;
        }
        temp[i]=tmp;
      }
      LcdClearScreen();
      LcdShowCaption("正在保存...", 24);
      sprintf((char*)buf,"%d.%d.%d.%d",temp[0],temp[1],temp[2],temp[3]);
      val=strlen((char*)buf);
      EepromPram_WritePram(E2_MAIN_SERVER_IP_ID, buf,val);
      EepromPram_ReadPram(E2_MAIN_SERVER_IP_ID, tmpbuf);
      LcdClearArea(0,19,131,44);
      if(strncmp((char*)buf,(char*)tmpbuf,val)==0)
      {
        LcdShowCaption("保存成功", 24);
        EepromPram_UpdateVariable(E2_MAIN_SERVER_IP_ID);
      }
      else
      {
        LcdShowCaption("保存错误,请检查", 24); 
        
      }
      s_stTaskAttrib.ControlId=0xff;
      LZM_SetAlarm(TEST_TIMER, SECS(1));
      break;                                
    default:                    
      val=ESpinResponse(&s_stESpin[s_stTaskAttrib.ControlId],TaskPara); 
      if(val)
      {
        ESpinLoseFocus(&s_stESpin[s_stTaskAttrib.ControlId]);
        if(s_stTaskAttrib.ControlId<s_stTaskAttrib.ControlMaxId)s_stTaskAttrib.ControlId++;
        else s_stTaskAttrib.ControlId = 0;
        ESpinSetFocus(&s_stESpin[s_stTaskAttrib.ControlId]);
      }
      break;
    }
    return 1;           
  case TEST_TIMER:
    LZM_StartNewTask(TaskMenuSystemServer);//系统管理
    break;
  case INT_RTC:
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}

/*************************************************************
** 函数名称: TaskSystemServerSetAPN
** 功能描述: "设置主APN"菜单
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskSystemServerSetAPN(LZM_MESSAGE *Msg)
{    
  unsigned char buf[20];
  unsigned char val;
  unsigned char i;
  switch(Msg->message)
  {
  case TASK_START:
    val=EepromPram_ReadPram(E2_MAIN_SERVER_APN_ID, buf);
    LcdClearScreen();
    s_stTaskAttrib.ControlId = 0xff; 
    buf[val]='\0';            
    LcdShowStr(0,3,"APN:",0);  
    LcdShowStr(32,3,(char *)buf,0);
    if(val>16)
    {
      LcdShowStr((val-12)*8+4,24,"修改为:",0);
    }
    else
      if(val>12)
      {
        LcdShowStr((val-12)*8+4,24,"确认修改为:",0);
      }
      else
        LcdShowStr(0,24,"确认修改为:",0);
    ////////////////////////////
    if(!IsBrushAdministratorCard())
    {        
      strcpy(s_stServerParam.APN,(char *)buf);
    }
    /////////////////////
    for(i=0;;i++)
    {
      if(strncmp(s_stServerParam.APN,(char*)s_apcAPNItems[i],5)==0)
      {
        s_stTaskAttrib.ControlId=i;
        break;
      }
      else 
        if(strcmp((char*)s_apcAPNItems[i],"")==0)
        {
          s_apcAPNItems[i]=s_stServerParam.APN;
          s_stTaskAttrib.ControlId=0x55;
          break;
        }
    }
    
    ////////////////////////
    SpinInit(&s_stSpin[0],4,45,(char **)s_apcAPNItems,"");
    s_stSpin[0].handle = i;
    SpinShow(&s_stSpin[0]);            
    SpinSetFocus(&s_stSpin[0]);
    return 1;
  case KEY_PRESS:
    if(s_stTaskAttrib.ControlId==0xff)
    {
      LZM_SetAlarm(TEST_TIMER, 1);
      break;
    }
    switch(TaskPara)
    {
    case KEY_BACK://返回
      LZM_StartNewTask(TaskMenuSystemServer);//系统管理
      break;
    case KEY_ENTER://确定
      if(s_stTaskAttrib.ControlId!=s_stSpin[0].handle)
      {
        val=strlen(s_stSpin[0].item[s_stSpin[0].handle]);
        EepromPram_WritePram(E2_MAIN_SERVER_APN_ID, (unsigned char *)s_stSpin[0].item[s_stSpin[0].handle],val);
        EepromPram_ReadPram(E2_MAIN_SERVER_APN_ID, buf);
        LcdClearScreen();
        if(strncmp((char*)buf,(char*)s_stSpin[0].item[s_stSpin[0].handle],val)==0)
        {
          LcdShowCaption("保存成功", 24);
          EepromPram_UpdateVariable(E2_MAIN_SERVER_APN_ID);
        }
        else
        {
          LcdShowCaption("保存错误,请检查", 24); 
          
        }
        LZM_SetAlarm(TEST_TIMER, SECS(1));
      }
      else
      {
        
        LZM_SetAlarm(TEST_TIMER, SECS(0.5));
      }                    
      break;                                
    default:
      SpinResponse(&s_stSpin[0],TaskPara);
      break;
    }
    return 1;           
  case TEST_TIMER:
    LZM_StartNewTask(TaskMenuSystemServer);//系统管理
    break;
  case INT_RTC:
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
/*************************************************************
** 函数名称: TaskSystemServerSetIP
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskSystemServerSetBackUpIP(LZM_MESSAGE *Msg)
{
  unsigned char buf[16];
  unsigned char val;       
  unsigned char temp[4],tmpbuf[16],i;
  unsigned short tmp;
  switch(Msg->message)
  {
  case TASK_START:            
    val=EepromPram_ReadPram(E2_BACKUP_SERVER_IP_ID, buf);
    LcdClearScreen();
    s_stTaskAttrib.ControlId=0;  
    s_stTaskAttrib.ControlMaxId =3;
    LcdShowStr(0,0,"当前备份IP:",0);                
    buf[val]='\0';
    LcdShowStr(0,16,(char *)buf,0);
    LcdShowStr(0,32,"确认修改为:",0);
    if(!IsBrushAdministratorCard())
    {
      ConvertStrToIP(buf, s_stServerParam.backIP,val);
    }
    ///////////////////
    for( i=0; i<=s_stTaskAttrib.ControlMaxId; i++)
    {
      ESpinInit(&s_stESpin[i], 3+i*33, 48, 3, s_stServerParam.backIP[i]);
      s_stESpin[i].loop =0;
      ESpinShow(&s_stESpin[i]);
      if(i<s_stTaskAttrib.ControlMaxId)LcdShowStr(29+i*33, 48, ".", 0); 
    }            
    ESpinSetFocus(&s_stESpin[s_stTaskAttrib.ControlId]);
    return 1;
  case KEY_PRESS:
    if(s_stTaskAttrib.ControlId==0xff)
    {
      LZM_SetAlarm(TEST_TIMER, 1);
      break;
    }
    switch(TaskPara)
    {
    case KEY_BACK://返回
      LZM_StartNewTask(TaskMenuSystemServer);//系统管理
      break;
    case KEY_ENTER://确定
      for( i=0; i<=s_stTaskAttrib.ControlMaxId; i++)
      {
        tmp=atoi(s_stESpin[i].buffer);
        if(tmp>255)
        {
          LcdCopyScreen();
          LcdClearScreen();
          LcdShowCaption("格式错误", 24);    
          LcdPasteScreen();
          ESpinLoseFocus(&s_stESpin[s_stTaskAttrib.ControlId]);
          s_stTaskAttrib.ControlId=i;
          ESpinSetFocus(&s_stESpin[s_stTaskAttrib.ControlId]);
          return 0;
        }
        temp[i]=tmp;
      }
      LcdClearScreen();
      LcdShowCaption("正在保存...", 24);
      sprintf((char*)buf,"%d.%d.%d.%d",temp[0],temp[1],temp[2],temp[3]);
      val=strlen((char*)buf);
      EepromPram_WritePram(E2_BACKUP_SERVER_IP_ID, buf,val);
      EepromPram_ReadPram(E2_BACKUP_SERVER_IP_ID, tmpbuf);
      LcdClearArea(0,19,131,44);
      if(strncmp((char*)buf,(char*)tmpbuf,val)==0)
      {
        LcdShowCaption("保存成功", 24);
        EepromPram_UpdateVariable(E2_BACKUP_SERVER_IP_ID);
      }
      else
      {
        LcdShowCaption("保存错误,请检查", 24); 
        
      }
      s_stTaskAttrib.ControlId=0xff;
      LZM_SetAlarm(TEST_TIMER, SECS(1));
      break;                                
    default:
      val=ESpinResponse(&s_stESpin[s_stTaskAttrib.ControlId],TaskPara); 
      if(val)
      {
        ESpinLoseFocus(&s_stESpin[s_stTaskAttrib.ControlId]);
        if(s_stTaskAttrib.ControlId<s_stTaskAttrib.ControlMaxId)s_stTaskAttrib.ControlId++;
        else s_stTaskAttrib.ControlId = 0;
        ESpinSetFocus(&s_stESpin[s_stTaskAttrib.ControlId]);
      }
      break;
    }
    return 1;           
  case TEST_TIMER:
    LZM_StartNewTask(TaskMenuSystemServer);//系统管理
    break;
  case INT_RTC:
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}

/*************************************************************
** 函数名称: TaskSystemServerSetTCPPort
** 功能描述: "设置TCP端口号"菜单
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskSystemServerSetTCPPort(LZM_MESSAGE *Msg)
{
  unsigned long val;
  unsigned long val1;
  unsigned char buf[6];
  switch(Msg->message)
  {
  case TASK_START:            
    val=ConvertReadPramToValue(E2_MAIN_SERVER_TCP_PORT_ID);
    LcdClearScreen();
    s_stTaskAttrib.ControlId=0;
    LcdShowStr(0,0,"当前TCP端口号:",0);                
    sprintf(s_stTaskAttrib.ShowBuffer[0],"%d",val);
    LcdShowStr(3,16,s_stTaskAttrib.ShowBuffer[0],0);
    LcdShowStr(0,32,"确认修改为:",0);
    if(IsBrushAdministratorCard())
    {
      val=s_stServerParam.tcp;
    }           
    ESpinInit(&s_stESpin[s_stTaskAttrib.ControlId], 3, 48, 8,val);
    ESpinShow(&s_stESpin[s_stTaskAttrib.ControlId]);
    
    ESpinSetFocus(&s_stESpin[s_stTaskAttrib.ControlId]);
    return 1;
  case KEY_PRESS:
    if(s_stTaskAttrib.ControlId==0xff)
    {
      LZM_SetAlarm(TEST_TIMER, 1);
      break;
    }
    switch(TaskPara)
    {
    case KEY_BACK://返回
      LZM_StartNewTask(TaskMenuSystemServer);//系统管理
      break; 
    case KEY_ENTER://确定
      LcdClearScreen();
      LcdShowCaption("正在保存...", 24);
      val=atol(s_stESpin[s_stTaskAttrib.ControlId].buffer);
      Public_ConvertLongToBuffer(val,buf);
      EepromPram_WritePram(E2_MAIN_SERVER_TCP_PORT_ID, buf,4);                    
      val1=ConvertReadPramToValue(E2_MAIN_SERVER_TCP_PORT_ID);
      LcdClearArea(0,19,131,44);
      if(val==val1)
      {
        LcdShowCaption("保存成功", 24);
        EepromPram_UpdateVariable(E2_MAIN_SERVER_TCP_PORT_ID);
      }
      else
      {
        LcdShowCaption("保存错误,请检查", 24); 
        
      }
      s_stTaskAttrib.ControlId=0xff;
      LZM_SetAlarm(TEST_TIMER, SECS(1));
      break;                               
    default:
      ESpinResponse(&s_stESpin[s_stTaskAttrib.ControlId],TaskPara);
      break;
    }
    return 1;         
  case TEST_TIMER:
    LZM_StartNewTask(TaskMenuSystemServer);//系统管理
    break;
  case INT_RTC:
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}

/*************************************************************
** 函数名称: TaskSystemServerSetUDPPort
** 功能描述: "设置UDP端口号"菜单
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskSystemServerSetUDPPort(LZM_MESSAGE *Msg)
{
  unsigned long val;
  unsigned long val1;
  unsigned char buf[6];
  switch(Msg->message)
  {
  case TASK_START:            
    val=ConvertReadPramToValue(E2_MAIN_SERVER_UDP_PORT_ID);
    LcdClearScreen();
    s_stTaskAttrib.ControlId=0;
    LcdShowStr(0,0,"当前UDP端口号:",0);                
    sprintf(s_stTaskAttrib.ShowBuffer[0],"%d",val);
    LcdShowStr(3,16,s_stTaskAttrib.ShowBuffer[0],0);
    LcdShowStr(0,32,"确认修改为:",0);
    if(IsBrushAdministratorCard())
    {
      val=s_stServerParam.udp;
    }            
    ESpinInit(&s_stESpin[s_stTaskAttrib.ControlId], 3, 48, 8,val);
    ESpinShow(&s_stESpin[s_stTaskAttrib.ControlId]);            
    ESpinSetFocus(&s_stESpin[s_stTaskAttrib.ControlId]);
    return 1;
  case KEY_PRESS:
    if(s_stTaskAttrib.ControlId==0xff)
    {
      LZM_SetAlarm(TEST_TIMER, 1);
      break;
    }    
    switch(TaskPara)
    {
    case KEY_BACK://返回
      LZM_StartNewTask(TaskMenuSystemServer);//系统管理
      break;
    case KEY_ENTER://确定
      LcdClearScreen();
      LcdShowCaption("正在保存...", 24);
      val=atol(s_stESpin[s_stTaskAttrib.ControlId].buffer);
      Public_ConvertLongToBuffer(val,buf);
      EepromPram_WritePram(E2_MAIN_SERVER_UDP_PORT_ID, buf,4);                    
      val1=ConvertReadPramToValue(E2_MAIN_SERVER_UDP_PORT_ID);
      LcdClearArea(0,19,131,44);
      if(val==val1)
      {
        LcdShowCaption("保存成功", 24);
        EepromPram_UpdateVariable(E2_MAIN_SERVER_UDP_PORT_ID); 
      }
      else
      {
        LcdShowCaption("保存错误,请检查", 24); 
        
      }
      s_stTaskAttrib.ControlId=0xff;
      LZM_SetAlarm(TEST_TIMER, SECS(1));
      break;                               
    default:
      ESpinResponse(&s_stESpin[s_stTaskAttrib.ControlId],TaskPara);
      break;
    }
    return 1;           
  case TEST_TIMER:
    LZM_StartNewTask(TaskMenuSystemServer);//系统管理
    break;
  case INT_RTC:
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
/*************************************************************
** 函数名称: TaskSystemServerSetIP
** 功能描述: "设置主IP"菜单
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskSystemServerSetIP2(LZM_MESSAGE *Msg)
{
  unsigned char val,buf[16];
  unsigned char i,tmpbuf[16];
  unsigned char temp[4];
  unsigned short tmp;
  switch(Msg->message)
  {
  case TASK_START:            
    val=EepromPram_ReadPram(E2_SECOND_MAIN_SERVER_IP_ID, buf);
    LcdClearScreen();
    s_stTaskAttrib.ControlId=0;  
    s_stTaskAttrib.ControlMaxId =3;
    LcdShowStr(0,0,"当前IP2:",0);                
    buf[val]='\0';
    LcdShowStr(0,16,(char *)buf,0);
    LcdShowStr(0,32,"确认修改为:",0);
    if(!IsBrushAdministratorCard())
    {
      ConvertStrToIP(buf,s_stServerParam.mainIP,val);
    }
    for(i=0; i<=s_stTaskAttrib.ControlMaxId; i++)
    {
      ESpinInit(&s_stESpin[i], 3+i*33, 48, 3, s_stServerParam.mainIP[i]);
      s_stESpin[i].loop =0;
      ESpinShow(&s_stESpin[i]);
      if(i<s_stTaskAttrib.ControlMaxId)LcdShowStr(29+i*33, 48, ".", 0); 
    }            
    ESpinSetFocus(&s_stESpin[s_stTaskAttrib.ControlId]);
    return 1;
  case KEY_PRESS: 
    if(s_stTaskAttrib.ControlId==0xff)
    {
      LZM_SetAlarm(TEST_TIMER, 1);
      break;
    }
    switch(TaskPara)
    {
    case KEY_BACK://返回
      LZM_StartNewTask(TaskMenuSystemServer);//系统管理
      break;
    case KEY_ENTER://确定
      for( i=0; i<=s_stTaskAttrib.ControlMaxId; i++)
      {
        tmp=atoi(s_stESpin[i].buffer);
        if(tmp>255)
        {
          LcdCopyScreen();
          LcdClearScreen();
          LcdShowCaption("格式错误", 24);
          LcdPasteScreen();
          ESpinLoseFocus(&s_stESpin[s_stTaskAttrib.ControlId]);
          s_stTaskAttrib.ControlId=i;
          ESpinSetFocus(&s_stESpin[s_stTaskAttrib.ControlId]);
          return 0;
        }
        temp[i]=tmp;
      }
      LcdClearScreen();
      LcdShowCaption("正在保存...", 24);
      sprintf((char*)buf,"%d.%d.%d.%d",temp[0],temp[1],temp[2],temp[3]);
      val=strlen((char*)buf);
      EepromPram_WritePram(E2_SECOND_MAIN_SERVER_IP_ID, buf,val);
      EepromPram_ReadPram(E2_SECOND_MAIN_SERVER_IP_ID, tmpbuf);
      LcdClearArea(0,19,131,44);
      if(strncmp((char*)buf,(char*)tmpbuf,val)==0)
      {
        LcdShowCaption("保存成功", 24);
        EepromPram_UpdateVariable(E2_SECOND_MAIN_SERVER_IP_ID);
      }
      else
      {
        LcdShowCaption("保存错误,请检查", 24); 
        
      }
      s_stTaskAttrib.ControlId=0xff;
      LZM_SetAlarm(TEST_TIMER, SECS(1));
      break;                                
    default:                    
      val=ESpinResponse(&s_stESpin[s_stTaskAttrib.ControlId],TaskPara); 
      if(val)
      {
        ESpinLoseFocus(&s_stESpin[s_stTaskAttrib.ControlId]);
        if(s_stTaskAttrib.ControlId<s_stTaskAttrib.ControlMaxId)s_stTaskAttrib.ControlId++;
        else s_stTaskAttrib.ControlId = 0;
        ESpinSetFocus(&s_stESpin[s_stTaskAttrib.ControlId]);
      }
      break;
    }
    return 1;           
  case TEST_TIMER:
    LZM_StartNewTask(TaskMenuSystemServer);//系统管理
    break;
  case INT_RTC:
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
/*************************************************************
** 函数名称: TaskSystemServerSetIP
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskSystemServerSetBackUpIP2(LZM_MESSAGE *Msg)
{
  unsigned char buf[16];
  unsigned char val;       
  unsigned char temp[4],tmpbuf[16],i;
  unsigned short tmp;
  switch(Msg->message)
  {
  case TASK_START:            
    val=EepromPram_ReadPram(E2_SECOND_BACKUP_SERVER_IP_ID, buf);
    LcdClearScreen();
    s_stTaskAttrib.ControlId=0;  
    s_stTaskAttrib.ControlMaxId =3;
    LcdShowStr(0,0,"当前备份IP2:",0);                
    buf[val]='\0';
    LcdShowStr(0,16,(char *)buf,0);
    LcdShowStr(0,32,"确认修改为:",0);
    if(!IsBrushAdministratorCard())
    {
      ConvertStrToIP(buf, s_stServerParam.backIP,val);
    }
    ///////////////////
    for( i=0; i<=s_stTaskAttrib.ControlMaxId; i++)
    {
      ESpinInit(&s_stESpin[i], 3+i*33, 48, 3, s_stServerParam.backIP[i]);
      s_stESpin[i].loop =0;
      ESpinShow(&s_stESpin[i]);
      if(i<s_stTaskAttrib.ControlMaxId)LcdShowStr(29+i*33, 48, ".", 0); 
    }            
    ESpinSetFocus(&s_stESpin[s_stTaskAttrib.ControlId]);
    return 1;
  case KEY_PRESS:
    if(s_stTaskAttrib.ControlId==0xff)
    {
      LZM_SetAlarm(TEST_TIMER, 1);
      break;
    }
    switch(TaskPara)
    {
    case KEY_BACK://返回
      LZM_StartNewTask(TaskMenuSystemServer);//系统管理
      break;
    case KEY_ENTER://确定
      for( i=0; i<=s_stTaskAttrib.ControlMaxId; i++)
      {
        tmp=atoi(s_stESpin[i].buffer);
        if(tmp>255)
        {
          LcdCopyScreen();
          LcdClearScreen();
          LcdShowCaption("格式错误", 24);    
          LcdPasteScreen();
          ESpinLoseFocus(&s_stESpin[s_stTaskAttrib.ControlId]);
          s_stTaskAttrib.ControlId=i;
          ESpinSetFocus(&s_stESpin[s_stTaskAttrib.ControlId]);
          return 0;
        }
        temp[i]=tmp;
      }
      LcdClearScreen();
      LcdShowCaption("正在保存...", 24);
      sprintf((char*)buf,"%d.%d.%d.%d",temp[0],temp[1],temp[2],temp[3]);
      val=strlen((char*)buf);
      EepromPram_WritePram(E2_SECOND_BACKUP_SERVER_IP_ID, buf,val);
      EepromPram_ReadPram(E2_SECOND_BACKUP_SERVER_IP_ID, tmpbuf);
      LcdClearArea(0,19,131,44);
      if(strncmp((char*)buf,(char*)tmpbuf,val)==0)
      {
        LcdShowCaption("保存成功", 24);
        EepromPram_UpdateVariable(E2_SECOND_BACKUP_SERVER_IP_ID);
      }
      else
      {
        LcdShowCaption("保存错误,请检查", 24); 
        
      }
      s_stTaskAttrib.ControlId=0xff;
      LZM_SetAlarm(TEST_TIMER, SECS(1));
      break;                                
    default:
      val=ESpinResponse(&s_stESpin[s_stTaskAttrib.ControlId],TaskPara); 
      if(val)
      {
        ESpinLoseFocus(&s_stESpin[s_stTaskAttrib.ControlId]);
        if(s_stTaskAttrib.ControlId<s_stTaskAttrib.ControlMaxId)s_stTaskAttrib.ControlId++;
        else s_stTaskAttrib.ControlId = 0;
        ESpinSetFocus(&s_stESpin[s_stTaskAttrib.ControlId]);
      }
      break;
    }
    return 1;           
  case TEST_TIMER:
    LZM_StartNewTask(TaskMenuSystemServer);//系统管理
    break;
  case INT_RTC:
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}

/*************************************************************
** 函数名称: TaskSystemServerSetTCPPort
** 功能描述: "设置TCP端口号"菜单
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskSystemServerSetTCPPort2(LZM_MESSAGE *Msg)
{
  unsigned long val;
  unsigned long val1;
  unsigned char buf[6];
  switch(Msg->message)
  {
  case TASK_START:            
    val=ConvertReadPramToValue(E2_SECOND_MAIN_SERVER_TCP_PORT_ID);
    LcdClearScreen();
    s_stTaskAttrib.ControlId=0;
    LcdShowStr(0,0,"当前TCP2:",0);                
    sprintf(s_stTaskAttrib.ShowBuffer[0],"%d",val);
    LcdShowStr(3,16,s_stTaskAttrib.ShowBuffer[0],0);
    LcdShowStr(0,32,"确认修改为:",0);
    if(IsBrushAdministratorCard())
    {
      val=s_stServerParam.tcp;
    }           
    ESpinInit(&s_stESpin[s_stTaskAttrib.ControlId], 3, 48, 8,val);
    ESpinShow(&s_stESpin[s_stTaskAttrib.ControlId]);
    
    ESpinSetFocus(&s_stESpin[s_stTaskAttrib.ControlId]);
    return 1;
  case KEY_PRESS:
    if(s_stTaskAttrib.ControlId==0xff)
    {
      LZM_SetAlarm(TEST_TIMER, 1);
      break;
    }
    switch(TaskPara)
    {
    case KEY_BACK://返回
      LZM_StartNewTask(TaskMenuSystemServer);//系统管理
      break; 
    case KEY_ENTER://确定
      LcdClearScreen();
      LcdShowCaption("正在保存...", 24);
      val=atol(s_stESpin[s_stTaskAttrib.ControlId].buffer);
      Public_ConvertLongToBuffer(val,buf);
      EepromPram_WritePram(E2_SECOND_MAIN_SERVER_TCP_PORT_ID, buf,4);                    
      val1=ConvertReadPramToValue(E2_SECOND_MAIN_SERVER_TCP_PORT_ID);
      LcdClearArea(0,19,131,44);
      if(val==val1)
      {
        LcdShowCaption("保存成功", 24);
        EepromPram_UpdateVariable(E2_SECOND_MAIN_SERVER_TCP_PORT_ID);
      }
      else
      {
        LcdShowCaption("保存错误,请检查", 24); 
        
      }
      s_stTaskAttrib.ControlId=0xff;
      LZM_SetAlarm(TEST_TIMER, SECS(1));
      break;                               
    default:
      ESpinResponse(&s_stESpin[s_stTaskAttrib.ControlId],TaskPara);
      break;
    }
    return 1;         
  case TEST_TIMER:
    LZM_StartNewTask(TaskMenuSystemServer);//系统管理
    break;
  case INT_RTC:
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}

/*************************************************************
** 函数名称: TaskSystemServerSetUDPPort
** 功能描述: "设置UDP端口号"菜单
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskSystemServerSetUDPPort2(LZM_MESSAGE *Msg)
{
  unsigned long val;
  unsigned long val1;
  unsigned char buf[6];
  switch(Msg->message)
  {
  case TASK_START:            
    val=ConvertReadPramToValue(E2_SECOND_MAIN_SERVER_UDP_PORT_ID);
    LcdClearScreen();
    s_stTaskAttrib.ControlId=0;
    LcdShowStr(0,0,"当前UDP2:",0);                
    sprintf(s_stTaskAttrib.ShowBuffer[0],"%d",val);
    LcdShowStr(3,16,s_stTaskAttrib.ShowBuffer[0],0);
    LcdShowStr(0,32,"确认修改为:",0);
    if(IsBrushAdministratorCard())
    {
      val=s_stServerParam.udp;
    }            
    ESpinInit(&s_stESpin[s_stTaskAttrib.ControlId], 3, 48, 8,val);
    ESpinShow(&s_stESpin[s_stTaskAttrib.ControlId]);            
    ESpinSetFocus(&s_stESpin[s_stTaskAttrib.ControlId]);
    return 1;
  case KEY_PRESS:
    if(s_stTaskAttrib.ControlId==0xff)
    {
      LZM_SetAlarm(TEST_TIMER, 1);
      break;
    }    
    switch(TaskPara)
    {
    case KEY_BACK://返回
      LZM_StartNewTask(TaskMenuSystemServer);//系统管理
      break;
    case KEY_ENTER://确定
      LcdClearScreen();
      LcdShowCaption("正在保存...", 24);
      val=atol(s_stESpin[s_stTaskAttrib.ControlId].buffer);
      Public_ConvertLongToBuffer(val,buf);
      EepromPram_WritePram(E2_SECOND_MAIN_SERVER_UDP_PORT_ID, buf,4);                    
      val1=ConvertReadPramToValue(E2_SECOND_MAIN_SERVER_UDP_PORT_ID);
      LcdClearArea(0,19,131,44);
      if(val==val1)
      {
        LcdShowCaption("保存成功", 24);
        EepromPram_UpdateVariable(E2_SECOND_MAIN_SERVER_UDP_PORT_ID); 
      }
      else
      {
        LcdShowCaption("保存错误,请检查", 24); 
        
      }
      s_stTaskAttrib.ControlId=0xff;
      LZM_SetAlarm(TEST_TIMER, SECS(1));
      break;                               
    default:
      ESpinResponse(&s_stESpin[s_stTaskAttrib.ControlId],TaskPara);
      break;
    }
    return 1;           
  case TEST_TIMER:
    LZM_StartNewTask(TaskMenuSystemServer);//系统管理
    break;
  case INT_RTC:
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
/*************************************************************
** 函数名称: TaskSystemServerSetMainDomainName
** 功能描述: "设置主中心域名"菜单
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskSystemServerSetMainDomainName(LZM_MESSAGE *Msg)
{    
  unsigned char buf[40];
  unsigned char val;
  switch(Msg->message)
  {
  case TASK_START:
    val=Public_ReadDataFromFRAM(FRAM_MAIN_DOMAIN_NAME_ADDR, buf,FRAM_MAIN_DOMAIN_NAME_LEN);
    LcdClearScreen();           
    LcdShowCaption("主中心域名",0);
    LcdShowHorLine(0, 131, 18, 1);
#if 0
    LcdShowHorLine(0, 131, 45, 1);
    LcdShowCaption("按<确认>保存",47);
    /////////////////////
    s_stTaskAttrib.ControlId = 0;
    if(strncmp((char*)buf,(char*)s_apcCenterDnsItems[1],5)==0)
    {
      s_stTaskAttrib.ControlId = 1;
    }            
    ////////////////////////
    SpinInit(&s_stSpin[0],4,24,s_apcCenterDnsItems,"");
    s_stSpin[0].handle = s_stTaskAttrib.ControlId;
    SpinShow(&s_stSpin[0]);            
    SpinSetFocus(&s_stSpin[0]); 
#else
    buf[val]= 0;
    LcdShowStr(0, 22, (char *)buf, 0);
    s_stTaskAttrib.ControlId=0xff;
#endif            
    return 1;
  case KEY_PRESS:
    if(s_stTaskAttrib.ControlId==0xff)
    {
      LZM_SetAlarm(TEST_TIMER, 1);
      break;
    }
    switch(TaskPara)
    {
    case KEY_BACK://返回
      LZM_StartNewTask(TaskMenuSystemServer);//系统管理
      break;
    case KEY_ENTER://确定
      val=strlen(s_stSpin[0].item[s_stSpin[0].handle]);
      memset(buf,0,FRAM_MAIN_DOMAIN_NAME_LEN);
      memcpy(buf,s_stSpin[0].item[s_stSpin[0].handle],val);
      Public_WriteDataToFRAM(FRAM_MAIN_DOMAIN_NAME_ADDR, buf,FRAM_MAIN_DOMAIN_NAME_LEN);
      Public_ReadDataFromFRAM(FRAM_MAIN_DOMAIN_NAME_ADDR, buf,FRAM_MAIN_DOMAIN_NAME_LEN);
      LcdClearScreen();
      if(strncmp((char*)buf,(char*)s_stSpin[0].item[s_stSpin[0].handle],val)==0)
      {
        LcdShowCaption("保存成功", 24);
      }
      else
      {
        LcdShowCaption("保存错误,请检查", 24); 
        
      }
      LZM_SetAlarm(TEST_TIMER, SECS(1));                   
      break;                                
    default:
      SpinResponse(&s_stSpin[0],TaskPara);
      break;
    }
    return 1;           
  case TEST_TIMER:
    LZM_StartNewTask(TaskMenuSystemServer);//系统管理
    break;
  case INT_RTC:
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
/*************************************************************
** 函数名称: TaskSystemServerSetMainDomainName
** 功能描述: "设置主中心域名"菜单
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskSystemServerSetBackUpDomainName(LZM_MESSAGE *Msg)
{    
  unsigned char buf[40];
  unsigned char val;
  switch(Msg->message)
  {
  case TASK_START:
    val=Public_ReadDataFromFRAM(FRAM_BACKUP_DOMAIN_NAME_ADDR, buf,FRAM_BACKUP_DOMAIN_NAME_LEN);
    LcdClearScreen();           
    LcdShowCaption("备份中心域名",0);
    LcdShowHorLine(0, 131, 18, 1);
#if 0
    LcdShowHorLine(0, 131, 45, 1);
    LcdShowCaption("按<确认>保存",47);
    /////////////////////
    s_stTaskAttrib.ControlId = 0;
    if(strncmp((char*)buf,(char*)s_apcCenterDnsItems[1],5)==0)
    {
      s_stTaskAttrib.ControlId = 1;
    }            
    ////////////////////////
    SpinInit(&s_stSpin[0],4,24,s_apcCenterDnsItems,"");
    s_stSpin[0].handle = s_stTaskAttrib.ControlId;
    SpinShow(&s_stSpin[0]);            
    SpinSetFocus(&s_stSpin[0]);
#else
    buf[val]= 0;
    LcdShowStr(0, 22, (char *)buf, 0);
    s_stTaskAttrib.ControlId=0xff;
#endif
    return 1;
  case KEY_PRESS:
    if(s_stTaskAttrib.ControlId==0xff)
    {
      LZM_SetAlarm(TEST_TIMER, 1);
      break;
    }
    switch(TaskPara)
    {
    case KEY_BACK://返回
      LZM_StartNewTask(TaskMenuSystemServer);//系统管理
      break;
    case KEY_ENTER://确定
      val=strlen(s_stSpin[0].item[s_stSpin[0].handle]);
      memset(buf,0,FRAM_BACKUP_DOMAIN_NAME_LEN);
      memcpy(buf,s_stSpin[0].item[s_stSpin[0].handle],val);
      Public_WriteDataToFRAM(FRAM_BACKUP_DOMAIN_NAME_ADDR, buf,FRAM_BACKUP_DOMAIN_NAME_LEN);
      Public_ReadDataFromFRAM(FRAM_BACKUP_DOMAIN_NAME_ADDR, buf,FRAM_BACKUP_DOMAIN_NAME_LEN);
      LcdClearScreen();
      if(strncmp((char*)buf,(char*)s_stSpin[0].item[s_stSpin[0].handle],val)==0)
      {
        LcdShowCaption("保存成功", 24);
      }
      else
      {
        LcdShowCaption("保存错误,请检查", 24); 
        
      }
      LZM_SetAlarm(TEST_TIMER, SECS(1));                                   
      break;                                
    default:
      SpinResponse(&s_stSpin[0],TaskPara);
      break;
    }
    return 1;           
  case TEST_TIMER:
    LZM_StartNewTask(TaskMenuSystemServer);//系统管理
    break;
  case INT_RTC:
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
/*************************************************************
** 函数名称: TaskSystemSetCarFeatureCoef
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskSystemSetCarFeatureCoef(LZM_MESSAGE *Msg)
{
  unsigned long val,val1;
  unsigned char buf[6];
  switch(Msg->message)
  {
  case TASK_START:            
    val=ConvertReadPramToValue(E2_CAR_FEATURE_COEF_ID);
    LcdClearScreen();
    s_stTaskAttrib.ControlId=0;
    LcdShowCaption("设置脉冲系数",0);
    LcdShowHorLine(0, 131, 18, 1);
    LcdShowHorLine(0, 131, 45, 1);
    LcdShowCaption("按<确认>保存",47);            
    LcdShowStr(1, 24, "脉冲系数:", 0);
    ESpinInit(&s_stESpin[s_stTaskAttrib.ControlId], 76, 24, 6,val);
    ESpinShow(&s_stESpin[s_stTaskAttrib.ControlId]);
    ESpinSetFocus(&s_stESpin[s_stTaskAttrib.ControlId]);
    return 1;
  case KEY_PRESS:
    if(s_stTaskAttrib.ControlId==0xff)break;
    switch(TaskPara)
    {
    case KEY_BACK://返回
      LZM_StartNewTask(TaskMenuSystemCarInfo);//系统管理
      break; 
    case KEY_ENTER://确定
      LcdClearArea(0,19,131,44);
      LcdShowCaption("正在保存...", 24);
      val=atol(s_stESpin[s_stTaskAttrib.ControlId].buffer);
      Public_ConvertLongToBuffer(val,buf);
      EepromPram_WritePram(E2_CAR_FEATURE_COEF_ID, &buf[1],3);                    
      val1=ConvertReadPramToValue(E2_CAR_FEATURE_COEF_ID);
      LcdClearArea(0,19,131,44);
      if(val==val1)
      {
        LcdShowCaption("保存成功", 24);
        EepromPram_UpdateVariable(E2_CAR_FEATURE_COEF_ID);
				//行车记录仪参数修改记录
        Recorder_ParaSet(0xc3);
      }
      else
      {
        LcdShowCaption("保存错误,请检查", 24); 
        
      }
      s_stTaskAttrib.ControlId=0xff;
      LZM_SetAlarm(TEST_TIMER, SECS(1));
      break;                               
    default:                    
      ESpinResponse(&s_stESpin[s_stTaskAttrib.ControlId],TaskPara);                     
      break;
    }
    return 1;
  case TEST_TIMER:
    LZM_StartNewTask(TaskMenuSystemCarInfo);//系统管理
    break;
  case INT_RTC:
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
/*************************************************************
** 函数名称: TaskSystemSetCarInitialMile
** 功能描述: "设置车辆初始里程"菜单
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskSystemSetCarTotalMile(LZM_MESSAGE *Msg)
{
  unsigned long val,val1;
  unsigned char buf[6];
  float temp;
  switch(Msg->message)
  {
  case TASK_START:
    if(E2ParamApp_ReadSpecialParam(E2_CAR_TOTAL_MILE_ID,buf))
    {
      val = Public_ConvertBufferToLong(buf);
    }
    //val=ConvertReadPramToValue(E2_CAR_TOTAL_MILE_ID);
    LcdClearScreen();
    s_stTaskAttrib.ControlId=0;
    LcdShowCaption("设置初始里程",0);
    LcdShowHorLine(0, 131, 18, 1);
    LcdShowHorLine(0, 131, 45, 1);
    LcdShowCaption("按<确认>保存",47);            
    LcdShowStr(0, 24, "里程:         km", 0);
    ESpinInit(&s_stESpin[s_stTaskAttrib.ControlId], 40, 24, 9,val);
    sprintf(s_stESpin[s_stTaskAttrib.ControlId].buffer,"%09.1f",val*0.1);
    ESpinShow(&s_stESpin[s_stTaskAttrib.ControlId]);
    ESpinSetFocus(&s_stESpin[s_stTaskAttrib.ControlId]);
    return 1;
  case KEY_PRESS:
    if(s_stTaskAttrib.ControlId==0xff)break;
    switch(TaskPara)
    {
    case KEY_BACK://返回
      LZM_StartNewTask(TaskMenuSystemCarInfo);//系统管理
      break; 
    case KEY_ENTER://确定                    
      LcdClearArea(0,19,131,44);
      LcdShowCaption("正在保存...", 24);
      temp=atof(s_stESpin[s_stTaskAttrib.ControlId].buffer);
      val=(unsigned long)(temp*10);
      Public_ConvertLongToBuffer(val,buf);
      EepromPram_WritePram(E2_CAR_TOTAL_MILE_ID, buf,4);                    
      val1=ConvertReadPramToValue(E2_CAR_TOTAL_MILE_ID);                    
      LcdClearArea(0,19,131,44);
      if(val==val1)
      {
        LcdShowCaption("保存成功", 24);
				//行车记录仪参数修改记录
        Recorder_ParaSet(0xc4);
        EepromPram_UpdateVariable(E2_CAR_TOTAL_MILE_ID);
      }
      else
      {
        LcdShowCaption("保存错误,请检查", 24); 
        
      }
      s_stTaskAttrib.ControlId=0xff;
      LZM_SetAlarm(TEST_TIMER, SECS(1));
      break;                               
    default:                    
      ESpinResponse(&s_stESpin[s_stTaskAttrib.ControlId],TaskPara);                     
      break;
    }
    return 1;
  case TEST_TIMER:
    LZM_StartNewTask(TaskMenuSystemCarInfo);//系统管理
    break;
  case INT_RTC:
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
/*************************************************************
** 函数名称: TaskSystemSetCarProvinceID
** 功能描述: "设置省域ID"菜单
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskSystemSetCarProvinceID(LZM_MESSAGE *Msg)
{
  unsigned short val,val1;
  unsigned char buf[6];
  switch(Msg->message)
  {
  case TASK_START:
    EepromPram_ReadPram(E2_CAR_PROVINCE_ID, buf);
    val=Public_ConvertBufferToShort(buf);            
    LcdClearScreen();
    s_stTaskAttrib.ControlId=0;
    LcdShowCaption("设置省域ID",0);
    LcdShowHorLine(0, 131, 18, 1);
    LcdShowHorLine(0, 131, 45, 1);
    LcdShowCaption("按<确认>保存",47);            
    LcdShowStr(0, 24, "省域ID:", 0);
    ESpinInit(&s_stESpin[s_stTaskAttrib.ControlId], 64, 24, 5,val);
    ESpinShow(&s_stESpin[s_stTaskAttrib.ControlId]);
    ESpinSetFocus(&s_stESpin[s_stTaskAttrib.ControlId]);
    return 1;
  case KEY_PRESS:
    if(s_stTaskAttrib.ControlId==0xff)break;
    switch(TaskPara)
    {
    case KEY_BACK://返回
      LZM_StartNewTask(TaskMenuSystemCarInfo);//系统管理
      break; 
    case KEY_ENTER://确定                    
      LcdClearArea(0,19,131,44);
      LcdShowCaption("正在保存...", 24);
      val=atoi(s_stESpin[s_stTaskAttrib.ControlId].buffer);
      Public_ConvertShortToBuffer(val,buf);                        
      EepromPram_WritePram(E2_CAR_PROVINCE_ID, buf,2);                    
      val1=ConvertReadPramToValue(E2_CAR_PROVINCE_ID);
      EepromPram_UpdateVariable(E2_CAR_PROVINCE_ID);
      LcdClearArea(0,19,131,44);
      if(val==val1)
      {
        LcdShowCaption("保存成功", 24);
      }
      else
      {
        LcdShowCaption("保存错误,请检查", 24); 
        
      }
      s_stTaskAttrib.ControlId=0xff;
      LZM_SetAlarm(TEST_TIMER, SECS(1));
      break;                               
    default:                    
      ESpinResponse(&s_stESpin[s_stTaskAttrib.ControlId],TaskPara);                     
      break;
    }
    return 1;
  case TEST_TIMER:
    LZM_StartNewTask(TaskMenuSystemCarInfo);//系统管理
    break;
  case INT_RTC:
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
/*************************************************************
** 函数名称: TaskSystemSetCarCityID
** 功能描述: "设置市域ID"菜单
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskSystemSetCarCityID(LZM_MESSAGE *Msg)
{
  unsigned short val,val1;
  unsigned char buf[6];
  switch(Msg->message)
  {
  case TASK_START:
    EepromPram_ReadPram(E2_CAR_CITY_ID, buf);
    val=Public_ConvertBufferToShort(buf);            
    LcdClearScreen();
    s_stTaskAttrib.ControlId=0;
    LcdShowCaption("设置市域ID",0);
    LcdShowHorLine(0, 131, 18, 1);
    LcdShowHorLine(0, 131, 45, 1);
    LcdShowCaption("按<确认>保存",47);            
    LcdShowStr(0, 24, "市域ID:", 0);
    ESpinInit(&s_stESpin[s_stTaskAttrib.ControlId], 64, 24, 5,val);
    ESpinShow(&s_stESpin[s_stTaskAttrib.ControlId]);
    ESpinSetFocus(&s_stESpin[s_stTaskAttrib.ControlId]);
    return 1;
  case KEY_PRESS:
    if(s_stTaskAttrib.ControlId==0xff)break;
    switch(TaskPara)
    {
    case KEY_BACK://返回
      LZM_StartNewTask(TaskMenuSystemCarInfo);//系统管理
      break; 
    case KEY_ENTER://确定                    
      LcdClearArea(0,19,131,44);
      LcdShowCaption("正在保存...", 24);
      val=atoi(s_stESpin[s_stTaskAttrib.ControlId].buffer);
      Public_ConvertShortToBuffer(val,buf);                        
      EepromPram_WritePram(E2_CAR_CITY_ID, buf,2);                    
      val1=ConvertReadPramToValue(E2_CAR_CITY_ID);
      EepromPram_UpdateVariable(E2_CAR_CITY_ID);
      LcdClearArea(0,19,131,44);
      if(val==val1)
      {
        LcdShowCaption("保存成功", 24);
      }
      else
      {
        LcdShowCaption("保存错误,请检查", 24); 
        
      }
      s_stTaskAttrib.ControlId=0xff;
      LZM_SetAlarm(TEST_TIMER, SECS(1));
      break;                               
    default:                    
      ESpinResponse(&s_stESpin[s_stTaskAttrib.ControlId],TaskPara);                     
      break;
    }
    return 1;
  case TEST_TIMER:
    LZM_StartNewTask(TaskMenuSystemCarInfo);//系统管理
    break;
  case INT_RTC:
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
/*************************************************************
** 函数名称: TaskSystemParamSetPhoneNumber
** 功能描述: "设置手机号"菜单
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskSystemSetCarOwnerPhone(LZM_MESSAGE *Msg)
{
  unsigned char len;
  unsigned char buf[13]={0};
  switch(Msg->message)
  {
  case TASK_START:   
    len=Public_ReadDataFromFRAM(FRAM_CAR_OWNER_PHONE_ADDR, buf,FRAM_CAR_OWNER_PHONE_LEN);
    if(FRAM_CAR_OWNER_PHONE_LEN!=len)
    {
      memset(&buf[len],0x30,FRAM_CAR_OWNER_PHONE_LEN-len);
    }
    buf[FRAM_CAR_OWNER_PHONE_LEN-1]='\0';
    LcdClearScreen();
    s_stTaskAttrib.ControlId=0;
    LcdShowCaption("车主手机号",0);
    LcdShowHorLine(0, 131, 18, 1);
    LcdShowHorLine(0, 131, 45, 1);
    LcdShowCaption("按<确认>保存",47);
    ESpinInit(&s_stESpin[s_stTaskAttrib.ControlId], 22, 24, 11,1);
    s_stESpin[s_stTaskAttrib.ControlId].type = 1;//字符串
    strcpy(s_stESpin[s_stTaskAttrib.ControlId].buffer,(char *)buf);                
    ESpinShow(&s_stESpin[s_stTaskAttrib.ControlId]);
    ESpinSetFocus(&s_stESpin[s_stTaskAttrib.ControlId]);
    return 1;
  case KEY_PRESS:
    if(s_stTaskAttrib.ControlId==0xff)break;
    switch(TaskPara)
    {
    case KEY_BACK://返回
      LZM_StartNewTask(TaskMenuSystemCarInfo);//系统管理
      break;
    case KEY_ENTER://确定 
      LcdClearArea(0,19,131,44);
      LcdShowCaption("正在保存...", 24);
      memcpy(buf,s_stESpin[s_stTaskAttrib.ControlId].buffer,11);
      buf[11]=0;
      Public_WriteDataToFRAM(FRAM_CAR_OWNER_PHONE_ADDR, buf,FRAM_CAR_OWNER_PHONE_LEN);
      LcdClearArea(0,19,131,44);
      LcdShowCaption("保存成功", 24);
      s_stTaskAttrib.ControlId=0xff;
      LZM_SetAlarm(TEST_TIMER, SECS(1));
      break;                                
    default:                    
      ESpinResponse(&s_stESpin[s_stTaskAttrib.ControlId],TaskPara);                     
      break;
    }
    return 1;
  case TEST_TIMER:
    LZM_StartNewTask(TaskMenuSystemCarInfo);//系统管理
    break;
  case INT_RTC:
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
/*************************************************************
** 函数名称: TaskSystemParamSetSpeedParam
** 功能描述: "设置车辆速度参数"菜单
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskSystemParamSetSpeedParam(LZM_MESSAGE *Msg)
{
  unsigned long val,val1;
  unsigned char temp,temp1;
  unsigned char buf[6];
  switch(Msg->message)
  {
  case TASK_START:
    LcdClearScreen();
    s_stTaskAttrib.ControlId=0;           
    LcdShowCaption("设置速度参数",2);
    LcdShowHorLine(0, 131, 20, 1);
    LcdShowStr(0, 24, "超速报警:", 0);
    LcdShowStr(98, 24, "km/h", 0);
    //LcdShowStr(0, 45, "速度选择:", 0); dxl,2015.5.14 改为“选择”
    LcdShowStr(0, 45, "选择:", 0);
    val=ConvertReadPramToValue(E2_MAX_SPEED_ID);
    ESpinInit(&s_stESpin[0], 72, 24, 3,val);
    //SpinInit(&s_stSpin[0],76,45,(char **)c_apcCarSpeedSelectItems,"");dxl,2015.5.14
    SpinInit(&s_stSpin[0],44,45,(char **)c_apcCarSpeedSelectItems,"");
    EepromPram_ReadPram(E2_SPEED_SELECT_ID,&temp); 
    //if(temp>1)temp=0;//dxl,2015.5.14,原来是2项,现在是4项
    if(temp>3)temp=0;
    s_stSpin[0].handle=temp;
    s_stSpin[0].keyflag =1;
    ////////////////////
    s_stESpin[0].loop = 0;
    ESpinShow(&s_stESpin[0]);
    SpinShow(&s_stSpin[0]);
    ESpinSetFocus(&s_stESpin[s_stTaskAttrib.ControlId]);
    return 1;
  case KEY_PRESS:
    if(s_stTaskAttrib.ControlId==0xff)break;
    switch(TaskPara)
    {
    case KEY_BACK://返回
      LZM_StartNewTask(TaskMenuSystemParameter);//系统管理
      break; 
    case KEY_ENTER://确定
      LcdClearScreen();
      LcdShowCaption("正在保存...", 24);
      val=atol(s_stESpin[0].buffer);
      Public_ConvertLongToBuffer(val,buf);
      EepromPram_WritePram(E2_MAX_SPEED_ID, buf,4);
      temp=s_stSpin[0].handle;
      EepromPram_WritePram(E2_SPEED_SELECT_ID,&temp,1);
      //////////////////////////////////////////
      val1=ConvertReadPramToValue(E2_MAX_SPEED_ID);
      EepromPram_ReadPram(E2_SPEED_SELECT_ID,&temp1); 
      LcdClearScreen();
      if(val==val1&&temp1==temp)
      {
        LcdShowCaption("保存成功", 24);
        EepromPram_UpdateVariable(E2_MAX_SPEED_ID);
        EepromPram_UpdateVariable(E2_SPEED_SELECT_ID);
      }
      else
      {
        LcdShowCaption("保存错误,请检查", 24); 
        
      }                    
      s_stTaskAttrib.ControlId=0xff;
      LZM_SetAlarm(TEST_TIMER, SECS(1));
      break;                               
    default:
      if(!s_stTaskAttrib.ControlId)
      {
        temp=ESpinResponse(&s_stESpin[0],TaskPara); 
        if(temp)
        {
          ESpinLoseFocus(&s_stESpin[0]);
          s_stTaskAttrib.ControlId++;
          SpinSetFocus(&s_stSpin[0]);
        }
      }
      else
      {
        temp=SpinResponse(&s_stSpin[0],TaskPara); 
        if(temp)
        {
          SpinLoseFocus(&s_stSpin[0]);
          s_stTaskAttrib.ControlId=0;
          ESpinSetFocus(&s_stESpin[0]);
        }
      }
      break;
    }
    return 1;
  case TEST_TIMER:
    LZM_StartNewTask(TaskMenuSystemParameter);//系统管理
    break;
  case INT_RTC:
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
/*************************************************************
** 函数名称: TaskSystemParamSetPhoneNumber
** 功能描述: "设置手机号"菜单
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskSystemParamSetPhoneNumber(LZM_MESSAGE *Msg)
{
  unsigned char val;
  unsigned char buf[13];
  switch(Msg->message)
  {
  case TASK_START:
    val=ReadAndCheckPhoneNumber(buf);
    if(0==val)
    {
      memcpy(buf,"00000000000",11);
    }
    LcdClearScreen();
    s_stTaskAttrib.ControlId=0;
    LcdShowCaption("设置手机号",0);
    LcdShowHorLine(0, 131, 18, 1);
    LcdShowHorLine(0, 131, 45, 1);
    LcdShowCaption("按<确认>保存",47);
    ESpinInit(&s_stESpin[s_stTaskAttrib.ControlId], 22, 24, 11,1);
    s_stESpin[s_stTaskAttrib.ControlId].type = 1;//字符串
    strcpy(s_stESpin[s_stTaskAttrib.ControlId].buffer,(char *)buf);                
    ESpinShow(&s_stESpin[s_stTaskAttrib.ControlId]);
    ESpinSetFocus(&s_stESpin[s_stTaskAttrib.ControlId]);
    return 1;
  case KEY_PRESS:
    if(s_stTaskAttrib.ControlId==0xff)break;
    switch(TaskPara)
    {
    case KEY_BACK://返回
      LZM_StartNewTask(TaskMenuSystemParameter);//系统管理
      break;
    case KEY_ENTER://确定 
      LcdClearArea(0,19,131,44);
      LcdShowCaption("正在保存...", 24);
      val=SaveAndCheckTerminalPhoneNumber((unsigned char *)s_stESpin[s_stTaskAttrib.ControlId].buffer);
      LcdClearArea(0,19,131,44);
      if(val==1)
      {
        LcdShowCaption("保存成功", 24);
        EepromPram_UpdateVariable(E2_DEVICE_PHONE_ID);
      }
      else
      {
        LcdShowCaption("保存错误,请检查", 24);
        
      }
      s_stTaskAttrib.ControlId=0xff;
      LZM_SetAlarm(TEST_TIMER, SECS(1));
      break;                                
    default:                    
      ESpinResponse(&s_stESpin[s_stTaskAttrib.ControlId],TaskPara);                     
      break;
    }
    return 1;
  case TEST_TIMER:
    LZM_StartNewTask(TaskMenuSystemParameter);//系统管理
    break;
  case INT_RTC:
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}

/*************************************************************
** 函数名称: TaskSystemParamSetDeviceID
** 功能描述: "设置终端ID"菜单
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskSystemParamSetDeviceID(LZM_MESSAGE *Msg)
{
  unsigned char val;
  char buf[17];
  unsigned long deviceID;
  static unsigned short manufacturer;
  switch(Msg->message)
  {
  case TASK_START:
    val=ReadSystemDeviceID(&deviceID,&manufacturer);
    if(val==1)
    {
      LcdClearScreen();
      s_stTaskAttrib.ControlId=0;
      LcdShowCaption("设置终端ID",0);
      LcdShowHorLine(0, 131, 18, 1);
      //LcdShowHorLine(0, 131, 45, 1);
      //LcdShowCaption("按<确认>保存",47);
      LcdShowStr(12, 23, "ID:", 0);
      sprintf(buf,"制造商ID:%05d",manufacturer);
      LcdShowCaption(buf,45);
      ESpinInit(&s_stESpin[s_stTaskAttrib.ControlId], 40, 23, 10,deviceID);
      s_stESpin[s_stTaskAttrib.ControlId].type = 1;//字符串              
      ESpinShow(&s_stESpin[s_stTaskAttrib.ControlId]);
      ESpinSetFocus(&s_stESpin[s_stTaskAttrib.ControlId]);
    }
    else
    {
      LcdClearScreen();
      LcdShowCaption("数据格式错误!",23);
      
      s_stTaskAttrib.ControlId=0xff;
      LZM_SetAlarm(TEST_TIMER, SECS(1));
    }
    return 1;
  case KEY_PRESS:
    if(s_stTaskAttrib.ControlId==0xff)break;
    switch(TaskPara)
    {
    case KEY_BACK://返回
      LZM_StartNewTask(TaskMenuSystemParameter);//系统管理
      break;
    case KEY_ENTER://确定 
      val=s_stESpin[s_stTaskAttrib.ControlId].buffer[0]-0x30;
      if(val>=4)
      {
        LcdCopyScreen();
        LcdClearArea(0,19,131,62);
        LcdShowCaption("数值超范围", 26);
        LcdPasteScreen();
        return 0;
      }
      deviceID=atol(s_stESpin[s_stTaskAttrib.ControlId].buffer);
      LcdClearArea(0,19,131,62);
      LcdShowCaption("正在保存...", 24);
      val=SaveAndCheckSystemDeviceID(deviceID,manufacturer);
      LcdClearArea(0,19,131,62);
      if(val==1)
      {
        LcdShowCaption("保存成功", 24);
        EepromPram_UpdateVariable(E2_DEVICE_PHONE_ID);
      }
      else
      {
        LcdShowCaption("保存错误,请检查", 24);
        
      }
      s_stTaskAttrib.ControlId=0xff;
      LZM_SetAlarm(TEST_TIMER, SECS(1));
      break;                                
    default:                    
      ESpinResponse(&s_stESpin[s_stTaskAttrib.ControlId],TaskPara);                     
      break;
    }
    return 1;
  case TEST_TIMER:
    LZM_StartNewTask(TaskMenuSystemParameter);//系统管理
    break;
  case INT_RTC:
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
#if 0
/*************************************************************
** 函数名称: TaskMenuSetPhoneTypeSelect
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskMenuSetPhoneTypeSelect(LZM_MESSAGE *Msg)
{    
  switch(Msg->message)
  {
  case TASK_START:
    LcdClearScreen();            
    s_stTaskAttrib.ControlId=0;
    LcdShowCaption("请选择类型",0);
    LcdShowHorLine(0, 131, 18, 1);
    LcdShowHorLine(0, 131, 45, 1);
    LcdShowCaption("按<确认>选择",47);
    LcdShowStr(18, 23, "通用", 0);
    LcdShowStr(80, 23, "交委", 0);
    LcdReverseBar(5,22,65,40);
    return 1;
  case KEY_PRESS:
    if(s_stTaskAttrib.ControlId==0xff)
    {
      LZM_StartNewTask(TaskMenuSystemAdvanced);//系统管理
      break;
    }
    switch(TaskPara)
    {
    case KEY_BACK://
      LZM_StartNewTask(TaskMenuSystemAdvanced);//系统管理
      break;
    case KEY_ENTER://
      if(s_stTaskAttrib.ControlId==0)
      {
        LZM_StartNewTask(TaskSystemParamSetPhoneNumber);
      }
      else
      {
        LZM_StartNewTask(TaskSystemParamSetDeviceID);
      }
      break;
    case KEY_DOWN:
    case KEY_UP:
      s_stTaskAttrib.ControlId^=1;
      LcdReverseBar(5,22,126,40);
      break;
    }
    return 1;
  case TEST_TIMER:
    LZM_StartNewTask(TaskMenuSystemAdvanced);//系统管理
    break;
  case INT_RTC:
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
#endif
/*************************************************************
** 函数名称: TaskSystemDebugShowIOstatus
** 功能描述: "输入状态显示"菜单
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskSystemDebugShowIOstatus(LZM_MESSAGE *Msg)
{
  switch(Msg->message)
  {
  case TASK_START:
    LcdClearScreen();            
    LcdShowCaption("输入状态显示",0);
    LcdShowHorLine(0, 131, 17, 1);
    LcdShowHorLine(0, 131, 45, 1);
    LcdShowCaption("按<确认>检测",47);
    s_stTaskAttrib.select    = 0;
    s_stTaskAttrib.ControlId = 0;
    return 1;
  case KEY_PRESS:
    switch(TaskPara)
    {
    case KEY_BACK://返回
      LZM_StartNewTask(TaskMenuSystemDebug);//系统管理
      break;
    case KEY_ENTER://确定                    
      s_stTaskAttrib.select=1;
      LcdClearArea(0,19,131,62);
      LcdShowCaption("正在检测,请稍候", 24);
      LZM_SetAlarm(TEST_TIMER, SECS(1));
      break;                                
    case KEY_UP:
      if(s_stTaskAttrib.select==2)
      {
        if(s_stTaskAttrib.ControlId)
        {
          s_stTaskAttrib.ControlId--;
          ///////
          ShowCurrentIOstatus(s_stTaskAttrib.ControlId);
        }
      }
      break;
    case KEY_DOWN:
      if(s_stTaskAttrib.select==2)
      {
        if((s_stTaskAttrib.ControlId+1)*IO_ONE_PAGE_NUM<IO_IN_ITEMS_NUM)
        {
          s_stTaskAttrib.ControlId++;
          /////////////
          ShowCurrentIOstatus(s_stTaskAttrib.ControlId);
        }
      }
      break;
    }
    return 1;
  case TEST_TIMER:
    s_stTaskAttrib.select = 2;
    ShowCurrentIOstatus(s_stTaskAttrib.ControlId);
    break;
  case INT_RTC:
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}

/*************************************************************
** 函数名称: TaskSystemDebugIOOut
** 功能描述: 输出状态调试菜单
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskSystemDebugIOOut(LZM_MESSAGE *Msg)
{
  unsigned char val;
  switch(Msg->message)
  {
  case TASK_START:
    LcdClearScreen();
    s_stTaskAttrib.ControlId = 0;
    LcdShowCaption("输出状态调试",0);
    LcdShowHorLine(0, 131, 17, 1);
    LcdShowHorLine(0, 131, 45, 1);
    LcdShowCaption("按<确认>更改",47);
    SpinInit(&s_stSpin[0],24,23,(char **)c_apcIOOutItems,"");
    SpinShow(&s_stSpin[0]);
    SpinSetFocus(&s_stSpin[0]);
    return 1;
  case KEY_PRESS:
    if(s_stTaskAttrib.ControlId==0xff&&TaskPara!=KEY_BACK)
    {
      break;
    }
    switch(TaskPara)
    {
    case KEY_BACK://返回
      LZM_StartNewTask(TaskMenuSystemDebug);//系统管理
      break;
    case KEY_ENTER://确定
      s_stTaskAttrib.select=s_stSpin[0].handle+IO_IN_MAX+1;
      val=GetCurrentIOstatus(s_stTaskAttrib.select);
      val^=1;
      ChangCurrentIOOutstatus(s_stTaskAttrib.select,val);
      LcdClearArea(112,23,128,40);
      s_stTaskAttrib.ControlId=0xff;
      LZM_SetAlarm(TEST_TIMER, SECS(3));
      break;                                
    default:
      SpinResponse(&s_stSpin[0],TaskPara);
      LcdClearArea(112,23,128,40);
      break;
    }
    return 1;
  case TEST_TIMER:
    //TestBeep(1);
    s_stTaskAttrib.ControlId =0;
    val=GetCurrentIOstatus(s_stTaskAttrib.select);
    LcdShowStr(112,23,val?"开":"关",0);
    break;
  case INT_RTC:
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
/////////////////////////////
extern u32 Link1ReportTimeSpace;//当前连接1汇报时间间隔
extern u32 Link1ReportDistanceSpace;//当前连接1汇报距离间隔
/*************************************************************
** 函数名称: ShowCurrentReportMode
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void ShowCurrentReportMode(void)
{    
  char buffer[30];
  unsigned char y;
  unsigned long ProgOfRpt,StrategyOfRpt;
  //////////////////
  ProgOfRpt =0; 
  StrategyOfRpt = 0;
  if(EepromPram_ReadPram(E2_POSITION_REPORT_SCHEME_ID,(unsigned char *)buffer))
  {
    ProgOfRpt = Public_ConvertBufferToLong((unsigned char *)buffer);
  }    
  if(EepromPram_ReadPram(E2_POSITION_REPORT_STRATEGY_ID,(unsigned char *)buffer))
  {
    StrategyOfRpt = Public_ConvertBufferToLong((unsigned char *)buffer);
  }
  /////////////////////
  LcdClearScreen();
  LcdShowCaption("当前汇报策略:", 0);
  LcdShowCaption(ProgOfRpt?"根据登录/ACC状态":"根据ACC状态",16);
  ///////////
  y=32;
  if(StrategyOfRpt==0)
  {
    LcdShowCaption("定时汇报",y);
    sprintf(buffer,"当前间隔:%d秒",Link1ReportTimeSpace);
  }
  else
    if(StrategyOfRpt==1)
    {
      LcdShowCaption("定距汇报",y);
      sprintf(buffer,"当前间隔:%d米",Link1ReportDistanceSpace);
    }
    else
      if(StrategyOfRpt==2)
      {
        LcdShowCaption("定时定距汇报",y);        
        sprintf(buffer,"间隔%ds/%dm",Link1ReportTimeSpace,Link1ReportDistanceSpace);
      }
      else
      {
        buffer[0]='\0';
      }
  y+=16;
  LcdShowCaption(buffer,y);
}
/*************************************************************
** 函数名称: TaskSystemDebugCurrentReportMode
** 功能描述: "当前汇报策略:"菜单
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskSystemDebugCurrentReportMode(LZM_MESSAGE *Msg)
{
  switch(Msg->message)
  {
  case TASK_START:
    ShowCurrentReportMode();
    return 1;
  case KEY_PRESS:
    switch(TaskPara)
    {
    case KEY_BACK://返回
      LZM_StartNewTask(TaskMenuSystemDebug);//系统管理
      break;
    case KEY_ENTER://确定
      break;                                
    case KEY_UP:                    
      break;
    case KEY_DOWN:                    
      break;
    }
    return 1;
  case TEST_TIMER:
    break;
  case INT_RTC:
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
/*************************************************************
** 函数名称: SystemDebugFixTime
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void SystemDebugFixTimeOk(void)
{
  unsigned char temp;
  unsigned char buffer[17];
  unsigned long val,val1;
  temp=0x55;
  EepromPram_WritePram(E2_INSTALL_TIME_FLAG_ID, &temp,1);
  val=RTC_GetCounter();
  Public_ConvertLongToBuffer(val,buffer);
  EepromPram_WritePram(E2_INSTALL_TIME_ID, buffer,4);
  val1=ConvertReadPramToValue(E2_INSTALL_TIME_ID);
  EepromPram_ReadPram(E2_INSTALL_TIME_FLAG_ID, &temp);
  LcdClearScreen();
  if(val1==val&&temp==0x55)
  {
    LcdShowCaption("保存成功", 23); 
    EepromPram_UpdateVariable(E2_INSTALL_TIME_ID);
    EepromPram_UpdateVariable(E2_INSTALL_TIME_FLAG_ID); 
    LZM_SetAlarm(TEST_TIMER, SECS(1));
  }
  else
  {
    LcdShowCaption("保存失败", 23);  
    
    LZM_SetAlarm(TEST_TIMER, SECS(1));
  }
}
/*************************************************************
** 函数名称: SystemDebugFixTime
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void SystemDebugFixTimeCancel(void)
{
  LZM_StartNewTask(TaskMenuSystemDebug);//系统管理
}

/*************************************************************
** 函数名称: TaskSystemDebugFixTime
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskSystemDebugFixTime(LZM_MESSAGE *Msg)
{
  unsigned char temp;
  unsigned long val;
  TIME_T time;
  switch(Msg->message)
  {
  case TASK_START:
    LcdClearScreen();
    EepromPram_ReadPram(E2_INSTALL_TIME_FLAG_ID, &temp);
    if(temp!=0x55)
    {
      s_stTaskAttrib.ControlId=0;
      PublicConfirmAll("设为安装时间?",SystemDebugFixTimeOk,SystemDebugFixTimeCancel,0,0);
    }
    else
    {
      val=ConvertReadPramToValue(E2_INSTALL_TIME_ID);
      Gmtime(&time,val);
      s_stTaskAttrib.ControlId=0xff;
      LcdShowCaption("安装时间:",1);
      LcdShowHorLine(0, 131, 18, 1);
      Public_LcdShowTime(24,time);                
      LcdShowHorLine(0, 131, 45, 1);
      LcdShowCaption("任意键返回",47);
    }
    return 1;
  case KEY_PRESS:
    if(s_stTaskAttrib.ControlId==0xff)
    {
      LZM_SetAlarm(TEST_TIMER, 1);
      break;
    }
    switch(TaskPara)
    {
    case KEY_BACK://返回
      LZM_StartNewTask(TaskMenuSystemDebug);//系统管理
      break;
    case KEY_ENTER://确定                    
      s_stTaskAttrib.ControlId=0xff;
      break;
    }
    return 1;
  case TEST_TIMER:
    LZM_StartNewTask(TaskMenuSystemDebug);//系统管理
    break;
  case INT_RTC:
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
/*************************************************************
** 函数名称: SystemDebugSetPositionEx
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void SystemDebugSetPositionEx(void)
{
  LcdClearScreen();
  if(BMA250_RollOverInitAngleCalibration())
  {
    LcdShowCaption("保存成功", 23); 
    LZM_SetAlarm(TEST_TIMER, SECS(1));
  }
  else
  {
    LcdShowCaption("保存失败", 23);
    LZM_SetAlarm(TEST_TIMER, SECS(1));
  }
}
/*************************************************************
** 函数名称: SystemDebugFixTime
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void SystemDebugSetPosition(void)
{
  LcdClearScreen();
  LcdShowCaption("正在校准", 23); 
  LZM_SetOperateTimer(SystemDebugSetPositionEx, PUBLICSECS(1));    
}
/*************************************************************
** 函数名称: CurrentSystemLogout
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void CurrentSystemLogout(void)
{
  LcdClearScreen();
  Lcd_SendTerminalLogout();
#ifdef HUOYUN_DEBUG_OPEN
#else
  LcdClearArea(0,19,131,44);
  LcdShowCaption("注销发送成功", 24);
#endif
  LZM_SetAlarm(TEST_TIMER, SECS(1.5));
}
/*************************************************************
** 函数名称: TaskSystemDebugSetPassWord
** 功能描述: "修改密码"菜单
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskSystemDebugSetPassWord(LZM_MESSAGE *Msg)
{
  unsigned long val,val1;
  unsigned char buf[6];
  switch(Msg->message)
  {
  case TASK_START:
    LcdClearScreen();
    s_stTaskAttrib.ControlId=0;
    LcdShowCaption("修改密码",0);
    LcdShowHorLine(0, 131, 18, 1);
    LcdShowHorLine(0, 131, 45, 1);
    LcdShowCaption("按<确认>保存",47);            
    LcdShowStr(1, 24, "密 码:", 0);
    ESpinInit(&s_stESpin[s_stTaskAttrib.ControlId], 64, 24, 6,s_stTaskAttrib.SysPassword);
    ESpinShow(&s_stESpin[s_stTaskAttrib.ControlId]);
    ESpinSetFocus(&s_stESpin[s_stTaskAttrib.ControlId]);
    return 1;
  case KEY_PRESS:
    if(s_stTaskAttrib.ControlId==0xff)break;
    switch(TaskPara)
    {
    case KEY_BACK://返回
      LZM_StartNewTask(TaskMenuSystemDebug);//系统管理
      break;
    case KEY_ENTER://确定                    
      val=atol(s_stESpin[s_stTaskAttrib.ControlId].buffer);
      if(val==s_stTaskAttrib.SysPassword)
      {
        s_stTaskAttrib.ControlId=0xff;
        LZM_SetAlarm(TEST_TIMER, SECS(0.5));
        break;
      }
      LcdClearArea(0,19,131,44);
      LcdShowCaption("正在保存...", 24);
      Public_ConvertLongToBuffer(val,buf);
      EepromPram_WritePram(E2_PASSWORD_ID, buf,4);
      val1=ConvertReadPramToValue(E2_PASSWORD_ID);
      LcdClearArea(0,19,131,44);
      if(val==val1)
      {
        LcdClearArea(0,44,131,63);
        LcdShowCaption("密码修改成功", 22);
        LcdShowCaption("请记住新密码", 40);
        s_stTaskAttrib.SysPassword=(val&0x00ffffff);
        
      }
      else
      {
        LcdShowCaption("保存错误,请检查", 24); 
        val=s_stTaskAttrib.SysPassword;
        Public_ConvertLongToBuffer(val,buf);
        EepromPram_WritePram(E2_PASSWORD_ID, buf,4);
        
      }
      s_stTaskAttrib.ControlId=0xff;
      LZM_SetAlarm(TEST_TIMER, SECS(2));
      break;                                
    default:                    
      ESpinResponse(&s_stESpin[s_stTaskAttrib.ControlId],TaskPara);                     
      break;
    }
    return 1;
  case TEST_TIMER:
    LZM_StartNewTask(TaskMenuSystemDebug);//系统管理
    break;
  case INT_RTC:
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}

/*************************************************************
** 函数名称: TaskMenuLogin
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void LoadDefaultParamter(void)
{
  LcdClearScreen();
  LcdShowCaption("正在恢复出厂",10);
  LcdShowCaption("设置,请稍候",32);
  EepromPram_DefaultSet();//modify by joneming 20130312
  LcdClearScreen();
  LcdShowCaption("已恢复出厂设置",23);    
  LZM_SetAlarm(TEST_TIMER, SECS(1.5));
}
#ifdef __LZMINPUTBOX_H
/*************************************************************
** 函数名称: TaskShowMainInterface
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskSystemSetCarPlateNumber(LZM_MESSAGE *Msg)
{
  unsigned char val,i;
  switch(Msg->message)
  {
  case TASK_START:
    LcdClearScreen();
    EepromPram_ReadPram(E2_CAR_PLATE_NUM_ID,GB2312Buffer);
    if(GB2312Buffer[0]<0x80)
    {
      strcpy((char *)GB2312Buffer,"粤B12345"); 
    }
    else
      for(i=0; i<8; i++)  
      {
        if(GB2312Buffer[i]<0x30)
          GB2312Buffer[i]=0x30;
      }
    GB2312Buffer[8]='\0';
    LcdShowStr(30,16,"车牌号码:",0);
    LcdShowStr(24,32,(char *)GB2312Buffer,0);
    LcdShowStr(16,48,"保存",0);
    LcdShowStr(80,48,"取消",0);
    LZM_InputBoxInit(&stInputBox[0],24,32,2,(char *)GB2312Buffer,(char **)c_apcInputStringEx);
    LZM_InputBoxInit(&stInputBox[1],40,32,6,(char *)&GB2312Buffer[2],(char **)c_apcInputString);
    stInputBox[0].loop = 0;
    stInputBox[0].step = 2;
    stInputBox[1].loop = 0;
    LZM_InputBoxShow(&stInputBox[0]);
    LZM_InputBoxShow(&stInputBox[1]);
    LZM_InputBoxSetFocus(&stInputBox[0]);
    s_stTaskAttrib.ControlId = 0;
    return 1;
  case KEY_PRESS:
    if(s_stTaskAttrib.ControlId < 2)
    {
      val=LZM_InputBoxResponse(&stInputBox[s_stTaskAttrib.ControlId],TaskPara);
      if(val==2||val==3)
      {
        LZM_InputBoxLoseFocus(&stInputBox[s_stTaskAttrib.ControlId]);
        s_stTaskAttrib.ControlId++;
        if(s_stTaskAttrib.ControlId==1)
          LZM_InputBoxSetFocus(&stInputBox[1]);
        else
          LcdShowStr(16,48,"保存",1);
      }
      else
        if(val==1)
        {
          if(0==s_stTaskAttrib.ControlId)
          {
            s_stTaskAttrib.ControlId = 3;
            LcdShowStr(80,48,"取消",1);
          }
          else
          {
            LZM_InputBoxLoseFocus(&stInputBox[s_stTaskAttrib.ControlId]);
            s_stTaskAttrib.ControlId = 0;
            LZM_InputBoxSetFocusEx(&stInputBox[0]);
          }
        }
        else
          if(val)
          {
            LZM_StartNewTask(TaskMenuSystemCarInfo);//系统管理
          }
      break;
    }
    switch(TaskPara)
    {
    case KEY_BACK://菜单
      LZM_StartNewTask(TaskMenuSystemCarInfo);//系统管理
      break;
    case KEY_UP://
      if(2==s_stTaskAttrib.ControlId)
      {
        s_stTaskAttrib.ControlId = 1;
        LcdShowStr(16,48,"保存",0);
        LZM_InputBoxSetFocusEx(&stInputBox[s_stTaskAttrib.ControlId]);
      }
      else
        if(3==s_stTaskAttrib.ControlId)
        {
          s_stTaskAttrib.ControlId = 2;
          LcdShowStr(16,48,"保存",1);
          LcdShowStr(80,48,"取消",0);
        }
      break;
    case KEY_DOWN://
      if(2==s_stTaskAttrib.ControlId)
      {
        s_stTaskAttrib.ControlId = 3;
        LcdShowStr(16,48,"保存",0);
        LcdShowStr(80,48,"取消",1);
      }
      else
        if(3==s_stTaskAttrib.ControlId)
        {
          s_stTaskAttrib.ControlId = 0;
          LcdShowStr(80,48,"取消",0);
          LZM_InputBoxSetFocus(&stInputBox[s_stTaskAttrib.ControlId]);
        }
      break;
    case KEY_ENTER://确定
      if(2==s_stTaskAttrib.ControlId)
      {
        LcdClearScreen();
        LcdShowCaption("正在保存...", 24);
        strcpy((char *)GB2312Buffer,stInputBox[0].buffer);
        strcat((char *)GB2312Buffer,stInputBox[1].buffer);
        EepromPram_WritePram(E2_CAR_PLATE_NUM_ID, GB2312Buffer,strlen((char *)GB2312Buffer));
        ///////////////////////////////////////
        val=EepromPram_ReadPram(E2_CAR_PLATE_NUM_ID, (unsigned char *)s_stTaskAttrib.ShowBuffer[0]);
        LcdClearArea(0,19,131,44);
        if(strncmp(s_stTaskAttrib.ShowBuffer[0],(char*)GB2312Buffer,val)==0)
        {
          LcdShowCaption("改更成功", 23); 
          EepromPram_UpdateVariable(E2_CAR_PLATE_NUM_ID);                           
          LZM_SetAlarm(TEST_TIMER, SECS(1));
        }
        else
        {
          LcdShowCaption("改更失败", 23);  
          
          LZM_SetAlarm(TEST_TIMER, SECS(1));
        }
      }
      else
        if(3==s_stTaskAttrib.ControlId)
        {
          LZM_StartNewTask(TaskMenuSystemCarInfo);//系统管理
        }
      break;
    }
    return 1;
  case TEST_TIMER:
    LZM_StartNewTask(TaskMenuSystemCarInfo);//系统管理
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
#else
/*************************************************************
** 函数名称: ShowReverseBarForOne
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void ShowReverseBarForOne(unsigned char startx,unsigned char starty,unsigned char post)
{
  unsigned short x,y;
  y=starty;
  x=startx+post*8;
  if(x+8>128)
  {
    x -= 128;
    y += 16;
  }
  LcdReverseBar(x,y,x+7,y+15);
}
/*************************************************************
** 函数名称: ShowReverseBarForOneEx
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void ShowReverseBarForOneEx(unsigned char startx,unsigned char starty,unsigned char post)
{
  unsigned short x,y;
  y=starty;
  x=startx+post*16;
  if(x+16>128)
  {
    x -= 128;
    y += 16;
  }
  LcdReverseBar(x,y,x+15,y+15);
}
/*************************************************************
** 函数名称: TaskSystemSetCarPlateNumber
** 功能描述: ""车牌号码:""菜单
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskSystemSetCarPlateNumber(LZM_MESSAGE *Msg)
{
  static unsigned char sucIuputSite,sucSelectSite,sucSaveFlag,sucShowFlag;
  unsigned char temp,i;
  switch(Msg->message)
  {
  case TASK_START:
    LcdClearScreen();
    s_stTaskAttrib.select=0;
    s_stTaskAttrib.ControlId=0;
    sucSaveFlag=0;
    sucIuputSite=0;
    sucSelectSite=0;
    sucShowFlag=0;
    LcdShowStr(0,0,c_apcInputStringEx[s_stTaskAttrib.ControlId],0);            
    temp=EepromPram_ReadPram(E2_CAR_PLATE_NUM_ID, GB2312Buffer);
    if(GB2312Buffer[0]<0x80)
    {
      strcpy((char *)GB2312Buffer,"粤B123456"); 
    }
    else
      for(i=0; i<9; i++)  
      {
        if(GB2312Buffer[i]<0x30)
          GB2312Buffer[i]=0x30;
      }
    GB2312Buffer[9]='\0';
    LcdShowStr(30,16,"车牌号码:",0);
    LcdShowStr(24,32,(char *)GB2312Buffer,0);
    LcdShowStr(16,48,"保存",0);
    LcdShowStr(80,48,"取消",0);
    LZM_SetTimer(TEST_TIMER, SECS(0.4));
    return 1;
  case KEY_PRESS:
    switch(TaskPara)
    {
    case KEY_BACK://返回
      if(s_stTaskAttrib.select==1)
      {
        s_stTaskAttrib.select=0;
        if(!sucShowFlag)
        {
          LcdShowStr(0,0,c_apcInputStringEx[s_stTaskAttrib.ControlId],0);
        }
        else
        {
          LcdShowStr(16,0,c_apcInputString[s_stTaskAttrib.ControlId],0);
        }
      }
      else
      {
        LZM_KillTimer(TEST_TIMER);
        LZM_StartNewTask(TaskMenuSystemCarInfo);//系统管理
      }
      break;
    case KEY_ENTER://确定
      LZM_KillTimer(TEST_TIMER);
      if(s_stTaskAttrib.select==0)
      {
        s_stTaskAttrib.select=1;
        if(!sucShowFlag)
        {
          LcdClearArea(0,0,131,16);
          LcdShowStr(0,0,c_apcInputStringEx[s_stTaskAttrib.ControlId],0);
          LcdShowStr(24,32,(char *)GB2312Buffer,0);
        }
        else
        {
          LcdClearArea(0,0,131,16);
          LcdShowStr(16,0,c_apcInputString[s_stTaskAttrib.ControlId],0);
          LcdShowStr(24,32,(char *)GB2312Buffer,0);
        }
        LZM_SetTimer(TEST_TIMER, SECS(0.4));
      }
      else
        if(s_stTaskAttrib.select==1)
        {
          s_stTaskAttrib.select=0;
          if(!sucShowFlag)
          {
            sucShowFlag=1;
            GB2312Buffer[0]=c_apcInputStringEx[s_stTaskAttrib.ControlId][2*sucSelectSite];
            GB2312Buffer[1]=c_apcInputStringEx[s_stTaskAttrib.ControlId][2*sucSelectSite+1];
            s_stTaskAttrib.ControlId=0;                            
            sucSelectSite=0;
            sucIuputSite =1;
          }
          else
          {
            GB2312Buffer[sucIuputSite]=c_apcInputString[s_stTaskAttrib.ControlId][sucSelectSite];
          }
          LcdClearArea(0,0,131,16);
          LcdShowStr(16,0,c_apcInputString[s_stTaskAttrib.ControlId],0);
          LcdShowStr(24,32,(char *)GB2312Buffer,0);
          /////////////////
          ////////////////
          if(sucIuputSite<7)
          {
            sucIuputSite++;
            LZM_SetTimer(TEST_TIMER, SECS(0.4));
          }
          else
          {
            s_stTaskAttrib.select=2;
            sucSaveFlag=0;
            LZM_KillTimer(TEST_TIMER);                            
            LcdShowStr(16,48,"保存",1-sucSaveFlag);
            LcdShowStr(80,48,"取消",sucSaveFlag);
          }
        }
        else
          if(s_stTaskAttrib.select==2)
          {
            if(sucSaveFlag==0)
            {
              LcdClearScreen();
              LcdShowCaption("正在保存...", 24);
              s_stTaskAttrib.select=3;
              EepromPram_WritePram(E2_CAR_PLATE_NUM_ID, GB2312Buffer,strlen((char *)GB2312Buffer));
              ///////////////////////////////////////
              temp=EepromPram_ReadPram(E2_CAR_PLATE_NUM_ID, (unsigned char *)&s_stTaskAttrib.ShowBuffer[0]);
              LcdClearArea(0,19,131,44);
              if(strncmp((char*)&s_stTaskAttrib.ShowBuffer[0],(char*)GB2312Buffer,temp)==0)
              {
                LcdShowCaption("改更成功", 23); 
                EepromPram_UpdateVariable(E2_CAR_PLATE_NUM_ID);   
								//行车记录仪参数修改记录
            		Recorder_ParaSet(0x82);
                LZM_SetAlarm(TEST_TIMER, SECS(1));
              }
              else
              {
                LcdShowCaption("改更失败", 23);  
                
                LZM_SetAlarm(TEST_TIMER, SECS(1));
              }
            }
            else
            {
              LZM_StartNewTask(TaskMenuSystemCarInfo);//系统管理
            }
          }                    
      break;                                
    case KEY_UP:
      if(s_stTaskAttrib.select==0)
      {
        if(sucIuputSite>2)
        {
          sucIuputSite--;
          LcdShowStr(24,32,(char *)GB2312Buffer,0);
        }
        else
          if(sucShowFlag==1) 
          {
            sucShowFlag=0;
            sucIuputSite=0;
            s_stTaskAttrib.ControlId=0;
            sucSelectSite =0;
            LcdShowStr(24,32,(char *)GB2312Buffer,0);
          }
          else
          {
            s_stTaskAttrib.select=2;
            sucSaveFlag=1;                            
            LcdShowStr(24,32,(char *)GB2312Buffer,0);
            LZM_KillTimer(TEST_TIMER);
            LcdShowStr(16,48,"保存",1-sucSaveFlag);
            LcdShowStr(80,48,"取消",sucSaveFlag);
          }
      }
      else
        if(s_stTaskAttrib.select==1)
        {
          if(!sucShowFlag)
          {
            if(s_stTaskAttrib.ControlId<3)s_stTaskAttrib.ControlId++;
            else
              s_stTaskAttrib.ControlId=0;
            ///////////////////////////
            temp =(strlen(c_apcInputStringEx[s_stTaskAttrib.ControlId]))>>1;
            temp--;
            if(sucSelectSite>temp)sucSelectSite=temp;
            LcdClearArea(100, 0, 128,16);
            ///////////////////////////
            LcdShowStr(0,0,c_apcInputStringEx[s_stTaskAttrib.ControlId],0);
          }
          else
          {
            if(s_stTaskAttrib.ControlId<2)s_stTaskAttrib.ControlId++;
            else
              s_stTaskAttrib.ControlId=0;
            LcdShowStr(16,0,c_apcInputString[s_stTaskAttrib.ControlId],0);
            
          }
        }
        else
          if(s_stTaskAttrib.select==2)
          {
            if(sucSaveFlag==1)
            {
              sucSaveFlag=0;
              LcdShowStr(16,48,"保存",1-sucSaveFlag);
              LcdShowStr(80,48,"取消",sucSaveFlag);
            }
            else
              if(sucSaveFlag==0)
              {
                LZM_SetTimer(TEST_TIMER, SECS(0.4));
                s_stTaskAttrib.select=0;
                sucIuputSite=8;
                sucShowFlag=1;
                s_stTaskAttrib.ControlId=0;
                sucSelectSite =0;
                LcdShowStr(24,32,(char *)GB2312Buffer,0);
                LcdShowStr(16,48,"保存",0);
                LcdShowStr(80,48,"取消",0);
              }
          }
      break;
    case KEY_DOWN: 
      if(s_stTaskAttrib.select==0)
      {
        if(sucShowFlag==0)
        {
          sucShowFlag=1;
          s_stTaskAttrib.ControlId=0;
          sucSelectSite =0;
          LcdShowStr(24,32,(char *)GB2312Buffer,0);
          sucIuputSite=2;
        }
        else
        {
          if(sucIuputSite<8)
          {
            sucIuputSite++;
            LcdShowStr(24,32,(char *)GB2312Buffer,0);
          }
          else
          {
            s_stTaskAttrib.select=2;
            sucSaveFlag=0;
            LZM_KillTimer(TEST_TIMER);
            LcdShowStr(24,32,(char *)GB2312Buffer,0);
            LcdShowStr(16,48,"保存",1-sucSaveFlag);
            LcdShowStr(80,48,"取消",sucSaveFlag);
          }
        }
      }
      else
        if(s_stTaskAttrib.select==1)
        {
          if(sucShowFlag==0)
          {
            LcdShowStr(0,0,c_apcInputStringEx[s_stTaskAttrib.ControlId],0); 
            temp=(strlen(c_apcInputStringEx[s_stTaskAttrib.ControlId]))>>1;
            temp--;
            if(sucSelectSite< temp )sucSelectSite++;
            else sucSelectSite=0;
          }
          else
          {
            LcdShowStr(16,0,c_apcInputString[s_stTaskAttrib.ControlId],0); 
            if(sucSelectSite<11)sucSelectSite++;
            else sucSelectSite=0;
          }
        }
        else
          if(s_stTaskAttrib.select==2)
          {
            if(sucSaveFlag==0)
            {
              sucSaveFlag=1;                            
              LcdShowStr(16,48,"保存",1-sucSaveFlag);
              LcdShowStr(80,48,"取消",sucSaveFlag);
            }
            else
              if(sucSaveFlag==1)
              {
                LZM_SetTimer(TEST_TIMER, SECS(0.4));
                LcdShowStr(16,48,"保存",0);
                LcdShowStr(80,48,"取消",0);
                s_stTaskAttrib.select=0;
                sucShowFlag=0;
                sucIuputSite=0;
                s_stTaskAttrib.ControlId=0;
                sucSelectSite =0;
                LcdShowStr(24,32,(char *)GB2312Buffer,0);
              }
          }                    
      break;
    }
    return 1;
  case TEST_TIMER:            
    if(s_stTaskAttrib.select==0)
    {
      if(!sucShowFlag)
      {
        ShowReverseBarForOneEx(24,32,0);
      }
      else
      {
        ShowReverseBarForOne(24,32,sucIuputSite);
      }
    }
    else
      if(s_stTaskAttrib.select==1)
      {
        if(!sucShowFlag)
        {
          ShowReverseBarForOneEx(0,0,sucSelectSite);
        }
        else
        {
          ShowReverseBarForOne(16,0,sucSelectSite);
        }
      }
      else
        if(s_stTaskAttrib.select==3)
        {
          LZM_KillTimer(TEST_TIMER);
          LZM_StartNewTask(TaskMenuSystemCarInfo);//系统管理
        }
    break;
  case INT_RTC:
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
#endif
/*************************************************************
** 函数名称: TaskSetCarPlateColor
** 功能描述: "设置车牌颜色"菜单
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskSystemSetCarPlateColor(LZM_MESSAGE *Msg)
{    
  unsigned char handle;
  switch(Msg->message)
  {
  case TASK_START:
    LcdClearScreen();           
    LcdShowCaption("选择车牌颜色",0);
    LcdShowHorLine(0, 131, 18, 1);
    LcdShowHorLine(0, 131, 45, 1);
    LcdShowCaption("按<确认>保存",47);
    SpinInit(&s_stSpin[0],36,24,(char **)c_apcCarPlakeColorItems,"");
    EepromPram_ReadPram(E2_CAR_PLATE_COLOR_ID, &handle); 
    if(handle>5)handle=5;
    else if(handle==0)handle=6;
    s_stTaskAttrib.select=handle;
    s_stSpin[0].handle=handle-1;
    SpinShow(&s_stSpin[0]);
    SpinSetFocus(&s_stSpin[0]);
    s_stTaskAttrib.ControlId=0;            
    return 1;
  case KEY_PRESS:
    if(s_stTaskAttrib.ControlId==0xff)
    {
      LZM_StartNewTask(TaskMenuSystemCarInfo);//系统管理
      break;
    }
    switch(TaskPara)
    {
    case KEY_BACK://返回
      LZM_StartNewTask(TaskMenuSystemCarInfo);//系统管理
      break;
    case KEY_ENTER://确定
      handle=SpinGetHandle(&s_stSpin[0])+1;
      if(handle!=s_stTaskAttrib.select)
      {
        if(5==handle)handle=9;
        else if(6==handle)handle=0;
        s_stTaskAttrib.select=handle;                        
        LcdClearArea(0,19,131,44);
        LcdShowCaption("正在保存..", 23);                        
        EepromPram_WritePram(E2_CAR_PLATE_COLOR_ID, &handle, 1);
        EepromPram_ReadPram(E2_CAR_PLATE_COLOR_ID, &handle);
        LcdClearArea(0,19,131,44);
        if(handle==s_stTaskAttrib.select)
        {
          LcdShowCaption("改更成功", 23); 
          EepromPram_UpdateVariable(E2_CAR_PLATE_COLOR_ID);                            
          LZM_SetAlarm(TEST_TIMER, SECS(1));
        }
        else
        {
          LcdShowCaption("改更失败", 23);
          LZM_SetAlarm(TEST_TIMER, SECS(1));
        }
      }
      else
      {
        LZM_SetAlarm(TEST_TIMER, SECS(0.5));
      }
      s_stTaskAttrib.ControlId=0xff; 
      break;                                
    default:                    
      SpinResponse(&s_stSpin[0],TaskPara);  
      break;
    }
    return 1;
  case TEST_TIMER:
    LZM_StartNewTask(TaskMenuSystemCarInfo);//系统管理
    break;
  case INT_RTC:
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
/*************************************************************
** 函数名称: TaskSystemSetCarPlateType
** 功能描述: "设置车牌分类"菜单
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskSystemSetCarPlateType(LZM_MESSAGE *Msg)
{    
  unsigned char handle;
  unsigned char buffer[30];
  switch(Msg->message)
  {
  case TASK_START:
    LcdClearScreen();           
    LcdShowCaption("选择车牌分类",0);
    LcdShowHorLine(0, 131, 18, 1);
    LcdShowHorLine(0, 131, 45, 1);
    LcdShowCaption("按<确认>保存",47);
    EepromPram_ReadPram(E2_CAR_TYPE_ID, buffer);
    SpinInit(&s_stSpin[0],16,24,(char **)c_apcCarPlakeTypeItems,"");
    s_stTaskAttrib.select=0xff;
    for(handle=0;handle<11;handle++)
    {
      if(strncmp((char*)buffer,(char*)c_apcCarPlakeTypeItems[handle],8)==0)
      {
        s_stTaskAttrib.select=handle;
        break;
      }
    }
    s_stSpin[0].handle=handle;
    SpinShow(&s_stSpin[0]);
    SpinSetFocus(&s_stSpin[0]);
    s_stTaskAttrib.ControlId=0;
    return 1;
  case KEY_PRESS:
    if(s_stTaskAttrib.ControlId==0xff)
    {
      LZM_StartNewTask(TaskMenuSystemCarInfo);//系统管理
      break;
    }
    switch(TaskPara)
    {
    case KEY_BACK://返回
      LZM_StartNewTask(TaskMenuSystemCarInfo);//系统管理
      break;
    case KEY_ENTER://确定
      handle=SpinGetHandle(&s_stSpin[0]);
      if(handle!=s_stTaskAttrib.select)
      {
        s_stTaskAttrib.select=handle;
        LcdClearArea(0,19,131,44);
        LcdShowCaption("正在保存..", 23);
        EepromPram_WritePram(E2_CAR_TYPE_ID, (unsigned char *)c_apcCarPlakeTypeItems[handle], strlen(c_apcCarPlakeTypeItems[handle]));                        
        EepromPram_ReadPram(E2_CAR_TYPE_ID, buffer);
        LcdClearArea(0,19,131,44);
        if(strncmp((char*)buffer,(char*)c_apcCarPlakeTypeItems[handle],4)==0)
        {
          LcdShowCaption("改更成功", 23); 
          EepromPram_UpdateVariable(E2_CAR_TYPE_ID);     
					//行车记录仪参数修改记录
          Recorder_ParaSet(0x82);
          LZM_SetAlarm(TEST_TIMER, SECS(1));
        }
        else
        {
          LcdShowCaption("改更失败", 23);  
          
          LZM_SetAlarm(TEST_TIMER, SECS(1));
        }
      }
      else
      {
        LZM_SetAlarm(TEST_TIMER, SECS(0.5));
      }
      s_stTaskAttrib.ControlId=0xff; 
      break;                                
    default:                    
      SpinResponse(&s_stSpin[0],TaskPara);  
      break;
    }
    return 1;
  case TEST_TIMER:
    LZM_StartNewTask(TaskMenuSystemCarInfo);//系统管理
    break;
  case INT_RTC:
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}

#ifdef __LZMINPUTBOX_H
/*************************************************************
** 函数名称: TaskShowMainInterface
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskSystemSetCarVinNumber(LZM_MESSAGE *Msg)
{
  unsigned char val,i;
  switch(Msg->message)
  {
  case TASK_START:
    LcdClearScreen();
    EepromPram_ReadPram(E2_CAR_IDENTIFICATION_CODE_ID, GB2312Buffer);
    if(GB2312Buffer[0]<0x30||GB2312Buffer[0]>0x80)
    {
      strcpy((char *)GB2312Buffer,"LSVAE000000000000");
    }
    else
      for(i=0; i<17; i++)
      {
        if(GB2312Buffer[i]<0x30||GB2312Buffer[i]>0x80)
          GB2312Buffer[i]=0x30;
      }
    GB2312Buffer[17]='\0';
    LcdShowStr(2,16,"车辆VIN:",0);
    LcdShowStr(16,48,"保存",0);
    LcdShowStr(80,48,"取消",0);
    LZM_InputBoxInit(&stInputBox[0],72,16,17,(char *)GB2312Buffer,(char **)c_apcInputString);
    stInputBox[0].loop = 0;
    LZM_InputBoxSetFocus(&stInputBox[0]);
    s_stTaskAttrib.ControlId = 0;
    return 1;
  case KEY_PRESS:
    if(0==s_stTaskAttrib.ControlId)
    {
      val=LZM_InputBoxResponse(&stInputBox[0],TaskPara);
      if(val==2||val==3)
      {
        s_stTaskAttrib.ControlId = 1;
        LcdShowStr(16,48,"保存",1);
      }
      else
        if(val==1)
        {
          s_stTaskAttrib.ControlId = 2;
          LcdShowStr(80,48,"取消",1);
        }
        else
          if(val)
          {
            LZM_StartNewTask(TaskMenuSystemCarInfo);//系统管理
          }
      break;
    }
    switch(TaskPara)
    {
    case KEY_BACK://菜单
      LZM_StartNewTask(TaskMenuSystemCarInfo);//系统管理
      break;
    case KEY_UP://
      if(1==s_stTaskAttrib.ControlId)
      {
        s_stTaskAttrib.ControlId = 0;
        LcdShowStr(16,48,"保存",0);
        LZM_InputBoxSetFocusEx(&stInputBox[0]);
      }
      else
        if(2==s_stTaskAttrib.ControlId)
        {
          s_stTaskAttrib.ControlId = 1;
          LcdShowStr(16,48,"保存",1);
          LcdShowStr(80,48,"取消",0);
        }
      break;
    case KEY_DOWN://
      if(1==s_stTaskAttrib.ControlId)
      {
        s_stTaskAttrib.ControlId = 2;
        LcdShowStr(16,48,"保存",0);
        LcdShowStr(80,48,"取消",1);
      }
      else
        if(2==s_stTaskAttrib.ControlId)
        {
          s_stTaskAttrib.ControlId = 0;
          LcdShowStr(80,48,"取消",0);
          LZM_InputBoxSetFocus(&stInputBox[0]);
        }
      break;
    case KEY_ENTER://确定
      if(1==s_stTaskAttrib.ControlId)
      {
        LcdClearScreen();
        LcdShowCaption("正在保存...", 24);
        EepromPram_WritePram(E2_CAR_IDENTIFICATION_CODE_ID,(unsigned char *)stInputBox[0].buffer,strlen(stInputBox[0].buffer));
        ////////////////////////////////////
        val=EepromPram_ReadPram(E2_CAR_IDENTIFICATION_CODE_ID, (unsigned char *)&s_stTaskAttrib.ShowBuffer[0]);
        LcdClearArea(0,19,131,44);
        if(strncmp(s_stTaskAttrib.ShowBuffer[0],stInputBox[0].buffer,val)==0)
        {
          LcdShowCaption("改更成功", 23); 
          EepromPram_UpdateVariable(E2_CAR_IDENTIFICATION_CODE_ID);                           
          LZM_SetAlarm(TEST_TIMER, SECS(1));
        }
        else
        {
          LcdShowCaption("改更失败", 23);                             
          LZM_SetAlarm(TEST_TIMER, SECS(1));
        }
      }
      else
        if(2==s_stTaskAttrib.ControlId)
        {
          LZM_StartNewTask(TaskMenuSystemCarInfo);//系统管理
        }
      break;
    }
    return 1;
  case TEST_TIMER:
    LZM_StartNewTask(TaskMenuSystemCarInfo);//系统管理
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
#else
/*************************************************************
** 函数名称: TaskSystemSetCarVinNumber
** 功能描述: "车辆VIN"
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskSystemSetCarVinNumber(LZM_MESSAGE *Msg)
{
  static unsigned char sucIuputSite,sucSelectSite,sucSaveFlag;
  unsigned char temp,i;
  switch(Msg->message)
  {
  case TASK_START:
    LcdClearScreen();
    s_stTaskAttrib.select=0;
    s_stTaskAttrib.ControlId=0;
    sucSaveFlag=0;
    sucIuputSite=0;
    sucSelectSite=0;
    LcdShowStr(12,0,c_apcInputString[s_stTaskAttrib.ControlId],0);            
    temp=EepromPram_ReadPram(E2_CAR_IDENTIFICATION_CODE_ID, GB2312Buffer);
    if(GB2312Buffer[0]<0x30||GB2312Buffer[0]>0x80)
    {
      strcpy((char *)GB2312Buffer,"LSVAE000000000000");
    }
    else
      for(i=0; i<17; i++)
      {
        if(GB2312Buffer[i]<0x30||GB2312Buffer[i]>0x80)
          GB2312Buffer[i]=0x30;
      }
    GB2312Buffer[17]='\0';
    LcdShowStr(2,16,"车辆VIN:",0);
    LcdShowStr(72,16,(char *)GB2312Buffer,0);
    LcdShowStr(16,48,"保存",0);
    LcdShowStr(80,48,"取消",0);
    LZM_SetTimer(TEST_TIMER, SECS(0.4));
    return 1;
  case KEY_PRESS:
    switch(TaskPara)
    {
    case KEY_BACK://返回
      if(s_stTaskAttrib.select==1)
      {
        s_stTaskAttrib.select=0;
        LcdShowStr(12,0,c_apcInputString[s_stTaskAttrib.ControlId],0);
        LcdShowStr(72,16,(char *)GB2312Buffer,0);
      }
      else
      {
        LZM_KillTimer(TEST_TIMER);
        LZM_StartNewTask(TaskMenuSystemCarInfo);//系统管理
      }
      break;
    case KEY_ENTER://确定
      LZM_KillTimer(TEST_TIMER);
      if(s_stTaskAttrib.select==0)
      {
        s_stTaskAttrib.select=1;
        LcdShowStr(12,0,c_apcInputString[s_stTaskAttrib.ControlId],0);
        LcdShowStr(72,16,(char *)GB2312Buffer,0);
        LZM_SetTimer(TEST_TIMER, SECS(0.4));
      }
      else
        if(s_stTaskAttrib.select==1)
        {
          s_stTaskAttrib.select=0;
          LcdShowStr(12,0,c_apcInputString[s_stTaskAttrib.ControlId],0);
          GB2312Buffer[sucIuputSite]=c_apcInputString[s_stTaskAttrib.ControlId][sucSelectSite];
          LcdShowStr(72,16,(char *)GB2312Buffer,0);
          if(sucIuputSite<16)
          {
            sucIuputSite++;
            LZM_SetTimer(TEST_TIMER, SECS(0.4));
          }
          else
          {
            s_stTaskAttrib.select=2;
            sucSaveFlag=0;
            LZM_KillTimer(TEST_TIMER);                            
            LcdShowStr(16,48,"保存",1-sucSaveFlag);
            LcdShowStr(80,48,"取消",sucSaveFlag);
          }
        }
        else
          if(s_stTaskAttrib.select==2)
          {
            if(sucSaveFlag==0)
            {
              LcdClearScreen();
              LcdShowCaption("正在保存...", 24);
              s_stTaskAttrib.select=3;
              EepromPram_WritePram(E2_CAR_IDENTIFICATION_CODE_ID, GB2312Buffer,strlen((char *)GB2312Buffer));
              ////////////////////////////////////
              temp=EepromPram_ReadPram(E2_CAR_IDENTIFICATION_CODE_ID, (unsigned char *)&s_stTaskAttrib.ShowBuffer[0]);
              LcdClearArea(0,19,131,44);
              if(strncmp((char*)&s_stTaskAttrib.ShowBuffer[0],(char*)GB2312Buffer,temp)==0)
              {
                LcdShowCaption("改更成功", 23); 
                EepromPram_UpdateVariable(E2_CAR_IDENTIFICATION_CODE_ID);  
								//行车记录仪参数修改记录
            		Recorder_ParaSet(0x82);
                LZM_SetAlarm(TEST_TIMER, SECS(1));
              }
              else
              {
                LcdShowCaption("改更失败", 23);  
                
                LZM_SetAlarm(TEST_TIMER, SECS(1));
              }
            }
            else
            {
              LZM_StartNewTask(TaskMenuSystemCarInfo);//系统管理
            }
          }                    
      break;                                
    case KEY_UP:
      if(s_stTaskAttrib.select==0)
      {
        if(sucIuputSite)
        {
          sucIuputSite--;
          LcdShowStr(72,16,(char *)GB2312Buffer,0);
        }
        else
        {
          s_stTaskAttrib.select=2;
          sucSaveFlag=1;
          LcdShowStr(72,16,(char *)GB2312Buffer,0);
          LZM_KillTimer(TEST_TIMER);
          LcdShowStr(16,48,"保存",1-sucSaveFlag);
          LcdShowStr(80,48,"取消",sucSaveFlag);
        }
      }
      else
        if(s_stTaskAttrib.select==1)
        {
          if(s_stTaskAttrib.ControlId<2)s_stTaskAttrib.ControlId++;
          else
            s_stTaskAttrib.ControlId=0;
          LcdShowStr(12,0,c_apcInputString[s_stTaskAttrib.ControlId],0);
        }
        else
          if(s_stTaskAttrib.select==2)
          {
            if(sucSaveFlag==1)
            {
              sucSaveFlag=0;
              LcdShowStr(16,48,"保存",1-sucSaveFlag);
              LcdShowStr(80,48,"取消",sucSaveFlag);
            }
            else
              if(sucSaveFlag==0)
              {
                LZM_SetTimer(TEST_TIMER, SECS(0.4));
                s_stTaskAttrib.select=0;
                sucIuputSite=16;
                LcdShowStr(16,48,"保存",0);
                LcdShowStr(80,48,"取消",0);
              }
          }
      break;
    case KEY_DOWN: 
      if(s_stTaskAttrib.select==0)
      {
        if(sucIuputSite<16)
        {
          sucIuputSite++;
          LcdShowStr(72,16,(char *)GB2312Buffer,0);
        }
        else
        {
          s_stTaskAttrib.select=2;
          sucSaveFlag=0;
          LZM_KillTimer(TEST_TIMER);
          LcdShowStr(72,16,(char *)GB2312Buffer,0);
          LcdShowStr(16,48,"保存",1-sucSaveFlag);
          LcdShowStr(80,48,"取消",sucSaveFlag);
        }
      }
      else
        if(s_stTaskAttrib.select==1)
        {
          LcdShowStr(12,0,c_apcInputString[s_stTaskAttrib.ControlId],0); 
          if(sucSelectSite<11)sucSelectSite++;
          else sucSelectSite=0;
        }
        else
          if(s_stTaskAttrib.select==2)
          {
            if(sucSaveFlag==0)
            {
              sucSaveFlag=1;                            
              LcdShowStr(16,48,"保存",1-sucSaveFlag);
              LcdShowStr(80,48,"取消",sucSaveFlag);
            }
            else
              if(sucSaveFlag==1)
              {
                LZM_SetTimer(TEST_TIMER, SECS(0.4));
                s_stTaskAttrib.select=0;
                sucIuputSite=0;
                LcdShowStr(72,16,(char *)GB2312Buffer,0);
                LcdShowStr(16,48,"保存",0);
                LcdShowStr(80,48,"取消",0);
              }
          }                    
      break;
    }
    return 1;
  case TEST_TIMER:            
    if(s_stTaskAttrib.select==0)
    {
      ShowReverseBarForOne(72,16,sucIuputSite);
    }
    else
      if(s_stTaskAttrib.select==1)
      {
        ShowReverseBarForOne(12,0,sucSelectSite);
      }
      else
        if(s_stTaskAttrib.select==3)
        {
          LZM_KillTimer(TEST_TIMER);
          LZM_StartNewTask(TaskMenuSystemCarInfo);//系统管理
        }
    break;
  case INT_RTC:
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
/*************************************************************
** 函数名称: TaskSystemSetCarEngineNum
** 功能描述: "车辆发动机号"
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskSystemSetCarEngineNum(LZM_MESSAGE *Msg)
{
  static unsigned char sucIuputSite,sucSelectSite,sucSaveFlag;
  unsigned char temp,i;
  switch(Msg->message)
  {
  case TASK_START:
    LcdClearScreen();
    s_stTaskAttrib.select=0;
    s_stTaskAttrib.ControlId=0;
    sucSaveFlag=0;
    sucIuputSite=0;
    sucSelectSite=0;
    LcdShowStr(12,0,c_apcInputString[s_stTaskAttrib.ControlId],0);            
    temp=EepromPram_ReadPram(E2_ENGINE_NUM_ID, GB2312Buffer);
    if(GB2312Buffer[0]<0x30||GB2312Buffer[0]>0x80)
    {
      strcpy((char *)GB2312Buffer,"SZEI000000000000");
    }
    else
      for(i=0; i<16; i++)
      {
        if(GB2312Buffer[i]<0x30||GB2312Buffer[i]>0x80)
          GB2312Buffer[i]=0x30;
      }
    GB2312Buffer[16]='\0';
    LcdShowStr(2,16,"车辆ENG:",0);
    LcdShowStr(72,16,(char *)GB2312Buffer,0);
    LcdShowStr(16,48,"保存",0);
    LcdShowStr(80,48,"取消",0);
    LZM_SetTimer(TEST_TIMER, SECS(0.4));
    return 1;
  case KEY_PRESS:
    switch(TaskPara)
    {
    case KEY_BACK://返回
      if(s_stTaskAttrib.select==1)
      {
        s_stTaskAttrib.select=0;
        LcdShowStr(12,0,c_apcInputString[s_stTaskAttrib.ControlId],0);
        LcdShowStr(72,16,(char *)GB2312Buffer,0);
      }
      else
      {
        LZM_KillTimer(TEST_TIMER);
        LZM_StartNewTask(TaskMenuSystemCarInfo);//系统管理
      }
      break;
    case KEY_ENTER://确定
      LZM_KillTimer(TEST_TIMER);
      if(s_stTaskAttrib.select==0)
      {
        s_stTaskAttrib.select=1;
        LcdShowStr(12,0,c_apcInputString[s_stTaskAttrib.ControlId],0);
        LcdShowStr(72,16,(char *)GB2312Buffer,0);
        LZM_SetTimer(TEST_TIMER, SECS(0.4));
      }
      else
        if(s_stTaskAttrib.select==1)
        {
          s_stTaskAttrib.select=0;
          LcdShowStr(12,0,c_apcInputString[s_stTaskAttrib.ControlId],0);
          GB2312Buffer[sucIuputSite]=c_apcInputString[s_stTaskAttrib.ControlId][sucSelectSite];
          LcdShowStr(72,16,(char *)GB2312Buffer,0);
          if(sucIuputSite<16)
          {
            sucIuputSite++;
            LZM_SetTimer(TEST_TIMER, SECS(0.4));
          }
          else
          {
            s_stTaskAttrib.select=2;
            sucSaveFlag=0;
            LZM_KillTimer(TEST_TIMER);                            
            LcdShowStr(16,48,"保存",1-sucSaveFlag);
            LcdShowStr(80,48,"取消",sucSaveFlag);
          }
        }
        else
          if(s_stTaskAttrib.select==2)
          {
            if(sucSaveFlag==0)
            {
              LcdClearScreen();
              LcdShowCaption("正在保存...", 24);
              s_stTaskAttrib.select=3;
              for(i = 0;i < 16;i++)
              {
                if(GB2312Buffer[i] == '?')//遇到'?'字符串截断
                  break;
              }
              EepromPram_WritePram(E2_ENGINE_NUM_ID, GB2312Buffer,i);
              ////////////////////////////////////
              temp=EepromPram_ReadPram(E2_ENGINE_NUM_ID, (unsigned char *)&s_stTaskAttrib.ShowBuffer[0]);
              LcdClearArea(0,19,131,44);
              if(strncmp((char*)&s_stTaskAttrib.ShowBuffer[0],(char*)GB2312Buffer,temp)==0)
              {
                LcdShowCaption("改更成功", 23); 
                EepromPram_UpdateVariable(E2_ENGINE_NUM_ID);                           
                LZM_SetAlarm(TEST_TIMER, SECS(1));
              }
              else
              {
                LcdShowCaption("改更失败", 23);  
                
                LZM_SetAlarm(TEST_TIMER, SECS(1));
              }
            }
            else
            {
              LZM_StartNewTask(TaskMenuSystemCarInfo);//系统管理
            }
          }                    
      break;                                
    case KEY_UP:
      if(s_stTaskAttrib.select==0)
      {
        if(sucIuputSite)
        {
          sucIuputSite--;
          LcdShowStr(72,16,(char *)GB2312Buffer,0);
        }
        else
        {
          s_stTaskAttrib.select=2;
          sucSaveFlag=1;
          LcdShowStr(72,16,(char *)GB2312Buffer,0);
          LZM_KillTimer(TEST_TIMER);
          LcdShowStr(16,48,"保存",1-sucSaveFlag);
          LcdShowStr(80,48,"取消",sucSaveFlag);
        }
      }
      else
        if(s_stTaskAttrib.select==1)
        {
          if(s_stTaskAttrib.ControlId<2)s_stTaskAttrib.ControlId++;
          else
            s_stTaskAttrib.ControlId=0;
          LcdShowStr(12,0,c_apcInputString[s_stTaskAttrib.ControlId],0);
        }
        else
          if(s_stTaskAttrib.select==2)
          {
            if(sucSaveFlag==1)
            {
              sucSaveFlag=0;
              LcdShowStr(16,48,"保存",1-sucSaveFlag);
              LcdShowStr(80,48,"取消",sucSaveFlag);
            }
            else
              if(sucSaveFlag==0)
              {
                LZM_SetTimer(TEST_TIMER, SECS(0.4));
                s_stTaskAttrib.select=0;
                sucIuputSite=16;
                LcdShowStr(16,48,"保存",0);
                LcdShowStr(80,48,"取消",0);
              }
          }
      break;
    case KEY_DOWN: 
      if(s_stTaskAttrib.select==0)
      {
        if(sucIuputSite<16)
        {
          sucIuputSite++;
          LcdShowStr(72,16,(char *)GB2312Buffer,0);
        }
        else
        {
          s_stTaskAttrib.select=2;
          sucSaveFlag=0;
          LZM_KillTimer(TEST_TIMER);
          LcdShowStr(72,16,(char *)GB2312Buffer,0);
          LcdShowStr(16,48,"保存",1-sucSaveFlag);
          LcdShowStr(80,48,"取消",sucSaveFlag);
        }
      }
      else
        if(s_stTaskAttrib.select==1)
        {
          LcdShowStr(12,0,c_apcInputString[s_stTaskAttrib.ControlId],0); 
          if(sucSelectSite<12)sucSelectSite++;
          else sucSelectSite=0;
        }
        else
          if(s_stTaskAttrib.select==2)
          {
            if(sucSaveFlag==0)
            {
              sucSaveFlag=1;                            
              LcdShowStr(16,48,"保存",1-sucSaveFlag);
              LcdShowStr(80,48,"取消",sucSaveFlag);
            }
            else
              if(sucSaveFlag==1)
              {
                LZM_SetTimer(TEST_TIMER, SECS(0.4));
                s_stTaskAttrib.select=0;
                sucIuputSite=0;
                LcdShowStr(72,16,(char *)GB2312Buffer,0);
                LcdShowStr(16,48,"保存",0);
                LcdShowStr(80,48,"取消",0);
              }
          }                    
      break;
    }
    return 1;
  case TEST_TIMER:            
    if(s_stTaskAttrib.select==0)
    {
      ShowReverseBarForOne(72,16,sucIuputSite);
    }
    else
      if(s_stTaskAttrib.select==1)
      {
        ShowReverseBarForOne(12,0,sucSelectSite);
      }
      else
        if(s_stTaskAttrib.select==3)
        {
          LZM_KillTimer(TEST_TIMER);
          LZM_StartNewTask(TaskMenuSystemCarInfo);//系统管理
        }
    break;
  case INT_RTC:
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}

#endif

/*************************************************************
** 函数名称: TaskSystemCarDoorSet
** 功能描述: "车门有效选择"
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskSystemParamDoorLevelSelect(LZM_MESSAGE *Msg)
{    
  unsigned char handle;
  switch(Msg->message)
  {
  case TASK_START:
    LcdClearScreen();           
    LcdShowCaption("车门有效选择",0);
    LcdShowHorLine(0, 131, 18, 1);
    LcdShowHorLine(0, 131, 45, 1);
    LcdShowCaption("按<确认>保存",47);
    SpinInit(&s_stSpin[0],24,24,(char **)c_apcCarDoorSetItems,"");
    EepromPram_ReadPram(E2_DOOR_SET_ID, &handle);            
    s_stTaskAttrib.select=handle;
    if(handle>1)handle=1;
    s_stSpin[0].handle=handle;
    SpinShow(&s_stSpin[0]);
    SpinSetFocus(&s_stSpin[0]);
    s_stTaskAttrib.ControlId=0;
    return 1;
  case KEY_PRESS:
    if(s_stTaskAttrib.ControlId==0xff)
    {
      LZM_StartNewTask(TaskMenuSystemParameter);//系统管理
      break;
    }
    switch(TaskPara)
    {
    case KEY_BACK://返回
      LZM_StartNewTask(TaskMenuSystemParameter);//系统管理
      break;
    case KEY_ENTER://确定
      handle=SpinGetHandle(&s_stSpin[0]);
      if(handle!=s_stTaskAttrib.select)
      {
        s_stTaskAttrib.select=handle;
        LcdClearArea(0,19,131,44);
        LcdShowCaption("正在保存..", 23);                        
        EepromPram_WritePram(E2_DOOR_SET_ID, &handle, 1);  
        EepromPram_ReadPram(E2_DOOR_SET_ID, &handle);
        LcdClearArea(0,19,131,44);
        if(handle==s_stTaskAttrib.select)
        {
          LcdShowCaption("改更成功", 23); 
          EepromPram_UpdateVariable(E2_DOOR_SET_ID);                            
          LZM_SetAlarm(TEST_TIMER, SECS(1));
        }
        else
        {
          LcdShowCaption("改更失败", 23);  
          
          LZM_SetAlarm(TEST_TIMER, SECS(1));
        }
      }
      else
      {
        LZM_SetAlarm(TEST_TIMER, SECS(0.5));
      }
      s_stTaskAttrib.ControlId=0xff; 
      break;                                
    default:                    
      SpinResponse(&s_stSpin[0],TaskPara);  
      break;
    }
    return 1;
  case TEST_TIMER:
    LZM_StartNewTask(TaskMenuSystemParameter);//系统管理
    break;
  case INT_RTC:
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}

/*************************************************************
** 函数名称: TaskSystemCarDoorSet
** 功能描述: "车门有效选择"
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskSystemParamHornLevelSelect(LZM_MESSAGE *Msg)
{    
  unsigned char handle;
  switch(Msg->message)
  {
  case TASK_START:
    LcdClearScreen();           
    LcdShowCaption("车喇叭电平选择",0);
    LcdShowHorLine(0, 131, 18, 1);
    LcdShowHorLine(0, 131, 45, 1);
    LcdShowCaption("按<确认>保存",47);
    SpinInit(&s_stSpin[0],24,24,(char **)c_apcCarHornSetItems,"");
    EepromPram_ReadPram(E2_HORN_SET_ID, &handle);            
    s_stTaskAttrib.select=handle;
    if(handle>1)handle=1;
    s_stSpin[0].handle=handle;
    SpinShow(&s_stSpin[0]);
    SpinSetFocus(&s_stSpin[0]);
    s_stTaskAttrib.ControlId=0;
    return 1;
  case KEY_PRESS:
    if(s_stTaskAttrib.ControlId==0xff)
    {
      LZM_StartNewTask(TaskMenuSystemParameter);//系统管理
      break;
    }
    switch(TaskPara)
    {
    case KEY_BACK://返回
      LZM_StartNewTask(TaskMenuSystemParameter);//系统管理
      break;
    case KEY_ENTER://确定
      handle=SpinGetHandle(&s_stSpin[0]);
      if(handle!=s_stTaskAttrib.select)
      {
        s_stTaskAttrib.select=handle;
        LcdClearArea(0,19,131,44);
        LcdShowCaption("正在保存..", 23);                        
        EepromPram_WritePram(E2_HORN_SET_ID, &handle, 1);  
        EepromPram_ReadPram(E2_HORN_SET_ID, &handle);
        LcdClearArea(0,19,131,44);
        if(handle==s_stTaskAttrib.select)
        {
          LcdShowCaption("改更成功", 23); 
          EepromPram_UpdateVariable(E2_HORN_SET_ID);                            
          LZM_SetAlarm(TEST_TIMER, SECS(1));
        }
        else
        {
          LcdShowCaption("改更失败", 23);
          LZM_SetAlarm(TEST_TIMER, SECS(1));
        }
      }
      else
      {
        LZM_SetAlarm(TEST_TIMER, SECS(0.5));
      }
      s_stTaskAttrib.ControlId=0xff; 
      break;                                
    default:
      SpinResponse(&s_stSpin[0],TaskPara);  
      break;
    }
    return 1;
  case TEST_TIMER:
    LZM_StartNewTask(TaskMenuSystemParameter);//系统管理
    break;
  case INT_RTC:
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
/*************************************************************
** 函数名称: TaskSystemCDMASleep
** 功能描述: CDMA休眠开关
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskSystemParamSetCDMASleep(LZM_MESSAGE *Msg)
{    
  unsigned char handle;
  switch(Msg->message)
  {
  case TASK_START:
    LcdClearScreen();           
    LcdShowCaption("CDMA休眠开关",0);
    LcdShowHorLine(0, 131, 18, 1);
    LcdShowHorLine(0, 131, 45, 1);
    LcdShowCaption("按<确认>保存",47);
    LcdShowStr(8,24,"休眠:",0);
    SpinInit(&s_stSpin[0],64,24,(char **)c_apcCDMASleepItems,"");
    EepromPram_ReadPram(E2_CDMA_SLEEP_ID, &handle);            
    s_stTaskAttrib.select=handle;
    if(handle!=1)handle=0;
    s_stSpin[0].handle=handle;
    SpinShow(&s_stSpin[0]);
    SpinSetFocus(&s_stSpin[0]);
    s_stTaskAttrib.ControlId=0;
    return 1;
  case KEY_PRESS:
    if(s_stTaskAttrib.ControlId==0xff)
    {
      LZM_StartNewTask(TaskMenuSystemParameter);//系统管理
      break;
    }
    switch(TaskPara)
    {
    case KEY_BACK://返回
      LZM_StartNewTask(TaskMenuSystemParameter);//系统管理
      break;
    case KEY_ENTER://确定
      handle=SpinGetHandle(&s_stSpin[0]);
      if(handle!=s_stTaskAttrib.select)
      {
        s_stTaskAttrib.select = handle;
        LcdClearArea(0,19,131,44);
        LcdShowCaption("正在保存..", 23);                        
        EepromPram_WritePram(E2_CDMA_SLEEP_ID, &handle, 1);
        EepromPram_ReadPram(E2_CDMA_SLEEP_ID, &handle);
        LcdClearArea(0,19,131,44);
        if(handle==s_stTaskAttrib.select)
        {
          LcdShowCaption("改更成功", 23); 
          EepromPram_UpdateVariable(E2_CDMA_SLEEP_ID);                            
          LZM_SetAlarm(TEST_TIMER, SECS(1));
        }
        else
        {
          LcdShowCaption("改更失败", 23);  
          
          LZM_SetAlarm(TEST_TIMER, SECS(1));
        }
      }
      else
      {
        LZM_SetAlarm(TEST_TIMER, SECS(0.5));
      }
      s_stTaskAttrib.ControlId=0xff; 
      break;                                
    default:                    
      SpinResponse(&s_stSpin[0],TaskPara);  
      break;
    }
    return 1;
  case TEST_TIMER:
    LZM_StartNewTask(TaskMenuSystemParameter);//系统管理
    break;
  case INT_RTC:
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}

/*************************************************************
** 函数名称: TaskSystemDebugShowIOstatus
** 功能描述: "输入状态显示"菜单
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskSystemShowTerminalID(LZM_MESSAGE *Msg)
{
  unsigned char buff[20],buffer[10];
  unsigned char len,i,temp;
  switch(Msg->message)
  {
  case TASK_START:
    LcdClearScreen();            
    LcdShowCaption("终端ID号:",2);
    len=EepromPram_ReadPram(E2_DEVICE_ID, buffer);
    ////////////////
    temp=Public_BCD2HEX(buffer[0]);
    sprintf((char *)&buff[0],"%d",temp);
    for(i=1; i<len; i++)
    {
      temp=Public_BCD2HEX(buffer[i]);
      sprintf((char *)&buff[2*i-1],"%02d",temp);
    }
    LcdShowCaption((char *)buff,22);
    LcdShowHorLine(0, 131, 43, 1);
    LcdShowCaption("任意键返回",47);
    return 1;
  case KEY_PRESS:
    switch(TaskPara)
    {
    case KEY_BACK://返回
    case KEY_ENTER://确定                               
    case KEY_UP:
    case KEY_DOWN:
      LZM_StartNewTask(TaskMenuSystemParameter);//系统管理
      break;
    }
    return 1;
  case TEST_TIMER:
    break;
  case INT_RTC:
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
/*************************************************************
** 函数名称: TaskSystemParamSetGNSSWordMode
** 功能描述: 设置北斗模块工作模式
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskSystemParamSetGNSSWordMode(LZM_MESSAGE *Msg)
{    
  unsigned char handle;
  unsigned char i,index;
  switch(Msg->message)
  {
  case TASK_START:
    LcdClearScreen();  
    index =0;
    if(!EepromPram_ReadPram(E2_GPS_SET_MODE_ID,&index))
    {
      PUBLIC_SETBIT(index,2);
    }
    LcdShowCaption("GNSS定位模式",0);
    LcdShowHorLine(0, 131, 18, 1);
    s_stTaskAttrib.ControlId = 0;
    s_stTaskAttrib.ControlMaxId =4;
    for(i=0; i<2; i++)
    {
      LcdShowStr(0, 22+i*22, c_apcGNSSWorkModeShow[i], 0);
      ////////////////////////
      SpinInit(&s_stSpin[i],80,22+i*22,(char **)c_apcGNSSworkModeItems,"");             
      //////////////////////////
      SpinInit(&s_stSpin[i+2],80,22+i*22,(char **)c_apcGNSSworkModeItems,"");
      /////////////////////
      s_stSpin[i].keyflag = 1;
      s_stSpin[i].handle = PUBLIC_CHECKBIT_EX(index,i);
      s_stSpin[i+2].keyflag = 1;
      s_stSpin[i+2].handle =PUBLIC_CHECKBIT_EX(index,i+2);
      ///////////////////////////
      SpinShow(&s_stSpin[i]);
    }
    /////////////////////////////////////////
    SpinSetFocus(&s_stSpin[s_stTaskAttrib.ControlId]);
    return 1;
  case KEY_PRESS:
    switch(TaskPara)
    {
    case KEY_BACK://返回
      LZM_StartNewTask(TaskMenuSystemParameter);//
      break;
    case KEY_ENTER://确定
      LcdClearArea(0,19,131,62);
      LcdShowCaption("正在保存", 26); 
      index = 0;
      for(i=0; i<s_stTaskAttrib.ControlMaxId; i++)
      {
        handle=SpinGetHandle(&s_stSpin[i]);
        if(handle)
          PUBLIC_SETBIT(index, i);
      }
      EepromPram_WritePram(E2_GPS_SET_MODE_ID,&index,1);
      handle = 0;
      EepromPram_ReadPram(E2_GPS_SET_MODE_ID,&handle);
      if(index==handle)
      {
        LcdShowCaption("保存成功", 26); 
        EepromPram_UpdateVariable(E2_GPS_SET_MODE_ID);
      }
      else
      {
        LcdShowCaption("保存失败", 26);
      }
      LZM_SetAlarm(TEST_TIMER, SECS(2));
      break;                                
    default:                    
      i=SpinResponse(&s_stSpin[s_stTaskAttrib.ControlId],TaskPara); 
      if(i)
      {
        SpinLoseFocus(&s_stSpin[s_stTaskAttrib.ControlId]);
        s_stTaskAttrib.ControlId++;
        if(s_stTaskAttrib.ControlId>=s_stTaskAttrib.ControlMaxId) s_stTaskAttrib.ControlId =0;
        //////////////////////////////
        if(2==s_stTaskAttrib.ControlId||0==s_stTaskAttrib.ControlId)
        {
          for(i=0; i<2; i++)
          {
            LcdShowStr(0, 22+i*22, c_apcGNSSWorkModeShow[s_stTaskAttrib.ControlId+i], 0);
            ///////////////////////////
            SpinShow(&s_stSpin[s_stTaskAttrib.ControlId+i]);
          }
        }
        //////////////////////////////
        SpinSetFocus(&s_stSpin[s_stTaskAttrib.ControlId]);
      }
      
      break;
    }
    return 1;
  case TEST_TIMER:
    LZM_StartNewTask(TaskMenuSystemParameter);//
    break;
  case INT_RTC:
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
/*************************************************************
** 函数名称: TaskMenuSystemAdvanced
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskMenuSystemCarInfo(LZM_MESSAGE *Msg)
{
  switch(Msg->message)
  {
  case TASK_START:
    MenuInit(&s_sutMenuSystemCarInfo,s_apcMenuSystemCarInfo);
    MenuShow(&s_sutMenuSystemCarInfo);
    return 1;
  case KEY_PRESS:
    switch(TaskPara)
    {                
    case KEY_BACK:
      LZM_StartNewTask(TaskMenuSystemAdvanced);
      break;
    case KEY_ENTER:
      switch(MenuGetHandle(&s_sutMenuSystemCarInfo))
      {
      case MENU_CAR_SET_PLATE_NUM:
        LZM_StartNewTask(TaskSystemSetCarPlateNumber);
        break;
      case MENU_CAR_SET_VIN_NUM:
        LZM_StartNewTask(TaskSystemSetCarVinNumber);
        break;
      case MENU_CAR_SET_PLATE_COLOR:
        LZM_StartNewTask(TaskSystemSetCarPlateColor);
        break;
      case MENU_CAR_SET_PLATE_TYPE:
        LZM_StartNewTask(TaskSystemSetCarPlateType);
        break;
      case MENU_CAR_SET_TOTAL_MILE:
        LZM_StartNewTask(TaskSystemSetCarTotalMile);
        break;  
      case MENU_CAR_SET_FEATUER_COEF:
        LZM_StartNewTask(TaskSystemSetCarFeatureCoef);
        break;
      case MENU_CAR_SET_PROVINCE_ID:
        LZM_StartNewTask(TaskSystemSetCarProvinceID);
        break;
      case MENU_CAR_SET_CITY_ID:
        LZM_StartNewTask(TaskSystemSetCarCityID);
        break;
      case MENU_CAR_SET_OWNER_PHONE:
        LZM_StartNewTask(TaskSystemSetCarOwnerPhone);
        break;
      case MENU_CAR_SET_ENGINE_NUM:
        LZM_StartNewTask(TaskSystemSetCarEngineNum);
        break;   
      default:
        break;					
      }
      break;
    default:
      MenuResponse(&s_sutMenuSystemCarInfo,TaskPara);
      break;
    }
    return 1;
  default:
    return LZM_DefTaskProc(Msg);
  } 
}
/*************************************************************
** 函数名称: TaskMenuSystemServer
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskMenuSystemManageServer(LZM_MESSAGE *Msg)
{
  switch(Msg->message)
  {
  case TASK_START:
    MenuInit(&s_sutMenuSysManageServer,s_apcSysManageServer);
    MenuShow(&s_sutMenuSysManageServer);
    return 1;
  case KEY_PRESS:
    switch(TaskPara)
    {                
    case KEY_BACK:
      LZM_StartNewTask(TaskMenuSystemAdvanced);
      break;
    case KEY_ENTER:
      switch(MenuGetHandle(&s_sutMenuSysManageServer))
      {
      case MENU_SYS_MANAGE_SERVER_QUERY:
        LZM_StartNewTask(TaskSystemQueryServerParameter);
        break;
      case MENU_SYS_MANAGE_SERVER_SET:
        PublicGetPasswordTo(_TASK(TaskMenuSystemServer),123456);
        break;
      }
      break;
    default:
      MenuResponse(&s_sutMenuSysManageServer,TaskPara);
      break;
    }
    return 1;
  default:
    return LZM_DefTaskProc(Msg);
  } 
}
/*************************************************************
** 函数名称: TaskMenuSystemServer
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskMenuSystemServer(LZM_MESSAGE *Msg)
{
  switch(Msg->message)
  {
  case TASK_START:
    MenuInit(&s_sutMenuSystemServer,s_apcMenuSystemServer);
    MenuShow(&s_sutMenuSystemServer);
    return 1;
  case KEY_PRESS:
    switch(TaskPara)
    {                
    case KEY_BACK:
      LZM_StartNewTask(TaskMenuSystemManageServer);
      break;
    case KEY_ENTER:
      switch(MenuGetHandle(&s_sutMenuSystemServer))
      {
      case MENU_SERVER_SET_IP:
        LZM_StartNewTask(TaskSystemServerSetIP);
        break;
      case MENU_SERVER_SET_BACKUP_IP:
        LZM_StartNewTask(TaskSystemServerSetBackUpIP);
        break;
      case MENU_SERVER_SET_APN:
        LZM_StartNewTask(TaskSystemServerSetAPN);
        break;                        
      case MENU_SERVER_SET_TCP:
        LZM_StartNewTask(TaskSystemServerSetTCPPort);
        break;
      case MENU_SERVER_SET_UDP:
        LZM_StartNewTask(TaskSystemServerSetUDPPort);
        break;
      case MENU_SERVER_MAIN_DNS:
        LZM_StartNewTask(TaskSystemServerSetMainDomainName);
        break;
      case MENU_SERVER_BAK_DNS:
        LZM_StartNewTask(TaskSystemServerSetBackUpDomainName);
        break;
      case MENU_SERVER_SET_IP2:
        LZM_StartNewTask(TaskSystemServerSetIP2);
        break;
      case MENU_SERVER_SET_BACKUP_IP2:
        LZM_StartNewTask(TaskSystemServerSetBackUpIP2);
        break;                      
      case MENU_SERVER_SET_TCP2:
        LZM_StartNewTask(TaskSystemServerSetTCPPort2);
        break;
      case MENU_SERVER_SET_UDP2:
        LZM_StartNewTask(TaskSystemServerSetUDPPort2);
        break;
      default:
        break;					
      }
      break;
    default:
      MenuResponse(&s_sutMenuSystemServer,TaskPara);
      break;
    }
    return 1;
  default:
    return LZM_DefTaskProc(Msg);
  } 
}
/*************************************************************
** 函数名称: TaskMenuSystemDebug
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskMenuSystemDebug(LZM_MESSAGE *Msg)
{
  switch(Msg->message)
  {
  case TASK_START:
    MenuInit(&s_sutMenuSystemDebug,s_apcMenuSystemDebug);
    MenuShow(&s_sutMenuSystemDebug);   
    s_stTaskAttrib.ControlId =0;
    return 1;
  case KEY_PRESS:
    if(s_stTaskAttrib.ControlId==0xff)break;
    switch(TaskPara)
    {                
    case KEY_BACK:
      LZM_StartNewTask(TaskMenuSystemAdvanced);
      break;
    case KEY_ENTER:
      switch(MenuGetHandle(&s_sutMenuSystemDebug))
      {
      case MENU_DEBUG_IO_IN:
        LZM_StartNewTask(TaskSystemDebugShowIOstatus);
        break;
      case MENU_DEBUG_IO_OUT:
        LZM_StartNewTask(TaskSystemDebugIOOut);
        break;
      case MENU_DEBUG_REPORT_MODE:
        LZM_StartNewTask(TaskSystemDebugCurrentReportMode);
        break;
      case MENU_DEBUG_CALI_FEATUER_COEF:
        CalibrationFeatureCoef(); 
        break;
      case MENU_DEBUG_FIX_TIME:
        LZM_StartNewTask(TaskSystemDebugFixTime);
        break;  
      case MENU_DEBUG_POSITION:
        PublicConfirm("侧翻角度校准?",SystemDebugSetPosition);
        break; 
      case MENU_DEBUG_LOGOUT:
        PublicConfirm("需要注销终端?",CurrentSystemLogout);
        break;
      case MENU_DEBUG_PASSWORD:
        LZM_StartNewTask(TaskSystemDebugSetPassWord);
        break;
      case MENU_DEBUG_RESET:
        PublicConfirm("是否重启终端?",NVIC_SystemReset);
        break;
      case MENU_DEBUG_TEST:
        LZM_StartNewTask(TaskMenuSystemDebugTest);//
        break;
      default:
        break;					
      }
      break;
    default:
      MenuResponse(&s_sutMenuSystemDebug,TaskPara);
      break;
    }
    return 1;
  case SHOW_TIMER:
    s_stTaskAttrib.ControlId=0;
    MenuShow(&s_sutMenuSystemDebug);
    break;
  default:
    return LZM_DefTaskProc(Msg);
  } 
  return 1;
}
/*************************************************************
** 函数名称: TaskMenuSystemParameter
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskMenuSystemParameter(LZM_MESSAGE *Msg)
{
  //unsigned char val;
  //unsigned char buf[13];
  switch(Msg->message)
  {
  case TASK_START:
    MenuInit(&s_sutMenuSystemParameter,s_apcMenuSystemParameter);
    MenuShow(&s_sutMenuSystemParameter);   
    s_stTaskAttrib.ControlId =0;
    return 1;
  case KEY_PRESS:
    if(s_stTaskAttrib.ControlId==0xff)break;
    switch(TaskPara)
    {                
    case KEY_BACK:
      LZM_StartNewTask(TaskMenuSystemAdvanced);
      break;
    case KEY_ENTER:
      switch(MenuGetHandle(&s_sutMenuSystemParameter))
      {
        
      case MENU_PARAM_PHONE_NUMBER:
        //val=ReadAndCheckPhoneNumber(buf);
        //if(val==1)
        //{
        LZM_StartNewTask(TaskSystemParamSetPhoneNumber);
        //}
        //else
        //{
        //    LZM_StartNewTask(TaskSystemParamSetDeviceID);
        //}
        break;                        
      case MENU_PARAM_SPEED_PARAM:
        LZM_StartNewTask(TaskSystemParamSetSpeedParam);
        break;
      case MENU_PARAM_DOOR_LEVEL_SEL:
        LZM_StartNewTask(TaskSystemParamDoorLevelSelect);
        break;
      case MENU_PARAM_HORN_LEVEL_SEL:
        LZM_StartNewTask(TaskSystemParamHornLevelSelect);
        break;
      case MENU_PARAM_CDMA_SLEEP:
        LZM_StartNewTask(TaskSystemParamSetCDMASleep);
        break;
      case MENU_PARAM_TERMINAl_ID:
        LZM_StartNewTask(TaskSystemShowTerminalID);
        break;
      case MENU_PARAM_DEFAULT_PARAM:
        PublicConfirm("恢复出厂设置?",LoadDefaultParamter);
        break;
      case MENU_PARAM_GNSS_WORD_MODE:
        LZM_StartNewTask(TaskSystemParamSetGNSSWordMode);
        break;
      default:
        break;					
      }
      break;
    default:
      MenuResponse(&s_sutMenuSystemParameter,TaskPara);
      break;
    }
    return 1;
  default:
    return LZM_DefTaskProc(Msg);
  } 
  return 1;
}
/*************************************************************
** 函数名称: TaskMenuBeforeLockTwo
** 功能描述: "使用前锁定一"
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskMenuBeforeLockOne(LZM_MESSAGE *Msg)
{
  switch(Msg->message)
  {
  case TASK_START:
    LcdClearScreen();
    LcdShowCaption("使用前锁定一",0);
    LcdShowHorLine(0, 131, 18, 1);
    LcdShowCaption("是否确定锁定?",24);
    LcdShowHorLine(0, 131, 45, 1);
    LcdShowCaption("按<确认>锁定",47);
    s_stTaskAttrib.step = 0;
    s_stTaskAttrib.select = 0;
    s_stTaskAttrib.ControlId = 0;
    s_stTaskAttrib.ControlMaxId =0;
    return 1;
  case KEY_PRESS:            
    switch(TaskPara)
    {
    case KEY_BACK://返回                    
      s_stTaskAttrib.step =0;
      s_stTaskAttrib.answerNum = 0;
      s_stTaskAttrib.ControlMaxId =0;
      if(s_stTaskAttrib.ControlId==0xff)break;
      LZM_StartNewTask(TaskMenuSystemAdvanced);//
      break;
    case KEY_ENTER://确定
      s_stTaskAttrib.step =0;
      s_stTaskAttrib.answerNum = 0;
      s_stTaskAttrib.ControlMaxId =0;
      if(s_stTaskAttrib.ControlId==0xff)break;
      s_stTaskAttrib.ControlId=0xff;
      SysAutoRunCountReset();
      LcdClearScreen();
      LcdShowCaption("终端锁定成功",24);
      SetTimerTask(TIME_CLOSE_GPRS,SECOND);                    
      //Communication_Close();
      LZM_SetAlarm(TEST_TIMER, PUBLICSECS(10));
      s_stTaskAttrib.select = 0;
      break;
    case KEY_UP: 
      if(s_stTaskAttrib.ControlId!=0xff)break;
      if(s_stTaskAttrib.step<4)
      {
        s_stTaskAttrib.step++;                        
        s_stTaskAttrib.answerNum = 0;
        s_stTaskAttrib.ControlMaxId = 0;
      }
      else                    
      {
        if(4==s_stTaskAttrib.ControlMaxId)
        {
          if(s_stTaskAttrib.answerNum <4)
          {
            s_stTaskAttrib.answerNum++;
          }
          else
          {
            s_stTaskAttrib.step = 0;
            s_stTaskAttrib.select = 0;                                
            s_stTaskAttrib.ControlId = 0;
            s_stTaskAttrib.ControlMaxId =0;
            Communication_Open();
            LZM_StartNewTask(TaskMenuSystemAdvanced);//
          }
        }
        else
          if(s_stTaskAttrib.ControlMaxId)
          {
            s_stTaskAttrib.step = 0;
            s_stTaskAttrib.ControlMaxId = 0;
          }
      }                    
      break;
    case KEY_DOWN:
      if(s_stTaskAttrib.ControlId!=0xff)break;
      if(4==s_stTaskAttrib.step)
      {
        s_stTaskAttrib.answerNum = 0;
        if(s_stTaskAttrib.ControlMaxId<4)
        {
          s_stTaskAttrib.ControlMaxId++;
        }
      }
      else
      {
        s_stTaskAttrib.step =0;
        s_stTaskAttrib.answerNum = 0;
        s_stTaskAttrib.ControlMaxId =0;
      }
      break;
    default: 
      break;
    }
    return 1;
  case TEST_TIMER:
    SysAutoRunCountReset();
    if(s_stTaskAttrib.select<30)//恢复正常的次数300s=30*10s;
    {
      s_stTaskAttrib.select++;
      LZM_SetAlarm(TEST_TIMER, PUBLICSECS(10));
    }
    else
    {
      s_stTaskAttrib.select = 0;
      Communication_Open();
      LZM_StartNewTask(TaskMenuSystemAdvanced);//
    }
    break;
  default:
    if(s_stTaskAttrib.ControlId==0xff)break;
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
/*************************************************************
** 函数名称: TaskMenuBeforeLockTwo
** 功能描述: "使用前锁定一"
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskMenuBeforeLockTwo(LZM_MESSAGE *Msg)
{
  switch(Msg->message)
  {
  case TASK_START:
    LcdClearScreen();
    LcdShowCaption("使用前锁定二",0);
    s_stTaskAttrib.step = 0;
    s_stTaskAttrib.select = 0;
    s_stTaskAttrib.ControlId = 0;
    s_stTaskAttrib.ControlMaxId =0;
    if(OpenAccount_GetCurLockStatus())
    {
      SysAutoRunCountReset(); 
      s_stTaskAttrib.ControlId = 0xff; 
      LcdShowCaption("终端已锁定",26); 
      SetTimerTask(TIME_CLOSE_GPRS,SECOND);                 
      LZM_SetAlarm(TEST_TIMER, PUBLICSECS(10));
    }
    else
    {            
      LcdShowHorLine(0, 131, 18, 1);
      LcdShowCaption("是否确定锁定?",24);
      LcdShowHorLine(0, 131, 45, 1);
      LcdShowCaption("按<确认>锁定",47);
    }
    return 1;
  case KEY_PRESS:            
    switch(TaskPara)
    {
    case KEY_BACK://返回                    
      s_stTaskAttrib.step =0;
      s_stTaskAttrib.answerNum = 0;
      s_stTaskAttrib.ControlMaxId =0;
      if(s_stTaskAttrib.ControlId==0xff)break;
      LZM_StartNewTask(TaskMenuSystemAdvanced);//
      break;
    case KEY_ENTER://确定
      s_stTaskAttrib.step =0;
      s_stTaskAttrib.answerNum = 0;
      s_stTaskAttrib.ControlMaxId =0;
      if(s_stTaskAttrib.ControlId==0xff)break;
      s_stTaskAttrib.ControlId=0xff;
      SysAutoRunCountReset();
      LcdClearScreen();
      LcdShowCaption("终端锁定成功",24);
      SetTimerTask(TIME_CLOSE_GPRS,SECOND);
      /////////////////////
#ifdef USE_QUICK_OPEN_ACCOUNT
      OpenAccount_SetLockStatus();
#endif
      s_stTaskAttrib.select = 0;
      /////////////////////                    
      LZM_SetAlarm(TEST_TIMER, PUBLICSECS(10));
      break;
    case KEY_UP: 
      if(s_stTaskAttrib.ControlId!=0xff)break;
      if(s_stTaskAttrib.step<4)
      {
        s_stTaskAttrib.step++;                        
        s_stTaskAttrib.answerNum = 0;
        s_stTaskAttrib.ControlMaxId = 0;
      }
      else                    
      {
        if(4==s_stTaskAttrib.ControlMaxId)
        {
          if(s_stTaskAttrib.answerNum <4)
          {
            s_stTaskAttrib.answerNum++;
          }
          else
          {
            s_stTaskAttrib.step = 0;
            s_stTaskAttrib.select = 0;
            s_stTaskAttrib.ControlId = 0;
            s_stTaskAttrib.ControlMaxId =0;
            OpenAccount_ClearLockStatus();
            Communication_Open();
            LZM_StartNewTask(TaskMenuSystemAdvanced);//
          }
        }
        else
          if(s_stTaskAttrib.ControlMaxId)
          {
            s_stTaskAttrib.step = 0;
            s_stTaskAttrib.ControlMaxId = 0;
          }
      }                    
      break;
    case KEY_DOWN:
      if(s_stTaskAttrib.ControlId!=0xff)break;
      if(4==s_stTaskAttrib.step)
      {
        s_stTaskAttrib.answerNum = 0;
        if(s_stTaskAttrib.ControlMaxId<4)
        {
          s_stTaskAttrib.ControlMaxId++;
        }
      }
      else
      {
        s_stTaskAttrib.step =0;
        s_stTaskAttrib.answerNum = 0;
        s_stTaskAttrib.ControlMaxId =0;
      }
      break;
    default: 
      break;
    }
    return 1;
  case TEST_TIMER:
    SysAutoRunCountReset();
    if(s_stTaskAttrib.select==2)
    {
#ifdef USE_NAVILCD
      if(!NaviLcd_GetCurOnlineStatus())
#endif
        OpenAccount_CheckAndDisposeLockStatus();
    }
    //////////////////////////
    if(s_stTaskAttrib.select<60)//恢复正常的次数600s=60*10s;
    {
      s_stTaskAttrib.select++;
      LZM_SetAlarm(TEST_TIMER, PUBLICSECS(10));
    }
    else
    {
      s_stTaskAttrib.select = 0;
      Communication_Open();
      OpenAccount_ClearLockStatus();
      LZM_StartNewTask(TaskMenuSystemAdvanced);//
    }
    break;
  default:
    if(s_stTaskAttrib.ControlId==0xff)break;
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
/*************************************************************
** 函数名称: TaskSystemOpenAccountEncrypt
** 功能描述: 加密
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskSystemOpenAccountEncrypt(LZM_MESSAGE *Msg)
{    
  unsigned char handle;
  switch(Msg->message)
  {
  case TASK_START:
    LcdClearScreen();           
    LcdShowCaption("开户是否加密",0);
    LcdShowHorLine(0, 131, 18, 1);
    LcdShowHorLine(0, 131, 45, 1);
    LcdShowCaption("按<确认>保存",47);
    SpinInit(&s_stSpin[0],30,24,(char **)c_apcEncryptItems,"");
    EepromPram_ReadPram(E2_TOPLIGHT_CONTROL_ID, &handle);            
    s_stTaskAttrib.select=handle;
    if(handle!=1)handle=0;
    s_stSpin[0].handle=handle;
    SpinShow(&s_stSpin[0]);
    SpinSetFocus(&s_stSpin[0]);
    s_stTaskAttrib.ControlId=0;
    return 1;
  case KEY_PRESS:
    if(s_stTaskAttrib.ControlId==0xff)
    {
      LZM_StartNewTask(TaskMenuSystemAdvanced);//系统管理
      break;
    }
    switch(TaskPara)
    {
    case KEY_BACK://返回
      LZM_StartNewTask(TaskMenuSystemAdvanced);//系统管理
      break;
    case KEY_ENTER://确定
      handle=SpinGetHandle(&s_stSpin[0]);
      if(handle!=s_stTaskAttrib.select)
      {
        s_stTaskAttrib.select = handle;
        LcdClearArea(0,19,131,44);
        LcdShowCaption("正在保存..", 23);                        
        EepromPram_WritePram(E2_TOPLIGHT_CONTROL_ID, &handle, 1);
        EepromPram_ReadPram(E2_TOPLIGHT_CONTROL_ID, &handle);
        LcdClearArea(0,19,131,44);
        if(handle==s_stTaskAttrib.select)
        {
          LcdShowCaption("改更成功", 23); 
          EepromPram_UpdateVariable(E2_CDMA_SLEEP_ID);                            
          LZM_SetAlarm(TEST_TIMER, SECS(1));
        }
        else
        {
          LcdShowCaption("改更失败", 23);  
          
          LZM_SetAlarm(TEST_TIMER, SECS(1));
        }
      }
      else
      {
        LZM_SetAlarm(TEST_TIMER, SECS(0.5));
      }
      s_stTaskAttrib.ControlId=0xff; 
      break;                                
    default:                    
      SpinResponse(&s_stSpin[0],TaskPara);  
      break;
    }
    return 1;
  case TEST_TIMER:
    LZM_StartNewTask(TaskMenuSystemAdvanced);//系统管理
    break;
  case INT_RTC:
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
/*************************************************************
** 函数名称: TaskMenuSystemAdvanced
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskMenuSystemAdvanced(LZM_MESSAGE *Msg)
{
  switch(Msg->message)
  {
  case TASK_START:
    MenuInit(&s_sutMenuSystemAdvanced,s_apcMenuSystemAdvanced);
    MenuShow(&s_sutMenuSystemAdvanced);
    return 1;
  case KEY_PRESS:
    switch(TaskPara)
    {                
    case KEY_BACK:
      LZM_StartNewTask(TaskMenuSystem);
      break;
    case KEY_ENTER:
      switch(MenuGetHandle(&s_sutMenuSystemAdvanced))
      {
      case MENU_SYS_CAR_INFO:
        LZM_StartNewTask(TaskMenuSystemCarInfo);
        break;
      case MENU_SYS_SERVER_INFO:
        LZM_StartNewTask(TaskMenuSystemManageServer);
        break;
      case MENU_SYS_DEBUG:
        LZM_StartNewTask(TaskMenuSystemDebug);
        break;
      case MENU_SYS_TERMINAL_PARAM:
        LZM_StartNewTask(TaskMenuSystemParameter);
        break;
      case MENU_SYS_ENCRYPT:
        LZM_StartNewTask(TaskSystemOpenAccountEncrypt);
        break;
      case MENU_SYS_LOCK_ONE:
        LZM_StartNewTask(TaskMenuBeforeLockOne);
        break;
      case MENU_SYS_LOCK_TWO:
        LZM_StartNewTask(TaskMenuBeforeLockTwo);
        break;
#ifdef CALIB_OIL_WEAR
      case MENU_SYS_OIL_DEMARCATE:
        LZM_StartNewTask(TaskCalibOilPointNumber);
        break;
#endif
      default:
        break;					
      }
      break;
    default:
      MenuResponse(&s_sutMenuSystemAdvanced,TaskPara);
      break;
    }
    return 1;
  default:
    return LZM_DefTaskProc(Msg);
  } 
}
/*************************************************************
** 函数名称: OpenFeatureCoefFlag
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void OpenFeatureCoefFlag(void)
{
  setAdjustFunction(ENABLE);
  s_stTaskAttrib.ControlId=0xff;
  LcdClearScreen();
  LcdShowCaption("脉冲系统已开启", 23);
  //TestBeep(1);
  LZM_SetAlarm(TEST_TIMER, SECS(1.5));
}
/*************************************************************
** 函数名称: OpenFeatureCoefFlag
** 功能描述: 1：正校准中
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned char GetFeatureCoefFlag(void)
{
  return ADJ_getAdjFlg();
}
/*************************************************************
** 函数名称: CalibrationFeatureCoefInit
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void CalibrationFeatureCoef(void)
{
  if(GetFeatureCoefFlag())
  {
    s_stTaskAttrib.ControlId=0xff;
    LcdShowMsgEx("校准已开启",SECS(2));
  }
  else
    PublicConfirm("开启校准脉冲系数",OpenFeatureCoefFlag);
}
/*************************************************************
** 函数名称: TaskMenuSystemAdvanced
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskMenuSystem(LZM_MESSAGE *Msg)
{
  unsigned char handle;
  switch(Msg->message)
  {
  case TASK_START:
    MenuInit(&s_sutMenuSystem,s_apcMenuSystem);
    MenuShow(&s_sutMenuSystem);    
    s_stTaskAttrib.ControlId = 0;
    s_stServerParam.brushcardFlag = 0;
    return 1;
  case KEY_PRESS:
    if(s_stTaskAttrib.ControlId==0xff)break;
    switch(TaskPara)
    {                
    case KEY_BACK:
      LZM_StartNewTask(TaskMenuMain);
      break;
    case KEY_ENTER:
      switch(MenuGetHandle(&s_sutMenuSystem))
      {
      case MENU_SYSTEM_REGISTER:
      case MENU_SYSTEM_QUALIFY:
        handle=MenuGetHandle(&s_sutMenuSystem);
        LcdClearScreen();
        Lcd_DisplaySystemTest(handle);
        LZM_SetAlarm(TEST_TIMER, SECS(0.3));
        s_stTaskAttrib.ControlId = 0xff;
        break;
      case MENU_SYSTEM_BACKLIGHT:
        LZM_StartNewTask(TaskLcdBacklight);
        break;
      case MENU_SYSTEM_ADVANCED:
        PublicGetPasswordTo(_TASK(TaskMenuSystemAdvanced),s_stTaskAttrib.SysPassword); 
        break;
      case MENU_SYSTEM_MAIN_SELECT:
        LZM_StartNewTask(TaskMainInterfaceSelection); //添加人:fanqinghai 添加时间:2015.07.06
        break;
      default:
        break;					
      }
      break;
    default:
      MenuResponse(&s_sutMenuSystem,TaskPara);
      break;
    }
    return 1;
  case TEST_TIMER:
    s_stTaskAttrib.ControlId=0;
    MenuShow(&s_sutMenuSystem);
    return 1;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 1;
}
/*************************************************************
** 函数名称: TaskMainInterfaceSelection
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
** 作者    :fanqinghai
*************************************************************/  
LZM_RET TaskMainInterfaceSelection(LZM_MESSAGE *Msg)
{
  
  switch(Msg->message)
  {
  case TASK_START:
    // LZM_KillTimer(TEST_TIMER);
    MenuInit(&s_sutMenuMainInterfaceSet,s_apcMenuMainInterfaceSet);
    MenuShow(&s_sutMenuMainInterfaceSet);
    return 1;
  case KEY_PRESS:
    switch(TaskPara)
    {
    case KEY_BACK:
      LZM_StartNewTask(TaskMenuMain);
      break;
    case KEY_ENTER: 
      MenuSwitchFlg = MenuGetHandle(&s_sutMenuMainInterfaceSet);
      FRAM_BufferWrite(FRAM_INTERFACE_SWOTCH_ADDR,&MenuSwitchFlg,FRAM_INTERFACE_SWOTCH_LEN-1);
      LZM_StartNewTask(TaskShowMainInterface);
      break;
    default:
      MenuResponse(&s_sutMenuMainInterfaceSet,TaskPara);
      break;
    }
    return 1;
  case INT_RTC:            
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
  
}

/*************************************************************
** 函数名称: TaskMenuSystemDebugTest
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskMenuSystemDebugTest(LZM_MESSAGE *Msg)
{
  unsigned char handle;
  switch(Msg->message)
  {
  case TASK_START:
    MenuInit(&s_sutMenuSystemTest,s_apcMenuSystemTest);
    MenuShow(&s_sutMenuSystemTest);
    s_stTaskAttrib.ControlId =0;
    return 1;
  case KEY_PRESS:
    if(s_stTaskAttrib.ControlId)
    {
      LZM_SetAlarm(TEST_TIMER,SECS(0.1));
      break;
    }
    switch(TaskPara)
    {
    case KEY_BACK:
      LZM_StartNewTask(TaskMenuSystemDebug);//系统管理
      break;
    case KEY_ENTER:
      handle=MenuGetHandle(&s_sutMenuSystemTest);
      s_stTaskAttrib.ControlId = 1;
      LcdClearScreen();
      Lcd_DisplaySystemTest(handle);
      LZM_SetAlarm(TEST_TIMER, SECS(0.3));
      break;
    default:
      MenuResponse(&s_sutMenuSystemTest,TaskPara);
      break;
    }
    return 1;        
  case TEST_TIMER:
    s_stTaskAttrib.ControlId = 0;
    MenuShow(&s_sutMenuSystemTest);
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
/*************************************************************
** 函数名称: TaskMenuDriveRecord
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskMenuDriveRecord(LZM_MESSAGE *Msg)
{ 
  switch(Msg->message)
  {
  case TASK_START:
    LZM_KillTimer(TEST_TIMER);
    MenuInit(&s_sutMenuDriveRecord,s_apcMenuDriveRecord);
    MenuShow(&s_sutMenuDriveRecord);
    return 1;
  case KEY_PRESS:
    switch(TaskPara)
    {
    case KEY_BACK:
      LZM_StartNewTask(TaskMenuMain);
      break;
    case KEY_ENTER:	
      switch(MenuGetHandle(&s_sutMenuDriveRecord))
      { 
      case MENUDRIVERECORD_CAR_INFO:
        LZM_StartNewTask(TaskShowCarInfo);
        break;
      case MENUDRIVERECORD_DRIVER_INFO:
        LZM_StartNewTask(TaskShowDriverInfo);
        break;
      case MENUDRIVERECORD_TIREDDRIVE_RECORD://超时驾驶记录
        LZM_StartNewTask(TaskShowTiredDriveRecord);
        break;
      case MENUDRIVERECORD_15MIN_SPEED:
        LZM_StartNewTask(TaskShow15MinSpeed);
        break;
      }
      break;
    default:
      MenuResponse(&s_sutMenuDriveRecord,TaskPara);
      break;
    }
    return 1;
  case INT_RTC:            
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
/*************************************************************
** 函数名称: TaskInfoShowCarLoadStatus
** 功能描述: 车辆载货状态
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskInfoShowCarLoadStatus(LZM_MESSAGE *Msg)
{    
  unsigned char handle;
  switch(Msg->message)
  {
  case TASK_START:
    LcdClearScreen();           
    LcdShowCaption("车辆载货状态",0);
    LcdShowHorLine(0, 131, 18, 1);
    LcdShowHorLine(0, 131, 45, 1);
    LcdShowCaption("按<确认>保存",47);
    LcdShowStr(8,24,"状态:",0);
    SpinInit(&s_stSpin[0],64,24,(char **)c_apcCarLoadStatusItems,"");
    Public_ReadDataFromFRAM(FRAM_CAR_CARRY_STATUS_ADDR, &handle, 1);          
    s_stTaskAttrib.select=handle;
    if(handle>2)handle=0;
    s_stSpin[0].handle = handle;
    SpinShow(&s_stSpin[0]);
    SpinSetFocus(&s_stSpin[0]);
    s_stTaskAttrib.ControlId=0;
    return 1;
  case KEY_PRESS:
    if(s_stTaskAttrib.ControlId==0xff)
    {
      LZM_StartNewTask(TaskMenuInfoServer);//系统管理
      break;
    }
    switch(TaskPara)
    {
    case KEY_BACK://返回
      LZM_StartNewTask(TaskMenuInfoServer);//系统管理
      break;
    case KEY_ENTER://确定
      handle=SpinGetHandle(&s_stSpin[0]);
      Public_SetCarLoadStatus(handle);
      if(handle!=s_stTaskAttrib.select)
      {
        s_stTaskAttrib.select = handle;
        LcdClearArea(0,19,131,44);
        LcdShowCaption("正在保存..", 23);                        
        Public_WriteDataToFRAM(FRAM_CAR_CARRY_STATUS_ADDR, &handle, 1);
        Public_ReadDataFromFRAM(FRAM_CAR_CARRY_STATUS_ADDR, &handle, 1);
        LcdClearArea(0,19,131,44);
        if(handle==s_stTaskAttrib.select)
        {
          LcdShowCaption("改更成功", 23);
          LZM_SetAlarm(TEST_TIMER, SECS(1));
          //////////////////////////
        }
        else
        {
          LcdShowCaption("改更失败", 23);
          LZM_SetAlarm(TEST_TIMER, SECS(1));
        }
      }
      else
      {
        LZM_SetAlarm(TEST_TIMER, SECS(0.5));
      }
      s_stTaskAttrib.ControlId=0xff; 
      break;                                
    default:                    
      SpinResponse(&s_stSpin[0],TaskPara);  
      break;
    }
    return 1;
  case TEST_TIMER:
    LZM_StartNewTask(TaskMenuInfoServer);//系统管理
    break;
  case INT_RTC:
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
/*************************************************************
** 函数名称: TaskMenuInfoServer
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskMenuInfoServer(LZM_MESSAGE *Msg)
{ 
  switch(Msg->message)
  {
  case TASK_START:
    LZM_KillTimer(TEST_TIMER);
    MenuInit(&s_sutMenuInfoServer,s_apcMenuInfoServer);
    MenuShow(&s_sutMenuInfoServer);
    return 1;
  case KEY_PRESS:
    switch(TaskPara)
    {
    case KEY_BACK:
      LZM_StartNewTask(TaskMenuMain);
      break;
    case KEY_ENTER:	
      switch(MenuGetHandle(&s_sutMenuInfoServer))
      {
      case MENU_INFO_TEXT_INFO://文本信息  
        LZM_StartNewTask(TaskShowTextInfo);
        break;
      case MENU_INFO_INFO_SERVICE://"点播信息",
        LZM_StartNewTask(TaskShowInfoService);
        break;
      case MENU_INFO_QUESTION_RESPONSE://"提问应答",
        LZM_StartNewTask(TaskShowQuestionList);
        break;
      case MENU_INFO_EVENT_REPORT: //"事件报告", 
        LZM_StartNewTask(TaskShowEventReport);//
        break;
      case MENU_INFO_RESERVE_SERVICE://"信息点播",    
        LZM_StartNewTask(TaskShowInformationDemand);
        break;
      case MENU_INFO_CAR_LOAD_STATUS:
        LZM_StartNewTask(TaskInfoShowCarLoadStatus);
        break;
      }
      break;
    default:
      MenuResponse(&s_sutMenuInfoServer,TaskPara);
      break;
    }
    return 1;
  case INT_RTC:            
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
/*************************************************************
** 函数名称: TaskMenuMain
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskMenuMain(LZM_MESSAGE *Msg)
{ 
  switch(Msg->message)
  {
  case TASK_START:
    LZM_KillTimer(TEST_TIMER);
    MenuInit(&s_sutMenuMain,s_apcMenuMain);
    MenuShow(&s_sutMenuMain);
    Lcd_SetDisplayCtrlFlag();
    return 1;
  case KEY_PRESS:
    switch(TaskPara)
    {
    case KEY_BACK:
      LZM_StartNewTask(TaskShowMainInterface);
      break;
    case KEY_ENTER:	
      switch(MenuGetHandle(&s_sutMenuMain))
      {
      case MENUMAIN_DRIVE_RECORD:
        LZM_StartNewTask(TaskMenuDriveRecord);
        break;
      case MENUMAIN_INFO_SERVER://文本信息  
        LZM_StartNewTask(TaskMenuInfoServer);
        break;
#ifdef USE_PHONE_BOOK
      case MENUMAIN_PHONE_SERVER:
        LZM_StartNewTask(TaskMenuPhone);
        break;
#endif
      case MENUMAIN_DRIVE_MILE:
        LZM_StartNewTask(TaskShowDriveMile);
        break;
      case MENUMAIN_PRODUCT_INFO:
        LZM_StartNewTask(TaskShowProductInfo);//
        break;
      case MENUMAIN_SIM_NUM:                            
        LZM_StartNewTask(TaskShowSIMCardNumber);//
        break; 
      case MENUMAIN_TTS:
        LZM_StartNewTask(TaskShowAdjustTtsVolume);
        break;
      case MENUMAIN_SYSTEM:
        LZM_StartNewTask(TaskMenuSystem);
        break;                           
      }
      break;
    default:
      MenuResponse(&s_sutMenuMain,TaskPara);
      break;
    }
    return 1;
  case INT_RTC:
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}

/*************************************************************
** 函数名称: TaskMenuPrintstart
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void TaskMenuPrintstart(void)
{
  LcdClearScreen();
  LcdShowCaption("打印任务",0);
  LcdShowHorLine(0, 131, 18, 1);
  LcdShowCaption("后台正在打印",23);
  LcdShowCaption("请稍等",46);
  print_init();
  ptr_pwr_ctrl(1);
  printer_task();
  s_stTaskAttrib.step = 0x55;
  s_stTaskAttrib.ControlId = 0xff;    
  LZM_SetAlarm(TEST_TIMER,SECS(8));    
  LZM_SetAlarm(REFRESH_TIMER,SECS(90));
}
/*************************************************************
** 函数名称: TaskMenuPrintstart
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void TaskMenuPrintend(void)
{
  print_init();
  ptr_pwr_ctrl(0);                
  Lcd_Initial();//重新初始化显示屏驱动
}
/*************************************************************
** 函数名称: TaskMenuPrint
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskMenuPrint(LZM_MESSAGE *Msg)
{
  switch(Msg->message)
  {
  case TASK_START:
    LZM_KillTimer(TEST_TIMER);
    LZM_KillTimer(REFRESH_TIMER);
    s_stTaskAttrib.ControlId = 0; 
    if(s_stTaskAttrib.step != 0xaa)
    {
      s_stTaskAttrib.step = 0;
      LZM_SetAlarm(TEST_TIMER,LZM_AT_ONCE);
      return 1;
    }
    ///////////////////////
    s_stTaskAttrib.step = 0;
    /////////////////////
    LcdClearScreen();
    if(SpeedMonitor_GetCurRunStatus())
    {
      LcdShowCaption("请停车,再打印!",23);                
      LZM_SetAlarm(TEST_TIMER, SECS(2));
    }
    else//如果主电源断电,则不打印            
      if(1 == Io_ReadAlarmBit(ALARM_BIT_POWER_SHUT))
      {
        LcdShowCaption("主电断电不能打印",23);                
        LZM_SetAlarm(TEST_TIMER, SECS(2));
      }
      else
      {
        LcdShowCaption("打印任务",0);
        LcdShowHorLine(0, 131, 18, 1);
        if(KEY_ENTER == KeyValue_Read())
        {
          s_stTaskAttrib.ControlMaxId = 0;
          s_stTaskAttrib.ControlId = 0x55;
          LcdShowCaption("松开按键后打印",23);
          LZM_SetAlarm(TEST_TIMER, SECS(0.1)); 
          
        }
        else
        {
          TaskMenuPrintstart();
        }
      }
    return 1;
  case KEY_PRESS:
    if(0xff==s_stTaskAttrib.ControlId)break;
    switch(TaskPara)
    {
    case KEY_ENTER://
      break;
    default:
      TaskMenuPrintend();
      LZM_StartNewTask(TaskShowMainInterface);//主界面
      break;
    }
    return 1;
  case TEST_TIMER:
    if(0xff==s_stTaskAttrib.ControlId)//打印时，需要再次延时，要不显示出现乱码，共用口原因
    {
      if(is_print_finished())//已完成
      {
        s_stTaskAttrib.ControlId = 1;
        LZM_SetAlarm(TEST_TIMER, SECS(5));                    
      }
      else
      {
        LZM_SetAlarm(TEST_TIMER, SECS(3)); 
      }
    }
    else
      if(0x55==s_stTaskAttrib.ControlId)//打印时，需要再次延时，要不显示出现乱码，共用口原因
      {
        if(KEY_ENTER == KeyValue_Read())
        {   
          if(s_stTaskAttrib.ControlMaxId<60)
          {
            s_stTaskAttrib.ControlMaxId++;
          }
          else
          {
            s_stTaskAttrib.ControlId    = 0;
            s_stTaskAttrib.ControlMaxId = 0;
          }
          LZM_SetAlarm(TEST_TIMER,SECS(0.1));
        }
        else
        {
          TaskMenuPrintstart();
        }
      }
      else
        if(0x01==s_stTaskAttrib.ControlId)
        {
          s_stTaskAttrib.ControlId = 0;
          TaskMenuPrintend();
          LZM_SetAlarm(TEST_TIMER,SECS(0.1));
        }
        else
        {
          LZM_StartNewTask(TaskShowMainInterface);//主界面  
        }
    break;
  case REFRESH_TIMER:
    if(0x55 == s_stTaskAttrib.step)
    {
      s_stTaskAttrib.step = 0;
      TaskMenuPrintend();
      LZM_StartNewTask(TaskShowMainInterface);//主界面
    }
    break;
  case INT_RTC:
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
#ifdef __LZMINPUTBOX_H
/*************************************************************
** 函数名称: TaskShowMainInterface
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskSystemTempTest(LZM_MESSAGE *Msg)
{
  unsigned char val;
  char buffer[20];
  switch(Msg->message)
  {
  case TASK_START:
    LcdClearScreen();
    val=EepromPram_ReadPram(E2_MAIN_SERVER_APN_ID, GB2312Buffer);            
    GB2312Buffer[val]='\0';
    LcdShowStr(2,16,"主APN:",0);
    LcdShowStr(16,48,"保存",0);
    LcdShowStr(80,48,"取消",0);
    LZM_InputBoxInit(&stInputBox[0],50,16,18,(char *)GB2312Buffer,(char **)c_apcInputParam);
    stInputBox[0].loop = 0;
    LZM_InputBoxSetFocus(&stInputBox[0]);
    s_stTaskAttrib.ControlId = 0;
    return 1;
  case KEY_PRESS:
    if(0==s_stTaskAttrib.ControlId)
    {
      val=LZM_InputBoxResponse(&stInputBox[0],TaskPara);
      if(val==2||val==3)
      {
        s_stTaskAttrib.ControlId = 1;
        LcdShowStr(16,48,"保存",1);
      }
      else
        if(val==1)
        {
          s_stTaskAttrib.ControlId = 2;
          LcdShowStr(80,48,"取消",1);
        }
        else
          if(val)
          {
            LZM_ReturnOldTaskEx();
          }
      break;
    }
    switch(TaskPara)
    {
    case KEY_BACK://菜单
      LZM_ReturnOldTaskEx();
      break;
    case KEY_UP://
      if(1==s_stTaskAttrib.ControlId)
      {
        s_stTaskAttrib.ControlId = 0;
        LcdShowStr(16,48,"保存",0);
        LZM_InputBoxSetFocusEx(&stInputBox[0]);
      }
      else
        if(2==s_stTaskAttrib.ControlId)
        {
          s_stTaskAttrib.ControlId = 1;
          LcdShowStr(16,48,"保存",1);
          LcdShowStr(80,48,"取消",0);
        }
      break;
    case KEY_DOWN://
      if(1==s_stTaskAttrib.ControlId)
      {
        s_stTaskAttrib.ControlId = 2;
        LcdShowStr(16,48,"保存",0);
        LcdShowStr(80,48,"取消",1);
      }
      else
        if(2==s_stTaskAttrib.ControlId)
        {
          s_stTaskAttrib.ControlId = 0;
          LcdShowStr(80,48,"取消",0);
          LZM_InputBoxSetFocus(&stInputBox[0]);
        }
      break;
    case KEY_ENTER://确定
      if(1==s_stTaskAttrib.ControlId)
      {
        LcdClearScreen();
        LcdShowCaption("正在保存...", 24);
        val=LZM_InputBoxGetContent(&stInputBox[0],buffer);
        EepromPram_WritePram(E2_MAIN_SERVER_APN_ID,(unsigned char *)buffer,val);
        ////////////////////////////////////
        val=EepromPram_ReadPram(E2_MAIN_SERVER_APN_ID, (unsigned char *)&s_stTaskAttrib.ShowBuffer[0]);
        LcdClearArea(0,19,131,44);
        if(strncmp(s_stTaskAttrib.ShowBuffer[0],buffer,val)==0)
        {
          LcdShowCaption("改更成功", 23);
          EepromPram_UpdateVariable(E2_MAIN_SERVER_APN_ID);                           
          LZM_SetAlarm(TEST_TIMER, SECS(1));
        }
        else
        {
          LcdShowCaption("改更失败", 23);                             
          LZM_SetAlarm(TEST_TIMER, SECS(1));
        }
      }
      else
        if(2==s_stTaskAttrib.ControlId)
        {
          LZM_ReturnOldTaskEx();
        }
      break;
    }
    return 1;
  case TEST_TIMER:
    LZM_ReturnOldTaskEx();
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
#endif
/*************************************************************
** 函数名称: ShowSelectMainInterface
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/  
void ShowSelectMainInterface(unsigned char select)
{
  switch(select)
  {
    //case MENU_INTERFACE_TEMPERATURE:
    //    LCD_TemperatureInterface();
    //    break;
    //case MENU_INTERFACE_LOAD:
    //    LCD_LoadInterface();
    //    break;
  case MENU_INTERFACE_SPEED:
    LCD_SpeedInterface();
    break;
    //case MENU_INTERFACE_DEEP:
    //    LCD_DeepInterface();
    //    break;
    //case MENU_INTERFACE_TURNSPEED:
    //    LCD_TurnSpeepInterface();
    //    break;
  default: 
    Lcd_MainInterface();
    break; 
  }
}

/*************************************************************
** 函数名称: TaskShowMainInterface
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskShowMainInterface(LZM_MESSAGE *Msg)
{
  static unsigned long sulTimeVal = 0;
  static unsigned long sulReInitTime = 0;
  static unsigned char lastMenuStatus = 0xff;
	
  switch(Msg->message)
  {
  case TASK_START:
    ptr_pwr_ctrl(0);
    s_stTaskAttrib.step = 0;
    s_stTaskAttrib.ControlId = 0;
    LZM_KillTimer(TEST_TIMER);
    LZM_SetTimer(REFRESH_TIMER,SECS(1));
    LcdClearScreen();
    Lcd_ClearDisplayCtrlFlag();
    Lcd_SetMainRedrawFlag();
    //Lcd_MainInterface(); 
    ShowSelectMainInterface(MenuSwitchFlg);
    sulTimeVal = 0;
    if(0==sulReInitTime)sulReInitTime = RTC_GetCounter();
    return 1;
  case KEY_PRESS:
    switch(TaskPara)
    {
    case KEY_BACK://菜单
      LZM_KillTimer(REFRESH_TIMER);                    
      LZM_StartNewTask(TaskMenuMain);//菜单
      break;                                
    case KEY_UP://
      sulTimeVal = 0;
      break;
    case KEY_DOWN://
      sulTimeVal = 0;                 
      break;
    case KEY_ENTER://确定
      LZM_SetAlarm(TEST_TIMER,SECS(0.2));
      if(0==sulTimeVal)
      {
        s_stTaskAttrib.step = 1;
        sulTimeVal =  Timer_Val()+SYSTICK_1SECOND*3;
      }
      else
        if(Timer_Val()>=sulTimeVal)
        {
          LZM_KillTimer(TEST_TIMER);
          LZM_KillTimer(REFRESH_TIMER);
          if(1==s_stTaskAttrib.step)s_stTaskAttrib.step = 0xaa;
          LZM_StartNewTask(TaskMenuPrint);//打印
        }
      break;
    default:
      sulTimeVal = 0;
      break;
    }
    return 1;
  case REFRESH_TIMER:
    if(RTC_GetCounter()>sulReInitTime+REINIT_LCD_TIME)//20分钟
    {
      sulReInitTime = RTC_GetCounter();
      Lcd_SetMainRedrawFlag();
      Lcd_Initial();//重新初始化显示屏驱动
    }
    ///////////////////
    MenuSwitchFlg = (Lcd_GetSpeed())?MENU_INTERFACE_SPEED:MENU_INTERFACE_CONVENTION;
    if(lastMenuStatus != MenuSwitchFlg)Lcd_SetMainRedrawFlag();
    lastMenuStatus = MenuSwitchFlg;
    ///////////////////
    if(!Lcd_GetDisplayCtrlFlag())//显示弹出内容,不显示主界面
    {
      //Lcd_MainInterface();
      ShowSelectMainInterface(MenuSwitchFlg);
    }
    else
    {
      Lcd_SetMainRedrawFlag();
    }
    break;
  case TEST_TIMER:
    sulTimeVal = 0;
    s_stTaskAttrib.step = 0;
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
/*************************************************************
** 函数名称: ShowStartLogo
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void ShowStartLogo(void)
{
  char name[20];
  GetCurentProductModel(name);
  LcdClearScreen();    
  LcdShowCaption(name, 2);
  LcdShowCaption("汽车行驶记录仪", 20);
  //LcdShowStr(9,40,ProductInfo[4],0);//显示版本号,dxl,去掉开机版本号,因为版本号后面改为自适应,G网为V1.03，C网为V1.02
  ///////////////////////
  LZM_TaskFunction = TaskShowLogo;
}
/*************************************************************
** 函数名称: ShowStartLogo
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void SaveCurSIMICcidNumberToE2Param(void)
{
  unsigned char data[20]={0}; 
  MOD_GetIccidBcdValue(data);
  EepromPram_WritePram(E2_SIM_ICCID_NUMBER_ID,data,E2_SIM_ICCID_NUMBER_ID_LEN);
}
/*************************************************************
** 函数名称: TaskShowMainInterface
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskShowLogo(LZM_MESSAGE *Msg)
{
  switch(Msg->message)
  {
  case TASK_START:
    LZM_KillTimer(REFRESH_TIMER);
    s_stTaskAttrib.ControlId = 0;
    ShowStartLogo();
    LZM_SetAlarm(REFRESH_TIMER, SECS(0.1));
    SysAutoRunCountReset();
    LZM_SetOperateTimer(SaveCurSIMICcidNumberToE2Param,PUBLICSECS(40));
    return 1;
  case KEY_PRESS:
    switch(TaskPara)
    {
    case KEY_ENTER://确定
      LZM_StartNewTask(TaskShowMainInterface);//主界面;
      break;
    case KEY_BACK://返回
      LZM_StartNewTask(TaskShowMainInterface);//主界面;
      break;                
    case KEY_UP://登录
      break;
    case KEY_DOWN://注销
      break;
    }
    return 1;
  case REFRESH_TIMER: 
    SysAutoRunCountReset();
    if(s_stTaskAttrib.ControlId < 3)
    {
      s_stTaskAttrib.ControlId++;
      LZM_SetAlarm(REFRESH_TIMER, SECS(0.1));
    }
    else
      LZM_StartNewTask(TaskShowMainInterface);//主界面;
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}

/*************************************************************
** 函数名称: TaskShowTextInfo
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskShowDisplayText(LZM_MESSAGE *Msg)
{
  static unsigned short susCurtStart = 0;
  static unsigned char LastCount = 0;
  static unsigned char flag = 0;
  switch(Msg->message)
  {
  case TASK_START:
    s_stTaskAttrib.ControlId=0;
    susCurtStart = 0;
    LastCount =0;
    flag=0;
    LZM_SetAlarm(TEST_TIMER, SECS(0.1));
    return 1;
  case KEY_PRESS:
    switch(TaskPara)
    {
    case KEY_BACK://返回
      if(LZM_PreTaskFunction!=TaskShowDisplayText&&LZM_PreTaskFunction!=TaskShowQuestionResponse)
      {
        ShowTextForMsgBusyFlag(1);
        LZM_ReturnOldTaskEx();//系统管理
      }
      else
      {
        ShowTextForMsgBusyFlag(1);
        ShowMainInterface();
      }
      break;
    case KEY_ENTER://确定
      LZM_SetAlarm(TEST_TIMER, SECS(0.1));
      break;
    case KEY_UP://--
      if(s_stTaskAttrib.ControlId>1)
      {
        s_stTaskAttrib.ControlId--;
        flag=1;
        susCurtStart=GetCurrentPageMsgDataStartAddr(s_stTaskAttrib.ControlId,(unsigned char *)s_apcShowP,0);
        LZM_SetAlarm(TEST_TIMER, SECS(0.1));
      }
      break;
    case KEY_DOWN://+
      if(s_stTaskAttrib.ControlId < s_stTaskAttrib.ControlMaxId)
      {
        LZM_SetAlarm(TEST_TIMER, SECS(0.1));
      }
      break;
    }
    return 1;
  case INT_RTC:
    break;
  case TEST_TIMER:
    if(!flag)s_stTaskAttrib.ControlId++;
    flag=0;
    if(s_stTaskAttrib.ControlId < s_stTaskAttrib.ControlMaxId)
    {
      LastCount=LcdShowOnePage((unsigned char *)s_apcShowP+susCurtStart);                
      susCurtStart += LastCount;
      LZM_SetAlarm(TEST_TIMER, SECS(15));
    }
    else
      if(s_stTaskAttrib.ControlId == s_stTaskAttrib.ControlMaxId)//显示最后那一页
      {
        SysAutoRunCountReset();
        LcdClearScreen();                
        LastCount=LcdShowStrEx(0,0,(const char *)s_apcShowP+susCurtStart,0,s_stTaskAttrib.textlen-susCurtStart);
        LZM_SetAlarm(TEST_TIMER, SECS(15));
      }
      else
        if(s_stTaskAttrib.ControlId == s_stTaskAttrib.ControlMaxId+1 && s_stTaskAttrib.ControlMaxId==1)//显示最后那一页
        {
          SysAutoRunCountReset();
          LZM_SetAlarm(TEST_TIMER, SECS(15));
        }
        else
        {
          ShowTextForMsgBusyFlag(1);///
          ShowMainInterface();
        }
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
/*************************************************************
** 函数名称: TaskShowTextInfo
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskShowQuestionResponse(LZM_MESSAGE *Msg)
{
  static unsigned short susCurtStart = 0;
  static unsigned char LastCount = 0;
  static unsigned char flag = 0;
  unsigned char x;
  u8 channel = CHANNEL_DATA_1|CHANNEL_DATA_2;
  u16 SerialNum;
  
  switch(Msg->message)
  {
  case TASK_START:
    LZM_KillTimer(REFRESH_TIMER);
    flag = 0;
    LastCount = 0;
    susCurtStart = 0;
    s_stTaskAttrib.step = 0;
    s_stTaskAttrib.select = 0;
    s_stTaskAttrib.ControlId = 0;
    LZM_SetAlarm(TEST_TIMER, SECS(0.1));
    return 1;
  case KEY_PRESS:
    switch(TaskPara)
    {
    case KEY_BACK://返回
      if(0 == s_stTaskAttrib.step)
      {
        if(LZM_PreTaskFunction!=TaskShowDisplayText&&LZM_PreTaskFunction!=TaskShowQuestionResponse)
        {
          ShowTextForMsgBusyFlag(1);
          LZM_ReturnOldTaskEx();//系统管理
        }
        else
        {
          ShowTextForMsgBusyFlag(1);
          ShowMainInterface();
        }
      }
      else
        if(1 == s_stTaskAttrib.step)
        {
          s_stTaskAttrib.step = 0;
          flag=1;
          LZM_SetAlarm(TEST_TIMER, SECS(0.1));
        }
      break;
    case KEY_ENTER://确定
      LZM_KillTimer(TEST_TIMER);
      if(0 == s_stTaskAttrib.step)
      {
        s_stTaskAttrib.step++;
        LcdClearScreen();   
        LcdShowCaption("选择答案:",2);
        LcdShowHorLine(0, 131, 20, 1);
        LcdShowStr(0,24,(const char *)s_stTaskAttrib.ShowBuffer[1],0);
        x=s_stTaskAttrib.select*2*8;
        LcdReverseBar(x,24,x+8,40);
        LcdShowHorLine(0, 131, 42, 1);
        LcdShowCaption("<确定>发送",44);
      }
      else
        if(1 == s_stTaskAttrib.step)
        {
          
          //////发送答案//////////   
          channel = RadioProtocol_GetDownloadCmdChannel(0x8302, &SerialNum);
          if(ACK_OK==RadioProtocol_QuestionAck(channel,s_stTaskAttrib.ShowBuffer[0][s_stTaskAttrib.select]))
          {
            LcdClearScreen();
            LcdShowCaption("答案发送成功",25);
            LZM_SetAlarm(REFRESH_TIMER, SECS(3));
          }
          else
          {
            Public_ShowTextInfo("答案发送失败", PUBLICSECS(3));
          }
        }
      break;
    case KEY_UP://--
      if(0 == s_stTaskAttrib.step)
      {
        if(s_stTaskAttrib.ControlId>1)
        {
          s_stTaskAttrib.ControlId--;
          flag=1;
          susCurtStart=GetCurrentPageMsgDataStartAddr(s_stTaskAttrib.ControlId,(unsigned char *)s_apcShowP,0);
          LZM_SetAlarm(TEST_TIMER, SECS(0.1));
        }
      }
      else
        if(1 == s_stTaskAttrib.step)
        {
          x=s_stTaskAttrib.select*2*8;
          LcdReverseBar(x,24,x+8,40);
          if(s_stTaskAttrib.select>0)
          {
            s_stTaskAttrib.select--;
          }
          else
          {
            s_stTaskAttrib.select=s_stTaskAttrib.answerNum-1;
          }
          x=s_stTaskAttrib.select*2*8;
          LcdReverseBar(x,24,x+8,40);
        }
      break;
    case KEY_DOWN://+
      if(0 == s_stTaskAttrib.step)
      {
        if(s_stTaskAttrib.ControlId < s_stTaskAttrib.ControlMaxId)
        {
          LZM_SetAlarm(TEST_TIMER, SECS(0.1));
        }
      }
      else
        if(1 == s_stTaskAttrib.step)
        {
          x=s_stTaskAttrib.select*2*8;
          LcdReverseBar(x,24,x+8,40);
          if(s_stTaskAttrib.select<s_stTaskAttrib.answerNum-1)
          {
            s_stTaskAttrib.select++;
          }
          else
          {
            s_stTaskAttrib.select =0;
          }
          x=s_stTaskAttrib.select*2*8;
          LcdReverseBar(x,24,x+8,40);
        }
      break;
    }
    return 1;
  case INT_RTC:
    break;
  case REFRESH_TIMER:
    if(0==s_stTaskAttrib.step)break;
    if(LZM_PreTaskFunction!=TaskShowQuestionResponse)
    {
      ShowTextForMsgBusyFlag(1);
      LZM_ReturnOldTaskEx();//系统管理
    }
    else
    {
      ShowTextForMsgBusyFlag(1);
      ShowMainInterface();
    }
    break;
  case TEST_TIMER:
    if(!flag)s_stTaskAttrib.ControlId++;
    flag=0;
    if(s_stTaskAttrib.ControlId < s_stTaskAttrib.ControlMaxId)
    {
      LastCount=LcdShowOnePage((unsigned char *)s_apcShowP+susCurtStart);                
      susCurtStart += LastCount;
      LZM_SetAlarm(TEST_TIMER, SECS(15));
    }
    else
      if(s_stTaskAttrib.ControlId == s_stTaskAttrib.ControlMaxId)//显示最后那一页
      {
        SysAutoRunCountReset();
        LcdClearScreen();                
        LastCount=LcdShowStrEx(0,0,(const char *)s_apcShowP+susCurtStart,0,s_stTaskAttrib.textlen-susCurtStart);
        LZM_SetAlarm(TEST_TIMER, SECS(15));
      }
      else
        if(s_stTaskAttrib.ControlId == s_stTaskAttrib.ControlMaxId+1 && s_stTaskAttrib.ControlMaxId==1)//显示最后那一页
        {
          SysAutoRunCountReset();
          LZM_SetAlarm(TEST_TIMER, SECS(15));
        }
        else
        {
          ShowTextForMsgBusyFlag(1);///
          ShowMainInterface();
        }
    break;
  default:
    return LZM_DefTaskProc(Msg);
  }
  return 0;
}
/*************************************************************
** 函数名称: ReadInterfaceSwitchflg
** 功能描述: 读取主界面切换标志
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
** 作者    :fanqinghai
** 日期    :2015.07.07
*************************************************************/  
void ReadInterfaceSwitchflg()
{
  if(FRAM_BufferRead(&MenuSwitchFlg,FRAM_INTERFACE_SWOTCH_LEN-1,FRAM_INTERFACE_SWOTCH_ADDR))
  {
    if(MenuSwitchFlg>=MENU_INTERFACE_MAX)MenuSwitchFlg = MENU_INTERFACE_CONVENTION;
  }
  else
  {
    MenuSwitchFlg = 0;
  }    
}

/*************************************************************
** 函数名称: PublicSysParamterInit
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void PublicSysParamterInit(void)
{
  /////////////////////////
  memset((char *)&s_stTaskAttrib,0,sizeof(s_stTaskAttrib));
  ////////////////////////
  ReadSystemPassWord();
  ReadSystemBacklightFlag();
  ReadInterfaceSwitchflg();   //添加人:fanqinghai 2015.0707
  LCD_LIGHT_ON();
  Message_ParameterInitialize();
}
/*************************************************************
** 函数名称: PublicSysParamterInit
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void ShowMainInterface(void)
{
  LZM_StartNewTask(TaskShowMainInterface);
}
/*************************************************************
** 函数名称: ReflashToReturnMainInterface
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void ReflashToReturnMainInterface(unsigned long time)
{
  if(!GetLcdCopyFlag())LcdCopyScreen();
  LZM_SetAlarm(SHOW_TIMER, time);
}
/*************************************************************
** 函数名称: SystemAutoShowText
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void SystemAutoShowText(unsigned short textlen,unsigned char *buffer)
{
  s_stTaskAttrib.textlen = textlen;
  s_stTaskAttrib.ControlMaxId = GetCurrentMsgPageNum(textlen,buffer,0);
  s_apcShowP=(char *)buffer;
  LZM_StartNewTask(TaskShowDisplayText);
}
/*************************************************************
** 函数名称: SystemAutoShowQuestionResponse
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void SystemAutoShowQuestionResponse(unsigned short datalen,unsigned char *buffer,unsigned char flag)
{
  unsigned char temp,ID;
  unsigned short len,len1,len2;
  s_stTaskAttrib.answerNum = 0;
  len =0;
  strcpy((char *)&GB2312Buffer[len],"问题：          ");
  len=strlen((char *)GB2312Buffer);
  len1 =0;
  ////////问题///////
  len2=buffer[len1++];//问题长度
  memcpy((char *)&GB2312Buffer[len],&buffer[len1],len2);//问题内容
  len += len2;
  len1 += len2;
  temp=GetCurrentNotEnoughRowNum(len,GB2312Buffer);
  if(temp)
  {
    memset(&GB2312Buffer[len],' ',16-temp);
    len +=16-temp;
  }  
  //////////////
  strcpy((char *)&GB2312Buffer[len],"备选答案：      ");
  len=strlen((char *)GB2312Buffer);
  //////////答案/////////////////
  temp =0;
  for(;;)
  {
    if(len1+3>=datalen)break;
    /////////答案ID/////////////
    ID = buffer[len1++];
    s_stTaskAttrib.ShowBuffer[0][s_stTaskAttrib.answerNum]=ID;        
    ////////答案长度///////////////
    len2 = Public_ConvertBufferToShort(&buffer[len1]);
    len1+=2;
    ////////////////////////
    if(len1+len2>datalen)break;
    //////////////////////
    if(ID<=9)//转成ASCII
    {
      ID+=0x30;
    }
    //////////////////////////////////
    s_stTaskAttrib.ShowBuffer[1][s_stTaskAttrib.answerNum*2]=ID;
    s_stTaskAttrib.ShowBuffer[1][s_stTaskAttrib.answerNum*2+1]=' ';
    ///////////////////////
    GB2312Buffer[len++] = ID;
    strcpy((char *)&GB2312Buffer[len],")");
    len=strlen((char *)GB2312Buffer);
    memcpy((char *)&GB2312Buffer[len],&buffer[len1],len2);//答案内容        
    ///////////////////////////
    temp=GetCurrentNotEnoughRowNum(len2+2,&GB2312Buffer[len-2]);
    if(temp)
    {
      memset(&GB2312Buffer[len+len2],' ',16-temp);
      len +=16-temp;
    }
    /////////////////
    len += len2;
    len1 += len2;
    s_stTaskAttrib.answerNum++;
    //len=strlen((char *)GB2312Buffer);
    ///////////////////////////////
    if(len1>=datalen)break;
  }
  ///////////////////////
  GB2312Buffer[len]='\0';
  ///////////////////////////////////
  if(s_stTaskAttrib.answerNum>8)s_stTaskAttrib.answerNum = 8;
  ///////////////////////
  ///////////////////////////
  s_stTaskAttrib.ShowBuffer[1][s_stTaskAttrib.answerNum*2]='\0';
  ///////////////////////////////////
  if(PUBLIC_CHECKBIT(flag,3))
  {
    Public_PlayTTSVoiceStr(GB2312Buffer);
  }
  s_stTaskAttrib.textlen = len;
  s_stTaskAttrib.ControlMaxId = GetCurrentMsgPageNum(len,GB2312Buffer,0);
  s_apcShowP=(char *)GB2312Buffer;
  LZM_StartNewTask(TaskShowQuestionResponse);
}
/*************************************************************
** 函数名称: ShowTextForMsgBusyFlag
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void ShowTextForMsgBusyFlag(unsigned char flag)
{
  static unsigned char sucMsgBusyFlag=0;
  if(!flag)
  {
    sucMsgBusyFlag = 1;
    CommonShareBufferBusyFlag = 1;
  }
  else
    if(sucMsgBusyFlag)
    {
      sucMsgBusyFlag=0;
      CommonShareBufferBusyFlag = 0;
    }    
}
/*************************************************************
** 函数名称: GetShowMainInterfaceFlag
** 功能描述: 是否在主界面
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned char GetShowMainInterfaceFlag(void)
{
  return ((TaskShowMainInterface==LZM_TaskFunction && !Lcd_ShowBigNumFlag())||(LZM_TaskFunction == TaskShowLogo)||(TaskMenuSystemDebugTest==LZM_TaskFunction)||(TaskMenuSystem==LZM_TaskFunction));
}
/*************************************************************
** 函数名称: IsBrushAdministratorCard
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned char IsBrushAdministratorCard(void)
{
  return (s_stServerParam.brushcardFlag == 0x55)?1:0;
}
/*************************************************************
** 函数名称: BrushAdministratorCardResponse
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned char BrushAdministratorCardResponse(unsigned char *data)
{
  if(TaskGetPassword!=LZM_TaskFunction)return 0;
  ////////////////
  unsigned char len,start;
  //////////////////////////
  start=0;
  ////////////////////////
  if(strncmp((char*)&data[start],"601703",6)!=0)
  {
    return 0;
  }
  ////////////////////////////////
  start +=6;
  len=data[start++];
  ConvertStrToIP(&data[start], s_stServerParam.mainIP,len);
  start +=len;
  len=data[start++];
  ConvertStrToIP(&data[start], s_stServerParam.backIP,len);
  start +=len;
  len=data[start++];
  memcpy(s_stServerParam.APN,&data[start],len);
  s_stServerParam.APN[len]='\0';
  start +=len;
  /////////////////////////////////
  s_stServerParam.tcp = Public_ConvertBufferToLong(&data[start]); 
  ////////////////////////////
  start +=4;
  ////////////////////////////
  s_stServerParam.udp = Public_ConvertBufferToLong(&data[start]);
  /////////////////////////
  s_stServerParam.brushcardFlag = 0x55;
  //////////////////////////////////
  PublicGetPasswordCorrect(); 
  ///////////////////////////    
  return 1;
}
/*************************************************************
** 函数名称: SystemAutoShowText
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void SystemAutoShowQuestion(void)
{
  SystemAutoShowQuestionResponse(CommonShareBufferLen-1,&RadioShareBuffer[1],RadioShareBuffer[0]);
}
/*************************************************************OK
** 函数名称: Question_DisposeRadioProtocol
** 功能描述: 提问下发协议解析
** 入口参数: pBuffer:数据首地址,BufferLen:数据长度			 
** 出口参数: 
** 返回参数: :
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Question_DisposeRadioProtocol(u8 *pBuffer, u16 BufferLen)
{
#ifdef USE_NAVILCD
  if(NaviLcd_GetCurOnlineStatus())
    NaviLcdSendCmdQuestionDisplay(1,pBuffer+1,BufferLen-1);
#endif
  QuestionResponse_SaveOneData(&pBuffer[1],BufferLen-1);
  CommonShareBufferBusyFlag = 1;//CommonShareBuffer忙标志
  CommonShareBufferLen=BufferLen;
  memcpy(RadioShareBuffer,pBuffer,CommonShareBufferLen);
  RadioShareBuffer[CommonShareBufferLen] = 0;//结尾标志
  /////////////////
  LZM_SetOperateTimer(SystemAutoShowQuestion, SECS(0.2));
}
/*************************************************************
** 函数名称: InfoService_DisposeRadioProtocol
** 功能描述: 点播信息操作协议解析
** 入口参数: pBuffer:数据首地址,BufferLen:数据长度
** 出口参数: 无
** 返回参数: 返回操作结果
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char InfoService_DisposeRadioProtocol(unsigned char *pBuffer,unsigned short BufferLen)
{
  unsigned char AckResult;    
#ifdef USE_NAVILCD
  if(NaviLcd_GetCurOnlineStatus())
    NaviLcdSendCmdInfoService(1,pBuffer,BufferLen);
#endif
  AckResult = 2;
  if(BufferLen>3&&BufferLen<1023)
  {
    ////////////////////////
    CommonShareBufferLen=Public_ConvertBufferToShort(&pBuffer[1]);
    if(CommonShareBufferLen+3<=BufferLen)
    {
      AckResult=0;
      memcpy(RadioShareBuffer,&pBuffer[3],CommonShareBufferLen);
      RadioShareBuffer[CommonShareBufferLen] = 0;//结尾标志
      CommonShareBufferBusyFlag = 1;//CommonShareBuffer忙标志
      InfoService_SaveOneData(RadioShareBuffer,CommonShareBufferLen);
      SetEvTask(EV_SHOW_TEXT);
    }
  }
  return AckResult;
}

/*******************************************************************************
*                             end of module
*******************************************************************************/

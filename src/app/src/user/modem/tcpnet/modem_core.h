/**
  ******************************************************************************
  * @file    modem_core.h 
  * @author  miaoyahan
  * @version V1.0.0
  * @date    2013-08-20 
  * @brief   
  ******************************************************************************
  * @attention
  ******************************************************************************
  */ 
#ifndef __MODEM_CORE_H
#define __MODEM_CORE_H



/*
********************************************************************************
*                               DEFINES
********************************************************************************
*/


/*
********************************************************************************
*                                 ENUM
********************************************************************************
*/
///////////////////////////////////////////////////////////////////////modem类型
typedef enum
{
  MODEM_TYPE_IDE=0, 
  MODEM_TYPE_Q, 
  MODEM_TYPE_ZTE,
  MODEM_TYPE_MAX,
  
} MODEM_TYPE;
///////////////////////////////////////////////////////////////////////modem状态
typedef enum
{
    MODEM_IDLE=0, //空闲
    MODEM_BSP,    //硬件准备
    MODEM_INIT,   //初始化
    MODEM_ACT,    //激活
    MODEM_OFF,    //关机
    MODEM_ERROR,  //错误处理
    MODEM_SYSRST, //系统复位
    MODEM_ATI,    //获取modem信息
    MODEM_SETAUD, //设置语音参数
    MODEM_NETS,   //查询网络情况
    MODEM_SMS,    //短信处理
    MODEM_NETC,   //配置网络
    MODEM_DIAL,   //拨号
    MODEM_ON,     //在线  
    MODEM_TTS,    //TTS播报模式,没有装SIM卡的情况下进入
    MODEM_Max,
}MODEM_STA;//modem状态

////////////////////////////////////////////////////////////////////////等待状态
typedef enum {
    MODWT_IDLE=0,  //空闲
    MODWT_NO,      //直发命令
    MODWT_CMD,     //基本指令
    MODWT_SEARCH,  //搜索
    MODWT_CCID,    //查询CCID
    MODWT_ATI,     //查询属性
    MODWT_LOCIP,   //获取IP
    MODWT_IPSTAT,  //查询状态
    MODWT_IPOPEN,  //打开连接
    MODWT_IPCLOSE, //关闭连接  
    MODWT_SEDATA,  //发送数据
    MODWT_REDATA,  //接收数据
    MODWT_FTP,     //FTP处理
    MODWT_SMS,     //短信处理
    MODWT_ATD,     //拨打电话
    MODWT_ERR,     //错误处理
    MODWT_MAX,
  
}MODWT_STA;//等待数据类型

////////////////////////////////////////////////////////////////////////在线状态
typedef enum
{
    MODON_IDLE=0,  //空闲
    MODON_CSQ,     //查询网络
    MODON_IPOPEN,  //打开连接
    MODON_IPCLOSE, //关闭连接
    MODON_IPSEND,  //发送数据
    MODON_DNSIP,   //配置连接使用域名还是IP
    MODON_FTP,     //文件下载
    MODON_SMSREC,  //短信业务处理,接收
    MODON_SMSSEND, //短信业务处理,发送
    MODON_PHONE,   //电话业务处理
    MODON_OFF,     //关机
    MODON_SYSREST, //系统复位
    MODON_ERR,     //错误
    MODON_MAX,

} MODON_STA;//在线状态下的处理

////////////////////////////////////////////////////////////////////////电话状态
typedef enum
{
    MODPH_IDLE=0,//空闲
    MODPH_ATD,   //拨打电话
    MODPH_ATA,   //接听电话
    MODPH_ATH,   //挂断电话
    MODPH_RING,  //振铃
    MODPH_AUDCH, //声道切换
    MODPH_CLVL,  //设置声音大小
    MODPH_VTS,   //根据语音提示输入的键值
    MODPH_ERR,   //错误
    MODPH_MAX,   //最大   

}MODPH_STA ;//电话业务状态机

/////////////////////////////////////////////////////////////////////////TTS状态
typedef enum
{
    MODTTS_IDLE=0,//空闲
    MODTTS_PLAY,  //播放TTS
    MODTTS_STOP,  //停止播报
    MODTTS_CLVL,  //调节TTS音量 
    MODTTS_ERR,   //错误

} MODTTS_STA;//TTS业务状态机

/////////////////////////////////////////////////////////////////////////SOC状态
typedef enum
{
    MODSOC_IDLE=0,//空闲
    MODSOC_WAIT,  //等待
    MODSOC_DOING, //正在连接   
    MODSOC_ONLN,  //在线
    MODSOC_ERR,   //错误
  
} MODSOC_STA;//sock连接状态

////////////////////////////////////////////////////////////////////////错误类型
typedef enum 
{
    MOD_IDLE=0,//空闲
    MOD_DO,    //忙碌
    MOD_OK,    //成功
    MOD_ER,    //失败
    MOD_TMOT,  //重发
    MOD_DOWN,  //下线
}MODERR_TYPE;//错误类型

////////////////////////////////////////////////////////////////////////短信类型
typedef enum
{
    SMS_PDU=0, //PDU格式
    SMS_PDU7,  //7位编码
    SMS_PDU8,  //8位编码
    SMS_PDUUCS,//中文编码
    SMS_TXT,   //文本模式
    SMS_ERR,   //错误类型
}MODSMS_TYPE;//modem的短信格式

////////////////////////////////////////////////////////////////////////打印类型
typedef enum
{
    MODPRTF_IDLE,   // 0空闲
    MODPRTF_ERR,    // 1出现错误
    MODPRTF_WTAPP,  // 2等待应用层打开
    MODPRTF_STAR,   // 3开始运行
    MODPRTF_BSP,    // 4驱动准备就绪
    MODPRTF_INIT,   // 5初始化
    MODPRTF_POWON,  // 6打开电源
    MODPRTF_POWOFF, // 7关闭电源
    MODPRTF_WTAT,   // 8等待AT响应
    MODPRTF_SDAT,   // 9波特率同步
    MODPRTF_WTRD,   // 10等待就绪...
    MODPRTF_CPIN,   // 11查询SIM卡状态
    MODPRTF_CCID,   // 12查询CCID
    MODPRTF_CREG,   // 13查询CREG注册情况
    MODPRTF_CGREG,  // 14查询CGREG注册情况
    MODPRTF_CSCA,   // 15查询短信中心号码
    MODPRTF_CPMS,   // 16设置短信存储器
    MODPRTF_CNMI,   // 17设置短信提示
    MODPRTF_CPPP,   // 18查询拨号状态
    MODPRTF_DPPP,   // 19PPP拨号
    MODPRTF_GIP,    // 20获取本地IP
    MODPRTF_ONLN,   // 21拨号成功,已上线!!!
    MODPRTF_RECOK,  // 22数据接收成功  
    MODPRTF_DATOK,  // 23数据发送成功  
    MODPRTF_DATFA,  // 24数据发送失败
    MODPRTF_DATER,  // 25数据发送错误
    MODPRTF_FTPOK,  // 26FTP下载成功
    MODPRTF_FTPFA,  // 27FTP下载失败
    MODPRTF_FTPER,  // 28FTP下载错误
    MODPRTF_SOCWT,  // 29等待连接应答
    MODPRTF_SOCOK,  // 30连接应答成功
    MODPRTF_SOCOT,  // 31连接不应答
    MODPRTF_MAX,    // 32 MAX
}MODPRT_TYPE;//打印指定信息的类型
/*
********************************************************************************
*                                STRUCT
********************************************************************************
*/
////////////////////////////////////////////////////////////////////AT命令结构体
typedef struct 
{
    u8  sendbuf[80]; //发送的AT命令缓存
    u8  recbuf[30];  //接收的AT命令缓存
    u16 outtime;     //指令超时时间
    u8  trynum;      //指令重发次数 
    u8  tryadd;      //每重发一次加1
}ST_MODAT;//modem发送AT指令结构体

/////////////////////////////////////////////////////////////////////modem结构体
typedef struct 
{		
    u8  csq;      //网络质量
    u8  csca[20]; //短信中心号码
    u8  apn[30];  //apn
    u8  user[30]; //拨号上网时的账号
    u8  pass[30]; //拨号上网时的密码
    u8  ccid[50]; //CCID编号

    u8  index;	  //执行指令序号
    u16 delay;    //指令延时时间
    u8  socclose; //要关闭的soc号
    u8  socopen;  //要打开的soc号
    u8  socdns;   //域名解析,0为IP连接，非0为域名连接
    u8  atack;    //at命令响应计数器,模块不响应加1, 响应则清零,主要防治死机
    u8  sleep;    //模块休眠时间，主要用于CMDA模块

    MODEM_STA state;//modem的状态
    MODON_STA onsta;//在线状态下的状态 
    MODWT_STA wait; //等待返回指令
    
}ST_MODEM;//modem信息结构体

/////////////////////////////////////////////////////////////modem运行信息结构体
typedef struct 
{
    u32 Tim_Core;//内核连续运行时间
    u32 Tim_Onln;//连续在线时间
    u32 Tim_Dwln;//连续下线时间
    u32 Tim_SocEr;//链路异常时间
    u32 Tim_SocOn[MODEM_SOC_NUM];//单个链路在线时间
    u32 Tim_SocDw[MODEM_SOC_NUM];//单个链路下线时间
    u32 Num_SocLn[MODEM_SOC_NUM];//单个链路连接次数
    u32 Num_SocRx[MODEM_SOC_NUM];//单个链路接收条数  
    u32 Num_SocTx[MODEM_SOC_NUM];//单个链路发送条数      
    u32 Num_Rest;//modem重启次数
    
}ST_MODINF;//modem运行的信息

///////////////////////////////////////////////////////////////////////FTP结构体
typedef struct 
{
    u8  ip[15];    //ip地址
    u8  port[5];   //端口
    u8  user[20];  //服务器名称
    u8  pass[20];  //服务器密码    
    u8  fpath[50]; //文件路径
    u8  fname[50]; //文件名
    u8  state;     //状态 
    u8  busy;      //ftp忙标志，传数据时非0
    u8  fend;      //文件下载结束标志
    u8  fres;      //文件下载结果 0:空闲 1:正确 2:错误
    u32 fsize;     //文件大小
}ST_MODFTP;

//////////////////////////////////////////////////////////////////////短信结构体
typedef struct
{
    MODSMS_TYPE  typr;//短信格式
    MODSMS_TYPE  typs;//短信格式 发送短信
    
    u8  busy;   //信息处理标志，忙为非0，空闲为0
    u8  rec;    //收短信标志 非0表示有新短信
    u8  send;   //短信发送标志位，非0表示有短信发送
    u16 pdulen; //PDU短信的数据长度仅用于PDU格式

    u16 dlen;                   //短信数据长度
    u8  dbuf[MODEM_SMS_SENDMAX];//短信存储缓存
    u8  num[30];                //短信号码

}ST_MODSMS;

//////////////////////////////////////////////////////////////////////电话结构体
typedef struct 
{
    MODON_STA otherwait;
    MODPH_STA state;  //电话处理状态
    u8  busy;         //电话处理标志，忙为非0，空闲为0
    u8  audch;        //语音通道
    u8  clvl;         //音量大小   
    u8  otherflg;     //通话前modem在忙 标志    
    u8  num[30];      //电话号码
    u8  vts[20];      //通话中按键    
    u8  clvlvalue[10];//一共有9个音量等级

}ST_MODHP;

//////////////////////////////////////////////////////////////////保存状态结构体
typedef struct 
{
    MODWT_STA othwait;//等待标志
    MODON_STA othsta; //状态  
    u16 othdly;       //延时
    u8  othindex;     //单步
    u8  othtryadd;    //已重试次数
    u8  othtrynum;    //重试次数

}ST_MODOTH;//用于保存状态的的阶段

///////////////////////////////////////////////////////////////////////TTS结构体
typedef struct STRUCT_MODTTS{
    u8  cmdack; //模块应答标志  收到OK清零
    u8  index;  //TTS单步处理
    u8  busy;   //TTS处理标志，非0为正在播报 ,0为播报完毕
    u8  wait;   //TTS等待标志，处理与主状态机冲突   
    u8  send;   //TTS发送播报标志
    u8  type;   //播报类型 0为立即播报  1为延时播报
    u8  len;    //TTS长度
    u8  clvl;   //TTS音量
    u8  clvlvalue[10];   //一共有9个音量等级 
    u8  buf[MODTTS_SIZE];//TTS缓存
    MODTTS_STA  state;   //TTS处理状态
}ST_MODTTS;

////////////////////////////////////////////////////////////////////IP参数结构体
typedef struct
{
    u8  mode[5];//TCP或UDP
    u8  ip[50]; //IP或域名
    u8  port[6];//端口
  
}SOC_IPVAL;//IP参数结构体

////////////////////////////////////////////////////////AT命令发送连接数据结构体
typedef struct 
{
    u8  soc;      //缓存对应的soc号
    u16 len;      //数据缓存区长度
    u8  buf[MODEM_SOC_TX_SIZE];//数据缓存区用于存放大批量数据
}SOC_ATBUF;//socket发送缓存,AT命令


/*
********************************************************************************
*                          GLOBAL VARIABLES
********************************************************************************
*/      
extern MODEM_TYPE   Modem_Type;

extern ST_MODAT     modat;
extern ST_MODEM     modem; 
extern ST_MODINF    modInf;
extern ST_MODFTP    modftp;   
extern ST_MODSMS    modsms;   
extern ST_MODHP     modphone;
extern ST_MODTTS    modtts;
extern SM_PARAM     modsmspdu;

extern u8  QuectelAtTtsFlag;
extern u8  Modem_ActPinOnDly;
extern u8  Modem_ActPinOffDly;

extern u8         ModSocBuf  [];
extern MODLIB_BUF ModSocNdNum[];
extern MODLIB_NOD ModSocNode [];

extern MODSOC_STA Modem_IpSock_STA[MODEM_SOC_NUM];

/*
********************************************************************************
*                               FUNCTIONS
********************************************************************************
*/
void  Modem_Printf (char *format, ...);
void  Modem_PrtfTo(MODPRT_TYPE type);
void  Modem_Run (void);
void  Modem_Delay (u8 delay);
void  Modem_Tts_Cmd (u8 *pCmd);

void  Modem_Send_cmd(MODWT_STA type,u8 *pCmd, u8 *pReply,u8 Outtime, u8 Try);
void  Modem_Send_cmd_Base(u8 *pCmd, u8 *pReply,u8 Outtime, u8 Try);
void  Modem_Send_cmd_Ati(u8 Outtime, u8 Try);
void  Modem_Send_cmd_AtiFibocom(u8 Outtime, u8 Try);
void  Modem_Send_cmd_Search(u8 *pCmd,u8 Outtime, u8 Try);
void  Modem_Send_cmd_CCID(u8 *pCmd,u8 Outtime, u8 Try);
void  Modem_Send_cmd_Apn(u8 Outtime, u8 Try);
void  Modem_Send_cmd_IpSta(u8 Outtime, u8 Try);
void  Modem_Send_cmd_IpDnsGip  (u8 *pCmd ,u8 Outtime, u8 Try);
void  Modem_Send_cmd_IpOpen(u8 index,u8 Outtime);
void  Modem_Send_cmd_IpClose(u8 index,u8 Outtime, u8 Try);
void  Modem_Send_cmd_IpLoc(u8 *pCmd ,u8 Outtime, u8 Try);
void  Modem_Send_cmd_IpData(u8 SocNum, u16 len, u8 Outtime);
void  Modem_Send_cmd_FtpLogin(u8 type, u8 Outtime, u8 Try);
void  Modem_Send_cmd_FtpDown(u8 type, u8 Outtime, u8 Try);
void  Modem_Send_cmd_Sms(u8 type ,u8 Outtime, u8 Try);

#endif








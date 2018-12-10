/*
********************************************************************************
*
*                                                              
*
* Filename      : modem_app_second.c
* Version       : V1.00
* Programmer(s) : miaoyahan
* @date         : 2014-10-23  
* 
*
********************************************************************************
*/

/*
********************************************************************************
*                               INCLUDES
********************************************************************************
*/
#ifdef EYE_MODEM
#include "include.h"
#else
#include  <app_cfg.h>
#include  <includes.h>
#endif

/*
********************************************************************************
*                               DEFIEN START
********************************************************************************
*/
#ifdef MODEM_APP_SECOND_H
/*
********************************************************************************
*                             LOCAL DEFINES
********************************************************************************
*/

#define  EYENET_SECOND_SOC                         (1)//SOC号

/*
********************************************************************************
*                         LOCAL DATA TYPES
********************************************************************************
*/

/*
********************************************************************************
*                          LOCAL VARIABLES
********************************************************************************
*/
static Modem_APP_STATE NetSecond_State;    //状态机
static APP_TYPE_DOWN   NetSecond_TypeDown; //下线类型
static SOC_IPVAL       NetSecond_AppointIp;//指定IP
static APP_TYPE_VAL    NetSecond_IpVal;    //参数

static u8  NetSecond_ConSta;//连接状态 0为下线  1为在线
static u8  NetSecond_ConTry;//连接重试次数
static u32 NetSecond_ConCnt;//当前连接类型持续计数器
static u32 NetSecond_ConVal;//当前连接类型持续时间

static u32 NetSecond_HeartVal;//心跳值
static u32 NetSecond_HeartCnt;//心跳计数器

/*
********************************************************************************
*                          GLOBAL VARIABLES
********************************************************************************
*/

/*
********************************************************************************
*                          EXTERN VARIABLES
********************************************************************************
*/

/*
********************************************************************************
*                           FUNCTIONS EXTERN
********************************************************************************
*/


/*
********************************************************************************
*                              FUNCTIONS
********************************************************************************
*/
/**
  * @brief  接获取参数
  * @param  type: 参数类型
  * @retval 返回参数结构 1->成功  0->非法数据
  */
u8 Net_Second_Value(APP_TYPE_VAL type, void *pValue)
{
    u8  tab[50]={0};
    u8  len;
    u8  result;
    u32 id_ip;
    u32 id_port;
    u16 temp;

    SOC_IPVAL *pstip = pValue;
    u32 *pHeart = pValue;

    if(type == VAL_IPMAIN)//主IP
    {
         id_ip   = E2_SECOND_MAIN_SERVER_IP_ID;
         id_port = E2_SECOND_MAIN_SERVER_TCP_PORT_ID;
    }
    else if(type == VAL_IPBACK)//备份IP
    {
         id_ip   = E2_SECOND_BACKUP_SERVER_IP_ID;
         id_port = E2_SECOND_MAIN_SERVER_TCP_PORT_ID;
    }
    else if(type == VAL_IPTO)//指定IP
    {
        *pstip = NetSecond_AppointIp;
        return 1;
    }
    else if(type == VAL_HEART)//心跳
    {
        if(EepromPram_ReadPram(E2_TERMINAL_HEARTBEAT_ID, tab) == 4)
        {
            *pHeart = (tab[0] << 24) + (tab[1] << 16) + (tab[2] << 8) + tab[3];
            return 1;//参数可用
        }
    }
    len = EepromPram_ReadPram(id_ip, tab);
    if((len > 0)&&(len < 30))
    {
        result = 0x0a;
        memcpy(pstip->ip,tab,len);
    }

    len = EepromPram_ReadPram(id_port, tab);
    if((len > 0)&&(len < 5))
    {
        result <<= 0x04;
        result |= 0x0a;
        temp = (tab[0] << 24) + (tab[1] << 16) + (tab[2] << 8) + tab[3];
        sprintf((char*)pstip->port,"%d",temp);
        if(temp == 0)
        {
            result = 0;//第二连接端口为0是不打开
        }
    }
    if(result == 0xaa)
    {
        strcpy((char*)pstip->mode,"TCP");//模式
        return 1;//参数可用
    }
    return 0;//参数非法
}

/**
  * @brief  连接登录，主要是触发注册鉴权任务
  * @param  type: 保留 
            state: 下一步执行的状态
  * @retval None
  */
void Net_Second_Login(Modem_APP_STATE state)
{
    NetSecond_ConTry = 0;
    NetSecond_ConCnt = 0;
    NetSecond_State  = state;
    SetLink2LoginAuthorizationState(LOGIN_READ_AUTHORIZATION_CODE);
    SetTimerTask(TIME_LINK2_LOGIN, SYSTICK_1SECOND);    
}
/**
  * @brief  连接下线，主要是清除捡软标志
  * @param  type: 下线类型
            state: 下一步执行的状态
  * @retval None
  */
void Net_Second_Down(APP_TYPE_DOWN type, Modem_APP_STATE state)
{
    NetSecond_TypeDown = type;//刷新类型
    NetSecond_State    = state;//刷新状态
    NetSecond_ConSta   = 0;//连接下线
    ClearTerminalAuthorizationFlag(CHANNEL_DATA_2);//鉴权标志清0
    
    //////////////////////////////////////////////////////////////////////切换IP
    if(type == DOWN_IPSW)
    {
        if((++NetSecond_ConTry) > 2)
        {
            NetSecond_ConTry = 0;
            if(NetSecond_IpVal == VAL_IPMAIN)
            {
                NetSecond_ConVal = 10*MODEM_TMIN;
                NetSecond_IpVal = VAL_IPBACK;//主IP-重试失败转到-备份IP
            }
            else if(NetSecond_IpVal == VAL_IPBACK)
            {
                NetSecond_IpVal = VAL_IPMAIN;//备份IP-重试失败转到-主IP
            }
            else if(NetSecond_IpVal == VAL_IPTO)
            {
                NetSecond_IpVal = VAL_IPMAIN;//指定IP-重试失败转到-主IP
            }            
        }  
    }
    //////////////////////////////////////////////////////////////////////当前IP
    else if(type == DOWN_IPOP)
    {
        ;//保留
    }
    ////////////////////////////////////////////////////////////////////////主IP
    else if(type == DOWN_IPMN)
    {
        NetSecond_IpVal = VAL_IPMAIN;//主IP
    }   
    //////////////////////////////////////////////////////////////////////指定IP
    else if(type == DOWN_IPTO)
    {
        NetSecond_IpVal = VAL_IPTO;//指定IP
    }     
}

/**
  * @brief  联接初始化
  * @param  None
  * @retval None
  */
void Net_Second_Init(void)
{
    SOC_IPVAL ip;
    u32 heartval;
    
    NetSecond_State  = MODEM_APP_IDLE;
    NetSecond_IpVal  = VAL_IPMAIN;//默认主连接
    NetSecond_ConVal = 0;//默认一直连接
    NetSecond_ConSta = 0;//默认下线
    NetSecond_HeartVal = MODEM_TMIN*5;//默认5分钟心跳

    //////////////////////////////////////////////////////////////////鉴权标志清
    ClearTerminalAuthorizationFlag(CHANNEL_DATA_2);

    //////////////////////////////////////////////////////////////////////IP参数
    if(Net_Second_Value(NetSecond_IpVal,&ip) == 0)
    {
        return;//未设置连接IP或参数错误
    }
    NetSecond_State = MODEM_APP_OPEN;//打开链接
    
    ////////////////////////////////////////////////////////////////////心跳参数
    if(Net_Second_Value(VAL_HEART,&heartval))
    {
        NetSecond_HeartVal = heartval*3*MODEM_TSEC;// 3个心跳值超时
    }
    if((NetSecond_HeartVal > (MODEM_TMIN*5))||(NetSecond_HeartVal < 30))//dxl,2015.4.21,增加最小值的限定,防止心跳时间间隔设置错误的情况发生
    {
        NetSecond_HeartVal = MODEM_TMIN*5;//强制转为5分钟
    }
    
}
/*
  * @brief  连接回调函数,平台下发数据时被调用
  * @param  *p；指向连接下发的数据
  * @param  len：数据长度
  * @retval None
  */
void Net_Second_Call(u8 *p,u16 len)
{
    NetSecond_ConSta   = 1;//置在线标志
    NetSecond_HeartCnt = 0;//清心跳计数
    RadioProtocol_AddRecvDataForParse(CHANNEL_DATA_2,p,len);   
}
/*
  * @brief  连接IP参数,连接IP时被调用
  * @param  *pMode：指向连接类型
  * @param  *pAddr：指向IP地址
  * @param  *pPort：指向端口号
  * @retval None
  */
void Net_Second_IP(u8 *pMode ,u8 *pAddr, u8 *pPort)
{ 
    SOC_IPVAL ip;

    memset((void*)&ip,0,sizeof(SOC_IPVAL));//清空结构体
    
    if(Net_Second_Value(NetSecond_IpVal,&ip) == 0)
    {
        NetSecond_IpVal = VAL_IPMAIN;//参数非法将强制转为主服务器
        Net_Second_Value(NetSecond_IpVal,&ip);
    }
    strcpy((char*)pMode,(char*)ip.mode); //TCP或UDP
    strcpy((char*)pAddr,(char*)ip.ip);   //IP地址或域名
    strcpy((char*)pPort,(char*)ip.port); //端口
}


/*
  * @brief  打开连接
  * @param  None
  * @retval None
  */
void Net_Second_Open(void)
{
    if((NetSecond_State == MODEM_APP_OPEN))//正在打开连接
    {
        return;
    }
    NetSecond_State = MODEM_APP_OPEN;
}
/*
  * @brief  关闭连接
  * @param  None
  * @retval None
  */
void Net_Second_Close(void)
{
    if(NetSecond_State != MODEM_APP_CLOSE)
    {
        NetSecond_State = MODEM_APP_CLOSE;
    }
}
/*
  * @brief  连接到指定IP
  * @param  ip_value: ip参数;
            time:有效截止时间
  * @retval None
  */
void Net_Second_ConTo(SOC_IPVAL ip_value, u16 time)
{
    NetSecond_AppointIp = ip_value;//IP参数
    NetSecond_ConVal    = MODEM_TMIN*time;//允许连接的时间 
    NetSecond_State     = MODEM_APP_DOWN;//当前连接时间已到
    NetSecond_TypeDown  = DOWN_IPTO;//下线连接到指定IP  
}

/*
  * @brief  发送数据到连接平台
  * @param  None
  * @retval 成功返回长度，失败返回0
  */
u16  Net_Second_Send(u8 *p, u16 len)
{
    u16 sendlen=0;

    if(Modem_State_SocSta(EYENET_SECOND_SOC))
    {
        sendlen = Modem_Api_Socket_Send(EYENET_SECOND_SOC,p,len);
    }
   
    return sendlen;
}
/**
  * @brief  连接监听(长期处于该状态)
  * @param  None
  * @retval None
  */
void Net_Second_Listen(void)
{
    ////////////////////////////////////////////////////////////////////链路检查
    if(Modem_Api_SocSta(EYENET_SECOND_SOC) == __FALSE)
    {
        Net_Second_Down(DOWN_IPOP,MODEM_APP_OPEN);
        return;
    } 
    //////////////////////////////////////////////////////////备份或指定连接检查
    if(NetSecond_ConVal)
    {
        if((NetSecond_ConCnt++) > NetSecond_ConVal)
        {
            NetSecond_ConCnt   = 0;
            NetSecond_ConVal   = 0;
            NetSecond_State    = MODEM_APP_DOWN;//关闭连接 -> 连接时间已到
            NetSecond_TypeDown = DOWN_IPMN;//下线切换到主IP
        }
    }
    ////////////////////////////////////////////////////////////////心跳超时检查
    if((NetSecond_HeartCnt++) > NetSecond_HeartVal)
    {
        NetSecond_HeartCnt = 0;
        NetSecond_State    = MODEM_APP_DOWN;//关闭连接 -> 心跳超时
        NetSecond_TypeDown = DOWN_IPOP;//下线重新打开(IP不变)   
    }
    
}

/**
  * @brief  连接状态
  * @param  None
  * @retval 0 -> 不在线
            1 -> 在线
  */
u8 Net_Second_ConSta(void)
{
    return NetSecond_ConSta;
}

/**
  * @brief  连接检查
  * @param  None
  * @retval None
  */
void Net_Second_Check(void)
{

}


/**
  * @brief  调试专用
  * @param  None
  * @retval None
  */
void Net_Second_Debug(void)
{
    static u8 Second_debug;
    
    if(Second_debug)
    {
        Second_debug = 0;
    }
}
/**
  * @brief  连接处理
  * @param  None
  * @retval None
  */
void Net_Second_Run(void)
{

    MODERR_TYPE modemack;

    Net_Second_Debug();

    Net_Second_Check();
    
    switch(NetSecond_State)
    {
        case MODEM_APP_IDLE:
            {
                ;
            }
        break; 
        case MODEM_APP_OPEN://开打一个连接
            {
                modemack = Modem_Api_Socket_Open(EYENET_SECOND_SOC,
                                                 Net_Second_IP,
                                                 Net_Second_Call);
                if(modemack == MOD_OK)
                {
                    Net_Second_Login(MODEM_APP_LISTEN);//成功->进入监听
                }
                else if(modemack == MOD_ER)
                {
                    Net_Second_Down(DOWN_IPSW,MODEM_APP_OPEN);//失败->切换IP
                }
            }
        break;     
        case MODEM_APP_LISTEN://监听一个连接,长期处于该状态
            {
                Net_Second_Listen();
            }
        break;        
        case MODEM_APP_CLOSE://关闭一个连接
            {   
                modemack = Modem_Api_Socket_Close(EYENET_SECOND_SOC);
                if(modemack == MOD_OK)
                {
                    NetSecond_State = MODEM_APP_IDLE;//等待应用打开
                }
                else if(modemack == MOD_ER)
                {
                    Modem_Api_Rst();//模块异常->重启模块
                }
            }
        break;        
        case MODEM_APP_DOWN://主动下线
            {
                modemack = Modem_Api_Socket_Close(EYENET_SECOND_SOC);
                if(modemack == MOD_OK)
                {
                    Net_Second_Down(NetSecond_TypeDown,MODEM_APP_OPEN);
                }
                else if(modemack == MOD_ER)
                {
                    Modem_Api_Rst();//关闭连接失败后重启通讯模块
                }
            }
        break; 
        default:
        break;

    }
}


/*
********************************************************************************
*                 DEFIEN END
********************************************************************************
*/
#endif 








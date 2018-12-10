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
#ifdef ICCARD_CERTIFICATE_SERVICE_H
/*
********************************************************************************
*                             LOCAL DEFINES
********************************************************************************
*/

#define  ICCARD_CERTIFICATE_SERVICE_SOC                         (2)//SOC号

extern u8	TerminalAuthorizationFlag;//终端鉴权标志,bit0:连接1;bit1:连接2;bit2~bit7保留;

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
static Modem_APP_STATE CertService_State;    //状态机
static APP_TYPE_DOWN   CertService_TypeDown; //下线类型
static SOC_IPVAL       CertService_AppointIp;//指定IP
static APP_TYPE_VAL    CertService_IpVal;    //参数

static u8  CertService_ConSta;//连接状态 0为下线  1为在线
static u8  CertService_ConTry;//连接重试次数
static u32 CertService_ConCnt;//当前连接类型持续计数器
static u32 CertService_ConVal;//当前连接类型持续时间

static u32 CertService_HeartVal;//心跳值
static u32 CertService_HeartCnt;//心跳计数器

static u8	CertService_Link_Flg = 0;
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
u8 CertService_ParaConfig(APP_TYPE_VAL type, void *pValue)
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
         id_ip   = E2_IC_MAIN_SERVER_IP_ID;
         id_port = E2_IC_MAIN_SERVER_TCP_PORT_ID;
    }
    else if(type == VAL_IPBACK)//备份IP
    {
         id_ip   = E2_IC_BACKUP_SERVER_IP_ID;
         id_port = E2_IC_MAIN_SERVER_TCP_PORT_ID;
    }
    else if(type == VAL_IPTO)//指定IP
    {
        *pstip = CertService_AppointIp;
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
		len = EepromPram_ReadPram(id_ip,tab);
    if((len > 0)&&(len < 30))
    {
        result = 0x0a;
        memcpy(pstip->ip,tab,len);
    }

		len = EepromPram_ReadPram(id_port,tab);
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
void CertService_Login(Modem_APP_STATE state)
{
    CertService_ConTry = 0;
    CertService_ConCnt = 0;
    CertService_State  = state;
	CertService_ConSta   = 1;//置在线标志
	TerminalAuthorizationFlag |= CHANNEL_DATA_3;    //强制置位鉴权成功标志  
    //SetLink2LoginAuthorizationState(LOGIN_READ_AUTHORIZATION_CODE);
    //SetTimerTask(TIME_LINK2_LOGIN, SYSTICK_1SECOND);    
}
/**
  * @brief  连接下线，主要是清除捡软标志
  * @param  type: 下线类型
            state: 下一步执行的状态
  * @retval None
  */
void CertService_Down(APP_TYPE_DOWN type, Modem_APP_STATE state)
{
    CertService_TypeDown = type;//刷新类型
    CertService_State    = state;//刷新状态
    CertService_ConSta   = 0;//连接下线
    ClearTerminalAuthorizationFlag(CHANNEL_DATA_3);//鉴权标志清0
    
    //////////////////////////////////////////////////////////////////////切换IP
    if(type == DOWN_IPSW)
    {
        if((++CertService_ConTry) > 2)
        {
            CertService_ConTry = 0;
            if(CertService_IpVal == VAL_IPMAIN)
            {
                CertService_ConVal = 10*MODEM_TMIN;
                CertService_IpVal = VAL_IPBACK;//主IP-重试失败转到-备份IP
            }
            else if(CertService_IpVal == VAL_IPBACK)
            {
                CertService_IpVal = VAL_IPMAIN;//备份IP-重试失败转到-主IP
            }
            else if(CertService_IpVal == VAL_IPTO)
            {
                CertService_IpVal = VAL_IPMAIN;//指定IP-重试失败转到-主IP
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
        CertService_IpVal = VAL_IPMAIN;//主IP
    }   
    //////////////////////////////////////////////////////////////////////指定IP
    else if(type == DOWN_IPTO)
    {
        CertService_IpVal = VAL_IPTO;//指定IP
    }     
}

/**
  * @brief  联接初始化
  * @param  None
  * @retval None
  */
void CertService_Init(void)
{
    SOC_IPVAL ip;
    u32 heartval;
    
    CertService_State  = MODEM_APP_IDLE;
    CertService_IpVal  = VAL_IPMAIN;//默认主连接
    CertService_ConVal = 0;//默认一直连接
    CertService_ConSta = 0;//默认下线
    CertService_HeartVal = MODEM_TMIN*5;//默认5分钟心跳  认证平台不检测心跳

    //////////////////////////////////////////////////////////////////鉴权标志清
    ClearTerminalAuthorizationFlag(CHANNEL_DATA_3);

    //////////////////////////////////////////////////////////////////////IP参数
    if(CertService_ParaConfig(CertService_IpVal,&ip) == 0)
    {
        return;//未设置连接IP或参数错误
    }
    CertService_State = MODEM_APP_OPEN;//打开链接
    
    ////////////////////////////////////////////////////////////////////心跳参数
    if(CertService_ParaConfig(VAL_HEART,&heartval))
    {
        CertService_HeartVal = heartval*3*MODEM_TSEC;// 3个心跳值超时
    }
    if((CertService_HeartVal > (MODEM_TMIN*5))||(CertService_HeartVal < 30))//dxl,2015.4.21,增加最小值的限定,防止心跳时间间隔设置错误的情况发生
    {
        CertService_HeartVal = MODEM_TMIN*5;//强制转为5分钟
    }

		CertService_HeartVal = 0;//认证平台不检测心跳
}
/*
  * @brief  连接回调函数,平台下发数据时被调用
  * @param  *p；指向连接下发的数据
  * @param  len：数据长度
  * @retval None
  */
void CertService_Call(u8 *p,u16 len)
{
    CertService_ConSta   = 1;//置在线标志
    CertService_HeartCnt = 0;//清心跳计数
    RadioProtocol_AddRecvDataForParse(CHANNEL_DATA_3,p,len);   
}
/*
  * @brief  连接IP参数,连接IP时被调用
  * @param  *pMode：指向连接类型
  * @param  *pAddr：指向IP地址
  * @param  *pPort：指向端口号
  * @retval None
  */
void CertService_IP(u8 *pMode ,u8 *pAddr, u8 *pPort)
{ 
    SOC_IPVAL ip;

    memset((void*)&ip,0,sizeof(SOC_IPVAL));//清空结构体
    
    if(CertService_ParaConfig(CertService_IpVal,&ip) == 0)
    {
        CertService_IpVal = VAL_IPMAIN;//参数非法将强制转为主服务器
        CertService_ParaConfig(CertService_IpVal,&ip);
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
void CertService_Open(void)
{
    if((CertService_State == MODEM_APP_OPEN))//正在打开连接
    {
        return;
    }
    CertService_State = MODEM_APP_OPEN;
}
/*
  * @brief  关闭连接
  * @param  None
  * @retval None
  */
void CertService_Close(void)
{
    if(CertService_State != MODEM_APP_CLOSE)
    {
        CertService_State = MODEM_APP_CLOSE;
    }
	ClearTerminalAuthorizationFlag(CHANNEL_DATA_3);//鉴权标志清0
}
/*
  * @brief  连接到指定IP
  * @param  ip_value: ip参数;
            time:有效截止时间
  * @retval None
  */
void CertService_ConTo(SOC_IPVAL ip_value, u16 time)
{
    CertService_AppointIp = ip_value;//IP参数
    CertService_ConVal    = MODEM_TMIN*time;//允许连接的时间 
    CertService_State     = MODEM_APP_DOWN;//当前连接时间已到
    CertService_TypeDown  = DOWN_IPTO;//下线连接到指定IP  
}

/*
  * @brief  发送数据到连接平台
  * @param  None
  * @retval 成功返回长度，失败返回0
  */
u16  CertService_Send(u8 *p, u16 len)
{
    u16 sendlen=0;

    if(Modem_State_SocSta(ICCARD_CERTIFICATE_SERVICE_SOC))
    {
        sendlen = Modem_Api_Socket_Send(ICCARD_CERTIFICATE_SERVICE_SOC,p,len);
    }
   
    return sendlen;
}
/**
  * @brief  连接监听(长期处于该状态)
  * @param  None
  * @retval None
  */
void CertService_Listen(void)
{
    ////////////////////////////////////////////////////////////////////链路检查
    if(Modem_Api_SocSta(ICCARD_CERTIFICATE_SERVICE_SOC) == __FALSE)
    {
        CertService_Down(DOWN_IPOP,MODEM_APP_OPEN);
        return;
    } 
    //////////////////////////////////////////////////////////备份或指定连接检查
    if(CertService_ConVal)
    {
        if((CertService_ConCnt++) > CertService_ConVal)
        {
            CertService_ConCnt   = 0;
            CertService_ConVal   = 0;
            CertService_State    = MODEM_APP_DOWN;//关闭连接 -> 连接时间已到
            CertService_TypeDown = DOWN_IPMN;//下线切换到主IP
        }
    }
    ////////////////////////////////////////////////////////////////心跳超时检查
    if(CertService_HeartVal){
	    if((CertService_HeartCnt++) > CertService_HeartVal)
	    {
	        CertService_HeartCnt = 0;
	        CertService_State    = MODEM_APP_DOWN;//关闭连接 -> 心跳超时
	        CertService_TypeDown = DOWN_IPOP;//下线重新打开(IP不变)   
	    }
    }
}

/**
  * @brief  连接状态
  * @param  None
  * @retval 0 -> 不在线
            1 -> 在线
  */
u8 CertService_ConState(void)
{
    return CertService_ConSta;
}

/**
  * @brief  连接检查
  * @param  None
  * @retval None
  */
void CertService_Check(void)
{

}


/**
  * @brief  调试专用
  * @param  None
  * @retval None
  */
void CertService_Debug(void)
{
    static u8 Second_debug;
    
    if(Second_debug)
    {
        Second_debug = 0;
    }
}
u8 CertService_GetLinkFlg(void)
{
	return CertService_Link_Flg;
}
void CertService_SetLinkFlg(void)
{
	CertService_Link_Flg = 1;
}
void CertService_ClrLinkFlg(void)
{
	CertService_Link_Flg = 0;
}

/**
  * @brief  连接处理
  * @param  None
  * @retval None
  */
void CertService_Run(void)
{

    MODERR_TYPE modemack;

    //CertService_Debug();

    //CertService_Check();
    if(0 == CertService_Link_Flg)return;//二汽IC卡没插入不连接认证服务器
    switch(CertService_State)
    {
        case MODEM_APP_IDLE:
            {
                ;
            }
        break; 
        case MODEM_APP_OPEN://开打一个连接
            {
                modemack = Modem_Api_Socket_Open(ICCARD_CERTIFICATE_SERVICE_SOC,
                                                 CertService_IP,
                                                 CertService_Call);
                if(modemack == MOD_OK)
                {
                    CertService_Login(MODEM_APP_LISTEN);//成功->进入监听
                }
                else if(modemack == MOD_ER)
                {
                    CertService_Down(DOWN_IPSW,MODEM_APP_OPEN);//失败->切换IP
                }
            }
        break;     
        case MODEM_APP_LISTEN://监听一个连接,长期处于该状态
            {
                CertService_Listen();
            }
        break;        
        case MODEM_APP_CLOSE://关闭一个连接
            {   
                modemack = Modem_Api_Socket_Close(ICCARD_CERTIFICATE_SERVICE_SOC);
                if(modemack == MOD_OK)
                {
                    CertService_State = MODEM_APP_IDLE;//等待应用打开
                }
                else if(modemack == MOD_ER)
                {
                    Modem_Api_Rst();//模块异常->重启模块
                }
            }
        break;        
        case MODEM_APP_DOWN://主动下线
            {
                modemack = Modem_Api_Socket_Close(ICCARD_CERTIFICATE_SERVICE_SOC);
                if(modemack == MOD_OK)
                {
                    CertService_Down(CertService_TypeDown,MODEM_APP_OPEN);
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










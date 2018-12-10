/**
  ******************************************************************************
  * @file    modem_state.c
  * @author  miaoyahan
  * @version V1.0.0
  * @date    2013-08-21
  * @brief   
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#ifdef EYE_MODEM
#include "include.h"
#else
#include  <app_cfg.h>
#include  <includes.h>
#endif


/*
*********************************************************************************************************
*                                               DEFINES
*********************************************************************************************************
*/
#define CSQSTAT_TIME                       (60*MODEM_TSEC)
#define PHONE_TIME_CLOSE                   (60*MODEM_TSEC)//关闭链路前最长通话时间

/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES   本地变量
*********************************************************************************************************
*/

static SOC_ATBUF    soc_atbuf;
static MODSOC_STA   Modem_Phone_SocSta[MODEM_SOC_NUM] 
                             = {MODSOC_IDLE};//用于长时间通话强制清零标志时暂存
static ST_MODOTH StOtherState;//保存各状态机状态

static MODON_STA    ModemStateDelay;//延时指定时间后切换状态的状态
static u32 ModemDelayCount;//延时指定时间后切换状态的计数器
static u8  ModemDelayStateFlg;//延时指定时间后切换状态的标志位
static u32 ModemSimNoTime;//连续没有装sim卡的时间
static u32 ModemAtdOutTime;//拨打电话超时时间
static u32 ModemPhoneTm;//通话时间计数器
static u8  ModemOffFLag;//关机标志

static u16 TtsOutTimeCount;//tts异常超时计数器

static u16 CsqStatIsCnt;//网络质量、链接状态查询计数器

/*
*********************************************************************************************************
*                                          GLOBAL VARIABLES    全局变量
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/
/*
  * @brief  modem初始化，主要有串口、IO口.
  * @param  None
  * @retval None
  */
void Modem_State_Init (void)
{      
    u8 i;
    
    ///////////////////////////////////////////////////////////////modem重启计数
    modInf.Num_Rest++;
    //////////////////////////////////////////////////////////////初始化数据结构
    memset((u8*)&modsmspdu,0,sizeof(SM_PARAM));
    memset((u8*)&modsms,0,sizeof(ST_MODSMS));
    memset((u8*)&modphone,0,sizeof(ST_MODHP));
    memset((u8*)&modftp,0,sizeof(ST_MODFTP));
    memset(Modem_IpSock_STA,MODSOC_IDLE,sizeof(Modem_IpSock_STA));
    modem.csq   = 0;
    modem.onsta = MODON_IDLE;
    modem.state = MODEM_ACT;//激活模块
    modem.index = 0;  

    for(i=0;i<MODEM_SOC_NUM;i++)
    {
        ModLib_NodRst(&ModSocNode[i]);
    }
                
    ////////////////////////////////////////////////////////////////初始化各接口
    Modem_Api_Init   (); 
    Modem_Port_Init  ();
    #ifdef MODEM_DEBUG_H
    Modem_Debug_Init ();
    #endif
    Modem_Uart_NodRst();
    
    Modem_PrtfTo(MODPRTF_INIT);
     
}
/**
  * @brief  在线状态下延时指定时间后切换状态
  * @param  delay:延时的时间,单位为秒
  * @param  state:要切换的状态
  * @retval None
  */
void  Modem_State_Delay(u16 delay,MODON_STA state)
{
    ModemDelayCount = delay*MODEM_TSEC;
    ModemStateDelay = state;
    ModemDelayStateFlg = 1;
}

/**
  * @brief  在线状态下延时指定时间后切换状态
  * @param  None
  * @retval None
  */
void Modem_State_DelayPro(void)
{
    if(modapi.wait != MODON_IDLE)
    {
        return ;//modem在忙
    } 
    if(ModemDelayStateFlg)
    {
        if(ModemDelayCount == 0)
        {
            modem.onsta = ModemStateDelay;
            ModemDelayStateFlg = 0;
        }
        else
        {
            ModemDelayCount--;
        }
    }
}
/*
  * @brief  modem类型
  * @param  type : 0->模块类型;
  *                1->行标类型;
  * @retval 返回当前模块类型
  */
u8  Modem_State_Type (u8 type) 
{
    u8 value=0;
    
    if(type == 0)
    {
        value = Modem_Type;
    }
    else
    {
        if(Modem_Type == MODEM_TYPE_Q)////////////移远 G网
        {              
            value = value | 0x01;
        }                
        else if(Modem_Type == MODEM_TYPE_ZTE)/////中兴 C网
        {
            value = value | 0x02;
        }  
        else
        {
            value = value | 0x01;//默认G网
        }
    }

    return value;
}

/*
  * @brief  modem的状态.
  * @param  None
  * @retval 返回modem所处的状态
  */
MODEM_STA  Modem_State_Is (void) 
{          
    return modem.state;  
}
/**
  * @brief  保存状态机状态
  * @param  None.
  * @retval None.
  */
void Modem_State_Push (void) 
{
    StOtherState.othdly      = modem.delay;
    StOtherState.othwait     = modem.wait;    
    StOtherState.othindex    = modem.index;
    StOtherState.othsta      = modem.onsta;
    StOtherState.othtryadd   = modat.tryadd;
    StOtherState.othtrynum   = modat.trynum;

    modem.delay = 0;//立即执行新动作
    modem.wait  = MODWT_IDLE;
}

/**
  * @brief  还原状态机状态
  * @param  None.
  * @retval None.
  */
void Modem_State_Pop (void) 
{
    modem.delay          = StOtherState.othdly;
    modem.wait           = StOtherState.othwait;
    modem.index          = StOtherState.othindex;
    modem.onsta      = StOtherState.othsta;
    modat.tryadd = StOtherState.othtryadd; 
    modat.trynum = StOtherState.othtrynum;
    modapi.wait          = StOtherState.othsta;
}
/*
  * @brief  切换modem的状态
  * @param  state:要切换的状态
  * @retval None
  */
void  Modem_State_Switch (MODEM_STA state) 
{          
    modem.state    = state;
    modem.index    = 0;
    modem.delay    = 0;
}

/*
  * @brief  切换在线状态下的modem状态
  * @param  state:要切换的状态
  * @retval None
  */
void  Modem_State_SwitchOn (MODON_STA state) 
{          
    modem.onsta= state;
    modem.index    = 0;
    if(state == MODON_OFF)
    {
        ModemOffFLag = 1;
    }    
}

/*
  * @brief  切换电话状态机
  * @param  state:要切换的状态
  * @retval None
  */
void  Modem_State_SwitchPhone (MODPH_STA state) 
{          
    modphone.state= state;
    modem.index = 0;
}
/*
  * @brief  检查是否有要发送的数据.
  * @param  None
  * @retval 有数据发送返回1, 无数据发送返回0
  */
u8  Modem_State_CheckSoc (void)
{
    u8 i;
    for(i=0;i<MODEM_SOC_NUM;i++)
    {
        if(ModLib_NodNum(&ModSocNode[i]))
        {
            soc_atbuf.soc = i;
            soc_atbuf.len = ModLib_NodRet(&ModSocNode[i],soc_atbuf.buf);

            if(Modem_IpSock_STA[i] == MODSOC_ONLN)
            {
                modapi.wait   = MODON_IPSEND;//进入等待状态 
                Modem_State_SwitchOn(MODON_IPSEND);//切换状态机
            }
            return 1;
        }
    }
    return 0;
}
/*
  * @brief  检查是否有短信信息
  * @param  None
  * @retval 有信息返回真，否则返回假
  */
BOOL  Modem_State_CheckSms (void)
{
    if(modapi.wait != MODON_IDLE)
    {
        return (__FALSE);//modem在忙
    }   
    if(modsms.busy != 0)
    {
        return (__FALSE);//正在处理短信业务
    }

    if(modsms.rec)//接收短信
    {
        modsms.busy = 1;
        modapi.wait = MODON_SMSREC;//处理接收短信
        Modem_State_SwitchOn(MODON_SMSREC);
        return (__TRUE);
    }
    else if(modsms.send)//发送短信
    {
        modsms.busy = 1;
        modapi.wait = MODON_SMSSEND;//处理发送短信
        Modem_State_SwitchOn(MODON_SMSSEND);
        return (__TRUE);
    }
    return (__FALSE);
}
/*
  * @brief  检查是否有电话要处理
  * @param  None
  * @retval 有信息返回真，否则返回假
  */
BOOL  Modem_State_CheckPhone (void)
{
    if(modapi.wait != MODON_IDLE)
    {
        return (__FALSE);//modem在忙
    }   
    return (__TRUE);
}
/*
  * @brief  modem状态机检查.该函数在run外运行不收状态机影响
  * @param  None
  * @retval None
  */
void  Modem_State_CheckRun (void) 
{       
    static u32 CsqOutTimeCount;//查询信号质量计数器,用于检查内核运行情况

    if(modem.atack > 10)//连续10条命令不响应则进入异常处理
    {
        modem.atack = 0;
        Modem_State_Switch(MODEM_ERROR);
    }

    if(modem.onsta == MODON_CSQ)
    {
        CsqOutTimeCount = 0;
    }
    else if(modem.onsta == MODON_PHONE)
    {
        CsqOutTimeCount = 0;
    }
    
    if(modem.onsta != MODON_CSQ)
    {
        CsqOutTimeCount++;
        if(CsqOutTimeCount > MODEM_TSEC*1800)
        {
            CsqOutTimeCount = 0;
            Modem_State_Switch(MODEM_ERROR);//连续30分钟不查询信号则进入异常处理
        }
    }
}
/*
  * @brief  检查是是否允许切换状态
  * @param  None
  * @retval 应用层有数据要发送返回真，否则返回假
  */
BOOL  Modem_State_Check (void)
{
    if(modftp.busy)
    {
        return (__FALSE);//modem忙
    }
    Modem_State_CheckSoc();//检查soc连接是否有数据要发送 
    Modem_State_CheckSms();//检查是否有新短信或要发送的短信
    Modem_State_CheckPhone();//检查是否有来电或要拨打的电话
    Modem_State_DelayPro();
    return (__TRUE);
}

/*
  * @brief  modem错误处理
  * @param  staok : 本功能执行成功后切换的状态；
  * @retval None
  */
void  Modem_State_Err (MODEM_STA staok)
{
    ////////////////////////////////////////////////////////////////清除对外标志
    Modem_State_Init();

    ////////////////////////////////////////////////////////////////////切换状态
    Modem_PrtfTo(MODPRTF_ERR);
    Modem_State_Switch(staok);
}

/*
  * @brief  关闭modem,.
  * @param  None 下一状态由函数内部决定
  * @retval None
  */
void  Modem_State_Off (void)
{   
    if(modtts.busy)
    {
        return;//等待播报完毕
    }

    switch (modem.index)
    {
        ////////////////////////////////////////////////////////////////////关机
        case 0:
            {
                Modem_Bsp_SetActive(0);
                Modem_Delay(Modem_ActPinOffDly);
                modem.index++;
            }
            break;
        ////////////////////////////////////////////////////////////////注销网络
        case 1:
            {
                Modem_Bsp_SetActive(1);
                Modem_Delay(15);
                modem.index++;
            }
            break;
        ////////////////////////////////////////////////////////////////////放电
        case 2:
            {                
                Modem_Bsp_SetPower(0);
                Modem_Delay(30);
                modem.index++;
                Modem_PrtfTo(MODPRTF_POWOFF);
            }
            break; 
        ////////////////////////////////////////////////////////判断是否长期关机 
        case 3:
            {                
                if(ModemOffFLag)
                {
                    ModemOffFLag = 0;
                    Modem_State_Switch(MODEM_IDLE);//等待应用开机
                    Modem_PrtfTo(MODPRTF_IDLE);
                    Modem_PrtfTo(MODPRTF_WTAPP);
                }
                else
                {
                    Modem_State_Switch(MODEM_ACT);//立即开机

                }
            }
            break;
    }
}

/*
  * @brief  激活modem,.
  * @param  staok : 本功能执行成功后切换的状态；
  * @param  staerr : .........失败后..........；
  * @retval None
  */
void  Modem_State_Act (MODEM_STA staok,MODEM_STA staerr) 
{   

    switch (modem.index)
    {
        case 0:
            {
                Modem_PrtfTo(MODPRTF_POWON);
                Modem_Bsp_SetPower(1);
                modem.index++;
            }
            break;
        case 1:
            {  
                Modem_Bsp_SetActive(0);
                Modem_Delay(Modem_ActPinOnDly);
                modem.index++;
            }
            break;
        case 2:
            {
                Modem_Bsp_SetActive(1);
                Modem_PrtfTo(MODPRTF_WTAT);
                Modem_Delay(5);
                modem.index++;
            }
            break;               
        case 3:
            {
                Modem_PrtfTo(MODPRTF_SDAT);
                Modem_Send_cmd_Base("AT\r","OK\r",2,30);
            }
            break; 
        case 4://返回正确命令，切换至staok状态
            {
                Modem_State_Switch(staok);
            }
            break;
        default://命令超时，切换至staerr状态
            {
                Modem_State_Switch(staerr);
            }
            break;            
    }
}




/*
  * @brief  获取modem信息,.
  * @param  staok : 本功能执行成功后切换的状态；
  * @param  staerr : .........失败后..........；
  * @retval None
  */
void  Modem_State_Ati (MODEM_STA staok,MODEM_STA staerr)
{
    switch (modem.index)
    {
        case 0://关闭回显
            {
                Modem_Send_cmd_Base("ATE0\r","OK\r",1,3);
            }
            break;      
        case 1://查询信息
            {
                Modem_Send_cmd_Ati(2,5);
            }
            break;
        case 2://等待10S模块就绪
            {
                Modem_Delay(20);
                modem.index++;
                Modem_PrtfTo(MODPRTF_WTRD);
            }
            break;        
        case 3://返回正确命令，切换至staok状态
            {
                Modem_State_Switch(staok);
            }
            break;         
        default://命令超时，切换至staerr状态
            {
                Modem_State_Switch(staerr);
            }
            break; 
    }   
}

/*
  * @brief  设置modem语音参数,.
  * @param  staok : 本功能执行成功后切换的状态；
  * @param  staerr : .........失败后..........；
  * @retval None
  */
void  Modem_State_Aud (MODEM_STA staok,MODEM_STA staerr)
{   
    u8  Tab_t[50];
    u8  value;
    u8  *pCmd;
    u8 tab_value_Q[10]   = {0,10,20,30,50,65,80,90,100,100};
    u8 tab_value_ZTE[10] = {0,1,2,2,3,4,5,6,7,7};      
    
    static u8 index_back;
    
    switch (modem.index)
    {
        case 0://读取参数
            {
                if(Modem_Type == MODEM_TYPE_Q)////////////移远
                {              
                    memcpy(modphone.clvlvalue,tab_value_Q,10);
                }      
                else if(Modem_Type == MODEM_TYPE_ZTE)/////中兴
                {
                    memcpy(modphone.clvlvalue,tab_value_ZTE,10);
                } 
                memcpy(modtts.clvlvalue,modphone.clvlvalue,10);//TTS音量和电话音量公用            
                Modem_Port_ReadAud();
                modem.index++;
            }
            break;    
        case 1://设置来电显示
            {
                Modem_Send_cmd_Base("AT+CLIP=1\r","OK\r",1,3);
            }
            break;
        case 2://设置去电显示
            {
                if(Modem_Type == MODEM_TYPE_Q)////////////移远
                {              
                    Modem_Send_cmd_Base("AT+COLP=1\r","OK\r",1,3);
                }               
                else if(Modem_Type == MODEM_TYPE_ZTE)/////中兴
                {
                    modem.index++;//跳过
                }            
            }
            break;
        case 3://设置默认语音通道
            {
                if(Modem_Type == MODEM_TYPE_Q)////////////移远
                {              
                    pCmd = "AT+QAUDCH=0\r";
                }                
                else if(Modem_Type == MODEM_TYPE_ZTE)/////中兴
                {
                    pCmd = "AT+SPEAKER=1\r";
                }     
                Modem_Send_cmd_Base(pCmd,"OK\r",1,3);
            }
            break;
        case 4://设置默认通道音量大小
            {
                value = modphone.clvlvalue[modphone.clvl];
                sprintf((char*)Tab_t,"AT+CLVL=%d\r",value);

                if(Modem_Type == MODEM_TYPE_ZTE)/////中兴
                {
                    sprintf((char*)Tab_t,"AT+VGR=%d\r",value);
                }   
                pCmd = Tab_t;
                Modem_Send_cmd_Base(pCmd,"OK\r",1,3);
                #ifdef MODEM_DEBUG_H
                if(ModDbInfType == MODDB_BASE)
                {
                    MODDB_PRTFBAS("modem->Phone Clvl = %d\r\n\r\n",value);
                }
                #endif
            }
            break;
        case 5://设置TTS音量大小
            {
                value = modtts.clvlvalue[modtts.clvl];
                if(Modem_Type == MODEM_TYPE_Q)////////////移远
                {         
                    strcpy((char*)Tab_t,"AT\r");
                    if(QuectelAtTtsFlag)
                    {
                        sprintf((char*)Tab_t,"AT+QMEDVL=%d\r",value);
                    }
                }       
                else if(Modem_Type == MODEM_TYPE_ZTE)/////中兴 AT+ZTTSP=3,1,3
                {
                    sprintf((char*)Tab_t,"AT+ZTTSP=%d,1,3\r",value);
                } 
                pCmd = Tab_t;
                Modem_Send_cmd_Base(pCmd,"OK\r",1,3);
                #ifdef MODEM_DEBUG_H
                if(ModDbInfType == MODDB_BASE)
                {
                    MODDB_PRTFBAS("modem->TTS   Clvl = %d\r\n\r\n",value);
                }
                #endif                
            }
            break;            
        case 6://设置默认通道麦克风参数
            {
                if(Modem_Type == MODEM_TYPE_Q)////////////移远
                {              
                    pCmd = "AT+QMIC=0,13\r";
                }                 
                else if(Modem_Type == MODEM_TYPE_ZTE)/////中兴
                {
                    pCmd = "AT+VGT=0\r";
                } 
                Modem_Send_cmd_Base(pCmd,"OK\r",1,3);
            }
            break;
        case 7://设置侧音参数
            {
                if(Modem_Type == MODEM_TYPE_Q)////////////移远
                {              
                    pCmd = "AT+QSIDET=0\r";
                }                
                else if(Modem_Type == MODEM_TYPE_ZTE)/////中兴
                {
                    pCmd = "AT+SIDET=1,0\r";//预留
                } 
                Modem_Send_cmd_Base(pCmd,"OK\r",1,3);
            }
            break;
        case 8://设置回声参数
            {
                if(Modem_Type == MODEM_TYPE_Q)////////////移远
                {              
                    pCmd = "AT+QECHO=221,100,100,374,1\r";
                }                
                else if(Modem_Type == MODEM_TYPE_ZTE)/////中兴
                {
                    pCmd = "AT+ECHO=1\r";
                } 
                Modem_Send_cmd_Base(pCmd,"OK\r",1,3);
            }
            break;
        case 9://设置来电音量
            {
                if(Modem_Type == MODEM_TYPE_Q)////////////移远
                {              
                    pCmd = "AT+CRSL=100\r";
                    Modem_Send_cmd_Base(pCmd,"OK\r",1,3);
                }                
                else if(Modem_Type == MODEM_TYPE_ZTE)/////中兴
                {
                    pCmd = "AT+ZCVF=2\r";
                    Modem_Send_cmd_Base(pCmd,"OK\r",1,3);
                } 
            }
            break;
        case 10://返回正确命令，切换至staok状态
            {
                Modem_State_Switch(staok);
            }
            break;
        default://命令超时，切换至staerr状态
            {
                #if 0
                    Modem_State_Switch(staerr);
                #else
                    modem.index = index_back+1;//命令错误,为保证其兼容性暂时通过,需后续处理
                #endif
            }
            break; 
    }
    index_back = modem.index;
}

/*
  * @brief  查询modem网络情况.
  * @param  staok : 本功能执行成功后切换的状态；
  * @param  staerr : .........失败后..........；
  * @retval None
  */
void  Modem_State_NetSer (MODEM_STA staok,MODEM_STA staerr)
{   
    switch (modem.index)
    {
        case 0://检查SIM卡
            {
                Modem_PrtfTo(MODPRTF_CPIN);
                Modem_Send_cmd_Search("AT+CPIN?\r",10,5);//为了兼容更多SIM卡必须这么长时间
            }
            break;
        case 1://查询CCID
            {
                if(Modem_Type == MODEM_TYPE_Q)////////////移远
                {              
                    Modem_Send_cmd_CCID("AT+QCCID\r",1,3);
                }                
                else if(Modem_Type == MODEM_TYPE_ZTE)/////中兴
                {
                    Modem_Send_cmd_CCID("AT+GETICCID\r",1,3);
                }
                Modem_PrtfTo(MODPRTF_CCID);
            }
            break;   
        case 2://查询网络注册情况
            {
                Modem_PrtfTo(MODPRTF_CREG);
                Modem_Send_cmd_Search("AT+CREG?\r",3,10);
            }
            break;
        case 3://查询网络质量
            {
                Modem_Send_cmd_Search("AT+CSQ\r",5,15);                               
            }
            break;
        case 4://///////////////////////////////////////////////////////////////////中兴在此结束
            {
                modem.index++;
                if(Modem_Type == MODEM_TYPE_ZTE)/////中兴
                {
                    Modem_State_Switch(staok);
                }
            }
            break;
        case 5://查询GPRS网络注册情况
            {
                Modem_PrtfTo(MODPRTF_CGREG);
                Modem_Send_cmd_Search("AT+CGREG?\r",3,10);
            }
            break;
        case 6://查询网络附着注册情况
            {
                Modem_Send_cmd_Base("AT+CGATT?\r","OK\r",3,10);
            }
            break;           
        case 7://返回正确命令，切换至staok状态
            {
                Modem_State_Switch(staok);
            }
            break;
        default://命令超时，切换至staerr状态
            {
                Modem_State_Switch(staerr);
            }
            break; 
    }
}
/*
  * @brief  配置网络环境.
  * @param  staok : 本功能执行成功后切换的状态；
  * @param  staerr : .........失败后..........；
  * @retval None
  */
void  Modem_State_NetCfg (MODEM_STA staok,MODEM_STA staerr)
{       
    u8 *pCmd;
    u8 Tab_t[50];
    
    static u8 index_back;
    
    switch (modem.index)
    {
        case 0://读取参数
            {
                memset(modem.user,0,sizeof(modem.user));
                memset(modem.pass,0,sizeof(modem.pass));

                if(Modem_Type == MODEM_TYPE_Q)
                {              
                    strcpy((char*)modem.apn,"cmnet");//G网默认公网
                }
                else if(Modem_Type == MODEM_TYPE_ZTE)
                {
                    strcpy((char*)modem.user,"card");//默认值
                    strcpy((char*)modem.pass,"card");
                }
                Modem_Port_ReadUserPass();//读取账号密码
                Modem_Port_ReadApn();     //读取apn
                Modem_Port_ReadSleep();   //读取休眠值 
                modem.index++;
            }
            break;
        case 1://设置APN
            {
                Modem_Send_cmd_Apn(1,3);
            }
            break;
        case 2://设置tcpnet场景
            {
                if(Modem_Type == MODEM_TYPE_Q)////////////移远
                {              
                    pCmd = "AT+QIFGCNT=0\r";//设置场景
                }                
                else if(Modem_Type == MODEM_TYPE_ZTE)/////中兴
                {
                    Modem_PrtfTo(MODPRTF_CPPP);
                    pCmd = "AT+ZPPPSTATUS\r";//查询PPP状态
                } 
                Modem_Send_cmd_Base(pCmd,"OK\r",1,3);
            }
            break;
        case 3://设置休眠
            {   
                modem.index++;
                if(Modem_Type == MODEM_TYPE_ZTE)/////中兴
                {
                    modem.index--;
                    sprintf((char*)Tab_t,"AT+CTA=%d\r",modem.sleep);
                    Modem_Send_cmd_Base(Tab_t,"OK\r",1,3);
                }
            }
            break;             
        case 4://设置多连接
            {               
                if(Modem_Type == MODEM_TYPE_Q)////////////移远
                {              
                    Modem_Send_cmd_Base("AT+QIMUX=1\r","OK\r",1,3);
                }
                else if(Modem_Type == MODEM_TYPE_ZTE)/////中兴
                {
                    Modem_State_Switch(staok);  
                } 
            }
            break;
        ////////////////////////////////////////////////////////////移远专用命令
        case 5://设置非透传模式
            {
                Modem_Send_cmd_Base("AT+QIMODE=0\r","OK\r",1,3);  
            }
            break;
        case 6://先关闭场景，确保开始连接前状态是正确的
            {
                Modem_Send_cmd_Base("AT+QIDEACT\r","DEACT OK\r",3,5);
            }
            break;
        case 7://AT+QISEND操作后,发送成功时不显示―>，返回―SEND OK
            {
                Modem_Send_cmd_Base("AT+QIPROMPT=0\r","OK\r",3,5);                
            }
            break;
        case 8://返回正确命令，切换至staok状态
            {
                Modem_State_Switch(staok);
            }
            break;
        default://命令超时，切换至staerr状态
            {
                modem.index = index_back+1;//为了避免版本差异,设置类命令强制转为成功
            }
            break; 
    } 
    index_back = modem.index;
}

/*
  * @brief  设置短信存储器.
  * @param  staok : 本功能执行成功后切换的状态；
  * @param  staerr : .........失败后..........；
  * @retval None
  */
void  Modem_State_Sms (MODEM_STA staok,MODEM_STA staerr)
{       
    u8 *pCmd;
    
    static u8 index_back;
    
    switch (modem.index)
    {
        case 0://查询短信中心号码
            {
                if(Modem_Type == MODEM_TYPE_Q)////////////移远
                {              
                    Modem_Send_cmd_Search("AT+CSCA?\r",10,15);//为了兼容更多SIM卡必须这么长时间
                }                 
                else if(Modem_Type == MODEM_TYPE_ZTE)/////中兴
                {
                    modem.index++;//跳过
                }
                Modem_PrtfTo(MODPRTF_CSCA);
            }
            break;     
        case 1://设置短信存储器  
            {              
                if(Modem_Type == MODEM_TYPE_ZTE)/////中兴
                {
                    pCmd = "AT+CPMS=\"MT\",\"MO\"\r";
                } 
                else
                {
                    pCmd = "AT+CPMS=\"SM\",\"SM\",\"SM\"\r";
                }
                Modem_PrtfTo(MODPRTF_CPMS);
                Modem_Send_cmd_Base(pCmd,"OK\r",10,15);//为了兼容更多SIM卡必须这么长时间
            }
            break;  
        case 2://设置短信提示符 
            {              
                if(Modem_Type == MODEM_TYPE_ZTE)/////中兴
                {
                    pCmd = "AT\r";
                } 
                else
                {
                    pCmd = "AT+CNMI=2,1\r";
                }
                Modem_PrtfTo(MODPRTF_CNMI);
                Modem_Send_cmd_Base(pCmd,"OK\r",10,15);//为了兼容更多SIM卡必须这么长时间
            }
            break;            
        case 3://返回正确命令，切换至staok状态
            {
                Modem_State_Switch(staok);
            }
            break;
        default://命令超时，切换至staerr状态
            {
                modem.index = index_back+1;//为了多兼容SIM卡,短信处理强制转为成功
            }
            break; 
    }
    index_back = modem.index;
}
/*
  * @brief  拨号上网.
  * @param  staok : 本功能执行成功后切换的状态；
  * @param  staerr : .........失败后..........；
  * @retval None
  */
void  Modem_State_Dial (MODEM_STA staok,MODEM_STA staerr)
{       
    u8  *pCmd,*pRec;
    u8  outtime;
    
    switch (modem.index)
    {
        case 0://PPP拨号
            {
                Modem_PrtfTo(MODPRTF_DPPP);
                if(Modem_Type == MODEM_TYPE_Q)////////////移远
                {              
                    pCmd = "AT+QIREGAPP\r";
                    pRec = "OK\r";
                }                  
                else if(Modem_Type == MODEM_TYPE_ZTE)/////中兴
                {
                    pCmd = "AT+ZPPPOPEN\r";
                    pRec = "+ZPPPSTATUS: OPENED";
                }
                Modem_Send_cmd_Base(pCmd,pRec,30,5);
            }
            break;
        case 1://激活场景
            {
                modem.index++;
                if(Modem_Type == MODEM_TYPE_Q)////////////移远
                {    
                    modem.index--;
                    Modem_Send_cmd_Base("AT+QIACT\r","OK\r",8,5);
                }
            }
            break;
        case 2://查询基站分配的IP
            {
                Modem_PrtfTo(MODPRTF_GIP);
                if(Modem_Type == MODEM_TYPE_Q)////////////移远
                {              
                    pCmd = "AT+QILOCIP\r";
                    outtime = 3;                    
                }               
                else if(Modem_Type == MODEM_TYPE_ZTE)/////中兴
                {
                    pCmd = "AT+ZIPGETIP\r";
                    outtime = 3;
                } 
                Modem_Send_cmd_IpLoc(pCmd,outtime,3);
            }
            break;
        case 3://返回正确命令，切换至staok状态
            {
                Modem_State_Switch(staok);
                Modem_PrtfTo(MODPRTF_ONLN);
            }
            break;
        default://命令超时，切换至staerr状态
            {
                Modem_State_Switch(staerr);
            }
            break;             
    }  
}

/*
  * @brief  组装发送的短信数据
  * @param  *pNum :指向手机号码,如果为NULL则使用接收号码
  * @param  *pData :指向短信数据内容
  * @param  len:短信数据的长度,一次只能发送(MODEM_SMS_SENDMAX)个字节，暂不支持短信拆包
  * @retval 返回处理结果
  */
u8  Modem_State_Sms_Dt(MODSMS_TYPE Type,u8 *pNum, u8 *pData, u8 len)
{
    u8  *pSendNum=modsms.num;
    
    len = strlen((char*)pData);//过滤伪0

    if(pNum != NULL)
    {
        strcpy((char*)modsms.num,(char*)pNum);
    } 
    else
    {
        if(strlen((char*)modsms.num) == 0)
        {
            return 0;//非法号码
        }
    }
    #ifdef MODEM_DEBUG_H
    if(ModDbInfType == MODDB_BASE)
    {
        MODDB_PRTFBAS("modem-> SMS=%s, Type=%d, Snd_Len=%d\r\n",modsms.num,Type,len);
        MODDB_PRTFBAS("%s",pData);
        MODDB_PRTFBAS("\r\n");
    }
    #endif 

    if(len > 140)
    {
        return 0;
    }
    memset(modsms.dbuf,0,MODEM_SMS_SENDMAX);
    ////////////////////////////////////////////////////////////////////号码处理
    if(Type == SMS_PDU)
    { 
        memset((u8*)&modsmspdu,0,sizeof(SM_PARAM));
        strcpy((char*)modsmspdu.SCA,(char*)modem.csca); //短信中心号码  
        strcpy((char*)modsmspdu.TPA,(char*)pSendNum);   //短信发送号码  

        memcpy(modsmspdu.TP_UD,pData,len);  
        modsmspdu.udLen = len;

        modsms.dlen   = ModLib_PduEncode(&modsmspdu,modsms.dbuf);
        modsms.pdulen = modsmspdu.PDU_LN;
    }
    else if(Type == SMS_TXT)
    {
        memcpy(modsms.dbuf,pData,len);
        modsms.dlen = len;           
    }
    else
    {
        return 0;//非法短信类型
    }
    ////////////////////////////////////////////////////////////////////数据处理
    modsms.send = 1;
    modsms.typs = Type;//发送短信模式单独控制
    return (__TRUE);

}
/*
  * @brief  短信业务，接收
  * @param  staok : 本功能执行成功后切换的状态；
  * @param  staerr : .........失败后..........；
  * @retval None
  */
void  Modem_State_Sms_Rec (MODON_STA staok,MODON_STA staerr)
{       
    u8 *pCmd;
    u8 Tab_t[30];

    switch (modem.index)
    {
        ////////////////////////////////////////////////////////////////////////
        case 0://设置短信格式
            {
                Modem_Send_cmd_Sms(0,3,2);//设置格式,超时3秒,重试2次
            }
            break;
        case 1://设置字符集
            {
                Modem_Send_cmd_Sms(1,3,2);//设置字符集,超时3秒,重试2次
            }
            break;  
        case 2://查询条数
            {
                Modem_Send_cmd_Sms(3,3,2);//查询个数,超时3秒,重试2次
            }
            break;  
        case 3://读取信息
            {
                strcpy((char*)Tab_t,"AT+CMGR=1\r");
                if(Modem_Type == MODEM_TYPE_ZTE)//中兴
                {
                    strcpy((char*)Tab_t,"AT+CMGR=0\r");    
                }                
                pCmd = Tab_t; 
                Modem_Send_cmd(MODWT_SMS,pCmd,NULL,2,1);
            }
            break; 
        case 4://删除信息
            {
                Modem_Send_cmd_Sms(4,3,2);//删除短信,超时3秒,重试2次
            }
            break; 
        case 5://返回正确命令，切换至staok状态
            {
                modsms.busy = 0;
                modsms.rec  = 0;
                Modem_State_SwitchOn(staok);  
            }
            break;
        default://命令超时，切换至staerr状态
            {
                modsms.busy = 0;
                modsms.rec  = 0;
                Modem_State_SwitchOn(staok);  
            }
            break; 
    }

}

/*
  * @brief  短信业务,发送
  * @param  staok : 本功能执行成功后切换的状态；
  * @param  staerr : .........失败后..........；
  * @retval None
  */
void  Modem_State_Sms_Send (MODON_STA staok,MODON_STA staerr)
{       
    switch (modem.index)
    {
        ////////////////////////////////////////////////////////////////////////
        case 0://设置短信格式
            {
                Modem_Send_cmd_Sms(0,3,2);//设置格式,超时3秒,重试2次
            }
            break;
        case 1://设置字符集
            {
                Modem_Send_cmd_Sms(1,3,2);//设置字符集,超时3秒,重试2次
            }
            break;            
        case 2://发送信息
            {
                Modem_Send_cmd_Sms(2,0,0);//发送信息,立即发送
            }
            break;
        case 3://50ms等待">"符
            {
                modem.index++;
            }
            break;             
        case 4://发送具体数据
            {
                modsms.dbuf[modsms.dlen++]=0x1a;
                Modem_Uart_Send(modsms.dbuf,modsms.dlen);
                Modem_Send_cmd(MODWT_SMS,NULL,NULL,10,1);//10s超时等待ok 
                modem.index++;               
            }
            break;   
        case 5://等待
            {
                ;//重发机制提供超时
            }
            break;            
        case 6://返回正确命令，切换至staok状态
            {
                modsms.busy = 0;
                modsms.send = 0;
                Modem_State_SwitchOn(staok); 
                #ifdef MODEM_DEBUG_H
                if(ModDbInfType == MODDB_BASE)
                {
                    MODDB_PRTFBAS("modem-> SMS Send Ok!!!\r\n\r\n");
                }
                #endif                 
            }
            break;
        default://命令超时，切换至staerr状态
            {
                modsms.busy = 0;
                modsms.send = 0;
                Modem_State_SwitchOn(staerr);
                #ifdef MODEM_DEBUG_H
                if(ModDbInfType == MODDB_BASE)
                {
                    MODDB_PRTFBAS("modem-> SMS Send Err!!!\r\n\r\n");
                }
                #endif                    
            }
            break;
    }

}

/*
  * @brief  查询指定soc链接状态
  * @param  Soc号
  * @retval 返回链接状态,在线返回真，否则返回假
  */
BOOL  Modem_State_SocSta (u8 soc) 
{  
    if((soc < MODEM_SOC_NUM)&&(Modem_IpSock_STA[soc] == MODSOC_ONLN))
    {
        return (__TRUE);
    }
    return (__FALSE);   
}
/*
  * @brief  查询modem连接状态、CSQ，如果失败则重启modem
  * @param  state:本状态执行完毕后的下个状态
  * @retval None
  */
void  Modem_State_CSQ (MODON_STA state) 
{
    switch (modem.index)
    {
        case 0://查询连接
            {
                Modem_Send_cmd_Search("AT+CSQ\r",1,1); 
            }
            break;
        case 1://发送查询命令
            {
                Modem_Send_cmd_IpSta(1,3);
            }
            break;
        case 2://检查SIM卡
            {
                Modem_Send_cmd_Search("AT+CPIN?\r",3,5);
            }
            break;
        case 3:
            {
                Modem_State_SwitchOn(state);
            }
            break;
        default://指令返回错误
            {
                Modem_State_Switch(MODEM_ERROR);//进入异常处理
            }
            break;
    }
}

/*
  * @brief  配置连接为域名连接
  * @param  dnsis : 0为IP连接，非0为域名连接
  * @param  staok : 本功能执行成功后切换的状态；
  * @param  staerr : .........失败后..........；
  * @retval None
  */
void  Modem_State_DNSIP (MODON_STA state)
{       
    u8 *pCmd;
    switch (modem.index)
    {
        case 0://发送域名解析命令
            {
                if(Modem_Type == MODEM_TYPE_Q)////////////移远
                {                                      
                    if(modem.socdns == 0)
                    {
                        pCmd = "AT+QIDNSIP=0\r";//ip连接
                    }
                    else
                    {
                        pCmd = "AT+QIDNSIP=1\r";//域名连接                    
                    }
                    Modem_Send_cmd_Base(pCmd,"OK\r",3,3); 
                }
                else if(Modem_Type == MODEM_TYPE_ZTE)/////中兴
                {
                    modem.index++;//跳过
                } 
            }
            break;
        case 1://返回正确命令，切换至staok状态
            {
                Modem_State_SwitchOn(state);
            }
            break;
        default://命令超时，切换至staerr状态
            {
                Modem_State_SwitchOn(state);
            }
            break; 
    }
}

/*
  * @brief  打开一个连接.
  * @param  state:本状态执行完毕后的下个状态
  * @retval None
  */
void  Modem_State_Ip_Open (MODON_STA state ,u8 SocNum)
{

    switch (modem.index)     
    {
        case 0://查询连接状态
            {      
                Modem_Send_cmd_IpSta(1,3);
            }
            break;  
        case 1://判断是否需要关闭连接
            {      
                if(Modem_IpSock_STA[SocNum] == MODSOC_IDLE)
                {
                    modem.index = 3;//直接打开
                }
                else
                {
                    modem.index = 2;//先关闭后打开
                }
            }
            break;
        case 2://关闭指定连接
            {  
                Modem_Send_cmd_IpClose(SocNum,3,3);
            }
            break;
        case 3://打开指定连接
            {      
                Modem_Send_cmd_IpOpen(SocNum,3);
                Modem_PrtfTo(MODPRTF_SOCWT);
            }
            break;
        case 4://成功
            {  
                modInf.Num_SocLn[SocNum]++;//连接次数计数
                Modem_IpSock_STA[SocNum] = MODSOC_WAIT;//等待服务器应答
                Modem_State_SwitchOn(state);
            }
            break;
        default://失败
            {
                modInf.Num_SocLn[SocNum]++;//连接次数计数
                Modem_IpSock_STA[SocNum] = MODSOC_IDLE;//转为空闲
                Modem_State_SwitchOn(state);
            }
            break;
    }   
}

/*
  * @brief  关闭指定Soc连接.
  * @param  state:本状态执行完毕后的下个状态
  * @param  index:Soc号
  * @retval None
  */
void  Modem_State_Ip_Close (MODON_STA state ,u8 SocNum) 
{
    static u8 closenum;
    
    switch (modem.index)
    {

        case 0://查询连接状态
            {      
                Modem_Send_cmd_IpSta(1,3);
            }
            break;
            
        case 1://强制从0开始关闭全部
            {      
                closenum = SocNum;
                
                if(closenum >= MODEM_SOC_NUM)
                {
                    closenum = 0;
                }
                modem.index++;
            }
            break;
        case 2://判断是否需要关闭连接
            {      
                if(Modem_IpSock_STA[closenum] == MODSOC_IDLE)
                {
                    modem.index += 2;//直接跳过 无需重复关闭
                }
                else
                {
                    modem.index++;//执行关闭动作
                }
            }
            break;
        case 3:
            {    
                Modem_Send_cmd_IpClose(closenum,3,3);
            }
            break;
        
        case 4:
            {
                Modem_IpSock_STA[closenum] = MODSOC_IDLE;//该sock成功下线

                if(SocNum >= MODEM_SOC_NUM)
                {
                    if((++closenum) < MODEM_SOC_NUM)
                    {
                        modem.index = 2;
                        break;
                    }
                }
                Modem_State_SwitchOn(state);
            }
            break;
            
        default://指令返回错误
            {
                Modem_State_SwitchOn(state);
            }
            break;
    }
}

/*
  * @brief  发送数据到指定Soc连接.
  * @param  state:本状态执行完毕后的下个状态
  * @param  Outtime: 等待SEND OK 超时时间,单位s.
  * @param  Try: 重试次数
  * @retval None
  */
void  Modem_State_Ip_Send (MODON_STA state ,u8 Outtime,u8 Try) 
{
    u8 SocNum = soc_atbuf.soc;
    u8 *pSend = soc_atbuf.buf;
    u16 len   = soc_atbuf.len;

    static u16 timecnt;
    static u8  trycnt;
    
    switch(modem.index)
    {
        case 0://发送命令
            {
                modtts.wait = 1;// lock
                Modem_Send_cmd_IpData(SocNum,len,3);
                trycnt++;
            }
            break;
        case 1://发送具体数据
            {
                Modem_Uart_Send(pSend,len);
                Modem_Send_cmd(MODWT_SEDATA,NULL,NULL,Outtime,1);//5s超时等待send ok 
                modem.index++;
                timecnt = 0;
                #ifdef MODEM_DEBUG_H
                u8  buf[610];
                if(ModDbInfType == MODDB_BASE)
                {
                    MODDB_PRTFBAS("modem-> Soc = %d   Snd_Len = %04d\r\n",SocNum,len);
                    if(len > 200)
                    {
                        len = 200;
                    }
                    memset(buf,0,600);
                    ModLib_ByteStrEx(buf,pSend,len);
                    MODDB_PRTFBAS("%s",buf);
                    MODDB_PRTFBAS("\r\n");
                }
                #endif
            }
            break;
        case 2://等待结果 
            {
                if((timecnt++) > Outtime);
                {
                    timecnt = 0;
                    modem.index++;//重发机制超时失败后使用该值
                }
            }
            break;
        case 3://检查是否还有数据发送
            {              
                trycnt      = 0;
                modtts.wait = 0;
                if(Modem_State_CheckSoc()==0)
                {
                    modem.index++;
                }
            } 
            break;
        case 4://所有数据发送成功
            {              
                Modem_State_SwitchOn(state);
            } 
            break;            
        default://发送数据失败
            {
                modtts.wait = 0;// unlock
                if(trycnt < Try)
                {
                    modem.index = 0;//重发
                    break;
                }
                trycnt = 0;
                Modem_State_SwitchOn(MODON_IPCLOSE);//链路异常 关闭该链路
                modem.socclose = SocNum;
            }
            break;
    }
}

/*
  * @brief  Ftp业务
  * @param  state:本状态执行完毕后的下个状态
  * @retval None
  */
void  Modem_State_Ftp (MODON_STA state)
{
    switch(modem.index)
    {
        ////////////////////////////////////////////////////////////////////////登录
        case 0://设置账号 (中兴设置IP、端口、账号、密码)
            {
                Modem_Send_cmd_FtpLogin(0,30,3);
            }
            break;
        case 1://中兴特殊处理
            {
                modem.index++;
                if(Modem_Type == MODEM_TYPE_ZTE)//中兴
                {
                    modem.index = 4;//成功登录跳到下载状态
                    modem.delay = MODEM_TSEC*3;//需延时3秒否则会卡机
                }
            }
            break;
        case 2://设置密码
            {
                Modem_Send_cmd_FtpLogin(1,3,3);
            }
            break;
        case 3://设置IP、端口
            {
                Modem_Send_cmd_FtpLogin(2,30,3);
            }
            break;
        ////////////////////////////////////////////////////////////////////////打开文件
        case 4://设置路径
            {
                Modem_Send_cmd_FtpDown(0,30,3);
            }
            break;
        case 5://中兴特殊处理
            {
                modem.index++;
                if(Modem_Type == MODEM_TYPE_ZTE)//中兴
                {
                    modem.index  = 7;//成功登录跳到下载状态
                    modftp.busy  = 1;
                    modftp.fend  = 0;
                    modftp.fres  = 0;
                    modftp.fsize = 0;                    
                }
            }
            break;
        case 6://设置文件名
            {
                Modem_Send_cmd_FtpDown(1,30,3);
            }
            break;
        ////////////////////////////////////////////////////////////////////////下载中
        case 7://标志位处理
            {
                modem.wait = MODWT_FTP;//超时处理
                modem.delay = MODEM_TSEC*60;
                modat.trynum = 0;
                modem.index++;
            }
            break;         
        case 8://
            {
                if(modftp.fres)//等待下载完毕
                {
                    modem.index++;
                }
            }
            break; 
        case 9://处理完毕
            {
                modftp.state = 0;//空闲   
                Modem_State_SwitchOn(state);
            }
            break;                         
        default://发送数据失败
            {
                modftp.state = 0;//空闲   
                Modem_State_SwitchOn(state);
            }
            break;
    }
}
/*
  * @brief  电话业务->进入电话状态,主要用于来电
  * @param  None
  * @retval None
  */
void  Modem_State_Phone_Enter(void)
{
    
    if(modem.state != MODEM_ON)
    {
        return ;//下线状态
    }

    if(modem.onsta == MODON_IPOPEN)
    {
        Modem_State_Push();
        modphone.otherflg = 1;
    }
    else if(modem.onsta != MODON_IDLE)
    {
        return ;//正在处理其他事情
    }
    modphone.busy = 1;
    modphone.state= MODPH_RING;
    modapi.wait = MODON_PHONE;     
    Modem_State_SwitchOn(MODON_PHONE);

}
/*
  * @brief  电话业务->退出电话状态,主要用于挂断电话
  * @param  None
  * @retval None
  */
void  Modem_State_Phone_Break(void)
{
    Modem_Bsp_SpkOff(BSP_SPK_TALK);
    Modem_Bsp_MicOff();
    modphone.busy = 0;
    modphone.state= MODPH_IDLE;
    Modem_Api_CallPhoneStatus(modphone.num,0);//通知外部

    if(modphone.otherflg)
    {
        modphone.otherflg = 0;
        Modem_State_Pop();
    }
    else
    {
        if(ModemPhoneTm > PHONE_TIME_CLOSE)//连续通话则关闭所有链路
        {
            modem.socclose = MODEM_SOC_NUM;//关闭所有链路
            Modem_State_SwitchOn(MODON_IPCLOSE);
        }
        else
        {
            Modem_State_SwitchOn(MODON_IDLE);//退出电话模式  
        }
        ModemPhoneTm = 0;
    }
}
/*
  * @brief  电话业务->拨打电话
  * @param  state:本状态执行完毕后的下个状态
  * @retval None
  */
void  Modem_State_Phone_Atd(MODPH_STA stat)
{
    u8 tab[30];

    switch(modem.index)
    {      
        case 0://拨打电话
            {          
                sprintf((char*)tab,"ATD%s\r",modphone.num);//电话号码
                Modem_Uart_Send(tab,strlen((char*)tab));
                Modem_Bsp_MicOff();
                modem.index++;
                ModemAtdOutTime = 0;
                #ifdef MODEM_DEBUG_H
                if(ModDbInfType == MODDB_BASE)
                {
                    MODDB_PRTFBAS("modem->%s\r\n",tab);
                }
                #endif                  
            }
            break; 
        case 1://等待对方接听
            {
                if((ModemAtdOutTime++)>(90*MODEM_TSEC))
                {
                    modem.index = 0xaa;
                    ModemPhoneTm = 0;//对方无应答归零计数器
                }
            }
            break;  
        case 2://针对MC8332通话中啸叫所采取的设置命令
            {
                if(Modem_Type == MODEM_TYPE_Q)////////////移远
                {              
                    modem.index++;
                }                
                else if(Modem_Type == MODEM_TYPE_ZTE)/////中兴
                {
                    Modem_Send_cmd_Base("AT+SIDET=0,1\r","OK\r",3,3);
                    //modem.index++; 
                }                 
            }
            break;            
        case 3://拨打电话成功
            {
                ModemPhoneTm = 0;//对方接听后归零计数器
                Modem_Bsp_MicOn();
                Modem_State_SwitchPhone(stat);
                Modem_Api_CallPhoneStatus(modphone.num,1);
                #ifdef MODEM_DEBUG_H
                if(ModDbInfType == MODDB_BASE)
                {
                    MODDB_PRTFBAS("modem->ATD OK!!!\r\n\r\n");
                }
                #endif                
            }
            break;            
        default://拨打电话失败
            {
                Modem_State_SwitchPhone(MODPH_ERR);
            }
            break;
    }
    
}
/*
  * @brief  电话业务->接听电话
  * @param  state:本状态执行完毕后的下个状态
  * @retval None
  */
void  Modem_State_Phone_Ata(MODPH_STA state)
{
    switch(modem.index)
    {    
        case 0://接听电话
            {
                #ifdef MODEM_DEBUG_H
                if(ModDbInfType == MODDB_BASE)
                {
                    MODDB_PRTFBAS("modem->接听...\r\n\r\n");
                }
                #endif             
                Modem_Send_cmd_Base("ATA\r","OK\r",3,3);
            }
            break;       
        case 1://接听电话成功
            {
                Modem_Bsp_MicOn();
                modphone.state= state; 
                ModemPhoneTm = 0;//通话前归零计数器
                Modem_Api_CallPhoneStatus(modphone.num,1);//通知外部已接通
            }
            break;            
        default://接听电话失败
            {
                Modem_State_SwitchPhone(MODPH_ERR);
            }
            break;
    }
}
/*
  * @brief  电话业务->挂断电话
  * @param  state:本状态执行完毕后的下个状态
  * @param  onstate:退出电话业务后切换状态
  * @retval None
  */
void  Modem_State_Phone_Ath(void)
{
    static u8 atherrcnt;
    
    switch(modem.index)
    {
        case 0://拨打电话
            {
                #ifdef MODEM_DEBUG_H
                if(ModDbInfType == MODDB_BASE)
                {
                    MODDB_PRTFBAS("modem->挂断...\r\n\r\n");
                }
                #endif              
                atherrcnt++;
                Modem_Send_cmd_Base("ATH\r","OK\r",3,3);
            }
            break;       
        case 1://挂断电话成功
            {
                atherrcnt = 0;
                Modem_State_Phone_Break();
            }
            break;            
        default://挂断电话失败
            {
                if(atherrcnt < 3)
                {
                    Modem_State_SwitchPhone(MODPH_ERR);
                }
                else
                {
                    atherrcnt = 0;
                    modem.index = 1;//强制挂断
                }
            }
            break;
    }
}
/*
  * @brief  电话业务->声道切换
  * @param  state:本状态执行完毕后的下个状态
  * @retval None
  */
void  Modem_State_Phone_Audch(MODPH_STA state)
{
    u8 tab[30]={0};

    if(Modem_Type == MODEM_TYPE_Q)////////////移远
    {              
        memcpy(tab,"AT+QAUDCH=",10);
        sprintf((char*)tab+10,"%d",modphone.audch);
        strcat((char*)tab,"\r");
    }      
    else if(Modem_Type == MODEM_TYPE_ZTE)/////中兴
    {
        memcpy(tab,"AT+SPEAKER=",11);
        sprintf((char*)tab+11,"%d",modphone.audch);
        strcat((char*)tab,"\r");
    } 
    switch(modem.index)
    {
        case 0://拨打电话
            {
                Modem_Send_cmd_Base(tab,"OK\r",3,3);
            }
            break;             
        case 1://拨打电话成功
            {
                Modem_State_SwitchPhone(state);//    
            }
            break;            
        default://拨打电话失败
            {
                Modem_State_SwitchPhone(MODPH_ERR);
            }
            break;
    }
}
/*
  * @brief  电话业务->音量控制
  * @param  state:本状态执行完毕后的下个状态
  * @retval None
  */
void  Modem_State_Phone_Clvl(MODPH_STA state)
{
    u8 tab[30]={0};
    u8 value = modphone.clvlvalue[modphone.clvl];

    switch(modem.index)
    {
        case 0://调节音量
            {
                #ifdef MODEM_DEBUG_H
                if(ModDbInfType == MODDB_BASE)
                {
                    MODDB_PRTFBAS("modem->Phone Clvl=%02d\r\n",value);
                }
                #endif            
                if(Modem_Type == MODEM_TYPE_Q)////////////移远
                {              
                    sprintf((char*)tab,"AT+CLVL=%d\r",value);
                }     
                else if(Modem_Type == MODEM_TYPE_ZTE)/////中兴
                {
                    sprintf((char*)tab,"AT+VGR=%d\r",value);        
                } 
                Modem_Send_cmd_Base(tab,"OK\r",3,3);
            }
            break;             
        case 1://成功
            {
                Modem_State_SwitchPhone(state);//    
            }
            break;            
        default://失败
            {
                Modem_State_SwitchPhone(MODPH_ERR);
            }
            break;
    }
}
/*
  * @brief  电话业务->根据语音提示输入键值控制
  * @param  state:本状态执行完毕后的下个状态
  * @retval None
  */
void  Modem_State_Phone_Vts(MODPH_STA state)
{
    char str_t[30];

    switch(modem.index)
    {
        case 0://设置按键
            {
                #ifdef MODEM_DEBUG_H
                if(ModDbInfType == MODDB_BASE)
                {
                    MODDB_PRTFBAS("modem->Phone Vts=%s\r\n",modphone.vts);
                }
                #endif                   
                sprintf(str_t,"AT+VTS=%s\r\n",modphone.vts);
                Modem_Send_cmd_Base((u8*)str_t,"OK\r",3,3);
            }
            break;             
        case 1://设置按键成功
            {
                Modem_State_SwitchPhone(state);//    
            }
            break;            
        default://设置按键失败
            {
                Modem_State_SwitchPhone(MODPH_ERR);
            }
            break;
    }
}
/*
  * @brief  电话业务->来电处理，主要把来电号码传递到外部,等待外部操作
  * @param  None
  * @retval None
  */
void  Modem_State_Phone_Ring(void)
{
    switch(modem.index)
    {
        case 0://针对MC8332通话中啸叫所采取的设置命令
            {
                if(Modem_Type == MODEM_TYPE_Q)////////////移远
                {              
                    modem.index++;
                }                  
                else if(Modem_Type == MODEM_TYPE_ZTE)/////中兴
                {
                    //Modem_Send_cmd_Base("AT+VGT=0\r","OK\r",3,3);
                    modem.index++;  
                }  
            }    
        case 1://判断来电号码
            {
                if(Modem_Api_CallPhoneStatus(modphone.num,2))
                {
                    modem.index++;
                    Modem_Bsp_SpkOn(BSP_SPK_TALK);
                }
            }
            break;                  
        case 2://不做任何等待应用层处理
            {
                 ;
            }
            break; 
        default://
            {
                Modem_State_SwitchPhone(MODPH_ERR);
            }
            break;
    }  

}
/*
  * @brief  电话业务->空闲模式，即正在通话中
  * @param  None
  * @retval None
  */
void  Modem_State_Phone_Idle(void)
{
    ModemPhoneTm++;//通话中

    if(ModemPhoneTm > PHONE_TIME_CLOSE)//连续通话则关闭所有链路标志
    {
        if(Modem_IpSock_STA[0] == MODSOC_ONLN)//判断主连接是否在线
        {
            //暂存Modem_IpSock_STA整个标志，用于关闭连接时使用
            memcpy(Modem_Phone_SocSta,Modem_IpSock_STA,sizeof(Modem_IpSock_STA));

            //清除所有在线标志
            memset(Modem_IpSock_STA,MODSOC_IDLE,sizeof(Modem_IpSock_STA));
        }
    }   
}
/*
  * @brief  电话业务处理
  * @param  state:本状态执行完毕后的下个状态
  * @retval None
  */
void  Modem_State_Phone_Pro(MODON_STA state)
{
    switch(modphone.state)
    {
        case MODPH_IDLE:
            {
                Modem_State_Phone_Idle();
            }
            break;
        case MODPH_ATD:
            {
                Modem_State_Phone_Atd(MODPH_IDLE);
            }
            break;
        case MODPH_ATA:
            {
                Modem_State_Phone_Ata(MODPH_IDLE);
            }
            break;
        case MODPH_RING:
            {
                Modem_State_Phone_Ring();
            }
            break; 
        case MODPH_ATH:
            {
                Modem_State_Phone_Ath();
            }
            break;            
        case MODPH_AUDCH:
            {
                Modem_State_Phone_Audch(MODPH_IDLE);
            }
            break;
        case MODPH_CLVL:
            {
                Modem_State_Phone_Clvl(MODPH_IDLE);
            }
            break;       
        case MODPH_VTS:
            {
                Modem_State_Phone_Vts(MODPH_IDLE);
            }
            break;  
        case MODPH_ERR:
            {
                Modem_Bsp_MicOff();
                Modem_State_SwitchPhone(MODPH_ATH);
            }
            break;            
            
    }

}
/*
  * @brief  modem状态机中的TTS,不装SIM卡的情况下进入该状态
  * @param  staok : 本功能执行成功后切换的状态；
  * @param  staerr : .........失败后..........；
  * @retval None
  */
void  Modem_State_Tts (MODEM_STA staok,MODEM_STA staerr)
{
    ModemSimNoTime++;
    if(ModemSimNoTime > MODEM_TSEC*600)//10分钟检查一次,dxl,2015.3.27为行驶记录仪检测而修改,原来时间更小
    {
        ModemSimNoTime = 0;
        Modem_State_Switch(staerr);
    }
}

/*
  * @brief  组装播报文本数据
  * @param  type : 0为立即播报,如果有正在播报的内容则会被打断；
  *                1为后续播报,如果有正在播报的内容则等待播完后再播报新的;
  * @param  *pSrc  :播报的数据
  * @param  SrcLen :播报数据的长度,如果参数为0则按字符串\0为结束
  * @retval 合法返回1, 非法返回0
  */
u8  Modem_State_Tts_Dt(u8 type, const u8 *pSrc, u16 SrcLen)
{
    u16 CmpSrcLen;
    
    CmpSrcLen = ModLib_AtLen((u8*)pSrc);//过滤伪0或者\r\n

    if(CmpSrcLen < SrcLen)
    {
        SrcLen = CmpSrcLen;
    }
    
    if(SrcLen == 0)
    {
        return 0;//空数据
    }
    #ifdef MODEM_DEBUG_H
    if(ModDbInfType == MODDB_BASE)
    {
        MODDB_PRTFBAS("modem->Tts Send Len=%03d\r\n",SrcLen);
        MODDB_PRTFBAS("%s\r\n",(char*)pSrc);
        MODDB_PRTFBAS("modem->Tts Send End\r\n\r\n");
    }
    #endif 
    if(Modem_Type == MODEM_TYPE_Q)////////////移远
    {      
        if(QuectelAtTtsFlag == 0)
        {
            return 0;//移远模块有些不支持TTS
        }
        if((SrcLen+15) >= MODTTS_SIZE)
        {
            return 0;//数据超出
        } 
        memset(modtts.buf,0,MODTTS_SIZE);//清空数据
        modtts.len = ModLib_AtCmy(modtts.buf,"AT+QTTS=2,\"");
        memcpy(modtts.buf+modtts.len,pSrc,SrcLen);
        strcat((char*)modtts.buf,"\"\r\n");
    }               
    else if(Modem_Type == MODEM_TYPE_ZTE)/////中兴 
    {
        if(((SrcLen+15)*2) >= MODTTS_SIZE)//ByteToAsc
        {
            return 0;//数据超出
        }
        memset(modtts.buf,0,MODTTS_SIZE);//清空数据
        modtts.len = ModLib_AtCmy(modtts.buf,"AT+ZTTS=2,\"");
        ModLib_ByteStr(modtts.buf+modtts.len,pSrc,SrcLen);
        strcat((char*)modtts.buf,"\"\r\n");
    } 
    else
    {
        return 0;
    }
    
    modtts.len = strlen((char*)modtts.buf);

    if(type)//需要延时播报
    {
        if(modtts.busy)//上一条还没播报完
        {
            modtts.type = 1;
        } 
    }
    return 1;
}
/*
  * @brief  TTS语音播报状态
  * @param  type : 0 -> 查询播报状态
  *                1 -> 查询是否支持TTS
  * @retval 正在播报返回1, 播报完毕返回0
  */
u8  Modem_State_Tts_Is(u8 type)
{
    ////////////////////////////////////////////////////////////////////查询状态
    if(type == 0)
    {
        if(modtts.busy)
        {
            return 1;
        }
    }
    ////////////////////////////////////////////////////////////////////查询支持   
    else
    {
        if(Modem_Type == MODEM_TYPE_Q)///移远
        {    
            if(QuectelAtTtsFlag)
            {
                return 1;
            }
        }                 
        else if(Modem_Type == MODEM_TYPE_ZTE)/////中兴 
        {
            return 1;
        }  
    }
    return 0;
}
/*
  * @brief  TTS业务处理 -> 调节TTS音量
  * @param  state:本状态执行完毕后的下个状态
  * @retval None
  */
void  Modem_State_Tts_Clvl(MODTTS_STA state)
{
    u8 tab[30]={0};  
    u8 value = modtts.clvlvalue[modtts.clvl];
    
    switch(modtts.index)
    {      
        case 0://发送音量参数
            {
                #ifdef MODEM_DEBUG_H
                if(ModDbInfType == MODDB_BASE)
                {
                    MODDB_PRTFBAS("modem->Tts Clvl=%02d\r\n",value);
                }
                #endif
                if(Modem_Type == MODEM_TYPE_Q)////////////移远
                {
                    sprintf((char*)tab,"AT+QMEDVL=%d\r",value);
                }      
                else if(Modem_Type == MODEM_TYPE_ZTE)/////中兴 AT+ZTTSP=3,1,3
                {
                    sprintf((char*)tab,"AT+ZTTSP=%d,1,3\r",value);        
                }             
                Modem_Tts_Cmd(tab);
                modtts.index++;
            }
            break;            
        default://TTS失败
            {
                modtts.state = state;
                modtts.index = 0;
                modtts.send  = 0;//清零发送数据标志位
            }
            break;
    }
}
/*
  * @brief  TTS业务处理 -> 播报语音
  * @param  state:本状态执行完毕后的下个状态
  * @retval None
  */
void  Modem_State_Tts_Play(MODTTS_STA state)
{ 
    static u8 delay;
    static u8 outtime;
    
    switch(modtts.index)
    {      
        case 0://发送新TTS数据前先停止播报
            {
                ////////////////////////////////////////////////////////////////冲突处理
                if(modtts.wait)
                {
                    if((outtime++) < 5)//超时5个轮询
                    {
                        return;
                    }
                }
                outtime = 0;

                ////////////////////////////////////////////////////////////////停止处理
                modtts.cmdack = 0;//命令不需要判断应答
                modtts.index  = 1;//不发送停止命令直接跳过                
                if(modtts.busy == 0)//TTS处于空闲
                {
                    modtts.busy  = 1;
                    break;
                }
                
                if(Modem_Type == MODEM_TYPE_Q)////////////移远
                {              
                    Modem_Tts_Cmd("AT+QTTS=0\r");
                }               
                else if(Modem_Type == MODEM_TYPE_ZTE)/////中兴 
                {
                    Modem_Tts_Cmd("AT+ZTTS=0\r");
                }
                modtts.cmdack = 1;//命令需要判断应答
                delay = 0;  
            }
            break; 
        case 1://延时用于等待OK
            {
                if(modtts.cmdack == 0)
                {
                    Modem_Bsp_SpkOn(BSP_SPK_TTS);//打开扬声器
                    modtts.index++;
                }
                else
                {
                    if((delay++) > (MODEM_TSEC))
                    {
                        delay = 0;
                        Modem_Bsp_SpkOn(BSP_SPK_TTS);//打开扬声器
                        modtts.index++;
                    }
                }
            }
            break;                 
        case 2://发送TTS数据 命令+数据
            {
                Modem_Uart_Send(modtts.buf,modtts.len);
                modtts.index++;
                modtts.cmdack = 1;//命令需要判断应答
                delay = 0;                
            }
            break;  
        case 3://结束
            {
                if(modtts.cmdack == 0)
                {
                    modtts.index++;
                }
                else
                {
                    if((delay++) > (3*MODEM_TSEC))
                    {
                        delay = 0;
                        modtts.busy = 0;
                        modtts.index++;
                    }
                }
            }
            break;  
        default://发送结束
            {
                modtts.state = state;
                modtts.index = 0;
                modtts.send  = 0;//清零发送数据标志位    
            }
            break;           
    }
}
/*
  * @brief  TTS业务处理 -> 停止播报
  * @param  state:本状态执行完毕后的下个状态
  * @retval None
  */
void  Modem_State_Tts_Stop(MODTTS_STA state)
{
    switch(modtts.index)
    {      
        case 0://发送停止播报
            {
                if(Modem_Type == MODEM_TYPE_Q)////////////移远
                {              
                    Modem_Tts_Cmd("AT+QTTS=0\r");
                }                
                else if(Modem_Type == MODEM_TYPE_ZTE)/////中兴 
                {
                    Modem_Tts_Cmd("AT+ZTTS=0\r");
                }   

                modtts.index++;
            }
            break;            
        default://TTS失败
            {
                modtts.state = state;
                modtts.index = 0;
            }
            break;
    }

}
/*
  * @brief  TTS业务检查是否有需要播报的文本信息
  * @param  state:本状态执行完毕后的下个状态
  * @retval None
  */
void  Modem_State_Tts_Check(void)
{
    ////////////////////////////////////////////////////////////////////////////搜索
    if(modtts.type == 0)//立即播报
    {
        if(modtts.send)
        {
            modtts.state= MODTTS_PLAY;
        }
    }
    else//延时播报
    {
        if(modtts.busy == 0)//等待上一条播报完毕
        {
            modtts.type = 0;
            modtts.state= MODTTS_PLAY;
        }
    }
    
    ////////////////////////////////////////////////////////////////////////////超时处理
    if(modtts.busy == 0)
    {
        TtsOutTimeCount=0;
    }
    else
    {
        TtsOutTimeCount++;
        if(TtsOutTimeCount > MODEM_TSEC*120)//连续120s没有播报完毕则强制放弃
        {
            modtts.busy = 0;
            TtsOutTimeCount = 0;
            Modem_Bsp_SpkOff(BSP_SPK_TTS);//关闭扬声器  
        }
    }
}
/*
  * @brief  TTS业务处理
  * @param  state:本状态执行完毕后的下个状态
  * @retval None
  */
void  Modem_State_Tts_Pro(void)
{
    switch(modtts.state)
    {
        case MODTTS_IDLE:
            {
                Modem_State_Tts_Check();
            }
            break;
        case MODTTS_PLAY:
            {
                Modem_State_Tts_Play(MODTTS_IDLE);
            }
            break;
        case MODTTS_STOP:
            {
                Modem_State_Tts_Stop(MODTTS_IDLE);
            }
            break;  
        case MODTTS_CLVL:
            {
                Modem_State_Tts_Clvl(MODTTS_IDLE);
            }
            break;             
        default:
            {
                modtts.state = MODTTS_IDLE;
                modtts.index = 0;
            }
            break;        
            
    }

}
/*
  * @brief  modem在线.
  * @param  None
  * @retval None
  */
void  Modem_State_OnLine (void) 
{           
    switch (modem.onsta)
    {
        ////////////////////////////////////////////////////////////////////空闲
        case MODON_IDLE:/*---------------------------------------------*/
            {                  //长期处于该状态
                if(modapi.wait != MODON_IDLE)
                {     
                    modapi.wait = MODON_IDLE;
                }
                if((CsqStatIsCnt++) > CSQSTAT_TIME)
                {
                    CsqStatIsCnt = 0;
                    modapi.wait = MODON_CSQ;
                    modem.onsta = MODON_CSQ;
                }
                Modem_State_Check();
            }
            break;
        ////////////////////////////////////////////////////////////////////查询
        case MODON_CSQ:
            {
                Modem_State_CSQ(MODON_IDLE);
            }
            break;
        ////////////////////////////////////////////////////////////////打开连接            
        case MODON_IPOPEN:
            {
                Modem_State_Ip_Open(MODON_IDLE,modem.socopen);
            }
            break;
        ////////////////////////////////////////////////////////////////关闭连接              
        case MODON_IPCLOSE:
            {
                Modem_State_Ip_Close(MODON_IDLE,modem.socclose);
            }
            break;
        ////////////////////////////////////////////////////////////////发送数据            
        case  MODON_IPSEND:
            {              
                Modem_State_Ip_Send(MODON_IDLE,10,3);
            }
        break;
        ////////////////////////////////////////////////////////////////连接类型        
        case  MODON_DNSIP:
            {              
                Modem_State_DNSIP(MODON_IDLE);
            }    
        break;
        /////////////////////////////////////////////////////////////////////FTP          
        case  MODON_FTP:
            {              
                Modem_State_Ftp(MODON_IDLE);
            }    
        break;    
        ////////////////////////////////////////////////////////////////短信接收         
        case  MODON_SMSREC:
            {              
                Modem_State_Sms_Rec(MODON_IDLE,MODON_IDLE);
            }    
        break;   
        ////////////////////////////////////////////////////////////////短信发送        
        case  MODON_SMSSEND:
            {              
                Modem_State_Sms_Send(MODON_IDLE,MODON_IDLE);
            }    
        break;
        ////////////////////////////////////////////////////////////////////电话       
        case  MODON_PHONE:
            {              
                Modem_State_Phone_Pro(MODON_IDLE);
            }    
        break;  
        ////////////////////////////////////////////////////////////////////关机        
        case  MODON_OFF:
            {              
                Modem_State_Switch(MODEM_OFF);
            }    
        break; 
        ////////////////////////////////////////////////////////////////////复位
        case  MODON_SYSREST:
            {              
                NVIC_SystemReset();//系统复位
            }    
        break;
        
    } 

}




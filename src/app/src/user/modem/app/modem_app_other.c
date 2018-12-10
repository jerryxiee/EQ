/*
********************************************************************************
*
*                                                              
*
* Filename      : modem_app_Other.c
* Version       : V1.00
* Programmer(s) : miaoyahan
* @date         : 2014-10-27  
* 
*
*Note:主要处理FTP升级和SMS短信
********************************************************************************
*/

/*
********************************************************************************
*                               INCLUDES
********************************************************************************
*/
#include "include.h"

/*
********************************************************************************
*                               DEFIEN START
********************************************************************************
*/
#ifdef MODEM_APP_OTHER_H
/*
********************************************************************************
*                             LOCAL DEFINES
********************************************************************************
*/


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
static Modem_APP_STATE NetOther_State;//状态机
static APP_FTP         NetOther_FtpVal;//FTP升级参数

static u32 NetOther_SysRestDelay;//系统复位延时时间
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

/*
  * @brief  短信报警发送处理
  * @param  None
  * @retval None
  */
void Sms_Set_Alarm(void)
{
    u8 tab_send[200]={0};
    u8 ntab[20]={0};
    u8 num[15]={0};
    u8 nlen=0;
    u8 len_send=0;

    EepromPram_ReadPram(E2_CAR_PLATE_NUM_ID, ntab);//车牌号码
    nlen = strlen((char *)ntab);
    memcpy(tab_send,ntab,nlen);
    tab_send[nlen] = '|';
    len_send = nlen + 1;
    
    memset(ntab,0,sizeof(ntab));  
    EepromPram_ReadPram(E2_PRAM_BASE_CUSTOM_ID+0x07, ntab);//终端手机号码
    nlen = strlen((char *)ntab);
    nlen = ModLib_ByteStr(num, ntab, nlen);
    memcpy(tab_send+len_send,num+1,11);  
    len_send = len_send + 11;
    tab_send[len_send] = '|';
    len_send++;    

    memcpy(tab_send+len_send,"紧急报警",8);//报警类型
    len_send = len_send + 8;
    tab_send[len_send] = '|';   
    len_send++;

    memset(ntab,0,sizeof(ntab)); 
    Public_ConvertNowTimeToBCDEx((u8*)&ntab);//实时时间
    nlen = strlen((char *)ntab);
    nlen = ModLib_ByteStr(tab_send+len_send, ntab, nlen);
    len_send = len_send + nlen;
    tab_send[len_send] = '|';   
    len_send++;

    memset(ntab,0,sizeof(ntab));            //位置信息
    Public_GetCurPositionInfoDataBlock(ntab);
    nlen = strlen((char *)ntab);
    nlen = ModLib_ByteStr(tab_send+len_send, ntab, nlen);

    memset(ntab,0,sizeof(ntab));  
    EepromPram_ReadPram(E2_SMS_TEXT_ALARM_PHONE_ID, ntab);//发送手机号码
    
    nlen = strlen((char *)tab_send);
    Modem_Api_SmsSend(SMS_PDUUCS,ntab,tab_send,nlen);
}

/*
  * @brief  短信监控设置处理
  * @param  type -> 0:监控平台短信号码
                    1:特权短信号码
  * @retval None
  */
void Sms_Set_Num(u8 type ,u8 *pRec, u16 len)
{//smsnum
    u8 tab[20]={0};

    strcpy((char*)tab,(char*)modsms.num);
    if(type == 0)
    {
        EepromPram_WritePram(E2_MONITOR_SERVER_SMS_PHONE_ID,tab,E2_MONITOR_SERVER_SMS_PHONE_LEN);
    }
    else if(type == 1)
    {
        EepromPram_WritePram(E2_MONITOR_SERVER_PRIVILEGE_SMS_ID,tab,E2_MONITOR_SERVER_PRIVILEGE_SMS_LEN);
    }
}

/*
  * @brief  短信设置IP
  * @param  type: IP地址连接为0，域名连接为1
  * @param  *p:指向搜索地址
  * @param  len:搜索地址数据的长度
  * @retval None
 */
u8  Sms_Set_Ip(u8 type,u8 *pRec, u16 len)
{

    u16 i=0;// ip,219.142.106.193,port,8500#  surl,www.gps7788.com,port,8500#
    u8  *p=NULL;
    u8  tab[50]={0};
    u8  ip[20]={0};
    u8  port[5]={0};
    u16 dport=0;
    u8  cnt1=0;//计算','的个数
    u8  cnt2=0;//计算'.'的个数
    u8  cnt3=0;//计算'#'的个数
    u8  addrbuf[5]={0};

    if(len > sizeof(tab))
    {
        return 0;//内存溢出
    }
    memcpy(tab,pRec,len);
    p=tab;

    for(i=0;i<len;i++)
    {
        if(*(p+i) == ',')
        {
            addrbuf[cnt1++] = i;
        }
        else if(*(p+i) == '.')
        {
            cnt2++;
        }
        else if(*(p+i) == '#')
        {
            cnt3++;
            addrbuf[cnt1] = i;
        }        
    }
    if(type == 0)//ip地址连接
    {
        if((cnt1 != 3)||(cnt2 != 3)||(cnt3 != 1))
        {
            return 0;//格式不符合要求返回错误
        }
    }
    else//域名连接
    {
        if((cnt1 != 3)||(cnt3 != 1))
        {
            return 0;//格式不符合要求返回错误
        }
    }
    
    memcpy(ip,p+addrbuf[0]+1,addrbuf[1]-addrbuf[0]-1);//IP地址或域名
    memcpy(port,p+addrbuf[2]+1,addrbuf[3]-addrbuf[2]-1);//端口
    
    dport =  atoi((char*)port);
    port[0] = 0;
    port[1] = 0;
    port[2] = dport>>8;
    port[3] = dport&0xff;
    port[4] = 0;
    EepromPram_WritePram(E2_MAIN_SERVER_IP_ID,ip,E2_MAIN_SERVER_IP_LEN);
    EepromPram_WritePram(E2_MAIN_SERVER_TCP_PORT_ID,port,E2_MAIN_SERVER_TCP_PORT_LEN);
    return 1;
} 

/*
  * @brief  短信设置终端手机号
  * @param  *p:指向搜索地址
  * @param  len:搜索地址数据的长度
  * @retval None
 */
u8  Sms_Set_DevicePhone(u8 *pRec, u16 len)
{//ID,0000xxxxxxx0#  (11位ID)

    u8 tab[20]={0};
    u8 num[10]={0};
    u8 tabnum[20]={0};
    u8 nlen=0;

    u8 i;
    u8 *p = pRec;
    u8 cnt=0;
    u8 addr;
    
    for(i=0;i<len;i++)
    {
        if(*(p+i) == '#')
        {
            cnt++;
            addr=i;
        }
    }
    if(cnt != 1)
    {
        return 0;
    }


    memcpy(tab,p+3,addr-3);
    nlen = strlen((char *)tab);
    if(nlen%2)
    {
        tabnum[0]='0';
        memcpy(tabnum+1,tab,nlen);
        nlen++;
    }
    else
    {
        memcpy(tabnum,tab,nlen);
    }
    ModLib_StrByte(tabnum,num,nlen);  
    EepromPram_WritePram(E2_PRAM_BASE_CUSTOM_ID+0x07,num,E2_DEVICE_PHONE_ID_LEN); 

    return 1;
}
/*
  * @brief  短信设置缺省时间间隔
  * @param  *p:指向搜索地址
  * @param  len:搜索地址数据的长度
  * @retval None
 */
u8  Sms_Set_ReportTime(u8 *pRec, u16 len)
{
    u8  tab[15]={0};//report=XXXX
    u32 val=0;

    val =  atoi((char*)pRec+7);
    tab[0] = val>>24;
    tab[1] = (val>>16)&0xff;
    tab[2] = (val>>8)&0xff;
    tab[3] = val&0xff;

    EepromPram_WritePram(E2_ACCON_REPORT_TIME_ID,tab,E2_ACCON_REPORT_TIME_LEN); 
    EepromPram_UpdateVariable(E2_ACCON_REPORT_TIME_ID);
    return 1;
}
/*
  * @brief  短信设置心跳时间
  * @param  *p:指向搜索地址
  * @param  len:搜索地址数据的长度
  * @retval None
 */
u8  Sms_Set_Heartbeat(u8 *pRec, u16 len)
{
    u8  tab[15]={0};//heart=
    u32 val=0;

    val =  atoi((char*)pRec+6);
    tab[0] = val>>24;
    tab[1] = (val>>16)&0xff;
    tab[2] = (val>>8)&0xff;
    tab[3] = val&0xff;

    EepromPram_WritePram(E2_TERMINAL_HEARTBEAT_ID,tab,E2_TERMINAL_HEARTBEAT_LEN); 
    EepromPram_UpdateVariable(E2_TERMINAL_HEARTBEAT_ID);
    return 1;
}
/*
  * @brief  短信设置终端ID
  * @param  *p:指向搜索地址
  * @param  len:搜索地址数据的长度
  * @retval None
 */
u8  Sms_Set_DeviceId(u8 *pRec, u16 len)
{//ID,0000xxxxxxx0#  (11位ID)

    u8 tab[20]={0};
    u8 tabnum[20]={0};
    u8 num[10]={0};
    u8 nlen=0;
    u8 i;
    u8 *p = pRec;
    u8 cnt=0;
    
    for(i=0;i<len;i++)
    {
        if(*(p+i) == '#')
        {
            cnt++;
        }
    }
    if(cnt != 1)
    {
        return 0;
    }

    memcpy(tab,p+3,len-4);
    nlen = strlen((char *)tab);
    if(nlen%2)
    {
        tabnum[0]='0';
        memcpy(tabnum+1,tab,nlen);
        nlen++;
    }
    else
    {
        memcpy(tabnum,tab,nlen);
    }
    ModLib_StrByte(tabnum,num,nlen);
 
    EepromPram_WritePram(E2_PRAM_BASE_CUSTOM_ID+0x06,num,E2_DEVICE_ID_LEN); 
    return 1;
}
/*
  * @brief  软件版本和时间
  * @param  *p:指向搜索地址
  * @param  len:搜索地址数据的长度
  * @retval 返回长度
 */
u8  Sms_Set_VerNo(u8 *pData)
{
    u8 len=0;

/******************************************************************************///EGS702
#ifdef   EYE_EGS702
    len = Version_GetFirmwareInformation(pData);

/******************************************************************************/
#elif    defined EYE_EGS701
    u8 tmp;
    u8 *p = pData;
    
    tmp = strlen(FIRMWARE_VERSION);
    memcpy(p,FIRMWARE_VERSION,tmp);
    len = tmp;
    p = p + len;

    *p = ',';
    p++;
    len++;

    tmp = strlen(FIRMWARE_TIME);
    memcpy(p,FIRMWARE_TIME,tmp);
    len = len + tmp;    
#endif

    return len;
}
/*
  * @brief  系统重启后向特权号码发送短信
  * @param  None
  * @retval 返回结果
 */
u8  Sms_Set_SmsRst(u8 *pData)
{
    u8 addr;
    u8 tab[40];

    EepromPram_ReadPram(E2_MONITOR_SERVER_PRIVILEGE_SMS_ID,tab);
    if(ModLib_AtCmp(tab,modsms.num))
    {
        return 0;
    }
    addr = ModLib_AtAddr(pData,'=')+1;
    if(*(pData+addr) == '1')
    {
        tab[0] = 1;//打开重启发送短信 
    }
    else
    {
        tab[0] = 0;//关闭重启发送短信
    }
    FRAM_BufferWrite(FRAM_HY_UPDATA_ADDR,tab,1);//累计里程
    tab[0] = 0XAA;
    FRAM_BufferRead(tab,1,FRAM_HY_UPDATA_ADDR);//累计里程    
    return 1;
}
/**
  * @brief  重启发送短信
  * @param  None
  * @retval None
  */
void Net_Other_SmsRst(void)
{
    u8   Num[20];
    u8   StrLen;
    u8   StrBuf[100];
    static u8  State;
    static u8  Value[5];
    static u16 TimDly;            

    switch(State)
    {
        case 0:
        {
            State++;
            FRAM_BufferRead(Value,1,FRAM_HY_UPDATA_ADDR);            
        }
        break;
        case 1:
        {
            if(Value[0] == 1)
            {
                State++;
            }
        }
        break;
        case 2:
        {
            if(TimDly++ > MODEM_TMIN)
            {
                State++;
                EepromPram_ReadPram(E2_MONITOR_SERVER_PRIVILEGE_SMS_ID,Num);
                Num[0] = ModLib_AtLen(Num);
                if((Num[0]==13)||(Num[0]==11))
                {
                    memset(StrBuf,0,sizeof(StrBuf));
                    StrLen=ModLib_AtCmy(StrBuf,"SysRst!!!,");
                    
                    Public_ConvertNowTimeToBCDEx(Num);
                    StrLen += ModLib_ByteStr(StrBuf+StrLen,Num,6);
                    StrLen += ModLib_AtCmy(StrBuf+StrLen,",");
                    
                    EepromPram_ReadPram(E2_FIRMWARE_VERSION_ID,StrBuf+StrLen);
                    StrLen += ModLib_AtLen(StrBuf+StrLen);
                    StrLen += ModLib_AtCmy(StrBuf+StrLen,",");

                    StrLen += ModLib_AtCmy(StrBuf+StrLen,FIRMWARE_TIME);
                    StrLen += ModLib_AtCmy(StrBuf+StrLen,",");
                    
                    EepromPram_ReadPram(E2_DEVICE_ID,Num);
                    StrLen += ModLib_ByteStr(StrBuf+StrLen,Num,7);
                    StrLen += ModLib_AtCmy(StrBuf+StrLen,",");
                    
                    EepromPram_ReadPram(E2_DEVICE_PHONE_ID,Num);
                    StrLen += ModLib_ByteStr(StrBuf+StrLen,Num,6);
                    
                    StrLen  = ModLib_AtLen(StrBuf);
                    EepromPram_ReadPram(E2_MONITOR_SERVER_PRIVILEGE_SMS_ID,Num);
                    Modem_Api_SmsSend(SMS_TXT,Num,StrBuf,StrLen);
                }
            }
        }
        break; 
        default:
        break;
    
    }
}

/**
  * @brief  短信查询设备状态
  * @param  None
  * @retval None
  */
void Net_Other_SmsSta(void)
{
    char buf[200];
    u8   timebcd[10];
    char timeasc[20];    
    u8  csq; 
    u32 timecore;
    u32 timeon;
    u8  len;

    memset(buf,0,sizeof(buf));
    Public_ConvertNowTimeToBCDEx(timebcd);//当前时间
    Public_ConvertBcdToAsc((u8*)timeasc,timebcd,6);
    timeasc[12] = 0;
    
    csq = Modem_Port_Csq();       //网络质量
    timecore = modInf.Tim_Core/60;//内核运行时间      单位 分钟
    timeon   = modInf.Tim_Onln/60;//拨号成功上线时间  单位 分钟


    sprintf(buf,"timenow=%s,timecore=%06d,timeon=%06d,csq=%02d",
                                   timeasc,timecore,timeon,csq);
    len = strlen(buf);
    
    Modem_Api_SmsSend(SMS_TXT,NULL,(u8*)buf,len);
}
/*
  * @brief  显示提示信息
  * @param  None
  * @retval None
  */
void Net_Other_DisPlay(u8 *pSrc)
{
    LcdShowMsgEx((char*)pSrc,100);
}
/*
  * @brief  打印登陆鉴权时的时间，每次发送登陆或鉴权时被调用
  * @param  ch: 
  *            0为注册  
  *            1为鉴权  
  *            2为位置信息   
  *            3为GNSS透传时间
  * @retval None
  */

void Net_Other_Printf(NET_PRINTF_TYPE type)
{
    TIME_T nowTime;
    char  tab[50];
    
    RTC_GetCurTime(&nowTime);

    switch(type)
    {
        case PRTF_NOWTIME://当前时间
        {
            strcpy(tab,"当前时间\r\n");
            break;
        }
        case PRTF_AUTHORIZATION://鉴权
        {
            strcpy(tab,"鉴权时间:%d:%d:%d\r\n\r\n");
            break;
        }
        case PRTF_REGISTER://注册
        {
            strcpy(tab,"注册时间:%d:%d:%d\r\n\r\n");
            break;
        }    
        case PRTF_POSITION://位置
        {
            strcpy(tab,"位置信息时间:%d:%d:%d\r\n\r\n");
            break;
        }
        case PRTF_GNSS://位置
        {
            strcpy(tab,"GNSS透传时间:%d:%d:%d\r\n\r\n");
            break;
        }        
        default:
        {
            return;
        }

    }
    
    MODAPP_PRTF(tab, nowTime.hour,nowTime.min,nowTime.sec );
}
/**
  * @brief  系统复位
  * @param  None
  * @retval None
  */
void Net_Other_SysRest(void)
{
    if((NetOther_SysRestDelay--) == 0)
    {
        NVIC_SystemReset();
    }
}

/*
  * @brief  ftp文件下载CRC校验初始化
  * @param  None
  * @retval None
  */
void Net_Other_FtpCrcInit(void)
{
	/* Enable CRC Clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC, ENABLE);
	/* Reset CRC generator */
  	CRC->CR = CRC_CR_RESET;
}
/*
  * @brief  ftp文件下载计算CRC校验
  * @param  *p:指向要校验的数据
  * @param  len:要校验数据的长度
  * @retval 返回计算后的校验值
  */
u32 Net_Other_FtpCrcDo(u8 *p, u16 len)
{
    u32  i = 0;
    u32  ch = 0;

    if(p != NULL)
	{
        for(i=0;i<len;i++)
        {
            ch = *((u8*)p + i);
            CRC->DR = ch;
        }
	}
	ch = CRC->DR;
	return(ch);
}

/*
  * @brief  获取ftp文件下载参数,IP、端口等
  * @param  None
  * @retval 返回是否执行升级, 1为升级，0为参数有问题不升级
  */
u8 Net_Other_FtpVal(void)
{
    //"183.238.13.194","21","egs701","000000","/", "huoyun.bin"); 
    //huoyun.bin//V20109.bin//ftptest.txt//hb201308151659V1.01.01updata.bin
    u8  *p=NULL;
    u8  len=0;
    u8  addr=0;
    u8  tab[100]={0};
    u8  num=0;//","个数
    u8  numaddr[5]={0};//","的地址
    u16 ch=0;
    u8  i;

    EepromPram_ReadPram(E2_UPDATA_MANUFACTURE_ID, tab);
    if (strncmp((char *)(tab),"70108",5) != 0)
    {
        return 0;
    }
    
    p = NetOther_FtpVal.ip;
    len = sizeof(NetOther_FtpVal.ip);
    memset(p,0,len);
    EepromPram_ReadPram(E2_UPDATA_IP_ID, p);

    p = NetOther_FtpVal.port;
    len = sizeof(NetOther_FtpVal.port);
    memset(p,0,len);
    EepromPram_ReadPram(E2_UPDATA_TCP_PORT_ID, tab);
    ch = (tab[0]<<8)|(tab[1]);
    sprintf((char*)p,"%d",ch);

    memset(tab,0,sizeof(tab));
    len = EepromPram_ReadPram(E2_UPDATA_URL_ID, tab);
    num = 0;
    for(i=0;i<len;i++)
    {
        if(tab[i] == ',')
        {
            numaddr[num] = i;
            num++;
        }
    }
    if(num != 3)
    {
        return 0;
    }
        
    p = NetOther_FtpVal.fpath;
    len = sizeof(NetOther_FtpVal.fpath);
    memset(p,0,len); 
    memcpy(NetOther_FtpVal.fpath,tab,numaddr[0]); 

    p = NetOther_FtpVal.fname;
    len = sizeof(NetOther_FtpVal.fname);
    memset(p,0,len);
    addr = numaddr[0] + 1;
    len  = numaddr[1] - numaddr[0] - 1;
    memcpy(p,tab+addr,len); 

    p = NetOther_FtpVal.user;
    len = sizeof(NetOther_FtpVal.user);
    memset(p,0,len);
    addr = numaddr[1] + 1;
    len  = numaddr[2] - numaddr[1] - 1;    
    memcpy(p,tab+addr,len); 

    p = NetOther_FtpVal.pass;
    len = sizeof(NetOther_FtpVal.pass);
    memset(p,0,len);
    addr = numaddr[2] + 1;
    len  = strlen((char *)tab) - addr;    
    memcpy(p,tab+addr,len);   
    return 1;
}

/*
  * @brief  检查ftp升级文件crc校验
  * @param  *pType : 指向升级类型 
  *                  0 -> 终端;
  *                  9 -> 称重控制器;
  * @retval 返回校验结果，成功返回IAR生成的BIN文件大小，失败返回0
  */
u32 Net_Other_FtpCheck(u8 *pType)
{
    u32 ftpfilesize = Modem_Api_FtpSize();
    u32 Addrbase   =  UPDATE_BASE_ADDRESS; 
    u32 Address    =  Addrbase;

    u32 verify;//升级文件中的校验
    u32 verifycmp;//计算后的校验
    u16 integer;//整数
    u16 remainder;//余数
    u16 i;
    u8  type;
    
    u8  ntab[520];

    if(ftpfilesize < 512)
    {
        return 0;
    }

    ftpfilesize = ftpfilesize - 22;
    
    sFLASH_ReadBuffer(ntab,Addrbase+ftpfilesize,22);

    if(Public_CheckTerminalAndPeripheralVerify(ntab,(unsigned long *)&verify,&type) == 0)
    {
        return 0;
    }
    *pType = type;//传递类型

    Net_Other_FtpCrcInit();//初始化硬件CRC校验
    integer =  ftpfilesize / 512;//计算整数页数  
    remainder = ftpfilesize % 512;//计算余数页数

    for(i = 0; i < integer;)
    {
        sFLASH_ReadBuffer(ntab,Address,512);
        verifycmp = Net_Other_FtpCrcDo(ntab,512);
        i++;            
        Address = Addrbase + (i * 512);
    }
    
    if(remainder)
    {
        sFLASH_ReadBuffer(ntab,Address,remainder);
        verifycmp = Net_Other_FtpCrcDo(ntab,remainder); 
    }

    if(verify == verifycmp)
    {
        return (ftpfilesize+22);
    }
    
    return 0;
}
/*
  * @brief  开始进行ftp文件下载
  * @param  None
  * @retval 返回是否执行升级, 1为升级，0为参数有问题不升级
  */
u8 Net_Other_FtpStart(void)
{
    if(NetOther_FtpVal.flag == 0)
    {
        memset((u8*)&NetOther_FtpVal,0,sizeof(APP_FTP));

        if(Net_Other_FtpVal() == 0)
        {
            return 0;//参数下发有问题，不执行升级
        }
        sFLASH_EraseSector(UPDATE_BASE_ADDRESS + (NetOther_FtpVal.sector * SECTOR_SIZE));

        ClrTimerTask(TIME_CAMERA);//关闭拍照
    	ClrTimerTask(TIME_RECORD_TIMER);//关闭录音
    	ClrTimerTask(TIME_RECORD);
    	ClrTimerTask(TIME_BLIND);
    	ClrTimerTask(TIME_POSITION);
    	
        NetOther_FtpVal.flag = 1;//置位ftp升级标志
        NetOther_State = MODEM_APP_FTP;//开启FTP升级流程
        
        return 1;
    }
    return 0;
}

/*
  * @brief  ftp文件下载回调函数
  * @param  None
  * @retval None
  */
void Net_Other_FtpCall(u8 *pData, u16 len)
{
    u32 addrbase   =  UPDATE_BASE_ADDRESS;
    u32 addrsector =  NetOther_FtpVal.sector * SECTOR_SIZE;
    u32 addrpage   =  NetOther_FtpVal.page * 512;
    u32 Address    =  addrbase + addrsector + addrpage;
    
    if(NetOther_FtpVal.flag == 0)
    {
        return;
    }

    sFLASH_WriteBuffer(pData, Address, len);

    NetOther_FtpVal.page++;
    if((NetOther_FtpVal.page * 512) >= SECTOR_SIZE)
    {
        NetOther_FtpVal.page = 0;
        NetOther_FtpVal.sector++;
        sFLASH_EraseSector(UPDATE_BASE_ADDRESS + NetOther_FtpVal.sector * SECTOR_SIZE);
    }
}
/*
  * @brief  ftp文件下载完成
  * @param  None
  * @retval None
  */
void Net_Other_FtpEnd(void)
{
    u32 ftpfilesize = Modem_Api_FtpSize();
    u8  tab[5]={0};
    u8  i=0;
    u8  sum=0;
    u8  type=0;//升级类型 

    if(Modem_Api_FtpResult())
    {
        ftpfilesize = Net_Other_FtpCheck(&type);//检查升级文件CRC校验

        if(ftpfilesize == 0)
        {
            MODAPP_PRTF("modem->Ftp_APP 校验出错!!!\r\n");
            
            tab[0] = 0;//升级类型
            FRAM_BufferWrite(FRAM_FIRMWARE_UPDATA_TYPE_ADDR, tab, FRAM_FIRMWARE_UPDATA_TYPE_LEN);

            tab[0] = 3;//升级标志 失败 3
            FRAM_BufferWrite(FRAM_FIRMWARE_UPDATA_FLAG_ADDR,tab,1);
            
            return;//校验失败，不进行升级
        }
        MODAPP_PRTF("modem->Ftp_APP 校验正确!!!\r\n");
    	tab[0] = ftpfilesize>>24;
    	tab[1] = (ftpfilesize>>16)&0xff;
    	tab[2] = (ftpfilesize>>8)&0xff;  
    	tab[3] =  ftpfilesize&0xff;   

        sum = 0;
		for (i=0; i<4; i++) 
		{
			sum += tab[i];
			E2prom_WriteByte(E2_PROGRAM_UPDATA_LENGTH_ID_ADDR + i, tab[i]) ;
		}
		E2prom_WriteByte(E2_PROGRAM_UPDATA_LENGTH_ID_ADDR + i, sum);

        ////////////////////////////////////////////////////////////////////////终端
        if(type == 0)
        {
            MODAPP_PRTF("modem->Ftp_APP 升级终端!!!\r\n");
            MODAPP_PRTF("modem->Ftp_APP 即将重启...进入IAP..\r\n");
            tab[0] = 0;//升级类型
            FRAM_BufferWrite(FRAM_FIRMWARE_UPDATA_TYPE_ADDR, tab, FRAM_FIRMWARE_UPDATA_TYPE_LEN);

            tab[0] = 2;//升级成功
            FRAM_BufferWrite(FRAM_FIRMWARE_UPDATA_FLAG_ADDR,tab,1);        

            //更新程序标志  
    		E2prom_WriteByte(E2_PROGRAM_UPDATA_REQUEST_ID_ADDR, 0xaa);
        	E2prom_WriteByte(E2_PROGRAM_UPDATA_REQUEST_ID_ADDR+1, 0xaa);
        }
        ////////////////////////////////////////////////////////////////////////载重控制器
        #ifdef  EYE_EGS701
        else if(type == 9)
        {
            MODAPP_PRTF("modem->Ftp_APP 升级外设!!!\r\n");
            MODAPP_PRTF("modem->Ftp_APP 即将外设升级任务...\r\n");
            tab[0] = 9;//升级类型
            FRAM_BufferWrite(FRAM_FIRMWARE_UPDATA_TYPE_ADDR, tab, FRAM_FIRMWARE_UPDATA_TYPE_LEN);
            
            //开启载重控制器升级任务
            SetTimerTask(TIME_LOAD_CONTROLLER_UPDATA, 3*SYSTICK_0p1SECOND);

            Communication_TaskSet(0);//关闭整个通讯模块任务
        }
        #endif
        MODAPP_PRTF("\r\n\r\n");
    }
}

/*
  * @brief  处理行标短息数据
  * @param  None
  * @retval None
  */
void Net_Other_SmsHB(u8 *pData, u8 len)
{
    u8 *p=pData;
    if((*p != 0x7e)||(*(p+len-1) != 0x7e))
    {
        return;
    } 
    uart2_recdat.rec_chanel = CHANNEL_SMS;//切换到短信通道
    
    RadioProtocol_AddRecvDataForParse(CHANNEL_SMS,p,len);
}
/*
  * @brief  处理短信后门数据
  * @param  None
  * @retval 无效数据 -> 0;
  *         命令数据 -> 1;
  *         行标数据 -> 2;
  */
u8 Net_Other_SmsDB(u8 *pData, u8 len)
{
    u8  flag=0;
	u8  tab_rec[80]={0};
	u8  tab[20]={0};
    
    MODSMS_TYPE  smstpye = SMS_TXT;//短信类型
  
    if(ModLib_AtCmp(pData,"smsnum") == 0)////////SMS监控号码 
    {//smsnum
        Sms_Set_Num(0,pData,len);
        sprintf((char*)tab_rec,"Set Ok ! %s",modsms.num);
        Modem_Api_SmsSend(smstpye,NULL,tab_rec,ModLib_AtLen(tab_rec));
        return 0;
    }
    EepromPram_ReadPram(E2_MONITOR_SERVER_SMS_PHONE_ID,tab);
    if(ModLib_AtLen(tab) == 0)
    {
        return 2;
    }
    if(ModLib_AtCmp(tab,modsms.num))
    {
        return 2;
    }
    if ((ModLib_AtCmp(pData,"ip,") == 0)||(ModLib_AtCmp(pData,"surl,") == 0))////IP设置
    {   //ip,219.142.106.193,port,8500#  surl,www.gps7788.com,port,8500# 
        if(Sms_Set_Ip(1,pData,len))
        {
            sprintf((char*)tab_rec,"Set Ok ! %s",pData);
            NetOther_SysRestDelay = MODEM_TMIN;//延时1分钟重启
            NetOther_State        = MODEM_APP_SYSREST;
        }
        else
        {
            ModLib_AtCmy(tab_rec,"Set Err !!!");
        }
        Modem_Api_SmsSend(smstpye,NULL,tab_rec,ModLib_AtLen(tab_rec));
        flag = 1;
    } 
    else if(ModLib_AtCmp(pData,"rest") == 0)///////////////////系统复位
    {
        Modem_Api_SmsSend(smstpye,NULL,"rest is OK!",11);
        NetOther_SysRestDelay = MODEM_TMIN;//延时1分钟重启
        NetOther_State        = MODEM_APP_SYSREST;
        flag = 1;
    }
    else if(ModLib_AtCmp(pData,"ID,") == 0)//////////////////终端手机号设置  
    {//ID,0000xxxxxxx0#  (11位ID)
        if(Sms_Set_DevicePhone(pData,len))
        {
            sprintf((char*)tab_rec,"Set Ok ! %s",pData);
            Modem_Api_SmsSend(smstpye,NULL,tab_rec,ModLib_AtLen(tab_rec));            
        }
        else
        {
            Modem_Api_SmsSend(smstpye,NULL,"ID SET ERR",10);
        } 
        flag = 1; 
    }   
    else if (ModLib_AtCmp(pData,"verno#") == 0)/////////////////////软件版本
    {
        len = Sms_Set_VerNo(tab_rec);
        Modem_Api_SmsSend(smstpye,NULL,tab_rec,len);       
        flag = 1; 
    }      
    else if (ModLib_AtCmp(pData,"heart=") == 0)/////////////////心跳时间设置
    {//heart=XXXX
        Sms_Set_Heartbeat(pData,len);
        sprintf((char*)tab_rec,"Set Ok ! %s",pData);
        Modem_Api_SmsSend(smstpye,NULL,tab_rec,ModLib_AtLen(tab_rec));  
        flag = 1;
    }
    else if (ModLib_AtCmp(pData,"report=") == 0)///////////////缺省时间设置
    {//report=XXXX
        Sms_Set_ReportTime(pData,len);
        sprintf((char*)tab_rec,"Set Ok ! %s",pData);
        Modem_Api_SmsSend(smstpye,NULL,tab_rec,ModLib_AtLen(tab_rec));  
        flag = 1;
    }
    else if (ModLib_AtCmp(pData,"numvip") == 0)///////////////特权短信号码设置
    {
        Sms_Set_Num(1,pData,len);
        sprintf((char*)tab_rec,"Set Ok ! %s",modsms.num);
        Modem_Api_SmsSend(smstpye,NULL,tab_rec,ModLib_AtLen(tab_rec));  
    }   
    else if (ModLib_AtCmp(pData,"smsrst=") == 0)//////系统重启后向特权号码发送短信
    {
        if(Sms_Set_SmsRst(pData))
        {
            sprintf((char*)tab_rec,"Set Ok ! %s",pData);
            Modem_Api_SmsSend(smstpye,NULL,tab_rec,ModLib_AtLen(tab_rec)); 
        }
    }  
    else if (ModLib_AtCmp(pData,"smssta") == 0)////////////////////////查询设备状态
    {
        Net_Other_SmsSta(); 
    }      
    else
    {
        flag = 2;
    }
    return flag;
}

/*
  * @brief  短信回调函数
  * @param  None
  * @retval None
  */
void Net_Other_SmsCall(MODSMS_TYPE type, u8 *p, u16 len)
{
    u8 tab[70]={0};

    if(type == SMS_TXT)
    {
        if((len/2) < sizeof(tab))
        {
            if(Net_Other_SmsDB(p,len) == 2)//处理字符数据
            {
                len = ModLib_StrByte(p,tab,len);
                Net_Other_SmsHB(tab,len);//处理行标数据
            }
        }
    }
    else if(type == SMS_PDU7)
    {
        Net_Other_SmsDB(p,len);//7bit 字符数据
    }
    else if(type == SMS_PDU8)
    {
        Net_Other_SmsHB(p,len);//8bit 纯数据
    }
}
/**
  * @brief  连接处理
  * @param  None
  * @retval None
  */
void Net_Other_Run(void)
{
    MODERR_TYPE modemack;

    Net_Other_SmsRst();//重启发送短信

    switch(NetOther_State)
    {
        case MODEM_APP_IDLE:
            {
                ;
            }
        break; 
        case MODEM_APP_FTP://FTP升级
            {
                modemack = Modem_Api_Ftp(NetOther_FtpVal.ip,
                                         NetOther_FtpVal.port,
                                         NetOther_FtpVal.user,
                                         NetOther_FtpVal.pass,
                                         NetOther_FtpVal.fpath,
                                         NetOther_FtpVal.fname);
                if(modemack == MOD_OK)
                {
                    Net_Other_FtpEnd();//下载结束 延时复位
                    NetOther_SysRestDelay = MODEM_TSEC*2;
                    NetOther_State        = MODEM_APP_SYSREST;
                }
                else if(modemack == MOD_ER)
                {
                    NetOther_SysRestDelay = 0;//下载失败 立即复位
                    NetOther_State = MODEM_APP_SYSREST;
                }
            }
        break;
        case MODEM_APP_SYSREST://复位系统
            {
    			 Net_Other_SysRest();
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

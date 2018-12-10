/*******************************************************************************
 * File Name:			ICCard.h 
 * Function Describe:	
 * Relate Module:		IC卡模块
 * Writer:				Joneming
 * Date:				2014-07-09
 * ReWriter:			
 * Date:				
 *******************************************************************************/
 /*******************************************
 //++++++++  结构体: 2012年版汽车行驶记录仪 驾驶人身份识别IC卡 ++++++++++++++

typedef struct tagSTT_CARD_VEHICLERECORDER_2012
{
    u8 Usage; /// 协议: 卡用途种类 驾驶员///    
    u8 Reserved[31]; ////    
    u8 DriverID[18];//机动车驾驶证号码//    
    u8 ValidityPeriod_Year;///机动车驾驶证有效期//
    u8 ValidityPeriod_Month;///机动车驾驶证有效期//
    u8 ValidityPeriod_Day;///机动车驾驶证有效期//    
    u8 QualificationCertificateID[18];///从业资格证号///    
    u8 StandardExtension[56];///标准扩展预留///    
    u8 VerifyCode;//校验字//
}STT_CARD_VEHICLERECORDER_2012;

//++++++++  结构体: 2011版 道路运输车辆卫星定位系统 驾驶员从业资格证  ++++++++++++++

typedef struct tagSTRUCT_IC_CARD_ROADTRANSPORT808_2011
{
    u8 Usage;// 协议: 卡用途种类 驾驶员 //    
    u8 DriverNameLength;///驾驶员姓名长度 ///    
    u8 DriverName[10];///驾驶员姓名 ///    
    u8 DriverID[20];///驾驶员身份证编码  长度为20///    
    u8 QualificationCertificateID[40];///从业资格证号 长度为40///    
    u8 IssueOrganizationLength;///发证机构名称长度 ///    
    u8 IssueOrganization[40];///发证机构名称长度///    
    u8 DriverCodeLength;///驾驶员代码长度 ///    
    u8 DriverCode[18];///驾驶员代码///    
    u8 ValidityPeriod_Year[2];///机动车驾驶证有效期///
    u8 ValidityPeriod_Month;///机动车驾驶证有效期///
    u8 ValidityPeriod_Day;///机动车驾驶证有效期///    
    u8 StandardExtension[56];///标准扩展预留///    
    u8 VerifyCode;///校验字///
}STT_CARD_ROADTRANSPORT808_2011;

//++++++++  结构体: 
typedef struct tagSTT_CARD_JTB
{
	u8 DriverNameLength;///驾驶员姓名长度 n///	
	u8 DriverName[20];// 驾驶员姓名 //
	u8 QCID[20];//驾驶员身份证编码  长度为20//	
	u8 IssueOrganizationLength;///发证机构名称长度///	
	u8 IssueOrganization[40];///发证机构名称长度 ///	
	u8 ValidityPeriod_Year[2];///机动车驾驶证有效期///
	u8 ValidityPeriod_Month;///机动车驾驶证有效期///
	u8 ValidityPeriod_Day;///机动车驾驶证有效期///	
}STT_CARD_JTB;
//++++++++  结构体: 
typedef struct tagSTT_CARD_DRIVER_RECORD_ITEM
{
    u8 EventTime[6];  //事件发生时间，BCD码 年-月-日 时-分-秒
    u8 DriverID[18];  //机动车驾驶证号码 ASCII码
    u8 EventType; //事件类型： 0x01: 登录  0x02:退出 ， 其它:预留  
}
******************************************/
////*****************************************//
//***************包含文件*****************
#include "include.h"
/////////////////////////////////
#define ICCARD_DATA_LEN_MIN                 9//10
#define ICCARD_DATA_LEN_MAX                 256
#define ICCARD_DATA_LEN_READ                128
/////////////////////////
#define ICCARD_PLAY_REMIND_GPOUP_TIME       300//300每组提示时间间隔(秒)
#define ICCARD_PLAY_REMIND_TOTAL_TIME       1800//1800提醒总时间(秒)
#define ICCARD_PLAY_REMIND_ONCE_NUMBER      3//3每组提示次数
#define ICCARD_PLAY_REMIND_ONCE_TIME        12//每次提示时间间隔(秒)
///////////////////////////
typedef struct 
{
    u8 NameLen;
    u8 Name[20];//驾驶员
    u8 DriverID[20];//驾驶证号码
    u8 QCID[40];//从业资格证号
    u8 OrganizationLen;
    u8 Organization[40];//发证机构名称
    u8 ValidTime[4];//机动车驾驶证有效期YYYY-MM-dd
}ST_DRIVER_REPORT;
//////////////
#define SIZE_ST_DRIVER_REPORT sizeof(ST_DRIVER_REPORT)

typedef struct
{
    ST_DRIVER_REPORT stDriver;
    unsigned short playTimeCnt;    
    unsigned char signTime[6];
    unsigned char preDriverID[18];  //机动车驾驶证号码 ASCII码
    unsigned char errCode;//
    unsigned char presignstatus;//
    unsigned char signstatus;//
    unsigned char resendCnt;
    unsigned char repairSendCnt; 
    unsigned char onlinestatus;
    unsigned char voicePrompt;
    unsigned char playflag;
    unsigned char firstcheckflag;
    unsigned char OnceNumber;//提示单次重复播报的次数(设定值)
    unsigned char OnceCnt;//提示单次重复播报的计数器    
}ST_ICCARD_ATTRIB;

static ST_ICCARD_ATTRIB s_stICCardAttrib;
/////////////////
#define ICCARD_DRIVER_SIGN_DATA_LEN_MAX        109//
typedef struct
{
    unsigned char len;//数据长度
    unsigned char data[ICCARD_DRIVER_SIGN_DATA_LEN_MAX];//内容
}STDRIVERSIGN;

static STDRIVERSIGN s_stDriverSign;
/////////////////////
#define STDRIVERSIGN_SIZE sizeof(STDRIVERSIGN)

typedef struct
{
    unsigned char saveflag;         //保存标志
    unsigned char recordCnt;        //当前保存登签记录总条数
    unsigned char sendrecordCnt;    //已发送的登签数据索引号
    unsigned char bak;
}ICCARD_SAVE_INDEX;

static ICCARD_SAVE_INDEX s_stSignSaveIndex;

#define SAVE_INDEX_FLAG 0xab

#define DRIVERSIGN_MAX_RECORD   40  //4k的空间,每条96字节
///////////////////////////////////////////////////////
#define DRIVER_SIGN_INFO_START_ADDR           (unsigned long)(FLASH_DRIVER_SIGN_INFO_START_SECTOR*FLASH_ONE_SECTOR_BYTES)//开始地址
#define DRIVER_SIGN_INFO_DATALEN              (unsigned long)FLASH_ONE_SECTOR_BYTES
#define FRAM_ICCARD_INDEX_ADDR                 FRAM_LAST_RUN_TIME_ADDR

enum
{
    ICCARD_CMD_INSERT = 0x41,
    ICCARD_CMD_PULL_OUT,    
    ICCARD_CMD_MAX,
};

enum
{
    ICCARD_ONLINE_STASUS_EMPTY,
    ICCARD_ONLINE_STASUS_ON, 
    ICCARD_ONLINE_STASUS_OFF,
    ICCARD_ONLINE_STASUS_MAX,
};
enum
{
    DRIVER_SIGN_EMPTY,
    DRIVER_SIGN_IN,
    DRIVER_SIGN_OUT,
    DRIVER_SIGN_INITIATIVE,	//主动读取信息0x43
    DRIVER_SIGN_PASSIVITY,	//被动上报信息0xA1
    DRIVER_SIGN_MAX,
};
enum
{
    ICCARD_ERR_NULL_DRIVER = 0,//
    ICCARD_ERR_NULL_EWAYBILL,//
    ICCARD_ERR_NULL_MANAGE,//
    ICCARD_ERR_NULL_MANAGE_EX,//
    ICCARD_ERR_CMD_UNKOWN,//未知命令
    ICCARD_ERR_TYPE_OUT,  //卡类型错误  
    ICCARD_ERR_PULL_OUT_LEN,//数据长度错误  
    ICCARD_ERR_TYPE_INSERT, //卡类型错误 
    ICCARD_ERR_INSERT_NONSUPPORT,//卡不支持或无效卡
    ICCARD_ERR_INSERT_LEN,//数据长度错误
    ICCARD_ERR_INSERT_EMPTY,//空卡
    ICCARD_ERR_INSERT_VERIFY,//数据校验出错
    ICCARD_ERR_INSERT_TYPE_UNKOWN,//无法识别
    ICCARD_ERR_INSERT_DATA_FORMAT,//数据格式错误    
    ICCARD_ERR_MAX,
};

enum 
{
    IC_TIMER_TASK,                  //0 timer
    IC_TIMER_TIMEOUT,               //1 timer
    IC_TIMER_CHECK,                 //2 timer
    IC_TIMER_TEST,                  //3 timer
    IC_TIMER_REPAIR,
    IC_TIMERS_MAX
}EICCARDTIME;

static LZM_TIMER s_stICTimer[IC_TIMERS_MAX];
/////////////////////////////////
void ICCard_ServiceDataRepairSignDataToPC(void);
/////////////////////////////////
/*************************************************************
** 函数名称: ICCard_ShowDebugInfo
** 功能描述: 数据解析
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void ICCard_ShowDebugInfo(const char *StrPtr)
{
    //TIME_T nowTime;
    //RTC_GetCurTime(&nowTime);
    //LOG_PR( "%s: %02d:%02d:%02d% \r\n",StrPtr,nowTime.hour,nowTime.min,nowTime.sec );
}
/*************************************************************
** 函数名称: ICCard_PackAndSendData
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
unsigned char ICCard_PackAndSendData(unsigned char cmd,unsigned char *pBuffer,unsigned short datalen)
{
    u8 buffer[160];
    u16 len;
    len=ICCardBase_CreatPackData(cmd,pBuffer,datalen,buffer,1);
    return CommICCard_SendData(buffer,len);
}
/*************************************************************OK
** 函数名称: ICCard_FlashIndexSave()
** 功能描述: 保存索引操作参数
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void ICCard_FlashIndexSave(void) 
{
    s_stSignSaveIndex.saveflag =SAVE_INDEX_FLAG;
    Public_WriteDataToFRAM(FRAM_ICCARD_INDEX_ADDR,(unsigned char *)&s_stSignSaveIndex,sizeof(ICCARD_SAVE_INDEX));    
}
/*************************************************************OK
** 函数名称: ICCard_FlashIndexRead
** 功能描述: 读取多媒体索引操作参数
** 入口参数: 			 
** 出口参数: 
** 返回参数: :
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void ICCard_FlashIndexRead(void) 
{
    Public_ReadDataFromFRAM(FRAM_ICCARD_INDEX_ADDR,(unsigned char *)&s_stSignSaveIndex,sizeof(ICCARD_SAVE_INDEX));
}
/*************************************************************OK
** 函数名称: ICCard_FlashFormat()
** 功能描述: Flash格式化
** 入口参数: startAddr:开始地址(相对地址);lenght:长度
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void ICCard_FlashFormat(void) 
{
    sFLASH_EraseSector(DRIVER_SIGN_INFO_START_ADDR);
}
/*************************************************************OK
** 函数名称: ICCard_SaveDataToFlash()
** 功能描述: Flash写数据
** 入口参数: s:所有保存的源数据指针,startAddr:开始地址(相对地址);lenght:长度
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void ICCard_SaveDataToFlash(unsigned char *s,unsigned short startAddr,unsigned short lenght) 
{
    //注意startAddr为相对地址
    unsigned long addr;
    addr = startAddr;
    addr += DRIVER_SIGN_INFO_START_ADDR;
    /////////////////////
    sFLASH_WriteBuffer(s,addr,lenght);
}
/*************************************************************OK
** 函数名称: ICCard_ReadDataFromFlash()
** 功能描述: Flash读数据
** 入口参数: s:数据指针，startAddr:开始地址(相对地址);lenght:长度
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void ICCard_ReadDataFromFlash(unsigned char *s,unsigned short startAddr,unsigned short lenght) 
{
    //注意startAddr为相对地址
    unsigned long addr;
    addr = startAddr;
    addr += DRIVER_SIGN_INFO_START_ADDR;
    ///////////////
    sFLASH_ReadBuffer(s,addr,lenght);
}
/*************************************************************OK
** 函数名称: ICCard_FlashIndexInit()
** 功能描述: 登签记录初始化
** 入口参数: 
** 出口参数: 
** 返回参数:
** 全局变量: 无
** 调用模块: ICCard_FlashIndexSave
*************************************************************/
void ICCard_SignFlashIndexInit(void)
{ 
    //先格式化
    ICCard_FlashFormat();
    s_stSignSaveIndex.recordCnt =0;
    s_stSignSaveIndex.sendrecordCnt=0;
    //////////////////////////////////////
    ICCard_FlashIndexSave();
}
/*************************************************************OK
** 函数名称: ICCardSaveData
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:           
*************************************************************/
void ICCard_CheckAndMoveSignData(void) //
{
    unsigned short len,startAddr,cnt;
    //////////////////////////////
    if(s_stSignSaveIndex.recordCnt+1>DRIVERSIGN_MAX_RECORD)
    {
        if(s_stSignSaveIndex.recordCnt == s_stSignSaveIndex.sendrecordCnt)
        {
            ICCard_SignFlashIndexInit();
        }
        else
        {
            cnt = s_stSignSaveIndex.sendrecordCnt;
            if(cnt < 3)cnt = 3;
            startAddr = cnt * STDRIVERSIGN_SIZE;
            len=(s_stSignSaveIndex.recordCnt-cnt);
            len*=STDRIVERSIGN_SIZE;
            ICCard_ReadDataFromFlash((unsigned char *)s_ucMsgFlashBuffer ,0,DRIVER_SIGN_INFO_DATALEN);
            ///////////////////////////////////////////////
            ICCard_FlashFormat();
            IWDG_ReloadCounter();
            /////////////////////////////////
            ICCard_SaveDataToFlash((unsigned char *)&s_ucMsgFlashBuffer[startAddr],0,len);            
            if(s_stSignSaveIndex.sendrecordCnt>cnt)s_stSignSaveIndex.sendrecordCnt -= cnt;
            else  s_stSignSaveIndex.sendrecordCnt =0;
            if(s_stSignSaveIndex.recordCnt>cnt)s_stSignSaveIndex.recordCnt -= cnt;
            else s_stSignSaveIndex.recordCnt =0;
            ICCard_FlashIndexSave();//存储参数
        }        
    }
}
/*************************************************************OK
** 函数名称: ICCardSaveData
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:           
*************************************************************/
void ICCard_SaveSignData(STDRIVERSIGN stSign) //
{
    unsigned short startAddr;
    ////////////////////////// 
    ICCard_CheckAndMoveSignData();
    /////////////////////////////////
    startAddr = s_stSignSaveIndex.recordCnt * STDRIVERSIGN_SIZE;    
    ////////////////////////////////////////////////////////////
    ICCard_SaveDataToFlash((unsigned char *)&stSign,startAddr,STDRIVERSIGN_SIZE);
    //更改索引操作参数////
    s_stSignSaveIndex.recordCnt++;    
    ICCard_FlashIndexSave();//存储参数
}
/*************************************************************
** 函数名称: Report_ResendPositionInformation
** 功能描述: 位置汇报时间任务
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void ICCard_SaveCurReportDriverSignRecord(void)
{
    if(s_stDriverSign.len&&s_stICCardAttrib.resendCnt)
    ICCard_SaveSignData(s_stDriverSign);
    s_stICCardAttrib.resendCnt = 0;
}
/*************************************************************OK
** 函数名称: ICCard_GetSignRecordData
** 功能描述: 
** 入口参数: index记录数(记录数从1开始)
** 出口参数: 
** 返回参数: 1表示读到内容,0表示没内容
** 全局变量: 无
** 调用模块:
*************************************************************/
void ICCard_GetSignRecordData(unsigned char index, STDRIVERSIGN *p)
{
   unsigned short startAddr;
   if(0==index)index = 1;
   startAddr =(index-1) * STDRIVERSIGN_SIZE;
   ////////////////////////////////
   ICCard_ReadDataFromFlash((unsigned char *)p ,startAddr,STDRIVERSIGN_SIZE);
   /////////////////////////////////////
}
/*************************************************************OK
** 函数名称: ICCard_FlashIndexSave()
** 功能描述: 保存索引操作参数
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void ICCard_ServiceDataRepairSucceedForSaveSignIndex(void) 
{
    s_stSignSaveIndex.sendrecordCnt++;
    ICCard_FlashIndexSave();
}
/*************************************************************OK
** 函数名称: ICCard_ReadRepairServiceDataForSendPC
** 功能描述: 检查发送盲区的签到、签到、营运数据
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void ICCard_ReadRepairServiceDataForSendPC(void) 
{
    if(s_stSignSaveIndex.sendrecordCnt<s_stSignSaveIndex.recordCnt)
    {
        s_stICCardAttrib.repairSendCnt = 0;
        LZM_PublicSetOnceTimer(&s_stICTimer[IC_TIMER_REPAIR],PUBLICSECS(1),ICCard_ServiceDataRepairSignDataToPC);
    }
}
/*******************************************************************************
**  函数名称  : ICCard_ServiceDataRepairToPC
**  函数功能  : 
**  输    入  : 无				
**  输    出  : 
**  全局变量  : 
**  调用函数  : 
**  中断资源  : 
**  备    注  :
*******************************************************************************/
void ICCard_ServiceDataRepairSignDataToPC(void)
{ 
    ProtocolACK ACK_val;
    STDRIVERSIGN stDriverSign;///////////////
    u8 channel = CHANNEL_DATA_1|CHANNEL_DATA_2;
    if(0==communicatio_GetMudulState(COMMUNICATE_STATE))
    {
        s_stICCardAttrib.repairSendCnt = 0;
        return;
    }
    s_stICCardAttrib.repairSendCnt++;
    if(s_stICCardAttrib.repairSendCnt < 5)
    {
        ICCard_GetSignRecordData(s_stSignSaveIndex.sendrecordCnt+1,&stDriverSign);
        ACK_val=RadioProtocol_DriverInformationReport(channel,stDriverSign.data, stDriverSign.len);
        if(ACK_val == ACK_OK)//上传到平台成功，传下一条
        {
            s_stICCardAttrib.repairSendCnt = 0;
            ICCard_ServiceDataRepairSucceedForSaveSignIndex();
            ICCard_ReadRepairServiceDataForSendPC();
        }
        else//上传到平台失败
        {
            LZM_PublicSetOnceTimer(&s_stICTimer[IC_TIMER_REPAIR],PUBLICSECS(3),ICCard_ServiceDataRepairSignDataToPC);
        }
    }
    else
    {
        s_stICCardAttrib.repairSendCnt = 0;
    }
}
/*************************************************************
** 函数名称: ICCard_DriverSignSendDataToPlatform
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void ICCard_DriverSignSendDataToPlatform(void) 
{
    unsigned char flag = 0;
    u8 channel = CHANNEL_DATA_1 | CHANNEL_DATA_2;
    
    if(0==s_stICCardAttrib.resendCnt)return;
    flag=communicatio_GetMudulState(COMMUNICATE_STATE);
	//if(Modem_IpSock_STA[2] == MODSOC_ONLN)//上线判断
	//	flag = 1;
    //////////////////////先把旧的传出去
    if(s_stSignSaveIndex.sendrecordCnt<s_stSignSaveIndex.recordCnt||(flag==0))//
    {
        ICCard_SaveCurReportDriverSignRecord();//把当前的保存起来
        if(flag)
        {
            ICCard_ReadRepairServiceDataForSendPC();
        }
        return;
    }
    /////////////////////////////////////
    if(ACK_OK == RadioProtocol_DriverInformationReport(channel,s_stDriverSign.data, s_stDriverSign.len))
    {
        ICCard_SaveCurReportDriverSignRecord();
        ICCard_ServiceDataRepairSucceedForSaveSignIndex();        
    }
    else//失败
    {
        s_stICCardAttrib.resendCnt++;
        if(s_stICCardAttrib.resendCnt < 7)
        {
            LZM_PublicSetOnceTimer(&s_stICTimer[IC_TIMER_REPAIR],PUBLICSECS(0.3),ICCard_DriverSignSendDataToPlatform);            
        }
        else
        {
            ICCard_SaveCurReportDriverSignRecord();//把当前的保存起来
        }
    }    
}
/*************************************************************
** 函数名称: ICCard_ReportDriverSignRecord
** 功能描述: 数据解析
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void ICCard_PackAndSendReportDriverSignRecord(unsigned char status)
{
    unsigned char buffer[STDRIVERSIGN_SIZE+20];
    unsigned char len;
    if(s_stICCardAttrib.resendCnt)
    {
        LZM_PublicKillTimer(&s_stICTimer[IC_TIMER_REPAIR]);
        ICCard_SaveCurReportDriverSignRecord();
    }    
    len =0;
    buffer[len++] = status;
    memcpy(&buffer[len],s_stICCardAttrib.signTime,6);
    len +=6;    
    //if(DRIVER_SIGN_IN == status)
    {
        buffer[len++] = 0x00;//读卡结果
        ////////////////////
        buffer[len++] = s_stICCardAttrib.stDriver.NameLen;
        memcpy(&buffer[len],s_stICCardAttrib.stDriver.Name,s_stICCardAttrib.stDriver.NameLen);        
        len += s_stICCardAttrib.stDriver.NameLen;
        //////////////////////////
        memcpy(&buffer[len],s_stICCardAttrib.stDriver.DriverID,20);
        len +=20;
        ////////////////
        buffer[len++] = s_stICCardAttrib.stDriver.OrganizationLen;
        memcpy(&buffer[len],s_stICCardAttrib.stDriver.Organization,s_stICCardAttrib.stDriver.OrganizationLen);
        len += s_stICCardAttrib.stDriver.OrganizationLen;
        //////////////////
        memcpy(&buffer[len],s_stICCardAttrib.stDriver.ValidTime,4);
        len += 4;
    } 
		if(DRIVER_SIGN_PASSIVITY == status)
		{
			ICCard_Transmit_E_Signature(&buffer[len]);//加上电子签名
			len += 8;
		}
    memset(&s_stDriverSign,0,STDRIVERSIGN_SIZE);
    s_stDriverSign.len = len;
    if(s_stDriverSign.len>ICCARD_DRIVER_SIGN_DATA_LEN_MAX)
    {
        s_stDriverSign.len= ICCARD_DRIVER_SIGN_DATA_LEN_MAX;
    }
    memcpy(s_stDriverSign.data,buffer,s_stDriverSign.len);
    /////////////////////
    s_stICCardAttrib.resendCnt = 1;
    ICCard_DriverSignSendDataToPlatform();
}
/*************************************************************
** 函数名称: ICCard_SaveRegisterTypeDriver
** 功能描述: 数据解析
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void ICCard_SaveRegisterTypeDriver(unsigned char status,unsigned long time)
{
    unsigned char len;
    unsigned char buffer[REGISTER_DRIVER_STEP_LEN];    
    memcpy(buffer,s_stICCardAttrib.signTime,6);
    len = 6;
    if(DRIVER_SIGN_IN==status)
    {
        memcpy(&buffer[len],s_stICCardAttrib.stDriver.DriverID,18);
        len+=18;
        buffer[len++] = status;
    }
    else//先把前一次的签到进行签退
    {
        memcpy(&buffer[len],s_stICCardAttrib.preDriverID,18);
        len+=18;
        buffer[len++] = status;
    }    
    Register_Write2( REGISTER_TYPE_DRIVER, buffer, (REGISTER_DRIVER_STEP_LEN-5),time);
}
/*************************************************************
** 函数名称: ICCard_CheckAndDisposeSignInAndOut
** 功能描述: 数据解析
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void ICCard_SaveAndReportDriverSignRecord(unsigned char status,unsigned long time)
{
    if(DRIVER_SIGN_IN!=status&&DRIVER_SIGN_OUT!=status&&DRIVER_SIGN_PASSIVITY!=status)return;
    ICCard_SaveRegisterTypeDriver(status,time);
    ICCard_PackAndSendReportDriverSignRecord(status);
}
/*************************************************************
** 函数名称: ICCard_PackReportDriverInfo
** 功能描述: 打包当前登签信息
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
unsigned char ICCard_PackReportDriverInfo(unsigned char *pBuffer,unsigned char *pBuffer_Temp)
{
		u8	len = s_stDriverSign.len;
    if(len)
    {
#if (ICCARD_JTD == ICCARD_SEL)
		{	memcpy(pBuffer_Temp,s_stDriverSign.data,len);
			memcpy(pBuffer_Temp+len,pBuffer+1,8);//加上电子签名
			*pBuffer_Temp = 0x04;//状态位 被动上报  04
			len += 8;
		}
#else
		{
			memcpy(pBuffer_Temp,s_stDriverSign.data,s_stDriverSign.len);
		}
#endif
    }
    return len;
}

/*************************************************************
** 函数名称: ICCard_ShowAndPlayAlarmStr
** 功能描述: 数据解析
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void ICCard_ShowAndPlayStr(const u8 *StrPtr)
{
    Public_PlayTTSVoiceStr(StrPtr);
    Public_ShowTextInfo((char * )StrPtr,PUBLICSECS(6));
}
/*************************************************************
** 函数名称: ICCard_ShowAndPlayAlarmStr
** 功能描述: 数据解析
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void ICCard_ShowAndPlayAlarmStr(const u8 *StrPtr)
{
    Public_PlayTTSVoiceAlarmStr(StrPtr);
    Public_ShowTextInfo((char * )StrPtr,PUBLICSECS(6));
}
/*************************************************************
** 函数名称: ICCard_ShowSignInDriverInformation
** 功能描述: 数据解析
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void ICCard_ShowSignInDriverInfoAndPlay(void)
{
    char LCDDisplayBuf[50];
    unsigned char len;
    Public_PlayTTSVoiceStr("驾驶员签到");
    strcpy(LCDDisplayBuf,"签到 驾驶证号码:");
    len=strlen(LCDDisplayBuf);
    memcpy(&LCDDisplayBuf[len],s_stICCardAttrib.stDriver.DriverID,18);
    LCDDisplayBuf[18+len]= 0;
    Public_ShowTextInfo(LCDDisplayBuf,PUBLICSECS(6));
}
/*************************************************************
** 函数名称: ICCard_UpdataCurSignParamAndShow
** 功能描述: 数据解析
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void ICCard_UpdataCurSignParamAndShow(void)
{
    EepromPram_WritePram(E2_CURRENT_LICENSE_ID,s_stICCardAttrib.stDriver.DriverID,18);
    s_stICCardAttrib.presignstatus = s_stICCardAttrib.signstatus;
    memcpy(s_stICCardAttrib.preDriverID,s_stICCardAttrib.stDriver.DriverID,18);
    if(DRIVER_SIGN_IN==s_stICCardAttrib.signstatus)
    {
        TiredDrive_Login();   
        ICCard_ShowSignInDriverInfoAndPlay();        
    }
    else
    if(DRIVER_SIGN_OUT==s_stICCardAttrib.signstatus)
    {
        TiredDrive_Logout();        
        ICCard_ShowAndPlayStr("驾驶员签退");
    }
}
/*************************************************************
** 函数名称: ICCard_CheckAndDisposeSignInAndOut
** 功能描述: 数据解析
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void ICCard_DisposeSignInAndOut(void)
{
    unsigned long time;
    time = RTC_GetCounter();
    Public_ConvertNowTimeToBCDEx(s_stICCardAttrib.signTime);
    ///////////////////
    if((DRIVER_SIGN_IN==s_stICCardAttrib.signstatus)&&(DRIVER_SIGN_IN==s_stICCardAttrib.presignstatus))
    {
        ICCard_SaveAndReportDriverSignRecord(DRIVER_SIGN_OUT,time);
        /////////////////////////////
        ICCard_SaveAndReportDriverSignRecord(s_stICCardAttrib.signstatus,time+1);
    }
    else
    {
        ICCard_SaveAndReportDriverSignRecord(s_stICCardAttrib.signstatus,time);
    } 
    //////////////////
    ICCard_UpdataCurSignParamAndShow();
}
/*************************************************************
** 函数名称: ICCard_CheckAndDisposeSignInAndOut
** 功能描述: 数据解析
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void ICCard_CheckAndDisposeSignInAndOut(void)
{
    if(SpeedMonitor_GetCurRunStatus())
    {
        LZM_PublicSetOnceTimer(&s_stICTimer[IC_TIMER_TASK], PUBLICSECS(1), ICCard_CheckAndDisposeSignInAndOut);
    }
    else
    {
       ICCard_DisposeSignInAndOut(); 
    }
}
/*************************************************************
** 函数名称: ICCard_CheckAndDisposeInsertOrPullOutResult
** 功能描述: 数据解析
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void ICCard_CheckAndDisposeInsertAndOutResult(void)
{
    switch(s_stICCardAttrib.errCode)
    {
        case ICCARD_ERR_NULL_EWAYBILL://
            ICCard_ShowAndPlayAlarmStr("电子运单卡已插入");
            break;
        case ICCARD_ERR_NULL_MANAGE://
            ICCard_ShowAndPlayAlarmStr("管理员卡已生效");
            break;
        case ICCARD_ERR_NULL_MANAGE_EX://
            ICCard_ShowAndPlayAlarmStr("在<高级设置>菜单下插入管理员卡才有效");            
            break;
        case ICCARD_ERR_CMD_UNKOWN://未知命令
            break;
        case ICCARD_ERR_TYPE_OUT://卡类型错误
            break;
        case ICCARD_ERR_PULL_OUT_LEN://数据长度错误
            ICCard_ShowAndPlayAlarmStr("数据长度错误");
            break;
        case ICCARD_ERR_TYPE_INSERT://卡类型错误
            ICCard_ShowAndPlayAlarmStr("类型错误" );
            break;
        case ICCARD_ERR_INSERT_NONSUPPORT://卡不支持或无效卡
            ICCard_ShowAndPlayAlarmStr("读卡出错或无效卡" );
            break;
        case ICCARD_ERR_INSERT_LEN://数据长度错误
            ICCard_ShowAndPlayAlarmStr("读卡出错或无效卡" );
            break;
        case ICCARD_ERR_INSERT_EMPTY://空卡
            ICCard_ShowAndPlayAlarmStr("该卡为空卡" );
            break;
        case ICCARD_ERR_INSERT_VERIFY://数据校验出错
            ICCard_ShowAndPlayAlarmStr("数据校验出错" );
            break;        
        case ICCARD_ERR_INSERT_TYPE_UNKOWN://无法识别
            ICCard_ShowAndPlayAlarmStr("无法识别" );
            break;
        case ICCARD_ERR_INSERT_DATA_FORMAT://数据格式错误
            ICCard_ShowAndPlayAlarmStr("数据格式错误" );
            break;
        case ICCARD_ERR_NULL_DRIVER:
            if((DRIVER_SIGN_OUT==s_stICCardAttrib.signstatus)&&(DRIVER_SIGN_OUT==s_stICCardAttrib.presignstatus))
            {
                //已签退,则不需要做提示
                break;
            }
            else
            if((DRIVER_SIGN_IN==s_stICCardAttrib.signstatus)&&(DRIVER_SIGN_IN==s_stICCardAttrib.presignstatus))
            {
                if(0==Public_CheckArrayValIsEqual(s_stICCardAttrib.stDriver.DriverID, s_stICCardAttrib.preDriverID, 18))
                {
                    //同一张卡已签退,则不需要做提示
                    break;
                }
            }
            ///////////////////////
            if(SpeedMonitor_GetCurRunStatus())
            {
                ICCard_ShowAndPlayAlarmStr("行驶中禁止插拔卡");
            }            
            ICCard_CheckAndDisposeSignInAndOut();
            break;
        default:
            break;
    }
}
/*************************************************************
** 函数名称: ICCard_DisposeExtractEwaybill
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
unsigned char ICCard_VerifyAscllValue(const u8 * InBuf, u16 InLength)
{
    u8 ret = 0 ;
    for( u16 i = 0 ; i < InLength ; i++ )
    {
        if((InBuf[i]&&InBuf[i]<0x20)||(InBuf[i]>'~'))
        {
            ret  = 1;
            break;
        }
    }
    return ret ;
}
/*************************************************************
** 函数名称: ICCard_DisposeExtractEwaybill
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void ICCard_DisposeExtractEwaybill(unsigned char * pInBuf, unsigned short InLength)
{
    unsigned long tmplen;
    u8 channel = CHANNEL_DATA_1;//为编译通过而添加
    tmplen=Public_ConvertBufferToLong(pInBuf);
    if(tmplen>InLength)return;
    RadioProtocol_ElectricReceiptReport(channel,&pInBuf[4], tmplen);
}

/*************************************************************
** 函数名称: ICCard_DisposeExtractDriver808_2011
** 功能描述: 从接收的字节流里，提出数据解析到808协议驾驶员对应的结构体相应的域中 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
unsigned char ICCard_DisposeExtractDriver808_2011( unsigned char * pInBuf, unsigned char InLength, ST_DRIVER_REPORT  * stDriver  )
{
    unsigned char i,temp;
    unsigned char len;
    unsigned char *buffer;
    memset(stDriver,0,SIZE_ST_DRIVER_REPORT);
    buffer=pInBuf;
    len = 0;
    //len++;/****  协议: 卡用途种类 驾驶员 ***/
    /*****************驾驶员姓名长度 n  ,最长10字节********************/
    stDriver->NameLen = buffer[len++];
    if(stDriver->NameLen > 20 )return 1;
    /*****************驾驶员姓名********************/
    for( i = 0 ; i <  stDriver->NameLen ; i++ )
    {
        stDriver->Name[i] = buffer[len++];
    }
    /*****************驾驶员身份证编码 长度为20********************/
    for( i = 0 ; i <  20 ; i++ )
    {
        stDriver->DriverID[ i ] = buffer[len++];
    }
    ///////////////////
    if(ICCard_VerifyAscllValue(stDriver->DriverID,20)) return 2;//
    /*****************从业资格证号 长度为40********************/
    for( i = 0 ; i <  40 ; i++ )
    {
        stDriver->QCID[ i ] = buffer[len++];
    }
    ////////////////////
    if(ICCard_VerifyAscllValue(stDriver->QCID,40))return 4;//

    /*****************发证机构名称长度 ,最长30字节 ********************/
    stDriver->OrganizationLen = buffer[len++];
    if( stDriver->OrganizationLen > 40 )return 5;//
    /*****************发证机构名称长度 ********************/
    for( i= 0 ;  i <  stDriver->OrganizationLen ; i++ )
    {
        stDriver->Organization[ i ]= buffer[len++];
    }    
    /*****************驾驶员代码长度 ********************/
    temp = buffer[len++];
    /*****************驾驶员代码 ********************/
    if(temp<20&&temp)//判断是否有效
    {
        len +=  temp;//跳过驾驶员代码
        ///////////////////
        /*****************机动车驾驶证有效期*******************/
        for( i= 0 ;  i <  4 ; i++ )
        {
            stDriver->ValidTime[i]= buffer[len++];
        }
    }
    else
    {
        stDriver->ValidTime[0]=0x20;
        stDriver->ValidTime[1]=0x99;
        stDriver->ValidTime[2]=0x12;
        stDriver->ValidTime[3]=0x31;
    }
    return 0;
}
/*************************************************************
** 函数名称: ICCard_DisposeExtractDriverRecorder2012
** 功能描述: 从接收的字节流里， 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
unsigned char ICCard_DisposeExtractDriverRecorder2012( unsigned char * pInBuf, unsigned char InLength, ST_DRIVER_REPORT *stDriver)
{
    char strDriverName[]= "驾驶员"; //
    char strOrganization[]="发证机构";
    unsigned char i;
    unsigned char len;
    unsigned char *buffer;
    memset(stDriver,0,SIZE_ST_DRIVER_REPORT);
    buffer=pInBuf;
    len = 0;
    //len++;/****  协议: 卡用途种类 驾驶员 ***/
    len +=31;//保留字节
    /*****************驾驶员身份证编码 长度为20********************/
    for( i = 0 ; i <  18 ; i++ )
    {
        stDriver->DriverID[ i ] = buffer[len++];
    }
    ///////////////////
    if(ICCard_VerifyAscllValue(stDriver->DriverID,20)) return 2;//    
    /*****************机动车驾驶证有效期*******************/
    stDriver->ValidTime[0]=0x20;//有效期补年份YYYY
    for( i= 0 ;  i <  3 ; i++ )
    {
        stDriver->ValidTime[1+i]= buffer[len++];
    }    
    /*****************从业资格证号 长度为40********************/
    for( i = 0 ; i <  18 ; i++ )
    {
        stDriver->QCID[ i ] = buffer[len++];
    }
    ////////////////////
    if(ICCard_VerifyAscllValue(stDriver->QCID,40))return 4;//
    //////////添加固定内容///////
    /*****************驾驶员姓名长度 n  ,最长10字节********************/
    stDriver->NameLen = sizeof(strDriverName)-1;//减去结束符
    /*****************驾驶员姓名********************/
    strcpy((char *)stDriver->Name,strDriverName);
    /*****************发证机构名称长度 ,最长30字节 ********************/    
    stDriver->OrganizationLen =sizeof(strOrganization)-1;
    /*****************发证机构名称 ********************/
    strcpy((char *)stDriver->Organization,strOrganization);  
    ////////////////
    return 0;
}
/********************************************************************
** 函数名称: ICCard_DisposeExtractDriverJTB
** 功能描述: 将输入的字节流根据交通部驾驶员数据格式
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
unsigned char ICCard_DisposeExtractDriverJTB(u8 *pInBuf, u8 InLength,ST_DRIVER_REPORT *stDriver)
{
    unsigned char i;
    unsigned char len;
    unsigned char *buffer;
    memset(stDriver,0,SIZE_ST_DRIVER_REPORT);
    buffer=pInBuf;
    len = 0;
	/*****************驾驶员姓名长度 n********************/
    stDriver->NameLen = buffer[len++] ;  
	/*****************驾驶员姓名********************/
    for( i = 0 ; i <  stDriver->NameLen ; i++ )
    {
		stDriver->Name[i] =  buffer[len++] ;  
    }    
	/*****************从业资格证号 长度为20********************/
    for( i = 0 ; i <  20 ; i++ )
    {
        stDriver->DriverID[i] = buffer[len++] ; 
        stDriver->QCID[i]     = stDriver->DriverID[i];  
    }
	/*****************发证机构名称长度 ********************/
    stDriver->OrganizationLen = buffer[len++] ; 
	/*****************发证机构名称长度 ********************/
    for( i= 0 ;  i <  stDriver->OrganizationLen ; i++ )
    {
        stDriver->Organization[ i ]= buffer[len++] ; 
    }	
	/*****************机动车驾驶证有效期*******************/
	
    for( i= 0 ;  i <  4 ; i++ )
    {
        stDriver->ValidTime[i]= buffer[len++] ; 
    }
	/*****************标准扩展预留********************/
    return 0;
}
/*************************************************************
** 函数名称: ICCard_DisposeExdeviceData
** 功能描述: 数据解析
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
unsigned char ICCard_DisposeExdeviceData(unsigned char * pInBuf, unsigned short InLength)
{
    ST_DRIVER_REPORT stTmpDriver;
    unsigned char result;
    unsigned char *buffer;
    unsigned char usage;
    buffer  = pInBuf;
    usage   = buffer[0];    
    switch(usage)
    {
        case ICCARD_USAGE_EWAYBILL:
            ICCard_DisposeExtractEwaybill(&buffer[1],InLength-1);
            result = ICCARD_ERR_NULL_EWAYBILL;
            break;
        case ICCARD_USAGE_MANAGE:
            if(BrushAdministratorCardResponse(&buffer[1]))
            {
                result = ICCARD_ERR_NULL_MANAGE;
            }
            else
            {
                result = ICCARD_ERR_NULL_MANAGE_EX;
            }
            break;
        case ICCARD_USAGE_DRIVER_808_2011:            
            if(0==ICCard_DisposeExtractDriver808_2011(&buffer[1],InLength-1,&stTmpDriver))
            {
                s_stICCardAttrib.signstatus = DRIVER_SIGN_IN;//签到
                memcpy(&s_stICCardAttrib.stDriver,&stTmpDriver,SIZE_ST_DRIVER_REPORT);
            }
            else
            {
                result = ICCARD_ERR_INSERT_DATA_FORMAT;
            }
            break;
        case ICCARD_USAGE_UNDEFINE:
            result = ICCARD_ERR_INSERT_TYPE_UNKOWN;
            break;
        default:
            if(0==ICCard_DisposeExtractDriverRecorder2012(&buffer[1],InLength-1,&stTmpDriver))
            {
                s_stICCardAttrib.signstatus = DRIVER_SIGN_IN;//签到
                memcpy(&s_stICCardAttrib.stDriver,&stTmpDriver,SIZE_ST_DRIVER_REPORT);
            }
            else
            {
                result = ICCARD_ERR_INSERT_DATA_FORMAT;
            }
            break;
    }
    return result;
}
/*************************************************************
** 函数名称: ICCard_ProtocolParseExdevice
** 功能描述: 数据解析
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
unsigned char ICCard_ProtocolParseExdevice(ST_PROTOCOL_EXDEVICE stExdevice, unsigned short datalen)
{
    unsigned char templen;
    unsigned char result;    
    result = ICCARD_ERR_NULL_DRIVER;
    ///////////////////////
    switch(stExdevice.Cmd)
    {
        case ICCARD_CMD_INSERT:
            templen = datalen - 8;           
            if(stExdevice.Data[0])//IC卡读卡器都不支持的卡片
            {
                result = ICCARD_ERR_INSERT_NONSUPPORT;
            }
            else
            if(ICCARD_DATA_LEN_READ != templen)//
            {
                result = ICCARD_ERR_INSERT_LEN;
            }
            else
            if((0x00==stExdevice.Data[2])&&(0x00==stExdevice.Data[3]))//
            {
                result = ICCARD_ERR_INSERT_EMPTY;
            }
            else
            if((0xff==stExdevice.Data[2])&&(0xff==stExdevice.Data[3]))//
            {
                result = ICCARD_ERR_INSERT_EMPTY;
            }
            else
            {
                s_stICCardAttrib.firstcheckflag = 0;
                #ifdef ICCARD_VERIFY_ENABLE                    
                u8 verify = ICCardBase_VerifyCalcXor( &stExdevice.Data[1] , templen-1  ); 
                if(stExdevice.Data[templen] != verify )
                {
                    result = ICCARD_ERR_INSERT_VERIFY;
                    break;
                }
                #endif                    
                result = ICCard_DisposeExdeviceData(&stExdevice.Data[1],templen);
            }
            break;
       case ICCARD_CMD_PULL_OUT:   
            if(8 == datalen)//
            {
                if(1==s_stICCardAttrib.firstcheckflag)
                {
                    result = ICCARD_ERR_TYPE_OUT;
                }
                else
                {
                    s_stICCardAttrib.signstatus = DRIVER_SIGN_OUT;//签退
                    memset(s_stICCardAttrib.stDriver.DriverID,0x30,18);
                }
            }
            else
            {
               result = ICCARD_ERR_PULL_OUT_LEN; 
            }
            break;
        default:
            result = ICCARD_ERR_CMD_UNKOWN;
            break;
    }
    return result;
}
/*************************************************************
** 函数名称: ICCard_DisposeExternalDriverSign
** 功能描述: 处理雄帝或交通部规定的驾驶员卡
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void ICCard_DisposeExternalDriverSign(void)
{
    s_stICCardAttrib.errCode = ICCARD_ERR_NULL_DRIVER;
    LZM_PublicSetOnceTimer(&s_stICTimer[IC_TIMER_TASK], LZM_AT_ONCE, ICCard_CheckAndDisposeInsertAndOutResult);
}
/*************************************************************
** 函数名称: ICCard_DisposeJTBCardDriverSignIn
** 功能描述: 处理雄帝或交通部规定的驾驶员卡
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void ICCard_DisposeJTBCardDriverSignIn(unsigned char *pBuffer, unsigned short datalen)
{
    ST_DRIVER_REPORT stTmpDriver;
    ICCard_DisposeExtractDriverJTB(pBuffer,datalen,&stTmpDriver);        
    memcpy(&s_stICCardAttrib.stDriver,&stTmpDriver,SIZE_ST_DRIVER_REPORT);
    s_stICCardAttrib.signstatus = DRIVER_SIGN_IN;//签到
    s_stICCardAttrib.firstcheckflag = 0;
    ICCard_DisposeExternalDriverSign();
}
/*************************************************************
** 函数名称: ICCard_DisposeJTBCardDriverSignOut
** 功能描述: 处理雄帝或交通部规定的驾驶员卡
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void ICCard_DisposeJTBCardDriverSignOut(unsigned char *pBuffer, unsigned short datalen)
{
    ST_DRIVER_REPORT stTmpDriver;
    ICCard_DisposeExtractDriverJTB(pBuffer,datalen,&stTmpDriver);        
    memcpy(&s_stICCardAttrib.stDriver,&stTmpDriver,SIZE_ST_DRIVER_REPORT);
    s_stICCardAttrib.signstatus = DRIVER_SIGN_OUT;//签退
    s_stICCardAttrib.firstcheckflag = 0;
    ICCard_DisposeExternalDriverSign();
}

/*************************************************************
** 函数名称: ICCard_DisposeJTBCardDriverSignPassivity
** 功能描述: 广州二汽 被动应答电子证据信息
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void ICCard_DisposeJTBCardDriverSignPassivity(unsigned char *pBuffer, unsigned short datalen)
{
		ST_DRIVER_REPORT stTmpDriver;
		ICCard_DisposeExtractDriverJTB(pBuffer,datalen,&stTmpDriver); 			 
		memcpy(&s_stICCardAttrib.stDriver,&stTmpDriver,SIZE_ST_DRIVER_REPORT);
		s_stICCardAttrib.signstatus = DRIVER_SIGN_PASSIVITY;//被动应答
		s_stICCardAttrib.firstcheckflag = 0;
		ICCard_DisposeExternalDriverSign();
}

/*************************************************************
** 函数名称: ICCard_DisposeExternalDriverSignIn
** 功能描述: 处理外部显示屏驾驶员卡签到
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void ICCard_DisposeExternalDriverSignIn(unsigned char *pBuffer, unsigned short datalen)
{
    ST_DRIVER_REPORT stTmpDriver;
    if(ICCard_DisposeExtractDriver808_2011(pBuffer,datalen,&stTmpDriver))return;
    memcpy(&s_stICCardAttrib.stDriver,&stTmpDriver,SIZE_ST_DRIVER_REPORT);
    s_stICCardAttrib.signstatus = DRIVER_SIGN_IN;//签到
    s_stICCardAttrib.firstcheckflag = 0;   
    ICCard_DisposeExternalDriverSign();
}
/*************************************************************
** 函数名称: ICCard_DisposeExternalCardDriverSignOut
** 功能描述: 处理外部驾驶员卡签退
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void ICCard_DisposeExternalCardDriverSignOut(void)
{
    if(DRIVER_SIGN_IN != s_stICCardAttrib.presignstatus)return;
    if(DRIVER_SIGN_OUT == s_stICCardAttrib.signstatus)return;    
    memset(s_stICCardAttrib.stDriver.DriverID,0x30,18);
    s_stICCardAttrib.signstatus = DRIVER_SIGN_OUT;//签退
    ICCard_DisposeExternalDriverSign();
}
/*************************************************************
** 函数名称: ICCard_DisposeRecvExdevice
** 功能描述: 数据解析
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void ICCard_DisposeRecvExdevice(ST_PROTOCOL_EXDEVICE stExdevice, unsigned short datalen)
{
    s_stICCardAttrib.errCode = ICCard_ProtocolParseExdevice(stExdevice,datalen);
    LZM_PublicSetOnceTimer(&s_stICTimer[IC_TIMER_TASK], LZM_AT_ONCE, ICCard_CheckAndDisposeInsertAndOutResult);
}
extern u8	SwipCardCh;//刷卡通道  0 二汽读卡器  1  伊爱IC卡
/*************************************************************
** 函数名称: ICCard_ProtocolParse
** 功能描述: IC卡数据解析
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void ICCard_ProtocolParse(unsigned char * pInBuf, unsigned short InLength)
{
    ST_PROTOCOL_EXDEVICE stTmpExdevice;
    unsigned char tmpBuffer[ICCARD_DATA_LEN_MAX];
    unsigned short iPosition[3],datalen=0;
    unsigned short i;
    unsigned char *buffer;
    unsigned char flag;
    if(InLength<ICCARD_DATA_LEN_MIN)return;
    buffer=pInBuf;
    flag =0;     
    for(i = 0; i<InLength; i++)
    {
        if(0x7E==buffer[i])
        {
            iPosition[flag++] = i ;
            if(2==flag)
            {
                datalen = iPosition[1]-iPosition[0]-1;
								if(datalen==0)continue;//这里接收数据有点bug,调整一下
								memcpy(tmpBuffer, &buffer[iPosition[0]+1],datalen);
								flag = 0xff;
            }
			else
			if(3==flag)
			{
				datalen = iPosition[2]-iPosition[1]-1; 
				memcpy(tmpBuffer, &buffer[iPosition[1]+1],datalen);
				flag = 0xff;
			}
			if(datalen>ICCARD_DATA_LEN_MAX)return;
			if(0xff==flag)
            {
	            //memcpy(tmpBuffer, &buffer[iPosition[0]+1],datalen);
							memset(&stTmpExdevice,0x0,sizeof(ST_PROTOCOL_EXDEVICE));
	            datalen=ICCardBase_ExtractExdevice(tmpBuffer,datalen,&stTmpExdevice);
	            if(0==datalen)return;
	            ///////////////////////
	            s_stICCardAttrib.onlinestatus = ICCARD_ONLINE_STASUS_ON;
	            Io_WriteAlarmBit(ALARM_BIT_IC_ERROR,RESET);
	            LZM_PublicKillTimer(&s_stICTimer[IC_TIMER_TIMEOUT]);                
	            ////////////////////////////////
	            //#if (ICCARD_EEYE == ICCARD_SEL)
	            //ICCard_DisposeRecvExdevice(stTmpExdevice,datalen);
	            //#elif (ICCARD_JTD == ICCARD_SEL) 
	            //ICCard_DisposeRecvExdeviceJTB(stTmpExdevice,datalen);
	            //#endif  
				if(SwipCardCh)
				{
					ICCard_DisposeRecvExdevice(stTmpExdevice,datalen);
				}
				else
				{
					ICCard_DisposeRecvExdeviceJTB(stTmpExdevice,datalen);
				}
	            break;	     
            }
        }
    }
}
/*************************************************************
** 函数名称: ICCard_SendCmdForReadDriverInfo
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
unsigned char ICCard_SendCmdForReadDriverInfo(void)
{  
    u8 TempData[2];
    return ICCard_PackAndSendData(0x43,TempData,0);
}
/*************************************************************
** 函数名称: ICCard_DisposeReadDriverInfoTimeOut
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void ICCard_DisposeReadDriverInfoTimeOut(void)
{
    ///////////////////////////////
    s_stICCardAttrib.firstcheckflag = 0;
    if(DRIVER_SIGN_IN != s_stICCardAttrib.presignstatus)return;
    if(s_stICCardAttrib.signstatus == DRIVER_SIGN_EMPTY)
    {
        s_stICCardAttrib.signstatus = DRIVER_SIGN_OUT;
        ICCard_DisposeSignInAndOut();
    }
}
/*************************************************************
** 函数名称: ICCard_DisposeReadDriverInfoTimeOut
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void ICCard_DisposeCommTimeOut(void)
{
    if(ICCardBase_GetUseIntFlag())return;
    s_stICCardAttrib.onlinestatus = ICCARD_ONLINE_STASUS_OFF;
    Io_WriteAlarmBit(ALARM_BIT_IC_ERROR,SET);
}
/*************************************************************
** 函数名称: ICCard_StartCheckCommTimeOut
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void ICCard_StartCheckCommTimeOut(void)
{
    LZM_PublicSetOnceTimer(&s_stICTimer[IC_TIMER_TIMEOUT],PUBLICSECS(4),ICCard_DisposeCommTimeOut);
}
/*************************************************************
** 函数名称: ICCard_StartCheckReadDriverInfoTimeOut
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void ICCard_StartCheckReadDriverInfoTimeOut(void)
{
    s_stICCardAttrib.firstcheckflag = 1;
    LZM_PublicSetOnceTimer(&s_stICTimer[IC_TIMER_TEST],PUBLICSECS(15),ICCard_DisposeReadDriverInfoTimeOut);
}
/*************************************************************
** 函数名称: ICCard_ReadDriverInfo
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void ICCard_ReadDriverInfo(void)
{
    if(ICCardBase_GetUseIntFlag())//通过自身CPU通过I2C读IC卡
    {
        ICCard_StartCheckReadDriverInfoTimeOut();
        return;
    }
    ////////////////////////
    if(0==ICCard_SendCmdForReadDriverInfo())
    {
        ICCard_StartCheckCommTimeOut();
        ICCard_StartCheckReadDriverInfoTimeOut();
    }
    else
    {
        LZM_PublicSetOnceTimer(&s_stICTimer[IC_TIMER_TEST],PUBLICSECS(2),ICCard_ReadDriverInfo);
    }
}
/*************************************************************OK
** 函数名称: ICCard_FlashIndexReadAndCheck
** 功能描述: 读取索引操作参数并检查是否合理
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 
             
             
*************************************************************/
void ICCard_FlashIndexReadAndCheck(void) //
{
    STDRIVERSIGN stDriverSign;
    ///////读操作参数/////////////
    ICCard_FlashIndexRead();    
    /////////////////////////////////
    if((s_stSignSaveIndex.recordCnt>DRIVERSIGN_MAX_RECORD+1)
      ||(s_stSignSaveIndex.sendrecordCnt>s_stSignSaveIndex.recordCnt)
      ||(s_stSignSaveIndex.saveflag != SAVE_INDEX_FLAG))
    {
        ICCard_SignFlashIndexInit();
    }
    ///////////////////////////////
    ICCard_CheckAndMoveSignData();
    //////////////////////////////
    if(s_stSignSaveIndex.recordCnt)
    {
        ICCard_GetSignRecordData(s_stSignSaveIndex.recordCnt,&stDriverSign);
        if((stDriverSign.len>ICCARD_DRIVER_SIGN_DATA_LEN_MAX)
        ||((stDriverSign.data[0]!=DRIVER_SIGN_IN)&&(stDriverSign.data[0]!=DRIVER_SIGN_OUT))
        ||((stDriverSign.data[0]==DRIVER_SIGN_OUT)&&(stDriverSign.len!=7)))
        {
            ICCard_SignFlashIndexInit();
            return;
        }
        //////////////////////////
        memcpy(&s_stDriverSign,&stDriverSign,STDRIVERSIGN_SIZE);
    }
    /////////////////////////////
    if(s_stSignSaveIndex.recordCnt<DRIVERSIGN_MAX_RECORD)
    {
        ICCard_GetSignRecordData(s_stSignSaveIndex.recordCnt+1,&stDriverSign);
        if(stDriverSign.len !=0xff||stDriverSign.data[0]!=0xff)
        {
            ICCard_SignFlashIndexInit();
        }
    }
}
/*************************************************************
** 函数名称: ICCard_ReadE2ParamDriverInfo
** 功能描述: 参数初始化
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void ICCard_ReadE2ParamDriverInfo(void) 
{
    unsigned char buffer[30];
    if(EepromPram_ReadPram(E2_CURRENT_LICENSE_ID, buffer))
    {
        memcpy(s_stICCardAttrib.preDriverID,buffer,18);
        memset(buffer,0x30,18);
        if(Public_CheckArrayValIsEqual(s_stICCardAttrib.preDriverID, buffer, 18))//不等于0x30
        {
            memset(buffer,0,18);
            if(Public_CheckArrayValIsEqual(s_stICCardAttrib.preDriverID, buffer, 18))//不等于0x00
            {
                s_stICCardAttrib.presignstatus = DRIVER_SIGN_IN;
                return;
            }
        }
    }
    /////////////////////////////
    s_stICCardAttrib.signstatus = DRIVER_SIGN_OUT;
    s_stICCardAttrib.presignstatus = DRIVER_SIGN_OUT;
    memset(s_stICCardAttrib.preDriverID,0x30,18);
}
/*************************************************************
** 函数名称: ICCard_CheckForVoicePrompt
** 功能描述: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char ICCard_CheckForVoicePrompt(void)
{
    if(1!=s_stICCardAttrib.voicePrompt)return 0;
    if(TiredDrive_GetLoginFlag())return 0;
    if(TiredDrive_GetCurContDriveTimeSec()>=ICCARD_PLAY_REMIND_TOTAL_TIME)return 0;
    return 1;
}
/*************************************************************
** 函数名称: ICCard_PlayVoicePromptSignIn
** 功能描述: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void ICCard_PlayVoicePromptSignIn(void)
{
    ICCard_ShowAndPlayAlarmStr("请停车后插入IC卡");
}
/*************************************************************
** 函数名称: ICCard_CheckForVoicePromptSignIn
** 功能描述: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void ICCard_CheckForVoicePromptSignIn(void)
{
    unsigned char runStatus;
    if(0==ICCard_CheckForVoicePrompt())
    {
        s_stICCardAttrib.playTimeCnt = 0;
        return;
    }
    ////////////////////
    runStatus = SpeedMonitor_GetCurRunStatus();
    if(runStatus)
    {
        if(0==s_stICCardAttrib.playTimeCnt)
        {
            s_stICCardAttrib.playflag = 1;
            s_stICCardAttrib.OnceCnt  = ICCARD_PLAY_REMIND_ONCE_TIME;
            s_stICCardAttrib.OnceNumber = 0;            
        }
    }
    else
    if(0==s_stICCardAttrib.playTimeCnt)
    {
        return;
    }    
    ///////////////////////
    s_stICCardAttrib.playTimeCnt++;
    if(s_stICCardAttrib.playTimeCnt>=ICCARD_PLAY_REMIND_GPOUP_TIME)
    {
        s_stICCardAttrib.playTimeCnt = 0;
    }
    //////////////////////////////
    if(0==s_stICCardAttrib.playflag)return;
    ///////////////////
    s_stICCardAttrib.OnceCnt++;
    if(s_stICCardAttrib.OnceCnt>=ICCARD_PLAY_REMIND_ONCE_TIME&&runStatus)
    {
        s_stICCardAttrib.OnceCnt = 0;
        //////////////////
        ICCard_PlayVoicePromptSignIn();//分组提示
        ///////////////////
        s_stICCardAttrib.OnceNumber++;        
        if(s_stICCardAttrib.OnceNumber>=ICCARD_PLAY_REMIND_ONCE_NUMBER)
        {
            s_stICCardAttrib.OnceNumber = 0;
            s_stICCardAttrib.playflag =0;
            s_stICCardAttrib.OnceCnt =0;
        }
    }
}
/*************************************************************
** 函数名称: ICCard_CheckForVoicePromptSignIn
** 功能描述: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void ICCard_UpdataParamVoicePrompt(void)
{
    unsigned char flag;
    LZM_PublicKillTimer(&s_stICTimer[IC_TIMER_CHECK]);
    if(EepromPram_ReadPram(E2_UNLOGIN_TTS_PLAY_ID, &flag))
    {
        s_stICCardAttrib.voicePrompt = flag;
        if(1==s_stICCardAttrib.voicePrompt)
        {
            LZM_PublicSetCycTimer(&s_stICTimer[IC_TIMER_CHECK],PUBLICSECS(1),ICCard_CheckForVoicePromptSignIn);
        }
    }
    else
    {
        s_stICCardAttrib.voicePrompt = 0;
    }
}
/*************************************************************
** 函数名称: ICCard_GetExternalDriverInfoForSignIn
** 功能描述: 获取驾驶员代码及姓名
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
unsigned char ICCard_GetExternalDriverInfoForSignIn(unsigned char *pBuffer)
{
    unsigned char len;
    if( s_stICCardAttrib.signstatus != DRIVER_SIGN_IN) return 0;
    len = 0;
    memcpy(&pBuffer[len],&s_stICCardAttrib.stDriver.DriverID[8],6);
    len +=6;
    pBuffer[len++]=s_stICCardAttrib.stDriver.NameLen;
    memcpy(&pBuffer[len],&s_stICCardAttrib.stDriver.Name,s_stICCardAttrib.stDriver.NameLen);
    len +=s_stICCardAttrib.stDriver.NameLen;
    return len;
}
/*************************************************************
** 函数名称: ICCard_GetExternalDriverInfo
** 功能描述: 获取驾驶员姓名及驾驶证号码
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
unsigned char ICCard_GetExternalDriverInfo(unsigned char *pBuffer)
{
    unsigned char len;
    if( s_stICCardAttrib.signstatus != DRIVER_SIGN_IN) return 0;
    len = 0;
    memcpy(&pBuffer[len],&s_stICCardAttrib.stDriver.Name,s_stICCardAttrib.stDriver.NameLen);
    len +=s_stICCardAttrib.stDriver.NameLen;
    pBuffer[len++]=',';
    memcpy(&pBuffer[len],s_stICCardAttrib.stDriver.DriverID,18);
    len +=18;
    return len;
}
/*************************************************************
** 函数名称: ICCard_GetExternalDriverInfo
** 功能描述: 获取驾驶员姓名
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
unsigned char ICCard_GetExternalDriverName(unsigned char *pBuffer)
{
        unsigned char len;
        
	if( s_stICCardAttrib.signstatus != DRIVER_SIGN_IN) return 0;
        len=s_stICCardAttrib.stDriver.NameLen;
        
	memcpy(pBuffer,&s_stICCardAttrib.stDriver.Name,len);
	return len;
}
/*************************************************************
** 函数名称: ICCard_GetExternalDriverInfo
** 功能描述: 获取驾驶员驾驶证号码
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
unsigned char ICCard_GetExternalDriverDriveID(unsigned char *pBuffer)
{
      u8 len;
	if( s_stICCardAttrib.signstatus != DRIVER_SIGN_IN) return 0;
        len=s_stICCardAttrib.stDriver.QCID[0];
	memcpy(pBuffer,&s_stICCardAttrib.stDriver.QCID[len+1],18);
	return 18;
}

/*************************************************************
** 函数名称: ICCard_TimeTask
** 功能描述: 定时器处理函数
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
FunctionalState ICCard_TimeTask(void)
{
    LZM_PublicTimerHandler(s_stICTimer,IC_TIMERS_MAX);
    ICCardBase_TimerTask();
    return ENABLE;
}
/*************************************************************
** 函数名称: ICCard_ParamInit
** 功能描述: 参数初始化
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void ICCard_TimeTaskStart(void) 
{
    memset(&s_stICCardAttrib,0,sizeof(ST_ICCARD_ATTRIB));
    /////////////////////////
    ICCard_ReadE2ParamDriverInfo();
    //////////////////////////////
    ICCard_FlashIndexReadAndCheck();
    /////////////////////
    ICCard_UpdataParamVoicePrompt();
    ///////////////////////
    if(0==ICCardBase_GetUseIntFlag())
    {
        CommICCardInit();
    }
    //////////////////////////////////////////////////////////
    LZM_PublicSetOnceTimer(&s_stICTimer[IC_TIMER_TEST],PUBLICSECS(10),ICCard_ReadDriverInfo);
    ///////////////////
    LZM_PublicSetOnceTimer(&s_stICTimer[IC_TIMER_CHECK],PUBLICSECS(25),ICCard_UpdataParamVoicePrompt);
}
/*************************************************************
** 函数名称: ICCard_ParamInit
** 功能描述: 参数初始化
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void ICCard_ParamInit(void) 
{
    LZM_PublicKillTimerAll(s_stICTimer,IC_TIMERS_MAX); 
    ////////////////////////
    memset(&s_stICCardAttrib,0,sizeof(ST_ICCARD_ATTRIB));
    /////////////////////////    
    ////////////////////////
    SetTimerTask(TIME_IC_CARD,LZM_TIME_BASE);
    //////////////////////////////////////////////////////////
    LZM_PublicSetOnceTimer(&s_stICTimer[IC_TIMER_TEST],LZM_AT_ONCE,ICCard_TimeTaskStart);
}
/******************************************************************************
**                            End Of File
******************************************************************************/

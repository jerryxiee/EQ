/*******************************************************************************
 * File Name:			Gnss_app.c 
 * Function Describe:	
 * Relate Module:		GNSS 模块控制
 * Writer:				Joneming
 * Date:				2013-12-11
 * ReWriter:			
 * Date:				
 *******************************************************************************/
#include "include.h"
//GNSS接收数据 长度缓冲区
#define	GNSS_RECV_BUFFER_SIZE           1000
#define GNSS_UPLOAD_TYPE                0x00
////////////////////////////////
#define	GNSS_BLIND_HEAD_LEN             7
////////////////////////
#define FLASH_GNSS_SAVE_STEP_LEN        800

//传输模式
enum 
{
    NONE_UPDATE     = 0,    //本地存储，不上传（默认值）；
    INTERVAL_TIME,          //按时间间隔上传；
    INTERVAL_METER,         //按距离间隔上传；     
    COUNT_TIME_STOP = 0x0B, //按累计时间上传，达到传输时间后自动停止上传；  
    COUNT_METER_STOP,       //按累计距离上传，达到距离后自动停止上传；
    COUNT_NUMBER_STOP,      //按累计条数上传，达到上传条数后自动停止上传*/
    GNSSUPLOADMODE_MAX
}E_GNSSUPLOADMODE;



//#define	GNSS_GGA_SIZE		100	
//sizeof("$GNGGA,000000.00,2309.920734,N,11325.877530,E,0,00,0.0,50.9,M,0.0,M,,,0.0*74\r\n")

typedef struct
{
    unsigned short recvdatalen;
    unsigned short datalen;	
    unsigned char nullDataCnt;
    unsigned char recvDataflag;
    unsigned char parseBuff[GNSS_RECV_BUFFER_SIZE];
    unsigned char recvBuff[GNSS_RECV_BUFFER_SIZE];
}ST_GNSS_ATTRIB;

static ST_GNSS_ATTRIB s_stGnssAttrib;

typedef	struct
{
    unsigned long preTimeVal;
    unsigned long preinterval;  
    unsigned long freq;	
    unsigned long interval;
    unsigned short reReportCnt;
    unsigned char uploadmode;
    unsigned char collectFlag;
    unsigned char freqUpFlag;
    unsigned char updataFlag;
    unsigned char resendCnt;
    unsigned char CheckBlindflag;    
}ST_GNSS_PARAM;
static ST_GNSS_PARAM s_stGnssParam;

	
const char *p_ConstSize[]=
{	
    "$GNGGA,000000.00,2309.920734,N,11325.877530,E,0,00,0.0,50.9,M,0.0,M,,,0.0*74\r\n",\
    "$GNGLL,2309.920734,N,11325.877530,E,000000.00,V,0*18\r\n",\
    "$GPGSA,A,1,,,,,,,,,,,,,0.0,0.0,0.0,0.0*32\r\n",\
    "$GNRMC,000000.00,V,2309.920734,N,11325.877530,E,,,010111,,,N*51\r\n",\
    "$GNTXT,01,01,01,ANTENNA OPEN*3B\r\n"
};
//参数0x0090定义如下：bit0为1启用GPS定位;bit1为1启用Compass定位;bit2为1启用GLONASS定位;bit3为1启用Galileo定位
#ifdef GPS_MODE_TYPE_TD3017//dxl,2013.11.28，泰斗与和芯星通的命令不一样
const char *p_startupCmd[]=
{
    "$CCSIR,2,1*4B\r\n",//单GPS定位(冷启动)
    "$CCSIR,1,1*48\r\n",//单Compass定位(冷启动)
    "$CCSIR,3,1*4A\r\n",//双模定位(冷启动)
    "$CCSIR,2,0*4A\r\n",//单GPS定位(热启动)
    "$CCSIR,1,0*49\r\n",//单Compass定位(热启动)
    "$CCSIR,3,0*4B\r\n"//双模定位(热启动)
};
#endif

#ifdef GPS_MODE_TYPE_UM330
const char *p_startupCmd[]=
{
    "$CFGSYS,h01\r\n",//单GPS定位
    "$CFGSYS,h10\r\n",//单Compass定位
    "$CFGSYS,h11\r\n",//双模定位
    "$CFGSYS,h01\r\n",//单GPS定位
    "$CFGSYS,h10\r\n",//单Compass定位
    "$CFGSYS,h11\r\n"//双模定位
};
#endif
	
//波利特修改指令
//4800--9600--19200--38400--57600 -- 115200 
const char *p_BpCmd[]=
{ \
    "$PCAS01,0*1C\r\n",\
    "$PCAS01,1*1D\r\n",\
    "$PCAS01,2*1E\r\n",\
    "$PCAS01,3*1F\r\n",\
    "$PCAS01,4*18\r\n",\
    "$PCAS01,5*19\r\n"
};
const u32 Baudrate[]={4800,9600,19200,38400,57600,115200};
//模块输出的数据更新频率
//500ms --1000ms --2000ms--3000ms--4000ms 更新一次
#ifdef GPS_MODE_TYPE_TD3017
const char *p_DatFreCmd[]=
{\
    "$PCAS02,500*1A\r\n",
    "$PCAS02,1000*2E\r\n",
    "$PCAS02,2000*2D\r\n",
    "$PCAS02,3000*2C\r\n",
    "$PCAS02,4000*2B\r\n"
};
#endif
#ifdef GPS_MODE_TYPE_UM330
const char *p_DatFreCmd[]=
{\
    "$CFGMSG,0,,1\r\n",//第2个参数为空表示设置该类别(NMEA)所有的语句,输出频率最高只能是1秒1次,
    "$CFGMSG,0,,1\r\n",//1秒1次
    "$CFGMSG,0,,2\r\n",//2秒1次
    "$CFGMSG,0,,3\r\n",//3秒1次
    "$CFGMSG,0,,4\r\n" //4秒1次
};
#endif

enum 
{
    GNSS_TIMER_TASK,                        //    
    GNSS_TIMER_TEST,						//  
    GNSS_TIMER_RESEND,						//  
    GNSS_TIMERS_MAX
}E_GNSSTIME;

static LZM_TIMER s_stGnssTimer[GNSS_TIMERS_MAX];
///////////////////////////////

//*************外部变量***************
static HISTORY_AREA_STATUS s_stGnssStatus;
//结构
typedef struct
{
    u8	ReportFlag;//盲区上报标志，1为要求上报或正上报，0为不要求
    s16	ReportSector ;//盲区补报的扇区
    s16	ReportStep;//盲区补报的步数
}ST_GNSS_SAVE;

static ST_GNSS_SAVE s_stGnssSave;
/*********************************************************************
//函数名称	:Gnss_EraseBlindArea()
//功能		:擦除盲区区域
//		:
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void Gnss_EraseBlindArea(void)
{
    u16 i;
    u32 Address;
    u8  Buffer[3];

    //****************擦除扇区*********************
    for(i=FLASH_GNSS_SAVE_START_SECTOR; i<FLASH_GNSS_SAVE_END_SECTOR; i++)
    {
        Address = i*FLASH_ONE_SECTOR_BYTES;
        sFLASH_EraseSector(Address);
        IWDG_ReloadCounter();//喂狗
    }
    //****************初始化变量*********************
    s_stGnssStatus.StartSector	= FLASH_GNSS_SAVE_START_SECTOR;		//开始扇区
    s_stGnssStatus.EndSector	= FLASH_GNSS_SAVE_END_SECTOR;		//结束扇区
    s_stGnssStatus.CurrentSector = s_stGnssStatus.StartSector;	//当前扇区
    s_stGnssStatus.OldestSector	= s_stGnssStatus.StartSector;	//最老扇区
    s_stGnssStatus.LoopCount	= 0;					//循环计数
    s_stGnssStatus.CurrentStep 	= 0;					//当前步数
    s_stGnssStatus.SectorStep 	= FLASH_ONE_SECTOR_BYTES / FLASH_GNSS_SAVE_STEP_LEN;	//单个扇区总步数 

    //从第0步开始
    Buffer[0] = 0;
    Buffer[1] = 0;
    FRAM_BufferWrite(FRAM_GNSS_SAVE_STEP_ADDR,Buffer,2);//盲区步数保存至FRAM
}
/*********************************************************************
//函数名称	:Gnss_CheckBlindArea
//功能		:检查盲区区域
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:正确返回SUCCESS,有误返回ERROR
//备注		:
*********************************************************************/
ErrorStatus  Gnss_CheckBlindArea(void)
{
    u32 	i,j,k;
    u32 	Address;
    u32 	MinTimeCount;
    u32 	MaxTimeCount;
    u32 	TimeCount;
    u8  	Buffer[FLASH_GNSS_SAVE_STEP_LEN+1];
    u8  	VerifySum = 0;
    u8      TempByte;
    u8	flag;
    unsigned short datalen;
    TIME_T	tt;    
    
    //***************************初始化变量*****************************************
    s_stGnssStatus.StartSector	= FLASH_GNSS_SAVE_START_SECTOR;		//开始扇区
    s_stGnssStatus.EndSector	= FLASH_GNSS_SAVE_END_SECTOR;		//结束扇区
    s_stGnssStatus.CurrentSector = s_stGnssStatus.StartSector;	//当前扇区
    s_stGnssStatus.OldestSector	= s_stGnssStatus.StartSector;	//最老扇区
    s_stGnssStatus.LoopCount	= 0;					//循环计数
    s_stGnssStatus.CurrentStep 	= 0;					//当前步数
    s_stGnssStatus.SectorStep 	= FLASH_ONE_SECTOR_BYTES / FLASH_GNSS_SAVE_STEP_LEN;	//单个扇区总步数        

    //***************************检查历史轨迹和卫星数据包区域内的数据*************************************
    //初始化最小值和最大值
    MinTimeCount = 0xFFFFFFFF;
    MaxTimeCount = 0;
    flag = 0;

    //检查每个扇区的数据包，开始扇区--->结束扇区
    for(k=s_stGnssStatus.StartSector; k<s_stGnssStatus.EndSector; k++)
    {
        IWDG_ReloadCounter();//喂狗
        //检查每一步的数据包内容
        for(j=0; j<s_stGnssStatus.SectorStep; j++)
        {
            //计算当前地址
            Address = k*FLASH_ONE_SECTOR_BYTES + j*FLASH_GNSS_SAVE_STEP_LEN;
            //读当前步数的时间字符串
            sFLASH_ReadBuffer(Buffer,Address,4);
            //小端排序
            TimeCount = Public_ConvertBufferToLong(Buffer);
            //检查是否找到有效的时间标签
            if(TimeCount != 0xFFFFFFFF)	//找到有效时间标签
            {
                Gmtime(&tt, TimeCount);
                if(SUCCESS == CheckTimeStruct(&tt))
                {
                    //标志置位//读该步的全部内容
                    sFLASH_ReadBuffer(Buffer,Address,FLASH_GNSS_SAVE_STEP_LEN);
                    //校验位置信息数据是否正确
                    datalen = Public_ConvertBufferToShort(&Buffer[4]);
                    VerifySum =0;
                    if(datalen+GNSS_BLIND_HEAD_LEN<=FLASH_GNSS_SAVE_STEP_LEN)
                    {
                        VerifySum = Public_GetSumVerify(&Buffer[GNSS_BLIND_HEAD_LEN], datalen);
                    }                    
                    if((VerifySum != Buffer[6])||(datalen== 0)||( datalen+GNSS_BLIND_HEAD_LEN>FLASH_GNSS_SAVE_STEP_LEN))////校验错误
                    {
                        flag = 0;
                        Gnss_EraseBlindArea();
                        //更新盲区位置
                        return SUCCESS;
                    }
                    flag = 1;
                    //判定时间最老扇区
                    if(TimeCount < MinTimeCount)
                    {
                        MinTimeCount = TimeCount;
                        s_stGnssStatus.OldestSector = k; //最老扇区
                    }
                    //判定时间最新扇区
                    if(TimeCount > MaxTimeCount)
                    {
                        MaxTimeCount = TimeCount;
                        s_stGnssStatus.CurrentStep = j;//最新步数
                        s_stGnssStatus.CurrentSector = k;//最新扇区号
                    }
                }
                else
                {
                    flag = 0;
                    Gnss_EraseBlindArea();
                    //更新盲区位置
                    return SUCCESS;
                }
            }
            else	//没有找到有效时间标签，跳出循环查找下一个扇区
            {

            }
        }
    }

    if(0 == flag)
    {

    }
    else
    {
        s_stGnssStatus.CurrentStep++;

    }
    //检查当前步数是否超出当前扇区
    if(s_stGnssStatus.CurrentStep >= s_stGnssStatus.SectorStep)
    {
        //指向下一个扇区的起始位置
        s_stGnssStatus.CurrentStep = 0;
        s_stGnssStatus.CurrentSector++;

        //检查当前扇区是否超出区域界外
        if(s_stGnssStatus.CurrentSector >= s_stGnssStatus.EndSector)
        {
            //重新指向起始扇区
            s_stGnssStatus.CurrentSector = s_stGnssStatus.StartSector;	
        }
    }
    //计算区域最后一步的地址
    Address = (s_stGnssStatus.EndSector-1)*FLASH_ONE_SECTOR_BYTES + (s_stGnssStatus.SectorStep-1)*FLASH_GNSS_SAVE_STEP_LEN;
    //读取最后一步的各个字节
    if(flag)
    {
        for(i=0; i<20; i++)
        {
            sFLASH_ReadBuffer(&TempByte,Address+i,1);
            //判断是否为有效字节	
            if(TempByte != 0xFF)
            {
                s_stGnssStatus.LoopCount = 1;
                break;
            }
        }
    }
    ///////////////////////
    s_stGnssParam.CheckBlindflag = 1;
    return SUCCESS;
}

/*********************************************************************
//函数名称	:Gnss_WriteOneGnssDataToFlash
//功能		:把一条信息写入FLASH
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:一条盲区数据包括：时间4字节+信息长度2字节+校验和1字节
*********************************************************************/
ErrorStatus Gnss_WriteOneGnssDataToFlash(u8 *pBuffer, unsigned short datalen)
{
    u32 Address;
    u8	Buffer[FLASH_GNSS_SAVE_STEP_LEN];
    u8	i;
    u8	j;
    u32	ttCount;
    u32	TimeCount;
    u8	VerifyCode;
    s16	Sector;
    s16	Step;
    TIME_T	tt;
    if(0==s_stGnssParam.CheckBlindflag)
    {
        Gnss_CheckBlindArea();
    }
    ttCount = RTC_GetCounter();
    //判断当前步数时间是否大于上一步数时间
    if((s_stGnssStatus.CurrentStep > 0)||
    (s_stGnssStatus.CurrentSector > s_stGnssStatus.StartSector)||
    (s_stGnssStatus.LoopCount > 0)) //存储区已有数据
    {

        Sector = s_stGnssStatus.CurrentSector;
        Step = s_stGnssStatus.CurrentStep;
        Step--;
        if(Step < 0)
        {
            Step = s_stGnssStatus.SectorStep-1;
            Sector--;
            if(Sector < s_stGnssStatus.StartSector)
            {
                Sector = s_stGnssStatus.EndSector-1;
            }
        }
        Address = Sector*FLASH_ONE_SECTOR_BYTES+Step*FLASH_GNSS_SAVE_STEP_LEN;
        sFLASH_ReadBuffer(Buffer,Address,4);//读时间字节
        TimeCount = Public_ConvertBufferToLong(Buffer);
        Gmtime(&tt, TimeCount);
        if(ERROR == CheckTimeStruct(&tt))
        {
            sFLASH_EraseSector(Address);//擦除该扇区
            s_stGnssStatus.CurrentStep = 0;//当前步数移到扇区开头
            return ERROR;
        }
        if(ttCount < TimeCount)
        {
            return ERROR;	
        }
    }
    Address = s_stGnssStatus.CurrentSector*FLASH_ONE_SECTOR_BYTES + s_stGnssStatus.CurrentStep*FLASH_GNSS_SAVE_STEP_LEN;//获取当前地址	
    sFLASH_ReadBuffer(Buffer,Address,10);//判断是否已写入数据
    for(i=0; i<10; i++)
    {
        if(0xff != Buffer[i])
        {
            sFLASH_EraseSector(Address);//擦除该扇区
            s_stGnssStatus.CurrentStep = 0;//当前步数移到扇区开头
            for(j=0; j<200; j++)//等待一段时间后写入
            {
            }
            Address = s_stGnssStatus.CurrentSector*FLASH_ONE_SECTOR_BYTES;//计数地址
            break;
        }
    }
    ///////////////////////////////
    if(datalen+GNSS_BLIND_HEAD_LEN>FLASH_GNSS_SAVE_STEP_LEN) datalen = FLASH_GNSS_SAVE_STEP_LEN - GNSS_BLIND_HEAD_LEN;
    ////////////////////
    Public_ConvertLongToBuffer(ttCount,&Buffer[0]);
    Public_ConvertShortToBuffer(datalen,&Buffer[4]);
    VerifyCode=Public_GetSumVerify(pBuffer,datalen);
    Buffer[6] = VerifyCode;//校验码
    memcpy(&Buffer[GNSS_BLIND_HEAD_LEN],pBuffer,datalen);
    ////////////////////////
    sFLASH_WriteBuffer(Buffer, Address, datalen+GNSS_BLIND_HEAD_LEN);//数据写入flash
    s_stGnssStatus.CurrentStep++;//当前步数加1
    if(s_stGnssStatus.CurrentStep  >= s_stGnssStatus.SectorStep)//判断当前步数有没有跨越扇区
    {
        s_stGnssStatus.CurrentStep = 0;	//当前步数为0
        s_stGnssStatus.CurrentSector++;	//当前扇区下移一个
        Address = s_stGnssStatus.CurrentSector*FLASH_ONE_SECTOR_BYTES;//擦除该扇区
        sFLASH_EraseSector(Address);
        if(s_stGnssStatus.CurrentSector >= s_stGnssStatus.EndSector)//判断当前扇区是否达到末尾
        {
            s_stGnssStatus.CurrentSector = s_stGnssStatus.StartSector;//指向起始扇区
            s_stGnssStatus.LoopCount++;	//循环存储次数加1
        }
        if(s_stGnssStatus.LoopCount > 0)//最老扇区下移
        {
            s_stGnssStatus.OldestSector++;
            if(s_stGnssStatus.OldestSector >= s_stGnssStatus.EndSector)
            {
                s_stGnssStatus.OldestSector = s_stGnssStatus.StartSector;//指向起始扇区
            }            
        }
    }	
    ///////////////////
    s_stGnssParam.reReportCnt = 1;
    //////////////////
    return SUCCESS;
}
/*********************************************************************
//函数名称	:Gnss_BlindReport
//功能		:盲区补报
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void Gnss_BlindReport(void)
{
    u8	Buffer2[4];
    u16 datalen;
    u32	Address;
    u8	count;
    u8	flag;
    u8	WriteFlag = 0;
    u8	VerifyCode;
    s16	BlindSectorBackup;
    s16	BlindStepBackup;
    s16 Sector;
      u8 channel = CHANNEL_DATA_1;//为编译通过而添加
    
    flag = 0;
    if((s_stGnssSave.ReportSector == s_stGnssStatus.CurrentSector)&&(s_stGnssSave.ReportStep >= s_stGnssStatus.CurrentStep))
    {
        flag = 1;
    }
    else if((s_stGnssSave.ReportSector >= FLASH_GNSS_SAVE_END_SECTOR)||(s_stGnssSave.ReportSector < FLASH_GNSS_SAVE_START_SECTOR))
    {
        flag = 1;
    }
    else if((s_stGnssSave.ReportStep >= s_stGnssStatus.SectorStep)||(s_stGnssSave.ReportStep < 0))
    {
        flag = 1;
    }
    if(0 == flag)
    {
        count = 0;
        flag = 0;
        BlindSectorBackup = s_stGnssSave.ReportSector;
        BlindStepBackup = s_stGnssSave.ReportStep;
        Address = s_stGnssSave.ReportSector*FLASH_ONE_SECTOR_BYTES + s_stGnssStatus.SectorStep*FLASH_GNSS_SAVE_STEP_LEN+1;
        sFLASH_ReadBuffer(&flag,Address,1);
        if(0xaa == flag)//该扇区已补报过
        {
            s_stGnssSave.ReportSector++;//指向下一个扇区
            s_stGnssSave.ReportStep = 0;//指向扇区起始步
            if(s_stGnssSave.ReportSector >= FLASH_GNSS_SAVE_END_SECTOR)
            {
                s_stGnssSave.ReportSector = FLASH_GNSS_SAVE_START_SECTOR;
            }
        }
        else
        {
            Address = s_stGnssSave.ReportSector*FLASH_ONE_SECTOR_BYTES + s_stGnssSave.ReportStep*FLASH_GNSS_SAVE_STEP_LEN;
            sFLASH_ReadBuffer(RadioShareBuffer,Address,FLASH_GNSS_SAVE_STEP_LEN);//读一条盲区数据
            datalen = Public_ConvertBufferToShort(&RadioShareBuffer[4]);
            VerifyCode = Public_GetSumVerify(&RadioShareBuffer[GNSS_BLIND_HEAD_LEN], datalen);            
            if(VerifyCode == RadioShareBuffer[6])//校验正确
            {
                count++;
            }
            s_stGnssSave.ReportStep++;
            if(s_stGnssSave.ReportStep >= s_stGnssStatus.SectorStep)
            {
                WriteFlag = 1;
                Sector = s_stGnssSave.ReportSector;
                s_stGnssSave.ReportSector++;//指向下一个扇区
                s_stGnssSave.ReportStep = 0;//指向扇区起始步
                if(s_stGnssSave.ReportSector >= FLASH_GNSS_SAVE_END_SECTOR)
                {
                    s_stGnssSave.ReportSector = FLASH_GNSS_SAVE_START_SECTOR;
                }
            }
        }
        flag = 0;
        if((s_stGnssSave.ReportSector == s_stGnssStatus.CurrentSector)&&(s_stGnssSave.ReportStep >= s_stGnssStatus.CurrentStep))
        {
            flag = 1;
        }
        else if((s_stGnssSave.ReportSector >= FLASH_GNSS_SAVE_END_SECTOR)||(s_stGnssSave.ReportSector < FLASH_GNSS_SAVE_START_SECTOR))
        {
            flag = 1;
        }
        else if((s_stGnssSave.ReportStep > s_stGnssStatus.SectorStep)||(s_stGnssSave.ReportStep < 0))
        {
            flag = 1;
        }
        ///////////
        if(1 == flag)
        {
            //不需要补报
            s_stGnssSave.ReportSector = s_stGnssStatus.CurrentSector;
            s_stGnssSave.ReportStep = s_stGnssStatus.CurrentStep;
        }
        if(count)//1-10条打包发送
        {
            if(ACK_OK == RadioProtocol_OriginalDataUpTrans(channel,GNSS_UPLOAD_TYPE,&RadioShareBuffer[GNSS_BLIND_HEAD_LEN],datalen))//盲区位置信息批量上传发送
            {
                if(1 == WriteFlag)
                {
                    WriteFlag = 0;
                    Address = Sector*FLASH_ONE_SECTOR_BYTES + s_stGnssStatus.SectorStep*FLASH_GNSS_SAVE_STEP_LEN+1;
                    flag = 0xaa;
                    sFLASH_WriteBuffer(&flag, Address, 1);//标记该扇区已上报
                }
            }
            else
            {
                s_stGnssSave.ReportSector = BlindSectorBackup;
                s_stGnssSave.ReportStep = BlindStepBackup;
            }            
            Buffer2[0] = s_stGnssSave.ReportStep >> 8;
            Buffer2[1] = s_stGnssSave.ReportStep;
            FRAM_BufferWrite(FRAM_GNSS_SAVE_STEP_ADDR,Buffer2,2);//盲区步数保存至FRAM
        }
    }
    else
    {
        ICCard_ReadRepairServiceDataForSendPC();
        s_stGnssParam.reReportCnt = 0;
        s_stGnssSave.ReportFlag = 0;//不需要补报
        s_stGnssSave.ReportSector = s_stGnssStatus.CurrentSector;
        s_stGnssSave.ReportStep = s_stGnssStatus.CurrentStep;        
        Buffer2[0] = s_stGnssSave.ReportStep >> 8;
        Buffer2[1] = s_stGnssSave.ReportStep;
        FRAM_BufferWrite(FRAM_GNSS_SAVE_STEP_ADDR,Buffer2,2);//盲区步数保存至FRAM
    }
}
/*********************************************************************
//函数名称	:Gnss_CheckAndBlindReport
//功能		:盲区补报
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void Gnss_CheckAndBlindReport(void)
{
    Gnss_BlindReport();
    if(s_stGnssSave.ReportFlag&&communicatio_GetMudulState(COMMUNICATE_STATE))
    {
        LZM_PublicSetOnceTimer(&s_stGnssTimer[GNSS_TIMER_TEST],PUBLICSECS(2.6),Gnss_CheckAndBlindReport);
    }
}
/*************************************************************
** 函数名称: Gnss_StartBlindReport
** 功能描述: 开启Gnss盲区补报
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Gnss_StartBlindReport(void)
{
    u8	BufferLen;
    u8	Buffer[5];
    s16	Sector;
    u32	OldestTimeCount;
    u32	TimeCount;
    u32	Address;
    u8	flag;

    if(0==s_stGnssParam.CheckBlindflag)
    {
        Gnss_CheckBlindArea();
    }
    //盲区补报开始补报的步数
    BufferLen = FRAM_BufferRead(Buffer, 2, FRAM_GNSS_SAVE_STEP_ADDR);
    if(BufferLen == 2)
    {
        s_stGnssSave.ReportStep = Public_ConvertBufferToShort(Buffer);
        if(s_stGnssSave.ReportStep >= s_stGnssStatus.SectorStep)
        {
            s_stGnssSave.ReportStep = 0;
        }
    }
    else
    {
        s_stGnssSave.ReportStep = 0;
    }
    //确定盲区补报的开始扇区
    Sector = FLASH_GNSS_SAVE_START_SECTOR;
    OldestTimeCount = 0xffffffff;
    for(;;)
    {
        Address = Sector*FLASH_ONE_SECTOR_BYTES + s_stGnssStatus.SectorStep*FLASH_GNSS_SAVE_STEP_LEN+1;
        sFLASH_ReadBuffer(&flag,Address,1);
        if(0xaa != flag)//该扇区未补报过
        {
            Address = Sector*FLASH_ONE_SECTOR_BYTES;
            sFLASH_ReadBuffer(Buffer,Address,4);//读一条盲区数据
            TimeCount=Public_ConvertBufferToLong(Buffer);
            if(TimeCount < OldestTimeCount)
            {
                OldestTimeCount = TimeCount;
                s_stGnssSave.ReportSector = Sector;
            }
        }
        Sector++;
        if(Sector >= FLASH_GNSS_SAVE_END_SECTOR)
        {
            break;
        }
    }

    if((s_stGnssSave.ReportSector == s_stGnssStatus.CurrentSector)
    &&(s_stGnssSave.ReportStep >= s_stGnssStatus.CurrentStep))
    {
        s_stGnssSave.ReportFlag   = 0;
        s_stGnssParam.reReportCnt = 0;
        ICCard_ReadRepairServiceDataForSendPC();
    }
    else
    {
        s_stGnssSave.ReportFlag     = 1;
        s_stGnssParam.reReportCnt   = 1;
        LZM_PublicSetOnceTimer(&s_stGnssTimer[GNSS_TIMER_TEST],PUBLICSECS(1),Gnss_CheckAndBlindReport);
    }
}
/*************************************************************
** 函数名称: Gnss_CommRxIsr
** 功能描述: 
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Gnss_CommRxIsr(unsigned char val)
{
    s_stGnssAttrib.nullDataCnt  = 0;
    s_stGnssAttrib.recvDataflag = 1;
    if(s_stGnssAttrib.recvdatalen < GNSS_RECV_BUFFER_SIZE)s_stGnssAttrib.recvdatalen++;
    s_stGnssAttrib.recvBuff[s_stGnssAttrib.recvdatalen] = val;
}
/*************************************************************
** 函数名称: TiredDrive_SaveFramTimeToDriverInfo
** 功能描述: 
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Gnss_ShowDebugInfo(unsigned char *str)
{
    #if 0   
    TIME_T stTime;
    RTC_GetCurTime(&stTime);
    LOG_PR("\r\n %s T=%02d-%02d %02d:%02d:%02d \r\n ",str,stTime.month,stTime.day,stTime.hour,stTime.min,stTime.sec);
    #endif
}
/*************************************************************
** 函数名称: EN_GnssArea
** 功能描述: 采集开启上传标志
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void EN_GnssArea(void)
{
    s_stGnssAttrib.datalen      = 0;
    s_stGnssParam.freqUpFlag    = 0;
    s_stGnssParam.updataFlag    = 0;
    s_stGnssParam.preTimeVal    = RTC_GetCounter();
    if(s_stGnssParam.uploadmode < 2)s_stGnssParam.preinterval = 0;
    /////////////////
    s_stGnssParam.collectFlag   = 1;
    //Gnss_ShowDebugInfo("EN_GnssArea");
}
/*************************************************************
** 函数名称: DIS_GnssArea
** 功能描述: 关闭上传标志
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void DIS_GnssArea(void)
{
    s_stGnssAttrib.datalen      = 0;
    s_stGnssParam.freqUpFlag    = 0;
    s_stGnssParam.updataFlag    = 0;
    s_stGnssParam.collectFlag   = 0;	
}
/*************************************************************
** 函数名称: Gnss_GetcollectFlag
** 功能描述: 获取上传区域标注
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Gnss_GetcollectFlag(void)
{
    return (1==s_stGnssParam.collectFlag)?1:0;
}

/*********************************************************************
//函数名称	:Delay_nop
//功能		://内部延迟函数
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void Delay_nop(u32 x)
{ 
    u32 i;
    for(i = 0; i <= x; i ++);
}
/*********************************************************************
//函数名称	:void GNSS_BaudrateInit(u32 rate)
//功能		:GPS初始化 波特率参数
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void GNSS_BaudrateInit(u32 rate)
{
    USART_InitTypeDef USART_InitStructure;
    memset(&USART_InitStructure,0,sizeof(USART_InitTypeDef));
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    COM_Init(COM4, &USART_InitStructure);  
    COM_NVIC_Config(COM4);                               //使能串口中断

    SetUsartITConfig(COM4,USART_IT_RXNE,ENABLE);        //使能串口接收中断	

}
/****************************************************************************/
/*function	name: 			uint Gnss_ReadCollectFreq(void)	*/
/*input parameter:			none  */
/*output parameter:			返回采集周期 */
/*FUNCTION: 						实时读取数据，按照用户设置
*/
/*****************************************************************************/	
u32 Gnss_ReadCollectFreq(void)
{
    uchar buf[5];
    if(EepromPram_ReadPram(E2_GPS_SET_GATHER_RATE_ID,buf))
    {
        return Public_ConvertBufferToLong(buf);
    }
    return 1;
}
/*************************************************************
** 函数名称: Gnss_ReadUpdateMode
** 功能描述: 读取定位数据上传方式
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Gnss_ReadUpdateMode(void)
{
    unsigned char tmp;
    if(EepromPram_ReadPram(E2_GPS_SET_UPLOAD_MODE_ID,&tmp))
    {
        switch(tmp)
        {
            case 0:
                if(Area_GetCollectGnssFlag())//modify by joneming
                {
                    EN_GnssArea();//modify by joneming
                }
                #ifdef HUOYUN_DEBUG_OPEN
                else
                {
                    DIS_GnssArea();//dxl,2013.12.6,为0时表示不上传
                }
                #endif
                return tmp;
                //break;
            case 1:
                if(!Area_GetCollectGnssFlag()&&Area_GetInAreaFlag())//modify by joneming
                {
                    DIS_GnssArea();//modify by joneming
                }
                #ifdef HUOYUN_DEBUG_OPEN
                else
                {
                    EN_GnssArea();//dxl,2013.12.6,为0时表示不上传
                }
                #endif
                return tmp;
            case 2:
            case 0x0b:
            case 0x0c:
            case 0x0d:
                return tmp;
            default:
                return 0;
        }
    }
    return 0;
}
/*************************************************************
** 函数名称: Gnss_GetCurUpdateMode
** 功能描述: 取得当前定位数据上传方式
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Gnss_GetCurUpdateMode(void)
{
    return s_stGnssParam.uploadmode;
}
/*************************************************************
** 函数名称: Gnss_ReadBaseData
** 功能描述: 
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/	//获取上传基准
u32 Gnss_ReadBaseData(void)
{
    uchar buf[5];
    if(EepromPram_ReadPram(E2_GPS_SET_UPLOAD_VALUE_ID,buf))
    {
        return Public_ConvertBufferToLong(buf);
    }
    return 0;	
}
/*********************************************************************/
/*function	name: 			void	GnssCheckChangePar(void)		*/
/*input parameter:			none  */
/*output parameter:			none */
/*FUNCTION:  实时参数改变函数，根据设置调度此函数，改变采集频率，改变波特率
             改变GNSS的采集模式 （北斗模式，GPS模式，双模式）
*/
/*********************************************************************/	
void Gnss_UpdataUploadParam(void)
{
    unsigned char uploadmode;
    //获取上传模式
    uploadmode = Gnss_ReadUpdateMode();
    if(s_stGnssParam.uploadmode != uploadmode)
    {
        s_stGnssParam.uploadmode = uploadmode;
        s_stGnssParam.preinterval = 0;
    }    
    //实时获取上传基准
    s_stGnssParam.interval  = Gnss_ReadBaseData();
    //获取上传频率
    s_stGnssParam.freq      = Gnss_ReadCollectFreq();
    /////////////////
    if(INTERVAL_TIME == s_stGnssParam.uploadmode)
    {
        if(0xffffffff == s_stGnssParam.interval||0 == s_stGnssParam.interval)
        {
            s_stGnssParam.interval = 1;//默认一秒
        }
    }
    //////////////////////////
    s_stGnssAttrib.datalen      = 0;
    s_stGnssParam.reReportCnt   = 0;
}
/*********************************************************************/
/*function	name: 			void	GnssCheckChangePar(void)		*/
/*input parameter:			none  */
/*output parameter:			none */
/*FUNCTION:  实时参数改变函数，根据设置调度此函数，改变采集频率，改变波特率
             改变GNSS的采集模式 （北斗模式，GPS模式，双模式）
*/
/*********************************************************************/	
void GnssCheckChangePar(void)
{
    uchar tmp;
    char buf[20];	
#ifdef GPS_MODE_TYPE_UM330
    u32 i;
#endif
    //实时改变采集频率
    if(EepromPram_ReadPram(E2_GPS_SET_OUTPUT_RATE_ID,&tmp))
    {
        if(tmp<5)//&&tmp>0)
        {
            strcpy(buf,p_DatFreCmd[tmp]);
            Gps_SendData((u8*)buf,strlen(buf));
            Delay_nop(500*1000);//ytl 2013 06 05 
        }
    }	

    /*
    //实时改变波特率
    //配置波特率速度
    if(EepromPram_ReadPram(E2_GPS_SET_BAUD_ID,&tmp))
    {
        if(tmp<5)
        {
            strcpy(buf,p_BpCmd[tmp]);
            //GNSS_putstr(buf);	
            Gps_SendData(buf,strlen(buf));
            //GNSS_BaudrateInit(Baudrate[tmp]);
            Delay_nop(500*1000);	//ytl 2013 06 05 
        }
    }
    */
    //实时改变GNSS模式
    if(EepromPram_ReadPram(E2_GPS_SET_MODE_ID,&tmp))
    {
        if(tmp==1)//单GPS模式,dxl
        {
            //strcpy(buf,p_startupCmd[1]);dxl,2013.6.7
            strcpy(buf,p_startupCmd[0+3]);
            //置位状态位的bit18,清除状态位的bit19,20,21
            /*北斗检测时打开
            Io_WriteStatusBit(STATUS_BIT_GPS, SET);
            Io_WriteStatusBit(STATUS_BIT_COMPASS, RESET);
            Io_WriteStatusBit(STATUS_BIT_GLONASS, RESET);
            Io_WriteStatusBit(STATUS_BIT_GALILEO, RESET);	
            */
        }
        else if(tmp==2)//单BD2模式,dxl
        {
            //strcpy(buf,p_startupCmd[0]);dxl,2013.6.7
            strcpy(buf,p_startupCmd[1+3]);
            //置位状态位的bit19,清除状态位的bit18,20,21
            /*
            Io_WriteStatusBit(STATUS_BIT_GPS, RESET);
            Io_WriteStatusBit(STATUS_BIT_COMPASS, SET);
            Io_WriteStatusBit(STATUS_BIT_GLONASS, RESET);
            Io_WriteStatusBit(STATUS_BIT_GALILEO, RESET);	
            */
        }
        else if(tmp==3)//BD2/GPS模式
        {
            //strcpy(buf,p_startupCmd[2]);dxl,2013.6.7
            strcpy(buf,p_startupCmd[2+3]);
            //置位状态位的bit18,19,清除状态位的bit20,21
            /*
            Io_WriteStatusBit(STATUS_BIT_GPS, SET);
            Io_WriteStatusBit(STATUS_BIT_COMPASS, SET);
            Io_WriteStatusBit(STATUS_BIT_GLONASS, RESET);
            Io_WriteStatusBit(STATUS_BIT_GALILEO, RESET);	
            */
        }        
        Gps_SendData((u8*)buf,strlen(buf));//行标检测时这行需屏蔽，3种模式都是采用混合定位模式        
        #ifdef GPS_MODE_TYPE_UM330
        //对于和芯星通的模块，设置完定位模式后，还得要发送保存命令，否则重启后又按默认的定位模式处理了
        //延时,dxl,2013.12.5
        for(i=0; i<1000000; i++)
        {

        }
        //memcpy((u8*)buf,"$CFGSAVE,",9);
        strcpy(buf,"$CFGSAVE,h1f\r\n");
        Gps_SendData((u8*)buf, strlen(buf));//保存卫星系统配置
        //延时
        for(i=0; i<1000000; i++)
        {

        }
        #endif
    }
    /////////////////
    Gnss_UpdataUploadParam();
}

/****************************************************************************/
/*function	name: 			void Gnss_App_TimeTask(void);	*/
/*input parameter:			none  */
/*output parameter:			none */
/*FUNCTION: 						
*/
/*****************************************************************************/	
FunctionalState Gnss_App_TimeTask(void)
{
    return DISABLE;
}
/*************************************************************
** 函数名称: Gnss_SaveCurCollectDataToFlash
** 功能描述: 
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Gnss_SaveCurCollectDataToFlash(void)
{
    if(s_stGnssAttrib.datalen)
    Gnss_WriteOneGnssDataToFlash(s_stGnssAttrib.parseBuff,s_stGnssAttrib.datalen);
}
/*************************************************************
** 函数名称: Gnss_CheckAndUploadData
** 功能描述: 处理判断接收数据
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Gnss_OriginalDataUpTrans(void)
{
    u8 channel = CHANNEL_DATA_1;//为编译通过而添加
    
    if(0==s_stGnssAttrib.datalen)return ;
    #ifdef HUOYUN_DEBUG_OPEN
    if(Area_GetCollectGnssFlag())
    {
        if(Area_GetCloseGPRSFlag())//保存盲区
        {
            Gnss_SaveCurCollectDataToFlash();
            return;
        } 
    }
    /////////////////////////
    if(s_stGnssParam.reReportCnt)
    {
        if(s_stGnssParam.reReportCnt < 1000)s_stGnssParam.reReportCnt++;
        else s_stGnssParam.reReportCnt = 0;
        return;
    }
    #endif
    if(communicatio_GetMudulState(COMMUNICATE_STATE))
    {
        
        if(ACK_ERROR==RadioProtocol_OriginalDataUpTrans(channel,GNSS_UPLOAD_TYPE,s_stGnssAttrib.parseBuff,s_stGnssAttrib.datalen))
        {
            if(s_stGnssParam.resendCnt++ < 4)//重发3次，一共4次
            {
                LZM_PublicSetOnceTimer(&s_stGnssTimer[GNSS_TIMER_RESEND],PUBLICSECS(0.2),Gnss_OriginalDataUpTrans);
            } 
            else
            {
                s_stGnssParam.resendCnt = 0;
            }
        }
        else
        {
            s_stGnssParam.resendCnt = 0;
        }
    }
    else
    {
        s_stGnssParam.resendCnt = 0;
    }
}
/*************************************************************
** 函数名称: Gnss_GetCurTotalMeter
** 功能描述: 获得距离(米)
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned long Gnss_GetCurTotalMeter(void)
{
    unsigned long curtotalmeter;    
    if(SpeedMonitor_GetCurSpeedType())
    {
        curtotalmeter = MileMeter_GetTotalMile();        
    }
    else
    {
        curtotalmeter = Pulse_GetTotalMile();
    }
    curtotalmeter *=10;
    return curtotalmeter;
}
/*********************************************************************
//函数名称	:uchar Gnss_CheckCmpConstantParam
//功能		:
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/		
unsigned char Gnss_CheckCmpConstantParam(unsigned long curcheckVal,unsigned long value)
{
    if(curcheckVal>s_stGnssParam.preinterval)
    {
        if(curcheckVal>=value+s_stGnssParam.preinterval)
        {
            s_stGnssParam.preinterval +=value;
            if(curcheckVal>s_stGnssParam.preinterval+value)
            {
                s_stGnssParam.preinterval = curcheckVal;
            }            
            return 1;
        }
    }
    else
    if(curcheckVal < s_stGnssParam.preinterval)
    {
        s_stGnssParam.preinterval = curcheckVal;
    }
    //////////////////
    return 0;
}

/*************************************************************
** 函数名称: Gnss_CheckAndUploadData
** 功能描述: 处理判断接收数据
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Gnss_CheckAndUploadData(void)
{
    unsigned long curcheckVal;    
    if(0==Gnss_GetcollectFlag())return;//判断需要采集不？
    if(0==Area_GetInAreaFlag())//没在任何区域内
    {
        if(Area_GetRectAreaTotalNum()||Area_GetRoundAreaTotalNum()||Polygon_GetAreaTotalNum())
        {
            DIS_GnssArea();
            return;//判断需要采集不？
        }
    }
    //获取时间
    curcheckVal = RTC_GetCounter();
    ///////////////////
    if(curcheckVal>=s_stGnssParam.freq+s_stGnssParam.preTimeVal)
    {
        s_stGnssParam.preTimeVal += s_stGnssParam.freq;
        if(curcheckVal>=s_stGnssParam.freq+s_stGnssParam.preTimeVal)
        {
            s_stGnssParam.preTimeVal = curcheckVal;
        }
        s_stGnssParam.freqUpFlag = 1;
    }
    ///////////////////////
    switch(s_stGnssParam.uploadmode)//采集模式
    {
        case NONE_UPDATE://本地保存,不管Gnss数据是否更新
            if(Gnss_CheckCmpConstantParam(curcheckVal,s_stGnssParam.interval))
            {
                #ifdef HUOYUN_DEBUG_OPEN
                if(Area_GetCollectGnssFlag())
                {
                    if(Area_GetCloseGPRSFlag())//保存盲区
                    {
                        Gnss_SaveCurCollectDataToFlash();
                    }                        
                    else
                    {
                        Gnss_OriginalDataUpTrans();
                    }                        
                }
                #endif
            }
            break;
        case INTERVAL_TIME://定时上传,不管Gnss数据是否更新
            if(Gnss_CheckCmpConstantParam(curcheckVal,s_stGnssParam.interval))
            {
                #if 0
                char buffer[50];
                TIME_T stTime;
                RTC_GetCurTime(&stTime);
                sprintf(buffer,"T=%02d-%02d %02d:%02d:%02d = ",stTime.month,stTime.day,stTime.hour,stTime.min,stTime.sec);
                LOG_DB(buffer,s_stGnssAttrib.parseBuff,s_stGnssAttrib.datalen);
                #endif
                if(Area_GetCollectGnssFlag()&&Area_GetCloseGPRSFlag())//保存盲区
                {
                    #ifdef HUOYUN_DEBUG_OPEN
                    Gnss_SaveCurCollectDataToFlash();
                    #endif
                }
                else
                {
                    Gnss_OriginalDataUpTrans();
                }
            }						
            break;
        case INTERVAL_METER://定距离间隔,不管Gnss数据是否更新
            curcheckVal = Gnss_GetCurTotalMeter();
            //////////////////////////
            if(Gnss_CheckCmpConstantParam(curcheckVal,s_stGnssParam.interval))
            {
                Gnss_OriginalDataUpTrans();
            }						
            break;
        case COUNT_TIME_STOP://按累计时间上传，达到传输时间后自动停止上传；  
            if(0==s_stGnssParam.preinterval)s_stGnssParam.preinterval=curcheckVal;
            if(curcheckVal<s_stGnssParam.preinterval+s_stGnssParam.interval)
            {
                if(s_stGnssParam.updataFlag)
                {
                    s_stGnssParam.updataFlag = 0;
                    Gnss_OriginalDataUpTrans();
                }
            }
            else
            {
                DIS_GnssArea();
            }					
            break;
        case COUNT_METER_STOP://，按累计距离上传，达到距离后自动停止上传
            curcheckVal =Gnss_GetCurTotalMeter();
            ////////////////
            if(0==s_stGnssParam.preinterval)s_stGnssParam.preinterval = curcheckVal;                   
            if(curcheckVal < s_stGnssParam.preinterval+s_stGnssParam.interval)	
            {
                if(s_stGnssParam.updataFlag)
                {
                    s_stGnssParam.updataFlag = 0;
                    Gnss_OriginalDataUpTrans();
                }
            }
            else
            {
                DIS_GnssArea();
            }
            break;
        case COUNT_NUMBER_STOP:////按累计条数上传，达到上传条数后自动停止上传*/
            if(s_stGnssParam.preinterval < s_stGnssParam.interval)
            {
                if(s_stGnssParam.updataFlag)
                {
                    s_stGnssParam.updataFlag = 0;
                    s_stGnssParam.preinterval++;                    
                    Gnss_OriginalDataUpTrans();
                }
            }
            else
            {
                DIS_GnssArea();
            }
            break;
        default:
            break;

    }
}
/*************************************************************
** 函数名称: Gnss_CheckAndUploadData
** 功能描述: 处理判断接收数据
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Gnss_StartCheckAndUploadData(void)
{
    LZM_PublicSetCycTimer(&s_stGnssTimer[GNSS_TIMER_TASK],PUBLICSECS(0.1),Gnss_CheckAndUploadData);
}
/****************************************************************************/
/*function	name: 			void InitConfigGNSS(void)		*/
/*input parameter:			none  */
/*output parameter:			none */
/*FUNCTION: 						按照要求初始化GNSS模块
*/
/*****************************************************************************/		
void Gnss_ReadParam(void)
{
    uchar tmp;//length;
    char buf[20];
    //默认上电配置成9600
    //GNSS_Init();
    //配置更新频率

    if(EepromPram_ReadPram(E2_GPS_SET_OUTPUT_RATE_ID,&tmp))
    {
        if(tmp<5)//&&tmp>0)
        {
            strcpy(buf,p_DatFreCmd[tmp]);
            Gps_SendData((u8*)buf,strlen(buf));
            Delay_nop(500*1000);//ytl 2013 06 05 
        }
    }

    /*
    //配置波特率速度
    if(EepromPram_ReadPram(E2_GPS_SET_BAUD_ID,&tmp))
    {
        if(tmp<5)
        {
            strcpy(buf,p_BpCmd[tmp]);
            //GNSS_putstr(buf);	
            Gps_SendData(buf,strlen(buf));
            //GNSS_BaudrateInit(Baudrate[tmp]);	
            Delay_nop(500*1000);//ytl 2013 06 05 
        }
    }
    */
    //配置启动方式---此部分不确定，暂时屏蔽
    if(EepromPram_ReadPram(E2_GPS_SET_MODE_ID,&tmp))
    {
        if(tmp==1)//单GPS模式
        {
            strcpy(buf,p_startupCmd[0]);
        }
        else if(tmp==2)//单BD2模式
        {
            strcpy(buf,p_startupCmd[1]);

        }
        else if(tmp==3)//BD2/GPS模式
        {
            strcpy(buf,p_startupCmd[2]);
        }
        Gps_SendData((u8*)buf,strlen(buf));
    }
    /////////////////
    Gnss_UpdataUploadParam(); 
    ///////////////////
    Gnss_CheckBlindArea();
    /////////////////////////
    LZM_PublicSetOnceTimer(&s_stGnssTimer[GNSS_TIMER_TASK],PUBLICSECS(25),Gnss_StartCheckAndUploadData);
    ///////////////////////////////////
}
/****************************************************************************/
/*function	name: 			void InitConfigGNSS(void)		*/
/*input parameter:			none  */
/*output parameter:			none */
/*FUNCTION: 						按照要求初始化GNSS模块
*/
/*****************************************************************************/		
void InitConfigGNSS(void)
{
    LZM_PublicKillTimerAll(s_stGnssTimer,GNSS_TIMERS_MAX); 
    //////////////////////////////
    s_stGnssAttrib.datalen      = 0;
    s_stGnssAttrib.recvdatalen  = 0;    
    s_stGnssAttrib.recvDataflag = 0;    
    ////////////////////////////
    memset(&s_stGnssParam,0,sizeof(s_stGnssParam));
    //////////////////////////////
    SetTimerTask(TIME_GNSS_REAL,LZM_TIME_BASE);
    //////////////////////////////////////////////////////////
    LZM_PublicSetOnceTimer(&s_stGnssTimer[GNSS_TIMER_TASK],PUBLICSECS(5),Gnss_ReadParam);
}
/*************************************************************
** 函数名称: Gnss_CheckReckData
** 功能描述: 处理判断接收数据
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Gnss_CheckReckData(void)
{
    s_stGnssAttrib.nullDataCnt++;    
    if(s_stGnssAttrib.nullDataCnt>=3)//>=150ms没接收到数据
    {
        if(s_stGnssAttrib.nullDataCnt>SYSTICK_10SECOND)//10s都没接收到数据
        {
            s_stGnssAttrib.datalen = 0;
        }
        if(s_stGnssAttrib.recvDataflag)
        {
            s_stGnssAttrib.recvDataflag = 0;
            if(s_stGnssParam.freqUpFlag)
            {
                s_stGnssParam.freqUpFlag = 0;
                s_stGnssParam.updataFlag = 1;
                s_stGnssAttrib.datalen   = s_stGnssAttrib.recvdatalen;                         
                memcpy(s_stGnssAttrib.parseBuff,s_stGnssAttrib.recvBuff,s_stGnssAttrib.datalen);
                //Gnss_ShowDebugInfo("updata");
            }
            s_stGnssAttrib.recvdatalen  = 0;
        }
    }
}
/*************************************************************
** 函数名称: GnssRealData_TimeTask
** 功能描述: 实时解析调用函数在 50ms调用一次
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
FunctionalState GnssRealData_TimeTask(void)
{
    Gnss_CheckReckData();
    //////////////////////
    LZM_PublicTimerHandler(s_stGnssTimer,GNSS_TIMERS_MAX);
    return ENABLE;       
}
/*********************************************************************
//函数名称	:uchar CmpConstantPar(u32 source,u32 obj,u32 const_num)
//功能		://内部切换函数
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/		
uchar CmpConstantPar(u32 source,u32 obj,u32 const_num)
{
    if(source>obj||source==obj)
    {
        if(source-obj>const_num||source-obj==const_num)
            return true;
        else
            return false;
    }
    else
    {
        if(obj-source>const_num||obj-source==const_num)
            return true;
        else
            return false;
    }
}
/*******************************************************************************
 *                             end of module
 *******************************************************************************/


/*******************************************************************************
 * File Name:			AreaManage.c 
 * Function Describe:	
 * Relate Module:		区域功能
 * Writer:				Joneming
 * Date:				2012-09-17
 * ReWriter:			Joneming
 * Date:				2013-05-21
 ******* 添加开门报警功能、关闭通讯模块功能、采集GNSS详细定位数据功能；
 ******* 修改进出区域的语音提示属性位置:由原来的位14、15修改为:12、13
 *******************************************************************************/
#include "include.h"
////////////////////////////
///////////////////////////////////
#define IN_OUT_AREA_OVERHEAD_INFO_ID              0x12 //进出区域报警
#define IN_OUT_AREA_OVERHEAD_INFO_LEN             0x06;//进出区域报警附加信息长度
///////////////////////////////////
///////////////////////////
//#define TEST_USE_BIG_AREA
#define TEST_USE_SMALL_AREA
//结构
typedef struct
{
    unsigned long LeftUpLat;        //左上点纬度
    unsigned long LeftUpLong;       //左上点经度
    unsigned long RightDownLat;     //右下点纬度
    unsigned long RightDownLong;    //右下点经度
    unsigned char AreaFlag;         //区域标志
    unsigned char CheckFlag;        //检查标志
}ST_TEST_AREA;

#ifdef TEST_USE_BIG_AREA
static ST_TEST_AREA s_stTestBigArea;
#endif
#ifdef TEST_USE_SMALL_AREA
static ST_TEST_AREA s_stTestSmallArea;
#endif

#define TEST_AREA_EXTRA_ADD_LATITUDE  600//单位为0.0001分:600*0.185m =111纬度
#define TEST_AREA_EXTRA_ADD_LONGITUDE 700//单位为0.0001分:700*0.15m 经度


#define MAX_AREA_ADDR           0xFFFFFFFF

#define MAX_RECT_AREA_SUM       100  //最多矩形区域数
#define MAX_ROUND_AREA_SUM      100  //最多圆形区域数

#ifndef FLASH_ONE_SECTOR_BYTES
#define FLASH_ONE_SECTOR_BYTES  ONE_SECTOR_BYTES
#endif

#ifndef FLASH_RECTANGLE_AREA_START_SECTOR
#define FLASH_RECTANGLE_AREA_START_SECTOR   (AREA_START_SECTOR)
#endif
#ifndef FLASH_RECTANGLE_AREA_END_SECTOR
#define FLASH_RECTANGLE_AREA_END_SECTOR     (AREA_START_SECTOR+4)
#endif

#ifndef FLASH_ROUND_AREA_START_SECTOR
#define FLASH_ROUND_AREA_START_SECTOR         (AREA_START_SECTOR+5)
#endif
#ifndef FLASH_ROUND_AREA_END_SECTOR
#define FLASH_ROUND_AREA_END_SECTOR     AREA_END_SECTOR
#endif
//////////////////////////////////////
#define RECT_AREA_ADDR_START  (unsigned long)(FLASH_RECTANGLE_AREA_START_SECTOR*FLASH_ONE_SECTOR_BYTES) //矩形区域存放点,扇区需要从0开始....
#define RECT_AREA_ADDR_END  (unsigned long)((FLASH_RECTANGLE_AREA_END_SECTOR)*FLASH_ONE_SECTOR_BYTES)

#define ROUND_AREA_ADDR_START   (unsigned long)(FLASH_ROUND_AREA_START_SECTOR*FLASH_ONE_SECTOR_BYTES) //圆形区域存放点,扇区需要从0开始....
#define ROUND_AREA_ADDR_END  (unsigned long)(FLASH_ROUND_AREA_END_SECTOR*FLASH_ONE_SECTOR_BYTES) 


// 0,超速  1:进圆形区 2,进矩形区域 3:出圆形区 4:出矩形区
typedef enum{ALARM_ITEM_SPEED_AREA=0,ALARM_ITEM_IN_ROUND_AREA,ALARM_ITEM_IN_RECT_AREA,ALARM_ITEM_OUT_ROUND_AREA,ALARM_ITEM_OUT_RECT_AREA} E_AREA_ALARM_ITEM;


#define VOICE_BYTE 50


//矩形区域属性结构	156字节
typedef struct
{
    unsigned long Head;             //头标志
    unsigned long AreaID;           //区域ID
    unsigned long LeftUpLat;        //左上点纬度
    unsigned long LeftUpLong;       //左上点经度
    unsigned long RightDownLat;     //右下点纬度
    unsigned long RightDownLong;    //右下点经度
    unsigned short AreaAttrib;      //区域属性
    unsigned short LimitSpeed;      //限速值    
    unsigned char  StartMoment[6];  //开始时刻
    unsigned char  EndMoment[6];    //结束时刻    
    unsigned char  speedDuration;   //持续时间
    unsigned char  Reserve[11];      //保留字节
    unsigned short UserAreaAttrib;   //自定义区域属性
    unsigned char  InVoiceSize;
    unsigned char  OutVoiceSize;
    unsigned char  InVoiceStr[VOICE_BYTE];
    unsigned char  OutVoiceStr[VOICE_BYTE];                   
}STRECT_AREA_ATTRIB;

#define STRECT_AREA_ATTRIB_SIZE sizeof(STRECT_AREA_ATTRIB)

//圆形区域属性结构	152字节
typedef struct
{
    unsigned long Head;             //头标志
    unsigned long AreaID;           //区域ID   
    unsigned short AreaAttrib;      //区域属性
    unsigned short LimitSpeed;      //限速值
    unsigned long CenterLat;        //中心纬度
    unsigned long CenterLong;       //中心经度
    unsigned long Radius;           //半径
    unsigned char  StartMoment[6];  //开始时刻
    unsigned char  EndMoment[6];    //结束时刻    
    unsigned char  speedDuration;   //保持时间
    unsigned char  Reserve[11];      //保留字节
    ////////////////////////////////////
    unsigned short UserAreaAttrib;   //自定义区域属性    
    unsigned char  InVoiceSize;
    unsigned char  OutVoiceSize;    
    unsigned char  InVoiceStr[VOICE_BYTE];
    unsigned char  OutVoiceStr[VOICE_BYTE];
}STROUND_AREA_ATTRIB;

#define STROUND_AREA_ATTRIB_SIZE sizeof(STROUND_AREA_ATTRIB)

#define HEAD_FLAG               0xABCDDCBA
#define AREA_HEAD_SIZE          8//读取头标志、区域ID
#define AREA_ATTRIB_SIZE        56//读取除语音部分的所有参数

//结构
typedef struct
{
    unsigned long Latitude;                     //纬度整型坐标
    unsigned long Longitude;                    //经度整型坐标
    unsigned long InOutAreaID;                  //进出区域ID
    unsigned char CheckRectNum;                  //矩形区域总数
    unsigned char CheckRoundNum;                 //圆形区域总数
    unsigned char ValidRectNum;                 //矩形区域有效总数
    unsigned char ValidRoundNum;                //圆形区域有效总数    
    unsigned char INOUTPostType;                //进出区域类型    
    unsigned char Orientation;                  //区域方向
    unsigned char InOutAreaAlarm;               //进出区域报警
    unsigned char preSecond;                    //当前秒数
    unsigned char preSpeed;                     //前一秒的速度
    unsigned char GpsSpeed;                     //
    unsigned char nonavigationCnt;              //不导航计数器
    unsigned char OpenDoorAlarmFlag;            //区域内非法开车门报警标志
    unsigned char CollectGnssFlag;              //区域采集GNSS标志
    unsigned char CloseGPRSFlag;                //区域关闭通讯模块标志
    unsigned char InAreaFlag;                   //在区域内标志
}ST_AREA_ATTRIB;

static ST_AREA_ATTRIB s_stAreaAttrib;

//区域控制块结构
typedef struct
{
    unsigned long addr;             //地址//当为MAX_AREA_NUM时,表示没有进入任何区域 
    unsigned long startTime;        //开始时间
    unsigned long endTime;          //结束时间
    unsigned long AreaID;           //区域ID 
    unsigned short AreaAttrib;      //区域属性
    unsigned short UserAreaAttrib;  //自定义区域属性
    unsigned short LimitSpeed;      //限速值
    unsigned char speedDuration;    //保持时间
    unsigned char AreaCnt;          //区域计数器
    unsigned char preAreaCnt;       //区域计数器
    unsigned char AreaFlag;         //区域标志
    unsigned char timeType;         //时间类型
    unsigned char protoclType;      //协议类型,设置区域时使用
    unsigned char VoiceStr[VOICE_BYTE];//存放区域语音
}STAREA_PCB;
///////////////////////
enum 
{
    AREA_TIMER_TASK,                        //
    AREA_TIMER_SCAN_ROUND,					//    
    AREA_TIMER_SCAN_RECT,					//     
    AREA_TIMER_TEST,						//    
    AREA_TIMERS_MAX
}STAREATIME;

static LZM_TIMER s_stAREATimer[AREA_TIMERS_MAX];

static STAREA_PCB s_stRectArea;  //矩形属性变量
static STAREA_PCB s_stRoundArea;  //圆形属性变量

extern unsigned char s_ucMsgFlashBufferer[];
///////////////////////////////////////////
unsigned char Area_IsInRectArea(STRECT_AREA_ATTRIB *Ptr) ; //判断当前坐标点是否在Ptr所指的矩形区域内
unsigned char Area_IsInRoundArea(STROUND_AREA_ATTRIB *Ptr) ; //判断当前坐标点是否在Ptr所指的圆形区域内
unsigned char Area_RoundComparePos(T_GPS_POS *CenterPos,T_GPS_POS *CurPos,unsigned long Radius); //判断点是否在圆形区域内
void Area_ScanRoundArea(void) ;//扫描圆形区域
void Area_ScanRectArea(void) ;//扫描矩形区域
void Area_ChoiceAreaArmDest(unsigned char AlarmItem); //区域范围内的报警处理
void AreaManage_ScanTask(void);
void Area_CloseCollectGNSSDetailLocatorData(void) ;
//////////////////////////////

#if 0//旧算法使用
//-------------------------------------------------
//距离公式,参数修正表
//-------------------------------------------------
const unsigned int COS_PARA[]={
1000,998,996,990,980,
970,956,941,924,904,
885,864,835,815,786,
748,720,693,657,623,
589,556,525,486,449,
414,380,347,316,281,
253,221,192,169,143,
120,98,  85, 62,43,
30, 19,  11, 5, 1,0
};
#define AREA_LENGTH (unsigned long)(111)  //111319m  //每一度对应的长度单位:Km   111000是GoogleEarch测量值
#define AREA_MAX(X,Y)  ((X)>=(Y)? (X):(Y)) //找出最大值
#define AREA_MIN(X,Y)  ((X)<(Y)? (X):(Y))  //找出最小值

//-----------------------------------------------
//功能:计算点到点距离
//输入:指向两个定位点结构的指针
//输出:米
//备注:
//-----------------------------------------------
unsigned long CalDotDotMile(T_GPS_POS *gpsdataSrc, T_GPS_POS *gpsdataDest)
{
    unsigned long long lon,lat,result,lontemp,lattemp; //使用64bit长整形
    unsigned char LatIndex;

    lat = gpsdataSrc->Lat;
    lattemp = gpsdataDest->Lat;

    lon = gpsdataSrc->Long;
    lontemp = gpsdataDest->Long;

    lat= AREA_MAX(lat,lattemp)-AREA_MIN(lat,lattemp);
    lon = AREA_MAX(lon,lontemp)-AREA_MIN(lon,lontemp);
    LatIndex=(lattemp/1000000UL)>>1;

    lon=lon*lon;	
    lon=lon*COS_PARA[LatIndex]/1000; 

    lat=lat*lat;

    result = (long long)sqrt(lon + lat);
    result = result*AREA_LENGTH/1000; //获得距离单位(米)	
    return  result;
}
/********************************************************************
* 功能 : 计算点到线距离
* 输入: 指向三个定位点结构的指针
* 输出: 米
* 全局变量: 
*
* 备注: (修改记录内容、时间)
********************************************************************/
unsigned long CalDotLineMile(T_GPS_POS *gpsdataSrc, T_GPS_POS *gpsdataDest1,T_GPS_POS *gpsdataDest2)
{
    unsigned long long lon,lat,result,lontemp1,lattemp1,lontemp2,lattemp2;
    unsigned char LatIndex;
    long long lonx,laty,lontempx1,lattempy1,lontempx2,lattempy2;

    lat = gpsdataSrc->Lat;
    lattemp1 = gpsdataDest1->Lat;
    lattemp2 = gpsdataDest2->Lat;	
    lon = gpsdataSrc->Long;
    lontemp1 = gpsdataDest1->Long;
    lontemp2 = gpsdataDest2->Long;
    laty=lat;
    lattempy1=lattemp1;
    lattempy2=lattemp2;
    lonx=lon;
    lontempx1=lontemp1;
    lontempx2=lontemp2;

    //先算公式分母部分
    LatIndex=(lat/1000000UL)>>1;
    lat = AREA_MAX(lattemp1,lattemp2)-AREA_MIN(lattemp1,lattemp2);
    lon = AREA_MAX(lontemp1,lontemp2)-AREA_MIN(lontemp1,lontemp2);

    lon=lon*lon;	
    lon=lon*COS_PARA[LatIndex]/1000UL; 

    lat=lat*lat;

    result = (long long)sqrt(lon + lat);
    //再求分子部分
    lonx=(lattempy2-lattempy1)*(lonx-lontempx1)-(lontempx2-lontempx1)*(laty-lattempy1);
    if(lonx<0)
        lonx = -lonx;
    lon=lonx;
    result=lon/result*AREA_LENGTH/1000*COS_PARA[LatIndex]/1000;  //获得距离单位(米)	
    return  result;
}
#else
//-----------------------------------------------
//功能:计算点到点距离
//输入:指向两个定位点结构的指针
//输出:米
//备注:
//-----------------------------------------------
unsigned long CalDotDotMile(T_GPS_POS *gpsdataSrc, T_GPS_POS *gpsdataDest)
{
    return (unsigned long)Route_GetPointDistance(gpsdataSrc,gpsdataDest);
}
#endif
/*************************************************************
** 函数名称: Area_GetCurSpeed
** 功能描述: 取得当前速度
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: SpeedMonitor_GetCurSpeed
*************************************************************/
unsigned char Area_GetCurSpeed(void)
{
    unsigned char speed,GpsSpeed;
    speed = SpeedMonitor_GetCurSpeed();
    GpsSpeed = Gps_ReadSpeed();
    if(0==speed&&GpsSpeed>5)speed = GpsSpeed;
    return speed;
}
/*************************************************************
** 函数名称: Area_TestRectAreaParameterInitialize
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void Area_TestRectAreaParameterInitialize(void)
{
    #ifdef TEST_USE_BIG_AREA
    s_stTestBigArea.AreaFlag = AREA_STATUS_MAX;
    s_stTestBigArea.CheckFlag = 0;
    #endif
    #ifdef TEST_USE_SMALL_AREA
    s_stTestSmallArea.AreaFlag = AREA_STATUS_MAX;
    s_stTestSmallArea.CheckFlag = 0;
    #endif
}
/*************************************************************
** 函数名称: Area_SetTestRectAreaParameter
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void Area_SetTestRectAreaParameter(T_GPS_POS *LeftUpPos,T_GPS_POS *RightDownPos)
{
    #ifdef TEST_USE_BIG_AREA
    if(0==s_stTestBigArea.CheckFlag)
    {
        //////大区域///////////////////
        s_stTestBigArea.AreaFlag = AREA_STATUS_MAX;
        s_stTestBigArea.CheckFlag = 1;
        ///////左上点////////////////////
        s_stTestBigArea.LeftUpLat =LeftUpPos->Lat+TEST_AREA_EXTRA_ADD_LATITUDE;//纬度整型坐标
        s_stTestBigArea.LeftUpLong =LeftUpPos->Long-TEST_AREA_EXTRA_ADD_LONGITUDE;//经度整型坐标
        ////////右下点///////////
        s_stTestBigArea.RightDownLat =RightDownPos->Lat-TEST_AREA_EXTRA_ADD_LATITUDE;//纬度整型坐标
        s_stTestBigArea.RightDownLong =RightDownPos->Long+TEST_AREA_EXTRA_ADD_LONGITUDE;//经度整型坐标
        //////////////////////////////////
    }
    #endif
    /////////////////////////
    #ifdef TEST_USE_SMALL_AREA
    if(0 == s_stTestSmallArea.CheckFlag)
    {
        //////小区域///////////////////
        s_stTestSmallArea.AreaFlag = AREA_STATUS_MAX;
        s_stTestSmallArea.CheckFlag = 1;
         ///////左上点////////////////////
        s_stTestSmallArea.LeftUpLat =LeftUpPos->Lat-TEST_AREA_EXTRA_ADD_LATITUDE;//纬度整型坐标
        s_stTestSmallArea.LeftUpLong =LeftUpPos->Long+TEST_AREA_EXTRA_ADD_LONGITUDE;//经度整型坐标
        ////////右下点///////////
        s_stTestSmallArea.RightDownLat =RightDownPos->Lat+TEST_AREA_EXTRA_ADD_LATITUDE;//纬度整型坐标
        s_stTestSmallArea.RightDownLong =RightDownPos->Long-TEST_AREA_EXTRA_ADD_LONGITUDE;//经度整型坐标
    }
    #endif
}
#ifdef TEST_USE_BIG_AREA
/*************************************************************
** 函数名称: Area_ScanTestRectArea
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void Area_ScanTestBigRectArea(void)
{
    unsigned char Status;
    if(0==s_stTestBigArea.CheckFlag)return;    
    if(AREA_STATUS_IN != s_stTestBigArea.AreaFlag)//不在区域里
    {
        Status = ComparePos( (T_GPS_POS *)&(s_stTestBigArea.LeftUpLat), (T_GPS_POS *)&(s_stTestBigArea.RightDownLat),(T_GPS_POS *)&s_stAreaAttrib.Latitude);	
        if(0 == Status)//0:在区域内
        {
            s_stTestBigArea.AreaFlag = AREA_STATUS_IN;
            Report_ClearReportTimeCount();
            ClrTimerTask(TIME_POSITION);//进大区域关位置汇报
            ///////////////////////
        }   
    }
    else
    if(AREA_STATUS_IN == s_stTestBigArea.AreaFlag)//不在区域里
    {
        Status = ComparePos( (T_GPS_POS *)&(s_stTestBigArea.LeftUpLat), (T_GPS_POS *)&(s_stTestBigArea.RightDownLat),(T_GPS_POS *)&s_stAreaAttrib.Latitude);	
        if(1 == Status)//1:不在在区域
        {
            s_stTestBigArea.AreaFlag = AREA_STATUS_OUT;
            /////////////////////////
            Report_ClearReportTimeCount();
            SetTimerTask(TIME_POSITION,SYSTICK_1SECOND);//出大区域开位置汇报
            ///////////////////////
        }
    }
}
#endif
/*************************************************************
** 函数名称: Area_ScanTestRectArea
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
#ifdef TEST_USE_SMALL_AREA
void Area_ScanTestSmallRectArea(void)
{
    unsigned char Status;
    if(0==s_stTestSmallArea.CheckFlag)return;    
    if(AREA_STATUS_IN != s_stTestSmallArea.AreaFlag)//不在区域里
    {
        Status = ComparePos( (T_GPS_POS *)&(s_stTestSmallArea.LeftUpLat), (T_GPS_POS *)&(s_stTestSmallArea.RightDownLat),(T_GPS_POS *)&s_stAreaAttrib.Latitude);	
        if(0 == Status)//0:在区域内
        {
            s_stTestSmallArea.AreaFlag = AREA_STATUS_IN;
            //进小区域开启GNSS采集
        }   
    }
    else
    if(AREA_STATUS_IN == s_stTestSmallArea.AreaFlag)//不在区域里
    {
        Status = ComparePos( (T_GPS_POS *)&(s_stTestSmallArea.LeftUpLat), (T_GPS_POS *)&(s_stTestSmallArea.RightDownLat),(T_GPS_POS *)&s_stAreaAttrib.Latitude);	
        if(1 == Status)//1:不在在区域
        {
            s_stTestSmallArea.AreaFlag = AREA_STATUS_OUT;
            //出小区域关GNSS采集
            ////////////////////////////
            Report_ClearReportTimeCount();
            Area_CloseCollectGNSSDetailLocatorData();
            /////////////////////
        }
    }
}
#endif
/*************************************************************
** 函数名称: Area_ScanTestRectArea
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
void Area_ScanTestRectArea(void)
{
    #ifdef TEST_USE_BIG_AREA
    Area_ScanTestBigRectArea();
    #endif
    #ifdef TEST_USE_SMALL_AREA
    Area_ScanTestSmallRectArea();
    #endif
}
/*************************************************************
** 函数名称: Area_GetConverseBCDTimeType
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/
unsigned char Area_GetConverseBCDTimeType(unsigned char *buffer)
{
    if(0==buffer[0]&& 0==buffer[1]&& 0==buffer[2])
    {
        return 1;
    }

    return 0;
}
//-----------------------------------------------
//功能:BCD格式时间,传成HEX
//输入:1,指向BCD格式时刻的指针，2，类型(0:全部 1:仅转化时分秒)
//输出:
//备注:
//-----------------------------------------------
unsigned long Area_ConverseBcdGmtime(unsigned char *Ptr, unsigned char Type)
{
    unsigned long Temp;
    if(Type==0)
    {
        TIME_T stTime;
        if(Public_ConvertBCDToTime(&stTime,Ptr))
        {
            Temp = ConverseGmtime(&stTime);
        }
        else
        {
            RTC_GetCurTime(&stTime);
            Temp = Public_BCD2HEX(*(Ptr+1)); //获得月
            if(Temp)stTime.month = Temp;
            Temp = Public_BCD2HEX(*(Ptr+2)); //获得日
            if(Temp)stTime.day=Temp;
            stTime.hour = Public_BCD2HEX(*(Ptr+3)); //获得时
            stTime.min = Public_BCD2HEX(*(Ptr+4)); //获得分
            stTime.sec = Public_BCD2HEX(*(Ptr+5)); //获得秒
            Temp = ConverseGmtime(&stTime);
        }
    }
    else
    {
        Temp = 0; 
        Temp = Temp*24 + Public_BCD2HEX(*(Ptr+3)); //获得时
        Temp = Temp*60 + Public_BCD2HEX(*(Ptr+4)); //获得分
        Temp = Temp*60 + Public_BCD2HEX(*(Ptr+5)); //获得秒
    }
    return Temp;
}

//-----------------------------------------------
//功能:BCD格式时间,传成HEX
//输入:1,指向BCD格式时刻的指针，2，类型(0:全部 1:仅转化时分秒)
//输出:
//备注:
//-----------------------------------------------
unsigned long Area_GetConvertCurTime(unsigned char Type)
{
    unsigned long Temp;    
    if(Type==0)
    {
        Temp =RTC_GetCounter();
    }
    else
    {
        TIME_T stTime;        
        Temp = 0;
        RTC_GetCurTime(&stTime);
        Temp = Temp*24 + stTime.hour; //获得时
        Temp = Temp*60 + stTime.min; //获得分
        Temp = Temp*60 + stTime.sec; //获得秒
    }
    return Temp;
}
/*************************************************************OK
** 函数名称: Area_MdyDataInFlash
** 功能描述: 写任意地址的Flash里的数据
** 入口参数: DataPtr数据首地址,addr为Flash地址,len为数据长度
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/  
void Area_MdyDataInFlash(unsigned char *DataPtr,unsigned long addr,unsigned short len)
{
    unsigned char buffer[100];
    unsigned long blockAddr,i,tempLen,tempAddr,datalen; //按4K取整地址    
    tempLen = 0;
    datalen = len;
    blockAddr = addr & 0xFFFFF000; //取数据开始的扇区地址
    sFLASH_ReadBuffer(s_ucMsgFlashBuffer,blockAddr,FLASH_ONE_SECTOR_BYTES); //读扇区 
    tempAddr = addr%FLASH_ONE_SECTOR_BYTES;     //计算偏移地址
    tempLen = FLASH_ONE_SECTOR_BYTES-tempAddr;  //在本块中还能保存的数据长度
    if(tempLen>datalen)tempLen = datalen;//在同一扇区内
    for(i=0; i<tempLen; i++)//
    {
        s_ucMsgFlashBuffer[tempAddr+i] = *DataPtr++;    
    }
    IWDG_ReloadCounter();
    sFLASH_EraseSector(blockAddr);          //擦除扇区
    sFLASH_WriteBuffer(s_ucMsgFlashBuffer,blockAddr,FLASH_ONE_SECTOR_BYTES);//写扇区
    //////校验/////////////////
    spi_Delay_uS(100);
    sFLASH_ReadBuffer(buffer,blockAddr,100); //读扇区 
    if(Public_CheckArrayValIsEqual(buffer,s_ucMsgFlashBuffer,100))
    {
        IWDG_ReloadCounter();
        sFLASH_EraseSector(blockAddr);          //擦除扇区
        sFLASH_WriteBuffer(s_ucMsgFlashBuffer,blockAddr,FLASH_ONE_SECTOR_BYTES);//写扇区
    }
    datalen -=tempLen;        
    while(datalen>0)//还有数据没写完////////////////
    {
        blockAddr += FLASH_ONE_SECTOR_BYTES;//取下一扇区地址
        sFLASH_ReadBuffer(s_ucMsgFlashBuffer,blockAddr,FLASH_ONE_SECTOR_BYTES); //读扇区数据
        tempLen = datalen;
        if(tempLen>FLASH_ONE_SECTOR_BYTES)tempLen = FLASH_ONE_SECTOR_BYTES;
        for(i=0; i<tempLen; i++)
        {
            s_ucMsgFlashBuffer[i] = *DataPtr++;    
        }
        IWDG_ReloadCounter();       
        sFLASH_EraseSector(blockAddr);          //擦除扇区
        sFLASH_WriteBuffer(s_ucMsgFlashBuffer,blockAddr,FLASH_ONE_SECTOR_BYTES);//写扇区
        //////校验/////////////////////
        spi_Delay_uS(100);
        sFLASH_ReadBuffer(buffer,blockAddr,100); //读扇区 
        if(Public_CheckArrayValIsEqual(buffer,s_ucMsgFlashBuffer,100))
        {
            IWDG_ReloadCounter();
            sFLASH_EraseSector(blockAddr);          //擦除扇区
            sFLASH_WriteBuffer(s_ucMsgFlashBuffer,blockAddr,FLASH_ONE_SECTOR_BYTES);//写扇区
        }
        datalen -= tempLen;
    }
}
/*************************************************************OK
** 函数名称: Area_ClearSpeedAreaBit
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/  
void Area_ClearSpeedAreaBit(unsigned char status)
{
    if(E_RECT_AREA == status)
    {
        SpeedMonitor_ClearSpeedParamValid(E_SPEED_TYPE_RECT_AREA);
    }
    else
    if( E_ROUND_AREA == status)
    {
        SpeedMonitor_ClearSpeedParamValid(E_SPEED_TYPE_ROUND_AREA);
    }
    else
    if( E_POLYGON_AREA == status)
    {
        SpeedMonitor_ClearSpeedParamValid(E_SPEED_TYPE_POLYGON_AREA);
    }
} 
/************************************************************
** 函数名称: Area_CloseGSMModule
** 功能描述: 关闭GSM模块
** 入口参数: 
** 出口参数:
************************************************************/ 
void Area_CloseGSMModule(void) 
{
    #ifdef __GNSS_APP__
    Communication_Close();
    #endif
}
/************************************************************
** 函数名称: Area_OpenGSMModule
** 功能描述: 开启GSM模块
** 入口参数: 
** 出口参数:
************************************************************/ 
void Area_OpenGSMModule(void) 
{
    #ifdef __GNSS_APP__
    Communication_Open();
    #endif
}
/************************************************************
** 函数名称: Area_CloseCollectGNSSDetailLocatorData
** 功能描述: 关闭采集GNSS详细定位数据
** 入口参数: 
** 出口参数:
************************************************************/ 
void Area_CloseCollectGNSSDetailLocatorData(void) 
{
    #ifdef __GNSS_APP__
    DIS_GnssArea();
    #endif
}
/************************************************************
** 函数名称: Area_OpenCollectGNSSDetailLocatorData
** 功能描述: 开启采集GNSS详细定位数据
** 入口参数: 
** 出口参数:
************************************************************/ 
void Area_OpenCollectGNSSDetailLocatorData(void) 
{
    #ifdef __GNSS_APP__
    EN_GnssArea();
    #endif
}
/************************************************************
** 函数名称: Area_OutGNSSAreaOpenPosition
** 功能描述: 
** 入口参数: 
** 出口参数:
************************************************************/
void Area_OutGNSSAreaOpenPosition(void) 
{
    //行标检测用
    //Report_ClearReportTimeCount();
    //SetTimerTask(TIME_POSITION,SYSTICK_1SECOND);
}
/************************************************************
** 函数名称: Area_OutGNSSAreaOpenPosition
** 功能描述: 
** 入口参数: 
** 出口参数:
************************************************************/
void Area_CloseCollectGNSS(void) 
{
    Area_CloseCollectGNSSDetailLocatorData();
    LZM_PublicSetOnceTimer(&s_stAREATimer[AREA_TIMER_TEST],PUBLICSECS(4),Area_OutGNSSAreaOpenPosition);
}
/************************************************************
** 函数名称: Area_OpenCollectGNSS
** 功能描述: 
** 入口参数: 
** 出口参数:
************************************************************/
void Area_OpenCollectGNSS(void) 
{
    //Report_ClearReportTimeCount();//行标检测用
    //ClrTimerTask(TIME_POSITION);//行标检测用
    LZM_PublicSetOnceTimer(&s_stAREATimer[AREA_TIMER_TEST],PUBLICSECS(4),Area_OpenCollectGNSSDetailLocatorData);    
}
/*************************************************************OK
** 函数名称: Area_GetCollectGnssFlag
** 功能描述: 获取当前Gnss是否需要采集标志
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/  
unsigned char Area_GetCollectGnssFlag(void)
{
    return s_stAreaAttrib.CollectGnssFlag;
}
/*************************************************************OK
** 函数名称: Area_SetCollectGnssFlag
** 功能描述: 设置当前Gnss需要采集标志
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/  
void Area_SetCollectGnssFlag(unsigned char type)
{
    if(type>E_ROUTE)return;
    if(PUBLIC_CHECKBIT(s_stAreaAttrib.CollectGnssFlag,type))return;
    if(0==s_stAreaAttrib.CollectGnssFlag)
    {
        Area_OpenCollectGNSS();
    }
    PUBLIC_SETBIT(s_stAreaAttrib.CollectGnssFlag,type);    
}
/*************************************************************OK
** 函数名称: Area_ClearCollectGnssFlag
** 功能描述: 清除当前Gnss需要采集标志
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/  
void Area_ClearCollectGnssFlag(unsigned char type)
{
    if(type>E_ROUTE)return;
    if(0==PUBLIC_CHECKBIT(s_stAreaAttrib.CollectGnssFlag,type))return;
    PUBLIC_CLRBIT(s_stAreaAttrib.CollectGnssFlag,type);
    if(0==s_stAreaAttrib.CollectGnssFlag)
    {
        Area_CloseCollectGNSS();
    }
}
/*************************************************************OK
** 函数名称: Area_GetCloseGPRSFlag
** 功能描述: 获取当前关闭通讯模块标志
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/  
unsigned char Area_GetCloseGPRSFlag(void)
{
    return s_stAreaAttrib.CloseGPRSFlag;
}
/*************************************************************OK
** 函数名称: Area_SetCloseGPRSFlag
** 功能描述: 设置当前关闭通讯模块标志
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/  
void Area_SetCloseGPRSFlag(unsigned char type)
{
    if(type>E_ROUTE)return;
    if(PUBLIC_CHECKBIT(s_stAreaAttrib.CloseGPRSFlag,type))return;
    if(0==s_stAreaAttrib.CloseGPRSFlag)
    {
        Area_CloseGSMModule();
    }
    PUBLIC_SETBIT(s_stAreaAttrib.CloseGPRSFlag,type);
}
/*************************************************************OK
** 函数名称: Area_ClearCloseGPRSFlag
** 功能描述: 清除当前关闭通讯模块标志
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/  
void Area_ClearCloseGPRSFlag(unsigned char type)
{
    if(type>E_ROUTE)return;
    if(0==PUBLIC_CHECKBIT(s_stAreaAttrib.CloseGPRSFlag,type))return;
    PUBLIC_CLRBIT(s_stAreaAttrib.CloseGPRSFlag,type);
    if(0==s_stAreaAttrib.CloseGPRSFlag)
    {
        Area_OpenGSMModule();
    }
}
/*************************************************************OK
** 函数名称: Area_GetCollectGnssFlag
** 功能描述: 获取当前是否在区域内标志
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/  
unsigned char Area_GetInAreaFlag(void)
{
    return s_stAreaAttrib.InAreaFlag;
}
/*************************************************************OK
** 函数名称: Area_SetCollectGnssFlag
** 功能描述: 设置当前在区域内标志
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/  
void Area_SetInAreaFlag(unsigned char type)
{
    if(type>E_ROUTE)return;
    if(PUBLIC_CHECKBIT(s_stAreaAttrib.InAreaFlag,type))return;
    PUBLIC_SETBIT(s_stAreaAttrib.InAreaFlag,type);
    if(!Area_GetCollectGnssFlag())
    {
        Area_CloseCollectGNSSDetailLocatorData();
    }
}
/*************************************************************OK
** 函数名称: Area_ClearCollectGnssFlag
** 功能描述: 清除当前在区域内标志
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/  
void Area_ClearInAreaFlag(unsigned char type)
{
    if(type>E_ROUTE)return;
    if(0==PUBLIC_CHECKBIT(s_stAreaAttrib.InAreaFlag,type))return;
    PUBLIC_CLRBIT(s_stAreaAttrib.InAreaFlag,type);
}
/*************************************************************OK
** 函数名称: Area_SetOpenDoorAlarmBit
** 功能描述: 设置开门报警位
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/  
void Area_SetOpenDoorAlarmBit(unsigned char type)
{
    if(type>E_ROUTE)return;
    if(PUBLIC_CHECKBIT(s_stAreaAttrib.OpenDoorAlarmFlag,type))return;
    PUBLIC_SETBIT(s_stAreaAttrib.OpenDoorAlarmFlag,type);
    #ifdef __GNSS_APP__
    Io_WriteAlarmBit(ALARM_BIT_LAWLESS_OPEN_DOOR,SET);
    #endif
}
/*************************************************************OK
** 函数名称: Area_ClearOpenDoorAlarmBit
** 功能描述: 清除开门报警位
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/  
void Area_ClearOpenDoorAlarmBit(unsigned char type)
{
    if(type>E_ROUTE)return;
    if(0==PUBLIC_CHECKBIT(s_stAreaAttrib.OpenDoorAlarmFlag,type))return;
    PUBLIC_CLRBIT(s_stAreaAttrib.OpenDoorAlarmFlag,type);
    ////////////////////////////
    if(0 == s_stAreaAttrib.OpenDoorAlarmFlag)
    {
        #ifdef __GNSS_APP__
        Io_WriteAlarmBit(ALARM_BIT_LAWLESS_OPEN_DOOR,RESET); 
        #endif
    }   
}
/*************************************************************OK
** 函数名称: Area_CheckOpenDoorAlarm
** 功能描述: 检查开门报警
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/  
void Area_CheckOpenDoorAlarm(unsigned char type)
{
    if(type>E_ROUTE)return;
    #ifdef __GNSS_APP__
    if(Io_ReadStatusBit(STATUS_BIT_DOOR1) || Io_ReadStatusBit(STATUS_BIT_DOOR2) || Io_ReadStatusBit(STATUS_BIT_DOOR3)
    || Io_ReadStatusBit(STATUS_BIT_DOOR4) || Io_ReadStatusBit(STATUS_BIT_DOOR5))
    {
        Area_SetOpenDoorAlarmBit(type);
    }
    else  //车门关闭，则主动清除报警标志
    {
        Area_ClearOpenDoorAlarmBit(type);
    }
    #endif
}
/*************************************************************OK
** 函数名称: Area_GetInOutAreaAlarmBit
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/  
unsigned char Area_GetInOutAreaAlarmBit(unsigned char status)
{
    return PUBLIC_CHECKBIT(s_stAreaAttrib.InOutAreaAlarm,status);
}
/*************************************************************OK
** 函数名称: Area_SetSpeedAreaBit
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/  
void Area_SetInOutAreaAlarmBit(unsigned char status)
{
    PUBLIC_SETBIT(s_stAreaAttrib.InOutAreaAlarm,status);
    Io_WriteAlarmBit(ALARM_BIT_IN_OUT_AREA,SET);
}
/*************************************************************
** 函数名称: Area_SetInOutAreaAlarmInfo
** 功能描述: 设置区域报警信息
** 入口参数: type:报警区域类型,AlarmID:报警区域ID,Orientation方向
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/  
void Area_SetInOutAreaAlarmInfo(unsigned char type,unsigned long AlarmID,unsigned char Orientation)
{
    s_stAreaAttrib.INOUTPostType = type;
    s_stAreaAttrib.InOutAreaID = AlarmID;    
    s_stAreaAttrib.Orientation = Orientation;
    Area_SetInOutAreaAlarmBit(type);
}
/*************************************************************OK
** 函数名称: Area_ClearInOutAreaAlarmBit
** 功能描述: 清除区域报警信息
** 入口参数: type区域类型
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/  
void Area_ClearInOutAreaAlarmBit(unsigned char type)
{
    PUBLIC_CLRBIT(s_stAreaAttrib.InOutAreaAlarm,type);
    if(0 == s_stAreaAttrib.InOutAreaAlarm)
    {
        Io_WriteAlarmBit(ALARM_BIT_IN_OUT_AREA,RESET);
    }   
}
/*************************************************************OK
** 函数名称: Area_ClearAllAlarm
** 功能描述: 清除区域报警信息
** 入口参数: type区域类型
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/  
void Area_ClearAllAlarm(unsigned char type)
{
    if(type>E_ROUTE)return;
     Area_ClearInOutAreaAlarmBit(type);
    if(0==PUBLIC_CHECKBIT(s_stAreaAttrib.InAreaFlag,type))return;
    Area_ClearSpeedAreaBit(type);
    Area_ClearOpenDoorAlarmBit(type);
    Area_ClearCloseGPRSFlag(type);
    Area_ClearCollectGnssFlag(type);
    Area_ClearInAreaFlag(type);
}
/*************************************************************
** 函数名称: Area_RectAreaResetParam
** 功能描述: 矩形区域重置参数
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Area_RectAreaResetParam(void) 
{
    s_stAreaAttrib.preSpeed = 0xff;
    s_stRectArea.AreaFlag   = AREA_STATUS_MAX;
    s_stRectArea.AreaCnt    = 0;
    s_stRectArea.addr       = MAX_AREA_ADDR;  
    ///////////////////////////////
    Area_ClearAllAlarm(E_RECT_AREA);
    ///////////////////
    Area_TestRectAreaParameterInitialize();
}
/*************************************************************
** 函数名称: Area_RectAreaCheckForResetParam
** 功能描述: 矩形区域检查是否重置参数
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Area_RectAreaCheckForResetParam(unsigned long AreaID) 
{
    if(s_stRectArea.AreaFlag==AREA_STATUS_IN&&s_stRectArea.AreaID==AreaID)
    {
        Area_RectAreaResetParam();
    }
}
/*************************************************************
** 函数名称: Area_RectAreaParameterInitialize
** 功能描述: 矩形区域参数初始化
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Area_RectAreaParameterInitialize(void) 
{
    Area_RectAreaResetParam();
    s_stAreaAttrib.CheckRectNum  = 0;
    s_stAreaAttrib.ValidRectNum = 0;
}
/*************************************************************
** 函数名称: Area_RoundAreaResetParam
** 功能描述: 圆形区域重置参数
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Area_RoundAreaResetParam(void) 
{
    s_stAreaAttrib.preSpeed = 0xff;
    s_stRoundArea.AreaFlag = AREA_STATUS_MAX;
    s_stRoundArea.AreaCnt = 0;
    s_stRoundArea.addr = MAX_AREA_ADDR;
    ////////////////////////////
    Area_ClearAllAlarm(E_ROUND_AREA);
}
/*************************************************************
** 函数名称: Area_RoundAreaCheckForResetParam
** 功能描述: 圆形区域检查是否重置参数
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Area_RoundAreaCheckForResetParam(unsigned long AreaID) 
{
    if(s_stRoundArea.AreaFlag==AREA_STATUS_IN&&s_stRoundArea.AreaID==AreaID)
    {
        Area_RoundAreaResetParam();
    }
}
/*************************************************************
** 函数名称: Area_RoundAreaParameterInitialize
** 功能描述: 圆形区域参数初始化
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Area_RoundAreaParameterInitialize(void) 
{
    Area_RoundAreaResetParam();
    //获得圆形区域数
    s_stAreaAttrib.CheckRoundNum  = 0;
    s_stAreaAttrib.ValidRoundNum = 0;
}

/*************************************************************
** 函数名称: Area_EraseFlash
** 功能描述: 清空区域
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Area_EraseFlash(unsigned short Start,unsigned short End)  //
{
    unsigned long addr;
    unsigned short i;
    addr =Start*FLASH_ONE_SECTOR_BYTES;
    for(i = Start; i< End; i++)
    {
        sFLASH_EraseSector(addr);
        addr += FLASH_ONE_SECTOR_BYTES;
    }
}
/*************************************************************
** 函数名称: Area_ClearRectArea
** 功能描述: 清除矩形区域
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Area_ClearRectArea(void)
{
    Area_RectAreaParameterInitialize();
    Area_EraseFlash(FLASH_RECTANGLE_AREA_START_SECTOR,FLASH_RECTANGLE_AREA_END_SECTOR); //擦除区域
}
/*************************************************************
** 函数名称: Area_ClearRoundArea
** 功能描述: 清除圆形区域
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Area_ClearRoundArea(void)
{
    Area_RoundAreaParameterInitialize();
    Area_EraseFlash(FLASH_ROUND_AREA_START_SECTOR,FLASH_ROUND_AREA_END_SECTOR); //擦除区域
}
/*************************************************************
** 函数名称: Area_EraseAllArea
** 功能描述: 清除区域
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Area_EraseAllArea(void)
{   
    Area_ClearRectArea();
    ///////////////
    Area_ClearRoundArea();
}

/*************************************************************
** 函数名称: Area_GetCurGpsCoordinate
** 功能描述: 获得当前坐标
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Area_GetCurGpsCoordinate(void)
{  
    unsigned char buffer[28];
    Public_GetCurBasicPositionInfo(buffer);
    Public_Mymemcpy((unsigned char *)&s_stAreaAttrib.Latitude,(unsigned char *)&buffer[8],4,1); 
    Public_Mymemcpy((unsigned char *)&s_stAreaAttrib.Longitude,(unsigned char *)&buffer[12],4,1); 
    /////////////////////  
}
//-----------------------------------------------
//功能:判断当前坐标点是否在Ptr所指的矩形区域内
//输入:指向区域的指针
//输出:0:在  1:不在  0XFF:出错
//备注:
//-----------------------------------------------
unsigned char Area_IsInRectArea(STRECT_AREA_ATTRIB *Ptr)
{
    ///////////////////////////
    return( ComparePos( (T_GPS_POS *)&(Ptr->LeftUpLat), (T_GPS_POS *)&(Ptr->RightDownLat),(T_GPS_POS *)&s_stAreaAttrib.Latitude));
}

//-----------------------------------------------
//功能:判断当前坐标点是否在Ptr所指的圆形区域内
//输入:指向区域的指针
//输出:0:在  1:不在  0XFF:出错
//备注:
//-----------------------------------------------
unsigned char Area_IsInRoundArea(STROUND_AREA_ATTRIB *Ptr)
{
    return( Area_RoundComparePos( (T_GPS_POS *)&(Ptr->CenterLat),(T_GPS_POS *)&s_stAreaAttrib.Latitude, Ptr->Radius) );
}
//-----------------------------------------------
//功能:判断点是否在矩形区域内
//输入:3点坐标
//输出:0:在区域内  1:不在 0XFF:出错
//备注:
//-----------------------------------------------	
unsigned char ComparePos(T_GPS_POS *LeftUpPos,T_GPS_POS *RightDownPos, T_GPS_POS *CurPos) 
{
    T_GPS_POS Dot1,Dot2;
    unsigned long Temp;
    if(LeftUpPos->Lat == RightDownPos->Lat || LeftUpPos->Long == RightDownPos->Long)
        return 0XFF;
    Dot1 = *LeftUpPos;
    Dot2 = *RightDownPos;
    //Dot1的经纬度<Dot2的经纬度
    if(Dot1.Lat >Dot2.Lat)
    {
        Temp = Dot1.Lat;
        Dot1.Lat = Dot2.Lat;
        Dot2.Lat = Temp;
    }
    else if(Dot1.Long >Dot2.Long)
    {
        Temp = Dot1.Long;
        Dot1.Long = Dot2.Long;
        Dot2.Long = Temp;
    }
    if(CurPos->Lat < Dot2.Lat && CurPos->Lat > Dot1.Lat && CurPos->Long < Dot2.Long && CurPos->Long > Dot1.Long)
        return 0; 
    else
        return 1;
}	
//-----------------------------------------------
//功能:判断点是否在圆形区域内
//输入:3点坐标
//输出:0:在区域内  1:不在 0XFF:出错
//备注:
//-----------------------------------------------	
unsigned char Area_RoundComparePos(T_GPS_POS *CenterPos,T_GPS_POS *CurPos,unsigned long Radius) 
{
	if(Route_GetPointDistance(CenterPos, CurPos)<=Radius)
	{
	    return 0;	
	}
	else
	{
	    return 1;	
	}
}
/*************************************************************
** 函数名称: Area_CheckRectTime
** 功能描述: 检测矩形是否符合时间范围
** 入口参数: 
** 出口参数: 
** 返回参数: 0:不符合时间范围,1:符合时间范围
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Area_CheckRectTime(void)
{
    u32 curTime = 0;
    //是否需要检查时间
    if(!PUBLIC_CHECKBIT(s_stRectArea.AreaAttrib,E_AREA_MOMENT))return 1; //不需要检查时间                  
    curTime=Area_GetConvertCurTime(s_stRectArea.timeType); //当前时间  
    return Public_CheckTimeIsInRange(curTime,s_stRectArea.startTime,s_stRectArea.endTime);    
}
/*************************************************************
** 函数名称: Area_CheckRoundTime
** 功能描述: 检测圆形是否符合时间范围
** 入口参数: 
** 出口参数: 
** 返回参数: 0:不符合时间范围,1:符合时间范围
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Area_CheckRoundTime(void)
{
    u32 curTime = 0;
    //是否需要检查时间
    if(!PUBLIC_CHECKBIT(s_stRoundArea.AreaAttrib,E_AREA_MOMENT))return 1; //不需要检查时间                  
    curTime=Area_GetConvertCurTime(s_stRoundArea.timeType); //当前时间  
    return Public_CheckTimeIsInRange(curTime,s_stRoundArea.startTime,s_stRoundArea.endTime);    
}
//-----------------------------------------------
//功能:扫描圆形区域
//输入:
//输出:
//备注:每秒执行一次
//-----------------------------------------------	
void Area_ScanRoundArea(void) 
{
    unsigned char i,j;
    unsigned char Status,len;
    unsigned long OffSetAddr;
    STROUND_AREA_ATTRIB stRoundArea;  //读取圆形区域属性
    //圆形区域检测
    if(AREA_STATUS_IN != s_stRoundArea.AreaFlag)//不在区域里
    {
        //判断是否在区域内
        if(MAX_AREA_ADDR == s_stRoundArea.addr)
        {
            for(i=0; s_stRoundArea.AreaCnt < s_stAreaAttrib.CheckRoundNum && i<10; s_stRoundArea.AreaCnt++,i++)
            {
                OffSetAddr=ROUND_AREA_ADDR_START + s_stRoundArea.AreaCnt*STROUND_AREA_ATTRIB_SIZE ;  
                sFLASH_ReadBuffer((unsigned char *)&stRoundArea,OffSetAddr,AREA_ATTRIB_SIZE);//读出区域数据	
                if(stRoundArea.AreaID==0) //跳过ID=0
                    continue;
                s_stRoundArea.AreaAttrib=stRoundArea.AreaAttrib;
                s_stRoundArea.timeType=Area_GetConverseBCDTimeType(stRoundArea.StartMoment);
                s_stRoundArea.startTime=Area_ConverseBcdGmtime(&stRoundArea.StartMoment[0],s_stRoundArea.timeType);
                s_stRoundArea.endTime=Area_ConverseBcdGmtime(&stRoundArea.EndMoment[0],s_stRoundArea.timeType);
                ///////////////////////////////////////
                if(0==Area_CheckRoundTime())//时间不符合
                    continue;
                Status = Area_IsInRoundArea(&stRoundArea);
                if(Status==0)
                {//进入区域
                    s_stRoundArea.addr = OffSetAddr;
                    break;
                } 
            }
            //////////           
            if(s_stRoundArea.AreaCnt>=s_stAreaAttrib.CheckRoundNum)s_stRoundArea.AreaCnt = 0;  //重新检索
            //////////////////////////
            if((MAX_AREA_ADDR == s_stRoundArea.addr) && (s_stRoundArea.preAreaCnt != s_stRoundArea.AreaCnt))LZM_PublicSetOnceTimer(&s_stAREATimer[AREA_TIMER_SCAN_ROUND],PUBLICSECS(0.2),Area_ScanRoundArea);
            /////////////////////////// 
        }
        //判断在区域内是否持续一段时间
        if(s_stRoundArea.addr != MAX_AREA_ADDR)
        {//进入
            s_stRoundArea.AreaFlag = AREA_STATUS_IN;
            sFLASH_ReadBuffer((unsigned char *)&stRoundArea,s_stRoundArea.addr,STROUND_AREA_ATTRIB_SIZE);//读出区域数据                
            if(PUBLIC_CHECKBIT(stRoundArea.AreaAttrib,E_IN_SENDTO_DRIVER)) 
            {
                if(PUBLIC_CHECKBIT(stRoundArea.UserAreaAttrib,E_USER_IN_PLAY))
                {
                    len=stRoundArea.InVoiceSize;
                    if(len>=VOICE_BYTE)len=VOICE_BYTE-1;
                    for(j=0; j<len;j++)
                    {
                        s_stRoundArea.VoiceStr[j]=stRoundArea.InVoiceStr[j];                    
                    }
                    s_stRoundArea.VoiceStr[j]='\0';
                }
                else
                {
                    strcpy((char *)s_stRoundArea.VoiceStr,"你已驶入圆形区域");
                }
            }
            //////////////////////////
            s_stRoundArea.AreaID =stRoundArea.AreaID; 
            s_stRoundArea.AreaAttrib = stRoundArea.AreaAttrib;
            s_stRoundArea.UserAreaAttrib = stRoundArea.UserAreaAttrib;            
            s_stRoundArea.LimitSpeed=stRoundArea.LimitSpeed; 
            s_stRoundArea.speedDuration=stRoundArea.speedDuration;            
            if(s_stRoundArea.speedDuration<10)s_stRoundArea.speedDuration = 10;  
            Area_ChoiceAreaArmDest(ALARM_ITEM_IN_ROUND_AREA);
        }
    }
    else
    if(AREA_STATUS_IN == s_stRoundArea.AreaFlag)//在区域里
    {
        if(0==Area_CheckRoundTime())//时间不符合
        {
            Area_RoundAreaResetParam();
            return;
        }
        //////////////////////////
        sFLASH_ReadBuffer((unsigned char *)&stRoundArea,s_stRoundArea.addr,STROUND_AREA_ATTRIB_SIZE);//读出区域数据
        Status = Area_IsInRoundArea(&stRoundArea);	
        if(Status)
        {//出区
            s_stRoundArea.AreaFlag = AREA_STATUS_OUT;//出区域
            /////////////////////////////////
            sFLASH_ReadBuffer((unsigned char *)&stRoundArea,s_stRoundArea.addr,STROUND_AREA_ATTRIB_SIZE);//读出区域数据	
            if(PUBLIC_CHECKBIT(stRoundArea.AreaAttrib,E_OUT_SENDTO_DRIVER)) 
            {
                if(PUBLIC_CHECKBIT(stRoundArea.UserAreaAttrib,E_USER_OUT_PLAY))
                {
                    len=stRoundArea.OutVoiceSize;
                    if(len>=VOICE_BYTE)len=VOICE_BYTE-1;
                    for(j=0; j<len;j++)
                    {
                        s_stRoundArea.VoiceStr[j]=stRoundArea.OutVoiceStr[j];                    
                    }
                    s_stRoundArea.VoiceStr[j]='\0';
                }
                else
                {
                    strcpy((char *)s_stRoundArea.VoiceStr,"你已离开圆形区域");
                }
            }
            ///////////////////////////
            s_stRoundArea.AreaID = stRoundArea.AreaID;
            s_stRoundArea.AreaAttrib = stRoundArea.AreaAttrib;
            s_stRoundArea.UserAreaAttrib = stRoundArea.UserAreaAttrib;
            Area_ChoiceAreaArmDest(ALARM_ITEM_OUT_ROUND_AREA); 
            ///////////////////////////
            s_stRoundArea.addr = MAX_AREA_ADDR;  
        }
    }
    //在区域内////////////////////////////////
    if((AREA_STATUS_IN==s_stRoundArea.AreaFlag)&&PUBLIC_CHECKBIT(s_stRoundArea.AreaAttrib,E_AREA_PROHIBIT_OPENDOOR))
    {
        Area_CheckOpenDoorAlarm(E_ROUND_AREA);
    }
}
//-----------------------------------------------
//功能:扫描矩形区域
//输入:
//输出:
//备注:每秒执行一次
//-----------------------------------------------
void Area_ScanRectArea(void)
{
    unsigned char i,j;
    unsigned char Status,len;
    unsigned long OffSetAddr;
    STRECT_AREA_ATTRIB	stRectArea;	  //读取矩形区域属性
    if(s_stRectArea.AreaFlag != AREA_STATUS_IN)
    {
        if(s_stRectArea.addr == MAX_AREA_ADDR)
        {
            for(i=0; s_stRectArea.AreaCnt<s_stAreaAttrib.CheckRectNum && i<10; s_stRectArea.AreaCnt++,i++)
            {
                OffSetAddr=RECT_AREA_ADDR_START + s_stRectArea.AreaCnt*STRECT_AREA_ATTRIB_SIZE ;  
                sFLASH_ReadBuffer((unsigned char *)&stRectArea,OffSetAddr,AREA_ATTRIB_SIZE);//读出区域数据	
                if(stRectArea.AreaID ==0) //跳过ID=0
                    continue;
                s_stRectArea.AreaAttrib=stRectArea.AreaAttrib;
                /////////////////////////////////
                #ifdef TEST_USE_BIG_AREA
                if(PUBLIC_CHECKBIT(stRectArea.AreaAttrib,E_AREA_ENTER_COLLECT_GNSS))//
                {
                    Area_SetTestRectAreaParameter((T_GPS_POS *)&(stRectArea.LeftUpLat),(T_GPS_POS*)&(stRectArea.RightDownLat));                    
                }
                #endif
                ////////////////////
                s_stRectArea.timeType=Area_GetConverseBCDTimeType(stRectArea.StartMoment);
                s_stRectArea.startTime=Area_ConverseBcdGmtime(&stRectArea.StartMoment[0],s_stRectArea.timeType);
                s_stRectArea.endTime=Area_ConverseBcdGmtime(&stRectArea.EndMoment[0],s_stRectArea.timeType);
                if(0==Area_CheckRectTime())//时间不符合
                    continue;
                Status = Area_IsInRectArea(&stRectArea);	
                if(Status==0)
                {//进入区域
                    s_stRectArea.addr = OffSetAddr;
                    break;
                }        	
            }            
            if(s_stRectArea.AreaCnt>=s_stAreaAttrib.CheckRectNum)s_stRectArea.AreaCnt = 0;  //重新检索
            ////////////////////////
            if((MAX_AREA_ADDR == s_stRectArea.addr) && (s_stRectArea.preAreaCnt != s_stRectArea.AreaCnt))LZM_PublicSetOnceTimer(&s_stAREATimer[AREA_TIMER_SCAN_RECT],PUBLICSECS(0.2),Area_ScanRectArea);
            //////////////////////////
        }
        ////////////////////////////////////
        if(s_stRectArea.addr != MAX_AREA_ADDR)//进入
        {
            s_stRectArea.AreaFlag = AREA_STATUS_IN;                
            sFLASH_ReadBuffer((unsigned char *)&stRectArea,s_stRectArea.addr,STRECT_AREA_ATTRIB_SIZE);//读出区域数据	
            if(PUBLIC_CHECKBIT(stRectArea.AreaAttrib,E_IN_SENDTO_DRIVER)) 
            {
                if(PUBLIC_CHECKBIT(stRectArea.UserAreaAttrib,E_USER_IN_PLAY))
                {
                    len=stRectArea.InVoiceSize;
                    if(len>=VOICE_BYTE)len=VOICE_BYTE-1;
                    for(j=0; j<len;j++)
                    {
                        s_stRectArea.VoiceStr[j]=stRectArea.InVoiceStr[j];                    
                    }
                    s_stRectArea.VoiceStr[j]='\0';
                }
                else
                {
                    strcpy((char *)s_stRectArea.VoiceStr,"你已驶入矩形区域");
                }
            }
            ///////////////////////////
            s_stRectArea.AreaID = stRectArea.AreaID;
            s_stRectArea.AreaAttrib = stRectArea.AreaAttrib;
            s_stRectArea.UserAreaAttrib = stRectArea.UserAreaAttrib;            
            s_stRectArea.LimitSpeed = stRectArea.LimitSpeed;
            s_stRectArea.speedDuration =stRectArea.speedDuration;
            if(s_stRectArea.speedDuration < 10) s_stRectArea.speedDuration = 10;
            ///////////////////////
            if(PUBLIC_CHECKBIT(stRectArea.AreaAttrib,E_AREA_ENTER_COLLECT_GNSS))//
            {
                Area_SetTestRectAreaParameter((T_GPS_POS *)&(stRectArea.LeftUpLat),(T_GPS_POS*)&(stRectArea.RightDownLat));                    
            }
            Area_ChoiceAreaArmDest(ALARM_ITEM_IN_RECT_AREA);
        }
    }
    else
    if(AREA_STATUS_IN == s_stRectArea.AreaFlag)
    {
        if(0==Area_CheckRectTime())//时间不符合
        {
            Area_RectAreaResetParam();
            return;
        }
        sFLASH_ReadBuffer((unsigned char *)&stRectArea,s_stRectArea.addr,AREA_ATTRIB_SIZE);//读出区域数据	        
        Status = Area_IsInRectArea(&stRectArea);	
        if(Status)
        {
            s_stRectArea.AreaFlag = AREA_STATUS_OUT;                
            sFLASH_ReadBuffer((unsigned char *)&stRectArea,s_stRectArea.addr,STRECT_AREA_ATTRIB_SIZE);//读出区域数据
            if(PUBLIC_CHECKBIT(stRectArea.AreaAttrib,E_OUT_SENDTO_DRIVER)) 
            {
                if(PUBLIC_CHECKBIT(stRectArea.UserAreaAttrib,E_USER_OUT_PLAY))
                {
                    len=stRectArea.OutVoiceSize;
                    if(len>=VOICE_BYTE)len=VOICE_BYTE-1;
                    for(j=0; j<len;j++)
                    {
                        s_stRectArea.VoiceStr[j]=stRectArea.OutVoiceStr[j];
                    }
                    s_stRectArea.VoiceStr[j]='\0'; 
                }
                else
                {
                    strcpy((char *)s_stRectArea.VoiceStr,"你已离开矩形区域");
                }
            }
            ////////////////////////////
            s_stRectArea.AreaID = stRectArea.AreaID;
            s_stRectArea.AreaAttrib = stRectArea.AreaAttrib;
            s_stRectArea.UserAreaAttrib = stRectArea.UserAreaAttrib;              
            Area_ChoiceAreaArmDest(ALARM_ITEM_OUT_RECT_AREA);  
            ///////////////////
            s_stRectArea.addr = MAX_AREA_ADDR;                
        }
    }
    //////////////////////////
    //在区域内
    if((AREA_STATUS_IN==s_stRectArea.AreaFlag)&&PUBLIC_CHECKBIT(s_stRectArea.AreaAttrib,E_AREA_PROHIBIT_OPENDOOR))
    {
        Area_CheckOpenDoorAlarm(E_RECT_AREA);
    }
    //////////////////////////////////  
    Area_ScanTestRectArea();
}
/************************************************************
** 函数名称: Area_ClearInOutRectAlarm
** 功能描述: 
** 入口参数: 
** 出口参数:
************************************************************/ 
void Area_ClearInOutRectAlarm(void) 
{
    Area_ClearInOutAreaAlarmBit(E_RECT_AREA);
    ///////////////////////////////////////
}
/************************************************************
** 函数名称: Area_DisposeDriveOutRectArea
** 功能描述: 处理驶出矩形区域
** 入口参数: 
** 出口参数:
************************************************************/
void Area_DisposeDriveOutRectArea(void) 
{
    Area_ClearAllAlarm(E_RECT_AREA);
    if(PUBLIC_CHECKBIT(s_stRectArea.AreaAttrib,E_OUT_SENDTO_DRIVER)) 
    {
        Public_PlayTTSVoiceStr(s_stRectArea.VoiceStr);
    }
    /////////////////////
    if(PUBLIC_CHECKBIT(s_stRectArea.AreaAttrib,E_AREA_ENTER_COLLECT_GNSS))//
    {
        //出区域重新关闭采集GNSS详细定位数据 ///////////////////////
        Area_TestRectAreaParameterInitialize();
    }
    ///////////////
    if(PUBLIC_CHECKBIT(s_stRectArea.AreaAttrib,E_OUT_SENDTO_SERVER)) 
    {
        //设为有效区域,一出就报
        Area_SetInOutAreaAlarmInfo(E_RECT_AREA,s_stRectArea.AreaID,AREA_STATUS_OUT);
    }
    //////////////////////
    if(PUBLIC_CHECKBIT(s_stRectArea.UserAreaAttrib,E_USER_NO_CHANGE_LIMIT_SPEED))//出区域不更改限速值
    { 
        if(PUBLIC_CHECKBIT(s_stRectArea.AreaAttrib,E_AREA_LIMIT_SPEED))//
        {
            SpeedMonitor_SetSpeedParamValid(E_SPEED_TYPE_GENERAL,0,s_stRectArea.LimitSpeed,s_stRectArea.speedDuration);
        }
        ////////////////////////////////
    }
}
/************************************************************
** 函数名称: Area_DisposeDriveInRectArea
** 功能描述: 处理驶入矩形区域
** 入口参数: 
** 出口参数:
************************************************************/
void Area_DisposeDriveInRectArea(void) 
{
    Area_ClearAllAlarm(E_RECT_AREA);
    //////////////////////
    if(PUBLIC_CHECKBIT(s_stRectArea.AreaAttrib,E_AREA_LIMIT_SPEED))//
    {
        SpeedMonitor_SetSpeedParamValid(E_SPEED_TYPE_RECT_AREA,s_stRectArea.AreaID,s_stRectArea.LimitSpeed,s_stRectArea.speedDuration);
    }    
    /////////////////预警限速////////////////
    ////////////////////////  
    if(PUBLIC_CHECKBIT(s_stRectArea.AreaAttrib,E_IN_SENDTO_DRIVER)) 
    {
        Public_PlayTTSVoiceStr(s_stRectArea.VoiceStr);
    }
    //////////////////////////////
    /////进区域关闭通信模块///////////////////////
    if(PUBLIC_CHECKBIT(s_stRectArea.AreaAttrib,E_AREA_ENTER_TURNOFF_GPRS))//
    {
        Area_SetCloseGPRSFlag(E_RECT_AREA);
    }
    ////////////////////////////
    if(PUBLIC_CHECKBIT(s_stRectArea.AreaAttrib,E_AREA_ENTER_COLLECT_GNSS))//
    {
        //进区域采集GNSS详细定位数据/////////////////////
        Area_SetCollectGnssFlag(E_RECT_AREA);
    }
    ///////////////////
    if(PUBLIC_CHECKBIT(s_stRectArea.AreaAttrib,E_IN_SENDTO_SERVER)) 
    {//设为禁区,一进入就报
        Area_SetInOutAreaAlarmInfo(E_RECT_AREA,s_stRectArea.AreaID,AREA_STATUS_IN);
    }
    Area_SetInAreaFlag(E_RECT_AREA);
}
/************************************************************
** 函数名称: Area_DisposeDriveOutRoundArea
** 功能描述: 处理驶出圆形区域
** 入口参数: 
** 出口参数:
************************************************************/
void Area_DisposeDriveOutRoundArea(void) 
{
    Area_ClearAllAlarm(E_ROUND_AREA);
    if(PUBLIC_CHECKBIT(s_stRoundArea.AreaAttrib,E_OUT_SENDTO_DRIVER)) 
    {
        Public_PlayTTSVoiceStr(s_stRoundArea.VoiceStr);
    }
    ////////////////////////////
    if(PUBLIC_CHECKBIT(s_stRoundArea.AreaAttrib,E_OUT_SENDTO_SERVER)) 
    {//设为有效区域,一出就报
        Area_SetInOutAreaAlarmInfo(E_ROUND_AREA,s_stRoundArea.AreaID,AREA_STATUS_OUT);
    }
    ///////////////////
    if(PUBLIC_CHECKBIT(s_stRoundArea.UserAreaAttrib,E_USER_NO_CHANGE_LIMIT_SPEED))//出区域不更改限速值
    { 
        if(PUBLIC_CHECKBIT(s_stRoundArea.AreaAttrib,E_AREA_LIMIT_SPEED))//
        {
            SpeedMonitor_SetSpeedParamValid(E_SPEED_TYPE_GENERAL,0,s_stRoundArea.LimitSpeed,s_stRoundArea.speedDuration);
        }
    }
}
/************************************************************
** 函数名称: Area_DisposeDriveInRectArea
** 功能描述: 处理驶入圆形区域
** 入口参数: 
** 出口参数:
************************************************************/
void Area_DisposeDriveInRoundArea(void) 
{ 
    Area_ClearAllAlarm(E_ROUND_AREA);    
    //////////////////////////////////    
    if(PUBLIC_CHECKBIT(s_stRoundArea.AreaAttrib,E_AREA_LIMIT_SPEED))//是否有区域限速
    {
        SpeedMonitor_SetSpeedParamValid(E_SPEED_TYPE_ROUND_AREA,s_stRoundArea.AreaID,s_stRoundArea.LimitSpeed,s_stRoundArea.speedDuration);
    }
    //////////////////////  
    if(PUBLIC_CHECKBIT(s_stRoundArea.AreaAttrib,E_IN_SENDTO_DRIVER)) 
    {
        Public_PlayTTSVoiceStr(s_stRoundArea.VoiceStr);
    }
    /////////////////////////
    /////进区域关闭通信模块///////////////////////
    if(PUBLIC_CHECKBIT(s_stRoundArea.AreaAttrib,E_AREA_ENTER_TURNOFF_GPRS))//
    {
        //Area_CloseGSMModule();
        Area_SetCloseGPRSFlag(E_ROUND_AREA);
    }
    ////////////////////////////
    if(PUBLIC_CHECKBIT(s_stRoundArea.AreaAttrib,E_AREA_ENTER_COLLECT_GNSS))//
    {
        //进区域采集GNSS详细定位数据/////////////////////
        Area_SetCollectGnssFlag(E_ROUND_AREA);
    } 
    ///////////////////
    if(PUBLIC_CHECKBIT(s_stRoundArea.AreaAttrib,E_IN_SENDTO_SERVER)) 
    {//设为禁区,一进入就报
        Area_SetInOutAreaAlarmInfo(E_ROUND_AREA,s_stRoundArea.AreaID,AREA_STATUS_IN);
    }
    ///////////////
    Area_SetInAreaFlag(E_ROUND_AREA);
}

/********************************************************************
* 功能 : 选择报警发送对象
* 输入: 
* 输出: 
* 全局变量: 
*
* 备注: 
// 0,超速  1:进圆形区 2,进矩形区域 3:出圆形区 4:出矩形区 5:进线 6:出线 7:偏离线路
typedef enum{ALARM_ITEM_SPEED_AREA=0,ALARM_ITEM_IN_ROUND_AREA,ALARM_ITEM_IN_RECT_AREA,ALARM_ITEM_OUT_ROUND_AREA,ALARM_ITEM_OUT_RECT_AREA} E_AREA_ALARM_ITEM;
********************************************************************/
void Area_ChoiceAreaArmDest(unsigned char AlarmItem) 
{
    switch(AlarmItem)
    {
        case ALARM_ITEM_SPEED_AREA:
            //Io_WriteOverSpeedConrtolBit(OVER_SPEED_BIT_AREA,SET);
            break;
        case ALARM_ITEM_OUT_RECT_AREA:
            Area_DisposeDriveOutRectArea();                    
            break;
        case ALARM_ITEM_OUT_ROUND_AREA:
            Area_DisposeDriveOutRoundArea();            
            break;           
        case  ALARM_ITEM_IN_RECT_AREA:
            Area_DisposeDriveInRectArea();            
            break;  
        case ALARM_ITEM_IN_ROUND_AREA:            
            Area_DisposeDriveInRoundArea();
            break;                                                                
        default:
            break;    
    }
}
/*********************************************************************
//函数名称	:Area_GetRectAreaUserDatalen
//功能		:设置矩形区域
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:
*********************************************************************/
unsigned short Area_GetRectAreaUserDatalen(unsigned char *pBuffer, unsigned short BufferLen,unsigned long *AreaID)
{
    unsigned short datalen;
    STRECT_AREA_ATTRIB stRectAreaAttrib;  //读取矩形区域属性
    datalen =0;
    //获得区域ID
    Public_Mymemcpy((unsigned char *)&stRectAreaAttrib.AreaID,(unsigned char *)&pBuffer[datalen],sizeof(unsigned long),1); 
    datalen+=sizeof(unsigned long);
    //获得区域属性
    Public_Mymemcpy((unsigned char *)&stRectAreaAttrib.UserAreaAttrib,(unsigned char *)&pBuffer[datalen],sizeof(unsigned short ),1); 
    datalen+=sizeof(unsigned short );    
    ///////自定义位///////////////////////    
    if(PUBLIC_CHECKBIT(stRectAreaAttrib.UserAreaAttrib,E_USER_IN_PLAY))
    {     
        //进区提示音长度
        Public_Mymemcpy((unsigned char *)&stRectAreaAttrib.InVoiceSize,(unsigned char *)&pBuffer[datalen],sizeof(unsigned char),0); 
        datalen+=sizeof(unsigned char);
        //提示音语句
        datalen+=stRectAreaAttrib.InVoiceSize;    
    }
    ///////////////////////
    if(PUBLIC_CHECKBIT(stRectAreaAttrib.UserAreaAttrib,E_USER_OUT_PLAY))
    {
         //出区提示音长度
        Public_Mymemcpy((unsigned char *)&stRectAreaAttrib.OutVoiceSize,(unsigned char *)&pBuffer[datalen],sizeof(unsigned char),0);
        datalen+=sizeof(unsigned char);
        //提示音语句
        datalen+=stRectAreaAttrib.OutVoiceSize;
    }
    /////////////////////////
    if(datalen>BufferLen)return 0;
    *AreaID =stRectAreaAttrib.AreaID;
    return datalen;
}
/*************************************************************
** 函数名称: Area_GetRectAreaDatalenStandard
** 功能描述: :设置区域时,标准协议长度
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned short Area_GetRectAreaDatalenStandard(unsigned char *pBuffer, unsigned short BufferLen,unsigned long *AreaID)
{
    unsigned short datalen;
    STRECT_AREA_ATTRIB stRectAreaAttrib;  //读取矩形区域属性
    datalen =0;
    //获得区域ID
    Public_Mymemcpy((unsigned char *)&stRectAreaAttrib.AreaID,(unsigned char *)&pBuffer[datalen],sizeof(unsigned long),1); 
    datalen+=sizeof(unsigned long);
    //获得区域属性
    Public_Mymemcpy((unsigned char *)&stRectAreaAttrib.AreaAttrib,(unsigned char *)&pBuffer[datalen],sizeof(unsigned short ),1); 
    datalen+=sizeof(unsigned short ); 
     //获得左上点纬度
    datalen+=sizeof(unsigned long);  
    //获得左上点经度
    datalen+=sizeof(unsigned long);  
    
    //获得右下点纬度
    datalen+=sizeof(unsigned long);  
    //获得右下点经度
    datalen+=sizeof(unsigned long);
    //////根据时间////////////////////////
    if(PUBLIC_CHECKBIT(stRectAreaAttrib.AreaAttrib,E_AREA_MOMENT))
    {
        //起始时间
        datalen += 6;  
        //结束时间
        datalen += 6;  
    }
    /////最高限速/////////////////////
    if(PUBLIC_CHECKBIT(stRectAreaAttrib.AreaAttrib,E_AREA_LIMIT_SPEED))
    {
        //最高限速
        datalen +=sizeof(unsigned short);    
        //超速持续时间
        datalen +=sizeof(unsigned char); 
    }   
    ///////////////////
    if(datalen>BufferLen)return 0;
    ///////////////////////
    *AreaID =stRectAreaAttrib.AreaID;
    return datalen;
}
/*************************************************************
** 函数名称: Area_GetRectAreaDatalenJiuTong
** 功能描述: :设置区域时,九通协议长度
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned short Area_GetRectAreaDatalenJiuTong(unsigned char *pBuffer, unsigned short BufferLen,unsigned long *AreaID)
{
    unsigned short datalen;
    STRECT_AREA_ATTRIB stRectAreaAttrib;  //读取矩形区域属性
    datalen =0;
    //获得区域ID
    Public_Mymemcpy((unsigned char *)&stRectAreaAttrib.AreaID,(unsigned char *)&pBuffer[datalen],sizeof(unsigned long),1); 
    datalen+=sizeof(unsigned long);
    //获得区域属性
    Public_Mymemcpy((unsigned char *)&stRectAreaAttrib.AreaAttrib,(unsigned char *)&pBuffer[datalen],sizeof(unsigned short ),1); 
    datalen+=sizeof(unsigned short ); 
     //获得左上点纬度
    datalen+=sizeof(unsigned long);  
    //获得左上点经度
    datalen+=sizeof(unsigned long);  
    
    //获得右下点纬度
    datalen+=sizeof(unsigned long);  
    //获得右下点经度
    datalen+=sizeof(unsigned long);
    //////根据时间////////////////////////
    if(PUBLIC_CHECKBIT(stRectAreaAttrib.AreaAttrib,E_AREA_MOMENT))
    {
        //起始时间
        datalen += 6;  
        //结束时间
        datalen += 6;  
    }
    /////最高限速/////////////////////
    if(PUBLIC_CHECKBIT(stRectAreaAttrib.AreaAttrib,E_AREA_LIMIT_SPEED))
    {
        //最高限速
        datalen +=sizeof(unsigned short);    
        //超速持续时间
        datalen +=sizeof(unsigned char); 
    }    
    ///////自定义位///////////////////////
    /////////////////////////////////
    if(PUBLIC_CHECKBIT(stRectAreaAttrib.AreaAttrib,14))
    {     
        //进区提示音长度
        Public_Mymemcpy((unsigned char *)&stRectAreaAttrib.InVoiceSize,(unsigned char *)&pBuffer[datalen],sizeof(unsigned char),0); 
        datalen+=sizeof(unsigned char);
        //提示音语句
        datalen+=stRectAreaAttrib.InVoiceSize;    
    }
    ///////////////////////
    if(PUBLIC_CHECKBIT(stRectAreaAttrib.AreaAttrib,15))
    {
        //出区提示音长度
        Public_Mymemcpy((unsigned char *)&stRectAreaAttrib.OutVoiceSize,(unsigned char *)&pBuffer[datalen],sizeof(unsigned char),0);
        datalen+=sizeof(unsigned char);
        //提示音语句
        datalen+=stRectAreaAttrib.OutVoiceSize;
    }    
    ///////////////////
    if(datalen>BufferLen)return 0;
    ///////////////////////
    *AreaID =stRectAreaAttrib.AreaID;
    return datalen;
}
/*************************************************************
** 函数名称: Area_GetRectAreaDatalenSuoMei
** 功能描述: :设置区域时,索美协议长度
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned short Area_GetRectAreaDatalenSuoMei(unsigned char *pBuffer, unsigned short BufferLen,unsigned long *AreaID)
{
    unsigned short datalen=0;
    return datalen;
}
/*********************************************************************
//函数名称	:Area_SaveOneRectArea
//功能		:设置矩形区域
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:
*********************************************************************/
unsigned short Area_SaveOneRectModifyUserData(unsigned long addr,unsigned char *pBuffer, unsigned short BufferLen)
{
    unsigned long AreaID;
    unsigned short datalen,UserAreaAttrib;
    STRECT_AREA_ATTRIB stRectAreaAttrib;  //读取矩形区域属性    
    if(addr+STRECT_AREA_ATTRIB_SIZE>RECT_AREA_ADDR_END)return 0;
    sFLASH_ReadBuffer((unsigned char*)&stRectAreaAttrib,addr,STRECT_AREA_ATTRIB_SIZE); //读扇区
    datalen = 0;
    Public_Mymemcpy((unsigned char *)&AreaID,(unsigned char *)&pBuffer[datalen],sizeof(unsigned long),1); //获得区域ID
    if(AreaID!=stRectAreaAttrib.AreaID)return 0;
    datalen+=sizeof(unsigned long);
    Public_Mymemcpy((unsigned char *)&UserAreaAttrib,(unsigned char *)&pBuffer[datalen],sizeof(unsigned short ),1); //获得区域属性
    datalen+=sizeof(unsigned short ); 
    //////////自定义/////////////////    
    if(PUBLIC_CHECKBIT(UserAreaAttrib,E_USER_IN_PLAY))
    { 
        PUBLIC_SETBIT(stRectAreaAttrib.UserAreaAttrib,E_USER_IN_PLAY);
        Public_Mymemcpy((unsigned char *)&stRectAreaAttrib.InVoiceSize,(unsigned char *)&pBuffer[datalen],sizeof(unsigned char),0); //进区提示音长度
        datalen+=sizeof(unsigned char);  
        Public_Mymemcpy(stRectAreaAttrib.InVoiceStr,(unsigned char *)&pBuffer[datalen],stRectAreaAttrib.InVoiceSize,0); //提示音语句
        datalen+=stRectAreaAttrib.InVoiceSize;    
    }
    if(PUBLIC_CHECKBIT(UserAreaAttrib,E_USER_OUT_PLAY))
    {
        PUBLIC_SETBIT(stRectAreaAttrib.UserAreaAttrib,E_USER_OUT_PLAY);
        Public_Mymemcpy((unsigned char *)&stRectAreaAttrib.OutVoiceSize,(unsigned char *)&pBuffer[datalen],sizeof(unsigned char),0); //出区提示音长度
        datalen+=sizeof(unsigned char);  
        Public_Mymemcpy(stRectAreaAttrib.OutVoiceStr,(unsigned char *)&pBuffer[datalen],stRectAreaAttrib.OutVoiceSize,0); //提示音语句
        datalen+=stRectAreaAttrib.OutVoiceSize;
    }
    ////////////////////////////
    if(datalen>BufferLen)return 0;
    //////////////////////////
    Area_MdyDataInFlash((unsigned char*)&stRectAreaAttrib,addr,STRECT_AREA_ATTRIB_SIZE);
    return datalen;
}
/*********************************************************************
//函数名称	:Area_SaveOneRectArea
//功能		:设置矩形区域
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:
*********************************************************************/
unsigned short Area_SaveOneRectArea(unsigned long addr,unsigned char *pBuffer, unsigned short BufferLen)
{
    unsigned short datalen;
    STRECT_AREA_ATTRIB stRectAreaAttrib;  //读取矩形区域属性    
    if(addr+STRECT_AREA_ATTRIB_SIZE>RECT_AREA_ADDR_END)return 0;
    memset(&stRectAreaAttrib,0,sizeof(STRECT_AREA_ATTRIB));
    datalen =0;
    Public_Mymemcpy((unsigned char *)&stRectAreaAttrib.AreaID,(unsigned char *)&pBuffer[datalen],sizeof(unsigned long),1); //获得区域ID
    datalen+=sizeof(unsigned long);
    Public_Mymemcpy((unsigned char *)&stRectAreaAttrib.AreaAttrib,(unsigned char *)&pBuffer[datalen],sizeof(unsigned short ),1); //获得区域属性
    datalen+=sizeof(unsigned short ); 
    Public_Mymemcpy((unsigned char *)&stRectAreaAttrib.LeftUpLat,(unsigned char *)&pBuffer[datalen],sizeof(unsigned long),1); //获得左上点纬度
    datalen+=sizeof(unsigned long);  
    Public_Mymemcpy((unsigned char *)&stRectAreaAttrib.LeftUpLong,(unsigned char *)&pBuffer[datalen],sizeof(unsigned long),1); //获得左上点经度
    datalen+=sizeof(unsigned long);  
    
    Public_Mymemcpy((unsigned char *)&stRectAreaAttrib.RightDownLat,(unsigned char *)&pBuffer[datalen],sizeof(unsigned long),1); //获得右下点纬度
    datalen+=sizeof(unsigned long);  
    Public_Mymemcpy((unsigned char *)&stRectAreaAttrib.RightDownLong,(unsigned char *)&pBuffer[datalen],sizeof(unsigned long),1); //获得右下点经度
    datalen+=sizeof(unsigned long); 

    //////////根据时间///////////////////
    if(PUBLIC_CHECKBIT(stRectAreaAttrib.AreaAttrib,E_AREA_MOMENT))
    {
        Public_Mymemcpy((unsigned char *)&stRectAreaAttrib.StartMoment[0],(unsigned char *)&pBuffer[datalen],6,0); //起始时间
        datalen+=6;  
        Public_Mymemcpy((unsigned char *)&stRectAreaAttrib.EndMoment[0],(unsigned char *)&pBuffer[datalen],6,0); //结束时间
        datalen+=6;  
    }
    //////////最高限速////////////
    if(PUBLIC_CHECKBIT(stRectAreaAttrib.AreaAttrib,E_AREA_LIMIT_SPEED))
    {
        Public_Mymemcpy((unsigned char *)&stRectAreaAttrib.LimitSpeed,(unsigned char *)&pBuffer[datalen],sizeof(unsigned short ),1); //最高
        datalen+=sizeof(unsigned short );    
        Public_Mymemcpy((unsigned char *)&stRectAreaAttrib.speedDuration,(unsigned char *)&pBuffer[datalen],sizeof(unsigned char),0); //超速持续时间
        datalen+=sizeof(unsigned char);  
    }
    //////////////////////////
    if(AREA_PROTOCOL_TYPE_JIUTONG == s_stRectArea.protoclType)
    {
        if(PUBLIC_CHECKBIT(stRectAreaAttrib.AreaAttrib,14))//
        {
            PUBLIC_CLRBIT(stRectAreaAttrib.AreaAttrib,14);
            PUBLIC_SETBIT(stRectAreaAttrib.UserAreaAttrib,E_USER_IN_PLAY);               
            Public_Mymemcpy((unsigned char *)&stRectAreaAttrib.InVoiceSize,(unsigned char *)&pBuffer[datalen],sizeof(unsigned char),0); //进区提示音长度
            datalen+=sizeof(unsigned char);  
            Public_Mymemcpy((unsigned char *)&stRectAreaAttrib.InVoiceStr[0],(unsigned char *)&pBuffer[datalen],stRectAreaAttrib.InVoiceSize,0); //提示音语句
            datalen+=stRectAreaAttrib.InVoiceSize;    
        }
        ///////////////////////
        if(PUBLIC_CHECKBIT(stRectAreaAttrib.AreaAttrib,15))
        {
            PUBLIC_CLRBIT(stRectAreaAttrib.AreaAttrib,15);
            PUBLIC_SETBIT(stRectAreaAttrib.UserAreaAttrib,E_USER_OUT_PLAY);
            Public_Mymemcpy((unsigned char *)&stRectAreaAttrib.OutVoiceSize,(unsigned char *)&pBuffer[datalen],sizeof(unsigned char),0); //出区提示音长度
            datalen+=sizeof(unsigned char);  
            Public_Mymemcpy((unsigned char *)&stRectAreaAttrib.OutVoiceStr[0],(unsigned char *)&pBuffer[datalen],stRectAreaAttrib.OutVoiceSize,0); //提示音语句
            datalen+=stRectAreaAttrib.OutVoiceSize;
        }
    }
    //////////////////////////
    if(datalen>BufferLen)return 0;
    //////////////////////
    stRectAreaAttrib.Head = HEAD_FLAG;
    ///////////////////////////
    Area_MdyDataInFlash((unsigned char*)&stRectAreaAttrib,addr,STRECT_AREA_ATTRIB_SIZE);
    return datalen;
}

/*********************************************************************
//函数名称	:Area_RectAreaUpdate(unsigned char *pBuffer, unsigned short BufferLen)
//功能		:设置矩形区域
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:
*********************************************************************/
unsigned char Area_RectAreaUpdate(unsigned char *pBuffer, unsigned short BufferLen)
{
    unsigned long OffSetAddr;
    unsigned char i,Sum;
    unsigned short datalen,templen;
    unsigned char *pTmp;
    //////////////
    if(BufferLen==0)return 2;  
    ////////////////////////
    Sum=*pBuffer++;
    if(Sum>MAX_RECT_AREA_SUM)return 1;
    ////////////////////    
    /////////////////
    Area_ClearRectArea();
    ////////////
    datalen=BufferLen-1;
    pTmp=pBuffer;
    //矩形
    Area_RectAreaParameterInitialize(); 
    for(i=0; i<Sum; i++)
    {
        OffSetAddr=RECT_AREA_ADDR_START + i*STRECT_AREA_ATTRIB_SIZE;
        templen=Area_SaveOneRectArea(OffSetAddr,pTmp,datalen);
        if(0 == templen) break;
        s_stAreaAttrib.CheckRectNum++;
        s_stAreaAttrib.ValidRectNum++;
        if(s_stAreaAttrib.CheckRectNum>=MAX_RECT_AREA_SUM)break;
        pTmp +=templen;
        if(datalen>templen)datalen -=templen;
        else break;
    }
    return 0;    
}

/*************************************************************
** 函数名称: Area_GetCurRectAreaDatalen
** 功能描述: 获取当前矩形结构体长度
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned short Area_GetCurRectAreaDatalen(unsigned char *pBuffer, unsigned short BufferLen,unsigned long *AreaID)
{
    if(AREA_PROTOCOL_TYPE_JIUTONG == s_stRectArea.protoclType)
    {
        return Area_GetRectAreaDatalenJiuTong(pBuffer,BufferLen,AreaID);
    }
    else
    {
       return Area_GetRectAreaDatalenStandard(pBuffer,BufferLen,AreaID);
    }
}
/*********************************************************************
//函数名称	:Area_RectAreaAdd(unsigned char *pBuffer, unsigned short BufferLen)
//功能		:设置矩形区域
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:
*********************************************************************/
unsigned char Area_RectAreaAdd(unsigned char *pBuffer, unsigned short BufferLen)
{
    unsigned long OffSetAddr,AreaID;
    unsigned char i,j,Sum;
    STRECT_AREA_ATTRIB stRectAreaAttrib;  //读取矩形区域属性
    unsigned short datalen,templen;
    unsigned char *pTmp;
    //////////////
    if(BufferLen==0)return 2;
    ////////////
    Sum=*pBuffer++;
    /////////////////////////
    if(Sum+s_stAreaAttrib.CheckRectNum>MAX_RECT_AREA_SUM)return 1;//失败
    //////////////////////////
    pTmp =pBuffer;
    datalen=BufferLen-1;
    //矩形
    for(i=0; i<Sum; i++)//检查ID
    {    
        templen=Area_GetCurRectAreaDatalen(pTmp,datalen,&AreaID);
        if(0 == templen)return 2;//消息有误;
        //////////////////////////
        for(j=0; j<s_stAreaAttrib.CheckRectNum; j++)
        {
            OffSetAddr=RECT_AREA_ADDR_START + j*STRECT_AREA_ATTRIB_SIZE;
            sFLASH_ReadBuffer((unsigned char *)&stRectAreaAttrib,OffSetAddr,AREA_HEAD_SIZE);
            if(AreaID==stRectAreaAttrib.AreaID)
            {
                return 1;////失败
            }
        }
        ///////////////////////////
        pTmp +=templen;
        if(datalen>templen)datalen -=templen;
        else break;
    }
    //////////////////////////////////////
    pTmp =pBuffer;
    datalen=BufferLen-1;    
    for(i=0; i<Sum; i++)
    {
        OffSetAddr=RECT_AREA_ADDR_START + s_stAreaAttrib.CheckRectNum*STRECT_AREA_ATTRIB_SIZE;
        templen=Area_SaveOneRectArea(OffSetAddr,pTmp,datalen);
        if(0 == templen)break;
        ////////////////////
        s_stAreaAttrib.CheckRectNum++;//
        s_stAreaAttrib.ValidRectNum++;
        if(s_stAreaAttrib.CheckRectNum>=MAX_RECT_AREA_SUM)break;
        ////////////////////
        pTmp +=templen;
        if(datalen>templen)datalen -=templen;
        else break;
    }    
    return 0;    
}
/*********************************************************************
//函数名称	:Area_RectAreaModify(unsigned char *pBuffer, unsigned short BufferLen)
//功能		:修改矩形区域
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:
*********************************************************************/
unsigned char Area_RectAreaModify(unsigned char *pBuffer, unsigned short BufferLen)
{
    unsigned long OffSetAddr,AreaID;
    unsigned char i,j,Sum,flag;
    STRECT_AREA_ATTRIB stRectAreaAttrib;  //读取矩形区域属性
    unsigned short datalen,templen;
    unsigned char *pTmp;
    //////////////
    if(BufferLen==0)return 2;
    ////////////
    Sum=*pBuffer++;
    pTmp =pBuffer;
    datalen=BufferLen-1;
    //矩形
    for(i=0; i<Sum; i++)//检查ID
    {  
        templen=Area_GetCurRectAreaDatalen(pTmp,datalen,&AreaID);        
        if(0 == templen)return 2;//消息有误;
        //////////////////////////
        flag =0;
        for(j=0; j<s_stAreaAttrib.CheckRectNum; j++)
        {
            OffSetAddr=RECT_AREA_ADDR_START + j*STRECT_AREA_ATTRIB_SIZE;
            sFLASH_ReadBuffer((unsigned char *)&stRectAreaAttrib,OffSetAddr,AREA_HEAD_SIZE);
            if(AreaID==stRectAreaAttrib.AreaID)
            {
                flag=1;//
                break;
            }
        }
        //////////////////
        if(0==flag)return 1;//失败;
        ///////////////////////////
        pTmp +=templen;
        if(datalen>templen)datalen -=templen;
        else break;
    }
    //////////////////////////////////////
    pTmp =pBuffer;
    datalen=BufferLen-1;
    for(i=0; i<Sum; i++)
    {
        templen = 0;
        Public_Mymemcpy((unsigned char *)&AreaID,(unsigned char *)&pTmp[0],sizeof(unsigned long),1); //获得区域ID
        for(j=0; j<s_stAreaAttrib.CheckRectNum; j++)
        {
            OffSetAddr=RECT_AREA_ADDR_START + j*STRECT_AREA_ATTRIB_SIZE;
            sFLASH_ReadBuffer((unsigned char *)&stRectAreaAttrib,OffSetAddr,AREA_HEAD_SIZE);
            if(AreaID==stRectAreaAttrib.AreaID)
            {
                templen=Area_SaveOneRectArea(OffSetAddr,pTmp,datalen);
                break;
            }
        }
        ///////////////
        Area_RectAreaCheckForResetParam(AreaID);
        /////////////////
        if(0 == templen)break;
        ////////////////////
        pTmp +=templen;
        if(datalen>templen)datalen -=templen;
        else break;
    }    
    return 0;    
}
/*************************************************************
** 函数名称: Area_SetRectAreaCheckProtoclType
** 功能描述: :设置区域时,查检是否有语音标志,仅为兼容九通版本
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Area_SetRectAreaCheckProtoclType(unsigned char *pBuffer, unsigned short BufferLen)
{
    unsigned long AreaID;
    unsigned char i,Sum;
    unsigned short datalen,templen;
    unsigned char *pTmp;
    //////////////    
    if(BufferLen==0)return AREA_PROTOCOL_TYPE_EMPTY;
    ////////////////////////    
    Sum=*pBuffer++;
    ////////////////////
    datalen=BufferLen-1;
    pTmp=pBuffer;
    ////////////////////
    for(i=0; i<Sum; i++)
    {
        templen=Area_GetRectAreaDatalenStandard(pTmp,datalen,&AreaID);
        if(0 == templen)//出错
        {
            break;
        }
        pTmp += templen;
        //////////////////////////
        if(datalen == templen)//相等,正确
        {
            return AREA_PROTOCOL_TYPE_STANDARD;
        }
        else
        if(datalen > templen)//大于,还有数据
        {
            datalen -= templen;
        }
        else////小于,出错
        {
            break;
        }
    }
    /////////////////////
    datalen=BufferLen-1;
    pTmp=pBuffer;
    ////////////////////
    for(i=0; i<Sum; i++)
    {
        templen=Area_GetRectAreaDatalenJiuTong(pTmp,datalen,&AreaID);
        if(0 == templen)//出错
        {
            break;
        }
        pTmp += templen;
        ///////////////////
        if(datalen == templen)//相等,正确
        {
            return AREA_PROTOCOL_TYPE_JIUTONG;//方法二,正确
        }
        else
        if(datalen > templen)//大于,还有数据
        {
            datalen -= templen;
        }
        else////小于,出错
        {
            break;
        }
    }
    return AREA_PROTOCOL_TYPE_EMPTY;
}
/*************************************************************
** 函数名称: Area_SetRectArea
** 功能描述: :设置矩形区域
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Area_SetRectArea(unsigned char *pBuffer, unsigned short BufferLen)
{
    unsigned char AckResult,type;
    unsigned char protoclType;
    if(BufferLen==0)return 2;
    /////////////////
    type=*pBuffer++;
    //////////////////////
    protoclType=Area_SetRectAreaCheckProtoclType(pBuffer,BufferLen-1);
    if(AREA_PROTOCOL_TYPE_EMPTY==protoclType)return 2;
    s_stRectArea.protoclType = protoclType;
    //////////////
    AckResult = 0;
    if(0== type)//更新
    {
        AckResult=Area_RectAreaUpdate(pBuffer,BufferLen-1);
    }
    else
    if(1 == type)//添加
    {   
        AckResult=Area_RectAreaAdd(pBuffer,BufferLen-1);
    }
    else
    if(2 == type)//修改
    {  
        AckResult=Area_RectAreaModify(pBuffer,BufferLen-1);
    }
    if(0==AckResult)
    {
        s_stAreaAttrib.preSpeed = 0xff;
    }
    return AckResult;
}
/*************************************************************
** 函数名称: Area_DelRectArea
** 功能描述: :删除矩形区域
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Area_DelRectArea(unsigned char *pBuffer, unsigned short BufferLen) 
{
    unsigned char i,j,Sum,flag;
    unsigned long OffSetAddr,AreaID;
    STRECT_AREA_ATTRIB stRectArea;  //矩形区域属性结构
    unsigned char *pTmp;
    
    if(BufferLen==0) return 2;
    Sum=*pBuffer++;    
    if(Sum)
    {
        pTmp =pBuffer;
        for(i=0; i<Sum; i++)
        {
            Public_Mymemcpy((unsigned char *)&AreaID,(unsigned char *)&pTmp[0],sizeof(unsigned long),1); //获得区域ID
            pTmp+=sizeof(unsigned long);
            flag =0;
            for(j=0; j<s_stAreaAttrib.CheckRectNum; j++)
            {
                OffSetAddr=RECT_AREA_ADDR_START + j*STRECT_AREA_ATTRIB_SIZE;
                sFLASH_ReadBuffer((unsigned char *)&stRectArea,OffSetAddr,AREA_HEAD_SIZE);
                if(AreaID==stRectArea.AreaID)
                {
                   flag =1;
                   break;
                }
            }
            ///////////////////////
            if(0 == flag)return 2;
            ///////////////////////
        }
        ///////////////////////////
        pTmp =pBuffer;
        for(i=0; i<Sum; i++)
        {
            Public_Mymemcpy((unsigned char *)&AreaID,(unsigned char *)&pTmp[0],sizeof(unsigned long),1); //获得区域ID
            pTmp+=sizeof(unsigned long);
            for(j=0; j<s_stAreaAttrib.CheckRectNum; j++)
            {
                OffSetAddr=RECT_AREA_ADDR_START + j*STRECT_AREA_ATTRIB_SIZE;
                sFLASH_ReadBuffer((unsigned char *)&stRectArea,OffSetAddr,AREA_HEAD_SIZE);
                if(AreaID==stRectArea.AreaID&&AreaID)//需要删除的区域ID
                {
                    stRectArea.AreaID = 0;  //删除区域,只需把ID=0
                    s_stAreaAttrib.ValidRectNum--;
                    OffSetAddr=RECT_AREA_ADDR_START + j*STRECT_AREA_ATTRIB_SIZE;
                    stRectArea.Head =HEAD_FLAG;
                    Area_MdyDataInFlash((unsigned char*)&stRectArea,OffSetAddr,AREA_HEAD_SIZE);
                }
            }
            ////////////////////////////
            Area_RectAreaCheckForResetParam(AreaID);
            ////////////////////////////
        } 
    }
    else//删除所有
    {
        Area_ClearRectArea();
    }
    //////////////
    return 0;
}
/*********************************************************************
//函数名称	:Area_GetRoundAreaDatalenStandard
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
unsigned short Area_GetRoundAreaUserDatalen(unsigned char *pBuffer, unsigned short BufferLen,unsigned long *AreaID)
{
    unsigned short datalen;
    STROUND_AREA_ATTRIB stRoundAreaAttrib;  //读取圆形区域属性
    datalen =0;    
    Public_Mymemcpy((unsigned char *)&stRoundAreaAttrib.AreaID,(unsigned char *)&pBuffer[datalen],sizeof(unsigned long),1); 
    datalen +=sizeof(unsigned long);
    //获得自定义区域属性   
    Public_Mymemcpy((unsigned char *)&stRoundAreaAttrib.UserAreaAttrib,(unsigned char *)&pBuffer[datalen],sizeof(unsigned short ),1); //获得区域属性
    datalen +=sizeof(unsigned short ); 
    ///////自定义位///////////////////////
    if(PUBLIC_CHECKBIT(stRoundAreaAttrib.UserAreaAttrib,E_USER_IN_PLAY))
    {     
        //进区提示音长度
        Public_Mymemcpy((unsigned char *)&stRoundAreaAttrib.InVoiceSize,(unsigned char *)&pBuffer[datalen],sizeof(unsigned char),0);
        datalen +=sizeof(unsigned char);  
        //提示音语句
        datalen += stRoundAreaAttrib.InVoiceSize;    
    }
    /////////////////////////////
    if(PUBLIC_CHECKBIT(stRoundAreaAttrib.UserAreaAttrib,E_USER_OUT_PLAY))
    {
        //出区提示音长度
        Public_Mymemcpy((unsigned char *)&stRoundAreaAttrib.OutVoiceSize,(unsigned char *)&pBuffer[datalen],sizeof(unsigned char),0); 
        datalen += sizeof(unsigned char);  
        //提示音语句
        datalen += stRoundAreaAttrib.OutVoiceSize;
    }
    ////////////////////////////
    if(datalen>BufferLen)return 0;
    ///////////////////////////
    *AreaID = stRoundAreaAttrib.AreaID;
    return datalen;
}

/*********************************************************************
//函数名称	:Area_GetRoundAreaDatalenStandard
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
unsigned short Area_GetRoundAreaDatalenStandard(unsigned char *pBuffer, unsigned short BufferLen,unsigned long *AreaID)
{
    unsigned short datalen;
    STROUND_AREA_ATTRIB stRoundAreaAttrib;  //读取圆形区域属性
    datalen =0;
    //获得区域ID
    Public_Mymemcpy((unsigned char *)&stRoundAreaAttrib.AreaID,(unsigned char *)&pBuffer[datalen],sizeof(unsigned long),1); 
    datalen +=sizeof(unsigned long);
    //获得区域属性   
    Public_Mymemcpy((unsigned char *)&stRoundAreaAttrib.AreaAttrib,(unsigned char *)&pBuffer[datalen],sizeof(unsigned short ),1); //获得区域属性
    datalen +=sizeof(unsigned short ); 
    //获得中心点纬度
    datalen +=sizeof(unsigned long);  
    //获得中心点经度
    datalen +=sizeof(unsigned long);  
    //获得半径
    datalen +=sizeof(unsigned long);  
    //////根据时间////////////////////////
    if(PUBLIC_CHECKBIT(stRoundAreaAttrib.AreaAttrib,E_AREA_MOMENT))
    {
        //起始时间
        datalen += 6;  
        //结束时间
        datalen += 6;  
    }
    /////最高限速/////////////////////
    if(PUBLIC_CHECKBIT(stRoundAreaAttrib.AreaAttrib,E_AREA_LIMIT_SPEED))
    {
        //最高限速
        datalen +=sizeof(unsigned short);    
        //超速持续时间
        datalen +=sizeof(unsigned char); 
    }
    ////////////////////
    if(datalen>BufferLen)return 0;
    ///////////////////////////
    *AreaID = stRoundAreaAttrib.AreaID;
    return datalen;
}
/*********************************************************************
//函数名称	:Area_GetRoundAreaDatalenStandard
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
unsigned short Area_GetRoundAreaDatalenJiuTong(unsigned char *pBuffer, unsigned short BufferLen,unsigned long *AreaID)
{
    unsigned short datalen;
    STROUND_AREA_ATTRIB stRoundAreaAttrib;  //读取圆形区域属性
    datalen =0;
    //获得区域ID
    Public_Mymemcpy((unsigned char *)&stRoundAreaAttrib.AreaID,(unsigned char *)&pBuffer[datalen],sizeof(unsigned long),1); 
    datalen +=sizeof(unsigned long);
    //获得区域属性   
    Public_Mymemcpy((unsigned char *)&stRoundAreaAttrib.AreaAttrib,(unsigned char *)&pBuffer[datalen],sizeof(unsigned short ),1); //获得区域属性
    datalen +=sizeof(unsigned short ); 
    //获得中心点纬度
    datalen +=sizeof(unsigned long);  
    //获得中心点经度
    datalen +=sizeof(unsigned long);  
    //获得半径
    datalen +=sizeof(unsigned long);  
    //////根据时间////////////////////////
    if(PUBLIC_CHECKBIT(stRoundAreaAttrib.AreaAttrib,E_AREA_MOMENT))
    {
        //起始时间
        datalen += 6;  
        //结束时间
        datalen += 6;  
    }
    /////最高限速/////////////////////
    if(PUBLIC_CHECKBIT(stRoundAreaAttrib.AreaAttrib,E_AREA_LIMIT_SPEED))
    {
        //最高限速
        datalen +=sizeof(unsigned short);    
        //超速持续时间
        datalen +=sizeof(unsigned char); 
    }
    //////////////////////////
    ///////自定义位///////////////////////
    /////////////////////////////////
    if(PUBLIC_CHECKBIT(stRoundAreaAttrib.AreaAttrib,14))
    {     
        //进区提示音长度
        Public_Mymemcpy((unsigned char *)&stRoundAreaAttrib.InVoiceSize,(unsigned char *)&pBuffer[datalen],sizeof(unsigned char),0); 
        datalen+=sizeof(unsigned char);
        //提示音语句
        datalen+=stRoundAreaAttrib.InVoiceSize;    
    }
    ///////////////////////
    if(PUBLIC_CHECKBIT(stRoundAreaAttrib.AreaAttrib,15))
    {
         //出区提示音长度
        Public_Mymemcpy((unsigned char *)&stRoundAreaAttrib.OutVoiceSize,(unsigned char *)&pBuffer[datalen],sizeof(unsigned char),0);
        datalen+=sizeof(unsigned char);
        //提示音语句
        datalen+=stRoundAreaAttrib.OutVoiceSize;
    }
    ////////////////////
    if(datalen>BufferLen)return 0;
    ///////////////////////////
    *AreaID = stRoundAreaAttrib.AreaID;
    return datalen;
}
/*********************************************************************
//函数名称	:Area_GetRoundAreaDatalenStandard
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
unsigned short Area_GetRoundAreaDatalenSuoMei(unsigned char *pBuffer, unsigned short BufferLen,unsigned long *AreaID)
{
    unsigned short datalen=0;
    return datalen;
}
/*********************************************************************
//函数名称	:Area_SaveOneRoundArea
//功能		:设置圆形区域
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:
*********************************************************************/
unsigned short Area_SaveOneRoundModifyUserData(unsigned long addr,unsigned char *pBuffer, unsigned short BufferLen)
{
    unsigned long AreaID;
    unsigned short datalen,UserAreaAttrib;
    STROUND_AREA_ATTRIB stRoundAreaAttrib;  //读取圆形区域属性
    if(addr+STROUND_AREA_ATTRIB_SIZE>ROUND_AREA_ADDR_END)return 0;
    sFLASH_ReadBuffer((unsigned char*)&stRoundAreaAttrib,addr,STROUND_AREA_ATTRIB_SIZE); //读扇区   
    datalen =0;
    Public_Mymemcpy((unsigned char *)&AreaID,(unsigned char *)&pBuffer[datalen],sizeof(unsigned long),1); //获得区域ID
    /////////////////////
    if(AreaID!=stRoundAreaAttrib.AreaID)return 0;
    //////////////////////////////
    datalen +=sizeof(unsigned long);
    Public_Mymemcpy((unsigned char *)&UserAreaAttrib,(unsigned char *)&pBuffer[datalen],sizeof(unsigned short ),1); //获得区域属性
    datalen +=sizeof(unsigned short );
    /////////自定义///////////////////////////
    if(PUBLIC_CHECKBIT(UserAreaAttrib,E_USER_IN_PLAY))
    {        
        PUBLIC_SETBIT(stRoundAreaAttrib.UserAreaAttrib,E_USER_IN_PLAY);
        Public_Mymemcpy((unsigned char *)&stRoundAreaAttrib.InVoiceSize,(unsigned char *)&pBuffer[datalen],sizeof(unsigned char),0); //进区提示音长度
        datalen +=sizeof(unsigned char);  
        Public_Mymemcpy(stRoundAreaAttrib.InVoiceStr,(unsigned char *)&pBuffer[datalen],stRoundAreaAttrib.InVoiceSize,0); //提示音语句
        datalen += stRoundAreaAttrib.InVoiceSize;    
    }
    if(PUBLIC_CHECKBIT(UserAreaAttrib,E_USER_OUT_PLAY))
    {
        PUBLIC_SETBIT(stRoundAreaAttrib.UserAreaAttrib,E_USER_OUT_PLAY);
        Public_Mymemcpy((unsigned char *)&stRoundAreaAttrib.OutVoiceSize,(unsigned char *)&pBuffer[datalen],sizeof(unsigned char),0); //出区提示音长度
        datalen += sizeof(unsigned char);  
        Public_Mymemcpy(stRoundAreaAttrib.OutVoiceStr,(unsigned char *)&pBuffer[datalen],stRoundAreaAttrib.OutVoiceSize,0); //提示音语句
        datalen += stRoundAreaAttrib.OutVoiceSize;
    }
    if(datalen>BufferLen)return 0;
    ///////////////////////////
    stRoundAreaAttrib.Head = HEAD_FLAG;
    Area_MdyDataInFlash((unsigned char*)&stRoundAreaAttrib,addr,STROUND_AREA_ATTRIB_SIZE);
    return datalen;
}
/*********************************************************************
//函数名称	:Area_SaveOneRoundArea
//功能		:设置圆形区域
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:
*********************************************************************/
unsigned short Area_SaveOneRoundArea(unsigned long addr,unsigned char *pBuffer, unsigned short BufferLen)
{
    unsigned short datalen;
    STROUND_AREA_ATTRIB stRoundAreaAttrib;  //读取圆形区域属性

    if(addr+STROUND_AREA_ATTRIB_SIZE>ROUND_AREA_ADDR_END)return 0;
    memset(&stRoundAreaAttrib,0,sizeof(STROUND_AREA_ATTRIB));
    datalen =0;
    Public_Mymemcpy((unsigned char *)&stRoundAreaAttrib.AreaID,(unsigned char *)&pBuffer[datalen],sizeof(unsigned long),1); //获得区域ID
    datalen +=sizeof(unsigned long);
    Public_Mymemcpy((unsigned char *)&stRoundAreaAttrib.AreaAttrib,(unsigned char *)&pBuffer[datalen],sizeof(unsigned short ),1); //获得区域属性
    datalen +=sizeof(unsigned short ); 
    Public_Mymemcpy((unsigned char *)&stRoundAreaAttrib.CenterLat,(unsigned char *)&pBuffer[datalen],sizeof(unsigned long),1); //获得中心点纬度
    datalen +=sizeof(unsigned long);  
    Public_Mymemcpy((unsigned char *)&stRoundAreaAttrib.CenterLong,(unsigned char *)&pBuffer[datalen],sizeof(unsigned long),1); //获得中心点经度
    datalen +=sizeof(unsigned long);  
    Public_Mymemcpy((unsigned char *)&stRoundAreaAttrib.Radius,(unsigned char *)&pBuffer[datalen],sizeof(unsigned long),1); //获得半径
    datalen +=sizeof(unsigned long);
    ////////////根据时间////////////////////
    if(PUBLIC_CHECKBIT(stRoundAreaAttrib.AreaAttrib,E_AREA_MOMENT))
    {
        Public_Mymemcpy((unsigned char *)&stRoundAreaAttrib.StartMoment[0],(unsigned char *)&pBuffer[datalen],6,0); //起始时间
        datalen += 6;  
        Public_Mymemcpy((unsigned char *)&stRoundAreaAttrib.EndMoment[0],(unsigned char *)&pBuffer[datalen],6,0); //结束时间
        datalen += 6;  
    }
    ////////////最高限速////////////////////
    if(PUBLIC_CHECKBIT(stRoundAreaAttrib.AreaAttrib,E_AREA_LIMIT_SPEED))
    {
        Public_Mymemcpy((unsigned char *)&stRoundAreaAttrib.LimitSpeed,(unsigned char *)&pBuffer[datalen],sizeof(unsigned short ),1); //起始时间
        datalen +=sizeof(unsigned short );    
        Public_Mymemcpy((unsigned char *)&stRoundAreaAttrib.speedDuration,(unsigned char *)&pBuffer[datalen],sizeof(unsigned char),0); //起始时间
        datalen +=sizeof(unsigned char); 
    }
    ///////////////////
    if(AREA_PROTOCOL_TYPE_JIUTONG == s_stRoundArea.protoclType)
    {
        if(PUBLIC_CHECKBIT(stRoundAreaAttrib.AreaAttrib,14))
        {
            PUBLIC_CLRBIT(stRoundAreaAttrib.AreaAttrib,14);
            PUBLIC_SETBIT(stRoundAreaAttrib.UserAreaAttrib,E_USER_IN_PLAY);              
            Public_Mymemcpy((unsigned char *)&stRoundAreaAttrib.InVoiceSize,(unsigned char *)&pBuffer[datalen],sizeof(unsigned char),0); //进区提示音长度
            datalen+=sizeof(unsigned char);  
            Public_Mymemcpy((unsigned char *)&stRoundAreaAttrib.InVoiceStr[0],(unsigned char *)&pBuffer[datalen],stRoundAreaAttrib.InVoiceSize,0); //提示音语句
            datalen+=stRoundAreaAttrib.InVoiceSize;    
        }
        ///////////////////////
        if(PUBLIC_CHECKBIT(stRoundAreaAttrib.AreaAttrib,15))
        {
            PUBLIC_CLRBIT(stRoundAreaAttrib.AreaAttrib,15);
            PUBLIC_SETBIT(stRoundAreaAttrib.UserAreaAttrib,E_USER_OUT_PLAY);
            Public_Mymemcpy((unsigned char *)&stRoundAreaAttrib.OutVoiceSize,(unsigned char *)&pBuffer[datalen],sizeof(unsigned char),0); //出区提示音长度
            datalen+=sizeof(unsigned char);  
            Public_Mymemcpy((unsigned char *)&stRoundAreaAttrib.OutVoiceStr[0],(unsigned char *)&pBuffer[datalen],stRoundAreaAttrib.OutVoiceSize,0); //提示音语句
            datalen+=stRoundAreaAttrib.OutVoiceSize;
        }
    }
    //////////////////////
    if(datalen>BufferLen)return 0;
    ///////////////////////////
    stRoundAreaAttrib.Head = HEAD_FLAG;
    Area_MdyDataInFlash((unsigned char*)&stRoundAreaAttrib,addr,STROUND_AREA_ATTRIB_SIZE);
    return datalen;
}
/*********************************************************************
//函数名称	:Area_RoundAreaUpdate(unsigned char *pBuffer, unsigned short BufferLen)
//功能		:设置圆形区域
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:
*********************************************************************/
unsigned char Area_RoundAreaUpdate(unsigned char *pBuffer, unsigned short BufferLen)
{
    unsigned long OffSetAddr;
    unsigned char i,Sum;
    unsigned short datalen,templen;
    unsigned char *pTmp;
    //////////////
    if(BufferLen==0)return 2;     
    Sum=*pBuffer++; 
    if(Sum>MAX_ROUND_AREA_SUM)return 1;
    ////////////////////    
    /////////////////
    Area_ClearRoundArea();
    //////////////////
    datalen=BufferLen-1;
    pTmp=pBuffer;
    //圆形
    Area_RoundAreaParameterInitialize();
    for(i=0; i<Sum; i++)
    {
        OffSetAddr=ROUND_AREA_ADDR_START + i*STROUND_AREA_ATTRIB_SIZE;
        templen=Area_SaveOneRoundArea(OffSetAddr,pTmp,datalen);
        if(0 == templen)break;
        s_stAreaAttrib.CheckRoundNum++;
        s_stAreaAttrib.ValidRoundNum++;
        if(s_stAreaAttrib.CheckRoundNum>=MAX_ROUND_AREA_SUM)break;
        pTmp +=templen;
        if(datalen>templen)datalen -=templen;
        else break;
    }
    return 0;    
}
/*************************************************************
** 函数名称: Area_GetCurRectAreaDatalen
** 功能描述: 获取当前圆形结构体长度
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned short Area_GetCurRoundAreaDatalen(unsigned char *pBuffer, unsigned short BufferLen,unsigned long *AreaID)
{
    if(AREA_PROTOCOL_TYPE_JIUTONG == s_stRectArea.protoclType)
    {
        return Area_GetRoundAreaDatalenJiuTong(pBuffer,BufferLen,AreaID);
    }
    else
    {
       return Area_GetRoundAreaDatalenStandard(pBuffer,BufferLen,AreaID);
    }
}
/*********************************************************************
//函数名称	:Area_RoundAreaAdd(unsigned char *pBuffer, unsigned short BufferLen)
//功能		:设置圆形区域
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:
*********************************************************************/
unsigned char Area_RoundAreaAdd(unsigned char *pBuffer, unsigned short BufferLen)
{
    unsigned long OffSetAddr,AreaID;
    unsigned char i,j,Sum;
    STROUND_AREA_ATTRIB stRoundAreaAttrib;  //读取圆形区域属性
    unsigned short datalen,templen;
    unsigned char *pTmp;
    //////////////
    if(BufferLen==0)return 2;
    ////////////
    Sum=*pBuffer++;
    if(Sum+s_stAreaAttrib.CheckRoundNum>MAX_ROUND_AREA_SUM)return 1;
    pTmp =pBuffer;
    datalen=BufferLen-1;
    //圆形
    for(i=0; i<Sum; i++)//检查ID
    {
        templen=Area_GetCurRoundAreaDatalen(pTmp,datalen,&AreaID);
        if(0 == templen)return 2;//消息有误;
        //////////////////////////
        for(j=0; j<s_stAreaAttrib.CheckRoundNum; j++)
        {
            OffSetAddr=ROUND_AREA_ADDR_START + j*STROUND_AREA_ATTRIB_SIZE;
            sFLASH_ReadBuffer((unsigned char *)&stRoundAreaAttrib,OffSetAddr,AREA_HEAD_SIZE);
            if(AreaID==stRoundAreaAttrib.AreaID)
            {
                return 1;////失败
            }
        }
        ///////////////////////////
        pTmp +=templen;
        if(datalen>templen)datalen -=templen;
        else break;
    }
    //////////////////////////////////////
    pTmp =pBuffer;
    datalen=BufferLen-1;
    for(i=0; i<Sum; i++)
    {
        OffSetAddr=ROUND_AREA_ADDR_START + s_stAreaAttrib.CheckRoundNum *STROUND_AREA_ATTRIB_SIZE;
        templen=Area_SaveOneRoundArea(OffSetAddr,pTmp,datalen);
        if(0 == templen)break;
        ////////////////////
        s_stAreaAttrib.CheckRoundNum++;//
        s_stAreaAttrib.ValidRoundNum++;
        if(s_stAreaAttrib.CheckRoundNum>=MAX_ROUND_AREA_SUM)break;
        ////////////////////
        pTmp +=templen;
        if(datalen>templen)datalen -=templen;
        else break;
    }    
    return 0;    
}
/*********************************************************************
//函数名称	:Area_RoundAreaModify(unsigned char *pBuffer, unsigned short BufferLen)
//功能		:修改圆形区域
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:
*********************************************************************/
unsigned char Area_RoundAreaModify(unsigned char *pBuffer, unsigned short BufferLen)
{
    unsigned long OffSetAddr,AreaID;
    unsigned char i,j,Sum,flag;
    STROUND_AREA_ATTRIB stRoundAreaAttrib;  //读取圆形区域属性
    unsigned short datalen,templen;
    unsigned char *pTmp;
    //////////////
    if(BufferLen==0)return 2;
    ////////////
    Sum=*pBuffer++;
    pTmp =pBuffer;
    datalen=BufferLen-1;
    //圆形
    for(i=0; i<Sum; i++)//检查ID
    {        
        templen=Area_GetCurRoundAreaDatalen(pTmp,datalen,&AreaID);
        if(0 == templen)return 2;//消息有误;
        //////////////////////////
        flag =0;
        for(j=0; j<s_stAreaAttrib.CheckRoundNum; j++)
        {
            OffSetAddr=ROUND_AREA_ADDR_START + j*STROUND_AREA_ATTRIB_SIZE;
            sFLASH_ReadBuffer((unsigned char *)&stRoundAreaAttrib,OffSetAddr,AREA_HEAD_SIZE);
            if(AreaID==stRoundAreaAttrib.AreaID)
            {
                flag=1;//
                break;
            }
        }
        //////////////////
        if(0==flag)return 1;//失败
        ///////////////////////////
        pTmp +=templen;
        if(datalen>templen)datalen -=templen;
        else break;
    }
    //////////////////////////////////////
    pTmp =pBuffer;
    datalen=BufferLen-1;
    for(i=0; i<Sum; i++)
    {
        templen = 0;
        Public_Mymemcpy((unsigned char *)&AreaID,(unsigned char *)&pTmp[0],sizeof(unsigned long),1); //获得区域ID
        for(j=0; j<s_stAreaAttrib.CheckRoundNum; j++)
        {
            OffSetAddr=ROUND_AREA_ADDR_START + j*STROUND_AREA_ATTRIB_SIZE;
            sFLASH_ReadBuffer((unsigned char *)&stRoundAreaAttrib,OffSetAddr,AREA_HEAD_SIZE);
            if(AreaID==stRoundAreaAttrib.AreaID)
            {
                templen=Area_SaveOneRoundArea(OffSetAddr,pTmp,datalen);
                break;
            }
        }
        ///////////////////////
        Area_RoundAreaCheckForResetParam(AreaID);
        /////////////////////
        if(0 == templen)break;
        ////////////////////
        pTmp +=templen;
        if(datalen>templen)datalen -=templen;
        else break;
    }    
    return 0;
}
/*************************************************************
** 函数名称: Area_SetRectAreaCheckProtoclType
** 功能描述: :设置区域时,查检是否有语音标志,仅为兼容九通版本
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Area_SetRoundAreaCheckProtoclType(unsigned char *pBuffer, unsigned short BufferLen)
{
    unsigned long AreaID;
    unsigned char i,Sum;
    unsigned short datalen,templen;
    unsigned char *pTmp;
    //////////////    
    if(BufferLen==0)return AREA_PROTOCOL_TYPE_EMPTY;
    ////////////////////////    
    Sum=*pBuffer++;
    ////////////////////
    datalen=BufferLen-1;
    pTmp=pBuffer;
    ////////////////////
    for(i=0; i<Sum; i++)
    {
        templen=Area_GetRoundAreaDatalenStandard(pTmp,datalen,&AreaID);
        if(0 == templen)//出错
        {
            break;
        }
        pTmp += templen;
        //////////////////////////
        if(datalen == templen)//相等,正确
        {
            return AREA_PROTOCOL_TYPE_STANDARD;
        }
        else
        if(datalen > templen)//大于,还有数据
        {
            datalen -= templen;
        }
        else////小于,出错
        {
            break;
        }
    }
    /////////////////////
    datalen=BufferLen-1;
    pTmp=pBuffer;
    ////////////////////
    for(i=0; i<Sum; i++)
    {
        templen=Area_GetRoundAreaDatalenJiuTong(pTmp,datalen,&AreaID);
        if(0 == templen)//出错
        {
            break;
        }
        pTmp += templen;
        
        if(datalen == templen)//相等,正确
        {
            return AREA_PROTOCOL_TYPE_JIUTONG;//方法二,正确
        }
        else
        if(datalen > templen)//大于,还有数据
        {
            datalen -= templen;
        }
        else////小于,出错
        {
            break;
        }
    }
    return AREA_PROTOCOL_TYPE_EMPTY;
}
/*************************************************************
** 函数名称: Area_SetRoundArea
** 功能描述: :设置圆形区域
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Area_SetRoundArea(unsigned char *pBuffer, unsigned short BufferLen)
{
    unsigned char AckResult,type;
    unsigned char protoclType;
    if(BufferLen==0)return 2;
    /////////////////
    type=*pBuffer++;
    ///////////////////
    protoclType=Area_SetRoundAreaCheckProtoclType(pBuffer,BufferLen-1);
    if(AREA_PROTOCOL_TYPE_EMPTY==protoclType)return 2;
    s_stRoundArea.protoclType = protoclType;
    //////////////
    AckResult = 0;
    if(0== type)//更新
    {
        AckResult=Area_RoundAreaUpdate(pBuffer,BufferLen-1);
    }
    else
    if(1 == type)//添加
    {   
        AckResult=Area_RoundAreaAdd(pBuffer,BufferLen-1);
    }
    else
    if(2 == type)//修改
    {  
        AckResult=Area_RoundAreaModify(pBuffer,BufferLen-1);
    }
    if(0==AckResult)
    {
        s_stAreaAttrib.preSpeed = 0xff;
    }
    return AckResult;
}
/*************************************************************
** 函数名称: Area_DelRoundArea
** 功能描述: :删除圆形区域
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Area_DelRoundArea(unsigned char *pBuffer, unsigned short BufferLen)
{
    unsigned char i,j,Sum,flag;
    unsigned long OffSetAddr,AreaID;
    STROUND_AREA_ATTRIB stRoundArea;  //读取圆形区域属性  
    unsigned char *pTmp;
    if(BufferLen==0)return 2;
    Sum=*pBuffer++;
    pTmp =pBuffer;
    if(Sum)
    {
        for(i=0; i<Sum; i++)
        {
            Public_Mymemcpy((unsigned char *)&AreaID,(unsigned char *)&pTmp[0],sizeof(unsigned long),1); //获得区域ID
            pTmp+=sizeof(unsigned long);
            flag =0;
            for(j=0; j<s_stAreaAttrib.CheckRoundNum; j++)
            {
                OffSetAddr=ROUND_AREA_ADDR_START + j*STROUND_AREA_ATTRIB_SIZE;
                sFLASH_ReadBuffer((unsigned char *)&stRoundArea,OffSetAddr,AREA_HEAD_SIZE);
                if(AreaID==stRoundArea.AreaID)
                {
                    flag =1;
                    break;
                }
            }
            if(0 == flag)return 2;
        }        
        ///////////////////////
        pTmp =pBuffer;
        for(i=0; i<Sum; i++)
        {
            Public_Mymemcpy((unsigned char *)&AreaID,(unsigned char *)&pTmp[0],sizeof(unsigned long),1); //获得区域ID
            pTmp+=sizeof(unsigned long);
            for(j=0; j<s_stAreaAttrib.CheckRoundNum; j++)
            {
                OffSetAddr=ROUND_AREA_ADDR_START + j*STROUND_AREA_ATTRIB_SIZE;
                sFLASH_ReadBuffer((unsigned char *)&stRoundArea,OffSetAddr,AREA_HEAD_SIZE);
                if(AreaID==stRoundArea.AreaID&&AreaID)
                {//需要修改该项目
                    stRoundArea.AreaID = 0;  //删除区域,只需把ID=0  
                    s_stAreaAttrib.ValidRoundNum--;
                    OffSetAddr=ROUND_AREA_ADDR_START + j*STROUND_AREA_ATTRIB_SIZE;
                    stRoundArea.Head = HEAD_FLAG;
                    Area_MdyDataInFlash((unsigned char*)&stRoundArea,OffSetAddr,AREA_HEAD_SIZE); 
                }
            }
            //////////////////////////////////////
            Area_RoundAreaCheckForResetParam(AreaID);
            ////////////////////////////////////////
        }
    }
    else
    { //删除所有区域
        Area_ClearRoundArea();
    }
    
    return 0;
}
/*********************************************************************
//函数名称	:Area_RoundAreaModify(unsigned char *pBuffer, unsigned short BufferLen)
//功能		:修改圆形区域
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:
*********************************************************************/
unsigned char Area_RoundAreaModifyUserData(unsigned char *pBuffer, unsigned short BufferLen)
{
    unsigned long OffSetAddr,AreaID;
    unsigned char i,j,Sum,flag;
    STROUND_AREA_ATTRIB stRoundAreaAttrib;  //读取圆形区域属性
    unsigned short datalen,templen;
    unsigned char *pTmp;
    //////////////
    if(BufferLen==0)return 2;
    ////////////
    Sum=*pBuffer++;
    pTmp =pBuffer;
    datalen=BufferLen-1;
    //圆形
    for(i=0; i<Sum; i++)//检查ID
    {        
        templen=Area_GetRoundAreaUserDatalen(pTmp,datalen,&AreaID);
        if(0 == templen)return 2;//消息有误;
        //////////////////////////
        flag =0;
        for(j=0; j<s_stAreaAttrib.CheckRoundNum; j++)
        {
            OffSetAddr=ROUND_AREA_ADDR_START + j*STROUND_AREA_ATTRIB_SIZE;
            sFLASH_ReadBuffer((unsigned char *)&stRoundAreaAttrib,OffSetAddr,AREA_HEAD_SIZE);
            if(AreaID==stRoundAreaAttrib.AreaID)
            {
                flag=1;//
                break;
            }
        }
        //////////////////
        if(0==flag)return 1;//失败
        ///////////////////////////
        pTmp +=templen;
        if(datalen>templen)datalen -=templen;
        else break;
    }
    //////////////////////////////////////
    pTmp =pBuffer;
    datalen=BufferLen-1;
    for(i=0; i<Sum; i++)
    {
        templen = 0;
        Public_Mymemcpy((unsigned char *)&AreaID,(unsigned char *)&pTmp[0],sizeof(unsigned long),1); //获得区域ID
        for(j=0; j<s_stAreaAttrib.CheckRoundNum; j++)
        {
            OffSetAddr=ROUND_AREA_ADDR_START + j*STROUND_AREA_ATTRIB_SIZE;
            sFLASH_ReadBuffer((unsigned char *)&stRoundAreaAttrib,OffSetAddr,AREA_HEAD_SIZE);
            if(AreaID==stRoundAreaAttrib.AreaID)
            {
                templen=Area_SaveOneRoundModifyUserData(OffSetAddr,pTmp,datalen);
                break;
            }
        }
        /////////////////////
        if(0 == templen)break;
        ////////////////////
        pTmp +=templen;
        if(datalen>templen)datalen -=templen;
        else break;
    }    
    return 0;
}
/*********************************************************************
//函数名称	:Area_RectAreaModify(unsigned char *pBuffer, unsigned short BufferLen)
//功能		:修改矩形区域
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:
*********************************************************************/
unsigned char Area_RectAreaModifyUserData(unsigned char *pBuffer, unsigned short BufferLen)
{
    unsigned long OffSetAddr,AreaID;
    unsigned char i,j,Sum,flag;
    STRECT_AREA_ATTRIB stRectAreaAttrib;  //读取矩形区域属性
    unsigned short datalen,templen;
    unsigned char *pTmp;
    //////////////
    if(BufferLen==0)return 2;
    ////////////
    Sum=*pBuffer++;
    pTmp =pBuffer;
    datalen=BufferLen-1;
    //矩形
    for(i=0; i<Sum; i++)//检查ID
    {        
        templen=Area_GetRectAreaUserDatalen(pTmp,datalen,&AreaID);
        if(0 == templen)return 2;//消息有误;
        //////////////////////////
        flag =0;
        for(j=0; j<s_stAreaAttrib.CheckRectNum; j++)
        {
            OffSetAddr=RECT_AREA_ADDR_START + j*STRECT_AREA_ATTRIB_SIZE;
            sFLASH_ReadBuffer((unsigned char *)&stRectAreaAttrib,OffSetAddr,AREA_HEAD_SIZE);
            if(AreaID==stRectAreaAttrib.AreaID)
            {
                flag=1;//
                break;
            }
        }
        //////////////////
        if(0==flag)return 1;//失败;
        ///////////////////////////
        pTmp +=templen;
        if(datalen>templen)datalen -=templen;
        else break;
    }
    //////////////////////////////////////
    pTmp =pBuffer;
    datalen=BufferLen-1;
    for(i=0; i<Sum; i++)
    {
        templen = 0;
        Public_Mymemcpy((unsigned char *)&AreaID,(unsigned char *)&pTmp[0],sizeof(unsigned long),1); //获得区域ID
        for(j=0; j<s_stAreaAttrib.CheckRectNum; j++)
        {
            OffSetAddr=RECT_AREA_ADDR_START + j*STRECT_AREA_ATTRIB_SIZE;
            sFLASH_ReadBuffer((unsigned char *)&stRectAreaAttrib,OffSetAddr,AREA_HEAD_SIZE);
            if(AreaID==stRectAreaAttrib.AreaID)
            {
                templen=Area_SaveOneRectModifyUserData(OffSetAddr,pTmp,datalen);
                break;
            }
        }
        /////////////////
        if(0 == templen)break;
        ////////////////////
        pTmp +=templen;
        if(datalen>templen)datalen -=templen;
        else break;
    }    
    return 0;    
}
/*************************************************************
** 函数名称: Area_DisposeEIExpandProtocol
** 功能描述: 处理区域伊爱扩展协议
** 入口参数: pBuffer:消息体内容首地址,BufferLen消息体内容长度
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Area_DisposeEIExpandProtocol(unsigned char *pBuffer, unsigned short BufferLen)
{
    unsigned char type;
    unsigned char *p;
    if(BufferLen<3)return 0;
    p = pBuffer;
    type = *p++;//区域类型
    //保留字节
    p++;
    switch(type)
    {
        case E_ROUND_AREA:
            Area_RoundAreaModifyUserData(p,BufferLen-2);
            break;
        case E_RECT_AREA:
            Area_RectAreaModifyUserData(p,BufferLen-2);
            break;
        case E_POLYGON_AREA:
            #ifdef __POLYGON__H_
            Polygon_AreaModifyUserData(p,BufferLen-2);
            #endif
            break;
    }
    return 1;
}
/*************************************************************
** 函数名称: Area_DisposeReadAreaProtocol
** 功能描述: 处理索美读区域协议
** 入口参数: pBuffer:消息体内容首地址,BufferLen消息体内容长度
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Area_DisposeReadAreaProtocol(unsigned char *pBuffer, unsigned short BufferLen)
{
    //unsigned char *pTmp;
    //pTmp=pBuffer;
    //if(BufferLen !=4)
    return 0;
}
/*************************************************************
** 函数名称: Area_GetRoundAreaTotalNum
** 功能描述: 获得圆形区域总数
** 入口参数: 
** 出口参数: 
** 返回参数: 圆形区域总数
*************************************************************/ 
unsigned char Area_GetRoundAreaTotalNum(void)
{

    return s_stAreaAttrib.ValidRoundNum;
}
/*************************************************************
** 函数名称: Area_GetRectAreaTotalNum
** 功能描述: 获得矩形区域总数
** 入口参数: 
** 出口参数: 
** 返回参数: 矩形区域总数
*************************************************************/ 
unsigned char Area_GetRectAreaTotalNum(void)
{

    return s_stAreaAttrib.ValidRectNum;
}
/*************************************************************
** 函数名称: AreaManage_ScanTask
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Area_CheckScanTask(void)
{
    unsigned char speed;
    //读进出区域报警屏蔽位状态
    if(Io_ReadAlarmMaskBit(ALARM_BIT_IN_OUT_AREA))
    {
        if(1==s_stAreaAttrib.nonavigationCnt)
        {
            s_stAreaAttrib.nonavigationCnt = 0;
            Area_ClearAllAlarm(E_RECT_AREA);
            Area_ClearAllAlarm(E_ROUND_AREA);
        }
        return;
    }
    Area_GetCurGpsCoordinate();//1s获取一次经纬度        
    s_stAreaAttrib.nonavigationCnt =1;    
    s_stRoundArea.preAreaCnt=s_stRoundArea.AreaCnt;
    s_stRectArea.preAreaCnt=s_stRectArea.AreaCnt;
    speed = Area_GetCurSpeed();
    ///////////////////////////
    if(0==speed&&0==s_stAreaAttrib.preSpeed)return;//速度为零,不处理
    /////////////////////////
    s_stAreaAttrib.preSpeed = speed;   
    if(s_stAreaAttrib.CheckRectNum)
    Area_ScanRectArea();
    if(s_stAreaAttrib.CheckRoundNum)
    LZM_PublicSetOnceTimer(&s_stAREATimer[AREA_TIMER_SCAN_ROUND],PUBLICSECS(0.1),Area_ScanRoundArea);
}
/*************************************************************
** 函数名称: AreaManage_ScanTask
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void AreaManage_ScanTask(void)
{
    if(Io_ReadStatusBit(STATUS_BIT_NAVIGATION))
    {
        GPS_STRUCT stTmpGps;
        Gps_CopygPosition(&stTmpGps);
        if(stTmpGps.Second!=s_stAreaAttrib.preSecond)
        {
            s_stAreaAttrib.preSecond = stTmpGps.Second; 
            LZM_PublicKillTimer(&s_stAREATimer[AREA_TIMER_SCAN_RECT]);
            Area_CheckScanTask();
        }
    }
    else
    {  
        s_stAreaAttrib.preSecond = 0xff;
        s_stAreaAttrib.preSpeed = 0xff;
        if(s_stAreaAttrib.nonavigationCnt<50)s_stAreaAttrib.nonavigationCnt++;//10s
        else
        if(s_stAreaAttrib.nonavigationCnt == 50)
        {
            s_stAreaAttrib.nonavigationCnt =100;
            LZM_PublicKillTimer(&s_stAREATimer[AREA_TIMER_SCAN_RECT]);
            LZM_PublicKillTimer(&s_stAREATimer[AREA_TIMER_SCAN_ROUND]);
            Area_ClearAllAlarm(E_RECT_AREA);
            Area_ClearAllAlarm(E_ROUND_AREA);
        }
    }
}

/*************************************************************
** 函数名称: AreaManage_ScanTask
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void AreaManage_StartScanTask(void)
{
    LZM_PublicSetCycTimer(&s_stAREATimer[AREA_TIMER_TASK],PUBLICSECS(0.2),AreaManage_ScanTask);
}
/*************************************************************
** 函数名称: AreaManage_TimeTask
** 功能描述: 区域功能定时器处理函数
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
FunctionalState AreaManage_TimeTask(void)
{
    LZM_PublicTimerHandler(s_stAREATimer,AREA_TIMERS_MAX);
    return ENABLE;
}
/*************************************************************
** 函数名称: Area_GetInOutAreaAlarmSubjoinInfo
** 功能描述: 获得进出区域报警附加信息包(包括附加信息ID、长度、附加信息体) :即附加信息ID为0x12的所有内容
** 入口参数: 需要保存进出区域报警附加信息包的首地址
** 出口参数: 
** 返回参数: 进出区域报警附加信息包的总长度,0:表示未进出区域
*************************************************************/
unsigned char Area_GetInOutAreaAlarmSubjoinInfo(unsigned char *data) 
{
    if(!Io_ReadAlarmBit(ALARM_BIT_IN_OUT_AREA))return 0;
    unsigned char len; 
    unsigned char buffer[20]={0};
    len =0;
    buffer[len++]= IN_OUT_AREA_OVERHEAD_INFO_ID;
    buffer[len++]= IN_OUT_AREA_OVERHEAD_INFO_LEN;    
    buffer[len++]= s_stAreaAttrib.INOUTPostType;
    Public_ConvertLongToBuffer(s_stAreaAttrib.InOutAreaID,&buffer[len]);
    len += 4;
    buffer[len++]= s_stAreaAttrib.Orientation;
    memcpy(data,buffer,len);
    return len;
}
/*************************************************************
** 函数名称: Area_ReadAndCheckParam
** 功能描述: 区域参数初始化
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Area_ReadAndCheckParam(void) 
{
    unsigned long OffSetAddr;
    unsigned short i;
    
    STRECT_AREA_ATTRIB stRectArea;	  //读取矩形区域属性
    STROUND_AREA_ATTRIB stRoundArea;  //读取圆形区域属性
   
    s_stAreaAttrib.preSecond        = 0xff;
    s_stAreaAttrib.nonavigationCnt  = 0xff;
    s_stAreaAttrib.InOutAreaAlarm   = 0;
    s_stAreaAttrib.CloseGPRSFlag    = 0;
    s_stAreaAttrib.CollectGnssFlag  = 0;
    s_stAreaAttrib.InAreaFlag       = 0;
    s_stAreaAttrib.OpenDoorAlarmFlag= 0;
    ////////////
    //矩形初始化    
    Area_RectAreaParameterInitialize();
    ////////////////////////
    for(i=0; i<MAX_RECT_AREA_SUM; i++)
    {
        OffSetAddr=RECT_AREA_ADDR_START + i*STRECT_AREA_ATTRIB_SIZE ;  
        sFLASH_ReadBuffer((unsigned char *)&stRectArea,OffSetAddr,AREA_HEAD_SIZE);//读出区域数据	
        if(HEAD_FLAG == stRectArea.Head)
        {
            if(0xFFFFFFFF == stRectArea.AreaID)
            {
                Area_ClearRectArea();
                break;
            }
            else
            {
                s_stAreaAttrib.CheckRectNum++;
                if(stRectArea.AreaID)
                {
                    s_stAreaAttrib.ValidRectNum++;
                }
            }
        }
        else 
        if(stRectArea.Head != 0xFFFFFFFF)
        {
            Area_ClearRectArea();
            break;
        }
        else 
        if(0xFFFFFFFF == stRectArea.Head)
        {
            break;
        }
    }    
    //圆形初始化
    Area_RoundAreaParameterInitialize();
    for(i=0; i<MAX_ROUND_AREA_SUM; i++)
    {
        OffSetAddr=ROUND_AREA_ADDR_START + i*STROUND_AREA_ATTRIB_SIZE ;  
        sFLASH_ReadBuffer((unsigned char *)&stRoundArea,OffSetAddr,AREA_HEAD_SIZE);//读出区域数据	
        if(HEAD_FLAG == stRoundArea.Head)
        {
            if(0xFFFFFFFF == stRoundArea.AreaID)
            {
                Area_ClearRoundArea();
                break;
            }
            else
            {
                s_stAreaAttrib.CheckRoundNum++;
                if(stRoundArea.AreaID)
                {
                    s_stAreaAttrib.ValidRoundNum++;
                }
            }
        }
        else 
        if(0xFFFFFFFF != stRoundArea.Head)
        {
            Area_ClearRoundArea();
            break;
        }
        else 
        if(0xFFFFFFFF == stRoundArea.Head)
        {
            break;
        }
    }
    //////////////////////////////  
    LZM_PublicSetOnceTimer(&s_stAREATimer[AREA_TIMER_TASK],PUBLICSECS(15),AreaManage_StartScanTask);    
}
/*************************************************************
** 函数名称: Area_ParameterInitialize
** 功能描述: 区域参数初始化
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Area_ParameterInitialize(void) 
{
    LZM_PublicKillTimerAll(s_stAREATimer,AREA_TIMERS_MAX); 
    //////////////////////////////
    SetTimerTask(TIME_AREA,SYSTICK_0p1SECOND);
    //////////////////////////////////////////////////////////
    LZM_PublicSetOnceTimer(&s_stAREATimer[AREA_TIMER_TASK],PUBLICSECS(15),Area_ReadAndCheckParam);    
}
/*******************************************************************************
 *                             end of module
 *******************************************************************************/


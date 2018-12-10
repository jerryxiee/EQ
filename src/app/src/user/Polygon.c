/*******************************************************************************
 * File Name:			PolygonManager.c 
 * Function Describe:	
 * Relate Module:		区域功能
 * Writer:				mark
 * Date:				2013-10-10
 ******* 添加开门报警功能、关闭通讯模块功能、采集GNSS详细定位数据功能；
 ******* 修改进出区域的语音提示属性位置:由原来的位14、15修改为:12、13
 *******************************************************************************/
#include"include.h"
/******************************************************************************
*区域状态的枚举类型定义
******************************************************************************/

/*****************************************************************************
*相关参数的预定义
*****************************************************************************/
#define POLYGON_MAX_AREA_ADDR                       0xFFFFFFFF
#define POLYGON_AREA_NUM_MAX                        20//20 at the most
#define ONE_POLYGON_VERTEX_NUM_MAX                  200//(256=12ms)
#define ONE_BLOCK_READ_VERTEX_NUM                   64//单次读取的顶点数
#define ONE_BLOCK_READ_VERTEX_DATALEN               (ONE_BLOCK_READ_VERTEX_NUM<<3)//单次读取的数据长度
///////////////////////////////////
#define  POLYGON_SAVE_FLAG              0xDCBAABCD
#define  POLYGON_HEAD_ID_SIZE           8//Define the size of head flag for polygon area
#define  SIZE_STPOLYGON_PARAM           sizeof(STPOLYGON_PARAM)//Define the size of area attrib for polygon area
#define  ONE_POLYGON_AREA_SIZE          (SIZE_STPOLYGON_PARAM+(ONE_POLYGON_VERTEX_NUM_MAX<<3))
//////////////////////
#define  VOICE_BYTE       50                         //Define the value for voice_byte as 50
////////////////////////////
#ifndef    FLASH_ONE_SECTOR_BYTES
#define    FLASH_ONE_SECTOR_BYTES                      ONE_SECTOR_BYTES
#endif

#ifndef    FLASH_POLYGON_AREA_START_SECTOR
#define    FLASH_POLYGON_AREA_START_SECTOR             POLYGON_AREA_START_SECTOR                                 //Define the start flash sector for polygon area
#endif

#ifndef    FLASH_POLYGON_AREA_END_SECTOR
#define    FLASH_POLYGON_AREA_END_SECTOR               POLYGON_AREA_END_SECTOR                                  //Define  the end  flash  sector for polygon area 
#endif

#define    POLYGON_AREA_ADDR_START    (unsigned long)(FLASH_POLYGON_AREA_START_SECTOR*FLASH_ONE_SECTOR_BYTES)    //Define the start address for polygon area
#define    POLYGON_AREA_ADDR_END      (unsigned long)(FLASH_POLYGON_AREA_END_SECTOR*FLASH_ONE_SECTOR_BYTES)      //Define the end   address for polygon area

/*****************************************************************************
*区域报警提示的枚举定义
******************************************************************************/
typedef enum
{
    ALARM_ITEM_IN_AREA,                 //Define one alarm as in polygon area
    ALARM_ITEM_OUT_AREA                 //Define one alarm as out polygon area
}E_POLYGON_ALARM_ITEM;
/*****************************************************************************
*多边形区域属性结构
******************************************************************************/
typedef struct //160字节
{
    unsigned long   Head;                   //保存标志
    unsigned long   areaID;                 //区域ID
    unsigned short  areaAttrib;             //区域属性
    unsigned short  vertexNum;              //多边形顶点数
    unsigned char   startMoment[6];         //开始时刻
    unsigned char   endMoment[6];           //结束时刻    
    unsigned short  limitspeed;             //限速值
    unsigned short  userAreaAttrib;         //自定义区域属性
    unsigned char   speedDuration;          //持续时间
    unsigned char   InVoiceSize;            //进区域语音大小
    unsigned char   OutVoiceSize;           //出区域语音大小
    unsigned char   reserve[13];
    T_GPS_POS       minpoint;               //最小点经纬度
    T_GPS_POS       maxpoint;               //最大点经纬度
    unsigned char   InVoiceStr[VOICE_BYTE]; //Define an array of invoice
    unsigned char   OutVoiceStr[VOICE_BYTE];//Define an array of outvoice
}STPOLYGON_PARAM;
///////////////////
/*****************************************************************************
*区域控制块定义
******************************************************************************/
typedef struct
{
    T_GPS_POS      minpoint;            //最小点经纬度
    T_GPS_POS      maxpoint;            //最大点经纬度
    T_GPS_POS      endpoint;            //终点经纬度
    T_GPS_POS      curpoint;            //当前点经纬度
    unsigned long  addr;                //Define the address for area ,no any area is in as the address is max_area_num
    unsigned long  vertexAddr;          //Define the address for area ,no any area is in as the address is max_area_num
    unsigned long  startTime;           //Define  starttime
    unsigned long  endTime;               //Define  endtime
    unsigned long  areaID;                //Define  the area id for a defined area    
    unsigned short areaAttrib;            //Define  the attrib for a defined area
    unsigned short vertexNum;              //Define the number of vertexs for polygon area
    unsigned short userAreaAttrib;        //Define  the user attrib for a defined area
    unsigned short limitSpeed;            //Define  the limit speed value for a defined area
    unsigned char  speedDuration;         //Define  an duration for overspeed    
    unsigned char  checkAreaNum;          //Define  the number of polygon area
    unsigned char  validAreaNum;          //Define  the number of polygon area    
    unsigned char  preSecond;             //Define  the second value of current moment
    unsigned char  noNavigationCnt;       //Define  a counter as no navigation
    unsigned char  areaCnt;               //Define  an area counter
    unsigned char  preAreaCnt;            //Define  a current area counter
    unsigned char  areaFlag;              //Define  the flag for a defined area
    unsigned char  timeType;              //Define  the time type
    unsigned char  preSpeed;
    unsigned char  protoclType;
    unsigned char  voiceStr[VOICE_BYTE];  //Define  an array for prompt voice    
}STPOLYGON_ATTRIB;//
////////////////////////////////////
static STPOLYGON_ATTRIB s_stPolygonAttrib;//Define  a STPOLYGON_ATTRIB type area
///////////////////////
enum
{
    POL_TIMER_TASK,
    POL_TIMER_SCAN,
    POL_TIMER_TEST,
    POL_TIMERS_MAX
}STPOLYGONAREATIME;                                  //Define an enum type variable  stareatime
//////////////////////////
static  LZM_TIMER s_stPOLTimer[POL_TIMERS_MAX]; //Define a LZM_TIMER type array s_stPOLTimer
////////////////////////////
extern void Area_ClearAllAlarm(unsigned char type);
extern void Area_SetCollectGnssFlag(unsigned char type);
extern void Area_SetCloseGPRSFlag(unsigned char type);
extern void Area_SetInAreaFlag(unsigned char type);
/*****************************************************************************
*函数定义
******************************************************************************/
/*************************************************************
** 函数名称: Area_RoundAreaResetParam
** 功能描述: 圆形区域重置参数
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Polygon_ResetParam(void) 
{
    s_stPolygonAttrib.preSpeed = 0xff;
    s_stPolygonAttrib.areaCnt   = 0;
    s_stPolygonAttrib.addr      = POLYGON_MAX_AREA_ADDR;
    s_stPolygonAttrib.areaFlag  = AREA_STATUS_MAX;
    ////////////////////////////
    Area_ClearAllAlarm(E_POLYGON_AREA);
}
/*****************************************************************************
*Function Name:  Polygon_AreaParameterInit()
*Input  Parameter:  None
*Output Parameter:  None
*Function Describtion:Initialize the polygon area parameter 
*Writer:mark
*Update Data:2013.10.11
******************************************************************************/
void Polygon_AreaParameterInit(void) 
{
    Polygon_ResetParam();
    s_stPolygonAttrib.checkAreaNum = 0;
    s_stPolygonAttrib.validAreaNum = 0;
}
/*************************************************************
** 函数名称: Polygon_CheckForResetParam
** 功能描述: 检查是否重置参数
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Polygon_CheckForResetParam(unsigned long AreaID) 
{
    if(s_stPolygonAttrib.areaFlag==AREA_STATUS_IN&&s_stPolygonAttrib.areaID==AreaID)
    {
        Polygon_ResetParam();
    }
}
/*****************************************************************************
*Function Name: Polygon_ClearArea()
*Input  Parameter:  None
*Output Parameter:  None
*Function Describtion:clear Polygon area flash sector  
*Writer:mark
*Update Data:2013.10.11
******************************************************************************/
void Polygon_ClearArea(void)
{
    Polygon_AreaParameterInit();
    Area_EraseFlash(FLASH_POLYGON_AREA_START_SECTOR,FLASH_POLYGON_AREA_END_SECTOR); //erase flash
}
/*****************************************************************************
*Function Name: Polygon_ComparePos(T_GPS_POS *Polygon,unsigned char VertexSum,T_GPS_POS *s_stPolygonAttrib.curpoint)
*Input  Parameter:  polygon,vertexsum,current position
*Output Parameter:  
*Function Describtion:Check the relation between current position and polygon area  
*Writer:mark
*Update Data:2013.10.11
******************************************************************************/
void Polygon_ReadEndPointData(void)
{
    unsigned long addr;
    addr = ((s_stPolygonAttrib.vertexNum-1)<<3);
    addr += s_stPolygonAttrib.vertexAddr;
    sFLASH_ReadBuffer((unsigned char *)&s_stPolygonAttrib.endpoint,addr,8);
}
/*****************************************************************************
*Function Name: Polygon_GetCurGpsCoordinate()
*Input  Parameter:  None
*Output Parameter:  None
*Function Describtion:Get GPS Coordinate of current location  
*Writer:mark
*Update Data:2013.10.11
******************************************************************************/
void Polygon_GetCurGpsCoordinate(void)
{  
    unsigned char buffer[28];
    Public_GetCurBasicPositionInfo(buffer);
    Public_Mymemcpy((unsigned char *)&s_stPolygonAttrib.curpoint.Lat,(unsigned char *)&buffer[8],4,1); 
    Public_Mymemcpy((unsigned char *)&s_stPolygonAttrib.curpoint.Long,(unsigned char *)&buffer[12],4,1);    
}
/*****************************************************************************
*Function Name: Polygon_CheckIsInMinRectArea
*Input  Parameter: polygon area
*Output Parameter:  
*Function Describtion:
*Writer:mark
*Update Data:2013.10.11
******************************************************************************/
unsigned char Polygon_CheckIsInMinRectArea(void)  
{
   if((s_stPolygonAttrib.curpoint.Lat < s_stPolygonAttrib.minpoint.Lat)
      ||(s_stPolygonAttrib.curpoint.Lat > s_stPolygonAttrib.maxpoint.Lat)
      ||(s_stPolygonAttrib.curpoint.Long < s_stPolygonAttrib.minpoint.Long)
      ||(s_stPolygonAttrib.curpoint.Long > s_stPolygonAttrib.maxpoint.Long))
        return 1;
   return 0;
}
/*****************************************************************************
*Function Name: Polygon_CalTrigonOrientation
*Input  Parameter: 
*Output Parameter:  
*Function Describtion:
*Writer:mark
*Update Data:2013.10.11
******************************************************************************/
unsigned char Polygon_CalTrigonOrientation(T_GPS_POS *curpoint,T_GPS_POS *point1,T_GPS_POS *point2)
{
    double dVal;
    double temp1,temp2,temp3;
    if(point2->Long==point1->Long)
    {
        dVal = point1->Lat;
    }
    else
    {
        temp1 = (double)curpoint->Long  - (double)point1->Long;
        temp2 = (double)point2->Lat     - (double)point1->Lat;
        temp3 = (double)point2->Long    - (double)point1->Long;
        dVal  = (temp1*temp2)/temp3;
        dVal  = point1->Lat + dVal;
    }
    //////////////////
    if(dVal == curpoint->Lat)//在边上
    {
        return 0xff;
    }
    else
    if(dVal < curpoint->Lat)
    {
        return 1;
    }
    else
    {
        return 0;
    }
    //////////////////
    //return (dVal<curpoint->Lat)?1:0;
}
/*************************************************************
** 函数名称: Polygon_ComparePosBlock
** 功能描述: 分块判断多边形
** 入口参数: addr
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Polygon_ComparePosBlock(unsigned long addr,unsigned char pointnum,unsigned char oddNodes,T_GPS_POS *point2)
{
    T_GPS_POS vertex[ONE_BLOCK_READ_VERTEX_NUM];
    T_GPS_POS *curpoint,*point1;
    unsigned short datalen;
    unsigned char i,result;
    curpoint = &s_stPolygonAttrib.curpoint;
    datalen  = (pointnum<<3);
    sFLASH_ReadBuffer((unsigned char *)vertex,addr,datalen);
    for(i=0; i< pointnum; i++)
    {
        point1 =&vertex[i];
        if((point1->Lat==curpoint->Lat)&&(point1->Long==curpoint->Long))//之前的算法处理边界时不准
        {
            return 0xff;//为其中的一个顶点
        }
        else
        if(point1->Long==curpoint->Long&&point2->Long==curpoint->Long)//经度相同
        {
            if((point1->Lat<curpoint->Lat&&curpoint->Lat<point2->Lat)
             ||(point1->Lat>curpoint->Lat&&curpoint->Lat>point2->Lat))
            {
                return 0xff;//为其中一条边
            }
        }
        else
        if((point1->Long< curpoint->Long && point2->Long>=curpoint->Long)
         ||(point1->Long>=curpoint->Long && point2->Long< curpoint->Long))
        {
            result = Polygon_CalTrigonOrientation(curpoint,point1,point2);
            if(0xff==result)return 0xff;//经度不同,为其中一条边
            oddNodes ^= result;
        }
        ///////////////////////////
        point2 = point1;
    }
    return oddNodes;
}
/*************************************************************
** 函数名称: Polygon_ComparePos
** 功能描述: 是否在多边形内
** 入口参数: 
** 出口参数: 
** 返回参数: 0:不在,1:在
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Polygon_ComparePos(void)
{
    T_GPS_POS point;
    unsigned long addr; 
    unsigned char oddNodes = 0;
    unsigned char i,block,pointnum;    
    ///////////////////////////////////    
    point   = s_stPolygonAttrib.endpoint;
    addr    = s_stPolygonAttrib.vertexAddr;
    /////////////////////    
    block   = s_stPolygonAttrib.vertexNum/ONE_BLOCK_READ_VERTEX_NUM;
    ///////////////////////
    for(i=0; i<block; i++)
    {
        oddNodes = Polygon_ComparePosBlock(addr,ONE_BLOCK_READ_VERTEX_NUM,oddNodes,&point);
        if(0xff==oddNodes)return 1;//为其中的一个顶点
        addr += ONE_BLOCK_READ_VERTEX_DATALEN;
    }
    //////////////////////
    pointnum = s_stPolygonAttrib.vertexNum%ONE_BLOCK_READ_VERTEX_NUM;    
    if(pointnum)
    {
        oddNodes = Polygon_ComparePosBlock(addr,pointnum,oddNodes,&point);
    }
    return oddNodes;
}

/*************************************************************
** 函数名称: Polygon_IsInArea
** 功能描述: 是否在多边形内
** 入口参数: 
** 出口参数: 
** 返回参数: 0:不在,1:在
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Polygon_IsInArea(void)  
{
    if(Polygon_CheckIsInMinRectArea())return 0;
    return Polygon_ComparePos();
}
/*************************************************************
** 函数名称: Polygon_CheckTime
** 功能描述: 检测线路是否符合时间范围
** 入口参数: 
** 出口参数: 
** 返回参数: 0:不符合时间范围,1:符合时间范围
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Polygon_CheckTime(void)
{
    u32 curTime = 0;
    //是否需要检查时间
    if(!PUBLIC_CHECKBIT(s_stPolygonAttrib.areaAttrib,E_AREA_MOMENT))return 1; //不需要检查时间                  
    curTime=Area_GetConvertCurTime(s_stPolygonAttrib.timeType); //当前时间  
    return Public_CheckTimeIsInRange(curTime,s_stPolygonAttrib.startTime,s_stPolygonAttrib.endTime);    
}
/*****************************************************************************
*Function Name: Polygon_DisposeDriveOutArea(void)
*Input  Parameter: None
*Output Parameter: None
*Function Describtion:Dispose the status as drive out of polygon area  
*Writer:mark
*Update Data:2013.10.11
******************************************************************************/
void Polygon_ClearInOutAlarm(void)                //Clear in/out polygon area alarm
{
    Area_ClearInOutAreaAlarmBit(E_POLYGON_AREA);
}
/*****************************************************************************
*Function Name: Polygon_DisposeDriveOutArea(void)
*Input  Parameter: None
*Output Parameter: None
*Function Describtion:Dispose the status as drive out of polygon area  
*Writer:mark
*Update Data:2013.10.11
******************************************************************************/
void Polygon_DisposeDriveOutArea(void) 
{
    Area_ClearAllAlarm(E_POLYGON_AREA);
    if(PUBLIC_CHECKBIT(s_stPolygonAttrib.areaAttrib,E_OUT_SENDTO_DRIVER)) 
    {
        Public_PlayTTSVoiceStr(s_stPolygonAttrib.voiceStr);
    }    
    /////////////////////////////
    if(PUBLIC_CHECKBIT(s_stPolygonAttrib.areaAttrib,E_OUT_SENDTO_SERVER)) 
    {
        Area_SetInOutAreaAlarmInfo(E_POLYGON_AREA, s_stPolygonAttrib.areaID,AREA_STATUS_OUT);
    }
    //////////////////////////
    if(PUBLIC_CHECKBIT(s_stPolygonAttrib.userAreaAttrib,E_USER_NO_CHANGE_LIMIT_SPEED))//出区域不更改限速值
    { 
        if(PUBLIC_CHECKBIT(s_stPolygonAttrib.areaAttrib,E_AREA_LIMIT_SPEED))//
        {
            SpeedMonitor_SetSpeedParamValid(E_SPEED_TYPE_GENERAL,0,s_stPolygonAttrib.limitSpeed,s_stPolygonAttrib.speedDuration);
        }
        ////////////////////////////////
    }
}
/*****************************************************************************
*Function Name: Area_DisposeDriveInRectArea
*Input  Parameter: None
*Output Parameter: None
*Function Describtion:Dispose the status as drive in the polygon area  
*Writer:mark
*Update Data:2013.10.11
******************************************************************************/
void Polygon_DisposeDriveInArea(void) 
{
    Area_ClearAllAlarm(E_POLYGON_AREA);
    ///////////////////////
    if(PUBLIC_CHECKBIT(s_stPolygonAttrib.areaAttrib,E_AREA_LIMIT_SPEED))
    {
        SpeedMonitor_SetSpeedParamValid(E_SPEED_TYPE_POLYGON_AREA,s_stPolygonAttrib.areaID,s_stPolygonAttrib.limitSpeed,s_stPolygonAttrib.speedDuration);
    }
    /////////////////预警限速////////////////
    ////////////////////////////
    if(PUBLIC_CHECKBIT(s_stPolygonAttrib.areaAttrib,E_IN_SENDTO_DRIVER)) 
    {
        Public_PlayTTSVoiceStr(s_stPolygonAttrib.voiceStr);
    }
    ///////////////////////
    if(PUBLIC_CHECKBIT(s_stPolygonAttrib.areaAttrib,E_AREA_ENTER_TURNOFF_GPRS))
    {
        Area_SetCloseGPRSFlag(E_POLYGON_AREA);
    }
    //////////////////////////
    if(PUBLIC_CHECKBIT(s_stPolygonAttrib.areaAttrib,E_AREA_ENTER_COLLECT_GNSS))
    {
        Area_SetCollectGnssFlag(E_POLYGON_AREA);
    }
    ///////////////////////////////
    if(PUBLIC_CHECKBIT(s_stPolygonAttrib.areaAttrib,E_IN_SENDTO_SERVER)) 
    {
        Area_SetInOutAreaAlarmInfo(E_POLYGON_AREA, s_stPolygonAttrib.areaID,AREA_STATUS_IN);
    }
    //////////////
    Area_SetInAreaFlag(E_POLYGON_AREA);
}
/*****************************************************************************
*Function Name: Polygon_ChoiceAreaArmDest(unsigned char AlarmItem)
*Input  Parameter: AlarmItem
*Output Parameter: None
*Function Describtion:Select an area alarm destination 
*Writer:mark
*Update Data:2013.10.11
******************************************************************************/
void Polygon_ChoiceAreaArmDest(unsigned char AlarmItem) 
{
    switch(AlarmItem)
    {
        case ALARM_ITEM_OUT_AREA:
            Polygon_DisposeDriveOutArea();                    
            break;        
        case  ALARM_ITEM_IN_AREA:
            Polygon_DisposeDriveInArea();            
            break;                                                                  
        default:
            break;    
    }
}
/*****************************************************************************
*Function Name: Polygon_ScanArea(void)
*Input  Parameter: None
*Output Parameter: None
*Function Describtion:Execute the task scan polygon area  
*Writer:mark
*Update Data:2013.10.11
******************************************************************************/
void Polygon_ScanArea(void) 
{
    unsigned char i,j,len;
    unsigned long addr;
    STPOLYGON_PARAM stPolygonParam;                                  //Define a STPOLYGON_PARAM type stPolygonParam
    if(AREA_STATUS_IN != s_stPolygonAttrib.areaFlag)                              //Out of the polygon area
    {
        if(POLYGON_MAX_AREA_ADDR == s_stPolygonAttrib.addr)                               //No any polygon area be in
        {
            for(i=0; s_stPolygonAttrib.areaCnt < s_stPolygonAttrib.checkAreaNum && i<10; s_stPolygonAttrib.areaCnt++,i++)
            {
                addr = POLYGON_AREA_ADDR_START + s_stPolygonAttrib.areaCnt*ONE_POLYGON_AREA_SIZE;  
                sFLASH_ReadBuffer((unsigned char *)&stPolygonParam,addr,SIZE_STPOLYGON_PARAM); //Read the data in flash	
                if(0 == stPolygonParam.areaID)//Override the status ID=0
                    continue;
                s_stPolygonAttrib.vertexAddr    = addr + SIZE_STPOLYGON_PARAM;
                s_stPolygonAttrib.areaAttrib    = stPolygonParam.areaAttrib;
                s_stPolygonAttrib.timeType      = Area_GetConverseBCDTimeType(stPolygonParam.startMoment);
                s_stPolygonAttrib.startTime     = Area_ConverseBcdGmtime(&stPolygonParam.startMoment[0],s_stPolygonAttrib.timeType);
                s_stPolygonAttrib.endTime       = Area_ConverseBcdGmtime(&stPolygonParam.endMoment[0],s_stPolygonAttrib.timeType);
                if(0 == Polygon_CheckTime())//时间不符合
                    continue;
                s_stPolygonAttrib.minpoint  = stPolygonParam.minpoint;
                s_stPolygonAttrib.maxpoint  = stPolygonParam.maxpoint;
                if(Polygon_CheckIsInMinRectArea())//不在最小矩形内
                {
                    continue;
                }
                s_stPolygonAttrib.vertexNum = stPolygonParam.vertexNum;                
                Polygon_ReadEndPointData();
                //////////////////
                if(Polygon_ComparePos())//
                {
                    s_stPolygonAttrib.addr = addr;
                }
                //////////////////////////
                break;//多边形顶点很多,一次只进行一次多边形判断
            }       
            if(s_stPolygonAttrib.areaCnt>=s_stPolygonAttrib.checkAreaNum)s_stPolygonAttrib.areaCnt = 0;  //Rescanning
            if((POLYGON_MAX_AREA_ADDR == s_stPolygonAttrib.addr) && (s_stPolygonAttrib.preAreaCnt != s_stPolygonAttrib.areaCnt))
                LZM_PublicSetOnceTimer(&s_stPOLTimer[POL_TIMER_SCAN],PUBLICSECS(0.1),Polygon_ScanArea);
        }
        /////////////////////////////////
        if(s_stPolygonAttrib.addr != POLYGON_MAX_AREA_ADDR)                                 //Get the duration as remain in polygon area
        {
            s_stPolygonAttrib.areaFlag = AREA_STATUS_IN;
            sFLASH_ReadBuffer((unsigned char *)&stPolygonParam,s_stPolygonAttrib.addr,SIZE_STPOLYGON_PARAM);//Read data in flash                
            if(PUBLIC_CHECKBIT(stPolygonParam.areaAttrib,E_IN_SENDTO_DRIVER)) 
            {
                if(PUBLIC_CHECKBIT(stPolygonParam.userAreaAttrib,E_USER_IN_PLAY))
                {
                    len=stPolygonParam.InVoiceSize;
                    if(len>=VOICE_BYTE)len=VOICE_BYTE-1;
                    for(j=0; j<len;j++)
                    {
                        s_stPolygonAttrib.voiceStr[j]=stPolygonParam.InVoiceStr[j];                    
                    }
                    s_stPolygonAttrib.voiceStr[j]='\0';
                }
                else
                {
                    strcpy((char *)s_stPolygonAttrib.voiceStr,"你已驶入多边形区域");
                }
            }
            ///////////////////////
            s_stPolygonAttrib.areaID            = stPolygonParam.areaID; 
            s_stPolygonAttrib.areaAttrib        = stPolygonParam.areaAttrib;
            s_stPolygonAttrib.userAreaAttrib    = stPolygonParam.userAreaAttrib;            
            s_stPolygonAttrib.limitSpeed        = stPolygonParam.limitspeed; 
            s_stPolygonAttrib.speedDuration     = stPolygonParam.speedDuration;            
            if(s_stPolygonAttrib.speedDuration<10)s_stPolygonAttrib.speedDuration = 10;                
            Polygon_ChoiceAreaArmDest(ALARM_ITEM_IN_AREA);
        }
    }
    else
    if(AREA_STATUS_IN == s_stPolygonAttrib.areaFlag)                              //In the area
    {
        if(0 == Polygon_CheckTime())//时间不符合
        {
            Polygon_ResetParam();
        }
        else
        if(0==Polygon_IsInArea())//Get into the Polygon area
        {
            s_stPolygonAttrib.areaFlag = AREA_STATUS_OUT;                         
            sFLASH_ReadBuffer((unsigned char *)&stPolygonParam,s_stPolygonAttrib.addr,SIZE_STPOLYGON_PARAM);//Read data in flash	
            if(PUBLIC_CHECKBIT(stPolygonParam.areaAttrib,E_OUT_SENDTO_DRIVER)) 
            {
                if(PUBLIC_CHECKBIT(stPolygonParam.userAreaAttrib,E_USER_OUT_PLAY))
                {
                    len=stPolygonParam.OutVoiceSize;
                    if(len>=VOICE_BYTE)len=VOICE_BYTE-1;
                    for(j=0; j<len;j++)
                    {
                        s_stPolygonAttrib.voiceStr[j]=stPolygonParam.OutVoiceStr[j];
                    }
                    s_stPolygonAttrib.voiceStr[j]='\0'; 
                }
                else
                {
                    strcpy((char *)s_stPolygonAttrib.voiceStr,"你已离开多边型区域");
                }
            }
            s_stPolygonAttrib.areaID = stPolygonParam.areaID;
            s_stPolygonAttrib.areaAttrib = stPolygonParam.areaAttrib;
            s_stPolygonAttrib.userAreaAttrib = stPolygonParam.userAreaAttrib;
            Polygon_ChoiceAreaArmDest(ALARM_ITEM_OUT_AREA); 
            ///////////////////////////
            s_stPolygonAttrib.addr = POLYGON_MAX_AREA_ADDR;
        }
    }
    
    if((AREA_STATUS_IN==s_stPolygonAttrib.areaFlag)&&PUBLIC_CHECKBIT(s_stPolygonAttrib.areaAttrib,E_AREA_PROHIBIT_OPENDOOR))
    {
        Area_CheckOpenDoorAlarm(E_POLYGON_AREA);
    }
}
/*************************************************************
** 函数名称: Polygon_GetAreaUserDatalen
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
*************************************************************/
unsigned short Polygon_GetAreaUserDatalen(unsigned char *pBuffer, unsigned short BufferLen,unsigned long *AreaID)
{
    unsigned short datalen,userAreaAttrib;
    STPOLYGON_PARAM stPolygonParam; 
    datalen = 0;
    Public_Mymemcpy((unsigned char *)&stPolygonParam.areaID,(unsigned char *)&pBuffer[datalen],sizeof(unsigned long),1); 
    datalen+=sizeof(unsigned long);
    Public_Mymemcpy((unsigned char *)&userAreaAttrib,(unsigned char *)&pBuffer[datalen],sizeof(unsigned short ),1); 
    datalen+=sizeof(unsigned short );    
    if(PUBLIC_CHECKBIT(userAreaAttrib,E_USER_IN_PLAY))
    {     
        PUBLIC_SETBIT(stPolygonParam.userAreaAttrib,E_USER_IN_PLAY);
        Public_Mymemcpy((unsigned char *)&stPolygonParam.InVoiceSize,(unsigned char *)&pBuffer[datalen],sizeof(unsigned char),0); 
        datalen+=sizeof(unsigned char);
        datalen+=stPolygonParam.InVoiceSize;
    }
    if(PUBLIC_CHECKBIT(userAreaAttrib,E_USER_OUT_PLAY))
    {
        PUBLIC_SETBIT(stPolygonParam.userAreaAttrib,E_USER_OUT_PLAY);
        Public_Mymemcpy((unsigned char *)&stPolygonParam.OutVoiceSize,(unsigned char *)&pBuffer[datalen],sizeof(unsigned char),0);
        datalen+=sizeof(unsigned char);
        datalen+=stPolygonParam.OutVoiceSize;
    }
    if(datalen>BufferLen)return 0;
    *AreaID =stPolygonParam.areaID;
    return datalen;
}
 /*************************************************************
** 函数名称: Polygon_GetAreaUserDatalen
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
*************************************************************/
unsigned short Polygon_GetAreaDatalenStandard(unsigned char *pBuffer, unsigned short BufferLen,unsigned long *AreaID)
{
    unsigned short datalen;
    STPOLYGON_PARAM stPolygonParam; 
    datalen =0;
    Public_Mymemcpy((unsigned char *)&stPolygonParam.areaID,(unsigned char *)&pBuffer[datalen],sizeof(unsigned long),1); 
    datalen+=sizeof(unsigned long);
    Public_Mymemcpy((unsigned char *)&stPolygonParam.areaAttrib,(unsigned char *)&pBuffer[datalen],sizeof(unsigned short ),1); 
    datalen+=sizeof(unsigned short ); 
    if(PUBLIC_CHECKBIT(stPolygonParam.areaAttrib,E_AREA_MOMENT))
    {
        datalen += 6;                //Startmoment
        datalen += 6;                //Endmoment
    }
    if(PUBLIC_CHECKBIT(stPolygonParam.areaAttrib,E_AREA_LIMIT_SPEED))
    {
        datalen +=sizeof(unsigned short);  //Limited speed
        datalen +=sizeof(unsigned char);   //The duration of limited speed status
    }   
    Public_Mymemcpy((unsigned char *)&stPolygonParam.vertexNum,(unsigned char *)&pBuffer[datalen],sizeof(unsigned short ),1);
    datalen+=sizeof(unsigned short );      //Get the number of polygon vertex
    if(stPolygonParam.vertexNum>ONE_POLYGON_VERTEX_NUM_MAX)return 0;
    datalen += (stPolygonParam.vertexNum<<3);
    if(datalen>BufferLen)return 0;
    *AreaID =stPolygonParam.areaID;
    return datalen;
}
 /*************************************************************
** 函数名称: Area_GetRectAreaDatalenJiuTong
** 功能描述: :设置多边形时,九通协议长度
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned short Polygon_GetAreaDatalenJiuTong(unsigned char *pBuffer, unsigned short BufferLen,unsigned long *AreaID)
{
    unsigned short datalen;
    STPOLYGON_PARAM stPolygonParam; //读取多边形区域属性
    datalen =0;
    Public_Mymemcpy((unsigned char *)&stPolygonParam.areaID,(unsigned char *)&pBuffer[datalen],sizeof(unsigned long),1); 
    datalen+=sizeof(unsigned long);
    Public_Mymemcpy((unsigned char *)&stPolygonParam.areaAttrib,(unsigned char *)&pBuffer[datalen],sizeof(unsigned short ),1); 
    datalen+=sizeof(unsigned short ); 
    if(PUBLIC_CHECKBIT(stPolygonParam.areaAttrib,E_AREA_MOMENT))
    {
        datalen += 6;                //Startmoment
        datalen += 6;                //Endmoment
    }
    if(PUBLIC_CHECKBIT(stPolygonParam.areaAttrib,E_AREA_LIMIT_SPEED))
    {
        datalen +=sizeof(unsigned short);  //Limited speed
        datalen +=sizeof(unsigned char);   //The duration of limited speed status
    }
    ///////自定义位///////////////////////
    if(PUBLIC_CHECKBIT(stPolygonParam.areaAttrib,14))
    {     
        //进区提示音长度
        Public_Mymemcpy((unsigned char *)&stPolygonParam.InVoiceSize,(unsigned char *)&pBuffer[datalen],sizeof(unsigned char),0); 
        datalen+=sizeof(unsigned char);
        //提示音语句
        datalen+=stPolygonParam.InVoiceSize;    
    }
    ///////////////////////
    if(PUBLIC_CHECKBIT(stPolygonParam.areaAttrib,15))
    {
        //出区提示音长度
        Public_Mymemcpy((unsigned char *)&stPolygonParam.OutVoiceSize,(unsigned char *)&pBuffer[datalen],sizeof(unsigned char),0);
        datalen+=sizeof(unsigned char);
        //提示音语句
        datalen+=stPolygonParam.OutVoiceSize;
    } 
    ///////////顶点项/////////////////////
    Public_Mymemcpy((unsigned char *)&stPolygonParam.vertexNum,(unsigned char *)&pBuffer[datalen],sizeof(unsigned short ),1);
    datalen+=sizeof(unsigned short );      //Get the number of polygon vertex
    if(stPolygonParam.vertexNum>ONE_POLYGON_VERTEX_NUM_MAX)return 0;
    datalen += (stPolygonParam.vertexNum<<3);
    if(datalen>BufferLen)return 0;
    *AreaID =stPolygonParam.areaID;
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
unsigned short Polygon_GetAreaDatalenSuoMei(unsigned char *pBuffer, unsigned short BufferLen,unsigned long *AreaID)
{
    unsigned short datalen=0;
    return datalen;
}
/*************************************************************
** 函数名称: Polygon_SetAreaCheckProtoclType
** 功能描述: :设置区域时,查检是否有语音标志,仅为兼容九通版本
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Polygon_SetAreaCheckProtoclType(unsigned char *pBuffer, unsigned short datalen,unsigned long *AreaID)
{
    unsigned short templen;
    //////////////    
    if(datalen==0)return AREA_PROTOCOL_TYPE_EMPTY;
    ///////////////////////
    templen = Polygon_GetAreaDatalenStandard(pBuffer,datalen,AreaID);//单次只下发一个多边型
    if(datalen == templen)//相等,正确
    {
        return AREA_PROTOCOL_TYPE_STANDARD;
    }
    /////////////////////////
    templen=Polygon_GetAreaDatalenJiuTong(pBuffer,datalen,AreaID);    
    if(datalen == templen)//相等,正确
    {
        return AREA_PROTOCOL_TYPE_JIUTONG;//方法二,正确
    }
    /////////////////////
    return AREA_PROTOCOL_TYPE_EMPTY;
}
 /*************************************************************
** 函数名称: Polygon_GetAreaUserDatalen
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
*************************************************************/
unsigned short Polygon_SaveOneModifyUserData(unsigned long addr,unsigned char *pBuffer, unsigned short BufferLen)
{
    unsigned long AreaID;
    unsigned short datalen,userAreaAttrib;
    STPOLYGON_PARAM stPolygonParam;  //读取多边形区域属性    
    if(addr+ONE_POLYGON_AREA_SIZE>POLYGON_AREA_ADDR_END)return 0;
    sFLASH_ReadBuffer((unsigned char*)&stPolygonParam,addr,SIZE_STPOLYGON_PARAM); //读扇区
    datalen = 0;
    Public_Mymemcpy((unsigned char *)&AreaID,(unsigned char *)&pBuffer[datalen],sizeof(unsigned long),1); //获得区域ID
    if(AreaID!=stPolygonParam.areaID)return 0;
    datalen+=sizeof(unsigned long);
    Public_Mymemcpy((unsigned char *)&userAreaAttrib,(unsigned char *)&pBuffer[datalen],sizeof(unsigned short ),1); //获得区域属性
    datalen+=sizeof(unsigned short ); 
    //////////自定义/////////////////    
    if(PUBLIC_CHECKBIT(stPolygonParam.userAreaAttrib,E_USER_IN_PLAY))
    {        
        PUBLIC_SETBIT(stPolygonParam.userAreaAttrib,E_USER_IN_PLAY);
        Public_Mymemcpy((unsigned char *)&stPolygonParam.InVoiceSize,(unsigned char *)&pBuffer[datalen],sizeof(unsigned char),0); //进区提示音长度
        datalen+=sizeof(unsigned char);  
        Public_Mymemcpy(stPolygonParam.InVoiceStr,(unsigned char *)&pBuffer[datalen],stPolygonParam.InVoiceSize,0); //提示音语句
        datalen+=stPolygonParam.InVoiceSize;    
    }
    if(PUBLIC_CHECKBIT(userAreaAttrib,E_USER_OUT_PLAY))
    {
        PUBLIC_SETBIT(stPolygonParam.userAreaAttrib,E_USER_OUT_PLAY);
        Public_Mymemcpy((unsigned char *)&stPolygonParam.OutVoiceSize,(unsigned char *)&pBuffer[datalen],sizeof(unsigned char),0); //出区提示音长度
        datalen+=sizeof(unsigned char);  
        Public_Mymemcpy(stPolygonParam.OutVoiceStr,(unsigned char *)&pBuffer[datalen],stPolygonParam.OutVoiceSize,0); //提示音语句
        datalen+=stPolygonParam.OutVoiceSize;
    }
    ////////////////////////////
    if(datalen>BufferLen)return 0;
    //////////////////////////
    Area_MdyDataInFlash((unsigned char*)&stPolygonParam,addr,SIZE_STPOLYGON_PARAM);
    return datalen;
}
/*************************************************************
** 函数名称: Polygon_SaveOneArea
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 0:保存成功,1保存失败
*************************************************************/ 
unsigned char Polygon_SaveOneArea(unsigned long addr,unsigned char *pBuffer, unsigned short BufferLen)
{
    STPOLYGON_PARAM stPolygonParam;  //读取多边形区域属性
    unsigned short datalen,i=0,index,tmplen;
    unsigned long tmpaddr; 
    T_GPS_POS point,vertex[32];    
    if(addr+ONE_POLYGON_AREA_SIZE>POLYGON_AREA_ADDR_END)return 1;
    memset(&stPolygonParam,0,sizeof(STPOLYGON_PARAM));
    datalen = 0;
    Public_Mymemcpy((unsigned char *)&stPolygonParam.areaID,(unsigned char *)&pBuffer[datalen],sizeof(unsigned long),1); //获得区域ID
    datalen+=sizeof(unsigned long);
    Public_Mymemcpy((unsigned char *)&stPolygonParam.areaAttrib,(unsigned char *)&pBuffer[datalen],sizeof(unsigned short ),1); //获得区域属性
    datalen+=sizeof(unsigned short ); 
    //////////根据时间///////////////////
    if(PUBLIC_CHECKBIT(stPolygonParam.areaAttrib,E_AREA_MOMENT))
    {
        Public_Mymemcpy((unsigned char *)&stPolygonParam.startMoment[0],(unsigned char *)&pBuffer[datalen],6,0); //起始时间
        datalen += 6;  
        Public_Mymemcpy((unsigned char *)&stPolygonParam.endMoment[0],(unsigned char *)&pBuffer[datalen],6,0); //结束时间
        datalen += 6;  
    }
    //////////最高限速////////////
    if(PUBLIC_CHECKBIT(stPolygonParam.areaAttrib,E_AREA_LIMIT_SPEED))
    {
        Public_Mymemcpy((unsigned char *)&stPolygonParam.limitspeed,(unsigned char *)&pBuffer[datalen],sizeof(unsigned short ),1); //最高
        datalen+=sizeof(unsigned short );    
        Public_Mymemcpy((unsigned char *)&stPolygonParam.speedDuration,(unsigned char *)&pBuffer[datalen],sizeof(unsigned char),0); //超速持续时间
        datalen+=sizeof(unsigned char);  
    }
    ///////////自定义//////////////
    if(AREA_PROTOCOL_TYPE_JIUTONG == s_stPolygonAttrib.protoclType)
    {
        if(PUBLIC_CHECKBIT(stPolygonParam.areaAttrib,14))
        {
            PUBLIC_CLRBIT(stPolygonParam.areaAttrib,14);
            PUBLIC_SETBIT(stPolygonParam.userAreaAttrib,E_USER_IN_PLAY);               
            Public_Mymemcpy((unsigned char *)&stPolygonParam.InVoiceSize,(unsigned char *)&pBuffer[datalen],sizeof(unsigned char),0); //进区提示音长度
            datalen+=sizeof(unsigned char);  
            Public_Mymemcpy((unsigned char *)&stPolygonParam.InVoiceStr[0],(unsigned char *)&pBuffer[datalen],stPolygonParam.InVoiceSize,0); //提示音语句
            datalen+=stPolygonParam.InVoiceSize;    
        }
        ///////////////////////
        if(PUBLIC_CHECKBIT(stPolygonParam.areaAttrib,15))
        {
            PUBLIC_CLRBIT(stPolygonParam.areaAttrib,15);
            PUBLIC_SETBIT(stPolygonParam.userAreaAttrib,E_USER_OUT_PLAY);            
            Public_Mymemcpy((unsigned char *)&stPolygonParam.OutVoiceSize,(unsigned char *)&pBuffer[datalen],sizeof(unsigned char),0); //出区提示音长度
            datalen+=sizeof(unsigned char);  
            Public_Mymemcpy((unsigned char *)&stPolygonParam.OutVoiceStr[0],(unsigned char *)&pBuffer[datalen],stPolygonParam.OutVoiceSize,0); //提示音语句
            datalen+=stPolygonParam.OutVoiceSize;
        }
    }
    ///////多边形顶点项///////////////////
    Public_Mymemcpy((unsigned char *)&stPolygonParam.vertexNum,(unsigned char *)&pBuffer[datalen],sizeof(unsigned short ),1);
    datalen += sizeof(unsigned short );//Get the number of polygon vertex
    tmpaddr = addr + SIZE_STPOLYGON_PARAM;
    index = 0;
    for(i=0;i< stPolygonParam.vertexNum; i++)
    {
        Public_Mymemcpy((unsigned char *)&point.Lat,(unsigned char *)&pBuffer[datalen],sizeof(unsigned long),1); 
        datalen+=sizeof(unsigned long);             //Get the length of vertex latitude
        Public_Mymemcpy((unsigned char *)&point.Long,(unsigned char *)&pBuffer[datalen],sizeof(unsigned long),1); 
        datalen+=sizeof(unsigned long);             //Get the length of vertex longitude
        //////找最大最小值/////////////
        if(0 == i)//
        {
           stPolygonParam.minpoint = point;
           stPolygonParam.maxpoint = point;
        }
        else
        {
            if(stPolygonParam.minpoint.Lat > point.Lat)
            {
                stPolygonParam.minpoint.Lat = point.Lat;
            }
            if(stPolygonParam.minpoint.Long > point.Long)
            {
                stPolygonParam.minpoint.Long = point.Long;
            }
            ///////////////////////
            if(stPolygonParam.maxpoint.Lat < point.Lat)
            {
                stPolygonParam.maxpoint.Lat = point.Lat;
            }
            if(stPolygonParam.maxpoint.Long < point.Long)
            {
                stPolygonParam.maxpoint.Long = point.Long;
            }
        }
        //////暂存//////////////////////////
        vertex[index++]=point;
        if(index>=32)//分块写入比较快
        {
            tmplen =(index<<3);            
            Area_MdyDataInFlash((unsigned char*)&vertex,tmpaddr,tmplen);
            tmpaddr += tmplen;
            index = 0;
        }
    }
    ////////////////////////
    if(index)//还有未写入的顶点数据
    {
        tmplen =(index<<3);
        Area_MdyDataInFlash((unsigned char*)&vertex,tmpaddr,tmplen);
    }
    //////////////////////
    stPolygonParam.Head = POLYGON_SAVE_FLAG;
    ///////////////////////////
    Area_MdyDataInFlash((unsigned char*)&stPolygonParam,addr,SIZE_STPOLYGON_PARAM);
    return 0;
}

/*************************************************************
** 函数名称: Polygon_SetArea
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
*************************************************************/
unsigned char Polygon_SetArea(unsigned char *pBuffer, unsigned short BufferLen)
{
    STPOLYGON_PARAM stPolygonParam;  //读取多边形区域属性
    unsigned long tmpAddr,address,AreaID,tmpAreaID;
    unsigned short datalen;
    unsigned char AckResult,i,protoclType;
    unsigned char *pTmp;
    //////////////////
    if(BufferLen==0)return 2;
    /////////////////
    pTmp = pBuffer;
    datalen = BufferLen;
    protoclType = Polygon_SetAreaCheckProtoclType(pTmp,datalen,&AreaID);//单次只下发一个多边型
    if(AREA_PROTOCOL_TYPE_EMPTY==protoclType)return 2;
    s_stPolygonAttrib.protoclType = protoclType;
    ///////////////////////////
    address =0;
    for(i = s_stPolygonAttrib.checkAreaNum; i>0; i--)
    {
        tmpAddr=POLYGON_AREA_ADDR_START + (i-1)*ONE_POLYGON_AREA_SIZE;
        sFLASH_ReadBuffer((unsigned char *)&stPolygonParam,tmpAddr,POLYGON_HEAD_ID_SIZE);
        if(AreaID==stPolygonParam.areaID)
        {
            
            address   = tmpAddr;
            tmpAreaID = stPolygonParam.areaID;
            break;
        }
        else
        if(0==stPolygonParam.areaID)
        {
            address   = tmpAddr;
            tmpAreaID = stPolygonParam.areaID;
        }
    }
    ////////////////////////////
    if(0 == address)//
    {
        if(s_stPolygonAttrib.checkAreaNum >= POLYGON_AREA_NUM_MAX)return 1;
        ////////////////////////
        address = POLYGON_AREA_ADDR_START + s_stPolygonAttrib.checkAreaNum*ONE_POLYGON_AREA_SIZE;
        s_stPolygonAttrib.checkAreaNum++;
        s_stPolygonAttrib.validAreaNum++;
    }
    else
    {
        if(0==tmpAreaID)//替换ID为0的多边形数据
        {
            s_stPolygonAttrib.validAreaNum++;
        }
    }
    ///////////////////////
    pTmp = pBuffer;
    datalen = BufferLen;
    AckResult = Polygon_SaveOneArea(address,pTmp,datalen);
    Polygon_CheckForResetParam(AreaID);
    s_stPolygonAttrib.preSpeed = 0xff;
    return AckResult;
}
/*************************************************************
** 函数名称: Polygon_SetArea
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
*************************************************************/ 
unsigned char Polygon_DeleteArea(unsigned char *pBuffer, unsigned short BufferLen) 
{
    unsigned char i,j,Sum,flag;
    unsigned long addr,AreaID;
    STPOLYGON_PARAM stPolygonParam;  
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
            for(j=0; j< s_stPolygonAttrib.checkAreaNum; j++)
            {
                addr=POLYGON_AREA_ADDR_START + j*ONE_POLYGON_AREA_SIZE;
                sFLASH_ReadBuffer((unsigned char *)&stPolygonParam,addr,POLYGON_HEAD_ID_SIZE);
                if(AreaID==stPolygonParam.areaID)
                {
                    flag = 1;
                    break;
                }
            }
            if(0 == flag)return 2;
        }
        pTmp =pBuffer;
        for(i=0; i<Sum; i++)
        {
            Public_Mymemcpy((unsigned char *)&AreaID,(unsigned char *)&pTmp[0],sizeof(unsigned long),1); //Get area ID
            pTmp+=sizeof(unsigned long);
            for(j=0; j<s_stPolygonAttrib.checkAreaNum; j++)
            {
                addr = POLYGON_AREA_ADDR_START + j*ONE_POLYGON_AREA_SIZE;
                sFLASH_ReadBuffer((unsigned char *)&stPolygonParam,addr,POLYGON_HEAD_ID_SIZE);
                if(AreaID==stPolygonParam.areaID&&AreaID)                          //Deleted area ID 
                {
                    stPolygonParam.areaID = 0;                             //Delete area and clear ID
                    s_stPolygonAttrib.validAreaNum--;
                    addr = POLYGON_AREA_ADDR_START + j*ONE_POLYGON_AREA_SIZE;
                    stPolygonParam.Head = POLYGON_SAVE_FLAG;
                    Area_MdyDataInFlash((unsigned char*)&stPolygonParam,addr,POLYGON_HEAD_ID_SIZE);
                    Polygon_CheckForResetParam(AreaID);
                }
            }
        } 
    }
    else//Clear all polygon area
    {
        Polygon_ClearArea();
    }
    return 0;
}
/*************************************************************
** 函数名称: Polygon_AreaModifyUserData
** 功能描述: 修改多边形区域自定义部分
** 入口参数: 
** 出口参数: 
** 返回参数: 
*************************************************************/ 
unsigned char Polygon_AreaModifyUserData(unsigned char *pBuffer, unsigned short BufferLen)
{
    unsigned long addr,AreaID;
    unsigned char i,j,Sum,flag;
    STPOLYGON_PARAM stPolygonParam;  //读取多边形区域属性
    unsigned short datalen,templen;
    unsigned char *pTmp;
    //////////////
    if(BufferLen==0)return 2;
    ////////////
    Sum=*pBuffer++;
    pTmp =pBuffer;
    datalen=BufferLen-1;
    //多边形
    for(i=0; i<Sum; i++)//检查ID
    {        
        templen=Polygon_GetAreaUserDatalen(pTmp,datalen,&AreaID);
        if(0 == templen)return 2;//消息有误;
        //////////////////////////
        flag =0;
        for(j=0; j<s_stPolygonAttrib.checkAreaNum; j++)
        {
            addr=POLYGON_AREA_ADDR_START + j*ONE_POLYGON_AREA_SIZE;
            sFLASH_ReadBuffer((unsigned char *)&stPolygonParam,addr,POLYGON_HEAD_ID_SIZE);
            if(AreaID==stPolygonParam.areaID)
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
        for(j=0; j<s_stPolygonAttrib.checkAreaNum; j++)
        {
            addr=POLYGON_AREA_ADDR_START + j*ONE_POLYGON_AREA_SIZE;
            sFLASH_ReadBuffer((unsigned char *)&stPolygonParam,addr,POLYGON_HEAD_ID_SIZE);
            if(AreaID==stPolygonParam.areaID)
            {
                templen=Polygon_SaveOneModifyUserData(addr,pTmp,datalen);
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
** 函数名称: Polygon_GetAreaTotalNum
** 功能描述: 获得多边形区域总数
** 入口参数: 
** 出口参数: 
** 返回参数: 多边形区域总数
*************************************************************/ 
unsigned char Polygon_GetAreaTotalNum(void)
{
    return s_stPolygonAttrib.validAreaNum;
}
/*************************************************************
** 函数名称: Polygon_ScanTask
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Polygon_CheckScanTask(void)
{
    unsigned char speed;
    //读进出区域报警屏蔽位状态
    if(Io_ReadAlarmMaskBit(ALARM_BIT_IN_OUT_AREA))
    {
        if(1==s_stPolygonAttrib.noNavigationCnt)
        {
            s_stPolygonAttrib.noNavigationCnt = 0;
            Polygon_ResetParam();
        }
        return;
    }
    Polygon_GetCurGpsCoordinate();//1s获取一次经纬度        
    s_stPolygonAttrib.noNavigationCnt = 1;
    s_stPolygonAttrib.preAreaCnt=s_stPolygonAttrib.areaCnt;
    speed = Area_GetCurSpeed();
    ///////////////////////////
    if(0==speed&&0==s_stPolygonAttrib.preSpeed)return;//速度为零,不处理
    /////////////////////////
    s_stPolygonAttrib.preSpeed = speed;
    Polygon_ScanArea();
}
/*************************************************************
** 函数名称: Polygon_ScanTask
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Polygon_ScanTask(void)
{
    if(Io_ReadStatusBit(STATUS_BIT_NAVIGATION))
    {
        GPS_STRUCT stTmpGps;
        Gps_CopygPosition(&stTmpGps);
        if(stTmpGps.Second!=s_stPolygonAttrib.preSecond)
        {
            s_stPolygonAttrib.preSecond = stTmpGps.Second;
            LZM_PublicKillTimer(&s_stPOLTimer[POL_TIMER_SCAN]);
            Polygon_CheckScanTask();
        }
    }
    else
    {  
        s_stPolygonAttrib.preSecond = 0xff;
        s_stPolygonAttrib.preSpeed = 0xff;
        if(s_stPolygonAttrib.noNavigationCnt<50)s_stPolygonAttrib.noNavigationCnt++;//10s
        else
        if(s_stPolygonAttrib.noNavigationCnt == 50)
        {
            s_stPolygonAttrib.noNavigationCnt =100;
            Area_ClearAllAlarm(E_POLYGON_AREA);
        }
    }
}

/*************************************************************
** 函数名称: Polygon_ScanTask
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Polygon_StartScanTask(void)
{
    LZM_PublicSetCycTimer(&s_stPOLTimer[POL_TIMER_TASK],PUBLICSECS(0.2),Polygon_ScanTask);
}
/*************************************************************
** 函数名称: Polygon_TimeTask
** 功能描述: 区域功能定时器处理函数
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
FunctionalState Polygon_TimeTask(void)
{
    LZM_PublicTimerHandler(s_stPOLTimer,POL_TIMERS_MAX);
    return ENABLE;
}
/*************************************************************
** 函数名称: Polygon_ParameterInitialize
** 功能描述: 区域参数初始化
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Polygon_ReadAndCheckParam(void) 
{
    unsigned long addr;
    unsigned short i;    
    STPOLYGON_PARAM stPolygonParam;	  //读取多边形区域属性
    s_stPolygonAttrib.preSecond = 0xff;
    s_stPolygonAttrib.preSpeed = 0xff;
    s_stPolygonAttrib.noNavigationCnt =0xff;  
    ////////////    
    //多边形初始化
    Polygon_AreaParameterInit();
    for(i=0; i<POLYGON_AREA_NUM_MAX; i++)
    {
        addr=POLYGON_AREA_ADDR_START + i*ONE_POLYGON_AREA_SIZE ;  
        sFLASH_ReadBuffer((unsigned char *)&stPolygonParam,addr,POLYGON_HEAD_ID_SIZE);//读出区域数据	
        if(POLYGON_SAVE_FLAG == stPolygonParam.Head)
        {
            if(0xFFFFFFFF == stPolygonParam.areaID)
            {
                Polygon_ClearArea();
                break;
            }
            else
            {
                s_stPolygonAttrib.checkAreaNum++;
                if(stPolygonParam.areaID)s_stPolygonAttrib.validAreaNum++;
            }
        }
        else 
        if(stPolygonParam.Head != 0xFFFFFFFF)
        {
            Polygon_ClearArea();
            break;
        }
        else 
        if(0xFFFFFFFF == stPolygonParam.Head)
        {
            break;
        }
    }
    //////////////////////////////////////////////////////////
    LZM_PublicSetOnceTimer(&s_stPOLTimer[POL_TIMER_TASK],PUBLICSECS(20),Polygon_StartScanTask);    
 }
/*************************************************************
** 函数名称: Polygon_ParameterInitialize
** 功能描述: 区域参数初始化
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Polygon_ParameterInitialize(void) 
{
    LZM_PublicKillTimerAll(s_stPOLTimer,POL_TIMERS_MAX);
    ///////////////////////////
    SetTimerTask(TIME_POLYGON,LZM_AT_ONCE);
    //////////////////////////////////////////////////////////
    LZM_PublicSetOnceTimer(&s_stPOLTimer[POL_TIMER_TASK],PUBLICSECS(10),Polygon_ReadAndCheckParam);    
 }

/*******************************************************************************
 *                             end of module
 *******************************************************************************/

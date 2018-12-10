/*******************************************************************************
 * File Name:			Route.c 
 * Function Describe:	
 * Relate Module:		线路检测
 * Writer:				Joneming
 * Date:				2013-10-17
 * ReWriter:			
 * Date:				
 ***根据平台算法改写,去掉延时判断
 *******************************************************************************/
//****************包含文件*************************
#include "include.h"

//****************宏定义********************
#define ONE_TIME_READ_NUM           10  //每一次判断的顶点数
//////////////////////////////////////
#define IN_OUT_ROUTE_OVERHEAD_INFO_ID               0x12 //进出线路报警
#define IN_OUT_ROUTE_OVERHEAD_INFO_LEN              0x06;//进出线路报警附加信息长度
//////////////////////////////////////
#define LOAD_OVER_DRIVE_OVERHEAD_INFO_ID            0x13 //路段行驶时间不足/过长报警
#define LOAD_OVER_DRIVE_OVERHEAD_INFO_LEN           0x07;//路段行驶时间不足/过长报警附加信息长度
//****************结构定义*******************
              
typedef enum{
    IN_ROUTE=0,    //进状态,在线路内
    OUT_ROUTE      //出状态,不在线路内
}E_INOUTROUTE;

//线路算法状态
typedef enum{
    ROUTE_FIND_STATUS=0,            //查找当前属于那条线路
    ROUTE_FIND_ROAD_SEGMENT,        //查找当前属于那个路段
    ROUTE_CHECK_IN_ROAD_SEGMENT,    //检查是否在某段路段
    ROUTE_HOLD_ROAD_SEGMENT,        //保持在某段路段
    ROUTE_CHANGE_ROAD_SEGMENT,      //改变为某段路段
    ROUTE_CHECK_OUT_ROUTE,          //在当前线路时，出线路检测
}T_ROUTE_STATUS; 

//线路属性                                                              
typedef enum{
    E_ROUTE_MOMENT      ,       //在那线路中时根据时间,在路段是行驶时间属性。
    E_ROUTE_SPEED       ,       //在线路属性中保留,在路段属性中:为限速
    E_INROUTE_TO_DRIVER ,       //进路线报警给司机
    E_INROUTE_TO_SERVER ,       //进路线报警给平台
    E_OUTROUTE_TO_DRIVER,       //出线路报警给司机
    E_OUTROUTE_TO_SERVER,        //出线路报警给平台
    E_OUTROUTE_MAX
}E_ROUTE_FLAG;

//-------------------------------------------------
//线路结构   
//-------------------------------------------------
typedef struct          //总长40字节
{
    u16  Head;            //头标志
    u8   reserve[2];      //保留    
    u8   startTime[6];    //开始时间
    u8   endTime[6];      //结束时间
    u32  routeID;         //线路ID
    u16  routeAttrib;     //线路属性
    u16  nodeSum;         //总的节点数
    T_GPS_POS LeftUpPos;  //矩形_左上点纬度
    T_GPS_POS RightDowPos;//矩形_右下点纬度
}STROUTE_HEAD;

#define STROUTE_HEAD_SIZE sizeof(STROUTE_HEAD)

//-------------------------------------------------
//节点结构  
//-------------------------------------------------
typedef struct             //总长28字节
{
    u32 NodeID;            //拐点ID
    u32 roadSegID;         //路段ID
    T_GPS_POS point;       //节点经纬度
    u16 longLimitTime;     //最长行驶时间
    u16 shortLimitTime;    //最短行驶时间
    u16 driverTime;        //记录当前路段已行驶时间。
    u16 limitSpeed;        //限速值
    u8  speedDuration;     //超速最大持续时间     
    u8  roadWidth;         //路宽
    u8  roadAttrib;        //路段属性
    u8  reserve;           //保留 
}ST_ROUTE_NODE;
/////////////////////////
#define STROUTE_NODE_SIZE sizeof(ST_ROUTE_NODE)
//-------------------------------------------------
//线路应用结构
//-------------------------------------------------
typedef struct
{
    u32 Latitude;//纬度
    u32 Longitude;//经度
    ///////////////////////    
    u32 startTime;       //道路通行时间段
    u32 endTime;         //道路通行时间段 
    /////////////////线路属性//////////////
    u32 routeID;          //当前线路ID
    u16 routeAttrib;      //线路属性 
    u16 nodeSum;          //当前线路节点总数
    T_GPS_POS LeftUpPos;  //矩形_左上点纬度
    T_GPS_POS RightDowPos;//矩形_右下点纬度
    T_GPS_POS point2;     //第二个节点经纬度 
    /////////////////////
    ST_ROUTE_NODE stNode;          //节点信息   
    ///////////////////////////
    STPLAY_ATTRIB stPlay; //偏离路线报警播报
    /////////////     
    double roadDistance; //线段距离
    u32  nodeAddr;         //节点首地址
    u32  startdriverTime;  //开始驾驶计时时间
    u16  curNode;          //当前节点号
    u8   status;           //线路状态 0:线路搜索  1:线路处理
    u8   curRoute;         //当前线路编号 
    u8   routeSum;         //线路总数 
    u8   minRoadWidth;     //最小路宽
    u8   timeType;   //线路在某个时间段可用类型。如果为0-则是年月日，时分秒。如果为1则是每天的，时分秒。
    u8   preSecond;
    u8   preSpeed;
    u8   nonavigationCnt;
    u8   overTimeFlag;
    u8   needplayflag;    
}ST_ROUTE_ATTRIB; 

static ST_ROUTE_ATTRIB s_stRouteAttrib;            //定义线路变量
//-------------------------------------------------
//线路经纬度和报警全局结构
//-------------------------------------------------
typedef struct 
{
    u32 INOUTTypeID;     //区域或线路ID
    u8 INOUTPostType;    //0:无特定位置  1:圆形  2:矩形  3:多边形 4:路段
    u8 Orientation;      //0进，1:出
    //路线行驶时间不足/过长附件信息
    u32 roadSegID;        //路段ID
    u16 driverTime;       //行驶时间
    u8 driverResult;      //0:不足 1:过长
}T_ROUTE_ALARM;

static T_ROUTE_ALARM s_stRouteAlarm;
//////////////////////////////////////////////
#define ROUTE_RESET_STATUS_COUNT        50///持续不导航10s则清空状态.(0.2s)
////////////////////////////////////////
#define ROUTE_ADDR_START             (u32)(FLASH_ROUTE_START_SECTOR * FLASH_ONE_SECTOR_BYTES)   //线路存放开始位置
////////////////////////////////////////////
#define ONE_ROUTE_AREA_SIZE          (u32)(STROUTE_HEAD_SIZE+(ROUTE_NODE_SUM*STROUTE_NODE_SIZE))
////////////////////////
#define ROUTE_HEAD_FLAG    0xFEDC

enum 
{
    ROUTE_TIMER_TASK,                       //  
    ROUTE_TIMER_TEST,						//   
    ROUTE_TIMERS_MAX
}STROUTETIME;

static LZM_TIMER s_stRouteTimer[ROUTE_TIMERS_MAX];
/////////////////
//****************函数声明*******************
void Route_GoNextStatus(u8 NextStatus); //Route状态转化
void Route_GetNodeData(u16 CurNode);//获得线路数据
void Route_CheckDriverTime(void) ; //检测行驶时长,是否满足要求
void Route_DisposeInOutAlarm(E_INOUTROUTE InOutAlarm);
void Route_SetSpeedParam(void);
void Route_CheckScanTask(void);
void Route_ParameterInitialize(void);

//****************函数定义******************
/////////////计算两点距离使用/////////////////////////////////////
#define DEF_PI180        0.01745329252 //=PI/180.0//
#define EARTH_RADIUS     6378.137
#define DEF_PI180_EX     0.00000001745329252 //=PI/180.0/1000000.0//(上传的经纬度,已放大1000000倍)
#define EARTH_RADIUS_EX  12756274.0 //=2*EARTH_RADIUS*1000;
#if 0
/*************************************************************OK
** 函数名称: Route_GetPointDistance
** 功能描述: 地球上位置点直接的距离运算(米)
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/ 
double Route_GetDistanceEx(unsigned long lat1, unsigned long lng1, unsigned long lat2, unsigned long lng2)
{
    double radLat1 = lat1* DEF_PI180/1000000.0;
    double radLat2 = lat2* DEF_PI180/1000000.0;
    double a = radLat1 - radLat2;
    double b = lng1* DEF_PI180/1000000.0 - lng2 * DEF_PI180/1000000.0;
    double s = 2.0 * asin(
    sqrt(pow(sin(a / 2), 2)
    +
    cos(radLat1) * cos(radLat2) * pow(sin(b / 2), 2)))
    * EARTH_RADIUS
    * 1000.0;
    return s;
}
#endif
/*************************************************************OK
** 函数名称: Route_GetPointDistance
** 功能描述: 地球上位置点直接的距离运算(米)
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/ 
double Route_GetDistance(unsigned long lat1, unsigned long lng1, unsigned long lat2, unsigned long lng2)
{
    double radLat1 = lat1* DEF_PI180_EX;
    double radLat2 = lat2* DEF_PI180_EX;
    double a = radLat1 - radLat2;
    double b = ((double)lng1-(double)lng2)* DEF_PI180_EX;
    double s = asin(
    sqrt(pow(sin(a / 2), 2)
    +
    cos(radLat1) * cos(radLat2) * pow(sin(b / 2), 2)))
    * EARTH_RADIUS_EX;
    return s;
}

/*************************************************************OK
** 函数名称: Route_GetPointDistance
** 功能描述: 地球上位置点直接的距离运算(米)
** 入口参数: Point1,Point2
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/ 
double Route_GetPointDistance(T_GPS_POS *Point1,T_GPS_POS *Point2)
{
    return Route_GetDistance(Point1->Lat, Point1->Long,Point2->Lat, Point2->Long);
}
/*************************************************************OK
** 函数名称: Route_GetNearestDistance
** 功能描述: 计算点p3到线段AB的距离
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/ 
double Route_GetNearestDistance(T_GPS_POS *curPoint,T_GPS_POS *PA,T_GPS_POS *PB,double Pab)   
{
    double d1,d2,d;
    double dd,dd2,dd1;
    double ss;
    d1 = Route_GetPointDistance(PB,curPoint);
    if(d1<=0.000001)
        return 0.0f;  
    ////////////////////////////////
    d =  Pab; 
    if(d<=0.000001)
        return d1;//如果PA和PB坐标相同，则退出函数，并返回距离
    /////////////////////
    d2 = Route_GetPointDistance(PA,curPoint);   
    if(d2<=0.000001)
        return 0.0f;  
    ///////////
    dd1 = d1*d1;
    dd2 = d2*d2;
    dd = d*d;
    //------------------------------ 
    if(dd1>=dd2+dd)
        return d2; 
    if(dd2>=dd1+dd)
        return d1; 
    //////////////////////
    dd=(d1+d2+d)/2;     //周长的一半   
    ss=sqrt(dd*(dd-d1)*(dd-d2)*(dd-d));  //海伦公式求面积   
    return (2.0*ss/d);
}
/*************************************************************OK
** 函数名称: Route_SetSpeedParam
** 功能描述: 进入路段后初始化超速检测
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/  
void Route_SetSpeedParam(void)
{
    unsigned char tmp = 0;
    tmp=(s_stRouteAttrib.stNode.speedDuration < 10) ? 10:s_stRouteAttrib.stNode.speedDuration;           //保证最少10秒
    if(PUBLIC_CHECKBIT(s_stRouteAttrib.stNode.roadAttrib,E_ROUTE_SPEED))     //有速度限制      
    {
        SpeedMonitor_SetSpeedParamValid(E_SPEED_TYPE_ROUTE,s_stRouteAttrib.routeID,s_stRouteAttrib.stNode.limitSpeed,tmp);
    }
    else
    {
        SpeedMonitor_ClearSpeedParamValid(E_SPEED_TYPE_ROUTE);
    }    
}
/*************************************************************OK
** 函数名称: Route_ClearSpeedParam
** 功能描述: 解除超速报警
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/  
void Route_ClearSpeedParam(void)
{
    SpeedMonitor_ClearSpeedParamValid(E_SPEED_TYPE_ROUTE);
}
/************************************************************
函数名称: 清除所有线路报警
功能描述: 
入口参数:
出口参数:
************************************************************/ 
void Route_ClearAlarm(void) 
{
    s_stRouteAttrib.status = ROUTE_FIND_STATUS;              //设置导航后，重新寻路。
    Io_WriteAlarmBit(ALARM_BIT_IN_OUT_ROAD,RESET);         //清除进去路线
    Io_WriteAlarmBit(ALARM_BIT_LOAD_EXCURSION,RESET);      //清除偏离路线报警  
    Io_WriteAlarmBit(ALARM_BIT_LOAD_OVER_DRIVE,RESET);     //清除行驶时间报警
    Route_ClearSpeedParam(); //解除超速报警 
    s_stRouteAttrib.startdriverTime = Timer_Val();                     //驾驶时长重新计算.因为不导航不知道下次导航还在本路段，还是到其他路段去了。
}
/*************************************************************
** 函数名称: Route_ResetParam
** 功能描述: 线路重置参数
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Route_ResetParam(void) 
{
    Route_ClearAlarm();
    ///////////////////
    s_stRouteAttrib.curNode = 0;
    s_stRouteAttrib.curRoute = 0;
    s_stRouteAttrib.preSpeed = 0xff;
}
/*************************************************************
** 函数名称: ClrRoute
** 功能描述: 把线路数据区清空
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void ClrRoute(void)
{
    Route_ParameterInitialize();
    Area_EraseFlash(FLASH_ROUTE_START_SECTOR,FLASH_ROUTE_END_SECTOR);      //擦除线路
}
/*************************************************************
** 函数名称: Route_GoNextStatusAtOnce
** 功能描述: 立即进入下一个状态
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Route_GoNextStatusAtOnce(u8 NextStatus)
{
    s_stRouteAttrib.status = NextStatus;
    ////////////////////////////////
    LZM_PublicSetOnceTimer(&s_stRouteTimer[ROUTE_TIMER_TEST],LZM_AT_ONCE, Route_CheckScanTask);
}
/*************************************************************
** 函数名称: Route_GoNextStatus
** 功能描述: 状态转化
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Route_GoNextStatus(u8 NextStatus)
{
    LZM_PublicKillTimer(&s_stRouteTimer[ROUTE_TIMER_TEST]);
    s_stRouteAttrib.status = NextStatus;
}
/*************************************************************
** 函数名称: Route_CheckIsInRoute
** 功能描述: 判断车辆是否在线路内
** 入口参数: 线路两端经纬度,路宽
** 出口参数: 
** 返回参数: 1:在线路内  0:不在
** 全局变量: 无
** 调用模块: 无
*************************************************************/
u8 Route_CheckIsInRoute( T_GPS_POS *curPos,T_GPS_POS *Dot1, T_GPS_POS *Dot2,u16 Width)
{
    double distance;
    unsigned short tmpWidth;
    tmpWidth = Width;
    if(tmpWidth < s_stRouteAttrib.minRoadWidth)
        tmpWidth = s_stRouteAttrib.minRoadWidth; //路宽做限定,不能小于30米
    ////////////////////
    tmpWidth>>=1;
    ////////////////
    distance = Route_GetNearestDistance(curPos,Dot1,Dot2,s_stRouteAttrib.roadDistance);
    if(distance > tmpWidth) return 0;//不在线路内
    return 1;//在线路内    
}
/*************************************************************
** 函数名称: Route_CheckIsInRouteEx
** 功能描述: 判断车辆是否在线路内
** 入口参数: 线路两端经纬度,路宽
** 出口参数: 
** 返回参数: 1:在线路内  0:不在
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Route_CheckIsInRouteEx(void)
{
    return Route_CheckIsInRoute((T_GPS_POS *)&s_stRouteAttrib.Latitude,&s_stRouteAttrib.stNode.point, &s_stRouteAttrib.point2, s_stRouteAttrib.stNode.roadWidth);
}
/*************************************************************
** 函数名称: Route_CheckTime
** 功能描述: 检测线路是否符合时间范围
** 入口参数: 
** 出口参数: 
** 返回参数: 0:不符合时间范围,1:符合时间范围
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Route_CheckTime(void)
{
    u32 curTime = 0;
    //是否需要检查时间
    if(!PUBLIC_CHECKBIT(s_stRouteAttrib.routeAttrib,E_ROUTE_MOMENT)) return 1; //不需要检查时间                  
    curTime=Area_GetConvertCurTime(s_stRouteAttrib.timeType); //当前时间  
    return Public_CheckTimeIsInRange(curTime,s_stRouteAttrib.startTime,s_stRouteAttrib.endTime);    
}
/*************************************************************
** 函数名称: Route_CheckTimeEx
** 功能描述: 检测线路是否符合时间范围
** 入口参数: 
** 出口参数: 
** 返回参数: 0:不符合时间范围,1:符合时间范围
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Route_CheckTimeEx(void)
{
    if(Route_CheckTime())return 1;
    //清除所有线路报警,重新查找线路功能
    Route_ClearAlarm();
    return 0;
}
/*************************************************************
** 函数名称: Route_GetNodeData
** 功能描述: 获取线路当前节点信息，和下一节点的GPS位置信息
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Route_GetNodeData(u16 CurNode)
{   
    u32 OffSetAddr; 
    ST_ROUTE_NODE NodeData;
    OffSetAddr= s_stRouteAttrib.nodeAddr + CurNode*STROUTE_NODE_SIZE; //存放节点的地址  
    sFLASH_ReadBuffer((u8 *)&s_stRouteAttrib.stNode,OffSetAddr,STROUTE_NODE_SIZE);                                  //读出节点数据
    //读接下来一条节点数据         
    OffSetAddr += STROUTE_NODE_SIZE; //存放节点的地址
    sFLASH_ReadBuffer((u8 *)&NodeData,OffSetAddr,STROUTE_NODE_SIZE);                                         //读出节点数据  
    s_stRouteAttrib.point2    = NodeData.point;//保存GPS数据信息
    //////////////////////
    s_stRouteAttrib.roadDistance = Route_GetPointDistance(&s_stRouteAttrib.stNode.point,&s_stRouteAttrib.point2);  
}
/*************************************************************
** 函数名称: Route_GetNodeDataAndCheckInRoute
** 功能描述: 获取线路当前节点信息，和下一节点的GPS位置信息,并判断车辆是否在线路内
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Route_GetNodeDataAndCheckInRoute(u16 CurNode)
{
    Route_GetNodeData(CurNode);
    return Route_CheckIsInRouteEx();
}
/*************************************************************
** 函数名称: Route_FindRouteAreaTask
** 功能描述: 查找车辆所在的线路区域
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Route_FindRouteAreaTask(void)
{
    unsigned char flag,i,check;    
    unsigned long addr;
    STROUTE_HEAD stRoute;	    //读取线路数据
    flag = 0;
    check = 0;    
    for(i=0; i<s_stRouteAttrib.routeSum;i++)
    {
        addr = ROUTE_ADDR_START + i*ONE_ROUTE_AREA_SIZE ;  
        sFLASH_ReadBuffer((u8 *)&stRoute,addr,STROUTE_HEAD_SIZE); //读出线路数据
        if(stRoute.routeID==0) //跳过ID=0
            continue;
        //如果外切矩形的，坐标为0,0,则说明是多包下发节点失败产生数据，这里直接跳过?        
        if((stRoute.LeftUpPos.Lat == 0)||(stRoute.LeftUpPos.Long == 0))//跳过
            continue;
        s_stRouteAttrib.timeType    = Area_GetConverseBCDTimeType(&stRoute.startTime[0]); 
        s_stRouteAttrib.startTime   = Area_ConverseBcdGmtime(&stRoute.startTime[0],s_stRouteAttrib.timeType); //转换时间为秒，便于对比。
        s_stRouteAttrib.endTime     = Area_ConverseBcdGmtime(&stRoute.endTime[0],s_stRouteAttrib.timeType);  
        //////////////////////////////
        s_stRouteAttrib.routeID     = stRoute.routeID;
        s_stRouteAttrib.routeAttrib = stRoute.routeAttrib;
        s_stRouteAttrib.nodeSum     = stRoute.nodeSum;
        s_stRouteAttrib.LeftUpPos   = stRoute.LeftUpPos;
        s_stRouteAttrib.RightDowPos = stRoute.RightDowPos;
        /////////////////////////////
        //memcpy(&s_stRouteAttrib.routeID,&stRoute.routeID,24);
        //检查线路是否在可用时间段内
        if(!Route_CheckTime())//跳过不在时间段内
            continue;
        if(PUBLIC_CHECKBIT(s_stRouteAttrib.routeAttrib,E_OUTROUTE_TO_SERVER))//出线路报警给平台
        {
            flag = 1;
        }
        //计算当前点是否在矩形内,优化算法。
        if(!ComparePos(&stRoute.LeftUpPos,&stRoute.RightDowPos,(T_GPS_POS *)&s_stRouteAttrib.Latitude))
        {
            check =1;
            s_stRouteAttrib.curRoute = i;//
            s_stRouteAttrib.nodeAddr = addr + STROUTE_HEAD_SIZE;
            break;
        }        
    }
    ////////////////////////////////
    if(check)//找到线路
    {
        s_stRouteAttrib.curNode = 0;
        Route_GoNextStatusAtOnce(ROUTE_FIND_ROAD_SEGMENT);
    }
    else//未找到任何线路
    if(flag)//要求出线路报警给平台
    {
        if(0==Io_ReadAlarmBit(ALARM_BIT_LOAD_EXCURSION)) //如果已设置偏离路线报警，则不重复。防止调用导致重复上传报警信息。
        {
            Io_WriteAlarmBit(ALARM_BIT_LOAD_EXCURSION,SET); //如果有路线，当又未在任何线路内，则设置偏离路线报警
        }
    }
    else
    {
         Io_WriteAlarmBit(ALARM_BIT_LOAD_EXCURSION,RESET);
    }
}
/*************************************************************
** 函数名称: Route_FindRoadSegmentTask
** 功能描述: 查找当前属于那个路段
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Route_FindRoadSegmentTask(void)
{
    unsigned char i;
    //检测线路是否在设置的可用时间段内.不在则重新查找路线。
    if(!Route_CheckTimeEx()) return;
    ///////////////////////////
    for(i=0 ;s_stRouteAttrib.curNode < (s_stRouteAttrib.nodeSum-1) && i<ONE_TIME_READ_NUM; s_stRouteAttrib.curNode++,i++)//节点存储是从0开始,例如有11个节点，则存储是0~10.当因为路段构成两个节点，所以9~10构成最后一条路段了。
    {  
        //读取当前节点信息和下一点的GPS信息
        if(Route_GetNodeDataAndCheckInRoute(s_stRouteAttrib.curNode))
        {
            Route_GoNextStatusAtOnce(ROUTE_CHECK_IN_ROAD_SEGMENT); //轮循节点完毕后,当前点不在任何路段内.查询下一条线路
            return;
        }
    }
    //////////////////////////////
    if(s_stRouteAttrib.curNode >= (s_stRouteAttrib.nodeSum-1))    
    {
        Route_GoNextStatus(ROUTE_FIND_STATUS); //轮循节点完毕后,当前点不在任何路段内.查询下一条线路。
    }
    else//加快搜索
    {
        LZM_PublicSetOnceTimer(&s_stRouteTimer[ROUTE_TIMER_TEST],LZM_AT_ONCE,Route_CheckScanTask);
    }
}
/*************************************************************
** 函数名称: Route_FindRoadSegmentTask
** 功能描述: 查找当前属于那个路段
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Route_CheckInRoadSegmentTask(void)
{
    if(Route_CheckIsInRouteEx())
    {
        Route_DisposeInOutAlarm(IN_ROUTE);                        //进线路报警。
        s_stRouteAttrib.startdriverTime = RTC_GetCounter();       //行驶时间,记录开始时间。
        Route_SetSpeedParam();
        Route_GoNextStatus(ROUTE_HOLD_ROAD_SEGMENT);                    //找到行驶线路后
    }
    else//继续轮循,查找路段 
    {
        Route_GoNextStatus(ROUTE_CHECK_IN_ROAD_SEGMENT);
    }
}
/*************************************************************
** 函数名称: Route_CheckNextRoadSegment
** 功能描述: 查找当前属于那个路段
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Route_GoToChangeRoadSegment(unsigned short NextNode)
{
    s_stRouteAttrib.curNode = NextNode;
    Route_GoNextStatusAtOnce(ROUTE_CHANGE_ROAD_SEGMENT);
}
/*************************************************************
** 函数名称: Route_CheckNextRoadSegment
** 功能描述: 查找当前属于那个路段
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Route_CheckNextRoadSegment(unsigned short NextNode)
{
    //判断当前GPS点是否在下一路线段中。
    if(Route_GetNodeDataAndCheckInRoute(NextNode))
    {
        //需要进一步判断是否进入下一条路段
        Route_GoToChangeRoadSegment(NextNode);
    }                                
    else//需要进一步判断是否离开路线
    {  
        s_stRouteAttrib.curNode = 0;
        Route_GoNextStatusAtOnce(ROUTE_CHECK_OUT_ROUTE);
    } 
}
/*************************************************************
** 函数名称: Route_FindRoadSegmentTask
** 功能描述: 查找当前属于那个路段
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Route_HoldRoadSegmentTask(void)
{
    //检测线路是否在设置的可用时间段内.不在则重新查找路线。
    if(!Route_CheckTimeEx()) return;
    //////////////
    if(!Route_CheckIsInRouteEx())
    {
        if(0 == s_stRouteAttrib.curNode)//线路首段,正向查找路段                               
        {
            Route_CheckNextRoadSegment(s_stRouteAttrib.curNode+1);
        }
        else //线路末段,反向查找路段
        if(s_stRouteAttrib.curNode == s_stRouteAttrib.nodeSum-2)
        {
            Route_CheckNextRoadSegment(s_stRouteAttrib.curNode-1);                                                                                                                            
        }
        else//线路中间路段
        {
             //正向查找路段
            if(Route_GetNodeDataAndCheckInRoute(s_stRouteAttrib.curNode+1))
            {
                Route_GoToChangeRoadSegment(s_stRouteAttrib.curNode+1);
            } 
            else //如果正向查找不到，就反向查找路段.
            {
                Route_CheckNextRoadSegment(s_stRouteAttrib.curNode-1); 
            }                                                           
        }
    }
}
/*************************************************************
** 函数名称: Route_CheckChangeRoadSegmentTask
** 功能描述: 检查是否改变了路段
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Route_CheckChangeRoadSegmentTask(void)
{
    if(Route_CheckIsInRouteEx())
    {
        Route_CheckDriverTime();
        Route_SetSpeedParam();
        Route_GoNextStatus(ROUTE_HOLD_ROAD_SEGMENT);//回到，上阶段的路线节点检测
    }
    else
    {
        s_stRouteAttrib.startdriverTime = RTC_GetCounter();
        Route_SetSpeedParam();
        Route_GoNextStatus(ROUTE_HOLD_ROAD_SEGMENT);//如果所画路段过短，以至于在3秒确认点在线路中时间都不够，这里设置=2出现问题，设置等于0重新检测线路。
    }
}
/*************************************************************
** 函数名称: Route_CheckOutRouteTask
** 功能描述: 检查是否出了线路
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Route_CheckOutRouteTask(void)
{
    unsigned char i;
    for(i=0 ;s_stRouteAttrib.curNode < (s_stRouteAttrib.nodeSum-1) &&i<ONE_TIME_READ_NUM; s_stRouteAttrib.curNode++,i++)
    {
        //读取当前节点信息和下一点的GPS信息
        if(Route_GetNodeDataAndCheckInRoute(s_stRouteAttrib.curNode))
        {
            Route_GoNextStatusAtOnce(ROUTE_CHANGE_ROAD_SEGMENT);
            return;
        }
    }
    ///////////////////////////////////
    if(s_stRouteAttrib.curNode >= (s_stRouteAttrib.nodeSum-1))                      //未搜索到点在任何路段中，确认离开路线。
    {
        Route_DisposeInOutAlarm(OUT_ROUTE);
        Route_GoNextStatus(ROUTE_FIND_STATUS);
    }
    else//加快搜索
    {
        LZM_PublicSetOnceTimer(&s_stRouteTimer[ROUTE_TIMER_TEST],LZM_AT_ONCE,Route_CheckScanTask);
    }
}
/*************************************************************
** 函数名称: Route_SetDriverTimeAlarm
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Route_SetDriverTimeAlarm(unsigned char flag,unsigned short drivertime)
{
    s_stRouteAlarm.roadSegID    = s_stRouteAttrib.stNode.roadSegID;
    s_stRouteAlarm.driverTime   = drivertime;
    s_stRouteAlarm.driverResult = flag;//行驶的太长 
    Io_WriteAlarmBit(ALARM_BIT_LOAD_OVER_DRIVE,SET);     //行使时间过长或过短报警
}
/*************************************************************
** 函数名称: Route_CheckDriverTime
** 功能描述: 检测行驶时长,是否满足要求
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Route_CheckDriverTime(void)
{
    unsigned long curtime;
    unsigned short drivertime;             //记录行驶时间(单位秒)  
    curtime = RTC_GetCounter();
    if(PUBLIC_CHECKBIT(s_stRouteAttrib.stNode.roadAttrib,E_ROUTE_MOMENT))              //有行驶时限限制               
    {
        if(curtime>s_stRouteAttrib.startdriverTime)
        {
            drivertime = curtime-s_stRouteAttrib.startdriverTime;
        }
        else
        {
            drivertime = 0;
        }
        //////////////////////
        if(drivertime > s_stRouteAttrib.stNode.longLimitTime) 
        {                 
            Route_SetDriverTimeAlarm(1,drivertime);//行驶过长
        }
        else
        if(drivertime < s_stRouteAttrib.stNode.shortLimitTime) 
        {
            Route_SetDriverTimeAlarm(0,drivertime);////行驶过短
        }
    }
    else
    {
        Io_WriteAlarmBit(ALARM_BIT_LOAD_OVER_DRIVE,RESET);//无该属性,需要清除该标志.
    }
    ////行驶时间清/////////////////////////////////
    s_stRouteAttrib.startdriverTime = curtime;
}
/************************************************************
函数名称: 线路报警
功能描述: 
入口参数: RouteAlarmItem:线路报警
出口参数:
************************************************************/ 
void Route_DisposeInOutAlarm(E_INOUTROUTE InOutAlarm) 
{
    s_stRouteAlarm.Orientation      = InOutAlarm;   //进出类型
    s_stRouteAlarm.INOUTPostType    = E_ROUTE;      //线路
    s_stRouteAlarm.INOUTTypeID      = s_stRouteAttrib.routeID;//线路ID
    switch(InOutAlarm)
    {
        case IN_ROUTE:
            ////////////////////////////////////////
            Io_WriteAlarmBit(ALARM_BIT_LOAD_EXCURSION,RESET);     //清除偏离路线报警 
            /////////////////////////////////////////
            if(PUBLIC_CHECKBIT(s_stRouteAttrib.routeAttrib,E_INROUTE_TO_DRIVER))     //如果进去报警给司机，则使用语音播报
            {
                //PlayMulTTSVoiceStr("您已进入设定路线!");
                Public_PlayTTSVoiceStr("您已进入设定路线!");//dxl,2014.8.8根据黎工提交修改
            }
            if(PUBLIC_CHECKBIT(s_stRouteAttrib.routeAttrib,E_INROUTE_TO_SERVER))     //如果线路属性:进线路报警，则此时就立即报警。
            {
                Io_WriteAlarmBit(ALARM_BIT_IN_OUT_ROAD,SET);      //标识进线路报警
            }
            else
            {
                Io_WriteAlarmBit(ALARM_BIT_IN_OUT_ROAD,RESET);   //如果无进路线报警属性,当是上次如果有出路线报警则需要清除。 
            }
            break; 
        case OUT_ROUTE:
            s_stRouteAttrib.needplayflag = 0;
            if(PUBLIC_CHECKBIT(s_stRouteAttrib.routeAttrib,E_OUTROUTE_TO_DRIVER)) 
            {
                s_stRouteAttrib.needplayflag = 1;
            }
            ////////////////////////////////////
            if(PUBLIC_CHECKBIT(s_stRouteAttrib.routeAttrib,E_OUTROUTE_TO_SERVER))    //如果线路属性:出线路报警，则此时就立即报警。
            {
                Io_WriteAlarmBit(ALARM_BIT_LOAD_EXCURSION,SET);      //路线偏离报警
                Io_WriteAlarmBit(ALARM_BIT_IN_OUT_ROAD,SET);      //标识出线路报警    
            }
            else
            {
                Io_WriteAlarmBit(ALARM_BIT_IN_OUT_ROAD,RESET);    //标识进线路报警 
            } 
            Route_ClearSpeedParam();//出路线，请超速报警。
            break; 
        default:
            break;
    }
}
/*********************************************************************
//功能:获取GPS数据
//输入:
//输出:数据长度
//备注:
*********************************************************************/
void Route_GetCurGpsCoordinate(void)
{
    unsigned char buffer[28];
    Public_GetCurBasicPositionInfo(buffer);
    Public_Mymemcpy((unsigned char *)&s_stRouteAttrib.Latitude,(unsigned char *)&buffer[8],sizeof(unsigned long),1); 
    Public_Mymemcpy((unsigned char *)&s_stRouteAttrib.Longitude,(unsigned char *)&buffer[12],sizeof(unsigned long),1); 
}
/*******************************************************************************
** 函数名称: Route_GetInOutAlarmSubjoinInfo
** 功能描述: 获取进出路线报警附加信息包(包括附加信息ID、长度、附加信息体) :即附加信息ID为0x12的所有内容
** 入口参数: 需要保存进出路线报警附加信息包的首地址
** 出口参数: 
** 返回参数: 进出路线报警附加信息包的总长度,0:表示未进出路线,
*******************************************************************************/
unsigned char Route_GetInOutAlarmSubjoinInfo(unsigned char *data) 
{
    if(!Io_ReadAlarmBit(ALARM_BIT_IN_OUT_ROAD))return 0;
    unsigned char len; 
    unsigned char buffer[20]={0};
    len =0;
    buffer[len++]= IN_OUT_ROUTE_OVERHEAD_INFO_ID;
    buffer[len++]= IN_OUT_ROUTE_OVERHEAD_INFO_LEN;
    buffer[len++]= s_stRouteAlarm.INOUTPostType;
    Public_ConvertLongToBuffer(s_stRouteAlarm.INOUTTypeID,&buffer[len]);
    len += 4;
    buffer[len++]= s_stRouteAlarm.Orientation;
    memcpy(data,buffer,len);
    return len;
}

/*************************************************************
** 函数名称: Route_GetDriveAlarmInfoSubjoinInfo
** 功能描述: 获取路段行驶时间不足/过长报警附加信息包(包括附加信息ID、长度、附加信息体) :即附加信息ID为0x13的所有内容
** 入口参数: 需要保存行驶时间报警附加信息包的首地址
** 出口参数: 
** 返回参数: 行驶时间报警附加信息包的总长度,0:表示未报警,
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Route_GetDriveAlarmInfoSubjoinInfo(unsigned char *data) 
{
    if(!Io_ReadAlarmBit(ALARM_BIT_LOAD_OVER_DRIVE))return 0;
    unsigned char len; 
    unsigned char buffer[20]={0};
    len =0;
    buffer[len++]= LOAD_OVER_DRIVE_OVERHEAD_INFO_ID;
    buffer[len++]= LOAD_OVER_DRIVE_OVERHEAD_INFO_LEN;
    Public_ConvertLongToBuffer(s_stRouteAlarm.roadSegID,&buffer[len]);
    len += 4;
    Public_ConvertShortToBuffer(s_stRouteAlarm.driverTime,&buffer[len]);
    len += 2;
    buffer[len++]= s_stRouteAlarm.driverResult;
    memcpy(data,buffer,len);
    return len;
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
void Route_CheckForResetParam(unsigned long RouteID) 
{
    if(s_stRouteAttrib.routeID!=RouteID)return;
    Route_ResetParam();
}
/*************************************************************
** 函数名称: Route_DisposeSetRoad
** 功能描述: :设置路线
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Route_DisposeSetRoad(unsigned char flag,unsigned char *pBuffer, unsigned short BufferLen)
{
    static T_GPS_POS MinLeftUpPos;   //取经纬度最小值为，矩形一点坐标.
    static T_GPS_POS MaxLeftUpPos;   //取经纬度最大值为矩形另一点坐标
    static u16 MaxWidth = 0;         //最大路宽    
    static u16 CurRoute  = 0;        //保存当前线路在Flash中的编号,不适用gRoutePcb.CurRoute是因为可能当前路线检测的时候正在适用，而如果路线时多包发送这里可能出现问题。
    static u16 NodeNum  = 0;         //保存当前线路在Flash中的节点编号
    static short NodeCount = 0;        //节点计数
    STROUTE_HEAD stRoute,stTmpRoute; //读取线路数据
    ST_ROUTE_NODE stNodeData;
    unsigned short datalen;
    u32 ExpLatLog = 0;               //路宽=等于扩大的经纬度值
    u32	addr,tmpAddr;        
    u8 *pTmp;               //下发协议数据项指针
    u8 i;               //
    ///////////////////   
    pTmp =pBuffer;
    datalen = 0;
    //////////////////
    if(flag<2)
    {
        CurRoute = 0;
        NodeNum  = 0;
        MaxWidth = 0;        
        NodeCount = 0;
        if(BufferLen<8)return 2;//消息有误
        memset(&stRoute,0,STROUTE_HEAD_SIZE);                                   //清空为0，同时左上角，右下角坐标为0.
        memset(&MinLeftUpPos,0,sizeof(T_GPS_POS));                              //设置最小值和最大值为0.
        memset(&MaxLeftUpPos,0,sizeof(T_GPS_POS));
        /////////////////////
        stRoute.routeID = Public_ConvertBufferToLong(&pTmp[datalen]);
        datalen += 4;
        stRoute.routeAttrib =Public_ConvertBufferToShort(&pTmp[datalen]);
        datalen += 2;
        //////////////////////
        if(PUBLIC_CHECKBIT(stRoute.routeAttrib, 0))//决定是否包含时间信息
        {
            if(BufferLen<20) return 2;//消息有误
            memcpy(stRoute.startTime,&pTmp[datalen],6);
            datalen += 6;
            memcpy(stRoute.endTime,&pTmp[datalen],6);
            datalen += 6;
        }
        stRoute.nodeSum = Public_ConvertBufferToShort(&pTmp[datalen]);          
        datalen += 2; 
        NodeCount = stRoute.nodeSum;//记录节点总数
        if(NodeCount > ROUTE_NODE_SUM||0==NodeCount)return 1;//如果节点总数大于最大节点数则返回失败。
        ///////////////////////
        if(0==flag)//未分包的情况
        {
            if(BufferLen < datalen+NodeCount*18)return 2;//消息有误
        }
        ///把线路信息写入Flash////////////
        addr = 0;        
        for(i =0; i< s_stRouteAttrib.routeSum; i++)//判断设置线路ID是否已存在
        {
            tmpAddr=ROUTE_ADDR_START + i*ONE_ROUTE_AREA_SIZE;  
            sFLASH_ReadBuffer((u8 *)&stTmpRoute,tmpAddr,STROUTE_HEAD_SIZE); //读出线路数据	
            if(stTmpRoute.routeID==stRoute.routeID)
            {
                CurRoute = i;
                addr = tmpAddr;
                break;
            }
            else
            if(0==stTmpRoute.routeID)
            {
                CurRoute = i;
                addr = tmpAddr;
            }
        }
        /////////////////////////
        if(0==addr)//
        {
            if(s_stRouteAttrib.routeSum >= ROUTE_NUM_MAX)return 1;//如果超过最大值,返回失败                      
            CurRoute = s_stRouteAttrib.routeSum;
            addr = ROUTE_ADDR_START + CurRoute*ONE_ROUTE_AREA_SIZE;
            s_stRouteAttrib.routeSum++;
        }
        /////////////////////
        stRoute.Head = ROUTE_HEAD_FLAG;
        Area_MdyDataInFlash((u8*)&stRoute,addr,STROUTE_HEAD_SIZE);  
        Route_CheckForResetParam(stRoute.routeID);
    }
    ///////////////////////
    do
    {
        if(datalen+18 > BufferLen)return 2;//消息有误
        NodeCount--;//节点总数-1
        //获得节点信息
        stNodeData.NodeID= Public_ConvertBufferToLong(&pTmp[datalen]);
        datalen += 4;
        stNodeData.roadSegID = Public_ConvertBufferToLong(&pTmp[datalen]);
        datalen += 4;
        stNodeData.point.Lat  = Public_ConvertBufferToLong(&pTmp[datalen]);
        datalen += 4;
        stNodeData.point.Long = Public_ConvertBufferToLong(&pTmp[datalen]);
        datalen += 4;
        ////////
        stNodeData.roadWidth = pTmp[datalen++];
        stNodeData.roadAttrib = pTmp[datalen++];
        if(PUBLIC_CHECKBIT(stNodeData.roadAttrib, 0))//行驶时间
        {
            stNodeData.longLimitTime = Public_ConvertBufferToShort(&pTmp[datalen]);
            datalen += 2;
            stNodeData.shortLimitTime = Public_ConvertBufferToShort(&pTmp[datalen]);
            datalen += 2;
        }
        /////////////////
        if(PUBLIC_CHECKBIT(stNodeData.roadAttrib, 1))//限速
        {
            stNodeData.limitSpeed = Public_ConvertBufferToShort(&pTmp[datalen]);
            datalen += 2;
            stNodeData.speedDuration = pTmp[datalen++];
        }
        ///////////////////////
        //构成线路的大矩形
        if(MinLeftUpPos.Lat == 0)                             //如果为0,保存第一个经纬度值
        {
            MinLeftUpPos.Lat = stNodeData.point.Lat;
            MaxLeftUpPos.Lat = stNodeData.point.Lat;
        }
        else 
        if(MinLeftUpPos.Lat > stNodeData.point.Lat)  //取最小值
        {
            MinLeftUpPos.Lat = stNodeData.point.Lat;
        }
        else 
        if(MaxLeftUpPos.Lat < stNodeData.point.Lat)  //取最大值
        {
            MaxLeftUpPos.Lat = stNodeData.point.Lat;
        }
        /////////////////////////////
        if(MinLeftUpPos.Long == 0) 
        {
            MinLeftUpPos.Long =  stNodeData.point.Long;
            MaxLeftUpPos.Long =  stNodeData.point.Long;
        }
        else 
        if(MinLeftUpPos.Long > stNodeData.point.Long)
        {
            MinLeftUpPos.Long =  stNodeData.point.Long;
        }
        else 
        if(MaxLeftUpPos.Long < stNodeData.point.Long)
        {
            MaxLeftUpPos.Long =  stNodeData.point.Long;
        }
        ////////////////////
        if(MaxWidth == 0)
        {
            MaxWidth = stNodeData.roadWidth;
        }
        else 
        if(MaxWidth < stNodeData.roadWidth)                                     //求路宽的最大值
        {
            MaxWidth =  stNodeData.roadWidth;
        }
        ////////////////////////
        //把节点信息写入Flash
        addr=ROUTE_ADDR_START + CurRoute*ONE_ROUTE_AREA_SIZE + STROUTE_HEAD_SIZE+NodeNum*STROUTE_NODE_SIZE; //存放节点的地址   
        Area_MdyDataInFlash((u8*)&stNodeData,addr,STROUTE_NODE_SIZE);
        NodeNum++;
    }
    while(datalen<BufferLen);
    //如果=0,说明收到分包总所有节点信息
    if(NodeCount == 0)
    {
        addr = ROUTE_ADDR_START + CurRoute*ONE_ROUTE_AREA_SIZE ;  
        sFLASH_ReadBuffer((u8 *)&stRoute,addr,STROUTE_HEAD_SIZE); //读出线路数据.
        //ExpLatLog = MaxWidth/30.9 *(0.0003*1000000)(30.9米/秒=30.9约为31,1秒=1/60/60约为0.000278度,这里约为0.0003.) 
        ExpLatLog = (MaxWidth*100*3)/31;                                     //这里用，经纬度弄一样是有误差的，纬度还好，经度是用了赤道那个位置最大值来算距离。
        MinLeftUpPos.Lat -= ExpLatLog;                                       //按路宽扩大矩形,防止可能在转弯时，有些路宽上的点不在矩形上。
        MinLeftUpPos.Long -= ExpLatLog;
        MaxLeftUpPos.Lat += ExpLatLog;
        MaxLeftUpPos.Long += ExpLatLog;
        ///////////////////////////////
        stRoute.LeftUpPos = MinLeftUpPos;                             //保存一个线路最大和最小点构成的大矩形，用于算法优化。
        stRoute.RightDowPos = MaxLeftUpPos;
        stRoute.Head = ROUTE_HEAD_FLAG;
        Area_MdyDataInFlash((u8*)&stRoute,addr,STROUTE_HEAD_SIZE);     //修改线路属性.        
        Route_CheckForResetParam(stRoute.routeID);
        s_stRouteAttrib.preSpeed = 0xff;
    }
    return 0;               //返回值
}
/*************************************************************
** 函数名称: Route_DisposeSetRoad
** 功能描述: :删除路线
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Route_DisposeDelRoad(unsigned char *pBuffer, unsigned short BufferLen)
{
    u8	i,j,flag;
    u32	OffSetAddr,RouteID;
    u8 *pTmp;
    u8 Sum;    
    STROUTE_HEAD stTmpRoute; //读取线路数据
    
    Sum = *pBuffer++;              //删除路线总数    
    if(Sum)
    {
        pTmp =pBuffer;
        for(i=0; i<Sum; i++)
        {
            Public_Mymemcpy((unsigned char *)&RouteID,(unsigned char *)&pTmp[0],sizeof(unsigned long),1); //获得路线ID
            pTmp+=sizeof(unsigned long);
            flag =0;
            for(j=0; j<s_stRouteAttrib.routeSum; j++)
            {
                OffSetAddr=ROUTE_ADDR_START + j*ONE_ROUTE_AREA_SIZE ;  
                sFLASH_ReadBuffer((u8 *)&stTmpRoute,OffSetAddr,STROUTE_HEAD_SIZE);//读出线路数据	
                if(RouteID==stTmpRoute.routeID)
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
            Public_Mymemcpy((unsigned char *)&RouteID,(unsigned char *)&pTmp[0],sizeof(unsigned long),1); //获得路线ID
            pTmp+=sizeof(unsigned long);
            flag =0;
            for(j=0; j<s_stRouteAttrib.routeSum; j++)
            {
                OffSetAddr=ROUTE_ADDR_START + j*ONE_ROUTE_AREA_SIZE ;
                sFLASH_ReadBuffer((u8 *)&stTmpRoute,OffSetAddr,STROUTE_HEAD_SIZE);//读出线路数据	
                if(RouteID==stTmpRoute.routeID)
                {
                    stTmpRoute.routeID = 0;
                    Area_MdyDataInFlash((u8*)&stTmpRoute,OffSetAddr,STROUTE_HEAD_SIZE);
                }
            }
            Route_CheckForResetParam(RouteID);
        }          
    }
    else
    {                                              //总数=0，删除所有线路
        ClrRoute();    
    }                  
    //只有应答
    return 0;
}
/********************************************************************
* 功能 : Route_CheckScanTask
* 输入: 
* 输出: 
* 全局变量: 
*
* 备注: 
********************************************************************/  
void Route_CheckScanTask(void)
{
    switch(s_stRouteAttrib.status)
    {
        case ROUTE_FIND_STATUS:
            Route_FindRouteAreaTask();
            break;
        case ROUTE_FIND_ROAD_SEGMENT:
            Route_FindRoadSegmentTask();
            break;
        case ROUTE_CHECK_IN_ROAD_SEGMENT:
            Route_CheckInRoadSegmentTask();
            break;
        case ROUTE_HOLD_ROAD_SEGMENT:
            Route_HoldRoadSegmentTask();
            break;
        case ROUTE_CHANGE_ROAD_SEGMENT:
            Route_CheckChangeRoadSegmentTask();
            break;
        case ROUTE_CHECK_OUT_ROUTE:
            Route_CheckOutRouteTask();
            break;
    }
}
/*************************************************************
** 函数名称: Route_PlayRoadExcursionAlarmVoice
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void Route_PlayRoadExcursionAlarmVoice(void)
{
    if(!Public_PlayE2ParamAlarmVoice(E2_ROUTE_EXCURSION_ALARM_VOICE_ID))
    {
        Public_PlayTTSVoiceAlarmStr("请按规定线路行驶");
    }
}
/*************************************************************
** 函数名称: Route_CheckAlarmVoiceTask
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void Route_CheckAlarmVoiceTask(void)
{
    unsigned char flag;
    flag = 0;
    if(s_stRouteAttrib.needplayflag&&Io_ReadAlarmBit(ALARM_BIT_LOAD_EXCURSION))
    {
        if(SpeedMonitor_GetCurRunStatus())
        flag = 1;
    }    
    ////////////
    if(SpeedMonitor_CheckPlayFlag(flag,&s_stRouteAttrib.stPlay))
    {
        Route_PlayRoadExcursionAlarmVoice();
    }
}
/********************************************************************
* 功能 : 检查车辆是否在线路上
* 输入: 
* 输出: 
* 全局变量: 
*
* 备注:每1s执行一次
********************************************************************/  
void Route_CheckGpsDataChange(void)
{
    unsigned char speed;
    GPS_STRUCT stTmpGps;
    Gps_CopygPosition(&stTmpGps);    
    if(stTmpGps.Second != s_stRouteAttrib.preSecond)
    {
        //读进出路线报警屏蔽位状态
        if(Io_ReadAlarmMaskBit(ALARM_BIT_IN_OUT_ROAD))
        {
            if(1==s_stRouteAttrib.nonavigationCnt)
            {
                s_stRouteAttrib.nonavigationCnt =0;
                Route_ClearAlarm();
            }
            return;
        }
        s_stRouteAttrib.nonavigationCnt = 1; 
        speed = Area_GetCurSpeed();
        ///////////////////
        Route_GetCurGpsCoordinate();        
        ///////////////////////////
        if(speed||s_stRouteAttrib.preSpeed)
        {
            LZM_PublicKillTimer(&s_stRouteTimer[ROUTE_TIMER_TEST]);
            Route_CheckScanTask();
        }
        //////////////////////////
        Route_CheckAlarmVoiceTask();
        ///////////////
        s_stRouteAttrib.preSpeed = speed;
        s_stRouteAttrib.preSecond = stTmpGps.Second;
    }
}
/********************************************************************
* 功能 : 检查车辆是否在线路上
* 输入: 
* 输出: 
* 全局变量: 
*
* 备注:每1s执行一次
********************************************************************/  
void Route_ScanTask(void)
{
    if(Io_ReadStatusBit(STATUS_BIT_NAVIGATION))
    {
        Route_CheckGpsDataChange();
    }
    else
    {
        s_stRouteAttrib.stPlay.GroupTimeCnt =0;
        s_stRouteAttrib.stPlay.OnceCnt =0;
        /////////////////
        s_stRouteAttrib.preSecond = 0xff;
        s_stRouteAttrib.preSpeed = 0xff;
        if(s_stRouteAttrib.nonavigationCnt<ROUTE_RESET_STATUS_COUNT)s_stRouteAttrib.nonavigationCnt++;//10s
        else
        if(s_stRouteAttrib.nonavigationCnt == ROUTE_RESET_STATUS_COUNT)
        {
            s_stRouteAttrib.nonavigationCnt ++;
            LZM_PublicKillTimer(&s_stRouteTimer[ROUTE_TIMER_TEST]);
            Route_ClearAlarm();
        }
    }
}
/*************************************************************
** 函数名称: Route_StartScanTask
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Route_StartScanTask(void)
{
   LZM_PublicSetCycTimer(&s_stRouteTimer[ROUTE_TIMER_TASK],PUBLICSECS(0.2),Route_ScanTask);
}
/*************************************************************
** 函数名称: Route_UpdatePram
** 功能描述: 设置或修改线路相关变量后,需要更线路速模块相关变量
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/
void Route_UpdatePram(void)
{
    u8 PramLen;
    u8 Buffer[20];
    ////////////////////
    //从EEPROM读取数据更新变量
    PramLen = EepromPram_ReadPram(E2_ROUTE_EXCURSION_ALARM_NUMBER_ID, Buffer);
    if(PramLen)
    {
        s_stRouteAttrib.stPlay.OnceNumber =Public_ConvertBufferToLong(Buffer);
    }
    else
    {
        s_stRouteAttrib.stPlay.OnceNumber = 3;
    }
    PramLen = EepromPram_ReadPram(E2_ROUTE_EXCURSION_ALARM_GPOUP_TIME_ID, Buffer);
    if(PramLen)
    {
        s_stRouteAttrib.stPlay.GroupTime =Public_ConvertBufferToLong(Buffer)*60;
    }
    else
    {
        s_stRouteAttrib.stPlay.GroupTime = 300;
    }
}
/*************************************************************
** 函数名称: Route_ParameterInitialize
** 功能描述: 线路参数初始化
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Route_ParameterInitialize(void) 
{
    unsigned char PramLen;
    unsigned char Buffer[30];
    Route_ResetParam();
    s_stRouteAttrib.routeSum =0;
    ///////////////////
    s_stRouteAttrib.nonavigationCnt = 0xff;
    ///////////////////////////////
    PramLen = EepromPram_ReadPram(E2_NIGHT_DRIVE_PREALARM_GPOUP_TIME_ID, Buffer);//
    if(PramLen)
    {
        s_stRouteAttrib.stPlay.GroupTime = Public_ConvertBufferToLong(Buffer)*60;
    }
    else
    {
        s_stRouteAttrib.stPlay.GroupTime = 300;//
    }
    PramLen = EepromPram_ReadPram(E2_NIGHT_DRIVE_PREALARM_NUMBER_ID, Buffer);//    
    if(PramLen)
    {
        s_stRouteAttrib.stPlay.OnceNumber = Public_ConvertBufferToLong(Buffer);
    }
    else
    {
        s_stRouteAttrib.stPlay.OnceNumber = 3;//
    }
    ////////////////////////////
    s_stRouteAttrib.minRoadWidth = 30;//最小路宽50,送检时为30
}
/********************************************************************
* 功能 : 初始化线路
* 输入:  
* 输出: 
* 全局变量: 
*
* 备注: 确定线路总数,必要的复位参数
********************************************************************/ 
void Route_ReadAndCheckParam(void) 
{
    u8 i;
    u32	OffSetAddr;
    STROUTE_HEAD  stRouteData;	  //读取线路数据
    //////////////////////
    memset(&s_stRouteAttrib,0,sizeof(s_stRouteAttrib));
    //////////////////////////
    Route_ParameterInitialize();
    //获得线路数
    for(i=0; i<ROUTE_NUM_MAX; i++)
    {
        OffSetAddr=ROUTE_ADDR_START + i*ONE_ROUTE_AREA_SIZE ;  
        sFLASH_ReadBuffer((u8 *)&stRouteData,OffSetAddr,STROUTE_HEAD_SIZE);//读出线路数据
        if(ROUTE_HEAD_FLAG == stRouteData.Head)
        {
            if(0xFFFFFFFF == stRouteData.routeID)
            {
                ClrRoute();
                break;
            }
            else
            {
                s_stRouteAttrib.routeSum++;
            }
        }
        else 
        if(0xFFFF != stRouteData.Head)
        {
            ClrRoute();
            break;
        }
        else 
        if(0xFFFF == stRouteData.Head)
        {
            break;
        }
    }
    //////////////////////////////////////////////////////////
    LZM_PublicSetOnceTimer(&s_stRouteTimer[ROUTE_TIMER_TASK],PUBLICSECS(10),Route_StartScanTask);    
}
/********************************************************************
* 功能 : 初始化线路
* 输入:  
* 输出: 
* 全局变量: 
*
* 备注: 确定线路总数,必要的复位参数
********************************************************************/ 
void InitRoute(void) 
{    
    LZM_PublicKillTimerAll(s_stRouteTimer,ROUTE_TIMER_TASK); 
    ///////////////////
    SetTimerTask(TIME_ROUTE,LZM_TIME_BASE);
    //////////////////////////////////////////////////////////
    LZM_PublicSetOnceTimer(&s_stRouteTimer[ROUTE_TIMER_TASK],PUBLICSECS(20),Route_ReadAndCheckParam);
}

/********************************************************************
* 功能 : 检查车辆是否在线路上
* 输入: 
* 输出: 
* 全局变量: 
*
* 备注:每1s执行一次
********************************************************************/  
FunctionalState Route_TimeTask(void)
{
    LZM_PublicTimerHandler(s_stRouteTimer,ROUTE_TIMERS_MAX);
    return ENABLE;   
}
/*******************************************************************************
 *                             end of module
 *******************************************************************************/


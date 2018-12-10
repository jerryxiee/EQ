/*******************************************************************************
 * File Name:			MuckDump.c 
 * Function Describe:	
 * Relate Module:		渣土倾倒区域功能 
 * Writer:				Joneming
 * Date:				2013-11-29
 * ReWriter:			
 * Date:				
 *******************************************************************************/
#include "include.h"
///////////////////////////
#ifdef ZHATUCHE_VERSION
#define MAX_MUCK_DUMP_AREA_ADDR         0xFFFFFFFF
///////////////////
#define MUCK_DUMP_HEAD_FLAG             0xABCD
#define MUCK_DUMP_HEAD_SIZE             8//读取头标志、区域ID
#define MAX_MUCK_DUMP_AREA_SUM          100  //最多渣土倾倒区域数

//////////////////////////////////////
#define MUCK_DUMP_AREA_START            (unsigned long)(FLASH_MUCK_DUMP_AREA_START_SECTOR*FLASH_ONE_SECTOR_BYTES) //渣土倾倒区域存放点,扇区需要从0开始....
#define MUCK_DUMP_AREA_END              (unsigned long)((FLASH_MUCK_DUMP_AREA_END_SECTOR)*FLASH_ONE_SECTOR_BYTES)

//渣土倾倒区域属性结构	24字节
typedef struct
{
    unsigned short Head;            //头标志    
    unsigned short AreaAttrib;      //区域属性 
    unsigned long AreaID;           //区域ID
    unsigned long LeftUpLat;        //左上点纬度
    unsigned long LeftUpLong;       //左上点经度
    unsigned long RightDownLat;     //右下点纬度
    unsigned long RightDownLong;    //右下点经度
}STMUCK_DUMP_AREA;

#define STMUCK_DUMP_AREA_SIZE sizeof(STMUCK_DUMP_AREA)
//结构
typedef struct
{
    unsigned long Latitude;                     //纬度整型坐标
    unsigned long Longitude;                    //经度整型坐标   
    unsigned long addr;                         //地址//当为MAX_AREA_NUM时,表示没有进入任何区域 
    unsigned long AreaID;                       //区域ID 
    unsigned short AreaAttrib;                  //区域属性
    unsigned short AreaNum;             //渣土倾倒区域总数
    unsigned char preSecond;                    //当前秒数
    unsigned char preSpeed;                     //前一秒的速度
    unsigned char nonavigationCnt;              //不导航计数器
    unsigned char AreaCnt;          //区域计数器
    unsigned char preAreaCnt;       //区域计数器
    unsigned char AreaFlag;         //区域标志
}ST_MUCK_DUMP_ATTRIB;

static ST_MUCK_DUMP_ATTRIB s_stMuckDAttrib;
///////////////////////
enum 
{
    MD_TIMER_TASK,                        // 
    MD_TIMER_TEST,
    MD_TIMERS_MAX
}E_MUCKDUMPTIME;

static LZM_TIMER s_stMDTimer[MD_TIMERS_MAX];

enum 
{
    MD_AREA_STATUS_IN,                      //    
    MD_AREA_STATUS_OUT,						//    
    MD_AREA_STATUS_MAX
}E_MD_AREA_STATUS;

///////////////////////////////////////////

/*************************************************************
** 函数名称: MuckDump_ResetParam
** 功能描述: 渣土倾倒区域重置参数
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void MuckDump_ResetParam(void) 
{
    s_stMuckDAttrib.AreaCnt  = 0;
    s_stMuckDAttrib.addr     = MAX_MUCK_DUMP_AREA_ADDR;
    s_stMuckDAttrib.AreaFlag = MD_AREA_STATUS_MAX;
    ///////////////////////////////
}
/*************************************************************
** 函数名称: MuckDump_CheckForResetParam
** 功能描述: 渣土倾倒区域检查是否重置参数
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void MuckDump_CheckForResetParam(unsigned long AreaID) 
{
    if(s_stMuckDAttrib.AreaFlag==MD_AREA_STATUS_IN&&s_stMuckDAttrib.AreaID==AreaID)
    {
        MuckDump_ResetParam();
    }
}
/*************************************************************
** 函数名称: MuckDump_ParamInit
** 功能描述: 渣土倾倒区域参数初始化
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void MuckDump_ParamInit(void) 
{
    MuckDump_ResetParam();
    s_stMuckDAttrib.AreaNum = 0;
}
/*************************************************************
** 函数名称: MuckDump_ClearArea
** 功能描述: 清除渣土倾倒区域
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void MuckDump_ClearArea(void)
{
    MuckDump_ParamInit();
    Area_EraseFlash(FLASH_MUCK_DUMP_AREA_START_SECTOR,FLASH_MUCK_DUMP_AREA_END_SECTOR); //擦除区域
}

/*************************************************************
** 函数名称: MuckDump_GetCurGpsCoordinate
** 功能描述: 获得当前坐标
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void MuckDump_GetCurGpsCoordinate(void)
{  
    unsigned char buffer[28];
    Public_GetCurBasicPositionInfo(buffer);
    Public_Mymemcpy((unsigned char *)&s_stMuckDAttrib.Latitude,(unsigned char *)&buffer[8],4,1); 
    Public_Mymemcpy((unsigned char *)&s_stMuckDAttrib.Longitude,(unsigned char *)&buffer[12],4,1); 
    /////////////////////  
}
//-----------------------------------------------
//功能:判断当前坐标点是否在Ptr所指的渣土倾倒区域内
//输入:指向区域的指针
//输出:0:在  1:不在  0XFF:出错
//备注:
//-----------------------------------------------
unsigned char MuckDump_IsInArea(STMUCK_DUMP_AREA *Ptr)
{
    ///////////////////////////
    return( ComparePos( (T_GPS_POS *)&(Ptr->LeftUpLat), (T_GPS_POS *)&(Ptr->RightDownLat),(T_GPS_POS *)&s_stMuckDAttrib.Latitude));
}
//-----------------------------------------------
//功能:扫描渣土倾倒区域
//输入:
//输出:
//备注:每秒执行一次
//-----------------------------------------------
void MuckDump_ScanArea(void)
{
    unsigned char i;
    unsigned long addr;
    STMUCK_DUMP_AREA stMuckDumpArea;	  //读取渣土倾倒区域属性
    if(s_stMuckDAttrib.AreaFlag != MD_AREA_STATUS_IN)
    {
        if(s_stMuckDAttrib.addr == MAX_MUCK_DUMP_AREA_ADDR)
        {
            for(i=0; s_stMuckDAttrib.AreaCnt<s_stMuckDAttrib.AreaNum && i<10; s_stMuckDAttrib.AreaCnt++,i++)
            {
                addr=MUCK_DUMP_AREA_START + s_stMuckDAttrib.AreaCnt*STMUCK_DUMP_AREA_SIZE ;  
                sFLASH_ReadBuffer((unsigned char *)&stMuckDumpArea,addr,STMUCK_DUMP_AREA_SIZE);//读出区域数据	
                if(stMuckDumpArea.AreaID ==0) //跳过ID=0
                    continue;
                if(0==MuckDump_IsInArea(&stMuckDumpArea))
                {//进入区域
                    s_stMuckDAttrib.addr     = addr;                    
                    s_stMuckDAttrib.AreaID   = stMuckDumpArea.AreaID;
                    s_stMuckDAttrib.AreaFlag = MD_AREA_STATUS_IN;
                    break;
                }
            }            
            if(s_stMuckDAttrib.AreaCnt>=s_stMuckDAttrib.AreaNum)s_stMuckDAttrib.AreaCnt = 0;  //重新检索
            ////////////////////////
            if((MAX_MUCK_DUMP_AREA_ADDR == s_stMuckDAttrib.addr) && (s_stMuckDAttrib.preAreaCnt != s_stMuckDAttrib.AreaCnt))LZM_PublicSetOnceTimer(&s_stMDTimer[MD_TIMER_TEST],PUBLICSECS(0.2),MuckDump_ScanArea);
            //////////////////////////
        }
    }
    else
    if(MD_AREA_STATUS_IN == s_stMuckDAttrib.AreaFlag)
    {
        sFLASH_ReadBuffer((unsigned char *)&stMuckDumpArea,s_stMuckDAttrib.addr,STMUCK_DUMP_AREA_SIZE);//读出区域数据
        if(MuckDump_IsInArea(&stMuckDumpArea))
        {
            s_stMuckDAttrib.AreaFlag = MD_AREA_STATUS_OUT; 
            s_stMuckDAttrib.addr     = MAX_MUCK_DUMP_AREA_ADDR;
        }
    }
}
/*************************************************************
** 函数名称: MuckDump_GetAreaDatalen
** 功能描述: :设置区域时,标准协议长度
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned short MuckDump_GetAreaDatalen(unsigned char *pBuffer, unsigned short BufferLen,unsigned long *AreaID)
{
    unsigned short datalen;
    STMUCK_DUMP_AREA stMuckDumpArea;  //读取渣土倾倒区域属性
    datalen =0;
    //获得区域ID
    Public_Mymemcpy((unsigned char *)&stMuckDumpArea.AreaID,(unsigned char *)&pBuffer[datalen],sizeof(unsigned long),1); 
    datalen+=sizeof(unsigned long);
    //获得区域属性
    Public_Mymemcpy((unsigned char *)&stMuckDumpArea.AreaAttrib,(unsigned char *)&pBuffer[datalen],sizeof(unsigned short ),1); 
    datalen+=sizeof(unsigned short ); 
     //获得左上点纬度
    datalen+=sizeof(unsigned long);  
    //获得左上点经度
    datalen+=sizeof(unsigned long);  
    
    //获得右下点纬度
    datalen+=sizeof(unsigned long);  
    //获得右下点经度
    datalen+=sizeof(unsigned long);      
    ///////////////////
    if(datalen>BufferLen)return 0;
    ///////////////////////
    *AreaID =stMuckDumpArea.AreaID;
    return datalen;
}
/*************************************************************
** 函数名称: MuckDump_SetAreaCheckDatalen
** 功能描述: :设置区域时,查检长度是否正确
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char MuckDump_SetAreaCheckDatalen(unsigned char *pBuffer, unsigned short BufferLen)
{
    unsigned long AreaID;
    unsigned char i,Sum;
    unsigned short datalen,templen;
    unsigned char *pTmp;
    //////////////    
    if(BufferLen==0)return 0;
    ////////////////////////    
    Sum=*pBuffer++;
    ////////////////////
    datalen=BufferLen-1;
    pTmp=pBuffer;
    ////////////////////
    for(i=0; i<Sum; i++)
    {
        templen=MuckDump_GetAreaDatalen(pTmp,datalen,&AreaID);
        if(0 == templen)//出错
        {
            break;
        }
        pTmp += templen;
        //////////////////////////
        if(datalen == templen)//相等,正确
        {
            return 1;
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
   
    return 0;
}
/*********************************************************************
//函数名称	:MuckDump_SaveOneArea
//功能		:设置渣土倾倒区域
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:
*********************************************************************/
unsigned short MuckDump_SaveOneArea(unsigned long addr,unsigned char *pBuffer, unsigned short BufferLen)
{
    unsigned short datalen;
    STMUCK_DUMP_AREA stMuckDumpArea;  //读取渣土倾倒区域属性    
    if(addr+STMUCK_DUMP_AREA_SIZE>MUCK_DUMP_AREA_END)return 0;
    memset(&stMuckDumpArea,0,sizeof(STMUCK_DUMP_AREA));
    datalen = 0;
    Public_Mymemcpy((unsigned char *)&stMuckDumpArea.AreaID,(unsigned char *)&pBuffer[datalen],sizeof(unsigned long),1); //获得区域ID
    datalen+=sizeof(unsigned long);
    Public_Mymemcpy((unsigned char *)&stMuckDumpArea.AreaAttrib,(unsigned char *)&pBuffer[datalen],sizeof(unsigned short ),1); //获得区域属性
    datalen+=sizeof(unsigned short ); 
    Public_Mymemcpy((unsigned char *)&stMuckDumpArea.LeftUpLat,(unsigned char *)&pBuffer[datalen],sizeof(unsigned long),1); //获得左上点纬度
    datalen+=sizeof(unsigned long);  
    Public_Mymemcpy((unsigned char *)&stMuckDumpArea.LeftUpLong,(unsigned char *)&pBuffer[datalen],sizeof(unsigned long),1); //获得左上点经度
    datalen+=sizeof(unsigned long);  
    
    Public_Mymemcpy((unsigned char *)&stMuckDumpArea.RightDownLat,(unsigned char *)&pBuffer[datalen],sizeof(unsigned long),1); //获得右下点纬度
    datalen+=sizeof(unsigned long);  
    Public_Mymemcpy((unsigned char *)&stMuckDumpArea.RightDownLong,(unsigned char *)&pBuffer[datalen],sizeof(unsigned long),1); //获得右下点经度
    datalen+=sizeof(unsigned long); 
    
    //////////////////////////
    if(datalen>BufferLen)return 0;
    //////////////////////
    stMuckDumpArea.Head = MUCK_DUMP_HEAD_FLAG;
    ///////////////////////////
    Area_MdyDataInFlash((unsigned char*)&stMuckDumpArea,addr,STMUCK_DUMP_AREA_SIZE);
    return datalen;
}

/*********************************************************************
//函数名称	:MuckDump_UpdateArea(unsigned char *pBuffer, unsigned short BufferLen)
//功能		:设置渣土倾倒区域
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:
*********************************************************************/
unsigned char MuckDump_UpdateArea(unsigned char *pBuffer, unsigned short BufferLen)
{
    unsigned long OffSetAddr;
    unsigned char i,Sum;
    unsigned short datalen,templen;
    unsigned char *pTmp;
    //////////////
    if(BufferLen==0)return 2;  
    ////////////////////////
    if(0==MuckDump_SetAreaCheckDatalen(pBuffer,BufferLen))return 2;
    Sum=*pBuffer++;
    if(Sum>MAX_MUCK_DUMP_AREA_SUM)return 1;
    ////////////////////    
    /////////////////
    MuckDump_ClearArea();
    ////////////
    datalen=BufferLen-1;
    pTmp=pBuffer;
    //渣土倾倒
    MuckDump_ParamInit();
    s_stMuckDAttrib.AreaNum = 0;    
    for(i=0; i<Sum; i++)
    {
        OffSetAddr=MUCK_DUMP_AREA_START + i*STMUCK_DUMP_AREA_SIZE;
        templen=MuckDump_SaveOneArea(OffSetAddr,pTmp,datalen);
        if(0 == templen) break;
        s_stMuckDAttrib.AreaNum++;
        if(s_stMuckDAttrib.AreaNum>=MAX_MUCK_DUMP_AREA_SUM)break;
        pTmp +=templen;
        if(datalen>templen)datalen -=templen;
        else break;
    }
    return 0;    
}

/*********************************************************************
//函数名称	:MuckDump_AddArea(unsigned char *pBuffer, unsigned short BufferLen)
//功能		:设置渣土倾倒区域
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:
*********************************************************************/
unsigned char MuckDump_AddArea(unsigned char *pBuffer, unsigned short BufferLen)
{
    unsigned long OffSetAddr,AreaID;
    unsigned char i,j,Sum;
    STMUCK_DUMP_AREA stMuckDumpArea;  //读取渣土倾倒区域属性
    unsigned short datalen,templen;
    unsigned char *pTmp;
    //////////////
    if(BufferLen==0)return 2;
    ////////////
    Sum=*pBuffer++;
    /////////////////////////
    if(Sum+s_stMuckDAttrib.AreaNum>MAX_MUCK_DUMP_AREA_SUM)return 1;//失败
    //////////////////////////
    pTmp =pBuffer;
    datalen=BufferLen-1;
    //渣土倾倒
    for(i=0; i<Sum; i++)//检查ID
    {    
        templen=MuckDump_GetAreaDatalen(pTmp,datalen,&AreaID);
        if(0 == templen)return 2;//消息有误;
        //////////////////////////
        for(j=0; j<s_stMuckDAttrib.AreaNum; j++)
        {
            OffSetAddr=MUCK_DUMP_AREA_START + j*STMUCK_DUMP_AREA_SIZE;
            sFLASH_ReadBuffer((unsigned char *)&stMuckDumpArea,OffSetAddr,MUCK_DUMP_HEAD_SIZE);
            if(AreaID==stMuckDumpArea.AreaID)
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
        OffSetAddr=MUCK_DUMP_AREA_START + s_stMuckDAttrib.AreaNum*STMUCK_DUMP_AREA_SIZE;
        templen=MuckDump_SaveOneArea(OffSetAddr,pTmp,datalen);
        if(0 == templen)break;
        ////////////////////
        s_stMuckDAttrib.AreaNum++;//
        if(s_stMuckDAttrib.AreaNum>=MAX_MUCK_DUMP_AREA_SUM)break;
        ////////////////////
        pTmp +=templen;
        if(datalen>templen)datalen -=templen;
        else break;
    }    
    return 0;    
}
/*********************************************************************
//函数名称	:MuckDump_ModifyArea(unsigned char *pBuffer, unsigned short BufferLen)
//功能		:修改渣土倾倒区域
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:
*********************************************************************/
unsigned char MuckDump_ModifyArea(unsigned char *pBuffer, unsigned short BufferLen)
{
    unsigned long OffSetAddr,AreaID;
    unsigned char i,j,Sum,flag;
    STMUCK_DUMP_AREA stMuckDumpArea;  //读取渣土倾倒区域属性
    unsigned short datalen,templen;
    unsigned char *pTmp;
    //////////////
    if(BufferLen==0)return 2;
    ////////////
    Sum=*pBuffer++;
    pTmp =pBuffer;
    datalen=BufferLen-1;
    //渣土倾倒
    for(i=0; i<Sum; i++)//检查ID
    {  
        templen=MuckDump_GetAreaDatalen(pTmp,datalen,&AreaID);        
        if(0 == templen)return 2;//消息有误;
        //////////////////////////
        flag =0;
        for(j=0; j<s_stMuckDAttrib.AreaNum; j++)
        {
            OffSetAddr=MUCK_DUMP_AREA_START + j*STMUCK_DUMP_AREA_SIZE;
            sFLASH_ReadBuffer((unsigned char *)&stMuckDumpArea,OffSetAddr,MUCK_DUMP_HEAD_SIZE);
            if(AreaID==stMuckDumpArea.AreaID)
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
        for(j=0; j<s_stMuckDAttrib.AreaNum; j++)
        {
            OffSetAddr=MUCK_DUMP_AREA_START + j*STMUCK_DUMP_AREA_SIZE;
            sFLASH_ReadBuffer((unsigned char *)&stMuckDumpArea,OffSetAddr,MUCK_DUMP_HEAD_SIZE);
            if(AreaID==stMuckDumpArea.AreaID)
            {
                templen=MuckDump_SaveOneArea(OffSetAddr,pTmp,datalen);
                break;
            }
        }
        ///////////////
        MuckDump_CheckForResetParam(AreaID);
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
** 函数名称: MuckDump_SetArea
** 功能描述: :设置渣土倾倒区域(0x860a)
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char MuckDump_SetArea(unsigned char *pBuffer, unsigned short BufferLen)
{
    unsigned char AckResult,type;
    if(BufferLen==0)return 2;
    /////////////////
    type=*pBuffer++;
    //////////////
    AckResult = 0;
    if(0== type)//更新
    {
        AckResult=MuckDump_UpdateArea(pBuffer,BufferLen-1);
    }
    else
    if(1 == type)//添加
    {   
        AckResult=MuckDump_AddArea(pBuffer,BufferLen-1);
    }
    else
    if(2 == type)//修改
    {  
        AckResult=MuckDump_ModifyArea(pBuffer,BufferLen-1);
    }
    return AckResult;
}
/*************************************************************
** 函数名称: MuckDump_DeleteArea
** 功能描述: :删除渣土倾倒区域
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char MuckDump_DeleteArea(unsigned char *pBuffer, unsigned short BufferLen) 
{
    unsigned char i,j,Sum,flag;
    unsigned long OffSetAddr,AreaID;
    STMUCK_DUMP_AREA stMuckDumpArea;  //渣土倾倒区域属性结构
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
            for(j=0; j<s_stMuckDAttrib.AreaNum; j++)
            {
                OffSetAddr=MUCK_DUMP_AREA_START + j*STMUCK_DUMP_AREA_SIZE;
                sFLASH_ReadBuffer((unsigned char *)&stMuckDumpArea,OffSetAddr,MUCK_DUMP_HEAD_SIZE);
                if(AreaID==stMuckDumpArea.AreaID)
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
            for(j=0; j<s_stMuckDAttrib.AreaNum; j++)
            {
                OffSetAddr=MUCK_DUMP_AREA_START + j*STMUCK_DUMP_AREA_SIZE;
                sFLASH_ReadBuffer((unsigned char *)&stMuckDumpArea,OffSetAddr,MUCK_DUMP_HEAD_SIZE);
                if(AreaID==stMuckDumpArea.AreaID)//需要删除的区域ID
                {
                    stMuckDumpArea.AreaID = 0;  //删除区域,只需把ID=0
                    OffSetAddr=MUCK_DUMP_AREA_START + j*STMUCK_DUMP_AREA_SIZE;
                    stMuckDumpArea.Head =MUCK_DUMP_HEAD_FLAG;
                    Area_MdyDataInFlash((unsigned char*)&stMuckDumpArea,OffSetAddr,MUCK_DUMP_HEAD_SIZE);
                }
            }
            ////////////////////////////
            MuckDump_CheckForResetParam(AreaID);
            ////////////////////////////
        } 
    }
    else//删除所有
    {
        MuckDump_ClearArea();
    }
    //////////////
    return 0;
}
/*************************************************************
** 函数名称: MuckDump_QueryArea
** 功能描述: 查询渣土倾倒区域(0x860c)
** 入口参数: 数据首地址
** 出口参数: 数据长度
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned short MuckDump_QueryArea(unsigned char *pBuffer)
{
    STMUCK_DUMP_AREA stMuckDumpArea;  //渣土倾倒区域属性结构
    unsigned long OffSetAddr;
    unsigned short len,i;
    unsigned char num;
    unsigned char *buffer;
    
    buffer = pBuffer;
    num = 0;
    len = 1; //区域总数,先跳开
    for(i=0; i<s_stMuckDAttrib.AreaNum; i++)
    {
        OffSetAddr=MUCK_DUMP_AREA_START + i*STMUCK_DUMP_AREA_SIZE;
        sFLASH_ReadBuffer((unsigned char *)&stMuckDumpArea,OffSetAddr,STMUCK_DUMP_AREA_SIZE);
        if(0 != stMuckDumpArea.AreaID)
        {
            Public_ConvertLongToBuffer(stMuckDumpArea.AreaID,&buffer[len]);
            len += 4;
            Public_ConvertShortToBuffer(stMuckDumpArea.AreaAttrib,&buffer[len]);
            len += 2;
            Public_ConvertLongToBuffer(stMuckDumpArea.LeftUpLat,&buffer[len]);
            len += 4;
            Public_ConvertLongToBuffer(stMuckDumpArea.LeftUpLong,&buffer[len]);
            len += 4;
            Public_ConvertLongToBuffer(stMuckDumpArea.RightDownLat,&buffer[len]);
            len += 4;
            Public_ConvertLongToBuffer(stMuckDumpArea.RightDownLong,&buffer[len]);
            len += 4;
            num ++;
        }
    }
    //////////////
    buffer[0] = num;
    return len;
}
/*************************************************************
** 函数名称: MuckDump_GetCurMuckDumpAreaID
** 功能描述: 获得当前渣土倾倒区域ID
** 入口参数: 无
** 出口参数: 
** 返回参数: 0:不在任何渣土倾倒区域内,非零:为渣土倾倒区域ID
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned long MuckDump_GetCurMuckDumpAreaID(void)
{
    if(MD_AREA_STATUS_IN != s_stMuckDAttrib.AreaFlag)return 0;
    if(MAX_MUCK_DUMP_AREA_ADDR  == s_stMuckDAttrib.addr)return 0;
    return (0==s_stMuckDAttrib.AreaID)?1:s_stMuckDAttrib.AreaID;
}
/*************************************************************
** 函数名称: MuckDump_ScanTask
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void MuckDump_CheckScanTask(void)
{
    unsigned char speed;
    MuckDump_GetCurGpsCoordinate();//1s获取一次经纬度        
    s_stMuckDAttrib.nonavigationCnt =1;  
    s_stMuckDAttrib.preAreaCnt=s_stMuckDAttrib.AreaCnt;
    speed = Area_GetCurSpeed();
    ///////////////////////////
    if(0==speed&&0==s_stMuckDAttrib.preSpeed)return;//速度为零,不处理
    /////////////////////////
    s_stMuckDAttrib.preSpeed = speed;   
    if(s_stMuckDAttrib.AreaNum)
    MuckDump_ScanArea();    
}
/*************************************************************
** 函数名称: MuckDump_ScanTask
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void MuckDump_ScanTask(void)
{
    if(Io_ReadStatusBit(STATUS_BIT_NAVIGATION))
    {
        GPS_STRUCT stTmpGps;
        Gps_CopygPosition(&stTmpGps);
        if(stTmpGps.Second!=s_stMuckDAttrib.preSecond)
        {
            s_stMuckDAttrib.preSecond = stTmpGps.Second; 
            MuckDump_CheckScanTask();
        }
    }
    else
    {  
        s_stMuckDAttrib.preSecond = 0xff;
        s_stMuckDAttrib.preSpeed = 0xff;
        if(s_stMuckDAttrib.nonavigationCnt<50)s_stMuckDAttrib.nonavigationCnt++;//10s
        else
        if(s_stMuckDAttrib.nonavigationCnt == 50)
        {
            s_stMuckDAttrib.nonavigationCnt =100;
        }
    }
}

/*************************************************************
** 函数名称: MuckDump_ScanTask
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void MuckDump_StartScanTask(void)
{
    LZM_PublicSetCycTimer(&s_stMDTimer[MD_TIMER_TASK],PUBLICSECS(0.2),MuckDump_ScanTask);
}

/*************************************************************
** 函数名称: MuckDump_ReadAndCheckParam
** 功能描述: 区域参数初始化
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void MuckDump_ReadAndCheckParam(void) 
{
    unsigned long OffSetAddr;
    unsigned short i;
    
    STMUCK_DUMP_AREA stMuckDumpArea;	  //读取渣土倾倒区域属性
       ////////////////
    s_stMuckDAttrib.preSecond       = 0xff;
    s_stMuckDAttrib.nonavigationCnt = 0xff;
    ////////////
    memset(&s_stMuckDAttrib,0,sizeof(s_stMuckDAttrib));
    //渣土倾倒初始化    
    MuckDump_ParamInit();
    ////////////////////////
    for(i=0; i<MAX_MUCK_DUMP_AREA_SUM; i++)
    {
        OffSetAddr=MUCK_DUMP_AREA_START + i*STMUCK_DUMP_AREA_SIZE ;  
        sFLASH_ReadBuffer((unsigned char *)&stMuckDumpArea,OffSetAddr,MUCK_DUMP_HEAD_SIZE);//读出区域数据	
        if(MUCK_DUMP_HEAD_FLAG == stMuckDumpArea.Head)
        {
            if(0xFFFFFFFF == stMuckDumpArea.AreaID)
            {
                MuckDump_ClearArea();
                break;
            }
            else
            {
                s_stMuckDAttrib.AreaNum++;
            }
        }
        else 
        if(stMuckDumpArea.Head != 0xFFFF)
        {
            MuckDump_ClearArea();
            break;
        }
        else 
        if(0xFFFF == stMuckDumpArea.Head)
        {
            break;
        }
    }
    //////////////////////////////  
    LZM_PublicSetOnceTimer(&s_stMDTimer[MD_TIMER_TASK],PUBLICSECS(15),MuckDump_StartScanTask);    
}
/*************************************************************
** 函数名称: MuckDump_ParameterInitialize
** 功能描述: 渣土倾倒区域参数初始化
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void MuckDump_ParameterInitialize(void) 
{
    LZM_PublicKillTimerAll(s_stMDTimer,MD_TIMERS_MAX); 
    //////////////////////////////
    SetTimerTask(TIME_MUCKDUMP,SYSTICK_0p1SECOND);
    //////////////////////////////////////////////////////////
    LZM_PublicSetOnceTimer(&s_stMDTimer[MD_TIMER_TASK],PUBLICSECS(15),MuckDump_ReadAndCheckParam);    
}

/*************************************************************
** 函数名称: MuckDump_TimeTask
** 功能描述: 区域功能定时器处理函数
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
FunctionalState MuckDump_TimeTask(void)
{
    LZM_PublicTimerHandler(s_stMDTimer,MD_TIMERS_MAX);
    return ENABLE;
}
#else
/*************************************************************
** 函数名称: MuckDump_ParameterInitialize
** 功能描述: 区域参数初始化
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void MuckDump_ParameterInitialize(void) 
{
}
/*************************************************************
** 函数名称: MuckDump_GetCurMuckDumpAreaID
** 功能描述: 获得当前渣土倾倒区域ID
** 入口参数: 无
** 出口参数: 
** 返回参数: 0:不在任何渣土倾倒区域内,非零:为渣土倾倒区域ID
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned long MuckDump_GetCurMuckDumpAreaID(void)
{
    return 0;
}
#endif
/*******************************************************************************
 *                             end of module
 *******************************************************************************/


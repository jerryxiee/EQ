/*******************************************************************************
 * File Name:			oilwearCalib.c 
 * Function Describe:	油耗校准
 * Relate Module:		
 * Writer:				Joneming
 * Date:				2013-05-25
 * ReWriter:			
 * Date:				
 *******************************************************************************/
#include "include.h"
/*******************************************************************************/
////////////////////////////////////////////
enum
{
    PINPORT_BIT_POS     = 0,     //计价器
    PINPORT_BIT_CAN_BUS,        //CAN-BUS信息采集模块
    PINPORT_BIT_JOINTECH_OIL,   //久通机电油位传感器
    PINPORT_BIT_BSJ_OIL,        //博实结油位传感器
    PINPORT_BIT_MAX             //最大值
}E_16PINPORT;//表明16pin接口接的外设类型,

static unsigned short s_us16PortPeripheralUse = 0;
////////////////////////////////////////////
#ifdef CALIB_OIL_WEAR
//#define CALI_DEBUG_SHOW
#define USE_SMOOTH_DATA  
/////////////////////////////
#define MAX_SMOOTH_DATA_LEN 11 //取中间值最好为奇数
typedef struct 
{
     unsigned short originalData[MAX_SMOOTH_DATA_LEN];//
     unsigned char datalen;//当前接收数据长度
     unsigned char index;//当前保存的数据的下标
}STSMOOTHDATA;

static STSMOOTHDATA s_stSmoothData;
////////////////////////////////////////////
typedef struct 
{
    unsigned short testVal[OIL_CALIB_MAX_NUM];//放大10倍保存
    unsigned short oilValue[OIL_CALIB_MAX_NUM];//放大10倍保存
    unsigned short fullOil;//满油量，放大10倍
    unsigned char CalibPoint;//标定点数
    unsigned char saveFlag;//保存标志
}STCALIBOILSAVE;
//////////////////////////
typedef struct 
{
    float coef[OIL_CALIB_MAX_NUM];
    float c[OIL_CALIB_MAX_NUM];//放大10倍
    unsigned char calFlag;//计算系数标志
    unsigned char bak[3];
}STCALIBOILCAL;
/////////////////////////////
static STCALIBOILSAVE s_stCalibOilSave;
///////////////////////////
#define STCALIBOILSAVE_SIZE sizeof(STCALIBOILSAVE)

#define CALI_SAVE    0xDD

#define CALI_DEFAULT_POINT 9
////////////////
const unsigned short c_usDefaultTestVal[]=
{
    1200,800,610,420,310,230,170,100,30,
};
///////////////////////////////
const float c_fDefaultOilVal[]=
{
    0.0,12.5,25.0,37.5,50.0,62.5,75.0,87.5,100.0,
};
////////////////////////

static STCALIBOILCAL s_stCalibOilCal;

static unsigned short s_usCurOilResVal  = 0;
static unsigned short s_usCurOilVal     = 0;


static unsigned char s_ucOilCtrlId;
static unsigned char s_ucOilCtrlMaxId;

extern ST_ESPIN s_stESpin[];
extern ST_SPIN s_stSpin[];


/****************************************************/
/*                                                  */
/* Local Definitions and macros                     */
/*                                                  */
/****************************************************/
LZM_RET TaskCalibOilDemarcate(LZM_MESSAGE *Msg);
extern LZM_RET TaskMenuSystemAdvanced(LZM_MESSAGE *Msg);


/****************************************************/
/*                                                  */
/* Local Definitions and macros                     */
/*                                                  */
/****************************************************/
/**************************************************************
*Function: OilWear_GetCurOilResVal
*Describe: 获得当前油耗电阻值(放大10倍)
*入口参数: 
**************************************************************/
unsigned short OilWear_GetCurOilResVal(void)
{
    return s_usCurOilResVal;
}
/**************************************************************
*Function: OilWear_GetCurOilVal
*Describe: 获得当前油耗油量值(1/10L)
*入口参数: 
**************************************************************/
unsigned short OilWear_GetCurOilVal(void)
{
    return s_usCurOilVal;
}
/*******************************************************************************
** 函数名称: OilWear_GetSubjoinInfoCurOilVal
** 功能描述: 取得当前油量附加信息包(包括附加信息ID、长度、附加信息体)
** 入口参数: 需要保存油量附加信息包的首地址
** 出口参数: 
** 返回参数: 油量附加信息包的总长度
*******************************************************************************/
unsigned char OilWear_GetSubjoinInfoCurOilVal(unsigned char *data)
{
    unsigned char buffer[10]={0};
    unsigned char len;
    unsigned short usVal;
    if(!OilWear_Get16PINPortInsertBSJOilFlag())return 0;//未接入博实结油位传感器
    len = 0;
    usVal = OilWear_GetCurOilVal();//(1/10L)
    buffer[len++] = 0x02;//附加信息ID
    buffer[len++] = 2;   //附加长度
    Public_ConvertShortToBuffer(usVal,&buffer[len]);//附加信息体
    //////////////////////////
    len += 2;
    ///////////////
    memcpy(data,buffer,len);
    /////////////////
    return len;
}
/*******************************************************************************
** 函数名称: OilWear_GetSubjoinInfoCurOilResVal
** 功能描述: 取得当前油量电阻值附加信息包(包括附加信息ID、长度、附加信息体)
** 入口参数: 需要保存油量电阻值附加信息包的首地址
** 出口参数: 
** 返回参数: 油量电阻值附加信息包的总长度
*******************************************************************************/
unsigned char OilWear_GetSubjoinInfoCurOilResVal(unsigned char *data)
{
    unsigned char buffer[10]={0};
    unsigned char len;
    unsigned short usVal;
    if(!OilWear_Get16PINPortInsertBSJOilFlag())return 0;//未接博实结油位传感器
    len = 0;
    usVal = OilWear_GetCurOilResVal()/10;
    buffer[len++] = 0xE5;//附加信息ID
    buffer[len++] = 2;   //附加长度
    Public_ConvertShortToBuffer(usVal,&buffer[len]);//附加信息体
    //////////////////////////
    len += 2;
    ///////////////
    memcpy(data,buffer,len);
    /////////////////
    return len;
}
/**************************************************************
*Function: OilWear_UartDataProtocolParse
*Describe: 油耗数据解析
*入口参数: pBuffer数据首地址,Len数据长度
*注意:     24 F1 05 X1 X2 X3 4F FF ,"获得油耗"BSJ协议
           24 F1 05 03 99 02 94 FF ,"获得油耗"国迈GM协议 
**************************************************************/
void OilWear_UartDataProtocolParse(unsigned char *pBuffer,unsigned short Len) 
{
    u8 *Ptr;     //指向接收数据
    u16 i;
    u16 DataLen,oil,tmplen; //数据长度    
    u8 ChkSum;  //24 F1 05 00 77 03 A4 FF  ,A4之前所有字节异或
    u8 ChkSum2; //24 F1 05 03 99 02 94 FF  ,94之前除24外所有数据之和
    u8 flag; 
    if(!OilWear_Get16PINPortInsertBSJOilFlag())return;//未接博实结油位传感器
    flag = 0;
    Ptr=pBuffer;    
    for(i=0; i<Len-1; i++)
    {
        if(*(Ptr+i)==0x24 && *(Ptr+i+1)==0xF1)
        {
            Ptr=Ptr+i;            
            tmplen = Len-i;
            flag = 1;
            break;
        }
    }
    /////////////////
    if(0==flag)return; //未找到头
    if(tmplen<3)return;//长度过小
    //取得数据包长度
    DataLen = *(Ptr+2); 
    //////////////////
    if(tmplen<DataLen+2)return;//长度过小
    ////////////////////
    ChkSum = 0;
    for(i=0; i<DataLen+1 ;i++)
    {
        ChkSum=ChkSum ^ *(Ptr+i);
    }
    ChkSum2 = 0;
    for(i=1; i<DataLen+1 ;i++)
    {
        ChkSum2=ChkSum2 + *(Ptr+i);
    }        
    if((ChkSum == *(Ptr+DataLen+1)|| ChkSum2 == *(Ptr+DataLen+1)) && *(Ptr+DataLen+2) == 0xFF)
    {
        oil = (*(Ptr+3) << 8) + *(Ptr+4);
        s_usCurOilResVal = oil*10 +  *(Ptr+5);
        s_usCurOilVal=(unsigned short)OilWear_GetCalibTestValue(s_usCurOilResVal);
    } 
}
/**************************************************************
*Function: Pulbic_QuickSortByAsc
*Describe: 快速排序,从小到大排序，对源数据排序，但只重新排列其下标,源数据序列不变
*入口参数: SortDataAddr为要排序的数据首地址,suff为原下标的顺序，low 为低位，high为高位
*注意:循环调用函数内部不能用static变量
**************************************************************/
void Pulbic_QuickSortByAsc(const unsigned short *SortDataAddr,unsigned char Suff[], unsigned char low, unsigned char high)
{ 
    unsigned char i,j; 
    unsigned char pivot;
    unsigned char tmplow,tmphigh;
    /////////////////////
    ///////////////	
    if(low<high)
    {
        tmplow=low; 
        tmphigh=high;
    }
    else
    {
        tmplow=high; 
        tmphigh=low;
    }
    pivot=Suff[tmplow]; 
    i=tmplow; 
    j=tmphigh;
    while(i<j) 
    { 
        while(i<j && SortDataAddr[Suff[j]]>=SortDataAddr[pivot]) 
            j--; 
        if(i<j) 
            Suff[i++]=Suff[j]; //将比枢轴记录小的记录移到低端 

        while (i<j && SortDataAddr[Suff[i]]<=SortDataAddr[pivot]) 
            i++; 
        if(i<j)  
            Suff[j--]=Suff[i]; //将比枢轴记录大的记录移到高端
    }
    Suff[i]=pivot; //枢轴记录移到最终位置		
    if(tmplow+1<i)
        Pulbic_QuickSortByAsc(SortDataAddr,Suff,tmplow,i-1); 
    if(i+1<tmphigh)
        Pulbic_QuickSortByAsc(SortDataAddr,Suff,i+1,tmphigh); 
}
/*************************************************************
** 函数名称: OilWear_GetMiddleValue
** 功能描述: 取得数据的中间值
** 入口参数: data数据首地址,len数据长度
** 出口参数:
** 返回参数: 中间值
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned short OilWear_GetMiddleValue(const unsigned short *data,unsigned char len)
{
    unsigned char SortNo[MAX_SMOOTH_DATA_LEN];
    unsigned char i,midindex;
    ///////////////////////
    if(0==len||len>MAX_SMOOTH_DATA_LEN)//长度为0的情况或超长的情况
    {
        return 0;
    }
    else
    if(len<3)//长度<3的情况
    {
        return data[len-1];
    }    
    ////其它情况排序才有意义/////////////////
    for(i=0; i<len;i++)
    {
        SortNo[i]=i;
    }
    ////////////////////
    midindex =((len-1)>>1);//取得排序中间值下标
    ////对数据进行排序/////////////////////
    Pulbic_QuickSortByAsc(data,SortNo,0,len-1);
    ///////////////////////
    return data[SortNo[midindex]];
}
/*************************************************************
** 函数名称: OilWear_ReceiveAndDisposeData
** 功能描述: 接收原始数据并进行平滑处理(11个点,从小到大排序好,取中间值)
** 入口参数: testVal原始数据
** 出口参数: 无
** 返回参数: 平滑之后的数据
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned short OilWear_ReceiveAndDisposeData(unsigned short testVal)
{
    static unsigned char sucFirstRunFlag=0;//
    if(0==sucFirstRunFlag)//首次调用
    {
        sucFirstRunFlag =1;
        s_stSmoothData.datalen = 0;
        s_stSmoothData.index = 0;
    }
    ////////保存当前数据//////////////////
    s_stSmoothData.originalData[s_stSmoothData.index]= testVal;
    ////////////////////////
    if(s_stSmoothData.datalen<MAX_SMOOTH_DATA_LEN)s_stSmoothData.datalen++;
    ///////////////////
    if(s_stSmoothData.index<MAX_SMOOTH_DATA_LEN-1)
    {
        s_stSmoothData.index++;
    }
    else
    {
        s_stSmoothData.index =0;
    }
    return OilWear_GetMiddleValue(s_stSmoothData.originalData,s_stSmoothData.datalen);
}
/*************************************************************
** 函数名称: OilWear_GetCalibCoefIndex
** 功能描述: 根据测量值获得校准系数下标
** 入口参数: 测量值(放大10倍)
** 出口参数: 无
** 返回参数: 下标
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned char OilWear_GetCalibCoefIndex(unsigned short val)
{
    unsigned char i;
    unsigned char pointnum;
    pointnum=s_stCalibOilSave.CalibPoint-1;
    if(s_stCalibOilSave.testVal[0]<s_stCalibOilSave.testVal[1])//从小到大
    {
        if(val<=s_stCalibOilSave.testVal[0])//小于最小
        {
            return 0;
        }
        else
        if(val>=s_stCalibOilSave.testVal[pointnum-1])//大于最大
        {
            return pointnum-1;
        }
        /////////////////////////        
        for(i= 1; i<= pointnum ; i++)
        {
            if(val<=s_stCalibOilSave.testVal[i])
            {
                return i-1;
            }
        }
    }
    else
    if(s_stCalibOilSave.testVal[0]>s_stCalibOilSave.testVal[1])//从大到小
    {
        if(val>=s_stCalibOilSave.testVal[0])//大于最大
        {
            return 0;
        }
        else
        if(val<=s_stCalibOilSave.testVal[pointnum-1])//小于最小
        {
            return pointnum-1;
        }
        /////////////////////////
        for(i= 1; i<= pointnum ; i++)//////////////
        {
            if(val>=s_stCalibOilSave.testVal[i])
            {
               return i-1;
            }
        }
    }
    ///////////////////////////////
    return 0;
}
/*************************************************************
** 函数名称: OilWear_GetCalibTestValue
** 功能描述: 根据测量值获得实际油量值
** 入口参数: 测量值(放大10倍)
** 出口参数: 无
** 返回参数: 实际油量值(1/10L)
** 全局变量: 
** 调用模块: 
*************************************************************/	
float OilWear_GetCalibTestValue(unsigned short testVal)
{
    unsigned char k;
    unsigned short value;
    float val,result;
    //////////////////
    OilWear_UpdatePram();//读取系数
    value = testVal;
    #ifdef USE_SMOOTH_DATA
    value=OilWear_ReceiveAndDisposeData(testVal);
    #endif    
    ////////////////////
    if(0==s_stCalibOilCal.calFlag)return value;
    ////////////////// 
    k = OilWear_GetCalibCoefIndex(value);
    ///////////////////////
    val = value;//测量值已放大10倍
    result= val*s_stCalibOilCal.coef[k] + s_stCalibOilCal.c[k];//c已放大10倍计算
    //////结果放大10倍/////////////////////////////
    /////////合理性判断///////////////
    if(result< 0.0)result = 0.0;
    ////////合理性判断//////////////////
    if(result>s_stCalibOilSave.fullOil)result=s_stCalibOilSave.fullOil;
    ///////////////////////////////////
    #ifdef CALI_DEBUG_SHOW    
    Public_ShowValue(0,16,"平滑前%d  ",(unsigned long)testVal);
    Public_ShowValue(0,32,"平滑后%d  ",(unsigned long)val);
    Public_ShowValue(0,48,"修正后%d  ",(unsigned long)result);    
    #endif
    return result;
}
/*************************************************************
** 函数名称: OilWear_SetCalibOilSaveValue
** 功能描述: 把油量标定值保存到结构体中
** 入口参数: index保存编号(从0开始),oilVal:油量值(放大10倍,即单位为0.1L),testVal:测量值(放大10倍)
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void OilWear_SetCalibOilSaveValue(unsigned char index,unsigned short oilVal,unsigned short testVal)
{
    s_stCalibOilSave.oilValue[index]= oilVal;
    s_stCalibOilSave.testVal[index]= testVal;
}
/*************************************************************
** 函数名称: OilWear_SetCalibOilSaveValue
** 功能描述: 把油量标定值保存到结构体中
** 入口参数: index保存编号(从0开始),oilVal:油量值(放大10倍,即单位为0.1L),testVal:测量值(放大10倍)
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void OilWear_GetCalibOilValue(unsigned char index,unsigned short *oilVal,unsigned short *testVal)
{
   *oilVal = s_stCalibOilSave.oilValue[index];
    *testVal=s_stCalibOilSave.testVal[index];
}
/*************************************************************
** 函数名称: OilWear_SetCalibPoint
** 功能描述: 设置油量标定点数
** 入口参数: CalibPoint标定点数
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void OilWear_SetCalibPoint(unsigned char CalibPoint)
{
    s_stCalibOilSave.CalibPoint = CalibPoint;
}
/*************************************************************
** 函数名称: OilWear_SetCalibPoint
** 功能描述: 设置油量标定点数
** 入口参数: CalibPoint标定点数
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned char OilWear_GetCalibPoint(void)
{
    return s_stCalibOilSave.CalibPoint;
}
/*************************************************************
** 函数名称: OilWear_SetCalibFullOil
** 功能描述: 设置满油量
** 入口参数: fullOil满油量(放大10倍)
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void OilWear_SetCalibFullOil(unsigned short fullOil)
{
    s_stCalibOilSave.fullOil = fullOil;
}
/*************************************************************
** 函数名称: OilWear_SetCalibFullOil
** 功能描述: 设置满油量
** 入口参数: fullOil满油量(放大10倍)
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned short OilWear_GetCalibFullOil(void)
{
    return s_stCalibOilSave.fullOil;
}
/*************************************************************
** 函数名称: OilWear_SaveCalibOilParamToFram
** 功能描述：把油量标定参数结构体保存到铁电中
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void OilWear_SaveCalibOilParamToFram(void)
{
    s_stCalibOilSave.saveFlag = CALI_SAVE;
    FRAM_BufferWrite(FRAM_OIL_WEAR_CALIB_ADDR,(unsigned char *)&s_stCalibOilSave,STCALIBOILSAVE_SIZE);
}
/*************************************************************
** 函数名称: OilWear_CalCoefAndC
** 功能描述：计算系数 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void OilWear_ReadCalibOilParam(void)
{
    unsigned char len;
    unsigned char i; 
    len=FRAM_BufferRead((unsigned char *)&s_stCalibOilSave,STCALIBOILSAVE_SIZE,FRAM_OIL_WEAR_CALIB_ADDR);
    if(STCALIBOILSAVE_SIZE != len||0 == s_stCalibOilSave.CalibPoint|| s_stCalibOilSave.CalibPoint>OIL_CALIB_MAX_NUM||s_stCalibOilSave.saveFlag !=CALI_SAVE)
    {
        s_stCalibOilSave.fullOil = 400;
        s_stCalibOilSave.CalibPoint = CALI_DEFAULT_POINT;
        for(i =0; i<s_stCalibOilSave.CalibPoint; i++)
        {
            s_stCalibOilSave.oilValue[i]=(unsigned short)(c_fDefaultOilVal[i]*10);
            s_stCalibOilSave.testVal[i]=c_usDefaultTestVal[i]*10;
        }
        ////////////////////////////////
    }
}
/*************************************************************
** 函数名称: OilWear_CalCoefAndC
** 功能描述：计算当前标定系数 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void OilWear_CalCoefAndC(void)
{
    unsigned char i; 
    float Ylen,v,fv,Xlen;   
    if(s_stCalibOilSave.saveFlag != CALI_SAVE)
    {
        s_stCalibOilCal.calFlag = 0;
        for(i=0; i<s_stCalibOilSave.CalibPoint-1; i++)
        {
            s_stCalibOilCal.coef[i]=1.0;
            s_stCalibOilCal.c[i]=0.0;
        }
        return;
    }
    //计算斜率和常数c
    for(i=0; i<s_stCalibOilSave.CalibPoint-1; i++)
    {
        fv = s_stCalibOilSave.testVal[i];
        v = s_stCalibOilSave.oilValue[i];	
        Xlen=s_stCalibOilSave.testVal[i+1]-fv;
        Ylen=s_stCalibOilSave.oilValue[i+1]-v;
        if(fabs(Xlen)<0.1)
        {
            s_stCalibOilCal.coef[i]=1.0;
            s_stCalibOilCal.c[i]=0.0;
        }
        else
        {
            s_stCalibOilCal.coef[i]=Ylen/Xlen;
            #ifdef CHECK_COEF
            if((s_stCalibOilCal.coef[i] > 10.0)||(s_stCalibOilCal.coef[i] < 0.10))
            {
                s_stCalibOilCal.coef[i]=1.0;
                s_stCalibOilCal.c[i]=0.0;
            }
            else
            #endif
                s_stCalibOilCal.c[i] = s_stCalibOilSave.oilValue[i]-s_stCalibOilCal.coef[i]*s_stCalibOilSave.testVal[i];
        }
    } 
    s_stCalibOilCal.calFlag = 1;
}
/*************************************************************
** 函数名称: OilWear_UpdatePram
** 功能描述: 读取油量标定参数并计算系数
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void OilWear_UpdatePram(void)
{
    static unsigned char sucOilFirstRun=0;
    if(sucOilFirstRun)return;
    sucOilFirstRun = 1;
    OilWear_ReadCalibOilParam();
    OilWear_CalCoefAndC();
    //////////////////////
    OilWear_Updata16PINPortPeripheralParam();
}
/*************************************************************
** 函数名称: TaskCalibOilPointNumber
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskCalibOilPointNumber(LZM_MESSAGE *Msg)
{
    unsigned char val;
    switch(Msg->message)
    {
        case TASK_START:
            LcdClearScreen();
            s_ucOilCtrlId=0;
            LcdShowCaption("油量标定",1);
            LcdShowHorLine(0, 131, 19, 1);
            LcdShowStr(0, 24, "标定点数:", 0);
            LcdShowStr(0, 44, "满油量:       L", 0);
            SpinInitData(&s_stSpin[0],80, 24, 3,1,25,1,"");
            ESpinInit(&s_stESpin[0], 64, 45, 5,0);
            sprintf(s_stESpin[0].buffer,"%05.1f",s_stCalibOilSave.fullOil*0.1);
            s_stSpin[0].keyflag = 1;
            s_stSpin[0].handle = s_stCalibOilSave.CalibPoint;
            SpinShow(&s_stSpin[0]);
            /////////
            s_stESpin[0].loop = 0;
            ESpinShow(&s_stESpin[0]);
            SpinSetFocus(&s_stSpin[0]);
            return 1;
        case KEY_PRESS:
            if(s_ucOilCtrlId==0xff)break;
            switch(TaskPara)
            {
                case KEY_BACK://打印
                    LZM_StartNewTask(TaskMenuSystemAdvanced);//系统管理
                    break; 
                case KEY_ENTER://菜单
                    OilWear_SetCalibPoint(s_stSpin[0].handle);
                    OilWear_SetCalibFullOil((unsigned short)(atof(s_stESpin[0].buffer)*10));
                    LZM_StartNewTask(TaskCalibOilDemarcate);//
                    break;                               
                default:
                    if(0==s_ucOilCtrlId)
                    {
                        val=SpinResponse(&s_stSpin[0],TaskPara);
                        if(val)
                        {
                            s_ucOilCtrlId=1;
                            SpinLoseFocus(&s_stSpin[0]);
                            ESpinSetFocus(&s_stESpin[0]);
                        }
                    }
                    else
                    {
                        val=ESpinResponse(&s_stESpin[0],TaskPara);
                        if(val)
                        {
                            s_ucOilCtrlId=0; 
                            ESpinLoseFocus(&s_stESpin[0]);
                            SpinSetFocus(&s_stSpin[0]);
                            
                        }
                    }
                    break;
            }
            return 1;
        case INT_RTC:
            break;
        default:
            return LZM_DefTaskProc(Msg);
    }
    return 0;
}
/*************************************************************
** 函数名称: OilWear_CalibOilDemarcateSaveModify
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void OilWear_CalibOilDemarcateSaveModify(void)
{
    LZM_SetAlarm(TEST_TIMER, PUBLICSECS(2));
    LcdClearScreen();
    LcdShowCaption("保存中,请稍候", 24);
    OilWear_SaveCalibOilParamToFram();
    OilWear_CalCoefAndC();
}
/*************************************************************
** 函数名称: OilWear_CalibOilDemarcateCancelModify
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void OilWear_CalibOilDemarcateCancelModify(void)
{
    OilWear_ReadCalibOilParam();
    LZM_StartNewTask(TaskMenuSystemAdvanced);//
}
/*************************************************************
** 函数名称: OilWear_CalibOilDemarcateReflashValue
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void OilWear_CalibOilDemarcateReflashValue(unsigned char index)
{
    unsigned char i;
    sprintf(s_stESpin[1].buffer,"%05.1f",s_stCalibOilSave.oilValue[index]*0.1);
    sprintf(s_stESpin[2].buffer,"%06.1f",s_stCalibOilSave.testVal[index]*0.1);
    for(i=1; i<= s_ucOilCtrlMaxId; i++)
    {
        s_stESpin[i].loop = 0;
        ESpinShow(&s_stESpin[i]);
    }
}
/*************************************************************
** 函数名称: OilWear_DisposeEIExpandProtocol
** 功能描述: 油耗伊爱扩展协议
** 入口参数: pBuffer:消息体内容首地址,BufferLen消息体内容长度
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char OilWear_DisposePlatformSetParameter(unsigned char *pBuffer, unsigned short BufferLen)
{
    unsigned short len,val,test;
    unsigned char i,num;
    unsigned char *p;
    p = pBuffer;
    len = 0;
    val=Public_ConvertBufferToShort(&p[len]);    
    len +=2;
    num =p[len++];
    if((3+num*4) != BufferLen)return 0;//长度不对
    ///////////////////
    OilWear_SetCalibFullOil(val);  
    /////////////////////
    OilWear_SetCalibPoint(num);
    for(i=0; i<num;i++)
    {
        val=Public_ConvertBufferToShort(&p[len]); 
        len +=2;
        test=Public_ConvertBufferToShort(&p[len]); 
        len +=2;
        OilWear_SetCalibOilSaveValue(i,val,test);
    }
    ////////////////////
    OilWear_SaveCalibOilParamToFram();
    OilWear_CalCoefAndC();
    ///////////////////
    return 1;
}
/*************************************************************
** 函数名称: OilWear_DisposeEIExpandProtocol
** 功能描述: 油耗伊爱扩展协议
** 入口参数: pBuffer:消息体内容首地址,BufferLen消息体内容长度
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void OilWear_DisposePlatformReadParameter(void)
{
    unsigned char buffer[128];
    unsigned short len,val,test;
    unsigned char i,num;
    len = 0;
    buffer[len++] = 1;//读参数
    buffer[len++] = 0;//保留
    Public_ConvertShortToBuffer(OilWear_GetCalibFullOil(),&buffer[len]);
    len +=2;
    num = OilWear_GetCalibPoint();
    buffer[len++] = num;  
    for(i=0; i<num;i++)
    {
        OilWear_GetCalibOilValue(i,&val,&test);
        Public_ConvertShortToBuffer(val,&buffer[len]);
        len +=2;
        Public_ConvertShortToBuffer(test,&buffer[len]);
        len +=2;
    }
    ////////////////////
    EIExpand_PotocolSendData(EIEXPAND_CMD_OIL_WEAR_CALIB,buffer,len);
}
/*************************************************************
** 函数名称: OilWear_DisposeEIExpandProtocol
** 功能描述: 油耗伊爱扩展协议
** 入口参数: pBuffer:消息体内容首地址,BufferLen消息体内容长度
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char OilWear_DisposeEIExpandProtocol(unsigned char *pBuffer, unsigned short BufferLen)
{
    unsigned char operate;
    unsigned char *p;
    p = pBuffer;
    operate = *p++;//操作标志   
    if(1==operate)//读操作
    {
        Public_SetOperateTimer(OilWear_DisposePlatformReadParameter,PUBLICSECS(2));
        return 1;
    }
    else
    if(2==operate)//写
    {
       p++;//保留字节 
       return OilWear_DisposePlatformSetParameter(p,BufferLen-2);       
    }
    return 1;
}
/*************************************************************
** 函数名称: TaskCalibOilPointNumber
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskCalibOilDemarcate(LZM_MESSAGE *Msg)
{
    unsigned short oilVal,testVal;
    unsigned char index;
    switch(Msg->message)
    {
        case TASK_START:
            LcdClearScreen();
            s_ucOilCtrlId=0;
            s_ucOilCtrlMaxId = 2;
            LcdShowStr(2, 2, "油量标定     点", 0);
            LcdShowHorLine(0, 131, 20, 1);
            LcdShowStr(0, 24, "油  量:       L", 0);
            LcdShowStr(0, 44, "采样值:", 0);
            SpinInitData(&s_stSpin[0],80, 2, 3,1,s_stCalibOilSave.CalibPoint,1,"");
            ESpinInit(&s_stESpin[1], 64, 25, 5,0);
            ESpinInit(&s_stESpin[2], 64, 45, 6,0);            
            s_stSpin[0].keyflag = 1;
            s_stSpin[0].handle = 1;
            OilWear_CalibOilDemarcateReflashValue(s_stSpin[0].handle-1);
            SpinShow(&s_stSpin[s_ucOilCtrlId]);
            SpinSetFocus(&s_stSpin[s_ucOilCtrlId]);
            return 1;
        case KEY_PRESS:
            if(s_ucOilCtrlId==0xff)break;
            switch(TaskPara)
            {
                case KEY_BACK://打印
                    PublicConfirmAll("是否保存修改?",OilWear_CalibOilDemarcateSaveModify,OilWear_CalibOilDemarcateCancelModify,0,0);
                    break; 
                case KEY_ENTER://菜单
                    /////////////////////
                    oilVal=(unsigned short)(atof(s_stESpin[1].buffer)*10);
                    testVal=(unsigned short)(atof(s_stESpin[2].buffer)*10);
                    OilWear_SetCalibOilSaveValue(s_stSpin[0].handle-1,oilVal,testVal);
                    /////////////////////
                    if(s_stSpin[0].handle<s_stCalibOilSave.CalibPoint)
                    {
                        s_stSpin[0].handle++;
                        SpinShow(&s_stSpin[0]);
                        OilWear_CalibOilDemarcateReflashValue(s_stSpin[0].handle-1);
                        ////////////
                        if(s_ucOilCtrlId)
                        {
                            ESpinLoseFocus(&s_stESpin[s_ucOilCtrlId]);
                            s_ucOilCtrlId = 0;
                            SpinSetFocus(&s_stSpin[s_ucOilCtrlId]);
                        }                        
                    }
                    else
                    {
                        PublicConfirmAll("是否保存修改?",OilWear_CalibOilDemarcateSaveModify,OilWear_CalibOilDemarcateCancelModify,0,1);
                    }
                    break;                               
                default:
                    if(0==s_ucOilCtrlId)
                    {
                        index=SpinResponse(&s_stSpin[s_ucOilCtrlId],TaskPara); 
                        if(index)
                        {
                            SpinLoseFocus(&s_stSpin[s_ucOilCtrlId]);
                            OilWear_CalibOilDemarcateReflashValue(s_stSpin[0].handle-1);
                            ///////////////////
                            s_ucOilCtrlId++;
                            ESpinSetFocus(&s_stESpin[s_ucOilCtrlId]);
                        }
                        else
                        {
                            OilWear_CalibOilDemarcateReflashValue(s_stSpin[0].handle-1);
                        }
                    }
                    else
                    {
                        index=ESpinResponse(&s_stESpin[s_ucOilCtrlId],TaskPara); 
                        if(index)
                        {
                            ESpinLoseFocus(&s_stESpin[s_ucOilCtrlId]);
                            if(s_ucOilCtrlId<s_ucOilCtrlMaxId)
                            {
                                s_ucOilCtrlId++;
                                ESpinSetFocus(&s_stESpin[s_ucOilCtrlId]);
                            }
                            else 
                            {
                                s_ucOilCtrlId = 0;                            
                                SpinSetFocus(&s_stSpin[s_ucOilCtrlId]);
                            }
                        }
                    }
                    break;
            }
            return 1;
        case TEST_TIMER:
            //LZM_StartNewTask(TaskCalibOilPointNumber);//系统管理
            break;
        case INT_RTC:
            break;
        default:
            return LZM_DefTaskProc(Msg);
    }
    return 0;
}
#if 0
/*************************************************************
** 函数名称: TaskOilPointShow
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskOilPointShow(LZM_MESSAGE *Msg)
{
    unsigned short val;
    float fv;
    char buffer[30];
    switch(Msg->message)
    {
        case TASK_START:
            LcdClearScreen();
            s_ucOilCtrlId=0;
            LcdShowCaption("油量",1);
            LcdShowHorLine(0, 131, 19, 1);
            LcdShowStr(0, 24, "测量值:", 0);          
            ESpinInit(&s_stESpin[1], 64, 25, 5,0);
            ESpinShow(&s_stESpin[1]);
            ESpinSetFocus(&s_stESpin[1]);
            return 1;
        case KEY_PRESS:
            if(s_ucOilCtrlId==0xff)break;
            switch(TaskPara)
            {
                case KEY_BACK://打印
                    LZM_ReturnOldTaskEx();
                    break; 
                case KEY_ENTER://菜单
                    val=atoi(s_stESpin[1].buffer);
                    fv=OilWear_GetCalibTestValue(val); 
                    sprintf(buffer,"校准值:%5.1f ",fv);
                    LcdShowStr(0, 42, buffer,0);
                    break;                               
                default:
                    ESpinResponse(&s_stESpin[1],TaskPara);
                    break;
            }
            return 1;
        case INT_RTC:
            break;
        default:
            return LZM_DefTaskProc(Msg);
    }
    return 0;
}
#endif
#endif
/*************************************************************
** 函数名称: OilWear_Updata16PINPortPeripheralParam
** 功能描述: 更新16pin接口接的外设参数
** 入口参数: (沿用EGS702的32pin的Pos接口参数ID)
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void OilWear_Updata16PINPortPeripheralParam(void)
{
    unsigned char buffer[4];
    if(EepromPram_ReadPram(E2_POS_PORT_ID, buffer))
    {
        s_us16PortPeripheralUse = Public_ConvertBufferToShort(buffer);
    }
}
/*************************************************************
** 函数名称: OilWear_Get16PINPortInsertBSJOilFlag
** 功能描述: 16pin接口接的外设类型,是否接入博实结油位传感器模块
** 入口参数: 
** 出口参数: 
** 返回参数: 1:接入,0:没有接入
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char OilWear_Get16PINPortInsertBSJOilFlag(void)
{
    return PUBLIC_CHECKBIT_EX(s_us16PortPeripheralUse,PINPORT_BIT_BSJ_OIL);
}
/*************************************************************
** 函数名称: OilWear_Get16PINPortInsertJointechOilFlag
** 功能描述: 16pin接口接的外设类型,是否接入久通机电油位传感器模块
** 入口参数: 
** 出口参数: 
** 返回参数: 1:接入,0:没有接入
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char OilWear_Get16PINPortInsertJointechOilFlag(void)
{
    return PUBLIC_CHECKBIT_EX(s_us16PortPeripheralUse,PINPORT_BIT_JOINTECH_OIL);
}

/*******************************************************************************
 *                             end of module
 *******************************************************************************/


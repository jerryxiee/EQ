/*******************************************************************************
 * File Name:			oilwearCalib.c 
 * Function Describe:	�ͺ�У׼
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
    PINPORT_BIT_POS     = 0,     //�Ƽ���
    PINPORT_BIT_CAN_BUS,        //CAN-BUS��Ϣ�ɼ�ģ��
    PINPORT_BIT_JOINTECH_OIL,   //��ͨ������λ������
    PINPORT_BIT_BSJ_OIL,        //��ʵ����λ������
    PINPORT_BIT_MAX             //���ֵ
}E_16PINPORT;//����16pin�ӿڽӵ���������,

static unsigned short s_us16PortPeripheralUse = 0;
////////////////////////////////////////////
#ifdef CALIB_OIL_WEAR
//#define CALI_DEBUG_SHOW
#define USE_SMOOTH_DATA  
/////////////////////////////
#define MAX_SMOOTH_DATA_LEN 11 //ȡ�м�ֵ���Ϊ����
typedef struct 
{
     unsigned short originalData[MAX_SMOOTH_DATA_LEN];//
     unsigned char datalen;//��ǰ�������ݳ���
     unsigned char index;//��ǰ��������ݵ��±�
}STSMOOTHDATA;

static STSMOOTHDATA s_stSmoothData;
////////////////////////////////////////////
typedef struct 
{
    unsigned short testVal[OIL_CALIB_MAX_NUM];//�Ŵ�10������
    unsigned short oilValue[OIL_CALIB_MAX_NUM];//�Ŵ�10������
    unsigned short fullOil;//���������Ŵ�10��
    unsigned char CalibPoint;//�궨����
    unsigned char saveFlag;//�����־
}STCALIBOILSAVE;
//////////////////////////
typedef struct 
{
    float coef[OIL_CALIB_MAX_NUM];
    float c[OIL_CALIB_MAX_NUM];//�Ŵ�10��
    unsigned char calFlag;//����ϵ����־
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
*Describe: ��õ�ǰ�ͺĵ���ֵ(�Ŵ�10��)
*��ڲ���: 
**************************************************************/
unsigned short OilWear_GetCurOilResVal(void)
{
    return s_usCurOilResVal;
}
/**************************************************************
*Function: OilWear_GetCurOilVal
*Describe: ��õ�ǰ�ͺ�����ֵ(1/10L)
*��ڲ���: 
**************************************************************/
unsigned short OilWear_GetCurOilVal(void)
{
    return s_usCurOilVal;
}
/*******************************************************************************
** ��������: OilWear_GetSubjoinInfoCurOilVal
** ��������: ȡ�õ�ǰ����������Ϣ��(����������ϢID�����ȡ�������Ϣ��)
** ��ڲ���: ��Ҫ��������������Ϣ�����׵�ַ
** ���ڲ���: 
** ���ز���: ����������Ϣ�����ܳ���
*******************************************************************************/
unsigned char OilWear_GetSubjoinInfoCurOilVal(unsigned char *data)
{
    unsigned char buffer[10]={0};
    unsigned char len;
    unsigned short usVal;
    if(!OilWear_Get16PINPortInsertBSJOilFlag())return 0;//δ���벩ʵ����λ������
    len = 0;
    usVal = OilWear_GetCurOilVal();//(1/10L)
    buffer[len++] = 0x02;//������ϢID
    buffer[len++] = 2;   //���ӳ���
    Public_ConvertShortToBuffer(usVal,&buffer[len]);//������Ϣ��
    //////////////////////////
    len += 2;
    ///////////////
    memcpy(data,buffer,len);
    /////////////////
    return len;
}
/*******************************************************************************
** ��������: OilWear_GetSubjoinInfoCurOilResVal
** ��������: ȡ�õ�ǰ��������ֵ������Ϣ��(����������ϢID�����ȡ�������Ϣ��)
** ��ڲ���: ��Ҫ������������ֵ������Ϣ�����׵�ַ
** ���ڲ���: 
** ���ز���: ��������ֵ������Ϣ�����ܳ���
*******************************************************************************/
unsigned char OilWear_GetSubjoinInfoCurOilResVal(unsigned char *data)
{
    unsigned char buffer[10]={0};
    unsigned char len;
    unsigned short usVal;
    if(!OilWear_Get16PINPortInsertBSJOilFlag())return 0;//δ�Ӳ�ʵ����λ������
    len = 0;
    usVal = OilWear_GetCurOilResVal()/10;
    buffer[len++] = 0xE5;//������ϢID
    buffer[len++] = 2;   //���ӳ���
    Public_ConvertShortToBuffer(usVal,&buffer[len]);//������Ϣ��
    //////////////////////////
    len += 2;
    ///////////////
    memcpy(data,buffer,len);
    /////////////////
    return len;
}
/**************************************************************
*Function: OilWear_UartDataProtocolParse
*Describe: �ͺ����ݽ���
*��ڲ���: pBuffer�����׵�ַ,Len���ݳ���
*ע��:     24 F1 05 X1 X2 X3 4F FF ,"����ͺ�"BSJЭ��
           24 F1 05 03 99 02 94 FF ,"����ͺ�"����GMЭ�� 
**************************************************************/
void OilWear_UartDataProtocolParse(unsigned char *pBuffer,unsigned short Len) 
{
    u8 *Ptr;     //ָ���������
    u16 i;
    u16 DataLen,oil,tmplen; //���ݳ���    
    u8 ChkSum;  //24 F1 05 00 77 03 A4 FF  ,A4֮ǰ�����ֽ����
    u8 ChkSum2; //24 F1 05 03 99 02 94 FF  ,94֮ǰ��24����������֮��
    u8 flag; 
    if(!OilWear_Get16PINPortInsertBSJOilFlag())return;//δ�Ӳ�ʵ����λ������
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
    if(0==flag)return; //δ�ҵ�ͷ
    if(tmplen<3)return;//���ȹ�С
    //ȡ�����ݰ�����
    DataLen = *(Ptr+2); 
    //////////////////
    if(tmplen<DataLen+2)return;//���ȹ�С
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
*Describe: ��������,��С�������򣬶�Դ�������򣬵�ֻ�����������±�,Դ�������в���
*��ڲ���: SortDataAddrΪҪ����������׵�ַ,suffΪԭ�±��˳��low Ϊ��λ��highΪ��λ
*ע��:ѭ�����ú����ڲ�������static����
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
            Suff[i++]=Suff[j]; //���������¼С�ļ�¼�Ƶ��Ͷ� 

        while (i<j && SortDataAddr[Suff[i]]<=SortDataAddr[pivot]) 
            i++; 
        if(i<j)  
            Suff[j--]=Suff[i]; //���������¼��ļ�¼�Ƶ��߶�
    }
    Suff[i]=pivot; //�����¼�Ƶ�����λ��		
    if(tmplow+1<i)
        Pulbic_QuickSortByAsc(SortDataAddr,Suff,tmplow,i-1); 
    if(i+1<tmphigh)
        Pulbic_QuickSortByAsc(SortDataAddr,Suff,i+1,tmphigh); 
}
/*************************************************************
** ��������: OilWear_GetMiddleValue
** ��������: ȡ�����ݵ��м�ֵ
** ��ڲ���: data�����׵�ַ,len���ݳ���
** ���ڲ���:
** ���ز���: �м�ֵ
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/	
unsigned short OilWear_GetMiddleValue(const unsigned short *data,unsigned char len)
{
    unsigned char SortNo[MAX_SMOOTH_DATA_LEN];
    unsigned char i,midindex;
    ///////////////////////
    if(0==len||len>MAX_SMOOTH_DATA_LEN)//����Ϊ0������򳬳������
    {
        return 0;
    }
    else
    if(len<3)//����<3�����
    {
        return data[len-1];
    }    
    ////������������������/////////////////
    for(i=0; i<len;i++)
    {
        SortNo[i]=i;
    }
    ////////////////////
    midindex =((len-1)>>1);//ȡ�������м�ֵ�±�
    ////�����ݽ�������/////////////////////
    Pulbic_QuickSortByAsc(data,SortNo,0,len-1);
    ///////////////////////
    return data[SortNo[midindex]];
}
/*************************************************************
** ��������: OilWear_ReceiveAndDisposeData
** ��������: ����ԭʼ���ݲ�����ƽ������(11����,��С���������,ȡ�м�ֵ)
** ��ڲ���: testValԭʼ����
** ���ڲ���: ��
** ���ز���: ƽ��֮�������
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/	
unsigned short OilWear_ReceiveAndDisposeData(unsigned short testVal)
{
    static unsigned char sucFirstRunFlag=0;//
    if(0==sucFirstRunFlag)//�״ε���
    {
        sucFirstRunFlag =1;
        s_stSmoothData.datalen = 0;
        s_stSmoothData.index = 0;
    }
    ////////���浱ǰ����//////////////////
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
** ��������: OilWear_GetCalibCoefIndex
** ��������: ���ݲ���ֵ���У׼ϵ���±�
** ��ڲ���: ����ֵ(�Ŵ�10��)
** ���ڲ���: ��
** ���ز���: �±�
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/	
unsigned char OilWear_GetCalibCoefIndex(unsigned short val)
{
    unsigned char i;
    unsigned char pointnum;
    pointnum=s_stCalibOilSave.CalibPoint-1;
    if(s_stCalibOilSave.testVal[0]<s_stCalibOilSave.testVal[1])//��С����
    {
        if(val<=s_stCalibOilSave.testVal[0])//С����С
        {
            return 0;
        }
        else
        if(val>=s_stCalibOilSave.testVal[pointnum-1])//�������
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
    if(s_stCalibOilSave.testVal[0]>s_stCalibOilSave.testVal[1])//�Ӵ�С
    {
        if(val>=s_stCalibOilSave.testVal[0])//�������
        {
            return 0;
        }
        else
        if(val<=s_stCalibOilSave.testVal[pointnum-1])//С����С
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
** ��������: OilWear_GetCalibTestValue
** ��������: ���ݲ���ֵ���ʵ������ֵ
** ��ڲ���: ����ֵ(�Ŵ�10��)
** ���ڲ���: ��
** ���ز���: ʵ������ֵ(1/10L)
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/	
float OilWear_GetCalibTestValue(unsigned short testVal)
{
    unsigned char k;
    unsigned short value;
    float val,result;
    //////////////////
    OilWear_UpdatePram();//��ȡϵ��
    value = testVal;
    #ifdef USE_SMOOTH_DATA
    value=OilWear_ReceiveAndDisposeData(testVal);
    #endif    
    ////////////////////
    if(0==s_stCalibOilCal.calFlag)return value;
    ////////////////// 
    k = OilWear_GetCalibCoefIndex(value);
    ///////////////////////
    val = value;//����ֵ�ѷŴ�10��
    result= val*s_stCalibOilCal.coef[k] + s_stCalibOilCal.c[k];//c�ѷŴ�10������
    //////����Ŵ�10��/////////////////////////////
    /////////�������ж�///////////////
    if(result< 0.0)result = 0.0;
    ////////�������ж�//////////////////
    if(result>s_stCalibOilSave.fullOil)result=s_stCalibOilSave.fullOil;
    ///////////////////////////////////
    #ifdef CALI_DEBUG_SHOW    
    Public_ShowValue(0,16,"ƽ��ǰ%d  ",(unsigned long)testVal);
    Public_ShowValue(0,32,"ƽ����%d  ",(unsigned long)val);
    Public_ShowValue(0,48,"������%d  ",(unsigned long)result);    
    #endif
    return result;
}
/*************************************************************
** ��������: OilWear_SetCalibOilSaveValue
** ��������: �������궨ֵ���浽�ṹ����
** ��ڲ���: index������(��0��ʼ),oilVal:����ֵ(�Ŵ�10��,����λΪ0.1L),testVal:����ֵ(�Ŵ�10��)
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/	
void OilWear_SetCalibOilSaveValue(unsigned char index,unsigned short oilVal,unsigned short testVal)
{
    s_stCalibOilSave.oilValue[index]= oilVal;
    s_stCalibOilSave.testVal[index]= testVal;
}
/*************************************************************
** ��������: OilWear_SetCalibOilSaveValue
** ��������: �������궨ֵ���浽�ṹ����
** ��ڲ���: index������(��0��ʼ),oilVal:����ֵ(�Ŵ�10��,����λΪ0.1L),testVal:����ֵ(�Ŵ�10��)
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/	
void OilWear_GetCalibOilValue(unsigned char index,unsigned short *oilVal,unsigned short *testVal)
{
   *oilVal = s_stCalibOilSave.oilValue[index];
    *testVal=s_stCalibOilSave.testVal[index];
}
/*************************************************************
** ��������: OilWear_SetCalibPoint
** ��������: ���������궨����
** ��ڲ���: CalibPoint�궨����
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/	
void OilWear_SetCalibPoint(unsigned char CalibPoint)
{
    s_stCalibOilSave.CalibPoint = CalibPoint;
}
/*************************************************************
** ��������: OilWear_SetCalibPoint
** ��������: ���������궨����
** ��ڲ���: CalibPoint�궨����
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/	
unsigned char OilWear_GetCalibPoint(void)
{
    return s_stCalibOilSave.CalibPoint;
}
/*************************************************************
** ��������: OilWear_SetCalibFullOil
** ��������: ����������
** ��ڲ���: fullOil������(�Ŵ�10��)
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/	
void OilWear_SetCalibFullOil(unsigned short fullOil)
{
    s_stCalibOilSave.fullOil = fullOil;
}
/*************************************************************
** ��������: OilWear_SetCalibFullOil
** ��������: ����������
** ��ڲ���: fullOil������(�Ŵ�10��)
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/	
unsigned short OilWear_GetCalibFullOil(void)
{
    return s_stCalibOilSave.fullOil;
}
/*************************************************************
** ��������: OilWear_SaveCalibOilParamToFram
** �����������������궨�����ṹ�屣�浽������
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/	
void OilWear_SaveCalibOilParamToFram(void)
{
    s_stCalibOilSave.saveFlag = CALI_SAVE;
    FRAM_BufferWrite(FRAM_OIL_WEAR_CALIB_ADDR,(unsigned char *)&s_stCalibOilSave,STCALIBOILSAVE_SIZE);
}
/*************************************************************
** ��������: OilWear_CalCoefAndC
** ��������������ϵ�� 
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
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
** ��������: OilWear_CalCoefAndC
** �������������㵱ǰ�궨ϵ�� 
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
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
    //����б�ʺͳ���c
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
** ��������: OilWear_UpdatePram
** ��������: ��ȡ�����궨����������ϵ��
** ��ڲ���: 
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: ��
** ����ģ��: ��
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
** ��������: TaskCalibOilPointNumber
** ��������: 
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/	
LZM_RET TaskCalibOilPointNumber(LZM_MESSAGE *Msg)
{
    unsigned char val;
    switch(Msg->message)
    {
        case TASK_START:
            LcdClearScreen();
            s_ucOilCtrlId=0;
            LcdShowCaption("�����궨",1);
            LcdShowHorLine(0, 131, 19, 1);
            LcdShowStr(0, 24, "�궨����:", 0);
            LcdShowStr(0, 44, "������:       L", 0);
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
                case KEY_BACK://��ӡ
                    LZM_StartNewTask(TaskMenuSystemAdvanced);//ϵͳ����
                    break; 
                case KEY_ENTER://�˵�
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
** ��������: OilWear_CalibOilDemarcateSaveModify
** ��������: 
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/	
void OilWear_CalibOilDemarcateSaveModify(void)
{
    LZM_SetAlarm(TEST_TIMER, PUBLICSECS(2));
    LcdClearScreen();
    LcdShowCaption("������,���Ժ�", 24);
    OilWear_SaveCalibOilParamToFram();
    OilWear_CalCoefAndC();
}
/*************************************************************
** ��������: OilWear_CalibOilDemarcateCancelModify
** ��������: 
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
*************************************************************/	
void OilWear_CalibOilDemarcateCancelModify(void)
{
    OilWear_ReadCalibOilParam();
    LZM_StartNewTask(TaskMenuSystemAdvanced);//
}
/*************************************************************
** ��������: OilWear_CalibOilDemarcateReflashValue
** ��������: 
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
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
** ��������: OilWear_DisposeEIExpandProtocol
** ��������: �ͺ�������չЭ��
** ��ڲ���: pBuffer:��Ϣ�������׵�ַ,BufferLen��Ϣ�����ݳ���
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
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
    if((3+num*4) != BufferLen)return 0;//���Ȳ���
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
** ��������: OilWear_DisposeEIExpandProtocol
** ��������: �ͺ�������չЭ��
** ��ڲ���: pBuffer:��Ϣ�������׵�ַ,BufferLen��Ϣ�����ݳ���
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void OilWear_DisposePlatformReadParameter(void)
{
    unsigned char buffer[128];
    unsigned short len,val,test;
    unsigned char i,num;
    len = 0;
    buffer[len++] = 1;//������
    buffer[len++] = 0;//����
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
** ��������: OilWear_DisposeEIExpandProtocol
** ��������: �ͺ�������չЭ��
** ��ڲ���: pBuffer:��Ϣ�������׵�ַ,BufferLen��Ϣ�����ݳ���
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
unsigned char OilWear_DisposeEIExpandProtocol(unsigned char *pBuffer, unsigned short BufferLen)
{
    unsigned char operate;
    unsigned char *p;
    p = pBuffer;
    operate = *p++;//������־   
    if(1==operate)//������
    {
        Public_SetOperateTimer(OilWear_DisposePlatformReadParameter,PUBLICSECS(2));
        return 1;
    }
    else
    if(2==operate)//д
    {
       p++;//�����ֽ� 
       return OilWear_DisposePlatformSetParameter(p,BufferLen-2);       
    }
    return 1;
}
/*************************************************************
** ��������: TaskCalibOilPointNumber
** ��������: 
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
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
            LcdShowStr(2, 2, "�����궨     ��", 0);
            LcdShowHorLine(0, 131, 20, 1);
            LcdShowStr(0, 24, "��  ��:       L", 0);
            LcdShowStr(0, 44, "����ֵ:", 0);
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
                case KEY_BACK://��ӡ
                    PublicConfirmAll("�Ƿ񱣴��޸�?",OilWear_CalibOilDemarcateSaveModify,OilWear_CalibOilDemarcateCancelModify,0,0);
                    break; 
                case KEY_ENTER://�˵�
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
                        PublicConfirmAll("�Ƿ񱣴��޸�?",OilWear_CalibOilDemarcateSaveModify,OilWear_CalibOilDemarcateCancelModify,0,1);
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
            //LZM_StartNewTask(TaskCalibOilPointNumber);//ϵͳ����
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
** ��������: TaskOilPointShow
** ��������: 
** ��ڲ���: ��
** ���ڲ���: ��
** ���ز���: ��
** ȫ�ֱ���: 
** ����ģ��: 
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
            LcdShowCaption("����",1);
            LcdShowHorLine(0, 131, 19, 1);
            LcdShowStr(0, 24, "����ֵ:", 0);          
            ESpinInit(&s_stESpin[1], 64, 25, 5,0);
            ESpinShow(&s_stESpin[1]);
            ESpinSetFocus(&s_stESpin[1]);
            return 1;
        case KEY_PRESS:
            if(s_ucOilCtrlId==0xff)break;
            switch(TaskPara)
            {
                case KEY_BACK://��ӡ
                    LZM_ReturnOldTaskEx();
                    break; 
                case KEY_ENTER://�˵�
                    val=atoi(s_stESpin[1].buffer);
                    fv=OilWear_GetCalibTestValue(val); 
                    sprintf(buffer,"У׼ֵ:%5.1f ",fv);
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
** ��������: OilWear_Updata16PINPortPeripheralParam
** ��������: ����16pin�ӿڽӵ��������
** ��ڲ���: (����EGS702��32pin��Pos�ӿڲ���ID)
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
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
** ��������: OilWear_Get16PINPortInsertBSJOilFlag
** ��������: 16pin�ӿڽӵ���������,�Ƿ���벩ʵ����λ������ģ��
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 1:����,0:û�н���
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
unsigned char OilWear_Get16PINPortInsertBSJOilFlag(void)
{
    return PUBLIC_CHECKBIT_EX(s_us16PortPeripheralUse,PINPORT_BIT_BSJ_OIL);
}
/*************************************************************
** ��������: OilWear_Get16PINPortInsertJointechOilFlag
** ��������: 16pin�ӿڽӵ���������,�Ƿ�����ͨ������λ������ģ��
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 1:����,0:û�н���
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
unsigned char OilWear_Get16PINPortInsertJointechOilFlag(void)
{
    return PUBLIC_CHECKBIT_EX(s_us16PortPeripheralUse,PINPORT_BIT_JOINTECH_OIL);
}

/*******************************************************************************
 *                             end of module
 *******************************************************************************/


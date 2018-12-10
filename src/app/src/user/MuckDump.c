/*******************************************************************************
 * File Name:			MuckDump.c 
 * Function Describe:	
 * Relate Module:		�����㵹������ 
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
#define MUCK_DUMP_HEAD_SIZE             8//��ȡͷ��־������ID
#define MAX_MUCK_DUMP_AREA_SUM          100  //��������㵹������

//////////////////////////////////////
#define MUCK_DUMP_AREA_START            (unsigned long)(FLASH_MUCK_DUMP_AREA_START_SECTOR*FLASH_ONE_SECTOR_BYTES) //�����㵹�����ŵ�,������Ҫ��0��ʼ....
#define MUCK_DUMP_AREA_END              (unsigned long)((FLASH_MUCK_DUMP_AREA_END_SECTOR)*FLASH_ONE_SECTOR_BYTES)

//�����㵹�������Խṹ	24�ֽ�
typedef struct
{
    unsigned short Head;            //ͷ��־    
    unsigned short AreaAttrib;      //�������� 
    unsigned long AreaID;           //����ID
    unsigned long LeftUpLat;        //���ϵ�γ��
    unsigned long LeftUpLong;       //���ϵ㾭��
    unsigned long RightDownLat;     //���µ�γ��
    unsigned long RightDownLong;    //���µ㾭��
}STMUCK_DUMP_AREA;

#define STMUCK_DUMP_AREA_SIZE sizeof(STMUCK_DUMP_AREA)
//�ṹ
typedef struct
{
    unsigned long Latitude;                     //γ����������
    unsigned long Longitude;                    //������������   
    unsigned long addr;                         //��ַ//��ΪMAX_AREA_NUMʱ,��ʾû�н����κ����� 
    unsigned long AreaID;                       //����ID 
    unsigned short AreaAttrib;                  //��������
    unsigned short AreaNum;             //�����㵹��������
    unsigned char preSecond;                    //��ǰ����
    unsigned char preSpeed;                     //ǰһ����ٶ�
    unsigned char nonavigationCnt;              //������������
    unsigned char AreaCnt;          //���������
    unsigned char preAreaCnt;       //���������
    unsigned char AreaFlag;         //�����־
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
** ��������: MuckDump_ResetParam
** ��������: �����㵹�������ò���
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void MuckDump_ResetParam(void) 
{
    s_stMuckDAttrib.AreaCnt  = 0;
    s_stMuckDAttrib.addr     = MAX_MUCK_DUMP_AREA_ADDR;
    s_stMuckDAttrib.AreaFlag = MD_AREA_STATUS_MAX;
    ///////////////////////////////
}
/*************************************************************
** ��������: MuckDump_CheckForResetParam
** ��������: �����㵹�������Ƿ����ò���
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void MuckDump_CheckForResetParam(unsigned long AreaID) 
{
    if(s_stMuckDAttrib.AreaFlag==MD_AREA_STATUS_IN&&s_stMuckDAttrib.AreaID==AreaID)
    {
        MuckDump_ResetParam();
    }
}
/*************************************************************
** ��������: MuckDump_ParamInit
** ��������: �����㵹���������ʼ��
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void MuckDump_ParamInit(void) 
{
    MuckDump_ResetParam();
    s_stMuckDAttrib.AreaNum = 0;
}
/*************************************************************
** ��������: MuckDump_ClearArea
** ��������: ��������㵹����
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void MuckDump_ClearArea(void)
{
    MuckDump_ParamInit();
    Area_EraseFlash(FLASH_MUCK_DUMP_AREA_START_SECTOR,FLASH_MUCK_DUMP_AREA_END_SECTOR); //��������
}

/*************************************************************
** ��������: MuckDump_GetCurGpsCoordinate
** ��������: ��õ�ǰ����
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
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
//����:�жϵ�ǰ������Ƿ���Ptr��ָ�������㵹������
//����:ָ�������ָ��
//���:0:��  1:����  0XFF:����
//��ע:
//-----------------------------------------------
unsigned char MuckDump_IsInArea(STMUCK_DUMP_AREA *Ptr)
{
    ///////////////////////////
    return( ComparePos( (T_GPS_POS *)&(Ptr->LeftUpLat), (T_GPS_POS *)&(Ptr->RightDownLat),(T_GPS_POS *)&s_stMuckDAttrib.Latitude));
}
//-----------------------------------------------
//����:ɨ�������㵹����
//����:
//���:
//��ע:ÿ��ִ��һ��
//-----------------------------------------------
void MuckDump_ScanArea(void)
{
    unsigned char i;
    unsigned long addr;
    STMUCK_DUMP_AREA stMuckDumpArea;	  //��ȡ�����㵹��������
    if(s_stMuckDAttrib.AreaFlag != MD_AREA_STATUS_IN)
    {
        if(s_stMuckDAttrib.addr == MAX_MUCK_DUMP_AREA_ADDR)
        {
            for(i=0; s_stMuckDAttrib.AreaCnt<s_stMuckDAttrib.AreaNum && i<10; s_stMuckDAttrib.AreaCnt++,i++)
            {
                addr=MUCK_DUMP_AREA_START + s_stMuckDAttrib.AreaCnt*STMUCK_DUMP_AREA_SIZE ;  
                sFLASH_ReadBuffer((unsigned char *)&stMuckDumpArea,addr,STMUCK_DUMP_AREA_SIZE);//������������	
                if(stMuckDumpArea.AreaID ==0) //����ID=0
                    continue;
                if(0==MuckDump_IsInArea(&stMuckDumpArea))
                {//��������
                    s_stMuckDAttrib.addr     = addr;                    
                    s_stMuckDAttrib.AreaID   = stMuckDumpArea.AreaID;
                    s_stMuckDAttrib.AreaFlag = MD_AREA_STATUS_IN;
                    break;
                }
            }            
            if(s_stMuckDAttrib.AreaCnt>=s_stMuckDAttrib.AreaNum)s_stMuckDAttrib.AreaCnt = 0;  //���¼���
            ////////////////////////
            if((MAX_MUCK_DUMP_AREA_ADDR == s_stMuckDAttrib.addr) && (s_stMuckDAttrib.preAreaCnt != s_stMuckDAttrib.AreaCnt))LZM_PublicSetOnceTimer(&s_stMDTimer[MD_TIMER_TEST],PUBLICSECS(0.2),MuckDump_ScanArea);
            //////////////////////////
        }
    }
    else
    if(MD_AREA_STATUS_IN == s_stMuckDAttrib.AreaFlag)
    {
        sFLASH_ReadBuffer((unsigned char *)&stMuckDumpArea,s_stMuckDAttrib.addr,STMUCK_DUMP_AREA_SIZE);//������������
        if(MuckDump_IsInArea(&stMuckDumpArea))
        {
            s_stMuckDAttrib.AreaFlag = MD_AREA_STATUS_OUT; 
            s_stMuckDAttrib.addr     = MAX_MUCK_DUMP_AREA_ADDR;
        }
    }
}
/*************************************************************
** ��������: MuckDump_GetAreaDatalen
** ��������: :��������ʱ,��׼Э�鳤��
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
unsigned short MuckDump_GetAreaDatalen(unsigned char *pBuffer, unsigned short BufferLen,unsigned long *AreaID)
{
    unsigned short datalen;
    STMUCK_DUMP_AREA stMuckDumpArea;  //��ȡ�����㵹��������
    datalen =0;
    //�������ID
    Public_Mymemcpy((unsigned char *)&stMuckDumpArea.AreaID,(unsigned char *)&pBuffer[datalen],sizeof(unsigned long),1); 
    datalen+=sizeof(unsigned long);
    //�����������
    Public_Mymemcpy((unsigned char *)&stMuckDumpArea.AreaAttrib,(unsigned char *)&pBuffer[datalen],sizeof(unsigned short ),1); 
    datalen+=sizeof(unsigned short ); 
     //������ϵ�γ��
    datalen+=sizeof(unsigned long);  
    //������ϵ㾭��
    datalen+=sizeof(unsigned long);  
    
    //������µ�γ��
    datalen+=sizeof(unsigned long);  
    //������µ㾭��
    datalen+=sizeof(unsigned long);      
    ///////////////////
    if(datalen>BufferLen)return 0;
    ///////////////////////
    *AreaID =stMuckDumpArea.AreaID;
    return datalen;
}
/*************************************************************
** ��������: MuckDump_SetAreaCheckDatalen
** ��������: :��������ʱ,��쳤���Ƿ���ȷ
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
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
        if(0 == templen)//����
        {
            break;
        }
        pTmp += templen;
        //////////////////////////
        if(datalen == templen)//���,��ȷ
        {
            return 1;
        }
        else
        if(datalen > templen)//����,��������
        {
            datalen -= templen;
        }
        else////С��,����
        {
            break;
        }
    }
   
    return 0;
}
/*********************************************************************
//��������	:MuckDump_SaveOneArea
//����		:���������㵹����
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:
//���ú���	:
//�ж���Դ	:
//����		:
//��ע		:
*********************************************************************/
unsigned short MuckDump_SaveOneArea(unsigned long addr,unsigned char *pBuffer, unsigned short BufferLen)
{
    unsigned short datalen;
    STMUCK_DUMP_AREA stMuckDumpArea;  //��ȡ�����㵹��������    
    if(addr+STMUCK_DUMP_AREA_SIZE>MUCK_DUMP_AREA_END)return 0;
    memset(&stMuckDumpArea,0,sizeof(STMUCK_DUMP_AREA));
    datalen = 0;
    Public_Mymemcpy((unsigned char *)&stMuckDumpArea.AreaID,(unsigned char *)&pBuffer[datalen],sizeof(unsigned long),1); //�������ID
    datalen+=sizeof(unsigned long);
    Public_Mymemcpy((unsigned char *)&stMuckDumpArea.AreaAttrib,(unsigned char *)&pBuffer[datalen],sizeof(unsigned short ),1); //�����������
    datalen+=sizeof(unsigned short ); 
    Public_Mymemcpy((unsigned char *)&stMuckDumpArea.LeftUpLat,(unsigned char *)&pBuffer[datalen],sizeof(unsigned long),1); //������ϵ�γ��
    datalen+=sizeof(unsigned long);  
    Public_Mymemcpy((unsigned char *)&stMuckDumpArea.LeftUpLong,(unsigned char *)&pBuffer[datalen],sizeof(unsigned long),1); //������ϵ㾭��
    datalen+=sizeof(unsigned long);  
    
    Public_Mymemcpy((unsigned char *)&stMuckDumpArea.RightDownLat,(unsigned char *)&pBuffer[datalen],sizeof(unsigned long),1); //������µ�γ��
    datalen+=sizeof(unsigned long);  
    Public_Mymemcpy((unsigned char *)&stMuckDumpArea.RightDownLong,(unsigned char *)&pBuffer[datalen],sizeof(unsigned long),1); //������µ㾭��
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
//��������	:MuckDump_UpdateArea(unsigned char *pBuffer, unsigned short BufferLen)
//����		:���������㵹����
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:
//���ú���	:
//�ж���Դ	:
//����		:
//��ע		:
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
    //�����㵹
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
//��������	:MuckDump_AddArea(unsigned char *pBuffer, unsigned short BufferLen)
//����		:���������㵹����
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:
//���ú���	:
//�ж���Դ	:
//����		:
//��ע		:
*********************************************************************/
unsigned char MuckDump_AddArea(unsigned char *pBuffer, unsigned short BufferLen)
{
    unsigned long OffSetAddr,AreaID;
    unsigned char i,j,Sum;
    STMUCK_DUMP_AREA stMuckDumpArea;  //��ȡ�����㵹��������
    unsigned short datalen,templen;
    unsigned char *pTmp;
    //////////////
    if(BufferLen==0)return 2;
    ////////////
    Sum=*pBuffer++;
    /////////////////////////
    if(Sum+s_stMuckDAttrib.AreaNum>MAX_MUCK_DUMP_AREA_SUM)return 1;//ʧ��
    //////////////////////////
    pTmp =pBuffer;
    datalen=BufferLen-1;
    //�����㵹
    for(i=0; i<Sum; i++)//���ID
    {    
        templen=MuckDump_GetAreaDatalen(pTmp,datalen,&AreaID);
        if(0 == templen)return 2;//��Ϣ����;
        //////////////////////////
        for(j=0; j<s_stMuckDAttrib.AreaNum; j++)
        {
            OffSetAddr=MUCK_DUMP_AREA_START + j*STMUCK_DUMP_AREA_SIZE;
            sFLASH_ReadBuffer((unsigned char *)&stMuckDumpArea,OffSetAddr,MUCK_DUMP_HEAD_SIZE);
            if(AreaID==stMuckDumpArea.AreaID)
            {
                return 1;////ʧ��
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
//��������	:MuckDump_ModifyArea(unsigned char *pBuffer, unsigned short BufferLen)
//����		:�޸������㵹����
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:
//���ú���	:
//�ж���Դ	:
//����		:
//��ע		:
*********************************************************************/
unsigned char MuckDump_ModifyArea(unsigned char *pBuffer, unsigned short BufferLen)
{
    unsigned long OffSetAddr,AreaID;
    unsigned char i,j,Sum,flag;
    STMUCK_DUMP_AREA stMuckDumpArea;  //��ȡ�����㵹��������
    unsigned short datalen,templen;
    unsigned char *pTmp;
    //////////////
    if(BufferLen==0)return 2;
    ////////////
    Sum=*pBuffer++;
    pTmp =pBuffer;
    datalen=BufferLen-1;
    //�����㵹
    for(i=0; i<Sum; i++)//���ID
    {  
        templen=MuckDump_GetAreaDatalen(pTmp,datalen,&AreaID);        
        if(0 == templen)return 2;//��Ϣ����;
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
        if(0==flag)return 1;//ʧ��;
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
        Public_Mymemcpy((unsigned char *)&AreaID,(unsigned char *)&pTmp[0],sizeof(unsigned long),1); //�������ID
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
** ��������: MuckDump_SetArea
** ��������: :���������㵹����(0x860a)
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
unsigned char MuckDump_SetArea(unsigned char *pBuffer, unsigned short BufferLen)
{
    unsigned char AckResult,type;
    if(BufferLen==0)return 2;
    /////////////////
    type=*pBuffer++;
    //////////////
    AckResult = 0;
    if(0== type)//����
    {
        AckResult=MuckDump_UpdateArea(pBuffer,BufferLen-1);
    }
    else
    if(1 == type)//���
    {   
        AckResult=MuckDump_AddArea(pBuffer,BufferLen-1);
    }
    else
    if(2 == type)//�޸�
    {  
        AckResult=MuckDump_ModifyArea(pBuffer,BufferLen-1);
    }
    return AckResult;
}
/*************************************************************
** ��������: MuckDump_DeleteArea
** ��������: :ɾ�������㵹����
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
unsigned char MuckDump_DeleteArea(unsigned char *pBuffer, unsigned short BufferLen) 
{
    unsigned char i,j,Sum,flag;
    unsigned long OffSetAddr,AreaID;
    STMUCK_DUMP_AREA stMuckDumpArea;  //�����㵹�������Խṹ
    unsigned char *pTmp;
    
    if(BufferLen==0) return 2;
    Sum=*pBuffer++;    
    if(Sum)
    {
        pTmp =pBuffer;
        for(i=0; i<Sum; i++)
        {
            Public_Mymemcpy((unsigned char *)&AreaID,(unsigned char *)&pTmp[0],sizeof(unsigned long),1); //�������ID
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
            Public_Mymemcpy((unsigned char *)&AreaID,(unsigned char *)&pTmp[0],sizeof(unsigned long),1); //�������ID
            pTmp+=sizeof(unsigned long);
            for(j=0; j<s_stMuckDAttrib.AreaNum; j++)
            {
                OffSetAddr=MUCK_DUMP_AREA_START + j*STMUCK_DUMP_AREA_SIZE;
                sFLASH_ReadBuffer((unsigned char *)&stMuckDumpArea,OffSetAddr,MUCK_DUMP_HEAD_SIZE);
                if(AreaID==stMuckDumpArea.AreaID)//��Ҫɾ��������ID
                {
                    stMuckDumpArea.AreaID = 0;  //ɾ������,ֻ���ID=0
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
    else//ɾ������
    {
        MuckDump_ClearArea();
    }
    //////////////
    return 0;
}
/*************************************************************
** ��������: MuckDump_QueryArea
** ��������: ��ѯ�����㵹����(0x860c)
** ��ڲ���: �����׵�ַ
** ���ڲ���: ���ݳ���
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
unsigned short MuckDump_QueryArea(unsigned char *pBuffer)
{
    STMUCK_DUMP_AREA stMuckDumpArea;  //�����㵹�������Խṹ
    unsigned long OffSetAddr;
    unsigned short len,i;
    unsigned char num;
    unsigned char *buffer;
    
    buffer = pBuffer;
    num = 0;
    len = 1; //��������,������
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
** ��������: MuckDump_GetCurMuckDumpAreaID
** ��������: ��õ�ǰ�����㵹����ID
** ��ڲ���: ��
** ���ڲ���: 
** ���ز���: 0:�����κ������㵹������,����:Ϊ�����㵹����ID
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
unsigned long MuckDump_GetCurMuckDumpAreaID(void)
{
    if(MD_AREA_STATUS_IN != s_stMuckDAttrib.AreaFlag)return 0;
    if(MAX_MUCK_DUMP_AREA_ADDR  == s_stMuckDAttrib.addr)return 0;
    return (0==s_stMuckDAttrib.AreaID)?1:s_stMuckDAttrib.AreaID;
}
/*************************************************************
** ��������: MuckDump_ScanTask
** ��������: 
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void MuckDump_CheckScanTask(void)
{
    unsigned char speed;
    MuckDump_GetCurGpsCoordinate();//1s��ȡһ�ξ�γ��        
    s_stMuckDAttrib.nonavigationCnt =1;  
    s_stMuckDAttrib.preAreaCnt=s_stMuckDAttrib.AreaCnt;
    speed = Area_GetCurSpeed();
    ///////////////////////////
    if(0==speed&&0==s_stMuckDAttrib.preSpeed)return;//�ٶ�Ϊ��,������
    /////////////////////////
    s_stMuckDAttrib.preSpeed = speed;   
    if(s_stMuckDAttrib.AreaNum)
    MuckDump_ScanArea();    
}
/*************************************************************
** ��������: MuckDump_ScanTask
** ��������: 
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
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
** ��������: MuckDump_ScanTask
** ��������: 
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void MuckDump_StartScanTask(void)
{
    LZM_PublicSetCycTimer(&s_stMDTimer[MD_TIMER_TASK],PUBLICSECS(0.2),MuckDump_ScanTask);
}

/*************************************************************
** ��������: MuckDump_ReadAndCheckParam
** ��������: ���������ʼ��
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void MuckDump_ReadAndCheckParam(void) 
{
    unsigned long OffSetAddr;
    unsigned short i;
    
    STMUCK_DUMP_AREA stMuckDumpArea;	  //��ȡ�����㵹��������
       ////////////////
    s_stMuckDAttrib.preSecond       = 0xff;
    s_stMuckDAttrib.nonavigationCnt = 0xff;
    ////////////
    memset(&s_stMuckDAttrib,0,sizeof(s_stMuckDAttrib));
    //�����㵹��ʼ��    
    MuckDump_ParamInit();
    ////////////////////////
    for(i=0; i<MAX_MUCK_DUMP_AREA_SUM; i++)
    {
        OffSetAddr=MUCK_DUMP_AREA_START + i*STMUCK_DUMP_AREA_SIZE ;  
        sFLASH_ReadBuffer((unsigned char *)&stMuckDumpArea,OffSetAddr,MUCK_DUMP_HEAD_SIZE);//������������	
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
** ��������: MuckDump_ParameterInitialize
** ��������: �����㵹���������ʼ��
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
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
** ��������: MuckDump_TimeTask
** ��������: �����ܶ�ʱ��������
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
FunctionalState MuckDump_TimeTask(void)
{
    LZM_PublicTimerHandler(s_stMDTimer,MD_TIMERS_MAX);
    return ENABLE;
}
#else
/*************************************************************
** ��������: MuckDump_ParameterInitialize
** ��������: ���������ʼ��
** ��ڲ���: 
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void MuckDump_ParameterInitialize(void) 
{
}
/*************************************************************
** ��������: MuckDump_GetCurMuckDumpAreaID
** ��������: ��õ�ǰ�����㵹����ID
** ��ڲ���: ��
** ���ڲ���: 
** ���ز���: 0:�����κ������㵹������,����:Ϊ�����㵹����ID
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
unsigned long MuckDump_GetCurMuckDumpAreaID(void)
{
    return 0;
}
#endif
/*******************************************************************************
 *                             end of module
 *******************************************************************************/


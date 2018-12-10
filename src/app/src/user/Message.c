/*******************************************************************************
 * File Name:			message.c 
 * Function Describe:	
 * Relate Module:		信息存储、电话簿、事件报告
 * Writer:				Joneming
 * Date:				2012-05-21
 * ReWriter:			
 * Date:				
 *******************************************************************************/
#include "include.h"
////////////////////////
/*******************************************************************************/
#ifdef MESSAGE
/*******************************************************************************/

extern const char *s_apcMenuShortMsg[];

/////////////////////////////////

/////////////////////////////////
#define MSG_SAVE    0xbb
#define PHONE_HEAD  0xabcd
#define EVENT_HEAD  0xcdab
#define INFO_HEAD   0xdcba
#define DRIVER_HEAD  0xaabb
#define INFO_SERVICE_HEAD  0xccdd
#define QUESTION_HEAD  0xEFCD
//////////////////////////////
#define ONE_DRIVER_INFO_MAX_LEN  144
///////////////////////////////////
typedef struct
{ 
    unsigned char drivercode[6];            //工号
    unsigned char password[6];              //密码  
    unsigned char datalen;                  //驾驶员数据结构有效长度
    unsigned char reverse[3];               //保留   
    unsigned char data[ONE_DRIVER_INFO_MAX_LEN];//符合行标驾驶员刷卡的数据结构
}STDRIVERINFO;

/******************************************************/
/***驾驶员刷卡数据结构**********************************
*0:     驾驶员姓名长度
*1:     驾驶员姓名  n
*1+n:   驾驶员身份证编码 20
*21+n:  从业资格证编码 40
*61+n:  发证机构名称长度1
*62+n:  发证机构名称
***************************************************/
/**************************************************/

#define STDRIVERINFO_SIZE sizeof(STDRIVERINFO)
////////////////////////////
////////////////////////////////////////////////////////////////////
#define PHONEBOOK_START_ADDR            (unsigned long)0
#define PHONEBOOK_DATALEN               (unsigned long)FLASH_ONE_SECTOR_BYTES
/////////////////////////////////////////
#define EVENTREPORT_START_ADDR          (unsigned long)(PHONEBOOK_START_ADDR+PHONEBOOK_DATALEN)
#define EVENTREPORT_DATALEN             (unsigned long)FLASH_ONE_SECTOR_BYTES
/////////////////////////////
#define INFODEMAND_START_ADDR           (unsigned long)(EVENTREPORT_START_ADDR+EVENTREPORT_DATALEN)
#define INFODEMAND_DATALEN              (unsigned long)FLASH_ONE_SECTOR_BYTES
//////////////////////////////////////////////////
#define DRIVER_INFO_START_ADDR           (unsigned long)(INFODEMAND_START_ADDR+INFODEMAND_DATALEN)
#define DRIVER_INFO_DATALEN              (unsigned long)FLASH_ONE_SECTOR_BYTES
//////////////////////////////////////////////////
#define QUESTION_RESPONSE_START_ADDR        (unsigned long)(DRIVER_INFO_START_ADDR+DRIVER_INFO_DATALEN)//
#define QUESTION_RESPONSE_DATALEN           (unsigned long)FLASH_ONE_SECTOR_BYTES
//////////////////////////////////////////////////
#define INFO_SERVICE_START_ADDR             (unsigned long)(QUESTION_RESPONSE_START_ADDR+QUESTION_RESPONSE_DATALEN)//
#define INFO_SERVICE_DATALEN                (unsigned long)FLASH_ONE_SECTOR_BYTES
//////////////////////////////////


////////////空间分配////////////////
#define MESSAGE_START_ADDR          (unsigned long)(FLASH_MESSAGE_START_SECTOR*FLASH_ONE_SECTOR_BYTES)
//////////////////////////////////////////
unsigned char s_ucMsgFlashBuffer[FLASH_ONE_SECTOR_BYTES];
//////////////////////////////////////////
//////////////////////////////////
#define ONE_QUESTION_RESPONSE_DATA_LEN          510
#define QUESTION_RESPONSE_MAX_COUNT   (unsigned char)((QUESTION_RESPONSE_DATALEN-STMESSAGEHEAD_SIZE)/ONE_QUESTION_RESPONSE_DATA_LEN)

#define ONE_INFO_SERVICE_DATA_LEN          510
#define INFO_SERVICE_MAX_COUNT   (unsigned char)((INFO_SERVICE_DATALEN-STMESSAGEHEAD_SIZE)/ONE_INFO_SERVICE_DATA_LEN)

enum 
{
    SETTYPE_DELETE,						//
    SETTYPE_UPDATE,						//
    SETTYPE_ADD,
    SETTYPE_MODIFY,
    SETTYPE_DEL_EX,
    SETTYPE_MAX,
}MESSAGESETTYPE;

enum 
{
    MSG_SET_DRIVER_INFO_EMPTY ,
    MSG_SET_DRIVER_INFO_UPDATE,						//      
    MSG_SET_DRIVER_INFO_ADD,
    MSG_SET_DRIVER_INFO_MODIFY,
    MSG_SET_DRIVER_INFO_DELETE,						//
    MSG_SET_DRIVER_INFO_QUERY,
    MSG_SET_DRIVER_INFO_MAX,
}E_MSGSET_DRIVER_INFO_TYPE;
////////////////////////////
/****************************************************/
/*                                                  */
/* Local Definitions and macros                     */
/*                                                  */
/****************************************************/ 
void EventReportDefaultValue(void);
unsigned char Messege_GetOneDriverInfoDatalen(unsigned char *pBuffer,unsigned short BufferLen);
/*************************************************************
** 函数名称: MessageFlashFormat()
** 功能描述: Flash格式化
** 入口参数: startAddr:开始地址(相对地址);lenght:长度
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void MessageFlashFormat(unsigned short startAddr) 
{
    //注意startAddr为相对地址
    unsigned long addr;
    addr = startAddr;
    addr += MESSAGE_START_ADDR;
    ////////////////
      sFLASH_EraseSector(addr);
}

/*************************************************************
** 函数名称: MessageSaveDataToFlash()
** 功能描述: Flash写数据
** 入口参数: s:所有保存的源数据指针,startAddr:开始地址(相对地址);lenght:长度
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void MessageSaveDataToFlash(unsigned char *s,unsigned short startAddr,unsigned short lenght) 
{
    //注意startAddr为相对地址
    unsigned long addr;
    addr = startAddr;
    addr += MESSAGE_START_ADDR;
    /////////////////////    
    sFLASH_WriteBuffer(s,addr,lenght);
}

/*************************************************************
** 函数名称: MessageReadDataFromFlash()
** 功能描述: Flash读数据
** 入口参数: s:数据指针，startAddr:开始地址(相对地址);lenght:长度
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void MessageReadDataFromFlash(unsigned char *s,unsigned short startAddr,unsigned short lenght) 
{
    //注意startAddr为相对地址
    unsigned long addr;
    addr = startAddr;
    addr += MESSAGE_START_ADDR;    
    ///////////////
    sFLASH_ReadBuffer(s,addr,lenght);
}

/*************************************************************
** 函数名称: PhoneBookReadDataFromFlash
** 功能描述: Flash读数据
** 入口参数: s:数据指针，startAddr:开始地址(相对地址);lenght:长度
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void PhoneBookReadDataFromFlash(void) 
{
    MessageReadDataFromFlash(s_ucMsgFlashBuffer,PHONEBOOK_START_ADDR,PHONEBOOK_DATALEN);
}

/*************************************************************
** 函数名称: PhoneBookSaveDataToFlash
** 功能描述: Flash读数据
** 入口参数: s:数据指针，startAddr:开始地址(相对地址);lenght:长度
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void PhoneBookSaveDataToFlash(void) 
{
    ///////////////在写入之前先格式化//////////////////////
    MessageFlashFormat(PHONEBOOK_START_ADDR);///
    ////////////////////////
    MessageSaveDataToFlash(s_ucMsgFlashBuffer,PHONEBOOK_START_ADDR,PHONEBOOK_DATALEN);
}

/*************************************************************
** 函数名称: EventReportReadDataFromFlash
** 功能描述: Flash读数据
** 入口参数: s:数据指针，startAddr:开始地址(相对地址);lenght:长度
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void EventReportReadDataFromFlash(void) 
{
    MessageReadDataFromFlash(s_ucMsgFlashBuffer,EVENTREPORT_START_ADDR,EVENTREPORT_DATALEN);
}

/*************************************************************
** 函数名称: EventReportSaveDataToFlash
** 功能描述: Flash读数据
** 入口参数: s:数据指针，startAddr:开始地址(相对地址);lenght:长度
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void EventReportSaveDataToFlash(void) 
{
    MessageFlashFormat(EVENTREPORT_START_ADDR);
    ////////////////////////////
    MessageSaveDataToFlash(s_ucMsgFlashBuffer,EVENTREPORT_START_ADDR,EVENTREPORT_DATALEN);
}

/*************************************************************
** 函数名称: InfoDemandReadDataFromFlash
** 功能描述: Flash读数据
** 入口参数: s:数据指针，startAddr:开始地址(相对地址);lenght:长度
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void InfoDemandReadDataFromFlash(void) 
{
    MessageReadDataFromFlash(s_ucMsgFlashBuffer,INFODEMAND_START_ADDR,INFODEMAND_DATALEN);
}

/*************************************************************
** 函数名称: InfoDemandSaveDataToFlash
** 功能描述: Flash读数据
** 入口参数: s:数据指针，startAddr:开始地址(相对地址);lenght:长度
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void InfoDemandSaveDataToFlash(void) 
{
    MessageFlashFormat(INFODEMAND_START_ADDR);
    ////////////////////////////
    MessageSaveDataToFlash(s_ucMsgFlashBuffer,INFODEMAND_START_ADDR,INFODEMAND_DATALEN);
}

/*************************************************************
** 函数名称: DriverInfoReadDataFromFlash
** 功能描述: 
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void DriverInfoReadDataFromFlash(void) 
{
    MessageReadDataFromFlash(s_ucMsgFlashBuffer,DRIVER_INFO_START_ADDR,DRIVER_INFO_DATALEN);
}
/*************************************************************
** 函数名称: DriverInfoSaveDataToFlash
** 功能描述: Flash读数据
** 入口参数: s:数据指针，startAddr:开始地址(相对地址);lenght:长度
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void DriverInfoSaveDataToFlash(void) 
{
    MessageFlashFormat(DRIVER_INFO_START_ADDR);
    ////////////////////////////
    MessageSaveDataToFlash(s_ucMsgFlashBuffer,DRIVER_INFO_START_ADDR,DRIVER_INFO_DATALEN);
}

/*************************************************************
** 函数名称: InfoServiceSaveDataToFlash
** 功能描述: 
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void InfoServiceReadDataFromFlash(void) 
{
    MessageReadDataFromFlash(s_ucMsgFlashBuffer,INFO_SERVICE_START_ADDR,INFO_SERVICE_DATALEN);
}
/*************************************************************
** 函数名称: InfoServiceSaveDataToFlash
** 功能描述: 
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void InfoServiceSaveDataToFlash(void) 
{
    MessageFlashFormat(INFO_SERVICE_START_ADDR);
    ////////////////////////////
    MessageSaveDataToFlash(s_ucMsgFlashBuffer,INFO_SERVICE_START_ADDR,INFO_SERVICE_DATALEN);
}
/*************************************************************
** 函数名称: InfoServiceSaveDataToFlash
** 功能描述: 
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void QuestionResponseReadDataFromFlash(void) 
{
    MessageReadDataFromFlash(s_ucMsgFlashBuffer,QUESTION_RESPONSE_START_ADDR,QUESTION_RESPONSE_DATALEN);
}
/*************************************************************
** 函数名称: InfoServiceSaveDataToFlash
** 功能描述: 
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void QuestionResponseSaveDataToFlash(void) 
{
    MessageFlashFormat(QUESTION_RESPONSE_START_ADDR);
    ////////////////////////////
    MessageSaveDataToFlash(s_ucMsgFlashBuffer,QUESTION_RESPONSE_START_ADDR,QUESTION_RESPONSE_DATALEN);
}
/*************************************************************
** 
**     Private  code 
** 
**************************************************************/
/*************************************************************
** 函数名称: PhoneBookDeleteAll
** 功能描述: 删除所有电话簿
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Message_stMsgHeadInit(STMESSAGEHEAD *stMsgHead,unsigned short HeadData)
{
    stMsgHead->currentAddr  = STMESSAGEHEAD_SIZE;
    stMsgHead->HeadData     = HeadData;
    stMsgHead->saveFlag     = MSG_SAVE;
    stMsgHead->recordCnt    = 0;
    stMsgHead->oldestindex  = 0;
    stMsgHead->curindex     = 0;
}
/*************************************************************
** 函数名称: PhoneBookDeleteAll
** 功能描述: 删除所有电话簿
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void PhoneBookDeleteAll(void)
{
    STMESSAGEHEAD stMsgHead;
    MessageFlashFormat(PHONEBOOK_START_ADDR);
    ////////////////////////////////////
    Message_stMsgHeadInit(&stMsgHead,PHONE_HEAD);
    /////////////////////////////////////
    MessageSaveDataToFlash((unsigned char *)&stMsgHead,PHONEBOOK_START_ADDR,STMESSAGEHEAD_SIZE);
}
/*************************************************************
** 函数名称: GetPhoneBookAddress
** 功能描述: 查找指定联系人的首地址
** 入口参数: name:联系人名字,num记录总数
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned short GetLinkmanAddressFromPhoneBook(unsigned char *name,unsigned char num)
{
    unsigned char i;
    unsigned short addr;
    STMSG_PHONEBOOK stPhone;
    addr = STMESSAGEHEAD_SIZE;
    for(i = 0; i < num; i++)
    {
        memcpy(&stPhone,&s_ucMsgFlashBuffer[addr],STPHONEBOOK_SIZE);
        if(!strcmp((char *)stPhone.name,(char *)name))
        {
            return addr;
        }
        addr +=STPHONEBOOK_SIZE;
    }
    return 0;
}
/*************************************************************
** 函数名称: PhoneBookUpdate
** 功能描述: 更新所有电话簿
** 入口参数: 更新电话簿资料
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void PhoneBookUpdate(unsigned char *data,unsigned short datalen)
{
    unsigned char i,length,num;
    unsigned short index,addr;
    STMSG_PHONEBOOK stPhone;
    STMESSAGEHEAD  stMsgHead; 
    Message_stMsgHeadInit(&stMsgHead,PHONE_HEAD);
    memset(s_ucMsgFlashBuffer,0,sizeof(s_ucMsgFlashBuffer));
    index = 0;
    num = data[index++];
    for(i = 0; i < num; i++)
    {
        stPhone.callflag = data[index++];
        length = data[index++];
        ////////////////////////
        if(length==0)return;
        memcpy(stPhone.number,&data[index],length);
        stPhone.number[length] = '\0';
        index += length;
        ///////////////////////
        length = data[index++];
        memcpy(stPhone.name,&data[index],length);
        stPhone.name[length] = '\0';
        index+=length;
        addr=GetLinkmanAddressFromPhoneBook(stPhone.name,stMsgHead.recordCnt);
        if(addr)//找到相应联系人
        {
            memcpy(&s_ucMsgFlashBuffer[addr],&stPhone,STPHONEBOOK_SIZE);
        }
        else
        {
            ////////////////////
            if(stMsgHead.currentAddr+STPHONEBOOK_SIZE > PHONEBOOK_DATALEN)break;
            ///////////////////////////////////////////
            memcpy(&s_ucMsgFlashBuffer[stMsgHead.currentAddr],&stPhone,STPHONEBOOK_SIZE);
            ////////////////////////
            stMsgHead.currentAddr += STPHONEBOOK_SIZE;
            stMsgHead.recordCnt++;
        }
        //////////////
        if(index>=datalen)break;
    }
    ///////////////////////////////
    memcpy(&s_ucMsgFlashBuffer[0],&stMsgHead,STMESSAGEHEAD_SIZE);
    /////////////////////////////////
    PhoneBookSaveDataToFlash();
}
/*************************************************************
** 函数名称: PhoneBookAdd
** 功能描述: 追加电话簿
** 入口参数: 追加电话簿资料
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void PhoneBookAdd(unsigned char *data,unsigned short datalen)
{
    unsigned char i,length,num;
    unsigned short index,addr;
    STMSG_PHONEBOOK stPhone;
    STMESSAGEHEAD stMsgHead;
    PhoneBookReadDataFromFlash();
    memcpy(&stMsgHead,s_ucMsgFlashBuffer,STMESSAGEHEAD_SIZE);
    index = 0;    
    num = data[index++];
    for(i = 0; i < num; i++)
    {
        stPhone.callflag = data[index++];
        length = data[index++];
        ////////////////////////
        if(length==0)return;
        memcpy(stPhone.number,&data[index],length);
        stPhone.number[length] = '\0';
        index += length;
        ///////////////////////
        length = data[index++];
        memcpy(stPhone.name,&data[index],length);
        stPhone.name[length] = '\0';
        index+=length;
        addr=GetLinkmanAddressFromPhoneBook(stPhone.name,stMsgHead.recordCnt);
        if(addr)//找到相应联系人
        {
            memcpy(&s_ucMsgFlashBuffer[addr],&stPhone,STPHONEBOOK_SIZE);
        }
        else
        {
            ////////////////////
            if(stMsgHead.currentAddr+STPHONEBOOK_SIZE > PHONEBOOK_DATALEN)break;
            /////////////////////////
            memcpy(&s_ucMsgFlashBuffer[stMsgHead.currentAddr],&stPhone,STPHONEBOOK_SIZE);
            ////////////////////////
            stMsgHead.currentAddr+=STPHONEBOOK_SIZE;
            stMsgHead.recordCnt++;
        }
        //////////////
        if(index>=datalen)break;
    }
    ///////////////////////////////
    memcpy(&s_ucMsgFlashBuffer[0],&stMsgHead,STMESSAGEHEAD_SIZE);
    /////////////////////////////////
    ////////////////////
    PhoneBookSaveDataToFlash();
}

/*************************************************************
** 函数名称: PhoneBookModify
** 功能描述: 修改电话簿
** 入口参数: 修改电话簿资料
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void PhoneBookModify(unsigned char *data,unsigned short datalen)
{
    unsigned char i,length,num;
    unsigned short index,addr;
    STMSG_PHONEBOOK stPhone;
    STMESSAGEHEAD  stMsgHead;
    PhoneBookReadDataFromFlash();
    memcpy(&stMsgHead,s_ucMsgFlashBuffer,STMESSAGEHEAD_SIZE);
    index = 0;    
    num = data[index++];
    for(i = 0; i < num; i++)
    {
        stPhone.callflag = data[index++];
        length = data[index++];
        if(length==0)return;
        ////////////////////////
        memcpy(stPhone.number,&data[index],length);
        stPhone.number[length] = '\0';
        index += length;
        ///////////////////////
        length = data[index++];
        memcpy(stPhone.name,&data[index],length);
        stPhone.name[length] = '\0';
        index+=length;
        ////////////////////
        addr=GetLinkmanAddressFromPhoneBook(stPhone.name,stMsgHead.recordCnt);
        if(addr)//找到相应联系人
        {
            memcpy(&s_ucMsgFlashBuffer[addr],&stPhone,STPHONEBOOK_SIZE);
        }
        //////////////
        if(index>=datalen)break;
    }    
    /////////////////////////////////
    ////////////////////
    PhoneBookSaveDataToFlash();
}

/*************************************************************
** 函数名称: EventReportDeleteAll
** 功能描述: 删除所有事件报告
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void EventReportDeleteAll(void)
{
    STMESSAGEHEAD stMsgHead;
    MessageFlashFormat(EVENTREPORT_START_ADDR);
    ////////////////////////////////////
    Message_stMsgHeadInit(&stMsgHead,EVENT_HEAD);
    /////////////////////////////////////
    MessageSaveDataToFlash((unsigned char *)&stMsgHead,EVENTREPORT_START_ADDR,STMESSAGEHEAD_SIZE);
    
}
/*************************************************************
** 函数名称: GetEventReportAddress
** 功能描述: 查找指定事件报告ID的首地址
** 入口参数: ID:事件报告ID,num记录总数
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned short GetAddressFromEventReport(unsigned char ID,unsigned char num)
{
    unsigned char i;
    unsigned short addr;
    STMESSAGE stEvent;
    addr = STMESSAGEHEAD_SIZE;
    for(i = 0; i < num; i++)
    {
        memcpy(&stEvent,&s_ucMsgFlashBuffer[addr],STMESSAGE_SIZE);
        if(stEvent.ID==ID)
        {
            return addr;
        }
        addr +=STMESSAGE_SIZE;
    }
    return 0;
}
/*************************************************************
** 函数名称: EventReportUpdate
** 功能描述: 更新所有事件报告
** 入口参数: 更新事件报告资料
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void EventReportUpdate(unsigned char *data,unsigned short datalen)
{
    unsigned char i,length,num;
    unsigned short index,addr;
    STMESSAGE stEvent;
    STMESSAGEHEAD  stMsgHead; 
    Message_stMsgHeadInit(&stMsgHead,EVENT_HEAD);
    memset(s_ucMsgFlashBuffer,0,sizeof(s_ucMsgFlashBuffer));
    index = 0;
    num = data[index++];
    for(i = 0; i < num; i++)
    {
        stEvent.ID = data[index++];
        length = data[index++];
        ////////////////////////
        if(length==0)return;
        memcpy(stEvent.content,&data[index],length);
        stEvent.content[length] = '\0';
        index += length;
        addr=GetAddressFromEventReport(stEvent.ID,stMsgHead.recordCnt);
        if(addr)//找到事件报告
        {
            memcpy(&s_ucMsgFlashBuffer[addr],&stEvent,STMESSAGE_SIZE);
        }
        else
        {
            ///////////////////////
            if(stMsgHead.currentAddr+STMESSAGE_SIZE > EVENTREPORT_DATALEN)break;
            ////////////////////
            memcpy(&s_ucMsgFlashBuffer[stMsgHead.currentAddr],&stEvent,STMESSAGE_SIZE);
            ////////////////////////
            stMsgHead.currentAddr += STMESSAGE_SIZE;
            stMsgHead.recordCnt++;
        }        
        //////////////
        if(index>=datalen)break;
    }
    ///////////////////////////////
    memcpy(&s_ucMsgFlashBuffer[0],&stMsgHead,STMESSAGEHEAD_SIZE);
    /////////////////////////////////
    EventReportSaveDataToFlash();
}
/*************************************************************
** 函数名称: EventReportAdd
** 功能描述: 追加事件报告
** 入口参数: 追加事件报告资料
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void EventReportAdd(unsigned char *data,unsigned short datalen)
{
    unsigned char i,length,num;
    unsigned short index,addr;
    STMESSAGE stEvent;
    STMESSAGEHEAD stMsgHead;
    EventReportReadDataFromFlash();
    memcpy(&stMsgHead,s_ucMsgFlashBuffer,STMESSAGEHEAD_SIZE);
    index = 0;    
    num = data[index++];
    for(i = 0; i < num; i++)
    {
        stEvent.ID = data[index++];
        length = data[index++];        
        ////////////////////////
        if(length==0)return;
        memcpy(stEvent.content,&data[index],length);
        stEvent.content[length] = '\0';
        index += length;
        ///////////////////////
        addr=GetAddressFromEventReport(stEvent.ID,stMsgHead.recordCnt);
        if(addr)//找到事件报告
        {
            memcpy(&s_ucMsgFlashBuffer[addr],&stEvent,STMESSAGE_SIZE);
        }
        else
        {
            if(stMsgHead.currentAddr+STMESSAGE_SIZE > EVENTREPORT_DATALEN)break;
            ////////////////////
            memcpy(&s_ucMsgFlashBuffer[stMsgHead.currentAddr],&stEvent,STMESSAGE_SIZE);
            ////////////////////////
            stMsgHead.currentAddr+=STMESSAGE_SIZE;
            stMsgHead.recordCnt++;
            //////////////
        }
        if(index>=datalen)break;
    }
    ///////////////////////////////
    memcpy(&s_ucMsgFlashBuffer[0],&stMsgHead,STMESSAGEHEAD_SIZE);
    /////////////////////////////////
    ////////////////////
    EventReportSaveDataToFlash();
}
/*************************************************************
** 函数名称: EventReportModify
** 功能描述: 修改事件报告
** 入口参数: 修改事件报告资料
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void EventReportModify(unsigned char *data,unsigned short datalen)
{
    unsigned char i,length,num;
    unsigned short index,addr;
    STMESSAGE stEvent;
    STMESSAGEHEAD  stMsgHead;
    EventReportReadDataFromFlash();
    memcpy(&stMsgHead,s_ucMsgFlashBuffer,STMESSAGEHEAD_SIZE);
    index = 0;    
    num = data[index++];
    for(i = 0; i < num; i++)
    {
        stEvent.ID = data[index++];
        length = data[index++];
        ////////////////////////
        memcpy(stEvent.content,&data[index],length);
        stEvent.content[length] = '\0';
        index += length;
        ////////////////////
        addr=GetAddressFromEventReport(stEvent.ID,stMsgHead.recordCnt);
        if(addr)//找到事件报告
        {
            memcpy(&s_ucMsgFlashBuffer[addr],&stEvent,STMESSAGE_SIZE);
        }
        //////////////
        if(index>=datalen)break;
    }    
    /////////////////////////////////
    ////////////////////
    EventReportSaveDataToFlash();
}
/*************************************************************
** 函数名称: EventReportDelEx
** 功能描述: 删除特定几项事件
** 入口参数: 事件报告资料
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char EventReportDelEx(unsigned char *data,unsigned short datalen)
{
    unsigned char i,num;
    unsigned short index,addr,tmpAddr,length;
    STMESSAGE stEvent;
    STMESSAGEHEAD  stMsgHead;
    EventReportReadDataFromFlash();
    memcpy(&stMsgHead,s_ucMsgFlashBuffer,STMESSAGEHEAD_SIZE);
    index = 0;    
    num = data[index++];
    if(num>stMsgHead.recordCnt)return 1;//超过事件总数
    for(i = 0; i < num; i++)
    {
        stEvent.ID = data[index++];        
        ////////////////////
        if(0==GetAddressFromEventReport(stEvent.ID,stMsgHead.recordCnt))//未找到事件报告
        {
            return 2;
        }
    }
    ///////////////
    index = 0;    
    num = data[index++];
    for(i = 0; i < num; i++)
    {
        stEvent.ID = data[index++];        
        ////////////////////
        addr=GetAddressFromEventReport(stEvent.ID,stMsgHead.recordCnt);
        length=stMsgHead.recordCnt*STMESSAGE_SIZE+STMESSAGEHEAD_SIZE;
        tmpAddr=addr+STMESSAGE_SIZE;
        if(length>tmpAddr)
        {
            memmove(&s_ucMsgFlashBuffer[addr],&s_ucMsgFlashBuffer[tmpAddr],length-tmpAddr);
        }
        if(stMsgHead.recordCnt)
        {
            stMsgHead.recordCnt--;
            stMsgHead.currentAddr -= STMESSAGE_SIZE;
        }
        //////////////
        if(index>=datalen)break;
    }
    ///////////////////////
    memcpy(&s_ucMsgFlashBuffer[0],&stMsgHead,STMESSAGEHEAD_SIZE);
    /////////////////////////////////
    EventReportSaveDataToFlash();
    return 0;
}
/*************************************************************
** 函数名称: EventReportDefaultValue
** 功能描述: 把预置短语放到事件报告中
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void EventReportDefaultValue(void)
{
    STMESSAGEHEAD stMsgHead;
    STMESSAGE stEvent;
    unsigned char i;
    i = 0;
    for(;;)
    {
        if(strlen(s_apcMenuShortMsg[i]) == 0)break;
        i ++;
    }
    //////////////////////////////////
    Message_stMsgHeadInit(&stMsgHead,EVENT_HEAD);
    memset(s_ucMsgFlashBuffer,0,sizeof(s_ucMsgFlashBuffer));
    stMsgHead.recordCnt = i;    
    for(i=0; i<stMsgHead.recordCnt;i++)
    {
        stEvent.ID=i+1;
        strcpy((char *)stEvent.content,s_apcMenuShortMsg[i]);  
        memcpy(&s_ucMsgFlashBuffer[stMsgHead.currentAddr],&stEvent,STMESSAGE_SIZE);
        stMsgHead.currentAddr +=STMESSAGE_SIZE;
    }
    memcpy(&s_ucMsgFlashBuffer[0],&stMsgHead,STMESSAGEHEAD_SIZE);
    ///////////////////////////
    EventReportSaveDataToFlash();
}
/*************************************************************
** 函数名称: GetEventReportTotalNum
** 功能描述: 取得事件报告总条数
** 入口参数: 
** 出口参数: 无
** 返回参数: 总条数
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char GetEventReportTotalNum(void)
{
    unsigned char buffer[STMESSAGEHEAD_SIZE];
    STMESSAGEHEAD  stMsgHead;
    MessageReadDataFromFlash(buffer,EVENTREPORT_START_ADDR,STMESSAGEHEAD_SIZE);
    memcpy(&stMsgHead,buffer,STMESSAGEHEAD_SIZE);
    if(EVENT_HEAD==stMsgHead.HeadData&&MSG_SAVE==stMsgHead.saveFlag)
    {
        return stMsgHead.recordCnt;
    }
    ///////////////////////
    return 0;    
}
/*************************************************************
** 函数名称: GetOneEventReportDetails
** 功能描述: 取得相应编号的事件报告详情
** 入口参数: index编号(从0开始),
** 出口参数: stEvent事件报告结构体
** 返回参数: 1:成功,0:失败
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char GetOneEventReportDetails(unsigned char index,STMESSAGE *stEvent)
{
    unsigned char buffer[STMESSAGEHEAD_SIZE];
    unsigned short addr;
    STMESSAGEHEAD  stMsgHead;
    MessageReadDataFromFlash(buffer,EVENTREPORT_START_ADDR,STMESSAGEHEAD_SIZE);
    memcpy(&stMsgHead,buffer,STMESSAGEHEAD_SIZE);
    if(EVENT_HEAD==stMsgHead.HeadData&&MSG_SAVE==stMsgHead.saveFlag)
    {
        if(index<stMsgHead.recordCnt)
        {
            addr = index*STMESSAGE_SIZE;
            addr += STMESSAGEHEAD_SIZE;            
            MessageReadDataFromFlash((unsigned char *)stEvent,EVENTREPORT_START_ADDR+addr,STMESSAGE_SIZE);
            return 1;
        }
    }
    return 0;
    /////////////////////// 
}
/*************************************************************
** 函数名称: InfoDemandDeleteAll
** 功能描述: 删除所有信息点播
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void InfoDemandDeleteAll(void)
{
    STMESSAGEHEAD stMsgHead;
    MessageFlashFormat(INFODEMAND_START_ADDR);
    ////////////////////////////////////
    Message_stMsgHeadInit(&stMsgHead,INFO_HEAD);
    /////////////////////////////////////
    MessageSaveDataToFlash((unsigned char *)&stMsgHead,INFODEMAND_START_ADDR,STMESSAGEHEAD_SIZE);
}
/*************************************************************
** 函数名称: GetInfoDemandAddress
** 功能描述: 查找指定信息点播ID的首地址
** 入口参数: ID:信息点播ID,num记录总数
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned short GetAddressFromInfoDemand(unsigned char ID,unsigned char num)
{
    unsigned char i;
    unsigned short addr;
    STMESSAGE stInfo;
    addr = STMESSAGEHEAD_SIZE;
    for(i = 0; i < num; i++)
    {
        memcpy(&stInfo,&s_ucMsgFlashBuffer[addr],STMESSAGE_SIZE);
        if(stInfo.ID==ID)
        {
            return addr;
        }
        addr +=STMESSAGE_SIZE;
    }
    return 0;
}
/*************************************************************
** 函数名称: InfoDemandUpdate
** 功能描述: 更新所有信息点播
** 入口参数: 更新信息点播资料
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void InfoDemandUpdate(unsigned char *data,unsigned short datalen)
{
    unsigned char i,num;
    unsigned short index,length,addr;
    STMESSAGE stInfo;
    STMESSAGEHEAD  stMsgHead;
    Message_stMsgHeadInit(&stMsgHead,INFO_HEAD);
    memset(s_ucMsgFlashBuffer,0,sizeof(s_ucMsgFlashBuffer));
    index = 0;    
    num = data[index++];
    for(i = 0; i < num; i++)
    {
        stInfo.ID = data[index++];
        length = data[index++];
        length<<=8;
        length |= data[index++];
        if(length==0)return;
        ////////////////////////
        memcpy(stInfo.content,&data[index],length);
        stInfo.content[length] = '\0';
        index += length;
        stInfo.status = 0;
        addr=GetAddressFromInfoDemand(stInfo.ID,stMsgHead.recordCnt);
        if(addr)//找到信息点播
        {
            memcpy(&s_ucMsgFlashBuffer[addr],&stInfo,STMESSAGE_SIZE);
        }
        else
        {
            ///////////////////////
            if(stMsgHead.currentAddr+STMESSAGE_SIZE > INFODEMAND_DATALEN)break;
            ////////////////////
            memcpy(&s_ucMsgFlashBuffer[stMsgHead.currentAddr],&stInfo,STMESSAGE_SIZE);
            ////////////////////////
            stMsgHead.currentAddr += STMESSAGE_SIZE;
            stMsgHead.recordCnt++;
        }
        //////////////
        if(index>=datalen)break;
    }
    ///////////////////////////////
    memcpy(&s_ucMsgFlashBuffer[0],&stMsgHead,STMESSAGEHEAD_SIZE);
    /////////////////////////////////
    InfoDemandSaveDataToFlash();
}
/*************************************************************
** 函数名称: InfoDemandAdd
** 功能描述: 追加信息点播
** 入口参数: 追加信息点播资料
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void InfoDemandAdd(unsigned char *data,unsigned short datalen)
{
    unsigned char i,num;
    unsigned short index,length,addr;
    STMESSAGE stInfo;
    STMESSAGEHEAD stMsgHead;
    InfoDemandReadDataFromFlash();
    memcpy(&stMsgHead,s_ucMsgFlashBuffer,STMESSAGEHEAD_SIZE);
    index = 0;    
    num = data[index++];
    for(i = 0; i < num; i++)
    {
        stInfo.ID = data[index++];
        length = data[index++];
        length<<=8;
        length |= data[index++];
        if(length==0)return;
        ////////////////////////
        memcpy(stInfo.content,&data[index],length);
        stInfo.content[length] = '\0';
        index += length;
        ///////////////////////
        stInfo.status = 0;
        addr=GetAddressFromInfoDemand(stInfo.ID,stMsgHead.recordCnt);
        if(addr)//找到信息点播
        {
            memcpy(&s_ucMsgFlashBuffer[addr],&stInfo,STMESSAGE_SIZE);
        }
        else
        {
            ////////////////////////
            if(stMsgHead.currentAddr+STMESSAGE_SIZE > INFODEMAND_DATALEN)break;
            ////////////////////
            memcpy(&s_ucMsgFlashBuffer[stMsgHead.currentAddr],&stInfo,STMESSAGE_SIZE);
            ////////////////////////
            stMsgHead.currentAddr+=STMESSAGE_SIZE;
            stMsgHead.recordCnt ++;
        }
        //////////////
        if(index>=datalen)break;
    }
    ///////////////////////////////
    memcpy(&s_ucMsgFlashBuffer[0],&stMsgHead,STMESSAGEHEAD_SIZE);
    /////////////////////////////////
    ////////////////////
    InfoDemandSaveDataToFlash();
}

/*************************************************************
** 函数名称: InfoDemandModify
** 功能描述: 修改信息点播
** 入口参数: 修改信息点播资料
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void InfoDemandModify(unsigned char *data,unsigned short datalen)
{
    unsigned char i,num;
    unsigned short index,addr,length;
    STMESSAGE stInfo;
    STMESSAGEHEAD  stMsgHead;
    InfoDemandReadDataFromFlash();
    memcpy(&stMsgHead,s_ucMsgFlashBuffer,STMESSAGEHEAD_SIZE);
    index = 0;    
    num = data[index++];
    for(i = 0; i < num; i++)
    {
        stInfo.ID = data[index++];
        length = data[index++];
        length<<=8;
        length |= data[index++];
        if(length==0)return;
        ////////////////////////
        memcpy(stInfo.content,&data[index],length);
        stInfo.content[length] = '\0';
        index += length;
        ////////////////
        stInfo.status = 0;
        ////////////////////
        addr=GetAddressFromInfoDemand(stInfo.ID,stMsgHead.recordCnt);
        if(addr)//找到信息点播
        {
            memcpy(&s_ucMsgFlashBuffer[addr],&stInfo,STMESSAGE_SIZE);
        }
        //////////////
        if(index>=datalen)break;
    }    
    /////////////////////////////////
    ////////////////////
    InfoDemandSaveDataToFlash();
}

/*************************************************************
** 函数名称: GetInfoDemandTotalNum
** 功能描述: 取得信息点播总条数
** 入口参数: 
** 出口参数: 无
** 返回参数: 总条数
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char GetInfoDemandTotalNum(void)
{
    unsigned char buffer[STMESSAGEHEAD_SIZE];
    STMESSAGEHEAD  stMsgHead;
    MessageReadDataFromFlash(buffer,INFODEMAND_START_ADDR,STMESSAGEHEAD_SIZE);
    memcpy(&stMsgHead,buffer,STMESSAGEHEAD_SIZE);
    if(INFO_HEAD==stMsgHead.HeadData&&MSG_SAVE==stMsgHead.saveFlag)
    {
        return stMsgHead.recordCnt;
    }
    ///////////////////////
    return 0;    
}
/*************************************************************
** 函数名称: GetOneInfoDemandDetails
** 功能描述: 取得相应编号的信息点播详情
** 入口参数: index编号(从0开始),
** 出口参数: stInfo信息点播结构体
** 返回参数: 1:成功,0:失败
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char GetOneInfoDemandDetails(unsigned char index,STMESSAGE *stInfo)
{
    unsigned char buffer[STMESSAGEHEAD_SIZE];
    unsigned short addr;
    STMESSAGEHEAD  stMsgHead;
    MessageReadDataFromFlash(buffer,INFODEMAND_START_ADDR,STMESSAGEHEAD_SIZE);
    memcpy(&stMsgHead,buffer,STMESSAGEHEAD_SIZE);
    if(INFO_HEAD==stMsgHead.HeadData&&MSG_SAVE==stMsgHead.saveFlag)
    {
        if(index<stMsgHead.recordCnt)
        {
            addr = index*STMESSAGE_SIZE;
            addr += STMESSAGEHEAD_SIZE;            
            MessageReadDataFromFlash((unsigned char *)stInfo,INFODEMAND_START_ADDR+addr,STMESSAGE_SIZE);
            return 1;
        }
    }
    return 0;
    /////////////////////// 
}
/*************************************************************
** 函数名称: DriverInfoDeleteAll
** 功能描述: 删除所有驾驶员信息
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void DriverInfoDeleteAll(void)
{
    STMESSAGEHEAD stMsgHead;
    MessageFlashFormat(DRIVER_INFO_START_ADDR);
    ////////////////////////////////////
    Message_stMsgHeadInit(&stMsgHead,DRIVER_HEAD);
    /////////////////////////////////////
    MessageSaveDataToFlash((unsigned char *)&stMsgHead,DRIVER_INFO_START_ADDR,STMESSAGEHEAD_SIZE);
    ///////////////////////////////
    #ifdef USE_NAVILCD
    NaviLcd_DriverLogout();
    #endif
}
/*************************************************************
** 函数名称: GetDriverInfoRecordNumber
** 功能描述: 取得相应工号的记录号(从1开始编号)
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char GetDriverInfoRecordNumber(unsigned char *drivercode)
{
    unsigned char i;
    unsigned short addr;
    STDRIVERINFO stDriver;
    STMESSAGEHEAD  stMsgHead;
    DriverInfoReadDataFromFlash();
    memcpy(&stMsgHead,s_ucMsgFlashBuffer,STMESSAGEHEAD_SIZE);
    addr = STMESSAGEHEAD_SIZE;
    for(i = 0; i < stMsgHead.recordCnt; i++)
    {
        memcpy(&stDriver,&s_ucMsgFlashBuffer[addr],STDRIVERINFO_SIZE);
        if(strncmp((char*)drivercode,(char*)stDriver.drivercode,6)==0)
        {
            return i+1;
        }
        addr +=STDRIVERINFO_SIZE;
    }
    /////////////////////
    return 0;
}
/*************************************************************
** 函数名称: GetDriverInfoRecordNumber
** 功能描述: 取得相应工号的记录号(从1开始编号)
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char GetDriverInfoRecordNumberEx(unsigned char *drivercode)
{
    unsigned char i;
    unsigned short addr;
    STDRIVERINFO stDriver;
    STMESSAGEHEAD  stMsgHead;
    memcpy(&stMsgHead,s_ucMsgFlashBuffer,STMESSAGEHEAD_SIZE);
    addr = STMESSAGEHEAD_SIZE;
    for(i = 0; i < stMsgHead.recordCnt; i++)
    {
        memcpy(&stDriver,&s_ucMsgFlashBuffer[addr],STDRIVERINFO_SIZE);
        if(strncmp((char*)drivercode,(char*)stDriver.drivercode,6)==0)
        {
            return i+1;
        }
        addr +=STDRIVERINFO_SIZE;
    }
    /////////////////////
    return 0;
}

/*************************************************************
** 函数名称: GetDriverInfoTotalNumber
** 功能描述: 获取当前驾驶员总个数
** 入口参数: 
** 出口参数: 无
** 返回参数: 0表示无驾驶员,非零为驾驶员个数
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char GetDriverInfoTotalNumber(void)
{
    unsigned char buffer[20];
    STMESSAGEHEAD  stMsgHead;
    MessageReadDataFromFlash(buffer,DRIVER_INFO_START_ADDR,STMESSAGEHEAD_SIZE);
    memcpy(&stMsgHead,buffer,STMESSAGEHEAD_SIZE);
    /////////////////////
    if(DRIVER_HEAD==stMsgHead.HeadData&&MSG_SAVE==stMsgHead.saveFlag)
    {
        return stMsgHead.recordCnt;
    }
    //////////////////////
    return 0;
}
/*************************************************************
** 函数名称: GetDriverInfoRecordNumberForCheckData
** 功能描述: 检查驾驶员姓名及驾驶员身份证编码，取得相应记录号(从1开始编号)
** 入口参数: 符合上报的格式的数据
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char GetDriverInfoRecordNumberForCheckData(unsigned char *data)
{
    unsigned char i,len;
    unsigned short addr;
    STDRIVERINFO stDriver;
    STMESSAGEHEAD  stMsgHead;    
    DriverInfoReadDataFromFlash();
    memcpy(&stMsgHead,s_ucMsgFlashBuffer,STMESSAGEHEAD_SIZE);
    addr = STMESSAGEHEAD_SIZE;
    for(i = 0; i < stMsgHead.recordCnt; i++)
    {
        memcpy(&stDriver,&s_ucMsgFlashBuffer[addr],STDRIVERINFO_SIZE);
        len=stDriver.data[0];//姓名长度
        len += 19;//驾驶员号码18个字节+姓名长度1个字节+姓名实际长度
        if(strncmp((char*)stDriver.data,(char*)data,len)==0)
        {
            return i+1;
        }
        addr +=STDRIVERINFO_SIZE;
    }
    /////////////////////
    return 0;
}
/*************************************************************
** 函数名称: CheckDriverInfoForLogin
** 功能描述: 
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char CheckDriverInfoForLogin(unsigned char *drivercode,unsigned char *password,unsigned char* recordnum)
{
    unsigned char i;
    unsigned short addr;
    STDRIVERINFO stDriver;
    STMESSAGEHEAD  stMsgHead;
    DriverInfoReadDataFromFlash();
    memcpy(&stMsgHead,s_ucMsgFlashBuffer,STMESSAGEHEAD_SIZE);
    addr = STMESSAGEHEAD_SIZE;
    for(i = 0; i < stMsgHead.recordCnt; i++)
    {
        memcpy(&stDriver,&s_ucMsgFlashBuffer[addr],STDRIVERINFO_SIZE);
        if(strncmp((char*)drivercode,(char*)stDriver.drivercode,6)==0)
        {
            if(strncmp((char*)password,(char*)stDriver.password,6)==0)
            {
                *recordnum =i+1;
                return 0;
            }
            else
            {
                ////////////////////
                Public_PlayTTSVoiceStr("密码错误!");
                return 1;
            }
        }
        addr +=STDRIVERINFO_SIZE;
    }
    /////////////////////
    Public_PlayTTSVoiceStr("该工号不存在");
    return 2;
}
/*************************************************************
** 函数名称: DriverInfoAdd
** 功能描述: 追加驾驶员信息
** 入口参数: 追加驾驶员信息
** 出口参数: 返回存储的记录号(编号从1开始),0:表示信息有误或保存出错
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char DriverInfoAdd(unsigned char *index,unsigned char *drivercode,unsigned char *password,unsigned char *data,unsigned short datalen)
{
    STDRIVERINFO stDriver;
    STMESSAGEHEAD stMsgHead;
    unsigned char num;
    /////////////////////////
    if(0==Messege_GetOneDriverInfoDatalen(data,datalen))
    {
        Public_PlayTTSVoiceStr("数据格式错误");
        return 4;
    }
    num=GetDriverInfoRecordNumber(drivercode);//看存在相同的工号没有
    if(num)
    {
        Public_PlayTTSVoiceStr("该工号已存在");
        return 1;
    }
    //////////////////////
    num=GetDriverInfoRecordNumberForCheckData(data);
    if(num)
    {
        Public_PlayTTSVoiceStr("该驾驶员已存在");
        return 2;
    }
    ///////////////////////////////////
    ////////////////////////////////////
    DriverInfoReadDataFromFlash();    
    memcpy(&stMsgHead,&s_ucMsgFlashBuffer[0],STMESSAGEHEAD_SIZE);
    /////////////////////////////////
    if((stMsgHead.currentAddr+STDRIVERINFO_SIZE)>INFODEMAND_DATALEN)
    {
        Public_PlayTTSVoiceStr("空间不足");
        return 3;
    }
    ////////////////////////////////////    
    stDriver.datalen = datalen;
    memcpy(stDriver.drivercode,drivercode,6);
    memcpy(stDriver.password,password,6);
    memcpy(stDriver.data,data,datalen);
    memcpy(&s_ucMsgFlashBuffer[stMsgHead.currentAddr],&stDriver,STDRIVERINFO_SIZE);
    ////////////////////
    stMsgHead.currentAddr +=STDRIVERINFO_SIZE;
    stMsgHead.recordCnt ++;
    memcpy(&s_ucMsgFlashBuffer[0],&stMsgHead,STMESSAGEHEAD_SIZE);
    DriverInfoSaveDataToFlash(); 
    ///////////////////
    *index =stMsgHead.recordCnt;
    return 0;
}

/*************************************************************
** 函数名称: DriverInfoModifyDriverCodeAndPassword
** 功能描述: 修改驾驶员信息工号/密码
** 入口参数: 修改驾驶员信息
** 出口参数: 
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char DriverInfoModifyDriverCodeAndPassword(unsigned char index ,unsigned char *drivercode,unsigned char *password)
{
    unsigned short addr;
    STDRIVERINFO stDriver;
    STMESSAGEHEAD stMsgHead;
    unsigned char num;
    /////////////////////
    DriverInfoReadDataFromFlash();    
    memcpy(&stMsgHead,&s_ucMsgFlashBuffer[0],STMESSAGEHEAD_SIZE);
    ///////////////////////////////
    if(stMsgHead.recordCnt<index)return 3;//未登录,
    if(index==0)return 3;//未登录,
    num=GetDriverInfoRecordNumber(drivercode);
    if(num&& (num != index))
    {
        Public_PlayTTSVoiceStr("该工号已存在");
        return 2;//如果工号存在,但不是当前需要修改的记录，则出错
    }
    /////////////////////////////////
    addr =(index-1)*STDRIVERINFO_SIZE;
    addr += STMESSAGEHEAD_SIZE;
    ///////////////////
    memcpy(&stDriver,&s_ucMsgFlashBuffer[addr],STDRIVERINFO_SIZE);
    //////////////////////////////////
    memcpy(stDriver.drivercode,drivercode,6);
    memcpy(stDriver.password,password,6);
    memcpy(&s_ucMsgFlashBuffer[addr],&stDriver,STDRIVERINFO_SIZE);
    ////////////////////
    DriverInfoSaveDataToFlash();  
    ///////////////////////////////
    Public_PlayTTSVoiceStr("修改成功,请记住修改内容");
    return 0;
}

/*************************************************************
** 函数名称: DriverInfoModify
** 功能描述: 修改驾驶员信息
** 入口参数: 修改驾驶员信息,index(编号从1开始),
** 出口参数: 
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char DriverInfoModify(unsigned char *index,unsigned char *drivercode,unsigned char *password,unsigned char *data,unsigned char datalen)
{
    unsigned short addr;
    STDRIVERINFO stDriver;
    STMESSAGEHEAD stMsgHead; 
    unsigned char num;
    /////////////////////
    if(0==Messege_GetOneDriverInfoDatalen(data,datalen))
    {
        Public_PlayTTSVoiceStr("数据格式错误");
        return 4;
    }
    DriverInfoReadDataFromFlash();    
    memcpy(&stMsgHead,&s_ucMsgFlashBuffer[0],STMESSAGEHEAD_SIZE);
    ///////////////////////////////
    if(stMsgHead.recordCnt<*index)
    {
        Public_PlayTTSVoiceStr("该记录号不存在");
        return 4;
    }
    ////////////////////
    if(*index==0)
    {
        Public_PlayTTSVoiceStr("该记录号为0");
        return 4;
    }
    //////////////////////////
    num=GetDriverInfoRecordNumber(drivercode);
    if(num&& (num != *index))
    {
        Public_PlayTTSVoiceStr("该工号已存在");
        return 1;//如果工号存在,但不是当前需要修改的记录，则出错
    }
    /////////////////////
    num=GetDriverInfoRecordNumberForCheckData(data);
    if(num&& (num != *index))
    {
        Public_PlayTTSVoiceStr("该驾驶员已存在");
        return 2;//如果驾驶员姓名、驾驶员身份证编码存在,但不是当前需要修改的记录，则出错
    }
    ////////////////////////////
    /////////////////////////////////
    addr =(*index-1)*STDRIVERINFO_SIZE;
    addr += STMESSAGEHEAD_SIZE;
    memcpy(&stDriver,&s_ucMsgFlashBuffer[addr],STDRIVERINFO_SIZE);
    stDriver.datalen = datalen;
    memcpy(stDriver.drivercode,drivercode,6);
    memcpy(stDriver.password,password,6);
    memcpy(stDriver.data,data,datalen);
    memcpy(&s_ucMsgFlashBuffer[addr],&stDriver,STDRIVERINFO_SIZE);
    ////////////////////
    DriverInfoSaveDataToFlash();  
    return 0;
}

/*************************************************************
** 函数名称: DriverInfoAddOrModify
** 功能描述: 追加/修改驾驶员信息
** 入口参数: 追加/修改驾驶员信息
** 出口参数: 返回存储的记录号(编号从1开始)0:表示信息有误或保存出错
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char DriverInfoAddOrModify(unsigned char *index,unsigned char *drivercode,unsigned char *password,unsigned char *data,unsigned short datalen)
{
    unsigned char result;
    STMESSAGEHEAD stMsgHead; 
    /////////////////////
    DriverInfoReadDataFromFlash();    
    memcpy(&stMsgHead,&s_ucMsgFlashBuffer[0],STMESSAGEHEAD_SIZE);
    if(stMsgHead.recordCnt>=*index)
    {
        result=DriverInfoModify(index,drivercode,password,data,datalen);
        if(0==result)
        {
            Public_PlayTTSVoiceStr("驾驶员修改成功");
        }
    }
    else
    {
        result=DriverInfoAdd(index,drivercode,password,data,datalen);
        if(0==result)
        {
            Public_PlayTTSVoiceStr("驾驶员添加成功");
        }
    }
    ///////////////
    return result;
    ///////////////////////////////
}
/*************************************************************
** 函数名称: MessageReadAndCheckPhoneBook
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 
*************************************************************/
void MessageReadAndCheckPhoneBook(void) //
{
    STMESSAGEHEAD stMsgHead;
    ////////////////////////////////////
    MessageReadDataFromFlash(s_ucMsgFlashBuffer,PHONEBOOK_START_ADDR,STMESSAGEHEAD_SIZE);
    memcpy(&stMsgHead,s_ucMsgFlashBuffer,STMESSAGEHEAD_SIZE);
    if((stMsgHead.HeadData != PHONE_HEAD)|| (stMsgHead.saveFlag != MSG_SAVE)||(stMsgHead.currentAddr>PHONEBOOK_DATALEN)||(stMsgHead.recordCnt*STPHONEBOOK_SIZE+STMESSAGEHEAD_SIZE!= stMsgHead.currentAddr))
    {
        PhoneBookDeleteAll();
    }
}
/*************************************************************
** 函数名称: MessageReadAndCheckEventReport
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 
*************************************************************/
void MessageReadAndCheckEventReport(void) //
{
    STMESSAGEHEAD stMsgHead;
    /////////////////////////////////
    MessageReadDataFromFlash(s_ucMsgFlashBuffer,EVENTREPORT_START_ADDR,STMESSAGEHEAD_SIZE);
    memcpy(&stMsgHead,s_ucMsgFlashBuffer,STMESSAGEHEAD_SIZE);
    if((stMsgHead.HeadData != EVENT_HEAD)|| (stMsgHead.saveFlag != MSG_SAVE)||(stMsgHead.currentAddr>INFODEMAND_DATALEN)||(stMsgHead.recordCnt*STMESSAGE_SIZE+STMESSAGEHEAD_SIZE!= stMsgHead.currentAddr))
    {
        //EventReportDeleteAll();
        EventReportDefaultValue();
    }   
}
/*************************************************************
** 函数名称: MessageReadAndCheckInfoDemand
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 
*************************************************************/
void MessageReadAndCheckInfoDemand(void) //
{
    STMESSAGEHEAD stMsgHead;
    /////////////////////////////////
    MessageReadDataFromFlash(s_ucMsgFlashBuffer,INFODEMAND_START_ADDR,STMESSAGEHEAD_SIZE);
    memcpy(&stMsgHead,s_ucMsgFlashBuffer,STMESSAGEHEAD_SIZE);
    if((stMsgHead.HeadData != INFO_HEAD)|| (stMsgHead.saveFlag != MSG_SAVE)||(stMsgHead.currentAddr>INFODEMAND_DATALEN) ||(stMsgHead.recordCnt*STMESSAGE_SIZE+STMESSAGEHEAD_SIZE!= stMsgHead.currentAddr))
    {
        InfoDemandDeleteAll();
    }   
}
/*************************************************************
** 函数名称: MessageReadAndCheckInfoDemand
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 
*************************************************************/
void MessageReadAndCheckDriverInfo(void) //
{
    STMESSAGEHEAD stMsgHead;
    /////////////////////////////////
    MessageReadDataFromFlash(s_ucMsgFlashBuffer,DRIVER_INFO_START_ADDR,STMESSAGEHEAD_SIZE);
    memcpy(&stMsgHead,s_ucMsgFlashBuffer,STMESSAGEHEAD_SIZE);
    if((stMsgHead.HeadData != DRIVER_HEAD)|| (stMsgHead.saveFlag != MSG_SAVE)||(stMsgHead.currentAddr>DRIVER_INFO_DATALEN)||(stMsgHead.recordCnt*STDRIVERINFO_SIZE+STMESSAGEHEAD_SIZE!= stMsgHead.currentAddr))
    {
        DriverInfoDeleteAll();
    }
}

/*************************************************************
** 函数名称: InfoServiceDeleteAll
** 功能描述: 删除所有信息服务内容
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void InfoServiceDeleteAll(void)
{
    STMESSAGEHEAD stMsgHead;
    MessageFlashFormat(INFO_SERVICE_START_ADDR);
    ////////////////////////////////////
    Message_stMsgHeadInit(&stMsgHead,INFO_SERVICE_HEAD);
    /////////////////////////////////////
    MessageSaveDataToFlash((unsigned char *)&stMsgHead,INFO_SERVICE_START_ADDR,STMESSAGEHEAD_SIZE);
    ///////////////////////////////
}
/*************************************************************
** 函数名称: MessageReadAndCheckInfoService
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 
*************************************************************/
void MessageReadAndCheckInfoService(void) //
{
    STMESSAGEHEAD stMsgHead;
    /////////////////////////////////
    MessageReadDataFromFlash(s_ucMsgFlashBuffer,INFO_SERVICE_START_ADDR,STMESSAGEHEAD_SIZE);
    memcpy(&stMsgHead,s_ucMsgFlashBuffer,STMESSAGEHEAD_SIZE);
    if((stMsgHead.HeadData != INFO_SERVICE_HEAD)|| (stMsgHead.saveFlag != MSG_SAVE)||(stMsgHead.currentAddr>INFO_SERVICE_DATALEN)||(stMsgHead.recordCnt*ONE_INFO_SERVICE_DATA_LEN+STMESSAGEHEAD_SIZE!= stMsgHead.currentAddr))
    {
        InfoServiceDeleteAll();
    }    
}

/*************************************************************
** 函数名称: QuestionResponseDeleteAll
** 功能描述: 删除所有提问应答信息
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void QuestionResponseDeleteAll(void)
{
    STMESSAGEHEAD stMsgHead;
    MessageFlashFormat(QUESTION_RESPONSE_START_ADDR);
    ////////////////////////////////////
    Message_stMsgHeadInit(&stMsgHead,QUESTION_HEAD);
    /////////////////////////////////////
    MessageSaveDataToFlash((unsigned char *)&stMsgHead,QUESTION_RESPONSE_START_ADDR,STMESSAGEHEAD_SIZE);
    ///////////////////////////////
}
/*************************************************************
** 函数名称: MessageReadAndCheckInfoService
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 
*************************************************************/
void MessageReadAndCheckQuestionResponse(void) //
{
    STMESSAGEHEAD stMsgHead;
    /////////////////////////////////
    MessageReadDataFromFlash(s_ucMsgFlashBuffer,QUESTION_RESPONSE_START_ADDR,STMESSAGEHEAD_SIZE);
    memcpy(&stMsgHead,s_ucMsgFlashBuffer,STMESSAGEHEAD_SIZE);
    if((stMsgHead.HeadData != QUESTION_HEAD)|| (stMsgHead.saveFlag != MSG_SAVE)||(stMsgHead.currentAddr>QUESTION_RESPONSE_DATALEN)||(stMsgHead.recordCnt*ONE_QUESTION_RESPONSE_DATA_LEN+STMESSAGEHEAD_SIZE!= stMsgHead.currentAddr))
    {
        QuestionResponseDeleteAll();
    }    
}
/*************************************************************
** 函数名称: MessageFlashReadAndCheck()
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 
*************************************************************/
void MessageFlashReadAndCheck(void) //
{
    ////////////////////////////////////
    MessageReadAndCheckPhoneBook();
    /////////////////////////////////
    MessageReadAndCheckEventReport();
    //////////////////////////////////
    MessageReadAndCheckInfoDemand();
    //////////////////////////
    MessageReadAndCheckDriverInfo();  
    ////////////////////////////////////
    MessageReadAndCheckInfoService();
    //////////////////////////////////
    MessageReadAndCheckQuestionResponse();
}
/*************************************************************
** 
**     Public  code 
** 
**************************************************************/
/*************************************************************
** 函数名称: CheckPhoneInformation
** 功能描述: 检查电话号码
** 入口参数: phone：需要检查的电话号码首地址； 
** 出口参数: name：返回的联系人姓名
** 返回参数: 0：没有该电话号码的资料；1: 呼入；2: 呼出；3：呼入/呼出
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char CheckPhoneInformation(unsigned char *number,unsigned char *name)
{
    unsigned char i;
    unsigned short addr;
    STMSG_PHONEBOOK stPhone;
    STMESSAGEHEAD  stMsgHead; 
    if(!strlen((char *)number))return 0;
    PhoneBookReadDataFromFlash();
    memcpy(&stMsgHead,s_ucMsgFlashBuffer,STMESSAGEHEAD_SIZE);
    addr = STMESSAGEHEAD_SIZE;
    for(i = 0; i < stMsgHead.recordCnt; i++)
    {
        memcpy(&stPhone,&s_ucMsgFlashBuffer[addr],STPHONEBOOK_SIZE);
        if(!strcmp((char *)stPhone.number,(char *)number))
        {
            strcpy((char *)name,(char *)stPhone.name);
            return stPhone.callflag;
        }
        addr += STPHONEBOOK_SIZE;
    }
    
    return 0;
}

/*************************************************************
** 函数名称: GetPhoneBook
** 功能描述: 取得电话簿
** 入口参数: callflag：电话簿属性标志；0所有；1: 呼入；2: 呼出；3：呼入/呼出
** 出口参数: buffer：返回相应属性标志的电话簿(格式见协议《电话本格式表》)
** 返回参数: 实际长度
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned short GetPhoneBook(unsigned char callflag,unsigned char *buffer)
{
    unsigned char i,length,num;
    unsigned short index,addr;
    STMSG_PHONEBOOK stPhone;
    STMESSAGEHEAD  stMsgHead; 
    PhoneBookReadDataFromFlash();
    memcpy(&stMsgHead,s_ucMsgFlashBuffer,STMESSAGEHEAD_SIZE);
    addr=STMESSAGEHEAD_SIZE;
    ///////////////////////
    index = 1;
    ////////////////////
    num =0;
    for(i = 0; i < stMsgHead.recordCnt; i++)
    {
        //////////////////////////////
        memcpy(&stPhone,&s_ucMsgFlashBuffer[addr],STPHONEBOOK_SIZE);
        ///////////////////////////////
        if((stPhone.callflag == callflag)||!callflag)
        {
            num ++;
            buffer[index++] = stPhone.callflag;
            /////////////////////
            length =strlen((char *)stPhone.number);
            buffer[index++] = length;
            memcpy(&buffer[index],&stPhone.number,length);
            index +=length;
            ///////////////////////
            length =strlen((char *)stPhone.name);
            buffer[index++] = length;
            memcpy(&buffer[index],&stPhone.name,length);
            index +=length;
        }
        addr +=STPHONEBOOK_SIZE;
    }
    ///////////////
    buffer[0] = num;
    /////////////////
    return index;
}
/*************************************************************
** 函数名称: GetEventReport
** 功能描述: 取得事件报告列表
** 入口参数: 
** 出口参数: 
** 返回参数: 实际长度
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned short GetEventReport(unsigned char *buffer)
{
    unsigned char i,length;
    unsigned short index,addr;
    STMESSAGE stEvent;
    STMESSAGEHEAD  stMsgHead; 
    EventReportReadDataFromFlash();
    memcpy(&stMsgHead,s_ucMsgFlashBuffer,STMESSAGEHEAD_SIZE);
    addr=STMESSAGEHEAD_SIZE;
    ///////////////////////
    index = 0;    
    buffer[index++] = stMsgHead.recordCnt;
    ////////////////////
    for(i = 0; i < stMsgHead.recordCnt; i++)
    {
        //////////////////////////////
        memcpy(&stEvent,&s_ucMsgFlashBuffer[addr],STMESSAGE_SIZE);
        ///////////////////////////////
        buffer[index++] = stEvent.ID;
        /////////////////////
        length =strlen((char *)stEvent.content);
        buffer[index++] = length;
        memcpy(&buffer[index],&stEvent.content,length);
        index +=length;
        ///////////////////////
        addr +=STMESSAGE_SIZE;
    }
    ///////////////
    /////////////////
    return index;
}
/*************************************************************
** 函数名称: GetInfoDemand
** 功能描述: 取得信息点播列表
** 入口参数: 
** 出口参数: 
** 返回参数: 实际长度
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned short GetInfoDemand(unsigned char *buffer)
{
    unsigned char i,length;
    unsigned short index,addr;
    STMESSAGE stInfo;
    STMESSAGEHEAD  stMsgHead; 
    InfoDemandReadDataFromFlash();
    memcpy(&stMsgHead,s_ucMsgFlashBuffer,STMESSAGEHEAD_SIZE);
    addr=STMESSAGEHEAD_SIZE;
    ///////////////////////
    index = 0;
    buffer[index++] = stMsgHead.recordCnt;    
    ////////////////////
    for(i = 0; i < stMsgHead.recordCnt; i++)
    {
        //////////////////////////////
        memcpy(&stInfo,&s_ucMsgFlashBuffer[addr],STMESSAGE_SIZE);
        ///////////////////////////////
        buffer[index++] = stInfo.ID;
        /////////////////////
        length =strlen((char *)stInfo.content);
        buffer[index++] = length;
        memcpy(&buffer[index],&stInfo.content,length);
        index +=length;
        buffer[index++] = stInfo.status;
        ///////////////////////
        addr +=STMESSAGE_SIZE;
    }
    ///////////////
    /////////////////
    return index;
}

/*************************************************************
** 函数名称: InfoDemandChangeStatus
** 功能描述: 更改信息点播状态
** 入口参数: ID:信息点播ID,status修改的状态
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void InfoDemandChangeStatus(unsigned char ID,unsigned char status)
{
    unsigned short addr;
    STMESSAGEHEAD  stMsgHead;
    InfoDemandReadDataFromFlash();
    memcpy(&stMsgHead,s_ucMsgFlashBuffer,STMESSAGEHEAD_SIZE);
    addr=GetAddressFromInfoDemand(ID,stMsgHead.recordCnt);
    if(addr)//找到信息点播
    {
        addr += STMESSAGE_SIZE;
        s_ucMsgFlashBuffer[addr-1] = status;
        /////////////////////////////
    }
    /////////////////////////////////
    /////////////////////////////////
    InfoDemandSaveDataToFlash();
}
/*************************************************************
** 函数名称: GetDriverInfoAll
** 功能描述: 取得相应记录号的驾驶员信息
** 入口参数: recordNumber记录号,buffer返回数据的首地址,flag标志:0:表示详细信息(符合上报的结构),1:表示详细信息(字符串格式,显示屏显示用),2:表示驾驶员姓名、驾驶员代码
** 出口参数: 无
** 返回参数: 返回数据的长度,为0表示没有相应记录号的驾驶员信息
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned short GetDriverInfoAll(unsigned char *buffer,unsigned char flag)
{
    unsigned char i,temp;
    unsigned short addr,len,index;
    STMESSAGEHEAD  stMsgHead;
    STDRIVERINFO stDriver;
    DriverInfoReadDataFromFlash();
    memcpy(&stMsgHead,s_ucMsgFlashBuffer,STMESSAGEHEAD_SIZE);
    ////////////////////////////
    len = 0;
    addr = STMESSAGEHEAD_SIZE;
    if(flag ==2||flag ==3)
    {
       buffer[len++] = stMsgHead.recordCnt;
    }
    /////////////////
    if(stMsgHead.recordCnt==0)return len;
    ///////////////////////
    for(i=0; i<stMsgHead.recordCnt;i++)
    {
        memcpy(&stDriver,&s_ucMsgFlashBuffer[addr],STDRIVERINFO_SIZE);    
        ///////////////////////    
        if(flag ==0)////上报平台的数据结构//////////////////////
        {
            temp=stDriver.datalen;        
            memcpy(&buffer[len],stDriver.data,temp);
            len+=temp;
        }
        else
        if(flag ==1)//字符串,显示用////////////////////////
        {
            memcpy(&buffer[len],stDriver.drivercode,6);
            len+=6;
            index =0;
            //姓名长度//////// 
            temp=stDriver.data[index++];
            //姓名//////// 
            memcpy(&buffer[len],&stDriver.data[index],temp);       
            index += temp;
            len += temp;
            buffer[len++]=',';
            ////驾驶员身份证编码 20//////////////////// 
            temp=strlen((char *)&stDriver.data[index]);
            if(temp>20)temp = 20;        
            memcpy(&buffer[len],&stDriver.data[index],temp);  
            index += 20;
            len += temp;
            buffer[len++]=',';
            /////////从业资格证编码 40/////////////
            temp=strlen((char *)&stDriver.data[index]);
            if(temp>40)temp = 40;        
            memcpy(&buffer[len],&stDriver.data[index],temp);   
            index += 40;
            len += temp;
            buffer[len++]=',';        
            /////发证机构名称长度//////////////////
            temp=stDriver.data[index++];
            ////发证机构名称///////////
            memcpy(&buffer[len],&stDriver.data[index],temp);       
            //index += temp;
            len += temp;
        }
        else
        if(flag ==2)////表示驾驶员代码、驾驶员姓名/////////////////////
        {
            memcpy(&buffer[len],stDriver.drivercode,6);
            len += 6;
            temp=stDriver.data[0];
            temp++;
            memcpy(&buffer[len],&stDriver.data[0],temp);
            len += temp;
        }
        else
        if(3 == flag)//////表示驾驶员代码、驾驶员密码,上报平台的数据结构/////////////////////
        {
            memcpy(&buffer[len],stDriver.drivercode,6);
            len += 6;
            memcpy(&buffer[len],stDriver.password,6);
            len += 6;
            temp = stDriver.datalen;
            memcpy(&buffer[len],stDriver.data,temp);
            len+=temp;
        }
        addr += STDRIVERINFO_SIZE;
    }
    //////////////////////////
    return len;
}
/*************************************************************
** 函数名称: GetDriverInfo
** 功能描述: 取得相应记录号的驾驶员信息
** 入口参数: recordNumber记录号,buffer返回数据的首地址,flag标志:0:表示详细信息(符合上报的结构),1:表示详细信息(字符串格式,显示屏显示用),2:表示驾驶员姓名、驾驶员代码
** 出口参数: 无
** 返回参数: 返回数据的长度,为0表示没有相应记录号的驾驶员信息
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned short GetDriverInfo(unsigned char recordNumber,unsigned char *buffer,unsigned char flag)
{
    unsigned char number,temp;
    unsigned short index,len,addr;
    STMESSAGEHEAD  stMsgHead;
    STDRIVERINFO stDriver;
    if(0 == recordNumber)return 0;
    DriverInfoReadDataFromFlash();
    number =recordNumber;
    memcpy(&stMsgHead,s_ucMsgFlashBuffer,STMESSAGEHEAD_SIZE);
    if(stMsgHead.recordCnt<number)return 0;
    addr = (number-1)*STDRIVERINFO_SIZE;
    addr += STMESSAGEHEAD_SIZE;
    memcpy(&stDriver,&s_ucMsgFlashBuffer[addr],STDRIVERINFO_SIZE);    
    ///////////////////////
    len = 0;
    if(flag ==0)////上报平台的数据结构//////////////////////
    {
        temp=stDriver.datalen;        
        memcpy(&buffer[len],stDriver.data,temp);
        len+=temp;
    }
    else
    if(flag ==1)//字符串,显示用////////////////////////
    {
        memcpy(&buffer[len],stDriver.drivercode,6);
        len+=6;
        index =0;
        //姓名长度//////// 
        temp=stDriver.data[index++];
        //姓名//////// 
        memcpy(&buffer[len],&stDriver.data[index],temp);       
        index += temp;
        len += temp;
        buffer[len++]=',';
        ////驾驶员身份证编码 20//////////////////// 
        temp=strlen((char *)&stDriver.data[index]);
        if(temp>20)temp = 20;        
        memcpy(&buffer[len],&stDriver.data[index],temp);  
        index += 20;
        len += temp;
        buffer[len++]=',';
        /////////从业资格证编码 40/////////////
        temp=strlen((char *)&stDriver.data[index]);
        if(temp>40)temp = 40;        
        memcpy(&buffer[len],&stDriver.data[index],temp);   
        index += 40;
        len += temp;
        buffer[len++]=',';        
        /////发证机构名称长度//////////////////
        temp=stDriver.data[index++];
        ////发证机构名称///////////
        memcpy(&buffer[len],&stDriver.data[index],temp);       
        //index += temp;
        len += temp;
    }
    else
    if(flag ==2)////表示驾驶员代码、驾驶员姓名/////////////////////
    {
        memcpy(&buffer[len],stDriver.drivercode,6);
        len += 6;
        temp=stDriver.data[0];
        temp++;
        memcpy(&buffer[len],&stDriver.data[0],temp);
        len += temp;
    }
    else
    if(3 == flag)////表示驾驶员代码、驾驶员密码,上报平台的数据结构/////////////////////
    {
        memcpy(&buffer[len],stDriver.drivercode,6);
        len += 6;
        memcpy(&buffer[len],stDriver.password,6);
        len += 6;
        temp=stDriver.datalen;
        memcpy(&buffer[len],stDriver.data,temp);
        len+=temp;
    }
    else
    if(4 == flag)////表示北斗上报平台的数据结构/////////////////////
    {
        len = 0;
        index =0;
        //姓名长度////////
        temp=stDriver.data[index++];
        buffer[len++]=temp;
        //姓名//////// 
        memcpy(&buffer[len],&stDriver.data[index],temp);       
        index += temp;
        len += temp;
        ////驾驶员身份证编码 20//////////////////// 
        temp=20;      
        memcpy(&buffer[len],&stDriver.data[index],temp);  
        index += temp;
        len += temp;
        ////////////
        index += 40;
        /////发证机构名称长度//////////////////
        temp=stDriver.data[index++];
        buffer[len++]=temp;
        ////发证机构名称///////////
        memcpy(&buffer[len],&stDriver.data[index],temp);
        len += temp;
        buffer[len++]=0x20;
        buffer[len++]=0x99;
        buffer[len++]=0x12;
        buffer[len++]=0x31;
    }
    //////////////////////////
    return len;
}
/*************************************************************
** 函数名称: SaveCurrentDriverParameter
** 功能描述: 
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void SaveCurrentDriverParameter(unsigned char recordnumber)
{
    unsigned long val;
    unsigned char number,buffer[10];
    unsigned short addr;
    STMESSAGEHEAD  stMsgHead;
    STDRIVERINFO stDriver;
    if(0 == recordnumber)return;
    DriverInfoReadDataFromFlash();
    number =recordnumber;
    memcpy(&stMsgHead,s_ucMsgFlashBuffer,STMESSAGEHEAD_SIZE);
    if(stMsgHead.recordCnt<number)return;
    addr = (number-1)*STDRIVERINFO_SIZE;
    addr += STMESSAGEHEAD_SIZE;
    memcpy(&stDriver,&s_ucMsgFlashBuffer[addr],STDRIVERINFO_SIZE);
    memcpy(buffer,&stDriver.drivercode,6);
    buffer[6]='\0';
    val=atol((char *)buffer);
    Public_ConvertLongToBuffer(val,buffer);
    //写驾驶员代码到eeprom
    EepromPram_WritePram(E2_CURRENT_DRIVER_ID, &buffer[1], 3);
    //////////////////////////
    number=stDriver.data[0];
    number++;
    EepromPram_WritePram(E2_CURRENT_LICENSE_ID, &stDriver.data[number], 18);    
}
/*************************************************************
** 函数名称: InfoService_SaveOneData
** 功能描述: 保存一条信息服务数据
** 入口参数: pBuffer:数据首地址,datalen数据长度
** 出口参数: 无
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void InfoService_SaveOneData(unsigned char *pBuffer,unsigned short datalen)
{
    STMESSAGEHEAD  stMsgHead;
    InfoServiceReadDataFromFlash();
    memcpy(&stMsgHead,s_ucMsgFlashBuffer,STMESSAGEHEAD_SIZE);
    ////////////////////////////
    if(stMsgHead.recordCnt<INFO_SERVICE_MAX_COUNT)
    {
        stMsgHead.curindex=stMsgHead.recordCnt;
        stMsgHead.recordCnt++;
        stMsgHead.oldestindex =0;
    }
    else
    {
        stMsgHead.curindex++;
        if(stMsgHead.curindex>=INFO_SERVICE_MAX_COUNT)
        {
            stMsgHead.curindex = 0;
        }
        //////////////////
        stMsgHead.oldestindex = stMsgHead.curindex+1;
        if(stMsgHead.oldestindex>=INFO_SERVICE_MAX_COUNT)
        {
            stMsgHead.oldestindex =0;
        }
    }
    //////////////////
    stMsgHead.currentAddr=stMsgHead.curindex*ONE_INFO_SERVICE_DATA_LEN;
    stMsgHead.currentAddr +=STMESSAGEHEAD_SIZE;
    ///////////////////////////////////////
    if(datalen>ONE_INFO_SERVICE_DATA_LEN-2)datalen = ONE_INFO_SERVICE_DATA_LEN-2;
    /////////////////////////////////////
    Public_ConvertShortToBuffer(datalen,&s_ucMsgFlashBuffer[stMsgHead.currentAddr]);
    memcpy(&s_ucMsgFlashBuffer[stMsgHead.currentAddr+2],pBuffer,datalen);
    ////////////////////////////////////////////////////
    stMsgHead.HeadData = INFO_SERVICE_HEAD;
    stMsgHead.saveFlag = MSG_SAVE;
    memcpy(&s_ucMsgFlashBuffer[0],&stMsgHead,STMESSAGEHEAD_SIZE);
    InfoServiceSaveDataToFlash();
}
/*************************************************************
** 函数名称: InfoService_ReadOneData
** 功能描述: 读取指定编号的一条信息服务数据
** 入口参数: index编号从1开始(从新到旧),pBuffer:数据首地址,
** 出口参数: 无
** 返回参数: 数据长度
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned short InfoService_ReadOneData(unsigned char index,unsigned char *pBuffer)
{
    unsigned char i;
    unsigned short addr;
    unsigned short datalen;
    STMESSAGEHEAD  stMsgHead;
    MessageReadDataFromFlash(s_ucMsgFlashBuffer,INFO_SERVICE_START_ADDR,STMESSAGEHEAD_SIZE);
    memcpy(&stMsgHead,s_ucMsgFlashBuffer,STMESSAGEHEAD_SIZE);
    if(0==stMsgHead.recordCnt||index>stMsgHead.recordCnt)return 0;
    i = (stMsgHead.recordCnt+stMsgHead.oldestindex-index)%stMsgHead.recordCnt;
    addr = i*ONE_INFO_SERVICE_DATA_LEN;
    addr +=STMESSAGEHEAD_SIZE;
    InfoServiceReadDataFromFlash();
    datalen=Public_ConvertBufferToShort(&s_ucMsgFlashBuffer[addr]);
    if(datalen>ONE_INFO_SERVICE_DATA_LEN-2)
    {
        return 0;
    }
    /////////////////////////////////
    memcpy(pBuffer,&s_ucMsgFlashBuffer[addr+2],datalen);
    return datalen;
}
/*************************************************************
** 函数名称: InfoService_GetTotalNum
** 功能描述: 取得信息服务总条数
** 入口参数: 
** 出口参数: 无
** 返回参数: 总条数
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char InfoService_GetTotalNum(void)
{
    STMESSAGEHEAD  stMsgHead;
    MessageReadDataFromFlash(s_ucMsgFlashBuffer,INFO_SERVICE_START_ADDR,STMESSAGEHEAD_SIZE);
    memcpy(&stMsgHead,s_ucMsgFlashBuffer,STMESSAGEHEAD_SIZE);
    if(INFO_SERVICE_HEAD==stMsgHead.HeadData&&MSG_SAVE==stMsgHead.saveFlag)
    {
        return stMsgHead.recordCnt;
    }
    ///////////////////////
    return 0;    
}
/*************************************************************
** 函数名称: QuestionResponse_SaveOneData
** 功能描述: 保存一条提问应答数据
** 入口参数: pBuffer:数据首地址,datalen数据长度
** 出口参数: 无
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void QuestionResponse_SaveOneData(unsigned char *pBuffer,unsigned short datalen)
{
    STMESSAGEHEAD  stMsgHead;
    QuestionResponseReadDataFromFlash();
    memcpy(&stMsgHead,s_ucMsgFlashBuffer,STMESSAGEHEAD_SIZE);
    ////////////////////////////
    if(stMsgHead.recordCnt<QUESTION_RESPONSE_MAX_COUNT)
    {
        stMsgHead.curindex=stMsgHead.recordCnt;
        stMsgHead.recordCnt++;
        stMsgHead.oldestindex =0;
    }
    else
    {
        stMsgHead.curindex++;
        if(stMsgHead.curindex>=QUESTION_RESPONSE_MAX_COUNT)
        {
            stMsgHead.curindex = 0;
        }
        //////////////////
        stMsgHead.oldestindex = stMsgHead.curindex+1;
        if(stMsgHead.oldestindex>=QUESTION_RESPONSE_MAX_COUNT)
        {
            stMsgHead.oldestindex =0;
        }
    }
    //////////////////
    stMsgHead.currentAddr = stMsgHead.curindex*ONE_QUESTION_RESPONSE_DATA_LEN;
    stMsgHead.currentAddr += STMESSAGEHEAD_SIZE;
    ///////////////////////////////////////
    if(datalen>ONE_QUESTION_RESPONSE_DATA_LEN-2)datalen = ONE_QUESTION_RESPONSE_DATA_LEN-2;
    /////////////////////////////////////
    Public_ConvertShortToBuffer(datalen,&s_ucMsgFlashBuffer[stMsgHead.currentAddr]);
    memcpy(&s_ucMsgFlashBuffer[stMsgHead.currentAddr+2],pBuffer,datalen);
    ////////////////////////////////////////////////////
    stMsgHead.HeadData = QUESTION_HEAD;
    stMsgHead.saveFlag = MSG_SAVE;
    memcpy(&s_ucMsgFlashBuffer[0],&stMsgHead,STMESSAGEHEAD_SIZE);
    QuestionResponseSaveDataToFlash();
}
/*************************************************************
** 函数名称: QuestionResponse_ReadOneData
** 功能描述: 读取指定编号的一条提问应答数据
** 入口参数: index编号从1开始(从新到旧),pBuffer:数据首地址,
** 出口参数: 无
** 返回参数: 数据长度
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned short QuestionResponse_ReadOneData(unsigned char index,unsigned char *pBuffer)
{
    unsigned char i;
    unsigned short addr;
    unsigned short datalen;
    STMESSAGEHEAD  stMsgHead;
    MessageReadDataFromFlash(s_ucMsgFlashBuffer,QUESTION_RESPONSE_START_ADDR,STMESSAGEHEAD_SIZE);
    memcpy(&stMsgHead,s_ucMsgFlashBuffer,STMESSAGEHEAD_SIZE);
    if(0==stMsgHead.recordCnt||index>stMsgHead.recordCnt)return 0;
    i = (stMsgHead.recordCnt+stMsgHead.oldestindex-index)%stMsgHead.recordCnt;
    addr = i*ONE_QUESTION_RESPONSE_DATA_LEN;
    addr +=STMESSAGEHEAD_SIZE;
    QuestionResponseReadDataFromFlash();
    datalen=Public_ConvertBufferToShort(&s_ucMsgFlashBuffer[addr]);
    if(datalen>ONE_QUESTION_RESPONSE_DATA_LEN-2)
    {
        return 0;
    }
    /////////////////////////////////
    memcpy(pBuffer,&s_ucMsgFlashBuffer[addr+2],datalen);
    return datalen;
}
/*************************************************************
** 函数名称: QuestionResponse_GetTotalNum
** 功能描述: 取得提问应答总条数
** 入口参数: 
** 出口参数: 无
** 返回参数: 总条数
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char QuestionResponse_GetTotalNum(void)
{
    STMESSAGEHEAD  stMsgHead;
    MessageReadDataFromFlash(s_ucMsgFlashBuffer,QUESTION_RESPONSE_START_ADDR,STMESSAGEHEAD_SIZE);
    memcpy(&stMsgHead,s_ucMsgFlashBuffer,STMESSAGEHEAD_SIZE);
    if(QUESTION_HEAD==stMsgHead.HeadData&&MSG_SAVE==stMsgHead.saveFlag)
    {
        return stMsgHead.recordCnt;
    }
    ///////////////////////
    return 0;
}
/*************************************************************
** 函数名称: EventReport_DisposeRadioProtocol
** 功能描述: 事件报告操作协议解析
** 入口参数: pBuffer:数据首地址,BufferLen:数据长度
** 出口参数: 无
** 返回参数: 返回操作结果
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char EventReport_DisposeRadioProtocol(unsigned char *pBuffer,unsigned short BufferLen)
{
    unsigned char SetType;
    SetType = *pBuffer++;
    switch(SetType)
    {
        case SETTYPE_DELETE:
            EventReportDeleteAll();
            break;
        case SETTYPE_UPDATE:
            EventReportUpdate(pBuffer,BufferLen-1);
            break;
        case SETTYPE_ADD:
            EventReportAdd(pBuffer,BufferLen-1);
            break;
        case SETTYPE_MODIFY:
            EventReportModify(pBuffer,BufferLen-1);
            break;
        case SETTYPE_DEL_EX:
            return EventReportDelEx(pBuffer,BufferLen-1);
            //break;
        default:
            return 3;
    }
    return 0;
}
/*************************************************************
** 函数名称: InfoDemand_DisposeRadioProtocol
** 功能描述: 信息点播操作协议解析
** 入口参数: pBuffer:数据首地址,BufferLen:数据长度
** 出口参数: 无
** 返回参数: 返回操作结果
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char InfoDemand_DisposeRadioProtocol(unsigned char *pBuffer,unsigned short BufferLen)
{
    unsigned char SetType;
    SetType = *pBuffer++;
    switch(SetType)
    {
        case SETTYPE_DELETE:
            InfoDemandDeleteAll();
            break;
        case SETTYPE_UPDATE:
            InfoDemandUpdate(pBuffer,BufferLen-1);
            break;
        case SETTYPE_ADD:
            InfoDemandAdd(pBuffer,BufferLen-1);
            break;
        case SETTYPE_MODIFY:
            InfoDemandModify(pBuffer,BufferLen-1);
            break;
        default:
            return 3;
    }
    return 0;
}
/*************************************************************
** 函数名称: PhoneBook_DisposeRadioProtocol
** 功能描述: 电话簿操作协议解析
** 入口参数: pBuffer:数据首地址,BufferLen:数据长度
** 出口参数: 无
** 返回参数: 返回操作结果
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char PhoneBook_DisposeRadioProtocol(unsigned char *pBuffer,unsigned short BufferLen)
{
    unsigned char SetType;
    SetType = *pBuffer++;
    switch(SetType)
    {
        case SETTYPE_DELETE:
            PhoneBookDeleteAll();
            break;
        case SETTYPE_UPDATE:
            PhoneBookUpdate(pBuffer,BufferLen-1);
            break;
        case SETTYPE_ADD:
            PhoneBookAdd(pBuffer,BufferLen-1);
            break;
        case SETTYPE_MODIFY:
            PhoneBookModify(pBuffer,BufferLen-1);
            break;
        default:
            return 3;
    }
    return 0;
}
/*************************************************************
** 函数名称: Message_GetPhoneBookTotalNum
** 功能描述: 取得电话簿总条数
** 入口参数: 
** 出口参数: 无
** 返回参数: 总条数
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Message_GetPhoneBookTotalNum(void)
{
    unsigned char buffer[STMESSAGEHEAD_SIZE];
    STMESSAGEHEAD  stMsgHead;
    MessageReadDataFromFlash(buffer,PHONEBOOK_START_ADDR,STMESSAGEHEAD_SIZE);
    memcpy(&stMsgHead,buffer,STMESSAGEHEAD_SIZE);
    if(PHONE_HEAD==stMsgHead.HeadData&&MSG_SAVE==stMsgHead.saveFlag)
    {
        return stMsgHead.recordCnt;
    }
    ///////////////////////
    return 0;    
}
/*************************************************************
** 函数名称: Message_GetOnePhoneBookContent
** 功能描述: 取得某条电话簿内容
** 入口参数: index索引号,stPhoneBook结构体指针
** 出口参数: 无
** 返回参数: 0,获取失败
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Message_GetOnePhoneBookContent(unsigned char index,STMSG_PHONEBOOK *stPhoneBook)
{
    unsigned short addr;
    STMESSAGEHEAD  stMsgHead;
    if(0==index)return 0;
    MessageReadDataFromFlash(s_ucMsgFlashBuffer,PHONEBOOK_START_ADDR,STMESSAGEHEAD_SIZE);
    memcpy(&stMsgHead,s_ucMsgFlashBuffer,STMESSAGEHEAD_SIZE);
    if(PHONE_HEAD!=stMsgHead.HeadData||MSG_SAVE!=stMsgHead.saveFlag)return 0;
    if(index>stMsgHead.recordCnt)return 0;
    PhoneBookReadDataFromFlash();
    addr = (index-1)*STPHONEBOOK_SIZE;
    addr +=STMESSAGEHEAD_SIZE;
    /////////////////////////////////
    memcpy(stPhoneBook,&s_ucMsgFlashBuffer[addr],STPHONEBOOK_SIZE);
    return STPHONEBOOK_SIZE;
}
/*************************************************************
** 函数名称: DriverInfo_DisposeRadioProtocol
** 功能描述: 驾驶员添加协议解析
** 入口参数: pBuffer:数据首地址,BufferLen:数据长度
** 出口参数: 无
** 返回参数: 返回操作结果
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char DriverInfo_DisposeRadioProtocol(unsigned char *pBuffer,unsigned short BufferLen)
{
    unsigned char buffer[10];
    unsigned char temp,i;
    unsigned short index,addr,len; 
    index =0;
    i =1;
    //////////////
    DriverInfoDeleteAll();  
    ////////////////////
    while(index<BufferLen)
    {
        addr =index;
        /////////////////////
        temp=pBuffer[index++];
        if(temp>20)return 1;//超过10个汉字
        index +=temp;
        ///////////////
        index +=20;
        index +=40;
        if(index>BufferLen)return 2;//长度
        temp=pBuffer[index++];
        if(temp>80)return 3;//超过40个汉字
        index +=temp;
        if(index>BufferLen)return 2;//长度
        //////////////////////////
        len=index-addr;
        if(len>ONE_DRIVER_INFO_MAX_LEN)return 4;//超过总长度
        sprintf((char *)buffer,"%06d",i);
        DriverInfoAdd(&temp,buffer,"000000",&pBuffer[addr],len);
        i++;
    };
    return 0;
}
/*************************************************************
** 函数名称: DriverInfo_ReadDataForRadioProtocol
** 功能描述: 读取驾驶员数据协议解析
** 入口参数: pBuffer:数据首地址,
** 出口参数: 无
** 返回参数: 数据长度
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned short DriverInfo_ReadDataForRadioProtocol(unsigned char *pBuffer)
{
    return GetDriverInfoAll(pBuffer,0);
}
/*************************************************************
** 函数名称: DriverInfo_GetOneDatalen
** 功能描述: 取得一个驾驶员信息体的长度
** 入口参数: pBuffer:数据首地址,BufferLen:数据长度
** 出口参数: 无
** 返回参数: 返回操作结果
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Messege_GetOneDriverInfoDatalen(unsigned char *pBuffer,unsigned short BufferLen)
{
    unsigned char temp;
    unsigned char len;
    if(BufferLen<62||BufferLen>ONE_DRIVER_INFO_MAX_LEN)return 0;//
    len = 0;
    temp=pBuffer[len++];//驾驶员姓名长度
    if(temp>20)return 0;//
    len +=temp;////驾驶员姓名
    ///////////////
    len +=20;///驾驶员身份证编码
    len +=40;
    if(len+1>BufferLen)return 0;//
    temp=pBuffer[len++];//发证机构内容长度
    if(temp>80)return 0;//
    len +=temp;    
    ////////////////////
    if(len>BufferLen)return 0;
    //////////////////////////
    return len;
}
/*************************************************************
** 函数名称: DriverInfo_PlatformReadData
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Messege_PlatformReadDriverInfoData(void)
{
    unsigned char buffer[1024];
    unsigned short len;
    buffer[0] = 5;//读操作
    buffer[1] = 0;//保留字节
    len=GetDriverInfoAll(&buffer[2],3);
    len +=2;
    EIExpand_PotocolSendData(EIEXPAND_CMD_SET_DRIVER_INFO,buffer,len);
}
/*************************************************************
** 函数名称: Messege_CheckDriverInfodatalen
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Messege_CheckDriverInfodatalen(unsigned char *pBuffer, unsigned short BufferLen)
{
    unsigned char *pTmp;
    unsigned short datalen;
    unsigned char sum,i,templen;   
    sum = *pBuffer++;
    datalen = BufferLen-1;
    pTmp =pBuffer;
    if(sum)
    {
        for(i= 0; i<sum; i++) 
        {
            templen = 12;//
            pTmp += templen;
            if(datalen < templen)return 0;
            templen = Messege_GetOneDriverInfoDatalen(pTmp,datalen);
            if(0==templen)return 0;
            if(datalen>templen)
            {
                pTmp += templen;
                datalen -=templen;
            }
            else
            if(datalen==templen)//
            {
                return 1;
            }
            else
            {
                return 0;
            }
        }
    }
    return 1;
}
/*************************************************************
** 函数名称: Messege_DriverInfoUpdate
** 功能描述: 
** 入口参数: pBuffer:消息体内容首地址,BufferLen消息体内容长度
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Messege_DriverInfoUpdate(unsigned char *pBuffer, unsigned short BufferLen)
{
    unsigned char flag;
    unsigned char *p;    
    unsigned char i,sum;
    STDRIVERINFO stDriver;
    STMESSAGEHEAD stMsgHead;
    unsigned short len,datalen;
    p = pBuffer;
    flag = Messege_CheckDriverInfodatalen(p,BufferLen);
    if(0==flag)return 0;    
    //////////////
    ////////////////
    p = pBuffer;
    sum = *p++;
    datalen = BufferLen - 1; 
    Message_stMsgHeadInit(&stMsgHead,DRIVER_HEAD);
    memset(s_ucMsgFlashBuffer,0,sizeof(s_ucMsgFlashBuffer));
    ////////////////////
    for(i= 0; i<sum; i++) 
    {
        memcpy(stDriver.drivercode,p,6);
        p += 6;
        memcpy(stDriver.password,p,6);
        p += 6;
        datalen -= 12;
        len = Messege_GetOneDriverInfoDatalen(p,datalen);
        memcpy(stDriver.data,p,len);
        stDriver.datalen=len;
        p += len;
        if(stMsgHead.currentAddr +STDRIVERINFO_SIZE>DRIVER_INFO_DATALEN)break;
        memcpy(&s_ucMsgFlashBuffer[stMsgHead.currentAddr],&stDriver,STDRIVERINFO_SIZE);
        ////////////////////
        stMsgHead.currentAddr += STDRIVERINFO_SIZE;
        stMsgHead.recordCnt++;
        if(datalen>len)datalen -= len;
        else break;
    }
    /////////////////////
    memcpy(&s_ucMsgFlashBuffer[0],&stMsgHead,STMESSAGEHEAD_SIZE);
    DriverInfoSaveDataToFlash(); 
    ////////////////
    return 1;    
}
/*************************************************************
** 函数名称: Messege_DriverInfoAdd
** 功能描述: 
** 入口参数: pBuffer:消息体内容首地址,BufferLen消息体内容长度
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Messege_DriverInfoAdd(unsigned char *pBuffer, unsigned short BufferLen)
{
    unsigned char *p,*pTmp;    
    unsigned char i,sum;
    unsigned short len,datalen;
    STDRIVERINFO stDriver;
    STMESSAGEHEAD stMsgHead;
    p = pBuffer;
    sum = *p++;
    if(0 == sum)return 0;
    ////////////////////
    pTmp = p;
    datalen = BufferLen - 1;
    for(i= 0; i<sum; i++) 
    {
        if(GetDriverInfoRecordNumber(pTmp))return 0;
        datalen -= 12;
        pTmp +=12;
        len = Messege_GetOneDriverInfoDatalen(pTmp,datalen);
        if(0 ==len)return 0;
        if(GetDriverInfoRecordNumberForCheckData(pTmp))return 0;
        pTmp +=len;
        if(datalen>len)datalen -= len;
        else break;
    }
    //////////////////////////////////
    pTmp = p;
    datalen = BufferLen - 1;
    ///////////////////////////////////
    DriverInfoReadDataFromFlash();
    memcpy(&stMsgHead,s_ucMsgFlashBuffer,STMESSAGEHEAD_SIZE);
    for(i= 0; i<sum; i++) 
    {
        memcpy(stDriver.drivercode,pTmp,6);
        pTmp += 6;
        memcpy(stDriver.password,pTmp,6);
        pTmp += 6;
        datalen -= 12;
        len = Messege_GetOneDriverInfoDatalen(pTmp,datalen);
        stDriver.datalen = len;
        memcpy(stDriver.data,pTmp,len);        
        if(stMsgHead.currentAddr +STDRIVERINFO_SIZE>DRIVER_INFO_DATALEN)break;
        memcpy(&s_ucMsgFlashBuffer[stMsgHead.currentAddr],&stDriver,STDRIVERINFO_SIZE);
        ////////////////////
        pTmp += len;
        stMsgHead.currentAddr += STDRIVERINFO_SIZE;
        stMsgHead.recordCnt++;
        if(datalen>len)datalen -= len;
        else break;
    }
    /////////////////////
    stMsgHead.HeadData = DRIVER_HEAD;
    stMsgHead.saveFlag = MSG_SAVE;
    /////////////////////
    memcpy(&s_ucMsgFlashBuffer[0],&stMsgHead,STMESSAGEHEAD_SIZE);
    DriverInfoSaveDataToFlash(); 
    return 1;    
}
/*************************************************************
** 函数名称: Messege_DriverInfoModify
** 功能描述: 
** 入口参数: pBuffer:消息体内容首地址,BufferLen消息体内容长度
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Messege_DriverInfoModify(unsigned char *pBuffer, unsigned short BufferLen)
{
    STDRIVERINFO stDriver;
    STMESSAGEHEAD stMsgHead;
    unsigned char *p,*pTmp;    
    unsigned char i,sum,index1,index2;
    unsigned short len,datalen;
    unsigned char buffer[30];
    unsigned short addr;    
    p = pBuffer;
    sum = *p++;
    if(0 == sum)return 0;
    ////////////////////
    pTmp = p;
    datalen = BufferLen - 1;
    for(i= 0; i<sum; i++) 
    {
        index1=GetDriverInfoRecordNumber(pTmp);
        /////////////////////
        datalen -= 12;
        pTmp +=12;        
        len = Messege_GetOneDriverInfoDatalen(pTmp,datalen);
        if(0 ==len)return 0;
        index2 = GetDriverInfoRecordNumberForCheckData(pTmp);
        if(0==index1&&0==index2)return 0;//未找到
        if(index1&&index2&&index1!=index2)return 0;//都找到,但不同
        if(index1)//暂存ID
        {
            buffer[i] = index1-1;
        }
        else
        {
            buffer[i] = index2-1;
        }
        pTmp +=len;
        if(datalen>len)datalen -= len;
        else break;
    }
    //////////////////////////////////
    pTmp = p;
    datalen = BufferLen - 1;
    ///////////////////////////////////
    DriverInfoReadDataFromFlash();
    memcpy(&stMsgHead,s_ucMsgFlashBuffer,STMESSAGEHEAD_SIZE);
    for(i= 0; i<sum; i++) 
    {
        memcpy(stDriver.drivercode,pTmp,6);
        pTmp += 6;
        memcpy(stDriver.password,pTmp,6);
        pTmp += 6;
        datalen -= 12;
        len = Messege_GetOneDriverInfoDatalen(pTmp,datalen);
        stDriver.datalen=len;
        memcpy(stDriver.data,pTmp,len);        
        addr = buffer[i]*STDRIVERINFO_SIZE+STMESSAGEHEAD_SIZE;
        memcpy(&s_ucMsgFlashBuffer[addr],&stDriver,STDRIVERINFO_SIZE);
        pTmp += len;
        ////////////////////
        if(datalen>len)datalen -= len;
        else break;
    }
    /////////////////////
    stMsgHead.HeadData = DRIVER_HEAD;
    stMsgHead.saveFlag = MSG_SAVE;
    /////////////////////
    memcpy(&s_ucMsgFlashBuffer[0],&stMsgHead,STMESSAGEHEAD_SIZE);
    DriverInfoSaveDataToFlash(); 
    return 1;    
}
/*************************************************************
** 函数名称: Messege_DeleteOneDriverInfo
** 功能描述: 删除驾驶员姓名及驾驶员身份证编码
** 入口参数: 符合上报的格式的数据
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Messege_DeleteOneDriverInfo(unsigned char index)
{
    unsigned short addr,len;
    STMESSAGEHEAD  stMsgHead;
    DriverInfoReadDataFromFlash();
    memcpy(&stMsgHead,s_ucMsgFlashBuffer,STMESSAGEHEAD_SIZE);
    addr = index-1;
    addr *= STDRIVERINFO_SIZE;
    addr += STMESSAGEHEAD_SIZE;
    len = 0;
    if(index < stMsgHead.recordCnt)
    {
        len = stMsgHead.recordCnt-index;
        len *= STDRIVERINFO_SIZE;
        memmove(&s_ucMsgFlashBuffer[addr],&s_ucMsgFlashBuffer[addr+STDRIVERINFO_SIZE],len);
    }
    memset(&s_ucMsgFlashBuffer[addr+len],0,STDRIVERINFO_SIZE);
    stMsgHead.recordCnt--;
    stMsgHead.currentAddr -= STDRIVERINFO_SIZE;
    memcpy(&s_ucMsgFlashBuffer[0],&stMsgHead,STMESSAGEHEAD_SIZE);
    DriverInfoSaveDataToFlash();
}
/*************************************************************
** 函数名称: Messege_DriverInfoUpdate
** 功能描述: 
** 入口参数: pBuffer:消息体内容首地址,BufferLen消息体内容长度
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Messege_DriverInfoDelete(unsigned char *pBuffer, unsigned short BufferLen)
{
    unsigned char *p,*pTmp;    
    unsigned char i,sum;
    unsigned short index,len,datalen;
    p = pBuffer;
    sum = *p++;
    ////////////////////
    if(sum)//按要求删除某驾驶员
    {
        pTmp = p;
        datalen = BufferLen - 1;
        for(i= 0; i<sum; i++) 
        {
            datalen -= 12;
            pTmp +=12;
            len = Messege_GetOneDriverInfoDatalen(pTmp,datalen);
            if(0 ==len)return 0;
            if(0==GetDriverInfoRecordNumberForCheckData(pTmp))return 0;
            pTmp +=len;
            if(datalen>len)datalen -= len;
            else break;
        }
        //////////////////////////////////
        pTmp = p;
        datalen = BufferLen - 1;
        ///////////////////////////////////
        for(i= 0; i<sum; i++) 
        {
            datalen -= 12;
            pTmp += 12;
            len = Messege_GetOneDriverInfoDatalen(pTmp,datalen);
            index = GetDriverInfoRecordNumberForCheckData(pTmp);
            Messege_DeleteOneDriverInfo(index);
            pTmp +=len;
        }
    }
    else//删除全部
    {
        DriverInfoDeleteAll();
    }
    return 1;    
}
/*************************************************************
** 函数名称: DriverInfo_DisposeEIExpandProtocol
** 功能描述: 伊爱扩展协议——设置驾驶员信息
** 入口参数: pBuffer:消息体内容首地址,BufferLen消息体内容长度
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Messege_DisposeEIExpandProtocolForDriverInfo(unsigned char *pBuffer, unsigned short BufferLen)
{
    unsigned char AckResult=0;
    unsigned char operate;
    unsigned char *p;    
    p = pBuffer;
    operate = *p++;//操作标志
    if(MSG_SET_DRIVER_INFO_QUERY == operate)//读取
    {
        LZM_SetOperateTimer(Messege_PlatformReadDriverInfoData,2*SYSTICK_1SECOND);
        return 0;
    }
    ////////////////
    if(BufferLen<2)return 2;//
    //////////////////
    p++;//保留字节  
    //////////////////
    if(MSG_SET_DRIVER_INFO_UPDATE == operate)//更新
    {
        AckResult= Messege_DriverInfoUpdate(p,BufferLen-2);
    }
    else
    if(MSG_SET_DRIVER_INFO_ADD == operate)//追加
    {
         AckResult= Messege_DriverInfoAdd(p,BufferLen-2);
    }
    else
    if(MSG_SET_DRIVER_INFO_MODIFY == operate)//修改
    {
        AckResult= Messege_DriverInfoModify(p,BufferLen-2);
    }
    else
    if(MSG_SET_DRIVER_INFO_DELETE == operate)//删除
    {
       AckResult= Messege_DriverInfoDelete(p,BufferLen-2);
    }
    return(AckResult)?0:1;
}
/*************************************************************
** 函数名称: Message_ParameterInitialize
** 功能描述: 参数初始化
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Message_ParameterInitialize(void)
{
    MessageFlashReadAndCheck();
    #ifdef __DOWNLOAD_GB2313_H
    DownloadGB2313_CheckFlashArea();
    #endif
    #ifdef USE_PHONE_BOOK
    PhoneBook_ReadAndCheckCallRecord();
    #endif
}
#endif
/******************************************************************************
**                            End Of File
******************************************************************************/

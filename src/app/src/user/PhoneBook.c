/*******************************************************************************
 * File Name:			PhoneBook.c 
 * Function Describe:	电话簿
 * Relate Module:		
 * Writer:				Joneming
 * Date:				2013-09-02
 * ReWriter:			
 * Date:				
 *******************************************************************************/
#include "include.h"
/*******************************************************************************/
////////////////////////////////////////////
#if 1

///////////////////////////
typedef enum
{
    PHONE_STATUS_MISSED_IN,
    PHONE_STATUS_TALK_IN,
    PHONE_STATUS_MISSED_OUT,
    PHONE_STATUS_TALK_OUT,
    PHONE_STATUS_MAX
}E_PHONESTATUS;
////////////////////////
typedef struct
{
    unsigned long startTime;    //时间
    unsigned short duration;    //时长
    unsigned char status;       //状态 
    unsigned char callflag;     //标志    
    char name[10];              //姓名
    char number[20];            //电话号码
    char bak[2];                //保留
}STCALLRECORD;
static STCALLRECORD s_stCallRecord;
#define ONE_STPHONE_RECORD_SIZE sizeof(STCALLRECORD)

typedef enum 
{
    CALL_STATUS_HANG_UP,    
    CALL_STATUS_ANSWER_CALL,
    CALL_STATUS_NEW_PHONE,
    CALL_STATUS_WAIT_FOR_CALL,
    CALL_STATUS_MAX
}E_CALL_STATUS;


typedef enum 
{
    PHONE_TASK_EMPTY,    
    PHONE_TASK_NEW_PHONE,
    PHONE_TASK_HOT_LINE,
    PHONE_TASK_BOOK,
    PHONE_TASK_RECORD,
    PHONE_TASK_BACK,
    PHONE_TASK_MAX,
}E_PHONE_TASK;

typedef struct
{
    unsigned char HangUp;          //
    unsigned char control;          //
    unsigned char callstatus;       //状态
    unsigned char specialFlag;      //
    unsigned char answermode;       //
    unsigned char preTaskFlag;       //
}STPHONEBOOK_ATTRIB; 

static STPHONEBOOK_ATTRIB s_stPhoneBook;

struct SUT_MENU s_stMenuPhone;
const char *s_apcMenuPhone[]= 
{
    "拨打客服中心",
    "电 话 簿",
    "通话记录",
    "拨打电话",
	""
};
typedef enum 
{
    MENU_PHONE_HOT_LINE, 
    MENU_PHONE_BOOK,    
    MENU_PHONE_RECORD, 
    MENU_PHONE_MAKE,
    MENU_PHONE_MAX
}E_MENU_PHONE;


const char *c_apcPhoneAttrib[] =
{
    "",
    "呼入",
    "呼出",
    "呼入/呼出",
};

const char *c_apcCallStatus[] =
{
    "未接",
    "已接",
    "已拨",
    "已拨",
};
/////////////////////////

#define SAVE_PHONE_RECORD_SECTOR        (FLASH_CALL_RECORD_START_SECTOR)
#define SAVE_PHONE_RECORD_ADDR          (SAVE_PHONE_RECORD_SECTOR*FLASH_ONE_SECTOR_BYTES)//
#define PHONEBOOK_RECORD_DATALEN        (unsigned long)FLASH_ONE_SECTOR_BYTES
#define PHONE_RECORD_MAX_COUNT          (unsigned char)(((PHONEBOOK_RECORD_DATALEN-STMESSAGEHEAD_SIZE)/ONE_STPHONE_RECORD_SIZE)-3)
#define PHONE_RECORD_HEAD               0xDCBA
#define PHONE_RECORD_SAVE               0xbb
/////////////////////////////////////////////
LZM_RET TaskShowNewCallPhone(LZM_MESSAGE *Msg);
LZM_RET TaskShowDialing(LZM_MESSAGE *Msg);
LZM_RET TaskShowCallTalking(LZM_MESSAGE *Msg);
LZM_RET TaskShowMakeNewDial(LZM_MESSAGE *Msg);
/*************************************************************
** 函数名称: PhoneBook_ReadCallRecord
** 功能描述: Flash读数据
** 入口参数: s:数据指针，startAddr:开始地址(相对地址);lenght:长度
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void PhoneBook_ReadCallRecord(void) 
{
    sFLASH_ReadBuffer(s_ucMsgFlashBuffer,SAVE_PHONE_RECORD_ADDR,PHONEBOOK_RECORD_DATALEN);
}
/*************************************************************
** 函数名称: PhoneBook_SaveCallRecord
** 功能描述: Flash读数据
** 入口参数: s:数据指针，startAddr:开始地址(相对地址);lenght:长度
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void PhoneBook_SaveCallRecord(void) 
{
    ///////////////在写入之前先格式化//////////////////////
    sFLASH_EraseSector(SAVE_PHONE_RECORD_ADDR);
    ////////////////////////
    sFLASH_WriteBuffer(s_ucMsgFlashBuffer,SAVE_PHONE_RECORD_ADDR,PHONEBOOK_RECORD_DATALEN);
}
/*************************************************************
** 函数名称: PhoneBook_CallRecordDeleteAll
** 功能描述: 删除所有通话记录内容
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void PhoneBook_CallRecordDeleteAll(void)
{
    STMESSAGEHEAD stMsgHead;
    sFLASH_EraseSector(SAVE_PHONE_RECORD_ADDR);
    ////////////////////////////////////
    stMsgHead.currentAddr = STMESSAGEHEAD_SIZE;
    stMsgHead.HeadData = PHONE_RECORD_HEAD;
    stMsgHead.saveFlag = PHONE_RECORD_SAVE;
    stMsgHead.recordCnt = 0;
    stMsgHead.oldestindex = 0;
    stMsgHead.curindex = 0;
    /////////////////////////////////////
    sFLASH_WriteBuffer((unsigned char *)&stMsgHead,SAVE_PHONE_RECORD_ADDR,STMESSAGEHEAD_SIZE);
    ///////////////////////////////
}
/*************************************************************
** 函数名称: PhoneBook_ReadAndCheckCallRecord
** 功能描述: 检查通话记录是否合理
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 
*************************************************************/
void PhoneBook_ReadAndCheckCallRecord(void) //
{
    STMESSAGEHEAD stMsgHead;
    /////////////////////////////////
    sFLASH_ReadBuffer(s_ucMsgFlashBuffer,SAVE_PHONE_RECORD_ADDR,STMESSAGEHEAD_SIZE);
    memcpy(&stMsgHead,s_ucMsgFlashBuffer,STMESSAGEHEAD_SIZE);
    if((stMsgHead.HeadData != PHONE_RECORD_HEAD)|| (stMsgHead.saveFlag != PHONE_RECORD_SAVE)||(stMsgHead.currentAddr>PHONEBOOK_RECORD_DATALEN)||(stMsgHead.currentAddr<STMESSAGEHEAD_SIZE)|| (0 !=(stMsgHead.currentAddr-STMESSAGEHEAD_SIZE)%ONE_STPHONE_RECORD_SIZE))
    {
        PhoneBook_CallRecordDeleteAll();
    }    
}
/*************************************************************
** 函数名称: PhoneBook_SaveOneCallRecordData
** 功能描述: 保存一条通话记录数据
** 入口参数: pBuffer:数据首地址,datalen数据长度
** 出口参数: 无
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void PhoneBook_SaveOneCallRecordData(void)
{
    STMESSAGEHEAD  stMsgHead;
    PhoneBook_ReadCallRecord();
    memcpy(&stMsgHead,s_ucMsgFlashBuffer,STMESSAGEHEAD_SIZE);
    ////////////////////////////
    if(stMsgHead.recordCnt<PHONE_RECORD_MAX_COUNT)
    {
        stMsgHead.curindex=stMsgHead.recordCnt;
        stMsgHead.recordCnt++;
        stMsgHead.oldestindex =0;
    }
    else
    {
        stMsgHead.curindex++;
        if(stMsgHead.curindex>=PHONE_RECORD_MAX_COUNT)
        {
            stMsgHead.curindex = 0;
        }
        //////////////////
        stMsgHead.oldestindex = stMsgHead.curindex+1;
        if(stMsgHead.oldestindex>=PHONE_RECORD_MAX_COUNT)
        {
            stMsgHead.oldestindex =0;
        }
    }
    //////////////////
    stMsgHead.currentAddr=stMsgHead.curindex*ONE_STPHONE_RECORD_SIZE;
    stMsgHead.currentAddr +=STMESSAGEHEAD_SIZE;
    /////////////////////////////////////
    memcpy(&s_ucMsgFlashBuffer[stMsgHead.currentAddr],&s_stCallRecord,ONE_STPHONE_RECORD_SIZE);
    ////////////////////////////////////////////////////
    stMsgHead.HeadData = PHONE_RECORD_HEAD;
    stMsgHead.saveFlag = PHONE_RECORD_SAVE;
    memcpy(&s_ucMsgFlashBuffer[0],&stMsgHead,STMESSAGEHEAD_SIZE);
    PhoneBook_SaveCallRecord();
}
/*************************************************************
** 函数名称: PhoneBook_ReadOneCallRecordData
** 功能描述: 读取指定编号的一条通话记录数据
** 入口参数: index编号从1开始(从新到旧),pBuffer:数据首地址,
** 出口参数: 无
** 返回参数: 数据长度
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned short PhoneBook_ReadOneCallRecordData(unsigned char index,STCALLRECORD *stCallRecord)
{
    unsigned char i;
    unsigned short addr;
    STMESSAGEHEAD  stMsgHead;
    sFLASH_ReadBuffer(s_ucMsgFlashBuffer,SAVE_PHONE_RECORD_ADDR,STMESSAGEHEAD_SIZE);
    memcpy(&stMsgHead,s_ucMsgFlashBuffer,STMESSAGEHEAD_SIZE);
    if(0==stMsgHead.recordCnt||index>stMsgHead.recordCnt)return 0;
    i = (stMsgHead.recordCnt+stMsgHead.oldestindex-index)%stMsgHead.recordCnt;
    addr = i*ONE_STPHONE_RECORD_SIZE;
    addr +=STMESSAGEHEAD_SIZE;
    PhoneBook_ReadCallRecord();
    /////////////////////////////////
    memcpy(stCallRecord,&s_ucMsgFlashBuffer[addr],ONE_STPHONE_RECORD_SIZE);
    return ONE_STPHONE_RECORD_SIZE;
}
/*************************************************************
** 函数名称: PhoneBook_GetCallRecordTotalNum
** 功能描述: 取得通话记录总条数
** 入口参数: 
** 出口参数: 无
** 返回参数: 总条数
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char PhoneBook_GetCallRecordTotalNum(void)
{
    STMESSAGEHEAD  stMsgHead;
    sFLASH_ReadBuffer(s_ucMsgFlashBuffer,SAVE_PHONE_RECORD_ADDR,STMESSAGEHEAD_SIZE);
    memcpy(&stMsgHead,s_ucMsgFlashBuffer,STMESSAGEHEAD_SIZE);
    if(PHONE_RECORD_HEAD==stMsgHead.HeadData&&PHONE_RECORD_SAVE==stMsgHead.saveFlag)
    {
        return stMsgHead.recordCnt;
    }
    ///////////////////////
    return 0;    
}
/*************************************************************
** 函数名称: PhoneBook_ModifyCurCallRecord
** 功能描述: 修改当前通话记录内容
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void PhoneBook_ModifyCurCallRecord(void)
{
    STMESSAGEHEAD stMsgHead;
    PhoneBook_ReadCallRecord();
    memcpy(&stMsgHead,s_ucMsgFlashBuffer,STMESSAGEHEAD_SIZE);
    if(PHONE_RECORD_HEAD!=stMsgHead.HeadData||PHONE_RECORD_SAVE!=stMsgHead.saveFlag||0==stMsgHead.recordCnt)return;
    memcpy(&s_ucMsgFlashBuffer[stMsgHead.currentAddr],&s_stCallRecord,ONE_STPHONE_RECORD_SIZE);
    PhoneBook_SaveCallRecord();
}
/*******************************************************************************
** 函数名称: PhoneBook_HangUpCall
** 函数功能: 挂断电话
** 入口参数:  
** 出口参数: 无
** 返回参数:  
** 全局变量: 无
** 调用模块: 无
*******************************************************************************/
void PhoneBook_HangUpCall(void)
{
    communication_CallPhone("13800000000",PHONE_END);
    if(s_stPhoneBook.HangUp==0)
    {
        s_stPhoneBook.HangUp =1;
        LZM_SetOperateTimer(PhoneBook_HangUpCall,PUBLICSECS(0.1));
    }
    else
    {
        s_stPhoneBook.HangUp =0;
    }
}
/*******************************************************************************
** 函数名称: PhoneBook_SetCurrentCallStatus
** 函数功能: 设置当前通话状态
** 入口参数:  
** 出口参数: 无
** 返回参数:  
** 全局变量: 无
** 调用模块: 无
*******************************************************************************/
void PhoneBook_SetCurrentCallStatus(unsigned char value)
{
    s_stPhoneBook.callstatus = value;
    switch(value)
    {
        case CALL_STATUS_HANG_UP:            
            break;        
        case CALL_STATUS_ANSWER_CALL:
            break;  
        case CALL_STATUS_NEW_PHONE:
            s_stPhoneBook.preTaskFlag = PHONE_TASK_NEW_PHONE;
            LZM_StartNewTask(TaskShowNewCallPhone);
            break;
    }
}
/*************************************************************
** 函数名称: PhoneBook_DisposeNewCallPhone
** 功能描述: 新来电命令
** 入口参数: 电话号码
** 出口参数: 无
** 返回参数: 等待接听标志:0:直接拒接来电;1:正在等待用户接听来电
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char PhoneBook_DisposeNewCallPhone(unsigned char *phone)
{
    unsigned char name[24];
    unsigned char flag; 
    //////////////////////////////
    flag=Public_CheckPhoneInformation(phone,name);//0：没有该电话号码的资料；1: 呼入；2: 呼出；3：呼入/呼出
    if(2 == flag||0 == flag)//电话簿存在号码,但属于呼出
    {
        PhoneBook_HangUpCall();
        //直接拒接电话/////////////
        return 0;
    }
    else//电话簿存在号码,属于可呼入
    if(flag)
    {
        strcpy(s_stCallRecord.name,(char *)name);
    }
    /////////////////////////////////////
    s_stCallRecord.callflag = flag;
    strcpy(s_stCallRecord.number,(char *)phone);    
    PhoneBook_SetCurrentCallStatus(CALL_STATUS_NEW_PHONE);
    return 1;
}
/*************************************************************
** 函数名称: Public_DisposePhoneStatus
** 功能描述: 处理电话状态命令
** 入口参数: phone:电话号码,status:电话状态：0：为挂断;1:接通;2:新来电;
** 出口参数: 无
** 返回参数: 如果是电话呼入状态为新来电时,返回值:0:直接拒接来电;1:正在等待用户接听来电
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char PhoneBook_DisposePhoneStatus(unsigned char *phone,unsigned char status)
{
    unsigned char result;
    result =0;
    switch(status)
    {
        case 0://挂机
            s_stPhoneBook.specialFlag =0;
            PhoneBook_SetCurrentCallStatus(CALL_STATUS_HANG_UP);            
            break;
        case 1://通话            
            PhoneBook_SetCurrentCallStatus(CALL_STATUS_ANSWER_CALL);
            break;
        case 2://新来电
            if(CALL_STATUS_HANG_UP==s_stPhoneBook.callstatus)
            result=PhoneBook_DisposeNewCallPhone(phone);
            break;
    }
    return result;
}
/*************************************************************
** 函数名称: ShowPhoneBookIndex
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void ShowPhoneBookIndex(unsigned char index,unsigned char count)
{
    char buffer[20];
    STMSG_PHONEBOOK stPhoneBook;
    if(0xff==index)
    {
        LcdShowStr(0, 0, "电话簿", 1);
        LcdShowStr(0, 32, "姓名:", 0);
        LcdShowStr(0, 48, "属性:", 0);
    }
    else
    {
        LcdClearArea(0, 16, 128, 32);
        LcdClearArea(43, 32, 128, 63);
        Message_GetOnePhoneBookContent(index,&stPhoneBook);
        sprintf(buffer,"%d/%-2d",count,index);
        LcdShowStr(88,0,buffer,1);
        LcdShowStr(0,16,(char *)stPhoneBook.number,0);
        LcdShowStr(44,32,(char *)stPhoneBook.name,0);
        LcdShowStr(44,48,c_apcPhoneAttrib[stPhoneBook.callflag],0);       
    }
}
/*************************************************************
** 函数名称: PhoneBook_CheckPhoneCallResult
** 功能描述: 检查电话号码及呼叫结果
** 入口参数: phone电话号码,newname名字
** 出口参数: 无
** 返回参数: 0呼叫成功
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned char PhoneBook_CheckPhoneCallResult(unsigned char *phone,unsigned char *newname)
{
    unsigned char name[20];
    unsigned char flag;
    s_stPhoneBook.specialFlag =Public_CheckPhoneIsSpecialNumber(phone);
    if(0==s_stPhoneBook.specialFlag&&Public_CheckIsPhoneServerRunStatus())
    {
        Public_ShowTextInfo("行驶中,禁止呼叫",SECS(3));
        return 1;
    }
    flag=Public_CheckPhoneInformation((unsigned char *)phone,name);//0：没有该电话号码的资料；1: 呼入；2: 呼出；3：呼入/呼出
    if(1==flag||0==flag)//禁止呼入/呼出,1: 呼入；2: 呼出；3：呼入/呼出
    {
        s_stPhoneBook.control =0x55;
        Public_ShowTextInfo("该号码禁止呼出!",SECS(3));
        return 2;
    }
    else
    {
        flag=communication_CallPhone((unsigned char *)phone,PHONE_NORMAL);
        if(flag)
        {
            PhoneBook_SetCurrentCallStatus(CALL_STATUS_MAX);
            strcpy(s_stCallRecord.number,(char *)phone);
            if(newname==NULL||(!strcmp((char *)newname,"未知")))
            strcpy(s_stCallRecord.name,(char *)name);
            else
            strcpy(s_stCallRecord.name,(char *)newname);
            /////////////////////////////            
            LZM_StartNewTask(TaskShowDialing);
            return 0;
        }
        else
        {
            s_stPhoneBook.control =0x55;
            PhoneBook_HangUpCall();
            Public_ShowTextInfo("呼叫出错,请检查",SECS(3));
            return 3;
        }
    }
}
/*************************************************************
** 函数名称: ShowPhoneBookTalkCall
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void ShowPhoneBookTalkCall(unsigned char index)
{
    STMSG_PHONEBOOK stPhoneBook;
    /////////////////////////
    Message_GetOnePhoneBookContent(index,&stPhoneBook);    
    //////////////////////////////////
    PhoneBook_CheckPhoneCallResult((unsigned char *)stPhoneBook.number,(unsigned char *)stPhoneBook.name);    
}
/*************************************************************
** 函数名称: TaskShowPhoneBook
** 功能描述: "显示电话簿"菜单
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskShowPhoneBook(LZM_MESSAGE *Msg)
{
    unsigned char temp;
    static unsigned char sucStep=0;
    static unsigned char sucMaxCount=0;
    switch(Msg->message)
    {
        case TASK_START:
            LcdClearScreen();            
            temp=Message_GetPhoneBookTotalNum();
            if(temp==0)
            {
                sucStep=0;
                sucMaxCount=0;
                LcdShowCaption("电话簿为空!", 23);                
                LZM_SetAlarm(TEST_TIMER, SECS(2));
                s_stPhoneBook.control=0xff;
            }
            else
            {
                if(sucStep>=temp||temp!=sucMaxCount)
                {
                    sucStep =1;
                }
                sucMaxCount=temp;
                s_stPhoneBook.control = 0;
                ShowPhoneBookIndex(0xff,sucMaxCount);
                ShowPhoneBookIndex(sucStep,sucMaxCount);
            }
            return 1;
        case KEY_PRESS:
            if(s_stPhoneBook.control)break;
            switch(TaskPara)
            {
                case KEY_BACK://返回
                    LZM_StartNewTask(TaskMenuPhone);
                    break;
                case KEY_ENTER://确定
                    ShowPhoneBookTalkCall(sucStep);
                    break;
                case KEY_UP://--	
                    if(sucStep>1)
                    {
                        sucStep--;
                    }
                    else
                    {
                        sucStep = sucMaxCount;
                    }
                    ShowPhoneBookIndex(sucStep,sucMaxCount);
                    break;
                case KEY_DOWN://++	
                    if(sucStep<sucMaxCount)
                    {
                        sucStep++;
                    }
                    else
                    {
                        sucStep=1;
                    }
                    ShowPhoneBookIndex(sucStep,sucMaxCount);
                    break;
            }
            return 1;
        case TEST_TIMER:
            if(0xff==s_stPhoneBook.control)
            {
                LZM_StartNewTask(TaskMenuPhone);
            }
            break;
        case SHOW_TIMER:
            s_stPhoneBook.control =0;
            LcdPasteScreen();
            break;
        default:
            return LZM_DefTaskProc(Msg);
    }
    return 0;
}

/*************************************************************
** 函数名称: ShowPhoneBookIndex
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void ShowCallRecordIndex(unsigned char index,unsigned char count)
{
    unsigned char hh,mm,ss;
    TIME_T stTime;
    char buffer[20];
    STCALLRECORD stCallRecord;
    LcdClearScreen();
    PhoneBook_ReadOneCallRecordData(index,&stCallRecord);    
    LcdShowStr(0,0,(char *)stCallRecord.name,0);
    sprintf(buffer,"%d/%-2d",count,index);
    LcdShowStr(90,0,buffer,1);
    LcdShowStr(0,16,(char *)stCallRecord.number,0);
    LcdShowStr(0,32,c_apcCallStatus[stCallRecord.status],0);
    if(stCallRecord.duration)
    {
        hh=stCallRecord.duration/3600;
        mm=stCallRecord.duration/60;
        ss=stCallRecord.duration%60;
        sprintf(buffer,"%02d:%02d:%02d",hh,mm,ss);
        LcdShowStr(36,32,buffer,0);
    }
    /////////////////////////////////
    Gmtime(&stTime,stCallRecord.startTime);
    Public_LcdShowTime(48,stTime);
}
/*************************************************************
** 函数名称: ShowPhoneBookTalkCall
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void ShowCallRecordTalkCall(unsigned char index)
{
    STCALLRECORD stCallRecord;    
    PhoneBook_ReadOneCallRecordData(index,&stCallRecord);
    PhoneBook_CheckPhoneCallResult((unsigned char *)stCallRecord.number,(unsigned char *)stCallRecord.name);
}
/*************************************************************
** 函数名称: TaskShowCallRecord
** 功能描述: "显示电话簿"菜单
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskShowCallRecord(LZM_MESSAGE *Msg)
{
    unsigned char temp;
    static unsigned char sucStep=0;
    static unsigned char sucMaxCount=0;
    switch(Msg->message)
    {
        case TASK_START:
            s_stPhoneBook.preTaskFlag = PHONE_TASK_RECORD;
            temp=PhoneBook_GetCallRecordTotalNum();
            if(temp==0)
            {
                sucStep=0;
                sucMaxCount=0;
                LcdClearScreen();
                LcdShowCaption("通话记录为空!", 23);                
                LZM_SetAlarm(TEST_TIMER, SECS(2));
                s_stPhoneBook.control=0xff;
            }
            else
            {
                if(sucStep>=temp||temp!=sucMaxCount)
                {
                    sucStep =1;
                }
                sucMaxCount=temp;
                s_stPhoneBook.control = 0;
                ShowCallRecordIndex(sucStep,sucMaxCount);
            }
            return 1;
        case KEY_PRESS:
            if(s_stPhoneBook.control)break;
            switch(TaskPara)
            {
                case KEY_BACK://返回
                    LZM_StartNewTask(TaskMenuPhone);
                    break;
                case KEY_ENTER://确定
                    ShowCallRecordTalkCall(sucStep);
                    break;
                case KEY_UP://--	
                    if(sucStep>1)
                    {
                        sucStep--;
                    }
                    else
                    {
                        sucStep = sucMaxCount;
                    }
                    ShowCallRecordIndex(sucStep,sucMaxCount);
                    break;
                case KEY_DOWN://++	
                    if(sucStep<sucMaxCount)
                    {
                        sucStep++;
                    }
                    else
                    {
                        sucStep=1;
                    }
                    ShowCallRecordIndex(sucStep,sucMaxCount);
                    break;
            }
            return 1;
        case TEST_TIMER:
            if(0xff==s_stPhoneBook.control)
            {
                LZM_StartNewTask(TaskMenuPhone);
            }
            break;
        case SHOW_TIMER:
            s_stPhoneBook.control =0;
            LcdPasteScreen();
            break;
        default:
            return LZM_DefTaskProc(Msg);
    }
    return 0;
}
/*************************************************************
** 函数名称: TaskShowNewCallPhone
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskShowNewCallPhone(LZM_MESSAGE *Msg)
{
    unsigned char buffer[10];
    switch(Msg->message)
    {
        case TASK_START:
            if(s_stPhoneBook.preTaskFlag>=PHONE_TASK_BACK)
            {
                LZM_StartNewTask(TaskShowMainInterface);
                return 1;
            }
            s_stPhoneBook.answermode = 0;
            if(EepromPram_ReadPram(E2_TERMINAL_GET_PHONE_STRATEGY_ID,buffer))
            {
                s_stPhoneBook.answermode =(unsigned char)Public_ConvertBufferToLong(buffer);
            }
            LcdClearScreen();
            LcdShowStr(0, 0,"新来电", 1); 
            LcdShowStr(0, 16,s_stCallRecord.number, 0);
            LcdShowStr(0, 32,s_stCallRecord.name, 0);
            LcdShowStr(0, 48,"确定接,返回拒绝",0);           
            s_stCallRecord.duration = 0;
            s_stCallRecord.startTime =RTC_GetCounter();
            s_stCallRecord.status = PHONE_STATUS_MISSED_IN;
            PhoneBook_SaveOneCallRecordData();
            LZM_SetAlarm(REFRESH_TIMER, PUBLICSECS(70));
            LZM_SetAlarm(TEST_TIMER, 1);
            SysAutoRunCountReset();
            s_stPhoneBook.specialFlag = Public_CheckPhoneIsSpecialNumber((unsigned char *)s_stCallRecord.number);            
            return 1;
        case KEY_PRESS:
            switch(TaskPara)
            {
                case KEY_BACK://返回
                    LZM_SetAlarm(REFRESH_TIMER, 1); 
                    break;
                case KEY_ENTER://确定
                    communication_CallPhone(gPhone.telephone,PHONE_TALK);
                    PhoneBook_SetCurrentCallStatus(CALL_STATUS_ANSWER_CALL); 
                    LZM_SetAlarm(TEST_TIMER, 1);
                    break;                                
                case KEY_UP://--                    
                    break;
                case KEY_DOWN://++                    
                    break;
            }
            return 1;
        case REFRESH_TIMER:
            PhoneBook_HangUpCall();
            PhoneBook_SetCurrentCallStatus(CALL_STATUS_HANG_UP); 
            LZM_SetAlarm(TEST_TIMER, 1);
            break;
        case TEST_TIMER:
            if(CALL_STATUS_HANG_UP==s_stPhoneBook.callstatus)
            {
                s_stPhoneBook.preTaskFlag = PHONE_TASK_BACK;
                LZM_ReturnOldTaskEx();
            }
            else
            if(CALL_STATUS_ANSWER_CALL==s_stPhoneBook.callstatus)
            {
                LZM_StartNewTaskEx(TaskShowCallTalking);
            }
            else
            if(CALL_STATUS_WAIT_FOR_CALL==s_stPhoneBook.callstatus)
            {
                communication_CallPhone(gPhone.telephone,PHONE_TALK);
                PhoneBook_SetCurrentCallStatus(CALL_STATUS_ANSWER_CALL); 
                LZM_SetAlarm(TEST_TIMER, 1);
            }
            else
            {
                SysAutoRunCountReset();
                LZM_SetAlarm(TEST_TIMER, PUBLICSECS(1));
                if(0==s_stPhoneBook.specialFlag&&Public_CheckIsPhoneServerRunStatus())
                {
                    PhoneBook_HangUpCall();
                    PhoneBook_SetCurrentCallStatus(CALL_STATUS_HANG_UP);
                    LZM_SetAlarm(TEST_TIMER, 1);
                    break;
                }
                else
                if(Io_ReadStatusBit(STATUS_BIT_ACC)||(0==s_stPhoneBook.answermode))
                {
                    PhoneBook_SetCurrentCallStatus(CALL_STATUS_WAIT_FOR_CALL); 
                    LZM_SetAlarm(TEST_TIMER, SECS(2));
                }
            }
            break;
        default:
            return LZM_DefTaskProc(Msg);
    }
    return 0;
}
/*************************************************************
** 函数名称: TaskShowDialing
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskShowDialing(LZM_MESSAGE *Msg)
{
    switch(Msg->message)
    {
        case TASK_START:
            SysAutoRunCountReset();
            LcdClearScreen();
            LcdShowStr(0, 0,"正在呼叫...", 1);
            LcdShowStr(0, 16,s_stCallRecord.number, 0);
            LcdShowStr(0, 32,s_stCallRecord.name, 0);
            LcdShowStr(0, 48,"<返回>挂断", 0);
            s_stCallRecord.duration = 0;
            s_stCallRecord.startTime =RTC_GetCounter();
            s_stCallRecord.status = PHONE_STATUS_MISSED_OUT;
            PhoneBook_SaveOneCallRecordData();
            LZM_SetAlarm(TEST_TIMER, PUBLICSECS(1));
            LZM_SetAlarm(REFRESH_TIMER, PUBLICSECS(70));
            return 1;
        case KEY_PRESS:
            switch(TaskPara)
            {
                case KEY_BACK://返回
                    LZM_SetAlarm(REFRESH_TIMER, 1);                     
                    break;
                case KEY_ENTER://确定
                    break;                                
                case KEY_UP://--                    
                    break;
                case KEY_DOWN://++                    
                    break;
            }
            return 1;
        case REFRESH_TIMER:  
            PhoneBook_HangUpCall();
            PhoneBook_SetCurrentCallStatus(CALL_STATUS_HANG_UP); 
            LZM_SetAlarm(TEST_TIMER, 1);
            break;
        case TEST_TIMER:
            if(CALL_STATUS_HANG_UP==s_stPhoneBook.callstatus)
            {
                if(PHONE_TASK_BOOK==s_stPhoneBook.preTaskFlag)
                {
                    s_stPhoneBook.preTaskFlag = PHONE_TASK_BACK;
                    LZM_StartNewTask(TaskShowPhoneBook);
                }
                else
                if(PHONE_TASK_RECORD==s_stPhoneBook.preTaskFlag)
                {
                    s_stPhoneBook.preTaskFlag = PHONE_TASK_BACK;
                    LZM_StartNewTask(TaskShowCallRecord);
                }
                else
                {
                    s_stPhoneBook.preTaskFlag = PHONE_TASK_BACK;
                    LZM_StartNewTask(TaskMenuPhone);
                }
            }
            else
            if(CALL_STATUS_ANSWER_CALL==s_stPhoneBook.callstatus)
            {
                LZM_StartNewTaskEx(TaskShowCallTalking);
            }
            else
            {
                SysAutoRunCountReset();
                LZM_SetAlarm(TEST_TIMER, PUBLICSECS(1));
                if(0==s_stPhoneBook.specialFlag&&Public_CheckIsPhoneServerRunStatus())
                {
                    PhoneBook_HangUpCall();
                    PhoneBook_SetCurrentCallStatus(CALL_STATUS_HANG_UP); 
                    LZM_SetAlarm(TEST_TIMER, 1);
                    break;
                }
            }
            break;
        default:
            return LZM_DefTaskProc(Msg);
    }
    return 0;
}
/*************************************************************
** 函数名称: TaskShowCallTalking
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskShowCallTalking(LZM_MESSAGE *Msg)
{
    char buffer[20];
    unsigned char hh,mm,ss;
    switch(Msg->message)
    {
        case TASK_START:
            if(s_stPhoneBook.preTaskFlag>=PHONE_TASK_BACK)
            {
                LZM_StartNewTask(TaskShowMainInterface);
            }
            else
            {
                LcdClearScreen();
                LcdShowStr(0, 0,"通话中", 1);
                LcdShowStr(0, 16,s_stCallRecord.number, 0);
                LcdShowStr(0, 32,s_stCallRecord.name, 0);
                s_stCallRecord.duration = 0; 
                s_stCallRecord.status++;
                s_stCallRecord.startTime =RTC_GetCounter();            
                PhoneBook_ModifyCurCallRecord();
                LZM_SetTimer(TEST_TIMER, PUBLICSECS(1));
                SysAutoRunCountReset();
                s_stPhoneBook.control = 0;
            }
            return 1;
        case KEY_PRESS:
            switch(TaskPara)
            {
                case KEY_BACK://返回
                    PhoneBook_HangUpCall();
                    PhoneBook_SetCurrentCallStatus(CALL_STATUS_HANG_UP); 
                    LZM_SetAlarm(TEST_TIMER, 1);                     
                    break;
                case KEY_ENTER://确定
                    break;                                
                case KEY_UP://-- 
                    SetTelVolumeIncr();
                    s_stPhoneBook.control = 1;
                    break;
                case KEY_DOWN://++
                    SetTelVolumeDecr();
                    s_stPhoneBook.control = 1;
                    break;
            }
            return 1;
        case TEST_TIMER:
            if(CALL_STATUS_HANG_UP==s_stPhoneBook.callstatus)
            {
                s_stPhoneBook.preTaskFlag = PHONE_TASK_BACK;
                LZM_KillTimer(TEST_TIMER);
                PhoneBook_ModifyCurCallRecord();
                LZM_ReturnOldTaskEx();
            }
            else
            {
                if(0==s_stPhoneBook.specialFlag&&Public_CheckIsPhoneServerRunStatus())
                {
                    PhoneBook_HangUpCall();
                    PhoneBook_SetCurrentCallStatus(CALL_STATUS_HANG_UP); 
                    LZM_SetAlarm(TEST_TIMER, 1);
                    break;//
                }
                s_stCallRecord.duration++;
                hh=s_stCallRecord.duration/3600;
                mm=s_stCallRecord.duration/60;
                ss=s_stCallRecord.duration%60;
                sprintf(buffer,"%02d:%02d:%02d",hh,mm,ss);
                LcdShowStr(0, 48,buffer, 0);
                SysAutoRunCountReset();
                if(s_stPhoneBook.control)
                {
                    s_stPhoneBook.control++;
                    if(s_stPhoneBook.control<6)
                    {
                        sprintf(buffer,"v:%d",GetTelVolume());
                        LcdShowStr(100, 0,buffer, 0);
                    }
                    else
                    {
                        s_stPhoneBook.control = 0;
                        LcdClearArea(100, 0, 128,16);
                    }
                }
            }
            break;
        default:
            return LZM_DefTaskProc(Msg);
    }
    return 0;
}
/*************************************************************
** 函数名称: TakeCallMonitorServerPhone
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
void TakeCallMonitorServerPhone(void)
{
    unsigned char len;
    unsigned char buffer[20];
    len = EepromPram_ReadPram(E2_MONITOR_SERVER_PHONE_ID, buffer);
    buffer[len]='\0';
    if(0==len)
    {
        s_stPhoneBook.control =0x55;
        Public_ShowTextInfo("中心号码未设置",SECS(3));
        return;
    }
    //////////////////////////
    if(communication_CallPhone(buffer,PHONE_NORMAL))
    {
        PhoneBook_SetCurrentCallStatus(CALL_STATUS_MAX);
        strcpy(s_stCallRecord.number,(char *)buffer);
        strcpy(s_stCallRecord.name,"客服中心");
        s_stCallRecord.callflag = 3;
        s_stPhoneBook.specialFlag = 1;
        s_stPhoneBook.preTaskFlag = PHONE_TASK_HOT_LINE;
        LZM_StartNewTask(TaskShowDialing);
    }
    else
    {
        s_stPhoneBook.control =0x55;
        PhoneBook_HangUpCall();
        Public_ShowTextInfo("呼叫出错,请检查",SECS(3));
    }
}
/*************************************************************
** 函数名称: TaskMenuPhone
** 功能描述: 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskMenuPhone(LZM_MESSAGE *Msg)
{ 
    switch(Msg->message)
    {
        case TASK_START:
            MenuInit(&s_stMenuPhone,s_apcMenuPhone);
            MenuShow(&s_stMenuPhone);
            s_stPhoneBook.control =0;
            s_stPhoneBook.preTaskFlag = PHONE_TASK_EMPTY;
            return 1;
        case KEY_PRESS:
            if(s_stPhoneBook.control)break;
            switch(TaskPara)
            {
                case KEY_BACK:
                    LZM_StartNewTask(TaskMenuMain);
                    break;
                case KEY_ENTER:	
                    switch(MenuGetHandle(&s_stMenuPhone))
                    {
                        case MENU_PHONE_HOT_LINE:
                            TakeCallMonitorServerPhone();
                            break;
                        case MENU_PHONE_BOOK:
                            LZM_StartNewTask(TaskShowPhoneBook);
                            break;
                        case MENU_PHONE_RECORD://
                            LZM_StartNewTask(TaskShowCallRecord);
                            break;
                        case MENU_PHONE_MAKE://
                            LZM_StartNewTask(TaskShowMakeNewDial);
                            break;
                            
                    }
                    break;
                default:
                    MenuResponse(&s_stMenuPhone,TaskPara);
                    break;
            }
            return 1;
        case SHOW_TIMER:
            s_stPhoneBook.control =0;
            LcdPasteScreen();            
            break;
        default:
            return LZM_DefTaskProc(Msg);
    }
    return 0;
}

/*************************************************************
** 函数名称: TaskShowMakeNewDial
** 功能描述: 拨打电话 
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 
** 调用模块: 
*************************************************************/	
LZM_RET TaskShowMakeNewDial(LZM_MESSAGE *Msg)
{
    static ST_ESPIN stESpinPhone;
    switch(Msg->message)
    {
        case TASK_START:            
            LcdClearScreen();
            s_stPhoneBook.control=0;
            LcdShowCaption("拨打电话",0);
            LcdShowHorLine(0, 131, 18, 1);
            LcdShowHorLine(0, 131, 45, 1);
            LcdShowCaption("按<确认>拨打",47);
            if((strlen(stESpinPhone.buffer)!=12)||(stESpinPhone.width != 12))
            {
                ESpinInit(&stESpinPhone, 16, 24, 12,1);
                stESpinPhone.type = 1;//字符串
                strcpy(stESpinPhone.buffer,"000000000000"); 
            }
            ESpinShow(&stESpinPhone);
            ESpinSetFocus(&stESpinPhone);
            return 1;
        case KEY_PRESS:
            if(s_stPhoneBook.control==0xff)break;
            switch(TaskPara)
            {
                case KEY_BACK://返回
                    LZM_StartNewTask(TaskMenuPhone);//系统管理
                    break;
                case KEY_ENTER://确定
                    if(stESpinPhone.buffer[0]==0x30)
                    {
                        unsigned char flag;
                        unsigned char name[20];                        
                        flag=Public_CheckPhoneInformation((unsigned char *)&stESpinPhone.buffer[1],name);//0：没有该电话号码的资料；1: 呼入；2: 呼出；3：呼入/呼出
                        if(flag)
                        {
                            PhoneBook_CheckPhoneCallResult((unsigned char *)&stESpinPhone.buffer[1],NULL);
                        }
                        else
                        {
                            PhoneBook_CheckPhoneCallResult((unsigned char *)stESpinPhone.buffer,NULL);
                        }
                    }
                    else
                    {
                        PhoneBook_CheckPhoneCallResult((unsigned char *)stESpinPhone.buffer,NULL);
                    }
                    break;                                
                default:                    
                    ESpinResponse(&stESpinPhone,TaskPara);                     
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
/*******************************************************************************
 *                             end of module
 *******************************************************************************/


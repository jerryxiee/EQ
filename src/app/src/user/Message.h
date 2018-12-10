/*******************************************************************************
 * File Name:			Message.h 
 * Function Describe:	
 * Relate Module:		
 * Writer:				joneming
 * Date:				2012-05-23
 * Rewriter:
 * Date:		
 *******************************************************************************/
#ifndef __MESSAGE_H_
#define __MESSAGE_H_ 

//#define EI_MSG_SET_DRIVER_INFO 
#define MESSAGE
/////////////////////////////
extern unsigned char s_ucMsgFlashBuffer[];
///////////////////////////
typedef struct
{
    unsigned short currentAddr;     //当前空闲的首地址; 
    unsigned short HeadData;        //数据开头0xabcd;
    unsigned char saveFlag;         //存储标志
    unsigned char recordCnt;        //记录数
    unsigned char oldestindex;      //最老记录索引号
    unsigned char curindex;         //当前记录索引号
}STMESSAGEHEAD;//8字节
#define STMESSAGEHEAD_SIZE sizeof(STMESSAGEHEAD)
///////////////////////////////
typedef struct
{ 
    unsigned char ID;           //ID
    unsigned char content[78];  //内容
    unsigned char status;         //标志:信息点播用到/事件报告没用
}STMESSAGE;
#define STMESSAGE_SIZE sizeof(STMESSAGE)
/////////////////////////////////////
typedef struct
{ 
    unsigned char callflag;     //标志
    unsigned char number[20];   //电话 
    unsigned char name[24];     //名称       
    unsigned char reverse[3];   //保留
}STMSG_PHONEBOOK;
////////////////////////////////////
#define STPHONEBOOK_SIZE sizeof(STMSG_PHONEBOOK)

/*************************************************************
** 函数名称: Message_ParameterInitialize
** 功能描述: 参数初始化
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Message_ParameterInitialize(void);
/*************************************************************
** 函数名称: CheckPhoneInformation
** 功能描述: 检查电话号码
** 入口参数: phone：需要检查的电话号码首地址； 
** 出口参数: name：返回的联系人姓名
** 返回参数: 0：没有该电话号码的资料；1: 呼入；2: 呼出；3：呼入/呼出
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char CheckPhoneInformation(unsigned char *number,unsigned char *name);
/*************************************************************
** 函数名称: GetPhoneBook
** 功能描述: 取得电话簿
** 入口参数: callflag：电话簿属性标志；0所有；1: 呼入；2: 呼出；3：呼入/呼出
** 出口参数: buffer：返回相应属性标志的电话簿(格式见协议《电话本格式表》)
** 返回参数: 实际长度
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned short GetPhoneBook(unsigned char callflag,unsigned char *buffer);
/*************************************************************
** 函数名称: GetEventReport
** 功能描述: 取得事件报告列表
** 入口参数: 
** 出口参数: 
** 返回参数: 实际长度
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned short GetEventReport(unsigned char *buffer);
/*************************************************************
** 函数名称: GetInfoDemand
** 功能描述: 取得信息点播列表
** 入口参数: 
** 出口参数: 
** 返回参数: 实际长度
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned short GetInfoDemand(unsigned char *buffer);
/*************************************************************
** 函数名称: InfoDemandChangeStatus
** 功能描述: 更改信息点播状态
** 入口参数: ID:信息点播ID,status修改的状态
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void InfoDemandChangeStatus(unsigned char ID,unsigned char status);
/*************************************************************
** 函数名称: DriverInfoAdd
** 功能描述: 追加驾驶员信息
** 入口参数: 追加驾驶员信息
** 出口参数: 返回存储的记录号(编号从1开始),0:表示信息有误或保存出错
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char DriverInfoAdd(unsigned char *index,unsigned char *drivercode,unsigned char *password,unsigned char *data,unsigned short datalen);
/*************************************************************
** 函数名称: GetDriverInfoAll
** 功能描述: 取得相应记录号的驾驶员信息
** 入口参数: recordNumber记录号,buffer返回数据的首地址,flag标志:0:表示详细信息(符合上报的结构),1:表示详细信息(字符串格式,显示屏显示用),2:表示驾驶员姓名、驾驶员代码
** 出口参数: 无
** 返回参数: 返回数据的长度,为0表示没有相应记录号的驾驶员信息
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned short GetDriverInfoAll(unsigned char *buffer,unsigned char flag);
/*************************************************************
** 函数名称: GetDriverInfoRecordNumber
** 功能描述: 取得相应记录号(从1开始编号)
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char GetDriverInfoRecordNumber(unsigned char *drivercode);
/*************************************************************
** 函数名称: GetDriverInfoRecordNumberForCheckData
** 功能描述: 检查驾驶员姓名及驾驶员身份证编码，取得相应记录号(从1开始编号)
** 入口参数: 符合上报的格式的数据
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char GetDriverInfoRecordNumberForCheckData(unsigned char *data);
/*************************************************************
** 函数名称: GetDriverInfo
** 功能描述: 取得相应记录号的驾驶员信息
** 入口参数: recordNumber记录号,buffer返回数据的首地址,flag标志:0:表示详细信息(符合上报的结构),1:表示详细信息(字符串格式,显示屏显示用),2:表示驾驶员姓名、驾驶员代码
** 出口参数: 无
** 返回参数: 返回数据的长度,为0表示没有相应记录号的驾驶员信息
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned short GetDriverInfo(unsigned char recordNumber,unsigned char *buffer,unsigned char flag);
/*************************************************************
** 函数名称: SaveCurrentDriverParameter
** 功能描述: 
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void SaveCurrentDriverParameter(unsigned char recordnumber);
/*************************************************************
** 函数名称: EventReport_DisposeRadioProtocol
** 功能描述: 事件报告操作协议解析
** 入口参数: pBuffer:数据首地址,BufferLen:数据长度
** 出口参数: 无
** 返回参数: 返回操作结果
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char EventReport_DisposeRadioProtocol(unsigned char *pBuffer,unsigned short BufferLen);
/*************************************************************
** 函数名称: InfoDemand_DisposeRadioProtocol
** 功能描述: 信息点播操作协议解析
** 入口参数: pBuffer:数据首地址,BufferLen:数据长度
** 出口参数: 无
** 返回参数: 返回操作结果
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char InfoDemand_DisposeRadioProtocol(unsigned char *pBuffer,unsigned short BufferLen);
/*************************************************************
** 函数名称: PhoneBook_DisposeRadioProtocol
** 功能描述: 电话簿操作协议解析
** 入口参数: pBuffer:数据首地址,BufferLen:数据长度
** 出口参数: 无
** 返回参数: 返回操作结果
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char PhoneBook_DisposeRadioProtocol(unsigned char *pBuffer,unsigned short BufferLen);
/*************************************************************
** 函数名称: DriverInfoModifyDriverCodeAndPassword
** 功能描述: 修改驾驶员信息工号/密码
** 入口参数: 修改驾驶员信息
** 出口参数: 
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char DriverInfoModifyDriverCodeAndPassword(unsigned char index ,unsigned char *drivercode,unsigned char *password);
/*************************************************************
** 函数名称: DriverInfoAddOrModify
** 功能描述: 追加/修改驾驶员信息
** 入口参数: 追加/修改驾驶员信息
** 出口参数: 返回存储的记录号(编号从1开始)
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char DriverInfoAddOrModify(unsigned char *index,unsigned char *drivercode,unsigned char *password,unsigned char *data,unsigned short datalen);
/*************************************************************
** 函数名称: DriverInfoDeleteAll
** 功能描述: 删除所有驾驶员信息
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void DriverInfoDeleteAll(void);
/*************************************************************
** 函数名称: CheckDriverInfoForLogin
** 功能描述: 
** 入口参数: 
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char CheckDriverInfoForLogin(unsigned char *drivercode,unsigned char *password,unsigned char* recordnum);
/*************************************************************
** 函数名称: DriverInfo_DisposeRadioProtocol
** 功能描述: 设置驾驶员协议解析
** 入口参数: pBuffer:数据首地址,BufferLen:数据长度
** 出口参数: 无
** 返回参数: 返回操作结果
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char DriverInfo_DisposeRadioProtocol(unsigned char *pBuffer,unsigned short BufferLen);
/*************************************************************
** 函数名称: DriverInfo_ReadDataForRadioProtocol
** 功能描述: 读取驾驶员数据协议解析
** 入口参数: pBuffer:数据首地址,
** 出口参数: 无
** 返回参数: 数据长度
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned short DriverInfo_ReadDataForRadioProtocol(unsigned char *pBuffer);
/*************************************************************
** 函数名称: GetDriverInfoTotalNumber
** 功能描述: 获取当前驾驶员总个数
** 入口参数: 
** 出口参数: 无
** 返回参数: 0表示无驾驶员,非零为驾驶员个数
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char GetDriverInfoTotalNumber(void);
/*************************************************************
** 函数名称: GetEventReportTotalNum
** 功能描述: 取得事件报告总条数
** 入口参数: 
** 出口参数: 无
** 返回参数: 总条数
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char GetEventReportTotalNum(void);
/*************************************************************
** 函数名称: GetOneEventReportDetails
** 功能描述: 取得相应编号的事件报告详情
** 入口参数: index编号(从0开始),
** 出口参数: stEvent事件报告结构体
** 返回参数: 1:成功,0:失败
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char GetOneEventReportDetails(unsigned char index,STMESSAGE *stEvent);
/*************************************************************
** 函数名称: GetInfoDemandTotalNum
** 功能描述: 取得信息点播总条数
** 入口参数: 
** 出口参数: 无
** 返回参数: 总条数
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char GetInfoDemandTotalNum(void);
/*************************************************************
** 函数名称: GetOneInfoDemandDetails
** 功能描述: 取得相应编号的信息点播详情
** 入口参数: index编号(从0开始),
** 出口参数: stInfo信息点播结构体
** 返回参数: 1:成功,0:失败
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char GetOneInfoDemandDetails(unsigned char index,STMESSAGE *stInfo);
/*************************************************************
** 函数名称: InfoService_SaveOneData
** 功能描述: 保存一条信息服务数据
** 入口参数: pBuffer:数据首地址,datalen数据长度
** 出口参数: 无
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void InfoService_SaveOneData(unsigned char *pBuffer,unsigned short datalen);
/*************************************************************
** 函数名称: InfoService_ReadOneData
** 功能描述: 读取指定编号的一条信息服务数据
** 入口参数: index编号从1开始(从新到旧),pBuffer:数据首地址,
** 出口参数: 无
** 返回参数: 数据长度
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned short InfoService_ReadOneData(unsigned char index,unsigned char *pBuffer);
/*************************************************************
** 函数名称: InfoService_GetTotalNum
** 功能描述: 取得信息服务总条数
** 入口参数: 
** 出口参数: 无
** 返回参数: 总条数
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char InfoService_GetTotalNum(void);
/*************************************************************
** 函数名称: QuestionResponse_SaveOneData
** 功能描述: 保存一条提问应答数据
** 入口参数: pBuffer:数据首地址,datalen数据长度
** 出口参数: 无
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void QuestionResponse_SaveOneData(unsigned char *pBuffer,unsigned short datalen);
/*************************************************************
** 函数名称: QuestionResponse_ReadOneData
** 功能描述: 读取指定编号的一条提问应答数据
** 入口参数: index编号从1开始(从新到旧),pBuffer:数据首地址,
** 出口参数: 无
** 返回参数: 数据长度
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned short QuestionResponse_ReadOneData(unsigned char index,unsigned char *pBuffer);
/*************************************************************
** 函数名称: QuestionResponse_GetTotalNum
** 功能描述: 取得提问应答总条数
** 入口参数: 
** 出口参数: 无
** 返回参数: 总条数
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char QuestionResponse_GetTotalNum(void);
/*************************************************************
** 函数名称: Message_GetPhoneBookTotalNum
** 功能描述: 取得电话簿总条数
** 入口参数: 
** 出口参数: 无
** 返回参数: 总条数
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Message_GetPhoneBookTotalNum(void);
/*************************************************************
** 函数名称: Message_GetOnePhoneBookContent
** 功能描述: 取得某条电话簿内容
** 入口参数: index索引号,stPhoneBook结构体指针
** 出口参数: 无
** 返回参数: 0,获取失败
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Message_GetOnePhoneBookContent(unsigned char index,STMSG_PHONEBOOK *stPhoneBook);
/*************************************************************
** 函数名称: DriverInfo_PlatformReadData
** 功能描述: 伊爱扩展协议——上传驾驶员信息
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void Messege_PlatformReadDriverInfoData(void);
/*************************************************************
** 函数名称: DriverInfo_DisposeEIExpandProtocol
** 功能描述: 伊爱扩展协议——设置驾驶员信息
** 入口参数: pBuffer:消息体内容首地址,BufferLen消息体内容长度
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Messege_DisposeEIExpandProtocolForDriverInfo(unsigned char *pBuffer, unsigned short BufferLen);
/*************************************************************
** 函数名称: PhoneBookDeleteAll
** 功能描述: 删除所有电话簿
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void PhoneBookDeleteAll(void);
/*************************************************************
** 函数名称: EventReportDeleteAll
** 功能描述: 删除所有事件报告
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void EventReportDeleteAll(void);
/*************************************************************
** 函数名称: InfoDemandDeleteAll
** 功能描述: 删除所有信息点播
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void InfoDemandDeleteAll(void);
/*************************************************************
** 函数名称: InfoServiceDeleteAll
** 功能描述: 删除所有信息服务内容
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void InfoServiceDeleteAll(void);
/*************************************************************
** 函数名称: QuestionResponseDeleteAll
** 功能描述: 删除所有提问应答信息
** 入口参数: 无
** 出口参数: 无
** 返回参数: 无
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void QuestionResponseDeleteAll(void);
#endif//__MESSAGE_H_
/*******************************************************************************
 *                             end of module
 *******************************************************************************/

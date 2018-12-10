/************************************************************************
//程序名称：Camera_App.c
//功能：该模块实现拍照功能
//版本号：V0.1
//版权说明：版权属于深圳市伊爱高新技术开发有限公司
//开发人：dxl
//开发时间：2014.11
//版本记录：版本记录需包含版本、修改人、修改时间、修改原因、修改简要说明
//V0.1：需要实现拍照功能，最多可支持4路摄像头，320*240和640*480两种分辨率
//拍照的方式有：平台下发指令拍照（可1张也可多张或无数张，可某1路摄像头，也可全部），
//事件触发拍照（一般是每路1张，也可多张，最多不超过3张），
//参数定时拍照，参数定距拍照。优先级从高到低分别为：平台下发指令拍照，事件触发拍照、
//定时参数拍照、定距参数拍照，只有在处理完高优先级的拍照请求后才会处理低优先级的拍照请求。
*************************************************************************/

/********************文件包含*************************/
#include <stdio.h>
#include <string.h>

#include "Camera_App.h"
#include "ff.h"
#include "modem_lib.h"
#include "modem_app_com.h"
#include "Public.h"
#include "fm25c160.h"
#include "GPIOControl.h"
#include "Io.h"
#include "RadioProtocol.h"
#include "Camera_Driver.h"
#include "Blind.h"

/********************本地变量*************************/
static u8 CameraTaskInitFlag = 0;
static u8 CameraState = 0;//当前摄像头状态
static u8 CameraSubState = 0;//当前摄像头子状态
static u8 CameraPhotoID = 1;//当前拍照ID，从1~CAMERA_ID_MAX
static u8 CameraFetchID = 1;//当前取图ID，从1~CAMERA_ID_MAX
static u8 CameraCheckID = 1;//当前自检ID，从1~CAMERA_ID_MAX
static u8 CameraPhotoInfo[CAMERA_ID_MAX*36] = {0};//8字节的图片信息+28字节的基本位置信息
static u8 CameraResolution[CAMERA_ID_MAX] = {0};//拍照分辨率，0表示320*240,1表示640*480
static u8 CameraPhotoType[CAMERA_ID_MAX] = {0};//拍照类型，0为定时（平台），1为事件
static u8 CameraEventType[CAMERA_ID_MAX] = {0};//多媒体事件类型
static u8 CameraStoreFlag[CAMERA_ID_MAX] = {0};//保存标志，0为不保存，1为保存
static u16 CameraTimeSpace[CAMERA_ID_MAX] = {0};//拍照时间间隔
static u8 CameraChannel[CAMERA_ID_MAX] = {0};//上传通道
static u16 CameraTotalPacket[CAMERA_ID_MAX] = {0};//图片总包数
static u16 CameraStateTimeCount[CAMERA_STATE_MAX+1] = {0};//状态计时
static u16 CameraPacket = 0;//取图包序号，从0开始
static FIL CameraFile;
static u8 CameraFileName[40];
static u32 StateCheckCount1 = 0;
static u32 StateCheckCount2 = 0;
/********************全局变量*************************/
CAMERA_STRUCT Camera[CAMERA_ID_MAX];
u8 CameraPowerFlag = 0;//摄像头供电标志，1为供电，0为关电
u8 CameraResendEnableFlag = 0;//重传（补传）使能标志，只有在上传完最后一包时才使能，补传完后禁止使能。
u8 CameraResendFlag = 0;//重传标志，1表示重传，0表示不是重传
u8 CameraStateAck[CAMERA_STATE_MAX+1] = {0};//状态标志，0为初始值，1为应答正确，2为无应答，3为应答错误，4为超时
u8 CameraStateErrorCount[CAMERA_STATE_MAX+1] = {0};//状态错误计数
u8 CameraDataBuffer[CAMERA_DATA_BUFFER_SIZE] = {0};//图像数据缓冲，存储的是摄像头图片数据，不包含摄像头协议的帧头，帧尾等内容
u16 CameraDataBufferLen;//图像数据长度
//u8 CameraDataBufferBusyFlag = 0;//忙标志，1忙，0空闲
u8 CameraCmdAckBuffer[CAMERA_CMD_BUFFER_SIZE] = {0};//摄像头命令应答缓冲，存储摄像头应答指令，但不包括取图的应答，包含帧头、帧尾内容
u8 CameraCmdAckBufferLen;//命令应答长度
u16 CameraResendPacketList[CAMERA_RESEND_LIST_SIZE] = {0};//重传包号列表
u16 CameraResendTotalPacket = 0;//重传总包数
u32 PhotoID = 1;//多媒体ID
/********************外部变量*************************/

/********************本地函数声明*********************/
static void Camera_StateSchedule(void);
static void Camera_StateCheck(void);
static void Camera_StatePhoto(void);
static void Camera_StateFetch(void);
static void Camera_StateStore(void);
static void Camera_StateSend(void);
static void Camera_StateResend(void);
static void Camera_StateSwitch(u8 State);

static void Camera_TaskInit(void);
static void Camera_ErrorHandle(void);
static u8 Camera_SchedulePhoto(void);
static u8 Camera_ScheduleFetch(void);
static void Camera_ScheduleCheck(void);
static void Camera_SavePhotoNumToFram(void);
static void Camera_WriteParameter(u8 Channel, u8 CameraID, u16 PhotoNum, u16 PhotoSpace, u8 Resolution, u8 StoreFlag, u8 PhotoType, u8 EventType);
/********************函数定义*************************/
/**************************************************************************
//函数名：Camera_TimeTask
//功能：实现拍照功能
//输入：无
//输出：无
//返回值：无
//备注：拍照定时任务，50ms调度1次，任务调度器需要调用此函数。
***************************************************************************/
FunctionalState Camera_TimeTask(void)
{

	if(0 == CameraTaskInitFlag)
	{
		CameraTaskInitFlag = 1;
		Camera_TaskInit();
		Camera_StateSwitch(CAMERA_STATE_CHECK);
	}
	
	switch(CameraState)
	{
		case CAMERA_STATE_SCHEDULE:
		{
			Camera_StateSchedule();
			break;
		}
		case CAMERA_STATE_CHECK:
		{
			Camera_StateCheck();
			break;
		}
		case CAMERA_STATE_PHOTO:
		{
			Camera_StatePhoto();
			break;
		}
		case CAMERA_STATE_FETCH:
		{
			Camera_StateFetch();
			break;
		}
		case CAMERA_STATE_STORE:
		{
			Camera_StateStore();
			break;
		}
		case CAMERA_STATE_SEND:
		{
			Camera_StateSend();
			break;
		}
		case CAMERA_STATE_RESEND:
		{
			Camera_StateResend();
			break;
		}
		default:
		{
			break;
		}
	}

	Camera_ErrorHandle();

	return ENABLE;
}
/**************************************************************************
//函数名：Camera_Photo
//功能：摄像头拍照
//输入：Channel:上传通道;只能是CHANNEL_DATA_1或者CHANNEL_DATA_2
//	CameraID:摄像头ID号;只能是0~CAMERA_ID_MAX，0表示全拍，其它表示单个拍
//	PhotoNum:拍照张数;当为0xfffe,0xffff表示一直拍照；当为0时表示停止拍照
//	PhotoSpace:拍照间隔;0值无效
//	Resolution:拍照分辨率;1为320*240分辨率，2为640*480分辨率，其它值无效
//	StoreFlag:存储标志;bit0置1表示需要上传，bit1置1表示需要保存，两个都置1表示既要上传也要保存，其它位保留
//	PhotoType:拍照类型;0表示定时拍照，1表示事件拍照，其它值无效
//	EventType:事件类型;0表示平台下发指令，1表示定时动作，2表示紧急报警，3表示碰撞或侧翻，当都不属于这几种时建议使用0
//输出：无
//返回值：0表示成功，1表示失败
//备注：当正在上传图片时又收到了新的拍照指令，则正在上传的图片会丢失，立即去执行新的拍照指令
***************************************************************************/
u8 Camera_Photo(u8 Channel, u8 CameraID, u16 PhotoNum, u16 PhotoSpace, u8 Resolution, u8 StoreFlag, u8 PhotoType, u8 EventType)
{
	u8 i;
	u8 Buffer[5];

	if(CameraID > CAMERA_ID_MAX)
	{
		return 1;
	}
	if(PhotoType > 1)
	{
		return 1;
	}

	if((Channel&CHANNEL_DATA_2) == CHANNEL_DATA_2)//需要对第2个连接拍照
	{
		if(1 == Blind_GetLink2OpenFlag())
		{

		}
		else
		{
			Channel = Channel & (~CHANNEL_DATA_2);
		}
	}
	

	if(((Channel&CHANNEL_DATA_1) == CHANNEL_DATA_1)||((Channel&CHANNEL_DATA_2) == CHANNEL_DATA_2))
	{
		if(0 == PhotoNum)
		{
			for(i=1; i<=CAMERA_ID_MAX; i++)
			{
				Camera[i-1].FetchFlag = 0;
				Camera[i-1].TimePhotoNum = 0;
				Camera[i-1].EventPhotoNum = 0;
				Public_ConvertLongToBuffer(Camera[i-1].TimePhotoNum,Buffer);
				FRAM_BufferWrite(FRAM_CAM_TIME_PHOTO_NUM_ADDR+(i-1)*(FRAM_CAM_TIME_PHOTO_NUM_LEN+1),Buffer+2,FRAM_CAM_TIME_PHOTO_NUM_LEN);//存储的后2字节
				Public_ConvertLongToBuffer(Camera[i-1].EventPhotoNum,Buffer);
				FRAM_BufferWrite(FRAM_CAM_EVENT_PHOTO_NUM_ADDR+(i-1)*(FRAM_CAM_EVENT_PHOTO_NUM_LEN+1),Buffer+2,FRAM_CAM_EVENT_PHOTO_NUM_LEN);//存储的后2字节
			}	
			for(i=0; i<=CAMERA_STATE_MAX; i++)
			{
				CameraStateAck[i] = 0;
				CameraStateErrorCount[i] = 0;
			}
			CameraState = 0;
			CameraSubState = 0;
			CameraPhotoID = 1;
			CameraFetchID = 1;
			CameraCheckID = 1;
			CameraPacket = 0;
			StateCheckCount1 = 0;
			StateCheckCount2 = 0;
			CameraResendEnableFlag = 0;
			CameraResendFlag = 0;
			CameraResendTotalPacket = 0;
			Camera_StateSwitch(CAMERA_STATE_SCHEDULE);
		
		}
		else
		{
			if(0 == CameraID)
			{
				
				for(i=1; i<=CAMERA_ID_MAX; i++)
				{
					Camera_WriteParameter(Channel,i,PhotoNum,PhotoSpace,Resolution,StoreFlag,PhotoType,EventType);
				}

			}
			else
			{
				Camera_WriteParameter(Channel,CameraID,PhotoNum,PhotoSpace,Resolution,StoreFlag,PhotoType,EventType);
			}
		}
	}
	else
	{
		return 1;
	}

	return 0;
}
/**************************************************************************
//函数名：Camera_MediaUploadAck
//功能：多媒体上传应答
//输入：pBuffer首地址指向重传包总数，然后是包列表
//输出：无
//返回值：无
//备注：当终端全部上传完所有包时，平台需下发多媒体上传应答命令0x8800，
//收到该命令后，需调用此函数
***************************************************************************/
void Camera_MediaUploadAck(u8 *pBuffer, u16 BufferLen)
{
	u8 i;
	u8 *p = pBuffer;
	u16 length;
	u16 temp;

	if((BufferLen > (2*CAMERA_RESEND_LIST_SIZE+1))||(0 == CameraResendEnableFlag))
	{
		return ;
	}

	length = BufferLen;
	
	CameraResendTotalPacket = *p++;
	length--;

	for(i=0; i<(length/2); i++)
	{
		temp = 0;
		temp |= *p++ << 8;
		temp |= *p++;
		CameraResendPacketList[i] = temp-1;//摄像头取图包序号是从0开始，上传的包序号是从1开始
	}
}
/**************************************************************************
//函数名：Camera_GetOnlineFlag
//功能：获取摄像头在线标志
//输入：无
//输出：无
//返回值：在线标志
//备注：bit7~bit4保留，bit3~bit0:分别表示1~4号摄像头在线标志，1在线，0不在线
***************************************************************************/
u8 Camera_GetOnlineFlag(void)
{
	u8 i;
	u8 temp;

	temp = 0;
	for(i=1; i<=CAMERA_ID_MAX; i++)
	{
		if(1 == Camera[i-1].OnOffFlag)
		{
			temp |= 1 << (i-1);
		}
	}
	
	return temp;	
}
/**************************************************************************
//函数名：Camera_GetTotalNum
//功能：获取总的在线摄像头个数
//输入：无
//输出：无
//返回值：总的在线摄像头个数
//备注：最多为CAMERA_ID_MAX个
***************************************************************************/
u8 Camera_GetTotalNum(void)
{
	u8 i;
	u8 temp;

	temp = 0;
	for(i=1; i<=CAMERA_ID_MAX; i++)
	{
		if(1 == Camera[i-1].OnOffFlag)
		{
			temp++;
		}
	}
	
	return temp;
}
/**************************************************************************
//函数名：Camera_TaskInit
//功能：初始化摄像头相关变量
//输入：无
//输出：无
//返回值：无
//备注：Camera_TimeTask需要调用此函数
***************************************************************************/
static void Camera_TaskInit(void)
{
	u8 i;
	u8 Buffer[5];
	u8 BufferLen;

	for(i=1; i<=CAMERA_ID_MAX; i++)
	{

		CameraResolution[i-1] = 0;
		CameraPhotoType[i-1] = 0;
		CameraEventType[i-1] = 0;
		CameraStoreFlag[i-1] = 0;
		CameraTimeSpace[i-1] = 0;
		CameraChannel[i-1] = 0;
		CameraTotalPacket[i-1] = 0;
		CameraStateTimeCount[i-1] = 0;

		Camera[i-1].OnOffFlag = 0;
		Camera[i-1].FetchFlag = 0;

		Camera[i-1].TimePhotoNum = 0;
		Camera[i-1].TimePhotoSpace = 0;
		Camera[i-1].TimePhotoStoreFlag = 0;
		Camera[i-1].TimePhotoResolution = 0;
		Camera[i-1].TimePhotoEventType = 0;
		Camera[i-1].TimePhotoChannel = 0;
		Camera[i-1].TimePhotoTime = 0;

		Camera[i-1].EventPhotoNum = 0;
		Camera[i-1].EventPhotoSpace = 0;
		Camera[i-1].EventPhotoStoreFlag = 0;
		Camera[i-1].EventPhotoResolution = 0;
		Camera[i-1].EventPhotoEventType = 0;
		Camera[i-1].EventPhotoChannel = 0;
		Camera[i-1].EventPhotoTime = 0;

		BufferLen = FRAM_BufferRead(Buffer,FRAM_CAM_TIME_PHOTO_NUM_LEN,FRAM_CAM_TIME_PHOTO_NUM_ADDR+(i-1)*(FRAM_CAM_TIME_PHOTO_NUM_LEN+1));
		if(FRAM_CAM_TIME_PHOTO_NUM_LEN == BufferLen)
		{
			Camera[i-1].TimePhotoNum = Public_ConvertBufferToShort(Buffer);
		}

		BufferLen = FRAM_BufferRead(Buffer,FRAM_CAM_TIME_PHOTO_SPACE_LEN,FRAM_CAM_TIME_PHOTO_SPACE_ADDR+(i-1)*(FRAM_CAM_TIME_PHOTO_SPACE_LEN+1));
		if(FRAM_CAM_TIME_PHOTO_SPACE_LEN == BufferLen)
		{
			Camera[i-1].TimePhotoSpace = Public_ConvertBufferToShort(Buffer);
		}

		BufferLen = FRAM_BufferRead(Buffer,FRAM_CAM_TIME_PHOTO_STORE_FLAG_LEN,FRAM_CAM_TIME_PHOTO_STORE_FLAG_ADDR+(i-1)*(FRAM_CAM_TIME_PHOTO_STORE_FLAG_LEN+1));
		if(FRAM_CAM_TIME_PHOTO_STORE_FLAG_LEN == BufferLen)
		{
			Camera[i-1].TimePhotoStoreFlag = Buffer[0];
		}

		BufferLen = FRAM_BufferRead(Buffer,FRAM_CAM_TIME_PHOTO_RESOLUTION_LEN,FRAM_CAM_TIME_PHOTO_RESOLUTION_ADDR+(i-1)*(FRAM_CAM_TIME_PHOTO_RESOLUTION_LEN+1));
		if(FRAM_CAM_TIME_PHOTO_RESOLUTION_LEN == BufferLen)
		{
			Camera[i-1].TimePhotoResolution = Buffer[0];
		}

		BufferLen = FRAM_BufferRead(Buffer,FRAM_CAM_TIME_PHOTO_EVENT_TYPE_LEN,FRAM_CAM_TIME_PHOTO_EVENT_TYPE_ADDR+(i-1)*(FRAM_CAM_TIME_PHOTO_EVENT_TYPE_LEN+1));
		if(FRAM_CAM_TIME_PHOTO_EVENT_TYPE_LEN == BufferLen)
		{
			Camera[i-1].TimePhotoEventType = Buffer[0];
		}

		BufferLen = FRAM_BufferRead(Buffer,FRAM_CAM_TIME_PHOTO_CHANNEL_LEN,FRAM_CAM_TIME_PHOTO_CHANNEL_ADDR+(i-1)*(FRAM_CAM_TIME_PHOTO_CHANNEL_LEN+1));
		if(FRAM_CAM_TIME_PHOTO_CHANNEL_LEN == BufferLen)
		{
			Camera[i-1].TimePhotoChannel = Buffer[0];
		}

		//////////////////////////////////////////
		BufferLen = FRAM_BufferRead(Buffer,FRAM_CAM_EVENT_PHOTO_NUM_LEN,FRAM_CAM_EVENT_PHOTO_NUM_ADDR+(i-1)*(FRAM_CAM_EVENT_PHOTO_NUM_LEN+1));
		if(FRAM_CAM_EVENT_PHOTO_NUM_LEN == BufferLen)
		{
			Camera[i-1].EventPhotoNum = Public_ConvertBufferToShort(Buffer);
		}

		BufferLen = FRAM_BufferRead(Buffer,FRAM_CAM_EVENT_PHOTO_SPACE_LEN,FRAM_CAM_EVENT_PHOTO_SPACE_ADDR+(i-1)*(FRAM_CAM_EVENT_PHOTO_SPACE_LEN+1));
		if(FRAM_CAM_EVENT_PHOTO_SPACE_LEN == BufferLen)
		{
			Camera[i-1].EventPhotoSpace = Public_ConvertBufferToShort(Buffer);
		}

		BufferLen = FRAM_BufferRead(Buffer,FRAM_CAM_EVENT_PHOTO_STORE_FLAG_LEN,FRAM_CAM_EVENT_PHOTO_STORE_FLAG_ADDR+(i-1)*(FRAM_CAM_EVENT_PHOTO_STORE_FLAG_LEN+1));
		if(FRAM_CAM_EVENT_PHOTO_STORE_FLAG_LEN == BufferLen)
		{
			Camera[i-1].EventPhotoStoreFlag = Buffer[0];
		}

		BufferLen = FRAM_BufferRead(Buffer,FRAM_CAM_EVENT_PHOTO_RESOLUTION_LEN,FRAM_CAM_EVENT_PHOTO_RESOLUTION_ADDR+(i-1)*(FRAM_CAM_EVENT_PHOTO_RESOLUTION_LEN+1));
		if(FRAM_CAM_EVENT_PHOTO_RESOLUTION_LEN == BufferLen)
		{
			Camera[i-1].EventPhotoResolution = Buffer[0];
		}

		BufferLen = FRAM_BufferRead(Buffer,FRAM_CAM_EVENT_PHOTO_EVENT_TYPE_LEN,FRAM_CAM_EVENT_PHOTO_EVENT_TYPE_ADDR+(i-1)*(FRAM_CAM_EVENT_PHOTO_EVENT_TYPE_LEN+1));
		if(FRAM_CAM_EVENT_PHOTO_EVENT_TYPE_LEN == BufferLen)
		{
			Camera[i-1].EventPhotoEventType = Buffer[0];
		}

		BufferLen = FRAM_BufferRead(Buffer,FRAM_CAM_EVENT_PHOTO_CHANNEL_LEN,FRAM_CAM_EVENT_PHOTO_CHANNEL_ADDR+(i-1)*(FRAM_CAM_EVENT_PHOTO_CHANNEL_LEN+1));
		if(FRAM_CAM_EVENT_PHOTO_CHANNEL_LEN == BufferLen)
		{
			Camera[i-1].EventPhotoChannel = Buffer[0];
		}
	}

	for(i=0; i<=CAMERA_STATE_MAX; i++)
	{
		CameraStateAck[i] = 0;
		CameraStateErrorCount[i] = 0;
	}

	BufferLen = FRAM_BufferRead(Buffer,FRAM_PHOTO_ID_LEN,FRAM_PHOTO_ID_ADDR);
	if(FRAM_PHOTO_ID_LEN == BufferLen)
	{
		PhotoID = Public_ConvertBufferToLong(Buffer);
	}

	CameraState = 0;
	CameraSubState = 0;
	CameraPhotoID = 1;
	CameraFetchID = 1;
	CameraCheckID = 1;
	CameraPacket = 0;
	StateCheckCount1 = 0;
	StateCheckCount2 = 0;
	CameraResendEnableFlag = 0;
	CameraResendFlag = 0;
	CameraResendTotalPacket = 0;
}

/**************************************************************************
//函数名：Camera_StateSwitch
//功能：实现摄像头状态切换功能
//输入：无
//输出：无
//返回值：无
//备注：Camera_StateSchedule需要调用此函数，超时时Camera_StateCheck
//状态切换规则：只能从其它状态切换到调度状态，或从调度状态切换到其它状态
***************************************************************************/
static void Camera_StateSwitch(u8 State)
{
	
	if((State > CAMERA_STATE_MAX)||(CameraState > CAMERA_STATE_MAX))
	{
		return ;
	}

	if(CAMERA_STATE_SCHEDULE == State)
	{
		if(CameraStateAck[CameraState] > 1)//出错
		{
			CameraStateErrorCount[CameraState]++;
			if(CameraStateErrorCount[CameraState] > 254)
			{
				CameraStateErrorCount[CameraState] = 254;
			}
		}
		else
		{
			CameraStateErrorCount[CameraState] = 0;
		}
		CameraState = State;
		CameraSubState = 0;
	}
	else
	{
		if(CAMERA_STATE_SCHEDULE == CameraState)
		{
			CameraState = State;
			CameraSubState = 0;
			CameraStateTimeCount[State] = 0;
			CameraStateAck[State] = 0;
		}
	}
}
/**************************************************************************
//函数名：Camera_ErrorHandle
//功能：摄像头错误处理
//输入：无
//输出：无
//返回值：无
//备注：摄像头拍照任务Camera_TimeTask需要调用此函数
***************************************************************************/
static void Camera_ErrorHandle(void)
{
	u8 flag;
	u8 i,j;
	u8 Buffer[5] = {0};

	flag = 0;

	if((CameraPhotoID > CAMERA_ID_MAX)||(0 == CameraPhotoID))//检测ID是否正常
	{
		flag = 1;
	}
	if((CameraFetchID > CAMERA_ID_MAX)||(0 == CameraFetchID))
	{
		flag = 1;
	}
	if((CameraCheckID > CAMERA_ID_MAX)||(0 == CameraCheckID))
	{
		flag = 1;
	}

	if( 0 == CameraResendFlag)//检测包序号
	{
		if(CameraPacket > CameraTotalPacket[CameraFetchID-1])
		{
			flag = 1;
		}
	}
	else
	{
		if(CameraPacket > CameraResendTotalPacket)
		{
			flag = 1;
		}
	}

	for(i=1; i<=CAMERA_ID_MAX; i++)
	{
		if(Camera[i-1].TimePhotoNum < 0)
		{
			Camera[i-1].TimePhotoNum = 0;
			for(j=0; j<4; j++)
			{
				Buffer[j] = 0;
			}
			FRAM_BufferWrite(FRAM_CAM_TIME_PHOTO_NUM_ADDR+(i-1)*(FRAM_CAM_TIME_PHOTO_NUM_LEN+1),Buffer,FRAM_CAM_TIME_PHOTO_NUM_LEN);
		}
		if(Camera[i-1].EventPhotoNum < 0)
		{
			Camera[i-1].EventPhotoNum = 0;
			for(j=0; j<4; j++)
			{
				Buffer[j] = 0;
			}
			FRAM_BufferWrite(FRAM_CAM_EVENT_PHOTO_NUM_ADDR+(i-1)*(FRAM_CAM_EVENT_PHOTO_NUM_LEN+1),Buffer,FRAM_CAM_EVENT_PHOTO_NUM_LEN);	
		}
	}

	if((CameraStateErrorCount[CAMERA_STATE_PHOTO] > 3)||(CameraStateErrorCount[CAMERA_STATE_FETCH] > 3))//检查各状态是否正常
	{
		flag = 1;
	}

	if(1 == flag)
	{
		CameraTaskInitFlag = 1;
	}

	
}
/**************************************************************************
//函数名：Camera_StateSchedule
//功能：实现摄像头拍照各状态的调度（状态切换）
//输入：无
//输出：无
//返回值：无
//备注：摄像头拍照任务Camera_TimeTask需要调用此函数
***************************************************************************/
static void Camera_StateSchedule(void)
{

	if( 1 == Camera_SchedulePhoto())//调度拍照，只包括拍照状态
	{
		StateCheckCount1 = 0;
		StateCheckCount2 = 0;
		return;
	}

	if(1 == Camera_ScheduleFetch())//调度取图，包括取图、存储、发送、补传几个状态
	{
		StateCheckCount1 = 0;
		StateCheckCount2 = 0;
		return;
	}

	Camera_ScheduleCheck();//自检控制，省电控制
}
/**************************************************************************
//函数名：Camera_SchedulePhoto
//功能：检测是否有拍照需求
//输入：无
//输出：无
//返回值：1表示有拍照需求，0表示没有
//备注：函数Camera_StateSchedule需要调用此函数
***************************************************************************/
static u8 Camera_SchedulePhoto(void)
{
	u8 i;
	u32 time;

	time = RTC_GetCounter();

	for(i=1; i<=CAMERA_ID_MAX; i++)//检查有无事件类拍照需求
	{
		if((1 == Camera[i-1].OnOffFlag)&&(0 == Camera[i-1].FetchFlag)&&(Camera[i-1].EventPhotoNum > 0))
		{
			if((0x01 == (0x01&Camera[i-1].EventPhotoStoreFlag))&&(CameraChannel[i-1] == (CameraChannel[i-1]&GetTerminalAuthorizationFlag())))//需要上传就判断通信是否在线
			{
				if(time >= Camera[i-1].EventPhotoTime)
				{
					CameraPhotoID = i;

					CameraResolution[i-1] = Camera[i-1].EventPhotoResolution;
					CameraEventType[i-1] = Camera[i-1].EventPhotoEventType;
					CameraTimeSpace[i-1] = Camera[i-1].EventPhotoSpace;
					CameraChannel[i-1] = Camera[i-1].EventPhotoChannel;
					CameraStoreFlag[i-1] = Camera[i-1].EventPhotoStoreFlag;
					CameraPhotoType[i-1] = 1;

					Camera_StateSwitch(CAMERA_STATE_PHOTO);

					return 1;
				}
			}

			if((0x02 == (0x02&Camera[i-1].EventPhotoStoreFlag))&&(1 == Media_GetSdOnLineFlag()))//需要保存就判断SD卡是否在线
			{
				if(time >= Camera[i-1].EventPhotoTime)
				{
					CameraPhotoID = i;

					CameraResolution[i-1] = Camera[i-1].EventPhotoResolution;
					CameraEventType[i-1] = Camera[i-1].EventPhotoEventType;
					CameraTimeSpace[i-1] = Camera[i-1].EventPhotoSpace;
					CameraChannel[i-1] = Camera[i-1].EventPhotoChannel;
					CameraStoreFlag[i-1] = Camera[i-1].EventPhotoStoreFlag;
					CameraPhotoType[i-1] = 1;

					Camera_StateSwitch(CAMERA_STATE_PHOTO);

					return 1;
				}
			}	
		}
	}

	for(i=1; i<=CAMERA_ID_MAX; i++)//检查有无平台定时拍照需求
	{
		if((1 == Camera[i-1].OnOffFlag)&&(0 == Camera[i-1].FetchFlag)&&(Camera[i-1].TimePhotoNum > 0))
		{
			if((0x01 == (0x01&Camera[i-1].TimePhotoStoreFlag))&&(CameraChannel[i-1] == (CameraChannel[i-1]&GetTerminalAuthorizationFlag())))//需要上传就判断通信是否在线
			{
				if(time >= Camera[i-1].TimePhotoTime)
				{
					CameraPhotoID = i;

					CameraResolution[i-1] = Camera[i-1].TimePhotoResolution;
					CameraEventType[i-1] = Camera[i-1].TimePhotoEventType;
					CameraTimeSpace[i-1] = Camera[i-1].TimePhotoSpace;
					CameraChannel[i-1] = Camera[i-1].TimePhotoChannel;
					CameraStoreFlag[i-1] = Camera[i-1].TimePhotoStoreFlag;
					CameraPhotoType[i-1] = 0;

					Camera_StateSwitch(CAMERA_STATE_PHOTO);

					return 1;
				}
			}

			if((0x02 == (0x02&Camera[i-1].TimePhotoStoreFlag))&&(1 == Media_GetSdOnLineFlag()))//需要保存就判断SD卡是否在线
			{
				if(time >= Camera[i-1].TimePhotoTime)
				{
					CameraPhotoID = i;

					CameraResolution[i-1] = Camera[i-1].TimePhotoResolution;
					CameraEventType[i-1] = Camera[i-1].TimePhotoEventType;
					CameraTimeSpace[i-1] = Camera[i-1].TimePhotoSpace;
					CameraChannel[i-1] = Camera[i-1].TimePhotoChannel;
					CameraStoreFlag[i-1] = Camera[i-1].TimePhotoStoreFlag;
					CameraPhotoType[i-1] = 0;

					Camera_StateSwitch(CAMERA_STATE_PHOTO);

					return 1;
				}
			}	
		}
	}


	return 0;
}
/**************************************************************************
//函数名：Camera_ScheduleFetch
//功能：检测是否有取图需求
//输入：无
//输出：无
//返回值：1表示有取图需求，0表示没有
//备注：函数Camera_StateSchedule需要调用此函数
***************************************************************************/
static u8 Camera_ScheduleFetch(void)
{
	u8 i,j;

	if((1 == Camera[CameraFetchID-1].OnOffFlag)&&(Camera[CameraFetchID-1].FetchFlag >= CAMERA_STATE_FETCH))//检查是否已有正在取图的
	{
		if(Camera[CameraFetchID-1].FetchFlag >= CAMERA_STATE_RESEND)
		{
			CameraPacket = 0;
			CameraResendEnableFlag = 0;
			if(CameraResendTotalPacket > 0)
			{
				CameraResendFlag = 1;
				Camera_StateSwitch(CAMERA_STATE_FETCH);
				return 1;
			}
			else
			{
				CameraResendFlag = 0;
				Camera[CameraFetchID-1].FetchFlag = 0;
				Camera_SavePhotoNumToFram();
			}
		}
		else if(Camera[CameraFetchID-1].FetchFlag == CAMERA_STATE_SEND)
		{
			if(0 == CameraResendFlag)//正常的上传
			{
				if(CameraPacket >= (CameraTotalPacket[CameraFetchID-1]-1))//最后一包
				{
					CameraResendTotalPacket = 0;
					for(i=0; i<CAMERA_RESEND_LIST_SIZE; i++)
					{
						CameraResendPacketList[i] = 0;
					}
					CameraResendEnableFlag = 1;
					Camera_StateSwitch(CAMERA_STATE_RESEND);
					return 1;
				
				}
				else
				{
					CameraPacket++;
					Camera_StateSwitch(CAMERA_STATE_FETCH);
					return 1;
				}
					
			}
			else//重传
			{
				if(CameraPacket >= (CameraResendTotalPacket-1))//最后一包
				{
					CameraResendFlag = 0;
					Camera[CameraFetchID-1].FetchFlag = 0;
					Camera_SavePhotoNumToFram();
				
				}
				else
				{
					CameraPacket++;
					Camera_StateSwitch(CAMERA_STATE_FETCH);
					return 1;
				}	
			}
			
		}
		else
		{
			Camera_StateSwitch(Camera[CameraFetchID-1].FetchFlag+1);
			return 1;
		}

			
	}

	j = CameraFetchID;//若其它有取图的标志，需要清0
	if((j > CAMERA_ID_MAX)||(0 == j))
	{
		j = 1;
	}
	for(i = 0; i<=CAMERA_ID_MAX; i++)//检查是否有取图请求
	{
		if((1 == Camera[j-1].OnOffFlag)&&(Camera[j-1].FetchFlag >= CAMERA_STATE_FETCH))
		{
			Camera[j-1].FetchFlag = 0;	
		}
		j++;
		if(j > CAMERA_ID_MAX)
		{
			j = 1;
		}
	}


	j = CameraFetchID+1;
	if((j > CAMERA_ID_MAX)||(0 == j))
	{
		j = 1;
	}
	for(i = 0; i<=CAMERA_ID_MAX; i++)//检查是否有取图请求
	{
		if((1 == Camera[j-1].OnOffFlag)&&(Camera[j-1].FetchFlag == CAMERA_STATE_PHOTO))
		{
			Camera_StateSwitch(CAMERA_STATE_FETCH);
			CameraFetchID = j;
			CameraPacket = 0;
			return 1;	
		}
		j++;
		if(j > CAMERA_ID_MAX)
		{
			j = 1;
		}
	}


	return 0;
}
/**************************************************************************
//函数名：Camera_ScheduleCheck
//功能：检测是否有自检需求
//输入：无
//输出：无
//返回值：1表示有自检需求，0表示没有
//备注：函数Camera_StateSchedule需要调用此函数
***************************************************************************/
static void Camera_ScheduleCheck(void)
{
	u8 acc;

	acc = Io_ReadStatusBit(STATUS_BIT_ACC);

	if(1 == acc)//ACC开时，没有拍照处理时每隔60秒检测一次摄像头的在线状态
	{
		StateCheckCount1++;
		if(StateCheckCount1 >= (20*60))
		{
			StateCheckCount1 = 0;
			CameraCheckID = 1;
			Camera_StateSwitch(CAMERA_STATE_CHECK);
		}
	}
	else//ACC OFF时节电处理:当无取图请求持续10秒将关闭摄像头电源，ACC OFF时不自检摄像头
	{
		
		StateCheckCount2++;
		if(StateCheckCount2 >= (20*10))
		{
			StateCheckCount2 = 0;
			CAMERA_POWER_OFF();//关电
		}		
	}

	return ;
}
/**************************************************************************
//函数名：Camera_SavePhotoNumToFram
//功能：保存剩余拍照张数到铁电
//输入：无
//输出：无
//返回值：无
//备注：函数Camera_ScheduleFetch需要调用此函数，每调用1次剩余张数自动减1
***************************************************************************/
static void Camera_SavePhotoNumToFram(void)
{
	u8 Buffer[5];

	if(0 == CameraPhotoType[CameraFetchID-1])
	{
		if(Camera[CameraFetchID-1].TimePhotoNum >= 0xfffe)//一直拍
		{

		}
		else if(Camera[CameraFetchID-1].TimePhotoNum > 0)
		{
			if(0x01 == (0x01&CameraStoreFlag[CameraFetchID-1]))
			{
				if(1 == CameraStateAck[CAMERA_STATE_SEND])
				{
					Camera[CameraFetchID-1].TimePhotoNum--;
				}
			}
			else if(0x02 == (0x02&CameraStoreFlag[CameraFetchID-1]))
			{
				if(1 == CameraStateAck[CAMERA_STATE_STORE])
				{
					Camera[CameraFetchID-1].TimePhotoNum--;
				}
			}		
		}
		else
		{
			Camera[CameraFetchID-1].TimePhotoNum = 0;
		}
		Public_ConvertLongToBuffer(Camera[CameraFetchID-1].TimePhotoNum,Buffer);
		FRAM_BufferWrite(FRAM_CAM_TIME_PHOTO_NUM_ADDR+(CameraFetchID-1)*(FRAM_CAM_TIME_PHOTO_NUM_LEN+1),Buffer+2,FRAM_CAM_TIME_PHOTO_NUM_LEN);//存储的后2字节
	}
	else
	{
		if(Camera[CameraFetchID-1].EventPhotoNum > 0)
		{
			if(0x01 == (0x01&CameraStoreFlag[CameraFetchID-1]))
			{
				if(1 == CameraStateAck[CAMERA_STATE_SEND])
				{
					Camera[CameraFetchID-1].EventPhotoNum--;
				}
			}
			else if(0x02 == (0x02&CameraStoreFlag[CameraFetchID-1]))
			{
				if(1 == CameraStateAck[CAMERA_STATE_STORE])
				{
					Camera[CameraFetchID-1].EventPhotoNum--;
				}
			}
		}
		else
		{
			Camera[CameraFetchID-1].EventPhotoNum = 0;
		}
		Public_ConvertLongToBuffer(Camera[CameraFetchID-1].EventPhotoNum,Buffer);
		FRAM_BufferWrite(FRAM_CAM_EVENT_PHOTO_NUM_ADDR+(CameraFetchID-1)*(FRAM_CAM_EVENT_PHOTO_NUM_LEN+1),Buffer+2,FRAM_CAM_EVENT_PHOTO_NUM_LEN);//存储的后2字节
	}
}
/**************************************************************************
//函数名：Camera_StateCheck
//功能：实现摄像头自检状态功能
//输入：无
//输出：无
//返回值：无
//备注：摄像头拍照任务Camera_TimeTask需要调用此函数
//依次发送自检命令给各路摄像头，若无应答则OnOffFlag为0，有应答OnOffFlag为1
***************************************************************************/
static void Camera_StateCheck(void)
{
	u8 Buffer[20] = {0x40,0x40,0x69,0xff,0xff,0x0d,0x0a};//摄像头自检命令
	u8 length = 7;
	u8 i;
	//u8 Buffer[20] = {0x40,0x40,0x60,0x5a,0xff,0xff,0x01,0x00,0x0d,0x0a};//设置摄像头的ID为1
	//u8 Buffer[20] = {0x40,0x40,0x60,0x5a,0xff,0xff,0x02,0x00,0x0d,0x0a};//设置摄像头的ID为2
	//u8 Buffer[20] = {0x40,0x40,0x60,0x5a,0xff,0xff,0x03,0x00,0x0d,0x0a};//设置摄像头的ID为3
	//u8 Buffer[20] = {0x40,0x40,0x60,0x5a,0xff,0xff,0x04,0x00,0x0d,0x0a};//设置摄像头的ID为4
	//length = 10;

	static u16 TimeCount = 0;



	CameraStateTimeCount[CAMERA_STATE_CHECK]++;
	if(CameraStateTimeCount[CAMERA_STATE_CHECK] > CAMERA_STATE_CHECK_OVERTIME)//超时
	{
		CameraStateAck[CAMERA_STATE_CHECK] = 4;
		CameraCheckID = 1;
		Camera_StateSwitch(CAMERA_STATE_SCHEDULE);
	}

	if(0 == CameraSubState)//上电
	{
		if(0 == CameraPowerFlag)
		{
			CAMERA_POWER_ON();//上电
			CameraSubState = 1;//上电后需等待一段时间
		}
		else
		{
			CameraSubState = 2;//跳过等待状态
			TimeCount = 0;
		}
	}
	else if(1 == CameraSubState)//上电延时
	{
		if(CameraStateTimeCount[CAMERA_STATE_CHECK] > (CAMERA_STATE_CHECK_OVERTIME/2))
		{
			CameraSubState = 2;
			TimeCount = 0;
		}
	}
	else if(2 == CameraSubState)//发送指令
	{
		Buffer[3] = CameraCheckID;
		Buffer[4] = 0;
		if(ACK_OK == Usart3_SendData(Buffer,length))
		{
			CameraSubState = 3;
		}
	}
	else if(3 == CameraSubState)//等待应答
	{
		TimeCount++;
		if(1 == CameraStateAck[CAMERA_STATE_CHECK])//有应答
		{
			Camera[CameraCheckID-1].OnOffFlag = 1;
			CameraCheckID++;//下一个摄像头
			if(CameraCheckID > CAMERA_ID_MAX)
			{
				for(i=1; i<=CAMERA_ID_MAX; i++)
				{
					if(1 == Camera[i-1].OnOffFlag)
					{
						break;
					}
				}
				if(i > CAMERA_ID_MAX)
				{
					Io_WriteAlarmBit(ALARM_BIT_CAMERA_FAULT,SET);
				}
				else
				{
					Io_WriteAlarmBit(ALARM_BIT_CAMERA_FAULT,RESET);
				}
				CameraCheckID = 1;
				Camera_StateSwitch(CAMERA_STATE_SCHEDULE);
			}
			CameraStateAck[CAMERA_STATE_CHECK] = 0;
			CameraSubState = 0;
			TimeCount = 0;
		}
		else if(2 == CameraStateAck[CAMERA_STATE_CHECK])//无应答
		{
			if(TimeCount > 20)
			{
				Camera[CameraCheckID-1].OnOffFlag = 0;
				CameraCheckID++;//下一个摄像头
				if(CameraCheckID > CAMERA_ID_MAX)
				{
					for(i=1; i<=CAMERA_ID_MAX; i++)
					{
						if(1 == Camera[i-1].OnOffFlag)
						{
							break;
						}
					}
					if(i > CAMERA_ID_MAX)
					{
						Io_WriteAlarmBit(ALARM_BIT_CAMERA_FAULT,SET);
					}
					else
					{
						Io_WriteAlarmBit(ALARM_BIT_CAMERA_FAULT,RESET);
					}
					CameraCheckID = 1;
					Camera_StateSwitch(CAMERA_STATE_SCHEDULE);
				}
				CameraSubState = 0;
				TimeCount = 0;
			}
		}
		else//检测超时
		{
			if(TimeCount > 20)
			{
				CameraStateAck[CAMERA_STATE_CHECK] = 2;//超时1秒，重发1次
				CameraSubState = 2;
				TimeCount = 0;
			}
		}
	}

	
}
/**************************************************************************
//函数名：Camera_StatePhoto
//功能：实现摄像头拍照状态功能
//输入：无
//输出：无
//返回值：无
//备注：摄像头拍照任务Camera_TimeTask需要调用此函数
***************************************************************************/
static void Camera_StatePhoto(void)
{
	u8 Buffer[20] = {0x40,0x40,0x61,0x81,0x00,0x00,0x00,0x02,0x0d,0x0a};//摄像头拍照命令
	u8 length = 10;
	u8 *p = NULL;
	u8 PhotoIDBuffer[5] = {0};

	static u16 TimeCount = 0;

	
	CameraStateTimeCount[CAMERA_STATE_PHOTO]++;
	if(CameraStateTimeCount[CAMERA_STATE_PHOTO] > CAMERA_STATE_PHOTO_OVERTIME)//超时
	{
		CameraStateAck[CAMERA_STATE_PHOTO] = 4;
		Camera_StateSwitch(CAMERA_STATE_SCHEDULE);
	}

	if(0 == CameraSubState)//上电
	{
		if(0 == CameraPowerFlag)
		{
			CAMERA_POWER_ON();//上电
			CameraSubState = 1;//上电后需等待一段时间
		}
		else
		{
			CameraSubState++;//跳过等待状态
			CameraSubState++;
			TimeCount = 0;
		}
	}
	else if(1 == CameraSubState)//上电延时
	{
		if(CameraStateTimeCount[CAMERA_STATE_PHOTO] > (CAMERA_STATE_CHECK_OVERTIME/2))
		{
			CameraSubState++;
			TimeCount = 0;
		}
	}
	else if(2 == CameraSubState)//发送指令
	{
		if(1 == CameraResolution[CameraPhotoID-1])
		{
			Buffer[3] = 0x81;
		}
		else if(2 == CameraResolution[CameraPhotoID-1])
		{
			Buffer[3] = 0x82;
		}
		else
		{
			Buffer[3] = 0x81;
		}
		Buffer[4] = CameraPhotoID;
		Buffer[5] = 0;
		if(ACK_OK == Usart3_SendData(Buffer,length))
		{
			CameraStateAck[CAMERA_STATE_PHOTO] = 0;
			CameraSubState++;
		}
	}
	else if(3 == CameraSubState)//等待应答
	{
		TimeCount++;
		if(1 == CameraStateAck[CAMERA_STATE_PHOTO])//有应答
		{
			CameraTotalPacket[CameraPhotoID-1] = (CameraCmdAckBuffer[6] << 8)|CameraCmdAckBuffer[5];
			PhotoID++;//ID加1
			Public_ConvertLongToBuffer(PhotoID,PhotoIDBuffer);
			FRAM_BufferWrite(FRAM_PHOTO_ID_ADDR,PhotoIDBuffer,FRAM_PHOTO_ID_LEN);
			p = CameraPhotoInfo+(CameraPhotoID-1)*36;
			*p++ = (PhotoID&0xff000000) >> 24;//多媒体ID
			*p++ = (PhotoID&0xff0000) >> 16;
			*p++ = (PhotoID&0xff00) >> 8;
			*p++ = PhotoID&0xff;
			*p++ = 0;//多媒体类型，0为图像
			*p++ = 0;//多媒体格式，0为JPEG
			*p++ = CameraEventType[CameraPhotoID-1];//多媒体事件
			*p++ = CameraPhotoID;//多媒体通道
			Report_GetPositionBasicInfo(p);

			Camera[CameraPhotoID-1].FetchFlag = CAMERA_STATE_PHOTO;
			if(0 == CameraPhotoType[CameraPhotoID-1])
			{
				Camera[CameraPhotoID-1].TimePhotoTime = RTC_GetCounter()+CameraTimeSpace[CameraPhotoID-1];
			}
			else
			{
				Camera[CameraPhotoID-1].EventPhotoTime = RTC_GetCounter()+CameraTimeSpace[CameraPhotoID-1];
			}

			TimeCount = 0;

			Camera_StateSwitch(CAMERA_STATE_SCHEDULE);
		}
		else if(3 == CameraStateAck[CAMERA_STATE_PHOTO])//应答出错
		{
			if(TimeCount > 20)
			{
				CameraSubState--;
				TimeCount = 0;
			}
		}
		else//超时检测
		{
			if(TimeCount > 20)
			{
				CameraSubState--;
				TimeCount = 0;
			}
		}
	}
}
/**************************************************************************
//函数名：Camera_StateFetch
//功能：实现摄像头取图状态功能
//输入：无
//输出：无
//返回值：无
//备注：摄像头拍照任务Camera_TimeTask需要调用此函数
***************************************************************************/
static void Camera_StateFetch(void)
{
	u8 Buffer[20] = {0x40,0x40,0x62,0x81,0x00,0x00,0x00,0x00,0x0d,0x0a};//摄像头取图命令
	u8 length = 10;
	static u16 TimeCount = 0;
	
	Camera[CameraFetchID-1].FetchFlag = CAMERA_STATE_FETCH;
	
	CameraStateTimeCount[CAMERA_STATE_FETCH]++;
	if(CameraStateTimeCount[CAMERA_STATE_FETCH] > CAMERA_STATE_FETCH_OVERTIME)//超时
	{
		CameraStateAck[CAMERA_STATE_FETCH] = 4;
		Camera_StateSwitch(CAMERA_STATE_SCHEDULE);
	}

	if(0 == CameraSubState)//发送指令
	{
		
		if(0 == CameraResolution[CameraFetchID-1])
		{
			Buffer[3] = 0x81;
		}
		else if(1 == CameraResolution[CameraFetchID-1])
		{
			Buffer[3] = 0x82;
		}
		else
		{
			Buffer[3] = 0x81;
		}
		Buffer[4] = CameraFetchID;
		Buffer[5] = 0;
		if(0 == CameraResendFlag)
		{
			Buffer[6] = CameraPacket&0x00ff;
			Buffer[7] = (CameraPacket&0xff00) >> 8;
		}
		else
		{
			Buffer[6] = CameraResendPacketList[CameraPacket]&0x00ff;
			Buffer[7] = (CameraResendPacketList[CameraPacket]&0xff00) >> 8;
		}
		if(ACK_OK == Usart3_SendData(Buffer,length))
		{
			CameraStateAck[CAMERA_STATE_FETCH] = 0;
			CameraSubState++;
		}
	}
	else if(1 == CameraSubState)//等待应答
	{
		TimeCount++;
		if(1 == CameraStateAck[CAMERA_STATE_FETCH])//有应答
		{
			Camera_StateSwitch(CAMERA_STATE_SCHEDULE);
			TimeCount = 0;
		}
		else if(3 == CameraStateAck[CAMERA_STATE_FETCH])//应答错误
		{
			CameraSubState--;
			TimeCount = 0;
		}
		else//超时检测
		{
			if(TimeCount > 20)
			{
				CameraSubState--;
				TimeCount = 0;
			}
		}
	}
}
/**************************************************************************
//函数名：Camera_StateStore
//功能：实现摄像头存储状态功能
//输入：无
//输出：无
//返回值：无
//备注：摄像头拍照任务Camera_TimeTask需要调用此函数
***************************************************************************/
static void Camera_StateStore(void)
{
	u32 Bw;
	static u8 WriteEnableFlag = 0;

	Camera[CameraFetchID-1].FetchFlag = CAMERA_STATE_STORE;

	CameraStateTimeCount[CAMERA_STATE_STORE]++;
	if(CameraStateTimeCount[CAMERA_STATE_STORE] > CAMERA_STATE_STORE_OVERTIME)//超时
	{
		CameraStateAck[CAMERA_STATE_STORE] = 4;
		Camera_StateSwitch(CAMERA_STATE_SCHEDULE);
	}

	if(1 == CameraResendFlag)
	{
		CameraStateAck[CAMERA_STATE_STORE] = 0;
		Camera_StateSwitch(CAMERA_STATE_SCHEDULE);
	}

	if((1 == Media_GetSdOnLineFlag())&&(1 == Camera[CameraFetchID-1].OnOffFlag)&&(0x02 == (0x02&CameraStoreFlag[CameraFetchID-1])))
	{
		if(0 == CameraPacket)//第1包
		{
			WriteEnableFlag = 0;
			Media_GetFileName(CameraFileName,MEDIA_TYPE_JPG);
			if(FR_OK == f_open(&CameraFile,ff_NameConver(CameraFileName),FA_OPEN_ALWAYS|FA_READ|FA_WRITE))
			{
				WriteEnableFlag = 1;
			}
			else
			{
				Camera_StateSwitch(CAMERA_STATE_SCHEDULE);
			}	
		}
		if(1 == WriteEnableFlag)
		{
			if(FR_OK == f_write(&CameraFile,CameraDataBuffer,CameraDataBufferLen,&Bw))
			{
				if(CameraPacket >= (CameraTotalPacket[CameraFetchID-1]-1))//最后1包
				{
					if(FR_OK == f_write(&CameraFile,CameraPhotoInfo+(CameraFetchID-1)*36,36,&Bw))
					{
				
					}
					else
					{
						
					}
					f_close(&CameraFile);
					Media_SaveLog(PhotoID,MEDIA_TYPE_JPG);
				}
				CameraStateAck[CAMERA_STATE_STORE] = 1;
				Camera_StateSwitch(CAMERA_STATE_SCHEDULE);
			}
			else
			{
				WriteEnableFlag = 0;
				CameraStateAck[CAMERA_STATE_STORE] = 2;
				Camera_StateSwitch(CAMERA_STATE_SCHEDULE);
			}
		}
		else
		{
			Camera_StateSwitch(CAMERA_STATE_SCHEDULE);
		}	
		
	}
	else
	{
		CameraStateAck[CAMERA_STATE_STORE] = 0;
		Camera_StateSwitch(CAMERA_STATE_SCHEDULE);
	}
}
/**************************************************************************
//函数名：Camera_StateSend
//功能：实现摄像头发送状态功能
//输入：无
//输出：无
//返回值：无
//备注：摄像头拍照任务Camera_TimeTask需要调用此函数
***************************************************************************/
static void Camera_StateSend(void)
{
	static u16 TimeCount = 0;
	static u8 WaitFlag = 0;
	static u16 WaitCount = 0;

	u16 length;
	
	if(1 == WaitFlag)
	{
		WaitCount++;
		if(WaitCount >= 5*SYSTICK_0p1SECOND)
		{
			WaitCount = 0;
			WaitFlag = 0;
		}
		return ;
	}
	
	Camera[CameraFetchID-1].FetchFlag = CAMERA_STATE_SEND;
	
	CameraStateTimeCount[CAMERA_STATE_SEND]++;
	if(CameraStateTimeCount[CAMERA_STATE_SEND] > CAMERA_STATE_SEND_OVERTIME)//超时
	{
		CameraStateAck[CAMERA_STATE_SEND] = 4;
		Camera_StateSwitch(CAMERA_STATE_SCHEDULE);
		return ;
	}

	if(0x06 == (0x06&GetTerminalAuthorizationFlag()))//两个通道都在线
	{

	}
	else if(CameraChannel[CameraFetchID-1] != (CameraChannel[CameraFetchID-1]&GetTerminalAuthorizationFlag()))//至少有一个通道不在线
	{
		CameraStateAck[CAMERA_STATE_SEND] = 2;
		Camera_StateSwitch(CAMERA_STATE_SCHEDULE);
		return ;
	}


	if(0x01 != (0x01&CameraStoreFlag[CameraFetchID-1]))
	{
		CameraStateAck[CAMERA_STATE_SEND] = 2;
		Camera_StateSwitch(CAMERA_STATE_SCHEDULE);
		return ;
	}

	if(0 == CameraSubState)//发送
	{
		if(0 == CameraResendFlag)
		{
			length = CameraDataBufferLen;
			if(0 == CameraPacket)//第1包
			{
				memmove(CameraDataBuffer+36,CameraDataBuffer,CameraDataBufferLen);
				memcpy(CameraDataBuffer,CameraPhotoInfo+(CameraFetchID-1)*36,36);
				length += 36;
			}
			if(ACK_OK == RadioProtocol_MultiMediaDataReport(CameraChannel[CameraFetchID-1],CameraDataBuffer,length,CameraTotalPacket[CameraFetchID-1],CameraPacket+1))
			{
				CameraStateAck[CAMERA_STATE_SEND] = 1;
				CameraSubState++;
				TimeCount = 0;
			}
			else
			{
				WaitFlag = 1;
			}
		}
		else
		{
			length = CameraDataBufferLen;
			if(0 == CameraResendPacketList[CameraPacket])//第1包
			{
				memmove(CameraDataBuffer+36,CameraDataBuffer,CameraDataBufferLen);
				memcpy(CameraDataBuffer,CameraPhotoInfo+(CameraFetchID-1)*36,36);
				length += 36;
			}
			if(ACK_OK == RadioProtocol_MultiMediaDataReport(CameraChannel[CameraFetchID-1],CameraDataBuffer,length,CameraTotalPacket[CameraFetchID-1],CameraResendPacketList[CameraPacket]))
			{
				CameraStateAck[CAMERA_STATE_SEND] = 1;
				CameraSubState++;
				TimeCount = 0;
			}
			else
			{
				WaitFlag = 1;
			}
		}
	}
	else if(1 == CameraSubState)//等待应答
	{
		TimeCount++;
		if((CHANNEL_DATA_1 == CameraChannel[CameraFetchID-1])||(CHANNEL_DATA_2 == CameraChannel[CameraFetchID-1]))
		{
			if(TimeCount >= 4*SYSTICK_0p1SECOND)
			{
				TimeCount = 0;
				Camera_StateSwitch(CAMERA_STATE_SCHEDULE);
			}
		}
		else//双通道同时发送图片的话不能过快
		{
			if(TimeCount >= SECOND)
			{
				TimeCount = 0;
				Camera_StateSwitch(CAMERA_STATE_SCHEDULE);
			}
		}
	}
}
/**************************************************************************
//函数名：Camera_StateResend
//功能：实现摄像头补传状态功能
//输入：无
//输出：无
//返回值：无
//备注：摄像头拍照任务Camera_TimeTask需要调用此函数
***************************************************************************/
static void Camera_StateResend(void)
{
	Camera[CameraFetchID-1].FetchFlag = CAMERA_STATE_RESEND;
	CameraStateTimeCount[CAMERA_STATE_RESEND]++;
	if(CameraStateTimeCount[CAMERA_STATE_RESEND] > CAMERA_STATE_RESEND_OVERTIME)//超时
	{
		CameraStateAck[CAMERA_STATE_RESEND] = 4;
		Camera_StateSwitch(CAMERA_STATE_SCHEDULE);
		return ;
	}

	if(CameraResendTotalPacket > 0)
	{
		Camera_StateSwitch(CAMERA_STATE_SCHEDULE);
		return ;
	}
}
/**************************************************************************
//函数名：Camera_WriteParameter
//功能：写拍照参数到拍照变量和铁电中
//输入：Channel:上传通道;只能是CHANNEL_DATA_1或者CHANNEL_DATA_2
//	CameraID:摄像头ID号;只能是1~CAMERA_ID_MAX
//	PhotoNum:拍照张数;当为0xfffe,0xffff表示一直拍照；当为0时表示停止拍照
//	PhotoSpace:拍照间隔;0值无效
//	Resolution:拍照分辨率;0为320*240分辨率，1为640*480分辨率，其它值无效
//	StoreFlag:存储标志;bit0置1表示需要上传，bit1置1表示需要保存，两个都置1表示既要上传也要保存，其它位保留
//	PhotoType:拍照类型;0表示定时拍照，1表示事件拍照，其它值无效
//	EventType:事件类型;0表示平台下发指令，1表示定时动作，2表示紧急报警，3表示碰撞或侧翻，当都不属于这几种时建议使用0
//输出：无
//返回值：无
//备注：
***************************************************************************/
static void Camera_WriteParameter(u8 Channel, u8 CameraID, u16 PhotoNum, u16 PhotoSpace, u8 Resolution, u8 StoreFlag, u8 PhotoType, u8 EventType)
{
	u8 Buffer[5];
	u8 i;

	if((CameraID == 0)||(CameraID > CAMERA_ID_MAX))
	{
		return;
	}

	if(0 == PhotoType)//写定时拍照参数
	{

		for(i=1; i<=CAMERA_ID_MAX; i++)
		{
			Camera[i-1].FetchFlag = 0;
		}	
		for(i=0; i<=CAMERA_STATE_MAX; i++)
		{
			CameraStateAck[i] = 0;
			CameraStateErrorCount[i] = 0;
		}
		CameraState = 0;
		CameraSubState = 0;
		CameraPhotoID = 1;
		CameraFetchID = 1;
		CameraCheckID = 1;
		CameraPacket = 0;
		StateCheckCount1 = 0;
		StateCheckCount2 = 0;
		CameraResendEnableFlag = 0;
		CameraResendFlag = 0;
		CameraResendTotalPacket = 0;

		Camera[CameraID-1].TimePhotoTime = RTC_GetCounter();

		Camera[CameraID-1].TimePhotoNum = PhotoNum;
		Public_ConvertShortToBuffer(PhotoNum,Buffer);
		FRAM_BufferWrite(FRAM_CAM_TIME_PHOTO_NUM_ADDR+(CameraID-1)*(FRAM_CAM_TIME_PHOTO_NUM_LEN+1),Buffer,FRAM_CAM_TIME_PHOTO_NUM_LEN);
						
		Camera[CameraID-1].TimePhotoSpace = PhotoSpace;
		Public_ConvertShortToBuffer(PhotoSpace,Buffer);
		FRAM_BufferWrite(FRAM_CAM_TIME_PHOTO_SPACE_ADDR+(CameraID-1)*(FRAM_CAM_TIME_PHOTO_SPACE_LEN+1),Buffer,FRAM_CAM_TIME_PHOTO_SPACE_LEN);

		Camera[CameraID-1].TimePhotoStoreFlag = StoreFlag;
		Buffer[0] = StoreFlag;
		FRAM_BufferWrite(FRAM_CAM_TIME_PHOTO_STORE_FLAG_ADDR+(CameraID-1)*(FRAM_CAM_TIME_PHOTO_STORE_FLAG_LEN+1),Buffer,FRAM_CAM_TIME_PHOTO_STORE_FLAG_LEN);

		Camera[CameraID-1].TimePhotoResolution = Resolution;
		Buffer[0] = Resolution;
		FRAM_BufferWrite(FRAM_CAM_TIME_PHOTO_RESOLUTION_ADDR+(CameraID-1)*(FRAM_CAM_TIME_PHOTO_RESOLUTION_LEN+1),Buffer,FRAM_CAM_TIME_PHOTO_RESOLUTION_LEN);

		Camera[CameraID-1].TimePhotoEventType = EventType;
		Buffer[0] = EventType;
		FRAM_BufferWrite(FRAM_CAM_TIME_PHOTO_EVENT_TYPE_ADDR+(CameraID-1)*(FRAM_CAM_TIME_PHOTO_EVENT_TYPE_LEN+1),Buffer,FRAM_CAM_TIME_PHOTO_EVENT_TYPE_LEN);

		Camera[CameraID-1].TimePhotoChannel = Channel;
		Buffer[0] = Channel;
		FRAM_BufferWrite(FRAM_CAM_TIME_PHOTO_CHANNEL_ADDR+(CameraID-1)*(FRAM_CAM_TIME_PHOTO_CHANNEL_LEN+1),Buffer,FRAM_CAM_TIME_PHOTO_CHANNEL_LEN);

		Camera_StateSwitch(CAMERA_STATE_SCHEDULE);
	}
	else if(1 == PhotoType)//写事件拍照参数
	{
		if((1 == Camera[CameraID-1].OnOffFlag)&&(Camera[CameraID-1].FetchFlag < CAMERA_STATE_FETCH))//空闲
		{
			Camera[CameraID-1].FetchFlag = 0;

			if(PhotoNum > 3)
			{
				PhotoNum = 3;
			}
			Camera[CameraID-1].EventPhotoNum = PhotoNum;
			Public_ConvertShortToBuffer(PhotoNum,Buffer);
			FRAM_BufferWrite(FRAM_CAM_EVENT_PHOTO_NUM_ADDR+(CameraID-1)*(FRAM_CAM_EVENT_PHOTO_NUM_LEN+1),Buffer,FRAM_CAM_EVENT_PHOTO_NUM_LEN);

			Camera[CameraID-1].EventPhotoTime = RTC_GetCounter();
						
			Camera[CameraID-1].EventPhotoSpace = PhotoSpace;
			Public_ConvertShortToBuffer(PhotoSpace,Buffer);
			FRAM_BufferWrite(FRAM_CAM_EVENT_PHOTO_SPACE_ADDR+(CameraID-1)*(FRAM_CAM_EVENT_PHOTO_SPACE_LEN+1),Buffer,FRAM_CAM_EVENT_PHOTO_SPACE_LEN);

			Camera[CameraID-1].EventPhotoStoreFlag = StoreFlag;
			Buffer[0] = StoreFlag;
			FRAM_BufferWrite(FRAM_CAM_EVENT_PHOTO_STORE_FLAG_ADDR+(CameraID-1)*(FRAM_CAM_EVENT_PHOTO_STORE_FLAG_LEN+1),Buffer,FRAM_CAM_EVENT_PHOTO_STORE_FLAG_LEN);

			Camera[CameraID-1].EventPhotoResolution = Resolution;
			Buffer[0] = Resolution;
			FRAM_BufferWrite(FRAM_CAM_EVENT_PHOTO_RESOLUTION_ADDR+(CameraID-1)*(FRAM_CAM_EVENT_PHOTO_RESOLUTION_LEN+1),Buffer,FRAM_CAM_EVENT_PHOTO_RESOLUTION_LEN);

			Camera[CameraID-1].EventPhotoEventType = EventType;
			Buffer[0] = EventType;
			FRAM_BufferWrite(FRAM_CAM_EVENT_PHOTO_EVENT_TYPE_ADDR+(CameraID-1)*(FRAM_CAM_EVENT_PHOTO_EVENT_TYPE_LEN+1),Buffer,FRAM_CAM_EVENT_PHOTO_EVENT_TYPE_LEN);

			Camera[CameraID-1].EventPhotoChannel = Channel;
			Buffer[0] = Channel;
			FRAM_BufferWrite(FRAM_CAM_EVENT_PHOTO_CHANNEL_ADDR+(CameraID-1)*(FRAM_CAM_EVENT_PHOTO_CHANNEL_LEN+1),Buffer,FRAM_CAM_EVENT_PHOTO_CHANNEL_LEN);
		}
		else if((1 == Camera[CameraID-1].OnOffFlag)&&(Camera[CameraID-1].FetchFlag >= CAMERA_STATE_FETCH))//已在取图
		{
			Camera[CameraID-1].EventPhotoNum = Camera[CameraID-1].EventPhotoNum+PhotoNum;
			if(Camera[CameraID-1].EventPhotoNum > 3)
			{
				Camera[CameraID-1].EventPhotoNum = 3;
			}
			Public_ConvertShortToBuffer(PhotoNum,Buffer);
			FRAM_BufferWrite(FRAM_CAM_EVENT_PHOTO_NUM_ADDR+(CameraID-1)*(FRAM_CAM_EVENT_PHOTO_NUM_LEN+1),Buffer,FRAM_CAM_EVENT_PHOTO_NUM_LEN);	
		}
	}
}






























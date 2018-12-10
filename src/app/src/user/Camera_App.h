#ifndef __CAMERA_APP_H
#define __CAMERA_APP_H

#include "stm32f2xx.h"

#define CAMERA_STATE_SCHEDULE 	0//调度状态
#define CAMERA_STATE_CHECK 	1//自检状态
#define CAMERA_STATE_PHOTO 	2//拍照状态
#define CAMERA_STATE_FETCH 	3//取图状态
#define CAMERA_STATE_STORE 	4//存储状态
#define CAMERA_STATE_SEND 	5//发送状态
#define CAMERA_STATE_RESEND 	6//补传状态
#define CAMERA_STATE_MAX	6

#define CAMERA_STATE_CHECK_OVERTIME 	(20*10)//自检状态超时时间，50ms为单位，10秒超时
#define CAMERA_STATE_PHOTO_OVERTIME 	(20*15)//拍照状态超时时间，50ms为单位，5秒超时
#define CAMERA_STATE_FETCH_OVERTIME 	(20*15)//取图状态超时时间
#define CAMERA_STATE_STORE_OVERTIME 	(20*15)//存储状态超时时间
#define CAMERA_STATE_SEND_OVERTIME 	(20*15)//发送状态超时时间
#define CAMERA_STATE_RESEND_OVERTIME 	(20*15)//补传状态超时时间

#define CAMERA_ID_MAX	4//最多支持4路，ID从1开始

#define CAMERA_DATA_BUFFER_SIZE 560
#define CAMERA_CMD_BUFFER_SIZE  20
#define CAMERA_RESEND_LIST_SIZE 20

typedef struct
{
	u8 OnOffFlag;//在线标志
	u8 FetchFlag;//取图请求标志

	s32 TimePhotoNum;//拍照张数（张数大于1，等于1的归类到事件拍照）
	u16 TimePhotoSpace;//拍照时间间隔
	u8 TimePhotoStoreFlag;//存储标志:bit0:1表示需要上传,0表示不需要上传;bit1:1表示需要保存,0表示不需要保存
	u8 TimePhotoResolution;//拍照分辨率
	u8 TimePhotoEventType;//拍照事件类型，一般固定为0
	u8 TimePhotoChannel;//上传通道
	u32 TimePhotoTime;//拍照时间
	
	s32 EventPhotoNum;//事件拍照张数，最多是3张，超过3张按3张拍
	u16 EventPhotoSpace;//事件拍照时间间隔
	u8 EventPhotoStoreFlag;//存储标志
	u8 EventPhotoResolution;//拍照分辨率
	u8 EventPhotoEventType;//拍照事件类型
	u8 EventPhotoChannel;//上传通道
	u32 EventPhotoTime;//拍照时间
	

}CAMERA_STRUCT;

/**************************************************************************
//函数名：Camera_TimeTask
//功能：实现拍照功能
//输入：无
//输出：无
//返回值：无
//备注：拍照定时任务，50ms调度1次，任务调度器需要调用此函数。
***************************************************************************/
FunctionalState Camera_TimeTask(void);
/**************************************************************************
//函数名：Camera_Photo
//功能：摄像头拍照
//输入：Channel:上传通道;只能是CHANNEL_DATA_1或者CHANNEL_DATA_2
//	CameraID:摄像头ID号;只能是0~CAMERA_ID_MAX，0表示全拍，其它表示单个拍
//	PhotoNum:拍照张数;当为0xfffe,0xffff表示一直拍照；当为0时表示停止拍照
//	PhotoSpace:拍照间隔;0值无效
//	Resolution:拍照分辨率;0为320*240分辨率，1为640*480分辨率，其它值无效
//	StoreFlag:存储标志;bit0置1表示需要上传，bit1置1表示需要保存，两个都置1表示既要上传也要保存，其它位保留
//	PhotoType:拍照类型;0表示定时拍照，1表示事件拍照，其它值无效
//	EventType:事件类型;0表示平台下发指令，1表示定时动作，2表示紧急报警，3表示碰撞或侧翻，当都不属于这几种时建议使用0
//输出：无
//返回值：0表示成功，1表示失败
//备注：当正在上传图片时又收到了新的拍照指令，则正在上传的图片会丢失，立即去执行新的拍照指令
***************************************************************************/
u8 Camera_Photo(u8 Channel, u8 CameraID, u16 PhotoNum, u16 PhotoSpace, u8 Resolution, u8 StoreFlag, u8 PhotoType, u8 EventType);
/**************************************************************************
//函数名：Camera_MediaUploadAck
//功能：多媒体上传应答
//输入：pBuffer首地址指向重传包总数，然后是包列表
//输出：无
//返回值：无
//备注：当终端全部上传完所有包时，平台需下发多媒体上传应答命令0x8800，
//收到该命令后，需调用此函数
***************************************************************************/
void Camera_MediaUploadAck(u8 *pBuffer, u16 BufferLen);
/**************************************************************************
//函数名：Camera_GetOnlineFlag
//功能：获取摄像头在线标志
//输入：物
//输出：无
//返回值：在线标志
//备注：bit7~bit4保留，bit3~bit0:分别表示1~4号摄像头在线标志，1在线，0不在线
***************************************************************************/
u8 Camera_GetOnlineFlag(void);
/**************************************************************************
//函数名：Camera_GetTotalNum
//功能：获取总的在线摄像头个数
//输入：无
//输出：无
//返回值：总的在线摄像头个数
//备注：最多为CAMERA_ID_MAX个
***************************************************************************/
u8 Camera_GetTotalNum(void);
#endif
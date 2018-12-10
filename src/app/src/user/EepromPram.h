#ifndef __EEPROM_PRAM_H
#define __EEPROM_PRAM_H

//*********文件包含************
#include "stm32f2xx.h"
#include "stm32f2xx_conf.h"
//********************************************************
//********************************************************
//////////分配空间时特需要注意//////////////////
//类型为1的总空间=数据内容长度(1字节)+数据内容(n字节)+校验(1字节)
//类型为0的总空间=数据内容(n字节)+校验(1字节)
//********************************************************
//********************************************************
//********宏定义****************
////////////参数索引的最大值,通过修改E2_SYSTEM_DEFINE_ID_MAX、E2_SELF_DEFINE_ID_MAX的相应值即可/////////////////////////
#define	E2_PRAM_ID_INDEX_MAX	(E2_SYSTEM_DEFINE_ID_MAX+(E2_SELF_DEFINE_ID_MAX-E2_PRAM_BASE_CUSTOM_ID)+1)//参数索引的最大值,即最多有多少项参数

#define	E2_PRAM_BASE_CUSTOM_ID	0xF200  //自定义参数基地址


//****************参数ID定义******************
#define E2_TERMINAL_HEARTBEAT_ID 		0x0001//终端心跳发送间隔//单位秒//默认值60
#define E2_TCP_ACK_OVERTIME_ID 			0x0002//TCP消息应答超时时间//单位秒//默认值10
#define E2_TCP_RESEND_TIMES_ID 			0x0003//TCP消息重传次数//默认值3
#define E2_UDP_ACK_OVERTIME_ID 			0x0004//UDP消息应答超时时间//单位秒//默认值10
#define E2_UDP_RESEND_TIMES_ID 			0x0005//UDP消息重传次数//默认值3
#define E2_SMS_ACK_OVERTIME_ID 			0x0006//SMS消息应答超时时间//单位秒//默认值30
#define E2_SMS_RESEND_TIMES_ID 			0x0007//SMS消息重传次数//默认值3
#define E2_MAIN_SERVER_APN_ID 			0x0010//主服务器APN
#define E2_MAIN_SERVER_DIAL_NAME_ID 		0x0011//主服务器无线通信拨号用户名
#define E2_MAIN_SERVER_DIAL_PASSWORD_ID 	0x0012//主服务器无线通信拨号密码
#define E2_MAIN_SERVER_IP_ID 			0x0013//主服务器地址//IP或域名
#define E2_BACKUP_SERVER_APN_ID 		0x0014//备份服务器APN//无线通信拨号访问点
#define E2_BACKUP_SERVER_DIAL_NAME_ID 		0x0015//备份服务器无线通信拨号用户名
#define E2_BACKUP_SERVER_DIAL_PASSWORD_ID 	0x0016//备份服务器无线通信拨号密码
#define E2_BACKUP_SERVER_IP_ID 			0x0017//备份服务器地址//IP或域名
#define E2_MAIN_SERVER_TCP_PORT_ID 		0x0018//主服务器TCP端口
#define E2_MAIN_SERVER_UDP_PORT_ID 		0x0019//主服务器UDP端口
#define E2_IC_MAIN_SERVER_IP_ID 		0x001A//IC卡认证主服务器IP地址或域名
#define E2_IC_MAIN_SERVER_TCP_PORT_ID           0x001B//IC卡认证主服务器TCP端口
#define E2_IC_MAIN_SERVER_UDP_PORT_ID           0x001C//IC卡认证主服务器UDP端口
#define E2_IC_BACKUP_SERVER_IP_ID 		0x001D//IC卡认证备份服务器IP地址或域名
#define E2_POSITION_REPORT_STRATEGY_ID 		0x0020//位置汇报策略,0为定时,1为定距,2为定时定距,默认为0
#define E2_POSITION_REPORT_SCHEME_ID 		0x0021//位置汇报方案,0根据ACC状态,1根据登录及ACC状态,默认为0
#define E2_DRIVER_UNLOGIN_REPORT_TIME_ID 	0x0022//驾驶员未登录汇报时间间隔//单位为秒//默认值为60
#define E2_SLEEP_REPORT_TIME_ID 		0x0027//休眠时汇报时间间隔//单位为秒//默认值为240
#define E2_EMERGENCY_REPORT_TIME_ID 		0x0028//紧急报警时汇报时间间隔//单位为秒//默认值为10
#define	E2_ACCON_REPORT_TIME_ID			0x0029//缺省时间汇报间隔,单位为秒,默认为30
#define E2_ACCON_REPORT_DISTANCE_ID		0x002C//缺省距离汇报间隔,单位为米,默认为500
#define E2_DRIVER_UNLOGIN_REPORT_DISTANCE_ID 	0x002D//驾驶员未登录汇报距离间隔//单位为米//默认值为500
#define E2_SLEEP_REPORT_DISTANCE_ID 		0x002e//休眠时汇报距离间隔//单位为米//默认值为200
#define E2_EMERGENCY_REPORT_DISTANCE_ID 	0x002f//紧急报警时汇报距离间隔//单位为米//默认值为200
#define E2_CORNER_REPORT_ID 			0x0030//拐点补传角度,单位度/秒//默认值为30
#define	E2_ELECTRON_RADIUS_ID			0x0031//电子围栏半径,单位米,默认值500
#define E2_MONITOR_SERVER_PHONE_ID 		0x0040//监控平台电话号码
#define E2_TERMINAL_RESET_PHONE_ID 		0x0041//终端复位电话号码
#define E2_TERMINAL_DEFAULT_SET_PHONE_ID 	0x0042//恢复出厂设置电话号码
#define E2_MONITOR_SERVER_SMS_PHONE_ID 		0x0043//监控平台SMS电话号码
#define E2_SMS_TEXT_ALARM_PHONE_ID 		0x0044//SMS文本报警电话号码
#define E2_TERMINAL_GET_PHONE_STRATEGY_ID 	0x0045//终端电话接听策略,0:自动接听;1:手动接听,默认为0
#define E2_MAX_PHONE_TIME_EACH_ID 		0x0046//每次最长通话时间//单位为秒//默认为0,关闭该功能
#define E2_MAX_PHONE_TIME_MONTH_ID 		0x0047//每月最长通话时间//单位为秒//默认为0,关闭该功能
#define E2_MONITOR_PHONE_ID 			0x0048//监听电话号码
#define E2_MONITOR_SERVER_PRIVILEGE_SMS_ID 	0x0049//监管平台特权短信号码
#define E2_ALARM_MASK_ID 			0x0050//报警屏蔽字//bit9,11,12默认屏蔽,LCD屏故障,TTS故障,IC卡模块故障默认为屏蔽
#define E2_ALARM_TEXT_SWITCH_ID 		0x0051//报警发送文本SMS开关//相应位为1表示产生报警了需发送报警短信,默认为0
#define E2_ALARM_PHOTO_SWITCH_ID 		0x0052//报警拍摄开关//相应位为1表示产生报警了需触发拍照,默认为1,紧急报警触发拍照
#define E2_ALARM_PHOTO_STORE_FLAG_ID 		0x0053//报警拍摄存储标志//相应位为1表示存储,0表示上传,默认为0
#define E2_KEY_FLAG_ID 				0x0054//关键标志//相应位为1表示该报警为关键报警,是关键报警则不受报警屏蔽字的限制,默认为0
#define E2_MAX_SPEED_ID 			0x0055//最高速度,单位km/h,DWORD,默认值130,九通需求为100
#define E2_OVER_SPEED_KEEP_TIME_ID 		0x0056//超速持续时间,单位为秒,DWORD,默认为10秒,九通需求为立马上报
#define E2_MAX_DRVIE_TIME_ID 			0x0057//连续驾驶时间门限//单位为秒//默认为14400秒(4小时)
#define E2_MAX_DRVIE_TIME_ONEDAY_ID 		0x0058//当天累计驾驶时间门限//单位为秒//默认为0xffffffff,不开启该功能
#define E2_MIN_RELAX_TIME_ID 			0x0059//最小休息时间//单位为秒//默认为1200（20分钟）
#define E2_MAX_STOP_CAR_TIME_ID 		0x005A//最长停车时间//单位为秒//默认为0xffffffff,不开启该功能
#define	E2_SPEED_EARLY_ALARM_DVALUE_ID          0x005B//超速报警预警差值，单位为1/10Km/h,DWORD,默认为50（5km/h）  
#define	E2_TIRE_EARLY_ALARM_DVALUE_ID	        0x005C//疲劳驾驶预警差值，单位为秒（s），>0  默认为1800秒（30分钟）
/*
碰撞报警参数设置：
b7-b0： 碰撞时间，单位4ms；
b15-b8：碰撞加速度，单位0.1g，设置范围在：0-79 之间，默认为10*/
#define	E2_SET_CRASH_ALARM_ID			0x005D//碰撞报警参数设置,碰撞加速度默认设置为4g
#define	E2_SET_ROLLOVER_ALARM_ID	        0x005E//侧翻报警参数设置：侧翻角度，单位1 度，默认为45 度
/*定时拍照控制位定义
位    定义                         描述及要求
0     摄像通道1定时拍照开关标志    0：不允许； 1：允许
1     摄像通道2定时拍照开关标志    0：不允许； 1：允许
2     摄像通道3定时拍照开关标志    0：不允许； 1：允许
3     摄像通道4定时拍照开关标志    0：不允许； 1：允许
4     摄像通道5定时拍照开关标志    0：不允许； 1：允许
5-7   保留
8     摄像通道1定时拍照存储标志    0：存储；   1：上传
9     摄像通道2定时拍照存储标志    0：存储；   1：上传
10    摄像通道3定时拍照存储标志    0：存储；   1：上传
11    摄像通道4定时拍照存储标志    0：存储；   1：上传
12    摄像通道5定时拍照存储标志    0：存储；   1：上传
13-15 保留
16    定时时间单位                 0：秒,当数值小于5 秒时,终端按5秒处理；
                                   1：分
17-31 定时时间间隔                 收到参数设置或重新启动后执行*/
#define	E2_TIME_PHOTO_CONTROL_ID	        0x0064//定时拍照控制,默认为不开启,值为0
/*
定距拍照控制位定义
位    定义                         描述及要求
0     摄像通道1定距拍照开关标志    0：不允许； 1：允许
1     摄像通道2定距拍照开关标志    0：不允许； 1：允许
2     摄像通道3定距拍照开关标志    0：不允许； 1：允许
3     摄像通道4定距拍照开关标志    0：不允许； 1：允许
4     摄像通道5定距拍照开关标志    0：不允许； 1：允许
5-7 保留
8     摄像通道1定距拍照存储标志    0：存储；   1：上传
9     摄像通道2定距拍照存储标志    0：存储；   1：上传
18
10    摄像通道3定距拍照存储标志    0：存储；   1：上传
11    摄像通道4定距拍照存储标志    0：存储；   1：上传
12    摄像通道5定距拍照存储标志    0：存储；   1：上传
13-15 保留
16 定距距离单位                    0：米，当数值小于100 米时，终端按100 米处理；
                                   1：公里
17-31 定距距离间隔 收到参数设置或重新启动后执行*/
#define	E2_MILE_PHOTO_CONTROL_ID		0x0065//定距拍照控制,默认为不开启,值为0
#define E2_PICTURE_QUALITY_ID 			0x0070//图像/视频质量//1~10,1为最好
#define E2_PICTURE_BRIGHTNESS_ID 		0x0071//亮度//0~255
#define E2_PICTURE_CONTRAST_ID 			0x0072//对比度//0~127
#define E2_PICTURE_SATURATION_ID 		0x0073//饱和度//0~127
#define E2_PICTURE_CHROMA_ID 			0x0074//色度//0~255
#define E2_CAR_TOTAL_MILE_ID 			0x0080//车辆里程表读数//0.1km
#define E2_CAR_PROVINCE_ID 			0x0081//车辆所在的省域ID
#define E2_CAR_CITY_ID 				0x0082//车辆所在的市域ID
#define E2_CAR_PLATE_NUM_ID 			0x0083//公安交通管理部门颁发的机动车号牌
#define E2_CAR_PLATE_COLOR_ID 			0x0084//车牌颜色//按照JT/T415-2006的5.4.12,1:蓝色,2:黄色,3:黑色,4:白色,9:其它
/*
GNSS  定位模式，定义如下：
bit0，0：禁用GPS 定位，     1：启用GPS 定位；
bit1，0：禁用北斗定位，     1：启用北斗定位；
bit2，0：禁用GLONASS 定位， 1：启用GLONASS 定位；
bit3，0：禁用Galileo 定位， 1：启用Galileo 定位.*/
#define	E2_GPS_SET_MODE_ID			0x0090//GNSS 定位模式，默认为0x02 北斗定位
/*
GNSS 波特率，定义如下：
0x00：4800；0x01：9600；
0x02：19200；0x03：38400；
0x04：57600；0x05：115200.*/
#define	E2_GPS_SET_BAUD_ID			0x0091//GNSS 波特率，默认为0x01 9600
/*
GNSS 模块详细定位数据输出频率，定义如下：
0x00：500ms；0x01：1000ms（默认值）；
0x02：2000ms；0x03：3000ms；
0x04：4000ms.*/
#define	E2_GPS_SET_OUTPUT_RATE_ID	        0x0092//GNSS 模块详细定位数据输出频率,默认为0x01 1000ms
#define	E2_GPS_SET_GATHER_RATE_ID		0x0093//GNSS 模块详细定位数据采集频率，单位为秒，默认为1.
/*
GNSS 模块详细定位数据上传方式：
0x00，本地存储，不上传（默认值）；
0x01，按时间间隔上传；
0x02，按距离间隔上传；
0x0B，按累计时间上传，达到传输时间后自动停止上传；
0x0C，按累计距离上传，达到距离后自动停止上传；
0x0D，按累计条数上传，达到上传条数后自动停止上传*/
#define	E2_GPS_SET_UPLOAD_MODE_ID		0x0094//GNSS 模块详细定位数据上传方式，默认为0x00，本地存储不上传
/*
GNSS 模块详细定位数据上传设置：
上传方式为0x01 时，单位为秒；
上传方式为0x02 时，单位为米；
上传方式为0x0B 时，单位为秒；
上传方式为0x0C 时，单位为米；
上传方式为0x0D 时，单位为条。*/
#define	E2_GPS_SET_UPLOAD_VALUE_ID		0x0095//GNSS 模块详细定位数据上传设置,默认为0	
#define	E2_CAN1_GATHER_TIME_ID			0x0100//CAN 总线通道1 采集时间间隔(ms)，0 表示不采集 
#define	E2_CAN1_UPLOAD_TIME_ID			0x0101//CAN 总线通道1 上传时间间隔(s)，0 表示不上传
#define	E2_CAN2_GATHER_TIME_ID			0x0102//CAN 总线通道2 采集时间间隔(ms)，0 表示不采集
#define	E2_CAN2_UPLOAD_TIME_ID			0x0103//CAN 总线通道2 上传时间间隔(s)，0 表示不上传
/*
CAN 总线ID 单独采集设置：
bit63-bit32    表示此ID 采集时间间隔(ms)， 0 表示不采集；
bit31          表示CAN 通道号，            0：CAN1，    1：CAN2；
bit30          表示帧类型，                0：标准帧，  1：扩展帧；
bit29          表示数据采集方式，          0：原始数据，1：采集区间的计算值；
bit28-bit0     表示CAN 总线ID*/
#define	E2_CAN_SET_ONLY_GATHER_0_ID		0x0110//CAN 总线ID 单独采集设置BYTE[8]
#define	E2_CAN_SET_ONLY_GATHER_1_ID		0x0111//CAN 总线ID 单独采集设置BYTE[8]

///////////系统定义的最大ID为最后的ID加1,请根据最后一项人工修改/////////////////////////////////////
#define	E2_SYSTEM_DEFINE_ID_MAX			(E2_CAN_SET_ONLY_GATHER_1_ID+1)//系统定义的最大ID为最后的ID加1,注意：每次新增参数时需要将最后的参数ID放在这
///////////////////////////////////////////////////////

//*****************自定义参数ID******************
#define E2_LOGINGPRS_PASSWORD_ID		E2_PRAM_BASE_CUSTOM_ID+0x00//鉴权码
#define E2_REGISTER_SMS_PHONE_ID		E2_PRAM_BASE_CUSTOM_ID+0x01//注册短消息号码,发注册指令到该号码
#define E2_PROGRAM_UPDATA_REQUEST_ID		E2_PRAM_BASE_CUSTOM_ID+0x02//参数ID，自定义，程序升级请求标志
#define E2_PROGRAM_UPDATA_VERIFYCODE_ID		E2_PRAM_BASE_CUSTOM_ID+0x03//参数ID，自定义，程序升级校验码
#define E2_PROGRAM_UPDATA_LENGTH_ID		E2_PRAM_BASE_CUSTOM_ID+0x04//参数ID，自定义，程序升级长度
#define E2_MANUFACTURE_ID			E2_PRAM_BASE_CUSTOM_ID+0x05//参数ID，自定义，制造商ID，5字节		
#define E2_DEVICE_ID				E2_PRAM_BASE_CUSTOM_ID+0x06//参数ID，自定义，厂商终端ID，7字节
#define E2_DEVICE_PHONE_ID			E2_PRAM_BASE_CUSTOM_ID+0x07//参数ID，自定义，终端手机号，6字节，
#define	E2_CAR_IDENTIFICATION_CODE_ID		E2_PRAM_BASE_CUSTOM_ID+0x08//参数ID，自定义，车辆识别码
#define	E2_CAR_TYPE_ID				E2_PRAM_BASE_CUSTOM_ID+0x09//参数ID，自定义，车牌分类
#define	E2_CURRENT_DRIVER_ID			E2_PRAM_BASE_CUSTOM_ID+0x0A//参数ID，自定义，当前驾驶员代码
#define	E2_CURRENT_LICENSE_ID			E2_PRAM_BASE_CUSTOM_ID+0x0B//参数ID，自定义，当前驾驶证号码
#define	E2_FIRMWARE_VERSION_ID			E2_PRAM_BASE_CUSTOM_ID+0x0C//参数ID，自定义，固件版本号
#define E2_ALL_DRIVER_INFORMATION_ID		E2_PRAM_BASE_CUSTOM_ID+0x0D//参数ID，自定义，所有驾驶员信息
#define	E2_SPEED_SENSOR_COEF_ID			E2_PRAM_BASE_CUSTOM_ID+0x0E//参数ID，自定义，速度传感器系数
#define	E2_CAR_FEATURE_COEF_ID			E2_PRAM_BASE_CUSTOM_ID+0x0F//参数ID，自定义，车辆特征系数
#define E2_SPEED_SELECT_ID			E2_PRAM_BASE_CUSTOM_ID+0x10//参数ID，自定义，速度选择开关
#define E2_BACKLIGHT_SELECT_ID			E2_PRAM_BASE_CUSTOM_ID+0x11//参数ID，自定义，背光选择开关
#define E2_FIRST_POWERON_FLAG_ID		E2_PRAM_BASE_CUSTOM_ID+0x12//参数ID，自定义，系统第1次上电标志
#define E2_ACC_ON_INTERVAL_ID			E2_PRAM_BASE_CUSTOM_ID+0x13//参数ID，自定义，ACC ON 时汇报时间间隔,没有用到
#define E2_ACC_ON_DISTANCE_ID			E2_PRAM_BASE_CUSTOM_ID+0x14//参数ID，自定义，ACC ON 时汇报距离间隔,没有用到
#define	E2_UPDATA_LINK_CONTROL_ID		E2_PRAM_BASE_CUSTOM_ID+0x15//参数ID，自定义，连接控制
#define	E2_UPDATA_DIAL_NAME_ID			E2_PRAM_BASE_CUSTOM_ID+0x16//参数ID，自定义，无限升级拨号点名称
#define	E2_UPDATA_DIAL_USER_ID			E2_PRAM_BASE_CUSTOM_ID+0x17//参数ID，自定义，无限升级拨号用户名
#define	E2_UPDATA_DIAL_PASSWORD_ID		E2_PRAM_BASE_CUSTOM_ID+0x18//参数ID，自定义，无限升级拨号密码
#define	E2_UPDATA_IP_ID				E2_PRAM_BASE_CUSTOM_ID+0x19//参数ID，自定义，无限升级IP地址
#define	E2_UPDATA_TCP_PORT_ID			E2_PRAM_BASE_CUSTOM_ID+0x1a//参数ID，自定义，无限升级TCP端口
#define	E2_UPDATA_UDP_PORT_ID			E2_PRAM_BASE_CUSTOM_ID+0x1b//参数ID，自定义，无限升级UDP端口
#define	E2_UPDATA_MONITOR_SERVER_CODE_ID	E2_PRAM_BASE_CUSTOM_ID+0x1c//参数ID，自定义，无限升级监管平台鉴权码
#define	E2_UPDATA_HARDWARE_VERSION_ID		E2_PRAM_BASE_CUSTOM_ID+0x1d//参数ID，自定义，无限升级硬件版本
#define	E2_UPDATA_FIRMWARE_VERSION_ID		E2_PRAM_BASE_CUSTOM_ID+0x1e//参数ID，自定义，无限升级固件版本
#define	E2_UPDATA_URL_ID			E2_PRAM_BASE_CUSTOM_ID+0x1f//参数ID，自定义，无限升级URL完整地址
#define E2_UPDATA_TIME_ID			E2_PRAM_BASE_CUSTOM_ID+0x20//参数ID，自定义，无限升级连接到指定服务器时限
#define E2_HOT_PHONE_ID				E2_PRAM_BASE_CUSTOM_ID+0x21//参数ID，自定义，热线电话,没有用到
#define E2_UPDATA_MANUFACTURE_ID		E2_PRAM_BASE_CUSTOM_ID+0x22//参数ID，自定义，无限升级制造商ID
#define	E2_PHONE_CALLBACK_ID			E2_PRAM_BASE_CUSTOM_ID+0x23//参数ID，自定义，电话回拨号码
#define	E2_CALLBACK_FLAG_ID			E2_PRAM_BASE_CUSTOM_ID+0x24//参数ID，自定义，电话回拨标志,1为监听;0为普通通话
#define E2_DEVICE_PHONE_BACKUP_ID		E2_PRAM_BASE_CUSTOM_ID+0x25//参数ID，自定义，终端手机号，6字节，ASCII码,备份
#define E2_LAST_LOCATION_ID			E2_PRAM_BASE_CUSTOM_ID+0x26//参数ID，自定义，最后一个导航点位置
#define E2_PHOTO_SPACE_ID                       E2_PRAM_BASE_CUSTOM_ID+0x27//参数ID，自定义，拍照时间间隔
#define E2_PHOTO_NUM_ID                         E2_PRAM_BASE_CUSTOM_ID+0x28//参数ID，自定义，拍照张数
#define E2_DOOR_SET_ID                          E2_PRAM_BASE_CUSTOM_ID+0x29//参数ID，自定义，车门选择，0为车门负有效，1为车门正有效
#define E2_DEVICE_BACKUP_ID			E2_PRAM_BASE_CUSTOM_ID+0x2a//参数ID，自定义，终端ID号备份
#define E2_CDMA_SLEEP_ID			E2_PRAM_BASE_CUSTOM_ID+0x2b//参数ID，自定义，CDMA休眠开关，1为开，非1为关，默认为关
#define E2_PASSWORD_ID				E2_PRAM_BASE_CUSTOM_ID+0x2c//参数ID，自定义，维护菜单密码
#define E2_INSTALL_TIME_ID			E2_PRAM_BASE_CUSTOM_ID+0x2d//参数ID，自定义，安装日期
#define E2_INSTALL_TIME_FLAG_ID			E2_PRAM_BASE_CUSTOM_ID+0x2e//参数ID，自定义，安装日期写入标志
#define E2_HORN_SET_ID				E2_PRAM_BASE_CUSTOM_ID+0x2f//参数ID，自定义，车喇叭（输入）选择，1为低有效，非1为高有效。默认为高有效
#define	E2_GPS_CONTROL_ID			E2_PRAM_BASE_CUSTOM_ID+0x30//参数ID，自定义，ACC OFF时GPS模块控制,0为关,1为打开
#define	E2_USB_UPDATA_FLAG_ID			E2_PRAM_BASE_CUSTOM_ID+0x31//参数ID，自定义，USB升级标志,
#define	E2_TAXIMETER_CONTROL_ID			E2_PRAM_BASE_CUSTOM_ID+0x32//参数ID，自定义，计价器功能开关,0为关闭,1为开启
#define E2_TOPLIGHT_CONTROL_ID			E2_PRAM_BASE_CUSTOM_ID+0x33//参数ID，自定义，顶灯功能开关,0为关闭,1为开启，不使用
#define E2_LEDLIGHT_CONTROL_ID			E2_PRAM_BASE_CUSTOM_ID+0x34//参数ID，自定义，LED广告功能开关,0为关闭,1为开启,不使用
#define	E2_HB_IC_DRIVER_ID			E2_PRAM_BASE_CUSTOM_ID+0x35//参数ID，自定义，设置符合行标标准格式（0x0702指令）的驾驶员信息到IC卡中
#define E2_STATUS_PHOTO_SWITCH_ID 		E2_PRAM_BASE_CUSTOM_ID+0x36//参数ID，自定义，状态拍摄开关
#define E2_STATUS_PHOTO_STORE_FLAG_ID 		E2_PRAM_BASE_CUSTOM_ID+0x37//参数ID，自定义，状态拍摄存储标志//默认为全关
#define	E2_RESUME_NOUSED_ID			E2_PRAM_BASE_CUSTOM_ID+0x38//参数ID，自定义，恢复终端为未使用状态
#define	E2_NIGHT_TIME0_ID			E2_PRAM_BASE_CUSTOM_ID+0x39//参数ID，自定义，夜间时间范围,BCD码,新的EGS701第1版使用,今后不再使用这个参数
#define	E2_NIGHT_MAX_DRVIE_TIME0_ID		E2_PRAM_BASE_CUSTOM_ID+0x3a//参数ID，自定义，夜间超时驾驶门限,0表示不开启夜间模式,新的EGS701第1版使用,今后不再使用这个参数
#define	E2_NIGHT_MAX_SPEED0_ID			E2_PRAM_BASE_CUSTOM_ID+0x3b//参数ID，自定义，夜间限速值,新的EGS701第1版使用,今后不再使用这个参数
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x3C，自定义，V1。13补充协议VTK用作车门和SOS输入切换，VTK用，行标无此项
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x3D，自定义，V1。13补充协议VTK用作自动设防启动时间，VTK用，行标无此项
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x3E，自定义，V1。13补充协议VTK用作休眠时间，VTK用，行标无此项
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x3F，自定义，V1。13补充协议VTK用作震动检测时间，VTK用，行标无此项
#define	E2_LOAD_MAX_ID				E2_PRAM_BASE_CUSTOM_ID+0x40//参数ID，自定义，载重最大限制，超过该值会有超载报警，单位为kg，4字节
#define E2_CAR_INIT_MILE_ID                     E2_PRAM_BASE_CUSTOM_ID+0x41//参数ID，自定义，车辆初始化里程，DWORD
#define E2_CAR_ONLY_NUM_ID                      E2_PRAM_BASE_CUSTOM_ID+0x42//参数ID，自定义，唯一性编号，字符串，35字节
#define E2_DEFINE_ALARM_MASK_WORD_ID		E2_PRAM_BASE_CUSTOM_ID+0x43//参数ID，自定义，自定义报警屏蔽字，DWORD
#define E2_DEFINE_STATUS_CHANGE_PHOTO_ENABLE_ID	E2_PRAM_BASE_CUSTOM_ID+0x44//参数ID，自定义，自定义状态变化拍照使能开关，DWORD
#define E2_DEFINE_STATUS_CHANGE_PHOTO_STORE_ID	E2_PRAM_BASE_CUSTOM_ID+0x45//参数ID，自定义，自定义状态变化拍照存储开关，DWORD
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x46，最小弯道角度，自定义，EGS702用，EGS701无此项
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x47，最大弯道角度，自定义，EGS702用，EGS701无此项
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x48，弯道速度限值，自定义，EGS702用，EGS701无此项
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x49，车辆初始油耗，自定义，老的EGS701用，EGS701无此项
#define E2_SHUTDOWN_MODE_TIME_ID		E2_PRAM_BASE_CUSTOM_ID+0x4a//参数ID，自定义，关机模式时间参数，
#define E2_SHUTDOWN_MODE_POWER_ID		E2_PRAM_BASE_CUSTOM_ID+0x4b//参数ID，自定义，深度休眠电压，
#define	E2_UNLOGIN_TTS_PLAY_ID			E2_PRAM_BASE_CUSTOM_ID+0x4c//参数ID，自定义，驾驶员未签到语音提醒，0为关，1为开。配IC卡时出货设置为开，否则设置为关
#define E2_TERMINAL_TYPE_ID                     E2_PRAM_BASE_CUSTOM_ID+0x4d//参数ID，自定义，终端类型，北斗协议查询终端属性命令用到了这项
#define E2_GPS_ATTRIBUTE_ID                     E2_PRAM_BASE_CUSTOM_ID+0x4e//参数ID，自定义，GNSS属性，北斗协议查询终端属性命令用到了这项
#define E2_GPRS_ATTRIBUTE_ID                    E2_PRAM_BASE_CUSTOM_ID+0x4f//参数ID，自定义，通讯模块属性，北斗协议查询终端属性命令用到了这项
//#define	E2_OVERSPEED_PREALARM_MARGIN_ID		E2_PRAM_BASE_CUSTOM_ID+0x50//参数ID，自定义，超速预警差值,DWORD,单位0.1km/h,默认值50,与标准的0x005b重复了
#define	E2_OVERSPEED_PREALARM_KEEPTIME_ID	E2_PRAM_BASE_CUSTOM_ID+0x50//参数ID，自定义，超速预警持续时间,DWORD,单位秒,默认值3
#define	E2_OVERSPEED_PREALARM_VOICE_ID		E2_PRAM_BASE_CUSTOM_ID+0x51//参数ID，自定义，超速预警语音,STRING,最大30字节,默认值为请控制车速
#define	E2_OVERSPEED_PREALARM_GPROUP_TIME_ID	E2_PRAM_BASE_CUSTOM_ID+0x52//参数ID，自定义，超速预警每组提示时间间隔,DWORD,单位分钟,默认值5
#define	E2_OVERSPEED_PREALARM_NUMBER_ID		E2_PRAM_BASE_CUSTOM_ID+0x53//参数ID，自定义，超速预警每组提示次数,DWORD,默认值3
#define	E2_OVERSPEED_PREALARM_NUMBER_TIME_ID	E2_PRAM_BASE_CUSTOM_ID+0x54//参数ID，自定义，超速预警每次提示时间间隔,DWORD,单位秒,默认值10
//#define	E2_OVERSPEED_ALARM_VALUE_ID		E2_PRAM_BASE_CUSTOM_ID+0x56//参数ID，自定义，超速报警值，DWORD,单位km/h,默认值130,与标准的0x0055重复了
//#define	E2_OVERSPEED_ALARM_KEEPTIME_ID		E2_PRAM_BASE_CUSTOM_ID+0x57//参数ID，自定义，超速报警持续时间，DWORD,单位秒默认值10,与标准的0x0056重复了
#define	E2_OVERSPEED_ALARM_VOICE_ID		E2_PRAM_BASE_CUSTOM_ID+0x55//参数ID，自定义，超速报警语音,STRING,最大30字节,
#define	E2_OVERSPEED_ALARM_GPROUP_TIME_ID	E2_PRAM_BASE_CUSTOM_ID+0x56//参数ID，自定义，超速报警每组提示时间间隔,DWORD,单位分钟,默认值5
#define	E2_OVERSPEED_ALARM_NUMBER_ID		E2_PRAM_BASE_CUSTOM_ID+0x57//参数ID，自定义，超速报警每组提示次数,DWORD,默认值3
#define	E2_OVERSPEED_ALARM_NUMBER_TIME_ID	E2_PRAM_BASE_CUSTOM_ID+0x58//参数ID，自定义，超速报警每次提示时间间隔,DWORD,单位秒,默认值10
#define E2_OVERSPEED_ALARM_REPORT_TIME_ID	E2_PRAM_BASE_CUSTOM_ID+0x59//参数ID，自定义，超速报警位置上报时间间隔,DWORD,单位秒,默认值0
#define	E2_OVERSPEED_NIGHT_TIME_ID		E2_PRAM_BASE_CUSTOM_ID+0x5a//参数ID，自定义，夜间时间,BYTE[14],作STRING处理,默认值全0,夜间超速使用
#define	E2_NIGHT_OVERSPEED_PERCENT_ID		E2_PRAM_BASE_CUSTOM_ID+0x5b//参数ID，自定义，夜间超速报警及预警百分比,DWORD,默认值80
#define	E2_NIGHT_DRIVE_PREALARM_TIME_ID		E2_PRAM_BASE_CUSTOM_ID+0x5c//参数ID，自定义，夜间行驶预警时间,,DWORD,单位分钟,默认值10
#define	E2_NIGHT_DRIVE_PREALARM_VOICE_ID	E2_PRAM_BASE_CUSTOM_ID+0x5d//参数ID，自定义，夜间行驶预警语音,STRING,最大30字节,默认值"即将进入夜间时段"
#define	E2_NIGHT_DRIVE_PREALARM_GPOUP_TIME_ID	E2_PRAM_BASE_CUSTOM_ID+0x5e//参数ID，自定义，夜间行驶预警提示时间间隔(每组),DWORD,单位分钟,默认值5
#define	E2_NIGHT_DRIVE_PREALARM_NUMBER_ID	E2_PRAM_BASE_CUSTOM_ID+0x5f//参数ID，自定义，夜间行驶预警提示次数,,DWORD,默认值3
#define	E2_FORBID_DRIVE_NIGHT_TIME_ID		E2_PRAM_BASE_CUSTOM_ID+0x60//参数ID，自定义，停车休息(禁止行驶)夜间时间,BYTE[14],作STRING处理,默认值全0
#define	E2_FORBID_DRIVE_PREALARM_TIME_ID	E2_PRAM_BASE_CUSTOM_ID+0x61//参数ID，自定义，禁行时段到达提醒提前时间,DWORD,单位分钟,默认值30
#define	E2_FORBID_DRIVE_PREALARM_VOICE_ID	E2_PRAM_BASE_CUSTOM_ID+0x62//参数ID，自定义，禁行时段到达提醒语音,STRING,最大30字节,默认值"请按规定时间停车休息"
#define	E2_FORBID_DRIVE_PREALARM_GPOUP_TIME_ID	E2_PRAM_BASE_CUSTOM_ID+0x63//参数ID，自定义，禁行时段提醒时间间隔,DWORD,单位分钟,默认值5
#define	E2_FORBID_DRIVE_PREALARM_NUMBER_ID	E2_PRAM_BASE_CUSTOM_ID+0x64//参数ID，自定义，禁行时段到达提醒播报的次数,DWORD,默认值3
#define	E2_FORBID_DRIVE_ALARM_VOICE_ID		E2_PRAM_BASE_CUSTOM_ID+0x65//参数ID，自定义，禁行时段到达提醒语音,STRING,最大30字节,默认值"请按规定时间停车休息"
#define	E2_FORBID_DRIVE_ALARM_GPOUP_TIME_ID	E2_PRAM_BASE_CUSTOM_ID+0x66//参数ID，自定义，禁行时段提醒时间间隔,DWORD,单位分钟,默认值5
#define	E2_FORBID_DRIVE_ALARM_NUMBER_ID		E2_PRAM_BASE_CUSTOM_ID+0x67//参数ID，自定义，禁行时段到达提醒播报的次数,DWORD,默认值3
#define	E2_FORBID_DRIVE_RUN_SPEED_ID		E2_PRAM_BASE_CUSTOM_ID+0x68//参数ID，自定义，禁行时段行驶速度,单位km/h,DWORD,默认值20
#define	E2_FORBID_DRIVE_RUN_SPEED_KEEPTIME_ID	E2_PRAM_BASE_CUSTOM_ID+0x69//参数ID，自定义，禁行时段行驶速度持续时间,单位分钟,DWORD,默认值5，
										//当速度大于禁行时段行驶速度且持续时间达到禁行时段行驶速度持续时间，认定为行驶
#define	E2_TIRED_DRIVE_NIGHT_TIME_ID		E2_PRAM_BASE_CUSTOM_ID+0x6a//参数ID，自定义，夜间时间,BYTE[14],作STRING处理,默认值全0,夜间超时使用
#define	E2_NIGHT_MAX_DRVIE_TIME_ID		E2_PRAM_BASE_CUSTOM_ID+0x6b//参数ID，自定义，夜间连续驾驶时间门限,单位秒,DWORD,默认值7200
#define	E2_NIGHT_MIN_RELAX_TIME_ID		E2_PRAM_BASE_CUSTOM_ID+0x6c//参数ID，自定义，夜间最小休息时间,单位秒,DWORD,默认值1200
#define	E2_TIRED_DRIVE_ALARM_TIME_ID		E2_PRAM_BASE_CUSTOM_ID+0x6d//参数ID，自定义，超时驾驶报警提示语音有效时间,单位分钟,DWORD,默认值30,国标要求
#define	E2_TIRED_DRIVE_PREALARM_VOICE_ID	E2_PRAM_BASE_CUSTOM_ID+0x6e//参数ID，自定义，超时驾驶预警提示语音,STRING,最大30字节,默认值请勿超时驾驶
#define	E2_TIRED_DRIVE_PREALARM_GPOUP_TIME_ID	E2_PRAM_BASE_CUSTOM_ID+0x6f//参数ID，自定义，超时驾驶预警每组提示时间间隔,单位分钟,DWORD,默认值5
#define	E2_TIRED_DRIVE_PREALARM_NUMBER_ID	E2_PRAM_BASE_CUSTOM_ID+0x70//参数ID，自定义，超时驾驶预警每组提示次数,DWORD,默认值3
#define	E2_TIRED_DRIVE_PREALARM_NUMBER_TIME_ID	E2_PRAM_BASE_CUSTOM_ID+0x71//参数ID，自定义，超时驾驶预警每次提示时间间隔,单位秒,DWORD,默认值10
#define	E2_TIRED_DRIVE_ALARM_VOICE_ID		E2_PRAM_BASE_CUSTOM_ID+0x72//参数ID，自定义，超时驾驶报警提示语音,STRING,最大30字节,默认值请勿超时驾驶
#define	E2_TIRED_DRIVE_ALARM_GPOUP_TIME_ID	E2_PRAM_BASE_CUSTOM_ID+0x73//参数ID，自定义，超时驾驶报警每组提示时间间隔,单位分钟,DWORD,默认值5
#define	E2_TIRED_DRIVE_ALARM_NUMBER_ID		E2_PRAM_BASE_CUSTOM_ID+0x74//参数ID，自定义，超时驾驶报警每组提示次数,DWORD,默认值3
#define	E2_TIRED_DRIVE_ALARM_NUMBER_TIME_ID	E2_PRAM_BASE_CUSTOM_ID+0x75//参数ID，自定义，超时驾驶报警每次提示时间间隔,单位秒,DWORD,默认值10
#define	E2_TIRED_DRIVE_REPORT_OPEN_TIME_ID	E2_PRAM_BASE_CUSTOM_ID+0x76//参数ID，自定义，超时驾驶报警位置上报开启时间,单位秒,DWORD,默认值0xffffffff,不开启,九通为20分钟
#define	E2_TIRED_DRIVE_REPORT_TIME_ID		E2_PRAM_BASE_CUSTOM_ID+0x77//参数ID，自定义，超时驾驶报警位置上报时间间隔,单位秒,DWORD,默认值300,九通为5分钟
#define	E2_TIRED_DRIVE_RUN_SPEED_ID		E2_PRAM_BASE_CUSTOM_ID+0x78//参数ID，自定义，超时驾驶行驶判定速度,单位km/h,DWORD,默认值5,
#define	E2_TIRED_DRIVE_RUN_KEEPTIME_ID		E2_PRAM_BASE_CUSTOM_ID+0x79//参数ID，自定义，超时驾驶行驶判定持续时间,单位秒,DWORD,默认值10,
#define	E2_ROUTE_EXCURSION_ALARM_NUMBER_ID      E2_PRAM_BASE_CUSTOM_ID+0x7A//参数ID，自定义，偏离线路 报警次数，0：不播报，255：持续播报 DWORD,默认值3
#define	E2_ROUTE_EXCURSION_ALARM_GPOUP_TIME_ID  E2_PRAM_BASE_CUSTOM_ID+0x7B//参数ID，自定义，偏离线路报警语音时间间隔,DWORD,默认值5
#define	E2_ROUTE_EXCURSION_ALARM_VOICE_ID       E2_PRAM_BASE_CUSTOM_ID+0x7C//参数ID，自定义，偏离线路报警语音,STRING,最大30字节,默认值请你按规定线路行驶
#define	E2_REPORT_FREQ_EVENT_SWITCH_ID          E2_PRAM_BASE_CUSTOM_ID+0x7D//参数ID，自定义，事件项，保留，用于指定产生事件后上报时间间隔的事件项 BIT0位＝1：超速报警,BIT1位＝1：超时报警,DWORD
#define	E2_RUN_FORBID_CALL_SWITCH_ID            E2_PRAM_BASE_CUSTOM_ID+0x7E//参数ID，自定义，行驶中禁止电话功能开关，0允许通话；1禁止通话，通用版默认为0，福建电信版默认为1
#define	E2_SIM_ICCID_NUMBER_ID                  E2_PRAM_BASE_CUSTOM_ID+0x7F//参数ID，自定义，SIM卡的ICCID号码,BCD[10],只读
#define	E2_LED_PORT_ID                          E2_PRAM_BASE_CUSTOM_ID+0x80//参数ID，自定义，表明32pin的LED接口接的外设类型,
                                                                   //2字节，bit0:条屏bit1:顶灯bit2:评价器bit3:载重传感器bit4-bit15:保留;相应位为1表示该端口接了相应外设。
#define	E2_POS_PORT_ID                          E2_PRAM_BASE_CUSTOM_ID+0x81//参数ID，自定义，表明16pin接口接的外设类型,
						                                            //2字节，bit0:留用,bit1:留用,bit2:久通机电油位传感器;bit3:博实结油位传感器;bit4-bit15:保留;相应位为1表示该端口接了相应外设。

#define E2_SECOND_MAIN_SERVER_IP_ID 			E2_PRAM_BASE_CUSTOM_ID+0x84//参数ID，自定义，第2个连接主服务器IP地址或域名
                                                                            //0xF281-0xF282为EGS702使用,0xF283为E-MIV404使用
#define E2_SECOND_MAIN_SERVER_TCP_PORT_ID 		E2_PRAM_BASE_CUSTOM_ID+0x85//参数ID，自定义，第2个连接主服务器TCP端口
#define E2_SECOND_MAIN_SERVER_UDP_PORT_ID 		E2_PRAM_BASE_CUSTOM_ID+0x86//参数ID，自定义，第2个连接主服务器UDP端口
#define E2_SECOND_BACKUP_SERVER_IP_ID 			E2_PRAM_BASE_CUSTOM_ID+0x87//参数ID，自定义，第2个连接备份服务器地址//IP或域名
#define E2_SECOND_LOGINGPRS_PASSWORD_ID		    E2_PRAM_BASE_CUSTOM_ID+0x88//参数ID，自定义，第2个连接鉴权码
#define E2_SECOND_ACC_OFF_REPORT_TIME_ID        E2_PRAM_BASE_CUSTOM_ID+0x89//参数ID，自定义，第2个连接休眠汇报时间间隔,单位秒,DWORD
#define E2_SECOND_ACC_ON_REPORT_TIME_ID        E2_PRAM_BASE_CUSTOM_ID+0x8a//参数ID，自定义，第2个连接缺省汇报时间间隔,单位秒,DWORD

#define E2_ENGINE_NUM_ID       				 E2_PRAM_BASE_CUSTOM_ID+0x9D//参数ID，自定义，发动机号码，最大20个字节


/////////自定义项最大ID为最后的ID加1,请根据最后一项人工修改/////////////////////////////////////
#define	E2_SELF_DEFINE_ID_MAX			(E2_ENGINE_NUM_ID+1)//自定义项最大ID,注意：每次新增参数时需要将最后的参数ID放在这

//预留一个ID给IC卡类型，有雄帝卡，自制接触式卡，自制非接触式卡

//#define	

//***************************************参数保存的eeprom地址*******************************
//#define EEPROM_PRAM_BASE_ADDR	0x420
#define EEPROM_FIRST_USE_FLAG_ADDR	0//eeprom第一次使用标志位存储地址
#define EEPROM_PRAM_BASE_ADDR	        10
#define E2_PRAM_START_ADDR    EEPROM_PRAM_BASE_ADDR
#define E2_PRAM_END_ADDR      (EEPROM_PRAM_BASE_ADDR+1610+10)
//参数的起始地址

#define E2_TERMINAL_HEARTBEAT_ADDR		EEPROM_PRAM_BASE_ADDR		
//参数ID0x0001，终端心跳发送间隔，单位秒，默认值60

#define E2_TCP_ACK_OVERTIME_ADDR		(EEPROM_PRAM_BASE_ADDR+5)			
//参数ID0x0002，TCP消息应答超时时间，单位秒，默认值10

#define E2_TCP_RESEND_TIMES_ADDR		(EEPROM_PRAM_BASE_ADDR+10)				
//参数ID0x0003，TCP消息重传次数，默认值3

#define E2_UDP_ACK_OVERTIME_ADDR		(EEPROM_PRAM_BASE_ADDR+15)			
//参数ID0x0004，UDP消息应答超时时间，单位秒，默认值10

#define E2_UDP_RESEND_TIMES_ADDR		(EEPROM_PRAM_BASE_ADDR+20)			
//参数ID0x0005，UDP消息重传次数，默认值3

#define E2_SMS_ACK_OVERTIME_ADDR		(EEPROM_PRAM_BASE_ADDR+25)				
//参数ID0x0006，SMS消息应答超时时间，单位秒，默认值30

#define E2_SMS_RESEND_TIMES_ADDR		(EEPROM_PRAM_BASE_ADDR+30)			
//参数ID0x0007，SMS消息重传次数，默认值3

#define E2_MAIN_SERVER_APN_ADDR			(EEPROM_PRAM_BASE_ADDR+35)			
//参数ID0x0010，主服务器APN

#define E2_MAIN_SERVER_DIAL_NAME_ADDR		(EEPROM_PRAM_BASE_ADDR+60)			
//参数ID0x0011，主服务器无线通信拨号用户名

#define E2_MAIN_SERVER_DIAL_PASSWORD_ADDR	(EEPROM_PRAM_BASE_ADDR+85)			
//参数ID0x0012，主服务器无线通信拨号密码

#define E2_MAIN_SERVER_IP_ADDR			(EEPROM_PRAM_BASE_ADDR+110)		
//参数ID0x0013，主服务器地址，IP或域名

#define E2_BACKUP_SERVER_APN_ADDR		(EEPROM_PRAM_BASE_ADDR+135)				
//参数ID0x0014，备份服务器APN，无线通信拨号访问点

#define E2_BACKUP_SERVER_DIAL_NAME_ADDR		(EEPROM_PRAM_BASE_ADDR+160)	
//参数ID0x0015，备份服务器无线通信拨号用户名

#define E2_BACKUP_SERVER_DIAL_PASSWORD_ADDR	(EEPROM_PRAM_BASE_ADDR+185)		
//参数ID0x0016，备份服务器无线通信拨号密码

#define E2_BACKUP_SERVER_IP_ADDR		(EEPROM_PRAM_BASE_ADDR+210)			
//参数ID0x0017，备份服务器地址，IP或域名

#define E2_MAIN_SERVER_TCP_PORT_ADDR		(EEPROM_PRAM_BASE_ADDR+235)				
//参数ID0x0018，主服务器TCP端口
	
#define E2_MAIN_SERVER_UDP_PORT_ADDR		(EEPROM_PRAM_BASE_ADDR+240)	
//参数ID0x0019，主服务器UDP端口

#define E2_BACKUP_SERVER_TCP_PORT_ADDR		(EEPROM_PRAM_BASE_ADDR+245)	
//参数ID0x001a，备份服务器TCP端口

#define E2_BACKUP_SERVER_UDP_PORT_ADDR		(EEPROM_PRAM_BASE_ADDR+250)		
//参数ID0x001b，备份服务器UDP端口

#define E2_REGISTER_SMS_CENTER_NUM_ADDR		(EEPROM_PRAM_BASE_ADDR+255)
//参数ID0x001C，注册短信中心号码

#define E2_PRIVILEGE_SMS_CENTER_NUM_ADDR	(EEPROM_PRAM_BASE_ADDR+280)		
//参数ID0x001D，控制短信号码（特权）

#define E2_POSITION_REPORT_STRATEGY_ADDR	(EEPROM_PRAM_BASE_ADDR+305)	
//参数ID0x0020，位置汇报策略

#define E2_POSITION_REPORT_SCHEME_ADDR		(EEPROM_PRAM_BASE_ADDR+310)		
//参数ID0x0021，位置汇报方案

#define E2_DRIVER_UNLOGIN_REPORT_TIME_ADDR	(EEPROM_PRAM_BASE_ADDR+315)		
//参数ID0x0022，驾驶员未登录汇报时间间隔，单位为秒，默认值为60

#define E2_SLEEP_REPORT_TIME_ADDR		(EEPROM_PRAM_BASE_ADDR+320)			
//参数ID0x0027，休眠时汇报时间间隔，单位为秒，默认值为60

#define E2_EMERGENCY_REPORT_TIME_ADDR		(EEPROM_PRAM_BASE_ADDR+325)		
//参数ID0x0028，紧急报警时汇报时间间隔，单位为秒，默认值为10

#define	E2_ACCON_REPORT_TIME_ADDR		(EEPROM_PRAM_BASE_ADDR+330)				
//0x0029缺省时间汇报间隔,单位为秒

#define E2_ACCON_REPORT_DISTANCE_ADDR	(EEPROM_PRAM_BASE_ADDR+335)		
//0x002C缺省距离汇报间隔,单位为米

#define E2_DRIVER_UNLOGIN_REPORT_DISTANCE_ADDR 	(EEPROM_PRAM_BASE_ADDR+340)
//0x002D驾驶员未登录汇报距离间隔//单位为米//默认值为100

#define E2_SLEEP_REPORT_DISTANCE_ADDR		(EEPROM_PRAM_BASE_ADDR+345)	
//参数ID0x002e，休眠时汇报距离间隔，单位为米，默认值为1000

#define E2_EMERGENCY_REPORT_DISTANCE_ADDR	(EEPROM_PRAM_BASE_ADDR+350)	
//参数ID0x002f，紧急报警时汇报距离间隔，单位为米，默认值为100

#define E2_CORNER_REPORT_ADDR		(EEPROM_PRAM_BASE_ADDR+355)	
//参数ID0x0030，拐点补传角度,单位度/秒，默认值为15

#define E2_MONITOR_SERVER_PHONE_ADDR		(EEPROM_PRAM_BASE_ADDR+360)			
//参数ID0x0040，监控平台电话号码

#define E2_TERMINAL_RESET_PHONE_ADDR		(EEPROM_PRAM_BASE_ADDR+385)				
//参数ID0x0041，终端复位电话号码

#define E2_TERMINAL_DEFAULT_SET_PHONE_ADDR	(EEPROM_PRAM_BASE_ADDR+410)	
//参数ID0x0042，恢复出厂设置电话号码

#define E2_MONITOR_SERVER_SMS_PHONE_ADDR	(EEPROM_PRAM_BASE_ADDR+435)	
//参数ID0x0043，监控平台SMS电话号码

#define E2_SMS_TEXT_ALARM_PHONE_ADDR		(EEPROM_PRAM_BASE_ADDR+460)		
//参数ID0x0044，SMS文本报警电话号码

#define E2_TERMINAL_GET_PHONE_STRATEGY_ADDR	(EEPROM_PRAM_BASE_ADDR+485)	
//参数ID0x0045，终端电话接听策略

#define E2_MAX_PHONE_TIME_EACH_ADDR		(EEPROM_PRAM_BASE_ADDR+490)			
//参数ID0x0046，每次最长通话时间，单位为秒，默认为0xffffffff，不限制

#define E2_MAX_PHONE_TIME_MONTH_ADDR		(EEPROM_PRAM_BASE_ADDR+495)		
//参数ID0x0047，每月最长通话时间，单位为秒，默认为0xffffffff，不限制

#define E2_MONITOR_PHONE_ADDR			(EEPROM_PRAM_BASE_ADDR+500)			
//参数ID0x0048，监听电话号码

#define E2_MONITOR_SERVER_PRIVILEGE_SMS_ADDR	(EEPROM_PRAM_BASE_ADDR+525)	
//参数ID0x0049，监管平台特权短信号码

#define E2_ALARM_MASK_ADDR			(EEPROM_PRAM_BASE_ADDR+550)		
//参数ID0x0050，报警屏蔽字，默认为全开

#define E2_ALARM_TEXT_SWITCH_ADDR		(EEPROM_PRAM_BASE_ADDR+555)				
//参数ID0x0051，报警发送文本SMS开关，默认为全关

#define E2_ALARM_PHOTO_SWITCH_ADDR		(EEPROM_PRAM_BASE_ADDR+560)			
//参数ID0x0052，报警拍摄开关，默认为全关

#define E2_ALARM_PHOTO_STORE_FLAG_ADDR		(EEPROM_PRAM_BASE_ADDR+565)	
//参数ID0x0053，报警拍摄存储标志，默认为全关

#define E2_KEY_FLAG_ADDR			(EEPROM_PRAM_BASE_ADDR+570)			
//参数ID0x0054，关键标志

#define E2_MAX_SPEED_ADDR			(EEPROM_PRAM_BASE_ADDR+575)				
//参数ID0x0055，最高速度

#define E2_OVER_SPEED_KEEP_TIME_ADDR		(EEPROM_PRAM_BASE_ADDR+580)			
//参数ID0x0056，超速持续时间，单位为秒，默认为10秒

#define E2_MAX_DRVIE_TIME_ADDR			(EEPROM_PRAM_BASE_ADDR+585)		
//参数ID0x0057，连续驾驶时间门限，单位为秒，默认为240秒

#define E2_MAX_DRVIE_TIME_ONEDAY_ADDR		(EEPROM_PRAM_BASE_ADDR+590)		
//参数ID0x0058，当天累计驾驶时间门限，单位为秒，默认为0

#define E2_MIN_RELAX_TIME_ADDR			(EEPROM_PRAM_BASE_ADDR+595)		
//参数ID0x0059，最小休息时间，单位为秒，默认为1200

#define E2_MAX_STOP_CAR_TIME_ADDR		(EEPROM_PRAM_BASE_ADDR+600)			
//参数ID0x005A，最长停车时间，单位为秒，默认为0

#define E2_PICTURE_QUALITY_ADDR			(EEPROM_PRAM_BASE_ADDR+605)		
//参数ID0x0070，图像/视频质量，1~10,1为最好

#define E2_PICTURE_BRIGHTNESS_ADDR		(EEPROM_PRAM_BASE_ADDR+610)		
//参数ID0x0071，亮度，0~255

#define E2_PICTURE_CONTRAST_ADDR		(EEPROM_PRAM_BASE_ADDR+615)			
//参数ID0x0072，对比度，0~127

#define E2_PICTURE_SATURATION_ADDR		(EEPROM_PRAM_BASE_ADDR+620)			
//参数ID0x0073，饱和度，0~127

#define E2_PICTURE_CHROMA_ADDR			(EEPROM_PRAM_BASE_ADDR+625)		
//参数ID0x0074，色度，0~255

#define E2_CAR_TOTAL_MILE_ADDR			(EEPROM_PRAM_BASE_ADDR+630)		
//参数ID0x0080，车辆里程表读数，0.1km

#define E2_CAR_PROVINCE_ID_ADDR			(EEPROM_PRAM_BASE_ADDR+635)		
//参数ID0x0081，车辆所在的省域ID

#define E2_CAR_CITY_ID_ADDR			(EEPROM_PRAM_BASE_ADDR+640)			
//参数ID0x0082，车辆所在的市域ID

#define E2_CAR_PLATE_NUM_ADDR			(EEPROM_PRAM_BASE_ADDR+645)			
//参数ID0x0083，公安交通管理部门颁发的机动车号牌

#define E2_CAR_PLATE_COLOR_ADDR			(EEPROM_PRAM_BASE_ADDR+670)	
//参数ID0x0084，车牌颜色，按照JT/T415-2006的5.4.12
	

//*************************自定义参数ID地址************************
#define E2_LOGINGPRS_PASSWORD_ID_ADDR		(EEPROM_PRAM_BASE_ADDR+675)//25
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x00，自定义，鉴权码
#define E2_REGISTER_SMS_PHONE_ID_ADDR		(EEPROM_PRAM_BASE_ADDR+700)//25
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x01，自定义，注册短消息号码
#define E2_PROGRAM_UPDATA_REQUEST_ID_ADDR	(EEPROM_PRAM_BASE_ADDR+725)//5
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x02，自定义，程序升级请求标志
#define E2_PROGRAM_UPDATA_VERIFYCODE_ID_ADDR	(EEPROM_PRAM_BASE_ADDR+730)//5
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x03，自定义，程序升级校验码
#define E2_PROGRAM_UPDATA_LENGTH_ID_ADDR	(EEPROM_PRAM_BASE_ADDR+735)//5
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x04，自定义，程序升级长度
#define E2_MANUFACTURE_ID_ADDR			(EEPROM_PRAM_BASE_ADDR+740)//10
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x05，自定义，制造商ID，5字节		
#define E2_DEVICE_ID_ADDR			(EEPROM_PRAM_BASE_ADDR+750)//10	
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x06，自定义，厂商终端ID，7字节
#define E2_DEVICE_PHONE_ID_ADDR			(EEPROM_PRAM_BASE_ADDR+760)//20
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x07，自定义，终端手机号，6字节
#define	E2_CAR_IDENTIFICATION_CODE_ID_ADDR	(EEPROM_PRAM_BASE_ADDR+780)//25
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x08，自定义，车辆识别码
#define	E2_CAR_TYPE_ID_ADDR			(EEPROM_PRAM_BASE_ADDR+805)//25
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x09，自定义，车牌分类
#define	E2_CURRENT_DRIVER_ID_ADDR		(EEPROM_PRAM_BASE_ADDR+830)//10
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x0A，自定义，当前驾驶员代码
#define	E2_CURRENT_LICENSE_ID_ADDR		(EEPROM_PRAM_BASE_ADDR+840)//25
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x0B，自定义，当前驾驶证号码
#define	E2_FIRMWARE_VERSION_ID_ADDR		(EEPROM_PRAM_BASE_ADDR+865)//10
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x0C，自定义，固件版本号
#define E2_ALL_DRIVER_INFORMATION_ID_ADDR	(EEPROM_PRAM_BASE_ADDR+875)//150-->注意：1015+10----1119+10空闲
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x0D，自定义，所有驾驶员信息
#define	E2_SPEED_SENSOR_COEF_ID_ADDR		(EEPROM_PRAM_BASE_ADDR+1130)//5
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x0E，自定义，速度传感器系数
#define	E2_CAR_FEATURE_COEF_ID_ADDR		(EEPROM_PRAM_BASE_ADDR+1135)//5
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x0F，自定义，车辆特征系数
#define E2_SPEED_SELECT_ID_ADDR			(EEPROM_PRAM_BASE_ADDR+1140)//5
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x10，自定义，速度选择开关
#define E2_BACKLIGHT_SELECT_ID_ADDR		(EEPROM_PRAM_BASE_ADDR+1145)//5
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x11，自定义，背光选择开关
#define E2_FIRST_POWERON_FLAG_ID_ADDR		(EEPROM_PRAM_BASE_ADDR+1150)//5
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x12，自定义，系统第1次上电标志
#define E2_ACC_ON_INTERVAL_ID_ADDR			(EEPROM_PRAM_BASE_ADDR+1155)//5
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x13，自定义，ACC ON 时汇报时间间隔
#define E2_ACC_ON_DISTANCE_ID_ADDR			(EEPROM_PRAM_BASE_ADDR+1160)//5
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x14，自定义，ACC ON 时汇报距离间隔
//(EEPROM_PRAM_BASE_ADDR+1145)				
#define	E2_UPDATA_LINK_CONTROL_ID_ADDR			(EEPROM_PRAM_BASE_ADDR+1165)//5
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x15，自定义，连接控制
#define	E2_UPDATA_DIAL_NAME_ID_ADDR			(EEPROM_PRAM_BASE_ADDR+1170)//5
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x16，自定义，无限升级拨号点名称
#define	E2_UPDATA_DIAL_USER_ID_ADDR			(EEPROM_PRAM_BASE_ADDR+1195)//25
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x17，自定义，无限升级拨号用户名
#define	E2_UPDATA_DIAL_PASSWORD_ID_ADDR			(EEPROM_PRAM_BASE_ADDR+1220)//25
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x18，自定义，无限升级拨号密码
#define	E2_UPDATA_IP_ID_ADDR				(EEPROM_PRAM_BASE_ADDR+1245)	//25	
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x19，自定义，无限升级IP地址
#define	E2_UPDATA_TCP_PORT_ID_ADDR			(EEPROM_PRAM_BASE_ADDR+1270)//25		
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x1a，自定义，无限升级TCP端口
#define	E2_UPDATA_UDP_PORT_ID_ADDR			(EEPROM_PRAM_BASE_ADDR+1275)	//5	
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x1b，自定义，无限升级UDP端口
#define	E2_UPDATA_MONITOR_SERVER_CODE_ID_ADDR		(EEPROM_PRAM_BASE_ADDR+1280)//25
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x1c，自定义，无限升级监管平台鉴权码
#define	E2_UPDATA_HARDWARE_VERSION_ID_ADDR		(EEPROM_PRAM_BASE_ADDR+1305)	//25
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x1d，自定义，无限升级硬件版本
#define	E2_UPDATA_FIRMWARE_VERSION_ID_ADDR		(EEPROM_PRAM_BASE_ADDR+1330)	//25
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x1e，自定义，无限升级固件版本
#define	E2_UPDATA_URL_ID_ADDR				(EEPROM_PRAM_BASE_ADDR+1355)	//100--->注意EEPROM_PRAM_BASE_ADDR+1445+10--EEPROM_PRAM_BASE_ADDR+1545+10为其它用途
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x1f，自定义，无限升级URL完整地址
#define E2_UPDATA_TIME_ID_ADDR				(EEPROM_PRAM_BASE_ADDR+1555)	//5
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x20，自定义，无限升级连接到指定服务器时限
#define E2_HOT_PHONE_ID_ADDR				(EEPROM_PRAM_BASE_ADDR+1560)	//25			
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x21，自定义，热线电话
#define E2_UPDATA_MANUFACTURE_ID_ADDR			(EEPROM_PRAM_BASE_ADDR+1585)//10
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x22，自定义，无限升级制造商ID
#define	E2_PHONE_CALLBACK_ID_ADDR			(EEPROM_PRAM_BASE_ADDR+1595)	//25	
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x23，自定义，电话回拨号码
#define	E2_CALLBACK_FLAG_ID_ADDR			(EEPROM_PRAM_BASE_ADDR+1620)//5
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x24，自定义，电话回拨标志,1为监听;0为普通通话
#define	E2_DEVICE_PHONE_BACKUP_ID_ADDR			(EEPROM_PRAM_BASE_ADDR+1625)//10
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x25，自定义，终端手机号，备份
#define	E2_LAST_LOCATION_ID_ADDR			(EEPROM_PRAM_BASE_ADDR+1635)//20
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x26，上一个定位位置
#define E2_PHOTO_SPACE_ID_ADDR                          (EEPROM_PRAM_BASE_ADDR+1655)//5
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x27，自定义，拍照时间间隔
#define E2_PHOTO_NUM_ID_ADDR                            (EEPROM_PRAM_BASE_ADDR+1660)//5                       
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x28，自定义，拍照张数
#define E2_DOOR_SET_ID_ADDR                             (EEPROM_PRAM_BASE_ADDR+1665)//5                       
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x29，自定义，拍照张数
#define E2_DEVICE_BACKUP_ID_ADDR			(EEPROM_PRAM_BASE_ADDR+1670)//10	
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x2a，自定义，厂商终端ID，7字节
#define E2_CDMA_SLEEP_ID_ADDR				(EEPROM_PRAM_BASE_ADDR+1680)//5
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x2b，自定义，CDMA休眠开关，1为开，非1为关，默认为关
#define E2_PASSWORD_ID_ADDR				(EEPROM_PRAM_BASE_ADDR+1685)//5
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x2c，自定义，维护菜单密码
#define E2_ELECTRON_RADIUS_ADDR				(EEPROM_PRAM_BASE_ADDR+1690)//5
//参数ID0x0031，电子围栏，标准补遗增加
#define E2_INSTALL_TIME_ID_ADDR				(EEPROM_PRAM_BASE_ADDR+1695)//5		
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x2d，自定义，初始化安装日期
#define E2_INSTALL_TIME_FLAG_ID_ADDR			(EEPROM_PRAM_BASE_ADDR+1700)//5		
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x2e，自定义，初始化安装日期
#define E2_HORN_SET_ID_ADDR				(EEPROM_PRAM_BASE_ADDR+1705)//5		
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x2f，自定义，车喇叭（输入）选择，1为低有效，非1为高有效。默认为高有效
#define E2_GPS_CONTROL_ID_ADDR				(EEPROM_PRAM_BASE_ADDR+1710)//5	
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x30，自定义，ACC OFF时GPS模块控制,0为关,1为打开
#define	E2_USB_UPDATA_FLAG_ID_ADDR			(EEPROM_PRAM_BASE_ADDR+1715)//5 
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x31，自定义，USB升级标志
#define	E2_TAXIMETER_CONTROL_ID_ADDR			(EEPROM_PRAM_BASE_ADDR+1720)//5
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x32，自定义，计价器功能开关,0为关闭,1为开启
#define E2_TOPLIGHT_CONTROL_ID_ADDR			(EEPROM_PRAM_BASE_ADDR+1725)//5
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x33，自定义，顶灯功能开关,0为关闭,1为开启，不使用
#define E2_LEDLIGHT_CONTROL_ID_ADDR			(EEPROM_PRAM_BASE_ADDR+1730)//5
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x34，自定义，LED广告功能开关,0为关闭,1为开启，不使用
#define E2_HB_IC_DRIVER_ID_ADDR				(EEPROM_PRAM_BASE_ADDR+1735)//120
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x35，自定义，设置符合行标标准格式（0x0702指令）的驾驶员信息
#define E2_STATUS_PHOTO_SWITCH_ID_ADDR 			(EEPROM_PRAM_BASE_ADDR+1855)//5
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x36，自定义，状态拍摄开关
#define E2_STATUS_PHOTO_STORE_FLAG_ID_ADDR 		(EEPROM_PRAM_BASE_ADDR+1860)//5
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x37，自定义，状态拍摄存储标志//默认为全关
#define	E2_RESUME_NOUSED_ID_ADDR			(EEPROM_PRAM_BASE_ADDR+1865)//5
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x38，自定义，恢复终端为未使用状态
#define	E2_NIGHT_TIME0_ID_ADDR				(EEPROM_PRAM_BASE_ADDR+1870)//5		
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x39，自定义，夜间时间范围,BCD码
#define	E2_NIGHT_MAX_DRVIE_TIME0_ID_ADDR			(EEPROM_PRAM_BASE_ADDR+1875)//5		
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x3a，自定义，夜间超时驾驶门限
#define	E2_NIGHT_MAX_SPEED0_ID_ADDR			(EEPROM_PRAM_BASE_ADDR+1880)//5		
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x3b，自定义，夜间限速值

#define E2_IC_MAIN_SERVER_IP_ADDR		        (EEPROM_PRAM_BASE_ADDR+1890)//41	
//参数ID0x001A//IC卡认证主服务器IP地址或域名
#define E2_IC_MAIN_SERVER_TCP_PORT_ADDR		        (EEPROM_PRAM_BASE_ADDR+1931)//5	
//参数ID0x001B//IC卡认证主服务器TCP端口
#define E2_IC_MAIN_SERVER_UDP_PORT_ADDR		        (EEPROM_PRAM_BASE_ADDR+1936)//5	
//参数ID0x001C//IC卡认证主服务器UDP端口
#define E2_IC_BACKUP_SERVER_IP_ADDR		        (EEPROM_PRAM_BASE_ADDR+1941)//41
//参数ID0x001D//IC卡认证备份服务器IP地址或域名
#define E2_SPEED_EARLY_ALARM_DVALUE_ADDR		(EEPROM_PRAM_BASE_ADDR+1982)//3
//参数ID0x005B//超速报警预警差值，单位为1/10Km/h，默认为50
#define E2_TIRE_EARLY_ALARM_DVALUE_ADDR		        (EEPROM_PRAM_BASE_ADDR+1985)//3
//参数ID0x005C//疲劳驾驶预警差值，单位为秒（s），>0，默认为30
#define E2_SET_CRASH_ALARM_ADDR		                (EEPROM_PRAM_BASE_ADDR+1988)//3
//参数ID0x005D//碰撞报警参数设置 
#define E2_SET_ROLLOVER_ALARM_ADDR		        (EEPROM_PRAM_BASE_ADDR+1991)//3
//参数ID0x005E//侧翻报警参数设置：侧翻角度，单位1 度，默认为30 度
#define E2_TIME_PHOTO_CONTROL_ADDR		        (EEPROM_PRAM_BASE_ADDR+1994)//5
//参数ID0x0064//定时拍照控制
#define E2_MILE_PHOTO_CONTROL_ADDR		        (EEPROM_PRAM_BASE_ADDR+1999)//5
//参数ID0x0065//定距拍照控制
#define E2_GPS_SET_MODE_ADDR		                (EEPROM_PRAM_BASE_ADDR+2004)//2
//参数ID0x0090//GNSS 定位模式，默认为E2_PRAM_BASE_CUSTOM_ID+0x 北斗定位
#define E2_GPS_SET_BAUD_ADDR		                (EEPROM_PRAM_BASE_ADDR+2006)//2
//参数ID0x0091//GNSS 波特率，默认为0x01 9600
#define E2_GPS_SET_OUTPUT_RATE_ADDR		        (EEPROM_PRAM_BASE_ADDR+2008)//2
//参数ID0x0092//GNSS 模块详细定位数据输出频率,默认为0x01 1000ms
#define E2_GPS_SET_GATHER_RATE_ADDR		        (EEPROM_PRAM_BASE_ADDR+2010)//5
//参数ID0x0093//GNSS 模块详细定位数据采集频率，单位为秒，默认为1
#define E2_GPS_SET_UPLOAD_MODE_ADDR		        (EEPROM_PRAM_BASE_ADDR+2015)//2
//参数ID0x0094//GNSS 模块详细定位数据上传方式
#define E2_GPS_SET_UPLOAD_VALUE_ADDR		        (EEPROM_PRAM_BASE_ADDR+2017)//5
//参数ID0x0095//GNSS 模块详细定位数据上传设置,默认为0

#define E2_CAN1_GATHER_TIME_ADDR		        (EEPROM_PRAM_BASE_ADDR+2022)//5
//参数ID0x0100//CAN 总线通道1 采集时间间隔(ms)，0 表示不采集
#define E2_CAN1_UPLOAD_TIME_ADDR		        (EEPROM_PRAM_BASE_ADDR+2027)//3
//参数ID0x0101//CAN 总线通道1 上传时间间隔(s)，0 表示不上传
#define E2_CAN2_GATHER_TIME_ADDR		        (EEPROM_PRAM_BASE_ADDR+2030)//5
//参数ID0x0102//CAN 总线通道2 采集时间间隔(ms)，0 表示不采集
#define E2_CAN2_UPLOAD_TIME_ADDR		        (EEPROM_PRAM_BASE_ADDR+2035)//3
//参数ID0x0103//CAN 总线通道2 上传时间间隔(s)，0 表示不上传
#define E2_CAN_SET_ONLY_GATHER_0_ADDR		        (EEPROM_PRAM_BASE_ADDR+2038)//10=长度+数据+校验
//参数ID0x0110//CAN 总线ID 单独采集设置0 BYTE[8]
#define E2_CAN_SET_ONLY_GATHER_1_ADDR		        (EEPROM_PRAM_BASE_ADDR+2048)//10=长度+数据+校验
//参数ID0x0111//CAN 总线ID 单独采集设置1 BYTE[8]
#define E2_CAR_INIT_MILE_ID_ADDR                    	(EEPROM_PRAM_BASE_ADDR+2058)//5
//参数ID E2_PRAM_BASE_CUSTOM_ID+0x41，自定义，车辆初始化里程，DWORD
#define E2_CAR_ONLY_NUM_ID_ADDR                      	(EEPROM_PRAM_BASE_ADDR+2063)//37=长度+数据+校验
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x42，自定义，唯一性编号，字符串，35字节
#define E2_DEFINE_ALARM_MASK_WORD_ID_ADDR		(EEPROM_PRAM_BASE_ADDR+2100)//5
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x43，自定义，自定义报警屏蔽字，DWORD
#define E2_DEFINE_STATUS_CHANGE_PHOTO_ENABLE_ID_ADDR	(EEPROM_PRAM_BASE_ADDR+2105)//5
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x44，自定义，自定义状态变化拍照使能开关，DWORD
#define E2_DEFINE_STATUS_CHANGE_PHOTO_STORE_ID_ADDR	(EEPROM_PRAM_BASE_ADDR+2110)//5
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x45，自定义，自定义状态变化拍照存储开关，DWORD
#define	E2_UNLOGIN_TTS_PLAY_ID_ADDR			(EEPROM_PRAM_BASE_ADDR+2115)//2
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x4c，自定义，驾驶员未签到语音提醒，0为关，1为开。配IC卡时出货设置为开，否则设置为关
#define E2_TERMINAL_TYPE_ID_ADDR		                (EEPROM_PRAM_BASE_ADDR+2117)//3
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x4d，自定义，终端类型
#define E2_GPS_ATTRIBUTE_ID_ADDR		                (EEPROM_PRAM_BASE_ADDR+2120)//2
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x4e，自定义，GNSS属性
#define E2_GPRS_ATTRIBUTE_ID_ADDR		                (EEPROM_PRAM_BASE_ADDR+2122)//2
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x4f，自定义，通讯模块属性
//#define	E2_OVERSPEED_PREALARM_MARGIN_ID_ADDR		(EEPROM_PRAM_BASE_ADDR+2124)//5
//参数ID，自定义，超速预警差值,DWORD,单位0.1km/h,默认值50
#define	E2_OVERSPEED_PREALARM_KEEPTIME_ID_ADDR		(EEPROM_PRAM_BASE_ADDR+2129)//5
//参数ID，自定义，超速预警持续时间,DWORD,单位秒,默认值3
#define	E2_OVERSPEED_PREALARM_VOICE_ID_ADDR		(EEPROM_PRAM_BASE_ADDR+2134)//31
//参数ID，自定义，超速预警语音,STRING,最大30字节
#define	E2_OVERSPEED_PREALARM_GPROUP_TIME_ID_ADDR	(EEPROM_PRAM_BASE_ADDR+2165)//5
//参数ID，自定义，超速预警每组提示时间间隔,DWORD,单位分钟,默认值5
#define	E2_OVERSPEED_PREALARM_NUMBER_ID_ADDR		(EEPROM_PRAM_BASE_ADDR+2170)//5
//参数ID，自定义，超速预警每组提示次数,DWORD,默认值3
#define	E2_OVERSPEED_PREALARM_NUMBER_TIME_ID_ADDR	(EEPROM_PRAM_BASE_ADDR+2175)//5
//参数ID，自定义，超速预警每次提示时间间隔,DWORD,单位秒,默认值10
//#define	E2_OVERSPEED_ALARM_VALUE_ID_ADDR		(EEPROM_PRAM_BASE_ADDR+2180)//5
//参数ID，自定义，超速报警值，DWORD,单位km/h,默认值130
//#define	E2_OVERSPEED_ALARM_KEEPTIME_ID_ADDR		(EEPROM_PRAM_BASE_ADDR+2185)//5
//参数ID，自定义，超速报警持续时间，DWORD,单位秒默认值10
#define	E2_OVERSPEED_ALARM_VOICE_ID_ADDR		(EEPROM_PRAM_BASE_ADDR+2190)//32
//参数ID，自定义，超速报警语音,STRING,最大30字节
#define	E2_OVERSPEED_ALARM_GPROUP_TIME_ID_ADDR		(EEPROM_PRAM_BASE_ADDR+2222)//5
//参数ID，自定义，超速报警每组提示时间间隔,DWORD,单位分钟,默认值5
#define	E2_OVERSPEED_ALARM_NUMBER_ID_ADDR		(EEPROM_PRAM_BASE_ADDR+2227)//5
//参数ID，自定义，超速报警每组提示次数,DWORD,默认值3
#define	E2_OVERSPEED_ALARM_NUMBER_TIME_ID_ADDR		(EEPROM_PRAM_BASE_ADDR+2232)//5
//参数ID，自定义，超速报警每次提示时间间隔,DWORD,单位秒,默认值10
#define E2_OVERSPEED_ALARM_REPORT_TIME_ID_ADDR		(EEPROM_PRAM_BASE_ADDR+2237)//5
//参数ID，自定义，超速报警位置上报时间间隔,DWORD,单位秒,默认值0
#define	E2_OVERSPEED_NIGHT_TIME_ID_ADDR			(EEPROM_PRAM_BASE_ADDR+2242)//16
//参数ID，自定义，夜间时间,BYTE[14],作STRING处理,默认值全0
#define	E2_NIGHT_OVERSPEED_PERCENT_ID_ADDR		(EEPROM_PRAM_BASE_ADDR+2258)//5
//参数ID，自定义，夜间超速报警及预警百分比,DWORD,默认值80
#define	E2_NIGHT_DRIVE_PREALARM_TIME_ID_ADDR		(EEPROM_PRAM_BASE_ADDR+2263)//5
//参数ID，自定义，夜间行驶预警时间,,DWORD,单位分钟,默认值10
#define	E2_NIGHT_DRIVE_PREALARM_VOICE_ID_ADDR		(EEPROM_PRAM_BASE_ADDR+2268)//32
//参数ID，自定义，夜间行驶预警语音,STRING,最大30字节
#define	E2_NIGHT_DRIVE_PREALARM_GPOUP_TIME_ID_ADDR	(EEPROM_PRAM_BASE_ADDR+2300)//5
//参数ID，自定义，夜间行驶预警提示时间间隔(每组),DWORD,单位分钟,默认值5
#define	E2_NIGHT_DRIVE_PREALARM_NUMBER_ID_ADDR		(EEPROM_PRAM_BASE_ADDR+2305)//5
//参数ID，自定义，夜间行驶预警提示次数,,DWORD,默认值3
#define	E2_FORBID_DRIVE_NIGHT_TIME_ID_ADDR		(EEPROM_PRAM_BASE_ADDR+2310)//16
//参数ID，自定义，停车休息(禁止行驶)夜间时间,BYTE[14],作STRING处理,默认值全0
#define	E2_FORBID_DRIVE_PREALARM_TIME_ID_ADDR		(EEPROM_PRAM_BASE_ADDR+2326)//5
//参数ID，自定义，禁行时段到达提醒提前时间,DWORD,单位分钟,默认值30
#define	E2_FORBID_DRIVE_PREALARM_VOICE_ID_ADDR		(EEPROM_PRAM_BASE_ADDR+2331)//32
//参数ID，自定义，禁行时段到达提醒语音,STRING,最大30字节
#define	E2_FORBID_DRIVE_PREALARM_GPOUP_TIME_ID_ADDR	(EEPROM_PRAM_BASE_ADDR+2363)//5
//参数ID，自定义，禁行时段提醒时间间隔,DWORD,单位分钟,默认值5
#define	E2_FORBID_DRIVE_PREALARM_NUMBER_ID_ADDR		(EEPROM_PRAM_BASE_ADDR+2368)//5
//参数ID，自定义，禁行时段到达提醒播报的次数,DWORD,默认值3
#define	E2_FORBID_DRIVE_ALARM_VOICE_ID_ADDR		(EEPROM_PRAM_BASE_ADDR+2373)//32
//参数ID，自定义，禁行时段到达提醒语音,STRING,最大30字节
#define	E2_FORBID_DRIVE_ALARM_GPOUP_TIME_ID_ADDR	(EEPROM_PRAM_BASE_ADDR+2405)//5
//参数ID，自定义，禁行时段提醒时间间隔,DWORD,单位分钟,默认值5
#define	E2_FORBID_DRIVE_ALARM_NUMBER_ID_ADDR		(EEPROM_PRAM_BASE_ADDR+2410)//5
//参数ID，自定义，禁行时段到达提醒播报的次数,DWORD,默认值3
#define	E2_FORBID_DRIVE_RUN_SPEED_ID_ADDR		(EEPROM_PRAM_BASE_ADDR+2415)//5
//参数ID，自定义，禁行时段行驶速度,单位km/h,DWORD,默认值20
#define	E2_FORBID_DRIVE_RUN_SPEED_KEEPTIME_ID_ADDR	(EEPROM_PRAM_BASE_ADDR+2420)//5
//参数ID，自定义，禁行时段行驶速度持续时间,单位分钟,DWORD,默认值5，
//当速度大于禁行时段行驶速度且持续时间达到禁行时段行驶速度持续时间，认定为行驶
#define E2_SHUTDOWN_MODE_TIME_ID_ADDR		(EEPROM_PRAM_BASE_ADDR+2425)//5
//参数ID，自定义，关机模式时间参数，
#define E2_SHUTDOWN_MODE_POWER_ID_ADDR		(EEPROM_PRAM_BASE_ADDR+2430)//3
//参数ID，自定义，深度休眠电压，
#define	E2_TIRED_DRIVE_NIGHT_TIME_ID_ADDR	(EEPROM_PRAM_BASE_ADDR+2435)//16
//参数ID，自定义，夜间时间,BYTE[14],作STRING处理,默认值全0,夜间超时使用
#define	E2_NIGHT_MAX_DRVIE_TIME_ID_ADDR			(EEPROM_PRAM_BASE_ADDR+2451)//5
//参数ID，自定义，夜间连续驾驶时间门限,单位秒,DWORD,默认值7200
#define	E2_NIGHT_MIN_RELAX_TIME_ID_ADDR			(EEPROM_PRAM_BASE_ADDR+2456)//5
//参数ID，自定义，夜间最小休息时间,单位秒,DWORD,默认值1200
#define	E2_TIRED_DRIVE_ALARM_TIME_ID_ADDR		(EEPROM_PRAM_BASE_ADDR+2461)//5
//参数ID，自定义，超时驾驶报警提示语音有效时间,单位分钟,DWORD,默认值30,国标要求
#define	E2_TIRED_DRIVE_PREALARM_VOICE_ID_ADDR		(EEPROM_PRAM_BASE_ADDR+2466)//32
//参数ID，自定义，超时驾驶预警提示语音,STRING,最大30字节,默认值请勿超时驾驶
#define	E2_TIRED_DRIVE_PREALARM_GPOUP_TIME_ID_ADDR	(EEPROM_PRAM_BASE_ADDR+2498)//5
//参数ID，自定义，超时驾驶预警每组提示时间间隔,单位分钟,DWORD,默认值5
#define	E2_TIRED_DRIVE_PREALARM_NUMBER_ID_ADDR		(EEPROM_PRAM_BASE_ADDR+2503)//5
//参数ID，自定义，超时驾驶预警每组提示次数,DWORD,默认值3
#define	E2_TIRED_DRIVE_PREALARM_NUMBER_TIME_ID_ADDR	(EEPROM_PRAM_BASE_ADDR+2508)//5
//参数ID，自定义，超时驾驶预警每次提示时间间隔,单位秒,DWORD,默认值10
#define	E2_TIRED_DRIVE_ALARM_VOICE_ID_ADDR		(EEPROM_PRAM_BASE_ADDR+2513)//32
//参数ID，自定义，超时驾驶报警提示语音,STRING,最大30字节,默认值请勿超时驾驶
#define	E2_TIRED_DRIVE_ALARM_GPOUP_TIME_ID_ADDR		(EEPROM_PRAM_BASE_ADDR+2545)//5
//参数ID，自定义，超时驾驶报警每组提示时间间隔,单位分钟,DWORD,默认值5
#define	E2_TIRED_DRIVE_ALARM_NUMBER_ID_ADDR		(EEPROM_PRAM_BASE_ADDR+2550)//5
//参数ID，自定义，超时驾驶报警每组提示次数,DWORD,默认值3
#define	E2_TIRED_DRIVE_ALARM_NUMBER_TIME_ID_ADDR	(EEPROM_PRAM_BASE_ADDR+2555)//5
//参数ID，自定义，超时驾驶报警每次提示时间间隔,单位秒,DWORD,默认值10
#define	E2_TIRED_DRIVE_REPORT_OPEN_TIME_ID_ADDR		(EEPROM_PRAM_BASE_ADDR+2560)//5
//参数ID，自定义，超时驾驶报警位置上报开启时间,单位秒,DWORD,默认值0xffffffff,不开启,九通为20分钟
#define	E2_TIRED_DRIVE_REPORT_TIME_ID_ADDR		(EEPROM_PRAM_BASE_ADDR+2565)//5
//参数ID，自定义，超时驾驶报警位置上报时间间隔,单位秒,DWORD,默认值300,九通为5分钟
#define	E2_TIRED_DRIVE_RUN_SPEED_ID_ADDR		(EEPROM_PRAM_BASE_ADDR+2570)//5
//参数ID，自定义，超时驾驶行驶判定速度,单位km/h,DWORD,默认值5,
#define	E2_TIRED_DRIVE_RUN_KEEPTIME_ID_ADDR		(EEPROM_PRAM_BASE_ADDR+2575)//5
//参数ID，自定义，超时驾驶行驶判定持续时间,单位秒,DWORD,默认值10,
#define	E2_ROUTE_EXCURSION_ALARM_NUMBER_ID_ADDR         (EEPROM_PRAM_BASE_ADDR+2580)//5
//参数ID，自定义，偏离线路 报警次数，0：不播报，255：持续播报 DWORD,默认值3
#define	E2_ROUTE_EXCURSION_ALARM_GPOUP_TIME_ID_ADDR     (EEPROM_PRAM_BASE_ADDR+2585)//5
//参数ID，自定义，偏离线路报警语音时间间隔,DWORD,默认值5
#define	E2_ROUTE_EXCURSION_ALARM_VOICE_ID_ADDR          (EEPROM_PRAM_BASE_ADDR+2590)//32
//参数ID，自定义，偏离线路报警语音,STRING,最大30字节,默认值请你按规定线路行驶
#define	E2_REPORT_FREQ_EVENT_SWITCH_ID_ADDR             (EEPROM_PRAM_BASE_ADDR+2622)//5
//参数ID，自定义，事件项，保留，用于指定产生事件后上报时间间隔的事件项 BIT0位＝1：超速报警,BIT1位＝1：超时报警,DWORD
#define	E2_RUN_FORBID_CALL_SWITCH_ID_ADDR               (EEPROM_PRAM_BASE_ADDR+2627)//2
//参数ID，自定义，行驶中禁止电话功能开关，0允许通话；1禁止通话，通用版默认为0，福建电信版默认为1
#define	E2_SIM_ICCID_NUMBER_ID_ADDR                     (EEPROM_PRAM_BASE_ADDR+2629)//12
//参数ID，自定义，SIM卡的ICCID号码,BCD[10],只读
#define	E2_LOAD_MAX_ID_ADDR                 (EEPROM_PRAM_BASE_ADDR+2641)//5	
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x40，自定义，载重最大限制
//参数ID，自定义，SIM卡的ICCID号码,BCD[10],只读
#define	E2_LED_PORT_ID_ADDR                 (EEPROM_PRAM_BASE_ADDR+2646)//3
//参数ID，自定义，表明32pin的LED接口接的外设类型,
//2字节，bit0:条屏bit1:顶灯bit2:评价器bit3:载重传感器bit4-bit15:保留;相应位为1表示该端口接了相应外设。

#define E2_SECOND_MAIN_SERVER_IP_ID_ADDR 			(EEPROM_PRAM_BASE_ADDR+2649)//32
//参数ID，自定义，第2个连接主服务器IP地址或域名
//0xF281-0xF282为EGS702使用,0xF283为E-MIV404使用
#define E2_SECOND_MAIN_SERVER_TCP_PORT_ID_ADDR 		(EEPROM_PRAM_BASE_ADDR+2681)//5
//参数ID，自定义，第2个连接主服务器TCP端口
#define E2_SECOND_MAIN_SERVER_UDP_PORT_ID_ADDR 		(EEPROM_PRAM_BASE_ADDR+2686)//5
//参数ID，自定义，第2个连接主服务器UDP端口
#define E2_SECOND_BACKUP_SERVER_IP_ID_ADDR 			(EEPROM_PRAM_BASE_ADDR+2691)//32
//参数ID，自定义，第2个连接备份服务器地址//IP或域名
#define E2_SECOND_LOGINGPRS_PASSWORD_ID_ADDR		(EEPROM_PRAM_BASE_ADDR+2723)//32
//参数ID，自定义，第2个连接鉴权码
#define	E2_POS_PORT_ID_ADDR                         (EEPROM_PRAM_BASE_ADDR+2756)//3
//参数ID，自定义，表明16pin接口接的外设类型,
//2字节，bit0:留用,bit1:留用,bit2:久通机电油位传感器;bit3:博实结油位传感器;bit4-bit15:保留;相应位为1表示该端口接了相应外设。
#define E2_SECOND_ACC_OFF_REPORT_TIME_ID_ADDR        (EEPROM_PRAM_BASE_ADDR+2759)//5
//参数ID，自定义，第2个连接休眠汇报时间间隔,单位秒,DWORD
#define E2_SECOND_ACC_ON_REPORT_TIME_ID_ADDR        (EEPROM_PRAM_BASE_ADDR+2764)//5
//参数ID，自定义，第2个连接缺省汇报时间间隔,单位秒,DWORD
#define E2_ENGINE_NUM_ID_ADDR               (EEPROM_PRAM_BASE_ADDR+2769)//16
//参数ID，自定义，发动机号码，最大20个字节

//********************************************************
//********************************************************
//////////分配空间时特需要注意//////////////////
//类型为1的总空间=数据内容长度(1字节)+数据内容(n字节)+校验(1字节)
//类型为0的总空间=数据内容(n字节)+校验(1字节)
//********************************************************
//********************************************************
 //注意：以上参数地址不能超过4095
#define E2_LAST_ADDR		0x1000//4096,
//***************************eeprom结尾地址为0x07ff(2047,0x420+991)*********************
//***************************************eeprom参数的长度*******************************

#define E2_TERMINAL_HEARTBEAT_LEN		4	
//参数ID0x0001，终端心跳发送间隔，单位秒，默认值60

#define E2_TCP_ACK_OVERTIME_LEN			4		
//参数ID0x0002，TCP消息应答超时时间，单位秒，默认值10

#define E2_TCP_RESEND_TIMES_LEN			4			
//参数ID0x0003，TCP消息重传次数，默认值3

#define E2_UDP_ACK_OVERTIME_LEN			4			
//参数ID0x0004，UDP消息应答超时时间，单位秒，默认值10

#define E2_UDP_RESEND_TIMES_LEN			4			
//参数ID0x0005，UDP消息重传次数，默认值3

#define E2_SMS_ACK_OVERTIME_LEN			4				
//参数ID0x0006，SMS消息应答超时时间，单位秒，默认值30

#define E2_SMS_RESEND_TIMES_LEN			4			
//参数ID0x0007，SMS消息重传次数，默认值3

#define E2_MAIN_SERVER_APN_LEN			20			
//参数ID0x0010，主服务器APN

#define E2_MAIN_SERVER_DIAL_NAME_LEN		20			
//参数ID0x0011，主服务器无线通信拨号用户名

#define E2_MAIN_SERVER_DIAL_PASSWORD_LEN	20			
//参数ID0x0012，主服务器无线通信拨号密码

#define E2_MAIN_SERVER_IP_LEN			20		
//参数ID0x0013，主服务器地址，IP或域名

#define E2_BACKUP_SERVER_APN_LEN		20				
//参数ID0x0014，备份服务器APN，无线通信拨号访问点

#define E2_BACKUP_SERVER_DIAL_NAME_LEN		20	
//参数ID0x0015，备份服务器无线通信拨号用户名

#define E2_BACKUP_SERVER_DIAL_PASSWORD_LEN	20		
//参数ID0x0016，备份服务器无线通信拨号密码

#define E2_BACKUP_SERVER_IP_LEN			20			
//参数ID0x0017，备份服务器地址，IP或域名

#define E2_MAIN_SERVER_TCP_PORT_LEN		4				
//参数ID0x0018，主服务器TCP端口
	
#define E2_MAIN_SERVER_UDP_PORT_LEN		4	
//参数ID0x0019，主服务器UDP端口

#define E2_IC_MAIN_SERVER_IP_LEN		39	
//参数ID0x001A//IC卡认证主服务器IP地址或域名

#define E2_IC_MAIN_SERVER_TCP_PORT_LEN		4	
//参数ID0x001B//IC卡认证主服务器TCP端口

#define E2_IC_MAIN_SERVER_UDP_PORT_LEN		4	
//参数ID0x001C//IC卡认证主服务器UDP端口

#define E2_IC_BACKUP_SERVER_IP_LEN		39	
//参数ID0x001D//IC卡认证备份服务器IP地址或域名

#define E2_POSITION_REPORT_STRATEGY_LEN		4	
//参数ID0x0020，位置汇报策略

#define E2_POSITION_REPORT_SCHEME_LEN		4		
//参数ID0x0021，位置汇报方案

#define E2_DRIVER_UNLOGIN_REPORT_TIME_LEN	4		
//参数ID0x0022，驾驶员未登录汇报时间间隔，单位为秒，默认值为60

#define E2_SLEEP_REPORT_TIME_LEN		4			
//参数ID0x0027，休眠时汇报时间间隔，单位为秒，默认值为60

#define E2_EMERGENCY_REPORT_TIME_LEN		4		
//参数ID0x0028，紧急报警时汇报时间间隔，单位为秒，默认值为10

#define	E2_ACCON_REPORT_TIME_LEN		4	
//0x0029缺省时间汇报间隔,单位为秒

#define E2_ACCON_REPORT_DISTANCE_LEN		4	
//0x002C缺省距离汇报间隔,单位为米

#define E2_DRIVER_UNLOGIN_REPORT_DISTANCE_LEN	4	
//参数ID0x002D，驾驶员未登录汇报距离间隔，单位为米，默认值为100

#define E2_SLEEP_REPORT_DISTANCE_LEN		4	
//参数ID0x002e，休眠时汇报距离间隔，单位为米，默认值为1000

#define E2_EMERGENCY_REPORT_DISTANCE_LEN	4	
//参数ID0x002f，紧急报警时汇报距离间隔，单位为米，默认值为100

#define E2_CORNER_REPORT_LEN			4	
//参数ID0x0030，拐点补传角度,单位度/秒，默认值为15

#define E2_ELECTRON_RADIUS_LEN			2
//参数ID0x0031，电子围栏半径

#define E2_MONITOR_SERVER_PHONE_LEN		20			
//参数ID0x0040，监控平台电话号码

#define E2_TERMINAL_RESET_PHONE_LEN		20				
//参数ID0x0041，终端复位电话号码

#define E2_TERMINAL_DEFAULT_SET_PHONE_LEN	20
//参数ID0x0042，恢复出厂设置电话号码

#define E2_MONITOR_SERVER_SMS_PHONE_LEN		20	
//参数ID0x0043，监控平台SMS电话号码

#define E2_SMS_TEXT_ALARM_PHONE_LEN		20	
//参数ID0x0044，SMS文本报警电话号码

#define E2_TERMINAL_GET_PHONE_STRATEGY_LEN	4
//参数ID0x0045，终端电话接听策略

#define E2_MAX_PHONE_TIME_EACH_LEN		4			
//参数ID0x0046，每次最长通话时间，单位为秒，默认为0xffffffff，不限制

#define E2_MAX_PHONE_TIME_MONTH_LEN		4		
//参数ID0x0047，每月最长通话时间，单位为秒，默认为0xffffffff，不限制

#define E2_MONITOR_PHONE_LEN			20			
//参数ID0x0048，监听电话号码

#define E2_MONITOR_SERVER_PRIVILEGE_SMS_LEN	20	
//参数ID0x0049，监管平台特权短信号码

#define E2_ALARM_MASK_LEN			4		
//参数ID0x0050，报警屏蔽字，默认为全开

#define E2_ALARM_TEXT_SWITCH_LEN		4				
//参数ID0x0051，报警发送文本SMS开关，默认为全关

#define E2_ALARM_PHOTO_SWITCH_LEN		4			
//参数ID0x0052，报警拍摄开关，默认为全关

#define E2_ALARM_PHOTO_STORE_FLAG_LEN		4	
//参数ID0x0053，报警拍摄存储标志，默认为全关

#define E2_KEY_FLAG_LEN				4			
//参数ID0x0054，关键标志

#define E2_MAX_SPEED_LEN			4				
//参数ID0x0055，最高速度

#define E2_OVER_SPEED_KEEP_TIME_LEN		4			
//参数ID0x0056，超速持续时间，单位为秒，默认为10秒

#define E2_MAX_DRVIE_TIME_LEN			4		
//参数ID0x0057，连续驾驶时间门限，单位为秒，默认为240秒

#define E2_MAX_DRVIE_TIME_ONEDAY_LEN		4		
//参数ID0x0058，当天累计驾驶时间门限，单位为秒，默认为0

#define E2_MIN_RELAX_TIME_LEN			4		
//参数ID0x0059，最小休息时间，单位为秒，默认为1200

#define E2_MAX_STOP_CAR_TIME_LEN		4			
//参数ID0x005A，最长停车时间，单位为秒，默认为0

#define E2_SPEED_EARLY_ALARM_DVALUE_LEN		2
//参数ID0x005B//超速报警预警差值，单位为1/10Km/h，默认为50

#define E2_TIRE_EARLY_ALARM_DVALUE_LEN		2
//参数ID0x005C//疲劳驾驶预警差值，单位为秒（s），>0，默认为30

#define E2_SET_CRASH_ALARM_LEN		        2
//参数ID0x005D//碰撞报警参数设置 

#define E2_SET_ROLLOVER_ALARM_LEN		2
//参数ID0x005E//侧翻报警参数设置：侧翻角度，单位1 度，默认为30 度

#define E2_TIME_PHOTO_CONTROL_LEN		4
//参数ID0x0064//定时拍照控制

#define E2_MILE_PHOTO_CONTROL_LEN		4
//参数ID0x0065//定距拍照控制

#define E2_PICTURE_QUALITY_LEN			4		
//参数ID0x0070，图像/视频质量，1~10,1为最好

#define E2_PICTURE_BRIGHTNESS_LEN		4		
//参数ID0x0071，亮度，0~255

#define E2_PICTURE_CONTRAST_LEN			4			
//参数ID0x0072，对比度，0~127

#define E2_PICTURE_SATURATION_LEN		4		
//参数ID0x0073，饱和度，0~127

#define E2_PICTURE_CHROMA_LEN			4		
//参数ID0x0074，色度，0~255

#define E2_CAR_TOTAL_MILE_LEN			4		
//参数ID0x0080，车辆里程表读数，0.1km

#define E2_CAR_PROVINCE_ID_LEN			2		
//参数ID0x0081，车辆所在的省域ID

#define E2_CAR_CITY_ID_LEN			2			
//参数ID0x0082，车辆所在的市域ID

#define E2_CAR_PLATE_NUM_LEN			20			
//参数ID0x0083，公安交通管理部门颁发的机动车号牌

#define E2_CAR_PLATE_COLOR_LEN			1
//参数ID0x0084，车牌颜色，按照JT/T415-2006的5.4.12

#define E2_GPS_SET_MODE_LEN		        1
//参数ID0x0090//GNSS 定位模式，默认为E2_PRAM_BASE_CUSTOM_ID+0x 北斗定位

#define E2_GPS_SET_BAUD_LEN		        1
//参数ID0x0091//GNSS 波特率，默认为0x01 9600

#define E2_GPS_SET_OUTPUT_RATE_LEN		1
//参数ID0x0092//GNSS 模块详细定位数据输出频率,默认为0x01 1000ms

#define E2_GPS_SET_GATHER_RATE_LEN		4
//参数ID0x0093//GNSS 模块详细定位数据采集频率，单位为秒，默认为1

#define E2_GPS_SET_UPLOAD_MODE_LEN		1
//参数ID0x0094//GNSS 模块详细定位数据上传方式，默认为0x00，本地存储不上传

#define E2_GPS_SET_UPLOAD_VALUE_LEN		4
//参数ID0x0095//GNSS 模块详细定位数据上传设置,默认为0

#define E2_CAN1_GATHER_TIME_LEN		        4
//参数ID0x0100//CAN 总线通道1 采集时间间隔(ms)，0 表示不采集

#define E2_CAN1_UPLOAD_TIME_LEN		        2
//参数ID0x0101//CAN 总线通道1 上传时间间隔(s)，0 表示不上传

#define E2_CAN2_GATHER_TIME_LEN		        4
//参数ID0x0102//CAN 总线通道2 采集时间间隔(ms)，0 表示不采集

#define E2_CAN2_UPLOAD_TIME_LEN		        2
//参数ID0x0103//CAN 总线通道2 上传时间间隔(s)，0 表示不上传

#define E2_CAN_SET_ONLY_GATHER_LEN		8
//参数ID0x0110//CAN 总线ID 单独采集设置BYTE[8]


//*****************自定义参数ID的长度****************
#define E2_LOGINGPRS_PASSWORD_ID_LEN		20
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x00，自定义，鉴权码
#define E2_REGISTER_SMS_PHONE_ID_LEN		20	
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x01，自定义，注册短消息号码
#define E2_PROGRAM_UPDATA_REQUEST_ID_LEN	1
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x02，自定义，程序升级请求标志
#define E2_PROGRAM_UPDATA_VERIFYCODE_ID_LEN	4
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x03，自定义，程序升级校验码
#define E2_PROGRAM_UPDATA_LENGTH_ID_LEN		4
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x04，自定义，程序升级长度
#define E2_MANUFACTURE_LEN			5
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x05，自定义，制造商ID，5字节		
#define E2_DEVICE_ID_LEN			7		
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x06，自定义，厂商终端ID，7字节
#define E2_DEVICE_PHONE_ID_LEN			6
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x07，自定义，终端手机号，6字节
#define	E2_CAR_IDENTIFICATION_CODE_ID_LEN	20	
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x08，自定义，车辆识别码
#define	E2_CAR_TYPE_ID_LEN			20				
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x09，自定义，车牌分类
#define	E2_CURRENT_DRIVER_ID_LEN		3		
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x0A，自定义，当前驾驶员代码
#define	E2_CURRENT_LICENSE_ID_LEN		20		
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x0B，自定义，当前驾驶证号码
#define	E2_FIRMWARE_VERSION_ID_LEN		5			
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x0C，自定义，固件版本号
#define E2_ALL_DRIVER_INFORMATION_ID_LEN	150		
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x0D，自定义，所有驾驶员信息
#define	E2_SPEED_SENSOR_COEF_ID_LEN		1		
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x0E，自定义，速度传感器系数
#define	E2_CAR_FEATURE_COEF_LEN		        3		
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x0F，自定义，车辆特征系数
#define E2_SPEED_SELECT_LEN			1		
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x10，自定义，速度选择开关
#define E2_BACKLIGHT_SELECT_ID_LEN		1
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x11，自定义，背光选择开关
#define E2_FIRST_POWERON_FLAG_ID_LEN		1
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x12，自定义，系统第1次上电标志
#define E2_ACC_ON_INTERVAL_ID_LEN		4
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x13，自定义，ACC ON 时汇报时间间隔
#define E2_ACC_ON_DISTANCE_ID_LEN		4
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x14，自定义，ACC ON 时汇报距离间隔
#define	E2_UPDATA_LINK_CONTROL_ID_LEN		1	
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x15，自定义，连接控制
#define	E2_UPDATA_DIAL_NAME_ID_LEN		20		
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x16，自定义，无限升级拨号点名称
#define	E2_UPDATA_DIAL_USER_ID_LEN		20
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x17，自定义，无限升级拨号用户名
#define	E2_UPDATA_DIAL_PASSWORD_ID_LEN		20
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x18，自定义，无限升级拨号密码
#define	E2_UPDATA_IP_ID_LEN			20
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x19，自定义，无限升级IP地址
#define	E2_UPDATA_TCP_PORT_ID_LEN		2
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x1a，自定义，无限升级TCP端口
#define	E2_UPDATA_UDP_PORT_ID_LEN		2
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x1b，自定义，无限升级UDP端口
#define	E2_UPDATA_MONITOR_SERVER_CODE_ID_LEN	20
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x1c，自定义，无限升级监管平台鉴权码
#define	E2_UPDATA_HARDWARE_VERSION_ID_LEN	20
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x1d，自定义，无限升级硬件版本
#define	E2_UPDATA_FIRMWARE_VERSION_ID_LEN	20
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x1e，自定义，无限升级固件版本
#define	E2_UPDATA_URL_ID_LEN			100//dxl,2013.6.11原来为200
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x1f，自定义，无限升级URL完整地址
#define E2_UPDATA_TIME_ID_LEN			2
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x20，自定义，无限升级连接到指定服务器时限
#define E2_HOT_PHONE_ID_LEN			20
//E2_PRAM_BASE_CUSTOM_ID+0x21参数ID，自定义，热线电话
#define E2_UPDATA_MANUFACTURE_ID_LEN		10
//E2_PRAM_BASE_CUSTOM_ID+0x22参数ID，自定义，无限升级制造商ID
#define	E2_PHONE_CALLBACK_ID_LEN		20			
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x23，自定义，电话回拨号码
#define	E2_CALLBACK_FLAG_ID_LEN			1
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x24，自定义，电话回拨标志,1为监听;0为普通通话
#define	E2_DEVICE_PHONE_BACKUP_ID_LEN		6
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x25，自定义，终端手机号，备份
#define	E2_LAST_LOCATION_ID_LEN			20
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x26，自定义，最后一个导航位置信息
#define E2_PHOTO_SPACE_ID_LEN                   4
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x27，自定义，拍照时间间隔
#define E2_PHOTO_NUM_ID_LEN                     4                     
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x28，自定义，拍照张数
#define E2_DOOR_SET_LEN                         1  //九通V2.81版本长度可能为4                   
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x29，自定义，拍照张数
#define E2_DEVICE_BACKUP_ID_LEN			7		
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x2a，自定义，厂商终端ID，7字节
#define E2_CDMA_SLEEP_ID_LEN			1
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x2b
#define E2_PASSWORD_ID_LEN			4
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x2c
#define E2_INSTALL_TIME_ID_LEN			4
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x2d
#define E2_INSTALL_TIME_FLAG_LEN		1
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x2e
#define E2_HORN_SET_ID_LEN			1
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x2f
#define E2_GPS_CONTROL_LEN			1
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x30
#define	E2_USB_UPDATA_FLAG_ID_LEN		1	
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x31，自定义，USB升级标志
#define	E2_TAXIMETER_CONTROL_LEN		1
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x32，自定义，计价器功能开关,0为关闭,1为开启
#define E2_TOPLIGHT_CONTROL_ID_LEN		1
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x33，自定义，顶灯功能开关,0为关闭,1为开启，不使用
#define E2_LEDLIGHT_CONTROL_ID_LEN		1
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x34，自定义，LED广告功能开关,0为关闭,1为开启，不使用
#define E2_HB_IC_DRIVER_ID_LEN			120//
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x35，自定义，设置符合行标标准格式（0x0702指令）的驾驶员信息
#define E2_STATUS_PHOTO_SWITCH_ID_LEN 		4
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x36，自定义，状态拍摄开关
#define E2_STATUS_PHOTO_STORE_FLAG_ID_LEN 	4
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x37，自定义，状态拍摄存储标志//默认为全关
#define	E2_RESUME_NOUSED_ID_LEN			1
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x38，自定义，恢复终端为未使用状态
#define	E2_NIGHT_TIME0_ID_LEN			4
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x39，自定义，夜间时间范围,BCD码
#define	E2_NIGHT_MAX_DRVIE_TIME0_ID_LEN		4
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x3a，自定义，夜间超时驾驶门限
#define	E2_NIGHT_MAX_SPEED0_ID_LEN		4
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x3b，自定义，夜间限速值
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x3C，自定义，V1。13补充协议VTK用作车门和SOS输入切换，VTK用，行标无此项
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x3D，自定义，V1。13补充协议VTK用作自动设防启动时间，VTK用，行标无此项
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x3E，自定义，V1。13补充协议VTK用作休眠时间，VTK用，行标无此项
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x3F，自定义，V1。13补充协议VTK用作震动检测时间，VTK用，行标无此项
#define	E2_LOAD_MAX_ID_LEN			4	//参数IDE2_PRAM_BASE_CUSTOM_ID+0x40，//参数ID，自定义，载重最大限制，超过该值会有超载报警，单位为kg，4字节
#define E2_CAR_INIT_MILE_ID_LEN                     4
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x41，自定义，车辆初始化里程，DWORD
#define E2_CAR_ONLY_NUM_ID_LEN                      35
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x42，自定义，唯一性编号，字符串，35字节
#define E2_DEFINE_ALARM_MASK_WORD_ID_LEN		4
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x43，自定义，自定义报警屏蔽字，DWORD
#define E2_DEFINE_STATUS_CHANGE_PHOTO_ENABLE_ID_LEN	4
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x44，自定义，自定义状态变化拍照使能开关，DWORD
#define E2_DEFINE_STATUS_CHANGE_PHOTO_STORE_ID_LEN	4
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x45，自定义，自定义状态变化拍照存储开关，DWORD
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x46，最小弯道角度，自定义，EGS702用，EGS701无此项
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x47，最大弯道角度，自定义，EGS702用，EGS701无此项
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x48，弯道速度限值，自定义，EGS702用，EGS701无此项
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x49，车辆初始油耗，自定义，老的EGS701用，EGS701无此项
#define	E2_UNLOGIN_TTS_PLAY_ID_LEN			1
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x4c，自定义，驾驶员未签到语音提醒，0为关，1为开。配IC卡时出货设置为开，否则设置为关
#define E2_TERMINAL_TYPE_ID_LEN			2
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x4d，自定义，终端类型
#define E2_GPS_ATTRIBUTE_ID_LEN			1
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x4e，自定义，GNSS属性
#define E2_GPRS_ATTRIBUTE_ID_LEN			1
//参数IDE2_PRAM_BASE_CUSTOM_ID+0x4f，自定义，通讯模块属性
//#define	E2_OVERSPEED_PREALARM_MARGIN_ID_LEN		4//5
//参数ID，自定义，超速预警差值,DWORD,单位0.1km/h,默认值50
#define	E2_OVERSPEED_PREALARM_KEEPTIME_ID_LEN		4//5
//参数ID，自定义，超速预警持续时间,DWORD,单位秒,默认值3
#define	E2_OVERSPEED_PREALARM_VOICE_ID_LEN		30//32
//参数ID，自定义，超速预警语音,STRING,最大30字节
#define	E2_OVERSPEED_PREALARM_GPROUP_TIME_ID_LEN	4//5
//参数ID，自定义，超速预警每组提示时间间隔,DWORD,单位分钟,默认值5
#define	E2_OVERSPEED_PREALARM_NUMBER_ID_LEN		4//5
//参数ID，自定义，超速预警每组提示次数,DWORD,默认值3
#define	E2_OVERSPEED_PREALARM_NUMBER_TIME_ID_LEN	4//5
//参数ID，自定义，超速预警每次提示时间间隔,DWORD,单位秒,默认值10
//#define	E2_OVERSPEED_ALARM_VALUE_ID_LEN			4//5
//参数ID，自定义，超速报警值，DWORD,单位km/h,默认值130
//#define	E2_OVERSPEED_ALARM_KEEPTIME_ID_LEN		4//5
//参数ID，自定义，超速报警持续时间，DWORD,单位秒默认值10
#define	E2_OVERSPEED_ALARM_VOICE_ID_LEN			30//32
//参数ID，自定义，超速报警语音,STRING,最大30字节
#define	E2_OVERSPEED_ALARM_GPROUP_TIME_ID_LEN		4//5
//参数ID，自定义，超速报警每组提示时间间隔,DWORD,单位分钟,默认值5
#define	E2_OVERSPEED_ALARM_NUMBER_ID_LEN		4//5
//参数ID，自定义，超速报警每组提示次数,DWORD,默认值3
#define	E2_OVERSPEED_ALARM_NUMBER_TIME_ID_LEN		4//5
//参数ID，自定义，超速报警每次提示时间间隔,DWORD,单位秒,默认值10
#define E2_OVERSPEED_ALARM_REPORT_TIME_ID_LEN		4//5
//参数ID，自定义，超速报警位置上报时间间隔,DWORD,单位秒,默认值0
#define	E2_OVERSPEED_NIGHT_TIME_ID_LEN			14//16
//参数ID，自定义，夜间时间,BYTE[14],作STRING处理,默认值全0
#define	E2_NIGHT_OVERSPEED_PERCENT_ID_LEN		4//5
//参数ID，自定义，夜间超速报警及预警百分比,DWORD,默认值80
#define	E2_NIGHT_DRIVE_PREALARM_TIME_ID_LEN		4//5
//参数ID，自定义，夜间行驶预警时间,,DWORD,单位分钟,默认值10
#define	E2_NIGHT_DRIVE_PREALARM_VOICE_ID_LEN		30//32
//参数ID，自定义，夜间行驶预警语音,STRING,最大30字节
#define	E2_NIGHT_DRIVE_PREALARM_GPOUP_TIME_ID_LEN	4//5
//参数ID，自定义，夜间行驶预警提示时间间隔(每组),DWORD,单位分钟,默认值5
#define	E2_NIGHT_DRIVE_PREALARM_NUMBER_ID_LEN		4//5
//参数ID，自定义，夜间行驶预警提示次数,,DWORD,默认值3
#define	E2_FORBID_DRIVE_NIGHT_TIME_ID_LEN		14//16
//参数ID，自定义，停车休息(禁止行驶)夜间时间,BYTE[14],作STRING处理,默认值全0
#define	E2_FORBID_DRIVE_PREALARM_TIME_ID_LEN		4//5
//参数ID，自定义，禁行时段到达提醒提前时间,DWORD,单位分钟,默认值30
#define	E2_FORBID_DRIVE_PREALARM_VOICE_ID_LEN		30//32
//参数ID，自定义，禁行时段到达提醒语音,STRING,最大30字节
#define	E2_FORBID_DRIVE_PREALARM_GPOUP_TIME_ID_LEN	4//5
//参数ID，自定义，禁行时段提醒时间间隔,DWORD,单位分钟,默认值5
#define	E2_FORBID_DRIVE_PREALARM_NUMBER_ID_LEN		4//5
//参数ID，自定义，禁行时段到达提醒播报的次数,DWORD,默认值3
#define	E2_FORBID_DRIVE_ALARM_VOICE_ID_LEN		30//32
//参数ID，自定义，禁行时段到达提醒语音,STRING,最大30字节
#define	E2_FORBID_DRIVE_ALARM_GPOUP_TIME_ID_LEN		4//5
//参数ID，自定义，禁行时段提醒时间间隔,DWORD,单位分钟,默认值5
#define	E2_FORBID_DRIVE_ALARM_NUMBER_ID_LEN		4//5
//参数ID，自定义，禁行时段到达提醒播报的次数,DWORD,默认值3
#define	E2_FORBID_DRIVE_RUN_SPEED_ID_LEN		4//5
//参数ID，自定义，禁行时段行驶速度,单位km/h,DWORD,默认值20
#define	E2_FORBID_DRIVE_RUN_SPEED_KEEPTIME_ID_LEN	4//5
//参数ID，自定义，禁行时段行驶速度持续时间,单位分钟,DWORD,默认值5，
#define E2_SHUTDOWN_MODE_TIME_ID_LEN			4//5
//参数ID，自定义，关机模式时间参数，
#define E2_SHUTDOWN_MODE_POWER_ID_LEN			2//3
//参数ID，自定义，深度休眠电压，
#define	E2_TIRED_DRIVE_NIGHT_TIME_ID_LEN		14//16
//参数ID，自定义，夜间时间,BYTE[14],作STRING处理,默认值全0,夜间超时使用
#define	E2_NIGHT_MAX_DRVIE_TIME_ID_LEN			4//5
//参数ID，自定义，夜间连续驾驶时间门限,单位秒,DWORD,默认值7200
#define	E2_NIGHT_MIN_RELAX_TIME_ID_LEN			4//5
//参数ID，自定义，夜间最小休息时间,单位秒,DWORD,默认值1200
#define	E2_TIRED_DRIVE_ALARM_TIME_ID_LEN		4//5
//参数ID，自定义，超时驾驶报警提示语音有效时间,单位分钟,DWORD,默认值30,国标要求
#define	E2_TIRED_DRIVE_PREALARM_VOICE_ID_LEN		30//32
//参数ID，自定义，超时驾驶预警提示语音,STRING,最大30字节,默认值请勿超时驾驶
#define	E2_TIRED_DRIVE_PREALARM_GPOUP_TIME_ID_LEN	4//5
//参数ID，自定义，超时驾驶预警每组提示时间间隔,单位分钟,DWORD,默认值5
#define	E2_TIRED_DRIVE_PREALARM_NUMBER_ID_LEN		4//5
//参数ID，自定义，超时驾驶预警每组提示次数,DWORD,默认值3
#define	E2_TIRED_DRIVE_PREALARM_NUMBER_TIME_ID_LEN	4//5
//参数ID，自定义，超时驾驶预警每次提示时间间隔,单位秒,DWORD,默认值10
#define	E2_TIRED_DRIVE_ALARM_VOICE_ID_LEN		30//32
//参数ID，自定义，超时驾驶报警提示语音,STRING,最大30字节,默认值请勿超时驾驶
#define	E2_TIRED_DRIVE_ALARM_GPOUP_TIME_ID_LEN		4//5
//参数ID，自定义，超时驾驶报警每组提示时间间隔,单位分钟,DWORD,默认值5
#define	E2_TIRED_DRIVE_ALARM_NUMBER_ID_LEN		4//5
//参数ID，自定义，超时驾驶报警每组提示次数,DWORD,默认值3
#define	E2_TIRED_DRIVE_ALARM_NUMBER_TIME_ID_LEN		4//5
//参数ID，自定义，超时驾驶报警每次提示时间间隔,单位秒,DWORD,默认值10
#define	E2_TIRED_DRIVE_REPORT_OPEN_TIME_ID_LEN		4//5
//参数ID，自定义，超时驾驶报警位置上报开启时间,单位秒,DWORD,默认值0xffffffff,不开启,九通为20分钟
#define	E2_TIRED_DRIVE_REPORT_TIME_ID_LEN		4//5
//参数ID，自定义，超时驾驶报警位置上报时间间隔,单位秒,DWORD,默认值300,九通为5分钟
#define	E2_TIRED_DRIVE_RUN_SPEED_ID_LEN			4//5
//参数ID，自定义，超时驾驶行驶判定速度,单位km/h,DWORD,默认值5,
#define	E2_TIRED_DRIVE_RUN_KEEPTIME_ID_LEN		4//5
//参数ID，自定义，超时驾驶行驶判定持续时间,单位秒,DWORD,默认值10,
#define	E2_ROUTE_EXCURSION_ALARM_NUMBER_ID_LEN          4//5
//参数ID，自定义，偏离线路 报警次数，0：不播报，255：持续播报 DWORD,默认值3
#define	E2_ROUTE_EXCURSION_ALARM_GPOUP_TIME_ID_LEN      4//5
//参数ID，自定义，偏离线路报警语音时间间隔,DWORD,默认值5
#define	E2_ROUTE_EXCURSION_ALARM_VOICE_ID_LEN           30//32
//参数ID，自定义，偏离线路报警语音,STRING,最大30字节,默认值请你按规定线路行驶
#define	E2_REPORT_FREQ_EVENT_SWITCH_ID_LEN              4//5
//参数ID，自定义，事件项，保留，用于指定产生事件后上报时间间隔的事件项 BIT0位＝1：超速报警,BIT1位＝1：超时报警DWORD
#define	E2_RUN_FORBID_CALL_SWITCH_ID_LEN               1//2
//参数ID，自定义，行驶中禁止电话功能开关，0允许通话；1禁止通话，通用版默认为0，福建电信版默认为1
#define	E2_SIM_ICCID_NUMBER_ID_LEN                     10//12
//参数ID，自定义，SIM卡的ICCID号码,BCD[10],只读
#define	E2_LED_PORT_ID_LEN                             2//3
//参数ID，自定义，表明32pin的LED接口接的外设类型,//2字节
#define E2_SECOND_MAIN_SERVER_IP_ID_LEN 			30
//参数ID，自定义，第2个连接主服务器IP地址或域名
//0xF281-0xF282为EGS702使用,0xF283为E-MIV404使用
#define E2_SECOND_MAIN_SERVER_TCP_PORT_ID_LEN 		4
//参数ID，自定义，第2个连接主服务器TCP端口
#define E2_SECOND_MAIN_SERVER_UDP_PORT_ID_LEN 		4
//参数ID，自定义，第2个连接主服务器UDP端口
#define E2_SECOND_BACKUP_SERVER_IP_ID_LEN 			20
//参数ID，自定义，第2个连接备份服务器地址//IP或域名
#define E2_SECOND_LOGINGPRS_PASSWORD_ID_LEN		    20
//参数ID，自定义，第2个连接鉴权码
#define	E2_POS_PORT_ID_LEN                             2//3
//参数ID，自定义，表明16pin接口接的外设类型,//2字节，bit0:留用,bit1:留用,bit2:久通机电油位传感器;bit3:博实结油位传感器;bit4-bit15:保留;相应位为1表示该端口接了相应外设。
#define E2_SECOND_ACC_OFF_REPORT_TIME_ID_LEN        4//5
//参数ID，自定义，第2个连接休眠汇报时间间隔,单位秒,DWORD
#define E2_SECOND_ACC_ON_REPORT_TIME_ID_LEN        4//5
//参数ID，自定义，发动机号码，最大16个字节
#define E2_ENGINE_NUM_ID_LEN            16

//*************函数声明***************
/*********************************************************************
//函数名称	:EepromPram_ReadPram(u32 PramID, u8 *pBuffer)
//功能		:读eeprom参数
//输入		:PramID:参数ID;pBuffer:指向数据（读出的字节流，读出的是数值则高字节在前）的指针;
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:返回值>0表示正确读出，返回值==0表示读错误;
*********************************************************************/
u16 EepromPram_ReadPram(u32 PramID, u8 *pBuffer);
/*********************************************************************
//函数名称	:EepromPram_WritePram(u32 PramID, u8 *pBuffer, BufferLen)
//功能		:写eeprom参数
//输入		:PramID:参数ID;pBuffer:指向数据（待写入的字节流,写入的是数值的话则高字节应在前）的指针;
//		:BufferLen:数据的长度
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:写入正确时，返回的长度等于BufferLen;写入错误时，返回0,此时可尝试再调用该函数写一次
*********************************************************************/
u16 EepromPram_WritePram(u32 PramID, u8 *pBuffer, u8 BufferLen);
/*********************************************************************
//函数名称	:EepromPram_UpdateVariable(PramID)
//功能		:更新PramID参数对应的变量
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:千万注意:EepromPram_UpdateVariable(E2_CAR_TOTAL_MILE_ID)不可随便调用，只能在设置完E2_CAR_TOTAL_MILE_ID后调用              
*********************************************************************/
void EepromPram_UpdateVariable(u32 PramID);
/*********************************************************************
//函数名称	:EepromPram_DefaultSet(void)
//功能		:系统默认设置
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:1.系统第1次运行时会调用;2.收到恢复出厂设置命令时会调用
*********************************************************************/
void EepromPram_DefaultSet(void);
/*********************************************************************
//函数名称	:EepromPram_DefaultSetRead(void)
//功能		:读系统第1次运行时的默认设置值
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void EepromPram_DefaultSetRead(void);
/*********************************************************************
//函数名称	:EepromPram_EraseAllPram(void)
//功能		:擦除所有的参数
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void EepromPram_EraseAllPram(void);
//*************************调试用********************
/*********************************************************************
//函数名称	:EepromPram_WriteAllPram(void)
//功能		:写eeprom所有参数
//输入		:PramID:参数ID;pBuffer:指向数据（待写入的字节流,写入的是数值的话则高字节应在前）的指针;
//		:BufferLen:数据的长度
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:写入正确时，返回的长度等于BufferLen;写入错误时，返回0,此时可尝试再调用该函数写一次
*********************************************************************/
void EepromPram_WriteAllPram(void);


/*********************************************************************
//函数名称	:EepromPram_SystemInitSet(void)
//功能		:eeprom参数系统初始化设置(主要是IP,PORT,APN)
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	;
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void EepromPram_SystemInitSet(void);
#endif

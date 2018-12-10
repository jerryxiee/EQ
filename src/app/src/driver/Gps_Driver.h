#ifndef	__GPS_DRIVER_H
#define	__GPS_DRIVER_H

extern u8	GpsOnOffFlag;//GPS开关标志,0为GPS关,1为GPS开

//*******************宏定义***********************
#define 	GPS_POWER_OFF()    	GpioOutOff(GPS_CTR);GpsOnOffFlag=0      //关闭GPS模块电源
#define 	GPS_POWER_ON()    	GpioOutOn(GPS_CTR);GpsOnOffFlag=1	//打开GPS电源
#define		GPS_RX_BUFFER_SIZE	100//接收缓冲大小
#define		GPRMC_BUFFER_SIZE	100
#define		GPGGA_BUFFER_SIZE	100
#define		GPGSV_BUFFER_SIZE	100
#define		GPS_SEND_BUFFER_SIZE	200
typedef struct
{
	u8	SendBuffer[GPS_SEND_BUFFER_SIZE];
	u16	SendLen;
	u16	SendCount;
}USART3_SEND_STRUCT;

/*********************************************************************
//函数名称	:Gps_Init(void)
//功能		:GPS初始化
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void Gps_Init(void);
/*********************************************************************
//函数名称	:GPS_RxIsr(void)
//功能		:GPS接收中断
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void Gps_RxIsr(void);
/*********************************************************************
//函数名称	:GPS_TxIsr(void)
//功能		:GPS发送中断
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void Gps_TxIsr(void);
/*********************************************************************
//函数名称	:GPS_SendData
//功能		:GPS发送数据
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:成功返回ACK_OK；失败返回ACK_ERROR
//备注		:
*********************************************************************/
ProtocolACK  Gps_SendData(u8 *pBuffer, u16 BufferLen);
/*********************************************************************
//函数名称	:GPS_ReadGPRMC(u8 *pBuffer)
//功能		:读GPRMC原始数据
//备注		:
*********************************************************************/
u8 Gps_ReadGPRMC(u8 *pBuffer);
/*********************************************************************
//函数名称	:GPS_ReadGPGSV(u8 *pBuffer)
//功能		:读GPGSV原始数据
//备注		:
*********************************************************************/
u8 Gps_ReadGPGSV(u8 *pBuffer);
void GPS_putc(int ch);
void GPS_putstr(char* str);
#endif
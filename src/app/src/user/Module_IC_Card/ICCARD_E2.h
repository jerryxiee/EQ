/*************************************************************************
* Copyright (c) 2014 All rights reserved., 
* 文件名称 : ICCARD_E2.h
* 当前版本 : 0.1
* 开发者   : MARK
* 编辑时间 : 2014年03月18日
* 更新时间 ：
*************************************************************************/

#ifndef _ICCARD_E2_H
#define _ICCARD_E2_H

#include <inttypes.h>

/****************************宏定义E2的各个性能参数*********************/

#define RMM_COMM 0x30    //读主存命令字
#define UMM_COMM 0x38    //写主存命令字
#define CVD_COMM 0x33    //校验密码
#define RSM_COMM 0x31    //读密码存储区
#define USM_COMM 0x39    //写密码存储区
#define RPM_COMM 0x34    //读保护存储区
#define WPM_COMM 0x3c    //写保护存储区

#define EE_24XX_HL_DEV_ADDR       0xA0            /* 24xx128的设备地址 */
#define EE_24XX_HL_PAGE_SIZE      64              /* 24xx128的页面大小 */
#define EE_24XX_HL_SIZE          16384            /* 24xx128 总容量 */

#define EE_24C128_PAGE_TOTAL      256             /*256= 16384/64*/

#define EE_24XX_HL_SELTEST_ADDR   0               /*自检起始地址*/
#define EE_24XX_HL_SELTEST_LEN    16              /*自检区域长度*/
///////////////////////
extern u8 EEPROM_24XX_HL_Byte_Write( const u16 Address ,  const u8 data );

extern u16 EEPROM_24XX_HL_Buf_Read(  const u16 InAddr, u8 * pOutBuf,  u16 OutLength );

extern u16 EEPROM_24XX_HL_Buf_Write( const u16 InAddr, u8 * pInBuf,  u16 InLength  );

extern u8 EEPROM_24XX_HL_Init(void );

extern u8 EEPROM_24XX_HL_SelCheck(void );

extern u8 EEPROM_HL_CheckOk(void);

extern u8 EEPROM_24C128_AllChip_Read( void  );

extern u8 EEPROM_24C128_AllChip_Read_From_EEPROM2RAM( void  );

extern u8 EEPROM_24C128_AllChip_Write_From_RAM2EEPROM( void  );

extern u8 SL4442_Buffer_Read(  u8 *pBuf, u16  LengthBuf );

/********************************************************************************************************
*	函 数 名: E2_ReadBytes
*	功能说明: 从串行EEPROM指定地址处开始读取若干数据
*	形    参：_pReadBuf : 存放读到的数据的缓冲区指针
*			 _ucDevAddr : 设备地址
*			 _usAddress : 起始地址
*			 _usSize : 数据长度，单位为字节
*			 _pReadBuf : 存放读到的数据的缓冲区指针
*	返 回 值: 0 表示失败，1表示成功
********************************************************************************************************/
u8 E2_ReadBytes(u8 *_pReadBuf, u8 _ucDevAddr, u8 _usAddress, u16 _usSize);
/**********************************************************************************************************
*	函 数 名: E2_WriteBytes
*	功能说明: 向串行EEPROM指定地址写入若干数据，采用页写操作提高写入效率
*	形    参：_pWriteBuf : 存放读到的数据的缓冲区指针
*			  _ucDevAddr : 设备地址
*			  _usAddress : 起始地址
*			  _usSize : 数据长度，单位为字节
*	返 回 值: 0 表示失败，1表示成功
**********************************************************************************************************/
u8 E2_WriteBytes(u8 *_pWriteBuf, u16 _ucDevAddr, u16 _usAddress, u16 _usSize);
/**********************************************************************************************************
*	函 数 名: E2_WriteOneByte
*	功能说明: 向串行EEPROM指定地址写入一个数据
*	形    参：data : 数据
*			  _ucDevAddr : 设备地址
*			  _ucAddress : 写入地址
*	返 回 值: 0 表示失败，1表示成功
**********************************************************************************************************/
u8 E2_WriteOneByte(u8 data, unsigned char _ucDevAddr, unsigned char _ucAddress);
/********************************************************************************************************
*	函 数 名: E2_ReadOneByte
*	功能说明: 从串行EEPROM指定地址处开始读取一个数据
*	形    参：_pReadBuf : 存放读到的数据的缓冲区指针
*			 _ucDevAddr : 设备地址
*			 _usAddress : 起始地址
*			 _pReadBuf : 存放读到的数据的缓冲区指针
*	返 回 值: 0 表示失败，1表示成功
********************************************************************************************************/
u8 E2_ReadOneByte(u8 *data, u8 _ucDevAddr, u8 _usAddress);
#endif
/*******************************************************************************
 *                             end of module
 *******************************************************************************/
 
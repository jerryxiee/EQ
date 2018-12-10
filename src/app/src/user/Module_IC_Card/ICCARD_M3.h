/*************************************************************************
* Copyright (c) 2014 All rights reserved., 
* 文件名称 : ICCARD_M3.h
* 当前版本 : 0.1
* 开发者   : MARK
* 编辑时间 : 2014年03月17日
* 更新时间 ：
*************************************************************************/

#ifndef _ICCARD_M3_H
#define _ICCARD_M3_H

#include <inttypes.h>

/********************宏定义通讯端口：I/0,CLK,RST,DETECT,POWER**************/

/*********************************I/0--->PF0*****************************/ 

#define RCC_ICCARD_IO    RCC_AHB1Periph_GPIOF           /* GPIO的F组端口时钟 */
#define GPIO_ICCARD_IO   GPIOF                          /* 选定F组端口*/ 
#define PIN_ICCARD_IO    GPIO_Pin_0                     /* 选定端口0*/ 

/********************************CLK--->PF1*****************************/ 

#define RCC_ICCARD_CLK    RCC_AHB1Periph_GPIOF           /* GPIO的F组端口时钟 */
#define GPIO_ICCARD_CLK   GPIOF                          /* 选定F组端口*/ 
#define PIN_ICCARD_CLK    GPIO_Pin_1                     /* 选定端口1*/   

/*******************************RST--->PG14****************************/

#define RCC_ICCARD_RST    RCC_AHB1Periph_GPIOG           /* GPIO的G组端口时钟 */
#define GPIO_ICCARD_RST   GPIOG                          /* 选定G组端口*/ 
#define PIN_ICCARD_RST    GPIO_Pin_14                    /* 选定端口14*/

/*****************************DETECT--->PD5****************************/

#define RCC_ICCARD_DETECT    RCC_AHB1Periph_GPIOD           /* GPIO的D组端口时钟 */
#define GPIO_ICCARD_DETECT   GPIOG                          /* 选定D组端口*/ 
#define PIN_ICCARD_DETECT    GPIO_Pin_9                     /* 选定端口5*/

/*****************************POWER--->PC9****************************/

#define RCC_ICCARD_POWER    RCC_AHB1Periph_GPIOC           /* GPIO的C组端口时钟 */
#define GPIO_ICCARD_POWER   GPIOC                          /* 选定C组端口*/ 
#define PIN_ICCARD_POWER    GPIO_Pin_9                     /* 选定端口9*/

/*****************************SHOCK_CS--->PD7****************************/

#define RCC_BMA250_SHOCK_CS    RCC_AHB1Periph_GPIOD           /* GPIO的D组端口时钟 */
#define GPIO_BMA250_SHOCK_CS   GPIOD                          /* 选定D组端口*/ 
#define PIN_BMA250_SHOCK_CS    GPIO_Pin_7                     /* 选定端口7*/

/******配置通讯端口：I/0,CLK,RST,DETECT,POWER的置位和复位**************/

/*********************************I/0--->PF0*****************************/ 

#define ICCARD_SDA_SET()  GPIO_SetBits(    GPIO_ICCARD_IO , PIN_ICCARD_IO )		    /* IO = 1 */
#define ICCARD_SDA_RESET()  GPIO_ResetBits(  GPIO_ICCARD_IO , PIN_ICCARD_IO )		    /* IO = 0 */
#define ICCARD_SDA_READ()  GPIO_ReadInputDataBit( GPIO_ICCARD_IO , PIN_ICCARD_IO )	/* 读IO口线状态 */

/********************************CLK--->PF1*****************************/ 

#define ICCARD_CLK_SET()  GPIO_SetBits(    GPIO_ICCARD_CLK , PIN_ICCARD_CLK )		/* CLK = 1 */
#define ICCARD_CLK_RESET()  GPIO_ResetBits(  GPIO_ICCARD_CLK , PIN_ICCARD_CLK )     /* CLK = 0 */

/*******************************RST--->PG14****************************/

#define ICCARD_RST_SET()  GPIO_SetBits(    GPIO_ICCARD_RST , PIN_ICCARD_RST )		/* RST = 1 */
#define ICCARD_RST_RESET()  GPIO_ResetBits(  GPIO_ICCARD_RST , PIN_ICCARD_RST )     /* RST = 1 */

/*****************************DETECT--->PD5****************************/

#define ICCARD_DETECT_SET()  GPIO_SetBits(    GPIO_ICCARD_DETECT , PIN_ICCARD_DETECT )		/* DETECT = 1 */
#define ICCARD_DETECT_RESET()  GPIO_ResetBits(  GPIO_ICCARD_DETECT , PIN_ICCARD_DETECT )        /* DETECT = 0 */
#define ICCARD_DETECT_READ()  GPIO_ReadInputDataBit( GPIO_ICCARD_DETECT , PIN_ICCARD_DETECT )	/* 读IO口线状态 */

/*****************************POWER--->PC9****************************/

#define ICCARD_POWER_ON()  GPIO_SetBits(    GPIO_ICCARD_POWER , PIN_ICCARD_POWER )		   /* POWER = 1 */
#define ICCARD_POWER_OFF()  GPIO_ResetBits(  GPIO_ICCARD_POWER , PIN_ICCARD_POWER )          /* POWER = 0 */

/*****************************SHOCK_CS--->PD7****************************/

#define BMA250_SHOCK_CS_SET()  GPIO_SetBits(    GPIO_BMA250_SHOCK_CS , PIN_BMA250_SHOCK_CS )		   /* SHOCK_CS = 1 */
#define BMA250_SHOCK_CS_RESET()  GPIO_ResetBits(  GPIO_BMA250_SHOCK_CS , PIN_BMA250_SHOCK_CS ) 

/**************************宏定义I2C功能位***************************/

#define I2C_WR	0	               	/* 写控制bit */
#define I2C_RD	1	            	/* 读控制bit */

/*****************************各函数声明****************************/ 

extern void   ICCARD_M3_Init(void);

extern void ICCARD_M3_I2C_Delay(void);

extern void ICCARD_M3_I2C_Start(void);

extern void ICCARD_M3_I2C_Stop(void);

extern void ICCARD_M3_I2C_SendByte(u8 Byte );

extern u8 ICCARD_M3_I2C_ReadByte(void);

extern u8 ICCARD_M3_I2C_CheckAck(void);

extern void ICCARD_M3_I2C_SendAck(void);

extern void ICCARD_M3_I2C_SendNAck(void);

extern u8 ICCARD_M3_I2C_CheckDevice(u8 Address);

extern u8 Power_On(void);

extern u8 InByte(void);

extern void OutByte(u8 ch);

extern void Start_Comm(void);

extern void Stop_Comm(void);

extern void SendComm(u8 a,u8 b,u8 c);

extern void Proce_Mod(void);

extern void Read_Mod(u8  *pt,u8 i);

extern void RstCard(void);

extern void Rmm(u8   cardAdd, u8    *pt,u8    i);

void ICCARD_M3_I2C_Delay(void);
void M3_I2C_Delays(unsigned char n);
#endif 
/*******************************************************************************
 *                             end of module
 *******************************************************************************/
 
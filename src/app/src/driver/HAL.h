/******************************************************************** 
//版权说明	:Shenzhen E-EYE Co. Ltd., Copyright 2009- All Rights Reserved.
//文件名称	:hal.h		
//功能		:硬件层头文件
//版本号	:
//开发人	:yjb
//开发时间	:2012.03
//修改者	:
//修改时间	:
//修改简要说明	:
//备注		:
***********************************************************************/
#ifndef     _HAL_H_
#define     _HAL_H_

//C 标准库目录
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//STM32库目录
#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"

//驱动层
//#include "cpu.h"
#include "GPIOControl.h"
#include "Usart_drive.h"
#include "Adc.h"
#include "Can.h"
#include "I2C.h"

#endif

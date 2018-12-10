/*************************************************************************
* Copyright (c) 2013,深圳市伊爱高新技术开发有限公司
* All rights reserved.
* 文件名称 : CAN1_APP.h
* 当前版本 : 1.0
* 开发者   : Shigle
* 修改时间 : 20113年6月10日
*************************************************************************/

/***  包含文件  ***/ 
#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "include.h"


#if 0
void CAN2_Test_Tx( void )
{
  
    u32 time1 = 0xffffff;
    
    u8 CANTestArray[13]={ 
      0x02, 0xF1, 0xFE, 0x18, //扩展ID 0x18 FE F1 02
      0x08, //Bit4:1:扩展帧 0:标准帧  Bit3~Bit0: 长度
      0x11, 0x22,0x33,0x44,0x55,0x66,0x77,0x88 //数据域
    };
    
  //  Can2Bus_Send( CANTestArray, 13u); 
 
    while( time1-- );
    
}

#endif
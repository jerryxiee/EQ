/*************************************************************************
* Copyright (c) 2013,�������������¼����������޹�˾
* All rights reserved.
* �ļ����� : CAN1_APP.h
* ��ǰ�汾 : 1.0
* ������   : Shigle
* �޸�ʱ�� : 20113��6��10��
*************************************************************************/

/***  �����ļ�  ***/ 
#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "include.h"


#if 0
void CAN2_Test_Tx( void )
{
  
    u32 time1 = 0xffffff;
    
    u8 CANTestArray[13]={ 
      0x02, 0xF1, 0xFE, 0x18, //��չID 0x18 FE F1 02
      0x08, //Bit4:1:��չ֡ 0:��׼֡  Bit3~Bit0: ����
      0x11, 0x22,0x33,0x44,0x55,0x66,0x77,0x88 //������
    };
    
  //  Can2Bus_Send( CANTestArray, 13u); 
 
    while( time1-- );
    
}

#endif
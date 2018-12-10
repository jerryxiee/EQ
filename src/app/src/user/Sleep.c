/********************************************************************
//版权说明	:Shenzhen E-EYE Co. Ltd., Copyright 2009- All Rights Reserved.
//文件名称	:Sleep.c		
//功能		:
//版本号	:V0.1
//开发人	:dxl
//开发时间	:2012.6 
//修改者	:
//修改时间	:
//修改简要说明	:
//备注		:
***********************************************************************/

//********************************头文件************************************
#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "include.h"
//********************************自定义数据类型****************************

//********************************宏定义************************************

//********************************全局变量**********************************

//********************************外部变量**********************************

//********************************本地变量**********************************
static u16	AccOnCount = 0;
static u16	AccOffCount = 0;
#ifdef HUOYUN_DEBUG_OPEN
u8       SleepFlag = 0;//休眠标志,1为进入休眠,0为不休眠,货运平台要求ACC OFF后30分钟进入休眠上报
#endif
//********************************函数声明**********************************

/*********************************************************************
//函数名称	:Sleep_TimeTask(void)
//功能		:休眠定时任务
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
FunctionalState  Sleep_TimeTask(void)
{
        
	//u8	Buffer[REPORT_BUFFER_SIZE];//位置信息存储缓冲
	//u8	length;
        u8      Acc = 0;
        static u8      LastAcc = 0;
        
        Acc = Io_ReadStatusBit(STATUS_BIT_ACC);
        
        if(Acc != LastAcc)
        {
            //进入休眠或退出休眠的第一秒,上报一个点
            //上报位置信息                
	   // length = Report_GetPositionInformation(Buffer);
	   //RadioProtocol_PostionInformationReport(Buffer, length);//位置信息汇报
            
        }
        
	if(1 == Acc)
	{
               // SleepCount = 0;
                //LoopCount = 0;
		//检测麦克风
		//Io_WriteMicConrtolBit(MIC_BIT_TEST, SET);
		AccOffCount = 0;
		AccOnCount++;
		if(20 == AccOnCount)
		{
			CHARGE_ON(); 
			CAN_PWR_ON();
			
		}
		Io_WriteIoStatusBit(IO_STATUS_BIT_SLEEP, RESET);
               
#ifdef HUOYUN_DEBUG_OPEN
                 SleepFlag = 0;
#endif
	}
	else
	{
		AccOnCount = 0;
		AccOffCount++;
		if(20 == AccOffCount)
		{
			CHARGE_OFF();
			CAN_PWR_OFF();
                        //判断外设2类型是否设置为CAN模块
	                if(1 == ReadPeripheral2TypeBit(1))//bit1:1接了CAN盒子；0:没有接
	                {
		              NaviLcdPowerOff();
	                }
		}
                else if(1800 == AccOffCount)
                {
		        //检测麦克风
		        //Io_WriteMicConrtolBit(MIC_BIT_TEST, RESET);
                        Io_WriteIoStatusBit(IO_STATUS_BIT_SLEEP, SET);
#ifdef HUOYUN_DEBUG_OPEN
                        SleepFlag = 1;
#endif
                }
                
           
                
	}

        LastAcc = Acc;
        #if	TACHOGRAPHS_19056_TEST
        Tachographs_Test_Speed_Reduce();
		#endif
        return ENABLE;
      
}
/*********************************************************************
//函数名称	:Sleep_Init(void)
//功能		:休眠初始化
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void Sleep_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
  	
	/* 实用GPIO时钟*/
  	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);

  	/* 配置PG10脚 50Mhz推挽输出,弱上拉,用于控制备电充电 */
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;  //LED灯控制不能 设置上拉 GPIO_PuPd_UP
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(GPIOG, &GPIO_InitStructure);
}





/*
********************************************************************************
*
*版权说明	:Shenzhen E-EYE Co. Ltd., Copyright 2009- All Rights Reserved.
*文件名称	:main.c		
*功能		:主函数 
*版本号	    :V1.0
*创建时间	:20140818
*备注		:
*
********************************************************************************
*/

/*
********************************************************************************
*                               INCLUDES
********************************************************************************
*/
#include "include.h"

/*
********************************************************************************
*                          LOCAL VARIABLES
********************************************************************************
*/


/*
********************************************************************************
*                          GLOBAL VARIABLES
********************************************************************************
*/
const char ProductInfo[][17] =
{
    {"产品名称:       "},
    {"汽车行驶记录仪  "},
    {"产品型号:       "},
    {"EGS701          "},
    {"软件版本:V1.03  "},
    {"("FIRMWARE_VERSION")"},
    {"版本时间:       "},
    {FIRMWARE_TIME},
    {"制造商:         "},
    {"深圳市伊爱高新技"},
    {"术开发有限公司  "},
    {"                "},
    {"地址:           "},
    {"深圳市福田区八卦"},
    {"一路617栋6-7楼  "},
};

//制造商ID Bytes:5,eeprom可设置该参数,当读取eeprom错误时取这个默认值
const u8 ProductID[5]         ={ '7','0', '1', '0', '8'}; 

//终端型号ID Bytes:20
const u8 TerminalTypeID[20]   ={'E', 'G', 'S','7', '0','1'} ;

//dxl,2014.6.5与固件升级相关的常量
const char MANUFACTURER[]     = "MANUFACTURER:EI";

//含2个空格
const char PRODUCT_MODEL[]    = "PRODUCT MODEL:EGS701  ";

//含1个空格
const char PCB_VERSION[]      = "PCB VERSION:A ";

//与main.h中的FIRMWARE_VERSION保持一致，前面1位填充0,后5位一致。
//const char SOFTWARE_VERSION[] = "SOFTWARE VERSION:010131";
const char SOFTWARE_VERSION[] = "SOFTWARE VERSION:0"FIRMWARE_VERSION;
/*
********************************************************************************
*                          EXTERN VARIABLES
********************************************************************************
*/
extern u8 FirmwareVersion[];//固件版本号,固定为5字节


/*
********************************************************************************
*                           FUNCTIONS EXTERN
********************************************************************************
*/


/*
********************************************************************************
*                              FUNCTIONS
********************************************************************************
*/


/**
  * @brief  主函数
  * @param  None
  * @retval None
  */
int main(void)
{
  
    PeripheralInit();              //外设初始化
 
    IWDG_ReloadCounter();          //喂狗
	
	ShowStartLogo();               //上电显示LOG

	MemoryChipSelfCheck();         //存储芯片自检

	IWDG_ReloadCounter();          //喂狗
	
    PowerOnUpdata();               //上电更新一些变量
	
	LZM_TaskMenuStart();           //菜单初始化

    GPS_POWER_ON();
	
	Io_PowerOnDetectMainPower();   //检测是12V供电还是24V供电

		TimerInit();
	//写入固件版本号
	EepromPram_WritePram(E2_FIRMWARE_VERSION_ID, FIRMWARE_VERSION, E2_FIRMWARE_VERSION_ID_LEN);//dxl,2014.9.3,修改了FirmwareVersion+4
    
#ifdef WATCHDOG_OPEN  //只在进入while前开启，因为文件系统操作有些情况下有些函数会耗时很久,dxl,2014.9.9
	   //开启看门狗
	IDWG_Configuration();
#endif
       
	while(1)
	{
		TimerTaskScheduler();
		EvTaskScheduler();
		//TimerManage1s_Callback();
		//TimerManage10ms_Callback();
		print_ctrl_task();
		IWDG_ReloadCounter();
        #ifdef SLEEP_OPEN
                __WFI();
        #endif
	}
}



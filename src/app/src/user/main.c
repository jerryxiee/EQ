/*
********************************************************************************
*
*��Ȩ˵��	:Shenzhen E-EYE Co. Ltd., Copyright 2009- All Rights Reserved.
*�ļ�����	:main.c		
*����		:������ 
*�汾��	    :V1.0
*����ʱ��	:20140818
*��ע		:
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
    {"��Ʒ����:       "},
    {"������ʻ��¼��  "},
    {"��Ʒ�ͺ�:       "},
    {"EGS701          "},
    {"����汾:V1.03  "},
    {"("FIRMWARE_VERSION")"},
    {"�汾ʱ��:       "},
    {FIRMWARE_TIME},
    {"������:         "},
    {"�������������¼�"},
    {"���������޹�˾  "},
    {"                "},
    {"��ַ:           "},
    {"�����и���������"},
    {"һ·617��6-7¥  "},
};

//������ID Bytes:5,eeprom�����øò���,����ȡeeprom����ʱȡ���Ĭ��ֵ
const u8 ProductID[5]         ={ '7','0', '1', '0', '8'}; 

//�ն��ͺ�ID Bytes:20
const u8 TerminalTypeID[20]   ={'E', 'G', 'S','7', '0','1'} ;

//dxl,2014.6.5��̼�������صĳ���
const char MANUFACTURER[]     = "MANUFACTURER:EI";

//��2���ո�
const char PRODUCT_MODEL[]    = "PRODUCT MODEL:EGS701  ";

//��1���ո�
const char PCB_VERSION[]      = "PCB VERSION:A ";

//��main.h�е�FIRMWARE_VERSION����һ�£�ǰ��1λ���0,��5λһ�¡�
//const char SOFTWARE_VERSION[] = "SOFTWARE VERSION:010131";
const char SOFTWARE_VERSION[] = "SOFTWARE VERSION:0"FIRMWARE_VERSION;
/*
********************************************************************************
*                          EXTERN VARIABLES
********************************************************************************
*/
extern u8 FirmwareVersion[];//�̼��汾��,�̶�Ϊ5�ֽ�


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
  * @brief  ������
  * @param  None
  * @retval None
  */
int main(void)
{
  
    PeripheralInit();              //�����ʼ��
 
    IWDG_ReloadCounter();          //ι��
	
	ShowStartLogo();               //�ϵ���ʾLOG

	MemoryChipSelfCheck();         //�洢оƬ�Լ�

	IWDG_ReloadCounter();          //ι��
	
    PowerOnUpdata();               //�ϵ����һЩ����
	
	LZM_TaskMenuStart();           //�˵���ʼ��

    GPS_POWER_ON();
	
	Io_PowerOnDetectMainPower();   //�����12V���绹��24V����

		TimerInit();
	//д��̼��汾��
	EepromPram_WritePram(E2_FIRMWARE_VERSION_ID, FIRMWARE_VERSION, E2_FIRMWARE_VERSION_ID_LEN);//dxl,2014.9.3,�޸���FirmwareVersion+4
    
#ifdef WATCHDOG_OPEN  //ֻ�ڽ���whileǰ��������Ϊ�ļ�ϵͳ������Щ�������Щ�������ʱ�ܾ�,dxl,2014.9.9
	   //�������Ź�
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



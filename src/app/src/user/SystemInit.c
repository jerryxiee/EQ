/********************************************************************
//��Ȩ˵��	:Shenzhen E-EYE Co. Ltd., Copyright 2009- All Rights Reserved.
//�ļ�����	:SystemInit.c		
//����		:ϵͳ��ʼ�� 
//�汾��	:
//������	:dxl
//����ʱ��	:2012.4
//�޸���	:
//�޸�ʱ��	:
//�޸ļ�Ҫ˵��	:
//��ע		:
***********************************************************************/
//***************�����ļ�*****************
#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "include.h"
//#include "Polygon.h"
//***************���ر���******************
//***************�ⲿ����*****************
extern u8      MultiCenterLinkFlag;//���������ӱ�׼,1Ϊ������Ҫ�����ӱ��ݷ�����
//extern u8	SdOnOffLineFlag;//SD�������־,0δ����,1����
extern u8	RadioProtocolRx1Buffer[];//����1���ջ���
extern Queue  RadioProtocolRx1Q;//����1���ն���
extern u8	RadioProtocolRx2Buffer[];//����2���ջ���
extern Queue  RadioProtocolRx2Q;//����2���ն���
extern u8	RadioProtocolRxSmsBuffer[];//���Ž��ջ���
extern Queue  RadioProtocolRxSmsQ;//���Ž��ն���
extern pCirQueue RadioProtocolRx3Q;

//****************ȫ�ֱ���*****************
u8 FirmwareVersion[] = FIRMWARE_VERSION;//�̼��汾��,�̶�Ϊ5�ֽ�
//u8	FirstRunFlag = 0;//��1��ʹ�ñ�־
u8      IC_Reader_flag=0;//0��ʾ�ϵ�PCB�汾,1��ʾ���ɱ����PCB�汾
//************** �궨��*******************
#define USER_PLLI2S_N   426
#define USER_PLLI2S_R   4
//**************��������*******************
/*********************************************************************
//��������	:PowerOnUpdata
//����		:�ϵ���±���
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:���µı����У�IO��GPS��λ�û㱨������ͷ
*********************************************************************/
void PowerOnUpdata(void)
{
        //����һ��ʹ�ñ�־
	u8	Buffer[6];
	//Doubt_UpdataPram();
	//��ʻ��¼�Ǵ洢�����Լ�
	if(0 == Register_CheckArea())
	{
		//�Լ�����
	}
	else
	{
		//�Լ����
	}
	//����IO����
	Io_UpdataPram();
	//����GPS����
        Gps_PowerOnUpdataPosition();
	//����������ʱ����
	HeartBeat_UpdatePram();
	//�����������
	Pulse_UpdataPram();
	//�����ٶ����ͱ���
	SpeedFlagUpdatePram();
	//���³��ٱ���
	SpeedMonitor_ParamInitialize();   
	//����ƣ�ͼ�ʻ����
	TiredDrive_ParamInitialize();
	//����GPSģ�鿪�ر���
	UpdataAccOffGpsControlFlag();
	 //�����ʼ��
	Area_ParameterInitialize(); 
	 //TTS������ʼ��
        TtsVolumeInit();
	//����ϵ��У׼��ʼ��
        Adjust_Init();
	

	Buffer[0] = 0;
	EepromPram_ReadPram(E2_FIRST_POWERON_FLAG_ID, Buffer);
	if((0xff == Buffer[0])||(0 == Buffer[0]))
	{
		EepromPram_DefaultSet();
		Buffer[0] = 1;
		EepromPram_WritePram(E2_FIRST_POWERON_FLAG_ID, Buffer, 1);
		
	}
	//�����ֻ�����
	RadioProtocol_UpdateTerminalPhoneNum();

        //��������ʼ��
        NaviLcd_ParameterInitialize();
        
	//¼�������ʼ��
	RecordTask_Init(); 

        ///////���ٿ�����ʼ��/////////////////
        OpenAccount_ParameterInitialize();
        
        //�����������ӱ�־,�б���ʱʹ��
        //FRAM_BufferRead(&MultiCenterLinkFlag, FRAM_MULTI_CENTER_LINK_LEN, FRAM_MULTI_CENTER_LINK_ADDR);

	//������߳�ʼ��
   	SleepDeep_ParameterInitialize();
        
        Polygon_ParameterInitialize();
        //����·����
        OilControl_ParameterInitialize();
        //���¹յ㲹������
        Corner_UpdataPram();
         //��ȡ�����궨����������ϵ��
        OilWear_UpdatePram();//lzm,2014.9.24
        
        
    
        //����Э�������������ѭ������
        RadioProtocolRx1Q = QueueCreate1(RADIO_PROTOCOL_RX1_BUFFER_SIZE);
        RadioProtocolRx2Q = QueueCreate2(RADIO_PROTOCOL_RX2_BUFFER_SIZE);
        RadioProtocolRxSmsQ = QueueCreate2(RADIO_PROTOCOL_RX_SMS_BUFFER_SIZE);

				RadioProtocolRx3Q = CreatCirQueue(256);
				if(NULL == RadioProtocolRx3Q)
				{
					while(1);
				}
}
/*********************************************************************
//��������	:MemoryChipSelfCheck
//����		:�ϵ�洢оƬ�Լ�
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:�Լ�Ĵ洢оƬ�У�FRAM��FLASH��EEPROM
*********************************************************************/
void MemoryChipSelfCheck(void)
{

	u8	flag = 0;
	u16	i;	
        u32	Delay = 0;
        
	if((ERROR == CheckFlashChip())&&(0 == flag))//���flash
	{
		for(i=0; i<10000; i++)
		{
			//��ʱһ��ʱ��,�ٴμ��
		}
		if((ERROR == CheckFlashChip())&&(0 == flag))//���flash
		{
			flag = 1;
		}
		
	}
        IWDG_ReloadCounter();//ι��
	if((ERROR ==  CheckFramChip())&&(0 == flag))//���fram
	{
		for(i=0; i<10000; i++)
		{
			//��ʱһ��ʱ��,�ٴμ��
		}
		if((ERROR ==  CheckFramChip())&&(0 == flag))//���fram
		{
			flag = 2;
		}
	}
        IWDG_ReloadCounter();//ι��
	if((ERROR ==  CheckEepromChip())&&(0 == flag))//���eeprom
	{
		for(i=0; i<10000; i++)
		{
			//��ʱһ��ʱ��,�ٴμ��
		}
		if((ERROR ==  CheckEepromChip())&&(0 == flag))//���eeprom
		{
			flag = 3;
		}
		
	}
        IWDG_ReloadCounter();//ι��
	if(1 == flag)//FLASH����
	{
                LcdClearScreen(); 
                LcdShowCaptionEx((char *)"flash�Լ����",2); 
                Delay  = Timer_Val();
	        while((Timer_Val() - Delay) < (20*600));//��ʱ10����
		//���ʹ�����Ϣ��������
		while(1)
		{
			
			
		}
	}
	else if(2 == flag)//FRAM����
	{
		LcdClearScreen(); 
                LcdShowCaptionEx((char *)"fram�Լ����",2); 
                Delay  = Timer_Val();
	        while((Timer_Val() - Delay) < (20*600));//��ʱ10����
		//���ʹ�����Ϣ��������
		while(1)
		{
			
			
		}
	}
	else if(3 == flag)//EEPROM����
	{
		LcdClearScreen(); 
                LcdShowCaptionEx((char *)"eeprom�Լ����",2); 
                Delay  = Timer_Val();
	        while((Timer_Val() - Delay) < (20*600));//��ʱ10����
		//���ʹ�����Ϣ��������
		while(1)
		{
			
			
		}
	}
}
/*********************************************************************
//��������	:PeripheralInit
//����		:�����ʼ��
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:��ʼ���������У�GPS��GPRS��FRAM��FLASH��EEPROM������ͷ
*********************************************************************/

void PeripheralInit(void)
{
	///u32	i;
	u32	time;
        u32     Delay;
	
	RCC_ClocksTypeDef  RCC_Clocks;
        
    for(Delay=0; Delay<100; Delay++)
    {
        
    }
	
	//��ʼ�������жϵ�ַ
	NVIC_Configuration(0x4000);
       
        
	//��ʼ��ʱ��
	RCC_GetClocksFreq(&RCC_Clocks);
	
	SysTick_Config(96000000/20);//50ms
    //
    #if 0//��дͨѶģ�����
    Modem_Bsp_Init();
    Modem_Bsp_SetPower(1);
    Modem_Bsp_SetActive(0); 
    for(;;)
    {
        ;
    }
    #endif
    
    Delay  = Timer_Val();
	while((Timer_Val() - Delay) < 10);//�ϵ���ʱ0.5s
      

	
	//����ģ���ʼ��
	Pulse_Init();
	
	//��ʼ��RTC
	InitRtc();
	
	//AD��ʼ��
	Ad_Init();
	
	//GPIO��ʼ��
	Io_Init();
     
    SD_POWER_OFF();//�ȹر�SD����Դ����
        
    //Ӳ����λ��ʾ��,ʹ�õ���USB��Դ��������
    USB_SW_Init();
    USB_SW_0();
    Delay  = Timer_Val();
    while((Timer_Val() - Delay) < 5);//�ϵ���ʱ0.25s
    USB_SW_1();
    Delay  = Timer_Val();
    while((Timer_Val() - Delay) < 5);//�ϵ���ʱ0.25s
  
    //Һ������ʼ��
	Lcd_Initial();
        
        
	//FLASHģ���ʼ�� 
    //LcdClearScreen(); 
    //LcdShowCaptionEx((char *)"flash��ʼ��",2); 
	sFLASH_DeInit();
	sFLASH_Init();
	SPI_FLASH_AllUnprotect();
        
    LcdClearScreen(); 
    LcdShowCaptionEx((char *)"eeprom��ʼ��",2); 
	//EEPROMģ���ʼ��
	E2prom_InitIO();
      
        UpdatePeripheralType();//��������1������2���ͱ���,dxl,2014.12.26,���ǵ�����ģ����õ�����1���ͻ�����2����,��˷�����ǰ��
	
	//FRAMģ���ʼ��
    LcdClearScreen(); 
    LcdShowCaptionEx((char *)"fram��ʼ��",2); 
	FRAM_Init();
       
	//GPSģ���ʼ��,ʹ�ô���4
    LcdClearScreen(); 
    LcdShowCaptionEx((char *)"gps��ʼ��",2); 
	Gps_Init();
	
	//����4��ʼ��,������,������
	//Comm485UartInit(); 
        
	//DB9���ڳ�ʼ��,ʹ�ô���1,��׼Ҫ��115200������
	COM1_Init(115200);
    LOG_SetUsart( COM1 );
	
	//TTS��ʼ��,ʹ��ģ�⴮�ڷ���,����δʹ��
    LcdClearScreen(); 
    LcdShowCaptionEx((char *)"tts��ʼ��",2); 
	TTS_Init();
	
	//��ӡ����ʼ��
    LcdClearScreen(); 
    LcdShowCaptionEx((char *)"printer��ʼ��",2); 
	print_init();
	
	//USB��ʼ��
    LcdClearScreen(); 
    LcdShowCaptionEx((char *)"usb��ʼ��",2); 
	USB_AppInitialize();
	
	//IC����ʼ����ʹ�ô���6
    
    //���PD5���ţ��Ը������ж��ǽ��ɱ����Ӳ��PCB���ǽ��ɱ�ǰ��Ӳ��PCB,
    //dxl,ע��ÿ�Ϊ���ÿڣ�ͬʱ������˷���ƣ���������Ϊ���룬Ȼ������Ϊ���
	//while((Timer_Val() - Delay) < 2);//�ϵ���ʱ0.1s,mq,2014.5.10
   
    
	SD_POWER_ON();//��SD������
	time  = Timer_Val();
	while((Timer_Val() - time) < 2);//��ʱ0.1s
        
    LcdClearScreen(); 
    LcdShowCaptionEx((char *)"ICCard��ʼ��",2); 
	ICCardBase_ParameterInitialize();
	
	//GPRSģ���ʼ��,ʹ�ô���5
    LcdClearScreen(); 
    LcdShowCaptionEx((char *)"gprs��ʼ��",2); 
	Communication_Init();

	//���߳�ʼ��,��Ҫ�Ǳ��������
    LcdClearScreen(); 
    LcdShowCaptionEx((char *)"sleep��ʼ��",2); 
	Sleep_Init();
        
	//����5��ʼ��
	//TaximeterComInit();
	
	//����ͷģ���ʼ��,ʹ�ô���3
    LcdClearScreen(); 
    LcdShowCaptionEx((char *)"camera��ʼ��",2); 
	Camera_Init();

    LcdClearScreen(); 
    LcdShowCaptionEx((char *)"can��ʼ��",2); 
	//CAN���߳�ʼ��
    CAN_TimeTask_Init();
		
	
	// SD SPI��ʼ
    Media_CheckSdOnOff();//dxl,2014.6.18,���SD���ڲ�����Ҫ����InitSDFile֮ǰ,��Ϊ��InitSDFile���滹����SD���Ƿ��𻵣������˵�ͬ�ڲ���
	LcdClearScreen(); 
    LcdShowCaptionEx((char *)"sd��ʼ��",2); 
	SPIx_Init(); 
    f_mount(1,&fs);
    InitSDFile();//�����ڴ˴������ν�����������ˣ�ż���ο���
	
	LcdClearScreen(); 
    LcdShowCaptionEx((char *)"TF���ļ����",2); 
	Media_FolderCheck();
	
	
	//��·��ʼ��
    LcdClearScreen(); 
    LcdShowCaptionEx((char *)"route��ʼ��",2); 
	InitRoute();
   
    //��ģ���ʼ��    
    LcdClearScreen(); 
    LcdShowCaptionEx((char *)"bma��ʼ��",2); 
    //InitBma250E_App();dxl,2014.4.24��Ҫ���Σ���������ǿ�޸ĺ���г�ͻ
        
    //GNSS��ϸ��Ϣ�ɼ���ʼ
    InitConfigGNSS();
	
	//��ʼ����ý������ʹ�ô���2
#if (ICCARD_JTD == ICCARD_SEL)
    {
    	COM2_Init(115200);
	}
#else
	{
		COM2_Init(9600);
	}
#endif     
     //���س�ʼ��
     CarLoad_ParamInitialize();

    Public_ParameterInitialize();
    
    //CAN���ӳ�ʼ��
     Can_ErQi_Init();
     
     //Ϊ����ʱ����MANUFACTURER���������Ż�����ȥ���ˣ����������´���,dxl,2014.6.5
     (void)&MANUFACTURER;
     (void)&PRODUCT_MODEL;
     (void)&PCB_VERSION;
     (void)&SOFTWARE_VERSION;
	
}
/*******************************************************************************
* Function Name  : RCC_Configuration
* Description    : Configures the different system clocks.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RCC_Configuration(void)
{
	ErrorStatus HSEStartUpStatus;
	/* RCC system reset(for debug purpose) */
	RCC_DeInit();
	
	/* Enable HSE */
	RCC_HSEConfig(RCC_HSE_ON);
	
	/* Wait till HSE is ready */
	HSEStartUpStatus = RCC_WaitForHSEStartUp();
	
	if(HSEStartUpStatus == SUCCESS)
	{
		/* Enable Prefetch Buffer */
		FLASH_PrefetchBufferCmd(ENABLE);
		
		/* Flash 2 wait state */
		FLASH_SetLatency(FLASH_Latency_2);
		
		/* Enables or disables the Instruction Cache */
		FLASH_InstructionCacheCmd(ENABLE); 
		
		/* Enables or disables the Data Cache */
		FLASH_DataCacheCmd(ENABLE);
		
		/* HCLK = SYSCLK */
		RCC_HCLKConfig(RCC_SYSCLK_Div1); 
		
		/* PCLK2 = HCLK/2 */
		RCC_PCLK2Config(RCC_HCLK_Div2); 
		
		/* PCLK1 = HCLK/4 */
		RCC_PCLK1Config(RCC_HCLK_Div4);
		
		/* PLLCK= (HSECLK/PLL_M)*PLL_N */
		RCC_PLLConfig(RCC_PLLSource_HSE, USER_PLL_M, USER_PLL_N, USER_PLL_P, USER_PLL_Q);
		
		/* Enable PLL */ 
		RCC_PLLCmd(ENABLE);
		
		/* Wait till PLL is ready */
		while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
		{
		}
		
		/* Select PLL as system clock source */
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
		
		/* Wait till PLL is used as system clock source */
		while (RCC_GetSYSCLKSource() != RCC_CFGR_SWS_PLL)
		{
			
		}
		
		/******************************************************************************/
		/*            I2S clock configuration (For devices Rev B and Y)               */
		/******************************************************************************/
  		/* PLLI2S clock used as I2S clock source */
  		RCC->CFGR &= ~RCC_CFGR_I2SSRC;

  		/* Configure PLLI2S */
  		RCC->PLLI2SCFGR = (USER_PLLI2S_N << 6) | (USER_PLLI2S_R << 28);

  		/* Enable PLLI2S */
  		RCC->CR |= ((uint32_t)RCC_CR_PLLI2SON);

  		/* Wait till PLLI2S is ready */
  		while((RCC->CR & RCC_CR_PLLI2SRDY) == 0)
  		{
			
  		}
	}     
}
/*******************************************************************************
* Function Name  : IDWG_Configuration
* Description    : Configures IDWG clock source and prescaler.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void IDWG_Configuration(void)
{
#ifdef WATCHDOG_OPEN
/* Enable the LSI oscillator ************************************************/
  RCC_LSICmd(ENABLE);
  
  /* Wait till LSI is ready */
  while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
  {}
  /* Enable write access to IWDG_PR and IWDG_RLR registers */
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

  /* IWDG counter clock: 40KHz(LSI) / 32 = 1.25 KHz */
  //IWDG_SetPrescaler(IWDG_Prescaler_32);
    IWDG_SetPrescaler(IWDG_Prescaler_128);/*40KHz(LSI) / 128 = 320Hz */
    
  /* Set counter reload value to 2500 */
  IWDG_SetReload(0x9c4);//2��
    //IWDG_SetReload(0x4E2);//1��
  //IWDG_SetReload(0x271);//0.5��
  //IWDG_SetReload(0xFA);//0.2��


  /* Reload IWDG counter */
  IWDG_ReloadCounter();

  /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
  IWDG_Enable();  
#endif
}

/*******************************************************************************
* Function Name  : PWR_PVDInit
* Description    : ��ԴPVD��ʼ��
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PWR_PVDInit(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;
	
	PWR_PVDLevelConfig(PWR_PVDLevel_7);
	PWR_PVDCmd(ENABLE);
	EXTI_StructInit(&EXTI_InitStructure);
	EXTI_InitStructure.EXTI_Line = EXTI_Line16;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
}
/*******************************************************************************
* Function Name  : NVIC_Configuration
* Description    : Configures the nested vectored interrupt controller.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void NVIC_Configuration(u32 Address)
{
  NVIC_InitTypeDef NVIC_InitStructure;


  /* Set the Vector Table base location at 0x08000000 */ 
  NVIC_SetVectorTable(NVIC_VectTab_FLASH, Address);   

  /* Configure the NVIC Preemption Priority Bits */  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
  
/* Enable the PVD Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = PVD_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

/* Enable the USART1 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  /* Enable the USART2 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  /* Enable the USART3 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
   /* Enable the UART4 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  /* Enable the UART5 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  /* Enable the USART6 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART6_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  // Enable the RTC Interrupt 
  NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
 
}
/*******************************************************************************
* Function Name  : USB_SW_Init
* Description    : USB�������ų�ʼ��������������Ӳ����λLCD��
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void   USB_SW_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
  
/******************************IO***********************************/    
    RCC_AHB1PeriphClockCmd(  RCC_USB_SW , ENABLE);	       /* ��GPIOʱ�� */

    GPIO_InitStructure.GPIO_Pin = PIN_USB_SW  ;
    
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;        /* ����ģʽ */ 
    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;  	         /* ���/���� */
    
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;           /* ��© */
    
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;             /* ���� */
    
    GPIO_Init( GPIO_USB_SW , &GPIO_InitStructure);
    
    USB_SW_1();
}

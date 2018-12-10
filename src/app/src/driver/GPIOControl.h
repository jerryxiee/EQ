/********************************************************************
//版权说明	:
//文件名称	:GPIOControl.h		
//功能		:GPIO 输入/输出控制
//版本号	:
//开发人	:yjb
//开发时间	:2013.03.09
//修改者	:
//修改时间	:
//修改简要说明	:
//备注		:1)
//               
***********************************************************************/
#ifndef __GPIOCONTROL_H
#define __GPIOCONTROL_H

#ifdef __cplusplus
 extern "C" {
#endif
   
//****************包含文件*****************   
#include "stm32f2xx.h"
   
 
//****************宏定义********************
   
//**************输出脚*******************
#define DLIGHTI_PIN                     GPIO_Pin_13
#define DLIGHTI_GPIO_PORT               GPIOE
#define DLIGHTI_GPIO_CLK                RCC_AHB1Periph_GPIOE

#define LLIGHTI_PIN                     GPIO_Pin_10
#define LLIGHTI_GPIO_PORT               GPIOE
#define LLIGHTI_GPIO_CLK                RCC_AHB1Periph_GPIOE
   
#define BREAK_PIN                       GPIO_Pin_8
#define BREAK_GPIO_PORT                 GPIOE
#define BREAK_GPIO_CLK                  RCC_AHB1Periph_GPIOE   
   
#define ACC_PIN                         GPIO_Pin_7
#define ACC_GPIO_PORT                   GPIOE
#define ACC_GPIO_CLK                    RCC_AHB1Periph_GPIOE

#define DOORI_N_PIN                         GPIO_Pin_5
#define DOORI_N_GPIO_PORT                   GPIOC
#define DOORI_N_GPIO_CLK                    RCC_AHB1Periph_GPIOC

#define DOORI_P_PIN                         GPIO_Pin_1
#define DOORI_P_GPIO_PORT                   GPIOG
#define DOORI_P_GPIO_CLK                    RCC_AHB1Periph_GPIOG

#define DOORI2_PIN                         GPIO_Pin_15//dxl,2013.11.5,原来是PE15,错了是PF15
#define DOORI2_GPIO_PORT                   GPIOF
#define DOORI2_GPIO_CLK                    RCC_AHB1Periph_GPIOF

#define LAMP_PIN                            GPIO_Pin_4
#define LAMP_GPIO_PORT                      GPIOC
#define LAMP_GPIO_CLK                       RCC_AHB1Periph_GPIOC  

#define LOCK_OUT_PIN                         GPIO_Pin_0
#define LOCK_OUT_GPIO_PORT                   GPIOB
#define LOCK_OUT_GPIO_CLK                    RCC_AHB1Periph_GPIOB     

#define UNLOCK_OUT_PIN                         GPIO_Pin_1
#define UNLOCK_OUT_GPIO_PORT                   GPIOB
#define UNLOCK_OUT_GPIO_CLK                    RCC_AHB1Periph_GPIOB    
   
#define XLIGHTI_PIN                         GPIO_Pin_14
#define XLIGHTI_GPIO_PORT                   GPIOE
#define XLIGHTI_GPIO_CLK                    RCC_AHB1Periph_GPIOE    

#define RLIGHTI_PIN                         GPIO_Pin_12
#define RLIGHTI_GPIO_PORT                   GPIOE
#define RLIGHTI_GPIO_CLK                    RCC_AHB1Periph_GPIOE    


#define BKHI_PIN                         GPIO_Pin_9
#define BKHI_GPIO_PORT                   GPIOE
#define BKHI_GPIO_CLK                    RCC_AHB1Periph_GPIOE     

#define SWITCHI_PIN                GPIO_Pin_14
#define SWITCHI_GPIO_PORT          GPIOF
#define SWITCHI_GPIO_CLK           RCC_AHB1Periph_GPIOF  
   
#define HORNI_PIN                GPIO_Pin_0
#define HORNI_GPIO_PORT          GPIOG
#define HORNI_GPIO_CLK           RCC_AHB1Periph_GPIOG    

#define VIRBRATIONI_PIN                GPIO_Pin_13
#define VIRBRATIONI_GPIO_PORT          GPIOF
#define VIRBRATIONI_GPIO_CLK           RCC_AHB1Periph_GPIOF      

#define COIL_PIN                GPIO_Pin_2
#define COIL_GPIO_PORT          GPIOB
#define COIL_GPIO_CLK           RCC_AHB1Periph_GPIOB   

#define HORN_OUT_PIN                GPIO_Pin_11
#define HORN_OUT_GPIO_PORT          GPIOF
#define HORN_OUT_GPIO_CLK           RCC_AHB1Periph_GPIOF   

#define DETCT1I_PIN                GPIO_Pin_11
#define DETCT1I_GPIO_PORT          GPIOB
#define DETCT1I_GPIO_CLK           RCC_AHB1Periph_GPIOB  
   
#define DETCT2I_PIN                GPIO_Pin_15
#define DETCT2I_GPIO_PORT          GPIOE
#define DETCT2I_GPIO_CLK           RCC_AHB1Periph_GPIOE     


#define GPS_OC_PIN                GPIO_Pin_2
#define GPS_OC_GPIO_PORT          GPIOF
#define GPS_OC_GPIO_CLK           RCC_AHB1Periph_GPIOF     

#define GPS_SC_PIN                GPIO_Pin_13
#define GPS_SC_GPIO_PORT          GPIOC
#define GPS_SC_GPIO_CLK           RCC_AHB1Periph_GPIOC    

#define GPS_CTR_PIN                GPIO_Pin_6
#define GPS_CTR_GPIO_PORT          GPIOE
#define GPS_CTR_GPIO_CLK           RCC_AHB1Periph_GPIOE      

#define MEM_WP_PIN                GPIO_Pin_15
#define MEM_WP_GPIO_PORT          GPIOA
#define MEM_WP_GPIO_CLK           RCC_AHB1Periph_GPIOA    
   
#define FLASH_CS_PIN                GPIO_Pin_4
#define FLASH_CS_GPIO_PORT          GPIOA
#define FLASH_CS_GPIO_CLK           RCC_AHB1Periph_GPIOA      

#define FR_CS_PIN                GPIO_Pin_8
#define FR_CS_GPIO_PORT          GPIOG
#define FR_CS_GPIO_CLK           RCC_AHB1Periph_GPIOG   

#define EPROM_CS_PIN                GPIO_Pin_4
#define EPROM_CS_GPIO_PORT          GPIOG
#define EPROM_CS_GPIO_CLK           RCC_AHB1Periph_GPIOG  

#define GSM_ON_PIN                GPIO_Pin_4
#define GSM_ON_GPIO_PORT          GPIOD
#define GSM_ON_GPIO_CLK           RCC_AHB1Periph_GPIOD  
   
#define GSM_PWC_PIN                GPIO_Pin_10
#define GSM_PWC_GPIO_PORT          GPIOC
#define GSM_PWC_GPIO_CLK           RCC_AHB1Periph_GPIOC     
   
#define GSM_DTR_PIN                GPIO_Pin_5
#define GSM_DTR_GPIO_PORT          GPIOG
#define GSM_DTR_GPIO_CLK           RCC_AHB1Periph_GPIOG    
   
#define GSM_RI_PIN                GPIO_Pin_13
#define GSM_RI_GPIO_PORT          GPIOG
#define GSM_RI_GPIO_CLK           RCC_AHB1Periph_GPIOG     
   
#define CAM_POWER_PIN                GPIO_Pin_12
#define CAM_POWER_GPIO_PORT          GPIOF
#define CAM_POWER_GPIO_CLK           RCC_AHB1Periph_GPIOF       
   
#define CAM_RD_PIN                GPIO_Pin_10
#define CAM_RD_GPIO_PORT          GPIOD
#define CAM_RD_GPIO_CLK           RCC_AHB1Periph_GPIOD       
   
#define TTS_RDY_PIN                GPIO_Pin_6
#define TTS_RDY_GPIO_PORT          GPIOG
#define TTS_RDY_GPIO_CLK           RCC_AHB1Periph_GPIOG   

#define TTS_P_PIN                GPIO_Pin_7
#define TTS_P_GPIO_PORT          GPIOG
#define TTS_P_GPIO_CLK           RCC_AHB1Periph_GPIOG     
  
#define LCD_ON_PIN                GPIO_Pin_11
#define LCD_ON_GPIO_PORT          GPIOC
#define LCD_ON_GPIO_CLK           RCC_AHB1Periph_GPIOC      

#define LCD_CS_PIN                GPIO_Pin_15
#define LCD_CS_GPIO_PORT          GPIOG
#define LCD_CS_GPIO_CLK           RCC_AHB1Periph_GPIOG    
   
#define LCD_AO_PIN                GPIO_Pin_3
#define LCD_AO_GPIO_PORT          GPIOD
#define LCD_AO_GPIO_CLK           RCC_AHB1Periph_GPIOD    
   
#define LCD_SPI_SCK_PIN                GPIO_Pin_0
#define LCD_SPI_SCK_GPIO_PORT          GPIOE
#define LCD_SPI_SCK_GPIO_CLK           RCC_AHB1Periph_GPIOE       

#define LCD_SPI_MISO_PIN                GPIO_Pin_8
#define LCD_SPI_MISO_GPIO_PORT          GPIOC
#define LCD_SPI_MISO_GPIO_CLK           RCC_AHB1Periph_GPIOC 
   
#define LCD_SPI_MOSI_PIN                GPIO_Pin_1
#define LCD_SPI_MOSI_GPIO_PORT          GPIOE
#define LCD_SPI_MOSI_GPIO_CLK           RCC_AHB1Periph_GPIOE    

#define CAN_CTR_MOSI_PIN                GPIO_Pin_15
#define CAN_CTR_MOSI_GPIO_PORT          GPIOD
#define CAN_CTR_MOSI_GPIO_CLK           RCC_AHB1Periph_GPIOD    

#define SHOCK_CS_MOSI_PIN                GPIO_Pin_7
#define SHOCK_CS_MOSI_GPIO_PORT          GPIOD
#define SHOCK_CS_MOSI_GPIO_CLK           RCC_AHB1Periph_GPIOD   
 
#define V_COL_MOSI_PIN                GPIO_Pin_7
#define V_COL_MOSI_GPIO_PORT          GPIOC
#define V_COL_MOSI_GPIO_CLK           RCC_AHB1Periph_GPIOC    

#define IC_P_MOSI_PIN                GPIO_Pin_9
#define IC_P_MOSI_GPIO_PORT          GPIOC
#define IC_P_MOSI_GPIO_CLK           RCC_AHB1Periph_GPIOC     
   
#define BUZZER_PIN                      GPIO_Pin_12
#define BUZZER_GPIO_PORT                GPIOG
#define BUZZER_GPIO_CLK                 RCC_AHB1Periph_GPIOG
   
#define	MIC_EN_PIN			GPIO_Pin_5//dxl,2013.5.15,PD5与IC卡检测口共用
#define MIC_EN_GPIO_PORT                GPIOD
#define MIC_EN_GPIO_CLK                 RCC_AHB1Periph_GPIOD 
	 
#define	I2S_AD_DA_PIN			GPIO_Pin_2//dxl,2013.5.16,PG2
#define I2S_AD_DA_GPIO_PORT             GPIOG
#define I2S_AD_DA_GPIO_CLK              RCC_AHB1Periph_GPIOG 
	 
#define	I2S_PWDAN_PIN			GPIO_Pin_12//dxl,2013.5.16,PD12
#define I2S_PWDAN_GPIO_PORT             GPIOD
#define I2S_PWDAN_GPIO_CLK              RCC_AHB1Periph_GPIOD 
	 
#define	I2S_PWADN_PIN			GPIO_Pin_11//dxl,2013.5.16,PD11
#define I2S_PWADN_GPIO_PORT             GPIOD
#define I2S_PWADN_GPIO_CLK              RCC_AHB1Periph_GPIOD 
	 
#define	SD_POWER_PIN			GPIO_Pin_3//dxl,2013.5.16,PG3
#define SD_POWER_GPIO_PORT             GPIOG
#define SD_POWER_GPIO_CLK              RCC_AHB1Periph_GPIOG 
   
#define PWR_TYPE_PIN                    GPIO_Pin_3//dxl,2013.5.26,PF3  
#define PWR_TYPE_GPIO_PORT             GPIOF
#define PWR_TYPE_GPIO_CLK              RCC_AHB1Periph_GPIOF 
	 
#define SD_DETECT_PIN                    GPIO_Pin_13//dxl,2013.8.16,PD13 
#define SD_DETECT_GPIO_PORT             GPIOD
#define SD_DETECT_GPIO_CLK              RCC_AHB1Periph_GPIOD 
	 
//****************结构定义*******************   
//输出脚,输出脚编号
typedef enum 
{
  //CON5接口,32PIN接口
  DLIGHTI = 0,         //大灯(远光灯),输入,高有效(正触发),别名[BIG_LIGHT]
  LLIGHTI,             //左转灯,输入,高有效(正触发),别名[RIGHT_LIGHT]
  BREAK,               //刹车,输入,高有效(正触发)
  ACC,                 //ACC,输入,高有效(正触发)
  DOORI_N,             //车门1,输入,低有效(负触发)
  DOORI_P,             //车门1,输入,高有效(正触发)
  DOORI2,              //车门2,输入,可配置,默认低有效(负触发)
  LAMP,                //转向灯,输出,别名[TURN_LIGHT]
  LOCK_OUT,            //中控锁-关,输出,
  UNLOCK_OUT,          //中控锁-开,输出,
  XLIGHTI,             //小灯,输入,高有效,别名[LITTLE_LIGHT]
  RLIGHTI,             //右转灯,输入,高有效(正触发)
  BKHI,                //空调,输入,高有效(正触发),别名[AIR_CONDITION]  
  SWITCHI ,            //紧急报警,输入,  
  HORNI,               //汽车喇叭,输入,默认高有效(正触发)
  VIRBRATIONI,         //一键通话,输入,
  COIL,                //断油路,输出,别名[OIL_CUT]
  HORN_OUT,            //报警喇叭,输出,
  //CON6,16PIN接口
  DETCT1I,             //近光灯,输入,,别名[NEAR_LIGHT]
  DETCT2I,             //机油压力,输入,,别名[GAS_PRESS]
  //GNSS模块
  GPS_OC,              //天线断开,输入,别名[ANT_SHUT]
  GPS_SC,              //天线短路,输入,别名[ANT_SHORT]
  GPS_CTR,             //GPS电源控制脚,输出
  //存储片选线和使能线
  MEM_WP,             //存储写保护使能线,输出
  FLASH_CS,           //Flash片选线,输出
  FR_CS,              //铁电片选线,输出
  EPROM_CS,           //EEPROM片选线,输出
  //通讯模块控制脚
  GSM_ON,             //通讯模块开机脚,输出
  GSM_PWC,            //通讯模块电源控制脚,输出
  GSM_DTR,            //通讯模块DTR脚,输出
  GSM_RI,             //通讯模块响铃脚,输入
  //摄像头控制脚
  CAM_POWER,          //摄像头电源控制脚,输出
  CAM_RD,             //摄像头485读写控制,输出   
  //TTS控制脚
  TTS_RDY,            //TTS语音播报就绪脚,输入
  TTS_P,              //TTS电源控制脚,输出
  //LCD控制脚
  LCD_ON,             //LCD电源控制脚,输出
  LCD_CS,             //LCD片选线,输出
  LCD_AO,             //LCD寄存器选择,输出
  LCD_SPI_SCK,        //LCD SPI时钟信号,输出
  LCD_SPI_MISO,       //LCD SPI信号输入,输入
  LCD_SPI_MOSI,       //LCD SPI信号输出,输出
  //CAN总线
  CAN_CTR,            //CAN电源控制,输出  
  //BMA220震动传感器
  SHOCK_CS,           //震动传感器地址选择  
  //喇叭控制
  V_COL,
  //ic 卡电源控制
  IC_P,
  //其他
  BUZZER ,             //蜂鸣器,输出,
  //麦克风使能控制
  MIC_EN,
  //I2S_AD/DA
  I2S_AD_DA,
  //I2S_PWDAN
  I2S_PWDAN,
    //I2S_PWADN
   I2S_PWADN,
   //SD供电控制
   SD_POWER,
   //POWER TYPE
   PWR_TYPE,
   //SD插入检测
   SD_DETECT,
  
  GPIO_OUT_IN_MAX,
} GPIO_OUT_IN_NUM;   

//输入模式
typedef enum 
{  
  GPIO_IN_MODE_GPIO = 0,   // 普通模式
  GPIO_IN_MODE_EXTI = 1    // 外部中断模式
} GPIO_IN_Mode;

//****************外部变量*******************

//****************全局变量*******************

//****************函数声明*******************
void GpioOutInit(GPIO_OUT_IN_NUM IoNum);
void GpioOutOn(GPIO_OUT_IN_NUM IoNum);
void GpioOutOff(GPIO_OUT_IN_NUM IoNum);
void GpioOutToggle(GPIO_OUT_IN_NUM IoNum);

void GpioInInit(GPIO_OUT_IN_NUM IoNum);
u8 GpioInGetState(GPIO_OUT_IN_NUM IoNum);

#ifdef __cplusplus
}
#endif

#endif /* __GPIOCONTROL_H */
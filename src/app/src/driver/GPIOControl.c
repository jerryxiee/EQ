/******************************************************************** 
//版权说明	:
//文件名称	:GPIOControl.c		
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
//****************包含文件*****************
#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "GPIOControl.h"

//****************宏定义********************

//****************外部变量*******************

//****************全局变量*******************
const uint16_t GPIO_OUT_IN_PIN[GPIO_OUT_IN_MAX] = {
                                                //CON5接口,32PIN接口
                                                DLIGHTI_PIN,
                                                LLIGHTI_PIN,
                                                BREAK_PIN,
                                                ACC_PIN,
                                                DOORI_N_PIN,
                                                DOORI_P_PIN,
                                                DOORI2_PIN,
                                                LAMP_PIN,
                                                LOCK_OUT_PIN,
                                                UNLOCK_OUT_PIN,
                                                XLIGHTI_PIN,
                                                RLIGHTI_PIN,
                                                BKHI_PIN,
                                                SWITCHI_PIN,
                                                HORNI_PIN,
                                                VIRBRATIONI_PIN,
                                                COIL_PIN,
                                                HORN_OUT_PIN,
                                                //CON6,16PIN接口
                                                DETCT1I_PIN,
                                                DETCT2I_PIN,
                                                //GNSS模块
                                                GPS_OC_PIN,
                                                GPS_SC_PIN,
                                                GPS_CTR_PIN,
                                                //存储片选线和使能线
                                                MEM_WP_PIN,
                                                FLASH_CS_PIN,
                                                FR_CS_PIN,
                                                EPROM_CS_PIN,
                                                //通讯模块控制脚
                                                GSM_ON_PIN,
                                                GSM_PWC_PIN,
                                                GSM_DTR_PIN,
                                                GSM_RI_PIN,
                                                //摄像头控制脚
                                                CAM_POWER_PIN,
                                                CAM_RD_PIN,
                                                //TTS控制脚
                                                TTS_RDY_PIN,
                                                TTS_P_PIN,
                                                //LCD控制脚
                                                LCD_ON_PIN,
                                                LCD_CS_PIN,
                                                LCD_AO_PIN,
                                                LCD_SPI_SCK_PIN,
                                                LCD_SPI_MISO_PIN,
                                                LCD_SPI_MOSI_PIN,
                                                //CAN总线
                                                CAN_CTR_MOSI_PIN,
                                                //BMA220震动传感器
                                                SHOCK_CS_MOSI_PIN,
                                                //喇叭控制
                                                V_COL_MOSI_PIN,
                                                //ic 卡电源控制
                                                IC_P_MOSI_PIN,
                                                //蜂鸣器控制
                                                BUZZER_PIN,
						//麦克风使能控制
						MIC_EN_PIN,
						//I2S
						I2S_AD_DA_PIN,
						//I2S
						I2S_PWDAN_PIN,
						//I2S
						I2S_PWADN_PIN,
						//SD_POWER
						SD_POWER_PIN,
                                                   //POWER TYPE
                                                PWR_TYPE_PIN,
						 //SD插入检测
   						SD_DETECT_PIN,
                                              };
GPIO_TypeDef* GPIO_OUT_IN_PORT[GPIO_OUT_IN_MAX] = {
                                                //CON5接口,32PIN接口
                                                DLIGHTI_GPIO_PORT,
                                                LLIGHTI_GPIO_PORT,
                                                BREAK_GPIO_PORT,
                                                ACC_GPIO_PORT,
                                                DOORI_N_GPIO_PORT,
                                                DOORI_P_GPIO_PORT,
                                                DOORI2_GPIO_PORT,
                                                LAMP_GPIO_PORT,
                                                LOCK_OUT_GPIO_PORT,
                                                UNLOCK_OUT_GPIO_PORT,
                                                XLIGHTI_GPIO_PORT,
                                                RLIGHTI_GPIO_PORT,
                                                BKHI_GPIO_PORT,
                                                SWITCHI_GPIO_PORT, 
                                                HORNI_GPIO_PORT,
                                                VIRBRATIONI_GPIO_PORT,
                                                COIL_GPIO_PORT,
                                                HORN_OUT_GPIO_PORT,
                                                //CON6,16PIN接口
                                                DETCT1I_GPIO_PORT,
                                                DETCT2I_GPIO_PORT,
                                                //GNSS模块
                                                GPS_OC_GPIO_PORT,
                                                GPS_SC_GPIO_PORT,
                                                GPS_CTR_GPIO_PORT,
                                                //存储片选线和使能线
                                                MEM_WP_GPIO_PORT,
                                                FLASH_CS_GPIO_PORT,
                                                FR_CS_GPIO_PORT,
                                                EPROM_CS_GPIO_PORT,
                                                //通讯模块控制脚
                                                GSM_ON_GPIO_PORT,
                                                GSM_PWC_GPIO_PORT,
                                                GSM_DTR_GPIO_PORT,
                                                GSM_RI_GPIO_PORT,
                                                //摄像头控制脚
                                                CAM_POWER_GPIO_PORT,
                                                CAM_RD_GPIO_PORT,
                                                //TTS控制脚
                                                TTS_RDY_GPIO_PORT,
                                                TTS_P_GPIO_PORT,
                                                //LCD控制脚
                                                LCD_ON_GPIO_PORT,
                                                LCD_CS_GPIO_PORT,
                                                LCD_AO_GPIO_PORT,
                                                LCD_SPI_SCK_GPIO_PORT,
                                                LCD_SPI_MISO_GPIO_PORT,
                                                LCD_SPI_MOSI_GPIO_PORT,
                                                //CAN总线
                                                CAN_CTR_MOSI_GPIO_PORT,
                                                //BMA220震动传感器
                                                SHOCK_CS_MOSI_GPIO_PORT,
                                                //喇叭控制
                                                V_COL_MOSI_GPIO_PORT,
                                                //ic 卡电源控制
                                                IC_P_MOSI_GPIO_PORT,
                                                //蜂鸣器控制
                                                BUZZER_GPIO_PORT,  
						//麦克风使能控制
						MIC_EN_GPIO_PORT,
						//I2S
						I2S_AD_DA_GPIO_PORT,
						//I2S
						I2S_PWDAN_GPIO_PORT,
						//I2S
						I2S_PWADN_GPIO_PORT,
						//SD_POWER
						SD_POWER_GPIO_PORT,
                                                   //POWER TYPE
                                                PWR_TYPE_GPIO_PORT,
						//SD插入检测
   						SD_DETECT_GPIO_PORT,
                                              };
const uint32_t GPIO_OUT_IN_CLK[GPIO_OUT_IN_MAX] = {  
                                                //CON5接口,32PIN接口
                                                DLIGHTI_GPIO_CLK,
                                                LLIGHTI_GPIO_CLK,  
                                                BREAK_GPIO_CLK,
                                                ACC_GPIO_CLK,  
                                                DOORI_N_GPIO_CLK,
                                                DOORI_P_GPIO_CLK,
                                                DOORI2_GPIO_CLK,
                                                LAMP_GPIO_CLK,
                                                LOCK_OUT_GPIO_CLK,
                                                UNLOCK_OUT_GPIO_CLK,
                                                XLIGHTI_GPIO_CLK,
                                                RLIGHTI_GPIO_CLK,
                                                BKHI_GPIO_CLK,
                                                SWITCHI_GPIO_CLK,
                                                HORNI_GPIO_CLK,
                                                VIRBRATIONI_GPIO_CLK,
                                                COIL_GPIO_CLK,
                                                HORN_OUT_GPIO_CLK,
                                                //CON6,16PIN接口
                                                DETCT1I_GPIO_CLK,
                                                DETCT2I_GPIO_CLK,
                                                //GNSS模块
                                                GPS_OC_GPIO_CLK,
                                                GPS_SC_GPIO_CLK,
                                                GPS_CTR_GPIO_CLK,
                                                //存储片选线和使能线
                                                MEM_WP_GPIO_CLK,
                                                FLASH_CS_GPIO_CLK,
                                                FR_CS_GPIO_CLK,
                                                EPROM_CS_GPIO_CLK,
                                                //通讯模块控制脚
                                                GSM_ON_GPIO_CLK,
                                                GSM_PWC_GPIO_CLK,
                                                GSM_DTR_GPIO_CLK,
                                                GSM_RI_GPIO_CLK,
                                                //摄像头控制脚
                                                CAM_POWER_GPIO_CLK,
                                                CAM_RD_GPIO_CLK,
                                                //TTS控制脚
                                                TTS_RDY_GPIO_CLK,
                                                TTS_P_GPIO_CLK,
                                                //LCD控制脚
                                                LCD_ON_GPIO_CLK,
                                                LCD_CS_GPIO_CLK,
                                                LCD_AO_GPIO_CLK,
                                                LCD_SPI_SCK_GPIO_CLK,
                                                LCD_SPI_MISO_GPIO_CLK,
                                                LCD_SPI_MOSI_GPIO_CLK,
                                                //CAN总线
                                                CAN_CTR_MOSI_GPIO_CLK,
                                                //BMA220震动传感器
                                                SHOCK_CS_MOSI_GPIO_CLK,
                                                //喇叭控制
                                                V_COL_MOSI_GPIO_CLK,
                                                //ic 卡电源控制
                                                IC_P_MOSI_GPIO_CLK,
                                                //蜂鸣器控制
                                                BUZZER_GPIO_CLK,  
						//麦克风使能控制
						MIC_EN_GPIO_CLK,
						//I2S
						I2S_AD_DA_GPIO_CLK,
						//I2S
						I2S_PWDAN_GPIO_CLK,
						//I2S
						I2S_PWADN_GPIO_CLK,
						//SD_POWER
						SD_POWER_GPIO_CLK,
                                                 //POWER TYPE
                                                PWR_TYPE_GPIO_CLK,
						//SD插入检测
   						SD_DETECT_GPIO_CLK,
                                              };


//****************函数声明*******************

/****************函数定义******************
//函数名称	:GpioOutInit
//功能		:初始化GPIO输出引脚
//输入		:IoNum 输入值,参考GPIO_OUT_IN_NUM枚举
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void GpioOutInit(GPIO_OUT_IN_NUM IoNum)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  
  /* 实用GPIO时钟*/
  RCC_AHB1PeriphClockCmd(GPIO_OUT_IN_CLK[IoNum], ENABLE);

  /* 配置GPIO脚 50Mhz推挽输出,弱上拉 */
  GPIO_InitStructure.GPIO_Pin = GPIO_OUT_IN_PIN[IoNum];
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;  //LED灯控制不能 设置上拉 GPIO_PuPd_UP
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIO_OUT_IN_PORT[IoNum], &GPIO_InitStructure);
}

/****************函数定义******************
//函数名称	:GpioOutOn
//功能		:设置GPIO输出引脚为高(即数字1)
//输入		:IoNum 输入值,参考GPIO_OUT_IN_NUM枚举
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void GpioOutOn(GPIO_OUT_IN_NUM IoNum)
{
  GPIO_OUT_IN_PORT[IoNum]->BSRRL = GPIO_OUT_IN_PIN[IoNum];
}

/****************函数定义******************
//函数名称	:GpioOutOff
//功能		:设置GPIO输出引脚为低(即数字0)
//输入		:IoNum 输入值,参考GPIO_OUT_IN_NUM枚举
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void GpioOutOff(GPIO_OUT_IN_NUM IoNum)
{
  GPIO_OUT_IN_PORT[IoNum]->BSRRH = GPIO_OUT_IN_PIN[IoNum];  
}

/****************函数定义******************
//函数名称	:GpioOutToggle
//功能		:取反当前GPIO输出引脚状态
//输入		:IoNum 输入值,参考GPIO_OUT_IN_NUM枚举
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void GpioOutToggle(GPIO_OUT_IN_NUM IoNum)
{
  GPIO_OUT_IN_PORT[IoNum]->ODR ^= GPIO_OUT_IN_PIN[IoNum];
}


/****************函数定义******************
//函数名称	:GpioInInit
//功能		:初始化输入引脚
//输入		:IoNum 输入值,参考GPIO_OUT_IN_NUM枚举
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void GpioInInit(GPIO_OUT_IN_NUM IoNum)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* 使能GPIO时钟 */
  RCC_AHB1PeriphClockCmd(GPIO_OUT_IN_CLK[IoNum], ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

  /* 配置按钮为输入模式*/
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Pin = GPIO_OUT_IN_PIN[IoNum];
  GPIO_Init(GPIO_OUT_IN_PORT[IoNum], &GPIO_InitStructure);
  
}

/****************函数定义******************
//函数名称	:GpioInGetState
//功能		:获取输入引脚状态
//输入		:IoNum 输入值,参考GPIO_OUT_IN_NUM枚举
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:当前IO口状态值,0或1
//备注		:
*********************************************************************/
u8 GpioInGetState(GPIO_OUT_IN_NUM IoNum)
{
  return GPIO_ReadInputDataBit(GPIO_OUT_IN_PORT[IoNum], GPIO_OUT_IN_PIN[IoNum]);
}
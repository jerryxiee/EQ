#ifndef __SPI_H
#define __SPI_H
//#include "sys.h"

//改变延时值可以改变操作频率
#define SD_CS_LOW()       GPIO_ResetBits(GPIOD, GPIO_Pin_14);spi_Delay_uS(20)//建议延时值不小于20
#define SD_CS_HIGH()      GPIO_SetBits(GPIOD, GPIO_Pin_14);spi_Delay_uS(20) 
#define SD_SPIx_SCK_RESET() GPIO_ResetBits(GPIOB, GPIO_Pin_10);spi_Delay_uS(15)//建议延时值不小于10  
#define SD_SPIx_SCK_SET()   GPIO_SetBits(GPIOB, GPIO_Pin_10);spi_Delay_uS(15) 
#define SD_SPIx_MOSI_RESET()    GPIO_ResetBits(GPIOC, GPIO_Pin_3);spi_Delay_uS(10)//建议延时值不小于10
#define SD_SPIx_MOSI_SET()      GPIO_SetBits(GPIOC, GPIO_Pin_3);spi_Delay_uS(10)  
/*
//建议在IO_Init中初始化SD_Power端口，提前打开SD卡电源，以缩短初始化时间。
#define SD_SD_Power_RESET() GPIO_ResetBits(GPIOG, GPIO_Pin_3);\
                            spi_Delay_uS(65000);\
                            spi_Delay_uS(65000);\
                            spi_Delay_uS(65000);\
                            spi_Delay_uS(65000);\
                            spi_Delay_uS(65000);\
                            spi_Delay_uS(65000);\
                            spi_Delay_uS(65000);\
                            spi_Delay_uS(65000)
#define SD_SD_Power_SET()   GPIO_SetBits(GPIOG, GPIO_Pin_3);spi_Delay_uS(5000); 
*/				  	    													  
void SPIx_Init(void);			 //初始化SPI口
void SPIx_SetSpeed(u8 SpeedSet); //设置SPI速度   
u8 SPIx_ReadWriteByte(u8 TxData);//SPI总线读写一个字节
		 
#endif


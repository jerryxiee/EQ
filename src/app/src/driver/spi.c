#include "include.h" 
#include "spi.h"

#define SPIx_IO_Simulate_DEBUG 0

SPI_InitTypeDef  SPI_InitStructure;

//以下是SPI模块的初始化代码，配置成主机模式，访问SD Card/W25X16/24L01/JF24C							  
//SPI口初始化
//这里针是对SPI3的初始化

#if SPIx_IO_Simulate_DEBUG

void SPIx_Init(void) 
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  //!< Enable GPIO clocks 
  //RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOC|RCC_AHB1Periph_GPIOD|RCC_AHB1Periph_GPIOG, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOC|RCC_AHB1Periph_GPIOD, ENABLE);
  /*
  //!< Configure sd SD_Power pin in output pushpull mode 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_Init(GPIOG, &GPIO_InitStructure);
  */
  //!< Configure sd Card CS pin in output pushpull mode 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
  
  //!< Configure SCK pin in output pushpull mode 
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  //GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;//GPIO_PuPd_NOPULL;dxl,2014.2.28
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOB, &GPIO_InitStructure);  
  
  //!< Configure MOSI pin in output pushpull mode 
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  //GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;//GPIO_PuPd_NOPULL;dxl,2014.2.28
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOC, &GPIO_InitStructure);  
  
  //!< Configure MISO pin in Input pushpull mode 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  //GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;dxl,2014.2.28
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  
  //!< Configure SD_CD pin in Input pushpull mode 
  //检测SD卡插入状态,0：卡插入，1：无卡。本程序中没有使用此功能。
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  //GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;dxl,2014.2.28
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
  
  //SD_SD_Power_RESET();//SD卡供电
  
  //!< Deselect the FLASH: Chip Select high 
  SD_CS_HIGH();
  
  SD_SPIx_SCK_SET();//sck=1

  //!< Enable the sFLASH_SPI 
  SPIx_ReadWriteByte(0xff);    //启动传输	

}

void SPIx_SetSpeed(u8 SpeedSet)
{

} 

//IO模拟SPI总线读写操作，模式3（CPOL = 1，CPHA = 1）
//SPIx 读写一个字节
//TxData:要写入的字节
//返回值:读取到的字节
u8 SPIx_ReadWriteByte(u8 TxData)
{
    u8 i,RxData = 0,TxDataTemp;
    
    TxDataTemp = TxData;   
    SD_SPIx_SCK_SET();  //sck = 1，空闲为高
    
    for (i = 0; i < 8; i++)
    {  
        SD_SPIx_SCK_RESET();    //第一个边沿跳变，MOSI数据不会被采样
        if(TxDataTemp & 0x80)
        {
            SD_SPIx_MOSI_SET(); //MOSI = 1
        }
        else
        {
            SD_SPIx_MOSI_RESET(); //MOSI = 0
        }
        
        TxDataTemp <<= 1; 
        SD_SPIx_SCK_SET();  //上升沿写入，MOSI数据被采样，同时MISO在上升沿已经输出数据

        if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_2))    //MISO数据采样
        {
            RxData |= 0x01; 
        }
        if(i < 7)
        {
            RxData <<= 1;
        }       
    }
    return RxData;
}

#else

void SPIx_Init(void) 
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /*!< Enable the SPI clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);

  /*!< Enable GPIO clocks */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOB, ENABLE);
  
  /*!< SPI pins configuration *************************************************/

  /*!< Connect SPI pins to AF6 */  
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_SPI3);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_SPI3);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI3);
  //GPIO_PinAFConfig(GPIOA, GPIO_PinSource15, GPIO_AF_SPI3);
  
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL; //GPIO_PuPd_NOPULL; GPIO_PuPd_DOWN;
        
  /*!< SPI SCK pin configuration */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  /*!< SPI MOSI pin configuration */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  /*!< SPI MISO pin configuration */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  /*!< Configure sd Card CS pin in output pushpull mode ********************/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
  
  GPIO_SetBits(GPIOB, GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5);
  
  //!< Configure SD_CD pin in Input pushpull mode 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  //GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;//dxl,2014.2.28
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
  
  /*!< Deselect the FLASH: Chip Select high */
  SD_CS_HIGH();
  /*!< SPI configuration */
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;

  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(SPI3, &SPI_InitStructure);

  /*!< Enable the sFLASH_SPI  */
  SPI_Cmd(SPI3, ENABLE);
  SPIx_ReadWriteByte(0xff);    //启动传输
}

//SPI 速度设置函数
//SpeedSet:
//SPI_BaudRatePrescaler_2   2分频   (SPI 36M@sys 72M)
//SPI_BaudRatePrescaler_4   4分频   (SPI 9M@sys 72M)
//SPI_BaudRatePrescaler_8   8分频   (SPI 9M@sys 72M)
//SPI_BaudRatePrescaler_16  16分频  (SPI 4.5M@sys 72M)
//SPI_BaudRatePrescaler_32  32分频   (SPI 9M@sys 72M)
//SPI_BaudRatePrescaler_64  64分频   (SPI 9M@sys 72M)
//SPI_BaudRatePrescaler_128 128分频   (SPI 9M@sys 72M)
//SPI_BaudRatePrescaler_256 256分频 (SPI 281.25K@sys 72M)
void SPIx_SetSpeed(u8 SpeedSet)
{
	SPI_InitStructure.SPI_BaudRatePrescaler = SpeedSet ;
  	SPI_Init(SPI3, &SPI_InitStructure);
	SPI_Cmd(SPI3,ENABLE);
} 

//SPIx 读写一个字节
//TxData:要写入的字节
//返回值:读取到的字节
u8 SPIx_ReadWriteByte(u8 TxData)
{
    u8 retry=0;
    /*!< Loop while DR register in not emplty */
    while (SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_TXE) == RESET)  //检查指定的SPI标志位设置与否:发送缓存空标志位
    {//发送缓存非空
        retry++;    
    	if(retry>200)return 0;
    }
    //发送缓存空
    /*!< Send byte through the SPI3 peripheral */
    SPI_I2S_SendData(SPI3,TxData);  //通过外设SPIx发送一个数据
    retry=0;
    /*!< Wait to receive a byte */
    while (SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_RXNE) == RESET) //检查指定的SPI标志位设置与否:接受缓存非空标志位
	{//接收缓存空
		retry++;
		if(retry>200)return 0;
	}//接收缓存非空
    /*!< Return the byte read from the SPI bus */
    return SPI_I2S_ReceiveData(SPI3); //返回通过SPIx最近接收的数据	
}

#endif














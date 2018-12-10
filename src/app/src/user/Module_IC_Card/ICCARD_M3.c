/*************************************************************************
* Copyright (c) 2014 All rights reserved., 
* 文件名称 : ICCARD_M3.c
* 当前版本 : 0.1
* 开发者   : MARK
* 编辑时间 : 2014年03月17日
* 更新时间 ：
*************************************************************************/



/****************************文件预处理**********************************/

 #include  "include.h" //包含IC卡读卡器工作参数设置的头文件

 #define NOP() asm("nop")
/***********************************************************************/


/***************************初始化处理**********************************/ 

/***************************管脚配置************************************/ 
/*************************************************************
** 函数名称: ICCARD_M3_IO_SDA_OUT
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void ICCARD_M3_SDA_OUT(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
  
/******************************IO***********************************/    
    RCC_AHB1PeriphClockCmd(  RCC_ICCARD_IO , ENABLE);	       /* 打开GPIO时钟 */

    GPIO_InitStructure.GPIO_Pin = PIN_ICCARD_IO  ;
    
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;        /* 快速模式 */ 
    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;  	         /* 输出/输入 */
    
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;           /* 开漏 */
    
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;             /* 上拉 */
    
    GPIO_Init( GPIO_ICCARD_IO , &GPIO_InitStructure);
}
/*************************************************************
** 函数名称: ICCARD_M3_IO_SDA_IN
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void ICCARD_M3_SDA_IN(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
  
/******************************IO***********************************/    
    RCC_AHB1PeriphClockCmd(  RCC_ICCARD_IO , ENABLE);	       /* 打开GPIO时钟 */

    GPIO_InitStructure.GPIO_Pin = PIN_ICCARD_IO  ;
    
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;        /* 快速模式 */ 
    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;  	         /* 输出/输入 */
    
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;           /* 开漏 */
    
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;             /* 上拉 */
    
    GPIO_Init( GPIO_ICCARD_IO , &GPIO_InitStructure);
}
/*************************************************************
** 函数名称: ICCARD_M3_IO_SDA_IN
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void ICCARD_M3_CLK_OUT(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    RCC_AHB1PeriphClockCmd(  RCC_ICCARD_CLK  , ENABLE);	    /* 打开GPIO时钟 */
  
    GPIO_InitStructure.GPIO_Pin = PIN_ICCARD_CLK ;
    
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz ;
    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT ;      	  /* 输出 */
    
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;          /* 开漏 */
    
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;            /* 上拉 */
    
    GPIO_Init( GPIO_ICCARD_CLK , &GPIO_InitStructure );
}
/*************************************************************
** 函数名称: ICCARD_M3_IO_SDA_IN
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void ICCARD_M3_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
  
/******************************IO***********************************/    
    ICCARD_M3_SDA_OUT();
  
/******************************CLK***********************************/    
    ICCARD_M3_CLK_OUT();
    
/******************************RST***********************************/     
    RCC_AHB1PeriphClockCmd(  RCC_ICCARD_RST , ENABLE);	    /* 打开GPIO时钟 */

    GPIO_InitStructure.GPIO_Pin = PIN_ICCARD_RST  ;
    
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;  	        /* 输出 */
    
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;          /* 推挽 */
    
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;        /* 无上拉 */
    
    GPIO_Init( GPIO_ICCARD_RST , &GPIO_InitStructure);
    
/******************************DETECT***********************************/    
    RCC_AHB1PeriphClockCmd(  RCC_ICCARD_DETECT  , ENABLE);	/* 打开GPIO时钟 */
  
    GPIO_InitStructure.GPIO_Pin = PIN_ICCARD_DETECT ;
    
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz ;
    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN ;       /* 输入 */
    
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;          /* 推挽 */
    
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;           /* 无上拉 */
    
    GPIO_Init( GPIO_ICCARD_DETECT , &GPIO_InitStructure );
    
/******************************POWER***********************************/      
    RCC_AHB1PeriphClockCmd(  RCC_ICCARD_POWER , ENABLE);	/* 打开GPIO时钟 */

    GPIO_InitStructure.GPIO_Pin = PIN_ICCARD_POWER  ;
    
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;       	/* 输出 */
    
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;          /* 推挽 */
    
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;         /* 无上拉 */
    
    GPIO_Init( GPIO_ICCARD_POWER , &GPIO_InitStructure);

/******************************SHOCK_CS***********************************/      
    RCC_AHB1PeriphClockCmd(  RCC_BMA250_SHOCK_CS , ENABLE);	/* 打开GPIO时钟 */

    GPIO_InitStructure.GPIO_Pin = PIN_BMA250_SHOCK_CS ;
    
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;       	/* 输出 */
    
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;          /* 推挽 */
    
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;         /* 无上拉 */
    
    GPIO_Init( GPIO_BMA250_SHOCK_CS , &GPIO_InitStructure);
    
    
    ICCARD_SDA_RESET();                          /* 数据端口置0 */

    ICCARD_CLK_RESET();                         /* 时钟端口置0 */

    ICCARD_RST_RESET();                         /* 复位端口置0 */

    ICCARD_DETECT_RESET();                      /* 检测端口置1 */

    ICCARD_POWER_ON();                       /* 打开电源 */    

    BMA250_SHOCK_CS_RESET();                    /* BMA250地址线 */

    ICCARD_M3_I2C_Stop();			              /* 发送停止信号 */

}

/*********************************延时配置****************************/
void ICCARD_M3_I2C_Delay(void)
{
    NOP();
    NOP();
    NOP();
    NOP();
    NOP();
}

/*********************************类I2C时序定义************************/ 

/**********************************I2C-->START*************************/
void ICCARD_M3_I2C_Start(void)
{
    ICCARD_M3_SDA_OUT();

    ICCARD_SDA_SET();    

    ICCARD_CLK_SET();

    ICCARD_M3_I2C_Delay();

    ICCARD_SDA_RESET();

    ICCARD_M3_I2C_Delay();

    ICCARD_CLK_RESET();

    ICCARD_M3_I2C_Delay();
}

/**********************************I2C-->STOP*************************/
void ICCARD_M3_I2C_Stop(void)
{
    ICCARD_M3_SDA_OUT();

    ICCARD_SDA_RESET();

    ICCARD_CLK_SET();

    ICCARD_M3_I2C_Delay();

    ICCARD_SDA_SET();
    /////////////////////
    ICCARD_M3_I2C_Delay();
}

/**********************************I2C-->SENDBYTE*************************/
void ICCARD_M3_I2C_SendByte(u8 Byte )
{

    u8 i;

    ICCARD_M3_SDA_OUT();
    

    for (i = 0; i < 8; i++)   /* 先发送字节的高位bit7 */
    {		

        if ( Byte & 0x80)
        {
            ICCARD_SDA_SET();
        }
        else
        {
            ICCARD_SDA_RESET();
        }

        ICCARD_M3_I2C_Delay();

        ICCARD_CLK_SET();

        ICCARD_M3_I2C_Delay();

        ICCARD_CLK_RESET();

        //if (i == 7)
        //{
        //    ICCARD_SDA_SET();        /* 释放总线 */
        //}

        Byte<<= 1;	             /* 左移一个bit */

        ICCARD_M3_I2C_Delay();
    }
    ////////////////////
    ICCARD_M3_I2C_Delay();
    ICCARD_M3_I2C_Delay();
    ICCARD_M3_I2C_Delay();
    ICCARD_M3_I2C_Delay();
}

/**********************************I2C-->READBYTE*************************/
u8 ICCARD_M3_I2C_ReadByte(void)
{
    u8 i;

    u8 value;

    ICCARD_M3_SDA_IN();

    ICCARD_CLK_RESET();
    /////////////////
    ICCARD_M3_I2C_Delay();

    value = 0;

    for (i = 0; i < 8; i++)    /* 读到第1个bit为数据的bit7 */
    {
        ICCARD_CLK_SET();

        ICCARD_M3_I2C_Delay();

        value <<= 1;

        if (ICCARD_SDA_READ())
        {
            value |= 0x01;
        }
        else
        {
            value &= 0xFE;
        }

        ICCARD_CLK_RESET();

        ICCARD_M3_I2C_Delay();
    }

    ICCARD_M3_SDA_OUT();

    ICCARD_M3_I2C_Delay();

    return value;

}

/**********************************I2C-->WAITACK*************************/
u8 ICCARD_M3_I2C_CheckAck(void)
{

    u8 Ack;

    ICCARD_M3_SDA_IN();

    ICCARD_SDA_SET();            	/* CPU释放SDA总线 */

    ICCARD_M3_I2C_Delay();

    ICCARD_CLK_SET();            	/* CPU驱动SCL = 1, 此时器件会返回ACK应答 */

    ICCARD_M3_I2C_Delay();

    if (ICCARD_SDA_READ())	        /* CPU读取SDA口线状态 */
    {
        Ack = 1;
    }
    else
    {
        Ack = 0;
    } 

    ICCARD_CLK_RESET();

    ICCARD_M3_I2C_Delay();

    return Ack;

}

/**********************************I2C-->ACK*************************/ 
void ICCARD_M3_I2C_SendAck(void)
{
    ICCARD_M3_SDA_OUT();
    ////////////
    ICCARD_SDA_RESET();	            /* CPU驱动SDA = 0 */

    ICCARD_M3_I2C_Delay();

    ICCARD_CLK_RESET();

    ICCARD_M3_I2C_Delay();

    ICCARD_CLK_SET();	            /* CPU产生1个时钟 */

    ICCARD_M3_I2C_Delay();

    ICCARD_CLK_RESET();

    ICCARD_M3_I2C_Delay();

    ICCARD_SDA_SET();	            /* CPU释放SDA总线 */

    ICCARD_M3_I2C_Delay();
}

/**********************************I2C-->NACK*************************/ 
void ICCARD_M3_I2C_SendNAck(void)
{
    ICCARD_M3_SDA_OUT();

    ICCARD_SDA_SET();	           /* CPU驱动SDA = 1 */

    ICCARD_M3_I2C_Delay();

    ICCARD_CLK_RESET();

    ICCARD_M3_I2C_Delay();

    ICCARD_CLK_SET();	           /* CPU产生1个时钟 */

    ICCARD_M3_I2C_Delay();

    ICCARD_CLK_RESET();  

    ICCARD_M3_I2C_Delay();
}

/*************************I2C-->CHECKDEVICE************************/
u8 ICCARD_M3_I2C_CheckDevice(u8 Address)
{  
    u8 Ack;

    ICCARD_M3_Init();		                      /* 配置GPIO */

    ICCARD_M3_I2C_Stop();			              /* 发送停止信号 */

    ICCARD_M3_I2C_Start();		              /* 发送启动信号 */

    ICCARD_M3_I2C_SendByte( Address | I2C_WR);

    Ack = ICCARD_M3_I2C_CheckAck();           	  /* 检测设备的ACK应答 */

    ICCARD_M3_I2C_Stop();			              /* 发送停止信号 */

    return Ack;
}
/**************************SPI--->POWER_ON***********************/
void M3_I2C_Delays(unsigned char n)
{
    for(u8 i=0;i<n;i++)
    ICCARD_M3_I2C_Delay();  
}
/**************************SPI--->POWER_ON***********************/
u8 Power_On(void)
{

    ICCARD_SDA_SET();

    ICCARD_SDA_SET();

    ICCARD_RST_SET();

    M3_I2C_Delays(50);

    ICCARD_RST_SET();


    M3_I2C_Delays(10);

    ICCARD_CLK_SET();

    M3_I2C_Delays(20);

    ICCARD_CLK_RESET();

    M3_I2C_Delays(10);

    ICCARD_RST_RESET();

    M3_I2C_Delays(20);              //发出复位时序

    InByte();
    InByte();
    InByte();
    InByte();

    InByte();
    InByte();

    ICCARD_CLK_SET();

    M3_I2C_Delays(20); 

    ICCARD_CLK_RESET();

    M3_I2C_Delays(20);

    ICCARD_CLK_SET();

    M3_I2C_Delays(20);

    ICCARD_CLK_RESET();
    
    M3_I2C_Delays(20);

    return 1;

}

/*******************************SPI--->INBYTE******************/
u8 InByte(void)
{
    u8 i,a;

    a=0;

    ICCARD_SDA_SET();//置为输入状态

    ICCARD_SDA_SET();//置为输入状态

    for (i=8;i>0;i--)
    {

        a=a>>1;

        if(ICCARD_SDA_READ())
        {
            a|=0x80;
        }

        ICCARD_CLK_SET();

        M3_I2C_Delays(20);

        ICCARD_CLK_RESET();

        M3_I2C_Delays(20);

    }

    return(a);

}

/****************************SPI--->OUTBIT**********************/
void OutByte(u8 ch)
{
    u8 i;

    for (i=8;i>0;i--)
    {

        if(ch&0x01)
        {
            ICCARD_SDA_SET();
        }   
        else
        {
            ICCARD_SDA_RESET();
        }

        ICCARD_CLK_SET();

        M3_I2C_Delays(20);

        ICCARD_CLK_RESET();

        M3_I2C_Delays(20);
        
        ch=ch>>1;                 //右移一位

    }

}

/****************************SPI--->STARTCOMM*******************/
void Start_Comm(void)
{

    ICCARD_SDA_SET();

  	ICCARD_SDA_SET();

    M3_I2C_Delays(20);
    
    ICCARD_CLK_SET();

    M3_I2C_Delays(20);

    ICCARD_SDA_RESET();

  	ICCARD_SDA_RESET();

    M3_I2C_Delays(20);

    ICCARD_CLK_RESET();

    M3_I2C_Delays(20);
    
}

/****************************SPI--->STOPCOMM********************/
void Stop_Comm(void)
{

    ICCARD_SDA_RESET();
    
    M3_I2C_Delays(40);

    ICCARD_CLK_SET();

    M3_I2C_Delays(10);
    
    ICCARD_SDA_SET();

    M3_I2C_Delays(10);

}

/****************************SPI--->SENDCOMM********************/
void SendComm(u8 a,u8 b,u8 c)
{
    Start_Comm();     //开始发送命令
    OutByte(a);          //发命令字
    OutByte(b);          //发地址
    OutByte(c);          //发数据
    Stop_Comm();      //结束发送命令
}

/***************************SPI--->PRO_MOD*********************/
void Proce_Mod(void)
{

    u8 i;

    ICCARD_CLK_RESET();

    M3_I2C_Delays(10);

    ICCARD_SDA_SET();

    for (i=250;i>0;i--)
    {

        ICCARD_CLK_SET();

        M3_I2C_Delays(20);

        ICCARD_CLK_RESET();

        M3_I2C_Delays(20);

        if(ICCARD_SDA_READ()) //输入为高就跳出

        break;

    }
    //如果超过10ms  INDATA 未恢复高，如何处理？
}

/*************************SPI--->READ_MOD************************/
void Read_Mod(u8  *pt,u8 i)
{
    u8  temp;

    ICCARD_CLK_RESET();

    M3_I2C_Delays(20);

    ICCARD_CLK_SET();

    do
    {
        temp=InByte();    //读入一个字节

        *pt=temp;    //读入一个字节

        pt++;            //指针加一

    }while(--i);          //计数器减一,判断

}

/*************************SPI--->RSTCARD************************/
void RstCard(void)
{

    ICCARD_RST_SET();

    M3_I2C_Delays(10);

    ICCARD_CLK_SET();

    M3_I2C_Delays(20);

    ICCARD_CLK_RESET();

    M3_I2C_Delays(10);

    ICCARD_RST_RESET();

    M3_I2C_Delays(10);    //发出复位时序

    InByte();

    InByte();

    InByte();

    InByte();  //空读4次

}

/***********************SPI--->Rmm******************************/
void Rmm(u8   cardAdd, u8    *pt,u8    i)
{
    SendComm(RMM_COMM,cardAdd, 0 );

    Read_Mod(pt,i);

    RstCard();

}
/******************************************************************************
**                            End Of File
******************************************************************************/


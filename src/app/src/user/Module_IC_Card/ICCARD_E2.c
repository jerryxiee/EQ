/*************************************************************************
* Copyright (c) 2014 All rights reserved., 
* 文件名称 : ICCARD_E2.c
* 当前版本 : 0.1
* 开发者   : MARK
* 编辑时间 : 2014年03月18日
* 更新时间 ：
*************************************************************************/



/****************************文件预处理**********************************/

 #include  "include.h"                /*包含IC卡读卡器工作参数设置的头文件*/
 
/***********************************************************************/
//#define EEPROM_24XX_USE_ALL_PAGE 
/***********************************************************************/
typedef struct tagSTT_AT24C128_CTRL{

    #ifdef EEPROM_24XX_USE_ALL_PAGE    
    u8 buf[ EE_24XX_HL_SIZE ];
    #else
    u8 buf[ 128 ];
    #endif

    u8 IsValid;

    u16 BTID_Addr;

    u8  BTID_len;

    u8  BTID_Start[ 6 ];

    u8  BTID_End[ 6 ] ;

    u16 BTID_STEPS_Total;

    u16 BTID_STEPS_Finished;

    u16 BTID_STEPS_unFinished;

    u16 BTID_STEPS_Successed;

}STT_AT24C128_CTRL;
 
STT_AT24C128_CTRL   g_sttAT24C128Ctrl;

/********************************************************************************************************
*	函 数 名: E2_Check_I2C_Ack
*	功能说明:
*	返 回 值: 1 表示失败，0 表示成功
********************************************************************************************************/
u8 E2_Check_I2C_Ack(void)
{
    if(ICCARD_M3_I2C_CheckAck())
	{
        ICCARD_M3_I2C_Stop();
		return 1;	/* EEPROM器件无应答 */
	}
    return 0;
}

/********************************************************************************************************
*	函 数 名: E2_ReadOneByte
*	功能说明: 从串行EEPROM指定地址处开始读取一个数据
*	形    参：_pReadBuf : 存放读到的数据的缓冲区指针
*			 _ucDevAddr : 设备地址
*			 _usAddress : 起始地址
*			 _pReadBuf : 存放读到的数据的缓冲区指针
*	返 回 值: 0 表示失败，1表示成功
********************************************************************************************************/
u8 E2_ReadOneByte(u8 *data, u8 _ucDevAddr, u8 _usAddress)
{	
	/* 第1步：发起I2C总线启动信号 */
	ICCARD_M3_I2C_Start();
	
	/* 第2步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
	ICCARD_M3_I2C_SendByte( _ucDevAddr | I2C_WR);	/* 此处是写指令 */
	
	/* 第3步：接收ACK */
    
	if (E2_Check_I2C_Ack())
	{
		return 0;	/* EEPROM器件无应答 */
	}

	/* 第4步：发送字节地址，24C02只有256字节，因此1个字节就够了，如果是24C04以上，那么此处需要连发多个地址 */
	ICCARD_M3_I2C_SendByte((u8)( _usAddress )&0x0FF  );
	
	/* 第5步：发送ACK */
	if (E2_Check_I2C_Ack())
	{
		return 0;	/* EEPROM器件无应答 */
	}
  	
	/* 第6步：重新启动I2C总线。前面的代码的目的向EEPROM传送地址，下面开始读取数据 */
	ICCARD_M3_I2C_Start();
	
	/* 第7步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
	ICCARD_M3_I2C_SendByte( _ucDevAddr | I2C_RD);	/* 此处是读指令 */
	
	/* 第8步：发送ACK */
	if (E2_Check_I2C_Ack())
	{
		return 0;	/* EEPROM器件无应答 */
	}	
	
	/* 第9步：循环读取数据 */    
	*data = ICCARD_M3_I2C_ReadByte() ;	/* 读1个字节 */
    
    ICCARD_M3_I2C_SendNAck();	/* 最后1个字节读完后，CPU产生NACK信号(驱动SDA = 1) */
	/* 发送I2C总线停止信号 */
	ICCARD_M3_I2C_Stop();
    ////////////
    return 1;	/* 执行成功 */
}
/********************************************************************************************************
*	函 数 名: E2_ReadBytes
*	功能说明: 从串行EEPROM指定地址处开始读取若干数据
*	形    参：_pReadBuf : 存放读到的数据的缓冲区指针
*			 _ucDevAddr : 设备地址
*			 _usAddress : 起始地址
*			 _usSize : 数据长度，单位为字节
*			 _pReadBuf : 存放读到的数据的缓冲区指针
*	返 回 值: 0 表示失败，1表示成功
********************************************************************************************************/
u8 E2_ReadBytes(u8 *_pReadBuf, u8 _ucDevAddr, u8 _usAddress, u16 _usSize)
{
	u16 i;
	
	/* 采用串行EEPROM随即读取指令序列，连续读取若干字节 */
	
	/* 第1步：发起I2C总线启动信号 */
	ICCARD_M3_I2C_Start();
	
	/* 第2步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
	ICCARD_M3_I2C_SendByte( _ucDevAddr | I2C_WR);	/* 此处是写指令 */
	
	/* 第3步：接收ACK */
    
	if (E2_Check_I2C_Ack())
	{
		return 0;	/* EEPROM器件无应答 */
	}

	/* 第4步：发送字节地址，24C02只有256字节，因此1个字节就够了，如果是24C04以上，那么此处需要连发多个地址 */
	ICCARD_M3_I2C_SendByte((u8)( _usAddress )&0x0FF  );
	
	/* 第5步：发送ACK */
	if (E2_Check_I2C_Ack())
	{
		return 0;	/* EEPROM器件无应答 */
	}
  	
	/* 第6步：重新启动I2C总线。前面的代码的目的向EEPROM传送地址，下面开始读取数据 */
	ICCARD_M3_I2C_Start();
	
	/* 第7步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
	ICCARD_M3_I2C_SendByte( _ucDevAddr | I2C_RD);	/* 此处是读指令 */
	
	/* 第8步：发送ACK */
	if (E2_Check_I2C_Ack())
	{
		return 0;	/* EEPROM器件无应答 */
	}	
	
	/* 第9步：循环读取数据 */
	for (i = 0; i < _usSize; i++)
	{
		_pReadBuf[i] = ICCARD_M3_I2C_ReadByte() ;	/* 读1个字节 */
		
		/* 每读完1个字节后，需要发送Ack， 最后一个字节不需要Ack，发Nack */
		if (i != _usSize - 1)
		{
			ICCARD_M3_I2C_SendAck();	/* 中间字节读完后，CPU产生ACK信号(驱动SDA = 0) */
		}
		else
		{
			ICCARD_M3_I2C_SendNAck();	/* 最后1个字节读完后，CPU产生NACK信号(驱动SDA = 1) */
		}
	}
	/* 发送I2C总线停止信号 */
	ICCARD_M3_I2C_Stop();
    ////////////
    return 1;	/* 执行成功 */
}
/**********************************************************************************************************
*	函 数 名: E2_WriteOneByte
*	功能说明: 向串行EEPROM指定地址写入一个数据
*	形    参：data : 数据
*			  _ucDevAddr : 设备地址
*			  _ucAddress : 写入地址
*	返 回 值: 0 表示失败，1表示成功
**********************************************************************************************************/
u8 E2_WriteOneByte(u8 data, unsigned char _ucDevAddr, unsigned char _ucAddress)
{
    unsigned char m;

    /*　第０步：发停止信号，启动内部写操作　*/
    ICCARD_M3_I2C_Stop();

    for (m = 0; m < 100; m++)
	{
		/* 第1步：发起I2C总线启动信号 */
		ICCARD_M3_I2C_Start();
		
		/* 第2步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
		ICCARD_M3_I2C_SendByte( _ucDevAddr | I2C_WR );	/* 此处是写指令 */
		
		/* 第3步：发送一个时钟，判断器件是否正确应答 */
		if (ICCARD_M3_I2C_CheckAck() == 0)
		{
			break;
		}
	}
	if (m  == 100)
	{
        ICCARD_M3_I2C_Stop();
        return 0;
	}

    /* 第4步：发送字节地址，24C02只有256字节，因此1个字节就够了，如果是24C04以上，那么此处需要连发多个地址 */
    ICCARD_M3_I2C_SendByte(_ucAddress & 0xFF );

    /* 第5步：发送ACK */
    if (E2_Check_I2C_Ack())
    {
        return 0;	/* EEPROM器件无应答 */
    }	
    /* 第6步：开始写入数据 */
    ICCARD_M3_I2C_SendByte(data);

    /* 第7步：发送ACK */
    if (E2_Check_I2C_Ack())
    {
        return 0;	/* EEPROM器件无应答 */
    }

    /* 命令执行成功，发送I2C总线停止信号 */
    ICCARD_M3_I2C_Stop();
    ////////////
    return 1;
}
/**********************************************************************************************************
*	函 数 名: E2_WriteBytes
*	功能说明: 向串行EEPROM指定地址写入若干数据，采用页写操作提高写入效率
*	形    参：_pWriteBuf : 存放读到的数据的缓冲区指针
*			  _ucDevAddr : 设备地址
*			  _usAddress : 起始地址
*			  _usSize : 数据长度，单位为字节
*	返 回 值: 0 表示失败，1表示成功
**********************************************************************************************************/
u8 E2_WriteBytes(u8 *_pWriteBuf, u16 _ucDevAddr, u16 _usAddress, u16 _usSize)
{

	u16 i,m;

	u16 usAddr;
	
	/* 
		写串行EEPROM不像读操作可以连续读取很多字节，每次写操作只能在同一个page。
		对于24xx02，page size = 8
		简单的处理方法为：按字节写操作模式，每写1个字节，都发送地址
		为了提高连续写的效率: 本函数采用page wirte操作。
	*/

	usAddr = _usAddress;	

	for (i = 0; i < _usSize; i++)
	{
		/* 当发送第1个字节或是页面首地址时，需要重新发起启动信号和地址 */
		if ((i == 0) || (usAddr & ( 64 - 1)) == 0)
		{
			/*　第０步：发停止信号，启动内部写操作　*/
			ICCARD_M3_I2C_Stop();
			
			/* 通过检查器件应答的方式，判断内部写操作是否完成, 一般小于 10ms 			
			CLK频率为200KHz时，查询次数为30次左右*/
			for (m = 0; m < 100; m++)
			{
				/* 第1步：发起I2C总线启动信号 */
				ICCARD_M3_I2C_Start();
				
				/* 第2步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
				ICCARD_M3_I2C_SendByte( _ucDevAddr | I2C_WR );	/* 此处是写指令 */
				
				/* 第3步：发送一个时钟，判断器件是否正确应答 */
				if (ICCARD_M3_I2C_CheckAck() == 0)
				{
					break;
				}

			}
			if (m  == 100)
			{
                ICCARD_M3_I2C_Stop();
                return 0;
			}
		
			/* 第4步：发送字节地址，24C02只有256字节，因此1个字节就够了，如果是24C04以上，那么此处需要连发多个地址 */
			ICCARD_M3_I2C_SendByte((u8)( usAddr >> 8 )&0x0FF   );
			
			/* 第5步：发送ACK */
			if (E2_Check_I2C_Ack())
			{
                return 0;	/* EEPROM器件无应答 */
			}
      
  			/* 第4步：发送字节地址，24C02只有256字节，因此1个字节就够了，如果是24C04以上，那么此处需要连发多个地址 */
			ICCARD_M3_I2C_SendByte((u8)usAddr & 0x0FF );
			
			/* 第5步：发送ACK */
			if (E2_Check_I2C_Ack())
			{
                return 0;	/* EEPROM器件无应答 */
			}     
		}	
		/* 第6步：开始写入数据 */
		ICCARD_M3_I2C_SendByte(_pWriteBuf[i]);
	
		/* 第7步：发送ACK */
		if (E2_Check_I2C_Ack())
		{
            return 0;	/* EEPROM器件无应答 */
		}
        
		usAddr++;	/* 地址增1 */
	}
	
	/* 命令执行成功，发送I2C总线停止信号 */
	ICCARD_M3_I2C_Stop();
    ////////////
    return 1;
}
/*********************************************************************************************************
*	函 数 名: ee_CheckOk
*	功能说明: 判断串行EERPOM是否正常
*	形    参：无
*	返 回 值: 1 表示正常， 0 表示不正常
*********************************************************************************************************/
u8 EEPROM_HL_CheckOk(void)
{	
    if(ICCARD_M3_I2C_CheckDevice(EE_24XX_HL_DEV_ADDR ) == 0)
    {	
        return 1;	
    }
    else
    {		
        ICCARD_M3_I2C_Stop();           /* 失败后，切记发送I2C总线停止信号 */				
        return 0;
    }
}
/*********************************************************/
u8 EEPROM_24XX_HL_Byte_Write(const u16 Address, const u8 data)
{
    if( Address > EE_24XX_HL_SIZE ) 
    {
        assert_param(0);
        return(0);
    }
    //////////////
    return E2_WriteBytes( (u8 *) &data,EE_24XX_HL_DEV_ADDR, Address, 1 ); 
}

/*********************************************************/

u16 EEPROM_24XX_HL_Buf_Read(const u16 InAddr, u8 * pOutBuf,  u16 OutLength )
{
  if(( pOutBuf == NULL) || ( OutLength == 0 ) || ( InAddr > EE_24XX_HL_SIZE ) || ( InAddr + OutLength  > EE_24XX_HL_SIZE  )  )
  {
      assert_param(0);
      return(0);
  }  
  return E2_ReadBytes(pOutBuf,EE_24XX_HL_DEV_ADDR, InAddr, OutLength );
}

/*********************************************************/

u16 EEPROM_24XX_HL_Buf_Write( const u16 InAddr, u8 * pInBuf,  u16 InLength  )
{
  if(( pInBuf == NULL) || (InLength == 0) || ( InAddr > EE_24XX_HL_SIZE ) || ( InAddr+ InLength > EE_24XX_HL_SIZE ) )
  {
      assert_param(0);
      return(0);
  }
  return E2_WriteBytes( pInBuf,EE_24XX_HL_DEV_ADDR ,InAddr , InLength ); 
}

#ifdef EEPROM_24XX_USE_ALL_PAGE
/*********************************************************/

u16 EEPROM_24XX_HL_Buf_Write_By_Page( const u16 InPage, u8 * pInBuf , u16 InLength   )
{
    u8 *pIn ;

    u16 addr;
    
    addr = InPage*EE_24XX_HL_PAGE_SIZE;
    
    if( InLength < EE_24XX_HL_PAGE_SIZE )
    {
        return  0 ;
    }
    
    pIn = pInBuf ; 

    EEPROM_24XX_HL_Buf_Write( addr , pIn , EE_24XX_HL_PAGE_SIZE  );
 
    return 1;
}

/*********************************************************/

u8 EEPROM_24XX_HL_Init(void )
{
    if (ICCARD_M3_I2C_CheckDevice( EE_24XX_HL_DEV_ADDR ) == 0)
    {
	   return 1;
    }
    else
    {
    	ICCARD_M3_I2C_Stop();    /* 失败后，切记发送I2C总线停止信号 */		
		return 0;
    }
}

/*********************************************************/

u8 EEPROM_24XX_HL_SelCheck( void )
{
  
  u8 temp[ EE_24XX_HL_SELTEST_LEN ];

  u8 i;
    
  for( i = 0 ; i < sizeof( temp ) ; i++ )
  {

      temp[ i ] = i + 'A' ;

  }
  
  EEPROM_24XX_HL_Buf_Write(  EE_24XX_HL_SELTEST_ADDR  , temp ,  sizeof( temp )  );
  
  memset( temp , 0 , sizeof( temp ) );
  
  EEPROM_24XX_HL_Buf_Read(  EE_24XX_HL_SELTEST_ADDR  , temp ,  sizeof( temp )  );
  
  for( i = 0 ; i < sizeof( temp ) ; i++ )
  {

     if(  temp[ i ]  != ( i + 'A' ) )break;

  }
  
  if( i == sizeof( temp ) )
  {

      return 0; 

  }
  else
  {

      return 1; 

  }
  
}

/*********************************************************/

u16 EEPROM_24XX_HL_Buf_Read_By_Page( const u16 InPage, u8 * pOutBuf , u16 OutLength   )
{

    u8 *pOut ;

    u16 addr;
    
    addr = InPage*EE_24XX_HL_PAGE_SIZE;
    
    if( OutLength < EE_24XX_HL_PAGE_SIZE )
    {
        return  0 ;
    }
    
    pOut = pOutBuf ; 

    EEPROM_24XX_HL_Buf_Read( addr , pOut , EE_24XX_HL_PAGE_SIZE  );

    return 1;
}

/*********************************************************/

u8 EEPROM_24C128_AllChip_Read( void )
{  

    u16 iPage;
  
    u8 buf[EE_24XX_HL_PAGE_SIZE];
   
    if( buf == NULL )
    {
      return 0; 
    }
    
    for( iPage = 0 ; iPage < EE_24C128_PAGE_TOTAL ; iPage++ )
    {
        EEPROM_24XX_HL_Buf_Read_By_Page( iPage , buf ,  sizeof( buf ) );
     }    
    
    return 0 ;

}

u8 EEPROM_24C128_AllChip_Read_From_EEPROM2RAM( void  )
{  

    u16 iPage;

    memset(  g_sttAT24C128Ctrl.buf , 0 ,  sizeof( g_sttAT24C128Ctrl.buf )   );
    
    for( iPage = 0 ; iPage < EE_24C128_PAGE_TOTAL ; iPage++ )
    {

        EEPROM_24XX_HL_Buf_Read_By_Page( iPage , &g_sttAT24C128Ctrl.buf[ iPage*EE_24XX_HL_PAGE_SIZE ] , EE_24XX_HL_PAGE_SIZE );

    }
  
    g_sttAT24C128Ctrl.IsValid = 1;

    return 0 ;

}

u8 EEPROM_24C128_AllChip_Write_From_RAM2EEPROM( void  )
{  

    u16 iPage;    
   
    if( 1 != g_sttAT24C128Ctrl.IsValid )
    {

        return 0 ;

    }
    
    for( iPage = 0 ; iPage < EE_24C128_PAGE_TOTAL ; iPage++ )
    {

        EEPROM_24XX_HL_Buf_Write_By_Page( iPage , &g_sttAT24C128Ctrl.buf[ iPage*EE_24XX_HL_PAGE_SIZE ] , EE_24XX_HL_PAGE_SIZE );

    }  

    return 0 ;

}
#endif

u8 SL4442_Buffer_Read(  u8 *pBuf, u16  LengthBuf )
{

    u8 sl4442pwd[3]={ 0xFF, 0xFF, 0xFF };

    u8 ret;

    u8 i;

    LengthBuf = LengthBuf;

    ret =Power_On();

    if( ret == 1)
    {
        for(i=1;i<4;i++) //校对3字节的密码
        {
            SendComm(CVD_COMM,i,sl4442pwd[i-1] ); //发出校对命令
            Proce_Mod();              //处理
        }

        Rmm( 0x00 , pBuf , 128 );

        return 	1;
    } 
    else 
    {
        return 0;
    }
}
/******************************************************************************
**                            End Of File
******************************************************************************/


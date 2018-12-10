/********************************************************************
//版权说明	:Shenzhen E-EYE Co. Ltd., Copyright 2009- All Rights Reserved.
//文件名称	:Subpacket.c		
//功能		:实现分包补传功能（对应平台命令0x8003）
//版本号	:
//开发人	:dxl  
//开发时间	:2013.7
//==============================================================================
* 修改者   : Shigle
* 修改时间 : 2013年11月18日
* 2013年11月18日为基线版本0.1 后续功能如有增减或修改，将要求以下追加说明
* 1、本固件程序增加了多媒体数据补传功能 ，由任务函数Subpackage_Task_Media()调试。
* 2、多媒体数据补传机制如下简述:
          A.多媒体数据补传的触发源在RadioProtocol.c文件的函数RadioProtocol_MultiMediaUploadAck里。
          B.当终端向平台连续上传多媒体数据时，因数据量大，所以需要分包传输。
            终端先一次连续将分包传给平台，平台不需要针对接收的每个终端包应答。平台只对终端上传的最后一个应答。
            平台会根据终端第一包所表述的包数，统计和整合多媒体数据.
            B1.如果均正确接收，且拼合成功，则平台下发的"多媒体数据上传应答"命令中，"多媒体ID"字段无后续数据。
            B2.如果平台检查发现接收的包有遗漏，或者不完整，校验和不符等错误，则平台下发的"多媒体数据上传应答"命令中，
               "多媒体ID"字段跟随需要补传的信息，主要有三个字段"重传包总数"，"重传包ID列表"。终端将延时，从源多媒体文件中
               定位到需补传的数据位置，读取相应的数据，重将调用"多媒体数据上传"将数据上传给平台。 如果中途出错，将再重发两次(即
               每包数据重发3次),如果任何一包，第4次发送失败，将结束本次多媒体补传任务。
*  3、外部任务向媒体数据补传需要调用的函数共2个，分别是:
           A1.RadioProtocol.c文件的函数RadioProtocol_MultiMediaUploadAck()里，已经添加
           A2.需要在调用RadioProtocol_MultiMediaDataReport()的文件里，调用Subpackage_Media_File_Name_Set()函数，给本任务拷贝多媒体文件名。
*  4、本任务不影响原行驶记录仪补传代码。



***********************************************************************/
//***************包含文件*****************
#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "include.h"

//****************外部变量****************
extern u16	RecorderSerialNum;//行驶记录仪应答流水号
extern u8	RecorderCmd;//行驶记录仪命令

//****************全局变量定义****************

u16  SubpackageList[SUBPACKET_LIST_BUFFER_SIZE];//分包列表,从平台给的应答中获得
u16  SubpackageResendFirstSerial;//分包重传第1包的流水号
u16  SubpackageCurrentSerial;//分包补传当前包流水号
u8   SubpacketUploadFlag;//分包上传标志,1为正在上传,0为不在上传


STT_SUBPACKETTRANSTER_CTRL g_sttSubPacketTransterCtrl ;
/*
分包补传类型,
1:行驶速度
2:位置信息记录;
3:事故疑点记录;
4:超时驾驶记录;
5:驾驶人身份记录;
6:外部供电记录;
7:参数修改记录;
8:速度状态日志
*/
static u8  SubpacketUploadType = 0;

/* 分包的总包数 */
u16 SubpackageTotalPacket = 0 ;




u32 Subpackage_Get_Media_Attribute_From_Memory( FIL *pInfile ,  u8 *pOutBuf   );
u32 Subpackage_Get_Media_Data_From_Memory( FIL *pInfile , u16 InPackIndex , u8 *pOutBuf   );

/*********************************************************************
//函数名称	:Subpackage_UploadTimeTask(void)
//功能		:分包补传定时任务
*********************************************************************/
FunctionalState Subpackage_UploadTimeTask(void)
{
  u16  i;
  u8  Buffer[800];
  u16  BufferLen;
  u8  *p = NULL;
  u8  *q = NULL;
  u16 length;
  u16 j;
  u8  CheckByte;
  //static u8 count = 0;
  u8 channel = CHANNEL_DATA_1;//为编译通过而添加
  
  if(  ( SubpacketUploadType > 0 )&&( SubpacketUploadType < REGISTER_TYPE_MAX ))//行驶记录仪数据分包补传
  {
    
   p = Buffer;
   BufferLen = 0 ;
  

  for( i = 0 ; i < SUBPACKET_LIST_BUFFER_SIZE ; i++ )
  {
      if( 0 != SubpackageList[i] )
      { /* 从0开始，找到第一个有效的需要补传分包 , 则直接跳出循环 */
        break;
      }
  }
        
        
  if(SUBPACKET_LIST_BUFFER_SIZE == i)
  {/* 如果已经没有找到任何 有效的需要补传分包 ，则将分包上传标志,清0，表明不在上传 ，禁掉该任务 ,然后退出循环*/
      SubpacketUploadFlag = 0;
      return DISABLE;//没有重传分包了
  }

  SubpacketUploadFlag = 1 ; //则将分包上传标志, 置1，表明在上传进行中........
  SubpackageCurrentSerial = SubpackageResendFirstSerial+SubpackageList[i]-1;

  if( 1 == SubpackageList[i] )//第1包
  {
      //应答流水号
       *p++ = (RecorderSerialNum&0xff00) >> 8;
       *p++ = RecorderSerialNum&0xff;
        //命令字
       *p++ = RecorderCmd;
  }
      
      //起始字头0X55，0X7A
      *p++ = 0x55;
      *p++ = 0x7A;
      BufferLen += 2;
      
      //命令字
      *p++ = RecorderCmd ;
      BufferLen += 1 ;
      
      //数据块长度,高字节在前
      q = p;
      *p++ = 0;
      *p++ = 0;
      BufferLen += 2;
       //保留字节
      *p++ = 0;
      BufferLen += 1;
       //数据块，获取包数据
      length = Register_GetSubpacketData( p , SubpacketUploadType ,  SubpackageList[i]  );
      *q++ = ( length&0xff00 ) >> 8;
      *q++ = length&0xff;
      BufferLen += length ;
      //校验字节
      CheckByte = 0;
      for( j=0; j<length; j++)
      {
          CheckByte = ( CheckByte ^ *(p+j)  );
      }
      p += length ;
      *p++ = CheckByte ;
      BufferLen++;
      
      //发送，失败会重发20次，20次连续发送失败则停止上传 
      /*
      if(ACK_OK == RadioProtocol_RecorderDataReport(Buffer, BufferLen,SubpackageTotalPacket,SubpackageList[i]))
      {
                              SubpackageList[i] = 0;
                              count = 0;
      }
      else
      {
          count++;
          if(count >= 20)
          {
              count = 0;
              SubpacketUploadFlag = 0;
              for(i=0; i<255; i++)
              {
                  SubpackageList[i] = 0;
              }
             return DISABLE;
          }
      }
      */
      RadioProtocol_RecorderDataReport( channel,Buffer, BufferLen,SubpackageTotalPacket ,SubpackageList[i]  );
      SubpackageList[i] = 0;
  }
  else//其它的分包补传还没有做
  {
      SubpacketUploadFlag = 0;
      for(i=0; i<SUBPACKET_LIST_BUFFER_SIZE; i++)
      {
          SubpackageList[i] = 0;
      }
      
      if( ( MEDIT_TYPE_PIC == g_sttSubPacketTransterCtrl.MediaType ) ||  ( MEDIT_TYPE_SOUND == g_sttSubPacketTransterCtrl.MediaType )  )
      {
          Subpackage_Task_Media();
          return ENABLE;  
      }
      else
      {
     // RadioProtocol_MultiMediaDataReport(u8 *pBuffer, u16 BufferLen, u16 TotalPacket, u16 Packet);
          return DISABLE;
      }

  }

  
  return ENABLE;  
}




/*********************************************************************
//函数名称 : Subpackage_Media_File_ID_IsEmpty(void)
//函数功能 : 检测是否有待补传的ID号 , 
*********************************************************************/
u8 Subpackage_Media_File_ID_IsEmpty( void  )
{
  return (( g_sttSubPacketTransterCtrl.PacketIDCntCursor == g_sttSubPacketTransterCtrl.PacketIDCntTotal )? 1:0 );
}


/*********************************************************************
//函数名称 : Subpackage_Media_File_ID_Inc(void)
//函数功能 : 移动待补传的ID缓冲区指针，取下一个待补传的ID号 , 
*********************************************************************/
u8 Subpackage_Media_File_ID_Inc( void  )
{

  u8 ret = 0 ;
  /* 待补传的总ID数 不能为0 ，至少有一个 */
  if( g_sttSubPacketTransterCtrl.PacketIDCntTotal < 1 )
  {
    return ret ; 
  }
  
  g_sttSubPacketTransterCtrl.PacketIDCntCursor++ ;
  
  /* 从0到 g_sttSubPacketTransterCtrl.PacketIDCntTotal-1 ,*/
  if( g_sttSubPacketTransterCtrl.PacketIDCntCursor >= g_sttSubPacketTransterCtrl.PacketIDCntTotal )
  {
      for( u8 i = 0 ; i < g_sttSubPacketTransterCtrl.PacketIDCntTotal ; i++ )
      {
         g_sttSubPacketTransterCtrl.PacketIDArray[i] = 0 ;
      }
      
      /* !!! 注意经下语句不能前置 */
      g_sttSubPacketTransterCtrl.PacketIDCntCursor = 0 ;
      g_sttSubPacketTransterCtrl.PacketIDCntTotal = 0;
      g_sttSubPacketTransterCtrl.MediaType = 0 ; 

      return ret ;
  }
  else
  {
      g_sttSubPacketTransterCtrl.PacketCntCursor = g_sttSubPacketTransterCtrl.PacketIDArray[ g_sttSubPacketTransterCtrl.PacketCntCursor ]-1 ; 
      ret = 1 ;
      return ret ;
  }

}


/*********************************************************************
//函数名称 : Subpackage_Media_File_Name_Set
//函数功能 : 给多媒体分包补传的任务，控制需要补传的文件名， 如果文件名超出允许的长度，返回0，否则返回实际的设置的字符串长度（不含尾字节'\0')
*********************************************************************/
u8 Subpackage_Media_File_Name_Set(  const char * pInMediaFileName  )
{

  u8 OutLength = 0 ;

  memset(  g_sttSubPacketTransterCtrl.FileName, 0 , sizeof(  g_sttSubPacketTransterCtrl.FileName ) );
  
  if(  ( strlen ( pInMediaFileName)+1 ) >  sizeof( g_sttSubPacketTransterCtrl.FileName ) )
  {
      OutLength = 0 ;
  }
  else
  {
      OutLength =  strlen ( pInMediaFileName);
      strcpy((char *)g_sttSubPacketTransterCtrl.FileName ,  pInMediaFileName );
  }
  

  return  OutLength ;
}

/*********************************************************************
//函数名称 : Subpackage_Open_Media_File_From_Memory
//函数功能 : 根据文件名，打开文件，并获得该文件的控制句柄，并计算出分包传统的总包数
*********************************************************************/
u8 Subpackage_Media_File_Open(  const char * pInMediaFileName , FIL * pFile   )
{
  u8 ret = 0;
  u8 TempDir[100];

  memset( TempDir , 0 , sizeof( TempDir ) );
  strcpy( (char *)TempDir , pInMediaFileName );

  if(FR_OK == f_open(  pFile, ff_NameConver( TempDir ) ,FA_READ  )  )
  {
      g_sttSubPacketTransterCtrl.PacketCntTotal = ( pFile->fsize - 36 )/512;
      if(0 != (  pFile->fsize  - 36 )%512)
      {
          g_sttSubPacketTransterCtrl.PacketCntTotal++;
      }
      ret = 1;
  }
  else
  {
    
  }

  return ret ;
  
}


/*********************************************************************
//函数名称 : Subpackage_Media_File_Close
//函数功能 : 关闭文件
*********************************************************************/
u8 Subpackage_Media_File_Close(  FIL * pFile   )
{
  
  u8 ret = 0;
  
  if(FR_OK == f_close(  pFile )  )
  {
      ret = 1;
  }
  else
  {
      ret = 0;
  }

  return ret ;
  
}

/*********************************************************************
//函数名称 : Subpackage_Media_File_Buf_Attribute_Get
//函数功能 : 通用文件控制句柄，获取文件36字节属性信息
*********************************************************************/
u32 Subpackage_Media_File_Buf_Attribute_Get( FIL *pInfile ,  u8 *pOutBuf   )
{
  
 u32 Byte = 0 ;
 /********************************************************/
  /* 入口参数 检查*/
  if( ( NULL == pInfile ) || ( NULL == pOutBuf ) )
  {
    return Byte; 
  }
  
  
  if( pInfile->fsize < 36 )
  {
    return Byte; 
  }
   
  /* 移动文件内偏移指针 */
  if( FR_OK != f_lseek( pInfile , pInfile->fsize-36 ) )//移动文件指令到属性起始处(起文件末尾36字节为属性信息
  {
    return Byte;
  }
  
  /* 读取文件内数据 */
  if( FR_OK == f_read ( pInfile, pOutBuf, 36, &Byte) )
  {
    f_lseek( pInfile , 0);
  }

  /* 返回实际的字节数 */
  return Byte ; 

}



/*********************************************************************
//函数名称 : Subpackage_Media_File_Buf_Data_Get
//函数功能 :  通用文件控制句柄，以及文件内的包序号，获取文件512字节文件字节流
*********************************************************************/
u32  Subpackage_Media_File_Buf_Data_Get( FIL *pInfile , u16 InPackIndex , u8 *pOutBuf   )
{

   u32 Byte = 0 ;

   
 /********************************************************/
  /* 入口参数 检查*/
  if( ( NULL == pInfile ) || ( NULL == pOutBuf ) )
  {
    return Byte; 
  }
  
  /* 移动文件内偏移指针 */
  if( FR_OK != f_lseek( pInfile , 0 ) )//移动文件指令到属性起始处(起文件末尾36字节为属性信息
  {
    return Byte;
  }


  /* 移动文件内偏移指针 */
  if( FR_OK != f_lseek( pInfile , InPackIndex*512 ) )//移动文件指令到属性起始处(起文件末尾36字节为属性信息
  {
    return Byte ;
  }


  /* 读取文件内数据 */
  if( FR_OK == f_read ( pInfile , pOutBuf , 512 , &Byte ) )
  {
    f_lseek( pInfile , 0);
  }

  /* 返回实际的字节数 */
  return Byte ; 

}


/*********************************************************************
//函数名称 : Subpackage_Media_File_Packet_Get
//函数功能 : 通用文件控制句柄，以及文件内的包序号，获取36字节文件属性 和 文件512字节文件字节流
*********************************************************************/
u16  Subpackage_Media_File_Buf_Packet_Get(  FIL *pInfile , u16 InPackIndex , u8 * pOutBuf )
{
 //  Subpackage_Open_Media_File_From_Memory(
  u16 len = 0 ;
  u16 retlen = 0 ;
  
  len = ( u16 ) Subpackage_Media_File_Buf_Attribute_Get( pInfile , pOutBuf );
  
  if( len != 36 )
  {
      retlen = 0 ;
      return retlen ;  
  }
  else
  {
      retlen += len ;
      /* 替换 多媒体ID */
      *( pOutBuf + 0 ) = (u8 ) ( (g_sttSubPacketTransterCtrl.PacketCntCursor&0xFF00) >> 8 ) ;
      *( pOutBuf + 1 ) = (u8 ) ( (g_sttSubPacketTransterCtrl.PacketCntCursor&0x00FF) >> 0 ) ; 
  
  }

  len = ( u16 ) Subpackage_Media_File_Buf_Data_Get( pInfile , InPackIndex , pOutBuf+32 );
  
  if( len < 1)
  {
      retlen = 0 ;
      return retlen ;  
  }
  else
  {
      retlen += len ;
  }
  
  return retlen;
  
}



/*********************************************************************
//函数名称 : Subpackage_Task_Media(void)
//函数功能 : 多媒体分包补传任务，含打开文件，取数，关闭文件，打包数据，往平台上传数据，以及修正下一分包ID 
*********************************************************************/
void Subpackage_Task_Media( void )
{
   u8 channel = CHANNEL_DATA_1;//为编译通过而添加
  
  /* 首先，检测是否有待补传的ID , 如果为空，则直接退出 */
  if( Subpackage_Media_File_ID_IsEmpty() )
  {
    return ;    
  }
  
  
 if( 0 == g_sttSubPacketTransterCtrl.ResendCnt )
 {
  // g_sttSubPacketTransterCtrl.PacketCntCursor = 1;
   
   
  /* 获取文件名 */
#if 1
   if( 0 == strlen( (char *)g_sttSubPacketTransterCtrl.FileName ) )
   {
     return ; 
   }
#else
//  memset(  g_sttSubPacketTransterCtrl.FileName, 0 , sizeof(  g_sttSubPacketTransterCtrl.FileName ) );
//  strcpy((char *)g_sttSubPacketTransterCtrl.FileName ,"1:/JPG1/15155028.jpg");
  
  Subpackage_Media_File_Name_Set( "1:/JPG1/15155028.jpg" ); 
 
#endif
  
  /* 打开文件, 获取文件句柄  */
  if( 0 == Subpackage_Media_File_Open( (char *)g_sttSubPacketTransterCtrl.FileName , &g_sttSubPacketTransterCtrl.FileHandler ) )
  {
      return ; 
  }
  /* 获取文件数据 */
  g_sttSubPacketTransterCtrl.DataBufLength = Subpackage_Media_File_Buf_Packet_Get( &g_sttSubPacketTransterCtrl.FileHandler , g_sttSubPacketTransterCtrl.PacketCntCursor , g_sttSubPacketTransterCtrl.DataBuf  );

  /* 关闭文件 */
  Subpackage_Media_File_Close( &g_sttSubPacketTransterCtrl.FileHandler  );

  /* 判断，获取的数据长度是否有效， 如果无效，则直接返回退出 */
  if( 0 == g_sttSubPacketTransterCtrl.DataBufLength )
  {
     return ;
  }

   g_sttSubPacketTransterCtrl.ResendCnt = 1;
  
 }
 else if( g_sttSubPacketTransterCtrl.ResendCnt < 3 ) //重发3次  1, 2 ,此时不需要重新从SD卡地取数据
 {
    g_sttSubPacketTransterCtrl.ResendCnt++;
 }
 else //第3次，及以上停止任务
 {
     g_sttSubPacketTransterCtrl.MediaType = 0 ;    
 }
  
  /* 通过无线平台发送数据 */
  if(  ACK_OK == RadioProtocol_MultiMediaDataReport( channel,
                                                     g_sttSubPacketTransterCtrl.DataBuf , 
                                                     g_sttSubPacketTransterCtrl.DataBufLength ,
                                                     g_sttSubPacketTransterCtrl.PacketCntTotal ,
                                                     g_sttSubPacketTransterCtrl.PacketCntCursor )
  )
  {
    
    /* 如果 "多媒体数据上传0x0801" 成功，则移动传下一个待传ID */
    Subpackage_Media_File_ID_Inc();
    g_sttSubPacketTransterCtrl.ResendCnt = 0 ;//发送成功，重发次数肖零

  }
  else
  {
     //开启分包补传任务
    //发送失败后，3秒后，再次得触发
     SetTimerTask( TIME_SUBPACKAGE, 3*SECOND ); 
  }

  


}

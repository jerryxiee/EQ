/*************************************************************************
* Copyright (c) 2013,深圳市伊爱高新技术开发有限公司
* All rights reserved.
* 文件名称 : JointechOiL_Protocol.c
* 当前版本 : 0.1
* 开发者   : Shigle
* 修改时间 : 2013年10月30日 

* 2013年10月30日为基线版本0.1 后续功能如有增减或修改，将要求以下追加说明
* 1、本固件程序针对深圳久通机油位传感器JT606协议开发。
* 2、终端每10秒（暂定 , 后期参数可配 )读取一次油位传感器的油量数据
*************************************************************************/


/***************   包含文件区   ********************************/

#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "include.h"

/***************   本地全局变量 定义区   ********************************/

STT_JOINTECH_OILCOST_PROTOCOL g_sttJointechOilCostA;
STT_JOINTECH_OILCOST_PROTOCOL g_sttJointechOilCostR;

STT_JOINTECH_COM     g_sttJointechComTx;
STT_JOINTECH_COM     g_sttJointechComRx;

STT_JOINTECH_OILCOST g_sttJointechVar ;

static STT_JOINTECH_OILCOST_CTRL g_sttJointechCtrl;
/***************   外部变量 定义区   ********************************/
extern u8  Uart1DataDownTransFlag;//串口1数据透传标志，
extern u16 Uart1DataDownTransCmd;//串口1数据透传数据中的命令字
extern u8  DelayBuffer[100];//延时缓冲，可把要延时发送的数据拷贝到这里，注意最大长度限制
extern u8  DelayBufferLen;//延时缓冲数据长度
extern u16     DelayAckCommand;//延时应答命令


/***************   本地局部函数 声明区   ********************************/


static void JointechOiLCost_StageMain_Set( u8 iStageMain );
static void Task_JointechOiLCost_Init( );
static void JointechOiLCost_StateMachine( );


static void JointechOiLCost_Operation_Damp( u8 InOpCode , u16 InDamp );
static void JointechOiLCost_Operation_TemperatureCompensation( u8 InOpCode );

/***************   本地函数 定义区   ********************************/

static LZM_TIMER g_ArrayTimeTask_JointechOil[ TASK_JOINTECHOIL_MAX ];



/*********************************************************************
*名称 : JointechOiLCost_COM_Buf_Write
*功能 : 深圳久通机电油位传感器 COM 批量写入字节流
*备注 :
*********************************************************************/
void JointechOiLCost_COM_Buf_Write(  u8 * pInBuf , u16 InLength )
{
  
  //TaximeterCommSendData( pInBuf, InLength );//dxl,2014.02.22,摄像头与玖通共用485接口,以前老的EGS701的接口,采用以下接口函数
  //Usart3_SendData(pInBuf, InLength);,dxl,2014.5.12屏蔽,油耗由32PIN 485接口改到16PIN 232接口,采用以下接口函数
    COM2_WriteBuff(  pInBuf , InLength );
  
}

/*********************************************************************
*名称 : JointechOiLCost_Protocol_Verify_Calc
*功能 : 深圳久通机电油位传感器 协议检验和计算
*备注 :
*********************************************************************/
u8 JointechOiLCost_Protocol_Verify_Calc( const u8 *pInBuf , u16 InLength )
{
  u8 verify = 0 ;
  const u8 *p = pInBuf ;
  
  for( u16 i = 0 ; i < InLength ; i++ )
  {
    verify += p[i] ;
  }
  return verify ;

}



/*********************************************************************
*名称 : JointechOiLCost_Is_ASCII_NumDec
*功能 : 判断是否是十进制数ASCII码 ， 是返回1  否返回 0 
*备注 : 
*********************************************************************/
u8 JointechOiLCost_Is_ASCII_NumDec( u8 pIn )
{
  
  u8 ret = 1 ;
  
  if( ( pIn < '0' )|| ( pIn > '9' ) )
  {
     ret =  0;
  }
  
  return ret ; 
    
  
}

/*********************************************************************
*名称 : JointechOiLCost_Is_ASCII_NumHex
*功能 : 判断是否是十六进制数ASCII码 ， 是返回1  否返回 0 
*备注 : 
*********************************************************************/
u8 JointechOiLCost_Is_ASCII_NumHex( u8 pIn )
{
  
  u8 ret = 0 ;
  
  if( ( pIn >= '0' )&& ( pIn <='9' ) )
  {
     ret =  1; 
  }
  else if( ( pIn >= 'A' )&& ( pIn <='F' ) )
  {
     ret =  1; 
  }
  
  return ret ; 

}


/*********************************************************************
*名称 : JointechOiLCost_2ASCII_To_1Hex
*功能 : 将两个字节的十六进制ASCII字符 ，转化成1个字节的十六进制数
*备注 : 
*********************************************************************/
u8 JointechOiLCost_2ASCII_To_1Hex( const u8 *pIn )
{
  u8 ret = 0 ;
  u8 HexAscii[ 16 ] = { '0' , '1' , '2' , '3' , '4' , '5' , '6' , '7' , '8' , '9' , 'A', 'B' , 'C', 'D' , 'E' , 'F' };
  u8 i = 0 ;
  const u8 * p = pIn ;
  
  
  for( i = 0 ; i < 16 ; i++ )
  {
    if( *p == HexAscii[ i ])
      break;
  }
  
  ret = i ;
  ret <<= 4;

   p++ ;
   for( i = 0 ; i < 16 ; i++ )
  {
    if( *p == HexAscii[ i ])
      break;
  }
  
  ret += i ;
  
  return ret;

 }



/*********************************************************************
*名称 : JointechOiLCost_2ASCII_To_1Hex
*功能 : 将1个字节的十六进制数转化成两个字节的十六进制ASCII字符
*备注 : 
*********************************************************************/
u8 JointechOiLCost_1Hex_To_2ASCII( u8 pIn , u8 *pOutBuf )
{
  u8 HexAscii[ 16 ] = { '0' , '1' , '2' , '3' , '4' , '5' , '6' , '7' , '8' , '9' , 'A', 'B' , 'C', 'D' , 'E' , 'F' };
  u8 ret = 2 ;
  u8 * p = pOutBuf ;
  
  *p++ =  HexAscii[ pIn / 16 ];
  *p   =  HexAscii[ pIn % 16 ];
  
  return ret;

 }



/*********************************************************************
*名称 : JointechOiLCost_2ASCII_To_1Dec
*功能 : 将两个字节的十进制ASCII字符 ，转化成1个字节的十进制数
*备注 : 
*********************************************************************/
u8 JointechOiLCost_2ASCII_To_1Dec( const u8 *pIn )
{
  u8 ret = 0 ;
  u8 DecAscii[ 10 ] = { '0' , '1' , '2' , '3' , '4' , '5' , '6' , '7' , '8' , '9'  };
  u8 i = 0 ;
  const u8 * p = pIn ;
  
  
  for( i = 0 ; i < 10 ; i++ )
  {
    if( *p == DecAscii[ i ])
      break;
  }
  
  ret = i ;
  ret <<= 4;
  
  
   p++ ;
   for( i = 0 ; i < 10 ; i++ )
  {
    if( *p == DecAscii[ i ])
      break;
  }
  
  ret += i ;
  
  return ret;

 }

/*********************************************************************
*名称 : JointechOiLCost_2ASCII_To_1Hex
*功能 : 将1个字节的十进制数转化成两个字节的十进制ASCII字符
*备注 : 
*********************************************************************/
u8 JointechOiLCost_1Dec_To_2ASCII( u8 pIn , u8 *pOutBuf )
{
  u8 HexAscii[ 16 ] = { '0' , '1' , '2' , '3' , '4' , '5' , '6' , '7' , '8' , '9' };
  u8 ret = 2 ;
  u8 * p = pOutBuf ;
  
  *p++ =  HexAscii[ pIn / 10 ];
  *p   =  HexAscii[ pIn % 10 ];
  
  return ret;

 }
/*********************************************************************
*名称 : JointechOiLCost_Process_CMD
*功能 : 深圳久通机电油位传感器命令处理  命令 A
*备注 : 指标标识 A  设备波特率
*********************************************************************/
u16 JointechOiLCost_Protocol_Extract( const u8 *pInBuf, u16 InLength, u8 *pOutBuf   )
{
   
  u16  i = 0 ;
  u8  j = 0;
  u8  iPosition[2]={ 0, 0 };
  u16 iTempLength = 0 ;
  u8  flag = 0;
  u8  cnt = 0 ;
  if( InLength<8){
    return iTempLength;
    
  }
  
  
  for( i = 0 ; i < InLength ; i++ ){

     /* 找包头'$" */
      if( '@' == pInBuf[ i ] ){
          
            flag = 1;
            cnt = 0 ;
			      iPosition[j] = i ;
            j++;
            continue;
		    }
      
        
      /* 找包尾'$" */ 
      if( 1 == flag ){
        
        cnt++;
        
        /* 长时未找到匹配的包尾，清零重来  */ 
        if( cnt > ( MAX_JOINTECH_DATA_LEN+7) ){
            flag = 1;
            cnt = 0 ;
            j = 0 ;
            break;
        }
        
        /* 找到包尾'#" */ 
        if( '#' == pInBuf[ i ] ){
            iPosition[j] = i ;
            j++;
        }
          
        
        
      }
        
                
      if( j == 2){
            break;
      }
  } 

   if( 0 == iPosition[1]){
     
      return iTempLength;
   }
  

   iTempLength = iPosition[1]-iPosition[0]-1 ;
  /* 去包头包尾，至少为7字节( 2字节ID + 1字节命令 + 2字节长度 + 2字节校检和 ) */
   if( iTempLength < 7 ){
     iTempLength = 0 ;
     return iTempLength;
   } 
  
   memcpy( pOutBuf ,  pInBuf+iPosition[0]+1, iTempLength );
   
   /* 校验和 判断 */
   u8 verify0 = JointechOiLCost_Protocol_Verify_Calc( pOutBuf , iTempLength-2 );
   u8 verify1 = JointechOiLCost_2ASCII_To_1Hex( ( u8 * )( pOutBuf+( iTempLength-2) ) );
   if( verify0 != verify1 ){
      iTempLength = 0;
    }
/*把这部分代码挪到接收中断中
   //将油耗传感器的应答数据发送给平台,dxl,2014.3.14
   if((0 != iTempLength)&&(1 == Uart1DataDownTransFlag))
   {
        if(Uart1DataDownTransCmd == *(pInBuf+iPosition[0]+3))
        {
             Uart1DataDownTransFlag = 0;
             Buffer[0] = 1;
             memcpy(Buffer+1,pInBuf+iPosition[0],iTempLength+2);
             RadioProtocol_OriginalDataUpTrans(0x41, Buffer, iTempLength+3);
        }
       
   }
   */
  return iTempLength;
  
}


/*********************************************************************
*名称 : JointechOiLCost_Process_CMD
*功能 : 深圳久通机电油位传感器命令处理  命令 A
*备注 : 指标标识 A  设备波特率
*********************************************************************/
u8 JointechOiLCost_Protocol_Parser( const u8 *pInBuf ,  u16 InLength , STT_JOINTECH_OILCOST_PROTOCOL *pOut )
{
  u8  ret = 0 ;


  const u8 *p = pInBuf ;
  
  /* 输入参数检查 */
  if( ( NULL == pOut ) ||( NULL == pInBuf ) || (InLength < 7 ) )
  {
    return ret ;
  }
  
  /* 取 ID */   

  pOut->id = JointechOiLCost_2ASCII_To_1Hex( p ) ;   
  p += 2;
    
  /* 取 命令 */ 
  pOut->cmd = *p ;
  p += 1 ;
   
  /* 取 长度 */
  pOut->len = JointechOiLCost_2ASCII_To_1Hex( p ) ;   
  p += 2;
  
  /* 判断 长度 */
  if(  InLength != ( pOut->len + 7 ) )
  {
    ret = 0; 
    return ret ;
  }
  
   /* 取数据 */
  if(  pOut->len < 1 )
  {

  }
  else if ( pOut->len > MAX_JOINTECH_DATA_LEN )
  {
    
    ret = 0;
    
  }
  else
  {
      for( u8 i = 0 ; i < pOut->len ; i++ )
      {
          pOut->data[ i ] = *p++;
      }
  }

  /* 取校验字节 */
  pOut->verify = JointechOiLCost_2ASCII_To_1Hex( p );
  
  ret = 1;
  
  return ret ;
  
}

/*********************************************************************
*名称 : JointechOiLCost_Process_CMD
*功能 : 深圳久通机电油位传感器命令处理  命令 A
*备注 : 指标标识 A  设备波特率
*********************************************************************/
u16 JointechOiLCost_Protocol_Packet( STT_JOINTECH_OILCOST_PROTOCOL *pIn ,  u8 *pOutBuf  )
{

 u8 *p = pOutBuf ;
 
  /* 包头 */
  *p++ = '@' ;
  
  /*  ID 号 */
  JointechOiLCost_1Dec_To_2ASCII( pIn->id , p )  ;
  p += 2 ;
  
  /* 命令字 */
  *p++ = pIn->cmd ;
  
  /* 命令长度 */
  JointechOiLCost_1Hex_To_2ASCII(  pIn->len , p );
  p += 2 ;
  
  /* 命令内容 */
  for( u8 i = 0 ; i < pIn->len ; i++ )
  {
    *p++ = pIn->data[ i ] ; 
  }
  
  /* 命令校验和 */
  
  u8 verify = JointechOiLCost_Protocol_Verify_Calc(  pOutBuf+1 , pIn->len +5 ) ;
  JointechOiLCost_1Hex_To_2ASCII(  verify , p );
  p += 2 ;
  
  /* 包尾 */
  *p++ = '#' ;
  
  return (   pIn->len + 9  );
  
}

/*********************************************************************
*名称 : JointechOiLCost_Process_CMD
*功能 : 深圳久通机电油位传感器命令处理  命令 A
*备注 : 指标标识 A  设备波特率
*********************************************************************/
u16 JointechOiLCost_Process_CMD_A( STT_JOINTECH_OILCOST_PROTOCOL *pIn , u8 *pOutBuf )
{
  u16 retlen = 0 ;
  u8 tempbuf[10];
  memset( tempbuf , 0 , sizeof( tempbuf ) );
  memcpy( tempbuf , &pIn->data[ 1 ] , pIn->len-1 );

  if(  NULL != strstr( ( char const *)tempbuf , "4800" ) )
  {
    g_sttJointechVar.iBaud = 0 ;
   
  }
  else if(  NULL !=  strstr( ( char const *)tempbuf , "9600" ) )
  {
    g_sttJointechVar.iBaud = 1 ;
   
  }
  else if(  NULL !=  strstr( ( char const *)tempbuf , "57600" ) )
  {
    g_sttJointechVar.iBaud = 2 ;
   
  }
  else if(  NULL !=  strstr( ( char const *)tempbuf , "115200" ) )
  {
    g_sttJointechVar.iBaud = 3 ;
   
  }
  else
  {
    g_sttJointechVar.iBaud = 0xFF ;
  }

 

  return retlen ;
  
}


/*********************************************************************
*名称 : JointechOiLCost_Process_CMD
*功能 : 深圳久通机电油位传感器命令处理  命令 B
*备注 : 指标标识 B  查询设置调零值
*********************************************************************/
u16 JointechOiLCost_Process_CMD_B( STT_JOINTECH_OILCOST_PROTOCOL *pIn , u8 *pOutBuf )
{
  u16 retlen = 0 ;
  u8 i = 0 ;
  u16 iWord = 0 ;
  
  
  for( i = 0 ; i < pIn->len ; i++ )
  {
     iWord *= 10 ;
     iWord +=  ( pIn->data[ i ]- '0' ) ;
     
  }
  
  g_sttJointechVar.TankZero = iWord ;
  

  return retlen ;
  
}



/*********************************************************************
*名称 : JointechOiLCost_Process_CMD
*功能 : 深圳久通机电油位传感器命令处理  命令 C
*备注 : 指标标识 C  查询设置调满值
*********************************************************************/
u16 JointechOiLCost_Process_CMD_C( STT_JOINTECH_OILCOST_PROTOCOL *pIn , u8 *pOutBuf )
{
  
  u16 retlen = 0 ;
  u8 i = 0 ;
  u16 iWord = 0 ;
  
  
  for( i = 0 ; i < pIn->len ; i++ )
  {
     iWord *= 10 ;
     iWord +=  ( pIn->data[ i ]- '0' ) ;

  }
  
  g_sttJointechVar.TankFull = iWord ;
  return retlen ;
  
}

/*********************************************************************
*名称 : JointechOiLCost_Process_CMD
*功能 : 深圳久通机电油位传感器命令处理  命令 D
*备注 : 指标标识 D  设置油箱体积
*********************************************************************/
u16 JointechOiLCost_Process_CMD_D( STT_JOINTECH_OILCOST_PROTOCOL *pIn , u8 *pOutBuf )
{
  u16 retlen = 0 ;
  u8 i = 0 ;
  u16 iWord = 0 ;
  
  
  iWord = 0 ;
  for( i = 0 ; i < pIn->len ; i++ )
  {
     iWord *= 10 ;
     iWord +=  ( pIn->data[ i ]- '0' ) ;
     
  }
  
  g_sttJointechVar.TankVolume = iWord*10 ;
  
  return retlen ;
  
}




/*********************************************************************
*名称 : JointechOiLCost_Process_CMD
*功能 : 深圳久通机电油位传感器命令处理  命令 E
*备注 : 指标标识 E  读取当前油位值
*********************************************************************/
u16 JointechOiLCost_Process_CMD_E( STT_JOINTECH_OILCOST_PROTOCOL *pIn , u8 *pOutBuf )
{
  u16 retlen = 0 ;
  u8 i = 0 ;
  u16 iWord = 0 ;
  
  /* 取 当前位高度万分比 ， 最大值为10000 */
  iWord = 0 ;
  for( i = 0 ; i < 5 ; i++ )
  {
     iWord *= 10 ;
     iWord +=  ( pIn->data[ 0+i ]- '0' ) ;
     
  }

  g_sttJointechVar.LiquidLevelPercent = iWord ;
  
  /* 取 当前经过调整后的油位值 ，0 ~ 4095 */
  iWord = 0 ;
  for( i = 0 ; i < 4 ; i++ )
  {
     iWord *= 10 ;
     iWord +=  ( pIn->data[ 5+i ]- '0' ) ;
     
  }

  g_sttJointechVar.OilVolumeAdjust = iWord ;
  
  
  /* 取 除以100 为当前的油位升数 ， 如038409 表示为 384.09L  */
  iWord = 0 ;
  for( i = 0 ; i < 6 ; i++ )
  {
     iWord *= 10 ;
     iWord +=  ( pIn->data[ 9 + i ]- '0' ) ;
     
  }

  g_sttJointechVar.OilVolumeCur = iWord ;
  
  
  /* 取 温度 ， 0 表示-40摄氏度 ， 如068 表示为28度 */
  iWord = 0 ;
  for( i = 0 ; i < 3 ; i++ )
  {
     iWord *= 10 ;
     iWord +=  ( pIn->data[ 16 + i ]- '0' ) ;
     
  }
  g_sttJointechVar.Temperature = iWord  ;
  
  
  return retlen ;
  
}


/*********************************************************************
*名称 : JointechOiLCost_Process_CMD
*功能 : 深圳久通机电油位传感器命令处理  命令 G
*备注 : 指标标识 G  设置，查询及探测ID号
*********************************************************************/
u16 JointechOiLCost_Process_CMD_G( STT_JOINTECH_OILCOST_PROTOCOL *pIn , u8 *pOutBuf )
{
  u16 retlen = 0 ;

  
  /* 取 安装序号 */
  g_sttJointechVar.iSetup = JointechOiLCost_2ASCII_To_1Dec( &pIn->data[ 0 ] );

  /* 取 版本号  */
  memcpy( (u8 *)g_sttJointechVar.Version , &pIn->data[ 2 ] , 8 );

  return retlen ;
  
}

/*********************************************************************
*名称 : JointechOiLCost_Process_CMD
*功能 : 深圳久通机电油位传感器命令处理  命令 H
*备注 : 指标标识 H  查询设置阻尼
*********************************************************************/
u16 JointechOiLCost_Process_CMD_H( STT_JOINTECH_OILCOST_PROTOCOL *pIn , u8 *pOutBuf )
{
  u16 retlen = 0 ;
  u8 i = 0 ;
  u16 iWord = 0 ;
  
  /* 取 阻尼 ， 单位：秒  */
  iWord = 0 ;
  for( i = 0 ; i < 4 ; i++ )
  {
     iWord *= 10 ;
     iWord +=  ( pIn->data[ 0+i ]- '0' ) ;
     
  }

  g_sttJointechVar.Damp = iWord ;
  
  return retlen ;
  
}


/*********************************************************************
*名称 : JointechOiLCost_Process_CMD
*功能 : 深圳久通机电油位传感器命令处理  命令 I
*备注 : 指标标识 I  设置是否开启温飘补偿
*********************************************************************/
u16 JointechOiLCost_Process_CMD_I( STT_JOINTECH_OILCOST_PROTOCOL *pIn , u8 *pOutBuf )
{
  u16 retlen = 0 ;

  
  /* 取 是否开启 温飘补偿  0：不开启  1: 开启  0xFF : 无效 */
  if(  '0' == pIn->data[ 0 ] )
  {
     g_sttJointechVar.IsTempCompensation = 0x00 ;
  }
  else  if(  '1' == pIn->data[ 0 ] )
  {
     g_sttJointechVar.IsTempCompensation = 0x01 ;    
  }
  else
  {
     g_sttJointechVar.IsTempCompensation = 0xFF ;    
  }


  return retlen ;
  
}


/*********************************************************************
*名称 : JointechOiLCost_Process_CMD
*功能 : 深圳久通机电油位传感器命令处理  命令 N
*备注 : 指标标识 N  报错命令
*********************************************************************/
u16 JointechOiLCost_Process_CMD_N( STT_JOINTECH_OILCOST_PROTOCOL *pIn , u8 *pOutBuf )
{
  u16 retlen = 0 ;

  u8 ErrorStatus = 0 ;
  
  ErrorStatus = pIn->data[ 0 ];
  
  if( ErrorStatus & 0x01 )
  {
      g_sttJointechVar.FlagErrorFrequency = 1 ; //JT606的输出情况  0:正常  1:出错  
  }
  else
  {
      g_sttJointechVar.FlagErrorFrequency = 0 ; //JT606的输出情况  0:正常  1:出错   
  }

  if( ErrorStatus & 0x02 )
  {
      g_sttJointechVar.FlagErrorTankZero = 1; //零值情况  0:正常  1:出错     
  }
  else
  {
      g_sttJointechVar.FlagErrorTankZero = 0 ; //零值情况  0:正常  1:出错 
  }

  if( ErrorStatus & 0x04 )
  {
      g_sttJointechVar.FlagErrorTankFull = 1 ; //满值情况  0:正常  1:出错     
  }
  else
  {
      g_sttJointechVar.FlagErrorTankFull = 0; //满值情况  0:正常  1:出错     
  }

  return retlen ;
  
}



/*********************************************************************
*名称 : JointechOiLCost_Process_CMD
*功能 : 深圳久通机电油位传感器命令处理  命令 R
*备注 : 指标标识 R  读取关键参数
*********************************************************************/
u16 JointechOiLCost_Process_CMD_R( STT_JOINTECH_OILCOST_PROTOCOL *pIn , u8 *pOutBuf )
{

  u16 retlen = 0 ;
  u16 iWord = 0 ;
  
  /* 取 阻尼值， 最大值为10000 */
  iWord = (u16 )JointechOiLCost_2ASCII_To_1Hex( &pIn->data[ 0 ] );
  iWord <<= 8 ;
  iWord += (u16 )JointechOiLCost_2ASCII_To_1Hex( &pIn->data[ 2 ] );

  g_sttJointechVar.SampleDamp = iWord ;


  /* 取 频率值， 十六进制ASCII , 如： 007A 表示十进制122  */
  iWord = (u16 )JointechOiLCost_2ASCII_To_1Hex( &pIn->data[ 4 ] );
  iWord <<= 8 ;
  iWord += (u16 )JointechOiLCost_2ASCII_To_1Hex( &pIn->data[ 6 ] );

  g_sttJointechVar.SampleFrequence = iWord ;
  

  /* 取 采集到的零值， 十六进制ASCII  */
  iWord = (u16 )JointechOiLCost_2ASCII_To_1Hex( &pIn->data[ 8 ] );
  iWord <<= 8 ;
  iWord += (u16 )JointechOiLCost_2ASCII_To_1Hex( &pIn->data[ 10 ] );

  g_sttJointechVar.SampleTankZero = iWord ;
  
  /* 取 采集到的零值， 十六进制ASCII  */
  iWord = (u16 )JointechOiLCost_2ASCII_To_1Hex( &pIn->data[ 12 ] );
  iWord <<= 8 ;
  iWord += (u16 )JointechOiLCost_2ASCII_To_1Hex( &pIn->data[ 14 ] );

  g_sttJointechVar.SampleTankFull = iWord ;
    
 
  
  
  return retlen ;
  
}

/*********************************************************************
*名称 : JointechOiLCost_Process_CMD
*功能 : 深圳久通机电油位传感器命令处理  命令 04H
*备注 : 04H 采集记录仪脉冲系数
*********************************************************************/
u16 JointechOiLCost_Process_CMD(  STT_JOINTECH_OILCOST_PROTOCOL *pIn , u8 *pOutBuf  )
{
  u16 retlen = 0 ;
  
  /* 输入参数检查 */
  if( NULL == pIn )
  {
     return retlen ;
  }
  
  switch( pIn->cmd )
  {


    case 'A': //指标标识 A  设备波特率
      retlen = JointechOiLCost_Process_CMD_A( pIn , pOutBuf );
    break;

    case 'B': //指标标识 B  查询设置调零值
      retlen = JointechOiLCost_Process_CMD_B( pIn , pOutBuf );
    break;

    case 'C': //指标标识 C  查询设置调满值
      retlen = JointechOiLCost_Process_CMD_C( pIn , pOutBuf );
    break;

    case 'D': //指标标识 D  设置油箱体积
      retlen = JointechOiLCost_Process_CMD_D( pIn , pOutBuf );
    break;
    
    case 'E': //指标标识 E  读取当前油位值
      retlen = JointechOiLCost_Process_CMD_E( pIn , pOutBuf );
    break;

    case 'G': //指标标识 G  设置，查询及探测ID号
      retlen = JointechOiLCost_Process_CMD_G( pIn , pOutBuf );
    break;

    case 'H': //指标标识 H  查询设置阻尼
      retlen = JointechOiLCost_Process_CMD_H( pIn , pOutBuf );
    break;

    case 'I': //指标标识 I  设置是否开启温飘补偿
      retlen = JointechOiLCost_Process_CMD_I( pIn , pOutBuf );
    break;

    case 'N': //指标标识 N  报错命令
      retlen = JointechOiLCost_Process_CMD_N( pIn , pOutBuf );
    break;

    case 'R': //指标标识 R  读取关键参数
      retlen = JointechOiLCost_Process_CMD_R( pIn , pOutBuf );
    break;

   
  }

  return retlen ;
  
}
 


/*********************************************************************
*名称 : JointechOiLCost_Process_CMD
*功能 : 深圳久通机电油位传感器命令处理  命令 A
*备注 : 指标标识 A  设备波特率
*********************************************************************/
u16 JointechOiLCost_Request_CMD_A( STT_JOINTECH_OILCOST_PROTOCOL *pIn , u8 *pOutBuf )
{

  u16 retlen = 0 ;
  u8 iBaud = 1;
  u8 iMode = 0;

 char a_baud[][8] = 
 {
    {"4800"},//0
    {"9600"},//1
    {"57600"},//2
    {"115200"},//3
 };


  pIn->id = 0x01  ;
  pIn->cmd = 'A' ;
  
  pIn->len = 1 + strlen( a_baud[ iBaud ] ) ; 
  pIn->data[ 0 ] = iMode+'0' ; // 0: 表示查询 1: 表示设置
  memcpy( &pIn->data[ 1 ] , a_baud[ iBaud ] , strlen( a_baud[ iBaud ] ) );
          
  retlen =  JointechOiLCost_Protocol_Packet( pIn , pOutBuf  );

  return retlen ;
  
  
}


/*********************************************************************
*名称 : JointechOiLCost_Process_CMD
*功能 : 深圳久通机电油位传感器命令处理  命令 B
*备注 : 指标标识 B  查询设置调零值
*********************************************************************/
u16 JointechOiLCost_Request_CMD_B( STT_JOINTECH_OILCOST_PROTOCOL *pIn , u8 *pOutBuf )
{
  u16 retlen = 0 ;
  u8 iMode = 1;

  
  pIn->id = 0x01  ;
  pIn->cmd = 'B' ;
  
  pIn->len = 1  ; 
  pIn->data[ 0 ] = iMode+'0' ; // 0: 表示查询 1: 表示设置


  retlen =  JointechOiLCost_Protocol_Packet( pIn , pOutBuf  );

  return retlen ;

}



/*********************************************************************
*名称 : JointechOiLCost_Process_CMD
*功能 : 深圳久通机电油位传感器命令处理  命令 C
*备注 : 指标标识 C  查询设置调满值
*********************************************************************/
u16 JointechOiLCost_Request_CMD_C( STT_JOINTECH_OILCOST_PROTOCOL *pIn , u8 *pOutBuf )
{
  u16 retlen = 0 ;
  u8 iMode = 1;

  
  pIn->id = 0x01  ;
  pIn->cmd = 'C' ;
  
  pIn->len = 1  ; 
  pIn->data[ 0 ] = iMode+'0' ; // 0: 表示查询 1: 表示设置


  retlen =  JointechOiLCost_Protocol_Packet( pIn , pOutBuf  );

  return retlen ;

  
}

/*********************************************************************
*名称 : JointechOiLCost_Process_CMD
*功能 : 深圳久通机电油位传感器命令处理  命令 D
*备注 : 指标标识 D  设置油箱体积
*********************************************************************/
u16 JointechOiLCost_Request_CMD_D( STT_JOINTECH_OILCOST_PROTOCOL *pIn , u8 *pOutBuf )
{

 u16 retlen = 0 ;
  u8 iMode = 1;
  u16 iTankVolume = 531 ;
  u8 iByte = 0 ;
  

  pIn->id = 0x01  ;
  pIn->cmd = 'D' ;
  
  
  if( 0x00 == iMode  )
  {
    pIn->len = 1  ; 
    pIn->data[ 0 ] = iMode+'0' ; // 0: 表示查询 1: 表示设置
  }
  else
  {
    pIn->len = 1 + 4 ; 
    pIn->data[ 0 ] = iMode+'0' ; // 0: 表示查询 1: 表示设置
  
    iByte = iTankVolume/100;
    JointechOiLCost_1Dec_To_2ASCII( iByte , &pIn->data[ 1 ] );
 
    iByte = iTankVolume%100;
    JointechOiLCost_1Dec_To_2ASCII( iByte , &pIn->data[ 3 ] );
    
  }
  retlen =  JointechOiLCost_Protocol_Packet( pIn , pOutBuf  );

  return retlen ;
  
}


/*********************************************************************
*名称 : JointechOiLCost_Process_CMD
*功能 : 深圳久通机电油位传感器命令处理  命令 E
*备注 : 指标标识 E  读取当前油位值
*********************************************************************/
u16 JointechOiLCost_Request_CMD_E( STT_JOINTECH_OILCOST_PROTOCOL *pIn , u8 *pOutBuf )
{
  
  u16 retlen = 0 ;

  pIn->id = 0x01  ;
  pIn->cmd = 'E' ; //
  pIn->len = 0x00 ;
  
  retlen =  JointechOiLCost_Protocol_Packet( pIn , pOutBuf  );

  return retlen ;
  
}


/*********************************************************************
*名称 : JointechOiLCost_Process_CMD
*功能 : 深圳久通机电油位传感器命令处理  命令 G
*备注 : 指标标识 G  设置，查询及探测ID号
*********************************************************************/
u16 JointechOiLCost_Request_CMD_G( STT_JOINTECH_OILCOST_PROTOCOL *pIn , u8 *pOutBuf )
{
  u16 retlen = 0 ;
  u8 iMode = 0;
  u8 id = 1;

  pIn->id = 0x01  ;
  pIn->cmd = 'G' ;
  
  pIn->len = 1+2; 
  pIn->data[ 0 ] = iMode+'0' ; // 0: 表示查询 1: 表示设置
  
  JointechOiLCost_1Hex_To_2ASCII( id , &pIn->data[ 1 ] );
     
  retlen =  JointechOiLCost_Protocol_Packet( pIn , pOutBuf  );

  return retlen ;
  
  
}

/*********************************************************************
*名称 : JointechOiLCost_Process_CMD
*功能 : 深圳久通机电油位传感器命令处理  命令 H
*备注 : 指标标识 H  查询设置阻尼
*********************************************************************/
u16 JointechOiLCost_Request_CMD_H( STT_JOINTECH_OILCOST_PROTOCOL *pIn , u8 *pOutBuf )
{

  u16 retlen = 0 ;
//  u8 iMode = 1;
//  u16 iDamp = 345 ;
//  u8 iByte = 0 ;
  

  pIn->id = 0x01  ;
  pIn->cmd = 'H' ;
  
#if 0
  
  if( 0x00 == iMode  )
  {
    pIn->len = 1  ; 
    pIn->data[ 0 ] = iMode+'0' ; // 0: 表示查询 1: 表示设置
  }
  else
  {
    pIn->len = 1 + 4 ; 
    pIn->data[ 0 ] = iMode+'0' ; // 0: 表示查询 1: 表示设置
  
    iByte = iDamp/100;
    JointechOiLCost_1Dec_To_2ASCII( iByte , &pIn->data[ 1 ] );
 
    iByte = iDamp%100;
    JointechOiLCost_1Dec_To_2ASCII( iByte , &pIn->data[ 3 ] );
    
  }
  
#endif
  
  
  
  retlen =  JointechOiLCost_Protocol_Packet( pIn , pOutBuf  );

  return retlen ;
  
}


/*********************************************************************
*名称 : JointechOiLCost_Process_CMD
*功能 : 深圳久通机电油位传感器命令处理  命令 I
*备注 : 指标标识 I  设置是否开启温飘补偿
*********************************************************************/
u16 JointechOiLCost_Request_CMD_I( STT_JOINTECH_OILCOST_PROTOCOL *pIn , u8 *pOutBuf )
{

  u16 retlen = 0 ;
//  u8 iMode = 0;


  pIn->id = 0x01  ;
  pIn->cmd = 'I' ;
  
  pIn->len = 1 ; 

#if 0 

  pIn->data[ 0 ] = iMode+'0' ; // 0: 表示查询 1: 表示设置为开启 2:表示关闭

#endif
          
  retlen =  JointechOiLCost_Protocol_Packet( pIn , pOutBuf  );

  return retlen ;
  
  
}


/*********************************************************************
*名称 : JointechOiLCost_Process_CMD
*功能 : 深圳久通机电油位传感器命令处理  命令 N
*备注 : 指标标识 N  报错命令
*********************************************************************/
u16 JointechOiLCost_Request_CMD_N( STT_JOINTECH_OILCOST_PROTOCOL *pIn , u8 *pOutBuf )
{
  
  
  u16 retlen = 0 ;
  

  pIn->id = 0x01  ;
  pIn->cmd = 'N' ;
  pIn->len = 0x00 ;
  

  
  retlen =  JointechOiLCost_Protocol_Packet( pIn , pOutBuf  );

  return retlen ;
  
  
}



/*********************************************************************
*名称 : JointechOiLCost_Process_CMD
*功能 : 深圳久通机电油位传感器命令处理  命令 R
*备注 : 指标标识 R  读取关键参数
*********************************************************************/
u16 JointechOiLCost_Request_CMD_R( STT_JOINTECH_OILCOST_PROTOCOL *pIn , u8 *pOutBuf )
{

  u16 retlen = 0 ;
  

  pIn->id =  0x01  ;
  pIn->cmd = 'R' ;
  pIn->len = 0x00 ;
  
  retlen =  JointechOiLCost_Protocol_Packet( pIn , pOutBuf  );

  return retlen ;
  
  
}



/*********************************************************************
*名称 : Task_JointechOiLCost
*功能 : 深圳久通机电油位传感器 任务
*********************************************************************/
FunctionalState Task_JointechOiLCost( )
{

//  static u8 cnt = 0;
   u16 len = 0 ;
   static u8  gJointechBuf[30];
  
   if(!OilWear_Get16PINPortInsertJointechOilFlag())//接入久通机电油位传感器
   {
        return ENABLE;
   }

  Task_JointechOiLCost_Init();
  
 // JointechOiLCost_StateMachine();
  
  LZM_PublicTimerHandler( g_ArrayTimeTask_JointechOil , STAGE_JOINTECHOIL_MAX );
  
  
  
/*********************************以下开启调试时使用 *********************************************/
#if 0
  
  if( cnt++ > 10 )
  {
    cnt = 0 ;
    g_sttJointechComTx.buflen = JointechOiLCost_Request_CMD_E( &g_sttJointechOilCostR , g_sttJointechComTx.buf  );
    JointechOiLCost_COM_Buf_Write( g_sttJointechComTx.buf , g_sttJointechComTx.buflen );

  }

#endif

   /****** 接收处理 *******/
  // g_sttJointechComRx.buflen = COM1_ReadBuff( g_sttJointechComRx.buf , sizeof( g_sttJointechComRx.buf ) );
  
   if( g_sttJointechComRx.buflen )
   {
     
       len = JointechOiLCost_Protocol_Extract( g_sttJointechComRx.buf , g_sttJointechComRx.buflen , gJointechBuf  );
      
      if( len ){

         memset( ( u8 * )&g_sttJointechOilCostA , 0 , sizeof ( g_sttJointechOilCostA ) );
         len = JointechOiLCost_Protocol_Parser( gJointechBuf, len , &g_sttJointechOilCostA );
         
         if( len ){

             g_sttJointechComTx.buflen = JointechOiLCost_Process_CMD( &g_sttJointechOilCostA ,  g_sttJointechComTx.buf );
             
             if( STAGE_JOINTECHOIL_E_OILCURRENT == g_sttJointechCtrl.StageMain )
             {
                 if( 'E' == g_sttJointechOilCostA.cmd )
                 {
                     g_sttJointechCtrl.IsAckOK = 1 ; //收到正确的应答
                 } 
             }
             else if( STAGE_JOINTECHOIL_H_DAMP == g_sttJointechCtrl.StageMain )
             {
                 if( 'H' == g_sttJointechOilCostA.cmd )
                 {
                     g_sttJointechCtrl.IsAckOK = 1 ; //收到正确的应答
                 }              
             }
             else if( STAGE_JOINTECHOIL_I_TEMPERATURE_COMPENSATION == g_sttJointechCtrl.StageMain )
             {
                 if( 'I' == g_sttJointechOilCostA.cmd )
                 {
                     g_sttJointechCtrl.IsAckOK = 1 ; //收到正确的应答
                 }              
             }
             else if( STAGE_JOINTECHOIL_R_CRITICALPARAMETERS == g_sttJointechCtrl.StageMain )
             {
                 if( 'R' == g_sttJointechOilCostA.cmd )
                 {
                     g_sttJointechCtrl.IsAckOK = 1 ; //收到正确的应答
                 }              
             }             
             
             else if( STAGE_JOINTECHOIL_NULL == g_sttJointechCtrl.StageMain )
             {
                g_sttJointechCtrl.IsAckOK = 0 ; //收到正确的应答
             }
         
         }
      }
      
      memset( g_sttJointechComRx.buf, 0 , sizeof( g_sttJointechComRx.buf ) );
  
  }
  
  
  /*
  
  if( JointechOiLCost_Online_Get() )
  {
     Io_WriteAlarmBit( ALARM_BIT_RESERVER30 , RESET );
  }
  else
  {
     Io_WriteAlarmBit( ALARM_BIT_RESERVER30 , SET );    
  }

  */
  
  
  return ENABLE;
  
}


/********/

FunctionalState JointechOiLCost_TimeTask( )
{
//#ifdef JIUTONG_OIL_VERSION,dxl,2014.5.12屏蔽,油耗由32PIN 485接口改到16PIN 232接口
    return  Task_JointechOiLCost();
//#else
    //return DISABLE;
//#endif
}

/*********************************************************************
*名称 : JointechOiLCost_OilVolumeCur_Get
*功能 : 获取油箱内当前剩余油量值，
*备注 : 分辨率: 0.01升/位 ,偏移量: 0
*********************************************************************/
u16 JointechOiLCost_OilVolumeCur_Get( void )
{
  return g_sttJointechVar.OilVolumeCur;
}
  
/*********************************************************************
*名称 : JointechOiLCost_OilVolumeCur_Get
*功能 : 获取油箱内当前液位高度万分比， 最大值为10000，
*备注 : 分辨率: 0.01% ,偏移量: 0
*********************************************************************/
u16 JointechOiLCost_OilLevelPercentCur_Get( void )
{
  return g_sttJointechVar.LiquidLevelPercent;
}

/********************************************************************
* 名称 : JointechOiLCost_CopyBuf
* 功能 : 从串口接收Buf里拷贝数据
* 说明  
********************************************************************/
void  JointechOiLCost_CopyBuf( u8 * pInBuf, u16 InLength )
{
       /*****************2013-09-14 ***********************/
    if( InLength > sizeof( g_sttJointechComRx.buf ) ){
      
      g_sttJointechComRx.buflen = sizeof( g_sttJointechComRx.buf );
      
    }else {
      
      g_sttJointechComRx.buflen = InLength ;
    }
    
    memcpy( g_sttJointechComRx.buf , pInBuf ,  g_sttJointechComRx.buflen );
    

}



/********************************************************************
* 名称 : JointechOiLCost_Error_Set
* 功能 : 久通机电油位传感器故障标志 置位
* 说明  
********************************************************************/
void  JointechOiLCost_Error_Set( void )
{
  g_sttJointechCtrl.ErrorFlag = 1 ;
}


/********************************************************************
* 名称 : JointechOiLCost_Error_Clr
* 功能 : 久通机电油位传感器故障标志 清除
* 说明  
********************************************************************/
void  JointechOiLCost_Error_Clr( void )
{
  g_sttJointechCtrl.ErrorFlag = 0 ;
}

/********************************************************************
* 名称 : JointechOiLCost_Error_Set
* 功能 : 久通机电油位传感器故障标志 置位
* 说明  
********************************************************************/
u8  JointechOiLCost_Error_Get( void )
{
  return g_sttJointechCtrl.ErrorFlag ;
}




/********************************************************************
* 名称 : JointechOiLCost_Error_Set
* 功能 : 久通机电油位传感器 在线
* 说明  
********************************************************************/
void  JointechOiLCost_Online_Set( void )
{
  g_sttJointechCtrl.IsExist = 1 ;
}


/********************************************************************
* 名称 : JointechOiLCost_Error_Clr
* 功能 : 久通机电油位传感器  离线
* 说明  
********************************************************************/
void  JointechOiLCost_Online_Clr( void )
{
  g_sttJointechCtrl.IsExist = 0 ;
}

/********************************************************************
* 名称 : JointechOiLCost_Error_Set
* 功能 : 查询久通机电油位传感器是否 在线
* 说明  
********************************************************************/
u8  JointechOiLCost_Online_Get( void )
{
  return g_sttJointechCtrl.IsExist ;
}




/*********************************************************************
*名称 : JointechOiLCost_StateMachine_E
*功能 : 深圳久通机电油位传感器  主状态机 ; 读当前油位
*备注 : 
*********************************************************************/
void JointechOiLCost_StateMachine_E( )
{
  if( 0 == g_sttJointechCtrl.StageSub )
  {
    
      g_sttJointechComTx.buflen = JointechOiLCost_Request_CMD_E( &g_sttJointechOilCostR , g_sttJointechComTx.buf  );
      JointechOiLCost_COM_Buf_Write( g_sttJointechComTx.buf , g_sttJointechComTx.buflen );

      g_sttJointechCtrl.StageSub = 1;    
      LZM_PublicSetOnceTimer( &g_ArrayTimeTask_JointechOil[ TASK_JOINTECHOIL_MAIN ], PUBLICSECS(9), JointechOiLCost_StateMachine ) ;
      
  }
  else if( g_sttJointechCtrl.ReCheckCnt < 3 )
  {
      if( 1 == g_sttJointechCtrl.IsAckOK )
      {
          g_sttJointechCtrl.IsExist = 1 ;  //置1表明存在
//          Io_WriteAlarmBit( ALARM_BIT_RESERVER30 , RESET );
          JointechOiLCost_StageMain_Set( STAGE_JOINTECHOIL_E_OILCURRENT );
      }
      else
      {
          g_sttJointechCtrl.ReCheckCnt++;
          g_sttJointechCtrl.StageSub = 0 ;
          LZM_PublicSetOnceTimer( &g_ArrayTimeTask_JointechOil[ TASK_JOINTECHOIL_MAIN ], PUBLICSECS(1), JointechOiLCost_StateMachine ) ;
      }

  }
  else
  {
    
    JointechOiLCost_Error_Set();
    
    if( g_sttJointechCtrl.ErrorCnt > 3 )
    {
      g_sttJointechCtrl.IsExist = 0;    //是否存在
//      Io_WriteAlarmBit( ALARM_BIT_RESERVER30 , SET ); 
    }

    JointechOiLCost_StageMain_Set( STAGE_JOINTECHOIL_E_OILCURRENT );
    
  }
  

  
}




/*********************************************************************
*名称 : JointechOiLCost_StateMachine_H
*功能 : 深圳久通机电油位传感器  主状态机 ; 查询/设置阻尼
*********************************************************************/
void JointechOiLCost_StateMachine_H( )
{
  /*
  if( 0 == g_sttJointechCtrl.StageSub )
  {
    
      g_sttJointechComTx.buflen = JointechOiLCost_Request_CMD_H( &g_sttJointechOilCostR , g_sttJointechComTx.buf  );
      JointechOiLCost_COM_Buf_Write( g_sttJointechComTx.buf , g_sttJointechComTx.buflen );

      g_sttJointechCtrl.StageSub = 1;    
      LZM_PublicSetOnceTimer( &g_ArrayTimeTask_JointechOil[ TASK_JOINTECHOIL_MAIN ], PUBLICSECS(6), JointechOiLCost_StateMachine ) ;
      
  }
  else if( g_sttJointechCtrl.ReCheckCnt < 3 )
  {
      if( 1 == g_sttJointechCtrl.IsAckOK )
      {
          g_sttJointechCtrl.IsExist = 1 ;  //置1表明存在
          JointechOiLCost_StageMain_Set( STAGE_JOINTECHOIL_E_OILCURRENT );//再次回到 获取 当前油位值状态
      }
      else
      {
          g_sttJointechCtrl.ReCheckCnt++;
          g_sttJointechCtrl.StageSub = 0 ;
          LZM_PublicSetOnceTimer( &g_ArrayTimeTask_JointechOil[ TASK_JOINTECHOIL_MAIN ], PUBLICSECS(1), JointechOiLCost_StateMachine ) ;
      }

  }
  else
  {
    
    JointechOiLCost_Error_Set();
    
    if( g_sttJointechCtrl.ErrorCnt > 3 )
    {
      g_sttJointechCtrl.IsExist = 0;    //是否存在
    }

    JointechOiLCost_StageMain_Set( STAGE_JOINTECHOIL_E_OILCURRENT );
    
  }
  */
    JointechOiLCost_StageMain_Set( STAGE_JOINTECHOIL_E_OILCURRENT );
}

/*********************************************************************
*名称 : JointechOiLCost_StateMachine_I
*功能 : 深圳久通机电油位传感器  主状态机 ; 设置是否开启温飘补偿
*********************************************************************/
void JointechOiLCost_StateMachine_I( )
{
  /*
  if( 0 == g_sttJointechCtrl.StageSub )
  {
    
      g_sttJointechComTx.buflen = JointechOiLCost_Request_CMD_I( &g_sttJointechOilCostR , g_sttJointechComTx.buf  );
      JointechOiLCost_COM_Buf_Write( g_sttJointechComTx.buf , g_sttJointechComTx.buflen );

      g_sttJointechCtrl.StageSub = 1;    
      LZM_PublicSetOnceTimer( &g_ArrayTimeTask_JointechOil[ TASK_JOINTECHOIL_MAIN ], PUBLICSECS(6), JointechOiLCost_StateMachine ) ;
      
  }
  else if( g_sttJointechCtrl.ReCheckCnt < 3 )
  {
      if( 1 == g_sttJointechCtrl.IsAckOK )
      {
          g_sttJointechCtrl.IsExist = 1 ;  //置1表明存在
        //  JointechOiLCost_StageMain_Set( STAGE_JOINTECHOIL_E_OILCURRENT );//再次回到 获取 当前油位值状态
          JointechOiLCost_Operation_Damp( 1 , DAMP_DEFAULT );
      }
      else
      {
          g_sttJointechCtrl.ReCheckCnt++;
          g_sttJointechCtrl.StageSub = 0 ;
          LZM_PublicSetOnceTimer( &g_ArrayTimeTask_JointechOil[ TASK_JOINTECHOIL_MAIN ], PUBLICSECS(1), JointechOiLCost_StateMachine ) ;
      }

  }
  else
  {
    
    JointechOiLCost_Error_Set();
    
    if( g_sttJointechCtrl.ErrorCnt > 3 )
    {
      g_sttJointechCtrl.IsExist = 0;    //是否存在
    }

    //JointechOiLCost_StageMain_Set( STAGE_JOINTECHOIL_E_OILCURRENT );
    
    JointechOiLCost_Operation_Damp( 1 , DAMP_DEFAULT );
    
  }
  */
  JointechOiLCost_Operation_Damp( 1 , DAMP_DEFAULT );

}



/*********************************************************************
*名称 : JointechOiLCost_StateMachine_R
*功能 : 深圳久通机电油位传感器  主状态机 ; 读关键参数
*备注 : 
*********************************************************************/
void JointechOiLCost_StateMachine_R( )
{
  if( 0 == g_sttJointechCtrl.StageSub )
  {
    
      g_sttJointechComTx.buflen = JointechOiLCost_Request_CMD_R( &g_sttJointechOilCostR , g_sttJointechComTx.buf  );
      JointechOiLCost_COM_Buf_Write( g_sttJointechComTx.buf , g_sttJointechComTx.buflen );

      g_sttJointechCtrl.StageSub = 1;    
      LZM_PublicSetOnceTimer( &g_ArrayTimeTask_JointechOil[ TASK_JOINTECHOIL_MAIN ], PUBLICSECS(6), JointechOiLCost_StateMachine ) ;
      
  }
  else if( g_sttJointechCtrl.ReCheckCnt < 3 )
  {
      if( 1 == g_sttJointechCtrl.IsAckOK )
      {
          g_sttJointechCtrl.IsExist = 1 ;  //置1表明存在
          JointechOiLCost_StageMain_Set( STAGE_JOINTECHOIL_E_OILCURRENT );//再次回到 获取 当前油位值状态
      }
      else
      {
          g_sttJointechCtrl.ReCheckCnt++;
          g_sttJointechCtrl.StageSub = 0 ;
          LZM_PublicSetOnceTimer( &g_ArrayTimeTask_JointechOil[ TASK_JOINTECHOIL_MAIN ], PUBLICSECS(6), JointechOiLCost_StateMachine ) ;
      }

  }
  else
  {
    
    JointechOiLCost_Error_Set();
    
    if( g_sttJointechCtrl.ErrorCnt > 3 )
    {
      g_sttJointechCtrl.IsExist = 0;    //是否存在
    }

    JointechOiLCost_StageMain_Set( STAGE_JOINTECHOIL_E_OILCURRENT );
    
  }
  

  
}


/*********************************************************************
*名称 : Task_JointechOiLCost
*功能 : 深圳久通机电油位传感器 任务
*备注 : 
*********************************************************************/
static void Task_JointechOiLCost_Init( )
{

 static u8 FlagInit = 0 ;

 if( 0 == FlagInit )
 {
   
   FlagInit = 1 ;
   
   //Comm485PowerOn( POWER_STATUS_CARD ); dxl,2014.02.22,未使用到
   
   memset( ( u8 * ) &g_sttJointechOilCostA , 0 , sizeof( STT_JOINTECH_OILCOST_PROTOCOL ) );
   memset( ( u8 * ) &g_sttJointechOilCostR , 0 , sizeof( STT_JOINTECH_OILCOST_PROTOCOL ) );
   
   memset( ( u8 * ) &g_sttJointechComTx , 0 , sizeof( STT_JOINTECH_COM ) );
   memset( ( u8 * ) &g_sttJointechComRx , 0 , sizeof( STT_JOINTECH_COM ) );
   memset( ( u8 * ) &g_sttJointechVar , 0 , sizeof( STT_JOINTECH_OILCOST ) );
   memset( ( u8 * ) &g_sttJointechCtrl , 0 , sizeof( STT_JOINTECH_OILCOST_CTRL ) );
   
   SetTimerTask( TIME_JOINTECHOILCOST , LZM_TIME_BASE );
   LZM_PublicKillTimerAll( g_ArrayTimeTask_JointechOil , TASK_JOINTECHOIL_MAX  );
   
  // JointechOiLCost_StageMain_Set( STAGE_JOINTECHOIL_E_OILCURRENT );
   
  // JointechOiLCost_StageMain_Set( STAGE_JOINTECHOIL_R_CRITICALPARAMETERS );
   
   JointechOiLCost_Operation_TemperatureCompensation( 1 );
   
 }


}


/*********************************************************************
*名称 : JointechOiLCost_StageMain_Set
*功能 : 深圳久通机电油位传感器 状态机 主状态设置
*********************************************************************/
static void JointechOiLCost_StageMain_Set( u8 iStageMain )
{

  g_sttJointechCtrl.StageMain =  iStageMain ; //主状态
  g_sttJointechCtrl.StageSub = 0 ; //次状态
  g_sttJointechCtrl.ReCheckCnt = 0 ; //重发计数器
  g_sttJointechCtrl.IsAckOK = 0 ;   //是否收到正确的应答
  

  if( STAGE_JOINTECHOIL_E_OILCURRENT == iStageMain )
  {
    LZM_PublicSetOnceTimer( &g_ArrayTimeTask_JointechOil[ TASK_JOINTECHOIL_MAIN ], PUBLICSECS(1), JointechOiLCost_StateMachine ) ;
  }
  else if( STAGE_JOINTECHOIL_H_DAMP == iStageMain )
  {
    LZM_PublicSetOnceTimer( &g_ArrayTimeTask_JointechOil[ TASK_JOINTECHOIL_MAIN ], PUBLICSECS(1), JointechOiLCost_StateMachine ) ;
  } 
  else if( STAGE_JOINTECHOIL_I_TEMPERATURE_COMPENSATION == iStageMain )
  {
    LZM_PublicSetOnceTimer( &g_ArrayTimeTask_JointechOil[ TASK_JOINTECHOIL_MAIN ], PUBLICSECS(1), JointechOiLCost_StateMachine ) ;
  } 
  else if( STAGE_JOINTECHOIL_R_CRITICALPARAMETERS == iStageMain )
  {
    LZM_PublicSetOnceTimer( &g_ArrayTimeTask_JointechOil[ TASK_JOINTECHOIL_MAIN ], PUBLICSECS(1), JointechOiLCost_StateMachine ) ;
  }  
  else if( STAGE_JOINTECHOIL_NULL == iStageMain )
  {
      LZM_PublicKillTimer( &g_ArrayTimeTask_JointechOil[ TASK_JOINTECHOIL_MAX ] );
   //     LZM_PublicKillTimer( &s_stLEDTimer[LED_TIMER_CHECK]);
  }

}


/*********************************************************************
*名称 : JointechOiLCost_StateMachine
*功能 : 深圳久通机电油位传感器  主状态机
*备注 : 
*********************************************************************/
void JointechOiLCost_StateMachine( )
{

  switch( g_sttJointechCtrl.StageMain )
  {
      case STAGE_JOINTECHOIL_E_OILCURRENT:
      JointechOiLCost_StateMachine_E();
      break;

      case STAGE_JOINTECHOIL_H_DAMP:
      JointechOiLCost_StateMachine_H();
      break;       

      case STAGE_JOINTECHOIL_I_TEMPERATURE_COMPENSATION:
      JointechOiLCost_StateMachine_I();
      break;    
    
      case STAGE_JOINTECHOIL_R_CRITICALPARAMETERS:
      JointechOiLCost_StateMachine_R();
      break;   
      
      
      
      
  
      default:
      break;

  }
  
}




/*********************************************************************
*名称 : JointechOiLCost_StateMachine_I
*功能 : 深圳久通机电油位传感器  主状态机 ; 查询/设置 阻尼
*备注 : 
*********************************************************************/
void JointechOiLCost_Operation_Damp( u8 InOpCode , u16 InDamp )
{
  u8 iByte = 0 ;
  
  if( InOpCode > 1 )
  {
      InOpCode = 0 ;
  }
  
  if( InDamp < 5 )
  {
    InDamp = 5 ;
  }
  
  if( InDamp > 600 )
  {
    InDamp = 600 ;
  }
  

  g_sttJointechOilCostR.data[ 0 ] = InOpCode + '0' ; 

  iByte = InDamp/100;
  JointechOiLCost_1Dec_To_2ASCII( iByte , &g_sttJointechOilCostR.data[ 1 ] );
  
  iByte = InDamp%100;
  JointechOiLCost_1Dec_To_2ASCII( iByte , &g_sttJointechOilCostR.data[ 3 ]  );
  
  
  g_sttJointechOilCostR.len = 5 ; 

  /* 设置主状态 为 温飘补偿操作状态 */
  JointechOiLCost_StageMain_Set( STAGE_JOINTECHOIL_H_DAMP );
}




/*********************************************************************
*名称 : JointechOiLCost_StateMachine_I
*功能 : 深圳久通机电油位传感器  主状态机 ; 设置是否开启温飘补偿
*备注 : 
*********************************************************************/
void JointechOiLCost_Operation_TemperatureCompensation( u8 InOpCode )
{
  
  if( InOpCode > 2 )
  {
      InOpCode = 0 ;
  }
  
  g_sttJointechOilCostR.data[ 0 ] = InOpCode + '0' ; 
  
  /* 设置主状态 为 温飘补偿操作状态 */
  JointechOiLCost_StageMain_Set( STAGE_JOINTECHOIL_I_TEMPERATURE_COMPENSATION );
}
/*********************************************************************
//函数名称	:JointechOiLCost_OilProtocolCheck
//功能		:久通油耗协议检查,正确的话会调用函数JointechOiLCost_CopyBuf,
//          :把油耗指令拷贝到g_sttJointechComRx.buf中，然后解析
//备注		:,dxl,2014.5.12增加,油耗由32PIN 485接口改到16PIN 232接口
//          :返回值为1表示已判定为油耗指令，为0表示不是油耗指令
*********************************************************************/
u8 JointechOiLCost_OilProtocolCheck(u8 *pBuffer, u16 BufferLen)
{
    u16 i = 0;
    u16 j = 0;
    u16 k = 0;
    u8  flag = 0;
    u8  HighByte = 0;
    u8  LowByte = 0;
    u8  sum = 0;
    u8  *p = NULL;
    
    if(!OilWear_Get16PINPortInsertJointechOilFlag())return 0;//bit2:未接入久通机电油位传感器,lzm,2014.9.24
    
    if((NULL != p)||(BufferLen >= 100))
    {
        return 0;
    }
    p = pBuffer;
    for(i=0; i<BufferLen; i++)
	{
		if('@' == *(p+i))
		{
				break;
		}
	}
    for(j=i; j<BufferLen; j++)
	{
        if('#' == *(p+j))
		{
            flag = 1;
            k = j+1;//长度
		    break;
		}
	}
	if((1 == flag)&&(k > 3))
    {
        sum = 0;
        for(j=i+1;j<k-3;j++)
        {
            sum += *(p+j);
        }
        
        HighByte = *(p+k-3);
        if((HighByte >= '0')&&(HighByte <= '9'))
        {
            HighByte -= '0';
        }
        else if((HighByte >= 'A')&&(HighByte <= 'F'))
        {
            HighByte -= 55;
        }
        else if((HighByte >= 'a')&&(HighByte <= 'f'))
        {
            HighByte -= 87;
        }
        else
        {
            flag = 0;//非法字符
        }
            
        LowByte = *(p+k-2);
        if((LowByte >= '0')&&(LowByte <= '9'))
        {
            LowByte -= '0';
        }
        else if((LowByte >= 'A')&&(LowByte <= 'F'))
        {
            LowByte -= 55;
        }
        else if((LowByte >= 'a')&&(LowByte <= 'f'))
        {
            LowByte -= 87;
        }
        else
        {
            flag = 0;//非法字符
        }
            
        if((sum == ((HighByte<<4)|LowByte))&&(1 == flag))
        {
            //发送到平台
            if((Uart1DataDownTransCmd == *(p+i+3))&&(1 == Uart1DataDownTransFlag))
            {
                Uart1DataDownTransFlag = 0;
                DelayBuffer[0] = 1;
                memcpy(DelayBuffer+1,p+i,k-i);
                DelayBufferLen = k-i+1;
                DelayAckCommand = 13;//下一步把应答发送到平台
                SetTimerTask(TIME_DELAY_TRIG, 1*SECOND);//触发延时任务
            }
            JointechOiLCost_CopyBuf(p+i,k-i); 
            
        }
        else
        {
            flag = 0;
        }
    }
    return flag;
	
}
/*******************************************************************************
** 函数名称: JointechOiLCost_GetSubjoinInfoCurOilVolume
** 功能描述: 取得当前油量附加信息包(包括附加信息ID、长度、附加信息体)
** 入口参数: 需要保存油量附加信息包的首地址
** 出口参数: 
** 返回参数: 油量附加信息包的总长度
*******************************************************************************/
unsigned char JointechOiLCost_GetSubjoinInfoCurOilVolume(unsigned char *data)
{
    unsigned char buffer[10]={0};
    unsigned char len;
    unsigned short usVal;
    if(!OilWear_Get16PINPortInsertJointechOilFlag())return 0;//未接入久通机电油位传感器
    len = 0;
    usVal = JointechOiLCost_OilVolumeCur_Get()/10;
    buffer[len++] = 0x02;//附加信息ID
    buffer[len++] = 2;   //附加长度
    Public_ConvertShortToBuffer(usVal,&buffer[len]);//附加信息体
    //////////////////////////
    len += 2;
    ///////////////
    memcpy(data,buffer,len);
    /////////////////
    return len;
}
/*******************************************************************************
** 函数名称: JointechOiLCost_GetSubjoinInfoCurOilLevelPercent
** 功能描述: 取得当前油量百分比附加信息包(包括附加信息ID、长度、附加信息体)
** 入口参数: 需要保存油量百分比附加信息包的首地址
** 出口参数: 
** 返回参数: 油量百分比附加信息包的总长度
*******************************************************************************/
unsigned char JointechOiLCost_GetSubjoinInfoCurOilLevelPercent(unsigned char *data)
{
    unsigned char buffer[10]={0};
    unsigned char len;
    unsigned short usVal;
    if(!OilWear_Get16PINPortInsertJointechOilFlag())return 0;//未接入久通机电油位传感器
    len = 0;
    usVal = JointechOiLCost_OilLevelPercentCur_Get();
    buffer[len++] = 0xE4;//附加信息ID
    buffer[len++] = 2;   //附加长度
    Public_ConvertShortToBuffer(usVal,&buffer[len]);//附加信息体
    //////////////////////////
    len += 2;
    ///////////////
    memcpy(data,buffer,len);
    /////////////////
    return len;
}
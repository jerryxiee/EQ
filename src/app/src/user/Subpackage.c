/********************************************************************
//��Ȩ˵��	:Shenzhen E-EYE Co. Ltd., Copyright 2009- All Rights Reserved.
//�ļ�����	:Subpacket.c		
//����		:ʵ�ְַ��������ܣ���Ӧƽ̨����0x8003��
//�汾��	:
//������	:dxl  
//����ʱ��	:2013.7
//==============================================================================
* �޸���   : Shigle
* �޸�ʱ�� : 2013��11��18��
* 2013��11��18��Ϊ���߰汾0.1 �������������������޸ģ���Ҫ������׷��˵��
* 1�����̼����������˶�ý�����ݲ������� ����������Subpackage_Task_Media()���ԡ�
* 2����ý�����ݲ����������¼���:
          A.��ý�����ݲ����Ĵ���Դ��RadioProtocol.c�ļ��ĺ���RadioProtocol_MultiMediaUploadAck�
          B.���ն���ƽ̨�����ϴ���ý������ʱ������������������Ҫ�ְ����䡣
            �ն���һ���������ְ�����ƽ̨��ƽ̨����Ҫ��Խ��յ�ÿ���ն˰�Ӧ��ƽֻ̨���ն��ϴ������һ��Ӧ��
            ƽ̨������ն˵�һ���������İ�����ͳ�ƺ����϶�ý������.
            B1.�������ȷ���գ���ƴ�ϳɹ�����ƽ̨�·���"��ý�������ϴ�Ӧ��"�����У�"��ý��ID"�ֶ��޺������ݡ�
            B2.���ƽ̨��鷢�ֽ��յİ�����©�����߲�������У��Ͳ����ȴ�����ƽ̨�·���"��ý�������ϴ�Ӧ��"�����У�
               "��ý��ID"�ֶθ�����Ҫ��������Ϣ����Ҫ�������ֶ�"�ش�������"��"�ش���ID�б�"���ն˽���ʱ����Դ��ý���ļ���
               ��λ���貹��������λ�ã���ȡ��Ӧ�����ݣ��ؽ�����"��ý�������ϴ�"�������ϴ���ƽ̨�� �����;���������ط�����(��
               ÿ�������ط�3��),����κ�һ������4�η���ʧ�ܣ����������ζ�ý�岹������
*  3���ⲿ������ý�����ݲ�����Ҫ���õĺ�����2�����ֱ���:
           A1.RadioProtocol.c�ļ��ĺ���RadioProtocol_MultiMediaUploadAck()��Ѿ����
           A2.��Ҫ�ڵ���RadioProtocol_MultiMediaDataReport()���ļ������Subpackage_Media_File_Name_Set()�������������񿽱���ý���ļ�����
*  4��������Ӱ��ԭ��ʻ��¼�ǲ������롣



***********************************************************************/
//***************�����ļ�*****************
#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "include.h"

//****************�ⲿ����****************
extern u16	RecorderSerialNum;//��ʻ��¼��Ӧ����ˮ��
extern u8	RecorderCmd;//��ʻ��¼������

//****************ȫ�ֱ�������****************

u16  SubpackageList[SUBPACKET_LIST_BUFFER_SIZE];//�ְ��б�,��ƽ̨����Ӧ���л��
u16  SubpackageResendFirstSerial;//�ְ��ش���1������ˮ��
u16  SubpackageCurrentSerial;//�ְ�������ǰ����ˮ��
u8   SubpacketUploadFlag;//�ְ��ϴ���־,1Ϊ�����ϴ�,0Ϊ�����ϴ�


STT_SUBPACKETTRANSTER_CTRL g_sttSubPacketTransterCtrl ;
/*
�ְ���������,
1:��ʻ�ٶ�
2:λ����Ϣ��¼;
3:�¹��ɵ��¼;
4:��ʱ��ʻ��¼;
5:��ʻ����ݼ�¼;
6:�ⲿ�����¼;
7:�����޸ļ�¼;
8:�ٶ�״̬��־
*/
static u8  SubpacketUploadType = 0;

/* �ְ����ܰ��� */
u16 SubpackageTotalPacket = 0 ;




u32 Subpackage_Get_Media_Attribute_From_Memory( FIL *pInfile ,  u8 *pOutBuf   );
u32 Subpackage_Get_Media_Data_From_Memory( FIL *pInfile , u16 InPackIndex , u8 *pOutBuf   );

/*********************************************************************
//��������	:Subpackage_UploadTimeTask(void)
//����		:�ְ�������ʱ����
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
  u8 channel = CHANNEL_DATA_1;//Ϊ����ͨ�������
  
  if(  ( SubpacketUploadType > 0 )&&( SubpacketUploadType < REGISTER_TYPE_MAX ))//��ʻ��¼�����ݷְ�����
  {
    
   p = Buffer;
   BufferLen = 0 ;
  

  for( i = 0 ; i < SUBPACKET_LIST_BUFFER_SIZE ; i++ )
  {
      if( 0 != SubpackageList[i] )
      { /* ��0��ʼ���ҵ���һ����Ч����Ҫ�����ְ� , ��ֱ������ѭ�� */
        break;
      }
  }
        
        
  if(SUBPACKET_LIST_BUFFER_SIZE == i)
  {/* ����Ѿ�û���ҵ��κ� ��Ч����Ҫ�����ְ� ���򽫷ְ��ϴ���־,��0�����������ϴ� ������������ ,Ȼ���˳�ѭ��*/
      SubpacketUploadFlag = 0;
      return DISABLE;//û���ش��ְ���
  }

  SubpacketUploadFlag = 1 ; //�򽫷ְ��ϴ���־, ��1���������ϴ�������........
  SubpackageCurrentSerial = SubpackageResendFirstSerial+SubpackageList[i]-1;

  if( 1 == SubpackageList[i] )//��1��
  {
      //Ӧ����ˮ��
       *p++ = (RecorderSerialNum&0xff00) >> 8;
       *p++ = RecorderSerialNum&0xff;
        //������
       *p++ = RecorderCmd;
  }
      
      //��ʼ��ͷ0X55��0X7A
      *p++ = 0x55;
      *p++ = 0x7A;
      BufferLen += 2;
      
      //������
      *p++ = RecorderCmd ;
      BufferLen += 1 ;
      
      //���ݿ鳤��,���ֽ���ǰ
      q = p;
      *p++ = 0;
      *p++ = 0;
      BufferLen += 2;
       //�����ֽ�
      *p++ = 0;
      BufferLen += 1;
       //���ݿ飬��ȡ������
      length = Register_GetSubpacketData( p , SubpacketUploadType ,  SubpackageList[i]  );
      *q++ = ( length&0xff00 ) >> 8;
      *q++ = length&0xff;
      BufferLen += length ;
      //У���ֽ�
      CheckByte = 0;
      for( j=0; j<length; j++)
      {
          CheckByte = ( CheckByte ^ *(p+j)  );
      }
      p += length ;
      *p++ = CheckByte ;
      BufferLen++;
      
      //���ͣ�ʧ�ܻ��ط�20�Σ�20����������ʧ����ֹͣ�ϴ� 
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
  else//�����ķְ�������û����
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
//�������� : Subpackage_Media_File_ID_IsEmpty(void)
//�������� : ����Ƿ��д�������ID�� , 
*********************************************************************/
u8 Subpackage_Media_File_ID_IsEmpty( void  )
{
  return (( g_sttSubPacketTransterCtrl.PacketIDCntCursor == g_sttSubPacketTransterCtrl.PacketIDCntTotal )? 1:0 );
}


/*********************************************************************
//�������� : Subpackage_Media_File_ID_Inc(void)
//�������� : �ƶ���������ID������ָ�룬ȡ��һ����������ID�� , 
*********************************************************************/
u8 Subpackage_Media_File_ID_Inc( void  )
{

  u8 ret = 0 ;
  /* ����������ID�� ����Ϊ0 ��������һ�� */
  if( g_sttSubPacketTransterCtrl.PacketIDCntTotal < 1 )
  {
    return ret ; 
  }
  
  g_sttSubPacketTransterCtrl.PacketIDCntCursor++ ;
  
  /* ��0�� g_sttSubPacketTransterCtrl.PacketIDCntTotal-1 ,*/
  if( g_sttSubPacketTransterCtrl.PacketIDCntCursor >= g_sttSubPacketTransterCtrl.PacketIDCntTotal )
  {
      for( u8 i = 0 ; i < g_sttSubPacketTransterCtrl.PacketIDCntTotal ; i++ )
      {
         g_sttSubPacketTransterCtrl.PacketIDArray[i] = 0 ;
      }
      
      /* !!! ע�⾭����䲻��ǰ�� */
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
//�������� : Subpackage_Media_File_Name_Set
//�������� : ����ý��ְ����������񣬿�����Ҫ�������ļ����� ����ļ�����������ĳ��ȣ�����0�����򷵻�ʵ�ʵ����õ��ַ������ȣ�����β�ֽ�'\0')
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
//�������� : Subpackage_Open_Media_File_From_Memory
//�������� : �����ļ��������ļ�������ø��ļ��Ŀ��ƾ������������ְ���ͳ���ܰ���
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
//�������� : Subpackage_Media_File_Close
//�������� : �ر��ļ�
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
//�������� : Subpackage_Media_File_Buf_Attribute_Get
//�������� : ͨ���ļ����ƾ������ȡ�ļ�36�ֽ�������Ϣ
*********************************************************************/
u32 Subpackage_Media_File_Buf_Attribute_Get( FIL *pInfile ,  u8 *pOutBuf   )
{
  
 u32 Byte = 0 ;
 /********************************************************/
  /* ��ڲ��� ���*/
  if( ( NULL == pInfile ) || ( NULL == pOutBuf ) )
  {
    return Byte; 
  }
  
  
  if( pInfile->fsize < 36 )
  {
    return Byte; 
  }
   
  /* �ƶ��ļ���ƫ��ָ�� */
  if( FR_OK != f_lseek( pInfile , pInfile->fsize-36 ) )//�ƶ��ļ�ָ�������ʼ��(���ļ�ĩβ36�ֽ�Ϊ������Ϣ
  {
    return Byte;
  }
  
  /* ��ȡ�ļ������� */
  if( FR_OK == f_read ( pInfile, pOutBuf, 36, &Byte) )
  {
    f_lseek( pInfile , 0);
  }

  /* ����ʵ�ʵ��ֽ��� */
  return Byte ; 

}



/*********************************************************************
//�������� : Subpackage_Media_File_Buf_Data_Get
//�������� :  ͨ���ļ����ƾ�����Լ��ļ��ڵİ���ţ���ȡ�ļ�512�ֽ��ļ��ֽ���
*********************************************************************/
u32  Subpackage_Media_File_Buf_Data_Get( FIL *pInfile , u16 InPackIndex , u8 *pOutBuf   )
{

   u32 Byte = 0 ;

   
 /********************************************************/
  /* ��ڲ��� ���*/
  if( ( NULL == pInfile ) || ( NULL == pOutBuf ) )
  {
    return Byte; 
  }
  
  /* �ƶ��ļ���ƫ��ָ�� */
  if( FR_OK != f_lseek( pInfile , 0 ) )//�ƶ��ļ�ָ�������ʼ��(���ļ�ĩβ36�ֽ�Ϊ������Ϣ
  {
    return Byte;
  }


  /* �ƶ��ļ���ƫ��ָ�� */
  if( FR_OK != f_lseek( pInfile , InPackIndex*512 ) )//�ƶ��ļ�ָ�������ʼ��(���ļ�ĩβ36�ֽ�Ϊ������Ϣ
  {
    return Byte ;
  }


  /* ��ȡ�ļ������� */
  if( FR_OK == f_read ( pInfile , pOutBuf , 512 , &Byte ) )
  {
    f_lseek( pInfile , 0);
  }

  /* ����ʵ�ʵ��ֽ��� */
  return Byte ; 

}


/*********************************************************************
//�������� : Subpackage_Media_File_Packet_Get
//�������� : ͨ���ļ����ƾ�����Լ��ļ��ڵİ���ţ���ȡ36�ֽ��ļ����� �� �ļ�512�ֽ��ļ��ֽ���
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
      /* �滻 ��ý��ID */
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
//�������� : Subpackage_Task_Media(void)
//�������� : ��ý��ְ��������񣬺����ļ���ȡ�����ر��ļ���������ݣ���ƽ̨�ϴ����ݣ��Լ�������һ�ְ�ID 
*********************************************************************/
void Subpackage_Task_Media( void )
{
   u8 channel = CHANNEL_DATA_1;//Ϊ����ͨ�������
  
  /* ���ȣ�����Ƿ��д�������ID , ���Ϊ�գ���ֱ���˳� */
  if( Subpackage_Media_File_ID_IsEmpty() )
  {
    return ;    
  }
  
  
 if( 0 == g_sttSubPacketTransterCtrl.ResendCnt )
 {
  // g_sttSubPacketTransterCtrl.PacketCntCursor = 1;
   
   
  /* ��ȡ�ļ��� */
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
  
  /* ���ļ�, ��ȡ�ļ����  */
  if( 0 == Subpackage_Media_File_Open( (char *)g_sttSubPacketTransterCtrl.FileName , &g_sttSubPacketTransterCtrl.FileHandler ) )
  {
      return ; 
  }
  /* ��ȡ�ļ����� */
  g_sttSubPacketTransterCtrl.DataBufLength = Subpackage_Media_File_Buf_Packet_Get( &g_sttSubPacketTransterCtrl.FileHandler , g_sttSubPacketTransterCtrl.PacketCntCursor , g_sttSubPacketTransterCtrl.DataBuf  );

  /* �ر��ļ� */
  Subpackage_Media_File_Close( &g_sttSubPacketTransterCtrl.FileHandler  );

  /* �жϣ���ȡ�����ݳ����Ƿ���Ч�� �����Ч����ֱ�ӷ����˳� */
  if( 0 == g_sttSubPacketTransterCtrl.DataBufLength )
  {
     return ;
  }

   g_sttSubPacketTransterCtrl.ResendCnt = 1;
  
 }
 else if( g_sttSubPacketTransterCtrl.ResendCnt < 3 ) //�ط�3��  1, 2 ,��ʱ����Ҫ���´�SD����ȡ����
 {
    g_sttSubPacketTransterCtrl.ResendCnt++;
 }
 else //��3�Σ�������ֹͣ����
 {
     g_sttSubPacketTransterCtrl.MediaType = 0 ;    
 }
  
  /* ͨ������ƽ̨�������� */
  if(  ACK_OK == RadioProtocol_MultiMediaDataReport( channel,
                                                     g_sttSubPacketTransterCtrl.DataBuf , 
                                                     g_sttSubPacketTransterCtrl.DataBufLength ,
                                                     g_sttSubPacketTransterCtrl.PacketCntTotal ,
                                                     g_sttSubPacketTransterCtrl.PacketCntCursor )
  )
  {
    
    /* ��� "��ý�������ϴ�0x0801" �ɹ������ƶ�����һ������ID */
    Subpackage_Media_File_ID_Inc();
    g_sttSubPacketTransterCtrl.ResendCnt = 0 ;//���ͳɹ����ط�����Ф��

  }
  else
  {
     //�����ְ���������
    //����ʧ�ܺ�3����ٴεô���
     SetTimerTask( TIME_SUBPACKAGE, 3*SECOND ); 
  }

  


}

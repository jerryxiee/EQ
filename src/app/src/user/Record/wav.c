/**
  ******************************************************************************
  * @file    I2S_Record.c 
  * @author  
  * @version 
  * @date    
  * @brief   I2S��DMA����
  *               
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "wav.h"
#include "include.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
FIL wav_fil;            /* ����wav�ļ��ṹָ��,ȷ��ÿ��ֻ�ܲ���һ��wav�ļ�*/
u8  WAV_OPEN_FLAG = 0;  /* WAV�ļ��򿪱�־,0-�ر�,1-��*/
u8   posBuf[50];
u8   posBufLen = 0;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  ����wav��Ƶ�ļ�ͷ
  * @note      
  * @param path:wav�ļ���
  * @param mode:����ģʽ���ļ���ģʽ
  * @param pfmt_block:fmt�ṹ����ָ��
  * @retval ����0�ɹ�����0ʧ��
  */
int Wav_Create_Head(	
	const char *path,
	BYTE mode,
        struct FMT_BLOCK *pfmt_block
)
{
	FRESULT res = FR_OK;
	
	UINT bytes_ret = 0;
        UCHAR headbuff[100];	              //wavͷ���ݻ�����
        
        UINT file_pos = 0;
        UINT str_size = 0;
        struct RIFF_HEADER    riff_header;
        char   fack_block_buffer[20];        //20byte��ȡfact������Ӧ���ǿ��Եģ�����г������ڵ���
        struct FACT_BLOCK    fact_block;
        struct DATA_BLOCK    data_block;        

        memset((void*)&wav_fil,0,sizeof(wav_fil));
        
        memset((void*)&riff_header,0,sizeof(riff_header));
        memset((void*)&fact_block,0,sizeof(fact_block));
        memset((void*)&data_block,0,sizeof(data_block));
        memset((void*)&fack_block_buffer,0,sizeof(fack_block_buffer));
        
        if(1 == WAV_OPEN_FLAG)             //���wav�ļ��Ѵ�,��ر����´�
        {
          WAV_OPEN_FLAG = 0;
          f_close(&wav_fil);
        }
 	res = f_open(&wav_fil, ff_NameConver((u8 *)path), mode);
	if(res == FR_OK)
	{
                //���濪ʼ¼��GPS��ַ���¼�,���ڹر�����ӵ��ļ�ĩβ  
                memset(posBuf,0,sizeof(posBuf));
                //��ý��ID(4�ֽ�)
                posBufLen = 4;         //��ͷ��ʼ,ͷ4���ֽڱ����ý��ID
                //��ý������(1�ֽ�),
                posBuf[posBufLen++] = 1;//��Ƶ
                //��ý���ʽ����(1�ֽ�)
                posBuf[posBufLen++] = 3;//wav�����ʽ
                //�¼�����(1�ֽ�),
                posBuf[posBufLen++] = 0;//�̶�Ϊ0
                //ͨ��ID(1�ֽ�),
                posBuf[posBufLen++] = 0;//�ȹ̶�Ϊ0
                Report_GetPositionBasicInfo(&posBuf[posBufLen]);
                posBufLen += 28;          
               
                //����RIFF ��
                str_size = sizeof(struct RIFF_HEADER);
                memcpy(riff_header.szRiffID,RIFF_ID,4);
                memcpy(riff_header.szRiffFormat,WAVE_STR,4);
	        riff_header.dwRiffSize =  RIFF_EXC_LEN;                         //ͷ�ļ�����58 - 8(RIFF_ID��WAVE_STR)    
                memcpy(&headbuff[file_pos],&riff_header,str_size);
                file_pos += str_size;
                
                //����FMT ��
                str_size = sizeof(struct FMT_BLOCK) - 2;  
                memcpy(&headbuff[file_pos],pfmt_block,str_size);
                file_pos += str_size;
                
                //����FACT��
                str_size = sizeof(struct FACT_BLOCK);
                memcpy(fact_block.szFactID,FACT_ID,4);
                fact_block.dwFactSize = 4;
                memcpy(&headbuff[file_pos],&fact_block,str_size);
                file_pos += str_size;
                memcpy(&headbuff[file_pos],fack_block_buffer,fact_block.dwFactSize);
                file_pos += fact_block.dwFactSize;
                
                //����DATA��
                str_size = sizeof(struct DATA_BLOCK);
                memcpy(data_block.szDataID,DATA_ID,4);
                data_block.dwDataSize = 0;                                      //����ͷ,���ݳ���������Ϊ0
                memcpy(&headbuff[file_pos],&data_block,str_size);
                file_pos += str_size;
                res = f_write(&wav_fil, headbuff, file_pos, &bytes_ret);
                if(res != FR_OK)
                {
                   f_close(&wav_fil);
                   return 1;
                }
                WAV_OPEN_FLAG = 1;                                              //��־�Ѵ�
                return 0;
	}
	f_close(&wav_fil);
	return 1;
}

/**
  * @brief  ����wav_G711�����ʽ��Ƶ�ļ�ͷ
  * @note      
  * @param  path: wav�ļ���
  * @param  mode: ����ģʽ���ļ���ģʽ
  * @retval ����0�ɹ�����0ʧ��
  */
int Wav_Write_G711(	
	const char *path,
	BYTE mode
)
{
  struct FMT_BLOCK      fmt_block;
  memset((void*)&fmt_block,0,sizeof(fmt_block));
  memcpy(fmt_block.szFmtID,FORMAT_ID,4);
  fmt_block.dwFmtSize = 18;
  fmt_block.wFormatTag = WAVE_FORMAT_ALAW;       //������Ƶ���뷽ʽ:(0x01-PCM,0x06-A-law,0x07-u-law)
  fmt_block.wChannels = 1;                       //������/������
  fmt_block.dwSamplesPerSec  = 8000;             //8Khz����Ƶ��
  fmt_block.wBitsPerSample   = 8 ;               //ÿ��������Ҫ16bit
  fmt_block.wBlockAlign      = fmt_block.wChannels*(fmt_block.wBitsPerSample/8) ;    //ÿ��������Ҫ�ֽ���,wChannels*(wBitsPerSample/8) =4
  fmt_block.dwAvgBytesPerSec = fmt_block.dwSamplesPerSec * fmt_block.wBlockAlign  ;  //ÿ�������ֽ���,dwSamplesPerSec*wBlockAlign
  return Wav_Create_Head(path,mode,&fmt_block);
}

/**
  * @brief  д��G711��Ƶ���ݵ��ļ���
  * @note      
  * @param  buff: wav��������
  * @param  nobtw: wav�������ݳ���
  * @retval ����0�ɹ�����0ʧ��
  */
int Wav_Write_Data(	
	const void *buff,
	UINT nobtw
)
{
  FRESULT res = FR_OK;
  UINT bytes_ret = 0;
  if(1 == WAV_OPEN_FLAG)
  {
    res = f_write(&wav_fil, buff, nobtw, &bytes_ret);
    if(res != FR_OK)
    {
      return res;
    }
    return 0;    
  }
  return 1;
}

/**
  * @brief  �رմ򿪵�WAV��ʽ�ļ�
  * @note      
  * @param  nobtw: wav�������ݳ���
  * @param  RecordID: ��ý��ID
  * @retval ����0�ɹ�����0ʧ��
  */
int Wav_Close(UINT nobtw,UINT RecordID)
{
  FRESULT res = FR_OK;
  UINT bytes_ret = 0;
  UINT utrifflen = 0 ;
  if(nobtw <= 0)
  {
    return 1;
  }
  if(1 == WAV_OPEN_FLAG)
  {
    posBuf[0] = RecordID >> 24;
    posBuf[1] = RecordID >> 16;
    posBuf[2] = RecordID >> 8;
    posBuf[3] = RecordID;    
    res = f_write(&wav_fil, posBuf, posBufLen, &bytes_ret);
    utrifflen = nobtw + RIFF_EXC_LEN;
    f_lseek(&wav_fil,RIFF_SIZE_POS);
    res = f_write(&wav_fil, (void*)&utrifflen, 4, &bytes_ret);
    f_lseek(&wav_fil,DATA_SIZE_POS);
    res = f_write(&wav_fil, (void*)&nobtw, 4, &bytes_ret);  
   
    f_sync(&wav_fil);
    WAV_OPEN_FLAG = 0;
    f_close(&wav_fil);
    if(res != FR_OK)
    {
      return res;
    }
    return 0;
  }
  return 1;
}

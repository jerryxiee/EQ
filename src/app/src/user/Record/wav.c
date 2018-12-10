/**
  ******************************************************************************
  * @file    I2S_Record.c 
  * @author  
  * @version 
  * @date    
  * @brief   I2S和DMA配置
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
FIL wav_fil;            /* 保存wav文件结构指针,确认每次只能操作一个wav文件*/
u8  WAV_OPEN_FLAG = 0;  /* WAV文件打开标志,0-关闭,1-打开*/
u8   posBuf[50];
u8   posBufLen = 0;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  创建wav音频文件头
  * @note      
  * @param path:wav文件名
  * @param mode:访问模式和文件打开模式
  * @param pfmt_block:fmt结构对象指针
  * @retval 返回0成功，非0失败
  */
int Wav_Create_Head(	
	const char *path,
	BYTE mode,
        struct FMT_BLOCK *pfmt_block
)
{
	FRESULT res = FR_OK;
	
	UINT bytes_ret = 0;
        UCHAR headbuff[100];	              //wav头内容缓冲区
        
        UINT file_pos = 0;
        UINT str_size = 0;
        struct RIFF_HEADER    riff_header;
        char   fack_block_buffer[20];        //20byte读取fact的数据应该是可以的，如果有超过的在调整
        struct FACT_BLOCK    fact_block;
        struct DATA_BLOCK    data_block;        

        memset((void*)&wav_fil,0,sizeof(wav_fil));
        
        memset((void*)&riff_header,0,sizeof(riff_header));
        memset((void*)&fact_block,0,sizeof(fact_block));
        memset((void*)&data_block,0,sizeof(data_block));
        memset((void*)&fack_block_buffer,0,sizeof(fack_block_buffer));
        
        if(1 == WAV_OPEN_FLAG)             //如果wav文件已打开,则关闭重新打开
        {
          WAV_OPEN_FLAG = 0;
          f_close(&wav_fil);
        }
 	res = f_open(&wav_fil, ff_NameConver((u8 *)path), mode);
	if(res == FR_OK)
	{
                //保存开始录音GPS地址和事件,用于关闭是添加到文件末尾  
                memset(posBuf,0,sizeof(posBuf));
                //多媒体ID(4字节)
                posBufLen = 4;         //从头开始,头4个字节保存多媒体ID
                //多媒体类型(1字节),
                posBuf[posBufLen++] = 1;//音频
                //多媒体格式编码(1字节)
                posBuf[posBufLen++] = 3;//wav编码格式
                //事件类型(1字节),
                posBuf[posBufLen++] = 0;//固定为0
                //通道ID(1字节),
                posBuf[posBufLen++] = 0;//先固定为0
                Report_GetPositionBasicInfo(&posBuf[posBufLen]);
                posBufLen += 28;          
               
                //设置RIFF 块
                str_size = sizeof(struct RIFF_HEADER);
                memcpy(riff_header.szRiffID,RIFF_ID,4);
                memcpy(riff_header.szRiffFormat,WAVE_STR,4);
	        riff_header.dwRiffSize =  RIFF_EXC_LEN;                         //头文件长度58 - 8(RIFF_ID和WAVE_STR)    
                memcpy(&headbuff[file_pos],&riff_header,str_size);
                file_pos += str_size;
                
                //设置FMT 块
                str_size = sizeof(struct FMT_BLOCK) - 2;  
                memcpy(&headbuff[file_pos],pfmt_block,str_size);
                file_pos += str_size;
                
                //设置FACT块
                str_size = sizeof(struct FACT_BLOCK);
                memcpy(fact_block.szFactID,FACT_ID,4);
                fact_block.dwFactSize = 4;
                memcpy(&headbuff[file_pos],&fact_block,str_size);
                file_pos += str_size;
                memcpy(&headbuff[file_pos],fack_block_buffer,fact_block.dwFactSize);
                file_pos += fact_block.dwFactSize;
                
                //设置DATA块
                str_size = sizeof(struct DATA_BLOCK);
                memcpy(data_block.szDataID,DATA_ID,4);
                data_block.dwDataSize = 0;                                      //创建头,数据长度先设置为0
                memcpy(&headbuff[file_pos],&data_block,str_size);
                file_pos += str_size;
                res = f_write(&wav_fil, headbuff, file_pos, &bytes_ret);
                if(res != FR_OK)
                {
                   f_close(&wav_fil);
                   return 1;
                }
                WAV_OPEN_FLAG = 1;                                              //标志已打开
                return 0;
	}
	f_close(&wav_fil);
	return 1;
}

/**
  * @brief  创建wav_G711编码格式音频文件头
  * @note      
  * @param  path: wav文件名
  * @param  mode: 访问模式和文件打开模式
  * @retval 返回0成功，非0失败
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
  fmt_block.wFormatTag = WAVE_FORMAT_ALAW;       //设置音频编码方式:(0x01-PCM,0x06-A-law,0x07-u-law)
  fmt_block.wChannels = 1;                       //单声道/立体声
  fmt_block.dwSamplesPerSec  = 8000;             //8Khz采样频率
  fmt_block.wBitsPerSample   = 8 ;               //每个采样需要16bit
  fmt_block.wBlockAlign      = fmt_block.wChannels*(fmt_block.wBitsPerSample/8) ;    //每个采样需要字节数,wChannels*(wBitsPerSample/8) =4
  fmt_block.dwAvgBytesPerSec = fmt_block.dwSamplesPerSec * fmt_block.wBlockAlign  ;  //每秒所需字节数,dwSamplesPerSec*wBlockAlign
  return Wav_Create_Head(path,mode,&fmt_block);
}

/**
  * @brief  写入G711音频数据到文件中
  * @note      
  * @param  buff: wav数据内容
  * @param  nobtw: wav数据内容长度
  * @retval 返回0成功，非0失败
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
  * @brief  关闭打开的WAV格式文件
  * @note      
  * @param  nobtw: wav数据内容长度
  * @param  RecordID: 多媒体ID
  * @retval 返回0成功，非0失败
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

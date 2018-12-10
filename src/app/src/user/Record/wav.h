//
//这是一个wave头文件的一些结构定义
//
//一般存在fact块的，wav文件头为58byte.
//例如一个总文件大小:63098byte wav文件
// dwRiffSize = 63098 - 8  =63090
// dwDataSize = 63098 - 58 =63048
//
//

#ifndef _WAV_H_
#define _WAV_H_
#include "stm32f2xx.h"
#include "ff.h"

#define WAV_FILE_MODE FA_CREATE_ALWAYS | FA_WRITE | FA_READ

#define RIFF_SIZE_POS 4
#define DATA_SIZE_POS 54
#define RIFF_EXC_LEN 50    //数据内容长度+RIFF_EXC_LEN=riff.dwRiffSize

#define RIFF_ID   "RIFF"
#define WAVE_STR  "WAVE"
#define FORMAT_ID "fmt "   //注意这里有个空格
#define FACT_ID   "fact"
#define DATA_ID   "data"

/* WAV保存的音频格式类型 */

#define	 WAVE_FORMAT_PCM    0x0001    //0x01-PCM,0x06-A-law,0x07-u-law
#define	 WAVE_FORMAT_ALAW   0x0006
#define  WAVE_FORMAT_MULAW  0x0007


struct RIFF_HEADER
{
	u8 szRiffID[4];    // 'R','I','F','F'
	u32 dwRiffSize;
	u8 szRiffFormat[4]; // 'W','A','V','E'
};

struct WAVE_FORMAT
{
	u16 wFormatTag;
	u16 wChannels;
	u32 dwSamplesPerSec;
	u32 dwAvgBytesPerSec;
	u16 wBlockAlign;
	u16 wBitsPerSample;
	u16 pack;        //附加信息
};

struct FMT_BLOCK
{
	u8 szFmtID[4];             // 'f','m','t',' '
	u32 dwFmtSize;
//	struct WAVE_FORMAT wavFormat;
	u16 wFormatTag;
	u16 wChannels;
	u32 dwSamplesPerSec;
	u32 dwAvgBytesPerSec;
	u16 wBlockAlign;
	u16 wBitsPerSample;
	u16 pack;                   //附加信息  
        u16  reserves1;              //保留,用于内存对齐-单所以计算读取和写入长度=sizeof(struct FMT_BLOCK) - 2
};

struct FACT_BLOCK
{
	u8 szFactID[4]; // 'f','a','c','t'
	u32 dwFactSize;
};

struct DATA_BLOCK
{
	u8 szDataID[4]; // 'd','a','t','a'
	u32 dwDataSize;
};

int Wav_Create_Head(	
	const char *path,
	u8 mode,
        struct FMT_BLOCK *pfmt_block
);

int Wav_Write_G711(	
	const char *path,
	u8 mode
);

int Wav_Write_Data(	
	const void *buff,
	u32 nobtw
);

int Wav_Close(u32 nobtw,UINT RecordID);

#endif
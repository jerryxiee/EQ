//
//����һ��waveͷ�ļ���һЩ�ṹ����
//
//һ�����fact��ģ�wav�ļ�ͷΪ58byte.
//����һ�����ļ���С:63098byte wav�ļ�
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
#define RIFF_EXC_LEN 50    //�������ݳ���+RIFF_EXC_LEN=riff.dwRiffSize

#define RIFF_ID   "RIFF"
#define WAVE_STR  "WAVE"
#define FORMAT_ID "fmt "   //ע�������и��ո�
#define FACT_ID   "fact"
#define DATA_ID   "data"

/* WAV�������Ƶ��ʽ���� */

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
	u16 pack;        //������Ϣ
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
	u16 pack;                   //������Ϣ  
        u16  reserves1;              //����,�����ڴ����-�����Լ����ȡ��д�볤��=sizeof(struct FMT_BLOCK) - 2
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
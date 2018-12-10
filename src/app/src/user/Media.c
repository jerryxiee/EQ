/********************************************************************
//版权说明	:Shenzhen E-EYE Co. Ltd., Copyright 2009- All Rights Reserved.
//文件名称	:Media.c		
//功能		:实现图像和录音的存储
//版本号	: 
//开发人	:dxl
//开发时间	:2012.9
//修改者	:
//修改时间	:
//修改简要说明	:
//备注		:
***********************************************************************/
//***************包含文件*****************
#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "include.h"
//****************宏定义****************

//***************本地变量******************
u8	CurrentWavFolder = 0;//当前录音文件夹
u8	CurrentJpgFolder = 0;//当前图像文件夹
static u16	CurrentWavFile = 0;//当前录音文件夹下的当前文件
static u16	CurrentJpgFile = 0;//当前图像文件夹下的当前文件
static u8	MediaDelEnableFlag = 0;//多媒体强制删除使能标志
static u8	EnableFlag = 1;//进入删除流程的使能标志
u8	SdOnOffFlag = 0;//SD卡在线标志；1在线，0不在线,当sd卡初始化失败时该标志也会清0
u32	JpgCreatTime;//jpg文件创建的时间
u32	WavCreatTime;//wav文件创建的时间
u8      MediaType = 0;//多媒体类型，0图像，1音频，2视频
u8      MediaFormat = 0;//多媒体格式编码,0JPEG,1TIF,2MP3,3WAV,4WMV
u8	MediaRatio = 1;//多媒体分辨率
u8      MediaChannelID = 0;//多媒体通道ID
u8      MediaEvenCode = 0;//多媒体事件编码
u16     MediaCommand = 0;//多媒体命令  0:停止  N:张数  0XFFFF:录象 0XFFFE:一直拍照  BY WYF
u16     MediaPhotoSpace = 0; //拍照间隔时间 BY WYF
u8	MemoryUploadFileName[_MAX_LFN];//存储多媒体数据上传的完整文件名
u8      MultiMediaSendFlag = 0;//多媒体发送标志，0为不在发送，1为在发送
u16	MemoryUploadSendCount = 0;//发送次数计数
//****************全局变量*****************

//****************外部变量*****************
extern RecordDef RECORD_DEF;            //该结构体包含了录音多媒体ID
extern u8      MultiMediaIndexSendFlag;//多媒体检索上传标志,1为需要检索上传,0为不需要
extern u8      RecordSendFlag;//录音文件上传标志,1为上传
extern u32     PhotoID;//图像多媒体ID
/*********************************************************************
//函数名称	:Media_GetSdOnLineFlag
//功能		:获得SD卡在线标志
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:返回1在线；返回0不在线
*********************************************************************/
u8 Media_GetSdOnLineFlag(void)
{
	return SdOnOffFlag;
}
/*********************************************************************
//函数名称	:Media_FindCurFolder
//功能		:查找JPG或WAV的当前位置（当前文件夹，当前文件）
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:Type只能为MEDIA_TYPE_JPG、MEDIA_TYPE_WAV
//		:文件夹下面都会有一个文件日志文件log.bin
*********************************************************************/
void Media_FindCurFolder(u8 *Folder, u16 *File, u8 Type)
{
	u8	Dir[15];
	u8	FileName[_MAX_LFN];
	u8	i;
	u8	flag;
	u8	length;
	FIL     file;
	DIR	dir;
	u8	CurFolder;
	u16	CurFile;
	u32	NewestTime;
	u32	Time;
	u8	Buffer[5];
	u32	Byte;
	//FRESULT res;
	
	if(0 == SdOnOffFlag)
	{
		return ;
	}

	if(MEDIA_TYPE_JPG == Type)
	{
		strcpy((char *)Dir,"1:/JPG");
	}
	else if(MEDIA_TYPE_WAV == Type)
	{
		strcpy((char *)Dir,"1:/WAV");
	}
	else
	{
		return ;
	}
	CurFolder = 0;
	CurFile = 0;
	flag = 0;
	NewestTime = 0;
	//查找有无已创建的目录
	for(i=1; i<=FOLDER_NUM_MAX; i++)//文件夹JPG1~JPG32
	{
        IWDG_ReloadCounter();//喂狗
		if(i < 10)
		{
			Dir[6] = i+0x30;
			Dir[7] = 0;
		}
		else
		{
			Dir[6] = i/10+0x30;
			Dir[7] = i%10+0x30;
			Dir[8] = 0;
		}
		if(FR_OK == f_opendir(&dir, ff_NameConver(Dir)))
		{
			strcpy((char *)FileName,(char const *)Dir);
			length = strlen((char const *)FileName);
			memcpy(FileName+length,"/log.bin",8);
			length += 8;
			FileName[length] = 0;
			if(f_open(&file,ff_NameConver(FileName),FA_READ|FA_WRITE) == FR_OK)
			{
				flag = 1;
				//if(0 == file.fsize%9)
                if((0 == file.fsize%9)&&(file.fsize >= 9))//dxl,2014.9.9修改为这个
				{
					if((file.fsize/9) < FOLDER_FILE_NUM_MAX)
					{
						f_close(&file);
						strcpy((char *)FileName,(char const *)Dir);
						length = strlen((char const *)FileName);
						strcpy((char *)FileName+length,"/delete.bin");
						if(f_open(&file,ff_NameConver(FileName),FA_READ|FA_WRITE) == FR_OK)
						{
						
						}
						else
						{
							//if(0 == file.fsize)
							//{
								f_close(&file);
								CurFolder = i;
								CurFile = file.fsize/9;
								break;
							//}
								/*
							res = f_lseek(&file,file.fsize-9);
							if(FR_OK == res)
							{
								if(FR_OK == f_read (&file, Buffer, 4, &Byte))
								{
									Time = 0;
									Time |= Buffer[0] << 24;
									Time |= Buffer[1] << 16;
									Time |= Buffer[2] << 8;
									Time |= Buffer[3];
									if(Time > NewestTime)
									{
										NewestTime = Time;
										CurFolder = i;
										CurFile = file.fsize/9;
									}	
								}
							}
								*/
							
							
						}
					}
					else
					{
						f_close(&file);
					}
				}
				else//log.bin文件出错
				{
					f_close(&file);
					strcpy((char *)FileName,(char const *)Dir);
					length = strlen((char const *)FileName);
					strcpy((char *)FileName+length,"/delete.bin");
					if(FR_OK == f_open(&file,ff_NameConver(FileName),FA_OPEN_ALWAYS|FA_READ|FA_WRITE))
					{
				
					}
					f_close(&file);
					
				}
			}
			else//log.bin不存在或打开出错
			{	
				strcpy((char *)FileName,(char const *)Dir);
				length = strlen((char const *)FileName);
				strcpy((char *)FileName+length,"/delete.bin");
				if(FR_OK == f_open(&file,ff_NameConver(FileName),FA_OPEN_ALWAYS|FA_READ|FA_WRITE))
				{
				
				}
				f_close(&file);
				
			}
			f_close(&file);
		}
		
	}
	if(0 == CurFolder)
	{
		if(0 == flag)//空卡时没有任何文件夹，创建JPG1
		{
			Dir[6] = '1';
			Dir[7] = 0;
			//创建新的目录
			f_mkdir(ff_NameConver(Dir));
			//创建日志文件
			strcpy((char *)FileName,(char const *)Dir);
			length = strlen((char const *)FileName);
			memcpy(FileName+length,"/log.bin",8);
			length += 8;
			FileName[length] = 0;
			//创建文件
			if(FR_OK == f_open(&file,ff_NameConver(FileName),FA_OPEN_ALWAYS|FA_READ|FA_WRITE))
            {
          
            }
            else
            {
                    //显示TF卡接口自检成功,dxl,2014.1.6,V1.01.06以后才具有该功能
                    LcdClearScreen(); 
                    LcdShowCaptionEx((char *)"TF卡自检失败",2); 
            }
			f_close(&file);
			CurFolder = 1;
			CurFile = 0;
		}
		else
		{
                        
			NewestTime = 0;
			//最新的时间为当前文件夹
			for(i=1; i<=FOLDER_NUM_MAX; i++)
			{
                IWDG_ReloadCounter();//喂狗
				if(i < 10)
				{
					Dir[6] = i+0x30;
					Dir[7] = 0;
				}
				else
				{
					Dir[6] = i/10+0x30;
					Dir[7] = i%10+0x30;
					Dir[8] = 0;
				}
				strcpy((char *)FileName,(char const *)Dir);
				length = strlen((char const *)FileName);
				memcpy(FileName+length,"/log.bin",8);
				FileName[length+8] = 0;
				if(FR_OK == f_open(&file,ff_NameConver(FileName),FA_READ|FA_WRITE))
				{
					if((0 == file.fsize%9)&&(file.fsize >=9))//至少要存储一半以上的文件
					{
						f_lseek(&file,file.fsize-9);
						if(FR_OK == f_read (&file, Buffer, 4, &Byte))
						{
							Time = 0;
							Time |= Buffer[0] << 24;
							Time |= Buffer[1] << 16;
							Time |= Buffer[2] << 8;
							Time |= Buffer[3];
							if(Time > NewestTime)
							{
								NewestTime = Time;
								CurFolder = i;
								CurFile = file.fsize/9;
							}
						}
					}
				}
				f_close(&file);
			}
			if(0 == NewestTime)//没有找到,默认为WAV1
			{
				Dir[6] = '1';
				Dir[7] = 0;
				//创建新的目录
				f_mkdir(ff_NameConver(Dir));
				//创建日志文件
				strcpy((char *)FileName,(char const *)Dir);
				length = strlen((char const *)FileName);
				memcpy(FileName+length,"/log.bin",8);
				length += 8;
				FileName[length] = 0;
				//创建文件
				if(FR_OK == f_open(&file,ff_NameConver(FileName),FA_OPEN_ALWAYS|FA_READ|FA_WRITE))
                {
                                
                }
                else
                {
                        //显示TF卡接口自检成功,dxl,2014.1.6,V1.01.06以后才具有该功能
                        LcdClearScreen(); 
                        LcdShowCaptionEx((char *)"TF卡自检失败",2); 
                }
				CurFolder = 1;
				CurFile = file.fsize/9;
				f_close(&file);
			}
		}
	}
        else
        {
          
        }
	*Folder = CurFolder;
	*File = CurFile;
}
/*********************************************************************
//函数名称	:Media_CheckSdOnOff
//功能		:检测SD卡在与不在
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:在:SdOnOffFlag = 1;不在SdOnOffFlag = 0,dxl,2014.6.18
//		:
*********************************************************************/
void Media_CheckSdOnOff(void)
{
    u8	Status;
	
	Status  = GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_13);
	if(0 == Status)
	{
		SdOnOffFlag = 1;
	}
	else
	{
		SdOnOffFlag = 0;
	}
}

/*********************************************************************
//函数名称	:Media_FolderCheck(void)
//功能		:文件夹自检
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:确定当前使用的录音文件夹和拍照文件夹
//		:文件夹下面都会有一个文件日志文件log.bin
*********************************************************************/
void Media_FolderCheck(void)
{
	

	if(1 == SdOnOffFlag)
	{
		//定位当前图像文件夹
		Media_FindCurFolder(&CurrentJpgFolder, &CurrentJpgFile, MEDIA_TYPE_JPG);
		//定位当前录音文件夹
		Media_FindCurFolder(&CurrentWavFolder, &CurrentWavFile, MEDIA_TYPE_WAV);
		//标记需要删除的文件
		Media_SignDelFlag(MEDIA_TYPE_JPG);
		//标记需要删除的文件
		Media_SignDelFlag(MEDIA_TYPE_WAV);
		
	}
}
/*********************************************************************
//函数名称	:Media_EnableDeleteFlag(void)
//功能		:使能删除标志
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:当容量非常小时会强制删除文件
*********************************************************************/
void Media_EnableDeleteFlag(void)
{

	if(0 == SdOnOffFlag)
	{
		return;
	}
	EnableFlag = 1;
	MediaDelEnableFlag = 1;
}
/*********************************************************************
//函数名称	:Media_GetSignNum
//功能		:获取已标记文件夹数量
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:Type只能为MEDIA_TYPE_JPG、MEDIA_TYPE_WAV
*********************************************************************/
u8 Media_GetSignNum(u8 Type)
{
	u8	i;
	u8	length;
	u8	FileName[_MAX_LFN];
	FIL	file;
	u8	count;
	
	if(0 == SdOnOffFlag)
	{
		return 0;
	}

	if(MEDIA_TYPE_JPG == Type)
	{
		strcpy((char *)FileName,"1:/JPG");
	}
	else if(MEDIA_TYPE_WAV == Type)
	{
		strcpy((char *)FileName,"1:/WAV");
	}
	length = strlen((char *)FileName);
	count = 0;
	for(i=1; i<= FOLDER_NUM_MAX; i++)
	{
        IWDG_ReloadCounter();//喂狗
		if(i<10)
		{
			FileName[length++] = i+0x30;
			strcpy((char *)FileName+length,"/delete.bin");
			if(FR_OK == f_open(&file,ff_NameConver(FileName),FA_READ|FA_WRITE))
			{
				count++;
				f_close(&file);
			}
			else
			{
				f_close(&file);
				length = 6;
				FileName[length++] = i+0x30;
				strcpy((char *)FileName+length,"/DELETE.BIN");
				if(FR_OK == f_open(&file,ff_NameConver(FileName),FA_READ|FA_WRITE))
				{
					count++;
				}
				f_close(&file);
			}
		}
		else 
		{
			FileName[length++] = i/10+0x30;
			FileName[length++] = i%10+0x30;
			strcpy((char *)FileName+length,"/delete.bin");
			if(FR_OK == f_open(&file,ff_NameConver(FileName),FA_READ|FA_WRITE))
			{
				count++;
				f_close(&file);
			}
			else
			{
				f_close(&file);
				length = 6;
				FileName[length++] = i/10+0x30;
				FileName[length++] = i%10+0x30;
				strcpy((char *)FileName+length,"/DELETE.BIN");
				if(FR_OK == f_open(&file,ff_NameConver(FileName),FA_READ|FA_WRITE))
				{
					count++;
				}
				f_close(&file);
			}
		}
		
	}
	return count;
	
}
/*********************************************************************
//函数名称	:Media_GetFileName
//功能		:获取文件名
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:Type只能为MEDIA_TYPE_JPG、MEDIA_TYPE_WAV
//		:录音文件名：月+日+时+分，遵循8.3文件名格式
//		:图像文件名：日+时+分+秒，遵循8.3文件名格式
*********************************************************************/
void Media_GetFileName(u8 *pBuffer, u8 Type)
{
	TIME_T tt;
	u8	*p;
	u8	length;

	if(0 == SdOnOffFlag)
	{
		return ;
	}
	if(MEDIA_TYPE_JPG == Type)
	{
		RTC_GetCurTime(&tt);
		JpgCreatTime = RTC_GetCounter();
		p=pBuffer;
		strcpy((char *)p,"1:/JPG");
		length = strlen((char const *)p);
		p += length;
		if(CurrentJpgFolder < 10)
		{
			*p++ = CurrentJpgFolder+0x30;
			*p++ = '/';
		} 
		else
		{
			*p++ = CurrentJpgFolder/10+0x30;
			*p++ = CurrentJpgFolder%10+0x30;
			*p++ = '/';
		}
		*p++ = tt.day/10+0x30;
		*p++ = tt.day%10+0x30;
		*p++ = tt.hour/10+0x30;
		*p++ = tt.hour%10+0x30;
		*p++ = tt.min/10+0x30;
		*p++ = tt.min%10+0x30;
		*p++ = tt.sec/10+0x30;
		*p++ = tt.sec%10+0x30;
		*p++ = '.';
		*p++ = 'j';
		*p++ = 'p';
		*p++ = 'g';
		*p++ = 0;//结尾字符
	}
	else if(MEDIA_TYPE_WAV == Type)
	{
		RTC_GetCurTime(&tt);
		WavCreatTime = RTC_GetCounter();
		p=pBuffer;
		strcpy((char *)p,"1:/WAV");
		length = strlen((char const *)p);
		p += length;
		if(CurrentWavFolder < 10)
		{
			*p++ = CurrentWavFolder+0x30;
			*p++ = '/';
		} 
		else
		{
			*p++ = CurrentWavFolder/10+0x30;
			*p++ = CurrentWavFolder%10+0x30;
			*p++ = '/';
		}
		*p++ = tt.month/10+0x30;
		*p++ = tt.month%10+0x30;
		*p++ = tt.day/10+0x30;
		*p++ = tt.day%10+0x30;
		*p++ = tt.hour/10+0x30;
		*p++ = tt.hour%10+0x30;
		*p++ = tt.min/10+0x30;
		*p++ = tt.min%10+0x30;
		*p++ = '.';
		*p++ = 'w';
		*p++ = 'a';
		*p++ = 'v';
		*p++ = 0;//结尾字符
	}
}
/*********************************************************************
//函数名称	:Media_SaveLog(void)
//功能		:保存log.bin文件
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:Type只能为MEDIA_TYPE_JPG、MEDIA_TYPE_WAV
//备注		:在保存完一副图像或一段录音后需要调用该函数，把相关信息保存至检索文件log.bin中
*********************************************************************/
void Media_SaveLog(u32 MediaID, u8 Type)
{
	FIL file;
	u8	Dir[15];
	u8	FileName[_MAX_LFN];
	u8	CurrentFolder;
	u16	CurrentFile;
	u8	i;
	u8	sum;
	u8	length;
	u8	Buffer[12];
	u32	temp;
	u32	Bw;
	u32	CreatTime;
	FRESULT  res;
	
	if(0 == SdOnOffFlag)
	{
		return ;
	}

	if(MEDIA_TYPE_JPG == Type)
	{
		CurrentJpgFile++;
		CurrentFolder = CurrentJpgFolder;
		if(0 == CurrentFolder)
		{
			return ;
		}
		strcpy((char *)Dir,"1:/JPG");
		CreatTime = JpgCreatTime;
		CurrentFile = CurrentJpgFile;
	}
	else if(MEDIA_TYPE_WAV == Type)
	{
		CurrentWavFile++;
		CurrentFolder = CurrentWavFolder;
		if(0 == CurrentFolder)
		{
			return ;
		}
		strcpy((char *)Dir,"1:/WAV");
		CreatTime = WavCreatTime;
		CurrentFile = CurrentWavFile;
	}
	else
	{
		return ;
	}
	if(CurrentFolder < 10)
	{
		Dir[6] = CurrentFolder+0x30;
		Dir[7] = 0;
	}
	else
	{
		Dir[6] = CurrentFolder/10+0x30;
		Dir[7] = CurrentFolder%10+0x30;
		Dir[8] = 0;
	}
	//写日志文档log.bin
	strcpy((char *)FileName,(char const *)Dir);
	length = strlen((char const *)FileName);
	memcpy(FileName+length,"/log.bin",8);
	FileName[length+8] = 0;
	if(FR_OK == f_open(&file,ff_NameConver(FileName),FA_OPEN_ALWAYS|FA_READ|FA_WRITE))
	{
		f_lseek(&file,file.fsize);
		//写入该文件创建的时间
		temp = CreatTime;
		Buffer[0] = temp >> 24;
		Buffer[1] = temp >> 16;
		Buffer[2] = temp >> 8;
		Buffer[3] = temp;
		//写入多媒体ID
		temp = MediaID;
		Buffer[4] = temp >> 24;
		Buffer[5] = temp >> 16;
		Buffer[6] = temp >> 8;
		Buffer[7] = temp;
		sum = 0;
		for(i=0; i<8; i++)
		{
			sum += Buffer[i];
		}
		Buffer[8] = sum;
		res = f_write(&file,Buffer,9,&Bw);
		f_close(&file);
	}
	if(CurrentFile >= FOLDER_FILE_NUM_MAX)//表明已满当前文件夹已满
	{
		CurrentFile = 0;
		//查找下一个文件夹存在否
		CurrentFolder++;
		if(CurrentFolder > FOLDER_NUM_MAX)
		{
			CurrentFolder = 1;
		}
		if(MEDIA_TYPE_JPG == Type)
		{
			CurrentJpgFolder = CurrentFolder;
			CurrentJpgFile = 0;
		}
		else if(MEDIA_TYPE_WAV == Type)
		{
			CurrentWavFolder = CurrentFolder;
			CurrentWavFile = 0;
		}
		if(CurrentFolder < 10)
		{
			Dir[6] = CurrentFolder+0x30;
			Dir[7] = 0;
		}
		else
		{
			Dir[6] = CurrentFolder/10+0x30;
			Dir[7] = CurrentFolder%10+0x30;
			Dir[8] = 0;
		}
		//创建新的文件夹
		res = f_mkdir(ff_NameConver(Dir));
		//创建日志文档
		strcpy((char *)FileName,(char const *)Dir);
		length = strlen((char const *)FileName);
		memcpy(FileName+length,"/log.bin",8);
		FileName[length+8] = 0;
		res = f_open(&file,ff_NameConver(FileName),FA_OPEN_ALWAYS|FA_READ|FA_WRITE);
		f_close(&file);
		//标记需要删除的文件
		Media_SignDelFlag(Type);
	}
	if(FR_OK == res)
	{
	
	}
}
/*********************************************************************
//函数名称	:Media_SignDelFlag(void)
//功能		:标记删除标志
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:在一个文件夹创建一个delete.bin文件，用于标识该文件夹需要删除,调用一次只能标记一个文件夹
*********************************************************************/
void Media_SignDelFlag(u8 Type)
{
	u32	ReservedByte;
	DIR dir;
	FIL file;
	u8	Dir[15];
	u8	Folder;
	u8	FileName[_MAX_LFN];
	u8	i;
	u8	j;
	u8	count;
	u32	OldestTime;
	u32	Time;
	u8	Buffer[5];
	u32	Byte;
	u8	length;
	u8	flag;
	u8	SignNum;
	
	if(0 == SdOnOffFlag)
	{
		return ;
	}

	if(MEDIA_TYPE_JPG == Type)
	{
		strcpy((char *)Dir,"1:/JPG");
		Folder = CurrentJpgFolder;
		if(0 == Folder)
		{
			return ;
		}
	}
	else
	{
		strcpy((char *)Dir,"1:/WAV");
		Folder = CurrentWavFolder;
		if(0 == Folder)
		{
			return ;
		}
	}
	flag = 0;
	//判断是否进入循环存储
	for(i=0; i<2; i++)
	{
		Folder++;
		if(Folder > FOLDER_NUM_MAX)
		{
			Folder = 1;
		}
		if(Folder < 10)
		{
			Dir[6] = Folder+0x30;
			Dir[7] = 0;
		}
		else
		{
			Dir[6] = Folder/10+0x30;
			Dir[7] = Folder%10+0x30;
			Dir[8] = 0;
		}
        IWDG_ReloadCounter();//喂狗,dxl,2014.6.18
		if(FR_OK == f_opendir(&dir,ff_NameConver(Dir)))
		{
			strcpy((char *)FileName,(char const *)Dir);
			length = strlen((char const *)FileName);
			memcpy(FileName+length,"/log.bin",8);
			FileName[length+8] = 0;
			if(FR_OK == f_open(&file,ff_NameConver(FileName),FA_READ|FA_WRITE))
			{
				if(file.fsize >= 9)
				{
					f_close(&file);
					flag = 1;
					strcpy((char *)FileName,(char const *)Dir);
					length = strlen((char const *)FileName);
					strcpy((char *)FileName+length,"/delete.bin");
					if(FR_OK == f_open(&file,ff_NameConver(FileName),FA_OPEN_ALWAYS|FA_READ|FA_WRITE))
					{
				
					}
					f_close(&file);
					Media_EnableDeleteFlag();//不管是ACC ON还是ACC OFF都会删除文件
				}
			}
			f_close(&file);
		}
	}
	if(1 == flag)
	{
		return ;//优先标记需要删除的
	}
		
	//读剩余空间,小于500M时开始标记
	SignNum = Media_GetSignNum(Type);
	if(SignNum >= 3)//达到3被标记时强制删除
	{
		Media_EnableDeleteFlag();//不管是ACC ON还是ACC OFF都会删除文件
		return ;//最多只能标记两个
	}
	ReservedByte = GetRestKByte();
	if(0 == ReservedByte)//读剩余空间错误
	{
		return ;
	}
	if(ReservedByte <= MEDIA_START_SIGN_SIZE)//小于500M时会标记要删除的文件
	{
		OldestTime = 0xffffffff;
		count = 0;
		for(i=1; i<=FOLDER_NUM_MAX; i++)
		{
                IWDG_ReloadCounter();//喂狗
			if(i < 10)
			{
				Dir[6] = i+0x30;
				Dir[7] = 0;
			}
			else
			{
				Dir[6] = i/10+0x30;
				Dir[7] = i%10+0x30;
				Dir[8] = 0;
			}
			strcpy((char *)FileName,(char const *)Dir);
			length = strlen((char const *)FileName);
			memcpy(FileName+length,"/log.bin",8);
			FileName[length+8] = 0;
            IWDG_ReloadCounter();//喂狗,dxl,2014.6.18
			if(FR_OK == f_open(&file,ff_NameConver(FileName),FA_READ|FA_WRITE))
			{
				count++;
				if(file.fsize >= 9)
				{
					f_lseek(&file,file.fsize-9);
					if(FR_OK == f_read (&file, Buffer, 4, &Byte))
					{
						Time = 0;
						Time |= Buffer[0] << 24;
						Time |= Buffer[1] << 16;
						Time |= Buffer[2] << 8;
						Time |= Buffer[3];
						f_close(&file);
						if(Time < OldestTime)
						{
							strcpy((char *)FileName,(char const *)Dir);
							length = strlen((char const *)FileName);
							strcpy((char *)FileName+length,"/delete.bin");
							if(FR_OK == f_open(&file,ff_NameConver(FileName),FA_READ|FA_WRITE))
							{
							
							}
							else
							{
								OldestTime = Time;
								j = i;
							}
						}
					}
				}
			}
			f_close(&file);
		}
		if((OldestTime != 0xffffffff)&&((count-SignNum) > 3))//至少保留3个没有被标记的文件夹
		{
			if(j < 10)
			{
				Dir[6] = j+0x30;
				Dir[7] = 0;
			}
			else
			{
				Dir[6] = j/10+0x30;
				Dir[7] = j%10+0x30;
				Dir[8] = 0;
			}
			strcpy((char *)FileName,(char const *)Dir);
			length = strlen((char const *)FileName);
			strcpy((char *)FileName+length,"/delete.bin");
			if(FR_OK == f_open(&file,ff_NameConver(FileName),FA_OPEN_ALWAYS|FA_READ|FA_WRITE))
			{
				
			}
			f_close(&file);
		}
		SignNum = Media_GetSignNum(Type);
		//标记图像和音频
		if((SignNum >= 3)||(ReservedByte <= MEDIA_UNLIMITED_SIGN_SIZE))//小于100M或达到3被标记时强制删除
		{
			Media_EnableDeleteFlag();//不管是ACC ON还是ACC OFF都会删除文件
			return ;//最多只能标记两个
		}
	}
	
	
	
	
}
/*********************************************************************
//函数名称	:Media_TimeTask
//功能		:该任务负责删除SD卡中标记了的文件夹
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:在一个文件夹创建一个delete.bin文件，用于标识该文件夹需要删除,调用一次只能标记一个文件夹
*********************************************************************/
FunctionalState   Media_TimeTask(void)
{
	static u8	DelFolder = 0;
	static u8	DelType = 0;
	static u8	DelState = 0;
	static u16	Count = 0;
	static u8	DelDir[15];
	static u16	Count2 = 0;
	
	u8	i;
	u8	ACC;
	u8	FileName[_MAX_LFN];
	u8	flag;
	u8	length;
	u8	Dir[15];
	u8	SignNum;
	u32	ReservedByte;
	DIR	dir;
	FRESULT res;
	FILINFO FileInfo;
	FIL	file;
	u8      FileName_1[_MAX_LFN];
        
    if(0 == SdOnOffFlag)
	{
		return ENABLE;
	}
        
	ACC = Io_ReadStatusBit(STATUS_BIT_ACC);
	if(0 == ACC)
	{
		SetTimerTask(TIME_MEDIA, SYSTICK_1SECOND);//1秒调度一次
		
	}
	else
	{
		SetTimerTask(TIME_MEDIA, 5*SYSTICK_0p1SECOND);//0.5秒调度一次
	}
	Count2++;
	if(Count2 >= 7200)//ACC ON时,每小时检测一次剩余空间,ACC OFF时每2小时检测一次
	{
		Count2 = 0;
		ReservedByte = GetRestKByte();
		if(ReservedByte <= MEDIA_UNLIMITED_SIGN_SIZE)//小于500M时会标记要删除的文件
		{
			//读当前标记的数量
			SignNum = Media_GetSignNum(MEDIA_TYPE_JPG);
			if(0 == SignNum)
			{
				Media_SignDelFlag(MEDIA_TYPE_JPG);
			}
			//读当前标记的数量
			SignNum = Media_GetSignNum(MEDIA_TYPE_WAV);
			if(0 == SignNum)
			{
				Media_SignDelFlag(MEDIA_TYPE_WAV);
			}
			Media_EnableDeleteFlag();
		}
	}
	if((0 == ACC)||(1 == MediaDelEnableFlag))//ACC OFF
	{
		if(0 == DelState)//空闲
		{
			if(1 == EnableFlag)
			{
				DelState = 1;
				Count = 0;
			}
			else
			{
				Count++;
				if(Count > 1800)//空闲半小时监测1次，看是否有文件需要删除
				{
					EnableFlag = 1;
				}
			}
		}
		else if(1 == DelState)//确定具体的文件夹
		{
			DelType = DelType%2;
			if(0 == DelType)
			{
				strcpy((char *)Dir,"1:/WAV");//录音优先
			}
			else
			{
				strcpy((char *)Dir,"1:/JPG");
			}
			DelFolder = 0;
			for(i=1; i<=FOLDER_NUM_MAX; i++)
			{
				if(i < 10)
				{
					Dir[6] = i+0x30;
					Dir[7] = 0;
				}
				else
				{
					Dir[6] = i/10+0x30;
					Dir[7] = i%10+0x30;
					Dir[8] = 0;
				}
				strcpy((char *)FileName,(char const *)Dir);
				length = strlen((char const *)FileName);
				strcpy((char *)FileName+length,"/delete.bin");
                IWDG_ReloadCounter();//喂狗,dxl,2014.6.18
				if(FR_OK == f_open(&file,ff_NameConver(FileName),FA_READ|FA_WRITE))
				{
					DelFolder = i;
					strcpy((char *)DelDir,(char const*)Dir);
					f_close(&file);
					break;
				}
				f_close(&file);
				
			}
			DelType++;//下一次删除的类型要改变
			if(0 != DelFolder)//找到了文件夹
			{
				DelState = 2;
			}
			else
			{
				DelType = DelType%2;
				if(0 == DelType)
				{
					strcpy((char *)Dir,"1:/WAV");//录音优先
				}
				else
				{
					strcpy((char *)Dir,"1:/JPG");
				}
				DelFolder = 0;
				for(i=1; i<=FOLDER_NUM_MAX; i++)
				{
					if(i < 10)
					{
						Dir[6] = i+0x30;
						Dir[7] = 0;
					}
					else
					{
						Dir[6] = i/10+0x30;
						Dir[7] = i%10+0x30;
						Dir[8] = 0;
					}
					strcpy((char *)FileName,(char const *)Dir);
					length = strlen((char const *)FileName);
					strcpy((char *)FileName+length,"/delete.bin");
                    IWDG_ReloadCounter();//喂狗,dxl,2014.6.18
					if(FR_OK == f_open(&file,ff_NameConver(FileName),FA_READ|FA_WRITE))
					{
						DelFolder = i;
						strcpy((char *)DelDir,(char const*)Dir);
						f_close(&file);
						break;
					}
					f_close(&file);
				
				}
				DelType++;//下一次删除的类型要改变
				if(0 != DelFolder)//找到了文件夹
				{
					DelState = 2;
				}
				else
				{
					EnableFlag = 0;//关闭删除流程
					DelState = 0;
					MediaDelEnableFlag = 0;
				}
			}
			
		}
		else if(2 == DelState)//删除文件
		{
			flag = 0;
			if(f_opendir(&dir, ff_NameConver(DelDir)) == FR_OK)   //打开目录成功
            {
                for(;;)
				{
                    IWDG_ReloadCounter();//喂狗,dxl,2014.6.18
                    res = f_readdir(&dir,&FileInfo);
                    if(res != FR_OK || FileInfo.fname[0]==0)break;
                    if(FileInfo.fname[0]=='.')continue  ;
				    strcpy((char *)FileName,(char const*)DelDir);
                    ff_NameForward(FileName_1,FileInfo.fname);
			        if((0 == strncmp((char const *)FileName_1,"delete",6))||(0 == strncmp((char const *)FileName_1,"DELETE",6)))
			        {
			            //到最后删除delete.bin文件
			        }
			        else
			        {
	                    flag = 1;
				        length = strlen((char const *)FileName);
					    FileName[length] = '/';
					    length++;
					    FileName[length] = 0;
                        ff_NameForward(FileName_1,FileInfo.fname);
                        strcat((char *)FileName,(char const *)FileName_1);
                        res = f_unlink(ff_NameConver(FileName));       //删除文件
                        break; 
			        }
		        }
			}
			//是否只剩最后一个文件了?
			if(0 == flag)
			{
				//删除delete.bin文件
				strcpy((char *)FileName,(char const *)DelDir);
				length = strlen((char const *)FileName);
				strcpy((char *)FileName+length,"/delete.bin");
				res = f_unlink(ff_NameConver(FileName));  //删除文件
				//删除目录
				res = f_unlink(ff_NameConver(DelDir)); //删除文件
				DelState = 0;
			}
			
			
		}
		
	}
	else
	{
		DelState = 0;
		DelType = 0;
		Count = 0;
		EnableFlag = 1;
	}
	return ENABLE;
}
/*********************************************************************
//函数名称	:Media_UploadTimeTask(void)
//功能		:存储多媒体数据上传定时任务
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:
//返回		:
//备注		:
*********************************************************************/
FunctionalState Media_UploadTimeTask(void)
{
	static  u16	MemoryUploadSendCountMax = 0;//发送次数计数最大值
	static  u8 	State = 0;//状态
	static  u8	count = 0;
	static  u8	MemoryUploadBuffer[600];
	static  u8	Buffer[37];
	static  FIL	file;
	u32	Byte;
	u16	length;
	u8	flag = 0;
	u8      Buffer2[5];
        u32     temp;
        u8 channel = CHANNEL_DATA_1;//为编译通过而添加
        
        if(0 == strlen((char const *)MemoryUploadFileName))
        {
                MultiMediaSendFlag = 0;
		State = 0;
		count = 0;
		MemoryUploadSendCount = 0;
		MemoryUploadSendCountMax = 0;
		memset(MemoryUploadFileName,0,33);
                RecordSendFlag = 0;
                return DISABLE;
        }
	if(0 == State)//打开文件
	{
		if(FR_OK == f_open(&file,ff_NameConver(MemoryUploadFileName),FA_READ))
		{
			count = 0;
			State++;
			MemoryUploadSendCount = 1;//发送次数计数
			MemoryUploadSendCountMax = (file.fsize-36)/512;
			if(0 != (file.fsize-36)%512)
			{
				MemoryUploadSendCountMax++;
			}
                        Subpackage_Media_File_Name_Set((char const *)MemoryUploadFileName);
                        
		}
		else
		{
			count++;
			if(count >= 3)
			{
				f_close(&file);
				flag = 1;
			}
		}
	}
        else if(1 == State)//获取属性信息
	{
		f_lseek(&file, file.fsize-36);
		if(FR_OK == f_read (&file, Buffer, 36, &Byte))
		{
			count = 0;
			State++;
			f_lseek(&file, 0);
		}
		else
		{
			count++;
			if(count >= 3)
			{
				f_close(&file);
				flag = 1;
			}
		}
	}
	else if(2 == State)//读取一包数据
	{
		if(FR_OK == f_read (&file, MemoryUploadBuffer, 512, &Byte))
		{
			count = 0;
			State++;
		}
		else
		{
			count++;
			if(count >= 3)
			{
				f_close(&file);
				flag = 1;
			}
		}
	}
	else if(3 == State)//发送
	{
		if(1 == MemoryUploadSendCount)//第1包
		{
			memmove(MemoryUploadBuffer+36,MemoryUploadBuffer,512);
			memcpy(MemoryUploadBuffer,Buffer,36);
			length = 36+512;
                        //设置心跳包时间间隔为10分钟
                        temp = 600;
	                Buffer2[0] = (temp & 0xff000000) >> 24;
	                Buffer2[1] = (temp & 0xff0000) >> 16;
	                Buffer2[2] = (temp & 0xff00) >> 8;
	                Buffer2[3] = temp;
	                EepromPram_WritePram(E2_TERMINAL_HEARTBEAT_ID, Buffer2, E2_TERMINAL_HEARTBEAT_LEN);
		}
		else
		{
			length = 512;
		}
		if(ACK_OK == RadioProtocol_MultiMediaDataReport(channel,MemoryUploadBuffer, length, MemoryUploadSendCountMax, MemoryUploadSendCount))
		{
			MemoryUploadSendCount++;
			if(MemoryUploadSendCount > MemoryUploadSendCountMax)
			{
				State++;
			}
			else
			{
				State--;
			}
			count = 0;
		}
		else
		{
			count++;
			if(count >= 30)//重传30次不成功，退出
			{
				flag = 1;
                                f_close(&file);
			}
		}
	}
	else
	{
		f_close(&file);
		flag = 1;
	}
	if(0 == SdOnOffFlag)
    {
        flag = 1;//SD卡不在或总线通信故障了直接关闭该任务,dxl,2014.6.18
    }
	if(0 == flag)
	{
		MultiMediaSendFlag = 1;
		//暂停任务,待上传完以后再开启
		ClrTimerTask(TIME_CAMERA);
		///ClrTimerTask(TIME_RECORD);
		//暂停心跳包
                ClrTimerTask(TIME_HEART_BEAT);
                //暂停位置汇报,行标检测时使用，实际出货时不使用
                //ClrTimerTask(TIME_POSITION);传输多媒体时定时任务是关闭的，为测试盲区不关闭定时上报
      		return ENABLE;
	}
	else
	{
		MultiMediaSendFlag = 0;
		State = 0;
		count = 0;
		MemoryUploadSendCount = 0;
		MemoryUploadSendCountMax = 0;
		memset(MemoryUploadFileName,0,33);
                RecordSendFlag = 0;
		//开启拍照
		SetTimerTask(TIME_CAMERA, 1);
                //SetTimerTask(TIME_CAMERA, 1);//dxl
		//SetTimerTask(TIME_RECORD,RECORD_TASK_TIME);//启动设置调度任务
                //开启心跳包
                SetTimerTask(TIME_HEART_BEAT, SYSTICK_1SECOND);
                //开启位置汇报,行标检测时使用，实际出货时不使用
                //SetTimerTask(TIME_POSITION, SYSTICK_1SECOND);
                //设置心跳包时间间隔为60秒
                //temp = 180;//行标检测时设置长些，以免影响其它检测功能
		        //temp = 60;
	        //Buffer2[0] = (temp & 0xff000000) >> 24;
	        //Buffer2[1] = (temp & 0xff0000) >> 16;
	        //Buffer2[2] = (temp & 0xff00) >> 8;
	        //Buffer2[3] = temp;
	        //EepromPram_WritePram(E2_TERMINAL_HEARTBEAT_ID, Buffer2, E2_TERMINAL_HEARTBEAT_LEN);
		return DISABLE;
	}
}

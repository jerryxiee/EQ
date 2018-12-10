/********************************************************************
//版权说明	:Shenzhen E-EYE Co. Ltd., Copyright 2009- All Rights Reserved.
//文件名称	:MediaSearch.c		
//功能		:实现图像和录音的检索
//版本号	:
//开发人	:dxl 
//开发时间	:2012.11
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
static u8	MediaFindFolder1 = 0;//多媒体检索获得的文件夹1
static u8	MediaFindFolder2 = 0;//多媒体检索获得的文件夹2

//****************全局变量*****************

//****************外部变量*****************

/*********************************************************************
//函数名称	:MediaSearch_Find
//功能		:查找符合条件文件夹和文件名
//输入		:查找的起始时间，结束时间，多媒体类型
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//备注		:满足条件的文件所在的文件夹和文件名
//		:从开始时间算起，最多只能检索30项（包长限制）
//		:*Folder1==0或*Folder2==0表示没有满足条件的文件夹，非0值表示找到了相应的文件
//		:找到了相应的文件夹后再去读对应buffer（如*Foler1对应*pBuffer）的值，4个字节一组，高位在前，
//		:当连续读到4个字节0时表示结束。每4个字节表示一个时间，对应一个文件名，要获取年月日时分秒
//		:需调用Gmtime(TIME_T *tt, u32 counter)函数。
//		:目前只检索满足时间要求的文件，不按事件编码，通道号检索
//		:Type只能为MEDIA_TYPE_JPG、MEDIA_TYPE_WAV
*********************************************************************/
void MediaSearch_Find(u8 *Folder1, u8 *Folder2, u8 *pBuffer1, u8 *pBuffer2, u8 MediaType, TIME_T *StartTime, TIME_T *EndTime)
{
	u32	StartTimeCount;
	u32	EndTimeCount;
	u32	TimeBuffer[64];
	u32	temp;
	u8	Buffer[10];
	u8	Folder[3];
	u8	Dir[15];
	u8	FileName[_MAX_LFN];
	u8	i;
	//u32	j;
	u8	length;
	UINT	Byte;
	u8	*p = NULL;
	u16	count1;
	u16	count3;
	DIR dir;
	FIL file1,file2;
	u16	Count;
	u16	CountMax;

	//检查时间格式，年的范围是00-99
	if(ERROR == CheckTimeStruct(StartTime))
	{
		*Folder1 = 0;//错误清0
		*Folder2 = 0;
		return ;
	}
	if(ERROR == CheckTimeStruct(EndTime))
	{
		*Folder1 = 0;
		*Folder2 = 0;
		return ;
	}
	//初始化目录
	if(MEDIA_TYPE_JPG == MediaType)
	{
		strcpy((char *)Dir,"1:/JPG");
	}
	else if(MEDIA_TYPE_WAV == MediaType)
	{
		strcpy((char *)Dir,"1:/WAV");
	}
	else
	{
		*Folder1 = 0;
		*Folder2 = 0;
		return ;
	}
	
	//把结构体时间转换成32位整数表示的时间
	StartTimeCount = ConverseGmtime(StartTime);
	EndTimeCount = ConverseGmtime(EndTime);
	if(StartTimeCount > EndTimeCount)
	{
		*Folder1 = 0;
		*Folder2 = 0;
		return ;
	}

	//读取32个文件夹log.bin文件的起始时间和结束时间，没有该文件夹或有delete.bin文件的填充0
	length = strlen((char const *)Dir);
	for(i=1; i<=32; i++)
	{
                IWDG_ReloadCounter();//喂狗
		//获得文件夹名字
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
		if(FR_OK == f_opendir(&dir,ff_NameConver(Dir)))//打开文件夹
		{
			//获得文件名
			strcpy((char *)FileName,(char const *)Dir);
			length = strlen((char const *)FileName);
			memcpy(FileName+length,"/log.bin",8);
			length += 8;
			FileName[length] = 0;
			if(FR_OK == f_open(&file1,ff_NameConver(FileName),FA_READ))
			{
				//判断是否标记了删除
				strcpy((char *)FileName,(char const *)Dir);
				length = strlen((char const *)FileName);
				memcpy(FileName+length,"/delete.bin",11);
				length += 11;
				FileName[length] = 0;
				if(FR_OK == f_open(&file2,ff_NameConver(FileName),FA_READ))
				{
					//被标记了
					TimeBuffer[2*(i-1)] = 0;
					TimeBuffer[2*(i-1)+1] = 0;
					
				}
				else
				{
					//读该文件夹下最老时间
					f_lseek(&file1,0);
					temp = 0;
                IWDG_ReloadCounter();//喂狗
					if(FR_OK == f_read (&file1, Buffer, 4, &Byte))
					{
						temp |= Buffer[0] << 24;
						temp |= Buffer[1] << 16;
						temp |= Buffer[2] << 8;
						temp |= Buffer[3];
					}
					else
					{
					
					}
					TimeBuffer[2*(i-1)] = temp;
					//读该文件夹下最新时间
					f_lseek(&file1,file1.fsize-9);
					temp = 0;
					if(FR_OK == f_read (&file1, Buffer, 4, &Byte))
					{
						temp |= Buffer[0] << 24;
						temp |= Buffer[1] << 16;
						temp |= Buffer[2] << 8;
						temp |= Buffer[3];
					}
					else
					{
					
					}
					TimeBuffer[2*(i-1)+1] = temp;
				}
				f_close(&file2);
			}
			else
			{
				TimeBuffer[2*(i-1)] = 0;
				TimeBuffer[2*(i-1)+1] = 0;
			}
			f_close(&file1);
		}
		else
		{
			TimeBuffer[2*(i-1)] = 0;
			TimeBuffer[2*(i-1)+1] = 0;
		}
		
	}
	//确定给定的起始时间和结束时间属于哪个文件夹，最多只查找两个文件夹
	*Folder1 = 0;
	*Folder2 = 0;
	for(i=0; i<4; i++)
	{
		*(pBuffer1+i) = 0;
	}
	for(i=0; i<4; i++)
	{
		*(pBuffer2+i) = 0;
	}
    temp = 0xffffffff;
	for(i=1; i<=32; i++)
	{
		//dxl,2014.3.26屏蔽
		//if((EndTimeCount>=TimeBuffer[2*(i-1)])&&(EndTimeCount<=TimeBuffer[2*(i-1)+1]))
		//{
			//if(0 == *Folder2)
			//{
				//*Folder2 = i;
                //break;
			//}	
		//}
        //if((StartTimeCount>=TimeBuffer[2*(i-1)])&&(StartTimeCount<=TimeBuffer[2*(i-1)+1]))
        //if(TimeBuffer[2*(i-1)+1] <= EndTimeCount)
		//{
			//if(0 == *Folder1)
			//{
				//*Folder1 = i;
                //break;
			//}
		//}
        if(TimeBuffer[2*(i-1)] >= StartTimeCount)//以离开始时间最近的那个时间为优先
        {
            if(TimeBuffer[2*(i-1)] < temp)
            {
                temp = TimeBuffer[2*(i-1)];
                *Folder1 = i;
            }
        }
        else 
        {
            if(TimeBuffer[2*(i-1)+1] >= StartTimeCount)
            {
                if(TimeBuffer[2*(i-1)+1] < temp)
                {
                    temp = TimeBuffer[2*(i-1)+1];
                    *Folder1 = i;
                }
            }
        }
	}
    i++;
    if(i > 32)
    {
        i = 1;
    }
    if((TimeBuffer[2*(i-1)] > StartTimeCount)&&(TimeBuffer[2*(i-2)+1] < EndTimeCount))
    {
        *Folder2 = i;
    }
	//读取满足条件的时间列表（文件名列表）
	if((0 == *Folder1)&&(0 == *Folder2))//没有找到
	{
		return ;
	}
	else if(((0 != *Folder1)&&(0 != *Folder2))&&(*Folder1 != *Folder2))//找到两个文件夹
	{
		Folder[0] = *Folder1;
		Folder[1] = *Folder2;
		MediaFindFolder1 = *Folder1;
		MediaFindFolder2 = *Folder2;
		count3 = 2;
	}
	else if((0 != *Folder1)||(0 != *Folder2))//找到一个文件夹
	{
		if(0 != *Folder1)
		{
			Folder[0] = *Folder1;
			*Folder2 = 0;
		}
		else
		{
			*Folder1 = *Folder2;
			Folder[0] = *Folder1;
			*Folder2 = 0;
		}
		MediaFindFolder1 = *Folder1;
		MediaFindFolder2 = *Folder2;
		count3 = 1;
	}
	//文件总量计数
	count1 = 0;
	//获得目录
	for(i=0; i<count3; i++)
	{
		if(0 == i)
		{
			p = pBuffer1;
		}
		else
		{
			p = pBuffer2;
		}

		if(Folder[i] < 10)
		{
			Dir[6] = Folder[i]+0x30;
			Dir[7] = 0;
		}
		else
		{
			Dir[6] = Folder[i]/10+0x30;
			Dir[7] = Folder[i]%10+0x30;
			Dir[8] = 0;
		}
		length = strlen((char const *)Dir);
		//获得文件名
		strcpy((char *)FileName,(char const *)Dir);
		length = strlen((char const *)FileName);
		memcpy(FileName+length,"/log.bin",8);
		length += 8;
		FileName[length] = 0;
		//打开log文件
                IWDG_ReloadCounter();//喂狗
		if(FR_OK == f_open(&file1,ff_NameConver(FileName),FA_READ))
		{
			Count = 0;
			CountMax = file1.fsize/9;
			f_lseek(&file1,0);
			for(;;)
			{
				IWDG_ReloadCounter();
				Count++;
				if(Count > CountMax)
				{
					break;
				}
				//for(j=0; j<10000; j++)
				//{
				
				//}
				if(FR_OK == f_read (&file1, Buffer, 9, &Byte))
				{
					temp = 0;
					temp |= Buffer[0] << 24;
					temp |= Buffer[1] << 16;
					temp |= Buffer[2] << 8;
					temp |= Buffer[3];
					if((temp >= StartTimeCount)&&(temp <= EndTimeCount))
					{
						*p++ = Buffer[0];
						*p++ = Buffer[1];
						*p++ = Buffer[2];
						*p++ = Buffer[3];
						count1++;
						if(count1 >= MEDIA_SEARCH_MAX_FILE_NUM)
						{
							break;
						}
					}
					else if(temp > EndTimeCount)
					{
						break;
					}
				}
				else
				{
					break;
				}
			}
			*p++ = 0;
			*p++ = 0;
			*p++ = 0;
			*p++ = 0;
			
		}
		f_close(&file1);
		if(count1 >= MEDIA_SEARCH_MAX_FILE_NUM)
		{
			break ;
		}
	}
	
}
/*********************************************************************
//函数名称	:MediaSearch_GetMediaIDFileName
//功能		:获取多媒体ID对应的文件名
//输入		:多媒体ID号
//输出		:完整的文件名目录，包含盘符，目录
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//备注		:
*********************************************************************/
void MediaSearch_GetMediaIDFileName(u8 *pBuffer,u8 MediaType, u32 MediaID)
{
	u8	Dir[20];
	u8	FileName[_MAX_LFN];
	u8	Buffer[10];
	UINT	Byte;
	u32	temp;
	u8	length;
	u8	Folder[2];
	u8	i;
	///u32	j;
	FIL	file;
	TIME_T	tt;
	u16	CountMax;
	u16	Count;
	*pBuffer = 0;
	if(MEDIA_TYPE_JPG == MediaType)
	{
		strcpy((char *)Dir,"1:/JPG");
	}
	else if(MEDIA_TYPE_WAV == MediaType)
	{
		strcpy((char *)Dir,"1:/WAV");
	}
	else
	{
		return ;
	}
	Folder[0] = MediaFindFolder1;
	Folder[1] = MediaFindFolder2;
	//先从上一次检索的目录Folder1查找
	for(i=0; i<2; i++)
	{
		if(0 == Folder[i])
		{
			continue;
		}
		if((Folder[i] >= 1)&&(Folder[i] <= 32))
		{
			//获取日志文件名
			if(Folder[i] < 10)
			{
				Dir[6] = Folder[i]+0x30;
				Dir[7] = '/';
				Dir[8] = 0;
			}
			else 
			{
				Dir[6] = Folder[i]/10+0x30;
				Dir[7] = Folder[i]%10+0x30;
				Dir[8] = '/';
				Dir[9] = 0;
			}
			strcpy((char *)FileName,(char const *)Dir);
			strcat((char *)FileName,"log.bin");
			//打开日志文件
			if(FR_OK == f_open(&file,ff_NameConver(FileName),FA_READ))
			{
				Count = 0;
				CountMax = file.fsize/9;
				f_lseek(&file,0);
				for(;;)
				{
					Count++;
					if(Count >= CountMax)
					{
						break;
					}
                IWDG_ReloadCounter();//喂狗
					
					//for(j = 0; j<10000; j++)
					//{
				
					//}
					if(FR_OK == f_read (&file, Buffer, 9, &Byte))
					{
						temp = 0;
						temp |= Buffer[4] << 24;
						temp |= Buffer[5] << 16;
						temp |= Buffer[6] << 8;
						temp |= Buffer[7];
						if(temp == MediaID)
						{
							temp = 0;
							temp |= Buffer[0] << 24;
							temp |= Buffer[1] << 16;
							temp |= Buffer[2] << 8;
							temp |= Buffer[3];
							Gmtime(&tt, temp);
							strcpy((char *)FileName,(char const *)Dir);
							length = strlen((char const *)FileName);
							if(MEDIA_TYPE_JPG == MediaType)
							{
								FileName[length++] =  tt.day/10+0x30;
								FileName[length++] =  tt.day%10+0x30;
								FileName[length++] =  tt.hour/10+0x30;
								FileName[length++] =  tt.hour%10+0x30;
								FileName[length++] =  tt.min/10+0x30;
								FileName[length++] =  tt.min%10+0x30;
								FileName[length++] =  tt.sec/10+0x30;
								FileName[length++] =  tt.sec%10+0x30;
								FileName[length++] =  '.';
								FileName[length++] =  'j';
								FileName[length++] =  'p';
								FileName[length++] =  'g';
								FileName[length++] =  0;
							}
							else if(MEDIA_TYPE_WAV == MediaType)
							{
								FileName[length++] =  tt.month/10+0x30;
								FileName[length++] =  tt.month%10+0x30;
								FileName[length++] =  tt.day/10+0x30;
								FileName[length++] =  tt.day%10+0x30;
								FileName[length++] =  tt.hour/10+0x30;
								FileName[length++] =  tt.hour%10+0x30;
								FileName[length++] =  tt.min/10+0x30;
								FileName[length++] =  tt.min%10+0x30;
								FileName[length++] =  '.';
								FileName[length++] =  'w';
								FileName[length++] =  'a';
								FileName[length++] =  'v';
								FileName[length++] =  0;
							}
							strcpy((char  *)pBuffer,(char const *)FileName);
							return ;
						}
						else if(temp > MediaID)
						{
							break;
						}
					}
					else
					{
						break;
					}
				}
			}
		}
	}
	//再从所有的目录查找
	for(i=1; i<32; i++)
	{
		//获取日志文件名
		if(i < 10)
		{
			Dir[6] = i+0x30;
			Dir[7] = '/';
			Dir[8] = 0;
		}
		else 
		{
			Dir[6] = i/10+0x30;
			Dir[7] = i%10+0x30;
			Dir[8] = '/';
			Dir[9] = 0;
		}
		strcpy((char  *)FileName,(char const *)Dir);
		strcat((char  *)FileName,"log.bin");
		//打开日志文件
		if(FR_OK == f_open(&file,ff_NameConver(FileName),FA_READ))
		{
			Count = 0;
			CountMax = file.fsize/9;
			f_lseek(&file,0);
			for(;;)
			{
				//for(j = 0; j<10000; j++)
				//{
				
				//}
                IWDG_ReloadCounter();//喂狗
				Count++;
				//if(Count >= CountMax) dxl,2013.5.29
                                if(Count > CountMax)
				{
					break;
				}
				if(FR_OK == f_read (&file, Buffer, 9, &Byte))
				{
					temp = 0;
					temp |= Buffer[4] << 24;
					temp |= Buffer[5] << 16;
					temp |= Buffer[6] << 8;
					temp |= Buffer[7];
					if(temp == MediaID)
					{
						temp = 0;
						temp |= Buffer[0] << 24;
						temp |= Buffer[1] << 16;
						temp |= Buffer[2] << 8;
						temp |= Buffer[3];
						Gmtime(&tt, temp);
						strcpy((char *)FileName,(char const *)Dir);
						length = strlen((char const *)FileName);
						if(MEDIA_TYPE_JPG == MediaType)
						{
							FileName[length++] =  tt.day/10+0x30;
							FileName[length++] =  tt.day%10+0x30;
							FileName[length++] =  tt.hour/10+0x30;
							FileName[length++] =  tt.hour%10+0x30;
							FileName[length++] =  tt.min/10+0x30;
							FileName[length++] =  tt.min%10+0x30;
							FileName[length++] =  tt.sec/10+0x30;
							FileName[length++] =  tt.sec%10+0x30;
							FileName[length++] =  '.';
							FileName[length++] =  'j';
							FileName[length++] =  'p';
							FileName[length++] =  'g';
							FileName[length++] =  0;
						}
						else if(MEDIA_TYPE_WAV == MediaType)
						{
							FileName[length++] =  tt.month/10+0x30;
							FileName[length++] =  tt.month%10+0x30;
							FileName[length++] =  tt.day/10+0x30;
							FileName[length++] =  tt.day%10+0x30;
							FileName[length++] =  tt.hour/10+0x30;
							FileName[length++] =  tt.hour%10+0x30;
							FileName[length++] =  tt.min/10+0x30;
							FileName[length++] =  tt.min%10+0x30;
							FileName[length++] =  '.';
							FileName[length++] =  'w';
							FileName[length++] =  'a';
							FileName[length++] =  'v';
							FileName[length++] =  0;
						}
						strcpy((char *)pBuffer,(char const *)FileName);
						return ;
					}
				}
				else
				{
					break;
				}
			}
		}
	}
}
/*********************************************************************
//函数名称	:MediaSearch_GetMediaIndexInformation
//功能		:获取多媒体检索信息
//输入		:查找的起始时间，结束时间，多媒体类型
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//备注		:目前只支持按时间检索，不分事件类型，通道ID
*********************************************************************/
u16 MediaSearch_GetMediaIndexInformation(u8 *pBuffer, u8 MediaType, TIME_T *StartTime, TIME_T *EndTime)
{

	u8	Buffer1[4*MEDIA_SEARCH_MAX_FILE_NUM];
	u8	Buffer2[4*MEDIA_SEARCH_MAX_FILE_NUM];
	u8	Buffer[_MAX_LFN];
	u8	FileName[_MAX_LFN];
	u8	Folder1;
	u8	Folder2;
	u8	Folder[2];
	u8	count;
	u8	Dir[15];
	UINT	Byte;
	u32	temp;
	u8	i;
	u8	*p1 = NULL;
	static u8	*p = NULL;
	u16	length;
	TIME_T  tt;
	FIL	file;

	//查找获取文件夹及文件名
	MediaSearch_Find(&Folder1, &Folder2, Buffer1, Buffer2, MediaType, StartTime, EndTime);
	//初始化目录
	if(MEDIA_TYPE_JPG == MediaType)
	{
		strcpy((char *)Dir,"1:/JPG");
	}
	else if(MEDIA_TYPE_WAV == MediaType)
	{
		strcpy((char *)Dir,"1:/WAV");
	}
	//分类处理
	if((0 == Folder1)&&(0 == Folder2))//没有找到
	{
		return 0;
	}
	else if((0 != Folder1)&&(0 != Folder2)&&(Folder1 != Folder2))
	{
		count = 2;
		Folder[0] = Folder1;
		Folder[1] = Folder2;
	}
	else if(Folder1 != 0)//找到在一个文件夹
	{
		count = 1;
		Folder[0] = Folder1;
	}
	p = pBuffer;
	length = 0;
	for(i=0; i<count; i++)
	{
		if(Folder[i] < 10)
		{
			Dir[6] = Folder[i]+0x30;
			Dir[7] = '/';
			Dir[8] = 0;
		}
		else
		{
			Dir[6] = Folder[i]/10+0x30;
			Dir[7] = Folder[i]%10+0x30;
			Dir[8] = '/';
			Dir[9] = 0;
		}
		if(0 == i)
		{
			p1 = Buffer1;
		}
		else
		{
			p1 = Buffer2;
		}
		for(;;)
		{
                IWDG_ReloadCounter();//喂狗
			temp = 0;
			temp |= *p1 << 24;
			p1++;
			temp |= *p1 << 16;
			p1++;
			temp |= *p1 << 8;
			p1++;
			temp |= *p1;
			p1++;
			if(0 == temp)
			{
				break;
			}
			else
			{
				//转换时间
				Gmtime(&tt, temp);
				//获得文件名
				if(MEDIA_TYPE_JPG == MediaType)
				{
					Buffer[0] = tt.day/10+0x30;
					Buffer[1] = tt.day%10+0x30;
					Buffer[2] = tt.hour/10+0x30;
					Buffer[3] = tt.hour%10+0x30;
					Buffer[4] = tt.min/10+0x30;
					Buffer[5] = tt.min%10+0x30;
					Buffer[6] = tt.sec/10+0x30;
					Buffer[7] = tt.sec%10+0x30;
					Buffer[8] = '.';
					Buffer[9] = 'j';
					Buffer[10] = 'p';
					Buffer[11] = 'g';
					Buffer[12] = 0;
				}
				else if(MEDIA_TYPE_WAV == MediaType)
				{
					Buffer[0] = tt.month/10+0x30;
					Buffer[1] = tt.month%10+0x30;
					Buffer[2] = tt.day/10+0x30;
					Buffer[3] = tt.day%10+0x30;
					Buffer[4] = tt.hour/10+0x30;
					Buffer[5] = tt.hour%10+0x30;
					Buffer[6] = tt.min/10+0x30;
					Buffer[7] = tt.min%10+0x30;
					Buffer[8] = '.';
					Buffer[9] = 'w';
					Buffer[10] = 'a';
					Buffer[11] = 'v';
					Buffer[12] = 0;
				}
				strcpy((char *)FileName,(char const *)Dir);
				strcat((char *)FileName,(char const *)Buffer);
				//打开文件
				if(FR_OK == f_open(&file,ff_NameConver(FileName),FA_READ))
				{
					//偏移指针
					f_lseek(&file,file.fsize-36);
					//获取多媒体属性
					if(FR_OK == f_read (&file, Buffer, 36, &Byte))
					{
						//多媒体ID
						*p++ = Buffer[0];
						*p++ = Buffer[1];
						*p++ = Buffer[2];
						*p++ = Buffer[3];
						//多媒体类型
						*p++ = Buffer[4];
						//通道ID
						*p++ = Buffer[7];
						//事件编码
						*p++ = Buffer[6];
						//位置信息
						memcpy(p,Buffer+8,28);
						p += 28;
						length += 35;
					}
				}
				f_close(&file);
			}
		}
	}
	p = NULL;
	return length;
}


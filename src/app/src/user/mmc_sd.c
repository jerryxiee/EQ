#include "include.h"
//#include "sys.h" 
#include "mmc_sd.h"			   
#include "spi.h"
//#include "usart.h"	
		
void IntToAsc(u32 temp, u8 *buff) ; //将无符号整数转成十进制以8个字符输出
DWORD GetIntTime (u8 Time[6]) ;     //把Hex时间转成长整形
u8 CheckDayTime(u32 iStartTime,u32 iEndTime, u32 fileTime) ; //判断文件创建时间是否符合检索要求
u8 _SearchJPEG(u8 Event, u8 StartTime[6], u8 EndTime[6], u8 *Str,DIR *pdir)  ;  //按时间,事件检索
u8 _SearchWAV(u8 Event, u8 StartTime[6], u8 EndTime[6], u8 *Str,DIR *pdir)  ;  //按时间,事件检索
void SetupIndexFile(void)  ;  //建立索引文件
u8 CheckJPEGEvent(u8 Event, FILINFO *fno) ;  //判断文件事件是否符合检索要求
u8 CheckWAVEvent(u8 Event, FILINFO *fno) ;  //判断文件事件是否符合检索要求
void *MyMemcpy(void *s1,  const void *s2, size_t n, size_t s) ; //s2的数据拷到s1中,n为拷贝长度,s为方向(1:倒序,0:顺序)
u32 SearchFileSD( FIL *fp,u32 SIndex,u32 EIndex, u32 MeidaID) ; //通过媒体ID二分查找法文件序列号

u8 SD_Type=0;//SD卡的类型 
FATFS fs; 

extern u8	SdOnOffFlag;//SD卡在线标志；1在线，0不在线,当sd卡初始化失败时该标志也会清0
//T_FILE_MAG gFileMag;
////////////////////////////////////移植修改区///////////////////////////////////
//移植时候的接口
//data:要写入的数据
//返回值:读到的数据
u8 SD_SPI_ReadWriteByte(u8 data)
{
	return SPIx_ReadWriteByte(data);
}
//SD卡初始化的时候,需要低速
void SD_SPI_SpeedLow(void)
{
 	SPIx_SetSpeed(SPI_BaudRatePrescaler_256);//设置到低速模式	
}
//SD卡正常工作的时候,可以高速了
void SD_SPI_SpeedHigh(void)
{
 	//SPIx_SetSpeed(SPI_BaudRatePrescaler_2);//设置到高速模式	
	//SPIx_SetSpeed(SPI_BaudRatePrescaler_4);
	SPIx_SetSpeed(SPI_BaudRatePrescaler_8);
}
//SPI硬件层初始化
void SD_SPI_Init(void)
{    
	SPIx_Init();
	SD_CS_HIGH();
}
///////////////////////////////////////////////////////////////////////////////////
//取消选择,释放SPI总线
void SD_DisSelect(void)
{
	SD_CS_HIGH();
	SD_SPI_ReadWriteByte(0xff);//提供额外的8个时钟
}
//选择sd卡,并且等待卡准备OK
//返回值:0,成功;1,失败;
u8 SD_Select(void)
{
	SD_CS_LOW();
	if(SD_WaitReady()==0)return 0;//等待成功
	SD_DisSelect();
	return 1;//等待失败
}
//等待卡准备好
//返回值:0,准备好了;其他,错误代码
u8 SD_WaitReady(void)
{
	u32 t=0;
    
    if(0 == SdOnOffFlag)//dxl,2014.6.18sd卡不在线的话直接返回
    {
        return 1;
    }
    
	do
	{
		if(SD_SPI_ReadWriteByte(0XFF)==0XFF)return 0;//OK
		t++;	
        IWDG_ReloadCounter(); //需要喂狗,否则SD卡出现问题后可能会一直重启
	}while(t<0X7FFFF);//不能延时太短时间,至少应该为0x7FFF,此处取值为0X7FFFF
    if(0X7FFFF == t)
    {
        SdOnOffFlag = 0;//指示sd卡总线读写出错了,清除sd卡在线标志,等同于没有接sd卡,dxl,2014.6.18
    }
	return 1;
}
//等待SD卡回应
//Response:要得到的回应值
//返回值:0,成功得到了该回应值
//    其他,得到回应值失败
u8 SD_GetResponse(u8 Response)
{
	u16 Count=0xFFF;
    
    if(0 == SdOnOffFlag)//dxl,2014.6.18
    {
        return MSD_RESPONSE_FAILURE;
    }
	while ((SD_SPI_ReadWriteByte(0XFF)!=Response)&&Count)
    {
        Count--;//等待得到准确的回应  	
        IWDG_ReloadCounter();
    }
	if (Count==0)
    {
        SdOnOffFlag = 0;//指示sd卡总线读写出错了,清除sd卡在线标志,等同于没有接sd卡,dxl,2014.6.18
        return MSD_RESPONSE_FAILURE;
    }//得到回应失败  
	else 
    {   
        return MSD_RESPONSE_NO_ERROR;
    }//正确回应
}
//从sd卡读取一个数据包的内容
//buf:数据缓存区
//len:要读取的数据长度.
//返回值:0,成功;其他,失败;	
u8 SD_RecvData(u8*buf,u16 len)
{			  	  
	if(SD_GetResponse(0xFE))return 1;//等待SD卡发回数据起始令牌0xFE
    while(len--)//开始接收数据
    {
        *buf=SPIx_ReadWriteByte(0xFF);
        buf++;
    }
    //下面是2个伪CRC（dummy CRC）
    SD_SPI_ReadWriteByte(0xFF);
    SD_SPI_ReadWriteByte(0xFF);									  					    
    return 0;//读取成功
}
//向sd卡写入一个数据包的内容 512字节
//buf:数据缓存区
//cmd:指令
//返回值:0,成功;其他,失败;	
u8 SD_SendBlock(u8*buf,u8 cmd)
{	
	u16 t;		  	  
	if(SD_WaitReady())return 1;//等待准备失效
	SD_SPI_ReadWriteByte(cmd);
	if(cmd!=0XFD)//不是结束指令
	{
		for(t=0;t<512;t++)SPIx_ReadWriteByte(buf[t]);//提高速度,减少函数传参时间
	    SD_SPI_ReadWriteByte(0xFF);//忽略crc
	    SD_SPI_ReadWriteByte(0xFF);
		t=SD_SPI_ReadWriteByte(0xFF);//接收响应
		if((t&0x1F)!=0x05)return 2;//响应错误									  					    
	}						 									  					    
    return 0;//写入成功
}

//向SD卡发送一个命令
//输入: u8 cmd   命令 
//      u32 arg  命令参数
//      u8 crc   crc校验值	   
//返回值:SD卡返回的响应															  
u8 SD_SendCmd(u8 cmd, u32 arg, u8 crc)
{
    u8 r1;	
	u8 Retry=0; 
	SD_DisSelect();//取消上次片选
	if(SD_Select())return 0XFF;//片选失效 
	//发送
    SD_SPI_ReadWriteByte(cmd | 0x40);//分别写入命令
    SD_SPI_ReadWriteByte(arg >> 24);
    SD_SPI_ReadWriteByte(arg >> 16);
    SD_SPI_ReadWriteByte(arg >> 8);
    SD_SPI_ReadWriteByte(arg);	  
    SD_SPI_ReadWriteByte(crc); 
	if(cmd==CMD12)SD_SPI_ReadWriteByte(0xff);//Skip a stuff byte when stop reading
    //等待响应，或超时退出
	Retry=0X1F;
	do
	{
		r1=SD_SPI_ReadWriteByte(0xFF);
        IWDG_ReloadCounter();
	}while((r1&0X80) && Retry--);	 
	//返回状态值
    return r1;
}		    																			  
//获取SD卡的CID信息，包括制造商信息
//输入: u8 *cid_data(存放CID的内存，至少16Byte）	  
//返回值:0：NO_ERR
//		 1：错误														   
u8 SD_GetCID(u8 *cid_data)
{
    u8 r1;	   
    //发CMD10命令，读CID
    r1=SD_SendCmd(CMD10,0,0x01);
    if(r1==0x00)
	{
		r1=SD_RecvData(cid_data,16);//接收16个字节的数据	 
    }
	SD_DisSelect();//取消片选
	if(r1)return 1;
	else return 0;
}																				  
//获取SD卡的CSD信息，包括容量和速度信息
//输入:u8 *cid_data(存放CID的内存，至少16Byte）	    
//返回值:0：NO_ERR
//		 1：错误														   
u8 SD_GetCSD(u8 *csd_data)
{
    u8 r1;	 
    r1=SD_SendCmd(CMD9,0,0x01);//发CMD9命令，读CSD
    if(r1==0)
	{
    	r1=SD_RecvData(csd_data, 16);//接收16个字节的数据 
    }
	SD_DisSelect();//取消片选
	if(r1)return 1;
	else return 0;
}  
//获取SD卡的总扇区数（扇区数）   
//返回值:0： 取容量出错 
//       其他:SD卡的容量(扇区数/512字节)
//每扇区的字节数必为512，因为如果不是512，则初始化不能通过.														  
u32 SD_GetSectorCount(void)
{
    u8 csd[16];
    u32 Capacity;  
    u8 n;
	u16 csize;  					    
	//取CSD信息，如果期间出错，返回0
    if(SD_GetCSD(csd)!=0) return 0;	    
    //如果为SDHC卡，按照下面方式计算
    if((csd[0]&0xC0)==0x40)	 //V2.00的卡
    {	
		csize = csd[9] + ((u16)csd[8] << 8) + 1;
		Capacity = (u32)csize << 10;//得到扇区数	 		   
    }else//V1.XX的卡
    {	
		n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
		csize = (csd[8] >> 6) + ((u16)csd[7] << 2) + ((u16)(csd[6] & 3) << 10) + 1;
		Capacity= (u32)csize << (n - 9);//得到扇区数   
    }
    return Capacity;
}
u8 SD_Idle_Sta(void)
{
	u16 i;
	u8 retry;	   	  
    for(i=0;i<0xf00;i++);//纯延时，等待SD卡上电完成	 
    //先产生>74个脉冲，让SD卡自己初始化完成
    for(i=0;i<10;i++)SPIx_ReadWriteByte(0xFF); 
    //-----------------SD卡复位到idle开始-----------------
    //循环连续发送CMD0，直到SD卡返回0x01,进入IDLE状态
    //超时则直接退出
    retry = 0;
    do
    {	   
        //发送CMD0，让SD卡进入IDLE状态
        i = SD_SendCmd(CMD0, 0, 0x95);
        retry++;
        IWDG_ReloadCounter();
    }while((i!=0x01)&&(retry<200));
    //跳出循环后，检查原因：初始化成功？or 重试超时？
    if(retry==200)return 1; //失败
	return 0;//成功	 						  
}
//初始化SD卡
u8 SD_Initialize(void)
{
    u8 r1;      // 存放SD卡的返回值
    u16 retry;  // 用来进行超时计数
    u8 buf[4];  
	u16 i;

	//LOG_PR_N("SD_Initialize()\r\n");

	SD_SPI_Init();		//初始化IO
 	SD_SPI_SpeedLow();	//设置到低速模式 
    //for(i=0;i<0xf00;i++);//纯延时，等待SD卡上电完成	  
	for(i=0;i<10;i++)SD_SPI_ReadWriteByte(0XFF);//发送最少74个脉冲
	retry=20;
	do
	{
		r1=SD_SendCmd(CMD0,0,0x95);//进入IDLE状态
	}while((r1!=0X01) && retry--);
 	SD_Type=0;//默认无卡
	if(r1==0X01)
	{
		if(SD_SendCmd(CMD8,0x1AA,0x87)==1)//SD V2.0
		{
			for(i=0;i<4;i++)buf[i]=SD_SPI_ReadWriteByte(0XFF);	//Get trailing return value of R7 resp
			if(buf[2]==0X01&&buf[3]==0XAA)//卡是否支持2.7~3.6V
			{
				//retry=0XFFFE;
                retry = 0xfff;
				do
				{
					SD_SendCmd(CMD55,0,0X01);	//发送CMD55
					r1=SD_SendCmd(CMD41,0x40000000,0X01);//发送CMD41
                   IWDG_ReloadCounter();
				}while(r1&&retry--);
				if(retry&&SD_SendCmd(CMD58,0,0X01)==0)//鉴别SD2.0卡版本开始
				{
					for(i=0;i<4;i++)buf[i]=SD_SPI_ReadWriteByte(0XFF);//得到OCR值
					if(buf[0]&0x40)SD_Type=SD_TYPE_V2HC;    //检查CCS
					else SD_Type=SD_TYPE_V2;   
				}
			}
		}else//SD V1.x/ MMC	V3
		{
			SD_SendCmd(CMD55,0,0X01);		//发送CMD55
			r1=SD_SendCmd(CMD41,0,0X01);	//发送CMD41
			if(r1<=1)
			{		
				SD_Type=SD_TYPE_V1;
				//retry=0XFFFE;
                retry = 0xfff;
				do //等待退出IDLE模式
				{
					SD_SendCmd(CMD55,0,0X01);	//发送CMD55
					r1=SD_SendCmd(CMD41,0,0X01);//发送CMD41
                    IWDG_ReloadCounter();
				}while(r1&&retry--);
			}else
			{
				SD_Type=SD_TYPE_MMC;//MMC V3
				//retry=0XFFFE;
                retry = 0xfff;
				do //等待退出IDLE模式
				{											    
					r1=SD_SendCmd(CMD1,0,0X01);//发送CMD1
                    IWDG_ReloadCounter();               
				}while(r1&&retry--);  
			}
			if(retry==0||SD_SendCmd(CMD16,512,0X01)!=0)SD_Type=SD_TYPE_ERR;//错误的卡
		}
	}
	SD_DisSelect();//取消片选
	SD_SPI_SpeedHigh();//高速
	if(SD_Type)return 0;
	else if(r1)return r1; 	   
	return 0xaa;//其他错误
}
 
//读SD卡
//buf:数据缓存区
//sector:扇区
//cnt:扇区数
//返回值:0,ok;其他,失败.
u8 SD_ReadDisk(u8*buf,u32 sector,u8 cnt)
{
	u8 r1;
	if(SD_Type!=SD_TYPE_V2HC)sector <<= 9;//转换为字节地址
	if(cnt==1)
	{
		r1=SD_SendCmd(CMD17,sector,0X01);//读命令
		if(r1==0)//指令发送成功
		{
			r1=SD_RecvData(buf,512);//接收512个字节	   
		}
	}else
	{
		r1=SD_SendCmd(CMD18,sector,0X01);//连续读命令
		do
		{
			r1=SD_RecvData(buf,512);//接收512个字节	 
			buf+=512;  
		}while(--cnt && r1==0); 	
		SD_SendCmd(CMD12,0,0X01);	//发送停止命令
	}   
	SD_DisSelect();//取消片选
	return r1;//
}
//写SD卡
//buf:数据缓存区
//sector:起始扇区
//cnt:扇区数
//返回值:0,ok;其他,失败.
u8 SD_WriteDisk(u8*buf,u32 sector,u8 cnt)
{
	u8 r1;
	if(SD_Type!=SD_TYPE_V2HC)sector *= 512;//转换为字节地址
	if(cnt==1)
	{
		r1=SD_SendCmd(CMD24,sector,0X01);//读命令
		if(r1==0)//指令发送成功
		{
			r1=SD_SendBlock(buf,0xFE);//写512个字节	   
		}
	}else
	{
		if(SD_Type!=SD_TYPE_MMC)
		{
			SD_SendCmd(CMD55,0,0X01);	
			SD_SendCmd(CMD23,cnt,0X01);//发送指令	
		}
 		r1=SD_SendCmd(CMD25,sector,0X01);//连续读命令
		if(r1==0)
		{
			do
			{
				r1=SD_SendBlock(buf,0xFC);//接收512个字节	 
				buf+=512;  
			}while(--cnt && r1==0);
			r1=SD_SendBlock(0,0xFD);//接收512个字节 
		}
	}   
	SD_DisSelect();//取消片选
	return r1;//
}	   

//获得剩余空间(KB)
//返回值:0失败   其他:剩余字节数
DWORD GetRestKByte(void)
{
    DWORD fre_clust;
    FATFS *fs;
    if( f_getfree(ff_NameConver("1:"),&fre_clust,&fs) == FR_OK )
    {
        return fre_clust * fs->csize /2;       
    }
    else
    {
        return 0;
    }
}

/*******************************************************
//功能:将无符号整数转成十进制以8个字符输出
//输入参数：整数,转换后的字符串(正序)
//返回：最大整数99999999, 以8个字符输出 
//例子: 0X1346 -> "00004936"  
*******************************************************/
void IntToAsc(u32 temp, u8 *buff) 
{
	u8 i,j;
	const u32 DivTbl[]={1,1,10,100,1000,10000,100000,1000000,10000000};
	for(i=8,j=0; i>0; i--)
	{
		buff[j++]=temp/DivTbl[i]+'0';
		temp=temp%DivTbl[i];
	}
    if(j>=8)
        buff[j]='\0';
}
/*******************************************************
Description:建立索引文件
Input: 
Output:
Return: 
note:    
*******************************************************/
void SetupIndexFile(void) 
{
	//LOG_PR_N("SetupIndexFile()\r\n");
	
/*
    FIL file;
    u8 buffer[9]={1,0,0,0,1,0,0,0,2};
    u32 Bw;     
    //WAV目录下创建文件,保存最早的文件名  FA_CREATE_NEW
    {   
        if(f_open(&file,"1:/WAV/INDEX.TXT",FA_CREATE_NEW|FA_READ|FA_WRITE) != FR_EXIST)
        { //文件不存在,创建文件
            f_close(&file);
            if(f_open(&file,"1:/WAV/INDEX.TXT",FA_CREATE_ALWAYS|FA_READ|FA_WRITE) == FR_OK)  
            {                       
               f_write(&file,buffer,sizeof(buffer),&Bw);                          
            } 
            f_close(&file);

            if(f_open(&file,"1:/WAV/INDEXBAK.TXT",FA_CREATE_ALWAYS|FA_READ|FA_WRITE) == FR_OK)  
            {                       
               f_write(&file,buffer,sizeof(buffer),&Bw);                          
            }                                
        }
        f_close(&file);  
    }
    {   
        if(f_open(&file,"1:/JPEG/INDEX.TXT",FA_CREATE_NEW|FA_READ|FA_WRITE) != FR_EXIST)
        { //文件不存在,创建文件
            f_close(&file);
            if(f_open(&file,"1:/JPEG/INDEX.TXT",FA_CREATE_ALWAYS|FA_READ|FA_WRITE) == FR_OK)  
            {                       
               f_write(&file,buffer,sizeof(buffer),&Bw);                          
            } 
            f_close(&file);

            if(f_open(&file,"1:/JPEG/INDEXBAK.TXT",FA_CREATE_ALWAYS|FA_READ|FA_WRITE) == FR_OK)  
            {                       
               f_write(&file,buffer,sizeof(buffer),&Bw);                          
            }                      
        }
        f_close(&file);
    }
*/
	
}
/*******************************************************
Description:SD文件初始化
Input: 
Output:
Return: 
note:    
*******************************************************/  
void InitSDFile(void) 
{
    FRESULT Result;
    //DIR dir;
    //FILINFO fileInfo;

	//LOG_PR_N("InitSDFile()\r\n");
    //创建目录1:WAV
    Result = f_mkdir(ff_NameConver("1:/WAV"));
    if(Result != FR_OK)
    {
        //SdOnOffFlag = 0;//dxl,2014.6.18,认为SD卡坏了
        if(Result == FR_EXIST)
        { //存在该目录,删除里面的文件  
            ;
#if 0            
            if(f_opendir(&dir, "1:/WAV") == FR_OK)   //打开目录成功
            {
                while(1)
                {
                    u8 Buffer[25]="1:/WAV/";
                    
                    IWDG_ReloadCounter();             
                    Result = f_readdir(&dir,&fileInfo);
                    if(Result != FR_OK || fileInfo.fname[0]==0)break;
                    if(fileInfo.fname[0]=='.')continue  ;
                    strcat((char *)Buffer,fileInfo.fname);
                    Result = f_unlink((char const *)Buffer);       //删除文件
                    if(Result != FR_OK) break;                   
                }
            } 
#endif
        }
        else
        {
            SdOnOffFlag = 0;//dxl,2014.8.25,认为SD卡坏了,原2014.6.18的修改有bug
        }
    }


    //创建目录1:JPEG
    Result = f_mkdir(ff_NameConver("1:/JPEG"));
    if(Result != FR_OK)
    {
        //SdOnOffFlag = 0;//dxl,2014.6.18,认为SD卡坏了
        if(Result == FR_EXIST)
        { //存在该目录,删除里面的文件  
            ;
#if 0  
            if(f_opendir(&dir, "1:/JPEG") == FR_OK)   //打开目录成功
            {
                while(1)
                {
                    u8 Buffer[25]="1:/JPEG/";
                    
                  IWDG_ReloadCounter();         
                    Result = f_readdir(&dir,&fileInfo);
                    if(Result != FR_OK || fileInfo.fname[0]==0)break;
                    if(fileInfo.fname[0]=='.')continue  ;
                    strcat((char *)Buffer,fileInfo.fname);
                    Result = f_unlink((char const *)Buffer);       //删除文件
                    if(Result != FR_OK) break;                  
                }
            } 
#endif            
        }
        else
        {
            SdOnOffFlag = 0;//dxl,2014.8.25,原2014.6.18的修改有bug
        }
    } 
    SetupIndexFile();
}

/*******************************************************
Description:获得图像文件名,Str空间不能小于25
Input: 
Output: 文件名
Return: 1:成功  0:失败
note:    
*******************************************************/ 
#define MAX_REST_SPACE   (100000) //KBYTES
#define MIN_REST_SPACE   (1000)   //KBYTES

//#define MAX_REST_SPACE   (3860580-1000) //DEBUG
//#define MIN_REST_SPACE   (3860580-2100)   //DEBUG
/*
u8 FetchJPEGFileName(u8 *Str) 
{
    FIL file,fileBak;  
    u8 Buffer[15],i;   
    u8 FileName[25];
    u32 Br,Bw;  
    u32 Result;   
    u32 StartIndex; //起始索引号  从1开始
    u32 EndIndex;   //结束索引号  从1开始
    
    u32 CStartIndex; //起始索引号 从0开始
    u32 CEndIndex;   //结束索引号 从0开始  
    //参数检测
    if(Str == NULL)
        return 0;
        
    //获得头尾文件序列号
    if(f_open(&file,"1:/JPEG/INDEX.TXT",FA_OPEN_ALWAYS|FA_READ|FA_WRITE) != FR_OK)  
        return 0;           
    f_read(&file,Buffer,9,&Br);   
    
    f_open(&fileBak,"1:/JPEG/INDEXBAK.TXT",FA_OPEN_ALWAYS|FA_READ|FA_WRITE);       
    Buffer[9] = 0; //借用Buffer[9],做校验和处理
    for(i=0; i<8; i++)
    {
        Buffer[9] += Buffer[i];
    }
    if(Buffer[9] != Buffer[8])
    { //校验出错
        f_read(&fileBak,Buffer,9,&Br);           
    }  
    
    StartIndex = *(u32 *)&Buffer[0]; 
    EndIndex =  *(u32 *)&Buffer[4];     
    CStartIndex = StartIndex - 1;
    CEndIndex = EndIndex - 1;
    Result = EndIndex;            //获得返回文件索引ID      
    
    
    if( (CEndIndex+1)%MAX_FILES_SUM == CStartIndex || GetRestKByte() < MIN_REST_SPACE) 
    { //索引满了或空间不够           
        do
        {   
            if(CEndIndex == CStartIndex)
            {//空了
                f_close(&file);
                f_close(&fileBak);  
                return 0;    
            }
            //空间过小,需要删除文件                          
            IntToAsc(CStartIndex+1,Buffer);
            strcpy((char *)FileName,"1:/JPEG/");
            strcat((char *)FileName,(char const *)Buffer);   
            strcat((char *)FileName,".JPG");       
            f_unlink((char const *)FileName);                       //删除文件腾出空间                
            CStartIndex = (CStartIndex+1)%MAX_FILES_SUM;
            //IWDG_ReloadCounter();dxl,2013.11.21,去掉喂狗,SD卡驱动有问题,会导致程序一直死在SD的某个驱动里面
        }
        while(GetRestKByte() < MAX_REST_SPACE );
    }
    CEndIndex = (CEndIndex+1)%MAX_FILES_SUM;
    
    StartIndex = CStartIndex + 1;
    EndIndex = CEndIndex + 1;
    {
        u8 i;
        for(i=0; i<4; i++)
            Buffer[i] =  *((u8 *)&StartIndex +i);
        for(i=0; i<4; i++)
            Buffer[i+4] =  *((u8 *)&EndIndex +i);   
            
        Buffer[8] = 0; 
        for(i=0; i<8; i++)
        {
            Buffer[8] += Buffer[i]; 
        }
                    
        f_lseek(&file,0);      
        f_lseek(&fileBak,0);     
        f_write(&file,Buffer,9,&Bw);
        f_write(&fileBak,Buffer,9,&Bw); 
    }
    f_close(&file);
    f_close(&fileBak); 
    IntToAsc(Result,Buffer);

    strcpy((char *)FileName,"1:/JPEG/");
    strcat((char *)FileName,(char const *)Buffer);   
    strcat((char *)FileName,".JPG");                         
    strcpy((char *)Str,(char const *)FileName);
    return 1;
}
*/
/*********************************************************************
//函数名称	:FetchJPEGFileName
//功能		:获取文件名
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
u8  FetchJPEGFileName(u8 *pBuffer)
{
	TIME_T  tt;
	RTC_GetCurTime(&tt);
	u8 *p;
	u8 length;

	//LOG_PR_N("FetchJPEGFileName()\r\n");
	
	p = pBuffer;
	length = 0;
	
	//根目录路径
	*p++ = '1';
	length++;
	*p++ = ':';
	length++;
	*p++ = '/';
	length++;
	*p++ = 'J';
	length++;
	*p++ = 'P';
	length++;
	*p++ = 'E';
	length++;
	*p++ = 'G';
	length++;
	*p++ = '/';
	length++;
	
	//文件名
	*p++ = tt.year/10+0x30;
	length++;
	*p++ = tt.year%10+0x30;
	length++;
	*p++ = tt.month/10+0x30;
	length++;
	*p++ = tt.month%10+0x30;
	length++;
	*p++ = tt.day/10+0x30;
	length++;
	*p++ = tt.day%10+0x30;
	length++;
	*p++ = tt.hour/10+0x30;
	length++;
	*p++ = tt.hour%10+0x30;
	length++;
	*p++ = tt.min/10+0x30;
	length++;
	*p++ = tt.min%10+0x30;
	length++;
	*p++ = tt.sec/10+0x30;
	length++;
	*p++ = tt.sec%10+0x30;
	length++;
	//后缀
	*p++ = '.';
	length++;
	*p++ = 'j';
	length++;
	*p++ = 'p';
	length++;
	*p++ = 'e';
	length++;
	*p++ = 'g';
	length++;
	//结尾字符
	*p++ = 0;
	length++;
		
	return length;
}
/*******************************************************
Description:获得录音文件名,Str空间不能小于25
Input: 
Output: 文件名
Return: 1:成功  0:失败
note:    
*******************************************************/ 
/*
u8 FetchWAVFileName(u8 *Str) 
{
    FIL file,fileBak;  
    u8 Buffer[15],i;  
    u8 FileName[25];
    u32 Br,Bw;  
    u32 Result;   
    u32 StartIndex; //起始索引号  从1开始
    u32 EndIndex;   //结束索引号  从1开始
    
    u32 CStartIndex; //起始索引号 从0开始
    u32 CEndIndex;   //结束索引号 从0开始  
    //参数检测
    if(Str == NULL)
        return 0;
      
    //获得头尾文件序列号
    if(f_open(&file,"1:/WAV/INDEX.TXT",FA_OPEN_ALWAYS|FA_READ|FA_WRITE) != FR_OK)  
        return 0;           
    f_read(&file,Buffer,9,&Br);   
    
    f_open(&fileBak,"1:/WAV/INDEXBAK.TXT",FA_OPEN_ALWAYS|FA_READ|FA_WRITE);      
    Buffer[9] = 0; //借用Buffer[9],做校验和处理
    for(i=0; i<8; i++)
    {
        Buffer[9] += Buffer[i];
    }
    if(Buffer[9] != Buffer[8])
    { //校验出错
        f_read(&fileBak,Buffer,9,&Br);           
    }  
    
    StartIndex = *(u32 *)&Buffer[0]; 
    EndIndex =  *(u32 *)&Buffer[4];     
    CStartIndex = StartIndex - 1;
    CEndIndex = EndIndex - 1;
    Result = EndIndex;            //获得返回文件索引ID      
    
    
    if( (CEndIndex+1)%MAX_FILES_SUM == CStartIndex || GetRestKByte() < MIN_REST_SPACE) 
    { //索引满了或空间不够           
        do
        {   
            if(CEndIndex == CStartIndex)
            {//空了
                f_close(&file);
                f_close(&fileBak);  
                return 0;    
            }
            //空间过小,需要删除文件                          
            IntToAsc(CStartIndex+1,Buffer);
            strcpy((char *)FileName,"1:/WAV/");
            strcat((char *)FileName,(char const *)Buffer);   
            strcat((char *)FileName,".WAV");       
            f_unlink((char const *)FileName);                       //删除文件腾出空间                
            CStartIndex = (CStartIndex+1)%MAX_FILES_SUM;
            IWDG_ReloadCounter(); dxl,2013.11.21,去掉喂狗,SD卡驱动有问题,会导致程序一直死在SD的某个驱动里面
        }
        while(GetRestKByte() < MAX_REST_SPACE );
    }
    CEndIndex = (CEndIndex+1)%MAX_FILES_SUM;
    
    StartIndex = CStartIndex + 1;
    EndIndex = CEndIndex + 1;
    {
        u8 i;
        for(i=0; i<4; i++)
            Buffer[i] =  *((u8 *)&StartIndex +i);
        for(i=0; i<4; i++)
            Buffer[i+4] =  *((u8 *)&EndIndex +i);   
            
        Buffer[8] = 0;  
        for(i=0; i<8; i++)
        {
            Buffer[8] += Buffer[i]; 
        }
                    
        f_lseek(&file,0);      
        f_lseek(&fileBak,0);      
        f_write(&file,Buffer,9,&Bw);
        f_write(&fileBak,Buffer,9,&Bw); 
    }
    f_close(&file);
    f_close(&fileBak);  
    IntToAsc(Result,Buffer);

    strcpy((char *)FileName,"1:/WAV/");
    strcat((char *)FileName,(char const *)Buffer);   
    strcat((char *)FileName,".WAV");                         
    strcpy((char *)Str,(char const *)FileName);
    return 1;
}
*/
u8  FetchWAVFileName(u8 *pBuffer)
{
	
	TIME_T  tt;
	RTC_GetCurTime(&tt);
	u8 *p;
	u8 length;

	//LOG_PR_N("FetchWAVFileName()\r\n");
	
	p = pBuffer;
	length = 0;
	
	//根目录路径
	*p++ = '1';
	length++;
	*p++ = ':';
	length++;
	*p++ = '/';
	length++;
	*p++ = 'W';
	length++;
	*p++ = 'A';
	length++;
	*p++ = 'V';
	length++;
	*p++ = '/';
	length++;
	
	//文件名
	*p++ = tt.year/10+0x30;
	length++;
	*p++ = tt.year%10+0x30;
	length++;
	*p++ = tt.month/10+0x30;
	length++;
	*p++ = tt.month%10+0x30;
	length++;
	*p++ = tt.day/10+0x30;
	length++;
	*p++ = tt.day%10+0x30;
	length++;
	*p++ = tt.hour/10+0x30;
	length++;
	*p++ = tt.hour%10+0x30;
	length++;
	*p++ = tt.min/10+0x30;
	length++;
	*p++ = tt.min%10+0x30;
	length++;
	*p++ = tt.sec/10+0x30;
	length++;
	*p++ = tt.sec%10+0x30;
	length++;
	//后缀
	*p++ = '.';
	length++;
	*p++ = 'w';
	length++;
	*p++ = 'a';
	length++;
	*p++ = 'v';
	length++;
	//结尾字符
	*p++ = 0;
	length++;
		
	return length;
}
/*******************************************************
Description:把Hex时间转成长整形
Input: Hex,Year-Month-Day-Hour-Minte-Second
Output:
Return: 长整形时间值
notye:    
*******************************************************/ 
DWORD GetIntTime (u8 Time[6])
{
    u8 Temp;
    DWORD iDayTime; 
    
    iDayTime = Time[0] + 2000 - 1980; //Year
    iDayTime = iDayTime<<25 ;
    Temp = Time[1];                   //Month
    iDayTime |= (Temp<<21);
    Temp = Time[2];                   //Day
    iDayTime |= (Temp<<16);
    Temp = Time[3];                   //Hour
    iDayTime |= (Temp<<11);
    Temp = Time[4];                   //Min
    iDayTime |= (Temp<<5);
    Temp = Time[5];                   //Sec
    iDayTime |= (Temp>>1);
	return iDayTime;
}

/*******************************************************
Description:判断文件创建时间是否符合检索要求
Input: 开始时间,结束时间,文件创建时间
Output:
Return: 1:符合检索要求 0:不符合
note:    
*******************************************************/ 
u8 CheckDayTime(u32 iStartTime,u32 iEndTime, u32 fileTime) 
{
    if(iStartTime < fileTime && iEndTime > fileTime)
    {
        return 1;    
    }
    else
    {
        return 0;
    }
}

/*******************************************************
Description:判断文件事件是否符合检索要求
Input: 事件,文件信息
Output:
Return: 1:符合检索要求 0:不符合
note:    
*******************************************************/ 
u8 CheckJPEGEvent(u8 Event, FILINFO *fno) 
{
    FIL file;
    u8 Buffer[40]={0};
    u32 Br;
    u8  FileName_1[_MAX_LFN];
    
    strcpy((char *)Buffer,"1:/JPEG/");
    ff_NameForward(FileName_1,fno->fname);
    strcat((char *)Buffer,(const char *)FileName_1);
    if(f_open(&file,ff_NameConver(Buffer),FA_OPEN_EXISTING|FA_READ) == FR_OK) //只读
    {                                          
        f_lseek(&file,file.fsize-sizeof(T_MEDIA_RECORD));
        f_read(&file,Buffer,sizeof(T_MEDIA_RECORD),&Br);
        if(Buffer[7] == Event)
        {
            f_close(&file);
            return 1;    
        }              
    }  
    f_close(&file);      
    return 0;
}
/*******************************************************
Description:判断文件事件是否符合检索要求
Input: 事件,文件信息
Output:
Return: 1:符合检索要求 0:不符合
note:    
*******************************************************/ 
u8 CheckWAVEvent(u8 Event, FILINFO *fno) 
{
    FIL file;
    u8 Buffer[40]={0};
    u32 Br;
    u8  FileName_1[_MAX_LFN];
    strcpy((char *)Buffer,"1:/WAV/");
    ff_NameForward(FileName_1,fno->fname);
    strcat((char *)Buffer,(const char *)FileName_1);
    if(f_open(&file,ff_NameConver(Buffer),FA_OPEN_EXISTING|FA_READ) == FR_OK)           //只读
    {                                          
        f_lseek(&file,file.fsize-sizeof(T_MEDIA_RECORD));
        f_read(&file,Buffer,sizeof(T_MEDIA_RECORD),&Br);
        if(Buffer[7] == Event)
        {
            f_close(&file);
            return 1;    
        }              
    }  
    f_close(&file);      
    return 0;
}

/*******************************************************
Description:由文件序列号获得绝对文件名
Input: 文件序列号码如:"0000000N",
Output: FullFileName
Return: NULL
note: filename不能小于9字节
      FullFileName不能小于25字节
*******************************************************/ 
void GetFullJPEGFileName(u8* filename)
{
    u8 Buff[25];
    if(strstr((char const *)filename, "1:") == NULL) 
    {//不包含字符串"1:"
        strcpy((char *)Buff,"1:/JPEG/");
        strcat((char *)Buff,(char const *)filename);
        if(strstr((char const *)filename, ".JPG") == NULL)
        { //没有找到".JPG"
            strcat((char *)Buff,".JPG");
            strcpy((char *)filename,(char const *)Buff);
        }
    }       
}
/*******************************************************
Description:由文件序列号获得绝对文件名
Input: 文件序列号码如:"0000000N",
Output: FullFileName
Return: NULL
note: filename不能小于25字节     
*******************************************************/ 
void GetFullWAVFileName(u8* filename) 
{
    u8 Buff[25];
    if(strstr((char const *)filename, "1:") == NULL) 
    {//不包含字符串"1:"
        strcpy((char *)Buff,"1:/WAV/");
        strcat((char *)Buff,(char const *)filename);
        if(strstr((char const *)filename, ".WAV") == NULL)
        { //没有找到".WAV"
            strcat((char *)Buff,".WAV");
            strcpy((char *)filename,(char const *)Buff);
        }
    }       
}
/*******************************************************
Description:获取文件记录
Input: 完整文件名称,必须包含路径,文件记录空间
Output:
Return: 1:成功 0:失败
note: 
*******************************************************/    
u8 GetFileRecord(u8 *filename, T_MEDIA_RECORD *Rec)
{
    FIL file;
    FRESULT Result;
    u32 Br;
    u8 Buf[4];
    if(filename == NULL || Rec == NULL)
        return 0;
        
    if(f_open(&file,ff_NameConver(filename),FA_OPEN_EXISTING|FA_READ) == FR_OK)           //只读
    {                                          
        Result = f_lseek( &file, file.fsize-sizeof(T_MEDIA_RECORD) );
        if(Result == FR_OK)
        {
            Result = f_read( &file,(u8 *)Rec,sizeof(T_MEDIA_RECORD),&Br );
            if(Result == FR_OK)
            {
                MyMemcpy(Buf, (u8 *)Rec, 4 , 1);  //T_MEDIA_RECORD的MediaID字段是大端存储,需要调整
                Rec->MediaID = *(u32 *)Buf;
                f_close(&file);
                return 1;                
            }        
        }            
    }  
    f_close(&file);      
    return 0;    
}

//-----------------------------------------------
//功能:s2的数据拷到s1中,n为拷贝长度,s为方向(1:倒序,0:顺序)
//输入:
//输出:
//备注:
//-----------------------------------------------
void *MyMemcpy(void *s1,  const void *s2, size_t n, size_t s)
{       /* copy char s2[n] to s1[n] in any order */
    char *su1 = (char *)s1;
    const char *su2 = (const char *)s2;
    u8 i;
    if(s==0)
    {
        for (; 0 < n; ++su1, ++su2, --n)
          *su1 = *su2;
    }
    else
    {
        for(i=0 ;i<n; i++)
        {
            *(su1+n-1-i) = *(su2+i);
        }	
    }
    return (s1);
}

/*******************************************************
//功能:按时间,事件检索
//输入参数：Event:事件类型, StartTime[6],EndTime[6]:YY-
//MM-DD-hh-mm-ss(Hex),Str返回的文件序列号,不能小于9字节
//返回：文件序列号长度,0表示检索完成
//注意
如果Event=0XFF, 表示仅仅按时间检索
如果 StartTime[0],EndTime[0] 有一个为0,表示仅仅按事件检索,
符合要求的文件ID通过Str输出,如: “0000000N”,每调用一次输
出一个文件序列,直到返回0
*******************************************************/
u8 _SearchJPEG(u8 Event, u8 StartTime[6], u8 EndTime[6], u8 *Str,DIR *pdir)  
{
    
    FILINFO fileInfo;
    u32 iStartTime,iEndTime,fileTime;
    FRESULT Result;
    static u8 SearchCnt=0;
    u8  FileName_1[_MAX_LFN];
    
    //参数检测
    if(Str == NULL)
        return 0;    
    
    while(1) 
    {
        IWDG_ReloadCounter();
        Result = f_readdir(pdir,&fileInfo);    //读目录信息到文件状态结构中
        if(Result != FR_OK || fileInfo.fname[0]==0)break;
        ff_NameForward(FileName_1,fileInfo.fname);
        if(FileName_1[0]=='.')continue  ;
        if(strstr((const char *)FileName_1, "INDEX") != NULL) continue;	//包含INDEX字符串的文件跳过
        SetOnceTimer(SEARCH_FILE_TIMER,50*2);  //2s  
        iStartTime = GetIntTime(StartTime);
        iEndTime   = GetIntTime(EndTime);
        fileTime = (fileInfo.fdate <<16) | fileInfo.ftime; //得到文件创建时间
        if(Event == 0XFF)
        {
            if(StartTime[0] && EndTime[0])
            { //按时间检索                
                if(CheckDayTime(iStartTime,iEndTime, fileTime))
                {
                    ff_NameForward(FileName_1,fileInfo.fname);
                    strncpy((char *)Str,(const char *)FileName_1,8);    
                    Str[8] = '\0';
                    if(++SearchCnt < 11)   
                    {
                        return strlen((char const*)Str);
                    }
                    else
                    {
                        SearchCnt=0;
                        break;
                    }
                }
                continue;//继续检索
            }    
            else
            {
                break;           //检索条件错误    
            }           
        }
        else
        {
            if(StartTime[0] && EndTime[0])
            { //按时间\事件检索
                if( !CheckDayTime(iStartTime,iEndTime, fileTime) )
                {
                    continue;//继续检索
                }
                else
                {
                    if(CheckJPEGEvent(Event, &fileInfo))
                    {
                        ff_NameForward(FileName_1,fileInfo.fname);
                        strncpy((char *)Str,(const char *)FileName_1,8);    
                        Str[8] = '\0';
                        if(++SearchCnt < 11)   
                        {
                            return strlen((char const*)Str);
                        }
                        else
                        {
                            SearchCnt=0;
                            break;
                        }                                                  
                    }   
                    continue;//继续检索
                }
                
            }    
            else
            { //按事件检索
                if(CheckJPEGEvent(Event, &fileInfo))
                {
                     ff_NameForward(FileName_1,fileInfo.fname);
                    strncpy((char *)Str,(const char *)FileName_1,8);    
                    Str[8] = '\0';
                    if(++SearchCnt < 11)   
                    {
                        return strlen((char const*)Str);
                    }
                    else
                    {
                        SearchCnt=0;
                        break;
                    }                                            
                }   
                continue;//继续检索                               
            } 
            
        }
    }
    return 0;
}            
/*******************************************************
//功能:按时间,事件检索
//输入参数：Event:事件类型, StartTime[6],EndTime[6]:YY-
//MM-DD-hh-mm-ss(Hex),Str返回的文件序列号,不能小于9字节
//返回：文件序列号长度,0表示检索完成
//注意
如果Event=0XFF, 表示仅仅按时间检索
如果 StartTime[0],EndTime[0] 有一个为0,表示仅仅按事件检索,
符合要求的文件ID通过Str输出,如: “0000000N”,每调用一次输
出一个文件序列,直到返回0
*******************************************************/
u8 _SearchWAV(u8 Event, u8 StartTime[6], u8 EndTime[6], u8 *Str,DIR *pdir)  
{
    
    FILINFO fileInfo;
    u32 iStartTime,iEndTime,fileTime;
    FRESULT Result;
    static u8 SearchCnt=0;
    u8 FileName_1[_MAX_LFN];
    
    //参数检测
    if(Str == NULL)
        return 0;    
    
    while(1) 
    {
       IWDG_ReloadCounter();
        Result = f_readdir(pdir,&fileInfo);    //读目录信息到文件状态结构中
        if(Result != FR_OK || fileInfo.fname[0]==0)break;
        ff_NameForward(FileName_1,fileInfo.fname);
        if(FileName_1[0]=='.')continue  ;
        if(strstr((const char *)FileName_1,"INDEX") != NULL) continue;	//包含INDEX字符串的文件跳过
        SetOnceTimer(SEARCH_FILE_TIMER,50*2);  //2s  
        iStartTime = GetIntTime(StartTime);
        iEndTime   = GetIntTime(EndTime);
        fileTime = (fileInfo.fdate <<16) | fileInfo.ftime; //得到文件创建时间
        if(Event == 0XFF)
        {
            if(StartTime[0] && EndTime[0])
            { //按时间检索                
                if(CheckDayTime(iStartTime,iEndTime, fileTime))
                {
                    ff_NameForward(FileName_1,fileInfo.fname);
                    strncpy((char *)Str,(const char *)FileName_1,8);    
                    Str[8] = '\0';
                    if(++SearchCnt < 11)   
                    {
                        return strlen((char const*)Str);
                    }
                    else
                    {
                        SearchCnt=0;
                        break;
                    }
                }
                continue;//继续检索
            }    
            else
            {
                break;           //检索条件错误    
            }           
        }
        else
        {
            if(StartTime[0] && EndTime[0])
            { //按时间\事件检索
                if( !CheckDayTime(iStartTime,iEndTime, fileTime) )
                {
                    continue;//继续检索
                }
                else
                {
                    if(CheckWAVEvent(Event, &fileInfo))
                    {
                         ff_NameForward(FileName_1,fileInfo.fname);
                        strncpy((char *)Str,(const char *)FileName_1,8);    
                        Str[8] = '\0';
                        if(++SearchCnt < 11)   
                        {
                            return strlen((char const*)Str);
                        }
                        else
                        {
                            SearchCnt=0;
                            break;
                        }                                                
                    }   
                    continue;//继续检索
                }
                
            }    
            else
            { //按事件检索
                if(CheckWAVEvent(Event, &fileInfo))
                {
                    ff_NameForward(FileName_1,fileInfo.fname);
                    strncpy((char *)Str,(const char *)FileName_1,8);    
                    Str[8] = '\0';                    
                    if(++SearchCnt < 11)   
                    {
                        return strlen((char const*)Str);
                    }
                    else
                    {
                        SearchCnt=0;
                        break;
                    }                  
                }   
                continue;//继续检索                               
            } 
            
        }
    }
    return 0;
} 
/*******************************************************
//功能:按时间,事件检索
//输入参数：Event:事件类型, StartTime[6],EndTime[6]:YY-
//MM-DD-hh-mm-ss(Hex),Str返回的文件序列号,不能小于9字节
//返回：文件序列号长度,0表示检索完成
//注意
如果Event=0XFF, 表示仅仅按时间检索
如果 StartTime[0],EndTime[0] 有一个为0,表示仅仅按事件检索,
符合要求的文件ID通过Str输出,如: “0000000N”,每调用一次输
出一个文件序列,直到返回0
*******************************************************/
u8 SearchJPEG(u8 Event, u8 StartTime[6], u8 EndTime[6], u8 *Str) 
{
    static u8 Status=0;
    static DIR dir;    
    u8 Result;
    
    
    //参数检测
    if(Str == NULL)
        return 0;
        
    if( IsOverTime(SEARCH_FILE_TIMER) )
        Status = 0;                           //检索文件超时,重新检索
        
    if(Status == 0)
    {
        if(f_opendir(&dir, ff_NameConver("1:/JPEG")) != FR_OK)   
            return 0;                        //打开目录失败
        Status = 1;  
        Result =  _SearchJPEG(Event,StartTime,EndTime,Str,&dir);
        if( !Result )
        {
            Status = 0;       
        }
    }     
    else
    {
        Result =  _SearchJPEG(Event,StartTime,EndTime,Str,&dir);
        if( !Result )
        {
            Status = 0;       
        }       
    } 
    return Result; 
}
/*******************************************************
//功能:按时间,事件检索
//输入参数：Event:事件类型, StartTime[6],EndTime[6]:YY-
//MM-DD-hh-mm-ss(Hex),Str返回的文件序列号,不能小于9字节
//返回：文件序列号长度,0表示检索完成
//注意
如果Event=0XFF, 表示仅仅按时间检索
如果 StartTime[0],EndTime[0] 有一个为0,表示仅仅按事件检索,
符合要求的文件ID通过Str输出,如: “0000000N”,每调用一次输
出一个文件序列,直到返回0
*******************************************************/
u8 SearchWAV(u8 Event, u8 StartTime[6], u8 EndTime[6], u8 *Str) 
{
    static u8 Status=0;
    static DIR dir;
    u8 Result;
    
    
    //参数检测
    if(Str == NULL)
        return 0;
        
    if( IsOverTime(SEARCH_FILE_TIMER) )
        Status = 0;                           //检索文件超时,重新检索
        
    if(Status == 0)
    {
        if(f_opendir(&dir, ff_NameConver("1:/WAV")) != FR_OK)   
            return 0;                        //打开目录失败
        Status = 1;  
        Result =  _SearchWAV(Event,StartTime,EndTime,Str,&dir);
        if( !Result )
        {
            Status = 0;       
        }
    }     
    else
    {
        Result =  _SearchWAV(Event,StartTime,EndTime,Str,&dir);
        if( !Result )
        {
            Status = 0;       
        }       
    } 
    return Result; 
}

/*******************************************************
//描述:在保存完JPEG文件后,需要调用该函数,目的是建立MediaID
//与文件序列号的对应关系,方便通过MediaID查找文件.
//功能:保存JPEG ID号
//输入参数：JPEG ID
//输出参数: NULL
//返回:1:成功  0:失败
*******************************************************/
u8 SaveJPEGMediaID(u32 MediaID)
{
/*
	//LOG_PR_N("SaveJPEGMediaID()\r\n");   
    u8 err=0; 
    FIL file;
    u32 Brw;
    u8 Buffer[15];
    u32 Index;      //最后一个文件的索引号 
  
    do
    {    
        if(f_open(&file,"1:/JPEG/INDEX.TXT",FA_OPEN_ALWAYS|FA_READ|FA_WRITE) == FR_OK)  
        {                                          
            f_read(&file,Buffer,8,&Brw);
            //StartIndex = *(u32 *)&Buffer[0]; 
            Index =  *(u32 *)&Buffer[4] - 1;   
            if(Index == 0)
                Index = MAX_FILES_SUM;
            
        }
        else
        {
            err = 1;
            break;
        } 
        if( f_lseek(&file,512+(Index-1)*sizeof(u32)) != FR_OK)  //跳过头尾索引号,Index从1开始
        {
            err = 1; 
            break;
        }
        if(f_write(&file,(u8 *)&MediaID,sizeof(u32),&Brw) != FR_OK)    //在对应索引号写入MediaID
        {
            err = 1;
            break;
        }
    }
    while(0);
    f_close(&file);
    if(err)
        return 0;
    else
        return 1; 
*/
   return 1;
	
}
/*******************************************************
//描述:在保存完WAV文件后,需要调用该函数,目的是建立MediaID
//与文件序列号的对应关系,方便通过MediaID查找文件.
//功能:保存WAV ID号
//输入参数：WAV ID
//输出参数: NULL
//返回:1:成功  0:失败
*******************************************************/
u8 SaveWAVMediaID(u32 MediaID) 
{
	//LOG_PR_N("SaveWAVMediaID()\r\n"); 
/*
    u8 err=0; 
    FIL file;
    u32 Brw;
    u8 Buffer[15];
    u32 Index;      //最后一个文件的索引号    
  
    do
    {    
        if(f_open(&file,"1:/WAV/INDEX.TXT",FA_OPEN_ALWAYS|FA_READ|FA_WRITE) == FR_OK)  
        {                                          
            f_read(&file,Buffer,8,&Brw);
            //StartIndex = *(u32 *)&Buffer[0]; 
            Index =  *(u32 *)&Buffer[4] - 1;                                       
            if(Index == 0)
                Index = MAX_FILES_SUM;
        }
        else
        {
            err = 1;
            break;
        } 
        if( f_lseek(&file,512+(Index-1)*sizeof(u32)) != FR_OK)  //跳过头尾索引号,Index从1开始
        {
            err = 1; 
            break;
        }
        if(f_write(&file,(u8 *)&MediaID,sizeof(u32),&Brw) != FR_OK)    //在对应索引号写入MediaID
        {
            err = 1;
            break;
        }
    }
    while(0);
    f_close(&file);
    if(err)
        return 0;
    else
        return 1; 
*/
    return 1;
}
/*******************************************************
//功能:通过媒体ID二分查找法文件序列号
//输入参数：文件指针,起始索引,结束索引,带查找的媒体ID
//输出参数:NULL
//返回:0:未找到 其他:文件序列号
*******************************************************/
u32 SearchFileSD( FIL *fp,u32 SIndex,u32 EIndex, u32 MeidaID) 
{
    u32 lo = SIndex;
    u32 hi = EIndex;
    u32 mid;   //折半值
    u32 ID;    //检索到的ID
    u32 Temp;
    u32 Brw;
    
    while(lo < hi)
    {
        mid =(hi + lo)/2;       
        Temp = mid % MAX_FILES_SUM;
        f_lseek(fp,512+sizeof(u32)*(Temp-1));              
        if(f_read(fp,(u8 *)&ID,sizeof(u32),&Brw) != FR_OK)
        {
            return 0; //读文件失败
        }                                                       
        if(MeidaID < ID)
            hi=mid;
        else if(MeidaID > ID)
            lo=mid+1;
        else        
            return mid % MAX_FILES_SUM;         
    }
    return 0;
}
/*******************************************************
//功能:用JPEG ID获得文件名
//输入参数：JPEG ID
//输出参数: 绝对路径文件名
//返回:1:成功  0:失败,或者没有该JPEG ID
*******************************************************/
u8 SearchJPEGWithID(u32 MediaID, u8 FileName[25]) 
{
    u8 err=0; 
    FIL file;
    u32 Brw;
    u8 Buffer[15];
    u32 StartIndex;    //第一个文件的索引号    
    u32 EndIndex;      //最后一个文件的索引号    
    u32 FileSD;        //文件序列号
  
    do
    {    
        if(f_open(&file,ff_NameConver("1:/JPEG/INDEX.TXT"),FA_OPEN_ALWAYS|FA_READ|FA_WRITE) == FR_OK)  
        {                                              
            if(f_read(&file,Buffer,8,&Brw) == FR_OK)
            {
                StartIndex = *(u32 *)&Buffer[0]; 
                EndIndex =  *(u32 *)&Buffer[4];
            }
            else
            {
                err = 1;
                break;
            }                                                  
        }
        else
        {
            err = 1;
            break;
        } 
        if(StartIndex > EndIndex)
        { //物理地址转成虚拟地址
            EndIndex += MAX_FILES_SUM;            
        }
        //二分法查找索引
        FileSD = SearchFileSD( &file,StartIndex,EndIndex,MediaID);
    }
    while(0);
    f_close(&file);
    if(err || FileSD==0)
        return 0;
    else
    {
        IntToAsc(FileSD,Buffer); //将整形数转成字符串
        strcpy((char *)FileName,"1:/JPEG/");
        strcat((char *)FileName,(char const *)Buffer);   
        strcat((char *)FileName,".JPG");                     
        return 1; 
    }
}
/*******************************************************
//功能:用WAV ID获得文件名
//输入参数：WAV ID
//输出参数: 绝对路径文件名
//返回:1:成功  0:失败,或者没有该WAV ID
*******************************************************/
u8 SearchWAVWithID(u32 MediaID, u8 FileName[25]) 
{
    u8 err=0; 
    FIL file;
    u32 Brw;
    u8 Buffer[15];
    u32 StartIndex;    //第一个文件的索引号    
    u32 EndIndex;      //最后一个文件的索引号    
    u32 FileSD;        //文件序列号
  
    do
    {    
        if(f_open(&file,ff_NameConver("1:/WAV/INDEX.TXT"),FA_OPEN_ALWAYS|FA_READ|FA_WRITE) == FR_OK)  
        {                                              
            if(f_read(&file,Buffer,8,&Brw) == FR_OK)
            {
                StartIndex = *(u32 *)&Buffer[0]; 
                EndIndex =  *(u32 *)&Buffer[4];
            }
            else
            {
                err = 1;
                break;
            }                                             
        }
        else
        {
            err = 1;
            break;
        } 
        if(StartIndex > EndIndex)
        { //物理地址转成虚拟地址
            EndIndex += MAX_FILES_SUM;   
        }
        //二分法查找索引
        FileSD = SearchFileSD( &file,StartIndex,EndIndex,MediaID);
    }
    while(0);
    f_close(&file);
    if(err || FileSD==0)
        return 0;
    else
    {
        IntToAsc(FileSD,Buffer); //将整形数转成字符串
        strcpy((char *)FileName,"1:/WAV/");
        strcat((char *)FileName,(char const *)Buffer);   
        strcat((char *)FileName,".WAV");                     
        return 1; 
    }
}


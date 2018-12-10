#ifndef	__SUBPACKAGE_H
#define	__SUBPACKAGE_H

//*************文件包含***************
#include "stm32f2xx_gpio.h"
#include "stm32f2xx.h"
#include "ff.h"
//*************宏定义****************
#define SUBPACKET_LIST_BUFFER_SIZE    1000

#define MEDIT_TYPE_PIC   0x10
#define MEDIT_TYPE_SOUND 0x11


#define SUBPACKET_ID_LIST_SIZE_MEDIA  100 
typedef struct tagSTT_SUBPACKETTRANSTER_CTRL{
u8  FileName[100];
FIL FileHandler ; 
u8  MediaType ;
u16 PacketCntTotal ;
u16 PacketCntCursor ;

u16 FirstSN ; //原始消息流水号
u8  PacketIDCntTotal ;//重传总包数
u8  PacketIDCntCursor ;//重传ID列表游标 
u8  PacketIDArray[ SUBPACKET_ID_LIST_SIZE_MEDIA ];//重传包ID列表数组

u8  DataBuf[512+36];
u16 DataBufLength;
u8  ResendCnt ; 

}STT_SUBPACKETTRANSTER_CTRL;




extern STT_SUBPACKETTRANSTER_CTRL g_sttSubPacketTransterCtrl ;
/*********************************************************************
//函数名称	:Subpackage_UploadTimeTask(void)
//功能		:分包补传定时任务（对应平台命令0x8003）
*********************************************************************/
FunctionalState Subpackage_UploadTimeTask(void);



/*********************************************************************
//函数名称 : Subpackage_Media_File_Name_Set
//函数功能 : 给多媒体分包补传的任务，控制需要补传的文件名， 如果文件名超出允许的长度，返回0，否则返回实际的设置的字符串长度（不含尾字节'\0')
//函数示例 :  Subpackage_Media_File_Name_Set( "1:/JPG1/15155028.jpg" ); 
*********************************************************************/
u8 Subpackage_Media_File_Name_Set(  const char * pInMediaFileName  );

/*********************************************************************
//函数名称 : Subpackage_Task_Media(void)
//函数功能 : 多媒体分包补传任务，含打开文件，取数，关闭文件，打包数据，往平台上传数据，以及修正下一分包ID 
*********************************************************************/
void Subpackage_Task_Media( void );



#endif
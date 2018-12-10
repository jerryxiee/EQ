/********************************************************************
//版权说明	:Shenzhen E-EYE Co. Ltd., Copyright 2009- All Rights Reserved.
//文件名称	:downloadGB2313.c
//功能		:存储与查询下发国标汉字
//版本号	:V0.1
//开发人	:dxl
//开发时间	:2010.06
//修改者	:
//修改时间	:
//修改简要说明	:
//备注		:
***********************************************************************/

//********************************头文件************************************
#include "include.h"

//********************************自定义数据类型****************************

//********************************宏定义************************************

//********************************全局变量**********************************

//********************************外部变量********************************** 
extern u8 s_ucMsgFlashBuffer[];

#define ATTEMPE_STEP_LEN	  1000

typedef struct
{
    s16 CurrentStep;	//当前步数
    s16 SectorStep;	//单个扇区总步数
    s16 LoopCount;	//循环存储标志
    s16 CurrentSector;	//当前时间扇区
    s16 OldestSector;	//最老时间扇区
    s16 StartSector;	//存储地址开始扇区
    s16 EndSector;	//存储地址结束扇区
}STFLASH_AREA_STATUS;
//********************************本地变量**********************************
static STFLASH_AREA_STATUS 	s_stTextAreaStatus;	//FLASH区域状态变量

#define ONE_SECTORSTEP_NUM (unsigned char)(FLASH_ONE_SECTOR_BYTES/ATTEMPE_STEP_LEN)
#define MAX_DOWN_NUM (unsigned char)((FLASH_ATTEMPE_END_SECTOR-FLASH_ATTEMPE_START_SECTOR)*ONE_SECTORSTEP_NUM)
//********************************函数声明**********************************

//********************************函数定义***********************************
/*********************************************************************
//函数名称	:DownloadGB2313_CheckFlashArea(void)
//功能		:检查下发国标汉字数据区域
//输入		:
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:  
//返回		:无
//备注		:
*********************************************************************/
ErrorStatus   DownloadGB2313_CheckFlashArea(void)
{
    s16 	j,k;
    u32 	Address;
    u8  	TimeChar[7];
    TIME_T  tt;
    u32 	MaxTimeCount;
    u32	MinTimeCount;
    u32 	TimeCount;

    //初始化相关变量
    s_stTextAreaStatus.StartSector = FLASH_ATTEMPE_START_SECTOR;
    s_stTextAreaStatus.OldestSector = FLASH_ATTEMPE_START_SECTOR;
    s_stTextAreaStatus.EndSector = FLASH_ATTEMPE_END_SECTOR;
    s_stTextAreaStatus.SectorStep = FLASH_ONE_SECTOR_BYTES/ATTEMPE_STEP_LEN;
    s_stTextAreaStatus.CurrentStep = 0;
    s_stTextAreaStatus.CurrentSector = FLASH_ATTEMPE_START_SECTOR;
    s_stTextAreaStatus.LoopCount = 0;
    //初始化最大值为0
    MaxTimeCount = 0;
    MinTimeCount = 0xFFFFFFFF;

    //确定当前步数,当前扇区,最老扇区
    //从开始扇区--->结束扇区
    for(k=FLASH_ATTEMPE_START_SECTOR; k<FLASH_ATTEMPE_END_SECTOR; k++)
    {
        #ifdef WATCHDOG_OPEN
        //喂狗
        IWDG_ReloadCounter();
        #endif
        //从扇区第0步--->扇区末尾
        for(j=0; j<s_stTextAreaStatus.SectorStep; j++)
        {
            //读取时间信息
            Address = k*FLASH_ONE_SECTOR_BYTES+j*ATTEMPE_STEP_LEN;
            sFLASH_ReadBuffer(TimeChar, Address, 5);

            //小端排序
            TimeCount = 0;
            TimeCount |= TimeChar[0] << 24;
            TimeCount |= TimeChar[1] << 16;
            TimeCount |= TimeChar[2] << 8;
            TimeCount |= TimeChar[3];

            //检查是否找到有效的时间标签
            if(TimeCount != 0xFFFFFFFF)	//找到有效时间标签
            {
                //转换成结构体时间
                Gmtime(&tt, TimeCount);

                //判定时间最老扇区
                if(TimeCount < MinTimeCount)
                {
                    MinTimeCount = TimeCount;
                    s_stTextAreaStatus.OldestSector = k; //最老扇区
                }

                //判定时间最新扇区
                if(TimeCount > MaxTimeCount)
                {
                    MaxTimeCount = TimeCount;
                    s_stTextAreaStatus.CurrentStep = j+1;//最新步数
                    s_stTextAreaStatus.CurrentSector = k;//最新扇区号
                }

            }
            else	//没有找到有效时间标签，跳出循环查找下一个扇区
            {
                break; 
            }
        }

    }
    if(s_stTextAreaStatus.CurrentStep >= s_stTextAreaStatus.SectorStep)
    {
        s_stTextAreaStatus.CurrentStep = 0;
        s_stTextAreaStatus.CurrentSector++;
        if(s_stTextAreaStatus.CurrentSector >= s_stTextAreaStatus.EndSector)
        {
            s_stTextAreaStatus.CurrentSector = s_stTextAreaStatus.StartSector;
        }
        Address = s_stTextAreaStatus.CurrentSector*FLASH_ONE_SECTOR_BYTES;
        sFLASH_EraseSector(Address);
    }

    //*************读最后一步的值，以确定是否进入循环存储*********
    Address = (s_stTextAreaStatus.EndSector-1)*FLASH_ONE_SECTOR_BYTES + (s_stTextAreaStatus.SectorStep-1)*ATTEMPE_STEP_LEN;
    sFLASH_ReadBuffer(TimeChar, Address, 6);
    if((0xff != TimeChar[0])||(0xff != TimeChar[1]))
    {
        s_stTextAreaStatus.LoopCount = 1;
    }
    return SUCCESS;
}
/*********************************************************************
//函数名称	:EraseDownloadGB2313FlashArea()
//功能		:擦除下发国标汉字区域
//		:
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void EraseDownloadGB2313FlashArea(void)
{
    u16 i;
    u32 Address;
    for(i=FLASH_ATTEMPE_START_SECTOR; i<FLASH_ATTEMPE_END_SECTOR; i++)
    {
        #ifdef WATCHDOG_OPEN
        //每检查一个扇区喂狗一次
        IWDG_ReloadCounter();
        #endif
        Address = i*FLASH_ONE_SECTOR_BYTES;
        sFLASH_EraseSector(Address);
    }
}
/*********************************************************************
//函数名称	:DownloadGB2313_WriteToFlash(u8 *pBuffer, u16 length)
//功能		:写下发国标汉字到FLASH
//输入		:pBuffer，数据指针
//		:length，数据长度
//输出		:
//使用资源	:
//全局变量	:
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void DownloadGB2313_WriteToFlash(u8 *pBuffer, u16 length)
{
    u32	Address;
    u16	i;
    u8	flag;
    u32	TimeCount;
    u8	BcdTime[6];
    u8	BcdTimeRead[6];

    //判断长度是否超出范围
    if(length > 1023)
    {
        return ;
    }

    //判断当前扇区是否超出范围
    if((s_stTextAreaStatus.CurrentSector >= FLASH_ATTEMPE_END_SECTOR)||(s_stTextAreaStatus.CurrentSector < FLASH_ATTEMPE_START_SECTOR))
    {
        return ;
    }

    //判断当前步数是否超出范围
    if((s_stTextAreaStatus.CurrentStep < 0)||(s_stTextAreaStatus.CurrentStep >= s_stTextAreaStatus.SectorStep))
    {
        return ;
    }

    //获取当前地址
    Address = s_stTextAreaStatus.CurrentSector*FLASH_ONE_SECTOR_BYTES+s_stTextAreaStatus.CurrentStep*ATTEMPE_STEP_LEN;

    //写之前读取FLASH的值，判断是否为0xFF
    sFLASH_ReadBuffer(s_ucMsgFlashBuffer, Address, ATTEMPE_STEP_LEN);
    flag = 0;
    for(i=0; i<ATTEMPE_STEP_LEN; i++)
    {
        if(0xff != s_ucMsgFlashBuffer[i])
        {
            flag = 1;
            break;
        }
    }
    if(0 != flag)
    {
        s_stTextAreaStatus.CurrentStep = 0;
        sFLASH_EraseSector(Address);
        return ;
    }

    flag = 0;
    //写当前时间和数据长度
    TimeCount = RTC_GetCounter();
    BcdTime[0] = TimeCount >> 24;
    BcdTime[1] = TimeCount >> 16;
    BcdTime[2] = TimeCount >> 8;
    BcdTime[3] = TimeCount;
    BcdTime[4] = (length & 0xff00) >> 8;
    BcdTime[5] = length & 0xff;
    sFLASH_WriteBuffer(BcdTime, Address, 6);

    //读刚写入的时间和长度
    sFLASH_ReadBuffer(BcdTimeRead, Address, 6);


    //判断读取的与写入的是否一致
    for(i=0; i<6; i++)
    {
        if(BcdTime[i] != BcdTimeRead[i])
        {
            flag = 1;
        }
    }


    if(0 != flag)
    {
        s_stTextAreaStatus.CurrentStep = 0;
        sFLASH_EraseSector(Address);
        return ;
    }

    //写数据
    sFLASH_WriteBuffer(pBuffer, Address+6, length);

    //读取刚写入的数据
    //sFLASH_ReadBuffer(s_ucMsgFlashBuffer, Address+6, length);

    //步数下移
    s_stTextAreaStatus.CurrentStep++;
    if(s_stTextAreaStatus.SectorStep == s_stTextAreaStatus.CurrentStep)
    {
        s_stTextAreaStatus.CurrentStep = 0;
        s_stTextAreaStatus.CurrentSector++;
        if(s_stTextAreaStatus.EndSector == s_stTextAreaStatus.CurrentSector)
        {
            s_stTextAreaStatus.CurrentSector = s_stTextAreaStatus.StartSector;
            s_stTextAreaStatus.LoopCount = 1;
        }
        Address = s_stTextAreaStatus.CurrentSector*FLASH_ONE_SECTOR_BYTES;
        sFLASH_EraseSector(Address);
    }
}
/*********************************************************************
//函数名称	:DownloadGB2313_ReadFromFlash(u8 Num, u8 *pBuffer)
//功能		:从FLASH里按序号读取下发国标汉字信息
//输入		:Num,序号。最新的那条信息为序号1，次新的那条信息为序号2，依此类推
//输出		:
//使用资源	:
//全局变量	: 
//调用函数	:
//中断资源	:  
//返回		:pBuffer长度
//备注		:
*********************************************************************/
u8 DownloadGB2313_ReadFromFlash(u8 Num, u8 *pBuffer)
{
    u8	TimeBuffer[7];
    u8	i;
    u16	length = 0;
    u32	Address;
    s16	SearchStep;
    s16	SearchSector;

    //最多只能存储6条
    if(Num >= MAX_DOWN_NUM)
    {
        return 0;//序号出错，当前最多只能查询6条信息
    }

    //从当前步数往前推Num步
    SearchStep = s_stTextAreaStatus.CurrentStep;
    SearchSector = s_stTextAreaStatus.CurrentSector;
    for(i=0; i<Num; i++)
    {
        SearchStep--;
        if(SearchStep < 0)
        {
            SearchStep = s_stTextAreaStatus.SectorStep - 1;
            SearchSector--;
            if(SearchSector < s_stTextAreaStatus.StartSector)
            {
                if(s_stTextAreaStatus.LoopCount > 0)
                {
                    SearchSector = s_stTextAreaStatus.EndSector - 1;
                }
                else
                {
                    return 0;//没有相应的记录，直接退出
                }
            }
        }	
    }

    //读取时间信息
    Address = SearchSector*FLASH_ONE_SECTOR_BYTES+SearchStep*ATTEMPE_STEP_LEN;
    sFLASH_ReadBuffer(TimeBuffer, Address, 6);

    //判断是否为有效数据
    if((0xff == TimeBuffer[0])||(0xff == TimeBuffer[4]))
    {
        return 0;
    }

    //读取国标汉字信息
    length = 0;
    length |= TimeBuffer[4] << 8;
    length |= TimeBuffer[5];
    if(length > 1023)
    {
        length = 1023;
    }
    sFLASH_ReadBuffer(pBuffer, Address+6, length);

    return length;

}
/*********************************************************************
//函数名称	:DownloadGB2313_GetTotalStep(void)
//功能		:获取下发国标汉字已存入的总步数
//输入		:
//输出		:
//使用资源	:
//全局变量	: 
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
u8 DownloadGB2313_GetTotalStep(void)
{
    u8	TotalStep = 0;

    if(0 == s_stTextAreaStatus.LoopCount)	//没有进入循环存储
    {
        TotalStep = (s_stTextAreaStatus.CurrentSector - s_stTextAreaStatus.StartSector)
        *s_stTextAreaStatus.SectorStep + 	s_stTextAreaStatus.CurrentStep;
    }
    else	//已进入循环存储
    {
        TotalStep = MAX_DOWN_NUM-s_stTextAreaStatus.SectorStep + s_stTextAreaStatus.CurrentStep;
    }

    return TotalStep;
}
/*******************************************************************************
 *                             end of module
 *******************************************************************************/


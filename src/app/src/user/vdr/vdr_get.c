/*
********************************************************************************
*
*
* Filename      : vdr_get.c
* Version       : V1.00
* Programmer(s) : miaoyahan
* Time          : 20140722
********************************************************************************
*/

/*
********************************************************************************
*                               INCLUDES
********************************************************************************
*/
#include "Vdr.h"
/*
********************************************************************************
*                             LOCAL DEFINES
********************************************************************************
*/

#ifdef USE_ONE_MIN_SAVE_SPEED_RECORD
#define VDR_SPEED_START_SECTOR	     588//行驶速度记录
#define VDR_SPEED_END_SECTOR	     692//416K byte
#else
#define	VDR_SPEED_START_SECTOR		 224//行驶速度记录
#define VDR_SPEED_END_SECTOR		 692//1868K byte
#endif

#define	VDR_POSITION_START_SECTOR	 693//位置信息记录
#define VDR_POSITION_END_SECTOR		 864//512K byte

#define	VDR_DOUBT_START_SECTOR		 865//疑点数据记录
#define VDR_DOUBT_END_SECTOR		 875//32K byte

#define	VDR_OVER_START_SECTOR		 876//超时驾驶记录
#define VDR_OVER_END_SECTOR		     880//12K byte

#define	VDR_DRIVER_START_SECTOR		 881//驾驶人身份记录
#define VDR_DRIVER_END_SECTOR		 885//12K byte

#define	VDR_POWER_START_SECTOR		 886//供电记录
#define VDR_POWER_END_SECTOR		 889//8K byte

#define	VDR_VALUE_START_SECTOR		 890//参数修改记录
#define VDR_VALUE_END_SECTOR		 893//8K byte

#define	VDR_SPDLOG_START_SECTOR      894//速度状态记录
#define VDR_SPDLOG_END_SECTOR	     897//8K byte


#ifdef USE_ONE_MIN_SAVE_SPEED_RECORD
#define	VDR_SPEED_STEP_LEN           144 //时间4+每分钟的行驶速度块126+预留13+校验1	
#else
#define	VDR_SPEED_STEP_LEN           11	 //时间4+行驶速度及状态2+预留4+校验1
#endif

#define	VDR_POSITION_STEP_LEN        24	 //时间4+位置信息10+平均速度1+里程4+预留4+校验1
#define	VDR_DOUBT_STEP_LEN           251 //时间4+疑点数据234+预留12+校验1
#define	VDR_OVER_STEP_LEN            67  //时间4+超时驾驶50+预留12+校验1
#define	VDR_DRIVER_STEP_LEN          36  //时间4+驾驶人身份25+预留6+校验1
#define	VDR_POWER_STEP_LEN           18  //时间4+外部供电7+预留6+校验1
#define	VDR_VALUE_STEP_LEN           18  //时间4+参数修改7+预留6+校验1
#define	VDR_SPDLOG_STEP_LEN          144 //时间4+速度状态133+预留6+校验1


#define	VDR_SPEED_UP_LEN             126	 
#define	VDR_POSITION_UP_LEN          666	 
#define	VDR_DOUBT_UP_LEN             234 
#define	VDR_OVER_UP_LEN              50  
#define	VDR_DRIVER_UP_LEN            25  
#define	VDR_POWER_UP_LEN             7 
#define	VDR_VALUE_UP_LEN             7
#define	VDR_SPDLOG_UP_LEN            133


/*
********************************************************************************
*                         LOCAL DATA TYPES
********************************************************************************
*/



/*
********************************************************************************
*                          CONST VARIABLES
********************************************************************************
*/
const u16 VdrSecAddr[]=//扇区地址
{
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
VDR_SPEED_START_SECTOR,
VDR_SPEED_END_SECTOR,
VDR_POSITION_START_SECTOR,
VDR_POSITION_END_SECTOR,
VDR_DOUBT_START_SECTOR,
VDR_DOUBT_END_SECTOR,
VDR_OVER_START_SECTOR,
VDR_OVER_END_SECTOR,
VDR_DRIVER_START_SECTOR,
VDR_DRIVER_END_SECTOR,
VDR_POWER_START_SECTOR,
VDR_POWER_END_SECTOR,
VDR_VALUE_START_SECTOR,
VDR_VALUE_END_SECTOR,
VDR_SPDLOG_START_SECTOR,
VDR_SPDLOG_END_SECTOR,
};
const u16 VdrStepLen[]=//单步长度
{
0,0,0,0,0,0,0,0,
VDR_SPEED_STEP_LEN,
VDR_POSITION_STEP_LEN,
VDR_DOUBT_STEP_LEN,
VDR_OVER_STEP_LEN,
VDR_DRIVER_STEP_LEN,
VDR_POWER_STEP_LEN,
VDR_VALUE_STEP_LEN,
VDR_SPDLOG_STEP_LEN,
};
const u16 VdrUpLen[]=//上传长度
{
0,0,0,0,0,0,0,0,
VDR_SPEED_UP_LEN, 
VDR_POSITION_UP_LEN, 
VDR_DOUBT_UP_LEN,
VDR_OVER_UP_LEN,
VDR_DRIVER_UP_LEN, 
VDR_POWER_UP_LEN,
VDR_VALUE_UP_LEN,
VDR_SPDLOG_UP_LEN,
};

const VDR_FUC_BASE Vdr_Fuc_Base[]=//采集基本信息
{
 Vdr_Get_GbtVer,
 Vdr_Get_DvrInfor,
 Vdr_Get_Rtc,
 Vdr_Get_Mileage,
 Vdr_Get_Pulse,
 Vdr_Get_CarInfor,
 Vdr_Get_StaSignal,
 Vdr_Get_OnlyNum,
};

/*
********************************************************************************
*                          LOCAL VARIABLES
********************************************************************************
*/
static u8  Vdr_PackBuf[VDR_PACKAGE_SIZE];//分包缓存

/*
********************************************************************************
*                          GLOBAL VARIABLES
********************************************************************************
*/


/*
********************************************************************************
*                           FUNCTIONS EXTERN
********************************************************************************
*/

/*
********************************************************************************
*                              FUNCTIONS
********************************************************************************
*/

/**
  * @brief  记录仪协议执行年号
  * @param  None
  * @retval None
  */
void Vdr_Get_GbtVer(VDR_HEAD *pHead)
{
    pHead->pbuf[0] = 0x12;
    pHead->pbuf[1] = 0x00;
    pHead->len     = 2;

}
/**
  * @brief  驾驶员信息
  * @param  None
  * @retval None
  */
void Vdr_Get_DvrInfor(VDR_HEAD *pHead)
{
    pHead->len = 18;
    memset(pHead->pbuf,0,pHead->len);
    Vdr_Fuc_Mem[VDR_MEM_DVRINFOR](VDR_MEM_READ,pHead->pbuf);
}
/**
  * @brief  实时时间
  * @param  None
  * @retval None
  */
void Vdr_Get_Rtc(VDR_HEAD *pHead)
{
    pHead->len = 6;
    memset(pHead->pbuf,0,pHead->len);
    Vdr_Fuc_Mem[VDR_MEM_RTC](VDR_MEM_READ,pHead->pbuf);
}
/**
  * @brief  累计里程
  * @param  None
  * @retval None
  */
void Vdr_Get_Mileage(VDR_HEAD *pHead)
{
    u8 *pDst=pHead->pbuf;

    pHead->len = 20;
    memset(pHead->pbuf,0,pHead->len);

    Vdr_Fuc_Mem[VDR_MEM_RTC](VDR_MEM_READ,pDst);//实时时间
    pDst += 6;
    
    Vdr_Fuc_Mem[VDR_MEM_TIMEINIT](VDR_MEM_READ,pDst);//初次时间
    pDst += 6;
    
    Vdr_Fuc_Mem[VDR_MEM_MILEINIT](VDR_MEM_READ,pDst);//初始里程
    pDst += 4;

    Vdr_Fuc_Mem[VDR_MEM_MILEADD](VDR_MEM_READ,pDst);//累计里程
    pDst += 4;
}
/**
  * @brief  脉冲系数
  * @param  None
  * @retval None
  */
void Vdr_Get_Pulse(VDR_HEAD *pHead)
{
    u8 *pDst=pHead->pbuf;

    pHead->len = 8;
    
    memset(pHead->pbuf,0,pHead->len);
    
    Vdr_Fuc_Mem[VDR_MEM_RTC](VDR_MEM_READ,pDst);//当前时间
    pDst += 6;
    
    Vdr_Fuc_Mem[VDR_MEM_PULSE](VDR_MEM_READ,pDst);//脉冲系数
    pDst += 2;
}
/**
  * @brief  车辆信息
  * @param  None
  * @retval None
  */
void Vdr_Get_CarInfor(VDR_HEAD *pHead)
{
    u8 *pDst=pHead->pbuf;
    
    pHead->len = 41;
    
    memset(pHead->pbuf,0,pHead->len);
    
    Vdr_Fuc_Mem[VDR_MEM_CARCODE](VDR_MEM_READ,pDst);  
    pDst += 17;

    Vdr_Fuc_Mem[VDR_MEM_CARPLATE](VDR_MEM_READ,pDst); 
    pDst += 12;    

    Vdr_Fuc_Mem[VDR_MEM_CARTYPE](VDR_MEM_READ,pDst); 
    pDst += 12;
}
/**
  * @brief  状态信号
  * @param  None
  * @retval None
  */
void Vdr_Get_StaSignal(VDR_HEAD *pHead)
{
    u8  *pDst=pHead->pbuf;

    pHead->len = 87;
    
    memset(pHead->pbuf,0,pHead->len);
    
    Vdr_Fuc_Mem[VDR_MEM_RTC](VDR_MEM_READ,pDst);      //当前时间
     pDst  += 6;

    *pDst++ = 1;                                      //状态个数

    Vdr_Fuc_Mem[VDR_MEM_STASIGNAL](VDR_MEM_READ,pDst);//状态信息
}
/**
  * @brief  唯一编号
  * @param  None
  * @retval None
  */
void Vdr_Get_OnlyNum(VDR_HEAD *pHead)
{
    u8  *pDst=pHead->pbuf;
    
    pHead->len = 35;
    
    memset(pHead->pbuf,0,pHead->len);
    
    Vdr_Fuc_Mem[VDR_MEM_ONLY](VDR_MEM_READ,pDst);
}
/**
  * @brief  根据命令地址获取基本信息
  * @param  None
  * @retval None
  */
void Vdr_Get_Base(u8 cmdnum,VDR_HEAD *pHead)
{
    pHead->cmd = Vdr_Cmd_Base[cmdnum];//命令字
    Vdr_Fuc_Base[cmdnum](pHead);      //数据
}

/**
  * @brief  根据命令地址获取记录信息
  * @param  None
  * @retval None
  */
void Vdr_Get_Ser(u8 cmdnum,VDR_SERT time, VDR_HEAD *pHead)
{
    pHead->cmd = Vdr_Cmd_Ser[cmdnum];                    //命令字

    pHead->len = Register_Read(cmdnum+1,                 //检索数据
                               pHead->pbuf,
                               time.t_start,
                               time.t_end,
                               time.maxblock);
}


/**
  * @brief  获取数据包信息
  * @param  None
  * @retval None
  */
void Vdr_Get_PackageInf(VDR_PACKINF PackInf, VDR_PACKSER *pPackage)
{
    u8  cmdnum   = PackInf.cmdnum;//命令编号
    u8  *p_data  = Vdr_BufData;//记录仪公共缓存
     
    u32 addrstar = VdrSecAddr[cmdnum*2]*VDR_SECTOR_SIZE;//开始地址
    u32 addrend  = VdrSecAddr[cmdnum*2+1]*VDR_SECTOR_SIZE;//结束地址
    u8  steplen  = VdrStepLen[cmdnum];//单步长度
    u16 upnum    = VDR_PACKAGE_SIZE/VdrUpLen[cmdnum];
    u16 upcnt    = 0;

    u32 stepnum  = VDR_SECTOR_SIZE/steplen;//扇区单步整数 有效数据
    u16 steprem  = VDR_SECTOR_SIZE%steplen;//扇区单步余数 无效数据 用于跳过余数地址

    u32 forend   = (VdrSecAddr[cmdnum*2+1]-VdrSecAddr[cmdnum*2])*(VDR_SECTOR_SIZE/steplen);//扇区内的总步数
    u16 stepcnt  = 0;//单扇区内单步计数器用于判断余数

    u32 addr     = 0;      
    u8  fwrflg   = 0;
    u8  datflg   = 0;    
    u8  wrtflg   = 0;
    u8  starflg  = 0;    
    u8  errcnt   = 0;

    u32 steptime;//存储器中每个单步的32位时间
    u32 steptimelast;//上个时间
    u32 i;

    TIME_T time_t;
    TIME_T timeback_t;  
    u8     timeis1,timeis2;

    memset((u8*)pPackage,0,sizeof(VDR_PACKSER));//清空结构体
    pPackage->cmd  = Vdr_Cmd_All[cmdnum];//命令
    pPackage->pbuf = Vdr_PackBuf;//缓存地址
    
    steptime     = 0;
    steptimelast = 0;
    addr         = addrstar; 
    
    timeback_t.year = 0x55;//初始值
    
    for(i=0; i<forend; i++)//遍历所有单步数据
    {  
        sFLASH_ReadBuffer(p_data,addr,steplen);//读取数据

        steptime  = (p_data[0]<<24)|(p_data[1]<<16)|(p_data[2]<<8)|p_data[3];//读取单步时间
        Gmtime(&time_t,steptime);//转换时间

        if(datflg == 0)//搜索开始地址
        {
            if(steptime != 0xffffffff)
            {
                if(steptimelast == 0xffffffff)
                {
                    i       = 0;//重新遍历         
                    datflg  = 1;//找到开始地址
                    wrtflg  = 1;//写入文件
                    pPackage->cyc = 0;//循环
                    pPackage->addrstar = addr;//开始地址
                }
                fwrflg = 1;              
            }
            steptimelast = steptime;//刷新上次时间

            if(((i+1) == forend)&&(fwrflg == 1))
            {
                i = 0;//未循环存储 重新遍历  
                if((errcnt++) > 1)
                {
                    break;//整块区域非法
                }                
            }
        }
        else if(steptime != 0xffffffff)//有效数据
        {
            if(steptime > PackInf.timeend)
            {
                if(upcnt)
                {
                    pPackage->all++;
                }
                break;//找到结束地址退出循环
            }
            else
            {
                wrtflg = 1;
            }
        }
        else//结束
        {
            if(upcnt)
            {
                pPackage->all++;
            }
            break;//找到结束地址退出循环
        }
        ////////////////////////////////////////////////////////////////////////
        if((wrtflg)&&(steptime >= PackInf.timestar))
        {
            if(starflg == 0)
            {
                starflg = 1;
                pPackage->addrstar = addr;//开始地址
            }
            else
            {
                pPackage->addrend = addr;//结束地址      
            }            
            if(cmdnum == 8)//速度 分钟组包
            {
                timeis1 = time_t.min;
                timeis2 = timeback_t.min;  
                #ifdef USE_ONE_MIN_SAVE_SPEED_RECORD
                cmdnum = 0x55;
                #endif
            }
            else if(cmdnum == 9)//位置 小时组包
            {
                timeis1 = time_t.hour;
                timeis2 = timeback_t.hour;               
            }
            if((cmdnum == 8)||(cmdnum == 9))
            {
                if((timeis1 != timeis2)&&(timeback_t.year != 0x55))
                {
                    if(++upcnt >= upnum)//检索
                    {
                        upcnt = 0;
                        pPackage->all++;//组包完毕
                    }
                }
                timeback_t = time_t;//刷新时间
            }
            else
            {    
                if(++upcnt >= upnum)//检索
                {
                    upcnt = 0;
                    pPackage->all++;
                }         
            }               
        }
        wrtflg = 0;
        ////////////////////////////////////////////////////////////////////////
        addr += steplen;//指向下一个数据
        if((++stepcnt) >= stepnum)
        {
            stepcnt = 0;
            addr += steprem;//跳过余数无效数据
        }
        if(addr >= addrend)
        {
            addr = addrstar;//扇区结束调头重新来
            pPackage->cyc = 1;//循环
        }
        IWDG_ReloadCounter();//喂狗
    }
}
/**
  * @brief  数据包检索
  * @param  None
  * @retval None
  */
void Vdr_Get_PackageSer(VDR_PACKSER *pPackage)
{
    u8  cmdnum   = Vdr_Cmd_To(pPackage->cmd);//命令编号
    u8  *p_send  = pPackage->pbuf;
    
    u32 addrstar = VdrSecAddr[cmdnum*2]*VDR_SECTOR_SIZE;//开始地址
    u32 addrend  = VdrSecAddr[cmdnum*2+1]*VDR_SECTOR_SIZE;//结束地址
    u8  steplen  = VdrStepLen[cmdnum];//单步长度
    u16 upnum    = VDR_PACKAGE_SIZE/VdrUpLen[cmdnum];
    u16 upcnt    = 0;

    u32 stepnum  = VDR_SECTOR_SIZE/steplen;//扇区单步整数 有效数据
    u16 steprem  = VDR_SECTOR_SIZE%steplen;//扇区单步余数 无效数据 用于跳过余数地址

    u32 addr     = pPackage->addrstar;  
    u32 steptime = 0;//存储器中每个单步的32位时间

    u32 backaddr = 0;      
    u16 backstep = 0;
    
    TIME_T time_t;
    TIME_T timeback_t;
    u8     timesv;
    u8     timesz;   
    u8     timeis1,timeis2;     

    u8  buf[VDR_STEP_SIZE];
    u8  spdmin;//速度按分钟存储标志

    if(pPackage->all == 0)
    {
        return ;//无封包数据
    }
    
    memset(p_send,0xff,VdrUpLen[cmdnum]*upnum);//填充缓存
    timeback_t.year = 0x55;//初始值
    pPackage->len   = 0;
    
    for(;;)//遍历所有单步数据
    {  
        if(pPackage->addrstar > pPackage->addrend)
        {      
            if(pPackage->cyc == 0)
            {
                if((pPackage->cnt < pPackage->all)||(upcnt))
                {
                    pPackage->cnt++;
                }
                break;//退出
            }
        }
        sFLASH_ReadBuffer(buf,addr,steplen);//读取数据

        backaddr = addr;
        backstep = pPackage->stepcnt;

        steptime  = (buf[0]<<24)|(buf[1]<<16)|(buf[2]<<8)|buf[3];//读取单步时间
        Gmtime(&time_t,steptime);//转换时间

        ////////////////////////////////////////////////////////////////////////
        addr += steplen;//指向下一个数据
        if((++pPackage->stepcnt) >= stepnum)
        {
            pPackage->stepcnt = 0;
            addr += steprem;//跳过余数无效数据
        }
        if(addr >= addrend)
        {
            addr = addrstar;//扇区结束调头重新来
        }
        pPackage->addrstar = addr;
        
        ////////////////////////////////////////////////////////////////////////
        if(cmdnum == 8)//速度 分钟组包
        {
            timesz  = 2;        
            timesv  = time_t.sec;
            timeis1 = time_t.min;
            timeis2 = timeback_t.min; 
            spdmin  = 0;
            #ifdef USE_ONE_MIN_SAVE_SPEED_RECORD
            spdmin  = 1;
            #endif            
        }
        else if(cmdnum == 9)//位置 小时组包
        {
            timesz  = 11;          
            timesv  = time_t.min;
            timeis1 = time_t.hour;
            timeis2 = timeback_t.hour;               
        }
        if(((cmdnum == 8)||(cmdnum == 9))&&(spdmin == 0))
        {                
            ////////////////////////////////////////////////////////////////////
            if((timeis1 != timeis2)&&(timeback_t.year != 0x55))
            {
                Public_ConvertTimeToBCDEx(timeback_t,p_send);
                p_send[5] = 0;//秒清零 
                if(timesz == 11)
                {
                    p_send[4] = 0;//分清零 
                }
                pPackage->len += VdrUpLen[cmdnum];//长度累加
                if(++upcnt >= upnum)//检索
                {
                    upcnt = 0;
                    pPackage->cnt++;//组包完毕
                    pPackage->addrstar = backaddr;
                    pPackage->stepcnt  = backstep;                    
                    break;
                }
                p_send += VdrUpLen[cmdnum];//转向下一包  
            }
            memcpy(p_send+6+(timesv*timesz),buf+4,timesz);//初次数据
            timeback_t = time_t;//刷新时间
        }
        else
        {
            pPackage->len += VdrUpLen[cmdnum];//长度累加            
            memcpy(p_send,buf+4,VdrUpLen[cmdnum]);          
            if(++upcnt >= upnum)//检索
            {
                upcnt = 0;
                pPackage->cnt++;//组包完毕
                break;
            }
            p_send += VdrUpLen[cmdnum];//转向下一步             
        }
        IWDG_ReloadCounter();//喂狗
    }
    if((pPackage->cyc)&&(pPackage->addrstar < pPackage->addrend))
    {
        pPackage->cyc = 0;
    }
}












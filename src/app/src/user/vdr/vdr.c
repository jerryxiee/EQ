/*
********************************************************************************
*
*
* Filename      : vdr.c
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
#define VDR_CMD_ERR_GET  0xfa
#define VDR_CMD_ERR_SET  0xfb
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
const u8 Vdr_Cmd_Base[]=//基本采集命令
{0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07};
const u8 Vdr_Cmd_Ser[]=//检索采集命令
{0x08,0x09,0x10,0x11,0x12,0x13,0x14,0x15};
const u8 Vdr_Cmd_Set[]=//设置命令
{0x82,0x83,0x84,0xc2,0xc3,0xc4};
const u8 Vdr_Cmd_Test[]=//检定命令
{0xe0,0xe1,0xe2,0xe3,0xe4,0xe5};

const u8 Vdr_Cmd_All[]=//所有命令
{0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,//0  - 7  采集
 0x08,0x09,0x10,0x11,0x12,0x13,0x14,0x15,//8  - 15 采集
 0x82,0x83,0x84,0xc2,0xc3,0xc4,          //16 - 21 设置
 0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,          //22 - 27 检定
 0xd0,                                   //28      参数
};

/*
********************************************************************************
*                          LOCAL VARIABLES
********************************************************************************
*/
static VDR_WIRE    Vdr_Wire;//无线采集
static VDR_WIREADD Vdr_WireAdd;//无线补传
/*
********************************************************************************
*                          GLOBAL VARIABLES
********************************************************************************
*/
u8  Vdr_BufData[1024];//数据缓存
u8  Vdr_BufSend[1200];//发送缓存
u16 Vdr_WireAck;//无线采集应答流水号    

/*
********************************************************************************
*                          EXTERN VARIABLES
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
* @brief  计算异或校验
* @param   pVef  -> 校验字
           pSrc  -> 源数据
           SrcLen-> 源长度
* @retval None
*/
void Vdr_XorVef(u8 *pVef,const u8 *pSrc, u16 SrcLen)
{ 
    u16 i;
    
    for(i=0;i<SrcLen;i++)
    {
        *pVef ^= *pSrc++;
    }
}
/**
  * @brief  获取搜索时间头数据
  * @param  None
  * @retval None
  */
void Vdr_Time_Get(const u8 *pSrc, VDR_SERT *pTime)
{
    u32 timecnt;

    //检查数据合法性
    memset(pTime,0,sizeof(VDR_SERT));//清空结构体

    memcpy(pTime->bcd_start,pSrc,6);//开始时间
    pSrc += 6;
    
    memcpy(pTime->bcd_end,pSrc,6);//结束时间
    pSrc += 6;

    pTime->maxblock = *pSrc++;
    pTime->maxblock = (pTime->maxblock<<8)|(*pSrc++);//数据块数 

    Public_ConvertBCDToTime(&pTime->t_start,
                             pTime->bcd_start);
                            
    Public_ConvertBCDToTime(&pTime->t_end, 
                             pTime->bcd_end);    

    timecnt = ConverseGmtime(&pTime->t_start);
    pTime->cnt_start = timecnt;
    
    timecnt = ConverseGmtime(&pTime->t_end)+1;
    pTime->cnt_end= timecnt; 

}


/**
  * @brief  获取记录仪帧头数据
  * @param  None
  * @retval None
  */
void Vdr_Head_Get(const u8 *pSrc, VDR_HEAD *pHead)
{
    pSrc += 2;                              //55 7a
    pHead->cmd  = *pSrc++;                  //命令
    pHead->len  = *pSrc++; 
    pHead->len  = (pHead->len<<8)|(*pSrc++);//长度
    pHead->back = *pSrc++;                  //备用字
    pHead->pbuf = (u8*)pSrc;                //数据 
}

/**
  * @brief  把记录仪结构体转成缓存字节流
  * @param  type: 0->头+数据+校验  
                  1->头+数据 
                  2->仅数据      
  * @retval None
  */
u16 Vdr_Head_ToBuf(u8 type, VDR_HEAD head, u8 *pDst)
{   
    u8  *pVef=pDst;
    u8  verf=0;    
    u16 len=0;
    
    if((head.cmd == VDR_CMD_ERR_GET)||(head.cmd == VDR_CMD_ERR_SET))
    {
         len    = 4;
        *pDst++ = 0x55;
        *pDst++ = 0x7a;
        *pDst++ = head.cmd;
        *pDst++ = PRO_DEF_RETAINS;
    }
    else
    {
        if(type != 2)//仅数据时不加入帧头
        {
             len    = 6;
            *pDst++ = 0x55;
            *pDst++ = 0x7a;
            *pDst++ = head.cmd;
            *pDst++ = head.len >> 8;
            *pDst++ = head.len & 0xff;
            *pDst++ = PRO_DEF_RETAINS;
        }
        memcpy(pDst,head.pbuf,head.len);
        pDst += head.len;
        len  += head.len;
    }

    if(type == 0)//完整数据才计算校验
    {
        Vdr_XorVef(&verf,pVef,len);
        *pDst++ = verf;
        len++;
    }

    return len;
}
/**
  * @brief  记录仪命令转地址
  * @param  cmd->命令
  * @retval 返回地址
  */
u8 Vdr_Cmd_To(u8 cmd)
{
    u8 i;
    
    for(i=0;i<sizeof(Vdr_Cmd_All);i++)
    {
        if(Vdr_Cmd_All[i] == cmd)
        {
            return i;
        }
    }
    return 0;//非法
}

/**
  * @brief  检查记录仪命令字
  * @param  cmd->命令
  * @retval 合法返回1  非法返回0
  */
u8 Vdr_Cmd_Chk(u8 cmd)
{
    u8 i;
    
    for(i=0;i<sizeof(Vdr_Cmd_All);i++)
    {
        if(Vdr_Cmd_All[i] == cmd)
        {
            return 1;//合法
        }
    }
    return 0;//非法
}
/**
  * @brief  根据命令返回命令类型、函数坐标
  * @param  cmd->记录仪命令
            pType->类型 0:基本指令 1:采集指令 2:设置 3:鉴定 4:伊爱
            pAddr->对应命令地址
  * @retval 返回结果 合法返回1，非法返回0 
  */
u8 Vdr_Cmd_Cnv(u8 cmd, u8 *pType, u8 *pAddr)
{
    u8 i;

    for(i=0;i<sizeof(Vdr_Cmd_All);i++)
    {
        if(Vdr_Cmd_All[i] == cmd)
        {
            cmd = i;
            break;
        }
    }
    if(i == sizeof(Vdr_Cmd_All))
    {
        return 0;//非法指令
    }
    
    if(cmd < 8)
    {
        *pType = 0;//基本指令
        *pAddr = cmd;
    }
    else if(cmd < 16)
    {
        *pType = 1;//采集指令
        *pAddr = cmd-8;            
    }
    else if(cmd < 22)
    {
        *pType = 2;//设置命令
        *pAddr = cmd-16;
    }
    else if(cmd < 28)
    {
        *pType = 3;//鉴定指令
        *pAddr = cmd-22;              
    } 
    else if(cmd == 28)
    {
        *pType = 4;//伊爱
        *pAddr = 0;  
    }
    
    return 1;//合法指令
}
/**
  * @brief  触发无线采集任务(无线专用)
  * @param  None
  * @retval None
  */
void Vdr_Wire_Even(VDR_CH ch, VDR_HEAD head)
{
    VDR_SERT    serch_time;//检索时间
    TIME_T      now_time;  //当前时间
    VDR_PACKINF packageinf;//分包信息

    if(Vdr_Wire.busy)
    {
        if(Vdr_Wire.ch == ch)
        {
            return;//相同通道重复下发数据
        }
    }
    Vdr_Wire.ch   = ch;//数据通道
    Vdr_Wire.busy = 1;//置忙标志
    
    if(head.len == 0)//全部数据
    {
        RTC_GetCurTime(&now_time);
        now_time.year -= 1;
        packageinf.timestar = ConverseGmtime(&now_time);//开始时间
        now_time.year += 2;
        packageinf.timeend = ConverseGmtime(&now_time);//结束时间
    }
    else//采集指定时间段数据
    {
        Vdr_Time_Get(head.pbuf,&serch_time);
        packageinf.timestar = serch_time.cnt_start;
        packageinf.timeend  = serch_time.cnt_end;
    }
    
    packageinf.cmdnum = Vdr_Cmd_To(head.cmd);//命令字编号
    
    Vdr_Wire.packageinf = packageinf;//复制分包信息
    SetTimerTask(TIME_WIRELESS_RECORDER, SYSTICK_1SECOND);//触发无线记录仪定时任务 
}
/**
  * @brief  触发无线补传任务(无线专用)
  * @param  pSrc->指向源数据
  * @retval 返回结果, 上传补包数据返回1, 无数据返回0
  */
u8 Vdr_Wire_AddEven(VDR_CH ch, u8 *pSrc)
{
    u8 i = 0;
    u8 high,low;

    if(Vdr_Wire.ch != ch)
    {
        Vdr_WireAdd.rdy = 0;
        return 0;//通道不一致
    }
    if(Vdr_WireAdd.rdy == 0)
    {
        return 0;//非记录仪8003
    }
    
    high = *pSrc++;
    low  = *pSrc++;
    Vdr_WireAdd.ack = (high <<8)| low;//流水号
    Vdr_WireAdd.all = *pSrc++;        //重传个数
    Vdr_WireAdd.cnt = 0;

    for(i=0;i<Vdr_WireAdd.all;i++)    //重传列表
    {
        high = *pSrc++;
        low  = *pSrc++;    
        Vdr_WireAdd.buf[i] = (high <<8)| low;     
    }
    if(Vdr_WireAdd.all)
    {
        Vdr_Wire.busy   = 1;//置忙标志
        Vdr_WireAdd.flg = 1;//置补传标志
        SetTimerTask(TIME_WIRELESS_RECORDER, SYSTICK_1SECOND);//触发无线记录仪定时任务 
        return 1;//开始补传数据
    }
    Vdr_WireAdd.rdy = 0;
    return 0;//补传完毕
}
/**
  * @brief  触发无线补传任务(无线专用)
  * @param  None
  * @retval None
  */
u8 Vdr_Wire_AddGet(VDR_PACKSER *pPackage)
{

    VDR_PACKSER Package = Vdr_Wire.packageadd;
    u16 i;
    
    if(Vdr_WireAdd.cnt >= Vdr_WireAdd.all)
    {
        Vdr_WireAdd.flg = 0;
        return 0;//补传完毕
    }
    
    for(i=1;i<=Package.all;i++)
    {
        Vdr_Get_PackageSer(&Package);
        if(Package.cnt == Vdr_WireAdd.buf[Vdr_WireAdd.cnt])
        {
            *pPackage = Package;
            Vdr_WireAdd.cnt++;  
            return 1;
        }
    }
    return 0;
}
/**
  * @brief  发送数据包数据(无线专用)
  * @param  None
  * @retval None
  */
u16 Vdr_Wire_Package(VDR_CH ch,VDR_PACKSER *pPackage)
{
    MESSAGE_HEAD msg_head;
    VDR_HEAD head;
    u16 addr;
    u16 len;
    u8  wirech;

    //////////////////////////////////////////////////////////////////////第一包
    addr = 0;
    if(pPackage->cnt == 1)
    {
        Vdr_BufSend[addr++] = Vdr_WireAck>>8;
        Vdr_BufSend[addr++] = Vdr_WireAck&0xff;
        Vdr_BufSend[addr++] = pPackage->cmd;
    } 
    ////////////////////////////////////////////////////////////////记录仪协议头
    head.cmd = pPackage->cmd;
    head.len = pPackage->len;
    head.pbuf= pPackage->pbuf;
    len = Vdr_Head_ToBuf(0,head,Vdr_BufSend+addr);
    len += addr;

    msg_head.TotalPackage   = pPackage->all;
    msg_head.CurrentPackage = pPackage->cnt;    
    msg_head.MessageID      = 0x0700;
    msg_head.SerialNum      = Vdr_WireAck; 
    msg_head.Attribute      = len;
    if(pPackage->all > 1)
    {
        msg_head.Attribute |= 0x2000;//分包上传
    }    

    //////////////////////////////////////////////////////////////////////通道号
    wirech = CHANNEL_DATA_1;
    if(ch == VDR_WIRE_2)
    {
        wirech = CHANNEL_DATA_2;
    }
    #if 0
    if((pPackage->cnt == 2)&&(!Vdr_WireAdd.flg))//强制丢包
    {
        if(pPackage->cnt >= pPackage->all)
        {
            pPackage->all = 0;
            pPackage->cnt = 0;
            return 0;
        }      
        return 1;
    }
    #endif
    RadioProtocol_SendCmd(wirech,msg_head,Vdr_BufSend,len,NULL);
    ////////////////////////////////////////////////////////////////////最后一包    
    if(pPackage->cnt >= pPackage->all)
    {
        pPackage->all = 0;
        pPackage->cnt = 0;
        return 0;
    }      
    return 1;
}

/**
  * @brief  记录仪无线处理
  * @param  None
  * @retval 返回结果
  */
u8 Vdr_Wire_Handle(void)
{
    VDR_PACKSER *pPackage = &Vdr_Wire.packageser;

    if(Vdr_Wire.busy == 0)
    {
        Vdr_WireAdd.flg = 0;
        Vdr_WireAdd.rdy = 0;
        return 0;
    }
    else if(Vdr_Wire.busy == 1)
    {
        Vdr_Wire.busy++;
        Vdr_Wire.state = 0;//进入正常发送
        if(Vdr_WireAdd.flg)
        {
            Vdr_Wire.state = 2;//进入补传
        }
    }
    
    switch(Vdr_Wire.state)
    {
        case 0://获取分包信息
        {
            Vdr_Get_PackageInf(Vdr_Wire.packageinf,pPackage);
            Vdr_Wire.packageadd = *pPackage;
            Vdr_Wire.state++;
            break;
        }
        case 1://传输分包数据
        {
            Vdr_Get_PackageSer(pPackage);                //获取数据
            if(Vdr_Wire_Package(Vdr_Wire.ch,pPackage)==0)//发送平台
            {
                Vdr_Wire.state  = 0xaa;//结束
                Vdr_WireAdd.rdy = 1;   //等待8003
            }
            break;
        }
        case 2://补传分包数据
        {
            if(Vdr_Wire_AddGet(pPackage) == 0)//获取补包数据
            {
                Vdr_Wire.state=0xaa;//结束
                break;
            }
            Vdr_Wire_Package(Vdr_Wire.ch,pPackage);
            break;
        }
        default:
        {
            Vdr_Wire.busy=0;//空闲
            return 0;//停止
        } 
    }
    return 1;//继续
}
/**
  * @brief  检查来自串口的数据
  * @param  None
  * @retval 返回错误类型
  */
VDR_ERR Vdr_Data_Chk(VDR_CH ch, u8 *pSrc, u16 SrcLen)
{
    VDR_ERR err=VDR_ERR_NO;//正确
    u8 vef=0;
    u8 *pVef=pSrc;
    u8 cmd;

    if(ch == VDR_COM)
    {
        ////////////////////////////////////////////////////////////////检查帧头
        if((*pSrc++ != 0xaa)||(*pSrc++ != 0x75))
        {
            return VDR_ERR_HEAD;//非记录仪命令
        }
        cmd = *pSrc++;//命令字
        ////////////////////////////////////////////////////////////////检查校验
        Vdr_XorVef(&vef,pVef,SrcLen-1);
        pVef += SrcLen-1;
        if(vef != *pVef)
        {
            return VDR_ERR_VF;//校验错误
        }
    }
    else
    {
        cmd = *pSrc++;//命令字
    }
    ////////////////////////////////////////////////////////////////////检查指令
    if(Vdr_Cmd_Chk(cmd) == 0)
    {
        err = VDR_ERR_CMD;//指令错误
    }
    
    return err;
}


/**
  * @brief  应答来自串口的数据,发送有效数据到串口
  * @param  None
  * @retval 正确返回 0
  *         错误返回 非0
  */
void Vdr_Data_Send(VDR_CH ch, VDR_HEAD head)
{
    MESSAGE_HEAD msg_head;
    u16 len;
    u16 addr;
    u8  wirech;

    if(ch == VDR_COM)
    {
        len = Vdr_Head_ToBuf(0,head,Vdr_BufSend);
        COM1_WriteBuff(Vdr_BufSend,len);
    }
    else
    {
        addr = 0;
        Vdr_BufSend[addr++] = Vdr_WireAck>>8;
        Vdr_BufSend[addr++] = Vdr_WireAck&0xff;

        Vdr_BufSend[addr++] = head.cmd;
        len = Vdr_Head_ToBuf(0,head,Vdr_BufSend+addr);//转为字节流
        len += addr;    
        
        msg_head.MessageID = 0x0700;//记录仪数据上传命令  
        msg_head.SerialNum = Vdr_WireAck; 
        msg_head.Attribute = len;  

        wirech = CHANNEL_DATA_1; 
        if(ch == VDR_WIRE_2)
        {
            wirech = CHANNEL_DATA_2;
        }
        RadioProtocol_SendCmd(wirech,msg_head,Vdr_BufSend,len,NULL);
    }
}

/**
  * @brief  处理来自串口或网络的数据
  * @param  None
  * @retval 返回处理结果
  */
u8 Vdr_Data_Pro(VDR_CH ch, VDR_HEAD head, VDR_HEAD *pHead)
{
    VDR_SERT  serch_time;
    u8  addr;
    u8  type;
    
    ////////////////////////////////////////////////////////////////////转换命令
    if(Vdr_Cmd_Cnv(head.cmd,&type,&addr) ==0 )
    {
        return 0;//非法数据
    }
    
    ////////////////////////////////////////////////////////////////////采集数据
    switch(type)
    {
        case 0://基本指令
        {
            Vdr_Get_Base(addr,pHead);
            Vdr_Wire.busy = 0;//停止网络发送
            break;
        }
        case 1://采集指令
        {
            if(ch == VDR_COM)//串口
            {
                Vdr_Time_Get(head.pbuf,&serch_time);
                Vdr_Get_Ser(addr,serch_time,pHead);
            }
            else//无线采集
            {   
                Vdr_Wire_Even(ch,head);
                return 0;
            }
            break;
        }
        case 2://设置指令
        {
            Vdr_Set_Pro(addr,head.pbuf,pHead);
            break;
        }
        case 3://鉴定指令
        {
            Vdr_Test_Pro(addr,pHead);
            break;
        } 
        case 4://伊爱指令
        {
            Vdr_Eye_Value(&head,pHead);
            break;
        }        
    }
    return 1;
}

/**
  * @brief  记录仪接收处理
  * @param  ch     -> 通道号
            type   -> 0:采集 1:设置
            pSrc   -> 源数据
            SrcLen -> 源长度
  * @retval None
  */
void Vdr_Data_Rec(VDR_CH ch, u8 type, u8 *pSrc, u16 SrcLen)
{
    VDR_HEAD HeadRec,HeadSend;
    VDR_ERR  err;

    ////////////////////////////////////////////////////////////////////检查数据
    err = Vdr_Data_Chk(ch,pSrc,SrcLen);
    if(err != VDR_ERR_NO)
    {
        if(err == VDR_ERR_CMD)
        {
            HeadSend.cmd = VDR_CMD_ERR_GET;      //采集错误
            if(type)
            {
                HeadSend.cmd = VDR_CMD_ERR_SET;  //设置错误
            }
            Vdr_Data_Send(ch,HeadSend);
        }
        return;                                
    }

    ////////////////////////////////////////////////////////////////////提取数据
    if(SrcLen < 6)//无数据头
    {
        HeadRec.cmd = *pSrc++;
        HeadRec.len = 0;
    }
    else//命令字 + 记录仪完整数据
    {
        if(ch != VDR_COM)
        {
            pSrc++;
        }
        Vdr_Head_Get(pSrc,&HeadRec);
    }
    
    ////////////////////////////////////////////////////////////////////处理数据
    HeadSend.pbuf = Vdr_BufData;
    if(Vdr_Data_Pro(ch,HeadRec,&HeadSend))
    {
        Vdr_Data_Send(ch,HeadSend);//发送数据
    }
}

/*
********************************************************************************
*                               END
********************************************************************************
*/




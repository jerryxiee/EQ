/********************************************************************
//版权说明	:Shenzhen E-EYE Co. Ltd., Copyright 2009- All Rights Reserved.
//文件名称	:Gps_App.c		
//功能		:GPS模块
//版本号	:
//开发人	:dxl
//开发时间	:2011.8
//修改者	:
//修改时间	:
//修改简要说明	:
//备注		:
***********************************************************************/ 
/*************************修改记录*************************/
//***********************创建(用于八厅分体机)*******************
//2011.8.25--1.整理GPS模块,将GPS模块分成3个部分：
//-----------GPS_RxIsr(void)：负责数据接收
//-----------GpsParse_EvTask(void):负责协议解析
//-----------Gps_TimeTask(void):负责模块管理
//20118.25--2.GPS模块稳定性措施：
//-----------(1)程序上电复位,GPS模块也上电复位,需要先关再开
//-----------(2)校验出错或漂移次数达到一定次数时,置位出错标志,对GPS模块复位
//--------------复位:模块先关闭5秒,后再打开,只有ACC ON时才会检查错误标志
//-----------(3)ACC OFF时,关GPS模块,每小时开1分钟
//-----------(4)连续40秒没有收到数据,置位出错标志
//-----------(5)漂移连续10次,置位出错标志
//-----------(6)校验出错连续10次,置位出错标志
//2011.8.25--3.去漂移的方法：
//-----------(1)纬度方向,距离除以时间>=250公里/小时,认为是漂移
//-----------(2)经度方向,距离除以时间>=250公里/小时,认为是漂移
//-----------(3)速度向上增大时,速度变化量/时间>=20海里/小时认为是漂移
//-----------(4)虽然导航,但星数小于3或水平精度因子大于等于10,认为是漂移
//**********************修改*********************
//2012.6.5--4.去漂移方法改为：
//-----------(1)纬度方向,距离除以时间>=250公里/小时,认为是漂移
//-----------(2)经度方向,距离除以时间>=250公里/小时,认为是漂移
//-----------(3)速度向上增大时,速度变化量/时间>=9海里/小时认为是漂移
//-----------(4)增加去漂移使能开关，连续10次2D导航后，开启去漂移，连续10次漂移后，关闭去漂移；
//****************文件包含**************************
#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "include.h"
//***************变量定义***********************
static u16	GprmcVerifyErrorCount = 0;//GPRMC校验出错计数
static u16	GpggaVerifyErrorCount = 0;//GPGGA校验出错计数
static u16	GprmcParseErrorCount = 0;//GPRMC解析出错计数
static u16	GpggaParseErrorCount = 0;//GPGGA解析出错计数
static u16	GpsParseEnterErrorCount = 0;//没有进入解析函数计数
static u8	StopExcursionEnable = 0;//车辆停止时去漂移使能开关,0为不使能,1为使能
static u8       GpsRunFlag = 0;//去漂移行驶标志,1为行驶，0为停驶
static  u8      AdjustRtcFlag = 0;//校时标志，上电后从不导航到导航校时1次，今后每隔50小时校时1次
//static  u8      AdjustRtcFlag1 = 0;//GPS从导航到不导航需要校时

u32	gPositionTime = 0;//gPosition有效时的时间 
u32	PositionTime = 0;//Position有效时的时间 

u8	LastLocationFlag = 0;//上一次定位状态
u8	LocationFlag = 0;//当前定位状态
//static u16	AccOffCount = 0;
//static u16	AccOnCount = 0; BY WYF
u8	LastLocationSpeed = 0;//上一次定位的速度 
GPS_STRUCT	gPosition;//当前有效位置
u8	GpsOnOffFlag = 0;//GPS开关标志,0为GPS关,1为GPS开
GPS_STRUCT	 Position;//临时解析用

//*****************外部变量********************
extern  u8	AccOffGpsControlFlag;//ACC OFF时GPS模块控制标志,0为关,1为开
extern u8	GprmcBuffer[];//存放GPRMC数据
extern u8	GprmcBufferBusyFlag;//GprmcBuffer缓冲忙标志
extern u8	GpggaBuffer[];//存放GPGGA数据
extern u8	GpggaBufferBusyFlag;//GpggaBuffer缓冲忙标志
extern u8	GpgsvBuffer[];//存放GPGGA数据
extern u8	GpgsvBufferBusyFlag;//GpggaBuffer缓冲忙标志
extern u8	SpeedFlag;//dxl,2015.5.11,0x00:手动脉冲,0x01:手动GPS,0x02:自动脉冲,0x03:自动GPS
extern TIME_T	CurTime;
static u8 GPS_GpsSpeedHanlde(u8 GpsSpeed);
//****************函数定义*********************
/*********************************************************************
//函数名称	:Gps_ReadStaNum(void)
//功能		:获取GPS的定位星数
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
u8 Gps_ReadStaNum(void)
{
  return gPosition.SatNum;
}
/*********************************************************************
//函数名称	:GPS_AdjustRtc(GPS_STRUCT *Position)
//功能		:GPS校时
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
ErrorStatus Gps_AdjustRtc(void)
{
  TIME_T time;
  u32 timecount;
  
  time.year = Position.Year;
  time.month = Position.Month;
  time.day = Position.Date;
  time.hour = Position.Hour;
  time.min = Position.Minute;
  time.sec = Position.Second;
  
  if(SUCCESS == CheckTimeStruct(&time))
  {
    
    timecount = ConverseGmtime(&time);
    timecount += 8*3600;
    timecount -= 3;//由于比省运管平台时间快了，这里减去3秒
    Gmtime(&time, timecount);
    if(SUCCESS == CheckTimeStruct(&time))
    {
      SetRtc(&time);
      return SUCCESS;		
    }
    else
    {
      return ERROR;
    }
  }
  else
  {
    return ERROR;
  }
}	
/*********************************************************************
//函数名称	:Gps_GetRtcAdjSta()
//功能		:获取GPS校时状态
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
u8 Gps_GetRtcAdjSta(void)
{
  return AdjustRtcFlag;
}
/*********************************************************************
//函数名称	:GPS_GpsDataIsOk(u8 *pBuffer, u8 BufferLen)
//功能		:判断GPS数据是否正确
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:正确返回1，错误返回0
//备注		:
*********************************************************************/
u8 Gps_DataIsOk(u8 *pBuffer, u8 BufferLen)
{
  u8	i;
  u8	j;
  u8	temp;
  u8	sum = 0;
  u8	count = 0;
  u8	High;
  u8	Low;
  u8	verify;
  
  for(i=1; i<BufferLen; i++)//第1个是起始符'$'
  {
    temp = *(pBuffer+i);
    if((0x0a == temp)||(0x0d == temp))
    {
      break;//跳出循环
    }
    else if('*' == temp)
    {
      j = i;	
      break;
    }
    else
    {
      sum ^= temp;//异或？
      if(',' == temp)
      {
        count++;
      }
    }
  }
  High = *(pBuffer+j+1);
  Low = *(pBuffer+j+2);
  if((High >= '0')&&(High <= '9'))
  {
    High = High - 0x30;
  }
  else if((High >= 'A')&&(High <= 'F'))
  {
    High = High - 0x37;
  }
  else
  {
    return 0;
  }
  if((Low >= '0')&&(Low <= '9'))
  {
    Low = Low - 0x30;
  }
  else if((Low >= 'A')&&(Low <= 'F'))
  {
    Low = Low - 0x37;
  }
  else
  {
    return 0;
  }
  verify = (High << 4)|Low;
  if(verify == sum)
  {
    return 1;
  }
  else
  {
    return 0;
  }	
}
/*********************************************************************
//函数名称	:GPS_GprmcIsLocation(u8 *pBuffer, u8 BufferLen)
//功能		:判断GPRMC数据是否定位
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:正确返回1，错误返回0
//备注		:
*********************************************************************/
u8 Gps_GprmcIsLocation(u8 *pBuffer, u8 BufferLen)
{
  u8	i;
  u8	temp;
  u8	count = 0;
  
  for(i=0; i<BufferLen; i++)
  {
    temp = *(pBuffer+i);
    if(',' == temp)
    {
      count++;
    }
    else if('A' == temp)
    {
      if(2 == count)
      {
        return 1;
      }	
    }
  }
  
  return 0;	
}

/*********************************************************************
//函数名称	:GPS_GprmcParse(u8 *pBuffer, u8 BufferLen)
//功能		:解析GPRMC数据
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void Gps_GprmcParse(u8 *pBuffer, u8 BufferLen)
{
  u8	i = 0;
  u8	j = 0;
  u8	k = 0;
  u8	l = 0;
  u8	m = 0;
  s8	z = 0;
  u8	temp;
  u8	flag = 0;
  u8	count = 0;
  u16	temp2;
  u16	temp3;
  
  for(i=0; i<BufferLen; i++)
  {
    temp = *(pBuffer+i);
    if((0x0a == temp)||(0x0d == temp))
    {
      break;
    }
    else if('.' == temp)
    {
      l = i;//.号的位置
    }
    else if(',' == temp)
    {
      k = i;//当前逗号的位置
      count++;
      switch(count)
      {
      case 2://第2个逗号,解析时分秒
        {
          if(7 == (l-j))
          {
            //解析时间,小数点后的秒抛弃
            Position.Hour = (*(pBuffer+j+1)-0x30)*10+(*(pBuffer+j+2)-0x30);
            Position.Minute = (*(pBuffer+j+3)-0x30)*10+(*(pBuffer+j+4)-0x30);
            Position.Second = (*(pBuffer+j+5)-0x30)*10+(*(pBuffer+j+6)-0x30);	
          }
          else if(1 == (k-j))//没有数据
          {
            
          }
          else
          {
            flag = 1;//错误
          }
          
          break;
        }
      case 3://第3个逗号,解析有效标志
        {
          if(2 == (k-j))
          {
            //解析有效标志
            if('A' == *(pBuffer+j+1))
            {
              Position.Status = 1;
            }
            else if('V' == *(pBuffer+j+1))
            {
              Position.Status = 0;
            }
            else
            {
              flag = 1;//错误
            }
            
          }
          else if(1 == (k-j))//没有数据
          {
            
          }
          else
          {
            flag = 1;
          }
          break;
        }
      case 4://第4个逗号,解析纬度值，精确到0.0001分，即取小数点后四位
        {
          if((k > l)&&(5 == l-j))
          {
            //解析纬度值
            Position.Latitue_D = (*(pBuffer+j+1)-0x30)*10+(*(pBuffer+j+2)-0x30);
            Position.Latitue_F = (*(pBuffer+j+3)-0x30)*10+(*(pBuffer+j+4)-0x30);
            temp2 = 0;
            //以下这段代码在分的小数位不足4位时解析有问题,dxl,2013.6.27
            for(m=l+1; m<l+5; m++)
            {
              temp3 = (*(pBuffer+m)-0x30);
              for(z=0; z<l+4-m; z++)
              {
                temp3 = temp3 * 10;	
              }
              temp2 += temp3;
            }
            Position.Latitue_FX = temp2;
            
            
          }
          else if(1 == (k-j))//没有数据
          {
            
          }
          else
          {
            flag = 1;
          }
          break;
        }
      case 5://第5个逗号,解析纬度方向
        {
          if(2 == (k-j))
          {
            //解析纬度方向
            if('N' == *(pBuffer+j+1))
            {
              Position.North = 1;
              
            }
            else if('S' == *(pBuffer+j+1))
            {
              Position.North = 0;
              
            }
            else
            {
              flag = 1;
            }
          }
          else if(1 == (k-j))//没有数据
          {
            
          }
          else
          {
            flag = 1;
          }
          break;
        }
      case 6://第6个逗号,解析经度值，精确到0.0001分
        {
          if((k > l)&&(6 == l-j))
          {
            //解析经度值
            Position.Longitue_D = (*(pBuffer+j+1)-0x30)*100+(*(pBuffer+j+2)-0x30)*10+*(pBuffer+j+3)-0x30;
            Position.Longitue_F = (*(pBuffer+j+4)-0x30)*10+(*(pBuffer+j+5)-0x30);
            temp2 = 0;
            for(m=l+1; m<l+5; m++)
            {
              temp3 = (*(pBuffer+m)-0x30);
              for(z=0; z<l+4-m; z++)
              {
                temp3 = temp3 * 10;	
              }
              temp2 += temp3;
            }
            Position.Longitue_FX = temp2;
          }
          else if(1 == (k-j))//没有数据
          {
            
          }
          else
          {
            flag = 1;
          }
          break;
        }
      case 7://第7个逗号,解析经度方向
        {
          if(2 == (k-j))
          {
            //解析经度方向
            if('E' == *(pBuffer+j+1))
            {
              Position.East = 1;
              
            }
            else if('W' == *(pBuffer+j+1))
            {
              Position.East = 0;
              
            }
            else
            {
              flag = 1;
            }
          }
          else if(1 == (k-j))//没有数据
          {
            
          }
          else
          {
            flag = 1;
          }
          break;
        }
      case 8://第8个逗号,解析速度，精确到整数，小数部分抛弃
        {
          if((k>l)&&(l>j))
          {
            temp2 = 0;
            for(m=j+1; m<l; m++)
            {
              temp3 = (*(pBuffer+m)-0x30);
              for(z=0; z<l-1-m; z++)
              {
                temp3 = temp3 * 10;	
              }
              temp2 += temp3;
            }
            Position.Speed = temp2;
            Position.SpeedX = *(pBuffer+l+1)-0x30;
            if(Position.SpeedX >= 10)
            {
              //解析错误
              Position.SpeedX = 0;
            }
          }
          else if(1 == (k-j))//没有数据
          {
            
          }
          else
          {
            flag = 1;
          }
          break;
        }
      case 9://第9个逗号,解析方向，精确到整数，小数部分抛弃
        {
          if((k>l)&&(l>j))
          {
            temp2 = 0;
            for(m=j+1; m<l; m++)
            {
              temp3 = (*(pBuffer+m)-0x30);
              for(z=0; z<l-1-m; z++)
              {
                temp3 = temp3 * 10;	
              }
              temp2 += temp3;
            }
            Position.Course = temp2;		
          }
          else if(1 == (k-j))//没有数据
          {
            Position.Course = 0;
          }
          else
          {
            flag = 1;
          }
          break;
        }
      case 10://第10个逗号,解析年月日
        {
          if(7 == (k-j))
          {
            //解析
            Position.Date = (*(pBuffer+j+1)-0x30)*10+(*(pBuffer+j+2)-0x30);
            Position.Month = (*(pBuffer+j+3)-0x30)*10+(*(pBuffer+j+4)-0x30);
            Position.Year = (*(pBuffer+j+5)-0x30)*10+(*(pBuffer+j+6)-0x30);
            //Position.Hour += 8;//加8小时,行标检测不能屏蔽这段，因为行标检测时不校时。
            //if(Position.Hour >= 24)//实际使用时加8小时是在校时函数里
            //{
            //Position.Hour -= 24;
            //Position.Date++;
            // }
          }
          else if(1 == (k-j))//没有数据
          {
            
          }
          else
          {
            flag = 1;
          }
          break;
        }
      default : break;
      
      }
      j = i;//上一个逗号的位置
    }
    if(1 == flag)//如果检测到错误，则提前跳出
    {
      break;
    }
    
  }
  if(1 == flag)
  {
    GprmcParseErrorCount++;
  }
  else
  {
    GprmcParseErrorCount = 0;	
  }
}
/*********************************************************************
//函数名称	:GPS_GpggaParse(u8 *pBuffer, u8 BufferLen)
//功能		:解析GPGGA数据
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void Gps_GpggaParse(u8 *pBuffer, u8 BufferLen)
{
  u8	i = 0;
  u8	j = 0;
  u8	k = 0;
  u8	l = 0;
  u8	m = 0;
  s8	z = 0;
  u8	temp;
  s16	temp2;
  s16	temp3;
  u8	flag = 0;
  u8	count = 0;
  for(i=0; i<BufferLen; i++)
  {
    temp = *(pBuffer+i);
    if((0x0a == temp)||(0x0d == temp))
    {
      break;
    }
    else if('.' == temp)
    {
      l = i;
    }
    else if(',' == temp)
    {
      k = i;
      count++;
      switch(count)
      {
        
      case 8://第8个逗号，解析星数
        {
          if(2 == (k-j))//只有一位数
          {
            Position.SatNum = (*(pBuffer+j+1)-0x30);
          }
          else if(3 == (k-j))//两位数
          {
            Position.SatNum = (*(pBuffer+j+1)-0x30)*10+(*(pBuffer+j+2)-0x30);
          }
          else if(1 == (k-j))//没有数据
          {
            
          }
          else
          {
            flag = 1;
          }
          break;
        }
      case 9://第9个逗号，解析水平精度因子，精确到整数，小数抛弃
        {
          temp2 = 0;
          for(m=j+1; m<l; m++)
          {
            temp3 = (*(pBuffer+m)-0x30);
            for(z=0; z<l-m-1; z++)
            {
              temp3 = temp3 * 10;	
            }
            temp2 += temp3;
          }
          Position.HDOP = temp2;
          break;
        }
      case 10://第10个逗号，解析天线离海平面的高度，精确到整数，小数抛弃
        {
          temp2 = 0;
          if(*(pBuffer+j+1) == '-')//负海拔
          {
            for(m=j+2; m<l; m++)
            {
              temp3 = (*(pBuffer+m)-0x30);
              for(z=0; z<l-m-1; z++)
              {
                temp3 = temp3 * 10;	
              }
              temp2 += temp3;
            }
            Position.High = -temp2;
          }
          else//正海拔
          {
            for(m=j+1; m<l; m++)
            {
              temp3 = (*(pBuffer+m)-0x30);
              for(z=0; z<l-m-1; z++)
              {
                temp3 = temp3 * 10;	
              }
              temp2 += temp3;
            }
            Position.High = temp2;
          }
          break;
        }
      case 12://解析高程误差值
        {
          temp2 = 0;
          if(*(pBuffer+j+1) == '-')//负偏差
          {
            for(m=j+2; m<l; m++)
            {
              temp3 = (*(pBuffer+m)-0x30);
              for(z=0; z<l-m-1; z++)
              {
                temp3 = temp3 * 10;	
              }
              temp2 += temp3;
            }
            Position.HighOffset = -temp2;
          }
          else//正偏差
          {
            for(m=j+1; m<l; m++)
            {
              temp3 = (*(pBuffer+m)-0x30);
              for(z=0; z<l-m-1; z++)
              {
                temp3 = temp3 * 10;	
              }
              temp2 += temp3;
            }
            Position.HighOffset = temp2;
          }
        }
      default : break;
      }
      j = i;
    }
  }
  if(1 == flag)
  {
    GpggaParseErrorCount++;
  }
  else
  {
    GpggaParseErrorCount = 0;	
  }
}
/*********************************************************************
//函数名称	:GPS_CopygPosition(GPS_STRUCT *dest, GPS_STRUCT *src)
//功能		:拷贝一份最新的gps位置数据，它是一份有效的定位数据
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void Gps_CopygPosition(GPS_STRUCT *dest)
{
  dest->Year 		= gPosition.Year;		//年
  dest->Month 		= gPosition.Month;		//月
  dest->Date 		= gPosition.Date;		//日
  dest->Hour 		= gPosition.Hour;		//时
  dest->Minute 		= gPosition.Minute;		//分
  dest->Second 		= gPosition.Second;		//秒
  dest->North		= gPosition.North;		//1:北纬,0:南纬
  dest->Latitue_D 	= gPosition.Latitue_D;	//纬度,度
  dest->Latitue_F 	= gPosition.Latitue_F;	//纬度,分
  dest->Latitue_FX 	= gPosition.Latitue_FX;	//纬度,分的小数,单位为0.0001分
  dest->East 		= gPosition.East;		//1:东经0:西经
  dest->Longitue_D 	= gPosition.Longitue_D;	//经度,度,最大180度
  dest->Longitue_F 	= gPosition.Longitue_F;	//经度,分		
  dest->Longitue_FX 	= gPosition.Longitue_FX;	//经度,分的小数,单位为0.0001分
  dest->Speed             = gPosition.Speed;		
  dest->SpeedX 		= gPosition.SpeedX;		//速度的小数
  dest->Course 		= gPosition.Course;		//航向,单位为度
  dest->High 		= gPosition.High;		//海拔,单位为米
  dest->HighOffset 	= gPosition.HighOffset;		//海拔偏差,单位为米
  dest->SatNum 		= gPosition.SatNum;		//卫星数量
  dest->HDOP 		= gPosition.HDOP;		//水平精度因子
  dest->Status 		= gPosition.Status;	//1:有效定位 0:无效定位
  dest->Error 		= gPosition.Error;	//1:GPS模块出错，不发送数据  0:模块正常
}
/*********************************************************************
//函数名称	:GPS_CopyPosition(GPS_STRUCT *dest, GPS_STRUCT *src)
//功能		:拷贝一份最新收到的gps位置数据，它不一定是有效的定位数据
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void Gps_CopyPosition(GPS_STRUCT *dest)
{
  dest->Year 		= Position.Year;		//年
  dest->Month 		= Position.Month;		//月
  dest->Date 		= Position.Date;		//日
  dest->Hour 		= Position.Hour;		//时
  dest->Minute 		= Position.Minute;		//分
  dest->Second 		= Position.Second;		//秒
  dest->North		= Position.North;		//1:北纬,0:南纬
  dest->Latitue_D 	= Position.Latitue_D;	//纬度,度
  dest->Latitue_F 	= Position.Latitue_F;	//纬度,分
  dest->Latitue_FX 	= Position.Latitue_FX;	//纬度,分的小数,单位为0.0001分
  dest->East 		= Position.East;		//1:东经0:西经
  dest->Longitue_D 	= Position.Longitue_D;	//经度,度,最大180度
  dest->Longitue_F 	= Position.Longitue_F;	//经度,分		
  dest->Longitue_FX 	= Position.Longitue_FX;	//经度,分的小数,单位为0.0001分
  dest->Speed 		= Position.Speed;		//速度,单位为海里/小时
  dest->SpeedX 		= Position.SpeedX;		//速度的小数
  dest->Course 		= Position.Course;		//航向,单位为度
  dest->High 		= Position.High;		//海拔,单位为米
  dest->HighOffset 	= Position.HighOffset;		//海拔偏差,单位为米
  dest->SatNum 		= Position.SatNum;		//卫星数量
  dest->HDOP 		= Position.HDOP;		//水平精度因子
  dest->Status 		= Position.Status;	//1:有效定位 0:无效定位
  dest->Error 		= Position.Error;	//1:GPS模块出错，不发送数据  0:模块正常
}
/*********************************************************************
//函数名称	:GPS_UpdatagPosition(void)
//功能		:更新变量gPosition
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void Gps_UpdatagPosition(void)
{
  gPosition.Year 		= Position.Year;		//年
  gPosition.Month 	= Position.Month;		//月
  gPosition.Date 		= Position.Date;		//日
  gPosition.Hour 		= Position.Hour;		//时
  gPosition.Minute 	= Position.Minute;		//分
  gPosition.Second 	= Position.Second;		//秒
  gPosition.North		= Position.North;		//1:北纬,0:南纬
  gPosition.Latitue_D 	= Position.Latitue_D;	//纬度,度
  gPosition.Latitue_F 	= Position.Latitue_F;	//纬度,分
  gPosition.Latitue_FX 	= Position.Latitue_FX;	//纬度,分的小数,单位为0.0001分
  gPosition.East 		= Position.East;		//1:东经0:西经
  gPosition.Longitue_D 	= Position.Longitue_D;	//经度,度,最大180度
  gPosition.Longitue_F 	= Position.Longitue_F;	//经度,分		
  gPosition.Longitue_FX 	= Position.Longitue_FX;	//经度,分的小数,单位为0.0001分
  gPosition.Speed 	= Position.Speed;		//速度,单位为海里/小时
  gPosition.SpeedX 	= Position.SpeedX;		//速度的小数
  gPosition.Course 	= Position.Course;		//航向,单位为度
  gPosition.High 		= Position.High;		//海拔,单位为米
  gPosition.HighOffset    = Position.HighOffset;		//海拔偏差,单位为米
  gPosition.SatNum 	= Position.SatNum;		//卫星数量
  gPosition.HDOP 		= Position.HDOP;		//水平精度因子
  gPosition.Status 	= Position.Status;	//1:有效定位 0:无效定位
  gPosition.Error 	= Position.Error;	//1:GPS模块出错，不发送数据  0:模块正常
}
//仅仅更新GPS时间
void Gps_UpdateOnlyTime(void)
{
  gPosition.Year 		= Position.Year;		//年
  gPosition.Month 	= Position.Month;		//月
  gPosition.Date 		= Position.Date;		//日
  gPosition.Hour 		= Position.Hour;		//时
  gPosition.Minute 	= Position.Minute;		//分
  gPosition.Second 	= Position.Second;		//秒
}

/*********************************************************************
//函数名称	:GPS_ReadGpsSpeed(void)
//功能		:获取GPS速度
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
u8 Gps_ReadSpeed(void)
{
  u8	GpsSpeed;
  u8      Speed;
  
  GpsSpeed = (gPosition.Speed*18520+gPosition.SpeedX*1852)/10000;
  
  //if(Speed < 3)//GPS速度小于3公里/小时都强制等于0
  if(GpsSpeed <= 5)//GPS速度小于5公里/小时都强制等于0,dxl,2014.7.26
  {
    GpsSpeed = 0;
  }
  
  Speed = GPS_GpsSpeedHanlde(GpsSpeed);
#if(TACHOGRAPHS_19056_TEST)
  return Tachographs_Test_Get_Speed(); 
#else
  return Speed;
#endif
}
/*********************************************************************
//函数名称	:GPS_GpsSpeedHanlde
//功能		:GPS速度去漂移处理
//输入		:处理前速度
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:处理后速度
//备注		:根据与广州伊爱约定的去漂移规则处理
*********************************************************************/
static u8 GPS_GpsSpeedHanlde(u8 GpsSpeed)
{
  u8 Speed;
  u8 PulseSpeed;
  
  Speed = GpsSpeed;
  
  if(GpsSpeed >= 100)//GPS速度超过100km/h才做处理
  {
    PulseSpeed = Pulse_GetSpeed();
    if(0 == PulseSpeed)//脉冲速度为0时
    {
      if(GpsSpeed <= 110)
      {
        Speed = GpsSpeed;
      }
      else
      {
        Speed = 0;//要求同步清除导航标志
        Io_WriteStatusBit(STATUS_BIT_NAVIGATION, RESET);
      }
    }
    else//脉冲速度不为0时
    {
      if(PulseSpeed < GpsSpeed)
      {
        Speed = PulseSpeed;
      }
      else
      {
        Speed = GpsSpeed;
      }
    }
  }
  
  return Speed;
}
/*********************************************************************
//函数名称	:GPS_ReadGpsStatus(void)
//功能		:获取GPS的状态
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
u8 Gps_ReadStatus(void)
{
  return gPosition.Status;
}
/*********************************************************************
//函数名称	:GPS_ReadGpsCourse(void)
//功能		:获取GPS的方向
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
u16 Gps_ReadCourse(void)
{
  return gPosition.Course;
}
/*********************************************************************
//函数名称	:GPS_ReadGpsCourseDiv2(void)
//功能		:获取GPS的方向（除以了2）
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
u8 Gps_ReadCourseDiv2(void)
{
  u8	Course;
  
  Course = gPosition.Course >> 1;
  
  return Course;
}
/*********************************************************************
//函数名称	:GPS_PowerOnUpdataPosition(void)
//功能		:上电更新经纬度
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void Gps_PowerOnUpdataPosition(void)
{
  u8	Buffer[12];
  u8	PramLen;
  
  //PramLen = EepromPram_ReadPram(E2_LAST_LOCATION_ID, Buffer);
  PramLen = FRAM_BufferRead(Buffer, 11, FRAM_LAST_LOCATION_ADDR);
  if(11 != PramLen)//数据错误
  {
    
  }
  else
  {
    gPosition.North      	= Buffer[0];	//1:北纬0:南纬,1字节 
    gPosition.Latitue_D  	= Buffer[1];//度,1字节
    gPosition.Latitue_F  	= Buffer[2];//分,1字节
    gPosition.Latitue_FX 	= Buffer[3]*256 + Buffer[4];//分小数部分,单位为0.0001分,2字节,高字节
    gPosition.East	 	    = Buffer[5];//1:东经0:西经,1字节
    gPosition.Longitue_D 	= Buffer[6];//度 最大180度,1字节,
    gPosition.Longitue_F 	= Buffer[7];//分，1字节，			
    gPosition.Longitue_FX 	= Buffer[8]*256 + Buffer[9];//经度分的小数部分,单位为0.0001分,2字节,高字节
    gPosition.Speed = 0;
    gPosition.SpeedX = 0;
    gPosition.Course = 0;
    gPosition.High = 0;
    gPosition.HighOffset = 0;
    gPosition.SatNum = 0;
    gPosition.HDOP = 0;
    gPosition.Status =  0; 
    gPosition.Error = 0;
  }
}
/*********************************************************************
//函数名称	:GPS_SavePositionToFram(void)
//功能		:把位置保存到eeprom中
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:执行条件ACC ON->ACC OFF || 导航->不导航
*********************************************************************/
void Gps_SavePositionToFram(void)
{
  u8	Buffer[12];
  
  Buffer[0] = gPosition.North;	//1:北纬0:南纬,1字节 
  Buffer[1] = gPosition.Latitue_D;//度,1字节
  Buffer[2] = gPosition.Latitue_F;//分,1字节
  Buffer[3] = (gPosition.Latitue_FX&0xff00)>>8;//分小数部分,单位为0.0001分,2字节,高字节
  Buffer[4] = gPosition.Latitue_FX&0x00ff;//分小数部分,单位为0.0001分,2字节,低字节
  Buffer[5] = gPosition.East;//1:东经0:西经,1字节
  Buffer[6] = gPosition.Longitue_D;//度，1字节
  Buffer[7] = gPosition.Longitue_F;//1字节			
  Buffer[8] = (gPosition.Longitue_FX&0xff00)>>8;//经度分的小数部分,单位为0.0001分,2字节,高字节
  Buffer[9] = gPosition.Longitue_FX&0x00ff;//经度分的小数部分,单位为0.0001分,2字节,低字节
  Buffer[10]= gPosition.Status ;	//1:有效定位 0:无效定位
  
  FRAM_BufferWrite(FRAM_LAST_LOCATION_ADDR, Buffer, 11);
  
}
/*********************************************************************
//函数名称	:Gps_GetRunFlag
//功能		:获取去漂移行驶标志
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:1为行驶，0为停驶
//备注		:
*********************************************************************/
u8 Gps_GetRunFlag(void)
{
  return GpsRunFlag;
}

/*********************************************************************
//函数名称	:Gps_IsInRunning
//功能		:判断当前是处于行驶状态还是停驶状态
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:1为行驶，0为停驶
//备注		:1秒调用1次
*********************************************************************/
void Gps_IsInRunning(void)
{
  
  static u32 UpCount = 0;
  static u32 DownCount = 0;
  
  u8  Speed;
  
  Speed = Position.Speed;
  if(Speed > 5)//dxl,2014.8.26,原来是速度大于0,现在纠正为5km/h
  {
    DownCount = 0;
    UpCount++;
    if(UpCount >= 10)
    {
      GpsRunFlag = 1;//大于0持续10秒为行驶
    }
  }
  else
  {
    UpCount = 0;
    DownCount++;
    if(DownCount >= 10)
    {
      GpsRunFlag = 0;//等于0持续10秒为停驶
    }
  }
}
/*********************************************************************
//函数名称	:GpsParse_EvTask(void)
//功能		:GPS解析任务
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void Gps_EvTask(void)
{
  u8	VerifyFlag = 0;
  u8	flag;
	//static u8 lastFlag = 0;
  u8	Num;
  u8      GpsSpeed;
  u8      PulseSpeed;
  u32	s1;
  u32	s2;
  u8  ACC;//增加ACC去漂移,dxl,2014.7.25
  
  static  u16	DriftCount = 0;
  static  u16	noNavigationCount = 0;//不导航计数，导航或校验出错计数清0
  static  u16     VerifyErrorCount = 0;//校验错误计数，校验正确清0
  static  u16     noUpdatePositionCount = 0;//定位模块给的是导航标志，但没有达到更新经纬度的要求
  static  u32     NavigationCount = 0;//dxl,2014.7.28,导航点计数，不导航或校验出错计数清0，该变量用于从不导航到导航滤除前面3个导航点 
  //static u8 rtcLastTime = 0xff;
  //static u8 rtcAdjCnt = 0;
  u32 Lat_Temp = 0ul;
  u32 Log_Temp = 0ul;
  
  ACC = Io_ReadStatusBit(STATUS_BIT_ACC);
  
  
  //判断数据是否有效
  VerifyFlag = Gps_DataIsOk(GprmcBuffer, GPRMC_BUFFER_SIZE);
  if(1 != VerifyFlag)
  {
    GprmcVerifyErrorCount++;
  }
  else
  {
    GprmcVerifyErrorCount = 0;
    //判断数据是否有效
    VerifyFlag = Gps_DataIsOk(GpggaBuffer, GPGGA_BUFFER_SIZE);
    if(1 != VerifyFlag)
    {
      GpggaVerifyErrorCount++;
    }
    else
    {
      GpggaVerifyErrorCount = 0;
    }
  }
  
  //校验是否通过
  if(1 == VerifyFlag)//通过
  {
    VerifyErrorCount = 0;
    //行标检测时屏蔽结束
    GpsParseEnterErrorCount = 0;
    //清零模块出错标志
    Io_WriteAlarmBit(ALARM_BIT_GNSS_FAULT, RESET);
    
    if((0 == GpsOnOffFlag)||
       ((1 == GpsOnOffFlag)&&(0 == AccOffGpsControlFlag)&&(0 == ACC)))//GPS模块关闭状态,dxl,2014.9.24去掉ACC OFF时GPS开启5分钟期间的解析    
    {
      //速度清0
      Position.Speed = 0;
      gPosition.Speed = 0;  
      Position.SpeedX = 0;
      gPosition.SpeedX = 0;  
      return ;
    }
    
    //判断是否定位
    flag = Gps_GprmcIsLocation(GprmcBuffer, GPRMC_BUFFER_SIZE);
    if(1 != flag)
    {
      
      NavigationCount = 0;
      //置位忙标志
      //GprmcBufferBusyFlag = 1;
      //解析
      //Gps_GprmcParse(GprmcBuffer, GPRMC_BUFFER_SIZE);//为行标送检添加
      //清除忙标志
      //GprmcBufferBusyFlag = 0;
      
      LocationFlag = 0;//当前点导航状态
      //从定位到不定位也校时
      if(LastLocationFlag)
      {
				Timer10msStart(TIMER_10MS_ID_gps,50,TIMER_MODE_ONCE,Gps_AdjustRtc);
      }
#ifdef HUOYUN_DEBUG_OPEN
      //不导航,导航标志清0
      Io_WriteStatusBit(STATUS_BIT_NAVIGATION, RESET);//不导航立即显示
#endif
      noNavigationCount++;
      if(noNavigationCount >= 10)//连续10秒种不导航,速度清0
      {
        noNavigationCount = 0;
        //速度清0
        Position.Speed = 0;
        gPosition.Speed = 0;
        Position.SpeedX = 0;
        gPosition.SpeedX = 0;
        //不导航,导航标志清0
        Io_WriteStatusBit(STATUS_BIT_NAVIGATION, RESET);
      }
      //有短路或断路报警的话,速度和导航标志清0
      if((1 == Io_ReadAlarmBit(ALARM_BIT_ANT_SHUT))
         ||(1 == Io_ReadAlarmBit(ALARM_BIT_ANT_SHORT))
           ||(0 == StopExcursionEnable))
      {
        StopExcursionEnable = 0;
        gPosition.Status = 0;
        //不导航,导航标志清0
        Io_WriteStatusBit(STATUS_BIT_NAVIGATION, RESET);
      }
    }
    else
    {
      noNavigationCount = 0;
      //*************解析GPRMC************
      //置位忙标志
      GprmcBufferBusyFlag = 1;
      //解析
      Gps_GprmcParse(GprmcBuffer, GPRMC_BUFFER_SIZE);
      //清除忙标志
      GprmcBufferBusyFlag = 0;
      //记录时间
      PositionTime = RTC_GetCounter();
      //*************解析GPGGA*************
      //置位忙标志
      GpggaBufferBusyFlag = 1;
      //解析
      Gps_GpggaParse(GpggaBuffer, GPGGA_BUFFER_SIZE);
      //清除忙标志
      GpggaBufferBusyFlag = 0;
      //**************去漂移**********************
      //行标检测时屏蔽开始
      flag = 0;
      //v=s/t >= 200km/h(55m/s)，认为是漂移,1分等价于1海里，即1852米
#define	DRIFT_SPEED_THRESHOLD	44	//44m/s <-----> 160km/h
      //纬度距离
      s1 = ((Position.Latitue_D*60+Position.Latitue_F)*1000+Position.Latitue_FX/10)*2;//用2代替1.852
      s2 = ((gPosition.Latitue_D*60+gPosition.Latitue_F)*1000+gPosition.Latitue_FX/10)*2; 
      if((s1 > s2)&&(0 != s1)&&(0 != s2))
      {
        
        if(((s1-s2)/(PositionTime - gPositionTime) > DRIFT_SPEED_THRESHOLD)&&(0 != Position.Latitue_D)&&(0 != gPosition.Latitue_D)&&(0 != gPositionTime))//刚上电没导航过经维度为0
        {
          flag = 1;//是漂移点
        }
      }
      else if((0 != s1)&&(0 != s2))
      {
        if(((s2-s1)/(PositionTime - gPositionTime) > DRIFT_SPEED_THRESHOLD)&&(0 != Position.Latitue_D)&&(0 != gPosition.Latitue_D)&&(0 != gPositionTime))
        {
          flag = 1;//是漂移点
        }
      }
      //经度距离
      s1 = ((Position.Longitue_D*60+Position.Longitue_F)*1000+Position.Longitue_FX/10)*2;//用2代替1.852
      s2 = ((gPosition.Longitue_D*60+gPosition.Longitue_F)*1000+gPosition.Longitue_FX/10)*2; 
      if((s1 > s2)&&(0 != s1)&&(0 != s2))
      {
        if(((s1-s2)/(PositionTime - gPositionTime) > DRIFT_SPEED_THRESHOLD)&&(0 != Position.Longitue_D)&&(0 != gPosition.Longitue_D)&&(0 != gPositionTime))
        {
          flag = 1;//是漂移点
        }
      }
      else if((0 != s1)&&(0 != s2))
      {
        if(((s2-s1)/(PositionTime - gPositionTime) > DRIFT_SPEED_THRESHOLD)&&(0 != Position.Longitue_D)&&(0 != gPosition.Longitue_D)&&(0 != gPositionTime))
        {
          flag = 1;//是漂移点
        }
      }
      //检测速度变化,当前速度小于上一次的速度变化率不做限制
      //if(Position.Speed >= LastLocationSpeed)
      if((Position.Speed > LastLocationSpeed)&&(0 != gPositionTime))//dxl,2013.6.4
      {
        if((Position.Speed-LastLocationSpeed) >= (PositionTime - gPositionTime)*6)
        {
          flag = 1;
        }
      }
      
      //以上速度，加速度去漂移，当连续300个点（5分钟）都漂移时可能是时间错误，则不去漂移，先纠正时间
      //有天线断路发生的话需要重新定位，StopExcursionEnable = 0,dxl,2013.10.16
      if(1 == Io_ReadAlarmBit(ALARM_BIT_ANT_SHUT))
      {
        StopExcursionEnable = 0;
      }
      if(1 == flag)
      {
        DriftCount++;
        if(DriftCount == 10)
        {
          //速度清0
          gPosition.Speed = 0;  
          gPosition.SpeedX = 0;  
        }
        else if(DriftCount >= 300)
        {
          flag = 0;
        }    
      }
      else
      {
        DriftCount = 0;
        
        if(0 == Gps_GetRunFlag())//判断是否行驶,1为行驶
        {
          if(1 == StopExcursionEnable)
          {
            flag = 1;//停驶不更新数据 
            Gps_UpdateOnlyTime();
          }
          //速度清0,注意这里不能清局部变量Position.Speed,dxl,2015.4.20
          gPosition.Speed = 0;  
          gPosition.SpeedX = 0;  
        }
        else
        {
          StopExcursionEnable = 0;
        }
      }
      
      
      
      
      if(0 == LastLocationFlag)
      {
        Num = 3;
      }
      else
      {
        Num = 2;
      }
      if(0 == AccOffGpsControlFlag)//ACC OFF时GPS模块关闭
      {
        //受ACC状态控制
      }
      else
      {
        //不受ACC状态控制
        ACC = 1;
      }
      /*天朝经纬度范围，北纬3~53，东经73~135*/
      Lat_Temp = (Position.Latitue_FX*100UL+Position.Latitue_F*1000000)/60+(Position.Latitue_D*1000000UL);//纬度
      Log_Temp = (Position.Longitue_FX*100UL+Position.Longitue_F*1000000)/60+(Position.Longitue_D*1000000UL);//经度
      if((Lat_Temp < 3000000)||(Lat_Temp > 53000000))
        flag = 1;
      if((Log_Temp < 73000000)||(Log_Temp > 135000000))
        flag = 1;
      if((Position.SatNum>Num)&&(Position.HDOP<=8)&&(Position.Speed<=110)&&(0 == flag)&&(1 == ACC))
      {
        //行标检测时屏蔽开始
        //更新变量
        NavigationCount++;
        
        if(NavigationCount > 3)//前3个导航点滤除掉
        {
          if(DriftCount >= 300)
          {
            DriftCount = 0;
          }
          noUpdatePositionCount = 0;
          Gps_UpdatagPosition();
          //记录时间
          gPositionTime = RTC_GetCounter();
          //上一次速度
          LastLocationSpeed = gPosition.Speed;
          //置位导航标志
          Io_WriteStatusBit(STATUS_BIT_NAVIGATION, SET);
          LocationFlag = 1;
					//从不定位到定位
		      if((0 == LastLocationFlag)&&(0 == AdjustRtcFlag))
		      {
		      	AdjustRtcFlag = 1;
						Timer1sStart(TIMER_1S_ID_gps,5,TIMER_MODE_ONCE,Gps_AdjustRtc);
						Gps_SavePositionToFram();
		      }
          //以下为配合广州伊爱去速度漂移而添加
          GpsSpeed = (gPosition.Speed*18520+gPosition.SpeedX*1852)/10000;
          PulseSpeed = Pulse_GetSpeed();
          if((GpsSpeed >= 110)&&(0 == PulseSpeed))
          {
            Io_WriteStatusBit(STATUS_BIT_NAVIGATION, RESET);
            LocationFlag = 0;
          }
          //////////////////////end 广州去速度漂移添加//////////
          //找到第一个静止点,开启停车去漂移，行标检测时屏蔽开始
          //if(0 == gPosition.Speed)
          if(0 == Gps_GetRunFlag())//dxl,2014.7.28
          {
            StopExcursionEnable = 1;
          }
        }
      }
      else
      {
        LocationFlag = 0;
        noUpdatePositionCount++;
        if(noUpdatePositionCount >= 10)
        {
          noUpdatePositionCount = 0;
          gPosition.Speed = 0;
          gPosition.SpeedX = 0;
        }
      }            
      
      //行标检测时屏蔽开始
    }
		//lastFlag = flag;
    //行标检测时屏蔽开始
  }
  else//校验不通过
  {
    noNavigationCount = 0;
    NavigationCount = 0;
    VerifyErrorCount++;
    if(VerifyErrorCount >= 10)
    {
      VerifyErrorCount = 0;
      //速度清0
      Position.Speed = 0;
      gPosition.Speed = 0;
      Position.SpeedX = 0;
      gPosition.SpeedX = 0;
    }
    //不导航
    //gPosition.Status = 0;
    //LocationFlag = 0;
    //导航标志清0
    //Io_WriteStatusBit(STATUS_BIT_NAVIGATION, RESET);,打印时会进入这里，
    
  }	
  
  //记录上1次定位状态
  LastLocationFlag = gPosition.Status;
  
  
}
/*********************************************************************
//函数名称	:Gps_TimeTask(void)
//功能		:GPS时间任务
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:1秒调度1次
*********************************************************************/
FunctionalState  Gps_TimeTask(void)
{
  static u8 	count = 0;
  static u32	AccOffGpsOffCount = 0;
  static u8   AccOffSaveFlag = 0;
  static u8   ErrorCount = 0;
  static u32	EnterCount = 0;
  u8	ACC;
  
  
  
  
  
  ACC = Io_ReadStatusBit(STATUS_BIT_ACC);
  
  static u8 AccOnFirstLocationReport = 0;//ACC ON第1次定位需立即上报1条位置信息
  if(0 == ACC)
  {
    AccOnFirstLocationReport = 0;
  }
  else
  {
    if(0 == AccOnFirstLocationReport)
    {
      if(1 == gPosition.Status) 
      {
        //SetEvTask(EV_REPORT); //触发警示,位置上报
        Report_UploadPositionInfo(CHANNEL_DATA_1);
        Report_UploadPositionInfo(CHANNEL_DATA_2);
        AccOnFirstLocationReport = 1;
      }
    }
  }
  
  
  
  //保存最后一个有效点到铁电
  count++;
  if(count > 5)
  {
    count = 0;
    if(0 == AccOffGpsControlFlag)//受ACC控制
    {
      if(0 == ACC)//ACC 关
      {
        if(0 == AccOffSaveFlag)
        {
          Gps_SavePositionToFram();
          AccOffSaveFlag = 1;
        }
      }
      else// ACC开
      {
        AccOffSaveFlag = 0;
        Gps_SavePositionToFram();
      }
    }
    else
    {
      AccOffSaveFlag = 0;
      Gps_SavePositionToFram();
    }
  }
  
  if(0 == ACC)
  {
    if(0 == AccOffGpsOffCount%7200)
    {
      GPS_POWER_ON();
    }
    else if(240 == AccOffGpsOffCount%7200)
    {
      if(0 == AccOffGpsControlFlag)
      {
        //断电报警时ACC检测为OFF,但此时不应关闭GPS
        if(0 == Io_ReadAlarmBit(ALARM_BIT_POWER_SHUT))
        {
          GPS_POWER_OFF(); 
        }
        else
        {
          
        }
      }
    }
    AccOffGpsOffCount++;
  }
  else
  {
    AccOffGpsOffCount = 0;
    if(0 == Io_ReadAlarmBit(ALARM_BIT_GNSS_FAULT))//dxl,2014.10.8
    {
      GPS_POWER_ON();
    }
    
  }
  
  
  //***************出错处理************************
  //次数加1
  if(1 == GpsOnOffFlag)//GPS模块开
  {
    GpsParseEnterErrorCount++;
    
    //*******************每隔50小时校时1次，避免行驶记录仪检测过程中校时*********
    EnterCount++;
    if((EnterCount >= 3600*50)&&(1 == LocationFlag))
    {
      EnterCount = 0;
      //校时，行标检测时需屏蔽这行
#ifdef HUOYUN_DEBUG_OPEN
#else
      Gps_AdjustRtc();
#endif
    }
  }
  
  if((GpsParseEnterErrorCount >= 40)
     ||(GprmcVerifyErrorCount >= 40)
       ||(GpggaVerifyErrorCount >= 40))//一旦出错则重新启动模块,先关模块5秒钟,后再打开
  {
    
    //不导航
    gPosition.Status = 0;
    //速度清0
    gPosition.Speed = 0;
    gPosition.SpeedX = 0;
    //不导航
    Position.Status = 0;
    //速度清0
    Position.Speed = 0;
    Position.SpeedX = 0;
    //保存上一次状态
    LastLocationFlag = 0;
    LocationFlag = 0;
    //导航标志清0
    Io_WriteStatusBit(STATUS_BIT_NAVIGATION, RESET);
    //置位出错
    gPosition.Error = 1;
    //置位GPS模块异常标志
    Io_WriteAlarmBit(ALARM_BIT_GNSS_FAULT, SET);
    
    ErrorCount++;//dxl,2014.10.8
    if(1 == ErrorCount)
    {
      GPS_POWER_OFF();
    }
    else if(ErrorCount >= 5)
    {
      GPS_POWER_ON();
      //错误计数变量清0
      GpsParseEnterErrorCount = 0;
      GprmcVerifyErrorCount = 0;
      GpggaVerifyErrorCount = 0;   
      ErrorCount = 0;
    } 
  }
  else
  {
    ErrorCount = 0;
  }
  //*********判断当前是行驶还是停驶*********
  Gps_IsInRunning();
  
  
  return ENABLE;
}

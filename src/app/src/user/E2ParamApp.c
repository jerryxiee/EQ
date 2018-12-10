/*******************************************************************************
* File Name:			E2ParamApp.c 
* Function Describe:	E2参数应用
* Relate Module:		
* Writer:				Joneming
* Date:				2013-10-16
* ReWriter:			
* Date:				
*******************************************************************************/
#include "include.h"
/*******************************************************************************/ 
////////////////////////////////////////////
#if 1

#define NEED_CONVERT_OLD_SELF_DEFINE_ID     //如果定义,则转换之前自定义参数ID为0x0200开始的参数
#define NEED_CONVERT_SUOMEI_PARAM_ID        //如果定义,则转换索美自定义参数ID为0xC000开始的参数
////////////////////
#define E2_PRAM_SUOMEI_START_ID 0xC000//索美参数的开始ID
#define E2_PRAM_SUOMEI_END_ID   0xD000//索美参数的结束ID
/////////////////////////////////////
//////////////////////////////////////////
extern u16 DelayAckCommand ;//延时应答命令
extern const u8 EepromPramLength[];
////////////////////////////////
#define E2PARAM_START_YEAR		00//索美时间格式转换2000开始
#define STANDARD_TIME_DIFF      28800//(北京与格林威治标准时间为8个时差8*3600)
/*********************************************************************
//函数名称	:ConverseGmtime
//功能		:把结构体类型时间转换成32位无符号的统一计数值
//输入		:TIME_T 结构体类型时间
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:32位无符号的统一计数值
//备注		:
*********************************************************************/
u32 E2ParamApp_ConverseGmtime(TIME_T *tt)
{
  s16 i;
  s16 year;
  u32 TotalDay;
  u32 TotalSec;
  
  TotalDay = 0;
  TotalSec = 0;
  year = tt->year;
  
  if((year >= E2PARAM_START_YEAR)&&(year <= END_YEAR))	//判断年是否符合范围
  {
    for(i = E2PARAM_START_YEAR; i < tt->year; i++)	//计算year年之前的天数
    {
      TotalDay += (365+LeapYear(i));
    }
    TotalDay += YearDay(tt->year, tt->month, tt->day);	//加上year当年的天数
    TotalSec = (TotalDay-1)*DAY_SECOND + tt->hour*HOUR_SECOND+ tt->min*60 + tt->sec;	//计算总的秒数
  }
  
  return (TotalSec-STANDARD_TIME_DIFF);
}
/*********************************************************************
//函数名称	:Gmtime
//功能		:把32位无符号的统一计数值转换成结构体类型时间
//输入		:指向TIME_T类型的指针，计数值
//输出		:指向TIME_T类型的指针
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void E2ParamApp_Gmtime(TIME_T *tt, u32 counter)
{
  s16 i;
  u32 sum ;
  u32 temp;
  u32 counter1 = 0;
  u32 timeVal;
  u8 flag;
  unsigned short jdays[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
  
  timeVal=counter+STANDARD_TIME_DIFF;
  //////////////////
  if(timeVal > 0xBC191380)//超过开始年的一百年,则不再转换
  {
    tt->year =E2PARAM_START_YEAR;
    tt->month = 1;
    tt->day = 1;
    tt->hour = 0;
    tt->min = 0;
    tt->sec= 0;
    return;//超过开始年的一百年,则不再转换
  }
  ///////////////////////
  sum = 0;
  temp = 0;    
  for(i=E2PARAM_START_YEAR; sum<=timeVal&&i<END_YEAR; i++)   /* 计算年份 */
  {
    sum += (LeapYear(i)*DAY_SECOND + YEAR_SECOND);
    if(sum <= timeVal)
    {
      temp= sum;
    }
  }
  ///////////////////////////
  tt->year =i-1;
  
  flag=LeapYear(tt->year);
  counter1=timeVal-temp;
  /////////////////////////
  sum=counter1;
  counter1=counter1%DAY_SECOND;
  temp =(sum-counter1)/DAY_SECOND;
  temp++;//TotalSec = (TotalDay-1)*DAY_SECOND + tt->hour*HOUR_SECOND+ tt->min*60 + tt->sec;	//计算总的秒数
  for(i=12; i>0; i--)//
  {            
    if((temp>(jdays[i-1]+flag)&&i>2)||(temp>jdays[i-1]&&i<=2))            
    {
      tt->month=i;
      break;
    }
  }
  //////////////////////////////////////
  tt->day=temp-jdays[tt->month-1];
  if(tt->month>2)tt->day-=flag;
  
  tt->hour =counter1/HOUR_SECOND;
  ///////////////////////////
  counter1=counter1%HOUR_SECOND;
  /////////////////////
  tt->min =counter1/MIN_SECOND;
  
  tt->sec=counter1%MIN_SECOND;
  /////////////////////
}
//////////////////////////////////////////
/*************************************************************
** 函数名称: E2ParamApp_ConvertOldSelfDefinePramID
** 功能描述: 转换旧的自定义参数ID(0x0200开始)
** 入口参数: 参数ID
** 出口参数: 
** 返回参数: 我们新自定义的ID(0xF200开始)
** 全局变量: 
** 调用模块: 
*************************************************************/
#ifdef NEED_CONVERT_OLD_SELF_DEFINE_ID
unsigned short E2ParamApp_ConvertOldSelfDefinePramID(unsigned short usParamID)
{
  unsigned short paramID;
  paramID=usParamID;
  //////////////////////
  if(0x0200<=usParamID&&usParamID<=0x23B)//
  {
    paramID=usParamID+0xF000;
  }
  else
    if(0x0241==usParamID||usParamID==0x242)//32pin的LED接口\32pin的POS
    {
      paramID=usParamID+0xF03F;//0xF280/0xF281
    }
    else
      if(0x0243==usParamID)//载重限值
      {
        paramID=0xF240;
      }
      else
        if(0x0244==usParamID)//自定义报警屏蔽字
        {
          paramID=0xF243;
        }
        else
          if(0x0246==usParamID)//驾驶员未签到语音提醒，0 为关，1 为开。
          {
            paramID=0xF24C;
          }
          else
            if(0x0247<=usParamID&&usParamID<=0x249)//最小弯道角度、最大弯道角度、弯道速度限值
            {
              paramID=usParamID+0xF000-1;
            }
            else
              if(0x024A==usParamID||usParamID==0x24B)//关机模式时间参数、休眠电压
              {
                paramID=usParamID+0xF000;
              }  
  
  ////////////////////////
  return paramID;
}
#endif
#ifdef NEED_CONVERT_SUOMEI_PARAM_ID
/*************************************************************
** 函数名称: E2ParamApp_ConvertSuomeiPramID
** 功能描述: 把索美参数ID转成我们自定义的ID
** 入口参数: 索美参数ID
** 出口参数: 
** 返回参数: 我们自定义的ID
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned short E2ParamApp_ConvertSuomeiPramID(unsigned short usParamID)
{
  unsigned short paramID;
  paramID=usParamID;
  if((E2_PRAM_SUOMEI_START_ID > usParamID)||(usParamID>E2_PRAM_SUOMEI_END_ID))
  {
    return paramID;
  }
  else
  {
    if(0xC100== usParamID)//预警语音
    {
      paramID = E2_OVERSPEED_PREALARM_VOICE_ID;
    }
    else
      if(0xC101== usParamID)//报警语音
      {
        paramID = E2_OVERSPEED_ALARM_VOICE_ID;
      }
      else
        if(0xC110== usParamID)//超速报警提示单次重复播报的次数
        {
          paramID = E2_OVERSPEED_ALARM_NUMBER_ID;
        }
        else
          if(0xC111== usParamID)//超速报警提示时间间隔
          {
            paramID = E2_OVERSPEED_ALARM_GPROUP_TIME_ID;
          }
          else
            if(0xC112== usParamID)//超速预警提示持续时间阀值
            {
              paramID = E2_OVERSPEED_PREALARM_KEEPTIME_ID;
            }
            else
              if(0xC113== usParamID)//超速预警提示时间间隔
              {
                paramID = E2_OVERSPEED_PREALARM_GPROUP_TIME_ID;
              }
              else
                if(0xC114 == usParamID)//超速预警提示的次数
                {
                  paramID = E2_OVERSPEED_PREALARM_NUMBER_ID;
                }
                else
                  if(0xC115== usParamID)////超速预警提示值
                  {
                    //特殊处理
                  }
                  else
                    if(0xC21B == usParamID)//夜间时间定义
                    {
                      //特殊处理
                      //paramID = E2_FORBID_DRIVE_NIGHT_TIME_ID;
                    }
                    else
                      if(0xC220 == usParamID)//禁行时段到达提醒提前时间
                      {
                        paramID = E2_FORBID_DRIVE_PREALARM_TIME_ID;
                      }
                      else
                        if(0xC221 == usParamID)//禁行时段提醒时间间隔
                        {
                          paramID = E2_FORBID_DRIVE_PREALARM_GPOUP_TIME_ID;
                        }
                        else
                          if(0xC222 == usParamID)//夜间时间定义
                          {
                            //特殊处理
                            //paramID = E2_OVERSPEED_NIGHT_TIME_ID;
                          }
                          else
                            if(0xC223 == usParamID)//夜间超速值及预警值定义
                            {
                              paramID = E2_NIGHT_OVERSPEED_PERCENT_ID;
                            }
                            else
                              if(0xC224 == usParamID)//超时驾驶预警提示时间提前值
                              {
                                //特殊处理
                              }
                              else
                                if(0xC226 == usParamID)//超时驾驶预警提示单次重复播报的次数
                                {
                                  paramID = E2_TIRED_DRIVE_PREALARM_NUMBER_ID;
                                }
                                else
                                  if(0xC227 == usParamID)//超时驾驶预警提示时间间隔
                                  {
                                    paramID = E2_TIRED_DRIVE_PREALARM_GPOUP_TIME_ID;
                                  }
                                  else
                                    if(0xC228 == usParamID)//超时驾驶报警提示单次重复播报的次数
                                    {
                                      paramID = E2_TIRED_DRIVE_ALARM_NUMBER_ID;
                                    }
                                    else
                                      if(0xC229 == usParamID)//超时驾驶报警提示时间间隔
                                      {
                                        paramID = E2_TIRED_DRIVE_ALARM_GPOUP_TIME_ID;
                                      }        
                                      else
                                        if(0xC230 == usParamID)//禁行时段到达提醒语音
                                        {
                                          paramID = E2_FORBID_DRIVE_PREALARM_VOICE_ID;
                                        }
                                        else
                                          if(0xC231 == usParamID)//禁行时段到达报警语音
                                          {
                                            paramID = E2_FORBID_DRIVE_ALARM_VOICE_ID;
                                          }
                                          else
                                            if(0xC232 == usParamID)//禁行时段到达提醒播报的次数
                                            {
                                              paramID = E2_FORBID_DRIVE_PREALARM_NUMBER_ID;
                                            }
                                            else
                                              if(0xC234 == usParamID)//报警提示次数
                                              {
                                                paramID = E2_FORBID_DRIVE_ALARM_NUMBER_ID;
                                              }
                                              else
                                                if(0xC235 == usParamID)//禁行时段到达报警报时间间隔
                                                {
                                                  paramID = E2_FORBID_DRIVE_ALARM_GPOUP_TIME_ID;
                                                }                
                                                else///
                                                  if(0xC236 == usParamID)//预警时间
                                                  {
                                                    paramID = E2_NIGHT_DRIVE_PREALARM_TIME_ID;
                                                  }
                                                  else
                                                    if(0xC237 == usParamID)//夜间预警语音
                                                    {
                                                      paramID = E2_NIGHT_DRIVE_PREALARM_VOICE_ID;
                                                    }
                                                    else
                                                      if(0xC238 == usParamID)//夜间预警提示单次重复播报的次数
                                                      {
                                                        paramID = E2_NIGHT_DRIVE_PREALARM_NUMBER_ID;
                                                      }
                                                      else
                                                        if(0xC239 == usParamID)//夜间预警提示时间间隔
                                                        {
                                                          paramID = E2_NIGHT_DRIVE_PREALARM_GPOUP_TIME_ID;
                                                        }
                                                        else
                                                          if(0xC250 == usParamID)//停车判定速度(公里/小时)
                                                          {
                                                            paramID = E2_FORBID_DRIVE_RUN_SPEED_ID;
                                                          }
                                                          else
                                                            if(0xC251 == usParamID)//停车判断：持续运行时间
                                                            {
                                                              paramID = E2_FORBID_DRIVE_RUN_SPEED_KEEPTIME_ID;
                                                            }
                                                            else
                                                              if(0xC252 == usParamID)//报警次数，0：不播报，255：持续播报
                                                              {
                                                                paramID = E2_ROUTE_EXCURSION_ALARM_NUMBER_ID;
                                                              }
                                                              else
                                                                if(0xC253 == usParamID)//偏离线路报警语音时间间隔 
                                                                {
                                                                  paramID = E2_ROUTE_EXCURSION_ALARM_GPOUP_TIME_ID;
                                                                }
                                                                else
                                                                  if(0xC254 == usParamID)//报警语音
                                                                  {
                                                                    paramID = E2_ROUTE_EXCURSION_ALARM_VOICE_ID;
                                                                  }
                                                                  else
                                                                    if(0xC255 == usParamID)//夜间时间定义
                                                                    {
                                                                      //特殊处理
                                                                      //paramID = E2_TIRED_DRIVE_NIGHT_TIME_ID;
                                                                    }
                                                                    else
                                                                      if(0xC257 == usParamID)//夜间最长连续驾驶时间
                                                                      {
                                                                        paramID = E2_NIGHT_MAX_DRVIE_TIME_ID;
                                                                      }
                                                                      else
                                                                        if(0xC258 == usParamID)//夜间驾驶最小休息时间
                                                                        {
                                                                          paramID = E2_NIGHT_MIN_RELAX_TIME_ID;
                                                                        }
                                                                        else
                                                                          if(0xC259 == usParamID)//夜间预警语音 
                                                                          {
                                                                            paramID = E2_TIRED_DRIVE_PREALARM_VOICE_ID;
                                                                          }
                                                                          else
                                                                            if(0xC25A == usParamID)//报警内容
                                                                            {
                                                                              paramID = E2_TIRED_DRIVE_ALARM_VOICE_ID;
                                                                            }
                                                                            else
                                                                              if(0xC25B == usParamID)//事件项，保留，用于指定产生事件后上报时间间隔的事件项 BIT0位＝1：超速报警 
                                                                              {
                                                                                //特殊处理
                                                                              }
                                                                              else
                                                                                if(0xC25C == usParamID)//点火产生事件后上报时间间隔， 事件解除后恢复正常上报，单位:秒，设置 0，禁止本功能
                                                                                {
                                                                                  paramID = E2_OVERSPEED_ALARM_REPORT_TIME_ID;
                                                                                }        
    ////////////////////
    return paramID;
  }
}
/*************************************************************
** 函数名称: E2ParamApp_ConvertSuomeiPramID
** 功能描述: 把我们自定义的ID转成索美参数ID
** 入口参数: 自定义的ID
** 出口参数: 
** 返回参数: 索美参数ID
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned short E2ParamApp_ConvertPramIDToSuomeiID(unsigned short usParamID)
{
  unsigned short paramID;
  paramID=usParamID;    
  if(usParamID<E2_SYSTEM_DEFINE_ID_MAX)
  {
    return paramID;
  }
  else
    if((E2_PRAM_SUOMEI_START_ID <= usParamID)&&(usParamID<=E2_PRAM_SUOMEI_END_ID))
    {
      return paramID;
    }
    else
    {
      if(E2_OVERSPEED_PREALARM_VOICE_ID == usParamID)//预警语音
      {
        paramID = 0xC100;
      }
      else
        if(E2_OVERSPEED_ALARM_VOICE_ID == usParamID)//报警语音
        {
          paramID =  0xC101;
        }
        else
          if(E2_OVERSPEED_ALARM_NUMBER_ID == usParamID)//超速报警提示单次重复播报的次数
          {
            paramID = 0xC110;
          }
          else
            if(E2_OVERSPEED_ALARM_GPROUP_TIME_ID== usParamID)//超速报警提示时间间隔
            {
              paramID = 0xC111;
            }
            else
              if(E2_OVERSPEED_PREALARM_KEEPTIME_ID == usParamID)//超速预警提示持续时间阀值
              {
                paramID = 0xC112;
              }
              else
                if(E2_OVERSPEED_PREALARM_GPROUP_TIME_ID== usParamID)//超速预警提示时间间隔
                {
                  paramID = 0xC113;
                }
                else
                  if(E2_OVERSPEED_PREALARM_NUMBER_ID == usParamID)//超速预警提示的次数
                  {
                    paramID = 0xC114;
                  }
                  else
                    if(E2_NIGHT_MAX_SPEED0_ID+1 == usParamID)////超速预警提示值
                    {
                      paramID = 0xC115;//特殊处理
                    }
                    else
                      if(E2_FORBID_DRIVE_NIGHT_TIME_ID == usParamID)//夜间时间定义
                      {
                        paramID = 0xC21B;
                      }
                      else
                        if(E2_FORBID_DRIVE_PREALARM_TIME_ID == usParamID)//禁行时段到达提醒提前时间
                        {
                          paramID = 0xC220;
                        }
                        else
                          if(E2_FORBID_DRIVE_PREALARM_GPOUP_TIME_ID == usParamID)//禁行时段提醒时间间隔
                          {
                            paramID = 0xC221;
                          }
                          else
                            if(E2_OVERSPEED_NIGHT_TIME_ID == usParamID)//夜间时间定义
                            {
                              paramID = 0xC222;
                            }
                            else
                              if(E2_NIGHT_OVERSPEED_PERCENT_ID == usParamID)//夜间超速值及预警值定义
                              {
                                paramID = 0xC223;
                              }
                              else
                                if(E2_NIGHT_MAX_SPEED0_ID+2  == usParamID)//超时驾驶预警提示时间提前值
                                {
                                  paramID = 0xC224;//特殊处理
                                }
                                else
                                  if(E2_TIRED_DRIVE_PREALARM_NUMBER_ID == usParamID)//超时驾驶预警提示单次重复播报的次数
                                  {
                                    paramID = 0xC226;
                                  }
                                  else
                                    if(E2_TIRED_DRIVE_PREALARM_GPOUP_TIME_ID == usParamID)//超时驾驶预警提示时间间隔
                                    {
                                      paramID = 0xC227;
                                    }
                                    else
                                      if(E2_TIRED_DRIVE_ALARM_NUMBER_ID == usParamID)//超时驾驶报警提示单次重复播报的次数
                                      {
                                        paramID = 0xC228;
                                      }
                                      else
                                        if(E2_TIRED_DRIVE_ALARM_GPOUP_TIME_ID == usParamID)//超时驾驶报警提示时间间隔
                                        {
                                          paramID = 0xC229;
                                        }
                                        else
                                          if(E2_FORBID_DRIVE_PREALARM_VOICE_ID== usParamID)//禁行时段到达提醒语音
                                          {
                                            paramID = 0xC230;
                                          }
                                          else
                                            if(E2_FORBID_DRIVE_ALARM_VOICE_ID == usParamID)//禁行时段到达报警语音
                                            {
                                              paramID = 0xC231;
                                            }
                                            else
                                              if(E2_FORBID_DRIVE_PREALARM_NUMBER_ID == usParamID)//禁行时段到达提醒播报的次数
                                              {
                                                paramID = 0xC232;
                                              }
                                              else
                                                if(E2_FORBID_DRIVE_ALARM_NUMBER_ID == usParamID)//报警提示次数
                                                {
                                                  paramID = 0xC234;
                                                }
                                                else
                                                  if(E2_FORBID_DRIVE_ALARM_GPOUP_TIME_ID == usParamID)//禁行时段到达报警报时间间隔
                                                  {
                                                    paramID = 0xC235;
                                                  }
                                                  else///
                                                    if(E2_NIGHT_DRIVE_PREALARM_TIME_ID == usParamID)//预警时间
                                                    {
                                                      paramID = 0xC236;
                                                    }
                                                    else
                                                      if(E2_NIGHT_DRIVE_PREALARM_VOICE_ID == usParamID)//夜间预警语音
                                                      {
                                                        paramID = 0xC237;
                                                      }
                                                      else
                                                        if(E2_NIGHT_DRIVE_PREALARM_NUMBER_ID == usParamID)//夜间预警提示单次重复播报的次数
                                                        {
                                                          paramID = 0xC238;
                                                        }
                                                        else
                                                          if(E2_NIGHT_DRIVE_PREALARM_GPOUP_TIME_ID  == usParamID)//夜间预警提示时间间隔
                                                          {
                                                            paramID = 0xC239;
                                                          }
                                                          else
                                                            if(E2_FORBID_DRIVE_RUN_SPEED_ID  == usParamID)//停车判定速度(公里/小时)
                                                            {
                                                              paramID = 0xC250;
                                                            }
                                                            else
                                                              if(E2_FORBID_DRIVE_RUN_SPEED_KEEPTIME_ID == usParamID)//停车判断：持续运行时间
                                                              {
                                                                paramID = 0xC251;
                                                              }
                                                              else
                                                                if(E2_ROUTE_EXCURSION_ALARM_NUMBER_ID == usParamID)//报警次数，0：不播报，255：持续播报
                                                                {
                                                                  paramID = 0xC252;
                                                                }
                                                                else
                                                                  if(E2_ROUTE_EXCURSION_ALARM_GPOUP_TIME_ID == usParamID)//偏离线路报警语音时间间隔 
                                                                  {
                                                                    paramID = 0xC253;
                                                                  }
                                                                  else
                                                                    if(E2_ROUTE_EXCURSION_ALARM_VOICE_ID == usParamID)//报警语音
                                                                    {
                                                                      paramID = 0xC254;
                                                                    }
                                                                    else
                                                                      if(E2_TIRED_DRIVE_NIGHT_TIME_ID == usParamID)//夜间时间定义
                                                                      {
                                                                        paramID = 0xC255;
                                                                      }
                                                                      else
                                                                        if(E2_NIGHT_MAX_DRVIE_TIME_ID == usParamID)//夜间最长连续驾驶时间
                                                                        {
                                                                          paramID = 0xC257;
                                                                        }
                                                                        else
                                                                          if(E2_NIGHT_MIN_RELAX_TIME_ID == usParamID)//夜间驾驶最小休息时间
                                                                          {
                                                                            paramID = 0xC258;
                                                                          }
                                                                          else
                                                                            if(E2_TIRED_DRIVE_PREALARM_VOICE_ID == usParamID)//夜间预警语音 
                                                                            {
                                                                              paramID = 0xC259;
                                                                            }
                                                                            else
                                                                              if(E2_TIRED_DRIVE_ALARM_VOICE_ID == usParamID)//报警内容
                                                                              {
                                                                                paramID = 0xC25A;
                                                                              }
                                                                              else
                                                                                if(E2_REPORT_FREQ_EVENT_SWITCH_ID == usParamID)//事件项，保留，用于指定产生事件后上报时间间隔的事件项 BIT0位＝1：超速报警 
                                                                                {
                                                                                  paramID = 0xC25B;//特殊处理
                                                                                }
                                                                                else
                                                                                  if(E2_OVERSPEED_ALARM_REPORT_TIME_ID == usParamID)//点火产生事件后上报时间间隔， 事件解除后恢复正常上报，单位:秒，设置 0，禁止本功能
                                                                                  {
                                                                                    paramID = 0xC25C;
                                                                                  }
      ////////////////////
      return paramID;
    }
}
#endif
/*************************************************************
** 函数名称: E2ParamApp_WriteTimeParam
** 功能描述: 根据索美时间格式写时间参数
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned char E2ParamApp_WriteTimeParam(unsigned short usParamID,unsigned char *data,unsigned char datalen)
{
  unsigned char buffer[20]={0};
  unsigned long temp;
  TIME_T stTime;    
  if(datalen!=10)return 0;
  memcpy(buffer,data,2);
  temp = Public_ConvertBufferToLong(&data[2]);
  E2ParamApp_Gmtime(&stTime,temp);
  Public_ConvertTimeToBCDEx(stTime,&buffer[2]);
  /////////////////////////////////
  temp = Public_ConvertBufferToLong(&data[6]);
  E2ParamApp_Gmtime(&stTime,temp);
  Public_ConvertTimeToBCDEx(stTime,&buffer[8]);
  EepromPram_WritePram(usParamID,buffer,14);
  return 10;
  
}
/*************************************************************
** 函数名称: E2ParamApp_ReadTimeParam
** 功能描述: 把读的时间参数转成索美时间格式
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned char E2ParamApp_ReadTimeParam(unsigned short usParamID,unsigned char *data)
{
  unsigned char buffer[20]={0};
  unsigned long temp;
  unsigned char len;
  TIME_T stTime;
  len=EepromPram_ReadPram(usParamID,buffer);
  if(len != 14)return 0;
  memcpy(data,buffer,2);
  if(!Public_ConvertBCDToTime(&stTime,&buffer[2]))return 0;
  temp = E2ParamApp_ConverseGmtime(&stTime);
  Public_ConvertLongToBuffer(temp,&data[2]);
  /////////////////////////////////
  if(!Public_ConvertBCDToTime(&stTime,&buffer[8]))return 0;
  temp = E2ParamApp_ConverseGmtime(&stTime);
  Public_ConvertLongToBuffer(temp,&data[6]);
  return 10;
}
/*************************************************************
** 函数名称: E2ParamApp_WriteSpecialParam
** 功能描述: 写特殊参数
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned char E2ParamApp_WriteSpecialParam(unsigned short usParamID,unsigned char *data,unsigned char datalen)
{
#ifdef NEED_CONVERT_SUOMEI_PARAM_ID
  unsigned long temp,val;
  unsigned char buffer[20]={0};    
  if(0xC115== usParamID)////超速预警提示值
  {
    if(datalen != 4)return 0;
    temp=Public_ConvertBufferToLong(data);
    if(temp)
    {
      if(EepromPram_ReadPram(E2_MAX_SPEED_ID,buffer))
      {
        val = Public_ConvertBufferToLong(buffer);
        temp =val-temp;
      }
    }
    ///////////////////
    temp *= 10;
    Public_ConvertShortToBuffer(temp,buffer);
    EepromPram_WritePram(E2_SPEED_EARLY_ALARM_DVALUE_ID,buffer,E2_SPEED_EARLY_ALARM_DVALUE_LEN);
    return 4;
  }
  else
    if(0xC21B == usParamID)//停车休息(禁止行驶)夜间时间定义
    {
      return E2ParamApp_WriteTimeParam(E2_FORBID_DRIVE_NIGHT_TIME_ID,data,datalen);
    }
    else
      if(0xC222 == usParamID)//超速夜间时间定义
      {
        return E2ParamApp_WriteTimeParam(E2_OVERSPEED_NIGHT_TIME_ID,data,datalen);
      }
      else
        if(0xC224 == usParamID)//超时驾驶预警提示时间提前值
        {
          if(datalen != 4)return 0;
          temp = Public_ConvertBufferToLong(data);
          temp *= 60;
          Public_ConvertShortToBuffer(temp,buffer);
          EepromPram_WritePram(E2_TIRE_EARLY_ALARM_DVALUE_ID,buffer,E2_TIRE_EARLY_ALARM_DVALUE_LEN);
          return 4;
        }
        else
          if(0xC250== usParamID)////停车判定速度(公里/小时),同时更新超时驾驶的判定速度
          {
            EepromPram_WritePram(E2_TIRED_DRIVE_RUN_SPEED_ID,data,E2_TIRED_DRIVE_RUN_SPEED_ID_LEN);
          }
          else
            if(0xC25B == usParamID)//事件项，保留，用于指定产生事件后上报时间间隔的事件项 BIT0位＝1：超速报警 
            {
              if(datalen != 4)return 0;
              temp = Public_ConvertBufferToLong(data);
              val = 0;
              if(EepromPram_ReadPram(E2_REPORT_FREQ_EVENT_SWITCH_ID,buffer))
              {
                val = Public_ConvertBufferToLong(buffer);
              }
              ///////索美只定义了超速回传/////////////////
              if(PUBLIC_CHECKBIT(temp, 0))//
              {
                PUBLIC_SETBIT(val, 0);
              }
              else
              {
                PUBLIC_CLRBIT(val, 0);
              }
              //////////////////////
              Public_ConvertLongToBuffer(val,buffer);
              EepromPram_WritePram(E2_REPORT_FREQ_EVENT_SWITCH_ID,buffer,E2_REPORT_FREQ_EVENT_SWITCH_ID_LEN);
            }    
            else
              if(0xC255 == usParamID)//连续驾驶夜间时间定义
              {
                return E2ParamApp_WriteTimeParam(E2_TIRED_DRIVE_NIGHT_TIME_ID,data,datalen);
              }
#endif
  return 0;
}
/*************************************************************
** 函数名称: E2ParamApp_ReadSpecialParam
** 功能描述: 读特殊参数
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned char E2ParamApp_ReadSpecialParam(unsigned short usParamID,unsigned char *data)
{
  unsigned long temp=0,val;
  unsigned char buffer[20]={0}; 
  if(E2_CAR_TOTAL_MILE_ID == usParamID)//车辆里程表读数//0.1km
  {
    Public_ReadDataFromFRAM(FRAM_MILEAGE_ADDR,buffer,FRAM_MILEAGE_LEN);//单位0.01公里
    val  = Public_ConvertBufferToLong(buffer);
    val *= 0.1;
    Public_ConvertLongToBuffer(val,data);
    return 4;
  }
#ifdef NEED_CONVERT_SUOMEI_PARAM_ID 
  else
    if(0xC115== usParamID)////超速预警提示值
    {
      if(EepromPram_ReadPram(E2_MAX_SPEED_ID,buffer))
      {
        val = Public_ConvertBufferToLong(buffer);
      }
      //////////////////
      if(EepromPram_ReadPram(E2_SPEED_EARLY_ALARM_DVALUE_ID,buffer))
      {
        temp = Public_ConvertBufferToShort(buffer);
        temp *=0.1;
        if(temp&&val>temp)
        {
          temp=val-temp;
        }
        else
        {
          temp = 0;
        }
      }        
      Public_ConvertLongToBuffer(temp,data);
      /////////////////
      return 4;
    }
    else
      if(0xC21B == usParamID)//停车休息(禁止行驶)夜间时间定义
      {
        return E2ParamApp_ReadTimeParam(E2_FORBID_DRIVE_NIGHT_TIME_ID,data);
      }
      else
        if(0xC222 == usParamID)//超速夜间时间定义
        {
          return E2ParamApp_ReadTimeParam(E2_OVERSPEED_NIGHT_TIME_ID,data);
        }
        else
          if(0xC224 == usParamID)//超时驾驶预警提示时间提前值
          {
            if(EepromPram_ReadPram(E2_TIRE_EARLY_ALARM_DVALUE_ID,buffer))
            {
              temp = Public_ConvertBufferToShort(buffer);
              temp /=60;
            }
            ////////////
            Public_ConvertLongToBuffer(temp,data);
            return 4;
          }
          else
            if(0xC250== usParamID)////停车判定速度(公里/小时),同时更新超时驾驶的判定速度
            {
              //
            }
            else
              if(0xC25B == usParamID)//事件项，保留，用于指定产生事件后上报时间间隔的事件项 BIT0位＝1：超速报警 
              {
                val = 0;
                if(EepromPram_ReadPram(E2_REPORT_FREQ_EVENT_SWITCH_ID,buffer))
                {
                  temp = Public_ConvertBufferToLong(buffer);
                }
                ///////索美只定义了超速回传/////////////////
                if(PUBLIC_CHECKBIT(temp, 0))//
                {
                  PUBLIC_SETBIT(val, 0);
                }
                else
                {
                  PUBLIC_CLRBIT(val, 0);
                }
                //////////////////////
                Public_ConvertLongToBuffer(val,data);
                return 4;
              }    
              else
                if(0xC255 == usParamID)//连续驾驶夜间时间定义
                {
                  return E2ParamApp_ReadTimeParam(E2_TIRED_DRIVE_NIGHT_TIME_ID,data);
                }
#endif
  return 0;
}
/*************************************************************
** 函数名称: E2ParamApp_ConvertPramID
** 功能描述: 根据要求转成我们新自定义的ID
** 入口参数: 参数ID
** 出口参数: 
** 返回参数: 我们新自定义的ID
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned short E2ParamApp_ConvertPramID(unsigned short usParamID)
{
  unsigned short paramID;
  paramID=usParamID;
#ifdef NEED_CONVERT_OLD_SELF_DEFINE_ID
  paramID=E2ParamApp_ConvertOldSelfDefinePramID(paramID);
#endif
#ifdef NEED_CONVERT_SUOMEI_PARAM_ID
  paramID=E2ParamApp_ConvertSuomeiPramID(paramID);
#endif
  return paramID;
}
/*************************************************************
** 函数名称: E2ParamApp_ConvertTranPramID
** 功能描述: 根据要求把参数ID转成上传识别的参数ID
** 入口参数: 参数ID
** 出口参数: 
** 返回参数: 上传识别的参数ID
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned short E2ParamApp_ConvertTranPramID(unsigned short usParamID)
{
  unsigned short paramID;
  paramID=usParamID;
  return paramID;
}
/*************************************************************
** 函数名称: E2ParamApp_ConvertPramIDToGroupIndex
** 功能描述: 把参数ID转成相关数组下标
** 入口参数: 参数ID
** 出口参数: 
** 返回参数: 数组下标
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned short E2ParamApp_ConvertPramIDToGroupIndex(unsigned short PramID)
{
  unsigned short ID;
  ID = 0;
  if(PramID < E2_SYSTEM_DEFINE_ID_MAX)
  {
    ID = PramID & 0xFFF;//取低字节
  }
  else 
    if(PramID >= E2_PRAM_BASE_CUSTOM_ID)
    {
      ID = (PramID-E2_PRAM_BASE_CUSTOM_ID+E2_SYSTEM_DEFINE_ID_MAX) & 0xFFF;
    }
    else//
    {
      ID = 0;
    }
  //////////////////
  return ID;
}
/*******************************************************************************
** 函数名称: E2ParamApp_CheckIPFormatData
** 函数功能: 判断数据是否为IP或域名格式
** 入口参数: data数据首地址,datalen数据长度
** 返回参数: 0是IP或域名格式,1不是IP或域名格式
** 全局变量: 无
** 调用模块: 无
*******************************************************************************/
unsigned char E2ParamApp_CheckIPFormatData(unsigned char *data,unsigned char datalen)
{
  unsigned char i;
  for(i=0; i<datalen; i++)
  {
    if(((data[i]>='0')&&(data[i]<='9'))||(data[i] == '.'))
    {
      
    }
    else
      if(((data[i]>='a')&&(data[i]<='z'))||((data[i]>='A')&&(data[i]<='Z')))
      {
        
      }
      else if(data[i] ==0)
      {
        if(i != (datalen-1))
        {
          return 1;
        }
      }
      else
      {
        return 1;
      }
  }
  return 0;
}
/*******************************************************************************
** 函数名称: E2ParamApp_CheckPhoneFormatData
** 函数功能: 判断数据是否为手机号格式,BCD[6]
** 入口参数: data数据首地址,datalen数据长度
** 返回参数: 0是手机号格式,1不是手机号格式
** 全局变量: 无
** 调用模块: 无,dxl,2014.4.29增加
*******************************************************************************/
unsigned char E2ParamApp_CheckPhoneFormatData(unsigned char *data,unsigned char datalen)
{
  u8 *p = NULL;
  u8  High4Bit = 0;//高4位
  u8  Low4Bit = 0;//低4位
  u8  i;
  if(NULL == p)
  {
    if(6 != datalen)
    {
      return 1;//长度有误
    }
    if((0 == *data)&&(30 == *(data+1)))//dxl,2014.6.11增加,广州设置手机号时不校验
    {
      return 0;//广州手机号设置不满足bcd格式,前两个字节是固定的0H，1eH
    }
    p = data;
    for(i=0; i<datalen; i++)
    {
      High4Bit = (*p&0xf0) >> 4;
      Low4Bit = *p&0x0f;
      if((High4Bit > 9)||(Low4Bit > 9))
      {
        return 1;//格式有误
      }
      p++;
    }
    return 0;
    
  }
  else
  {
    return 1;//返回错误
  }
}
ProtocolACK E2ParamApp_ResumeFactory(void)
{
  BEEP_OFF();
  //擦除盲区记录
  Blind_Erase(0xff);
  //恢复为第1次使用时自动配置的参数
  EepromPram_DefaultSet();
  //删除所有驾驶员信息,dxl,2014.3.15从这之后是新增加的
  DriverInfoDeleteAll();
  //删除所有电话簿
  PhoneBookDeleteAll();
  //删除所有事件报告
  EventReportDeleteAll();
  //删除所有信息点播
  InfoDemandDeleteAll();
  //删除所有点播的信息
  InfoServiceDeleteAll();
  //删除所有提问应答信息
  QuestionResponseDeleteAll();
  //删除所有文本信息
  EraseDownloadGB2313FlashArea();
  //删除所有通话记录内容
  PhoneBook_CallRecordDeleteAll();
  //清除矩形、圆形区域
  Area_EraseAllArea();
  //清除多边形区域
  Polygon_ClearArea();
  //清空线路
  ClrRoute(); 
  //清空当前暂存在铁电中所                  
  TiredDrive_TiredDriverInfoInit();
  //擦除记录仪数据（含超时驾驶记录）
  Register_EraseRecorderData();
  //恢复状态位
  Io_ClearStatusWord();
  Io_ClearExtCarStatusWord();
  //恢复报警位
  Io_ClearAlarmWord();
  //恢复里程计数
  GpsMile_SetPram(0);
  
  return ACK_OK;
}
/*************************************************************
** 函数名称: E2ParamApp_DisposeWriteParam
** 功能描述: 处理协议写参数
** 入口参数: pBuffer数据首地址,BufferLen数据长度
** 出口参数: 
** 返回参数: 处理结果
** 全局变量: 
** 调用模块: 
*************************************************************/	
ProtocolACK E2ParamApp_DisposeWriteParam(u8 channel,u8 *pBuffer, u16 BufferLen)
{
  u8	*p;
  u8	Buffer[30];
  u8	ReadLen;
  u8	PramLen;
  s16	length;
  u32	PramID,prePramID;
  u16	ID;
  u8	flag;
  u8  ch;
  
  p = pBuffer;
  length = BufferLen;
  
  //先只考虑单值的情况
  while(length > 0)
  {
    //读参数ID
    prePramID=Public_ConvertBufferToLong(p);
    p += 4;
    ////////////////////////
    PramID = E2ParamApp_ConvertPramID(prePramID);
    /////////////////////////
    ID = E2ParamApp_ConvertPramIDToGroupIndex(PramID);
    ///////////////
    flag = 0;
    //读取参数长度
    PramLen = *p++;
    /////////////////////////
    E2ParamApp_WriteSpecialParam(prePramID,p,PramLen);
    ////////////////////////////
    if(E2_ALL_DRIVER_INFORMATION_ID == PramID)//EGS702使用,写所有驾驶员信息,最多只能写2个驾驶员
    {
      DriverInfo_DisposeRadioProtocol(p,PramLen);//保存在FLASH中 
      flag = 1;//EGS702使用时才需要写E2
    }
    else
      if(E2_HB_IC_DRIVER_ID == PramID)//EGS702使用,写驾驶员信息到IC卡中
      {
        flag = 1;//不需要写到E2里
      }
    //////////////
    if(ID&&ID< E2_PRAM_ID_INDEX_MAX)//处理ID已识别的,其它跳过
    {
      //检查参数ID和参数长度是否正确
      if(EepromPramLength[ID] >= PramLen)
      {
        //检查一些特定的参数是否正确
        //if((E2_MAIN_SERVER_IP_ID == PramID)||(E2_BACKUP_SERVER_IP_ID == PramID))从V1.01.09版本开始不检查ip或域名的合法性了,dxl,2014.7.15
        //{
        //检查主、备IP参数是否正确
        //flag = E2ParamApp_CheckIPFormatData(p,PramLen);
        //}
        if(E2_DEVICE_PHONE_ID == PramID)//dxl,2014.4.29,增加对手机号参数的格式检查
        {
          flag = E2ParamApp_CheckPhoneFormatData(p,PramLen);
        }
        if(E2_CAR_ONLY_NUM_ID == PramID)/*唯一性编号，强制写入CCC认证编号、产品型号*/
        {
          memcpy(p,Recorder_CCC_ID,7);
          //memcpy(p+7,Recorder_Product_VER,16);    产品型号由初始化软件设置，终端不再固化,dxl,2013.9.27                              
        }
        //写参数
        if(0 == flag)
        {
          ////只有写了才进行下面操作/////////////////////      
          EepromPram_WritePram(PramID, p, PramLen);
          ////////////////////////
          if(E2_CAR_TOTAL_MILE_ID == PramID)//设置车辆里程
          {
            //行车记录仪参数修改记录
            Recorder_ParaSet(0xc4);
          }
          if( 
             (E2_CAR_IDENTIFICATION_CODE_ID == PramID)||//车辆识别码
               (E2_CAR_TYPE_ID == PramID)||//车牌分类
                 (E2_CAR_PLATE_NUM_ID == PramID)//车牌号码
                   )
          {
            //行车记录仪参数修改记录
            Recorder_ParaSet(0x82);
          }
          if(E2_CAR_FEATURE_COEF_ID == PramID)//设置车辆脉冲
          {
            //行车记录仪参数修改记录
            Recorder_ParaSet(0xc3);
          }
          if(
             (E2_MAIN_SERVER_IP_ID == PramID)||                  
               (E2_MAIN_SERVER_UDP_PORT_ID == PramID)||
                 (E2_MAIN_SERVER_TCP_PORT_ID == PramID)||
                   (E2_BACKUP_SERVER_IP_ID == PramID)
                     )
          {
            //擦除通道1鉴权码,下次上线重新发注册指令
            ClearTerminalAuthorizationCode(CHANNEL_DATA_1);
          }
          
          if(
             (E2_SECOND_MAIN_SERVER_IP_ID == PramID)||
               (E2_SECOND_MAIN_SERVER_TCP_PORT_ID == PramID)||
                 (E2_SECOND_MAIN_SERVER_UDP_PORT_ID == PramID)
                   )
          {
            //擦除通道1鉴权码,下次上线重新发注册指令
            ClearTerminalAuthorizationCode(CHANNEL_DATA_2);
          }
          
          if( 
             (E2_CAR_PROVINCE_ID == PramID)||//省域ID
               (E2_CAR_CITY_ID == PramID)||//市域ID
                 (E2_DEVICE_ID == PramID)||//制造商ID
                   (E2_CAR_PLATE_NUM_ID == PramID)||//车牌号码
                     (E2_CAR_PLATE_COLOR_ID == PramID)//车牌颜色
                       )
          {
            //擦除两个通道鉴权码,下次上线重新发注册指令
            ClearTerminalAuthorizationCode(CHANNEL_DATA_1);
            ClearTerminalAuthorizationCode(CHANNEL_DATA_2);
          }
          
          if(E2_DEVICE_PHONE_ID == PramID)//写入备份区域,终端手机号
          {
            EepromPram_WritePram(E2_DEVICE_PHONE_BACKUP_ID, p, PramLen);
          }
          else if(E2_DEVICE_ID == PramID)//写入备份区域,终端ID
          {
            EepromPram_WritePram(E2_DEVICE_BACKUP_ID, p, PramLen);
          }
          else if(E2_CDMA_SLEEP_ID == PramID)//设置CDMA休眠开关，1为开，非1为关，默认为关
          {
            DelayAckCommand = 4;
            //触发延时任务
            SetTimerTask(TIME_DELAY_TRIG, 2);//2秒后重新启动模块
          }
          else if(E2_PASSWORD_ID == PramID)//设置维护密码
          {
#ifdef USE_NAVILCD//如果定义了导航屏
            if(NaviLcd_GetCurOnlineStatus())//在线
            {
              NaviLcdSendCmdModifyPassWord(p);
            }
#endif
          }
          else if(E2_RESUME_NOUSED_ID == PramID)//恢复为重来没有使用过的状态
          {
            if(*p)//执行
            {
              E2ParamApp_ResumeFactory();
            }
            //重新启动
            DelayAckCommand = 5;
            //触发延时任务
            SetTimerTask(TIME_DELAY_TRIG, SECOND);//1秒后重新启动
          }
          else if(E2_TIME_PHOTO_CONTROL_ID == PramID)//写参数定时拍照通道
          {
            ch = channel;
            FRAM_BufferWrite(FRAM_PARAMETER_TIME_PHOTO_CHANNEL_ADDR,&ch,FRAM_PARAMETER_TIME_PHOTO_CHANNEL_LEN);
          }
          else if(E2_MILE_PHOTO_CONTROL_ID == PramID)//写参数定时拍照通道
          {
            ch = channel;
            FRAM_BufferWrite(FRAM_PARAMETER_DISTANCE_PHOTO_CHANNEL_ADDR,&ch,FRAM_PARAMETER_DISTANCE_PHOTO_CHANNEL_LEN);
          }
          //读出刚写入的参数
          
#ifdef HUOYUN_DEBUG_OPEN//全国货运平台联调时不能远程设置TCP端口号
          if(E2_MAIN_SERVER_TCP_PORT_ID == PramID)
          {
            
          }
          else
          {
            if(PramLen < 30)
            { 
              ReadLen = EepromPram_ReadPram(PramID, Buffer);
              if(Public_CheckArrayValIsEqual(Buffer,p,ReadLen))//写出错
              {
                return ACK_ERROR;//写出错
              }
            }
            //更新相应的变量
            EepromPram_UpdateVariable(PramID);
          }
#else
          
          if(PramLen < 30)
          { 
            ReadLen = EepromPram_ReadPram(PramID, Buffer);
            if(Public_CheckArrayValIsEqual(Buffer,p,ReadLen))//写出错
            {
              return ACK_ERROR;//写出错
            }
          }
          //更新相应的变量
          EepromPram_UpdateVariable(PramID);
#endif
          
        }
      }
      else//跳过长度不正确的
      {
        //return 0;
      }
    }
    else//跳过ID不认识的
    {
      //return 0;
    }
    ////////////////////
    length = length - 4 - 1 - PramLen;
    p += PramLen;
  }
  return ACK_OK;
}
/*************************************************************
** 函数名称: E2ParamApp_ReadOneParam
** 功能描述: 处理协议读参数
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 
** 调用模块: 
*************************************************************/	
unsigned char E2ParamApp_ReadOneParam(unsigned long ReadPramID,unsigned long PramID,unsigned char *data)
{
  u8 PramLen;
  u8 Buffer[200];
  PramLen = E2ParamApp_ReadSpecialParam(PramID,Buffer);
  if(PramLen)
  {
    //写参数ID
    Public_ConvertLongToBuffer(PramID,data);
    //写参数长度
    data[4] = PramLen;
    //写参数内容
    memcpy(&data[5],Buffer,PramLen);
    return PramLen+5;
  }
  //////////////////////////
  PramLen = EepromPram_ReadPram(ReadPramID, Buffer);
  if((PramLen != 0)&&(PramLen < 50))
  {
    //写参数ID
    Public_ConvertLongToBuffer(PramID,data);
    //写参数长度
    data[4] = PramLen;
    //写参数内容
    memcpy(&data[5],Buffer,PramLen);
    return PramLen+5;
  }
  return 0;
}
/*************************************************************
** 函数名称: E2ParamApp_DisposeReadAllParam
** 功能描述: 处理协议读参数
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 
** 调用模块: 
** 备    注: 读取的参数总个数，*pDstBuffer按ID号，ID长度，ID内容的顺序存放，*DstBufferLen总的读取长度，不包括参数总个数字节
: ReadType:0为有线(串口)，每次只能读550字节；1为无线（GPRS），每次可读1000字节
*************************************************************/	
unsigned char E2ParamApp_DisposeReadAllParam(unsigned char *pDstBuffer,s16 *DstBufferLen,unsigned char ReadType)
{
  unsigned char *des;
  unsigned char paramNum,len;
  signed short length;
  unsigned short maxlen;
  unsigned long PramID,readParamID;  
  length =0;
  paramNum =0;    
  des = pDstBuffer;
  if(0==ReadType)
    maxlen = 550;
  else
    maxlen = 1000;
  //**************平台指定的参数*****************
  for(PramID=1; PramID<E2_SYSTEM_DEFINE_ID_MAX; PramID++)
  {
    len = E2ParamApp_ReadOneParam(PramID,PramID,des);
    if(len)
    {
      if(length+len>maxlen)break;
      des += len;
      length+=len;
      paramNum++;
    }
  }	
  //******************自定义的参数******************
  for(readParamID=E2_PRAM_BASE_CUSTOM_ID; readParamID<E2_SELF_DEFINE_ID_MAX; readParamID++)
  {
    if((readParamID != E2_ALL_DRIVER_INFORMATION_ID)&&(readParamID != E2_UPDATA_URL_ID)&&(readParamID != E2_HB_IC_DRIVER_ID))
    {
      PramID=E2ParamApp_ConvertTranPramID(readParamID);
      //读参数
      len=E2ParamApp_ReadOneParam(readParamID,PramID,des);
      if(len)
      {
        if(length+len>maxlen)break;
        des += len;
        length+=len;
        paramNum++;
      }
    }
  }    
  ////////////////////
  *DstBufferLen=length;
  ////////////////
  return paramNum;
}
/*************************************************************
** 函数名称: E2ParamApp_DisposeReadParam
** 功能描述: 处理协议读参数
** 入口参数: 把需要读的参数ID号放入缓冲pIDBuffer中，每个ID四字节，高在前，IDBufferLen为4的整数倍
** 出口参数: 
** 返回参数: 
** 全局变量: 
** 调用模块: 
** 备    注: 读取的参数总个数，*pDstBuffer按ID号，ID长度，ID内容的顺序存放，*DstBufferLen总的读取长度，不包括参数总个数字节
: ReadType:0为有线(串口)，每次只能读550字节；1为无线（GPRS），每次可读1000字节
*************************************************************/	
unsigned char E2ParamApp_DisposeReadParam(unsigned char *pDstBuffer,s16 *DstBufferLen,unsigned char *pIDBuffer,signed short IDBufferLen,unsigned char ReadType)
{
  unsigned char *des,*src;
  unsigned char paramNum,len;
  signed short length,datalen;
  unsigned short maxlen;
  unsigned long PramID,readParamID;    
  paramNum =0;
  length =0;
  des = pDstBuffer;    
  src = pIDBuffer;//    
  datalen =IDBufferLen;
  if(0==ReadType)
    maxlen = 550;
  else
    maxlen = 1000;
  //////////////////
  while(datalen > 0)
  {
    PramID=Public_ConvertBufferToLong(src);        
    readParamID = E2ParamApp_ConvertPramID(PramID);
    //读参数
    if(readParamID == E2_ALL_DRIVER_INFORMATION_ID)//EGS702专用//驾驶员信息
    {
      len=DriverInfo_ReadDataForRadioProtocol(des+5);//读FLASH中所有驾驶员信息
      ////////////////////
      Public_ConvertLongToBuffer(PramID,des);
      des[4] = len;
      ///////////////////
      if(length+len+5>maxlen)break;
      des += len+5;
      length+=len+5;
      paramNum++;
    }
    else
      if((readParamID != E2_UPDATA_URL_ID)&&(readParamID != E2_HB_IC_DRIVER_ID))
      {
        len = E2ParamApp_ReadOneParam(readParamID,PramID,des);
        if(len)
        {
          if(length+len>maxlen)break;
          des += len;
          length+=len;
          paramNum++;
        }
      }
    src += 4;
    datalen -=4;
  }
  ////////////////////
  *DstBufferLen=length;
  ////////////////
  return paramNum;
}
#endif
/*******************************************************************************
*                             end of module
*******************************************************************************/


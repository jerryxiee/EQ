/********************************************************************
//版权说明	:Shenzhen E-EYE Co. Ltd., Copyright 2009- All Rights Reserved.
//文件名称	:JTReport.c		
//功能		:实现九通渣土车上报需求
//版本号	:
//开发人	:dxl
//开发时间	:2013.11.28 
//修改者	:
//修改时间	:
//修改简要说明	:
//备注		:在设定的区域内有举升和厢盖开启，认为是正常的状态，
//                如果是在设定的区域外有举升和厢盖开启动作则认为是异常的状态，
//                产生异常后需5分钟汇报1次位置信息，每次需带九通附加ID0xe4
//                1次倾倒需要上报一条带九通附加ID0xe3的位置信息
***********************************************************************/
//***************包含文件*****************
#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "include.h"
//***************本地变量******************
static u32 JTIoStatus = 0;//九通状态位，
//****************全局变量*****************

//***************函数定义********************
/*********************************************************************
//函数名称	:JTReport_TimeTask
//功能		:九通位置汇报（九通渣土车应用）
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
FunctionalState JTReport_TimeTask(void)
{
      u8  Status1 = 0;//厢盖状态
      u8  Status2 = 0;//举升状态
     // u8  Status1Error = 0;//厢盖传感器故障报警
     // u8  Status2Error = 0;//举升传感器故障报警
      u8  Buffer[FLASH_BLIND_STEP_LEN];
      u8  BufferLen = 0;
      u8  InOutAreaFlag = 0;//在或不在区域标志,1为在倾倒点区域
      static TIME_T StartTime;//倾倒开始时间
      static TIME_T EndTime;//倾倒结束时间
      static u8 InOutAreaFlag1 = 0;//倾倒开始是否在区域内的标志
      static u8  InOutAreaFlag2 = 0;//倾倒结束是否在区域内的标志
      static u8  LastStatus1 = 0;//上一次厢盖状态
      static u8  LastStatus2 = 0;//上一次举升状态
      static u8  SendFlag1 = 0;//发送标志
      static u8  SendFlag2 = 0;//发送标志
      static u8  Status3 = 0;//倾倒状态
      static u16 AlarmCount = 0;//厢盖或举升异常报警计数
      u8 channel = CHANNEL_DATA_1;//为编译通过而添加
       
      //读厢盖状态
      Status1 = Io_ReadSelfDefine2Bit(DEFINE_BIT_9);
      //Status1Error = Io_ReadSelfDefine2Bit(DEFINE_BIT_29);
      //读举升状态
      Status2 = Io_ReadSelfDefine2Bit(DEFINE_BIT_10);
      //Status2Error = Io_ReadSelfDefine2Bit(DEFINE_BIT_30);
      //读是否在倾倒点
      InOutAreaFlag = MuckDump_GetCurMuckDumpAreaID();
      //倾倒开始时刻点,倾倒结束时刻点
      //if((1 == Status1)&&(0 == Status1Error))//厢盖处于开启且厢盖检测传感器无故障的状态下，车厢由正常水平位置到举升，认为是倾倒开始点，由举升到回到正常水平位置为倾倒结束点
     // {
             if((0 == LastStatus2)&&(1 == Status2))//倾倒开始
             {
                      //读当前时间
                      RTC_GetCurTime(&StartTime);
                      //读是否在区域内的状态
                      InOutAreaFlag1 = InOutAreaFlag;
             }
             else if((1 == LastStatus2)&&(0 == Status2))//倾倒结束
             {
                      
                      //读当前时间
                      RTC_GetCurTime(&EndTime);
                      //读是否在区域内的状态
                      InOutAreaFlag2 = InOutAreaFlag;
                      
                      if((InOutAreaFlag1)&&(InOutAreaFlag2))//开始和结束时刻都在区域内才认为是正常倾倒，否则为异常倾倒
                      {
                              Status3 = 0;
                      }
                      else
                      {
                              Status3 = 1;
                      }
                      
                      SendFlag1 = 1;
             }
     // }
      if(1 == SendFlag1)
      {
              //读取位置信息
              BufferLen = Report_GetPositionInfo(Buffer);
             //增加九通的位置附加ID E3
              Buffer[BufferLen++] = 0xE3;//附加ID
              Buffer[BufferLen++] = 13;//附加ID长度
              //附加ID内容----倾倒状态
              Buffer[BufferLen++] = Status3;
              //附加ID内容----倾倒开始时间
              Buffer[BufferLen++] = Public_HEX2BCD(StartTime.year);
              Buffer[BufferLen++] = Public_HEX2BCD(StartTime.month);
              Buffer[BufferLen++] = Public_HEX2BCD(StartTime.day);
              Buffer[BufferLen++] = Public_HEX2BCD(StartTime.hour);
              Buffer[BufferLen++] = Public_HEX2BCD(StartTime.min);
              Buffer[BufferLen++] = Public_HEX2BCD(StartTime.sec);
              //附加ID内容----倾倒结束时间
              Buffer[BufferLen++] = Public_HEX2BCD(EndTime.year);
              Buffer[BufferLen++] = Public_HEX2BCD(EndTime.month);
              Buffer[BufferLen++] = Public_HEX2BCD(EndTime.day);
              Buffer[BufferLen++] = Public_HEX2BCD(EndTime.hour);
              Buffer[BufferLen++] = Public_HEX2BCD(EndTime.min);
              Buffer[BufferLen++] = Public_HEX2BCD(EndTime.sec);
              //发送位置信息到平台
              if(ACK_OK == RadioProtocol_PostionInformationReport(channel,Buffer, BufferLen))
	      {
		      SendFlag1 = 0;
	      }
     }
  
      //倾倒状态，倾倒开始时刻点和倾倒结束时刻点都在区域内认为是正常的倾倒，否则认为是异常倾倒
      //厢盖状态或举升状态不在指定区域内打开时认为是异常，异常情况下5分钟汇报1次
     // if((1 == Status1)&&(0 == Status1Error)&&(0 == InOutAreaFlag))
     if((1 == Status1)&&(0 == InOutAreaFlag))
      {
              AlarmCount++;
              if(1 == AlarmCount)
              {
                      SendFlag2 = 1;
              }
              else if(AlarmCount >= 300)
              {
                    AlarmCount = 0;
              }
              JTReport_WriteIoStatusBit(0, SET);
      }
     // else if((1 == Status2)&&(0 == Status2Error)&&(0 == InOutAreaFlag))
      else if((1 == Status2)&&(0 == InOutAreaFlag))
      {
              AlarmCount++;
              if(1 == AlarmCount)
              {
                      SendFlag2 = 1;
              }
              else if(AlarmCount >= 300)
              {
                    AlarmCount = 0;
              }
              JTReport_WriteIoStatusBit(1, SET);
      }
     // else if((0 == Status1)&&(1 == LastStatus1)&&(0 == Status1Error))//厢盖由开到关，加入这个判断是为了防止两个异常同时产生时，逐个消除时能快速看到
      else if((0 == Status1)&&(1 == LastStatus1))
      {
              AlarmCount = 0;
              JTReport_WriteIoStatusBit(0, RESET);
      }
     // else if((0 == Status2)&&(1 == LastStatus2)&&(0 == Status2Error))//举升由开到关
      else if((0 == Status2)&&(1 == LastStatus2))//举升由开到关
      {
              AlarmCount = 0;
              JTReport_WriteIoStatusBit(1, RESET);
      }
      else
      {
              AlarmCount = 0;
              JTReport_WriteIoStatusBit(0, RESET);
              JTReport_WriteIoStatusBit(1, RESET);
      }
  
      if(1 == SendFlag2)
      {
             //读取位置信息
              BufferLen = Report_GetPositionInfo(Buffer);
             //增加九通的位置附加ID E4
              Buffer[BufferLen++] = 0xE4;//附加ID
              Buffer[BufferLen++] = 4;//附加ID长度
              //附加ID内容----倾倒开始时间
              Buffer[BufferLen++] = (JTIoStatus&0xff000000)>>24;
              Buffer[BufferLen++] = (JTIoStatus&0xff0000)>>16;
              Buffer[BufferLen++] = (JTIoStatus&0xff00)>>8;
              Buffer[BufferLen++] = JTIoStatus&0xff;

              //发送位置信息到平台
              if(ACK_OK == RadioProtocol_PostionInformationReport(channel,Buffer, BufferLen))
	      {
		      SendFlag2 = 0;
	      }
      }
      
      LastStatus1 = Status1;
      LastStatus2 = Status2;
      
  
      return ENABLE;
}
/*********************************************************************
//函数名称	:JTReport_WriteIoStatusBit
//功能		:写九通自定义的状态位，
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:附加ID E4会用到
*********************************************************************/
void JTReport_WriteIoStatusBit(u8 bit, FlagStatus state)
{
	if(bit < 32)
	{
		if(SET==state)
		{
			JTIoStatus |= 1<<bit;
		}
		else if(RESET==state)
		{
			JTIoStatus &= ~(1<<bit);
		}
	
	}
}
/********************************************************************
//版权说明	:Shenzhen E-EYE Co. Ltd., Copyright 2009- All Rights Reserved.
//文件名称	:Pulse_App.c		
//功能		:脉冲速度检测功能
//版本号	:
//开发人	:dxl 
//开发时间	:2012.6
//修改者	:
//修改时间	:
//修改简要说明	:
//备注		:
***********************************************************************/
/*************************修改记录*************************/
/*************************文件包含***********************/
#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "include.h"
/*************************宏定义***********************/
#define	TIME_COUNT_ARRAY_SIZE	5
/*************************外部变量***********************/
extern u8	SpeedFlag;//速度类型,dxl,2015.5.11,0x00:手动脉冲,0x01:手动GPS,0x02:自动脉冲,0x03:自动GPS
extern vu32 	myPulseCnt;             //校准计数使用
extern u8	TestStatus;//测试状态;E1H,进入里程误差测量;E2H,进入脉冲系数误差测量;E3H,进入实时时钟误差测量
/*************************全局变量***********************/
u32	PulseTotalMile = 0;//累计行驶里程，即总里程,单位是0.01公里
u32	FeatureCoef = 0;//车辆特征系数
/*************************本地变量***********************/
static u16	TimeCountMin = 0;//定时器计数允许最小值，小于该值认为是干扰
static u16	TimeCountArray[TIME_COUNT_ARRAY_SIZE+1] = {0};

#ifdef SPEED_PINGJUN
static u8	TimeCountArrayCount = 0;
#endif

u32 	ExtiCount = 0;//外部中断进入次数计数
u32	PulseMileCount = 0;//脉冲个数（里程）计数
u8	PulseSpeed = 0;//脉冲瞬时速度，km/h
u8	LastPulseSpeed = 0;//上一次脉冲速度
u8	PulseMinuteSpeed = 0;//脉冲每分钟平均速度
/*********************************************************************
//函数名称	:Pulse_UpdataPram
//功能		:脉冲模块更新变量
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
//		:
*********************************************************************/
void Pulse_UpdataPram(void)
{
	u8	TmpData[4] = {0};
	u8	PramLen = 0;
	u32	temp;
	//读特征系数
	PramLen = EepromPram_ReadPram(E2_CAR_FEATURE_COEF_ID, TmpData);
	if(3 != PramLen)
	{
			FeatureCoef = 7200;//默认为3600，取消了速度传感器系数,dxl,2015.4.17后改为7200
	}
	else
	{
			FeatureCoef = 0;
			FeatureCoef |= (TmpData[0]<<16);//先取高位
			FeatureCoef |= (TmpData[1] << 8);//中字节
			FeatureCoef |= TmpData[2];//低字节位
	}
	//定时器计数最小允许值
	temp = (3600*(u32)1000000)/(50*FeatureCoef*255);
	TimeCountMin = temp&0xffff;
	//读累计行驶里程
	PramLen = FRAM_BufferRead(TmpData, FRAM_MILEAGE_LEN, FRAM_MILEAGE_ADDR);
	//检查校验字节是否正确
	if(FRAM_MILEAGE_LEN == PramLen)//校验和正确
	{
		PulseTotalMile = 0;
		PulseTotalMile |= TmpData[0] << 24;	//高字节
		PulseTotalMile |= TmpData[1] << 16;	//次高字节
		PulseTotalMile |= TmpData[2] << 8;		//中字节
		PulseTotalMile |= TmpData[3];		//低字节
	}
	//如果脉冲系数为默认值3600,则自动开启脉冲系数校准功能,dxl,2015.4.17后改为7200
	if((3600 == FeatureCoef)||(7200 == FeatureCoef))
	{
		setAdjustFunction(ENABLE);//开启脉冲系数校准功能
	}
	
}

/*********************************************************************
//函数名称	:Pulse_GetSpeed
//功能		:获取脉冲瞬时速度,单位km/h
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
//		:
*********************************************************************/
u8 Pulse_GetSpeed(void)
{
	#if(TACHOGRAPHS_19056_TEST)
    	return Tachographs_Test_Get_Speed();
	#else
		return PulseSpeed;
	#endif
}
/*********************************************************************
//函数名称	:Pulse_GetMinuteSpeed
//功能		:获取脉冲每分钟平均速度
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
//		:
*********************************************************************/
u8 Pulse_GetMinuteSpeed(void)
{
	return PulseMinuteSpeed;
}
/*********************************************************************
//函数名称	:Pulse_GetTotalMile
//功能		:获取脉冲总里程
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:单位是0.01公里
//		:
*********************************************************************/
u32 Pulse_GetTotalMile(void)
{
	return PulseTotalMile;
}
/*********************************************************************
//函数名称	:Pulse_Time3Isr
//功能		:定时器3中断
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
//		:
*********************************************************************/
void Pulse_Tim3Isr(void)
{
	u8	i = 0;
	static u32	TestCount = 0;

	if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
  	{
		if(0xE3 == TestStatus)
		{
			TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
			//定时器3计数清0
			TIM_SetCounter(TIM3, 0);
			
			TestCount++;
			if(0 == TestCount%2)
			{
				GPIO_SetBits(GPIOB, GPIO_Pin_4);
			}
			else
			{
				GPIO_ResetBits(GPIOB, GPIO_Pin_4);
			}
		}
		else if(0xE1 == TestStatus)
		{
			TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
			//定时器3计数清0
			TIM_SetCounter(TIM3, 0);
			Recorder_MileageTest();//每秒钟发送一个应答帧
		}
		else
		{
    			TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
			//定时器3计数清0
			TIM_SetCounter(TIM3, 0);
			//关闭定时器3
			TIM_Cmd(TIM3, DISABLE);
			ExtiCount = 0;
			PulseSpeed = 0;
			LastPulseSpeed = 0;
			//定时器4计数清0
			TIM_SetCounter(TIM4, 0);
			//关闭定时器4
			TIM_Cmd(TIM4, DISABLE);
			for(i=0; i<TIME_COUNT_ARRAY_SIZE; i++)
			{
				TimeCountArray[i] = 0;
			}
			//TimeCountArrayCount = 0;
		}
		
	}
}
/*********************************************************************
//函数名称	:Pulse_Time4Isr
//功能		:定时器4中断
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:之前是0.2秒计算1次瞬时速度，由于这种方法实时性滞后不能满足检测要求因此
//		:放入到外部中断中计算瞬时速度，该定时器现在不用了
*********************************************************************/
void Pulse_Tim4Isr(void)
{
	
	if(TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
  	{
    		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
		//定时器4计数清0
		TIM_SetCounter(TIM4, 0);
		//关闭定时器4
		TIM_Cmd(TIM4, DISABLE);
		
	}
}
/*********************************************************************
//函数名称	:Pulse_ExtiIsr
//功能		:外部中断
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
//		:
*********************************************************************/
void Pulse_ExtiIsr(void)
{
	
	
	static u32	TestCount = 0;
	
	//读取速度类型及ACC状态
	//if((0 != SpeedFlag)||(0 == Io_ReadStatusBit(STATUS_BIT_ACC)))
	//{
		/* Clear the EXTI line 0 pending bit */
    		//EXTI_ClearITPendingBit(PULSE_EXTI_LINE);
		//return ;
	//}
	
	if(0xE2 == TestStatus)
	{
		TestCount++;
		if(0 == TestCount%2)
		{
			GPIO_SetBits(GPIOB, GPIO_Pin_4);
		}
		else
		{
			GPIO_ResetBits(GPIOB, GPIO_Pin_4);
		}
	}
	else
	{
		Pulse_MileAdd();
	}
	/* Clear the EXTI line 0 pending bit */
    	EXTI_ClearITPendingBit(PULSE_EXTI_LINE);
}
/*********************************************************************
//函数名称	:Pulse_MileAdd
//功能		:里程累加
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
//		:
*********************************************************************/
void Pulse_MileAdd(void)
{
	u16	TimeCount = 0;
        
	u32	sum = 0;
	u16	temp;
        float   v = 0;
        u8      Acc = 0;
	
	static  u8	Count3 = 0;
        
    Acc = Io_ReadStatusBit(STATUS_BIT_ACC);//dxl,2013.11.21加入ACC的限制
        
	ExtiCount++;
	PulseMileCount++;
	myPulseCnt++;          //校准脉冲计数+1
	if(1 == ExtiCount)
	{
		//定时器3计数清0
		TIM_SetCounter(TIM3, 0);
		//打开定时器3
		TIM_Cmd(TIM3, ENABLE);
		//打开定时器4
		TIM_Cmd(TIM4, ENABLE);
	}
	else 
	{
		//读计数器的值
		TimeCount = TIM_GetCounter(TIM3);
		if(TimeCount >= TimeCountMin)
		{
			//计算里程
			if((PulseMileCount > (FeatureCoef/50))&&(1 == Acc))//大于等于0.02公里
			{
				PulseTotalMile += PulseMileCount*100/FeatureCoef;
				PulseMileCount = 0;
			}
#ifdef SPEED_PINGJUN
			TimeCountArrayCount++;
			if(TimeCountArrayCount >= TIME_COUNT_ARRAY_SIZE)
			{
				TimeCountArrayCount = 0;
			}
			TimeCountArray[TimeCountArrayCount] = TimeCount;
#else 
                        TimeCountArray[0] = TimeCount; 
                	sum = TimeCountArray[0];
#endif
#ifdef  SPEED_PINGJUN
        u8	i = 0;
	u16	CountMin = 0xffff;
	u16	CountMax = 0;
                sum = 0;
		for(i=0; i<TIME_COUNT_ARRAY_SIZE; i++)
		{
			if(TimeCountArray[i] > CountMax)
			{
				CountMax = TimeCountArray[i];
			}
			if(TimeCountArray[i] < CountMin)
			{
				CountMin = TimeCountArray[i];
			}
			sum += TimeCountArray[i];
                }
		sum -= CountMin;
		sum -= CountMax;
		sum = sum/3;
#endif
                if(0 == sum)
                {
                    return;
                }
                if(1 == Acc)
                {
		v = (3600.0*(u32)1000000.0)/((float)sum*50.0*(float)FeatureCoef);//对应50us计数单位
                temp = (int)(v);
                if((v-temp)>=0.5)
                {
                    temp += 1;
                }
#ifdef SPEED_LVBO
		if(temp > LastPulseSpeed)
		{
			if((temp-LastPulseSpeed) >= 50)//两次跳变50认为是干扰
			{
				Count3++;
				if(Count3 >= 100)//干扰产生连续100次，认为是异常掉电造成突变
				{
					Count3 = 0;
					PulseSpeed = temp;
					LastPulseSpeed = temp;
				}
			}
			else
			{
				Count3 = 0;
				PulseSpeed = temp;
				LastPulseSpeed = temp;
			}
		}
		else
		{
			PulseSpeed = temp;
			LastPulseSpeed = temp;
		}
#else
                PulseSpeed = temp;
		LastPulseSpeed = temp;
#endif
                }
                else
                {
                        PulseSpeed = 0;
		        LastPulseSpeed = 0;
                }
                
                // 为编译不报错而添加
                if(0 == Count3)
                {
                
                }
                if(0 == LastPulseSpeed)
                {
                
                }
                        
                
		}
		else
		{
			PulseMileCount--;//脉冲干扰
			myPulseCnt--;
		}
		//定时器3计数清0
		TIM_SetCounter(TIM3, 0);
	}	
}
/*********************************************************************
//函数名称	:Pulse_TimeTask
//功能		:脉冲检测定时任务
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:0.2秒调用1次
*********************************************************************/
FunctionalState  Pulse_TimeTask(void)
{
	


	u8	Speed1 = 0;//脉冲速度
	u8	Speed2 = 0;//GPS速度
  u8  flag = 0;
	static	u16	Count4 = 0;
	static	u16	Count5 = 0;
	u8	TmpData[5];
	static  u16	Count1 = 0;
	static  u16	Count2 = 0;
	static  u16	Count3 = 0;
	static  u16	TotalPulseSpeed = 0;//瞬时速度之和
	static  u32	LasePulseTotalMile = 0;//上一次累计行驶里程，即总里程,单位是0.01公里
	
        if(0 == (SpeedFlag&0x01))//脉冲速度
	{
	        //计算累计里程
	        Count1++;
	        if(Count1 >= 5)//1秒写一次里程
	        {
		        Count1 = 0;
		        //if(0 == Io_ReadAlarmBit(ALARM_BIT_VSS_FAULT))dxl,2013.11.21,该行有vbug，当选择GPS速度时有问题，两种累计里程都在记录
                
		        //{
                        if(LasePulseTotalMile != PulseTotalMile)
                        {
			                TmpData[3] = PulseTotalMile;	//低字节
			                TmpData[2] = PulseTotalMile>>8;	//中字节
			                TmpData[1] = PulseTotalMile>>16;	//中字节
			                TmpData[0] = PulseTotalMile>>24;	//高字节
			                FRAM_BufferWrite(FRAM_MILEAGE_ADDR, TmpData, FRAM_MILEAGE_LEN); //存储总里程
                        }
		       // }
	        }
	        //计算每分钟平均速度
	        Count2++;
	        if(Count2 >= 5)
	        {
		        Count2 = 0;
		        Count3++;
		        TotalPulseSpeed += PulseSpeed;
		        if(Count3 >= 60)
		        {
			      Count3 = 0;
			      PulseMinuteSpeed = TotalPulseSpeed/60;
			      TotalPulseSpeed = 0;
		        }
	        }
      }
	//置位脉冲速度传感器故障
        //行标检测时屏蔽开始
	Speed1 = Pulse_GetSpeed();
	Speed2 = Gps_ReadSpeed();
	if((1 == Io_ReadStatusBit(STATUS_BIT_ACC))
	   &&(1 == Io_ReadStatusBit(STATUS_BIT_NAVIGATION)))
		  // &&(0==SpeedMonitor_GetCurSpeedType()))
	{
		if(Speed2 > 15)
		{
			if(0 == Speed1)
			{
				Count5 = 0;
				Count4++;
				if(Count4 >= 300)//持续1分钟
				{
					Count4 = 0;
					//置位速度传感器故障
					Io_WriteAlarmBit(ALARM_BIT_VSS_FAULT, SET);
                    //设置速度类型为gps速度类型
                    if((0x03 != SpeedFlag)&&(0x02 == SpeedFlag))//dxl,2015.5.11,0x00:手动脉冲,0x01:手动GPS,0x02:自动脉冲,0x03:自动GPS
                    {
                        //之前是脉冲切换为gps
                        flag = 0x03;//dxl,2015.5.11
                        EepromPram_WritePram(E2_SPEED_SELECT_ID, &flag, 1);
                        SpeedFlagUpdatePram();
                    }
				}
			}
			else
			{
				Count4 = 0;
				Count5++;
				if(Count5 >= 300)//持续1分钟
				{
					Count5 = 0;
					//清除速度传感器故障
					Io_WriteAlarmBit(ALARM_BIT_VSS_FAULT, RESET);
                    //设置速度类型为脉冲速度类型
                    if((0x02 != SpeedFlag)&&(0x03 == SpeedFlag))//dxl,2015.5.11
                    {
                        //之前是GPS切换为脉冲
                        flag = 0x02;//dxl,2015.5.11
                        EepromPram_WritePram(E2_SPEED_SELECT_ID, &flag, 1);
                        SpeedFlagUpdatePram();
                    }
				}
			}
		}
		else
		{
			Count4 = 0;
			Count5 = 0;
		}
	}
	else
	{
		Count4 = 0;
		Count5 = 0;
	}
	return ENABLE;
}
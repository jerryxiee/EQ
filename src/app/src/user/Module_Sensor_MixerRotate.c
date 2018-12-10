/*************************************************************************
* Copyright (c) 2013,深圳市伊爱高新技术开发有限公司
* All rights reserved.
* 文件名称 : Module_Sensor_MixerRotate.c
* 当前版本 : 1.1
* 开发者   : Shigle 
* 修改时间 : 2013年10月9日


* 2013年10月9日为基线版本1.0, 后续功能如有增减或修改，将要求以下追加说明
* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
* 1、正反转传感器的硬件连接：
    V+     ---------- 32PIN摄像头接口的电源正- P17- 红线
    V-     ---------  32PIN 摄像头接口的电源负 - P18-  黑线
    转速线 ---------- 32PIN的空调  - P22- 绿白线 - STM32:PE9
    方向线 ---------- 32PIN的车门2 - P9-  粉红线 - STM32:PF15
* 2、方向线检测口, 低电平，则设为正转；为高电平，则设为反转
* 3、转速放大10X，单位: 转/分钟, 例如，返回的正反转转速值为52 ,表明当前检测到滚筒转速为5.2转/分钟
* 4、当连续60秒未接收到转速线上的外部中断，则判定滚筒停止，相关量相应复位。
* 5、正反转传感器的供电: ACC OFF 停止供电， ACC ON 开启供电
* 2013年10月28日升为版本1.1
* 1、修改为当正反转的状态发生变化时，将触发一条位置信息上报。
* 2013年11月09日升为版本1.2
* 1、修改为上报平台的状态值: 00:表示 无效， 11：表示停止
*************************************************************************/



//***************包含文件*****************
#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"


#include "include.h"


#ifdef ZHENGFANZHUAN

//*************** 全局变量 定义区 *****************

MIXER_CAR_OBJECT        gMixerCarObject;


/*******************************************************************************
**  函数名称  :	MixerRotate_Exti_Init
**  函数功能  : 搅拌车正反转传感器 外部中断初始化
*******************************************************************************/
void MixerRotate_Exti_Init(void)
{
  
  //   转速线----------32PIN的空调    p22-绿白-PE9
	  EXTI_InitTypeDef   EXTI_InitStructure;
  	GPIO_InitTypeDef   GPIO_InitStructure;
  	NVIC_InitTypeDef   NVIC_InitStructure;

  	/* Enable GPIOE clock */
  	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
  	/* Enable SYSCFG clock */
  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
  
  	/* Configure PE9 pin as input floating */
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_9;
  	GPIO_Init( GPIOE , &GPIO_InitStructure );

  	/* Connect EXTI Line2 to PE9 pin */
  	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource9);

  	/* Configure EXTI Line9 */
  	EXTI_InitStructure.EXTI_Line = EXTI_Line9;
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; //EXTI_Trigger_Rising_Falling;
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  	EXTI_Init( &EXTI_InitStructure );

  	/* Enable and set EXTI14 Interrupt to the lowest priority */
  	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	  NVIC_Init(&NVIC_InitStructure);
}



/*******************************************************************************
**  函数名称  :	MixerRotate_IO_Polarity_Init
**  函数功能  : 搅拌车正反转传感器 脉冲 检测外部中断  使能
*******************************************************************************/
void MixerRotate_Exti_Enable(void)
{
  
	  NVIC_InitTypeDef   NVIC_InitStructure;
    
	/* Enable and set EXTI9 Interrupt to the lowest priority */
  	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn ; //EXTI15_10_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	  NVIC_Init(&NVIC_InitStructure);
}

/*******************************************************************************
**  函数名称  :	MixerRotate_IO_Polarity_Init
**  函数功能  : 搅拌车正反转传感器 脉冲 检测外部中断  禁止
*******************************************************************************/
void MixerRotate_Exti_Disable(void)
{
  
	  NVIC_InitTypeDef   NVIC_InitStructure;
	/* Enable and set EXTI14 Interrupt to the lowest priority */
  	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn ; // EXTI15_10_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
	  NVIC_Init(&NVIC_InitStructure);
}


/*******************************************************************************
**  函数名称  :	MixerRotate_Exti_Isr
**  函数功能  : 搅拌车正反转传感器 脉冲 检测外部中断服务程序
*******************************************************************************/
void MixerRotate_Exti_Isr(void)
{
  u32 nowtimer;
//脉冲信号线
    if(EXTI_GetITStatus(EXTI_Line9) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line9); //清除外部中断标志
  
        if( gMixerCarObject.bMixterStateChange == 0)
        {
            gMixerCarObject.bMixterStateChange = 1;
        }
        
        if( gMixerCarObject.PulseStart ==0)
        {
            gMixerCarObject.PulseStart = 1;  
            gMixerCarObject.PulseCnt = 0;
            gMixerCarObject.RevSampleTime = Timer_Val();//更新震动产生的时刻
        }
        else
        {
            gMixerCarObject.PulseCnt++;  //脉冲数加1
            
#if 1
            nowtimer = Timer_Val();//更新震动产生的时刻
            gMixerCarObject.OneLoopTime = nowtimer - gMixerCarObject.RevSampleTime ;
            gMixerCarObject.RevSampleTime = nowtimer ; //更新震动产生的时刻
            
            gMixerCarObject.Rev = 12000/( gMixerCarObject.OneLoopTime*gMixerCarObject.NumMagneticCylinder); 
            /*
            if(   (1200 - gMixerCarObject.Rev*gMixerCarObject.OneLoopTime*gMixerCarObject.NumMagneticCylinder)*2 > gMixerCarObject.OneLoopTime*gMixerCarObject.NumMagneticCylinder    )
            {
               gMixerCarObject.Rev++;
            }
          */
#else            
       
         //   if(gMixerCarObject.PulseCnt == 1)
             if(gMixerCarObject.PulseCnt%2 == 1)
            {
              
                gMixerCarObject.RevSampleTime = Timer_Val();//更新震动产生的时刻
                
          //  }else if(gMixerCarObject.PulseCnt >1) {     //搅拌桶一周安装了4个磁钢，转一周产生4个脉冲
             }else if(gMixerCarObject.PulseCnt %2 == 0 ) {     //搅拌桶一周安装了4个磁钢，转一周产生4个脉冲
               
       //         gMixerCarObject.PulseCnt = 0;
               
                nowtimer = Timer_Val();//更新震动产生的时刻
                gMixerCarObject.OneLoopTime = nowtimer - gMixerCarObject.RevSampleTime ;
                gMixerCarObject.Rev = 1200/( gMixerCarObject.OneLoopTime*gMixerCarObject.NumMagneticCylinder);      
                gMixerCarObject.RevSampleTime = nowtimer ; //更新震动产生的时刻
            }
            
#endif           
            
            
         }      

    }
    
 
}


/*******************************************************************************
**  函数名称  :	MixerRotate_IO_Polarity_Init
**  函数功能  : 搅拌车正反转传感器 极性（方向）检测IO口初始化
*******************************************************************************/
void MixerRotate_IO_Speed_Init(void)
{
  //转速线----------32PIN的空调    p22-绿白-PE9
	  EXTI_InitTypeDef   EXTI_InitStructure;
  	GPIO_InitTypeDef   GPIO_InitStructure;
  	NVIC_InitTypeDef   NVIC_InitStructure;

  	/* Enable GPIOE clock */ 
  	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
  	/* Enable SYSCFG clock */
  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
  
  	/* Configure PE9 pin as input floating */
    
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  	GPIO_Init(GPIOB, &GPIO_InitStructure);

  	/* Connect EXTI Line2 to PE9 pin */
  	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource9);

  	/* Configure EXTI Line9 */
  	EXTI_InitStructure.EXTI_Line = EXTI_Line9;
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling; //EXTI_Trigger_Rising_Falling;
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  	EXTI_Init(&EXTI_InitStructure);

  	/* Enable and set EXTI9 Interrupt to the lowest priority */
  	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn ; //EXTI15_10_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	  NVIC_Init(&NVIC_InitStructure);
    
    
    
}






/*******************************************************************************
**  函数名称  :	MixerRotate_IO_Polarity_Init
**  函数功能  : 搅拌车正反转传感器 极性（方向）检测IO口初始化
*******************************************************************************/
void MixerRotate_IO_Polarity_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStructure;
  
  /* 使能GPIO时钟 */
  
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
  
  //方向线----------32PIN的车门2    PF15
  
  /* 配置 正反转传感器 极性检测口 为输入模式*/
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15 ;
  GPIO_Init( GPIOF, &GPIO_InitStructure);
  
}
/*******************************************************************************
**  函数名称  :	MixerRotate_RAM_Init
**  函数功能  : 搅拌车正反转 RAM 全局变量初始化
*******************************************************************************/
void MixerRotate_RAM_Init(void)
{

    gMixerCarObject.RevErrorFlag = 0 ; //GprsNetwork_GetMixerSensorFail();//初始化搅拌车正反转传感器无故障
    gMixerCarObject.Reserver = 0;
    
    gMixerCarObject.MixerFailTotal =0 ; //初始化清零搅拌车正反转传感器故障计数
    gMixerCarObject.PulseCnt = 0 ;      //脉冲计数
    gMixerCarObject.PulseStart = 0 ;    //脉冲是始计数
    gMixerCarObject.updata = 0;          //是否更新

    gMixerCarObject.bMixterStateChange= 0 ;
    gMixerCarObject.Mixer_Rotate_Polarity_last = 0;
    gMixerCarObject.Mixer_Rotate_Polarity_current= 0;

    
    gMixerCarObject.CarRunStopCounter = 0 ; //Shigle 2012-11-12  
    
    gMixerCarObject.RevSampleTime = Timer_Val();//更新震动产生的时刻

    
    gMixerCarObject.Rev = 0;            //初始化搅拌车正反转传感器转速为0
    gMixerCarObject.RevState =  ROTATE_STATE_UNKNOWNED ; //初始化为未知状态
    gMixerCarObject.NumMagneticCylinder = NUM_MAGNETIC_CYLINDER; //每个滚筒磁缸数量 

}




/*******************************************************************************
*  函数名称  : MixerRotate_Task_Init
*  函数功能  : 搅拌车滚筒任务初始化
*******************************************************************************/
void MixerRotate_Task_Init(void)
{
  
  
  MixerRotate_RAM_Init();
  
  MixerRotate_IO_Polarity_Init();

 // MixerRotate_IO_Speed_Init();

  MixerRotate_Exti_Init();
  
  MixerRotate_Exti_Enable();
 // MixerRotate_Exti_Disable();

}



/*******************************************************************************
*  函数名称  : MixerRotate_Update_RotateState
*  函数功能  : 获得搅拌车滚筒的状态: 00：停止 01:反转 10:正转 11：未知
*******************************************************************************/
void MixerRotate_Update_RotateState(void)
{
      
    static u8 cnt = 0 ;
    if( cnt++ > 1*SYSTICK_1SECOND ){
      
      cnt = 0 ;
      
    }else{
      
      return ;
      
    }

    switch( gMixerCarObject.RevState )
    {
      
        case 0://00:停止
#if 1
          Io_WriteSelfDefine2Bit( DEFINE_BIT_13 , SET );
          Io_WriteSelfDefine2Bit( DEFINE_BIT_14 , SET );
          
#else
          Io_WriteSelfDefine2Bit( DEFINE_BIT_13 , RESET );
          Io_WriteSelfDefine2Bit( DEFINE_BIT_14 , RESET );
#endif
          
          break;
          
        case 1: //01反转
          Io_WriteSelfDefine2Bit( DEFINE_BIT_13 , SET );
          Io_WriteSelfDefine2Bit( DEFINE_BIT_14 , RESET ); 
          break;
          
        case 2: //10:正转
          
          Io_WriteSelfDefine2Bit( DEFINE_BIT_13 , RESET );
          Io_WriteSelfDefine2Bit( DEFINE_BIT_14 , SET );
          break;
          
        case 3: //11:未知
          
#if 1
          Io_WriteSelfDefine2Bit( DEFINE_BIT_13 , RESET );
          Io_WriteSelfDefine2Bit( DEFINE_BIT_14 , RESET );          
#else

          Io_WriteSelfDefine2Bit( DEFINE_BIT_13 , SET );
          Io_WriteSelfDefine2Bit( DEFINE_BIT_14 , SET );          
#endif         
     
          break;
          
        default:
          
#if 1
          Io_WriteSelfDefine2Bit( DEFINE_BIT_13 , RESET );
          Io_WriteSelfDefine2Bit( DEFINE_BIT_14 , RESET );          
#else

          Io_WriteSelfDefine2Bit( DEFINE_BIT_13 , SET );
          Io_WriteSelfDefine2Bit( DEFINE_BIT_14 , SET );          
#endif  
          
          break;
          
    }
    

}
/*******************************************************************************
*  函数名称  : Task_MixerRotate()
*  函数功能  : 搅拌车滚筒定时任务
*******************************************************************************/
FunctionalState Task_MixerRotate(void)
{
  	u8	ACC;
    static u8 InitCnt = 0 ;
    static u8 RevState_Current = ROTATE_STATE_UNKNOWNED ; 
    
    


   //------------------------ACC OFF ------------------------------------  
    
    ACC = Io_ReadStatusBit(STATUS_BIT_ACC);
    if( 0 == ACC )
    {
      
      //ACC OFF 关闭时，停止给正反转传感器供电
      //久通油耗传感器与摄像头共用接口，读油耗传感器接入标志,1为接入,0为未接入
      if(0 == JointechOiLCost_Online_Get())
      {
        CAMERA_POWER_OFF();//关摄像头电源
      }
       //CAMERA_POWER_OFF(); //搅拌车正反转传感器供电控制口 0:停止 1:,dxl,2014.3.19
       
       gMixerCarObject.RevState = ROTATE_STATE_UNKNOWNED ;  //搅拌车停止状态
       gMixerCarObject.Rev = 0;  //搅拌车转速　０　

       gMixerCarObject.bMixterStateChange =0; //转向无变化
       gMixerCarObject.PulseStart = 0; //脉冲计数起启位重新开始 
       gMixerCarObject.PulseCnt = 0; //脉冲计数 归零
       gMixerCarObject.Mixer_Rotate_Polarity_last = 0;
       gMixerCarObject.Mixer_Rotate_Polarity_current = 0 ;//更新上一个极性状态位
       gMixerCarObject.MixerFailTotal = 0;
       gMixerCarObject.RevSampleTime = Timer_Val();//更新震动产生的时刻

       MixerRotate_Update_RotateState();
       
       return ENABLE;
    }
    
    
    
    //------------------------ACC ON ------------------------------------    
    
    CAMERA_POWER_ON();
    
    
    

//======================正反转状态变换===================
    if(  GPIO_ReadInputDataBit( GPIOF ,  GPIO_Pin_15 ) )
    {
        gMixerCarObject.Mixer_Rotate_Polarity_current = 1; //低电平
        
    } else {
      
        gMixerCarObject.Mixer_Rotate_Polarity_current = 0; //高电平或悬空
    }
    
    
    
    if( InitCnt== 0   ){
      InitCnt = 1;
      
      if( gMixerCarObject.Mixer_Rotate_Polarity_current ) 
      {
        
          gMixerCarObject.RevState = 1;//如果单前极性为高电平，则设为反转
          
      } else {
        
          gMixerCarObject.RevState = 2;//如果单前极性为低电平，则设为正转
          
      }    

    }
    
    //------------------------------------------------------          
    if(  gMixerCarObject.bMixterStateChange ==0)
    {
        //以下进行 从搅拌车滚筒状态转换， 由正转->停止 或 反转->停止 判断
        if( (gMixerCarObject.RevState== 1  )||(gMixerCarObject.RevState== 2 ) )
        {
          
            if(  gMixerCarObject.CarRunStopCounter++ > NUM_SENCOND_NOEXTI * SYSTICK_1SECOND   )
            {
              //如果连连续10*15.625ms没有侦测到 滚筒脉冲信息，则认为 滚筒停止
                gMixerCarObject.RevState = 0  ;
                gMixerCarObject.Rev = 0;
            }
            
        }

    } else if(  gMixerCarObject.bMixterStateChange < 4  ) {//滚筒发出 脉冲信号，则进入以下分支 
    
          if( gMixerCarObject.RevState==0 )
         {
              gMixerCarObject.bMixterStateChange =0;
              gMixerCarObject.Mixer_Rotate_Polarity_last = gMixerCarObject.Mixer_Rotate_Polarity_current;
              
              if( gMixerCarObject.Mixer_Rotate_Polarity_current ) 
              {
                  gMixerCarObject.RevState = 1;//如果单前极性为高电平，则设为反转
              }
              else
              {
                  gMixerCarObject.RevState = 2;//如果单前极性为低电平，则设为正转
              }
              
         } else {
      
            if( gMixerCarObject.Mixer_Rotate_Polarity_current != gMixerCarObject.Mixer_Rotate_Polarity_last){
              
              //滚筒发出 脉冲信号有变化，但计数还没有连续累加到上限
               gMixerCarObject.bMixterStateChange++;
            } else {
              
              //滚筒发出 脉冲信号测有变化，则重新清零 
                gMixerCarObject.bMixterStateChange =0; 
            }
            
         }
         
         //将滚筒传感器故障侦测相关量清零
          gMixerCarObject.MixerFailTotal = 0;
          gMixerCarObject.RevErrorFlag = 0;

          gMixerCarObject.CarRunStopCounter = 0; 
          
          
    }else{
      
           //滚筒发出 脉冲信号有变化，计数连续累加到上限,则重新清零, 则更新当前极性
           gMixerCarObject.bMixterStateChange = 0;
           
           //当搅拉车正反转极性发生变化时.....
           gMixerCarObject.PulseStart = 0; //脉冲计数起启位重新开始 
           gMixerCarObject.PulseCnt = 0; //脉冲计数 归零
           gMixerCarObject.Rev = 0; //转速清零　
           
           gMixerCarObject.Mixer_Rotate_Polarity_last = gMixerCarObject.Mixer_Rotate_Polarity_current;//更新上一个极性状态位
          
           gMixerCarObject.CarRunStopCounter = 0 ; //刷新停止计时
           
           if( gMixerCarObject.Mixer_Rotate_Polarity_current ) 
           {
              gMixerCarObject.RevState = 1;//如果单前极性为高电平，则设为反转
           }
           else
           {
              gMixerCarObject.RevState = 2;//如果单前极性为低电平，则设为正转
           }
           
           //将滚筒传感器故障侦测相关量清零
            gMixerCarObject.MixerFailTotal = 0;
           
    }
    
    
    MixerRotate_Update_RotateState();
    
    
    if( RevState_Current != gMixerCarObject.RevState )
    {
        RevState_Current = gMixerCarObject.RevState ;
        //SetEvTask( EV_REPORT );
        Report_UploadPositionInfo(CHANNEL_DATA_1);
        Report_UploadPositionInfo(CHANNEL_DATA_2);
    }
    
    return ENABLE;
    
}



/*******************************************************************************
*  函数名称  : MixerRotate_TimeTask()
*  函数功能  : 搅拌车滚筒定时任务
*******************************************************************************/
FunctionalState MixerRotate_TimeTask(void)
{
  
  return  Task_MixerRotate();
}



/*******************************************************************************
*  函数名称  : MixerRotate_Get_RotateState
*  函数功能  : 获得搅拌车滚筒的状态: 00：停止 01:反转 10:正转 11：未知
*******************************************************************************/
u8 MixerRotate_Get_RotateState(void)
{
  
#if 1
  
    return  gMixerCarObject.RevState;
    
#else

    
    u8  retval =  ROTATE_STATE_UNKNOWNED ;
    switch( gMixerCarObject.RevState )
    {
      
        case 0:
          retval =  ROTATE_STATE_STOP      ;//00:停止
          break;
        case 1:
          retval =  ROTATE_STATE_POSITIVE  ;//10:正转
          break;
        case 2:
          retval =  ROTATE_STATE_REVERSE   ;//01反转
          break;
        case 3:
          retval =  ROTATE_STATE_UNKNOWNED ;//11:未知
          break;
        default:
           retval =  ROTATE_STATE_STOP      ;//00:停止
          break;
    }
    
    return  retval;
    
#endif
    
}



/*******************************************************************************
*  函数名称  : MixerRotate_Get_RotateSpeed
*  函数功能  : 获得搅拌车滚筒的转速
*******************************************************************************/
u8 MixerRotate_Get_RotateSpeed(void)
{

    if( (gMixerCarObject.RevState &0x03) == 0)
    {
         gMixerCarObject.Rev = 0;
    }
 
    if( gMixerCarObject.bMixterStateChange >1 )
    {
        gMixerCarObject.Rev = 0;
    }
    
    return  gMixerCarObject.Rev;

}



#endif

/******************************************************************************
**                            End Of File
******************************************************************************/


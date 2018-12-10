/*
********************************************************************************
*
* Filename      : Acceleration.h
* Version       : V1.00
* Programmer(s) : zengliang
* @date         : 2016-10-28 
* 
********************************************************************************
*/


/*
********************************************************************************
*                               INCLUDES
********************************************************************************
*/
#include "include.h"
#include "Public.h"
//#include "VDR_Doubt.h"
//#include "VDR_Speed.h"
/*
********************************************************************************
*                               DEFIEN START
********************************************************************************
*/
#ifdef ACCELERATION_H

/*
********************************************************************************
*                             LOCAL DEFINES
********************************************************************************
*/


/*
********************************************************************************
*                         LOCAL DATA TYPES
********************************************************************************
*/

/*
********************************************************************************
*                          LOCAL VARIABLES
********************************************************************************
*/
enum ThreeEnerStatus{
  ENER_STATUS_SHARP_ACCELERATE = 0, //急加速
  ENER_STATUS_SHARP_SLOWDOWN, 			//急减速
  ENER_STATUS_SHARP_TRUN,						//急转弯
  ENER_STATUS_MAX
};

static u8 Initflag=0;
static u8 acce_flg=0;//急加速标志,急减速标志,急转弯
static u8 Report[150],Report_len=0;
static u8 Acc_Threshold[4]={20,19,18,17};
static u8 Slowdwn_Threshold[2]={12,10};
static u8 ThreeEmerClrDlyTime[ENER_STATUS_MAX] = {0,};

static u8 ThreeEmerCheckDisable[3] = {0,0,0};//三级禁能标志

static u16 EmerCornerAngleMax=15;
#define EMER_TURN_ANGLE_SPEED    25
#define	EMER_CLR_DLY_COUNT				5
/*
********************************************************************************
*                          GLOBAL VARIABLES
********************************************************************************
*/

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
void CleanInitFlag(void)
{
  Initflag=0;
}


void ThreeEmerAlarmBit(u32 bit, FlagStatus state)
{
  if(bit >= 32)return;
  if(SET==state)
  {
    PUBLIC_SETBIT(acce_flg, bit);
  }
  else if(RESET==state)
  {
    PUBLIC_CLRBIT(acce_flg, bit);
  }
} 

u8 ThreeEmerCleckAlarmBit(u32 bit)
{
  
  if(PUBLIC_CHECKBIT(acce_flg,bit))
  {
    return 0;
  }
  return 1;
} 

/**
* @brief  向平台发送透传命令
* @param  None
* @retval None
*/
void Acceleration_8900Send(u8 *Acc,u8 len)
{
  EIExpand_PotocolSendData(0xf8f0,Acc,len);
}

/**
* @brief  设置急加速急减速阀值接口
* @param  None
* @retval None
*/
void Set_Emer_Threshold(u8 *Threshold,u8 emer_type_flag)
{
  u8 i=0;
  if(emer_type_flag==0)
  {
    for(i=0;i<4;i++)
    {
      Acc_Threshold[i]=Threshold[i];
    }
  }
  else if(emer_type_flag==1)
  {
    for(i=0;i<2;i++)
    {
      Slowdwn_Threshold[i]=Threshold[i];
    }        
  }
  else
  {
    EmerCornerAngleMax=Threshold[0];
    EmerCornerAngleMax=EmerCornerAngleMax<<8|Threshold[1];
  }
}

/**
* @brief  读取急加速急减速阀值接口
* @param  None
* @retval None
*/
u8* Read_Emer_Threshold(u8 emer_type_flag)
{
  if(emer_type_flag==0)
  {
    return Acc_Threshold;
  }
  else if(emer_type_flag==1)
  {
    return Slowdwn_Threshold;       
  }
  return NULL;
}

//zengliang add 2016-12-7
extern u8 SpeedFlag;
u8 Get_Speed(u8 *CurrentSpeed)
{
	static u8 chk_valid = 0;
	static u8 pos_valid_cnt = 0;
  //优先使用脉冲速度
  if(Pulse_GetSpeed())
  {
    *CurrentSpeed = Pulse_GetSpeed();
		chk_valid = 1;
  }
  else
  {
    *CurrentSpeed = Gps_ReadSpeed();
		if(!Gps_ReadStatus())//未定位
		{
			pos_valid_cnt = 0;
			chk_valid = 0;
		}
		else
		{
			if(0 == chk_valid)
			{
				if(pos_valid_cnt++ > 5)
					chk_valid = 1;
			}
		}
  }
  
  return chk_valid;
}

/**
* @brief 3急状态产生上报
* @param  ReportType:上报类型
* @retval None
*/
static void Emer_Acce_Occur_Report(u8 ReportType)
{
  //检测是否已经有急加速上报
  if(ThreeEmerCleckAlarmBit(ReportType)){
    //Public_PlayTTSVoiceStr("急加速"); 
    //LcdShowCaptionEx((char *)"急加速",2); 
    ThreeEmerAlarmBit(ReportType,SET);
    //产生急加速时间，上报
    Report_len= Report_GetPositionInfo(Report);
    RadioProtocol_PostionInformationReport(CHANNEL_DATA_1|CHANNEL_DATA_2,Report,Report_len);
		ThreeEmerAlarmBit(ReportType,RESET);//上报之后清除，只上报一次
  }
  
}
/**
* @brief 3急状态清除上报
* @param  ReportType:上报类型
* @retval None
*/
static void Emer_Acce_Clear_Report(u8 ReportType)
{
  
  if(!ThreeEmerCleckAlarmBit(ReportType))	
  {
    if(ThreeEmerClrDlyTime[ReportType]++ > EMER_CLR_DLY_COUNT)	
    {
      ThreeEmerClrDlyTime[ReportType] = 0;
      //Public_PlayTTSVoiceStr("急加速清除"); 
      ThreeEmerAlarmBit(ReportType,RESET);
      Report_len= Report_GetPositionInfo(Report);
      RadioProtocol_PostionInformationReport(CHANNEL_DATA_1|CHANNEL_DATA_2,Report,Report_len);  
    }
  }
}
//直接用速度
void Emer_acce_check(u8 flag)
{
  static u8 oldspeed = 0;
  u8 newspeed ;
	u8 chk_valid = 0;
	//static u8 pos_valid_cnt = 0;
  static float acce;
  static u8 *emer_acce;
  u8	i;
  
  if(flag == 0)
  {
    ThreeEmerAlarmBit(0,RESET);
		chk_valid = Get_Speed(&oldspeed);
    emer_acce=Read_Emer_Threshold(0);
    return ;
  }
  
  emer_acce=Read_Emer_Threshold(0);  
  
  chk_valid = Get_Speed(&newspeed);
  
  if(0 == chk_valid)
  {
    Emer_Acce_Clear_Report(ENER_STATUS_SHARP_ACCELERATE);	
  }
  else
  {
  //使用GPS速度且未定位状态下不检测3级状态
    if((newspeed > oldspeed)&&oldspeed)
    {
      acce = newspeed - oldspeed; //单位是km/h 
      //ThreeEmerAlarmBit(0,RESET);
      if(oldspeed >= 90){
        i = 3;
      }
      else{
        i = oldspeed/30;
      }
      
      if(acce >= emer_acce[i]) 
      {
        Emer_Acce_Occur_Report(ENER_STATUS_SHARP_ACCELERATE);
      }
      else 
      {
        Emer_Acce_Clear_Report(ENER_STATUS_SHARP_ACCELERATE);
      }
    }
    else
    {   
      Emer_Acce_Clear_Report(ENER_STATUS_SHARP_ACCELERATE);	
    }
  }
  oldspeed = newspeed;
}
/**
* @brief 急减速判断
* @param  None
* @retval None
*/
//直接用速度
void Emer_slowdwn_check(u8 flag)
{
  static u8 oldspeed = 0;
  u8 newspeed;
	u8 chk_valid = 0;
	//static u8 pos_valid_cnt = 0;
  static float acce;
  static u8 *emer_slowdwn;
	
  if(flag == 0)
  {
    ThreeEmerAlarmBit(1,RESET);
    chk_valid = Get_Speed(&oldspeed);
    emer_slowdwn=Read_Emer_Threshold(1);
    return ;
  }
  
  chk_valid = Get_Speed(&newspeed);
  
  emer_slowdwn=Read_Emer_Threshold(1);
  
  if(0 == chk_valid)
  {
    Emer_Acce_Clear_Report(ENER_STATUS_SHARP_SLOWDOWN);	
  }
  else
  {
    if((newspeed < oldspeed)&&oldspeed)
    {
      acce = oldspeed - newspeed;  
      //ThreeEnerAlarmBit(1,RESET);	
      if(oldspeed < 30)
      {
        if(acce >= emer_slowdwn[0])
        {
          Emer_Acce_Occur_Report(ENER_STATUS_SHARP_SLOWDOWN);
        }
        else
        {
          Emer_Acce_Clear_Report(ENER_STATUS_SHARP_SLOWDOWN);
        }
      }
      else
      {
        if(acce >= emer_slowdwn[1])
        {
          Emer_Acce_Occur_Report(ENER_STATUS_SHARP_SLOWDOWN);
        }
        else
        {
          Emer_Acce_Clear_Report(ENER_STATUS_SHARP_SLOWDOWN);
        }
      }
    }
    else
    { 
      Emer_Acce_Clear_Report(ENER_STATUS_SHARP_SLOWDOWN);
    } 
  }
  oldspeed = newspeed;
}
/**
* @brief 加速度初始化
* @param  None
* @retval None
*/
void Acceleration_Init(void)
{
  u8 A_Threshold[4]={0,0,0,0};
  u8 L_Threshold[2]={0,0};
  u8 Angle[2]={0,0};
  u8 read_flag=0,i=0;
  if(Initflag)
  {
    return;//已初始化完毕
  }
  read_flag=FRAM_BufferRead(A_Threshold,4,FRAM_ACCELERATION_ADDR);
  
  //检查参数是否合法  
  if(read_flag)//校验通过
  {
    for(i=0;i<4;i++)    
    {
      Acc_Threshold[i]=A_Threshold[i];
    }
    if(A_Threshold[0]==0||A_Threshold[1]==0||A_Threshold[2]==0||A_Threshold[3]==0)
    {
      ThreeEmerCheckDisable[0] = 1;
    }
  }
  else//default
  {
    FRAM_BufferWrite(FRAM_ACCELERATION_ADDR,Acc_Threshold,FRAM_ACCELERATION_LEN);
  }
  
  read_flag=FRAM_BufferRead(L_Threshold,2,FRAM_DECELERATION_ADDR);
  //检查参数是否合法
  if(read_flag)//校验通过
  {
    Slowdwn_Threshold[0]=L_Threshold[0];
    Slowdwn_Threshold[1]=L_Threshold[1];
    if(L_Threshold[0]==0||L_Threshold[1]==0)
    {		
      ThreeEmerCheckDisable[1] = 1;
    }
  }
  else
  {
    FRAM_BufferWrite(FRAM_DECELERATION_ADDR,Slowdwn_Threshold,FRAM_DECELERATION_LEN);
  }
  
  read_flag=FRAM_BufferRead(Angle,2,FRAM_TURN_ANGLE_ADDR);
  //检查参数是否合法
  if(read_flag)//校验通过
  {
    EmerCornerAngleMax=Angle[0];
    EmerCornerAngleMax=EmerCornerAngleMax<<8|Angle[1];
    if(Angle[0]==0&&Angle[1]==0)
    {
      ThreeEmerCheckDisable[2] = 1;
    }
  }
  else
  {
    EmerCornerAngleMax=15;
    L_Threshold[0]=0;L_Threshold[1]=15;
    FRAM_BufferWrite(FRAM_TURN_ANGLE_ADDR,L_Threshold,FRAM_TURN_ANGLE_LEN);
  }		
  
  Initflag = 1;
  //Emer_acce_check(0);
  //Emer_slowdwn_check(0);
}


void Emer_Turn_check(void)//急转弯判断
{
  u32 CornerAngle = 0;
  static u32 LastCornerAngle = 0;
  u8  Acc = 0;
  u8  RunFlag = 0;
  u8  Navigation = 0;
  u8  Speed;
	
  Acc = Io_ReadStatusBit(STATUS_BIT_ACC);
  Navigation = Io_ReadStatusBit(STATUS_BIT_NAVIGATION);
  RunFlag = SpeedMonitor_GetCurRunStatus();
  CornerAngle = Gps_ReadCourse();
  Get_Speed(&Speed);
  
  if((1 == Acc)&&(1 == Navigation)&&(1 == RunFlag))
  {
    if(CornerAngle >= LastCornerAngle)
    {
      if((CornerAngle - LastCornerAngle) >= 180)
      {
        if(((360+LastCornerAngle - CornerAngle) >= EmerCornerAngleMax)&&(Speed>EMER_TURN_ANGLE_SPEED))
        {
          Emer_Acce_Occur_Report(ENER_STATUS_SHARP_TRUN);
        }
        else
        {
          Emer_Acce_Clear_Report(ENER_STATUS_SHARP_TRUN);
        }
      }
      else
      {
        if(((CornerAngle - LastCornerAngle) >= EmerCornerAngleMax)&&(Speed>EMER_TURN_ANGLE_SPEED))
        { 
          Emer_Acce_Occur_Report(ENER_STATUS_SHARP_TRUN);
        }
        else
        {
          Emer_Acce_Clear_Report(ENER_STATUS_SHARP_TRUN);
        }
      }
      
    }
    else
    {
      if((LastCornerAngle - CornerAngle) >= 180)
      {
        if(((360+CornerAngle - LastCornerAngle) >= EmerCornerAngleMax)&&(Speed>EMER_TURN_ANGLE_SPEED))
        {
          Emer_Acce_Occur_Report(ENER_STATUS_SHARP_TRUN);
        }
        else
        {
          Emer_Acce_Clear_Report(ENER_STATUS_SHARP_TRUN);
        }
      }  
      else
      {
        if(((LastCornerAngle - CornerAngle) >= EmerCornerAngleMax)&&(Speed>EMER_TURN_ANGLE_SPEED))
        {
          Emer_Acce_Occur_Report(ENER_STATUS_SHARP_TRUN);
        }
        else
        {
          Emer_Acce_Clear_Report(ENER_STATUS_SHARP_TRUN);
        }
      }
    }
  }
  else
  {
    Emer_Acce_Clear_Report(ENER_STATUS_SHARP_TRUN);    
  }
  LastCornerAngle = CornerAngle;
}
/**
* @brief  位置信息附加信息 0xe9
* @param  指向数据地址
* @retval 返回数据长度
*/
u16  Acceleration_PosEx(u8 *pDst)
{
  u16 acc;
  acc = (u16)acce_flg;
  if((acc == 0))
  {
    return 0;
  }
  acc=acc<<8;
  *pDst++ = 0xE9;//附加信息ID
  *pDst++ = 2;//附加长度
  memcpy(pDst,&acc,2);
  return (4);
}
/**
* @brief 加速度判断任务
* @param  None
* @retval None
*/
FunctionalState Acceleration_TimeTask(void)
{
  if(ReadPeripheral2TypeBit(6))return DISABLE;
  //未接A2型OBD模块
  Acceleration_Init();//初始化
  if(0 == ThreeEmerCheckDisable[0])
  {
    Emer_acce_check(1);//急加速判断
  }
  else
  {
    Emer_Acce_Clear_Report(ENER_STATUS_SHARP_ACCELERATE);
  }
  if(0 == ThreeEmerCheckDisable[1])
  {
    Emer_slowdwn_check(1);//急减速判断
  }
  else
  {
    Emer_Acce_Clear_Report(ENER_STATUS_SHARP_SLOWDOWN);
  }
  if(0 == ThreeEmerCheckDisable[2])
  {
    Emer_Turn_check();//急转弯判断
  }
  else
  {
    Emer_Acce_Clear_Report(ENER_STATUS_SHARP_TRUN);
  }
  return ENABLE;
}
/*
********************************************************************************
*                 DEFIEN END
********************************************************************************
*/
#endif 


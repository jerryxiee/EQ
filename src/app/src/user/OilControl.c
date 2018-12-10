/*******************************************************************************
 * File Name:			OilControl.c 
 * Function Describe:	
 * Relate Module:		
 * Writer:				Joneming
 * Date:				2012-12-04
 * ReWriter:			
 * Date:				
 *******************************************************************************/
#include "include.h"
////////////////////////
/*******************************************************************************/
/*******************************************************************************/
#define CUT_OIL_SPEED       20	//切断油路速度，小于该速度时才可以切断油路,20km/h
//////////////////////////////////////
typedef enum
{
   OIL_OPEN =0,          //通油路,其值代表IO脚的电平
   OIL_CLOSE,            //断油路
   OIL_MAX               //
}OIL_STATUS;

typedef enum
{
    CTRL_OIL_EMPTY=0x55,       //空
    CTRL_OIL_OPEN,             //通油路
    CTRL_OIL_CLOSE,            //断油路    
    CTRL_OIL_MAX               //
}CTRL_OIL_STATUS;
typedef enum
{
    CTRLOIL_STEP_EMPTY,            //空
    CTRLOIL_STEP_OPEN,             //通油路
    CTRLOIL_STEP_CLOSE,            //断油路
    CTRLOIL_STEP_CHECK_OPEN,       //查检是否为通油路
    CTRLOIL_STEP_CHECK_CLOSE,      //查检是否为断油路
    CTRLOIL_STEP_MAX               //
}CTRLOIL_STEP;
///////////////////////////////////
static	unsigned char	s_ucOilControlFlag = CTRL_OIL_EMPTY;//油路打开标志,0x00:已通油路,0x01:已断油路
static 	unsigned char	s_ucOilControlStepFlag = CTRLOIL_STEP_EMPTY;//油路控制参数,0x00:通油路,0x01:断油路
extern  u8	SpeedFlag;//速度类型,dxl,2015.5.11,0x00:手动脉冲,0x01:手动GPS,0x02:自动脉冲,0x03:自动GPS
/*************************************************************OK
** 函数名称: OilControl_IoInit
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块:
*************************************************************/  
void OilControl_IoInit(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}  
/*******************************************************************************
**  函数名称  : OilControl_GetOilWayStatus
**  函数功能  : 
**  输    入  : 				
**  输    出  : 
**  全局变量  : 
**  调用函数  : 
**  中断资源  : 
**  备    注  :
*******************************************************************************/
unsigned char OilControl_GetOilWayStatus(void)
{
    return GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_2);   
}
/*******************************************************************************
**  函数名称  : OilControl_CtrlOilWay
**  函数功能  : 
**  输    入  : 				
**  输    出  : 
**  全局变量  : 
**  调用函数  : 
**  中断资源  : 
**  备    注  :
*******************************************************************************/
void OilControl_CtrlOilWay(unsigned char status)
{
    if(0 == status)//
    {
        GPIO_ResetBits(GPIOB, GPIO_Pin_2);//
    }
    else//
    {
        GPIO_SetBits(GPIOB, GPIO_Pin_2);//
    }
}
/*******************************************************************************
**  函数名称  : OilControl_ParameterInitialize
**  函数功能  : 断油路初始化
**  输    入  : 无				
**  输    出  : 
**  全局变量  : 
**  调用函数  : 
**  中断资源  : 
**  备    注  :
*******************************************************************************/
void OilControl_ParameterInitialize(void)
{
    OilControl_IoInit();
    s_ucOilControlStepFlag = CTRLOIL_STEP_EMPTY;
    FRAM_BufferRead((unsigned char *)&s_ucOilControlFlag,1,FRAM_OIL_CTRL_ADDR);
    if(CTRL_OIL_CLOSE == s_ucOilControlFlag)
    {
        s_ucOilControlFlag=CTRL_OIL_EMPTY;
        OilControl_OffOil();
    }
    else//其它的就通油
    {
        OilControl_OpenOil();
    }
}
/*******************************************************************************
**  函数名称  : OilControl_OpenOil
**  函数功能  : 通油
**  输    入  : 无				
**  输    出  : 
**  全局变量  : 
**  调用函数  : 
**  中断资源  : 
**  备    注  :
*******************************************************************************/
void OilControl_OpenOil(void)
{
    unsigned char temp;
    temp = CTRL_OIL_OPEN;
    FRAM_BufferWrite(FRAM_OIL_CTRL_ADDR,(unsigned char *)&temp,1);
    s_ucOilControlStepFlag =CTRLOIL_STEP_OPEN;
    SetTimerTask(TIME_OIL_CTRL, 2*SYSTICK_0p1SECOND);
}
/*******************************************************************************
**  函数名称  : OilControl_OffOil
**  函数功能  : 断油
**  输    入  : 无				
**  输    出  : 
**  全局变量  : 
**  调用函数  : 
**  中断资源  : 
**  备    注  :
*******************************************************************************/
void OilControl_OffOil(void)
{
    unsigned char temp;
    temp = CTRL_OIL_CLOSE;
    FRAM_BufferWrite(FRAM_OIL_CTRL_ADDR,(unsigned char *)&temp,1);
    s_ucOilControlStepFlag = CTRLOIL_STEP_CLOSE;
    SetTimerTask(TIME_OIL_CTRL, SYSTICK_1SECOND);
}
/*******************************************************************************
**  函数名称  : ControlOil_TimeTask
**  函数功能  : 油路控制任务调度接口函数
**  输    入  : 无				
**  输    出  : 
**  全局变量  : 
**  调用函数  : 
**  中断资源  : 
**  备    注  :
*******************************************************************************/
FunctionalState ControlOil_TimeTask(void)
{
    unsigned char OilCtrlFlag = 0;//油路控制标志
    unsigned char Speed;//当前速度
    unsigned char temp;//
    static  unsigned char   sucTryCtrlCnt = 0;
    static  unsigned char   sucCtrlOffStepCnt = 0; 

	
    if(CTRLOIL_STEP_CHECK_OPEN  == s_ucOilControlStepFlag)//通油
    {
        sucCtrlOffStepCnt =0;
        if(OIL_OPEN == OilControl_GetOilWayStatus())
        {
            sucTryCtrlCnt = 0;
        }
        else
        if(sucTryCtrlCnt<3)
        {
            sucTryCtrlCnt++;
            OilControl_CtrlOilWay(OIL_OPEN);//通油
            return ENABLE;
        }
    }
    else
    if(CTRLOIL_STEP_OPEN  == s_ucOilControlStepFlag)//通油
    {
        sucCtrlOffStepCnt =0;
        s_ucOilControlFlag = CTRL_OIL_OPEN;        
        FRAM_BufferWrite(FRAM_OIL_CTRL_ADDR,(unsigned char *)&s_ucOilControlFlag,1);
        /////////////////////////////////
        OilControl_CtrlOilWay(OIL_OPEN);//通油
        s_ucOilControlStepFlag =CTRLOIL_STEP_CHECK_OPEN;
        FRAM_BufferRead((unsigned char *)&OilCtrlFlag,1,FRAM_OIL_CTRL_ADDR);
        if(OilCtrlFlag!=CTRL_OIL_OPEN)
        {
            FRAM_BufferWrite(FRAM_OIL_CTRL_ADDR,(unsigned char *)&s_ucOilControlFlag,1);
        }
        //////////////////////////////////
        sucTryCtrlCnt =0;
        SetTimerTask(TIME_OIL_CTRL, 3*SYSTICK_1SECOND);
        ////////////////////
        return ENABLE;
    }    
    else
    if(CTRLOIL_STEP_CHECK_CLOSE  == s_ucOilControlStepFlag)//断油
    {
        sucCtrlOffStepCnt =0;
        if(OIL_OPEN != OilControl_GetOilWayStatus())
        {
            sucTryCtrlCnt = 0;
        }
        else
        if(sucTryCtrlCnt<3)
        {
            sucTryCtrlCnt++;
            OilControl_CtrlOilWay(OIL_CLOSE);//断油
            return ENABLE;
        }
    }
    else    
    if(CTRLOIL_STEP_CLOSE  == s_ucOilControlStepFlag)//断油
    {
        if(CTRL_OIL_CLOSE == s_ucOilControlFlag)//
        {
            s_ucOilControlStepFlag =CTRLOIL_STEP_CHECK_CLOSE;
            sucTryCtrlCnt =0;
            SetTimerTask(TIME_OIL_CTRL, 3*SYSTICK_1SECOND);
            return ENABLE;
        }
	    //获取速度选择
        if(0 == (SpeedFlag&0x01))
        {
	   Speed = Pulse_GetSpeed();
        }
        else
        {
	   Speed = Gps_ReadSpeed();
        }
	if(Speed > CUT_OIL_SPEED)//行驶速度大于20km/h时不能断油路
	{
	    return ENABLE;
	}
        //////////////////////////////
	//if(sucCtrlOffStepCnt<20)
        if(sucCtrlOffStepCnt<10)//dxl,2012.12.19,发5个脉冲,只有在速度低于
        {
            sucCtrlOffStepCnt++;
            if(sucCtrlOffStepCnt%2==1)
            {
                OilControl_CtrlOilWay(OIL_CLOSE);//断油
            }
            else
            {
                OilControl_CtrlOilWay(OIL_OPEN);//通油
            }
        }
        else
        {
            OilControl_CtrlOilWay(OIL_CLOSE);//断油
            sucCtrlOffStepCnt =0;
            s_ucOilControlFlag = CTRL_OIL_CLOSE;
            FRAM_BufferWrite(FRAM_OIL_CTRL_ADDR,(unsigned char *)&s_ucOilControlFlag,1);
            /////////////////////////////////
            OilControl_CtrlOilWay(OIL_CLOSE);//断油
            s_ucOilControlStepFlag = CTRLOIL_STEP_CHECK_CLOSE;
            FRAM_BufferRead((unsigned char *)&temp,1,FRAM_OIL_CTRL_ADDR);
            if(temp != CTRL_OIL_CLOSE)
            {
                FRAM_BufferWrite(FRAM_OIL_CTRL_ADDR,(unsigned char *)&s_ucOilControlFlag,1);
            }
            //////////////////////////////////
            sucTryCtrlCnt = 0;
            SetTimerTask(TIME_OIL_CTRL, 3*SYSTICK_1SECOND);
        }       
        /////////////////////
        return ENABLE;
    }
    //////////////////////////////
    sucTryCtrlCnt =0;
    sucCtrlOffStepCnt =0;
    temp =OilControl_GetOilWayStatus();
    if(OIL_OPEN==temp)
    {
        Io_WriteStatusBit(STATUS_BIT_OIL_CUT,RESET) ;     //油路通开
    }
    else
    {
        Io_WriteStatusBit(STATUS_BIT_OIL_CUT,SET) ;     //油路断开
    }
    ///////////////////////////////////
    s_ucOilControlStepFlag = CTRLOIL_STEP_MAX;
    return DISABLE;
}
/******************************************************************************
**                            End Of File
******************************************************************************/


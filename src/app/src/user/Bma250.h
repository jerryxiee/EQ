/*******************************************************************************
 * File Name:			BMA250.h 
 * Function Describe:	
 * Relate Module:		碰撞\侧翻功能
 * Writer:				Joneming
 * Date:				2014-07-16
 * ReWriter:			
 * Date:				
 *******************************************************************************/
#ifndef _BMA250_API_
#define _BMA250_API_

//初始化函数
void BMA250_ParamInitialize(void);
//调度线程
FunctionalState BMA250_TimeTask(void);
//更新碰撞参数
void BMA250_UpdataCrashAlarmParam(void);
//更新侧翻参数
void BMA250_UpdataRollOverAlarmParam(void);
/*************************************************************
** 函数名称: BMA250_RollOverInitAngleCalibration
** 功能描述: 侧翻初始角度校准
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char BMA250_RollOverInitAngleCalibration(void);
/*************************************************************
** 函数名称: BMA250_GetCurRunStatus
** 功能描述: 获取当前车辆的状态(行驶或停止)
** 入口参数: 
** 出口参数: 
** 返回参数: 1为行驶，0为停止
** 全局变量: 无
** 调用模块:
*************************************************************/
unsigned char BMA250_GetCurRunStatus(void);
#endif

/*******************************************************************************
 *                             end of module
 *******************************************************************************/
 
/*
********************************************************************************
*
*                                                              
*
* Filename      : JTOther.c
* Version       : V1.00
* Programmer(s) : miaoyahan
* @date         : 2014-11-08  
* @note         : 九通需求,随九通开关编译
* @ver -> V1.00 : ->启动后侦测外部传感器,有效后关闭任务;
              
*
********************************************************************************
*/

/*
********************************************************************************
*                               INCLUDES
********************************************************************************
*/
#ifdef EYE_MODEM
#include "include.h"
#endif

/*
********************************************************************************
*                               DEFIEN START
********************************************************************************
*/

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
// 0:近光 1:远光 2:右转 3:左转 4:刹车 5:ACC 6:手刹 7:车门 8:速度
static u8  JT_IoSta[9];
static u8  JT_IoScn[9];
static u8  JT_IoTm;//IO侦测时间

const u8 JT_IoStaHz[][10]=
{
"近光:无效 ",
"远光:无效 ",
"右转:无效 ",
"左转:无效 ",
"刹车:无效 ",
"ACC :无效 ",
"手刹:无效 ",
"车门:无效 ",
"速度:无效 ",
};

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

/**
  * @brief  初始化
  * @param  None
  * @retval NOne
  */
void JTOTher_Init(void)
{
    static u8 InitFlg;
    
    if(InitFlg)
    {
        return;
    }
    InitFlg = 1;    
    
}
/**
  * @brief  IO侦测,每两秒侦测一次
  * @param  None
  * @retval NOne
  */
void JTOTher_IoSense(void)
{
    u8 iocnt=0;
    u8 spdtype=0;

    if(SpeedMonitor_GetCurSpeedType() == 0)
    {
        spdtype = 1;
    }

    if(++JT_IoTm < 2)
    {
        return;
    }

    JT_IoScn[iocnt++] = Io_ReadExtCarStatusBit(CAR_STATUS_BIT_NEAR_LIGHT);//近光
    JT_IoScn[iocnt++] = Io_ReadExtCarStatusBit(CAR_STATUS_BIT_FAR_LIGHT);//远光
    JT_IoScn[iocnt++] = Io_ReadExtCarStatusBit(CAR_STATUS_BIT_RIGHT_LIGHT);//右转
    JT_IoScn[iocnt++] = Io_ReadExtCarStatusBit(CAR_STATUS_BIT_LEFT_LIGHT);//左转 
    JT_IoScn[iocnt++] = Io_ReadExtCarStatusBit(CAR_STATUS_BIT_BRAKE);//制动 
    JT_IoScn[iocnt++] = Io_ReadStatusBit(STATUS_BIT_ACC);//ACC
    JT_IoScn[iocnt++] = 0;//手刹    
    JT_IoScn[iocnt++] = Io_ReadStatusBit(STATUS_BIT_DOOR1);//车门
    JT_IoScn[iocnt++] = spdtype;//速度

    for(iocnt=0;iocnt<sizeof(JT_IoSta);iocnt++)
    {
        if(JT_IoSta[iocnt] == 0)
        {
            JT_IoSta[iocnt] = JT_IoScn[iocnt];
        }
    }
}

/**
  * @brief  车辆IO状态,返回9字节的状态
  *         0:近光 1:远光 2:右转 3:左转 4:刹车 5:ACC 6:手刹 7:车门 8:速度
  * @param  None
  * @retval NOne
  */
void JTOTher_IoSta(u8 *pData)
{
    memcpy(pData,JT_IoSta,sizeof(JT_IoSta));
}
/**
  * @brief  车辆IO状态,汉子形式数据,需90个字节
  * @param  None
  * @retval 返回长度
  */
u8 JTOTher_IoStaHz(u8 *pData)
{
    u8 i;
    u8 ionum = sizeof(JT_IoSta);
    u8 hzlen = sizeof(JT_IoStaHz[0]);
    
    for(i=0;i<ionum;i++)
    {
        memcpy(pData+i*hzlen,(u8*)&JT_IoStaHz[i][0],hzlen);
        if(JT_IoSta[i])
        {
            memcpy(pData+i*hzlen+5,"有效",4);
        }
    }

    return 90;
}

/**
  * @brief  位置信息中的标准状态,后8位九通专用
  * @param  None
  * @retval 返回刷新后的状态
  */
u32 JTOTher_IoReport(u32 sta)
{
    u32 jtio=0;
    u8  spdtype=0;

    if(SpeedMonitor_GetCurSpeedType() == 0)
    {
        spdtype = 1;
    }

    jtio = (jtio<<1) | Io_ReadExtCarStatusBit(CAR_STATUS_BIT_BRAKE); 
    jtio = (jtio<<1) | Io_ReadExtCarStatusBit(CAR_STATUS_BIT_LEFT_LIGHT); 
    jtio = (jtio<<1) | Io_ReadExtCarStatusBit(CAR_STATUS_BIT_RIGHT_LIGHT);  
    jtio = (jtio<<1) | Io_ReadExtCarStatusBit(CAR_STATUS_BIT_FAR_LIGHT);
    jtio = (jtio<<1) | Io_ReadExtCarStatusBit(CAR_STATUS_BIT_NEAR_LIGHT); 
    jtio = (jtio<<1) | spdtype;//脉冲开关 
    jtio = (jtio<<1) | 0;//倒车  
    jtio = (jtio<<1) | 0;//报警
    jtio = (jtio<<1) | 0;//手刹

    sta |= (jtio<<23);

    return sta;
}
/**
  * @brief  九通需求任务,每秒调用一次
  * @param  None
  * @retval 返回任务状态
  */
FunctionalState JTOTher_TimeTask(void)
{
    JTOTher_Init();

    JTOTher_IoSense();//IO侦测

    return ENABLE;
}

/*
********************************************************************************
*                 DEFIEN END
********************************************************************************
*/ 

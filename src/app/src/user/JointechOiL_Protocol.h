#ifndef _JOINTECHOIL_H_
#define _JOINTECHOIL_H_




#define DAMP_DEFAULT 60 //默认的阻尼值 , 单位:秒  取值范围5~600

/***************   枚举类型 定义区   ********************************/


typedef enum{

 STAGE_JOINTECHOIL_NULL = 0 , 
 STAGE_JOINTECHOIL_E_OILCURRENT ,//读取当前油量信息
 STAGE_JOINTECHOIL_H_DAMP , //查询/设置阻尼
 STAGE_JOINTECHOIL_I_TEMPERATURE_COMPENSATION , //设置是否开启温标补偿
 STAGE_JOINTECHOIL_R_CRITICALPARAMETERS , //读关键参数
 STAGE_JOINTECHOIL_MAX //久通机电油位传感器任务 状态机的上限
} ENUM_STAGE_JOINTECHOIL ;


typedef enum{

 TASK_JOINTECHOIL_MAIN = 0 , //主任务
 TASK_JOINTECHOIL_SUB , //辅任务
 TASK_JOINTECHOIL_MAX //
} ENUM_TASK_JOINTECHOIL ;

/***************   结构体 定义区   ********************************/

/* 深圳久通油位传感器 JT606 协议 结构体 */

#define MAX_JOINTECH_DATA_LEN 40

typedef struct tagSTT_JOINTECH_OILCOST_PROTOCOL{
u8 id  ;
u8 cmd ;
u8 len ;
u8 data[ MAX_JOINTECH_DATA_LEN ];
u8 verify ;
}STT_JOINTECH_OILCOST_PROTOCOL;





/* 深圳久通油位传感器 JT606 所用COM控制结构体  */

#define MAX_COM_BUF_LEN  40
typedef struct tagSTT_JOINTECH_COM{
u16 buflen ;
u8  buf[ MAX_COM_BUF_LEN ];
}STT_JOINTECH_COM;




/* 深圳久通油位传感器 JT606 数据量结构体 */

typedef struct tagSTT_OILCOST{
u8  ID ;  //当前油位传感器的编号
u8  iBaud; //通信波特率 选择序号
u16 TankVolume ; //当前油箱体积 分辨率: 1L 
u16 TankZero; //当前的调零值
u16 TankFull; //当前的调满值
u16 LiquidLevelPercent ; //当前液位高度万分比，最大值为10000;
u16 OilVolumeCur; //当前的油
u16 OilVolumeAdjust ; //当前经过调后的油位值 , 0~4095

u8 iSetup ; //安装序号
char Version[10]; //版本号
u16 Damp; //阻尼 单位: 秒 范围是5~600

u8  Temperature;         //温度值 ，分辨率: 1Cel 偏移: -40Cel
u8  IsTempCompensation ; //是否开启温飘补偿 0:不开启 1: 开启

u16 SampleDamp; //阻尼 单位: 秒 范围是5~600
u16 SampleFrequence ; //采集频率值 , 分辨率: 0.1kHz
u16 SampleTankZero ; //采集到的零值
u16 SampleTankFull ; //采集到的满值

u8  FlagErrorFrequency ; //JT606的输出情况  0:正常  1:出错 
u8  FlagErrorTankZero ; //零值情况  0:正常  1:出错 
u8  FlagErrorTankFull ; //满值情况  0:正常  1:出错 

}STT_JOINTECH_OILCOST ; 




/* 深圳久通油位传感器 JT606 任务状态机控制结构体 */

typedef struct
{
  u8 StageMain;  //主状态
  u8 StageSub ;  //次状态
  u8 IsExist;    //是否存在
  u8 ReCheckCnt ; //重发计数器
  u8 IsAckOK;   //是否收到正确的应答
  u8 ErrorCnt ; //统计任务错误的总次数
  u8 ErrorFlag ; //错误标志
}STT_JOINTECH_OILCOST_CTRL ;


/***************   外部函数 声明区 ********************************/

extern FunctionalState JointechOiLCost_TimeTask( );


/********************************************************************
* 名称 : JointechOiLCost_CopyBuf
* 功能 : 从串口接收Buf里拷贝数据
* 说明  
********************************************************************/
extern void  JointechOiLCost_CopyBuf( u8 * pInBuf, u16 InLength );

/*********************************************************************
*名称 : JointechOiLCost_OilVolumeCur_Get
*功能 : 获取油箱内当前剩余油量值，
*备注 : 分辨率: 0.01升/位 ,偏移量: 0
*********************************************************************/
extern u16 JointechOiLCost_OilVolumeCur_Get( void );


/*********************************************************************
*名称 : JointechOiLCost_OilVolumeCur_Get
*功能 : 获取油箱内当前液位高度万分比， 最大值为10000，
*备注 : 分辨率: 0.01% ,偏移量: 0
*********************************************************************/
extern u16 JointechOiLCost_OilLevelPercentCur_Get( void );

/*********************************************************************
*名称 : JointechOiLCost_COM_Buf_Write
*功能 : 深圳久通机电油位传感器 COM 批量写入字节流
*备注 :
*********************************************************************/
void JointechOiLCost_COM_Buf_Write(  u8 * pInBuf , u16 InLength );



/********************************************************************
* 名称 : JointechOiLCost_Error_Set
* 功能 : 久通机电油位传感器故障标志 置位
* 说明  
********************************************************************/
void  JointechOiLCost_Error_Set( void );


/********************************************************************
* 名称 : JointechOiLCost_Error_Clr
* 功能 : 久通机电油位传感器故障标志 清除
* 说明  
********************************************************************/
void  JointechOiLCost_Error_Clr( void );


/********************************************************************
* 名称 : JointechOiLCost_Error_Set
* 功能 : 久通机电油位传感器故障标志 置位
* 说明  
********************************************************************/
u8  JointechOiLCost_Error_Get( void );



/********************************************************************
* 名称 : JointechOiLCost_Error_Set
* 功能 : 久通机电油位传感器 在线
* 说明  
********************************************************************/
void  JointechOiLCost_Online_Set( void );



/********************************************************************
* 名称 : JointechOiLCost_Error_Clr
* 功能 : 久通机电油位传感器  离线
* 说明  
********************************************************************/
void  JointechOiLCost_Online_Clr( void );

/********************************************************************
* 名称 : JointechOiLCost_Error_Set
* 功能 : 查询久通机电油位传感器是否 在线
* 说明  
********************************************************************/
u8  JointechOiLCost_Online_Get( void );

/*********************************************************************
//函数名称	:JointechOiLCost_OilProtocolCheck
//功能		:久通油耗协议检查,正确的话会调用函数JointechOiLCost_CopyBuf,
//          :把油耗指令拷贝到g_sttJointechComRx.buf中，然后解析
//备注		:,dxl,2014.5.12增加,油耗由32PIN 485接口改到16PIN 232接口
//          :返回值为1表示已判定为油耗指令，为0表示不是油耗指令
*********************************************************************/
u8 JointechOiLCost_OilProtocolCheck(u8 *pBuffer, u16 BufferLen);

/*******************************************************************************
** 函数名称: JointechOiLCost_GetSubjoinInfoCurOilVolume
** 功能描述: 取得当前油量附加信息包(包括附加信息ID、长度、附加信息体)
** 入口参数: 需要保存油量附加信息包的首地址
** 出口参数: 
** 返回参数: 油量附加信息包的总长度
*******************************************************************************/
unsigned char JointechOiLCost_GetSubjoinInfoCurOilVolume(unsigned char *data);
/*******************************************************************************
** 函数名称: JointechOiLCost_GetSubjoinInfoCurOilLevelPercent
** 功能描述: 取得当前油量百分比附加信息包(包括附加信息ID、长度、附加信息体)
** 入口参数: 需要保存油量百分比附加信息包的首地址
** 出口参数: 
** 返回参数: 油量百分比附加信息包的总长度
*******************************************************************************/
unsigned char JointechOiLCost_GetSubjoinInfoCurOilLevelPercent(unsigned char *data);

#endif

/*******************************************************************************
 * File Name:			BMA250.h 
 * Function Describe:	
 * Relate Module:		碰撞\侧翻功能
 * Writer:				Joneming
 * Date:				2014-07-16
 * ReWriter:			
 * Date:				
 *******************************************************************************/
//碰撞传感器BMA250E
//本硬件设计采取IIC模式进行通讯，所以驱动上必须保持选择IIC模式
#include"include.h"
//////////////////////////
#define USE_ACC_RANGE   ACC_RANGE_4G//当前使用的量程
////////////////////////
////////////////////////////////////////
//加速度范围。
#define ACC_RANGE_2G		 3
#define ACC_RANGE_4G		 5
#define ACC_RANGE_8G		 8
#define ACC_RANGE_16G		12

//滤波范围频率 HZ。 
#define FILTER_1000HZ	    	15
#define FILTER_500HZ		    14
#define FILTER_250HZ		    13
#define FILTER_125HZ		    12
#define FILTER_62HZ             11
#define FILTER_31HZ             10
#define FILTER_15HZ             9
#define FILTER_7HZ              8																		 

//使能X,Y,Z轴。
#define EN_HIGH_X       1
#define EN_HIGH_Y		2
#define EN_HIGH_Z		4

//加速度传感器BMA250E的内部寄存器分布。
#define BMA250_DEVICE_ID             0x00  //器件ID。
#define BMA250_ACCEL_X_L             0x02  
#define BMA250_ACCEL_X_H             0x03  //X轴加速度。
#define BMA250_ACCEL_Y_L             0x04
#define BMA250_ACCEL_Y_H             0x05  //Y轴加速度。
#define BMA250_ACCEL_Z_L             0x06  
#define BMA250_ACCEL_Z_H             0x07  //Z轴加速度。**_L>>6+**_H<<2
#define BMA250_TEMP                  0x08  //TEMP值。
#define BMA250_FOTS_INT              0x09  //BMA250E的平坦、方向、抖动和斜率中断。
#define BMA250_DATA_FIFO_INT         0x0A  //BMA250E的数据、FIFO中断。
#define BMA250_TAP_SLOPE_INT         0x0B  //BMA250E的抖动和斜率。
#define BMA250_FLAT_ORI_HIGH_INT     0x0C  //BMA250E的平坦、方向和XYZ的定义。
#define BMA250_FIFO                  0x0E  //BMA250E的FIFO定义。
#define BMA250_ACC_RANGE             0x0F  //BMA250E的ACCELERATION 范围。
#define BMA250_FILTER_BW             0x10  //BMA250E的滤波器带宽。
#define BMA250_POWER_SET             0x11  //BMA250E的电源工作模式设置。
#define BMA250_POWER_MODE            0x12  //BMA250E的电源工作模式。
#define BMA250_DATASHADOW            0x13  //BMA250E的加速度数据存储方式（shadow将8bit和2bit组合成一个10bit的数据）。
#define BMA250_SOFTRESET             0x14  //BMA250E的软件复位。
#define BMA250_FOTS_EN               0x16  //BMA250E的平坦、方向、抖动和斜率的使能。
#define BMA250_DATA_FIFO_EN          0x17  //BMA250E的FIFO、数据和XYZ的使能。
#define BMA250_SLOPE_EN              0x18  //BMA250E的斜率检测的使能。
#define BMA250_FOTS_INT1             0x19  //BMA250E的平坦、方向、抖动和斜率中断1。
#define BMA250_DATA_FIFO_INT1_2      0x1A  //BMA250E的数据和FIFO的中断1和2。
#define BMA250_FOTS_INT2             0x1B  //BMA250E的平坦、方向、抖动和斜率中断2。
#define BMA250_SRC                   0x1E  //BMA250E的SRC中断。
#define BMA250_OD_v_INT              0x20  //BMA250E的OD和|v|中断。
#define BMA250_RESET_LATCH_INT       0x21  //BMA250E的复位和锁存中断。
#define BMA250_LOW_DUR               0x22  //BMA250E的LOW的间隔。
#define BMA250_LOW_TH                0x23  //BMA250E的LOW的阈值。
#define BMA250_HL_HY                 0x24  //BMA250E的HIGH和LOW的HY。
#define BMA250_HIGH_DUR              0x25  //BMA250E的HIGH的间隔。
#define BMA250_HIGH_TH               0x26  //BMA250E的HIGH的阈值。
#define BMA250_SLOPE_NOMOTION_DUR    0x27  //BMA250E的SLOPE的间隔。
#define BMA250_SLOPE_TH              0x28  //BMA250E的SLOPE的阈值。
#define BMA250_SLOPE_NOMOTION_TH     0x29  //BMA250E的SLOPE_NOMOTION的阈值。
#define BMA250_TAP_DUR               0x2A  //BMA250E的TAP的静止、震动和间隔。
#define BMA250_TAP_TH                0x2B  //BMA250E的TAP的抽样和阈值。
#define BMA250_ORI_HBM               0x2C  //BMA250E的方向的HY、阻塞锁存和模式。
#define BMA250_ORI_EN_THETA          0x2D  //BMA250E的方向的使能及theta角。
#define BMA250_FLAT_THETA            0x2E  //BMA250E的平坦的theta角。
#define BMA250_FLAT_HT_HY            0x2F  //BMA250E的平坦的保持时间及HY。
#define BMA250_FIFO_WMARK            0x30  //BMA250E的FIFO的水印。
#define BMA250_SELFTEST_SET          0x32  //BMA250E的自测设置。
#define BMA250_NVM_SET               0x33  //BMA250E的NVM设置。
#define BMA250_IICSPI_WDT_SET        0x34  //BMA250E的IIC和SPI设置。
#define BMA250_OFFSET_CAL_HP         0x36  //BMA250E的偏移、计算及XYZ使能。
#define BMA250_OFFSET_TARGET_XYZ     0x37  //BMA250E的XYZ的OFFSET TARGET的设置。
#define BMA250_OFFSET_X_SET		       0x38  //BMA250E的X的OFFSET设置。
#define BMA250_OFFSET_Y_SET		       0x39  //BMA250E的Y的OFFSET设置。
#define BMA250_OFFSET_Z_SET		       0x3A  //BMA250E的Z的OFFSET设置。    
#define BMA250_GP0                   0x3B  //BMA250E的GPO设置。
#define BMA250_GP1          		     0x3C  //BMA250E的GP1设置。
#define BMA250_FIFO_MODE_DATA        0x3E  //BMA250E的FIFO的模式及数据选择。
#define BMA250_FIFO_DATAREG          0x3F  //BMA250E的FIFO的数据输出寄存器。
////////////////////////////////
////////////////////////////
#define BMA250_G_SENSOR_ADDR       (0x18<<1)      //BMA250地址
////////////////////////
enum 
{
    BMA_TYPE_X,                 
    BMA_TYPE_Y,               
    BMA_TYPE_Z,
    BMA_TYPE_MAX
}E_BMA_TYPE;
////////////////////////
typedef struct 
{
    signed short angle[BMA_TYPE_MAX];	  //角度    
    unsigned char saveflag;	  //
    unsigned char bak;//
}ST_BMA250_INIT;

typedef struct 
{
    float angle[BMA_TYPE_MAX];//角度
    float anglelimit;//角度
    float gravity;//1G对应的值
    unsigned char status;	  //状态
    unsigned char setCnt;
    unsigned char resetCnt;
    unsigned char range;	  //
}ST_BMA250_ATTRIB;

static ST_BMA250_ATTRIB s_stBmaAttrib;
///////////////////////////////////
#define BMA_PI              3.14159265358979
#define BMA_2_PI            6.28318530717958

const unsigned short c_usGravity[]={256,128,64,32};

#define BMA250_MIN(val,tmp) (val>tmp)?tmp:val
#define BMA250_MAX(val,tmp) (val>tmp)?val:tmp

//////////////////////////////
//读取中断标志 （how many interrupts in total)
unsigned char Bma250_GetInterruptFlag_A(void);
unsigned char Bma250_GetInterruptFlag_B(void);
unsigned char Bma250_GetInterruptFlag_C(void);
unsigned char Bma250_GetInterruptFlag_D(void);
////////////////////////////
static void (*BMA250_WriteData)(unsigned char addr,unsigned char value);//
static unsigned char (*BMA250_ReadData)(unsigned char addr);//
static unsigned char (*BMA250_GetStatus)(void);//
////////////////////////

///////////////////////////////////
/*************************************************************
** 函数名称: BMA250_I2C_WriteData
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void BMA250_I2C_WriteData(unsigned char addr,unsigned char value)
{
    I2C_WriteDat(addr,value);
}
/*************************************************************
** 函数名称: BMA250_I2C_ReadData
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char BMA250_I2C_ReadData(unsigned char addr)
{
    unsigned char temp;    
    I2C_ReadDat(addr,&temp);    
    return temp;
}

/*************************************************************
** 函数名称: BMA250_E2_WriteData
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void BMA250_E2_WriteData(unsigned char addr,unsigned char value)
{
    s_stBmaAttrib.status = E2_WriteOneByte(value, BMA250_G_SENSOR_ADDR, addr);    
}
/*************************************************************
** 函数名称: BMA250_E2_ReadData
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char BMA250_E2_ReadData(unsigned char addr)
{
    unsigned char temp;
    s_stBmaAttrib.status = E2_ReadOneByte(&temp,BMA250_G_SENSOR_ADDR,addr); 
    return temp;
}
/*************************************************************
** 函数名称: Bma250_GetInterruptFlag_D
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char BMA250_E2_GetStatus(void)
{
    return (s_stBmaAttrib.status)?0:1;
}
/*************************************************************
** 函数名称: BMA250_Delay
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void BMA250_Delay(unsigned short m)
{
    unsigned short i;
    for(i=0;i<m;i++);
}

/*************************************************************
** 函数名称: BMA250_ReadAccelValue
** 功能描述: X/Y/Z轴加速度值
            根据加速度量程设置范围：+-2G/+-4G/+-8G/+-16G,对应的Gravity_Acc_1G为16LSB/8LSB/4LSB/2LSB。
            读取加速度值
            0-->+31;0-->+2G
            0-->-32;0-->-2G
            加速度寄存器地址分配：ACC_X-->0x02(L2),0x03(H8)；将H8放到16bits的L8上(包含有符号位)，接着对16bits左移两位，然后将16bits数据与L2相加，这样就得到了10bits的X轴加速度值。
            ACC_Y-->0x04(L2),0x05(H8)；将H8放到16bits的L8上(包含有符号位)，接着对16bits左移两位，然后将16bits数据与L2相加，这样就得到了10bits的Y轴加速度值。
            ACC_Z-->0x06(L2),0x07(H8)；将H8放到16bits的L8上(包含有符号位)，接着对16bits左移两位，然后将16bits数据与L2相加，这样就得到了10bits的Z轴加速度值
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
float BMA250_ReadAccelValue(unsigned char type)
{
    unsigned char data[2];
    short sval;					  //定义两个整数型变量temp1和temp2
    unsigned char accel_L,accel_H;
    if(BMA_TYPE_X==type)
    {
        accel_L=BMA250_ACCEL_X_L;
        accel_H=BMA250_ACCEL_X_H;
    }
    else
    if(BMA_TYPE_Y==type)
    {
        accel_L=BMA250_ACCEL_Y_L;
        accel_H=BMA250_ACCEL_Y_H;
    }
    else
    {
        accel_L=BMA250_ACCEL_Z_L;
        accel_H=BMA250_ACCEL_Z_H;
    }
    data[0]=BMA250_ReadData(accel_L);            //读取X轴加速度值的低两位
    data[1]=BMA250_ReadData(accel_H);			  //读取X轴加速度值的高八位
    sval = data[0]>>6;//将X轴加速度值的低两位右移六位
    sval |=data[1]<<2;//将X轴加速度值的高八位左移两位
    sval = sval << 6;//变成有符号
    sval = sval >> 6;//变成有符号
    return (float)sval;									  //返回处理后的X轴加速度值
}

/*************************************************************
** 函数名称: Bma250_GetInterruptFlag_D
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Bma250_GetInterruptFlag_A(void)
{
    unsigned char temp;										          //定义变量temp
    temp=BMA250_ReadData(BMA250_FOTS_INT);	//读取中断事件A的标志位
    return temp;										          //返回中断事件A的标志位
}
/*************************************************************
** 函数名称: Bma250_GetInterruptFlag_D
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Bma250_GetInterruptFlag_B(void)
{
    unsigned char temp;										          //定义变量temp
    temp=BMA250_ReadData(BMA250_DATA_FIFO_INT);	//读取中断事件B的标志位
    return temp;										              //返回中断事件B的标志位
}
/*************************************************************
** 函数名称: Bma250_GetInterruptFlag_D
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Bma250_GetInterruptFlag_C(void)
{
    unsigned char temp;										          //定义变量temp
    temp=BMA250_ReadData(BMA250_TAP_SLOPE_INT); //读取中断事件C的标志位
    return temp;										              //返回中断事件C的标志位
}
/*************************************************************
** 函数名称: Bma250_GetInterruptFlag_D
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char Bma250_GetInterruptFlag_D(void)
{
    unsigned char temp;										             //定义变量temp
    temp=BMA250_ReadData(BMA250_FLAT_ORI_HIGH_INT);//读取中断事件D的标志位
    return temp;										                 //返回中断事件D的标志位
}

/*************************************************************
** 函数名称: BMA250_ReadAndSetCrashAlarmParam
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void BMA250_ReadAndSetCrashAlarmParam(void)                                      
{
    unsigned char buf[4];
    unsigned char temp;
    unsigned char crashTime,accelerate;
    float fval;    
    if(EepromPram_ReadPram(E2_SET_CRASH_ALARM_ID,buf))
    {
        crashTime = buf[1];
        accelerate = buf[0];
        if(crashTime==0||crashTime>8)
        {
            crashTime=4;
        }
        ////////////////////
        if(accelerate==0||accelerate>s_stBmaAttrib.range)
        {
            accelerate = s_stBmaAttrib.range;            //加速度量程选择了+-4G,'20'表示20*0.1(2),即4G的一半。
        }
    }
    else
    {
        crashTime = 4;
        accelerate = s_stBmaAttrib.range;
    }
    //----------------------------碰撞报警参数设置--------------------------------------------
    temp=(crashTime/2)-1;                 //((crashTime*4)/8)-1;     '4'表示平台设置该参数的LSB,'8'表示BMA250E选择BW=62HZ后SLOPE_DURATION的LSB,'-1'是由DURATION的计算机制决定的。
    BMA250_WriteData(BMA250_SLOPE_NOMOTION_DUR,temp);//设置时间间隔为temp个间隔点

    fval=accelerate;//单位0.1g
    fval=fval/10.0;//有多少1g
    fval=fval*s_stBmaAttrib.gravity;//对应的数据
    temp=(unsigned char)(fval-1); 
    BMA250_WriteData(BMA250_SLOPE_TH,temp);         //设置阈值为
}

/*************************************************************
** 函数名称: BMA250_ReadAndSetRollOverAlarmParam
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void BMA250_ReadAndSetRollOverAlarmParam(void)                                      
{
    unsigned char buf[5];
    unsigned short angle,i;
    ST_BMA250_INIT stBmaInit;
    if(EepromPram_ReadPram(E2_SET_ROLLOVER_ALARM_ID,buf))
    {
        angle=Public_ConvertBufferToShort(buf);
        if(angle==0||angle>180)
        {
            angle=30;
        }
    }
    else
    {
        angle = 30;
    }
    //////////////
    Public_ReadDataFromFRAM(FRAM_ROLL_OVER_INIT_ANGLE_ADDR, (unsigned char *)&stBmaInit, sizeof(ST_BMA250_INIT));
    if(0x55!=stBmaInit.saveflag)
    {
        memset(&stBmaInit,0,sizeof(ST_BMA250_INIT));
        stBmaInit.angle[0] = (signed short)(BMA_PI*5000.0);
        stBmaInit.angle[1] = (signed short)(BMA_PI*5000.0);
        stBmaInit.angle[2] = 0; //       
    }
    /////////////////////
    s_stBmaAttrib.anglelimit = angle/180.0*BMA_PI;
    ////////////////////
    for(i = BMA_TYPE_X;i<= BMA_TYPE_Z; i++)
    {
        s_stBmaAttrib.angle[i] = stBmaInit.angle[i]/10000.0;//基准角
    }
    //////////////////////
}
/*************************************************************
** 函数名称: BMA250_RollOverInitAngleCalibration
** 功能描述: 侧翻初始角度校准
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
signed short BMA250_GetAngleCalibration(unsigned char type)
{
    float temp,min,max;
    float sum;
    unsigned char i;
    sum = 0.0 ;
    min = 512;
    max =-512;
    for(i=0; i<10; i++)
    {
        temp = BMA250_ReadAccelValue(type); //读取Z轴加速度值
        min = BMA250_MIN(min,temp);
        max = BMA250_MAX(max,temp);
        sum += temp; //读取Z轴加速度值        
    }  
    sum=sum-max-min;
    ///////
    temp=(sum/8.0);
    /////////////
    temp =temp/s_stBmaAttrib.gravity;
    if(temp>1.0)
        temp = 1.0;
    else 
    if(temp<-1.0)
        temp = -1.0;
    
    return (signed short)(acos(temp)*10000.0);
}
/*************************************************************
** 函数名称: BMA250_RollOverInitAngleCalibration
** 功能描述: 侧翻初始角度校准
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
unsigned char BMA250_RollOverInitAngleCalibration(void)
{
    unsigned char i;
    ST_BMA250_INIT stBmaInit;
    ///////////////
    BMA250_ReadAccelValue(BMA_TYPE_Z); //读取Z轴加速度值
    ///////////////////
    if(BMA250_GetStatus())
    {
        return 0;
    }
    /////////////////
    memset(&stBmaInit,0,sizeof(ST_BMA250_INIT));
    //////////////////
    for(i = BMA_TYPE_X; i <= BMA_TYPE_Z; i++)
    {
        stBmaInit.angle[i] = BMA250_GetAngleCalibration(i);
    }
    ////////////////////
    stBmaInit.saveflag = 0x55;
    //////////////////
    Public_WriteDataToFRAM(FRAM_ROLL_OVER_INIT_ANGLE_ADDR, (unsigned char *)&stBmaInit, sizeof(ST_BMA250_INIT));
    ////////////////
    BMA250_ReadAndSetRollOverAlarmParam();
    return 1;
}
/*************************************************************
** 函数名称: BMA250_GetAccRange
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void BMA250_GetAccRange(void)
{
    unsigned char val,index,range;
    val=BMA250_ReadData(BMA250_ACC_RANGE);	//选择加速度范围为+-4G,另可选择+-2G、+-8G和+-16G。
    if(ACC_RANGE_16G == val)
    {
        index = 3;
        range = 8;//
    }
    else
    if(ACC_RANGE_8G==val)
    {
        index = 2;
        range = 4;//
    }
    else
    if(ACC_RANGE_4G==val)
    {
        index = 1;
        range = 2;//
    }
    else
    {
        index = 0;
        range = 1;//
    }

    s_stBmaAttrib.gravity = (float)c_usGravity[index];
    s_stBmaAttrib.range = (range *10);//单位0.1g
}
/*************************************************************
** 函数名称: BMA250_UpdataSetParam
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void BMA250_UpdataSetParam(void)
{    
    BMA250_WriteData(BMA250_SOFTRESET,0xB6);
    BMA250_Delay(1000);
    BMA250_WriteData(BMA250_OFFSET_CAL_HP,0x80);		//设置漂移补偿目标值。
    BMA250_WriteData(BMA250_IICSPI_WDT_SET,6);        //使能看门狗定时器，周期为50ms。（4   周期为1ms）
    BMA250_WriteData(BMA250_ACC_RANGE,USE_ACC_RANGE);	//选择加速度范围为+-4G,另可选择+-2G、+-8G和+-16G。
    BMA250_WriteData(BMA250_FILTER_BW,FILTER_62HZ);   //配置滤波器带宽：62Hz    
    BMA250_WriteData(BMA250_FOTS_EN,0xf7); 			//使能XYZ三个方向的加速度检测,slopexyz_en(SET)。
    BMA250_WriteData(BMA250_DATA_FIFO_EN,0x07);		//使能XYZ三个方向的HIGH-G模式。
    //BMA250_WriteData(BMA250_OFFSET_CAL_HP,0x60);		//设置漂移补偿目标值。//碰撞
    //BMA250_WriteData(BMA250_OFFSET_TARGET_XYZ,0x20);	//设置Z轴为补偿目标。
    BMA250_WriteData(BMA250_RESET_LATCH_INT,0x80); 	//使能复位中断。
    BMA250_WriteData(BMA250_OFFSET_Z_SET,0x00);		//使能平坦、方向、抖动和倾斜中断
    //BMA250_WriteData(BMA250_NVM_SET,0x0B);
    //BMA250_WriteData(BMA250_HL_HY,0x80);				//设置high_hy为2。
    /////////////////////////
    BMA250_GetAccRange();
    //////////////////////
    BMA250_ReadAndSetCrashAlarmParam();
    ///////////////
    BMA250_ReadAndSetRollOverAlarmParam();    
}
/*************************************************************
** 函数名称: BMA250_ParamInitialize
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void BMA250_ParamInitialize(void)
{
    ///////////////////
    memset(&s_stBmaAttrib,0,sizeof(s_stBmaAttrib));
    ////////////////////
    if(ICCardBase_GetUseIntFlag())
    {
        BMA250_WriteData = BMA250_E2_WriteData;
        BMA250_ReadData  = BMA250_E2_ReadData;
        BMA250_GetStatus = BMA250_E2_GetStatus;
        if(ICCARD_M3_I2C_CheckDevice( BMA250_G_SENSOR_ADDR ) == 0)
        {
            BMA250_Delay(20000);
        }
        ///////////////
        if(ICCARD_M3_I2C_CheckDevice( BMA250_G_SENSOR_ADDR ) == 0)
        {
            BMA250_Delay(20000);
        }
        //LOG_PR_N("使用 E2 \r\n"); 
    }
    
    else
    {
        BMA250_WriteData = BMA250_I2C_WriteData;
        BMA250_ReadData  = BMA250_I2C_ReadData;
        BMA250_GetStatus = I2c_GetStatus;
        //////////////////////////
        if(!I2C_Config())
        {
            BMA250_Delay(20000);
            //////////
            if(!I2C_Config())
            {
                if(!I2C_Config());
            }
        } 
        //LOG_PR_N("使用 I2C \r\n"); 
    }   
    
    //////////////////////
    BMA250_Delay(20000);
    ///////////////////////
    BMA250_UpdataSetParam();
}
/*************************************************************
** 函数名称: BMA250_CheckCrashAlarm
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
float BMA250_GetAngleDifference(unsigned char type)
{
    float temp,angle,base;
    base=s_stBmaAttrib.angle[type];
    temp=BMA250_ReadAccelValue(type); //读取Z轴加速度值    
    //////////////
    if(BMA250_GetStatus())
    {
        return 0.0;
    }
    //////////////////    
    temp = temp/s_stBmaAttrib.gravity;
    if(temp>1.0)
    {
        temp=1.0;
    }
    else
    if(temp<-1.0)
    {
        temp=-1.0;
    }
    //////////////////
    angle=acos(temp);//计算角度
    angle=fabs(angle-base);
    if(angle>BMA_PI)
    {
        angle=fabs(BMA_2_PI-angle);
    }
    return angle;
}
/*************************************************************
** 函数名称: BMA250_CheckCrashAlarm
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void BMA250_CheckCrashAlarm(void)
{
    float sign[BMA_TYPE_MAX];	                //定义变量
    unsigned char i;
    ////////////
    BMA250_ReadAccelValue(BMA_TYPE_Z); //读取Z轴加速度值
    ///////////////
    for(i = BMA_TYPE_X;i<= BMA_TYPE_Z; i++)
    {
        sign[i] = BMA250_GetAngleDifference(i);
    }
    ////////////////
    if(BMA250_GetStatus())
    {
        return;
    }
    ////////////////////////////
    if(sign[BMA_TYPE_X]>=s_stBmaAttrib.anglelimit||sign[BMA_TYPE_Y]>=s_stBmaAttrib.anglelimit||sign[BMA_TYPE_Z]>=s_stBmaAttrib.anglelimit)
    {
        s_stBmaAttrib.resetCnt = 0;
        if(s_stBmaAttrib.setCnt < 4)
        {
            s_stBmaAttrib.setCnt++;
        }
        else
        {
            Io_WriteAlarmBit(ALARM_BIT_SIDE_TURN_PRE_ALARM,SET);
        }
    }
    else
    {
        s_stBmaAttrib.setCnt = 0;        
        if(s_stBmaAttrib.resetCnt < 4)
        {
            s_stBmaAttrib.resetCnt++;
        }
        else
        {
            Io_WriteAlarmBit(ALARM_BIT_SIDE_TURN_PRE_ALARM,RESET);
        }
    }
}
/*************************************************************
** 函数名称: BMA250_CheckAccelOver_App
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void BMA250_CheckAccelOver_App(void)
{
    unsigned char temp;
    static unsigned char num=0;
    static unsigned char semph=0;
    
    temp=Bma250_GetInterruptFlag_A();
    if(temp&4)
    {
        Io_WriteAlarmBit(ALARM_BIT_IMPACT_PRE_ALARM,SET);//报告发生碰撞。  
        if(!semph)
        {
            //SetEvTask(EV_REPORT);
            Report_UploadPositionInfo(CHANNEL_DATA_1);
            Report_UploadPositionInfo(CHANNEL_DATA_2);
        }
        num=0;
        semph=true;																	                            
    }
    else
    {
        num++;
        if(num>100)
        {
            Io_WriteAlarmBit(ALARM_BIT_IMPACT_PRE_ALARM,RESET);//碰撞标志位置位。
        }
        semph=false;
    }
}

/*************************************************************
** 函数名称: BMA250_UpdataCrashAlarmParam
** 功能描述: 更新碰撞报警标志位
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void BMA250_UpdataCrashAlarmParam(void)
{
    BMA250_ReadAndSetCrashAlarmParam();
}
/*************************************************************
** 函数名称: BMA250_UpdataRollOverAlarmParam
** 功能描述: 更新侧翻报警标志位
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void BMA250_UpdataRollOverAlarmParam(void)
{
    BMA250_ReadAndSetRollOverAlarmParam();
}

/*************************************************************
** 函数名称: BMA250_UpdataSetParam
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
void BMA250_ShowDebugInfo(void)
{ 
    #if 0
    float temp[BMA_TYPE_MAX];
    unsigned char val,val1;
    char buffer[100];
    unsigned char i;
    for(i=0;i<BMA_TYPE_MAX; i++ )
    temp[i]=BMA250_ReadAccelValue(i); //读取Z轴加速度值

    val = BMA250_ReadData(BMA250_ACC_RANGE);

    val1=BMA250_ReadData(BMA250_SLOPE_TH);
    //////////////
    sprintf(buffer,"x=%5.2f;y=%5.2f;z=%5.2f  range =%d th =%d \r\n",temp[0],temp[1],temp[2],val,val1);
    LOG_PR_N(buffer);
    #endif
}
/*************************************************************
** 函数名称: BMA250_TimeTask
** 功能描述: 
** 入口参数: 
** 出口参数: 
** 返回参数: 
** 全局变量: 无
** 调用模块: 无
*************************************************************/
FunctionalState BMA250_TimeTask(void)
{
    if(ICCardBase_GetUseIntFlag())
    {
        if(ICCardBase_GetReadFlag())return ENABLE;        
    }
    ////////////////////
    BMA250_CheckCrashAlarm();//倾斜检测
    ///////////
    BMA250_CheckAccelOver_App(); //碰撞检测
    /////////////////////
    //BMA250_ShowDebugInfo();
    
    return ENABLE;
}
		   	
/******************************************************************************
**                            End Of File
******************************************************************************/
	  																			  

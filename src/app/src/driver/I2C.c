/******************************************************************** 
//版权说明	:
//文件名称	:I2C.c		
//功能		:I2C接口操作函数
//版本号	:
//开发人	:yjb
//开发时间	:2012.6
//修改者	:
//修改时间	:
//修改简要说明	:
//备注		: 1) 调试时,重复打开将会导致第二次打开后读写数据出错.
//              : 2)使用如下:
//              :	u8 tmp = 0x7B;
//              :       u8 ret = 0;
//              :       ret = I2C_Config();
//              :        if(ret == 1)                          //初始成功
//              :        {
//              :          ret = I2C_ReadDat(0x00,&tmp);        //读一个字节,需要判断返回值是否失败        
//              :          ret = I2C_WriteDat(0x0A,0x7B);       //写一个字节,需要判断返回值是否失败
//              :          ret = I2C_ReadDat(0x0A,&tmp);        //读一个字节,需要判断返回值是否失败
//              :        }
//              :    注意:初始化,读写函数都有返回值，说明不能确保100%操作成功，需要判断返回值.
***********************************************************************/
//****************包含文件*****************
#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "HAL.h"
#include <stdio.h>

#define		true	1
#define		false	0
//****************宏定义********************
#define   G_SENSOR_ADDR       (0x18<<1)        //BMA250地址

#define   TIME_OUT_COUNT      1000		//1000*1000       //超时计数时间(要小于看门狗时间)
//****************外部变量*******************

//****************全局变量*******************
u8	g_I2C_status;
//****************函数声明*******************

//****************函数定义******************
//设置状态
void	I2c_SetStatus(u8 status)
{
		g_I2C_status=status;
}

//获取I2C工作状态,返回0 工作正常，返回1工作不正常
u8 I2c_GetStatus(void)
{
	return	g_I2C_status;
}
//硬延时
//
//
void I2C_Delay(__IO uint32_t nCount)
{
    while(nCount--)
    {
    }    
}

/****************************************************************
//函数名称	:I2C_Config
//功能		:I2C接口配置
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:失败返回0,成功返回1
//备注		:
*********************************************************************/
u8 I2C_Config(void)
{
	I2C_InitTypeDef  I2C_InitStructure;
	GPIO_InitTypeDef  GPIO_InitStructure; 
        //u16 timeoutmain = 0 ;  //主超时计数
        u16 timeout = 0;       //次级超时计数
    
        //配置地址模式
        GpioOutInit(SHOCK_CS);
        GpioOutOff(SHOCK_CS);       
        
        //使能I2C时钟
        RCC_APB1PeriphClockCmd(I2Cx_CLK,ENABLE);
        
        //复位I2C时钟 - 防止重复打开时序不对
        RCC_APB1PeriphResetCmd(I2Cx_CLK, ENABLE);
        RCC_APB1PeriphResetCmd(I2Cx_CLK, DISABLE);        
        
        RCC_AHB1PeriphClockCmd(I2Cx_SDA_GPIO_CLK | I2Cx_SCL_GPIO_CLK,DISABLE);
        RCC_AHB1PeriphClockCmd(I2Cx_SDA_GPIO_CLK | I2Cx_SCL_GPIO_CLK,ENABLE);
    
        /* PF0,1 SCL and SDA */
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
        GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
        
        GPIO_InitStructure.GPIO_Pin =  I2Cx_SCL_PIN;
        GPIO_Init(I2Cx_SCL_GPIO_PORT, &GPIO_InitStructure);
        
        GPIO_InitStructure.GPIO_Pin =  I2Cx_SDA_PIN;                    //必须在初始化SCL前初始化SDA IO口.
        GPIO_Init(I2Cx_SDA_GPIO_PORT, &GPIO_InitStructure);             //I2C接口使用的GPIO管脚初始化
        
        //映射 I2C_SCL , I2C_SDA
        GPIO_PinAFConfig(I2Cx_SCL_GPIO_PORT, I2Cx_SCL_SOURCE, I2Cx_SCL_AF);
        GPIO_PinAFConfig(I2Cx_SDA_GPIO_PORT, I2Cx_SDA_SOURCE, I2Cx_SDA_AF);       
        
        I2C_DeInit(I2Cx);
        I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;         //设置I2C接口模式 
        I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2; //设置I2C接口的高低电平周期
        I2C_InitStructure.I2C_OwnAddress1 = 0x30;          //设置I2C接口的主机地址
        I2C_InitStructure.I2C_Ack = I2C_Ack_Disable;        //设置是否开启ACK响应          
        I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit; //地址模式,7字节模式
        I2C_InitStructure.I2C_ClockSpeed = 100000;        //100K速度
    
        I2C_Cmd(I2Cx, ENABLE);                            //使能I2C接口
        I2C_Init(I2Cx, &I2C_InitStructure);               //I2C接口初始化
       
        
        //等待I2C总线空闲
        do
        {     
            /*起始位*/
            I2C_GenerateSTART(I2Cx, ENABLE);
            I2C_ReadRegister(I2Cx, I2C_Register_SR1);  //读状态
            I2C_Delay(300*1000);                      //必须给个延迟,否知发送地址会出错.
            I2C_Delay(300*1000); 
            I2C_Delay(300*1000); 
            I2C_Delay(300*1000); 
            I2C_Send7bitAddress(I2Cx, G_SENSOR_ADDR, I2C_Direction_Transmitter);
            if(timeout++ > 3)
            {
                break;
            }
        }while(!(I2C_ReadRegister(I2Cx, I2C_Register_SR1) & 0x0002));
        
        I2C_ClearFlag(I2Cx, I2C_FLAG_AF);
        /*停止位*/    
        I2C_GenerateSTOP(I2Cx, ENABLE);      
        
        if(timeout > 3)                                 //不是超时,说明初始化正常.
        {
            return 0;
        }else{
             return 1;
        }
}
/****************************************************************
//函数名称	:I2C_ReadDat
//功能		:从地址addr读取一个字节
//输入		:addr 地址
//输出		:dat   接收字节数据指针
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:失败返回0,成功返回1
//备注		:
*********************************************************************/
u8 I2C_ReadDat(u8 addr,u8 *dat)
{
        u32 timeout = 0;
        
        timeout = 0;
	while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY))
        {
            if(timeout++ > TIME_OUT_COUNT)
            {
            	I2c_SetStatus(true);
							/*停止位*/    
        			I2C_GenerateSTOP(I2Cx, ENABLE);             	
                return 0;
            }
        };          

	/* 发送起始位 */
        timeout = 0;
        I2C_GenerateSTART(I2Cx, ENABLE);
        while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT))/*EV5,主模式*/
        {
            if(timeout++ > TIME_OUT_COUNT)
            {
            	I2c_SetStatus(true);
                return 0;
            }
        };           
      
        /*发送器件地址(写)*/
        timeout = 0;
        I2C_Send7bitAddress(I2Cx,  G_SENSOR_ADDR, I2C_Direction_Transmitter);      //默认
        while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
        {
            if(timeout++ > TIME_OUT_COUNT)
            {
            	I2c_SetStatus(true);
                return 0;
            }
        };       
        /*发送地址*/
        timeout = 0;
        I2C_SendData(I2Cx, addr);
        while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED))/*数据已发送*/
        {
            if(timeout++ > TIME_OUT_COUNT)
            {
            	I2c_SetStatus(true);
                return 0;
            }
        };           
                
        /*起始位*/
        timeout = 0;
        I2C_GenerateSTART(I2Cx, ENABLE);
        while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT))
        {
            if(timeout++ > TIME_OUT_COUNT)
            {
            	I2c_SetStatus(true);
                return 0;
            }
        };           
        
        /*器件读*/
        timeout = 0;
        I2C_Send7bitAddress(I2Cx, G_SENSOR_ADDR, I2C_Direction_Receiver);
        while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
        {
            if(timeout++ > TIME_OUT_COUNT)
            {
            	I2c_SetStatus(true);
                return 0;
            }
        };             
	
        I2C_GenerateSTOP(I2Cx, ENABLE);		//发送停止位	

        timeout = 0;
        while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED)) /* EV7 */
        {
            if(timeout++ > TIME_OUT_COUNT)
            {
            	I2c_SetStatus(true);
                return 0;
            }
        };               
        *dat =  I2C_ReceiveData(I2Cx);
         
        I2c_SetStatus(false);
        return 1;
}
/****************************************************************
//函数名称	:I2C_WriteDat
//功能		:向地址addr写一个字节
//输入		:addr 地址
//输出		:dat  待写入数据
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:失败返回0,成功返回1
//备注		:
*********************************************************************/
u8 I2C_WriteDat(u8 addr,u8 dat)
{
        u32 timeout = 0;
        
        /* 起始位 */
        timeout = 0;
  	I2C_GenerateSTART(I2Cx, ENABLE);
  	while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT))
        {
            if(timeout++ > TIME_OUT_COUNT)
            {
            	I2c_SetStatus(true);
                return 0;
            }
        };

  	/* 发送器件地址(写)*/
        timeout = 0;
  	I2C_Send7bitAddress(I2Cx, G_SENSOR_ADDR, I2C_Direction_Transmitter);
  	while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
        {
            if(timeout++ > TIME_OUT_COUNT)
            {
            	I2c_SetStatus(true);
                return 0;
            }
        };          
  
  	/*发送地址*/
        timeout = 0;
  	I2C_SendData(I2Cx, addr);
	while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
        {
            if(timeout++ > TIME_OUT_COUNT)
            {
            	I2c_SetStatus(true);
                return 0;
            }
        };              

	/* 写一个字节*/
        timeout = 0;
  	I2C_SendData(I2Cx, dat); 
  	while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
        {
            if(timeout++ > TIME_OUT_COUNT)
            {
            	I2c_SetStatus(true);
                return 0;
            }
        };          
	
  	/* 停止位*/
  	I2C_GenerateSTOP(I2Cx, ENABLE);
        I2c_SetStatus(false);
        return 1;
  
}
/****************************************************************
//函数名称	:I2C_Test
//功能		:测试用
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void I2C_Test(void)
{
	u8 tmp = 0x7B;
        u8 ret = 0;
        ret = I2C_Config();
        if(ret == 1)                          //初始成功
        {
          ret = I2C_ReadDat(0x00,&tmp);        //读一个字节
          ret = ret?ret:ret;          
          ret = I2C_WriteDat(0x0A,0x7B);        //写一个字节
          tmp = tmp?tmp:tmp;
          ret = ret?ret:ret;          
          tmp = 0;
          ret = I2C_ReadDat(0x0A,&tmp);        //读一个字节
          tmp = tmp?tmp:tmp;
          ret = ret?ret:ret;
        }
}
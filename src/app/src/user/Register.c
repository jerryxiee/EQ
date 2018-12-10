/********************************************************************
//版权说明	:Shenzhen E-EYE Co. Ltd., Copyright 2009- All Rights Reserved.
//文件名称	:Register.c		
//功能		:实现行驶记录仪数据记录和查询功能
//版本号	:
//开发人	:dxl
//开发时间	:2013.3
//修改者	:
//修改时间	: 
//修改简要说明	:
//备注		:
***********************************************************************/
//***************包含文件*****************
#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "include.h"
//****************宏定义****************
//***************常量定义***************
const	u16	RegisterStartSector[REGISTER_TYPE_MAX] = 
		{
			0,
			FLASH_SPEED_START_SECTOR,
			FLASH_POSITION_START_SECTOR,
			FLASH_DOUBT_START_SECTOR,
			FLASH_OVER_TIME_START_SECTOR,
			FLASH_DRIVER_START_SECTOR,
			FLASH_POWER_ON_START_SECTOR,
			FLASH_PRAMATER_START_SECTOR,
			FLASH_SPEED_STATUS_START_SECTOR,
		};
const	u16	RegisterEndSector[REGISTER_TYPE_MAX] = 
		{
			0,
			FLASH_SPEED_END_SECTOR,
			FLASH_POSITION_END_SECTOR,
			FLASH_DOUBT_END_SECTOR,
			FLASH_OVER_TIME_END_SECTOR,
			FLASH_DRIVER_END_SECTOR,
			FLASH_POWER_ON_END_SECTOR,
			FLASH_PRAMATER_END_SECTOR,
			FLASH_SPEED_STATUS_END_SECTOR,
		};
const	u16	RegisterStepLen[REGISTER_TYPE_MAX] = 
		{
			0,
			REGISTER_SPEED_STEP_LEN,
			REGISTER_POSITION_STEP_LEN,
			REGISTER_DOUBT_STEP_LEN,
			REGISTER_OVER_TIME_STEP_LEN,
			REGISTER_DRIVER_STEP_LEN,
			REGISTER_POWER_ON_STEP_LEN,
			REGISTER_PRAMATER_STEP_LEN,
			REGISTER_SPEED_STATUS_STEP_LEN,
		};
const	u16	RegisterCollectLen[REGISTER_TYPE_MAX] = 
		{
			0,
			REGISTER_SPEED_COLLECT_LEN,
			REGISTER_POSITION_COLLECT_LEN,
			REGISTER_DOUBT_COLLECT_LEN,
			REGISTER_OVER_TIME_COLLECT_LEN,
			REGISTER_DRIVER_COLLECT_LEN,
			REGISTER_POWER_ON_COLLECT_LEN,
			REGISTER_PRAMATER_COLLECT_LEN,
			REGISTER_SPEED_STATUS_COLLECT_LEN,
		};
const	u16	RegisterMaxBlock[REGISTER_TYPE_MAX] = 
		{
			0,
			REGISTER_SPEED_MAX_BLOCK,
			REGISTER_POSITION_MAX_BLOCK,
			REGISTER_DOUBT_MAX_BLOCK,
			REGISTER_OVER_TIME_MAX_BLOCK,
			REGISTER_DRIVER_MAX_BLOCK,
			REGISTER_POWER_ON_MAX_BLOCK,
			REGISTER_PRAMATER_MAX_BLOCK,
			REGISTER_SPEED_STATUS_MAX_BLOCK,
		};
const	u8	RegisterSubpacketList[REGISTER_TYPE_MAX] = 
		{
			0,
			REGISTER_SPEED_PACKET_LIST,
			REGISTER_POSITION_PACKET_LIST,
			REGISTER_DOUBT_PACKET_LIST,
			REGISTER_OVER_TIME_PACKET_LIST,
			REGISTER_DRIVER_PACKET_LIST,
			REGISTER_POWER_ON_PACKET_LIST,
			REGISTER_PRAMATER_PACKET_LIST,
			REGISTER_SPEED_STATUS_PACKET_LIST,
		};
const	u8	RegisterBlockList[REGISTER_TYPE_MAX] = 
		{
			0,
			REGISTER_SPEED_BLOCK_LIST,
			REGISTER_POSITION_BLOCK_LIST,
			REGISTER_DOUBT_BLOCK_LIST,
			REGISTER_OVER_TIME_BLOCK_LIST,
			REGISTER_DRIVER_BLOCK_LIST,
			REGISTER_POWER_ON_BLOCK_LIST,
			REGISTER_PRAMATER_BLOCK_LIST,
			REGISTER_SPEED_STATUS_BLOCK_LIST,
		};

//*****************变量定义****************
REGISTER_STRUCT Register[REGISTER_TYPE_MAX];

//****************函数定义*****************
/*********************************************************************
//函数名称	:Register_CheckArea(void)
//功能		:检查所有存储区域，确定最老扇区，最新扇区，最新步数
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:正确返回0，否则表示哪个类型的自检出错
//备注		:在上电运行while前需调用该函数，
//		:只有该函数先被调用后才能保证后续的读、写、擦除函数运行正确
*********************************************************************/
u8 Register_CheckArea(void)
{
  /*
	//自检行驶速度区域
	if(SUCCESS != Register_CheckOneArea(REGISTER_TYPE_SPEED))
	{
		if(SUCCESS != Register_CheckOneArea(REGISTER_TYPE_SPEED))//再检一次
		{
			Register_EraseOneArea(REGISTER_TYPE_SPEED);
			return REGISTER_TYPE_SPEED;
		}
	}
	//自检位置信息区域
	if(SUCCESS != Register_CheckOneArea(REGISTER_TYPE_POSITION))
	{
		if(SUCCESS != Register_CheckOneArea(REGISTER_TYPE_POSITION))
		{
			Register_EraseOneArea(REGISTER_TYPE_POSITION);
			return REGISTER_TYPE_POSITION;
		}
	}
  */
	//自检位置信息区域
	if(SUCCESS != Register_CheckOneArea(REGISTER_TYPE_DOUBT))
	{
		if(SUCCESS != Register_CheckOneArea(REGISTER_TYPE_DOUBT))
		{
			Register_EraseOneArea(REGISTER_TYPE_DOUBT);
			return REGISTER_TYPE_DOUBT;
		}
	}
	//自检超时驾驶区域
	if(SUCCESS != Register_CheckOneArea(REGISTER_TYPE_OVER_TIME))
	{
		if(SUCCESS != Register_CheckOneArea(REGISTER_TYPE_OVER_TIME))
		{
			Register_EraseOneArea(REGISTER_TYPE_OVER_TIME);
			return REGISTER_TYPE_OVER_TIME;
		}
	}
	//自检驾驶人身份区域
	if(SUCCESS != Register_CheckOneArea(REGISTER_TYPE_DRIVER))
	{
		if(SUCCESS != Register_CheckOneArea(REGISTER_TYPE_DRIVER))
		{
			Register_EraseOneArea(REGISTER_TYPE_DRIVER);
			return REGISTER_TYPE_DRIVER;
		}
	}
	//自检外部供电区域
	if(SUCCESS != Register_CheckOneArea(REGISTER_TYPE_POWER_ON))
	{
		if(SUCCESS != Register_CheckOneArea(REGISTER_TYPE_POWER_ON))
		{
			Register_EraseOneArea(REGISTER_TYPE_POWER_ON);
			return REGISTER_TYPE_POWER_ON;
		}
	}
	//自检外部供电区域
	if(SUCCESS != Register_CheckOneArea(REGISTER_TYPE_PRAMATER))
	{
		if(SUCCESS != Register_CheckOneArea(REGISTER_TYPE_PRAMATER))
		{
			Register_EraseOneArea(REGISTER_TYPE_PRAMATER);
			return REGISTER_TYPE_PRAMATER;
		}
	}
	//自检速度状态区域
	if(SUCCESS != Register_CheckOneArea(REGISTER_TYPE_SPEED_STATUS))
	{
		if(SUCCESS != Register_CheckOneArea(REGISTER_TYPE_SPEED_STATUS))
		{
			Register_EraseOneArea(REGISTER_TYPE_SPEED_STATUS);
			return REGISTER_TYPE_SPEED_STATUS;
		}
	}

	return 0;
}
/*********************************************************************
//函数名称	:Register_CheckOneArea(void)
//功能		:检查某一个存储区域
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:正确返回SUCCESS，错误返回ERROR
//备注		:
*********************************************************************/
ErrorStatus Register_CheckOneArea(u8 Type)
{
	s16 	i;
	s16	Sector;
	s16	Step;
	u32 	Address;
    	u32 	MinTimeCount;
    	u32 	MaxTimeCount;
    	u32 	TimeCount;
    	u8  	Buffer[REGISTER_MAX_STEP_LEN];
    	u8      VerifySum;
    	TIME_T	tt;

    //*******************检查输入参数**************************

	if(Type >= REGISTER_TYPE_MAX)
	{
		return ERROR;
	}

    //*********************初始化变量*************************
	Register[Type].StartSector = RegisterStartSector[Type];
	Register[Type].EndSector = RegisterEndSector[Type];
	Register[Type].StepLen = RegisterStepLen[Type];
	Register[Type].SectorStep = FLASH_ONE_SECTOR_BYTES/RegisterStepLen[Type];
	Register[Type].OldestSector = RegisterStartSector[Type];
	Register[Type].CurrentSector = RegisterStartSector[Type];
	Register[Type].CurrentStep = 0;
	Register[Type].LoopFlag = 0;

    //********检查历史轨迹和卫星数据包区域内的数据************
    	MinTimeCount = 0xFFFFFFFF;
    	MaxTimeCount = 0;
    	//检查每个扇区的数据包，开始扇区--->结束扇区
    	for(Sector=Register[Type].StartSector; Sector<Register[Type].EndSector; Sector++)
    	{
                IWDG_ReloadCounter();//喂狗
        	for(Step=0; Step<Register[Type].SectorStep; Step++)
        	{
			//读取数据
            		Address = Sector*FLASH_ONE_SECTOR_BYTES + Step*Register[Type].StepLen;
            		sFLASH_ReadBuffer(Buffer,Address,Register[Type].StepLen);

			//确定最新扇区和最新步数
			TimeCount = Register_BytesToInt(Buffer);
			if(TimeCount != 0xFFFFFFFF)	//找到有效时间标签
            		{
			/*
			TimeCount = 0;
			TimeCount |= Buffer[0] << 24;
			TimeCount |= Buffer[1] << 16;
			TimeCount |= Buffer[2] << 8;
			TimeCount |= Buffer[3];
			*/
				//校验数据
				VerifySum = Register_GetVerifySum(Buffer,Register[Type].StepLen-1);//步长==时间字节4+数据n字节+校验1字节
				if(VerifySum != Buffer[Register[Type].StepLen-1])
				{
					//错误，直接下一条
				}
                                else
                                {
                		        Gmtime(&tt, TimeCount);
                                        if(SUCCESS == CheckTimeStruct(&tt))
                                        {
                		                if(TimeCount < MinTimeCount)
                		                {
                    			                MinTimeCount = TimeCount;
                    			                Register[Type].OldestSector = Sector; //最老扇区
                		                }
                		                if(TimeCount > MaxTimeCount)
                		                {
                    			                MaxTimeCount = TimeCount;
                    			                Register[Type].CurrentStep = Step+1;//最新步数
                    			                Register[Type].CurrentSector = Sector;//最新扇区号
                		                }
                                        }
                                        else
                                        {
                                                //错误，直接下一条
                                        }
                                }
            		}
            		else	//没有找到有效时间标签，跳出循环查找下一个扇区
            		{
                		break; 
            		}
    		}
    	}
    	//判断当前步是否达到扇区边界
    	if(Register[Type].CurrentStep >= Register[Type].SectorStep)
    	{
        	Register[Type].CurrentStep = 0;
        	Register[Type].CurrentSector++;
        	if(Register[Type].CurrentSector >= Register[Type].EndSector)
        	{
            		Register[Type].CurrentSector = Register[Type].StartSector;	
        	}
    	}
    	//计算区域最后一步的地址，判断是否进入循环存储
    	Address = (Register[Type].EndSector-1)*FLASH_ONE_SECTOR_BYTES + (Register[Type].SectorStep-1)*Register[Type].StepLen;
	sFLASH_ReadBuffer(Buffer,Address,Register[Type].StepLen);
    	for(i=0; i<Register[Type].StepLen; i++)
    	{
        	if(Buffer[i] != 0xFF)
        	{
            		Register[Type].LoopFlag = 1;
        	}
    	}
	return SUCCESS;
}
/*********************************************************************
//函数名称	:Register_CheckOneArea2(void)
//功能		:检查某一个存储区域
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:返回结束标志，2为参数错误，1为该区域检查结束，0为未结束，
//备注		:只用于位置记录、速度记录；调用该函数的应用需要保证只有
//              :返回值为1时（表明自检结束）才能往该区域写数据
//              :REGISTER_TYPE_SPEED、REGISTER_TYPE_POSITION这两个类型的检查可调用此函数
*********************************************************************/
u8 Register_CheckOneArea2(u8 Type)
{
	s16 	i;
	s16	Step;
	u32 	Address;
    	
    	u32 	TimeCount;
    	u8  	Buffer[REGISTER_MAX_STEP_LEN];
    	u8      VerifySum;
    	TIME_T	tt;
        
        static s16	Sector[3] = {0,0,0};
        static u32 	MinTimeCount[3] = {0,0,0};
    	static u32 	MaxTimeCount[3] = {0,0,0};

	if(Type <= REGISTER_TYPE_POSITION)//只有REGISTER_TYPE_SPEED、REGISTER_TYPE_POSITION这两个类型的检查可调用此函数
	{
		
	}
        else
        {
                return 2;
        }

        if(0 == Sector[Type])//第1次进入该函数
        {
                Register[Type].StartSector = RegisterStartSector[Type];
	        Register[Type].EndSector = RegisterEndSector[Type];
	        Register[Type].StepLen = RegisterStepLen[Type];
	        Register[Type].SectorStep = FLASH_ONE_SECTOR_BYTES/RegisterStepLen[Type];
	        Register[Type].OldestSector = RegisterStartSector[Type];
	        Register[Type].CurrentSector = RegisterStartSector[Type];
	        Register[Type].CurrentStep = 0;
	        Register[Type].LoopFlag = 0;
                Sector[Type] = RegisterStartSector[Type];
                MinTimeCount[Type] = 0xFFFFFFFF;
    	        MaxTimeCount[Type] = 0;
        }
        else if(Sector[Type] >= Register[Type].EndSector)
        {
                return 1;
        }
    	
    	//一次检查一个扇区的数据包
        IWDG_ReloadCounter();//喂狗
        for(Step=0; Step<Register[Type].SectorStep; Step++)
        {
		//读取数据
            	Address = Sector[Type]*FLASH_ONE_SECTOR_BYTES + Step*Register[Type].StepLen;
            	sFLASH_ReadBuffer(Buffer,Address,Register[Type].StepLen);

		//确定最新扇区和最新步数
		TimeCount = Register_BytesToInt(Buffer);
		if(TimeCount != 0xFFFFFFFF)	//找到有效时间标签
            	{
			//Gmtime(&tt, TimeCount);
			/*
			TimeCount = 0;
			TimeCount |= Buffer[0] << 24;
			TimeCount |= Buffer[1] << 16;
			TimeCount |= Buffer[2] << 8;
			TimeCount |= Buffer[3];
			*/
			//校验数据
			VerifySum = Register_GetVerifySum(Buffer,Register[Type].StepLen-1);//步长==时间字节4+数据n字节+校验1字节
			if(VerifySum != Buffer[Register[Type].StepLen-1])
			{
				//错误，直接下一条
			}
                        else
                        {
                	        Gmtime(&tt, TimeCount);
                                if(SUCCESS == CheckTimeStruct(&tt))
                                {
                	                if(TimeCount < MinTimeCount[Type])
                	                {
                    		                MinTimeCount[Type] = TimeCount;
                    		                Register[Type].OldestSector = Sector[Type]; //最老扇区
                	                }
                	                if(TimeCount > MaxTimeCount[Type])
                	                {
                    		                MaxTimeCount[Type] = TimeCount;
                    		                Register[Type].CurrentStep = Step+1;//最新步数
                    		                Register[Type].CurrentSector = Sector[Type];//最新扇区号
                	                }
                                }
                                else
                                {
                                        //错误，直接下一条
                                }
                        }
            	}
            	else	//没有找到有效时间标签，跳出循环查找下一个扇区
            	{
                	break; 
            	}
    	}
        Sector[Type]++;
        if(Sector[Type] >= Register[Type].EndSector)
        {
                //判断当前步是否达到扇区边界
    	        if(Register[Type].CurrentStep >= Register[Type].SectorStep)
    	        {
        	        Register[Type].CurrentStep = 0;
        	        Register[Type].CurrentSector++;
        	        if(Register[Type].CurrentSector >= Register[Type].EndSector)
        	        {
            		        Register[Type].CurrentSector = Register[Type].StartSector;	
        	        }
    	        }
    	        //计算区域最后一步的地址，判断是否进入循环存储
    	        Address = (Register[Type].EndSector-1)*FLASH_ONE_SECTOR_BYTES + (Register[Type].SectorStep-1)*Register[Type].StepLen;
	        sFLASH_ReadBuffer(Buffer,Address,Register[Type].StepLen);
    	        for(i=0; i<Register[Type].StepLen; i++)
    	        {
        	        if(Buffer[i] != 0xFF)
        	        {
            		        Register[Type].LoopFlag = 1;
        	        }
    	        }
                return 1;
        }
        else
        {
                return 0;
        }
}
/*********************************************************************
//函数名称	:Register_Write(u8 Type,u8 *pBuffer, u8 length)
//功能		:记录一条某种类型的信息
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:正确返回SUCCESS，错误返回ERROR
//备注		:
*********************************************************************/
ErrorStatus Register_Write(u8 Type,u8 *pBuffer, u16 length)
{
  
    	u8	Buffer[REGISTER_MAX_STEP_LEN];
    	u16	i;
    	u16	j;
	u32	TimeCount;
	u32 	Address;
	u8	VerifySum;
	
	/*
	static  u32 TestTimeCount = 0;
	
	if(TestTimeCount == 0)
	{
		//TestTimeCount = RTC_GetCounter();
	}
	
	if(Type == REGISTER_TYPE_POSITION)
	{
		  
		    TimeCount = TestTimeCount;
		    TestTimeCount += 60;
		    if(Register[Type].LoopFlag != 0)
		   {
			  TestTimeCount = 0;
		   }
		     TIME_T tt;
    		    u8	*p;
    		    u8	i;
    
                    p = pBuffer;
                    Gmtime(&tt, TimeCount);
    
    	            *p++ = Public_HEX2BCD(tt.sec);
	            *p++ = Public_HEX2BCD(tt.min);
	            *p++ = Public_HEX2BCD(tt.hour);
	            *p++ = Public_HEX2BCD(tt.day);
	            *p++ = Public_HEX2BCD(tt.sec);
	            *p++ = Public_HEX2BCD(tt.min);
	            *p++ = Public_HEX2BCD(tt.hour);
	            *p++ = Public_HEX2BCD(tt.day);
	            for(i=0; i<11; i++)
	            {
		            *p++ = 0;
	            }
	}
	else
	{
	*/
	//读当前时间
	TimeCount = RTC_GetCounter();
	//}
	//检查参数正确性
	if(ERROR == Register_CheckPram(Type,TimeCount,length))
	{
		return ERROR;
	}
        
	
    	//检查所写的区域是否全为0XFF
    	Address = Register[Type].CurrentSector*FLASH_ONE_SECTOR_BYTES + Register[Type].CurrentStep*Register[Type].StepLen;
    	sFLASH_ReadBuffer(Buffer,Address,Register[Type].StepLen);
    	for(i=0; i<Register[Type].StepLen; i++)
    	{
        	if(0xff != Buffer[i])
        	{
            		//擦除该扇区，写入到第0步
            		sFLASH_EraseSector(Address);
            		Register[Type].CurrentStep = 0;
            		for(j=0; j<200; j++);
            		Address = Register[Type].CurrentSector*FLASH_ONE_SECTOR_BYTES;
            		break;
        	}
    	}	
	
	//写时间与长度,Buffer[0]-Buffer[3]是存储的数据，Buffer[4]-Buffer[7]是读取的数据
	Register_IntToBytes(Buffer, TimeCount);
	sFLASH_WriteBuffer(Buffer, Address, 4);
    	sFLASH_ReadBuffer(Buffer+4,Address,4);
    	for(i=0; i<4; i++)
    	{
        	if(Buffer[i] != Buffer[i+4])
        	{
            		//不一样，擦除该扇区，立即返回
            		sFLASH_EraseSector(Address);
            		Register[Type].CurrentStep = 0;
            		return ERROR;
        	}
    	}
	Address += 4;
	VerifySum = Register_GetVerifySum(Buffer, 4);

    	//写数据，并读取出来比较
    	sFLASH_WriteBuffer(pBuffer, Address, length);
    	sFLASH_ReadBuffer(Buffer,Address,length);
    	for(i=0; i<length; i++)
    	{
        	if(*(pBuffer+i) != Buffer[i])
        	{
            		//不一样，擦除该扇区，立即返回
            		sFLASH_EraseSector(Address);
            		Register[Type].CurrentStep = 0;
            		return ERROR;
        	}
    	}
	Address += length;

	//写校验码
	VerifySum += Register_GetVerifySum(pBuffer, length);
	sFLASH_WriteBuffer(&VerifySum, Address, 1);
    	sFLASH_ReadBuffer(Buffer,Address,1);
	if(Buffer[0] != VerifySum)
	{
		//不一样，擦除该扇区，立即返回
            	sFLASH_EraseSector(Address);
            	Register[Type].CurrentStep = 0;
            	return ERROR;
	}

    	//当前步数加1
    	Register[Type].CurrentStep++;
    	if(Register[Type].CurrentStep  >= Register[Type].SectorStep)
    	{
       		Register[Type].CurrentStep = 0;	
        	Register[Type].CurrentSector++;	//当前扇区下移一个
        	//判断当前扇区是否达到末尾
        	if(Register[Type].CurrentSector >= Register[Type].EndSector)
        	{
           		Register[Type].CurrentSector = Register[Type].StartSector;//指向起始扇区
            		Register[Type].LoopFlag = 1;
        	}
        	//最老扇区下移
        	if(Register[Type].LoopFlag > 0)
        	{
            		Register[Type].OldestSector++;
            		if(Register[Type].OldestSector >= Register[Type].EndSector)
            		{
                		Register[Type].OldestSector = Register[Type].StartSector;//指向起始扇区
            		}
        	}
		Address = Register[Type].CurrentSector*FLASH_ONE_SECTOR_BYTES;
        	sFLASH_EraseSector(Address);
    	}	
    	return SUCCESS;
}
/*********************************************************************
//函数名称	:Register_Write2(u8 Type,u8 *pBuffer, u8 length, u32 Time)
//功能		:记录一条某种类型的信息
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:正确返回SUCCESS，错误返回ERROR
//备注		:
*********************************************************************/
ErrorStatus Register_Write2(u8 Type,u8 *pBuffer, u16 length,u32 Time)
{
  
    	u8	Buffer[REGISTER_MAX_STEP_LEN];
    	u16	i;
    	u16	j;
	u32	TimeCount;
	u32 	Address;
	u8	VerifySum;
	
	/*
	static  u32 TestTimeCount = 0;
	
	if(TestTimeCount == 0)
	{
		//TestTimeCount = RTC_GetCounter();
	}
	
	if(Type == REGISTER_TYPE_POSITION)
	{
		  
		    TimeCount = TestTimeCount;
		    TestTimeCount += 60;
		    if(Register[Type].LoopFlag != 0)
		   {
			  TestTimeCount = 0;
		   }
		     TIME_T tt;
    		    u8	*p;
    		    u8	i;
    
                    p = pBuffer;
                    Gmtime(&tt, TimeCount);
    
    	            *p++ = Public_HEX2BCD(tt.sec);
	            *p++ = Public_HEX2BCD(tt.min);
	            *p++ = Public_HEX2BCD(tt.hour);
	            *p++ = Public_HEX2BCD(tt.day);
	            *p++ = Public_HEX2BCD(tt.sec);
	            *p++ = Public_HEX2BCD(tt.min);
	            *p++ = Public_HEX2BCD(tt.hour);
	            *p++ = Public_HEX2BCD(tt.day);
	            for(i=0; i<11; i++)
	            {
		            *p++ = 0;
	            }
	}
	else
	{
	*/
	//读当前时间
	//TimeCount = RTC_GetCounter();
        TimeCount = Time;
	//}
	//检查参数正确性T
	if(ERROR == Register_CheckPram(Type,TimeCount,length))
	{
		return ERROR;
	}
        
	
    	//检查所写的区域是否全为0XFF
    	Address = Register[Type].CurrentSector*FLASH_ONE_SECTOR_BYTES + Register[Type].CurrentStep*Register[Type].StepLen;
    	sFLASH_ReadBuffer(Buffer,Address,Register[Type].StepLen);
    	for(i=0; i<Register[Type].StepLen; i++)
    	{
        	if(0xff != Buffer[i])
        	{
            		//擦除该扇区，写入到第0步
            		sFLASH_EraseSector(Address);
            		Register[Type].CurrentStep = 0;
            		for(j=0; j<200; j++);
            		Address = Register[Type].CurrentSector*FLASH_ONE_SECTOR_BYTES;
            		break;
        	}
    	}	
	
	//写时间与长度,Buffer[0]-Buffer[3]是存储的数据，Buffer[4]-Buffer[7]是读取的数据
	Register_IntToBytes(Buffer, TimeCount);
	sFLASH_WriteBuffer(Buffer, Address, 4);
    	sFLASH_ReadBuffer(Buffer+4,Address,4);
    	for(i=0; i<4; i++)
    	{
        	if(Buffer[i] != Buffer[i+4])
        	{
            		//不一样，擦除该扇区，立即返回
            		sFLASH_EraseSector(Address);
            		Register[Type].CurrentStep = 0;
            		return ERROR;
        	}
    	}
	Address += 4;
	VerifySum = Register_GetVerifySum(Buffer, 4);

    	//写数据，并读取出来比较
    	sFLASH_WriteBuffer(pBuffer, Address, length);
    	sFLASH_ReadBuffer(Buffer,Address,length);
    	for(i=0; i<length; i++)
    	{
        	if(*(pBuffer+i) != Buffer[i])
        	{
            		//不一样，擦除该扇区，立即返回
            		sFLASH_EraseSector(Address);
            		Register[Type].CurrentStep = 0;
            		return ERROR;
        	}
    	}
	Address += length;

	//写校验码
	VerifySum += Register_GetVerifySum(pBuffer, length);
	sFLASH_WriteBuffer(&VerifySum, Address, 1);
    	sFLASH_ReadBuffer(Buffer,Address,1);
	if(Buffer[0] != VerifySum)
	{
		//不一样，擦除该扇区，立即返回
            	sFLASH_EraseSector(Address);
            	Register[Type].CurrentStep = 0;
            	return ERROR;
	}

    	//当前步数加1
    	Register[Type].CurrentStep++;
    	if(Register[Type].CurrentStep  >= Register[Type].SectorStep)
    	{
       		Register[Type].CurrentStep = 0;	
        	Register[Type].CurrentSector++;	//当前扇区下移一个
        	//判断当前扇区是否达到末尾
        	if(Register[Type].CurrentSector >= Register[Type].EndSector)
        	{
           		Register[Type].CurrentSector = Register[Type].StartSector;//指向起始扇区
            		Register[Type].LoopFlag = 1;
        	}
        	//最老扇区下移
        	if(Register[Type].LoopFlag > 0)
        	{
            		Register[Type].OldestSector++;
            		if(Register[Type].OldestSector >= Register[Type].EndSector)
            		{
                		Register[Type].OldestSector = Register[Type].StartSector;//指向起始扇区
            		}
        	}
		Address = Register[Type].CurrentSector*FLASH_ONE_SECTOR_BYTES;
        	sFLASH_EraseSector(Address);
    	}	
    	return SUCCESS;
}
/*********************************************************************
//函数名称	:Register_Read(u8 Type,u8 *pBuffer, TIME_T StartTime, TIME_T EndTime, u16 MaxBlock)
//功能		:读取指定时间段，指定最大数据块的记录数据
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:读取的数据长度
//备注		:
*********************************************************************/
u16 Register_Read(u8 Type,u8 *pBuffer, TIME_T StartTime, TIME_T EndTime, u16 MaxBlock)
{
	u16	BlockCount = 0;
	u32	ttCount;
	u32	ttCount2;
	u32	ttCount3;
	u32	NextCount;
	u32	TimeCoeff;
	u32	StartTimeCount;
	u32	EndTimeCount;
	s16	Sector;
	s16	Step;
	s16	NextSector;
	s16	NextStep;
	s16	EndSector;
	s16	EndStep;
	u16	length = 0;
	u32	Address;
	u8	Buffer[REGISTER_MAX_STEP_LEN];
	s8	WriteCount = 0;
	s32	i,j,k;
	u8	flag;
	u8	*p = NULL;
	u8	IncludeFlag;
	u32	IncludeCount;
	TIME_T	tt;
	TIME_T	test2;
	TIME_T	test3;

	//转换时间
	StartTimeCount = ConverseGmtime(&StartTime);	
	EndTimeCount = ConverseGmtime(&EndTime);

	//判断时间先后关系
	if(StartTimeCount >= EndTimeCount)
	{
		return length;
	}	

	if(Type == REGISTER_TYPE_SPEED)
	{
		StartTimeCount -= StartTime.sec;//秒清0
		EndTimeCount -= EndTime.sec;//秒清0	
	}
	else if(Type == REGISTER_TYPE_POSITION)	
	{
		StartTimeCount -= StartTime.sec;//秒清0
		StartTimeCount -= StartTime.min*60;//分清0
		EndTimeCount -= EndTime.sec;//秒清0
		EndTimeCount -= EndTime.min*60;//分清0
	}
	p = pBuffer;
	IncludeFlag = 0;
	//确定查找起始步和结束步
	ttCount = Register_Search(Type, EndTimeCount, &Sector, &Step);//起始位置Sector,Step,从结束时间往前找
	if(ttCount > EndTimeCount)//没有满足条件的,后续不需要再查找
	{
		return length;
	}
	IncludeCount = Register_Search(Type, StartTimeCount, &EndSector, &EndStep);//结束位置EndSector,EndStep
	if(IncludeCount >= StartTimeCount)
	{
		IncludeFlag = 1;
	}
	switch(Type)
	{	
    #if (REGISTER_SPEED_STEP_LEN == 11)//modify by joneming
    case REGISTER_TYPE_SPEED:
    #endif
	case REGISTER_TYPE_POSITION:
		{
			if((Sector == EndSector)&&(Step == EndStep))//只有一条，默认为0块。
			{
				return length;
			}            
            #if (REGISTER_SPEED_STEP_LEN == 11)//modify by joneming
            if(REGISTER_TYPE_SPEED == Type)
			{
				TimeCoeff = 60;//1分钟一个数据块
			}
			else
            #endif
			{
				TimeCoeff = 3600;//1小时一个数据块
			}            
			ttCount = EndTimeCount - TimeCoeff;
			for(;;)
			{
				ttCount2 = Register_Search(Type, ttCount, &Sector, &Step);
				Gmtime(&test2, ttCount2);
				//下移一步,dxl,2013.5.10
				NextSector = Sector;
				NextStep = Step;
				NextStep++;
				if(NextStep >= Register[Type].SectorStep)
				{
					NextStep = 0;
					NextSector++;
					if(NextSector >= Register[Type].EndSector)
					{
						NextSector = Register[Type].StartSector;
					}
				}
				Address = NextSector*FLASH_ONE_SECTOR_BYTES+NextStep*Register[Type].StepLen;
				sFLASH_ReadBuffer(Buffer,Address,4);
				NextCount = Register_BytesToInt(Buffer);
				//if(ttCount >= (ttCount2+TimeCoeff))
				if((ttCount >= (ttCount2+TimeCoeff))&&(ttCount < (NextCount-TimeCoeff)))//dxl,2013.5.10
				{
					for(;;)
					{
						if(ttCount < (ttCount2+2*TimeCoeff))
						{
							break;
						}
						else
						{
							ttCount -= TimeCoeff;
						}
					}
					
				}
				else if(ttCount < ttCount2)
				{
					return length;
				}
				else 
				{
					//下移一步，因为查找返回的数据总是小于或等于给定值
					flag = 0;
					if(ttCount2 < ttCount)
					{
						for(;;)
						{
							//下移一步
							Step++;
							if(Step >= Register[Type].SectorStep)
							{
								Step = 0;
								Sector++;
								if(Sector >= Register[Type].EndSector)
								{
									Sector = Register[Type].StartSector;
								}
							}
							Address = Sector*FLASH_ONE_SECTOR_BYTES+Step*Register[Type].StepLen;
							sFLASH_ReadBuffer(Buffer,Address,4);
							ttCount2 = Register_BytesToInt(Buffer);
							Gmtime(&test2, ttCount2);
							if(ttCount2 >= ttCount+TimeCoeff)
							{
								flag = 1;
								break;
							}
							else if(ttCount2 >= ttCount)
							{
								break;
							}
							if((Sector == EndSector)&&(Step == EndStep))
							{
								break;
							}
						}
					}
					if(0 == flag)
					{
						//按格式要求写入开始时间
						Gmtime(&tt, ttCount);
						*p++ = Public_HEX2BCD(tt.year);
						length++;
						*p++ = Public_HEX2BCD(tt.month);
						length++;
						*p++ = Public_HEX2BCD(tt.day);
						length++;
						*p++ = Public_HEX2BCD(tt.hour);
						length++;
						*p++ = Public_HEX2BCD(tt.min);
						length++;
						*p++ = Public_HEX2BCD(tt.sec);
						length++;
						WriteCount = 0;
						//写入补齐的字节0xff
						j = (ttCount2 - ttCount)*60/TimeCoeff;
						for(i=0; i<j; i++)
						{
							for(k=0; k<RegisterCollectLen[Type]; k++)
							{
								*p++ = 0xff;
								length++;
							}
							WriteCount++;
						}
						//写入数据
						for(;;)
						{
							//读取并写入数据
							Address = Sector*FLASH_ONE_SECTOR_BYTES+Step*Register[Type].StepLen;
							sFLASH_ReadBuffer(Buffer,Address,Register[Type].StepLen);
							ttCount3 = Register_BytesToInt(Buffer);
							Gmtime(&test3, ttCount3);
							//写入补齐的字节0xff
							j = (ttCount3 - ttCount2)*60/TimeCoeff;
							j--;
							if(j > (60-WriteCount))
							{
								j = 60-WriteCount;
							}
							for(i=0; i<j; i++)
							{
								for(k=0; k<RegisterCollectLen[Type]; k++)
								{
									*p++ = 0xff;
									length++;
								}
								WriteCount++;
							}
							if(WriteCount >= 60)
							{
								break;
							}
							if(Buffer[Register[Type].StepLen-1] == Register_GetVerifySum(Buffer, Register[Type].StepLen-1))
							{
								memcpy(p,Buffer+4,RegisterCollectLen[Type]);
								p += RegisterCollectLen[Type];
								length += RegisterCollectLen[Type];
								WriteCount++;
								if(WriteCount >= 60)
								{
									break;
								}
							}
							//下移一步
							Step++;
							if(Step >= Register[Type].SectorStep)
							{
								Step = 0;
								Sector++;
								if(Sector >= Register[Type].EndSector)
								{
									Sector = Register[Type].StartSector;
								}
							}
							ttCount2 = ttCount3;
							Gmtime(&test2, ttCount2);
						}
						BlockCount++;      
					
					}
				}
				if((BlockCount >= RegisterMaxBlock[Type])||(BlockCount >= MaxBlock))
				{
					break;
				}
				if((Sector == EndSector)&&(Step == EndStep))
				{
					break;
				}
				ttCount -= TimeCoeff;
				if(ttCount < StartTimeCount)
				{
					break;
				}
			}
			break;
		}
    #if (REGISTER_SPEED_STEP_LEN != 11)//modify by joneming
    case REGISTER_TYPE_SPEED:
    #endif
	case REGISTER_TYPE_OVER_TIME:
	case REGISTER_TYPE_DOUBT:
	case REGISTER_TYPE_DRIVER:
	case REGISTER_TYPE_POWER_ON:
	case REGISTER_TYPE_PRAMATER:
	case REGISTER_TYPE_SPEED_STATUS:
		{
			if((Sector == EndSector)&&(Step == EndStep))
			{
				if((ttCount >= StartTimeCount)&&(ttCount <= EndTimeCount))//有一个数据满足条件的
				{
					//读取并校验数据
					Address = Sector*FLASH_ONE_SECTOR_BYTES+Step*Register[Type].StepLen;
					sFLASH_ReadBuffer(Buffer,Address,Register[Type].StepLen);
					ttCount = Register_BytesToInt(Buffer);
					if(Buffer[Register[Type].StepLen-1] == Register_GetVerifySum(Buffer, Register[Type].StepLen-1))
					{
				
						memcpy(p,Buffer+4,RegisterCollectLen[Type]);
						if(Public_JudgeBCD(p, 6))
						{
							p += RegisterCollectLen[Type];
							length += RegisterCollectLen[Type];
						}
					}
				}
				return length;
			}
			for(;;)
			{
				//读取并校验数据
				Address = Sector*FLASH_ONE_SECTOR_BYTES+Step*Register[Type].StepLen;
				sFLASH_ReadBuffer(Buffer,Address,Register[Type].StepLen);
				ttCount = Register_BytesToInt(Buffer);
				if(Buffer[Register[Type].StepLen-1] == Register_GetVerifySum(Buffer, Register[Type].StepLen-1))
				{
					if(ttCount < StartTimeCount)
					{
						break;
					}
					else
					{
						memcpy(p,Buffer+4,RegisterCollectLen[Type]);
						if(Public_JudgeBCD(p, 6))
						{
							p += RegisterCollectLen[Type];
							length += RegisterCollectLen[Type];
						}
						BlockCount++;
						if(length > (1000-RegisterCollectLen[Type]))
						{
							break;
						}
						if((BlockCount >= RegisterMaxBlock[Type])||(BlockCount >= MaxBlock))
						{
							break;
						}
					}
				}
				//上移一步
				Step--;
				if(Step < 0)
				{
					Step = Register[Type].SectorStep-1;
					Sector--;
					if(Sector < Register[Type].StartSector)
					{
						Sector = Register[Type].EndSector-1;
					}
				}
				//判断是否到达终止步
				if((Sector == EndSector)&&(Step == EndStep))
				{
					if(1 == IncludeFlag)
					{
						Address = Sector*FLASH_ONE_SECTOR_BYTES+Step*Register[Type].StepLen;
						sFLASH_ReadBuffer(Buffer,Address,Register[Type].StepLen);
						memcpy(p,Buffer+4,RegisterCollectLen[Type]);
						if(Public_JudgeBCD(p, 6))
						{
							p += RegisterCollectLen[Type];
							length += RegisterCollectLen[Type];
						}
						BlockCount++;
					}
					break;
				}
					
			}	
			break;
		}
	default: break;
		
	}
	
	return length;
}
/*********************************************************************
//函数名称	:Register_GetNewestTime(u8 Type)
//功能		:获取当前存储的最新步数的时间
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:4字节表示的时间
//备注		:
*********************************************************************/
u32 Register_GetNewestTime(u8 Type)
{
	s16	Step;
	s16	Sector;
	u32	Address;
	u32	ttCount;
	u8	Buffer[5];
	
	Step = Register[Type].CurrentStep;
	Sector = Register[Type].CurrentSector;
	Step--;
	if(Step < 0)
	{
		Step = Register[Type].SectorStep-1;
		Sector--;
		if(Sector < Register[Type].StartSector)
		{
			Sector = Register[Type].EndSector - 1;
		}
	}
	Address = Sector*FLASH_ONE_SECTOR_BYTES+Step*Register[Type].StepLen;
	sFLASH_ReadBuffer(Buffer,Address,4);
	ttCount = Register_BytesToInt(Buffer);
	return ttCount;
}
/*********************************************************************
//函数名称	:Register_GetOldestTime(u8 Type)
//功能		:获取当前存储的最老步数的时间
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:4字节表示的时间
//备注		:
*********************************************************************/
u32 Register_GetOldestTime(u8 Type)
{
	s16	Sector;
	u32	Address;
	u32	ttCount;
	u8	Buffer[5];
	
	Sector = Register[Type].OldestSector;
	Address = Sector*FLASH_ONE_SECTOR_BYTES;
	sFLASH_ReadBuffer(Buffer,Address,4);
	ttCount = Register_BytesToInt(Buffer);
	return ttCount;
}
/*********************************************************************
//函数名称	:Register_Search(u8 Type,u32 Time, s16 *Sector, s16 *Step)
//功能		:查找某一给定时间的存储信息
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:实际查找得到的时间，该时间总是不大于Time，除非存储的时间都是大于Time的
//备注		:查找的结构放在Sector，Step中
*********************************************************************/
u32 Register_Search(u8 Type, u32 Time, s16 *Sector, s16 *Step)
{
	s16	SearchSector;
	s16	SearchStep;
	u32	Address;
	u8	Buffer[5];
	u8	flag = 0;
	u32	TimeCount;
	
	
                IWDG_ReloadCounter();//喂狗

	//从最新扇区开始查找
	SearchSector = Register[Type].CurrentSector;
	SearchStep = 0;
	if(0 == Register[Type].CurrentStep)
	{
		SearchSector--;
		if(SearchSector < Register[Type].StartSector)
		{
			SearchSector = Register[Type].EndSector-1;
		}
	}

	//确定给定时间是属于哪个扇区
	for(;;)
	{
		Address = SearchSector*FLASH_ONE_SECTOR_BYTES;
		sFLASH_ReadBuffer(Buffer,Address,4);
		TimeCount = Register_BytesToInt(Buffer);
		/*
		TimeCount = 0;
		TimeCount |= Buffer[0] << 24;
		TimeCount |= Buffer[1] << 16;
		TimeCount |= Buffer[2] << 8;
		TimeCount |= Buffer[3];
		*/
		if(TimeCount > Time)
		{
			SearchSector--;
			if(SearchSector < Register[Type].StartSector)
			{
				SearchSector = Register[Type].EndSector-1;
			}
			if(SearchSector == Register[Type].OldestSector)//达到最老扇区
			{
				//break;dxl,2013.6.7,不需要往后找了,没有满足条件的
                                /*********************************************///传递第一扇区地址以及时间 myh 130624
                                Address = SearchSector*FLASH_ONE_SECTOR_BYTES;
                                sFLASH_ReadBuffer(Buffer,Address,4);
                                TimeCount = Register_BytesToInt(Buffer);
                                /*********************************************/
                                *Sector = SearchSector;
				*Step = SearchStep;                                        
                                break;                                         //原来为return TimeCount，退出循环进行步数确定 myh 130624   
			}
		}
		else if(TimeCount < Time)
		{
			break;
		}
		else
		{
			*Sector = SearchSector;
			*Step = SearchStep;
			return TimeCount;
		}	
	}
	flag = 0;
	//确定给定时间是属于哪步
	for(;;)
	{
		Address = SearchSector*FLASH_ONE_SECTOR_BYTES+SearchStep*Register[Type].StepLen;
		sFLASH_ReadBuffer(Buffer,Address,4);
		TimeCount = Register_BytesToInt(Buffer);
		/*
		TimeCount = 0;
		TimeCount |= Buffer[0] << 24;
		TimeCount |= Buffer[1] << 16;
		TimeCount |= Buffer[2] << 8;
		TimeCount |= Buffer[3];
		*/
		if(TimeCount > Time)
		{
			flag = 1;
			break;	
		}
		else if(TimeCount < Time)
		{
			if(SearchSector == Register[Type].CurrentSector)
			{
				SearchStep++;
				///if(SearchStep >= Register[Type].CurrentStep-1)
				if(SearchStep >= Register[Type].CurrentStep)//dxl,2013.6.7下一步将是当前步数了
				{
					flag = 1;//dxl,2013.6.7,需要退一步
					break;
				}
			}
			else
			{
				SearchStep++;
				//if(SearchStep >= Register[Type].SectorStep-1)
				if(SearchStep >= Register[Type].SectorStep)//dxl,2013.6.7下一步将是下一个扇区了
				{
					flag = 1;//dxl,2013.6.7,需要退一步
					break;
				}
			}
		}
		else
		{
			break;
		}
	}
	if(1 == flag)
	{
		SearchStep--;
		if(SearchStep < 0)
		{
			SearchStep = 0;
		}
	}
	Address = SearchSector*FLASH_ONE_SECTOR_BYTES+SearchStep*Register[Type].StepLen;
	sFLASH_ReadBuffer(Buffer,Address,4);
	TimeCount = Register_BytesToInt(Buffer);
	*Sector = SearchSector;
	*Step = SearchStep;
	return TimeCount;
}
/*********************************************************************
//函数名称	:Register_GetVerifySum(u8 *pBuffer, u16 length)
//功能		:获取输入字符串数据的校验和
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
u8 Register_GetVerifySum(u8 *pBuffer, u16 length)
{
	u8	sum = 0;
	u16	i;

	for(i=0; i<length; i++)
	{
		sum += *(pBuffer+i);
	}
	return sum;
}
/*********************************************************************
//函数名称	:Register_CheckPram(u8 Type, u32 Time, u8 length)
//功能		:检查输入参数的合法性
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:成功返回SUCCESS,失败返回ERROR
//备注		:
*********************************************************************/
ErrorStatus Register_CheckPram(u8 Type, u32 Time, u16 length)
{
    	u32 	Address;
    	u32 	TimeCount = 0;
    	u8	Buffer[5];
    	s16	Sector;
    	s16	Step;

	//检查类型
	if(Type >= REGISTER_TYPE_MAX)
	{
		return ERROR;
	}

    	//检查字节长度
    	if(length != Register[Type].StepLen-5)//4字节时间+1字节校验
    	{
        	return ERROR;
    	}

    	//判断当前步数时间是否大于上一步数时间
    	if((Register[Type].CurrentStep > 0)||
    		(Register[Type].CurrentSector > Register[Type].StartSector)||
    		(Register[Type].LoopFlag > 0)) //存储区已有数据
    	{
        	Sector = Register[Type].CurrentSector;
        	Step = Register[Type].CurrentStep;
        	Step--;
        	if(Step < 0)
        	{
            		Step = Register[Type].SectorStep-1;
            		Sector--;
            		if(Sector < Register[Type].StartSector)
            		{
                		Sector = Register[Type].EndSector-1;
            		}
        	}
        	Address = Sector*FLASH_ONE_SECTOR_BYTES+Step*Register[Type].StepLen;
        	sFLASH_ReadBuffer(Buffer,Address,4);
		TimeCount = Register_BytesToInt(Buffer);
		/*
		TimeCount = 0;
		TimeCount |= Buffer[0] << 24;
		TimeCount |= Buffer[1] << 16;
		TimeCount |= Buffer[2] << 8;
		TimeCount |= Buffer[3];
		*/
        	if(Time <= TimeCount)
        	{
            	return ERROR;	
        	}                
    	}
    	return SUCCESS;
}
/*********************************************************************
//函数名称	:Register_BytesToInt(u8 *pBuffer)
//功能		:四个字节流转换成一个整形数据，大端模式，高字节在前
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
u32 Register_BytesToInt(u8 *pBuffer)
{
	u32	a = 0;
	a |= *pBuffer << 24;
	a |= *(pBuffer+1) << 16;
	a |= *(pBuffer+2) << 8;
	a |= *(pBuffer+3);
	
	return a;
}
/*********************************************************************
//函数名称	:Register_IntToBytes(u8 *pBuffer, u32 a)
//功能		:一个整形数据转换成四个字节流，大端模式，高字节在前
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
u8 Register_IntToBytes(u8 *pBuffer, u32 a)
{
	*pBuffer = (a&0xff000000) >> 24;
	*(pBuffer+1) = (a&0xff0000) >> 16;
	*(pBuffer+2) = (a&0xff00) >> 8;
	*(pBuffer+3) = (a&0xff);
	return 4;
}
/*********************************************************************
//函数名称	:Register_BytesToShortInt(u8 *pBuffer)
//功能		:2个字节流转换成一个短整形数据，大端模式，高字节在前
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
u16 Register_BytesToShortInt(u8 *pBuffer)
{
	u16	a = 0;
	a |= *pBuffer << 8;
	a |= *(pBuffer+1);
	
	return a;
}
/*********************************************************************
//函数名称	:Register_ShortIntToBytes(u8 *pBuffer, u16 a)
//功能		:一个短整形数据转换成2个字节流，大端模式，高字节在前
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
u8 Register_ShortIntToBytes(u8 *pBuffer, u16 a)
{
	*pBuffer = (a&0xff00) >> 8;
	*(pBuffer+1) = (a&0xff);
	return 2;
}
/*********************************************************************
//函数名称	:Register_EraseRecorderData(void)
//功能		:擦除行驶记录仪所有的存储数据
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void Register_EraseRecorderData(void)
{
	//擦除
	Register_EraseOneArea(REGISTER_TYPE_SPEED);
	Register_EraseOneArea(REGISTER_TYPE_POSITION);
	Register_EraseOneArea(REGISTER_TYPE_DOUBT);
	Register_EraseOneArea(REGISTER_TYPE_OVER_TIME);
	Register_EraseOneArea(REGISTER_TYPE_DRIVER);
	Register_EraseOneArea(REGISTER_TYPE_POWER_ON);
	Register_EraseOneArea(REGISTER_TYPE_PRAMATER);
	Register_EraseOneArea(REGISTER_TYPE_SPEED_STATUS);
}
/*********************************************************************
//函数名称	:Register_EraseOneArea(u8 Type)
//功能		:擦除某个类型的存储区域
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:
*********************************************************************/
void Register_EraseOneArea(u8 Type)
{
	u16 i;
    	u32 Address;

	if(Type >= REGISTER_TYPE_MAX)
	{
		return ;
	}
    	for(i=Register[Type].StartSector; i<Register[Type].EndSector; i++)
    	{
        	Address = i*FLASH_ONE_SECTOR_BYTES;
        	sFLASH_EraseSector(Address);
                IWDG_ReloadCounter();//喂狗
    	}
	Register_CheckOneArea(Type);//为什么以前这行屏蔽了？dxl,2014.3.15，是需要消耗很多时间？？
}
/*********************************************************************
//函数名称	:Register_GetSubpacketData
//功能		:获取分包数据
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:该函数用于北斗功能测试，北斗功能测试要求上传所有的行驶
//              :记录仪数据，从最新时间开始上传。分包规则：每包最多512字节，
//              :（除位置信息是666字节外）PacketNum从1开始
*********************************************************************/
u16 Register_GetSubpacketData(u8 *pBuffer,u8 Type, u16 PacketNum)
{
        u32 Address;
        s16 Sector;
        s16 Step;
        u8  Buffer[REGISTER_MAX_STEP_LEN];
        u8  i;
        u8  *p = NULL;
        u16 length;
        u32 TimeCount;
        TIME_T  tt;
        
        if((Type >= REGISTER_TYPE_MAX)||(0==Type))
        {
                return 0;
        }
        p = pBuffer;
        length = 0;
        Register_GetSubpacketStoreStep(&Sector,&Step,Type,PacketNum);
        
        if((0==Sector)&&(0==Step))
        {
                return 0;
        }
        
        switch(Type)
        {        
        case REGISTER_TYPE_SPEED://48小时速度//modify by joneming
        {
                for(i=0; i<RegisterSubpacketList[Type]; i++)
                {
                        //获取一步数据
                        Address = Sector*FLASH_ONE_SECTOR_BYTES+Step*Register[Type].StepLen;
                        sFLASH_ReadBuffer(Buffer,Address,Register[Type].StepLen); 
                        if(0 == i)
                        {
                                //开始时间
                              TimeCount = Register_GetOldestTime(Type);
                              Gmtime(&tt, TimeCount);
                              Public_ConvertTimeToBCDEx(tt,p);
                              p += 6;
                                //结束时间
                              TimeCount = Register_GetNewestTime(Type);
                              Gmtime(&tt, TimeCount);
                              Public_ConvertTimeToBCDEx(tt,p);
                              p += 6;
                               //最大单位数据块
                              *p++ = 0;
                              *p++ = 4;//每包包含4个数据块
                              
                              length += 14;
                      }
                      #if (REGISTER_SPEED_STEP_LEN == 11)//modify by joneming
                      if(0 == i%RegisterBlockList[Type])
                      {    
                                //每个数据块的开始时间
                              TimeCount = 0;
                              TimeCount |= Buffer[0] << 24;
                              TimeCount |= Buffer[1] << 16;
                              TimeCount |= Buffer[2] << 8;
                              TimeCount |= Buffer[3];
                              Gmtime(&tt, TimeCount);
                              *p++ = Public_HEX2BCD(tt.year);
                              *p++ = Public_HEX2BCD(tt.month);
                              *p++ = Public_HEX2BCD(tt.day);
                              *p++ = Public_HEX2BCD(tt.hour);
                              *p++ = Public_HEX2BCD(tt.min);
                              *p++ = 0;//0秒开始
                              
                              length += 6;
                      }
                      #endif                      
                      //拷贝至目标缓冲
                      memcpy(p,Buffer+4,RegisterCollectLen[Type]);
                      p += RegisterCollectLen[Type];
                      length += RegisterCollectLen[Type];
                      //步数加1
                      Step++;
                      if(Step >= Register[Type].SectorStep)
                      {
                              Step = 0;
                              Sector++;
                              if(Sector >= Register[Type].EndSector)
                              {
                                      Sector = Register[Type].StartSector;
                              }
                      }
                } 
                break;                
          }
          case REGISTER_TYPE_POSITION://360小时块位置
          {
                for(i=0; i<RegisterSubpacketList[Type]; i++)
                {
                        Address = Sector*FLASH_ONE_SECTOR_BYTES+Step*Register[Type].StepLen;
                        sFLASH_ReadBuffer(Buffer,Address,Register[Type].StepLen); 
                        if(0 == i%RegisterBlockList[Type])
                        {
                                //开始时间
                              TimeCount = Register_GetOldestTime(Type);
                              Gmtime(&tt, TimeCount);
                              *p++ = Public_HEX2BCD(tt.year);
                              *p++ = Public_HEX2BCD(tt.month);
                              *p++ = Public_HEX2BCD(tt.day);
                              *p++ = Public_HEX2BCD(tt.hour);
                              *p++ = 0;//分,秒为0
                              *p++ = 0;
                              length += 6;
                      }
                      memcpy(p,Buffer+4,RegisterCollectLen[Type]);
                      p += RegisterCollectLen[Type];
                      length += RegisterCollectLen[Type];
                       //步数加1
                      Step++;
                      if(Step >= Register[Type].SectorStep)
                      {
                              Step = 0;
                              Sector++;
                              if(Sector >= Register[Type].EndSector)
                              {
                                      Sector = Register[Type].StartSector;
                              }
                      }
                }
                break;
          }
        case REGISTER_TYPE_DOUBT://疑点数据
        case REGISTER_TYPE_OVER_TIME://超时记录
        case REGISTER_TYPE_DRIVER://驾驶人身份
        case REGISTER_TYPE_POWER_ON://供电记录
        case REGISTER_TYPE_PRAMATER://修改参数
        case REGISTER_TYPE_SPEED_STATUS://速度日志
          {
                for(i=0; i<RegisterSubpacketList[Type]; i++)
                {
                        Address = Sector*FLASH_ONE_SECTOR_BYTES+Step*Register[Type].StepLen;
                        sFLASH_ReadBuffer(Buffer,Address,Register[Type].StepLen); 
                        memcpy(p,Buffer+4,RegisterCollectLen[Type]);
                        p += RegisterCollectLen[Type];
                        length += RegisterCollectLen[Type];
                         //步数加1
                        Step++;
                        if(Step >= Register[Type].SectorStep)
                        {
                              Step = 0;
                              Sector++;
                              if(Sector >= Register[Type].EndSector)
                              {
                                      Sector = Register[Type].StartSector;
                              }
                        }
                }
                break;
          }
          
        default :break;
        }
       
        
        return length;
}
/*********************************************************************
//函数名称	:Register_GetSubpacketStoreStep
//功能		:获取某个分包的存储步数
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:该函数用于北斗功能测试，北斗功能测试要求上传所有的行驶
//              :记录仪数据，从最新时间开始上传。分包规则：每包最多512字节，
//              :（除位置信息是666字节外），最新时间分包序号为1
*********************************************************************/
void Register_GetSubpacketStoreStep(s16 *Sector, s16 *Step,u8 Type, u16 PacketNum)
{
      u16 TotalList;
      u16 i;
      s16 ReadSector;
      s16 ReadStep;
      
      TotalList = Register_GetSubpacketTotalList(Type);
      if((RegisterSubpacketList[Type]*PacketNum)> TotalList)
      { 
            *Sector = 0;
            *Step = 0;
            return ;
      }
      ReadSector = Register[Type].CurrentSector;
      ReadStep = Register[Type].CurrentStep;
      for(i=0; i<PacketNum; i++)
      {
              ReadStep -= RegisterSubpacketList[Type];
              if(ReadStep < 0)
              {
                    ReadStep = ReadStep+Register[Type].SectorStep;
                    ReadSector--;
                    if(ReadSector < Register[Type].StartSector)
                    {
                          ReadSector = Register[Type].EndSector-1;
                    }
              }
      }
      *Sector = ReadSector;
      *Step = ReadStep;
}
/*********************************************************************
//函数名称	:Register_GetSubpacketTotalList
//功能		:获取分包总条数
//输入		:
//输出		:
//使用资源	:
//全局变量	:   
//调用函数	:
//中断资源	:  
//返回		:
//备注		:该函数用于北斗功能测试，北斗功能测试要求上传所有的行驶
//              :记录仪数据，从最新时间开始上传。分包规则：每包最多512字节，
//              :（除位置信息是666字节外），最新时间分包序号为1
*********************************************************************/
u16 Register_GetSubpacketTotalList(u8 Type)
{
        u16   TotalList = 0;
        
        if((Type == 0)||(Type >= REGISTER_TYPE_MAX))
        {
              return 0;
        }
        
        if(0 == Register[Type].LoopFlag)//没有循环存储
        {
                TotalList = (Register[Type].CurrentSector-Register[Type].OldestSector)*Register[Type].SectorStep
                  +Register[Type].CurrentStep;
        }
        else//进入循环存储
        {
                TotalList = (Register[Type].CurrentSector-Register[Type].StartSector)*Register[Type].SectorStep
                  +Register[Type].CurrentStep
                    +(Register[Type].EndSector-Register[Type].OldestSector)*Register[Type].SectorStep;
        }
        return  TotalList;
}
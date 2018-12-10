/********************************************************************
//��Ȩ˵��	:Shenzhen E-EYE Co. Ltd., Copyright 2009- All Rights Reserved.
//�ļ�����	:Register.c		
//����		:ʵ����ʻ��¼�����ݼ�¼�Ͳ�ѯ����
//�汾��	:
//������	:dxl
//����ʱ��	:2013.3
//�޸���	:
//�޸�ʱ��	: 
//�޸ļ�Ҫ˵��	:
//��ע		:
***********************************************************************/
//***************�����ļ�*****************
#include "stm32f2xx.h"
#include "stm32f2xx_lib.h"
#include "include.h"
//****************�궨��****************
//***************��������***************
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

//*****************��������****************
REGISTER_STRUCT Register[REGISTER_TYPE_MAX];

//****************��������*****************
/*********************************************************************
//��������	:Register_CheckArea(void)
//����		:������д洢����ȷ�������������������������²���
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:��ȷ����0�������ʾ�ĸ����͵��Լ����
//��ע		:���ϵ�����whileǰ����øú�����
//		:ֻ�иú����ȱ����ú���ܱ�֤�����Ķ���д����������������ȷ
*********************************************************************/
u8 Register_CheckArea(void)
{
  /*
	//�Լ���ʻ�ٶ�����
	if(SUCCESS != Register_CheckOneArea(REGISTER_TYPE_SPEED))
	{
		if(SUCCESS != Register_CheckOneArea(REGISTER_TYPE_SPEED))//�ټ�һ��
		{
			Register_EraseOneArea(REGISTER_TYPE_SPEED);
			return REGISTER_TYPE_SPEED;
		}
	}
	//�Լ�λ����Ϣ����
	if(SUCCESS != Register_CheckOneArea(REGISTER_TYPE_POSITION))
	{
		if(SUCCESS != Register_CheckOneArea(REGISTER_TYPE_POSITION))
		{
			Register_EraseOneArea(REGISTER_TYPE_POSITION);
			return REGISTER_TYPE_POSITION;
		}
	}
  */
	//�Լ�λ����Ϣ����
	if(SUCCESS != Register_CheckOneArea(REGISTER_TYPE_DOUBT))
	{
		if(SUCCESS != Register_CheckOneArea(REGISTER_TYPE_DOUBT))
		{
			Register_EraseOneArea(REGISTER_TYPE_DOUBT);
			return REGISTER_TYPE_DOUBT;
		}
	}
	//�Լ쳬ʱ��ʻ����
	if(SUCCESS != Register_CheckOneArea(REGISTER_TYPE_OVER_TIME))
	{
		if(SUCCESS != Register_CheckOneArea(REGISTER_TYPE_OVER_TIME))
		{
			Register_EraseOneArea(REGISTER_TYPE_OVER_TIME);
			return REGISTER_TYPE_OVER_TIME;
		}
	}
	//�Լ��ʻ���������
	if(SUCCESS != Register_CheckOneArea(REGISTER_TYPE_DRIVER))
	{
		if(SUCCESS != Register_CheckOneArea(REGISTER_TYPE_DRIVER))
		{
			Register_EraseOneArea(REGISTER_TYPE_DRIVER);
			return REGISTER_TYPE_DRIVER;
		}
	}
	//�Լ��ⲿ��������
	if(SUCCESS != Register_CheckOneArea(REGISTER_TYPE_POWER_ON))
	{
		if(SUCCESS != Register_CheckOneArea(REGISTER_TYPE_POWER_ON))
		{
			Register_EraseOneArea(REGISTER_TYPE_POWER_ON);
			return REGISTER_TYPE_POWER_ON;
		}
	}
	//�Լ��ⲿ��������
	if(SUCCESS != Register_CheckOneArea(REGISTER_TYPE_PRAMATER))
	{
		if(SUCCESS != Register_CheckOneArea(REGISTER_TYPE_PRAMATER))
		{
			Register_EraseOneArea(REGISTER_TYPE_PRAMATER);
			return REGISTER_TYPE_PRAMATER;
		}
	}
	//�Լ��ٶ�״̬����
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
//��������	:Register_CheckOneArea(void)
//����		:���ĳһ���洢����
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:��ȷ����SUCCESS�����󷵻�ERROR
//��ע		:
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

    //*******************����������**************************

	if(Type >= REGISTER_TYPE_MAX)
	{
		return ERROR;
	}

    //*********************��ʼ������*************************
	Register[Type].StartSector = RegisterStartSector[Type];
	Register[Type].EndSector = RegisterEndSector[Type];
	Register[Type].StepLen = RegisterStepLen[Type];
	Register[Type].SectorStep = FLASH_ONE_SECTOR_BYTES/RegisterStepLen[Type];
	Register[Type].OldestSector = RegisterStartSector[Type];
	Register[Type].CurrentSector = RegisterStartSector[Type];
	Register[Type].CurrentStep = 0;
	Register[Type].LoopFlag = 0;

    //********�����ʷ�켣���������ݰ������ڵ�����************
    	MinTimeCount = 0xFFFFFFFF;
    	MaxTimeCount = 0;
    	//���ÿ�����������ݰ�����ʼ����--->��������
    	for(Sector=Register[Type].StartSector; Sector<Register[Type].EndSector; Sector++)
    	{
                IWDG_ReloadCounter();//ι��
        	for(Step=0; Step<Register[Type].SectorStep; Step++)
        	{
			//��ȡ����
            		Address = Sector*FLASH_ONE_SECTOR_BYTES + Step*Register[Type].StepLen;
            		sFLASH_ReadBuffer(Buffer,Address,Register[Type].StepLen);

			//ȷ���������������²���
			TimeCount = Register_BytesToInt(Buffer);
			if(TimeCount != 0xFFFFFFFF)	//�ҵ���Чʱ���ǩ
            		{
			/*
			TimeCount = 0;
			TimeCount |= Buffer[0] << 24;
			TimeCount |= Buffer[1] << 16;
			TimeCount |= Buffer[2] << 8;
			TimeCount |= Buffer[3];
			*/
				//У������
				VerifySum = Register_GetVerifySum(Buffer,Register[Type].StepLen-1);//����==ʱ���ֽ�4+����n�ֽ�+У��1�ֽ�
				if(VerifySum != Buffer[Register[Type].StepLen-1])
				{
					//����ֱ����һ��
				}
                                else
                                {
                		        Gmtime(&tt, TimeCount);
                                        if(SUCCESS == CheckTimeStruct(&tt))
                                        {
                		                if(TimeCount < MinTimeCount)
                		                {
                    			                MinTimeCount = TimeCount;
                    			                Register[Type].OldestSector = Sector; //��������
                		                }
                		                if(TimeCount > MaxTimeCount)
                		                {
                    			                MaxTimeCount = TimeCount;
                    			                Register[Type].CurrentStep = Step+1;//���²���
                    			                Register[Type].CurrentSector = Sector;//����������
                		                }
                                        }
                                        else
                                        {
                                                //����ֱ����һ��
                                        }
                                }
            		}
            		else	//û���ҵ���Чʱ���ǩ������ѭ��������һ������
            		{
                		break; 
            		}
    		}
    	}
    	//�жϵ�ǰ���Ƿ�ﵽ�����߽�
    	if(Register[Type].CurrentStep >= Register[Type].SectorStep)
    	{
        	Register[Type].CurrentStep = 0;
        	Register[Type].CurrentSector++;
        	if(Register[Type].CurrentSector >= Register[Type].EndSector)
        	{
            		Register[Type].CurrentSector = Register[Type].StartSector;	
        	}
    	}
    	//�����������һ���ĵ�ַ���ж��Ƿ����ѭ���洢
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
//��������	:Register_CheckOneArea2(void)
//����		:���ĳһ���洢����
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:���ؽ�����־��2Ϊ��������1Ϊ�������������0Ϊδ������
//��ע		:ֻ����λ�ü�¼���ٶȼ�¼�����øú�����Ӧ����Ҫ��ֻ֤��
//              :����ֵΪ1ʱ�������Լ������������������д����
//              :REGISTER_TYPE_SPEED��REGISTER_TYPE_POSITION���������͵ļ��ɵ��ô˺���
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

	if(Type <= REGISTER_TYPE_POSITION)//ֻ��REGISTER_TYPE_SPEED��REGISTER_TYPE_POSITION���������͵ļ��ɵ��ô˺���
	{
		
	}
        else
        {
                return 2;
        }

        if(0 == Sector[Type])//��1�ν���ú���
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
    	
    	//һ�μ��һ�����������ݰ�
        IWDG_ReloadCounter();//ι��
        for(Step=0; Step<Register[Type].SectorStep; Step++)
        {
		//��ȡ����
            	Address = Sector[Type]*FLASH_ONE_SECTOR_BYTES + Step*Register[Type].StepLen;
            	sFLASH_ReadBuffer(Buffer,Address,Register[Type].StepLen);

		//ȷ���������������²���
		TimeCount = Register_BytesToInt(Buffer);
		if(TimeCount != 0xFFFFFFFF)	//�ҵ���Чʱ���ǩ
            	{
			//Gmtime(&tt, TimeCount);
			/*
			TimeCount = 0;
			TimeCount |= Buffer[0] << 24;
			TimeCount |= Buffer[1] << 16;
			TimeCount |= Buffer[2] << 8;
			TimeCount |= Buffer[3];
			*/
			//У������
			VerifySum = Register_GetVerifySum(Buffer,Register[Type].StepLen-1);//����==ʱ���ֽ�4+����n�ֽ�+У��1�ֽ�
			if(VerifySum != Buffer[Register[Type].StepLen-1])
			{
				//����ֱ����һ��
			}
                        else
                        {
                	        Gmtime(&tt, TimeCount);
                                if(SUCCESS == CheckTimeStruct(&tt))
                                {
                	                if(TimeCount < MinTimeCount[Type])
                	                {
                    		                MinTimeCount[Type] = TimeCount;
                    		                Register[Type].OldestSector = Sector[Type]; //��������
                	                }
                	                if(TimeCount > MaxTimeCount[Type])
                	                {
                    		                MaxTimeCount[Type] = TimeCount;
                    		                Register[Type].CurrentStep = Step+1;//���²���
                    		                Register[Type].CurrentSector = Sector[Type];//����������
                	                }
                                }
                                else
                                {
                                        //����ֱ����һ��
                                }
                        }
            	}
            	else	//û���ҵ���Чʱ���ǩ������ѭ��������һ������
            	{
                	break; 
            	}
    	}
        Sector[Type]++;
        if(Sector[Type] >= Register[Type].EndSector)
        {
                //�жϵ�ǰ���Ƿ�ﵽ�����߽�
    	        if(Register[Type].CurrentStep >= Register[Type].SectorStep)
    	        {
        	        Register[Type].CurrentStep = 0;
        	        Register[Type].CurrentSector++;
        	        if(Register[Type].CurrentSector >= Register[Type].EndSector)
        	        {
            		        Register[Type].CurrentSector = Register[Type].StartSector;	
        	        }
    	        }
    	        //�����������һ���ĵ�ַ���ж��Ƿ����ѭ���洢
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
//��������	:Register_Write(u8 Type,u8 *pBuffer, u8 length)
//����		:��¼һ��ĳ�����͵���Ϣ
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:��ȷ����SUCCESS�����󷵻�ERROR
//��ע		:
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
	//����ǰʱ��
	TimeCount = RTC_GetCounter();
	//}
	//��������ȷ��
	if(ERROR == Register_CheckPram(Type,TimeCount,length))
	{
		return ERROR;
	}
        
	
    	//�����д�������Ƿ�ȫΪ0XFF
    	Address = Register[Type].CurrentSector*FLASH_ONE_SECTOR_BYTES + Register[Type].CurrentStep*Register[Type].StepLen;
    	sFLASH_ReadBuffer(Buffer,Address,Register[Type].StepLen);
    	for(i=0; i<Register[Type].StepLen; i++)
    	{
        	if(0xff != Buffer[i])
        	{
            		//������������д�뵽��0��
            		sFLASH_EraseSector(Address);
            		Register[Type].CurrentStep = 0;
            		for(j=0; j<200; j++);
            		Address = Register[Type].CurrentSector*FLASH_ONE_SECTOR_BYTES;
            		break;
        	}
    	}	
	
	//дʱ���볤��,Buffer[0]-Buffer[3]�Ǵ洢�����ݣ�Buffer[4]-Buffer[7]�Ƕ�ȡ������
	Register_IntToBytes(Buffer, TimeCount);
	sFLASH_WriteBuffer(Buffer, Address, 4);
    	sFLASH_ReadBuffer(Buffer+4,Address,4);
    	for(i=0; i<4; i++)
    	{
        	if(Buffer[i] != Buffer[i+4])
        	{
            		//��һ������������������������
            		sFLASH_EraseSector(Address);
            		Register[Type].CurrentStep = 0;
            		return ERROR;
        	}
    	}
	Address += 4;
	VerifySum = Register_GetVerifySum(Buffer, 4);

    	//д���ݣ�����ȡ�����Ƚ�
    	sFLASH_WriteBuffer(pBuffer, Address, length);
    	sFLASH_ReadBuffer(Buffer,Address,length);
    	for(i=0; i<length; i++)
    	{
        	if(*(pBuffer+i) != Buffer[i])
        	{
            		//��һ������������������������
            		sFLASH_EraseSector(Address);
            		Register[Type].CurrentStep = 0;
            		return ERROR;
        	}
    	}
	Address += length;

	//дУ����
	VerifySum += Register_GetVerifySum(pBuffer, length);
	sFLASH_WriteBuffer(&VerifySum, Address, 1);
    	sFLASH_ReadBuffer(Buffer,Address,1);
	if(Buffer[0] != VerifySum)
	{
		//��һ������������������������
            	sFLASH_EraseSector(Address);
            	Register[Type].CurrentStep = 0;
            	return ERROR;
	}

    	//��ǰ������1
    	Register[Type].CurrentStep++;
    	if(Register[Type].CurrentStep  >= Register[Type].SectorStep)
    	{
       		Register[Type].CurrentStep = 0;	
        	Register[Type].CurrentSector++;	//��ǰ��������һ��
        	//�жϵ�ǰ�����Ƿ�ﵽĩβ
        	if(Register[Type].CurrentSector >= Register[Type].EndSector)
        	{
           		Register[Type].CurrentSector = Register[Type].StartSector;//ָ����ʼ����
            		Register[Type].LoopFlag = 1;
        	}
        	//������������
        	if(Register[Type].LoopFlag > 0)
        	{
            		Register[Type].OldestSector++;
            		if(Register[Type].OldestSector >= Register[Type].EndSector)
            		{
                		Register[Type].OldestSector = Register[Type].StartSector;//ָ����ʼ����
            		}
        	}
		Address = Register[Type].CurrentSector*FLASH_ONE_SECTOR_BYTES;
        	sFLASH_EraseSector(Address);
    	}	
    	return SUCCESS;
}
/*********************************************************************
//��������	:Register_Write2(u8 Type,u8 *pBuffer, u8 length, u32 Time)
//����		:��¼һ��ĳ�����͵���Ϣ
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:��ȷ����SUCCESS�����󷵻�ERROR
//��ע		:
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
	//����ǰʱ��
	//TimeCount = RTC_GetCounter();
        TimeCount = Time;
	//}
	//��������ȷ��T
	if(ERROR == Register_CheckPram(Type,TimeCount,length))
	{
		return ERROR;
	}
        
	
    	//�����д�������Ƿ�ȫΪ0XFF
    	Address = Register[Type].CurrentSector*FLASH_ONE_SECTOR_BYTES + Register[Type].CurrentStep*Register[Type].StepLen;
    	sFLASH_ReadBuffer(Buffer,Address,Register[Type].StepLen);
    	for(i=0; i<Register[Type].StepLen; i++)
    	{
        	if(0xff != Buffer[i])
        	{
            		//������������д�뵽��0��
            		sFLASH_EraseSector(Address);
            		Register[Type].CurrentStep = 0;
            		for(j=0; j<200; j++);
            		Address = Register[Type].CurrentSector*FLASH_ONE_SECTOR_BYTES;
            		break;
        	}
    	}	
	
	//дʱ���볤��,Buffer[0]-Buffer[3]�Ǵ洢�����ݣ�Buffer[4]-Buffer[7]�Ƕ�ȡ������
	Register_IntToBytes(Buffer, TimeCount);
	sFLASH_WriteBuffer(Buffer, Address, 4);
    	sFLASH_ReadBuffer(Buffer+4,Address,4);
    	for(i=0; i<4; i++)
    	{
        	if(Buffer[i] != Buffer[i+4])
        	{
            		//��һ������������������������
            		sFLASH_EraseSector(Address);
            		Register[Type].CurrentStep = 0;
            		return ERROR;
        	}
    	}
	Address += 4;
	VerifySum = Register_GetVerifySum(Buffer, 4);

    	//д���ݣ�����ȡ�����Ƚ�
    	sFLASH_WriteBuffer(pBuffer, Address, length);
    	sFLASH_ReadBuffer(Buffer,Address,length);
    	for(i=0; i<length; i++)
    	{
        	if(*(pBuffer+i) != Buffer[i])
        	{
            		//��һ������������������������
            		sFLASH_EraseSector(Address);
            		Register[Type].CurrentStep = 0;
            		return ERROR;
        	}
    	}
	Address += length;

	//дУ����
	VerifySum += Register_GetVerifySum(pBuffer, length);
	sFLASH_WriteBuffer(&VerifySum, Address, 1);
    	sFLASH_ReadBuffer(Buffer,Address,1);
	if(Buffer[0] != VerifySum)
	{
		//��һ������������������������
            	sFLASH_EraseSector(Address);
            	Register[Type].CurrentStep = 0;
            	return ERROR;
	}

    	//��ǰ������1
    	Register[Type].CurrentStep++;
    	if(Register[Type].CurrentStep  >= Register[Type].SectorStep)
    	{
       		Register[Type].CurrentStep = 0;	
        	Register[Type].CurrentSector++;	//��ǰ��������һ��
        	//�жϵ�ǰ�����Ƿ�ﵽĩβ
        	if(Register[Type].CurrentSector >= Register[Type].EndSector)
        	{
           		Register[Type].CurrentSector = Register[Type].StartSector;//ָ����ʼ����
            		Register[Type].LoopFlag = 1;
        	}
        	//������������
        	if(Register[Type].LoopFlag > 0)
        	{
            		Register[Type].OldestSector++;
            		if(Register[Type].OldestSector >= Register[Type].EndSector)
            		{
                		Register[Type].OldestSector = Register[Type].StartSector;//ָ����ʼ����
            		}
        	}
		Address = Register[Type].CurrentSector*FLASH_ONE_SECTOR_BYTES;
        	sFLASH_EraseSector(Address);
    	}	
    	return SUCCESS;
}
/*********************************************************************
//��������	:Register_Read(u8 Type,u8 *pBuffer, TIME_T StartTime, TIME_T EndTime, u16 MaxBlock)
//����		:��ȡָ��ʱ��Σ�ָ��������ݿ�ļ�¼����
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:��ȡ�����ݳ���
//��ע		:
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

	//ת��ʱ��
	StartTimeCount = ConverseGmtime(&StartTime);	
	EndTimeCount = ConverseGmtime(&EndTime);

	//�ж�ʱ���Ⱥ��ϵ
	if(StartTimeCount >= EndTimeCount)
	{
		return length;
	}	

	if(Type == REGISTER_TYPE_SPEED)
	{
		StartTimeCount -= StartTime.sec;//����0
		EndTimeCount -= EndTime.sec;//����0	
	}
	else if(Type == REGISTER_TYPE_POSITION)	
	{
		StartTimeCount -= StartTime.sec;//����0
		StartTimeCount -= StartTime.min*60;//����0
		EndTimeCount -= EndTime.sec;//����0
		EndTimeCount -= EndTime.min*60;//����0
	}
	p = pBuffer;
	IncludeFlag = 0;
	//ȷ��������ʼ���ͽ�����
	ttCount = Register_Search(Type, EndTimeCount, &Sector, &Step);//��ʼλ��Sector,Step,�ӽ���ʱ����ǰ��
	if(ttCount > EndTimeCount)//û������������,��������Ҫ�ٲ���
	{
		return length;
	}
	IncludeCount = Register_Search(Type, StartTimeCount, &EndSector, &EndStep);//����λ��EndSector,EndStep
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
			if((Sector == EndSector)&&(Step == EndStep))//ֻ��һ����Ĭ��Ϊ0�顣
			{
				return length;
			}            
            #if (REGISTER_SPEED_STEP_LEN == 11)//modify by joneming
            if(REGISTER_TYPE_SPEED == Type)
			{
				TimeCoeff = 60;//1����һ�����ݿ�
			}
			else
            #endif
			{
				TimeCoeff = 3600;//1Сʱһ�����ݿ�
			}            
			ttCount = EndTimeCount - TimeCoeff;
			for(;;)
			{
				ttCount2 = Register_Search(Type, ttCount, &Sector, &Step);
				Gmtime(&test2, ttCount2);
				//����һ��,dxl,2013.5.10
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
					//����һ������Ϊ���ҷ��ص���������С�ڻ���ڸ���ֵ
					flag = 0;
					if(ttCount2 < ttCount)
					{
						for(;;)
						{
							//����һ��
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
						//����ʽҪ��д�뿪ʼʱ��
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
						//д�벹����ֽ�0xff
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
						//д������
						for(;;)
						{
							//��ȡ��д������
							Address = Sector*FLASH_ONE_SECTOR_BYTES+Step*Register[Type].StepLen;
							sFLASH_ReadBuffer(Buffer,Address,Register[Type].StepLen);
							ttCount3 = Register_BytesToInt(Buffer);
							Gmtime(&test3, ttCount3);
							//д�벹����ֽ�0xff
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
							//����һ��
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
				if((ttCount >= StartTimeCount)&&(ttCount <= EndTimeCount))//��һ����������������
				{
					//��ȡ��У������
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
				//��ȡ��У������
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
				//����һ��
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
				//�ж��Ƿ񵽴���ֹ��
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
//��������	:Register_GetNewestTime(u8 Type)
//����		:��ȡ��ǰ�洢�����²�����ʱ��
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:4�ֽڱ�ʾ��ʱ��
//��ע		:
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
//��������	:Register_GetOldestTime(u8 Type)
//����		:��ȡ��ǰ�洢�����ϲ�����ʱ��
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:4�ֽڱ�ʾ��ʱ��
//��ע		:
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
//��������	:Register_Search(u8 Type,u32 Time, s16 *Sector, s16 *Step)
//����		:����ĳһ����ʱ��Ĵ洢��Ϣ
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:ʵ�ʲ��ҵõ���ʱ�䣬��ʱ�����ǲ�����Time�����Ǵ洢��ʱ�䶼�Ǵ���Time��
//��ע		:���ҵĽṹ����Sector��Step��
*********************************************************************/
u32 Register_Search(u8 Type, u32 Time, s16 *Sector, s16 *Step)
{
	s16	SearchSector;
	s16	SearchStep;
	u32	Address;
	u8	Buffer[5];
	u8	flag = 0;
	u32	TimeCount;
	
	
                IWDG_ReloadCounter();//ι��

	//������������ʼ����
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

	//ȷ������ʱ���������ĸ�����
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
			if(SearchSector == Register[Type].OldestSector)//�ﵽ��������
			{
				//break;dxl,2013.6.7,����Ҫ��������,û������������
                                /*********************************************///���ݵ�һ������ַ�Լ�ʱ�� myh 130624
                                Address = SearchSector*FLASH_ONE_SECTOR_BYTES;
                                sFLASH_ReadBuffer(Buffer,Address,4);
                                TimeCount = Register_BytesToInt(Buffer);
                                /*********************************************/
                                *Sector = SearchSector;
				*Step = SearchStep;                                        
                                break;                                         //ԭ��Ϊreturn TimeCount���˳�ѭ�����в���ȷ�� myh 130624   
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
	//ȷ������ʱ���������Ĳ�
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
				if(SearchStep >= Register[Type].CurrentStep)//dxl,2013.6.7��һ�����ǵ�ǰ������
				{
					flag = 1;//dxl,2013.6.7,��Ҫ��һ��
					break;
				}
			}
			else
			{
				SearchStep++;
				//if(SearchStep >= Register[Type].SectorStep-1)
				if(SearchStep >= Register[Type].SectorStep)//dxl,2013.6.7��һ��������һ��������
				{
					flag = 1;//dxl,2013.6.7,��Ҫ��һ��
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
//��������	:Register_GetVerifySum(u8 *pBuffer, u16 length)
//����		:��ȡ�����ַ������ݵ�У���
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
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
//��������	:Register_CheckPram(u8 Type, u32 Time, u8 length)
//����		:�����������ĺϷ���
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:�ɹ�����SUCCESS,ʧ�ܷ���ERROR
//��ע		:
*********************************************************************/
ErrorStatus Register_CheckPram(u8 Type, u32 Time, u16 length)
{
    	u32 	Address;
    	u32 	TimeCount = 0;
    	u8	Buffer[5];
    	s16	Sector;
    	s16	Step;

	//�������
	if(Type >= REGISTER_TYPE_MAX)
	{
		return ERROR;
	}

    	//����ֽڳ���
    	if(length != Register[Type].StepLen-5)//4�ֽ�ʱ��+1�ֽ�У��
    	{
        	return ERROR;
    	}

    	//�жϵ�ǰ����ʱ���Ƿ������һ����ʱ��
    	if((Register[Type].CurrentStep > 0)||
    		(Register[Type].CurrentSector > Register[Type].StartSector)||
    		(Register[Type].LoopFlag > 0)) //�洢����������
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
//��������	:Register_BytesToInt(u8 *pBuffer)
//����		:�ĸ��ֽ���ת����һ���������ݣ����ģʽ�����ֽ���ǰ
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
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
//��������	:Register_IntToBytes(u8 *pBuffer, u32 a)
//����		:һ����������ת�����ĸ��ֽ��������ģʽ�����ֽ���ǰ
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
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
//��������	:Register_BytesToShortInt(u8 *pBuffer)
//����		:2���ֽ���ת����һ�����������ݣ����ģʽ�����ֽ���ǰ
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
*********************************************************************/
u16 Register_BytesToShortInt(u8 *pBuffer)
{
	u16	a = 0;
	a |= *pBuffer << 8;
	a |= *(pBuffer+1);
	
	return a;
}
/*********************************************************************
//��������	:Register_ShortIntToBytes(u8 *pBuffer, u16 a)
//����		:һ������������ת����2���ֽ��������ģʽ�����ֽ���ǰ
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
*********************************************************************/
u8 Register_ShortIntToBytes(u8 *pBuffer, u16 a)
{
	*pBuffer = (a&0xff00) >> 8;
	*(pBuffer+1) = (a&0xff);
	return 2;
}
/*********************************************************************
//��������	:Register_EraseRecorderData(void)
//����		:������ʻ��¼�����еĴ洢����
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
*********************************************************************/
void Register_EraseRecorderData(void)
{
	//����
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
//��������	:Register_EraseOneArea(u8 Type)
//����		:����ĳ�����͵Ĵ洢����
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
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
                IWDG_ReloadCounter();//ι��
    	}
	Register_CheckOneArea(Type);//Ϊʲô��ǰ���������ˣ�dxl,2014.3.15������Ҫ���ĺܶ�ʱ�䣿��
}
/*********************************************************************
//��������	:Register_GetSubpacketData
//����		:��ȡ�ְ�����
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:�ú������ڱ������ܲ��ԣ��������ܲ���Ҫ���ϴ����е���ʻ
//              :��¼�����ݣ�������ʱ�俪ʼ�ϴ����ְ�����ÿ�����512�ֽڣ�
//              :����λ����Ϣ��666�ֽ��⣩PacketNum��1��ʼ
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
        case REGISTER_TYPE_SPEED://48Сʱ�ٶ�//modify by joneming
        {
                for(i=0; i<RegisterSubpacketList[Type]; i++)
                {
                        //��ȡһ������
                        Address = Sector*FLASH_ONE_SECTOR_BYTES+Step*Register[Type].StepLen;
                        sFLASH_ReadBuffer(Buffer,Address,Register[Type].StepLen); 
                        if(0 == i)
                        {
                                //��ʼʱ��
                              TimeCount = Register_GetOldestTime(Type);
                              Gmtime(&tt, TimeCount);
                              Public_ConvertTimeToBCDEx(tt,p);
                              p += 6;
                                //����ʱ��
                              TimeCount = Register_GetNewestTime(Type);
                              Gmtime(&tt, TimeCount);
                              Public_ConvertTimeToBCDEx(tt,p);
                              p += 6;
                               //���λ���ݿ�
                              *p++ = 0;
                              *p++ = 4;//ÿ������4�����ݿ�
                              
                              length += 14;
                      }
                      #if (REGISTER_SPEED_STEP_LEN == 11)//modify by joneming
                      if(0 == i%RegisterBlockList[Type])
                      {    
                                //ÿ�����ݿ�Ŀ�ʼʱ��
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
                              *p++ = 0;//0�뿪ʼ
                              
                              length += 6;
                      }
                      #endif                      
                      //������Ŀ�껺��
                      memcpy(p,Buffer+4,RegisterCollectLen[Type]);
                      p += RegisterCollectLen[Type];
                      length += RegisterCollectLen[Type];
                      //������1
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
          case REGISTER_TYPE_POSITION://360Сʱ��λ��
          {
                for(i=0; i<RegisterSubpacketList[Type]; i++)
                {
                        Address = Sector*FLASH_ONE_SECTOR_BYTES+Step*Register[Type].StepLen;
                        sFLASH_ReadBuffer(Buffer,Address,Register[Type].StepLen); 
                        if(0 == i%RegisterBlockList[Type])
                        {
                                //��ʼʱ��
                              TimeCount = Register_GetOldestTime(Type);
                              Gmtime(&tt, TimeCount);
                              *p++ = Public_HEX2BCD(tt.year);
                              *p++ = Public_HEX2BCD(tt.month);
                              *p++ = Public_HEX2BCD(tt.day);
                              *p++ = Public_HEX2BCD(tt.hour);
                              *p++ = 0;//��,��Ϊ0
                              *p++ = 0;
                              length += 6;
                      }
                      memcpy(p,Buffer+4,RegisterCollectLen[Type]);
                      p += RegisterCollectLen[Type];
                      length += RegisterCollectLen[Type];
                       //������1
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
        case REGISTER_TYPE_DOUBT://�ɵ�����
        case REGISTER_TYPE_OVER_TIME://��ʱ��¼
        case REGISTER_TYPE_DRIVER://��ʻ�����
        case REGISTER_TYPE_POWER_ON://�����¼
        case REGISTER_TYPE_PRAMATER://�޸Ĳ���
        case REGISTER_TYPE_SPEED_STATUS://�ٶ���־
          {
                for(i=0; i<RegisterSubpacketList[Type]; i++)
                {
                        Address = Sector*FLASH_ONE_SECTOR_BYTES+Step*Register[Type].StepLen;
                        sFLASH_ReadBuffer(Buffer,Address,Register[Type].StepLen); 
                        memcpy(p,Buffer+4,RegisterCollectLen[Type]);
                        p += RegisterCollectLen[Type];
                        length += RegisterCollectLen[Type];
                         //������1
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
//��������	:Register_GetSubpacketStoreStep
//����		:��ȡĳ���ְ��Ĵ洢����
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:�ú������ڱ������ܲ��ԣ��������ܲ���Ҫ���ϴ����е���ʻ
//              :��¼�����ݣ�������ʱ�俪ʼ�ϴ����ְ�����ÿ�����512�ֽڣ�
//              :����λ����Ϣ��666�ֽ��⣩������ʱ��ְ����Ϊ1
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
//��������	:Register_GetSubpacketTotalList
//����		:��ȡ�ְ�������
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:�ú������ڱ������ܲ��ԣ��������ܲ���Ҫ���ϴ����е���ʻ
//              :��¼�����ݣ�������ʱ�俪ʼ�ϴ����ְ�����ÿ�����512�ֽڣ�
//              :����λ����Ϣ��666�ֽ��⣩������ʱ��ְ����Ϊ1
*********************************************************************/
u16 Register_GetSubpacketTotalList(u8 Type)
{
        u16   TotalList = 0;
        
        if((Type == 0)||(Type >= REGISTER_TYPE_MAX))
        {
              return 0;
        }
        
        if(0 == Register[Type].LoopFlag)//û��ѭ���洢
        {
                TotalList = (Register[Type].CurrentSector-Register[Type].OldestSector)*Register[Type].SectorStep
                  +Register[Type].CurrentStep;
        }
        else//����ѭ���洢
        {
                TotalList = (Register[Type].CurrentSector-Register[Type].StartSector)*Register[Type].SectorStep
                  +Register[Type].CurrentStep
                    +(Register[Type].EndSector-Register[Type].OldestSector)*Register[Type].SectorStep;
        }
        return  TotalList;
}
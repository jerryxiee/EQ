#ifndef	__REGISTER_H
#define	__REGISTER_H

//*************�ļ�����***************
#include "stm32f2xx_gpio.h"
#include "stm32f2xx.h"
//*************�궨��****************
//////////////////////////////

///////////////////////////////////
//��¼����
#define REGISTER_TYPE_SPEED                 1	//��ʻ�ٶȼ�¼
#define REGISTER_TYPE_POSITION              2	//λ����Ϣ��¼
#define REGISTER_TYPE_DOUBT                 3	//�ɵ����ݼ�¼
#define REGISTER_TYPE_OVER_TIME             4	//��ʱ��ʻ��¼
#define REGISTER_TYPE_DRIVER                5	//��ʻ����ݼ�¼
#define REGISTER_TYPE_POWER_ON              6	//�ⲿ�����¼
#define REGISTER_TYPE_PRAMATER              7	//�����޸ļ�¼
#define REGISTER_TYPE_SPEED_STATUS          8	//�ٶ�״̬��¼
#define	REGISTER_TYPE_MAX                   9	
//�ɼ����ݳ�������
#ifdef USE_ONE_MIN_SAVE_SPEED_RECORD
#define	REGISTER_SPEED_COLLECT_LEN          126	//�����洢//modify by joneming	
#else
#define	REGISTER_SPEED_COLLECT_LEN          2	//�����洢
#endif
#define	REGISTER_POSITION_COLLECT_LEN       11
#define	REGISTER_DOUBT_COLLECT_LEN          234	//����洢
#define	REGISTER_OVER_TIME_COLLECT_LEN      50
#define	REGISTER_DRIVER_COLLECT_LEN         25
#define	REGISTER_POWER_ON_COLLECT_LEN       7
#define	REGISTER_PRAMATER_COLLECT_LEN       7
#define	REGISTER_SPEED_STATUS_COLLECT_LEN   133
//�ɼ�������������
#define	REGISTER_SPEED_MAX_BLOCK            7	//1000�ֽ�/�鳤��126�ֽ�
#define	REGISTER_POSITION_MAX_BLOCK         1	//1000�ֽ�/�鳤��666�ֽ�
#define	REGISTER_DOUBT_MAX_BLOCK            4	//1000�ֽ�/�鳤��234�ֽ�
#define	REGISTER_OVER_TIME_MAX_BLOCK        20	//1000�ֽ�/�鳤��50�ֽ�
#define	REGISTER_DRIVER_MAX_BLOCK           40	//1000�ֽ�/�鳤��25�ֽ�
#define	REGISTER_POWER_ON_MAX_BLOCK         142	//1000�ֽ�/�鳤��7�ֽ�
#define	REGISTER_PRAMATER_MAX_BLOCK         142	//1000�ֽ�/�鳤��7�ֽ�
#define	REGISTER_SPEED_STATUS_MAX_BLOCK	    7	//1000�ֽ�/�鳤��133�ֽ�
//��¼����
#ifdef USE_ONE_MIN_SAVE_SPEED_RECORD
#define	REGISTER_SPEED_STEP_LEN             144	//ʱ��4+ÿ���ӵ���ʻ�ٶȿ�126+Ԥ��13+У��1//modify by joneming	
#else
#define	REGISTER_SPEED_STEP_LEN             11	//ʱ��4+��ʻ�ٶȼ�״̬2+Ԥ��4+У��1
#endif
#define	REGISTER_POSITION_STEP_LEN          24	//ʱ��4+λ����Ϣ10+ƽ���ٶ�1+���4+Ԥ��4+У��1
#define	REGISTER_DOUBT_STEP_LEN             251	//ʱ��4+�ɵ�����234+Ԥ��12+У��1
#define	REGISTER_OVER_TIME_STEP_LEN         67	//ʱ��4+��ʱ��ʻ50+Ԥ��12+У��1
#define	REGISTER_DRIVER_STEP_LEN            36	//ʱ��4+��ʻ�����25+Ԥ��6+У��1
#define	REGISTER_POWER_ON_STEP_LEN          18	//ʱ��4+�ⲿ����7+Ԥ��6+У��1
#define	REGISTER_PRAMATER_STEP_LEN          18	//ʱ��4+�����޸�7+Ԥ��6+У��1
#define	REGISTER_SPEED_STATUS_STEP_LEN      144	//ʱ��4+�ٶ�״̬133+Ԥ��6+У��1
#define	REGISTER_MAX_STEP_LEN               252

//Զ����ʻ��¼�����ݴ���ʱÿ��������
#ifdef USE_ONE_MIN_SAVE_SPEED_RECORD
#define	REGISTER_SPEED_PACKET_LIST          4	//ÿ4������ʻ�ٶȼ�״̬���ݴ��һ�����ϴ�//modify by joneming	
#else
#define	REGISTER_SPEED_PACKET_LIST          240	//ÿ4Сʱ��ÿ��1��������ʻ�ٶȼ�״̬���ݴ��һ�����ϴ�
#endif
#define	REGISTER_POSITION_PACKET_LIST       60	//ÿ1Сʱ��ÿ����1����λ����Ϣ���ݴ��һ�����ϴ�
#define	REGISTER_DOUBT_PACKET_LIST          2	//ÿ2���ɵ����ݴ��һ�����ϴ�
#define	REGISTER_OVER_TIME_PACKET_LIST      10	//ÿ10����ʱ��ʻ���ݴ��һ�����ϴ�
#define	REGISTER_DRIVER_PACKET_LIST         20	//ÿ20����ʻ��������ݴ��һ�����ϴ�
#define	REGISTER_POWER_ON_PACKET_LIST       100	//ÿ100���ⲿ�������ݴ��һ�����ϴ�
#define	REGISTER_PRAMATER_PACKET_LIST       100	//ÿ100�������޸����ݴ��һ�����ϴ�
#define	REGISTER_SPEED_STATUS_PACKET_LIST   4	//ÿ4���ٶ�״̬���ݴ��һ�����ϴ�

//��ʻ��¼�Ǳ�׼��ÿ�������ݰ���������
#ifdef USE_ONE_MIN_SAVE_SPEED_RECORD
#define	REGISTER_SPEED_BLOCK_LIST           1//modify by joneming
#else
#define	REGISTER_SPEED_BLOCK_LIST           60
#endif
#define	REGISTER_POSITION_BLOCK_LIST        60	
#define	REGISTER_DOUBT_BLOCK_LIST           1	
#define	REGISTER_OVER_TIME_BLOCK_LIST       1	
#define	REGISTER_DRIVER_BLOCK_LIST          1	
#define	REGISTER_POWER_ON_BLOCK_LIST        1	
#define	REGISTER_PRAMATER_BLOCK_LIST        1
#define	REGISTER_SPEED_STATUS_BLOCK_LIST    1	

//************�ṹ������******************
typedef struct
{
    s16 StartSector;	//�洢��ַ��ʼ����
    s16 EndSector;	//�洢��ַ��������
    s16 StepLen;	//����
    s16 SectorStep;	//���������ܲ���
    s16 OldestSector;	//����ʱ������
    s16 CurrentSector;	//��ǰʱ������
    s16 CurrentStep;	//��ǰ����
    s8  LoopFlag;	//ѭ���洢��־��0Ϊ��ѭ����1Ϊѭ��	
}REGISTER_STRUCT;
//*************��������**************
//****************�ⲿ�ӿں���*****************
/*********************************************************************
//��������	:Register_CheckArea(void)
//����		:������д洢����ȷ�������������������������²���
//����		:��ȷ����0�������ʾ�ĸ����͵��Լ����
//��ע		:���ϵ�����whileǰ����øú�����
//		:ֻ�иú����ȱ����ú���ܱ�֤�����Ķ���д����������������ȷ
*********************************************************************/
u8 Register_CheckArea(void);
/*********************************************************************
//��������	:Register_CheckOneArea(void)
//����		:���ĳһ���洢����
//����		:��ȷ����SUCCESS�����󷵻�ERROR
*********************************************************************/
ErrorStatus Register_CheckOneArea(u8 Type);
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
u8 Register_CheckOneArea2(u8 Type);
/*********************************************************************
//��������	:Register_Write(u8 Type,u8 *pBuffer, u8 length)
//����		:��¼һ��ĳ�����͵���Ϣ
//����		:��ȷ����SUCCESS�����󷵻�ERROR
*********************************************************************/
ErrorStatus Register_Write(u8 Type,u8 *pBuffer, u16 length);
/*********************************************************************
//��������	:Register_Write2(u8 Type,u8 *pBuffer, u8 length, u32 Time)
//����		:��¼һ��ĳ�����͵���Ϣ 
//����		:��ȷ����SUCCESS�����󷵻�ERROR
//��ע		:TimeΪ32λ��ʾ��ʱ�䣬
//              :����ƣ�ͼ�ʻ���ʱ����ƣ�ͼ�ʻ����ʱ�䣻
//              :���ڵ����豣����������ʱ���ǵ���ǰ��¼�����ʱ��
//              :�������ͨ�����ʱ���ǵ�ǰʱ�䣬����RTC_GetCounter�ɻ�õ�ǰʱ�䡣
*********************************************************************/
ErrorStatus Register_Write2(u8 Type,u8 *pBuffer, u16 length,u32 Time);
/*********************************************************************
//��������	:Register_Read(u8 Type,u8 *pBuffer, TIME_T StartTime, TIME_T EndTime, u16 MaxBlock)
//����		:��ȡָ��ʱ��Σ�ָ��������ݿ�ļ�¼����
//����		:��ȡ�����ݳ���
*********************************************************************/
u16 Register_Read(u8 Type,u8 *pBuffer, TIME_T StartTime, TIME_T EndTime, u16 MaxBlock);
/*********************************************************************
//��������	:Register_GetNewestTime(u8 Type)
//����		:��ȡ��ǰ�洢�����²�����ʱ�� 
//����		:4�ֽڱ�ʾ��ʱ��
*********************************************************************/
u32 Register_GetNewestTime(u8 Type);
/*********************************************************************
//��������	:Register_GetOldestTime(u8 Type)
//����		:��ȡ��ǰ�洢�����ϲ�����ʱ�� 
//����		:4�ֽڱ�ʾ��ʱ��
*********************************************************************/
u32 Register_GetOldestTime(u8 Type);
/*********************************************************************
//��������	:Register_Search(u8 Type,u32 Time, s16 *Sector, s16 *Step)
//����		:����ĳһ����ʱ��Ĵ洢λ��
//����		:ʵ�ʲ��ҵõ���ʱ��
//��ע		:���ҵĽ������Sector��Step��
*********************************************************************/
u32 Register_Search(u8 Type, u32 Time, s16 *Sector, s16 *Step);
/*********************************************************************
//��������	:Register_EraseRecorderData(void)
//����		:������ʻ��¼�����еĴ洢����
//��ע		:
*********************************************************************/
void Register_EraseRecorderData(void);
/*********************************************************************
//��������	:Register_EraseOneArea(u8 Type)
//����		:����ĳ�����͵Ĵ洢����
//��ע		:
*********************************************************************/
void Register_EraseOneArea(u8 Type);
/*********************************************************************
//��������	:Register_GetSubpacketData
//����		:��ȡ�ְ�����
//��ע		:�ú������ڱ������ܲ��ԣ��������ܲ���Ҫ���ϴ����е���ʻ
//              :��¼�����ݣ�������ʱ�俪ʼ�ϴ����ְ�����ÿ�����512�ֽڣ�
//              :����λ����Ϣ��666�ֽ��⣩PacketNum��1��ʼ
*********************************************************************/
u16 Register_GetSubpacketData(u8 *pBuffer,u8 Type, u16 PacketNum);
/*********************************************************************
//��������	:Register_GetSubpacketStoreStep
//����		:��ȡĳ���ְ��Ĵ洢����
//��ע		:�ú������ڱ������ܲ��ԣ��������ܲ���Ҫ���ϴ����е���ʻ
//              :��¼�����ݣ�������ʱ�俪ʼ�ϴ����ְ�����ÿ�����512�ֽڣ�
//              :����λ����Ϣ��666�ֽ��⣩������ʱ��ְ����Ϊ1
*********************************************************************/
void Register_GetSubpacketStoreStep(s16 *Sector, s16 *Step,u8 Type, u16 PacketNum);
/*********************************************************************
//��������	:Register_GetSubpacketTotalList
//����		:��ȡ�ְ�������
//��ע		:�ú������ڱ������ܲ��ԣ��������ܲ���Ҫ���ϴ����е���ʻ
//              :��¼�����ݣ�������ʱ�俪ʼ�ϴ����ְ�����ÿ�����512�ֽڣ�
//              :����λ����Ϣ��666�ֽ��⣩������ʱ��ְ����Ϊ1
*********************************************************************/
u16 Register_GetSubpacketTotalList(u8 Type);
//*****************�ڲ��ӿں���**********************
/*********************************************************************
//��������	:Register_GetVerifySum(u8 *pBuffer, u16 length)
//����		:��ȡ�����ַ������ݵ�У���
//��ע		:
*********************************************************************/
u8 Register_GetVerifySum(u8 *pBuffer, u16 length);
/*********************************************************************
//��������	:Register_CheckPram(u8 Type, u32 Time, u8 length)
//����		:�����������ĺϷ���
//��ע		:
*********************************************************************/
ErrorStatus Register_CheckPram(u8 Type, u32 Time, u16 length);
/*********************************************************************
//��������	:Register_BytesToInt(u8 *pBuffer)
//����		:�ĸ��ֽ���ת����һ���������ݣ����ģʽ�����ֽ���ǰ
//��ע		:
*********************************************************************/
u32 Register_BytesToInt(u8 *pBuffer);
/*********************************************************************
//��������	:Register_IntToBytes(u8 *pBuffer, u32 a)
//����		:һ����������ת�����ĸ��ֽ��������ģʽ�����ֽ���ǰ
//��ע		:
*********************************************************************/
u8 Register_IntToBytes(u8 *pBuffer, u32 a);
/*********************************************************************
//��������	:Register_BytesToShortInt(u8 *pBuffer)
//����		:2���ֽ���ת����һ�����������ݣ����ģʽ�����ֽ���ǰ
//��ע		:
*********************************************************************/
u16 Register_BytesToShortInt(u8 *pBuffer);
/*********************************************************************
//��������	:Register_ShortIntToBytes(u8 *pBuffer, u16 a)
//����		:һ������������ת����2���ֽ��������ģʽ�����ֽ���ǰ
//��ע		:
*********************************************************************/
u8 Register_ShortIntToBytes(u8 *pBuffer, u16 a);


#endif

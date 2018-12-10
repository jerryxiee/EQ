#ifndef __FRAM_H
#define __FRAM_H

//*********************ͷ�ļ�********************************
#include "stm32f2xx.h"
#include "spi_flash.h"

//*********************�Զ�����������************************


//**********************�궨��********************************
//******************����ռ����******************
//#define 	FRAM_DOUBT_POINT_ADDR			0	//�ɵ�����,200�ֽ�,֮ǰ2003��׼�õ��ɵ����ݵ�ַ
//#define 	FRAM_DOUBT_POINT_LEN			300	//�ɵ����ݵ�ַ����,����У���ֽ�

#define 	FRAM_SPEED_RUN_RECORD_ADDR          0//���浱ǰ����һ���ӵ���ʻ��¼����
#define 	FRAM_SPEED_RUN_RECORD_LEN           128//���浱ǰ����һ���ӵ���ʻ��¼����
#define 	FRAM_SPEED_PLAY_ADDR                130
#define 	FRAM_SPEED_PLAY_ADDR_LEN            8//���浱ǰ�����ṹ��
#define     FRAM_PHONE_MONTH_PARAM_ADDR         139//ÿ��ͨ������
#define     FRAM_PHONE_MONTH_PARAM_LEN          12//ÿ��ͨ����������
#define		FRAM_MILEAGE_REMAINDER_ADDR		310	//�ۼ����������ַ����������������
#define		FRAM_MILEAGE_REMAINDER_LEN		2	//�ۼ�����������ȣ���������������
#define		FRAM_CURRENT_DRIVER_MILEAGE_ADDR	313	//�Լ�ʻ��̣���ǰ��ʻԱ���
#define		FRAM_CURRENT_DRIVER_MILEAGE_LEN		3	//��ǰ��ʻԱ��̴洢����
#define 	FRAM_START_TIME_ADDR            	317  	//����ʱ��
#define 	FRAM_START_TIME_LEN			4	//����ʱ��洢����
#define 	FRAM_END_TIME_ADDR              	322  	//ͣ��ʱ��
#define 	FRAM_END_TIME_LEN			4	//ͣ��ʱ��洢����
#define		FRAM_TIRED_START_TIME_ADDR		327  	//ƣ�ͼ�ʻ��¼����ʱ��
#define		FRAM_TIRED_START_TIME_LEN		4	//ƣ�ͼ�ʻ����ʱ��洢����
#define		FRAM_TIRED_END_TIME_ADDR		332  	//ƣ�ͼ�ʻ��¼ͣ��ʱ��
#define		FRAM_TIRED_END_TIME_LEN			4	//ƣ�ͼ�ʻͣ��ʱ��洢����
#define 	FRAM_TIRED_DRIVER_ADDR          	337  	//ƣ�ͼ�ʻ��ʻ֤�����3���ֽ�
#define 	FRAM_TIRED_DRIVER_LEN			3	//ƣ�ͼ�ʻ��ʻԱ����洢����
#define 	FRAM_TRIED_DRIVE_ADDR			341	//ƣ�ͼ�ʻʱ��
#define 	FRAM_TRIED_DRIVE_LEN			4	//ƣ�ͼ�ʻʱ��洢����
#define		FRAM_STOP_MEAN_SPEED_ADDR		346  	//ͣ��ǰ15���ӳ���
#define		FRAM_STOP_MEAN_SPEED_LEN		96	//ͣ��ǰ15���ӳ���
#define 	FRAM_MEAN_SPEED_BASE_ADDR		FRAM_STOP_MEAN_SPEED_ADDR
#define 	FRAM_MEAN_SPEED_STEP_LEN		5
#define		FRAM_LAST_LOCATION_ADDR			450	//��һ����Ч��λ��
#define 	FRAM_LAST_LOCATION_LEN			11	//��һ����Ч��λ��
#define		FRAM_MILEAGE_ADDR			472	//����̴洢��ַ,��У���ֽ�,
#define		FRAM_MILEAGE_LEN			4	//����̴洢����,��У���ֽ�
#define		FRAM_LAST_RUN_TIME_ADDR			477	//�����ʻ���Ǹ�ʱ��,��У���ֽ�
#define		FRAM_LAST_RUN_TIME_LEN			4	//�����ʻ���Ǹ�ʱ�̳���
#define		FRAM_15MIN_END_TIME_ADDR		482	//15����ͣ��ʱ��
#define         FRAM_15MIN_END_TIME_LEN			4	
#define		FRAM_MEDIA_ID_ADDR			487	//��ý��ID
#define		FRAM_MEDIA_ID_LEN			4
/* �ϵ����ճ���ʹ�õĺ궨�壬˫���Ӱ汾����ʹ��
#define		FRAM_CAM1_PHOTO_NUM_ADDR		492	//����ʣ������
#define		FRAM_CAM1_PHOTO_NUM_LEN			2
#define		FRAM_CAM1_PHOTO_TIME_ADDR		495	//����ʱ����
#define		FRAM_CAM1_PHOTO_TIME_LEN		2
#define		FRAM_CAM1_SAVE_FLAG_ADDR		498	//���㱣���־
#define		FRAM_CAM1_SAVE_FLAG_LEN			1
#define		FRAM_CAM1_RESOLUTION_ADDR		500	//����ֱ���
#define		FRAM_CAM1_RESOLUTION_LEN		1
#define		FRAM_CAM2_PHOTO_NUM_ADDR		502	//����ʣ������
#define		FRAM_CAM2_PHOTO_NUM_LEN			2
#define		FRAM_CAM2_PHOTO_TIME_ADDR		505	//����ʱ����
#define		FRAM_CAM2_PHOTO_TIME_LEN		2
#define		FRAM_CAM2_SAVE_FLAG_ADDR		508	//���㱣���־
#define		FRAM_CAM2_SAVE_FLAG_LEN			1
#define		FRAM_CAM2_RESOLUTION_ADDR		510	//����ֱ���
#define		FRAM_CAM2_RESOLUTION_LEN		1
#define		FRAM_CAM3_PHOTO_NUM_ADDR		512	//����ʣ������
#define		FRAM_CAM3_PHOTO_NUM_LEN			2
#define		FRAM_CAM3_PHOTO_TIME_ADDR		515	//����ʱ����
#define		FRAM_CAM3_PHOTO_TIME_LEN		2
#define		FRAM_CAM3_SAVE_FLAG_ADDR		518	//���㱣���־
#define		FRAM_CAM3_SAVE_FLAG_LEN			1
#define		FRAM_CAM3_RESOLUTION_ADDR		520	//����ֱ���
#define		FRAM_CAM3_RESOLUTION_LEN		1
#define		FRAM_CAM4_PHOTO_NUM_ADDR		522	//����ʣ������
#define		FRAM_CAM4_PHOTO_NUM_LEN			2
#define		FRAM_CAM4_PHOTO_TIME_ADDR		525	//����ʱ����
#define		FRAM_CAM4_PHOTO_TIME_LEN		2
#define		FRAM_CAM4_SAVE_FLAG_ADDR		528	//���㱣���־
#define		FRAM_CAM4_SAVE_FLAG_LEN			1
#define		FRAM_CAM4_RESOLUTION_ADDR		530	//����ֱ���
#define		FRAM_CAM4_RESOLUTION_LEN		1
//522-572Ϊ����ͷ5-8����
*/
#define		FRAM_CAM_TIME_PHOTO_NUM_ADDR		    492	//ƽ̨��ʱ����ʣ������
#define		FRAM_CAM_TIME_PHOTO_NUM_LEN		    2//4·����ͷ��ռ��12�ֽ�,ÿ·3�ֽ�,��1��4,˳������
#define		FRAM_CAM_TIME_PHOTO_SPACE_ADDR	    504	//ƽ̨��ʱ����ʱ��������λ��
#define		FRAM_CAM_TIME_PHOTO_SPACE_LEN		    2//4·����ͷ��ռ��12�ֽ�,ÿ·3�ֽ�,��1��4,˳������
#define		FRAM_CAM_TIME_PHOTO_STORE_FLAG_ADDR	516	//ƽ̨��ʱ���ձ����־,bit0��1��ʾ��Ҫ�ϴ���bit1��1��ʾ��Ҫ������SD��
#define		FRAM_CAM_TIME_PHOTO_STORE_FLAG_LEN    1//4·����ͷ��ռ��8�ֽ�,ÿ·2�ֽ�,��1��4,˳������
#define		FRAM_CAM_TIME_PHOTO_RESOLUTION_ADDR	524	//ƽ̨��ʱ���շֱ���,0Ϊ320*240,1Ϊ640*480
#define		FRAM_CAM_TIME_PHOTO_RESOLUTION_LEN    1//4·����ͷ��ռ��8�ֽ�,ÿ·2�ֽ�,��1��4,˳������
#define		FRAM_CAM_TIME_PHOTO_EVENT_TYPE_ADDR	532	//ƽ̨��ʱ�����¼�����
#define		FRAM_CAM_TIME_PHOTO_EVENT_TYPE_LEN    1//4·����ͷ��ռ��8�ֽ�,ÿ·2�ֽ�,��1��4,˳������
#define		FRAM_CAM_TIME_PHOTO_CHANNEL_ADDR	    540	//ƽ̨��ʱ�����ϴ�ͨ����ֻ��ΪCHANNEL_DATA_1������CHANNEL_DATA_2
#define		FRAM_CAM_TIME_PHOTO_CHANNEL_LEN       1//4·����ͷ��ռ��8�ֽ�,ÿ·2�ֽ�,��1��4,˳������

#define		FRAM_CAM_EVENT_PHOTO_NUM_ADDR		    548	//�¼�����ʣ������
#define		FRAM_CAM_EVENT_PHOTO_NUM_LEN		    2//4·����ͷ��ռ��12�ֽ�,ÿ·3�ֽ�,��1��4,˳������
#define		FRAM_CAM_EVENT_PHOTO_SPACE_ADDR	        560	//�¼�����ʱ��������λ��
#define		FRAM_CAM_EVENT_PHOTO_SPACE_LEN		    2//4·����ͷ��ռ��12�ֽ�,ÿ·3�ֽ�,��1��4,˳������
#define		FRAM_CAM_EVENT_PHOTO_STORE_FLAG_ADDR	572	//�¼����ձ����־,bit0��1��ʾ��Ҫ�ϴ���bit1��1��ʾ��Ҫ������SD��
#define		FRAM_CAM_EVENT_PHOTO_STORE_FLAG_LEN     1//4·����ͷ��ռ��8�ֽ�,ÿ·2�ֽ�,��1��4,˳������
#define		FRAM_CAM_EVENT_PHOTO_RESOLUTION_ADDR	580	//�¼����շֱ���,0Ϊ320*240,1Ϊ640*480
#define		FRAM_CAM_EVENT_PHOTO_RESOLUTION_LEN     1//4·����ͷ��ռ��8�ֽ�,ÿ·2�ֽ�,��1��4,˳������
#define		FRAM_CAM_EVENT_PHOTO_EVENT_TYPE_ADDR	588	//�¼������¼�����
#define		FRAM_CAM_EVENT_PHOTO_EVENT_TYPE_LEN     1//4·����ͷ��ռ��8�ֽ�,ÿ·2�ֽ�,��1��4,˳������
#define		FRAM_CAM_EVENT_PHOTO_CHANNEL_ADDR	    596	//�¼������ϴ�ͨ����ֻ��ΪCHANNEL_DATA_1������CHANNEL_DATA_2
#define		FRAM_CAM_EVENT_PHOTO_CHANNEL_LEN        1//4·����ͷ��ռ��8�ֽ�,ÿ·2�ֽ�,��1��4,˳������

#define         FRAM_PARAMETER_TIME_PHOTO_CHANNEL_ADDR  604//������ʱ����ͨ��
#define         FRAM_PARAMETER_TIME_PHOTO_CHANNEL_LEN   1
#define         FRAM_PARAMETER_DISTANCE_PHOTO_CHANNEL_ADDR  606//������������ͨ��
#define         FRAM_PARAMETER_DISTANCE_PHOTO_CHANNEL_LEN   1

#define         FRAM_PHOTO_ID_ADDR                      608
#define         FRAM_PHOTO_ID_LEN                       4


//��һ����ַ��613

#define		FRAM_TEMP_TRACK_NUM_ADDR		647	//��ʱ����ʣ�����
#define		FRAM_TEMP_TRACK_NUM_LEN			4
#define		FRAM_TEMP_TRACK_SPACE_ADDR		652	//��ʱ���ټ��
#define		FRAM_TEMP_TRACK_SPACE_LEN		2	
//#define		FRAM_BLIND_SECTOR_ADDR			591	//ä���㱨������
//#define		FRAM_BLIND_SECTOR_LEN			2
//#define		FRAM_BLIND_STEP_ADDR			594	//ä�������Ĳ���
//#define		FRAM_BLIND_STEP_LEN			    2
#define		FRAM_EMERGENCY_FLAG_ADDR		655	//����������һ��״̬
#define		FRAM_EMERGENCY_FLAG_LEN			1
#define		FRAM_RECORD_FLAG_ADDR			657	//¼����¼��������
#define 	FRAM_RECORD_FLAG_LEN 			20
#define  	FRAM_TEXI_SERVICE_ADDR 			678 	//���⳵ҵ��
#define  	FRAM_TEXI_SERVICE_LEN 			24
#define		FRAM_RECORD_CONTROL_ADDR		703	//¼�����Ʊ�־
#define		FRAM_RECORD_CONTROL_LEN			1
#define		FRAM_RECORD_CLOSE_ADDR			705	//��¼����־
#define		FRAM_RECORD_CLOSE_LEN			1
#define		FRAM_RECORD_OPEN_ADDR			707	//��¼����־
#define		FRAM_RECORD_OPEN_LEN			1
#define		FRAM_TTS_VOLUME_ADDR			709	//TTS����
#define		FRAM_TTS_VOLUME_LEN			    1
#define		FRAM_TEL_VOLUME_ADDR			711	//�绰����
#define		FRAM_TEL_VOLUME__LEN			1
/*
#define		FRAM_CAM1_EVENT_PHOTO_NUM_ADDR		666	//�¼���������ʣ������
#define		FRAM_CAM1_EVENT_PHOTO_NUM_LEN		2
#define		FRAM_CAM1_EVENT_PHOTO_TIME_ADDR		669	//����ʱ����
#define		FRAM_CAM1_EVENT_PHOTO_TIME_LEN		2
#define		FRAM_CAM2_EVENT_PHOTO_NUM_ADDR		672	//�¼���������ʣ������
#define		FRAM_CAM2_EVENT_PHOTO_NUM_LEN		2
#define		FRAM_CAM2_EVENT_PHOTO_TIME_ADDR		675	//����ʱ����
#define		FRAM_CAM2_EVENT_PHOTO_TIME_LEN		2
#define		FRAM_CAM3_EVENT_PHOTO_NUM_ADDR		678	//�¼���������ʣ������
#define		FRAM_CAM3_EVENT_PHOTO_NUM_LEN		2
#define		FRAM_CAM3_EVENT_PHOTO_TIME_ADDR		681	//����ʱ����
#define		FRAM_CAM3_EVENT_PHOTO_TIME_LEN		2
#define		FRAM_CAM4_EVENT_PHOTO_NUM_ADDR		684	//�¼���������ʣ������
#define		FRAM_CAM4_EVENT_PHOTO_NUM_LEN		2
#define		FRAM_CAM4_EVENT_PHOTO_TIME_ADDR		687	//����ʱ����
#define		FRAM_CAM4_EVENT_PHOTO_TIME_LEN		2
//Ԥ������ͷ5~8
*/
#define FRAM_OIL_CTRL_ADDR                  		713 //����·
#define FRAM_OIL_CTRL_ADDR_LEN         			1
#define FRAM_SPEED_LOG_TIME                             715   //�ٶ���־��Ϣ
#define FRAM_SPEED_LOG_TIME_LEN                         7
#define FRAM_DOUBT_POINT_ADDR 				723  //�ɵ�����,2byte���� + 7byteʵʱʱ�� + 450byte�ٶ�״̬
#define FRAM_DOUBT_POINT_LEN 				459  //�ɵ����ݵ�ַ����,����У���ֽ�
#define FRAM_POWER_LOG_TIME 				1183 //���繤��ʱ��--����¼Ϊ�ޱ���ʹ��
#define FRAM_POWER_LOG_TIME_LEN 			6    //��¼ʱ�䳤��Ϊ6
#define FRAM_DOUBT_POSITION_ADDR                        1190 //�ɵ�ɼ�ʱλ����Ϣ��ÿһ��д��һ��
#define FRAM_DOUBT_POSITION_LEN                         10
#define FRAM_SPEED_STATUS_ADDR                          1201 //�ٶ�״̬��¼��־
#define FRAM_SPEED_STATUS_LEN                           1
#define FRAM_SPEED_STA_TIME_ADDR                        1203 //�ٶ�״̬ʱ���¼
#define FRAM_SPEED_STA_TIME_LEN                         6
#define	FRAM_STOP_TIME_ADDR				1210//��ǰ
#define	FRAM_STOP_TIME_LEN				4
#define FRAM_CAR_OWNER_PHONE_ADDR                       1215//�����ֻ���,�ַ���ASCII��,dxl
#define FRAM_CAR_OWNER_PHONE_LEN                        12//д����ȡ�������ʱ��������д12,λ������ʱ��0x00
#define FRAM_MAIN_DOMAIN_NAME_ADDR 			1228//�������������,�ַ���,ASCII��,Ϊ�б��ͼ�����,dxl
#define FRAM_MAIN_DOMAIN_NAME_LEN                       30//д����ȡ�������ʱ��������д30,λ������ʱ��0x00
#define FRAM_BACKUP_DOMAIN_NAME_ADDR 		        1259//���ݼ����������,�ַ���,ASCII��,Ϊ�б��ͼ�����,dxl
#define FRAM_BACKUP_DOMAIN_NAME_LEN                      30//д����ȡ�������ʱ��������д30,λ������ʱ��0x00
#define FRAM_CAR_CARRY_STATUS_ADDR 			1290//�����ػ�״̬,1�ֽ�,dxl
#define FRAM_CAR_CARRY_STATUS_LEN                         1//д����ȡ�ò���ʱ������Ϊ1
#define FRAM_QUICK_OPEN_ACCOUNT_ADDR                    1292//���ٿ�������״̬�洢��ַ
#define FRAM_QUICK_OPEN_ACCOUNT_LEN                       1//д����ȡ�ò���ʱ������Ϊ1
#define FRAM_MULTI_CENTER_LINK_ADDR                     1294//���������ӱ�־
#define FRAM_MULTI_CENTER_LINK_LEN                        1//���������ӱ�־����

#define FRAM_STOP_15MINUTE_SPEED_ADDR                   1296//ͣ��ǰ15�����ٶ�
#define FRAM_STOP_15MINUTE_SPEED_LEN                    62  // 1byte + 1byte = ���� + У��
                                                            // (2byte + 1byte + 1byte = ʱ�� + �ٶ� + У��)*15
#define FRAM_STOPING_TIME_ADDR                          1359// ����ͣʻ��һʱ�̵�ʱ��
#define FRAM_STOPING_TIME_LEN                           6   // 6byte  BCD������ʱ���� 

#define FRAM_FIRMWARE_UPDATA_FLAG_ADDR                  1366//�������񴥷���־������1��ʾ������Զ������������2��ʾ�����ɹ���
                                                            //����3��ʾ����ʧ�ܣ�����4��ʾ�ѷ��͹�0x0108�����ˣ�ÿ���ϵ��Ȩ��ɺ���Ҫ��ȡ��ֵ��
#define FRAM_FIRMWARE_UPDATA_FLAG_LEN                   1
#define FRAM_GNSS_SAVE_STEP_ADDR                        1368	//Gnssä�������Ĳ���
#define FRAM_GNSS_SAVE_STEP_LEN                         2

#define FRAM_HY_MILEAGE_ADDR                            1371//����ƽ̨ר��                         
#define FRAM_HY_MILEAGE_LEN                             4
#define FRAM_HY_UPDATA_ADDR                             1376//����ƽ̨ר��                         
#define FRAM_HY_UPDATA_LEN                              1
#define FRAM_FIRMWARE_UPDATA_TYPE_ADDR                  1378//Զ����������,0Ϊ�ն�,9Ϊ���ؿ�����,Ŀǰֻ֧��������
#define FRAM_FIRMWARE_UPDATA_TYPE_LEN                   1

#define FRAM_ROLL_OVER_INIT_ANGLE_ADDR                  1380//�෭��ʼ�Ƕ�
#define FRAM_ROLL_OVER_INIT_ANGLE_LEN                   8

#define FRAM_OIL_WEAR_CALIB_ADDR                        1400//�ͺ�У׼//lzm,2014.9.24
 #define FRAM_OIL_WEAR_CALIB_ADDR_LEN                    104

//1505-1515�ڱ�׼���б�ʹ���ˣ������ܿ���
#define FRAM_ACCELERATION_ADDR                          1516
#define FRAM_ACCELERATION_LEN                           4//0-30,30-60,60-90,>90
#define FRAM_DECELERATION_ADDR                          1521
#define FRAM_DECELERATION_LEN                           2//0-30,>30
#define FRAM_TURN_ANGLE_ADDR                			1524
#define FRAM_TURN_ANGLE_LEN                 			2//��ת��Ƕȷ�

#define FRAM_INTERFACE_SWOTCH_ADDR                       1527
#define FRAM_INTERFACE_SWOTCH_LEN                        2

//��һ����ַΪ1524

#define		FRAM_LAST_ADDR				0x07fc  //2043,FRAM��������ֽ�����оƬ�Լ�

//*******************����������*****************
//#define FRAM_CS	  		GPIOC
//#define FRAM_Pin_CS	  	GPIO_Pin_4

#define FRAM_CS_HIGH()	GpioOutOn(FR_CS);spi_Delay_uS(60)
#define FRAM_CS_LOW()	GpioOutOff(FR_CS);spi_Delay_uS(60)

//***********************��������********************************
/*********************************************************************
//��������	:FRAM_Init()
//����		:�����ʼ��
//��ע		:
*********************************************************************/
void FRAM_Init(void);
/*********************************************************************
//��������	:CheckFramChip(void)
//����		:�������оƬ�����Ƿ�����
//����		:�����ַ����������ֽ������Լ�
//��ע		:
*********************************************************************/
ErrorStatus CheckFramChip(void);
/*********************************************************************
//��������	:FRAM_BufferWrite2(u16 WriteAddr, u8 *pBuffer, u16 NumBytesToWrite)
//����		:��������д����,������ĳβ����У���ֽ�
//����		:WriteAddr�������ַ
//		:pBuffer�����ݻ���
//		:NumBytesToWrite��д����ֽ���
//��ע		:
*********************************************************************/
void FRAM_BufferWrite(u16 WriteAddr, u8 *pBuffer, u16 NumBytesToWrite);
/*********************************************************************
//��������	:FRAM_BufferRead(u8 *pBuffer, u16 NumBytesToRead, u16 ReadAddr)
//����		:�������������
//����		:ReadAddr�������ַ
//		:pBuffer��Ŀ�껺��
//		:NumBytesToRead���������ֽ��� 
//����		:ʵ�ʶ������ֽ���
//��ע		:
*********************************************************************/
u8 FRAM_BufferRead(u8 *pBuffer, u16 NumBytesToRead, u16 ReadAddr);
/*********************************************************************
//��������	:FRAM_WriteEnable(void)
//����		:����дʹ��
//��ע		:
*********************************************************************/
void FRAM_WriteEnable(void);
/*********************************************************************
//��������	:FRAM_WriteDisable(void)
//����		:����д��ֹ
//��ע		:
*********************************************************************/
void FRAM_WriteDisable(void);
/*********************************************************************
//��������	:FRAM_WriteStatusRegister(u8 Byte)
//����		:д����״̬�Ĵ���
//��ע		:/WP�����Ǹߵ�ƽ����Ч��ƽ,/WP�����Ǳ���д����״̬�Ĵ���
*********************************************************************/
void FRAM_WriteStatusRegister(u8 Byte);
/*********************************************************************
//��������	:FRAM_ReadStatusRegister(void)
//����		:������״̬�Ĵ��� 
//����		:״̬�Ĵ�����ֵ
//��ע		:
*********************************************************************/
u8   FRAM_ReadStatusRegister(void);
/*********************************************************************
//��������	:FRAM_EraseChip(void)
//����		:������������
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:״̬�Ĵ�����ֵ
//��ע		:
*********************************************************************/
void  FRAM_EraseChip(void);
#endif

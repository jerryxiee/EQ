
#ifndef __MAIN_H
#define __MAIN_H

//*********�궨��****************   
#define WATCHDOG_OPEN	1
#define SLEEP_OPEN	2
//#define DEBUG_OPEN	3
//#define HUOYUN_DEBUG_OPEN  1//����ƽ̨�������øú꣬���������ú�Ҫ���Σ�������
//#define HUOYUN 1//��ʻ��¼�Ƿְ��̶�900�ֽ���Ҫ�����ú꣬���������ú�Ҫ���Σ�������

#define LOG_TYPE 0   //��ӡ���������Ϣ,��������ʱӦΪ0:0-����,1-���

#define GPS_MODE_TYPE_UM330   //ֻ�ܶ�ѡһ
//#define GPS_MODE_TYPE_TD3017  


#define  BEEP_ON()  		GpioOutOn(BUZZER);BeepOnOffFlag = 1;BeepOnTime=Timer_Val()//ʵ��Ӧ��ʹ��
//#define  BEEP_ON()  		GpioOutOff(BUZZER);BeepOnOffFlag = 1;BeepOnTime=Timer_Val()//����ʱ����

#define  ICCARD_SEL ICCARD_JTD //ʹ�ý�ͨ��ָ��IC��
//#define  ICCARD_SEL  ICCARD_EEYE//ʹ�������Ŀ�

#define USE_ONE_MIN_SAVE_SPEED_RECORD     //��������ˣ���ʻ�ٶȼ�¼��һ����һ���Ĵ洢��ʽ,��˫���Ӱ汾��ʼһֱ�����ú�

#define	FIRMWARE_VERSION	 "11713"//�ر�ע���V1.01.09�汾��ʼ�̼������з�֧У��,�˴���������main.c�еĹ̼��汾��һ��,main.c�еĺ�5λ��Ver:xxxxx��ͬ

//�汾�Ź�5λ���֣���1λ��ʾ����EGS701���汾�ŵ�2λ����3��ʾ��֧�汾�š�������ʾG����ż����ʾC�����汾�ŵ�4λ����5λ��ʾ�Ӱ汾�ţ�����ˮ�źš�
//������ʹ�õķ�֧�汾��Ϊ��01-02����ʾͨ�ð汾��03-04����ʾ��ͨ�汾��05-06����ʾ�����汾��07-08��ʾխ����ʾ���汾��09-10:����ת�������汾��11-12�������汾��13-14��ʽ��Ŀ,15-16�������
#define FIRMWARE_TIME		 "201809170024"//����ʱ����HB-EGS701GBN_BZ-V1.11.02��201412271042�汾���޸ĵĵ�1��

#define Recorder_CCC_ID             "C000116"          /*7���ֽ�*/
#define Recorder_Product_VER        "EGS701          " /*16���ֽ�*/

extern const char ProductInfo[][17]; 
extern const char MANUFACTURER[];
extern const char PRODUCT_MODEL[];//��2���ո�
extern const char PCB_VERSION[];//��1���ո�
extern const char SOFTWARE_VERSION[];//��main.h�е�FIRMWARE_VERSION����һ�£�ǰ��1λ���0,��5λһ�¡�
//*********��������**************
#endif

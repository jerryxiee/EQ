
#ifndef __PW__LOG__
#define __PW__LOG__

#ifndef	_EXT_PW_
	#define	EXT_PW_LOG		extern
#else
	#define	EXT_PW_LOG
#endif

//��Դ��¼ƫ�ƺ궨��
#define		RECORD_PWER_LENGTH		7+6	//	7�ֽڼ�¼���ȣ�6�ֽ�Ԥ������

#pragma pack(1)
typedef struct _POWER_RE_
{
		TIME_T	time;
		uchar 	status;
}_API_PWER_RE_;
#pragma pack()

//�ⲿ�����Ƿѹ����ֵ
//������ⲿ����12V��Ӧ11��5������ⲿ����24V����Ӧ21.5V��
//��ѹ��λmV
#define		mV		*1
#define		MAIN_PW_UNDER_12V		11*1000mV	//11.5*1000mV		//11.5V ƫ��-5%
#define		MAIN_PW_UNDER_24V		21*1000mV	//21.5*1000mV		//21.5Vƫ�� -5%

#define		REFRENCE_12V_OR_24V		18*1000mV		//�ж����繩����12V���� 24V
#define		ZERO_VOLTAGE_GND		0.6*1000mV		//��-�տ��أ���Ӧ��ѹ ����С��0.6V

#define		MAIN_PW_RUN				1		//1��ͨ��
#define		MAIN_PW_STOP				2		//2���ϵ�	

//�ⲿ����7�ֽ�+Ԥ��6�ֽڣ�Ԥ���ֽ������0x00������13�ֽ�
//�������͹���ʱ�䣨BCD���ʽ��	�¼�����
//�� �� ��	| ʱ �� ��	|   1��ͨ�磬2���ϵ�
// 3�ֽ�	  | 3�ֽ�	    |   1�ֽ�
FunctionalState CheckMainPowerRun_TimeTask(void);

//ת������
void HexToBcd(uchar *p_bcd,uchar *p_hex,uchar length);

#endif

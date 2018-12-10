//AD����ɨ�躯��
//AD���̵�Ӳ��ԭ����������ݲ�ͬ��AD����ֵ��������Ӧ��AD���̹�ϵ��
//ÿ�������ƫ��ֵ��+-5%
#define		_EXT_KEY_VOL_ 

#include "include.h"
//#include "Adc.h"
#include "adc_tran_voltag.h"
#include "adc_key.h"

#define		CONST_OFFSET	100
/**********************************************************************************/
/*function	name: 			void  KeyBoardAdc_TimeTask(void)		*/
/*input parameter:			none  */
/*output parameter:			none */
/*FUNCTION: 			����ɨ�躯�� ��ʱ100mS����
*/
/**********************************************************************************/
FunctionalState KeyBoardAdc_TimeTask(void)
{
	uint value,key_tmp;
	//key_code
	value=Adc_GetKeyValue();
    //LOG_PR( "value = %d\r\n",value);
	key_tmp=Ad_GetValue(ADC_KEY);
	if(key_tmp>value)
		{
			if(key_tmp-value>CONST_OFFSET)
				{
					key_code=	KEY_NONE;
					
					return ENABLE;
				}
			}
	else
		 if(value-key_tmp>CONST_OFFSET)
			{
					key_code=	KEY_NONE;
					
					return ENABLE;
				}
	//�޼�����
	if(value>KEY_NONE_MIN)
		key_code=	KEY_NONE;
	else 	//��1����
		if(value>KEY1_ADC_MIN&&value<KEY1_ADC_MAX)
			key_code=	KEY_VALUE1;
	else 	//��2����
		if(value>KEY2_ADC_MIN&&value<KEY2_ADC_MAX)
			key_code=	KEY_VALUE2;
	else 	//��3����
		if(value>KEY3_ADC_MIN&&value<KEY3_ADC_MAX)
			key_code=	KEY_VALUE3;		
	else 	//��4����
		if(value>KEY4_ADC_MIN&&value<KEY4_ADC_MAX)
			key_code=	KEY_VALUE4;	
	else	//��������
		key_code=	KEY_NONE;//KEY_ERR0;
	
	return ENABLE;
}
//���ؾ�����̵������ֵ
uchar KeyValue_Read(void)
{
	static uchar key;

	if(key!=key_code)
	{
		key=key_code;
                
                return  KEY_NONE;
	}
	else
	{
		return key;

        }
}
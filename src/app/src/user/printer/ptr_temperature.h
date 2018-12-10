#ifndef _PTR_TEMPERATURE_H
#define _PTR_TEMPERATURE_H 

struct _T_ctrl
{
	char status;
	u32 delay;
	u16 cnt;
	u32 conv_sum;
    char finished;
	u32 finished_keep_time;
	s8 temperature;
};


//------------Êä³öº¯Êý---------------
extern void ptr_adc_cfg(void);
extern void temperature_detect_tick(void);
extern int start_T_datect(void);
extern int force_start_T_datect(void);
extern s16 get_temperature(void);
extern s8 is_T_detect_finished(void);
extern int is_too_hot(void);
//-----------------------------------

#endif


/**
  ******************************************************************************
  * @file    modem_state.h 
  * @author  miaoyahan
  * @version V1.0.0
  * @date    2013-08-21 
  * @brief   
  ******************************************************************************
  * @attention
  ******************************************************************************
  */ 
#ifndef __MODEM_STATE_H
#define __MODEM_STATE_H

/*
*********************************************************************************************************
*                                               DEFINES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                          GLOBAL VARIABLES
*********************************************************************************************************
*/   

/*
*********************************************************************************************************
*                                            FUNCTION PROTOTYPES
*********************************************************************************************************
*/
MODEM_STA  Modem_State_Is (void);

u8    Modem_State_Type (u8 type);
u8    Modem_State_Tts_Is(u8 type);
u8    Modem_State_Tts_Dt(u8 type, const u8 *pSrc, u16 SrcLen);
void  Modem_State_Tts_Pro(void);
void  Modem_State_Push (void);
u8    Modem_State_Sms_Dt(MODSMS_TYPE Type,u8 *pNum,u8 *pData, u8 len);

BOOL  Modem_State_SocSta (u8 soc);
void  Modem_State_Init (void);
void  Modem_State_Delay(u16 delays,MODON_STA state);
void  Modem_State_CheckRun (void);
void  Modem_State_Switch (MODEM_STA state);
void  Modem_State_SwitchOn (MODON_STA state);
void  Modem_State_SwitchPhone (MODPH_STA state);
void  Modem_State_Off (void);
void  Modem_State_Err (MODEM_STA staok);

void  Modem_State_Act (MODEM_STA staok,MODEM_STA staerr);
void  Modem_State_Ati (MODEM_STA staok,MODEM_STA staerr);
void  Modem_State_Aud (MODEM_STA staok,MODEM_STA staerr);
void  Modem_State_NetSer  (MODEM_STA staok,MODEM_STA staerr);
void  Modem_State_NetCfg (MODEM_STA staok,MODEM_STA staerr);
void  Modem_State_Sms (MODEM_STA staok,MODEM_STA staerr);
void  Modem_State_Dial(MODEM_STA staok, MODEM_STA staerr);
void  Modem_State_Tts (MODEM_STA staok,MODEM_STA staerr);
void  Modem_State_Phone_Break(void);
void  Modem_State_Phone_Enter(void);


void  Modem_State_OnLine (void);

#endif

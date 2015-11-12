#ifndef __NPCA110X_H__
#define __NPCA110X_H__
#include "type.h"
#ifdef FUNC_NPCA110X_EN
#define NPCA110X_ADDR 0xE6

typedef enum _INPUT_CH
{
	INPUT_CH0 = 0,	//line in
	INPUT_CH1,		//bt
	INPUT_CH2		//music	
}INPUT_CH;

BOOL NPCA110X_Init(VOID);
BOOL NPCA110X_SetBass(USHORT i, BassLedCallbackFun BassCallFun);
BOOL NPCA110X_NormalBass(USHORT i, BassLedCallbackFun BassCallFun);
void Set_AudioCodec_Volume(BYTE volume);
BOOL NPCA110X_SetOutMute(BOOL on);
BOOL NPCA110X_ADC_Input_CH_Select(INPUT_CH ch);
BOOL NPCA110X_DAC1_Set_Volume_and_Mute(BYTE Vol);



#endif
#endif


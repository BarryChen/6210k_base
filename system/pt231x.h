#ifndef __PT231X_H__
#define __PT231X_H__
#include "type.h"

#define	PT231X_AD		0x88  

#define		PT231X_LINEIN		0
#define		PT231X_MP3FM		1
#define		PT231X_NONE			2

#define		VOL_MAIN			0
#define		VOL_TREB			1
#define		VOL_BASS			2

extern BYTE	VolSel;
extern BYTE 	VolType;	
extern BYTE	VolMain;
extern BYTE	VolTreb;	
extern BYTE	VolBass;	
extern BYTE	VolFront;
extern BYTE	VolRear;

void PT2313SelInput(BYTE input);
void PT2313MuteOnOff(BYTE MuteOn);
void PT2313Init(void);
void PT2313VolAdd(void);
void PT2313TrebBassSetDefault();
void PT2313VolSub(void);
VOID PT2315E_LoudnessOnOff(VOID);
VOID PT2315E_Do(BYTE Message);
#endif

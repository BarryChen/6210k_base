#ifndef __TOUCHKEY_CTRL_H__
#define __TOUCHKEY_CTRL_H__


#define DOUT_SMOOTH_LEN				6
#define TOUCHKEY_STOP_WORK			0x00
#define TOUCHKEY_START_WORK			0x60

//TouchKeyChInfo
typedef struct __TOUCH_KEY_INFO
{
	WORD	Base;
	SWORD	Delta;
	WORD	Threshold; 		
	WORD	SmoothBuffer[6];
	BYTE	SmoothCount;
	WORD	SmoothOutput;
//	WORD	BaseBuffer[4];
	WORD	BaseCount;	
	BYTE	KeyState;
	BYTE	RepCount;		
}TOUCH_KEY_INFO;

typedef struct	__TOUCH_KEY
{
	BYTE	ChannelEn;
	BYTE	Index;	
	BYTE	BaseCheckState;
	BYTE	BaseFirFlag;	
	TOUCH_KEY_INFO ChData[8];	
}TOUCH_KEY;

extern TOUCH_KEY TK;
extern BYTE gTouchKeyVol;

VOID TouchKeyDoutPro(VOID);

#endif
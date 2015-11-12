#include <reg51.h>										//include public header files
#include <stdio.h>

#include "type.h"										//include project header files
#include "syscfg.h"
#include "utility.h"
#include "debug.h"
#include "timer.h"
#include "sysctrl.h"
#include "key.h"
#include "touch_key.h"
#include "gpio.h"
#include "touchkey_ctrl.h"
#include "message.h"
#include "touchkeylib.h"
#include "touch_key.h"
#include "touchkey_ctrl.h"

#ifdef FUNC_TOUCH_KEY_EN

#define 	TOUCH_KEY_SCAN_TIME			10
#define 	TOUCH_KEY_JTTER_TIME		10
#define 	TOUCH_KEY_CP_TIME			1000
#define		TOUCH_KEY_PWRDOWN_CP_TIME	2000	//cp-time for power-down key
#define 	TOUCH_KEY_CPH_TIME			450

#define		TOUCH_KEY_COUNT				11

#define 	TOUCH_KEY_POWER0_IDX		8		//POWER-DOWN KEY index.
#define 	TOUCH_KEY_POWER1_IDX		15		//POWER-DOWN KEY index.
#define 	TOUCH_KEY_POWER2_IDX		20

typedef enum _TOUCH_KEY_STATE
{
	TOUCH_KEY_STATE_IDLE,
	TOUCH_KEY_STATE_JITTER,
	TOUCH_KEY_STATE_PRESS_DOWN,
	TOUCH_KEY_STATE_CP

} TOUCH_KEY_STATE;

TIMER			TouchKeyInitTimer;
TIMER			TouchKeyWaitTimer;
TIMER			TouchKeyScanTimer;
TOUCH_KEY_STATE	TouchKeyState;

extern BYTE gTouchKeyVol;


static CODE BYTE TouchKeyEvent[][4] = 
{
//	SP						CPS					CPH					CPR
	{MSG_VOL_ADD,		MSG_VOL_ADD,		MSG_VOL_ADD,			MSG_NONE	},	//SW1	
	{MSG_VOL_SUB,	MSG_VOL_SUB,			MSG_VOL_SUB,			MSG_NONE		},	//SW2
	{MSG_NONE,		MSG_NONE,		MSG_NONE,			MSG_NONE	},	//SW3	
	{MSG_NONE,		MSG_NONE,		MSG_NONE,		MSG_NONE		},	//SW4
	{MSG_NONE,		MSG_NONE,		MSG_NONE,		MSG_NONE		},	//SW5	
	{MSG_PLAY_PAUSE,		MSG_NONE,			MSG_NONE,			MSG_NONE		},	//SW6
	{MSG_NEXT,		MSG_FF_START,			MSG_NONE,			MSG_NONE		},	//SW7	
	{MSG_PRE, 		MSG_FB_START,			MSG_NONE,			MSG_NONE		},	//SW8
};


BYTE TKChannlCk(BYTE Mask)
{
	BYTE i;
	
	for(i = 0; i < 8; i++)
	{
		if(Mask & (1 << i))
		{
			return i;
		}
	}
}


VOID TouchKeyScanInit(VOID)
{
//	BYTE i;

	TR1  = 0;
	RstCts();
	TouchKeyPortInit(TK_FREQU_OUTPUTPIN_A6, TOUCHKEYPIN_MASK);
	memset(&TK, 0, sizeof(TK));
//	TK.Start = TOUCHKEY_START_WORK;
	TK.ChannelEn = TOUCHKEYPIN_MASK;
	TK.Index = TKChannlCk(TOUCHKEYPIN_MASK);
	TK.BaseFirFlag = 0xFF;
	//TK.BaseCheckState = 0;
	gTouchKeyVol = 0;
	//非常重要
/*测试板  参数*/
//	TK.ChData[0].Threshold = 10;	
//	TK.ChData[1].Threshold = 10;
//	TK.ChData[2].Threshold = 9;
//	TK.ChData[3].Threshold = 12;
//	TK.ChData[4].Threshold = 20;
//	TK.ChData[5].Threshold = 15;
//	TK.ChData[6].Threshold = 6;
//	TK.ChData[7].Threshold = 11;

/*开发板 参数*/
	TK.ChData[0].Threshold = 40;	
	TK.ChData[1].Threshold = 40;
	TK.ChData[2].Threshold = 40;
	TK.ChData[3].Threshold = 40;
	TK.ChData[4].Threshold = 40;
	TK.ChData[5].Threshold = 40;//15;
	TK.ChData[6].Threshold = 40;//6;
	TK.ChData[7].Threshold = 40;//11;

	TouchKeyState = TOUCH_KEY_STATE_IDLE;
	TimeOutSet(&TouchKeyScanTimer, 1000);

	TouchKeyStartConvert(TK.Index);//首次启动
	//TR1  = 1;
}


static BYTE	TouchChannelKeyGet(VOID)							
{
	if(gTouchKeyVol == 0)
	{
		return -1;
	}
	else
	{
		DBG1(("TK.Index = %02BX\n", TK.Index));
		DBG1(("gTouchKeyVol = %02BX\n", gTouchKeyVol));
		return TKChannlCk(gTouchKeyVol);		
	}
}


// Key process, image key value to key event.
MESSAGE TouchKeyEventGet(VOID)							
{
	static 	BYTE PreKeyIndex = -1;
	static  BYTE KeyIndex;
	
//	if(IsTimeOut(&TouchKeyInitTimer))//更新 base
//	{
//		TouchKeyScanInit();
//		TR1 = 1;
//		TimeOutSet(&TouchKeyInitTimer, 5000);				
//	}
	if(!IsTimeOut(&TouchKeyScanTimer))//防止上电时候会带来干扰，
	{
		return MSG_NONE;
	}
	TimeOutSet(&TouchKeyScanTimer, TOUCH_KEY_SCAN_TIME);

	KeyIndex = TouchChannelKeyGet();

	switch(TouchKeyState)
	{
		case TOUCH_KEY_STATE_IDLE:
			if(KeyIndex == -1)
			{
				return MSG_NONE;
			}
			PreKeyIndex = KeyIndex;
			TouchKeyState = TOUCH_KEY_STATE_JITTER;//设置抖动时间
				
		case TOUCH_KEY_STATE_JITTER:
			if(PreKeyIndex != KeyIndex)
			{
				TouchKeyState = TOUCH_KEY_STATE_IDLE;//无效
			}
			else
			{
 				TimeOutSet(&TouchKeyWaitTimer, TOUCH_KEY_CP_TIME);
				TouchKeyState = TOUCH_KEY_STATE_PRESS_DOWN;
			}
			break;

		case TOUCH_KEY_STATE_PRESS_DOWN:
			if(PreKeyIndex != KeyIndex)//本次 和 上次的键值不同  或者  松开按键了   返回短按事件
			{
				//DBG(("SHORT KEY\n"));
				TouchKeyState = TOUCH_KEY_STATE_IDLE;
				return TouchKeyEvent[PreKeyIndex][0];
			}
			else if(IsTimeOut(&TouchKeyWaitTimer))//本次与上次按键相同  且没有松手
			{
				//return key cp value
				//DBG(("LONG KEY START\n"));
				TimeOutSet(&TouchKeyWaitTimer, TOUCH_KEY_CPH_TIME);
				TouchKeyState = TOUCH_KEY_STATE_CP;
				return TouchKeyEvent[PreKeyIndex][1];
			}
			break;

		case TOUCH_KEY_STATE_CP:
			if(PreKeyIndex != KeyIndex)
			{
				//return key cp value
				//DBG(("LONG SF!\n"));
				TouchKeyState = TOUCH_KEY_STATE_IDLE;
				return TouchKeyEvent[PreKeyIndex][3];
			}
			else if(IsTimeOut(&TouchKeyWaitTimer))
			{
				//return key cph value
				//DBG(("LONG HOLD !\n"));
				TimeOutSet(&TouchKeyWaitTimer, TOUCH_KEY_CPH_TIME);
				return TouchKeyEvent[PreKeyIndex][2];
			}
			break;

		default:
			TouchKeyState = TOUCH_KEY_STATE_IDLE;
			break;
	}
	return MSG_NONE;
}

#endif

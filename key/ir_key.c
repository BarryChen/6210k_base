#include <reg51.h>

#include "type.h"
#include "syscfg.h"
#include "utility.h"
#include "debug.h"
#include "sysctrl.h"
#include "key.h"
#include "timer.h"
#include "ir_key.h"
#include "ir.h"
#include "message.h"
#include "spi_fs.h"
#include "user_interface.h"


#ifdef FUNC_IR_KEY_EN

#define IR_KEY_C_DEBUG

#define 	IR_KEY_SCAN_TIME			5
#define 	IR_KEY_JTTER_TIME			30
#define		IR_KEY_CP_TIME				500	//1000	//CP condition is 1s
#define 	IR_KEY_CPH_TIME				300	//300

#define		IR_KEY_SUM					28		//IR KEY SUM

#define		IR_KEY_PWRDOWN_INDEX		0		//power-down key index
#define		IR_KEY_PWRDOWN_CP_TIME		3000	//cp-time for power-down key


typedef enum _IR_KEY_STATE
{
	IR_KEY_STATE_IDLE,
	IR_KEY_STATE_JITTER,
	IR_KEY_STATE_PRESS_DOWN,
	IR_KEY_STATE_CP
} IR_KEY_STATE;

static TIMER			IrKeyHoldTimer;
static TIMER			IrKeyWaitTimer;
static TIMER			IrKeyLockTimer;

static TIMER			IrKeyScanTimer;
static IR_KEY_STATE 	IrKeyState;

static DWORD 		IrKeyVal;
static BYTE			IrClkDivRate;

extern BOOL GetIrIsOn(void);
extern BOOL GetSpiPlay();
#if 0
static CODE BYTE gIrVal[IR_KEY_SUM] = 					//TOP
{
	0xED,						// POWER
	0xE3,						// Mode
	0xFA,						// VOL+
	0xE1,						//Mute

	0xFE,						// SD
	0xF8,						// PREV
	0xF7,						// PLAY/PAUSE
	0xF6,						// NEXT

	0xFD,						// USB
	0xF5,						// TREB+
	0xE4,						// VOL-
	0xE0,						// BASS+

	0xEC,						// AUX
	0xF3,						// TREB-
	0xF2,						// DEFAULT
	0xF1,						// BASS-

	0xFB,						// 100+
	0xF9,						// 200+
	0xEF,						// 1
	0xBB,						// 2
	
	0xBF,						// 3
	0xEB,						// 4
	0xB9,						// 5
	0xE9,						// 6		

	0xAB,						// 7
	0xA9,						// 8
	0xAF,						// 9
       0xFF,						// 0
};

#else
static CODE USHORT gIrVal1[IR_KEY_SUM] = 					//TOP
{
	0x5AA5,
	0x5FA0,	
	0x5EA1,
	0x5DA2,
	0x5CA3,
	0x59A6,
	0x5BA4,
};

static CODE BYTE gIrVal2[IR_KEY_SUM] = 				//sunplus
{
	0xEA,						// VOL-
	0xF6,						// VOL+
	0xE9,						// 0				

	0xF3,						// 1
	0xE7,						// 2
	0xA1,						// 3

	0xF7,						// 4
	0xE3,						// 5
	0xA5,						// 6

	0xBD,						// 7
	0xAD,						// 8
	0xB5,						// 9

	0xBA,						// POWER
	0xB9,						// Mode
	0xBB,						// PLAY/PAUSE //CH+

	0xE6,						// RPT
	0xB8,						//Mute
	0xF2,						//AUTO scan

	0xF8,						// EQ
	0xBF,						// PREV
	0xBC,						// NEXT
};

static CODE BYTE gIrVal3[IR_KEY_SUM] = 				// JIAN rong
{
	0xE9,						// VOL-
	0xED,						// VOL+
	0xEF,						// 0				

	0xFB,						// 1
	0xFA,						// 2
	0xF9,						// 3

	0xF7,						// 4
	0xF6,						// 5
	0xF5,						// 6

	0xF3,						// 7
	0xF2,						// 8
	0xF1,						// 9

	0xFF,						// POWER
	0xFD,						// Mode
	0xFE,						// PLAY/PAUSE //AUTO SCAN

	0xEE,						// 10+
	0xE7,						//FB //FAST REWIND
	0xE6,						//FF //FAST FORWARD

	0xE5,						// EQ
	0xEB,						// PREV //CH-//SCAN DW
	0xEA,						// NEXT//CH+//SCAN UP
};

//A
static CODE BYTE gIrVal4[IR_KEY_SUM + 5] = 
{
	0xFA,						// VOL-
	0xF9,						// VOL+
	0xF7,						// 0				

	0xF3,						// 1
	0xF2,						// 2
	0xF1,						// 3

	0xEF,						// 4
	0xEE,						// 5
	0xED,						// 6

	0xEB,						// 7
	0xEA,						// 8
	0xE9,						// 9

	0xE1,						// STOP
	0xE5,						// Mode
	0xFF,						// PLAY/PAUSE //CH+

	0xE7,						// PICK SONG
	0xFE,						//10- //FREQ -
	0xFD,						//10+//FREQ +

	0xFB,						// EQ
	0xF6,						// PREV
	0xF5,						// NEXT

	0x01,						// USB
	0x02,						// SD
	0x03,						// FM
	0x04,						// LINE IN
	0xE1,						// STOP	
};
#endif

static CODE BYTE IrKeyEvent[][4] =  	 
{
#if 0
//	SP					CPS					CPH					CPR	
	{MSG_POWER, 		MSG_POWER, 		MSG_NONE, 		MSG_NONE		},	// POWER
	{MSG_MODE_SW, 		MSG_NONE, 		MSG_NONE, 		MSG_NONE		},	// Mode
	{MSG_VOL_ADD, 		MSG_VOL_ADD, 		MSG_VOL_ADD, 		MSG_NONE		},	// VOL+
	{MSG_MUTE, 		MSG_NONE, 		MSG_NONE, 		MSG_NONE		},	// Mute

	{MSG_MODE_SD, 		MSG_NONE, 		MSG_NONE, 		MSG_NONE		},	// SD
	{MSG_PRE, 			MSG_FB_START, 		MSG_NONE, 			MSG_FF_FB_END	},	// PREV
	{MSG_PLAY_PAUSE, 		MSG_NONE, 		MSG_NONE, 		MSG_NONE		},	// PLAY/PAUSE
	{MSG_NEXT, 			MSG_FF_START, 		MSG_NONE, 			MSG_FF_FB_END	},	// NEXT

	{MSG_MODE_USB, 		MSG_NONE, 		MSG_NONE, 		MSG_NONE		},	// USB
	{MSG_TREBUP, 		MSG_TREBUP, 		MSG_TREBUP, 		MSG_NONE		},	// TREB+
	{MSG_VOL_SUB, 		MSG_VOL_SUB, 		MSG_VOL_SUB, 		MSG_NONE		},	// VOL-
	{MSG_BASSUP, 		MSG_BASSUP, 		MSG_BASSUP, 		MSG_NONE		},	// BASS+

	{MSG_MODE_AUX, 		MSG_NONE, 		MSG_NONE, 		MSG_NONE		},	// AUX
	{MSG_TREBDN, 		MSG_TREBDN, 		MSG_TREBDN, 		MSG_NONE		},	// TREB-
	{MSG_DEFAULT, 		MSG_NONE, 		MSG_NONE, 		MSG_NONE		},	// DEFAULT
	{MSG_BASSDN, 		MSG_BASSDN, 		MSG_BASSDN, 		MSG_NONE		},	// BASS-

	{MSG_100TRACK_ADD, 		MSG_NONE, 		MSG_NONE, 		MSG_NONE		},	// 100+
	{MSG_200TRACK_ADD, 		MSG_NONE, 		MSG_NONE, 		MSG_NONE		},	// 200+
	{MSG_NUM_1, 		MSG_NUM_1CP, 		MSG_NONE, 			MSG_NONE		},	// 1
	{MSG_NUM_2, 		MSG_NUM_2CP, 		MSG_NONE, 			MSG_NONE		},	// 2

	{MSG_NUM_3, 		MSG_NUM_3CP, 		MSG_NONE, 			MSG_NONE		},	// 3
	{MSG_NUM_4, 		MSG_NUM_4CP, 		MSG_NONE, 			MSG_NONE		},	// 4
	{MSG_NUM_5, 		MSG_NUM_5CP, 		MSG_NONE, 			MSG_NONE		},	// 5
	{MSG_NUM_6, 		MSG_NUM_6CP, 		MSG_NONE, 			MSG_NONE		},	// 6

	{MSG_NUM_7, 		MSG_NUM_7CP, 		MSG_NONE, 			MSG_NONE		},	// 7
	{MSG_NUM_8, 		MSG_NUM_8CP, 		MSG_NONE, 			MSG_NONE		},	// 8
	{MSG_NUM_9, 		MSG_NUM_9CP, 		MSG_NONE, 			MSG_NONE		},	// 9
	{MSG_NUM_0, 		MSG_NUM_0CP, 		MSG_NONE, 			MSG_NONE		},	// 0
#elif defined(AU6210K_ZB_BT007_CSR) && !defined(AU6210K_ZB_BT007_IR_IC_IS_334M_CSR)
	{MSG_NEXT,			MSG_VOL_ADD,			MSG_VOL_ADD,				MSG_NONE	},	//SW3
	{MSG_PRE,			MSG_VOL_SUB,			MSG_VOL_SUB,				MSG_NONE	},	//SW2	
	{MSG_PLAY_PAUSE,	MSG_NONE,				MSG_NONE,					MSG_NONE	},	//SW4
	{MSG_MODE_SW,		MSG_NONE,				MSG_NONE,					MSG_NONE	},	//SW3
	{MSG_BLUETOOTH_ANSWER,	MSG_BLUETOOTH_REJECT,	MSG_BLUETOOTH_REJECT,		MSG_NONE	},	//SW4
#elif defined(AU6210K_ZB_BT007_CSR) && defined(AU6210K_ZB_BT007_IR_IC_IS_334M_CSR)
	{MSG_PLAY_PAUSE,		MSG_NONE,			MSG_NONE,					MSG_NONE	},	//SW4
	{MSG_MODE_SW,			MSG_IR_ON_OFF,		MSG_NONE,					MSG_NONE	},	//SW3
	{MSG_BLUETOOTH_ANSWER,	MSG_BT_REJECT,		MSG_BLUETOOTH_REJECT,		MSG_NONE	},	//SW4
	{MSG_PRE,				MSG_NONE,			MSG_NONE,					MSG_NONE},	//SW2	
	{MSG_NEXT,				MSG_NONE,			MSG_NONE,					MSG_NONE},	//SW3
	{MSG_V_ADD,				MSG_V_ADD,			MSG_VOL_ADD,					MSG_NONE},	//SW2	
	{MSG_V_SUB,				MSG_V_SUB,			MSG_VOL_SUB,					MSG_NONE},	//SW3

#elif defined(AU6210K_HXX_B002)
	{MSG_NEXT, 			MSG_NONE, 		MSG_NONE, 		MSG_NONE		},	// vol-		idx-0
	{MSG_PLAY_PAUSE, 	MSG_NONE,			MSG_NONE, 			MSG_NONE		},	// vol+		idx-1
	{MSG_VOL_SUB, 		MSG_VOL_SUB, 		MSG_VOL_SUB, 			MSG_NONE		},	// 0		idx-2

	{MSG_NUM_1, 		MSG_NUM_1CP, 		MSG_NONE, 			MSG_NONE		},	// 1		idx-3
	{MSG_NUM_2, 		MSG_NUM_2CP, 		MSG_NONE, 			MSG_NONE		},	// 2		idx-4
	{MSG_NUM_3, 		MSG_NUM_3CP, 		MSG_NONE, 			MSG_NONE		},	// NEXT		idx-5

	{MSG_NUM_4, 		MSG_NUM_4CP, 		MSG_NONE, 			MSG_NONE		},	// EQ		idx-6
	{MSG_NUM_5, 		MSG_NUM_5CP, 		MSG_NONE, 			MSG_NONE		},	// VOL-		idx-7
	{MSG_NUM_6, 		MSG_NUM_6CP, 		MSG_NONE, 			MSG_NONE		},	// VOL+		idx-8

	{MSG_NUM_7, 		MSG_NUM_7CP, 		MSG_NONE, 			MSG_NONE		},	// 0		idx-9
	{MSG_NUM_8, 		MSG_NUM_8CP, 		MSG_NONE, 			MSG_NONE		},	// REPEAT	idx-10
	{MSG_NUM_9, 		MSG_NUM_9CP, 		MSG_NONE, 			MSG_NONE		},	// USB/SD	idx-11
	
	{MSG_POWER, 		MSG_POWER, 			MSG_NONE, 			MSG_NONE		},	// 1		idx-12
	{MSG_REPEAT, 		MSG_NONE, 			MSG_NONE, 			MSG_NONE		},	// 2		idx-13
	{MSG_FREQ_DN, 		MSG_FREQ_DN, 		MSG_FREQ_DN, 			MSG_NONE		},	// 3		idx-14

	{MSG_VOL_ADD, 		MSG_VOL_ADD, 		MSG_VOL_ADD, 			MSG_NONE		},	// 4		idx-15
	{MSG_MODE_SW, 		MSG_NONE, 			MSG_NONE, 			MSG_NONE		},	// 5		idx-16
	{MSG_NUM_0, 		MSG_NUM_0CP, 			MSG_NONE, 			MSG_NONE		},	// 6		idx-17

	{MSG_PRE, 			MSG_NONE, 		MSG_NONE, 			MSG_NONE		},	// 7		idx-18
	{MSG_FREQ_UP,	 	MSG_FREQ_UP, 		MSG_FREQ_UP, 			MSG_NONE	},	//SW13		idx-19
	{MSG_EQ_SW, 		MSG_NONE, 			MSG_NONE, 			MSG_NONE	},	//SW12		idx-20
	
#else
//	SP					CPS					CPH					CPR	
// C & D
	{MSG_VOL_SUB, 		MSG_VOL_SUB, 		MSG_VOL_SUB, 		MSG_NONE		},	// vol-
	{MSG_VOL_ADD, 		MSG_VOL_ADD,		MSG_VOL_ADD, 		MSG_NONE		},	// vol+
	{MSG_NUM_0, 		MSG_NUM_0CP, 		MSG_NONE, 			MSG_NONE		},	// 0

	{MSG_NUM_1, 		MSG_NUM_1CP, 		MSG_NONE, 			MSG_NONE		},	// 1
	{MSG_NUM_2, 		MSG_NUM_2CP, 		MSG_NONE, 			MSG_NONE		},	// 2
	{MSG_NUM_3, 		MSG_NUM_3CP, 		MSG_NONE, 			MSG_NONE		},	// NEXT

	{MSG_NUM_4, 		MSG_NUM_4CP, 		MSG_NONE, 			MSG_NONE		},	// EQ
	{MSG_NUM_5, 		MSG_NUM_5CP, 		MSG_NONE, 			MSG_NONE		},	// VOL-
	{MSG_NUM_6, 		MSG_NUM_6CP, 		MSG_NONE, 			MSG_NONE		},	// VOL+

	{MSG_NUM_7, 		MSG_NUM_7CP, 		MSG_NONE, 			MSG_NONE		},	// 0
	{MSG_NUM_8, 		MSG_NUM_8CP, 		MSG_NONE, 			MSG_NONE		},	// REPEAT
	{MSG_NUM_9, 		MSG_NUM_9CP, 		MSG_NONE, 			MSG_NONE		},	// USB/SD
	
	{MSG_POWER, 		MSG_POWER, 			MSG_NONE, 			MSG_NONE		},	// 1
	{MSG_MODE_SW, 		MSG_NONE, 			MSG_NONE, 			MSG_NONE		},	// 2
	{MSG_PLAY_1, 		MSG_PP_STOP, 		MSG_NONE, 			MSG_NONE		},	// 3

	{MSG_REPEAT, 		MSG_REPEAT, 		MSG_NONE, 			MSG_NONE		},	// 4
	{MSG_MUTE, 			MSG_NONE, 			MSG_NONE, 			MSG_NONE		},	// 5
	{MSG_INTRO, 		MSG_NONE, 			MSG_NONE, 			MSG_NONE		},	// 6

	{MSG_EQ_CH_SUB, 	MSG_NONE, 			MSG_NONE, 			MSG_NONE		},	// 7
	{MSG_PREV1,	 		MSG_FB_START, 		MSG_NONE, 			MSG_FF_FB_END	},	//SW13
	{MSG_NEXT1, 		MSG_FF_START, 		MSG_NONE, 			MSG_FF_FB_END	},	//SW12

//jian rong	--> B
	{MSG_POWER, 		MSG_POWER, 			MSG_NONE, 			MSG_NONE		},	// 
	{MSG_MODE_SW, 		MSG_NONE, 			MSG_NONE, 			MSG_NONE		},	// 
	{MSG_PLAY_PAUSE, 	MSG_PP_STOP, 		MSG_NONE, 			MSG_NONE		},	//

	{MSG_NUM_10ADD, 	MSG_NUM_10ADDCP, 	MSG_NONE, 			MSG_NONE		},	// 10+
	{MSG_FREQ_DN, 		MSG_FB_START, 		MSG_NONE, 			MSG_FF_FB_END	},	// 
	{MSG_FREQ_UP, 		MSG_FF_START, 		MSG_NONE, 			MSG_FF_FB_END	},	// 

	{MSG_EQ_SW, 		MSG_NONE, 			MSG_NONE, 			MSG_NONE		},	//
	{MSG_PRE, 			MSG_10TRACK_SUB, 	MSG_10TRACK_SUBCP, 	MSG_NONE		},	//
	{MSG_NEXT, 			MSG_10TRACK_ADD, 	MSG_10TRACK_ADDCP, 	MSG_NONE		},	//	

// mv remote --> A
	{MSG_STOP, 			MSG_NONE, 			MSG_NONE, 			MSG_NONE		},	// 
	{MSG_MODE_SW, 		MSG_POWER, 			MSG_NONE, 			MSG_NONE		},	//SW9
	{MSG_PLAY_PAUSE, 	MSG_PP_STOP, 		MSG_NONE, 			MSG_NONE		},	// 

	{MSG_IR_SELECT, 	MSG_NONE, 			MSG_NONE, 			MSG_NONE		},	// pick song
	{MSG_FREQDN_10TRK, 	MSG_10TRACK_SUB, 	MSG_NONE, 			MSG_NONE		},	// 
	{MSG_FREQUP_10TRK, 	MSG_10TRACK_ADD, 	MSG_NONE, 			MSG_NONE		},	// 

	{MSG_EQ_SW, 		MSG_REPEAT, 		MSG_REPEAT, 		MSG_NONE		},	// 
	{MSG_PRE, 			MSG_FB_START, 		MSG_NONE, 			MSG_FF_FB_END	},	//
	{MSG_NEXT, 			MSG_FF_START, 		MSG_NONE, 			MSG_FF_FB_END	},	//	

//for ir slave  five cmds
	{MSG_MODE_USB, 		MSG_NONE, 			MSG_NONE, 			MSG_NONE		},	// usb
	{MSG_MODE_SD, 		MSG_NONE, 			MSG_NONE, 			MSG_NONE		},	// sd
	{MSG_MODE_FM, 		MSG_NONE, 			MSG_NONE, 			MSG_NONE		},	// FM 

	{MSG_MODE_AUX, 		MSG_NONE, 			MSG_NONE, 			MSG_NONE		},	// line in
	{MSG_STOP,			MSG_NONE, 			MSG_NONE, 			MSG_NONE		},	// STOP	
#endif
};

extern WORD KeyInLedFlashHoldTmr;
//get ir key index
BYTE GetIrKeyIndex(VOID)
{
	static BYTE KeyIndex = -1;
	static BYTE IrCphCnt = 0;
	BOOL IsIrShortPrs; 
	BOOL IsIrContinuePrs;

	if(IrIsKeyCome())
	{
		KeyInLedFlashHoldTmr = 500;
		IsIrShortPrs = TRUE;
		IsIrContinuePrs = FALSE;
		IrKeyVal = IrGetKeyCode();  
#ifdef IR_KEY_C_DEBUG			
#endif
	DBG1(("Key: %08LX\n", IrKeyVal)); 

	}
	else if(IrIsContinuePrs())
	{
#ifdef IR_KEY_C_DEBUG			
#endif	
			  DBG1(("IrIsContinuePrs\n")); 
              KeyInLedFlashHoldTmr = 500;
		IsIrShortPrs = FALSE;
		IsIrContinuePrs = TRUE;
	} 
	else
	{
		IsIrShortPrs = FALSE;
		IsIrContinuePrs = FALSE;
	}
	if(IsIrShortPrs || IsIrContinuePrs)
	{
		//fast response 
		if(IsIrShortPrs)
		{
			IrCphCnt = 0;
		}
		if(IrCphCnt < 5)
		{
			IrCphCnt++;
		}
		TimeOutSet(&IrKeyHoldTimer, 70 * IrCphCnt);

		if(((BYTE*)(&IrKeyVal))[0] + ((BYTE*)(&IrKeyVal))[1] != 0xFF)
		{
			KeyIndex = -1;
			return -1;
		}
#if 0
		if((((BYTE*)(&IrKeyVal))[2] == 0xFF) 
		     && (((BYTE*)(&IrKeyVal))[3] == 0x00))
		{
			for(KeyIndex = 0; KeyIndex < IR_KEY_SUM; KeyIndex++)
			{
				if(((BYTE*)(&IrKeyVal))[0] == gIrVal[KeyIndex])		
				{
					return KeyIndex;
				}
			}
		}
		if((((BYTE*)(&IrKeyVal))[2] == 0x7F) 
			&& (((BYTE*)(&IrKeyVal))[3] == 0x80))
		{
			for(KeyIndex = 0; KeyIndex < IR_KEY_SUM; KeyIndex++)
			{
				if(((BYTE*)(&IrKeyVal))[0] == gIrVal1[KeyIndex])		
				{
					return KeyIndex;
				}
			}
		}
#else
		if((((BYTE*)(&IrKeyVal))[2] == 0x7F) 
			&& (((BYTE*)(&IrKeyVal))[3] == 0x80))
		{
			for(KeyIndex = 0; KeyIndex < IR_KEY_SUM; KeyIndex++)
			{
				if(((USHORT*)(&IrKeyVal))[0] == gIrVal1[KeyIndex])		
				{
#ifdef AU6210K_ZB_BT007_IR_IC_IS_334M_CSR
					if(gSys.SystemMode == SYS_MODE_BLUETOOTH && GetIrIsOn())
					{
						switch(KeyIndex)
						{
						case 0:						
							baGPIOCtrl[GPIO_D_OUT] |= 0x40; //D6
							break;
						case 2:
							baGPIOCtrl[GPIO_D_OUT] |= 0x20; //D5
							break;
						case 3:
							baGPIOCtrl[GPIO_A_OUT] |= 0x02; //A1
							break;
						case 4:
							baGPIOCtrl[GPIO_A_OUT] |= 0x04; //A2
							break;
						default:
							break;
						}
					}

#endif
					baGPIOCtrl[GPIO_A_OUT] |= 0x40; //A2
				
					return KeyIndex;
				}

				
			}
		}

		else if((((BYTE*)(&IrKeyVal))[2] == 0xFF) 
		     && (((BYTE*)(&IrKeyVal))[3] == 0x00))
		{
			for(KeyIndex = 0; KeyIndex < IR_KEY_SUM; KeyIndex++)
			{
				if(((BYTE*)(&IrKeyVal))[0] == gIrVal2[KeyIndex])		
				{
					return KeyIndex;
				}
			}
		}
		else if((((BYTE*)(&IrKeyVal))[2] == 0xBF)
		     && (((BYTE*)(&IrKeyVal))[3] == 0x00))
		{
			for(KeyIndex = 0; KeyIndex < 12; KeyIndex++)
			{
				if(((BYTE*)(&IrKeyVal))[0] == gIrVal3[KeyIndex])		
				{
					return KeyIndex;
				}
			}
			for(KeyIndex = 12; KeyIndex < IR_KEY_SUM; KeyIndex++)
			{
				if(((BYTE*)(&IrKeyVal))[0] == gIrVal3[KeyIndex])		
				{
					KeyIndex += 9;
					return (KeyIndex);
				}
			}
		}
		else if((((BYTE*)(&IrKeyVal))[2] == 0xFD)
		     && (((BYTE*)(&IrKeyVal))[3] == 0x02))
		{
			for(KeyIndex = 0; KeyIndex < 12; KeyIndex++)
			{
				if(((BYTE*)(&IrKeyVal))[0] == gIrVal4[KeyIndex])		
				{
					return KeyIndex;
				}
			}
			for(KeyIndex = 12; KeyIndex < (IR_KEY_SUM + 5); KeyIndex++)
			{
				if(((BYTE*)(&IrKeyVal))[0] == gIrVal4[KeyIndex])		
				{
					KeyIndex += 18;
					return (KeyIndex);
				}
			}
		}
#endif
		KeyIndex = -1;
		return -1;
	}
	else if(!IsTimeOut(&IrKeyHoldTimer))
	{
		return KeyIndex;
	}
	else
	{
		IrKeyVal = 0;
		return -1;
	}
}


// IR decoder initilize.
VOID IrKeyScanInit(VOID)
{	
	IrKeyState = IR_KEY_STATE_IDLE;
	TimeOutSet(&IrKeyScanTimer, 0);
	TimeOutSet(&IrKeyLockTimer, 0);
	
//#if (POWER_SAVING_MODE_OPTION == POWER_SAVING_MODE_SLEEP)
//#if (WIS_GPIO_IR == WIS_GPB6_LOW)
//	IRChannelSel(IR_INPUT_GPIOB6);
//#elif (WIS_GPIO_IR == WIS_GPE0_LOW)
//	IRChannelSel(IR_INPUT_GPIOE0);	
//#endif
//#else
	//未定义sleep模式下IR唤醒，则客户自己配置IR端口
	IRChannelSel(IR_KEY_PORT);

	IrIgnoreLeadHeader(FALSE);
	IrInit();

	baGPIOCtrl[GPIO_D_IE] &= ~0x20;//D5
	baGPIOCtrl[GPIO_D_OE] |= 0x20;
	baGPIOCtrl[GPIO_D_PU] |= 0x20;
	baGPIOCtrl[GPIO_D_PD] |= 0x20; 
	baGPIOCtrl[GPIO_D_OUT] &= ~0x10; 
	WaitMs(2);
	baGPIOCtrl[GPIO_D_IE] &= ~0x40;//D6
	baGPIOCtrl[GPIO_D_OE] |= 0x40;
	baGPIOCtrl[GPIO_D_PU] |= 0x40;
	baGPIOCtrl[GPIO_D_PD] |= 0x40; 
	baGPIOCtrl[GPIO_D_OUT] &= ~0x40; 
	WaitMs(2);
	baGPIOCtrl[GPIO_A_IE] &= ~0x04;//A2
	baGPIOCtrl[GPIO_A_OE] |= 0x04;
	baGPIOCtrl[GPIO_A_PU] |= 0x04;
	baGPIOCtrl[GPIO_A_PD] |= 0x04; 
	baGPIOCtrl[GPIO_A_OUT] &= ~0x04; 
	WaitMs(2);
	baGPIOCtrl[GPIO_A_IE] &= ~0x02;//A2
	baGPIOCtrl[GPIO_A_OE] |= 0x02;
	baGPIOCtrl[GPIO_A_PU] |= 0x02;
	baGPIOCtrl[GPIO_A_PD] |= 0x02; 
	baGPIOCtrl[GPIO_A_OUT] &= ~0x02; 
	WaitMs(2);
	
}


// Key process, image key value to key event.
MESSAGE IrKeyEventGet(VOID)							
{
	static 	BYTE 	PreKeyIndex = -1;
	BYTE			KeyIndex;

	if(!IsTimeOut(&IrKeyScanTimer))
	{
		return MSG_NONE;
	}
	TimeOutSet(&IrKeyScanTimer, IR_KEY_SCAN_TIME);	
	//DBG1(("GetSpiPlay() = %d\n",GetSpiPlay()));
//	if(GetSpiPlay())
	//{
	//	KeyIndex == -1;
	//}else
	{
		KeyIndex = GetIrKeyIndex();
	//DBG(("*PreKeyIndex:%bx, KeyIndex:%bx\n", PreKeyIndex, KeyIndex));
	}

	switch(IrKeyState)
	{
		case IR_KEY_STATE_IDLE:
			if(KeyIndex == -1)
			{
				baGPIOCtrl[GPIO_D_OUT] &= ~0x20;	
				baGPIOCtrl[GPIO_D_OUT] &= ~0x40;	
				baGPIOCtrl[GPIO_A_OUT] &= ~0x04;	
				baGPIOCtrl[GPIO_A_OUT] &= ~0x02;
				if(IsTimeOut(&IrKeyLockTimer))
				{
					//DBG1(("timer out!!\n"));
					baGPIOCtrl[GPIO_A_OUT] &= ~0x40; //A2
				}

				
				return MSG_NONE;
			}

			PreKeyIndex = KeyIndex;
			if((PreKeyIndex == 12) || (PreKeyIndex == 21))
			{	 
				TimeOutSet(&IrKeyWaitTimer, IR_KEY_PWRDOWN_CP_TIME);	
			}
			else
			{
				TimeOutSet(&IrKeyWaitTimer, IR_KEY_CP_TIME);
			}
			IrKeyState = IR_KEY_STATE_PRESS_DOWN;
			break;

		case IR_KEY_STATE_PRESS_DOWN:
			if(!GetIrIsOn())
			{
									//DBG1(("keep timer out!!\n"));
				TimeOutSet(&IrKeyLockTimer, 2000);	
			}
			else
			{
				DBG1(("keep timer 222!!\n"));
				TimeOutSet(&IrKeyLockTimer, 0);	 //A2
			}
			if(PreKeyIndex != KeyIndex)
			{
#ifdef IR_KEY_C_DEBUG				
				DBG1(("IR SP: %02BX\n", IrKeyEvent[PreKeyIndex][0]));
#endif				
				IrKeyState = IR_KEY_STATE_IDLE;

				if(PreKeyIndex != 1)
				switch(PreKeyIndex)
				{
				case 0:						
					baGPIOCtrl[GPIO_D_OUT] &= ~0x40; //D6
					break;
				case 2:
					baGPIOCtrl[GPIO_D_OUT] &= ~0x20; //D5
					break;
				case 3:
					baGPIOCtrl[GPIO_A_OUT] &= ~0x02; //A1
					break;
				case 4:
					baGPIOCtrl[GPIO_A_OUT] &= ~0x04; //A2
					break;
				default:
					break;
				}

				return IrKeyEvent[PreKeyIndex][0];
			}
			else if(IsTimeOut(&IrKeyWaitTimer))
			{
#ifdef IR_KEY_C_DEBUG			
				DBG(("IR CPS: %02BX\n", IrKeyEvent[PreKeyIndex][1]));
#endif

				if(PreKeyIndex == 36)	
				{	 
					TimeOutSet(&IrKeyWaitTimer, 1000);	
				}
				else
				{
					TimeOutSet(&IrKeyWaitTimer, IR_KEY_CPH_TIME);
				}
				//TimeOutSet(&IrKeyWaitTimer, IR_KEY_CPH_TIME);
				IrKeyState = IR_KEY_STATE_CP;
				return IrKeyEvent[PreKeyIndex][1];
			}
			break;

		case IR_KEY_STATE_CP:
			if(PreKeyIndex != KeyIndex)
			{
#ifdef IR_KEY_C_DEBUG			
				DBG(("IR CPR: %02BX\n", IrKeyEvent[PreKeyIndex][3]));
#endif				
				IrKeyState = IR_KEY_STATE_IDLE;
				return IrKeyEvent[PreKeyIndex][3];
			}
			else if(IsTimeOut(&IrKeyWaitTimer))
			{
#ifdef IR_KEY_C_DEBUG			
				DBG(("IR CPH: %02BX\n", IrKeyEvent[PreKeyIndex][2]));
#endif				
				if(PreKeyIndex == 36)	
				{	 
					TimeOutSet(&IrKeyWaitTimer, 1000);	
				}
				else
				{
					TimeOutSet(&IrKeyWaitTimer, IR_KEY_CPH_TIME);
				}
				return IrKeyEvent[PreKeyIndex][2];
			}
			break;

		default:
			IrKeyState = IR_KEY_STATE_IDLE;
			break;
	}
	return MSG_NONE;
}

#endif

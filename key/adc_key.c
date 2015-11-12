#include <reg51.h>										//include public header files

#include "type.h"										//include project header files
#include "syscfg.h"
#include "utility.h"
#include "debug.h"
#include "timer.h"
#include "sysctrl.h"
#include "key.h"
#include "adc_key.h"
#include "gpio.h"
#include "adc.h"
#include "message.h"
#include "device.h"
#include "seg_panel.h"
#include "seg_led_disp.h"


#ifdef FUNC_ADC_KEY_EN

#if (defined(FUNC_LED_ADC1_KEY_MULTIPLE_EN) || defined(FUNC_LED_ADC2_KEY_MULTIPLE_EN))
#define		ADC_KEY_SCAN_TIME			20
#define		ADC_KEY_JTTER_TIME			40
#else
#define		ADC_KEY_SCAN_TIME			10
#define		ADC_KEY_JTTER_TIME			20
#endif
#define		ADC_KEY_CP_TIME				600	//1000
#define		ADC_KEY_PWRDOWN_CP_TIME		2000	//cp-time for power-down key
#define		ADC_KEY_CPH_TIME			200		//450
#define		ADC_KEY_HPPUSHOUT_CP_TIME	200

//#define		ADC_KEY_COUNT				11

#define 	ADC_KEY_POWER0_IDX			8		//POWER-DOWN KEY index.
#define 	ADC_KEY_POWER1_IDX			15		//POWER-DOWN KEY index.
#define 	ADC_KEY_POWER2_IDX			20

#ifdef HEADPHONE_ADC_PORT_CH
#ifdef HEADPHONE_ADC_PU_EN
#define 	ADC_KEY_HPPUSHOUT_IDX		23
#define 	ADC_KEY_HPPUSHIN_IDX		22
#else
#define 	ADC_KEY_HPPUSHOUT_IDX		22
#define 	ADC_KEY_HPPUSHIN_IDX		23
#endif
#endif

typedef enum _ADC_KEY_STATE
{
	ADC_KEY_STATE_IDLE,
	ADC_KEY_STATE_JITTER,
	ADC_KEY_STATE_PRESS_DOWN,
	ADC_KEY_STATE_CP

} ADC_KEY_STATE;

TIMER			AdcKeyWaitTimer;
TIMER			AdcKeyScanTimer;
ADC_KEY_STATE	AdcKeyState;


#ifdef FUNC_BEEP_SOUND_EN 	
extern BOOL gPushKeyNeedBeepFlag;
#endif

#ifdef FUNC_SEG_LED_EN
extern BOOL gLedDispRefreshFlag;
#endif

#if defined(AU6210K_JLH_JH82B)
#define		ADC_KEY_COUNT				3
static CODE BYTE AdcKeyEvent[][4] = 
{
//	SP						CPS					CPH					CPR	
	{MSG_PRE,			MSG_VOL_SUB,		MSG_VOL_SUB,	MSG_NONE	},	//SW2	
	{MSG_NEXT,			MSG_VOL_ADD,		MSG_VOL_ADD,	MSG_NONE	},	//SW3
	{MSG_PLAY_PAUSE,	MSG_MODE_SW,		MSG_NONE,		MSG_NONE	},	//SW1
};


BYTE  CODE KeyVal1[ADC_KEY_COUNT] = 
{ 
#if 1			//上拉10K
	4,	//SW3	0 		0	0	//SW13
	17,	//SW4	1.5k		8	8	//SW14
	30,	//SW5	6.8K 		26	26	//SW15
#else
	3,	//SW3	0 		0	0	//SW13
	9,	//SW4	2.2k		6	6	//SW14
	14,	//SW5	4.7K 		11	12	//SW15
#endif
};

#elif defined(AU6210K_NR_D_9X)
#define		ADC_KEY_COUNT				3
static CODE BYTE AdcKeyEvent[][4] = 
{
//	SP						CPS					CPH					CPR	
	{MSG_PLAY_PAUSE,		MSG_NONE,			MSG_NONE,		MSG_NONE	},	//SW4
	{MSG_NEXT,			MSG_VOL_ADD,		MSG_VOL_ADD,	MSG_NONE	},	//SW3
	{MSG_PRE,			MSG_VOL_SUB,		MSG_VOL_SUB,	MSG_NONE	},	//SW2	
};


BYTE  CODE KeyVal1[ADC_KEY_COUNT] = 
{ 
	3,	//SW3	0 		0	0	//SW13
	9,	//SW4	2.2k		6	6	//SW14
	14,	//SW5	4.7K 		11	12	//SW15
};

/*#elif defined(AU6210K_NR_D_9X_XJ_HTS)
#define		ADC_KEY_COUNT				3
static CODE BYTE AdcKeyEvent[][4] = 
{
//	SP						CPS					CPH					CPR	
	{MSG_PLAY_PAUSE,	MSG_BLUETOOTH_REJECT,			MSG_NONE,		MSG_NONE	},	//SW4
	{MSG_PRE,			MSG_VOL_SUB,		MSG_VOL_SUB,	MSG_NONE	},	//SW2	
	{MSG_NEXT,			MSG_VOL_ADD,		MSG_VOL_ADD,	MSG_NONE	},	//SW3
};


BYTE  CODE KeyVal1[ADC_KEY_COUNT] = 
{ 
	3,	//SW3	0 		0	0	//SW13
	9,	//SW4	2.2k		6	6	//SW14
	14,	//SW5	4.7K 		11	12	//SW15
};*/
#elif defined(AU6210K_NR_D_9X_XJ_HTS)
#define		ADC_KEY_COUNT				5
static CODE BYTE AdcKeyEvent[][4] = 
{
//	SP						CPS					CPH					CPR	
	{MSG_VOL_SUB, 	    MSG_NONE, 			    MSG_NONE, 			MSG_NONE		},	//SW1	
	{MSG_NEXT, 			MSG_MODE_SW, 		    MSG_NONE, 		    MSG_NONE		},	//SW2	
	{MSG_PRE, 			MSG_BT_DISCONNECT, 		        MSG_NONE, 		    MSG_NONE		},	//SW3
	{MSG_PLAY_PAUSE, 	MSG_BLUETOOTH_REJECT,   MSG_NONE, 			MSG_NONE		}, //SW4
	{MSG_VOL_ADD, 		MSG_NONE, 			    MSG_NONE, 			MSG_NONE		},	//SW5
};


BYTE  CODE KeyVal1[ADC_KEY_COUNT] = 
{ 
	3,	//SW3	0 		0	0	//SW13
	9,	//SW4	2.2k		6	6	//SW14
	14,	//SW5	4.7K 		11	12	//SW15
	19,	//SW6	7.5K		16	17	//SW16
	25,	//SW7	12K		22	22	//SW17
};

#elif defined(AU6210K_HXX_B002)
#define		ADC_KEY_COUNT				5
static CODE BYTE AdcKeyEvent[][4] = 
{
//	SP						CPS					CPH					CPR	
	{MSG_PLAY_PAUSE,		MSG_SCAN,			MSG_NONE,		MSG_NONE	},	//SW4
	{MSG_NEXT,			MSG_VOL_ADD,		MSG_VOL_ADD,	MSG_NONE	},	//SW3
	{MSG_PRE,			MSG_VOL_SUB,		MSG_VOL_SUB,	MSG_NONE	},	//SW2	
	{MSG_MODE_SW,	MSG_NONE,		MSG_NONE,		MSG_NONE	},	//SW1
};


BYTE  CODE KeyVal1[ADC_KEY_COUNT] = 
{ 
	3,	//SW3	0 		0	0	//SW13
	9,	//SW4	2.2k		6	6	//SW14
	14,	//SW5	4.7K 		11	12	//SW15
	19,	//SW6	7.5K		16	17	//SW16
};

#elif defined(AU6210K_NR_D_8_CSRBT)
#define		ADC_KEY_COUNT				8
static CODE BYTE AdcKeyEvent[][4] = 
{
//	SP						CPS					CPH							CPR	
	{MSG_DEFAULT,		MSG_NONE,				MSG_NONE,					MSG_NONE	},	//SW1
	{MSG_NEXT,			MSG_VOL_ADD,			MSG_VOL_ADD,				MSG_NONE	},	//SW3
	{MSG_PRE,			MSG_VOL_SUB,			MSG_VOL_SUB,				MSG_NONE	},	//SW2	
	{MSG_PLAY_PAUSE,	MSG_BLUETOOTH_REJECT,	MSG_BLUETOOTH_REJECT,		MSG_NONE	},	//SW4
	{MSG_TREBUP,		MSG_TREBUP,				MSG_TREBUP,					MSG_NONE	},	//SW4MSG_TREBUP
	{MSG_TREBDN,		MSG_TREBDN,				MSG_TREBDN,					MSG_NONE	},	//SW4MSG_TREBUP	
	{MSG_BASSUP,		MSG_BASSUP,				MSG_BASSUP,					MSG_NONE	},	//SW4MSG_TREBUP
	{MSG_BASSDN,		MSG_BASSDN,				MSG_BASSDN,					MSG_NONE	},	//SW4MSG_TREBUP
};


BYTE  CODE KeyVal1[ADC_KEY_COUNT] = 
{ 
	3,	//SW3	0 		0	0	//SW13
	9,	//SW4	2.2k		6	6	//SW14
	14,	//SW5	4.7K 		11	12	//SW15
	19,	//SW6	7.5K		16	17	//SW16
	25,	//SW7	12K		22	22	//SW17
	30,	//SW8	16K		26	27	//SW18 	
	36,	//SW9	24K		33	33	//SW19
	42,	//SW10	36K		39	39 	//SW20
	
};
#elif defined(AU6210K_LK_SJ_CSRBT)
#define		ADC_KEY_COUNT				4
static CODE BYTE AdcKeyEvent[][4] = 
{
//	SP						CPS					CPH							CPR	
	{MSG_MODE_SW,		MSG_NONE,				MSG_NONE,					MSG_NONE	},	//SW1
	{MSG_NEXT,			MSG_VOL_ADD,			MSG_VOL_ADD,				MSG_NONE	},	//SW3
	{MSG_PRE,			MSG_VOL_SUB,			MSG_VOL_SUB,				MSG_NONE	},	//SW2	
	{MSG_PLAY_PAUSE,	MSG_BLUETOOTH_REJECT,	MSG_BLUETOOTH_REJECT,		MSG_NONE	},	//SW4
};


BYTE  CODE KeyVal1[ADC_KEY_COUNT] = 
{ 
	3,	//SW3	0 		0	0	//SW13
	9,	//SW4	2.2k		6	6	//SW14
	14,	//SW5	4.7K 		11	12	//SW15
	19,	//SW6	7.5K		16	17	//SW16
};

#elif defined(AU6210K_ZB_BT007_CSR)
#define		ADC_KEY_COUNT				6
static CODE BYTE AdcKeyEvent[][4] = 
{
//	SP						CPS					CPH							CPR	
	{MSG_MODE_SW,		MSG_NONE,				MSG_NONE,					MSG_NONE	},	//SW3
	{MSG_NEXT,			MSG_V_ADD,			MSG_VOL_ADD,				MSG_NONE	},	//SW3
	{MSG_PRE,			MSG_V_SUB,			MSG_VOL_SUB,				MSG_NONE	},	//SW2	
	{MSG_PLAY_PAUSE,	MSG_BT_CHANGE_PHONE,	MSG_NONE,					MSG_NONE	},	//SW4
	{MSG_BLUETOOTH_ANSWER,	MSG_BLUETOOTH_REJECT,	MSG_BLUETOOTH_REJECT,		MSG_NONE	},	//SW4
	{MSG_IR_ON_OFF,	MSG_IR_ON_OFF,	MSG_NONE,		MSG_NONE	},	//SW4
};


BYTE  CODE KeyVal1[ADC_KEY_COUNT] = 
{ 
	3,	//SW3	0 		0	0	//SW13
	9,	//SW4	2.2k		6	6	//SW14
	14,	//SW5	4.7K 		11	12	//SW15
	19,	//SW6	7.5K		16	17	//SW16
	25,	//SW7	12K		22	22	//SW17
	30,	//SW8	16K		26	27	//SW18 	
	
};
#else
#define		ADC_KEY_COUNT				11
static CODE BYTE AdcKeyEvent[][4] = 
{
//	SP						CPS					CPH					CPR
#ifdef FUNC_LED_ADC1_KEY_MULTIPLE_EN	
	{MSG_NONE,			MSG_NONE,			MSG_NONE,			MSG_NONE		},	//SW1	
	{MSG_NONE,			MSG_NONE,			MSG_NONE,			MSG_NONE		},	//SW2	
	{MSG_NONE,			MSG_NONE,			MSG_NONE,			MSG_NONE		},	//SW3
	{MSG_NONE,			MSG_NONE,			MSG_NONE,			MSG_NONE		}, //SW4
	{MSG_PLAY_PAUSE, 	MSG_NONE, 			MSG_NONE, 			MSG_NONE		},	//SW5
	{MSG_NEXT, 			MSG_VOL_ADD, 		MSG_VOL_ADD, 		MSG_NONE		},	//SW6
	{MSG_PRE, 			MSG_VOL_SUB, 		MSG_VOL_SUB, 		MSG_NONE		},	//SW7
	{MSG_MODE_SW, 		MSG_NONE, 			MSG_NONE, 			MSG_NONE		},	//SW8
	{MSG_EQ_SW, 		MSG_NONE, 			MSG_NONE, 			MSG_NONE		},	//SW11
	{MSG_VOL_ADD, 		MSG_VOL_ADD, 		MSG_VOL_ADD, 		MSG_NONE		},	//SW12
	{MSG_VOL_SUB, 		MSG_VOL_SUB, 		MSG_VOL_SUB, 		MSG_NONE		},	//SW24
#else
	{MSG_PLAY_PAUSE, 	MSG_NONE, 			MSG_NONE, 			MSG_NONE		},	//SW1	
	{MSG_NEXT, 			MSG_VOL_ADD, 		MSG_VOL_ADD, 		MSG_NONE		},	//SW2	
	{MSG_PRE, 			MSG_VOL_SUB, 		MSG_VOL_SUB, 		MSG_NONE		},	//SW3
	{MSG_MODE_SW, 		MSG_NONE, 			MSG_NONE, 			MSG_NONE		}, //SW4
	{MSG_EQ_SW, 		MSG_NONE, 			MSG_NONE, 			MSG_NONE		},	//SW5
	{MSG_VOL_SUB, 		MSG_VOL_SUB, 		MSG_VOL_SUB, 		MSG_NONE		},	//SW6
	{MSG_VOL_ADD, 		MSG_VOL_ADD, 		MSG_VOL_ADD, 		MSG_NONE		},	//SW7
	{MSG_REPEAT, 		MSG_NONE, 			MSG_NONE, 			MSG_NONE		},	//SW8
	{MSG_MUTE, 			MSG_NONE, 			MSG_NONE, 			MSG_NONE		},	//SW11
	{MSG_NEXT1, 		MSG_10TRACK_ADD, 	MSG_10TRACK_ADDCP, 	MSG_NONE		},	//SW12
	{MSG_PREV1, 		MSG_10TRACK_SUB, 	MSG_10TRACK_SUBCP, 	MSG_NONE		},	//SW24
#endif

#ifdef FUNC_LED_ADC2_KEY_MULTIPLE_EN		
	{MSG_NONE,			MSG_NONE,			MSG_NONE,			MSG_NONE		},	//SW13	
	{MSG_NONE,			MSG_NONE,			MSG_NONE,			MSG_NONE		},	//SW14
	{MSG_NONE,			MSG_NONE,			MSG_NONE,			MSG_NONE		},	//SW15
	{MSG_NONE,			MSG_NONE,			MSG_NONE,			MSG_NONE		},	//SW16	
	{MSG_NEXT, 			MSG_FF_START, 		MSG_NONE, 			MSG_FF_FB_END	},	//SW17
	{MSG_PRE, 			MSG_FB_START, 		MSG_NONE, 			MSG_FF_FB_END	},	//SW18
	{MSG_NEXT_FOLDER, 	MSG_NONE, 			MSG_NONE, 			MSG_NONE		},	//SW19
	{MSG_PRE_FOLDER, 	MSG_NONE, 			MSG_NONE, 			MSG_NONE		},	//SW20
	{MSG_NEXT1, 		MSG_10TRACK_ADD, 	MSG_10TRACK_ADDCP, 	MSG_NONE		},	//SW21
	{MSG_PREV1, 		MSG_10TRACK_SUB, 	MSG_10TRACK_SUBCP, 	MSG_NONE		},	//SW22
	{MSG_MUTE, 			MSG_NONE, 			MSG_NONE, 			MSG_NONE		},	//SW25
#else
	{MSG_NEXT, 			MSG_FF_START, 		MSG_NONE, 			MSG_FF_FB_END	},	//SW13	
	{MSG_PRE, 			MSG_FB_START, 		MSG_NONE, 			MSG_FF_FB_END	},	//SW14
	{MSG_NEXT_FOLDER, 	MSG_NONE, 			MSG_NONE, 			MSG_NONE		},	//SW15
	{MSG_PRE_FOLDER, 	MSG_NONE, 			MSG_NONE, 			MSG_NONE		},	//SW16	
	{MSG_FOLDER_EN, 	MSG_NONE, 			MSG_NONE, 			MSG_NONE		},	//SW17
	{MSG_MODE_USB, 		MSG_NONE, 			MSG_NONE, 			MSG_NONE		},	//SW18
	{MSG_MODE_SD, 		MSG_NONE, 			MSG_NONE, 			MSG_NONE		},	//SW19
	{MSG_MODE_FM, 		MSG_NONE, 			MSG_NONE, 			MSG_NONE		},	//SW20
	{MSG_MODE_AUX, 		MSG_NONE, 			MSG_NONE, 			MSG_NONE		},	//SW21
	{MSG_MODE_MP3, 		MSG_NONE, 			MSG_NONE, 			MSG_NONE		},	//SW22
	{MSG_UPDATE_FLASH, 	MSG_NONE, 			MSG_NONE, 			MSG_NONE		},	//SW25
#endif
};

#if (defined(FUNC_LED_ADC1_KEY_MULTIPLE_EN) || defined(FUNC_LED_ADC2_KEY_MULTIPLE_EN))
//上拉电阻100k
BYTE CODE KeyVal1[ADC_KEY_COUNT] = 
{ 
	3,	//SW3	0 		0	0	//SW13
	9,	//SW4	12k (0.35v)	6	7	//SW14
	14,	//SW5	20k(0.55v) 	10	11	//SW15
	19,	//SW6	33k(0.81v)	15	16	//SW16 
	23,	//SW7	47k(1.05v)	20	21	//SW17
	28,	//SW8	68k(1.33v)	25	26	//SW18 	
	32,	//SW9	91k(1.58v)	30	31	//SW19
	38,	//SW10	120k(1.80v)	34	35 	//SW20
	43,	//SW11	180k(2.12v)	41	41	//SW21
	48,	//SW12	240k(2.33v)	45	45	//SW22
	54	//SW24	390k(2.63v)	50	51 	//SW25
};
#else
//上拉电阻22k
BYTE CODE KeyVal1[ADC_KEY_COUNT] = 
{ 
	3,	//SW3	0 		0	0	//SW13
	9,	//SW4	2.2k		6	6	//SW14
	14,	//SW5	4.7K 		11	12	//SW15
	19,	//SW6	7.5K		16	17	//SW16
	25,	//SW7	12K		22	22	//SW17
	30,	//SW8	16K		26	27	//SW18 	
	36,	//SW9	24K		33	33	//SW19
	42,	//SW10	36K		39	39 	//SW20
	47,	//SW11	51K		44	44	//SW21
	54,	//SW12	91K		51	51	//SW22
	60	//SW24	220K		57	58 	//SW25
};
#endif
#endif

#ifdef HEADPHONE_ADC_PORT_CH
#ifdef HEADPHONE_ADC_PU_EN
//上拉电阻[拔出耳机3.3v，插上耳机1.6v]
BYTE CODE KeyVal2[2] = 
{ 
	50,	//SW1	32
	66	//SW2	64
};
#else
//下拉电阻[拔出耳机0v，插上耳机1.6v]
BYTE CODE KeyVal2[2] = 
{ 
	15,	//SW1	0	
	50	//SW2	32
};
#endif
#endif


// Key process, image key value to key event.
static BYTE	AdcChannelKeyGet(BYTE Channel)							
{
	BYTE Val;
	BYTE KeyIndex;
	BYTE KeyCount;

#if (defined(FUNC_LED_ADC1_KEY_MULTIPLE_EN) || defined(FUNC_LED_ADC2_KEY_MULTIPLE_EN))
	Val = AdcReadOneChannel(Channel, ADC_VREF_AVDD33);
#else
	Val = (AdcReadOneChannel(Channel, ADC_VREF_AVDD33)
	     + AdcReadOneChannel(Channel, ADC_VREF_AVDD33) 
		 + AdcReadOneChannel(Channel, ADC_VREF_AVDD33) + 2) / 3;	
#endif

#ifdef HEADPHONE_ADC_PORT_CH
	if(Channel == HEADPHONE_ADC_PORT_CH)
	{
		KeyCount = 2;
	}
	else
#endif
	{
		KeyCount = ADC_KEY_COUNT;
	}

	for(KeyIndex = 0; KeyIndex < KeyCount; KeyIndex++)
	{
#ifdef HEADPHONE_ADC_PORT_CH
		if(Channel == HEADPHONE_ADC_PORT_CH)
		{
			if(Val < KeyVal2[KeyIndex])
			{
				return KeyIndex;	
			}
		}
		else
#endif
		{
			if(Val < KeyVal1[KeyIndex])
			{
#ifdef AU6210K_ZB_BT007_CSR
				DBG1(("22222222"));
				DBG1(("22222222aaaaaKeyIndex = %d",KeyIndex));
				if(gSys.SystemMode == SYS_MODE_BLUETOOTH )
				{
					switch(KeyIndex)
					{
					case 1: 
						DBG1(("aaaabbb\n"));
						baGPIOCtrl[GPIO_A_OUT] |= 0x04; //A2
						break;
					case 2:
						DBG1(("aaacc\n"));
						baGPIOCtrl[GPIO_A_OUT] |= 0x02; //A1
						break;
					case 3:
						DBG1(("aaaddb\n"));
						baGPIOCtrl[GPIO_D_OUT] |= 0x40; //D6
						break;
					case 4:
						DBG1(("aaasdf\n"));
						baGPIOCtrl[GPIO_D_OUT] |= 0x20; //D5
						break;
					default:
						break;
					}
				}

#endif
			
			
				//DBG(("AdcKeyIndex:%bd\n", KeyIndex));
				return KeyIndex;	
			}
		}
	}	
	
	return -1;
}


static BYTE GetAdcKeyIndex(VOID)
{
	BYTE KeyIndex;
	
	KeyIndex = -1;	
	
#if (defined(FUNC_LED_ADC1_KEY_MULTIPLE_EN) || defined(FUNC_LED_ADC2_KEY_MULTIPLE_EN))
	//段码LED屏与ADC复用示例
#if (defined(FUNC_7PIN_SEG_LED_EN) || defined(FUNC_6PIN_SEG_LED_EN))
	gLedDispRefreshFlag = FALSE;	
	LedPinGpioInit();
#endif

	//切回ADC 口
#ifdef FUNC_LED_ADC1_KEY_MULTIPLE_EN	
	baGPIOCtrl[LED_ADC1_KEY_MULTIPLE_PORT_PU] &= ~LED_ADC1_KEY_MULTIPLE_BIT;
	baGPIOCtrl[LED_ADC1_KEY_MULTIPLE_PORT_PD] |= LED_ADC1_KEY_MULTIPLE_BIT; 	
#endif

#ifdef FUNC_LED_ADC2_KEY_MULTIPLE_EN
	baGPIOCtrl[LED_ADC2_KEY_MULTIPLE_PORT_PU] &= ~LED_ADC2_KEY_MULTIPLE_BIT;
	baGPIOCtrl[LED_ADC2_KEY_MULTIPLE_PORT_PD] |= LED_ADC2_KEY_MULTIPLE_BIT;
#endif

	WaitUs(40); //复用时确保AD端口稳定
#endif

#ifdef ADC_KEY_PORT_CH1
	KeyIndex = AdcChannelKeyGet(ADC_KEY_PORT_CH1);
#endif

#ifdef ADC_KEY_PORT_CH2
	if(KeyIndex == -1)
	{
		KeyIndex = AdcChannelKeyGet(ADC_KEY_PORT_CH2);
		if(KeyIndex != -1)
		{
			KeyIndex += ADC_KEY_COUNT;
		}
	}
#endif

#if (defined(FUNC_LED_ADC1_KEY_MULTIPLE_EN) || defined(FUNC_LED_ADC2_KEY_MULTIPLE_EN))
#if (defined(FUNC_7PIN_SEG_LED_EN) || defined(FUNC_6PIN_SEG_LED_EN))
	gLedDispRefreshFlag = TRUE; 
#endif
#endif

#ifdef HEADPHONE_ADC_PORT_CH
	if(KeyIndex == -1)
	{
		KeyIndex = AdcChannelKeyGet(HEADPHONE_ADC_PORT_CH);
		if(KeyIndex != -1)
		{
			KeyIndex += 22;
		}
	}	
#endif

	return KeyIndex;
}


// Initialize hardware key scan (GPIO) operation.
VOID AdcKeyScanInit(VOID)
{
	AdcKeyState = ADC_KEY_STATE_IDLE;
	TimeOutSet(&AdcKeyScanTimer, 0);
	
#ifdef AU6210K_NR_D_8_CSRBT
	SET_B6_ANALOG_IN(); 
#endif

#ifdef ADC_KEY_PORT_CH1
#if (ADC_KEY_PORT_CH1 == ADC_CHANNEL_B6)
	SET_B6_ANALOG_IN(); 
#endif

#if (ADC_KEY_PORT_CH1 == ADC_CHANNEL_D7) 
	SET_D7_ANALOG_IN();
#endif

#if (ADC_KEY_PORT_CH1 == ADC_CHANNEL_E0)
	SET_E0_ANALOG_IN();
#endif

#if (ADC_KEY_PORT_CH1 == ADC_CHANNEL_E1)
	SET_E1_ANALOG_IN();
#endif
#endif

#ifdef ADC_KEY_PORT_CH2
#if (ADC_KEY_PORT_CH2 == ADC_CHANNEL_B6)
	SET_B6_ANALOG_IN();
#endif

#if (ADC_KEY_PORT_CH2 == ADC_CHANNEL_D7)
	SET_D7_ANALOG_IN();
#endif

#if (ADC_KEY_PORT_CH2 == ADC_CHANNEL_E0)
	SET_E0_ANALOG_IN();
#endif

#if (ADC_KEY_PORT_CH2 == ADC_CHANNEL_E1)
	SET_E1_ANALOG_IN();
#endif
#endif

#ifdef HEADPHONE_ADC_PORT_CH
#if (HEADPHONE_ADC_PORT_CH == ADC_CHANNEL_B6)
	SET_B6_ANALOG_IN();
#endif
	
#if (HEADPHONE_ADC_PORT_CH == ADC_CHANNEL_D7)
	SET_D7_ANALOG_IN();
#endif
	
#if (HEADPHONE_ADC_PORT_CH == ADC_CHANNEL_E0)
	SET_E0_ANALOG_IN();
#endif
	
#if (HEADPHONE_ADC_PORT_CH == ADC_CHANNEL_E1)
	SET_E1_ANALOG_IN();
#endif
#endif

#ifdef AU6210K_ZB_BT007_CSR
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
	DBG1(("dfdfd"));
#endif
}


// Key process, image key value to key event.
MESSAGE AdcKeyEventGet(VOID)							
{
	static 	BYTE PreKeyIndex = -1;
	static  BYTE KeyIndex;

	//DBG(("AdcKeyEventGet*******\n"));
	if(!IsTimeOut(&AdcKeyScanTimer))
	{
		return MSG_NONE;
	}
	TimeOutSet(&AdcKeyScanTimer, ADC_KEY_SCAN_TIME);	

	KeyIndex = GetAdcKeyIndex();

#ifdef FUNC_BEEP_SOUND_EN	
	gPushKeyNeedBeepFlag = FALSE;
#endif

	switch(AdcKeyState)
	{
		case ADC_KEY_STATE_IDLE:
			if(KeyIndex == -1)
			{
#ifdef AU6210K_ZB_BT007_CSR
				baGPIOCtrl[GPIO_D_OUT] &= ~0x20;	
				baGPIOCtrl[GPIO_D_OUT] &= ~0x40;	
				baGPIOCtrl[GPIO_A_OUT] &= ~0x04;	
				baGPIOCtrl[GPIO_A_OUT] &= ~0x02;	
#endif				
				return MSG_NONE;
			}

			PreKeyIndex = KeyIndex;
			TimeOutSet(&AdcKeyWaitTimer, ADC_KEY_JTTER_TIME);
			//DBG(("GOTO JITTER!\n"));
			AdcKeyState = ADC_KEY_STATE_JITTER;
				
		case ADC_KEY_STATE_JITTER:
			if(PreKeyIndex != KeyIndex)
			{
				//DBG(("%bx != %bx,GOTO IDLE Because jitter!\n", PreKeyIndex, KeyIndex));
				AdcKeyState = ADC_KEY_STATE_IDLE;
			}
			else if(IsTimeOut(&AdcKeyWaitTimer))
			{
				//DBG(("GOTO PRESS_DOWN!\n"));
//				if((PreKeyIndex == ADC_KEY_POWER0_IDX)	
//				||(PreKeyIndex == ADC_KEY_POWER1_IDX)
//				|| (PreKeyIndex == ADC_KEY_POWER2_IDX)				
//				)
//				{	 
//					//set cp-time 3000ms for power-down key 
// 					TimeOutSet(&AdcKeyWaitTimer, ADC_KEY_PWRDOWN_CP_TIME);	
//				}	

#ifdef HEADPHONE_ADC_PORT_CH
				if(PreKeyIndex == ADC_KEY_HPPUSHOUT_IDX)
				{
					TimeOutSet(&AdcKeyWaitTimer, ADC_KEY_HPPUSHOUT_CP_TIME);
				}
				else
#endif				
				{
					//DBG(("ADC_KEY_CP_TIME!\n"));
 					TimeOutSet(&AdcKeyWaitTimer, ADC_KEY_CP_TIME);
				}
				AdcKeyState = ADC_KEY_STATE_PRESS_DOWN;
			}
			break;

		case ADC_KEY_STATE_PRESS_DOWN:
			if(PreKeyIndex != KeyIndex)
			{
				//return key sp value
				//DBG(("KEY  SP  %bu\n",PreKeyIndex));
				AdcKeyState = ADC_KEY_STATE_IDLE;
#ifdef FUNC_BEEP_SOUND_EN	
				gPushKeyNeedBeepFlag = TRUE;
#endif

#ifdef HEADPHONE_ADC_PORT_CH
				if((PreKeyIndex == ADC_KEY_HPPUSHIN_IDX) || (PreKeyIndex == ADC_KEY_HPPUSHOUT_IDX))
				{
					break;
				}
#endif

#if 0 //def AU6210K_ZB_BT007_CSR
				if(PreKeyIndex != 1)
				switch(PreKeyIndex)
				{
				case 1:						
					baGPIOCtrl[GPIO_A_OUT] &= ~0x04; //A2
					break;
				case 2:
					baGPIOCtrl[GPIO_A_OUT] &= ~0x02; //A1
					break;
				case 3:
					baGPIOCtrl[GPIO_D_OUT] &= ~0x40; //D6
					break;
				case 4:
					baGPIOCtrl[GPIO_D_OUT] &= ~0x20; //D5
					break;
				default:
					break;
				}

#endif

				return AdcKeyEvent[PreKeyIndex][0];
			}
			else if(IsTimeOut(&AdcKeyWaitTimer))
			{
				//return key cp value
				//DBG(("KEY CPS  %bu\n",PreKeyIndex));
				TimeOutSet(&AdcKeyWaitTimer, ADC_KEY_CPH_TIME);
				AdcKeyState = ADC_KEY_STATE_CP;
#ifdef FUNC_BEEP_SOUND_EN	
				gPushKeyNeedBeepFlag = TRUE;
#endif

#ifdef HEADPHONE_ADC_PORT_CH
				if(PreKeyIndex == ADC_KEY_HPPUSHIN_IDX) 
				{					
					break;
				}
				if(PreKeyIndex == ADC_KEY_HPPUSHOUT_IDX)
				{
					IsHeadphoneLinkFlag = FALSE;
					break;
				}
#endif
				return AdcKeyEvent[PreKeyIndex][1];
			}
			break;

		case ADC_KEY_STATE_CP:
			if(PreKeyIndex != KeyIndex)
			{
				//return key cp value
				//DBG(("ADC KEY CPR!\n"));
				AdcKeyState = ADC_KEY_STATE_IDLE;				
#ifdef HEADPHONE_ADC_PORT_CH
				if((PreKeyIndex == ADC_KEY_HPPUSHIN_IDX) || (PreKeyIndex == ADC_KEY_HPPUSHOUT_IDX))
				{
					break;
				}
#endif
				return AdcKeyEvent[PreKeyIndex][3];
			}
			else if(IsTimeOut(&AdcKeyWaitTimer))
			{
				//return key cph value
				//DBG(("ADC KEY CPH!\n"));
//				if((PreKeyIndex == ADC_KEY_POWER0_IDX)	
//				||(PreKeyIndex == ADC_KEY_POWER1_IDX)
//				|| (PreKeyIndex == ADC_KEY_POWER2_IDX))
//				{	 
//					//set cp-time 3000ms for power-down key 
//					//DBG(("ADC POWER!\n"));
// 					TimeOutSet(&AdcKeyWaitTimer, 3000);
//				}
//				else
//				{
					TimeOutSet(&AdcKeyWaitTimer, ADC_KEY_CPH_TIME);
//				}
#ifdef HEADPHONE_ADC_PORT_CH
				if(PreKeyIndex == ADC_KEY_HPPUSHIN_IDX)
				{
					IsHeadphoneLinkFlag = TRUE;
					break;
				}
				else if(PreKeyIndex == ADC_KEY_HPPUSHOUT_IDX)
				{
					IsHeadphoneLinkFlag = FALSE;
					break;
				}
#endif
				return AdcKeyEvent[PreKeyIndex][2];
			}
			break;

		default:
			AdcKeyState = ADC_KEY_STATE_IDLE;
			break;
	}
	return MSG_NONE;
}


#else
	#warning	"MV: ADC KEY NOT ENABLE!"
	
#endif

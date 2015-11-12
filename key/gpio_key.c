
/*
**********************************************************
*					INCLUDE
**********************************************************
*/
#include <reg51.h>										//include public header files

#include "type.h"										//include project header files
#include "syscfg.h"
#include "utility.h"
#include "debug.h"
#include "timer.h"
#include "sysctrl.h"
#include "key.h"
#include "gpio_key.h"
#include "gpio.h"
#include "message.h"


#ifdef FUNC_GPIO_KEY_EN

/*
**********************************************************
*					LOCAL MACRO
**********************************************************
*/
#define 	GPIO_KEY_SCAN_TIME			10
#define 	GPIO_KEY_JTTER_TIME			20
#define 	GPIO_KEY_CP_TIME			1000
#define		GPIO_KEY_PWRDOWN_CP_TIME	2000	//cp-time for power-down key
#define 	GPIO_KEY_CPH_TIME			200//450

#define		GPIO_KEY_COUNT				5


#define 	GPIO_KEY_POWER0_IDX		8			//POWER-DOWN KEY index.
#define 	GPIO_KEY_POWER1_IDX		15			//POWER-DOWN KEY index.
#define 	GPIO_KEY_POWER2_IDX		20

/*
**********************************************************
*					LOCAL VARIABLE
**********************************************************
*/
typedef enum _GPIO_KEY_STATE
{
	GPIO_KEY_STATE_IDLE,
	GPIO_KEY_STATE_JITTER,
	GPIO_KEY_STATE_PRESS_DOWN,
	GPIO_KEY_STATE_CP

} GPIO_KEY_STATE;

TIMER			GpioKeyWaitTimer;
TIMER			GpioKeyScanTimer;
GPIO_KEY_STATE	GpioKeyState;


extern BOOL SLedOnOffFlag;


/*
**********************************************************
*					LOCAL TABLE
**********************************************************
*/
#if 1
static CODE BYTE GpioKeyEvent[][4] = 
{
//	SP						CPS					CPH					CPR
	{MSG_VOL_SUB,		MSG_NONE,		MSG_NONE,		MSG_NONE		},	//SW3
	{MSG_VOL_ADD,		MSG_NONE,		MSG_NONE,		MSG_NONE		},	//SW2
};
#else
static CODE BYTE GpioKeyEvent[][4] = 
{
//	SP						CPS					CPH					CPR
	{MSG_MODE_SW,	MSG_NONE,		MSG_NONE,			MSG_NONE		},	//SW1	
	{MSG_PRE,			MSG_FB_START,		MSG_NONE,			MSG_FF_FB_END	},	//SW5
	{MSG_NEXT,			MSG_FF_START,		MSG_NONE,			MSG_FF_FB_END	},	//SW4	
	{MSG_VOL_SUB,		MSG_VOL_SUB,		MSG_VOL_SUB,		MSG_NONE		},	//SW3
	{MSG_VOL_ADD,		MSG_VOL_ADD,		MSG_VOL_ADD,		MSG_NONE		},	//SW2
};
#endif


/*
**********************************************************
*					LOCAL FUNCTIONS
**********************************************************
*/
//
// Key process, image key value to key event.
//
extern WORD KeyInLedFlashHoldTmr;
static BYTE GetGpioKeyIndex(VOID)
{
	BYTE KeyIndex = -1;

#if 1
	if(!(baGPIOCtrl[GPIO_C_IN] & 0x01))  //GPC0
	{
		DBG(("SW1\n"));
		KeyInLedFlashHoldTmr = 500;
		return 0;
	}	

	if(!(baGPIOCtrl[GPIO_C_IN] & 0x02))  //GPC1
	{
		DBG(("SW2\n"));
		KeyInLedFlashHoldTmr = 500;
		return 1;
	}

#else
//示例: 3个GPIO连接3 个按键的扫描
//	if(!(baGPIOCtrl[GPIO_B_IN] & 0x04))
//	{
//		DBG(("SW1\n"));
//		return 0;
//	}	

//	if(!(baGPIOCtrl[GPIO_B_IN] & 0x02))
//	{
//		DBG(("SW2\n"));
//		return 1;
//	}	

//	if(!(baGPIOCtrl[GPIO_B_IN] & 0x10))
//	{
//		DBG(("SW3\n"));
//		return 2;
//	}	


//示例: 3个GPIO连接5 个按键的扫描 
	baGPIOCtrl[GPIO_B_IE] |= 0x40;
	baGPIOCtrl[GPIO_B_OE] &= ~0x40;
	baGPIOCtrl[GPIO_B_PU] &= ~0x40;
	baGPIOCtrl[GPIO_B_PD] &= ~0x40; 
	baGPIOCtrl[GPIO_B_DS] |= 0x40; 

	baGPIOCtrl[GPIO_D_IE] |= 0x80;
	baGPIOCtrl[GPIO_D_OE] &= ~0x80;
	baGPIOCtrl[GPIO_D_PU] |= 0x80;
	baGPIOCtrl[GPIO_D_PD] |= 0x80;
	baGPIOCtrl[GPIO_D_DS] &= ~0x80;

	baGPIOCtrl[GPIO_E_OE] |= 0x01;
	baGPIOCtrl[GPIO_E_OE] &= ~0x01;
	baGPIOCtrl[GPIO_E_PU] |= 0x01;
	baGPIOCtrl[GPIO_E_PD] |= 0x01;
	baGPIOCtrl[GPIO_E_DS] &= ~0x01;
    WaitUs(1);
	if(baGPIOCtrl[GPIO_E_IN] & 0x01)//SW4
	{
		DBG(("SW4\n"));
		return 2;
	}
	if(baGPIOCtrl[GPIO_D_IN] & 0x80)//SW2
	{
		DBG(("SW2\n"));
		return 4;
	}
	baGPIOCtrl[GPIO_D_PU] &= ~0x80;
	baGPIOCtrl[GPIO_D_PD] &= ~0x80;
	baGPIOCtrl[GPIO_E_PU] &= ~0x01;
	baGPIOCtrl[GPIO_E_PD] &= ~0x01;
	WaitUs(1);
	if((baGPIOCtrl[GPIO_E_IN] & 0x01) == 0)//SW5
	{
		DBG(("SW5\n"));
		return 1;
	}
	if((baGPIOCtrl[GPIO_D_IN] & 0x80) == 0)//SW1
	{
		DBG(("SW1\n"));
		return 0;
	}
	baGPIOCtrl[GPIO_E_PU] |= 0x01;
	baGPIOCtrl[GPIO_E_PD] |= 0x01;
	baGPIOCtrl[GPIO_D_DS] |= 0x80;
	WaitUs(2);
	if(baGPIOCtrl[GPIO_E_IN] & 0x01)//SW3
	{
		DBG(("SW3\n"));
		return 3;
	}	
#endif
	return KeyIndex;
}


/*
**********************************************************
*					GLOBAL FUNCTION
**********************************************************
*/
// Initialize hardware key scan (GPIO) operation.
VOID GpioKeyScanInit(VOID)
{
	GpioKeyState = GPIO_KEY_STATE_IDLE;
	TimeOutSet(&GpioKeyScanTimer, 0);	
	
//示例: 3个GPIO连接3个按键的GPIO初始化
	baGPIOCtrl[GPIO_C_IE] |= 0x01;
	baGPIOCtrl[GPIO_C_OE] &= ~0x01;
	baGPIOCtrl[GPIO_C_PU] &= ~0x01;
	baGPIOCtrl[GPIO_C_PD] &= ~0x01; 
	baGPIOCtrl[GPIO_C_DS] &= ~0x01; 

	baGPIOCtrl[GPIO_C_IE] |= 0x02;
	baGPIOCtrl[GPIO_C_OE] &= ~0x02;
	baGPIOCtrl[GPIO_C_PU] &= ~0x02;
	baGPIOCtrl[GPIO_C_PD] &= ~0x02; 
	baGPIOCtrl[GPIO_C_DS] &= ~0x02; 
	
//	baGPIOCtrl[GPIO_B_IE] |= 0x04;
//	baGPIOCtrl[GPIO_B_OE] &= ~0x04;
//	baGPIOCtrl[GPIO_B_PU] &= ~0x04;
//	baGPIOCtrl[GPIO_B_PD] &= ~0x04; 
//	baGPIOCtrl[GPIO_B_DS] &= ~0x04; 
	
//	baGPIOCtrl[GPIO_B_IE] |= 0x08;
//	baGPIOCtrl[GPIO_B_OE] &= ~0x08;
//	baGPIOCtrl[GPIO_B_PU] &= ~0x08;
//	baGPIOCtrl[GPIO_B_PD] &= ~0x08; 
//	baGPIOCtrl[GPIO_B_DS] &= ~0x08; 	
}


//
// Key process, image key value to key event.
//
MESSAGE GpioKeyEventGet(VOID)							
{
	static 	BYTE 	PreKeyIndex = -1;
	static  BYTE	KeyIndex;

	if (!IsTimeOut(&GpioKeyScanTimer))
	{
		return MSG_NONE;
	}
	TimeOutSet(&GpioKeyScanTimer, GPIO_KEY_SCAN_TIME);	

	KeyIndex = GetGpioKeyIndex();

	//示例: 按键端口与单个LED端口共用时LED的点亮操作
	#ifdef FUNC_SINGLE_LED_EN	
//	if(SLedOnOffFlag == TRUE)
//	{
//		baGPIOCtrl[GPIO_B_IE] &= ~0x40;
//		baGPIOCtrl[GPIO_B_OE] |= 0x40;
//		baGPIOCtrl[GPIO_B_OUT] &= ~0x40; 
//	}
	#endif

	switch(GpioKeyState)
	{
		case GPIO_KEY_STATE_IDLE:
			if(KeyIndex == -1)
			{
				return MSG_NONE;
			}

			PreKeyIndex = KeyIndex;
			TimeOutSet(&GpioKeyWaitTimer, GPIO_KEY_JTTER_TIME);
			//DBG(("GOTO JITTER!\n"));
			GpioKeyState = GPIO_KEY_STATE_JITTER;
				
		case GPIO_KEY_STATE_JITTER:
			if(PreKeyIndex != KeyIndex)
			{
				//DBG(("%bx != %bx,GOTO IDLE Because jitter!\n", PreKeyIndex, KeyIndex));
				GpioKeyState = GPIO_KEY_STATE_IDLE;
			}
			else if(IsTimeOut(&GpioKeyWaitTimer))
			{
				//DBG(("GOTO PRESS_DOWN!\n"));
				if((PreKeyIndex == GPIO_KEY_POWER0_IDX)	
					||(PreKeyIndex == GPIO_KEY_POWER1_IDX)
					||(PreKeyIndex == GPIO_KEY_POWER2_IDX)
				)
				{	 
					//set cp-time 3000ms for power-down key 
 					TimeOutSet(&GpioKeyWaitTimer, GPIO_KEY_PWRDOWN_CP_TIME);	
				}
				else
				{
					//DBG(("GPIO_KEY_CP_TIME!\n"));
 					TimeOutSet(&GpioKeyWaitTimer, GPIO_KEY_CP_TIME);
				}
				GpioKeyState = GPIO_KEY_STATE_PRESS_DOWN;
			}
			break;

		case GPIO_KEY_STATE_PRESS_DOWN:
			if(PreKeyIndex != KeyIndex)
			{
				//return key sp value
				//DBG(("KEY  SP  %bu\n",PreKeyIndex));
				GpioKeyState = GPIO_KEY_STATE_IDLE;
				return GpioKeyEvent[PreKeyIndex][0];
			}
			else if(IsTimeOut(&GpioKeyWaitTimer))
			{
				//return key cp value
				//DBG(("KEY CPS  %bu\n",PreKeyIndex));
				TimeOutSet(&GpioKeyWaitTimer, GPIO_KEY_CPH_TIME);
				GpioKeyState = GPIO_KEY_STATE_CP;
				return GpioKeyEvent[PreKeyIndex][1];
			}
			break;

		case GPIO_KEY_STATE_CP:
			if(PreKeyIndex != KeyIndex)
			{
				//return key cp value
				//DBG(("GPIO KEY CPR!\n"));
				GpioKeyState = GPIO_KEY_STATE_IDLE;
				return GpioKeyEvent[PreKeyIndex][3];
			}
			else if(IsTimeOut(&GpioKeyWaitTimer))
			{
				//return key cph value
				//DBG(("GPIO KEY CPH!\n"));
				if((PreKeyIndex == GPIO_KEY_POWER0_IDX)	
					||(PreKeyIndex == GPIO_KEY_POWER1_IDX)
					||(PreKeyIndex == GPIO_KEY_POWER2_IDX)
				)
				{	 
					//set cp-time 3000ms for power-down key 
					//DBG(("GPIO POWER!\n"));
 					TimeOutSet(&GpioKeyWaitTimer, 3000);
				}
				else
				{
					TimeOutSet(&GpioKeyWaitTimer, GPIO_KEY_CPH_TIME);
				}
				return GpioKeyEvent[PreKeyIndex][2];
			}
			break;

		default:
			GpioKeyState = GPIO_KEY_STATE_IDLE;
			break;
	}
	return MSG_NONE;
}


#else
	#warning	"MV: GPIO KEY NOT ENABLE!"
#endif


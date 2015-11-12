#include <reg51.h>	

#include "type.h"	
#include "syscfg.h"
#include "utility.h"
#include "debug.h"
#include "timer.h"
#include "sysctrl.h"
#include "key.h"
#include "power_key.h"


#if	(PWR_KEY_MODE == PWR_KEY_PUSH_BUTTON)

	#warning	"MV: PUSH BUTTON POWER KEY ENABLE!"

extern BOOL	gIsPwrkeyPadOn;				//POWER_KEY引脚的状态判定是在定时中断内执行的，通过该全局变量与本文件的软开关处理流程交互
										//系统启动时POWER_KEY默认是接通状态
#define 	POWER_KEY_CP_TIME			100			//要求轻触开关机时，CP时间需要短
#define 	POWER_KEY_CPH_TIME			100


typedef enum _POWER_KEY_STATE
{
	POWER_KEY_STATE_IDLE,
	POWER_KEY_STATE_PRESS_DOWN,
	POWER_KEY_STATE_CP

} POWER_KEY_STATE;


TIMER			PowerKeyWaitTimer;
POWER_KEY_STATE	PowerKeyState;
BOOL			IsPwrkeyInSysOn = TRUE;		//表示当前是否是系统启动阶段的POWER_KEY识别

#ifdef AU6210K_NR_D_8_CSRBT
static CODE BYTE PowerKeyEvent[3] = {MSG_MODE_SW, MSG_SYS_ON, MSG_SYS_OFF};	
#else
static CODE BYTE PowerKeyEvent[3] = {MSG_MODE_SW, MSG_SYS_ON, MSG_SYS_OFF};	
#endif


// Initialize POWER_KEY scan operation.
VOID PowerKeyScanInit(VOID)
{
	DBG(("PowerKeyScanInit*******\n"));
	PowerKeyState = POWER_KEY_STATE_IDLE;
	IsPwrkeyInSysOn = TRUE;					//首先软开关用于系统启动，此后用于系统关闭
}


//
// POWER_KEY与普通的按键不同，连接按钮开关（软开关）时的主要作用还是系统开关机，当然，也允许复用短按功能。
// 短按产生时，推送短按消息。超过短按区间，推送系统开关机消息。
MESSAGE PowerKeyEventGet(VOID)							
{
	//DBG(("PowerKeyEventGet*******\n"));
	//DBG(("PowerKeyState:%bx\n", PowerKeyState));
	//DBG(("gIsPwrkeyPadOn:%bx\n", gIsPwrkeyPadOn));
	switch(PowerKeyState)
	{
		case POWER_KEY_STATE_IDLE:
			if(gIsPwrkeyPadOn == FALSE)
			{
				IsPwrkeyInSysOn = FALSE;
				return MSG_NONE;
			}
			DBG(("GOTO POWER PRESS DOWN!\n"));

#if defined(TIME_POWER_ON_HOLD)
			if(IsPwrkeyInSysOn == TRUE)
			{
				TimeOutSet(&PowerKeyWaitTimer, TIME_POWER_ON_HOLD);	
			}
			else
#endif
			{
				TimeOutSet(&PowerKeyWaitTimer, TIME_POWER_OFF_HOLD);
			}

			PowerKeyState = POWER_KEY_STATE_PRESS_DOWN;
			break;

		case POWER_KEY_STATE_PRESS_DOWN:
			if(gIsPwrkeyPadOn == FALSE)
			{
				IsPwrkeyInSysOn = FALSE;
				PowerKeyState = POWER_KEY_STATE_IDLE;
				return PowerKeyEvent[0];			//return key sp value
			}
			else if(IsTimeOut(&PowerKeyWaitTimer))
			{
				TimeOutSet(&PowerKeyWaitTimer, POWER_KEY_CPH_TIME);
				PowerKeyState = POWER_KEY_STATE_CP;
				if(IsPwrkeyInSysOn == TRUE)			//系统启动时，推送MSG_SYS_ON消息，用于通知POWER_KEY开机保持时间已到
				{
					DBG(("PowerKeyEvent[1]:%bx\n", PowerKeyEvent[1]));
					return PowerKeyEvent[1];
				}
				else								//系统启动之后，POWER_KEY的长按只发送MSG_SYS_OFF消息，用于关闭系统
				{
					DBG(("PowerKeyEvent[2]:%bx\n", PowerKeyEvent[2]));
					return PowerKeyEvent[2];
				}
			}
			break;
			
		case POWER_KEY_STATE_CP:
			if(gIsPwrkeyPadOn == FALSE)
			{
				IsPwrkeyInSysOn = FALSE;			//系统启动后，POWER_KEY长按的作用变为关机
				PowerKeyState = POWER_KEY_STATE_IDLE;
			}
			else if(IsTimeOut(&PowerKeyWaitTimer))
			{
				TimeOutSet(&PowerKeyWaitTimer, POWER_KEY_CPH_TIME);
				if(IsPwrkeyInSysOn == FALSE)		//系统启动后，POWER_KEY的长按只发送MSG_SYS_OFF消息，用于关闭系统
				{
					DBG(("PowerKeyEvent[2]:%bx\n", PowerKeyEvent[2]));
					return PowerKeyEvent[2];
				}
			}
			break;	
					
		default:
			PowerKeyState = POWER_KEY_STATE_IDLE;
			break;
	}
	return MSG_NONE;
}

#endif

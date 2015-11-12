#include "type.h"
#include "syscfg.h"
#include "utility.h"
#include "dac.h"
#include "sysctrl.h"
#include "debug.h"
#include "ir_key.h"
#include "timer.h"


#ifdef FUNC_TIMER1_REFRESH_LED_EN
extern BYTE gClkDivRate;
#endif


// System clock divider setting.
VOID SysClkDivSet(BYTE ClkDivRate)
{
#if 1
//	DBG(("CLK divid enable!\n"));

//	ModuleClkDis(GCLK_USB_MASK | GCLK_CARD_MASK | GCLK_DECD_MASK | GCLK_DAC_MASK);

//	//如果选择的是DPLL，则系统时钟是在12MHZ基础上再分频
//	ClockSwitchToDPLL();
//	SetClkDivRate(ClkDivRate);
//	TimerCountSet(ClkDivRate);
	
	//选择分频后系统时钟是来自于DPLL还是RC振荡器
	//如果选择的是RC振荡器，则系统时钟是在3MHZ的基础上再分频  
	//ClkDivRate必须是4，8，12，16等4的整数倍 
	//如果不为4的整数倍，除4取整数。
	SetClkDivRate(ClkDivRate / 4);
	ClockSwitchToRc();
	TimerCountSet((ClkDivRate / 4) * 4);

#ifdef FUNC_TIMER1_REFRESH_LED_EN
	gClkDivRate = ClkDivRate * 4;
#endif
#endif
	gSys.IsClkDiv = TRUE;
}


// System clock divider clear.
VOID SysClkDivClr(VOID)
{
	ClockSwitchToDPLL();	//系统时钟切换到DPLL
	SetClkDivRate(1);
	TimerCountSet(1);
//	ClkDivSwitchDis();
//	WaitMs(2);
//	ModuleClkEn(GCLK_USB_MASK | GCLK_CARD_MASK | GCLK_DECD_MASK | GCLK_DAC_MASK);

#ifdef FUNC_TIMER1_REFRESH_LED_EN
	gClkDivRate = 1;
#endif

	gSys.IsClkDiv = FALSE;

#ifdef FUNC_WATCHDOG_EN
	FeedWatchDog();
#endif
}



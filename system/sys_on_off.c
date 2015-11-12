#include "syscfg.h"
#include "key.h"
#include "utility.h"
#include "user_interface.h"
#include "power.h"
#include "chip.h"
#include "ir.h"
#include "radio_ctrl.h"
#include "debug.h"
#include "display.h"
#include "bluetooth_ctrl.h"
#include "npca110x.h"


BYTE XDATA gWakeupTrigFlg _at_ 0x06D8; //不要轻易改动地址


//系统启动，包括系统从SLEEP状态下唤醒启动或者从POWER DOWN状态下以片内主电源上电方式启动的相关处理
//一般情况下，2种启动方式在用户产品中不会同时存在
VOID SystemOn(VOID)
{
	TIMER	WaitTimer;

	DBG1(("*******SystemOn************\n"));
	

#if (POWER_SAVING_MODE_OPTION == POWER_SAVING_MODE_SLEEP)
	
	//获得从SLEEP状态唤醒的触发源标志
	DBG1((">>WakeUpOp(), gWakeupTrigFlg:%bx\n", gWakeupTrigFlg));

	//系统定义了IR唤醒功能，IR接收头连接于GPIOB6/GPIOE0，IR信号唤醒系统后，则判断键值
#ifdef  WIS_FLG_IR
	if((gWakeupTrigFlg == WIS_FLG_GPB6) || (gWakeupTrigFlg == WIS_FLG_GPE0))
	{		
		DWORD IrKeyData;
		

		TimeOutSet(&WaitTimer, 3500);
		//如果在规定时间内未发现预定的IR键值，则继续睡眠
		while(!IsTimeOut(&WaitTimer))	
		{
#ifdef FUNC_WATCHDOG_EN
			FeedWatchDog();
#endif
			IrKeyData = IrKeyDataVal();
			DBG1(("IrKeyData = : %08LX\n", IrKeyData)); 

			//如果检测到预定的IR键值，则开始运行系统
			if( (IrKeyData == 0x5AA57F80) ||
				(IrKeyData == 0x5FA07F80) ||
				(IrKeyData == 0x5EA17F80) ||
				(IrKeyData == 0x5DA27F80) ||
				(IrKeyData == 0x5CA37F80) ||
				(IrKeyData == 0x59A67F80) ||
				(IrKeyData == 0x5BA47F80) )
			{
				MessageInit();
				DBG1(("WAKE UP FROM SLEEP BY IR!\n"));
				baGPIOCtrl[GPIO_A_IE] &= ~0x40;//A2
				return;
			}
			else
			{
			 	DBG1(("ERROR IR KEY!\n"));
				break;
			}
		}
		
		DBG1(("TIMEOUT!\n"));
#ifdef IR_KEY_PORT
		if(IR_KEY_PORT == IR_INPUT_GPIOB6)
		{
			SetWakeupSrcInSleep(WIS_GPB6_LOW);
			DBG1(("*GPIO_B_IN:%bx\n", GetGpioReg(GPIO_B_IN)));
			while((GetGpioReg(GPIO_B_IN) & 0x40) != 0x40)
			{
				DBG1(("请确保进入Sleep状态前WakeUp条件不成立！(GPIO_C_IN:%bx(GPIOC3 shoule be 0))\n", GetGpioReg(GPIO_B_IN)));
				WaitMs(10);
#ifdef FUNC_WATCHDOG_EN
				FeedWatchDog();
#endif
			}
		}
		else if(IR_KEY_PORT == IR_INPUT_GPIOE0)
		{
			SetWakeupSrcInSleep(WIS_GPE0_LOW);
			DBG(("*GPIO_E_IN:%bx\n", GetGpioReg(GPIO_E_IN)));
			while((GetGpioReg(GPIO_E_IN) & 0x01) != 0x01)
			{
				DBG1(("请确保进入Sleep状态前WakeUp条件不成立！(GPIO_C_IN:%bx(GPIOC3 shoule be 0))\n", GetGpioReg(GPIO_B_IN)));
				WaitMs(10);
#ifdef FUNC_WATCHDOG_EN
			FeedWatchDog();
#endif
			}
		}
#endif

		GoToSleepMode(SLEEP_MOD_OTP_PD);

		DBG1(("<<WakeUpOp()\n")); 
		return;
	}

	
#endif
	

#elif (POWER_SAVING_MODE_OPTION == POWER_SAVING_MODE_POWERDOWN)
#if	(PWR_KEY_MODE == PWR_KEY_PUSH_BUTTON)
	DBG(("\nPWR_KEY_PUSH_BUTTON\n\n"));	
	SetChip8SRstMode(CRM_IN_PUSH_BUTTON);

#elif(PWR_KEY_MODE == PWR_KEY_SLIDE_SWITCH)
	DBG(("\nPWR_KEY_SLIDE_SWITCH\n\n"));	
	SetChip8SRstMode(CRM_IN_SLIDE_SWITCH);
#endif

	gWakeupTrigFlg = GetWakeupFlgFromPD();			//获得从powerdown状态触发片内主电源上电系统启动的触发源标志
	DBG(("gWakeupTrigFlg:%bx\n", gWakeupTrigFlg));

	if(WIP_SRC_PWR_KEY & gWakeupTrigFlg)
	{
		DBG(("POWER KEY SYS ON\n"));
	}
	else if(WIP_SRC_LP_RTC & gWakeupTrigFlg)
	{
		DBG(("LP_RTC SYS ON\n"));
	}

	//如果POWER_KEY使用软开关(push button)，并且定义了POWER KEY开关机保持时间
	//例如，要求按住POWER_KEY超过2秒后，系统才会点亮显示继续运行，否则系统再次关闭，用户不会感觉到系统运行过
	//POWER_KEY持续按下了规定时间的判定是在power_key.c内完成的。
#if	((PWR_KEY_MODE == PWR_KEY_PUSH_BUTTON) && defined(TIME_POWER_ON_HOLD))	
	if(WIP_SRC_PWR_KEY & gWakeupTrigFlg)		//必须是POWER_KEY唤醒系统时，才检查开机保持时间，而WTD复位等系统重新启动则不需要
	{
		TimeOutSet(&WaitTimer, (TIME_POWER_ON_HOLD * 2));
		
		while(!IsTimeOut(&WaitTimer))
		{
#ifdef FUNC_WATCHDOG_EN
			FeedWatchDog();
#endif
			KeyEventGet();	

			if(MessageGet(MSG_FIFO_KEY) == MSG_SYS_ON)		//如果达到开机保持时间，则系统继续运行
			{			
				MessageInit();
				DBG(("POWER_KEY Hold time,MSG_SYS_ON\n"));
				return;
			}
		}
		
		DBG(("TIMEOUT!\n"));
		//如果在规定时间内没有收到由power_key.c推送的MSG_SYS_ON消息，需要再次系统关闭。
		GoToPDMode();
	}

#elif(PWR_KEY_MODE == PWR_KEY_SLIDE_SWITCH)
	DBG(("\nPWR_KEY_MODE == PWR_KEY_SLIDE_SWITCH\n\n"));	
#endif		
		
#endif
}


//系统关闭，包括系统以SLEEP方式关闭或者以片内主电源下电POWER DOWN方式关闭的相关处理
VOID SystemOff(VOID)
{
#if (POWER_SAVING_MODE_OPTION == POWER_SAVING_MODE_SLEEP)
	//休眠，进入SLEEP模式
	DBG(("SLEEP!\n"));
#ifdef FUNC_DISP_EN
	DispDeInit();
#endif

#ifdef FUNC_RADIO_EN
	if(gSys.SystemMode == SYS_MODE_RADIO)
	{
		RadioCtrlDeinit();
	}
#endif

	MuteOn(FALSE, TRUE);
	//NPCA110X_SetOutMute(TRUE);
	InDacChannelSel(DAC_CH_NONE);
	InDacPowerDown(FALSE);

	//红外唤醒源选择时需要注意选择的GPIO端口
	SetWakeupSrcInSleep(WIS_GPB6_LOW);
	//SetWakeupSrcInSleep(WIS_GPD7_HIGH);
	//SetWakeupSrcInSleep(WIS_GPE0_LOW);
	//SetWakeupSrcInSleep(WIS_GPE1_HIGH);
	//SetWakeupSrcInSleep(WIS_RTC_ALARM);
	//SetWakeupSrcInSleep(WIS_PWR_KEY); 
	GoToSleepMode(SLEEP_MOD_OTP_PD);
	//GoToSleepMode(SLEEP_MOD_NORMAL);
	return;

#elif (POWER_SAVING_MODE_OPTION == POWER_SAVING_MODE_POWERDOWN)

#if(PWR_KEY_MODE == PWR_KEY_PUSH_BUTTON) //POWER_KEY为按钮开关（软开关）
	DBG(("\nSYSTEM OFF BY PUSH BUTTON!\n\n"));

#ifdef	FUNC_ALARM_EN
	SetWakeupLpRtcInPD(); // RTC闹钟唤醒使能
#endif

#elif(PWR_KEY_MODE == PWR_KEY_SLIDE_SWITCH)
		DBG(("\nSYSTEM OFF BY SLID SWITCH!\n\n"));	
#endif
	
#ifdef FUNC_DISP_EN
	DispDeInit();
#endif

	MuteOn(FALSE, TRUE);
//执行InDacMuteEn、InDacChannelSel和InDacPowerDown 共800ms左右，造成关机后不能快速按键开机
//屏蔽这部分代码后可能导致快速开机会有POP声
//	InDacChannelSel(DAC_CH_NONE);
//	InDacPowerDown(FALSE);
	
	GoToPDMode();	
#endif
}


#ifdef FUN_SYSTEM_POWEROFF_WAIT_TIME
SystemTimingOff gSysTimeOffState;


VOID SystemOffTimeInit(VOID)
{
	gSysTimeOffState.ProState = SYS_OFF_TIMEING_IDLE;
	gSysTimeOffState.EnState = FALSE;
	gSysTimeOffState.SleepState = SLEEP_00;
}


static VOID SystemOffSetTimer(VOID)
{
	DWORD SysOffWaitTime;

	switch(gSysTimeOffState.SleepState)
	{	
		case SLEEP_15:
			SysOffWaitTime = 900000;
			break;
			
		case SLEEP_30:
			SysOffWaitTime = 1800000;
			break;

		case SLEEP_45:
			SysOffWaitTime = 2700000;
			break;
			
		case SLEEP_60:
			SysOffWaitTime = 3600000;
			break;
			
		case SLEEP_75:
			SysOffWaitTime = 4500000;
			break;	
			
		case SLEEP_90:
			SysOffWaitTime = 5400000;
			break;

		case SLEEP_105:
			SysOffWaitTime = 6300000;
			break;
			
		case SLEEP_120:
			SysOffWaitTime = 7200000;
			break;
			
		default:
			break;
	}
	
	TimeOutSet(&gSysTimeOffState.AutoOutTimer, SysOffWaitTime);	
	DBG(("Start....                                          %2BX\n",gSysTimeOffState.SleepState));
}


VOID SystemOffTimeEnDis(BYTE Type)
{
	TimeOutSet(&gSysTimeOffState.DisTimer, 3000);
		
	if(SYSOFF_TIME_SET_VALUE == Type)
	{
		if(gSysTimeOffState.EnState == FALSE)
		{
			gSysTimeOffState.EnState = TRUE;				
			gSysTimeOffState.ProState = SYS_OFF_TIMEING_SET_MODE_VALID;	
			SystemOffSetTimer();
		}
		else
		{				
			gSysTimeOffState.EnState = FALSE;		
			gSysTimeOffState.ProState = SYS_OFF_TIMEING_SET_DIS;
		}	
	}
	else if(SYSOFF_TIME_READ_ONLY == Type)
	{		
		if((gSysTimeOffState.ProState == SYS_OFF_TIMEING_WAIT_POWEROFF) || (gSysTimeOffState.ProState == SYS_OFF_TIMEING_SET_MODE_VALID))			
		{
			gSysTimeOffState.ProState = SYS_OFF_TIMEING_READ_ONLY_TIMING;//回到定时关机或设置定时关机状态
		}	
		else
		{
			gSysTimeOffState.ProState = SYS_OFF_TIMEING_READ_ONLY_IDLE;//回到空闲状态		
		}
	}
}


VOID SystemOffTimeSet(VOID)
{				
	TimeOutSet(&gSysTimeOffState.DisTimer, 3000);	

	gSysTimeOffState.SleepState++;
	if(gSysTimeOffState.SleepState >= SLEEP_MAX)
	{
		gSysTimeOffState.SleepState = SLEEP_00;
	}	
	gSysTimeOffState.ProState = SYS_OFF_TIMEING_SET_MODE_VALID;	
	
	if(gSysTimeOffState.SleepState != SLEEP_00)
	{
		gSysTimeOffState.EnState = TRUE;
		SystemOffSetTimer();
	}
	else
	{
		gSysTimeOffState.EnState = FALSE;
	}	

//	if(gSysTimeOffState.EnState == TRUE)
//	{
//		gSysTimeOffState.SleepState++;
//		if(gSysTimeOffState.SleepState >= SLEEP_MAX)
//		{
//			gSysTimeOffState.SleepState = SLEEP_00;
//		}	
//		SystemOffSetTimer();
//		gSysTimeOffState.ProState = SYS_OFF_TIMEING_SET_MODE_VALID;	
//	}
//	else
//	{
//		gSysTimeOffState.ProState  = SYS_OFF_TIMEING_SET_MODE_INVALID;
//	}
}


//如果有和此函数类似用法(嵌入到各个模式)的函数位于其后，此函数不要有default，防止拦截后者有用的消息
VOID SystemOffTimeMsgPro(BYTE Msg)
{	
	if(Msg != MSG_NONE)
	{
		TimeOutSet(&gSysTimeOffState.DisTimer, 0);	
	}
	
	switch(Msg)
	{
		case MSG_SLEEP:
			SystemOffTimeSet();
			break;		
			
//		case MSG_SYS_TIMING_EN:
//			SystemOffTimeEnDis(SYSOFF_TIME_SET_VALUE);
//			break;

//		case MSG_SYS_TIMING_SET:
//			SystemOffTimeSet();
//			break;

//		case MSG_SYS_TIMING_READ:	
//			SystemOffTimeEnDis(SYSOFF_TIME_READ_ONLY);
//			break;		

		default:
			break;
	}
}


VOID SystemOffTimeWait(VOID)
{
	switch(gSysTimeOffState.ProState)
	{
		case SYS_OFF_TIMEING_IDLE:
			break;

		case SYS_OFF_TIMEING_READ_ONLY_IDLE:
		case SYS_OFF_TIMEING_READ_ONLY_TIMING:
		case SYS_OFF_TIMEING_SET_DIS:
		case SYS_OFF_TIMEING_SET_MODE_VALID:
		case SYS_OFF_TIMEING_SET_MODE_INVALID:
			if(IsTimeOut(&gSysTimeOffState.DisTimer))
			{
				if((gSysTimeOffState.ProState == SYS_OFF_TIMEING_SET_MODE_VALID) || (gSysTimeOffState.ProState == SYS_OFF_TIMEING_READ_ONLY_TIMING))
				{
					gSysTimeOffState.ProState = SYS_OFF_TIMEING_WAIT_POWEROFF;
				}
				else
				{
					gSysTimeOffState.ProState = SYS_OFF_TIMEING_IDLE;
				}
			}
			break;

		case SYS_OFF_TIMEING_WAIT_POWEROFF:
			if(IsTimeOut(&gSysTimeOffState.AutoOutTimer))
			{
				if(gSysTimeOffState.EnState == TRUE)
				{
					DBG(("SYSTEM OFF BY TimeOut!\n"));
					SystemOff();
				}
			}
			break;
			
		default:
			break;					
	}
}

#endif



#ifndef __SLEEP_H__
#define __SLEEP_H__

// 'WIS'是WAKEUP IN SLEEP的缩写
// Wake up mode.
#define		WIS_NONE				0x0000

#define		WIS_GPB6_LOW			0x0001
#define		WIS_GPB6_HIGH			0x0101

#define		WIS_GPD7_LOW			0x0002
#define		WIS_GPD7_HIGH			0x0202

#define		WIS_GPE0_LOW			0x0004
#define		WIS_GPE0_HIGH			0x0404

#define		WIS_GPE1_LOW			0x0008
#define		WIS_GPE1_HIGH			0x0808

#define		WIS_PWR_KEY				0x1010
#define		WIS_RTC_ALARM			0x0020


// 芯片启动工作时，2个标志位
#define		SYS_ON_FLG_POR						(1 << 0)	//power on reset后，芯片启动
#define		SYS_ON_FLG_WDG						(1 << 1)	//watchdog reset后，芯片重新启动
// flags of all wake up sources (from sleep), its are used for
// functions: GetWakeupInSleepTrigFlg()
// 'WIS'是WAKEUP IN SLEEP的缩写
#define		WIS_FLG_RTC         	  			(1 << 2)
#define		WIS_FLG_GPB6						(1 << 3)
#define		WIS_FLG_GPD7            			(1 << 4)
#define		WIS_FLG_GPE0          				(1 << 5)
#define		WIS_FLG_GPE1          				(1 << 6)
#define		WIS_FLG_PWR_KEY						(1 << 7)


// Sleep mode
#define 	SLEEP_MOD_NORMAL					0
#define		SLEEP_MOD_OTP_PD					1

	
// Set chip go to sleep mode.
VOID GoToSleepMode(BYTE SleepMode);

// Config the wakeup source and active voltage level of GPIO.
// 1). GPIO唤醒源 B6,D7,EO,E0，均可以设置高低电平唤醒
// 2). RTC 唤醒时候需要使能RTC_WAKEUP和RTC32K中断      
// 3). 正确设置WakeUp源后，不执行GoToSleepMode函数，如外部满足WakeUp条件，芯片会被复位。
VOID SetWakeupSrcInSleep(WORD WakeUpMode);

// clear the wakeup source 
// 清除在SLEEP状态下的唤醒源需要慎重 
VOID ClrWakeupSrcInSleep(WORD WakeUpMode); 

// Get WakeUp Source flag.
// 系统起来的时候必须调用该函数
// 执行本函数后，WakeUp功能被禁止，如果之前已调用SetWakeUpMode函数设置WakeUp条件，
// 可直接调用GoToSleepMode函数，WakeUp条件满足后芯片仍能正常WakeUp起来。
BYTE GetWakeupFlgFromSleep(VOID);


#endif

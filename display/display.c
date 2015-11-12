#include <string.h>
#include <stdio.h>

#include "type.h"										//include project header files
#include "utility.h"
#include "syscfg.h"
#include "key.h"
#include "sysctrl.h"
#include "playctrl.h"
#include "timer.h"
#include "rtc.h"
#include "radio_ctrl.h"
#include "lcd_seg.h"
#include "display.h"
#include "seg_panel.h"
#include "seg_lcd_disp.h"
#include "seg_led_disp.h"
#include "sled_disp.h"
#include "rtc_ctrl.h"
#include "breakpoint.h"
#include "sys_on_off.h"
#include "power_monitor.h"
#include "radio_app_interface.h"
#include "debug.h"
#include "pt231x.h"

#ifdef FUNC_DISP_EN

#ifdef FUNC_SEG_LCD_EN
extern TIMER	gAmplitudeTmr;
#endif

VOID ClearSceen(VOID);

//==============================================================================
// Global variants.
DISP_TYPE	gDispBuff[MAX_COM_NUM] = {0};
DISP_DEV	gDisplayMode = DISP_DEV_NONE;
BYTE 		gDispPrevRep;
BYTE 		gDispPrevEQ;
BYTE 		gDispPrevVol;
#ifdef FUNC_PT231X_EN
BYTE 		gDispPrevTreb;
BYTE 		gDispPrevBass;
#endif
BYTE 		gDispPrevPlayState;

TIMER		DispTmr;
TIMER		DispSetTmr;

UN_BLINK gBlink;

WORD DispHoldTime;
WORD Temp;

#ifdef FUNC_POWER_ON_AUDIO_EN
extern BOOL gUSBChargePowerOnFlag;
#endif

#ifdef FUNC_NUMBER_KEY_EN
TIMER NumKeyErrDispTimer;
extern TIMER RecvNumTimer;
extern BOOL NumKeyErrorDispFlag;
extern BOOL NumKeyRadioFreqErrorFlag;
#endif


typedef enum _DISP_STATE
{
	DISP_STATE_IDLE = 0,	//此状态下允许显示播放时间，Radio频率等
	DISP_STATE_DEV,
	DISP_STATE_FILE	//MP3才有此状态	

} DISP_STATE;

DISP_STATE gDispState;//= DISP_STATE_DEV;

extern BYTE gFsInitOK;

#ifdef FUNC_SEG_LED_EN
extern BOOL gLedDispRefreshFlag;
#endif


//----------------------------------------------------------

// Clear digit-value(清数值显示位).
VOID ClearDigit(VOID)
{
#if (defined(FUNC_SEG_LED_EN) || defined(FUNC_SEG_LCD_EN))
	DispString("    ");	  //灭
	DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
	DispIcon(ICON_COLON, LIGHT_OFF);
#ifdef FUNC_SEG_LCD_EN
	if(gDisplayMode == DISP_DEV_LCD58)
	{
		LcdFlushDispData();
	}
#endif
#endif
}


// Clear display(显示全灭).
VOID ClearScreen(VOID)
{
#ifdef FUNC_SEG_LCD_EN
	if(gDisplayMode == DISP_DEV_LCD58)
	{
		memset(gDispBuff, 0, sizeof(gDispBuff));
		LcdFlushDispData();
	}
#endif

#ifdef FUNC_SEG_LED_EN
	if((gDisplayMode < DISP_DEV_LCD58) && (gDisplayMode >= DISP_DEV_LED1888))
	{
		memset(gDispBuff, 0, sizeof(gDispBuff));
#if (defined(FUNC_7PIN_SEG_LED_EN) || defined(FUNC_6PIN_SEG_LED_EN))
		LedPinGpioInit();
//		SetA2IcsReg(A2_ICS_FOR_GPIO, A2_ICS_CLOSE); 
#else
		baGPIOCtrl[LED_SEG_PORT_1P7MA] &= (~MASK_LED_SEG);	
		baGPIOCtrl[LED_SEG_PORT_2P4MA] &= (~MASK_LED_SEG);
#endif
	}
#endif
}


// Display initilization
// Indicate whether the system in "STAND-BY" state or not.
VOID DispInit(BOOL IsStandBy)
{
	//DBG((">>DispInit(%bd)\n", IsStandBy));
	TimeOutSet(&DispTmr, 0);
#ifdef FUNC_NUMBER_KEY_EN
	TimeOutSet(&NumKeyErrDispTimer, 0);
#endif
	gDispPrevRep = gPlayCtrl.RepeatMode;
	gDispPrevEQ = gPlayCtrl.Eq;
	gDispPrevPlayState = gPlayCtrl.State;
#ifdef FUNC_PT231X_EN
       gDispPrevVol = VolMain; 
       gDispPrevTreb = VolTreb;
	gDispPrevBass = VolBass;
#else
	gDispPrevVol = gSys.Volume;
#endif
	gBlink.BlinkFlag = 0;
	gDispState = DISP_STATE_DEV;
	memset(&gDispBuff, 0, sizeof(gDispBuff));	

#ifdef FUNC_SINGLE_LED_EN
	SLedInit();
	SLedLightOp(LED_POWER, LIGHTON);	
#endif

#ifdef FUNC_SEG_LED_EN
#if (defined(FUNC_7PIN_SEG_LED_EN) || defined(FUNC_6PIN_SEG_LED_EN))
	LedPinGpioInit();
//	SetA2IcsReg(A2_ICS_FOR_GPIO, A2_ICS_CLOSE); 
#else
	ClrGpioRegBit(LED_SEG_PORT_PU, MASK_LED_SEG);	// Pull-Up.
	ClrGpioRegBit(LED_SEG_PORT_PD, MASK_LED_SEG);	// Pull-Up.
	ClrGpioRegBit(LED_SEG_PORT_IE, MASK_LED_SEG);	// Input Disable.
	ClrGpioRegBit(LED_SEG_PORT_OE, MASK_LED_SEG);	// Output Disable.
	//ClrGpioRegBit(LED_SEG_PORT_OUT, MASK_LED_SEG);
	SetGpioRegBit(LED_COM_PORT_PU, MASK_LED_COM);	// Pull-Up.
	SetGpioRegBit(LED_COM_PORT_PD, MASK_LED_COM);	// Pull-Up.
	ClrGpioRegBit(LED_COM_PORT_IE, MASK_LED_COM);	// Input Disable.
	SetGpioRegBit(LED_COM_PORT_OE, MASK_LED_COM);	// Output Enable.
#endif
#endif

#ifdef FUNC_SEG_LCD_EN
	//SetGpioRegBit(GPIO_TYPE_REG, MSK_ADC_A1_SWITCH);		//ADC Vol  preq
//	LcdInit(MASK_LCD_SEG, MASK_LCD_COM, LCD_BIAS_2);
//	LcdInit(MASK_LCD_SEG, MASK_LCD_COM, LCD_BIAS_3);
	LcdInit(MASK_LCD_SEG, MASK_LCD_COM, LCD_BIAS_4);
	DBG(("LcdInit(LCD初始化%x,%x,)\n", MASK_LCD_SEG, MASK_LCD_COM));
#endif

#ifdef FUNC_SEG_LED_EN
	gLedDispRefreshFlag = TRUE;
#endif

#if (defined(FUNC_SEG_LED_EN) || defined(FUNC_SEG_LCD_EN))
	if(IsStandBy == TRUE)
	{
		//DBG(("CLS\n"));
		ClearScreen();
	}
	else
	{
		if((gDisplayMode == DISP_DEV_NONE) && (gDisplayMode == DISP_DEV_SLED))
		{
			gDispState = DISP_STATE_IDLE;
		}
		else
		{
			//DBG(("DispLoad()\n"));
			DispPowerOn();
			TimeOutSet(&DispTmr, NORMAL_INTERVAL);
			DispDev();
		}
	}
#ifdef FUNC_SEG_LCD_EN
	if(gDisplayMode == DISP_DEV_LCD58)
	{
		TimeOutSet(&gAmplitudeTmr, 0);
		LcdFlushDispData();
	}	
#endif



#endif	
#if defined(FUNC_SINGLE_LED_EN )
	gDisplayMode = DISP_DEV_SLED;
#elif defined(FUNC_SEG_LCD_EN)
	gDisplayMode = DISP_DEV_LCD58;
#else
	//gDisplayMode = DISP_DEV_LED1888;
	//gDisplayMode = DISP_DEV_LED8888;
	gDisplayMode = DISP_DEV_LED57;
#endif
}


// Display de-initialization.
// Indicate whether the system in "STAND-BY" state or not.
VOID DispDeInit(VOID)
{
#if (defined(FUNC_SEG_LED_EN) || defined(FUNC_SEG_LCD_EN))
	if((gDisplayMode > 0) && (gDisplayMode != DISP_DEV_SLED))
	{
		ClearScreen();
	}
#endif

#ifdef FUNC_SINGLE_LED_EN
	if(gDisplayMode == DISP_DEV_SLED)
	{
		SLedDeinit();	
	}
#endif

#ifdef FUNC_SEG_LED_EN
	gLedDispRefreshFlag = FALSE;	//解决进入Power down 过程中还会有段点亮问题

	if((gDisplayMode == DISP_DEV_LED1888) || (gDisplayMode == DISP_DEV_LED8888) || (gDisplayMode == DISP_DEV_LED57))
	{
#if (defined(FUNC_7PIN_SEG_LED_EN) || defined(FUNC_6PIN_SEG_LED_EN))
		LedPinGpioInit();
//		SetA2IcsReg(A2_ICS_FOR_GPIO, A2_ICS_CLOSE); 
#else
		ClrGpioRegBit(LED_SEG_PORT_PU, MASK_LED_SEG);	// Pull-Up.
		ClrGpioRegBit(LED_SEG_PORT_PD, MASK_LED_SEG);	// Pull-Up.
		SetGpioRegBit(LED_SEG_PORT_IE, MASK_LED_SEG);	// Input Enable.
		ClrGpioRegBit(LED_SEG_PORT_OE, MASK_LED_SEG);	// Output Disable.
		//ClrGpioRegBit(LED_SEG_PORT_OUT,  MASK_LED_SEG);
		ClrGpioRegBit(LED_COM_PORT_PU, MASK_LED_COM);	// Pull-Up.
		ClrGpioRegBit(LED_COM_PORT_PD, MASK_LED_COM);	// Pull-Up.
		SetGpioRegBit(LED_COM_PORT_IE, MASK_LED_COM);	// Input Enable.
		ClrGpioRegBit(LED_COM_PORT_OE, MASK_LED_COM);	// Output Enable.
#endif
	}
#endif

#ifdef FUNC_SEG_LCD_EN
	if(gDisplayMode == DISP_DEV_LCD58)
	{
		//SetGpioRegBit(GPIO_TYPE_REG, MSK_ADC_A1_SWITCH);		//ADC Vol  preq
		LcdDeinit();
	}
#endif
	
	gDispPrevRep = gPlayCtrl.RepeatMode;
	gDispPrevEQ = gPlayCtrl.Eq;
	gDispPrevPlayState = gPlayCtrl.State;
#ifdef FUNC_PT231X_EN
       gDispPrevVol = VolMain; 
       gDispPrevTreb = VolTreb;
	gDispPrevBass = VolBass;
#else	
	gDispPrevVol = gSys.Volume;	
#endif
}


// 设置Repeat模式时调用.
VOID DispRepeat(BOOL IsForced)
{
#if (defined(FUNC_SEG_LED_EN) || defined(FUNC_SEG_LCD_EN))
	if(((gSys.SystemMode == SYS_MODE_USB) || (gSys.SystemMode == SYS_MODE_SD))
	&& ((IsForced == TRUE) || (gDispPrevRep != gPlayCtrl.RepeatMode)))
	{
#ifdef FUNC_SEG_LED_EN
		if((gDisplayMode < DISP_DEV_LCD58) && (gDisplayMode >= DISP_DEV_LED1888))
		{
			LedDispRepeat();
			TimeOutSet(&DispTmr, INTERIM_INTERVAL);
		}
#endif
	
#ifdef FUNC_SEG_LCD_EN
		if(gDisplayMode == DISP_DEV_LCD58)
		{
			LcdDispRepeat();
			LcdFlushDispData();	
		}
#endif
		
#ifdef FUNC_SINGLE_LED_EN
		if(gDisplayMode == DISP_DEV_SLED)
		{

		}
#endif

		gDispPrevRep = gPlayCtrl.RepeatMode;
//		gstDispPlayArg.bRep = gPlayCtrl.RepeatMode & 0x03;
	}
#endif

#if (defined(FUNC_SINGLE_LED_EN) && defined(SLED_REP_EN))
	if(gDisplayMode == DISP_DEV_SLED)
	{
		SLedDispRepeat();
	}
#endif
}


// 设置EQ时调用.
VOID DispEQ(BOOL IsForced)
{
#if (defined(FUNC_SEG_LED_EN) || defined(FUNC_SEG_LCD_EN))
	if((gSys.SystemMode == SYS_MODE_USB) || (gSys.SystemMode == SYS_MODE_SD))
	{
		BYTE CODE EqName[DECD_EQ_SUM][4] =
		{
			" NOR", " ROC", " POP", " CLA", " JAS", " BLU", " HAL", " BAS", " SOF", " COU", " OPE"
		};
	
		if((gDisplayMode <= DISP_DEV_LCD58) && (gDisplayMode >= DISP_DEV_LED1888))
		{
			if((IsForced == TRUE) || (gDispPrevEQ != gPlayCtrl.Eq))
			{
				DispString(&EqName[gPlayCtrl.Eq][0]);
			}
			DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
			DispIcon(ICON_COLON, LIGHT_OFF);
			TimeOutSet(&DispTmr, INTERIM_INTERVAL);
#ifdef FUNC_SEG_LCD_EN
			if(gDisplayMode == DISP_DEV_LCD58)
			{
				LcdFlushDispData();
			}
#endif
		}
	}
	gDispPrevEQ = gPlayCtrl.Eq;
#endif		
}


// 播放状态变化时调用(Play/Pause/Stop/FF/FB).
VOID DispPlayState(VOID)
{
#ifdef FUNC_SEG_LCD_EN
	if(gDisplayMode == DISP_DEV_LCD58)
	{
		LcdDispPlayState();
		LcdFlushDispData();
	}
#endif

#ifdef FUNC_SEG_LED_EN
	if((gDisplayMode <= DISP_DEV_LED57) && (gDisplayMode >= DISP_DEV_LED1888))
	{
		LedDispPlayState();
		if(gDisplayMode != DISP_DEV_LED57)
		{
			TimeOutSet(&DispTmr, NORMAL_INTERVAL);
		}
	}
#endif

#ifdef FUNC_SINGLE_LED_EN
#if (defined(FUNC_USB_EN) || defined(FUNC_CARD_EN))
	if(gDisplayMode == DISP_DEV_SLED)
	{
		SLedDispPlayState();
	}
#endif
#endif
	gDispPrevPlayState = gPlayCtrl.State;
}


// 文件夹功能打开/关闭时调用.
VOID DispFoldState(VOID)
{
#if (defined(FUNC_SEG_LCD_EN) || defined(FUNC_SEG_LED_EN))
	if((gDisplayMode <= DISP_DEV_LCD58) && (gDisplayMode >= DISP_DEV_LED1888))
	{
		if(gPlayCtrl.FolderEnable == TRUE)
		{
			DispString(" FDI");
		}
		else
		{
			DispString(" FEN");
		}
		DispIcon(ICON_COLON, LIGHT_OFF);
		DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
		TimeOutSet(&DispTmr, NORMAL_INTERVAL);
#ifdef FUNC_SEG_LCD_EN
		if(gDisplayMode == DISP_DEV_LCD58)
		{
			LcdFlushDispData();
		}
#endif
	}
#endif
}


// 音量设置时调用.
VOID DispVolume(BOOL IsForced)
{
#if (defined(FUNC_SEG_LED_EN) || defined(FUNC_SEG_LCD_EN))
#ifdef FUNC_PT231X_EN
	//DBG(("DispVolume,0\n"));
	if((gDisplayMode <= DISP_DEV_LCD58) && (gDisplayMode >= DISP_DEV_LED1888))
	{
		if((IsForced == TRUE) || (gDispPrevVol != VolMain))
		{
			//DBG(("DispVolume,1\n"));
			DispChar(' ', POS_COM_THOUSAND);
			DispChar('U', POS_COM_HUNDREDS);
			DispChar((VolMain / 10), POS_COM_TENS);
			DispChar((VolMain % 10), POS_COM_UNITS);
			TimeOutSet(&DispTmr, INTERIM_INTERVAL);
			DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
			DispIcon(ICON_COLON, LIGHT_OFF);
#ifdef FUNC_SEG_LCD_EN
			if(gDisplayMode == DISP_DEV_LCD58)
			{
				DBG(("DispVolume,LcdFlushDispData\n"));
				LcdFlushDispData();
			}
#endif
		}
	}

	gDispPrevVol = gSys.Volume;
#else
	//DBG(("DispVolume,0\n"));
	if((gDisplayMode <= DISP_DEV_LCD58) && (gDisplayMode >= DISP_DEV_LED1888))
	{
		if((IsForced == TRUE) || (gDispPrevVol != gSys.Volume))
		{
			//DBG(("DispVolume,1\n"));
			DispChar(' ', POS_COM_THOUSAND);
			DispChar('U', POS_COM_HUNDREDS);
			DispChar((gSys.Volume / 10), POS_COM_TENS);
			DispChar((gSys.Volume % 10), POS_COM_UNITS);
			TimeOutSet(&DispTmr, INTERIM_INTERVAL);
			DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
			DispIcon(ICON_COLON, LIGHT_OFF);
#ifdef FUNC_SEG_LCD_EN
			if(gDisplayMode == DISP_DEV_LCD58)
			{
				DBG(("DispVolume,LcdFlushDispData\n"));
				LcdFlushDispData();
			}
#endif
		}
	}

	gDispPrevVol = gSys.Volume;
#endif
#endif
	IsForced = 0;
}

#ifdef FUNC_PT231X_EN
// 高音设置时调用.
VOID DispTreb(BOOL IsForced)
{
#if (defined(FUNC_SEG_LED_EN) || defined(FUNC_SEG_LCD_EN))
	//DBG(("DispVolume,0\n"));
	if((gDisplayMode <= DISP_DEV_LCD58) && (gDisplayMode >= DISP_DEV_LED1888))
	{
		if((IsForced == TRUE) || (gDispPrevTreb!= VolTreb))
		{
			//DBG(("DispVolume,1\n"));
			DispChar(' ', POS_COM_THOUSAND);
			DispChar('t', POS_COM_HUNDREDS);
			DispChar((VolTreb / 10), POS_COM_TENS);
			DispChar((VolTreb % 10), POS_COM_UNITS);
			TimeOutSet(&DispTmr, INTERIM_INTERVAL);
			DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
			DispIcon(ICON_COLON, LIGHT_OFF);
#ifdef FUNC_SEG_LCD_EN
			if(gDisplayMode == DISP_DEV_LCD58)
			{
				DBG(("DispVolume,LcdFlushDispData\n"));
				LcdFlushDispData();
			}
#endif
		}
	}

	gDispPrevTreb = VolTreb;

#endif
	IsForced = 0;
}

//  低音设置时调用.
VOID DispBass(BOOL IsForced)
{
#if (defined(FUNC_SEG_LED_EN) || defined(FUNC_SEG_LCD_EN))
	//DBG(("DispVolume,0\n"));
	if((gDisplayMode <= DISP_DEV_LCD58) && (gDisplayMode >= DISP_DEV_LED1888))
	{
		if((IsForced == TRUE) || (gDispPrevBass!= VolBass))
		{
			//DBG(("DispVolume,1\n"));
			DispChar(' ', POS_COM_THOUSAND);
			DispChar('b', POS_COM_HUNDREDS);
			DispChar((VolBass / 10), POS_COM_TENS);
			DispChar((VolBass % 10), POS_COM_UNITS);
			TimeOutSet(&DispTmr, INTERIM_INTERVAL);
			DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
			DispIcon(ICON_COLON, LIGHT_OFF);
#ifdef FUNC_SEG_LCD_EN
			if(gDisplayMode == DISP_DEV_LCD58)
			{
				DBG(("DispVolume,LcdFlushDispData\n"));
				LcdFlushDispData();
			}
#endif
		}
	}

	gDispPrevBass = VolBass;

#endif
	IsForced = 0;
}
#endif

// 显示数值.
VOID DispNum(WORD Num)
{
#if (defined(FUNC_SEG_LED_EN) || defined(FUNC_SEG_LCD_EN))
	//DBG(("DispVolume,0\n"));
	if((gDisplayMode <= DISP_DEV_LCD58) && (gDisplayMode >= DISP_DEV_LED1888))
	{	
#ifdef	FUNC_NUMBER_KEY_EN
		if(((NumKeyRadioFreqErrorFlag == TRUE) && (gSys.SystemMode == SYS_MODE_RADIO))
		|| ((Num > gFsInfo.FileSum) && ((gSys.SystemMode == SYS_MODE_USB) || (gSys.SystemMode == SYS_MODE_SD))))
		{
			if(NumKeyErrorDispFlag == FALSE)
			{
				Num2Char(Num, FALSE);
				NumKeyErrorDispFlag = TRUE;
				TimeOutSet(&RecvNumTimer, NORMAL_INTERVAL);
				TimeOutSet(&DispTmr, NORMAL_INTERVAL);
				TimeOutSet(&NumKeyErrDispTimer, 500);  //数字显示500ms
			}
			else
			{
				if(IsTimeOut(&NumKeyErrDispTimer))
				{
					DispString(" Err");
				}					
			}
		}
		else
#endif
		{
			Num2Char(Num, FALSE);
		}

#ifdef	FUNC_NUMBER_KEY_EN
		if((NumKeyErrorDispFlag == FALSE) && (NumKeyRadioFreqErrorFlag == FALSE))
#endif
		{
			TimeOutSet(&DispTmr, SET_INTERVL);
		}
#ifdef	FUNC_NUMBER_KEY_EN
		else
		{
			if(IsTimeOut(&DispTmr))
			{
				NumKeyRadioFreqErrorFlag = FALSE;
				NumKeyErrorDispFlag = FALSE;
			}			
		}
#endif
		DispIcon(ICON_COLON, LIGHT_OFF);
	}
#ifdef FUNC_SEG_LCD_EN
	if(gDisplayMode == DISP_DEV_LCD58)
	{
		LcdFlushDispData();
	}
#endif
#else
	Num = 0;
#endif
}


// 文件切换时调用.
VOID DispFileNum(VOID)
{
#if (defined(FUNC_SEG_LED_EN) || defined(FUNC_SEG_LCD_EN))

#ifdef FUNC_RTC_EN
	if(RtcAutOutTimeCount > 0)
	{
		return;
	}
#endif

	//DBG((">>DispFileNum\n"));
	if(gFsInfo.FileSum > 0)	//if(gFsInitOK == 1)
	{
		//DBG(("gPlayCtrl.FileNum:%u\n", gPlayCtrl.FileNum));
		Num2Char(gPlayCtrl.FileNum, FALSE);		
		if((gPlayCtrl.FileNum > 9999) && (gDisplayMode > DISP_DEV_LED1888))
		{
			DispChar('F', 3);
		}
	}
	else
	{
		DBG(("NOF\n"));
		DispString(" NOF");
	}
	TimeOutSet(&DispTmr, DEV_SYMBOL_INTERVAL);
	DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
	DispIcon(ICON_COLON, LIGHT_OFF);
	DispIcon(ICON_PAUSE, LIGHT_OFF);
#ifdef FUNC_SEG_LCD_EN
	if(gDisplayMode == DISP_DEV_LCD58)
	{
		LcdFlushDispData();
	}
#endif
#endif
}


// 显示播放时间.
VOID DispPlayTime(BYTE Type)
{
#if (defined(FUNC_SEG_LCD_EN) || defined(FUNC_SEG_LED_EN))
	WORD TimeMMSS;
	static DWORD PrevTime = 0;

	if(DIS_PLAYTIME_PALY == Type)
	{
		if(PrevTime != gSongInfo.CurPlayTime)
		{
			PrevTime = gSongInfo.CurPlayTime;
		}
		else
		{
			return;
		}
	}

	TimeMMSS = gSongInfo.CurPlayTime / 60;
	TimeMMSS *= 100;
	TimeMMSS += gSongInfo.CurPlayTime % 60;
	Num2Char(TimeMMSS, TRUE);
	//DBG(("gSongInfo.CurPlayTime:%u, TimeMMSS:%u\n", gSongInfo.CurPlayTime, TimeMMSS));
	if((TimeMMSS > 9999) && (gDisplayMode > DISP_DEV_LED1888))
	{
		DispChar('P', 3);
	}

	if(DIS_PLAYTIME_PAUSE == Type)
	{
		return;
	}
	
	DispIcon(ICON_COLON, LIGHT_ON);
	DispIcon(ICON_PLAY, LIGHT_ON);
	DispIcon(ICON_PAUSE, LIGHT_OFF);
	DispIcon(ICON_MP3, LIGHT_ON);
#ifdef FUNC_SEG_LCD_EN
	if(gDisplayMode == DISP_DEV_LCD58)
	{
		LcdFlushDispData();
	}
#endif
	//DBG(("**%02BX, %02BX, %02BX, %02BX, %02BX\n", gDispBuff[0], gDispBuff[1], gDispBuff[2], gDispBuff[3], gDispBuff[3]));
#else	
	Type = 0;
#endif	
}


// Exit from Special Display-Mode.
VOID DispResume(VOID)
{	
#if (defined(FUNC_SEG_LCD_EN) || defined(FUNC_SEG_LED_EN))
	if((gDisplayMode <= DISP_DEV_LCD58) && (gDisplayMode >= DISP_DEV_LED1888))
	{
		switch(gSys.SystemMode)
		{
			case SYS_MODE_NONE:				
#ifndef FUNC_RTC_EN
				//DBG(("NOD1"));
				DispString(" NOD");	
#else
				DispRtc();
#endif
				break;

#if (defined(FUNC_USB_EN) || defined(FUNC_CARD_EN))
			case SYS_MODE_SD:
			case SYS_MODE_USB:
				if(gFsInfo.FileSum == 0)
				{
					//DBG(("**NOF\n"));
					DispString(" NOF");	
				}
				else if((gPlayCtrl.State == PLAY_STATE_STOP) || (gPlayCtrl.State == PLAY_STATE_PAUSE))
				{
					DispPlayState();
				}
#ifdef FUNC_FAST_PLAY_EN
				else if((gPlayCtrl.State == PLAY_STATE_PLAY) || (gPlayCtrl.State == PLAY_STATE_FF) || (gPlayCtrl.State == PLAY_STATE_FB))
#endif
				{
					if(gDisplayMode == DISP_DEV_LED1888)
					{
						DispFileNum();
					}
					else
					{						
						DispPlayTime(DIS_PLAYTIME_PALY);
					}
				}
				if(gSys.SystemMode == SYS_MODE_SD)
				{
					DispIcon(ICON_SD, LIGHT_ON);
				}
				else
				{
					DispIcon(ICON_USB, LIGHT_ON);
				}
				break;
#endif

#ifdef FUNC_RADIO_EN
			case SYS_MODE_RADIO:
				if(RadioDisFlag == RadioDisCurFreNum)
				{
					if((gRadioData.CurrFreq >= RADIO_LOWER_BOUND) && (gRadioData.CurrFreq <= RADIO_UPPER_BOUND))
					{
						if(gSys.MuteFg == TRUE)	//暂停状态
						{
							DispString("PAUS");
						}
						else
						{
							Num2Char(gRadioData.CurrFreq, FALSE);						
							DispIcon(ICON_DP_FM_MHZ, LIGHT_ON);
						}
#ifdef AU6210K_BOOMBOX_DEMO
						if(gRadioCtrl.State != RADIO_IDLE)
						{
							DispIcon(ICON_STERO, LIGHT_OFF);
						}
						else if((gRadioMonoEnableFlag == FALSE) && (RadioGetStereoState()))
						{
							DispIcon(ICON_STERO, LIGHT_ON);
						}
#endif
					}					
#ifdef AU6210K_BOOMBOX_DEMO
					DispIcon(ICON_MEMORY, LIGHT_OFF);
#endif
				}
				else if(RadioDisFlag == RadioDisCurChAutoscan)
				{						
					DispChar(' ', POS_COM_THOUSAND);
					DispChar('P', POS_COM_HUNDREDS);
					if(gRadioData.StationSum > 9)
					{
						DispChar((gRadioData.StationSum / 10), POS_COM_TENS);
						DispChar((gRadioData.StationSum % 10), POS_COM_UNITS);
					}
					else
					{
						DispChar(' ', POS_COM_TENS);
						DispChar(gRadioData.StationSum, POS_COM_UNITS);
					}
					DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
#ifdef AU6210K_BOOMBOX_DEMO
					DispIcon(ICON_STERO, LIGHT_OFF);
#endif
				}
				else if(RadioDisFlag == RadioDisCurChChange)
				{
					DispChar(' ', POS_COM_THOUSAND);
					DispChar('P', POS_COM_HUNDREDS);
					if((gRadioData.CurrStation + 1) > 9)
					{
						DispChar(((gRadioData.CurrStation + 1) / 10), POS_COM_TENS);
						DispChar(((gRadioData.CurrStation + 1) % 10), POS_COM_UNITS);
					}
					else
					{
						DispChar(' ', POS_COM_TENS);
						DispChar((gRadioData.CurrStation + 1), POS_COM_UNITS);
					}
					DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
#ifdef AU6210K_BOOMBOX_DEMO
					DispIcon(ICON_MEMORY, LIGHT_OFF);
					DispIcon(ICON_STERO, LIGHT_OFF);
#endif
				}
#ifdef AU6210K_BOOMBOX_DEMO
				else if(RadioDisFlag == RadioDisCurProgramNum)
				{
					DispChar(' ', POS_COM_THOUSAND);
					DispChar('P', POS_COM_HUNDREDS);
					if((gRadioData.CurrStation + 1) > 9)
					{
						DispChar(((gRadioData.CurrStation + 1) / 10), POS_COM_TENS);
						DispChar(((gRadioData.CurrStation + 1) % 10), POS_COM_UNITS);
					}
					else
					{
						DispChar(' ', POS_COM_TENS);
						DispChar((gRadioData.CurrStation + 1), POS_COM_UNITS);
					}
					DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
					DispIcon(ICON_MEMORY, LIGHT_OFF);
					DispIcon(ICON_STERO, LIGHT_OFF);
				}
				else if(RadioDisFlag == RadioDisCurPresetCh)
				{
					if((gRadioData.CurrFreq >= RADIO_LOWER_BOUND) && (gRadioData.CurrFreq <= RADIO_UPPER_BOUND))
					{							
						Num2Char(gRadioData.CurrFreq, FALSE);						
						DispIcon(ICON_DP_FM_MHZ, LIGHT_ON);
						DispIcon(ICON_MEMORY, LIGHT_ON);	
						if((gRadioMonoEnableFlag == FALSE) && (RadioGetStereoState()))
						{
							DispIcon(ICON_STERO, LIGHT_ON);
				}
					}
				}
#endif
				DispIcon(ICON_USB, LIGHT_OFF);
				DispIcon(ICON_SD, LIGHT_OFF);
				DispIcon(ICON_COLON, LIGHT_OFF);
				break;

#endif

#ifdef FUNC_LINEIN_EN
			case SYS_MODE_LINEIN:
				DispString(" AUX");
				DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
				DispIcon(ICON_COLON, LIGHT_OFF);
				break;
#endif

#ifdef FUNC_BLUETOOTH_EN
			case SYS_MODE_BLUETOOTH:
				DispIcon(ICON_USB, LIGHT_OFF);
				DispIcon(ICON_SD, LIGHT_OFF);
				DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
				DispIcon(ICON_MP3, LIGHT_OFF);
				DispIcon(ICON_PLAY, LIGHT_OFF);
				DispIcon(ICON_PAUSE, LIGHT_OFF);
				DispIcon(ICON_COLON, LIGHT_OFF);
				DispString("BLUE");
				break;
#endif


//#ifdef FUNC_RTC_EN
//	   		case SYS_MODE_RTC:
//				Temp = gRtcTime.Hour*100;
//				Temp += gRtcTime.Min;
//				Num2Char(Temp, TRUE);
//				DispIcon(ICON_COLON, LIGHT_BLINK);
//				break;
//#endif

#ifdef FUNC_AUDIO_EN
			case SYS_MODE_AUDIO:				
				DispString(" PC ");
			 	break;
#endif

#ifdef FUNC_READER_EN
			case SYS_MODE_READER:
				DispString(" REA");
			 	break;
#endif				

#ifdef FUNC_AUDIO_READER_EN
			case SYS_MODE_AUDIO_READER:
				DispString(" PC ");
			 	break;
#endif

#ifdef FUNC_STANDBY_EN
			case SYS_MODE_STANDBY:				
#ifndef FUNC_RTC_EN
				DispString(" CLO"); 
#else
				DispRtc();
#endif				
				break;
#endif

			default:
				break;
		}

#ifdef FUNC_SEG_LCD_EN
		if(gDisplayMode == DISP_DEV_LCD58)
		{
			LcdFlushDispData();
		}
#endif
	}
#endif	
}


// Mute状态变化时调用.
VOID DispMute(VOID)
{
#ifdef FUNC_SEG_LCD_EN
	if(gDisplayMode == DISP_DEV_LCD58)
	{
		if(gSys.MuteFg == TRUE)
		{
	   		DispIcon(ICON_MUTE, LIGHT_ON);
		}
		else
		{
			DispIcon(ICON_MUTE, LIGHT_OFF);
		}

		LcdFlushDispData();
	}
#endif

#ifdef FUNC_SEG_LED_EN
//这部分代码用户可以根据需要打开
//	if((gDisplayMode < DISP_DEV_LCD58) && (gDisplayMode >= DISP_DEV_LED1888))
//	{
//	 	if(gSys.MuteFg == TRUE)
//		{
//			//DBG(("LED MUTE\n"));
//			if(((gSys.SystemMode == SYS_MODE_USB) || (gSys.SystemMode == SYS_MODE_SD)) 
//			&& ((gPlayCtrl.State == PLAY_STATE_STOP) || (gPlayCtrl.State == PLAY_STATE_BROWSE) || (gPlayCtrl.State == PLAY_STATE_IDLE)))
//			{
//				//DBG(("MUTE Exit\n"));
//				gBlink.Blink.MuteBlink = 0;
//				gBlink.Blink.MuteBlankScreen = 0;
//				TimeOutSet(&DispTmr, 0);
//				// Todo here...
//			}
//			else if(!gBlink.Blink.MuteBlink)
//			{
//				//DBG(("MUTE Enter\n"));
//				gBlink.Blink.MuteBlink = 1;
//				gBlink.Blink.MuteBlankScreen = 1;
//				ClearDigit();
//				TimeOutSet(&DispTmr, MUTE_BLINK_INTERVAL);	
//			}
//		}
//		else
//		{
//			gBlink.Blink.MuteBlink = 0;
//			gBlink.Blink.MuteBlankScreen = 0;
//			TimeOutSet(&DispTmr, 0);
//			// Todo here...
//		}
//	}
#endif

#ifdef FUNC_SINGLE_LED_EN
	if(gDisplayMode == DISP_DEV_SLED)
	{
		//DBG(("SLedDispMute\n"));
		SLedDispMute();
	}
#endif
}


//// 换台、存台时调用.
//VOID DispStationNum(VOID)
//{
//
//}


// Show Device symbol.
VOID DispDevSymbol(VOID)
{
#ifdef FUNC_SEG_LED_EN
	if((gDisplayMode == DISP_DEV_LED8888) || (gDisplayMode == DISP_DEV_LED1888) || (gDisplayMode == DISP_DEV_LED57))
	{
		LedDispDevSymbol();
		TimeOutSet(&DispTmr, DispHoldTime);
	}
#endif

#ifdef FUNC_SEG_LCD_EN
	if(gDisplayMode == DISP_DEV_LCD58)
	{
		LcdDispDevSymbol();
		TimeOutSet(&DispTmr, DispHoldTime);
		LcdFlushDispData();
	}
#endif

#ifdef FUNC_SINGLE_LED_EN
	if(gDisplayMode == DISP_DEV_SLED)
	{
		SLedDispDevSymbol();
	}	
#endif	
}


VOID DispLoad(VOID)
{
#if (defined(FUNC_SEG_LED_EN) || defined(FUNC_SEG_LCD_EN))
	if((gDisplayMode <= DISP_DEV_LCD58) && (gDisplayMode >= DISP_DEV_LED1888))
	{
		ClearScreen();
#ifdef FUNC_POWER_MONITOR_EN
		PowerMonitorDisp();
#endif
		DispString(" LOD");
#ifdef FUNC_SEG_LCD_EN
		if(gDisplayMode == DISP_DEV_LCD58)
		{
			LcdFlushDisp();
		}
#endif
	}
#endif
}


// 进入Audio显示时调用
VOID DispAudio(VOID)
{	
#if (defined(FUNC_SEG_LED_EN) || defined(FUNC_SEG_LCD_EN))
	ClearScreen();
	DispString(" PC ");
#endif	
}


//显示Power on信息
VOID DispPowerOn(VOID)
{
#if ( defined(FUNC_SEG_LED_EN) || defined(FUNC_SEG_LCD_EN) )
	if( (gDisplayMode <= DISP_DEV_LCD58) && (gDisplayMode >= DISP_DEV_LED1888) )
	{
		ClearScreen();
		DispString(" HI");
#ifdef FUNC_SEG_LCD_EN
		if(gDisplayMode == DISP_DEV_LCD58)
		{
			LcdFlushDispData();
		}
#endif
	}
#endif
}


// 设备切换时调用.
VOID DispDev(VOID)
{
//	DBG((">>DispDev()\n"));

#if (defined(FUNC_SEG_LED_EN) || defined(FUNC_SEG_LCD_EN))
	if((gDisplayMode <= DISP_DEV_LCD58) && (gDisplayMode >= DISP_DEV_LED1888))
	{
		if(gDispState == DISP_STATE_IDLE)
		{
			gDispState = DISP_STATE_DEV;	
		}
		//DBG(("gDispState:%bd\n", gDispState));
	
		if(IsTimeOut(&DispTmr))
		{
			switch(gDispState)
			{
				case DISP_STATE_DEV:
					//DBG(("DISP_STATE_DEV\n"));
					DispDevSymbol();
					if((gSys.SystemMode == SYS_MODE_USB) || (gSys.SystemMode == SYS_MODE_SD))
					{
						if(gFsInitOK == 1)
						{
							gDispState = DISP_STATE_FILE;
						}
						else if(gFsInitOK != 0)
						{
							gDispState = DISP_STATE_IDLE;	
						}
					}
					else
					{
						gDispState = DISP_STATE_IDLE;
					}
					break;
		
				case DISP_STATE_FILE:
					//DBG(("DISP_STATE_FILE\n"));
					if((gSys.SystemMode == SYS_MODE_USB) || (gSys.SystemMode == SYS_MODE_SD))
					{
						if(gFsInitOK == 1)
						{
							DispFileNum();
						}
						else
						{
							DispString(" FER");
						}
					}
					gDispState = DISP_STATE_IDLE;
					break;
		
				default:
					//DBG(("DISP_STATE_IDLE\n"));
					gDispState = DISP_STATE_IDLE;
					break;
			}
	
#ifdef FUNC_SEG_LCD_EN
			//DBG(("gDisplayMode:%bd\n", gDisplayMode));
			if(gDisplayMode == DISP_DEV_LCD58)
			{
				LcdFlushDispData();
			}
#endif
			//DBG(("%02BX, %02BX, %02BX, %02BX, %02BX\n", gDispBuff[0], gDispBuff[1], gDispBuff[2], gDispBuff[3], gDispBuff[3]));
		}
	}
#endif

#ifdef FUNC_SINGLE_LED_EN
	if(gDisplayMode == DISP_DEV_SLED)
	{	
		SLedDispDevSymbol();
	}
#endif

//	DBG(("<<DispDev()\n"));
}


#ifdef FUN_SYSTEM_POWEROFF_WAIT_TIME
//该函数级别高主要处理设置
static BYTE DispSysTimOff(VOID)
{
	if((gSysTimeOffState.ProState > SYS_OFF_TIMEING_IDLE) && (gSysTimeOffState.ProState < SYS_OFF_TIMEING_WAIT_POWEROFF))
	{
		if(gDisplayMode == DISP_DEV_LED57)
		{
			DispIcon(ICON_PLAY, LIGHT_OFF);	
			DispIcon(ICON_PAUSE, LIGHT_OFF);
			DispIcon(ICON_COLON, LIGHT_OFF);
			DispIcon(ICON_MP3, LIGHT_OFF);				
		}
		else if(gDisplayMode == DISP_DEV_LED8888)
		{
			DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
			DispIcon(ICON_COLON, LIGHT_OFF);		
		}
							
		if(gSysTimeOffState.EnState == TRUE)
		{			
			DispIcon(ICON_SLEEP, LIGHT_ON);	
			
			switch(gSysTimeOffState.SleepState)
			{
				case SLEEP_15:
					DispString("  15");
					break;

				case SLEEP_30:
					DispString("  30");
					break;

				case SLEEP_45:
					DispString("  45");
					break;
					
				case SLEEP_60:
					DispString("  60");
					break;

				case SLEEP_75:
					DispString("  75");
					break;
					
				case SLEEP_90:
					DispString("  90");
					break;

				case SLEEP_105:
					DispString(" 105");
					break;
					
				case SLEEP_120:
					DispString(" 120");
					break;	
					
				default:
					break;
			}	
		}
		else
		{
			DispString(" CLO");
			DispIcon(ICON_SLEEP, LIGHT_OFF);		
		}
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
#endif


#ifdef FUNC_RTC_EN
VOID DispRtc(VOID)
{
	RTC_DATE_TIME gDisRtcVal;
	
#ifdef FUNC_ALARM_EN		
	if(RtcState == RTC_STATE_SET_ALARM)
	{
		gDisRtcVal = gAlarmTime;
	}
	 else
#endif	 	
	{
		gDisRtcVal = gRtcTime;
	}
	
#if (defined(FUNC_SEG_LED_EN) || defined(FUNC_SEG_LCD_EN))
	if((gDisplayMode <= DISP_DEV_LCD58) && (gDisplayMode >= DISP_DEV_LED1888))
	{	
		if(gDisplayMode == DISP_DEV_LED57)
		{
			DispIcon(ICON_PLAY, LIGHT_OFF);	
			DispIcon(ICON_PAUSE, LIGHT_OFF);
			DispIcon(ICON_USB, LIGHT_OFF);
			DispIcon(ICON_SD, LIGHT_OFF);
			DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
			DispIcon(ICON_MP3, LIGHT_OFF);
		}
		else if(gDisplayMode == DISP_DEV_LED8888)
		{
			DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
		}
		
#ifdef	AU6210K_CAR_MODEL_DEMO
		if(RtcState == RTC_STATE_SET_SOURCE)
		{
			switch(RtcSourceState)
			{
				case RTC_SOURCE_CLOCK:
					Temp = gRtcTime.Hour*100;
					Temp += gRtcTime.Min;		
					Num2Char(Temp, TRUE);
					DispIcon(ICON_SLEEP, LIGHT_OFF);
					if(gRtcTime.Sec%2)//秒闪烁
					{
						DispIcon(ICON_COLON, LIGHT_ON);	
					}
					else
					{
						DispIcon(ICON_COLON, LIGHT_OFF);	
					}			
					break;
					
				case RTC_SOURCE_YEAR:
					Temp = gRtcTime.Year;	
					Num2Char(Temp, TRUE);
					DispIcon(ICON_COLON, LIGHT_OFF);				
					break;

				case RTC_SOURCE_MON:
					Temp = gRtcTime.Mon*100;
					Temp += gRtcTime.Date;		
					Num2Char(Temp, TRUE);
					DispIcon(ICON_COLON, LIGHT_OFF);					
					break;

				case RTC_SOURCE_ALARM:
					Temp = gAlarmTime.Hour*100;
					Temp += gAlarmTime.Min;		
					Num2Char(Temp, TRUE);
					DispIcon(ICON_COLON, LIGHT_ON);	
					if(gRtcTime.Sec%2)//秒闪烁
					{
						DispIcon(ICON_SLEEP, LIGHT_ON);
					}
					else
					{
						DispIcon(ICON_SLEEP, LIGHT_OFF);
					}		
					break;

				case RTC_SOURCE_ONOFF:
					DispIcon(ICON_COLON, LIGHT_OFF);	
					if(gRtcTime.Sec%2)//秒闪烁
					{
						DispIcon(ICON_SLEEP, LIGHT_ON);
					}
					else
					{
						DispIcon(ICON_SLEEP, LIGHT_OFF);
					}		
					if(gAlarmState == ALARM_STATUS_OPEN)
					{
						DispChar(' ', 0);
						DispChar('O', 1);
						DispChar('N', 2);
						DispChar(' ', 3);
					}
					else if(gAlarmState == ALARM_STATUS_CLOSE)
					{
						DispChar(' ', 0);
						DispChar('O', 1);
						DispChar('F', 2);
						DispChar('F', 3);
					}
					break;

				default:
					break;			
			}
		}
		else
#endif
		{
			switch(RtcSubState)
			{
				case RTC_SET_NONE:			
					Temp = gRtcTime.Hour * 100;
					Temp += gRtcTime.Min;		
					Num2Char(Temp, TRUE);
					
#ifdef AU6210K_BOOMBOX_DEMO
					if(gAlarm1State == ALARM_STATUS_OPEN)
					{
						DispIcon(ICON_ALARM1, LIGHT_ON);
					}
					else
					{
						DispIcon(ICON_ALARM1, LIGHT_OFF);
					}
					
					if(gAlarm2State == ALARM_STATUS_OPEN)
					{
						DispIcon(ICON_ALARM2, LIGHT_ON);
					}
					else
					{
						DispIcon(ICON_ALARM2, LIGHT_OFF);
					}

					DispIcon(ICON_COLON, LIGHT_ON);	
					
#else
					
					if(gRtcTime.Sec % 2)//秒闪烁
					{
						DispIcon(ICON_COLON, LIGHT_ON);	
					}
					else
					{
						DispIcon(ICON_COLON, LIGHT_OFF);	
					}				
#endif
					break;
				
#ifdef FUNC_ALARM_EN
				case RTC_SET_ALARM_NUM:	
					DispIcon(ICON_COLON, LIGHT_OFF);	
					DispChar('A', 0);
					DispChar('L', 1);
					DispChar(' ', 2);
					
					if(RtcAutOutTimeCount % 2)
					{
						DispChar(RtcAlarmNum + 0x30, 3);
					}
					else
					{
						DispChar(' ', 3);
					}
					break;
		
				case RTC_SET_ALARM_MODE:
					DispIcon(ICON_COLON, LIGHT_OFF);	
					DispChar('A', 0);
					DispChar('O', 1);
					DispChar(' ', 2);
					
					if(RtcAutOutTimeCount % 2)
					{
						DispChar(gAlarmMode + 0x30, 3);
					}
					else
					{
						DispChar(' ', 3);
					}
					break;
		
				case RTC_SET_WEEK:
					DispIcon(ICON_COLON, LIGHT_OFF);	
					DispChar('A', 0);
					DispChar(' ', 1);
					DispChar(' ', 2);
					if(RtcAutOutTimeCount%2)
					{
						DispChar((gDisRtcVal.WDay + 0x30), 3);
					}
					else
					{
						DispChar(' ', 3);
					}
					break;
#endif
				
				case RTC_SET_YEAR:
					DispIcon(ICON_COLON, LIGHT_OFF);		
					if(gRtcRdWrFlag == SET_RTC_TIME_YEAR)		
					{					
						if(RtcAutOutTimeCount % 2)
						{
							Temp = gDisRtcVal.Year;
							Num2Char(Temp, TRUE);				
						}
						else
						{
							DispChar(' ', 3);
							DispChar(' ', 2);					
							DispChar(' ', 1);
							DispChar(' ', 0);	
						}	
					}
					else if(gRtcRdWrFlag == READ_RTC_TIME)//year->mon data->weekday->hour min
					{					
						if(RtcAutOutTimeCount > 15)				
						{
							Temp = gDisRtcVal.Year;
						}
						else if(RtcAutOutTimeCount > 10)
						{
							Temp = gDisRtcVal.Mon * 100;
							Temp += gDisRtcVal.Date;
						}
						else if(RtcAutOutTimeCount > 5)
						{
							Temp = gDisRtcVal.WDay;
						}
						else
						{
							Temp = gDisRtcVal.Hour * 100;
							Temp += gDisRtcVal.Min;
						}
						Num2Char(Temp, TRUE);									
					}
					break;
		
				case RTC_SET_MON:
					DispIcon(ICON_COLON, LIGHT_OFF);	
					if(RtcAutOutTimeCount % 2)
					{
						Temp = gDisRtcVal.Mon * 100;
						Temp += gDisRtcVal.Date;		
						Num2Char(Temp, TRUE);
					}
					else
					{
						DispChar(' ', 1);
						DispChar(' ', 0);
					}		
					break;
		
				case RTC_SET_DATE:
					DispIcon(ICON_COLON, LIGHT_OFF);	
					if(RtcAutOutTimeCount % 2)
					{
						Temp = gDisRtcVal.Mon * 100;
						Temp += gDisRtcVal.Date;		
						Num2Char(Temp, TRUE);
					}
					else
					{					
						DispChar(' ', 3);
						DispChar(' ', 2);
					}	
					break;
											
				case RTC_SET_HR:
					DispIcon(ICON_COLON, LIGHT_ON);	
					if(RtcAutOutTimeCount % 2)
					{
						Temp = gDisRtcVal.Hour * 100;
						Temp += gDisRtcVal.Min;		
						Num2Char(Temp, TRUE);
					}
					else
					{
						DispChar(' ', 1);
						DispChar(' ', 0);				
					}		
					break;
		
				case RTC_SET_MIN:
					DispIcon(ICON_COLON, LIGHT_ON);	
					if(RtcAutOutTimeCount % 2)
					{
						Temp = gDisRtcVal.Hour * 100;
						Temp += gDisRtcVal.Min;		
						Num2Char(Temp, TRUE);					
					}
					else
					{					
						DispChar(' ', 3);
						DispChar(' ', 2);
					}	
					break;

#ifdef	AU6210K_CAR_MODEL_DEMO
				case RTC_SET_ALARM_ONOFF:
					DispIcon(ICON_COLON, LIGHT_OFF);						
					if(gAlarmState == ALARM_STATUS_OPEN)
					{
					   	if(RtcAutOutTimeCount%2)
						{
							DispChar(' ', 0);
							DispChar('O', 1);
							DispChar('N', 2);
							DispChar(' ', 3);
						}
						else
						{
							DispChar(' ', 3);
							DispChar(' ', 2);
							DispChar(' ', 1);
							DispChar(' ', 0);
						}
					}
					else if(gAlarmState == ALARM_STATUS_CLOSE)
					{
					   	if(RtcAutOutTimeCount%2)
						{
							DispChar(' ', 0);
							DispChar('O', 1);
							DispChar('F', 2);
							DispChar('F', 3);
						}
						else
						{
							DispChar(' ', 3);
							DispChar(' ', 2);
							DispChar(' ', 1);
							DispChar(' ', 0);
						}
					}
					break;
#endif
					
#ifdef AU6210K_BOOMBOX_DEMO
				case RTC_SET_ALARM_SOURCE:
					if(gAlarmSource == RTC_ALARM_SOURCE_RADIO)
					{
						DispChar(' ', 0);
						DispChar('A', 1);
						DispChar('F', 2);
						DispChar(' ', 3);
					}
					else if(gAlarmSource == RTC_ALARM_SOURCE_USB)
					{
						DispChar(' ', 0);
						DispChar('A', 1);
						DispChar('U', 2);
						DispChar(' ', 3);
					}
					else if(gAlarmSource == RTC_ALARM_SOURCE_CARD)
					{
						DispChar(' ', 0);
						DispChar('A', 1);
						DispChar('D', 2);
						DispChar(' ', 3);
					}
					else if(gAlarmSource == RTC_ALARM_SOURCE_AUX)
					{
						DispChar(' ', 0);
						DispChar('A', 1);
						DispChar('A', 2);
						DispChar(' ', 3);
					}
					else if(gAlarmSource == RTC_ALARM_ONOFF)
					{
						DispChar(' ', 0);
						DispChar('O', 1);
						DispChar('F', 2);
						DispChar('F', 3);
					}
					DispIcon(ICON_COLON, LIGHT_OFF);	
					break;
		
				case RTC_SET_ALARM_VOLUME:					
					DispChar(' ', POS_COM_THOUSAND);
					DispChar('U', POS_COM_HUNDREDS);
					DispChar((gAlarmVolume / 10), POS_COM_TENS);
					DispChar((gAlarmVolume % 10), POS_COM_UNITS);
					DispIcon(ICON_COLON, LIGHT_OFF);	
					break;
#endif
					
				default:
					break;
			}	
		}
	}
#endif
}


//该 函数级别高主要处理RTC按键和判断闹钟是否到来
static BYTE DispRtcSet(VOID)
{
	BOOL Ret = FALSE;

	if(RtcSubState == RTC_SET_NONE)
	{	
#ifdef FUNC_ALARM_EN	
		if(RtcAutOutTimeCount > 0)//闹钟到	
		{
			Temp = gRtcTime.Hour * 100;//此时二者时间相等
			Temp += gRtcTime.Min;	
			
#if (defined(FUNC_SEG_LED_EN) || defined(FUNC_SEG_LCD_EN))
			if(gDisplayMode == DISP_DEV_LED57)
			{
				DispIcon(ICON_PLAY, LIGHT_OFF);	
				DispIcon(ICON_PAUSE, LIGHT_OFF);
				DispIcon(ICON_USB, LIGHT_OFF);
				DispIcon(ICON_SD, LIGHT_OFF);
				DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
				DispIcon(ICON_MP3, LIGHT_OFF);
			}
			else if(gDisplayMode == DISP_DEV_LED8888)
			{
				DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
			}
		
			if(RtcAutOutTimeCount % 2)
			{
				Num2Char(Temp, TRUE);
			}
			else
			{
				ClearDigit();
			}

			Ret = TRUE;		
#endif		
		}
#endif				
	}
	else
	{
		DispRtc();//设置时间或者闹钟
		Ret = TRUE;
	}

#ifdef FUNC_SEG_LCD_EN
	if(gDisplayMode == DISP_DEV_LCD58)
	{
		LcdFlushDispData();
	}
#endif

	return Ret;	 
}
#endif


// 主函数调用.
VOID Display(VOID)
{
#if (defined(FUNC_SEG_LED_EN) || defined(FUNC_SEG_LCD_EN))
	if((gDisplayMode <= DISP_DEV_LCD58) && (gDisplayMode >= DISP_DEV_LED1888))
	{
#ifdef FUN_SYSTEM_POWEROFF_WAIT_TIME
		if(DispSysTimOff())
		{
			return;
		}
#endif

#ifdef FUNC_RTC_EN
		if(DispRtcSet())
		{
			return;
		}
#endif

		//DBG(("gDispState:%bd\n", gDispState));
		if(gDispState != DISP_STATE_IDLE)
		{
			DispDev();
			return;	// Waiting for " LOD" timeout.
		}
		
		if(IsTimeOut(&DispTmr))
		{
			if(gBlink.BlinkFlag)
			{
				TimeOutSet(&DispTmr, MUTE_BLINK_INTERVAL);
				if(gBlink.Blink.MuteBlink)		// MUTE闪烁
				{
					if(gBlink.Blink.MuteBlankScreen)
					{
						 gBlink.Blink.MuteBlankScreen = 0;
						 DispResume();
					}
					else
					{
						gBlink.Blink.MuteBlankScreen = 1;
						ClearDigit();	//清数值显示
					}
				}			
			}
			else
			{
				DispResume();
			}			
		}
#ifdef FUNC_SEG_LCD_EN
		else if(gDisplayMode == DISP_DEV_LCD58)
		{//为解决显示音量、EQ等状态时没有幅度显示的问题，增加以下代码。
			if(IsTimeOut(&gAmplitudeTmr))
			{
				LcdFlushDispData();//在LcdFlushDispData函数内部调用幅度显示函数。
			}
		}
#endif
	}
#endif


#ifdef FUNC_SINGLE_LED_EN
	if(gDisplayMode == DISP_DEV_SLED)
	{	
		SLedDisplay();
	}
#endif
}


#endif

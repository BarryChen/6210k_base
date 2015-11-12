#include "syscfg.h"
#include "sysctrl.h"
#include "device.h"
#include "user_interface.h"
#include "breakpoint.h"
#include "debug.h"
#include "sys_on_off.h"
#include "otp_play.h"
#include "pt231x.h"
#include "npca110x.h"

#ifdef FUNC_LINEIN_EN

#ifdef FUNC_NPCA110X_EN
extern BOOL isDefaultBass;
#endif

//extern TIMER 	VolumeUpTimer;
//extern BYTE 	VolumeVal;
//extern BOOL 	IsVolumeUpEnd;


//static BYTE LineInVolume = 4;
// Volume table in dB set.
BYTE CODE gLineInGain[VOLUME_MAX + 1] = 
{
//注:Line In音频从E0、E1口输入到芯片内部，请用gFmInGain[VOLUME_MAX + 1]表中参数值
//#if ((LINEIN_IO_TYPE == DAC_CH_E0_L) || (LINEIN_IO_TYPE == DAC_CH_E1_R) || (LINEIN_IO_TYPE == DAC_CH_E0E1))
//	63,		61,		59,		57,		55,		53,		51,		49,
//	47,		45,		43,		41,		39,		37,		35,		33,
//	31,		30,		29,		28,		27,		26,		25,		24,
//	23,		22,		21,		20,		19,		18,		17,		16,
//	15
#if (VOLUME_MAX == 16)
   #if defined(AU6210K_NR_D_9X)||defined(AU6210K_NR_D_9X_XJ_HTS)
	42,		35,		30,		25,		20,		18,		16,		15,
	14,		13,		12,		11,		10,		9,		8,		7,
	6
   #else
    63,		51,		39,		29,		20,		18,		16,		15,
	14,		13,		12,		11,		10,		9,		8,		7,
	6
   #endif	
#else
	#if defined(AU6210K_LK_SJ_CSRBT)
	63,
	62,		60,		57,		54,		52,		50,		48,		45,		
	42,		41,		39,		37,		36,		34,		33,		32,		
	30,		29,		28,		26,		25,		24,		23,		21,		
	19,		17,		15,		13,		11,		9,		8,		7,	
	#else
	63,
	62,		58,		54,		51,		48,		45,		42,		39,
	37,		35,		33,		31,		29,		27,		25,		23,
	21,		19,		17,		16,		15,		14,		13,		12,
	11,		10,		9,		7,		6,		4,		2,		1
	#endif
#endif
};


//set line-in volume.
VOID SetLineInVolume(BYTE Vol)
{
	InDacLineInGain(gLineInGain[Vol], gLineInGain[Vol]);
}


VOID LineInCtrlInit(VOID)
{
//	VolumeVal = 0;
//	IsVolumeUpEnd = FALSE;
//	SetLineInVolume(VolumeVal);
//	TimeOutSet(&VolumeUpTimer, 10);
#ifdef AU6210K_NR_D_8_CSRBT
	NPCA110X_DAC1_Set_Volume_and_Mute(gSys.Volume);
#else
	SetLineInVolume(gSys.Volume);
#endif
	UnMute();
	gSys.MuteFg = FALSE;
	DBG(("<<LineInCtrlInit()\n"));
#ifdef FUNC_BREAK_POINT_EN
	BP_SaveInfo(&gBreakPointInfo.PowerMemory.SystemMode, sizeof(gBreakPointInfo.PowerMemory.SystemMode));
#endif
}


// LineIn IO DeInitialization.
VOID LineinIODeinit(VOID)
{
#if (LINEIN_IO_TYPE == DAC_CH_E0_L)
	ClrGpioRegBit(GPIO_E_OE, 0x01);
	SetGpioRegBit(GPIO_E_IE, 0x01);
	ClrGpioRegBit(GPIO_E_PU, 0x01);
	ClrGpioRegBit(GPIO_E_PD, 0x01);
#elif (LINEIN_IO_TYPE == DAC_CH_E1_R)
	ClrGpioRegBit(GPIO_E_OE, 0x02);
	SetGpioRegBit(GPIO_E_IE, 0x02);
	ClrGpioRegBit(GPIO_E_PU, 0x02);
	ClrGpioRegBit(GPIO_E_PD, 0x02);
#elif (LINEIN_IO_TYPE == DAC_CH_E0E1)
	ClrGpioRegBit(GPIO_E_OE, 0x03);
	SetGpioRegBit(GPIO_E_IE, 0x03);
	ClrGpioRegBit(GPIO_E_PU, 0x03);
	ClrGpioRegBit(GPIO_E_PD, 0x03);
#endif	
}


VOID LineInStateCtrl(VOID)
{
	MESSAGE Event = MessageGet(MSG_FIFO_KEY);
	BYTE minvolflag = 0;
	BYTE maxvolflag = 0;

//	if((!IsVolumeUpEnd) && (VolumeVal < gSys.Volume) && (IsTimeOut(&VolumeUpTimer)))
//	{
//		VolumeVal++;
//		SetLineInVolume(VolumeVal);
//		if(VolumeVal == gSys.Volume)
//		{
//			IsVolumeUpEnd = TRUE;
//		}
//		else
//		{
//			TimeOutSet(&VolumeUpTimer, 10);
//		}	
//	}
 //   DBG(("LineinON\n"));
	switch(Event)
	{
	case MSG_V_ADD:
	case MSG_VOL_ADD:			
#if defined(FUNC_PT231X_EN) && !defined(AU6210K_NR_D_8_CSRBT)
		VolType = VOL_MAIN;
		PT2313VolAdd(); 	
#else
		if(gSys.Volume < VOLUME_MAX)
		{
			maxvolflag = 1;
			VolumeAdjust(UP);
		}
#endif    
#if defined(AU6210K_NR_D_9X_XJ_HTS)|| defined(AU6210K_NR_D_8_CSRBT)||defined(AU6210K_LK_SJ_CSRBT)|| defined(AU6210K_ZB_BT007_CSR)
		if (Event == MSG_V_ADD && gSys.Volume == VOLUME_MAX && !maxvolflag)
		{
			SPI_PlaySelectNum(SPIPLAY_SONG_MAX_VOLUME, 1);			
			
		}
		
#endif
		break;
	case MSG_V_SUB:		
	case MSG_VOL_SUB:			
#if defined(FUNC_PT231X_EN) && !defined(AU6210K_NR_D_8_CSRBT)
		VolType = VOL_MAIN;
		PT2313VolSub(); 	
#else
		if(gSys.Volume > VOLUME_MIN)
		{
			minvolflag = 1;
			VolumeAdjust(DOWN);
		}
#endif
#if defined(AU6210K_NR_D_9X_XJ_HTS) || defined(AU6210K_NR_D_8_CSRBT)||defined(AU6210K_LK_SJ_CSRBT)|| defined(AU6210K_ZB_BT007_CSR)
		if (Event == MSG_V_SUB && gSys.Volume == VOLUME_MIN && !minvolflag)
		{
			SPI_PlaySelectNum(SPIPLAY_SONG_MIN_VOLUME, 1);
			//UnMute();
		}
#endif
		break;	
#ifdef FUNC_PT231X_EN
        case MSG_TREBUP:
			VolType = VOL_TREB;
        	PT2313VolAdd();  	
			break;  	
		case MSG_TREBDN:
			VolType = VOL_TREB;
            PT2313VolSub();  	
			break;  	
		case MSG_BASSUP:
			VolType = VOL_BASS;
            PT2313VolAdd();  	
			break;  	
		case MSG_BASSDN:
			VolType = VOL_BASS;
            PT2313VolSub();  	
			break;  	
		case MSG_DEFAULT:
			PT2313TrebBassSetDefault();
			break;  	
#elif 0//defined(FUNC_NPCA110X_EN)
		case MSG_DEFAULT:
			if(isDefaultBass)
			{
				NPCA110X_SetBass();
				isDefaultBass = FALSE;
			}
			else
			{
				NPCA110X_NormalBass();
				isDefaultBass = TRUE;
			}
			break;
#endif

		case MSG_MUTE:		//mute enable
		case MSG_PLAY_PAUSE:
#ifdef FUNC_BEEP_SOUND_EN	
			if(gSys.MuteFg == FALSE)
			{
				PushKeyBeep(1);
			}
#endif

#if 0//def FUNC_SPI_KEY_SOUND_EN
			if(gSys.MuteFg)
			{
				SPI_PlaySelectNum(SPIPLAY_SONG_UNMUTE, 0);
			}
			else
			{	
				SPI_PlaySelectNum(SPIPLAY_SONG_MUTE, 0);				
			}
#endif		
			MuteStatusChange();
			break;

		default:
			break;
	}

#ifdef FUN_SYSTEM_POWEROFF_WAIT_TIME
	SystemOffTimeMsgPro(Event);
#endif
}

#endif

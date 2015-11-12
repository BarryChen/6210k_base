#include<string.h>
#include <reg51.h>
#include <intrins.h>
#include "type.h"
#include "utility.h"
#include "debug.h"
#include "i2c.h"
#include "gpio.h"
#include "mv_fm.h"
#include "breakpoint.h"
#include "24cxx.h"
#include "sysctrl.h"
#include "dac.h"
#include "user_interface.h"
#include "radio_app_interface.h"
#include "display.h"


#ifdef FUNC_RADIO_EN

BYTE CODE gFmInGain[VOLUME_MAX + 1] = 
{
//FM音频从D3、D4口输出，然后从Line in通道输入到芯片内部
//	63, 	51, 	39, 	29, 	20, 	18, 	16, 	15,
//	14, 	13, 	12, 	11, 	10, 	9,		8,		7,
//	6
#if (VOLUME_MAX == 16)
	63,		55,		47,		39,		35,		31,		27,		24,
	23,		22,		21,		20,		19,		18,		17,		16,
	15
#else
	63,		61,		59,		57,		55,		53,		51,		49,
	47,		45,		43,		41,		39,		37,		35,		33,
	31,		30,		29,		28,		27,		26,		25,		24,
	23,		22,		21,		20,		19,		18,		17,		16,
	15
#endif
};

#ifdef FUNC_RADIO_ESD_AUTO_RESUME
extern WORD gRadioCurrFreqBack;
#endif


// Save the information into external storage device.
// If succeed return TRUE,otherwise return FALSE.
BOOL RadioWriteData(VOID)
{
#ifdef FUNC_BREAK_POINT_EN	
	BP_SaveInfo((BYTE*)&gBreakPointInfo.PowerMemory.RadioData2Store, sizeof(gBreakPointInfo.PowerMemory.RadioData2Store));
	return TRUE;
#else
	return FALSE;
#endif
}


// Radio Module Initilization
// If succeed return TRUE,otherwise return FALSE.
// 只在系统复位(上电)时调用.
VOID RadioInit(VOID)
{
//	DBG((">>RadioInit()\n"));
	
#ifdef FUNC_BREAK_POINT_EN	   
	if((gRadioData.StationSum > MAX_RADIO_STATION_NUM) 
	|| (((gRadioData.CurrStation + 1) > gRadioData.StationSum) && gRadioData.StationSum)
	|| ((gRadioData.Area1StationSum) > gRadioData.StationSum)
#ifdef FUNC_SLAVE_UART_EN
	|| (gRadioData.CurrFreq > gRadioCtrl.RadioUpperBound) || (gRadioData.CurrFreq < gRadioCtrl.RadioLowerBound)
#else
	|| (gRadioData.CurrFreq > RADIO_UPPER_BOUND) || (gRadioData.CurrFreq < RADIO_LOWER_BOUND)
#endif
	)		
#endif		
	{
		DBG(("\nRadio Module Debug Info:\n"));
		DBG(("please check eeprom chip and fm chip\n"));
		DBG(("If they work no problem,maybe the data in eeprom is empty\n"));
		memset(&gRadioData, 0, sizeof(gRadioData));	
		gRadioData.CurrFreq = RADIO_LOWER_BOUND;
	}

	RadioShowDebug(RadioShowPowerOn);
	gRadioCtrl.State = RADIO_INIT;
	gRadioCtrl.SeekMode = SEEK_NONE;
	Radio_Name = RADIO_NONE;
	RadioDisFlag = RadioDisCurFreNum;

	RadioWriteData();
#ifdef RADIO_MVFM_EN
			
	SetGpioRegBit(GPIO_FMD_OE, 0x04);
	ClrGpioRegBit(GPIO_FMD_IE, 0x04);
	SetGpioRegBit(GPIO_FMD_PU, 0x0C);
	ClrGpioRegBit(GPIO_FMD_PD, 0x0C);
	SetGpioRegBit(GPIO_FMD_OUT, 0x04);

	if(MVFM_ReadID())
	{
		DBG(("MVFM!\n"));
		Radio_Name = RADIO_MVFM;						
		return;
	}
#endif
	
//   	DBG(("<<RadioInit()\n"));
}


// Radio Power-On.
VOID RadioPowerOn(VOID)	
{
	switch(Radio_Name)
	{
#ifdef RADIO_BK1080_EN	
		case RADIO_BK1080:
			BK1080PowerOn();	
			break;
#endif

#ifdef RADIO_KT0830_EN
		case RADIO_KT0830E:
			KT0830PowerOn();
			break;
#endif

#ifdef RADIO_RDA5807_EN
		case RADIO_RDA5807:
			RDA5807PowerOn();
			break;
#endif

#ifdef RADIO_QN8035_EN
		case RADIO_QN8035:
			QN8035PowerOn();
			break;
#endif

#ifdef RADIO_MVFM_EN
		case RADIO_MVFM:
			MVFM_PowerOn();
			break;
#endif

		default:
			break;
	}
}

// Radio Power-Down.
VOID RadioPowerDown(VOID)	
{
	switch(Radio_Name)
	{
#ifdef RADIO_BK1080_EN	
		case RADIO_BK1080:
			BK1080PowerDown();	
			break;
#endif

#ifdef RADIO_KT0830_EN
		case RADIO_KT0830E:
			KT0830PowerDown();
			break;
#endif


#ifdef RADIO_RDA5807_EN
		case RADIO_RDA5807:
			RDA5807PowerDown();
			break;
#endif

#ifdef RADIO_QN8035_EN
		case RADIO_QN8035:
			QN8035PowerDown();
			break;
#endif

#ifdef RADIO_MVFM_EN
		case RADIO_MVFM:
			MVFM_PowerDown();
			break;
#endif

		default:
			break;
	}
}


// Radio Set Frequency
// If succeed return TRUE,otherwise return FALSE.
BOOL RadioSetFreq(VOID)
{
	DBG(("Fre:%d\n", gRadioData.CurrFreq));
	
#ifdef FUNC_RADIO_ESD_AUTO_RESUME
	gRadioCurrFreqBack = gRadioData.CurrFreq;
#endif
	
	switch(Radio_Name)
	{
#ifdef RADIO_RDA5807_EN
		case RADIO_RDA5807:
			RDA5807SetFreq(gRadioData.CurrFreq);
			break;
#endif

#ifdef RADIO_BK1080_EN
		case RADIO_BK1080:
			BK1080SetFreq(gRadioData.CurrFreq);
			break;
#endif

#ifdef RADIO_KT0830_EN
		case RADIO_KT0830E:
			KT0830SetFreq(gRadioData.CurrFreq);
			break;
#endif

#ifdef RADIO_QN8035_EN
		case RADIO_QN8035:			
			QN8035SetFreq(gRadioData.CurrFreq);
			break;
#endif		

#ifdef RADIO_MVFM_EN
		case RADIO_MVFM:
			MVFM_SetFreq(gRadioData.CurrFreq);
			break;
#endif

		default:
			return FALSE;
	}

	if(gSys.MuteFg == TRUE)
	{
		UnMute();
		gSys.MuteFg = FALSE;	
		
#ifdef FUNC_DISP_EN	
#ifdef FUNC_SINGLE_LED_EN  
		DispDev();
#else
		DispMute(); 
#endif
#endif
	}
	return TRUE;
}


//set fm-in volume.
VOID RadioSetVolume(BYTE Volume)
{
	InDacLineInGain(gFmInGain[Volume], gFmInGain[Volume]);
	DBG(("FM LR,V:%bd\n", Volume));
}


BOOL RadioMute(BOOL MuteFlag)
{
	switch(Radio_Name)
	{
#ifdef RADIO_RDA5807_EN
		case RADIO_RDA5807:
			RDA5807Mute(MuteFlag);
			break;
#endif

#ifdef RADIO_BK1080_EN
		case RADIO_BK1080:
			BK1080Mute(MuteFlag);
			break;
#endif

#ifdef RADIO_KT0830_EN
		case RADIO_KT0830E:
			KT0830Mute(MuteFlag);
			break;
#endif

#ifdef RADIO_QN8035_EN
		case RADIO_QN8035:
			QN8035Mute(MuteFlag);
			break;
#endif

#ifdef RADIO_MVFM_EN
		case RADIO_MVFM:
			MVFM_Mute(MuteFlag);
			break;
#endif

		default:
			return FALSE;
	}

	return TRUE;
}


// Received Signal Strength Indication
// RSSI_SEEK_THRESHOLD
BYTE RadioRSSIRead(BOOL AutoSeekFlag)
{
	switch(Radio_Name)
	{
#ifdef RADIO_RDA5807_EN
		case RADIO_RDA5807:
			return RDA5807SearchRead(AutoSeekFlag);	
#endif

#ifdef RADIO_BK1080_EN
		case RADIO_BK1080:
			return BK1080SearchRead(AutoSeekFlag);
#endif

#ifdef RADIO_KT0830_EN
		case RADIO_KT0830E:
			return KT0830SearchRead(AutoSeekFlag);
#endif

#ifdef RADIO_QN8035_EN
		case RADIO_QN8035:
			return QN8035SearchRead(AutoSeekFlag);
#endif

#ifdef RADIO_MVFM_EN
		case RADIO_MVFM:
			return MVFM_SearchRead(AutoSeekFlag);
#endif

		default:
			return FALSE;
	}
}


VOID RadioSearchSet(WORD Freq)  
{
	switch(Radio_Name)
	{
#ifdef RADIO_RDA5807_EN
		case RADIO_RDA5807:
			RDA5807SearchSet(Freq);
			break;
#endif

#ifdef RADIO_BK1080_EN
		case RADIO_BK1080:
			BK1080SearchSet(Freq);
			break;
#endif

#ifdef RADIO_KT0830_EN
		case RADIO_KT0830E:
			KT0830SearchSet(Freq);
			break;
#endif

#ifdef RADIO_QN8035_EN
		case RADIO_QN8035:			
			QN8035SearchSet(Freq);
			break;
#endif		
	
#ifdef RADIO_MVFM_EN
		case RADIO_MVFM:
			MVFM_SearchSet(Freq);
			break;
#endif

		default:
			break;
	}
}


//设置FM 的Mono  模式
VOID RadioMonoModeSet(BOOL MonoEnFlag)  
{
	MVFM_SetMonoMode(MonoEnFlag);
}


//读取FM 信号的Stereo 和 Mono 状态
BOOL RadioGetStereoState(VOID)  
{
	return MVFM_GetStereoStatus();
}


#endif

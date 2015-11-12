#include "syscfg.h"
#include "display.h"
#include "radio_app_interface.h"
#include "user_interface.h"
#include "playctrl.h"
#include "breakpoint.h"
#include "debug.h"
#include "power.h"
#include "device.h"
#include "spi_play.h"
#include "touchkey_ctrl.h"
#include "pt231x.h"
#include "bluetooth_ctrl.h"
#include "npca110x.h"



BOOL IsDacMute = FALSE;
BOOL KeyInLedFlashOn = 0;
WORD KeyInLedFlashHoldTmr = 0;
WORD KeyInLedFlashIntTmr = 0;
#ifdef FUNC_BEEP_SOUND_EN 	
BOOL gPushKeyNeedBeepFlag = FALSE;	// 需要产生按键嘀嘀声标志
BOOL gPushKeyBeepStartFlag = FALSE;	//按键嘀嘀声启动标志
TIMER gPushKeyBeepTimer;	//按键嘀嘀声鸣叫一次的时间长度定时
#ifdef FUNC_RADIO_EN
extern BYTE CODE gFmInGain[];
#endif
#ifdef FUNC_LINEIN_EN
extern BYTE CODE gLineInGain[];
#endif
#endif
extern VOID BluetothCtrlEnd(VOID);

extern BOOL GetBtMuteFlag();

#ifdef FUNC_SAVE_PLAYTIME_EN
BOOL gSavePlayTimeEnFlag = FALSE;
#endif

#ifdef	FUNC_DEVICE_FORCEPAUSE_EN
extern BOOL gDevicePlayForcePauseFlag;
#endif

extern VOID PlayVol2Decd(VOID);
extern VOID SetLineInVolume(BYTE Vol);
extern VOID DeviceAudioSetVolume(VOID);
extern VOID SetBluetoothVolume(BYTE Vol);

extern BYTE gRunningDev;
extern BOOL ROM_QuickResponse(BYTE Dev);
BOOL IsSupportSpecialDevice = FALSE;	//支持部分特色设备，MBR 512字节最后两个字节不是0x55AA【默认为FLASE，不支持特殊设备】


// 选择DAC 和外部功放Mute状态
// Param: 
//		DacMuteEn: 1--选择Dac Mute,  0--选择设置音量为0
//		ExMuteEn: 外部功放Mute 使能选择
VOID MuteOn(BOOL DacMuteEn, BOOL ExMuteEn)
{
#ifdef FUNC_EXMUTE_EN
	if(ExMuteEn == TRUE)
	{
		ExMuteOn(); 	
	}
#endif
#if defined(FUNC_PT231X_EN) && !defined(AU6210K_NR_D_8_CSRBT)
      PT2313MuteOnOff(1);
#endif
	if(DacMuteEn == TRUE)
	{	
#ifdef AU6210K_NR_D_8_CSRBT
		NPCA110X_SetOutMute(TRUE);
#else
#endif
		InDacMuteEn();	

		//可以用以下代码解决LineIn Mute状态下漏音问题，但是会带来轻微的POP.
#ifdef FUNC_LINEIN_EN
		if(gSys.SystemMode == SYS_MODE_LINEIN)
		{
			DBG(("InDacChannelSel(DAC_CH_NONE),Mute\n"));
			InDacChannelSel(DAC_CH_NONE);
		}
#endif


#if defined(FUNC_BLUETOOTH_CSR_EN)||defined(AU6210K_NR_D_8_CSRBT)
		if(gSys.SystemMode == SYS_MODE_BLUETOOTH)
		{
			DBG(("btInDacChannelSel(DAC_CH_NONE),Mute\n"));
			InDacChannelSel(DAC_CH_NONE);
		}
#endif


#if defined(AU6210K_JLH_JH82B)
#else
#ifdef FUNC_BLUETOOTH_EN
		if (gSys.SystemMode == SYS_MODE_BLUETOOTH)
		{
			InDacChannelSel(DAC_CH_NONE);

		}
#endif
#endif

	}
	else
	{
#ifdef AU6210K_NR_D_8_CSRBT
		NPCA110X_DAC1_Set_Volume_and_Mute(0);
		NPCA110X_SetOutMute(TRUE);
#else
		InDacSetVolume(0, 0);
#endif
	}	

#ifdef	FUNC_RADIO_EN
	if(gSys.SystemMode == SYS_MODE_RADIO)
	{
		RadioMute(TRUE);
	}
#endif	

	IsDacMute = TRUE;
}


// 解除Mute状态
VOID UnMute(VOID)
{
#ifdef	FUNC_DEVICE_FORCEPAUSE_EN
	if(gDevicePlayForcePauseFlag == TRUE)
	{
		return;
	}
#endif

	DBG1(("unmute.....  \n"));
	if(gSys.Volume == 0)
	{
		return;
	}	

	if(IsDacMute == TRUE)
	{		
#ifdef	FUNC_RADIO_EN
		if(gSys.SystemMode == SYS_MODE_RADIO)
		{
			RadioMute(FALSE);
			RadioSetVolume(gSys.Volume);
		}
#endif	

		//如果MuteOn函数调用InDacChannelSel函数，需要在UnMute函数中重新连通LINEIN通道.
#ifdef FUNC_LINEIN_EN
		if(gSys.SystemMode == SYS_MODE_LINEIN)
		{
			DBG(("InDacChannelSel(DAC_CH_LINEIN),UnMute,gSys.Volume:%bd\n", gSys.Volume));

#ifdef AU6210K_NR_D_8_CSRBT
			NPCA110X_ADC_Input_CH_Select(INPUT_CH0);
			NPCA110X_DAC1_Set_Volume_and_Mute(gSys.Volume);
#else
			InDacChannelSel(LINEIN_IO_TYPE);
			SetLineInVolume(gSys.Volume);
#endif			
		}
#endif	

#ifdef FUNC_BLUETOOTH_CSR_EN
		if(gSys.SystemMode == SYS_MODE_BLUETOOTH)
		{
#ifdef AU6210K_NR_D_8_CSRBT
			NPCA110X_ADC_Input_CH_Select(INPUT_CH1);
			NPCA110X_DAC1_Set_Volume_and_Mute(gSys.Volume);
#else
		
			DBG(("InDacChannelSel(DAC_CH_LINEIN),UnMute,gSys.Volume:%bd\n", gSys.Volume));
			InDacChannelSel(BLUETOOTH_CH_TYPE);
			SetBluetoothVolume(gSys.Volume);
		
#endif			
		}
#endif	

#if defined(AU6210K_JLH_JH82B)
		SetBluetoothVolume(gSys.Volume);
#else
#ifdef FUNC_BLUETOOTH_EN
		if (gSys.SystemMode == SYS_MODE_BLUETOOTH)
		{
			DBG(("bluetooth unmte\n"));
			SET_E0_ANALOG_IN();	//将E0E1配置为模拟端口
			SET_E1_ANALOG_IN();	
			InDacChannelSel(DAC_CH_E0E1); //蓝牙用E0E1通道
			//SetBluetoothVolume(gSys.Volume);
		}
#endif
#endif

	}
#ifdef AU6210K_NR_D_8_CSRBT
	NPCA110X_SetOutMute(FALSE);
#endif
	InDacMuteDis();

#ifdef FUNC_EXMUTE_EN
	if(gSys.Volume > 0)
	{
#ifdef	FUNC_HEADPHONE_DETECT_EN
		if(IsHeadphoneLinkFlag == FALSE)
#endif
		{
			ExMuteOff();
		}	
	}		
#endif
#if defiend(FUNC_PT231X_EN) && !defiend(AU6210K_NR_D_8_CSRBT)
      PT2313MuteOnOff(0);
#endif

	IsDacMute = FALSE;
}


// 改变Mute状态
VOID MuteStatusChange(VOID)
{	
	DBG((">>MuteStatusChange()\n"));
	if(gSys.MuteFg)
	{
		gSys.MuteFg = FALSE;		
		UnMute();	
		DBG1(("4433\n"));
	}
	else
	{	
		gSys.MuteFg = TRUE;
		MuteOn(TRUE, TRUE);       
	}
#ifdef FUNC_DISP_EN
   	DispMute();
#endif	
	DBG(("<<MuteStatusChange()\n"));
}


// 设置系统音量
VOID SetVolume(VOID)
{
	DBG((">>SetVolume() %d\n",(WORD)gSys.Volume));
	
	switch(gSys.SystemMode)
	{
#if (defined(FUNC_USB_EN) || defined(FUNC_CARD_EN))
#ifdef FUNC_USB_EN
		case SYS_MODE_USB:
#endif
#ifdef FUNC_CARD_EN
		case SYS_MODE_SD:
#endif
#ifdef	FUNC_OTP_PLAY_EN
		case SYS_MODE_OTPDEV:
#endif
			PlayVol2Decd();
			break;
#endif

#ifdef FUNC_LINEIN_EN
		case SYS_MODE_LINEIN:
			SetLineInVolume(gSys.Volume);
			break;
#endif

#ifdef FUNC_BLUETOOTH_CSR_EN
		case SYS_MODE_BLUETOOTH:
			SetBluetoothVolume(gSys.Volume);
			break;
#endif

#if 0//def FUNC_BLUETOOTH_EN
		case SYS_MODE_BLUETOOTH:
			SetBluetoothVolume(gSys.Volume);
			break;
#endif


#ifdef FUNC_RADIO_EN
		case SYS_MODE_RADIO:
			RadioSetVolume(gSys.Volume);
			break;
#endif

#ifdef FUNC_AUDIO_EN
		case SYS_MODE_AUDIO:
			DeviceAudioSetVolume();
			break;
#endif

#ifdef FUNC_AUDIO_READER_EN
		case SYS_MODE_AUDIO_READER:
			DeviceAudioSetVolume();
			break;
#endif

		default:
			return;//break;
	}

#ifdef FUNC_BREAK_POINT_EN
	BP_SaveInfo(&gBreakPointInfo.PowerMemory.Volume,sizeof(gBreakPointInfo.PowerMemory.Volume));
#endif

}


// 设置系统音量并同时解除Mute状态
VOID SetVolumeWithMute(VOID)
{
#ifdef AU6210K_NR_D_8_CSRBT
	DBG((">>NPCA110X_DAC1_Set_Volume_and_Mute() %d\n",(WORD)gSys.Volume));
#if (defined(FUNC_USB_EN) || defined(FUNC_CARD_EN))
	if(gSys.SystemMode == SYS_MODE_SD)
		SetVolume();
	else
#endif
	{
		NPCA110X_DAC1_Set_Volume_and_Mute(gSys.Volume);
#ifdef FUNC_BREAK_POINT_EN
		BP_SaveInfo(&gBreakPointInfo.PowerMemory.Volume,sizeof(gBreakPointInfo.PowerMemory.Volume));
#endif
		
	}
	DBG(("gSys.Volume = %x\n",gSys.Volume));	
DBG(("gBreakPointInfo.PowerMemory.Volume = %x\n",gBreakPointInfo.PowerMemory.Volume));	


#else
	SetVolume();
#endif

	if((gSys.Volume == 0) && (IsDacMute == FALSE))
	{
		gSys.MuteFg = FALSE;		
		MuteOn(FALSE, TRUE);		
	}
	if((IsDacMute == TRUE) && (gSys.Volume > 0))
	{
		gSys.MuteFg = TRUE;
	}
	if(gSys.MuteFg == TRUE)
	{
		gSys.MuteFg = FALSE;
		DBG1(("rrrrrr\n"));
		UnMute();	
	}

#ifdef FUNC_DISP_EN
#ifndef FUNC_SINGLE_LED_EN
	DispMute();	  //正确恢复Mute显示状态。避免从Mute状态下设置音量造成的显示错误。
#endif	
#endif
}


// 设置系统音量并同时解除Mute状态、显示音量
VOID SetVolumeWithDisplay(VOID)
{
	SetVolumeWithMute();
#ifdef FUNC_DISP_EN
	DispVolume(TRUE);
#endif
}


//调节系统音量
VOID VolumeAdjust(BYTE Direction)
{	
#ifdef	FUNC_DEVICE_FORCEPAUSE_EN
	if(gDevicePlayForcePauseFlag == TRUE) 
	{
		return;
	}					
#endif
	
	if(Direction == UP)
	{
		DBG(("Vol+\n"));			
		if(gSys.Volume < VOLUME_MAX)
		{
			gSys.Volume++;	
		}	
	}
	else
	{
		DBG(("Vol-\n"));			
		if(gSys.Volume > VOLUME_MIN)
		{
			gSys.Volume--;
		}	
	}
#ifdef AU6210K_ZB_BT007_CSR
		{
			if(gSys.Volume == VOLUME_MAX)	
			{
				SPI_PlaySelectNum(SPIPLAY_SONG_MAX_VOLUME, 1);			
				
				if(gSys.SystemMode == SYS_MODE_BLUETOOTH && !GetBtMuteFlag())
				{
					DBG1(("123\m"));
					UnMute();
				}
			}
	
			if(gSys.Volume == VOLUME_MIN)	
			{
				SPI_PlaySelectNum(SPIPLAY_SONG_MIN_VOLUME, 1);			
				if(gSys.SystemMode == SYS_MODE_BLUETOOTH && !GetBtMuteFlag())
				{
					DBG1(("345\m"));
					UnMute();
				}
			}
		}
#endif	
	SetVolumeWithDisplay();	
			
#ifdef FUNC_BEEP_SOUND_EN	
	if(gSys.Volume < VOLUME_MAX)	
	{
		PushKeyBeep(1);
	}
	else
	{
		PushKeyBeep(2);
	}
#endif	
}


#ifndef FUNC_RADIO_EN
//由于DAC驱动代码中会调用，需要在FUNC_RADIO_EN宏关闭时声明以下函数
VOID RadioCtrlInit(VOID)
{
	//DBG(("RadioCtrlInit,00\n"));
}


VOID RadioCtrlDeinit(VOID)
{
	//DBG(("RadioCtrlDeinit,00\n"));
}


BOOL RadioMute(BOOL MuteFlag)
{
	//DBG(("RadioMute,00\n"));
	return TRUE;
}
#endif


#ifdef FUNC_BEEP_SOUND_EN
//  产生按键嘀嘀声
// Param:
//		ContinueNum: 连续产生按键嘀嘀声次数
VOID PushKeyBeep(BYTE ContinueNum)
{		
	if(gPushKeyNeedBeepFlag == TRUE)		
	{
#ifdef	FUNC_RADIO_EN
		if(gSys.SystemMode == SYS_MODE_RADIO)  
		{
			SysClkDivClr();
		}
#endif
		
#ifdef FUNC_EXMUTE_EN
		if((gSys.Volume > 0) && (IsDacMute == TRUE))
		{		
#ifdef	FUNC_HEADPHONE_DETECT_EN
			if(IsHeadphoneLinkFlag == FALSE)
#endif
			{
				ExMuteOff();
			}
		}		
#endif
		
		while(ContinueNum)
		{		
			ContinueNum--;
			//设置GPE0(FM_L)作为信号输入通道
			baGPIOCtrl[GPIO_E_OE] &= ~0x01;
			baGPIOCtrl[GPIO_E_IE] &= ~0x01;
			baGPIOCtrl[GPIO_E_PU] &= ~0x01;
			baGPIOCtrl[GPIO_E_PD] |= 0x01;		
			DBG(("InDacExFmConnect(DAC_CH_E0_L)\n"));
			InDacExFmConnect(DAC_CH_E0_L);	
#ifdef	FUNC_LINEIN_EN
			if(gSys.SystemMode == SYS_MODE_LINEIN) 
			{
				InDacLineInGain(gLineInGain[gSys.Volume], gLineInGain[gSys.Volume]);					
			}
			else
#endif
			{
#ifdef	FUNC_RADIO_EN
				InDacLineInGain(gFmInGain[gSys.Volume], gFmInGain[gSys.Volume]);
#else
				InDacLineInGain(gLineInGain[gSys.Volume], gLineInGain[gSys.Volume]);	
#endif
			}
			WaitUs(10);
			baGPIOCtrl[GPIO_E_OE] |= 0x01;  //设置E0为输出
			gPushKeyBeepStartFlag = TRUE;
			
			TimeOutSet(&gPushKeyBeepTimer, 20);
			while(!IsTimeOut(&gPushKeyBeepTimer))
			{			
				if((gSys.SystemMode == SYS_MODE_USB) || (gSys.SystemMode == SYS_MODE_SD)) 
				{
					SongPlayDo();
				}	
			}
			gPushKeyBeepStartFlag = FALSE;		
			
			DBG(("InDacExFmDisConnect()\n"));
			InDacExFmDisConnect();		
			baGPIOCtrl[GPIO_E_OE] &= ~0x01;
			baGPIOCtrl[GPIO_E_IE] &= ~0x01;
			baGPIOCtrl[GPIO_E_PU] &= ~0x01;
			baGPIOCtrl[GPIO_E_PD] &= ~0x01;				

			if(ContinueNum)
			{
				TimeOutSet(&gPushKeyBeepTimer, 100);
				while(!IsTimeOut(&gPushKeyBeepTimer))
				{			
					if((gSys.SystemMode == SYS_MODE_USB) || (gSys.SystemMode == SYS_MODE_SD)) 
					{
						SongPlayDo();
					}	
				}		
			}
		}
		
#ifdef FUNC_EXMUTE_EN
		if((gSys.Volume > 0) && (IsDacMute == TRUE))
		{		
#ifdef	FUNC_HEADPHONE_DETECT_EN
			if(IsHeadphoneLinkFlag == FALSE)
#endif
			{
				ExMuteOn();
			}
		}		
#endif
		
#ifdef	FUNC_RADIO_EN
		if(gSys.SystemMode == SYS_MODE_RADIO)  
		{
			SysClkDivSet(CLK_DIV_RATE);
		}
#endif
		gPushKeyNeedBeepFlag = FALSE;
	}
}
#endif


//----------------------------------------------------------
// 如果用户需要【滤空文件夹】功能，要在应用代码上重写FsInfoInit()函数
#ifdef FUNC_FOLDER_EN
#define	MAX_FOLDER_NUM_SAVE 	1024			//save foler 1-2048 empty flag
BYTE gFolderEmpty[MAX_FOLDER_NUM_SAVE / 8];
extern WORD gMaxFoldBitMapNum;
extern BYTE XDATA* gpFoldEmpty;


// 滤空文件夹初始化
VOID FsInfoInit(VOID)
{

	gpFoldEmpty = &gFolderEmpty;
	gMaxFoldBitMapNum = MAX_FOLDER_NUM_SAVE;
	memset(gpFoldEmpty, 0xFF, sizeof(gFolderEmpty));
	DBG(("FsInfoInit,gpFoldEmpty:0x%lx, gMaxFoldBitMapNum:%u\n", (DWORD)gpFoldEmpty, gMaxFoldBitMapNum));	
}
#endif


//----------------------------------------------------------
// 如果用户需要实现【按键快速响应】功能，要在应用代码上重写QuickResponse()函数
#ifdef FUNC_QUICK_RESPONSE_EN
extern BOOL IsSwitchMode;
// 如果需要快速退出返回TRUE.
BOOL QuickResponse(VOID)
{
	// To do...	
	if(MessageGet(MSG_FIFO_DEV_CTRL) == MSG_MODE_SW)
	{
		IsSwitchMode = TRUE;  
		MessageInit();
		return TRUE;
	}

	return FALSE;
}
#endif


//----------------------------------------------------------
// 如果用户需要在1ms系统时基上做处理，要在应用代码上重写TimerTick1ms()函数
extern BOOL gIsNvmOnUse;
VOID TimerTick1ms(VOID)
{
#if	(PWR_KEY_MODE == PWR_KEY_SLIDE_SWITCH)
	extern BOOL gIsPwrkeyPadOn;

	if((gIsPwrkeyPadOn == FALSE) && (gIsNvmOnUse == FALSE))
	{
		//不能执行InDacChannelSel、InDacPowerDown之类操作。
#ifdef FUNC_DISP_EN
		DispDeInit();
#endif	
#ifdef FUNC_EXMUTE_EN		
		ExMuteOn(); 
#endif
		if(gSys.SystemMode == SYS_MODE_BLUETOOTH)
			BluetothCtrlEnd();
		
		DBG1(("22222GoToPDMode\n"));
		//InDacMuteEn();	//关闭是为了 解决关机时间太长问题
		GoToPDMode();		//执行硬开关的系统关闭动作
	}
#endif

#ifdef FUNC_SEG_LED_EN 
#if !defined(FUNC_TIMER1_REFRESH_LED_EN)
	LedFlushDisp();
#endif
#endif

#ifdef FUNC_SEG_LCD_EN 
	LcdFlushDisp();
#endif
#if 0
      	if(KeyInLedFlashHoldTmr)
	{
		KeyInLedFlashHoldTmr--;
		if(KeyInLedFlashIntTmr)
		{
			KeyInLedFlashIntTmr--;				
		}
		else
		{
			KeyInLedFlashIntTmr =100;
			if(KeyInLedFlashOn)
			{
				KeyInLedFlashOn =0;
				baGPIOCtrl[GPIO_B_OE]|=0x80;
				baGPIOCtrl[GPIO_B_OUT]|=0x80;
			}
			else
			{
				KeyInLedFlashOn =1;
				baGPIOCtrl[GPIO_B_OE]|=0x80;
				baGPIOCtrl[GPIO_B_OUT]&=(~0x80);
			}
		}
	}
#endif
	// To do...
}


// 如果客户需要忽略特定VID/PID的USB设备，可以通过本函数实现
BOOL USB_IgnoreDev(VOID)
{
	if(gLibUsbInfo.VID == 0x05AC)	//忽略苹果公司所有设备
	{
		DBG(("Apple device!\n"));
		return TRUE;
	}

	return FALSE;
}


//----------------------------------------------------------
// 如果不支持USB Audio模式，系统不检测PC连接状态，以增强U盘兼容性。
BOOL IsAudDetectDisable(VOID)
{
#if (defined(FUNC_AUDIO_EN) || defined(FUNC_READER_EN) || defined(FUNC_AUDIO_READER_EN))
	return FALSE;
#else
	return TRUE;
#endif
}


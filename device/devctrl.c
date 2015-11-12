#include "syscfg.h"
#include "sysctrl.h"
#include "device_audio.h"
#include "usb_audio_ctrl.h"
#include "card_reader_ctrl.h"
#include "playctrl.h"
#include "device.h"
#include "rtc_ctrl.h"
#include "breakpoint.h"
#include "linein_ctrl.h"
#include "sleep.h"
#include "radio_app_interface.h"
#include "user_interface.h"
#include "debug.h"
#include "sys_on_off.h"
#include "radio_ctrl.h"
#include "display.h"
#include "key.h"
#include "spi_fs.h"
#include "otp_play.h"
#include "touch_key.h"
#include "pt231x.h"
#include "bluetooth_ctrl.h"
#include "slave_ctrl.h"
#include "bt.h"
#include "npca110x.h"

#ifdef FUNC_QUICK_RESPONSE_EN 
BOOL IsSwitchMode;
BYTE CurMode;
#endif
#ifdef FUNC_NPCA110X_EN
extern BOOL isDefaultBass;
#endif

BOOL isDealSetBass = FALSE;
static 	USHORT i = 0;

#ifdef   FUNC_USB_CHARGE_EN
//当前系统中是否连接了Apple设备标志
BOOL     AppleDeviceFlag = FALSE;	
//gMassStorInvalidVid = 0x05AC时，把Ipod设备作为充电设备
WORD	 gMassStorInvalidVid = 0x05AC;
#endif

//#define FUNC_PAUSE_DURING_DEVTRY	//设备尝试切换时停止播放，显示LOD.

extern BOOL gDeviceError;
extern SYS_MODE gSysMode;	// define in library.
extern BOOL IsCardLinkFlag; //for audio
extern BOOL gIsUsbRemove;//for usb Reconnect in mode switch 
extern BOOL IsUsbLinkFlag; 

#define PLAY_WATCH_TIME				10000
extern VOID SongPlayTimePause(VOID);
extern VOID SongPlayTimeResume(VOID);
extern TIMER gPlayWatchTimer;
//extern TIMER VolumeUpTimer;
extern TIMER CardDctTimer;
extern VOID BassLed_CallBak(VOID);
#define SET_BASS_TIME	1	
TIMER SetBassTimer;

//#ifdef FUNC_USB_MSC_SN_EN
//BYTE gUDiskSN[MAX_USB_SN_LEN];
//#endif

#ifdef	FUNC_DEVICE_FORCEPAUSE_EN
BOOL gDevicePlayForcePauseFlag = FALSE;	//强制暂停设备播放状态
#endif

#ifdef	FUNC_HEADPHONE_DETECT_EN
static TIMER sHeadPhoneCheckTimer;
#endif

#ifdef FUNC_POWER_ON_AUDIO_EN
BOOL gUSBChargePowerOnFlag = TRUE;	//关机状态下USB充电器插上系统开机标志
#endif

#ifdef FUNC_STANDBY_EN
SYS_MODE gSystemExecState;
#endif

#ifdef FUNC_SNOOZE_EN
#define SNOOZE_TIME		300000	// 5mins

static TIMER sSnoozeTimer;
static BOOL sSnoozeOnFlag = FALSE;
static BOOL sAlarmPowerOnFlag = FALSE;
#endif

#ifdef FUNC_BEEP_SOUND_EN 	
extern 	TIMER gPushKeyBeepTimer;
#endif


////////////////////////////////////////////////////
///////////////////////////////////////////////////
//强制进入某种模式
//并做原模式退出时的善后处理工作
static VOID GotoMode(BYTE Mode)
{
#ifdef FUNC_TOUCH_KEY_EN
	TouchKeyScanInit();
#endif

	switch(gSys.SystemMode)		//原模式
	{
#ifdef FUNC_USB_EN
		case SYS_MODE_USB:		//离开USB模式
			//停止播放
			PlayCtrlEnd();
			break;
#endif

#ifdef FUNC_CARD_EN
		case SYS_MODE_SD:		//离开SD模式
			//停止播放
			PlayCtrlEnd();
			break;
#endif

#ifdef FUNC_SPI_PLAY_EN
		case SYS_MODE_SPIDEV:		//离开SPI模式
			//停止播放
			PlayCtrlEnd();
			break;
#endif

#ifdef FUNC_RADIO_EN
		case SYS_MODE_RADIO: 		//离开RADIO模式
			if(Mode == SYS_MODE_RADIO)
			{
				break;
			}	
			RadioCtrlDeinit();				
//#if (FM_IO_TYPE == DAC_OUT_LR)
//			DBG(("LeaveRad,DacInit\n"));
//			InDacInit(FALSE);
//#endif
			SysClkDivClr();	
			InDacSetVmid(FALSE);
			break;
#endif

#ifdef FUNC_LINEIN_EN
		case SYS_MODE_LINEIN:	//离开line-in模式
			if(Mode == SYS_MODE_LINEIN)
			{
				break;
			}
			LineinIODeinit();
			//SysClkDivClr();	
			break;
#endif
#ifdef FUNC_BLUETOOTH_CSR_EN
		case SYS_MODE_BLUETOOTH:	
			if(Mode == SYS_MODE_BLUETOOTH)
			{
				break;
			}
			BluetothCtrlEnd();
			//SysClkDivClr();	
			break;
#endif
#if 0//def FUNC_BLUETOOTH_EN
		case SYS_MODE_BLUETOOTH:
			if (Mode == SYS_MODE_BLUETOOTH)
			{
				break;
			}
			BluetothCtrlEnd();
			//SysClkDivClr();
			break;
#endif

		case SYS_MODE_SLEEP:	//离开SLEEP模式
			
			break;

#ifdef FUNC_RTC_EN
		case SYS_MODE_RTC:		//离开RTC模式

#ifdef FUNC_POWER_AMPLIFIER_EN
			if (Mode == SYS_MODE_RTC)
			{
				break;
			}
			if (Mode != SYS_MODE_RADIO)
			{
				ABPowerAmplifierOn();
			}
#endif	

#ifdef FUNC_USB_VOLUP_EN
			if (Mode == SYS_MODE_RTC)
			{
				break;
			}
			if (Mode != SYS_MODE_USB)
			{
				USB_VOLUP_Off();
			}
#endif		
			break;
#endif
			
#if (defined(FUNC_AUDIO_EN) || defined(FUNC_READER_EN) || defined(FUNC_AUDIO_READER_EN))
		case SYS_MODE_AUDIO:	//离开USB声卡模式
		case SYS_MODE_READER:		//离开USB读卡器模式
		case SYS_MODE_AUDIO_READER:	//离开USB一线通模式
			DeviceAudioCtrlEnd();
			DeviceStorCtrlEnd();
			break;
#endif

		default:
			break;
	}
	
#ifdef FUNC_STANDBY_EN
	if(Mode != SYS_MODE_STANDBY)
	{
		gSystemExecState = Mode;
	}
#endif

	gSys.SystemMode = Mode;
	gSysMode = Mode;		//定义在底层，为了避免底层直接引用gSys.SystemMode增加(防止应用层代码修改gSys结构体影响到底层)。
	MuteOn(TRUE, TRUE);	//关闭DAC、LineIn、FMIn通道输出。
	gSys.MuteFg = FALSE;
	
	MessageInit();

#ifdef FUN_SYSTEM_POWEROFF_WAIT_TIME
	TimeOutSet(&gSysTimeOffState.DisTimer, 0);	
#endif

#ifdef FUNC_DISP_EN
	DispMute();	//正确恢复Mute显示状态。避免从Mute状态下切换设备造成的显示错误。
	DispDev();
#endif
}  


#ifdef FUNC_STANDBY_EN
//尝试切换到Standby模式
static BOOL TryGotoModeStandby(VOID)
{	
	DBG(("TryGotoModeStandby()\n"));

	ModuleClkEn(MASK_ALL_CLOCK);
	SysClkDivClr();	
	
	GotoMode(SYS_MODE_STANDBY);	
	
	InDacChannelSel(DAC_CH_NONE);	
	InDacPowerDown(FALSE);	

#ifdef FUNC_POWER_AMPLIFIER_EN
	ABPowerAmplifierOff();
#endif

#ifdef FUNC_SNOOZE_EN
	sAlarmPowerOnFlag = FALSE;
#endif

	return TRUE;
}
#endif


//尝试切换到IDLE模式
static BOOL TryGotoModeIdle(VOID)
{
	DBG(("TryGotoModeIdle()\n"));
	GotoMode(SYS_MODE_NONE);

	InDacChannelSel(DAC_CH_NONE);

#ifdef FUNC_POWER_AMPLIFIER_EN
	ABPowerAmplifierOff();
#endif

	return TRUE;
}


#ifdef FUNC_CARD_EN
//尝试切换到SD卡模式
static BOOL TryGotoModeSD(VOID)
{
	BOOL TempFlag = FALSE;
		
	DBG1(("TryGotoModeSD()\n"));
	
#ifdef FUNC_POWER_ON_AUDIO_EN
	if(gUSBChargePowerOnFlag == TRUE)
	{
		return FALSE;
	}
#endif
	
	if(IsCardLinkFlag == TRUE)
	{
		MuteOn(TRUE, TRUE); //解决SD或USB之间直接切换会停一下再出之前设备歌曲声音问题
		TempFlag = TRUE;
	}

	//等待SD卡或U盘设备连接
	if(!IsCardLink())
	{
		
		DBG1(("IsCardLink() FALSE!\n"));
		IsCardLinkFlag = FALSE;
		return FALSE;
	}
	DBG1(("IsCardLink() OK!\n"));

	

   	ModuleClkEn(MASK_ALL_CLOCK);
	SysClkDivClr();	

	gPlayCtrl.State = PLAY_STATE_STOP;
	
#ifdef FUNC_DISP_EN
	DispLoad();
#endif
	if(TempFlag == FALSE)
	{
		MuteOn(TRUE, TRUE); //解决SD或USB之间直接切换会停一下再出之前设备歌曲声音问题
	}

	//WaitMs(50);

//	if(!CardInit())	
	{
		//WaitMs(50);
		if(!CardInit())	
		{
			DBG1(("CardInit() error!\n"));
			return FALSE;
		}
		
	}
	DBG1(("Card OK!\n"));

	GotoMode(SYS_MODE_SD);
#ifdef AU6210K_NR_D_8_CSRBT
	NPCA110X_ADC_Input_CH_Select(INPUT_CH2);
#endif
	InDacChannelSel(DAC_CH_DAC);
	
#ifdef FUNC_SPI_KEY_SOUND_EN	
	SPI_PlaySelectNum(SPIPLAY_SONG_PLAY_TF, 0);
#endif
	
	DBG1(("FSInit(DEV_ID_SD)\n"));
	if(!FSInit(DEV_ID_SD))
	{
		DBG1(("FSInit() error!\n"));
		return FALSE;
	}

	DBG1(("*******SD MODE********\n"));
	PlayCtrlInit();	 
#ifdef FUNC_TOUCH_KEY_EN
	TR1 = 1;
#endif
	return TRUE;
}
#endif


#ifdef FUNC_USB_EN
//尝试切换到USB模式
static BOOL TryGotoModeUSB(VOID)
{	
	BOOL TempFlag = FALSE;
	
	DBG(("TryGotoModeUSB()\n"));
	
#ifdef FUNC_POWER_ON_AUDIO_EN
	if(gUSBChargePowerOnFlag == TRUE)
	{
		return FALSE;
	}
#endif	
	
	if(IsUsbLinkFlag == TRUE)
	{
		MuteOn(TRUE, TRUE); //解决SD或USB之间直接切换会停一下再出之前设备歌曲声音问题
		TempFlag = TRUE;
	}
	
	//U盘电气连接检查
	if(!IsUSBLink())
	{
		DBG(("IsUSBLink() FALSE!\n"));
		return FALSE;
	}
	DBG(("IsUSBLink() OK!\n"));

	if(TempFlag == FALSE)
	{
		MuteOn(TRUE, TRUE); //解决SD或USB之间直接切换会停一下再出之前设备歌曲声音问题
	}	
	
	ModuleClkEn(MASK_ALL_CLOCK);	
	SysClkDivClr();	

#ifdef	FUNC_USB_CHARGE_EN
	//防止TryGotoModeUSB()函数被多次调用时，导致AppleDevice多次提示充电
	if(AppleDeviceFlag)
	{
		return FALSE;
	}
#endif

#ifdef FUNC_DISP_EN
	DispLoad();
#endif

//	DBG(("IsUSBLink() OK!\n"));
	if(!HostStorInit())	
	{
		DBG(("HostStorInit() error!\n"));
#ifdef	FUNC_USB_CHARGE_EN
		//if(IsInvalidUsbDevice())
		if(gLibUsbInfo.VID == 0x05AC)
		{
			extern BYTE UsbSetConfiguration(BYTE configuration);
			AppleDeviceFlag = TRUE;							
			UsbSetConfiguration(1);
			DBG(("USB充电开始!\n"));
		}
#endif
		return FALSE;
	}
	DBG(("USB OK!\n"));

	GotoMode(SYS_MODE_USB);
	InDacChannelSel(DAC_CH_DAC);
	
#if 0//def FUNC_SPI_KEY_SOUND_EN	
	SPI_PlaySelectNum(SPIPLAY_SONG_HAVE_CALL, 0);
#endif

	DBG(("FSInit(DEV_ID_USB)\n"));
	if(!FSInit(DEV_ID_USB))
	{
		DBG(("FSInit() error!\n"));
		return FALSE;
	}

	DBG(("*******USB MODE********\n"));
	PlayCtrlInit();
	
#ifdef FUNC_TOUCH_KEY_EN
	TR1 = 1;
#endif
	return TRUE;
}
#endif


#ifdef FUNC_RADIO_EN
//尝试切换到RADIO模式
static BOOL TryGotoModeRADIO(VOID)
{		
	DBG(("TryGotoModeRADIO()\n"));

#ifdef FUNC_POWER_ON_AUDIO_EN
	if(gUSBChargePowerOnFlag == TRUE)
	{
		return FALSE;
	}
#endif

	//U盘电气连接检查  
	if(!IsRadioLink())
	{
		DBG(("IsRadioLink() FALSE!\n"));
		return FALSE;
	}
	DBG(("IsRadioLink() OK!\n"));
	
	GotoMode(SYS_MODE_RADIO);
	
#if 0//def FUNC_SPI_KEY_SOUND_EN	
	SPI_PlaySelectNum(SPIPLAY_SONG_NEXT, 0);
#endif
 
	RadioPowerOn();
	InDacSetVmid(TRUE);

	//FM set
#ifdef FM_DAC_OUT_D3D4
	//如果要让FM音频从D3、D4口输出，则需要做如下配置	
	SetGpioRegBit(GPIO_FMD_OE, 0x04);
	ClrGpioRegBit(GPIO_FMD_IE, 0x04);
	SetGpioRegBit(GPIO_FMD_OUT, 0x04);

	ClrGpioRegBit(GPIO_D_PU, 0x18);
	SetGpioRegBit(GPIO_D_PD, 0x18);
	SetGpioRegBit(GPIO_FMA_EN, 0x03);	//片内FM模块的音频信号连接到D3、D4

	//再选择音频信号输入到芯片内部，然后从 DAC输出方式
	//从Line in通道输入到芯片内部
	InDacChannelSel(DAC_CH_LINEIN); 

	//从E0 E1输入到芯片内部
	//SET_E0_ANALOG_IN();	//将E0E1配置为模拟端口
	//SET_E1_ANALOG_IN();		
	//InDacChannelSel(DAC_CH_E0E1);		

	RadioCtrlInit();

#else
	//默认选择片内FM音频通路
	InDacChannelSel(DAC_CH_ONCHIP_FM);
#endif

//FM 	
#ifdef FUNC_PWM_EN
	ModuleClkDis(MASK_ALL_CLOCK & (~MSK_PWM_CLOCK));
#else
	ModuleClkDis(MASK_ALL_CLOCK);
#endif
// 切换到FM后，时钟分频函数
// O20和以前芯片不一样，有两个时钟源，默认是配置为RC clk。
#ifndef	FUNC_SLAVE_UART_EN
	SysClkDivSet(CLK_DIV_RATE);
#endif
//#if (FM_IO_TYPE != FM_ONCHIP_IP)	
//	RadioCtrlInit();
//#endif

	RadioEnterInit();
#ifdef FUNC_TOUCH_KEY_EN
	TR1 = 1;
#endif
	return TRUE;
}
#endif


#ifdef FUNC_LINEIN_EN	  
//尝试切换到line-in模式
static BOOL TryGotoModeLINEIN(VOID)
{
	DBG(("TryGotoModeLINEIN()\n"));
	
#ifdef FUNC_POWER_ON_AUDIO_EN
	if(gUSBChargePowerOnFlag == TRUE)
	{
		return FALSE;
	}
#endif	

	//U盘电气连接检查
	if(!IsLineInLink())
	{
		DBG(("IsLineInLink() FALSE!\n"));
		return FALSE;
	}
   	ModuleClkEn(MASK_ALL_CLOCK);
	SysClkDivClr();	
	DBG(("IsLineInLink() OK!\n"));

	GotoMode(SYS_MODE_LINEIN);

#ifdef FUNC_SPI_KEY_SOUND_EN	
	SPI_PlaySelectNum(SPIPLAY_SONG_PLAY_AUX, 0);
#endif	
#ifdef AU6210K_NR_D_8_CSRBT
	NPCA110X_ADC_Input_CH_Select(INPUT_CH0);
	InDacChannelSel(LINEIN_IO_TYPE);
#else


	//默认LINE_IN模式的音源选择片外LINE_IN通路
	//LINE_IN模式的音源也可以选择E0、E1通路, 则需要做如下配置	
/*#if (LINEIN_IO_TYPE == DAC_CH_E0_L)
	SET_E0_ANALOG_IN();	//将E0配置为模拟端口
#elif (LINEIN_IO_TYPE == DAC_CH_E1_R)
	SET_E1_ANALOG_IN();	//将E1配置为模拟端口
#elif (LINEIN_IO_TYPE == DAC_CH_E0E1)
*/
#if (LINEIN_IO_TYPE == DAC_CH_E0E1)
	SET_E0_ANALOG_IN();	//将E0E1配置为模拟端口
	SET_E1_ANALOG_IN();	
#endif

	InDacChannelSel(LINEIN_IO_TYPE);


#endif	
	LineInCtrlInit();

#ifdef FUNC_TOUCH_KEY_EN
	TR1 = 1;
#endif
	return TRUE;
}
#endif

#ifdef FUNC_BLUETOOTH_CSR_EN	  
//尝试切换到line-in模式
static BOOL TryGotoBluetooth(VOID)
{
	DBG(("TryGotoModeLINEIN()\n"));
	
#ifdef FUNC_POWER_ON_AUDIO_EN
	if(gUSBChargePowerOnFlag == TRUE)
	{
		return FALSE;
	}
#endif	

   	ModuleClkEn(MASK_ALL_CLOCK);
	SysClkDivClr();	
	DBG1(("IsLineInLink() OK!\n"));

	GotoMode(SYS_MODE_BLUETOOTH);

#ifdef FUNC_SPI_KEY_SOUND_EN	
	SPI_PlaySelectNum(SPIPLAY_SONG_PLAY_BT, 0);
#endif	
#ifndef FUNC_PT231X_EN
	//默认LINE_IN模式的音源选择片外LINE_IN通路
	//LINE_IN模式的音源也可以选择E0、E1通路, 则需要做如下配置	
/*#if (BLUETOOTH_CJ_IO_TYPE == DAC_CH_E0_L)
	SET_E0_ANALOG_IN();	//将E0配置为模拟端口
#elif (BLUETOOTH_CJ_IO_TYPE == DAC_CH_E1_R)
	SET_E1_ANALOG_IN();	//将E1配置为模拟端口
#elif (BLUETOOTH_CJ_IO_TYPE == DAC_CH_E0E1)
	SET_E0_ANALOG_IN();	//将E0E1配置为模拟端口
	SET_E1_ANALOG_IN();	
#endif*/

	//InDacChannelSel(BLUETOOTH_CJ_IO_TYPE);
#endif	
	BluetoothCtrlInit();

#ifdef FUNC_TOUCH_KEY_EN
	TR1 = 1;
#endif
	return TRUE;
}
#endif
#if 0//def FUNC_BLUETOOTH_EN
static BOOL TryGotoBluetooth()
{
	DBG(("TryGotoModeBLUETOOTH()\n"));
#ifdef BLUETOOTH_ALWAYS_OPEN
	if (!gBTConnected)//蓝牙未连接上时，不进入蓝牙状态
	{
		DBG(("Bluetooth disconnected, don't entry bluetooth mode.\n"));
		return FALSE;
	}
#endif
	
	MuteOn(TRUE, TRUE);//先禁掉声音，去POPO声，BluetoothCtrlInit()中会再次启动声音
	ModuleClkEn(MASK_ALL_CLOCK);
	SysClkDivClr();//重置系统状态
	GotoMode(SYS_MODE_BLUETOOTH);	//进入BLUETOOTH模式

#ifdef RTC_SHOWTIME_ANYMODE
	DispTimeOnOff(FALSE);
#endif

#if defined(AU6210K_JLH_JH82B)
	InDacChannelSel(DAC_CH_NONE);
#endif

#if defined(AU6210K_NR_D_8_CSRBT)
	InDacChannelSel(BLUETOOTH_CJ_IO_TYPE);
#endif

	return BluetoothCtrlInit();
}
#endif


////尝试切换到RTC模式
//static BOOL TryGotoModeRTC(VOID)
//{
//	DBG(("TryGotoModeRTC()\n"));
//	GotoMode(SYS_MODE_RTC);
//
//	InDacChannelSel(DAC_CH_NONE);
//#ifdef FUNC_POWER_AMPLIFIER_EN
	//ABPowerAmplifierOff();
//#endif
//	return TRUE;
//}


#ifdef FUNC_AUDIO_EN
//尝试切换到AUDIO模式
static BOOL TryGotoModeAUDIO(VOID)
{
#ifdef FUNC_POWER_ON_AUDIO_EN	
	static TIMER Timer;
#endif

	DBG(("TryGotoModeAUDIO()\n"));	
	
#ifdef FUNC_POWER_ON_AUDIO_EN
	//连接PC开机直接进入Audio 模式
	if(gUSBChargePowerOnFlag == TRUE)
	{
		TimeOutSet(&Timer, 1500);	//如果客户希望连接PC开机需要直接进入Audio模式,此时间必须大于1.4s
		while(!IsTimeOut(&Timer))
		{
			if(IsPcLink())
			{
				break;
			}
		}			
	}
	gUSBChargePowerOnFlag = FALSE;
#endif
	
	//PC连接检查
	IsPcInsert();	//为了PC连接开机情况下检查PC 状态
	
	if(!IsPcLink())
	{
		DBG(("Audio,IsPcLink() FALSE!\n"));
		return FALSE;
	}

#ifdef FUNC_DISP_EN	
	DispAudio();
#endif	

	MuteOn(TRUE, TRUE);	//解决连接 PC过程中会再出之前设备歌曲声音问题

	ModuleClkEn(MASK_ALL_CLOCK);		
	SysClkDivClr();

	GotoMode(SYS_MODE_AUDIO);
	InDacChannelSel(DAC_CH_DAC);
	
	DeviceAudioSetMode(USB_DEVICE_AUDIO);
	DeviceAudioCtrlInit();
#ifdef FUNC_TOUCH_KEY_EN
	TR1 = 1;
#endif

//	DBG(("Usb audio mode ok!\n"));
	return TRUE;
}
#endif


#ifdef FUNC_READER_EN
//尝试切换到READER模式
static BOOL TryGotoModeREADER(VOID)
{	
	DBG(("TryGotoModeREADER()\n"));
	//PC连接检查
	if(!IsPcLink())
	{
		DBG(("Reader,IsPcLink() FALSE!\n"));
		return FALSE;
	}

	MuteOn(TRUE, TRUE);	//解决连接 PC过程中会再出之前设备歌曲声音问题

#ifdef FUNC_DISP_EN
	DispLoad();
#endif

	ModuleClkEn(MASK_ALL_CLOCK);		
	SysClkDivClr();

	GotoMode(SYS_MODE_READER);
	InDacChannelSel(DAC_CH_DAC);

	DeviceAudioSetMode(USB_DEVICE_READER);
#ifdef FUNC_CARD_EN 
	DeviceStorCtrlInit();
#endif
#ifdef FUNC_TOUCH_KEY_EN
	TR1 = 1;
#endif
//	DBG(("Usb reader mode ok!\n"));
	return TRUE;
}
#endif


#ifdef FUNC_AUDIO_READER_EN
//尝试切换到AUDIO_READER模式
static BOOL TryGotoModeAUDIO_READER(VOID)
{	
#ifdef FUNC_POWER_ON_AUDIO_EN	
	static TIMER Timer;
#endif

	DBG(("TryGotoModeAUDIO_READER()\n"));	
	
#ifdef FUNC_POWER_ON_AUDIO_EN
	//连接PC开机直接进入Audio 模式
	if(gUSBChargePowerOnFlag == TRUE)
	{
		TimeOutSet(&Timer, 1500);	//如果客户希望连接PC开机需要直接进入Audio模式,此时间必须大于1.4s
		while(!IsTimeOut(&Timer))
		{
			if(IsPcLink())
			{
				break;
			}
		}			
	}
	gUSBChargePowerOnFlag = FALSE;
#endif

	//PC连接检查
	IsPcInsert();	//为了PC连接开机情况下检查PC 状态

	if(!IsPcLink())
	{
		DBG(("Audio,IsPcLink() FALSE!\n"));
		return FALSE;
	}	

#ifdef FUNC_DISP_EN	
	DispAudio();
#endif	

	MuteOn(TRUE, TRUE);	//解决连接 PC过程中会再出之前设备歌曲声音问题
	
	ModuleClkEn(MASK_ALL_CLOCK);		
	SysClkDivClr();

	GotoMode(SYS_MODE_AUDIO_READER);
	InDacChannelSel(DAC_CH_DAC);
	
	DeviceAudioSetMode(USB_DEVICE_AUDIO_READER);
	DeviceAudioCtrlInit();
#ifdef FUNC_CARD_EN 
	DeviceStorCtrlInit();
#endif
#ifdef FUNC_TOUCH_KEY_EN
	TR1 = 1;
#endif
//	DBG(("Usb audio reader mode ok!\n"));
	return TRUE;
}
#endif


#ifdef	FUNC_SPI_UPDATE_EN
extern BYTE gIsMVFFileFind;
#endif
#ifdef FUNC_SPI_PLAY_EN
//尝试切换到spi-dev模式
static BOOL TryGotoModeSpiDev()
{
	DBG(("->TryGotoSPIDev!\n"));

	if(!IsSpiFlashHandleGot())
	{
		DBG(("No SPI Dev!\n"));
		return FALSE;
	}
#ifdef FUNC_DISP_EN
	DispLoad();
#endif

	ModuleClkEn(MASK_ALL_CLOCK);		
	SysClkDivClr();

	GotoMode(SYS_MODE_SPIDEV);
	InDacChannelSel(DAC_CH_DAC);

#ifdef	FUNC_SPI_UPDATE_EN
	gIsMVFFileFind = 0;
#endif

	FSInit(DEV_ID_SPI);
	PlayCtrlInit();
	gSongInfo.SongType = SONG_TYPE_MP3; 
	gPlayCtrl.File.FileType = FILE_TYPE_MP3;
	gPlayCtrl.FileNum = 1;
	DBG(("<-TryGotoSPIDev!\n"));
#ifdef FUNC_TOUCH_KEY_EN
	TR1 = 1;
#endif	
	return TRUE;
}
#endif


#ifdef FUNC_OTP_PLAY_EN
static BOOL TryGotoModeOtpDev(VOID)
{
	DBG((">>TryGotoModeOtpDev()\n"));
#ifdef FUNC_DISP_EN
	DispLoad();
#endif

	ModuleClkEn(MASK_ALL_CLOCK);		
	SysClkDivClr();
		
	GotoMode(SYS_MODE_OTPDEV);
	InDacChannelSel(DAC_CH_DAC);

	FSInit(DEV_ID_OTP);
	PlayCtrlInit();
	gSongInfo.SongType = SONG_TYPE_MP3; 
	gPlayCtrl.File.FileType=FILE_TYPE_MP3;
	gPlayCtrl.FileNum = 1;
#ifdef FUNC_TOUCH_KEY_EN
	TR1 = 1;
#endif
	return TRUE;
}


//播放选择的OTP歌曲
static VOID OTP_PlaySelectNum(BYTE PlayNum)
{	
	BYTE SystemMode, Volume;

	SystemMode = gSys.SystemMode;	
	Volume = gSys.Volume;	
	
	DBG(("<-OTPSongPlayStart()\n"));	
	PlayNum = 1;	
	gSys.Volume = VOLUME_MAX;
#ifdef AU6210K_NR_D_8_CSRBT
	NPCA110X_DAC1_Set_Volume_and_Mute(gSys.Volume);
	InDacSetVolume(gVolArr[32], gVolArr[32]);
#else
	InDacSetVolume(gVolArr[gSys.Volume], gVolArr[gSys.Volume]);
#endif	
	TryGotoModeOtpDev();
	UnMute();	
	while(1)
	{    
		FeedWatchDog();	
		if(FileEOF(&gPlayCtrl.File) && IsFifoEmpty())
		{
			DBG(("Song play end!\n"));
			//等待PCM数据全部被取走
			break;
		}
		if(!SongPlayDo())
		{
			DBG(("PlayStatePlayOp,!SongPlayDo()\n"));			
			break;
		}    	
	}
	DBG(("<-OTPSongPlayStop()\n"));
	PlayCtrlEnd();	
	InDacChannelSel(DAC_CH_NONE);
	gSys.SystemMode = SystemMode;
	gSys.Volume = Volume;
}
#endif


//系统上电开机音乐声
VOID SystemPowerOnSound(VOID)
{	
#ifdef	FUNC_OTP_KEY_SOUND_EN
	OTP_PlaySelNum(OTPPLAY_NUM_POWERON, 0);
#endif

#ifdef FUNC_SPI_FLASH_POWERON_SOUND
	SPI_PlaySelectNum(SPIPLAY_SONG_POWER_ON_SOUND1, 0);
#endif
}


//尝试切换到指定设备
static BOOL TryGotoMode(BYTE Mode)
{	
#ifdef	FUNC_QUICK_RESPONSE_EN
	//如果是强行切换模式
	if(IsSwitchMode)
	{
		DBG(("300\n"));
		return TRUE;
	}

	CurMode = Mode;
#endif

	switch(Mode)
	{
#ifdef FUNC_SLAVE_UART_EN
		case SYS_MODE_NONE:
			return TryGotoModeIdle();
#endif		
	
#ifdef FUNC_USB_EN
		case SYS_MODE_USB:
			return TryGotoModeUSB();
#endif

#ifdef FUNC_CARD_EN
		case SYS_MODE_SD:
			return TryGotoModeSD();
#endif

#ifdef FUNC_LINEIN_EN
		case SYS_MODE_LINEIN:
			return TryGotoModeLINEIN();
#endif
#ifdef FUNC_BLUETOOTH_CSR_EN
		case SYS_MODE_BLUETOOTH:
			return TryGotoBluetooth();
#endif
#ifdef FUNC_BLUETOOTH_EN
		case SYS_MODE_BLUETOOTH:
			return TryGotoBluetooth();
#endif


#ifdef FUNC_RADIO_EN
		case SYS_MODE_RADIO:
			DBG(("TryGotoModeRADIO\n"));
			return TryGotoModeRADIO();
#endif

#ifdef FUNC_AUDIO_EN
		case SYS_MODE_AUDIO:
			DBG(("TryGotoModeAUDIO\n"));
			return TryGotoModeAUDIO();
#endif

#ifdef FUNC_READER_EN
		case SYS_MODE_READER:
			DBG(("TryGotoModeREADER\n"));
			return TryGotoModeREADER();
#endif

#ifdef FUNC_AUDIO_READER_EN
		case SYS_MODE_AUDIO_READER:
			DBG(("TryGotoModeAUDIO_READER\n"));
			return TryGotoModeAUDIO_READER();
#endif

#ifdef FUNC_SPI_PLAY_EN
		case SYS_MODE_SPIDEV:
			DBG(("TryGotoModeOtpDevSPI\n"));
			return TryGotoModeSpiDev();
			break;
#endif

#ifdef FUNC_OTP_PLAY_EN
		case SYS_MODE_OTPDEV:
			DBG(("TryGotoModeOtpDev001\n"));
			return TryGotoModeOtpDev();
			break;
#endif

#ifdef FUNC_STANDBY_EN
		case SYS_MODE_STANDBY:
			return TryGotoModeStandby();
			break;
#endif

		default:
#ifdef FUNC_USB_EN
			return TryGotoModeUSB();
#elif defined(FUNC_CARD_EN)
			return TryGotoModeSD();
#else
			return FALSE;
#endif
	}
}


//尝试切换到Mode模式，如果不成功，则进入下一个模式。
static VOID TryGotoNextMode(BYTE Mode)
{
	BYTE i;
	BYTE ModeIndex;

	//这个数组决定设备切换时的顺序，该顺序是循环的
	static BYTE ModeOrder[] = 
	{
#ifdef FUNC_BLUETOOTH_EN
		SYS_MODE_BLUETOOTH, 
#endif

#ifdef FUNC_BLUETOOTH_CSR_EN
		SYS_MODE_BLUETOOTH, 
#endif

#ifdef FUNC_USB_EN
		SYS_MODE_USB,
#endif	

#ifdef FUNC_CARD_EN
		SYS_MODE_SD,
#endif

#ifdef FUNC_RADIO_EN
		SYS_MODE_RADIO,
#endif	

#ifdef FUNC_LINEIN_EN
		SYS_MODE_LINEIN,
#endif	
 
#ifdef FUNC_AUDIO_EN
		SYS_MODE_AUDIO,
#endif

#ifdef FUNC_READER_EN
		SYS_MODE_READER,
#endif

#ifdef FUNC_AUDIO_READER_EN
		SYS_MODE_AUDIO_READER,
#endif

#ifdef FUNC_SPI_PLAY_EN
		SYS_MODE_SPIDEV,
#endif

#ifdef FUNC_OTP_PLAY_EN
		SYS_MODE_OTPDEV,
#endif
	};

	for(ModeIndex = 0; ModeIndex < sizeof(ModeOrder); ModeIndex++)
	{
		if(Mode == ModeOrder[ModeIndex])
		{
		 	break;
		}
	}

	for(i = 0; i < sizeof(ModeOrder); i++)
	{
		FeedWatchDog();
		ModeIndex++;
		if(ModeIndex >= sizeof(ModeOrder))
		{
			ModeIndex = 0;
		} 		
		
		if(TryGotoMode(ModeOrder[ModeIndex]))
		{
			return;
		}
	}

	TryGotoModeIdle();
}


//尝试切换到Mode模式，如果不成功，则进入下一个模式。
static VOID TryGotoValidMode(BYTE Mode)
{
	if(!TryGotoMode(Mode))
	{
		TryGotoNextMode(Mode);
	}
}


//尝试切换到Mode模式，如果不成功，则进入原来的模式。
static VOID TryGotoValidModeReturn(BYTE Mode)
{
	BYTE OldMode = gSys.SystemMode;

#ifdef	FUNC_DEVICE_FORCEPAUSE_EN
	gDevicePlayForcePauseFlag = FALSE;
#endif

	if(!TryGotoMode(Mode))
	{
		if(!TryGotoMode(OldMode))
		{
			TryGotoNextMode(SYS_MODE_USB);
		}
	}
}


VOID DevCtrlInit(VOID)
{
	static TIMER Timer;

//	TimeOutSet(&VolumeUpTimer, 0);
	TimeOutSet(&gPlayWatchTimer, 0);
	
#ifdef	FUNC_HEADPHONE_DETECT_EN
	TimeOutSet(&sHeadPhoneCheckTimer, 0);
#endif
	
#ifdef FUNC_BEEP_SOUND_EN
	TimeOutSet(&gPushKeyBeepTimer, 0);
#endif	

#ifdef FUNC_SNOOZE_EN
	TimeOutSet(&sSnoozeTimer, 0);
#endif	

#ifdef FUNC_LED_LINEIN_MULTIPLE_EN
	TimeOutSet(&gLineInCheckWaitTimer, 0);
#endif

#ifndef FUNC_BREAK_POINT_EN
	gSys.SystemMode = SYS_MODE_NONE;
	gSys.Volume = VOLUME_INIT;
	gPlayCtrl.Eq = DECD_EQ_NORMAL;
	gPlayCtrl.RepeatMode = REPEAT_ALL;
#ifdef FUNC_FOLDER_EN
	gPlayCtrl.FolderEnable = TRUE;
#else
	gPlayCtrl.FolderEnable = FALSE;
#endif
#endif

#ifdef FUNC_QUICK_RESPONSE_EN
	IsSwitchMode = FALSE;
#endif

#ifdef FUNC_USB_EN
	DBG(("InitUsb()\n"));
	HostInit();
#endif

#ifdef FUNC_CARD_EN
#ifdef CARD_BUS_A3A4A5_EN
	SET_CARD_TO_A3A4A5();
#endif
#ifdef	CARD_BUS_A0E2E3_EN
	SET_CARD_TO_A0E2E3();
#endif
#ifdef	CARD_BUS_D5D6D7_EN
	SET_CARD_TO_D5D6D7();
#endif
	DBG(("CardOpen()\n"));
	CardOpen();
	TimeOutSet(&CardDctTimer, 0);
#endif

#ifdef FUNC_RADIO_EN
#ifdef FUNC_SLAVE_UART_EN
       gRadioCtrl.RadioUpperBound = RADIO_UPPER_BOUND;
       gRadioCtrl.RadioLowerBound = RADIO_LOWER_BOUND;
	gRadioCtrl.RadioFreqBase = gRadioCtrl.RadioLowerBound;   
	gRadioCtrl.RadioSeekStep = RADIO_SEEK_STEP_100KHZ;
#endif

	RadioInit();
	RadioPowerDown();
#endif

	//等待300毫秒，第一次检测所有设备的连接状态
	//消除系统复位后第一次的后插先播
	TimeOutSet(&Timer, 300);
	while(!IsTimeOut(&Timer))
	{	
#ifdef FUNC_HEADPHONE_ADC_DETECT_EN
		KeyEventGet();	
#endif
	
#ifdef FUNC_USB_EN
		IsUsbInsert();
#endif
#ifdef AU6210K_NR_D_8_CSRBT
#ifdef FUNC_LINEIN_EN
		IsLineInInsert();
#endif
#ifdef FUNC_CARD_EN
		IsCardInsert();
#endif

#else
#ifdef FUNC_CARD_EN
		IsCardInsert();
#endif

#ifdef FUNC_LINEIN_EN
		IsLineInInsert();
#endif
#endif
	}

#ifdef	FUNC_HEADPHONE_DETECT_EN
	IsHeadPhonInsert();
#endif

#ifdef OPTION_CHARGER_DETECT
	IsChargerInsert();
	
	if(IsChargerLinkFlag == TRUE)  
	{
#ifdef	FUNC_DEVICE_FORCEPAUSE_EN
		gDevicePlayForcePauseFlag = TRUE;
#endif
	}	
	else
	{			
#ifdef FUNC_POWER_ON_AUDIO_EN
		gUSBChargePowerOnFlag = FALSE;					
#endif
	}
#endif	 

#if (defined(FUNC_OTP_KEY_SOUND_EN) || defined(FUNC_SPI_FLASH_POWERON_SOUND))
	SystemPowerOnSound();	//系统开机音乐播放
#endif

#if (defined(FUNC_AUDIO_EN) || defined(FUNC_AUDIO_READER_EN))
//连接PC开机直接进入Audio 模式
#ifdef FUNC_POWER_ON_AUDIO_EN
#ifdef FUNC_AUDIO_EN
	gSys.SystemMode = SYS_MODE_AUDIO;
#else
	gSys.SystemMode = SYS_MODE_AUDIO_READER;
#endif
#endif
#endif

#ifdef FUNC_STANDBY_EN
	gSys.SystemMode = SYS_MODE_STANDBY;
#endif
#ifdef FUNC_PT231X_EN
       gSys.Volume =VOLUME_INIT;
       PT2313Init();
#endif

#if defined(AU6210K_NR_D_9X_XJ_HTS)
	if (IsUsbLinkFlag == TRUE)
	{
		gSys.SystemMode = SYS_MODE_USB;
	}
	else if (IsCardLinkFlag == TRUE)
	{
		gSys.SystemMode = SYS_MODE_SD;
	}
	else
	{
		gSys.SystemMode = SYS_MODE_BLUETOOTH;
	}
#endif

#if defined(AU6210K_NR_D_8_CSRBT)|| defined(AU6210K_ZB_BT007_CSR)
	if (IsLineInLinkFlag == TRUE)
	{
		gSys.SystemMode = SYS_MODE_LINEIN;
	}
	else if(IsCardLinkFlag == TRUE)
	{
		gSys.SystemMode = SYS_MODE_SD;
	}
	else
	{
		gSys.SystemMode = SYS_MODE_BLUETOOTH;
	}
#endif


#if defined(AU6210K_HXX_B002)
	if (IsCardLinkFlag == TRUE)
	{
		gSys.SystemMode = SYS_MODE_SD;
	}
	else
	{
		gSys.SystemMode = SYS_MODE_BLUETOOTH;
	}
#endif

#if defined(AU6210K_JLH_JH82B)
		gSys.SystemMode = SYS_MODE_BLUETOOTH;
#endif
	if(!TryGotoMode(gSys.SystemMode))
	{
#ifndef FUNC_QUICK_RESPONSE_EN
#ifdef FUNC_BLUETOOTH_EN
		TryGotoValidMode(SYS_MODE_BLUETOOTH);
#else
#ifdef AU6210K_NR_D_8_CSRBT
		TryGotoValidMode(SYS_MODE_SD);
#else
		TryGotoValidMode(SYS_MODE_BLUETOOTH);
#endif
#endif
#else
		//进入断点信息中的模式时失败，则尝试进入第一个有效模式
		if(!IsSwitchMode)
		{
			TryGotoValidMode(SYS_MODE_USB);
		}

		//有模式按键，则尝试切换模式
		while(IsSwitchMode)
		{
			IsSwitchMode = FALSE;
			DBG(("CurMode: %d\n", (WORD)CurMode));
			TryGotoNextMode(CurMode);
		}
#endif
	}
}

// Device state control.
VOID DevStateCtrl(VOID)
{
	static MESSAGE Event;	//为解决Event值被修改bug将变量声明为static类型.	
	BYTE TempMode = gSys.SystemMode;

	Event = MessageGet(MSG_FIFO_DEV_CTRL);	 
//	DBG(("DevStateCtrl\nevent:%bx\n", Event));
//DBG1(("vol = %x\n",gSys.Volume));
#ifdef AU6210K_NR_D_8_CSRBT
#if defined(FUNC_NPCA110X_EN) 
	if(Event == MSG_DEFAULT)
	{
		DBG1(("msg_default\n"));
		if(!isDealSetBass)
		{			
			isDealSetBass = TRUE;
			TimeOutSet(&SetBassTimer, 0);

			if(isDefaultBass)
			{
				isDefaultBass = FALSE;
			}
			else
			{
				isDefaultBass = TRUE;
			}
		}
	}

	
	if(IsTimeOut(&SetBassTimer) && isDealSetBass)
	{

		TimeOutSet(&SetBassTimer, SET_BASS_TIME);
		if(isDefaultBass)
		{
			NPCA110X_NormalBass(i,BassLed_CallBak);
		}
		else
		{
			NPCA110X_SetBass(i,BassLed_CallBak);
		}
		i++;
				DBG1(("msg_default deal i = %x\n",i));

	}
	else
	{
		i = 0;
	}
#endif
#endif

#ifdef FUNC_QUICK_RESPONSE_EN
	IsSwitchMode = FALSE;
#endif

#ifdef FUNC_STANDBY_EN
	//待机状态下只响应待机退出消息
	if(gSys.SystemMode == SYS_MODE_STANDBY)
	{
		if(Event == MSG_POWER)
		{
			DBG(("EXIT STANDBY!\n"));
			InDacInit(FALSE);			
			if(!TryGotoMode(gSystemExecState))
			{
				TryGotoValidMode(SYS_MODE_USB);
			}	
		    #ifdef FUNC_POWER_AMPLIFIER_EN
		       if(gSystemExecState != SYS_MODE_RADIO)
			      ABPowerAmplifierOn();
            #endif
			
			#ifdef FUNC_USB_VOLUP_EN
			    if (Mode != SYS_MODE_USB)
			   {
				  USB_VOLUP_Off();
			   }
            #endif		
			
		}
		return;
	}
#endif

#ifdef POWER_SAVING_MODE_OPTION
	//如果系统带有关闭功能(POWERDOWN或SLEEP)，收到系统关闭消息后，则执行系统关闭操作
	if((Event == MSG_SYS_OFF) || (Event == MSG_POWER))
	{	
#ifdef FUNC_STANDBY_EN
		TryGotoValidMode(SYS_MODE_STANDBY);
#else
		SystemOff();
#endif
	}
#endif
	
#ifdef FUNC_USB_EN
	//后插先播检测
	//DBG(("gIsUsbReconnect%bx\n",gIsUsbReconnect));
	if(IsUsbInsert() || ((gIsUsbRemove == TRUE) && (IsUSBLink())))			//有U盘新插入
	{		
		IsUsbLinkFlag = TRUE;	//usb 快速插拔过程中一个逻辑漏洞
		if((gFsInfo.DevID == DEV_ID_OTP) && (gSys.SystemMode == SYS_MODE_OTPDEV))
		{
			gPlayCtrl.State = PLAY_STATE_STOP;
		}				
		DBG(("IsUsbInsert()\n"));
		if((gIsUsbRemove == TRUE) && IsUSBLink())
		{
			DBG(("**Ins**\n"));
		}
#ifdef	FUNC_USB_CHARGE_EN
		AppleDeviceFlag = FALSE;    
#endif
		gIsUsbRemove = FALSE;
		if(!TryGotoMode(SYS_MODE_USB))
		{
			TryGotoValidMode(TempMode);
		}			
	}
#endif
	//DBG1(("IsDacMute = %x\n",IsDacMute));

#ifdef FUNC_CARD_EN
	//有卡新插入，并且当前不是处于读卡器或一线通模式，则做后插先播处理
	if(IsCardInsert()		
	&& (gSys.SystemMode != SYS_MODE_READER)
	&& (gSys.SystemMode != SYS_MODE_AUDIO_READER))		
	{	
		if((gFsInfo.DevID == DEV_ID_OTP) && (gSys.SystemMode == SYS_MODE_OTPDEV))
		{
			gPlayCtrl.State = PLAY_STATE_STOP;
		}				
		DBG(("IsCardInsert()\n"));
		MuteOn(TRUE,TRUE);
		WaitMs(150);//解决init sd card error问题
		TryGotoValidModeReturn(SYS_MODE_SD);
	}
#endif

#ifdef FUNC_LINEIN_EN
	if(IsLineInInsert())	//有line-in新插入
	{
		DBG(("IsLineInInsert()\n"));
		TryGotoValidModeReturn(SYS_MODE_LINEIN);
	}
#endif

#if ((defined FUNC_AUDIO_EN) || (defined FUNC_READER_EN) || (defined FUNC_AUDIO_READER_EN))
	if(IsPcInsert()) 		//有PC新插入
	{
		DBG(("IsPcInsert()\n"));

#if (defined FUNC_AUDIO_EN)
		TryGotoValidModeReturn(SYS_MODE_AUDIO);
#elif (defined FUNC_READER_EN)
		TryGotoValidModeReturn(SYS_MODE_READER);
#elif (defined FUNC_AUDIO_READER_EN)
		TryGotoValidModeReturn(SYS_MODE_AUDIO_READER);
#endif
	}
#endif

#ifdef	FUNC_DEVICE_FORCEPAUSE_EN
	if(IsChargerInsert())	//有PC或充电器新插入
	{
		gDevicePlayForcePauseFlag = TRUE;
#ifdef	FUNC_RADIO_EN
		if((gSys.SystemMode == SYS_MODE_RADIO) && (gRadioCtrl.State != RADIO_IDLE))
		{
			gRadioCtrl.State = RADIO_IDLE;	
		}
#endif
		MuteOn(TRUE, TRUE);
	}
	else if(gDevicePlayForcePauseFlag == TRUE)
	{
		if(!IsInCharge())
		{
			gDevicePlayForcePauseFlag = FALSE;
			if((gSys.SystemMode == SYS_MODE_SD) || (gSys.SystemMode == SYS_MODE_USB)) 
			{
				MessageSend(MSG_FIFO_KEY, MSG_PLAY_PAUSE);
			}	
#ifdef	FUNC_RADIO_EN
			else if(gSys.SystemMode == SYS_MODE_RADIO)
			{
				UnMute();
			}
#endif
		}
	}
#endif

#ifdef	FUNC_HEADPHONE_DETECT_EN
	if(IsTimeOut(&sHeadPhoneCheckTimer))
	{		
#ifdef FUNC_HEADPHONE_ADC_DETECT_EN		
		TimeOutSet(&sHeadPhoneCheckTimer, 0);
		if(IsHeadphoneLinkFlag == TRUE)
#else		
		TimeOutSet(&sHeadPhoneCheckTimer, 100);
		if(IsHeadPhonInsert())  //有耳机插入
#endif
		{
#ifdef FUNC_EXMUTE_EN
			ExMuteOn(); 	
#endif			
			//DBG(("HeadPhone insert\n"));
		}
		else if((IsHeadphoneLinkFlag == FALSE) && (IsDacMute == FALSE))
		{
#ifdef FUNC_EXMUTE_EN
			ExMuteOff();
#endif			
			//DBG(("HeadPhone no insert\n"));
		}
	}
#endif

	//设备拔除检测
	//cunrrent device is removed.
	switch(gSys.SystemMode)
	{
#ifdef FUNC_USB_EN
		case SYS_MODE_USB:			//current system mode is USB		
			if(gDeviceError)		//usb error
			{
				DBG(("gDeviceError\n"));
				gDeviceError = 0;
				//如果USB卡出现错误，则这里会尝试重新进入USB模式
				if(!TryGotoModeUSB())
				{
#if 0
					if (!TryGotoModeSD())
					{
						TryGotoValidMode(SYS_MODE_LINEIN);
					}
#else
					TryGotoValidMode(SYS_MODE_SD);
#endif
				}
			}
			else if(!IsUSBLink())		//usb removed
			{
				DBG(("usb removed!\n"));
#ifdef FUNC_USB_CHARGE_EN			
				AppleDeviceFlag = FALSE;
#endif

#if 0
				if (!TryGotoModeSD())
				{
					TryGotoValidMode(SYS_MODE_LINEIN);
				}
#else
				TryGotoValidMode(SYS_MODE_SD);
#endif
			}	
			break;
#endif

#ifdef FUNC_CARD_EN
		case SYS_MODE_SD:			//current system mode is SD
			if(gDeviceError)		//sd-card error
			{
				DBG(("gDeviceError\n"));
				//如果SD卡出现错误，则这里会尝试重新进入SD模式
				if(!TryGotoModeSD())
				{
#if defined(AU6210K_JLH_JH82B) || defined(AU6210K_NR_D_8_CSRBT)
					if (!TryGotoModeLINEIN())
					{
						TryGotoValidMode(SYS_MODE_BLUETOOTH);
					}
#elif defined(AU6210K_HXX_B002)
					TryGotoValidMode(SYS_MODE_BLUETOOTH);
#else
					TryGotoValidMode(SYS_MODE_USB);
#endif
				}
			}
			else if(!IsCardLink())		//sd-card removed
			{
				DBG(("card removed!\n"));
#if defined(AU6210K_JLH_JH82B) || defined(AU6210K_NR_D_8_CSRBT)
				if (!TryGotoModeLINEIN())
				{
					TryGotoValidMode(SYS_MODE_BLUETOOTH);
				}
#elif defined(AU6210K_HXX_B002)
				TryGotoValidMode(SYS_MODE_BLUETOOTH);
#else
				TryGotoValidMode(SYS_MODE_USB);
#endif
			}
			break;
#endif

#ifdef FUNC_LINEIN_EN
		case SYS_MODE_LINEIN:		//current system mode is LINEIN
			if(!IsLineInLink())		//line-in removed
			{
				DBG(("line-in removed!\n"));
				TryGotoValidMode(SYS_MODE_USB);
			}
			break;
#endif

#if (defined(FUNC_AUDIO_EN) || defined(FUNC_READER_EN) || defined(FUNC_AUDIO_READER_EN))
		case SYS_MODE_AUDIO:		//current system mode is AUDIO
		case SYS_MODE_READER:		//current system mode is READER
		case SYS_MODE_AUDIO_READER:	//current system mode is AUDIO_READER
			//DBG(("DevStateCtrl->SYS_MODE_AUDIO\n"));
			if(!IsPcLink())			//pc removed
			{
				DBG(("pc removed 0!\n"));
				WaitMs(400);
				if(IsPcInsert())
				{
					DBG(("pc !rem"));
					break;
				}
				DBG(("pc removed!\n"));
#ifdef AU6210K_ZB_BT007_CSR
				TryGotoValidMode(SYS_MODE_SD);
#else
				TryGotoValidMode(SYS_MODE_USB);
#endif
			}
			break;
#endif

#ifdef	FUNC_RADIO_EN
		case SYS_MODE_RADIO:
			break;
#endif

		case SYS_MODE_NONE: 
#ifdef FUN_SYSTEM_POWEROFF_WAIT_TIME
			SystemOffTimeMsgPro(Event);
#endif	
			break;

		default:
			break;
	}

	//按键切换检测
	
	if((Event == MSG_MODE_SW) 
#ifdef FUNC_QUICK_RESPONSE_EN
	|| IsSwitchMode
#endif	
	)
	{
		DBG(("DevStateCtrl,MODE\n"));
		if((gFsInfo.DevID == DEV_ID_OTP) && (gSys.SystemMode == SYS_MODE_OTPDEV))
		{
			gPlayCtrl.State = PLAY_STATE_STOP;
		}
		
#ifdef	FUNC_DEVICE_FORCEPAUSE_EN
		gDevicePlayForcePauseFlag = FALSE;
#endif

#ifdef FUNC_BEEP_SOUND_EN	
		PushKeyBeep(1);
#endif		

#ifndef FUNC_QUICK_RESPONSE_EN
		TryGotoNextMode(gSys.SystemMode);
#else
		IsSwitchMode = TRUE;
		while(IsSwitchMode)
		{
			IsSwitchMode = FALSE;
			DBG(("CurMode: %d\n", (WORD)CurMode));
			TryGotoNextMode(CurMode);
		}
#endif
	} 
	
#if defined(AU6210K_DEMO)
#ifdef FUNC_USB_EN
	if((Event == MSG_MODE_USB) && (gSys.SystemMode != SYS_MODE_USB))
	{
		if(IsUsbLinkFlag == TRUE)
		{
			//DBG(("DevStateCtrl,USB MODE\n"));
			TryGotoValidModeReturn(SYS_MODE_USB);
		}
		else
		{
			//DBG(("DevStateCtrl,NO USB\n"));
		}
	}
#endif

#ifdef FUNC_CARD_EN
	if((Event == MSG_MODE_SD) && (gSys.SystemMode != SYS_MODE_SD))
	{
		if(IsCardLinkFlag == TRUE)
		{
			//DBG(("DevStateCtrl,SD MODE\n"));
			TryGotoValidModeReturn(SYS_MODE_SD);
		}
		else
		{
			//DBG(("DevStateCtrl,NO CARD\n"));
		}
	}	
#endif

#ifdef FUNC_RADIO_EN
	if((Event == MSG_MODE_FM) && (gSys.SystemMode != SYS_MODE_RADIO))
	{
		if(Radio_Name != RADIO_NONE)
		{
			//DBG(("DevStateCtrl,FM MODE\n"));
			TryGotoValidModeReturn(SYS_MODE_RADIO);
		}
		else
		{
			//DBG(("DevStateCtrl, NO FM or FM Error\n"));
		}
	}
#endif

#ifdef FUNC_LINEIN_EN
	if((Event == MSG_MODE_AUX) && (gSys.SystemMode != SYS_MODE_LINEIN)) 
	{
		if(IsLineInLink())
		{
			//DBG(("DevStateCtrl,AUX MODE\n"));
			TryGotoValidModeReturn(SYS_MODE_LINEIN);
		}
		else
		{
			//DBG(("DevStateCtrl, NO AUX\n"));
		}
	}	
#endif
#endif

#ifdef FUNC_SNOOZE_EN
	//SNOOZE 按键检测
	if((sAlarmPowerOnFlag == TRUE) && (Event == MSG_SNOOZE))
	{
		TryGotoValidMode(SYS_MODE_STANDBY);
		sSnoozeOnFlag = TRUE;
	}
#endif
}


#ifdef FUNC_STANDBY_EN
VOID StandbyStateCtrl(VOID)
{	
#ifdef AU6210K_BOOMBOX_DEMO
#ifdef FUNC_ALARM_EN
	if(gAlarm1OnFlag == TRUE)
	{
		gAlarm1OnFlag = FALSE;
		sAlarmPowerOnFlag = TRUE;
		gSys.Volume = gAlarm1Volume;
		gSys.SystemMode = gAlarm1Source;
		InDacInit(FALSE);
		TryGotoValidMode(gSys.SystemMode);		 
	}
	else if(gAlarm2OnFlag == TRUE)
	{
		gAlarm2OnFlag = FALSE;
		sAlarmPowerOnFlag = TRUE;
		gSys.Volume = gAlarm2Volume;
		gSys.SystemMode = gAlarm2Source;
		InDacInit(FALSE);
		TryGotoValidMode(gSys.SystemMode);
	}	
#endif
#endif

#ifdef FUNC_SNOOZE_EN
	if((sSnoozeOnFlag == TRUE) && (IsTimeOut(&sSnoozeTimer)))
	{
		sSnoozeOnFlag = FALSE;
		TryGotoValidMode(gSystemExecState);
	}
#endif
}
#endif



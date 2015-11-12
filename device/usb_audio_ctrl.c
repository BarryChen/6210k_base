#include "syscfg.h"
#include "sysctrl.h"
#include "audio_stream.h"
#include "device_audio.h"
#include "device_hcd.h"
//#include "display.h"
#include "breakpoint.h"
//#include "24cxx.h"
#include "user_interface.h"
#include "debug.h"
#include "sys_on_off.h"


#if (defined(FUNC_AUDIO_EN) || defined(FUNC_READER_EN) || defined(FUNC_AUDIO_READER_EN))

//单独的声卡模式，使用下面的VID、PID
WORD AudioVID = 0x0000;
WORD AudioPID = 0x0106;

//一线通模式，使用下面的VID、PID
WORD AudioReaderVID = 0x0000;
WORD AudioReaderPID = 0x0108;

//单独的声卡模式或一线通模式，都是使用下面的STRING
BYTE* AudioString_Manu = "MVSILICON.INC.";			//max length: 32bytes
BYTE* AudioString_Product = "MV USB AUDIO";			//max length: 32bytes
BYTE* AudioString_SerialNumber = "2011090112345";	//max length: 32bytes



//做USB声卡时的音量调节方案是：
//1. 最终DAC输出的音量为本地音量值gVolArr[gSys.Volume]与PC端音量的相乘后的结果。
//	 所以本地通过按键调节gSys.Volume可以改变音量，PC端调节音量也可以改变音量。
//2. 当在PC端调节“扬声器”音量时，PC会发命令通知USB声卡将音量设置为指定值，
//	 声卡驱动将该音量值保存于AudioLeftVolume和AudioRightVolume变量中，
//	 然后调用DeviceAudioSetVolume()函数完成音量的设置。
//3. 左右声道的音量值分别调节，PC端的balance调节实际就是对USB声卡的左右声道分别设置不同音量值。
//4. 当USB声卡刚连接到PC时，USB声卡会将AudioLeftVolume和AudioRightVolume的值返回给PC，做为初始默认音量。


//下面两个变量缓存PC端的音量值
WORD AudioLeftVolume = AUDIO_MAX_VOLUME / 10;		//PC端默认音量为最大音量的10%
WORD AudioRightVolume = AUDIO_MAX_VOLUME / 10;	//PC端默认音量为最大音量的10%



//下面几个函数供AudioStreamProcess()函数回调

//设置音量，PC端调节“扬声器---音量”、“扬声器---平衡”时会调用到这个函数。
VOID DeviceAudioSetVolume(VOID)
{
	extern WORD	CODE gVolArr[];
	WORD LeftVolume;
	WORD RightVolume;
	
	LeftVolume = ((DWORD)AudioLeftVolume) * gVolArr[gSys.Volume] / AUDIO_MAX_VOLUME;
	RightVolume = ((DWORD)AudioRightVolume) * gVolArr[gSys.Volume] / AUDIO_MAX_VOLUME;
	InDacSetVolume(LeftVolume, RightVolume);
//	DBG(("AudioLeftVolume:%d, AudioRightVolume:%d, gVolArr[%bd]:%d, LeftVolume:%d, RightVolume:%d\n", AudioLeftVolume, AudioRightVolume, gSys.Volume, gVolArr[gSys.Volume], LeftVolume, RightVolume));
#ifdef FUNC_BREAK_POINT_EN
	BP_SaveInfo(&gBreakPointInfo.PowerMemory.Volume, sizeof(gBreakPointInfo.PowerMemory.Volume));
#endif
}


//静音使能，PC端调节“扬声器---全部静音”时会调用到这个函数。
VOID DeviceAudioMuteEn(VOID)
{
	InDacSetVolume(0, 0);
}


//静音取消，PC端调节“扬声器---全部静音”时会调用到这个函数。
VOID DeviceAudioMuteDis(VOID)
{
	DeviceAudioSetVolume();
	UnMute();
	gSys.MuteFg = FALSE;
}


//设备配置好
VOID DeviceAudioConfigOK(VOID)
{
	AudioStreamInit();
	SetChipWorkOnAudioMode();
	SetIntEnBit(MASK_INT_OTG_MC);	
}


//initial usb audio device. 
BOOL DeviceAudioCtrlInit(VOID)
{		
	DeviceAudioInit();
	DeviceAudioSetVolume();
	UnMute();
	gSys.MuteFg = FALSE;
	gAudioState = AUDIO_INIT;
#ifdef FUNC_BREAK_POINT_EN
	BP_SaveInfo(&gBreakPointInfo.PowerMemory.SystemMode, sizeof(gBreakPointInfo.PowerMemory.SystemMode));
#endif
	return TRUE;

#ifdef FUNC_USB_VOLUP_EN

	USB_VOLUP_Off();//usb状态下给个高
			
#endif		
}


//end usb audio device.
VOID DeviceAudioCtrlEnd(VOID)
{	
	UsbDeviceDisConnect();
	ClrIntEnBit(MASK_INT_OTG_MC);
}


//usb audio device state control.
VOID DeviceAudioStateCtrl(VOID)
{	
	BYTE Event = MessageGet(MSG_FIFO_KEY);

	//DBG(("Event;%bx\n",Event));
	if(gAudioState != AUDIO_INIT)
	{// Audio初始化成功后再响应按键命令.
		if(Event == MSG_VOL_ADD)
		{		
			VolumeAdjust(UP);
		}
		else if(Event == MSG_VOL_SUB)
		{		
			VolumeAdjust(DOWN);
		}
		else if((Event == MSG_PLAY_PAUSE) || (Event == MSG_PLAY_1))
		{
			DBG(("MSG_PAUSE\n"));
			DeviceAudioSendPcCmd(PC_CMD_PLAY_PAUSE);
		}
		else if(Event == MSG_STOP)
		{
			DBG(("MSG_STOP\n"));
			DeviceAudioSendPcCmd(PC_CMD_STOP);
		}
		else if(Event == MSG_NEXT)
		{
			DBG(("MSG_NEXT\n"));
			DeviceAudioSendPcCmd(PC_CMD_NEXT);
		}
		else if(Event == MSG_PRE)
		{
			DBG(("MSG_PRE\n"));
			DeviceAudioSendPcCmd(PC_CMD_PRE);
		}
		else if(Event == MSG_MUTE)
		{		
			MuteStatusChange();
			DBG(("MSG_MUTE,gSys.MuteFg:%bd\n", gSys.MuteFg));
			DeviceAudioSendPcCmd(PC_CMD_MUTE);
		}
#ifdef FUN_SYSTEM_POWEROFF_WAIT_TIME
		SystemOffTimeMsgPro(Event);
#endif
	}
	
	//USB控制命令处理
	DeviceAudioRequestProcess();

	//USB ISO数据包处理
	AudioStreamProcess();
}


#endif

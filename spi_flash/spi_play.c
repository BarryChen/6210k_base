#include "syscfg.h"
#include "sysctrl.h"
#include "playctrl.h"
#include "spi_play.h"
#include "spi_fs.h"
#include "devctrl.h"
#include "debug.h"
#include "spi_flash.h"
#include "message.h"
#include "key.h"
#include "timer.h"
#include "spi_flash.h"
#include "spi_fs.h"
#include "user_interface.h"
#include "pcm_amplitude.h"
#include "device.h"
#include "bt.h"
#include "Adc_key.h"
#include "Bluetooth_cmd.h"
#include "bt.h"
#include "npca110x.h"

#ifdef FUNC_SPI_KEY_SOUND_EN

typedef struct _ST_SPIFILEINFO
{
	DWORD dwStartAddr;
	DWORD dwFileSize;
	DWORD dwOffset;
	BYTE  ucFileType;
	BYTE  ucFileNum;
	SONG_INFO gSongInfo;
	BYTE  Volume;

} ST_SPIFILEINFO;

ST_SPIFILEINFO gstSpiFileInfo;
static BOOL isSpiPlay = FALSE;

extern BOOL		s_BreakPointFlag;

extern VOID PlayVol2Decd(VOID);
extern VOID SongPlayTimePause(VOID);
extern VOID BluetoothRefreshStatus();
BOOL GetSpiPlay()
{
	return isSpiPlay ;
}
BOOL SPIFileEOF()
{
	DBG(("gSpiFsCtrl.CurrOpLen:		%lu\n", (DWORD)gSpiFsCtrl.CurrOpLen));
	DBG(("gSpiFsCtrl.CurrOpFile.FileSize:	  %lu\n", (DWORD)gSpiFsCtrl.CurrOpFile.FileSize));
	if(gSpiFsCtrl.CurrOpLen >= gSpiFsCtrl.CurrOpFile.FileSize)
	{
		return TRUE;
	}

	return FALSE;
}


//fet file info
BOOL SPIFileGetInfo()
{
	//gstSpiFileInfo.dwStartAddr = 48;
	//gstSpiFileInfo.dwFileSize = 153435;//1114697;

	gstSpiFileInfo.dwOffset = 0;
	gstSpiFileInfo.ucFileType = SONG_TYPE_MP3;

	// Setting song information	
	gstSpiFileInfo.gSongInfo.ChannelNum = 1;
	gstSpiFileInfo.gSongInfo.IsVBR = FALSE;
	gstSpiFileInfo.gSongInfo.Mp3MpegVersion = 3;//MPEG_1;
	gstSpiFileInfo.gSongInfo.Mp3Layer = 1;//LAYER_3;
	gstSpiFileInfo.gSongInfo.BytesPerSecond = 192000;
	gstSpiFileInfo.gSongInfo.SamplesPerSecond = 44100;
	gstSpiFileInfo.gSongInfo.SongType = SONG_TYPE_MP3;

	return TRUE;
}

BOOL PlayPhoneNumBreakoff = FALSE;  		//播放电话号码时是否产生中断标志
extern BLUETOOTH_STATUS gBlutoothStatus;


//指定播放时间，初始化decoder，给decoder数据，启动decoder解码
//快进，快退恢复播放
//进入播放状态
BOOL SPI_SongPlayStart(BYTE SpiFileNum)
{	
	isSpiPlay = TRUE;

	if(!SPIFileGetInfo())
	{
		//DBG(("SPIFileGetInfo Error!\n"));
		return FALSE;
	}

	RstDecd();
 	SetDecoderMode(gstSpiFileInfo.gSongInfo.SongType);	       
	if(!SpiFlashFileOpen(SpiFileNum, 0))
	{
		DBG(("SpiFlashFileOpen() Failed!\n"));
		return FALSE;
	}
	
	if(!SpiFlashFileRead(gSongInfo.PlayBuffer, 512))
	{
		DBG(("SpiFlashFileRead() Failed!\n"));
		return FALSE;
	}
	SetDecoderData(gSongInfo.PlayBuffer, 512);	
#ifdef AU6210K_NR_D_8_CSRBT
	gSys.Volume = VOLUME_MAX - 6;	
#else

#ifdef AU6210K_ZB_BT007_CSR
	if(SPIPLAY_SONG_IR_KEY == SpiFileNum || SPIPLAY_SONG_IR_ON_OFF == SpiFileNum || SPIPLAY_SONG_POWER_ON_SOUND1 == SpiFileNum)
		gSys.Volume = VOLUME_MAX/2;	
	else
#endif		
		gSys.Volume = VOLUME_MAX-8;	
#endif
	InDacChannelSel(DAC_CH_DAC);       
#ifdef AU6210K_NR_D_8_CSRBT
	NPCA110X_ADC_Input_CH_Select(INPUT_CH2);
	NPCA110X_SetOutMute(FALSE);
#endif

	InDacSetSampleRate(gstSpiFileInfo.gSongInfo.SamplesPerSecond);
	InDacMuteDis();	
#ifdef FUNC_EXMUTE_EN
#ifdef	FUNC_HEADPHONE_DETECT_EN
	if(IsHeadphoneLinkFlag == FALSE)
#endif
	{
		ExMuteOff();
		DBG1(("r90rr\n"));
	}	
#endif
	WaitMs(50);	

	PlayVol2Decd();
	DecoderStartPlay();
	DBG(("<-SPI_SongPlayStart()\n"));
	
	return TRUE;
}


BOOL SPI_SongPlayDo(VOID)					
{	
	if(SPIFileEOF())
	{
		DBG(("SPIFileEOF!\n"));
		return FALSE;
	}
	
	if(!IsDecoderDone())
	{
		return TRUE;
	}
	
	if(!SpiFlashFileRead(gSongInfo.PlayBuffer, 512))
	{
		DBG(("SpiFlashFileRead() Failed,1!\n"));
		return FALSE;
	}

	SetDecoderData(gSongInfo.PlayBuffer, 512);	
	//DBG(("<-SPI_SongPlayDo()\n"));
	
	return TRUE;
}


//停止解码，关闭DAC
VOID SPISongPlayStop(VOID)						
{
	isSpiPlay = FALSE;
	MuteOn(TRUE, TRUE);
}


static VOID NormalSongPlayResume(VOID)
{
	s_BreakPointFlag = TRUE;
	if (!SongPlayInit())
	{
		DBG(("Song Init Error!\n"));
		return;
	}		
}


BOOL SPI_PlaySelectNum(BYTE SpiPlayNum, BOOL PlayResumeFlag)
{	
	BYTE Volume;

	if(SpiPlayNum == SPIPLAY_SONG_IR_ON_OFF || 
		SpiPlayNum == SPIPLAY_SONG_IR_KEY )
		return FALSE;
		
	if(SpiFlashModel == SpiFlash_None)
	{
		DBG(("unsupport spi dev!\n"));
		return FALSE;
	}

	Volume = gSys.Volume;	
	DecoderPause();
	SongPlayTimePause();
	
	if(gSys.SystemMode == SYS_MODE_RADIO)
	{
		MuteOn(TRUE, TRUE);		
	}
	
	SysClkDivClr();	
 	SPI_SongPlayStart(SpiPlayNum);

	while(SPI_SongPlayDo())
	{
#ifdef FUNC_WATCHDOG_EN
		FeedWatchDog();
#endif
	}

	SPISongPlayStop();	

	gSys.Volume = Volume;		
	
	if(PlayResumeFlag)
	{
		NormalSongPlayResume();	//恢复MP3歌曲播放
		UnMute();
	}

	DBG(("SpiFlashPlayEnd \n"));
#if 0//def AU6210K_ZB_BT007_IR_IC_IS_334M_CSR
	if(gSys.SystemMode == SYS_MODE_BLUETOOTH)	
	{
		if (BluetoothMuteOnOff())
		{
			ExMuteOn();
		}
		else if (gSys.Volume > VOLUME_MIN)
		{
			NormalSongPlayResume();	//恢复MP3歌曲播放
			ExMuteOff();
		}
	}
#endif	

	return TRUE;
}

#endif	


#include "syscfg.h"
#include "sysctrl.h"
#include "playctrl.h"
#include "audio_stream.h"
#include "breakpoint.h"
#include "rtc.h"
#include "user_interface.h"
#include "debug.h"
#include "display.h"
#include "sys_on_off.h"
#include "device.h"
#include "spi_fs.h"
#include "otp_play.h"
#include "pt231x.h"
#include "npca110x.h"


extern WORD gFileRdSize;
#ifdef	FUNC_NUMBER_KEY_EN
extern WORD	gRecvNum;
#endif
#ifdef FUNC_NPCA110X_EN
extern BOOL isDefaultBass;
#endif

#ifdef	FUNC_SPI_UPDATE_EN
extern BYTE gIsMVFFileFind;
#endif


//播放监视定时器
#define PLAY_WATCH_TIME				10000
TIMER	gPlayWatchTimer;

TIMER 	VolumeUpTimer;
BYTE 	VolumeVal = 0;
BOOL 	IsVolumeUpEnd = FALSE;
TIMER   SDCARDPOWEROFF_TIME;
static BOOL PowerOffTimeBeginFlag = FALSE;
BOOL SDCARDPowerOffTime_Start = FALSE;




//快速切歌定时器
#ifdef FUNC_OPEN_FILE_DELAY_EN
#define	OPEN_FILE_DELAY_TIME		1000

static TIMER 	s_OpenFileDelayTimer;
static BOOL 	s_IsOpenFileDelay;
#endif

DWORD 	g_OldPlayTime = -1;


PLAY_CTRL 	gPlayCtrl;
BOOL		s_BreakPointFlag;
//extern BOOL IsFirstSetFrame;

#ifdef FUNC_BREAK_POINT_EN
extern BYTE gFileNameCrc8;//记录CRC8 校验值
extern WORD gCurFileNum;
extern WORD gCurFileNumInFolder;
extern WORD gCurFolderNum;
#endif



//EQ数组，共10个分段，每个分段的频率范围分别是：
//20HZ-680HZ,		680HZ-1.3KHZ,	1.3KHZ-2.1KHZ,	2.1KHZ-2.8KHZ,	2.8KHZ-3.5KHZ,
//3.5KHZ-4.1KHZ,	4.1KHZ-5.5KHZ,	5.5KHZ-8.3KHZ,	8.3KHZ-13.7KHZ,	13.7KHZ-22.0KHZ
//每个分段由一个字节决定该频段内的衰减幅度，-9～+3，表示-9dB～+3dB
BYTE CODE gDecdEQ[][EQ_BANDS_NUM] = 
{
	{0,		0,		0,		0,		0,		0,		0,		0,		0,		0	},	//normal
	{2,		-6,		-4,		-3,		-2,		-4,		-6,		-6,		 2,		 2	},	//ROCK			
	{-1,	 2,		-1,		-3,		-6,		-6,		-6,		-3,		-1,		 2	},	//POP			
	{ 2,	-1,		-1,		 0,		 2,		 2,		 2,		 2,		-3,		-6	},	//classic			
	{-6,	-1,		 2,		 0,		-1,		 0,		 2,		 2,		 1,		-1	},	//JAZZ			
	{-2,	-6,		-2,		-2,		-2,		-4,		-6,		-2,		 2,		-2	},	//blue		
	{ 2,	-1,		-1,		-3,		-6,		-6,		-6,		-6,		 2,		 2	},	//hall 	  		
	{ 2,	 1,		 1,		-1,		-2,		-2,		-2,		-3,		-3,		-3	},	//bass		
	{-6,	-6,		-1,		 1,		 2,		 2,		 2,		 2,		 2,		-1	},	//soft		
	{-3,	 2,		 2,		 2,		 2,		 2,		 2,		-3,		 2,		 2	},	//country 	  		
	{-1,	-6,		-1,		 2,		 2,		 2,		 2,		 2,		 2,		 2	},	//opera
};


// Volume table in dB set.
WORD CODE gVolArr[VOLUME_MAX + 1] =
{
#if (VOLUME_MAX == 16)
   #if defined(AU6210K_NR_D_9X)||defined(AU6210K_NR_D_9X_XJ_HTS)
	0,
	26,		36,		49,		67,
	91,		125,	173,	240,
	332,	460,	636,	880,	
	1318,	1433,	1686,	1830	
   #else
    0,
	26,		36,		49,		67,
	91,		125,	173,	240,
	332,	460,	636,	880,	
	1318,	1986,	2366,	2746	//最高音量约680mVrms
   #endif
#else
	0,
	26,		31,		36,		42,		49,		58,		67,		78,		
	91,		107,	125,	147,	173,	204,	240,	282,	
	332,	391,	460,	541,	636,	748,	880,	1035,	
	1218,	1433,	1686,	1984,	2334,	2746,	3230,	3800	//最高音量约930mVrms
#endif
};


#ifdef FUNC_UARTDBG_EN
VOID PrintPlayInfo(VOID);
#endif

#ifdef	FUNC_DEVICE_FORCEPAUSE_EN
extern BOOL gDevicePlayForcePauseFlag;
#endif


///////////////////////播放计时相关函数////////////////////////////////////////////////////////
//开始使用定时器计时
VOID SongPlayTimeStart(VOID)
{
	g_OldPlayTime = -1;
}

//暂停定时器计时
VOID SongPlayTimePause(VOID)
{
	g_OldPlayTime = -1;
}

//恢复定时器计时
VOID SongPlayTimeResume(VOID)
{
	g_OldPlayTime = -1;
}


//更新播放时间
//播放时，由本模块计时，并将是将更新到歌曲信息结构体中
//快进、快退时，由decoder底层计时
VOID SongPlayTimeUpdate(VOID)			
{
	static BOOL RTC0p5State = FALSE;

//	if(FileEOF(&gPlayCtrl.File))
//	{
//		return;
//	}

	if((RTC0p5State) && HalfSecStatus())
	{
		RTC0p5State = FALSE;
	}
	else if((!RTC0p5State) && !HalfSecStatus())
	{
		RTC0p5State = TRUE;
		if(gPlayCtrl.State == PLAY_STATE_PLAY)
		{
			gSongInfo.CurPlayTime++;
		}
	}

	if((g_OldPlayTime != gSongInfo.CurPlayTime) && (gSongInfo.CurPlayTime > 0))
	{
		g_OldPlayTime = gSongInfo.CurPlayTime;
#ifdef FUNC_UARTDBG_EN
		PrintPlayInfo();
#endif

#ifdef FUNC_BREAK_POINT_EN
		BP_SetPlayAttrib();
#endif
	}	
}


#ifdef FUNC_UARTDBG_EN
//歌曲播放前，显示歌曲信息
VOID DisplaySongInfo(VOID)
{
	DBG(("FileSize:%lu\n", gPlayCtrl.File.Size));
#ifdef FUNC_LONG_NAME_EN
	{
		BYTE LongFileName[66];	 
		BOOL Ret;
		WORD* p;
	
		Ret = FileGetLongName(&gPlayCtrl.File, LongFileName);
		DBG(("%d\n", (WORD)Ret));
		for(p = (WORD*)LongFileName; *p != 0; p++)
		{
			DBG(("%-.2BX %-.2BX ", ((BYTE*)p)[0], ((BYTE*)p)[1]));
		}
		DBG(("\n"));
	}
#endif

//	DBG(("********SONG INFO*************\n"));
	//打印歌曲信息
	switch(gSongInfo.SongType)
	{
		case SONG_TYPE_MP3:
			DBG(("SongType: MP3\n"));
			break;

		case SONG_TYPE_PCM_1CH:
			DBG(("SongType: PCM_1CH\n"));
			break;

		case SONG_TYPE_PCM_2CH:
			DBG(("SongType: PCM_2CH\n"));
			break;

		default:
			DBG(("SongType: UNKOWN(%bu)\n", gSongInfo.SongType));
			break;
	}
	
	DBG(("ChannelNum:       %u\n", (WORD)gSongInfo.ChannelNum));
	DBG(("SamplesPerSecond: %u\n", (WORD)gSongInfo.SamplesPerSecond));
	//kbps = (BytesPerSecond*8)/1000 = BytesPerSecond/125
	DBG(("BytesPerSecond:   %lu (%lukbps)\n", (DWORD)gSongInfo.BytesPerSecond, (DWORD)(gSongInfo.BytesPerSecond/125)));
	DBG(("TotalPlayTime:    %lu\n", (DWORD)gSongInfo.TotalPlayTime));
	DBG(("CurPlayTime:      %lu\n", (DWORD)gSongInfo.CurPlayTime));
	DBG(("FrameLen:         %lu\n", (DWORD)gSongInfo.FrameLen));
	DBG(("FrameNum:         %lu\n", (DWORD)gSongInfo.FrameNum));	
	DBG(("FileHeaderLen:    %lu\n", (DWORD)gSongInfo.FileHeaderLen));
	DBG(("IsVBR:            %u\n", (WORD)gSongInfo.IsVBR));
	DBG(("%s", (gSongInfo.Mp3MpegVersion == 3)?"MPEG_1\n":""));
	DBG(("%s", (gSongInfo.Mp3MpegVersion == 2)?"MPEG_2\n":""));
	DBG(("%s", (gSongInfo.Mp3MpegVersion == 0)?"MPEG_2_5\n":""));
	DBG(("%s", (gSongInfo.Mp3Layer == 3)?"LAYER_1\n":""));
	DBG(("%s", (gSongInfo.Mp3Layer == 2)?"LAYER_2\n":""));
	DBG(("%s", (gSongInfo.Mp3Layer == 1)?"LAYER_3\n":""));
	DBG(("****************************\n"));
	DBG(("\n"));	
}
#endif


#ifdef FUNC_UARTDBG_EN
//歌曲播放中，显示播放信息
VOID PrintPlayInfo(VOID)
{	
	BYTE CODE RepeatName[][4] = {"ALL", "ONE", "RAN", "INT"};
	BYTE CODE EqName[][5] = {"NORM", "ROCK", "POP ", "CLAS", "JAZZ", "BLUE", "HALL", "BASS", "SOFT", "COUN", "OPER"};

		if(gFsInfo.DevID == DEV_ID_USB)
		{
			DBG(("USB\t")); 
		}
		else if(gFsInfo.DevID == DEV_ID_SD)
		{
			DBG(("SD\t"));	
		}

#ifdef FUNC_FOLDER_EN
		if(gPlayCtrl.FolderEnable)
		{
			DBG(("(%u/%u, %u/%u, %u)\t", gPlayCtrl.FolderNum, gFsInfo.ValidFolderSum, gPlayCtrl.FileNum, gPlayCtrl.Folder.IncFileCnt, gFsInfo.FileSum));
			//DBG(("(*%u/%u, %u/%u, %u)\t", gPlayCtrl.File.FolderNum, gFsInfo.FolderSum, gPlayCtrl.FileNum, gPlayCtrl.Folder.IncFileCnt, gFsInfo.FileSum));
		}
		else
		{
			DBG(("(%u/%u, %u/%u)\t", gPlayCtrl.FolderNum, gFsInfo.ValidFolderSum, gPlayCtrl.FileNum, gFsInfo.FileSum));
			//DBG(("(*%u/%u, %u/%u)\t", gPlayCtrl.File.FolderNum, gFsInfo.FolderSum, gPlayCtrl.FileNum, gFsInfo.FileSum));
		}
#else				   
	DBG(("(%u/%u)\t", gPlayCtrl.FileNum, gFsInfo.FileSum));
#endif
		
	DBG(("%-.8s.%-.3s ", 
		&gPlayCtrl.File.ShortName[0],
		&gPlayCtrl.File.ShortName[8]));

	DBG(("%-.2d:%-.2d ", 
		(WORD)(gSongInfo.CurPlayTime/60), 
		(WORD)(gSongInfo.CurPlayTime%60)));

	DBG(("%-.2d ", (WORD)gSys.Volume));
	DBG(("%s ", RepeatName[gPlayCtrl.RepeatMode]));
	DBG(("%s ", EqName[gPlayCtrl.Eq]));

	DBG(("\n"));
}
#endif


// Send volume to decoder.
VOID PlayVol2Decd(VOID)		
{
#ifdef AU6210K_NR_D_8_CSRBT
	InDacSetVolume(gVolArr[32], gVolArr[32]);
	NPCA110X_DAC1_Set_Volume_and_Mute(gSys.Volume);
#else
	InDacSetVolume(gVolArr[gSys.Volume], gVolArr[gSys.Volume]);
#endif
}


// Send system EQ to decoder.
VOID PlayEQ2Decd(VOID)	
{
	DecoderSetEQ(gDecdEQ[gPlayCtrl.Eq]);
}


// Play song, update song data for playing.
BOOL SongPlayDo(VOID)					
{		
	//每次循环中都检查一次文件是否到结尾
	//如果到文件结尾则直接返回TRUE
	//否则执行到FileRead()行会返回FALSE，导致REPEAT ONE模式下也会切到下一首
	//DBG(("********SongPlayDo*************\n"));
	if((gPlayCtrl.State != PLAY_STATE_PLAY) || FileEOF(&gPlayCtrl.File))
	{
	 	//DBG(("gPlayCtrl.State:%bx\n",gPlayCtrl.State));
		return TRUE;
	}
	if(!IsDecoderDone())
	{
		//decoder监视定时器检查
		if(IsTimeOut(&gPlayWatchTimer))
		{
			DBG(("Play Watcher Over!\n"));
			return FALSE;
		}
		return TRUE;
	}
	TimeOutSet(&gPlayWatchTimer, PLAY_WATCH_TIME);
	if(gSongInfo.SongType == SONG_TYPE_MP3)
	{
		//DBG(("gSongInfo.PlayBuffer:%x\n",gSongInfo.PlayBuffer));
		if(!FileRead(&gPlayCtrl.File, gSongInfo.PlayBuffer, gFileRdSize))
		{
			gPlayCtrl.State = PLAY_STATE_IDLE;
			if(gFsError != FS_ERROR_INVALID_ADDRESS)
			{
				DBG(("FileRead() FALSE 021!\n"));
				gFsError = FS_ERROR_READ_FAILED;
			}
			DBG(("FileRead() FALSE 002!\n"));
			return FALSE;
		}
		//DBG(("FR(%x, %x)\n", gSongInfo.PlayBuffer, gFileRdSize));
		SetDecoderData(gSongInfo.PlayBuffer, gFileRdSize);
	}	

	return TRUE;			
}


// Stop decoding and go to stop state.
VOID SongPlayStop(VOID)						
{
	DBG(("SongPlayStop()\n"));
	FileRewind(&gPlayCtrl.File);
	gSongInfo.CurPlayTime = 0;
	gPlayCtrl.State = PLAY_STATE_STOP;

	//关闭DAC
	MuteOn(FALSE, FALSE);	
	WaitMs(100);	//消除歌曲切换时的POP声
}


extern VOID SetFstFrmAddr(WORD Addr);
//指定播放时间，初始化decoder，给decoder数据，启动decoder解码
//快进，快退恢复播放
//进入播放状态
BOOL SongPlayStart(VOID)
{
	//必须RstDecd否则WMA播放可能出问题。
	RstDecd();	
	SetDecoderMode(gSongInfo.SongType);
	
	InDacSetSampleRate(gSongInfo.SamplesPerSecond);
	PlayEQ2Decd();	
	//VolumeVal = 0;
	IsVolumeUpEnd = FALSE;

//	if(!gSys.MuteFg)
//	{
//		DBG(("SongPlayStart,UnMute\n"));	
//		UnMute();
//	} 	

	//DBG(("Resume to BP!\n"));
	DecoderStartPlay();	
#ifdef FUNC_OTP_PLAY_EN	
	if(gSys.SystemMode != SYS_MODE_OTPDEV)
#endif
	{
		DecoderSeekToTime(gSongInfo.CurPlayTime);  //指定播放时间
	}

	//DBG(("gSongInfo.PlayBuffer:%x, gFileRdSize:%x\n",gSongInfo.PlayBuffer, gFileRdSize));
	if(!FileRead(&gPlayCtrl.File, gSongInfo.PlayBuffer, gFileRdSize))
	{
		DBG(("FileRead() FALSE 001!\n"));
		if(gFsError != FS_ERROR_INVALID_ADDRESS)
		{
			gFsError = FS_ERROR_READ_FAILED;
		}
		return FALSE;
	}	

	SetFstFrmAddr(gSongInfo.DataOffset);
	SetDecoderData((gSongInfo.PlayBuffer + gSongInfo.DataOffset), (gFileRdSize - gSongInfo.DataOffset));
	DBG(("SongPlayStart(),gSongInfo.DataOffset:%x\n", gSongInfo.DataOffset));
	SongPlayTimeStart();
	TimeOutSet(&gPlayWatchTimer, PLAY_WATCH_TIME);		
	//如果是快进、快退、断点播放，不是从歌曲开头播放，则延时一段时间再打开DAC的音量，消除pop或变调声。
	TimeOutSet(&VolumeUpTimer, (gSongInfo.CurPlayTime > 0) ? 200 : 0);	
	gPlayCtrl.State = PLAY_STATE_PLAY;
	
#ifdef FUNC_DISP_EN
#ifndef FUNC_SINGLE_LED_EN	//单个LED取消是因为换曲LED显示不正常
	DispMute(); //解决切换歌曲后Mute显示状态出错问题。
#endif
#endif

	return TRUE;
}


//开始一首歌曲的播放: 打开文件，分析文件，初始化decoder
BOOL SongPlayInit(VOID)
{
	FOLDER* Folder = NULL;
	
#ifdef FUNC_OPEN_FILE_DELAY_EN
	s_IsOpenFileDelay = FALSE;
#endif
					
#ifdef FUNC_FOLDER_EN
	if(gPlayCtrl.FolderEnable)
	{
		Folder = &gPlayCtrl.Folder;
	}
#endif

#ifdef FUNC_FOLDER_EN
	if(gPlayCtrl.FolderEnable)
	{
		if(gPlayCtrl.FileNum > gPlayCtrl.Folder.IncFileCnt)
		{
			gPlayCtrl.FileNum = 1;
		}
	}
#endif
	if((!gPlayCtrl.FolderEnable) && (gPlayCtrl.FileNum > gFsInfo.FileSum))
	{
		gPlayCtrl.FileNum = 1;
	}

	//打开文件
	DBG(("打开文件,gPlayCtrl.FileNum:%u,gPlayCtrl.FolderNum:%u\n", gPlayCtrl.FileNum, (DWORD)Folder, gPlayCtrl.FolderNum));
	DBG(("FileOpenByNum(%lx, %lx, %x)\n", (DWORD)(&gPlayCtrl.File), (DWORD)Folder, gPlayCtrl.FileNum));
		
	if(!FileOpenByNum(&gPlayCtrl.File, Folder, gPlayCtrl.FileNum))
	{
		DBG(("FileOpenByNum() error!\n"));
		gFsError = FS_ERROR_FILEOPEN_FAILED;
		return FALSE;
	}
#ifdef FUNC_BREAK_POINT_EN
//	DBG(("播放歌曲：%-.8s%-.3s\n", &gPlayCtrl.File.ShortName[0], &gPlayCtrl.File.ShortName[8]));
	gFileNameCrc8 = CRC8Cal(&gPlayCtrl.File.ShortName, 11); 
#ifdef FUNC_OTP_PLAY_EN
	if(gSys.SystemMode != SYS_MODE_OTPDEV)	//开机音乐播放不需要记忆断点信息
#endif
	{
		BP_SaveInfo((BYTE*)(&gBreakPointInfo.PowerMemory), sizeof(gBreakPointInfo.PowerMemory));
	}
//	DBG(("SongPlayInit,BP_SaveInfo(),gFileNameCrc8:%bu\n", gFileNameCrc8));
#endif

#ifdef FUNC_FOLDER_EN
	if((gPlayCtrl.Folder.FolderNum != gPlayCtrl.File.FolderNum)
	#ifdef FUNC_OTP_PLAY_EN
		&& (gSys.SystemMode != SYS_MODE_OTPDEV)
	#endif
	)
	{
		// 更新gPlayCtrl.Folder数据
		gPlayCtrl.FolderNum = GetValidFolderNum(gPlayCtrl.File.FolderNum);
		if(!FolderOpenByValidNum(&gPlayCtrl.Folder, NULL, gPlayCtrl.FolderNum))
		{
			DBG(("*FoldOpenyByNum(%d) Failed!\n", gPlayCtrl.File.FolderNum));
			gFsError = FS_ERROR_FOLDOPEN_FAILED;
			return FALSE;
		}
	}
#endif

	DBG(("\n"));
	DBG(("-----------------------------------\n"));
	DBG(("DEV: %bd\n", gFsInfo.DevID));
	DBG(("FileNumInDisk: %u\n", gPlayCtrl.File.FileNumInDisk));
	DBG(("FileNumInFolder: %u\n", gPlayCtrl.File.FileNumInFolder));
	DBG(("FolderNum: %u\n", gPlayCtrl.File.FolderNum));
	DBG(("DirSecNum: %lu\n", gPlayCtrl.File.DirSecNum));
	DBG(("DirOffset: %u\n", (WORD)gPlayCtrl.File.DirOffset));
	DBG(("FileName: %-.11s\n", gPlayCtrl.File.ShortName));
	DBG(("FileType: %-.2bd\n", gPlayCtrl.File.FileType));
	DBG(("StartSec: %lu\n", gPlayCtrl.File.StartSec));
	DBG(("SecNum: %lu\n", gPlayCtrl.File.SecNum));
	DBG(("OpSec: %lu\n", gPlayCtrl.File.OpSec));
	DBG(("OpSecCnt: %lu\n", gPlayCtrl.File.OpSecCnt));
	DBG(("Size: %lu\n", gPlayCtrl.File.Size));
	DBG(("-----------------------------------\n\n"));	

	//解析文件
	if(!DecoderAnalysis(&gPlayCtrl.File))
	{
		DBG(("\nDecoderAnalysis Error!\n\n"));
		gFsError = FS_ERROR_ANALYSIS_FAILED;
		return FALSE;
	}
#ifdef FUNC_UARTDBG_EN
	DisplaySongInfo();
#endif	

	gSongInfo.CurPlayTime = 0;

#ifdef FUNC_BREAK_POINT_EN
	//DBG(("!!!s_BreakPointFlag:%bu\n", s_BreakPointFlag));
	if(s_BreakPointFlag)
	{
		s_BreakPointFlag = FALSE;
		BP_GetPlayAttrib();
		if(gSongInfo.CurPlayTime >= gSongInfo.TotalPlayTime)
		{
			gSongInfo.CurPlayTime = 0;
		}
		//DBG(("gSongInfo.CurPlayTime:%lu\n", gSongInfo.CurPlayTime));	
	}
#endif

//	IsFastPlayResume = FALSE;
	
	//初始化decoder，并进入播放状态
//	IsFirstSetFrame = TRUE;
	if(!SongPlayStart())
	{
		DBG(("\nSongPlayStart Error!\n"));
		return FALSE;
	}
	return TRUE;
}


static VOID GetNextSongNum(BYTE Direction)
{
	WORD	SongSum;
	WORD	Temp;
	BYTE	StepCnt;

   	//DBG((">>>GetNextSongNum(%bd)\n", Direction));
#ifdef	FUNC_FOLDER_EN	
	if(gPlayCtrl.FolderEnable)
	{
		//如果文件夹号有变化，则重新打开
		if(gPlayCtrl.FolderNum != gPlayCtrl.Folder.ValidFolderNum)
		{
			if(!FolderOpenByValidNum(&gPlayCtrl.Folder, NULL, gPlayCtrl.FolderNum))
			{
				DBG(("FolderOpenByValidNum() error!\n"));
				return;
			}
		}		
		SongSum = gPlayCtrl.Folder.IncFileCnt;
	}
	else
	{
		SongSum = gFsInfo.FileSum;
	}
#else
	SongSum = gFsInfo.FileSum;
#endif

	//DBG(("SongSum: %u(InFold:%u,InDisk:%u)\n", SongSum, gPlayCtrl.Folder.IncFileCnt, gFsInfo.FileSum));

	if(SongSum == 0)
	{
		return;
	}

	switch (gPlayCtrl.RepeatMode)
	{	
		case REPEAT_ONE:
		case REPEAT_ALL: 		
		case REPEAT_INTRO:  
		case REPEAT_FOLDER:
			if(Direction == PLAY_DIRECT_NEXT_10)
			{
				StepCnt = 10;
				Direction = PLAY_DIRECT_NEXT;
			}
			else if(Direction == PLAY_DIRECT_PRE_10)
			{			 	
				StepCnt = 10;
				Direction = PLAY_DIRECT_PRE;
			}
			else
			{
			 	StepCnt = 1;
			} 

			if(Direction == PLAY_DIRECT_NEXT)
			{
				if((gPlayCtrl.FileNum + StepCnt) > SongSum)
				{
					gPlayCtrl.FileNum = 1;
				}
				else
				{
					gPlayCtrl.FileNum += StepCnt;
				}
			}
			else if(Direction == PLAY_DIRECT_PRE)
			{
				if(gPlayCtrl.FileNum <= StepCnt)
				{
					gPlayCtrl.FileNum = SongSum;
				}
				else
				{
					gPlayCtrl.FileNum -= StepCnt;
				}
			}
			break;

		case REPEAT_RANDOM:
			Temp = (gSysTick%SongSum) + 1;
			if(Temp == gPlayCtrl.FileNum)
			{
				Temp++;
			}

			if(Temp > SongSum)
			{
				Temp = 1;	
			}

			gPlayCtrl.FileNum = Temp;
			break;

		default:
			break;
	}
	
	gPlayCtrl.Direction = Direction;
	//DBG(("<<<GetNextSongNum(%bd)\n", Direction));
#ifdef FUNC_DISP_EN
	DispFileNum();
#endif
	return;
}


// Play state process.
VOID PlayStatePlayOp(VOID)					
{	
	//歌曲播放结束，或者浏览播放10秒，切换歌曲
	//如果当前是播放到文件末尾，则需要等待PCM FIFO中的数据播放完毕。
	if((FileEOF(&gPlayCtrl.File) && IsFifoEmpty())
	||((gPlayCtrl.RepeatMode == REPEAT_INTRO) && (gSongInfo.CurPlayTime > 10)))
	{
		DBG(("Song play end!\n"));	
		//等待PCM数据全部被取走
		SongPlayStop();	
		if(gPlayCtrl.RepeatMode != REPEAT_ONE)
		{	   
			GetNextSongNum(PLAY_DIRECT_NEXT);
		}
		if(!SongPlayInit())
		{
			DBG(("PlayStatePlayOp,!SongPlayInit()\n"));
		 	MessageSend(MSG_FIFO_KEY, MSG_NEXT);	
			gPlayCtrl.State = PLAY_STATE_STOP;
			return;
		}
	}

	if(IsTimeOut(&VolumeUpTimer) && (!gSys.MuteFg) && (!IsVolumeUpEnd))
	{
		DBG(("SongPlayStart,UnMute\n"));	
		UnMute();				
		PlayVol2Decd();
		IsVolumeUpEnd = TRUE;
		//DBG(("VolumeUp %bd/%bd\n", VolumeVal, gSys.Volume));
	}		

	//DBG(("SongPlayDo()\n"));
	
#ifdef	FUNC_DEVICE_FORCEPAUSE_EN
	if(gDevicePlayForcePauseFlag == TRUE)
	{
		gPlayCtrl.State = PLAY_STATE_PAUSE;
		gPlayCtrl.Event = MSG_PLAY_PAUSE;
	}
#endif
	if(!SongPlayDo())
	{
		SongPlayStop();
		DBG(("PlayStatePlayOp,!SongPlayDo()\n"));
		if((gPlayCtrl.Direction == PLAY_DIRECT_NEXT_10) || (gPlayCtrl.Direction == PLAY_DIRECT_NEXT))
		{
			MessageSend(MSG_FIFO_KEY, MSG_NEXT);
		}
		else
		{
			MessageSend(MSG_FIFO_KEY, MSG_PRE);
		}	
		gPlayCtrl.State = PLAY_STATE_STOP;	
		return;
	}

	switch(gPlayCtrl.Event)
	{
		case MSG_PLAY_PAUSE:
		case MSG_PLAY_1:
			DBG(("PAUSE\n"));
#ifdef FUNC_BEEP_SOUND_EN	
			PushKeyBeep(1);
#endif
			DecoderPause();
			SongPlayTimePause();
			gPlayCtrl.State = PLAY_STATE_PAUSE;			
#if 0//def FUNC_SPI_KEY_SOUND_EN
			SPI_PlaySelectNum(SPIPLAY_SONG_PAUSE, 0);
#endif
#ifdef FUNC_OTP_KEY_SOUND_EN
			OTP_PlaySelNum(OTPPLAY_NUM_PAUSE, 0);
#endif
			MuteOn(FALSE, TRUE);			
#ifdef FUNC_DISP_EN
			DispPlayState();
#endif
			break;

		case MSG_PP_STOP:
		case MSG_STOP:
			DBG(("STOP\n"));
			SongPlayStop();
			gSys.MuteFg = FALSE;
#ifdef FUNC_DISP_EN
			DispPlayState();
#endif		
#if (FUNC_RESTORE_DEVICE_SELECE == FUNC_RESTORE_DEVICE_EEPROM)			
#ifdef FUNC_BREAK_POINT_EN
			BP_SetPlayAttrib(); //清除记忆播放时间
#endif
#endif
			break;

#ifdef FUNC_FAST_PLAY_EN
		case MSG_FF_START:
#ifdef	FUNC_DEVICE_FORCEPAUSE_EN
			if(gDevicePlayForcePauseFlag == TRUE) 
			{
				break;
			}					
#endif
			DBG(("FF_START\n"));
			MuteOn(FALSE, TRUE);
			DecoderFastPlaySet(FAST_FORWARD, 32);
			gPlayCtrl.State = PLAY_STATE_FF;
#ifdef FUNC_DISP_EN
			DispPlayState();
#endif
			break;

		case MSG_FB_START:
#ifdef	FUNC_DEVICE_FORCEPAUSE_EN
			if(gDevicePlayForcePauseFlag == TRUE) 
			{
				break;
			}					
#endif
			DBG(("FB_START\n"));
			MuteOn(FALSE, TRUE);
			DecoderFastPlaySet(FAST_BACKWARD, 32);
			gPlayCtrl.State = PLAY_STATE_FB;
#ifdef FUNC_DISP_EN
			DispPlayState();
#endif
			break;
#endif

		default:
			break;
	}
}


// Play pause state process.
VOID PlayStatePauseOp(VOID)					
{	
	switch (gPlayCtrl.Event)
	{					
		case MSG_PLAY_PAUSE:
		case MSG_PLAY_1:
			DBG(("RESUME\n"));
#ifdef	FUNC_DEVICE_FORCEPAUSE_EN
			gDevicePlayForcePauseFlag = FALSE;
#endif		
			SetVolumeWithMute();
			DecoderStartPlay();
			SongPlayTimeResume();
			TimeOutSet(&gPlayWatchTimer, PLAY_WATCH_TIME);
			gPlayCtrl.State = PLAY_STATE_PLAY;			
#if 0//def FUNC_SPI_KEY_SOUND_EN
			SPI_PlaySelectNum(SPIPLAY_SONG_PLAY, 1);
#endif
#ifdef FUNC_OTP_KEY_SOUND_EN
			OTP_PlaySelNum(OTPPLAY_NUM_PLAY, 1);
#endif

#ifdef FUNC_DISP_EN
			DispPlayState();
#endif
			break;	
	
		case MSG_PP_STOP:
		case MSG_STOP:
			DBG(("STOP\n"));
			SongPlayStop();
			gSys.MuteFg = FALSE;
#ifdef FUNC_DISP_EN
			DispPlayState();
#endif
#if (FUNC_RESTORE_DEVICE_SELECE == FUNC_RESTORE_DEVICE_EEPROM)			
#ifdef FUNC_BREAK_POINT_EN
			BP_SetPlayAttrib(); //清除记忆播放时间
#endif
#endif
			break;
	
		default:
			break;			
	}	
}


// Play stop state process.
VOID PlayStateStopOp(VOID)				
{
	switch (gPlayCtrl.Event)
	{
		case MSG_NEXT:
		case MSG_PRE:
		case MSG_PREV1:
		case MSG_NEXT1:
			break;
					
		case MSG_PLAY_PAUSE:
		case MSG_PLAY_1:
			DBG(("PLAY\n"));	
#ifdef	FUNC_DEVICE_FORCEPAUSE_EN
			gDevicePlayForcePauseFlag = FALSE;
#endif			
#if 0//def FUNC_SPI_KEY_SOUND_EN
			SPI_PlaySelectNum(SPIPLAY_SONG_PLAY, 0);
#endif
			if(!SongPlayInit())
			{
				DBG(("PlayStateStopOp,!SongPlayInit()\n"));
				gPlayCtrl.State = PLAY_STATE_STOP;
			}
			SetVolumeWithMute();
			break;		
			
		default:
			break;			
	}
}


#ifdef FUNC_FAST_PLAY_EN
// Play State fast forward control.
VOID PlayStateFF_FBOp(VOID)
{
//	DBG((">>PlayStateFF_FBOp()\n"));
	DecoderFastPlayProc();
//	if(DecoderFastPlayProc() == FALSE)
//	{//快进到文件尾或快退到文件头,如果需要自动切换曲目
//		gPlayCtrl.Event = MSG_FF_FB_END;
//	}
	
	switch(gPlayCtrl.Event)
	{
		case MSG_FF_FB_END:
//			DBG(("MSG_FF_FB_END\n"));
			if(!SongPlayStart())
			{
			 	MessageSend(MSG_FIFO_KEY, MSG_NEXT);	
				gPlayCtrl.State = PLAY_STATE_STOP;
			}
#ifdef FUNC_DISP_EN
			DispPlayState();
#endif			
			break;

		default:
			break;
	}
//	DBG(("<<PlayStateFF_FBOp()\n"));
}
#endif


// Play state control.
VOID PlayStateCtrl(VOID)									
{
//	DBG((">>>PlayStateCtrl()\n"));	
	BYTE minvolflag = 0;
	BYTE maxvolflag = 0;

	gPlayCtrl.Event = MessageGet(MSG_FIFO_KEY);

#ifdef FUNC_OPEN_FILE_DELAY_EN
	if(s_IsOpenFileDelay)
	{	
		if((gPlayCtrl.Event == MSG_NONE) && IsTimeOut(&s_OpenFileDelayTimer))
		{
			if(!SongPlayInit())
			{
				MessageSend(MSG_FIFO_KEY, (gPlayCtrl.Direction == PLAY_DIRECT_PRE) ? MSG_PRE : MSG_NEXT);
				gPlayCtrl.State = PLAY_STATE_STOP; 
			}
			s_IsOpenFileDelay = FALSE;
		}
	}
#endif

	if(gPlayCtrl.State != PLAY_STATE_BROWSE)
	{
		//处理各种状态下的通用事件
		switch (gPlayCtrl.Event)
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
		if (gPlayCtrl.Event == MSG_V_ADD && gSys.Volume == VOLUME_MAX && !maxvolflag)
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
		if (gPlayCtrl.Event == MSG_V_SUB && gSys.Volume == VOLUME_MIN && !minvolflag)
		{
			SPI_PlaySelectNum(SPIPLAY_SONG_MIN_VOLUME, 1);
			//UnMute();
		}
#endif
		break;	
#ifdef FUNC_PT231X_EN
	        case MSG_TREBUP:
			case MSG_TREBDN:
			case MSG_BASSUP:
			case MSG_BASSDN:
			case MSG_DEFAULT:
				PT2315E_Do(gPlayCtrl.Event);			
				break;  

#elif 0// defined(FUNC_NPCA110X_EN)
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

			case MSG_INTRO:
				if(gPlayCtrl.RepeatMode == REPEAT_INTRO)
				{
					gPlayCtrl.RepeatMode = REPEAT_ALL;
				}
				else
				{
					gPlayCtrl.RepeatMode = REPEAT_INTRO;
				}
#ifdef FUNC_DISP_EN
				DispRepeat(TRUE);
#endif				
				break;

			case MSG_REPEAT:				
				DBG(("REPEAT\n"));
				/*gPlayCtrl.RepeatMode++;
				if(gPlayCtrl.RepeatMode >= REPEAT_MODE_SUM)
				{
					gPlayCtrl.RepeatMode = REPEAT_ALL;
				}
				if(gPlayCtrl.RepeatMode == REPEAT_FOLDER)
				{
					gPlayCtrl.FolderEnable = TRUE;
					gPlayCtrl.FileNum = gPlayCtrl.File.FileNumInFolder;
				}
				else
				{
					gPlayCtrl.FolderEnable = FALSE;
					gPlayCtrl.FileNum = gPlayCtrl.File.FileNumInDisk;
				}*/
                if(gPlayCtrl.RepeatMode == REPEAT_ALL)
				{
					gPlayCtrl.RepeatMode = REPEAT_ONE;
				}
				else
				{   
				    gPlayCtrl.RepeatMode = REPEAT_ALL;
				}

				
				if(gPlayCtrl.RepeatMode == REPEAT_FOLDER)
				{
					gPlayCtrl.FolderEnable = TRUE;
					gPlayCtrl.FileNum = gPlayCtrl.File.FileNumInFolder;
				}
				else
				{
					gPlayCtrl.FolderEnable = FALSE;
					gPlayCtrl.FileNum = gPlayCtrl.File.FileNumInDisk;
				}
#ifdef FUNC_BREAK_POINT_EN
				BP_SetPlayAttrib();
#endif

#ifdef FUNC_DISP_EN
				DispRepeat(TRUE);
#endif
				break;

			//case MSG_EQ_SW:
			case MSG_EQ_CH_SUB:
				DBG(("EQ\n"));	
				gPlayCtrl.Eq++;
				if(gPlayCtrl.Eq >= DECD_EQ_SUM)
				{
					gPlayCtrl.Eq = DECD_EQ_NORMAL; 	
				}
				PlayEQ2Decd();			

#ifdef FUNC_BREAK_POINT_EN
				BP_SetPlayAttrib();
#endif

#ifdef FUNC_DISP_EN
				DispEQ(TRUE);
#endif
				break;
				
			case MSG_MUTE:				
#if 0//def FUNC_SPI_KEY_SOUND_EN
				if(gSys.MuteFg)
				{
					if(gPlayCtrl.State == PLAY_STATE_PLAY)
					{
						SPI_PlaySelectNum(SPIPLAY_SONG_UNMUTE, 1);
					}
					else
					{
						SPI_PlaySelectNum(SPIPLAY_SONG_UNMUTE, 0);
					}
				}
				else
				{	
					SPI_PlaySelectNum(SPIPLAY_SONG_MUTE, 0);
				}
#endif
				MuteStatusChange();	
				break;
	
			case MSG_NEXT:
			case MSG_NEXT1:
#ifdef	FUNC_DEVICE_FORCEPAUSE_EN
				if(gDevicePlayForcePauseFlag == TRUE) 
				{
					break;
				}					
#endif
				DBG1(("NEXT\n"));
#if 0//def FUNC_SPI_KEY_SOUND_EN
				SPI_PlaySelectNum(SPIPLAY_SONG_NEXT, 0);
#endif

#ifdef FUNC_DISP_EN
				DispDev();
#endif
#ifndef FUNC_FOLDER_EN
				if(gFsInfo.FileSum > 1)
#else
				if(((gPlayCtrl.FolderEnable == TRUE) && (gPlayCtrl.Folder.IncFileCnt > 1))
				|| ((gPlayCtrl.FolderEnable == FALSE) && (gFsInfo.FileSum > 1)))
#endif
				{
					SongPlayStop();
					GetNextSongNum(PLAY_DIRECT_NEXT);
					
#ifdef FUNC_BEEP_SOUND_EN	
					PushKeyBeep(1);
#endif
					gSys.MuteFg = FALSE;

#ifdef FUNC_OPEN_FILE_DELAY_EN
					TimeOutSet(&s_OpenFileDelayTimer, OPEN_FILE_DELAY_TIME);
					s_IsOpenFileDelay = TRUE;
#else

					if(!SongPlayInit())
					{
						MessageSend(MSG_FIFO_KEY, MSG_NEXT);
						gPlayCtrl.State = PLAY_STATE_STOP; 
					}		
#endif
				}
				else
				{		
#ifdef FUNC_OTP_PLAY_EN
					if(gSys.SystemMode == SYS_MODE_OTPDEV)
					{
						SongPlayStop();
						GetNextSongNum(PLAY_DIRECT_NEXT);
						gSys.MuteFg = FALSE;
						if(!SongPlayInit())
						{
							MessageSend(MSG_FIFO_KEY, MSG_NEXT);
							gPlayCtrl.State = PLAY_STATE_STOP; 
						}
						break;
					}
#endif
					DBG(("ONE S1\n"));
					DBG(("gPlayCtrl.FolderEnable:0x%bx, gFsInfo.ValidFolderSum:0x%x,gFsInfo.FileSum:0x%x\n", gPlayCtrl.FolderEnable, gFsInfo.ValidFolderSum, gFsInfo.FileSum));
					DBG(("gPlayCtrl.File.FolderNum:0x%x,gPlayCtrl.Folder.IncFileCnt:0x%x\n", gPlayCtrl.File.FolderNum, gPlayCtrl.Folder.IncFileCnt));
					//To do...
#ifdef FUNC_FOLDER_EN
					if(gPlayCtrl.FolderEnable == TRUE)
					{
						DBG(("**gFsInfo.ValidFolderSum:%u,FolderSum:%u\n", gFsInfo.ValidFolderSum, gFsInfo.FolderSum));
						if(gFsInfo.FolderSum > 1)
						{
							MessageSend(MSG_FIFO_KEY, MSG_NEXT_FOLDER);
							DBG(("*NxtFold\n"));
						}
#ifdef FUNC_UARTDBG_EN
						else
						{
							DBG(("*FoldSum = 1\n"));
						}
#endif	
					}
#else
					//MessageSend(MSG_FIFO_KEY, MSG_STOP);
#endif
				}
				break;
	
			case MSG_PRE:
			case MSG_PREV1:
#ifdef	FUNC_DEVICE_FORCEPAUSE_EN
				if(gDevicePlayForcePauseFlag == TRUE) 
				{
					break;
				}					
#endif
				DBG1(("PRE\n"));
#if 0//def FUNC_SPI_KEY_SOUND_EN
				SPI_PlaySelectNum(SPIPLAY_SONG_PREV, 0);
#endif

#ifdef FUNC_DISP_EN
				DispDev();
#endif
#ifndef FUNC_FOLDER_EN
				if(gFsInfo.FileSum > 1)
#else
				if(((gPlayCtrl.FolderEnable == TRUE) && (gPlayCtrl.Folder.IncFileCnt > 1) && (gPlayCtrl.File.FolderNum))
				|| ((gPlayCtrl.FolderEnable == FALSE) && (gFsInfo.FileSum > 1)))
#endif
				{					
					SongPlayStop();
					GetNextSongNum(PLAY_DIRECT_PRE);
					
#ifdef FUNC_BEEP_SOUND_EN	
					PushKeyBeep(1);
#endif					
					gSys.MuteFg = FALSE;

#ifdef FUNC_OPEN_FILE_DELAY_EN
					TimeOutSet(&s_OpenFileDelayTimer, OPEN_FILE_DELAY_TIME);
					s_IsOpenFileDelay = TRUE;
#else
					if(!SongPlayInit())
					{
						MessageSend(MSG_FIFO_KEY, MSG_PRE);
						gPlayCtrl.State = PLAY_STATE_STOP; 
					}	
#endif
				}
				else
				{
#ifdef FUNC_OTP_PLAY_EN
					if(gSys.SystemMode == SYS_MODE_OTPDEV)
					{
						SongPlayStop();
						GetNextSongNum(PLAY_DIRECT_PRE);
						gSys.MuteFg = FALSE;
						if(!SongPlayInit())
						{
							MessageSend(MSG_FIFO_KEY, MSG_PRE);
							gPlayCtrl.State = PLAY_STATE_STOP; 
						}
						break;
					}
#endif
					DBG(("ONE S2\n"));
					DBG(("gPlayCtrl.FolderEnable:0x%bx, gFsInfo.ValidFolderSum:0x%x,gFsInfo.FileSum:0x%x\n", gPlayCtrl.FolderEnable, gFsInfo.ValidFolderSum, gFsInfo.FileSum));
					DBG(("gPlayCtrl.File.FolderNum:0x%x,gPlayCtrl.Folder.IncFileCnt:0x%x\n", gPlayCtrl.File.FolderNum, gPlayCtrl.Folder.IncFileCnt));
					//To do...
#ifdef FUNC_FOLDER_EN
					if(gPlayCtrl.FolderEnable == TRUE)
					{
						DBG(("**gFsInfo.ValidFolderSum:%u,FolderSum:%u\n", gFsInfo.ValidFolderSum, gFsInfo.FolderSum));
						if(gFsInfo.FolderSum > 1)
						{
							MessageSend(MSG_FIFO_KEY, MSG_PRE_FOLDER);
							DBG(("**PrevFold\n"));
						}
#ifdef FUNC_UARTDBG_EN
						else
						{
							DBG(("**FoldSum = 1\n"));
						}
#endif	
					}
#else
					//MessageSend(MSG_FIFO_KEY, MSG_STOP);
#endif
				}
				break;

#ifdef	FUNC_NUMBER_KEY_EN
			case MSG_FREQUP_10TRK:
			case MSG_10TRACK_ADD:
			case MSG_10TRACK_ADDCP:	//10+
#ifndef FUNC_FOLDER_EN
				if(gFsInfo.FileSum > 1)
#else
				if(((gPlayCtrl.FolderEnable == TRUE) && (gPlayCtrl.Folder.IncFileCnt > 1) && (gPlayCtrl.File.FolderNum))
				|| ((gPlayCtrl.FolderEnable == FALSE) && (gFsInfo.FileSum > 1)))
#endif
				{					
					SongPlayStop();
					GetNextSongNum(PLAY_DIRECT_NEXT_10);
					gSys.MuteFg = FALSE;
					if(!SongPlayInit())
					{
						MessageSend(MSG_FIFO_KEY, MSG_NEXT);
						gPlayCtrl.State = PLAY_STATE_STOP; 
					}
				}
				else
				{
					DBG(("10+,FN<1\n"));
					//DBG(("FileSum:0x%x\n", gFsInfo.FileSum));
					//DBG(("Folder.IncFileCnt:0x%x\n", gPlayCtrl.Folder.IncFileCnt));
					//To do...
					//MessageSend(MSG_FIFO_KEY, MSG_STOP);
				}
				break;

			case MSG_FREQDN_10TRK:
			case MSG_10TRACK_SUB:
			case MSG_10TRACK_SUBCP:
#ifndef FUNC_FOLDER_EN
				if(gFsInfo.FileSum > 1)
#else
				if(((gPlayCtrl.FolderEnable == TRUE) && (gPlayCtrl.Folder.IncFileCnt > 1) && (gPlayCtrl.File.FolderNum))
				|| ((gPlayCtrl.FolderEnable == FALSE) && (gFsInfo.FileSum > 1)))
#endif
				{					
					SongPlayStop();
					GetNextSongNum(PLAY_DIRECT_PRE_10);
					gSys.MuteFg = FALSE;
					if(!SongPlayInit())
					{
						MessageSend(MSG_FIFO_KEY, MSG_PRE);
						gPlayCtrl.State = PLAY_STATE_STOP; 
					}
				}
				else
				{
					DBG(("10-,FN<1\n"));
					//DBG(("FileSum:0x%x\n", gFsInfo.FileSum));
					//DBG(("Folder.IncFileCnt:0x%x\n", gPlayCtrl.Folder.IncFileCnt));
					//To do...
					//MessageSend(MSG_FIFO_KEY, MSG_STOP);
				}
				break;

			case MSG_RECV_NUM:
			case MSG_NUM_SAVE_STAITON:
				DBG(("MP3 RECV_NUM\n"));
#ifdef FUNC_PICKSONG_WHOLE_DISK_EN
					DBG(("WHOLE DISK,0\n"));
					if((gFsInfo.FileSum >= gRecvNum) && (gRecvNum > 0))
#else
					if( ((gFsInfo.FileSum >= gRecvNum)  && (gRecvNum > 0) && (gPlayCtrl.FolderEnable == FALSE))
#ifdef FUNC_FOLDER_EN
					 || ((gPlayCtrl.FolderEnable == TRUE) && (gPlayCtrl.Folder.IncFileCnt >= gRecvNum))
#endif
						)
#endif
					{
#ifdef FUNC_PICKSONG_WHOLE_DISK_EN
						DBG(("WHOLE DISK,1\n"));
						if(gPlayCtrl.File.FileNumInDisk != gRecvNum)
#else
						if(gPlayCtrl.FileNum != gRecvNum)
#endif
						{
							DBG(("---MP3 RECV_NUM,RecvNum:%u,FileNum:%u,FileNumInDisk:%u\n", gRecvNum, gPlayCtrl.FileNum, gPlayCtrl.File.FileNumInDisk));
							
							SongPlayStop();
#ifdef FUNC_PICKSONG_WHOLE_DISK_EN
							gPlayCtrl.FolderEnable = FALSE;
#endif
								gPlayCtrl.FileNum = gRecvNum;
								gRecvNum = 0;
								if(!SongPlayInit())
								{
									MessageSend(MSG_FIFO_KEY, MSG_NEXT);
									gPlayCtrl.State = PLAY_STATE_STOP; 
								}
								DBG(("===MP3 RECV_NUM,RecvNum:%u,FileNum:%u,FileNumInDisk:%u\n", gRecvNum, gPlayCtrl.FileNum, gPlayCtrl.File.FileNumInDisk));
						}	
					}				
#ifdef FUNC_DISP_EN			
				if((gRecvNum > gFsInfo.FileSum) && (gRecvNum > 0))
				{
					TimeOutSet(&DispTmr, NORMAL_INTERVAL);	//按遥控确认键输入错误Err显示保留时间
				}
				else
				{
					TimeOutSet(&DispTmr, 0);	//退出数值显示
				}
				gRecvNum = 0;
#endif
				break;
#endif


#ifdef FUNC_FOLDER_EN
			case MSG_FOLDER_EN:
				if(gPlayCtrl.FolderEnable)
				{
					DBG(("FOLDER_DIS\n"));
					gPlayCtrl.FolderEnable = FALSE;
					gPlayCtrl.FileNum = gPlayCtrl.File.FileNumInDisk;
				}
				else
				{
					DBG(("FOLDER_EN\n"));
					gPlayCtrl.FolderEnable = TRUE;
					gPlayCtrl.FileNum = gPlayCtrl.File.FileNumInFolder;
				}	
#ifdef FUNC_BREAK_POINT_EN
				if(gSys.SystemMode == SYS_MODE_USB)	
				{		
					BP_SaveInfo(&gBreakPointInfo.PowerMemory.USB_FolderEnFlag, sizeof(gBreakPointInfo.PowerMemory.USB_FolderEnFlag));
				}
				else if(gSys.SystemMode == SYS_MODE_SD)
				{
					BP_SaveInfo(&gBreakPointInfo.PowerMemory.SD_FolderEnFlag, sizeof(gBreakPointInfo.PowerMemory.SD_FolderEnFlag));
				}							
#endif

#ifdef FUNC_DISP_EN
				DispFoldState();
#endif
				break;
	
			case MSG_NEXT_FOLDER:
				DBG(("NEXT_FOLDER\n"));	
				gPlayCtrl.FolderEnable = TRUE;
				
#ifdef FUNC_OPEN_FILE_DELAY_EN
				s_IsOpenFileDelay = FALSE;
#endif

				SongPlayStop();
				if(gPlayCtrl.FolderNum < gFsInfo.ValidFolderSum)
				{
					gPlayCtrl.FolderNum++;
				}
				else
				{
					gPlayCtrl.FolderNum = 1;
				}
	
				if(!FolderOpenByValidNum(&gPlayCtrl.Folder, NULL, gPlayCtrl.FolderNum))
				{
					DBG(("FolderOpenByValidNum() error!\n"));
					MessageSend(MSG_FIFO_KEY, MSG_NEXT_FOLDER);
					return;
				}
				DBG(("FolderOpenByValidNum(0x%lx, NULL, %u)\n", (DWORD)(&gPlayCtrl.Folder), gPlayCtrl.FolderNum));
				DBG(("gPlayCtrl.File.FolderNum:0x%x, gPlayCtrl.Folder.IncFileCnt:0x%x\n", gPlayCtrl.File.FolderNum, gPlayCtrl.Folder.IncFileCnt));
				
				gPlayCtrl.FileNum = 1;
				gSys.MuteFg = FALSE;
				if(!SongPlayInit())
				{
					DBG(("SongPlayInit error!\n"));
				 	MessageSend(MSG_FIFO_KEY, MSG_NEXT); 
				}
				DBG(("SongPlayInit OK!\n"));
				break;
	
			case MSG_PRE_FOLDER:
				DBG(("PRE_FOLDER\n"));	
				gPlayCtrl.FolderEnable = TRUE;	
				
#ifdef FUNC_OPEN_FILE_DELAY_EN
				s_IsOpenFileDelay = FALSE;
#endif	

				SongPlayStop();
				if(gPlayCtrl.FolderNum > 1)
				{
					gPlayCtrl.FolderNum--;
				}
				else
				{
					gPlayCtrl.FolderNum = gFsInfo.ValidFolderSum;
				}
	
				if(!FolderOpenByValidNum(&gPlayCtrl.Folder, NULL, gPlayCtrl.FolderNum))
				{
					DBG(("FolderOpenByValidNum() error!\n"));
					MessageSend(MSG_FIFO_KEY, MSG_PRE_FOLDER);
					return;
				}
				DBG(("FolderOpenByValidNum(0x%lx, NULL, %u)\n", (DWORD)(&gPlayCtrl.Folder), gPlayCtrl.FolderNum));
				DBG(("gPlayCtrl.File.FolderNum:0x%x, gPlayCtrl.Folder.IncFileCnt:0x%x\n", gPlayCtrl.File.FolderNum, gPlayCtrl.Folder.IncFileCnt));
	
				gPlayCtrl.FileNum = 1;
				gSys.MuteFg = FALSE;
				if(!SongPlayInit())
				{
				 	MessageSend(MSG_FIFO_KEY, MSG_NEXT); 
				}
				break;
#endif
	
#ifdef FUNC_FILE_BROWSER_EN
			case MSG_BROWSE_START:
				FileBrowseInit();
				SongPlayStop();
				gPlayCtrl.State = PLAY_STATE_BROWSE;
				gPlayCtrl.Event = MSG_NONE;
				break;
#endif

#ifdef FUNC_SPI_UPDATE_EN
			case MSG_UPDATE_FLASH:
				if(gIsMVFFileFind == 1)
				{
					BOOL  IsResume = FALSE;
					
					DBG(("Update MVF start....\n"));
					//保护现场
					//SectNum = gPlayCtrl.File.OpSec;					
					if (gPlayCtrl.State == PLAY_STATE_PLAY)
					{
						PlayStatePauseOp();
						IsResume = TRUE;
					}
					
					DownloadMVFToFsh();
					gIsMVFFileFind = 2;
					DBG(("Update MVF finish...\n"));
					
					//gPlayCtrl.File.OpSec = SectNum;
					//恢复播放
					if(IsResume == TRUE)
					{
						//PlayVolDecd(); //设置音量	
						PlayStatePlayOp();
					}				
				}
				break;
#endif
	
			default:			
				break;
		}
		
#ifdef FUN_SYSTEM_POWEROFF_WAIT_TIME
		SystemOffTimeMsgPro(gPlayCtrl.Event);
#endif
	}




	if(gPlayCtrl.State != PLAY_STATE_PLAY && !PowerOffTimeBeginFlag)
	{
		PowerOffTimeBeginFlag = TRUE;
		TimeOutSet(&SDCARDPOWEROFF_TIME, BT_BtPOWEROFF_TIME);
		SDCARDPowerOffTime_Start =	TRUE;
		DBG1(("begin !!!\n"));
		
	}

	if(gPlayCtrl.State == PLAY_STATE_PLAY && PowerOffTimeBeginFlag)
	{
		//TimeOutSet(&SDCARDPOWEROFF_TIME, 0);
		PowerOffTimeBeginFlag =	FALSE;
		SDCARDPowerOffTime_Start = FALSE;
		DBG1(("END!!!\n"));
	}

	
#if defined(AU6210K_ZB_BT007_CSR)
	if(SDCARDPowerOffTime_Start)
	if(IsTimeOut(&SDCARDPOWEROFF_TIME))
	{
		//关机流程
		DBG1(("power off doing\n"));

		if(gPlayCtrl.State != PLAY_STATE_PLAY )
		{
			
			WaitMs(2);
			SPI_PlaySelectNum(SPIPLAY_SONG_POWEROFF, 0);//关机提示音
			WaitMs(1000);
			SystemOff();
		}else
		{
			SDCARDPowerOffTime_Start = FALSE;
			TimeOutSet(&SDCARDPOWEROFF_TIME, 0);
		}
	}
#endif	

 	switch(gPlayCtrl.State)
	{
		case PLAY_STATE_PLAY:
			PlayStatePlayOp();
			break;

		case PLAY_STATE_PAUSE:
			PlayStatePauseOp();
			break;

		case PLAY_STATE_STOP:
			PlayStateStopOp();
			break;

#ifdef FUNC_FAST_PLAY_EN
		case PLAY_STATE_FF:			
		case PLAY_STATE_FB:			
			PlayStateFF_FBOp();
			break;
#endif
		
#ifdef FUNC_FILE_BROWSER_EN
		case PLAY_STATE_BROWSE:
			PlayStateBrowseOp();
			break;
#endif

		default:
			break;
	}	

	SongPlayTimeUpdate();
}


// Initialize playing control structure extend.
BOOL PlayCtrlInit(VOID)
{
	DBG(("PlayCtrlInit()\n"));
#ifndef FUNC_FOLDER_EN
		gPlayCtrl.FolderEnable = FALSE;
#else
		if(gPlayCtrl.FolderEnable > 1)
		{
			gPlayCtrl.FolderEnable = TRUE;	
		}	
#endif

#ifdef FUNC_OPEN_FILE_DELAY_EN
	s_IsOpenFileDelay = FALSE;
#endif
		
#ifdef FUNC_BREAK_POINT_EN
	if(IsBPValid() == TRUE)
	{		
		if(gIsFindLastestSong == FALSE)
		{
			DBG(("play frist song\n"));
			s_BreakPointFlag = FALSE;
			gPlayCtrl.FileNum = 1;
#ifdef FUNC_FOLDER_EN                           
			gPlayCtrl.FolderNum = 1;
#endif     
			//gPlayCtrl.Eq = DECD_EQ_NORMAL;
			gPlayCtrl.RepeatMode = REPEAT_ALL;
		}
		else if(gCurFileNum > 0)
		{
			s_BreakPointFlag = TRUE;
			if(gCurFileNum > gFsInfo.FileSum)
			{
				//DBG(("PlayCtrlInit,502\n"));
				gCurFileNum = 1;
			}
			if(gCurFolderNum > gFsInfo.FolderSum)
			{
				//DBG(("PlayCtrlInit,503\n"));
				gCurFolderNum = 1;
			}

#ifdef FUNC_FOLDER_EN
			if(gPlayCtrl.FolderEnable == TRUE)
			{ 
				gPlayCtrl.FileNum = gCurFileNumInFolder;
				gPlayCtrl.FolderNum = GetValidFolderNum(gCurFolderNum);
				DBG(("BP FoldNum:%u,ValidFoldNum:%u\n", gCurFolderNum, GetValidFolderNum(gCurFolderNum)));	
			}
			else
			{
				gPlayCtrl.FileNum = gCurFileNum;
				gPlayCtrl.FolderNum = 1;
			}
			
#else
			gPlayCtrl.FolderEnable = FALSE;
			gPlayCtrl.FileNum = gCurFileNum;
#endif

			//gPlayCtrl.Eq = gBreakPointInfo.Eq;	
			//gPlayCtrl.RepeatMode = gBreakPointInfo.RepeatMode;
			gPlayCtrl.RepeatMode = REPEAT_ALL;	//Repeat 模式不记忆
			
//			DBG(("gPlayCtrl.FolderEnable:%bu\n", gPlayCtrl.FolderEnable));
//			DBG(("gPlayCtrl.FileNum:%u, gPlayCtrl.FolderNum:%u\n", gPlayCtrl.FileNum, gPlayCtrl.FolderNum));
//			DBG(("gPlayCtrl.Eq:%bu\n", gPlayCtrl.Eq));
//			DBG(("gPlayCtrl.RepeatMode:%bu\n", gPlayCtrl.RepeatMode));
		}
	}
	else
	{
		s_BreakPointFlag = FALSE;
		gPlayCtrl.FolderNum = 1;
		gPlayCtrl.FileNum = 1;
		//gPlayCtrl.Eq = DECD_EQ_NORMAL;
		gPlayCtrl.RepeatMode = REPEAT_ALL;
		gSongInfo.CurPlayTime = 0;
		DBG(("BP InVald\n"));		
	}
#else
	DBG(("\n!BP\n\n"));
	s_BreakPointFlag = FALSE;
	gPlayCtrl.FolderNum = 1;
	gPlayCtrl.FileNum = 1;
	//gPlayCtrl.Eq = DECD_EQ_NORMAL;
	gPlayCtrl.RepeatMode = REPEAT_ALL;
	gSongInfo.CurPlayTime = 0;
#endif

	gPlayCtrl.Direction = PLAY_DIRECT_NEXT;

#ifdef FUNC_FOLDER_EN
	if((gSys.SystemMode != SYS_MODE_OTPDEV))
	{
		if(gPlayCtrl.FolderNum > gFsInfo.FolderSum)
		{
			gPlayCtrl.FolderNum = 1;
		}

		if(!FolderOpenByValidNum(&gPlayCtrl.Folder, NULL, gPlayCtrl.FolderNum))
		{
			DBG(("FolderOpenByValidNum() error!\n"));
			return FALSE;
		}
#ifdef	FUNC_LONG_NAME_EN		//长文件名功能														 
		{
			BYTE LongFileName[66];	 
			BOOL Ret;
			WORD* p;

			Ret = FolderGetLongName(&gPlayCtrl.Folder, LongFileName);
			DBG(("FolderGetLongName %d\n", (WORD)Ret));
			for(p = (WORD*)LongFileName; *p != 0; p++)
			{
				DBG(("%-.2BX %-.2BX ", ((BYTE*)p)[0], ((BYTE*)p)[1]));
			}
			DBG(("\n"));
		}
#endif

		if((gPlayCtrl.FolderEnable == TRUE) && (gPlayCtrl.FileNum > gPlayCtrl.Folder.IncFileCnt))
		{
			DBG(("PlayCtrlInit, 505\n"));
			gPlayCtrl.FileNum = 1;
		}
	}
#endif

	if(gFsInfo.FileSum == 0)
	{
		DBG(("No Song in Disk!!\n"));
		gPlayCtrl.State = PLAY_STATE_STOP;
		return TRUE;
	}

	//DBG(("gPlayCtrl.State11:%bx\n",gPlayCtrl.State));
	
	gPlayCtrl.State = PLAY_STATE_STOP;
#ifdef FUNC_AUTO_PLAY_EN
	if(!SongPlayInit())
	{
		MessageSend(MSG_FIFO_KEY, MSG_NEXT);	
	}
#endif
	//DBG(("gPlayCtrl.State22:%bx\n", gPlayCtrl.State));
//清标志
#ifdef FUNC_BREAK_POINT_EN	
	gCurFileNum = 0;
	gCurFileNumInFolder = 0;
	gCurFolderNum = 0;
#endif 

	return TRUE;
}


// Call this function before leaving play state.
BOOL PlayCtrlEnd(VOID)
{
	DBG(("PlayCtrlEnd()\n"));
	
	//关闭DAC	
	SongPlayStop();	

	return TRUE;
}

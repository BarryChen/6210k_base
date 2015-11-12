#include <reg51.h>
#include <string.h>
#include "type.h"
#include "syscfg.h"
#include "sysctrl.h"
#include "spi_flash.h"
#include "spi_fs.h"
#include "playctrl.h"
#include "file.h"
#include "utility.h"
#include "buffer.h"
#include "debug.h"
#include "sys_on_off.h"


#ifdef	FUNC_SPI_UPDATE_EN
BYTE gIsMVFFileFind = 0;
#endif
#if (defined(FUNC_SPI_UPDATE_EN) || defined(FUNC_SPI_KEY_SOUND_EN))
SPI_FS_CTRL	gSpiFsCtrl;	
FILE gUpdataFile;
#endif


#if (defined(FUNC_SPI_PLAY_EN) || defined(FUNC_SPI_KEY_SOUND_EN))
// SPI flash file system initial.
BOOL SpiFlashFsInit(VOID)
{
	DBG(("->SpiFlashFsInit\n"));
	//including calculate total number of respective file types, such as .mp3, .txt, .oth
	memset(&gSpiFsCtrl, 0, sizeof(gSpiFsCtrl));
	//gSpiFsCtrl.DevState = FALSE;
	if(SpiFlashModel == SpiFlash_None)
	{
		return FALSE;
	}
	SpiFlashBlkRd(0, (BYTE XDATA *)&gSpiFsCtrl.FsHead, sizeof(SPI_FS_HEAD));		//暂定为头部信息为32字节

	DBG(("SpiDevCustomerId=0x%lx\n", gSpiFsCtrl.FsHead.SpiDevCustomerId));
	DBG(("TotalFileNum=%d\n", gSpiFsCtrl.FsHead.TotalFileNum));
	DBG(("Mp3FileSum=%BU\n", gSpiFsCtrl.FsHead.Mp3FileSum));
	DBG(("TXTFileSum=%BU\n", gSpiFsCtrl.FsHead.TxtFileSum));
	DBG(("OTHFileSum=%BU\n", gSpiFsCtrl.FsHead.OthFileSum));
	
	if (gSpiFsCtrl.FsHead.SpiDevCustomerId == SPI_DEV_CUSTOMER_ID)				//compare a valid device customer ID
	{	
		DBG(("valid spi dev!\n"));
	//	gSys.FsCtrl.FileSum = gSpiFsCtrl.FsHead.Mp3FileSum;
	//	gSys.FsCtrl.Folder.Sum = 1;
	//	gSys.FsCtrl.Folder.CurrNum = 0;
		gSpiFsCtrl.DevState = TRUE;
	}
	else
	{
		DBG(("invalid spi dev!\n"));
		gSpiFsCtrl.DevState = FALSE;
		memset(&gSpiFsCtrl.FsHead, 0, sizeof(SPI_FS_HEAD));
	}

	gFsInfo.FileSum = gSpiFsCtrl.FsHead.TotalFileNum;
	
	gPlayCtrl.File.FileType = FILE_TYPE_MP3;
	//verify mvsilicon FS legal flag.
	DBG(("<-SpiFlashFsInit\n"));
	return TRUE;
}


// Open the specified file in SPI flash file system.
BOOL SpiFlashFileOpen(WORD FileNum, SPI_FILE_TYPE FileType)
{
	WORD	TempFileNum = 1;
	
	if (FileType > SPI_FILE_OTH)
	{
		return FALSE;
	} 

	if (FileType == SPI_FILE_MP3)
	{
		if (FileNum > gSpiFsCtrl.FsHead.Mp3FileSum)
		{
			return FALSE;
		}
		TempFileNum = (FileNum + gSpiFsCtrl.FsHead.OthFileSum);
	}
	else if (FileType == SPI_FILE_TXT)
	{
		if (FileNum > gSpiFsCtrl.FsHead.TxtFileSum)
		{
			return FALSE;
		}
		TempFileNum = (FileNum + gSpiFsCtrl.FsHead.OthFileSum + gSpiFsCtrl.FsHead.Mp3FileSum);
	}
	else
	{
		if (FileNum > gSpiFsCtrl.FsHead.OthFileSum)
		{
			return FALSE;
		}
		TempFileNum = FileNum;
	}

	gSpiFsCtrl.CurrFileNum = FileNum;
	gSpiFsCtrl.CurrFileType = FileType;
	gSpiFsCtrl.CurrOpLen = 0;

	SpiFlashBlkRd(MVFAT_START_ADDR + (TempFileNum - 1) * MVFAT_FILE_ENTRY_UNIT_SIZE, (BYTE XDATA *)&gSpiFsCtrl.CurrOpFile, sizeof(SPI_FILE_ENTRY)); 	//get file entry

	//DBG(("gSpiFsCtrl.CurrOpFile.StartAdd=%lx\n", gSpiFsCtrl.CurrOpFile.StartAdd));
	//DBG(("gSpiFsCtrl.CurrOpFile.TotalLen=%lx\n", gSpiFsCtrl.CurrOpFile.FileSize));
	
	if (FileType == SPI_FILE_MP3)
	{
		if(gSpiFsCtrl.CurrOpFile.FileSize == 0xffffffff)	
		{
			gSpiFsCtrl.CurrOpFile.FileSize = 0;
			return FALSE;
		}
		
#if (defined(FUNC_SPI_KEY_SOUND_EN))	
		memset(&gUpdataFile, 0, sizeof(FILE));
		//gSys.FsCtrl.CurrFileNum = fileNum;
		memset(&gUpdataFile.ShortName[0], '0', 4);
		memcpy(&gUpdataFile.ShortName[4], &gSpiFsCtrl.CurrOpFile.ucaFileName[0], 4);
		memcpy(&gUpdataFile.ShortName[8], &gSpiFsCtrl.CurrOpFile.ucaFileName[5], 3);

		gUpdataFile.Size = gSpiFsCtrl.CurrOpFile.FileSize;
		gUpdataFile.SecNum = (gSpiFsCtrl.CurrOpFile.FileSize + 511) / SPI_SECTOR_SIZE;
		gUpdataFile.StartSec = (gSpiFsCtrl.CurrOpFile.StartAdd - (MVFAT_START_ADDR + gSpiFsCtrl.FsHead.TotalFileNum * MVFAT_FILE_ENTRY_UNIT_SIZE)) / SPI_SECTOR_SIZE;
		gUpdataFile.OpSec = gUpdataFile.StartSec;
		gUpdataFile.OpSecCnt = 0;	
		gUpdataFile.FileType = FILE_TYPE_MP3;
		
		DBG(("gFile.Size=%lx\n", gUpdataFile.Size));
		DBG(("gFile.SecNum=%lx\n", gUpdataFile.SecNum));
		DBG(("gFile.OpSec=%lx\n", gUpdataFile.OpSec));
		DBG(("gPlayCtrl.File.FileType=%bu\n", gUpdataFile.FileType));
#endif

#if (defined(FUNC_SPI_PLAY_EN))	
		memset(&gPlayCtrl.File, 0, sizeof(FILE));
		//gSys.FsCtrl.CurrFileNum = fileNum;
		memset(&gPlayCtrl.File.ShortName[0], '0', 4);
		memcpy(&gPlayCtrl.File.ShortName[4], &gSpiFsCtrl.CurrOpFile.ucaFileName[0], 4);
		memcpy(&gPlayCtrl.File.ShortName[8], &gSpiFsCtrl.CurrOpFile.ucaFileName[5], 3);
		gPlayCtrl.File.Size = gSpiFsCtrl.CurrOpFile.FileSize;
		gPlayCtrl.File.SecNum = (gSpiFsCtrl.CurrOpFile.FileSize + 511) / SPI_SECTOR_SIZE;
		gPlayCtrl.File.StartSec = (gSpiFsCtrl.CurrOpFile.StartAdd - (MVFAT_START_ADDR + gSpiFsCtrl.FsHead.TotalFileNum * MVFAT_FILE_ENTRY_UNIT_SIZE)) / SPI_SECTOR_SIZE;
		gPlayCtrl.File.OpSec = gPlayCtrl.File.StartSec;
		gPlayCtrl.File.OpSecCnt = 0;	
		gPlayCtrl.File.FileType = FILE_TYPE_MP3;

		//	gSys.BP_ResumeFlag = FALSE;	
		DBG(("gFile.Size=%lx\n", gPlayCtrl.File.Size));
		DBG(("gFile.SecNum=%lx\n", gPlayCtrl.File.SecNum));
		DBG(("gFile.OpSec=%lx\n", gPlayCtrl.File.OpSec));
		DBG(("gPlayCtrl.File.FileType=%bu\n", gPlayCtrl.File.FileType));
#endif
	}
	
	return TRUE;	
}


// Read File data from the specified file.
WORD SpiFlashFileRead(WORD Buf, WORD Length)
{
	SpiFlashBlkRd((gSpiFsCtrl.CurrOpFile.StartAdd + gSpiFsCtrl.CurrOpLen), (BYTE XDATA*)Buf, Length);
	
	gSpiFsCtrl.CurrOpLen += Length;

	if (gSpiFsCtrl.CurrFileType == SPI_FILE_MP3)
	{
#if (defined(FUNC_SPI_PLAY_EN))			
		gPlayCtrl.File.OpSecCnt = (gSpiFsCtrl.CurrOpLen + 511) / SPI_SECTOR_SIZE;
		gPlayCtrl.File.OpSec = gPlayCtrl.File.StartSec + gPlayCtrl.File.OpSecCnt;
		DBG(("gPlayCtrl.File.OpSecCnt=%lx\n", gPlayCtrl.File.OpSecCnt));
		DBG(("gPlayCtrl.File.OpSec=%lx\n", gPlayCtrl.File.OpSec));
#endif

#if (defined(FUNC_SPI_KEY_SOUND_EN))
		gUpdataFile.OpSecCnt = (gSpiFsCtrl.CurrOpLen + 511) / SPI_SECTOR_SIZE;
		gUpdataFile.OpSec = gPlayCtrl.File.StartSec + gUpdataFile.OpSecCnt;
		DBG(("gUpdataFile.OpSecCnt=%lx\n", gUpdataFile.OpSecCnt));
		DBG(("gUpdataFile.OpSec=%lx\n", gUpdataFile.OpSec));
#endif
	}
	
	return Length;
}


VOID SpiFlashFileRewind(VOID)
{
	gSpiFsCtrl.CurrOpLen = 0;
}


VOID SpiFlashFileSeek(VOID)
{
#if (defined(FUNC_SPI_PLAY_EN))			
	gSpiFsCtrl.CurrOpLen = (gPlayCtrl.File.OpSec - gPlayCtrl.File.StartSec) * SPI_SECTOR_SIZE;
#endif

#if (defined(FUNC_SPI_KEY_SOUND_EN))			
	gSpiFsCtrl.CurrOpLen = (gUpdataFile.OpSec - gUpdataFile.StartSec) * SPI_SECTOR_SIZE;
#endif

	DBG(("gSpiFsCtrl.CurrOpLen:%lx\n", gSpiFsCtrl.CurrOpLen));
}
#endif


//======================================================================================================================
// #ifdef	FUNC_SPI_UPDATE_EN
//Download MVF to SPI flash
//
#ifdef	FUNC_SPI_UPDATE_EN
VOID DownloadMVFToFsh(VOID)
{
	WORD	Length;
	DWORD	Addr = 0;

	DBG(("->DownloadMVFToFsh\n"));
#ifdef FUNC_WATCHDOG_EN
	WatchDogDis();
#endif	
		
	DBG(("\nSpiFlashErase!\n"));
	SpiFlashErase();
	DBG(("\nUpdate start!\n"));

	while(!FileEOF(&gUpdataFile))
	{
		if((Length = FileRead(&gUpdataFile, PLAY_BUFFER_START, 512)) == 0)
		{
#ifdef FUNC_WATCHDOG_EN
			WatchDogEn();
#endif
			DBG(("DownloadMVFToFsh,ReadFile Failed!\n"));
			return ;
		}
		
		SPIFlashMultiBytePrgm(Addr, (BYTE XDATA *)DMA_XBUF_START, Length);
		Addr += Length;
		DBG(("%ld%%\n", (Addr * 100UL) / gUpdataFile.Size));
		WaitUs(10);
	}
	DBG(("\nUpdate OK!\n"));	

#ifdef FUNC_WATCHDOG_EN
	WatchDogEn();
#endif
	return ;
}
#endif	//#ifdef	FUNC_SPI_UPDATE_EN


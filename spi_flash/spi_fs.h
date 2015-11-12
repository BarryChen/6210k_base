#ifndef	__SPI_FLASH_FS_H__
#define __SPI_FLASH_FS_H__


#define		SPI_SECTOR_SIZE						512
#define		MVFAT_START_ADDR					32
#define		MVFAT_FILE_ENTRY_UNIT_SIZE			16
#define		MVFAT_FILENAME_START				8
#define		MVFAT_FILENAME_LEN					8
#define		MVFAT_FILEEXT_START					5

// define function: is SPI flash handle got.
#define		IsSpiFlashHandleGot()			(gSpiFsCtrl.DevState)
// Get MP3 file sum in SPI flash file system.
#define		SpiFlashMp3FileSumGet()			(gSpiFsCtrl.FsHead.Mp3FileSum)
#define		SpiFlashTxtFileSumGet()			(gSpiFsCtrl.FsHead.TxtFileSum)
#define		SpiFlashOthFileSumGet()			(gSpiFsCtrl.FsHead.OthFileSum)


// the SPI flash file system boot header. 
// note: a copy of this data structure must be stored at the 0x0000 address
//	of the SPI flash.
typedef	struct _SPI_FS_HEAD  							//cover 32 bytes from 0 address
{
	DWORD	SpiDevCustomerId;							//customer ID for comparing with a definition in syscfg.h
	DWORD	FlashSize;									//the max flash size
	WORD	TotalFileNum;								//total file number in SPI flash
	BYTE	Mp3FileSum;
	BYTE	TxtFileSum;
	BYTE	OthFileSum;
	BYTE    ucaRsv[19];
} SPI_FS_HEAD;




// the file entry structure in the SPI flash file system.
typedef struct _SPI_FILE_ENTRY
{
	DWORD	StartAdd;									//file entry addr
	DWORD	FileSize;									//file size in byte
	BYTE	ucaFileName[8];
} SPI_FILE_ENTRY;


// the file type in SPI flash FS.
typedef enum _SPI_FILE_TYPE
{
	SPI_FILE_MP3 = 0,
	SPI_FILE_TXT,
	SPI_FILE_OTH
} SPI_FILE_TYPE;


// the SPI FS control structure.
typedef struct _SPI_FS_CTRL
{
	SPI_FS_HEAD		FsHead;
	BYTE			DevState;
	BYTE			CurrFileNum;
	SPI_FILE_TYPE	CurrFileType;	
	SPI_FILE_ENTRY	CurrOpFile;
	DWORD			CurrOpLen;
} SPI_FS_CTRL;


extern	SPI_FS_CTRL		gSpiFsCtrl;

BOOL SpiFlashFsInit(VOID);
BOOL SpiFlashFileOpen(WORD FileNum, SPI_FILE_TYPE FileType);
WORD SpiFlashFileRead(WORD Buf, WORD Length);
VOID SpiFlashFileSeek(VOID);


//	
//Download MVF to SPI flash
//
#ifdef	FUNC_SPI_UPDATE_EN
extern VOID DownloadMVFToFsh(VOID);
#endif


#endif

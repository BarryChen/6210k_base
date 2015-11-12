#ifndef __BREAKPOINT_H__
#define __BREAKPOINT_H__

#include "radio_ctrl.h"
#include "rtc_ctrl.h"
#include "rtc.h"

#define BP_SIGNATURE	0x5A
#define MASK_BP_VOLUME	0x3F
#define MASK_BP_VOLFLAG	0x80

#define	EEPROM_ADDR_BP_START		0 
#define EEPROM_ADDR_BP_SIGNATURE	EEPROM_ADDR_BP_START
#define EEPROM_CHECK_READ			0
#define EEPROM_CHECK_WRITE_READ		1


extern BOOL gIsFindLastestSong;



typedef struct _BP_POWERMEM
{
	BYTE	Signature;			// Only when this field is equal to BP_SIGNATURE, this structure is valid.
	BYTE	SystemMode;			// system function mode.
	BYTE	Volume;				// Volume:0--32

	//USB
	BYTE	USB_FolderEnFlag;	// FolderEnFlag
	DWORD	USB_FileAddress;	// file Address --secNum
	BYTE	USB_FileNameCRC8;	// USB_FileNameCRC8 cherck
	DWORD   USB_CurPlayTime;	//play time
	
	//SD
	BYTE	SD_FolderEnFlag;	// FolderEnFlag
	DWORD	SD_FileAddress;		// file Address --secNum
	BYTE	SD_FileNameCRC8;	// SD_FileNameCRC8 cherck
	DWORD   SD_CurPlayTime;		//play time
	
#ifdef FUNC_RADIO_EN
	RADIO_DATA	RadioData2Store;
#endif

#ifdef FUNC_ALARM_EN
	AlARM_INFO	AlarmList2Store[MAX_ALARM_NUM];
#endif

	BYTE	EepRomCRC8;				//for E2prom data

} BP_POWERMEM;


typedef struct _BP_INFO
{
	BYTE		Eq;					// EQ
	BYTE		RepeatMode;			// Play mode
	
	BP_POWERMEM	PowerMemory;
				  					
} BP_INFO;


extern BP_INFO	gBreakPointInfo;
extern WORD  	gCurFileNum;
extern WORD	 	gCurFileNumInFolder;
extern WORD  	gCurFolderNum;
extern BOOL 	gIsEEPROMExist;

BOOL IsBPValid(VOID);

VOID BP_LoadInfo(VOID);

// Save break point information to EEPROM
VOID BP_SaveInfo(BYTE* Data, BYTE Size);

#ifdef FUNC_SAVE_PLAYTIME_EN
// Save break point information to EEPROM
VOID BP_SavePlayTime(BYTE* Data, BYTE Size);

VOID BP_UpdatePlayTime(VOID);
#endif

VOID BP_GetPlayAttrib(VOID);

VOID BP_SetPlayAttrib(VOID);

BYTE BP_GetSystemMode(VOID);

BYTE CRC8Cal(BYTE* Ptr, BYTE Len);

VOID FindLatestSong(VOID);

#endif

#ifndef __PARTITION_H__
#define __PARTITION_H__

#define SIGNATURE_OFFSET    	0x01fe            		// will contain 0x55AA
#define PARTITION_OFFSET    	0x01be            		// will contain partition table
#define COMMON_SIGNATURE    	0x55AA

//define partition structure
typedef struct _PARTITION         // Partition Entries definition    
{
    BYTE 	Active;
    BYTE 	StartHead;
    BYTE 	StartSector;
    BYTE 	StartCylinder;
    BYTE 	PartitionType;
    BYTE 	EndHead;
    BYTE 	EndSector;
    BYTE 	EndCylinder;
    DWORD 	FirstSectorNum;            
    DWORD 	SectorCount;

} PARTITION;


//define BPB structe
typedef struct
{
	BYTE 	BS_jmpBoot[3];		// 0-2
	BYTE	BS_OEMName[8];		// 3-10
	WORD	BPB_BytsPerSec;		// 11-12 
	BYTE	BPB_SecPerClus;		// 13
	WORD	BPB_RsvdSecCnt;		// 14-15
	BYTE	BPB_NumFATs;		// 16
	WORD	BPB_RootEntCnt;		// 17-18
	WORD	BPB_TotSec16;		// 19-20
	BYTE	BPB_Media;			// 21
	WORD	BPB_FATSz16;		// 22-23
	WORD	BPB_SecPerTrk;		// 24-25
	WORD	BPB_NumHeads;		// 26-27
	DWORD	BPB_HiddSec;		// 28-31
	DWORD	BPB_TotSec32;		// 32-35
	DWORD	BPB_FATSz32;		// 36-39
	WORD	BPB_ExtFlags;		// 40-41
	WORD	BPB_FSVer;			// 42-43
	DWORD	BPB_RootClus;		// 44-47
	WORD	BPB_FSInfo;			// 48-49
	WORD	BPB_BkBootSec;		// 50-51
	
} BOOT_SECTOR;


BOOL PartitionLoad(VOID);

#endif

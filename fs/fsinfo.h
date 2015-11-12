#ifndef __FSINFO_H__
#define __FSINFO_H__

#include <string.h>
#include "type.h"
#include "syscfg.h"


#define		SECTOR_SIZE			512		//bytes per sector


// Define device ID in system.
typedef	enum _DEV_ID
{//由于底层驱动会引用，所以显式声明的枚举变量值不能被修改.
	DEV_ID_NONE	= 0,
	DEV_ID_USB	= 1,
	DEV_ID_SD	= 2,
	DEV_ID_OTP	= 3,		
	DEV_ID_SPI	= 4,		
   	DEV_SUM

} DEV_ID;


// File system structure
typedef struct _FS_INFO
{//由于该结构体变量在底层驱动中使用，该结构体不能被修改.
	DEV_ID 	DevID;				//device id of this drive
	BOOL 	FAT32;				//is FAT32 file system
	BOOL	IsCpatFS;			//is compatalbe file system
	
	BOOL	IsFATChange;
	BOOL	IsDIRChange;
	
	DWORD	CurrFATSecNum;		//current FAT sector number
	DWORD	CurrDirSecNum;		//current directory sector number

	BYTE  	ClusterSize;		//The number of bytes of Cluster.    
	DWORD   FatStart;			//the sector number of FAT start
	DWORD  	RootStart;			//This is set to the cluster number of the first cluster of the root director.,
	DWORD  	DataStart;			//the first sector of data
	DWORD  	MaxCluster;			//max cluster number in this drive
	DWORD  	MaxSector;			//max sector number in this drive

	DWORD  	FreeClusNum;			

	WORD	FileSum;			//整个设备上的歌曲文件总数
	WORD	FileSumInFolder;	//文件系统内部使用		
	WORD	FolderSum;			//整个设备上的文件夹总数
	WORD	FolderNumInFolder;	//文件系统内部使用

	WORD	ValidFolderSum;		//整个设备上的非空文件夹总数

	WORD	RootFileSum;		//根文件夹下的文件个数

} FS_INFO;


extern FS_INFO	gFsInfo;


// 下面一组函数为文件系统内部使用。
VOID FSINFO_Init(VOID);
VOID FSINFO_SaveFdi(VOID);
VOID FSINFO_ChangeFolderNum(VOID);
VOID FSINFO_ChangeFileNum(VOID);
VOID FSINFO_InitOpenFolder(WORD StartFolderNum, WORD FolderNum);
BOOL FSINFO_FindLastNode(DWORD SecNum, BYTE Offset, BYTE Depth, BOOL RecFlag, BOOL RootFlag);
VOID FSINFO_InitOpenFile(WORD FileNum);


// 将某个文件夹标识设置为非空。
VOID ClrFolderEmptyFlag(WORD FolderNum);

// 判断某个文件夹是否为空。
BOOL IsFolderEmpty(WORD FolderNum);

// 根据原始文件夹号计算有效文件夹号（滤除空文件夹后的序号）。
WORD GetValidFolderNum(WORD OriginalFolderNum);

// 根据有效文件夹号（滤除空文件夹后的序号）计算原始文件夹号。
WORD GetOriginalFolderNum(WORD ValidFolderNum);

// 计算非空文件夹数目
//WORD GetValidFolderSum(VOID);


#endif

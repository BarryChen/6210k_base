#ifndef __FAT_H__
#define __FAT_H__


// Define EOF flag for FAT16 and FAT32
#define EOF_FAT32				0x0FFFFFF8
#define	EOF_FAT16				0xFFF8


// Get the sector number
DWORD GetSecNum(DWORD clusNum);

// Get the cluster number
DWORD GetClusNum(DWORD secNum);

// Load one directory sector
BOOL ReadDirSec(DWORD secNum);
	
// Load one FAT sector
BOOL ReadFATSec(DWORD secNum);

// Get next cluster number
DWORD GetNextClusNum(DWORD ClusNum);

// Get next sector number
DWORD GetNextSecNum(DWORD SecNum);

// 更改FAT表项
//BOOL ModifyFatItem(DWORD ClusNum, DWORD WriteData);

// 判断簇是否空闲
//BOOL IsFreeClus(DWORD ClusNum);

// 计算当前簇内剩余的空闲扇区个数
//BYTE FreeSectorOfClus(DWORD CurSectorNum);

// 分配一个蔟
//DWORD FreeClusAlloc(DWORD StartClusNum);

// 删除一个簇链
//VOID DeleteClusterChain(DWORD StartClusNum);

// 从一个簇链的当前位置向后跳过若干个簇
//BOOL SeekClusterChain(DWORD StartClusNum, DWORD ClusCnt);

#endif

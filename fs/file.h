#ifndef __FILE_H__
#define __FILE_H__


#include "folder.h"


//define file seek position.
#define	SEEK_FILE_SET				0					//seek file from the first sector of file
#define	SEEK_FILE_CUR				1					//seek file from the current sector of file
#define	SEEK_FILE_END				2					//seek file from the last sector of file 

#define	MAX_FILE_LONGNAME_SIZE		64
#define	MAX_LONGNAME_BUFFSIZE		(MAX_FILE_LONGNAME_SIZE + 2)

// File system control error type.
typedef enum _FS_ERROR
{
	FS_ERROR_NONE = 0,
	FS_ERROR_PARTITION_LOAD,
	FS_ERROR_FILE_LIST_CREATE,
	FS_ERROR_NO_FILE,
	FS_ERROR_FILE_CMPT,
	FS_ERROR_FILEOPEN_FAILED,
	FS_ERROR_FOLDOPEN_FAILED,
	FS_ERROR_ANALYSIS_FAILED,
	FS_ERROR_READ_FAILED,
	FS_ERROR_INVALID_ADDRESS,
	FS_ERROR_SUM

} FS_ERROR;

typedef enum _FILE_TYPE
{
	FILE_TYPE_UNKNOWN = 0,
	FILE_TYPE_MP3,

} FILE_TYPE;


//文件的描述结构
typedef struct _FILE
{
	WORD	FileNumInDisk;			//在整个设备上的文件号
	WORD	FileNumInFolder;		//在当前文件夹中的文件号
	WORD	FolderNum;				//所在文件夹的编号
//	WORD	ValidFolderNum;			//所在文件夹的有效文件夹编号

	DWORD	ParentFoldSect;			//文件父目录扇区号
	DWORD	DirSecNum;				//文件目录项存储位置扇区号
	BYTE	DirOffset;				//文件目录项在所在扇区中的偏移量

	BYTE	ShortName[11];			//短文件名

	BYTE	FileType;				//文件夹类型

//	DWORD	StartClusNum;			//文件第一个簇号

	DWORD	StartSec;				//文件第一个扇区号
	DWORD	SecNum;					//文件占用的扇区总数
	DWORD	OpSec;					//当前要读写的扇区号
	DWORD	OpSecCnt;				//当前位置在文件中偏移扇区数
	DWORD	Size;					//文件的总字节数

} FILE;


extern FS_ERROR gFsError;

// 判断文件的类型，由文件系统底层调用
BYTE FileGetType(VOID);

// 读文件数据到缓冲区
// Len长度必须为512字节整数倍
WORD FileRead(FILE* File, WORD Buffer, WORD Len);

// 判断文件是否结束
BOOL FileEOF(FILE* File);

// 返回到文件开头
VOID FileRewind(FILE* File);

// 文件指针定位
BOOL FileSeek(FILE* File, SDWORD Offset, BYTE Base);

// 按文件序号打开文件
// 打开成功：返回TRUE，目标文件信息填充到File所指的结构体。
// 打开失败：返回FALSE。
// Folder == NULL: 打开整个设备上的第FileNum个文件。
// Folder != NULL: 打开Folder文件夹中的第FileNum个文件夹。
BOOL FileOpenByNum(FILE* File, FOLDER* Folder, WORD FileNum);

// 按文件名称打开指定文件夹中的文件。
// 打开成功：返回TRUE，目标文件信息填充到File所指的结构体。
// 打开失败：返回FALSE。
// Folder指针不能为空指针。
BOOL FileOpenByName(FILE* File, FOLDER* Folder, BYTE* FileName);

// 获取指定文件的长文件名。
// 长文件名最长为66个字节，所以LongFileName[]数组至少要有66个字节，否则将会导致不可预料的错误。
// 获取到长文件名，返回TRUE。
// 无长文件名，返回FALSE。
BOOL FileGetLongName(FILE* File, BYTE* LongFileName);	//LongFileName[]: 66 Bytes


#endif

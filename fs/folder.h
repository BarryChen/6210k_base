#ifndef __FOLDER_H__
#define __FOLDER_H__


// Folder info structure define.
typedef struct _FOLDER
{	
	WORD			FolderNum;							//文件夹号
	WORD			ValidFolderNum;						//有效文件夹号

   	DWORD			ParentFoldSect;						//Folder父目录扇区号
	DWORD			DirSecNum;							//Folder目录项存储位置扇区号
	BYTE			DirOffset;							//Folder目录项在扇区中的偏移量

	DWORD			EndDirSecNum;						//文件夹中最后一个文件目录项所在扇区号
	BYTE			EndDirOffset;						//文件夹中最后一个文件目录项所在扇区中的偏移量

	DWORD			StartSecNum;					   	//folder存储位置首扇区
	BYTE			ShortName[9];						//folder short name

	WORD			StartFileNum;						//for calculate absolute file number in whole partition.

	WORD			IncFileCnt;							//include file sum of the folder, not recursion
	WORD			IncFolderCnt;						

	WORD			RecFileCnt;							//recursion file sum of the current folder
	WORD			RecFolderCnt;
		
} FOLDER;


// 按文件夹序号打开文件夹
// 打开成功：返回TRUE，目标文件夹信息填充到Folder所指的结构体。
// 打开失败：返回FALSE。
// ParentFolder == NULL: 打开整个设备上的第FolderNum个文件夹。
// ParentFolder != NULL: 打开ParentFolder文件夹中的第FolderNum个文件夹。
BOOL FolderOpenByNum(FOLDER* Folder, FOLDER* ParentFolder, WORD FolderNum);

// 按文件夹有效序号（滤除空文件夹后的序号）打开文件夹。
// 函数功能类似于FolderOpenByNum()。
BOOL FolderOpenByValidNum(FOLDER* Folder, FOLDER* ParentFolder, WORD ValidFolderNum);

// 按文件夹名称打开文件夹。
// 打开成功：返回TRUE，目标文件夹信息填充到Folder所指的结构体。
// 打开失败：返回FALSE。
// ParentFolder == NULL: 打开根目录中的FolderName文件夹。
// ParentFolder != NULL: 打开ParentFolder文件夹中的FolderName文件夹。
// FolderName[]长度不大于8字节，根目录名称为"\\"
BOOL FolderOpenByName(FOLDER* Folder, FOLDER* ParentFolder, BYTE* FolderName);

// 获取指定文件夹的长文件名。
// 长文件名最长为66个字节，所以LongFileName[]数组至少要有66个字节，否则将会导致不可预料的错误。
// 获取到长文件名，返回TRUE。
// 无长文件名，返回FALSE。
BOOL FolderGetLongName(FOLDER* Folder, BYTE* LongFileName);	//LongFileName[]: 66 Bytes


#endif

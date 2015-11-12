#include "syscfg.h"
#include "file.h"
#include "debug.h"
#include "spi_fs.h"
#include "playctrl.h"
#include "breakpoint.h"

FS_ERROR gFsError;
extern WORD gFileRdSize;
BYTE gFsInitOK = 0;

#ifdef	FUNC_SPI_UPDATE_EN
extern BYTE gIsMVFFileFind;
extern FILE gUpdataFile;
#endif

#ifdef FUNC_QUICK_RESPONSE_EN
extern BOOL IsSwitchMode;
extern BOOL QuickResponse(VOID);
extern VOID KeyEventGet(VOID);
#endif

#ifdef FUNC_ENCRYPT_DECODE_EN
BOOL gIsEncryptFile = FALSE;
#endif

#ifdef FUNC_ENCRYPT_DECODE_EN
BOOL IsEncryptSong(VOID)
{
    return 	gIsEncryptFile;
}
#else
BOOL IsEncryptSong(VOID)
{
    return FALSE;
}
#endif




//判断文件的类型，由文件系统底层调用
BYTE FileGetType(VOID)
{
	BYTE ExtFileName[3];
	BOOL Ret;

	ExtFileName[0] = gCurrentEntry->FileName[8];
	ExtFileName[1] = gCurrentEntry->FileName[9];
	ExtFileName[2] = gCurrentEntry->FileName[10];

	if((ExtFileName[0] == 'M') && (ExtFileName[1] == 'P') && (ExtFileName[2] == '3'))
	{
		Ret = FILE_TYPE_MP3;
#ifdef FUNC_ENCRYPT_DECODE_EN
		gIsEncryptFile = FALSE;
#endif
		
	}
#ifdef FUNC_ENCRYPT_DECODE_EN
	else if(memcmp(ExtFileName, ENCRYPT_EXT, 3) == 0)
	{
	    Ret = FILE_TYPE_MP3;
		gIsEncryptFile = TRUE;//加密文件 
	}
#endif
	
	else
	{
		//DBG(("FILE_TYPE_UNKNOWN!!\n"));
		Ret = FILE_TYPE_UNKNOWN;
	}

#ifdef FUNC_BREAK_POINT_EN	
	//匹配记录歌曲相关信息，寻找断电记忆歌曲
	if((gFsInitOK == 0) && (Ret != FILE_TYPE_UNKNOWN))
	{
		FindLatestSong();	
	}
#endif

	return Ret;
}


//统计文件和文件夹总数
BOOL PreSearch(VOID)
{
	ENTRY_TYPE EntryType;

	//DBG(("PreSearch start!\n"));
	
#ifdef FUNC_HIDDEN_FOLD_EN
	HiddenFolderEnable = TRUE;		//播放隐藏文件夹中的歌曲	
#else
	HiddenFolderEnable = FALSE;		//忽略隐藏文件夹中的歌曲	 
#endif

#ifdef FUNC_RECYCLED_FOLD_EN
	RecycledFolderEnable = TRUE;	//播放回收站文件夹中的歌曲	
#else
	RecycledFolderEnable = FALSE;	//忽略回收站文件夹中的歌曲
#endif

	DirSetStartEntry(gFsInfo.RootStart, 0, TRUE);
	FSINFO_Init();

	gFolderDirStart = gFsInfo.RootStart;

	while(1)
	{
		FeedWatchDog();
			
		EntryType = DirGetNextEntry();

		if(gFsInfo.FileSum > 65534)
		{
			break;	
		}
		if(gEntryOffset == 0)
		{	
			FSINFO_SaveFdi();			
#ifdef FUNC_QUICK_RESPONSE_EN
			//强行退出
			KeyEventGet();
			if(QuickResponse())
			{
				return FALSE;	
			}
#endif
		}

		switch(EntryType)
		{
			case ENTRY_FILE: 	//是歌曲
				if(FileGetType() != FILE_TYPE_UNKNOWN)
				{
					//DBG(("找到歌曲：%-.8s%-.3s\n", &gCurrentEntry->FileName[0], &gCurrentEntry->FileName[8])); 
					gFsInfo.FileSum++;
					gFsInfo.FileSumInFolder++;
					//DBG(("\n******gFsInfo.FileSum:%u, gFsInfo.FileSumInFolder:%u\n",gFsInfo.FileSum,gFsInfo.FileSumInFolder));
#ifdef FUNC_BREAK_POINT_EN
					if((gIsFindLastestSong == TRUE) && (gCurFileNum == 0))
					{				
						gCurFileNum = gFsInfo.FileSum; //记录歌曲号
						gCurFileNumInFolder = gFsInfo.FileSumInFolder; //记录歌曲号
						gCurFolderNum = gFsInfo.FolderSum; //记录文件夹号
						//DBG(("\n******gPlayCtrl.FolderEnable:%bu, gCurFileNum:%u, gCurFileNumInFolde:%u, gCurFolderNum:%u\n",gPlayCtrl.FolderEnable, gCurFileNum, gCurFileNumInFolder, gCurFolderNum));
					}
#endif
				}

#ifdef FUNC_SPI_UPDATE_EN
				//检查是否为SPI FLASH升级文件：*.MVF
				//只检查根目录
				if((gFsInfo.FolderSum == 1) && (gIsMVFFileFind != 1))
				{
					if(memcmp(&gCurrentEntry->FileName[8], "MVF", 3) == 0)
					{
						extern BOOL FileGetInfo(FILE* File);

						gIsMVFFileFind = 1;
						FileGetInfo(&gUpdataFile);
					}	
				}	
#endif
				break;

			case ENTRY_FOLDER:	//是文件夹，下次循环不会进入这个文件夹内
				gFsInfo.FolderNumInFolder++;
				//DBG(("ENTRY_FOLDER: %d\n", gFsInfo.FolderNumInFolder));
				break;
			
			case ENTRY_END:		//返回上一级文件夹，调整之前记录的部分节点
				FSINFO_ChangeFileNum();

				//记录根文件夹中的文件个数
				if(gFsInfo.FolderSum == 1)
				{
				 	gFsInfo.RootFileSum = gFsInfo.FileSum;
				}

				//回到当前文件夹的开头位置
				//DBG(("开始查找下一个文件夹的位置！\n"));
				RewindFolderStart(); 	
				
				//从当前位置，在整个文件系统中寻找下一个文件夹
				if(!FindNextFolder())	
				{	
					//DBG(("PreSearch end! %d ms \n", (WORD)PastTimeGet(&Timer))); 
					DBG(("PreSearch end!\n"));
					return TRUE;
				}
				else	//进入下一个文件夹成功
				{
					//DBG(("找到下一个文件夹，即将进入该文件夹！%-.8s\n", &gCurrentEntry->FileName[0]));
					gFsInfo.FolderSum++;
					gFsInfo.FolderNumInFolder = 0;
					gFsInfo.FileSumInFolder = 0;
					if(!DirEnterSubFolder())
					{
						DBG(("进入子文件夹失败！\n"));
						gFsInfo.FolderSum--;
					}					
					//DBG(("进入子文件夹成功！\n"));
				}
				break;

			default:
				break;
		}	
	}

	return TRUE;
}


//文件系统初始化，分析分区表，预搜索统计文件系统有关信息.
BOOL FSInit(BYTE DeviceID)
{						
	memset(&gFsInfo, 0, sizeof(FS_INFO));
	gFsInfo.DevID = DeviceID;
	gFsInfo.CurrFATSecNum = -1;
	gFsInfo.CurrDirSecNum = -1;
	gFsError = FS_ERROR_NONE;
	gFsInitOK = 0;
	gFileRdSize = 512;

	DBG(("gFileRdSize:0x%x\n", gFileRdSize));
	DBG((">>FsInit(%bd)\n", DeviceID));

#ifdef FUNC_OTP_PLAY_EN
	if(gFsInfo.DevID == DEV_ID_OTP)
	{
		extern BOOL OtpFsInit(VOID);
		DBG(("OtpFsInit()\n"));
		return OtpFsInit();
	}
#endif

#ifdef FUNC_SPI_PLAY_EN
	if(gFsInfo.DevID == DEV_ID_SPI)
	{
		extern BOOL SpiFlashFsInit(VOID);
		DBG(("SpiFlashFsInit()\n"));
		return SpiFlashFsInit();
	}
#endif

	if(!PartitionLoad())
	{
		DBG(("PartitionLoad() error!\n"));
		gFsError = FS_ERROR_PARTITION_LOAD;
		gFsInitOK = 2;	//?????????????????????????????
		return FALSE;
	}	
	DBG(("PartitionLoad() OK!\n"));

#ifdef FUNC_BREAK_POINT_EN
	//先对gIsFindLastestSong清零一次
	gIsFindLastestSong = FALSE;
#endif	

	if(!PreSearch())
	{
		DBG(("PreSearch() error!\n"));
		gFsError = FS_ERROR_FILE_LIST_CREATE;
		gFsInitOK = 3;	//?????????????????????????????
		return FALSE;
	}
	DBG(("PreSearch() OK\n"));

	if(gFsInfo.FileSum == 0)
	{
		gFsError = FS_ERROR_NO_FILE;
	}

	DBG(("Song sum in disk: %U\n", gFsInfo.FileSum));
	DBG(("Folder sum in disk: %U\n", gFsInfo.FolderSum));
	DBG(("Valid folder sum in disk: %U\n", gFsInfo.ValidFolderSum));
	DBG(("***********************************************\n"));
	DBG(("\n"));
	gFsInitOK = 1;
	
#ifdef FUNC_SPI_UPDATE_AUTO_EN
	if(gIsMVFFileFind)
	{
		DownloadMVFToFsh();
	}
#endif

	return TRUE;
}
				

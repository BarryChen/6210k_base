#ifndef __OTP_FS_H__
#define __OTP_FS_H__

#include <reg51.h>
#include "type.h"

#define  OTP_MAX_SONG_NUM     1

typedef struct _OTP_FILE_ENTRY
{
	BYTE CODE *StartDataPtr; //MP3数据指针
	WORD DataLen;		     //MP3数据长度

} OTP_FILE_ENTRY;

typedef  struct _OTP_FILE_HANDLE
{
	BYTE CurrFileNum;
	BYTE CODE *StartDataPtr; //MP3起始数据指针
	WORD DataLen;		     //MP3数据长度
	BYTE CODE *CurrDataPtr;	 //MP3当前数据指针
	WORD RemainBytes;		 //还剩下没有读取的字节数

} OTP_FILE_HANDLE;



BOOL OtpFileOpen(WORD FileNum);
WORD OtpFileRead(WORD Addr, WORD Len);
BOOL IsEndOfFile(VOID);

#endif



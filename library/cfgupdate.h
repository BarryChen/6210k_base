#ifndef __CFGUPDATE_H__
#define __CFGUPDATE_H__

typedef struct _CORRECT_INFO
{
	BYTE ValidFlg;      //纠错信息有效标志应该为0xAA
	WORD StartAddr;		//2Byte, OTP坏块的起始地址
	BYTE Dat[5];		//记录的坏块处的正确信息，该部分的大小需要根据OTP中坏块的统计情况进行修改
} CORRECT_INFO;



BOOL NeedCfgUpdate(VOID);

BOOL CfgInfoUpdate(BYTE* PCfgBuf);

BOOL CfgFileValidCheck(BYTE* PCfgBuf);

VOID DispUpdateError(VOID);

#endif


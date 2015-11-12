#ifndef __CARD_H__
#define __CARD_H__


typedef struct _CARD_INFO
{  
	BYTE	MID;			// Manufacturer ID
	BYTE	OID[2];			// OEM/Application ID
	BYTE	PNM[5];			// Product name
	BYTE	PRV;			// Product revision
	BYTE	PSN[4];			// Product serial number
	WORD	Rsv : 4;		// reserved
	WORD	MDT : 12;		// Manufacturing date
	BYTE	CRC : 7;		// CRC7 checksum
	BYTE	NoUse : 1;		// not used, always 1

} CARD_INFO;


extern CARD_INFO	gLibCardInfo;


#define	SD_CLK_6MHZ		0x00
#define	SD_CLK_3MHZ		0x01
#define	SD_CLK_1P5MHZ	0x02
#define SD_CLK_750KHZ	0x03
#define SD_CLK_375KHZ	0x04
#define SD_CLK_187KHZ	0x05
#define SD_CLK_94KHZ	0x06
#define SD_CLK_OFF		0x10

// If the host wants to stop the clock, poll busy bit by ACMD41 command at less than 50 ms intervals
// #define	SD_CMD_TIMEOUT	50 	
// Setting Clock rate of SD bus.
// 设定频率除以系统时钟分配比才是实际的频率(Clock=Clk/SysDiv).
VOID SD_SetClock(BYTE Clk);

// SD mode initialize card.
VOID CardOpen(VOID);

// Polling card insert and pull out.
BOOL CardInit(VOID);							

// Read card stor block
BOOL ReadCardStorBlock(DWORD Block, WORD Buffer, BYTE Size);

// Write card stor block.
BOOL WriteCardStorBlock(DWORD Block, WORD Buffer, BYTE Size);

// Card detect, to check card whether insert or pull out.
BOOL CardDetect(VOID);

// 获取卡的BLOCK总数
DWORD CardGetBlockNum(VOID);

#endif


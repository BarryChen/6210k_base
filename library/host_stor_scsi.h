#ifndef __HOST_STOR_H__
#define	__HOST_STOR_H__


//	Mass Storage device structure
typedef struct _STORAGE
{
	BYTE			MaxLUN;
	BYTE			LuNum;
	WORD			BlockSize;
	DWORD			LastLBA;
		
} STORAGE;


//	command block wrapper
typedef struct _CBW
{
	DWORD		Signature;
	DWORD		Tag;
	DWORD		DataTransferLength;
	BYTE		Flags;
	BYTE		Lun;
	BYTE		Length;
	BYTE		CDB[16];
} CBW;

#define US_BULK_CB_SIGN			0x43425355	//spells out USBC
#define US_BULK_FLAG_IN			0x80
#define US_BULK_FLAG_OUT		0x00

// command status wrapper
typedef struct _CSW
{
	DWORD		Signature;
	DWORD		Tag;
	DWORD		Residue;
	BYTE		Status;
} CSW;

#define US_BULK_CS_SIGN			0x53425355	// spells out 'USBS'
#define US_BULK_STAT_OK			0
#define US_BULK_STAT_FAIL		1
#define US_BULK_STAT_PHASE		2


// SCSI opcodes
#define TEST_UNIT_READY       0x00
#define REQUEST_SENSE  		  0x03
#define READ_6                0x08
#define WRITE_6               0x0a
#define SEEK_6                0x0b
#define INQUIRY               0x12
#define MODE_SELECT           0x15
#define MODE_SENSE            0x1a
#define START_STOP_UNIT       0x1b
#define ALLOW_MEDIUM_REMOVAL  0x1e
#define	READ_FORMAT_CAPACITY  0x23
#define READ_CAPACITY         0x25
#define READ_10               0x28
#define WRITE_10              0x2a
#define SEEK_10               0x2b
#define VERYFY                0x2f
#define MODE_SELECT_10        0x55
#define MODE_SENSE_10         0x5a
#define READ_12               0xa8
#define WRITE_12              0xaa
#define READ_DISC_STRUCTURE   0xad
#define DVD_MACHANISM_STATUS  0xbd
#define REPORT_KEY			  0xa4
#define SEND_KEY			  0xa3
#define READ_TOC			  0x43
#define READ_MSF			  0xb9



// scsi command
//typedef struct _SCSI_CMD
//{
//	BYTE		Cmd;
//	BYTE		CmdLen;
//	BYTE		Lun;
//	BYTE		DataDirection;
//	VOID		*Buf;
//	WORD		BufLen;
//	DWORD		Lba;
//	WORD		BlockSize;
//} SCSI_CMD;


//VOID StorOpen(VOID);
BOOL StorBulkTransport(BYTE* Buf, WORD Len);
BOOL StorStartDevice(VOID);



#endif

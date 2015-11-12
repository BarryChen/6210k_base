#ifndef __HOST_H__
#define	__HOST_H__

#define		MAX_USB_SN_LEN		50

typedef struct _USB_INFO
{  
	WORD	VID;
	WORD	PID;
	BYTE	LenSN;	//length of SN 
	BYTE*	pSN;

} USB_INFO;

extern USB_INFO		gLibUsbInfo;

// init usb host module
VOID HostInit(VOID);

//check usb link status
BOOL HostIsLink(VOID);

//init usb device
BOOL HostStorInit(VOID);

//check usb link status
BOOL HostUsbPortLink(VOID);

//usb host read storage block
BOOL HostReadBlock(DWORD Lba, VOID* Buf, BYTE Size);

//get storage device block size
WORD HostGetBlockSize(VOID);

//get storage device last lba number
DWORD HostGetLastLBA(VOID);



#endif

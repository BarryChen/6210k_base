#ifndef __DEVICE_STOR_H__
#define	__DEVICE_STOR_H__

typedef enum _READER_STATE
{
	READER_INIT = 0,
	READER_IDLE = 1,
	READER_COPY = 2

} READER_STATE;

extern READER_STATE gReaderState;

//mass-storage device initial
VOID DeviceStorInit(VOID);

//mass-storage device process
VOID DeviceStorProcess(VOID);

BOOL DeviceStorIsPrevent(VOID);

#endif

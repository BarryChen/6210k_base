#ifndef __DEVICE_HCD_H__
#define	__DEVICE_HCD_H__


// Max packet size. Fixed, user should not modify.
#define	DEVICE_FS_CONTROL_MPS		64		
#define	DEVICE_FS_BULK_IN_MPS		64		
#define	DEVICE_FS_BULK_OUT_MPS		64		
#define	DEVICE_FS_INT_IN_MPS		64		
#define	DEVICE_FS_ISO_OUT_MPS		192		


// Endpoint number. Fixed, user should not modify.
#define	DEVICE_CONTROL_EP			0x00
#define	DEVICE_BULK_IN_EP			0x81
#define	DEVICE_BULK_OUT_EP			0x02
#define	DEVICE_INT_IN_EP			0x83
#define	DEVICE_ISO_OUT_EP			0x04


// Bus event define
#define	USB_VBUS_ERROR				(1 << 7)
#define	USB_SESSION_REQ				(1 << 6)
#define	USB_DISCONNECT				(1 << 5)
#define	USB_CONNECT					(1 << 4)
#define	USB_SOF						(1 << 3)
#define	USB_RESET					(1 << 2)
#define	USB_RESUME					(1 << 1)
#define	USB_SUSPEND					(1 << 0)


// 检查系统是否连接了一个USB主机
BOOL UsbDeviceIsLink(VOID);

// Device initial
VOID UsbDeviceInit(VOID);

// Device software disconnect.
VOID UsbDeviceDisConnect(VOID);


VOID SetChipWorkOnAudioMode(VOID);

// Send stall
VOID UsbDeviceSendStall(BYTE EndpointNum);

// Set receive endpoint to ISO mode.
VOID UsbDeviceSetRcvISO(VOID);

// Reset endpoint
VOID UsbDeviceResetEndpoint(BYTE EndpointNum); 

// Set device address
VOID UsbDeviceSetAddress(BYTE Address);	 

// Get bus event
BYTE UsbDeviceGetBusEvent(VOID);

// Send data to control endpoint
VOID UsbDeviceControlSend(BYTE* Addr, BYTE Len);

// Receive data from control endpoint
BYTE UsbDeviceControlRcv(BYTE* Buf, BYTE MaxLen);

// Receive setup packet from control endpoint
BYTE UsbDeviceSetupRcv(BYTE* Buf, BYTE MaxLen);	

// Send data to bulk in endpoint
VOID UsbDeviceBulkSend(BYTE* Addr, WORD Len);

// Receive data from bulk out endpoint
WORD UsbDeviceBulkRcv(BYTE* Buf, WORD Len);	 

// hid
VOID UsbDeviceIntSend(BYTE* Addr, WORD Len);

#endif
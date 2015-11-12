#ifndef __HOST_HCD_H__
#define	__HOST_HCD_H__


//OTG连接检测
typedef enum _USB_LINK_STATE
{
	USB_HOST_LINK, 				//U盘已连接状态		:下拉使能，上拉禁止，DP == 1
	USB_HOST_LINK_JITTER, 		//U盘连接消抖状态	:下拉使能，上拉禁止，DP == 1
	USB_HOST_DETECT,			//检测U盘连接状态	:下拉使能，上拉禁止，DP == 0
	
	USB_DEVICE_DETECT,			//检测PC连接状态	:下拉禁止，上拉禁止，DP == 1
	USB_DEVICE_LINK_JITTER,		//PC连接消抖状态
	USB_DEVICE_UNLINK_JITTER,  	//PC断开消抖状态
	USB_DEVICE_LINK, 			//PC已连接状态		:下拉禁止，上拉禁止，DP == 0
	USB_DEVICE_ACTIVE, 			//PC已激活状态		:下拉禁止，上拉使能，SOF持续
	USB_DEVICE_SOF_TIMEOUT		//SOF超时状态		:下拉禁止，上拉禁止，DP == 0

} USB_LINK_STATE;

extern BYTE gUsbLinkState;


// Endpoint number. Fixed, user should not modify.
//作为HOST时，描述的端点号跟物理端点号可以不相同
#define	HOST_CONTROL_EP				0
#define	HOST_BULK_IN_EP				1
#define	HOST_BULK_OUT_EP			1


//pipe type
#define PIPE_TYPE_CONTROL		0
#define PIPE_TYPE_ISO_IN		1
#define PIPE_TYPE_ISO_OUT		2
#define PIPE_TYPE_INT_IN		3
#define PIPE_TYPE_INT_OUT		4
#define PIPE_TYPE_BULK_IN		5
#define PIPE_TYPE_BULK_OUT		6

//control transfer direction
#define USB_DIR_OUT				0			
#define USB_DIR_IN				1

//define OTG endpoint descriptor, used for configuration endpoint information
typedef struct _PIPE_INFO
{
	BYTE	EndpointNum	;	//endpoint number
	BYTE	PipeType;		//endpoint type: ctrl, bulk, interrupt, isochronous
	BYTE	MaxPacketSize;	//max packet size

} PIPE_INFO;

extern BOOL IsUsbIOUseToGpio; //USB IO端口用于普通GPIO

VOID OTGLinkCheck(VOID);

BOOL HostUsbOpen(VOID);
BOOL HostIsLink(VOID);		//检查USB设备连接状态
VOID HostUsbPortReset(VOID);
VOID HostUsbSetAddress(BYTE Address); // Set device address

//从控制端点完成一次传输
BOOL HostUsbSetupPacket(BYTE* SetupPacket, BYTE Direction, BYTE* Buf, WORD Len);

//BOOL HostSendSetup(PIPE_INFO* Pipe, BYTE* Buf, BYTE Len);
BOOL HostRcvPacket(PIPE_INFO* Pipe, BYTE* Buf, WORD Len);
BOOL HostSendPacket(PIPE_INFO* Pipe, BYTE* Buf, WORD Len);

#endif
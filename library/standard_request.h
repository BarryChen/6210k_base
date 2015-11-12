#ifndef __STANDARD_REQUEST_H__
#define	__STANDARD_REQUEST_H__


//USB types, the second of three bRequestType fields
#define USB_TYPE_MASK			(0x03 << 5)
#define USB_TYPE_STANDARD		(0x00 << 5)
#define USB_TYPE_CLASS			(0x01 << 5)
#define USB_TYPE_VENDOR			(0x02 << 5)
#define USB_TYPE_RESERVED		(0x03 << 5)

//USB recipients, the third of three bRequestType fields
#define USB_RECIP_MASK			0x1F
#define USB_RECIP_DEVICE		0x00
#define USB_RECIP_INTERFACE		0x01
#define USB_RECIP_ENDPOINT		0x02
#define USB_RECIP_OTHER			0x03


//Standard requests, for the bRequest field of a SETUP packet.
//These are qualified by the bRequestType field, so that for example
//TYPE_CLASS or TYPE_VENDOR specific feature flags could be retrieved
//by a GET_STATUS request.
#define USB_REQ_GET_STATUS				0
#define USB_REQ_CLEAR_FEATURE			1
#define USB_REQ_SET_FEATURE				3
#define USB_REQ_SET_ADDRESS				5
#define USB_REQ_GET_DESCRIPTOR			6
#define USB_REQ_SET_DESCRIPTOR			7
#define USB_REQ_GET_CONFIGURATION		8
#define USB_REQ_SET_CONFIGURATION		9
#define USB_REQ_GET_INTERFACE			10
#define USB_REQ_SET_INTERFACE			11
#define USB_REQ_SYNCH_FRAME				12

//USB feature flags are written using USB_REQ_{CLEAR,SET}_FEATURE, 
//and are read as a bit array returned by USB_REQ_GET_STATUS.  
//(So there are at most sixteen features of each type.)
#define USB_DEVICE_SELF_POWERED			0	//(read only) from GET_STATUS request
#define USB_DEVICE_REMOTE_WAKEUP		1	//dev may initiate wakeup
#define USB_DEVICE_TEST_MODE			2	//(high speed only)
#define USB_DEVICE_B_HNP_ENABLE			3	//dev may initiate HNP
#define USB_DEVICE_A_HNP_SUPPORT		4	//RH port supports HNP 
#define USB_DEVICE_A_ALT_HNP_SUPPORT	5	//other RH port does
#define USB_DEVICE_DEBUG_MODE			6	//(special devices only)

#define USB_ENDPOINT_HALT				0	//IN/OUT will STALL


typedef struct _USB_REQUEST 
{
	BYTE	RequestType;
	BYTE	Request;
	WORD	Value;
	WORD	Index;
	WORD	Length;

} USB_REQUEST;


//STANDARD DESCRIPTORS ... as returned by GET_DESCRIPTOR, 
//or (rarely) accepted by SET_DESCRIPTOR.
//Note that all multi-byte values here are encoded in little endian
//byte order "on the wire".  But when exposed through Linux-USB APIs,
//they've been converted to cpu byte order.
#define USB_DT_DEVICE					1
#define USB_DT_CONFIG					2
#define USB_DT_STRING					3
#define USB_DT_INTERFACE				4
#define USB_DT_ENDPOINT					5
#define USB_DT_DEVICE_QUALIFIER			6
#define USB_DT_OTHER_SPEED_CONFIG		7
#define USB_DT_INTERFACE_POWER			8

//these are from a minor usb 2.0 revision (ECN)
#define USB_DT_OTG						9
#define USB_DT_DEBUG					10
#define USB_DT_INTERFACE_ASSOCIATION	11

//conventional codes for class-specific descriptors
#define USB_DT_CS_DEVICE		0x21
#define USB_DT_CS_CONFIG		0x22
#define USB_DT_CS_STRING		0x23
#define USB_DT_CS_INTERFACE		0x24
#define USB_DT_CS_ENDPOINT		0x25

//All standard descriptors have these 2 fields at the beginning
typedef struct _USB_DESCRIPTOR_HEADER 
{
	BYTE  	Length;
	BYTE  	DescriptorType;

} USB_DESCRIPTOR_HEADER;


//USB_DT_DEVICE: Device descriptor
typedef struct _USB_DEVICE_DESCRIPTOR 
{
	BYTE 	Length;
	BYTE  	DescriptorType;
	WORD 	BcdUSB;
	BYTE  	DeviceClass;
	BYTE  	DeviceSubClass;
	BYTE  	DeviceProtocol;
	BYTE  	MaxPacketSize0;
	WORD	IdVendor;
	WORD	IdProduct;
	WORD	BcdDevice;
	BYTE  	ManufacturerIndex;
	BYTE  	ProductIndex;
	BYTE  	SerialNumberIndex;
	BYTE  	NumConfigurations;

} USB_DEVICE_DESCRIPTOR;

#define USB_DT_DEVICE_SIZE		18


//Device and/or Interface Class codes as found in bDeviceClass or bInterfaceClass
//and defined by www.usb.org documents
#define USB_CLASS_PER_INTERFACE		0	//for DeviceClass
#define USB_CLASS_AUDIO				1
#define USB_CLASS_COMM				2
#define USB_CLASS_HID				3
#define USB_CLASS_PHYSICAL			5
#define USB_CLASS_STILL_IMAGE		6
#define USB_CLASS_PRINTER			7
#define USB_CLASS_MASS_STORAGE		8
#define USB_CLASS_HUB				9
#define USB_CLASS_CDC_DATA			10
#define USB_CLASS_CSCID				11	//chip+ smart card
#define USB_CLASS_CONTENT_SEC		13	//content security
#define USB_CLASS_VIDEO				14
#define USB_CLASS_APP_SPEC			0xFE
#define USB_CLASS_VENDOR_SPEC		0xFF

#define USB_CLASS_HID_REPORT		0x22


//USB_DT_CONFIG: Configuration descriptor information.
//USB_DT_OTHER_SPEED_CONFIG is the same descriptor, except that the
//descriptor type is different.  Highspeed-capable devices can look
//different depending on what speed they're currently running.  Only
//devices with a USB_DT_DEVICE_QUALIFIER have any OTHER_SPEED_CONFIG descriptors.
typedef struct _USB_CONFIG_DESCRIPTOR 
{
	BYTE  	Length;
	BYTE  	DescriptorType;
	WORD  	TotalLength;
	BYTE  	NumInterfaces;
	BYTE  	ConfigurationValue;
	BYTE  	ConfigurationIndex;
	BYTE  	Attributes;
	BYTE  	MaxPower;

} USB_CONFIG_DESCRIPTOR;

#define USB_DT_CONFIG_SIZE		9

//from config descriptor bmAttributes
#define USB_CONFIG_ATT_ONE			(1 << 7)	//must be set
#define USB_CONFIG_ATT_SELFPOWER	(1 << 6)	//self powered
#define USB_CONFIG_ATT_WAKEUP		(1 << 5)	//can wakeup


//USB_DT_STRING: String descriptor
typedef struct _USB_STRING_DESCRIPTOR 
{
	BYTE  	Length;
	BYTE  	DescriptorType;
	
	//UTF-16LE encoded
	WORD 	Data[1];
			
} USB_STRING_DESCRIPTOR;

//note that "string" zero is special, it holds language codes that
//the device supports, not Unicode characters.

//USB_DT_INTERFACE: Interface descriptor
typedef struct _USB_INTERFACE_DESCRIPTOR 
{
	BYTE  	Length;
	BYTE  	DescriptorType;
	BYTE  	InterfaceNumber;
	BYTE  	AlternateSetting;
	BYTE  	NumEndpoints;
	BYTE  	InterfaceClass;
	BYTE  	InterfaceSubClass;
	BYTE  	InterfaceProtocol;
	BYTE  	Interface;

} USB_INTERFACE_DESCRIPTOR;

#define USB_DT_INTERFACE_SIZE		9


//USB_DT_ENDPOINT: Endpoint descriptor
typedef struct _USB_ENDPOINT_DESCRIPTOR 
{
	BYTE  	Length;
	BYTE  	DescriptorType;
	BYTE  	EndpointAddress;
	BYTE  	Attributes;
	WORD 	MaxPacketSize;
	BYTE  	Interval;
	// NOTE:  these two are _only_ in audio endpoints.
	// use USB_DT_ENDPOINT*_SIZE in bLength, not sizeof.
	BYTE  	Refresh;
	BYTE  	SynchAddress;

} USB_ENDPOINT_DESCRIPTOR;

#define USB_DT_ENDPOINT_SIZE		7
#define USB_DT_ENDPOINT_AUDIO_SIZE	9	//Audio extension


#endif
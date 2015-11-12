#include "syscfg.h"
#include "device_hcd.h"
#include "device_stor.h"


#if (defined(FUNC_AUDIO_EN) || defined(FUNC_READER_EN) || defined(FUNC_AUDIO_READER_EN))

//单独的读卡器模式，使用下面的VID、PID、STRING
WORD ReaderVID = 0x0000;
WORD ReaderPID = 0x0000;
BYTE* ReaderString_Manu = "MVSILICON.INC.";			//max length: 32bytes
BYTE* ReaderString_Product = "MV CARD READER";		//max length: 32bytes
BYTE* ReaderString_SerialNumber = "2011090112345";	//max length: 32bytes


//mass-storage device initial
VOID DeviceStorCtrlInit(VOID)
{	
	DeviceStorInit();
}


//mass-storage device initial
VOID DeviceStorCtrlEnd(VOID)
{	
	UsbDeviceDisConnect();
}


//mass-storage device process
VOID DeviceStorStateCtrl(VOID)
{	
	DeviceStorProcess();
}

#endif

#ifndef __USB_AUDIO_CTRL_H__
#define __USB_AUDIO_CTRL_H__

// initial usb audio device. 
BOOL DeviceAudioCtrlInit(VOID);

// end usb audio device.
VOID DeviceAudioCtrlEnd(VOID);

// usb audio device state control.
VOID DeviceAudioStateCtrl(VOID);

#endif

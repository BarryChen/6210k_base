#ifndef __RADIO_APP_INTERFACE_H__
#define __RADIO_APP_INTERFACE_H__


BOOL RadioWriteData(VOID);
VOID RadioInit(VOID);
BOOL RadioSetFreq(VOID);
VOID RadioSetVolume(BYTE Volume);
BOOL RadioMute(BOOL MuteFlag);
BYTE RadioRSSIRead(BOOL AutoSeekFlag);
VOID RadioSearchSet(WORD Freq);
VOID RadioPowerOn(VOID);	
VOID RadioPowerDown(VOID);	
VOID RadioMonoModeSet(BOOL MonoEnFlag);
BOOL RadioGetStereoState(VOID);

#endif

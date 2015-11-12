#ifndef __PCM_AMPLITUDE_H__
#define __PCM_AMPLITUDE_H__


//get PCM data,return LCD Amplitude degree(0,1,2,3,4)
BYTE GetPCMAmplitude(VOID);

VOID ClearPcmFifo(VOID);

WORD GetPcmFifoBytes(VOID);

#endif
 
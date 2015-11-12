#ifndef  __MV_FM_H__
#define  __MV_FM_H__


VOID MVFM_SetCh(WORD Freq);
WORD MVFM_GetCh(VOID);
VOID MVFM_VolumeSet(VOID);
VOID MVFM_Mute(BOOL MuteFlag);
VOID MVFM_Initialization(VOID);
VOID MVFM_SetFreq(WORD Freq);
VOID MVFM_Init(VOID);
VOID MVFM_PowerOn(VOID);
VOID MVFM_PowerDown(VOID);
VOID MVFM_RXSetTH(VOID);
VOID MVFM_SearchSet(WORD Freq);
BYTE MVFM_SearchRead(BOOL AutoSeekFlag);
BOOL MVFM_ReadID(VOID);
VOID MVFM_VolSet(BYTE Vol);
VOID MVFM_AutoSeekConfig(WORD StartFreq, WORD StopFreq, BYTE SeekStep, BOOL SeekMode, BOOL SeekStartFlag);
VOID MVFM_SetMonoMode(BOOL MonoEnFlag);
BOOL MVFM_GetStereoStatus(VOID); 
BYTE MVFM_ReadReg(BYTE RegAddr);

#endif


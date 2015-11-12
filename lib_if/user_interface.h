#ifndef __USER_INTERFACE_H__
#define __USER_INTERFACE_H__

#define UP		1
#define DOWN	0

extern BOOL IsDacMute;
#ifdef FUNC_SAVE_PLAYTIME_EN
extern BOOL gSavePlayTimeEnFlag;
#endif

// 如果用户需要实现按键快速响应功能，要在应用代码上重写QuickResponse()函数
BOOL QuickResponse(VOID);

// 如果用户需要在1ms系统时基上做处理，要在应用代码上重写TimerTick1ms()函数
VOID TimerTick1ms(VOID);

// 选择DAC 和外部功放Mute状态
VOID MuteOn(BOOL DacMuteEn, BOOL ExMuteEn);

// 解除Mute状态
VOID UnMute(VOID);

// 改变Mute状态
VOID MuteStatusChange(VOID);

// 设置系统音量
VOID SetVolume(VOID);

// 设置系统音量并同时解除Mute状态
VOID SetVolumeWithMute(VOID);

// 设置系统音量并同时解除Mute状态、显示音量
VOID SetVolumeWithDisplay(VOID);

// 产生按键嘀嘀声
VOID PushKeyBeep(BYTE ContinueNum);

//调节系统音量
VOID VolumeAdjust(BYTE Direction);


#endif

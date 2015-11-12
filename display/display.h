#ifndef __DISPLAY_H__
#define __DISPLAY_H__


typedef enum _DISP_DEV
{
	DISP_DEV_NONE = 0,
	DISP_DEV_SLED,
	DISP_DEV_LED1888,
	DISP_DEV_LED8888,
	DISP_DEV_LED57,
	DISP_DEV_LCD58
}DISP_DEV;


#define AMPLITUDE_INTERVAL		100
#define FFFB_BLINK_INTERVAL		300
#define PLAY_BLINK_INTERVAL		500
#define MUTE_BLINK_INTERVAL		1000
#define SCAN_BLINK_INTERVAL		300
#define DEV_SYMBOL_INTERVAL		1000
#define NORMAL_INTERVAL			1000
#define INTERIM_INTERVAL		2000	// EQ、Repeat、Volume显示保持状态
#define SET_INTERVL				5000	// RTC设置等显示状态保持时间

typedef struct _ST_BLINK
{
	BYTE MuteBlink		:1;	//复用做SLED MP3 PAUSE/STOP/MUTE闪烁标志(慢速闪烁)
	BYTE HourSet		:1;	//复用做SLED MP3快进快退/Radio搜台闪烁标志(快速闪烁)
	BYTE MinSet			:1;	//复用做SLED MP3播放闪烁标志(常速闪烁)
	BYTE RepeatBlink	:1;
	BYTE MuteBlankScreen:1;
	BYTE HourBlankScreen:1;
	BYTE MinBlankScreen	:1;
	BYTE RepeatOff		:1;

} ST_BLINK;

typedef union _UN_BLINK
{
	ST_BLINK 	Blink;
	BYTE		BlinkFlag;

} UN_BLINK;
	
extern UN_BLINK gBlink;
extern TIMER	DispTmr;
extern TIMER            LedFlashTmr;
extern BOOL             LedFlashOn ;
extern BYTE gDispPrevRep;
extern BYTE gDispPrevEQ;
extern BYTE gDispPrevVol;
extern BYTE gDispPrevPlayState;
extern DISP_DEV	gDisplayMode;


// Clear Screen
VOID ClearScreen(VOID);

// Display initilization
// Indicate whether the system in "STAND-BY" state or not.
VOID DispInit(BOOL IsStandBy);

// Display de-initialization.
// Indicate whether the system in "STAND-BY" state or not.
VOID DispDeInit(VOID);

// 显示刷新.
// Call by display refresh ISR.
VOID LedFlushDisp(VOID);

// 显示刷新.
// Lcd display refresh ISR.
VOID LcdFlushDisp(VOID);

// 设置Repeat模式时调用.
VOID DispRepeat(BOOL IsForced);

// 设置EQ时调用.
VOID DispEQ(BOOL IsForced);

// 播放状态变化时调用(Play/Pause/Stop/FF/FB).
VOID DispPlayState(VOID);

// 文件夹功能打开/关闭时调用.
VOID DispFoldState(VOID);

// 音量设置时调用.
VOID DispVolume(BOOL IsForced);
#ifdef FUNC_PT231X_EN
// 高音设置时调用.
VOID DispTreb(BOOL IsForced);

// 低音设置时调用.
VOID DispBass(BOOL IsForced);
#endif
// Mute状态变化时调用.
VOID DispMute(VOID);

//" LOD"
VOID DispLoad(VOID);

// 文件切换时调用.
VOID DispDev(VOID);

#ifdef FUNC_RTC_EN
VOID DispRtc(VOID);
#endif

VOID DispNum(WORD Num);

// 设备切换时调用.
VOID DispFileNum(VOID);

#define DIS_PLAYTIME_PALY    0
#define DIS_PLAYTIME_PAUSE   1
VOID DispPlayTime(BYTE Type);
VOID DispResume(VOID);

// 主函数调用.
VOID Display(VOID);

//显示Power on信息
VOID DispPowerOn(VOID);

// 进入Audio显示时调用
VOID DispAudio(VOID);

#endif

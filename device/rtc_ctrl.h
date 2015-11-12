#ifndef __RTC_CTRL_H__
#define __RTC_CTRL_H__

#include "syscfg.h"
#include "rtc.h"

// RTC control state.
typedef enum _RTC_STATE
{
	RTC_STATE_IDLE,
	RTC_STATE_SET_TIME,
	RTC_STATE_SET_ALARM,
	RTC_STATE_SET_SOURCE

} RTC_STATE;


typedef enum _RTC_SUB_STATE
{
	RTC_SET_NONE = 0,
	RTC_SET_ALARM_NUM,
	RTC_SET_ALARM_MODE,
	RTC_SET_YEAR,
	RTC_SET_MON,
	RTC_SET_DATE,
	RTC_SET_WEEK,
	RTC_SET_HR,
	RTC_SET_MIN,
	RTC_SET_SCE,
#ifdef AU6210K_BOOMBOX_DEMO
	RTC_SET_ALARM_SOURCE,
	RTC_SET_ALARM_VOLUME,
#endif
	RTC_SET_ALARM_ONOFF

} RTC_SUB_STATE;

typedef enum _RTC_SOURCE_STATE
{
	RTC_SOURCE_CLOCK,
	RTC_SOURCE_YEAR,
	RTC_SOURCE_MON,
	RTC_SOURCE_ALARM,
	RTC_SOURCE_ONOFF

} RTC_SOURCE_STATE;

typedef enum _RTC_ALARM_SOURCE_STATE
{
	RTC_ALARM_SOURCE_RADIO,
	RTC_ALARM_SOURCE_USB,
	RTC_ALARM_SOURCE_CARD,
	RTC_ALARM_SOURCE_AUX,
	RTC_ALARM_ONOFF

} RTC_ALARM_SOURCE_STATE;


#ifdef FUNC_ALARM_EN
#ifndef FUNC_BREAK_POINT_EN
	extern AlARM_INFO  AlarmList2Store[MAX_ALARM_NUM]; 
	#define gAlarmList AlarmList2Store
#else
	#define gAlarmList gBreakPointInfo.PowerMemory.AlarmList2Store
#endif
#endif

extern RTC_STATE RtcState;
extern RTC_SUB_STATE RtcSubState;
extern BYTE  RtcAutOutTimeCount;
extern RTC_DATE_TIME gRtcTime;

#ifdef	AU6210K_CAR_MODEL_DEMO
extern RTC_SOURCE_STATE RtcSourceState;
extern BYTE gAlarmState;
extern BOOL gAlarmOnFlag;
#endif

#ifdef AU6210K_BOOMBOX_DEMO
extern BYTE gAlarm1State, gAlarm2State;
extern BOOL gAlarm1OnFlag;
extern BOOL gAlarm2OnFlag;
extern BYTE gAlarmVolume;
extern BYTE gAlarm1Volume;
extern BYTE gAlarm2Volume; 
extern RTC_ALARM_SOURCE_STATE gAlarmSource;
extern RTC_ALARM_SOURCE_STATE gAlarm1Source;
extern RTC_ALARM_SOURCE_STATE gAlarm2Source;
#endif


#define READ_RTC_TIME                     0
#define SET_RTC_TIME_YEAR             1
extern BYTE gRtcRdWrFlag;  

#ifdef FUNC_ALARM_EN
extern BYTE RtcAlarmNum; 
extern RTC_DATE_TIME gAlarmTime;
extern BYTE gAlarmMode;
#endif


// Rtc initial.
VOID RtcCtrlInit(VOID);

// Rtc mode state control.
VOID RtcStateCtrl(VOID);


#endif

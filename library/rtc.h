#ifndef __RTC_H__
#define __RTC_H__


// this calendar supports from 1980.01.01 to 2099.12.31 year
#define		RTC_START_YEAR		1980	
#define		RTC_END_YEAR		2099

//闹钟号为 1 ~ MAX_ALARM_NUM
#define		MAX_ALARM_NUM		2

#define   ALARM_MODE_ONCE_ONLY   0x01
#define   ALARM_MODE_PER_DAY     0x02
#define   ALARM_MODE_PER_WEEK    0x03
#define   ALARM_MODE_INVALID     0xFF//无效mode


#define ALARM_STATUS_OPEN        0x01
#define ALARM_STATUS_CLOSE       0x02


// solar calendar and time structure.
typedef struct _RTC_DATE_TIME
{
	WORD	Year;
	BYTE	Mon;
	BYTE	Date;
	BYTE	WDay;	//week value: 0--6, 0 is sunday
	BYTE	Hour;
	BYTE	Min;
	BYTE	Sec;
				
} RTC_DATE_TIME;


typedef struct _ALARM_INFO
{
    BYTE AlarmMode;
    DWORD AlarmTimeAsSeconds;
	BYTE AlarmStatus;	//1:open, 2:close

} AlARM_INFO;


// lunar calendar structure.
typedef struct _RTC_LUNAR_DATE
{
	WORD Year;
	BYTE Month;
	BYTE Date;
	BYTE MonthDays;		//30 or 29
	BOOL IsLeapMonth;

} RTC_LUNAR_DATE;


// 在系统上电后要先调用次函数
VOID InitRtc(AlARM_INFO* pAlarmList, BYTE AlarmNum);

// 查询闹钟中断标志
// return 0 if no alarm arrived
// return alarm number if alarm arrived
BYTE CheckRtcAlarmFlag(VOID);

// 打开某个闹钟
BOOL OpenAlarm(BYTE AlarmNum);

// 关闭某个闹钟
BOOL CloseAlarm(BYTE AlarmNum);

// 获取闹钟状态
BYTE GetAlarmStatus(BYTE AlarmNum);


// 设置当前时间 
VOID SetRTCCurrTime(RTC_DATE_TIME* CurrTime);

// 获取当前时间
VOID GetRTCCurrTime(RTC_DATE_TIME* CurrTime);

// Get Current Alarm Setting
// input:  BYTE AlarmIndex
// 返回值为AlarmMode
BYTE GetAlarmTime(BYTE AlarmIndex, RTC_DATE_TIME* timePtr);

// Do alarm setting
BOOL SetAlarmTime(BYTE AlarmNum, BYTE AlarmMode, RTC_DATE_TIME* AlarmData);

// 获取阳历某年某月的天数
BYTE GetMonthDays(WORD Year, BYTE Mon);

//////////////////////////与农历相关的函数///////////////////////////////////////////////////////////////////////////////////////
// 阳历日期转农历日期
VOID SolarToLunar(RTC_DATE_TIME* DateTime, RTC_LUNAR_DATE* LunarDate);

// 获取农历年份的天干
// 返回0-9, 分别对应天干: {"甲", "乙", "丙", "丁", "戊", "己", "庚", "辛", "壬", "癸"}
BYTE GetHeavenlyStem(WORD Year);

// 获取农历年份的地支
// 返回0-11, 分别对应地支: {"子", "丑", "寅", "卯", "辰", "巳", "午", "未", "申", "酉", "戌", "亥"}
//          分别对应生肖: {"鼠", "牛", "虎", "兔", "龙", "蛇", "马", "羊", "猴", "鸡", "狗", "猪"}
BYTE GetEarthlyBranch(WORD Year);

BOOL HalfSecStatus(VOID);


#endif

#ifndef __SYS_ON_OFF_H__
#define __SYS_ON_OFF_H__


//系统启动，包括系统从SLEEP状态下唤醒启动或者从POWER DOWN状态下以片内主电源上电方式启动的相关处理
//一般情况下，2种启动方式在用户产品中不会同时存在
VOID SystemOn(VOID);

//系统关闭，包括系统以SLEEP方式关闭或者以片内主电源下电POWER DOWN方式关闭的相关处理
VOID SystemOff(VOID);

typedef enum _SLEEP_STATE
{
	SLEEP_00 = 0,	
	SLEEP_120,
	SLEEP_105,
	SLEEP_90,
	SLEEP_75,
	SLEEP_60,
	SLEEP_45,
	SLEEP_30,
	SLEEP_15,	
	SLEEP_MAX

} SLEEP_STATE;


#ifdef FUN_SYSTEM_POWEROFF_WAIT_TIME
typedef struct _SystemTimingOff
{
	SLEEP_STATE     SleepState;
	BYTE     ProState;
	BYTE     EnState;
	TIMER   AutoOutTimer;
	TIMER   DisTimer;
}SystemTimingOff;


//flag取值
//0~120m,1~90m,2~60m,3~30m,4~15m 

//ProState取值
#define SYS_OFF_TIMEING_IDLE                      0//空闲状态
#define SYS_OFF_TIMEING_READ_ONLY_IDLE            1//读状态后回到空闲状态
#define SYS_OFF_TIMEING_READ_ONLY_TIMING          2//读状态后回到定时关机或设置定时关机
#define SYS_OFF_TIMEING_SET_DIS                   3//关闭定时关机
#define SYS_OFF_TIMEING_SET_MODE_VALID            4//有效的定时关机时间
#define SYS_OFF_TIMEING_SET_MODE_INVALID          5//无效的定时关机时间
#define SYS_OFF_TIMEING_WAIT_POWEROFF             6//等待定时关机时间到

//AutoOutTimer取值
//120m,90m,60m,30m,15m

//DisTimer取值
//3s

extern SystemTimingOff gSysTimeOffState;

VOID SystemOffTimeInit(VOID);

#define SYSOFF_TIME_READ_ONLY	0	
#define SYSOFF_TIME_SET_VALUE   1

VOID SystemOffTimeEnDis(BYTE Type);
VOID SystemOffTimeSet(VOID);
VOID SystemOffTimeMsgPro(BYTE Msg);
VOID SystemOffTimeWait(VOID);	
#endif

#endif

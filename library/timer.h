#ifndef __TIMER_H__
#define __TIMER_H__


// define softtimer structure.
typedef struct	_TIMER
{
	DWORD	TimeOutVal; 								//time out value
	BOOL	IsTimeOut;									//time out flag

} TIMER;


extern DWORD gSysTick; 					//system tick counter


// System tick (1ms) initilization.
VOID SysTickInit(VOID);

VOID TimerTick(VOID);

// Get now time from system start.
DWORD TimeNow(VOID);

// Get time of some softtimer from setting to now.
//DWORD PastTimeGet(TIMER* timer);

// Set time out value of softtimer.
VOID TimeOutSet(TIMER* timer, DWORD timeout);

// Check whether time out.
BOOL IsTimeOut(TIMER* timer);

// set timer count
VOID TimerCountSet(BYTE div);

#endif

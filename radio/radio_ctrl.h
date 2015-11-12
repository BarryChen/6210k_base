#ifndef __RADIO_CTRL_H__
#define __RADIO_CTRL_H__
#include "syscfg.h"


// Radio Freqency setting
#ifdef AU6210K_BOOMBOX_DEMO
#define MAX_RADIO_STATION_NUM		40	//前20个存台频率；后20个存对应的预存号
#else
#define MAX_RADIO_STATION_NUM		50
#endif

// Radio Tune Space
#define TUNE_SPACE
#define RADIO_TUNE_SPACE_STEP			

//seek timer value
#ifdef FUNC_RADIO_AUTOSEEK_EN
#define RadioSeekTimeValue		100
#else
#define RadioSeekTimeValue		10	//50
#endif

//搜到某个频道后是否需要停留指定时间同时显示台号
#define RADIO_SEEK_WAIT_EN
#ifdef RADIO_SEEK_WAIT_EN 
#if defined(FUNC_SINGLE_LED_EN)
#ifdef FUNC_EXMUTE_EN
#define RadioPreVimeTimerValue		2000	//有功放Mute时此时间不能太短
#else
#define RadioPreVimeTimerValue		1000 
#endif
#else
#define RadioPreVimeTimerValue		1000 
#endif
#endif


typedef enum _RADIO_NAME
{
	RADIO_NONE		= 0,	
	RADIO_RDA5807	= 1,	
	RADIO_SI4702	= 2,	
	RADIO_CS2010	= 3,	
	RADIO_CL6017X	= 4,	
	RADIO_KT0830E	= 5,
	RADIO_AR1000F	= 6,	
	RADIO_BK1080	= 7,
	RADIO_QN8035	= 8,
	RADIO_MVFM		= 9
	
} RADIO_NAME;


typedef enum _RADIO_SEEK_RESULT
{
	RADIO_SEEK_NONE = 0,
	RADIO_SEEK_FALSE,
	RADIO_SEEK_STC,
	RADIO_SEEK_VALIDSTATION,
	RADIO_SEEK_BANDLIMIT

} RADIO_SEEK_RESULT;


typedef enum _RADIO_SEEK_STATE
{
	RADIO_SEEK_START		= 0,
	RADIO_SEEK_SETTHRESHOLD = 1,
	RADIO_SEEK_SETFREQ		= 2,
	RADIO_SEEK_READRSSI		= 3,
	RADIO_SEEK_END

} RADIO_SEEK_STATE;


typedef enum _SEEK_MODE
{
	SEEK_NONE				= 0,
	SEEK_AUTOSCAN			= 1,
	SEEK_SCANUP				= 2,
	SEEK_SCANDOWN			= 3	
		
} SEEK_MODE;


typedef enum _RADIO_STATE
{
	RADIO_INIT = 0,
	RADIO_IDLE,
	RADIO_SEEK,
	RADIO_PROG,
	RADIO_READ
		
} RADIO_STATE;

//BandXtal:
//bit7 crystal or exclk 
//bit6~bit4 reserved for exclk select
//bit3~bit2 band select,For example:875~1080 and others,default :875~1080 other value may change some codes
//bit1~bit0 step select,For example:25K,50k,100k,200k,default 100k other value may change some codes
typedef struct _RADIO_DATA
{
	BYTE	BandXtalSel;		//基本配置便于扩充       	
	WORD	CurrFreq; 			//当前频率
	BYTE    CurrStation;		//当前台号// Start from 1,0表示没有台
	BYTE    Area1StationSum;	//760~874区间的频道数
	BYTE    StationSum;			//总的有效台数目(760~874 + 875~1080)// Start from 1,0表示没有台
	BYTE	Stations[MAX_RADIO_STATION_NUM];	

} RADIO_DATA;


//#define EEPROM_RADIO_DATA_BASEADDR	0x60
//#define NVM_RADIO_DATA_BASEADDR		0x10	// only for test!!!
//#define RADIO_DATA_BASEADDR			EEPROM_RADIO_DATA_BASEADDR

//搜台过程中的起点和终点频率
#define RADIO_LOWER_BOUND		875
#define RADIO_UPPER_BOUND		1080

//单步搜台步长选择
#define RADIO_STEP_100KHZ		10
#define RADIO_STEP_50KHZ		5
#define RADIO_STEP_200KHZ		20
#define RADIO_STEPSIZE			RADIO_STEP_100KHZ

//自动搜台步长选择
#define RADIO_SEEK_STEP_50KHZ	0
#define RADIO_SEEK_STEP_100KHZ	1
#define RADIO_SEEK_STEP_200KHZ	2
#define RADIO_SEEK_STEP			RADIO_SEEK_STEP_100KHZ


//存储在外部存储器中的频率需要加上此基数，才是正确的频率
//区间1：起点(第1参考点:RADIO_FREQ_BASE_760)~第2参考点(RADIO_FREQ_BASE_875)
//区间2：第2参考点(RADIO_FREQ_BASE_875)     ~终点
//区间1区间2须小于256，如760~1080，二宏可取760，875
#define RADIO_FREQ_BASE_760		760	//874-760
#define RADIO_FREQ_BASE_875		875	//1080-875


// RADIO control structure.
typedef struct _RADIO_CTRL
{
	RADIO_STATE	State;
	SEEK_MODE	SeekMode;
	TIMER		SeekPreviewTimer;	//seek time and preview time
#ifdef FUNC_SLAVE_UART_EN
       WORD RadioLowerBound;
       WORD RadioUpperBound;
	WORD RadioFreqBase;
	BYTE   RadioSeekStep;
	BYTE   RadioSeekResult;
#endif

} RADIO_CTRL;


#define RadioBlinkTime		300
#define RadioProgramWaitTime	5000


#define RadioDisCurFreNum		0	//显示当前频率
#define RadioDisCurChAutoscan	1	//显示当前台号，自动搜台过程中
#define RadioDisCurChChange		2	//显示当前台号，手动切换频道
#define RadioDisCurFreBlink		3	//闪烁
#define RadioDisCurProgramNum	4	//显示当前频率存储台号
#define RadioDisCurPresetCh		5	//显示当前预存台


//变量声明
extern RADIO_CTRL	gRadioCtrl;
extern BYTE RadioDisFlag;

#ifndef FUNC_BREAK_POINT_EN
extern RADIO_DATA  gRadioData2Store;
#define gRadioData gRadioData2Store
#else
#define gRadioData gBreakPointInfo.PowerMemory.RadioData2Store
#endif

extern RADIO_NAME Radio_Name;	//区分FM的型号

#ifdef AU6210K_BOOMBOX_DEMO
extern BOOL gRadioMonoEnableFlag;	
#endif


//函数声明
//#define __RADIOCTRL_DEBUG__
#ifdef   __RADIOCTRL_DEBUG__
#define RadioShowPowerOn				0
#define RadioShowCancelSearch			1
#define RadioShowSearchEndOverflow		2
#define RadioShowSearchEndBufferEnough	3
#define RadioShowSearchUpDown			4
#define RadioShowSaveCurFreToCurChannle	5
#define RadioShowSaveFre				6
#define RadioShowCurStationChange		7
#define RadioShowFreChange				8
VOID RadioShowDebug(BYTE index);
#else
#define RadioShowDebug(x)    //debug函数
#endif

VOID RadioCtrlInit(VOID);//进入FM前初始化函数
VOID RadioCtrlDeinit(VOID);//离开FM前初始化函数
VOID RadioEnterInit(VOID);
VOID CancelSearch(VOID);
VOID RadioArrayInsert(VOID);
VOID RadioEventProcess(VOID);//按键响应

#ifdef FUNC_DISP_EN	
VOID RadioDisBlink(VOID);
#endif

VOID RadioStationSearch(VOID);
VOID RadioStateCtrl(VOID);//按键响应,状态处理

#endif

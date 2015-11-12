#include "syscfg.h"
#include "message.h"
#include "rtc.h"
#include "rtc_ctrl.h"
#include "breakpoint.h"
#include "debug.h"

BYTE RtcFlag = 1;
BYTE gAlarmMode = ALARM_MODE_PER_DAY;

#ifdef FUNC_RTC_EN

#ifdef FUNC_ALARM_EN
#ifndef FUNC_BREAK_POINT_EN
	AlARM_INFO AlarmList2Store[MAX_ALARM_NUM];
#endif
#endif

RTC_STATE RtcState;
RTC_SUB_STATE RtcSubState;
BYTE RtcUpdateDisplay = FALSE;


static TIMER RtcReadTimer;
static TIMER RtcAutoOutTimer;	
extern TIMER RtcSetCntTimer;	

#define RTC_AUTO_OUT_TIME_UNIT	          250//ms
#define RTC_AUTO_OUT_TIME_COUNT           20
BYTE  RtcAutOutTimeCount = 0;

RTC_DATE_TIME gRtcTime;
BYTE gRtcRdWrFlag;  

#ifdef FUNC_ALARM_EN
BYTE RtcAlarmNum = 1;	
BYTE gCurAlarmNum; //当前响应的闹钟号	
RTC_DATE_TIME gAlarmTime;

SBYTE CODE ModeDisp[3][16] = 
{
    {"ALARM_ONCE_ONLY"},
    {"ALARM_PER_DAY"},
    {"ALARM_PER_WEEK"}
};

static RTC_DATE_TIME AlarmTemp;
#endif

#ifdef	AU6210K_CAR_MODEL_DEMO
RTC_SOURCE_STATE RtcSourceState;
BYTE gAlarmState;
BOOL gAlarmOnFlag = FALSE;
#endif

#ifdef AU6210K_BOOMBOX_DEMO
BYTE gAlarm1State, gAlarm2State;
BOOL gAlarm1OnFlag = FALSE;
BOOL gAlarm2OnFlag = FALSE;
BYTE gAlarmVolume = VOLUME_INIT;
BYTE gAlarm1Volume = VOLUME_INIT;
BYTE gAlarm2Volume = VOLUME_INIT; 
RTC_ALARM_SOURCE_STATE gAlarmSource;
RTC_ALARM_SOURCE_STATE gAlarm1Source = SYS_MODE_RADIO;
RTC_ALARM_SOURCE_STATE gAlarm2Source = SYS_MODE_RADIO;
#endif

//这2个函数可以放在lib中,开始...
extern WORD DateToDays(RTC_DATE_TIME* Time);
extern BYTE WeekDayGet(WORD Days);


static VOID RtcTimeDown(RTC_DATE_TIME* Time)
{
	switch(RtcSubState)
	{
#ifdef FUNC_ALARM_EN
		case RTC_SET_ALARM_NUM:
			RtcAlarmNum--;	
			if(RtcAlarmNum < 1)
			{
				//RtcAlarmNum = 8; 
				RtcAlarmNum = MAX_ALARM_NUM;    
			}
			gAlarmMode = GetAlarmTime(RtcAlarmNum, Time);
			DBG(("RtcAlarmNum:%bu\n", RtcAlarmNum));	
			break;
       
		case RTC_SET_ALARM_MODE:
			if(ALARM_MODE_ONCE_ONLY == gAlarmMode)
			{
				AlarmTemp = *Time;
			}
			else if(ALARM_MODE_PER_WEEK == gAlarmMode)
			{
				AlarmTemp.WDay = Time->WDay;
			}
			
			gAlarmMode--;		
			if(gAlarmMode < ALARM_MODE_ONCE_ONLY)
			{
				gAlarmMode = ALARM_MODE_PER_WEEK;    
			}
			
			if(ALARM_MODE_ONCE_ONLY == gAlarmMode)
			{
				Time->Year = AlarmTemp.Year;
				Time->Mon = AlarmTemp.Mon;
				Time->Date = AlarmTemp.Date;
				Time->WDay = AlarmTemp.WDay;
			}
			else if(ALARM_MODE_PER_WEEK == gAlarmMode)
			{
				Time->WDay = AlarmTemp.WDay;
			}
			DBG(("gAlarmMode[%bu]=%s\n", gAlarmMode, ModeDisp[gAlarmMode - 1]));
			break;
		 
#ifdef	AU6210K_CAR_MODEL_DEMO
		 case RTC_SET_ALARM_ONOFF:
			 if(gAlarmState == ALARM_STATUS_OPEN)
			 {
				 gAlarmState = ALARM_STATUS_CLOSE;
				 DBG(("ALARM_STATUS_CLOSE\n"));
			 }
			 else
			 {
				 gAlarmState = ALARM_STATUS_OPEN;
				 DBG(("ALARM_STATUS_OPEN\n"));
			 }
			 break;
#endif
#endif

		case RTC_SET_YEAR:
			Time->Year--;
			if(Time->Year < RTC_START_YEAR)
			{
				Time->Year = RTC_END_YEAR;
			}
			Time->WDay = WeekDayGet(DateToDays(Time));
			break;
	
		case RTC_SET_MON:
			Time->Mon--;
			if(Time->Mon == 0)
			{
				Time->Mon = 12;
			}
			if(Time->Date > GetMonthDays(Time->Year, Time->Mon))
			{
				Time->Date = GetMonthDays(Time->Year, Time->Mon);
			}
			Time->WDay = WeekDayGet(DateToDays(Time));
			break;
	
		case RTC_SET_DATE:
			Time->Date--;
			if(Time->Date == 0)
			{
				Time->Date = GetMonthDays(Time->Year, Time->Mon);
			}
			Time->WDay = WeekDayGet(DateToDays(Time));				
			break;
	
#ifdef FUNC_ALARM_EN	
		case RTC_SET_WEEK:
			if(Time->WDay > 0)
			{
				Time->WDay--;			
			}
			else
			{
				Time->WDay = 6;	
			}
			break;
#endif
	
		case RTC_SET_HR:
			Time->Sec = 0;	
			Time->Hour--;
			if(Time->Hour > 23)
			{
				Time->Hour = 23;
			}
			break;
	
		case RTC_SET_MIN:
			Time->Sec = 0;		
			Time->Min--;
			if(Time->Min > 59)
			{
				Time->Min = 59;
			}
			break;

#ifdef AU6210K_BOOMBOX_DEMO
		case RTC_SET_ALARM_SOURCE:
			if(gAlarmSource > RTC_ALARM_SOURCE_RADIO)
			{
				gAlarmSource--;
			}
			break;

		case RTC_SET_ALARM_VOLUME:
			if(gAlarmVolume > VOLUME_MIN)
			{
				gAlarmVolume--; 
			}		
			break;
#endif

		default:
			break;
	}
}


static VOID RtcTimeUp(RTC_DATE_TIME* Time)
{
	switch(RtcSubState)
	{
#ifdef FUNC_ALARM_EN
		case RTC_SET_ALARM_NUM:
			RtcAlarmNum++;	
			if(RtcAlarmNum > MAX_ALARM_NUM)
			{
			    RtcAlarmNum = 1;    
			}
			gAlarmMode = GetAlarmTime(RtcAlarmNum, Time);
			DBG(("RtcAlarmNum:%bu\n", RtcAlarmNum));	
			break;

		case RTC_SET_ALARM_MODE:
			if(ALARM_MODE_ONCE_ONLY == gAlarmMode)
			{
				AlarmTemp = *Time;
			}
			else if(ALARM_MODE_PER_WEEK == gAlarmMode)
			{
				AlarmTemp.WDay = Time->WDay;
			}

			gAlarmMode++;
			if(gAlarmMode > ALARM_MODE_PER_WEEK)
			{
				gAlarmMode = ALARM_MODE_ONCE_ONLY;    
			}
			if(ALARM_MODE_ONCE_ONLY == gAlarmMode)
			{
				Time->Year = AlarmTemp.Year;
				Time->Mon = AlarmTemp.Mon;
				Time->Date = AlarmTemp.Date;
				Time->WDay = AlarmTemp.WDay;
			}
			else if(ALARM_MODE_PER_WEEK == gAlarmMode)
			{
				Time->WDay = AlarmTemp.WDay;
			}
			DBG(("gAlarmMode[%bu]=%s\n", gAlarmMode,ModeDisp[gAlarmMode-1]));
			break;
		
#ifdef	AU6210K_CAR_MODEL_DEMO
		case RTC_SET_ALARM_ONOFF:
			if(gAlarmState == ALARM_STATUS_OPEN)
			{
				gAlarmState = ALARM_STATUS_CLOSE;
				DBG(("ALARM_STATUS_CLOSE\n"));
			}
			else
			{
				gAlarmState = ALARM_STATUS_OPEN;
				DBG(("ALARM_STATUS_OPEN\n"));
			}	
   		 	break;
#endif
#endif

		case RTC_SET_YEAR:
			DBG(("Time->Year:%u\n", Time->Year));
			Time->Year++;
			if(Time->Year > RTC_END_YEAR)
			{
				Time->Year = RTC_START_YEAR;
			}
			Time->WDay = WeekDayGet(DateToDays(Time));
			break;
	
		case RTC_SET_MON:
			Time->Mon++;
			if(Time->Mon > 12)
			{
				Time->Mon = 1;
			}
			if(Time->Date > GetMonthDays(Time->Year, Time->Mon))
			{
				Time->Date = GetMonthDays(Time->Year, Time->Mon);
			}
			Time->WDay = WeekDayGet(DateToDays(Time));
			break;
	
		case RTC_SET_DATE:
			Time->Date++;
			if(Time->Date > GetMonthDays(Time->Year, Time->Mon))
			{
				Time->Date = 1;
			}
			Time->WDay = WeekDayGet(DateToDays(Time));
			break;

#ifdef FUNC_ALARM_EN	
		case RTC_SET_WEEK:
			if(Time->WDay < 6)
			{
				Time->WDay++;			
			}
			else
			{
				Time->WDay = 0;	
			}
			break;
#endif
	
		case RTC_SET_HR:
			Time->Sec = 0;
			Time->Hour++;
			if(Time->Hour > 23)
			{
				Time->Hour = 0;
			}
			break;
	
		case RTC_SET_MIN:
			Time->Sec = 0;
			Time->Min++;
			if(Time->Min > 59)
			{
				Time->Min = 0;
			}
			break;

#ifdef AU6210K_BOOMBOX_DEMO
		case RTC_SET_ALARM_SOURCE:
			if(gAlarmSource < RTC_ALARM_ONOFF)
			{
				gAlarmSource++;
			}
			break;

		case RTC_SET_ALARM_VOLUME:
			if(gAlarmVolume < VOLUME_MAX)
			{
				gAlarmVolume++;	
			}		
			break;
#endif

		default:
			break;
	}
}


static VOID RtcNextSubState(VOID)
{
	switch(RtcSubState)
	{
#ifdef FUNC_ALARM_EN
		case RTC_SET_ALARM_NUM:
			DBG(("RTC_SET_ALARM_MODE:%2BX\n", gAlarmMode));
			RtcSubState = RTC_SET_ALARM_MODE;
			break;

		case RTC_SET_ALARM_MODE:
			switch(gAlarmMode)
			{
			    case 1:
			        DBG(("RTC_SET_YEAR\n"));
			        RtcSubState = RTC_SET_YEAR;
			        break;
			        
			    case 2:
			        DBG(("RTC_SET_HR\n"));
			        RtcSubState = RTC_SET_HR;
			        break;
			        
			    case 3:
			        DBG(("RTC_SET_WEEK\n"));
			        RtcSubState = RTC_SET_WEEK;
			        break;
			        
			    default:
			        break;   
			}
			break;
		
#ifdef	AU6210K_CAR_MODEL_DEMO
		case RTC_SET_ALARM_ONOFF:
			if(gAlarmState == ALARM_STATUS_OPEN)
			{
				OpenAlarm(RtcAlarmNum);
				DBG(("ALARM_STATUS_OPEN\n"));
			}
			else
			{
				CloseAlarm(RtcAlarmNum);			
				DBG(("ALARM_STATUS_CLOSE\n"));
			}				
			RtcState = RTC_STATE_SET_SOURCE;
			break;
#endif
#endif

		case RTC_SET_YEAR:
			DBG(("RTC_SET_MON\n"));
#ifdef	AU6210K_CAR_MODEL_DEMO
			RtcState = RTC_STATE_SET_SOURCE;
#else
			RtcSubState = RTC_SET_MON;
#endif
			break;
	
		case RTC_SET_MON:
			DBG(("RTC_SET_DATE\n"));
			RtcSubState = RTC_SET_DATE;
			break;
	
		case RTC_SET_DATE:
			DBG(("RTC_SET_HR\n"));
#ifdef	AU6210K_CAR_MODEL_DEMO
			RtcState = RTC_STATE_SET_SOURCE;
#else
			RtcSubState = RTC_SET_HR;
#endif
			break;

		case RTC_SET_WEEK:
			DBG(("RTC_SET_HR\n"));
			RtcSubState = RTC_SET_HR;
			break;

		case RTC_SET_HR:
			DBG(("RTC_SET_MIN\n"));
			RtcSubState = RTC_SET_MIN;
			break;

		case RTC_SET_MIN:
#ifdef	AU6210K_CAR_MODEL_DEMO
			RtcState = RTC_STATE_SET_SOURCE;
#else
#ifdef AU6210K_BOOMBOX_DEMO
			if(RtcState == RTC_STATE_SET_ALARM)
			{
				RtcSubState = RTC_SET_ALARM_SOURCE;
				if(RtcAlarmNum == 1)
				{
					gAlarmSource = gAlarm1Source;
				}
				else 
				{
					gAlarmSource = gAlarm2Source;
				}
				break;
			}
#endif
			RtcSubState = RTC_SET_NONE;
			RtcAutOutTimeCount = 0;
#endif
			break;
			
#ifdef AU6210K_BOOMBOX_DEMO
		case RTC_SET_ALARM_SOURCE:
			if(RtcAlarmNum == 1)
			{
				gAlarm1Source = gAlarmSource;
				gAlarmVolume = gAlarm1Volume;
			}
			else 
			{
				gAlarm2Source = gAlarmSource;
				gAlarmVolume = gAlarm2Volume;
			}
			
			if(gAlarmSource == RTC_ALARM_ONOFF)
			{
				CloseAlarm(RtcAlarmNum);			
				DBG(("ALARM_STATUS_CLOSE\n"));				
			}
			else
			{				
				OpenAlarm(RtcAlarmNum);
				DBG(("ALARM_STATUS_OPEN\n"));					
			}
			
			RtcSubState = RTC_SET_ALARM_VOLUME;
			break;

		case RTC_SET_ALARM_VOLUME:
			if(RtcAlarmNum == 1)
			{
				gAlarm1Volume = gAlarmVolume;
			}
			else 
			{
				gAlarm2Volume = gAlarmVolume;
			}			
			RtcSubState = RTC_SET_NONE;
			break;
#endif
			
		case RTC_SET_NONE:
		default:
			if(RtcState == RTC_STATE_SET_TIME)
			{
#ifdef	AU6210K_CAR_MODEL_DEMO
				DBG(("RTC_SET_CLOCK\n"));
				RtcSubState = RTC_SET_HR;
#else
				DBG(("RTC_SET_YEAR\n"));
				RtcSubState = RTC_SET_YEAR;
#endif
			}
#ifdef FUNC_ALARM_EN
			else if(RtcState == RTC_STATE_SET_ALARM)
			{
				DBG(("RTC_SET_ALARM_NUM\n"));
					DBG(("RtcAlarmNum:%bu\n", RtcAlarmNum));
				DBG(("_________________gAlarmMode1:%2BX\n", gAlarmMode));

#ifdef	AU6210K_CAR_MODEL_DEMO
				RtcState = RTC_STATE_SET_SOURCE;
#else
				RtcSubState = RTC_SET_ALARM_NUM;
#endif				
			}
#endif	
			break;
	}
}


#ifdef FUNC_LUNAR_EN
static VOID DisplayLunarDate(VOID)
{
	//农历年份名称
	BYTE CODE LunarYearName[12][2] = {"鼠", "牛", "虎", "兔", "龙", "蛇", "马", "羊", "猴", "鸡", "狗", "猪"};
	
	//农历月份名称
	BYTE CODE LunarMonthName[12][2] = {"正", "二", "三", "四", "五", "六", "七", "八", "九", "十", "冬", "腊"};
	
	//农历日期名称
	BYTE CODE LunarDateName[30][4] = {"初一", "初二", "初三", "初四", "初五", "初六", "初七", "初八", "初九", "初十", 
									  "十一", "十二", "十三", "十四", "十五", "十六", "十七", "十八", "十九", "二十",
									  "廿一", "廿二", "廿三", "廿四", "廿五", "廿六", "廿七", "廿八", "廿九", "三十"};
	
	//农历天干名称
	BYTE CODE HeavenlyStemName[10][2] = {"甲", "乙", "丙", "丁", "戊", "己", "庚", "辛", "壬", "癸"};
	
	//农历地支名称
	BYTE CODE EarthlyBranchName[12][2] = {"子", "丑", "寅", "卯", "辰", "巳", "午", "未", "申", "酉", "戌", "亥"};
	
	RTC_LUNAR_DATE LunarDate;

	SolarToLunar(&gRtcTime, &LunarDate);	
	DBG(("农历 %d年 ", (WORD)LunarDate.Year));
	DBG(("%-.2s%-.2s年 ", HeavenlyStemName[GetHeavenlyStem(LunarDate.Year)], 
						  EarthlyBranchName[GetEarthlyBranch(LunarDate.Year)]));
	DBG(("%-.2s年 ", LunarYearName[GetEarthlyBranch(LunarDate.Year)]));
	if(LunarDate.IsLeapMonth)
	{
		DBG(("闰"));
	}
	DBG(("%-.2s月", LunarMonthName[LunarDate.Month - 1]));
	
	if(LunarDate.MonthDays == 29)
	{
		DBG(("(小)"));
	}
	else
	{
		DBG(("(大)"));
	}
	
	DBG(("%-.4s ", LunarDateName[LunarDate.Date - 1]));

	if((LunarDate.Month == 1) && (LunarDate.Date == 1))			//春节
	{
		DBG(("春节"));
	}
	else if((LunarDate.Month == 1) && (LunarDate.Date == 15))	//元宵节
	{
		DBG(("元宵节"));
	}
	else if((LunarDate.Month == 5) && (LunarDate.Date == 5))	//端午节
	{
		DBG(("端午节"));
	}
	else if((LunarDate.Month == 7) && (LunarDate.Date == 7))	//七夕情人节
	{
		DBG(("七夕情人节"));
	}
	else if((LunarDate.Month == 7) && (LunarDate.Date == 15))	//中元节
	{
		DBG(("中元节"));
	}
	else if((LunarDate.Month == 8) && (LunarDate.Date == 15))	//中秋节
	{
		DBG(("中秋节"));
	}
	else if((LunarDate.Month == 9) && (LunarDate.Date == 9))	//重阳节
	{
   		DBG(("重阳节"));
	}
	else if((LunarDate.Month == 12) && (LunarDate.Date == 8))	//腊八节
	{
	 	DBG(("腊八节"));
	}
	else if((LunarDate.Month == 12) && (LunarDate.Date == 23))	//小年
	{
		DBG(("小年"));
	}
	else if((LunarDate.Month == 12) && (LunarDate.Date == LunarDate.MonthDays))	//除夕
	{
		DBG(("除夕"));
	}
}
#endif


VOID RtcDisplay(VOID)
{
#ifdef FUNC_UARTDBG_EN
	static BYTE TempSec = -1;
#endif

	//Display RTC time
	if(IsTimeOut(&RtcReadTimer))
	{
		TimeOutSet(&RtcReadTimer, 500);
		if(RtcState != RTC_STATE_SET_TIME)
		{
			GetRTCCurrTime(&gRtcTime);
		}
#ifdef FUNC_UARTDBG_EN
		if(gRtcTime.Sec != TempSec)
		{
			TempSec = gRtcTime.Sec;	
			RtcUpdateDisplay = TRUE;
		}
#endif
	}

#ifdef FUNC_UARTDBG_EN
	if(RtcUpdateDisplay)
	{
		RtcUpdateDisplay = FALSE;
		
		if(RtcFlag == 1)
		{
			//YYYY-MM-DD(W) HH:MM:SS
			DBG(("%d-%-.2d-%-.2d(星期%d) ", 
			(WORD)gRtcTime.Year, 
			(WORD)gRtcTime.Mon, 
			(WORD)gRtcTime.Date, 
			(WORD)(gRtcTime.WDay)));

			#ifdef FUNC_LUNAR_EN  
			DisplayLunarDate();
			#endif
				
			DBG((" %-.2d:%-.2d:%-.2d  ",  
			(WORD)gRtcTime.Hour, 
			(WORD)gRtcTime.Min, 
			(WORD)gRtcTime.Sec));
			DBG(("\n"));
		}
#ifdef FUNC_ALARM_EN
		else if(RtcFlag == 2)
		{
			switch(gAlarmMode)
			{
				case ALARM_MODE_ONCE_ONLY:
					DBG(("闹钟模式:单次(once only) "));
					DBG(("%d-%-.2d-%-.2d(星期%d) ", 
					(WORD)gAlarmTime.Year, 
					(WORD)gAlarmTime.Mon, 
					(WORD)gAlarmTime.Date, 
					(WORD)gAlarmTime.WDay));
					break;

				case ALARM_MODE_PER_DAY:
					DBG(("闹钟模式:每天一次(every day)"));
					break;

				case ALARM_MODE_PER_WEEK:
					DBG(("闹钟模式:每周一次(every week) 星期%2BX", gAlarmTime.WDay));
					break;

				default:
					DBG(("模式错误(mode error)\n"));
					break;
			}	

			DBG((" %-.2d:%-.2d:%-.2d  ",  
			(WORD)gAlarmTime.Hour, 
			(WORD)gAlarmTime.Min, 
			(WORD)(gAlarmTime.Sec+1)));
			DBG(("\n"));    
		}
		DBG(("RtcAlarmNum = :%2BX\n",RtcAlarmNum));
#endif
	}
#endif
}


BOOL CheckTimeIsValid(RTC_DATE_TIME* Time)
{
	BOOL TimeModifyFlag = TRUE;
	
	if((Time->Year > RTC_END_YEAR) || (Time->Year < RTC_START_YEAR))
	{
		Time->Year = 1980;
		TimeModifyFlag = FALSE;
	}
	
	if((Time->Mon > 12) || (Time->Mon == 0))
	{
		Time->Mon = 1;
		TimeModifyFlag = FALSE;
	}
	
	if((Time->Date == 0) || (Time->Date > GetMonthDays(Time->Year, Time->Mon)))
	{
		Time->Date = 1;
		TimeModifyFlag = FALSE;
	}

	if(Time->WDay > 6)
	{
		Time->WDay = WeekDayGet(DateToDays(Time));
		TimeModifyFlag = FALSE;
	}

	if(Time->Hour > 23)
	{
		Time->Hour = 0;
		TimeModifyFlag = FALSE;
	}

	if(Time->Min > 59)
	{
		Time->Min  = 0;
		TimeModifyFlag = FALSE;
	}

	if(Time->Sec > 59)
	{
		Time->Sec = 0;
		TimeModifyFlag = FALSE;
	}

	return TimeModifyFlag;
}


#ifdef FUNC_ALARM_EN
BOOL CheckAlarmTime(BYTE AlarmNum, BYTE AlarmMode, RTC_DATE_TIME* Time)
{
	if(GetAlarmStatus(AlarmNum) == 2) 
	{
		DBG(("闹钟由关闭到打开,注意调整闹钟时间\n"));
		if((AlarmMode <= ALARM_MODE_PER_WEEK) && (AlarmMode >= ALARM_MODE_ONCE_ONLY))
		{
			gAlarmMode = AlarmMode;
		}
		else
		{
			gAlarmMode = ALARM_MODE_ONCE_ONLY;
		}
	}
	
	if(AlarmMode == ALARM_MODE_ONCE_ONLY)
	{
		if(CheckTimeIsValid(Time) == FALSE)
		{
			DBG(("闹钟时间非法已纠正\n"));
		}
	}	
	else if(AlarmMode == ALARM_MODE_PER_DAY)
	{
		Time->Year = 1980;
		Time->Mon = 1;
		Time->Date = 1;
	}
	else if(AlarmMode == ALARM_MODE_PER_WEEK)
	{
		Time->Year = 1980;
		Time->Mon = 1;
		if(Time->WDay > 6)
		{			
			Time->WDay = WeekDayGet(DateToDays(Time));
		}
	}
	else
	{
		return FALSE;
	}
	return TRUE;
}
#endif


//这2个函数可以放在lib中,...结束
VOID RtcCtrlInit(VOID)
{	
#ifdef	FUNC_UARTDBG_EN
	 BYTE i;
#endif

#ifdef FUNC_ALARM_EN		
	extern BYTE GetRtcCurAlarmNum(VOID);
	
	InitRtc(gAlarmList, sizeof(gAlarmList) / sizeof(AlARM_INFO));
	gCurAlarmNum = GetRtcCurAlarmNum(); //0表示没有闹钟 
	DBG(("gCurAlarmNum:%02BX\n", gCurAlarmNum));	
#else
	InitRtc(NULL, 0); // 该函数必须调用
#endif

	RtcState = RTC_STATE_IDLE;
	RtcSubState = RTC_SET_NONE;
	RtcUpdateDisplay = FALSE;
	RtcFlag = 1;
	RtcAutOutTimeCount = 0;
	TimeOutSet(&RtcReadTimer, 0);
	TimeOutSet(&RtcSetCntTimer, 0);

#ifdef FUNC_ALARM_EN
#ifdef FUNC_BREAK_POINT_EN
	BP_SaveInfo((BYTE *)&gBreakPointInfo.PowerMemory.AlarmList2Store, sizeof(gBreakPointInfo.PowerMemory.AlarmList2Store));
#endif	

#ifdef	FUNC_UARTDBG_EN
	for(i = 0; i < MAX_ALARM_NUM; i++)
	{
		 DBG(("gpAlarmList[%bu].AlarmMode:%bx,", i, gAlarmList[i].AlarmMode));
		 DBG(("gAlarmList[%bu].AlarmTimeAsSeconds:%lu,", i, gAlarmList[i].AlarmTimeAsSeconds));
		 DBG(("gAlarmList[%bu].AlarmStatus:%bx\n", i, gAlarmList[i].AlarmStatus));
	}
#endif

#ifdef	AU6210K_CAR_MODEL_DEMO
	DBG(("InitRtc2\n"));
	RtcSourceState = RTC_SOURCE_CLOCK;
	DBG(("RtcSourceState=%bu\n", RtcSourceState));
	if(GetAlarmStatus(RtcAlarmNum))
	{
		gAlarmState = ALARM_STATUS_OPEN;
		DBG(("gAlarmState= ALARM_STATUS_OPEN\n"));
	}
	else
	{
		gAlarmState = ALARM_STATUS_CLOSE;
		DBG(("gAlarmState= ALARM_STATUS_CLOSE\n"));
	}			
#endif

	
#ifdef AU6210K_BOOMBOX_DEMO
	if(GetAlarmStatus(1))
	{
		gAlarm1State = ALARM_STATUS_OPEN;
		DBG(("gAlarm1State= ALARM1_STATUS_OPEN\n"));
	}
	else
	{
		gAlarm1State = ALARM_STATUS_CLOSE;
		DBG(("gAlarm1State= ALARM1_STATUS_CLOSE\n"));
	}		

	if(GetAlarmStatus(2))
	{
		gAlarm2State = ALARM_STATUS_OPEN;
		DBG(("gAlarm2State= ALARM2_STATUS_OPEN\n"));
	}
	else
	{
		gAlarm2State = ALARM_STATUS_CLOSE;
		DBG(("gAlarm2State= ALARM2_STATUS_CLOSE\n"));
	}			
#endif
#endif

	
//	//测试用，客户根据实际情况设置。
//	gRtcTime.Year = 1980;
//	gRtcTime.Mon = 1;
//	gRtcTime.Date = 1;
//	gRtcTime.WDay = 1;	// 0:星期日
//	gRtcTime.Hour = 7;
//	gRtcTime.Min = 0; 
//	gRtcTime.Sec = 0;
//  SetRTCCurrTime(&gRtcTime);
//	gAlarmTime.Year = 1980; 
//	gAlarmTime.Mon = 1;
//	gAlarmTime.Date = 1;
//	gAlarmTime.WDay = 1;
//	gAlarmTime.Hour = 8;
//	gAlarmTime.Min = 0;
//	gAlarmTime.Sec = 0;
//  gAlarmMode = ALARM_MODE_ONCE_ONLY;
//  SetAlarmTime(RtcAlarmNum, gAlarmMode, &gAlarmTime);

#ifdef AU6210K_BOOMBOX_DEMO	
	gRtcTime.Hour = 12;
	gRtcTime.Min = 0; 
	gRtcTime.Sec = 0;
	SetRTCCurrTime(&gRtcTime);
	
	gAlarmTime.Hour = 0;
	gAlarmTime.Min = 0;
	gAlarmTime.Sec = 0;
	gAlarmMode = ALARM_MODE_PER_DAY;
	SetAlarmTime(1, gAlarmMode, &gAlarmTime);
	SetAlarmTime(2, gAlarmMode, &gAlarmTime);
#endif
}


static BYTE RtcAlarmSetWaitTime(VOID)
{
	if(RtcAutOutTimeCount == RTC_AUTO_OUT_TIME_COUNT)
	{
		TimeOutSet(&RtcAutoOutTimer, RTC_AUTO_OUT_TIME_UNIT);
		RtcAutOutTimeCount--;
	}
	else if(IsTimeOut(&RtcAutoOutTimer))
	{				
		if(RtcAutOutTimeCount > 0)
		{
			TimeOutSet(&RtcAutoOutTimer, RTC_AUTO_OUT_TIME_UNIT);
			RtcAutOutTimeCount--;
		}
		else
		{		
			RtcState = RTC_STATE_IDLE;
			RtcSubState = RTC_SET_NONE;
			return TRUE;
		}
	}
	return FALSE;;
}

#if (defined(AU6210K_CAR_MODEL_DEMO))
static VOID RtcSourceDown(VOID)
{
	switch(RtcSourceState)
	{
		case RTC_SOURCE_CLOCK:
			RtcSourceState = RTC_SOURCE_ONOFF;
			DBG(("RTC_SOURCE_ONOFF\n"));
			break;
			
		case RTC_SOURCE_YEAR:
			RtcSourceState = RTC_SOURCE_CLOCK;
			DBG(("RTC_SOURCE_CLOCK\n"));
			break;

		case RTC_SOURCE_MON:
			RtcSourceState = RTC_SOURCE_YEAR;
			DBG(("RTC_SOURCE_YEAR\n"));
			break;

		case RTC_SOURCE_ALARM:
			RtcSourceState = RTC_SOURCE_MON;
			DBG(("RTC_SOURCE_MON\n"));
			break;

		case RTC_SOURCE_ONOFF:
			RtcSourceState = RTC_SOURCE_ALARM;
			DBG(("RTC_SOURCE_ALARM\n"));
			break;

		default:
			break;			
	}
}

static VOID RtcSourceUp(VOID)
{
	switch(RtcSourceState)
	{
		case RTC_SOURCE_CLOCK:
			RtcSourceState = RTC_SOURCE_YEAR;
			DBG(("RTC_SOURCE_YEAR\n"));
			break;
			
		case RTC_SOURCE_YEAR:
			RtcSourceState = RTC_SOURCE_MON;
			DBG(("RTC_SOURCE_MON\n"));
			break;

		case RTC_SOURCE_MON:
			RtcSourceState = RTC_SOURCE_ALARM;
			DBG(("RTC_SOURCE_ALARM\n"));
			break;

		case RTC_SOURCE_ALARM:
			RtcSourceState = RTC_SOURCE_ONOFF;
			DBG(("RTC_SOURCE_ONOFF\n"));
			break;

		case RTC_SOURCE_ONOFF:
			RtcSourceState = RTC_SOURCE_CLOCK;
			DBG(("RTC_SOURCE_CLOCK\n"));
			break;

		default:
			break;			
	}
}

static VOID RtcSourceEnter(VOID)
{
	RtcFlag = 1;
	
	switch(RtcSourceState)
	{
		case RTC_SOURCE_CLOCK:
			RtcSubState = RTC_SET_HR;
			RtcState = RTC_STATE_SET_TIME;
			DBG(("RTC_SOURCE_CLOCK\n"));
			break;
			
		case RTC_SOURCE_YEAR:
			RtcSubState = RTC_SET_YEAR;
			RtcState = RTC_STATE_SET_TIME;
			gRtcRdWrFlag = SET_RTC_TIME_YEAR;
			DBG(("RTC_SOURCE_YEAR\n"));
			break;

		case RTC_SOURCE_MON:
			RtcSubState = RTC_SET_MON;
			RtcState = RTC_STATE_SET_TIME;
			DBG(("RTC_SOURCE_MON\n"));
			break;

		case RTC_SOURCE_ALARM:
			RtcFlag = 2;
			RtcUpdateDisplay = TRUE;
			if((RtcAlarmNum > 0) && (RtcAlarmNum < MAX_ALARM_NUM))
			{
				gAlarmMode = GetAlarmTime(RtcAlarmNum, &gAlarmTime);
				if((gAlarmMode < ALARM_MODE_ONCE_ONLY) || (gAlarmMode > ALARM_MODE_PER_WEEK))
				 {
					gAlarmMode = ALARM_MODE_PER_DAY;	
				}

				CheckAlarmTime(RtcAlarmNum, gAlarmMode, &gAlarmTime);									
			}
			else
			{
				DBG(("闹钟错误\n"));
			}
			RtcSubState = RTC_SET_HR;
			RtcState = RTC_STATE_SET_ALARM;
			DBG(("RTC_SOURCE_ALARM\n"));
			break;

		case RTC_SOURCE_ONOFF:
			RtcSubState = RTC_SET_ALARM_ONOFF;
			RtcState = RTC_STATE_SET_ALARM;
			if(GetAlarmStatus(RtcAlarmNum))
			{
				gAlarmState = ALARM_STATUS_OPEN;
			}
			else
			{
				gAlarmState = ALARM_STATUS_CLOSE;
			}
			DBG(("RTC_SOURCE_ONOFF\n"));
			break;

		default:
			break;			
	}
}
#endif



//如果有和此函数类似用法(嵌入到各个模式)的函数位于其后，此函数不要有default，防止拦截后者有用的消息
VOID RtcMsgPro(BYTE Msg)
{	
	if(RtcState == RTC_STATE_IDLE)
	{
#ifdef	AU6210K_CAR_MODEL_DEMO
		if((Msg != MSG_NONE) && (gAlarmOnFlag == TRUE))
		{
			gAlarmOnFlag = FALSE;
		}
#endif
		
		switch(Msg)
		{	
#ifdef	AU6210K_CAR_MODEL_DEMO
			case MSG_REPEAT:
#endif
#ifdef AU6210K_BOOMBOX_DEMO
			case MSG_CLOCK:
#endif
			case MSG_RTC_SET:
				DBG(("RTC_SET\n"));
				RtcFlag = 1;
				RtcUpdateDisplay = TRUE;
				RtcAutOutTimeCount = RTC_AUTO_OUT_TIME_COUNT;
#ifdef AU6210K_BOOMBOX_DEMO
				RtcSubState = RTC_SET_HR;
				RtcState = RTC_STATE_SET_TIME;
				break;
#endif
				RtcState = RTC_STATE_SET_TIME;
				RtcSubState = RTC_SET_NONE;
				
#ifdef	AU6210K_CAR_MODEL_DEMO
				if(Msg == MSG_REPEAT)
#else
				if(Msg == MSG_RTC_SET)
#endif
				{
					gRtcRdWrFlag = SET_RTC_TIME_YEAR;
				}
				else 
				{
					gRtcRdWrFlag = READ_RTC_TIME;
				}
				RtcNextSubState();
				break;

#ifdef FUNC_ALARM_EN
#ifdef AU6210K_BOOMBOX_DEMO
			case MSG_ALARM1_SET:
			case MSG_ALARM2_SET:
#endif
			case MSG_ALARM_SET:
#ifndef	AU6210K_CAR_MODEL_DEMO
				DBG(("ALARM_SET\n"));
				RtcFlag = 2;
				RtcUpdateDisplay = TRUE;
				RtcAutOutTimeCount = RTC_AUTO_OUT_TIME_COUNT;
#ifdef AU6210K_BOOMBOX_DEMO
				if(Msg == MSG_ALARM1_SET)
				{
					RtcAlarmNum = 1;
				}
				else if(Msg == MSG_ALARM2_SET)
				{
					RtcAlarmNum = 2;
				}
#endif
	
				if((RtcAlarmNum > 0) && (RtcAlarmNum <= MAX_ALARM_NUM))
				{
					gAlarmMode = GetAlarmTime(RtcAlarmNum, &gAlarmTime);
					if((gAlarmMode < ALARM_MODE_ONCE_ONLY) || (gAlarmMode > ALARM_MODE_PER_WEEK))
					{
						gAlarmMode = ALARM_MODE_PER_DAY;	
					}
					CheckAlarmTime(RtcAlarmNum, gAlarmMode, &gAlarmTime);									
				}
				else
				{
					DBG(("闹钟错误\n"));
				}
				
#ifdef AU6210K_BOOMBOX_DEMO
				RtcSubState = RTC_SET_HR;
				RtcState = RTC_STATE_SET_ALARM;
				break;
#endif
				RtcState = RTC_STATE_SET_ALARM;
				RtcSubState = RTC_SET_NONE;
				RtcNextSubState();
#endif
#endif
				break;
								
			default:
				break;
		}
	}
	else if(RtcState == RTC_STATE_SET_TIME)
	{
		switch(Msg)
		{
#ifdef	AU6210K_CAR_MODEL_DEMO
			case MSG_PRE:
#endif
#ifdef AU6210K_BOOMBOX_DEMO
			case MSG_PREV1:
#endif
			case MSG_RTC_DOWN:
				DBG(("RTC_DOWN\n"));
				RtcUpdateDisplay = TRUE;
				GetRTCCurrTime(&gRtcTime);
				RtcTimeDown(&gRtcTime);
				SetRTCCurrTime(&gRtcTime);
				RtcAutOutTimeCount = RTC_AUTO_OUT_TIME_COUNT;
				break;

#ifdef	AU6210K_CAR_MODEL_DEMO
			case MSG_NEXT:
#endif
#ifdef AU6210K_BOOMBOX_DEMO
			case MSG_NEXT1:
#endif
			case MSG_RTC_UP:
				DBG(("RTC_UP\n"));
				RtcUpdateDisplay = TRUE;
				GetRTCCurrTime(&gRtcTime);
				RtcTimeUp(&gRtcTime);
				SetRTCCurrTime(&gRtcTime);
				RtcAutOutTimeCount = RTC_AUTO_OUT_TIME_COUNT;
				break;

#ifdef	AU6210K_CAR_MODEL_DEMO
			case MSG_PLAY_PAUSE:
#endif
#ifdef	AU6210K_BOOMBOX_DEMO
			case MSG_CLOCK:
#endif
			case MSG_RTC_SW:
				DBG(("RTC_SW\n"));
#ifdef AU6210K_BOOMBOX_DEMO
				if(RtcSubState == RTC_SET_MIN)
				{
					RtcState = RTC_STATE_IDLE;
					RtcSubState = RTC_SET_NONE;
					RtcAutOutTimeCount = 0;
					break;
				}
#endif
				RtcUpdateDisplay = TRUE;
				RtcNextSubState();
				RtcAutOutTimeCount = RTC_AUTO_OUT_TIME_COUNT;
				break;

			default:
				break;
		}

		if(TRUE == RtcAlarmSetWaitTime())
		{		
			//这样设置时间是因为SetRtcRefcnt函数有延时如果此时在播放mp3按键有卡声音	
			if(gRtcRdWrFlag != READ_RTC_TIME)
			{
				SetRTCCurrTime(&gRtcTime);
			}
		}
	}
#ifdef FUNC_ALARM_EN
	else if(RtcState == RTC_STATE_SET_ALARM)
	{
		switch(Msg)
		{
#ifdef	AU6210K_CAR_MODEL_DEMO
			case MSG_PRE:
#endif

#ifdef AU6210K_BOOMBOX_DEMO
			case MSG_PREV1:
#endif
			case MSG_RTC_DOWN:
				DBG(("ALARM RTC_DOWN\n"));
				RtcUpdateDisplay = TRUE;      				
				RtcTimeDown(&gAlarmTime);
				CheckAlarmTime(RtcAlarmNum, gAlarmMode, &gAlarmTime);
				SetAlarmTime(RtcAlarmNum, gAlarmMode, &gAlarmTime);
				RtcAutOutTimeCount = RTC_AUTO_OUT_TIME_COUNT;
#ifdef FUNC_BREAK_POINT_EN
				BP_SaveInfo((BYTE*)&gBreakPointInfo.PowerMemory.AlarmList2Store, sizeof(gBreakPointInfo.PowerMemory.AlarmList2Store));
#endif
				break;

#ifdef	AU6210K_CAR_MODEL_DEMO
			case MSG_NEXT:
#endif

#ifdef AU6210K_BOOMBOX_DEMO
			case MSG_NEXT1:
#endif

			case MSG_RTC_UP:
				DBG(("ALARM RTC_UP\n"));
				RtcUpdateDisplay = TRUE;               				
				RtcTimeUp(&gAlarmTime);
				CheckAlarmTime(RtcAlarmNum, gAlarmMode, &gAlarmTime);
				SetAlarmTime(RtcAlarmNum, gAlarmMode, &gAlarmTime);
				RtcAutOutTimeCount = RTC_AUTO_OUT_TIME_COUNT;
#ifdef FUNC_BREAK_POINT_EN
				BP_SaveInfo((BYTE*)&gBreakPointInfo.PowerMemory.AlarmList2Store, sizeof(gBreakPointInfo.PowerMemory.AlarmList2Store));
#endif
				break;

#ifdef	AU6210K_CAR_MODEL_DEMO
			case MSG_PLAY_PAUSE:
#endif
#ifdef AU6210K_BOOMBOX_DEMO
			case MSG_ALARM1_SET:
			case MSG_ALARM2_SET:
#endif
			case MSG_RTC_SW:
				DBG(("ALARM SW\n"));
#ifdef AU6210K_BOOMBOX_DEMO
				if(((RtcAlarmNum == 1) && (Msg != MSG_ALARM1_SET)) || ((RtcAlarmNum == 2) && (Msg != MSG_ALARM2_SET)))			
				{
					break;
				}

				if(RtcSubState == RTC_SET_ALARM_VOLUME)
				{					
					RtcState = RTC_STATE_IDLE;
					RtcSubState = RTC_SET_NONE;
					RtcAutOutTimeCount = 0;
					break;
				}
#endif
				RtcUpdateDisplay = TRUE;
				RtcNextSubState();		
				RtcAutOutTimeCount = RTC_AUTO_OUT_TIME_COUNT;
				break;

			default:
				break;
		}

		if(TRUE == RtcAlarmSetWaitTime())
		{
			DBG(("EXIT RTC_STATE_SET_TIME!\n"));		
			RtcState = RTC_STATE_IDLE;
			RtcSubState = RTC_SET_NONE;
			RtcFlag = 1;		
		}
	}
#endif
#ifdef	AU6210K_CAR_MODEL_DEMO
	else if(RtcState == RTC_STATE_SET_SOURCE)
	{
		switch(Msg)
		{
			case MSG_PRE:			
				DBG(("RTC SOURCE_DOWN\n"));
				RtcUpdateDisplay = TRUE;
				RtcSourceDown();
				RtcAutOutTimeCount = RTC_AUTO_OUT_TIME_COUNT;
				break;

			case MSG_NEXT:
				DBG(("RTC SOURCE_UP\n"));
				RtcUpdateDisplay = TRUE;
				RtcSourceUp();
				RtcAutOutTimeCount = RTC_AUTO_OUT_TIME_COUNT;
				break;

			case MSG_REPEAT:
				DBG(("RTC SOURCE_Enter\n"));
				RtcUpdateDisplay = TRUE;
				RtcSourceEnter();
				RtcAutOutTimeCount = RTC_AUTO_OUT_TIME_COUNT;
				break;
				
			default:
				break;
		}

		if(TRUE == RtcAlarmSetWaitTime())
		{
			DBG(("EXIT RTC_STATE_SET_SOURCE!\n"));		
			RtcState = RTC_STATE_IDLE;
			RtcSubState = RTC_SET_NONE;
			RtcFlag = 1;		
		}	
	}
#endif
}


VOID RtcStateCtrl(VOID)
{
	MESSAGE Event = MessageGet(MSG_FIFO_RTC);

#ifdef FUNC_STANDBY_EN
	if(gSys.SystemMode == SYS_MODE_STANDBY)
#endif
	{
		RtcMsgPro(Event);
	}
	
#ifdef FUNC_ALARM_EN	
	if(RtcSubState == RTC_SET_NONE)
	{
		//Check alarm
		if(gCurAlarmNum = CheckRtcAlarmFlag())
		{
			DBG(("\nRTC ALARM COME!\n"));
			
#ifdef	AU6210K_CAR_MODEL_DEMO
			gAlarmOnFlag = TRUE;
#endif
#ifdef FUNC_BREAK_POINT_EN
			BP_SaveInfo((BYTE*)&gBreakPointInfo.PowerMemory.AlarmList2Store, sizeof(gBreakPointInfo.PowerMemory.AlarmList2Store));
#endif

#ifdef AU6210K_BOOMBOX_DEMO
			if(gCurAlarmNum == 1)
			{
				gAlarm1OnFlag = TRUE;
			}
			else
			{
				gAlarm2OnFlag = TRUE;
			}
#endif
		}
	}
#endif

	//Display time
	RtcDisplay();
}


#endif

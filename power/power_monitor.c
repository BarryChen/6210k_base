#include "syscfg.h"
#include "message.h"
#include "debug.h"
#include "key.h"
#include "sysctrl.h"
#include "adc.h"
#include "timer.h"
#include "utility.h"
#include "power.h"
#include "device.h"
#include "display.h"
#include "Sys_on_off.h"
#include "Nvm.h"
#include "power_monitor.h"
#include "sled_disp.h"

#ifdef FUNC_DISP_EN
#include "seg_panel.h"
#include "Lcd_seg.h"
#endif
extern VOID UnMute(VOID);
extern BOOL GetBtMuteFlag();

#ifdef FUNC_POWER_MONITOR_EN

#define LDOIN_SAMPLE_COUNT			100		//获取LDOIN幅度时用来平均的采样次数
#define LDOIN_SAMPLE_PERIOD			50		//获取LDOIN幅度时获取采样值的间隔(ms)
#define LOW_POWEROFF_TIME			10000		//低电检测关机连续检测时间(ms)


//以下定义不同的电压检测事件的触发电压(单位mV)，用户根据自身系统电池的特点来配置
#ifdef AU6210K_NR_D_8_CSRBT

#define LDOIN_VOLTAGE_FULL			58
#define LDOIN_VOLTAGE_HIGH			50
#define LDOIN_VOLTAGE_LOW			52
#define LDOIN_VOLTAGE_OFF			47	//低于此电压值进入关机powerdown状态

#else

#define LDOIN_VOLTAGE_FULL			4200
#define LDOIN_VOLTAGE_HIGH			3600
#define LDOIN_VOLTAGE_LOW			3500
#if defined(AU6210K_NR_D_9X_XJ_HTS) ||defined(AU6210K_LK_SJ_CSRBT) || defined(AU6210K_ZB_BT007_CSR)
#define LDOIN_VOLTAGE_OFF			3400	//低于此电压值进入关机powerdown状态
#else 
#define LDOIN_VOLTAGE_OFF			3300
#endif

#endif
//电压检测时不同的显示处理


//主要用于电压状态显示的变量
static TIMER BlinkTimer;
static PWR_MNT_DISP PwrMntDisp;
static BOOL LowPwr_Flag = TRUE;



//用于电压检测的变量
TIMER PowerMonitorTimer;
TIMER PowerOffTimer;
DWORD LdoinSampleSum = 0; 
WORD  LdoinSampleCnt = LDOIN_SAMPLE_COUNT;
DWORD LdoinLevelAverage = 0;		//当前LDOIN电压平均值

extern BOOL SongPlayDo(VOID);		
extern BOOL SPI_PlaySelectNum(BYTE SpiPlayNum, BOOL PlayResumeFlag);

PWR_MNT_DISP GetPwrDisp(void)
{
	return PwrMntDisp;
}
VOID PowerMonitorDisp(VOID)
{
#if (defined(FUNC_SEG_LED_EN) || defined(FUNC_SEG_LCD_EN))
	static BOOL IsToShow = FALSE;

	switch(PwrMntDisp)
	{
		case PWR_MNT_DISP_CHARGE:
			//闪烁充电ICON,表示正在充电
		    if(IsTimeOut(&BlinkTimer))
			{
				TimeOutSet(&BlinkTimer, 500);
				if(IsToShow)
				{
					//DBG(("Charge 111\n"));						
					DispIcon(ICON_BATFUL, TRUE);
					DispIcon(ICON_BATHAF, FALSE);
	                IsToShow = FALSE;
				}
				else
				{
					//DBG(("Charge 000\n"));						
					DispIcon(ICON_BATFUL, FALSE);
					DispIcon(ICON_BATHAF, TRUE);
					IsToShow = TRUE;
				}
			}
			break;

		case PWR_MNT_DISP_HIGH_V:
			//DBG(("BAT FULL\n"));				
			DispIcon(ICON_BATFUL, TRUE);
			DispIcon(ICON_BATHAF, FALSE);
			//可以显示满格电量，请添加显示代码
			break;
			
		case PWR_MNT_DISP_MID_V:
			//DBG(("BAT HALF\n"));				
			DispIcon(ICON_BATFUL, FALSE);
			DispIcon(ICON_BATHAF, TRUE);
			//可以显示2格电量，请添加显示代码
			break;

		case PWR_MNT_DISP_LOW_V:
			DispIcon(ICON_BATFUL, FALSE);
			DispIcon(ICON_BATHAF, TRUE);
			//可以显示1格电量，请添加显示代码
			break;
			
		case PWR_MNT_DISP_EMPTY_V:
			//DBG(("BAT EMPTY\n"));				
			DispIcon(ICON_BATFUL, FALSE);

			if(IsTimeOut(&BlinkTimer))
			{
				TimeOutSet(&BlinkTimer, 300);
				if(IsToShow)
				{
					DispIcon(ICON_BATHAF, TRUE);
				}
				else
				{
					DispIcon(ICON_BATHAF, FALSE);
				}
				IsToShow = !IsToShow;
			}
			//可以显示0格电量，请添加显示代码
			break;
			
		case PWR_MNT_DISP_SYS_OFF:
			//DBG(("BAT OFF\n"));
			ClearScreen();			//清除显示				
			DispString(" LO ");
			break;
			
		default:
			break;
	}
#endif	
}


//监测LDOIN的电压值，执行对应检测点的处理
//PowerOnInitFlag: TRUE--第一次上电执行电源监控检测
static VOID PowerLdoinLevelMonitor(BOOL PowerOnInitFlag)
{	
	BOOL PowerOffFlag = FALSE;
		
	if(LdoinSampleCnt > 0)
	{
#ifdef AU6210K_NR_D_8_CSRBT
		LdoinSampleSum +=  (AdcReadOneChannel(ADC_CHANNEL_B6, ADC_VREF_AVDD33)
			+ AdcReadOneChannel(ADC_CHANNEL_B6, ADC_VREF_AVDD33) 
		 	+ AdcReadOneChannel(ADC_CHANNEL_B6, ADC_VREF_AVDD33) + 2) / 3;	
#else
		LdoinSampleSum += GetLdoinVoltage();			
#endif
		LdoinSampleCnt--;
	}

	//采样够LDOIN_SAMPLE_COUNT次数，求LDOIN平均值
	if(LdoinSampleCnt == 0)
	{
		LdoinLevelAverage = LdoinSampleSum / LDOIN_SAMPLE_COUNT;

		DBG(("LODin 5V Volt: %lu", (DWORD)LdoinLevelAverage));

		//为继续检测LDOIN，初始化变量
		LdoinSampleCnt = LDOIN_SAMPLE_COUNT;
		LdoinSampleSum = 0;

#ifdef	OPTION_CHARGER_DETECT
		if(IsInCharge())		//充电器已经接入的处理
		{		
			return;
		}
#endif
		
		if(LdoinLevelAverage > LDOIN_VOLTAGE_HIGH)	  
		{
			//可以显示满格电量，请在PowerMonitorDisp中添加显示代码
			//PowerMonitorDisp(PWR_MNT_DISP_HIGH_V);
			PwrMntDisp = PWR_MNT_DISP_HIGH_V;
			//DBG(("bat full\n"));
		}
		/*
		else if(LdoinLevelAverage > LDOIN_VOLTAGE_MID)
		{
			//可以显示2格电量，请在PowerMonitorDisp中添加显示代码
			//PowerMonitorDisp(PWR_MNT_DISP_MID_V);
			PwrMntDisp = PWR_MNT_DISP_MID_V;
		}
		//*/
		else if(LdoinLevelAverage > LDOIN_VOLTAGE_LOW)
		{
			//可以显示1格电量，请在PowerMonitorDisp中添加显示代码
			//PowerMonitorDisp(PWR_MNT_DISP_LOW_V);
			PwrMntDisp = PWR_MNT_DISP_LOW_V;
			//DBG(("bat LOW\n"));
		}
		else if(LdoinLevelAverage > LDOIN_VOLTAGE_OFF)
		{
			//可以显示0格电量，请在PowerMonitorDisp中添加显示代码
			//PowerMonitorDisp(PWR_MNT_DISP_EMPTY_V);
			PwrMntDisp = PWR_MNT_DISP_EMPTY_V;
		}

		if((LdoinLevelAverage > LDOIN_VOLTAGE_OFF) && (LdoinLevelAverage < LDOIN_VOLTAGE_LOW))
			PwrMntDisp = PWR_MNT_DISP_EMPTY_V;
	
		#if defined(AU6210K_NR_D_9X_XJ_HTS) || defined(AU6210K_NR_D_8_CSRBT) || defined(AU6210K_LK_SJ_CSRBT) || defined(AU6210K_ZB_BT007_CSR)
		if(PowerOnInitFlag == TRUE) 
		{		
			if(LdoinLevelAverage <= LDOIN_VOLTAGE_OFF)
			{
				PowerOffFlag = TRUE;
			}			
		}
		#else
		if(PowerOnInitFlag == TRUE) 
		{		
			if(LdoinLevelAverage <= LDOIN_VOLTAGE_LOW)
			{
				PowerOffFlag = TRUE;
			}			
		}
        #endif
		if((PowerOnInitFlag == FALSE) && (LdoinLevelAverage <= LDOIN_VOLTAGE_OFF))
		{
			PowerOffFlag = TRUE;
		}
		
		if(PowerOffFlag == TRUE)
		{
			//低于关机电压，进入关机流程
			//可以先行显示低电压提示，然后执行关机动作
			PwrMntDisp = PWR_MNT_DISP_SYS_OFF;
			PowerMonitorDisp();			
			FeedWatchDog();
			#if defined(AU6210K_NR_D_9X_XJ_HTS)|| defined(AU6210K_NR_D_8_CSRBT)
	            SPI_PlaySelectNum(SPIPLAY_SONG_LOW_PWR,0);
		    //	SLedLightOp(LED_POWER, FALSE);	
				WaitMs(1000);
			#endif
			#if defined(AU6210K_ZB_BT007_CSR)
			if(LowPwr_Flag)
			{
				LowPwr_Flag = FALSE;
				TimeOutSet(&PowerOffTimer, 10000);
				SPI_PlaySelectNum(SPIPLAY_SONG_LOW_PWR,1);
				WaitMs(100);
				if(gSys.SystemMode == SYS_MODE_BLUETOOTH && !GetBtMuteFlag())
				{
				DBG1(("ccccccccpower off\m"));
					UnMute();
				}
				
			}
			else
			{
				if(IsTimeOut(&PowerOffTimer))
				{
					SPI_PlaySelectNum(SPIPLAY_SONG_LOW_PWR,0);
					WaitMs(1000);
					SystemOff();
				}
			
			}
			#endif
			//停止正常工作流程，包括关显示、关DAC、关功放电源等动作
			DBG(("PowerMonitor, PD\n"));	
			#if defined(AU6210K_ZB_BT007_CSR)
			#else
			SystemOff();
			#endif
		}		
	}
	PowerMonitorDisp();
}


//电能监视初始化
//实现系统启动过程中的低电压检测处理，以及初始化充电设备接入检测IO等
VOID PowerMonitorInit(VOID)
{
	TimeOutSet(&PowerMonitorTimer, 0);	
	TimeOutSet(&BlinkTimer, 0);	

#ifdef OPTION_CHARGER_DETECT
	//如果系统启动时，充电设备已经接入，就不执行下面这段低电压检测和处理过程
	if(!IsInCharge())
#endif	
	{
		//系统启动过程中的低电压检测
		//开机时电压检测，如果小于开机电压，不进入设备检测和播放流程，直接关机
		//检测过程为时50ms，并进行对应的处理
		while(LdoinSampleCnt)
		{
			LdoinSampleCnt--;
#ifdef AU6210K_NR_D_8_CSRBT
			LdoinSampleSum +=  (AdcReadOneChannel(ADC_CHANNEL_B6, ADC_VREF_AVDD33)
				+ AdcReadOneChannel(ADC_CHANNEL_B6, ADC_VREF_AVDD33) 
			 	+ AdcReadOneChannel(ADC_CHANNEL_B6, ADC_VREF_AVDD33) + 2) / 3;	
#else
			LdoinSampleSum += GetLdoinVoltage();			
#endif			
			WaitMs(5);
		}		
		//为后边的LDOIN检测初始化变量
		PowerLdoinLevelMonitor(TRUE);
	}
}


//系统电源状态监控和处理
//系统启动后如果LDOIN大于开机电压，放在系统正常运行中监测LDOIN
VOID PowerMonitor(VOID)
{
	if(IsTimeOut(&PowerMonitorTimer))
	{
		TimeOutSet(&PowerMonitorTimer, LDOIN_SAMPLE_PERIOD);

#ifdef OPTION_CHARGER_DETECT
		if(IsInCharge())		//充电器已经接入的处理
		{
			if(LdoinLevelAverage >= LDOIN_VOLTAGE_FULL) 
			{
				//PowerMonitorDisp(PWR_MNT_DISP_HIGH_V);		//显示充电状态
				PwrMntDisp = PWR_MNT_DISP_HIGH_V;
				//DBG(("charger full\n"));
			}
			else
			{
				//PowerMonitorDisp(PWR_MNT_DISP_CHARGE);		//显示充电状态
				PwrMntDisp = PWR_MNT_DISP_CHARGE;
				//DBG(("charger.....\n"));
			}
		}
#endif		
		//没有采样够LDOIN_SAMPLE_COUNT次数，继续采样
		PowerLdoinLevelMonitor(FALSE);
	}
}


#endif

#include <reg51.h>

#include "type.h"
#include "syscfg.h"
#include "utility.h"
#include "ir_key.h"
#include "adc_key.h"
#include "sysctrl.h"
#include "message.h"
#include "timer.h"
#include "power_key.h"
#include "key.h"
#include "radio_ctrl.h"
#include "debug.h"
#include "gpio_key.h"
#include "display.h"
#include "coding_key.h"
#include "touch_key.h"
#include "spi_fs.h"
#include "bt.h"


#ifdef	FUNC_NUMBER_KEY_EN
#define  IR_KEY_NUM_WAIT_TIME  2000
#define  IR_KEY_NUM_FILL_ZERO_WAIT_TIME  1000

extern BOOL BTPowerOffTime_Start;
extern TIMER   CSR_BTPOWEROFF_TIME;

extern BOOL SDCARDPowerOffTime_Start;
extern TIMER   SDCARDPOWEROFF_TIME;


BOOL	gIRNUM10FG = 0;
WORD	gRecvNum = 0;
TIMER	RecvNumTimer;
BOOL NumKeyRadioFreqErrorFlag = FALSE;
BOOL NumKeyErrorDispFlag = FALSE;
#endif

extern VOID UnMute(VOID);
extern VOID MuteOn(BOOL DacMuteEn, BOOL ExMuteEn);
#ifdef FUNC_IR_KEY_EN
static BOOL IrIsOn = TRUE;
#endif

#ifdef FUNC_TOUCH_KEY_EN
extern TIMER TouchKeyInitTimer;
#endif

extern BOOL GetBtMuteFlag();

#ifdef AU6210K_ZB_BT007_IR_IC_IS_334M_CSR
BOOL GetIrIsOn(void)
{
	return IrIsOn;
}
#endif
#ifdef FUNC_BLUETOOTH_CSR_EN
MESSAGE IsDoubleClick(MESSAGE event)
{
	TIMER DoubleClickTimer;
	static BYTE ClickNum = 0;
#ifdef AU6210K_ZB_BT007_CSR
	if (event == MSG_BLUETOOTH_ANSWER)
#else
	if (event == MSG_PLAY_PAUSE)
#endif		
	{
		if (ClickNum >= 2)
			ClickNum = 0;
		ClickNum++;

		if (ClickNum == 1)
		{
			TimeOutSet(&DoubleClickTimer,500);
		}
	}
	if (!IsTimeOut(&DoubleClickTimer) && ClickNum == 1)
	{
		event = MSG_NONE;
	}
	else if (!IsTimeOut(&DoubleClickTimer) && ClickNum == 2)
	{
		event = MSG_BT_RECALL;
		ClickNum = 0;
	}
	else if(IsTimeOut(&DoubleClickTimer) && ClickNum == 1)
	{
			
#ifdef AU6210K_ZB_BT007_CSR
		event = MSG_BLUETOOTH_ANSWER;
#else
		event = MSG_PLAY_PAUSE;
#endif	
		ClickNum = 0;
#if defined(AU6210K_ZB_BT007_CSR) && defined(AU6210K_ZB_BT007_IR_IC_IS_334M_CSR)//juest using 168
		if(GetIrIsOn())
		{
			SPI_PlaySelectNum(SPIPLAY_SONG_IR_KEY, 0);
			if(gSys.SystemMode == SYS_MODE_BLUETOOTH && !GetBtMuteFlag())
			{
			DBG1(("ccccccccddddddddddddd\m"));
				UnMute();
			}
		}
#endif		

	}

	return event;
}    
#endif

static MESSAGE GetKeyEventA(VOID)							
{
	MESSAGE	Event = MSG_NONE;
#if	(PWR_KEY_MODE == PWR_KEY_PUSH_BUTTON)
	if(Event == MSG_NONE)
	{
		Event = PowerKeyEventGet();
	}
#endif

#ifdef FUNC_ADC_KEY_EN
	if(Event == MSG_NONE)
	{
		Event = AdcKeyEventGet();
#if 0//def FUNC_BLUETOOTH_CSR_EN
        if(gSys.SystemMode == SYS_MODE_BLUETOOTH)
			Event = IsDoubleClick(Event);
#endif
#ifdef AU6210K_ZB_BT007_IR_IC_IS_334M_CSR
			
			if(!IrIsOn && (Event != MSG_IR_ON_OFF))
			{
#ifdef FUNC_SPI_KEY_SOUND_EN	
				if(Event != MSG_NONE)
				{
					SPI_PlaySelectNum(SPIPLAY_SONG_LOCKING, 1);
				}
#endif
	
				Event = MSG_NONE;
			}
			if(Event == MSG_IR_ON_OFF)
			{
				if(IrIsOn)
				{
					IrIsOn = FALSE;
					
#ifdef FUNC_SPI_KEY_SOUND_EN
					if(gSys.MuteFg)
						SPI_PlaySelectNum(SPIPLAY_SONG_LOCK_ON, 0); 			
					else
						SPI_PlaySelectNum(SPIPLAY_SONG_LOCK_ON, 1);
#endif
				}
				else
				{
					IrIsOn = TRUE;
					
#ifdef FUNC_SPI_KEY_SOUND_EN
					if(gSys.MuteFg)
						SPI_PlaySelectNum(SPIPLAY_SONG_LOCK_OFF, 0);			
					else
						SPI_PlaySelectNum(SPIPLAY_SONG_LOCK_OFF, 1);
#endif
				}
				baGPIOCtrl[GPIO_A_OUT] &= ~0x40; //A2
				WaitMs(500);
				
			}
			if(Event != MSG_IR_ON_OFF && Event != MSG_NONE && Event != MSG_VOL_ADD && Event != MSG_VOL_SUB)
			{
				if(Event == MSG_MODE_SW)
				{
					SPI_PlaySelectNum(SPIPLAY_SONG_IR_KEY, 0);
					MuteOn(TRUE,FALSE);
				}else
				{
					if(gSys.SystemMode == SYS_MODE_BLUETOOTH)
					{	
						if((Event != MSG_VOL_ADD) && (Event != MSG_VOL_SUB)&&(Event != MSG_BLUETOOTH_REJECT)&& Event != MSG_BLUETOOTH_ANSWER)
							SPI_PlaySelectNum(SPIPLAY_SONG_IR_KEY, 0);
					}
					else
					{
						if(Event == MSG_PLAY_PAUSE)// || Event == MSG_NEXT || Event == MSG_PRE)
							SPI_PlaySelectNum(SPIPLAY_SONG_IR_KEY, 0);
						else
							SPI_PlaySelectNum(SPIPLAY_SONG_IR_KEY, 1);
					}
					if(gSys.SystemMode == SYS_MODE_BLUETOOTH && !GetBtMuteFlag())
					{
						DBG1(("ccccccccddddddddddddd\m"));
						UnMute();
					}
				}
			}
			
			/*if(Event == MSG_V_ADD)
				Event = MSG_VOL_ADD;
			if(Event == MSG_V_SUB)
				Event = MSG_VOL_SUB;*/
			if(Event == MSG_BT_REJECT)
				Event =MSG_BLUETOOTH_REJECT;
	
#endif			
		
	}
#endif

#ifdef FUNC_CODING_KEY_EN
	if(Event == MSG_NONE)
	{
		Event = CodingKeyEventGet();
	}
#endif

#ifdef FUNC_TOUCH_KEY_EN
	if(Event == MSG_NONE)
	{
		Event = TouchKeyEventGet();
	}
#endif
	
#ifdef FUNC_IR_KEY_EN
	if(Event == MSG_NONE)
	{
		Event = IrKeyEventGet();
#ifdef FUNC_BLUETOOTH_CSR_EN
        if(gSys.SystemMode == SYS_MODE_BLUETOOTH)
			Event = IsDoubleClick(Event);

#endif
#ifdef AU6210K_ZB_BT007_IR_IC_IS_334M_CSR
		
		if(!IrIsOn && (Event != MSG_IR_ON_OFF))
		{
#ifdef FUNC_SPI_KEY_SOUND_EN	
			if(Event != MSG_NONE)
			{
				SPI_PlaySelectNum(SPIPLAY_SONG_LOCKING, 1);
			}
#endif

			Event = MSG_NONE;
		}
		if(Event == MSG_IR_ON_OFF)
		{
			if(IrIsOn)
			{
				IrIsOn = FALSE;
				
#ifdef FUNC_SPI_KEY_SOUND_EN
				if(gSys.MuteFg)
					SPI_PlaySelectNum(SPIPLAY_SONG_LOCK_ON, 0); 			
				else
					SPI_PlaySelectNum(SPIPLAY_SONG_LOCK_ON, 1);
#endif
			}
			else
			{
				IrIsOn = TRUE;
				
#ifdef FUNC_SPI_KEY_SOUND_EN
				if(gSys.MuteFg)
					SPI_PlaySelectNum(SPIPLAY_SONG_LOCK_OFF, 0);			
				else
					SPI_PlaySelectNum(SPIPLAY_SONG_LOCK_OFF, 1);
#endif
			}
			baGPIOCtrl[GPIO_A_OUT] &= ~0x40; //A2
			WaitMs(500);
			
		}
		if(Event != MSG_IR_ON_OFF && Event != MSG_NONE && Event != MSG_VOL_ADD && Event != MSG_VOL_SUB)
		{
			if(Event == MSG_MODE_SW)
			{
				SPI_PlaySelectNum(SPIPLAY_SONG_IR_KEY, 0);
				MuteOn(TRUE,FALSE);
			}else
			{
				if(gSys.SystemMode == SYS_MODE_BLUETOOTH)
				{	
					if((Event != MSG_VOL_ADD) && (Event != MSG_VOL_SUB)&&(Event != MSG_BLUETOOTH_REJECT)&& Event != MSG_BLUETOOTH_ANSWER)
						SPI_PlaySelectNum(SPIPLAY_SONG_IR_KEY, 0);
				}
				else
				{
					if(Event == MSG_PLAY_PAUSE)// || Event == MSG_NEXT || Event == MSG_PRE)
						SPI_PlaySelectNum(SPIPLAY_SONG_IR_KEY, 0);
					else
						SPI_PlaySelectNum(SPIPLAY_SONG_IR_KEY, 1);
				}
				if(gSys.SystemMode == SYS_MODE_BLUETOOTH && !GetBtMuteFlag())
				{
					DBG1(("ccccccccddddddddddddd\m"));
					UnMute();
				}
			}
		}
		
		/*if(Event == MSG_V_ADD)
			Event = MSG_VOL_ADD;
		if(Event == MSG_V_SUB)
			Event = MSG_VOL_SUB;*/
		if(Event == MSG_BT_REJECT)
			Event =MSG_BLUETOOTH_REJECT;

#endif			
		
	}
#endif

#ifdef FUNC_GPIO_KEY_EN
	if(Event == MSG_NONE)
	{
		Event = GpioKeyEventGet();
	}
#endif

	return Event;									//cache key value for respective module
}


// Initialize key scan operation.
VOID KeyScanInit(VOID)
{
#if	(PWR_KEY_MODE == PWR_KEY_PUSH_BUTTON)
	PowerKeyScanInit();
#endif

#ifdef FUNC_ADC_KEY_EN
	AdcKeyScanInit();
#endif

#ifdef FUNC_CODING_KEY_EN
	CodingKeyScanInit();
#endif

#ifdef FUNC_TOUCH_KEY_EN
	TouchKeyScanInit();
	TimeOutSet(&TouchKeyInitTimer, 5000);
#endif

#ifdef FUNC_IR_KEY_EN
	IrKeyScanInit();
#endif

#ifdef	FUNC_GPIO_KEY_EN
	GpioKeyScanInit();
#endif

#ifdef	FUNC_NUMBER_KEY_EN
	gRecvNum = 0;
	TimeOutSet(&RecvNumTimer, 0);
#endif
}



#ifdef	FUNC_NUMBER_KEY_EN
VOID RecvIrNum(BYTE num)
{	
	if(gRecvNum < 1000)
	{
		gRecvNum = (gRecvNum * 10) + num;
	}	
	else
	{
		gRecvNum = num;
	}
}
#endif


// Key process
VOID KeyEventGet(VOID)							
{
	MESSAGE	Event = GetKeyEventA();		

#if defined(AU6210K_ZB_BT007_CSR)
	if(Event != MSG_NONE)
	{
		TimeOutSet(&CSR_BTPOWEROFF_TIME, BT_BtPOWEROFF_TIME);
		//BTPowerOffTime_Start =	FALSE;


		TimeOutSet(&SDCARDPOWEROFF_TIME, BT_BtPOWEROFF_TIME);
		//SDCARDPowerOffTime_Start = FALSE;
	}
#endif

#ifdef FUNC_NUMBER_KEY_EN
	if(NumKeyErrorDispFlag == TRUE)
	{		
#ifdef FUNC_DISP_EN
		DispNum(gRecvNum);
#endif		
	}

	if((gRecvNum) && ((IsTimeOut(&RecvNumTimer)) || (Event == MSG_IR_SELECT)))
	{
		TimeOutSet(&RecvNumTimer, SET_INTERVL);		
		Event = MSG_RECV_NUM;	
		NumKeyErrorDispFlag = FALSE;
	}	 
	else if((Event >= MSG_NUM_0) && (Event <= MSG_NUM_10ADD))
	{
		if(((gSys.SystemMode == SYS_MODE_USB) || (gSys.SystemMode == SYS_MODE_SD))
	#ifdef FUNC_RADIO_EN
		|| ((gSys.SystemMode == SYS_MODE_RADIO)&&(gRadioCtrl.State == RADIO_IDLE))
	#endif
		)
		{		
			NumKeyErrorDispFlag = FALSE;			
			NumKeyRadioFreqErrorFlag = FALSE;
			
			if(Event == MSG_NUM_10ADD)
			{
				if(gRecvNum % 10)
				{
			    	gRecvNum = 0;
				}
			    gRecvNum += 10;
			    gIRNUM10FG = 1;
			}
			else
			{
				if(gIRNUM10FG)
				{
					gRecvNum += Event - MSG_NUM_0;
				}
				else
				{
					RecvIrNum(Event - MSG_NUM_0);
				}
			   	gIRNUM10FG = 0;
			}
			Event = MSG_NONE;
			TimeOutSet(&RecvNumTimer, IR_KEY_NUM_WAIT_TIME);
		#ifdef FUNC_DISP_EN
			DispNum(gRecvNum);
		#endif
		}
		else
		{
			Event = MSG_NONE;
		}
	}
	else if((Event >= MSG_NUM_0CP) && (Event <= MSG_NUM_10ADDCP))
	{
		if(((gSys.SystemMode == SYS_MODE_USB) || (gSys.SystemMode == SYS_MODE_SD))
	#ifdef FUNC_RADIO_EN
		|| ((gSys.SystemMode == SYS_MODE_RADIO) && (gRadioCtrl.State == RADIO_IDLE))
	#endif
		)
		{		
			NumKeyErrorDispFlag = FALSE;
			NumKeyRadioFreqErrorFlag = FALSE;
			
			TimeOutSet(&RecvNumTimer, SET_INTERVL);
			if(Event == MSG_NUM_10ADDCP)
			{
				gRecvNum += 10;
			}
			else
			{
				RecvIrNum(Event - MSG_NUM_0CP);
			}
		#ifdef FUNC_DISP_EN
			DispNum(gRecvNum);
		#endif

			if((((gSys.SystemMode == SYS_MODE_USB) || (gSys.SystemMode == SYS_MODE_SD)) 
			&& (gRecvNum <= gFsInfo.FileSum))
		#ifdef FUNC_RADIO_EN
		   	 || ((gSys.SystemMode == SYS_MODE_RADIO) && (gRadioCtrl.State == RADIO_IDLE) && (gRecvNum <= MAX_RADIO_STATION_NUM))
		#endif
			)
			{
				Event = MSG_NUM_SAVE_STAITON;	//FM: 将当前频率存到指定的台号，如果台号>StationSum，就存到(StationSum+1)台。
												//MP3:数字选歌
			}
			else			
			{
				Event = MSG_NONE;
			}
		}
	}
	else
	{
		if(Event != MSG_NONE)
		{
			gRecvNum = 0;				
			NumKeyErrorDispFlag = FALSE;
			NumKeyRadioFreqErrorFlag = FALSE;
		}
	}
#endif

	if(Event != MSG_NONE)
	{
		//DBG(("KeyEvent:%02BX\n", Event));
		MessageSend(MSG_FIFO_KEY, Event);
	  	MessageSend(MSG_FIFO_DEV_CTRL, Event);		
#ifdef	FUNC_RTC_EN
		MessageSend(MSG_FIFO_RTC, Event);	
#endif
	}	

}



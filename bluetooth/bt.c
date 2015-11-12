 #include "syscfg.h"
#include "sysctrl.h"
#include "device.h"
#include "user_interface.h"
#include "breakpoint.h"
#include "debug.h"
#include "sys_on_off.h"
#include "otp_play.h"
#include "pt231x.h"
#include "slave_cmd.h"
#include "slave_ctrl.h"
#include "bluetooth_cmd.h"
#include "uart.h"
#include "bt.h"
#include "GPIO.h"
#include "display.h"
#include "npca110x.h"

#ifdef FUNC_BLUETOOTH_CSR_EN

#define		BT_ACTIVE_AFTERWARE	4000
TIMER	BT_Statup;
TIMER	BT_WaitConnectedTime;

#if defined(FUNC_AUTO_BTSTANDBY)  
TIMER   BT_standby;
#define		BT_STAND_TIME	15000
BYTE standby_flag;
#endif




TIMER   CSR_PRESS_TIME;
TIMER   CSR_BTPOWEROFF_TIME;

//short press：500ms
//Double press：500ms
//Long press:1000ms
//Very long press:2500ms
//Very Very long press:5000ms
//Repeat time:800ms

static BYTE BTjustEnter;
BYTE PlayPairFlag;
BLUETOOTH_STATUS gBlutoothStatus = BT_IDLE;
extern BYTE Connect;
static BOOL BtMuteFlag;
BOOL BTPowerOffTime_Start = FALSE;

BYTE CODE gBTInGain[VOLUME_MAX + 1] = 
{
//注:Line In音频从E0、E1口输入到芯片内部，请用gFmInGain[VOLUME_MAX + 1]表中参数值
//#if ((LINEIN_IO_TYPE == DAC_CH_E0_L) || (LINEIN_IO_TYPE == DAC_CH_E1_R) || (LINEIN_IO_TYPE == DAC_CH_E0E1))
//	63,		61,		59,		57,		55,		53,		51,		49,
//	47,		45,		43,		41,		39,		37,		35,		33,
//	31,		30,		29,		28,		27,		26,		25,		24,
//	23,		22,		21,		20,		19,		18,		17,		16,
//	15
#if (VOLUME_MAX == 16)
	63,		60,		58,		56,		54,		52,		50,		48,
	46,		44,		42,		40,		36,		32,		28,		24,
	20
#else

#ifdef AU6210K_ZB_BT007_CSR
	63,
	62,		58,		54,		51,		48,		45,		42,		39,
	37,		35,		33,		31,		29,		27,		25,		23,
	21,		20,		19,		18,		17,		16,		15,		14,
	13,		12,		11,		10,		9,		8,		7,		6
#else
	63,
	62,		58,		54,		51,		48,		45,		42,		39,
	37,		35,		33,		31,		29,		27,		25,		23,
	21,		19,		17,		16,		15,		14,		13,		12,
	11,		10,		9,		8,		7,		6,		5,		4
#endif	
#endif
};


//set line-in volume.
VOID SetBluetoothVolume(BYTE Vol)
{
	InDacLineInGain(gBTInGain[Vol], gBTInGain[Vol]);
}


VOID BluetoothCtrlInit(VOID)
{
    
	BTjustEnter = 1;
	PlayPairFlag = 1;
	BT_POWER_H();

#if defined(FUNC_AUTO_BTSTANDBY)  
    TimeOutSet(&BT_standby, BT_STAND_TIME);	
#endif	

#if defined(AU6210K_NR_D_8_CSRBT)
#else
	SET_E0_ANALOG_IN();	//将E0E1配置为模拟端口
	SET_E1_ANALOG_IN();
#endif

	InDacChannelSel(BLUETOOTH_CH_TYPE);
#ifdef AU6210K_NR_D_8_CSRBT
	NPCA110X_ADC_Input_CH_Select(INPUT_CH1);
	NPCA110X_DAC1_Set_Volume_and_Mute(gSys.Volume);
#else
	SetBluetoothVolume(gSys.Volume);
#endif
	UnMute();
	if (BTisMute())
	{
		BtMuteFlag = TRUE;
		ExMuteOn();
	}
	else
	{
#ifdef	FUNC_HEADPHONE_DETECT_EN
		if(IsHeadphoneLinkFlag == FALSE)
#endif
			BtMuteFlag = FALSE;
			ExMuteOff();
			DBG1(("r0000r\n"));
	}

	if (BTisMute() && BtMuteFlag)
	{
		BtMuteFlag = TRUE;
#if defined(AU6210K_ZB_BT007_CSR)
		TimeOutSet(&CSR_BTPOWEROFF_TIME, BT_BtPOWEROFF_TIME);
		BTPowerOffTime_Start =  TRUE;
		DBG1(("BTPowerOffTime_Start =  TRUE;\n"));
#endif		
        
	}
	
	DBG(("<<LineInCtrlInit()\n"));
#ifdef FUNC_BREAK_POINT_EN
	BP_SaveInfo(&gBreakPointInfo.PowerMemory.SystemMode, sizeof(gBreakPointInfo.PowerMemory.SystemMode));
#endif

//与CSR连接的IO口初始化
	BTIO_PORT1_INIT();//blue led in MCU
	BTIO_PORT2_INIT();
	BTIO_PORT3_INIT();
	BTIO_PORT4_INIT();//Red led in mcu
	BTIO_PORT5_INIT();
	BTIO_PORT6_INIT();
	BTIO_PORT7_INIT();
	BTIO_PORT_BLUE_LED_INIT();
#ifdef AU6210K_NR_D_8_CSRBT
#else
	BTIO_PORT_RED_LED_INIT();
#endif
	BTIO_PORT_MUTE_INIT();

}


// LineIn IO DeInitialization.
VOID BluetothCtrlEnd(VOID)
{
#ifdef AU6210K_NR_D_8_CSRBT
	NPCA110X_SetOutMute(TRUE);
#else
#endif
	MuteOn(TRUE,TRUE);
    InDacMuteEn();
	InDacChannelSel(DAC_CH_NONE);
	BT_POWER_L();
#if 1//defined(AU6210K_LK_SJ_CSRBT)||defined(AU6210K_ZB_BT007_CSR)
	BTIO_PORT1_END();
	BTIO_PORT2_END();
	BTIO_PORT3_END();
	BTIO_PORT4_END();
	BTIO_PORT5_END();
	BTIO_PORT6_END();
	BTIO_PORT7_END();
#endif

}


BOOL Bluetooth_BlueLED()
{
	//BTIO_PORT_BLUE_LED_INIT();
	//WaitUs(10);
	if(GET_BLUE_LED_STATUS())
		return FALSE;
	else
		return TRUE;
}

BOOL Bluetooth_RedLED()
{
	//BTIO_PORT_RED_LED_INIT();
	//WaitUs(10);
	if(GET_RED_LED_STATUS())
		return FALSE;
	else
		return TRUE;
}

BOOL BTisMute()
{
	//BTIO_PORT_MUTE_INIT();
	//WaitUs(10);
	if(GET_BT_MUTE_STATUS())
		return FALSE;
	else
		return TRUE;

}
BOOL GetBtMuteFlag(void)
{
	return BtMuteFlag;
}
VOID BluetoothStateCtrl(VOID)
{   
    MESSAGE Event;
	BYTE minvolflag = 0;
	BYTE maxvolflag = 0;
#if 0
	if (IsTimeOut(&BT_Statup) && BTjustEnter && gSys.Volume > VOLUME_MIN)
	{
		DBG(("active bluetooth time out.\n"));
		BTjustEnter = 0;		
		//BT_POWER_H();
		//开声音

		//WaitMs(300);//等待通道切完后才解mute
		ExMuteOff();	
	}
#endif	
	/*if (IsTimeOut(&BT_WaitConnectedTime) && PlayPairFlag)
	{
//		TimeOutSet(&BT_WaitConnectedTime,0);
		PlayPairFlag = 0;
		#ifdef FUNC_SPI_KEY_SOUND_EN
		SPI_PlaySelectNum(SPIPLAY_SONG_WAITCONNECTION,0);
		InDacChannelSel(BLUETOOTH_CJ_IO_TYPE);
		UnMute();
		#endif
	}*/
#if defined(AU6210K_ZB_BT007_CSR)||defined(AU6210K_LK_SJ_CSRBT)
//DBG1(("BtMuteFlag = %x\n",BtMuteFlag));

#else
    if (Bluetooth_BlueLED())
	{
		LED_BLUE_H();
	}
	else
		LED_BLUE_L();

/*	if (Bluetooth_RedLED())
	{
		LED_RED_H();
	}
	else
		LED_RED_L();*/
#endif	
#if defined(AU6210K_ZB_BT007_CSR)|| defined(AU6210K_NR_D_8_CSRBT)||defined(AU6210K_LK_SJ_CSRBT)

//#if defiend(AU6210K_NR_D_8_CSRBT) || defined(AU6210K_LK_SJ_CSRBT) || defiend(AU6210K_ZB_BT007_CSR)
	if (BTisMute() && !BtMuteFlag)
	{
		BtMuteFlag = TRUE;
#if defined(AU6210K_ZB_BT007_CSR)
		TimeOutSet(&CSR_BTPOWEROFF_TIME, BT_BtPOWEROFF_TIME);
		BTPowerOffTime_Start =  TRUE;
		DBG1(("BTPowerOffTime_Start =  TRUE;\n"));
#endif		
        
	}
	if ((!BTisMute() && BtMuteFlag) && (gSys.Volume > VOLUME_MIN))
	{
#ifdef	FUNC_HEADPHONE_DETECT_EN
		if(IsHeadphoneLinkFlag == FALSE)
#endif
			BtMuteFlag = FALSE;
			
			DBG1(("r----\n"));
#if defined(AU6210K_ZB_BT007_CSR)			
			TimeOutSet(&CSR_BTPOWEROFF_TIME, 0);
			BTPowerOffTime_Start =	FALSE;
			DBG1(("BTPowerOffTime_Start =  FALSE;\n"));
#endif			
	}
#endif
	if(BtMuteFlag)
		ExMuteOn();
	else
	{
		UnMute();
	}


#if defined(AU6210K_ZB_BT007_CSR)
	if(IsTimeOut(&CSR_BTPOWEROFF_TIME) && BTPowerOffTime_Start)
    {
     	//关机流程
     	DBG1(("power off doing\n"));

		if(BTisMute())
		{
			BT_POWER_L();
			WaitMs(2);
			SPI_PlaySelectNum(SPIPLAY_SONG_POWEROFF, 0);//关机提示音
			WaitMs(1000);
			SystemOff();
		}else
		{
			BTPowerOffTime_Start = FALSE;
			TimeOutSet(&CSR_BTPOWEROFF_TIME, 0);
		}
    }
#endif	

#if defined(FUNC_AUTO_BTSTANDBY)  
	if((Connect == 0) && (IsTimeOut(&BT_standby)))
	{
	    BT_POWER_L();
		standby_flag = 1;
	}
	else
	{
		BT_POWER_H();
		standby_flag = 0;
	}
#endif	

    if(IsTimeOut(&CSR_PRESS_TIME))
    {
	   // ClrGpioRegBit(GPIO_D_OUT, 1<<5);
    }	

	Event = MessageGet(MSG_FIFO_KEY);
    switch(Event)
	{
#ifdef AU6210K_ZB_BT007_CSR
#elif 0
#ifdef AU6210K_ZB_BT007_CSR
	case MSG_PLAY_PAUSE:
		TimeOutSet(&CSR_PRESS_TIME, 500);
        while(!IsTimeOut(&CSR_PRESS_TIME))
        {
            SetGpioRegBit(GPIO_D_OUT, 1<<6);
        }
		ClrGpioRegBit(GPIO_D_OUT, 1<<6);
#ifdef AU6210K_ZB_BT007_IR_IC_IS_334M_CSR		
		SPI_PlaySelectNum(SPIPLAY_SONG_IR_KEY, 0);
		UnMute();
#endif		
		break;
#endif

	case MSG_BT_CHANGE_PHONE:
		TimeOutSet(&CSR_PRESS_TIME, 1000);
        while(!IsTimeOut(&CSR_PRESS_TIME))
        {
            SetGpioRegBit(GPIO_D_OUT, 1<<6);
        }
		ClrGpioRegBit(GPIO_D_OUT, 1<<6);
#ifdef AU6210K_ZB_BT007_IR_IC_IS_334M_CSR
		SPI_PlaySelectNum(SPIPLAY_SONG_IR_KEY, 0);
		UnMute();
#endif		
		break;

#ifdef AU6210K_ZB_BT007_CSR
	case MSG_BLUETOOTH_ANSWER:
#else
	case MSG_PLAY_PAUSE:	
#endif		
		{
#if defined(FUNC_AUTO_BTSTANDBY)  
			if(standby_flag == 1)
			{
			    BT_POWER_H();
				standby_flag = 0;
				Connect = 0;
				TimeOutSet(&BT_standby, BT_STAND_TIME);
			}
			else
#endif          
            TimeOutSet(&CSR_PRESS_TIME, 500);
            while(!IsTimeOut(&CSR_PRESS_TIME))
            {
            	SetGpioRegBit(GPIO_D_OUT, 1<<5);
            }
			ClrGpioRegBit(GPIO_D_OUT, 1<<5);
#if 0//def AU6210K_ZB_BT007_IR_IC_IS_334M_CSR
			SPI_PlaySelectNum(SPIPLAY_SONG_IR_KEY, 0);
			UnMute();
#endif
		
#ifdef FUNC_BEEP_SOUND_EN	
			{
				PushKeyBeep(1);
				UnMute();
				
			}
#endif
		}	
		break;
			
	case MSG_NEXT:	
		{   
			TimeOutSet(&CSR_PRESS_TIME, 500);
            while(!IsTimeOut(&CSR_PRESS_TIME))
            {
                SetGpioRegBit(GPIO_A_OUT, 1<<2);
            }
			ClrGpioRegBit(GPIO_A_OUT, 1<<2);
			DBG(("MSG_NEXT\n"));
#ifdef AU6210K_ZB_BT007_IR_IC_IS_334M_CSR
			SPI_PlaySelectNum(SPIPLAY_SONG_IR_KEY, 0);
			UnMute();
#endif
		}	
		break;
	case MSG_PRE:	
		{
			TimeOutSet(&CSR_PRESS_TIME, 500);
            while(!IsTimeOut(&CSR_PRESS_TIME))
            {
                SetGpioRegBit(GPIO_A_OUT, 1<<1);
            }
			ClrGpioRegBit(GPIO_A_OUT, 1<<1);
			DBG(("MSG_PRE\n"));
#ifdef AU6210K_ZB_BT007_IR_IC_IS_334M_CSR
			SPI_PlaySelectNum(SPIPLAY_SONG_IR_KEY, 0);
			UnMute();
#endif
			
		}	
		break;
	
	case MSG_BLUETOOTH_REJECT:
		{   
			TimeOutSet(&CSR_PRESS_TIME, 1000);
            while(!IsTimeOut(&CSR_PRESS_TIME))
            {
                SetGpioRegBit(GPIO_D_OUT, 1<<5);
            }
			ClrGpioRegBit(GPIO_D_OUT, 1<<5);
			
#ifdef AU6210K_ZB_BT007_IR_IC_IS_334M_CSR
			SPI_PlaySelectNum(SPIPLAY_SONG_IR_KEY, 0);
			UnMute();
#endif
		}
		break;

	case MSG_BT_RECALL:
		{   
			TimeOutSet(&CSR_PRESS_TIME, 500);
            while(!IsTimeOut(&CSR_PRESS_TIME))
            {
                SetGpioRegBit(GPIO_D_OUT, 1<<5);
            }
			TimeOutSet(&CSR_PRESS_TIME, 300);
            while(!IsTimeOut(&CSR_PRESS_TIME))
            {
                ClrGpioRegBit(GPIO_D_OUT, 1<<5);
            }
			TimeOutSet(&CSR_PRESS_TIME, 500);
		    while(!IsTimeOut(&CSR_PRESS_TIME))
            {
                SetGpioRegBit(GPIO_D_OUT, 1<<5);
            }
			    ClrGpioRegBit(GPIO_D_OUT, 1<<5);
#ifdef AU6210K_ZB_BT007_IR_IC_IS_334M_CSR
			SPI_PlaySelectNum(SPIPLAY_SONG_IR_KEY, 0);
			UnMute();
#endif
			
		}
		break;
	case MSG_BT_DISCONNECT:
		{   
#ifdef AU6210K_ZB_BT007_IR_IC_IS_334M_CSR
			SPI_PlaySelectNum(SPIPLAY_SONG_IR_KEY, 0);
			UnMute();
#endif
			TimeOutSet(&CSR_PRESS_TIME, 5000);
            SetGpioRegBit(GPIO_D_OUT, 1<<5); 	
			
		}
		break;

#endif
	case MSG_V_ADD:
	case MSG_VOL_ADD:			
#if defined(FUNC_PT231X_EN) && !defined(AU6210K_NR_D_8_CSRBT)
		VolType = VOL_MAIN;
		PT2313VolAdd(); 	
#else
		if(gSys.Volume < VOLUME_MAX)
		{
			maxvolflag = 1;
			VolumeAdjust(UP);
		}
#endif    
#if defined(AU6210K_NR_D_9X_XJ_HTS)|| defined(AU6210K_NR_D_8_CSRBT)||defined(AU6210K_LK_SJ_CSRBT)|| defined(AU6210K_ZB_BT007_CSR)
		if (Event == MSG_V_ADD && gSys.Volume == VOLUME_MAX && !maxvolflag)
		{
			SPI_PlaySelectNum(SPIPLAY_SONG_MAX_VOLUME, 1);		
			if( !GetBtMuteFlag())
			{
			DBG1(("7777\m"));
				UnMute();
			}
			
		}
		
#endif
		break;
	case MSG_V_SUB:		
	case MSG_VOL_SUB:			
#if defined(FUNC_PT231X_EN) && !defined(AU6210K_NR_D_8_CSRBT)
		VolType = VOL_MAIN;
		PT2313VolSub(); 	
#else
		if(gSys.Volume > VOLUME_MIN)
		{
			minvolflag = 1;
			VolumeAdjust(DOWN);
		}
#endif
#if defined(AU6210K_NR_D_9X_XJ_HTS) || defined(AU6210K_NR_D_8_CSRBT)||defined(AU6210K_LK_SJ_CSRBT)|| defined(AU6210K_ZB_BT007_CSR)
		if (Event == MSG_V_SUB && gSys.Volume == VOLUME_MIN && !minvolflag)
		{
			SPI_PlaySelectNum(SPIPLAY_SONG_MIN_VOLUME, 1);
			if( !GetBtMuteFlag())
			{
			DBG1(("5555\m"));
				UnMute();
			}
			//UnMute();
		}
#endif
		break;		
		
	default:
		break;
	}
}
#endif


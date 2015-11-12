#include "syscfg.h"
#include "device.h"
#include "device_hcd.h"
#include "radio_app_interface.h"
#include "radio_ctrl.h"
#include "debug.h"
#include "seg_panel.h"
#include "seg_led_disp.h"
#include "bluetooth_ctrl.h"



BOOL IsCardLinkFlag = FALSE;
BOOL IsUsbLinkFlag = FALSE;
BOOL IsLineInLinkFlag = FALSE;
BOOL IsPcLinkFlag = FALSE;
#ifdef OPTION_CHARGER_DETECT
BOOL IsChargerLinkFlag = FALSE;
#endif

#ifdef	FUNC_HEADPHONE_DETECT_EN
BOOL IsHeadphoneLinkFlag = TRUE;
#endif

#ifdef FUNC_LED_LINEIN_MULTIPLE_EN
TIMER gLineInCheckWaitTimer;
#endif

#ifdef FUNC_SEG_LED_EN
extern BOOL gLedDispRefreshFlag;
#endif


/////////////////////////////////////////////////////////////////////////////////////
#ifdef FUNC_USB_EN
//检查U盘连接状态
BOOL IsUSBLink(VOID)
{
	return HostIsLink();
}


//检查是否有新插入U盘
BOOL IsUsbInsert(VOID)
{	
	BOOL Temp = IsUsbLinkFlag;
	IsUsbLinkFlag = IsUSBLink();
	return (!Temp && IsUsbLinkFlag);
}
#endif


#if (defined(FUNC_CARD_EN) || defined(FUNC_AUDIO_EN) || defined(FUNC_READER_EN) || defined(FUNC_AUDIO_READER_EN))

#define 	CARD_DETECT_TIME	300

extern VOID CardSdClkStop(VOID);
extern BOOL gCardPrevLinkState;

TIMER	CardDctTimer;


//检查SD卡连接状态
BOOL IsCardLink(VOID)
{
#define CARD_JETTER_TIME 10//防抖时间

	static BOOL CardLinkFlag = FALSE;

#ifdef	SD_DETECT_HW_SW
	static BOOL IsWaitCardPwrup = TRUE;

	CardSdClkStop();	//停止SD的CLK的输出，减少对系统的干扰	

#if (defined(SD_DETECT_PIN_USE_A4) || defined(SD_DETECT_PIN_USE_D6) || defined(SD_DETECT_PIN_USE_E3))
	SET_CARD_NOT_TO_GPIO();		//将SD_CLK变成普通GPIO
#endif

	ClrGpioRegBit(CARD_DETECT_PORT_DS, CARD_DETECT_BIT);		//使用弱上拉可以提高SD卡兼容性
	ClrGpioRegBit(CARD_DETECT_PORT_PU, CARD_DETECT_BIT);
	ClrGpioRegBit(CARD_DETECT_PORT_PD, CARD_DETECT_BIT);	
	ClrGpioRegBit(CARD_DETECT_PORT_OE, CARD_DETECT_BIT);
	SetGpioRegBit(CARD_DETECT_PORT_IE, CARD_DETECT_BIT);
	//GPIO口做硬检测时，为信号建立等待一段时间
	//电阻使用2.2K时,等待10us；使用10K时, 需要等20us才能稳定检测
	WaitUs(20);

	if(GetGpioReg(CARD_DETECT_PORT_IN) & CARD_DETECT_BIT)
	{
		//DBG(("NO CARD HW IN\n"));
		IsWaitCardPwrup = TRUE;
		CardLinkFlag = FALSE;
		return FALSE;
	}
	else
	{
		if(IsWaitCardPwrup == TRUE)
		{
			IsWaitCardPwrup = FALSE;
			gCardPrevLinkState = TRUE;
			CardOpen();
			TimeOutSet(&CardDctTimer, CARD_DETECT_TIME);	//用于卡的插入消抖，当发现卡插入，到执行软检测延时CARD_DETECT_TIME时间
		}
	}

#ifdef  SD_DETECT_PIN_USE_A4
	SET_CARD_TO_A3A4A5();	//将A4恢复设置为SD总线
#endif
	
#ifdef	SD_DETECT_PIN_USE_D6
	SET_CARD_TO_D5D6D7();	//将D6恢复设置为SD总线
#endif
	
#ifdef	SD_DETECT_PIN_USE_E3
	SET_CARD_TO_A0E2E3();	//将E3恢复设置为SD总线
#endif
	
	//本次硬件检测到卡插入后，如果前一次卡的连接状态为FALSE时，会按timeout时间去执行一次软件检测。
	//或者在卡的通信过程中出现错误，也会执行一次软件检测。

	if(((CardLinkFlag == FALSE) && IsTimeOut(&CardDctTimer)) || (gCardPrevLinkState == FALSE))
#else //!SD_DETECT_HW_SW
	if(IsTimeOut(&CardDctTimer) || (gCardPrevLinkState == FALSE))  
#endif
	{
	 	//DBG(("CARD SW\n"));	 	
		TimeOutSet(&CardDctTimer, CARD_DETECT_TIME);
		gCardPrevLinkState = TRUE;
		//执行本次软检测
		CardLinkFlag = CardDetect();
		//停止SD的CLK的输出,减少对系统干扰
		CardSdClkStop();

		WaitMs(20);
		//DBG1(("CardLinkFlag: %d\n", (WORD)CardLinkFlag));
	}

	return CardLinkFlag;
}


//检查是否有新插入的SD卡
BOOL IsCardInsert(VOID)
{
	BOOL Temp = IsCardLinkFlag;

	IsCardLinkFlag = IsCardLink();
//	DBG(("CardIn:%d, %d\n", (WORD)Temp, (WORD)IsCardLinkFlag));
	return (!Temp && IsCardLinkFlag);
}
#endif


#ifdef FUNC_LINEIN_EN
//检查line-in连接状态
//使能内部上拉。无line-in时，检测口为高电平，有line-in时，检测口为低电平
BOOL IsLineInLink(VOID)
{
#define LINEIN_JETTER_TIME 20//防抖时间
	static BYTE LowLevelTimes = 0;
#ifdef LINEIN_DETECT_BIT
#ifdef FUNC_LED_LINEIN_MULTIPLE_EN
	if(!IsTimeOut(&gLineInCheckWaitTimer))	
	{
		return IsLineInLinkFlag;
	}
	TimeOutSet(&gLineInCheckWaitTimer, 150);
		
#if (defined(FUNC_7PIN_SEG_LED_EN) || defined(FUNC_6PIN_SEG_LED_EN))
	gLedDispRefreshFlag = FALSE;   
	LedPinGpioInit();
	baGPIOCtrl[LINEIN_DETECT_PORT_IE] |= LINEIN_DETECT_BIT;
	baGPIOCtrl[LINEIN_DETECT_PORT_PU] &= ~LINEIN_DETECT_BIT;
#endif
#else
	baGPIOCtrl[LINEIN_DETECT_PORT_IE] |= LINEIN_DETECT_BIT;
	baGPIOCtrl[LINEIN_DETECT_PORT_PU] &= ~LINEIN_DETECT_BIT;
	baGPIOCtrl[LINEIN_DETECT_PORT_PD] &= ~LINEIN_DETECT_BIT;	
	baGPIOCtrl[LINEIN_DETECT_PORT_OE] &= ~LINEIN_DETECT_BIT;
#endif
	WaitUs(10);

	if(baGPIOCtrl[LINEIN_DETECT_PORT_IN] & LINEIN_DETECT_BIT)
	{
		IsLineInLinkFlag = FALSE;
		LowLevelTimes = 0;
	}
	else
	{
		if(LowLevelTimes < LINEIN_JETTER_TIME)
		{
			LowLevelTimes++;
		}
		else
		{
			IsLineInLinkFlag = TRUE;
		}
	}
	
#ifdef FUNC_LED_LINEIN_MULTIPLE_EN
	gLedDispRefreshFlag = TRUE;
#endif

#else
	IsLineInLinkFlag = TRUE;
#endif
	return IsLineInLinkFlag;
}


//检查是否有新插入line-in
BOOL IsLineInInsert(VOID)
{	
	BOOL Temp = IsLineInLinkFlag;

	IsLineInLinkFlag = IsLineInLink();
	return (!Temp && IsLineInLinkFlag);
}
#endif


#ifdef FUNC_RADIO_EN
//检查是否有插入Radio 模块
BOOL IsRadioLink(VOID)			
{
	if(Radio_Name == RADIO_NONE)
	{
		RadioInit();
	}
	return (Radio_Name != RADIO_NONE);
}
#endif


#if (defined(FUNC_AUDIO_EN) || defined(FUNC_READER_EN) || defined(FUNC_AUDIO_READER_EN))
//检查PC连接状态
BOOL IsPcLink(VOID)			
{
	return UsbDeviceIsLink();
}


//检查是否有新插入PC
BOOL IsPcInsert(VOID)
{	
	BOOL Temp = IsPcLinkFlag;

	IsPcLinkFlag = IsPcLink();
	return (!Temp && IsPcLinkFlag);
}
#endif


#ifdef OPTION_CHARGER_DETECT
//硬件检测PC 或充电器连接状态
//使能内部下拉。无PC或充电器连接时，检测口为低电平，有时检测口为高电平
BOOL IsInCharge(VOID)
{
	baGPIOCtrl[CHARGE_DETECT_PORT_PU] |= CHARGE_DETECT_BIT;
	baGPIOCtrl[CHARGE_DETECT_PORT_PD] |= CHARGE_DETECT_BIT;
	baGPIOCtrl[CHARGE_DETECT_PORT_OE] &= ~CHARGE_DETECT_BIT;
	baGPIOCtrl[CHARGE_DETECT_PORT_IE] |= CHARGE_DETECT_BIT;
	WaitUs(2);
	
	if(baGPIOCtrl[CHARGE_DETECT_PORT_IN] & CHARGE_DETECT_BIT)
	{
		return TRUE;
	}   	
#if 0//def AU6210K_NR_D_8_CSRBT
	baGPIOCtrl[GPIO_E_PU] |= (1 << 1);
	baGPIOCtrl[GPIO_E_PD] |= (1 << 1);
	baGPIOCtrl[GPIO_E_OE] &= ~(1 << 1);
	baGPIOCtrl[GPIO_E_IE] |= (1 << 1);
	WaitUs(2);
#endif
	return FALSE;
}


//检查是否有新插入PC或充电器
BOOL IsChargerInsert(VOID)
{	
	BOOL Temp = IsChargerLinkFlag;

	IsChargerLinkFlag = IsInCharge();
	return ((!Temp) && IsChargerLinkFlag);
}
#endif


#ifdef	FUNC_HEADPHONE_DETECT_EN
//检查HeadPhone连接状态
//使能内部上拉。无HeadPhone时，检测口为高电平，有HeadPhone时，检测口为低电平
BOOL IsHeadPhoneLink(VOID)
{	
#ifdef HEADPHONE_DETECT_BIT
	baGPIOCtrl[HEADPHONE_DETECT_PORT_PU] &= ~HEADPHONE_DETECT_BIT;
	baGPIOCtrl[HEADPHONE_DETECT_PORT_PD] &= ~HEADPHONE_DETECT_BIT;	
	baGPIOCtrl[HEADPHONE_DETECT_PORT_OE] &= ~HEADPHONE_DETECT_BIT;
	baGPIOCtrl[HEADPHONE_DETECT_PORT_IE] |= HEADPHONE_DETECT_BIT;
	WaitUs(2);
	if(baGPIOCtrl[HEADPHONE_DETECT_PORT_IN] & HEADPHONE_DETECT_BIT)
	{
		IsHeadphoneLinkFlag = FALSE;
	}
	else
	{
		IsHeadphoneLinkFlag = TRUE;
	}	
	
#ifdef FUNC_HEADPHONE_SINGLE_LED_EN	//耳机检测端口与单个LED端口共用
	baGPIOCtrl[HEADPHONE_DETECT_PORT_IE] &= ~HEADPHONE_DETECT_BIT;
	baGPIOCtrl[HEADPHONE_DETECT_PORT_OE] |= HEADPHONE_DETECT_BIT;
#endif
#endif

	return IsHeadphoneLinkFlag;
}


//检查是否有新插入HeadPhone
BOOL IsHeadPhonInsert(VOID)
{	
	BOOL Temp = IsHeadphoneLinkFlag;
	
	IsHeadphoneLinkFlag = IsHeadPhoneLink();
	return ((!Temp) && IsHeadphoneLinkFlag);
}

#endif


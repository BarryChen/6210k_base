#include <reg51.h>										//include public header files

#include "type.h"										//include project header files
#include "utility.h"
#include "power_key.h"
#include "key.h"
#include "syscfg.h"
#include "sysctrl.h"
#include "timer.h"
#include "debug.h"
#include "ir_key.h"
#include "rtc.h"
#include "gpio.h"
#include "int.h"
#include "interrupt.h"
#include "display.h"
#include "coding_key.h"
#include "touchkey_ctrl.h"
#include "slave_ctrl.h"
#include "slave_cmd.h"

#ifdef FUNC_BEEP_SOUND_EN		
volatile WORD PushKeyBeepTimerCount = 0;
extern BOOL gPushKeyBeepStartFlag;
#endif

#ifdef FUNC_TIMER1_REFRESH_LED_EN
BYTE gClkDivRate = 1;
#define TIMER1USTIMER	500	//定义Timer1的定时微秒单位时间
#endif

extern VOID OnDataReceived(VOID);
extern VOID OnDataSend(VOID);
extern VOID OnOtgRcvPacket(VOID);


// EXTERNAL INT 0 interrupt service function.
VOID ExternalInt0(VOID)		interrupt 0	
{

}


// TIMER/COUNTER 0 interrupt service function.
// Note: this ISR is used as system tick.
VOID Timer0(VOID)			interrupt 1			
{  
	TimerTick();

#ifdef FUNC_BEEP_SOUND_EN		
	if(gPushKeyBeepStartFlag)
	{
		PushKeyBeepTimerCount++;
		if(PushKeyBeepTimerCount >= 1)
		{
			PushKeyBeepTimerCount = 0;
			if(baGPIOCtrl[GPIO_E_OUT] & 0x01)
			{
				baGPIOCtrl[GPIO_E_OUT] &= ~0x01;
			}
			else
			{
				baGPIOCtrl[GPIO_E_OUT] |= 0x01;
			}
		}
	}
#endif
}


// EXTERNAL INT 1 interrupt service function.
// Note: extend-timer(1s) interrupt source for system tick;
//	GPIO_C port interrupt source, for IR soft-decode;
VOID ExternalInt1(VOID)		interrupt 2
{
	WORD intType;
	BYTE intEnMap;

	intType = GetIntFlag();								//check extended interrupt source flag
	intEnMap = intType & GetIntEn();

//	DBG(("---%-.2BX\n", intEnMap));
#if (defined(FUNC_AUDIO_EN) || defined(FUNC_AUDIO_READER_EN))
	if(intEnMap & MASK_INT_OTG_MC)
	{
		OnOtgRcvPacket();
	}
#endif

	if(intEnMap & MASK_INT_GPIO)	
	{		
#ifdef FUNC_CODING_KEY_EN	
		if(GetGpioReg(CODING_KEY_A_PORT_INTS) & CODING_KEY_A_BIT)	//coding key interrupt
		{	
			CodingKeyInterrupt();	
		}
		SetGpioRegBit(CODING_KEY_A_PORT_INTC, CODING_KEY_A_BIT);
#endif	
	}
}


// TIMER/COUNTER 1 interrupt service function.
VOID Timer1(VOID)			interrupt 3			
{
	//for user code	
#ifdef FUNC_TIMER1_REFRESH_LED_EN
#ifdef FUNC_SEG_LED_EN 
	LedFlushDisp();
	TR1 = 0;
	TH1 = (0xFFFF - (TIMER1USTIMER / gClkDivRate)) >> 8;					   
	TL1 = (0xFFFF - (TIMER1USTIMER / gClkDivRate)) & 0xFF; 
	TR1 = 1;
#endif
#endif

#ifdef FUNC_TOUCH_KEY_EN
	static BYTE Count = 0;

	Count++;
	if(Count >= 3)
	{
		Count = 0;
		TR1 = 0;
		TouchKeyDoutPro();		
		TR1 = 1;		
	}
#endif
}


// Serial Port interrupt service function.
VOID Serial(VOID)			interrupt 4	
{
	//for user code
#if 0	//csr蓝牙案子如果用到串口通讯则在此处开宏
	if(RI == 1)
	{
	       baGPIOCtrl[GPIO_D_OE]|=0x01;
		baGPIOCtrl[GPIO_D_OUT]&=(~0x01);   
		RI=0;
		#if defined(FUNC_BLUETOOTH_CSR_EN) && !defined(AU6210K_NR_D_8_CSRBT)
		OnDataReceived();
		//TESTSlaveSendRespSecond();
		#endif
		baGPIOCtrl[GPIO_D_OUT]|=0x01;
	}
#endif	
}


// Interrupts control initialize operation.
VOID ExInt1Init(VOID)
{									
	IT1 = 0;											//voltage level trigger
	EX1 = 1;                                            //enable EX-INT1
	EA = 1;
}


#if (defined(FUNC_TIMER1_REFRESH_LED_EN) || defined(FUNC_TOUCH_KEY_EN))
VOID Timer1Init(VOID)
{			
#ifdef FUNC_TIMER1_REFRESH_LED_EN
	TMOD |= 0x10;                      //T1 方式1   
	TH1 = 0xFC;                       
	TL1 = 0x17;                      
	TR1 = 1;                          //启用定时器T1   
	ET1 = 1;                          //开定时器T1中断 	
#endif

#ifdef FUNC_TOUCH_KEY_EN
	//配置为8位自动重装载模式
	ET1 = 0;	
	TR1 = 0;
	TMOD |= 0x20;
	TH1 = 0X06;
	TL1 = 0X06; 
	ET1 = 1;
	TR1 = 0;
#endif
}
#endif



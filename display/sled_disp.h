#ifndef __SLED_DISP_H__
#define __SLED_DISP_H__

#if defined(AU6210K_ZB_BT007_CSR)||defined(AU6210K_LK_SJ_CSRBT)
#define SLED_MP3_EN
#endif
//#define SLED_USB_EN
//#define SLED_SD_EN
//#define SLED_AUX_EN
//#define SLED_REP_EN
//#define SLED_RAD_EN
//#define SLED_AUD_EN

#define LIGHTOFF	0
#define LIGHTON		1

#define SLOW_BLINK_INTERVAL		1000
#define NORMAL_BLINK_INTERVAL	500
#define QUICK_BLINK_INTERVAL	50//100 
#define REPEAT_BLINK_INTERVAL	300 

#if defined(AU6210K_HXX_B002) || defined(AU6210K_NR_D_9X)||defined(AU6210K_NR_D_9X_XJ_HTS)
#define SLED_PORT_IE		GPIO_A_IE
#define SLED_PORT_OE		GPIO_A_OE
#define SLED_PORT_OUT		GPIO_A_OUT
#define SLED_PORT_PU		GPIO_A_PU
#define SLED_PORT_PD		GPIO_A_PD

#define SLED_POWER_PORT		GPIO_A_OUT
#define MASK_SLED_POWER		(1 << 0)	// A[2]
#elif defined(AU6210K_JLH_JH82B)
#define SLED_PORT_IE		GPIO_D_IE
#define SLED_PORT_OE		GPIO_D_OE
#define SLED_PORT_OUT		GPIO_D_OUT
#define SLED_PORT_PU		GPIO_D_PU
#define SLED_PORT_PD		GPIO_D_PD

#define SLED_POWER_PORT		GPIO_D_OUT
#define MASK_SLED_POWER		(1 << 5)	// D[2]

#elif defined(AU6210K_NR_D_8_CSRBT)
#define SLED_PORT_IE		GPIO_A_IE
#define SLED_PORT_OE		GPIO_A_OE
#define SLED_PORT_OUT		GPIO_A_OUT
#define SLED_PORT_PU		GPIO_A_PU
#define SLED_PORT_PD		GPIO_A_PD

#define SLED_POWER_PORT		GPIO_A_OUT
#define MASK_SLED_POWER		(1 << 0)	// a[0]


#elif defined(AU6210K_LK_SJ_CSRBT) || defined(AU6210K_ZB_BT007_CSR)
#define SLED_PORT_IE		GPIO_A_IE
#define SLED_PORT_OE		GPIO_A_OE
#define SLED_PORT_OUT		GPIO_A_OUT
#define SLED_PORT_PU		GPIO_A_PU
#define SLED_PORT_PD		GPIO_A_PD

#define SLED_POWER_PORT		GPIO_A_OUT
#define MASK_SLED_POWER		(1 << 0)	// a[0]  mp3 led blue




#define SLED_MP3_IE		GPIO_B_IE
#define SLED_MP3_OE		GPIO_B_OE
#define SLED_MP3_OUT	GPIO_B_OUT
#define SLED_MP3_PU		GPIO_B_PU
#define SLED_MP3_PD		GPIO_B_PD

#define SLED_MP3_PORT		GPIO_B_OUT
#define MASK_SLED_MP3		(1 << 5)	// a[0]



#else
#define SLED_PORT_IE		GPIO_D_IE
#define SLED_PORT_OE		GPIO_D_OE
#define SLED_PORT_OUT		GPIO_D_OUT
#define SLED_PORT_PU		GPIO_D_PU
#define SLED_PORT_PD		GPIO_D_PD

#define SLED_POWER_PORT		GPIO_D_OUT
#define MASK_SLED_POWER		(1 << 2)	// D[2]
#endif

//#define SLED_MP3_PORT	   	GPIO_B_OUT
//#define SLED_USB_PORT	   	GPIO_B_OUT
//#define SLED_SD_PORT	   	GPIO_B_OUT
//#define SLED_AUX_PORT	   	GPIO_B_OUT
//#define SLED_RADIO_PORT	   	GPIO_B_OUT
//#define SLED_REPEAT_PORT   	GPIO_B_OUT
//#define SLED_AUDIO_PORT	   	GPIO_B_OUT
//
//#define MASK_SLED_MP3	   	0x02
//#define MASK_SLED_USB	   	0x04
//#define MASK_SLED_SD	   	0x08
//#define MASK_SLED_AUX	  	0x10
//#define MASK_SLED_RADIO	 	0x20
//#define MASK_SLED_REPEAT	0x40
//#define MASK_SLED_AUDIO		0x80


typedef enum _SLED_IDX
{
	LED_POWER,
	LED_MP3,
	LED_USB,
	LED_SD,
	LED_AUX,
	LED_RADIO,
	LED_AUDIO,
	LED_REPEAT

} SLED_IDX;

VOID SLedLightOp(SLED_IDX Led, BOOL IsLightOn);
VOID SLedInit(VOID);
VOID SLedDeinit(VOID);
VOID SLedDispDevSymbol(VOID);
VOID SLedDispPlayState(VOID);
VOID SLedDispRepeat(VOID);
VOID SLedDispRadio(VOID);
VOID SLedDispAudio(VOID);
VOID SLedDispRTC(VOID);
VOID SLedDispMute(VOID);
VOID SLedDisplay(VOID);


#endif

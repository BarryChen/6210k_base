#ifndef __DEVICE_H__
#define __DEVICE_H__

#include "syscfg.h"

BOOL InitDev(VOID);						

BOOL IsUSBLink(VOID); 		//检查USB连接状态
BOOL IsCardLink(VOID); 		//检查SD卡连接状态
BOOL IsLineInLink(VOID);	//检查line-in连接状态
BOOL IsRadioLink(VOID);		//检查RADIO连接状态
BOOL IsCardInsert(VOID);  	//检查是否有新插入的SD卡
BOOL IsUsbInsert(VOID);		//检查是否有新插入U盘
BOOL IsLineInInsert(VOID);	//检查是否有新插入line-in
BOOL IsIpodLink(VOID);
BOOL IsPcLink(VOID); 		//检查PC连接状态
BOOL IsPcInsert(VOID); 		//检查是否有新插入PC

#ifdef OPTION_CHARGER_DETECT
extern BOOL IsChargerLinkFlag;
BOOL IsInCharge(VOID);		//硬件检查PC或充电器连接状态
BOOL IsChargerInsert(VOID);	//硬件检查是否有新插入PC或充电器
#endif

#ifdef	FUNC_HEADPHONE_DETECT_EN
extern BOOL IsHeadphoneLinkFlag;
BOOL IsHeadPhoneLink(VOID);		//硬件检查耳机连接状态
BOOL IsHeadPhonInsert(VOID);		 //硬件检查是否有新插入耳机
#endif

extern BOOL IsCardLinkFlag;
extern BOOL IsUsbLinkFlag;
extern BOOL IsLineInLinkFlag;
extern BOOL IsPcLinkFlag;

#ifdef FUNC_LED_LINEIN_MULTIPLE_EN
extern TIMER gLineInCheckWaitTimer;
#endif

#endif

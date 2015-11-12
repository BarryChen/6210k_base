/*
*****************************************************************************
*					Mountain View Silicon Tech. Inc.
*	Copyright 2010, Mountain View Silicon Tech. Inc., ShangHai, China
*					All rights reserved.
*
* Filename:			coding_key.h
* Description:		coding_key operation h file
*
* Change History:
*			Orson	- 6/xx/2010 - V0.1
*					- created
******************************************************************************
*/
#if !defined(__CODING_KEY_H__)
#define __CODING_KEY_H__
#include "key.h"
#include "message.h"



#ifdef FUNC_CODING_KEY_EN
/*
**********************************************************
*					GLOBAL MACRO
**********************************************************
*/



/*
**********************************************************
*					GLOBAL DATA TYPE
**********************************************************
*/



/*
**********************************************************
*					GLOBAL VARIABLE
**********************************************************
*/



/*
**********************************************************
*					GLOBAL FUNCTION PROTOTYPE
**********************************************************
*/
// Initialize coding key scan (GPIO) operation.
VOID CodingKeyScanInit();

// Key process, image key value to key event.
MESSAGE CodingKeyEventGet();

// Coding key signal-A interrupt.
VOID CodingKeyInterrupt();

#endif 
#endif 

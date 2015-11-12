#ifndef __KEY_H__
#define __KEY_H__
#include "message.h"

#define	KEY_EVENT_BASE_ADDR		0xA0


// Initialize key scan operation.
VOID KeyScanInit(VOID);

// Key process.
VOID KeyEventGet(VOID);							


#endif 

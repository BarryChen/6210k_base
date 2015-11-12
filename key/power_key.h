#ifndef __POWER_KEY_H__
#define __POWER_KEY_H__

#include "message.h"


// Initialize POWER_KEY scan operation.
VOID PowerKeyScanInit(VOID);

// POWER_KEY与普通的按键不同，连接按钮开关（软开关）时的主要作用还是系统开关机，当然，也允许复用短按功能。
// 短按产生时，推送短按消息。超过短按区间，推送系统开关机消息。
MESSAGE PowerKeyEventGet(VOID);


#endif

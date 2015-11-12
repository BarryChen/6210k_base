#ifndef __ADC_KEY_H__
#define __ADC_KEY_H__
#include "message.h"


#ifdef FUNC_ADC_KEY_EN

// Initialize adc key scan (GPIO) operation.
VOID AdcKeyScanInit(VOID);


// Key process, image key value to key event.
MESSAGE AdcKeyEventGet(VOID);


#endif
#endif 

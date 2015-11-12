#ifndef __IR_KEY_H__
#define __IR_KEY_H__


// IR decoder initilize.
VOID IrKeyScanInit(VOID);

// Check IR key value from IR controller.
MESSAGE IrKeyEventGet(VOID);

#endif 

#ifndef __INT_H__
#define __INT_H__


// bit mask of INT_EN_ADR_REG, INT_MASK_ADR_REG, INT_FLAG_REG
// used by the functions:

#define MASK_INT_GPIO						(1 << 0)
#define MASK_INT_OTG_MC						(1 << 1)


// Enable the corresponding extended interrupts' source.
VOID SetIntEnBit(BYTE Mask);

// Disable the corresponding extended interrupts' source.
VOID ClrIntEnBit(BYTE Mask);

// Get the value of interrupt enable register.
BYTE GetIntEn(VOID);

// Clear extended interrupts' flag bit.
// Note: call it after response the interrupt.
VOID ClrIntFlagBit(BYTE Mask);

// Get the triggered extended interrupts' flag.
WORD GetIntFlag(VOID);				

// Mask extended interrupts' trigger, used to mask unexpectable interrupt signal.
VOID SetIntMask(BYTE Mask);


#endif

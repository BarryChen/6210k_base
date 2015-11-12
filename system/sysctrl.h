#ifndef __SYSCTRL_H__
#define __SYSCTRL_H__

// Define system control structure.
// Note: inlcude play control structure, file system control structure, device control structure.
typedef struct _SYSTEM
{
	BOOL IsClkDiv; 
	BYTE SystemMode;
	BYTE Volume;
	BOOL MuteFg;
	BOOL ModeChangeFlag;	

} SYSTEM;


extern SYSTEM gSys;

#endif

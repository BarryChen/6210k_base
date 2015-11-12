#ifndef __UTILITY_H__
#define __UTILITY_H__

#define		FOOD_FOR_DOG	0x7F						//set [25:19] bits of watchdog counter
														//watchdog over time about 5s

// MCU Wait Ms.
VOID WaitMs(WORD Ms);	

// MCU Wait Us.	   //名称要改，不是Us
VOID WaitUs(WORD Us);	

// Feed WatchDog, the "FOOD_FOR_DOG" please refer to "syscfg.h".
VOID FeedWatchDog(VOID);					

// Big endian to Little endian 16bit, Change 16bit data
// between big endian and little endian.
WORD LE16ToCPU(WORD Val);

// Big endian to Little endian 32bit, Change 32bit data
// between big endian and little endian.
// Return: Return little endian data of input data
// 32bit big endian data
DWORD LE32ToCPU(DWORD Val);

#endif

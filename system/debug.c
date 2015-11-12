#include <reg51.h>										//include public header files
#include "type.h"
#include "syscfg.h"
#include "gpio.h"
#include "clk.h"
#include "rst.h"
#include "playctrl.h"
#include "uart.h"

#if defined(FUNC_UARTDBG_EN)
	#warning	"MV: UART DEBUG ENABLE!"


// Put a char from UART.
CHAR putchar(CHAR c)  
{
	if(c == '\n')
	{
		I51UartSendByte(0x0D);
	}
 	return I51UartSendByte(c);
}


// _getkey, Get a char from UART.
//Return char value from UART 
CHAR _getkey(VOID)  								
{	
	char c;
 	I51UartRecvByte(&c);

	return c;
}

#endif

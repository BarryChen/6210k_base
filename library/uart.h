#ifndef __UART_H__
#define	__UART_H__


//Fast Uart可以选择使用A0E2或B0B1为接口.
#define UART_USE_A1A2	1
#define UART_USE_TXD	2			// this mode only valid for debug-chip,no RXD

// BAUD RATE
// I51Uart Only support 1200/2400/4800 baud-rate.
#define BAUDRATE_115200		115		//tx only, no rxd
#define BAUDRATE_57600		57		//tx only, no rxd
#define BAUDRATE_38400		38		//tx only, no rxd
#define BAUDRATE_4800		48		//154
#define BAUDRATE_2400		24		//310
#define BAUDRATE_1200		12		//623


// 0xF3----2400,误差0.16%
// 0xF9----4800,误差7.8%,所以用0xF3
// 0xE6----1200,误差0.16%
VOID I51UartOpen(BYTE BaudRate,BYTE UartSel);

// Internal Uart Close
VOID I51UartClose(VOID);

// I51Uart send a byte in inquiring mode.
BYTE I51UartSendByte(BYTE c);

// I51Uart receive a byte in inquiring mode.
// If succeed return TRUE,otherwise return FALSE.
BOOL I51UartRecvByte(BYTE* Val);


#endif

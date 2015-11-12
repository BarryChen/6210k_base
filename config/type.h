#ifndef __TYPE_H__
#define __TYPE_H__


#define FALSE			0
#define TRUE			1

#define	SUCCESS			0
#define	FAILURE			1

#define					IN
#define					OUT

#define	BYTE_HIGH		0			
#define	BYTE_LOW		1

#define BYTE0(val)			(((BYTE*)(&val))[0])
#define BYTE1(val)			(((BYTE*)(&val))[1])
#define BYTE2(val)			(((BYTE*)(&val))[2])
#define BYTE3(val)			(((BYTE*)(&val))[3])

#define WORD0(val)			(((WORD*)(&val))[0])
#define WORD1(val)			(((WORD*)(&val))[1])

//#define NULL				(VOID *)0


#define	XDATA			xdata
#define	DATA			data
#define	CODE			code

#define XBYTE 			((unsigned char volatile xdata *) 0)
#define CBYTE 			((unsigned char volatile code *) 0)

#define	BYTE_0_OF(x)	(((unsigned char *)&(x))[0])
#define	BYTE_1_OF(x)	(((unsigned char *)&(x))[1])
#define	BYTE_2_OF(x)	(((unsigned char *)&(x))[2])
#define	BYTE_3_OF(x)	(((unsigned char *)&(x))[3])


#define ACT_CFG_B(x) 		(*((unsigned char code *)(x)))
#define ACT_CFG_W(x) 		(*((unsigned short code *)(x)))
#define ACT_CFG_DW(x) 		(*((unsigned long code *)(x)))


typedef void				VOID, *PVOID;
typedef char				BOOL;
typedef char				BOOLEAN;
typedef char				*PCHAR;
typedef char				CHAR;
typedef unsigned char		BYTE, UCHAR, *PUCHAR;
typedef unsigned short		USHORT, *PUSHORT;
typedef unsigned short		WORD, *PWORD;
typedef unsigned long		DWORD, ULONG, *PULONG;
typedef volatile UCHAR		*PVUCHAR;
typedef volatile USHORT		*PVUSHORT;
typedef volatile ULONG 		*PVULONG;

typedef signed char 		SBYTE;
typedef signed int			SWORD;
typedef signed long			SDWORD;

typedef unsigned long		MEMADDR;
typedef unsigned long		SIZE_T;

typedef char				USB_STATUS;

typedef unsigned char		EVENT;
typedef unsigned char		STATE;

typedef	unsigned short		WCHAR;						//for unicode string.
typedef VOID(*BassLedCallbackFun)(VOID);



#endif


#include<string.h>
#include <reg51.h>
#include <intrins.h>
#include "syscfg.h"
#include "type.h"
#include "utility.h"
#include "debug.h"
#include "i2c.h"
#include "24cxx.h"


#define EEPROM_PAGE_SIZE	8		//8 bytes/page


#ifdef FUNC_BREAK_POINT_EN

#if (FUNC_RESTORE_DEVICE_SELECE == FUNC_RESTORE_DEVICE_EEPROM)
// Read data to buffer from eeprom 
BOOL EEPROM_ReadBytes(BYTE Addr, BYTE* Buf, BYTE Len)
{
	return I2C_ReadNByte(EEPROM_WRITE_ADDR, ((WORD)Addr), ((BYTE*)Buf), (BYTE)Len, EEPROM_IIC);		
}


// Read one byte from eeprom
BYTE EEPROM_ReadByte(BYTE Addr)
{
	BYTE Temp = 0;
	EEPROM_ReadBytes(Addr, &Temp, 1);
	return Temp;
}


// Write data to eeprom from buffer
BOOL EEPROM_WriteBytesInOnePage(BYTE Addr, BYTE* Buf, BYTE Len)
{
	return I2C_WriteNByte(EEPROM_WRITE_ADDR, ((WORD)Addr), ((BYTE*)Buf), ((BYTE)Len), EEPROM_IIC);
}


// Write data to eeprom from buffer
BOOL EEPROM_WriteBytes(BYTE Addr, BYTE* Buf, BYTE Len)
{		
	while(Len)
	{
		BYTE n = EEPROM_PAGE_SIZE - (Addr % EEPROM_PAGE_SIZE);
	 	if(n > Len)
		{
			n = Len;
		}	
			 	
		if(!EEPROM_WriteBytesInOnePage(Addr, Buf, n))
		{
			return FALSE;
		}

		Addr += n;
		Buf	+= n;
		Len -= n;
	}
	return TRUE;	
}


// Write one byte to eeprom
BOOL EEPROM_WriteByte(BYTE Addr, BYTE Dat)
{
	return EEPROM_WriteBytes(Addr, &Dat, 1);
}

#endif
#endif

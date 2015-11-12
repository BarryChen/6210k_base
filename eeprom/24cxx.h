#ifndef __24CXX_H__
#define __24CXX_H__
#include "type.h"

//24X02
#define EEPROM_WRITE_ADDR	0xA0
#define EEPROM_READ_ADDR	0xA1


// Read data to buffer from eeprom
BOOL EEPROM_ReadBytes(BYTE Addr, BYTE* Buf, BYTE Len);

// Read one byte from eeprom
BYTE EEPROM_ReadByte(BYTE Addr);

// Write data to eeprom from buffer 
BOOL EEPROM_WriteBytesInOnePage(BYTE Addr, BYTE* Buf, BYTE Len);

// Write data to eeprom from buffer
BOOL EEPROM_WriteBytes(BYTE Addr, BYTE* Buf, BYTE Len);

// Write one byte to eeprom
BOOL EEPROM_WriteByte(BYTE Addr, BYTE Dat);


#endif
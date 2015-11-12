#ifndef __I2C_H__
#define __I2C_H__

#include "type.h"

#define EEPROM_WRITE_TIME	10		//10ms

#define	IIC_READ	0x01
#define	IIC_WRITE	0x00

#define FM_IIC			0x01
#define EEPROM_IIC		0x02
#define PT231X_IIC  	0x03//bycc
#define NPCA110X_IIC  	0x04//bycc


VOID I2C_Init(VOID);
VOID I2C_Start(BYTE I2cDevice);
VOID I2C_Stop(BYTE I2cDevice);
VOID I2C_SendAck(BYTE I2cDevice);
VOID I2C_SendNoAck(BYTE I2cDevice);
BOOL I2C_ChkAck(BYTE I2cDevice);
BYTE I2C_ReadByte(BYTE I2cDevice);
BOOL I2C_ReadBytes(BYTE* Buf, BYTE Len, BYTE I2cDevice);
BOOL I2C_WriteByte(BYTE Dat, BYTE I2cDevice);
BOOL I2C_WriteDWord(DWORD Dat, BYTE I2cDevice);

BOOL I2C_WriteBytes(BYTE* Buf, BYTE Len, BYTE I2cDevice);
BOOL I2C_WriteNByte(BYTE DevAddr, WORD Addr, BYTE* Buf, BYTE Len, BYTE I2cDevice);
BOOL I2C_ReadNByte(BYTE DevAddr, WORD Addr, BYTE* Buf, BYTE Len, BYTE I2cDevice);
BOOL I2C_SendAddr(BYTE DevAddr, WORD Addr, BYTE ReadWriteFlag, BYTE I2cDevice);

#endif

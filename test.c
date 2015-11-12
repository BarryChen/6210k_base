#include<string.h>
#include <reg51.h>
#include <intrins.h>

#include "syscfg.h"
#include "i2c.h"
#include "debug.h"

#if (defined(FUNC_RADIO_EN))
//SCL
#define FM_PORT_OUT_SCL			GPIO_FMD_OUT
#define FM_PORT_IN_SCL			GPIO_FMD_IN
#define FM_PORT_IE_SCL			GPIO_FMD_IE
#define FM_PORT_OE_SCL			GPIO_FMD_OE	
#define FM_PORT_PU_SCL			GPIO_FMD_PU
#define FM_PORT_PD_SCL			GPIO_FMD_PD
#define	FM_MASK_BIT_SCL 		0x02		

//SDA
#define FM_PORT_OUT_SDA			GPIO_FMD_OUT
#define FM_PORT_IN_SDA			GPIO_FMD_IN
#define FM_PORT_IE_SDA			GPIO_FMD_IE
#define FM_PORT_OE_SDA			GPIO_FMD_OE		
#define FM_PORT_PU_SDA			GPIO_FMD_PU
#define FM_PORT_PD_SDA			GPIO_FMD_PD
#define	FM_MASK_BIT_SDA 		0x01
#endif


#if (defined(FUNC_PT231X_IIC) || \
	 defined(FUNC_RADIO_EN)	|| \
	 defined(FUNC_NPCA110X_EN) || \
	 (defined(FUNC_BREAK_POINT_EN) && (FUNC_RESTORE_DEVICE_SELECE == FUNC_RESTORE_DEVICE_EEPROM)))
//#define IMPROVE_FM_SEEK_NOISE	//针对不同硬件设计，改善FM I2C通讯(搜台或切换频点操作)可能引起的噪声问题


#define	KtIICDelay()	WaitUs(20)


VOID SetOutputSCL(BYTE I2cDevice)
{
	EA = 0;
#if defined(FUNC_PT231X_IIC)||defined(AU6210K_NR_D_8_CSRBT)
	if((I2cDevice == PT231X_IIC) || (I2cDevice == NPCA110X_IIC))
	{
		baGPIOCtrl[PORT_OE_SCL] |= (MASK_BIT_SCL);
		baGPIOCtrl[PORT_IE_SCL] &= (~MASK_BIT_SCL);
	}
	
#else	

#if	(defined(FUNC_BREAK_POINT_EN) && (FUNC_RESTORE_DEVICE_SELECE == FUNC_RESTORE_DEVICE_EEPROM))
	if(I2cDevice == EEPROM_IIC)
	{
		baGPIOCtrl[PORT_OE_SCL] |= (MASK_BIT_SCL);
		baGPIOCtrl[PORT_IE_SCL] &= (~MASK_BIT_SCL);
	}
#endif
#if ((defined(FUNC_RADIO_EN)) && (defined(FUNC_BREAK_POINT_EN) && (FUNC_RESTORE_DEVICE_SELECE == FUNC_RESTORE_DEVICE_EEPROM)))
	else 
#endif
#if (defined(FUNC_RADIO_EN))	
	if(I2cDevice == FM_IIC)
	{	
#if !defined(IMPROVE_FM_SEEK_NOISE)
		baGPIOCtrl[FM_PORT_OE_SCL] |= (FM_MASK_BIT_SCL);
		baGPIOCtrl[FM_PORT_IE_SCL] &= (~FM_MASK_BIT_SCL);	
#endif

	}
#endif	

#endif
	EA = 1;
}	

						
VOID SetSCL(BYTE I2cDevice)
{		
	EA = 0;
#if defined(FUNC_PT231X_IIC)||defined(AU6210K_NR_D_8_CSRBT)

	if((I2cDevice == PT231X_IIC) || (I2cDevice == NPCA110X_IIC))
	{
		baGPIOCtrl[PORT_OUT_SCL] |= MASK_BIT_SCL;
	}
#else	
	
#if	(defined(FUNC_BREAK_POINT_EN) && (FUNC_RESTORE_DEVICE_SELECE == FUNC_RESTORE_DEVICE_EEPROM))
	if(I2cDevice == EEPROM_IIC)
	{
		baGPIOCtrl[PORT_OUT_SCL] |= MASK_BIT_SCL;
	}
#endif
#if ((defined(FUNC_RADIO_EN)) && (defined(FUNC_BREAK_POINT_EN) && (FUNC_RESTORE_DEVICE_SELECE == FUNC_RESTORE_DEVICE_EEPROM)))
	else
#endif
#if (defined(FUNC_RADIO_EN))
	if(I2cDevice == FM_IIC)
	{
#if defined(IMPROVE_FM_SEEK_NOISE)
		baGPIOCtrl[FM_PORT_PU_SCL] &= (~FM_MASK_BIT_SCL);	
		baGPIOCtrl[FM_PORT_PD_SCL] &= (~FM_MASK_BIT_SCL);
		baGPIOCtrl[FM_PORT_OE_SCL] &= (~FM_MASK_BIT_SCL);
		baGPIOCtrl[FM_PORT_IE_SCL] |= FM_MASK_BIT_SCL;	
#else
		baGPIOCtrl[FM_PORT_OUT_SCL] |= FM_MASK_BIT_SCL;
#endif
	}
#endif

#endif
	EA = 1;
}

					
VOID ClrSCL(BYTE I2cDevice)
{		
	EA = 0;
#if defined(FUNC_PT231X_IIC)||defined(AU6210K_NR_D_8_CSRBT)
	if((I2cDevice == PT231X_IIC) || (I2cDevice == NPCA110X_IIC))
	{
		baGPIOCtrl[PORT_OUT_SCL] &= (~MASK_BIT_SCL);
	}
#else	
#if	(defined(FUNC_BREAK_POINT_EN) && (FUNC_RESTORE_DEVICE_SELECE == FUNC_RESTORE_DEVICE_EEPROM))
	if(I2cDevice == EEPROM_IIC)
	{
		baGPIOCtrl[PORT_OUT_SCL] &= (~MASK_BIT_SCL);
	}
#endif 
#if ((defined(FUNC_RADIO_EN)) && (defined(FUNC_BREAK_POINT_EN) && (FUNC_RESTORE_DEVICE_SELECE == FUNC_RESTORE_DEVICE_EEPROM)))
	else
#endif 
#if (defined(FUNC_RADIO_EN))
if(I2cDevice == FM_IIC)
	{
#if defined(IMPROVE_FM_SEEK_NOISE)
		baGPIOCtrl[FM_PORT_PU_SCL] |= FM_MASK_BIT_SCL;	
		baGPIOCtrl[FM_PORT_PD_SCL] |= (FM_MASK_BIT_SCL);
		baGPIOCtrl[FM_PORT_OE_SCL] &= (~FM_MASK_BIT_SCL);
		baGPIOCtrl[FM_PORT_IE_SCL] |= FM_MASK_BIT_SCL;	
#else
		baGPIOCtrl[FM_PORT_OUT_SCL] &= (~FM_MASK_BIT_SCL);
#endif
	}
#endif

#endif
	EA = 1;
}


VOID SetInputSDA(BYTE I2cDevice)
{	
	EA = 0;
#if defined(FUNC_PT231X_IIC)||defined(AU6210K_NR_D_8_CSRBT)
	if((I2cDevice == PT231X_IIC) || (I2cDevice == NPCA110X_IIC))
	{
		baGPIOCtrl[PORT_OE_SDA] &= (~MASK_BIT_SDA);
		baGPIOCtrl[PORT_IE_SDA] |= MASK_BIT_SDA;
	}
#else
#if	(defined(FUNC_BREAK_POINT_EN) && (FUNC_RESTORE_DEVICE_SELECE == FUNC_RESTORE_DEVICE_EEPROM))
	if(I2cDevice == EEPROM_IIC)
	{
		baGPIOCtrl[PORT_OE_SDA] &= (~MASK_BIT_SDA);
		baGPIOCtrl[PORT_IE_SDA] |= MASK_BIT_SDA;
	}
#endif
#if ((defined(FUNC_RADIO_EN)) && (defined(FUNC_BREAK_POINT_EN) && (FUNC_RESTORE_DEVICE_SELECE == FUNC_RESTORE_DEVICE_EEPROM)))
	else
#endif
#if (defined(FUNC_RADIO_EN))
	if(I2cDevice == FM_IIC)
	{
		baGPIOCtrl[FM_PORT_OE_SDA] &= (~FM_MASK_BIT_SDA);
		baGPIOCtrl[FM_PORT_IE_SDA] |= FM_MASK_BIT_SDA;
	}
#endif
#endif
	EA = 1;
}

						
VOID SetOutputSDA(BYTE I2cDevice)
{	
	EA = 0;
#if defined(FUNC_PT231X_IIC)||defined(AU6210K_NR_D_8_CSRBT)
	if((I2cDevice == PT231X_IIC) || (I2cDevice == NPCA110X_IIC))
	{
		baGPIOCtrl[PORT_OE_SDA] |= MASK_BIT_SDA;
		baGPIOCtrl[PORT_IE_SDA] &= (~MASK_BIT_SDA);
	}
#else
	
#if	(defined(FUNC_BREAK_POINT_EN) && (FUNC_RESTORE_DEVICE_SELECE == FUNC_RESTORE_DEVICE_EEPROM))
	if(I2cDevice == EEPROM_IIC)
	{
		baGPIOCtrl[PORT_OE_SDA] |= MASK_BIT_SDA;
		baGPIOCtrl[PORT_IE_SDA] &= (~MASK_BIT_SDA);
	}
#endif
#if ((defined(FUNC_RADIO_EN)) && (defined(FUNC_BREAK_POINT_EN) && (FUNC_RESTORE_DEVICE_SELECE == FUNC_RESTORE_DEVICE_EEPROM)))
	else
#endif
#if (defined(FUNC_RADIO_EN))
	if(I2cDevice == FM_IIC)
	{
#if !defined(IMPROVE_FM_SEEK_NOISE)
		baGPIOCtrl[FM_PORT_OE_SDA] |= FM_MASK_BIT_SDA;
		baGPIOCtrl[FM_PORT_IE_SDA] &= (~FM_MASK_BIT_SDA);
#endif
	}
#endif

#endif
	EA = 1;
}

						
VOID SetSDA(BYTE I2cDevice)
{
	EA = 0;
#if defined(FUNC_PT231X_IIC)||defined(AU6210K_NR_D_8_CSRBT)
	if((I2cDevice == PT231X_IIC) || (I2cDevice == NPCA110X_IIC))
	{
		baGPIOCtrl[PORT_OUT_SDA] |= MASK_BIT_SDA;
	}
#else
#if	(defined(FUNC_BREAK_POINT_EN) && (FUNC_RESTORE_DEVICE_SELECE == FUNC_RESTORE_DEVICE_EEPROM))
	if(I2cDevice == EEPROM_IIC)
	{
		baGPIOCtrl[PORT_OUT_SDA] |= MASK_BIT_SDA;
	}
#endif
#if ((defined(FUNC_RADIO_EN)) && (defined(FUNC_BREAK_POINT_EN) && (FUNC_RESTORE_DEVICE_SELECE == FUNC_RESTORE_DEVICE_EEPROM)))
	else
#endif
#if (defined(FUNC_RADIO_EN))
	if(I2cDevice == FM_IIC)
	{	
#if defined(IMPROVE_FM_SEEK_NOISE)
		baGPIOCtrl[FM_PORT_PU_SDA] &= (~FM_MASK_BIT_SDA);	
		baGPIOCtrl[FM_PORT_PD_SDA] &= (~FM_MASK_BIT_SDA);
		baGPIOCtrl[FM_PORT_OE_SDA] &= (~FM_MASK_BIT_SDA);
		baGPIOCtrl[FM_PORT_IE_SDA] |= FM_MASK_BIT_SDA;	
#else
		baGPIOCtrl[FM_PORT_OUT_SDA] |= FM_MASK_BIT_SDA;
#endif
	}
#endif
#endif
	EA = 1;
}

					
VOID ClrSDA(BYTE I2cDevice)
{
	EA = 0;
#if defined(FUNC_PT231X_IIC)||defined(AU6210K_NR_D_8_CSRBT)
	if((I2cDevice == PT231X_IIC) || (I2cDevice == NPCA110X_IIC))
	{
		baGPIOCtrl[PORT_OUT_SDA] &= (~MASK_BIT_SDA);
	}
	
#else
#if	(defined(FUNC_BREAK_POINT_EN) && (FUNC_RESTORE_DEVICE_SELECE == FUNC_RESTORE_DEVICE_EEPROM))
	if(I2cDevice == EEPROM_IIC)
	{
		baGPIOCtrl[PORT_OUT_SDA] &= (~MASK_BIT_SDA);
	}
#endif
#if ((defined(FUNC_RADIO_EN)) && (defined(FUNC_BREAK_POINT_EN) && (FUNC_RESTORE_DEVICE_SELECE == FUNC_RESTORE_DEVICE_EEPROM)))
	else
#endif
#if (defined(FUNC_RADIO_EN))
	if(I2cDevice == FM_IIC)
	{	
#if defined(IMPROVE_FM_SEEK_NOISE)
		baGPIOCtrl[FM_PORT_PU_SDA] |= FM_MASK_BIT_SDA;	
		baGPIOCtrl[FM_PORT_PD_SDA] |= FM_MASK_BIT_SDA;
		baGPIOCtrl[FM_PORT_OE_SDA] &= (~FM_MASK_BIT_SDA);
		baGPIOCtrl[FM_PORT_IE_SDA] |= FM_MASK_BIT_SDA;	
#else
		baGPIOCtrl[FM_PORT_OUT_SDA] &= (~FM_MASK_BIT_SDA);
#endif
	}
#endif
#endif
	EA = 1;
}

					
BYTE GetSDA(BYTE I2cDevice)
{
#if defined(FUNC_PT231X_IIC)||defined(AU6210K_NR_D_8_CSRBT)
	if((I2cDevice == PT231X_IIC) || (I2cDevice == NPCA110X_IIC))
	{		
		return (baGPIOCtrl[PORT_IN_SDA] & MASK_BIT_SDA);
	}
	
#else

#if	(defined(FUNC_BREAK_POINT_EN) && (FUNC_RESTORE_DEVICE_SELECE == FUNC_RESTORE_DEVICE_EEPROM))	
	if(I2cDevice == EEPROM_IIC)
	{		
		return (baGPIOCtrl[PORT_IN_SDA] & MASK_BIT_SDA);
	}
#endif
#if ((defined(FUNC_RADIO_EN)) && (defined(FUNC_BREAK_POINT_EN) && (FUNC_RESTORE_DEVICE_SELECE == FUNC_RESTORE_DEVICE_EEPROM)))
	else
#endif
#if (defined(FUNC_RADIO_EN))
	if(I2cDevice == FM_IIC)
	{
		return	(baGPIOCtrl[FM_PORT_IN_SDA] & FM_MASK_BIT_SDA);
	}
#endif
#endif
}


VOID I2C_Init(VOID)
{
	baGPIOCtrl[PORT_OE_SCL] |= MASK_BIT_SCL;
	baGPIOCtrl[PORT_IE_SCL] &= (~MASK_BIT_SCL);
	baGPIOCtrl[PORT_DS_SCL] |= MASK_BIT_SCL;
	baGPIOCtrl[PORT_PU_SCL] &= (~MASK_BIT_SCL);
	baGPIOCtrl[PORT_PD_SCL] &= (~MASK_BIT_SCL);

	baGPIOCtrl[PORT_OE_SDA] |= MASK_BIT_SDA;
	baGPIOCtrl[PORT_IE_SDA] &= (~MASK_BIT_SDA);
	baGPIOCtrl[PORT_DS_SDA] |= MASK_BIT_SDA;
	baGPIOCtrl[PORT_PU_SDA] &= (~MASK_BIT_SDA);
	baGPIOCtrl[PORT_PD_SDA] &= (~MASK_BIT_SDA);	
}


// send START signal. 
VOID I2C_Start(BYTE I2cDevice)
{
	SetSDA(I2cDevice);
	SetSCL(I2cDevice);
	
	SetOutputSCL(I2cDevice);
	SetOutputSDA(I2cDevice);

	SetSDA(I2cDevice);
	SetSCL(I2cDevice);
	KtIICDelay();
	ClrSDA(I2cDevice);
	ClrSCL(I2cDevice);
}


// send STOP signal. 
VOID I2C_Stop(BYTE I2cDevice)
{
//	DBG(("I2C_Stop()\n")); 
	SetOutputSCL(I2cDevice);
	SetOutputSDA(I2cDevice);
	
	ClrSDA(I2cDevice); 	
	KtIICDelay();
	SetSCL(I2cDevice);
	SetSDA(I2cDevice);

	SetInputSDA(I2cDevice);
}


// send Acknowledgement(ACK or NACK) signal. 
VOID I2C_SendAck(BYTE I2cDevice)
{
//	DBG(("I2C_SendAck()\n")); 	
	ClrSDA(I2cDevice); 
	SetSCL(I2cDevice);
	KtIICDelay();
	ClrSCL(I2cDevice);
}


// send Acknowledgement(ACK or NACK) signal. 
VOID I2C_SendNoAck(BYTE I2cDevice)
{
	SetSDA(I2cDevice); 
	SetSCL(I2cDevice);
	KtIICDelay();
	ClrSCL(I2cDevice);
}


// Check Ackowledge signal(ACK/NACK). 
BOOL I2C_ChkAck(BYTE I2cDevice)
{
	BOOL Ack;

	ClrSCL(I2cDevice);
	SetInputSDA(I2cDevice);
	SetSCL(I2cDevice);
	KtIICDelay();
	Ack = !GetSDA(I2cDevice);
	ClrSCL(I2cDevice);
	return Ack;
}

BOOL I2C_WriteDWord(DWORD Dat, BYTE I2cDevice)
{
	BYTE i = sizeof(DWORD);

	DBG(("I2C_WriteByte(%X)\n", Dat)); 
	SetOutputSDA(I2cDevice);	 
	ClrSCL(I2cDevice);
	while(i--)
	{
		if(Dat & 0x80000000)				//MSB output first
		{
		  	SetSDA(I2cDevice); 
		}
		else
		{
			ClrSDA(I2cDevice); 
		}
		Dat <<= 1;
		SetSCL(I2cDevice);
		KtIICDelay();
		ClrSCL(I2cDevice);
	}
	return I2C_ChkAck(I2cDevice);
}

// Send one byte via I2C(check ACK). 
BOOL I2C_WriteByte(BYTE Dat, BYTE I2cDevice)
{
	BYTE i = 8;
	static BYTE j = 1;
	//DBG(("(%-.2BX)", Dat)); 
	
	SetOutputSDA(I2cDevice);	 
	ClrSCL(I2cDevice);
	while(i--)
	{
		if(Dat & 0x80)				//MSB output first
		{
		  	SetSDA(I2cDevice); 
		}
		else
		{
			ClrSDA(I2cDevice); 
		}
		Dat <<= 1;
		SetSCL(I2cDevice);
		KtIICDelay();
		ClrSCL(I2cDevice);
	}
	return I2C_ChkAck(I2cDevice);
}


// Receive one byte via I2C. 
BYTE I2C_ReadByte(BYTE I2cDevice)
{
	BYTE i = 8;
	BYTE Dat = 0;

	SetInputSDA(I2cDevice);
	while(i--)
	{	
		Dat <<= 1;
		if(GetSDA(I2cDevice))
		{
		  	Dat |= 0x01;
		}
		SetSCL(I2cDevice);
		KtIICDelay();
		ClrSCL(I2cDevice);	 
	}
	SetOutputSDA(I2cDevice);
	return Dat;
}


BOOL I2C_WriteBytes(BYTE* Buf, BYTE Len, BYTE I2cDevice)
{
	while(Len--)
	{
		if(!I2C_WriteByte(*(Buf++), I2cDevice))
		{
			DBG(("I2C WR err\n"));
			return FALSE;
		}
	}

	return TRUE;
}


// Read data via I2C. 
BOOL I2C_ReadBytes(BYTE* Buf, BYTE Len, BYTE I2cDevice)
{
	while(Len--)
	{
		*(Buf++) = I2C_ReadByte(I2cDevice);
		if(Len == 0)
		{
			I2C_SendNoAck(I2cDevice);
		}
		else
		{
			I2C_SendAck(I2cDevice);
		}
	}

	return TRUE;
}


extern BOOL SongPlayDo(VOID);		
extern VOID KeyEventGet(VOID);	
// Write data to eeprom from buffer
BOOL I2C_WriteNByte(BYTE DevAddr, WORD Addr, BYTE* Buf, BYTE Len, BYTE I2cDevice)
{
	BOOL Ack = FALSE;

#if (defined(FUNC_BREAK_POINT_EN) && (FUNC_RESTORE_DEVICE_SELECE == FUNC_RESTORE_DEVICE_EEPROM))

	if(((gSys.SystemMode == SYS_MODE_USB) || (gSys.SystemMode == SYS_MODE_SD)) && (I2cDevice == EEPROM_IIC)) 
	{
#if (I2C_PIN_CONFIGURE == SDIO_TO_A3A4A5)
		SET_CARD_TO_A3A4A5();
#if defined(SD_DETECT_PIN_USE_A4)
		ClrGpioRegBit(CARD_DETECT_PORT_PD, CARD_DETECT_BIT);
#endif
#endif
		//写E2PROM时快速响应需要读卡时需要重新将复用IO设置回总线模式
		SongPlayDo();

#if (I2C_PIN_CONFIGURE == SDIO_TO_A3A4A5)
#if defined(SD_DETECT_PIN_USE_A4)		
		SetGpioRegBit(CARD_DETECT_PORT_PD, CARD_DETECT_BIT);
#endif
		SET_CARD_NOT_TO_GPIO();
#endif
		KeyEventGet();		
	}
#endif
		
	Ack = I2C_SendAddr(DevAddr, Addr, IIC_WRITE, I2cDevice);
	if(Ack == TRUE)
	{
		Ack = I2C_WriteBytes(Buf, Len, I2cDevice);
	}
	I2C_Stop(I2cDevice);

//	DBG_APP(("I2C_WriteNByte, ACK: %bu\n", (BYTE)Ack));
	return Ack;
}


// Read data from eeprom.
BOOL I2C_ReadNByte(BYTE DevAddr, WORD Addr, BYTE* Buf, BYTE Len, BYTE I2cDevice)
{
    BOOL Ack = FALSE;
#if (defined(FUNC_BREAK_POINT_EN) && (FUNC_RESTORE_DEVICE_SELECE == FUNC_RESTORE_DEVICE_EEPROM))

if(((gSys.SystemMode == SYS_MODE_USB) || (gSys.SystemMode == SYS_MODE_SD)) && (I2cDevice == EEPROM_IIC)) 
	{
#if (I2C_PIN_CONFIGURE == SDIO_TO_A3A4A5)
		SET_CARD_TO_A3A4A5();
#if defined(SD_DETECT_PIN_USE_A4)
		ClrGpioRegBit(CARD_DETECT_PORT_PD, CARD_DETECT_BIT);
#endif
#endif
		//写E2PROM时快速响应需要读卡时需要重新将复用IO设置回总线模式
		SongPlayDo();

#if (I2C_PIN_CONFIGURE == SDIO_TO_A3A4A5)
#if defined(SD_DETECT_PIN_USE_A4)		
		SetGpioRegBit(CARD_DETECT_PORT_PD, CARD_DETECT_BIT);
#endif
		SET_CARD_NOT_TO_GPIO();
#endif	
	}

#endif

	Ack = I2C_SendAddr(DevAddr, Addr, IIC_READ, I2cDevice);
	if(Ack == TRUE)
	{
		Ack = I2C_ReadBytes(Buf, Len, I2cDevice);
	}
	I2C_Stop(I2cDevice);

	return Ack;
}


// send address via IIC.
BOOL I2C_SendAddr(BYTE DevAddr, WORD Addr, BYTE ReadWriteFlag, BYTE I2cDevice)
{	
	I2C_Start(I2cDevice);
	if(!I2C_WriteByte(DevAddr, I2cDevice))
	{
		WaitMs(EEPROM_WRITE_TIME); // Wairt for Programming-time.
		I2C_Start(I2cDevice);
		if(!I2C_WriteByte(DevAddr, I2cDevice))// | PageAddr))
		{
			I2C_Stop(I2cDevice);
			return FALSE;
		}
	}
	
   	if(!I2C_WriteByte((BYTE)Addr, I2cDevice))
	{
	 	I2C_Stop(I2cDevice);
		return FALSE;
	}
	
	if(ReadWriteFlag == IIC_READ)
	{
		I2C_Start(I2cDevice);
		if(!I2C_WriteByte((DevAddr | IIC_READ), I2cDevice))
		{
		 	I2C_Stop(I2cDevice);
			return FALSE;
		}
	}
	
	return TRUE;
}


#endif


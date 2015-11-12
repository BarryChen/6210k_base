#include <reg51.h>
#include <string.h>
#include "type.h"
#include "syscfg.h"
#include "buffer.h"
#include "spi_flash.h"
#include "utility.h"
#include "dir.h"
#include "file.h"
#include "debug.h"

#ifdef FUNC_SPI_FLASH_EN

SPI_FLASH_MODEL	SpiFlashModel;

// SST Status Register bit.
#define	SPITESTADDR	0x0123
#define	BPL		0x80
#define	BP3		0x20
#define	BP2		0x10
#define	BP1		0x08
#define	BP0		0x04

#define FLASH_CS_PORT	GPIO_B_IN
#define FLASH_DI_PORT	GPIO_B_IN
#define FLASH_DO_PORT	GPIO_B_IN
#define FLASH_CLK_PORT	GPIO_B_IN

#define FLASH_CS_PIN		0x08//b3
#define FLASH_DI_PIN		0x02//b1
#define FLASH_DO_PIN		0x04//b2
#define FLASH_CLK_PIN		0x01//b0

#define	SpiFlashCsAssert()		baGPIOCtrl[FLASH_CS_PORT + 1] &= (~FLASH_CS_PIN)
#define	SpiFlashCsDeassert()	baGPIOCtrl[FLASH_CS_PORT + 1] |= FLASH_CS_PIN

extern BOOL SpiSendByte(BYTE Dat);
extern BYTE SpiRecvByte(VOID);

BYTE SPI_DI = FLASH_DI_PIN; 


VOID SpiInit(VOID)
{
	baGPIOCtrl[SPI_FLASH_CS_PORT + 1] |= FLASH_CS_PIN;
	baGPIOCtrl[FLASH_CLK_PORT + 1] |= FLASH_CLK_PIN;
	baGPIOCtrl[FLASH_DI_PORT + 1] |= FLASH_DI_PIN;
	baGPIOCtrl[FLASH_CS_PORT + 3] |= FLASH_CS_PIN;
	baGPIOCtrl[FLASH_CLK_PORT + 3] |= FLASH_CLK_PIN;
	baGPIOCtrl[FLASH_DO_PORT + 3] &= (~FLASH_DO_PIN);
	baGPIOCtrl[FLASH_DI_PORT + 3] |= FLASH_DI_PIN;
}


//SPI接口连续发送N个字节
VOID SpiSendNBytes(WORD SendNum, BYTE* Buffer)
{
	WORD i;
	BYTE* p;
	p = Buffer;
	
	for(i = 0; i < SendNum; i++)
	{
		SpiSendByte(*p);
		p++;		
	}
}


// Brief: Send command.
VOID SPIFlashSendCMD(BYTE Cmd)
{
	SpiFlashCsAssert();
	SpiSendByte(Cmd);	//	instruction
}


// Brief: Write Status Register.
VOID SPIFlashSendAddr(DWORD Addr)
{
	SpiSendByte(((BYTE*)&Addr)[1]);	
	SpiSendByte(((BYTE*)&Addr)[2]);		
	SpiSendByte(((BYTE*)&Addr)[3]);		
}


// Brief: Write Status Register.
//Bit7--6--5---4---3---2---1---0---
//BPL AAI BP3 BP2 BP1 BP0 WEL BUSY
VOID SPIFlashWriteReg(BYTE DataNum)
{
	SPIFlashSendCMD(FLASH_EWSR);//GD Write enable  06
	SpiFlashCsDeassert();
	
	SPIFlashSendCMD(FLASH_WRSR);
	SpiSendByte(DataNum);
	SpiFlashCsDeassert();
}


// Brief: Check the busy status.
VOID SPIFlashWaitReady(VOID)
{
	BYTE Status;
		
	do
	{//wait chip ready
		SPIFlashSendCMD(FLASH_RDSR);
		Status = SpiRecvByte();	
		SpiFlashCsDeassert();
		WaitMs(1);
	}
	while((Status & 0x01) != 0);
}


// SST Multi-Bytes Read(High-Speed).
VOID SSTReadNBytes(DWORD Addr, BYTE* Buffer, WORD Length)
{
	WORD Temp = 0;

	SPIFlashSendCMD(FLASH_FAST_READ);
	SPIFlashSendAddr(Addr);
	SpiSendByte(0xFF);		//a dummy byte
	
	while(Temp < Length)
	{
		Buffer[Temp] = SpiRecvByte();
		Temp++;
	}
	
	SpiFlashCsDeassert();	
}


// Brief: SST Block/Chip Earse.
// @Note: Block erase指令擦除的Block的大小与具体的Flash型号有关.
VOID SSTErase(DWORD Addr, BYTE Cmd)
{
	DBG(("->SSTErase\n"));

	SPIFlashWriteReg(~(BPL | BP3 | BP2 | BP1 | BP0));
	SPIFlashWaitReady();
	SPIFlashSendCMD(FLASH_WREN);
	SpiFlashCsDeassert();
	SPIFlashSendCMD(Cmd);
	
	if(Cmd != FLASH_CE)
	{
		SPIFlashSendAddr(Addr);
	}
	
	SpiFlashCsDeassert();
//	DBG(("SpiFlashErase,Wait\n"));
	SPIFlashWaitReady();
	DBG(("<-SSTErase\n"));
}


// Breif: SST Byte-Program
VOID SSTByteProgram(DWORD Addr, BYTE DataNum)
{
	SPIFlashSendCMD(FLASH_WREN);
	SpiFlashCsDeassert();
	
	SPIFlashSendCMD(FLASH_BP);
	SPIFlashSendAddr(Addr);
	SpiSendByte(DataNum);
	SpiFlashCsDeassert();
	SPIFlashWaitReady();
}


// Brief: SST Auto-Address-Increment programming.
VOID SSTAAIProgram(DWORD Addr, BYTE* Buffer, WORD Length)
{
	WORD TempCount;

	if(Length == 0)
	{
		return;
	}
	
	if((Addr & 0x01) != 0)
	{
		SSTByteProgram(Addr++, *Buffer++);
		Length -= 1;
	}
	
	SPIFlashSendCMD(FLASH_WREN);
	SpiFlashCsDeassert();
	SPIFlashSendCMD(FLASH_AAI);
	SPIFlashSendAddr(Addr);
	SpiSendByte(*Buffer++);
	SpiSendByte(*Buffer++);
	SpiFlashCsDeassert();
	WaitUs(1);	//wait > 10 Us or SPIFlashWaitReady();
	
	for(TempCount = (Length - 2); TempCount > 1; )
	{
		SPIFlashSendCMD(FLASH_AAI);
		SpiSendByte(*Buffer++);
		SpiSendByte(*Buffer++);
		SpiFlashCsDeassert();
		TempCount -= 2;
		WaitUs(1);				//wait > 10 Us or SPIFlashWaitReady();
	}
	
	SPIFlashSendCMD(FLASH_WRDI);	// exit AAI mode
	SpiFlashCsDeassert();
	SPIFlashWaitReady();
	
	if((Length & 0x01) != 0)
	{
		SSTByteProgram((Addr + Length - 1), *Buffer);
	}
}


// Brief: GD Block/Chip Earse.
// @Note: Block erase指令擦除的Block的大小与具体的Flash型号有关.
VOID GDFlashErase(DWORD Addr, BYTE Cmd)
{
	DBG(("->GDErase\n"));

	SPIFlashWriteReg((~( BP2 | BP1 | BP0)) & 0x1C); 
	SPIFlashWaitReady();

	SPIFlashSendCMD(FLASH_WREN);
	SpiFlashCsDeassert();
	SPIFlashSendCMD(Cmd);	
	//if(Cmd != FLASH_CE)
	//if(Cmd != FLASH_CHIPERASE)	
	{
		//SPIFlashSendAddr(Addr);
	}
	SpiFlashCsDeassert();
	//DBG(("SpiFlashErase,Wait\n"));
	SPIFlashWaitReady();
	DBG(("<-GDErase\n"));
}


//GD Multi-Bytes Read(High-Speed).
VOID GDReadNBytes(DWORD Addr, BYTE* Buffer, WORD Length)
{
	WORD Temp = 0;
	
	SPIFlashSendCMD(FLASH_FAST_READ);
	SPIFlashSendAddr(Addr);
	SpiSendByte(0xFF);	//a dummy byte
	
	while(Temp < Length)
	{
		Buffer[Temp] = SpiRecvByte();
		//DBG((" 0x%BX", (BYTE) Buffer[sTemp]));
		//if(!(sTemp%16))
		//DBG(("\n"));	
		Temp++;
	}
	
	SpiFlashCsDeassert();	
}


// Breif:GD Byte-Program
VOID GDByteProgram(DWORD Addr,	BYTE DataNum)
{
	
}


// Breif:GD Page-Program
VOID GDPageProgram(DWORD Addr, BYTE* Buffer, WORD Length)
{
	if((Length == 0) || (Length > MX_PP_LENGTH))
	{
		return;
	}
	
	SPIFlashSendCMD(FLASH_WREN);
	SpiFlashCsDeassert();
	
	SPIFlashSendCMD(FLASH_PP);
	SPIFlashSendAddr(Addr);
	SpiSendNBytes(Length, Buffer);
	SpiFlashCsDeassert();
	SPIFlashWaitReady();
}


// Breif:GD & Natan Page-Program
VOID GDMultiBytePrgm(DWORD Addr, BYTE* Buffer, WORD Length)
{
	BYTE Temp;
	WORD Iloop;
	
	Temp = 0xFF - (BYTE)(Addr & 0x000000FF) + 1;
	
	if(Temp != 0)
	{
		GDPageProgram(Addr, Buffer, Temp);
		Addr += Temp;
		Buffer += Temp;
		Length -= Temp;
		DBG(("Temp%bu\n", Temp));
	}
	
	if(Length >= MX_PP_LENGTH)
	{		
		for(Iloop = 0; Iloop < (Length / MX_PP_LENGTH); Iloop++)
		{
			GDPageProgram(Addr, Buffer, MX_PP_LENGTH);
			Addr += MX_PP_LENGTH;
			Buffer += MX_PP_LENGTH;
			DBG(("loop%d\n", Iloop));
		}
	}
	if((Length %= MX_PP_LENGTH) != 0)
	{
		GDPageProgram(Addr, Buffer, Length);
		DBG((" yusu\n"));
	}
}


// Brief: Read serial flash Manufacturer's ID and deviceID.
VOID SpiFlashIdRead(VOID)
{
	SERIAL_FLASH_ID	IdVal;

	SpiFlashCsAssert();
	
	SpiSendByte(FLASH_RDID);		
	IdVal.Mid = SpiRecvByte();	
	((BYTE *)IdVal.Did)[0] = SpiRecvByte();   //just need the manfacture's ID
	((BYTE *)IdVal.Did)[1] = SpiRecvByte();

	SpiFlashCsDeassert();

	DBG(("FlashID=0x%BX\n", IdVal.Mid));

	SpiFlashModel = SpiFlash_None;
	if(IdVal.Mid == 0xEF)	//WINBOND
	{
		SpiFlashModel = SpiFlash_WINBOND;
	}
	else if(IdVal.Mid == 0xC8)	//GD
	{
		SpiFlashModel = SpiFlash_GD;
	}
	else if(IdVal.Mid == 0xC2)	//MX
	{
		SpiFlashModel = SpiFlash_MX;
	}
	else if(IdVal.Mid == 0xBF)	//sst
	{
		SpiFlashModel = SpiFlash_SST;
	}
	else
	{
		DBG(("unsupport spi dev!\n"));
	}
}


// Brief: Inital SPI device structure.
VOID SpiFlashInit(VOID)
{
	SetGpioRegBit((SPI_FLASH_CS_PORT+3), SPI_FLASH_CS_PIN);		//CS OE Output
	SpiInit();
	SpiFlashModel = SpiFlash_None;	
//	SpiFlashCsDeassert();

	SpiFlashIdRead();

	if(SpiFlashModel == SpiFlash_None)
	{
		DBG(("unsupport spi dev!\n"));
	}
	else if(SpiFlash_GD == SpiFlashModel)
	{
		SPIFlashWriteReg((BP2 | BP1 | BP0)); 
		SPIFlashWaitReady();
	}
}


// Brief: Block/Chip Earse.
// @Note: Block erase指令擦除的Block的大小与具体的Flash型号有关.
VOID SpiFlashErase(VOID)
{
	DBG(("->SpiFlashErase\n"));
	if (SpiFlashModel == SpiFlash_SST)
	{
		SSTErase(0, FLASH_CE);
	}
//	else if(SpiFlashModel == SpiFlash_WINBOND)
	else if(SpiFlashModel == SpiFlash_GD)	//GD
	{
		GDFlashErase(0, FLASH_CE);
	}
//	else if(SpiFlashModel == SpiFlash_MX)	//MX
//	else

	DBG(("<-SpiFlashErase\n"));
}


// Brief: Read block from SPI flash Device
VOID SpiFlashBlkRd(DWORD StartAddr, BYTE* Buffer, WORD Length)
{
	//DBG(("->SpiFlashBlkRd\n"));
	if((Buffer == NULL) || (Length == 0))
	{
		return;
	}

	if((WORD)Buffer >= DMA_XBUF_START)
	{
		if(Length % 4)
		{// align to quatuor-bytes
			Length += (4 - (Length % 4));	
		}
	}
	
	if (SpiFlashModel == SpiFlash_SST)
	{
		SSTReadNBytes(StartAddr, Buffer, Length);		
	}
//	else if(SpiFlashModel == SpiFlash_WINBOND)
	else if(SpiFlashModel == SpiFlash_GD)	//GD
	{
		GDReadNBytes(StartAddr, Buffer, Length);			
	}
//	else if(SpiFlashModel == SpiFlash_MX)	//MX
//	else

	if((WORD)Buffer >= DMA_XBUF_START)
	{
		//ReleaseREQ();
	}

	//DBG(("<-SpiFlashBlkRd\n"));
}


// Brief: Multi-Bytes Program.
VOID SPIFlashMultiBytePrgm(DWORD Addr, BYTE* Buffer, WORD Length)
{
	if (SpiFlashModel == SpiFlash_SST)
	{
		SSTAAIProgram(Addr, Buffer, Length);	
	}
//	else if(SpiFlashModel == SpiFlash_WINBOND)
	else if(SpiFlashModel == SpiFlash_GD) //GD
	{
		DBG(("->GDPageProgram\n"));
		GDMultiBytePrgm(Addr, Buffer, Length);			
	}
//	else if(SpiFlashModel == SpiFlash_MX)	//MX
//	else	
}


#endif	//#ifdef FUNC_SPI_FLASH_EN

#ifndef __SERIAL_FLASH_H__
#define	__SERIAL_FLASH_H__


#define		FLASH_READ			0x03	//Read
#define		FLASH_FAST_READ		0x0B	//High Speed Read
#define		FLASH_RDID			0x9F	//Read-ID
#define		FLASH_WREN			0x06	//Write-Enable
#define		FLASH_WRDI			0x04	//Write-Disable
#define		FLASH_RDSR			0x05	//Read status register
#define		FLASH_WRSR			0x01	//Write status register
#define		FLASH_SE			0x20	//Erase sector of memory array
#define		FLASH_CE			0xC7	//Chip-Erase erase full Memory Array 
#define		FLASH_BE			0xD8	//Block-Erase Memory Array. for sst use this instrction can erase 64Kbyte block
//sst dosen't have these three instructions.
#define		FLASH_DP			0xB9	//Deep power down
#define		FLASH_RDP			0xAB	//Release deep power down
#define		FLASH_PP			0x02	//To program one page

//MX and Winbond have the same instruction. This instruction just MX and Winbond have
#define		FLASH_FAST_DUAL_READ	0x3b	//Fast read dual output

//These instructions just sst have.
#define		FLASH_BP				0x02	//To program one data byte
#define		FLASH_BE32K				0x52	//Erase 32 KByte block of memory array
#define		FLASH_AAI				0xAD	//Auto Address increment programming
#define		FLASH_EWSR				0x50	//Enable writer state register

#define	MX_PP_LENGTH	256	

#define 	DMA_XBUF_START			PLAY_BUFFER_START		


typedef struct	_SERIAL_FLASH_ID
{
	BYTE	Mid;		//Manufacturer¡¯s ID
	WORD	Did;		//Device ID
} SERIAL_FLASH_ID;

// serial flash manufacturer's ID and device ID structure
typedef enum _SPI_FLASH_MODEL{
	SpiFlash_None=0,
	SpiFlash_GD,
	SpiFlash_MX,
	SpiFlash_SST,
	SpiFlash_WINBOND		
} SPI_FLASH_MODEL;
	
extern SPI_FLASH_MODEL	SpiFlashModel;


VOID SpiFlashInit(VOID);

VOID SpiFlashBlkRd(DWORD addr_start, BYTE *buffer, WORD	data_length);

VOID SpiFlashErase(VOID);

VOID SPIFlashMultiBytePrgm(DWORD Addr, BYTE *Buffer, WORD Lengh);

#endif


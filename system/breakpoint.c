#include "syscfg.h"
#include "playctrl.h"
#include "breakpoint.h"
#include "sysctrl.h"
#include "debug.h"
#include "24cxx.h"
#include "dir.h"
#include "nvm.h"
#include "user_interface.h"


#ifdef FUNC_BREAK_POINT_EN

#define BP_POS_CRC8CHECKCODE	(sizeof(BP_POWERMEM) - 1)

BYTE 	gFileNameCrc8 = 0;
WORD 	gCurFileNum = 0;
WORD	gCurFileNumInFolder = 0;
WORD 	gCurFolderNum = 0;
BOOL 	gIsFindLastestSong = FALSE;
BOOL	gIsEEPROMExist	= FALSE;
BP_INFO	gBreakPointInfo _at_ 0x06D9;


// Save the break point information.
extern BYTE gWakeupTrigFlg;
extern PLAY_CTRL gPlayCtrl;
extern VOID SetRtcAlarmValid(VOID);

#ifdef FUNC_UARTDBG_EN
VOID BP_InfoDisplay(VOID)
{
	DBG(("**********************************\n"));
	DBG(("*  Signature:%X\n", (WORD)gBreakPointInfo.PowerMemory.Signature));		
	DBG(("*\n"));
	
	DBG(("*  USB_FolderEnFlag:%d\n", (WORD)gBreakPointInfo.PowerMemory.USB_FolderEnFlag));		
	DBG(("*  USB_FileAddress:%lu\n", (DWORD)gBreakPointInfo.PowerMemory.USB_FileAddress));
	DBG(("*  USB_FileNameCRC8:0x%bx\n", gBreakPointInfo.PowerMemory.USB_FileNameCRC8));
	DBG(("*  USB_PlaySeconds:%lu\n", (DWORD)gBreakPointInfo.PowerMemory.USB_CurPlayTime));


	DBG(("*  SD_FolderEnFlag:%d\n", (WORD)gBreakPointInfo.PowerMemory.SD_FolderEnFlag));		
	DBG(("*  SD_FileAddress:%lu\n", (DWORD)gBreakPointInfo.PowerMemory.SD_FileAddress));
	DBG(("*  SD_FileNameCRC8:0x%bx\n", gBreakPointInfo.PowerMemory.SD_FileNameCRC8));
	DBG(("*  SD_PlaySeconds:%lu\n", (DWORD)gBreakPointInfo.PowerMemory.SD_CurPlayTime));
	DBG(("*\n"));	

	DBG(("*\n"));	
	DBG(("*  SystemMode:%bu\n", gBreakPointInfo.PowerMemory.SystemMode));
	DBG(("*\n"));	
	DBG(("*  Volume:%bd(Flag:0x%bx)\n", (gBreakPointInfo.PowerMemory.Volume&MASK_BP_VOLUME), (gBreakPointInfo.PowerMemory.Volume & MASK_BP_VOLFLAG)));
	//DBG(("*  Eq:%d\n", (WORD)gBreakPointInfo.Eq));
	//DBG(("*  Mode:%d\n", (WORD)gBreakPointInfo.RepeatMode));
#ifdef FUNC_RADIO_EN	
	DBG(("*\n"));
	DBG(("*  BandXtalSel:%bu\n", gBreakPointInfo.PowerMemory.RadioData2Store.BandXtalSel));//基本配置便于扩充       	
	DBG(("*  CurrFreq:%u\n",(WORD)gBreakPointInfo.PowerMemory.RadioData2Store.CurrFreq));//当前频率
	DBG(("*  CurrFreq:%u\n",(WORD)gBreakPointInfo.PowerMemory.RadioData2Store.CurrStation));//当前台号// Start from 1,0表示没有台
	DBG(("*  CurrFreq:%u\n",(WORD)gBreakPointInfo.PowerMemory.RadioData2Store.Area1StationSum));//760~874区间的频道数	
	DBG(("*  CurrFreq:%u\n",(WORD)gBreakPointInfo.PowerMemory.RadioData2Store.StationSum));//总的有效台数目(760~874 + 875~1080)// Start from 1,0表示没有台
#endif

	DBG(("*  CRC8:%bu\n",gBreakPointInfo.PowerMemory.EepRomCRC8));
	DBG(("**********************************\n"));
}
#endif


#if (FUNC_RESTORE_DEVICE_SELECE == FUNC_RESTORE_DEVICE_EEPROM)
VOID IsEepromValid(BYTE CheckMod)
{
	if(CheckMod == EEPROM_CHECK_WRITE_READ)
	{
		EEPROM_WriteByte(EEPROM_ADDR_BP_SIGNATURE, BP_SIGNATURE);	
	}

	if(EEPROM_ReadByte(EEPROM_ADDR_BP_SIGNATURE) != BP_SIGNATURE)
	{
		gIsEEPROMExist = FALSE;
	}
	else
	{
		gIsEEPROMExist = TRUE;
	}
}
#endif


BOOL IsBPValid(VOID)
{
	if(gBreakPointInfo.PowerMemory.Signature == BP_SIGNATURE)
	{
		return TRUE;
	}
	return FALSE;	
}


// 在BP_LoadInfo()函数中调用一次
// 对文件夹进行非法值纠正并存入记忆单元
// 需要
VOID CheckBPDataValid(VOID) 
{
	BYTE Temp;

#if (FUNC_RESTORE_DEVICE_SELECE == FUNC_RESTORE_DEVICE_EEPROM)
	Temp = EEPROM_ReadByte((BYTE*)(&gBreakPointInfo.PowerMemory.SD_FolderEnFlag) - (BYTE*)(&gBreakPointInfo.PowerMemory));
#elif (FUNC_RESTORE_DEVICE_SELECE == FUNC_RESTORE_DEVICE_NVM)
	Temp = NvmReadOneByte((BYTE*)(&gBreakPointInfo.PowerMemory.SD_FolderEnFlag) - (BYTE*)(&gBreakPointInfo.PowerMemory));
#endif

	if(Temp > 1)
	{
#ifdef FUNC_FOLDER_EN
		gBreakPointInfo.PowerMemory.SD_FolderEnFlag = TRUE;
#else
		gBreakPointInfo.PowerMemory.SD_FolderEnFlag = FALSE;
#endif
		BP_SaveInfo((BYTE*)(&gBreakPointInfo.PowerMemory.SD_FolderEnFlag), sizeof(gBreakPointInfo.PowerMemory.SD_FolderEnFlag));
	}

#if (FUNC_RESTORE_DEVICE_SELECE == FUNC_RESTORE_DEVICE_EEPROM)
	Temp = EEPROM_ReadByte((BYTE*)(&gBreakPointInfo.PowerMemory.USB_FolderEnFlag) - (BYTE*)(&gBreakPointInfo.PowerMemory));
#elif (FUNC_RESTORE_DEVICE_SELECE == FUNC_RESTORE_DEVICE_NVM)
	Temp = NvmReadOneByte((BYTE*)(&gBreakPointInfo.PowerMemory.USB_FolderEnFlag) - (BYTE*)(&gBreakPointInfo.PowerMemory));
#endif

	if(Temp > 1)
	{
#ifdef FUNC_FOLDER_EN
		gBreakPointInfo.PowerMemory.USB_FolderEnFlag = TRUE;
#else
		gBreakPointInfo.PowerMemory.USB_FolderEnFlag = FALSE;
#endif
		BP_SaveInfo((BYTE*)(&gBreakPointInfo.PowerMemory.USB_FolderEnFlag), sizeof(gBreakPointInfo.PowerMemory.USB_FolderEnFlag));
	}

#if (FUNC_RESTORE_DEVICE_SELECE == FUNC_RESTORE_DEVICE_EEPROM)
	Temp = EEPROM_ReadByte((BYTE*)(&gBreakPointInfo.PowerMemory.Volume) - (BYTE*)(&gBreakPointInfo.PowerMemory));
#elif (FUNC_RESTORE_DEVICE_SELECE == FUNC_RESTORE_DEVICE_NVM)
	Temp = NvmReadOneByte((BYTE*)(&gBreakPointInfo.PowerMemory.Volume) - (BYTE*)(&gBreakPointInfo.PowerMemory));
#endif

	if(Temp != gSys.Volume)
	{
		BP_SaveInfo((BYTE*)(&gBreakPointInfo.PowerMemory.Volume), sizeof(gBreakPointInfo.PowerMemory.Volume));
	}		
}

// Load break point information from EEPROM
// 该函数只在上电时执行一次
VOID BP_LoadInfo(VOID)
{			
	BYTE TempCRC8;

	//DBG((">>BP_LoadInfo()\n"));

//#ifdef FUNC_UARTDBG_EN
//	DBG(("********BP_LoadInfo,000********\n"));
//	BP_InfoDisplay();
//#endif

	TempCRC8 = CRC8Cal(((BYTE*)&gBreakPointInfo.PowerMemory), sizeof(BP_POWERMEM) - 1);
	//DBG(("Signature:0x%bx, EepRomCRC8:0x%bx, tempCRC8:0x%bx\n", gBreakPointInfo.PowerMemory.Signature, gBreakPointInfo.PowerMemory.EepRomCRC8, TempCRC8));

	if((!(gWakeupTrigFlg & SYS_ON_FLG_POR)) && (gBreakPointInfo.PowerMemory.Signature == BP_SIGNATURE) && (TempCRC8 == gBreakPointInfo.PowerMemory.EepRomCRC8))
	{
		//RAM中有数据，无需从EEPROM读取。
		DBG(("RAM data valid\n"));
		//DBG(("BP_LoadInfo,Valid!USB_CurPlayTime:%u, SD_CurPlayTime:%u.\n", gBreakPointInfo.PowerMemory.USB_CurPlayTime, gBreakPointInfo.PowerMemory.SD_CurPlayTime));
		gWakeupTrigFlg = 0;
		//DBG(("BP_WakeUp!\n"));
		gSys.SystemMode = gBreakPointInfo.PowerMemory.SystemMode;
		gSys.Volume = gBreakPointInfo.PowerMemory.Volume & MASK_BP_VOLUME;
		DBG(("gSys.Volume11111111111111 = %x\n",gSys.Volume));	
DBG(("gBreakPointInfo.PowerMemory.Volume11 = %x\n",gBreakPointInfo.PowerMemory.Volume));	

		if(gSys.Volume > VOLUME_MAX) 
		{
			gSys.Volume = VOLUME_INIT;
		}	
		
		if(gBreakPointInfo.Eq >= DECD_EQ_SUM)
		{
			gBreakPointInfo.Eq = DECD_EQ_NORMAL;
		}
		
		if(gBreakPointInfo.RepeatMode >= REPEAT_MODE_SUM)
		{
			gBreakPointInfo.RepeatMode = REPEAT_ALL;
		}
		
		gPlayCtrl.Eq = gBreakPointInfo.Eq;
		gPlayCtrl.RepeatMode = gBreakPointInfo.RepeatMode;
		
		if(gSys.SystemMode == SYS_MODE_USB)
		{
			gFileNameCrc8 = gBreakPointInfo.PowerMemory.USB_FileNameCRC8;
			gPlayCtrl.File.StartSec = gBreakPointInfo.PowerMemory.USB_FileAddress;//文件地址
			gSongInfo.CurPlayTime = gBreakPointInfo.PowerMemory.USB_CurPlayTime;
#ifdef FUNC_FOLDER_EN
			gPlayCtrl.FolderEnable = gBreakPointInfo.PowerMemory.USB_FolderEnFlag;
#else
			gPlayCtrl.FolderEnable = FALSE;
#endif
		}
		else if(gSys.SystemMode == SYS_MODE_SD)
		{
			gFileNameCrc8 = gBreakPointInfo.PowerMemory.SD_FileNameCRC8;
			gPlayCtrl.File.StartSec = gBreakPointInfo.PowerMemory.SD_FileAddress;
			gSongInfo.CurPlayTime = gBreakPointInfo.PowerMemory.SD_CurPlayTime;
#ifdef FUNC_FOLDER_EN
			gPlayCtrl.FolderEnable = gBreakPointInfo.PowerMemory.SD_FolderEnFlag;
#else
			gPlayCtrl.FolderEnable = FALSE;
#endif
		}
		if(gPlayCtrl.FolderEnable > 1)
		{
			gPlayCtrl.FolderEnable = FALSE;
		}

		//DBG(("SysMode:%bd, Volume:%bd, Eq:%bd, RepeatMode:%bd, FolderEnable:%bd, CurPlayTime:%ld\n", gSys.SystemMode, gSys.Volume, gPlayCtrl.Eq, gPlayCtrl.RepeatMode, gPlayCtrl.FolderEnable, gSongInfo.CurPlayTime));
		return;	
	}

	memset(&gBreakPointInfo, 0, sizeof(gBreakPointInfo));	
#if (FUNC_RESTORE_DEVICE_SELECE == FUNC_RESTORE_DEVICE_EEPROM)
	EEPROM_ReadBytes(0, (BYTE XDATA*)&gBreakPointInfo.PowerMemory, sizeof(BP_POWERMEM));
#elif (FUNC_RESTORE_DEVICE_SELECE == FUNC_RESTORE_DEVICE_NVM)
	NvmReadMulti(0, (BYTE XDATA*)&gBreakPointInfo.PowerMemory, sizeof(BP_POWERMEM));
#endif

#ifdef FUNC_UARTDBG_EN
	DBG(("********BP_LoadInfo,001********\n"));
	BP_InfoDisplay();
#endif
	
#ifdef FUNC_SAVE_PLAYTIME_EN
	if(gBreakPointInfo.PowerMemory.Signature != BP_SIGNATURE)
#else
	TempCRC8 = CRC8Cal(((BYTE*)&gBreakPointInfo.PowerMemory), sizeof(BP_POWERMEM) - 1);
DBG(("gBreakPointInfo.PowerMemory.Signature = %x\n",gBreakPointInfo.PowerMemory.Signature));	
DBG(("gBreakPointInfo.PowerMemory.EepRomCRC8 = %x\n",gBreakPointInfo.PowerMemory.EepRomCRC8));	
DBG(("TempCRC8 = %x\n",TempCRC8));	

	if((gBreakPointInfo.PowerMemory.Signature != BP_SIGNATURE)
	|| (gBreakPointInfo.PowerMemory.EepRomCRC8 != TempCRC8))//数据无效！
#endif
	{	
#if (FUNC_RESTORE_DEVICE_SELECE == FUNC_RESTORE_DEVICE_EEPROM)
		IsEepromValid(EEPROM_CHECK_WRITE_READ);
		if(gIsEEPROMExist == FALSE)//EEPROM不存在或坏掉了
		{
			gSys.SystemMode = SYS_MODE_NONE;
			gSys.Volume = VOLUME_INIT;
			//gPlayCtrl.Eq = DECD_EQ_NORMAL;
			//gPlayCtrl.RepeatMode = REPEAT_ALL;
#ifdef FUNC_FOLDER_EN
			gPlayCtrl.FolderEnable = TRUE;
#else
			gPlayCtrl.FolderEnable = FALSE;
#endif

#ifdef FUNC_UARTDBG_EN
			DBG(("00,EEPROM not exist!\n"));
#endif		
		}	
		else//只是数据无效
#elif (FUNC_RESTORE_DEVICE_SELECE == FUNC_RESTORE_DEVICE_NVM)
#endif		
		{	
			DBG(("Break point information is invalid!\n"));
			gIsEEPROMExist = TRUE;
			//赋初值
			//memset(&gBreakPointInfo, 0, sizeof(BP_INFO));
			
			gBreakPointInfo.Eq = DECD_EQ_NORMAL;
			gBreakPointInfo.RepeatMode = REPEAT_ALL;
			gBreakPointInfo.PowerMemory.Signature = BP_SIGNATURE;
			gBreakPointInfo.PowerMemory.SystemMode = SYS_MODE_USB;
#ifdef FUNC_FOLDER_EN
			gBreakPointInfo.PowerMemory.USB_FolderEnFlag = TRUE;
			gBreakPointInfo.PowerMemory.SD_FolderEnFlag = TRUE;
#else
			gBreakPointInfo.PowerMemory.USB_FolderEnFlag = FALSE;
			gBreakPointInfo.PowerMemory.SD_FolderEnFlag = FALSE;
#endif
		
			gSys.SystemMode = SYS_MODE_NONE;
			gSys.Volume = VOLUME_INIT;

#if (FUNC_RESTORE_DEVICE_SELECE == FUNC_RESTORE_DEVICE_EEPROM)
			gBreakPointInfo.PowerMemory.USB_CurPlayTime = 0;
			gBreakPointInfo.PowerMemory.SD_CurPlayTime = 0;
#elif(FUNC_RESTORE_DEVICE_SELECE == FUNC_RESTORE_DEVICE_NVM)
#endif

//#ifdef FUNC_UARTDBG_EN
//			DBG(("********BP_LoadInfo,002********\n"));
//			BP_InfoDisplay();
//#endif	
		}			
	}
	else	//数据有效！
	{
		DBG(("Break point information is valid!\n"));
		gIsEEPROMExist = TRUE;

#ifdef	FUNC_ALARM_EN
#if (FUNC_RESTORE_DEVICE_SELECE == FUNC_RESTORE_DEVICE_EEPROM)
		SetRtcAlarmValid(); //RTC闹钟有效处理
#endif
#endif
		//优先选择原来的设备模式
		gSys.SystemMode = BP_GetSystemMode();
		gSys.Volume = gBreakPointInfo.PowerMemory.Volume & MASK_BP_VOLUME;	
DBG(("gBreakPointInfo.PowerMemory.Volume = %x\n",gBreakPointInfo.PowerMemory.Volume));	
DBG(("gSys.Volume = %x\n",gSys.Volume));	

		if(gSys.Volume > VOLUME_MAX) 
		{
			gSys.Volume = VOLUME_INIT;
			//DBG(("Volume valid(%bd)!\n", gBreakPointInfo.PowerMemory.Volume));	
		}
#if 0 //ndef FUNC_RESTORE_VOLUME
		if((gSys.Volume > 28) || (gSys.Volume < 10))
		{
			gSys.Volume = VOLUME_INIT;
			//DBG(("Volume valid(%bd)!\n", gBreakPointInfo.PowerMemory.Volume));	
		}
#endif
		DBG(("Volume valid(%bu)!\n", gBreakPointInfo.PowerMemory.Volume));
		if(gBreakPointInfo.Eq >= DECD_EQ_SUM)
		{
			gBreakPointInfo.Eq = DECD_EQ_NORMAL;
		}
		if(gBreakPointInfo.RepeatMode >= REPEAT_MODE_SUM)
		{
			gBreakPointInfo.RepeatMode = REPEAT_ALL;
		}

#ifdef FUNC_FOLDER_EN
		if(gBreakPointInfo.PowerMemory.USB_FolderEnFlag > 1)
		{
			gBreakPointInfo.PowerMemory.USB_FolderEnFlag = TRUE;
		}
		if(gBreakPointInfo.PowerMemory.SD_FolderEnFlag > 1)
		{
			gBreakPointInfo.PowerMemory.SD_FolderEnFlag = TRUE;
		}
#else
		gBreakPointInfo.PowerMemory.USB_FolderEnFlag = FALSE;
		gBreakPointInfo.PowerMemory.SD_FolderEnFlag = FALSE;
#endif

		if(gSys.SystemMode == SYS_MODE_USB)
		{
			gPlayCtrl.FolderEnable = gBreakPointInfo.PowerMemory.USB_FolderEnFlag;
			gFileNameCrc8 = gBreakPointInfo.PowerMemory.USB_FileNameCRC8;
			gPlayCtrl.File.StartSec = gBreakPointInfo.PowerMemory.USB_FileAddress;//文件地址
		}
		else if(gSys.SystemMode == SYS_MODE_SD)
		{
			gPlayCtrl.FolderEnable = gBreakPointInfo.PowerMemory.SD_FolderEnFlag;
			gFileNameCrc8 = gBreakPointInfo.PowerMemory.SD_FileNameCRC8;
			gPlayCtrl.File.StartSec = gBreakPointInfo.PowerMemory.SD_FileAddress;
		}

		gPlayCtrl.Eq = gBreakPointInfo.Eq;
		gPlayCtrl.RepeatMode = gBreakPointInfo.RepeatMode;

#if (FUNC_RESTORE_DEVICE_SELECE == FUNC_RESTORE_DEVICE_EEPROM)
#ifdef FUNC_SAVE_PLAYTIME_EN
		if(!gSavePlayTimeEnFlag)
		{          	       
			if(gBreakPointInfo.PowerMemory.USB_CurPlayTime)
			{
				gBreakPointInfo.PowerMemory.USB_CurPlayTime = 0;
				DBG(("BP_SavePlayTime USB_CurPlayTime = 0\n"));	
				BP_SavePlayTime((BYTE*)(&gBreakPointInfo.PowerMemory.USB_CurPlayTime), sizeof(gBreakPointInfo.PowerMemory.USB_CurPlayTime));	
			}
			
			if(gBreakPointInfo.PowerMemory.SD_CurPlayTime)
			{
				gBreakPointInfo.PowerMemory.SD_CurPlayTime = 0;
				DBG(("BP_SavePlayTime SD_CurPlayTime = 0\n"));	
				BP_SavePlayTime((BYTE*)(&gBreakPointInfo.PowerMemory.SD_CurPlayTime), sizeof(gBreakPointInfo.PowerMemory.SD_CurPlayTime));
			}
		}	
#else
		gBreakPointInfo.PowerMemory.USB_CurPlayTime = 0;
		gBreakPointInfo.PowerMemory.SD_CurPlayTime = 0;
#endif

#elif(FUNC_RESTORE_DEVICE_SELECE == FUNC_RESTORE_DEVICE_NVM)
#endif
			
//#ifdef FUNC_UARTDBG_EN
//		DBG(("********BP_LoadInfo,003********\n"));
//		BP_InfoDisplay();
//#endif	
	}
	CheckBPDataValid();
}


// Save break point information to EEPROM
VOID BP_SaveInfo(BYTE* Data, BYTE Size)
{
//	DBG((">>BP_SaveInfo()\n"));
	//不存入E2P
	gBreakPointInfo.Eq = gPlayCtrl.Eq;
	gBreakPointInfo.RepeatMode = gPlayCtrl.RepeatMode;

	if(gBreakPointInfo.Eq >= DECD_EQ_SUM)
	{
		gBreakPointInfo.Eq = DECD_EQ_NORMAL;
	}

	if(gBreakPointInfo.RepeatMode >= REPEAT_MODE_SUM)
	{
		gBreakPointInfo.RepeatMode = REPEAT_ALL;
	}	

	gBreakPointInfo.PowerMemory.Signature = BP_SIGNATURE;

	if(gSys.SystemMode != SYS_MODE_NONE)
	{
		gBreakPointInfo.PowerMemory.SystemMode = gSys.SystemMode;
	}
	
	gBreakPointInfo.PowerMemory.Volume = gSys.Volume;
#ifdef FUNC_RESTORE_VOLUME
	gBreakPointInfo.PowerMemory.Volume |= MASK_BP_VOLFLAG;
#endif

	if(gPlayCtrl.FolderEnable > 1)
	{
#ifdef FUNC_FOLDER_EN
		gPlayCtrl.FolderEnable = TRUE;
#else
		gPlayCtrl.FolderEnable = FALSE;
#endif
	}
	
	if(gSys.SystemMode == SYS_MODE_USB)
	{
		gBreakPointInfo.PowerMemory.USB_FolderEnFlag = gPlayCtrl.FolderEnable;
		gBreakPointInfo.PowerMemory.USB_FileNameCRC8 = gFileNameCrc8;
		gBreakPointInfo.PowerMemory.USB_FileAddress = gPlayCtrl.File.StartSec;//文件地址

#if (FUNC_RESTORE_DEVICE_SELECE == FUNC_RESTORE_DEVICE_EEPROM)
		gSongInfo.CurPlayTime = gBreakPointInfo.PowerMemory.USB_CurPlayTime;
		gBreakPointInfo.PowerMemory.USB_CurPlayTime = 0;
#endif
	}
	else if(gSys.SystemMode == SYS_MODE_SD)
	{
		gBreakPointInfo.PowerMemory.SD_FolderEnFlag = gPlayCtrl.FolderEnable;
		gBreakPointInfo.PowerMemory.SD_FileNameCRC8 = gFileNameCrc8;
		gBreakPointInfo.PowerMemory.SD_FileAddress = gPlayCtrl.File.StartSec;//DirGetSonClusNum();//文件地址

#if (FUNC_RESTORE_DEVICE_SELECE == FUNC_RESTORE_DEVICE_EEPROM)		
		gSongInfo.CurPlayTime = gBreakPointInfo.PowerMemory.SD_CurPlayTime;
		gBreakPointInfo.PowerMemory.SD_CurPlayTime = 0;
#endif
	}

	//对E2PROM 内容做CRC8 校验
	gBreakPointInfo.PowerMemory.EepRomCRC8 = CRC8Cal(((BYTE*)&gBreakPointInfo.PowerMemory), sizeof(BP_POWERMEM) - 1);

	if((Data != NULL) && (Size != 0))
	{		
#if (FUNC_RESTORE_DEVICE_SELECE == FUNC_RESTORE_DEVICE_EEPROM)
		if(gIsEEPROMExist == TRUE)
		{		
#if (I2C_PIN_CONFIGURE == SDIO_TO_A3A4A5)
#if defined(SD_DETECT_PIN_USE_A4)
			SetGpioRegBit(CARD_DETECT_PORT_PD, CARD_DETECT_BIT);
#endif
			SET_CARD_NOT_TO_GPIO(); //add for I2C clk & data multiplexed with SDIO clk & data
#endif		
			EEPROM_WriteByte(0, BP_SIGNATURE);
			EEPROM_WriteBytes(Data - (BYTE*)(&gBreakPointInfo.PowerMemory), Data, Size);			
			EEPROM_WriteByte(BP_POS_CRC8CHECKCODE, gBreakPointInfo.PowerMemory.EepRomCRC8);
			
			IsEepromValid(EEPROM_CHECK_READ);
#ifdef FUNC_UARTDBG_EN
			if(gIsEEPROMExist == FALSE)
			{
				DBG(("11,EEPROM not exist!\n"));
			}
#endif

#if (I2C_PIN_CONFIGURE == SDIO_TO_A3A4A5)		
			SET_CARD_TO_A3A4A5(); //add for I2C clk & data multiplexed with SDIO clk & data
#if defined(SD_DETECT_PIN_USE_A4)
			ClrGpioRegBit(CARD_DETECT_PORT_PD, CARD_DETECT_BIT);
#endif
#endif
		}
		
#elif (FUNC_RESTORE_DEVICE_SELECE == FUNC_RESTORE_DEVICE_NVM)
		NvmWriteOneByte(0, BP_SIGNATURE);
		NvmWriteMulti(Data - (BYTE*)(&gBreakPointInfo.PowerMemory), Data, Size);
		NvmWriteOneByte(BP_POS_CRC8CHECKCODE, gBreakPointInfo.PowerMemory.EepRomCRC8);
#endif
	}

#if (FUNC_RESTORE_DEVICE_SELECE == FUNC_RESTORE_DEVICE_EEPROM)
	if(gSys.SystemMode == SYS_MODE_USB)
	{
		gBreakPointInfo.PowerMemory.USB_CurPlayTime = gSongInfo.CurPlayTime;
	}
	else if(gSys.SystemMode == SYS_MODE_SD)
	{	
		gBreakPointInfo.PowerMemory.SD_CurPlayTime = gSongInfo.CurPlayTime;
	}
#endif
}


#ifdef FUNC_SAVE_PLAYTIME_EN
// Save break point information to EEPROM
static DWORD sSavePlayTimeBackup = 0;

VOID BP_SavePlayTime(BYTE* Data, BYTE Size)
{
	if((Data != NULL) && (Size != 0))
	{		
#if (FUNC_RESTORE_DEVICE_SELECE == FUNC_RESTORE_DEVICE_EEPROM)
		if(gIsEEPROMExist == TRUE)
		{		
#if (I2C_PIN_CONFIGURE == SDIO_TO_A3A4A5)
			SetGpioRegBit(CARD_DETECT_PORT_PD, CARD_DETECT_BIT);
			SetGpioRegBit(GPIO_CFG1_SEL, MASK_SD_NOT_TO_GPIO);	//add for I2C clk & data multiplexed with SDIO clk & data 
#endif		

			EEPROM_WriteBytes(Data - (BYTE*)(&gBreakPointInfo.PowerMemory), Data, Size);			

#if (I2C_PIN_CONFIGURE == SDIO_TO_A3A4A5)		
			ClrGpioRegBit(GPIO_CFG1_SEL, MASK_SD_NOT_TO_GPIO);	//add for I2C clk & data multiplexed with SDIO clk & data
			ClrGpioRegBit(CARD_DETECT_PORT_PD, CARD_DETECT_BIT);
#endif
		}
		
#endif
	}
}


VOID BP_UpdatePlayTime(VOID)
{
	if(gSys.SystemMode == SYS_MODE_USB)	
	{	
		if(gBreakPointInfo.PowerMemory.USB_CurPlayTime != sSavePlayTimeBackup)
		{
			sSavePlayTimeBackup = gBreakPointInfo.PowerMemory.USB_CurPlayTime;
			BP_SavePlayTime((BYTE*)(&gBreakPointInfo.PowerMemory.USB_CurPlayTime), sizeof(gBreakPointInfo.PowerMemory.USB_CurPlayTime));
		}
	}
	else if(gSys.SystemMode == SYS_MODE_SD)
	{
		if(gBreakPointInfo.PowerMemory.SD_CurPlayTime != sSavePlayTimeBackup)
		{
			sSavePlayTimeBackup = gBreakPointInfo.PowerMemory.SD_CurPlayTime;
			BP_SavePlayTime((BYTE*)(&gBreakPointInfo.PowerMemory.SD_CurPlayTime), sizeof(gBreakPointInfo.PowerMemory.SD_CurPlayTime));
		}
	}
}
#endif


VOID BP_GetPlayAttrib(VOID)
{
	//DBG(("*****BP_GetPlayAttrib()*****\n"));
	if(gSys.SystemMode == SYS_MODE_USB)			//USB
	{
		gSongInfo.CurPlayTime = gBreakPointInfo.PowerMemory.USB_CurPlayTime;		
	}
	else if(gSys.SystemMode == SYS_MODE_SD)		//SD 
	{		
		gSongInfo.CurPlayTime = gBreakPointInfo.PowerMemory.SD_CurPlayTime;;	
	}
	gPlayCtrl.Eq = gBreakPointInfo.Eq;
	gPlayCtrl.RepeatMode = gBreakPointInfo.RepeatMode;
}


VOID BP_SetPlayAttrib(VOID)
{
	//DBG(("*****BP_SetPlayAttrib()*****\n"));
	if(gSys.SystemMode == SYS_MODE_USB)			//USB
	{
		gBreakPointInfo.PowerMemory.USB_CurPlayTime = gSongInfo.CurPlayTime;
	}
	else if(gSys.SystemMode == SYS_MODE_SD)		//SD 
	{			
		gBreakPointInfo.PowerMemory.SD_CurPlayTime = gSongInfo.CurPlayTime;
	}
	gBreakPointInfo.Eq = gPlayCtrl.Eq;
	gBreakPointInfo.RepeatMode = gPlayCtrl.RepeatMode;

#if (FUNC_RESTORE_DEVICE_SELECE == FUNC_RESTORE_DEVICE_EEPROM)
//EEPROM寿命有限不保存歌曲即时播放时间
//需要调整下校验值以便正常识别sleep模式
	gBreakPointInfo.PowerMemory.EepRomCRC8 = CRC8Cal(((BYTE*)&gBreakPointInfo.PowerMemory), sizeof(BP_POWERMEM) - 1);
#elif(FUNC_RESTORE_DEVICE_SELECE == FUNC_RESTORE_DEVICE_NVM)
//NVM保存歌曲即时播放时间
	if(gSys.SystemMode == SYS_MODE_USB)	
	{		
		BP_SaveInfo((BYTE*)(&gBreakPointInfo.PowerMemory.USB_CurPlayTime), sizeof(gBreakPointInfo.PowerMemory.USB_CurPlayTime));
	}
	else if(gSys.SystemMode == SYS_MODE_SD)
	{
		BP_SaveInfo((BYTE*)(&gBreakPointInfo.PowerMemory.SD_CurPlayTime), sizeof(gBreakPointInfo.PowerMemory.SD_CurPlayTime));
	}
#endif
}


BYTE BP_GetSystemMode(VOID)
{
	return gBreakPointInfo.PowerMemory.SystemMode;
}


BYTE CRC8Cal(BYTE* Ptr, BYTE Len)
{
	BYTE Crc = 0;
 	BYTE i;

	while(Len--)
	{
		Crc ^= *Ptr++;
		for(i = 0; i < 8; i++)
		{
			if(Crc & 0x01)
			{
				Crc = (Crc >> 1) ^ 0x8C;
			}
			else
			{
				Crc >>= 1;
			}
		}
	}
    return Crc;
}


VOID FindLatestSong(VOID)
{
	//匹配记录歌曲相关信息，寻找断电记忆歌曲	
	if(gIsFindLastestSong == FALSE)
	{
		if(gFsInfo.DevID == DEV_ID_USB)
		{
#ifdef FUNC_SAVE_PLAYTIME_EN
			if(gBreakPointInfo.PowerMemory.USB_FileAddress == GetSecNum(DirGetSonClusNum()))			
#else		
			if((gBreakPointInfo.PowerMemory.USB_FileAddress == GetSecNum(DirGetSonClusNum()))			
			&& (gBreakPointInfo.PowerMemory.USB_FileNameCRC8 == (CRC8Cal(gCurrentEntry->FileName, 11))))
#endif
			{
				DBG(("!usb find song\n"));
				//表示找到所要的歌曲
				gIsFindLastestSong = TRUE;
			}
		}
		else if(gFsInfo.DevID == DEV_ID_SD)
		{
#ifdef FUNC_SAVE_PLAYTIME_EN
			if(gBreakPointInfo.PowerMemory.SD_FileAddress == GetSecNum(DirGetSonClusNum()))			
#else				
			if((gBreakPointInfo.PowerMemory.SD_FileAddress == GetSecNum(DirGetSonClusNum()))
			&& (gBreakPointInfo.PowerMemory.SD_FileNameCRC8 == (CRC8Cal(gCurrentEntry->FileName, 11))))
#endif			
			{
				DBG(("!sd find song\n"));
				//表示找到所要的歌曲
				gIsFindLastestSong = TRUE;
			}	
		}
	}	
}


#endif

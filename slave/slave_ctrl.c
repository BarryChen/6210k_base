/*
*****************************************************************************
*					Mountain View Silicon Tech. Inc.
*	Copyright 2007, Mountain View Silicon Tech. Inc., ShangHai, China
*					All rights reserved.
*
* Filename:			uart_event.c
* Description:		sysctrl control c file
*
*write by			richard					
******************************************************************************
*/
#include <reg51.h>										//include public header files
#include "type.h"
#include "syscfg.h"
#include "key.h"
#include "sysctrl.h"
#include "device.h"
#include "debug.h"
#include "message.h"
#include "slave_cmd.h"
#include "slave_ctrl.h"
#include "devctrl.h"
#include "rtc_ctrl.h"
#include "rtc.h"
#include "playctrl.h"
#include "breakpoint.h"
#include "user_interface.h"
#include "pwm.h"
#include "power.h"
#include "radio_ctrl.h"
#include "radio_app_interface.h"

#if 0//def FUNC_SLAVE_UART_EN

#define SLAVE_NEXT_MODE				0X0101
#define SLAVE_SELECT_MODE			0X0102
#define SLAVE_GET_MODE              0X0103
#define SLAVE_GET_DEVICE_LINK       0X0104
#define SLAVE_MCU_UPDATE       		0X0105
#define SLAVE_GET_BREAKPOINT_INFO  	0X0106
#define SLAVE_GET_CHECKSUM_INFO  	0X0107

#define SLAVE_GET_VALID_FOLDER_SUM  0X0201
#define SLAVE_GET_FILE_SUM          0X0202
#define SLAVE_GET_FOLDER_INFO       0X0203
#define SLAVE_GET_FILE_INFO         0X0204
#define SLAVE_PLAY                  0X0301
#define SLAVE_PAUSE                 0X0302
#define SLAVE_STOP                  0X0303
#define SLAVE_NEXT                  0X0304
#define SLAVE_PRE                   0X0305
#define SLAVE_PLAY_PAUSE            0X0306
#define SLAVE_FF                    0X0307
#define SLAVE_FB                    0X0308
#define SLAVE_STOP_FF_FB            0X0309
#define SLAVE_SEEK_PLAY_TIME        0X030A
#define SLAVE_NEXT_FOLDER        	0X030B
#define SLAVE_PRE_FOLDER        	0X030C
#define SLAVE_SELECT_SONG        	0X030D
#define SLAVE_GET_PLAY_STATUS       0X0320
#define SLAVE_GET_SONG_INFO         0X0321
#define SLAVE_SET_REPEAT_MODE       0X0322
#define SLAVE_GET_REPEAT_MODE       0X0323
#define SLAVE_ENABLE_FOLDER         0X0324
#define SLAVE_DISABLE_FOLDER        0X0325
#define SLAVE_GET_TITLE             0X0330
#define SLAVE_GET_ARTIST            0X0331
#define SLAVE_GET_ALBUM             0X0332
#define SLAVE_GET_COMMENT           0X0333
#define SLAVE_GET_YEAR              0X0334
#define SLAVE_VOLADD                0X0401
#define SLAVE_VOLSUB                0X0402
#define SLAVE_SET_VOL               0X0403
#define SLAVE_GET_VOL               0X0404
#define SLAVE_SET_EQ                0X0405
#define SLAVE_GET_EQ                0X0406
#define SLAVE_MUTE                  0X0407
#define SLAVE_UNMUTE                0X0408
//#define SLAVE_SET_LINEIN_CHANNEL    0X0409
#define SLAVE_SET_LINEIN_GAIN       0X0409
#define SLAVE_ENABLE_MIC            0X040A
#define SLAVE_DISABLE_MIC           0X040B
#define SLAVE_GET_LINEIN_GAIN       0X040C
#define SLAVE_SET_TIME              0X0501
#define SLAVE_GET_TIME              0X0502
#define SLAVE_SET_ALARM_TIME        0X0503
#define SLAVE_GET_ALARM_TIME        0X0504
#define SLAVE_ENABLE_ALARM			0x0505
#define SLAVE_DISABLE_ALARM			0x0506
#define SLAVE_GET_ALARM_STATUS      0X0507
#define SLAVE_GET_ALARM_FLAG		0x0508
#define SLAVE_GET_MONTH_DAYS        0X0510
#define SLAVE_SOLAR_TO_LUNAR        0X0511
#define SLAVE_GET_HEAVENLY_STEM     0X0512
#define SLAVE_GET_EARTHLY_BRANCH    0X0513
#define SLAVE_SET_GPIO_REG          0X0601
#define SLAVE_GET_GPIO_REG          0X0602
#define SLAVE_SET_GPIO_REG_BIT      0X0603
#define SLAVE_CLR_GPIO_REG_BIT      0X0604
#define SLAVE_ENABLE_ADC_CHANNEL    0X0701
#define SLAVE_DISABLE_ADC_CHANNEL   0X0702
#define SLAVE_GET_ADC_CHANNEL_VALUE 0X0703
#define SLAVE_GET_VIN_VOLTAGE       0X0704
#define SLAVE_PWM_CONFIG_CHANNEL    0X0801
#define SLAVE_ENABLE_PWM_CHANNEL    0X0802
#define SLAVE_DISABLE_PWM_CHANNEL   0X0803
#define SLAVE_SET_DRIVE_MODE_CHANNEL   0X0804
#define SLAVE_SET_WAKEUP_MODE       0X0901
#define SLAVE_GOTO_SLEEP            0X0902
#define SLAVE_SET_RADIO_PARAM        0X0A01
#define SLAVE_SET_RADIO_FREQ        0X0A02
#define SLAVE_NEXT_STEP        0X0A03
#define SLAVE_PREV_STEP        0X0A04
#define SLAVE_NEXT_STATION        0X0A05
#define SLAVE_PREV_STATION        0X0A06
#define SLAVE_AUTO_SCAN        0X0A07
#define SLAVE_NEXT_SEEK        0X0A08
#define SLAVE_PREV_SEEK        0X0A09
#define SLAVE_SAVE_STATION        0X0A0A
#define SLAVE_SET_MONO_MODE        0X0A0B
#define SLAVE_GET_CUR_STATUS       0X0A0C
#define SLAVE_GET_STORED_STATION      0X0A0D


//BOOL gIsMcuFileFind;		//for upgrade Master MCU with file "MCU.BIN"
//FILE gMcuUpdataFile;			//for upgrade Master MCU with file "MCU.BIN"
//BYTE gMcuUpdataWaitTime=0;

//BYTE gUserLineInGain=10;
BYTE SuspendTick = 0;
BYTE SlaveRespBuf[251];//251
//DWORD gOldPlayTime=0;
extern VOID PlayVol2Decd();
extern VOID PlayEQ2Decd();
extern VOID SetLineInVolume();
#ifdef FUNC_USB_EN
extern BOOL IsUSBLink(VOID);
#endif
#if 0
VOID ReadFirstBlock()
{
	BYTE i,j;
	WORD	Len;
	DWORD	WriteAddr = 0;
	FILE	file;
    //DBG(("ReadFirstBlock!\n"));
	file = gMcuUpdataFile;

	if(gMcuUpdataFile.Size != 0x10000UL)	//64K
	{
		//DBG(("File size invalid!\n"));
		//return FALSE;
	}

	if((Len = FileRead(&file, gSongInfo.PlayBuffer, 512)) == 0)
	{
		DBG(("Update data,ReadFile Failed!\n"));
		return;
	}
	SlaveSendResp(SLAVE_MCU_UPDATE,(BYTE XDATA*)(gSongInfo.PlayBuffer),64);	
	//DBG(("ReadFirstBlock end!\n"));
}

VOID UpdateMcu()
{
	BYTE i,j;
	WORD	Len;
	DWORD	WriteAddr = 0;
	FILE	file;

	file = gMcuUpdataFile;
    //DBG(("UpdateMcu!\n"));
	if(file.Size != 0x10000UL)	//64K
	{
		//DBG(("File size invalid!\n"));
		//return FALSE;
	}
	while(!FileEOF(&file))
	{
		FeedWatchDog();
		
		if((Len = FileRead(&file, gSongInfo.PlayBuffer, 512)) == 0)
		{
			DBG(("Update data,ReadFile Failed!\n"));
			return;
		}
		for(i=0;i<8;i++)	//Send Data to Master
		{
			SlaveSendResp(SLAVE_MCU_UPDATE,(BYTE XDATA*)(gSongInfo.PlayBuffer+i*64),64);	
			if(gMcuUpdataWaitTime)
			{
				WaitMs(gMcuUpdataWaitTime);
			}			
		}
	}
	//DBG(("update complete!\n"));
}
#endif

VOID SlaveCtrlInit()
{

	SlaveUartInit();
}
BOOL isUSBorSD(void)
{
	if((gSys.SystemMode==SYS_MODE_USB)||(gSys.SystemMode==SYS_MODE_SD))
		return TRUE;
	else
		return FALSE;
}

void SlaveSysModeProcess(WORD cmd)
{
	BYTE temp=0;
	switch(cmd)
	{
#if 0	
		case SLAVE_MCU_UPDATE:
			SuspendTick = 0;
			if(!gIsMcuFileFind)
			{
				SlaveSendResp(SLAVE_MCU_UPDATE,SlaveRespBuf,0);
				return;
			}
			gMcuUpdataWaitTime=gSlaveCmdData[0];
			if(gSlaveCmdData[1]==0x00)
			{
				ReadFirstBlock();
			}
			else
			{
				UpdateMcu();
			}
			//SlaveSendResp(SLAVE_NEXT_MODE,SlaveRespBuf,0);
			break;
			
		case SLAVE_GET_CHECKSUM_INFO:
			
/*otp中相关烧录信息空间说明(0xffe0-0xffff)：
 第1，2字节是低32K的校验值的低16位
第3，4字节是文件的长度
第5，6,7,8字节是总的校验值
第9，10,11,12,13，14字节是小时，日，月，年, 秒，分
第15，16字节是，低32K的校验值的高16位 
第17,18,19,20,21,22字节是烧录器的ID号
第25,26,27,28字节是高32K的校验值
*/			
			SuspendTick = 0;
			//checksum info
			SlaveRespBuf[0]=CBYTE[0xffe4];
			SlaveRespBuf[1]=CBYTE[0xffe5];
			SlaveRespBuf[2]=CBYTE[0xffe6];
			SlaveRespBuf[3]=CBYTE[0xffe7];
			//time info
			SlaveRespBuf[4]=CBYTE[0xffe8];
			SlaveRespBuf[5]=CBYTE[0xffe9];
			SlaveRespBuf[6]=CBYTE[0xffea];
			SlaveRespBuf[7]=CBYTE[0xffeb];
			SlaveRespBuf[8]=CBYTE[0xffec];
			SlaveRespBuf[9]=CBYTE[0xffed];
			SlaveSendResp(SLAVE_GET_CHECKSUM_INFO,SlaveRespBuf,10);
			break;

		case SLAVE_GET_BREAKPOINT_INFO:
			SuspendTick = 0;
#ifdef FUNC_BREAK_POINT_EN
			 SlaveRespBuf[0]=gBreakPointInfo.Signature;
			 SlaveRespBuf[1]=(BYTE)(gBreakPointInfo.USB_CurFolderNum>>8);
			 SlaveRespBuf[2]=(BYTE)(gBreakPointInfo.USB_CurFolderNum);
			 SlaveRespBuf[3]=(BYTE)(gBreakPointInfo.USB_CurFileNum>>8);
			 SlaveRespBuf[4]=(BYTE)(gBreakPointInfo.USB_CurFileNum);
			 SlaveRespBuf[5]=(BYTE)(gBreakPointInfo.USB_CurPlayTime>>24);
			 SlaveRespBuf[6]=(BYTE)(gBreakPointInfo.USB_CurPlayTime>>16);
			 SlaveRespBuf[7]=(BYTE)(gBreakPointInfo.USB_CurPlayTime>>8);
			 SlaveRespBuf[8]=(BYTE)(gBreakPointInfo.USB_CurPlayTime);
			 SlaveRespBuf[9]=(BYTE)(gBreakPointInfo.USB_FolderSum>>8);
			 SlaveRespBuf[10]=(BYTE)(gBreakPointInfo.USB_FolderSum);
			 SlaveRespBuf[11]=(BYTE)(gBreakPointInfo.USB_FileSum>>8);
			 SlaveRespBuf[12]=(BYTE)(gBreakPointInfo.USB_FileSum);
			 memcpy(&SlaveRespBuf[13], &gBreakPointInfo.USB_FileName[0], 8);

			 SlaveRespBuf[21]=(BYTE)(gBreakPointInfo.SD_CurFolderNum>>8);
			 SlaveRespBuf[22]=(BYTE)(gBreakPointInfo.SD_CurFolderNum);
			 SlaveRespBuf[23]=(BYTE)(gBreakPointInfo.SD_CurFileNum>>8);
			 SlaveRespBuf[24]=(BYTE)(gBreakPointInfo.SD_CurFileNum);
			 SlaveRespBuf[25]=(BYTE)(gBreakPointInfo.SD_CurPlayTime>>24);
			 SlaveRespBuf[26]=(BYTE)(gBreakPointInfo.SD_CurPlayTime>>16);
			 SlaveRespBuf[27]=(BYTE)(gBreakPointInfo.SD_CurPlayTime>>8);
			 SlaveRespBuf[28]=(BYTE)(gBreakPointInfo.SD_CurPlayTime);
			 SlaveRespBuf[29]=(BYTE)(gBreakPointInfo.SD_FolderSum>>8);
			 SlaveRespBuf[30]=(BYTE)(gBreakPointInfo.SD_FolderSum);
			 SlaveRespBuf[31]=(BYTE)(gBreakPointInfo.SD_FileSum>>8);
			 SlaveRespBuf[32]=(BYTE)(gBreakPointInfo.SD_FileSum);
			 memcpy(&SlaveRespBuf[33], &gBreakPointInfo.SD_FileName[0], 8);

			SlaveRespBuf[41]=gBreakPointInfo.SystemMode;
			SlaveRespBuf[42]=gBreakPointInfo.Volume;
			SlaveRespBuf[43]=gBreakPointInfo.Eq;
			SlaveRespBuf[44]=gBreakPointInfo.RepeatMode;

			SlaveSendResp(SLAVE_GET_BREAKPOINT_INFO,SlaveRespBuf,45);
#endif
			
			break;
#endif
		case SLAVE_NEXT_MODE:
			SuspendTick = 0;
			//TryGotoNextMode(gSys.SystemMode);	
			SlaveRespBuf[0]=gSys.SystemMode;
			SlaveSendResp(SLAVE_NEXT_MODE,SlaveRespBuf,1);
			break;
		case SLAVE_SELECT_MODE:				
			SuspendTick = 0;
			//TryGotoValidModeReturn(gSlaveCmdData[0]);	
			SlaveRespBuf[0]=gSys.SystemMode;
			SlaveSendResp(SLAVE_SELECT_MODE,SlaveRespBuf,1);
			break;

		case SLAVE_GET_MODE:
			SuspendTick = 0;	
			SlaveRespBuf[0]=gSys.SystemMode;
			SlaveSendResp(SLAVE_GET_MODE,SlaveRespBuf,1);
			break;
		case SLAVE_GET_DEVICE_LINK:
			SuspendTick = 0;
#ifdef FUNC_USB_EN		
			temp=IsUSBLink()?1:0;
#endif
#ifdef FUNC_CARD_EN			
			temp|=IsCardLink()? 2:0;
#endif
#ifdef FUNC_LINEIN_EN
			temp|=IsLineInLink()?4:0;
#endif
#if (defined(FUNC_AUDIO_EN) || defined(FUNC_READER_EN))			
			temp|=IsPcLink()?8:0;
#endif
			SlaveRespBuf[0]=temp;
			SlaveSendResp(SLAVE_GET_DEVICE_LINK,SlaveRespBuf,1);
			break;
	}
}
void SlaveFileSystemProcess(WORD cmd)
{

	BYTE temp=0;
	//FOLDER  folder;
//	FOLDER  ParentFolder;
//	FILE    file;
//	WORD 	SongNum;	
	WORD* 	P;	


	switch(cmd)
	{
#ifdef FUNC_FOLDER_EN
		case SLAVE_GET_VALID_FOLDER_SUM:
			if(gPlayCtrl.FolderEnable)
			{				
				SuspendTick = 0;
				SlaveRespBuf[0]=(BYTE)(gFsInfo.FolderSum>>8);
				SlaveRespBuf[1]=(BYTE)(gFsInfo.FolderSum);
				SlaveRespBuf[2]=(BYTE)(gFsInfo.ValidFolderSum>>8);
				SlaveRespBuf[3]=(BYTE)(gFsInfo.ValidFolderSum);
				SlaveSendResp(SLAVE_GET_VALID_FOLDER_SUM,SlaveRespBuf,4);					
			}
			break;
#endif
		case SLAVE_GET_FILE_SUM:
			SuspendTick = 0;
			SlaveRespBuf[0]=(BYTE)(gFsInfo.FileSum>>8);
			SlaveRespBuf[1]=(BYTE)(gFsInfo.FileSum);
			SlaveSendResp(SLAVE_GET_FILE_SUM,SlaveRespBuf,2);					
			break;
		case SLAVE_GET_FOLDER_INFO:
			#if 1//Get Current Folder Info(Play status)
			SuspendTick = 0;
			SlaveRespBuf[0]=(BYTE)(gPlayCtrl.Folder.FolderNum>>8);
			SlaveRespBuf[1]=(BYTE)gPlayCtrl.Folder.FolderNum;
			gPlayCtrl.Folder.ValidFolderNum = GetValidFolderNum(gPlayCtrl.Folder.FolderNum);
			SlaveRespBuf[2]=(BYTE)(gPlayCtrl.Folder.ValidFolderNum>>8);
			SlaveRespBuf[3]=(BYTE)gPlayCtrl.Folder.ValidFolderNum;
			SlaveRespBuf[4]=(BYTE)(gPlayCtrl.Folder.StartFileNum>>8);
			SlaveRespBuf[5]=(BYTE)gPlayCtrl.Folder.StartFileNum;
			SlaveRespBuf[6]=(BYTE)(gPlayCtrl.Folder.IncFileCnt>>8);
			SlaveRespBuf[7]=(BYTE)gPlayCtrl.Folder.IncFileCnt;
			SlaveRespBuf[8]=(BYTE)(gPlayCtrl.Folder.IncFolderCnt>>8);
			SlaveRespBuf[9]=(BYTE)gPlayCtrl.Folder.IncFolderCnt;
			SlaveRespBuf[10]=(BYTE)(gPlayCtrl.Folder.RecFileCnt>>8);
			SlaveRespBuf[11]=(BYTE)gPlayCtrl.Folder.RecFileCnt;
			SlaveRespBuf[12]=(BYTE)(gPlayCtrl.Folder.RecFolderCnt>>8);
			SlaveRespBuf[13]=(BYTE)gPlayCtrl.Folder.RecFolderCnt;
			memcpy(&SlaveRespBuf[14], &gPlayCtrl.Folder.ShortName[0], 8);
			FolderGetLongName(&gPlayCtrl.Folder, &SlaveRespBuf[23]);
			for(P = (WORD*)(&SlaveRespBuf[23]); *P != 0; P++)//计算长文件名长度
				temp++;	
			SlaveRespBuf[22]=temp*2;						
			SlaveSendResp(SLAVE_GET_FOLDER_INFO,SlaveRespBuf,23+SlaveRespBuf[22]);
			#else
			if(*(WORD*)gSlaveCmdData>gFsInfo.FolderSum)
				break;
			//打开相应文件夹
			if(*(WORD*)gSlaveCmdData==0)	
			{
				if(!FolderOpenByValidNum(&folder,NULL,*(WORD*)(&gSlaveCmdData[2])))
					break;
			}
			else
			{
				if( !FolderOpenByValidNum(&ParentFolder, NULL, *(WORD*)gSlaveCmdData) )
					break;
				if( !FolderOpenByValidNum(&folder, &ParentFolder, *(WORD*)(&gSlaveCmdData[2])) )
					break;
			}
			SuspendTick = 0;
			SlaveRespBuf[0]=(BYTE)(folder.FolderNum>>8);
			SlaveRespBuf[1]=(BYTE)folder.FolderNum;
			folder.ValidFolderNum = GetValidFolderNum(folder.FolderNum);
			SlaveRespBuf[2]=(BYTE)(folder.ValidFolderNum>>8);
			SlaveRespBuf[3]=(BYTE)folder.ValidFolderNum;
			SlaveRespBuf[4]=(BYTE)(folder.StartFileNum>>8);
			SlaveRespBuf[5]=(BYTE)folder.StartFileNum;
			SlaveRespBuf[6]=(BYTE)(folder.IncFileCnt>>8);
			SlaveRespBuf[7]=(BYTE)folder.IncFileCnt;
			SlaveRespBuf[8]=(BYTE)(folder.IncFolderCnt>>8);
			SlaveRespBuf[9]=(BYTE)folder.IncFolderCnt;
			SlaveRespBuf[10]=(BYTE)(folder.RecFileCnt>>8);
			SlaveRespBuf[11]=(BYTE)folder.RecFileCnt;
			SlaveRespBuf[12]=(BYTE)(folder.RecFolderCnt>>8);
			SlaveRespBuf[13]=(BYTE)folder.RecFolderCnt;
			memcpy(&SlaveRespBuf[14], &folder.ShortName[0], 8);
			FolderGetLongName(&folder, &SlaveRespBuf[23]);
			for(P = (WORD*)(&SlaveRespBuf[23]); *P != 0; P++)//计算长文件名长度
				temp++;	
			SlaveRespBuf[22]=temp*2;						
			SlaveSendResp(SLAVE_GET_FOLDER_INFO,SlaveRespBuf,23+SlaveRespBuf[22]);
			#endif
			break;
		case SLAVE_GET_FILE_INFO:
			#if 1//Get Current File Info(Play status)
			SuspendTick = 0;
			SlaveRespBuf[0]=(BYTE)(gPlayCtrl.File.FileNumInDisk>>8);
			SlaveRespBuf[1]=(BYTE)gPlayCtrl.File.FileNumInDisk;
			SlaveRespBuf[2]=(BYTE)(gPlayCtrl.File.FileNumInFolder>>8);
			SlaveRespBuf[3]=(BYTE)gPlayCtrl.File.FileNumInFolder;
			SlaveRespBuf[4]=(BYTE)( GetValidFolderNum(gPlayCtrl.File.FolderNum)>>8);
			SlaveRespBuf[5]=(BYTE)GetValidFolderNum(gPlayCtrl.File.FolderNum);
			memcpy(&SlaveRespBuf[6], &gPlayCtrl.File.ShortName[0], 11);
			FileGetLongName(&gPlayCtrl.File, &SlaveRespBuf[18]);			
			for(P = (WORD*)(&SlaveRespBuf[18]); *P != 0; P++)//计算长文件名长度
				temp++;
			SlaveRespBuf[17]=temp*2;	
			SlaveSendResp(SLAVE_GET_FILE_INFO,SlaveRespBuf,18+SlaveRespBuf[17]);
			#else
			SongNum = *(WORD*)(&gSlaveCmdData[2]);
			if(*(WORD*)gSlaveCmdData>gFsInfo.FolderSum)
				break;
			//打开相应文件夹
			if(*(WORD*)gSlaveCmdData!=0)	
			{
				if(!FolderOpenByNum(&folder,NULL,*(WORD*)gSlaveCmdData))
					break;
				if(!FileOpenByNum(&file, &folder, SongNum))
					break;
			}
			else
			{				
				if(!FileOpenByNum(&file, NULL, SongNum))
					break;
			}
			SuspendTick = 0;
			SlaveRespBuf[0]=(BYTE)(file.FileNumInDisk>>8);
			SlaveRespBuf[1]=(BYTE)file.FileNumInDisk;
			SlaveRespBuf[2]=(BYTE)(file.FileNumInFolder>>8);
			SlaveRespBuf[3]=(BYTE)file.FileNumInFolder;
			SlaveRespBuf[4]=(BYTE)( GetValidFolderNum(file.FolderNum)>>8);
			SlaveRespBuf[5]=(BYTE)GetValidFolderNum(file.FolderNum);
			memcpy(&SlaveRespBuf[6], &file.ShortName[0], 11);
			FileGetLongName(&file, &SlaveRespBuf[18]);			
			for(P = (WORD*)(&SlaveRespBuf[18]); *P != 0; P++)//计算长文件名长度
				temp++;
			SlaveRespBuf[17]=temp*2;	
			SlaveSendResp(SLAVE_GET_FILE_INFO,SlaveRespBuf,18+SlaveRespBuf[17]);
			#endif
			break;
	}
}
void SlavePlayCtrlProcess(WORD Cmd)
{
	FOLDER  folder;
	BYTE temp=0;
	WORD 	SongNum;
//	WORD*  P;
	switch(Cmd)
	{
		case SLAVE_PLAY:
			if(!isUSBorSD())
				break;		
			SuspendTick = 0;
			if(gPlayCtrl.State != PLAY_STATE_PLAY)
				MessageSend(MSG_FIFO_KEY, MSG_PLAY_PAUSE);
			SlaveSendResp(SLAVE_PLAY,SlaveRespBuf,0);
			break;
		case SLAVE_PAUSE:
			if(!isUSBorSD())
				break;		
			SuspendTick = 0;
			if(gPlayCtrl.State == PLAY_STATE_PLAY)
				MessageSend(MSG_FIFO_KEY, MSG_PLAY_PAUSE);
			SlaveSendResp(SLAVE_PAUSE,SlaveRespBuf,0);			
			break;
		case SLAVE_STOP:
			SuspendTick = 0;
			MessageSend(MSG_FIFO_KEY, MSG_STOP);
			SlaveSendResp(SLAVE_STOP,SlaveRespBuf,0);
			break;
		case SLAVE_NEXT:  
			SuspendTick = 0;
				//gPlayCtrl.FoldDirection=PLAY_DIRECT_NEXT;
			MessageSend(MSG_FIFO_KEY, MSG_NEXT);
			SlaveSendResp(SLAVE_NEXT,SlaveRespBuf,0);
			break;

		case SLAVE_PRE:
			SuspendTick = 0;
				//gPlayCtrl.FoldDirection=PLAY_DIRECT_PRE;	
			MessageSend(MSG_FIFO_KEY, MSG_PRE);
			SlaveSendResp(SLAVE_PRE,SlaveRespBuf,0);
			break;		

		case SLAVE_FF:
			if(!isUSBorSD())
				break;		
			SuspendTick = 0;
			MessageSend(MSG_FIFO_KEY, MSG_FF_START);
			SlaveSendResp(SLAVE_FF,SlaveRespBuf,0);
			break;

		case SLAVE_FB:
			if(!isUSBorSD())
				break;		
			SuspendTick = 0;
			MessageSend(MSG_FIFO_KEY, MSG_FB_START);
			SlaveSendResp(SLAVE_FB,SlaveRespBuf,0);		
			break;
		case SLAVE_STOP_FF_FB:
			if(!isUSBorSD())
				break;		
			SuspendTick = 0;
			MessageSend(MSG_FIFO_KEY, MSG_FF_FB_END);
			SlaveSendResp(SLAVE_STOP_FF_FB,SlaveRespBuf,0);						
			break;
		case SLAVE_SEEK_PLAY_TIME:
			if(!isUSBorSD())
				break;		
			SuspendTick = 0;
			if(*(WORD*)gSlaveCmdData<= (WORD) gSongInfo.TotalPlayTime)
			{
				gSongInfo.CurPlayTime = *(WORD*)gSlaveCmdData;	
				SongPlayStart();
			}
			SlaveSendResp(SLAVE_SEEK_PLAY_TIME,SlaveRespBuf,0);
			break;
		case SLAVE_NEXT_FOLDER:
			if(!isUSBorSD())
				break;		
			SuspendTick = 0;
			MessageSend(MSG_FIFO_KEY, MSG_NEXT_FOLDER);
			SlaveSendResp(SLAVE_NEXT_FOLDER,SlaveRespBuf,0);
			break;
		case SLAVE_PRE_FOLDER:
			if(!isUSBorSD())
				break;		
			SuspendTick = 0;
			MessageSend(MSG_FIFO_KEY, MSG_PRE_FOLDER);
			SlaveSendResp(SLAVE_PRE_FOLDER,SlaveRespBuf,0);
			break;
		case SLAVE_PLAY_PAUSE:
			SuspendTick = 0;
			MessageSend(MSG_FIFO_KEY, MSG_PLAY_PAUSE);
			SlaveSendResp(SLAVE_PLAY_PAUSE,SlaveRespBuf,0);
			break;
		case SLAVE_SELECT_SONG:
			if(!isUSBorSD())
				break;		
                     gPlayCtrl.FolderEnable = TRUE;
	              gPlayCtrl.FileNum = gPlayCtrl.File.FileNumInFolder;
			SongNum= *(WORD*)(&gSlaveCmdData[2]); //需要打开的曲目号
#ifdef	FUNC_FOLDER_EN		
			if((*(WORD*)gSlaveCmdData>gFsInfo.ValidFolderSum)||(*(WORD*)gSlaveCmdData==0))
				break;
			if(!FolderOpenByValidNum(&folder, NULL, *(WORD*)gSlaveCmdData))
			{
				MessageSend(MSG_FIFO_KEY, MSG_NEXT_FOLDER);
				break;
			}
			if( (SongNum<=0) || (SongNum>folder.IncFileCnt) )
				break;						
			gPlayCtrl.Folder=folder;
			gPlayCtrl.FolderNum = *(WORD*)gSlaveCmdData;
			gPlayCtrl.FileNum =SongNum;				
			
#else			
			if((SongNum<=0)|| (SongNum>gFsInfo.FileSum))
				break;				
			gPlayCtrl.FileNum =SongNum;				
#endif
			if(!SongPlayInit())
			{
				gPlayCtrl.State = PLAY_STATE_STOP;
					MessageSend(MSG_FIFO_KEY, MSG_NEXT);
				break;	
			}
			SuspendTick = 0;
			SlaveSendResp(SLAVE_SELECT_SONG,SlaveRespBuf,1);
			break;
		case SLAVE_GET_PLAY_STATUS:
			if(!isUSBorSD())
				break;		
			SuspendTick = 0;
			SlaveRespBuf[0]=gSys.SystemMode;//gFsInfo.DevID;
			SlaveRespBuf[1]=gPlayCtrl.State;
			gPlayCtrl.Folder.ValidFolderNum = GetValidFolderNum(gPlayCtrl.Folder.FolderNum);
			SlaveRespBuf[2]=(BYTE)(gPlayCtrl.Folder.FolderNum>>8);
			SlaveRespBuf[3]=(BYTE)gPlayCtrl.Folder.FolderNum;	 
			SlaveRespBuf[4]=(BYTE)(gPlayCtrl.Folder.ValidFolderNum>>8);
			SlaveRespBuf[5]=(BYTE)gPlayCtrl.Folder.ValidFolderNum;	 
			SlaveRespBuf[6]=(BYTE)(gPlayCtrl.File.FileNumInDisk>>8);
			SlaveRespBuf[7]=(BYTE)gPlayCtrl.File.FileNumInDisk;
			SlaveRespBuf[8]=(BYTE)(gPlayCtrl.File.FileNumInFolder>>8);
			SlaveRespBuf[9]=(BYTE)gPlayCtrl.File.FileNumInFolder;
			SlaveRespBuf[10]=(BYTE)(gSongInfo.CurPlayTime>>24);
			SlaveRespBuf[11]=(BYTE)(gSongInfo.CurPlayTime>>16);
			SlaveRespBuf[12]=(BYTE)(gSongInfo.CurPlayTime>>8);
			SlaveRespBuf[13]=(BYTE)gSongInfo.CurPlayTime;
			SlaveSendResp(SLAVE_GET_PLAY_STATUS,SlaveRespBuf,14);
			break;
		case SLAVE_GET_SONG_INFO:
			if(!isUSBorSD())
				break;		
			SuspendTick = 0;
			SlaveRespBuf[0]=gSongInfo.SongType;
			SlaveRespBuf[1]=gSongInfo.ChannelNum;
			SlaveRespBuf[2]=(BYTE)(gSongInfo.SamplesPerSecond>>24);
			SlaveRespBuf[3]=(BYTE)(gSongInfo.SamplesPerSecond>>16);
			SlaveRespBuf[4]=(BYTE)(gSongInfo.SamplesPerSecond>>8);
			SlaveRespBuf[5]=(BYTE)gSongInfo.SamplesPerSecond;

			SlaveRespBuf[6]=(BYTE)(gSongInfo.BytesPerSecond>>24);			
			SlaveRespBuf[7]=(BYTE)(gSongInfo.BytesPerSecond>>16);			
			SlaveRespBuf[8]=(BYTE)(gSongInfo.BytesPerSecond>>8);			
			SlaveRespBuf[9]=(BYTE)gSongInfo.BytesPerSecond;			
			SlaveRespBuf[10]=(BYTE)(gSongInfo.TotalPlayTime>>24);
			SlaveRespBuf[11]=(BYTE)(gSongInfo.TotalPlayTime>>16);
			SlaveRespBuf[12]=(BYTE)(gSongInfo.TotalPlayTime>>8);
			SlaveRespBuf[13]=(BYTE)gSongInfo.TotalPlayTime;
			SlaveRespBuf[14]=(BYTE)(gSongInfo.FileHeaderLen>>24);
			SlaveRespBuf[15]=(BYTE)(gSongInfo.FileHeaderLen>>16);
			SlaveRespBuf[16]=(BYTE)(gSongInfo.FileHeaderLen>>8);
			SlaveRespBuf[17]=(BYTE)gSongInfo.FileHeaderLen;
			SlaveRespBuf[18]=(BYTE)gSongInfo.IsVBR;
			SlaveRespBuf[19]=gSongInfo.Mp3MpegVersion;
			SlaveRespBuf[20]=gSongInfo.Mp3Layer;
			SlaveSendResp(SLAVE_GET_SONG_INFO,SlaveRespBuf,21);
			break;		
		case SLAVE_SET_REPEAT_MODE:
			if(!isUSBorSD())
				break;		
			SuspendTick = 0;
			if(gSlaveCmdData[0] < REPEAT_MODE_SUM)
			{
				gPlayCtrl.RepeatMode = gSlaveCmdData[0];
                            if(gPlayCtrl.RepeatMode==REPEAT_RANDOM)
				{
					gPlayCtrl.FolderEnable = FALSE;
					gPlayCtrl.FileNum = gPlayCtrl.File.FileNumInDisk;
				}
				else if(gPlayCtrl.RepeatMode==REPEAT_FOLDER)
				{
					gPlayCtrl.FolderEnable = TRUE;
					gPlayCtrl.FileNum = gPlayCtrl.File.FileNumInFolder;
				}
			}
#ifdef FUNC_BREAK_POINT_EN
			BP_SetPlayAttrib();
#endif

			SlaveSendResp(SLAVE_SET_REPEAT_MODE,SlaveRespBuf,0);
			break;		
		case SLAVE_GET_REPEAT_MODE:
			SuspendTick = 0;
			SlaveRespBuf[0]=gPlayCtrl.RepeatMode;
			SlaveSendResp(SLAVE_GET_REPEAT_MODE,SlaveRespBuf,1);
			break;		
		case SLAVE_ENABLE_FOLDER:
			SuspendTick = 0;
			if(!gPlayCtrl.FolderEnable)
			{
					gPlayCtrl.FolderEnable = TRUE;
					gPlayCtrl.FileNum = gPlayCtrl.File.FileNumInFolder;
			}	
#ifdef FUNC_BREAK_POINT_EN
			if(gSys.SystemMode == SYS_MODE_USB)	
			{		
				BP_SaveInfo(&gBreakPointInfo.PowerMemory.USB_FolderEnFlag, sizeof(gBreakPointInfo.PowerMemory.USB_FolderEnFlag));
			}
			else if(gSys.SystemMode == SYS_MODE_SD)
			{
				BP_SaveInfo(&gBreakPointInfo.PowerMemory.SD_FolderEnFlag, sizeof(gBreakPointInfo.PowerMemory.SD_FolderEnFlag));
			}							
#endif
			SlaveSendResp(SLAVE_ENABLE_FOLDER,SlaveRespBuf,0);
			break;		
		case SLAVE_DISABLE_FOLDER:
			if(!isUSBorSD())
				break;		
			SuspendTick = 0;
			if(gPlayCtrl.FolderEnable)
			{
					gPlayCtrl.FolderEnable = FALSE;
					gPlayCtrl.FileNum = gPlayCtrl.File.FileNumInDisk;
			}
#ifdef FUNC_BREAK_POINT_EN
			if(gSys.SystemMode == SYS_MODE_USB)	
			{		
				BP_SaveInfo(&gBreakPointInfo.PowerMemory.USB_FolderEnFlag, sizeof(gBreakPointInfo.PowerMemory.USB_FolderEnFlag));
			}
			else if(gSys.SystemMode == SYS_MODE_SD)
			{
				BP_SaveInfo(&gBreakPointInfo.PowerMemory.SD_FolderEnFlag, sizeof(gBreakPointInfo.PowerMemory.SD_FolderEnFlag));
			}							
#endif
			SlaveSendResp(SLAVE_DISABLE_FOLDER,SlaveRespBuf,0);		
			break;	
#if 0//def FUNC_TAG_EN	
		case SLAVE_GET_TITLE:
			if(!isUSBorSD())
				break;		
			SuspendTick = 0;
			SlaveRespBuf[0]=gTagInfo.TagType;
			SlaveRespBuf[1]=gTagInfo.TagCharSet;
			for(P = (WORD*)(&gTagInfo.Title[0]); *P != 0; P++)//计算Artist长度
				temp++;
			temp=temp*2;
			if(temp>30) temp=30;	
			SlaveRespBuf[2]=temp;						
			memcpy(&SlaveRespBuf[3],&gTagInfo.Title[0],temp);
			SlaveSendResp(SLAVE_GET_TITLE,SlaveRespBuf,temp+3);	
			break;		
		case SLAVE_GET_ARTIST:
			if(!isUSBorSD())
				break;		
			SuspendTick = 0;
			SlaveRespBuf[0]=gTagInfo.TagType;
			SlaveRespBuf[1]=gTagInfo.TagCharSet;			
			for(P = (WORD*)(&gTagInfo.Artist[0]); *P != 0; P++)//计算Artist长度
				temp++;
			temp=temp*2;
			if(temp>30) temp=30;	
			SlaveRespBuf[2]=temp;						
			memcpy(&SlaveRespBuf[3],&gTagInfo.Artist[0],temp);
			SlaveSendResp(SLAVE_GET_ARTIST,SlaveRespBuf,temp+3);	
			break;		
		case SLAVE_GET_ALBUM:
			if(!isUSBorSD())
				break;		
			SuspendTick = 0;
			SlaveRespBuf[0]=gTagInfo.TagType;
			SlaveRespBuf[1]=gTagInfo.TagCharSet;
			for(P = (WORD*)(&gTagInfo.Album[0]); *P != 0; P++)//计算Artist长度
				temp++;
			temp=temp*2;
			if(temp>30) temp=30;	
			SlaveRespBuf[2]=temp;						
			memcpy(&SlaveRespBuf[3],&gTagInfo.Album[0],temp);
			SlaveSendResp(SLAVE_GET_ALBUM,SlaveRespBuf,temp+3);	
			break;		
		case SLAVE_GET_COMMENT:
			if(!isUSBorSD())
				break;		
			SuspendTick = 0;
			SlaveRespBuf[0]=gTagInfo.TagType;
			SlaveRespBuf[1]=gTagInfo.TagCharSet;
			for(P = (WORD*)(&gTagInfo.Comment[0]); *P != 0; P++)//计算Artist长度
				temp++;
			temp=temp*2;
			if(temp>30) temp=30;	
			SlaveRespBuf[2]=temp;						
			memcpy(&SlaveRespBuf[3],&gTagInfo.Comment[0],temp);
			SlaveSendResp(SLAVE_GET_COMMENT,SlaveRespBuf,temp+3);	
			break;		
		case SLAVE_GET_YEAR:
			if(!isUSBorSD())
				break;		
			SuspendTick = 0;
			SlaveRespBuf[0]=gTagInfo.TagType;
			SlaveRespBuf[1]=gTagInfo.TagCharSet;
			for(P = (WORD*)(&gTagInfo.Year[0]); *P != 0; P++)//计算Artist长度
				temp++;
			temp=temp*2;
			if(temp>30) temp=30;	
			SlaveRespBuf[2]=temp;						
			memcpy(&SlaveRespBuf[3],&gTagInfo.Year[0],temp);
			SlaveSendResp(SLAVE_GET_YEAR,SlaveRespBuf,temp+3);	
			break;		
#endif
	}
}
void SlaveAudioProcess(WORD Cmd)
{
	switch(Cmd)
	{
		case SLAVE_VOLADD:
			SuspendTick = 0;
			MessageSend(MSG_FIFO_KEY, MSG_VOL_ADD);
			SlaveSendResp(SLAVE_VOLADD,SlaveRespBuf,0);
			break;		

		case SLAVE_VOLSUB:
			SuspendTick = 0;
			MessageSend(MSG_FIFO_KEY, MSG_VOL_SUB);
			SlaveSendResp(SLAVE_VOLSUB,SlaveRespBuf,0);
			break;
		case SLAVE_SET_VOL:
			SuspendTick = 0;
			if(gSlaveCmdData[0] <= VOLUME_MAX)
			{ 
				gSys.Volume=gSlaveCmdData[0];
			}
			SetVolume();
#ifdef FUNC_BREAK_POINT_EN
			BP_SaveInfo((BYTE*)(&gBreakPointInfo.PowerMemory.Volume), sizeof(gBreakPointInfo.PowerMemory.Volume));
#endif
			SlaveSendResp(SLAVE_SET_VOL,SlaveRespBuf,0);
			break;
		case SLAVE_GET_VOL:
			SuspendTick = 0;
			SlaveRespBuf[0]=gSys.Volume;
			SlaveSendResp(SLAVE_GET_VOL,SlaveRespBuf,1);
			break;
		case SLAVE_SET_EQ:				
			SuspendTick = 0;
			if(gPlayCtrl.Eq <= 11)
			{
				gPlayCtrl.Eq=gSlaveCmdData[0];
			}
			PlayEQ2Decd();
#ifdef FUNC_BREAK_POINT_EN
			BP_SaveInfo((BYTE*)(&gBreakPointInfo.Eq), sizeof(gBreakPointInfo.Eq));
#endif
			SlaveSendResp(SLAVE_SET_EQ,SlaveRespBuf,0);
			break;
		case SLAVE_GET_EQ:
			SuspendTick = 0;
			SlaveRespBuf[0]=gPlayCtrl.Eq;
			SlaveSendResp(SLAVE_GET_EQ,SlaveRespBuf,1);
			break;			
		case SLAVE_MUTE:
			SuspendTick = 0;
			if(!gSys.MuteFg)
			{				
				gSys.MuteFg = TRUE;
				MuteOn(TRUE, TRUE);    
			}
			SlaveSendResp(SLAVE_MUTE,SlaveRespBuf,0);
			break;
		case SLAVE_UNMUTE:
			SuspendTick = 0;
			if(gSys.MuteFg)
			{
				gSys.MuteFg = FALSE;
				UnMute();			
			}
			SlaveSendResp(SLAVE_UNMUTE,SlaveRespBuf,0);
			break;
#if 0
		case SLAVE_SET_LINEIN_CHANNEL:
			SuspendTick = 0;			
			if(gSlaveCmdData[0]&0x04)
				SET_A6A7_FMIN();		// config A6/A7 as IN.
			if(gSlaveCmdData[0]&0x08)
				SET_C0C1_FMIN();		// config c0/c1 as IN.
			InDacChannelSel(gSlaveCmdData[0]);		//0x1 0x2,0x4,0x8,0x10
			//InDacMuteDis();
			SlaveSendResp(SLAVE_SET_LINEIN_CHANNEL,SlaveRespBuf,0);
			break;

		case SLAVE_SET_LINEIN_GAIN:		//增益最大64级，0 ~63
			SuspendTick = 0;
			
			if(gSlaveCmdData[0]<=63) 
			{
				InDacLineInGain(gLineInGain[gSlaveCmdData[0]], gLineInGain[gSlaveCmdData[0]]);
				gUserLineInGain = gSlaveCmdData[0]; 
			}
			SlaveSendResp(SLAVE_SET_LINEIN_GAIN,SlaveRespBuf,0);
			break;
			
		case SLAVE_GET_LINEIN_GAIN:		//增益最大64级，0 ~63
			SuspendTick = 0;
			SlaveRespBuf[0] = gUserLineInGain;
			SlaveSendResp(SLAVE_GET_LINEIN_GAIN,SlaveRespBuf,1);
			break;
#endif			

	}
}
#ifdef FUNC_RTC_EN
void SlaveRtcProcess(WORD Cmd)
{
	BYTE uRtcAlarmNum;
	BYTE uAlarmMode;
	RTC_DATE_TIME uRtcTime;
	RTC_DATE_TIME uAlarmTime;
	RTC_LUNAR_DATE uLunarDate;

	BYTE temp=0;
	
	switch(Cmd)
	{
		case SLAVE_SET_TIME:
			SuspendTick = 0;
			uRtcTime.Year= *(WORD*)gSlaveCmdData;
			uRtcTime.Mon =gSlaveCmdData[2];
			uRtcTime.Date=gSlaveCmdData[3];
			uRtcTime.Hour=gSlaveCmdData[4];
			uRtcTime.Min =gSlaveCmdData[5];
			uRtcTime.Sec =gSlaveCmdData[6];
			SetRTCCurrTime(&uRtcTime);
			SlaveSendResp(SLAVE_SET_TIME,SlaveRespBuf,0);
			break;
		case SLAVE_GET_TIME:
			SuspendTick = 0;
			GetRTCCurrTime(&uRtcTime);
			SlaveRespBuf[0]=(BYTE)(uRtcTime.Year>>8);
			SlaveRespBuf[1]=(BYTE)uRtcTime.Year;
			SlaveRespBuf[2]=uRtcTime.Mon;
			SlaveRespBuf[3]=uRtcTime.Date;
			SlaveRespBuf[4]=uRtcTime.WDay;			
			SlaveRespBuf[5]=uRtcTime.Hour;
			SlaveRespBuf[6]=uRtcTime.Min;
			SlaveRespBuf[7]=uRtcTime.Sec;
			SlaveSendResp(SLAVE_GET_TIME,SlaveRespBuf,8);
			break;
#ifdef FUNC_ALARM_EN
		case SLAVE_SET_ALARM_TIME:
			
			if((gSlaveCmdData[0] <= 8)&&(gSlaveCmdData[0] != 0))	//RtcAlarm must be available
			{
				SuspendTick = 0;
				//OpenAlarm(gSlaveCmdData[0]);
				uRtcAlarmNum = gSlaveCmdData[0];
				uAlarmMode  = gSlaveCmdData[1];
				uAlarmTime.Year = *((WORD*)(&gSlaveCmdData[2]));
				uAlarmTime.Mon = gSlaveCmdData[4];
				uAlarmTime.Date = gSlaveCmdData[5];
				uAlarmTime.WDay = gSlaveCmdData[6];
				uAlarmTime.Hour = gSlaveCmdData[7];
				uAlarmTime.Min = gSlaveCmdData[8];
				uAlarmTime.Sec = gSlaveCmdData[9];
				SetAlarmTime(uRtcAlarmNum, uAlarmMode, &uAlarmTime);
				SlaveSendResp(SLAVE_SET_ALARM_TIME,SlaveRespBuf,0);
			}
			break;
		case SLAVE_GET_ALARM_TIME:
			temp=GetAlarmTime(gSlaveCmdData[0],&uAlarmTime);
			if( (temp!=0)&&(temp!=0xff) )
			{
				SuspendTick = 0;
				uAlarmMode = temp;
				SlaveRespBuf[0]=uAlarmMode;
				SlaveRespBuf[1]=(BYTE)(uAlarmTime.Year>>8);
				SlaveRespBuf[2]=(BYTE)(uAlarmTime.Year);
				SlaveRespBuf[3]=uAlarmTime.Mon;
				SlaveRespBuf[4]=uAlarmTime.Date;
				SlaveRespBuf[5]=uAlarmTime.WDay;			
				SlaveRespBuf[6]=uAlarmTime.Hour;
				SlaveRespBuf[7]=uAlarmTime.Min;
				SlaveRespBuf[8]=uAlarmTime.Sec;
				SlaveSendResp(SLAVE_GET_ALARM_TIME,SlaveRespBuf,9);
			}			
			break;
		case SLAVE_ENABLE_ALARM:			
			SuspendTick = 0;			
			if((gSlaveCmdData[0]<=8)&&(gSlaveCmdData[0]!=0))
			{
				OpenAlarm(gSlaveCmdData[0]);
				SlaveSendResp(SLAVE_ENABLE_ALARM,SlaveRespBuf,0);
			}
			break;
		case SLAVE_DISABLE_ALARM:			
			SuspendTick = 0;			
			if((gSlaveCmdData[0]<=8)&&(gSlaveCmdData[0]!=0))
			{
				CloseAlarm(gSlaveCmdData[0]);
				SlaveSendResp(SLAVE_DISABLE_ALARM,SlaveRespBuf,0);
			}
			break;
		case SLAVE_GET_ALARM_STATUS:			
			SuspendTick = 0;
			SlaveRespBuf[0]=GetAlarmStatus(gSlaveCmdData[0]);
			SlaveSendResp(SLAVE_GET_ALARM_STATUS,SlaveRespBuf,1);
			break;

		case SLAVE_GET_ALARM_FLAG:			
			SuspendTick = 0;			
			SlaveRespBuf[0]=CheckRtcAlarmFlag();
			SlaveSendResp(SLAVE_GET_ALARM_FLAG,SlaveRespBuf,1);
			break;

#endif
		case SLAVE_GET_MONTH_DAYS:
			SuspendTick = 0;
			SlaveRespBuf[0]=GetMonthDays(*(WORD*)gSlaveCmdData,gSlaveCmdData[2]);
			SlaveSendResp(SLAVE_GET_MONTH_DAYS,SlaveRespBuf,1);
			break;

#ifdef FUNC_LUNAR_EN
		case SLAVE_SOLAR_TO_LUNAR:
			SuspendTick = 0;
	    		uRtcTime.Year=*((WORD*)gSlaveCmdData);
	    		uRtcTime.Mon=gSlaveCmdData[2]; 
	    		uRtcTime.Date=gSlaveCmdData[3]; 
			SolarToLunar(&uRtcTime,&uLunarDate);
			SlaveRespBuf[0]= (BYTE)(uLunarDate.Year>>8);
			SlaveRespBuf[1]= (BYTE)uLunarDate.Year;
			SlaveRespBuf[2]= uLunarDate.Month;
			SlaveRespBuf[3]= uLunarDate.Date;
			SlaveRespBuf[4]= uLunarDate.MonthDays;
			SlaveRespBuf[5]= (BYTE)uLunarDate.IsLeapMonth;
			SlaveSendResp(SLAVE_SOLAR_TO_LUNAR,SlaveRespBuf,6);			
			break;
		case SLAVE_GET_HEAVENLY_STEM:
			SuspendTick = 0;
			SlaveRespBuf[0]=GetHeavenlyStem(*(WORD*)gSlaveCmdData);
			SlaveSendResp(SLAVE_GET_HEAVENLY_STEM,SlaveRespBuf,1);			
			break;
		case SLAVE_GET_EARTHLY_BRANCH:
			SuspendTick = 0;
			SlaveRespBuf[0]=GetEarthlyBranch(*(WORD*)gSlaveCmdData);
			SlaveSendResp(SLAVE_GET_EARTHLY_BRANCH,SlaveRespBuf,1);			
			break;
#endif


	}	
}
#endif
void SlaveGpioProcess(WORD Cmd)
{
	
	switch(Cmd)
	{
		case SLAVE_SET_GPIO_REG:  
			SuspendTick = 0;
			SetGpioReg(gSlaveCmdData[0], gSlaveCmdData[1]);
 			SlaveSendResp(SLAVE_SET_GPIO_REG,SlaveRespBuf,0);
			break;        
		case SLAVE_GET_GPIO_REG:  
			SuspendTick = 0;
			SlaveRespBuf[0]=GetGpioReg(gSlaveCmdData[0]);        
			SlaveSendResp(SLAVE_GET_GPIO_REG,SlaveRespBuf,1);
			break;
		case SLAVE_SET_GPIO_REG_BIT:   
			SuspendTick = 0;
			SetGpioRegBit(gSlaveCmdData[0], gSlaveCmdData[1]);   
			SlaveSendResp(SLAVE_SET_GPIO_REG_BIT,SlaveRespBuf,0);
			break;
		case SLAVE_CLR_GPIO_REG_BIT:      
			SuspendTick = 0;
			ClrGpioRegBit(gSlaveCmdData[0],gSlaveCmdData[1]);        
			SlaveSendResp(SLAVE_CLR_GPIO_REG_BIT,SlaveRespBuf,0);
			break;
	}
}

void SlaveAdcProcess(WORD Cmd)
{
	BYTE i=0;
 	WORD 	rtn;
	switch(Cmd)
	{
		case SLAVE_ENABLE_ADC_CHANNEL:
			SuspendTick = 0;
			switch(gSlaveCmdData[0])
			{
				case ADC_CHANNEL_B6:
					SET_B6_ANALOG_IN();
					break;
				case ADC_CHANNEL_D7:
					SET_D7_ANALOG_IN();
					break;
				case ADC_CHANNEL_E0:
					SET_E0_ANALOG_IN();
					break;
				case ADC_CHANNEL_E1:
					SET_E1_ANALOG_IN();
					break;
			}
			SlaveSendResp(SLAVE_ENABLE_ADC_CHANNEL,SlaveRespBuf,0);
			break;
		case SLAVE_DISABLE_ADC_CHANNEL:
			SuspendTick = 0;
			switch(gSlaveCmdData[0])
			{
				case ADC_CHANNEL_B6:
					SET_B6_GPIO_IN();
					break;
				case ADC_CHANNEL_D7:
					SET_D7_GPIO_IN();
					break;
				case ADC_CHANNEL_E0:
					SET_E0_GPIO_IN();
					break;
				case ADC_CHANNEL_E1:
					SET_E1_GPIO_IN();
					break;
			}
			SlaveSendResp(SLAVE_DISABLE_ADC_CHANNEL,SlaveRespBuf,0);
			break;
		case SLAVE_GET_ADC_CHANNEL_VALUE:
			i=gSlaveCmdData[0];						
			if((i>0)&&(i<=7))
			{
				SuspendTick = 0;

				rtn=AdcReadOneChannel(i, 1);	 //参考电压为3.3v
				SlaveRespBuf[0]=(BYTE)rtn;
				SlaveSendResp(SLAVE_GET_ADC_CHANNEL_VALUE,SlaveRespBuf,1);
			}
			break;
		case SLAVE_GET_VIN_VOLTAGE:
			SuspendTick = 0;
			rtn=GetLdoinVoltage();	 ////return votage at LDO_5VIN pin, unit is mV.
			SlaveRespBuf[0]=(BYTE)(rtn>>8);
			SlaveRespBuf[1]=(BYTE)rtn;
			SlaveSendResp(SLAVE_GET_VIN_VOLTAGE,SlaveRespBuf,2);
			break;
	}
}

void SlavePwmProcess(WORD Cmd)
{
	BYTE i=0;	
	switch(Cmd)
	{
		case SLAVE_PWM_CONFIG_CHANNEL:				 //refer to pwm.h			
			if( (gSlaveCmdData[0]<=5)&&(gSlaveCmdData[1]<=1)
				&&( *(WORD*)(&gSlaveCmdData[2])>=*(WORD*)(&gSlaveCmdData[4]) )  
			)
			{
				SuspendTick = 0;
				PwmConfigChannel(gSlaveCmdData[0], gSlaveCmdData[1], *(WORD*)(&gSlaveCmdData[2]), *(WORD*)(&gSlaveCmdData[4]));
				SlaveSendResp(SLAVE_PWM_CONFIG_CHANNEL,SlaveRespBuf,0);
			}
			break;
		case SLAVE_ENABLE_PWM_CHANNEL:	//注意：C0和D0不能同时用做PWM输出，C1和D1不能同时用做PWM输出
			if(gSlaveCmdData[0]<=5)
			{
				SuspendTick = 0;
				PwmEnableChannel(gSlaveCmdData[0]);
				SlaveSendResp(SLAVE_ENABLE_PWM_CHANNEL,SlaveRespBuf,0);
			}
			break;
		case SLAVE_DISABLE_PWM_CHANNEL:
			if(gSlaveCmdData[0]<=5)
			{
				SuspendTick = 0;
				PwmDisableChannel(gSlaveCmdData[0]);
				SlaveSendResp(SLAVE_DISABLE_PWM_CHANNEL,SlaveRespBuf,0);
			}
			break;
		case SLAVE_SET_DRIVE_MODE_CHANNEL:
			if((gSlaveCmdData[0]<=5)&&(gSlaveCmdData[1]<=3))
			{
				SuspendTick = 0;
				PwmSetDriveMode(gSlaveCmdData[0],gSlaveCmdData[1]);
				SlaveSendResp(SLAVE_SET_DRIVE_MODE_CHANNEL,SlaveRespBuf,0);
			}
			break;	
	}
}
void SlaveSleepProcess(WORD Cmd)
{
	
	switch(Cmd)
	{
		case SLAVE_SET_WAKEUP_MODE:
			SuspendTick = 0;
			SlaveSendResp(SLAVE_SET_WAKEUP_MODE,SlaveRespBuf,0);
			SetWakeupSrcInSleep( *(WORD*) gSlaveCmdData );			
			break;
		case SLAVE_GOTO_SLEEP:
			SuspendTick = 0;
			SlaveSendResp(SLAVE_GOTO_SLEEP,SlaveRespBuf,0);
			if(gSlaveCmdData[0]<=1)
			{
				InDacPowerDown(FALSE);
				InDacChannelSel(DAC_CH_NONE);
				GoToSleepMode(gSlaveCmdData[0]);
			}
			break;
	}
}

VOID SlaveRadioRetCurrentStatus(VOID)
{
#ifdef FUNC_RADIO_EN
	SlaveRespBuf[0] = gRadioCtrl.State;
	SlaveRespBuf[1] = gRadioCtrl.SeekMode ;
	SlaveRespBuf[2] = gRadioCtrl.RadioSeekResult;
	SlaveRespBuf[3] = RadioGetStereoState();
	SlaveRespBuf[4] = (BYTE)(gRadioData.CurrFreq>>8);
	SlaveRespBuf[5] = (BYTE)gRadioData.CurrFreq;
	SlaveRespBuf[6] = gRadioData.CurrStation ;//当前台号// Start from 1,0表示没有台
	SlaveRespBuf[7] = gRadioData.Area1StationSum ;	//760~874区间的频道数
	SlaveRespBuf[8] = gRadioData.StationSum ;
	SlaveSendResp(SLAVE_GET_CUR_STATUS,SlaveRespBuf,9);
#endif
}

VOID SlaveRadioProcess(WORD Cmd)
{
       BYTE i;
#ifdef FUNC_RADIO_EN
	switch(Cmd)
	{
		case SLAVE_SET_RADIO_PARAM:
			SuspendTick = 0;
			SlaveSendResp(SLAVE_SET_RADIO_PARAM,SlaveRespBuf,0);
			gRadioCtrl.RadioLowerBound = *((WORD*)(&gSlaveCmdData[0])); 
			gRadioCtrl.RadioUpperBound = *((WORD*)(&gSlaveCmdData[2])); 
			gRadioCtrl.RadioFreqBase = gRadioCtrl.RadioLowerBound;
			//gRadioCtrl.RadioSeekStep = gSlaveCmdData[4];
			RadioInit();
			break;
		case SLAVE_SET_RADIO_FREQ:
			SuspendTick = 0;
			SlaveSendResp(SLAVE_SET_RADIO_FREQ,SlaveRespBuf,0);
			gRadioData.CurrFreq =  *((WORD*)(&gSlaveCmdData[0])); 
			RadioSetFreq();
			RadioMute(FALSE);		
			RadioWriteData();
			break;			
		case SLAVE_NEXT_STEP:
			SuspendTick = 0;
			SlaveSendResp(SLAVE_NEXT_STEP,SlaveRespBuf,0);
			MessageSend(MSG_FIFO_KEY, MSG_FREQ_UP);
			break;		
		case SLAVE_PREV_STEP:
			SuspendTick = 0;
			SlaveSendResp(SLAVE_PREV_STEP,SlaveRespBuf,0);
			MessageSend(MSG_FIFO_KEY, MSG_FREQ_DN);
			break;		
		case SLAVE_NEXT_STATION:
			SuspendTick = 0;
			SlaveSendResp(SLAVE_NEXT_STATION,SlaveRespBuf,0);
			MessageSend(MSG_FIFO_KEY, MSG_NEXT);
			break;		
		case SLAVE_PREV_STATION:
			SuspendTick = 0;
			SlaveSendResp(SLAVE_PREV_STATION,SlaveRespBuf,0);
			MessageSend(MSG_FIFO_KEY, MSG_PRE);
			break;		
		case SLAVE_AUTO_SCAN:
			SuspendTick = 0;
			SlaveSendResp(SLAVE_AUTO_SCAN,SlaveRespBuf,0);
			MessageSend(MSG_FIFO_KEY, MSG_SCAN);
			break;
		case SLAVE_NEXT_SEEK:
			SuspendTick = 0;
			SlaveSendResp(SLAVE_NEXT_SEEK,SlaveRespBuf,0);
			MessageSend(MSG_FIFO_KEY, MSG_FF_START);
			break;		
		case SLAVE_PREV_SEEK:
			SuspendTick = 0;
			SlaveSendResp(SLAVE_PREV_SEEK,SlaveRespBuf,0);
			MessageSend(MSG_FIFO_KEY, MSG_FB_START);
			break;	
		case SLAVE_SAVE_STATION:
			SuspendTick = 0;
			SlaveSendResp(SLAVE_SAVE_STATION,SlaveRespBuf,0);
			MessageSend(MSG_FIFO_KEY, MSG_SAVE_STATION);
			break;		
		case SLAVE_SET_MONO_MODE:
			SuspendTick = 0;			
			SlaveSendResp(SLAVE_SET_MONO_MODE,SlaveRespBuf,0);
			if(gSlaveCmdData[0]<=1)
			{
				RadioMonoModeSet(gSlaveCmdData[0]); // 1 -- Force receiver in mono mode;	0 -- Not forced,stereo and mono auto selected
			}
			break;			
		case SLAVE_GET_CUR_STATUS:
			SuspendTick = 0;	
			SlaveRadioRetCurrentStatus();
			break;	
		case SLAVE_GET_STORED_STATION:
			SuspendTick = 0;		
			for(i=0;i<MAX_RADIO_STATION_NUM;i++)
			{
				SlaveRespBuf[i] = gRadioData.Stations[i];
			}
			SlaveSendResp(SLAVE_GET_STORED_STATION,SlaveRespBuf,MAX_RADIO_STATION_NUM);
			break;	
	}
#endif
}
//
//Deal with the recived cmd

VOID SlaveStateCtrl()
{

	BYTE ErrorAck[1]=0xFF;
	WORD CmdCode;
	/*if(!SlaveRcvCmd())
	{
		return;
	}*/
	memset(SlaveRespBuf,0,251);
	SuspendTick = 1;
	CmdCode = (gSlaveCmd[1] << 8) + gSlaveCmd[2];
	
	switch (gSlaveCmd[1])
	{
		case 0x01:
			SlaveSysModeProcess(CmdCode);
			break;
		case 0x02:
			if(!isUSBorSD())
				break;		
			SlaveFileSystemProcess(CmdCode);
			break;
		case 0x03:
			SlavePlayCtrlProcess(CmdCode);
			break;
		case 0x04:
			SlaveAudioProcess(CmdCode);
			break;
#ifdef FUNC_RTC_EN
		case 0x05:
			SlaveRtcProcess(CmdCode);
			break;
#endif
	  	case 0x06:
			SlaveGpioProcess(CmdCode);
			break;
	 	case 0x07:
			SlaveAdcProcess(CmdCode);
			break;
		case 0x08:
			SlavePwmProcess(CmdCode);
			break;
		case 0x09:
			SlaveSleepProcess(CmdCode);
			break;
		case 0x0A:
			SlaveRadioProcess(CmdCode);
			break;				
		default:
			break;
	}

	if(SuspendTick & (CmdCode != 0))
	{
		SuspendTick = 0;
		SlaveSendResp(CmdCode,ErrorAck,1);
	}


}

#endif

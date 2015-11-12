#include <reg51.h>										//include public header files
#include <string.h>
#include "type.h"
#include "utility.h"
#include "timer.h"
#include "syscfg.h"
#include "sysctrl.h"
#include "key.h"
#include "debug.h"
#include "message.h"
#include "Uart.h"
#include "slave_cmd.h"
#include "slave_ctrl.h"
#include "Bluetooth_ctrl.h"
#include "bt.h"
//#if (defined(FUNC_BLUETOOTH_CJ_EN) && (!defined(AU6210K_NR_D_8_CSRBT)))   //csr蓝牙不用发命令
#if 0//(defined(FUNC_BLUETOOTH_CJ_EN) && (!defined(AU6210K_NR_D_8_CSRBT))) //此文件是用来做串口发命令控制蓝牙的案子 比如创捷

//蓝牙串口通讯指令
static BYTE CMD_RECONNECT[7] = {0XAA, 0X00, 0X03, 0X02, 0X00, 0X16, 0XE5};
static BYTE CMD_PLAY_PAUSE[7] = {0XAA, 0X00, 0X03, 0X02, 0X00, 0X32, 0XC9};
static BYTE CMD_NEXT[7] = {0XAA, 0X00, 0X03, 0X02, 0X00, 0X34, 0XC7};
static BYTE CMD_PRE[7] = {0XAA, 0X00, 0X03, 0X02, 0X00, 0X35, 0XC6};
static BYTE CMD_VOLUP[7] = {0XAA, 0X00, 0X03, 0X02, 0X00, 0X30, 0XCB};
static BYTE CMD_VOLDOWN[7] = {0XAA, 0X00, 0X03, 0X02, 0X00, 0X31, 0XCA};
static BYTE CMD_ANSWERCALL[7] = {0XAA, 0X00, 0X03, 0X02, 0X00, 0X04, 0XF7};
static BYTE CMD_REJECTCALL[7] = {0XAA, 0X00, 0X03, 0X02, 0X00, 0X05, 0XF6};
static BYTE CMD_HUNGUPCALL[7] = {0xAA, 0x00, 0x03, 0x02, 0x00, 0x06, 0xF5};
static BYTE CMD_REDIAL[7] = {0xAA, 0x00, 0x03, 0x02, 0x00, 0x0C, 0xEF};
static BYTE CMD_DISCONNECT[7] = {0xAA, 0x00, 0x03, 0x02, 0x00, 0x3B, 0xC0};

//蓝牙应答
static BYTE RESP_BT_CONNECTED[] = {0xAA, 0x00, 0x03, 0x01, 0x05};//, link_info:0x00 ,校验0xF7
static BYTE RESP_test_RESP[] = {0xAA, 0x00, 0x02, 0x00, 0x02};//

//#define MAX_CMD_DATA_LEN	16						//命令数据域最大长度
//#define RX_QUEUE_LENGTH 	(MAX_CMD_DATA_LEN + 10)		//串口中断接收缓冲区长度

//static BYTE RxQueue[RX_QUEUE_LENGTH];
//static BYTE RxQueueHead;
//static BYTE RxQueueTail;

//BYTE gBluetoothCmd[MAX_CMD_DATA_LEN + 1];					//[命令长度域、命令码域] + 1Byte校验码域
static BYTE BluetoothRxIndex = -2;
static BYTE BluetoothHead = 0;

//static BYTE BluetoothConnectedCount = 0;

//extern BOOL PlayPhoneNumBreakoff;
//extern BYTE PlayPairFlag;
extern BYTE USEGetRxQueueLen();
extern BYTE GetRxQueueData();
BOOL BluetoothRcvCmd();
VOID BluetoothSendCmd(BYTE* cmd, BYTE cmdLength);

/*VOID BluetoothCmdInit()
{
	RxQueueHead = 0;
	RxQueueTail = 0;
	BluetoothRxIndex = -2;
	BluetoothHead = 0;
	gBlutoothStatus = BT_IDLE;
	BluetoothConnectedCount = 0;
	//RstFastUart();
	//FastUartOpen(FAST_UART_USE_A0E2, 19200, 8);
	I51UartOpen(BAUDRATE_4800, UART_USE_A1A2);
	//Test
	//BluetoothSendCmd(CMD_PLAY_PAUSE, sizeof(CMD_PLAY_PAUSE));
}*/

VOID BluetoothCmdPlay()
{
	BluetoothSendCmd(CMD_PLAY_PAUSE, sizeof(CMD_PLAY_PAUSE));
}

VOID BluetoothCmdNext()
{
	BluetoothSendCmd(CMD_NEXT, sizeof(CMD_NEXT));
}

VOID BluetoothCmdPrev()
{
	BluetoothSendCmd(CMD_PRE, sizeof(CMD_PRE));
}

VOID BluetoothCmdVolUp()
{
	BluetoothSendCmd(CMD_VOLUP, sizeof(CMD_VOLUP));
}

VOID BluetoothCmdVowDown()
{
	BluetoothSendCmd(CMD_VOLDOWN, sizeof(CMD_VOLDOWN));
}

VOID BluetoothCmdReconnect()
{
	BluetoothSendCmd(CMD_RECONNECT, sizeof(CMD_RECONNECT));
}

VOID BluetoothCmdAnswerCall()
{
	BluetoothSendCmd(CMD_ANSWERCALL, sizeof(CMD_ANSWERCALL));
}

VOID BluetoothCmdRejectCall()
{
	BluetoothSendCmd(CMD_REJECTCALL, sizeof(CMD_REJECTCALL));
}

VOID BluetoothCmdHungUpCall()
{
	BluetoothSendCmd(CMD_HUNGUPCALL, sizeof(CMD_HUNGUPCALL));
}

VOID BluetoothCmdRedial()
{
	BluetoothSendCmd(CMD_REDIAL, sizeof(CMD_REDIAL));
}
VOID BluetoothCmdDisconnect()
{
	BluetoothSendCmd(CMD_DISCONNECT, sizeof(CMD_DISCONNECT));
}
/*VOID BluetoothPlayPhoneNum()
{s
	BYTE Cmdbuf;
	BYTE PhoneNumSum;
	BYTE BluetoothPhoneNum[MAX_CMD_DATA_LEN + 1];
	if (gBluetoothCmd[3] == 0x30)
		PhoneNumSum = 15;
	else
		PhoneNumSum = 14;
	for (Cmdbuf=0;Cmdbuf<=PhoneNumSum;Cmdbuf++)		//防止再次接收命令时gBluetoothCmd[Cmdbuf]被改变导致号码错误
	{
		BluetoothPhoneNum[Cmdbuf] = gBluetoothCmd[Cmdbuf];
	}
	for(Cmdbuf=3;Cmdbuf<=PhoneNumSum ;&& PlayPhoneNumBreakoff == FALSE;Cmdbuf++)
	{
#if defined(AU6210K_NR_D_9X_XJ_HTS)
		if (Cmdbuf == 3)
		{
			SPI_PlaySelectNum(SPIPLAY_SONG_HAVE_CALL,0);
		}
#endif
		switch(BluetoothPhoneNum[Cmdbuf])
		{
			case 0x30:
				SPI_PlaySelectNum(SPIPLAY_SONG_NUM_0,0);
				break;
			case 0x31:
				SPI_PlaySelectNum(SPIPLAY_SONG_NUM_1,0);
				break;
			case 0x32:
				SPI_PlaySelectNum(SPIPLAY_SONG_NUM_2,0);
				break;
			case 0x33:
				SPI_PlaySelectNum(SPIPLAY_SONG_NUM_3,0);
				break;
			case 0x34:
				SPI_PlaySelectNum(SPIPLAY_SONG_NUM_4,0);
				break;
			case 0x35:
				SPI_PlaySelectNum(SPIPLAY_SONG_NUM_5,0);
				break;
			case 0x36:
				SPI_PlaySelectNum(SPIPLAY_SONG_NUM_6,0);
				break;
			case 0x37:
				SPI_PlaySelectNum(SPIPLAY_SONG_NUM_7,0);
				break;
			case 0x38:
				SPI_PlaySelectNum(SPIPLAY_SONG_NUM_8,0);
				break;
			case 0x39:
				SPI_PlaySelectNum(SPIPLAY_SONG_NUM_9,0);
				break;
			default:
				break;
		}
	}
	InDacChannelSel(LINEIN_IO_TYPE); 
	SetBluetoothVolume();
	//PlayPhoneNumBreakoff = FALSE;
}*/
/*
VOID BluetoothRefreshStatus()
{
	//PlayPhoneNumBreakoff = FALSE;
	if (BluetoothRcvCmd())
	{
		switch (gBluetoothCmd[1])
		{
			case 0x01: //Connecting status
				{
					if (gBluetoothCmd[2] == 0x05)
					{
						PlayPairFlag = 0;
						gBlutoothStatus = BT_CONNECTED;
						BluetoothConnectedCount++;
						if (BluetoothConnectedCount > 2)
							BluetoothConnectedCount = 2;
						#ifdef FUNC_SPI_KEY_SOUND_EN
						if (BluetoothConnectedCount == 1)
						{
							SPI_PlaySelectNum(SPIPLAY_SONG_PHONE_ONE_CONNECTED,0);
							InDacChannelSel(LINEIN_IO_TYPE); 
							SetBluetoothVolume();
						}
						else if (BluetoothConnectedCount == 2)
						{
							SPI_PlaySelectNum(SPIPLAY_SONG_PHONE_TWO_CONNECTED,0);
							InDacChannelSel(LINEIN_IO_TYPE);	 
							SetBluetoothVolume();
						}
						#endif	
					} 
					else if (gBluetoothCmd[2] == 0x07)
					{
						#ifdef FUNC_SPI_KEY_SOUND_EN
						if (BluetoothConnectedCount == 2)
						{
							SPI_PlaySelectNum(SPIPLAY_SONG_PHONE_ONE_DISCONNECTED,0);
							InDacChannelSel(LINEIN_IO_TYPE); 
							SetBluetoothVolume();
						}
						else if (BluetoothConnectedCount == 1)
						{
							SPI_PlaySelectNum(SPIPLAY_SONG_PHONE_TWO_DISCONNECTED,0);
							InDacChannelSel(LINEIN_IO_TYPE);
							SetBluetoothVolume();
						}
						#endif	
						if (BluetoothConnectedCount > 0)
							BluetoothConnectedCount--;
						if (BluetoothConnectedCount == 0)
						{
							gBlutoothStatus = BT_IDLE;
						}
					}
				}
				break;
			case 0x02:	//Call status
				{
					if (gBluetoothCmd[2] == 0x00 || gBluetoothCmd[2] == 0x01) //Call 
					{
						if (gBluetoothCmd[3] == 0x02) //Incoming call
						{
							gBlutoothStatus = BT_INCOMING_CALL;
						}
						else if (gBluetoothCmd[3] == 0x03)
						{
							gBlutoothStatus = BT_MAKE_CALL;
						}
						else if (gBluetoothCmd[3] == 0x04) //Answer call
						{
							gBlutoothStatus = BT_IN_THE_CALL;
							//PlayPhoneNumBreakoff = TRUE;
						}	
						else if (gBluetoothCmd[3] == 0x00) //End call
						{
							gBlutoothStatus = BT_CONNECTED;
							//PlayPhoneNumBreakoff = TRUE;
						}
					} 
				}
				break;
			case 0x03:	//phone num
				{
					if (gBluetoothCmd[2] == 0x00)
					{
						gBlutoothStatus = BT_INCOMING_CALL;
						//PlayPhoneNumBreakoff = FALSE;
					#ifdef FUNC_SPI_KEY_SOUND_EN
						BluetoothPlayPhoneNum();
					#endif
					}
				}
				break;
			default:
				break;
		}
	}
}*/

//#pragma disable 


/*BYTE GetRxQueueLen()
{
	return ((RxQueueTail + RX_QUEUE_LENGTH - RxQueueHead) % RX_QUEUE_LENGTH);
}*/





/*BOOL BluetoothRcvCmd()
{
	BYTE Temp;
	BYTE CheckSum;
	BYTE i;
	TIMER Timer;

	TimeOutSet(&Timer, 500);
	while(USEGetRxQueueLen() > 0)
	{	
		if(IsTimeOut(&Timer))
		{
			//DBG(("timeout\n"));
			return FALSE;
		}		
		Temp = GetRxQueueData();	
		if(BluetoothRxIndex == -2)
		{
			if(Temp==0xAA)
			{
				//FastUartSendByte(0xF1);
				BluetoothRxIndex =-1;
			}
		}
		else if(BluetoothRxIndex == -1)
		{
			if(Temp==0x00)
			{
				//FastUartSendByte(0xF2);
				BluetoothRxIndex = 0;
			}
		}
		else 
		{	
			//SlaveHead=0;
			gBluetoothCmd[BluetoothRxIndex++] = Temp;		//CmdDataLength + CmdData[] + CRC
			//如果长度域超过最大长度，则重新接收
			if((gBluetoothCmd[0] > MAX_CMD_DATA_LEN))
			{		
				BluetoothRxIndex = -2;
				continue;
			}
			//FastUartSendByte(0xF0);
			//FastUartSendByte(Temp);
			if(BluetoothRxIndex >= gBluetoothCmd[0] + 2)	//CS接收完
			{
				CheckSum = 0;
				for (i=0; i<BluetoothRxIndex-1; i++)
				{
					CheckSum += gBluetoothCmd[i];
				}
				CheckSum = 0xFF - CheckSum + 1;
				BluetoothRxIndex = -2;	
				//FastUartSendByte(0xFF);
				return TRUE;  //如需要CheckSum则注释此行
				if(CheckSum == gBluetoothCmd[BluetoothRxIndex-1])
				{
					return TRUE;
				}
				else
				{

	 				return FALSE;
				}			
			}
		}
	}
	return FALSE;
}*/

VOID BluetoothSendCmd(BYTE* cmd, BYTE cmdLength)
{
	int i;
	for (i=0; i<cmdLength; i++)
	{
		I51UartSendByte(cmd[i]);
	}
}


#endif

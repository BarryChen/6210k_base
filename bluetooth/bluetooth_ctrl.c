#include "syscfg.h"
#include "sysctrl.h"
#include "device.h"
#include "timer.h"
#include "message.h"
#include "debug.h"
#include "key.h"
#include "utility.h"
#include "display.h"
#include "breakpoint.h"	  
#include "user_interface.h"
#include "devctrl.h"
#include "bluetooth_ctrl.h"
#include "Sled_disp.h"

#if 0//def FUNC_BLUETOOTH_EN

//�������Ʒ�ʽ��0Ϊ�����������ƣ�1Ϊ��������Ϣ������������ģ��������
#define		VOLUMN_ADJUST_BY_MCU	0
#define		VOLUMN_ADJUST_BY_BT		1
#define		VOLUMN_ADJUST_METHOD	VOLUMN_ADJUST_BY_MCU //OR VOLUMN_ADJUST_BY_BT

#define		BT_KEYSP_delay		300		//�̰�
#define		BT_KEYCPS_delay		1000  	//����
#define		BT_ACTIVE_AFTERWARE	500


BYTE CODE gBTInGain[VOLUME_MAX + 1] = 
{
#if (VOLUME_MAX == 16)
#ifdef AU6210K_MZBT_SH_01
	63,		56,		51,		47,		43, 	39, 	35, 	32,		
	29,		26,		23,		20,		18,		16,		15,		14,		
	13		
#else
	63,		51,		39,		29,		20,		18,		16,		15,
	14,		13,		12,		11,		10,		9,		8,		7,
	6
#endif	
#else
	63, 
	62, 	59, 	57, 	55,		53,		51,		49,		47, 	
	45, 	43, 	41, 	39,     37, 	35, 	33, 	31,
	29,		27,		25,  	23, 	21, 	20, 	19,  	18,  
	17,  	16,  	15,  	14, 	13,  	12,  	11,  	10
#endif
};

TIMER	BT_Statup;
TIMER 	BT_Key;

static BYTE justEnter;

#ifdef BLUETOOTH_ALWAYS_OPEN
#define BT_SCAN_BIT			1<<2

#define BT_PHONE_STATUS_PU	GPIO_B_PU
#define BT_PHONE_STATUS_PD	GPIO_B_PD
#define BT_PHONE_STATUS_PU	GPIO_B_PU
#define BT_PHONE_STATUS_OE	GPIO_B_OE
#define BT_PHONE_STATUS_IE	GPIO_B_IE
#define BT_PHONE_STATUS_IN	GPIO_B_IN
#define BT_PHONE_STATUS_OUT	GPIO_B_OUT
#if 0
#define BT_PHONE_STATUS_BIT	1<<1
#else
#define BT_PHONE_STATUS_BIT	0x01
#endif
//����и�������ã����Բο�832�Ĵ���

BOOL gBTConnected = FALSE;
BOOL gBTPhoneInCall = FALSE;

#define BLUETOOTH_OFF_TIME 600000      //10mins
TIMER	gBluetoothOffTimer;

#define		BT_STATUS_SAMPLE_PERIOD	50
#define		BT_STATUS_SAMPLE_COUNT	20
TIMER	BTStateMonitorTimer;
WORD	BTStateConnectedMonitorCount = BT_STATUS_SAMPLE_COUNT;
WORD	BTStateCallInMonitorCount = BT_STATUS_SAMPLE_COUNT;
#ifdef AU6210K_MZBT_SH_01
#define BluetoothPhoneVolume	10
static BYTE sSysVolumeBackup;
#endif

#endif

#ifdef RDA_BLUETOOTH
//TIMER rdaBluetoothPowerOnTimer;
//BOOL rdaBluetoothPowerOnInProcess = FALSE;
VOID RDABluettothPowerOn()
{
	//RDA��������POWERKEY_INT��һ��ʱ��ߵ�ƽ�������ٸ�һ��ʱ��ߵ�ƽ�ػ�
	DBG(("RDABluettothPowerOn\n"));
	//rdaBluetoothPowerOnInProcess = TRUE;
	BT_POWER_H();
	WaitMs(RDABluetoothPowerOn_DELAY);
	BT_POWER_L();
	//TimeOutSet(&rdaBluetoothPowerOnTimer, RDABluetoothPowerOn_DELAY);
}
#endif


#ifdef RDA_BLUETOOTH_DET
BOOL BluetoothIsOpened()
{
	BOOL opened;
	SetGpioRegBit(RDABT_DET_PU, RDABT_DET_BIT);
	SetGpioRegBit(RDABT_DET_PD, RDABT_DET_BIT);
	ClrGpioRegBit(RDABT_DET_OE, RDABT_DET_BIT);
	SetGpioRegBit(RDABT_DET_IE, RDABT_DET_BIT);
	WaitUs(2);

	opened = (GetGpioReg(RDABT_DET_IN) & RDABT_DET_BIT);
	DBG(("Bluetooth opened:%bu\n", opened));
	return opened;
}
#endif

BOOL BluetoothCtrlInit()
{
	//SET_BT_GPIO_OUTPUT();
	DBG(("BluetoothCtrlInit entry.\n"));
	//����CSR������Ҫ����ɨ��ģʽ���е�IO��Ҫ����	
	BT_NEXT_L();
	BT_PREV_L();
	BT_PLAY_L();

#if (VOLUMN_ADJUST_METHOD == VOLUMN_ADJUST_BY_MCU)
	InDacLineInGain(gBTInGain[gSys.Volume], gBTInGain[gSys.Volume]);
#else
	InDacLineInGain(gBTInGain[VOLUME_MAX], gBTInGain[VOLUME_MAX]); //������������ʱ�������������������
#endif

	justEnter = 1;
#ifdef RDA_BLUETOOTH
#ifdef RDA_BLUETOOTH_DET
	if (!BluetoothIsOpened())
#endif
	{
		RDABT_POWER_HOLD_H();
		RDABluettothPowerOn();
		TimeOutSet(&BT_Statup, BT_ACTIVE_AFTERWARE); //��������BT_PREV_L�����Ļָ��ȱ�RDAģ���������ų���			
	}
#else
#ifndef BLUETOOTH_ALWAYS_OPEN
	BT_POWER_L();//������POWER��Ȼ���ڼ�ʱ�������������������ܽ�������ɨ��ģʽ
#endif
	TimeOutSet(&BT_Statup, BT_ACTIVE_AFTERWARE);
#endif

#ifdef FUNC_BREAK_POINT_EN
	BP_SaveInfo(&gBreakPointInfo.PowerMemory.SystemMode, sizeof(gBreakPointInfo.PowerMemory.SystemMode));
#endif

//��������ʱ����������˸����Ϊ����̬����������ģ�鴦����
#if 0
				SetGpioRegBit(SLED_PORT_IE,(1 << 4));
				ClrGpioRegBit(SLED_PORT_OE,(1 << 4));
				SetGpioRegBit(SLED_PORT_PU,(1 << 4));
				ClrGpioRegBit(SLED_PORT_PD,(1 << 4));
#elif defined(AU6210K_HXX_B002)
				SetGpioRegBit(GPIO_D_IE,(1 << 4));
				ClrGpioRegBit(GPIO_D_OE,(1 << 4));
				SetGpioRegBit(GPIO_D_PU,(1 << 4));
				ClrGpioRegBit(GPIO_D_PD,(1 << 4));
#endif
	return TRUE;
}

VOID BluetothCtrlEnd()
{
	DBG(("BluetoothCtrlEnd entry.\n"));
	BT_NEXT_L();
	BT_PLAY_L();
	BT_PREV_L();	
#ifdef RDA_BLUETOOTH
	RDABT_POWER_HOLD_L();//���ý�Ϊ�ͣ�RDA����ģ����Զ��ر��Լ�
#else
#ifndef BLUETOOTH_ALWAYS_OPEN
	BT_POWER_L();
#endif
#endif

//�����ر�ʱ����������˸�����ԭ״̬
#if 0
	SLedInit();
#elif defined(AU6210K_HSWY_001)
	SetGpioRegBit(GPIO_D_PD, (1 << 4));		// Pull-Down.
	SetGpioRegBit(GPIO_D_PU, (1 << 4));		// Pull-Down.
	ClrGpioRegBit(GPIO_D_IE, (1 << 4));		// Input Disable
	SetGpioRegBit(GPIO_D_OE, (1 << 4));		// Output Enable. // D[2:7]
	SetGpioRegBit(GPIO_D_OUT, (1 << 4));		// Output 0
#endif

}

VOID SetBluetoothVolume(BYTE vol)
{
	InDacLineInGain(gBTInGain[vol], gBTInGain[vol]);
}

// line-in mode state control.
VOID BluetoothStateCtrl()
{
	MESSAGE msg;

	if (IsTimeOut(&BT_Key)) 
	{
		BT_NEXT_L();
		BT_PREV_L();
		BT_PLAY_L();
	}

#ifdef RDA_BLUETOOTH //RDA POWERKEY_INT��һ��ʱ��Ϊ��
	/*if (IsTimeOut(&rdaBluetoothPowerOnTimer) && rdaBluetoothPowerOnInProcess)
	{
		rdaBluetoothPowerOnInProcess = FALSE;
		BT_POWER_L();
	}*/
#endif
	
	if (IsTimeOut(&BT_Statup) && justEnter)
	{
		DBG(("active bluetooth time out.\n"));
		justEnter = 0;
#ifdef RDA_BLUETOOTH
#else
#ifndef BLUETOOTH_ALWAYS_OPEN
		BT_POWER_H();
#endif
#endif

		UnMute();
		gSys.MuteFg = FALSE;
	}

	msg = MessageGet(MSG_FIFO_KEY);
	switch(msg)
	{
		case MSG_PLAY_PAUSE:	
			{
				BT_PLAY_H();
				TimeOutSet(&BT_Key,BT_KEYSP_delay); 
				DBG(("MSG_PLAY_PAUSE\n"));
			}	
			break;
		
		case MSG_BLUETOOTH_ANSWER:
			{
				BT_PLAY_H();
				TimeOutSet(&BT_Key, BT_KEYSP_delay); 
				DBG(("MSG_BLUETOOTH_ANSWER\n"));
			}
			break;
#if defined(AU6210K_HXX_B002)
		case MSG_SCAN:
#endif
		case MSG_BLUETOOTH_REJECT:
			{
				BT_PLAY_H();
				TimeOutSet(&BT_Key, BT_KEYCPS_delay); 
				DBG(("MSG_BLUETOOTH_REJECT\n"));
			}
			break;
		case MSG_NEXT:	
			{
				BT_NEXT_H();
				TimeOutSet(&BT_Key,BT_KEYSP_delay); 
				DBG(("MSG_NEXT\n"));
			}	
			break;
		case MSG_PRE:	
			{
				BT_PREV_H();
				TimeOutSet(&BT_Key,BT_KEYSP_delay); 
				DBG(("MSG_PRE\n"));
			}	
			break;
		case MSG_VOL_ADD:
			{
			#if (VOLUMN_ADJUST_METHOD == VOLUMN_ADJUST_BY_MCU)
				//ʹ�����ص���������
				VolumeAdjust(UP);
			#else
				BT_NEXT_H();
				TimeOutSet(&BT_Key, BT_KEYCPS_delay);
			#endif
				DBG(("MSG_VOL_ADD\n"));	
			}			
			break;
		case MSG_VOL_SUB:
			{
			#if (VOLUMN_ADJUST_METHOD == VOLUMN_ADJUST_BY_MCU)
				VolumeAdjust(DOWN);
			#else
				BT_PREV_H();
				TimeOutSet(&BT_Key, BT_KEYCPS_delay);
			#endif
				DBG(("MSG_VOL_SUB\n"));
			}
			break;
		default:
			break;
	}	
}

#ifdef BLUETOOTH_ALWAYS_OPEN
VOID BluetoothStateReset()
{
	SetGpioRegBit(GPIO_E_PU, BT_SCAN_BIT);
	SetGpioRegBit(GPIO_E_PD, BT_SCAN_BIT);
	SetGpioRegBit(GPIO_E_IE, BT_SCAN_BIT);
	ClrGpioRegBit(GPIO_E_OE, BT_SCAN_BIT);
	//ClrGpioRegBit(GPIO_E_OUT, BT_SCAN_BIT);

	SetGpioRegBit(BT_PHONE_STATUS_PU, BT_PHONE_STATUS_BIT); //Pull-up disable
	SetGpioRegBit(BT_PHONE_STATUS_PD, BT_PHONE_STATUS_BIT); //Pull-down disable
	ClrGpioRegBit(BT_PHONE_STATUS_OE, BT_PHONE_STATUS_BIT);
	SetGpioRegBit(BT_PHONE_STATUS_IE, BT_PHONE_STATUS_BIT);
	//ClrGpioRegBit(BT_PHONE_STATUS_OUT, BT_PHONE_STATUS_BIT);
	
	BT_NEXT_L();
	BT_PREV_L();
	BT_PLAY_L();
}

VOID BluetoothPowerOn()
{
	BluetoothStateReset();
	BT_POWER_H();
	TimeOutSet(&gBluetoothOffTimer, BLUETOOTH_OFF_TIME);
}

VOID BluetoothPowerOff()
{
	BluetoothStateReset();
	BT_POWER_L();
}

/*VOID BluetoothWakeupFromSelfSleep()
{
	BT_PLAY_H();
	WaitMs(BT_KEYSP_delay);
	BT_PLAY_L();
	//BT_POWER_H();
	//TimeOutSet(&BT_Key, BT_ENTER_SCAN_delay);
}*/

VOID BluetoothStateMonitor()
{
	BOOL btConnectedIconOn;
#ifdef FUNC_UARTDBG_EN
	BOOL tempBTConnected;
	BOOL tempPhoneState;
#endif
	//DBG(("BluetoothStateMonitor:"));

	//��������ģ��RLED�ƽ�����˸����ͼ��
	SetGpioRegBit(GPIO_E_PU, BT_SCAN_BIT);
	SetGpioRegBit(GPIO_E_PD, BT_SCAN_BIT);
	ClrGpioRegBit(GPIO_E_OE, BT_SCAN_BIT);
	SetGpioRegBit(GPIO_E_IE, BT_SCAN_BIT);
	WaitUs(2);
	btConnectedIconOn = (GetGpioReg(GPIO_E_IN) & BT_SCAN_BIT);//�߱�ʾ������
#ifdef FUNC_DISP_EN	
	DispBluetoothStatus(btConnectedIconOn);
#endif
	if (IsTimeOut(&BTStateMonitorTimer))
	{
		TimeOutSet(&BTStateMonitorTimer, BT_STATUS_SAMPLE_PERIOD);

#ifdef FUNC_UARTDBG_EN
		tempBTConnected = gBTConnected;
#endif
		//��������������״̬,����ʾ.����������������ʱΪ�ߣ�δ����ʱΪ��˸
		if (btConnectedIconOn)
		{
			if (--BTStateConnectedMonitorCount <= 0)
			{
				BTStateConnectedMonitorCount = BT_STATUS_SAMPLE_COUNT;
				gBTConnected = TRUE;
			}			
		}
		else
		{	
			//�����Ͽ�ʱ���������ùر�������Դʱ��
			if(gBTConnected == TRUE)
				TimeOutSet(&gBluetoothOffTimer, BLUETOOTH_OFF_TIME);
			gBTConnected = FALSE;
			BTStateConnectedMonitorCount= BT_STATUS_SAMPLE_COUNT;
		}
		if (IsTimeOut(&gBluetoothOffTimer) && gBTConnected == FALSE)
		{
			BluetoothPowerOff();
		}
#ifdef FUNC_UARTDBG_EN
		if (tempBTConnected != gBTConnected)
			DBG(("BluetoothStateMonitor: Connected:%bu\n", gBTConnected));
#endif
		//���������ĵ绰״̬��ʹ���ڲ��������е绰ʱΪ�ߣ�û�е绰ʱΪ��
		if (gBTConnected) {
#ifdef FUNC_UARTDBG_EN
			tempPhoneState = gBTPhoneInCall;
#endif
			ClrGpioRegBit(BT_PHONE_STATUS_PU, BT_PHONE_STATUS_BIT); //Pull-up disable
			ClrGpioRegBit(BT_PHONE_STATUS_PD, BT_PHONE_STATUS_BIT); //Pull-down disable
			ClrGpioRegBit(BT_PHONE_STATUS_OE, BT_PHONE_STATUS_BIT);
			SetGpioRegBit(BT_PHONE_STATUS_IE, BT_PHONE_STATUS_BIT);
			WaitUs(2);
			if (!(GetGpioReg(BT_PHONE_STATUS_IN) & BT_PHONE_STATUS_BIT)) //�ͱ�ʾ�绰����
			{
				if (--BTStateCallInMonitorCount <= 0)
				{
#ifdef AU6210K_MZBT_SH_01
					if(gBTPhoneInCall == FALSE && gSys.Volume > BluetoothPhoneVolume)
					{
						sSysVolumeBackup = gSys.Volume;
						gSys.Volume = BluetoothPhoneVolume;
						SetBluetoothVolume(BluetoothPhoneVolume);
					}
#endif
				
					BTStateCallInMonitorCount = BT_STATUS_SAMPLE_COUNT;
					gBTPhoneInCall = TRUE;
				}
			}
			else
			{
#ifdef AU6210K_MZBT_SH_01
				if(gBTPhoneInCall == TRUE && sSysVolumeBackup > BluetoothPhoneVolume)
				{
					gSys.Volume = sSysVolumeBackup;
					SetBluetoothVolume(gSys.Volume);
				}
#endif
				BTStateCallInMonitorCount = BT_STATUS_SAMPLE_COUNT;
				gBTPhoneInCall = FALSE;				
			}
#ifdef FUNC_UARTDBG_EN
			if (tempPhoneState!= gBTPhoneInCall)
				DBG(("BluetoothStateMonitor: phoneState=%bu\n", gBTPhoneInCall));
#endif
		}
	}
	//DBG(("\n"));
}

#endif

#endif
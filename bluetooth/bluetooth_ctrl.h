#ifndef _BLUETOOTH_CTRL_H_
#define _BLUETOOTH_CTRL_H_

#if 0

#define BT_PLAY_L()        SetGpioRegBit(GPIO_B_PU, 1<<2),\	
						   SetGpioRegBit(GPIO_B_PD, 1<<2),\	
						   SetGpioRegBit(GPIO_B_OE, 1<<2),\	
	                       ClrGpioRegBit(GPIO_B_OUT, 1<<2);

#define BT_PLAY_H()        SetGpioRegBit(GPIO_B_OE, 1<<2),\	
	                       SetGpioRegBit(GPIO_B_OUT, 1<<2);


#define BT_PREV_L()        SetGpioRegBit(GPIO_B_PU, 1<<3),\	
						   SetGpioRegBit(GPIO_B_PD, 1<<3),\	
						   SetGpioRegBit(GPIO_B_OE, 1<<3),\	
	                       ClrGpioRegBit(GPIO_B_OUT, 1<<3);
#define BT_PREV_H()        SetGpioRegBit(GPIO_B_OE, 1<<3),\	
	                       SetGpioRegBit(GPIO_B_OUT, 1<<3);

#define BT_NEXT_L()        SetGpioRegBit(GPIO_B_PU, 1<<1),\	
						   SetGpioRegBit(GPIO_B_PD, 1<<1),\	
						   SetGpioRegBit(GPIO_B_OE, 1<<1),\	
	                       ClrGpioRegBit(GPIO_B_OUT, 1<<1);
#define BT_NEXT_H()        SetGpioRegBit(GPIO_B_OE, 1<<1),\	
	                       SetGpioRegBit(GPIO_B_OUT, 1<<1);

#define BT_POWER_L()       SetGpioRegBit(GPIO_B_PU, 1<<0),\	
						   SetGpioRegBit(GPIO_B_PD, 1<<0),\	
						   SetGpioRegBit(GPIO_B_OE, 1<<0),\	
                   		   ClrGpioRegBit(GPIO_B_OUT, 1<<0);
#define BT_POWER_H()       SetGpioRegBit(GPIO_B_OE, 1<<0),\	
	                       SetGpioRegBit(GPIO_B_OUT, 1<<0);

#ifdef RDA_BLUETOOTH
#define RDABluetoothPowerOn_DELAY 380 //ms 
//RDA蓝牙开机状态维持脚，当这个脚为低时，RDA模块会自己关闭;其与BT_POWER_H的区别是:BT_POWER_H用来给一个短高脉冲开关机，平时只是低电平
#define RDABT_POWER_HOLD_L()	SetGpioRegBit(GPIO_A_PU, 1<<6),\	
								SetGpioRegBit(GPIO_A_PD, 1<<6),\	
								SetGpioRegBit(GPIO_A_OE, 1<<6),\	
		                   		ClrGpioRegBit(GPIO_A_OUT, 1<<6);
#define RDABT_POWER_HOLD_H()    SetGpioRegBit(GPIO_A_OE, 1<<6),\	
		                        SetGpioRegBit(GPIO_A_OUT, 1<<6);
#define RDABT_DET_PU		GPIO_A_PU
#define RDABT_DET_PD		GPIO_A_PD
#define RDABT_DET_OE		GPIO_A_OE
#define RDABT_DET_IE		GPIO_A_IE
#define RDABT_DET_IN		GPIO_A_IN
#define RDABT_DET_BIT		1<<7
#endif

#ifdef BLUETOOTH_ALWAYS_OPEN
extern BOOL gBTConnected;
extern BOOL gBTPhoneInCall;

//当蓝牙进入休眠后，调用这个将重新进入扫描
//VOID BluetoothWakeupFromSelfSleep();
VOID BluetoothPowerOff();
VOID BluetoothPowerOn();
//监控蓝牙状态，如果蓝牙处于电话状态
VOID BluetoothStateMonitor();
#endif

BOOL BluetoothCtrlInit();
VOID BluetothCtrlEnd();
// bluetooth mode state control.
VOID BluetoothStateCtrl();
VOID SetBluetoothVolume(BYTE vol);
#endif
#endif


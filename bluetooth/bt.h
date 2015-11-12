#ifndef __BT_CTRL_H__
#define __BT_CTRL_H__


#ifdef FUNC_BLUETOOTH_CSR_EN
//蓝牙GPIO口控制

#define		BT_POWER_L()	SetGpioRegBit(GPIO_B_OE,	1<<7),\	
	                       	ClrGpioRegBit(GPIO_B_OUT, 	1<<7);

#define 	BT_POWER_H()  	SetGpioRegBit(GPIO_B_OE, 	1<<7),\	
	                        SetGpioRegBit(GPIO_B_OUT, 	1<<7);

//////////////////////////////////////////////////////////////
//灯设置状态 从MCU输出的口
//////////////////////////////////////////////////////////////
#define		LED_BLUE_H()    SetGpioRegBit(GPIO_A_OUT, 	1<<0);
#define		LED_BLUE_L()    ClrGpioRegBit(GPIO_A_OUT, 	1<<0);    

#define		LED_RED_H()     SetGpioRegBit(GPIO_B_OUT, 	1<<5);
#define		LED_RED_L()     ClrGpioRegBit(GPIO_B_OUT, 	1<<5);

#endif

//////////////////////////////////////////////////////////////
//在进入蓝牙模式的时候，io口初始化 //不同的脚位请用宏控制好
//////////////////////////////////////////////////////////////
#ifdef FUNC_SINGLE_LED_HIGHON_EN//高亮，低灭，下拉
#define BTIO_PORT1_INIT()/*A0 灯*/	do{ \
    SetGpioRegBit(GPIO_A_PU, (1<<0));	\	
	SetGpioRegBit(GPIO_A_PD, (1<<0));	\	
	ClrGpioRegBit(GPIO_A_IE, (1<<0));	\	
	SetGpioRegBit(GPIO_A_OE, (1<<0));	\
	}while(0)
#else
#define BTIO_PORT1_INIT()/*A0 灯*/	do{ \
    SetGpioRegBit(GPIO_A_PU, (1<<0));	\	
	ClrGpioRegBit(GPIO_A_PD, (1<<0));	\	
	ClrGpioRegBit(GPIO_A_IE, (1<<0));	\	
	SetGpioRegBit(GPIO_A_OE, (1<<0));	\
	}while(0)
#endif

#define BTIO_PORT2_INIT()/*A1 is CSR's PIO15 */	do{ \	
	SetGpioRegBit(GPIO_A_PU, (1 << 1));	\	
    SetGpioRegBit(GPIO_A_PD, (1 << 1));	\	
    ClrGpioRegBit(GPIO_A_IE, (1 << 1));	\	
	SetGpioRegBit(GPIO_A_OE, (1 << 1));\
	ClrGpioRegBit(GPIO_A_OUT, (1 << 1));\
	}while(0)

#define BTIO_PORT3_INIT()/*A2 is CSR's PIO14*/	do{ \	
	SetA2IcsReg(A2_ICS_FOR_GPIO, A2_ICS_CLOSE); \
	SetGpioRegBit(GPIO_A_PU, (1 << 2));		\
    SetGpioRegBit(GPIO_A_PD, (1 << 2));		\
    ClrGpioRegBit(GPIO_A_IE, (1 << 2));		\
	SetGpioRegBit(GPIO_A_OE, (1 << 2));\
	ClrGpioRegBit(GPIO_A_OUT, (1 << 2));\
	}while(0)


#ifdef FUNC_SINGLE_LED_HIGHON_EN//高亮，低灭，下拉
#define BTIO_PORT4_INIT()/*b5 灯*/	do{ \
    SetGpioRegBit(GPIO_B_PU, (1<<5));	\	
	SetGpioRegBit(GPIO_B_PD, (1<<5));	\	
	ClrGpioRegBit(GPIO_B_IE, (1<<5));	\	
	SetGpioRegBit(GPIO_B_OE, (1<<5));	\
	}while(0)
#else
#ifdef AU6210K_ZB_BT007_CSR
#define BTIO_PORT4_INIT()/*b5 灯*/	do{ \
    ClrGpioRegBit(GPIO_B_PU, (1<<5));	\	
	ClrGpioRegBit(GPIO_B_PD, (1<<5));	\	
	ClrGpioRegBit(GPIO_B_IE, (1<<5));	\	
	SetGpioRegBit(GPIO_B_OE, (1<<5));	\
	SetGpioRegBit(GPIO_B_OUT, (1<<5));	\
	}while(0)
#else
#define BTIO_PORT4_INIT()/*b5 灯*/	do{ \
    ClrGpioRegBit(GPIO_B_PU, (1<<5));	\	
	ClrGpioRegBit(GPIO_B_PD, (1<<5));	\	
	ClrGpioRegBit(GPIO_B_IE, (1<<5));	\	
	SetGpioRegBit(GPIO_B_OE, (1<<5));	\
	SetGpioRegBit(GPIO_B_OUT, (1<<5));	\
	}while(0)

#endif
#endif


#define BTIO_PORT5_INIT()/*D2 is CSR's PIO7*/		do{ \	
	SetGpioRegBit(GPIO_D_PU, (1 << 2));		\
    SetGpioRegBit(GPIO_D_PD, (1 << 2));		\
    ClrGpioRegBit(GPIO_D_IE, (1 << 2));		\
	SetGpioRegBit(GPIO_D_OE, (1 << 2));\
	ClrGpioRegBit(GPIO_D_OUT, (1 << 2));\
	}while(0)

#define BTIO_PORT6_INIT()/*D5 is CSR's PIO18*/    	do{ \
	SetGpioRegBit(GPIO_D_PU, (1 << 5));		\
    SetGpioRegBit(GPIO_D_PD, (1 << 5));		\
    ClrGpioRegBit(GPIO_D_IE, (1 << 5));		\
	SetGpioRegBit(GPIO_D_OE, (1 << 5));\
	ClrGpioRegBit(GPIO_D_OUT, (1 << 5));\
	}while(0)

#define BTIO_PORT7_INIT()/*D6 is CSR's PIO21*/	do{ \	
	SetGpioRegBit(GPIO_D_PU, (1 << 6));		\
    SetGpioRegBit(GPIO_D_PD, (1 << 6));		\
    ClrGpioRegBit(GPIO_D_IE, (1 << 6));		\
	SetGpioRegBit(GPIO_D_OE, (1 << 6));\
	ClrGpioRegBit(GPIO_D_OUT, (1 << 6));\
	}while(0)


/////////////////////////////////////////////////////////IO_PORT_END
#define BTIO_PORT1_END()/*A0 */	do{ \
   	SetGpioRegBit(GPIO_A_PU, (1<<0));	\	
	SetGpioRegBit(GPIO_A_PD, (1<<0));	\	
	ClrGpioRegBit(GPIO_A_IE, (1<<0));	\	
	SetGpioRegBit(GPIO_A_OE, (1<<0));	\
	}while(0)

#define BTIO_PORT2_END()/*A1*/	do{ \	
	SetGpioRegBit(GPIO_A_PU, (1 << 1));	\	
    SetGpioRegBit(GPIO_A_PD, (1 << 1));	\	
    ClrGpioRegBit(GPIO_A_IE, (1 << 1));	\	
	SetGpioRegBit(GPIO_A_OE, (1 << 1));\
	ClrGpioRegBit(GPIO_A_OUT, (1 << 1));\
	}while(0)

#define BTIO_PORT3_END()/*A2*/	do{ \	
	SetA2IcsReg(A2_ICS_FOR_GPIO, A2_ICS_CLOSE); \
	SetGpioRegBit(GPIO_A_PU, (1 << 2));		\
    SetGpioRegBit(GPIO_A_PD, (1 << 2));		\
    ClrGpioRegBit(GPIO_A_IE, (1 << 2));		\
	SetGpioRegBit(GPIO_A_OE, (1 << 2));\
	ClrGpioRegBit(GPIO_A_OUT, (1 << 2));\
	}while(0)

#define BTIO_PORT4_END()/*B5*/	do{ \	
	SetGpioRegBit(GPIO_B_PU, (1<<5));		\
	SetGpioRegBit(GPIO_B_PD, (1<<5));		\
	ClrGpioRegBit(GPIO_B_IE, (1<<5));		\
	SetGpioRegBit(GPIO_B_OE, (1<<5));\
    SetGpioRegBit(GPIO_B_OUT,(1<<5));\
	}while(0)


#define BTIO_PORT5_END()/*D2*/		do{ \	
	SetGpioRegBit(GPIO_D_PU, (1 << 2));		\
    SetGpioRegBit(GPIO_D_PD, (1 << 2));		\
    ClrGpioRegBit(GPIO_D_IE, (1 << 2));		\
	SetGpioRegBit(GPIO_D_OE, (1 << 2));\
	ClrGpioRegBit(GPIO_D_OUT, (1 << 2));\
	}while(0)

#define BTIO_PORT6_END()/*D5*/    	do{ \
	SetGpioRegBit(GPIO_D_PU, (1 << 5));	\	
    SetGpioRegBit(GPIO_D_PD, (1 << 5));	\	
    ClrGpioRegBit(GPIO_D_IE, (1 << 5));	\	
	SetGpioRegBit(GPIO_D_OE, (1 << 5));\
	ClrGpioRegBit(GPIO_D_OUT, (1 << 5));\
	}while(0)

#define BTIO_PORT7_END()/*D6*/	do{ \	
	SetGpioRegBit(GPIO_D_PU, (1 << 6));	\	
    SetGpioRegBit(GPIO_D_PD, (1 << 6));	\	
    ClrGpioRegBit(GPIO_D_IE, (1 << 6));	\	
	SetGpioRegBit(GPIO_D_OE, (1 << 6));\
	ClrGpioRegBit(GPIO_D_OUT, (1 << 6));\
	}while(0)
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
//灯 输入在这里设置
//////////////////////////////////////////////////////////////////////
#define BTIO_PORT_BLUE_LED_INIT()/*D0*/	do{ \	
	SetGpioRegBit(GPIO_D_PU, (1 << 0));\
	ClrGpioRegBit(GPIO_D_PD, (1 << 0));	\
	ClrGpioRegBit(GPIO_D_OE, (1 << 0));\
	SetGpioRegBit(GPIO_D_IE, (1 << 0));\
	}while(0)
#define GET_BLUE_LED_STATUS()/*D0*/	 \	
	GetGpioReg(GPIO_D_IN) & (1 << 0)


#define BTIO_PORT_RED_LED_INIT()/*D1*/	do{ \	
	SetGpioRegBit(GPIO_D_PU, (1 << 1));\
	ClrGpioRegBit(GPIO_D_PD, (1 << 1));	\
	ClrGpioRegBit(GPIO_D_OE, (1 << 1));\
	SetGpioRegBit(GPIO_D_IE, (1 << 1));\
	}while(0)
#define GET_RED_LED_STATUS()/*D1*/	\	
	GetGpioReg(GPIO_D_IN) & (1 << 1)



//////////////////////////////////////////////////////////////////////
//mute设置
//////////////////////////////////////////////////////////////////////

#ifdef AU6210K_LK_SJ_CSRBT

#define BTIO_PORT_MUTE_INIT()/*B6*/	do{ \	
	SetGpioRegBit(GPIO_A_PU, (1 << 7));\
	ClrGpioRegBit(GPIO_A_PD, (1 << 7));	\
	ClrGpioRegBit(GPIO_A_OE, (1 << 7));\
	SetGpioRegBit(GPIO_A_IE, (1 << 7));\
	}while(0)
#define GET_BT_MUTE_STATUS()/*B6*/	\	
	GetGpioReg(GPIO_A_IN) & (1 << 7)
	

#elif defined(AU6210K_ZB_BT007_CSR) 

#define BTIO_PORT_MUTE_INIT()/*B5*/	do{ \	
	SetGpioRegBit(GPIO_A_IE, (1 << 7));\
	ClrGpioRegBit(GPIO_A_OE, (1 << 7));\
	SetGpioRegBit(GPIO_A_PU, (1 << 7));\
	SetGpioRegBit(GPIO_A_PD, (1 << 7));	\
	}while(0)
#define GET_BT_MUTE_STATUS()/*B5*/	\	
	GetGpioReg(GPIO_A_IN) & (1 << 7)
		
#else //纳瑞csr+6210k+dsp/pt2313 D8案子是用B5 ,而B6做电压检测了
#define BTIO_PORT_MUTE_INIT()/*B5*/	do{ \	
	ClrGpioRegBit(GPIO_B_PU, (1 << 5));\
	ClrGpioRegBit(GPIO_B_PD, (1 << 5));	\
	ClrGpioRegBit(GPIO_B_OE, (1 << 5));\
	SetGpioRegBit(GPIO_B_IE, (1 << 5));\
	}while(0)
#define GET_BT_MUTE_STATUS()/*B5*/	\	
	GetGpioReg(GPIO_B_IN) & (1 << 5)
#endif
	
VOID LineInCtrlInit(VOID);

VOID LineinIODeinit(VOID);

// line-in mode state control.
VOID LineInStateCtrl(VOID);
typedef enum _BLUETOOTH_STATUS {
	BT_IDLE = 0,
	BT_CONNECTED,
	BT_INCOMING_CALL,
	BT_MAKE_CALL,
	BT_IN_THE_CALL,
}BLUETOOTH_STATUS;
extern BLUETOOTH_STATUS gBlutoothStatus;
VOID BluetoothCtrlInit();
VOID BluetothCtrlEnd();
// bluetooth mode state control.
VOID BluetoothStateCtrl();
VOID SetBluetoothVolume(BYTE Vol);
BOOL BTisMute();
#endif

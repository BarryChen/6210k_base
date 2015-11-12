#include<string.h>
#include <reg51.h>
#include <intrins.h>
#include "syscfg.h"
#include "type.h"
#include "utility.h"
#include "debug.h"
#include "i2c.h"
#include "pt231x.h"
#include "display.h"
#ifdef FUNC_PT231X_EN

BYTE CODE VOL_Step_Map[]=
{	
	0x3f,	// 0
	0x38,	// 1
	0x34,	// 2
	0x30,	// 3
	0x2c,
	0x28,
	0x24,
	0x22,
//	0x21,
	0x20,
	0x19,
	0x18,
	0x17,
	0x16,
	0x15,
	0x14,
	0x13,
	0x12,
	0x11,
	0x10,
	0x0f,
	0x0e,
	0x0d,
	0x0c,
	0x0b,
	0x0a,
	0x09,
	0x08,
	0x07,
	0x06,
	0x05,
	0x04,
	0x03,
	0x02,
//	0x01,
//	0x00,
};

BYTE CODE Bass_Step_Map[]=
{//treble=bass|0x10
    0x00,	// 0 (-14 dB)
	0x01,	// 1 (-12 dB)
	0x02,	// 2 (-10 dB)
	0x03,	// 3 (-8 dB)
	0x04,	// 4 (-6 dB)
	0x05,	//5 (-4 dB)
	0x06,	//6 (-2 dB)
	0x0f,	//7 ( 0 dB)
	0x0e,	//8 (+2 dB)
	0x0d,	//9 (+4 dB)
	0x0c,	//10 (+6 dB)
	0x0b,	//11 (+8 dB)
	0x0a,	//12 (+10 dB)
	0x09,	//13 (+12 dB)    
	0x08,	//14 (+14 dB)
};

BYTE CODE Front_Rear_Step_Map[]=
{  //sl     sr=sl|0x20
       0x1f,	// 0 (-38 dB)
	0x1e,	// 1 (-37 dB)
	0x1d,	// 2 (-36 dB)
	0x1c,	// 3 (-35 dB)
	0x1b,	// 4 (-33 dB)
	0x1a,	//5 (-32 dB)
	0x19,	//6 (-31 dB)
	0x18,	//7 ( -30 dB)
	0x17,	//8 (-28 dB)
	0x16,	//9 (-27 dB)
	0x15,	//10 (-26 dB)
	0x14,	//11 (-25 dB)
	0x13,	//12 (-23 dB)
	0x12,	//13 (-22 dB)    
	0x11,	//14 (-21 dB)
	0x10,	//15(-20 dB)
	0x0f,	//16(-18 dB)
	0x0e,	//17(-17 dB)
	0x0d,	//18(-16 dB)
	0x0c,	//19(-15 dB)
	0x0b,	//20(-13 dB)
	0x0a,	//21(-12 dB)
	0x09,	//22(-11 dB)
	0x08,	//23(-10 dB)
	0x07,	//24(-08 dB)
	0x06,	//25(-07 dB)
	0x05,	//26(-06 dB)
	0x04,	//27(-05 dB)
	0x03,	//28(-03 dB)
	0x02,	//29(-02 dB)
	0x01,	//30(-01 dB)
	0x00,	//31(-00 dB)
	0x00,	//32(-00 dB)	
};


BYTE	VolSel = PT231X_NONE;

BYTE 	VolType = 0;	//main vol:1  2---treb; 3----bass

BYTE	VolMain = 17;

BYTE	VolTreb = 11;	//0-14

BYTE	VolBass	= 7;	//0-14

BYTE	VolFront =31;
BYTE	VolRear = 23;

///////////////////////////////////////////////////////////////////
BOOL I2cWritePT2313(BYTE ivol)
{
	BOOL acktemp = 0;

#if defined(FUNC_PT231X_IIC)||defined(AU6210K_NR_D_8_CSRBT)
	I2C_Start(PT231X_IIC);
    acktemp = I2C_WriteByte(PT231X_AD,PT231X_IIC);
	acktemp |= I2C_WriteByte(ivol,PT231X_IIC);
	I2C_Stop(PT231X_IIC);	
#else
	I2C_Start(EEPROM_IIC);
    acktemp = I2C_WriteByte(PT231X_AD,EEPROM_IIC);
	acktemp |= I2C_WriteByte(ivol,EEPROM_IIC);
	I2C_Stop(EEPROM_IIC);	
#endif
	return acktemp;
}						

void PT2313SelInput(BYTE input)
{
	if(input == PT231X_MP3FM)	//usb/sd
	{
		I2cWritePT2313 (0x41 | 0x10);
	}
	else if(input == PT231X_LINEIN)	//aux 1
	{
		I2cWritePT2313 (0x40| 0x10);
	}
	else if(input == PT231X_NONE)	//aux 2
	{
		I2cWritePT2313 (0x42| 0x10);
	}
}

void PT2313MuteOnOff(BYTE MuteOn)
{
	if(MuteOn)	//mute on
	{
		//MuteOnOff(1);
		PT2313SelInput(PT231X_NONE);
		
		I2cWritePT2313(0xc0|0x1f);
		I2cWritePT2313(0xe0|0x1f);
		I2cWritePT2313(0x80|0x1f);
		I2cWritePT2313(0xa0|0x1f);

		I2cWritePT2313(0x3f);
	}
	else
	{
		//if((VolMain)&&(EarState == 0)&&(gSys.MuteFg == FALSE))
		if((VolMain)&&(gSys.MuteFg == FALSE))
		{
			switch(gSys.SystemMode)
			{

				case SYS_MODE_USB:
				case SYS_MODE_SD:
				case SYS_MODE_RADIO:
					VolSel =PT231X_MP3FM;
					break;
				case SYS_MODE_LINEIN:
#ifdef FUNC_BLUETOOTH_CSR_EN
				case SYS_MODE_BLUETOOTH:
#endif
					VolSel =PT231X_LINEIN;
					break;
			}
			
			PT2313SelInput(VolSel);
			I2cWritePT2313(0xc0|Front_Rear_Step_Map[VolRear]);
			I2cWritePT2313(0xe0|Front_Rear_Step_Map[VolRear]);
			I2cWritePT2313(0x80|Front_Rear_Step_Map[VolFront]);
			I2cWritePT2313(0xa0|Front_Rear_Step_Map[VolFront]);
			I2cWritePT2313(VOL_Step_Map[VolMain]);
			//MuteOnOff(0);
		}
	}

}
void PT2313Init(void)
{
	VolType = VOL_MAIN;
	VolTreb = 7;	//0-14
	VolBass	= 7;	//0-14
	VolFront =31;
	VolRear = 31;
#if 0
	if(EarState)	// mute 
	{
		MuteOnOff(1);
		PT2313SelInput(PT2313_NONE);
		
		I2cWritePT2313(0xc0|0x1f);
		I2cWritePT2313(0xe0|0x1f);
		I2cWritePT2313(0x80|0x1f);
		I2cWritePT2313(0xa0|0x1f);
		I2cWritePT2313(0x60|Bass_Step_Map[VolBass]);	
		I2cWritePT2313(0x70|Bass_Step_Map[VolTreb]);
		I2cWritePT2313(0x3f);

	}
	else
#endif		
	{
		I2cWritePT2313(0xc0|Front_Rear_Step_Map[VolRear]);
		I2cWritePT2313(0xe0|Front_Rear_Step_Map[VolRear]);
		I2cWritePT2313(0x80|Front_Rear_Step_Map[VolFront]);
		I2cWritePT2313(0xa0|Front_Rear_Step_Map[VolFront]);
		I2cWritePT2313(0x60|Bass_Step_Map[VolBass]);	
		I2cWritePT2313(0x70|Bass_Step_Map[VolTreb]);
		I2cWritePT2313(0x02);//默认调节最大音量
		//MuteOnOff(0);
	}
	
}



void PT2313VolAdd(void)
{
	if(VolType == VOL_MAIN)	//main vol
	{
		if(VolMain == 0)
		{
			VolMain = 1;
//			PT2313GetRear();			
			PT2313MuteOnOff(0);
			
		}
		else if(VolMain < 32)
		{
			VolMain++;
			//if(EarState == 0)
			{
				I2cWritePT2313(VOL_Step_Map[VolMain]);
			}
		}
#ifdef FUNC_DISP_EN
		DispVolume(TRUE);
#endif
	}
	else if(VolType == VOL_TREB)	//treb
	{
		if(VolTreb < 14)
		{
			VolTreb++;
			//if(EarState == 0)
			{
				I2cWritePT2313(0x70|Bass_Step_Map[VolTreb]);
			}
		}
#ifdef FUNC_DISP_EN		
		DispTreb(TRUE);
#endif
	}
	else if(VolType == VOL_BASS)	//bass
	{
		if(VolBass< 14)
		{
			VolBass++;
			I2cWritePT2313(0x60|Bass_Step_Map[VolBass]);
		}
#ifdef FUNC_DISP_EN		
		DispBass(TRUE);
#endif
		
	}
//	else if(VolType == 4)	//front
//	{
//		if(VolFront < 31)
//		{
//			VolFront++;
//			I2cWritePT2313(0x80|Front_Rear_Step_Map[VolFront]);
//			I2cWritePT2313(0xa0|Front_Rear_Step_Map[VolFront]);			
//		}
//	}
//	else if(VolType == VOL_BASS)	//rear
//	{
//		if(VolRear < 31)
//		{
//			VolRear++;
			//if(EarState == 0)
//			{
//				I2cWritePT2313(0xc0|Front_Rear_Step_Map[VolRear]);
//				I2cWritePT2313(0xe0|Front_Rear_Step_Map[VolRear]);			
//			}
//		}
//	}	
}

void PT2313TrebBassSetDefault()
{
	VolTreb = 7;	//0-14
	VolBass	= 7;	//0-14
	VolFront =31;
	VolRear = 31;
	//if(EarState == 0)
	{
		I2cWritePT2313(0x60|Bass_Step_Map[VolBass]);	
		I2cWritePT2313(0x70|Bass_Step_Map[VolTreb]);
		I2cWritePT2313(0x80|Front_Rear_Step_Map[VolFront]);
		I2cWritePT2313(0xa0|Front_Rear_Step_Map[VolFront]);
	}
}

void PT2313VolSub(void)
{
	if(VolType == VOL_MAIN)	//main vol
	{
		if(VolMain == 1)
		{
			VolMain = 0;
			PT2313MuteOnOff(1);
		}
		else if(VolMain > 0)
		{
			VolMain--;
			//if(EarState == 0)
			{
				I2cWritePT2313(VOL_Step_Map[VolMain]);
			}
//			AT24C02WriteByte(ADDR_VOLUME, VolMain);
		}
#ifdef FUNC_DISP_EN
		DispVolume(TRUE);
#endif
		
	}
	else if(VolType == VOL_TREB)	//treb
	{
		if(VolTreb > 0)
		{
			VolTreb--;
			//if(EarState == 0)
			{
				I2cWritePT2313(0x70|Bass_Step_Map[VolTreb]);
			}
		}
#ifdef FUNC_DISP_EN		
		DispTreb(TRUE);
#endif
		
	}
	else if(VolType == VOL_BASS)	//bass
	{
		if(VolBass > 0)
		{
			VolBass--;
			I2cWritePT2313(0x60|Bass_Step_Map[VolBass]);
		}
#ifdef FUNC_DISP_EN		
		DispBass(TRUE);
#endif		

	}
//	else if(VolType == 4)	//front
//	{
//		if(VolFront > 0)
//		{
//			VolFront--;
//			I2cWritePT2313(0x80|Front_Rear_Step_Map[VolFront]);
//			I2cWritePT2313(0xa0|Front_Rear_Step_Map[VolFront]);			
//		}
//	}
//	else if(VolType == VOL_BASS)	//rear
//	{
//		if(VolRear > 0)
//		{
//			VolRear--;
			//if(EarState == 0)
//			{
//				I2cWritePT2313(0xc0|Front_Rear_Step_Map[VolRear]);
//				I2cWritePT2313(0xe0|Front_Rear_Step_Map[VolRear]);
//			}
//		}
//	}	
}


VOID PT2315E_LoudnessOnOff(VOID)
{
	static BOOL LoudnessOnOffFlag = FALSE;
	if(LoudnessOnOffFlag)
	{
		LoudnessOnOffFlag = FALSE;
		I2cWritePT2313(0x40|0x00);
	}
	else
	{
		LoudnessOnOffFlag = TRUE;	
		I2cWritePT2313(0x40|0x40);
	}
}

VOID PT2315E_Do(BYTE Message)
{
	switch(Message)
	{
	case MSG_TREBUP:
		VolType = VOL_TREB;
	    PT2313VolAdd();  	
		break;  	
	case MSG_TREBDN:
		VolType = VOL_TREB;
	    PT2313VolSub();  	
		break;  	
	case MSG_BASSUP:
		VolType = VOL_BASS;
	    PT2313VolAdd();  	
		break;  	
	case MSG_BASSDN:
		VolType = VOL_BASS;
	    PT2313VolSub();  	
		break;  	
	case MSG_DEFAULT:
		PT2313TrebBassSetDefault();
		break;  

	}
}
#endif


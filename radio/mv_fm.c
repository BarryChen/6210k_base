#include<string.h>
#include "type.h"
#include "utility.h"
#include "debug.h"
#include "timer.h"
#include "mv_fm.h"
#include "i2c.h"
#include "gpio.h"
#include "breakpoint.h"

#include "radio_ctrl.h"


#ifdef RADIO_MVFM_EN

#define MVFM_CHIP_ADDR 		0x20

#define MVFM_USING_INDUCTOR	1 
#define MVFM_INVERSE_IMR	1
#define MVFM_PILOT_CCA 		1  // 0: disable stereo judgement for the country has many mono FM stations


#define MVFM_DEF_FREQ		875
#define CH_SETUP_DELAY_TIME	50 


//FM模块寄存器定义
#define MVFM_REG_SYSTEM1			0x00
	#define MASK_R_TXRX				0x30
	#define MASK_R_FMAM				0xC0
	#define MASK_STNBY				0x20	
	#define MASK_RXMONO 			0x04
	#define MASK_RXCCA				0x03
	#define MASK_CHSC				0x02	

#define	MVFM_REG_CCA				0x01
	#define MASK_SINE_CLOCK			0x00    //inject sine-wave clock  
	#define MASK_DIGITAL_CLOCK		0x80 
	
#define MVFM_REG_SNR				0x02
#define MVFM_REG_RSSISIG			0x03
#define MVFM_REG_STATUS1			0x04
	#define MASK_RXCCA_FAIL			0x08
	#define MASK_ST_MO_RX			0x01

#define MVFM_REG_CID1				0x05
#define MVFM_REG_CID2				0x06
#define MVFM_REG_CH					0x07
#define MVFM_REG_CH_START			0x08
#define MVFM_REG_CH_STOP			0x09
#define MVFM_REG_CH_STEP			0x0A
	#define MASK_STEP_CONSTANT		0x40

#define MVFM_REG_RDSD0				0x0B
#define MVFM_REG_STATUS2			0x13
#define MVFM_REG_VOL_CTL			0x14
#define	MVFM_REG_XTAL_DIV0			0x15
#define	MVFM_REG_XTAL_DIV1			0x16
#define	MVFM_REG_XTAL_DIV2			0x17
#define	MVFM_REG_INT_CTRL			0x18
#define MVFM_REG_GAIN_SEL			0x1B
#define MVFM_REG_SYSTEM_CTL2		0x1D
#define MVFM_REG_CCA1				0x27
#define MVFM_REG_PLT1				0x2F
#define	MVFM_REG_PLT2				0x30
#define MVFM_REG_CCOND1				0x33  
#define	MVFM_REG_CCA_CNT1			0x37
#define	MVFM_REG_CCA_CNT2			0x38
#define	MVFM_REG_CCA_SNR_TH_1		0x39
#define	MVFM_REG_CCA_SNR_TH_2		0x3A
#define MVFM_REG_REF				0x49


#define SMSTART_VAL					12
#define SNCSTART_VAL				51
#define HCCSTART_VAL				18


#define CH_CH						0x03
#define CH_CH_STOP					0x30
#define CH_CH_START					0x0C




BOOL MVFM_WriteReg(WORD RegAddr, BYTE Data)
{
  	BYTE TryCount = 8;
	
	while(--TryCount)
	{
	  	if(I2C_WriteNByte(MVFM_CHIP_ADDR, RegAddr, &Data, 1, FM_IIC))
		{
	  		break;
		}
	}
	
	return TRUE;
}


BYTE MVFM_ReadReg(BYTE RegAddr)
{
	BYTE Temp = 0;    
	BYTE TryCount = 8;

	while(--TryCount)
	{
		if(I2C_ReadNByte(MVFM_CHIP_ADDR, RegAddr, &Temp, 1, FM_IIC))
		{
			break;
		}
	}
	
	return Temp;
}

VOID MVFM_SetRegBit(BYTE RegAddr, BYTE BitMask, BYTE Data)
{
	BYTE Temp;
	
	Temp = MVFM_ReadReg(RegAddr);
	Temp &= (~BitMask);
	Temp |= Data & BitMask;
	MVFM_WriteReg((WORD)RegAddr, Temp);
}


VOID MVFM_SetCh(WORD Freq)
{
	BYTE Temp;
	
	Freq = ((Freq << 1) - 1200);
	//writing lower 8 bits of CCA channel start index
	MVFM_WriteReg(MVFM_REG_CH_START, (BYTE)Freq);
	//writing lower 8 bits of CCA channel stop index
	MVFM_WriteReg(MVFM_REG_CH_STOP, (BYTE)Freq);
	//writing lower 8 bits of channel index
	MVFM_WriteReg(MVFM_REG_CH, (BYTE)Freq);

	//writing higher bits of CCA channel start,stop and step index
	Temp = (BYTE) ((Freq >> 8) & CH_CH);
	Temp |= ((BYTE)(Freq >> 6) & CH_CH_START);
	Temp |= ((BYTE) (Freq >> 4) & CH_CH_STOP);
	Temp |= MASK_STEP_CONSTANT;
	MVFM_WriteReg(MVFM_REG_CH_STEP, Temp);
}


WORD MVFM_GetCh(VOID) 
{
    BYTE Temp; 
    WORD ChIndex = 0;
	
    Temp = MVFM_ReadReg(MVFM_REG_CH_STEP);
    Temp &= CH_CH;
    ChIndex = Temp ;
    Temp = MVFM_ReadReg(MVFM_REG_CH);    
    ChIndex = (ChIndex << 8 )+Temp;
    return ((ChIndex + 1200) >> 1);
}


VOID MVFM_VolumeSet(VOID)
{
	MVFM_WriteReg((WORD)0x14, (MVFM_ReadReg(0x14) & 0xC0) | 0x07);
}


VOID MVFM_Mute(BOOL MuteFlag)
{
	if(MuteFlag)
	{
		MVFM_WriteReg(0x4A, 0x30);
	}
	else
	{
		WaitMs(200);
		MVFM_WriteReg(0x4A, 0x10);
	}
}


VOID MVFM_Initialization(VOID)
{  
	MVFM_WriteReg(0x00, 0x81); 
	WaitMs(10);

    /*********User sets chip working clock **********/
    //Following is where change the input clock wave type,as sine-wave or square-wave.
    //default set is 32.768KHZ square-wave input.
	MVFM_WriteReg(MVFM_REG_CCA, MASK_SINE_CLOCK);//XTAL_INJ，晶体选择
//	MVFM_WriteReg(MVFM_REG_XTAL_DIV0, 0x6E);
//	MVFM_WriteReg(MVFM_REG_XTAL_DIV1, 0x01);
//	MVFM_WriteReg(MVFM_REG_XTAL_DIV2, 0x54); 

	MVFM_WriteReg(MVFM_REG_XTAL_DIV0, 0x01);
	MVFM_WriteReg(MVFM_REG_XTAL_DIV1, 0x08);
	MVFM_WriteReg(MVFM_REG_XTAL_DIV2, 0x5C); 
	//WaitMs(10);

	MVFM_WriteReg(0x54, 0x47);//mod PLL setting
	//select SNR as filter3,SM step is 2db
	MVFM_WriteReg(0x19, 0xC4);
	MVFM_WriteReg(0x40, 0x70);	//set SNR as SM,SNC,HCC MPX
	MVFM_WriteReg(0x33, 0x9E);//set HCC and SM Hystersis 5db
	MVFM_WriteReg(0x2D, 0xD6);//notch filter threshold adjusting
	MVFM_WriteReg(0x43, 0x10);//notch filter threshold enable
	MVFM_WriteReg(0x47, 0x39);
	//MVFM_WriteReg(0x57, 0x21);//only for test
	//enter receiver mode directly
	MVFM_WriteReg(0x00, 0x11);
	//Enable the channel condition filter3 adaptation,Let ccfilter3 adjust freely
	MVFM_WriteReg(0x1D, 0xA9);	
	MVFM_WriteReg(0x4F, 0x40);//dsiable auto tuning
	MVFM_WriteReg(0x34, SMSTART_VAL); ///set SMSTART
	MVFM_WriteReg(0x35, SNCSTART_VAL); ///set SNCSTART
	MVFM_WriteReg(0x36, HCCSTART_VAL); ///set HCCSTART
   	MVFM_Mute(TRUE);
}


VOID MVFM_SetFreq(WORD Frep) 
{
	BYTE Temp;	 

	MVFM_WriteReg(MVFM_REG_REF, 0x7A);
	
	//RXInit
	MVFM_WriteReg(0x1B, 0x70);	//Let NFILT adjust freely
	MVFM_WriteReg(0x2C, 0x52);
	MVFM_WriteReg(0x45, 0x50);	//Set aud_thrd will affect ccfilter3's tap number
	MVFM_WriteReg(0x41, 0xCA);

#if MVFM_INVERSE_IMR
	Temp = MVFM_ReadReg(MVFM_REG_CCA) & (~0x40);
	if((Frep == 934) || (Frep == 939) || (Frep == 953) || (Frep == 998) || (Frep == 1048))
	{
		Temp |= 0x40;	// inverse IMR.
	}
	else
	{
		Temp &= ~0x40;
	}
	MVFM_WriteReg(MVFM_REG_CCA, Temp);
#endif

	MVFM_Mute(TRUE);
//	MVFM_SetCh(Frep,Frep,1);
	MVFM_SetCh(Frep); 
	//enable CCA mode with user write into frequency
	MVFM_WriteReg(0x00, 0x13);

#if MVFM_USING_INDUCTOR	
	//Auto tuning
	MVFM_WriteReg(0x4F, 0x80);
	Temp = MVFM_ReadReg(0x4F);
	Temp >>= 1;
	MVFM_WriteReg(0x4F, Temp);
#endif
	
	///avoid the "POP" noise.
    WaitMs(CH_SETUP_DELAY_TIME);
	///decrease reference PLL charge pump current.
	MVFM_WriteReg(MVFM_REG_REF, 0x70);
	//MVFM_Mute(FALSE);  //加上此句自动搜台中有"哒哒"声。
}


VOID MVFM_Init(VOID) 
{
	MVFM_Initialization();
	MVFM_SetFreq(MVFM_DEF_FREQ);
	MVFM_VolumeSet();
}		 


VOID MVFM_PowerOn(VOID)
{
	MVFM_SetRegBit(MVFM_REG_SYSTEM1, MASK_R_TXRX, ~MASK_STNBY);		//power up	
	WaitMs(200);	// wait for power stable
}


VOID MVFM_PowerDown(VOID)
{
	MVFM_Mute(TRUE);
	MVFM_SetRegBit(MVFM_REG_SYSTEM1, MASK_R_TXRX, MASK_STNBY);	// power down	
}


VOID MVFM_RXSetTH(VOID)
{	
	//increase reference PLL charge pump current.
	MVFM_WriteReg(MVFM_REG_REF, 0x7A);
	//NFILT program is enabled
	MVFM_WriteReg(0x1B, 0x78);
	//using Filter3
	MVFM_WriteReg(MVFM_REG_CCA1, 0x75);
	//setting CCA IF counter error range value(768).
	MVFM_WriteReg(MVFM_REG_CCA_CNT2, 0x03);

#if MVFM_PILOT_CCA
	MVFM_WriteReg(MVFM_REG_PLT1, 0x00);
#endif

	//selection the time of CCA FSM wait RF front end and AGC to settle:20ms
	//0x00:     10ms
	//0x40:     20ms(default)
	//0x80:     40ms
	//0xC0:     60ms	
	MVFM_SetRegBit(MVFM_REG_CCA_SNR_TH_2, 0xC0, 0xC0);  

	MVFM_SetRegBit(MVFM_REG_CCA, 0x3F, 0x14);  //setting RSSI threshold for CCA


	//selection the time of CCA FSM wait SNR calculator to settle:20ms
	//0x00:	    20ms(default)
	//0x40:	    40ms
	//0x80:	    60ms
	//0xC0:	    100m	
	MVFM_WriteReg(MVFM_REG_CCA_SNR_TH_1, 0x80 | 0x09); //setting SNR threshold for CCA
}


VOID MVFM_SearchSet(WORD Freq)
{
	BYTE Temp;
	
	//MVFM_Mute(TRUE);
	//MVFM_RXSetTH();	//为了减少自动搜台时间
	MVFM_SetCh(Freq);  

#if MVFM_USING_INDUCTOR
	//Auto tuning
	MVFM_WriteReg(0x00, 0x11);
	MVFM_WriteReg(0x4F, 0x80);
	Temp = MVFM_ReadReg(0x4F);
	Temp = (Temp >> 1);
	MVFM_WriteReg(0x4F, Temp);
#endif 
 
	MVFM_WriteReg(0x00, 0x12);	
}


BYTE MVFM_SearchRead(BOOL AutoSeekFlag)
{
	static	BYTE TimeOut = 0;
	BYTE i;
	BYTE StereoCount;

	if((!(MVFM_ReadReg(MVFM_REG_SYSTEM1) & MASK_CHSC)) || (TimeOut > 25))//5))
	{
		TimeOut = 0;	
		
		if(AutoSeekFlag)
		{
			gRadioData.CurrFreq = MVFM_GetCh();
			DBG(("Seek Current Freq %d\n", gRadioData.CurrFreq));
		}
		
		if(!(MVFM_ReadReg(MVFM_REG_STATUS1) & MASK_RXCCA_FAIL))
		{       
			DBG(("Search  Freq %d\n", gRadioData.CurrFreq));
#if MVFM_PILOT_CCA
			WaitMs(60);
			//snr = MVFM_ReadReg(MVFM_REG_SNR);
 			DBG(("SNR %d\n", (WORD)MVFM_ReadReg(MVFM_REG_SNR)));

			//自动搜台停台参数设置:
			//注:用户可能需要根据实际测试结果调整以下这些参数
			//if(gRadioData.CurrFreq != 960)	//特殊频点(指定检测Stero信号) 范围设置		
			{				
				if(((gRadioData.CurrFreq >= 950) && (gRadioData.CurrFreq <= 954))
				|| ((gRadioData.CurrFreq >= 1055) && (gRadioData.CurrFreq <= 1059)))
				{
					if(MVFM_ReadReg(MVFM_REG_SNR) > 30) //特殊有效频点SNR范围设置
					{
						DBG(("Search Freq %d\n", gRadioData.CurrFreq));
						DBG(("SNR > 30 %d\n", gRadioData.CurrFreq));
						return 1;
					}
				}
				else
				{
					if(MVFM_ReadReg(MVFM_REG_SNR) > 10)	//普通有效频点SNR范围设置  原来值为16
					{
						DBG(("Search Freq %d\n", gRadioData.CurrFreq));
						DBG(("SNR > 16 %d\n", gRadioData.CurrFreq));
						return 1;
					}
				}
			}

			//检测Stero信号
			StereoCount = 0;
			for(i = 0; i < 10; i++)
			{
				WaitMs(2);
				StereoCount += ((MVFM_ReadReg(MVFM_REG_STATUS1) & MASK_ST_MO_RX) ? 0 : 1);
				if(StereoCount > 3)
				{ 
			 		DBG((" Freq ok  okok %d\n", gRadioData.CurrFreq));
	  				return 1;
				}
			}
			return 0;
#endif
			return 1;  
					
		}
		return 0;  
	}
	TimeOut++;
	
	return 3;
}


BOOL MVFM_ReadID(VOID)
{
	if((MVFM_ReadReg(0x06) & 0xFC) == 0x84)
	{
		DBG(("MVFM_ID : 0x84\n"));
		MVFM_Init();
		return TRUE;
	}
	else
	{
		DBG(("MVFM_ID ERR : %BU\n", (MVFM_ReadReg(0x06) & 0xFC)));
		return FALSE;
	}
	MVFM_Mute(TRUE);
}


//Description: Config start, stop, step register for FM/AM CCA or CCS
//Param:
//    StartFreq
//        Set the frequency (10kHz) where scan to be started,
//        e.g. 7600 for 76.00MHz.
//    StopFreq
//        Set the frequency (10kHz) where scan to be stopped,
//        e.g. 10800 for 108.00MHz
//    SeekStep        
//        1: set leap step to (FM)100kHz
//        2: set leap step to (FM)200kHz 
//        0:  set leap step to (FM)50kHz
//	SeekMode:Select seek mode
//		1: 以硬件自动搜台和软件设置频点方式实现自动搜台
//		0: 以软件设置单步频点的方式实现自动搜台		
//	SeekStartFlag: Auto seek starts  flag
//		1:第一次启动搜台配置
//		0:搜台过程中的配置
//Return Value:       None
VOID MVFM_AutoSeekConfig(WORD StartFreq, WORD StopFreq, BYTE SeekStep, BOOL SeekMode, BOOL SeekStartFlag)
{
	BYTE Temp = 0;

	if(SeekStartFlag)
	{
		 MVFM_RXSetTH();	//启动自动搜台开始时执行一次就可以了
	}

	if(SeekMode)  
	{		
		StartFreq = ((StartFreq << 1) - 1200);
		StopFreq = ((StopFreq << 1) - 1200);
		// set to reg: CH_START
		MVFM_WriteReg(MVFM_REG_CH_START, (BYTE) StartFreq);
		Temp |= ((BYTE) (StartFreq >> 6) & CH_CH_START);
		// set to reg: CH_STOP
		MVFM_WriteReg(MVFM_REG_CH_STOP, (BYTE) StopFreq);
		Temp |= ((BYTE) (StopFreq >> 4) & CH_CH_STOP);
		// set to reg: CH_STEP
		Temp |= SeekStep << 6;
		MVFM_WriteReg(MVFM_REG_CH_STEP, Temp);

		//enter CCA mode,channel index is decided by internal CCA
		MVFM_SetRegBit(MVFM_REG_SYSTEM1, MASK_RXCCA, MASK_CHSC);	

		MVFM_WriteReg(0x4f, 0x00);//enable auto tunnging in CCA mode
	}
	else
	{
		MVFM_SearchSet(StartFreq);
	}
}


// Get   Stereo status
//返回值: 0 -- Mono;	1 -- Stero
BOOL MVFM_GetStereoStatus(VOID) 
{      
   return ((MVFM_ReadReg(MVFM_REG_STATUS1) & MASK_ST_MO_RX) ? 0 : 1);  
}


//Set receiver in mono mode
// Param:
//		MonoEnFlag: 1 -- Force receiver in mono mode;	0 -- Not forced,stereo and mono auto selected
VOID MVFM_SetMonoMode(BOOL MonoEnFlag)
{
	MVFM_SetRegBit(MVFM_REG_SYSTEM1, MASK_RXMONO, (MonoEnFlag << 2)); 
}


// Get the RSSI value
BYTE MVFM_GetRSSI(VOID) 
{      
   return MVFM_ReadReg(MVFM_REG_RSSISIG);  
}


// Get the SNR value
BYTE MVFM_GetSNR(VOID) 
{      
   return MVFM_ReadReg(MVFM_REG_SNR);  
}


//The MVFM integrates an analog volume controller and a digital volume controller to set audio output gain. 
//The digital gain step is 1dB, and the analog gain step is 6dB. 
//The total gain range is -47 dB to 0 dB. 
//Refer to Reg14h for more descriptions.
VOID MVFM_VolSet(BYTE Vol)
{
	BYTE Temp;	
	
	Temp = MVFM_ReadReg(0x14); 	
	Vol &= 0x07;//we just use the 6dB
	Temp &= 0xF8;
	Temp |= Vol;
	MVFM_WriteReg(0x14, Temp); 		
}


#endif

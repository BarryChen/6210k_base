#ifndef __DAC_H__
#define __DAC_H__


// Mixer Channel Select
#define DAC_CH_NONE		0x00
#define DAC_CH_DAC			0x09	//DECODER
#define DAC_CH_LINEIN		0x24	//LINE_IN_L, LINE_IN_R
#define DAC_CH_ONCHIP_FM_L	0x02	//ONCHIP_FM_L
#define DAC_CH_ONCHIP_FM_R	0x10	//ONCHIP_FM_R
#define DAC_CH_ONCHIP_FM	0x12	//ONCHIP_FM_L, ONCHIP_FM_R
#define DAC_CH_E0_L			0x40	//GPE0
#define DAC_CH_E1_R			0x80	//GPE1
#define DAC_CH_E0E1			0xC0	//GPE0, GPE1



#define MUTE_LOUT			0x04
#define MUTE_ROUT			0x40
#define MUTE_LR_BOTH	(MUTE_LOUT|MUTE_ROUT)


//如果是AU6210XX-E型号，需要使能下面的宏，DAC才能正常工作
//#define InDacSetSampleRate  		InDacSetSampleRate_2
//#define InDacSetVolume  			InDacSetVolume_2
//#define InDacInit					InDacInit_2
//#define InDacPowerDown  			InDacPowerDown_2
//#define InDacMuteEn 				InDacMuteEn_2
//#define InDacMuteDis				InDacMuteDis_2
//#define InDacChannelSel 			InDacChannelSel_2
//#define InDacLineInGain 			InDacLineInGain_2
//#define InDacAdjustSrAsPerPcmFifo	InDacAdjustSrAsPerPcmFifo_2
//#define InDacVCOMSet				InDacVCOMSet_2
//#define InDacExFmConnect			InDacExFmConnect_2
//#define InDacExFmDisConnect 		InDacExFmDisConnect_2
//#define UseAsSlaveDecd  			UseAsSlaveDecd_2


// Set Sample Rate
VOID InDacSetSampleRate(WORD SampleRate);
	
// Set DAC L/R Channel digital volume(0~4095)
VOID InDacSetVolume(WORD LVol, WORD RVol);

// DAC初始化
// 本函数会重新配置采样率，如果客户播放歌曲必须重新配置采样率，不管播放状态是PAUSE还是其它。
VOID InDacInit(BOOL IsFastCharge);

// DAC PowerDown
// 进入Sleep状态前必须调用本函数，否则WakeUp唤醒之后会有POPO声。
VOID InDacPowerDown(BOOL IsFastCharge);

// DAC Mute Enable.
// 本函数对Mixer Out信号操作，对LineIn/FMIn/DACIN均有效。
VOID InDacMuteEn(VOID);

// DAC Mute Disable.
// 本函数对Mixer Out信号操作，对LineIn/FMIn/DACIN均有效。
VOID InDacMuteDis(VOID);

// DAC(Mixer)通道选择(DAC/LineIn/FMIn)
// 1).配置FM通道，应用代码必须根据实际需要调用SET_A6A7_FMIN或SET_C0C1_FMIN宏
//    来正确配置相应IO的状态.
// 2).总共有DAC_CH_DAC/DAC_CH_LINEIN/DAC_CH_ONCHIP_FM/DAC_CH_E0E1四个通道可供选择,
//    其中DAC_CH_LINEIN、DAC_CH_ONCHIP_FM、DAC_CH_E0E1的增益控制使用相同的寄存器，只
//    要修改其中一路设置其它两路都受影响。
// 3).同时多个通道混合在一起的时候实际的输出增益与手册有较大偏差，请以实测结果为准。
VOID InDacChannelSel(BYTE Channel);

// Set LineIn/FMIn Volume(L、R).
VOID InDacLineInGain(BYTE LGain, BYTE RGain);

// DAC采样率微调
VOID InDacAdjustSrAsPerPcmFifo(WORD TotalPcmBytes);

// DAC 输出免电容配置.
// 设置isVCOMEn为TRUE开启DAC输出免电容功能。
VOID InDacVCOMSet(BOOL IsVCOMEn);

// mixer connect to External FM.
// 为了支持bibi声应用将EX FM L/R中一路或两路信号直接并到Mixer上。
// 用EX FM L/R产生方波时还需要将对应IO的OE位置为1(不能设置IE、PU、PD对应位)。
VOID InDacExFmConnect(BYTE FmCh);

// mixer connect to External FM.
// 为了支持bibi声应用将EX FM直接从Mixer断开。
VOID InDacExFmDisConnect(VOID);

// Use as Slave Decoder(默认设置采样率44.1K)
// GPD[0]->BCLK,  INPUT
// GPD[1]->LRCLK, INPUT 
// GPD[2]->DATA,  OUTPUT
VOID UseAsSlaveDecd(VOID);

//调整Dac Vmid输出电压，用于解决部分客户FM模式下刷屏会有噪音
//参数设置为TRUE，设置为1.5V，解决刷屏噪音问题，Dac输出音量衰减0.93
//参数设置为FLASE，设置为1.6V，Dac输出音量正常，模式配置为1.6V
VOID InDacSetVmid(BOOL VmdSclFlag);

#endif

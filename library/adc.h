#ifndef __ADC_H__
#define	__ADC_H__


// ADC referenct voltage select
//以下宏用于选择参考电压
#define		ADC_VREF_BG24			0  	   //使用BandGap 2.4V作为参考电压
#define		ADC_VREF_AVDD33			1  	   //使用VDDA 3.3V作为参考电压


// ADC Channel Select
//以下宏用于选择采样通道
#define  ADC_CHANNEL_AUDIO_L		1	   //DAC左声道采样
#define  ADC_CHANNEL_AUDIO_R		2	   //DAC右声道采样
#define  ADC_CHANNEL_B6				4	   //GPIO B6通道采样
#define  ADC_CHANNEL_D7				5      //GPIO D7通道采样
#define  ADC_CHANNEL_E0				6	   //GPIO E0通道采样
#define  ADC_CHANNEL_E1				7      //GPIO E1通道采样



// ADC value generate function, read value from one channel.
BYTE AdcReadOneChannel(BYTE	Channel, BYTE Vref);


#endif


#ifndef __TOUCHEKYLIB_H__
#define __TOUCHEKYLIB_H__

#include "type.h"


//进入TouchKey时引脚初始化相应引脚
//参数FreOutPutPin选择频率输出引脚
#define	TK_FREQU_OUTPUTPIN_A6	0

//参数TouchKeyPin选择频率输出引脚
#define	TOUCHKEYPIN_MASK		0XE3//参数的bit0~bit7，该参数bit置1为配置该pin为TouchKey为0则不配置为touchkey pin

//touchykey对应的通道
#define	TK_EO_CH0		0
#define	TK_E1_CH1		1
#define	TK_E2_CH2		2
#define	TK_E3_CH3		3
#define	TK_A0_CH4		4
#define	TK_A7_CH5		5
#define	TK_B0_CH6		6
#define	TK_B1_CH7		7


//功能：TouchKeyPin IO初始化
//参数1：FreOutPutPin选择频率输出引脚
//参数2：TouchKeyPin 对应的touchkey的引脚mask位，E0~E3 A0 A7 B0 B1对应该参数的bit0~bit7，该参数bit置1为配置该pin为TouchKey为0则不配置为touchkey pin，如该参数配置为0xff则表示开启8个TouchKey通道
VOID TouchKeyPortInit(BYTE FreOutPutPin, BYTE TouchKeyPin);

//离开TouchKey时引脚还原为默认状态
VOID TouchKeyPortDeInit();

//启动某个通道开始采集
//共8路，E0~E3 A0 A7 B0 B1对应该参数为：TK_EO_CH0，TK_E1_CH1，TK_E2_CH2，TK_E3_CH3，TK_A0_CH4，TK_A7_CH5，TK_B0_CH6，TK_B7_CH7
VOID TouchKeyStartConvert(BYTE Ch);

//判断当前所启动通道采集是否结束
//返回值FALSE:转换尚未完成，需要继续等待
//返回值TRUE:转换完成，可以读取Dout
BOOL TouchKeyIsConvertOK(VOID);

//获取当前通道的采集值，在通道转换完成后读取该通道的Dout输出
//返回值为该通道Dout输出
WORD TouchKeyGetDout(VOID);


#endif
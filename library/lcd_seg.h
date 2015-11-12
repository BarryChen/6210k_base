#ifndef __LCD_SEG_H__
#define __LCD_SEG_H__


//该宏定义不要修改，底层会调用该参数
#define 	LCD_BIAS_2		0
#define 	LCD_BIAS_3		1	
#define 	LCD_BIAS_4		2


// LCD模块初始化(配置LCD使用的IO、配置BIAS)  
// LCD : XXXX XXXX | XXXX XXX16	|	15	14	13	12	11	10	9	8	|	7	6	5	4	3	2	1	0
// GPIO: XXXX XXXX | XXXX XXXA7	|	A6	D7	D6	D5	D4	D3	D2	D1	|	D0	B7	B6	B4  B3	B2	B1	B0
// MaskSeg/MaskCom的bit映射关系为Bit[0:16]一一对应Lcd Channel[0:16](Bit0->Channel0 etc..)
// MaskSeg: 指定SEG端口使用了哪些Lcd Channel,最低有效位对应的管脚为SEG[0],最高有效位对应的管脚为SEG[M-1]
// MaskCom: 指定COM端口使用了哪些Lcd Channel,最低有效位对应的管脚为COM[0],最高有效位对应的管脚为COM[N-1]
// Bias: 选择偏置电压类型
VOID LcdInit(DWORD MaskSeg, DWORD MaskCom, BYTE Bias);

//退出LCD模式
VOID LcdDeinit(VOID);

//根据显示内容计算刷新数据
//要显示的内容已放在Buf[]中,某个比特为1时,对应的笔画点亮;
// 如果SEG数量不大于8,Buf[]长度等于COM个数,Buf[i]对应COM[i]的显示内容;
// 如果SEG数量大于8,Buf[]长度等于2倍COM个数,则Buf[2*i + 0]对应COM[i]的显示内容的高字节,Buf[2*i + 1]对应COM[i]的显示内容的低字节;
VOID LcdSetData(BYTE* Buf);

// 更新LCD显示刷新Mem数据
VOID LcdSetRefMem(VOID);

#endif

#ifndef __CHARGE_LED_H__
#define	__CHARGE_LED_H__


#define	FREQ_0HZ5		15
#define	FREQ_1HZ		7
#define	FREQ_2HZ		3
#define	FREQ_4HZ		1
#define	FREQ_8HZ		0


#define	CHARGE_DONE_LIGHT	1
#define	CHARGE_DONE_DARK	0


// 使能充电显示功能，占空比为50%
// 1.Freq表示充电指示灯闪烁频率
// 2.Polarity表示充电完成（当电池电压高于4.15V）后，灯是亮还是灭
VOID ChargeLedOpen(BYTE Freq, BYTE Polarity);

//关闭LED充电指示，注意此时GPIO_A2_ICS下拉电流源会被关闭
VOID ChargeLedClose(VOID);

#endif
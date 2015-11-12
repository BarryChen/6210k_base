#ifndef __GPIO_KEY_H__
#define __GPIO_KEY_H__

// 3个GPIO，实现5个按键扫描算法基本原理:
//1)、两个GPIO短接在一起，一个使能内部弱上拉，一个使能内部强下拉，
//		那么这两个GPIO上都是低电平（0.1V左右）。 
//2)、两个GPIO短接在一起，一个使能内部强上拉，一个使能内部弱下拉，
//		那么这两个GPIO上都是高电平（3.0V左右）

//
// Initialize gpio key scan (GPIO) operation.
//
VOID GpioKeyScanInit(VOID);

//
// Key process, image key value to key event.
//
MESSAGE GpioKeyEventGet(VOID);
#endif 


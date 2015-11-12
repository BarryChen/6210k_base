#ifndef __PWM_H__
#define	__PWM_H__

typedef enum _PWM_OUTPUT_SEL			  
{
	PWM3_OUTPUT_GPIOD7,
	PWM3_OUTPUT_GPIOA2,
	PWM2_OUTPUT_GPIOD6,
	PWM2_OUTPUT_GPIOA1,
	PWM1_OUTPUT_GPIOD5,
	PWM1_OUTPUT_GPIOA0,

} PWM_OUTPUT_SEL;

#define PWM_CLK_12MHZ		0
#define PWM_CLK_46875HZ		1

#define PWM_DRIVE_MODE_OUTPUT		0		//PWM信号从GPIO直接输出
#define PWM_DRIVE_MODE_ICS_1MA7		1		//PWM信号控制1.7mA下拉电流源
#define PWM_DRIVE_MODE_ICS_2MA4		2		//PWM信号控制2.4mA下拉电流源
#define PWM_DRIVE_MODE_ICS_4MA1		3		//PWM信号控制4.1mA下拉电流源


VOID PwmEnableChannel(PWM_OUTPUT_SEL PwmOutSel);
VOID PwmDisableChannel(PWM_OUTPUT_SEL PwmOutSel);

//ClkSel: 时钟源选择，PWM_CLK_12MHZ 或者 PWM_CLK_46875HZ
// 0: 12MHZ,    183.1HZ --- 6MHZ 
// 1: 46875HZ, 	0.715HZ  --- 46875HZ    
//Div: 分频比，1---65535
//Duty: 占空比，0---65535, 并且Duty <= Div, 占空比 = Duty/Div
//注意：1)、调用该函数后，必须调用相同通道的PwmEnable函数才有PWM输出
//	2)、如果执行了相同通道的PwmDisablel函数 后，必须重新执行上1)操作
VOID PwmConfigChannel(PWM_OUTPUT_SEL PwmOutSel, BYTE ClkSel, WORD Div, WORD Duty);

//PwmOutSel: 本次设置所针对的PWM通道
//DriveMode: 0---PWM由GPIO直接输出
//			 1---PWM信号控制1.7mA下拉电流源，PWM信号高电平期间1.7mA下拉电流源被使能，低电平期间GPIO为高阻态，用于呼吸灯应用
//           2---PWM信号控制2.4mA下拉电流源，PWM信号高电平期间2.4mA下拉电流源被使能，低电平期间GPIO为高阻态，用于呼吸灯应用
//           3---PWM信号控制4.1mA下拉电流源，PWM信号高电平期间4.1mA下拉电流源被使能，低电平期间GPIO为高阻态，用于呼吸灯应用
//如果不调用本函数，默认PWM由GPIO直接输出
VOID PwmSetDriveMode(PWM_OUTPUT_SEL PwmOutSel, BYTE PwmDriveMode);

#endif

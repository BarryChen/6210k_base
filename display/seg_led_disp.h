#ifndef __SEG_LED_DISP__
#define __SEG_LED_DISP__

#ifdef FUNC_SEG_LED_EN

//7和6脚LED屏的驱动程序，扫描算法如下：
 
//7脚LED屏，习惯上将7个管脚的序号称为1脚--7脚
//为了方便写代码，本模块中将7个脚序号改为0-6
//COM阶段分别为COM0--COM6
//SEG段码分别为SEG0--SEG6
 
//扫描算法
 
//COM高选中: 输出高电平
//COM非选中: 高阻态
 
//SEG亮: 使能下拉电流源
//SEG灭: 高阻态
 
//COM0: 1脚做COM输出高电平, 其他6个脚做SEG, 亮的笔画SEG使能下拉电流源，灭的笔画SEG设置为高阻态 
//COM1: 2脚做COM输出高电平, 其他6个脚做SEG, 亮的笔画SEG使能下拉电流源，灭的笔画SEG设置为高阻态
//COM2: 3脚做COM输出高电平, 其他6个脚做SEG, 亮的笔画SEG使能下拉电流源，灭的笔画SEG设置为高阻态
//COM3: 4脚做COM输出高电平, 其他6个脚做SEG, 亮的笔画SEG使能下拉电流源，灭的笔画SEG设置为高阻态
//COM4: 5脚做COM输出高电平, 其他6个脚做SEG, 亮的笔画SEG使能下拉电流源，灭的笔画SEG设置为高阻态
//COM5: 6脚做COM输出高电平, 其他6个脚做SEG, 亮的笔画SEG使能下拉电流源，灭的笔画SEG设置为高阻态
//COM6: 7脚做COM输出高电平, 其他6个脚做SEG, 亮的笔画SEG使能下拉电流源，灭的笔画SEG设置为高阻态
 

#if (defined(FUNC_7PIN_SEG_LED_EN) || defined(FUNC_6PIN_SEG_LED_EN))

#ifdef FUNC_6PIN_SEG_LED_EN
#define MAX_LED_PIN_NUM		6	//LED 引脚数
#else
#define MAX_LED_PIN_NUM		7	//LED 引脚数
#endif


//LED 屏引脚与GPIO 连接定义
#if 0//def AU6210K_6PIN_LED_DEMO
//6只脚段码LED屏示例
	#define LED_PIN1_PORT_PU 		GPIO_D_PU
	#define LED_PIN1_PORT_PD 		GPIO_D_PD
	#define LED_PIN1_PORT_OE 		GPIO_D_OE
	#define LED_PIN1_PORT_IE 		GPIO_D_IE
	#define LED_PIN1_PORT_ICS_1MA7 	GPIO_D_ICS_1MA7
	#define LED_PIN1_PORT_ICS_2MA4 	GPIO_D_ICS_2MA4
	#define LED_PIN1_PORT_OUT 		GPIO_D_OUT
	#define LED_PIN1_BIT			(1 << 0)

	#define LED_PIN2_PORT_PU 		GPIO_D_PU
	#define LED_PIN2_PORT_PD 		GPIO_D_PD
	#define LED_PIN2_PORT_OE 		GPIO_D_OE
	#define LED_PIN2_PORT_IE 		GPIO_D_IE
	#define LED_PIN2_PORT_ICS_1MA7 	GPIO_D_ICS_1MA7
	#define LED_PIN2_PORT_ICS_2MA4 	GPIO_D_ICS_2MA4
	#define LED_PIN2_PORT_OUT 		GPIO_D_OUT
	#define LED_PIN2_BIT			(1 << 1)

	#define LED_PIN3_PORT_PU 		GPIO_D_PU
	#define LED_PIN3_PORT_PD 		GPIO_D_PD
	#define LED_PIN3_PORT_OE 		GPIO_D_OE
	#define LED_PIN3_PORT_IE 		GPIO_D_IE
	#define LED_PIN3_PORT_ICS_1MA7 	GPIO_D_ICS_1MA7
	#define LED_PIN3_PORT_ICS_2MA4 	GPIO_D_ICS_2MA4
	#define LED_PIN3_PORT_OUT 		GPIO_D_OUT
	#define LED_PIN3_BIT			(1 << 2)

	#define LED_PIN4_PORT_PU 		GPIO_D_PU
	#define LED_PIN4_PORT_PD 		GPIO_D_PD
	#define LED_PIN4_PORT_OE 		GPIO_D_OE
	#define LED_PIN4_PORT_IE 		GPIO_D_IE
	#define LED_PIN4_PORT_ICS_1MA7 	GPIO_D_ICS_1MA7
	#define LED_PIN4_PORT_ICS_2MA4 	GPIO_D_ICS_2MA4
	#define LED_PIN4_PORT_OUT 		GPIO_D_OUT
	#define LED_PIN4_BIT			(1 << 3)

	#define LED_PIN5_PORT_PU 		GPIO_D_PU
	#define LED_PIN5_PORT_PD 		GPIO_D_PD
	#define LED_PIN5_PORT_OE 		GPIO_D_OE
	#define LED_PIN5_PORT_IE 		GPIO_D_IE
	#define LED_PIN5_PORT_ICS_1MA7 	GPIO_D_ICS_1MA7
	#define LED_PIN5_PORT_ICS_2MA4 	GPIO_D_ICS_2MA4
	#define LED_PIN5_PORT_OUT 		GPIO_D_OUT
	#define LED_PIN5_BIT			(1 << 4)

	#define LED_PIN6_PORT_PU 		GPIO_D_PU
	#define LED_PIN6_PORT_PD 		GPIO_D_PD
	#define LED_PIN6_PORT_OE 		GPIO_D_OE
	#define LED_PIN6_PORT_IE 		GPIO_D_IE
	#define LED_PIN6_PORT_ICS_1MA7 	GPIO_D_ICS_1MA7
	#define LED_PIN6_PORT_ICS_2MA4 	GPIO_D_ICS_2MA4
	#define LED_PIN6_PORT_OUT 		GPIO_D_OUT
	#define LED_PIN6_BIT			(1 << 5)

	#define LED_PIN7_PORT_PU 		GPIO_D_PU
	#define LED_PIN7_PORT_PD 		GPIO_D_PD
	#define LED_PIN7_PORT_OE 		GPIO_D_OE
	#define LED_PIN7_PORT_IE 		GPIO_D_IE
	#define LED_PIN7_PORT_ICS_1MA7 	GPIO_D_ICS_1MA7
	#define LED_PIN7_PORT_ICS_2MA4 	GPIO_D_ICS_2MA4
	#define LED_PIN7_PORT_OUT 		GPIO_D_OUT
	#define LED_PIN7_BIT			0
#elif 1
	#define LED_PIN1_PORT_PU 		GPIO_D_PU
	#define LED_PIN1_PORT_PD 		GPIO_D_PD
	#define LED_PIN1_PORT_OE 		GPIO_D_OE
	#define LED_PIN1_PORT_OE 		GPIO_D_OE
	#define LED_PIN1_PORT_ICS_1MA7 	GPIO_D_ICS_1MA7
	#define LED_PIN1_PORT_ICS_2MA4 	GPIO_D_ICS_2MA4
	#define LED_PIN1_PORT_OUT 		GPIO_D_OUT
	#define LED_PIN1_BIT			(1 << 6)

	#define LED_PIN2_PORT_PU 		GPIO_D_PU
	#define LED_PIN2_PORT_PD 		GPIO_D_PD
	#define LED_PIN2_PORT_OE 		GPIO_D_OE
	#define LED_PIN2_PORT_OE 		GPIO_D_OE
	#define LED_PIN2_PORT_ICS_1MA7 	GPIO_D_ICS_1MA7
	#define LED_PIN2_PORT_ICS_2MA4 	GPIO_D_ICS_2MA4
	#define LED_PIN2_PORT_OUT 		GPIO_D_OUT
	#define LED_PIN2_BIT			(1 << 5)

	#define LED_PIN3_PORT_PU 		GPIO_D_PU
	#define LED_PIN3_PORT_PD 		GPIO_D_PD
	#define LED_PIN3_PORT_OE 		GPIO_D_OE
	#define LED_PIN3_PORT_OE 		GPIO_D_OE
	#define LED_PIN3_PORT_ICS_1MA7 	GPIO_D_ICS_1MA7
	#define LED_PIN3_PORT_ICS_2MA4 	GPIO_D_ICS_2MA4
	#define LED_PIN3_PORT_OUT 		GPIO_D_OUT
	#define LED_PIN3_BIT			(1 << 4)

	#define LED_PIN4_PORT_PU 		GPIO_D_PU
	#define LED_PIN4_PORT_PD 		GPIO_D_PD
	#define LED_PIN4_PORT_OE 		GPIO_D_OE
	#define LED_PIN4_PORT_OE 		GPIO_D_OE
	#define LED_PIN4_PORT_ICS_1MA7 	GPIO_D_ICS_1MA7
	#define LED_PIN4_PORT_ICS_2MA4 	GPIO_D_ICS_2MA4
	#define LED_PIN4_PORT_OUT 		GPIO_D_OUT
	#define LED_PIN4_BIT			(1 << 3)

	#define LED_PIN5_PORT_PU 		GPIO_A_PU
	#define LED_PIN5_PORT_PD 		GPIO_A_PD
	#define LED_PIN5_PORT_OE 		GPIO_A_OE
	#define LED_PIN5_PORT_OE 		GPIO_A_OE
	#define LED_PIN5_PORT_ICS_1MA7 	GPIO_A_ICS_1MA7
	#define LED_PIN5_PORT_ICS_2MA4 	GPIO_A_ICS_2MA4
	#define LED_PIN5_PORT_OUT 		GPIO_A_OUT
	#define LED_PIN5_BIT			(1 << 2)

	#define LED_PIN6_PORT_PU 		GPIO_A_PU
	#define LED_PIN6_PORT_PD 		GPIO_A_PD
	#define LED_PIN6_PORT_OE 		GPIO_A_OE
	#define LED_PIN6_PORT_OE 		GPIO_A_OE
	#define LED_PIN6_PORT_ICS_1MA7 	GPIO_A_ICS_1MA7
	#define LED_PIN6_PORT_ICS_2MA4 	GPIO_A_ICS_2MA4
	#define LED_PIN6_PORT_OUT 		GPIO_A_OUT
	#define LED_PIN6_BIT			(1 << 1)

	#define LED_PIN7_PORT_PU 		GPIO_A_PU
	#define LED_PIN7_PORT_PD 		GPIO_A_PD
	#define LED_PIN7_PORT_OE 		GPIO_A_OE
	#define LED_PIN7_PORT_OE 		GPIO_A_OE
	#define LED_PIN7_PORT_ICS_1MA7 	GPIO_A_ICS_1MA7
	#define LED_PIN7_PORT_ICS_2MA4 	GPIO_A_ICS_2MA4
	#define LED_PIN7_PORT_OUT 		GPIO_A_OUT
	#define LED_PIN7_BIT			(1 << 0)
#else
//7只脚段码LED屏示例
#if (defined(FUNC_LED_ADC1_KEY_MULTIPLE_EN) || defined(FUNC_LED_ADC2_KEY_MULTIPLE_EN))
	#define LED_PIN1_PORT_PU 		GPIO_A_PU
	#define LED_PIN1_PORT_PD 		GPIO_A_PD
	#define LED_PIN1_PORT_OE 		GPIO_A_OE
	#define LED_PIN1_PORT_IE 		GPIO_A_IE
	#define LED_PIN1_PORT_ICS_1MA7 	GPIO_A_ICS_1MA7
	#define LED_PIN1_PORT_ICS_2MA4 	GPIO_A_ICS_2MA4
	#define LED_PIN1_PORT_OUT 		GPIO_A_OUT
	#define LED_PIN1_BIT			(1 << 0)
#else	
	#define LED_PIN1_PORT_PU 		GPIO_D_PU
	#define LED_PIN1_PORT_PD 		GPIO_D_PD
	#define LED_PIN1_PORT_OE 		GPIO_D_OE
	#define LED_PIN1_PORT_IE 		GPIO_D_IE
	#define LED_PIN1_PORT_ICS_1MA7 	GPIO_D_ICS_1MA7
	#define LED_PIN1_PORT_ICS_2MA4 	GPIO_D_ICS_2MA4
	#define LED_PIN1_PORT_OUT 		GPIO_D_OUT
	#define LED_PIN1_BIT			(1 << 7)
#endif
	
	#define LED_PIN2_PORT_PU 		GPIO_D_PU
	#define LED_PIN2_PORT_PD 		GPIO_D_PD
	#define LED_PIN2_PORT_OE 		GPIO_D_OE
	#define LED_PIN2_PORT_IE 		GPIO_D_IE
	#define LED_PIN2_PORT_ICS_1MA7 	GPIO_D_ICS_1MA7
	#define LED_PIN2_PORT_ICS_2MA4 	GPIO_D_ICS_2MA4
	#define LED_PIN2_PORT_OUT 		GPIO_D_OUT
	#define LED_PIN2_BIT			(1 << 6)

	#define LED_PIN3_PORT_PU 		GPIO_D_PU
	#define LED_PIN3_PORT_PD 		GPIO_D_PD
	#define LED_PIN3_PORT_OE 		GPIO_D_OE
	#define LED_PIN3_PORT_IE 		GPIO_D_IE
	#define LED_PIN3_PORT_ICS_1MA7 	GPIO_D_ICS_1MA7
	#define LED_PIN3_PORT_ICS_2MA4 	GPIO_D_ICS_2MA4
	#define LED_PIN3_PORT_OUT 		GPIO_D_OUT
	#define LED_PIN3_BIT			(1 << 5)

	#define LED_PIN4_PORT_PU 		GPIO_D_PU
	#define LED_PIN4_PORT_PD 		GPIO_D_PD
	#define LED_PIN4_PORT_OE 		GPIO_D_OE
	#define LED_PIN4_PORT_IE 		GPIO_D_IE
	#define LED_PIN4_PORT_ICS_1MA7 	GPIO_D_ICS_1MA7
	#define LED_PIN4_PORT_ICS_2MA4 	GPIO_D_ICS_2MA4
	#define LED_PIN4_PORT_OUT 		GPIO_D_OUT
	#define LED_PIN4_BIT			(1 << 4)

	#define LED_PIN5_PORT_PU 		GPIO_D_PU
	#define LED_PIN5_PORT_PD 		GPIO_D_PD
	#define LED_PIN5_PORT_OE 		GPIO_D_OE
	#define LED_PIN5_PORT_IE 		GPIO_D_IE
	#define LED_PIN5_PORT_ICS_1MA7 	GPIO_D_ICS_1MA7
	#define LED_PIN5_PORT_ICS_2MA4 	GPIO_D_ICS_2MA4
	#define LED_PIN5_PORT_OUT 		GPIO_D_OUT
	#define LED_PIN5_BIT			(1 << 3)

#if defined(FUNC_LED_ADC1_KEY_MULTIPLE_EN)
	#define LED_PIN6_PORT_PU		GPIO_D_PU
	#define LED_PIN6_PORT_PD		GPIO_D_PD
	#define LED_PIN6_PORT_OE		GPIO_D_OE
	#define LED_PIN6_PORT_IE		GPIO_D_IE
	#define LED_PIN6_PORT_ICS_1MA7	GPIO_D_ICS_1MA7
	#define LED_PIN6_PORT_ICS_2MA4	GPIO_D_ICS_2MA4
	#define LED_PIN6_PORT_OUT		GPIO_D_OUT
	#define LED_PIN6_BIT			(1 << 7)
#else
	#define LED_PIN6_PORT_PU 		GPIO_A_PU
	#define LED_PIN6_PORT_PD 		GPIO_A_PD
	#define LED_PIN6_PORT_OE 		GPIO_A_OE
	#define LED_PIN6_PORT_IE 		GPIO_A_IE
	#define LED_PIN6_PORT_ICS_1MA7 	GPIO_A_ICS_1MA7
	#define LED_PIN6_PORT_ICS_2MA4 	GPIO_A_ICS_2MA4
	#define LED_PIN6_PORT_OUT 		GPIO_A_OUT
	#define LED_PIN6_BIT			(1 << 1)
#endif

#if defined(FUNC_LED_ADC2_KEY_MULTIPLE_EN)
	#define LED_PIN7_PORT_PU		GPIO_B_PU
	#define LED_PIN7_PORT_PD		GPIO_B_PD
	#define LED_PIN7_PORT_OE		GPIO_B_OE
	#define LED_PIN7_PORT_IE		GPIO_B_IE
	#define LED_PIN7_PORT_ICS_1MA7	GPIO_B_ICS_1MA7
	#define LED_PIN7_PORT_ICS_2MA4	GPIO_B_ICS_2MA4
	#define LED_PIN7_PORT_OUT		GPIO_B_OUT
	#define LED_PIN7_BIT			(1 << 6)
#else
	#define LED_PIN7_PORT_PU 		GPIO_A_PU
	#define LED_PIN7_PORT_PD 		GPIO_A_PD
	#define LED_PIN7_PORT_OE 		GPIO_A_OE
	#define LED_PIN7_PORT_IE 		GPIO_A_IE
	#define LED_PIN7_PORT_ICS_1MA7 	GPIO_A_ICS_1MA7
	#define LED_PIN7_PORT_ICS_2MA4 	GPIO_A_ICS_2MA4
	#define LED_PIN7_PORT_OUT 		GPIO_A_OUT
	#define LED_PIN7_BIT			(1 << 0)
#endif
#endif


//LED 引脚对应 GPIO 初始化
#define LedPinGpioInit()	baGPIOCtrl[LED_PIN1_PORT_PU] |= LED_PIN1_BIT, \
						baGPIOCtrl[LED_PIN2_PORT_PU] |= LED_PIN2_BIT, \
						baGPIOCtrl[LED_PIN3_PORT_PU] |= LED_PIN3_BIT, \
						baGPIOCtrl[LED_PIN4_PORT_PU] |= LED_PIN4_BIT, \
						baGPIOCtrl[LED_PIN5_PORT_PU] |= LED_PIN5_BIT, \
						baGPIOCtrl[LED_PIN6_PORT_PU] |= LED_PIN6_BIT, \
						baGPIOCtrl[LED_PIN7_PORT_PU] |= LED_PIN7_BIT, \
						baGPIOCtrl[LED_PIN1_PORT_PD] &= (~LED_PIN1_BIT), \  
						baGPIOCtrl[LED_PIN2_PORT_PD] &= (~LED_PIN2_BIT), \  
						baGPIOCtrl[LED_PIN3_PORT_PD] &= (~LED_PIN3_BIT), \  
						baGPIOCtrl[LED_PIN4_PORT_PD] &= (~LED_PIN4_BIT), \  
						baGPIOCtrl[LED_PIN5_PORT_PD] &= (~LED_PIN5_BIT), \  
						baGPIOCtrl[LED_PIN6_PORT_PD] &= (~LED_PIN6_BIT), \  
						baGPIOCtrl[LED_PIN7_PORT_PD] &= (~LED_PIN7_BIT), \  
						baGPIOCtrl[LED_PIN1_PORT_OE] &= (~LED_PIN1_BIT), \
						baGPIOCtrl[LED_PIN2_PORT_OE] &= (~LED_PIN2_BIT), \   
						baGPIOCtrl[LED_PIN3_PORT_OE] &= (~LED_PIN3_BIT), \       
						baGPIOCtrl[LED_PIN4_PORT_OE] &= (~LED_PIN4_BIT), \       
						baGPIOCtrl[LED_PIN5_PORT_OE] &= (~LED_PIN5_BIT), \       
						baGPIOCtrl[LED_PIN6_PORT_OE] &= (~LED_PIN6_BIT), \       
						baGPIOCtrl[LED_PIN7_PORT_OE] &= (~LED_PIN7_BIT), \       
						baGPIOCtrl[LED_PIN1_PORT_ICS_1MA7] &= (~LED_PIN1_BIT), \
						baGPIOCtrl[LED_PIN2_PORT_ICS_1MA7] &= (~LED_PIN2_BIT), \   
						baGPIOCtrl[LED_PIN3_PORT_ICS_1MA7] &= (~LED_PIN3_BIT), \       
						baGPIOCtrl[LED_PIN4_PORT_ICS_1MA7] &= (~LED_PIN4_BIT), \       
						baGPIOCtrl[LED_PIN5_PORT_ICS_1MA7] &= (~LED_PIN5_BIT), \       
						baGPIOCtrl[LED_PIN6_PORT_ICS_1MA7] &= (~LED_PIN6_BIT), \       
						baGPIOCtrl[LED_PIN7_PORT_ICS_1MA7] &= (~LED_PIN7_BIT), \       
						baGPIOCtrl[LED_PIN1_PORT_ICS_2MA4] &= (~LED_PIN1_BIT), \
						baGPIOCtrl[LED_PIN2_PORT_ICS_2MA4] &= (~LED_PIN2_BIT), \   
						baGPIOCtrl[LED_PIN3_PORT_ICS_2MA4] &= (~LED_PIN3_BIT), \       
						baGPIOCtrl[LED_PIN4_PORT_ICS_2MA4] &= (~LED_PIN4_BIT), \       
						baGPIOCtrl[LED_PIN5_PORT_ICS_2MA4] &= (~LED_PIN5_BIT), \       
						baGPIOCtrl[LED_PIN6_PORT_ICS_2MA4] &= (~LED_PIN6_BIT), \       
						baGPIOCtrl[LED_PIN7_PORT_ICS_2MA4] &= (~LED_PIN7_BIT)      


//LED 各个引脚使用前要先禁流
#define LedAllPinGpioInput()	baGPIOCtrl[LED_PIN1_PORT_OE] &= (~LED_PIN1_BIT), \
							baGPIOCtrl[LED_PIN2_PORT_OE] &= (~LED_PIN2_BIT), \   
							baGPIOCtrl[LED_PIN3_PORT_OE] &= (~LED_PIN3_BIT), \       
							baGPIOCtrl[LED_PIN4_PORT_OE] &= (~LED_PIN4_BIT), \       
							baGPIOCtrl[LED_PIN5_PORT_OE] &= (~LED_PIN5_BIT), \       
							baGPIOCtrl[LED_PIN6_PORT_OE] &= (~LED_PIN6_BIT), \       
							baGPIOCtrl[LED_PIN7_PORT_OE] &= (~LED_PIN7_BIT), \       
							baGPIOCtrl[LED_PIN1_PORT_ICS_1MA7] &= (~LED_PIN1_BIT), \
							baGPIOCtrl[LED_PIN2_PORT_ICS_1MA7] &= (~LED_PIN2_BIT), \   
							baGPIOCtrl[LED_PIN3_PORT_ICS_1MA7] &= (~LED_PIN3_BIT), \       
							baGPIOCtrl[LED_PIN4_PORT_ICS_1MA7] &= (~LED_PIN4_BIT), \       
							baGPIOCtrl[LED_PIN5_PORT_ICS_1MA7] &= (~LED_PIN5_BIT), \       
							baGPIOCtrl[LED_PIN6_PORT_ICS_1MA7] &= (~LED_PIN6_BIT), \       
							baGPIOCtrl[LED_PIN7_PORT_ICS_1MA7] &= (~LED_PIN7_BIT), \       
							baGPIOCtrl[LED_PIN1_PORT_ICS_2MA4] &= (~LED_PIN1_BIT), \
							baGPIOCtrl[LED_PIN2_PORT_ICS_2MA4] &= (~LED_PIN2_BIT), \   
							baGPIOCtrl[LED_PIN3_PORT_ICS_2MA4] &= (~LED_PIN3_BIT), \       
							baGPIOCtrl[LED_PIN4_PORT_ICS_2MA4] &= (~LED_PIN4_BIT), \       
							baGPIOCtrl[LED_PIN5_PORT_ICS_2MA4] &= (~LED_PIN5_BIT), \       
							baGPIOCtrl[LED_PIN6_PORT_ICS_2MA4] &= (~LED_PIN6_BIT), \       
							baGPIOCtrl[LED_PIN7_PORT_ICS_2MA4] &= (~LED_PIN7_BIT)      



//设置LED 各个引脚输出高电平
#define LED_PIN1_OUT_HIGH	baGPIOCtrl[LED_PIN1_PORT_OE] |= LED_PIN1_BIT, \
							baGPIOCtrl[LED_PIN1_PORT_OUT] |= LED_PIN1_BIT;
                      
#define LED_PIN2_OUT_HIGH	baGPIOCtrl[LED_PIN2_PORT_OE] |= LED_PIN2_BIT, \
							baGPIOCtrl[LED_PIN2_PORT_OUT] |= LED_PIN2_BIT;

#define LED_PIN3_OUT_HIGH	baGPIOCtrl[LED_PIN3_PORT_OE] |= LED_PIN3_BIT, \
							baGPIOCtrl[LED_PIN3_PORT_OUT] |= LED_PIN3_BIT;

#define LED_PIN4_OUT_HIGH	baGPIOCtrl[LED_PIN4_PORT_OE] |= LED_PIN4_BIT, \
							baGPIOCtrl[LED_PIN4_PORT_OUT] |= LED_PIN4_BIT;

#define LED_PIN5_OUT_HIGH	baGPIOCtrl[LED_PIN5_PORT_OE] |= LED_PIN5_BIT, \
							baGPIOCtrl[LED_PIN5_PORT_OUT] |= LED_PIN5_BIT;

#define LED_PIN6_OUT_HIGH	baGPIOCtrl[LED_PIN6_PORT_OE] |= LED_PIN6_BIT, \
							baGPIOCtrl[LED_PIN6_PORT_OUT] |= LED_PIN6_BIT;

#define LED_PIN7_OUT_HIGH	baGPIOCtrl[LED_PIN7_PORT_OE] |= LED_PIN7_BIT, \
							baGPIOCtrl[LED_PIN7_PORT_OUT] |= LED_PIN7_BIT;                      


//设置LED 各个引脚恒流源输入
#if 0
#define LED_PIN1_IN_ON	baGPIOCtrl[LED_PIN1_PORT_ICS_1MA7] |= LED_PIN1_BIT;

#define LED_PIN2_IN_ON	baGPIOCtrl[LED_PIN2_PORT_ICS_1MA7] |= LED_PIN2_BIT;


#define LED_PIN3_IN_ON	baGPIOCtrl[LED_PIN3_PORT_ICS_1MA7] |= LED_PIN3_BIT;

#define LED_PIN4_IN_ON	baGPIOCtrl[LED_PIN4_PORT_ICS_1MA7] |= LED_PIN4_BIT;

#define LED_PIN5_IN_ON	SetA2IcsReg(A2_ICS_FOR_GPIO, A2_ICS_1MA7);//baGPIOCtrl[LED_PIN5_PORT_ICS_1MA7] |= LED_PIN5_BIT;						

#define LED_PIN6_IN_ON	baGPIOCtrl[LED_PIN6_PORT_ICS_1MA7] |= LED_PIN6_BIT;

#define LED_PIN7_IN_ON	baGPIOCtrl[LED_PIN7_PORT_ICS_1MA7] |= LED_PIN7_BIT; 
#else
#define LED_PIN1_IN_ON	baGPIOCtrl[LED_PIN1_PORT_ICS_2MA4] |= LED_PIN1_BIT \                      
						//baGPIOCtrl[LED_PIN1_PORT_ICS_1MA7] |= LED_PIN1_BIT;

#define LED_PIN2_IN_ON	baGPIOCtrl[LED_PIN2_PORT_ICS_2MA4] |= LED_PIN2_BIT \                      
						//baGPIOCtrl[LED_PIN2_PORT_ICS_1MA7] |= LED_PIN2_BIT;


#define LED_PIN3_IN_ON	baGPIOCtrl[LED_PIN3_PORT_ICS_2MA4] |= LED_PIN3_BIT \                      
						//baGPIOCtrl[LED_PIN3_PORT_ICS_1MA7] |= LED_PIN3_BIT;

#define LED_PIN4_IN_ON	baGPIOCtrl[LED_PIN4_PORT_ICS_2MA4] |= LED_PIN4_BIT \                      
						//baGPIOCtrl[LED_PIN4_PORT_ICS_1MA7] |= LED_PIN4_BIT;

#define LED_PIN5_IN_ON	baGPIOCtrl[LED_PIN5_PORT_ICS_2MA4] |= LED_PIN5_BIT \                      
						//baGPIOCtrl[LED_PIN5_PORT_ICS_1MA7] |= LED_PIN5_BIT;

#define LED_PIN6_IN_ON	baGPIOCtrl[LED_PIN6_PORT_ICS_2MA4] |= LED_PIN6_BIT \                      
						//baGPIOCtrl[LED_PIN6_PORT_ICS_1MA7] |= LED_PIN6_BIT;

#define LED_PIN7_IN_ON	baGPIOCtrl[LED_PIN7_PORT_ICS_2MA4] |= LED_PIN7_BIT \                      
						//baGPIOCtrl[LED_PIN7_PORT_ICS_1MA7] |= LED_PIN7_BIT;    
#endif

//禁止LED 各个引脚电流输入
#define LED_PIN1_IN_OFF	baGPIOCtrl[LED_PIN1_PORT_ICS_1MA7] &= (~LED_PIN1_BIT), \                      
						baGPIOCtrl[LED_PIN1_PORT_ICS_2MA4] &= (~LED_PIN1_BIT);

#define LED_PIN2_IN_OFF	baGPIOCtrl[LED_PIN2_PORT_ICS_1MA7] &= (~LED_PIN2_BIT), \                      
						baGPIOCtrl[LED_PIN2_PORT_ICS_2MA4] &= (~LED_PIN2_BIT);

#define LED_PIN3_IN_OFF	baGPIOCtrl[LED_PIN3_PORT_ICS_1MA7] &= (~LED_PIN3_BIT), \                      
						baGPIOCtrl[LED_PIN3_PORT_ICS_2MA4] &= (~LED_PIN3_BIT);

#define LED_PIN4_IN_OFF	baGPIOCtrl[LED_PIN4_PORT_ICS_1MA7] &= (~LED_PIN4_BIT), \                      
						baGPIOCtrl[LED_PIN4_PORT_ICS_2MA4] &= (~LED_PIN4_BIT);

#define LED_PIN5_IN_OFF	baGPIOCtrl[LED_PIN5_PORT_ICS_1MA7] &= (~LED_PIN5_BIT), \                      
						baGPIOCtrl[LED_PIN5_PORT_ICS_2MA4] &= (~LED_PIN5_BIT);

#define LED_PIN6_IN_OFF	baGPIOCtrl[LED_PIN6_PORT_ICS_1MA7] &= (~LED_PIN6_BIT), \                      
						baGPIOCtrl[LED_PIN6_PORT_ICS_2MA4] &= (~LED_PIN6_BIT);

#define LED_PIN7_IN_OFF	baGPIOCtrl[LED_PIN7_PORT_ICS_1MA7] &= (~LED_PIN7_BIT), \                      
						baGPIOCtrl[LED_PIN7_PORT_ICS_2MA4] &= (~LED_PIN7_BIT);     

#endif

VOID DispLedIcon(ICON_IDX Icon, ICON_STATU Light);
VOID LedDispDevSymbol(VOID);
VOID LedDispRepeat(VOID);
VOID LedDispPlayState(VOID);

#endif

#endif


#ifndef __SYSCFG_H__
#define __SYSCFG_H__


/*
**********************************************************
*					INCLUDE
* 说明：头文件包含列表，无需修改
**********************************************************
*/
#include <reg51.h>
#include <string.h>	
#include <stdlib.h>

#include "type.h"
#include "utility.h"
#include "sysctrl.h"
#include "timer.h"
#include "rst.h"
#include "host.h"
#include "card.h"
#include "dac.h"
#include "gpio.h"
#include "int.h"
#include "clk.h"
#include "sleep.h"
#include "adc.h"
#include "fsinfo.h"
#include "fat.h"
#include "dir.h"
#include "file.h"
#include "folder.h"
#include "partition.h"
#include "presearch.h"
#include "message.h"
#include "decd.h"
#include "mp3.h"
#include "storage_if.h"
#include "clk_if.h"
#include "spi_play.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////			
//用户项目定义
////////////////////////////////////////////////////////////////////////////////////////////////////////////			
//#define AU6210K_DEMO
//#define AU6210K_JLH_JH82B			//金洛华
//#define AU6210K_HXX_B002			//宏鑫芯B002
//#define AU6210K_NR_D_9X			//纳瑞D-9X系列
//#define AU6210K_NR_D_9X_XJ_HTS  //纳瑞D-9X系列更换部分线路

//#define AU6210K_NR_D_8_CSRBT     //ID=581
//#define AU6210K_LK_SJ_CSRBT     //莱卡·户外
#define AU6210K_ZB_BT007_CSR    //智博bt007 key 

#ifdef AU6210K_ZB_BT007_CSR
#define AU6210K_ZB_BT007_IR_IC_IS_334M_CSR  //移动电源的案子    //与智博bt007 仅有部分功能不同: ir ic  //168
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////////////			
// 看门狗功能
////////////////////////////////////////////////////////////////////////////////////////////////////////////			
#define	FUNC_WATCHDOG_EN  			//看门狗功能控制宏，请默认保持开启。


////////////////////////////////////////////////////////////////////////////////////////////////////////////			
// 系统支持设备工作模式配置
////////////////////////////////////////////////////////////////////////////////////////////////////////////	
#if defined(AU6210K_JLH_JH82B) || \
	defined(AU6210K_HXX_B002) || \
	defined(AU6210K_NR_D_8_CSRBT) || \
	defined(AU6210K_LK_SJ_CSRBT) || \
	defined(AU6210K_ZB_BT007_CSR)
#else
//#define FUNC_USB_EN					//USB HOST功能(可连接U盘播歌)
#endif

#if defined(AU6210K_LK_SJ_CSRBT)
#else
#define FUNC_CARD_EN				//CARD HOST功能(可连接SD/TF卡播歌),卡检测引脚配置见下面
#endif

#define FUNC_LINEIN_EN				

#if defined(AU6210K_NR_D_8_CSRBT)||defined(AU6210K_ZB_BT007_CSR)
#else
#define FUNC_RADIO_EN				//Radio功能,该型号芯片内部集成高性能FM模块
#endif

#ifdef AU6210K_ZB_BT007_CSR
#define		BT_BtPOWEROFF_TIME	(DWORD)(600000)///10min 5000//5s///5000//5s use debug

#define FUNC_READER_EN      		//单独的USB读卡器模式
#endif
//#define FUNC_AUDIO_READER_EN		//一线通模式，PC同时识别出USB声卡和USB读卡器	
//#define FUNC_AUDIO_EN				//单独的USB声卡模式	
//#define FUNC_STANDBY_EN		//支持待机模式，默认不开启
//#define FUNC_BLUETOOTH_EN

#define FUNC_BLUETOOTH_CSR_EN
#ifdef FUNC_BLUETOOTH_CSR_EN
     //#define FUNC_AUTO_BTSTANDBY     //一段时间内蓝牙没连接自动关掉蓝牙
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////			
// 配置OTP 播放的模式
// FUNC_OTP_KEY_SOUND_EN : 控制OTP 按键音功能的关闭和开启  
// FUNC_OTP_PLAY_EN：控制OTP 播放模式的关闭和开启
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//#define FUNC_OTP_EN
#ifdef FUNC_OTP_EN
	#define FUNC_OTP_KEY_SOUND_EN	//打开此宏定义，支持OTP 播放按键音功能
	//#define FUNC_OTP_PLAY_EN			//打开此宏定义，将OTP当做一个MP3设备播放
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////////////			
// 配置SPI 播放的模式
// SDK中仅实现了SST /GD SPI FLASH的代码，其他厂家需要客户自己添加
//FUNC_SPI_KEY_SOUND_EN : 控制SPI 按键音的关闭和开启
// FUNC_SPI_PLAY_EN：  控制SPI PLAY的关闭和开启
// FUNC_SPI_UPDATE_EN：是否从USB/SD卡升级MVF到SPI FLASH
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define	FUNC_SPI_FLASH_EN
#ifdef FUNC_SPI_FLASH_EN
	#define FUNC_SPI_KEY_SOUND_EN	//打开此宏定义，支持SPI 播放按键音
	//#define FUNC_SPI_PLAY_EN		    // 打开此宏定义，将SPI Flash当做一个MP3设备
	#define FUNC_SPI_UPDATE_EN			// Update SPI via U-Disk/SD-Card(place *.MVF under root-diretory).
	// Define SPI flash CS port and pin.
	#define	SPI_FLASH_CS_PORT		GPIO_B_IN
	#define	SPI_FLASH_CS_PIN		(1 << 3)
	// Define the customer ID in SPI flash device
	// Caution: the ID must equal to the value in flash memory of products
	#define	SPI_DEV_CUSTOMER_ID		0x55AA01FE		
	#define	SPI_SECTOR_SIZE			512

#ifdef FUNC_SPI_UPDATE_EN
	//#define FUNC_SPI_UPDATE_AUTO_EN		//打开此宏定义，支持SPI Flash自动升级，否者需要手动按键升级
#endif

#ifdef FUNC_SPI_KEY_SOUND_EN
	#define FUNC_SPI_FLASH_POWERON_SOUND		//打开该宏定义，支持SPI Flash开机音乐
#endif
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////////////			
// Ipod充电功能(注:系统时钟不能分频才能对Ipod充电)
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//#define FUNC_USB_CHARGE_EN


////////////////////////////////////////////////////////////////////////////////////////////////////////////			
// 【按键快速响应】功能，应用代码在user_interface.c文件中QuickResponse()函数
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//#define FUNC_QUICK_RESPONSE_EN


////////////////////////////////////////////////////////////////////////////////////////////////////////////			
// 配置RTC的模式
// FUNC_RTC_EN：  控制RTC的关闭和开启
// FUNC_ALARM_EN：是否有闹钟功能
// FUNC_LUNAR_EN：是否开启农历功能
////////////////////////////////////////////////////////////////////////////////////////////////////////////			
//#define FUNC_RTC_EN								//RTC功能
#ifdef FUNC_RTC_EN								//RTC子模块
	#define FUNC_ALARM_EN						//是否有闹钟功能
	#define FUNC_LUNAR_EN						//是否有农历功能
	
#ifdef FUNC_ALARM_EN
	//#define FUNC_SNOOZE_EN						//打开该宏定义，支持SNOOZE 功能
#endif
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////////////			
// 显示模块定义，根据实际选择可支持LED,LCD,单颗LED灯
// FUNC_DISP_EN：是否开启显示功能
// FUNC_SEG_LCD_EN, FUNC_SEG_LED_EN,	FUNC_SINGLE_LED_EN根据需要选择一个或多个，其他更详细的信息见segPanel.h
// 另外还需要配置gDisplayMode变量为需要的模式,显示模式列在下面.
// DISP_DEV_SLED	-> 单颗LED灯
// DISP_DEV_LED1888	-> 三位半段码LED屏
// DISP_DEV_LED8888	-> 四位段码LED屏
// DISP_DEV_LED57	-> 5*7段码LED屏
// DISP_DEV_LCD58	-> 5*8段码LCD屏
//注意显示控制宏与gDisplayMode变量的对应关系，例如要选择LED57显示设备，必须要开启FUNC_SEG_LED_EN宏并将gDisplayMode赋值为DISP_DEV_LED57。
////////////////////////////////////////////////////////////////////////////////////////////////////////////			
#define FUNC_DISP_EN					//显示功能
#ifdef FUNC_DISP_EN						//显示子模块
//#define FUNC_SEG_LCD_EN				//段码LCD屏
//#define FUNC_SEG_LED_EN				//段码LED屏
#if 1
#define FUNC_SINGLE_LED_EN			//单颗LED灯
#endif
#if defined(AU6210K_JLH_JH82B) || defined(AU6210K_NR_D_8_CSRBT)
#define FUNC_SINGLE_LED_HIGHON_EN //单颗LED是否是高亮
#endif
#endif

//特殊段码LED 屏选择
#ifdef	FUNC_SEG_LED_EN
#define FUNC_7PIN_SEG_LED_EN		//7只脚的段码LED 屏
//#define FUNC_6PIN_SEG_LED_EN		//6只脚的段码LED 屏
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////////////			
//7只脚的段码LED 屏与其它功能GPIO复用示例(以AU6210HD Demo使用的LED为例)
////////////////////////////////////////////////////////////////////////////////////////////////////////////			
#if (defined(FUNC_7PIN_SEG_LED_EN))
//#define FUNC_LED_ADC1_KEY_MULTIPLE_EN	//打开该宏定义，支持段码LED与ADC1 KEY端口复用功能
//#define FUNC_LED_ADC2_KEY_MULTIPLE_EN	//打开该宏定义，支持段码LED与ADC2 KEY端口复用功能
//#define FUNC_LED_LINEIN_MULTIPLE_EN		//打开该宏定义，支持段码LED与Line in检测端口复用功能
//#define FUNC_TIMER1_REFRESH_LED_EN	//打开该宏定义，支持Timer1 刷新段码LED显示(改善LED复用闪烁问题)
#endif

#ifdef FUNC_LED_ADC1_KEY_MULTIPLE_EN
#define LED_ADC1_KEY_MULTIPLE_PORT_PU			GPIO_D_PU
#define LED_ADC1_KEY_MULTIPLE_PORT_PD			GPIO_D_PD
#define LED_ADC1_KEY_MULTIPLE_PORT_IN			GPIO_D_IN
#define LED_ADC1_KEY_MULTIPLE_PORT_IE			GPIO_D_IE
#define LED_ADC1_KEY_MULTIPLE_PORT_OE			GPIO_D_OE
#define LED_ADC1_KEY_MULTIPLE_PORT_OUT			GPIO_D_OUT
#define LED_ADC1_KEY_MULTIPLE_BIT				(1 << 7)
#endif
	
#ifdef FUNC_LED_ADC2_KEY_MULTIPLE_EN
#define LED_ADC2_KEY_MULTIPLE_PORT_PU			GPIO_B_PU
#define LED_ADC2_KEY_MULTIPLE_PORT_PD			GPIO_B_PD
#define LED_ADC2_KEY_MULTIPLE_PORT_IN			GPIO_B_IN
#define LED_ADC2_KEY_MULTIPLE_PORT_IE			GPIO_B_IE
#define LED_ADC2_KEY_MULTIPLE_PORT_OE			GPIO_B_OE
#define LED_ADC2_KEY_MULTIPLE_PORT_OUT			GPIO_B_OUT
#define LED_ADC2_KEY_MULTIPLE_BIT				(1 << 6)
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////////////			
// 断点断电记忆配置
// 需要配置的宏：
//     FUNC_BREAK_POINT_EN: 是否开启断点断电功能
//     FUNC_RESTORE_VOLUME：音量是否强制恢复为记忆的音量
////////////////////////////////////////////////////////////////////////////////////////////////////////////			
#define FUNC_BREAK_POINT_EN				//断点、断电记忆功能
#ifdef FUNC_BREAK_POINT_EN//断点、断电记忆子模式
	//#define FUNC_RESTORE_VOLUME			//音量记忆模式选择，
	#define FUNC_RESTORE_DEVICE_EEPROM   	0
	#define FUNC_RESTORE_DEVICE_NVM         1
#if 0	
	#define FUNC_RESTORE_DEVICE_SELECE    	FUNC_RESTORE_DEVICE_EEPROM		//FUNC_RESTORE_DEVICE_EEPROM
#else
	#define FUNC_RESTORE_DEVICE_SELECE    	FUNC_RESTORE_DEVICE_NVM
#endif
	
//MP3播放时间掉电记忆选择，此功能仅用于只有AC供电突然掉电情况下记忆时间
#if (FUNC_RESTORE_DEVICE_SELECE == FUNC_RESTORE_DEVICE_EEPROM)
	//#define FUNC_SAVE_PLAYTIME_EN 	//默认关闭，支持此功能需要硬件增加大电容
#endif
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////////////			
// 说明：串口调试使能，使能时UART口可以打印出debug信息
//(注意: UART_USE_A1A2支持的最大波特率是4800)
////////////////////////////////////////////////////////////////////////////////////////////////////////////			
//#define	FUNC_UARTDBG_EN		      		//串口调试功能宏，注意UART端口分配，防止冲突！
#ifdef FUNC_UARTDBG_EN
	#define	UART_PORT		UART_USE_TXD		//UART使用的端口：UART_USE_A1A2 or UART_USE_TXD
	#define	UART_BAUDRATE	BAUDRATE_57600		//UART波特率：1200, 2400, 4800, 38400, 57600, 115200 
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////			
// 说明：串口从机使能，使能时UART口可以与MCU进行通信
//(注意: UART_USE_A1A2支持的最大波特率是4800)
////////////////////////////////////////////////////////////////////////////////////////////////////////////			

//#define	FUNC_SLAVE_UART_EN
////////////////////////////////////////////////////////////////////////////////////////////////////////////			
// 说明：按键功能配置
////////////////////////////////////////////////////////////////////////////////////////////////////////////			
#define FUNC_ADC_KEY_EN					//ADC按键功能

#if defined(AU6210K_ZB_BT007_CSR) && !defined(AU6210K_ZB_BT007_IR_IC_IS_334M_CSR)//007
#define FUNC_ADC_KEY_EN					//ADC按键功能
#endif
#if defined(AU6210K_HXX_B002) || defined(AU6210K_ZB_BT007_IR_IC_IS_334M_CSR)
#define FUNC_IR_KEY_EN				    //IR按键功能
#endif
//#define FUNC_GPIO_KEY_EN				//GPIO 扫描按键功能
//#define FUNC_CODING_KEY_EN			//旋转拨码开关
//#define FUNC_TOUCH_KEY_EN
//ADC按键端口选择
#ifdef FUNC_ADC_KEY_EN							
//SDK中的驱动，默认使用2个ADC通道来做按键，每个通道支持11个按键
#if (defined(FUNC_LED_ADC1_KEY_MULTIPLE_EN) || defined(FUNC_LED_ADC2_KEY_MULTIPLE_EN))
#define ADC_KEY_PORT_CH1	ADC_CHANNEL_D7	//ADC按键使用的端口：B6, D7, E0, E1，如果只使用一个通道，则保留本行
#define ADC_KEY_PORT_CH2	ADC_CHANNEL_B6	//ADC按键使用的端口：B6, D7, E0, E1，如果只使用一个通道，则注释掉本行
#elif 1//(defined(AU6210K_DEMO))
#define	ADC_KEY_PORT_CH1	ADC_CHANNEL_D7
#else
#define ADC_KEY_PORT_CH1	ADC_CHANNEL_E0	//ADC按键使用的端口：B6, D7, E0, E1，如果只使用一个通道，则保留本行
#define ADC_KEY_PORT_CH2	ADC_CHANNEL_E1	//ADC按键使用的端口：B6, D7, E0, E1，如果只使用一个通道，则注释掉本行
#endif
#endif

//IR 使用的端口选择
#ifdef FUNC_IR_KEY_EN	
#define	IR_KEY_PORT			IR_INPUT_GPIOB6	//IR使用的端口：IR_INPUT_GPIOB6 or IR_INPUT_GPIOE0
#endif

#if !defined(AU6210K_NR_D_8_CSRBT)
#define FUNC_NUMBER_KEY_EN		//打开该宏定义，支持数字按键功能
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////////////			
// 歌曲播放相关配置
////////////////////////////////////////////////////////////////////////////////////////////////////////////			
//#define	FUNC_LONG_NAME_EN		//长文件名功能														 
//#define	FUNC_FAST_PLAY_EN		//快进、快退功能
#define	FUNC_AUTO_PLAY_EN		//自动播放功能
//#define FUNC_FOLDER_EN			//文件夹功能
//#define FUNC_HIDDEN_FOLD_EN 	//播放隐藏文件夹中的歌曲
//#define FUNC_RECYCLED_FOLD_EN	//播放回收站文件夹中的歌曲
#define FUNC_OPEN_FILE_DELAY_EN		//是否有延迟响应PRE、NEXT命令，连续多次按PRE、NEXT时可以快速响应
//#define FUNC_DEVICE_FORCEPAUSE_EN	//打开该宏开关，支持插入充电器时强制暂停当前设备的播放

//加密播放功能
//#define FUNC_ENCRYPT_DECODE_EN
#ifdef 	FUNC_ENCRYPT_DECODE_EN
    #define ENCRYPT_EXT  "HMR" //客户可自定义后缀名	  
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////////////			
// FM,LINEIN音频输出通道选择配置
// FM、LINEIN模式配置，用户需要确保模式配置与实际电路相符
// LINEIN_IO_TYPE：选择LINEIN音频输出脚，即DAC_CH_E0_L、DAC_CH_E1_R、DAC_CH_E0E1、DAC_CH_LINEIN选一
////////////////////////////////////////////////////////////////////////////////////////////////////////////			
#ifdef FUNC_RADIO_EN	
#define RADIO_MVFM_EN	//Radio功能,该型号芯片内部集成高性能FM模块

//#define FM_DAC_OUT_D3D4		//打开该宏定义，支持FM模块音频信号从D3、D4直接输出
#endif

#ifdef FUNC_LINEIN_EN
#if defined(AU6210K_NR_D_8_CSRBT)
#define LINEIN_IO_TYPE		DAC_CH_NONE
#elif defined(AU6210K_LK_SJ_CSRBT)||defined(AU6210K_ZB_BT007_CSR)
#define LINEIN_IO_TYPE		DAC_CH_E0E1
#else
#define LINEIN_IO_TYPE		DAC_CH_LINEIN
#endif
#endif

#ifdef FUNC_BLUETOOTH_CSR_EN
#if defined(AU6210K_NR_D_8_CSRBT)
#define BLUETOOTH_CH_TYPE		DAC_CH_NONE
#elif defined(AU6210K_LK_SJ_CSRBT)||defined(AU6210K_ZB_BT007_CSR)
#define BLUETOOTH_CH_TYPE		DAC_CH_LINEIN
#else
#define BLUETOOTH_CH_TYPE		DAC_CH_E0E1
#endif

#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////			
//收音操作功能配置
////////////////////////////////////////////////////////////////////////////////////////////////////////////			
#ifdef	FUNC_RADIO_EN
#if defined(FUNC_SINGLE_LED_EN)
#define FUNC_RADIO_AUTOSEEK_EN	// 打开该宏定义，支持FM 硬件自动搜台功能,可以缩短搜台时间，但是不支持频点显示
#endif
//#define FUNC_RADIO_STEPOVER_EN		// 打开该宏定义， 支持FM 手动单步搜台功能
//#define FUNC_RADIO_SEMO_AUTOSEEK_EN 	// 打开该宏定义，支持FM  半自动搜台功能
#define FUNC_RADIO_ESD_AUTO_RESUME	// 打开该宏定义，支持FM  打ESD无声后自动恢复功能
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////////////			
//其它功能配置
////////////////////////////////////////////////////////////////////////////////////////////////////////////			
#if (defined(FUNC_AUDIO_EN) || defined(FUNC_AUDIO_READER_EN))
#define FUNC_POWER_ON_AUDIO_EN	//打开该宏定义，支持连接PC 开机直接进入声卡模式,默认关闭
#endif
//#define FUNC_DACOUT_DAPLESS_EN	// 打开该宏定义,支持 DAC输出端免电容直驱耳机功能
//#define FUNC_BEEP_SOUND_EN	//打开该宏定义，支持按键产生嘀嘀声
//#define FUNC_PWM_EN		//打开该宏定义，支持PWM功能


////////////////////////////////////////////////////////////////////////////////////////////////////////////			
// 系统分配比设置
////////////////////////////////////////////////////////////////////////////////////////////////////////////			
// Maximum system clock divid value is not big than 16
#define		CLK_DIV_RATE_MAX	16
// usually recommand values are 2, 4, 6, 8
#if defined(FUNC_SINGLE_LED_EN)
#define		CLK_DIV_RATE		8
#else
#define		CLK_DIV_RATE		4
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////////////			
// 定义电能监视(适用于带电池系统)的功能宏和选项宏
// 电能监视包括，电池电压检测及低电压后的系统行为以及充电指示等
// 电池电压检测，是指LDOIN输入端的电压检测功能(电池系统一般都是电池直接给LDOIN管脚供电)
// 该功能宏打开后，默认包含电池电压检测功能，有关电池电压检测的其它可定义参数，请详见power_monitor.c文件
////////////////////////////////////////////////////////////////////////////////////////////////////////////			
//#if defined(AU6210K_NR_D_9X_XJ_HTS)
#define FUNC_POWER_MONITOR_EN
//#endif

#ifdef FUNC_POWER_MONITOR_EN
//充电设备接入检测功能
//#define	OPTION_CHARGER_DETECT		//打开该宏定义，支持GPIO检测充电设备插入功能

//充电检测端口设置
#ifdef OPTION_CHARGER_DETECT
	#define CHARGE_DETECT_PORT_PU			GPIO_E_PU
	#define CHARGE_DETECT_PORT_PD			GPIO_E_PD
	#define CHARGE_DETECT_PORT_IN			GPIO_E_IN
	#define CHARGE_DETECT_PORT_IE			GPIO_E_IE
	#define CHARGE_DETECT_PORT_OE			GPIO_E_OE
	#define CHARGE_DETECT_BIT				(1 << 0)
#endif
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////////////			
// 系统省电模式配置
// 
// 需要配置的宏：
//     POWER_SAVING_MODE_OPTION：系统省电模式选择，不定义则取消系统省电功能，可选配置为
//		->POWER_SAVING_MODE_SLEEP：系统省电时进入SLEEP模式(片内LDO没有关闭,功耗较高)
//		->POWER_SAVING_MODE_POWERDOWN：系统省电时进入POWERDOWN模式(片内LDO关闭,功耗低)
// 注意事项：
//	  当选择POWERDOWN模式时，系统唤醒只有两种方式
//		->POWER_KEY唤醒；
//		->LOW POWER RTC唤醒(RTC_MODE==LOWPOWER_RTC,定义WAKEUP_IN_PWRDWN_BY_LP_RTC宏开启该功能)。
//     当选择Sleep模式时，系统唤醒方式有
//		->POWER_KEY唤醒；
//		->GPIO唤醒(必须是中断IO) ；
//		->NORMAL RTC(RTC_MODE==NORMAL_RTC)唤醒；
//		->IR唤醒。
//	  如果需要RTC唤醒功能，需要正确配置RTC_MODE和POWER_SAVING_MODE_OPTION。
////////////////////////////////////////////////////////////////////////////////////////////////////////////			

//定义POWER_KEY的开关形式，2选1，只能定义1种
#define PWR_KEY_DISABLE			0
#define PWR_KEY_PUSH_BUTTON		1		//按钮开关(软开关)
#define PWR_KEY_SLIDE_SWITCH	2		//拨动开关(硬开关)

#define POWER_SAVING_MODE_SLEEP			0
#define POWER_SAVING_MODE_POWERDOWN		1
#define POWER_SAVING_MODE_OPTION		POWER_SAVING_MODE_SLEEP////POWER_SAVING_MODE_POWERDOWN	//

#if (POWER_SAVING_MODE_OPTION == POWER_SAVING_MODE_POWERDOWN)	//设置系统省电模式为POWERDOWN
	//POWERDOWN方式: 系统关闭时关闭片内LDO，系统唤醒时片内LDO先上电
	//该方式要求芯片带POWER_KEY引脚，可以实现极低功耗(powerdown)下保留重要信息的要求
	//选择该方式后，默认系统包含POWER_KEY功能(必须选择带POWER KEY的芯片型号)
	#if defined(AU6210K_ZB_BT007_CSR) || defined(AU6210K_LK_SJ_CSRBT)
	 #define PWR_KEY_MODE	 PWR_KEY_SLIDE_SWITCH	//PWR_KEY_PUSH_BUTTON
	#else
     #define PWR_KEY_MODE	 PWR_KEY_PUSH_BUTTON
	#endif

#if	(PWR_KEY_MODE == PWR_KEY_PUSH_BUTTON)
		//如果选择软开关，可以定义POWER KEY开关机保持时间
		//在开机阶段，是指用户必须按住POWER_KEY超过[开关机保持时间]后，系统才会点亮显示继续运行，否则芯片再次进入powerdown
		//在关机阶段，用户必须按住POWER_KEY超过[开关机保持时间]后，系统才会关闭显示等，然后进入powerdown.
	#define TIME_POWER_ON_HOLD		1000	//单位ms
	#define TIME_POWER_OFF_HOLD		1000

	//除了POWER_KEY可以唤醒芯片启动片内主电源外，还可定义LP_RTC的闹钟源作为启动触发源，RTC唤醒仅仅在PWR_KEY_PUSH_BUTTON模式下应用
	#define	WAKEUP_IN_PWRDWN_BY_LP_RTC

	//定时关机功能和显示使能
	//#define FUN_SYSTEM_POWEROFF_WAIT_TIME
#endif
#else
	//#define PWR_KEY_MODE	PWR_KEY_DISABLE
#endif

#if (POWER_SAVING_MODE_OPTION == POWER_SAVING_MODE_SLEEP)	//设置系统省电模式为SLEEP
	//SLEEP方式: 系统关闭采用休眠的方式，该方式下片内主电源一直带电，功耗比POWERDOWN方式大
	//该方式下，系统的启动由具备唤醒功能的IO或芯片模块触发

	//定义哪些IO或者芯片模块可以唤醒系统
	//WIS是WAKEUP_IN_SLEEP的缩写
	#define		WIS_FLG_IR	 //定义IR 唤醒宏，IR IO选择由IR_KEY_PORT定义
	#define 	PWR_KEY_MODE	PWR_KEY_SLIDE_SWITCH
#endif		


////////////////////////////////////////////////////////////////////////////////////////////////////////////			
//说明：SD卡总线引脚选择
////////////////////////////////////////////////////////////////////////////////////////////////////////////			
#ifdef	FUNC_CARD_EN	
#define CARD_BUS_A3A4A5_EN
//#define CARD_BUS_A0E2E3_EN
//#define CARD_BUS_D5D6D7_EN
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////////////			
// SD卡插入和拔出检测的范式和引脚
////////////////////////////////////////////////////////////////////////////////////////////////////////////			
#ifdef	FUNC_CARD_EN
#if defined(AU6210K_NR_D_8_CSRBT) || defined(AU6210K_ZB_BT007_CSR)
#else
#define SD_DETECT_HW_SW				//定义卡的检测方式为软硬件结合检测方式(推荐)
#endif

#ifdef SD_DETECT_HW_SW
//定义卡连接硬件检测的pin脚
//在GPIO资源充足时，也可以定义一个单独的GPIO脚作为硬件检测脚，从而节省一个电阻
#if defined(AU6210K_JLH_JH82B) || defined(AU6210K_ZB_BT007_CSR)
#else
#define SD_DETECT_PIN_USE_A4	//使用A4复用为检测脚时打开此宏定义(可以节约GPIO资源)
//#define SD_DETECT_PIN_USE_E3	//使用E3复用为检测脚时打开此宏定义(可以节约GPIO资源)
//#define SD_DETECT_PIN_USE_D6	//使用D6复用为检测脚时打开此宏定义(可以节约GPIO资源)
//#define SD_DETECT_PIN_USE_A1	//使用A1复用为检测脚时打开此宏定义(可以节约GPIO资源)

#endif
#if defined(SD_DETECT_PIN_USE_A4)
	#define	CARD_DETECT_PORT_IN			GPIO_A_IN
	#define	CARD_DETECT_PORT_OE			GPIO_A_OE
	#define	CARD_DETECT_PORT_DS			GPIO_A_DS
	#define	CARD_DETECT_PORT_IE			GPIO_A_IE
	#define	CARD_DETECT_PORT_PU			GPIO_A_PU
	#define	CARD_DETECT_PORT_PD			GPIO_A_PD
	#define	CARD_DETECT_BIT				(1 << 4)
#elif defined(SD_DETECT_PIN_USE_A1)
	#define	CARD_DETECT_PORT_IN			GPIO_A_IN
	#define	CARD_DETECT_PORT_OE			GPIO_A_OE
	#define	CARD_DETECT_PORT_DS			GPIO_A_DS
	#define	CARD_DETECT_PORT_IE			GPIO_A_IE
	#define	CARD_DETECT_PORT_PU			GPIO_A_PU
	#define	CARD_DETECT_PORT_PD			GPIO_A_PD
	#define	CARD_DETECT_BIT				(1 << 1)
#elif defined(SD_DETECT_PIN_USE_E3)
	#define	CARD_DETECT_PORT_IN			GPIO_E_IN
	#define	CARD_DETECT_PORT_OE			GPIO_E_OE
	#define	CARD_DETECT_PORT_DS			GPIO_E_DS
	#define	CARD_DETECT_PORT_IE			GPIO_E_IE
	#define	CARD_DETECT_PORT_PU			GPIO_E_PU
	#define	CARD_DETECT_PORT_PD			GPIO_E_PD
	#define	CARD_DETECT_BIT				(1 << 3)
#elif defined(SD_DETECT_PIN_USE_D6)
	#define	CARD_DETECT_PORT_IN			GPIO_D_IN
	#define	CARD_DETECT_PORT_OE			GPIO_D_OE
	#define	CARD_DETECT_PORT_DS			GPIO_D_DS
	#define	CARD_DETECT_PORT_IE			GPIO_D_IE
	#define	CARD_DETECT_PORT_PU			GPIO_D_PU
	#define	CARD_DETECT_PORT_PD			GPIO_D_PD
	#define	CARD_DETECT_BIT				(1 << 6)
#elif defined(AU6210K_JLH_JH82B)
	#define	CARD_DETECT_PORT_IN			GPIO_E_IN
	#define	CARD_DETECT_PORT_OE			GPIO_E_OE
	#define	CARD_DETECT_PORT_DS			GPIO_E_DS
	#define	CARD_DETECT_PORT_IE			GPIO_E_IE
	#define	CARD_DETECT_PORT_PU			GPIO_E_PU
	#define	CARD_DETECT_PORT_PD			GPIO_E_PD
	#define	CARD_DETECT_BIT				(1 << 0)
#else
	#define	CARD_DETECT_PORT_IN			GPIO_A_IN
	#define	CARD_DETECT_PORT_OE			GPIO_A_OE
	#define	CARD_DETECT_PORT_DS			GPIO_A_DS
	#define	CARD_DETECT_PORT_IE			GPIO_A_IE
	#define	CARD_DETECT_PORT_PU			GPIO_A_PU
	#define	CARD_DETECT_PORT_PD			GPIO_A_PD
	#define	CARD_DETECT_BIT				(1 << 2)
#endif
#else
	   //如果没有定义卡的检测方式为软硬件结合方式，系统则按纯软件检测方式处理卡的插入检测
	   //纯软件检测方式下，系统的辐射干扰会变大!
#endif
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////////////			
// LINEIN检测引脚配置(line in pin configure)
// LINEIN_DETECT_PORT_IN ~ LINEIN_DETECT_BIT：LINEIN插入和拔除的检测脚，比如E2
////////////////////////////////////////////////////////////////////////////////////////////////////////////			
#ifdef FUNC_LINEIN_EN
#if defined(AU6210K_JLH_JH82B)
	#define LINEIN_DETECT_PORT_IN		GPIO_D_IN
	#define LINEIN_DETECT_PORT_OE		GPIO_D_OE
	#define LINEIN_DETECT_PORT_IE		GPIO_D_IE
	#define LINEIN_DETECT_PORT_PU		GPIO_D_PU
	#define LINEIN_DETECT_PORT_PD		GPIO_D_PD	
	#define LINEIN_DETECT_BIT			(1 << 4)
#elif 1//defined(AU6210K_CSR_BT) || defined(AU6210K_NR_D_8_CSRBT)
	#define	LINEIN_DETECT_PORT_IN		GPIO_B_IN
	#define	LINEIN_DETECT_PORT_OE		GPIO_B_OE
	#define LINEIN_DETECT_PORT_IE		GPIO_B_IE
	#define	LINEIN_DETECT_PORT_PU		GPIO_B_PU
	#define	LINEIN_DETECT_PORT_PD		GPIO_B_PD
	#define LINEIN_DETECT_BIT			(1 << 4)
#else
	#define	LINEIN_DETECT_PORT_IN		GPIO_E_IN
	#define	LINEIN_DETECT_PORT_OE		GPIO_E_OE
	#define LINEIN_DETECT_PORT_IE		GPIO_E_IE
	#define	LINEIN_DETECT_PORT_PU		GPIO_E_PU
	#define	LINEIN_DETECT_PORT_PD		GPIO_E_PD	
#if !(defined(AU6210K_DEMO))
	//#define LINEIN_DETECT_BIT			(1 << 2)
#endif
#endif
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////////////			
// 旋转开关IO配置(coding key pin configure)
// CODING_KEY_A_BIT ~ CODING_KEY_B_BIT：旋转开关的A,B引脚配置，比如D7,D3
////////////////////////////////////////////////////////////////////////////////////////////////////////////			
#ifdef FUNC_CODING_KEY_EN	
	#define CODING_KEY_A_PORT_IN	GPIO_D_IN	
	#define CODING_KEY_A_PORT_IE	GPIO_D_IE
	#define CODING_KEY_A_PORT_OE	GPIO_D_OE			
	#define CODING_KEY_A_PORT_PU	GPIO_D_PU 
	#define CODING_KEY_A_PORT_PD	GPIO_D_PD	
	#define CODING_KEY_A_PORT_DS	GPIO_D_DS	
	#define	CODING_KEY_A_BIT		(1 << 7) 
	
	#define	CODING_KEY_B_PORT_IN	GPIO_D_IN 	
	#define CODING_KEY_B_PORT_IE	GPIO_D_IE
	#define	CODING_KEY_B_PORT_OE	GPIO_D_OE 			
	#define	CODING_KEY_B_PORT_PU	GPIO_D_PU 
	#define	CODING_KEY_B_PORT_PD	GPIO_D_PD 		
	#define CODING_KEY_B_PORT_DS	GPIO_D_DS	
	#define	CODING_KEY_B_BIT		(1 << 3)

	//中断参数配置
	#define CODING_KEY_A_PORT_PTRIG		GPIO_D_SEP_PTRIG	
	#define CODING_KEY_A_PORT_INTC		GPIO_D_SEP_INTC	
	#define CODING_KEY_A_PORT_INTE		GPIO_D_SEP_INTE	
	#define	CODING_KEY_A_PORT_INTS		GPIO_D_SEP_INTS
#endif 


////////////////////////////////////////////////////////////////////////////////////////////////////////////			
// 外部Mute电路IO配置(external mute pin configure)
// FUNC_EXMUTE_EN：是否支持IO控制声音使能
// EXMUTE_PORT_IE ~ EXMUTE_PORT_OUT：外部Mute引脚配置，比如A6
////////////////////////////////////////////////////////////////////////////////////////////////////////////			
#define FUNC_EXMUTE_EN

#ifdef FUNC_EXMUTE_EN
#if 0
	#define EXMUTE_PORT_IE				GPIO_B_IE
	#define EXMUTE_PORT_OE				GPIO_B_OE
	#define EXMUTE_PORT_PU				GPIO_B_PU
	#define EXMUTE_PORT_PD				GPIO_B_PD
	#define EXMUTE_PORT_BIT				(1 << 6)   //B6
	#define EXMUTE_PORT_OUT				GPIO_B_OUT
#elif defined(AU6210K_JLH_JH82B)
	#define EXMUTE_PORT_IE				GPIO_D_IE
	#define EXMUTE_PORT_OE				GPIO_D_OE
	#define EXMUTE_PORT_PU				GPIO_D_PU
	#define EXMUTE_PORT_PD				GPIO_D_PD
	#define EXMUTE_PORT_BIT				(1 << 6)   //D6
	#define EXMUTE_PORT_OUT				GPIO_D_OUT
#elif 	defined(AU6210K_NR_D_9X_XJ_HTS) ||\
		defined(AU6210K_NR_D_8_CSRBT) || \
		defined(AU6210K_LK_SJ_CSRBT) ||\
		defined(AU6210K_ZB_BT007_CSR)
	#define EXMUTE_PORT_IE				GPIO_D_IE
	#define EXMUTE_PORT_OE				GPIO_D_OE
	#define EXMUTE_PORT_PU				GPIO_D_PU
	#define EXMUTE_PORT_PD				GPIO_D_PD
	#define EXMUTE_PORT_BIT				(1 << 4)   //D6
	#define EXMUTE_PORT_OUT				GPIO_D_OUT
#elif defined(AU6210K_HXX_B002)
	#define EXMUTE_PORT_IE				GPIO_A_IE
	#define EXMUTE_PORT_OE				GPIO_A_OE
	#define EXMUTE_PORT_PU				GPIO_A_PU
	#define EXMUTE_PORT_PD				GPIO_A_PD
	#define EXMUTE_PORT_BIT				(1 << 1)   //A1
	#define EXMUTE_PORT_OUT				GPIO_A_OUT
#else
	#define EXMUTE_PORT_IE				GPIO_A_IE
	#define EXMUTE_PORT_OE				GPIO_A_OE
	#define EXMUTE_PORT_PU				GPIO_A_PU
	#define EXMUTE_PORT_PD				GPIO_A_PD
	#define EXMUTE_PORT_BIT 			(1 << 2)   //A6
	#define EXMUTE_PORT_OUT 			GPIO_A_OUT
#endif
	#if defined(AU6210K_NR_D_9X) || defined(AU6210K_NR_D_8_CSRBT) || defined(AU6210K_ZB_BT007_CSR)
        #define ExMuteOn()	do{ \
							SetGpioRegBit(EXMUTE_PORT_PU, EXMUTE_PORT_BIT); \
							ClrGpioRegBit(EXMUTE_PORT_PD, EXMUTE_PORT_BIT); \
							ClrGpioRegBit(EXMUTE_PORT_IE, EXMUTE_PORT_BIT); \	
							SetGpioRegBit(EXMUTE_PORT_OE, EXMUTE_PORT_BIT); \
							ClrGpioRegBit(EXMUTE_PORT_OUT, EXMUTE_PORT_BIT); \
						}while(0)
	
	    #define ExMuteOff()	do{ \
							SetGpioRegBit(EXMUTE_PORT_PU, EXMUTE_PORT_BIT); \
							ClrGpioRegBit(EXMUTE_PORT_PD, EXMUTE_PORT_BIT); \
							ClrGpioRegBit(EXMUTE_PORT_IE, EXMUTE_PORT_BIT); \	
							SetGpioRegBit(EXMUTE_PORT_OE, EXMUTE_PORT_BIT); \
							SetGpioRegBit(EXMUTE_PORT_OUT, EXMUTE_PORT_BIT); \
						}while(0)
	#else	//高mute
	    #define ExMuteOn()	do{ \
							SetGpioRegBit(EXMUTE_PORT_PU, EXMUTE_PORT_BIT); \
							ClrGpioRegBit(EXMUTE_PORT_PD, EXMUTE_PORT_BIT); \
							ClrGpioRegBit(EXMUTE_PORT_IE, EXMUTE_PORT_BIT); \	
							SetGpioRegBit(EXMUTE_PORT_OE, EXMUTE_PORT_BIT); \
							SetGpioRegBit(EXMUTE_PORT_OUT, EXMUTE_PORT_BIT); \
						}while(0)
	
	    #define ExMuteOff()	do{ \
							SetGpioRegBit(EXMUTE_PORT_PU, EXMUTE_PORT_BIT); \
							ClrGpioRegBit(EXMUTE_PORT_PD, EXMUTE_PORT_BIT); \
							ClrGpioRegBit(EXMUTE_PORT_IE, EXMUTE_PORT_BIT); \	
							SetGpioRegBit(EXMUTE_PORT_OE, EXMUTE_PORT_BIT); \
							ClrGpioRegBit(EXMUTE_PORT_OUT, EXMUTE_PORT_BIT); \
						}while(0)
    #endif
//	#define IsExMuteOn() (!(GetGpioReg(EXMUTE_PORT_OUT) & EXMUTE_PORT_BIT)) 										
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////////////			
// 说明：Headphone检测引脚配置(Headphones in pin configure)
//FUNC_HEADPHONE_DETECT_EN：是否支持HEADPHONE插入和拔除检测使能
//HEADPHONE_DETECT_PORT_IN ~ HEADPHONE_DETECT_BIT：HEADPHONE插入和拔除的检测脚，比如A7
////////////////////////////////////////////////////////////////////////////////////////////////////////////	
#if defined(AU6210K_LK_SJ_CSRBT)
#else
//#define FUNC_HEADPHONE_DETECT_EN
#endif

#ifdef FUNC_HEADPHONE_DETECT_EN
//	#define 03FUNC_HEADPHONE_ADC_DETECT_EN	//利用ADC 口检测耳机插入状态打开此宏定义

#if	0
#define FUNC_HEADPHONE_SINGLE_LED_EN		//耳机检测口与单LED灯口复用时，打开该宏
#endif

#ifdef FUNC_HEADPHONE_ADC_DETECT_EN
	#define HEADPHONE_ADC_PORT_CH	ADC_CHANNEL_B6	//选择耳机检测使用的ADC 端口
//	#define HEADPHONE_ADC_PU_EN		//打开该宏定义, ADC电阻选择上拉电阻设计方式,否则选择下拉电阻方式
#elif 1//defined(AU6210K_CSR_BT)
	#define HEADPHONE_DETECT_PORT_IN 		GPIO_E_IN
	#define HEADPHONE_DETECT_PORT_OE 		GPIO_E_OE
	#define HEADPHONE_DETECT_PORT_DS 		GPIO_E_DS
	#define HEADPHONE_DETECT_PORT_IE 		GPIO_E_IE
	#define HEADPHONE_DETECT_PORT_PU 		GPIO_E_PU
	#define HEADPHONE_DETECT_PORT_PD 		GPIO_E_PD
	#define HEADPHONE_DETECT_BIT 			(1 << 2)
#else
	#define HEADPHONE_DETECT_PORT_IN 		GPIO_A_IN
	#define HEADPHONE_DETECT_PORT_OE 		GPIO_A_OE
	#define HEADPHONE_DETECT_PORT_DS 		GPIO_A_DS
	#define HEADPHONE_DETECT_PORT_IE 		GPIO_A_IE
	#define HEADPHONE_DETECT_PORT_PU 		GPIO_A_PU
	#define HEADPHONE_DETECT_PORT_PD 		GPIO_A_PD
	#define HEADPHONE_DETECT_BIT 			(1 << 2)
#endif
#endif

#if defined(AU6210K_NR_D_9X_XJ_HTS)
#define FUNC_USB_VOLUP_EN	//usb升压
#endif

#ifdef FUNC_USB_VOLUP_EN
	#define USB_VOLUP_PORT_IE				GPIO_D_IE
	#define USB_VOLUP_PORT_OE				GPIO_D_OE
	#define USB_VOLUP_PORT_PU				GPIO_D_PU
	#define USB_VOLUP_PORT_PD				GPIO_D_PD
	//#define USB_VOLUP_PORT_BIT			(1 << 2)   //A0
	#define USB_VOLUP_PORT_OUT			GPIO_D_OUT
	#define USB_VOLUP_On()	do{ \
										SetGpioRegBit(POWER_AMPLIFIER_PORT_PU, POWER_AMPLIFIER_PORT_BIT); \
										ClrGpioRegBit(POWER_AMPLIFIER_PORT_PD, POWER_AMPLIFIER_PORT_BIT); \
										ClrGpioRegBit(POWER_AMPLIFIER_PORT_IE, POWER_AMPLIFIER_PORT_BIT); \	
										SetGpioRegBit(POWER_AMPLIFIER_PORT_OE, POWER_AMPLIFIER_PORT_BIT); \
										SetGpioRegBit(POWER_AMPLIFIER_PORT_OUT, POWER_AMPLIFIER_PORT_BIT); \
									}while(0)
	
	#define USB_VOLUP_Off()	do{ \
										SetGpioRegBit(POWER_AMPLIFIER_PORT_PU, POWER_AMPLIFIER_PORT_BIT); \
										ClrGpioRegBit(POWER_AMPLIFIER_PORT_PD, POWER_AMPLIFIER_PORT_BIT); \
										ClrGpioRegBit(POWER_AMPLIFIER_PORT_IE, POWER_AMPLIFIER_PORT_BIT); \	
										SetGpioRegBit(POWER_AMPLIFIER_PORT_OE, POWER_AMPLIFIER_PORT_BIT); \
										ClrGpioRegBit(POWER_AMPLIFIER_PORT_OUT, POWER_AMPLIFIER_PORT_BIT); \
									}while(0)
#endif


#if !defined(AU6210K_NR_D_8_CSRBT) || defined(AU6210K_LK_SJ_CSRBT) || defined(AU6210K_ZB_BT007_CSR)
#define FUNC_POWER_AMPLIFIER_EN	//AB和D类功放选择
#endif

#ifdef FUNC_POWER_AMPLIFIER_EN
#if 1//defined(AU6210K_NR_D_9X_XJ_HTS)||defined(AU6210K_ZB_BT007_CSR)
    #define POWER_AMPLIFIER_PORT_IE				GPIO_D_IE
	#define POWER_AMPLIFIER_PORT_OE				GPIO_D_OE
	#define POWER_AMPLIFIER_PORT_PU				GPIO_D_PU
	#define POWER_AMPLIFIER_PORT_PD				GPIO_D_PD
	#define POWER_AMPLIFIER_PORT_BIT			(1 << 3)   //A0
	#define POWER_AMPLIFIER_PORT_OUT			GPIO_D_OUT
#else
	#define POWER_AMPLIFIER_PORT_IE				GPIO_E_IE
	#define POWER_AMPLIFIER_PORT_OE				GPIO_E_OE
	#define POWER_AMPLIFIER_PORT_PU				GPIO_E_PU
	#define POWER_AMPLIFIER_PORT_PD				GPIO_E_PD
	#define POWER_AMPLIFIER_PORT_BIT			(1 << 1)   //A0
	#define POWER_AMPLIFIER_PORT_OUT			GPIO_E_OUT
#endif	
	#define ABPowerAmplifierOn()	do{ \
										SetGpioRegBit(POWER_AMPLIFIER_PORT_PU, POWER_AMPLIFIER_PORT_BIT); \
										ClrGpioRegBit(POWER_AMPLIFIER_PORT_PD, POWER_AMPLIFIER_PORT_BIT); \
										ClrGpioRegBit(POWER_AMPLIFIER_PORT_IE, POWER_AMPLIFIER_PORT_BIT); \	
										SetGpioRegBit(POWER_AMPLIFIER_PORT_OE, POWER_AMPLIFIER_PORT_BIT); \
										SetGpioRegBit(POWER_AMPLIFIER_PORT_OUT, POWER_AMPLIFIER_PORT_BIT); \
									}while(0)
	
	#define ABPowerAmplifierOff()	do{ \
										SetGpioRegBit(POWER_AMPLIFIER_PORT_PU, POWER_AMPLIFIER_PORT_BIT); \
										ClrGpioRegBit(POWER_AMPLIFIER_PORT_PD, POWER_AMPLIFIER_PORT_BIT); \
										ClrGpioRegBit(POWER_AMPLIFIER_PORT_IE, POWER_AMPLIFIER_PORT_BIT); \	
										SetGpioRegBit(POWER_AMPLIFIER_PORT_OE, POWER_AMPLIFIER_PORT_BIT); \
										ClrGpioRegBit(POWER_AMPLIFIER_PORT_OUT, POWER_AMPLIFIER_PORT_BIT); \
									}while(0)
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////			
// I2C IO配置(IIC pin configure)
////////////////////////////////////////////////////////////////////////////////////////////////////////////			
// I2C SDA and CLK pin define	
#define SDIO_TO_OTHER_GPIO	0	// I2C总线使用独立GPIO
#define SDIO_TO_A3A4A5	1	// I2C总线与SD卡总线A3A4A5复用
#define SDIO_TO_A0E2E3	2	// I2C总线与SD卡总线A0E2E3复用
#define SDIO_TO_D5D6D7	3	// I2C总线与SD卡总线D5D6D7复用

#define I2C_PIN_CONFIGURE	SDIO_TO_OTHER_GPIO	

#if (I2C_PIN_CONFIGURE == SDIO_TO_A3A4A5)
	//SCL: GPIO_A[4]
	#define PORT_OUT_SCL		GPIO_A_OUT
	#define PORT_IN_SCL			GPIO_A_IN
	#define PORT_IE_SCL			GPIO_A_IE
	#define PORT_OE_SCL			GPIO_A_OE
	#define PORT_DS_SCL			GPIO_A_DS
	#define PORT_PU_SCL			GPIO_A_PU
	#define PORT_PD_SCL			GPIO_A_PD
	#define	MASK_BIT_SCL 		(1 << 4)		
	
	//SDA: GPIO_A[3]
	#define PORT_OUT_SDA		GPIO_A_OUT
	#define PORT_IN_SDA			GPIO_A_IN
	#define PORT_IE_SDA			GPIO_A_IE
	#define PORT_OE_SDA			GPIO_A_OE
	#define PORT_DS_SDA			GPIO_A_DS
	#define PORT_PU_SDA			GPIO_A_PU
	#define PORT_PD_SDA			GPIO_A_PD
	#define	MASK_BIT_SDA 		(1 << 3)

#elif (I2C_PIN_CONFIGURE == SDIO_TO_A0E2E3)

#elif (I2C_PIN_CONFIGURE == SDIO_TO_D5D6D7)

#else

#ifdef AU6210K_NR_D_8_CSRBT //pt231x
//SCL: GPIO_B[2]
	#define PORT_OUT_SCL		GPIO_A_OUT
	#define PORT_IN_SCL			GPIO_A_IN
	#define PORT_IE_SCL			GPIO_A_IE
	#define PORT_OE_SCL			GPIO_A_OE
	#define PORT_DS_SCL			GPIO_A_DS
	#define PORT_PU_SCL			GPIO_A_PU
	#define PORT_PD_SCL			GPIO_A_PD
	#define	MASK_BIT_SCL 		(1 << 6)		
	
	//SDA: GPIO_B[3]
	#define PORT_OUT_SDA		GPIO_A_OUT
	#define PORT_IN_SDA			GPIO_A_IN
	#define PORT_IE_SDA			GPIO_A_IE
	#define PORT_OE_SDA			GPIO_A_OE
	#define PORT_DS_SDA			GPIO_A_DS
	#define PORT_PU_SDA			GPIO_A_PU
	#define PORT_PD_SDA			GPIO_A_PD
	#define	MASK_BIT_SDA 		(1 << 7)
#else
	//SCL: GPIO_B[2]
	#define PORT_OUT_SCL		GPIO_B_OUT
	#define PORT_IN_SCL			GPIO_B_IN
	#define PORT_IE_SCL			GPIO_B_IE
	#define PORT_OE_SCL			GPIO_B_OE
	#define PORT_DS_SCL			GPIO_B_DS
	#define PORT_PU_SCL			GPIO_B_PU
	#define PORT_PD_SCL			GPIO_B_PD
	#define	MASK_BIT_SCL 		(1 << 2)		
	
	//SDA: GPIO_B[3]
	#define PORT_OUT_SDA		GPIO_B_OUT
	#define PORT_IN_SDA			GPIO_B_IN
	#define PORT_IE_SDA			GPIO_B_IE
	#define PORT_OE_SDA			GPIO_B_OE
	#define PORT_DS_SDA			GPIO_B_DS
	#define PORT_PU_SDA			GPIO_B_PU
	#define PORT_PD_SDA			GPIO_B_PD
	#define	MASK_BIT_SDA 		(1 << 3)
#endif	
#endif	

#ifdef AU6210K_NR_D_8_CSRBT //D8音效IC选择
#define FUNC_NPCA110X_EN
//#define FUNC_PT231X_EN
#endif

#ifdef FUNC_PT231X_EN
#define FUNC_PT231X_IIC     //PT231X用iic通讯
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////			
// Define system volume range.
////////////////////////////////////////////////////////////////////////////////////////////////////////////			
#if defined(AU6210K_DEMO)||	defined(AU6210K_LK_SJ_CSRBT) || defined(AU6210K_ZB_BT007_CSR)
#define	VOLUME_MAX		32	
#define	VOLUME_INIT		25	
#elif defined(AU6210K_NR_D_8_CSRBT) 
#define	VOLUME_MAX		42	
#define	VOLUME_INIT		38	
#elif defined(AU6210K_NR_D_9X_XJ_HTS) 
#define	VOLUME_MAX		16		
#define	VOLUME_INIT		16		
#endif
#define	VOLUME_MIN		0


////////////////////////////////////////////////////////////////////////////////////////////////////////////			
// 设备模式列表，无需修改
// 如果客户需要增加新的工作模式，请添加在SYS_MODE_OTPDEV之后
////////////////////////////////////////////////////////////////////////////////////////////////////////////			
typedef	enum _SYS_MODE
{
	SYS_MODE_NONE,
	SYS_MODE_USB,
	SYS_MODE_SD,
	SYS_MODE_LINEIN,
	SYS_MODE_RADIO,
	SYS_MODE_RTC,
	SYS_MODE_AUDIO,			//单独的USB声卡模式
	SYS_MODE_READER, 		//单独的USB读卡器模式
	SYS_MODE_AUDIO_READER,	//一线通模式，PC同时识别出USB声卡和USB读卡器
	SYS_MODE_SLEEP,
	SYS_MODE_STANDBY,
	SYS_MODE_SPIDEV,
	SYS_MODE_BLUETOOTH,
	SYS_MODE_OTPDEV

} SYS_MODE;

#endif

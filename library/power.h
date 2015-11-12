#ifndef __POWER_H__
#define __POWER_H__


// LDOIN检测电压范围
#define 	LDOIN_MIN_VOL		3000	    //最低检测电压3.00V
#define 	LDOIN_MAX_VOL		4600	    //最高检测电压4.60V


// 能将系统从Powerdown状态唤醒的唤醒源标志
// 'WIP'是WAKEUP IN POWERDOWN的缩写
#define 	WIP_SRC_LP_RTC			0x01				
#define		WIP_SRC_PWR_KEY 		0x02


//定义芯片8s reset模式，软开关还是硬开关
// 'CRM' CHIP RESET MODE 的缩写
//可以隐藏到lib中，后续再做优化  
#define CRM_IN_PUSH_BUTTON		0x01//按钮开关(软开关)
#define CRM_IN_SLIDE_SWITCH		0x00//拨动开关(硬开关)


// return votage at LDOIN pin, unit is mV.
// 3000mV--4600mV.
WORD GetLdoinVoltage(VOID);

// 如果系统设计中允许LP_RTC模块采用闹钟方式启动片内主电源唤醒系统运行，
// 调用此函数，允许LP_RTC作为系统从powerdown状态下唤醒系统的唤醒源。
// 此外，要实现这个功能，LP_RTC需要正常工作，并且配置了闹钟功能等。
VOID SetWakeupLpRtcInPD(VOID);

//清除在POWERDOWN状态下低功耗RTC唤醒芯片功能
//调用该函数需要慎重
VOID ClrWakeupLpRtcInPD(VOID);

//从PowerKey按下到s_onoff状态变化的响应时间
//默认值为0x63-->0.8s，最小8ms，最大约2s，单位8ms
VOID SetChipResponseTime(BYTE Cnt);

//1 indicates PAD_ONKEY = 1 lasts for 8s can reset system, 
//0 indicates PAD_ONKEY = 0 && mcu_off_seq = 0 lasts for 8s can reset system.
//byPass模式或者硬开关，需要设置为0
//软开关模式时，需要设置为1
VOID SetChip8SRstMode(BYTE Mode);

// Set chip go to lowest power consumption (power down) mode.
// 执行该函数后，芯片的主电源LDO33O和COREVDD电源将关闭，芯片进入POWER DOWN状态
VOID GoToPDMode(VOID);

// 从powerdown状态唤醒系统后，此函数获得是哪一个触发源触发了该行为
BYTE GetWakeupFlgFromPD(VOID);


#endif

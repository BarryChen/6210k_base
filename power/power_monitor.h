#ifndef __POWER_MONITOR_H__
#define __POWER_MONITOR_H__


/*
**********************************************************
*					函数声明
**********************************************************
*/
//
//电能监视初始化
//实现系统启动过程中的低电压检测处理，以及配置充电设备接入检测IO等
//

typedef enum _PWR_MNT_DISP
{
	PWR_MNT_DISP_NONE = 0,
	PWR_MNT_DISP_CHARGE,		 
	PWR_MNT_DISP_HIGH_V, 
	PWR_MNT_DISP_MID_V, 
	PWR_MNT_DISP_LOW_V, 
	PWR_MNT_DISP_EMPTY_V, 
	PWR_MNT_DISP_SYS_OFF
	 
} PWR_MNT_DISP;

VOID PowerMonitorInit(VOID);

//
//系统电源状态监控和处理
//系统启动后如果LDOIN大于开机电压，放在系统正常运行中监测LDOIN
//
VOID PowerMonitor(VOID);

VOID PowerMonitorDisp(VOID);
PWR_MNT_DISP GetPwrDisp(void);


#endif

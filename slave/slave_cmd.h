#ifndef _SLAVE_CMD_H_
#define _SLAVE_CMD_H_


extern BYTE gSlaveCmd[];
#define gSlaveCmdData		(&gSlaveCmd[3]) 
#define gSlaveCmdCodeH		(gSlaveCmd[1]) 
#define gSlaveCmdCodeL		(gSlaveCmd[2]) 

// Initialize 8051 ex UART
VOID SlaveUartInit();

VOID OnDataReceived();

BOOL SlaveRcvCmd();
VOID BluetoothRefreshStatus();
VOID SlaveSendResp(WORD Cmd, BYTE* Buf, BYTE Len);
VOID TESTSlaveSendRespSecond();
#endif
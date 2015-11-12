#ifndef  _SLAVE_CTRL_H_
#define	 _SLAVE_CTRL_H_

extern BOOL gIsMcuFileFind;
extern FILE gMcuUpdataFile;

VOID SlaveRadioRetCurrentStatus(VOID);
VOID UpdateMcu();

VOID SlaveCtrlInit();

VOID SlaveStateCtrl();

#endif

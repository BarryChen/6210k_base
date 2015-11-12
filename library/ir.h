#ifndef __IR_H__
#define	__IR_H__

typedef enum			  
{
	IR_INPUT_GPIOB6,
	IR_INPUT_GPIOE0,

}  IRGPIO_INPUT_SEL;

VOID IrInit(VOID);

VOID IRDisable(VOID);

//选择IR的端口GPIOB6，GPIOE0
//IR的硬件不支持高压输入，IR接收头供电必须为3.3v 
VOID IRChannelSel(IRGPIO_INPUT_SEL IRChannel);

BOOL IrIsKeyCome(VOID);

DWORD IrGetKeyCode(VOID);

BOOL IrIsContinuePrs(VOID);

VOID IrIgnoreLeadHeader(BOOL IsIgnore);

//sleep 后从IR状态唤醒时获取的IR键值
DWORD IrKeyDataVal(VOID);

#endif

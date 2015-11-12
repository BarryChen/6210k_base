#include <reg51.h>										//include public header files
#include <string.h>

#include "type.h"										//include project header files
#include "syscfg.h"
#include "utility.h"
#include "debug.h"
#include "ir_key.h"
#include "adc_key.h"
#include "sysctrl.h"
#include "message.h"


#define	MAX_FIFO_DEPTH	8

typedef struct _MESSAGE_FIFO
{
 	BYTE		IndexHead;
	BYTE		Count;
	MESSAGE	Buf[MAX_FIFO_DEPTH];

} MESSAGE_FIFO;

MESSAGE_FIFO MsgFifos[MSG_FIFO_COUNT];


// message module initial
VOID MessageInit(VOID)							
{
	memset(&MsgFifos, 0, sizeof(MsgFifos));
}


// send message process
BOOL MessageSend(BYTE MsgFifoIndex, BYTE Event)							
{	
	MESSAGE_FIFO* pMsgFifo = &MsgFifos[MsgFifoIndex]; 

	if(pMsgFifo->Count >= MAX_FIFO_DEPTH)		//fifo full
	{
//		DBG(("fifo %bu is full!\n", MsgFifoIndex));
		return FALSE;
	}	
	pMsgFifo->Buf[(pMsgFifo->IndexHead + (pMsgFifo->Count++)) % MAX_FIFO_DEPTH] = Event;
	return TRUE;
}


// get message process
//fifo empty return MSG_NONE, else return message
BYTE MessageGet(BYTE MsgFifoIndex)							
{
	MESSAGE Event = MSG_NONE;
	MESSAGE_FIFO* pMsgFifo = &MsgFifos[MsgFifoIndex];

	if(pMsgFifo->Count)			//fifo is not empty
	{
		Event = pMsgFifo->Buf[pMsgFifo->IndexHead];
		pMsgFifo->IndexHead = (pMsgFifo->IndexHead + 1) % MAX_FIFO_DEPTH;
		pMsgFifo->Count--;
	}				
	return Event;
}

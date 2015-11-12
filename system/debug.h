#ifndef __DEBUG_H__
#define __DEBUG_H__

#include "syscfg.h"

#ifdef FUNC_UARTDBG_EN

	#ifndef	LIB_CREATE_EN
		#include <stdio.h>

		#define	DBG(x)			//printf x
		#define	DBG1(x)			printf x
		#define DBGL(x)		
		#define	DBGR(x)
	#else

		#warning	"MV: IN LIBRARY CREATE, DBG() DISABLE!"
		#define	DBG(x)	
		#define DBGL(x)
		#define	DBGR(x)
	#endif

#else

	#define	DBG(x)
	#define	DBG1(x)		
	#define	DBGL(x)
	#define	DBGR(x)

#endif

#endif

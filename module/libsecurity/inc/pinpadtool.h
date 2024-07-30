#ifndef __PINPADTOOL_H__
#define __PINPADTOOL_H__

#include	<string.h>
#include	<stdlib.h>
#include	<ctype.h>
#include	<time.h>
#include	<stdarg.h>



#define ERR_PINPADLIB_BASE		(-3000)
#define ERR_UNKNOWPINPAD_FAIL	(ERR_PINPADLIB_BASE - 1)


#define ERR_PINPAD_BASE 		(-3200)
#define ERR_PINPAD_FAIL 		(ERR_PINPAD_BASE-1)	
#define ERR_PINPAD_QUIT 		(ERR_PINPAD_BASE-2)	
#define ERR_PINPAD_PORT 		(ERR_PINPAD_BASE-3)	
#define ERR_PINPAD_INVAL		(ERR_PINPAD_BASE-4)	
#define ERR_PINPAD_SENDEXRF		(ERR_PINPAD_BASE-5)		
#define ERR_PINPAD_RECEIVEEXRF	(ERR_PINPAD_BASE-6)
#define ERR_PINPAD_NOTRET 		(ERR_PINPAD_BASE-7)	
#define ERR_PINPAD_RECEIVE		(ERR_PINPAD_BASE-8)	
#define ERR_PINPAD_GROUPNO 		(ERR_PINPAD_BASE-9) 		
#define ERR_PINPAD_SERIALNO 	(ERR_PINPAD_BASE-10)	
#define ERR_PINPAD_INITAUX		(ERR_PINPAD_BASE-11)	
#define ERR_PINPAD_PARAM 		(ERR_PINPAD_BASE-12)	
#define ERR_PINPAD_TIMEOUT		(ERR_PINPAD_BASE-13)	
#define ERR_PINPAD_SEND			(ERR_PINPAD_BASE-14)		
#define ERR_PINPAD_NOTSUPPORT	(ERR_PINPAD_BASE-15)	
#define ERR_PINPAD_NOTCONTACT	(ERR_PINPAD_BASE-16)	
#define ERR_PINPAD_NOSUPPORTMODE 	(ERR_PINPAD_BASE-17)	
#define ERR_PINPAD_PINDESERROR (ERR_PINPAD_BASE-18)	
#define ERR_PINPAD_PINUNDESERROR 	(ERR_PINPAD_BASE-19)
#define ERR_PINPAD_PININITERROR (ERR_PINPAD_BASE-20)
#define ERR_PINPAD_NOSUPPORT_WK					(ERR_PINPAD_BASE-21)


#define ERR_NAPI_ERR_BASE	(-3600)
#define ERR_KEYERASE		(ERR_NAPI_ERR_BASE - 1)		
#define ERR_LOADKEY			(ERR_NAPI_ERR_BASE - 2)		
#define ERR_GETLCDSIZE		(ERR_NAPI_ERR_BASE - 3)		
#define ERR_GETPIN			(ERR_NAPI_ERR_BASE - 4)		
#define ERR_GETPINRESULT	(ERR_NAPI_ERR_BASE - 5)		
#define ERR_GETMAC			(ERR_NAPI_ERR_BASE - 6)		
#define ERR_CALCDES			(ERR_NAPI_ERR_BASE - 7)		
#define ERR_ALGDES			(ERR_NAPI_ERR_BASE - 8)		

#define ESC	0x1b

void ProTraceSecurity(char* lpszFormat, ...);
void ProGetTraceInfo(char* buf);

#define PINPAD_TRACE_SECU(fmt,args...) PubDebug("[SECU][%s][%s][%d]>>>"""fmt,__FILE__, __FUNCTION__, __LINE__,##args)
#define PINPAD_TRACEHEX_SECU(buf,len,title,args...) PubDebugData("[%s][%s][%d]>>>"""title,buf,len,__FILE__, __FUNCTION__, __LINE__,##args)

extern int ProSetSecurityErrCode(int nErr , int nNapiErr);
#endif 

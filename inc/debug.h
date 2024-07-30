/***************************************************************************
** Copyright (c) 2019 Newland Payment Technology Co., Ltd All right reserved   
** File name:  debug.h
** File indentifier: 
** Synopsis:  
** Current Verion:  v1.0
** Auther: sunh
** Complete date: 2016-9-5
** Modify record: 
** Modify date: 
** Version: 
** Modify content: 
***************************************************************************/
#ifndef _DEBUG_H_
#define _DEBUG_H_

#include "ltool.h"

extern int Debug(const char *pszFile, const char *pszFunc, const int nLine, const int nRes);
extern void DispTrace(char* lpszFormat, ...);
extern void DispTraceHex(char* pszHexBuf, int nLen, char* lpszFormat, ...);
extern int emvDebug(const char *psLog, uint nLen);
extern int MenuEmvSetDebug(void);

#define TRACE_HEX(buf,len,title,args...) PubDebugData("[%s][%s][%d]>>>"""title,buf,len,__FILE__, __FUNCTION__, __LINE__,##args)
#define TRACE(fmt,args...) PubDebug("[%s][%s][%d]>>>"""fmt,__FILE__, __FUNCTION__, __LINE__,##args)

#define ASSERT(e) Debug(__FILE__, __FUNCTION__, __LINE__,e)

#define ASSERT_FAIL(e) \
		{\
			int nTmpRet = e;\
			if (nTmpRet!=APP_SUCC)\
			{\
				TRACE("%d",nTmpRet);\
				return APP_FAIL;\
			}\
		}
	
#define ASSERT_QUIT(e) \
	{\
		int nTmpRet = e;\
		if (nTmpRet!=APP_SUCC)\
		{\
			TRACE("%d",nTmpRet);\
			return APP_QUIT;\
		}\
	}
	
#define ASSERT_CONTINUE(e) \
	{\
		int nTmpRet = e;\
		if (nTmpRet!=APP_SUCC)\
		{\
			TRACE("%d",nTmpRet);\
			continue;\
		}\
	}

#define ASSERT_HANGUP_QUIT(e) \
	if (ASSERT(e) != APP_SUCC)\
	{\
		CommHangUp();\
		return APP_QUIT;\
	}

#define ASSERT_HANGUP_FAIL(e) \
	if (ASSERT(e) != APP_SUCC)\
	{\
		CommHangUp();\
		return APP_FAIL;\
	}
	
#define ASSERT_UPCARD_POWERDOWN_FAIL(e)\
	if (ASSERT(e) != APP_SUCC)\
	{\
		PubMsgDlg(NULL, "Read Fail", 3, 3);\
		PubCardPowerDown();\
		return APP_FAIL;\
	}
#define ASSERT_RETURNCODE(e) \
	{\
		int nTemp=e;\
		if (nTemp != APP_SUCC)\
		{\
			return nTemp;\
		}\
	}
#define ASSERT_RETURN(e)\
	{\
		int nAssertRtn;\
		if((nAssertRtn=ASSERT(e)) != APP_SUCC)\
		{\
			return nAssertRtn;\
		}\
	}	

#endif




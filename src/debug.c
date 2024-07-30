/***************************************************************************
** Copyright (c) 2019 Newland Payment Technology Co., Ltd All right reserved   
** File name:  debug.c
** File indentifier: 
** Synopsis:  
** Current Verion:  v1.0
** Auther: sunh
** Complete date: 2016-7-4
** Modify record: 
** Modify date: 
** Version: 
** Modify content: 
***************************************************************************/
#include "apiinc.h"
#include "libapiinc.h"
#include "appinc.h"

/**
** brief: for debug
** param [in]: 
** param [out]: 
** return: 
** auther: 
** date: 2016-7-4
** modify: 
*/
int Debug(const char *pszFile, const char *pszFunc, const int nLine, const int nRes)
{
	STAPPCOMMPARAM stCommPara;

	GetAppCommParam(&stCommPara);
	if(stCommPara.cCommType == COMM_RS232)
	{
		return nRes;
	}
	if (nRes != APP_SUCC)
	{
		PubDebug("[%s][%s][%s][%d]>>>%d", APP_NAMEDESC, pszFile, pszFunc,nLine,nRes);
	}
	return nRes;
}

/**
* @brief Display msg int the screen
* @param in lpszFormat
* @return	void
*/
void DispTrace(char* lpszFormat, ...)
{
	va_list args;
	char buf[2048] = {0};
	va_start(args, lpszFormat);
	vsprintf(buf, lpszFormat, args);
	PubClearAll();
	NAPI_ScrPrintf(buf);
	PubUpdateWindow();
	PubWaitConfirm(0);
	va_end(args);
}

/**
* @brief Display trace hex
* @param in lpszFormat 
* @return	void
*/
void DispTraceHex(char* pszHexBuf, int nLen, char* lpszFormat, ...)
{
	va_list args;
	int nBuf;
	char buf[2048];
	int i;
	
	va_start(args, lpszFormat);

	nBuf = vsprintf(buf, lpszFormat, args);

	for(i = 0; i < nLen; i++)
	{
		sprintf(buf + nBuf + i * 3, "%02X ", *((char*)pszHexBuf + i));
	}
	PubClearAll();
	PubDispMultLines(DISPLAY_ALIGN_BIGFONT, 1, 1, "%s", buf);
	va_end(args);
	PubWaitConfirm(0);
}

int MenuEmvSetDebug(void)
{
	char *pszItems[] = {
		tr("1.Close"),
		tr("2.Debug"),
		tr("3.Debug All"),
	}; 
	int nSelcItem = 1, nStartItem = 1;

	if (PubGetDebugMode() == DEBUG_NONE)
	{
		PubMsgDlg(tr("EMV DEBUG"), "PLEASE OPEN DEBUG MODE FIRST!", 0, 60);
		return APP_FAIL;
	}
	
	ASSERT_QUIT(PubShowMenuItems(tr("EMV DEBUG"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 0));
	switch(nSelcItem)
	{
		case 1:
			TxnL3SetDebugMode(LV_CLOSE);
			break;
		case 2:
			TxnL3SetDebugMode(LV_DEBUG);
			break;
		case 3:
			TxnL3SetDebugMode(LV_ALL);
			break;
		default :
			return APP_FAIL;
			break;
	}

	return APP_SUCC;
}

int emvDebug(const char *psLog, uint nLen)
{
	PubDebug("%*.*s", nLen, nLen, psLog);
	return 0;
}


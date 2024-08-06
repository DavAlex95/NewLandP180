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

static void dump(char *in_buf, int offset, int num_bytes, int ascii_rep) {
  int index = 0;
  int inMaxChr;
  char temp_buf[10];
  char disp_buf[500];

  if (ascii_rep == TRUE) {
    if (num_bytes > 16) {
      // disp_buf overflow!
      TRACE("dump ERR - num_bytes=%d > 15", num_bytes);
      return;
    }
    inMaxChr = 16;
  } else {
    if (num_bytes > 23) {
      // disp_buf overflow!
      TRACE("dump ERR - num_bytes=%d > 20", num_bytes);
      return;
    }
    inMaxChr = 23;
  }

  memset(temp_buf, 0, sizeof(temp_buf));
  memset(disp_buf, 0, sizeof(disp_buf));
  sprintf(temp_buf, "%04X | ", offset);
  strcat(disp_buf, temp_buf);
  for (index = 0; index < inMaxChr; ++index) {
    memset(temp_buf, 0, sizeof(temp_buf));
    if (index < num_bytes)
      sprintf(temp_buf, "%02X ", (int)(unsigned char)in_buf[offset + index]);
    else
      strcpy(temp_buf, "   ");

    strcat(disp_buf, temp_buf);

    if (!((index + 1) % 8) && ((index + 1) < inMaxChr)) strcat(disp_buf, "  ");
  }

  if (ascii_rep == TRUE) {
    char c;
    strcat(disp_buf, "| ");
    for (index = 0; index < num_bytes; ++index) {
      memset(temp_buf, 0, sizeof(temp_buf));
      c = in_buf[offset + index];
      if (c < 0x20 || c > 0x7E) c = '.';
      sprintf(temp_buf, "%c", c);
      strcat(disp_buf, temp_buf);
    }
  }

  TRACE("%s", disp_buf);
}

void pdump(void *pvdMem, int num_bytes, char *title) {
  int offset = 0;
  int r = 0;
  char *in_buf;

  if (strlen(title) > 0) TRACE("%s (%d):", title, num_bytes);

  if (num_bytes < 0) num_bytes = 0;

  in_buf = pvdMem;
  while (offset < num_bytes) {
    r = num_bytes - offset;
    if (r > 16) r = 16;
    dump(in_buf, offset, r, TRUE);
    offset += r;
  }
}

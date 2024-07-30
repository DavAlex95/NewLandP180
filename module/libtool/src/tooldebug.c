#include <string.h>
#include <stdarg.h>
#include <sys/timeb.h>
#include "ltool.h"
#include "libapiinc.h"
#include "emvl3.h"

#define FILEDIR             "/tmp/"
#define OPENDB_FILE			FILEDIR"ONFDEBUG"
#define OPENDB_PORT			FILEDIR"ONPDEBUG"
#define PORT_USB			FILEDIR"PORTUSB"
#define PORT_RS232			FILEDIR"PORTSERIAL"
#define PORT_PINPAD			FILEDIR"PORTPINPAD"
#define FILEDEBUG			FILEDIR"FLDEBUG.NL"
#define OLDFILEDEBUG		FILEDIR"FLDEBUGOLD.NL"
#define OPENDB_CONSOLE		FILEDIR"CONSOLEDBG"

static char gcInitAux = 0;
static char gcInitUsbAux = 0;

static int ProInitDebugPort(int nPortType);
static void WriteFileLog(const char* psData, int nLen)
{
	int nFd;
	uint unLen;

	PubFsFileSize(FILEDEBUG, &unLen);
	if(unLen > 1024*1024*2)
	{
		PubFsDel(OLDFILEDEBUG);
		PubFsRename(FILEDEBUG, OLDFILEDEBUG);
	}
	if((nFd=PubFsOpen(FILEDEBUG,"w")) <= 0)
	{
		return ;
	}
	PubFsSeek (nFd, (ulong)0, SEEK_END);
	PubFsWrite(nFd, psData, nLen);
	PubFsClose(nFd);
	return ;
}

void PubExportDebugFile()
{
		int nFd, nLen;
	char sBuf[1024+1];
	int nPortNum;

	nPortNum = PubGetDebugPortNum();

	if(ProInitDebugPort(nPortNum) != NAPI_OK)
	{
		return;
	}

	if((nFd=PubFsOpen(OLDFILEDEBUG, "r")) > 0)
	{
		PubFsSeek(nFd, (ulong)0, SEEK_SET);
		do{
			memset(sBuf, 0, sizeof(sBuf));
			nLen = PubFsRead(nFd, sBuf, 1024);
			if (nLen > 0)
			{
                NAPI_PortWrite((PORT_TYPE)nPortNum, (uchar *)sBuf, nLen);
				PubSysMsDelay(100);
			}
		}while(nLen > 0);

		PubFsClose(nFd);
		PubFsDel(OLDFILEDEBUG);
	}

	if((nFd=PubFsOpen(FILEDEBUG, "r")) > 0)
	{
		PubFsSeek(nFd, (ulong)0, SEEK_SET);
		do{
			memset(sBuf, 0, sizeof(sBuf));
			nLen = PubFsRead(nFd, sBuf, 1024);
			if (nLen >0)
			{
                NAPI_PortWrite((PORT_TYPE)nPortNum, (uchar *)sBuf, nLen);
				PubSysMsDelay(100);
			}
		}while(nLen > 0);
		PubFsClose(nFd);
		PubFsDel(FILEDEBUG);
	}

    NAPI_PortClose((PORT_TYPE)nPortNum);
	if (nPortNum == USB_SERIAL)
	{
		gcInitUsbAux = 0;
	}

	return ;
}

/**
* @brief Send debug info to aux
* @param [in] pszBuf
* @param [in] nBufLen
* @return
*/
static void DebugBufToAux(const char *pszBuf, const int nBufLen)
{
	int nIndex;
	int nPortNum;
	int nRet;

	nPortNum = PubGetDebugPortNum();
	if (PubGetDebugMode() == DEBUG_FILE)
	{
		WriteFileLog(pszBuf,nBufLen);
	}
	else if(PubGetDebugMode() == DEBUG_CONSOLE)
	{
		fwrite(pszBuf, sizeof(char), nBufLen, stdout);
		fflush(stdout);
	}
	else
	{
		if(ProInitDebugPort(PubGetDebugPortNum()) != NAPI_OK) /* NAPI suggest that if do write or read need to init */
		{
			return;
		}
		nIndex = 0;
		while(nIndex < nBufLen)
		{
			if ((nIndex+1024) < nBufLen)
			{
                nRet = NAPI_PortWrite((PORT_TYPE)nPortNum, (uchar *)pszBuf+nIndex, 1024);
				nIndex += 1024;
				PubSysMsDelay(100);
			}
			else
			{
                nRet = NAPI_PortWrite((PORT_TYPE)nPortNum, (uchar *)pszBuf+nIndex, nBufLen - nIndex);
				nIndex = nBufLen;
				PubSysMsDelay(20);
			}
			if (nRet != NAPI_OK) {
				gcInitUsbAux = 0;
			}
		}
	}
	return ;
}

static void DebugSendHex(const void * psRecvBuf, int nLen)
{
	int i=0, nStrLen, nIndex;
	char szBuf[1024*3+1]={0};

	for(i=0; i<nLen; i++)
	{
		sprintf(szBuf+i*3, "%02X ", *((char*)psRecvBuf+i));
	}
	nIndex = 0;
	nStrLen = nLen*3;
	while(nIndex < nStrLen)
	{
		if ((nIndex+1024) < nStrLen)
		{
			DebugBufToAux(szBuf+nIndex, 1024);
			nIndex += 1024;
		}
		else
		{
			DebugBufToAux(szBuf+nIndex, nStrLen-nIndex);
			nIndex = nStrLen;
		}
	}
}

void PubDebugData(const char* pszTitle,const void* pData,int nLen,...)
{
	int nIndex;
	char sBuf[256]={0};
	struct timeb NowTime;

	if (PubGetDebugMode() == DEBUG_NONE || pData == NULL)
	{
		return;
	}

	ftime(&NowTime);
	strftime(sBuf, sizeof(sBuf)-1, "\r\n[%m-%d %H:%M:%S", localtime(&NowTime.time));
	sprintf(sBuf+strlen(sBuf), ".%3.3d]", NowTime.millitm);
	DebugBufToAux(sBuf, strlen(sBuf));



	if (pszTitle != NULL)
	{
		va_list args;
		memset(sBuf,0x00,sizeof(sBuf));
		va_start(args, nLen);
		if(vsprintf(sBuf, pszTitle, args) > sizeof(sBuf)-1)
		{
			return;
		}
		va_end(args);
		DebugBufToAux(sBuf, strlen(sBuf));
	}

	nIndex = 0;
	while(nIndex < nLen)
	{
		if ((nIndex+1024) < nLen)
		{
			DebugSendHex(pData+nIndex, 1024);
			nIndex += 1024;
		}
		else
		{
			DebugSendHex(pData+nIndex, nLen-nIndex);
			nIndex = nLen;
		}
	}
}


void PubDebug(char* lpszFormat, ...)
{
    if (PubGetDebugMode() == DEBUG_NONE)
	{
		return ;
    }

	int nLen;
	va_list args;
	struct timeb NowTime;
	char szBuf[4096]={0};

	ftime(&NowTime);
	strftime(szBuf, sizeof(szBuf)-1, "\r\n[%m-%d %H:%M:%S", localtime(&NowTime.time));
	sprintf(szBuf+strlen(szBuf), ".%3.3d]", NowTime.millitm);
	va_start(args, lpszFormat);
	nLen = strlen(szBuf);
	if(vsprintf(szBuf+nLen, lpszFormat, args) > sizeof(szBuf)-nLen-1)
	{
		return;
	}
	va_end(args);
	DebugBufToAux(szBuf, strlen(szBuf));
}

void PubBufToAux(const char *pszBuf, const int nBufLen)
{
	if (PubGetDebugMode() == DEBUG_NONE)
	{
		return ;
    }
	DebugBufToAux(pszBuf, nBufLen);
}

/**
* @brief Get debug mode (port or file)
* @return
* @li DEBUG_NONE: Debug closed
* @li DEBUG_PORT: Port mode (including serial and USB)
* @li DEBUG_FILE: Debug mode: Debug file
*/
int PubGetDebugMode(void)
{
	if (PubFsExist(OPENDB_PORT) == NAPI_OK)
    {
        return DEBUG_PORT;
    }
    else if (PubFsExist(OPENDB_FILE) == NAPI_OK)
    {
        return DEBUG_FILE;
    }
    else if (PubFsExist(OPENDB_CONSOLE) == NAPI_OK)
    {
        return DEBUG_CONSOLE;
    }
	return DEBUG_NONE;
}


/**
* @brief Set debug mode
* @param [in] nMode  debug mode DEBUG_NONE---Close  DEBUG_PORT---PORT DEBUG_FILE---File
* @return
* @li APP_FAIL
* @li APP_SUCC
*/
int PubSetDebugMode(int nMode)
{
    int nFileHandle = 0, nPort;

    switch(nMode & 0x0F)
    {
    case DEBUG_NONE:
        PubFsDel(OPENDB_FILE);
		PubFsDel(OPENDB_PORT);
		PubFsDel(FILEDEBUG);
		PubFsDel(OLDFILEDEBUG);
		PubFsDel(OPENDB_CONSOLE);
		nPort = PubGetDebugPortNum();
        NAPI_PortClose((PORT_TYPE)nPort);
		if (nPort == USB_SERIAL) {
			gcInitUsbAux = 0;
		}
		
		return APP_SUCC;
		break;
	case DEBUG_PORT:
		PubFsDel(OPENDB_FILE);
		PubFsDel(OPENDB_CONSOLE);
		if((nFileHandle = PubFsOpen(OPENDB_PORT, "w")) <= 0)
		{
			return APP_FAIL;
		}
		PubFsClose(nFileHandle);
		break;
	case DEBUG_FILE:
		PubFsDel(OPENDB_PORT);
		PubFsDel(OPENDB_CONSOLE);
		if((nFileHandle = PubFsOpen(OPENDB_FILE, "w")) <= 0)
		{
			return APP_FAIL;
		}
		PubFsClose(nFileHandle);
		if(gcInitAux == 1)
		{
			//NDK_PortClose(PubGetDebugPortNum());
			gcInitAux = 0;
		}
		if(PubGetDebugPortNum() == APP_FAIL)
		{
			PubSetDebugPort(RS232);
		}
		break;
	case DEBUG_CONSOLE:
        PubFsDel(OPENDB_FILE);
		PubFsDel(OPENDB_PORT);
		if((nFileHandle = PubFsOpen(OPENDB_CONSOLE, "w")) <= 0)
		{
			return APP_FAIL;
		}
		PubFsClose(nFileHandle);
		break;
		
	default:
		return APP_FAIL;
		break;
	}
	if(PubGetDebugLevel() == APP_FAIL)
	{
		PubSetDebugLevel(DEBUG_LEVEL_NORMAL);
	}
	return APP_SUCC;
}

/**
* @brief Get debug level
* @return
* @li DEBUG_LEVEL_NORMAL:  	Basic debug log
* @li DEBUG_LEVEL_WARNING:  Basic + Warning log
* @li DEBUG_LEVEL_ALL:  	All log
* @li APP_FAIL:  Not set
*/
int PubGetDebugLevel(void)
{
	int nFileHandle = 0;
	char cDebugLevel=0;

	if (PubFsExist(OPENDB_PORT) == NAPI_OK)
    {
        if((nFileHandle = PubFsOpen(OPENDB_PORT, "w")) <= 0)
		   return APP_FAIL;
    }
    else if (PubFsExist(OPENDB_FILE) == NAPI_OK)
    {
        if((nFileHandle = PubFsOpen(OPENDB_FILE, "w")) <= 0)
		   return APP_FAIL;
    }
	else if (PubFsExist(OPENDB_CONSOLE) == NAPI_OK)
    {
        if((nFileHandle = PubFsOpen(OPENDB_CONSOLE, "w")) <= 0)
		   return APP_FAIL;
    }
	else
	{
		return APP_FAIL;
	}

	PubFsRead(nFileHandle, &cDebugLevel, 1);
    PubFsClose(nFileHandle);
	if(cDebugLevel != DEBUG_LEVEL_NORMAL
		&& cDebugLevel != DEBUG_LEVEL_WARNING
		&& cDebugLevel != DEBUG_LEVEL_ALL)
	{
		return DEBUG_LEVEL_NORMAL;
	}
	return cDebugLevel;
}


/**
* @brief Set debug level
* @param [in] nLevel Debug level
*					DEBUG_LEVEL_NORMAL:	Basic debug log
*                   DEBUG_LEVEL_WARNING:Basic + Warning log
*                   DEBUG_LEVEL_ALL:  	All log
* @return
* @li APP_FAIL
* @li APP_SUCC
*/
int PubSetDebugLevel(int nLevel)
{
	int nFileHandle = 0;
	char cDebugLevel=0;

	if (PubFsExist(OPENDB_PORT) == NAPI_OK)
    {
        if((nFileHandle = PubFsOpen(OPENDB_PORT, "w")) <= 0)
		   return APP_FAIL;
    }
    else if (PubFsExist(OPENDB_FILE) == NAPI_OK)
    {
        if((nFileHandle = PubFsOpen(OPENDB_FILE, "w")) <= 0)
		   return APP_FAIL;
    }
	else if (PubFsExist(OPENDB_CONSOLE) == NAPI_OK)
    {
        if((nFileHandle = PubFsOpen(OPENDB_CONSOLE, "w")) <= 0)
		   return APP_FAIL;
    }
	else
	{
		return APP_FAIL;
	}
	if(cDebugLevel == DEBUG_LEVEL_NORMAL
		|| cDebugLevel == DEBUG_LEVEL_WARNING
		|| cDebugLevel == DEBUG_LEVEL_ALL)
	{
		cDebugLevel = nLevel;
	}
	PubFsWrite(nFileHandle, &cDebugLevel, 1);
    PubFsClose(nFileHandle);

	return APP_SUCC;

}


/**
* @brief Output debug according to debug level
* @param [in] cLevel 1-3 , 3 is lowest
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author sunh
* @date
*/
int PubDebugSelectly(char cLevel, char* lpszFormat, ...)
{
	char cDebugLevel=0;
	va_list args;
	char szBuf[4096];

	if (PubGetDebugMode() == DEBUG_NONE)
	{
		return APP_FAIL;
    }

	cDebugLevel = PubGetDebugLevel();
	va_start(args, lpszFormat);
	vsprintf(szBuf, lpszFormat, args);
	va_end(args);

	switch(cLevel)
	{
	case 1://Print all
		if ((cDebugLevel == DEBUG_LEVEL_WARNING) ||(cDebugLevel == DEBUG_LEVEL_ALL))
		{
			PubDebug("%s", szBuf);
		}
		break;
	case 2: //print partly
		if (cDebugLevel == DEBUG_LEVEL_ALL)
		{
			PubDebug("%s", szBuf);
		}
		break;
	case 3://print basic
	default:
		PubDebug("%s", szBuf);
		break;
	}
	return APP_SUCC;
}


/**
* @brief Get debug port number
* @return
* @li PORT_NUM_COM1: RS232
* @li PORT_NUM_PINPAD: PINPAD
* @li PORT_NUM_USB:  USB
* @li APP_FAIL:  Not set
*/
int PubGetDebugPortNum(void)
{
	if (PubFsExist(PORT_RS232) == NAPI_OK)
	{
		return RS232;
	}
	else if (PubFsExist(PORT_PINPAD) == NAPI_OK)
	{
		return PINPAD;
	}
	else if (PubFsExist(PORT_USB) == NAPI_OK)
	{
		return USB_SERIAL;
	}
	return APP_FAIL;
}


/**
* @brief Set debug Port
* @param [in] nPortType Debug port number
*					RS232
*                   PINPAD
*                   USB
* @return
* @li APP_FAIL
* @li APP_SUCC
*/
void PubSetDebugPort(int nPortType)
{
	static int nOldPortNum = -1;
	char szPortFile[20] = {0};
	int nFileHandle;

	if (nPortType == RS232)
	{
		strcpy(szPortFile, PORT_RS232);
		PubFsDel(PORT_USB);
		PubFsDel(PORT_PINPAD);
	}
	else if (nPortType == PINPAD)
	{
		strcpy(szPortFile, PORT_PINPAD);
		PubFsDel(PORT_USB);
		PubFsDel(PORT_RS232);
	}
	else if (nPortType == USB_SERIAL)
	{
		strcpy(szPortFile, PORT_USB);
		PubFsDel(PORT_RS232);
		PubFsDel(PORT_PINPAD);
	}
	else
	{
		return;
	}

	nFileHandle = PubFsOpen(szPortFile, "w");
	if (nFileHandle > 0)
	{
		PubFsClose(nFileHandle);
	}

	if(nOldPortNum != nPortType)
	{
        NAPI_PortClose((PORT_TYPE)nOldPortNum);
		if (nPortType == USB_SERIAL)
		{
			gcInitUsbAux = 0;
		}
	}

	if (ProInitDebugPort(nPortType) != NAPI_OK)
	{
		return;
	}
	nOldPortNum = nPortType;

	return;
}


int ProInitDebugPort(int nPortType)
{
	PORT_SETTINGS PortSettings;
	int nRet;

	PortSettings.BaudRate = BAUD115200;
	PortSettings.DataBits = DATA_8;
	PortSettings.Parity = PAR_NONE;
	PortSettings.StopBits = STOP_1;

	if (nPortType == USB_SERIAL && gcInitUsbAux == 1) {
		return NAPI_OK;
	}

	nRet = NAPI_PortOpen((PORT_TYPE)nPortType, PortSettings);
	if (nRet == NAPI_OK && nPortType == USB_SERIAL) {
		gcInitUsbAux = 1;
	} else {
		gcInitUsbAux = 0;
	}

    return nRet;
}

int PubGetDebugPortLevel(void)
{
	switch (PubGetDebugPortNum())
	{
	case RS232:
		return 0;
		break;
	case PINPAD:
		return 1;
		break;
	case USB_SERIAL:
		return 2;
		break;
	default:
		return 0;
	}
}

void ProGetLogFileAttr(char *pszFilePath, char *pszLogFileName, char *pszOldLogFileName)
{
    if (pszFilePath)
    {
        strcpy(pszFilePath, FILEDIR);
    }

    if (pszLogFileName)
    {
        strcpy(pszLogFileName, FILEDEBUG);
    }

    if (pszOldLogFileName)
    {
        strcpy(pszOldLogFileName, OLDFILEDEBUG);
    }

}


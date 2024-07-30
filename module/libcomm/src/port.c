/**
* @file port.c
* @brief Port module
* @version  1.0
* @author Liug
* @date 2012-06-05
*/
#include <string.h>
#include "commerror.h"
#include "commtool.h"
#include "port.h"
#include "process.h"
#include "libapiinc.h"

#define PORT_MAX_COMMLEN 1024*2  /**<Max length communication buffer*/
#define PORT_DEBUG_LEVEL (3)
#define PORT_TRACE(level, fmt, args...) \
    PubDebugSelectly(level, "[%s][%s][%d]>>>" fmt, __FILE__, __FUNCTION__, __LINE__, ##args)

static PORT_TYPE nAux = RS232;
static char gcIsPortOpen = 0;
static PORT_SETTINGS gstPortSetting;

static void DebugPortParam(STPORTPARAM *pstPortParam)
{
	if (pstPortParam == NULL)
	{
		return;
	}
	PubDebugSelectly(PORT_DEBUG_LEVEL, "========Debug Port Param start===========");
	PubDebugSelectly(PORT_DEBUG_LEVEL, "nAux : %d", pstPortParam->nAux);
	PubDebugSelectly(PORT_DEBUG_LEVEL, "nBaudRate : %d", pstPortParam->nBaudRate);
	PubDebugSelectly(PORT_DEBUG_LEVEL, "nDataBits : %d", pstPortParam->nDataBits);
	PubDebugSelectly(PORT_DEBUG_LEVEL, "nParity : %d", pstPortParam->nParity);
	PubDebugSelectly(PORT_DEBUG_LEVEL, "nStopBits : %d", pstPortParam->nStopBits);
	PubDebugSelectly(PORT_DEBUG_LEVEL, "========Debug Port Param end===========");
}
/**
* @brief Port initialize
* @param [in] pstPortParam
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author Liug
* @date 2012-6-5
*/
int PortInit(STPORTPARAM *pstPortParam)
{
	int nRet;
    PORT_SETTINGS stPortSetting;

    memset(&stPortSetting, 0, sizeof(stPortSetting));
	DebugPortParam(pstPortParam);
	if (pstPortParam == NULL)
	{
		SetCommError(FAIL_RS232_INVAILIDPARAM, 0);
		PORT_TRACE(PORT_DEBUG_LEVEL, "PortInit invalid param");
		return APP_FAIL;
	}

    stPortSetting.BaudRate = (BAUD_RATE_TYPE)pstPortParam->nBaudRate;
    stPortSetting.DataBits =(DATA_BITS_TYPE) pstPortParam->nDataBits;
    stPortSetting.Parity = (PARITY_TYPE)pstPortParam->nParity;
    stPortSetting.StopBits = (STOP_BITS_TYPE)pstPortParam->nStopBits;
    nRet = NAPI_PortOpen((PORT_TYPE)pstPortParam->nAux, stPortSetting);
	if (nRet != NAPI_OK)
	{
		SetCommError(FAIL_RS232_INIT, nRet);
		PORT_TRACE(PORT_DEBUG_LEVEL, "NAPI_PortOpen error[%d]---[Port:%d]",nRet,\
				pstPortParam->nAux);
		return APP_FAIL;
	}
    nAux = (PORT_TYPE)pstPortParam->nAux;
	memcpy(&gstPortSetting, &stPortSetting, sizeof(stPortSetting));
	PORT_TRACE(PORT_DEBUG_LEVEL, "PortInit succ");

	return APP_SUCC;
}

/**
* @brief Clear buffer
* @param
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author Liug
* @date 2012-6-5
*/
int PortClearBuf()
{
	int nRet;

	PORT_TRACE(PORT_DEBUG_LEVEL,"PortClearBuf start");
	nRet = NAPI_PortFlush(nAux);
	if (nRet != NAPI_OK)
	{
		SetCommError(FAIL_RS232_CLEARBUF, nRet);
		PORT_TRACE(PORT_DEBUG_LEVEL, "NAPI_PortFlush error[%d]---[Port:%d].",nRet,nAux);
		return nRet;
	}
	PORT_TRACE(PORT_DEBUG_LEVEL, "PortClearBuf succ");

	return APP_SUCC;
}

/**
* @brief Port connect(open port)
* @param [in] pstServerAddress
* @param [out] pnConnectIndex
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author Liug
* @date 2012-6-5
*/
int PortConnect(const STSERVERADDRESS * pstServerAddress,int *pnConnectIndex,int nTimeOut, int nIsPreConnect)
{
	int nRet = -1;

    COMM_UNUSED(pstServerAddress);
    COMM_UNUSED(pnConnectIndex);
    COMM_UNUSED(nTimeOut);

	PORT_TRACE(PORT_DEBUG_LEVEL, "PreConnect start... [nIsPreConnect: %d][gcIsPortOpen: %d]", nIsPreConnect, gcIsPortOpen);
    nRet = NAPI_PortOpen((PORT_TYPE)nAux, gstPortSetting);
	if (nRet != NAPI_OK)
	{
		SetCommError(FAIL_RS232_INIT, nRet);
		PORT_TRACE(PORT_DEBUG_LEVEL, "NAPI_PortOpen error[%d]---[Port:%d].", nRet, nAux);
		return APP_FAIL;
	}
	PORT_TRACE(PORT_DEBUG_LEVEL, "PortConnect succ: ");

	return APP_SUCC;
}

/**
* @brief Do nothing
* @param
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author Liug
* @date 2012-11-15
*/
int PortGetConnectState()
{
	//PubDebugSelectly(3, "PortGetConnectState succ...");
	return APP_SUCC;
}

/**
* @brief Sending data to port
* @param [in]  psData
* @param [in]  nDataLen
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author Liug
* @date 2012-5-23
*/
int PortWrite(const char *psData,int nDataLen)
{
	int nIndex,nRet;

	PubDebugData("PortWrite start: ", psData, nDataLen);
	nIndex = 0;
	while(nIndex < nDataLen)
	{
		if ((nIndex+PORT_MAX_COMMLEN) < nDataLen)
		{
			nRet = NAPI_PortWrite(nAux, (uchar *)psData+nIndex, PORT_MAX_COMMLEN);
			if (nRet != NAPI_OK)
			{
				SetCommError(FAIL_RS232_WRITE, nRet);
				PORT_TRACE(PORT_DEBUG_LEVEL, "NAPI_PortWrite error[%d]---[Port:%d].", nRet, nAux);
				return APP_FAIL;
			}
			nIndex += PORT_MAX_COMMLEN;
			PubSysMsDelay(100);
		}
		else
		{
			PORT_TRACE(PORT_DEBUG_LEVEL, "[nAux=%d][%d]",nAux, nDataLen - nIndex);
			nRet = NAPI_PortWrite(nAux, (uchar *)psData+nIndex, nDataLen - nIndex );
			if (nRet != NAPI_OK)
			{
				SetCommError(FAIL_RS232_WRITE, nRet);
				PORT_TRACE(PORT_DEBUG_LEVEL, "NAPI_PortWrite error[%d]---[Port:%d].",nRet,nAux);
				return APP_FAIL;
			}
			nIndex = nDataLen;
		}
	}
	PORT_TRACE(PORT_DEBUG_LEVEL, "PortWrite succ");
	return APP_SUCC;
}

/**
* @brief Read data from port
* @param [in]   nLen
* @param [in]   nTimeOut
* @param [out]  psOutData
* @return
* @li >0
* @li Fail
* @li TIMEOUT
* @author Liug
* @date 2012-5-23
*/
int PortRead(int nIsNonBlock, int nLen,int nTimeOut,char *psOutData)
{
	int nRet;
	int nRecvLen=0, nTmpLen = nLen, nMaxRlen = 0;
	uint unOverTime = 0;
	int nKeyCode;

	nMaxRlen = nTmpLen > PORT_MAX_COMMLEN ? PORT_MAX_COMMLEN : nTmpLen;
	PORT_TRACE(PORT_DEBUG_LEVEL, "PortRead start1---[nAux=%d][len:%d][nMaxRlen: %d][nTimeOut %d]", nAux, nLen, nMaxRlen, nTimeOut);
	unOverTime = PubGetOverTimer(nTimeOut*1000);
	while(1)
	{
		if (PubTimerIsOver(unOverTime) == APP_SUCC)
		{
			SetCommError(APP_TIMEOUT, 0);
			return APP_TIMEOUT;
		}
		nKeyCode = PubKbHit();
		if (nKeyCode == KEY_ESC)
		{
			SetCommError(APP_QUIT, 0);
			return APP_QUIT;
		}

		nRet = NAPI_PortRead(nAux, (uchar *)psOutData + nRecvLen, &nMaxRlen, 1000);
		if (nRet == NAPI_ERR_TIMEOUT)
		{
			PORT_TRACE(PORT_DEBUG_LEVEL, "wait for data, timeout ...");
			continue;
		}
		if (nRet != NAPI_OK)
		{
			SetCommError(FAIL_RS232_READ, nRet);
			PORT_TRACE(PORT_DEBUG_LEVEL, "NAPI_PortRead error[%d]---[Port:%d].",nRet, nAux);
			return APP_FAIL;
		}
		nTmpLen = nTmpLen - nMaxRlen;
		nRecvLen += nMaxRlen;
		if (nIsNonBlock || nMaxRlen == 0 || nTmpLen == 0)
		{
			break;
		}
		nMaxRlen = nTmpLen > PORT_MAX_COMMLEN ? PORT_MAX_COMMLEN : nTmpLen;
	}
	PubDebugData("PortRead succ: ", psOutData, nRecvLen);

	return nRecvLen;
}


/**
* @brief hangup (close port)
* @param [in] nFlag
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author Liug
* @date 2012-6-5
*/
int PortHangUp(int nFlag)
{
	if (HANGUP_PPP == nFlag)
	{
		NAPI_PortClose(nAux);
		PORT_TRACE(PORT_DEBUG_LEVEL, "PortClose succ");
	}
	PORT_TRACE(PORT_DEBUG_LEVEL, "PortHangUp succ...[flag: %d]", nFlag);

	return APP_SUCC;
}


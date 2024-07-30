/**
* @file dial.c
* @brief Asyn dial module
* @version  1.0
* @author Liug
* @date 2012-06-18
*/
#include <string.h>
#include "commerror.h"
#include "commtool.h"
#include "libapiinc.h"
#include "process.h"
#include "napi_modem.h"

static char	szPredialNo[10];	    /**<predial number*/
static char	lszTelNo[5][21];	    /**<5 phone number*/
static int	nCycTimes; 			    /**<Redial times*/
static char szAtDialNum[50];	    /**<At dial number*/
static int  nLastDialSuccIndex=-1;  /**<connected index*/

static int ProGetAsynModemStatus(uint unOverTime);
static int ProGetAsynTelNo(char* pszTelNo, int nIndex);
static int ProAsynDial(char *pszNum, int nIsPreConnect);
extern int SynDialHangUp(int nFlag);

/**
* @brief Asyn dial initialize
* @param [in] pstDialParam
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author Hec
* @date 2012-7-18
*/
int AsynDialInit(STDIALPARAM *pstDialParam)
{
	int nRet;

	PubDebugSelectly(3, "AsynDialInit start---[nCountryid:%d]", pstDialParam->nCountryid);
	if (pstDialParam == NULL)
	{
		SetCommError(FAIL_ASYN_DIAL_INVAILIDPARAM, 0);
		PubDebugSelectly(3, "AsynDialInit invalid param");
		return APP_FAIL;
	}
	PubDebugSelectly(1, "NAPI_MdmAsynInit [nCountryid:%d]...", pstDialParam->nCountryid);

	nRet = NAPI_MdmAsynInit((EM_MDM_PatchType)pstDialParam->nCountryid);
	if (nRet != NAPI_OK)
	{
		SetCommError(FAIL_ASYN_DIAL_INIT, nRet);
		PubDebugSelectly(3, "NAPI_MdmAsynInit error[%d]---[nCountryid:%d]",nRet,pstDialParam->nCountryid);
		return APP_FAIL;
	}
	memcpy(szPredialNo,pstDialParam->szPredialNo,sizeof(szPredialNo));
	memcpy(lszTelNo,pstDialParam->lszTelNo,sizeof(lszTelNo));
	nCycTimes = pstDialParam->nCycTimes;
	if (nCycTimes < 1)
	{
		nCycTimes = 1;
	}
	if (nCycTimes > 6)
	{
		nCycTimes = 6;
	}
	PubDebugSelectly(3, "AsynDialInit succ");
	return APP_SUCC;
}

/**
* @brief Asyn connection
* @param [in] pstServerAddress
* @param [out] pnConnectIndex
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author Hec
* @date 2012-7-18
*/
int AsynDialConnect(const STSERVERADDRESS * pstServerAddress,int *pnConnectIndex, int nTimeOut, int nIsPreConnect)
{
	int nRet, nDialRet;
	int i;
	char szTelNo[32];
    char szLastDialNo[32];
	uint unOverTime = 0, nAdjustTimeOut = nTimeOut-2;//150710 cliect

    COMM_UNUSED(pstServerAddress);
	if (EM_PRECONNECT == nIsPreConnect) //
	{
		if (nLastDialSuccIndex != -1) //
		{
			nDialRet = ProAsynDial(szAtDialNum, nIsPreConnect);
		}
		else //
		{
			if (ProGetAsynTelNo(szTelNo, 0) != APP_SUCC)
			{
				return APP_FAIL;
			}
			nDialRet = ProAsynDial(szTelNo, nIsPreConnect);
		}
		if (nDialRet != APP_SUCC)
			return APP_FAIL;
		PubDebugSelectly(3, "PreConnect succ...");
		return nDialRet;
	}

	PubDebugSelectly(3, "AsynDialConnect start...[index:%d][nCycTimes:%d]", nLastDialSuccIndex, nCycTimes);

	if (nLastDialSuccIndex != -1)//
	{
		unOverTime = PubGetOverTimer(nAdjustTimeOut*1000);
		nDialRet = ProAsynDial(szAtDialNum, nIsPreConnect);
		if (nDialRet == APP_SUCC)
		{
			if ((nRet = ProGetAsynModemStatus(unOverTime)) != APP_FAIL)
			{
				return nRet;//
			}
		}
		else if (nDialRet == APP_FUNCQUIT)//
		{
			return APP_FAIL;
		}
	}

    memset(szLastDialNo, 0, sizeof(szLastDialNo));

	for(i = 0; i < nCycTimes; i++)
	{
		memset(szTelNo, 0, sizeof(szTelNo));
		if((nLastDialSuccIndex != -1) && (nLastDialSuccIndex == (i%3)))
		{
			continue;
		}
		if(ProGetAsynTelNo(szTelNo, i%3) != APP_SUCC)
		{
			return APP_FAIL;
		}
		memset(szAtDialNum, 0, sizeof(szAtDialNum));
		sprintf(szAtDialNum, "%s", szTelNo);//ATDT
		/**< by liugang 20140607*/
		if(strcmp(szAtDialNum, szLastDialNo) != 0)
		{
			strcpy(szLastDialNo, szAtDialNum);
			unOverTime = PubGetOverTimer(nAdjustTimeOut*1000);
			nDialRet = ProAsynDial(szAtDialNum, nIsPreConnect);
			if (nDialRet == APP_SUCC)
			{
				if ((nRet = ProGetAsynModemStatus(unOverTime)) != APP_FAIL)
				{
					if (nRet == APP_SUCC)
					{
						nLastDialSuccIndex = i % 3;
						*pnConnectIndex = nLastDialSuccIndex;
					}
					return nRet;//
				}
			}
			else if (nDialRet == APP_FUNCQUIT)//
			{
				return APP_FAIL;
			}
		}
		continue;
	}
	return APP_FAIL;
}

/**
* @brief Get asyn connection status
* @param
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author Liug
* @date 2012-11-15
*/
int AsynDialGetConnectState()
{
	PubDebugSelectly(2, "AsynDialGetConnectState start---[nLastDialSuccIndex: %d]", nLastDialSuccIndex);
	if (nLastDialSuccIndex == -1)
	{
		PubDebugSelectly(2, "AsynDialGetConnectState fail");//20140107
		return APP_FAIL;
	}
	PubDebugSelectly(2, "AsynDialGetConnectState succ");
	return APP_SUCC;
}

/**
* @brief Sending data
* @param [in]  psData
* @param [in]  nDataLen
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author Hec
* @date 2012-7-18
*/
int AsynDialWrite(const char *psData,int nDataLen)
{
	int nRet;

	PubDebugData("AsynDialWrite start: ", psData, nDataLen);
	PubDebugSelectly(1, "NAPI_MdmWrite [DataLen:%d]...", nDataLen);
	nRet = NAPI_MdmWrite(psData,nDataLen);
	if(nRet != NAPI_OK)
	{
		SetCommError(FAIL_ASYN_DIAL_WRITE, nRet);
		PubDebugSelectly(3, "NAPI_MdmWrite error[%d]",nRet);
		nLastDialSuccIndex++;
		nLastDialSuccIndex = nLastDialSuccIndex%3;
		if (ProGetAsynTelNo(szAtDialNum, nLastDialSuccIndex) != APP_SUCC)
		{
			nLastDialSuccIndex = -1;
		}
		return APP_FAIL;
	}
	PubDebugSelectly(3, "AsynDialWrite succ");
	return APP_SUCC;
}

/**
* @brief Receiving data
* @param [in]   nIsNonBlock
* @param [in]   nLen
* @param [in]   nTimeOut
* @param [out]  psOutData
* @return
* @li >0
* @li Fail
* @li TIMEOUT
* @author Hec
* @date 2012-7-18
*/
int AsynDialRead(int nIsNonBlock, int nLen,int nTimeOut,char *psOutData)
{
	int nRet;
    int nReadLen = 0;
	uint unOverTime = 0;

	PubDebugSelectly(2, "AsynDialRead start---[len:%d][nTimeOut:%d]", nLen, nTimeOut);


    if (nIsNonBlock)
    {
		NAPI_MdmGetreadlen((uint *)&nReadLen);
	    if (0 == nReadLen)
	    {
	        PubDebug("AsynDialRead ReadLen is 0.");
		    return 0;
	    }
       	nLen = nReadLen > nLen ? nLen : nReadLen;
		nTimeOut = 0;
    }
	else
	{
		unOverTime = PubGetOverTimer(nTimeOut*1000);
		while(1)
		{
			if (PubTimerIsOver(unOverTime) == APP_SUCC)
			{
				SetCommError(APP_TIMEOUT, 0);
				return APP_TIMEOUT;
			}
			if (PubKbHit() == KEY_ESC)
			{
				SetCommError(APP_QUIT, 0);
				return APP_QUIT;
			}
			NAPI_MdmGetreadlen((uint *)&nReadLen);
			if (nReadLen > 0)
			{
				break;
			}
		}
	}
	PubDebugSelectly(2, "NAPI_MdmReadLen is [%d]",nReadLen);
	nRet = NAPI_MdmRead(psOutData,(uint *)&nLen, nTimeOut);
	if(nRet != NAPI_OK)
	{
		SetCommError(FAIL_ASYN_DIAL_READ, nRet);
		if (NAPI_ERR_TIMEOUT == nRet)
		{
			return APP_TIMEOUT;
		}
		PubDebugSelectly(3, "NAPI_MdmRead data error[%d]",nRet);
		return APP_FAIL;
	}
	SetLedComFlick();
	PubDebugData("AsynDialRead succ: ", psOutData, nLen);
	return nLen;
}

/**
* @brief Hangup
* @param [in] nFlag
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author Hec
* @date 2012-7-18
*/
int AsynDialHangUp(int nFlag)
{
	int nRet = 0;

    COMM_UNUSED(nFlag);
	PubDebugSelectly(2, "AsynDialHangUp start");
	nRet = NAPI_MdmHangup();
	if (nRet != NAPI_OK)
	{
		SetCommError(FAIL_ASYN_DIAL_HANGUP, nRet);
		PubDebugSelectly(3, "NAPI_MdmHangup error[%d]",nRet);
		return APP_FAIL;
	}
	PubDebugSelectly(3, "AsynDialHangUp succ");
	CommserverLedShine(NAPI_LED_COM_OFF);
	CommserverLedShine(NAPI_LED_ONL_OFF);
	return APP_SUCC;
}

/**
* @brief Clear buffer
* @param
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author Hec
* @date 2012-7-18
*/
int AsynDialClearBuf()
{
	int nRet = 0;

	PubDebugSelectly(2, "AsynDialClearBuf start");
	nRet = NAPI_MdmClrbuf();
	if (nRet != NAPI_OK)
	{
		SetCommError(FAIL_ASYN_DIAL_CLEARBUF, nRet);
		PubDebugSelectly(3, "NAPI_MdmClrbuf error[%d]",nRet);
		return APP_FAIL;
	}
	PubDebugSelectly(3, "AsynDialClearBuf succ");
	return APP_SUCC;
}
/**
* @brief Get modem status
* @param
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author Liug
* @date 2012-6-8
*/
static int ProGetAsynModemStatus(uint unOverTime)
{
    EM_MDMSTATUS nModemStatus = (EM_MDMSTATUS)(0xff);
	int nRet = 0;

	PubDebugSelectly(2, "ProGetAsynModemStatus start");
	while(1)
	{
		if (PubTimerIsOver(unOverTime) == APP_SUCC)
		{
			PubDebugSelectly(3, "Asyn Connect Timeout");
			SetCommError(APP_TIMEOUT, 0);
			nLastDialSuccIndex = -1;
			AsynDialHangUp(HANGUP_PPP);
			return APP_TIMEOUT;
		}
		PubDebugSelectly(1, "NAPI_MdmCheck...");
		nRet = NAPI_MdmCheck(&nModemStatus);//modem
		PubDebugSelectly(3, "NAPI_MdmCheck [Ret:%d][ModemStatus:%d]", nRet, nModemStatus);
		if (nRet != NAPI_OK)
		{
			SetCommError(FAIL_ASYN_DIAL_CHECK, nRet);
			PubDebugSelectly(3, "NAPI_MdmCheck error[%d]---[nModemStatus:%d]", nRet, nModemStatus);
			AsynDialHangUp(HANGUP_PPP);
			return APP_FAIL;
		}
		if (PubKbHit() == KEY_ESC)
		{
			SetCommError(APP_QUIT, 0);
			AsynDialHangUp(HANGUP_PPP);
			return APP_QUIT;
		}
		switch (nModemStatus)
		{
		case MDMSTATUS_NORETURN_AFTERPREDIAL:// 0
		case MDMSTATUS_OK_AFTERPREDIAL:// 1
			PubSysMsDelay(500);
			continue;
			break;
		case MDMSTATUS_CONNECT_AFTERPREDIAL :// 2 Modem
			PubDebugSelectly(2, "ProGetAsynModemStatus succ");
			PubDebugSelectly(3, "AsynDialConnect succ");
			CommserverLedShine(NAPI_LED_ONL_ON);
			return APP_SUCC;
			break;
		case MDMSTATUS_MS_NODIALTONE :
			SetCommError(FAIL_ASYN_DIAL_NODIALTONE, 0);
			PubDebugSelectly(3, "Asyn dial no dial tone");
			break;
		case MDMSTATUS_MS_NOCARRIER:
			SetCommError(FAIL_ASYN_DIAL_NOCARRIER, 0);
			PubDebugSelectly(3, "Asyn dial no carrier");
			break;
		case MDMSTATUS_MS_BUSY:
			SetCommError(FAIL_ASYN_DIAL_BUSY, 0);
			PubDebugSelectly(3, "Asyn dial line busy");
			break;
		case MDMSTATUS_MS_ERROR :
			SetCommError(FAIL_ASYN_DIAL_ERROR, 0);
			PubDebugSelectly(3, "Asyn dial command error");
			break;
		case MDMSTATUS_NOPREDIAL  :
			SetCommError(FAIL_ASYN_DIAL_NOPREDIAL, 0);
			PubDebugSelectly(3, "Asyn dial no predial");
			break;
		default:
			if (nModemStatus > 2)//
			{
				PubDebugSelectly(3, "AsynDialConnect succ");
				CommserverLedShine(NAPI_LED_ONL_ON);
				return APP_SUCC;
			}
			else
			{
				SetCommError(FAIL_ASYN_DIAL_UNKNOW, 0);
				PubDebugSelectly(3, "Asyn dial unknow error");
			}
			break;
		}
		AsynDialHangUp(HANGUP_PPP);
		nLastDialSuccIndex = -1;
		return APP_FAIL;
	}
}

/**
* @brief Get phone number
* @param [in] nIndex
* @param [out] pszTelNo
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author Liug
* @date 2012-6-18
*/
static int ProGetAsynTelNo(char* pszTelNo, int nIndex)
{
	char szTelNo[21];
	int i = 0;
	int nTmpIndex = nIndex;

	memset(szTelNo, 0, sizeof(szTelNo));

	if ((nIndex < 0)||(nIndex >2))
	{
		SetCommError(FAIL_ASYN_DIAL_TELNO, 0);
		return APP_FAIL;
	}

	for (i  = 0; i < 3; i++)
	{
		if (strlen(lszTelNo[nTmpIndex%3]) == 0)
		{
			nTmpIndex++;
		}
		else
		{
			memcpy(szTelNo, lszTelNo[nTmpIndex%3], 20);
			break;
		}
	}
	if (i == 3)
	{
		SetCommError(FAIL_ASYN_DIAL_TELNO, 0);
		return APP_FAIL;
	}
	if (strlen(szPredialNo) > 0)
	{
		sprintf(pszTelNo, "%s%s",szPredialNo, szTelNo);
	}
	else
	{
		sprintf(pszTelNo,"%s",szTelNo);
	}
	return APP_SUCC;
}

static int ProAsynDial(char *pszNum, int nIsPreConnect)
{
	int nRet = -1;
	EM_MDMSTATUS emModemStatus;

	if (nIsPreConnect == EM_CONNECT)//
	{
		PubDebugSelectly(1, "NAPI_MdmCheck...");
		nRet = NAPI_MdmCheck(&emModemStatus);//modem
		PubDebugSelectly(1, "NAPI_MdmCheck [nRet:%d][nModemStatus:%d]",nRet,emModemStatus);
		if (nRet == 0 && emModemStatus >= 0)//
		{
			return APP_SUCC;
		}
	}

	CommserverLedShine(NAPI_LED_ONL_FLICK);//LED

	PubDebugSelectly(3, "Asyn Dial... [index:%d][num:%s]",nLastDialSuccIndex, pszNum);
	nRet = NAPI_MdmDial(pszNum);
	if(nRet != NAPI_OK)
	{
		SynDialHangUp(HANGUP_PPP);//sh 20150722
		switch (nRet)
		{
		case NAPI_ERR_MODEM_NOLINE:
			SetCommError(FAIL_ASYN_DIAL_LINE, nRet);
			PubDebugSelectly(3, "NAPI_MdmDial error [%d]---[dialNum:%s]", nRet,pszNum);
			nLastDialSuccIndex = -1;
			return APP_FUNCQUIT;
			break;
		case NAPI_ERR_MODEM_OTHERMACHINE:
			SetCommError(FAIL_ASYN_DIAL_OTHERMACHINE, nRet);
			PubDebugSelectly(3, "NAPI_MdmDial error [%d]---[dialNum:%s]", nRet,pszNum);
			nLastDialSuccIndex = -1;
			return APP_FUNCQUIT;
			break;
		default:
			SetCommError(FAIL_ASYN_DIAL_PREDIAL, nRet);
			PubDebugSelectly(3, "NAPI_MdmDial error [%d]---[dialNum:%s]", nRet,pszNum);
			nLastDialSuccIndex = -1;
			//AsynDialHangUp(HANGUP_PPP);
			return APP_FAIL;
			break;
		}
	}
	return APP_SUCC;
}


/**
* @brief Set parameter
* @param [in] pstDialParam
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author Liug
* @date 2012-6-18
*/
int AsynDialSetParam(const STDIALPARAM *pstDialParam)
{
	PubDebugSelectly(2, "AsynDialSetParam start...");
	if (pstDialParam == NULL)
	{
		SetCommError(FAIL_ASYN_DIAL_INVAILIDPARAM, 0);
		PubDebugSelectly(3, "AsynDialSetParam invalid param");
		return APP_FAIL;
	}

	memcpy(szPredialNo,pstDialParam->szPredialNo,sizeof(szPredialNo));
	memcpy(lszTelNo,pstDialParam->lszTelNo,sizeof(lszTelNo));
	nCycTimes = pstDialParam->nCycTimes;
	PubDebugSelectly(3, "AsynDialSetParam succ");//20140107

	return APP_SUCC;
}

/**
* @brief Reset the connected index
* @param
* @return
* @author sh
* @date 2014-2-17
*/
void AsynResetIndex()
{
	nLastDialSuccIndex = -1;
}


/**
* @file syn.c
* @brief Syn dial module
* @version  1.0
* @author Liug
* @date 2012-06-18
*/
#include <string.h>
#include "commerror.h"
#include "commtool.h"
#include "libapiinc.h"
#include "process.h"
#include "syndial.h"
#include "napi_modem.h"
static char	szPredialNo[10];	    /**<Pridial number*/
static char	lszTelNo[3][21];	    /**<3 phone number*/
static int	nCycTimes; 			    /**<Redial times*/
static char szAtDialNum[50];	    /**<At dial number*/
static int  nLastDialSuccIndex=-1;  /**<connected index*/

static int ProGetSynModemStatus(uint unOverTime);
static int ProGetSynTelNo(char* pszTelNo, int nIndex);
static int ProSynDial(char *pszNum, int nIsPreConnect);

/**
* @brief Syn dial initialize
* @param [in] pstDialParam
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author Liug
* @date 2012-6-18
*/
int SynDialInit(STDIALPARAM *pstDialParam)
{
	int nRet;

	PubDebugSelectly(3, "SynDialInit start---[countryId:%d][nBps:%d]", pstDialParam->nCountryid, pstDialParam->nBps);
	if (pstDialParam == NULL)
	{
		SetCommError(FAIL_SYN_DIAL_INVAILIDPARAM, 0);
		PubDebugSelectly(3, "SynDialInit invalid param");
		return APP_FAIL;
	}

	if(pstDialParam->nBps >= 1)
	{
		nRet = NAPI_MdmSdlcSingleForceInit((EM_MDM_PatchType)pstDialParam->nCountryid, (EM_MDM_BPS)pstDialParam->nBps);
	}
	else
	{
		nRet = NAPI_MdmSdlcInit((EM_MDM_PatchType)pstDialParam->nCountryid);
	}

	if (nRet != NAPI_OK)
	{
		SetCommError(FAIL_SYN_DIAL_INIT, nRet);
		PubDebugSelectly(3, "NAPI_MdmSdlcInit error[%d]---[nCountryid:%d][nBps:%d]", nRet, pstDialParam->nCountryid, pstDialParam->nBps);
		return APP_FAIL ;
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
	PubDebugSelectly(3, "SynDialInit succ");
	return APP_SUCC;
}

/**
* @brief Syn dial connection
* @param [in] pstServerAddress
* @param [out] pnConnectIndex
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author Liug
* @date 2012-6-7
*/
int SynDialConnect(const STSERVERADDRESS * pstServerAddress,int *pnConnectIndex,int nTimeOut, int nIsPreConnect)
{
	int nRet;
	int nDialRet;
	int i;
	char szTelNo[32];
	uint unOverTime = 0, nAdjustTimeOut = nTimeOut-2;

    COMM_UNUSED(pstServerAddress);
	if (EM_PRECONNECT == nIsPreConnect)
	{
		if (nLastDialSuccIndex != -1) //
		{
			nDialRet = ProSynDial(szAtDialNum, nIsPreConnect);
		}
		else //
		{
			if (APP_SUCC != ProGetSynTelNo(szTelNo, 0))
			{
				return APP_FAIL;
			}
			nDialRet =  ProSynDial(szTelNo, nIsPreConnect);
		}
		if (nDialRet != APP_SUCC)
			return APP_FAIL;
		PubDebugSelectly(3, "Preconnect succ...");
		return nDialRet;
	}

	PubDebugSelectly(3, "SynDialConnect start...[index:%d][nCycTimes:%d]", nLastDialSuccIndex, nCycTimes);

	if (nLastDialSuccIndex != -1)//
	{
		unOverTime = PubGetOverTimer(nAdjustTimeOut*1000);
		nDialRet = ProSynDial(szAtDialNum, nIsPreConnect);
		if (nDialRet == APP_SUCC)
		{
			if ((nRet = ProGetSynModemStatus(unOverTime)) != APP_FAIL)
			{
				return nRet;//succ/timeout/cancel to finish.
			}
		}
		else if (nDialRet == APP_FUNCQUIT)
		{
			return APP_FAIL;
		}
	}

	for(i = 0; i< nCycTimes; i++)
	{
		if ((nLastDialSuccIndex != -1)&&(nLastDialSuccIndex == (i%3)))
		{
			continue;
		}
		memset(szTelNo, 0, sizeof(szTelNo));
		if(ProGetSynTelNo(szTelNo, i%3) != APP_SUCC)
		{
			//continue;
			return APP_FAIL;
		}
		memset(szAtDialNum,0,sizeof(szAtDialNum));
		strcpy(szAtDialNum,szTelNo);
		unOverTime = PubGetOverTimer(nAdjustTimeOut*1000);
		nDialRet = ProSynDial(szAtDialNum, nIsPreConnect);
		if (nDialRet == APP_SUCC)
		{
			if ((nRet = ProGetSynModemStatus(unOverTime)) != APP_FAIL)
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
		continue;
	}
	return APP_FAIL;
}

/**
* @brief get Syn dial connection status
* @param
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author Liug
* @date 2012-11-15
*/
int SynDialGetConnectState()
{
	int nRet;
//	int nModemStatus;
	EM_MDMSTATUS emModemStatus;
	PubDebugSelectly(2, "SynDialGetConnectState start....");

	nRet = NAPI_MdmCheck(&emModemStatus);//modem
	PubDebugSelectly(2, "[nRet:%d][nModemStatus:%d]", nRet, emModemStatus);
	if (nRet != NAPI_OK)
	{
		SetCommError(FAIL_SYN_DIAL_CHECK, nRet);
		PubDebugSelectly(3, "NAPI_MdmCheck error---[nModemStatus:%d]", emModemStatus);
		return APP_FAIL;
	}
	if (emModemStatus != MDMSTATUS_CONNECT_AFTERPREDIAL)
	{
		PubDebugSelectly(2, "SynDialGetConnectState Fail...");
		return APP_FAIL;
	}
	PubDebugSelectly(2, "SynDialGetConnectState succ");
	return APP_SUCC;
}

/**
* @brief Sending data
* @param [in]  psData
* @param [in]  nDataLen
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author Liug
* @date 2012-6-18
*/
int SynDialWrite(const char *psData,int nDataLen)
{
	int nRet;
	PubDebugData("SynDialWrite start: ", psData, nDataLen);
	nRet = NAPI_MdmWrite(psData,nDataLen);
	if(nRet != NAPI_OK)
	{
		SetCommError(FAIL_SYN_DIAL_WRITE, nRet);
		PubDebugSelectly(3, "NAPI_MdmWrite error[%d]",nRet);

		nLastDialSuccIndex++;
		nLastDialSuccIndex = nLastDialSuccIndex%3;
		if (ProGetSynTelNo(szAtDialNum, nLastDialSuccIndex) != APP_SUCC)
		{
			nLastDialSuccIndex = -1;
		}
		return APP_FAIL;
	}
	PubDebugSelectly(3, "SynDialWrite succ");
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
* @author Liug
* @date 2012-6-18
*/
int SynDialRead(int nIsNonBlock, int nLen,int nTimeOut,char *psOutData)
{
	int nRet;
	EM_MDMSTATUS emMdmStatus;
    int nReadBufLen = 0;
	uint unOverTime = 0;

	PubDebugSelectly(2, "SynDialRead start.---[nLen:%d][nTimeOut:%d]", nLen, nTimeOut);

	if(NAPI_MdmCheck(&emMdmStatus) == NAPI_OK && emMdmStatus < 0)
	{
		PubDebug("SynDialRead Dial Break");
		return APP_FAIL;
	}

    if (nIsNonBlock)
    {
		NAPI_MdmGetreadlen((uint *)&nReadBufLen);
	    if (0 == nReadBufLen)
	    {
	        PubDebug("SynDialRead ReadLen is 0.");
		    return 0;
	    }
	    nLen = nReadBufLen > nLen ? nLen : nReadBufLen;
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
			NAPI_MdmGetreadlen((uint *)&nReadBufLen);
		    if (nReadBufLen > 0)
		    {
		        break;
		    }
		}
	}
	nRet = NAPI_MdmRead(psOutData, (uint *)&nLen, nTimeOut);
	if(nRet != NAPI_OK)
	{
		SetCommError(FAIL_SYN_DIAL_READ, nRet);
		if (NAPI_ERR_TIMEOUT == nRet)
		{
			return APP_TIMEOUT;
		}
		PubDebugSelectly(3, "NAPI_MdmRead error[%d]",nRet);
		return APP_FAIL;
	}
	SetLedComFlick();
	PubDebugData("SynDialRead succ: ", psOutData, nLen);
	return nLen;
}
/**
* @brief Hangup
* @param [in] nFlag
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author Liug
* @date 2012-6-18
*/
int SynDialHangUp(int nFlag)
{
	int nRet = 0;

	PubDebugSelectly(2, "SynDialHangUp start--[LastDialSuccIndex: %d]", nLastDialSuccIndex);
	nRet = NAPI_MdmHangup();
	if (nRet != NAPI_OK)
	{
		SetCommError(FAIL_SYN_DIAL_HANGUP, nRet);
		PubDebugSelectly(3, "NAPI_MdmHangup error[%d]",nRet);
		return APP_FAIL;
	}
	PubDebugSelectly(3, "SynDialHangUp succ");
	CommserverLedShine(NAPI_LED_COM_OFF);
	CommserverLedShine(NAPI_LED_ONL_OFF);
	return APP_SUCC;
}


/**
* @brief Clear buffer
* @param [in]
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author Liug
* @date 2012-6-18
*/
int SynDialClearBuf()
{
	PubDebugSelectly(2, "SynDialClearBuf succ");
	return APP_SUCC;
}
/**
* @brief Get phone number from index
* @param [in] nIndex
* @param [out] pszTelNo
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author Liug
* @date 2012-6-18
*/
static int ProGetSynTelNo(char* pszTelNo, int nIndex)
{
	char szTelNo[21];
	int i = 0;
	int nTmpIndex = nIndex;

	memset(szTelNo, 0, sizeof(szTelNo));

	if ((nIndex < 0)||(nIndex >2))
	{
		SetCommError(FAIL_SYN_DIAL_TELNO, 0);
		return APP_FAIL;
	}

	//memcpy(szTelNo, lszTelNo[nIndex], 20);

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
		SetCommError(FAIL_SYN_DIAL_TELNO, 0);
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
/**
* @brief Get modem status
* @param
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author Liug
* @date 2012-6-8
*/
static int ProGetSynModemStatus(uint unOverTime)
{
    EM_MDMSTATUS nModemStatus = (EM_MDMSTATUS)(0xff);
	int nRet = 0;

	while(1)
	{
		if (PubTimerIsOver(unOverTime) == APP_SUCC)
		{
			PubDebugSelectly(3, "Syn Connect Timeout");
			SetCommError(APP_TIMEOUT, 0);
			SynDialHangUp(HANGUP_PPP);
			nLastDialSuccIndex = -1;
			return APP_TIMEOUT;
		}
		if (PubKbHit() == KEY_ESC)
		{
			SetCommError(APP_QUIT, 0);
			SynDialHangUp(HANGUP_PPP);
			return APP_QUIT;
		}
		nRet = NAPI_MdmCheck(&nModemStatus);//modem
		PubDebugSelectly(3, "[nRet:%d][nModemStatus:%d]", nRet, nModemStatus);
		if (nRet != NAPI_OK)
		{
			PubDebugSelectly(3, "NAPI_MdmCheck error---[nModemStatus:%d]", nModemStatus);
			SetCommError(FAIL_SYN_DIAL_CHECK, nRet);
			SynDialHangUp(HANGUP_PPP);
			nLastDialSuccIndex = -1;
			return APP_FAIL;
		}
		switch (nModemStatus)
		{
		case MDMSTATUS_NORETURN_AFTERPREDIAL:// 0
		case MDMSTATUS_OK_AFTERPREDIAL:// 1
			PubSysMsDelay(100);
			continue;
			break;
		case MDMSTATUS_CONNECT_AFTERPREDIAL :// 2 Modem
			PubDebugSelectly(2, "ProGetSynModemStatus succ");
			PubDebugSelectly(3, "SynDialConnect succ");
			CommserverLedShine(NAPI_LED_ONL_ON);
			return APP_SUCC;
			break;
		case MDMSTATUS_MS_NODIALTONE :
			SetCommError(FAIL_SYN_DIAL_NODIALTONE, 0);
			PubDebugSelectly(3, "Syn dial no dial tone");
			break;
		case MDMSTATUS_MS_NOCARRIER:
			SetCommError(FAIL_SYN_DIAL_NOCARRIER, 0);
			PubDebugSelectly(3, "Syn dial no carrier");
			break;
		case MDMSTATUS_MS_BUSY:
			SetCommError(FAIL_SYN_DIAL_BUSY, 0);
			PubDebugSelectly(3, "Syn dial line busy");
			break;
		case MDMSTATUS_MS_ERROR :
			SetCommError(FAIL_SYN_DIAL_ERROR, 0);
			PubDebugSelectly(3, "Syn dial command error");
			break;
		case MDMSTATUS_NOPREDIAL  :
			SetCommError(FAIL_SYN_DIAL_NOPREDIAL, 0);
			PubDebugSelectly(3, "Syn dial no predial");
			break;
		default:
			SetCommError(FAIL_SYN_DIAL_UNKNOW, 0);
			PubDebugSelectly(3, "Syn dial unknow error");
			break;
		}
		SynDialHangUp(HANGUP_PPP);
		nLastDialSuccIndex = -1;
		return APP_FAIL;
	}
}

static int ProSynDial(char *pszNum, int nIsPreConnect)
{
	int nRet = -1;
	EM_MDMSTATUS emModemStatus;

	if (nIsPreConnect == EM_CONNECT)//
	{
		nRet = NAPI_MdmCheck((EM_MDMSTATUS *)&emModemStatus);//modem
		if (nRet == 0 && emModemStatus >= 0)//
		{
			return APP_SUCC;
		}
	}
	CommserverLedShine(NAPI_LED_ONL_FLICK);//LED
	PubDebugSelectly(3, "SynDial... [index:%d][num:%s]",nLastDialSuccIndex, pszNum);
	nRet = NAPI_MdmDial(pszNum);
	if(nRet != NAPI_OK)
	{
		SynDialHangUp(HANGUP_PPP);//sh 20150722
		switch (nRet)
		{
		case NAPI_ERR_MODEM_NOLINE:
			SetCommError(FAIL_SYN_DIAL_LINE, nRet);
			PubDebugSelectly(3, "NAPI_MdmDial error [%d]---[dialNum:%s]", nRet,pszNum);
			nLastDialSuccIndex = -1;
			return APP_FUNCQUIT;
			break;
		case NAPI_ERR_MODEM_OTHERMACHINE:
			SetCommError(FAIL_SYN_DIAL_OTHERMACHINE, nRet);
			PubDebugSelectly(3, "NAPI_MdmDial error [%d]---[dialNum:%s]", nRet,pszNum);
			nLastDialSuccIndex = -1;
			return APP_FUNCQUIT;
			break;
		default:
			SetCommError(FAIL_SYN_DIAL_PREDIAL, nRet);
			PubDebugSelectly(3, "NAPI_MdmDial error [%d]---[dialNum:%s]", nRet,pszNum);
			//SynDialHangUp(HANGUP_PPP);
			nLastDialSuccIndex = -1;
			return APP_FAIL;
			break;
		}
	}
	return APP_SUCC;
}

/**
* @brief Set syn dial parameter
* @param [in] pstDialParam
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author Liug
* @date 2012-6-18
*/
int SynDialSetParam(STDIALPARAM *pstDialParam)
{
	PubDebugSelectly(2, "SynDialSetParam start...[reDialNum: %d]", pstDialParam->nCycTimes);
	if (pstDialParam == NULL)
	{
		SetCommError(FAIL_SYN_DIAL_INVAILIDPARAM, 0);
		PubDebugSelectly(3, "SynDialSetParam invalid param");
		return APP_FAIL;
	}

	memcpy(szPredialNo,pstDialParam->szPredialNo,sizeof(szPredialNo));
	memcpy(lszTelNo,pstDialParam->lszTelNo,sizeof(lszTelNo));
	nCycTimes = pstDialParam->nCycTimes;
	PubDebugSelectly(3, "SynDialSetParam succ");

	return APP_SUCC;
}

/**
* @brief Reset dial connected index
* @param
* @return
* @author sh
* @date 2014-2-17
*/
void SynResetIndex()
{
	nLastDialSuccIndex = -1;
}


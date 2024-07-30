/**
* @file nwireless.c
* @brief new wireless module use forth interface
* @version  1.0
* @author chenxiulin
* @date 2019-07-24
*/
#include "wireless.h"
#include "commtool.h"
#include "ltool.h"
#include "process.h"
#include "commerror.h"
#include "lui.h"
#include "napi_wlm.h"
#include "napi_net.h"

#define WIRELESS_DEFAULT_TIMEOUT 	(30)
#define WIRELESS_LPPP_MIN_TIMEOUT 	(5)
#define WIRELESS_SPPP_MIN_TIMEOUT 	(0)
#define WIRELESS_DEBUG_LEVEL 		(3)
#define WIRELESS_TRACE(level, fmt, args...) \
    PubDebugSelectly(level, "[%s][%s][%d]>>>" fmt, __FILE__, __FUNCTION__, __LINE__, ##args)

static int gnHandle;					/**<Socket handle*/
static int gnLastConnectIndex = -1; 	/**<the last socket handle index*/
static char gcIsPppConnected = FALSE;	/**<PPP status**/
static char gcSslFlag;					/**<use ssl or not */
static char cIsRegister = FALSE;

int WirelessClearBuf(void)
{
	WIRELESS_TRACE(WIRELESS_DEBUG_LEVEL, "New WirelessClearBuf succ");

	return APP_SUCC;
}

int WirelessGetConnectState(void)
{
	return TcpGetStatus(gnHandle, gnLastConnectIndex);
}

static int WireLessDataStatusCallback(EM_DATA_ENABLE status)
{
    //COMM_UNUSED(status);
	return APP_SUCC;
}

static int WireLessWaitTimeOut(int nTimeOut, int (*ConditionFunc)())
{
	int nFlag = FALSE;
	uint unOverTime;

	if (nTimeOut <= 0)
	{
		SetCommError(APP_TIMEOUT, 0);
		return APP_TIMEOUT;
	}

	unOverTime = PubGetOverTimer(nTimeOut*1000);
	while (1)
	{
		nFlag = (*ConditionFunc) == NULL ? FALSE : (*ConditionFunc)();
		if (nFlag == TRUE)
		{
			break;
		}
		else if (nFlag != FALSE)
		{
			WIRELESS_TRACE(WIRELESS_DEBUG_LEVEL, "NWireLessWaitTimeOut cFlag [%d] ", nFlag);
			SetCommError(nFlag, 0);
			return nFlag;
		}
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
	}

	return APP_SUCC;
}

static int WireLessIsConnect(void)
{
	EM_DATA_ENABLE emStatus;
	int nRet;

	nRet = NAPI_WlmGetDataEnabled(&emStatus);
	if (nRet != NAPI_OK)
	{
		SetCommError(FAIL_WIRELESS_GETENABLE, nRet);
		WIRELESS_TRACE(WIRELESS_DEBUG_LEVEL, "NAPI_WlmgetDataEnabled nRet [%d]", nRet);
		return nRet;
	}
	if (emStatus == WLM_STATE_SIM_LOCKED_OR_ABSENT || emStatus == WLM_STATE_SIM_NOT_READY)
	{
		WIRELESS_TRACE(WIRELESS_DEBUG_LEVEL, "NWireLessIsConnect emStatus [%d]", emStatus);
		return FAIL_WIRELESS_NO_SIM;
	}
	else if (emStatus == WLM_STATE_PPP_ON)
	{
		return TRUE;
	}

	return FALSE;

}

static int WireLessCheckConnected(int nTimeOut)
{
	int nRet;
	ulong ulLocalAddr;
	ulong ulHostAddr;

	nRet = WireLessWaitTimeOut(nTimeOut, WireLessIsConnect);
	if (nRet != APP_SUCC)
	{
		return nRet;
	}
	nRet = NAPI_WlmGetLinkProperties(&ulLocalAddr, &ulHostAddr);
	if (nRet != NAPI_OK)
	{
		SetCommError(FAIL_WIRELESS_PPP_GETADDR, nRet);
		WIRELESS_TRACE(WIRELESS_DEBUG_LEVEL, "NEW NAPI_WlmGetLinkProperties fail nRet = %d", nRet);
		return APP_FAIL;
	}
	WIRELESS_TRACE(WIRELESS_DEBUG_LEVEL, "New WL Local IP:  %ld.%ld.%ld.%ld  Host IP: %ld.%ld.%ld.%ld ",
		ulLocalAddr&0xFF, (ulLocalAddr>>8)&0xFF, (ulLocalAddr>>16)&0xFF, (ulLocalAddr>>24)&0xff,
		ulHostAddr&0xFF, (ulHostAddr>>8)&0xFF, (ulHostAddr>>16)&0xFF, (ulHostAddr>>24)&0xff);
	gcIsPppConnected = TRUE;

	return APP_SUCC;
}

static int WireLessDial(int nWait, int nTimeOut)
{
	int nRet;
	EM_DATA_ENABLE emStatus;

	emStatus = WLM_STATE_ON;
	nRet = NAPI_WlmSetDataEnabled(&emStatus);
	if (nRet != NAPI_OK && nRet != NAPI_ERR_PPP_OPEN)
	{
		SetCommError(FAIL_WIRELESS_PPP_CONFIG, nRet);
		WIRELESS_TRACE(WIRELESS_DEBUG_LEVEL, "NEW NAPI_WlmSetDataEnabled fail nRet = %d", nRet);
		return APP_FAIL;
	}

	if (nWait == TRUE)
	{
		nRet = WireLessCheckConnected(nTimeOut);
		if (nRet != APP_SUCC)
		{
			WIRELESS_TRACE(WIRELESS_DEBUG_LEVEL, "NEW NAPI_WlmsetDailCfg fail");
			return APP_FAIL;
		}
	}
	WIRELESS_TRACE(WIRELESS_DEBUG_LEVEL, "NEW WireLess Dial succ ret[%d]", nRet);

	return APP_SUCC;
}

int WirelessInit(STGPRSPARAM *pstGprsParam, STCDMAPARAM *pstCdmaParam, char cSsl)
{
	ST_PPP_CFG stPPPCfg;
	char szSimPin[31+1] = {0};
	int nRet;
	int nStatus;
	EM_SIM_Status emSimStatus;

	memset(&stPPPCfg, 0, sizeof(stPPPCfg));
	if (pstGprsParam == NULL && pstCdmaParam == NULL)
	{
		SetCommError(FAIL_WIRELESS_INVAILIDPARAM, 0);
		WIRELESS_TRACE(WIRELESS_DEBUG_LEVEL, "New WirelessInit invalid param");
		return APP_FAIL;
	}
	nRet = NAPI_WlmGetSIMStatus(&emSimStatus);
	if (nRet != NAPI_OK)
	{
		SetCommError(FAIL_WIRELESS_GET_SIM, 0);
		WIRELESS_TRACE(WIRELESS_DEBUG_LEVEL, "New NAPI_WlmgetSIMStatus Err");
		return APP_FAIL;
	}

	switch (emSimStatus)
	{
	case SIM_ABSENT:
	case SIM_NOT_READY:
		SetCommError(FAIL_WIRELESS_NO_SIM, nRet);
		return APP_FAIL;
	case SIM_PIN:
		if (pstGprsParam != NULL)
		{
			strcpy(szSimPin, pstGprsParam->szPinPassWord);
		}
		else if (pstCdmaParam == NULL)
		{
			strcpy(szSimPin, pstCdmaParam->szPinPassWord);
		}
		WIRELESS_TRACE(WIRELESS_DEBUG_LEVEL, "szSimPin = %s", szSimPin);
		nRet = NAPI_WlmAuthenticatedSimPin(szSimPin);
		if (nRet != NAPI_OK)
		{
			SetCommError(FAIL_WIRELESS_PIN, nRet);
			return APP_FAIL;
		}
		break;
	case SIM_PUK:
		SetCommError(FAIL_WIRELESS_LOCKED, nRet);
		return APP_FAIL;
	default:
		break;
	}

	nRet = WirelessConnectState(&nStatus);
	if (nRet != NAPI_OK)
	{
		WIRELESS_TRACE(WIRELESS_DEBUG_LEVEL, "New WirelessConnectState fail");
		return APP_FAIL;
	}
	WIRELESS_TRACE(WIRELESS_DEBUG_LEVEL, "WirelessConnectState nStatus = %d emSimStatus = %d", nStatus, emSimStatus);

	if (gcIsPppConnected == TRUE || nStatus == WLM_STATE_PPP_ON)
	{
		if (WirelessHangUp(HANGUP_PPP) != APP_SUCC)
		{
			WIRELESS_TRACE(WIRELESS_DEBUG_LEVEL, "WirelessHangUp hang up ppp fail");
			return APP_FAIL;
		}
	}

	if (cIsRegister == FALSE)
	{
		nRet = NAPI_WlmStatusRegister(WireLessDataStatusCallback);
		if (nRet != APP_SUCC)
		{
			WIRELESS_TRACE(WIRELESS_DEBUG_LEVEL, "NEW NAPI_WlmStatusRegister fail");
			SetCommError(FAIL_WIRELESS_REGISTER, nRet);
			return APP_FAIL;
		}
		cIsRegister = TRUE;
	}

	if (pstGprsParam != NULL)
	{
		strcpy(stPPPCfg.szPin, pstGprsParam->szPinPassWord);
		strcpy(stPPPCfg.szDialNum, pstGprsParam->szModemDialNo);
		strcpy(stPPPCfg.szApn, pstGprsParam->szGprsApn);
		strcpy(stPPPCfg.pdpType, pstGprsParam->szPdpType);
	}
	else if(pstCdmaParam != NULL)
	{
		strcpy(stPPPCfg.szPin, pstCdmaParam->szPinPassWord);
		strcpy(stPPPCfg.szDialNum, pstCdmaParam->szModemDialNo);
		strcpy(stPPPCfg.pdpType, pstCdmaParam->szPdpType);
	}
	if (strlen(stPPPCfg.pdpType) == 0)
	{
		strcpy(stPPPCfg.pdpType, "IP");
	}
	stPPPCfg.nPPPFlag = LCP_PPP_KEEP;
	stPPPCfg.PPPIntervalTimeOut = KEEP_PPP_TIME;

	WIRELESS_TRACE(WIRELESS_DEBUG_LEVEL, "NEW WirelessInit start--[simPin: %s][FlagStatus:%d][Apn:%s][DialNo:%s]", stPPPCfg.szPin, gcIsPppConnected\
		, stPPPCfg.szApn, stPPPCfg.szDialNum);


	nRet = NAPI_WlmSetDialCfg(&stPPPCfg, sizeof(stPPPCfg));
	if (nRet != NAPI_OK)
	{
		WIRELESS_TRACE(WIRELESS_DEBUG_LEVEL, "NEW NAPI_WlmsetDailCfg fail");
		SetCommError(FAIL_WIRELESS_PPP_CONFIG, nRet);
		return APP_FAIL;
	}

	WIRELESS_TRACE(WIRELESS_DEBUG_LEVEL, "stPPPCfg.szPin = %s", stPPPCfg.szPin);
	gnHandle = 0;
	gcSslFlag = cSsl;

	if (0x01 == GetVarCommMode())
	{
		WireLessDial(FALSE, WIRELESS_DEFAULT_TIMEOUT);
	}

	WIRELESS_TRACE(WIRELESS_DEBUG_LEVEL, "NEW NAPI_WlmsetDailCfg succ");

	return APP_SUCC;
}

int WirelessConnectState(int *pnStatus)
{
	int nRet = 0;

	nRet = NAPI_WlmGetDataEnabled((EM_DATA_ENABLE *)pnStatus);
	if (nRet != NAPI_OK)
	{
		SetCommError(FAIL_WIRELESS_PPP_CHECK, nRet);
		WIRELESS_TRACE(WIRELESS_DEBUG_LEVEL, "NEW WirelessConnectState fail...[%d]", nRet);
		return APP_FAIL;
	}

	return APP_SUCC;
}

int WirelessHangUp(int nFlag)
{
	int nRet = 0;
	EM_DATA_ENABLE emStatus;

	WIRELESS_TRACE(WIRELESS_DEBUG_LEVEL, "NEW WirelessHangUp start...[ nFlag %d]", nFlag);

	CommserverLedShine(LED_COM_OFF);
	CommserverLedShine(LED_ONL_OFF);
	nRet = TcpHangUp(&gnHandle, gcSslFlag, nFlag);
	if (nRet != APP_SUCC)
	{
		WIRELESS_TRACE(WIRELESS_DEBUG_LEVEL, "TcpHangUp  hang up fail");
		return APP_FAIL;
	}
	gnHandle = 0;
	if (nFlag == HANGUP_PPP)
	{
		WIRELESS_TRACE(WIRELESS_DEBUG_LEVEL, "NEW WirelessHangUp ppp....");
		emStatus = WLM_STATE_OFF;
		nRet = NAPI_WlmSetDataEnabled(&emStatus);
		if(nRet != NAPI_OK)
		{
			SetCommError(FAIL_WIRELESS_PPP_HANGUP, nRet);
			WIRELESS_TRACE(WIRELESS_DEBUG_LEVEL, "NEW NAPI_PppHangup error..[%d]",nRet);
			return APP_FAIL;
		}
		gcIsPppConnected = FALSE;
		cIsRegister = FALSE;
		NAPI_WlmStatusUnregister();
	}
	WIRELESS_TRACE(WIRELESS_DEBUG_LEVEL, "NEW WirelessHangUp HangUp succ...[nFlag %d]", nFlag);

	return APP_SUCC;
}

static inline int _getPPPMinTimeout(int nTimeout)
{
	int nMinTimeout = 0;
	if (0x01 == GetVarCommMode())
	{
		nMinTimeout = WIRELESS_LPPP_MIN_TIMEOUT;
	}
	else
	{
		nMinTimeout = WIRELESS_SPPP_MIN_TIMEOUT;
	}
	return nTimeout > nMinTimeout ? nMinTimeout : nTimeout;
}

/**
* @brief Wireless connection
* @param [in] pstServerAddress
* @param [out] pnConnectIndex
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author Liug
* @date 2012-6-7
*/
int WirelessConnect(const STSERVERADDRESS * pstServerAddress,int *pnConnectIndex, int nTimeOut, int nIsPreConnect)
{
	int nRet = APP_FAIL;
	int i;
	int nTmpTimeOut = nTimeOut, nBeginTime, nMinTimeOut, nNextTime;
	char szIp[20] = {0};
	char cIsDnsCheck = YES;

	WIRELESS_TRACE(WIRELESS_DEBUG_LEVEL, "New WirelessConnect start---[nTimeOut:%d][ConnectIndex:%d][ppp:%d][nIsPreConnect : %d]",
		nTmpTimeOut, gnLastConnectIndex, gcIsPppConnected, nIsPreConnect);

	CommserverLedShine(LED_ONL_FLICK);
	nBeginTime = PubGetOverTimer(0);
	nNextTime = nBeginTime;

	/**< PPP had connected, skip */
	if (gcIsPppConnected == TRUE)
	{
		if (EM_PRECONNECT == nIsPreConnect)
		{
			WIRELESS_TRACE(WIRELESS_DEBUG_LEVEL, "PreConnect Succ...");
			return APP_SUCC;
		}
	}
	else
	{
		if (EM_PRECONNECT == nIsPreConnect)
		{
			if (0x01 != GetVarCommMode())
			{
				if (WireLessDial(FALSE, 0) != APP_SUCC)
				{
					WIRELESS_TRACE(WIRELESS_DEBUG_LEVEL, "New wireless PreConnect dial...");
					return APP_FAIL;
				}
			}
			WIRELESS_TRACE(WIRELESS_DEBUG_LEVEL, "New Wireless PreConnect Succ...");
			return APP_SUCC;
		}

		nMinTimeOut = _getPPPMinTimeout(nTmpTimeOut);
		nRet = WireLessCheckConnected(nMinTimeOut);
		if (nRet == APP_QUIT)
		{
			return APP_QUIT;
		}
		if (nRet != APP_SUCC)
		{
			WIRELESS_TRACE(WIRELESS_DEBUG_LEVEL, "NEW NWireLessCheckConnected fail");
			nNextTime = PubGetOverTimer(0);
			nTmpTimeOut -= GetTimeDif(nBeginTime, nNextTime);
			if (WireLessDial(TRUE, nTmpTimeOut) != APP_SUCC)
			{
				WIRELESS_TRACE(WIRELESS_DEBUG_LEVEL, "New Wireless dial fail...");
				return APP_FAIL;
			}
		}
	}
	nTmpTimeOut -= GetTimeDif(nNextTime, PubGetOverTimer(0));

	/* domain connection */
	if ((strlen(pstServerAddress->lszIp[0]) == 0) && (strlen(pstServerAddress->szDN) != 0))
	{
		while (1)
		{
			if (APP_SUCC != ParseDnsIp(WLM, pstServerAddress, &nTmpTimeOut, szIp, cIsDnsCheck))
			{
				return APP_FAIL;
			}

			//
			if (APP_SUCC != TcpOpen(&gnHandle, gcSslFlag))
			{
				return APP_FAIL;
			}
			nRet = TcpConnect(gnHandle, gcSslFlag, szIp,pstServerAddress->lnPort[0], nTmpTimeOut);//20150901 ssl5s
			if (APP_SUCC != nRet)
			{
				WirelessHangUp(HANGUP_SOCKET);

				if (YES == cIsDnsCheck)
				{
					cIsDnsCheck = NO;
					memset(szIp, 0, sizeof(szIp));
					continue;
				}
				WIRELESS_TRACE(WIRELESS_DEBUG_LEVEL, "New WirelessTcpConnect fail...");
				return APP_FAIL;
			}
			else
			{
				CommserverLedShine(LED_ONL_ON);
				PubDebugSelectly(3, "WirelessConnect succ");
				return APP_SUCC;
			}
		}
	}

	//connected before, get the last successful index
	if (gnLastConnectIndex != -1)
	{
		//
		if (APP_SUCC != TcpOpen(&gnHandle, gcSslFlag))
		{
			WIRELESS_TRACE(WIRELESS_DEBUG_LEVEL, "TcpOpen fail...");
			return APP_FAIL;
		}
		nRet = TcpConnect(gnHandle, gcSslFlag, pstServerAddress->lszIp[gnLastConnectIndex],pstServerAddress->lnPort[gnLastConnectIndex], nTmpTimeOut/3);
		if (APP_SUCC != nRet)
		{
			nTmpTimeOut -= nTmpTimeOut/3;
			gnLastConnectIndex = -1;
			WirelessHangUp(HANGUP_SOCKET);
			WIRELESS_TRACE(WIRELESS_DEBUG_LEVEL, "TcpConnect succ");
		}
		else
		{
			CommserverLedShine(LED_ONL_ON);
			*pnConnectIndex = gnLastConnectIndex;
			WIRELESS_TRACE(WIRELESS_DEBUG_LEVEL, "New WirelessConnect succ");
			return APP_SUCC;
		}
	}

	//IP
	for (i=0;i<2;i++)
	{
		//
		if (APP_SUCC != TcpOpen(&gnHandle, gcSslFlag))
		{
			WIRELESS_TRACE(WIRELESS_DEBUG_LEVEL, "TcpOpen fail");
			return APP_FAIL;
		}

		if (strlen(pstServerAddress->lszIp[i]) <= 0)
		{
			SetCommError(FAIL_TCPIP_CONNECTNULL, 0);
			WIRELESS_TRACE(WIRELESS_DEBUG_LEVEL, "Tcp Connect null error[%d]---[ServerIp:%s][ServerPort:%d].",nRet,\
				pstServerAddress->lszIp[i],pstServerAddress->lnPort[i]);
			WirelessHangUp(HANGUP_SOCKET);
			continue;
		}
		nRet = TcpConnect(gnHandle, gcSslFlag, pstServerAddress->lszIp[i],pstServerAddress->lnPort[i], nTmpTimeOut/2);
		if (nRet != NAPI_OK)
		{
			WirelessHangUp(HANGUP_SOCKET);
			continue;
		}
		CommserverLedShine(LED_ONL_ON);
		gnLastConnectIndex = i;
		*pnConnectIndex = gnLastConnectIndex;
		WIRELESS_TRACE(WIRELESS_DEBUG_LEVEL, "New WirelessConnect succ");
		return APP_SUCC;
	}
	WIRELESS_TRACE(WIRELESS_DEBUG_LEVEL, "New WirelessConnect fail");
	gnLastConnectIndex = -1;
	WirelessHangUp(HANGUP_SOCKET);

	return APP_FAIL;
}

int WirelessWrite(const char *psData,int nDataLen)
{
	int nRet;

	if (psData == NULL || nDataLen <= 0)
	{
		WIRELESS_TRACE(WIRELESS_DEBUG_LEVEL, "New wireless write param is invalid ...");
		SetCommErrorCode(FAIL_TCPIP_INVAILIDPARAM);
		return APP_FAIL;
	}

	nRet = TcpWrite(gnHandle, gcSslFlag, psData, nDataLen);
	if (APP_SUCC != nRet)
	{
		WirelessHangUp(HANGUP_SOCKET);
		WIRELESS_TRACE(WIRELESS_DEBUG_LEVEL, "New Wireless write fail");
		return APP_FAIL;
	}
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
* @date 2012-5-23
*/
int WirelessRead(int nIsNonBlock, int nLen,int nTimeOut,char *psOutData)
{
	int nRet;
	uint unReadLen = 0;//

	nRet = TcpRead(nIsNonBlock, gnHandle, gcSslFlag, psOutData, nLen, &unReadLen, nTimeOut);
	switch(nRet)
	{
	case APP_QUIT:
	case APP_TIMEOUT:
	case APP_FAIL:
		WirelessHangUp(HANGUP_SOCKET);
		return nRet;
	case APP_SUCC:
		if (unReadLen > 0) {
			SetLedComFlick();
		}
	default:
		break;
	}

	return unReadLen;
}

int WirelessGetTcpHandle(int* pnHandle)
{
	return TcpGetHandle(gnHandle, pnHandle);
}

void WirelessResetIndex(void)
{
	gnLastConnectIndex = -1;
}

void WirelessUpdateSslFlag(char cSsl)
{
	gcSslFlag = cSsl;
}

int WirelessPppDial(int nTimeOut)
{
	int nRet = 0;
	int nBeginTime, nTmpTimeOut = nTimeOut, nMinTimeOut;

	PubDebugSelectly(3, "New WirelessPppDial start...[timeout:%d]", nTimeOut);

	if((0x01 != GetVarCommMode() && FALSE == gcIsPppConnected))
	{
		if (WireLessDial(FALSE, 0) != APP_SUCC)
		{
			return APP_FAIL;
		}
	}
	nBeginTime = PubGetOverTimer(0);
	nMinTimeOut = _getPPPMinTimeout(nTmpTimeOut);
	nRet = WireLessCheckConnected(nMinTimeOut);
	if (nRet == APP_QUIT)
	{
		return APP_FAIL;
	}
	else if (nRet != APP_SUCC)
	{
		nTmpTimeOut -= GetTimeDif(nBeginTime, PubGetOverTimer(0));
		nRet = WireLessDial(TRUE, nTmpTimeOut);
		if (nRet != APP_SUCC)
		{
			return nRet;
		}
	}

	return APP_SUCC;
}


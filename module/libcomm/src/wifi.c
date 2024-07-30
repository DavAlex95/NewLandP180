/**
* @file nwifi.c
* @brief new wifi module use forth interface
* @version  1.0
* @author chenxiulin
* @date 2019-07-16
*/
#include <pthread.h>
#include "lcomm.h"
#include "wifi.h"
#include "commerror.h"
#include "commtool.h"
#include "lui.h"
#include "ltool.h"
#include "process.h"
#include "napi_net.h"

#define WIFI_DEBUG_LEVEL		(3)
#define WIFI_DEFAULT_TIMEOUT	(30)
#define WIFI_MIN_TIMEOUT		(5)

#define WIFI_TRACE(level, fmt, args...) \
    PubDebugSelectly(level, "[%s][%s][%d]>>>" fmt, __FILE__, __FUNCTION__, __LINE__, ##args)

typedef int (*ConFunc)(); /**< condition function */

static int gnHandle;					/**<Socket handle*/
static int gnLastConnectIndex = -1;		/**<the last socket handle index*/
static char gcWifiAPConnected = FALSE;	/**<AP connected status */
static char gcSslFlag;					/**<use ssl or not */
static char gcIsNWifiInit = FALSE;  	/**<WIFI init or not*/
static ST_WIFI_CONFIG_T gstWifi;
static EM_WIFI_SIGNAL_T gemSignal;  /**< used for multiThread communication */
static pthread_mutex_t gmutex = PTHREAD_MUTEX_INITIALIZER; /**< used for Ensuring Thread Safety*/

static int WifiDial(int nWait, int nTimeOut);
static int ProGetWifiAPConnected(void);
static void WifiDumpParam(ST_WIFI_CONFIG_T *pstWifi)
{
	WIFI_TRACE(WIFI_DEBUG_LEVEL, "=========  WIFI_PARAM_DUMP  [start]=========");
	WIFI_TRACE(WIFI_DEBUG_LEVEL, "szSsid: [%s]", pstWifi->szSsid);
	WIFI_TRACE(WIFI_DEBUG_LEVEL, "szPasswd: [%s]", pstWifi->szPasswd);
	WIFI_TRACE(WIFI_DEBUG_LEVEL, "nNetId: [%d]", pstWifi->nNetId);
	WIFI_TRACE(WIFI_DEBUG_LEVEL, "ucFlag: [%d]", pstWifi->ucFlag);
	WIFI_TRACE(WIFI_DEBUG_LEVEL, "emAuthType: [%d]", pstWifi->emAuthType);
	WIFI_TRACE(WIFI_DEBUG_LEVEL, "nPriority: [%d]", pstWifi->nPriority);
	WIFI_TRACE(WIFI_DEBUG_LEVEL, "szMacAddr: [%02x %02x %02x %02x %02x}", pstWifi->stMac.szMacAddr[0],
		pstWifi->stMac.szMacAddr[1], pstWifi->stMac.szMacAddr[2], pstWifi->stMac.szMacAddr[3],
		pstWifi->stMac.szMacAddr[4]);
	WIFI_TRACE(WIFI_DEBUG_LEVEL, "szIpAddr: [%s]", pstWifi->stIpAddr.szIpAddr);
	WIFI_TRACE(WIFI_DEBUG_LEVEL, "szNetMask: [%s]", pstWifi->stIpAddr.szNetMask);
	WIFI_TRACE(WIFI_DEBUG_LEVEL, "szGateWay: [%s]", pstWifi->stIpAddr.szGateWay);
	WIFI_TRACE(WIFI_DEBUG_LEVEL, "szDnsPrimary: [%s]", pstWifi->stIpAddr.szDnsPrimary);
	WIFI_TRACE(WIFI_DEBUG_LEVEL, "szDnsSecondary: [%s]", pstWifi->stIpAddr.szDnsSecondary);
	WIFI_TRACE(WIFI_DEBUG_LEVEL, "gcSslFlag: [%d]", gcSslFlag);
	WIFI_TRACE(WIFI_DEBUG_LEVEL, "gcIsNWifiInit: [%d]", gcIsNWifiInit);
	WIFI_TRACE(WIFI_DEBUG_LEVEL, "gnLastConnectIndex: [%d]", gnLastConnectIndex);
	WIFI_TRACE(WIFI_DEBUG_LEVEL, "AP connected: [%d]", ProGetWifiAPConnected());
	WIFI_TRACE(WIFI_DEBUG_LEVEL, "=========  WIFI_PARAM_DUMP  [end]=========");
}

static int WifiWaitTimeOut(int nTimeOut, int (*ConditionFunc)())
{
	char cFlag = FALSE;
	uint unOverTime;

	if (nTimeOut <= 0)
	{
		SetCommError(APP_TIMEOUT, 0);
		return APP_TIMEOUT;
	}

	unOverTime = PubGetOverTimer(nTimeOut*1000);
	while (1)
	{
		cFlag = (*ConditionFunc) == NULL ? FALSE : (*ConditionFunc)();
		if (cFlag == TRUE)
		{
			break;
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

static int CheckWifiState(EM_WIFI_STATE_T inState)
{
	NAPI_ERR_CODE emRet;
	EM_WIFI_STATE_T state;

	emRet = NAPI_WifiGetState(&state);
	if (emRet != NAPI_OK || state != inState)
	{
		return FALSE;
	}
	return TRUE;
}

static int WifiIsOpened(void)
{
	return CheckWifiState(WIFI_STATE_ENABLED);
}

static int WifiIsClosed(void)
{
	return CheckWifiState(WIFI_STATE_DISABLED);
}

int WifiSetEnable(EM_WIFI_ENABLE enable, int nWait)
{
	int nRet, nCommErr;

	ConFunc pConfunc[2] = {
		WifiIsClosed,
		WifiIsOpened,
		};

	nRet = NAPI_WifiSetEnabled(enable);
	if (nRet != NAPI_OK)
	{
		nCommErr = enable == WIFI_DISABLE ? FAIL_WIFI_DISABLE : FAIL_WIFI_ENABLE;
		SetCommError(nCommErr, nRet);
		return nRet;
	}

	if (nWait == TRUE)
	{
		if ((nRet = WifiWaitTimeOut(WIFI_DEFAULT_TIMEOUT, pConfunc[enable])) != APP_SUCC)
		{
			return APP_QUIT;
		}
	}

	return APP_SUCC;
}

void WifiSetSignalVal(EM_WIFI_SIGNAL_T emSignal)
{
	pthread_mutex_lock(&gmutex);
	gemSignal = emSignal;
	switch (gemSignal)
	{
	case WIFI_SIGNAL_CONNECTED:
		gcWifiAPConnected = TRUE;
		break;
	case WIFI_SIGNAL_AUTH_FAILED:        /**< Authentication failure */
	case WIFI_SIGNAL_DISCONNECTED:       /**< Disconnected */
		gcWifiAPConnected = FALSE;
		break;
	default:
		break;
	}
	pthread_mutex_unlock(&gmutex);

}

EM_WIFI_SIGNAL_T WifiGetSignalVal(void)
{
	EM_WIFI_SIGNAL_T emSignal;

	pthread_mutex_lock(&gmutex);
	emSignal = gemSignal;
	pthread_mutex_unlock(&gmutex);

	return emSignal;
}

int ProGetWifiAPConnected(void)
{
	char cConnected;

	pthread_mutex_lock(&gmutex);
	cConnected = gcWifiAPConnected;
	pthread_mutex_unlock(&gmutex);

    if (cConnected == TRUE)
    {
        return cConnected;
    }

    ST_WIFI_INFO_T stConnectInfo;
    if (NAPI_WifiGetConnectionInfo(&stConnectInfo) == NAPI_OK  && stConnectInfo.emConState == WIFI_CONNECTED)
    {
        cConnected = TRUE;
        WifiSetSignalVal(WIFI_SIGNAL_CONNECTED);
    }

	return cConnected;
}

void WifiSignalCallback(EM_WIFI_SIGNAL_T signal, void *arg)
{
	//WIFI_TRACE(WIFI_DEBUG_LEVEL, "enter New WIFI signal callback [signal]:[%d]..", signal);
	WifiSetSignalVal(signal);
}

static int WifiRegSignalFunc(void)
{
	static char cIsRegister = FALSE;
	int nRet = 0;

	if (cIsRegister == TRUE)
	{
		WIFI_TRACE(WIFI_DEBUG_LEVEL, "New NWifiRegSignalFunc is ok");
		return APP_SUCC;
	}

	nRet = NAPI_WifiRegisterSignalFunc(WifiSignalCallback);
	if (nRet != NAPI_OK)
	{
		SetCommError(FAIL_WIFI_RIGSTERSIGNAL, nRet);
		WIFI_TRACE(WIFI_DEBUG_LEVEL, "New NWifiRegSignalFunc is fail");
		return APP_FAIL;
	}
	cIsRegister = TRUE;

	return APP_SUCC;
}

static int WifiCoreInit(void)
{
	if (gcIsNWifiInit == TRUE)
	{
		WIFI_TRACE(WIFI_DEBUG_LEVEL, "New WIFI already Init succ..");
		return APP_SUCC;
	}
	/* register new wifi signal callback.*/
	if (WifiRegSignalFunc() != APP_SUCC)
	{
		return APP_FAIL;
	}

	/* check wifi state, TCP can be created only if the wifi state is enable */
	if (!CheckWifiState(WIFI_STATE_ENABLED) && !CheckWifiState(WIFI_STATE_ENABLING))
	{
		if (WifiSetEnable(WIFI_ENABLE, TRUE) != APP_SUCC)
		{
            WIFI_TRACE(WIFI_DEBUG_LEVEL, "New WIFI Init fail..");
			return APP_FAIL;
		}
	}
	gcIsNWifiInit = TRUE;
	WIFI_TRACE(WIFI_DEBUG_LEVEL, "New WIFI Init succ..");

	return APP_SUCC;
}

void WifiResetIndex(void)
{
	gnLastConnectIndex = -1;
}

void WifiUpdateSslFlag(char cSsl)
{
	gcSslFlag = cSsl;
}

int WifiSetParam(STWIFIPARAM *pstWifiParam)
{
	WIFI_TRACE(WIFI_DEBUG_LEVEL, "New WifiSetParam start...");
	if (pstWifiParam == NULL)
	{
		SetCommError(FAIL_WIFI_INVAILIDPARAM, 0);
		WIFI_TRACE(WIFI_DEBUG_LEVEL, "New WifiSetParam invalid param ");
		return APP_FAIL;
	}

	memcpy(gstWifi.szSsid, pstWifiParam->szSsid, 32);
	memcpy(gstWifi.szPasswd, pstWifiParam->szKey, 32);

    gstWifi.emAuthType = (EM_WIFI_AUTH_MODE)pstWifiParam->cWifiMode;
    if (pstWifiParam->ucIfDHCP != '1' && pstWifiParam->ucIfDHCP != 1)
	{
		gstWifi.ucFlag |= WIFI_AP_FLAG_STATIC_IP_SET;
	}
	else
	{
		gstWifi.ucFlag &= ~ WIFI_AP_FLAG_STATIC_IP_SET;
	}
	memcpy(gstWifi.stIpAddr.szIpAddr, pstWifiParam->szIP, 16);
	memcpy(gstWifi.stIpAddr.szNetMask, pstWifiParam->szMask, 16);
	memcpy(gstWifi.stIpAddr.szGateWay, pstWifiParam->szGateway, 16);
	strcpy(gstWifi.stIpAddr.szDnsPrimary, pstWifiParam->szDnsPrimary);
	strcpy(gstWifi.stIpAddr.szDnsSecondary, pstWifiParam->szDnsSecondary);
	gstWifi.nNetId = pstWifiParam->nNetId;

	WIFI_TRACE(WIFI_DEBUG_LEVEL, "New WifiSetParam succ");

	return APP_SUCC;
}

int WifiInit(STWIFIPARAM *pstWifiParam, char cSsl)
{
	if (pstWifiParam == NULL)
	{
		SetCommError(FAIL_WIFI_INVAILIDPARAM, 0);
		WIFI_TRACE(WIFI_DEBUG_LEVEL, "New WirelessInit invalid param");
		return APP_FAIL;
	}

	if (WifiCoreInit() != APP_SUCC)
	{
		SetCommError(FAIL_WIFI_INIT, 0);
		WIFI_TRACE(WIFI_DEBUG_LEVEL, "New WifiCoreInit int fail");
		return APP_FAIL;
	}

	/* set param */
	WifiSetParam(pstWifiParam);
	gnHandle = 0;
	gcSslFlag = cSsl;

	/* predictive dialer (means connect wifi hot spot)*/
	WifiDial(FALSE, 0);

	return APP_SUCC;
}

int WifiGetTcpHandle(int *pnHandle)
{
	return TcpGetHandle(gnHandle, pnHandle);
}

static int WifiCheckConnected(int nTimeOut)
{
	return WifiWaitTimeOut(nTimeOut, ProGetWifiAPConnected);
}

int WifiConnect(const STSERVERADDRESS *pstServerAddress,int *pnConnectIndex,int nTimeOut, int nIsPreConnect)
{
	int nRet = 0;
	int i, nBeginTime;
	int nTmpTimeOut = nTimeOut;
	char szIp[20] = {0};
	char cIsDnsCheck = YES;
	char cWifiAPConnected;

	WIFI_TRACE(WIFI_DEBUG_LEVEL, "WIFI connect enter ...");
	if (pstServerAddress == NULL || pnConnectIndex == NULL)
	{
		SetCommError(FAIL_WIFI_INVAILIDPARAM, 0);
		WIFI_TRACE(WIFI_DEBUG_LEVEL, "WIFI break of, shutdown wifi...");
		return APP_FAIL;
	}
	CommserverLedShine(LED_ONL_FLICK);

	cWifiAPConnected = ProGetWifiAPConnected();
	WIFI_TRACE(WIFI_DEBUG_LEVEL, "NEW WIFI connected [%d]..", cWifiAPConnected);
	nBeginTime = PubGetOverTimer(0);
	if (cWifiAPConnected == TRUE)
	{
		if (EM_PRECONNECT == nIsPreConnect) /* connect the wifi */
		{
			WIFI_TRACE(WIFI_DEBUG_LEVEL, "NEW PreConnect Succ...");
			return APP_SUCC;
		}
	}
	else
	{
		if (EM_PRECONNECT == nIsPreConnect)
		{
			WifiDial(FALSE, 0);
			WIFI_TRACE(WIFI_DEBUG_LEVEL, "New PreConnect Succ...");
			return APP_SUCC;
		}
		if (WifiDial(TRUE, nTmpTimeOut) != APP_SUCC)
		{
			WIFI_TRACE(WIFI_DEBUG_LEVEL, "New NWifiDial fail...");
			return APP_FAIL;
		}
	}
	nTmpTimeOut -= GetTimeDif(nBeginTime, PubGetOverTimer(0));


	/**< domain connection */
	if ((strlen(pstServerAddress->lszIp[0]) == 0) && (strlen(pstServerAddress->szDN) != 0))
	{
		WIFI_TRACE(WIFI_DEBUG_LEVEL, " enter szDN");
		while (1)
		{
			if (APP_SUCC != ParseDnsIp(WIFI, pstServerAddress, &nTmpTimeOut, szIp, cIsDnsCheck))
			{
				WIFI_TRACE(WIFI_DEBUG_LEVEL, "ParseDnsIp fail....");
				return APP_FAIL;
			}

			if (APP_SUCC != TcpOpen(&gnHandle, gcSslFlag))
			{
				return APP_FAIL;
			}
			nRet = TcpConnect(gnHandle, gcSslFlag, szIp,pstServerAddress->lnPort[0], nTmpTimeOut);
			if (APP_SUCC != nRet)
			{
				WifiHangUp(HANGUP_SOCKET);
				if (YES == cIsDnsCheck)
				{
					cIsDnsCheck = NO;
					memset(szIp, 0, sizeof(szIp));
					continue;
				}
				WIFI_TRACE(WIFI_DEBUG_LEVEL, "NEW WIFI TcpConnect fail...");
				return APP_FAIL;
			}
			else
			{
				CommserverLedShine(LED_ONL_ON);
				WIFI_TRACE(WIFI_DEBUG_LEVEL, "WifiConnect succ");
				return APP_SUCC;
			}
		}
	}
	WIFI_TRACE(WIFI_DEBUG_LEVEL, " start to gnLastConnectIndex %d", gnLastConnectIndex);
	if (gnLastConnectIndex != -1)
	{
		if (APP_SUCC != TcpOpen(&gnHandle, gcSslFlag))
		{
			WIFI_TRACE(WIFI_DEBUG_LEVEL, "TcpOpen fail....");
			return APP_FAIL;
		}

		nRet = TcpConnect(gnHandle, gcSslFlag, pstServerAddress->lszIp[gnLastConnectIndex],pstServerAddress->lnPort[gnLastConnectIndex], nTmpTimeOut/3);
		if (APP_SUCC != nRet)
		{
			nTmpTimeOut -= nTmpTimeOut/3;
			gnLastConnectIndex = -1;
			WifiHangUp(HANGUP_SOCKET);
		}
		else
		{
			CommserverLedShine(LED_ONL_ON);
			*pnConnectIndex = gnLastConnectIndex;
			WIFI_TRACE(WIFI_DEBUG_LEVEL, "WifiConnect succ");
			return APP_SUCC;
		}
	}
	WIFI_TRACE(WIFI_DEBUG_LEVEL, " start to open tcp");

	for (i=0; i<2; i++)
	{
		if (APP_SUCC != TcpOpen(&gnHandle, gcSslFlag))
		{
			WIFI_TRACE(WIFI_DEBUG_LEVEL, "TcpOpen fail....");
			return APP_FAIL;
		}
		WIFI_TRACE(WIFI_DEBUG_LEVEL, "TcpOpen");
		if (strlen(pstServerAddress->lszIp[i]) <= 0)
		{
			SetCommError(FAIL_TCPIP_CONNECTNULL, 0);
			WIFI_TRACE(WIFI_DEBUG_LEVEL, "Tcp Connect null error[%d]---[ServerIp:%s][ServerPort:%d].",nRet,\
				pstServerAddress->lszIp[i],pstServerAddress->lnPort[i]);
			WifiHangUp(HANGUP_SOCKET);
			continue;
		}
		WIFI_TRACE(WIFI_DEBUG_LEVEL, "nTmpTimeOut/2 = %d", nTmpTimeOut/2);
		nRet = TcpConnect(gnHandle, gcSslFlag, pstServerAddress->lszIp[i],pstServerAddress->lnPort[i], nTmpTimeOut/2);
		if (nRet != NAPI_OK)
		{
			WifiHangUp(HANGUP_SOCKET);
			continue;
		}
		CommserverLedShine(LED_ONL_ON);
		gnLastConnectIndex = i;
		*pnConnectIndex = gnLastConnectIndex;
		WIFI_TRACE(WIFI_DEBUG_LEVEL, "WifiConnect succ");
		return APP_SUCC;
	}
	gnLastConnectIndex = -1;
	WifiHangUp(HANGUP_SOCKET);
	WIFI_TRACE(WIFI_DEBUG_LEVEL, "New WirelessConnect fail");

	return APP_FAIL;
}

int WifiClearBuf(void)
{
	WIFI_TRACE(WIFI_DEBUG_LEVEL, "NWifiClearBuf succ ");

	return APP_SUCC;
}

int WifiGetConnectState(void)
{
	return TcpGetStatus(gnHandle, gnLastConnectIndex);
}

int WifiHangUp(int nFlag)
{
	int nRet = 0;

	WIFI_TRACE(WIFI_DEBUG_LEVEL, "WIFI HangUp start---[flag:%d]", nFlag);

	CommserverLedShine(LED_COM_OFF);
	CommserverLedShine(LED_ONL_OFF);
	nRet = TcpHangUp(&gnHandle, gcSslFlag, nFlag);
	if (nRet != APP_SUCC)
	{
		return APP_FAIL;
	}
	gnHandle = 0;
	if (nFlag == HANGUP_PPP)
	{
		NAPI_WifiDisconnectNetwork();
		nRet = WifiSetEnable(WIFI_DISABLE, FALSE);
		if(nRet != APP_SUCC)
		{
			SetCommError(FAIL_WIFI_SHUTDOWN, nRet);
			WIFI_TRACE(WIFI_DEBUG_LEVEL, "NWifiSetEnable error[%d]",nRet);
			return APP_FAIL;
		}
		gcIsNWifiInit = FALSE;
		WIFI_TRACE(WIFI_DEBUG_LEVEL, "WIFI Shutdown succ...");
	}
	return APP_SUCC;
}

static int WifiIsScanOver(void)
{
	return WifiGetSignalVal() == WIFI_SIGNAL_SCAN_RESULTS ? TRUE : FALSE;
}

int WifiScan(ST_WIFI_AP_LIST *pstList)
{
	int nRet;

	if (pstList == NULL)
	{
		WIFI_TRACE(WIFI_DEBUG_LEVEL, "New WIFI scan param is NULL...");
		SetCommErrorCode(FAIL_WIFI_SCAN);
		return APP_FAIL;
	}

	if (WifiCoreInit() != APP_SUCC)
	{
		return APP_FAIL;
	}

    WifiSetSignalVal(WIFI_SIGNAL_UNKNOWN);
	nRet = NAPI_WifiStartScan();
	if (nRet != NAPI_OK)
	{
		SetCommError(FAIL_WIFI_SCAN, nRet);
		WIFI_TRACE(WIFI_DEBUG_LEVEL, "NAPI_WifiStartScan fail.nRet [%d]..", nRet);
		return APP_FAIL;
	}

	if ((nRet = WifiWaitTimeOut(WIFI_DEFAULT_TIMEOUT, WifiIsScanOver)) != APP_SUCC)
	{
		SetCommErrorCode(nRet);
		return APP_FAIL;
	}
	nRet = NAPI_WifiGetScanResult(pstList);
	if (nRet < 0)
	{
		SetCommError(FAIL_WIFI_SCAN, nRet);
		WIFI_TRACE(WIFI_DEBUG_LEVEL, "New WIFI NAPI_WifiGetScanResult fail...");
		return APP_FAIL;
	}
	WIFI_TRACE(WIFI_DEBUG_LEVEL, "New WIFI scan succ..");

	return APP_SUCC;
}

static int WifiDial(int nWait, int nTimeOut)
{
	int nNet_id = -1, nRet;

	WifiDumpParam(&gstWifi);

	if (WifiCoreInit() != APP_SUCC)
	{
		WIFI_TRACE(WIFI_DEBUG_LEVEL, "New WIFI dial & core init fail...");
		return APP_FAIL;
	}
	WIFI_TRACE(WIFI_DEBUG_LEVEL, "NAPI_NAPI_WifiAddNetwork start..");
	nNet_id = NAPI_WifiAddNetwork(gstWifi);
	if (nNet_id < 0)
	{
		SetCommError(FAIL_WIFI_ADDNETWORK, nNet_id);
		WIFI_TRACE(WIFI_DEBUG_LEVEL, "NAPI_WifiAddNetwork fail...%d", nNet_id);
		return APP_FAIL;
	}
	//NAPI_WifiDisableNetwork(nNet_id);
	//WifiSetSignalVal(WIFI_SIGNAL_DISCONNECTED);
	nRet = NAPI_WifiConnectNetwork(nNet_id);
	if (nRet != NAPI_OK)
	{
		SetCommError(FAIL_WIFI_CONNECT, nRet);
		WIFI_TRACE(WIFI_DEBUG_LEVEL, "NAPI_WifiConnectNetwork fail...");
		return APP_FAIL;
	}
	WIFI_TRACE(WIFI_DEBUG_LEVEL, "NAPI_WifiConnectNetwork end..");
	if (nWait == TRUE)
	{
		WIFI_TRACE(WIFI_DEBUG_LEVEL, "WifiCheckConnected wait..nTimeOut %d", nTimeOut);
		if ((nRet = WifiCheckConnected(nTimeOut)) != APP_SUCC)
		{
			SetCommError(nRet, 0);
			WIFI_TRACE(WIFI_DEBUG_LEVEL, "New Wifi dial fail ...");
			return APP_FAIL;
		}
	}
	WIFI_TRACE(WIFI_DEBUG_LEVEL, "New Wifi dial succ");

	return APP_SUCC;
}

int WifiWrite(const char *psData, int nDataLen)
{
	int nRet;

	if (psData == NULL || nDataLen <= 0)
	{
		WIFI_TRACE(WIFI_DEBUG_LEVEL, "New Wifi write param is invalid ...");
		SetCommErrorCode(FAIL_TCPIP_INVAILIDPARAM);
		return APP_FAIL;
	}
	nRet = TcpWrite(gnHandle, gcSslFlag, psData, nDataLen);
	if (APP_SUCC != nRet)
	{
		WifiHangUp(HANGUP_SOCKET);
		return APP_FAIL;
	}

	return APP_SUCC;
}

int WifiRead(int nIsNonBlock,int nLen,int nTimeOut,char *psOutData)
{
	int nRet;
	uint unReadLen = 0;

	if (psOutData == NULL)
	{
		WIFI_TRACE(WIFI_DEBUG_LEVEL, "New Wifi read param is invalid ...");
		SetCommError(FAIL_TCPIP_INVAILIDPARAM, 0);
		return APP_FAIL;
	}

	nRet = TcpRead(nIsNonBlock, gnHandle, gcSslFlag, psOutData, nLen, &unReadLen, nTimeOut);
	switch (nRet)
	{
	case APP_QUIT:
	case APP_TIMEOUT:
	case APP_FAIL:
		WifiHangUp(HANGUP_SOCKET);
		return nRet;
	case APP_SUCC:
		if (unReadLen > 0)
		{
			SetLedComFlick();
		}
	default:
		break;
	}

	return unReadLen;
}

int WifiSsidConnect(int nTimeOut)
{
	int nRet = 0;
	int nTmpTimeOut = nTimeOut;
	int nBeginTime;

	WIFI_TRACE(WIFI_DEBUG_LEVEL, "WifiSsidConnect start...");

	nBeginTime = PubGetOverTimer(0);
	nRet = WifiCheckConnected(nTmpTimeOut/2);
	if (nRet == APP_QUIT)
	{
		return APP_FAIL;
	}
	else if (nRet != APP_SUCC)
	{
		nRet = WifiDial(FALSE, 0);
		if (nRet != APP_SUCC)
		{
			return nRet;
		}
		nTmpTimeOut -= GetTimeDif(nBeginTime, PubGetOverTimer(0));
		nRet = WifiCheckConnected(nTmpTimeOut);
		if (nRet != APP_SUCC)
		{
			return nRet;
		}
	}

	return APP_SUCC;
}

int WifiConnectState(int *pnStatus)
{
	*pnStatus = ProGetWifiAPConnected();

	return APP_SUCC;
}

int WifiGetCurrentInfo(ST_WIFI_INFO_T *pstCurrWifiinfo)
{
    if (pstCurrWifiinfo == NULL)
    {
        return APP_FAIL;
    }

	if (NAPI_WifiGetConnectionInfo(pstCurrWifiinfo) != NAPI_OK)
	{
		return APP_FAIL;
	}

	return APP_SUCC;
}


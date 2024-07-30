/*
* Newland payment(c) 2006-2008
*
* POS standard public library
* main module  --- lcomm.c
* Author    		Ling
* Date    		2012-06-19
*/
#include <sys/stat.h>
#include <dirent.h>
#include "libapiinc.h"
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/wait.h>
#include <time.h>
#include "commerror.h"
#include <pthread.h>
#include "ltool.h"
#include "commtool.h"
#include "process.h"
#include "napi.h"
#include "napi_wifi.h"
#include "wifi.h"
#include "asyndial.h"
#include "syndial.h"
#include "eth.h"
#include "wireless.h"

#define COMMLIBVER "ALCOMM0117052301"

extern STCOMM gstCommParam;
static STSHOWINFOXY gstShowInfoxy= {0};
static pthread_t gnThread = 0;
static int gnTimerCount = 0;
static char gcIsTimerOn = FALSE;

static void Showinfo();
/**
* global variable
*/
static const char szErrCode[][48]=
{
	{"-2001Invalid Serial Port Params"},
	{"-2002Init Serial Port Err"},
	{"-2003Serial Port Write Err"},
	{"-2004Serial Port Read Err"},
	{"-2005Clear Serial Port Buffer Err"},
	{"-2006Serial Port Timeout"},
	{"-2007Serial Port Buffer Overflow"},
	{"-2101Invalid Sync Modem Params"},
	{"-2102Init Sync Modem Err"},
	{"-2103Sync Modem Dial Err"},
	{"-2104Line Disconnected"},
	{"-2105Line Occupied"},
	{"-2106Sync Modem Detection Err"},
	{"-2107Sync Modem No Dial Tone"},
	{"-2108Sync Modem No Carrier"},
	{"-2109Sync Modem Line Busy"},
	{"-2110Sync Modem CMD Line Err"},
	{"-2111Sync Modem No Pre-dial"},
	{"-2112Sync Modem Unknown Err"},
	{"-2113Sync Modem Write Err"},
	{"-2114Sync Modem Read Err"},
	{"-2115Sync Modem Hangup Err"},
	{"-2116Sync Modem Num Not Exist"},
	{"-2201Invalid Async Modem Parames"},
	{"-2202Async Modem Init Err"},
	{"-2203Async Modem Dial Err"},
	{"-2204Line Disconnected"},
	{"-2205Line Occupied"},
	{"-2206Async Modem Detection Err"},
	{"-2207Async Modem No Dial Tone"},
	{"-2208Async Modem No Carrier"},
	{"-2209Async Modem Line Busy"},
	{"-2210Async Modem CMD Line Err"},
	{"-2211Async Modem No Pre-dial"},
	{"-2212Async Modem Unknown Err"},
	{"-2213Async Modem Write Err"},
	{"-2214Async Modem Read Err"},
	{"-2215Async Modem Hangup Err"},
	{"-2216Async Modem Buffer Clear Err"},
	{"-2217Async Modem Num Not Exist"},
	{"-2301Invalid Ethernet Params"},
	{"-2302Ethernet Set Local Address Err"},
	{"-2303TCP Opening Err"},
	{"-2304TCP Binding Err"},
	{"-2305DHCP Err"},
	{"-2306Invalid Service IP address"},
	{"-2307TCP Connect Err"},
	{"-2308TCP Write Err"},
	{"-2309TCP Read Err"},
	{"-2310TCP Close Err"},
	{"-2311TCP Wait Err"},
	{"-2312One-way SSL Open Err"},
	{"-2313Two-way SSL Open Err"},
	{"-2314Load CA Cert Err"},
	{"-2315Load Local Cert Err"},
	{"-2316Load Cert Private Key Err"},
	{"-2317SSL Connect Err"},
	{"-2318SSL Write Err"},
	{"-2319SSL Read Err"},
	{"-2320SSL Disconnect Err"},
	{"-2321SSL Close Err"},
	{"-2322Set DNS IP Address Err"},
	{"-2323DN Resolution Err"},
	{"-2324Set Socket Alive Err"},
	{"-2325Eth Line Disconnected"},
	{"-2401Invalid Wireless Modem params"},
	{"-2402No SIM card"},
	{"-2403Incorrect PWD"},
	{"-2404SIM Card is Locked"},
	{"-2405SIM Card Undefined Error"},
	{"-2406SIM Card Return Err"},
	{"-2407Init Wireless modem Err"},
	{"-2408Obtain Wireless Signal Err"},
	{"-2409PPP Config Err"},
	{"-2410PPP Connect Err"},
	{"-2411PPP Detection Err"},
	{"-2412PPP Disconnected"},
	{"-2413PPP Get IP Address Err"},
	{"-2414PPP Hangup Err"},
	{"-2415NEW WLM RIGSTER ERR"},/**< used for new api */
	{"-2416NEW WLM GETENABLE ERR"},/**< used for new api */
	{"-2417NEW WLM GET SIM STATUS ERR"},/**< used for new api */
	{"-2501Invalid WIFI Params"},
	{"-2502Init WIFI Err"},
	{"-2503WIFI Connect Err"},
	{"-2504WIFI Get Status Err"},
	{"-2505WIFI Disconnected"},
	{"-2506WIFI Get IP Address Err"},
	{"-2507WIFI Close Err"},
	{"-2508WIFI Scan Err"},
	{"-2509WIFI Get SSID Mode Err"},
	{"-2510NEW WIFI REGISTER SIGNAL ERR"},/**< used for new api */
	{"-2511NEW WIFI ENABLE ERR"},/**< used for new api */
	{"-2512NEW WIFI DISABLE ERR"},/**< used for new api */
	{"-2513NEW WIFI ADD NETWORK ERR"},/**< used for new api */
	{"-2514NEW WIFI Scan Get Result ERR"},/**< used for new api */
	{"-2601Invalid MODEM Params"},
	{"-2602Init MODEM Err"},
	{"-2603MODEM PPP Check Err"},
	{"-2604MODEM PPP Diconnected"},
	{"-2605MODEM Get Addr Err"},
	{"-2606MODEM PPP Config Err"},
	{"-2607MODEM Dial Err"},
	{"-2608Telephone Line Disconnected"},
	{"-2609Line Occupied"},
	{"-2610Internet Dial-up Err"},
	{"-2608MODEM PPP Dial Err"},
	{"-2609MODEM Read Err"},
	{"-2610MODEM Write Err"},
	{"-2611MODEM PPP Hangup Err"},
	{"-2612MODEM Hangup Err"},
	{"-2   User Cancel"},
	{"-3   Timeout"},
	{"-9999Unknown error code"},
};

static int InsertAsyn(char *psBuf, uint *punLen);
static int InsertAsyn1(char *psBuf, uint *punLen);
static int AddTpdu(char *psBuf, uint *punLen, char *psTpdu);

static int DeleteAsyn(char *psBuf, uint *punLen);
static int DeleteAsyn1(char *psBuf, uint *punLen);
static int DelTpdu(char *psBuf, uint *punLen);

static int ProShowInfo();
static void ProExitShowInfo(void);

static int ProGetErrMsg(int nCommErrCode, char *pszErrMsg);


/**
* @brief Check whether the device support the communication type
* @param [in] cCommType
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author sunh
* @date 2014-3-24
*/
static int CommCheckIfSupport(char cCommType)
{
	char szBuf[100] = {0};

	switch (cCommType & 0xF0)
	{
	case COMMTYPE_SYNDIAL:
	case COMMTYPE_ASYNDIAL:
		if (APP_SUCC != PubGetHardwareSuppot(HARDWARE_SUPPORT_MODEM,szBuf))
		{
			PubMsgDlg(NULL, "The terminal doesn't support <DIAL>, please change settings", 3, 3);
			return APP_FAIL;
		}
		break;
	case COMMTYPE_PORT:
		if (APP_SUCC != PubGetHardwareSuppot(HARDWARE_SUPPORT_COMM_NUM,szBuf))
		{
			PubMsgDlg(NULL, "The terminal doesn't support <AUX>, please change settings", 3, 3);
			return APP_FAIL;
		}
		break;
	case COMMTYPE_GPRS:
		if ((APP_SUCC != PubGetHardwareSuppot(HARDWARE_SUPPORT_WIRELESS, szBuf) )||(0 != memcmp("GPRS", szBuf, 4)))
		{
			PubMsgDlg(NULL, "The terminal doesn't support <GPRS>, please change settings", 3, 3);
			return APP_FAIL;
		}
		break;
	case COMMTYPE_CDMA:
		if ((APP_SUCC != PubGetHardwareSuppot(HARDWARE_SUPPORT_WIRELESS, szBuf)) || (0 != memcmp("CDMA", szBuf, 4)))
		{
			PubMsgDlg(NULL, "The terminal doesn't support <CDMA>, please change settings", 3, 3);
			return APP_FAIL;
		}
		break;
	case COMMTYPE_ETH:
		if (APP_SUCC != PubGetHardwareSuppot(HARDWARE_SUPPORT_ETH,szBuf))
		{
			PubMsgDlg(NULL, "The terminal doesn't support <ETH>, please change settings", 3, 3);
			return APP_FAIL;
		}
		break;
	case COMMTYPE_USB:
		if (APP_SUCC != PubGetHardwareSuppot(HARDWARE_SUPPORT_USB,szBuf))
		{
			PubMsgDlg(NULL, "The terminal doesn't support <USB>, please change settings", 3, 3);
			return APP_FAIL;
		}
		break;
	case COMMTYPE_WIFI:
		if (APP_SUCC != PubGetHardwareSuppot(HARDWARE_SUPPORT_WIFI,szBuf))
		{
			PubMsgDlg(NULL, "The terminal doesn't support <WIFI>, please change settings", 3, 3);
			return APP_FAIL;
		}
		break;
	default:
		PubMsgDlg(NULL, "Invalid communication type", 3, 3);
		return APP_FAIL;
		break;
	}
	return APP_SUCC;
}


/**
* @brief Set communication parameters and initialize communication module
* @param [in] pstCommParam  Communication parameters
* @return
* @li APP_FAIL Success
* @li APP_SUCC Fail
* @author liug
* @date 2012-5-25
*/
int PubCommInit(const STCOMMPARAM *pstCommParam)
{
	int nRet;

	PubDebugSelectly(2, "PubCommInit Start... cCommType:%x", pstCommParam->cCommType);

	if (APP_SUCC != CommCheckIfSupport(pstCommParam->cCommType))
	{
		return APP_FAIL;
	}

	nRet = PubSetCommParam(pstCommParam);
	if (nRet != APP_SUCC)
	{
		return nRet;
	}
	NAPI_KbFlush();
	if(InitComm() != APP_SUCC)
	{
		char szErrMsg[64+1] = {0};
		ProGetErrMsg(GetCommErrorCode(), szErrMsg);
		PubSetErrorCode(GetCommErrorCode(),szErrMsg,GetNapiErrorCode());
		return APP_FAIL;
	}

	PubDebugSelectly(2, "PubCommInit Succ.");
	return APP_SUCC;
}

/**
* @brief Get DHCP local address
* @param [out] pstEthParam
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author liug
* @date 2013-5-7
*/
int PubGetDhcpLocalAddr(STETHPARAM  *pstEthParam)
{
	STETHPARAM stEthParam;

	memset(&stEthParam, 0, sizeof(stEthParam));
	if(CommGetNetAddr(&stEthParam) != APP_SUCC)
	{
		return APP_FAIL;
	}

	strcpy(pstEthParam->szIP, stEthParam.szIP);
	strcpy(pstEthParam->szMask, stEthParam.szMask);
	strcpy(pstEthParam->szGateway, stEthParam.szGateway);
	if (0 != strlen(stEthParam.szDNS))
	{
		strcpy(pstEthParam->szDNS, stEthParam.szDNS);
	}

	PubDebugSelectly(3, "PubGetDhcpLocalAddr Succ.[ip:%s][mask:%s][gw:%s][dns:%s]"
		,pstEthParam->szIP, pstEthParam->szMask, pstEthParam->szGateway, pstEthParam->szDNS);
	return APP_SUCC;
}

/**
* @brief Connect beforehand
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author liug
* @date 2012-5-24
*/
int PubCommPreConnect(void)
{
	int nRet;
	PubDebugSelectly(2, "PubCommPreConnect Start...");//

	if (gstCommParam.stCommParam.cPreDialFlag == 0)
	{
		return APP_SUCC;
	}

	nRet = ProCommConnect(EM_PRECONNECT);/**/
	PubDebugSelectly(2, "PubCommPreConnect:%d",nRet);
	return nRet;
}


/**
* @brief Communication connect
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author liug
* @date 2012-5-25
*/
int PubCommConnect()
{
	PubDebugSelectly(2, "PubCommConnect Start...");//

	ProShowInfo();
	while(1)
	{
		if(ProCommConnect(EM_CONNECT) != APP_SUCC)
		{
			char szErrMsg[64+1] = {0};
			ProGetErrMsg(GetCommErrorCode(), szErrMsg);
			if ((GetCommErrorCode() >= -2116 && GetCommErrorCode() <= -2101)
				|| (GetCommErrorCode() >= -2217 && GetCommErrorCode() <= -2201)
				|| (GetCommErrorCode() >= -2614 && GetCommErrorCode() <= -2601))
			{
				NAPI_ScrPush();
				PubClear2To4();
				PubDispMultLines(0, 2, 1, "%s%d(%d)", szErrMsg, GetCommErrorCode(), GetNapiErrorCode());
				PubDisplayStrInline(0, 4, "[ENTER] to retry");
				PubUpdateWindow();
				if (KEY_ENTER == PubWaitConfirm(30))
				{
					NAPI_ScrPop();
					PubUpdateWindow();
					continue;
				}
			}
			PubSetErrorCode(GetCommErrorCode(),szErrMsg,GetNapiErrorCode());
			ProExitShowInfo();
			return APP_FAIL;
		}
		break;
	}
	PubDebugSelectly(2, "PubCommConnect Succ");
	ProExitShowInfo();
	return APP_SUCC;
}

static int CommRead(int nReadType, char* psData, int nMaxLen, int* pnOutLen)
{
	int nRet;
	if(gstCommParam.stCommParam.nTimeOut != 0)
	{
		ProShowInfo();
	}
	nRet = ProCommRecv(nReadType, nMaxLen, psData,	pnOutLen);
	if(nRet != APP_SUCC)
	{
		char szErrMsg[64+1] = {0};
		ProGetErrMsg(GetCommErrorCode(), szErrMsg);
		PubSetErrorCode(GetCommErrorCode(),szErrMsg,GetNapiErrorCode());
		ProExitShowInfo();
		return nRet;
	}
	ProExitShowInfo();
	return APP_SUCC;
}


/**
* @brief Read data from data channel
* @param [out] psData        Data received
* @param [in] nMaxLen       Max buffer size to receive data
* @param [out] *pnDataLen    Length of data received
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author liug
* @date 2012-5-23
*/
int PubCommRead(char* psData, int nMaxLen, int* pnOutLen)
{
	int nRet;

	PubDebugSelectly(3, "PubCommRead Start...[nMaxLen:%d][timeout:%d]",nMaxLen, gstCommParam.stCommParam.nTimeOut);
	nRet = CommRead(0xFF, psData, nMaxLen, pnOutLen);
	if(nRet != APP_SUCC)
	{
	     return nRet;
	}
	PubDebugSelectly(2, "PubCommRead Succ ,ReadLen is [%d]",*pnOutLen);
	return APP_SUCC;
}

/**
* @brief Write data
* @details Write data directly to the data channel
* @param [in]  psData        Data to be sent
* @param [in]  nDataLen      Data length
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author liug
* @date 2012-5-23
*/
int PubCommWrite(const char *psData,int nDataLen)
{
	if(ProCommSend(psData, nDataLen) != APP_SUCC)
	{
		char szErrMsg[64+1] = {0};
		ProGetErrMsg(GetCommErrorCode(), szErrMsg);
		PubSetErrorCode(GetCommErrorCode(),szErrMsg,GetNapiErrorCode());
		return APP_FAIL;
	}
	return APP_SUCC;
}

/**
* @brief In accordance with the modes of communication to write data
* @param [in] psData        Data to be sent
* @param [in] nDataLen      Data length
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author liug
* @date 2012-5-23
*/
int PubCommSend(const char *psData,int nDataLen)
{
	int nRealLen = nDataLen;
	char szSendData[MAX_SEND_SIZE] = {0};
	int nHaveSendLen = 0, nRemainLen = 0;
	int nRet = 0;
	int nConnectIndex = 0;

	PubDebugSelectly(2, "PubCommSend nDataLen is [%d]", nDataLen);
	memcpy(szSendData, psData, nDataLen);
	switch (gstCommParam.stCommParam.cCommType)
	{
	case COMMTYPE_PORT_ASYN:
	case COMMTYPE_USB_ASYN:
	case COMMTYPE_ASYNDIAL_ASYN:
		InsertAsyn(szSendData, (uint *)&nRealLen);
		break;
	case COMMTYPE_PORT_ASYN_TPDU:
	case COMMTYPE_USB_ASYN_TPDU:
		AddTpdu(szSendData, (uint *)&nRealLen, gstCommParam.stCommParam.ConnInfo.stPortParam.sTPDU);
		InsertAsyn(szSendData, (uint *)&nRealLen);
		break;
	case COMMTYPE_ASYNDIAL_ASYN_TPDU:
        PubGetConnectIndex(&nConnectIndex);
	    PubDebugSelectly(1, "nConnectIndex [%d]", nConnectIndex);
		AddTpdu(szSendData, (uint *)&nRealLen, gstCommParam.stCommParam.ConnInfo.stDialParam.lsTPDU[nConnectIndex]);
		InsertAsyn(szSendData, (uint *)&nRealLen);
		break;
	case COMMTYPE_PORT_ASYN1:
	case COMMTYPE_USB_ASYN1:
	case COMMTYPE_ASYNDIAL_ASYN1:
	case COMMTYPE_GPRS_ASYN1:
	case COMMTYPE_CDMA_ASYN1:
	case COMMTYPE_ETH_ASYN1:
	case COMMTYPE_WIFI_ASYN1:
	case COMMTYPE_MODEM_ASYN1:
		InsertAsyn1(szSendData, (uint *)&nRealLen);
		break;
	case COMMTYPE_PORT_ASYN1_TPDU:
	case COMMTYPE_USB_ASYN1_TPDU:
		AddTpdu(szSendData, (uint *)&nRealLen, gstCommParam.stCommParam.ConnInfo.stPortParam.sTPDU);
		InsertAsyn1(szSendData, (uint *)&nRealLen);
		break;
	case COMMTYPE_ASYNDIAL_ASYN1_TPDU:
        PubGetConnectIndex(&nConnectIndex);
	    PubDebugSelectly(1, "nConnectIndex [%d]", nConnectIndex);
		AddTpdu(szSendData, (uint *)&nRealLen, gstCommParam.stCommParam.ConnInfo.stDialParam.lsTPDU[nConnectIndex]);
		InsertAsyn1(szSendData, (uint *)&nRealLen);
		break;
	case COMMTYPE_GPRS_ASYN1_TPDU:
		AddTpdu(szSendData, (uint *)&nRealLen, gstCommParam.stCommParam.ConnInfo.stGprsParam.sTPDU);
		InsertAsyn1(szSendData, (uint *)&nRealLen);
		break;
	case COMMTYPE_CDMA_ASYN1_TPDU:
		AddTpdu(szSendData, (uint *)&nRealLen, gstCommParam.stCommParam.ConnInfo.stCdmaParam.sTPDU);
		InsertAsyn1(szSendData, (uint *)&nRealLen);
		break;
	case COMMTYPE_ETH_ASYN1_TPDU:
		AddTpdu(szSendData, (uint *)&nRealLen, gstCommParam.stCommParam.ConnInfo.stEthParam.sTPDU);
		InsertAsyn1(szSendData, (uint *)&nRealLen);
		break;
	case COMMTYPE_WIFI_ASYN1_TPDU:
		AddTpdu(szSendData, (uint *)&nRealLen, gstCommParam.stCommParam.ConnInfo.stWifiParam.sTPDU);
		InsertAsyn1(szSendData, (uint *)&nRealLen);
		break;
	case COMMTYPE_MODEM_ASYN1_TPDU:
		AddTpdu(szSendData, (uint *)&nRealLen, gstCommParam.stCommParam.ConnInfo.stModemParam.sTPDU);
		InsertAsyn1(szSendData, (uint *)&nRealLen);
		break;
	case COMMTYPE_SYNDIAL_TPDU:
        PubGetConnectIndex(&nConnectIndex);
	    PubDebugSelectly(1, "nConnectIndex [%d]", nConnectIndex);
		AddTpdu(szSendData, (uint *)&nRealLen, gstCommParam.stCommParam.ConnInfo.stDialParam.lsTPDU[nConnectIndex]);
		break;
	case COMMTYPE_SYNDIAL_HEADLEN:
	case COMMTYPE_GPRS_HEADLEN:
	case COMMTYPE_CDMA_HEADLEN:
	case COMMTYPE_ETH_HEADLEN:
	case COMMTYPE_WIFI_HEADLEN:
	case COMMTYPE_MODEM_HEADLEN:
		memcpy(szSendData + COMM_TCPIP_HEAD_LEN, psData, nRealLen);
		PubIntToC2((unsigned char *)szSendData, nRealLen);
		nRealLen += COMM_TCPIP_HEAD_LEN;
		break;
	case COMMTYPE_SYNDIAL_TPDU_HEADLEN:
		memcpy(szSendData + COMM_TCPIP_HEAD_LEN, psData, nRealLen);
		AddTpdu(szSendData + COMM_TCPIP_HEAD_LEN, (uint *)&nRealLen, gstCommParam.stCommParam.ConnInfo.stDialParam.lsTPDU[nConnectIndex]);
		PubIntToC2((unsigned char *)szSendData, nRealLen);
		nRealLen += COMM_TCPIP_HEAD_LEN;
		break;
	case COMMTYPE_GPRS_TPDU_HEADLEN:
		memcpy(szSendData + COMM_TCPIP_HEAD_LEN, psData, nRealLen);
		AddTpdu(szSendData + COMM_TCPIP_HEAD_LEN, (uint *)&nRealLen, gstCommParam.stCommParam.ConnInfo.stGprsParam.sTPDU);
		PubIntToC2((unsigned char *)szSendData, nRealLen);
		nRealLen += COMM_TCPIP_HEAD_LEN;
		break;
	case COMMTYPE_CDMA_TPDU_HEADLEN:
		memcpy(szSendData + COMM_TCPIP_HEAD_LEN, psData, nRealLen);
		AddTpdu(szSendData + COMM_TCPIP_HEAD_LEN, (uint *)&nRealLen, gstCommParam.stCommParam.ConnInfo.stCdmaParam.sTPDU);
		PubIntToC2((unsigned char *)szSendData, nRealLen);
		nRealLen += COMM_TCPIP_HEAD_LEN;
		break;
	case COMMTYPE_ETH_TPDU_HEADLEN:
		memcpy(szSendData + COMM_TCPIP_HEAD_LEN, psData, nRealLen);
		AddTpdu(szSendData + COMM_TCPIP_HEAD_LEN, (uint *)&nRealLen, gstCommParam.stCommParam.ConnInfo.stEthParam.sTPDU);
		PubIntToC2((unsigned char *)szSendData, nRealLen);
		nRealLen += COMM_TCPIP_HEAD_LEN;
		break;
	case COMMTYPE_WIFI_TPDU_HEADLEN:
		memcpy(szSendData + COMM_TCPIP_HEAD_LEN, psData, nRealLen);
		AddTpdu(szSendData + COMM_TCPIP_HEAD_LEN, (uint *)&nRealLen, gstCommParam.stCommParam.ConnInfo.stWifiParam.sTPDU);
		PubIntToC2((unsigned char *)szSendData, nRealLen);
		nRealLen += COMM_TCPIP_HEAD_LEN;
		break;
	case COMMTYPE_MODEM_TPDU_HEADLEN:
		memcpy(szSendData + COMM_TCPIP_HEAD_LEN, psData, nRealLen);
		AddTpdu(szSendData + COMM_TCPIP_HEAD_LEN, (uint *)&nRealLen, gstCommParam.stCommParam.ConnInfo.stModemParam.sTPDU);
		PubIntToC2((unsigned char *)szSendData, nRealLen);
		nRealLen += COMM_TCPIP_HEAD_LEN;
		break;
	case COMMTYPE_SYNDIAL:
	case COMMTYPE_ASYNDIAL:
	case COMMTYPE_PORT:
	case COMMTYPE_USB:
	case COMMTYPE_GPRS:
	case COMMTYPE_CDMA:
	case COMMTYPE_ETH:
	case COMMTYPE_WIFI:
	case COMMTYPE_MODEM:
	default:
		break;
	}

	nRemainLen = nRealLen;

	while(1)
	{
		if (nRemainLen <= MAX_SEND_SIZE)
		{
			nRet = PubCommWrite(szSendData + nHaveSendLen, nRemainLen);
			if (nRet != APP_SUCC)
			{
				return nRet;
			}
			return APP_SUCC;
		}
		nRet = PubCommWrite(szSendData + nHaveSendLen, MAX_SEND_SIZE);
		if (nRet != APP_SUCC)
		{
			return nRet;
		}
		nHaveSendLen += MAX_SEND_SIZE;
		nRemainLen = nRealLen - nHaveSendLen;
		PubSysMsDelay(100);
	}
	return APP_SUCC;
}

/**
* @brief Read data
* @details  In accordance with the modes of communication to read data
* @param [out] psData        Data received
* @param [out] *pnDataLen    Length of data received
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author liug
* @date 2012-5-23
*/
int PubCommRecv(char *psData,int *pnDataLen)
{
	int nRecvLen = 0;
	int nRet = 0;
   	int nRecvSucc = 0;
	char sRecvBuf[MAX_RECV_SIZE+1] = {0};	/**<*/
	int nMaxLen = MAX_RECV_SIZE;

	PubDebugSelectly(3, "PubCommRecv Start...[nScheme: %x][timeout:%d]", gstCommParam.stCommParam.cCommType,  gstCommParam.stCommParam.nTimeOut);
	//20131217 sh  4k
	if ((gstCommParam.stCommParam.cCommType & 0xF0) == COMMTYPE_PORT || (gstCommParam.stCommParam.cCommType & 0xF0) == COMMTYPE_USB)
	{
		nMaxLen = 4*1024;
	}
	nRet = CommRead(gstCommParam.stCommParam.cCommType, sRecvBuf, nMaxLen, &nRecvLen);
	if (nRet != APP_SUCC)
	{
		*pnDataLen = 0;
		return nRet;
	}

	//tpdu
	switch (gstCommParam.stCommParam.cCommType)
	{
	case COMMTYPE_SYNDIAL_TPDU:
	case COMMTYPE_SYNDIAL_TPDU_HEADLEN:
	case COMMTYPE_GPRS_TPDU_HEADLEN:
	case COMMTYPE_CDMA_TPDU_HEADLEN:
	case COMMTYPE_ETH_TPDU_HEADLEN:
	case COMMTYPE_WIFI_TPDU_HEADLEN:
	case COMMTYPE_MODEM_TPDU_HEADLEN:
		DelTpdu(sRecvBuf, (uint *)&nRecvLen);
		if (nRet != APP_SUCC)
		{
			break;
		}
        nRecvSucc = 1;
		break;
	case COMMTYPE_PORT_ASYN:
	case COMMTYPE_USB_ASYN:
	case COMMTYPE_ASYNDIAL_ASYN:
		nRet = DeleteAsyn(sRecvBuf, (uint *)&nRecvLen);
		if (nRet != APP_SUCC)
		{
			break;
		}
        nRecvSucc = 1;
		break;
	case COMMTYPE_PORT_ASYN_TPDU:
	case COMMTYPE_USB_ASYN_TPDU:
	case COMMTYPE_ASYNDIAL_ASYN_TPDU:
		nRet = DeleteAsyn(sRecvBuf, (uint *)&nRecvLen);
		if (nRet != APP_SUCC)
		{
			break;
		}
		nRet = DelTpdu(sRecvBuf, (uint *)&nRecvLen);
		if (nRet != APP_SUCC)
		{
			break;
		}
        	nRecvSucc = 1;
		break;
	case COMMTYPE_PORT_ASYN1:
	case COMMTYPE_USB_ASYN1:
	case COMMTYPE_ASYNDIAL_ASYN1:
	case COMMTYPE_GPRS_ASYN1:
	case COMMTYPE_CDMA_ASYN1:
	case COMMTYPE_ETH_ASYN1:
	case COMMTYPE_WIFI_ASYN1:
	case COMMTYPE_MODEM_ASYN1:
		nRet = DeleteAsyn1(sRecvBuf, (uint *)&nRecvLen);
		if (nRet != APP_SUCC)
		{
			break;
		}
        	nRecvSucc = 1;
		break;
	case COMMTYPE_PORT_ASYN1_TPDU:
	case COMMTYPE_USB_ASYN1_TPDU:
	case COMMTYPE_ASYNDIAL_ASYN1_TPDU:
	case COMMTYPE_GPRS_ASYN1_TPDU:
	case COMMTYPE_CDMA_ASYN1_TPDU:
	case COMMTYPE_ETH_ASYN1_TPDU:
	case COMMTYPE_WIFI_ASYN1_TPDU:
	case COMMTYPE_MODEM_ASYN1_TPDU:
		nRet = DeleteAsyn1(sRecvBuf, (uint *)&nRecvLen);
		if (nRet != APP_SUCC)
		{
			break;
		}
		nRet = DelTpdu(sRecvBuf, (uint *)&nRecvLen);
		if (nRet != APP_SUCC)
		{
			break;
		}
        nRecvSucc = 1;
		break;
	default:
        nRecvSucc = 1; /*COMMTYPE_GPRS*/
		break;
	}
	gcIsTimerOn = FALSE;
	if(nRecvSucc)
	{
		PubDebugSelectly(2, "PubCommRecv Succ: nRecvLen is [%d]",nRecvLen);
		memcpy(psData, sRecvBuf, nRecvLen);
		*pnDataLen = nRecvLen;
		return APP_SUCC;
	}
	else
	{
		PubDebugSelectly(2, "PubCommRecv Fail");
		return APP_FAIL;
	}
}

/**
* @brief Clear communication buffer
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author liug
* @date 2012-5-24
*/
int PubCommClear()
{
	switch(gstCommParam.stCommParam.cCommType & 0xF0)
	{/*<USB*/
	case COMMTYPE_ASYNDIAL:
	case COMMTYPE_PORT:
	case COMMTYPE_USB:
		break;
	default:
		PubDebugSelectly(2, "PubCommClear Not Support:");
		return APP_SUCC;
		break;
	}
	PubDebugSelectly(2, "PubCommClear Start:");

	if(CommClearBuf() != APP_SUCC)
	{
		return APP_FAIL;
	}
	PubDebugSelectly(2, "PubCommClear Succ.");
	return APP_SUCC;
}

/**
* @brief Hang up communication link
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author liug
* @date 2012-5-24
*/
int PubCommHangUp()
{
	ProExitShowInfo();
	PubDebugSelectly(2, "PubCommHangUp Start:");
	CommHungUp(HANGUP_SOCKET);
	PubDebugSelectly(2, "PubCommHangUp Succ.");
	return APP_SUCC;
}

/**
* @brief Close TCP link immediately and clear buffer
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2016-01-14
*/
int PubCommHangUpReset()
{
	ProExitShowInfo();
	PubDebugSelectly(2, "PubCommHangUpReset Start:");
	CommHungUp(HANGUP_RESET);
	PubDebugSelectly(2, "PubCommHangUpReset Succ.");
	return APP_SUCC;
}


/**
* @brief Close communication
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author liug
* @date 2012-5-24
*/
int PubCommClose()
{
	CommHungUp(HANGUP_PPP);
	return APP_SUCC;
}

/**
* @brief Set communication parameter
* @param [in] pstCommParam
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author liug
* @date 2012-5-24
*/
int PubSetCommParam(const STCOMMPARAM* pstCommParam)
{
	switch(pstCommParam->cCommType & 0xF0)
	{
	case COMMTYPE_ASYNDIAL:
		//*
		if (gstCommParam.stCommParam.ConnInfo.stDialParam.nCountryid != pstCommParam->ConnInfo.stDialParam.nCountryid)
		{
			if (gstCommParam.cIsConnect == TRUE)
			{
				gstCommParam.pHangUp(HANGUP_PPP);
				gstCommParam.cIsConnect = FALSE;
			}
			gstCommParam.cIsInitComm = FALSE;
		}
		else
		{
			//
			if (memcmp(&gstCommParam.stCommParam.ConnInfo.stDialParam, &pstCommParam->ConnInfo.stDialParam,
				sizeof(STDIALPARAM)) != 0)
			{
				AsynDialSetParam(&pstCommParam->ConnInfo.stDialParam);
			}
		}
		//
		if (memcmp(&gstCommParam.stCommParam.ConnInfo.stDialParam, &pstCommParam->ConnInfo.stDialParam,
			sizeof(STDIALPARAM)) != 0)
		{
			AsynResetIndex();
		}
		break;
	case COMMTYPE_SYNDIAL:
		//*
		if (gstCommParam.stCommParam.ConnInfo.stDialParam.nCountryid != pstCommParam->ConnInfo.stDialParam.nCountryid
			|| gstCommParam.stCommParam.ConnInfo.stDialParam.nBps != pstCommParam->ConnInfo.stDialParam.nBps)
		{
			if (gstCommParam.cIsConnect == TRUE)
			{
				gstCommParam.pHangUp(HANGUP_PPP);
				gstCommParam.cIsConnect = FALSE;
			}
			gstCommParam.cIsInitComm = FALSE;
		}
		else
		{
			//
			if (memcmp(&gstCommParam.stCommParam.ConnInfo.stDialParam, &pstCommParam->ConnInfo.stDialParam,
				sizeof(STDIALPARAM)) != 0)
			{
                SynDialSetParam((STDIALPARAM *)&pstCommParam->ConnInfo.stDialParam);
			}
		}
		//
		if (memcmp(&gstCommParam.stCommParam.ConnInfo.stDialParam, &pstCommParam->ConnInfo.stDialParam,
			sizeof(STDIALPARAM)) != 0)
		{
			SynResetIndex();
		}
		break;
	case COMMTYPE_PORT:
	case COMMTYPE_USB:
		if (gstCommParam.stCommParam.ConnInfo.stPortParam.nBaudRate != pstCommParam->ConnInfo.stPortParam.nBaudRate
			|| gstCommParam.stCommParam.ConnInfo.stPortParam.nDataBits != pstCommParam->ConnInfo.stPortParam.nDataBits
			|| gstCommParam.stCommParam.ConnInfo.stPortParam.nParity != pstCommParam->ConnInfo.stPortParam.nParity
			|| gstCommParam.stCommParam.ConnInfo.stPortParam.nStopBits != pstCommParam->ConnInfo.stPortParam.nStopBits)
		{
			gstCommParam.cIsInitComm = FALSE;
		}
		break;
	case COMMTYPE_GPRS:
		if (memcmp(gstCommParam.stCommParam.ConnInfo.stGprsParam.szGprsApn,	pstCommParam->ConnInfo.stGprsParam.szGprsApn,40)!=0
			|| memcmp(gstCommParam.stCommParam.ConnInfo.stGprsParam.szModemDialNo, pstCommParam->ConnInfo.stGprsParam.szModemDialNo,21)!=0)
		{
			if (gstCommParam.cIsConnect == TRUE)
			{
				gstCommParam.pHangUp(HANGUP_PPP);
				gstCommParam.cIsConnect = FALSE;
			}
			gstCommParam.cIsInitComm = FALSE;
		}

		//
		if (memcmp(&gstCommParam.stCommParam.ConnInfo.stGprsParam, &pstCommParam->ConnInfo.stGprsParam, sizeof(STGPRSPARAM)) != 0
			||memcmp(&gstCommParam.stCommParam.stServerAddress, &pstCommParam->stServerAddress, sizeof(STSERVERADDRESS)) != 0)
		{
			WirelessResetIndex();
		}

		//ssl
		if (gstCommParam.stCommParam.cSslFlag != pstCommParam->cSslFlag)
		{
			WirelessUpdateSslFlag(pstCommParam->cSslFlag);
		}
		break;
	case COMMTYPE_CDMA:
		if ((memcmp(gstCommParam.stCommParam.ConnInfo.stCdmaParam.szNetUsername,pstCommParam->ConnInfo.stCdmaParam.szNetUsername,40)!=0)
			||(memcmp(gstCommParam.stCommParam.ConnInfo.stCdmaParam.szNetPassword,pstCommParam->ConnInfo.stCdmaParam.szNetPassword,20)!=0)
			|| memcmp(gstCommParam.stCommParam.ConnInfo.stCdmaParam.szModemDialNo,pstCommParam->ConnInfo.stCdmaParam.szModemDialNo,21)!=0)
		{
			if (gstCommParam.cIsConnect == TRUE)
			{
				gstCommParam.pHangUp(HANGUP_PPP);
				gstCommParam.cIsConnect = FALSE;
			}
			gstCommParam.cIsInitComm = FALSE;
		}
		//
		if (memcmp(&gstCommParam.stCommParam.ConnInfo.stCdmaParam, &pstCommParam->ConnInfo.stCdmaParam, sizeof(STCDMAPARAM)) != 0
			||memcmp(&gstCommParam.stCommParam.stServerAddress, &pstCommParam->stServerAddress, sizeof(STSERVERADDRESS)) != 0)
		{
			WirelessResetIndex();
		}

		//ssl
		if (gstCommParam.stCommParam.cSslFlag != pstCommParam->cSslFlag)
		{
			WirelessUpdateSslFlag(pstCommParam->cSslFlag);
		}
		break;
	case COMMTYPE_ETH:
		if (memcmp(&(gstCommParam.stCommParam.ConnInfo.stEthParam),&(pstCommParam->ConnInfo.stEthParam),sizeof(STETHPARAM))!= 0)
		{
			if (gstCommParam.cIsConnect == TRUE)
			{
				gstCommParam.pHangUp(HANGUP_PPP);
				gstCommParam.cIsConnect = FALSE;
			}
			gstCommParam.cIsInitComm = FALSE;
		}
		//
		if (memcmp(&gstCommParam.stCommParam.ConnInfo.stEthParam, &pstCommParam->ConnInfo.stEthParam, sizeof(STETHPARAM)) != 0
			||memcmp(&gstCommParam.stCommParam.stServerAddress, &pstCommParam->stServerAddress, sizeof(STSERVERADDRESS)) != 0)
		{
			EthResetIndex();
		}

		//ssl
		if (gstCommParam.stCommParam.cSslFlag != pstCommParam->cSslFlag)
		{
			EthUpdateSslFlag(pstCommParam->cSslFlag);
		}
		break;
	case COMMTYPE_WIFI://
		//
		if (memcmp(&gstCommParam.stCommParam.ConnInfo.stWifiParam, &pstCommParam->ConnInfo.stWifiParam, sizeof(STWIFIPARAM)) != 0)
		{
			WifiSetParam((STWIFIPARAM *)&pstCommParam->ConnInfo.stWifiParam);
		}

		//
		if (memcmp(&gstCommParam.stCommParam.ConnInfo.stWifiParam, &pstCommParam->ConnInfo.stWifiParam,sizeof(STWIFIPARAM)) != 0
			||memcmp(&gstCommParam.stCommParam.stServerAddress, &pstCommParam->stServerAddress, sizeof(STSERVERADDRESS)) != 0)
		{
			WifiResetIndex();
		}

		//ssl
		if (gstCommParam.stCommParam.cSslFlag != pstCommParam->cSslFlag)
		{
			WifiUpdateSslFlag(pstCommParam->cSslFlag);
		}
		break;
	default:
		return APP_FAIL;
	}
	memcpy(&gstCommParam.stCommParam, pstCommParam, sizeof(STCOMMPARAM));

	if (pstCommParam->ShowFunc != NULL)
	{
        gstCommParam.stCommParam.ShowFunc = pstCommParam->ShowFunc;
	}
	else
	{
		gstCommParam.stCommParam.ShowFunc = Showinfo;
	}
	return APP_SUCC;
}


/**
* @brief Get communication parameter
* @param  [out] pstCommParam
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author sunh
* @date 2013-9-15
*/
int PubGetCommParam(STCOMMPARAM* pstCommParam)
{
	if (pstCommParam == NULL)
	{
		return APP_FAIL;
	}
	memcpy(pstCommParam, &gstCommParam.stCommParam, sizeof(STCOMMPARAM));
	PubDebugSelectly(2, "PubGetCommParam succ .");
	return APP_SUCC;
}

/**
* @brief Get connection index
* @param [out] pnIndex
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author liug
* @date 2012-12-20
*/
int PubGetConnectIndex(int *pnIndex)
{
	GetCommConnectIndex(pnIndex);
	return APP_SUCC;
}


/**
* @brief Get the version of communication module
* @param [out] pszVer  Version string
* @return
* @li void
*/
void PubGetCommVerion(char *pszVer)
{
	if (pszVer == NULL)
	{
		return;
	}
	strcpy(pszVer,COMMLIBVER);
}

/**
* @brief Configure ssl and use it before connection. Use default configuration if there is no setting
* @param [in] pstSslMode ssl Related parameters
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author sunh
* @date 2014-1-1
*/
int PubSslSetMode(STSSLMODE *pstSslMode)
{
	char szDir[40] = {0};
	STSSLMODE stSslMode;

	if (pstSslMode == NULL)
		return APP_FAIL;

	memset(&stSslMode, 0, sizeof(STSSLMODE));

	getcwd(szDir, sizeof(szDir));
	stSslMode.nType = pstSslMode->nType;
	stSslMode.nFileFormat = pstSslMode->nFileFormat;
	stSslMode.nAuthOpt = pstSslMode->nAuthOpt;
	if (strlen(pstSslMode->szPwd) != 0)
		strcpy(stSslMode.szPwd, pstSslMode->szPwd);
	if (strlen(pstSslMode->szServerCert) != 0)
		sprintf(stSslMode.szServerCert, "%s/%s", szDir, pstSslMode->szServerCert);
	if (strlen(pstSslMode->szClientCert) != 0)
		sprintf(stSslMode.szClientCert, "%s/%s", szDir, pstSslMode->szClientCert);
	if (strlen(pstSslMode->szClientPrivateKey) != 0)
		sprintf(stSslMode.szClientPrivateKey, "%s/%s", szDir, pstSslMode->szClientPrivateKey);
	SetSslMode(&stSslMode);
	PubDebugSelectly(2, "PubSetSSlcertificates Succ...");
	return APP_SUCC;
}

/**
* @brief Get SSL server certificate information
* @param  [out] pstSslCertMsg Certificate information
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author sunh
* @date 2014-1-1
*/
int PubSslGetCertMsg(STSSLCERTMSG* pstSslCertMsg)
{
	STSSLCERTMSG stSslCertMsg;

	if (pstSslCertMsg == NULL)
		return APP_FAIL;

	memset(&stSslCertMsg, 0, sizeof(stSslCertMsg));
	GetSslCertMsg(&stSslCertMsg);
	memcpy(pstSslCertMsg, &stSslCertMsg, sizeof(STSSLCERTMSG));
	PubDebugSelectly(2, "PubSslGetCertMsg Succ...");
	return APP_SUCC;
}

/**
* @brief Scan WIFI, select SSID, and get the encryption mode
* @param  [in] pszTitle ssid List title, show one more line if set NULL
* @param  [in] nTimeout Timeout
* @param  [out] pszOutSsid Wifi hotspot name
* @param  [out] pnWifiMode Wifi encryption mode
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author sunh
* @date 2014-1-10
*/
int PubCommScanWifi(const char *pszTitle, char *pszOutSsid, int *pnWifiMode, int nTimeout)
{
	int i = 0;
	int nMaxSsldNum = 0;
	int nKey = 0;
	int nPage = 0;
	int nMaxPage = 0;
	int nMaxSelNum = 0;
	int nMaxLcdLine = 0;
	char szDispItem[100] = {0};

	int nSelectItem, nRet;
	char *pszItems1[WIFI_AP_NUM_MAX+1] = {
	};

	ST_WIFI_AP_LIST stAp_list;
	ST_WIFI_AP_INFO_T lstAp_info[WIFI_AP_NUM_MAX];

	if (pszTitle == NULL || pszOutSsid == NULL || pnWifiMode == NULL)
	{
		return APP_FAIL;
	}

	if (APP_SUCC != PubGetHardwareSuppot(HARDWARE_SUPPORT_WIFI, NULL))
	{
		PubMsgDlg(pszTitle, "The terminal doesn't support <WIFI>", 1, 3);
		return APP_FAIL;
	}

	PubDebugSelectly(1, "PubCommScanWifi start...");
	memset(&stAp_list, 0, sizeof(stAp_list));
	memset(lstAp_info, 0, sizeof(lstAp_info));
	stAp_list.pstList = lstAp_info;

	if(WifiScan(&stAp_list) != APP_SUCC)
	{
		return APP_FAIL;
	}

	nMaxSsldNum = stAp_list.num;
	PubDebugSelectly(1, "nMaxSsldNum [%d]", nMaxSsldNum);

	if (PubGetKbAttr() == KB_VIRTUAL)
	{
			for (i = 0; i < nMaxSsldNum; i++) {
			pszItems1[i] = lstAp_info[i].szSsid;
			PubDebugSelectly(1, "ssid(%d): %s emAuthType = %d", i, pszItems1[i], lstAp_info[i].emAuthType);
		}
		nRet = PubShowMenuItems((char *)pszTitle, pszItems1, nMaxSsldNum, &nSelectItem, NULL, nTimeout);
		if (nRet == APP_QUIT || nRet == APP_TIMEOUT) {
			return APP_QUIT;
		}
		strcpy(pszOutSsid, lstAp_info[nSelectItem-1].szSsid);
		*pnWifiMode = lstAp_info[nSelectItem-1].emAuthType;
		PubDebugSelectly(1, "nMaxSsldNum [%d]", nMaxSsldNum);
		PubDebugSelectly(1, "ssid: %s  %d", pszOutSsid, *pnWifiMode);
		return APP_SUCC;
	}

	PubGetDispView(&nMaxLcdLine, NULL);

	if (pszTitle !=NULL)
	{
		nMaxSelNum = nMaxLcdLine-1;
	}
	else
	{
		nMaxSelNum = nMaxLcdLine;
	}
	if (nMaxSelNum > 9 )
	{
		nMaxSelNum = 9;  /* range is KEY_1 --- KEY_9 */
	}

	nMaxPage = (nMaxSsldNum +(nMaxSelNum - 1)) / nMaxSelNum;
	nPage = 1;

	while(1)
	{
		PubClearAll();
		if (pszTitle != NULL)
		{
			PubDisplayTitle((char *)pszTitle);
		}
		for (i = 0; i < nMaxSelNum && nMaxSsldNum; i++)
		{
			memset(szDispItem, 0, sizeof(szDispItem));
			sprintf(szDispItem, "%d.%s", i+1, lstAp_info[(nPage-1)*nMaxSelNum+i].szSsid);
			if (pszTitle != NULL)
			{
				PubDisplayStrInline(1, i+2, szDispItem);
			}
			else
			{
				PubDisplayStrInline(1, i+1, szDispItem);
			}

			if (((nPage-1)*nMaxSelNum + i + 1) == nMaxSsldNum)
			{
				i++;
				break;
			}
		}
		PubUpdateWindow();
		nKey = PubGetKeyCode(nTimeout);
		switch (nKey)
		{
		case 0:
			return APP_TIMEOUT;
		case KEY_ESC:
			return APP_QUIT;
			continue;
		case KEY_LEFT:
		case K_DOT:     /**< used for forth platform */
			if (nPage > 1)
				nPage--;
			continue;
		case KEY_RIGHT:
		case K_ZMK:     /**< used for forth platform */
			if (nPage < nMaxPage)
				nPage++;
			continue;
			break;
		case KEY_0 ... KEY_9:
			if (((nKey-'0') <= i) && ((nKey-'0') >= 1))
			{
				strcpy(pszOutSsid, lstAp_info[(nPage-1)*nMaxSelNum+(nKey-'0'-1)].szSsid);
				*pnWifiMode = lstAp_info[(nPage-1)*nMaxSelNum+(nKey-'0'-1)].emAuthType;
				PubDebugSelectly(1, "PubCommScanWifi succ...[ssid: %s][mode: %d]", pszOutSsid, *pnWifiMode);
				return APP_SUCC;
			}
			continue;
			break;
		default:
			break;
		}
	}

	return APP_SUCC;
}

/**
* @brief 0x02 length data lrc(length+data) 0x03
* @param in/out psBuf
* @param in/out punLen
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
static int InsertAsyn(char *psBuf, uint *punLen)
{
	int nLen = 2;
	char sTemp[MAX_SEND_SIZE];

	if (NULL == psBuf || NULL == punLen)
	{
		return APP_FAIL;
	}

	memcpy(sTemp, psBuf, *punLen);
	psBuf[0] = STX;
	PubIntToBcd(psBuf + 1, &nLen, *punLen);
	memcpy(psBuf + 3, sTemp, *punLen);
	PubCalcLRC(psBuf + 1, *punLen + 2, psBuf + *punLen + 3);
	psBuf[*punLen + 4] = ETX;
	*punLen += 5;
	return APP_SUCC;
}

/**
* @brief 0x02 length data 0x03 lrc(length+data+0x03)
* @param in/out psBuf
* @param in/out punLen
* @li APP_SUCC
* @li APP_FAIL
*/
static int InsertAsyn1(char *psBuf, uint *punLen)
{
	int nLen = 2;
	char sTemp[MAX_SEND_SIZE];

	if (NULL == psBuf || NULL == punLen)
	{
		return APP_FAIL;
	}

	memcpy(sTemp, psBuf, *punLen);
	psBuf[0] = STX;
	PubIntToBcd(psBuf + 1, &nLen, *punLen);
	memcpy(psBuf + 3, sTemp, *punLen);
	psBuf[*punLen + 3] = ETX;
	PubCalcLRC(psBuf + 1, *punLen + 3, psBuf + *punLen + 4);
	*punLen += 5;
	return APP_SUCC;
}
/**
* @brief Add tpdu
* @param in/out psBuf
* @param in/out punLen
* @li APP_SUCC
* @li APP_FAIL
*/
static int AddTpdu(char *psBuf, uint *punLen, char *psTpdu)
{
	char sTemp[MAX_SEND_SIZE];

	if (NULL == psBuf ||NULL == punLen)
	{
		return APP_FAIL;
	}
	memcpy(sTemp, psBuf, *punLen);
	memcpy(psBuf, psTpdu, 5);
	memcpy(psBuf + 5, sTemp, *punLen);
	*punLen += 5;

	return APP_SUCC;
}

/**
* @brief Delete Asyn format data(0x02 lrc 0x03 length)
* @param in/out psBuf
* @param in/out punLen
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
static int DeleteAsyn(char *psBuf, uint *punLen)
{
	int nLen = 0;
	char cLRC = 0;
	char sTemp[MAX_SEND_SIZE];

	if (NULL == psBuf || NULL == punLen)
	{
		return APP_FAIL;
	}
	if (psBuf[0] != STX)
	{
		return APP_FAIL;
	}
	if (*punLen < 5)
	{
		return APP_FAIL;
	}
	if (PubBcdToInt(psBuf + 1, &nLen) != APP_SUCC)
	{
		return APP_FAIL;
	}
	if (*punLen != nLen +5)
	{
		return APP_FAIL;
	}
	if (PubCalcLRC(psBuf + 1, nLen + 2, &cLRC) != APP_SUCC)
	{
		return APP_FAIL;
	}
	if (*(psBuf + *punLen -2) != cLRC)
	{
		return APP_FAIL;
	}
	memcpy(sTemp, psBuf + 3, nLen);
	memcpy(psBuf, sTemp, nLen);
	*punLen = nLen;
	return APP_SUCC;
}

/**
* @brief Delete Asyn format data(0x02 lrc 0x03 length)
* @param in/out psBuf
* @param in/out punLen
* @li APP_SUCC
* @li APP_FAIL
*/
static int DeleteAsyn1(char *psBuf, uint *punLen)
{
	int nLen = 0;
	char cLRC = 0;
	char sTemp[MAX_SEND_SIZE];

	if (NULL == psBuf || NULL == punLen)
	{
		return APP_FAIL;
	}
	if (psBuf[0] != STX)
	{
		return APP_FAIL;
	}
	if (*punLen < 5)
	{
		return APP_FAIL;
	}
	if (PubBcdToInt(psBuf + 1, &nLen) != APP_SUCC)
	{
		return APP_FAIL;
	}
	if (*punLen != nLen +5)
	{
		return APP_FAIL;
	}
	if (PubCalcLRC(psBuf + 1, nLen + 3, &cLRC) != APP_SUCC)
	{
		return APP_FAIL;
	}
	if (*(psBuf + *punLen -1) != cLRC)
	{
		return APP_FAIL;
	}
	memcpy(sTemp, psBuf + 3, nLen);
	memcpy(psBuf, sTemp, nLen);
	*punLen = nLen;
	return APP_SUCC;
}

/**
* @brief Delete tpdu
* @param in/out psBuf
* @param in/out punLen
* @li APP_SUCC
* @li APP_FAIL
*/
static int DelTpdu(char *psBuf, uint *punLen)
{
	char sTemp[MAX_SEND_SIZE];

	if (NULL == psBuf ||NULL == punLen)
	{
		return APP_FAIL;
	}
	if (*punLen < 5)
	{
		return APP_FAIL;
	}
	*punLen -= 5;
	memcpy(sTemp, psBuf + 5, *punLen);
	memcpy(psBuf, sTemp, *punLen);
	return APP_SUCC;
}


static void Showinfo()
{

	PubDebugSelectly(1, "Showinfo start...[nType: %d][row: %d][column: %d]", gstShowInfoxy.nType, gstShowInfoxy.nRow, gstShowInfoxy.nColumn);
	while(1)
	{
		if (TRUE == gcIsTimerOn)
		{
			if (gstShowInfoxy.nType == 0)
			{
				gnTimerCount++;
				if (gnTimerCount > gstCommParam.stCommParam.nTimeOut)
				{
					gnTimerCount = 1;
				}
			}
			else if (gstShowInfoxy.nType == 1)
			{
				gnTimerCount--;
				if (gnTimerCount < 0)
				{
					gnTimerCount = 0;
				}
			}
			PubDisplayStr(DISPLAY_MODE_CLEARLINE, gstShowInfoxy.nRow, gstShowInfoxy.nColumn, "%02d ",gnTimerCount);
			PubUpdateWindow();
			PubSysDelay(10);
		}
		else
		{
			PubSysMsDelay(100);
		}
	}
}

static int ProShowInfo()
{
	//3
	if(((gstShowInfoxy.nRow ==  0) && (gstShowInfoxy.nColumn== 0)) || gstCommParam.stCommParam.nTimeOut > 999)
		return APP_SUCC;

	gcIsTimerOn = TRUE;
	if (gstShowInfoxy.nType == 1)
	{
		gnTimerCount = gstCommParam.stCommParam.nTimeOut;
	}
	else
	{
		gnTimerCount = 0;
	}

	PubDebugSelectly(1, "ProShowInfo start..[gnThread: %d]", gnThread);//sh
	if (gnThread > 0) /**/
	{
		return APP_SUCC;
	}

	if (pthread_create(&gnThread, NULL, (void *)gstCommParam.stCommParam.ShowFunc, NULL)!= 0)
	{
		PubDebugSelectly(3, "ProShowInfo pthread_create fail");
		return APP_FAIL;
	}
	return APP_SUCC;
}


static void ProExitShowInfo(void)
{
	gcIsTimerOn = FALSE;
	if (gnThread > 0)
	{
        PubDebugSelectly(1, "ProExitShowInfo start...[gnThread: %d]", gnThread);
		if (pthread_cancel(gnThread) != 0)
		{
			PubDebugSelectly(1, "ProExitShowInfo pthread_cancel fail...");
		}
		pthread_join(gnThread,NULL);
	}
	gnThread = 0;
}

/**
* @brief Set style for displaying timeout interface
* @param [in] stShowInfoxy
* @return
* @li void
* @author liug
* @date 2012-5-24
*/
void PubSetShowXY(STSHOWINFOXY stShowInfoxy)
{
	gstShowInfoxy.nRow = stShowInfoxy.nRow;
	gstShowInfoxy.nColumn = stShowInfoxy.nColumn;
	gstShowInfoxy.nType = stShowInfoxy.nType;
}


/**
* @brief Set the timeout
* @param [in] nTimeOut  timeout(>=0)
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author liug
* @date 2012-5-25
*/
int PubSetCommTimeOut(int nTimeOut)
{
	gstCommParam.stCommParam.nTimeOut = nTimeOut;
	return APP_SUCC;
}

/**
* @brief Set the times of redial
* @param [in] nReDialNum  Number of redial(1---9)
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author liug
* @date 2012-5-25
*/
int PubSetReDialNum(int nReDialNum)
{
	gstCommParam.stCommParam.ConnInfo.stDialParam.nCycTimes = nReDialNum;
	return APP_SUCC;
}

static int ProGetErrMsg(int nCommErrCode, char *pszErrMsg)
{
	int i = 0;
	int nErrCode = 0;
	char szErrNum[12+1] = {0};

	if (pszErrMsg == NULL)
	{
		return APP_FAIL;
	}
	for (i = 0; ; i++)
	{
		memcpy(szErrNum, szErrCode[i], 5);
		nErrCode = atoi(szErrNum);
		if ((nErrCode == nCommErrCode) || (nErrCode == -9999))
		{
			strcpy(pszErrMsg, szErrCode[i] + 5);
			PubDebugSelectly(2, "ProGetErrMsg succ...[nCommErrCode: %d][%s]", nCommErrCode, pszErrMsg);
			return APP_SUCC;
		}
	}
	return APP_FAIL;
}

/**
* @brief Connect to network(wifi or ppp)
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author sunh
* @date 2015-1-8
*/
int PubCommDialNet()
{
	PubDebugSelectly(2, "PubCommPppDial Start:");

	if(CommPppDial() != APP_SUCC)
	{
        char szErrMsg[64+1] = {0};
        ProGetErrMsg(GetCommErrorCode(), szErrMsg);
        PubSetErrorCode(GetCommErrorCode(),szErrMsg,GetNapiErrorCode());
		return APP_FAIL;
	}
	PubDebugSelectly(2, "PubCommPppDial Succ.");
	return APP_SUCC;
}


/**
* @brief Domain name resolution
* @param [in] pszDn Domain name
* @param [in] pszDnsIp Domain name service address
* @param [in] cIsQuery Mode YES:read backup file; NO:reparse
* @param [out] pszIp parsed IP
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author sunh
* @date 2015-1-10
*/
int PubCommDnParse(const char *pszDn, const char *pszDnsIp, char *pszIp, char cIsQuery)
{
	PubDebugSelectly(2, "PubCommDnParse Start:");

	if(CommParseDn(pszDn, pszDnsIp, pszIp, cIsQuery) != APP_SUCC)
	{
		return APP_FAIL;
	}

	PubDebugSelectly(2, "PubCommDnParse Succ.");
	return APP_SUCC;
}

/**
* @brief Detecting link connection
* @param [out] pnStatus return EM_DATA_ENABLE or (true or false)
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author sunh
* @date 2015-11-12
*/
int PubCommNetCheck(int *pnStatus)
{
	PubDebugSelectly(2, "PubCommNetCheck Start:");

	if (NULL == pnStatus)
	{
		return APP_FAIL;
	}

	if(CommNetStatusCheck(pnStatus) != APP_SUCC)
	{
		return APP_FAIL;
	}

	PubDebugSelectly(2, "PubCommNetCheck Succ.");
	return APP_SUCC;
}

/* End of lcomm.c */

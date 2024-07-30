/**
* @file process.c
* @brief Process module
* @version  1.0
* @author Liug
* @date 2012-06-05
*/
#include <string.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include "libapiinc.h"
#include <openssl/x509.h>
#include "commtool.h"
#include "commerror.h"
#include "port.h"
#include "asyndial.h"
#include "syndial.h"
#include "wireless.h"
#include "eth.h"
#include "wifi.h"
#include "process.h"
#include "sslsockets.h"
#include "napi_net.h"

STCOMM gstCommParam;
static int nCurrentIndex=1;			 /**<Current communication type index*/

static STSSLMODE gstSslMode;
static char gcIsCustom = 0;

#define CONNECT_TIMEOUT 3*60  /*Connection keeping time*/
#define CHECKHANGUPBEGIN 1000       
static int gnSleepFlag = 0;  /*value between +1 and -1 disable-1,enable +1*/

/**
* @brief Set communication error code
* @param [in]  nError error code
* @return 
* @author Liug
* @date 2012-6-6
*/
void SetCommErrorCode(int nError)
{
	gstCommParam.nErrorCode = nError;
}


/**
* @brief Get communication error code
* @return error code
* @author Liug
* @date 2012-6-6
*/
int GetCommErrorCode()
{
	return gstCommParam.nErrorCode;
}

/**
* @brief Set communication error code
* @param [in]  nError error code
* @return 
* @author Liug
* @date 2012-6-6
*/
void SetNapiErrorCode(int nError)
{
	gstCommParam.nNAPIErrorCode = nError;
}

/**
* @brief Get Napi error code
* @return error code
* @author Liug
* @date 2012-6-6
*/
int GetNapiErrorCode()
{
	return gstCommParam.nNAPIErrorCode;
}


/**
* @brief initialize communication
* @param 
* @return 
* @li APP_FAIL 
* @li APP_SUCC 
* @author Liug
* @date 2012-6-5
*/
int InitComm()
{
	int nRet;
	
PubDebugSelectly(1, "[gstCommParam.stCommParam.cCommType][%d]", gstCommParam.stCommParam.cCommType);

	switch (gstCommParam.stCommParam.cCommType & 0xF0)
	{
	case COMMTYPE_SYNDIAL:
		nRet = SynDialInit(&(gstCommParam.stCommParam.ConnInfo.stDialParam));
		if (nRet != APP_SUCC)
		{
			return nRet;
		}
		gstCommParam.cIsInitComm = TRUE;
		gstCommParam.pClear = SynDialClearBuf;
		gstCommParam.pConnect = SynDialConnect;
		gstCommParam.pGetConnectState = SynDialGetConnectState;
		gstCommParam.pWrite = SynDialWrite;
		gstCommParam.pRead = SynDialRead;
		gstCommParam.pHangUp = SynDialHangUp;
		break;
	case COMMTYPE_ASYNDIAL:
		nRet = AsynDialInit(&(gstCommParam.stCommParam.ConnInfo.stDialParam));
		if (nRet != APP_SUCC)
		{
			return nRet;
		}
		gstCommParam.cIsInitComm = TRUE;
		gstCommParam.pClear = AsynDialClearBuf;
		gstCommParam.pConnect = AsynDialConnect;
		gstCommParam.pGetConnectState = AsynDialGetConnectState;
		gstCommParam.pWrite = AsynDialWrite;
		gstCommParam.pRead = AsynDialRead;
		gstCommParam.pHangUp = AsynDialHangUp;
		break;
	case COMMTYPE_PORT:
	case COMMTYPE_USB:
		nRet = PortInit(&(gstCommParam.stCommParam.ConnInfo.stPortParam));
		if (nRet != APP_SUCC)
		{
			return nRet;
		}
		gstCommParam.cIsInitComm = TRUE;
		gstCommParam.pClear = PortClearBuf;
		gstCommParam.pConnect = PortConnect;
		gstCommParam.pGetConnectState = PortGetConnectState;
		gstCommParam.pWrite = PortWrite;
		gstCommParam.pRead = PortRead;
		gstCommParam.pHangUp = PortHangUp;
		break;
	case COMMTYPE_GPRS:
	case COMMTYPE_CDMA:
		if((gstCommParam.stCommParam.cCommType & 0xF0) == COMMTYPE_GPRS)
			nRet = WirelessInit(&(gstCommParam.stCommParam.ConnInfo.stGprsParam), NULL, gstCommParam.stCommParam.cSslFlag);
		else
			nRet = WirelessInit(NULL, &(gstCommParam.stCommParam.ConnInfo.stCdmaParam), gstCommParam.stCommParam.cSslFlag);
		if (nRet != APP_SUCC)
		{
			return nRet;
		}
		gstCommParam.cIsInitComm = TRUE;
		gstCommParam.pClear = WirelessClearBuf;
		gstCommParam.pConnect = WirelessConnect;
		gstCommParam.pGetConnectState = WirelessGetConnectState;
		gstCommParam.pWrite = WirelessWrite;
		gstCommParam.pRead = WirelessRead;
		gstCommParam.pHangUp = WirelessHangUp;
		break;
	case COMMTYPE_ETH:
		nRet = EthInit(&(gstCommParam.stCommParam.ConnInfo.stEthParam), gstCommParam.stCommParam.cSslFlag);
		if (nRet != APP_SUCC)
		{
			return nRet;
		}
		gstCommParam.cIsInitComm = TRUE;
		gstCommParam.pClear = EthClearBuf;
		gstCommParam.pConnect = EthConnect;
		gstCommParam.pGetConnectState = EthGetConnectState;
		gstCommParam.pWrite = EthWrite;
		gstCommParam.pRead = EthRead;
		gstCommParam.pHangUp = EthHangUp;
		break;
	case COMMTYPE_WIFI:
		nRet = WifiInit(&(gstCommParam.stCommParam.ConnInfo.stWifiParam), gstCommParam.stCommParam.cSslFlag);
		if (nRet != APP_SUCC)
		{
			return nRet;
		}
		gstCommParam.cIsInitComm = TRUE;
		gstCommParam.pClear = WifiClearBuf;
		gstCommParam.pConnect = WifiConnect;
		gstCommParam.pGetConnectState = WifiGetConnectState;
		gstCommParam.pWrite = WifiWrite;
		gstCommParam.pRead = WifiRead;
		gstCommParam.pHangUp = WifiHangUp;
		break;
	default:
		return APP_FAIL;
	}
	return APP_SUCC;
}

/**
* @brief Connection
* @param 
* @return 
* @li APP_FAIL 
* @li APP_SUCC 
* @author Liug
* @date 2012-6-5
*/
int ProCommConnect(char cPreDial)
{
	int nRet;
	PubDebugSelectly(1, "Process ProCommConnect start [cIsConnect: %d][ssl:%d]", gstCommParam.cIsConnect, 
				gstCommParam.stCommParam.cSslFlag);

	if (0 == gnSleepFlag)//
	{
		CommSetSuspend(0); /* 20130628*/ 
		gnSleepFlag--;
	}

	if (gstCommParam.cIsInitComm != TRUE)
	{
		PubDebugSelectly(3, "ProCommConnect has not inited,then init");
		if (APP_SUCC != InitComm())
		{
			return APP_FAIL;
		}
	}

	if (gstCommParam.pConnect == NULL)
	{
		return APP_FAIL;
	}

	if (gstCommParam.cIsConnect == TRUE) //
	{

		if (gstCommParam.pGetConnectState() == APP_SUCC)
		{
			return APP_SUCC;
		}
	}

	nRet = gstCommParam.pConnect(&(gstCommParam.stCommParam.stServerAddress),&(gstCommParam.nConnectIndex),
						gstCommParam.stCommParam.nTimeOut, cPreDial);
PubDebugSelectly(1, "[nConnectIndex: %d][%d]", gstCommParam.nConnectIndex, nRet);
	if (nRet != APP_SUCC)
	{
		gstCommParam.cIsConnect = FALSE;
		return nRet;
	}
	gstCommParam.cIsConnect = TRUE;
	return APP_SUCC;
}

/**
* @brief Clear buffer
* @param [in]  nFlag
* @return 
* @li APP_FAIL 
* @li APP_SUCC 
* @author Liug
* @date 2012-6-7
*/
int CommClearBuf(void)
{
	int nRet;

	if (gstCommParam.cIsInitComm != TRUE)
	{
		return APP_FAIL;
	}

	if (gstCommParam.pClear == NULL)
	{
		return APP_FAIL;
	}
	nRet = gstCommParam.pClear();
	if (nRet != APP_SUCC)
	{
		return nRet;
	}
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
* @date 2012-5-23
*/
int ProCommSend(const char *psData, int nDataLen)
{
	if (gstCommParam.pWrite == NULL)
	{
		return APP_FAIL;
	}
	return gstCommParam.pWrite(psData, nDataLen);
}


/**
* @brief Receiving data
* @param [in]   nNeedLen          
* @param [out]  psOutData         
* @param [out]  pnOutLen          
* @return 
* @li >0      
* @li Fail    
* @li TIMEOUT 
* @author Liug
* @date 2012-5-23
*/
int ProCommRecv(int CommReadType, int nNeedLen,char *psOutData,int *pnOutLen)
{
	int nRecvLen = 0, nTrueLen = 0;
	char sRecvBuf[MAX_RECV_SIZE+1];	/**<*/
	char *psRecv = NULL;
	int nWantLen = 0;
	int nTmpLen = 0;
	int nHasReadLen = FALSE;
	int nTimeout = gstCommParam.stCommParam.nTimeOut;
	
	if (gstCommParam.pRead == NULL)
	{
		return APP_FAIL;
	}

	//
	switch (CommReadType)
	{
	case COMMTYPE_PORT_ASYN:
	case COMMTYPE_PORT_ASYN_TPDU:
	case COMMTYPE_PORT_ASYN1:
	case COMMTYPE_PORT_ASYN1_TPDU:
	case COMMTYPE_USB_ASYN:
	case COMMTYPE_USB_ASYN_TPDU:
	case COMMTYPE_USB_ASYN1:
	case COMMTYPE_USB_ASYN1_TPDU:
	case COMMTYPE_ASYNDIAL_ASYN:
	case COMMTYPE_ASYNDIAL_ASYN_TPDU:
	case COMMTYPE_ASYNDIAL_ASYN1:
	case COMMTYPE_ASYNDIAL_ASYN1_TPDU:
	case COMMTYPE_GPRS_ASYN1:
	case COMMTYPE_CDMA_ASYN1:
	case COMMTYPE_ETH_ASYN1:
	case COMMTYPE_WIFI_ASYN1:
	case COMMTYPE_MODEM_ASYN1:
	case COMMTYPE_GPRS_ASYN1_TPDU:
	case COMMTYPE_CDMA_ASYN1_TPDU:
	case COMMTYPE_ETH_ASYN1_TPDU: 
	case COMMTYPE_WIFI_ASYN1_TPDU: 
	case COMMTYPE_MODEM_ASYN1_TPDU: 
		while (1)
		{
			sRecvBuf[0] = 0;
			nRecvLen = gstCommParam.pRead(0, 1, nTimeout, sRecvBuf);
			if (nRecvLen < 0 )
			{
				return nRecvLen;
			}
			if (sRecvBuf[0] == STX)
			{
				break;
			}
		}
		if (sRecvBuf[0] != STX)
		{
			PubDebugSelectly(3, "Not Received 02");
			return APP_FAIL;
		}

		nRecvLen = gstCommParam.pRead(0, 2, nTimeout, sRecvBuf+1);
		if (nRecvLen < 0 )
		{
			return nRecvLen;
		}

		PubBcdToInt(sRecvBuf + 1, &nTrueLen);
		if (nTrueLen >= MAX_RECV_SIZE)
		{
			PubDebugSelectly(3, "Len[%d]  is Out of Limit ",nTrueLen);
			return APP_FAIL;
		}
		nTmpLen = 3;
		nWantLen = nTrueLen + 2;
		while(1)
		{
    		nRecvLen = gstCommParam.pRead(0, nWantLen, nTimeout, sRecvBuf + nTmpLen);
    		if (nRecvLen < 0 )
    		{
    			return nRecvLen;
    		}
			nTmpLen += nRecvLen;

			if (nTmpLen >= nTrueLen + 5)
			{
				break;
			}
			nWantLen -= nRecvLen;
	    }
		nRecvLen =nTrueLen+5;	
		psRecv = sRecvBuf;
		break;
	case COMMTYPE_SYNDIAL_HEADLEN:
	case COMMTYPE_GPRS_HEADLEN:
	case COMMTYPE_CDMA_HEADLEN:
	case COMMTYPE_ETH_HEADLEN:
	case COMMTYPE_WIFI_HEADLEN:
	case COMMTYPE_MODEM_HEADLEN:
	case COMMTYPE_SYNDIAL_TPDU_HEADLEN:
	case COMMTYPE_GPRS_TPDU_HEADLEN:
	case COMMTYPE_CDMA_TPDU_HEADLEN:
	case COMMTYPE_ETH_TPDU_HEADLEN:
	case COMMTYPE_WIFI_TPDU_HEADLEN:
	case COMMTYPE_MODEM_TPDU_HEADLEN:
		while (1)
		{
			if (nHasReadLen == FALSE)//
			{
				nRecvLen = gstCommParam.pRead(0, COMM_TCPIP_HEAD_LEN, nTimeout,sRecvBuf);
				if (nRecvLen < 0 )
				{
					return nRecvLen;
				}	
				
				if (nRecvLen == 2)
				{
					nHasReadLen = TRUE;
					PubC2ToInt((unsigned int *)&nTrueLen, (unsigned char *)sRecvBuf);
				}
				else if (nRecvLen > 2)//
				{
					nHasReadLen = TRUE;
					PubC2ToInt((unsigned int *)&nTrueLen, (unsigned char *)sRecvBuf);
					nTmpLen += nRecvLen-COMM_TCPIP_HEAD_LEN;
					nTrueLen -= nRecvLen-COMM_TCPIP_HEAD_LEN;	
					if (nTrueLen <= 0)
					{
						break;//
					}
				}

			}
			else 
			{
				if (nTrueLen >= MAX_RECV_SIZE)
				{
					nWantLen = MAX_RECV_SIZE;
				}
				else
				{
					nWantLen = nTrueLen;
				}			
				nRecvLen = gstCommParam.pRead(0, nWantLen, nTimeout,sRecvBuf+COMM_TCPIP_HEAD_LEN+nTmpLen);
				if (nRecvLen < 0 )
				{
					return nRecvLen;
				}	
				nTmpLen += nRecvLen;
				nTrueLen -= nRecvLen;	
				if (nTrueLen <= 0)
				{
					break;
				}				
			}
		}
		nRecvLen = nTmpLen;	
		psRecv = sRecvBuf + COMM_TCPIP_HEAD_LEN;
		break;
	case COMMTYPE_PORT:
	case COMMTYPE_USB:
	case COMMTYPE_SYNDIAL_TPDU:
	case COMMTYPE_SYNDIAL:
	case COMMTYPE_ASYNDIAL:
	case COMMTYPE_GPRS:
	case COMMTYPE_CDMA:
	case COMMTYPE_ETH:
	case COMMTYPE_WIFI:
	case COMMTYPE_MODEM:
		nRecvLen = gstCommParam.pRead(0, nNeedLen, nTimeout,sRecvBuf);
		if (nRecvLen < 0 )
		{
			return nRecvLen;
		}	
		psRecv = sRecvBuf;
		break;
    case 0xFF:
        nRecvLen = gstCommParam.pRead(1, nNeedLen, nTimeout,sRecvBuf);
		if (nRecvLen < 0 )
		{
			return nRecvLen;
		}	
		psRecv = sRecvBuf;
		break;
	}
	memcpy(psOutData, psRecv, nRecvLen);
	*pnOutLen = nRecvLen;
	return APP_SUCC;
}

/**
* @brief Hangup 
* @param [in]   nFlag         
* @return 
* @li APP_SUCC    
* @li Fail    
* @author Liug
* @date 2012-5-23
*/
int CommHungUp(int nFlag)
{
	PubDebugSelectly(1, "CommHungUp start......");
	
	if (-1 == gnSleepFlag)//
	{
		CommSetSuspend(1); /* 20130628*/ 
		gnSleepFlag++;
	}

	if (gstCommParam.cIsInitComm != TRUE)
	{
		PubDebugSelectly(3, "CommHungUp ... has not inited");
		return APP_FAIL;
	}
#if 0//
	if (gstCommParam.cIsConnect != TRUE)
	{
		return APP_FAIL;
	}
#endif 	
	if (gstCommParam.pHangUp == NULL)
	{
		return APP_FAIL;
	}
	
	if( gstCommParam.pHangUp(nFlag) == APP_SUCC)
	{
		gstCommParam.cIsConnect = FALSE;
	}

	return APP_SUCC;
}


/**
* @brief Get connected index
* @param [out]   pnConnectIndex      
* @return 
* @li APP_SUCC    
* @li Fail    
* @author Liug
* @date 2012-6-19
*/
int GetCommConnectIndex(int *pnConnectIndex)
{
	if (gstCommParam.cIsInitComm != TRUE)
	{
		return APP_FAIL;
	}
	
	if (gstCommParam.cIsConnect != TRUE)
	{
		return APP_FAIL;
	}

	*pnConnectIndex = gstCommParam.nConnectIndex;
	return APP_SUCC;
}

/**
* @brief Get socket handle
* @param [out]   pnHandle    
* @return 
* @li APP_SUCC    
* @li Fail    
* @author sunh
* @date 2014-1-1
*/
int GetCommTcpHandle(int *pnHandle)
{
	if (gstCommParam.cIsInitComm != TRUE)
	{
		return APP_FAIL;
	}
	
	if (gstCommParam.cIsConnect != TRUE)
	{
		return APP_FAIL;
	}
	switch (gstCommParam.stCommParam.cCommType & 0xF0)
	{
	//wireless
	case COMMTYPE_GPRS:
	case COMMTYPE_CDMA:
		return WirelessGetTcpHandle(pnHandle);
		break;
	//ethernet
	case COMMTYPE_ETH:
		return EthGetTcpHandle(pnHandle);
		break;
	//WIFI
	case COMMTYPE_WIFI:
		return WifiGetTcpHandle(pnHandle);
		break;
	default:
		return APP_FAIL;
		break;
	}
	return APP_SUCC;
}

int SetSslMode(STSSLMODE *pstSslMode)
{
	if (NULL == pstSslMode)
		return APP_FAIL;

	memcpy(&gstSslMode, pstSslMode, sizeof(STSSLMODE));
	gcIsCustom = 1;
	return APP_SUCC;
}

int GetSslMode(STSSLMODE *pstSslMode, char *pcIsCustomized)
{
	if (NULL == pstSslMode || NULL == pcIsCustomized)
		return APP_FAIL;
	
	memcpy(pstSslMode, &gstSslMode, sizeof(STSSLMODE));
	*pcIsCustomized = gcIsCustom;
	return APP_SUCC;
}

int GetSslCertMsg(STSSLCERTMSG* pstSslCertMsg)
{
	int nHandle = 0;
	void *pvCerificate;
	char szVer[10] = {0};
	
	if (NULL == pstSslCertMsg)
		return APP_FAIL;
	
	GetCommTcpHandle(&nHandle);	
	if (nHandle == 0)
	{
		PubDebugSelectly(2, "GetCommTcpHandle Fail...");
		return APP_FAIL;
	}
	pvCerificate = _SSLGetPeerCerificate_((SSL_HANDLE)nHandle);
	if(pvCerificate != NULL)
	{
		X509 *pX509=NULL;
		X509_NAME * name;
		char *pstr;
		ASN1_TIME *time;	/*< */
	
		pX509 = (X509 *)pvCerificate;

		pstr = X509_NAME_oneline(X509_get_subject_name (pX509),0,0);
		strcpy(pstSslCertMsg->szSubjectName, pstr);
		CRYPTO_free (pstr);

		pstr = X509_NAME_oneline (X509_get_issuer_name	(pX509),0,0);
		strcpy(pstSslCertMsg->szIssuername, pstr);
		CRYPTO_free (pstr);

		pstSslCertMsg->nVersion = ASN1_INTEGER_get(pX509->cert_info->version);

		time = X509_get_notBefore(pX509);
		strcpy(pstSslCertMsg->szNotBeforeTime, (char *)time->data);

		time = X509_get_notAfter(pX509);
		strcpy(pstSslCertMsg->szNotAfterTime, (char *)time->data);

		name = X509_get_subject_name(pX509);

		X509_NAME_get_text_by_NID(name, NID_countryName, pstSslCertMsg->szCountryName, 32);

		X509_NAME_get_text_by_NID(name, NID_commonName, pstSslCertMsg->szCommonName, 32);

		X509_NAME_get_text_by_NID(name, NID_organizationName, pstSslCertMsg->szOrganizationName, 64);

		X509_NAME_get_text_by_NID(name, NID_organizationalUnitName, pstSslCertMsg->szOrganizationalUnitName, 64);

		PubDebugSelectly(3, "owner info:[%s]", pstSslCertMsg->szSubjectName);
		PubDebugSelectly(3, "Issuername:[%s]", pstSslCertMsg->szIssuername);
		PubDebugSelectly(3, "Ver:[%s]",szVer);
		PubDebugSelectly(3, "effective time:[%s]",pstSslCertMsg->szNotBeforeTime);
		PubDebugSelectly(3, "disable time:[%s]",pstSslCertMsg->szNotAfterTime);
		PubDebugSelectly(3, "Country Name:[%s]", pstSslCertMsg->szCountryName);
		PubDebugSelectly(3, "server Name:[%s]", pstSslCertMsg->szCommonName);
		PubDebugSelectly(3, "OrganizationName:[%s]", pstSslCertMsg->szOrganizationName);
		PubDebugSelectly(3, "szOrganizationalUnitName:[%s]", pstSslCertMsg->szOrganizationalUnitName);
		return APP_SUCC;
	}

	PubDebugSelectly(3, "GetSslCertMsg Fail...");
	return APP_FAIL;
}

int GetVarCommMode()
{
	return gstCommParam.stCommParam.cMode;
}

int CommSetSuspend(uint unFlag)
{
	int nRet = -1;
	int i = 0;
	
	while (1)
	{
		if (i == 3)
		{
			break;
		}
		nRet = NAPI_SysSetAutoSleep(unFlag);
		if (NAPI_OK != nRet)
		{
			PubSysDelay(30);//3
			i++;
			continue;
		}
		return APP_SUCC;
	}
	return APP_FAIL;
}

int CommPppDial()
{
	int nRet = 0;
	
PubDebugSelectly(1, "CommPppDial start... [nCurrentIndex: %d]",nCurrentIndex);

	if (gstCommParam.cIsInitComm != TRUE)
	{
		PubDebugSelectly(3, "CommPppDial. has not inited,then init");
		if (APP_SUCC != InitComm())
		{
			return APP_FAIL;
		}
		//return APP_FAIL;
	}
	
	switch (gstCommParam.stCommParam.cCommType & 0xF0)
	{
	case COMMTYPE_GPRS:
	case COMMTYPE_CDMA:
		nRet = WirelessPppDial(gstCommParam.stCommParam.nTimeOut);
		break;
	case COMMTYPE_WIFI:
		nRet = WifiSsidConnect(gstCommParam.stCommParam.nTimeOut);
		break;
	default:
		break;
	}
	return nRet;
}

int CommParseDn(const char *pszDn, const char *pszDnsIp, char *pszIp, char cIsQuery)
{
	int nType = 0;
	int nTimeout = 60;
	STSERVERADDRESS stServerAddress;

	memset(&stServerAddress, 0, sizeof(STSERVERADDRESS));

	strcpy(stServerAddress.szDN, pszDn);
	if (pszDnsIp != NULL)
	{
		strcpy(stServerAddress.szDNSIp, pszDnsIp);
	}
	switch (gstCommParam.stCommParam.cCommType & 0xF0)
	{
	case COMMTYPE_GPRS:
	case COMMTYPE_CDMA:
		nType = WLM;
		break;
	case COMMTYPE_WIFI:
		nType = WIFI;
		break;
	case COMMTYPE_ETH:
		nType = ETH;
		break;
	default:
		PubDebugSelectly(3, "CommGetNetAddr not support...[%d]", gstCommParam.stCommParam.cCommType);
		return APP_FAIL;
		break;
	}
	return ParseDnsIp(nType, &stServerAddress, &nTimeout, pszIp, cIsQuery);
}

void SetCommError(int nCommErr, int nNapiErr)
{
	SetCommErrorCode(nCommErr);
	SetNapiErrorCode(nNapiErr);
}

int CommNetStatusCheck(int *pnStatus)
{
	int nRet = 0;
	
	switch (gstCommParam.stCommParam.cCommType & 0xF0)
	{
	case COMMTYPE_GPRS:
	case COMMTYPE_CDMA:
		nRet = WirelessConnectState(pnStatus);
		break;
	case COMMTYPE_WIFI:
		nRet = WifiConnectState(pnStatus);
		break;
	default:
		break;
	}
	
	return nRet;
}
	
//Get first address of DNS
static void FetchFirstDns(const char *pszSrc, char *pszDst)
{
	int nLen = strlen(pszSrc);
	int i = 0;

	if (nLen == 0)
		return;

	while (i<nLen)
	{
		if (pszSrc[i] == ';')
			return;
		pszDst[i] = pszSrc[i];
		i++;
	}
}

int CommGetNetAddr(STETHPARAM  *pstEthParam)
{
	int nRet = 0, nType = 0;
	char szDns[256] = {0};
	
	switch (gstCommParam.stCommParam.cCommType & 0xF0)
	{
	case COMMTYPE_WIFI:
		nType = WIFI;
		break;
	case COMMTYPE_ETH:
		nType = ETH;
		break;
	case COMMTYPE_GPRS:
	case COMMTYPE_CDMA:
		nType = WLM;
		break;
	default:
		PubDebugSelectly(3, "CommGetNetAddr not support...[%d]", gstCommParam.stCommParam.cCommType);
		return APP_FAIL;
		break;
	}

	nRet = NAPI_NetGetContext((NET_TYPE)nType,pstEthParam->szIP,pstEthParam->szMask,pstEthParam->szGateway,szDns);
	if(NAPI_OK != nRet)
	{
		PubDebugSelectly(3, "NAPI_NetGetAddr fail...[%d]", nRet);
		return APP_FAIL;
	}

	FetchFirstDns(szDns, pstEthParam->szDNS);
	
	return APP_SUCC;
}


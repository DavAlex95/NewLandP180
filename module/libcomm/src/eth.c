#include <string.h>
#include "eth.h"
#include "process.h"
#include "commerror.h"
#include "commtool.h"
#include "libapiinc.h"
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <dirent.h>
#include <stdarg.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <sys/wait.h>
#include "napi_net.h"
#include "napi_eth.h"

static int gnHandle;
static int gnLastConnectIndex = -1;//connected index
static char gcSslFlag;//ssl flag

/**
* @brief Ethernet initialize, set the IPDNS
* @param [in] pstEthParam
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author Linw
* @date 2012-06-19
*/
int EthInit(STETHPARAM *pstEthParam, char cSsl)
{
	int nRet = 0;
	STETHPARAM stTmpEthParam;
    char szDNS[50];

	nRet = NAPI_EthOpen();
	if (nRet != NAPI_OK)
	{
		SetCommError(FAIL_ETH_DEVICE_OPEN, nRet);
		PubDebugSelectly(3, "Eth device open fail");
		return APP_FAIL;
	}

	PubDebugSelectly(3, "EthInit start---[DHCP:%d][Ip:%s][Mask:%s][Gate:%s][DNS:%s]",pstEthParam->nDHCP,pstEthParam->szIP,\
			pstEthParam->szMask,pstEthParam->szGateway, pstEthParam->szDNS);
	if (!pstEthParam->nDHCP)
	{
		NAPI_EthDisableDHCP();
		if( strlen(pstEthParam->szIP) == 0 || strlen(pstEthParam->szMask) == 0
			|| strlen(pstEthParam->szGateway) == 0)
	        {
				SetCommError(FAIL_TCPIP_INVAILIDPARAM, 0);
				PubDebugSelectly(3, "Eth Param error---[Ip:%s][Mask:%s][Gateway:%s]",pstEthParam->szIP,\
				pstEthParam->szMask,pstEthParam->szGateway);
		            return APP_FAIL;
	        }
		if (strlen(pstEthParam->szDNS) < 8)
		{
			nRet = NAPI_NetSetContext(ETH, pstEthParam->szIP, pstEthParam->szMask, pstEthParam->szGateway, NULL);
		}
		else
		{
			nRet = NAPI_NetSetContext(ETH, pstEthParam->szIP, pstEthParam->szMask, pstEthParam->szGateway, pstEthParam->szDNS);
	    }

		if (NAPI_OK != nRet)
		{
			SetCommError(FAIL_TCPIP_SETLOCALIP, nRet);
			PubDebugSelectly(3, "NAPI_EthSetAddress error[%d]---[Ip:%s][Mask:%s][Gateway:%s]",nRet,\
			pstEthParam->szIP,pstEthParam->szMask,pstEthParam->szGateway);//pstEthParam->szDNS
        	return APP_FAIL;
		}
	}
	else
	{
		nRet = NAPI_EthEnableDHCP();
		if (NAPI_OK != nRet)
		{
			SetCommError(FAIL_TCPIP_DHCP, nRet);
			PubDebugSelectly(3, "NAPI_NetDHCP error[%d]",nRet);
			return APP_FAIL;
		}
		memset(&stTmpEthParam, 0, sizeof(stTmpEthParam));
        nRet = NAPI_NetGetContext(ETH, stTmpEthParam.szIP, stTmpEthParam.szMask,  stTmpEthParam.szGateway, szDNS);
        if (nRet != NAPI_OK)
        {
            PubDebugSelectly(3, "NAPI_NetGetContext fail!!!");
            return APP_FAIL;
       }
    }
	gnHandle = 0;
	gcSslFlag = cSsl;
	PubDebugSelectly(3, "EthInit succ");
	return APP_SUCC;
}

/**
* @brief Ethernet connection
* @param [in] pstServerAddress
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author Linw
* @date 2012-06-19
*/
int EthConnect(const STSERVERADDRESS *pstServerAddress, int *pnConnectIndex,int nTimeOut, int nIsPreConnect)
{
	int i = 0;
	int nRet = APP_FAIL;
	int nTmpTimeout = nTimeOut;

	//
	CommserverLedShine(LED_ONL_FLICK);

	if (EM_PRECONNECT == nIsPreConnect)
	{
		PubDebugSelectly(3, "PreConnect succ: ");
		return APP_SUCC;
	}

	PubDebugSelectly(3, "EthConnect start...[gnLastConnectIndex: %d][gnHandle: %d]", gnLastConnectIndex, gnHandle);

	//domain connection
	if ((strlen(pstServerAddress->lszIp[0]) == 0) && (strlen(pstServerAddress->szDN) != 0))
	{
		char szIp[20] = {0};
		char cIsDnsCheck = YES;

		while (1)
		{
			if (APP_SUCC != ParseDnsIp(ETH, pstServerAddress, &nTmpTimeout, szIp, cIsDnsCheck))
			{
				return APP_FAIL;
			}

			//
			if (APP_SUCC != TcpOpen(&gnHandle, gcSslFlag))
			{
				return APP_FAIL;
			}
			nRet = TcpConnect(gnHandle, gcSslFlag, szIp,pstServerAddress->lnPort[0], nTmpTimeout);
			if (APP_SUCC != nRet)
			{
				EthHangUp(HANGUP_SOCKET);
				if (YES == cIsDnsCheck)
				{
					cIsDnsCheck = NO;
					memset(szIp, 0, sizeof(szIp));
					continue;
				}
				return APP_FAIL;
			}
			else
			{
				CommserverLedShine(LED_ONL_ON);
				PubDebugSelectly(3, "EthConnect succ[hd: %s]", gnHandle);
				return APP_SUCC;
			}
		}
	}

	if (gnLastConnectIndex != -1)
	{
		//
		if (APP_SUCC != TcpOpen(&gnHandle, gcSslFlag))
		{
			return APP_FAIL;
		}
		//
		nRet = TcpConnect(gnHandle, gcSslFlag, pstServerAddress->lszIp[gnLastConnectIndex],pstServerAddress->lnPort[gnLastConnectIndex], nTmpTimeout/3);
		if (APP_SUCC != nRet)
		{
			EthHangUp(HANGUP_SOCKET);
			nTmpTimeout -= nTmpTimeout/3;
			gnLastConnectIndex = -1;
		}
		else
		{
			CommserverLedShine(LED_ONL_ON);
			*pnConnectIndex = gnLastConnectIndex;
			PubDebugSelectly(3, "EthConnect succ[%d]", gnHandle);
			return APP_SUCC;
		}
	}

	for (i=0;i<2;i++)
	{
		//
		if (APP_SUCC != TcpOpen(&gnHandle, gcSslFlag))
		{
			return APP_FAIL;
		}

		if (strlen(pstServerAddress->lszIp[i]) <= 0)
		{
			SetCommError(FAIL_TCPIP_CONNECTNULL, 0);
			PubDebugSelectly(3, "Tcp Connect null error[%d]---[TcpHandle:%d][ServerIp:%s][ServerPort:%d].",nRet,\
				gnHandle,pstServerAddress->lszIp[i],pstServerAddress->lnPort[i]);
			EthHangUp(HANGUP_SOCKET);
			continue;
		}
		//
		nRet = TcpConnect(gnHandle, gcSslFlag, pstServerAddress->lszIp[i],pstServerAddress->lnPort[i], nTmpTimeout/2);
		if (nRet != APP_SUCC)
		{
			EthHangUp(HANGUP_SOCKET);
			continue;
		}
		CommserverLedShine(LED_ONL_ON);
		gnLastConnectIndex = i;
		*pnConnectIndex = gnLastConnectIndex;
		PubDebugSelectly(3, "EthConnect succ[%d]", gnHandle);
		return APP_SUCC;
	}
	gnLastConnectIndex = -1;
	EthHangUp(HANGUP_SOCKET);
	return APP_FAIL;
}

/**
* @brief Get ethernet connection status
* @param
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author Ling
* @date 2012-11-15
*/
int EthGetConnectState()
{
	return TcpGetStatus(gnHandle, gnLastConnectIndex);
}

/**
* @brief Sending data
* @param [in]  psData
* @param [in]  nDataLen
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author Linw
* @date 2012-06-19
*/
int EthWrite(const char *psData,int nDataLen)
{
	int nRet;

	nRet = TcpWrite(gnHandle, gcSslFlag, psData, nDataLen);
	if (APP_SUCC != nRet)
	{
		EthHangUp(HANGUP_SOCKET);//
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
* @author Linw
* @date 2012-06-19
*/
int EthRead(int nIsNonBlock, int nLen,int nTimeOut,char *psOutData)
{
	int nRet;
	uint unReadLen=0;

	nRet = TcpRead(nIsNonBlock, gnHandle, gcSslFlag, psOutData, nLen, &unReadLen, nTimeOut);
	switch(nRet)
	{
	case APP_QUIT:
	case APP_TIMEOUT:
	case APP_FAIL:
		EthHangUp(HANGUP_SOCKET);//
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


/**
* @brief Hangup
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author Linw
* @date 2012-06-19
*/
int EthHangUp(int nFlag)
{
	int nRet;
	PubDebugSelectly(2, "EthHangUp start---[flag:%d]", nFlag);

	CommserverLedShine(LED_COM_OFF);
	CommserverLedShine(LED_ONL_OFF);

	nRet = TcpHangUp(&gnHandle, gcSslFlag, nFlag);
	if (nRet != APP_SUCC)
	{
		return APP_FAIL;
	}

	if (nFlag == HANGUP_PPP)
	{
		NAPI_EthDisableDHCP();
		NAPI_EthClose();
	}

	return nRet;
}

/**
* @brief Clear buffer
* @param
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author Linw
* @date 2012-6-7
*/
int EthClearBuf()
{
	PubDebugSelectly(1, "EthClearBuf succ");
	return APP_SUCC;
}

int EthGetTcpHandle(int *pnHandle)
{
	return TcpGetHandle(gnHandle, pnHandle);
}


/**
* @brief Reset index
* @param
* @return
* @author sh
* @date 2014-2-17
*/
void EthResetIndex()
{
	gnLastConnectIndex = -1;
}

void EthUpdateSslFlag(char cSsl)
{
	gcSslFlag = cSsl;
}


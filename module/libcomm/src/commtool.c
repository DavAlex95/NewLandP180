#include <string.h>
#include "libapiinc.h"
#include <netinet/tcp.h>
#include <errno.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "sslsockets.h"
#include "commerror.h"
#include "commtool.h"
#include "process.h"
#include "linuxsocket.h"
#include "napi_net.h"

/**
* @brief Get time interval
* @param [in] unFirstDatetime begining time
* @param [in] unSecondDatetime ending time
* @return time interval value
* @author sunh
* @date
*/
int GetTimeDif(uint unFirstDatetime,uint unSecondDatetime)
{
	struct tm stDatetime;
	char szDatetime[12]={0};
	char szBuf[10+1];
	time_t tSecond;

	memset(&stDatetime,0,sizeof(stDatetime));
	sprintf(szDatetime,"%09u",unFirstDatetime);
	stDatetime.tm_year = 112;
	stDatetime.tm_mon = 10;
	stDatetime.tm_mday = 11;
	memset(szBuf,0,sizeof(szBuf));
	memcpy(szBuf,szDatetime,2);//h
	stDatetime.tm_hour = atol(szBuf);

	memset(szBuf,0,sizeof(szBuf));
	memcpy(szBuf,szDatetime+2,2);//s
	stDatetime.tm_min = atol(szBuf);

	memset(szBuf,0,sizeof(szBuf));
	memcpy(szBuf,szDatetime+4,2);//s
	stDatetime.tm_sec = atol(szBuf);

	tSecond = mktime(&stDatetime);
	memset(&stDatetime,0,sizeof(stDatetime));
	sprintf(szDatetime,"%09u",unSecondDatetime);
	stDatetime.tm_year = 112;
	stDatetime.tm_mon = 10;
	stDatetime.tm_mday = 11;
	memset(szBuf,0,sizeof(szBuf));
	memcpy(szBuf,szDatetime,2);//h
	stDatetime.tm_hour = atol(szBuf);

	memset(szBuf,0,sizeof(szBuf));
	memcpy(szBuf,szDatetime+2,2);//s
	stDatetime.tm_min = atol(szBuf);

	memset(szBuf,0,sizeof(szBuf));
	memcpy(szBuf,szDatetime+4,2);//s
	stDatetime.tm_sec = atol(szBuf);

	tSecond = mktime(&stDatetime) - tSecond;

	if (tSecond < 0)
		tSecond = 0;
	return tSecond;
}

/**********************
* about ssl socket
*/
static int ciphers[] = {
	SSL3_CIPHER_RSA_NULL_MD5,
	SSL3_CIPHER_RSA_NULL_SHA,
	SSL3_CIPHER_RSA_RC4_40_MD5,
	SSL3_CIPHER_RSA_RC4_128_MD5,
	SSL3_CIPHER_RSA_RC4_128_SHA,
	SSL3_CIPHER_RSA_RC2_40_MD5,
	SSL3_CIPHER_RSA_IDEA_128_SHA,
	SSL3_CIPHER_RSA_DES_40_CBC_SHA,
	SSL3_CIPHER_RSA_DES_64_CBC_SHA,
	SSL3_CIPHER_RSA_DES_192_CBC3_SHA,
	SSL3_CIPHER_DH_RSA_DES_192_CBC3_SHA,
	SSL3_CIPHER_DH_DSS_DES_40_CBC_SHA,
	SSL3_CIPHER_DH_DSS_DES_64_CBC_SHA,
	SSL3_CIPHER_DH_DSS_DES_192_CBC3_SHA,
	SSL3_CIPHER_DH_RSA_DES_40_CBC_SHA,
	SSL3_CIPHER_DH_RSA_DES_64_CBC_SHA,
	SSL3_CIPHER_EDH_DSS_DES_40_CBC_SHA,
	SSL3_CIPHER_EDH_DSS_DES_64_CBC_SHA,
	SSL3_CIPHER_EDH_DSS_DES_192_CBC3_SHA,
	SSL3_CIPHER_EDH_RSA_DES_40_CBC_SHA,
	SSL3_CIPHER_EDH_RSA_DES_64_CBC_SHA,
	SSL3_CIPHER_EDH_RSA_DES_192_CBC3_SHA,
	SSL3_CIPHER_ADH_RC4_40_MD5,
	SSL3_CIPHER_ADH_RC4_128_MD5,
	SSL3_CIPHER_ADH_DES_40_CBC_SHA,
	SSL3_CIPHER_FZA_DMS_NULL_SHA,
	SSL3_CIPHER_CK_FZA_DMS_FZA_SHA,
	SSL3_CIPHER_CK_FZA_DMS_RC4_SHA,
	SSL3_CIPHER_CK_ADH_DES_64_CBC_SHA,
	SSL3_CIPHER_CK_ADH_DES_192_CBC_SHA,
	0x0	};

int TcpOpen(int *pnHandle, char cSslFlag)
{
	int nRet = 0;
	int nHandle;
	STSSLMODE stSslMode;
	SSL_HANDLE handle;
	char cCustomFlag = 0;

	if (NULL == pnHandle)
		return APP_FAIL;

	//get certificate
	memset(&stSslMode, 0, sizeof(stSslMode));
	GetSslMode(&stSslMode, &cCustomFlag);

	switch(cSslFlag)
	{
	case SSL_SINGLE_AUTH:
		if (cCustomFlag != 1)
		{
			handle = _OpenSSLSocket_(HANDSHAKE_SSLv3,SSL_AUTH_NONE,ciphers);
		}
		else
		{
			handle = _OpenSSLSocket_(stSslMode.nType,stSslMode.nAuthOpt,ciphers);
		}
		PubDebugSelectly(2, "_OpenSSLSocket_ ... [nHandle:%d]",handle);
		if(handle == NULL) //0
		{
			SetCommErrorCode(FAIL_TCPIP_SINGLE_SSL_OPEN);
			//SetNapiErrorCode(nRet);
			PubDebugSelectly(3, "_OpenSSLSocket_ error[%d]",nRet);
			break;
		}

		if (cCustomFlag == 1)
		{
			if (strlen(stSslMode.szServerCert) != 0)
			{
				nRet= _LoadServerCertificate_((SSL_HANDLE)handle,stSslMode.szServerCert,stSslMode.nFileFormat);
				if(nRet != NAPI_OK)
				{
					SetCommErrorCode(FAIL_TCPIP_SSL_SERVERCERT_LOAD);
					SetNapiErrorCode(nRet);//ssl
					PubDebugSelectly(3, "_LoadServerCertificate_ error[%d]",nRet);
					break;
				}
				PubDebugSelectly(3, "_LoadServerCertificate_ succ");
			}
		}
		*pnHandle = (int)handle;
		return APP_SUCC;
		break;
	case SSL_MUTUAL_AUTH:
		handle = _OpenSSLSocket_(stSslMode.nType,stSslMode.nAuthOpt,ciphers);
		if(handle == NULL)
		{
			SetCommErrorCode(FAIL_TCPIP_MUTURE_SSL_OPEN);
			//SetNapiErrorCode(nRet);
			PubDebugSelectly(3, "_OpenSSLSocket_ error[%d]",nRet);
			break;
		}
		nRet= _LoadServerCertificate_((SSL_HANDLE)handle,stSslMode.szServerCert,stSslMode.nFileFormat);
		if(nRet!=NAPI_OK)
		{
			SetCommErrorCode(FAIL_TCPIP_SSL_SERVERCERT_LOAD);
			SetNapiErrorCode(nRet);
			PubDebugSelectly(3, "_LoadServerCertificate_ error[%d]",nRet);
			break;
		}
		PubDebugSelectly(2, "_LoadServerCertificate_ succ");
		nRet =  _LoadClientCertificate_((SSL_HANDLE)handle,stSslMode.szClientCert,stSslMode.nFileFormat);
		if(nRet!=NAPI_OK)
		{
			SetCommErrorCode(FAIL_TCPIP_SSL_CLIENTCERT_LOAD);
			SetNapiErrorCode(nRet);
			PubDebugSelectly(3, "_LoadClientCertificate_ error[%d]",nRet);
			break;
		}
		PubDebugSelectly(2, "_LoadClientCertificate_ succ");
		if (strlen(stSslMode.szPwd) == 0)
			nRet = _LoadClientPrivateKey_((SSL_HANDLE)handle,stSslMode.szClientPrivateKey,stSslMode.nFileFormat,NULL);
		else
			nRet = _LoadClientPrivateKey_((SSL_HANDLE)handle,stSslMode.szClientPrivateKey,stSslMode.nFileFormat,stSslMode.szPwd);
		if(nRet!=NAPI_OK)
		{
			SetCommErrorCode(FAIL_TCPIP_SSL_CLIENTKEY_LOAD);
			SetNapiErrorCode(nRet);
			PubDebugSelectly(3, "_LoadClientPrivateKey_ error[%d]",nRet);
			break;
		}
		PubDebugSelectly(2, "_LoadClientPrivateKey_ succ");
		*pnHandle = (int)handle;
		return APP_SUCC;
		break;
	case SSL_FORBIDDEN:
	default:
		nRet = _TcpOpen((uint *)&nHandle);
		if (TCP_OK != nRet)
		{
			SetCommError(FAIL_TCPIP_OPEN, nRet);
			PubDebugSelectly(3, "_TcpOpen error[%d]",nRet);
			break;
		}
		*pnHandle = (int)nHandle;
		return APP_SUCC;
		break;
	}
	return APP_FAIL;
}

int TcpConnect(int nHandle, char cSslFlag, const char *pszRemoteIp, ushort usRemotePort, uint unTimeout)
{
	int nRet = 0;

	PubDebugSelectly(3, "TcpConnect start---[Ip:%s][port:%d][timeout:%d][ssl:%d]",
						pszRemoteIp,usRemotePort, unTimeout, cSslFlag);
	switch (cSslFlag)
	{
	case SSL_SINGLE_AUTH:
    case SSL_MUTUAL_AUTH:{
        ST_SOCKET_ADDR stAddr;
		memset(&stAddr, 0, sizeof(ST_SOCKET_ADDR));
		stAddr.unAddrType = SSL_ADDR_IPV4;
		stAddr.usPort = usRemotePort;
		stAddr.psAddr = (char*)pszRemoteIp;
		nRet= _SSLConnect_((SSL_HANDLE)nHandle, &stAddr,unTimeout*1000);//ms
		if (NAPI_OK != nRet)
		{
			SetCommErrorCode(FAIL_TCPIP_SSL_CONNECT);
			SetNapiErrorCode(nRet);
			PubDebugSelectly(3, "_SSLConnect_ error...[%d]",nRet);
			return APP_FAIL;
		}
    }
    break;

	case SSL_FORBIDDEN:
	default:
		nRet = _TcpConnect(nHandle,pszRemoteIp,usRemotePort, unTimeout);//TCP_CONNECT_TIMEOUT
		if (nRet != TCP_OK)
		{
			SetCommError(FAIL_TCPIP_CONNECT, nRet);
			PubDebugSelectly(3, "_TcpConnect error...[%d]",nRet);
			return APP_FAIL;
		}
		break;
	}
	PubDebugSelectly(3, "TcpConnect succ...");
	return APP_SUCC;
}

int TcpGetStatus(int nHandle, int index)
{
	struct tcp_info stInfo;
	int nLen = sizeof(stInfo);

	PubDebugSelectly(2, "TcpGetStatus start...[index:%d]", index);
	if (nHandle <= 0|| index == -1)
	{
		PubDebugSelectly(2, "TcpGetStatus  fail...");
		return APP_FAIL;
	}
	getsockopt(nHandle, IPPROTO_TCP, TCP_INFO, &stInfo, (socklen_t *)&nLen);
	if((stInfo.tcpi_state == TCP_ESTABLISHED))
	{
		PubDebugSelectly(2, "TcpGetStatus  succ...");
		return APP_SUCC;
	}
	else
	{
		PubDebugSelectly(3, "TcpGetStatus(%d) Fail...", nHandle);
		return APP_FAIL;
	}
}

int TcpWrite(int nHandle, char cSslFlag, const char *psData,int nDataLen)
{
	int nRet, nLen = 0;

	PubDebugData( "TcpWrite start: ",psData,nDataLen);

	switch (cSslFlag)
	{
	case SSL_SINGLE_AUTH:
	case SSL_MUTUAL_AUTH:
		nRet= _SSLSend_((SSL_HANDLE)nHandle, psData, nDataLen,(uint *)&nLen);
		if (nRet != NAPI_OK)
		{
			SetCommErrorCode(FAIL_TCPIP_SSL_WRITE);
			SetNapiErrorCode(nRet);
			PubDebugSelectly(3, "NAPI_SSLSend error[%d]---[nDataLen:%d]",nRet,\
				nDataLen);
			return APP_FAIL;
		}
		break;
	case SSL_FORBIDDEN:
	default:
		nRet = _TcpWrite(nHandle,psData,nDataLen,TCP_WRITE_TIMEOUT,(uint *)&nLen);
		if (nRet != TCP_OK)
		{
			SetCommError(FAIL_TCPIP_WRITE, nRet);
			PubDebugSelectly(3, "_TcpWrite error[%d]---[TcpHandle:%d] [nDataLen:%d]",nRet,\
				nHandle, nDataLen);
			return APP_FAIL;
		}
		break;
	}

	PubDebugSelectly(3, "TcpWrite succ");
	return APP_SUCC;
}

static char gcSSLDataAvailable = FALSE;//

int TcpRead(int nIsNonBlock, int nHandle, char cSslFlag, char *psOutData, uint nLen, uint *punReadLen, int nTimeOut)
{
	int nRet;
	uint unOverTime = 0;

	PubDebugSelectly(2, "TcpRead start---[len:%d] cSslFlag [%d] nIsNonBlock %d gcSSLDataAvailable %d, nTimeOut [%d]",nLen, cSslFlag,
	nIsNonBlock, gcSSLDataAvailable, nTimeOut);

	switch (cSslFlag)
	{
	case SSL_SINGLE_AUTH:
	case SSL_MUTUAL_AUTH:
		if (nIsNonBlock == 1)
		{
			_SetSSLBlockingMode_((SSL_HANDLE)nHandle, NAPI_NOWAIT);
			PubDebugSelectly(2, "SSL TcpRead NoBlock");
		}
		if (gcSSLDataAvailable == FALSE)
		{
			nRet= _SSLDataAvailable_((SSL_HANDLE)nHandle, nTimeOut);
			if(nRet != NAPI_OK)
			{
				SetCommErrorCode(FAIL_TCPIP_SSL_READ);
				SetNapiErrorCode(nRet);
				if (nRet == NAPI_ERR_SSL_TIMEOUT)
				{
                    PubDebugSelectly(3, "SSL timeout !!!!");
                    SetCommError(APP_TIMEOUT, 0);
					return APP_TIMEOUT;
				}
				else
				{
					PubDebugSelectly(3, "_SSLDataAvailable_ error[%d]---[NeedLen:%d]",nRet,\
						nLen);
				}
				return APP_FAIL;
			}
		}
		nRet = _SSLReceive_((SSL_HANDLE)nHandle, psOutData, nLen, punReadLen);
		if(nRet != NAPI_OK)
		{
			SetCommErrorCode(FAIL_TCPIP_SSL_READ);
			SetNapiErrorCode(nRet);
			PubDebugSelectly(3, "_SSLReceive_ error[%d]---[NeedLen:%d]",nRet,\
					nLen);
			return APP_FAIL;
		}
		if (*punReadLen <= 4 && *punReadLen > 0) //NAPI_SSLDataAvailable
		{
			gcSSLDataAvailable = TRUE;
		}
		else
		{
			gcSSLDataAvailable = FALSE;
		}
		break;
	case SSL_FORBIDDEN:
	default:
		if (nIsNonBlock == 0)
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
				nRet = _TcpRead(nHandle, psOutData, nLen, 0, punReadLen);
				if(nRet != TCP_OK)
				{
					if(nRet == TCP_ERR_TIMEOUT)
					{
						continue;
					}
					SetCommError(APP_FAIL, nRet);
					return APP_FAIL;
				}
				break;
			}
		}
        else
		{
			nRet = _TcpRead(nHandle, psOutData, nLen, 0, punReadLen);
			if(nRet != TCP_OK)
			{
				if(nRet == TCP_ERR_TIMEOUT)
				{
					return 0;
				}
				else
				{
					SetCommError(FAIL_TCPIP_READ, nRet);
					PubDebugSelectly(3, "_TcpRead error[%d]---[TcpHandle:%d][NeedLen:%d]",nRet,nHandle,nLen);
					return APP_FAIL;
				}
			}
        }
		break;
	}

	PubDebugData("TcpRead succ: ", psOutData,*punReadLen);
	return APP_SUCC;
}

int TcpHangUp(int *pnHandle, char cSslFlag, int nFlag)
{
	int nRet;

	PubDebugSelectly(2, "TcpHangUp start...[nHandle: %d] [cSslFlag: %d]", *pnHandle, cSslFlag);
	switch (cSslFlag)
	{
	case SSL_SINGLE_AUTH:
	case SSL_MUTUAL_AUTH:
		if (*pnHandle != 0)
		{
			nRet = _SSLDisconnect_((SSL_HANDLE)(*pnHandle));
			if (NAPI_OK != nRet)
			{
				SetCommErrorCode(FAIL_TCPIP_SSL_DISCONNECT);
				SetNapiErrorCode(nRet);
				PubDebugSelectly(3, "NAPI_SSLDisconnect error[%d]",nRet);
				return APP_FAIL;//ssldisconnect
			}
			nRet = _CloseSSLSocket_((SSL_HANDLE)(*pnHandle));
			if (NAPI_OK != nRet)
			{
				SetCommErrorCode(FAIL_TCPIP_SSL_CLOSE);
				SetNapiErrorCode(nRet);
				PubDebugSelectly(3, "NAPI_CloseSSLSocket error[%d]",nRet);
				return APP_FAIL;
			}
#if 0//
			nRet = _TcpWait(*pnHandle);
			if (TCP_OK != nRet)
			{
				SetCommErrorCode(FAIL_TCPIP_WAIT);
				SetNapiErrorCode(nRet);
				PubDebugSelectly(3, "_TcpWait error[%d]---[TcpHandle:%d]",nRet,*pnHandle);
				return APP_FAIL;
			}
#endif
			*pnHandle = 0;
		}
		break;
	case SSL_FORBIDDEN:
	default:
		if (*pnHandle != 0)
		{
			if((nFlag == HANGUP_RESET))
			{
				nRet = _TcpReset(*pnHandle);
				if (TCP_OK != nRet)
				{
					SetCommError(FAIL_TCPIP_CLOSE, nRet);
					PubDebugSelectly(3, "_TcpReset error[%d]---[TcpHandle:%d]",nRet,*pnHandle);
					return APP_FAIL;
				}
			}
			else
			{
				nRet = _TcpClose(*pnHandle);
				if (TCP_OK != nRet)
				{
					SetCommError(FAIL_TCPIP_CLOSE, nRet);
					PubDebugSelectly(3, "_TcpClose error[%d]---[TcpHandle:%d]",nRet,*pnHandle);
					return APP_FAIL;
				}

				PubDebugSelectly(2, "_TcpWait wait...");
				nRet = _TcpWait(*pnHandle);
				if (TCP_OK != nRet)
				{
					SetCommError(FAIL_TCPIP_WAIT, nRet);
					PubDebugSelectly(3, "_TcpWait error[%d]---[TcpHandle:%d]",nRet,*pnHandle);
					return APP_FAIL;
				}
			}
			*pnHandle = 0;
		}
		break;
	}

	PubDebugSelectly(3, "Tcp HangUp succ...");
	return APP_SUCC;
}

int TcpGetHandle(int nHandle, int *pnDestHandle)
{
	if (pnDestHandle == NULL)
		return APP_FAIL;

	*pnDestHandle = nHandle;

	return APP_SUCC;
}

#define DNS_TEMPFILE "FLDNS"
//DN
static int UpdateDns(const STSERVERADDRESS *pstServerAddress)
{
	int fp;
	int nRet = 0xff;
	uint nFileSize = 0;
	int nCount = 0;
	STSERVERADDRESS stServerAddress;

	if((fp = PubFsOpen(DNS_TEMPFILE, "w"))<0)
	{
		return APP_FAIL;
	}

	if (PubFsFileSize(DNS_TEMPFILE, &nFileSize) != NAPI_OK)
	{
		PubFsClose(fp);
		return APP_FAIL;
	}
    PubDebugSelectly(2, "UpdateDns ...[nFileSize:%d]", nFileSize);
	if (nFileSize != 0)
	{
		int i = 0;

		nCount = nFileSize/sizeof(STSERVERADDRESS);
        PubDebugSelectly(2, "UpdateDns ...[nCount:%d]", nCount);
		for(i = 0; i < nCount; i++)
		{
			nRet = PubFsRead(fp, (char *)&stServerAddress, sizeof(STSERVERADDRESS));
			if (nRet != sizeof(STSERVERADDRESS))
			{
				PubFsClose(fp);
				return APP_FAIL;
			}
			if (0 == memcmp(stServerAddress.szDN, pstServerAddress->szDN, sizeof(pstServerAddress->szDN)))
			{
				PubFsSeek(fp,  -sizeof(STSERVERADDRESS), SEEK_CUR);
				nRet = PubFsWrite(fp, (char *)pstServerAddress, sizeof(STSERVERADDRESS));
PubDebugSelectly(2, "UpdateDns ...write1");
				if (nRet != sizeof(STSERVERADDRESS))
				{
					PubFsClose(fp);
					return APP_FAIL;
				}
				PubFsClose(fp);
				return APP_SUCC;
			}

		}
	}
	nRet = PubFsWrite(fp, (char *)pstServerAddress, sizeof(STSERVERADDRESS));
PubDebugSelectly(2, "UpdateDns ...write2");
	if(nRet != sizeof(STSERVERADDRESS))
	{
		PubFsClose(fp);
		return APP_FAIL;
	}
	PubFsClose(fp);
	return APP_SUCC;
}

//dns,ip, APP_FAIL
static int ChkDnsRecord(const STSERVERADDRESS *pstServerAddress, char *pszIp)
{
	int fp;
	int nRet = 0xff;
	uint nFileSize = 0;
	int nCount = 0;
	STSERVERADDRESS stServerAddress;

	memset(&stServerAddress, 0, sizeof(STSERVERADDRESS));

	//
	if((fp = PubFsOpen(DNS_TEMPFILE, "r"))<0)
	{
		return APP_FAIL;
	}

	if (PubFsFileSize(DNS_TEMPFILE, &nFileSize) != NAPI_OK)
	{
		PubFsClose(fp);
		return APP_FAIL;
	}
	nCount = nFileSize/sizeof(STSERVERADDRESS);
    PubDebugSelectly(2, "ChkDnsRecord ...[nCount:%d]", nCount);
	if (nCount == 0)
	{
		//
		PubFsClose(fp);
		return APP_FAIL;
	}
	else
	{
		int i = 0;
		for(i = 0; i < nCount; i++)
		{
			nRet = PubFsRead(fp, (char *)&stServerAddress, sizeof(STSERVERADDRESS));
			if (nRet != sizeof(STSERVERADDRESS))
			{
				PubFsClose(fp);
				return APP_FAIL;
			}
            PubDebugSelectly(1, "[dnOld: %s][dnNew: %s]...[dnipOld:%s][dnipNew:%s]", stServerAddress.szDN, pstServerAddress->szDN
		    ,stServerAddress.szDNSIp, pstServerAddress->szDNSIp);
			if (memcmp(stServerAddress.szDN, pstServerAddress->szDN, sizeof(pstServerAddress->szDN)) != 0)
			{
				//dn
				PubFsClose(fp);
				nRet = PubFsDel(DNS_TEMPFILE);
				return APP_FAIL;
			}
			else
			{  //DN
				if (memcmp(stServerAddress.szDNSIp, pstServerAddress->szDNSIp, sizeof(pstServerAddress->szDNSIp)) != 0)
				{ //dns ip
					PubDebugSelectly(1, "DNS ip...[Ip:%s]", pszIp);
			#if 0
					PubDebugSelectly(3, "DNS ip...[Ip:%s]", pszIp);
					continue;
			#else
				//dns ip 20150313
				PubFsClose(fp);
				nRet = PubFsDel(DNS_TEMPFILE);
				return APP_FAIL;
			#endif
				}
				else
				{//dns ip
					if (pszIp != NULL)
						strcpy(pszIp, stServerAddress.lszIp[0]);
	PubDebugSelectly(2, "ChkDnsRecord succ...[Ip:%s]", pszIp);
					return APP_SUCC;
				}
			}
		}
		if (i == nCount)
		{	//nds ip
			return APP_FAIL;
		}
	}
	return APP_SUCC;
}

#define CFG_SERVER_STR "nameserver"
#define COPYMODE     0644
static int ProFileTouch(char *pszFilename)
{
    char error[1024] = {0};
    int fd;
    if(pszFilename!=NULL) {
        fd=creat(pszFilename,COPYMODE);
        if(-1 == fd) {
            if(errno != EISDIR) {
				PubDebugSelectly(3, "create file : %s error",pszFilename);
                perror(error);
                return -1;
            }
        } else {
            close(fd);
        }
    }
    return 0;
}

/**
* @brief dns
* @param [in] emCommType
* @param [in] pszDNSBuf dns
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author sunh
* @date 2015-7-3
*/
int CommAddDnsIp(NET_TYPE emCommType,char* pszDNSBuf)
{
	int dns_fd = 0, count = 0;
	char Tmpbuf[128], szWbuf[256] = {0};
	char *pTmp1, *pTmp2;
	char resolve_path[64]={0};

	switch(emCommType)
	{
	case ETH:
	    strcpy(resolve_path,"/tmp/eth_resolv.conf");
	    break;
	case WIFI:
	    strcpy(resolve_path,"/tmp/wifi_resolv.conf");
	    break;
	case WLM:
	    strcpy(resolve_path,"/tmp/wls_resolv.conf");
	    break;
	default:
	    return NAPI_ERR_NET_UNKNOWN_COMMTYPE;
	}

	pTmp1 = pszDNSBuf;
	if (strlen(pszDNSBuf) == 0)
	{
		return NAPI_ERR;
	}

	memset(szWbuf, 0, sizeof(szWbuf));
	while((pTmp2 = strchr(pTmp1, ';')) != NULL)
	{
		count++;
		if(count > 2)		/**<3DNS*/
			return NAPI_ERR;
		memset(Tmpbuf, 0, sizeof(Tmpbuf));
		memcpy(Tmpbuf, pTmp1, pTmp2-pTmp1);
		if(inet_addr(Tmpbuf) == INADDR_NONE)	/**<IP*/
			return NAPI_ERR_NET_ADDRILLEGAL;
		sprintf(szWbuf+strlen(szWbuf), "%s %s\n", CFG_SERVER_STR, Tmpbuf);
		pTmp1 = pTmp2+1;
	}
	memset(Tmpbuf, 0, sizeof(Tmpbuf));
	strcpy(Tmpbuf, pTmp1);
	if(inet_addr(Tmpbuf) == INADDR_NONE)		/**<IP*/
		return NAPI_ERR_NET_ADDRILLEGAL;
	sprintf(szWbuf+strlen(szWbuf), "%s %s\n", CFG_SERVER_STR, Tmpbuf);

	/**<*/
	if (ProFileTouch(resolve_path)!=0)
	{
		PubDebugSelectly(3, "ProFileTouch fail:");
        return NAPI_ERR;
	}
	if ((dns_fd=PubFsOpen(resolve_path, "w")) < 0)
	{
		PubDebugSelectly(3, "open fail:");
        return NAPI_ERR;
    }
	if (PubFsWrite(dns_fd, szWbuf, strlen(szWbuf)) < strlen(szWbuf))
	{
		PubFsClose(dns_fd);
		PubDebugSelectly(3, "write fail:");
        return NAPI_ERR;
    }

	PubFsClose(dns_fd);
	return NAPI_OK;
}


//EM_COMM_TYPE
int ParseDnsIp(char cCommType, const STSERVERADDRESS *pstServerAddress, int *pnTimeout, char *pszIp, char cIsQuery)
{
	uint nBeginTime = 0, nRet;
	STSERVERADDRESS stServerAddress;
	char szIp[16+1] = {0};

	if (pszIp == NULL)
	{
		return APP_FAIL;
	}

	PubDebugSelectly(3, "ParseDnsIp start...[dn:%s][dnsIp:%s][ischeck:%d][type: %d]", pstServerAddress->szDN, pstServerAddress->szDNSIp, cIsQuery, cCommType);

	if (YES == cIsQuery)
	{
		if (ChkDnsRecord((const STSERVERADDRESS *)pstServerAddress, pszIp) == APP_SUCC)
		{
			PubDebugSelectly(3, "ParseDnsIp ChkDnsRecord succ...[ip:%s]", pszIp);
			return APP_SUCC;
		}
	}

	nBeginTime = PubGetOverTimer(0);

    CommAddDnsIp((NET_TYPE)cCommType, (char *)pstServerAddress->szDNSIp);

    nRet = NAPI_NetPerformNSLookUp((NET_TYPE)cCommType, (char *)pstServerAddress->szDN, szIp);
	if (nRet != NAPI_OK)
	{
		SetCommError(FAIL_TCPIP_GET_DN_ADDRESS, nRet);
		PubDebugSelectly(3, "NAPI_NetDnsResolv error[%d]", nRet);
		return APP_FAIL;
	}

	if (strlen(szIp) != 0)
	{
		strcpy(pszIp, szIp);
	}
	else
	{
		return APP_FAIL;
	}

	*pnTimeout -= GetTimeDif(nBeginTime, PubGetOverTimer(0));
	if (*pnTimeout <= 0)
	{
		SetCommError(APP_TIMEOUT, 0);
		return APP_TIMEOUT;
	}
	memcpy(&stServerAddress, pstServerAddress, sizeof(STSERVERADDRESS));
	strcpy(stServerAddress.lszIp[0], pszIp);
	strcpy(stServerAddress.lszIp[1], pszIp);
	UpdateDns((const STSERVERADDRESS *)&stServerAddress);
	PubDebugSelectly(3, "ParseDnsIp succ...[ip:%s]", pszIp);
	return APP_SUCC;
}


/**
* @brief conctrol led filck
* @return
* @li
*/
void SetLedComFlick()
{
#if 0
	static int snSupport = 1;
	ST_LED_FLICK stFlickParam;

	if(snSupport == NAPI_ERR_NO_DEVICES)
	{/*<POS*/
		return;
	}
	if(snSupport == 1)
	{
		stFlickParam.unFlickOn = 5;
		stFlickParam.unFlickOff = 5;
		NAPI_LedSetFlickParam(LED_COM_FLICK, stFlickParam);
	}
	if(NAPI_LedStatus(LED_COM_FLICK) != NAPI_OK)
	{/*<POS*/
		snSupport = NAPI_ERR_NO_DEVICES;
	}
#endif
}


STLEDFLAG gstLedFlag;
void SetCommLedFlag()
{
	return; // no support
#if 0
	memset(&gstLedFlag, 0, sizeof(STLEDFLAG));

	if (NAPI_OK == NAPI_LedStatus(LED_COM_OFF))
	{
		gstLedFlag.cLedComm = 1;
	}

	if (NAPI_OK == NAPI_LedStatus(LED_ONL_OFF))
	{
		gstLedFlag.cLedOnline = 1;
	}
#endif
}

void CommserverLedShine(EM_LED emStatus)
{
	//Q_UNUSED(emStatus)
	return; // no support
#if 0
	switch(emStatus)
	{
	case LED_COM_ON:
	case LED_COM_OFF:
	case LED_COM_FLICK:
		if (1 == gstLedFlag.cLedComm)
		{
			NAPI_LedStatus(emStatus);
		}
		break;
	case LED_ONL_ON:
	case LED_ONL_OFF:
	case LED_ONL_FLICK:
		if (1 == gstLedFlag.cLedOnline)
		{
			NAPI_LedStatus(emStatus);
		}
		break;
	default:
		break;
	}
#endif
}


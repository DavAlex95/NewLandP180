/**
* @file process.h
* @brief process module
* @version  1.0
* @author Liug
* @date 2012-06-04
*/
#ifndef _PROCESS_H_
#define _PROCESS_H_


typedef struct {
	STCOMMPARAM stCommParam;
	char cIsInitComm;
	char cIsConnect;
	int  nConnectIndex;
	int  nErrorCode;
	int  nNAPIErrorCode;
	int (*pClear)(void);
	int (*pConnect)(const STSERVERADDRESS *,int *,int, int);
	int (*pGetConnectState)(void);
	int (*pWrite)(const char *, int);
	int (*pRead)(int,int,int,char *);
	int (*pHangUp)(int);
}STCOMM;

/**
* @brief Set communication error code
* @param [in]  nError error code
* @return
* @author Liug
* @date 2012-6-6
*/
void SetCommErrorCode(int nError);

/**
* @brief Get communication error code
* @return error code
* @author Liug
* @date 2012-6-6
*/
int GetCommErrorCode(void);

void SetNapiErrorCode(int nError);
int GetNapiErrorCode(void);

/**
* @brief initialize communication
* @param
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author Liug
* @date 2012-6-5
*/
int InitComm(void);


/**
* @brief Connecting, If CommType1 is failed, use CommType2
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author Jack
* @date 2019-1-19
*/
int SwitchCommType();

/**
* @brief Connecting
* @param [in] cPreDial
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author Liug
* @date 2012-6-5
*/
int CommConnect(char cPreDial);

/**
* @brief Connection
* @param
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author Liug
* @date 2012-6-5
*/
int ProCommConnect(char cPreDial);

/**
* @brief Clear buffer
* @param [in]  nFlag
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author Liug
* @date 2012-6-7
*/
int CommClearBuf(void);

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
int CommSend(const char *psData,int nDataLen);

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
int CommRecv(int CommReadType, int nNeedLen,char *psOutData,int *pnOutLen);

/**
* @brief Communication hangup
* @param [in]   nFlag
* @return
* @li APP_SUCC
* @li Fail
* @author Liug
* @date 2012-5-23
*/
int CommHungUp(int nFlag);

/**
* @brief Get communication index
* @param [out]   pnConnectIndex
* @return
* @li APP_SUCC
* @li Fail
* @author Liug
* @date 2012-6-19
*/
int GetCommConnectIndex(int *pnConnectIndex);

/**
* @brief Get socket handle
* @param [out]   pnHandle
* @return
* @li APP_SUCC
* @li Fail
* @author sunh
* @date 2014-1-1
*/
int GetCommTcpHandle(int *pnHandle);

int SetSslMode(STSSLMODE *pstSslMode);
int GetSslMode(STSSLMODE *pstSslMode, char *pcIsCustomized);

int GetSslCertMsg(STSSLCERTMSG* pstSslCertMsg);
int GetVarCommMode();
int CommPppDial();
int CommParseDn(const char *pszDn, const char *pszDnsIp, char *pszIp, char cIsQuery);
void SetCommError(int nCommErr, int nNapiErr);
int CommNetStatusCheck(int *pnStatus);
int CommGetNetAddr(STETHPARAM  *pstEthParam);
int ProCommRecv(int CommReadType, int nNeedLen,char *psOutData,int *pnOutLen);
int ProCommSend(const char *psData, int nDataLen);

int CommSetSuspend(uint unFlag);

#endif /* _PROCESS_H_ */


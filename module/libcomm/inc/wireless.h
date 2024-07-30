/**
* @file wireless.h
* @brief Wireless module
* @version
*	v2.0	chenxiulin	2019-07-24
*	v1.0	Liug		2012-06-07
*/
#ifndef _WIRELESS_H_
#define _WIRELESS_H_
#include "lcomm.h"
/**
* @brief New Wireless Clear buffer
* @param
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author chenxiulin
* @date 2019-07-24
*/
int WirelessClearBuf(void);

/**
* @brief Get connect status
* @param
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author chenxiulin
* @date 2019-07-24
*/
int WirelessGetConnectState(void);

/**
* @brief New Wireless init
* @param [in] pstWLParam
* @param [in] cSsl
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author chenxiulin
* @date 2019-07-24
*/
int WirelessInit(STGPRSPARAM *pstGprsParam, STCDMAPARAM *pstCdmaParam, char cSsl);


/**
* @brief New Wireless Get Connect States
* @param [in] pnStatus
* @param [in] cSsl
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author chenxiulin
* @date 2019-07-24
*/
int WirelessConnectState(int *pnStatus);

/**
* @brief New Wireless hangUp
* @param [in] nFlag (EM_HANGUPMODE)
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author chenxiulin
* @date 2019-07-24
*/
int WirelessHangUp(int nFlag);

/**
* @brief New Wireless connection
* @param [in] pstServerAddress
* @param [out] pnConnectIndex
* @param [in] nTimeOut
* @param [in] nIsPreConnect
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author chenxiulin
* @date 2019-07-24
*/
int WirelessConnect(const STSERVERADDRESS *pstServerAddress,int *pnConnectIndex, int nTimeOut, int nIsPreConnect);

/**
* @brief New Wireless Sending data
* @param [in]  psData
* @param [in]  nDataLen
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author chenxiulin
* @date 2019-07-24
*/
int WirelessWrite(const char *psData,int nDataLen);

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
* @author chenxiulin
* @date 2019-07-24
*/
int WirelessRead(int nIsNonBlock, int nLen,int nTimeOut,char *psOutData);

/**
* @brief Get TCP Handle
* @param [out]  pnHandle
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author chenxiulin
* @date 2019-07-24
*/
int WirelessGetTcpHandle(int* pnHandle);

/**
* @brief Reset index
* @param
* @return
* @author chenxiulin
* @date 2019-07-24
*/
void WirelessResetIndex(void);

/**
* @brief Update Ssl
* @param
* @return
* @author chenxiulin
* @date 2019-07-24
*/
void WirelessUpdateSslFlag(char cSsl);

/**
* @brief ppp dial
* @param
* @return
* @author chenxiulin
* @date 2019-07-24
*/
int WirelessPppDial(int nTimeOut);

#endif /* _WIRELESS_H_ */

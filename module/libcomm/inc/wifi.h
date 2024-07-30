/**
* @file wireless.h
* @brief WIFI module
* @version
*	v2.0 chenxiulin  2019-07-16
*	v1.0 Liug  2012-06-04
*/
#ifndef _WIFI_H_
#define _WIFI_H_
#include "napi.h"
#include "napi_wifi.h"
#include "lcomm.h"
/**
* @brief Wifi initialize
* @param [in] pstWirelessParam
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author chenxiulin
* @date 2019-07-16
*/
int WifiInit(STWIFIPARAM *pstWifiParam, char cSsl);

/**
* @brief New Wifi set signal value (see EM_WIFI_SIGNAL_T)
* 		1> this function will be called Automatically by NWifiSignalCallback (Called by multithreading in the library)
		2> User calls it to initialize to 0 (eg: NWifiSetSignalVal(0))
* @param [in] emSignal
* @return
* @author chenxiulin
* @date 2019-07-16
*/
void WifiSetSignalVal(EM_WIFI_SIGNAL_T emSignal);

/**
* @brief New Wifi get signal Value (see EM_WIFI_SIGNAL_T)
* @param
* @return
* @li EM_WIFI_SIGNAL_T : currunt wifi signal
* @author chenxiulin
* @date 2019-07-16
*/
EM_WIFI_SIGNAL_T NWifiGetSignalVal(void);

/**
* @brief New Wifi enable wifi
* @param[in] enable : 0. close  1. open
* @param[in] nWait : FALSE: no wait.
*                TRUE:  wait the wifi state ENABLE or DISABLE (blocking)
* @return
* @li APP_FAIL : use GetCommErrorCode(),GetNapiErrorCode() to get Error
* @li APP_SUCC
* @author chenxiulin
* @date 2019-07-16
*/
int WifiSetEnable(EM_WIFI_ENABLE enable, int nWait);

/**
* @brief New Wifi get tcp handle
* @param [out] pnHandle : Not NULL
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author chenxiulin
* @date 2019-07-16
*/
int WifiGetTcpHandle(int *pnHandle);

/**
* @brief New wifi connection
* @param [in] pstServerAddress
* @param [in] pnConnectIndex
* @param [in] nTimeOut：Recommended more than 30
* @param [out] nIsPreConnect
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author chenxiulin
* @date 2019-07-16
*/
int WifiConnect(const STSERVERADDRESS *pstServerAddress,int *pnConnectIndex,int nTimeOut, int nIsPreConnect);

/**
* @brief CLear New wifi data
* @param
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author chenxiulin
* @date 2019-07-16
*/
int WifiClearBuf(void);

/**
* @brief Get wifi connection status
* @param
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author chenxiulin
* @date 2019-07-16
*/
int WifiGetConnectState(void);

/**
* @brief New WIFI Hangup
* @param [in] nFlag
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author chenxiulin
* @date 2019-07-16
*/
int WifiHangUp(int nFlag);

/**
* @brief New WIFI scan
* @param [out] pstList : scan results
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author chenxiulin
* @date 2019-07-16
*/
int WifiScan(ST_WIFI_AP_LIST *pstList);

/**
* @brief Sending data
* @param [in]  psData
* @param [in]  nDataLen
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author chenxiulin
* @date 2019-07-16
*/
int WifiWrite(const char *psData, int nDataLen);

/**
* @brief Receiving data
* @param [in]   nLen
* @param [in]   nTimeOut
* @param [out]  psOutData
* @return
* @li >0
* @li Fail
* @li TIMEOUT
* @author chenxiulin
* @date 2019-07-16
*/
int WifiRead(int nIsNonBlock,int nLen,int nTimeOut,char *psOutData);

/**
* @brief New WIFI set parameter
* @param [in] pstDialParam
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author chenxiulin
* @date 2019-07-16
*/
int WifiSetParam(STWIFIPARAM *pstWifiParam);

/**
* @brief New WIFI set ssl
* @param [in] cSsl
* @return
* @author chenxiulin
* @date 2019-07-16
*/
void WifiUpdateSslFlag(char cSsl);

/**
* @brief New WIFI reset socket index
* @param
* @return
* @author chenxiulin
* @date 2019-07-16
*/
void WifiResetIndex(void);

/**
* @brief New WIFI connect
* @param [in] nTimeOut
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author chenxiulin
* @date 2019-07-16
*/
int WifiSsidConnect(int nTimeOut);

/**
* @brief New Wifi ConnectState
* @param [out] pnStatus EM_WIFI_CONNECT_STATUS
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author chenxiulin
* @date 2019-07-16
*/
int WifiConnectState(int *pnStatus);

/**
* @brief get current wifi connect information
* @param [out] ST_WIFI_INFO_T
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author lingdz
* @date 2019-12-19
*/
int WifiGetCurrentInfo(ST_WIFI_INFO_T *pstCurrWifiinfo);

#endif /* _WIFI_H_ */

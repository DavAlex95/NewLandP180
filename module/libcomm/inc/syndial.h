#ifndef _SYNDIAL_H_
#define _SYNDIAL_H_
/**
* @brief Syn dial initialize
* @param [in] pstDialParam
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author Liug
* @date 2012-6-18
*/
int SynDialInit(STDIALPARAM *pstDialParam);

/**
* @brief Syn connect
* @param [in] pstServerAddress
* @param [out] pnConnectIndex
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author Liug
* @date 2012-6-7
*/
int SynDialConnect(const STSERVERADDRESS * pstServerAddress,int *pnConnectIndex,int nTimeout, int nIsPreConnect);

/**
* @brief Get the dialing status
* @param
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author Liug
* @date 2012-11-15
*/
int SynDialGetConnectState();

/**
* @brief Clear buffer
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author Liug
* @date 2012-6-7
*/
int SynDialClearBuf();

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
int SynDialWrite(const char *psData,int nDataLen);

/**
* @brief Receiving data
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
int SynDialRead(int nIsNonBlock,int nLen,int nTimeOut,char *psOutData);


/**
* @brief Hangup
* @param [in] nFlag
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author Liug
* @date 2012-6-18
*/
int SynDialHangUp(int nFlag);
void SynResetIndex();
int SynDialSetParam(STDIALPARAM *pstDialParam);

#endif


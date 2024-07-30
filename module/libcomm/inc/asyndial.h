/**
* @brief Asyn Dial Init
* @param [in] pstDialParam
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author
* @date 2012-7-18
*/
int AsynDialInit(STDIALPARAM *pstDialParam);

/**
* @brief Clear buffer of asyn
* @param
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author
* @date 2012-7-18
*/
int AsynDialClearBuf();

/**
* @brief Asyn connection
* @param [in] pstServerAddress
* @param [out] pnConnectIndex    conncection index
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author
* @date 2012-7-18
*/
int AsynDialConnect(const STSERVERADDRESS * pstServerAddress,int *pnConnectIndex,int nTimeOut, int nIsPreConnect);

/**
* @brief Get connection status
* @param
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author
* @date 2012-11-15
*/
int AsynDialGetConnectState();

/**
* @brief Sending
* @param [in]  psData
* @param [in]  nDataLen
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author
* @date 2012-7-18
*/
int AsynDialWrite(const char *psData,int nDataLen);

/**
* @brief Receiving
* @param [in]   nLen          Experted data length
* @param [in]   nTimeOut
* @param [out]  psOutData
* @return
* @li >0      Actual data reveived
* @li Fail
* @li TIMEOUT
* @author
* @date 2012-7-18
*/
int AsynDialRead(int nIsNonBlock,int nLen,int nTimeOut,char *psOutData);


/**
* @brief Hangup
* @param [in] nFlag
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author
* @date 2012-7-18
*/
int AsynDialHangUp(int nFlag);
void AsynResetIndex();
int AsynDialSetParam(const STDIALPARAM *pstDialParam);

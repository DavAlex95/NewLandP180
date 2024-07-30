/**
* @file eth.h
* @brief eth module
* @version  1.0
* @author Linw
* @date 2012-06-19
*/
#ifndef _ETH_H_
#define _ETH_H_
#include "lcomm.h"
/**
* @brief Ethernet initialize, set the IPDNS
* @param [in] pstEthParam   
* @return 
* @li APP_FAIL 
* @li APP_SUCC 
* @author Linw
* @date 2012-06-19
*/
int EthInit(STETHPARAM *pstEthParam, char cSsl);

/**
* @brief Ehernet connect
* @param [in] pstServerAddress   
* @return 
* @li APP_FAIL 
* @li APP_SUCC 
* @author Linw
* @date 2012-06-19
*/
int EthConnect(const STSERVERADDRESS *pstServerAddress, int *pnConnectIndex,int nTimeOut, int nIsPreConnect);

/**
* @brief Get network connection status
* @param
* @return 
* @li APP_FAIL 
* @li APP_SUCC 
* @author Liug
* @date 2012-11-15
*/
int EthGetConnectState();

/**
* @brief Ethernet send
* @param [in]  psData        
* @param [in]  nDataLen      
* @return 
* @li APP_FAIL 
* @li APP_SUCC 
* @author Linw
* @date 2012-06-19
*/
int EthWrite(const char *psData,int nDataLen);

/**
* @brief Ehernet receive
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
int EthRead(int nIsNonBlock,int nLen,int nTimeOut,char *psOutData);

/**
* @brief Ethernet hangup
* @return 
* @li APP_FAIL 
* @li APP_SUCC 
* @author Linw
* @date 2012-06-19
*/
int EthHangUp(int nFlag);


/**
* @brief Clear buffer
* @param 
* @return 
* @li APP_FAIL 
* @li APP_SUCC 
* @author Linw
* @date 2012-6-7
*/
int EthClearBuf();

int EthGetTcpHandle(int *pnHandle);
void EthResetIndex();
void EthUpdateSslFlag(char cSsl);

#endif


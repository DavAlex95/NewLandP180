/**
* @file port.h
* @brief Port module
* @version  1.0
* @author Liug
* @date 2012-06-04
*/
#ifndef _PORT_H_
#define _PORT_H_

/**
* @brief Port initialize
* @param [in] pstPortParam   
* @return 
* @li APP_FAIL 
* @li APP_SUCC 
* @author Liug
* @date 2012-6-5
*/
int PortInit(STPORTPARAM *pstPortParam);

/**
* @brief Clear buffer
* @param 
* @return 
* @li APP_FAIL 
* @li APP_SUCC 
* @author Liug
* @date 2012-6-5
*/
int PortClearBuf();

/**
* @brief Port connect
* @param [in] pstServerAddress   
* @param [out] pnConnectIndex    
* @return 
* @li APP_FAIL 
* @li APP_SUCC 
* @author Liug
* @date 2012-6-5
*/
int PortConnect(const STSERVERADDRESS * pstServerAddress,int *pnConnectIndex,int nTimeOut, int nIsPreConnect);

/**
* @brief Get connection status
* @param 
* @return 
* @li APP_FAIL 
* @li APP_SUCC 
* @author Liug
* @date 2012-11-15
*/
int PortGetConnectState();

/**
* @brief Write to port
* @param [in]  psData        
* @param [in]  nDataLen      
* @return 
* @li APP_FAIL 
* @li APP_SUCC 
* @author Liug
* @date 2012-5-23
*/
int PortWrite(const char *psData,int nDataLen);

/**
* @brief Read from port
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
int PortRead(int nIsNonBlock,int nLen,int nTimeOut,char *psOutData);

/**
* @brief Hangup
* @param [in] nFlag 
* @return 
* @li APP_FAIL 
* @li APP_SUCC 
* @author Liug
* @date 2012-6-5
*/
int PortHangUp(int nFlag);

#endif

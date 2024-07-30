#ifndef _LINUXSOCKET_H_
#define _LINUXSOCKET_H_

typedef enum
{
	TCP_OK = 0,
	TCP_ERR = -1,
	TCP_ERR_PARAM = -2,
	TCP_ERR_ALLOC = -3,
	TCP_ERR_TIMEOUT = -4,
	TCP_ERR_INVADDR = -5,
	TCP_ERR_CONNECT = -6,
	TCP_ERR_PROTOCOL = -7,
	TCP_ERR_NETWORK = -8,
	TCP_ERR_SEND = -9,
	TCP_ERR_RECV = -10,

	TCP_ERR_LINUX_ERRNO_BASE = -5000 /* system error no*/
}TCP_ERR_CODE;


TCP_ERR_CODE _TcpOpen(uint * punFd);

/**
 *@brief	Close TCP channel
 *@param[in] unFd	TCP channel handle
 *@return
 *@li \ref TCP_ERR_CODE "TCP_OK" Success
 *@li \ref TCP_ERR_CODE others fail
*/
TCP_ERR_CODE _TcpClose(uint unFd);

/**
 *@brief	 Wait for TCP to be closed
 *@details  Call this function after \ref _TcpClose "_TcpClose" to ensure TCP link is closed completely
 *@param[in] unFd	TCP channel handle
 *@return
 *@li \ref TCP_ERR_CODE "TCP_OK" Success
 *@li \ref TCP_ERR_CODE others fail
*/
TCP_ERR_CODE _TcpWait(uint unFd);

/**
 *@brief	Connect
 *@param[in] unFd	TCP channel handle
 *@param[in] pszRemoteIp	Remote address
 *@param[in] usRemotePort	Remote port
 *@param[in] unTimeout	Timeout in seconds
 *@return
 *@li \ref TCP_ERR_CODE "TCP_OK"  				Success
 *@li \ref TCP_ERR_CODE others fail
*/
TCP_ERR_CODE _TcpConnect(uint unFd, const char *pszRemoteIp, ushort usRemotePort, uint unTimeout);

/**
 *@brief	 Send data
 *@param[in] unFd	TCP channel handle
 *@param[in] pvInbuf	Send buffer
 *@param[in] unLen	Length of data sent
 *@param[in] unTimeout	Timeout in seconds
 *@param[out]	punWriteLen	Actual length sent
 *@return
 *@li \ref TCP_ERR_CODE "TCP_OK"  				Success
 *@li \ref TCP_ERR_CODE others fail
*/
TCP_ERR_CODE _TcpWrite(uint unFd, const void *pvInbuf, uint unLen, uint unTimeout, uint *punWriteLen);

/**
 *@brief	 Receive data
 *@param[in] unFd	TCP channel handle
 *@param[in] unLen	Length of data to be received
 *@param[in] unTimeout	Timeout in seconds
 *@param[out]	pvOutBuf	Buffer to save data
 *@param[out]	punReadLen	 Actual length sent
 *@return
 *@li \ref TCP_ERR_CODE "TCP_OK" Success
 *@li \ref TCP_ERR_CODE others fail
*/
TCP_ERR_CODE _TcpRead(uint unFd, void *pvOutBuf, uint unLen, uint unTimeout, uint *punReadLen);

/**
 *@brief     Shut down TCP channel
 *@param[in]   unFd   TCP channel handle
 *@return
 *@li \ref TCP_ERR_CODE "TCP_OK" Success
 *@li \ref TCP_ERR_CODE others fail
*/
TCP_ERR_CODE _TcpReset(uint unFd);

#endif /* _LINUXSOCKET_H_ */

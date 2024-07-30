/*
* Newland payment(c) 2011-2015
* NAPI 
* Date	2012-08-17
*/

#ifndef __SSLSOCKETS__H
#define __SSLSOCKETS__H

#include <stdio.h>
#include <time.h>
#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void *SSL_HANDLE; /**<File descriptor*/


typedef enum
{
	NAPI_NOWAIT = 0,						
	NAPI_SUSPEND = 0xFFFFFFFF,
}EM_BLOCK_OPT;

typedef struct st_socket_addr
{
	uint  unAddrType;
	char *psAddr;							// IP ASCIIz
	ushort usPort;							// IP
}ST_SOCKET_ADDR;

typedef enum
{
	HANDSHAKE_SSLv2,	//!< SSLv2
    HANDSHAKE_SSLv23, 	//!< SSLv3 SSLv2
	HANDSHAKE_SSLv3,	//!< SSLv3
	HANDSHAKE_TLSv1,	//!< TLSv1
    HANDSHAKE_TLSv1_1,	//!< TLSv1.1
    HANDSHAKE_TLSv1_2,	//!< TLSv1.2
}EM_SSL_HANDSHAKE_OPT;

typedef enum
{
    SSL_AUTH_NONE,					
	SSL_AUTH_CLIENT,								
}EM_SSL_AUTH_OPT;

typedef enum
{
        SSL3_CIPHER_RSA_NULL_MD5 = 1,			///< -RSA_NULL_MD5
        SSL3_CIPHER_RSA_NULL_SHA,				///< -RSA_NULL_SHA
        SSL3_CIPHER_RSA_RC4_40_MD5,				///< -RSA_RC4_40_MD5
        SSL3_CIPHER_RSA_RC4_128_MD5,			///< -RSA_RC4_128_MD5
        SSL3_CIPHER_RSA_RC4_128_SHA,			///< -RSA_RC4_128_SHA
        SSL3_CIPHER_RSA_RC2_40_MD5,				///< -RSA_RC2_40_MD5
        SSL3_CIPHER_RSA_IDEA_128_SHA,			///< -IDEA_128_SHA
        SSL3_CIPHER_RSA_DES_40_CBC_SHA,			///< -DES_40_CBC_SHA
        SSL3_CIPHER_RSA_DES_64_CBC_SHA,			///< -DES_64_CBC_SHA
        SSL3_CIPHER_RSA_DES_192_CBC3_SHA,		///< -DES_192_CBC3_SHA
        SSL3_CIPHER_DH_RSA_DES_192_CBC3_SHA,	///< -DH_DSS_DES_192_CBC3_SHA
        SSL3_CIPHER_DH_DSS_DES_40_CBC_SHA,		///< -DH_DSS_DES_40_CBC_SHA
        SSL3_CIPHER_DH_DSS_DES_64_CBC_SHA,		///< -DH_DSS_DES_64_CBC_SHA
        SSL3_CIPHER_DH_DSS_DES_192_CBC3_SHA,	///< -DH_DSS_DES_192_CBC3_SHA
        SSL3_CIPHER_DH_RSA_DES_40_CBC_SHA,		///< -DH_RSA_DES_40_CBC_SHA
        SSL3_CIPHER_DH_RSA_DES_64_CBC_SHA,		///< -DH_RSA_DES_64_CBC_SHA
        SSL3_CIPHER_EDH_DSS_DES_40_CBC_SHA,		///< -EDH_DSS_DES_40_CBC_SHA
        SSL3_CIPHER_EDH_DSS_DES_64_CBC_SHA,		///< -EDH_DSS_DES_64_CBC_SHA
        SSL3_CIPHER_EDH_DSS_DES_192_CBC3_SHA,	///< -EDH_DSS_DES_192_CBC3_SHA
        SSL3_CIPHER_EDH_RSA_DES_40_CBC_SHA,		///< -EDH_RSA_DES_40_CBC_SHA
        SSL3_CIPHER_EDH_RSA_DES_64_CBC_SHA,		///< -EDH_RSA_DES_64_CBC_SHA
        SSL3_CIPHER_EDH_RSA_DES_192_CBC3_SHA,	///< -EDH_RSA_DES_192_CBC3_SHA
        SSL3_CIPHER_ADH_RC4_40_MD5,				///< -ADH_RC4_40_MD5
        SSL3_CIPHER_ADH_RC4_128_MD5,			///< -ADH_RC4_128_MD5
        SSL3_CIPHER_ADH_DES_40_CBC_SHA,			///< -ADH_DES_40_CBC_SHA
        SSL3_CIPHER_FZA_DMS_NULL_SHA,			///< -FZA_DMS_NULL_SHA
        SSL3_CIPHER_CK_FZA_DMS_FZA_SHA,			///< -CK_FZA_DMS_FZA_SHA
        SSL3_CIPHER_CK_FZA_DMS_RC4_SHA,			///< -CK_FZA_DMS_RC4_SHA
        SSL3_CIPHER_CK_ADH_DES_64_CBC_SHA,		///< -CK_ADH_DES_64_CBC_SHA
        SSL3_CIPHER_CK_ADH_DES_192_CBC_SHA,		///< -CK_ADH_DES_192_CBC_SHA
}EM_SSL_CIPHER_OPT;

typedef enum
{
    SSL_IS_DISCONNECTED,
    SSL_CONNECTION_IN_PROGRESS,
    SSL_IS_CONNECTED
}EM_SSL_CONNECTION_STATE;

typedef enum
{
    SSL_FILE_DER,		/**<DER(ASN1) */
    SSL_FILE_PEM,		/**<PEM (BASE64) */
}EM_SSL_FILE_FORMAT;

typedef enum
{
	SSL_ADDR_IPV4,		/**<IP v4*/
    SSL_ADDR_IPV6,		/**<IP v6*/
}EM_ADDR_TYPE;

/**
 *@brief	load local Client Certificate
 *@param  handle  SSL handle
 *@param  psFileName  certificate name
 *@param  nFormat     certificate format(ref EM_SSL_FILE_FORMAT "EM_SSL_FILE_FORMAT")
 *@return
 *@li	NAPI_OK
 *@li	\ref NAPI_ERR_SSL_PARAM "NAPI_ERR_SSL_PARAM"	(SSL unopen/psFileName is NULL)
 *@li	\ref NAPI_ERR_SSL_ALREADCLOSE "NAPI_ERR_SSL_ALREADCLOSE"	ssl connection close
 *@li	\ref NAPI_ERR_SSL_MODEUNSUPPORTED "NAPI_ERR_SSL_MODEUNSUPPORTED"	mode unsupport(format is not supported)
 *@li	\ref NAPI_ERR "NAPI_ERR"
*/
int _LoadClientCertificate_(SSL_HANDLE handle, const char *psFileName, int nFormat);
/**
 *@brief	Load client private key
 *@param  handle  SSL handle
 *@param  psFileName  file name
 *@param  nFormat    file format(ref EM_SSL_FILE_FORMAT "EM_SSL_FILE_FORMAT")
 *@param  pszPassword  NULL
 *@return
 *@li	NAPI_OK
 *@li	\ref NAPI_ERR_SSL_PARAM "NAPI_ERR_SSL_PARAM"	(SSL unopen/psFileName is NULL)
 *@li	\ref NAPI_ERR_SSL_ALREADCLOSE "NAPI_ERR_SSL_ALREADCLOSE"	ssl connection close
 *@li	\ref NAPI_ERR_SSL_MODEUNSUPPORTED "NAPI_ERR_SSL_MODEUNSUPPORTED"	mode unsupport(format is not supported)
 *@li	\ref NAPI_ERR "NAPI_ERR"
*/
int _LoadClientPrivateKey_(SSL_HANDLE handle, const char *psFileName, int nFormat,char *pszPassword);
/**
 *@brief	Load CA file
 *@param  handle  SSL handle
 *@param  psFileName  CA file name
 *@param  nFormat    CA format(ref EM_SSL_FILE_FORMAT "EM_SSL_FILE_FORMAT")
 *@return
 *@li	NAPI_OK
 *@li	\ref NAPI_ERR_SSL_PARAM "NAPI_ERR_SSL_PARAM"	(SSL unopen/psFileName is NULL)
 *@li	\ref NAPI_ERR_SSL_ALREADCLOSE "NAPI_ERR_SSL_ALREADCLOSE"	ssl connection close
 *@li	\ref NAPI_ERR_SSL_MODEUNSUPPORTED "NAPI_ERR_SSL_MODEUNSUPPORTED"	mode unsupport(format is not supported)
 *@li	\ref NAPI_ERR "NAPI_ERR"
*/
int _LoadServerCertificate_(SSL_HANDLE handle, const char *psFileName, int nFormat);

/**
 *@brief	Create SSL socket
 *@param	nType		Handshake protocol type (ref EM_SSL_HANDSHAKE_OPT "EM_SSL_HANDSHAKE_OPT")
 *@param  	nAuthOpt 	Auth option(ref EM_SSL_AUTH_OPT "EM_SSL_AUTH_OPT")
 *@param  	pnCipher   	Supported cipher(ref EM_SSL_CIPHER_OPT "EM_SSL_CIPHER_OPT")
 *@return
 *@li	SSL handle
 *@li	NULL
*/
SSL_HANDLE _OpenSSLSocket_(int nType,int nAuthOpt,int* pnCipher);
/**
 *@brief	Close SSL socket
 *@param  	handle  SSL handle
 *@return
 *@li	NAPI_OK
 *@li	\ref NAPI_ERR_SSL_PARAM "NAPI_ERR_SSL_PARAM"	(SSL)
 *@li	\ref NAPI_ERR_SSL_ALREADCLOSE "NAPI_ERR_SSL_ALREADCLOSE"	(SSL)
 *@li	\ref NAPI_ERR "NAPI_ERR"	SSL
*/
int _CloseSSLSocket_(SSL_HANDLE handle);
/**
 *@brief	SSL
 *@param  	handle  SSL handle
 *@return
 *@li	NAPI_OK
 *@li	\ref NAPI_ERR_SSL_PARAM "NAPI_ERR_SSL_PARAM"	(SSL)
 *@li	\ref NAPI_ERR_SSL_ALREADCLOSE "NAPI_ERR_SSL_ALREADCLOSE"	(SSL)
 *@li	\ref NAPI_ERR "NAPI_ERR"	SSL
*/
int _SSLDisconnect_(SSL_HANDLE handle);
/**
 *@brief	SSL connect
 *@param  	handle  	SSL handle
 *@param  	pstServer  	Server address
 *@param  	nTimeOut  	ms
 *@return
 *@li	NAPI_OK
 *@li	\ref NAPI_ERR_SSL_PARAM "NAPI_ERR_SSL_PARAM"	(SSLpServerNULLnTimeOut)
 *@li	\ref NAPI_ERR_SSL_ALREADCLOSE "NAPI_ERR_SSL_ALREADCLOSE"	(SSL)
 *@li	\ref NAPI_ERR_SSL_ALLOC "NAPI_ERR_SSL_ALLOC"	()
 *@li	\ref NAPI_ERR "NAPI_ERR"
 *@li	\ref NAPI_ERR_SSL_TIMEOUT "NAPI_ERR_SSL_TIMEOUT"
 *@li	\ref NAPI_ERR_SSL_INVADDR "NAPI_ERR_SSL_INVADDR"
*/
int _SSLConnect_(SSL_HANDLE handle, ST_SOCKET_ADDR *pstServer, int nTimeOut);
/**
 *@brief	Get SSL blocking mode
 *@param  	handle  	SSL handle
 *@return
 *@li	\ref NAPI_NOWAIT "NAPI_NOWAIT"()		\ref NAPI_SUSPEND "NAPI_SUSPEND"()
 *@li	\ref NAPI_ERR_SSL_PARAM "NAPI_ERR_SSL_PARAM"	(SSL)
 *@li	\ref NAPI_ERR_SSL_ALLOC "NAPI_ERR_SSL_ALLOC"	()
 *@li	\ref NAPI_ERR_SSL_ALREADCLOSE "NAPI_ERR_SSL_ALREADCLOSE"	(SSL)
 *@li	\ref NAPI_ERR "NAPI_ERR"	SSL
*/
int _GetSSLBlockingMode_(SSL_HANDLE handle);
/**
 *@brief	Set SSL blocking mode
 *@param  	handle  	SSL handle
 *@param  	nMode    (ref EM_BLOCK_OPT "EM_BLOCK_OPT")
 *@return
 *@li	NAPI_OK
 *@li	\ref NAPI_ERR_SSL_PARAM "NAPI_ERR_SSL_PARAM"	(SSL)
 *@li	\ref NAPI_ERR_SSL_ALLOC "NAPI_ERR_SSL_ALLOC"	()
 *@li	\ref NAPI_ERR_SSL_ALREADCLOSE "NAPI_ERR_SSL_ALREADCLOSE"	(SSL)
 *@li	\ref NAPI_ERR "NAPI_ERR"	fcntl()
 *@li	\ref NAPI_ERR_SSL_MODEUNSUPPORTED "NAPI_ERR_SSL_MODEUNSUPPORTED"	nMode
*/
int _SetSSLBlockingMode_(SSL_HANDLE handle,int nMode);
/**
 *@brief	Sending data
 *@param  	handle  	SSL handle
 *@param  	psBuffer 	data buffer
 *@param  	unBufferLen expected sending length
 *@retval 	punSendLen    actual sending length
 *@return
 *@li	NAPI_OK
 *@li	\ref NAPI_ERR_SSL_PARAM "NAPI_ERR_SSL_PARAM"	(SSLpsBuffer/punSendLenNULL)
 *@li	\ref NAPI_ERR_SSL_SEND "NAPI_ERR_SSL_SEND"	(SSL_write)
 *@li	\ref NAPI_ERR "NAPI_ERR"	SSL
*/
int _SSLSend_(SSL_HANDLE handle, const char *psBuffer, uint unBufferLen, uint *punSendLen);
/**
 *@brief	Receiving data
 *@param  	handle  	SSL handle
 *@param  	pvBuffer 	data buffer
 *@param  	unBufferLen expected receiving length
 *@retval 	punRecvLen  actual receiving length
 *@return
 *@li	NAPI_OK
 *@li	\ref NAPI_ERR_SSL_PARAM "NAPI_ERR_SSL_PARAM"	(SSLpvBuffer/punRecvLenNULL)
 *@li	\ref NAPI_ERR_SSL_RECV "NAPI_ERR_SSL_RECV"	(SSL_read)
 *@li	\ref NAPI_ERR "NAPI_ERR"	SSL
*/
int _SSLReceive_(SSL_HANDLE handle, void *pvBuffer, const uint unBufferLen, uint *punRecvLen);
/**
 *@brief	SSL bind
 *@param  	handle  	SSL handle
 *@param  	pstAddr
 *@return
 *@li	NAPI_OK
 *@li	\ref NAPI_ERR_SSL_PARAM "NAPI_ERR_SSL_PARAM"	(SSLpstAddrNULL)
 *@li	\ref NAPI_ERR_SSL_ALREADCLOSE "NAPI_ERR_SSL_ALREADCLOSE"	(SSL)
 *@li	\ref NAPI_ERR_SSL_ALLOC "NAPI_ERR_SSL_ALLOC"
 *@li	\ref NAPI_ERR_SSL_INVADDR "NAPI_ERR_SSL_INVADDR"	bind
 *@li	\ref NAPI_ERR "NAPI_ERR"
*/
int _SSLBind_(SSL_HANDLE handle, ST_SOCKET_ADDR *pstAddr);
/**
 *@brief	Get connection status
 *@param  	handle  	SSL handle
 *@retval  	pnState  	status(ref EM_SSL_CONNECTION_STATE "EM_SSL_CONNECTION_STATE")
 *@return
 *@li	NAPI_OK
 *@li	\ref NAPI_ERR_SSL_PARAM "NAPI_ERR_SSL_PARAM"	(SSLpnStateNULL)
 *@li	\ref NAPI_ERR_SSL_ALREADCLOSE "NAPI_ERR_SSL_ALREADCLOSE"	(SSL)
 *@li	\ref NAPI_ERR "NAPI_ERR"
*/
int _GetSSLConnectStatus_(SSL_HANDLE handle, int *pnState);
/**
 *@brief	Check whether the data is available
 *@param  	handle  	SSL handle
 *@param  	unTimeOut  	10s
 *@return
 *@li	NAPI_OK
 *@li	\ref NAPI_ERR_SSL_PARAM "NAPI_ERR_SSL_PARAM"	(SSLtimeout<0)
 *@li	\ref NAPI_ERR_SSL_ALREADCLOSE "NAPI_ERR_SSL_ALREADCLOSE"	(SSL)
 *@li	\ref NAPI_ERR_SSL_CONNECT "NAPI_ERR_SSL_CONNECT"	SSL
 *@li	\ref NAPI_ERR_SSL_TIMEOUT "NAPI_ERR_SSL_TIMEOUT"
*/
int _SSLDataAvailable_(SSL_HANDLE handle, uint unTimeOut);
/**
 *@brief	get certifiate in server structure of X509
 *@param  	handle  	SSL handle
 *@return
 *@li	    X509
 *@li	    NULL
*/
void *_SSLGetPeerCerificate_(SSL_HANDLE handle);

#ifdef __cplusplus
}
#endif

#endif


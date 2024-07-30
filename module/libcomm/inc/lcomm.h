/***************************************************************************
** All right reserved:  Newland Copyright (c) 2006 - 2008 
** File name:  lcomm.h
** File indentifier: 
** Brief:  Communication processing module
** Current Verion:  v1.0
** Auther: Liug
** Complete date: 2016-9-25
** Modify record: 
** Modify record: 
** Modify date: 
** Version: 
** Modify content: 
***************************************************************************/
#ifndef _LCOMM_H_
#define _LCOMM_H_

#include "napi_wifi.h"

/** @addtogroup Communication
* @{
*/

typedef enum 
{
	SERVERMODE_RECVQUIT = (1<<0),				/**<User may Quit as receiving return value*/
	SERVERMODE_CLOSENOWAIT = (1<<1),	   		/**<Execute communication off (background)*/
	SERVERMODE_INITNOWAIT = (1<<2),		   		/**<Execute initialization (background)*/
	SERVERMODE_CLOSEWAIT = (1<<3),		   		/**<Communication off (foreground)*/
	SERVERMODE_INITWAIT = (1<<4),		   		/**<Initialization (foreground)*/
	SERVERMODE_PRECONNECT = (1<<5),				/**<Execute SOCKET predictived dial(background), Default SOCKET None*/
	SERVERMODE_CONNECTNOTQUIT = (1<<6)			/**<Can not QUIT as connecting*/
 }EM_SERVERMODE;					/**<no predictived dialer by default*/		

 
typedef enum 	/**<Hangup mode*/
{
	SOCKET_HANGUP=0,	 	/**<Hangup Socket*/
	PPP_HANGUP,		 		/**<Hangup PPP*/
}EM_HANGUP_MODE;

typedef enum {
	COMMTYPE_SYNDIAL=0x00,  			   	/**<Synchronous dial*/
	COMMTYPE_SYNDIAL_TPDU,    				/**<Synchronous dial+TPDU*/
	COMMTYPE_SYNDIAL_HEADLEN, 			    /**<Synchronous dial+two Bytes lenth*/  
	COMMTYPE_SYNDIAL_TPDU_HEADLEN,    		/**<Synchronous dial+TPDU+two Bytes lenth*/
	
	COMMTYPE_ASYNDIAL=0x10,  			   	/**<Asynchronous dial*/
	COMMTYPE_ASYNDIAL_ASYN, 				/**<Asynchronous dial:0x02start+[two Bytes lenth]+[data]+[lrc]+0x03end*/  
	COMMTYPE_ASYNDIAL_ASYN_TPDU,      		/**<Asynchronous dial:0x02start+[two Bytes lenth]+TPDU+[data]+[lrc]+0x03end*/
	COMMTYPE_ASYNDIAL_ASYN1, 			   	/**<Asynchronous dial:0x02start+[two Bytes lenth]+[data]+0x03end+[lrc]*/  
	COMMTYPE_ASYNDIAL_ASYN1_TPDU,     		/**<Asynchronous dial:0x02start+[two Bytes lenth]+TPDU+[data]+0x03end+[lrc]*/
	
	COMMTYPE_PORT=0x20,  			  		/**<Serial port*/
	COMMTYPE_PORT_ASYN, 				  	/**<Serial port:0x02start+[two Bytes lenth]+[data]+[lrc]+0x03end*/  
	COMMTYPE_PORT_ASYN_TPDU,           		/**<Serial port:0x02start+[two Bytes lenth]+TPDU+[data]+[lrc]+0x03end*/
	COMMTYPE_PORT_ASYN1, 			  		/**<Serial port:0x02start+[two Bytes lenth]+[data]+0x03end+[lrc]*/  
	COMMTYPE_PORT_ASYN1_TPDU,     	  		/**<Serial port:0x02start+[two Bytes lenth]+TPDU+[data]+0x03end+[lrc]*/
	
	COMMTYPE_GPRS=0x30,  			  		/**<GPRS*/
	COMMTYPE_GPRS_HEADLEN, 			  		/**<GPRS+two Bytes lenth*/  
	COMMTYPE_GPRS_TPDU_HEADLEN,        		/**<GPRS+TPDU+two Bytes lenth*/
	COMMTYPE_GPRS_ASYN1, 			    	/**<GPRS:0x02start+[two Bytes lenth]+[data]+0x03end+[lrc]*/  
	COMMTYPE_GPRS_ASYN1_TPDU,     			/**<GPRS:0x02start+[two Bytes lenth]+TPDU+[data]+0x03end+[lrc]*/
	
	COMMTYPE_CDMA=0x40,  			  		/**<CDMA*/
	COMMTYPE_CDMA_HEADLEN, 			  		/**<CDMA+two Bytes lenth*/  
	COMMTYPE_CDMA_TPDU_HEADLEN,        		/**<CDMA+TPDU+two Bytes lenth*/
	COMMTYPE_CDMA_ASYN1, 			    	/**<CDMA:0x02start+[two Bytes lenth]+[data]+0x03end+[lrc]*/  
	COMMTYPE_CDMA_ASYN1_TPDU,     			/**<CDMA:0x02start+[two Bytes lenth]+TPDU+[data]+0x03end+[lrc]*/

	COMMTYPE_ETH=0x50,  				 	/**<ETH*/
	COMMTYPE_ETH_HEADLEN, 			  		/**<ETH+two Bytes lenth*/  
	COMMTYPE_ETH_TPDU_HEADLEN,          	/**<ETH+TPDU+two Bytes lenth*/
	COMMTYPE_ETH_ASYN1, 			    	/**<ETH:0x02start+[two Bytes lenth]+[data]+0x03end+[lrc]*/  
	COMMTYPE_ETH_ASYN1_TPDU,     			/**<ETH:0x02start+[two Bytes lenth]+TPDU+[data]+0x03end+[lrc]*/

	COMMTYPE_USB=0x60,  			  		/**<USB*/
	COMMTYPE_USB_ASYN, 				  		/**<USB:0x02start+[two Bytes lenth]+[data]+[lrc]+0x03end*/  
	COMMTYPE_USB_ASYN_TPDU,           		/**<USB:0x02start+[two Bytes lenth]+TPDU+[data]+[lrc]+0x03end*/
	COMMTYPE_USB_ASYN1, 			  		/**<USB:0x02start+[two Bytes lenth]+[data]+0x03end+[lrc]*/  
	COMMTYPE_USB_ASYN1_TPDU,    	  		/**<USB:0x02start+[two Bytes lenth]+TPDU+[data]+0x03end+[lrc]*/

	COMMTYPE_WIFI=0x70,  			  		/**<WIFI*/
	COMMTYPE_WIFI_HEADLEN, 			  		/**<WIFI+two Bytes lenth*/  
	COMMTYPE_WIFI_TPDU_HEADLEN,        		/**<WIFI+TPDU+two Bytes lenth*/
	COMMTYPE_WIFI_ASYN1, 			    	/**<WIFI:0x02start+[two Bytes lenth]+[data]+0x03end+[lrc]*/  
	COMMTYPE_WIFI_ASYN1_TPDU,     			/**<WIFI:0x02start+[two Bytes lenth]+TPDU+[data]+0x03end+[lrc]*/

	COMMTYPE_MODEM=0x80,  			  		/**<MODEM*/
	COMMTYPE_MODEM_HEADLEN, 			  	/**<MODEM+two Bytes lenth*/  
	COMMTYPE_MODEM_TPDU_HEADLEN,        	/**<MODEM+TPDU+two Bytes lenth*/
	COMMTYPE_MODEM_ASYN1, 			    	/**<MODEM:0x02start+[two Bytes lenth]+[data]+0x03end+[lrc]*/  
	COMMTYPE_MODEM_ASYN1_TPDU,     			/**<MODEM:0x02start+[two Bytes lenth]+TPDU+[data]+0x03end+[lrc]*/

}EM_COMMTYPE;


typedef struct
{
	char  szPredialNo[10];  			/**<External number*/
	char  lszTelNo[3][21];  			/**<3 groups of telephone numbers*/
	char  lsTPDU[3][5];     			/**<3 groups of TPDU*/
	int	  nCycTimes; 		    		/**<cycle dialing times*/
	int   nCountryid; 	    			/**<modem country ID, See EM_MDM_PatchType for reference*/
	int   nBps;							/**<modem Baud rate, See EM_MDM_BPS for reference*/
}STDIALPARAM;

typedef struct
{
	int nAux;							/**<Serial port number*/
	int nBaudRate;
	int nDataBits;
	int nParity;
	int nStopBits;
	char sTPDU[5];   	 				/**<TPDU*/
}STPORTPARAM;

typedef struct
{
	char lszIp[2][16]; 		  			/**<Server IP address---2 groups*/
	int  lnPort[2]; 					/**<Service port number  ---2 groups*/
	char szDN[50+1];         			/**<Server domain name*/
	char szDNSIp[32+1];      			/**<Domain name server IP*/
}STSERVERADDRESS;

typedef struct 
{
 	char szPinPassWord[20];             /**<Password of SIM card---GPRS and CDMA require*/
	char szNetUsername[40]; 			/**<Required user name as establishing a PPP link---2 groups */
	char szNetPassword[20]; 			/**<Required password as establishing a PPP link ---2 groups*/
	char szModemDialNo[21]; 			/**<Required access number as establishing a PPP link ---2 groups*/
	char szGprsApn[40]; 				/**< APN name --- 2 groups*/
	char sTPDU[5];               		/**<TPDU*/
	char szPdpType[20];            		/*IP,IPV6,IPV4V6*/
}STGPRSPARAM;

typedef struct 
{
	char szPinPassWord[20];             /**<SIM card password---GPRS and CDMA require*/
	char szNetUsername[40]; 	       	/**<PPP username---CDMA require*/
	char szNetPassword[20];   	     	/**<PPP password---CDMA reequire*/
	char szModemDialNo[21]; 		    /**<Required access number as establishing a PPP link ---2 groups*/
	char sTPDU[5];                		/**<TPDU*/
	char szPdpType[20];            		/*IP,IPV6,IPV4V6*/
}STCDMAPARAM;

typedef struct
{
	int  nDHCP;				    		/**<Use of DHCP, '1' - On */
	char szIP[16];				        /**<Local IP address*/
	char szGateway[16];		    		/**<local Gateway*/
	char szMask[16];		  		    /**<Local Subnet mask*/
	char szDNS[16];          		    /**<DNS*/
	char sTPDU[5];            	  		/**<TPDU*/
}STETHPARAM;

typedef struct
{
	uchar ucIfDHCP;             		/**<Use DHCP or not  1 means use*/
	char szSsid[32];               		/**<Router SSID*/
	char szKey[32];         	        /**<Router password*/
	char szIP[16];              		/**<Terminal IP address*/
	char szMask[16];         			/**<Subnet mask address*/
	char szGateway[16];         		/**<Gateway address*/
	char szDnsPrimary[16];      		/**<First DNS address*/
	char szDnsSecondary[16];			/**<Second DNS address*/
	signed char cWifiMode;				/**<wifiMode*/
	int nNetId;							/**< NET ID*/
	char sTPDU[5];            	  		/**<TPDU*/
} STWIFIPARAM;

typedef struct
{
	char szPredialNo[10]; 				/**<External number*/
	char szTelNo[21];  					/**<3 groups of phone numbers*/
	int  nCountryid ; 	  				/**<modem country ID*/
	char szNetUsername[40];			 	/**<PPP username*/
	char szNetPassword[20]; 			/**<PPP password*/
	char sTPDU[5];           		 	/**<TPDU*/
}STMODEMPARAM;

typedef struct
{
	char cCommType;				      	/**<Communication type, see enum EM_COMMTYPE*/
	char cPreDialFlag;				  	/**<Setting of pre Dial. Set '1' for dial, '0' for no dial*/
	char cReDialNum;				  	/**<Redial times*/
	int  nTimeOut;					  	/**<Timeout*/
	char cMode;						    /**<Flag of long/short connection*/
	char cIsSupportQuitRecv;            /**<COMMSERVER mode, see EM_SERVERMODE enum. Use '|' if support multi mode*/
	union
	{
		STDIALPARAM stDialParam;  		/**<Structure of dial parameter */
		STPORTPARAM stPortParam;  		/**<Structure of Serial port parameter */
		STGPRSPARAM stGprsParam;  		/**<Structure of GPRS parameter */
		STCDMAPARAM stCdmaParam;  		/**<Structure of CDMA partameter*/
		STETHPARAM  stEthParam;   		/**<Structure of Ethernet parameter*/
		STWIFIPARAM  stWifiParam;  		/**<Structure of Wifi parameter*/
		STMODEMPARAM  stModemParam;   	/**<Structure of Wifi parameter*/
	}ConnInfo;
	STSERVERADDRESS stServerAddress; 	/**<Server parameters*/
	void ( *ShowFunc )( void );			/**<Displayed information when blocking*/
	char cSslFlag;                      /**<ssl sign: 0.Disable ssl ; 1.ssl One-way authentication; 2.ssl Two-way authentication*/
	char sRfu[39];
}STCOMMPARAM;

typedef struct
{
    int nColumn; 						/**<Column (from 1)*/
    int nRow; 							/**<Row (from 1)*/
    int nType; 							/**<Displaying type, 0-increase 1-decrease*/
}STSHOWINFOXY; 							/**<Structure of countdown display, when both nColumn and nType are set 0, countdown display will disappear*/

typedef struct
{
	int nType;                  		/**<Handshake protocol type, see EM_SSL_HANDSHAKE_OPT*/
	int nAuthOpt;						/**<Authentication mode, see EM_SSL_AUTH_OPT*/
	int nFileFormat;          		 	/**<Certificate file format, see EM_SSL_FILE_FORMAT*/
	char szPwd[16];          			/**<Certificate password if existed*/
	char szServerCert[64];  			/**<CA certificate file*/
	char szClientCert[64];  			/**<local certificate file*/
	char szClientPrivateKey[64];    	/**<local private key*/
}STSSLMODE;

typedef struct
{
	char szSubjectName[256];            /**<Certificate owner*/
	char szIssuername[256];				/**<Certificate issuer*/
	char nVersion;           			/**<Certificate version*/
	char szNotBeforeTime[16];          	/**<Certificate effective time*/
	char szNotAfterTime[16];          	/**<Certificate expiration time*/
	char szCountryName[32];           	/**<Name of certificate country*/
	char szCommonName[32];           	/**<Name of certificate host*/
	char szOrganizationName[64];  		/**<Name of certificate organization*/
	char szOrganizationalUnitName[64];  /**<Name of certificate organization unit*/
}STSSLCERTMSG;


/**
* @brief Set communication parameters and initialize communication module
* @param [in] pstCommParam  Communication parameters
* @return 
* @li APP_FAIL Success
* @li APP_SUCC Fail
* @author liug
* @date 2012-5-25
*/
int PubCommInit(const STCOMMPARAM *pstCommParam);    

/**
* @brief Connect beforehand 
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author liug
* @date 2012-5-24
*/
int PubCommPreConnect(void);

/**
* @brief Communication connect
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author liug
* @date 2012-5-25
*/
int PubCommConnect(void);

/**
* @brief In accordance with the modes of communication to write data
* @param [in] psData        Data to be sent
* @param [in] nDataLen      Data length
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author liug
* @date 2012-5-23
*/
int PubCommSend(const char *psData, int nDataLen);

/**
* @brief Read data
* @details  In accordance with the modes of communication to read data
* @param [out] psData        Data received 
* @param [out] *pnDataLen    Length of data received
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author liug
* @date 2012-5-23
*/
int PubCommRecv(char *psData, int *pnDataLen);

/**
* @brief Write data
* @details Write data directly to the data channel
* @param [in]  psData        Data to be sent
* @param [in]  nDataLen      Data length
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author liug
* @date 2012-5-23
*/
int PubCommWrite(const char *psData, int nDataLen);

/**
* @brief Read data from data channel
* @param [out] psData        Data received
* @param [in] nMaxLen       Max buffer size to receive data
* @param [out] *pnDataLen    Length of data received
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author liug
* @date 2012-5-23
*/
int PubCommRead(char *psData, int nMaxLen, int *pnDataLen);

/**
* @brief Hang up communication link
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author liug
* @date 2012-5-24
*/
int PubCommHangUp(void);

/**
* @brief Close TCP link immediately and clear buffer
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2016-01-14
*/
int PubCommHangUpReset(void);

/**
* @brief Close communication
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author liug
* @date 2012-5-24
*/
int PubCommClose(void);

/**
* @brief Get the version of communication module
* @param [out] pszVer  Version string
* @return 
* @li void
*/
void PubGetCommVerion(char *pszVer);

/**
* @brief Set communication parameter
* @param [in] pstCommParam
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author liug
* @date 2012-5-24
*/
int PubSetCommParam(const STCOMMPARAM *pstCommParam);

/**
* @brief Clear communication buffer
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author liug
* @date 2012-5-24
*/
int PubCommClear(void);

/**
* @brief Set style for displaying timeout interface
* @param [in] stShowInfoxy
* @return 
* @li void
* @author liug
* @date 2012-5-24
*/
void PubSetShowXY(STSHOWINFOXY stShowInfoxy);

/**
* @brief Set the timeout
* @param [in] nTimeOut  timeout(>=0)
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author liug
* @date 2012-5-25
*/
int PubSetCommTimeOut(int nTimeOut);

/**
* @brief Set the times of redial
* @param [in] nReDialNum  Number of redial(1---9)
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author liug
* @date 2012-5-25
*/
int PubSetReDialNum(int nReDialNum);

/**
* @brief Get DHCP local address
* @param [out] pstEthParam
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author liug
* @date 2013-5-7
*/
int PubGetDhcpLocalAddr(STETHPARAM *pstEthParam);

/**
* @brief Get connection index
* @param [out] pnIndex
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author liug
* @date 2012-12-20
*/
int PubGetConnectIndex(int *pnIndex);

/**
* @brief Get communication parameter
* @param  [out] pstCommParam
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author sunh
* @date 2013-9-15
*/
int PubGetCommParam(STCOMMPARAM* pstCommParam);

/**
* @brief Configure ssl and use it before connection. Use default configuration if there is no setting 
* @param [in] pstSslMode ssl Related parameters
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author sunh
* @date 2014-1-1
*/
int PubSslSetMode(STSSLMODE *pstSslMode);

/**
* @brief Get SSL server certificate information
* @param  [out] pstSslCertMsg Certificate information
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author sunh
* @date 2014-1-1
*/
int PubSslGetCertMsg(STSSLCERTMSG* pstSslCertMsg);

/**
* @brief Scan WIFI, select SSID, and get the encryption mode
* @param  [in] pszTitle ssid List title, show one more line if set NULL
* @param  [in] nTimeout Timeout
* @param  [out] pszOutSsid Wifi hotspot name
* @param  [out] pnWifiMode Wifi encryption mode
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author sunh
* @date 2014-1-10
*/
int PubCommScanWifi(const char *pszTitle, char *pszOutSsid, int *pnWifiMode, int nTimeout);

/**
* @brief Connect to network(wifi or ppp)
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author sunh
* @date 2015-1-8
*/
int PubCommDialNet(void);

/**
* @brief Domain name resolution
* @param [in] pszDn Domain name
* @param [in] pszDnsIp Domain name service address
* @param [in] cIsQuery Mode YES:read backup file; NO:reparse
* @param [out] pszIp parsed IP
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author sunh
* @date 2015-1-10
*/
int PubCommDnParse(const char *pszDn, const char *pszDnsIp, char *pszIp, char cIsQuery);

/**
* @brief Get ssid list 
* @param  [out] lszSsid SSID list
* @param  [out] lnMode SSID mode
* @param  [in, out] pnSsidNum in, Maximum number of ssid out, returned number of ssid
* @param  [in] nTimeout Timeout time
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author sunh
* @date 2015-11-12
*/
int PubCommWifiGetSsid(char lszSsid[][32], int lnMode[], int *pnSsidNum, int nTimeout);

/**
* @brief Detecting link connection
* @param [out] pnStatus Return EM_PPP_STATUS or EM_WPA_CONSTATE
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author sunh
* @date 2015-11-12
*/
int PubCommNetCheck(int *pnStatus);


/** @}*/ // End of Communication

#endif


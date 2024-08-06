/***************************************************************************
** Copyright (c) 2019 Newland Payment Technology Co., Ltd All right reserved
** File name: comm.h
** File indentifier:
** Synopsis:
** Current Verion:  v1.0
** Auther: sunh
** Complete date: 2016-9-5
** Modify record:
** Modify date:
** Version:
** Modify content:
***************************************************************************/
#ifndef _COMM_H_
#define _COMM_H_

#include "napi_wifi.h"

#define FILE_APPCOMMPARAM	APP_NAME"COMM"

/**
* @struct STAPPCOMMPARAM
*/
typedef struct
{
	char cCommType;					/**<see enum COMMTYPE*/
	char cCommType2;
	char cPreDialFlag;				/**<predial identifier*/
	char cReDialNum;				/**<redial time*/
	char cReSendNum;				/**<resend time*/
	char cTimeOut;					/**<timeout*/
	char sTpdu[10];					/**<TPDU*/
	char szPreDial[10+1];			/**<exterior number	AN10*/
	char szTelNum1[19+1];			/**<Telephone1	N14*/
	char szTelNum2[19+1];			/**<Telephone2	N14*/
	char szTelNum3[19+1];			/**<Telephone3	N14*/
	char szIp1[16+1];				/**<Ip1	An16*/
	char szPort1[6+1];				/**<port1	N4*/
	char szIp2[16+1];				/**<Ip2	An16*/
	char szPort2[6+1];				/**<port2	N4*/
	char szWirelessDialNum[40+1];	/**<Wireless Dial Num	An20*/
	char szAPN1[40+1];				/**<APN1	An30*/
	char szAPN2[40+1];				/**<APN2	An30*/
	char szUser[40+1];				/**<user	An40*/
	char szPassWd[40+1];			/**<password	An40*/
	char szSIMPassWd[20+1];			/**<SIM card password	N10*/
	char cMode;						/**<ppp mode	N1*/
	char szIpAddr[16+1];			/**<local ip	An16*/
	char szMask[16+1];				/**<Mask	An16*/
	char szGate[16+1];				/**<Gate	An16*/
	char szDNSIp1[16+1];			/**<DNS IP1	An16*/
	char szDNSIp2[16+1];			/**<DNS IP2	An16*/
	char szDNSIp3[16+1];			/**<DNS IP3	An16*/
	char szDomain[50+1];			/**<Domain	An100*/
	char szDomain2[50+1];			/*Domain2 AN100*/
	char cAuxIsAddTpdu;				/**<aux need tpdu, 1 YES; 0 NO*/
	char cIsDns;                    /*use DNS*/
	char cOffResendNum;				/**<offline resend time*/
	char cIsDHCP;					/**<use DHCP*/
	char szWifiSsid[32+1];			/**<WIFI ssid */
	char szWifiKey[64+1];			/**<WIFI password */
	char cWifiMode;					/**<wifi mode EM_WIFI_NET_SEC*/
	char szNii[3+1];				/**EDC NII*/
	char cIsSSL;
	char szPdpType[20];

    char szTOMSAppDomain[50+1]; 		/**<TOMS APP Domain An50*/
    char szTOMSParamDomain[50+1]; 		/**<TOMS Param Domain An50*/
    char szTOMSKeyPosDomain[50+1]; 		/**<TOMS Key POS Domain An50*/
    char szTOMSFileServerDomain[50+1];  /**<TOMS file server domain An50 */
    char szTOMSTdasDomain[50+1];        /**<TOMS file server domain An50 */

    char cIsUserOid;                  /**< is user oid*/
    char szTomsUserOid[64+1];          /**< user oid*/
}STAPPCOMMPARAM;

enum COMMTYPE
{
	COMM_NONE = 0,
	COMM_DIAL = 1,		/**<NAC*/
	COMM_GPRS,			/**<GPRS*/
	COMM_CDMA,			/**<CDMA*/
	COMM_RS232,		/**<AUX*/
	COMM_ETH,			/**<ETH*/
	COMM_WIFI,			/*WIFI*/
	COMM_USB			/**<USB*/
};

#define MAX_SEND_SIZE (MAX_PACK_SIZE + 20)
#define MAX_RECV_SIZE MAX_SEND_SIZE

extern void SetControlCommInit(void);
extern int CommInit(void);
extern int CommPreDial(void);
extern int CommHangUp(void);
extern int CommHangUpSocket(void);
extern int CommConnect(void);
extern int CommSend(const char *, int);
extern int CommRecv(char *,int *);
extern int CommSendRecv(const char *, int, char *,int *);
extern int CommDump(void);
extern int InitCommParam(void);
extern int SetFuncCommPreDialNum(void);
extern int SetFuncCommType2(void);
extern int SetFuncCommType(void);
extern int GetVarCommTimeOut(char *);
extern int SetVarCommTimeOut(const char *);
extern int GetVarCommReSendNum(char * );
extern int SetVarCommReSendNum(const char * );
extern int GetVarCommReDialNum(char *);
extern int SetVarCommReDialNum(const char * );
extern int SetFuncCommReSendNum(void);
extern void GetAppCommParam(STAPPCOMMPARAM *);
extern void SetAppCommParam(STAPPCOMMPARAM);
extern int GetVarCommOffReSendNum(char *);
extern void CommMenu(void);
extern int DoSetPreDialNum(void);
extern int GetVarCommNii(char *);
extern int ExportCommParam(void);
extern int SetFuncCommNii(void);
extern int SetComm();
extern int SwitchCommType(void);
extern int SetFuncTOMSUserOID(void);
extern char GetUserOidSwitch();
extern void SetUserOidSwitch(int nMode);
extern int SetFuncCommTpdu(void);
extern int SetFuncUserOid(char *pOidBuf, int nOidBufLen);
extern int GetFuncUserOid(char *pOidBuf, int nOidBufLen);
#endif


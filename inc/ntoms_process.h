/***************************************************************************
** Copyright (c) 2023 Newland Payment Technology Co., Ltd All right reserved
** File name: ntoms_process.h
** File identifier:
** Synopsis:
** Current Version:  v1.0
** Author: Jax Ji
** Complete date: 2023-09-18
** Modify record:
** Modify date:
** Version:
** Modify content:
***************************************************************************/

#ifndef _NTOMS_PROCESS_H_
#define _NTOMS_PROCESS_H_

#ifdef USE_NTOMS

/*******************************
 * Macro Definition
 *******************************/
/* Param Type String */
#define NTOMSTYPE_STR_STRING		"STRING"
#define NTOMSTYPE_STR_BOOLEAN		"BOOLEAN"
#define NTOMSTYPE_STR_NUMBER		"NUMBER"
#define NTOMSTYPE_STR_HEX			"HEX"
#define NTOMSTYPE_STR_REFERENCE		"REFERENCE"
#define NTOMSTYPE_STR_FILE			"FILE"

/* BOOL_VALUE */
#define NTOMS_BOOL_TURE				"true"
#define NTOMS_BOOL_FALSE			"false"

/* CFG param */
#define NTOMSTAG_BODY				"body"
#define NTOMSTAG_PARAMVALUES		"paramValues"
#define NTOMSTAG_TYPE				"type"
#define NTOMSTAG_VALUE				"value"
#define NTOMSTAG_SUBVALUE			"value"

/* basic param */
#define NTOMSTAG_VERSION			"appVersion"

/* basic param */
#define NTOMSTAG_MERCHANTID			"merchantId"
#define NTOMSTAG_TERMINALID			"terminalId"
#define NTOMSTAG_MERCHANTNAMEEN		"merchantName_EN"
#define NTOMSTAG_MERCHANTADDR1		"merchantAddr1"
#define NTOMSTAG_MERCHANTADDR2		"merchantAddr2"
#define NTOMSTAG_MERCHANTADDR3		"merchantAddr3"

/* transaction control */
#define	NTOMSTAG_SUPPORTSALE			"consumeYN"
#define	NTOMSTAG_SUPPORTVOIDSALE		"cousumeVoidYN"
#define	NTOMSTAG_SUPPORTREFUND			"refundYN"
#define	NTOMSTAG_SUPPORTBALANCE			"balanceYN"
#define	NTOMSTAG_SUPPORTPREAUTH			"preAuthYN"
#define	NTOMSTAG_SUPPORTVOIDPEAUTH		"preAuthVoidYN"
#define	NTOMSTAG_SUPPORTAUTHSALE		"authConsumeYN"
#define	NTOMSTAG_SUPPORTVOIDAUTHSALE	"authConsumeVoidYN"
#define	NTOMSTAG_SUPPORTADJUST			"adjustYN"

/* system param */
#define	NTOMSTAG_TRANS_TRACENO		"traceNo"
#define	NTOMSTAG_TRANS_BATCHNO		"batchNo"
#define	NTOMSTAG_ISPINPAD			"pinpadOutSideYN"
#define	NTOMSTAG_MAXTRANSCNT		"maxTransaction"

/* communication param */
#define	NTOMSTAG_COMM_NII			"NII"
#define	NTOMSTAG_COMM_TYPE			"commType"
#define	NTOMSTAG_COMM_TIMEOUT		"commTimeOut"
#define NTOMSTAG_COMM_TPDU			"tpdu"
/* LINE(ETH) */
#define	NTOMSTAG_COMM_IP1_E			"ethHost1"
#define	NTOMSTAG_COMM_PORT1_E		"ethPort1"
#define	NTOMSTAG_COMM_IP2_E			"ethHost2"
#define	NTOMSTAG_COMM_PORT2_E		"ethPort2"
#define	NTOMSTAG_COMM_IPADDR_E		"ethIP"
#define	NTOMSTAG_COMM_MASK_E		"ethMask"
#define	NTOMSTAG_COMM_GATE_E		"ethGateway"
#define NTOMSTAG_COMM_ISDHCP_E		"ethDHCP"

/* DIAL */
#define	NTOMSTAG_COMM_PREDIALNO		"preDialNum"
#define	NTOMSTAG_COMM_PREPHONE		"prePhone"
#define	NTOMSTAG_COMM_TELNO1		"tel1"
#define	NTOMSTAG_COMM_TELNO2		"tel2"
#define	NTOMSTAG_COMM_TELNO3		"tel3"
#define	NTOMSTAG_COMM_ISPREDIAL		"predialYN"

/* GPRS */
#define NTOMSTAG_COMM_WIRELESSDIAL_G "gprsNo"
#define	NTOMSTAG_COMM_IP1_G			"gprsHost1"
#define	NTOMSTAG_COMM_PORT1_G		"gprsPort1"
#define	NTOMSTAG_COMM_IP2_G			"gprsHost2"
#define	NTOMSTAG_COMM_PORT2_G		"gprsPort2"
#define	NTOMSTAG_COMM_APN1			"gprsAPN"
#define	NTOMSTAG_COMM_USER_G		"gprsUser"
#define	NTOMSTAG_COMM_PWD_G			"gprsPwd"
#define	NTOMSTAG_COMM_MODE_G		"gprsHangup"
/* CDMA */
#define	NTOMSTAG_COMM_IP1_C			"cdmaHost1"
#define	NTOMSTAG_COMM_PORT1_C		"cdmaPort1"
#define	NTOMSTAG_COMM_IP2_C			"cdmaHost2"
#define	NTOMSTAG_COMM_PORT2_C		"cdmaPort2"
#define	NTOMSTAG_COMM_USER_C		"cdmaUser"
#define	NTOMSTAG_COMM_PWD_C			"cdmaPwd"
#define	NTOMSTAG_COMM_MODE_C		"cdmaHangup"
/* WIFI */
#define	NTOMSTAG_COMM_WIFISSID		"wifiSSID"
#define	NTOMSTAG_COMM_WIFIKEY		"wifiPwd"
#define	NTOMSTAG_COMM_WIFIMODE		"wifiEncrypt"
#define	NTOMSTAG_COMM_IP1_W			"wifiHost1"
#define	NTOMSTAG_COMM_PORT1_W		"wifiPort1"
#define	NTOMSTAG_COMM_IP2_W			"wifiHost2"
#define	NTOMSTAG_COMM_PORT2_W		"wifiPort2"
#define	NTOMSTAG_COMM_IPADDR_W		"wifiIP"
#define	NTOMSTAG_COMM_MASK_W		"wifiMask"
#define	NTOMSTAG_COMM_GATE_W		"wifiGateway"
#define NTOMSTAG_COMM_ISDHCP_W		"wifiDHCP"
/* KEY */
#define NTOMSTAG_MAINKEYNO			"keyIndex"
#define NTOMSTAG_ENCRYMODE			"desType"
#define NTOMSTAG_ADMINPWD			"systemPwd"
#define NTOMSDEFAULT_FUNCTION		"safePwd"
/* PRINT */
#define NTOMSTAG_PNTTITLE			"printHead"
#define NTOMSTAG_PNTPAGECNT			"printNum"
#define NTOMSTAG_FONTSIZE			"printFont"
#define NTOMSTAG_PNTDETAIL			"printDetail"
#define NTOMSTAG_ISPREAUTHSHIELDPAN	"preauthprintMaskCardNo"

#define NTOMSTAG_TESTFILE			"test.file"

/*******************************
 * Type Definition
 *******************************/
typedef enum
{
	NTOMSTYPE_STRING = 0,
	NTOMSTYPE_BOOLEAN,
	NTOMSTYPE_NUMBER,
	NTOMSTYPE_HEX,
	NTOMSTYPE_REFERENCE,
	NTOMSTYPE_FILE,
}EM_NTOMS_PARAM_TYPE;

typedef enum
{
	NTOMS_COMMTYPE_DAIL = 0,
	NTOMS_COMMTYPE_ETH,
	NTOMS_COMMTYPE_CDMA,
	NTOMS_COMMTYPE_GPRS,
	NTOMS_COMMTYPE_RS232,
	NTOMS_COMMTYPE_WIFI,
}EM_NTOMS_PARAM_COMMTYPE;

typedef enum
{
	NTOMS_WIFITYPE_OPEN = 1,
	NTOMS_WIFITYPE_WEP,
	NTOMS_WIFITYPE_WPA_PSK,
	NTOMS_WIFITYPE_WPA2_PSK,
	NTOMS_WIFITYPE_WPA_CCMK,
}EM_NTOMS_PARAM_WIFITYPE;


/*******************************
 * Global Function Declaration
 *******************************/

extern int NTOMS_proInit();

extern void NTOMS_ProDealLockTerminal();


#endif // end of USE_NTOMS

#endif //_NTOMS_PROCESS_H_

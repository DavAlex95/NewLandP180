/***************************************************************************
** Copyright (c) 2023 Newland Payment Technology Co., Ltd All right reserved
** File name: ntoms_callback.c
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

#ifdef USE_NTOMS

#include "ntoms_process.h"
#include "ntoms_callback.h"

#include "cJSON.h"

#include "apiinc.h"
#include "appinc.h"
#include "libapiinc.h"


/*******************************
 * Macro Definition
 *******************************/

#define ASSERT_NTOMS_PARAM_FAIL(e, m) \
if (e != APP_SUCC) { \
	PubMsgDlg(tr("NTOMS PARAM"), m, 1, 3); \
	return APP_FAIL; \
}

/*******************************
 * Local Function Declaration
 *******************************/

static int CheckJsonNode(cJSON *CheckNode, int nType);
static int ParseTomsParam(cJSON *MainNode, const char *pszStoragePath, const char *szAppName);
static int GetTomsParamValueFromJson(cJSON *ParamNode, char * pszInStr, void *pOut, EM_NTOMS_PARAM_TYPE emOutType);
static int GetCommTypeByFlag(EM_NTOMS_PARAM_COMMTYPE emFlag);
static int GetWifiTypeByFlag(EM_NTOMS_PARAM_COMMTYPE emFlag);
static int DispPrompt(char* pszTitle, char* pszData);


/*******************************
 * Local Function Definition
 *******************************/
/**
* @brief	Check if JSON node is valid.
* @param [in] CheckNode		cJSON node to be checked
* @param [in] nType			Type cJSON node should be
* @return
* @li APP_FAIL JSON node is invalid
* @li APP_SUCC JSON node is valid
* @date 2020-07-10
*/
static int CheckJsonNode(cJSON *CheckNode, int nType)
{
	if(CheckNode == NULL)
	{
		TRACE("JSON node is NULL.");
		return APP_FAIL;
	}
	else if(CheckNode->type != nType)
	{
		TRACE("JSON node type mismatching. expect_type=%d, actual_type = %d", nType, CheckNode->type);
		return APP_FAIL;
	}
	else
	{
		switch(CheckNode->type)
		{
			case cJSON_String:
				if(CheckNode->valuestring == NULL)
				{
					TRACE("JSON node has no target value. CheckNode->type=%d", CheckNode->type);
					return APP_FAIL;
				}
				break;
			case cJSON_Array:
			case cJSON_Object:
				if(CheckNode->child == NULL)
				{
					TRACE("JSON node has no target value. CheckNode->type=%d", CheckNode->type);
					return APP_FAIL;
				}
				break;

			default:
				break;
		}
	}

	return APP_SUCC;
}

/**
* @brief	Parse all TOMS parameters and store them.
* @param [in] MainNode    JSON node contains parameter values
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2020-07-10
*/
static int ParseTomsParam(cJSON *MainNode, const char *pszStoragePath, const char *szAppName)
{
	STAPPCOMMPARAM stAppCommParam;
	STAPPPOSPARAM stAppPosParam;
	char szLabelValue[10+1] = {0};
	char szInfo[128] = {0};
	char szCmpVer[10] = {0};
	cJSON *BodyNode = NULL;
	cJSON *ParamNode = NULL;
	char *pszFileRelativePath = NULL;
	char szFilePath[256] = {0};

	BodyNode = cJSON_GetObjectItem(MainNode, NTOMSTAG_BODY);
	if(CheckJsonNode(BodyNode, cJSON_Object) != APP_SUCC)
	{
		TRACE("Fail to parse body node.");
	}

	ParamNode = cJSON_GetObjectItem(BodyNode, NTOMSTAG_PARAMVALUES);
	if(CheckJsonNode(ParamNode, cJSON_Object) != APP_SUCC)
	{
		TRACE("Fail to parse param node.");
	}

	GetTomsParamValueFromJson(ParamNode, NTOMSTAG_VERSION, szCmpVer, NTOMSTYPE_STRING);

	strcpy(szFilePath, pszStoragePath);
	pszFileRelativePath = szFilePath + strlen(szFilePath);

	GetAppCommParam(&stAppCommParam);
	GetAppPosParam(&stAppPosParam);

	/* basic param */
	GetTomsParamValueFromJson(ParamNode, NTOMSTAG_MERCHANTID, stAppPosParam.szMerchantId, NTOMSTYPE_STRING);
	GetTomsParamValueFromJson(ParamNode, NTOMSTAG_TERMINALID, stAppPosParam.szTerminalId, NTOMSTYPE_STRING);
	GetTomsParamValueFromJson(ParamNode, NTOMSTAG_MERCHANTNAMEEN, stAppPosParam.szMerchantNameEn, NTOMSTYPE_STRING);
	GetTomsParamValueFromJson(ParamNode, NTOMSTAG_MERCHANTADDR1, stAppPosParam.szMerchantAddr[0], NTOMSTYPE_STRING);
	GetTomsParamValueFromJson(ParamNode, NTOMSTAG_MERCHANTADDR2, stAppPosParam.szMerchantAddr[1], NTOMSTYPE_STRING);
	GetTomsParamValueFromJson(ParamNode, NTOMSTAG_MERCHANTADDR3, stAppPosParam.szMerchantAddr[2], NTOMSTYPE_STRING);

	/* transaction control */
	if (GetTomsParamValueFromJson(ParamNode, NTOMSTAG_SUPPORTSALE, szLabelValue, NTOMSTYPE_BOOLEAN) == APP_SUCC)
	{
		(strcmp(szLabelValue, NTOMS_BOOL_TURE) == 0) ? SetTransSwitchOnoff(TRANS_SALE, YES) : SetTransSwitchOnoff(TRANS_SALE, NO);
	}

	if (GetTomsParamValueFromJson(ParamNode, NTOMSTAG_SUPPORTVOIDSALE, szLabelValue, NTOMSTYPE_BOOLEAN) == APP_SUCC)
	{
		(strcmp(szLabelValue, NTOMS_BOOL_TURE) == 0) ? SetTransSwitchOnoff(TRANS_VOID, YES) : SetTransSwitchOnoff(TRANS_VOID, NO);
	}

	if (GetTomsParamValueFromJson(ParamNode, NTOMSTAG_SUPPORTREFUND, szLabelValue, NTOMSTYPE_BOOLEAN) == APP_SUCC)
	{
		(strcmp(szLabelValue, NTOMS_BOOL_TURE) == 0) ? SetTransSwitchOnoff(TRANS_REFUND, YES) : SetTransSwitchOnoff(TRANS_REFUND, NO);
	}

	if (GetTomsParamValueFromJson(ParamNode, NTOMSTAG_SUPPORTBALANCE, szLabelValue, NTOMSTYPE_BOOLEAN) == APP_SUCC)
	{
		(strcmp(szLabelValue, NTOMS_BOOL_TURE) == 0) ? SetTransSwitchOnoff(TRANS_BALANCE, YES) : SetTransSwitchOnoff(TRANS_BALANCE, NO);
	}

	if (GetTomsParamValueFromJson(ParamNode, NTOMSTAG_SUPPORTPREAUTH, szLabelValue, NTOMSTYPE_BOOLEAN) == APP_SUCC)
	{
		(strcmp(szLabelValue, NTOMS_BOOL_TURE) == 0) ? SetTransSwitchOnoff(TRANS_PREAUTH, YES) : SetTransSwitchOnoff(TRANS_PREAUTH, NO);
	}

	if (GetTomsParamValueFromJson(ParamNode, NTOMSTAG_SUPPORTVOIDPEAUTH, szLabelValue, NTOMSTYPE_BOOLEAN) == APP_SUCC)
	{
		(strcmp(szLabelValue, NTOMS_BOOL_TURE) == 0) ? SetTransSwitchOnoff(TRANS_VOID_PREAUTH, YES) : SetTransSwitchOnoff(TRANS_VOID_PREAUTH, NO);
	}

	if (GetTomsParamValueFromJson(ParamNode, NTOMSTAG_SUPPORTAUTHSALE, szLabelValue, NTOMSTYPE_BOOLEAN) == APP_SUCC)
	{
		(strcmp(szLabelValue, NTOMS_BOOL_TURE) == 0) ? SetTransSwitchOnoff(TRANS_AUTHCOMP, YES) : SetTransSwitchOnoff(TRANS_AUTHCOMP, NO);
	}

	if (GetTomsParamValueFromJson(ParamNode, NTOMSTAG_SUPPORTVOIDAUTHSALE, szLabelValue, NTOMSTYPE_BOOLEAN) == APP_SUCC)
	{
		(strcmp(szLabelValue, NTOMS_BOOL_TURE) == 0) ? SetTransSwitchOnoff(TRANS_VOID_AUTHSALE, YES) : SetTransSwitchOnoff(TRANS_VOID_AUTHSALE, NO);
	}

	if (GetTomsParamValueFromJson(ParamNode, NTOMSTAG_SUPPORTADJUST, szLabelValue, NTOMSTYPE_BOOLEAN) == APP_SUCC)
	{
		(strcmp(szLabelValue, NTOMS_BOOL_TURE) == 0) ? SetTransSwitchOnoff(TRANS_ADJUST, YES) : SetTransSwitchOnoff(TRANS_ADJUST, NO);
	}

	GetTransSwitchValue(stAppPosParam.sTransSwitch);

	/* system param */
	if(GetTomsParamValueFromJson(ParamNode, NTOMSTAG_TRANS_TRACENO, szLabelValue, NTOMSTYPE_STRING) == APP_SUCC)
	{
		SetVarTraceNo(szLabelValue);
	}

	if(GetTomsParamValueFromJson(ParamNode, NTOMSTAG_TRANS_BATCHNO, szLabelValue, NTOMSTYPE_STRING) == APP_SUCC)
	{
		SetVarBatchNo(szLabelValue);
	}

	if (GetTomsParamValueFromJson(ParamNode, NTOMSTAG_ISPINPAD, szLabelValue, NTOMSTYPE_BOOLEAN) == APP_SUCC)
	{
		stAppPosParam.cIsPinPad = (strcmp(szLabelValue, NTOMS_BOOL_TURE) == 0) ? YES : NO;
	}

	GetTomsParamValueFromJson(ParamNode, NTOMSTAG_MAXTRANSCNT, stAppPosParam.szMaxTransCount, NTOMSTYPE_NUMBER);

	/* communication param */
	GetTomsParamValueFromJson(ParamNode, NTOMSTAG_COMM_NII, stAppCommParam.szNii, NTOMSTYPE_STRING);
	memset(szLabelValue, 0, sizeof(szLabelValue));
	if (GetTomsParamValueFromJson(ParamNode, NTOMSTAG_COMM_TYPE, szLabelValue, NTOMSTYPE_NUMBER) == APP_SUCC)
	{
		stAppCommParam.cCommType = GetCommTypeByFlag(szLabelValue[0]);
	}
	memset(szLabelValue, 0, sizeof(szLabelValue));
	if (GetTomsParamValueFromJson(ParamNode, NTOMSTAG_COMM_TPDU, szLabelValue, NTOMSTYPE_NUMBER) == APP_SUCC)
	{
		PubAscToHex((uchar *)szLabelValue, 10, 0, (uchar *)stAppCommParam.sTpdu);
	}

	switch (stAppCommParam.cCommType)
	{
		case COMM_ETH:
			ASSERT_NTOMS_PARAM_FAIL(PubGetHardwareSuppot(HARDWARE_SUPPORT_ETH, NULL), "No Support ETH");
			GetTomsParamValueFromJson(ParamNode, NTOMSTAG_COMM_IP1_E, stAppCommParam.szIp1, NTOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, NTOMSTAG_COMM_PORT1_E, stAppCommParam.szPort1, NTOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, NTOMSTAG_COMM_IP2_E, stAppCommParam.szIp2, NTOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, NTOMSTAG_COMM_PORT2_E, stAppCommParam.szPort2, NTOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, NTOMSTAG_COMM_IPADDR_E, stAppCommParam.szIpAddr, NTOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, NTOMSTAG_COMM_MASK_E, stAppCommParam.szMask, NTOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, NTOMSTAG_COMM_GATE_E, stAppCommParam.szGate, NTOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, NTOMSTAG_COMM_TIMEOUT, &(stAppCommParam.cTimeOut), NTOMSTYPE_NUMBER);
			if (GetTomsParamValueFromJson(ParamNode, NTOMSTAG_COMM_ISDHCP_E, szLabelValue, NTOMSTYPE_BOOLEAN) == APP_SUCC)
			{
				stAppCommParam.cIsDHCP = (strcmp(szLabelValue, NTOMS_BOOL_TURE) == 0) ? 1 : 0;
			}
			break;

		case COMM_DIAL:
			ASSERT_NTOMS_PARAM_FAIL(PubGetHardwareSuppot(HARDWARE_SUPPORT_MODEM, NULL), "No Support DIAL");
			GetTomsParamValueFromJson(ParamNode, NTOMSTAG_COMM_PREDIALNO, &(stAppCommParam.cReDialNum), NTOMSTYPE_NUMBER);
			GetTomsParamValueFromJson(ParamNode, NTOMSTAG_COMM_PREPHONE, stAppCommParam.szPreDial, NTOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, NTOMSTAG_COMM_TELNO1, stAppCommParam.szTelNum1, NTOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, NTOMSTAG_COMM_TELNO2, stAppCommParam.szTelNum2, NTOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, NTOMSTAG_COMM_TELNO3, stAppCommParam.szTelNum3, NTOMSTYPE_STRING);
			if (GetTomsParamValueFromJson(ParamNode, NTOMSTAG_COMM_ISPREDIAL, szLabelValue, NTOMSTYPE_BOOLEAN) == APP_SUCC)
			{
				stAppCommParam.cPreDialFlag = (szLabelValue[0] == 'Y') ? 1 : 0;
			}
			break;

		case COMM_GPRS:
			ASSERT_NTOMS_PARAM_FAIL((PubGetHardwareSuppot(HARDWARE_SUPPORT_WIRELESS, szInfo) || memcmp(szInfo, "GPRS", 4)), "No Support GPRS");
			GetTomsParamValueFromJson(ParamNode, NTOMSTAG_COMM_WIRELESSDIAL_G, stAppCommParam.szWirelessDialNum, NTOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, NTOMSTAG_COMM_IP1_G, stAppCommParam.szIp1, NTOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, NTOMSTAG_COMM_PORT1_G, stAppCommParam.szPort1, NTOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, NTOMSTAG_COMM_IP2_G, stAppCommParam.szIp2, NTOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, NTOMSTAG_COMM_PORT2_G, stAppCommParam.szPort2, NTOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, NTOMSTAG_COMM_APN1, stAppCommParam.szAPN1, NTOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, NTOMSTAG_COMM_USER_G, stAppCommParam.szUser, NTOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, NTOMSTAG_COMM_PWD_G, stAppCommParam.szPassWd, NTOMSTYPE_STRING);
			if (GetTomsParamValueFromJson(ParamNode, NTOMSTAG_COMM_MODE_G, szLabelValue, NTOMSTYPE_BOOLEAN) == APP_SUCC)
			{
				stAppCommParam.cMode = (strcmp(szLabelValue, NTOMS_BOOL_FALSE) == 0) ? 1 : 0;  //N - alive
			}
			break;

		case COMM_CDMA:
			ASSERT_NTOMS_PARAM_FAIL((PubGetHardwareSuppot(HARDWARE_SUPPORT_WIRELESS, szInfo) || memcmp(szInfo, "CDMA", 4)), "No Support CDMA");
			GetTomsParamValueFromJson(ParamNode, NTOMSTAG_COMM_IP1_C, stAppCommParam.szIp1, NTOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, NTOMSTAG_COMM_PORT1_C, stAppCommParam.szPort1, NTOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, NTOMSTAG_COMM_IP2_C, stAppCommParam.szIp2, NTOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, NTOMSTAG_COMM_PORT2_C, stAppCommParam.szPort2, NTOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, NTOMSTAG_COMM_USER_C, stAppCommParam.szUser, NTOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, NTOMSTAG_COMM_PWD_C, stAppCommParam.szPassWd, NTOMSTYPE_STRING);
			if (GetTomsParamValueFromJson(ParamNode, NTOMSTAG_COMM_MODE_G, szLabelValue, NTOMSTYPE_BOOLEAN) == APP_SUCC)
			{
				stAppCommParam.cMode = (strcmp(szLabelValue, NTOMS_BOOL_FALSE) == 0) ? 1 : 0;  //N - alive
			}
			break;

		case COMM_RS232:
			ASSERT_NTOMS_PARAM_FAIL(PubGetHardwareSuppot(HARDWARE_SUPPORT_COMM_NUM, NULL), "No Support SERIAL");
			break;

		case COMM_WIFI:
			ASSERT_NTOMS_PARAM_FAIL(PubGetHardwareSuppot(HARDWARE_SUPPORT_WIFI, NULL), "No Support WIFI");
			GetTomsParamValueFromJson(ParamNode, NTOMSTAG_COMM_IP1_W, stAppCommParam.szIp1, NTOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, NTOMSTAG_COMM_PORT1_W, stAppCommParam.szPort1, NTOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, NTOMSTAG_COMM_IP2_W, stAppCommParam.szIp2, NTOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, NTOMSTAG_COMM_PORT2_W, stAppCommParam.szPort2, NTOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, NTOMSTAG_COMM_IPADDR_W, stAppCommParam.szIpAddr, NTOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, NTOMSTAG_COMM_MASK_W, stAppCommParam.szMask, NTOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, NTOMSTAG_COMM_GATE_W, stAppCommParam.szGate, NTOMSTYPE_STRING);
			memset(szLabelValue, 0, sizeof(szLabelValue));
			if (GetTomsParamValueFromJson(ParamNode, NTOMSTAG_COMM_WIFIMODE, szLabelValue, NTOMSTYPE_NUMBER) == APP_SUCC)
			{
				stAppCommParam.cWifiMode = GetWifiTypeByFlag(szLabelValue[0]);

				switch(stAppCommParam.cWifiMode)
				{
					case 1:
						stAppCommParam.cWifiMode = WIFI_AUTH_OPEN;
						break;
					case 2:
						stAppCommParam.cWifiMode = WIFI_AUTH_WEP_PSK;
						break;
					case 3:
						stAppCommParam.cWifiMode = WIFI_AUTH_WPA_PSK;
						break;
					case 4:
						stAppCommParam.cWifiMode = WIFI_AUTH_WPA2_PSK;
						break;
					case 5:
						stAppCommParam.cWifiMode = WIFI_AUTH_WPA_WPA2_MIXED_PSK; // ?
						break;
					default:
						ASSERT_NTOMS_PARAM_FAIL(APP_FAIL, "Wifi mode error");
						return APP_FAIL;
				}
			}
			memset(szLabelValue, 0, sizeof(szLabelValue));
			if (GetTomsParamValueFromJson(MainNode, NTOMSTAG_COMM_ISDHCP_W, szLabelValue, NTOMSTYPE_BOOLEAN) == APP_SUCC)
			{
				stAppCommParam.cIsDHCP = (szLabelValue[0] == 'Y') ? 1 : 0;
			}
			GetTomsParamValueFromJson(ParamNode, NTOMSTAG_COMM_WIFISSID, stAppCommParam.szWifiSsid, NTOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, NTOMSTAG_COMM_WIFIKEY, stAppCommParam.szWifiKey, NTOMSTYPE_STRING);
			break;
		default:
			ASSERT_NTOMS_PARAM_FAIL(APP_FAIL, "Comm Type INVALID");
			return APP_FAIL;
	}

	//key passwd
	GetTomsParamValueFromJson(ParamNode, NTOMSTAG_MAINKEYNO, stAppPosParam.szMainKeyNo, NTOMSTYPE_STRING);
	memset(szLabelValue, 0, sizeof(szLabelValue));
	if (GetTomsParamValueFromJson(ParamNode, NTOMSTAG_ENCRYMODE, szLabelValue, NTOMSTYPE_STRING) == APP_SUCC)
	{
		if (szLabelValue[0] == '1')
		{
			stAppPosParam.cEncyptMode = DESMODE_3DES;
		}
		else
		{
			stAppPosParam.cEncyptMode = DESMODE_DES;
		}
	}
	GetTomsParamValueFromJson(ParamNode, NTOMSTAG_ADMINPWD, stAppPosParam.szAdminPwd, NTOMSTYPE_STRING);
	GetTomsParamValueFromJson(ParamNode, NTOMSDEFAULT_FUNCTION, stAppPosParam.szFuncPwd, NTOMSTYPE_STRING);

	// print settings
	GetTomsParamValueFromJson(ParamNode, NTOMSTAG_PNTTITLE, stAppPosParam.szPntTitleEn, NTOMSTYPE_STRING);
	memset(szLabelValue, 0, sizeof(szLabelValue));
	if (GetTomsParamValueFromJson(ParamNode, NTOMSTAG_PNTPAGECNT, szLabelValue, NTOMSTYPE_STRING) == APP_SUCC)
	{
		stAppPosParam.cPrintPageCount = szLabelValue[0];
	}
	memset(szLabelValue, 0, sizeof(szLabelValue));
	if (GetTomsParamValueFromJson(ParamNode, NTOMSTAG_FONTSIZE, szLabelValue, NTOMSTYPE_STRING) == APP_SUCC)
	{
		if (szLabelValue[0] == 'S')
		{
			stAppPosParam.cFontSize = 16;
		}
		else if (szLabelValue[0] == 'M')
		{
			stAppPosParam.cFontSize = 24;
		}
		else if (szLabelValue[0] == 'B')
		{
			stAppPosParam.cFontSize = 32;
		}
	}
	memset(szLabelValue, 0, sizeof(szLabelValue));
	if (GetTomsParamValueFromJson(ParamNode, NTOMSTAG_PNTDETAIL, szLabelValue, NTOMSTYPE_BOOLEAN) == APP_SUCC)
	{
		stAppPosParam.cIsPntDetail = (szLabelValue[0] == 'Y') ? YES : NO;
	}
	memset(szLabelValue, 0, sizeof(szLabelValue));
	if (GetTomsParamValueFromJson(ParamNode, NTOMSTAG_ISPREAUTHSHIELDPAN, szLabelValue, NTOMSTYPE_BOOLEAN) == APP_SUCC)
	{
		stAppPosParam.cIsPreauthShieldPan = (szLabelValue[0] == 'Y') ? YES : NO;
	}

	if(GetTomsParamValueFromJson(ParamNode, NTOMSTAG_TESTFILE, pszFileRelativePath, NTOMSTYPE_BOOLEAN) == APP_SUCC)
	{
		char szContent[256] = {0};
		FILE* fd = NULL;
		PubDebug("pszFileRelativePath=%s", pszFileRelativePath);
		PubDebug("szFilePath=%s", szFilePath);

		fd = fopen(szFilePath, "r");
		if (fd)
		{
			fread(szContent, sizeof(char), 256, fd);
			fclose(fd);
		}
		PubMsgDlg("TEST FILE", szContent, 0, 60);
	}

	UpdateAppPosParam(FILE_APPPOSPARAM, stAppPosParam);
	UpdateAppCommParam(FILE_APPCOMMPARAM, stAppCommParam);
	CommInit();

	return APP_SUCC;
}


/**
* @brief	Return COMM_TYPE according to intput flag
* @param [in] pszFlag		Input flag
* @return
* @li Ref to type COMM_DIAL
* @date 2020-07-10
*/
static int GetCommTypeByFlag(EM_NTOMS_PARAM_COMMTYPE emFlag)
{
	switch(emFlag)
	{
		case NTOMS_COMMTYPE_DAIL:
			return COMM_DIAL;
			break;
		case NTOMS_COMMTYPE_ETH:
			return COMM_ETH;
			break;
		case NTOMS_COMMTYPE_CDMA:
			return COMM_CDMA;
			break;
		case NTOMS_COMMTYPE_GPRS:
			return COMM_GPRS;
			break;
		case NTOMS_COMMTYPE_RS232:
			return COMM_RS232;
			break;
		case NTOMS_COMMTYPE_WIFI:
			return COMM_WIFI;
			break;
		default:
			return COMM_NONE;
			break;
	}
}


/**
* @brief	Get parameter's value from TOMS Json
* @param [in] ParamNode    "paramValues" node
* @param [in] pszInStr		Key string
* @param [in] emOutType		Output value type
* @param [out] pOut			Output value
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2020-07-10
*/
static int GetTomsParamValueFromJson(cJSON *ParamNode, char * pszInStr, void *pOut, EM_NTOMS_PARAM_TYPE emOutType)
{
	cJSON *TargetNode = NULL;
	cJSON *TypeNode = NULL;
	cJSON *ValueNode = NULL;
	cJSON *SubValueNode = NULL;
	char szOutTypeStr[10+1] = {0};
	int nType = cJSON_NULL;

	//TRACE("=== Start parse param <%s> ===", pszInStr);
	TargetNode = cJSON_GetObjectItem(ParamNode, pszInStr);
	ASSERT_FAIL(CheckJsonNode(TargetNode, cJSON_Object));

	TypeNode = cJSON_GetObjectItem(TargetNode, NTOMSTAG_TYPE);
	ASSERT_FAIL(CheckJsonNode(TypeNode, cJSON_String));
	switch(emOutType)
	{
		case NTOMSTYPE_STRING:
			strcpy(szOutTypeStr, NTOMSTYPE_STR_STRING);
			nType = cJSON_String;
			break;

		case NTOMSTYPE_BOOLEAN:
			strcpy(szOutTypeStr, NTOMSTYPE_STR_BOOLEAN);
			nType = cJSON_String;
			break;

		case NTOMSTYPE_NUMBER:
			strcpy(szOutTypeStr, NTOMSTYPE_STR_NUMBER);
			nType = cJSON_String;
			break;

		case NTOMSTYPE_HEX:
			strcpy(szOutTypeStr, NTOMSTYPE_STR_HEX);
			nType = cJSON_String;
			break;

		case NTOMSTYPE_REFERENCE:
			strcpy(szOutTypeStr, NTOMSTYPE_STR_REFERENCE);
			nType = cJSON_Object;
			break;
		case NTOMSTYPE_FILE:
			strcpy(szOutTypeStr, NTOMSTYPE_STR_FILE);
			nType = cJSON_String;
			break;
		default:
			TRACE("Output type out of range. [emOutType=%d]", emOutType);
			return APP_FAIL;
			break;
	}
	if(strcmp(TypeNode->valuestring, szOutTypeStr) != 0)
	{
		TRACE("Output type mismatch. [TypeNode->valuestring=%s] [szOutTypeStr=%s]", TypeNode->valuestring, szOutTypeStr);
		return APP_FAIL;
	}

	ValueNode = cJSON_GetObjectItem(TargetNode, NTOMSTAG_VALUE);
	ASSERT_FAIL(CheckJsonNode(ValueNode, cJSON_Array));

	SubValueNode = cJSON_GetArrayItem(ValueNode->child, 1);
	ASSERT_FAIL(CheckJsonNode(SubValueNode, nType));
	switch(emOutType)
	{
		case NTOMSTYPE_BOOLEAN:
		case NTOMSTYPE_STRING:
		case NTOMSTYPE_HEX:
		case NTOMSTYPE_FILE:
			strcpy(pOut, SubValueNode->valuestring);
			TRACE("Param parsed. [Value=%s].", pOut);
			break;

		case NTOMSTYPE_NUMBER:
			*((int*)pOut) = atoi(SubValueNode->valuestring);
			TRACE("Param parsed. [Value=%d].", *((int*)pOut));
			break;

		case NTOMSTYPE_REFERENCE:
			pOut = SubValueNode->child;
			TRACE("Param parsed. Value is an pointer.");
			break;

		default:
			TRACE("Output type out of range. [emOutType=%d]", emOutType);
			return APP_FAIL;
			break;
	}

	return APP_SUCC;
}


/**
* @brief	Return COMM_TYPE according to intput flag
* @param [in] pszFlag		Input flag
* @return
* @li Ref to type COMM_DIAL
* @date 2020-07-10
*/
static int GetWifiTypeByFlag(EM_NTOMS_PARAM_COMMTYPE emFlag)
{
	switch(emFlag)
	{
		case NTOMS_WIFITYPE_OPEN:
			return WIFI_AUTH_OPEN;
			break;

		case NTOMS_WIFITYPE_WEP:
			return WIFI_AUTH_WEP_PSK;
			break;

		case NTOMS_WIFITYPE_WPA_PSK:
			return WIFI_AUTH_WPA_PSK;
			break;

		case NTOMS_WIFITYPE_WPA2_PSK:
			return WIFI_AUTH_WPA2_PSK;
			break;

		case NTOMS_WIFITYPE_WPA_CCMK:
			return WIFI_AUTH_WPA_WPA2_MIXED_PSK;
			break;

		default:
			return 5;
			break;
	}
}

static int DispPrompt(char* pszTitle, char* pszData)
{
	PubClear2To4();
	PubDisplayTitle(pszTitle);
	PubDisplayStr(0, 3, 1, "%s", pszData);
	PubUpdateWindow();
	return APP_SUCC;
}

//********************************************** public function definitions *******************************************

/**
 * @brief callback for ntoms_exec_show_menu_cb in libntoms
 * for example:
	menu = {
			"1. hello",
			"2. OTA"
			}
	cnt = 2;
	You should show these items in your screen, then when user selects one of them, the sequence number will be returned
* @param [in] title
* @param [in] menu array
* @param [in] the count of menu
* @param [in] def selected
* @return
* @li int : user select item number. >= 1
 */
int NTOMS_CB_ShowMenu(char *title, char **menu, int cnt)
{
	int nSelectItem = 1;
	int nStartItem = 1;

	if (cnt <= 0) {
		PubMsgDlg(title, "Nothing to update", 0, 3);
		return NTOMS_ERR_QUIT;
	}

	while(1)
	{
		NAPI_KbFlush();
		if (PubShowMenuItems(title, menu, cnt, &nSelectItem, &nStartItem, 0) != APP_SUCC)
		{
			return NTOMS_ERR_QUIT;
		}
		return nSelectItem;
	}
}

/**
 * @brief callback for ntoms_exec_update_param_cb in libntoms
 * @param szFilePath  file path in our terminal (like /app/share/xxxx/a),  you need to parse this file. it can be json
 * or xml ...
 * @param pszStoragePath you can configure file in parameter like picture, and lib will return the directory then you
 * can get the file by 'strcat' the file name and path
 * @param szAppName
 * @return
 * @li NTOMS_ERRCODE
 */
NTOMS_ERRCODE NTOMS_CB_UpdateAppParam(const char *szFilePath, const char *pszStoragePath, const char* szAppName)
{
	NTOMS_ERRCODE nRet = NTOMS_ERR_FAIL;
	int nFd = 0;
	int nFileSize = 0;
	char IsUpdate = NO;
	char* pFileBuffer = NULL;
	cJSON *MainNode = NULL;

	PubClearAll();
	PubDisplayStrInline(0, 2, tr("Parsing Parameters..."));
	PubDisplayStrInline(0, 4, tr("Please wait..."));
	PubUpdateWindow();

	//TRACE("Start Update Param. szFilePath=%s", szFilePath);
	PubFsFileSize(szFilePath, (uint *)&nFileSize);
	if(nFileSize <= 0)
	{
		TRACE("File size error. nFileSize=%d", nFileSize);
		return NTOMS_ERR_FAIL;
	}

	pFileBuffer = (char*)malloc(sizeof(char) * nFileSize);
	if((nFd=PubFsOpen (szFilePath, "r")) < 0)
	{
		TRACE("Fail to open file. [%s]", szFilePath);
		goto FAIL;
	}

	PubFsSeek(nFd, 0L, SEEK_SET);
	if((nFileSize = PubFsRead(nFd, pFileBuffer, nFileSize)) < 0)
	{
		TRACE("Fail to read file.");
		PubFsClose(nFd);
		goto FAIL;
	}
	PubFsClose(nFd);

	MainNode = cJSON_Parse(pFileBuffer);
	if(CheckJsonNode(MainNode, cJSON_Object) != APP_SUCC)
	{
		TRACE("Fail to parse file buffer.");
		goto FAIL;
	}

	if (ParseTomsParam(MainNode, pszStoragePath, szAppName) != APP_SUCC)
	{
		TRACE("Fail to parse TOMS param.");
		goto FAIL;
	}
	nRet = NTOMS_OK;
	TRACE("TOMS param parse succ.");

	IsUpdate = YES;

FAIL:
	if (MainNode != NULL)
	{
		cJSON_Delete(MainNode);
	}
	if (pFileBuffer != NULL)
	{
		free(pFileBuffer);
	}
	if (IsUpdate == YES)
	{
		PubFsDel(szFilePath);
	}

	return nRet;
}

/**
 * @brief callback for ntoms_exec_output_log_cb in libntoms
 */
void NTOMS_CB_OutputLog(const char *log, int len)
{
	PubBufToAux(log, len);
}

/**
 * @brief callback for ntoms_exec_show_ui_msg_cb
 */
NTOMS_ERRCODE NTOMS_CB_ShowUiMsg(EM_NTOMS_UI_ID_T emUIID, char* title, char* line1, char* line2)
{
	switch(emUIID)
	{
		case NTOMS_UI_PROCESSING:
			if (title == NULL)
			{
				title = "TOMS";
			}
			DispPrompt(title, line1);
			break;
		case NTOMS_UI_ERROR:
			if (title == NULL)
			{
				title = "Result";
			}
			DispPrompt( title, line1);
			PubGetKeyCode(2);
			break;
		case NTOMS_UI_SUCC:
			if (title == NULL)
			{
				title = "Result";
			}
			DispPrompt(title, line1);
			PubGetKeyCode(2);
			break;
		default:
			break;
	}

	return NTOMS_OK;
}

/**
 * @brief callback for ntoms_exec_switch_dlg_cb in libntoms
 */
int NTOMS_CB_SwitchDialog(char *title, char *info, int def)
{
	char cSelect = def;

	if (PubSelectYesOrNo(title, info, NULL, &cSelect) != 0)
	{
		return -1;
	}

	return cSelect == '0' ? 0 : 1;
}

/**
 * @brief callback for ntoms_exec_input_dlg_cb in libntoms
 */
int NTOMS_CB_InputDialog(char *title, char *tip, char *str, int maxLen)
{
	int oLen = 0;

	return PubInputDlg(title, tip, str, &oLen, 0, maxLen, 60, INPUT_MODE_STRING);
}

/**
 * @brief callback for ntoms_exec_demo_mode_cb in libntoms
 */
void NTOMS_CB_DemoMode(int demoMode, char *oid)
{
	SetUserOidSwitch(demoMode);
	SetFuncUserOid(oid, strlen(oid));
}

/**
 * @brief callback for ntoms_cmd_quit_app_cb in libntoms
 */
void NTOMS_CB_AppQuit(char *cause)
{
	//release your app memory or others like quit child-process

	PubMsgDlg(NULL, cause, 3, 3);

	exit(0);
}


#endif // end of USE_NTOMS


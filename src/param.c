/***************************************************************************
** Copyright (c) 2019 Newland Payment Technology Co., Ltd All right reserved
** File name:  param.c
** File indentifier:
** Brief:  Parameter manage module
** Current Verion:  v1.0
** Auther: sunh
** Complete date: 2016-9-21
** Modify record:
** Modify date:
** Version:
** Modify content:
***************************************************************************/

#include "apiinc.h"
#include "libapiinc.h"
#include "appinc.h"
#include "napi_wifi.h"

#define DEFAULT_ADMINPWD 		"88888888"	/**<Admin pwd*/
#define DEFAULT_USRPWD 		"12345678"	/**<Usr pwd*/
#define DEFAULT_FUNCTION 		"1234"	/**<Function pwd*/
#define FILE_PARAM_INI 	  	APP_NAME"PARA.ini"
#define FILE_PARAM_EXTERN   	APP_NAME"PARAEXT"
#define FILE_EMVCODE_INI		"EMVERRCODE.INI"

/**
* define global variable
*/
static STAPPPOSPARAM gstAppPosParam;	/**<App parameter*/
static STSETTLEPARAM gstSettleParam;	/**<Settle parameter*/
static STREVERSALPARAM gstReversalParam;/**<Reversal parameter*/
static STTRANSPARAM gstTransParam;		/**<Bank parameter*/

/**
* Interface function implementation
*/

/**
* @brief Set App check value
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int GenerateAppChkValue(char *pszChkValue)
{
	int	i;
	char cCh=0, sBuf[APPCHKLEN+1] = {0};

	memset(sBuf, 0, sizeof(sBuf));
	strcpy(sBuf, APPCHECKVALUE);

	for(i = 0; i < APPCHKLEN; i++)
	{
		cCh ^= sBuf[i];
	}
	sBuf[i] = cCh;
	memcpy(pszChkValue, sBuf, APPCHKLEN + 1);

	return APP_SUCC;
}

/**
* @brief Check is first run
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int IsFirstRun()
{
	int	i;
	char cCh=0;
	int nLen;
	char szBuf[APPCHKLEN+1]= {0};
	char szTmp[APPCHKLEN+1]= {0};

	strcpy(szTmp, APPCHECKVALUE);
	ASSERT_FAIL(GetTag(FILE_APPPOSPARAM, TAG_APPCHKVALUE, &nLen, szBuf));
	for(i = 0; i < APPCHKLEN; i++)
	{
		cCh ^= szBuf[i];
	}

	if((memcmp(szTmp, szBuf, APPCHKLEN) == 0) && (cCh == szBuf[APPCHKLEN]))
	{
		return APP_SUCC;
	}

	return APP_FAIL;
}

void InitSettleParam()
{
	STSETTLE stSettle;

	memset(&stSettle, 0, sizeof(STSETTLE));
	memset(&gstSettleParam,0,sizeof(gstSettleParam));
	gstSettleParam.nOffLineUnsendNum = 0;
	gstSettleParam.nRecNum = 0;
	gstSettleParam.nHaveReSendNum = 0;
	gstSettleParam.cPrintSettleHalt = NO;
	gstSettleParam.cPrintDetialHalt = NO;
	gstSettleParam.cIsTotalMatch = YES;
	gstSettleParam.cBatchHaltFlag = NO;
	gstSettleParam.cPrintRecordHaltFlag = NO;
	gstSettleParam.cClrSettleDataFlag = NO;
	gstSettleParam.nBatchSum = 0;
	memcpy(gstSettleParam.sSettleDateTime, "\x00\x00\x00\x00\x00", 5);
	SetSettleData(&stSettle);

	InitSettleFile(gstSettleParam);
}

/**
* @brief Initialize default app parameter
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int InitPosDefaultParam()
{
	/**<APP parameter*/
	memset(&gstAppPosParam, 0, sizeof(gstAppPosParam));
	strcpy(gstAppPosParam.szTerminalId, "55555588");
	strcpy(gstAppPosParam.szMerchantId, "000001100100199");
	strcpy(gstAppPosParam.szAppDispname, APP_NAMEDESC);
	strcpy(gstAppPosParam.szMerchantNameEn, "EDC TEST NAME");
	strcpy(gstAppPosParam.szMerchantAddr[0], "TEST MERCHANT ADDR1");
	strcpy(gstAppPosParam.szAcqName, "VISA");
	strcpy(gstAppPosParam.szIssuerName, "VISA");
	strcpy(gstAppPosParam.szCurrencyName, "$");
	gstAppPosParam.cIsPinPad = NO;
	gstAppPosParam.cPinPadAuxNo = PINPAD;
	strcpy(gstAppPosParam.szPinPadTimeOut, "60");
	strcpy(gstAppPosParam.szAdminPwd, DEFAULT_ADMINPWD);
	strcpy(gstAppPosParam.szUsrPwd, DEFAULT_USRPWD);
	strcpy(gstAppPosParam.szFuncPwd, DEFAULT_FUNCTION);
	strcpy(gstAppPosParam.szMainKeyNo, "1");
	gstAppPosParam.cEncyptMode = DESMODE_3DES;
	gstAppPosParam.cPinEncyptMode = PINTYPE_WITHPAN;
	gstAppPosParam.cIsPntDetail = YES;
	gstAppPosParam.cIsPreauthShieldPan = YES;
	gstAppPosParam.cIsPrintErrReport = NO;
	gstAppPosParam.cPrintPageCount = '2';
	strcpy(gstAppPosParam.szMaxTransCount, "300");
	memcpy(gstAppPosParam.sTransSwitch, "\xFF\xFF\xFF\xFF",4);
	SetTransSwitchOnoff(TRANS_ADJUST, NO);
	gstAppPosParam.cIsTipFlag = NO;
	gstAppPosParam.cIsSupportContact = YES;
	gstAppPosParam.cIsCardInput = YES;
	gstAppPosParam.cIsVoidStrip = NO;
	gstAppPosParam.cIsVoidPin = NO;
	gstAppPosParam.cIsNeedCVV2 = YES;
	gstAppPosParam.cDefaultTransType = '1';
	memcpy(gstAppPosParam.szTipRate, "15", 2);
	gstAppPosParam.cIsDispEMV_TVRTSI = NO;
	gstAppPosParam.cIsShieldPan = YES;
	gstAppPosParam.cIsSupportRF = YES;
	gstAppPosParam.cIsExRF = NO;
	gstAppPosParam.cPntTitleMode = '0';
	strcpy(gstAppPosParam.szPntTitleEn, "POS RECEIPT");
	gstAppPosParam.cIsAdminPwd = YES;
	gstAppPosParam.cIsPrintMinus = YES;
	gstAppPosParam.cIsSupportSwipe = YES;
	gstAppPosParam.cIsPrintIso = NO;
	gstAppPosParam.cKeySystemType = '0';
	gstAppPosParam.cLanguage = LANG_EN_US;
	gstAppPosParam.cFontSize = 24;
	gstAppPosParam.cPinPadUsage = PINPADMODE_CARD | PINPADMODE_PIN;
	gstAppPosParam.cTomsObtainCmd = NO;
	gstAppPosParam.cElecSignFlag = NO;
	gstAppPosParam.cPinPadCallbackFlag = 0xFB;
	InitPosParamFile(gstAppPosParam);

	/**<Settle data*/
	InitSettleParam();

	/**<Reversal data*/
	memset(&gstReversalParam, 0, sizeof(STREVERSALPARAM));
	gstReversalParam.cIsReversal = NO;
	InitReversalFile(gstReversalParam);

	/**<Trans data*/
	memset(&gstTransParam,0,sizeof(gstTransParam));
	gstTransParam.cIsLogin = NO;
	strcpy(gstTransParam.szTraceNo, "000001");
	strcpy(gstTransParam.szBatchNo, "000001");
	strcpy(gstTransParam.szInvoiceNo, "000001");
	memcpy(gstTransParam.sLastLoginDateTime, "\x00\x00\x00\x00\x00", 5);
	gstTransParam.nOnlineUpNum = 1;
	memcpy(gstTransParam.szQPSLimit, "\x00\x00\x00\x03\x00\x00", 6);
	InitTransParamFile(gstTransParam);

	return APP_SUCC;
}

// support ini, if fail do nothing
#define ASSERT_PARA_FAIL(e, msg) \
	if ((e) != APP_SUCC)\
	{\
		PubMsgDlg("SET PARAM", msg, 3, 3);\
		PubCloseFile(&nIniHandle);\
		return APP_FAIL;\
	}

/**
* @brief Get parameter fron ini file
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetParamFromIni()
{
	const char *szIniFile = FILE_PARAM_INI;
	int nIniHandle;
	int nValueLen, nValue,nCommValue;
	int nNum_Tradition = TRANSSWITCHNUM;
	int i=0;
	char *pszTitle = "SET PARAM FROM INI";
	char *szName_Tradition[TRANSSWITCHNUM] = {tr("SALE"),tr("VOID"),"REFUND",tr("PREAUTH"), tr("AUTHCOMP"),
	                               tr("ADJUST"),tr("BALANCE"), NULL, tr("VOIDAUTHSALE"), tr("VOIDPREAUTH")};

	STAPPCOMMPARAM stAppCommParam;
	memset(&stAppCommParam, 0, sizeof(STAPPCOMMPARAM));

	GetAppCommParam(&stAppCommParam);
	ASSERT_QUIT(PubOpenFile (szIniFile, "r", &nIniHandle));

	PubClearAll();
	PubDisplayStrInline(0, 3, tr("Processing..."));
	PubUpdateWindow();

	ASSERT_PARA_FAIL(PubGetINIItemStr (nIniHandle, tr("BASE"), "MERCHANTID",  sizeof(gstAppPosParam.szMerchantId) - 1, gstAppPosParam.szMerchantId, &nValueLen), "Get mid failed");
	ASSERT_PARA_FAIL(PubGetINIItemStr (nIniHandle, tr("BASE"), "POSID",  sizeof(gstAppPosParam.szTerminalId) - 1, gstAppPosParam.szTerminalId, &nValueLen), "Get tid failed");
	ASSERT_PARA_FAIL(PubGetINIItemStr (nIniHandle, tr("BASE"), "MERCHANTNAME",  sizeof(gstAppPosParam.szMerchantNameEn) - 1, gstAppPosParam.szMerchantNameEn, &nValueLen), "Get merchant name failed");
	ASSERT_PARA_FAIL(PubGetINIItemStr (nIniHandle, tr("BASE"), "MERCHANTADDR1",  sizeof(gstAppPosParam.szMerchantAddr[0]) - 1, gstAppPosParam.szMerchantAddr[0], &nValueLen), "Get merchant addr1 failed");

	ASSERT_PARA_FAIL(PubGetINIItemStr (nIniHandle, tr("BASE"), "MERCHANTADDR2",  sizeof(gstAppPosParam.szMerchantAddr[1]) - 1, gstAppPosParam.szMerchantAddr[1], &nValueLen), "Get merchant addr2 failed");

	ASSERT_PARA_FAIL(PubGetINIItemStr (nIniHandle, tr("BASE"), "MERCHANTADDR3",  sizeof(gstAppPosParam.szMerchantAddr[2]) - 1, gstAppPosParam.szMerchantAddr[2], &nValueLen), "Get merchant addr3 failed");

	ASSERT_PARA_FAIL(PubGetINIItemStr (nIniHandle, "BASE", "TRACENO",  sizeof(gstTransParam.szTraceNo) - 1, gstTransParam.szTraceNo, &nValueLen), "Get trace failed");
	ASSERT_PARA_FAIL(PubGetINIItemStr (nIniHandle, "BASE", "BATCHNO",  sizeof(gstTransParam.szBatchNo) - 1, gstTransParam.szBatchNo, &nValueLen), "Get batch failed");
	ASSERT_PARA_FAIL(PubGetINIItemInt (nIniHandle, "BASE", "PRTCOUNT",	&nValue), "Get print pageNum failed");
		gstAppPosParam.cPrintPageCount = nValue + '0';


	for (i=0; i<nNum_Tradition; i++)
	{

		if (szName_Tradition[i] == NULL)
		{
			continue;
		}
		ASSERT_PARA_FAIL(PubGetINIItemInt (nIniHandle, "TRANS", szName_Tradition[i], &nValue), "Get trans switch failed");
		if (nValue)
		{
			gstAppPosParam.sTransSwitch[i/8] |= 0x01<<(7-i%8);
		}
		else
		{
			gstAppPosParam.sTransSwitch[i/8] &= ~(0x01<<(7-i%8));
		}
	}
	ASSERT_PARA_FAIL(PubGetINIItemInt (nIniHandle, "TRANS", "TOMSOBTAINCMD",	&nValue), "Get toms obtain cmd flag failed");
		gstAppPosParam.cTomsObtainCmd = nValue + '0';

	ASSERT_PARA_FAIL(PubGetINIItemInt (nIniHandle, "PIN", "PINPAD",	&nValue), "Get pinpad flag failed");
		gstAppPosParam.cIsPinPad = nValue + '0';

	ASSERT_PARA_FAIL(PubGetINIItemInt (nIniHandle, tr("COMM"), "TIMEOUT",  &nValue), "Get timeout failed");
	stAppCommParam.cTimeOut = nValue;

	ASSERT_PARA_FAIL(PubGetINIItemStr (nIniHandle, tr("COMM"), "NII",  sizeof(stAppCommParam.szNii)-1, stAppCommParam.szNii, &nValueLen), "Get Nii failed");

	ASSERT_PARA_FAIL(PubGetINIItemInt (nIniHandle, tr("COMM"), "COMMTYPE",  &nCommValue), "Get comm type failed");
	if (PubGetINIItemInt (nIniHandle, tr("COMM"), "ISDHCP",  &nValue) == APP_SUCC)
	{
		stAppCommParam.cIsDHCP = nValue;
	}

	switch(nCommValue)
	{
	case 0:
		stAppCommParam.cCommType = COMM_DIAL;
		ASSERT_PARA_FAIL(PubGetINIItemInt (nIniHandle, tr("DIAL"), "DIALNUM",  &nValue), "redial err");
		stAppCommParam.cReDialNum = nValue;

		nValueLen = 0;
		memset(stAppCommParam.szPreDial, 0, sizeof(stAppCommParam.szPreDial));
		PubGetINIItemStr (nIniHandle, tr("DIAL"), "PREPHONE",  sizeof(stAppCommParam.szPreDial) - 1, stAppCommParam.szPreDial, &nValueLen);

		ASSERT_PARA_FAIL(PubGetINIItemStr (nIniHandle, tr("DIAL"), "PHONE1",  sizeof(stAppCommParam.szTelNum1) - 1, stAppCommParam.szTelNum1, &nValueLen), "phone1 err");
		ASSERT_PARA_FAIL(PubGetINIItemStr (nIniHandle, tr("DIAL"), "PHONE2",  sizeof(stAppCommParam.szTelNum2) - 1, stAppCommParam.szTelNum2, &nValueLen), "phone2 err");
		ASSERT_PARA_FAIL(PubGetINIItemStr (nIniHandle, tr("DIAL"), "PHONE3",  sizeof(stAppCommParam.szTelNum3) - 1, stAppCommParam.szTelNum3, &nValueLen), "phone3 err");
		break;
	case 1:
		stAppCommParam.cCommType =COMM_ETH;

		ASSERT_PARA_FAIL(PubGetINIItemStr (nIniHandle, tr("LINE"), "SERVERIP1", sizeof(stAppCommParam.szIp1)-1, stAppCommParam.szIp1, &nValue), "ip1 err");
		ASSERT_PARA_FAIL(PubGetINIItemInt (nIniHandle, tr("LINE"), "PORT1",  &nValue), "port1 err");
		sprintf(stAppCommParam.szPort1, "%d", nValue);

		ASSERT_PARA_FAIL(PubGetINIItemStr (nIniHandle, tr("LINE"), "SERVERIP2", sizeof(stAppCommParam.szIp2)-1, stAppCommParam.szIp2, &nValue), "ip2 err");
		ASSERT_PARA_FAIL(PubGetINIItemInt (nIniHandle, tr("LINE"), "PORT2",  &nValue), "port2 err");
		sprintf(stAppCommParam.szPort2, "%d", nValue);

		ASSERT_PARA_FAIL(PubGetINIItemStr (nIniHandle, tr("LINE"), "LOCALIP", sizeof(stAppCommParam.szIpAddr)-1, stAppCommParam.szIpAddr, &nValue), "local ip err");
		ASSERT_PARA_FAIL(PubGetINIItemStr (nIniHandle, tr("LINE"), "MASK", sizeof(stAppCommParam.szMask)-1, stAppCommParam.szMask, &nValue), "mask err");
		ASSERT_PARA_FAIL(PubGetINIItemStr (nIniHandle, tr("LINE"), "GATEWAY", sizeof(stAppCommParam.szGate)-1, stAppCommParam.szGate, &nValue), "gate err");
		break;
	case 2://CDMA
		stAppCommParam.cCommType =COMM_CDMA;

		ASSERT_PARA_FAIL(PubGetINIItemStr (nIniHandle, "CDMA", "SERVERIP1", sizeof(stAppCommParam.szIp1)-1, stAppCommParam.szIp1, &nValue), "ip2 err");
		ASSERT_PARA_FAIL(PubGetINIItemInt (nIniHandle, "CDMA", "PORT1",  &nValue), "port1 err");
		sprintf(stAppCommParam.szPort1, "%d", nValue);

		ASSERT_PARA_FAIL(PubGetINIItemStr (nIniHandle, "CDMA", "SERVERIP2", sizeof(stAppCommParam.szIp2)-1, stAppCommParam.szIp2, &nValue), "port2 err");
		ASSERT_PARA_FAIL(PubGetINIItemInt (nIniHandle, "CDMA", "PORT2",  &nValue), "ip2 err");
		sprintf(stAppCommParam.szPort2, "%d", nValue);

		ASSERT_PARA_FAIL(PubGetINIItemStr (nIniHandle, "CDMA", "USERNAME", sizeof(stAppCommParam.szUser)-1, stAppCommParam.szUser, &nValue), "user err");
		ASSERT_PARA_FAIL(PubGetINIItemStr (nIniHandle, "CDMA", "PWD", sizeof(stAppCommParam.szPassWd)-1, stAppCommParam.szPassWd, &nValue), "pwd err");

		ASSERT_PARA_FAIL(PubGetINIItemInt (nIniHandle,"CDMA", "MODE", &nValue), "ppp mode err");
		stAppCommParam.cMode = nValue;
		break;
	case 3://GPRS
		stAppCommParam.cCommType =COMM_GPRS;
		ASSERT_PARA_FAIL(PubGetINIItemStr (nIniHandle, "GPRS", "SERVERIP1", sizeof(stAppCommParam.szIp1)-1, stAppCommParam.szIp1, &nValue), "ip1 err");
		ASSERT_PARA_FAIL(PubGetINIItemInt (nIniHandle, "GPRS", "PORT1",  &nValue), "port1 err");
		sprintf(stAppCommParam.szPort1, "%d", nValue);

		ASSERT_PARA_FAIL(PubGetINIItemStr (nIniHandle, "GPRS", "SERVERIP2", sizeof(stAppCommParam.szIp2)-1, stAppCommParam.szIp2, &nValue), "ip2 err");
		ASSERT_PARA_FAIL(PubGetINIItemInt (nIniHandle, "GPRS", "PORT2",  &nValue), "port2 err");
		sprintf(stAppCommParam.szPort2, "%d", nValue);
		ASSERT_PARA_FAIL(PubGetINIItemStr (nIniHandle, "GPRS", "APN", sizeof(stAppCommParam.szAPN1)-1, stAppCommParam.szAPN1, &nValue), "apn err");
		strcpy(stAppCommParam.szAPN2, stAppCommParam.szAPN1);
		ASSERT_PARA_FAIL(PubGetINIItemStr (nIniHandle, "GPRS", "USERNAME", sizeof(stAppCommParam.szUser)-1, stAppCommParam.szUser, &nValue), "user err");
		ASSERT_PARA_FAIL(PubGetINIItemStr (nIniHandle, "GPRS", "PWD", sizeof(stAppCommParam.szPassWd)-1, stAppCommParam.szPassWd, &nValue), "pwd err");

		ASSERT_PARA_FAIL(PubGetINIItemInt (nIniHandle,"GPRS", "MODE", &nValue), "ppp mode err");
		stAppCommParam.cMode = nValue;

		break;
	case 4:
		stAppCommParam.cCommType =COMM_RS232;
		break;
	case 5:
		stAppCommParam.cCommType= COMM_WIFI;
		ASSERT_PARA_FAIL(PubGetINIItemStr (nIniHandle, "WIFI", "SERVERIP1", sizeof(stAppCommParam.szIp1)-1, stAppCommParam.szIp1, &nValue), "wifi ip1 err");
		ASSERT_PARA_FAIL(PubGetINIItemInt (nIniHandle, "WIFI", "PORT1",  &nValue), "wifi port1 err");
		sprintf(stAppCommParam.szPort1, "%d", nValue);

		ASSERT_PARA_FAIL(PubGetINIItemStr (nIniHandle, "WIFI", "SERVERIP2", sizeof(stAppCommParam.szIp2)-1, stAppCommParam.szIp2, &nValue), "wifi ip2 err");
		ASSERT_PARA_FAIL(PubGetINIItemInt (nIniHandle, "WIFI", "PORT2",  &nValue), "wifi port2 err");
		sprintf(stAppCommParam.szPort2, "%d", nValue);

		ASSERT_PARA_FAIL(PubGetINIItemStr (nIniHandle, "WIFI", "LOCALIP", sizeof(stAppCommParam.szIpAddr)-1, stAppCommParam.szIpAddr, &nValue), "wifi local ip err");
		ASSERT_PARA_FAIL(PubGetINIItemStr (nIniHandle, "WIFI", "MASK", sizeof(stAppCommParam.szMask)-1, stAppCommParam.szMask, &nValue), "wifi mask err");
		ASSERT_PARA_FAIL(PubGetINIItemStr (nIniHandle, "WIFI", "GATEWAY", sizeof(stAppCommParam.szGate)-1, stAppCommParam.szGate, &nValue), "wifi gate err");

		ASSERT_PARA_FAIL(PubGetINIItemInt (nIniHandle, "WIFI", "MODE",  &nValue), "wifi mode err");
		switch(nValue)
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
			stAppCommParam.cWifiMode = WIFI_AUTH_WPA_WPA2_MIXED_PSK;
			break;
		default:
			PubMsgDlg(pszTitle, "Wifi mode error", 1, 3);
			return APP_QUIT;
		}
		ASSERT_PARA_FAIL(PubGetINIItemStr (nIniHandle, "WIFI", "SSID", sizeof(stAppCommParam.szWifiSsid)-1, stAppCommParam.szWifiSsid, &nValue), "Ssid err");
		ASSERT_PARA_FAIL(PubGetINIItemStr (nIniHandle, "WIFI", "PWD", sizeof(stAppCommParam.szWifiKey)-1, stAppCommParam.szWifiKey, &nValue), "Ssid pwd err");
		break;
	default:
		stAppCommParam.cCommType =COMM_DIAL;
		break;
	}

	PubCloseFile(&nIniHandle);
	PubDelFile(szIniFile);

	UpdateAppPosParam(FILE_APPPOSPARAM, gstAppPosParam);
	UpdateAppCommParam(FILE_APPCOMMPARAM, stAppCommParam);

	PubMsgDlg(pszTitle, tr("Update ok"), 1, 1);

	return APP_SUCC;
}

/**
* @brief Export pos param
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int ExportPosParam()
{
	ASSERT_FAIL(LoadTagParam(FILE_APPSETTLEMENT, (void *)&gstSettleParam));
	SetSettleData(&gstSettleParam.stSettle);
	ASSERT_FAIL(LoadTagParam(FILE_APPREVERSAL, (void *)&gstReversalParam));
	ASSERT_FAIL(LoadTagParam(FILE_APPTRANSPARAM, (void *)&gstTransParam));
	ASSERT_FAIL(LoadTagParam(FILE_APPPOSPARAM, (void *)&gstAppPosParam));

	return APP_SUCC;
}

/**
* @brief Get Is login
* @return
* @li YES
* @li NO
*/
YESORNO GetVarIsLogin()
{
	if (gstTransParam.cIsLogin == YES)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}

/**
* @brief Set login flag
* @param in pFlag YES/NO
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarIsLogin(const YESORNO cFlag)
{
	gstTransParam.cIsLogin = cFlag;
	ASSERT_FAIL(UpdateTagParam(FILE_APPTRANSPARAM, TAG_TRANS_ISLOGIN, 1, &gstTransParam.cIsLogin));

	return APP_SUCC;
}

/**
* @brief Get Trace no
* @param out pszTraceNo Fix 6 bytes
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int GetVarTraceNo(char *pszTraceNo)
{
	memcpy(pszTraceNo, gstTransParam.szTraceNo, 6);

	return APP_SUCC;
}

/**
* @brief Save trace no
* @param in pszTraceNo Fix 6 bytes
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarTraceNo(const char *pszTraceNo)
{
	memcpy(gstTransParam.szTraceNo, pszTraceNo, strlen(pszTraceNo));
	ASSERT_FAIL(UpdateTagParam(FILE_APPTRANSPARAM, TAG_TRANS_TRACENO, strlen(gstTransParam.szTraceNo), gstTransParam.szTraceNo));

	return APP_SUCC;
}

/**
* @brief Increace Trace No
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int IncVarTraceNo()
{
	PubIncNum(gstTransParam.szTraceNo);
	SetVarTraceNo(gstTransParam.szTraceNo);

	return APP_SUCC;
}

/**
* @brief Get Batch No
* @param out pszBatchNo Batch No
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int GetVarBatchNo(char *pszBatchNo)
{
	memcpy(pszBatchNo, gstTransParam.szBatchNo, 6);

	return APP_SUCC;
}

/**
* @brief Set batch No
* @param in pszBatchNo Fix 6 bytes
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarBatchNo(const char *pszBatchNo)
{
	memcpy(gstTransParam.szBatchNo, pszBatchNo, 6);
	ASSERT_FAIL(UpdateTagParam(FILE_APPTRANSPARAM, TAG_TRANS_BATCH, 6, gstTransParam.szBatchNo));

	return APP_SUCC;
}

/**
* @brief Increase Batch no
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int IncVarBatchNo()
{
	PubIncNum(gstTransParam.szBatchNo);
	SetVarBatchNo(gstTransParam.szBatchNo);

	return APP_SUCC;
}

/**
* @brief Get Invoice No
* @param out pszInvoiceNo Fix 6 bytes
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int GetVarInvoiceNo(char *pszInvoiceNo)
{
	memcpy(pszInvoiceNo, gstTransParam.szInvoiceNo, strlen(gstTransParam.szInvoiceNo));
	return APP_SUCC;
}

/**
* @brief Save invoice No
* @param in pszInvoiceNo
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarInvoiceNo(const char *pszInvoiceNo)
{
	memcpy(gstTransParam.szInvoiceNo, pszInvoiceNo, strlen(pszInvoiceNo));
	ASSERT_FAIL(UpdateTagParam(FILE_APPTRANSPARAM, TAG_TRANS_INVOICENO, strlen(gstTransParam.szInvoiceNo), gstTransParam.szInvoiceNo));

	return APP_SUCC;
}

/**
* @brief Increase invoice No
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int IncVarInvoiceNo()
{
	PubIncNum(gstTransParam.szInvoiceNo);
	SetVarInvoiceNo(gstTransParam.szInvoiceNo);

	return APP_SUCC;
}

/**
* @brief Get Is reversal
* @return
* @li YES
* @li NO
*/
YESORNO GetVarIsReversal()
{
	if (gstReversalParam.cIsReversal == YES)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}

/**
* @brief Save Is reversal
* @param in pFlag YES/NO
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarIsReversal(const YESORNO cFlag)
{
	gstReversalParam.cIsReversal = cFlag;
	ASSERT_FAIL(UpdateTagParam(FILE_APPREVERSAL, TAG_REVERSE_FLAG, 1, &gstReversalParam.cIsReversal));

	return APP_SUCC;
}


/**
* @brief Get the num the time had sent
* @param out pnNum: the time had sent
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int GetVarHaveReversalNum(int *pnNum)
{
	*pnNum = gstReversalParam.nHaveReversalNum;

	return APP_SUCC;
}

/**
* @brief Save the time had sent
* @param in nNum: the time had sent
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarHaveReversalNum(const int nNum)
{
	gstReversalParam.nHaveReversalNum = nNum;
	ASSERT_FAIL(UpdateTagParam(FILE_APPREVERSAL, TAG_REVERSE_HADSENDCNT, 1, (char *)&gstReversalParam.nHaveReversalNum));

	return APP_SUCC;
}

/**
* @brief Increase the time had sent
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int IncVarHaveReversalNum()
{
	gstReversalParam.nHaveReversalNum++;
	SetVarHaveReversalNum(gstReversalParam.nHaveReversalNum);

	return APP_SUCC;
}

/**
* @brief Get reversal data
* @param out psData
* @param out pnLen
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int GetReversalData(STREVERSAL *pstReverSal)
{
	memcpy((char *)pstReverSal, (char *)&gstReversalParam.stReversal, sizeof(STREVERSAL));

	return APP_SUCC;
}

/**
* @brief Save reversal data and len
* @param in sData
* @param in nLen
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetReversalData(STREVERSAL *pstReverSal)
{
	memcpy((char *)&gstReversalParam.stReversal, (char *)pstReverSal, sizeof(STREVERSAL));
	ASSERT_FAIL(UpdateReverseData(FILE_APPREVERSAL, gstReversalParam.stReversal));

	return APP_SUCC;
}

/**
* @brief Get TID
* @param out pszValue
* @return void
*/
void GetVarTerminalId(char *pszValue)
{
	memcpy(pszValue, gstAppPosParam.szTerminalId, sizeof(gstAppPosParam.szTerminalId) - 1);
}

/**
* @brief Save TID
* @param out pszValue Max 8 bytes
* @return void
*/
void SetVarTerminalId(char *pszValue)
{
	memcpy(gstAppPosParam.szTerminalId, pszValue, sizeof(gstAppPosParam.szTerminalId) - 1);
	UpdateTagParam(FILE_APPPOSPARAM, TAG_TERMINALID, strlen(gstAppPosParam.szTerminalId), gstAppPosParam.szTerminalId);
}

/**
* @brief Get MID
* @param out pszValue Max 15 bytes
* @return void
*/
void GetVarMerchantId(char *pszValue)
{
	memcpy(pszValue,gstAppPosParam.szMerchantId, sizeof(gstAppPosParam.szMerchantId) - 1);
}

/**
* @brief Get MID
* @param out pszValue
* @return void
*/
void SetVarMerchantId(char *pszValue)
{
	memcpy(gstAppPosParam.szMerchantId, pszValue, sizeof(gstAppPosParam.szMerchantId) - 1);
	UpdateTagParam(FILE_APPPOSPARAM, TAG_MERCHANTID, strlen(gstAppPosParam.szMerchantId), gstAppPosParam.szMerchantId);
}

/**
* @brief Get merchant english name
* @param out pszValue Max 40 bytes
* @return void
*/
void GetVarMerchantNameEn(char *pszValue)
{
	memcpy(pszValue, gstAppPosParam.szMerchantNameEn, sizeof(gstAppPosParam.szMerchantNameEn) - 1);
}

/**
* @brief Get is use pinpad
* @return
* @li YES
* @li NO
*/
YESORNO GetVarIsPinpad()
{
	if (APP_SUCC == PubGetHardwareSuppot(HARDWARE_SUPPORT_PINPAD, NULL))
	{
		if (gstAppPosParam.cIsPinPad == YES)
		{
			return YES;
		}
		else
		{
			return NO;
		}
	}
	else
	{
		return NO;
	}
}

/**
* @brief Get the pinpad port at terminal
* @param out pnValue Port no, 1:AUX1,2:AUX2
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int GetVarPinpadAuxNo(int *pnValue)
{
	*pnValue = gstAppPosParam.cPinPadAuxNo;
	return APP_SUCC;
}

/**
* @brief Get pinpad timeout
* @param out pnValue timeout, 0-999
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int GetVarPinpadTimeOut(int *pnValue)
{
	char szTmp[10]= {0};

	memcpy(szTmp, gstAppPosParam.szPinPadTimeOut, sizeof(gstAppPosParam.szPinPadTimeOut) - 1);
	if (atoi(szTmp) > 0 && atoi(szTmp) < 999)
	{
		*pnValue = atoi(szTmp);
	}
	else
	{
		*pnValue = 60;
	}

	return APP_SUCC;
}

/**
* @brief Get password according to type
* @param out pnValue
* @param in cType See[EM_PWD_TYPE]
* @return void
*/
void GetVarPwd(char *pszValue, uchar cType)
{
	if (EM_TRANS_PWD == cType)
	{
		memcpy(pszValue, gstAppPosParam.szUsrPwd, sizeof(gstAppPosParam.szUsrPwd) - 1);
	}
	else if (EM_SYSTEM_PWD == cType)
	{
		memcpy(pszValue, gstAppPosParam.szAdminPwd, sizeof(gstAppPosParam.szAdminPwd) - 1);
	}
	else if (EM_FUNC_PWD == cType)
	{
		memcpy(pszValue, gstAppPosParam.szFuncPwd, sizeof(gstAppPosParam.szFuncPwd) - 1);
	}
}

/**
* @brief Get mainkey index
* @param out pnValue index no
* @return void
*/
void GetVarMainKeyNo(int *pnValue)
{
	*pnValue = atoi(gstAppPosParam.szMainKeyNo);
}

/**
* @brief Save mainkey index
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetVarMainKeyNo(const int nIndex)
{
	memset(gstAppPosParam.szMainKeyNo, 0, sizeof(gstAppPosParam.szMainKeyNo));
	sprintf(gstAppPosParam.szMainKeyNo, "%d", nIndex);
	ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_MAINKEYNO, strlen(gstAppPosParam.szMainKeyNo), gstAppPosParam.szMainKeyNo));

	return APP_SUCC;
}

/**
* @brief Get encrypt mode
* @return
* @li DESMODE_3DES
* @li DESMODE_DES
*/
int  GetVarEncyptMode()
{
	if (gstAppPosParam.cEncyptMode == DESMODE_3DES)
	{
		return DESMODE_3DES;
	}
	return DESMODE_DES;
}
/**
* @brief Save encrypt mode
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetVarEncyptMode(const char cEncyptMode)
{
	if(DESMODE_3DES == cEncyptMode || DESMODE_DES == cEncyptMode)
	{
		gstAppPosParam.cEncyptMode = cEncyptMode;
		ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_ENCRYMODE, 1, &gstAppPosParam.cEncyptMode));
		return APP_SUCC;
	}

	TRACE("cEncyptMode=%d error",cEncyptMode);
	return APP_FAIL;
}

/**
* @brief Get Pin encrypt mode
* @return
* @li PINTYPE_WITHOUTPAN
* @li PINTYPE_WITHPAN
*/
int GetVarPinEncyptMode()
{
	if (gstAppPosParam.cPinEncyptMode == PINTYPE_WITHOUTPAN)
	{
		return PINTYPE_WITHOUTPAN;
	}
	return PINTYPE_WITHPAN;
}

/**
* @brief Get is display TVR OR TSI during transaction
* @return
* @li YES
* @li NO
*/
YESORNO GetVarIsDispEMV_TVRTSI(void)
{
	if (gstAppPosParam.cIsDispEMV_TVRTSI == YES)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}


/**
* @brief Get is shield pan
* @return
* @li YES
* @li NO
*/
YESORNO GetVarIsShieldPan(void)
{
	if (gstAppPosParam.cIsShieldPan != YES  && gstAppPosParam.cIsShieldPan != NO)
	{
		gstAppPosParam.cIsShieldPan = YES;
	}
	if (gstAppPosParam.cIsShieldPan == YES)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}

/**
* @brief Get is print record
* @return
* @li YES
* @li NO
*/
YESORNO GetVarIsPntDetail()
{
	if (gstAppPosParam.cIsPntDetail == YES)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}

/**
* @brief Get CVV2 flag
* @return
* @li YES
* @li NO
*/
YESORNO GetVarIsNeedCVV2()
{
	if (gstAppPosParam.cIsNeedCVV2 == NO)
	{
		return NO;
	}
	else
	{
		return YES;
	}
}

/**
* @brief Is preauth shield pan
* @return
* @li YES
* @li NO
*/
YESORNO GetVarIsPreauthShieldPan()
{
	if (gstAppPosParam.cIsPreauthShieldPan == YES)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}

/**
* @brief Get is print err report
* @return
* @li YES
* @li NO
*/
YESORNO GetVarIsPrintErrReport()
{
	if (gstAppPosParam.cIsPrintErrReport == YES)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}

/**
* @brief Get the count of print page
* @param out pnValue page count
* @return void
*/
void GetVarPrintPageCount(int *pnValue)
{
	if(gstAppPosParam.cPrintPageCount >= '0' && gstAppPosParam.cPrintPageCount <= '9')
	{
		*pnValue = gstAppPosParam.cPrintPageCount - 0x30;
	}
	else
	{
		*pnValue = 1;
	}
	return;
}

/**
* @brief Get MAX trans count, more than this count , you need to settle
* @param out pnValue 1-99999
* @return void
*/
void GetVarMaxTransCount(int *pnValue)
{
	char szTmp[10] = {0};

	memcpy(szTmp, gstAppPosParam.szMaxTransCount, sizeof(gstAppPosParam.szMaxTransCount) - 1);
	if (atoi(szTmp) > 0 && atoi(szTmp) < 99999)
	{
		*pnValue = atoi(szTmp);
	}
	else
	{
		*pnValue = 1000;
	}
}

/**
* @brief Get Is support tip
* @return
* @li YES support
* @li NO nonsupport
*/
YESORNO GetVarIsTipFlag(void)
{
	if (gstAppPosParam.cIsTipFlag == YES)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}



/**
* @brief Save is support tip
* @param in pFlag YES/NO
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarIsTipFlag(const YESORNO pFlag)
{
	gstAppPosParam.cIsTipFlag = pFlag;
	ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_TIPFLAG, 1, &gstAppPosParam.cIsTipFlag));

	return APP_SUCC;
}

/**
* @brief Get Tip Rate
* @param out pcValue
* @return void
*/
void GetVarTipRate(char *pszValue)
{
	memcpy(pszValue, gstAppPosParam.szTipRate, sizeof(gstAppPosParam.szTipRate) - 1);
}


/**
* @brief Get is support Contact
* @return
* @li YES
* @li NO
*/
YESORNO GetVarIsSupportContact()
{
	if (gstAppPosParam.cIsSupportContact == YES)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}

/**
* @brief Get is support manual
* @return
* @li YES
* @li NO
*/
YESORNO GetVarIsCardInput()
{
	if (gstAppPosParam.cIsCardInput == YES)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}

/**
* @brief Save is support manual
* @param in pFlag YES/NO
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarIsCardInput(const YESORNO cFlag)
{
	gstAppPosParam.cIsCardInput = cFlag;
	ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_CARDINPUTMODE, 1, &gstAppPosParam.cIsCardInput));

	return APP_SUCC;
}

/**
* @brief Get is void need to capture card
* @return
* @li YES need
* @li NO  no need
*/
YESORNO GetVarIsVoidStrip()
{
	if (gstAppPosParam.cIsVoidStrip == YES)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}

/**
* @brief Get void is need to Get Pin
* @return
* @li YES need
* @li NO void no need
*/
YESORNO GetVarIsVoidPin()
{
	if (gstAppPosParam.cIsVoidPin == YES)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}

/**
* @brief Get default transtype
* @param out pcValue '0'-Sale, 1'-Preauth
* @return void
*/
void GetVarDefaultTransType(char *pcValue)
{
	gstAppPosParam.cDefaultTransType = gstAppPosParam.cDefaultTransType == '0' ? '0' : '1';
	*pcValue=gstAppPosParam.cDefaultTransType;
}

/**
* @brief Get QPS Limit
* @param out pcValue
* @return void
*/
void GetVarQPSLimit(char *pszValue)
{
	memcpy(pszValue, gstTransParam.szQPSLimit, sizeof(gstTransParam.szQPSLimit) - 1);
}

static void DeleteLeftZeroFromStr(uchar* pszString)
{
	int i,j;
	int nValidDataFlag = 0;

	if(pszString == NULL)
	{
		return;
	}
	for (i=0,j=0; i<strlen((char*)pszString); i++)
	{
		if (pszString[i] != '0' || nValidDataFlag == 1)
		{
			pszString[j] = pszString[i];
			j++;
			nValidDataFlag = 1;
		}
	}
	pszString[j] = 0;
}

/**
* @brief Set QPS Limit
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionQPSLimit(void)
{
	int nAmtLen = 12;
	char szAscAmt[12+1] = {0};

	PubHexToAsc((uchar* )gstTransParam.szQPSLimit, 12, 0, (uchar* )szAscAmt);
	DeleteLeftZeroFromStr((uchar* )szAscAmt);
	ASSERT_QUIT(PubInputAmount(tr("SET QPS LIMIT"), tr("QPS LIMIT AMOUNT"), szAscAmt, &nAmtLen, INPUT_AMOUNT_MODE_NOT_NONE, 60))
	PubAscToHex((uchar* )szAscAmt, 12, 0, (uchar* )gstTransParam.szQPSLimit);
	ASSERT_FAIL(UpdateTagParam(FILE_APPTRANSPARAM, TAG_TRANS_QPSLIMIT, 6, gstTransParam.szQPSLimit));
	return APP_SUCC;
}

/**
* @brief
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int GetVarEmvOnlineUpNum(int *pnSerial)
{
	*pnSerial = gstTransParam.nOnlineUpNum;
	return APP_SUCC;
}


/**
* @brief
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarEmvOnlineUpNum(const int nSerial)
{
	char szBuf[2+1] = {0};

	gstTransParam.nOnlineUpNum = nSerial;
	PubIntToC2((uchar *)szBuf, gstTransParam.nOnlineUpNum);
	ASSERT_FAIL(UpdateTagParam(FILE_APPTRANSPARAM, TAG_TRANS_ONLINEUPNUM, 2, szBuf));

	return APP_SUCC;
}

/**
* @brief
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int IncVarEmvOnlineUpNum()
{
	gstTransParam.nOnlineUpNum++;
	return APP_SUCC;
}

/**
* @brief Set Mainkey index
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionMainKeyNo(void)
{
	int nLen;
	char szNumber[3+1] = {0};

	strcpy(szNumber, gstAppPosParam.szMainKeyNo);
	while(1)
	{
		ASSERT_QUIT(PubInputDlg(tr("SET KEY INDEX"), tr("ENTER KEY INDEX(1-255):"), szNumber, &nLen, 1, 3, 0, INPUT_MODE_NUMBER));
		if(atoi(szNumber) >= 1 && atoi(szNumber) <= 255)
		{
			break;
		}
		memset(szNumber, 0, sizeof(szNumber));
	}

	strcpy(gstAppPosParam.szMainKeyNo, szNumber);
	ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_MAINKEYNO, strlen(gstAppPosParam.szMainKeyNo), gstAppPosParam.szMainKeyNo));

	return APP_SUCC;
}

/**
* @brief Set print page
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionPrintPageCount(void)
{
	char szNum[2] = {0};
	int nNum, nLen;

	if (PubGetPrinter() != _PRINTTYPE_TP)
	{
		PubMsgDlg(tr("Warning"),tr("NONSUPPORT"), 3, 1);
		return APP_FUNCQUIT;
	}

	while(1)
	{
		szNum[0] = gstAppPosParam.cPrintPageCount;
		ASSERT_RETURNCODE( PubInputDlg(tr("Receipt pages"), tr("Num of page(1-3):"), szNum, &nLen, 1, 1, 0, INPUT_MODE_NUMBER));
		nNum = atoi(szNum);
		if( (nNum >= 1) && (nNum <= 3) )
		{
			break;
		}
		PubMsgDlg(tr("Receipt pages"),tr("INVALID INPUT"), 1, 1);
	}
	gstAppPosParam.cPrintPageCount = '0' + atoi(szNum);
	ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_PNTPAGECNT, 1, &gstAppPosParam.cPrintPageCount));

	return APP_SUCC;
}

/**
* @brief Set max trans count
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionMaxTransCount(void)
{
	int nLen, nNumber;
	char szNumber[5+1] = {0};

	nNumber = atoi(gstAppPosParam.szMaxTransCount);
	while(1)
	{
		sprintf(szNumber, "%d", nNumber);
		ASSERT_RETURNCODE(PubInputDlg(tr("TRANS COUNT"), tr("MAX TRANS COUNT\n(   <=999):"), szNumber, &nLen, 1, 3, 0, INPUT_MODE_NUMBER));
		nNumber = atoi(szNumber);
		if ((nNumber > 0) && (nNumber <= 999))
		{
			break;
		}
		PubMsgDlg(tr("TRANS COUNT"),tr("INVALID INPUT!"), 0, 1);
	}
	sprintf(gstAppPosParam.szMaxTransCount, "%05d", nNumber);
	ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_MAXTRANSCNT, strlen(gstAppPosParam.szMaxTransCount), gstAppPosParam.szMaxTransCount));

	return APP_SUCC;
}

/**
* @brief Set MID
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionMerchantId(void)
{
	int nLen, nRecNum;
	char szMerchantId[15+1] = {0};

	memcpy(szMerchantId, gstAppPosParam.szMerchantId, sizeof(gstAppPosParam.szMerchantId) - 1);
	ASSERT_RETURNCODE(PubInputDlg(tr("MERCHANT ID"), tr("MERCHANT ID(15):"), szMerchantId, &nLen, 10, 15, 0, INPUT_MODE_STRING));
	if (memcmp(szMerchantId, gstAppPosParam.szMerchantId, 15) != 0)
	{
		GetRecordNum(&nRecNum);
		if (nRecNum > 0)
		{
			PubMsgDlg(tr("MERCHANT ID"), tr("PLEASE SETTLE FITSTLY"), 3, 10);
			return APP_FAIL;
		}
		memcpy(gstAppPosParam.szMerchantId, szMerchantId, sizeof(gstAppPosParam.szMerchantId));
		ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_MERCHANTID, strlen(szMerchantId), szMerchantId));
	}

	return APP_SUCC;
}

/**
* @brief Set TID
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionTerminalId(void)
{
	int nLen, nRecNum;
	char szTerminalId[8+1] = {0};

	memcpy(szTerminalId, gstAppPosParam.szTerminalId, sizeof(gstAppPosParam.szTerminalId) - 1);
	ASSERT_RETURNCODE( PubInputDlg(tr("TERMINAL ID"), tr("TERMINAL ID(8):"), szTerminalId, &nLen, 8, 8, 0, INPUT_MODE_STRING));
	if (memcmp(szTerminalId, gstAppPosParam.szTerminalId, 8) !=0)
	{
		GetRecordNum(&nRecNum);
		if (nRecNum > 0)
		{
			PubMsgDlg(tr("TERMINAL ID"),tr("PLEASE SETTLE"), 3, 10);
			return APP_FAIL;
		}
		memcpy(gstAppPosParam.szTerminalId, szTerminalId, sizeof(gstAppPosParam.szTerminalId));
		ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_TERMINALID, strlen(szTerminalId), szTerminalId));
	}

	return APP_SUCC;
}

/**
* @brief Set APP name
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionAppName(void)
{
	int nLen = 0;
	char szTmp[128] = {0};

	strcpy(szTmp, gstAppPosParam.szAppDispname);
	ASSERT_RETURNCODE(PubInputDlg(tr("APP NAME"), NULL, szTmp, &nLen, 0, 14, 0, INPUT_MODE_STRING));
	strcpy(gstAppPosParam.szAppDispname, szTmp);
	ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_SHOWAPPNAME, strlen(gstAppPosParam.szAppDispname), gstAppPosParam.szAppDispname));

	return APP_SUCC;
}


/**
* @brief Set merchant address
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionMerchantAddr(void)
{

	int nLen = 0;
	int i, nTagId;
	int nRet;
	char szContent[32+1] = {0};

	for (i = 1; i <= 3; i++)
	{
		sprintf(szContent, "MERCHANT ADDR(%d):", i);
		nRet = PubInputDlg(tr("MERCHANT"), szContent, gstAppPosParam.szMerchantAddr[i-1], &nLen, 0, 80, 60, INPUT_MODE_STRING);
		if(nRet == KEY_UP)
		{
			if(i <= 1)
			{
				return KEY_UP;
			}
			else
			{
				i -= 2;
				continue;
			}
		}
		else if(nRet == KEY_DOWN)
		{
			continue;
		}
		ASSERT_RETURNCODE(nRet);
		if (i == 1)
		{
			nTagId = TAG_MERCHANTADDR1;
		}
		else if (i == 2)
		{
			nTagId = TAG_MERCHANTADDR2;
		}
		else
		{
			nTagId = TAG_MERCHANTADDR3;
		}
		ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, nTagId, strlen(gstAppPosParam.szMerchantAddr[i-1]), gstAppPosParam.szMerchantAddr[i-1]));
	}

	return APP_SUCC;
}

/**
* @brief Set merchant name
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionMerchantName(void)
{
	int nLen = 0;
	char szTmp[40+1] = {0};

	strcpy(szTmp, gstAppPosParam.szMerchantNameEn);
	ASSERT_RETURNCODE(PubInputDlg(tr("MERCHANT NAME"), NULL, szTmp, &nLen, 0, 40, 0, INPUT_MODE_STRING));
	if (memcmp(szTmp, gstAppPosParam.szMerchantNameEn, sizeof(gstAppPosParam.szMerchantNameEn) - 1) != 0)
	{
		memcpy(gstAppPosParam.szMerchantNameEn, szTmp, sizeof(gstAppPosParam.szMerchantNameEn));
		ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_MERCHANTNAMEEN, strlen(szTmp), szTmp));
	}

	return APP_SUCC;
}

/**
* @brief  Set stan
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionTraceNo(void)
{
	int nLen,nRecNum;
	char szTraceNo[6+1]= {0};

	memcpy(szTraceNo, gstTransParam.szTraceNo, 6);
	ASSERT_RETURNCODE(PubInputDlg(tr("TRACE NO"),tr("ENTER:"), szTraceNo, &nLen, 6, 6, 0, INPUT_MODE_NUMBER));
	if (memcmp(gstTransParam.szTraceNo, szTraceNo, 6) !=0)
	{
		GetRecordNum(&nRecNum);
		if (nRecNum > 0)
		{
			PubMsgDlg(tr("TRACE NO"),tr("PLEASE SETTLE FIRSTLT"), 3, 10);
			return APP_FAIL;
		}
		memcpy(gstTransParam.szTraceNo, szTraceNo, 6);
		ASSERT_FAIL(UpdateTagParam(FILE_APPTRANSPARAM, TAG_TRANS_TRACENO, 6, gstTransParam.szTraceNo));
	}
	return APP_SUCC;
}

/**
* @brief  Set batch no
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionBatchNo(void)
{
	int nLen,nRecNum;
	char szBatchNo[6+1] = {0};

	memcpy(szBatchNo, gstTransParam.szBatchNo, sizeof(gstTransParam.szBatchNo) - 1);
	ASSERT_RETURNCODE(PubInputDlg(tr("BATCH NO"),tr("ENTER:"), szBatchNo, &nLen, 6, 6, 0, INPUT_MODE_NUMBER));

	if (memcmp(gstTransParam.szBatchNo, szBatchNo, 6) !=0)
	{
		GetRecordNum(&nRecNum);
		if (nRecNum > 0)
		{
			PubMsgDlg(tr("BATCH NO"),tr("PLEASE SETTLE FIRST"), 3, 10);
			return APP_FAIL;
		}
		memcpy(gstTransParam.szBatchNo, szBatchNo, 6);
		UpdateTagParam(FILE_APPTRANSPARAM, TAG_TRANS_BATCH, strlen(gstTransParam.szBatchNo), gstTransParam.szBatchNo);
	}

	return APP_SUCC;
}

static char gcIsChkPinpad = YES;/**<Is need to check pinpad*/
static char gcIsChkInside = YES;/**<Is inside pinpad init>*/
static char gcIsChkRF = YES; /**<Is need to check RF(contactless)>*/

/*
* @brief Save Is need to check pinpad
* @param in cIsChk YES/NO
* @return  void
*/
void SetControlChkPinpad(const YESORNO cIsChk)
{
	gcIsChkPinpad = cIsChk;
}
/**
* @brief Save inside pinpad init
* @param in cIsChk YES/NO
* @return  void
*/
void SetControlChkInside(const YESORNO cIsChk)
{
	gcIsChkInside = cIsChk;
}
/**
* @brief
* @param in cIsChk YES/NO
* @return  void
*/
void SetControlChkRF(const YESORNO cIsChk)
{
	gcIsChkRF = cIsChk;
}

/**
* @brief Get is check pinpad
* @return
* @li YES
* @li NO
*/
YESORNO GetControlChkPinpad(void)
{
	if (gcIsChkPinpad == YES)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}
/**
* @brief
* @return
* @li YES
* @li NO
*/
YESORNO GetControlChkInside(void)
{
	if (gcIsChkInside == YES)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}
/**
* @brief
* @return
* @li YES
* @li NO
*/
YESORNO GetControlChkRF(void)
{
	if (gcIsChkRF == YES)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}

/**
* @brief Set is check pinpad
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionIsPinPad(void)
{
	if (APP_SUCC == PubGetHardwareSuppot(HARDWARE_SUPPORT_PINPAD, NULL))
	{
		char* SelMenu[] = {"0.BUILT-IN","1.EXTERNAL"};
		ASSERT_RETURNCODE(PubSelectYesOrNo(tr("PINPAD"),tr("EXTERNAL PINPAD"), SelMenu, &gstAppPosParam.cIsPinPad));
		ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_ISPINPAD, 1, &gstAppPosParam.cIsPinPad));
	}
	else
	{
		gstAppPosParam.cIsPinPad = NO;
		PubMsgDlg(tr("Warning"),tr("BUILDIN SUPPORT ONLY"), 1, 3);
	}
	if (gstAppPosParam.cIsPinPad == YES)
	{
		ASSERT_FAIL(SetFunctionPinpadType());
		ASSERT_FAIL(SetFunctionPinpadTimeOut());
		ASSERT_FAIL(SetFunctionPinpadUsage());
		ASSERT_FAIL(SetFunctionPinpadPort());
		ASSERT_FAIL(SetFuncPinpadCallBack());
	}
	TRACE("GetVarIsPinpadReadCard() = %d", GetVarIsPinpadReadCard());

	return APP_SUCC;
}

/**
* @brief Set is support contact
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionIsContact(void)
{
	ASSERT_RETURNCODE(PubSelectYesOrNo(tr("OTHER"),tr("SUPPORT CONTACT"), NULL, &gstAppPosParam.cIsSupportContact));
	ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_SUPPORTCONTACT, 1, &gstAppPosParam.cIsSupportContact));

	return APP_SUCC;
}

/**
* @brief Set is support manual
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionIsManul(void)
{
	ASSERT_RETURNCODE(PubSelectYesOrNo(tr("OTHER CONTROL"),tr("MANUAL ENABLE"), NULL,&gstAppPosParam.cIsCardInput));
	ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_CARDINPUTMODE, 1, &gstAppPosParam.cIsCardInput));

	return APP_SUCC;
}

/**
* @brief Set is print error report
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionIsPrintErrReport(void)
{
	ASSERT_QUIT(PubSelectYesOrNo(tr("OTHER CONTROL"), tr("PRINT FAULT REPORT"), NULL, &gstAppPosParam.cIsPrintErrReport));
	ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_ISPRINTERRREPORT, 1, &gstAppPosParam.cIsPrintErrReport));

	return APP_SUCC;
}

/**
* @brief Set dafault transaction type
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionDefaultTransType(void)
{
	char *SelMenu[] = {"0.AUTH", "1.SALE"};

	ASSERT_RETURNCODE(PubSelectYesOrNo(tr("TRANS PARAM"),tr("DEFAULT TRANSACTION"), SelMenu, &gstAppPosParam.cDefaultTransType));
	ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_DEFAULTTRANS, 1, &gstAppPosParam.cDefaultTransType));

	return APP_SUCC;
}

/**
* @brief Set void is need to input pin
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionIsVoidPin(void)
{
	ASSERT_RETURNCODE(PubSelectYesOrNo(tr("SET VOID"), tr("Void Input Password"), NULL, &gstAppPosParam.cIsVoidPin));
	ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_ISVOIDPIN, 1, &gstAppPosParam.cIsVoidPin));

	return APP_SUCC;
}

/**
* @brief Set void is need to capture card
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetFunctionIsVoidStrip(void)
{
	ASSERT_RETURNCODE(PubSelectYesOrNo(tr("Capture Card ENABLE"), tr("Void Capture Card"), NULL,&gstAppPosParam.cIsVoidStrip));
	ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_ISSALEVOIDSTRIP, 1, &gstAppPosParam.cIsVoidStrip));

	return APP_SUCC;
}

/**
* @brief Set pin encrypt mode
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionPinEncyptMode(void)
{
	char *selMenu[2] = {tr("0.With Pan"), tr("1.Without Pan")};
	ASSERT_RETURNCODE(PubSelectYesOrNo(tr("Set terminal"), tr("PIN MODE"), selMenu, &gstAppPosParam.cPinEncyptMode));
	if('0' == gstAppPosParam.cPinEncyptMode)
	{
		gstAppPosParam.cPinEncyptMode = PINTYPE_WITHPAN;
	}
	else
	{
		gstAppPosParam.cPinEncyptMode = PINTYPE_WITHOUTPAN;
	}
	ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_PINENCRYMODE, 1, &gstAppPosParam.cPinEncyptMode));

	return APP_SUCC;
}

/**
* @brief Set Preauth is shield pan
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionIsPreauthShieldPan(void)
{
	ASSERT_RETURNCODE(PubSelectYesOrNo(tr("OTHER"), tr("IS PREAUTH SHIELDED PAN"), NULL, &gstAppPosParam.cIsPreauthShieldPan));
	ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_ISPREAUTHSHIELDPAN, 1, &gstAppPosParam.cIsPreauthShieldPan));

	return APP_SUCC;
}

/**
* @brief Set is print detail
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionIsPntDetail(void)
{
	ASSERT_RETURNCODE( PubSelectYesOrNo(tr("SETTLE CONTROL"), tr("PRINT DETAIL"), NULL, &gstAppPosParam.cIsPntDetail));
	ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_PNTDETAIL, 1, &gstAppPosParam.cIsPntDetail));
	return APP_SUCC;
}

/**
* @brief Set is print detail
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionIsNeedCVV2(void)
{
	ASSERT_RETURNCODE( PubSelectYesOrNo(tr("OTHER CONTROL"), tr("CVV2 ENABLE"), NULL, &gstAppPosParam.cIsNeedCVV2));
	ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_CVV2, 1, &gstAppPosParam.cIsNeedCVV2));
	return APP_SUCC;
}

/**
* @brief Set is support tip
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionIsTipFlag(void)
{
	if (gstAppPosParam.cIsTipFlag != '0' && gstAppPosParam.cIsTipFlag != '1')
	{
		gstAppPosParam.cIsTipFlag = '1';
	}
	ASSERT_RETURNCODE(PubSelectYesOrNo(tr("OTHER CONTROL"), tr("TIP ENABLE"), NULL, &gstAppPosParam.cIsTipFlag));
	ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_TIPFLAG, 1, &gstAppPosParam.cIsTipFlag));

	if (gstAppPosParam.cIsTipFlag == NO)
	{
		SetTransSwitchOnoff(TRANS_ADJUST, NO);
		ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_TRANSSWITCH, 4, gstAppPosParam.sTransSwitch));
	}

	return APP_SUCC;
}

/**
* @brief Set tip rate
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionTipRate()
{
	int nLen;

	if( gstAppPosParam.szTipRate[0] == 0)
	{
		strcpy( gstAppPosParam.szTipRate, "15");
	}

	ASSERT_RETURNCODE(PubInputDlg(tr("SET TIP RATE"), tr("Tip rate(%):"), gstAppPosParam.szTipRate, &nLen, 1, 2, 0, INPUT_MODE_NUMBER));
	ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_TIPRATE, strlen(gstAppPosParam.szTipRate), gstAppPosParam.szTipRate));

	return APP_SUCC;
}

/**
* @brief Set is show TVR and TSI
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionIsDispEMV_TVRTSI(void)
{
	ASSERT_QUIT(PubSelectYesOrNo(tr("OTHER"), tr("SHOW TVR TSI IN TRANS"), NULL, &gstAppPosParam.cIsDispEMV_TVRTSI));
	ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_ISSHOWTVRTSI, 1, &gstAppPosParam.cIsDispEMV_TVRTSI));

	return APP_SUCC;
}

/**
* @brief Set is shield pan
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionIsShieldPan(void)
{
	if (gstAppPosParam.cIsShieldPan != YES && gstAppPosParam.cIsShieldPan != NO)
	{
		gstAppPosParam.cIsShieldPan = YES;
	}
	ASSERT_RETURNCODE(PubSelectYesOrNo(tr("SYSTEM MANAGE"), tr("Shield pan"), NULL, &gstAppPosParam.cIsShieldPan));
	ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_ISSHIELDPAN, 1, &gstAppPosParam.cIsShieldPan));

	return APP_SUCC;
}


/**
* @brief  Check the password is correct
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int ProCheckPwd(char *pszTitle, uchar cType)
{
	int nLen;
	char szAdminPasswd[16+1];
	char szThePwd[16+1] = {0};
	char szTmp[128] = {0};

	switch (cType)
	{
	case EM_NONE_PWD:
		return APP_SUCC;
		break;
	case EM_SYSTEM_PWD:
		strcpy(szThePwd, gstAppPosParam.szAdminPwd);
		strcpy(szTmp, tr("SYSTEM PASSWORD:"));
		break;
	case EM_TRANS_PWD:
		strcpy(szThePwd, gstAppPosParam.szUsrPwd);
		strcpy(szTmp, tr("INPUT PASSWORD:"));
		break;
	case EM_FUNC_PWD:
		strcpy(szThePwd, gstAppPosParam.szFuncPwd);
		strcpy(szTmp, "FUNCTION PASSWORD:");
		break;
	}

	while(1)
	{
		memset(szAdminPasswd, 0, sizeof(szAdminPasswd));
		ASSERT_QUIT(PubInputDlg(pszTitle, szTmp, szAdminPasswd, &nLen, 1, 16, 0, INPUT_MODE_PASSWD));
		if(strlen(szThePwd) != nLen
			|| memcmp(szThePwd, szAdminPasswd, nLen) != 0)
		{
			char szDispBuf[100];
			PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szDispBuf, tr("ERROR,TRY AGAIN"));
			PubMsgDlg(pszTitle, szDispBuf, 3, 1 ) ;
			continue;
		}
		break;
	}
	return APP_SUCC;
}

/**
* @brief  Modify password
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int ChangePwd(uchar cType)
{
	int nLen;
	char szAdminPasswd1[16+1];
	char szAdminPasswd2[16+1];
	char szTmpCheck[64] = {0}, szTmpOld[64] = {0}, szTmpNew[64] = {0};
	char *szTitle = tr("CHANGE PASSWORD");
	char szThePwd[16+1] = {0};

	switch (cType)
	{
	case EM_NONE_PWD:
		return APP_SUCC;
		break;
	case EM_SYSTEM_PWD:
		strcpy(szThePwd, gstAppPosParam.szAdminPwd);
		strcpy(szTmpCheck, tr("System password:"));
		break;
	case EM_TRANS_PWD:
		strcpy(szThePwd, gstAppPosParam.szUsrPwd);
		strcpy(szTmpCheck, tr("Transaction password:"));
		break;
	case EM_FUNC_PWD:
		strcpy(szThePwd, gstAppPosParam.szFuncPwd);
		strcpy(szTmpCheck, tr("Function password:"));
		break;
	}
	strcpy(szTmpOld, tr("New password:"));
	strcpy(szTmpNew, tr("Confirm new password:"));

	memset(szAdminPasswd1, 0, sizeof(szAdminPasswd1));

	ASSERT_QUIT(PubInputDlg(tr("VERIFY PASSWORD"), szTmpCheck, szAdminPasswd1, &nLen, 1, 16, 0, INPUT_MODE_PASSWD));
	if(nLen != strlen(szThePwd)
		|| memcmp(szThePwd, szAdminPasswd1, nLen) != 0 )
	{
		PubMsgDlg(szTitle, tr("Verify failed"), 0, 5) ;
		return APP_FAIL;
	}

	while(1)
	{
		/**
		* input new password
		*/
		memset(szAdminPasswd1, 0, sizeof(szAdminPasswd1));
		ASSERT_QUIT(PubInputDlg(szTitle, szTmpOld, szAdminPasswd1, &nLen, 1, 16, 0, INPUT_MODE_PASSWD));

		memset(szAdminPasswd2, 0, sizeof(szAdminPasswd2));
		ASSERT_QUIT(PubInputDlg(szTitle, szTmpNew, szAdminPasswd2, &nLen, 1, 16, 0, INPUT_MODE_PASSWD));

		if(memcmp(szAdminPasswd1, szAdminPasswd2, 16) != 0)
		{
			PubMsgDlg(szTitle, tr("Old and new are mismatch"), 3, 1) ;
			continue;
		}
		else
		{
			SetVarPwd(szAdminPasswd1, cType);
			PubMsgDlg(szTitle, tr("CHANGE OK"), 1, 1) ;
			return APP_SUCC;
		}
	}
}


/**
* @brief  Save password
* @param in szSecurity Passwd
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarPwd(const char *szSecurityPasswd, uchar cType)
{
	switch (cType)
	{
	case EM_NONE_PWD:
		return APP_SUCC;
		break;
	case EM_SYSTEM_PWD:
		memset(gstAppPosParam.szAdminPwd,0,sizeof(gstAppPosParam.szAdminPwd));
		strcpy(gstAppPosParam.szAdminPwd, szSecurityPasswd);
		ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_ADMINPWD, strlen(gstAppPosParam.szAdminPwd), gstAppPosParam.szAdminPwd));
		break;
	case EM_TRANS_PWD:
		memset(gstAppPosParam.szUsrPwd,0,sizeof(gstAppPosParam.szUsrPwd));
		strcpy(gstAppPosParam.szUsrPwd, szSecurityPasswd);
		ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_USRWD, strlen(gstAppPosParam.szUsrPwd), gstAppPosParam.szUsrPwd));
		break;
	case EM_FUNC_PWD:
		memset(gstAppPosParam.szFuncPwd,0,sizeof(gstAppPosParam.szFuncPwd));
		strcpy(gstAppPosParam.szFuncPwd, szSecurityPasswd);
		ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_FUNCPWD, strlen(gstAppPosParam.szFuncPwd), gstAppPosParam.szFuncPwd));
		break;
	}

	return APP_SUCC;
}


/**
* @brief Save record number
* @param in nRecNum
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarRecordSum(const int  nRecNum)
{
	char szNum[2+1] = {0};

	gstSettleParam.nRecNum = nRecNum;
	PubIntToC2((uchar *)szNum, gstSettleParam.nRecNum);
	ASSERT_FAIL(UpdateTagParam(FILE_APPSETTLEMENT, TAG_SETTLE_RECNUM, 2, szNum));

	return APP_SUCC;
}

/**
* @brief Get record num
* @param in pnRecordSum
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int GetVarRecordSum(int *pnRecordSum)
{
	*pnRecordSum = gstSettleParam.nRecNum;
	return APP_SUCC;
}

/**
* @brief Save count had sent
* @param in nHaveReSendNum
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarHaveReSendNum(const int  nHaveReSendNum)
{
	gstSettleParam.nHaveReSendNum = nHaveReSendNum;
	ASSERT_FAIL(UpdateTagParam(FILE_APPSETTLEMENT, TAG_SETTLE_HADRESENDNUM, 1, (char *)&gstSettleParam.nHaveReSendNum));

	return APP_SUCC;
}

/**
* @brief Get count had sent
* @param in pnHaveReSendNum
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int GetVarHaveReSendNum(int *pnHaveReSendNum)
{
	*pnHaveReSendNum = gstSettleParam.nHaveReSendNum;
	return APP_SUCC;
}

/**
* @brief Increase count had sent
* @param in void
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int IncVarOfflineUnSendNum()
{
	char szBuf[2+1] = {0};

	gstSettleParam.nOffLineUnsendNum++;
	PubIntToC2((uchar *)szBuf, gstSettleParam.nOffLineUnsendNum);
	ASSERT_FAIL(UpdateTagParam(FILE_APPSETTLEMENT, TAG_SETTLE_OFFLINEUNSENDNUM, 2, szBuf));

	return APP_SUCC;
}

/**
* @brief Decrease the offline count whcich have not sent
* @param in void
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int DelVarOfflineUnSendNum()
{
	char szBuf[2+1] = {0};

	gstSettleParam.nOffLineUnsendNum--;
	if(gstSettleParam.nOffLineUnsendNum < 0)
	{
		gstSettleParam.nOffLineUnsendNum = 0;
	}
	PubIntToC2((uchar *)szBuf, gstSettleParam.nOffLineUnsendNum);
	ASSERT_FAIL(UpdateTagParam(FILE_APPSETTLEMENT, TAG_SETTLE_OFFLINEUNSENDNUM, 2, szBuf));

	return APP_SUCC;
}


/**
* @brief Get offline count have not sent
* @param in void
* @return
* @li
*/
int GetVarOfflineUnSendNum()
{
	return gstSettleParam.nOffLineUnsendNum;
}


/**
* @brief Save offline count have not sent
* @param in void
* @return
* @li APP_SUCC
*/
int SetVarOfflineUnSendNum(int nNum)
{
	char szBuf[2+1] = {0};

	PubIntToC2((uchar *)szBuf, gstSettleParam.nOffLineUnsendNum);
	ASSERT_FAIL(UpdateTagParam(FILE_APPSETTLEMENT, TAG_SETTLE_OFFLINEUNSENDNUM, 2, szBuf));

	return APP_SUCC;
}

/**
* @brief
* @param in
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarPrintSettleHalt(const YESORNO cPrintSettleHalt)
{
	gstSettleParam.cPrintSettleHalt = cPrintSettleHalt;
	ASSERT_FAIL(UpdateTagParam(FILE_APPSETTLEMENT, TAG_SETTLE_PNTSETTLEHALT, 1, &gstSettleParam.cPrintSettleHalt));

	return APP_SUCC;
}

/**
* @brief
* @return
* @li YES
* @li NO
*/
YESORNO GetVarPrintSettleHalt()
{
	if (gstSettleParam.cPrintSettleHalt == YES)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}

/**
* @brief
* @param in
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarPrintDetialHalt(const YESORNO cPrintDetialHalt)
{
	gstSettleParam.cPrintDetialHalt = cPrintDetialHalt;

	ASSERT_FAIL(UpdateTagParam(FILE_APPSETTLEMENT, TAG_SETTLE_PNTDETAILHALT, 1, &gstSettleParam.cPrintDetialHalt));
	return APP_SUCC;
}

/**
* @brief
* @return
* @li YES
* @li NO
*/
YESORNO GetVarPrintDetialHalt(void)
{
	if (gstSettleParam.cPrintDetialHalt == YES)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}

/**
* @brief
* @param in
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarPrintRecordHaltFlag(const YESORNO cPrintRecordHaltFlag)
{
	gstSettleParam.cPrintRecordHaltFlag = cPrintRecordHaltFlag;
	ASSERT_FAIL(UpdateTagParam(FILE_APPSETTLEMENT, TAG_SETTLE_PNTRECORDHALT, 1, &gstSettleParam.cPrintRecordHaltFlag));
	return APP_SUCC;
}

/**
* @brief
* @return
* @li YES
* @li NO
*/
YESORNO GetVarPrintRecordHaltFlag(void)
{
	if (gstSettleParam.cPrintRecordHaltFlag == YES)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}

/**
* @brief
* @param in
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarClrSettleDataFlag(const YESORNO cClrSettleDataFlag)
{
	gstSettleParam.cClrSettleDataFlag = cClrSettleDataFlag;
	ASSERT_FAIL(UpdateTagParam(FILE_APPSETTLEMENT, TAG_SETTLE_CLRSETTLE, 1, &gstSettleParam.cClrSettleDataFlag));

	return APP_SUCC;
}

/**
* @brief
* @return
* @li YES
* @li NO
*/
YESORNO GetVarClrSettleDataFlag(void)
{
	if (gstSettleParam.cClrSettleDataFlag == YES)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}

/**
* @brief Save is summary match flag
* @param in cIsTotalMatch
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarIsTotalMatch(const char cIsTotalMatch)
{
	gstSettleParam.cIsTotalMatch = cIsTotalMatch;

	ASSERT_FAIL(UpdateTagParam(FILE_APPSETTLEMENT, TAG_SETTLE_TOTALMATCHFLAG, 1, &gstSettleParam.cIsTotalMatch));
	return APP_SUCC;
}

/**
* @brief Get is summary match flag
* @return
* @li YES
* @li NO
*/
char GetVarIsTotalMatch(void)
{
	return gstSettleParam.cIsTotalMatch;
}

/**
* @brief
* @param in BatchHaltFlag
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarBatchHaltFlag(const YESORNO cBatchHaltFlag)
{
	gstSettleParam.cBatchHaltFlag = cBatchHaltFlag;

	ASSERT_FAIL(UpdateTagParam(FILE_APPSETTLEMENT, TAG_SETTLE_BATCHHALT, 1, &gstSettleParam.cBatchHaltFlag));
	return APP_SUCC;
}

/**
* @brief
* @return
* @li YES
* @li NO
*/
YESORNO GetVarBatchHaltFlag(void)
{
	if (gstSettleParam.cBatchHaltFlag == YES)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}

/**
* @brief
* @param in nMessageHaltFlag
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarBatchMagOfflineHaltFlag(const int  nBatchMagOfflineHaltFlag)
{
	gstSettleParam.nBatchMagOfflinHaltFlag = nBatchMagOfflineHaltFlag;
	ASSERT_FAIL(UpdateTagParam(FILE_APPSETTLEMENT, TAG_SETTLE_BATCHOFFLINEHALT, 1, (char *)&gstSettleParam.nBatchMagOfflinHaltFlag));

	return APP_SUCC;
}

/**
* @brief
* @param in pnMessageHaltFlag
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int GetVarBatchMagOfflineHaltFlag(int *pnBatchMagOfflineHaltFlag)
{
	*pnBatchMagOfflineHaltFlag = gstSettleParam.nBatchMagOfflinHaltFlag;

	return APP_SUCC;
}

/**
* @brief Save batch sum
* @param in nBatchSum
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarBatchSum(const int  nBatchSum)
{
	char szBuf[2+1] = {0};

	gstSettleParam.nBatchSum = nBatchSum;
	PubIntToC2((uchar *)szBuf, gstSettleParam.nBatchSum);
	ASSERT_FAIL(UpdateTagParam(FILE_APPSETTLEMENT, TAG_SETTLE_BATCH_SUM, 2, szBuf));

	return APP_SUCC;
}

/**
* @brief Get batch sum
* @param in pnBatchSum
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int GetVarBatchSum(int *pnBatchSum)
{
	*pnBatchSum = gstSettleParam.nBatchSum;
	return APP_SUCC;
}

/**
* @brief Get settle date
* @param in psSettleDateTime
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int GetVarSettleDateTime(char *psSettleDateTime)
{
	memcpy(psSettleDateTime, gstSettleParam.sSettleDateTime, 5);
	return APP_SUCC;
}

/**
* @brief Save settle time
* @param in nBatchSum
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarSettleDateTime(const char *psSettleDateTime)
{
	memcpy( gstSettleParam.sSettleDateTime, psSettleDateTime, 5);

	ASSERT_FAIL(UpdateTagParam(FILE_APPSETTLEMENT, TAG_SETTLE_DATETIME, 5, gstSettleParam.sSettleDateTime));
	return APP_SUCC;
}

/**
* @brief Save settle data item
* @param in psSettDataItem
* @param in nOffset
* @param in nLen
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SaveSettleDataItem(char cTransType, SETTLE_NUM *pnTransNum, SETTLE_AMT *pstTransAmt)
{
	int nTagidNum;
	int nTagidAmt;
	char szNum[2+1] = {0};

	switch (cTransType)
	{
	case TRANS_SALE:
		nTagidNum = TAG_SETTLE_SALENUM;
		nTagidAmt = TAG_SETTLE_SALEAMT;
		break;
	case TRANS_VOID:
		nTagidNum = TAG_SETTLE_VOIDSALENUM;
		nTagidAmt = TAG_SETTLE_VOIDSALEAMT;
		break;
	case TRANS_AUTHCOMP:
		nTagidNum = TAG_SETTLE_AUTHSALENUM;
		nTagidAmt = TAG_SETTLE_AUTHSALEAMT;
		break;
	case TRANS_VOID_AUTHSALE:
		nTagidNum = TAG_SETTLE_VOIDAUTHSALENUM;
		nTagidAmt = TAG_SETTLE_VOIDAUTHSALEAMT;
		break;
	case TRANS_PREAUTH:
		nTagidNum = TAG_SETTLE_PREAUTHNUM;
		nTagidAmt = TAG_SETTLE_PREAUTHAMT;
		break;
	case TRANS_VOID_PREAUTH:
		nTagidNum = TAG_SETTLE_VOIDPREAUTHNUM;
		nTagidAmt = TAG_SETTLE_VOIDPREAUTHAMT;
		break;
	case TRANS_REFUND:
		nTagidNum = TAG_SETTLE_REFUNDNUM;
		nTagidAmt = TAG_SETTLE_REFUNDAMT;
		break;
	case TRANS_ADJUST:
		nTagidNum = TAG_SETTLE_ADJUSTNUM;
		nTagidAmt = TAG_SETTLE_ADJUSTAMT;
		break;

	case TRANS_OFFLINE:
		nTagidNum = TAG_SETTLE_OFFLINENUM;
		nTagidAmt = TAG_SETTLE_OFFLINEAMT;
		break;
	default:
		return APP_FAIL;
		break;
	}

	PubIntToC2((uchar *)szNum, *pnTransNum);
	ASSERT_FAIL(UpdateTagParam(FILE_APPSETTLEMENT, nTagidNum, 2, szNum));
	ASSERT_FAIL(UpdateTagParam(FILE_APPSETTLEMENT, nTagidAmt, sizeof(SETTLE_AMT), (char *)*pstTransAmt));

	return APP_SUCC;
}

/**
* @brief Get APP POS parameter
* @param out pstAppPosParam
* @return void
*/
void GetAppPosParam(STAPPPOSPARAM *pstAppPosParam )
{
	memcpy((char *)pstAppPosParam, (char *)(&gstAppPosParam), sizeof(STAPPPOSPARAM) );
}

/**
* @brief Set APP POS parameter
* @param out pstAppPosParam
* @return void
*/
void SetAppPosParam(STAPPPOSPARAM stAppPosParam)
{
	memcpy((char *)&gstAppPosParam, (char *)(&stAppPosParam), sizeof(STAPPPOSPARAM) );
}

/**
* @brief Get is use extern RF
* @return
* @li YES
* @li NO
*/
YESORNO GetVarIsExRF(void)
{
	if (APP_SUCC == PubGetHardwareSuppot(HARDWARE_SUPPORT_PINPAD, NULL))
	{
		if (gstAppPosParam.cIsExRF == YES)
		{
			return YES;
		}
		else
		{
			return NO;
		}
	}
	else
	{
		return NO;
	}
}

/**
* @brief Set is use extern RF
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionIsExRF(void)
{
	char cOldIsExRF = gstAppPosParam.cIsExRF;
	char *SelMenu[] = {"0.INSIDE", "1.EXTERN"};

	if (APP_SUCC == PubGetHardwareSuppot(HARDWARE_SUPPORT_PINPAD, NULL))	/* Pinpad includes Ex RF */
	{
		if(gstAppPosParam.cIsSupportRF == NO)
		{
			return APP_FUNCQUIT;
		}

		ASSERT_RETURNCODE(PubSelectYesOrNo(tr("EXTERN RF"), tr("EXTERN RF"), SelMenu, &gstAppPosParam.cIsExRF));
		ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_ISEXRF, 1, &gstAppPosParam.cIsExRF));
		if (cOldIsExRF != gstAppPosParam.cIsExRF)
		{
			SetControlChkRF(YES);
		}
	}
	else
	{
		gstAppPosParam.cIsExRF = NO;
		PubMsgDlg(tr("Warning"),tr("BUILD-IN ONLY"), 1, 3);
	}
	ChkRF();

	return APP_SUCC;
}

/**
* @brief Get is support RF
* @return
* @li YES
* @li NO
*/
YESORNO GetVarIsSupportRF(void)
{
	if (gstAppPosParam.cIsSupportRF == YES)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}

/**
* @brief transation is support RF
* @return
* @li YES
* @li NO
*/
YESORNO Trans_Support_RF(char cTransType)
{
	switch(cTransType) {
	case TRANS_SALE:
	case TRANS_PREAUTH:
	case TRANS_BALANCE:
		return YES;
		break;
	default:
		break;
	}

	return NO;
}

/**
* @brief Set is support RF
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionIsSupportRF(void)
{
	char cOldIsSupportRF = gstAppPosParam.cIsSupportRF;

	ASSERT_RETURNCODE(PubSelectYesOrNo(tr("OTHER"),tr("SUPPORT RF"), NULL, &gstAppPosParam.cIsSupportRF));
	ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_ISSUPPORTRF, 1, &gstAppPosParam.cIsSupportRF));
	if ((NO == cOldIsSupportRF) && (YES == gstAppPosParam.cIsSupportRF))
	{
		SetControlChkRF(YES);
		ChkRF();
	}
	else if(NO == gstAppPosParam.cIsSupportRF)
	{
		SetControlChkRF(NO);
	}

	return APP_SUCC;
}

void GetVarSoftVer(char* pszSoftVer)
{
	memcpy(pszSoftVer, APP_VERSION, 8);
}

int SetFunctionPinpadTimeOut(void)
{
	int nLen;
	char szOldTimeOut[3+1] = {0};

	memcpy(szOldTimeOut, gstAppPosParam.szPinPadTimeOut, 3);

	if (YES == GetVarIsPinpad())
	{
		ASSERT_RETURNCODE(PubInputDlg(tr("Pinpad param"),tr("TIMEOUT:"), gstAppPosParam.szPinPadTimeOut, &nLen, 1, 3, 0, INPUT_MODE_NUMBER));
		ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_PINPADTIMEOUT, strlen(gstAppPosParam.szPinPadTimeOut), gstAppPosParam.szPinPadTimeOut));
		if (memcmp(szOldTimeOut, gstAppPosParam.szPinPadTimeOut, 3) != 0)
		{
			SetControlChkPinpad(YES);
		}
		return APP_SUCC;
	}

	return APP_SUCC;
}
/**
* @brief Set head print mode of receipt
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionPrintTitleMode(void)
{
	char *selMenu[] = {tr("0.LOGO"), tr("1.STRING")};

	ASSERT_RETURNCODE(PubSelectYesOrNo(tr("SET PRINT"),tr("RECEIPT HEAD"), selMenu, &gstAppPosParam.cPntTitleMode));
	ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_PNTTITLEMODE, 1, &gstAppPosParam.cPntTitleMode));
	if (YES == gstAppPosParam.cPntTitleMode)
	{
		return SetFunctionPntTitle();
	}

	return APP_SUCC;
}
/**
* @brief Get Title print mode
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
YESORNO GetIsPntTitleMode(void)
{
	if (gstAppPosParam.cPntTitleMode == YES || gstAppPosParam.cPntTitleMode == 1)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}

/**
* @brief Set receipt title
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionPntTitle(void)
{
	int nLen = 0;
	char szTmp[128] = {0};

	strcpy(szTmp, gstAppPosParam.szPntTitleEn);
	ASSERT_RETURNCODE(PubInputDlg(tr("PRINT TITLE"), NULL, szTmp, &nLen, 0, 40, 0, INPUT_MODE_STRING));
	strcpy(gstAppPosParam.szPntTitleEn, szTmp);
	ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_PNTTITLEEN, strlen(gstAppPosParam.szPntTitleEn), gstAppPosParam.szPntTitleEn));

	return APP_SUCC;
}
/**
* @brief  Get receipt title
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int GetFunctionPntTitle(char *pPntTitleCn)
{
	strcpy(pPntTitleCn, gstAppPosParam.szPntTitleEn);
	return APP_SUCC;
}

/**
* @brief Get is need to check pin when do void
* @return
* @li YES
* @li NO void
*/
YESORNO GetVarIsAdminPin()
{
	if (gstAppPosParam.cIsAdminPwd == YES)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}

/**
* @brief Set is need to check pin when do void
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionIsTransPinInput(void)
{
	ASSERT_RETURNCODE(PubSelectYesOrNo(tr("OTHER CONTROL"),tr("VERIFY TRANS PWD"),  NULL, &gstAppPosParam.cIsAdminPwd));
	ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_ISADMINPWD, 1, &gstAppPosParam.cIsAdminPwd));

	return APP_SUCC;
}


YESORNO GetVarIsPrintPrintMinus(void)
{
	return gstAppPosParam.cIsPrintMinus;
}

int SetFunctionIsPrintPrintMinus(void)
{
	ASSERT_RETURNCODE(PubSelectYesOrNo(tr("SET PRINT"),tr("PRINT MINUS"),  NULL, &gstAppPosParam.cIsPrintMinus));
	ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_PNTMINUS, 1, &gstAppPosParam.cIsPrintMinus));

	return APP_SUCC;
}

int SetFunctionReprintSettle()
{
	ASSERT_RETURNCODE(PubSelectYesOrNo(tr("REPRINT SETTLE"),tr("ENABLE REPRINT"), NULL,  &gstAppPosParam.cIsReprintSettle));
	ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_ISREPNTSETTLE, 1, &gstAppPosParam.cIsReprintSettle));

	return APP_SUCC;
}

YESORNO GetVarReprintSettle(void)
{
	if (YES == gstAppPosParam.cIsReprintSettle)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}

void GetAcqName(char *pszAcqName)
{
	memcpy(pszAcqName, gstAppPosParam.szAcqName, 20);
}

YESORNO GetVarShowRFLogo()
{
	if (YES == GetVarIsExRF())
	{
		return NO;
	}
	return YES;
}

/**
* @brief Show terminal version
* @param void
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int Version(void)
{
	char szSoftVer[16]= {0};

	szSoftVer[0] = 'V';
	GetVarSoftVer(&szSoftVer[1]);

	PubClearAll();
	PubDisplayTitle(tr("Software INFO"));
	PubDisplayStrInline(DISPLAY_MODE_CLEARLINE, 2, tr("Version Num:"));
	PubDisplayStrInline(DISPLAY_MODE_CLEARLINE, 3, "%s", szSoftVer);
	PubUpdateWindow();
	TxnWaitAnyKey(0);
	return APP_SUCC;
}


/**
* @brief Get emv error code msg from ini
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int GetEmvErrorCodeIni(int nEmvErrCode, char *pszErr)
{
	const char *szIniFile = FILE_EMVCODE_INI;
	int nIniHandle,nRet;
	int nLen;
	char szBuf[100];
	char szErrCode[12] = {0};

	if (NULL == pszErr || nEmvErrCode >= 0)
	{
		return APP_QUIT;
	}

	sprintf(szErrCode, "%d", nEmvErrCode);


	ASSERT_QUIT(PubOpenFile (szIniFile, "r", &nIniHandle));

	nRet = PubGetINIItemStr (nIniHandle, tr("BASE"), szErrCode+1,  sizeof(szBuf) - 1, szBuf, &nLen);
	if (nRet == APP_SUCC)
	{
		sprintf(pszErr, "%s%s", szBuf, szErrCode);
	}
	else
	{
		sprintf(pszErr, "Unknown error%s", szErrCode);
	}
	PubCloseFile(&nIniHandle);
	return nRet;
}


static int CheckIsValidTime(const char *pszTime)
{
	char szTmp[2+1];

	memset(szTmp, 0, sizeof(szTmp));
	memcpy(szTmp, pszTime, 2);
	if(atoi(szTmp) >= 24)
	{
		return APP_FAIL;
	}

	memset(szTmp, 0, sizeof(szTmp));
	memcpy(szTmp, pszTime+2, 2);
	if(atoi(szTmp) >= 60)
	{
		return APP_FAIL;
	}
	memset(szTmp, 0, sizeof(szTmp));
	memcpy(szTmp, pszTime+4, 2);
	if(atoi(szTmp) >= 60)
	{
		return APP_FAIL;
	}
	return APP_SUCC;
}


/**
* @brief Set date time
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionPosDateTime(void)
{
	int  nLen = 0;
	char szDateTime[14+1]={0};
	char szDate[8+1]={0};
	char szTime[6+1]={0};


	PubGetCurrentDatetime(szDateTime);
	memcpy(szDate, szDateTime, 8);
	memcpy(szTime, szDateTime+8, 6);
	ASSERT_RETURNCODE(PubInputDate(tr("SYSTEM TIME"),tr("DATE(YYYYMMDD):"), szDate, INPUT_DATE_MODE_YYYYMMDD, 60));

	if(PubIsValidDate(szDate) != APP_SUCC)
	{
		PubMsgDlg(tr("SYSTEM TIME"),tr("INVALID INPUT"), 3, 5);//"Input date error"
		return APP_FAIL;
	}

	PubSetPosDateTime(szDate, "YYYYMMDD", szTime);

	ASSERT_RETURNCODE(PubInputDlg(tr("SYSTEM TIME"),tr("TIME(HHMMSS):"), szTime, &nLen, 6, 6, 60, INPUT_MODE_NUMBER));

	if(CheckIsValidTime(szTime) != APP_SUCC)
	{
		PubMsgDlg(tr("SYSTEM TIME"),tr("INVALID INPUT"), 3, 5);
		return APP_FAIL;
	}

	PubSetPosDateTime(szDate, "YYYYMMDD", szTime);
	return APP_SUCC;

}

/**
* @brief Set the switch of transaction
* @param in cTransType Trade type
* @return
* @li YES
* @li NO
* @author
* @date
*/
int SetTransSwitchOnoff(int nTransType, char cIsEnable)
{
	int nTransNum;

	nTransNum = nTransType;
	if((nTransType >= 0) && (nTransType < sizeof(gstAppPosParam.sTransSwitch)*8))
	{
		if (cIsEnable == YES)
		{
			gstAppPosParam.sTransSwitch[nTransNum/8] |= (0x80>>(nTransNum%8));
		}
		else
		{
			gstAppPosParam.sTransSwitch[nTransNum/8] &= ~(0x80>>(nTransNum%8));
		}
	}
	else
	{
		TRACE("cTransType=%d cIsEnable=%d",nTransType,cIsEnable);
		return APP_FAIL;
	}

	return APP_SUCC;
}

void GetTransSwitchValue(char *pszTransSwitch)
{
	memcpy(pszTransSwitch, gstAppPosParam.sTransSwitch, 4);
}

/**
* @brief Get merchant address
* @param out pszValue
* @return void
*/
void GetVarMerchantAddr(char *pszValue, int nIndex)
{
	if (nIndex >= (sizeof(gstAppPosParam.szMerchantAddr) / sizeof(gstAppPosParam.szMerchantAddr[0])))
	{
		return;
	}
	memcpy(pszValue, gstAppPosParam.szMerchantAddr[nIndex], sizeof(gstAppPosParam.szMerchantAddr[nIndex]) - 1);
}

/**
* @brief Get Issuer bank Name
* @param out pszValue
* @return void
*/
void GetVarIssueBank(char *pszValue)
{
	memcpy(pszValue, gstAppPosParam.szIssuerName, sizeof(gstAppPosParam.szIssuerName) - 1);
}

/**
* @brief Get currency type
* @param out pszValue
* @return void
*/
void GetVarCurrencyName(char *pszValue)
{
	memcpy(pszValue, gstAppPosParam.szCurrencyName, sizeof(gstAppPosParam.szCurrencyName) - 1);
}


/**
* @brief Set acquirer bank name
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionAcqName(void)
{
	int nLen = 0;
	char szTmp[128] = {0};

	strcpy(szTmp, gstAppPosParam.szAcqName);
	ASSERT_RETURNCODE(PubInputDlg(tr("ACQUIRER NAME"), NULL, szTmp, &nLen, 0, 20, 0, INPUT_MODE_STRING));
	strcpy(gstAppPosParam.szAcqName, szTmp);
	ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_ACQNAME, strlen(gstAppPosParam.szAcqName), gstAppPosParam.szAcqName));

	return APP_SUCC;

}

/**
* @brief Set issue name
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionIssuerName(void)
{
	int nLen = 0;
	char szTmp[128] = {0};

	strcpy(szTmp, gstAppPosParam.szIssuerName);
	ASSERT_RETURNCODE(PubInputDlg(tr("ISSUER NAME"), NULL, szTmp, &nLen, 0, 11, 0, INPUT_MODE_STRING));
	strcpy(gstAppPosParam.szIssuerName, szTmp);
	ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_ISSUERNAME, strlen(gstAppPosParam.szIssuerName), gstAppPosParam.szIssuerName));

	return APP_SUCC;

}
/**
* @brief Get switch of transaction, if support ,return YES
* @param in cTransType
* @return
* @li YES
* @li NO
*/
YESORNO GetTransSwitchOnoff(char cTransType)
{
	int nTransNum;

	nTransNum = cTransType;
	if (cTransType >= TRANS_MAX)
	{
		return NO;
	}

	if (gstAppPosParam.sTransSwitch[nTransNum/8] & (0x80 >> (nTransNum % 8)))
	{
		return YES;
	}
	else
	{
		return NO;
	}
}

/**
* @brief Set switch of transaction
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionTransSwitch(void)
{
	int i, nRet;
	char cSelect;
	STTRANSACTION stTransAction[] = {
	 	{TRANS_SALE, tr("SALE")},
	 	{TRANS_VOID, tr("VOID")},
	 	{TRANS_REFUND, tr("REFUND")},
	 	{TRANS_PREAUTH, tr("PREAUTH")},
	 	{TRANS_AUTHCOMP, tr("AUTHCOMP")},
	 	{TRANS_ADJUST, tr("ADJUST")},
	 	{TRANS_BALANCE, tr("BALANCE")},
	 	{TRANS_VOID_AUTHSALE, tr("VOID AUTHSALE")},
	 	{TRANS_VOID_PREAUTH, tr("VOID PREAUTH")},
	 	{TRANS_CASHBACK, tr("CASHBACK")},
	 };

	for(i = 0; i < sizeof(stTransAction) / sizeof(STTRANSACTION); i++)
	{
		cSelect =  (GetTransSwitchOnoff(stTransAction[i].cTransType) == YES) ? 1 : 0;
		nRet = PubSelectYesOrNo(tr("IS SUPPORT?"), stTransAction[i].pszTransName , NULL, &cSelect);
		if (nRet == KEY_UP)
		{
			i = i - 2;
			if (i  < -1)
			{
				i = -1;
			}
			continue;
		}
		else if(nRet == KEY_DOWN)
		{
			continue;
		}
		else if (nRet == APP_FAIL || nRet == APP_QUIT)
		{
			break;
		}
		if (cSelect == '1')
		{
			SetTransSwitchOnoff(stTransAction[i].cTransType, YES);
		}
		else
		{
			SetTransSwitchOnoff(stTransAction[i].cTransType, NO);
		}
	}

	ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_TRANSSWITCH, 4, gstAppPosParam.sTransSwitch));
	if (GetTransSwitchOnoff(TRANS_ADJUST) == YES) // The adjustment is the tip adjustment
	{
		gstAppPosParam.cIsTipFlag = YES;
		ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_TIPFLAG, 1, &gstAppPosParam.cIsTipFlag));
	}

	return APP_SUCC;
}

int ChangePassword()
{
	char *pszItems[] = {
		tr("1.SYSTEM PWD"),
		tr("2.USER PWD"),
		tr("3.FUNCTION PWD")
	};
	int nSelcItem = 1, nStartItem = 1;

	ASSERT_QUIT(PubShowMenuItems(tr("CHANGE PASSWORD"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 0));

	switch(nSelcItem)
	{
	case 1:
		ChangePwd(EM_SYSTEM_PWD);
		break;
	case 2:
		ChangePwd(EM_TRANS_PWD);
		break;
	case 3:
		ChangePwd(EM_FUNC_PWD);
		break;
	default:
		break;
	}
	return APP_SUCC;
}

int ViewTranList()
{
	RecordOneByOne();
	return APP_SUCC;
}

int ViewSpecList()
{
	FindByInvoice(0);
	return APP_SUCC;
}

int ViewTotal()
{
	DispTotal();
	return APP_SUCC;
}

int ViewTVR_TSI()
{
	SetFunctionIsDispEMV_TVRTSI();
	return APP_SUCC;
}

int RePrnSettle()
{

	return APP_SUCC;
}

int PrnLastTrans()
{
	int nRecordNum = -1;
	STTRANSRECORD stTransRecord;

	GetRecordNum(&nRecordNum);
	if (nRecordNum <=0 )
	{
		char szDispBuf[100];
		PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szDispBuf, "|CNO RECORD");
		PubMsgDlg(tr("PRINT LAST"), szDispBuf, 0, 50);
		return APP_FAIL;
	}
	FetchLastRecord(&stTransRecord);
	PubClearAll();
	DISP_PRINTING_NOW;
	PrintRecord(&stTransRecord, REPRINT);
	return APP_SUCC;
}

int RePrnSpecTrans()
{
	int nRecordNum = -1;

	GetRecordNum(&nRecordNum);
	if (nRecordNum <=0 )
	{
		char szDispBuf[100];
		PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szDispBuf, "|CNO RECORD");
		PubMsgDlg(tr("PRINT SPECIAL"), szDispBuf, 0, 50);
		return APP_FAIL;
	}
	FindByInvoice(1);
	return APP_SUCC;
}

int PrnTotal()
{
	int nRet = -1;

	nRet = PubConfirmDlg(tr("PRINT TOTAL"), tr("CONFIRM?"), 0, 50);
	if (nRet != APP_SUCC)
	{
		return nRet;
	}
	PubClearAll();
	DISP_PRINTING_NOW;
	PrintTotal();

	return APP_SUCC;
}

int SendOffSale()
{
	TxnSendOffline(0);
	return APP_SUCC;
}

int SetVarLastLoginDateTime(const char *psLastLoginDateTime)
{
	memcpy(gstTransParam.sLastLoginDateTime, psLastLoginDateTime, 5);
	ASSERT_FAIL(UpdateTagParam(FILE_APPTRANSPARAM, TAG_TRANS_LOGINDATETIME, 5, gstTransParam.sLastLoginDateTime));

	return APP_SUCC;
}

int GetVarLastLoginDateTime(char *psLastLoginDateTime)
{
	memcpy(psLastLoginDateTime, gstTransParam.sLastLoginDateTime, 5);
	return APP_SUCC;
}

//return YES or NO
YESORNO GetVarIsSwipe()
{
	if (gstAppPosParam.cIsSupportSwipe == YES)
	{
		return YES;
	}

	return NO;
}

/**
* @brief set to support swipe
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionIsSwipe(void)
{
	ASSERT_RETURNCODE(PubSelectYesOrNo(tr("PAYMENT TYPE"),tr("SWIPE ENABLE"),  NULL, &gstAppPosParam.cIsSupportSwipe));
	ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_SUPPOTRSWIPE, 1, &gstAppPosParam.cIsSupportSwipe));

	return APP_SUCC;
}
/**
* @brief  set invoice
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionInvoiceNo(void)
{
	int nLen;
	char szInvoiceNo[6+1] = {0};
	int nRecNum = 0;

	memcpy(szInvoiceNo, gstTransParam.szInvoiceNo, sizeof(gstTransParam.szInvoiceNo) - 1);
	ASSERT_RETURNCODE(PubInputDlg(tr("Trans Param"),tr("Invoice NO."), szInvoiceNo, &nLen, 6, 6, 0, INPUT_MODE_NUMBER));
	if (memcmp(gstTransParam.szInvoiceNo, szInvoiceNo, 6) !=0)
	{
		GetRecordNum(&nRecNum);
		if (nRecNum > 0)
		{
			PubMsgDlg(tr("SET MAYBANK"),tr("Please settle"), 3, 10);
			return APP_FAIL;
		}
		memcpy(gstTransParam.szInvoiceNo, szInvoiceNo, 6);
		ASSERT_FAIL(UpdateTagParam(FILE_APPTRANSPARAM, TAG_TRANS_INVOICENO, 6, gstTransParam.szInvoiceNo));
	}

	return APP_SUCC;
}

/**
* @brief Get iso print flag
* @param void
* @return
* @li YES
* @li NO
* @author
* @date
*/
int GetVarIsPrintIso(void)
{
	return gstAppPosParam.cIsPrintIso;
}

/**
* @brief Set iso print flag
* @param void
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
* @author
* @date
*/
int SetFunctionIsPrintIso(void)
{
	ASSERT_QUIT(PubSelectYesOrNo(tr("SET PRINTF"),tr("IS PRINT ISO"),  NULL, &gstAppPosParam.cIsPrintIso));
	ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_ISPNTISO, 1, &gstAppPosParam.cIsPrintIso));

	return APP_SUCC;
}

/**
* @brief Set key system type
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionKeySystemType(void)
{
	char* SelMenu[] = {
		"0.MK/SK",
		"1.DUKPT"
		};

	ASSERT_RETURNCODE(PubSelectYesOrNo(tr("KEY SYSTEM TYPE"),tr("KEY SYSTEM TYPE"), SelMenu, &gstAppPosParam.cKeySystemType));
	ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_KEYSYSTEMTYPE, 1, &gstAppPosParam.cKeySystemType));
	if (gstAppPosParam.cKeySystemType == KS_DUKPT)
	{
		SetVarIsLogin(YES);
	}

	PubSetKeySystemType(gstAppPosParam.cKeySystemType == KS_MSK ? SECRITY_KEYSYSTEM_MSKEY : SECRITY_KEYSYSTEM_DUKPT);
	return APP_SUCC;
}

/**
* @brief Get key system type
* @param void
* @return
* @li '0' -- MK/SK
* @li '1' -- DUKPT
* @author
* @date
*/
int GetVarKeySystemType(void)
{
	return gstAppPosParam.cKeySystemType;
}

/**
* @brief Set system language
* @param int
* @return
* @li cLanguage EM_LANG
* @author
* @date
*/
int SetVarSystemLanguage(const EM_LANG cLanguage)
{
	ASSERT_FAIL(SetCustomizedFont(cLanguage));
	gstAppPosParam.cLanguage = cLanguage;
	ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_LANGUAGE, 1, &gstAppPosParam.cLanguage));

	return APP_SUCC;
}

/**
* @brief Get system language
* @return
* @li EM_LANG
* @author
*/
EM_LANG GetVarSystemLanguage(void)
{
	return gstAppPosParam.cLanguage;
}

/**
* @brief Set system font size
* @param int
* @return
* @li cFontSize font size
* @author
* @date
*/
int SetVarSystemFontSize(const char cFontSize)
{
	PubSetDispForm(cFontSize, cFontSize / 4);
	gstAppPosParam.cFontSize = cFontSize;
	ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_FONTSIZE, 1, &gstAppPosParam.cFontSize));
	return APP_SUCC;
}

/**
* @brief Set system font size
* @return
* @li font size
* @author
*/
int GetVarSystemFontSize(void)
{
	return gstAppPosParam.cFontSize;
}


/**
* @brief Get Transaction name according to type
* @param in char cTransType
* @param out char *pszTtitle
* @return void
*/
void GetTransName(char cTransType, char *pszTransName)
{
	switch( cTransType ) {
	case TRANS_SALE:
		strcpy(pszTransName, tr("SALE" )) ;
		break;
	case TRANS_CASHBACK:
		strcpy(pszTransName, tr("CASH BACK" )) ;
		break;
	case TRANS_VOID:
		strcpy(pszTransName, tr("VOID" ));
		break;
	case TRANS_ADJUST:
		strcpy(pszTransName, tr("ADJUST"));
		break;
	case TRANS_BALANCE:
		strcpy(pszTransName, tr("BALANCE" ));
		break;
	case TRANS_PREAUTH:
		strcpy(pszTransName, tr("PREAUTH" )) ;
		break;
	case TRANS_VOID_PREAUTH:
		strcpy(pszTransName, tr("PREAUTH VOID" ));
		break;
	case TRANS_AUTHCOMP:
		strcpy(pszTransName, tr("AUTH_COMP"));
		break;
	case TRANS_AUTHSALEOFF:
		strcpy(pszTransName, tr("AUTH SETTLEMENT"));
		break;
	case TRANS_VOID_AUTHSALE:
		strcpy(pszTransName, tr("AUTH_COMP VOID"));
		break;
	case TRANS_REFUND:
		strcpy(pszTransName, tr("REFUND"));
		break;
	case TRANS_OFFLINE:
		strcpy(pszTransName, tr("OFFLINE"));
		break;
	case TRANS_LOGIN:
		strcpy(pszTransName, tr("SIGN IN"));
		break;
	case TRANS_UPLOAD:
		strcpy(pszTransName, tr("UPLOAD"));
		break;
	default:
		break;
	}
}

char GetPinpadCallBackFlag()
{
	if (gstAppPosParam.cPinPadCallbackFlag == YES || gstAppPosParam.cPinPadCallbackFlag == NO || gstAppPosParam.cPinPadCallbackFlag == 0)
	{
		return 0xFF;
	}
	return gstAppPosParam.cPinPadCallbackFlag;
}

/**
* @brief Set ME51P Mode
* @param void
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
* @author
* @date
*/
int SetFunctionPinpadUsage(void)
{
	char *pszItems[] = {
		tr("1.Only PIN"),
		tr("2.Card And PIN"),
	};
	int nSelcItem = 1, nStartItem = 1;

	while(1)
	{
		if (gstAppPosParam.cPinPadUsage == 3)
		{
			nSelcItem = 2;
		}
		else if ((gstAppPosParam.cPinPadUsage & PINPADMODE_PIN) == PINPADMODE_PIN)
		{
			nSelcItem = 1;
		}
		ASSERT_QUIT(PubShowMenuItems(tr("PINPAD USAGE"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 0));
		switch(nSelcItem)
		{
		case 1:
			gstAppPosParam.cPinPadUsage = PINPADMODE_PIN;
			break;
		case 2:
			gstAppPosParam.cPinPadUsage |= PINPADMODE_PIN;
			gstAppPosParam.cPinPadUsage |= PINPADMODE_CARD;
			break;
		default:
			break;
		}
		break;
	}
	ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_PINPADUSAGE, 1, &gstAppPosParam.cPinPadUsage));
	return APP_SUCC;
}

/**
* @brief Set Pinpad Mode
* @param void
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
* @author
* @date
*/
int SetFunctionPinpadPort(void)
{
	char *pszItems[] = {
		tr("1.PINPAD"),
		tr("2.USB"),
		tr("3.RS232"),
	};
	int nSelcItem = 1, nStartItem = 1, nOldPort;

	while(1)
	{
		nOldPort = gstAppPosParam.cPinPadAuxNo;
		if (gstAppPosParam.cPinPadAuxNo == PINPAD)
		{
			nSelcItem = 1;
		}
		else if (gstAppPosParam.cPinPadAuxNo == USB_HOST)
		{
			nSelcItem = 2;
		}
		else
		{
			nSelcItem = 3;
		}
		ASSERT_QUIT(PubShowMenuItems(tr("PINPAD PORT"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 0));
		switch(nSelcItem)
		{
		case 1:
			gstAppPosParam.cPinPadAuxNo = PINPAD;
			break;
		case 2:
			gstAppPosParam.cPinPadAuxNo = USB_HOST;
			break;
		case 3:
			gstAppPosParam.cPinPadAuxNo = RS232;
			break;
		default:
			break;
		}
		break;
	}
	if (nOldPort != gstAppPosParam.cPinPadAuxNo)
	{
		SetControlChkPinpad(YES);
	}
	ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_PINPADAUXNO, 1, &gstAppPosParam.cPinPadAuxNo));
	return APP_SUCC;
}

/**
* @brief Get pinpad usage
* @param void
* @return
* @li
* @li
* @author
* @date
*/
int GetVarPinPadUsage(void)
{
	return gstAppPosParam.cPinPadUsage;
}

/**
* @brief Set callback of L3 in pinpad
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFuncPinpadCallBack(void)
{
	int i, nRet;
	char cSelect;
	STITEM stItem[] = {
	 	{PINPAD_L3_CALLBACK_SELECT_CANDIDATE_LIST, tr("candidate list")},
		{PINPAD_L3_CALLBACK_CARDNUMBER_CONFIRM, tr("card confirm")},
	 };

	if (gstAppPosParam.cPinPadCallbackFlag == YES || gstAppPosParam.cPinPadCallbackFlag == NO || gstAppPosParam.cPinPadCallbackFlag == 0)
	{
		gstAppPosParam.cPinPadCallbackFlag = 0xFF;
	}

	for(i = 0; i < sizeof(stItem) / sizeof(STITEM); i++)
	{
		cSelect =  (stItem[i].cType & gstAppPosParam.cPinPadCallbackFlag) ? 1 : 0;
		nRet = PubSelectYesOrNo(tr("enable?"), stItem[i].pszName , NULL, &cSelect);
		if (nRet == KEY_UP)
		{
			i = i - 2;
			if (i  < -1)
			{
				i = -1;
			}
			continue;
		}
		else if(nRet == KEY_DOWN)
		{
			continue;
		}
		else if (nRet == APP_FAIL || nRet == APP_QUIT)
		{
			break;
		}
		if (cSelect == '1')
		{
			gstAppPosParam.cPinPadCallbackFlag |= stItem[i].cType;
		}
		else
		{
			gstAppPosParam.cPinPadCallbackFlag &= (~stItem[i].cType);
		}
	}

	TRACE("pinpad callback: %0x", gstAppPosParam.cPinPadCallbackFlag);
	ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_PINPADCALLBACKFLAG, 1, &gstAppPosParam.cPinPadCallbackFlag));

	return APP_SUCC;
}

/**
* @brief Set Pinpad type
* @param void
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
* @author
* @date
*/
int SetFunctionPinpadType(void)
{
	char *pszItems[] = {
		tr("1.ME51P"),
		tr("2.SP100"),
	};
	int nSelcItem = 1, nStartItem = 1, nOldType;

	nOldType = gstAppPosParam.cPinpadType;
	while(1)
	{
		nSelcItem = gstAppPosParam.cPinpadType;
		TRACE("nSelcItem = %d", nSelcItem);
		if (nSelcItem == PINPAD_ME51)
		{
			nSelcItem = 1;
		}
		else if (nSelcItem == PINPAD_SP100)
		{
			nSelcItem = 2;
		}
		ASSERT_QUIT(PubShowMenuItems(tr("PINPAD TYPE"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 0));
		switch(nSelcItem)
		{
		case 1:
			gstAppPosParam.cPinpadType = PINPAD_ME51;
			break;
		case 2:
			gstAppPosParam.cPinpadType = PINPAD_SP100;
			gstAppPosParam.cPinPadCallbackFlag = NO;
			ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_PINPADCALLBACKFLAG, 1, &gstAppPosParam.cPinPadCallbackFlag));
			break;
		default:
			break;
		}
		break;
	}
	if (nOldType != gstAppPosParam.cPinpadType)
	{
		SetControlChkPinpad(YES);
	}
	ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_PINPADTYPE, 1, &gstAppPosParam.cPinpadType));
	return APP_SUCC;
}

/**
* @brief Get pinpad type
* @param void
* @return
* @li
* @li
* @author
* @date
*/
int GetVarPinPadType(void)
{
	return gstAppPosParam.cPinpadType;
}

YESORNO GetVarIsPinpadReadCard()
{
	if (GetVarIsPinpad() == YES && (GetVarPinPadUsage() & PINPADMODE_CARD) == PINPADMODE_CARD)
	{
		return YES;
	}
	return NO;
}

void SetL3initStatus(char cStatus)
{
	if (cStatus != L3INIT_INSIDE && cStatus != L3INIT_PINPAD)
	{
		if (cStatus == L3INIT_NONE)
		{
			gstAppPosParam.cL3initStatus = L3INIT_NONE;
			UpdateTagParam(FILE_APPPOSPARAM, TAG_L3INITSTATUS, 1, &gstAppPosParam.cL3initStatus);
		}
		TRACE("cStatus = %d", cStatus);
		return;
	}
	gstAppPosParam.cL3initStatus |= cStatus;
	UpdateTagParam(FILE_APPPOSPARAM, TAG_L3INITSTATUS, 1, &gstAppPosParam.cL3initStatus);
}

char GetL3initStatus()
{
	return gstAppPosParam.cL3initStatus;
}


void SetLockTerminal(char cStatus)
{
    cStatus = cStatus > 0 ? 1 : 0;
    gstAppPosParam.cLockTerminal = cStatus;
	UpdateTagParam(FILE_APPPOSPARAM, TAG_LOCKTERMINAL, 1, &gstAppPosParam.cLockTerminal);
}

char GetLockTerminal()
{
	return gstAppPosParam.cLockTerminal;
}

int SetLockPromptInfo(char *pszPromptInfo)
{
    if (pszPromptInfo == NULL) {
        return APP_FAIL;
    }

    memset(gstAppPosParam.szLockPromptInfo, 0, sizeof(gstAppPosParam.szLockPromptInfo));
    snprintf (gstAppPosParam.szLockPromptInfo, sizeof(gstAppPosParam.szLockPromptInfo), "%s", pszPromptInfo);
    return UpdateTagParam(FILE_APPPOSPARAM, TAG_LOCKPROMPTINFO, sizeof(gstAppPosParam.szLockPromptInfo), gstAppPosParam.szLockPromptInfo);
}

char *GetLockPromptInfo()
{
    return gstAppPosParam.szLockPromptInfo;
}

int GetIsLoadXMLConfig()
{
	if (TxnL3EnumCapk(0, 50, NULL) > 0 &&
		TxnL3EnumEmvConfig(L3_CONTACT, NULL, 50) > 0 && TxnL3EnumEmvConfig(L3_CONTACTLESS, NULL, 50) > 0)
	{
		return FALSE;
	}
	return TRUE;
}

char GetElecSignFlag()
{
	return gstAppPosParam.cElecSignFlag;
}

void SetElecSignFlag(char cFlag)
{
	if (cFlag != YES && cFlag != NO) {
		cFlag = NO;
	}
	gstAppPosParam.cElecSignFlag = cFlag;
	UpdateTagParam(FILE_APPPOSPARAM, TAG_ELECSIGN, 1, &gstAppPosParam.cElecSignFlag);
}

/**
* @brief Set is use Electronic sign
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionIsELecSign(void)

{
	char cSelect = 0;

	if (APP_SUCC != PubGetHardwareSuppot(HARDWARE_SUPPORT_TOUCH_SCREEN, NULL))
	{
		PubMsgDlg(tr("Warning"),tr("Touch screen is not supported"), 1, 3);
		return APP_QUIT;
	}

	cSelect = gstAppPosParam.cElecSignFlag == YES ? 1 : 0;
	ASSERT_RETURNCODE(PubSelectYesOrNo((char*)tr("SET ELEC SIGN"), (char*)tr("OPEN ELEC SIGN?"), NULL, &cSelect));
	switch(cSelect)
	{
	case '0':
	case '1':
		gstAppPosParam.cElecSignFlag = cSelect;
		break;
	default:
		return APP_QUIT;
	}
	ASSERT_FAIL(UpdateTagParam(FILE_APPPOSPARAM, TAG_ELECSIGN, 1, &gstAppPosParam.cElecSignFlag));

	return APP_SUCC;
}





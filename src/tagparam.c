/***************************************************************************
** Copyright (c) 2019 Newland Payment Technology Co., Ltd All right reserved
** File name:tagparam.c
** File indentifier:
** Brief:  processing tag parameters, (comm/posparam/reversal/settle...)
** Current Verion:  v1.0
** Auther: lingdz
** Complete date:
** Modify record:
** Modify date:
** Version:
** Modify content:
***************************************************************************/

#include "apiinc.h"
#include "libapiinc.h"
#include "appinc.h"

// for adding new tag
static STTAGINFO stExPosParam[] =
{
	{FILE_APPPOSPARAM, TAG_PINPADUSAGE, 1, "\x03"},
	{FILE_APPPOSPARAM, TAG_PINPADCALLBACKFLAG, 1, "1"},
	{FILE_APPPOSPARAM, TAG_PINPADTYPE, 1, "\x00"},
	{FILE_APPPOSPARAM, TAG_L3INITSTATUS, 1, "\x00"},
};

/**
* @brief move file content (forward or backward)
* @param [in] nFd - file handle
* @param [in] ulCurrent - start position
* @param [in] nOff - move nOff bytes
* @param [in] pnFileSize - file size
* @param [out] pnFileSize - the file size after the move
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
static int MoveContent(int nFd, ulong ulCurrent, int nOff, uint *pnFileSize)
{
	char cFirstReadFlag = YES;
	char szBuf[1024+1];
	int nReadLen, nRet;
	int nCnt, nSize;
	ulong ulWriteDistance, unPosition;
	ulong nReadIndex = 0, ulReadDistance = 0;

	nSize = *pnFileSize - ulCurrent;
	nCnt = nSize / 1024 + 1;
	while(nCnt >= 1)
	{
		memset(szBuf, 0, sizeof(szBuf));
		if (nCnt > 1)
		{
			nReadLen = 1024;
		}
		else
		{
			nReadLen = nSize % 1024;
		}

		if (nReadLen == 0)
		{
			*pnFileSize += nOff; // update filesize
			return APP_SUCC;
		}
		// goto read location
		if (nOff > 0)
		{
			ulReadDistance += -nReadLen;
			unPosition = SEEK_END;
		}
		else if (nOff < 0)
		{
			ulReadDistance = ulCurrent + nReadIndex;
			unPosition = SEEK_SET;
 		}
		else
		{
			return APP_SUCC;
		}
		ASSERT_FAIL(PubFsSeek(nFd, ulReadDistance, unPosition));
		nRet = PubFsRead(nFd, szBuf, nReadLen);
		if (nRet != nReadLen)
		{
			TRACE("nRet = %d nReadLen = %d", nRet, nReadLen);
			return APP_FAIL;
		}

		//TRACE_HEX(szBuf, nReadLen, "move buf:");
		// goto write location
		ulWriteDistance = ulReadDistance + nOff;
		if (nOff > 0)
		{
			unPosition = SEEK_END;
		}
		else if (nOff < 0)
		{
			unPosition = SEEK_SET;
		}
		ASSERT_FAIL(PubFsSeek(nFd, ulWriteDistance, unPosition));
		nRet = PubFsWrite(nFd, szBuf, nReadLen);
		if (nRet != nReadLen)
		{
			TRACE("nRet = %d, nReadLen", nRet, nReadLen);
			return APP_FAIL;
		}

		if (nOff > 0 && cFirstReadFlag == YES)
		{
			cFirstReadFlag = NO;
			ulReadDistance -= nOff; // The file size has increased by nOff bytes
		}
		else if (nOff < 0)
		{
			nReadIndex += nReadLen; // to goto next read location
		}
		nCnt--;
	}

	*pnFileSize += nOff; // update filesize

	return APP_SUCC;
}

/**
* @brief read one tag from file
* @param [in] nFd - file handle
* @param [out] pszOutBuf - taginfo [tag+len+value]
* @param [out] pnLen  the len of pszOutBuf
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
static int ReadOneTag(int nFd, char *pszOutBuf, int *pnLen)
{
	int nLen, nOff = 0;
	char szTagId[TLV_TAGLEN+1] = {0};
	char szValueLen[TLV_TAGLEN+1] = {0};
	char szBuf[MAX_VALUE_LEN+1] = {0};

	if (pszOutBuf == NULL)
	{
		return APP_FAIL;
	}

	if(PubFsRead(nFd, szTagId, TLV_TAGLEN) != TLV_TAGLEN)
	{
		TRACE_HEX(szTagId, TLV_TAGLEN, "szTagId");
		return APP_FAIL;
	}

	// assign tag
	memcpy(szBuf + nOff, szTagId, TLV_TAGLEN);
	nOff += TLV_TAGLEN;
	if(PubFsRead(nFd, szValueLen, TLV_LENGTHLEN) != TLV_LENGTHLEN)
	{
		TRACE_HEX(szValueLen, TLV_LENGTHLEN, "szTagLen");
		return APP_FAIL;
	}

	// assign value len
	memcpy(szBuf + nOff, szValueLen, TLV_LENGTHLEN);
	nOff += TLV_LENGTHLEN;
	if (PubIsDigitStr(szValueLen) != APP_SUCC)
	{
		TRACE_HEX(szValueLen, TLV_LENGTHLEN, "szTagLen");
		return APP_FAIL;
	}

	nLen = atoi(szValueLen);
	if(PubFsRead(nFd, szBuf + nOff, nLen) != nLen)
	{
		TRACE_HEX(szBuf, nLen, "szBuf");
		return APP_FAIL;
	}
	nOff += nLen;

	memcpy(pszOutBuf, szBuf, nOff);

	*pnLen = nOff;
	//TRACE_HEX(szBuf, nOff, "read tag:");

	return APP_SUCC;
}

/**
* @brief check if the tag is in the file(pszFileName)
* @param [in] pszFileName
* @param [in] nTagid tagid
* @param [out] pulDistance: the tag is Offset in the file
* @param [out] pszValue: the taginfo
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
static int IsTagExist(const char *pszFileName, int nTagid, ulong *pulDistance, char *pszValue)
{
	int nFd, nLen;
	char szValue[MAX_VALUE_LEN+1] = {0};
	char szTagId[TLV_TAGLEN+1] = {0};

	nFd = PubFsOpen(pszFileName, "r");
	if(nFd < 0)
	{
		TRACE("nFd = %d", nFd);
		return APP_FAIL;
	}
	sprintf(szTagId, "%03d", nTagid);
	while (1)
	{
		memset(szValue, 0, sizeof(szValue));
		ASSERT_FILE_FAIL(PubFsTell(nFd, pulDistance), nFd);
		ASSERT_FILE_FAIL(ReadOneTag(nFd, szValue, &nLen), nFd);
		if (memcmp(szValue, szTagId, TLV_TAGLEN) == 0)
		{
			memcpy(pszValue, szValue, nLen);
			break;
		}
	}
	PubFsClose(nFd);

	return APP_SUCC;
}

/**
* @brief check if the tag is in the file(pszFileName)
* @param [in] pszFileName
* @param [in] nTagid tagid
* @param [out] pnLen: len of the value
* @param [out] pszValue: the tag value
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int GetTag(char *pszFileName, int nTagId, int *pnLen, char *pszValue)
{
	int nRet, nFd, nOff = 0, nLen;
	char szTagId[TLV_TAGLEN+1] = {0};
	char szValueLen[TLV_TAGLEN+1] = {0};
	char szValue[MAX_VALUE_LEN+1] = {0};

	if (pszValue == NULL)
	{
		TRACE("pszValue is NULL");
		return APP_FAIL;
	}

	nFd = PubFsOpen(pszFileName, "r");
	if(nFd < 0)
	{
		TRACE("nFd = %d", nFd);
		return APP_FAIL;
	}

	sprintf(szTagId, "%03d", nTagId);
	while(1)
	{
		memset(szValue, 0, sizeof(szValue));
		nRet = ReadOneTag(nFd, szValue, &nLen);
		if (nRet != APP_SUCC)
		{
			PubFsClose(nFd);
			return APP_FAIL;
		}
		if (memcmp(szValue, szTagId, TLV_TAGLEN) == 0)
		{
			nOff += TLV_TAGLEN;
			memcpy(szValueLen, szValue + nOff, TLV_LENGTHLEN);
			nOff += TLV_LENGTHLEN;
			*pnLen = atoi(szValueLen);
			memcpy(pszValue, szValue + nOff, *pnLen);
			break;
		}
	}
	PubFsClose(nFd);

	return APP_SUCC;
}

/**
* @brief merge tag+len+value into tag str(tag:001 len:002 value:12-->00100212)
* @param [in] nTagId
* @param [in] nValueLen -- the len of  the pszTagValue
* @param [in] pszTagValue -- tag value
* @param [out] pnLen: len of the taginfo
* @param [out] pszOutBuf: the tlvstr
* @return
* @li nOff  the len of the pszOutBuf
* @li APP_FAIL
*/
static int TlvTransferToStr(int nTagId, int nValueLen, const char *pszTagValue, char *pszOutBuf)
{
	char szValueLen[TLV_LENGTHLEN+1] = {0};
	char szTagId[TLV_TAGLEN+1] = {0};
	char szBuf[MAX_VALUE_LEN+1] = {0};
	int nOff = 0;

	// tag
	sprintf(szTagId, "%03d", nTagId);
	memcpy(szBuf + nOff, szTagId, TLV_TAGLEN);
	nOff += TLV_TAGLEN;

	// len
	sprintf(szValueLen, "%03d", nValueLen);
	memcpy(szBuf + nOff, szValueLen, TLV_LENGTHLEN);
	nOff += TLV_LENGTHLEN;

	//value
	memcpy(szBuf + nOff, pszTagValue, atoi(szValueLen));
	nOff += atoi(szValueLen);

	memcpy(pszOutBuf, szBuf, nOff);

	return nOff;
}

/**
* @brief  assigned the tag to the STAPPPOSPARAM
* @param [in] pszFileName
* @param [in] nTagId
* @param [in] nValueLen -- the len of  the pszTagValue
* @param [in] pszTagValue -- tag value
* @param [out] pstAppPosParam
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
static void AssignToPosParam(int nTagid, int nValueLen, char *pszTagValue, STAPPPOSPARAM *pstAppPosParam)
{
	switch (nTagid)
	{
	case TAG_MERCHANTID:
		memcpy(pstAppPosParam->szMerchantId, pszTagValue, nValueLen);
		break;
	case TAG_TERMINALID:
		memcpy(pstAppPosParam->szTerminalId, pszTagValue, nValueLen);
		break;
	case TAG_MERCHANTNAMEEN:
		memcpy(pstAppPosParam->szMerchantNameEn, pszTagValue, nValueLen);
		break;
	case TAG_MERCHANTADDR1:
		memcpy(pstAppPosParam->szMerchantAddr[0], pszTagValue, nValueLen);
		break;
	case TAG_MERCHANTADDR2:
		memcpy(pstAppPosParam->szMerchantAddr[1], pszTagValue, nValueLen);
		break;
	case TAG_MERCHANTADDR3:
		memcpy(pstAppPosParam->szMerchantAddr[2], pszTagValue, nValueLen);
		break;
	case TAG_ADMINPWD:
		memcpy(pstAppPosParam->szAdminPwd, pszTagValue, nValueLen);
		break;
	case TAG_USRWD:
		memcpy(pstAppPosParam->szUsrPwd, pszTagValue, nValueLen);
		break;
	case TAG_FUNCPWD:
		memcpy(pstAppPosParam->szFuncPwd, pszTagValue, nValueLen);
		break;
	case TAG_TRANSSWITCH:
		memcpy(pstAppPosParam->sTransSwitch, pszTagValue, nValueLen);
		break;
	case TAG_ACQNAME:
		memcpy(pstAppPosParam->szAcqName, pszTagValue, nValueLen);
		break;
	case TAG_ISSUERNAME:
		memcpy(pstAppPosParam->szIssuerName, pszTagValue, nValueLen);
		break;
	case TAG_CURRENCYNAME:
		memcpy(pstAppPosParam->szCurrencyName, pszTagValue, nValueLen);
		break;
	case TAG_PNTPAGECNT:
		pstAppPosParam->cPrintPageCount = pszTagValue[0];
		break;
	case TAG_PNTDETAIL:
		pstAppPosParam->cIsPntDetail = pszTagValue[0];
		break;
	case TAG_TOMSOBTAINCMD:
		pstAppPosParam->cTomsObtainCmd = pszTagValue[0];
		break;
	case TAG_CVV2:
		pstAppPosParam->cIsNeedCVV2 = pszTagValue[0];
		break;
	case TAG_TIPFLAG:
		pstAppPosParam->cIsTipFlag = pszTagValue[0];
		break;
	case TAG_TIPRATE:
		memcpy(pstAppPosParam->szTipRate, pszTagValue, nValueLen);
		break;
	case TAG_CARDINPUTMODE:
		pstAppPosParam->cIsCardInput = pszTagValue[0];
		break;
	case TAG_ISSALEVOIDSTRIP:
		pstAppPosParam->cIsVoidStrip = pszTagValue[0];
		break;
	case TAG_ISVOIDPIN:
		pstAppPosParam->cIsVoidPin = pszTagValue[0];
		break;
	case TAG_ISSUPPORTRF:
		pstAppPosParam->cIsSupportRF = pszTagValue[0];
		break;
	case TAG_ISEXRF:
		pstAppPosParam->cIsExRF = pszTagValue[0];
		break;
	case TAG_ISPINPAD:
		pstAppPosParam->cIsPinPad = pszTagValue[0];
		break;
	case TAG_PINPADAUXNO:
		pstAppPosParam->cPinPadAuxNo = pszTagValue[0];
		break;
	case TAG_PINPADTIMEOUT:
		memcpy(pstAppPosParam->szPinPadTimeOut, pszTagValue, nValueLen);
		break;
	case TAG_MAINKEYNO:
		memcpy(pstAppPosParam->szMainKeyNo, pszTagValue, nValueLen);
		break;
	case TAG_ENCRYMODE:
		pstAppPosParam->cEncyptMode = pszTagValue[0];
		break;
	case TAG_PINENCRYMODE:
		pstAppPosParam->cPinEncyptMode = pszTagValue[0];
		break;
	case TAG_MAXTRANSCNT:
		memcpy(pstAppPosParam->szMaxTransCount, pszTagValue, nValueLen);
		break;
	case TAG_ISPRINTERRREPORT:
		pstAppPosParam->cIsPrintErrReport = pszTagValue[0];
		break;
	case TAG_SUPPORTCONTACT:
		pstAppPosParam->cIsSupportContact = pszTagValue[0];
		break;
	case TAG_DEFAULTTRANS:
		pstAppPosParam->cDefaultTransType = pszTagValue[0];
		break;
	case TAG_ISSHOWTVRTSI:
		pstAppPosParam->cIsDispEMV_TVRTSI = pszTagValue[0];
		break;
	case TAG_ISSHIELDPAN:
		pstAppPosParam->cIsShieldPan = pszTagValue[0];
		break;
	case TAG_PNTTITLEMODE:
		pstAppPosParam->cPntTitleMode = pszTagValue[0];
		break;
	case TAG_PNTTITLEEN:
		memcpy(pstAppPosParam->szPntTitleEn, pszTagValue, nValueLen);
		break;
	case TAG_SHOWAPPNAME:
		memcpy(pstAppPosParam->szAppDispname, pszTagValue, nValueLen);
		break;
	case TAG_ISADMINPWD:
		pstAppPosParam->cIsAdminPwd = pszTagValue[0];
		break;
	case TAG_PNTMINUS:
		pstAppPosParam->cIsPrintMinus = pszTagValue[0];
		break;
	case TAG_ISPREAUTHSHIELDPAN:
		pstAppPosParam->cIsPreauthShieldPan = pszTagValue[0];
		break;
	case TAG_ISREPNTSETTLE:
		pstAppPosParam->cIsReprintSettle = pszTagValue[0];
		break;
	case TAG_SUPPOTRSWIPE:
		pstAppPosParam->cIsSupportSwipe = pszTagValue[0];
		break;
	case TAG_ISPNTISO:
		pstAppPosParam->cIsPrintIso = pszTagValue[0];
		break;
	case TAG_KEYSYSTEMTYPE:
		pstAppPosParam->cKeySystemType = pszTagValue[0];
		break;
	case TAG_LANGUAGE:
		pstAppPosParam->cLanguage = pszTagValue[0];
		break;
	case TAG_FONTSIZE:
		pstAppPosParam->cFontSize = pszTagValue[0];
		break;
	case TAG_APPCHKVALUE:
		memcpy(pstAppPosParam->szVerChkValue, pszTagValue, nValueLen);
		break;
	case TAG_VERSION:
		memcpy(pstAppPosParam->szVersion, pszTagValue, nValueLen);
		break;
	case TAG_PINPADUSAGE:
		pstAppPosParam->cPinPadUsage = pszTagValue[0];
		break;
	case TAG_PINPADCALLBACKFLAG:
		pstAppPosParam->cPinPadCallbackFlag = pszTagValue[0];
		break;
	case TAG_PINPADTYPE:
		pstAppPosParam->cPinpadType = pszTagValue[0];
		break;
	case TAG_L3INITSTATUS:
		pstAppPosParam->cL3initStatus = pszTagValue[0];
		break;
    case TAG_LOCKTERMINAL:
        pstAppPosParam->cLockTerminal = pszTagValue[0];
        break;
    case TAG_LOCKPROMPTINFO:
        memcpy(pstAppPosParam->szLockPromptInfo, pszTagValue, nValueLen);
        break;
	case TAG_ELECSIGN:
        pstAppPosParam->cElecSignFlag = pszTagValue[0];
        break;
	default:
	break;
	}

	return;
}

/**
* @brief  assigned the tag to the pstAppCommParam
* @param [in] pszFileName
* @param [in] nTagId
* @param [in] nValueLen -- the len of  the pszTagValue
* @param [in] pszTagValue -- tag value
* @param [out] pstAppCommParam
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
static void AssignToCommParam(int nTagid, int nValueLen, char *pszTagValue, STAPPCOMMPARAM *pstAppCommParam)
{
	switch (nTagid)
	{
	case TAG_COMM_TYPE:
		pstAppCommParam->cCommType = pszTagValue[0];
		break;
	case TAG_COMM_TYPE2:
		pstAppCommParam->cCommType2 = pszTagValue[0];
		break;
	case TAG_COMM_ISPREDIAL:
		pstAppCommParam->cPreDialFlag = pszTagValue[0];
		break;
	case TAG_COMM_REDIALNUM:
		pstAppCommParam->cReDialNum = pszTagValue[0];
		break;
	case TAG_COMM_RESENDNUM:
		pstAppCommParam->cReSendNum = pszTagValue[0];
		break;
	case TAG_COMM_TIMEOUT:
		pstAppCommParam->cTimeOut = pszTagValue[0];
		break;
	case TAG_COMM_TPDU:
		memcpy(pstAppCommParam->sTpdu, pszTagValue, nValueLen);
		break;
	case TAG_COMM_PREDIALNO:
		memcpy(pstAppCommParam->szPreDial, pszTagValue, nValueLen);
		break;
	case TAG_COMM_TELNO1:
		memcpy(pstAppCommParam->szTelNum1, pszTagValue, nValueLen);
		break;
	case TAG_COMM_TELNO2:
		memcpy(pstAppCommParam->szTelNum2, pszTagValue, nValueLen);
		break;
	case TAG_COMM_TELNO3:
		memcpy(pstAppCommParam->szTelNum3, pszTagValue, nValueLen);
		break;
	case TAG_COMM_IP1:
		memcpy(pstAppCommParam->szIp1, pszTagValue, nValueLen);
		break;
	case TAG_COMM_PORT1:
		memcpy(pstAppCommParam->szPort1, pszTagValue, nValueLen);
		break;
	case TAG_COMM_IP2:
		memcpy(pstAppCommParam->szIp2, pszTagValue, nValueLen);
		break;
	case TAG_COMM_PORT2:
		memcpy(pstAppCommParam->szPort2, pszTagValue, nValueLen);
		break;
	case TAG_COMM_WIRELESSDIALNUM:
		memcpy(pstAppCommParam->szWirelessDialNum, pszTagValue, nValueLen);
		break;
	case TAG_COMM_APN1:
		memcpy(pstAppCommParam->szAPN1, pszTagValue, nValueLen);
		break;
	case TAG_COMM_APN2:
		memcpy(pstAppCommParam->szAPN2, pszTagValue, nValueLen);
		break;
	case TAG_COMM_USER:
		memcpy(pstAppCommParam->szUser, pszTagValue, nValueLen);
		break;
	case TAG_COMM_PWD:
		memcpy(pstAppCommParam->szPassWd, pszTagValue, nValueLen);
		break;
	case TAG_COMM_SIMPWD:
		memcpy(pstAppCommParam->szSIMPassWd, pszTagValue, nValueLen);
		break;
	case TAG_COMM_MODE:
		pstAppCommParam->cMode = pszTagValue[0];
		break;
	case TAG_COMM_IPADDR:
		memcpy(pstAppCommParam->szIpAddr, pszTagValue, nValueLen);
		break;
	case TAG_COMM_MASK:
		memcpy(pstAppCommParam->szMask, pszTagValue, nValueLen);
		break;
	case TAG_COMM_GATE:
		memcpy(pstAppCommParam->szGate, pszTagValue, nValueLen);
		break;
	case TAG_COMM_DNSIP1:
		memcpy(pstAppCommParam->szDNSIp1, pszTagValue, nValueLen);
		break;
	case TAG_COMM_DNSIP2:
		memcpy(pstAppCommParam->szDNSIp2, pszTagValue, nValueLen);
		break;
	case TAG_COMM_DNSIP3:
		memcpy(pstAppCommParam->szDNSIp3, pszTagValue, nValueLen);
		break;
	case TAG_COMM_DOMAIN1:
		memcpy(pstAppCommParam->szDomain, pszTagValue, nValueLen);
		break;
	case TAG_COMM_DOMAIN2:
		memcpy(pstAppCommParam->szDomain2, pszTagValue, nValueLen);
		break;
	case TAG_COMM_AUXISADDTPDU:
		pstAppCommParam->cAuxIsAddTpdu = pszTagValue[0];
		break;
	case TAG_COMM_ISDNS:
		pstAppCommParam->cIsDns = pszTagValue[0];
		break;
	case TAG_COMM_OFFRESENDNUM:
		pstAppCommParam->cOffResendNum = pszTagValue[0];
		break;
	case TAG_COMM_ISDHCP:
		pstAppCommParam->cIsDHCP = pszTagValue[0];
		break;
	case TAG_COMM_WIFISSID:
		memcpy(pstAppCommParam->szWifiSsid, pszTagValue, nValueLen);
		break;
	case TAG_COMM_WIFIKEY:
		memcpy(pstAppCommParam->szWifiKey, pszTagValue, nValueLen);
		break;
	case TAG_COMM_WIFIMODE:
		pstAppCommParam->cWifiMode = pszTagValue[0];
		break;
	case TAG_COMM_NII:
		memcpy(pstAppCommParam->szNii, pszTagValue, nValueLen);
		break;
	case TAG_COMM_ISSSL:
		pstAppCommParam->cIsSSL = pszTagValue[0];
		break;
    case TAG_COMM_TOMSAPPDOMAIN:
        memcpy(pstAppCommParam->szTOMSAppDomain, pszTagValue, nValueLen);
        break;
    case TAG_COMM_TOMSPARAMDOMAIN:
        memcpy(pstAppCommParam->szTOMSParamDomain, pszTagValue, nValueLen);
        break;
    case TAG_COMM_TOMSKEYPOSDOMAIN:
        memcpy(pstAppCommParam->szTOMSKeyPosDomain, pszTagValue, nValueLen);
        break;
    case TAG_COMM_TOMSFILESERDOMAIN:
        memcpy(pstAppCommParam->szTOMSFileServerDomain, pszTagValue, nValueLen);
        break;
    case TAG_COMM_TOMSTDASDOMAIN:
        memcpy(pstAppCommParam->szTOMSTdasDomain, pszTagValue, nValueLen);
        break;
    case TAG_COMM_TOMSISUSEROID:
        pstAppCommParam->cIsUserOid = pszTagValue[0];
        break;
    case TAG_COMM_TOMSUSEROID:
        memcpy(pstAppCommParam->szTomsUserOid, pszTagValue, nValueLen);
        break;
	default:
		break;
	}

	return;
}

/**
* @brief  assigned the tag to the pstTransParam
* @param [in] pszFileName
* @param [in] nTagId
* @param [in] nValueLen -- the len of  the pszTagValue
* @param [in] pszTagValue -- tag value
* @param [out] pstTransParam
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
static void AssignToTransParam(int nTagid, int nValueLen, char *pszTagValue, STTRANSPARAM *pstTransParam)
{
	char szBuf[4+1] = {0};

	switch (nTagid)
	{
	case TAG_TRANS_ISLOGIN:
		pstTransParam->cIsLogin = pszTagValue[0];
		break;
	case TAG_TRANS_TRACENO:
		memcpy(pstTransParam->szTraceNo, pszTagValue, nValueLen);
		break;
	case TAG_TRANS_BATCH:
		memcpy(pstTransParam->szBatchNo, pszTagValue, nValueLen);
		break;
	case TAG_TRANS_INVOICENO:
		memcpy(pstTransParam->szInvoiceNo, pszTagValue, nValueLen);
		break;
	case TAG_TRANS_LOGINDATETIME:
		memcpy(pstTransParam->sLastLoginDateTime, pszTagValue, nValueLen);
		break;
	case TAG_TRANS_ONLINEUPNUM:
		memcpy(szBuf, pszTagValue, nValueLen);
		PubC2ToInt(&pstTransParam->nOnlineUpNum, (uchar *)szBuf);
		break;
	case TAG_TRANS_QPSLIMIT:
		memcpy(pstTransParam->szQPSLimit, pszTagValue, nValueLen);
		break;
	default:
		break;
	}
}

/**
* @brief  assigned the tag to the pstReversalParam
* @param [in] pszFileName
* @param [in] nTagId
* @param [in] nValueLen -- the len of  the pszTagValue
* @param [in] pszTagValue -- tag value
* @param [out] pstReversalParam
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
static void AssignToReverseParam(int nTagid, int nValueLen, char *pszTagValue, STREVERSALPARAM *pstReversalParam)
{
	STREVERSAL *pstReversal = &pstReversalParam->stReversal;

	switch (nTagid)
	{
	case TAG_REVERSE_FLAG:
		pstReversalParam->cIsReversal = pszTagValue[0];
		break;
	case TAG_REVERSE_HADSENDCNT:
		pstReversalParam->nHaveReversalNum = pszTagValue[0];
		break;
	case TAG_REVERSE_TRANSTYPE:
		pstReversal->cTransType = pszTagValue[0];
		break;
	case TAG_REVERSE_TRANSATTR:
		pstReversal->cTransAttr = pszTagValue[0];
		break;
	case TAG_REVERSE_PAN:
		memcpy(pstReversal->szPan, pszTagValue, nValueLen);
		break;
	case TAG_REVERSE_PROCESSCODE:
		memcpy(pstReversal->szProcCode, pszTagValue, nValueLen);
		break;
	case TAG_REVERSE_AMT:
		memcpy(pstReversal->szAmount, pszTagValue, nValueLen);
		break;
	case TAG_REVERSE_TRACE:
		memcpy(pstReversal->szTrace, pszTagValue, nValueLen);
		break;
	case TAG_REVERSE_EXDATE:
		memcpy(pstReversal->szExpDate, pszTagValue, nValueLen);
		break;
	case TAG_REVERSE_INPUTMODE:
		memcpy(pstReversal->szInputMode, pszTagValue, nValueLen);
		break;
	case TAG_REVERSE_CARDSN:
		memcpy(pstReversal->szCardSerialNo, pszTagValue, nValueLen);
		break;
	case TAG_REVERSE_NII:
		memcpy(pstReversal->szNii, pszTagValue, nValueLen);
		break;
	case TAG_REVERSE_SERVERCODE:
		memcpy(pstReversal->szServerCode, pszTagValue, nValueLen);
		break;
	case TAG_REVERSE_TRACK1:
		memcpy(pstReversal->szTrack1, pszTagValue, nValueLen);
		break;
	case TAG_REVERSE_TRACK2:
		memcpy(pstReversal->szTrack2, pszTagValue, nValueLen);
		break;
	case TAG_REVERSE_RESPONSECODE:
		memcpy(pstReversal->szResponse, pszTagValue, nValueLen);
		break;
	case TAG_REVERSE_INVOICE:
		memcpy(pstReversal->szInvoice, pszTagValue, nValueLen);
		break;
	case TAG_REVERSE_CVV2:
		memcpy(pstReversal->szCVV2, pszTagValue, nValueLen);
		break;
	case TAG_REVERSE_ADD1:
		memcpy(pstReversal->szFieldAdd1, pszTagValue, nValueLen);
		pstReversal->nFieldAdd1Len = nValueLen;
		break;
	case TAG_REVERSE_CASHBACKAMT:
		memcpy(pstReversal->szCashbackAmount, pszTagValue, nValueLen);
		break;
	case TAG_REVERSE_OLDAUTHCODE:
		memcpy(pstReversal->szOldAuthCode, pszTagValue, nValueLen);
		break;
	default:
		break;
	}
}

/**
* @brief  assigned the tag to the pstSettleParam
* @param [in] pszFileName
* @param [in] nTagId
* @param [in] nValueLen -- the len of  the pszTagValue
* @param [in] pszTagValue -- tag value
* @param [out] pstSettleParam
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
static void AssignToSettleParam(int nTagid, int nValueLen, char *pszTagValue, STSETTLEPARAM *pstSettleParam)
{
	char szBuf[2+1] = {0};

	switch (nTagid)
	{
	case TAG_SETTLE_OFFLINEUNSENDNUM:
		memcpy(szBuf, pszTagValue, nValueLen);
		PubC2ToInt((uint *)&pstSettleParam->nOffLineUnsendNum, (uchar *)szBuf);
		break;
	case TAG_SETTLE_RECNUM:
		memcpy(szBuf, pszTagValue, nValueLen);
		PubC2ToInt((uint *)&pstSettleParam->nRecNum, (uchar *)szBuf);
		break;
	case TAG_SETTLE_HADRESENDNUM:
		pstSettleParam->nHaveReSendNum = pszTagValue[0];
		break;
	case TAG_SETTLE_PNTSETTLEHALT:
		pstSettleParam->cPrintSettleHalt = pszTagValue[0];
		break;
	case TAG_SETTLE_PNTDETAILHALT:
		pstSettleParam->cPrintDetialHalt = pszTagValue[0];
		break;
	case TAG_SETTLE_PNTRECORDHALT:
		pstSettleParam->cPrintRecordHaltFlag = pszTagValue[0];
		break;
	case TAG_SETTLE_TOTALMATCHFLAG:
		pstSettleParam->cIsTotalMatch = pszTagValue[0];
		break;
	case TAG_SETTLE_BATCHHALT:
		pstSettleParam->cBatchHaltFlag = pszTagValue[0];
		break;
	case TAG_SETTLE_CLRSETTLE:
		pstSettleParam->cClrSettleDataFlag = pszTagValue[0];
		break;
	case TAG_SETTLE_BATCHOFFLINEHALT:
		pstSettleParam->nBatchMagOfflinHaltFlag = pszTagValue[0];
		break;
	case TAG_SETTLE_BATCH_SUM:
		memcpy(szBuf, pszTagValue, nValueLen);
		PubC2ToInt((uint *)&pstSettleParam->nBatchSum, (uchar *)szBuf);
		break;
	case TAG_SETTLE_DATETIME:
		memcpy(pstSettleParam->sSettleDateTime, pszTagValue, nValueLen);
		break;
	case TAG_SETTLE_SALENUM:
		memcpy(szBuf, pszTagValue, nValueLen);
		PubC2ToInt(&pstSettleParam->stSettle._SaleNum, (uchar *)szBuf);
		break;
	case TAG_SETTLE_SALEAMT:
		memcpy(pstSettleParam->stSettle._SaleAmount, pszTagValue, nValueLen);
		break;
	case TAG_SETTLE_VOIDSALENUM:
		memcpy(szBuf, pszTagValue, nValueLen);
		PubC2ToInt(&pstSettleParam->stSettle._VoidSaleNum, (uchar *)szBuf);
		break;
	case TAG_SETTLE_VOIDSALEAMT:
		memcpy(pstSettleParam->stSettle._VoidSaleAmount, pszTagValue, nValueLen);
		break;
	case TAG_SETTLE_AUTHSALENUM:
		memcpy(szBuf, pszTagValue, nValueLen);
		PubC2ToInt(&pstSettleParam->stSettle._AuthSaleNum, (uchar *)szBuf);
		break;
	case TAG_SETTLE_AUTHSALEAMT:
		memcpy(pstSettleParam->stSettle._AuthSaleAmount, pszTagValue, nValueLen);
		break;
	case TAG_SETTLE_VOIDAUTHSALENUM:
		memcpy(szBuf, pszTagValue, nValueLen);
		PubC2ToInt(&pstSettleParam->stSettle._VoidAuthSaleNum, (uchar *)szBuf);
		break;
	case TAG_SETTLE_VOIDAUTHSALEAMT:
		memcpy(pstSettleParam->stSettle._VoidAuthSaleAmount, pszTagValue, nValueLen);
		break;
	case TAG_SETTLE_PREAUTHNUM:
		memcpy(szBuf, pszTagValue, nValueLen);
		PubC2ToInt(&pstSettleParam->stSettle._PreAuthNum, (uchar *)szBuf);
		break;
	case TAG_SETTLE_PREAUTHAMT:
		memcpy(pstSettleParam->stSettle._PreAuthAmount, pszTagValue, nValueLen);
		break;
	case TAG_SETTLE_REFUNDNUM:
		memcpy(szBuf, pszTagValue, nValueLen);
		PubC2ToInt(&pstSettleParam->stSettle._RefundNum, (uchar *)szBuf);
		break;
	case TAG_SETTLE_REFUNDAMT:
		memcpy(pstSettleParam->stSettle._RefundAmount, pszTagValue, nValueLen);
		break;
	case TAG_SETTLE_ADJUSTNUM:
		memcpy(szBuf, pszTagValue, nValueLen);
		PubC2ToInt(&pstSettleParam->stSettle._AdjustNum, (uchar *)szBuf);
		break;
	case TAG_SETTLE_ADJUSTAMT:
		memcpy(pstSettleParam->stSettle._AdjustAmount, pszTagValue, nValueLen);
		break;
	case TAG_SETTLE_VOIDPREAUTHNUM:
		memcpy(szBuf, pszTagValue, nValueLen);
		PubC2ToInt(&pstSettleParam->stSettle._VoidPreAuthNum, (uchar *)szBuf);
		break;
	case TAG_SETTLE_VOIDPREAUTHAMT:
		memcpy(pstSettleParam->stSettle._VOidPreAuthAmount, pszTagValue, nValueLen);
		break;
	case TAG_SETTLE_OFFLINENUM:
		memcpy(szBuf, pszTagValue, nValueLen);
		PubC2ToInt(&pstSettleParam->stSettle._OfflineNum, (uchar *)szBuf);
		break;
	case TAG_SETTLE_OFFLINEAMT:
		memcpy(pstSettleParam->stSettle._OfflineAmount, pszTagValue, nValueLen);
		break;
	default:
		break;
	}
}

/**
* @brief assigned tag to the pszResult(like STAPPPOSPARAM/STAPPCOMMPARAM ...) from the file(pszFileName)
* @param [in] pszFileName
* @param [out] pszParam: the param of  STAPPPOSPARAM/STAPPCOMMPARAM...
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int LoadTagParam(char *pszFileName, void *pszParam)
{
	char szTagId[TLV_TAGLEN+1] = {0};
	char szValueLen[TLV_LENGTHLEN+1] = {0};
	char szBuf[MAX_VALUE_LEN+1] = {0};
	char szValue[MAX_VALUE_LEN+1];
	int nRet, nFd, nNum = 0, nOff;
	int nLen, nValueLen, nTagId;

	ExportFileContent(pszFileName);
	//Load paramter from file
	TRACE("===== Load data from file[%s] =====", pszFileName);
	nFd = PubFsOpen(pszFileName, "r");
	if(nFd < 0)
	{
		TRACE("nFd = %d filename = %s", nFd, pszFileName);
		return APP_FAIL;
	}

	ASSERT_FILE_FAIL(PubFsSeek(nFd, 0, SEEK_SET), nFd);
	while (1)
	{
		memset(szBuf, 0, sizeof(szBuf));
		memset(szValue, 0, sizeof(szValue));
		nRet = ReadOneTag(nFd, szBuf, &nLen);
		if (nRet != APP_SUCC)
		{
			break;
		}

		nOff = 0;
		nNum++;
		memcpy(szTagId, szBuf, TLV_TAGLEN);
		nOff += TLV_TAGLEN;
		if (PubIsDigitStr(szTagId) != APP_SUCC)
		{
			TRACE_HEX(szBuf, nLen, "valid tag:");
			PubFsClose(nFd);
			return APP_FAIL;
		}
		memcpy(szValueLen, szBuf + nOff, TLV_LENGTHLEN);
		nOff += TLV_LENGTHLEN;

		nTagId = atoi(szTagId);
		nValueLen = atoi(szValueLen);
		memcpy(szValue, szBuf + nOff, nValueLen);

		if (strcmp(pszFileName, FILE_APPPOSPARAM) == 0)
		{
			AssignToPosParam(nTagId, nValueLen, szValue, (STAPPPOSPARAM *)pszParam);
		}
		else if (strcmp(pszFileName, FILE_APPCOMMPARAM) == 0)
		{
			AssignToCommParam(nTagId, nValueLen, szValue, (STAPPCOMMPARAM *)pszParam);
		}
		else if (strcmp(pszFileName, FILE_APPTRANSPARAM) == 0)
		{
			AssignToTransParam(nTagId, nValueLen, szValue, (STTRANSPARAM *)pszParam);
		}
		else if (strcmp(pszFileName, FILE_APPREVERSAL) == 0)
		{
			AssignToReverseParam(nTagId, nValueLen, szValue, (STREVERSALPARAM *)pszParam);
		}
		else if (strcmp(pszFileName, FILE_APPSETTLEMENT) == 0)
		{
			AssignToSettleParam(nTagId, nValueLen, szValue, (STSETTLEPARAM *)pszParam);
		}
	}

	PubFsClose(nFd);
	TRACE("loadtagNum = %d ", nNum);

	return APP_SUCC;
}

/**
* @brief Add tagparam at the end of the file
* @param [in] pszFileName
* @param [in] nTagId
* @param [in] nValueLen -- the len of  the pszTagValue
* @param [in] pszTagValue -- tag value
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
static int AddTagParam(const char *pszFileName, int nTagId, int nValueLen, char *pszTagValue)
{
	int nFd, nRet, nLen;
	char szBuf[MAX_VALUE_LEN+1] = {0};

	nFd = PubFsOpen(pszFileName, "w+");
	if(nFd < 0)
	{
		TRACE("nFd = %d", nFd);
		return APP_FAIL;
	}

	if (nValueLen > MAX_VALUE_LEN - 6) // MAX_VALUE_LEN - TLV_LENGTHLEN - TLV_TAGLEN
	{
		TRACE("add value fail nValueLen = %d", nValueLen);
		return APP_FAIL;
	}
	nLen = TlvTransferToStr(nTagId, nValueLen, pszTagValue, szBuf);
	ASSERT_FILE_FAIL(PubFsSeek(nFd, 0L, SEEK_END), nFd);
	nRet = PubFsWrite(nFd, szBuf, nLen);
	if(nRet != nLen)
	{
		TRACE_HEX(szBuf, nLen, "add tag:");
		TRACE("PubFsWrite error nRet = %d	nlen = %d", nRet, nLen);
		PubFsClose(nFd);
		return APP_FAIL;
	}
	PubFsClose(nFd);

	return APP_SUCC;
}

/**
* @brief update tag, if the tag has already in the file ,then update, othewise add the tag at the end of the file
* @param [in] pszFileName
* @param [in] nTagId
* @param [in] nValueLen -- the len of  the pszTagValue
* @param [in] pszTagValue -- tag value
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int UpdateTagParam(const char *pszFileName, int nTagId, int nValueLen, char *pszTagValue)
{
	int nRet, nFd, nLen, nOff = 0;
	uint nFileSize, nFileSizeBeforeUpdate, nMoveSize;
	char szBuf[MAX_VALUE_LEN+1] = {0};
	char szOldValueLen[TLV_LENGTHLEN+1] = {0};
	ulong ulDistance;

	if (IsTagExist(pszFileName, nTagId, &ulDistance, szBuf) != APP_SUCC)
	{
		ASSERT_FAIL(AddTagParam(pszFileName, nTagId, nValueLen, pszTagValue));
		return APP_SUCC;
	}

	ASSERT_FAIL(PubFsFileSize(pszFileName, &nFileSize));
	nFileSizeBeforeUpdate = nFileSize;
	nFd = PubFsOpen(pszFileName, "w+");
	if(nFd < 0)
	{
		TRACE("nFd = %d", nFd);
		return APP_FAIL;
	}
	nOff += TLV_TAGLEN;
	memcpy(szOldValueLen, szBuf + nOff, TLV_LENGTHLEN);
	nOff += TLV_LENGTHLEN;
	nOff += atoi(szOldValueLen);

	nMoveSize = nValueLen - atoi(szOldValueLen);
	if (nMoveSize != 0)
	{
		ASSERT_FILE_FAIL(MoveContent(nFd, ulDistance + nOff, nMoveSize, &nFileSize),  nFd);
	}

	memset(szBuf, 0, sizeof(szBuf));
	nLen = TlvTransferToStr(nTagId, nValueLen, pszTagValue, szBuf);
	//TRACE_HEX(szBuf, nLen, "update tag:");
	ASSERT_FILE_FAIL(PubFsSeek(nFd, ulDistance, SEEK_SET), nFd);
	nRet = PubFsWrite(nFd, szBuf, nLen);
	if (nRet != nLen)
	{
		TRACE("nRet = %d nLen = %d", nRet, nLen);
		PubFsClose(nFd);
		return APP_FAIL;
	}
	if (nFileSizeBeforeUpdate > nFileSize)
	{
		ASSERT_FILE_FAIL(PubFsTruncate(pszFileName, nFileSize), nFd);
	}
	PubFsClose(nFd);

	return APP_SUCC;
}

/**
* @brief update reversal data
* @param [in] stReversal
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int UpdateReverseData(char *pszFileName, STREVERSAL stReversal)
{
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_REVERSE_TRANSTYPE, 1, &stReversal.cTransType));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_REVERSE_TRANSATTR, 1, &stReversal.cTransAttr));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_REVERSE_PAN, sizeof(stReversal.szPan) - 1, stReversal.szPan));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_REVERSE_PROCESSCODE, sizeof(stReversal.szProcCode) - 1, stReversal.szProcCode));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_REVERSE_AMT, sizeof(stReversal.szAmount) - 1, stReversal.szAmount));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_REVERSE_TRACE, sizeof(stReversal.szTrace) - 1, stReversal.szTrace));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_REVERSE_EXDATE, sizeof(stReversal.szExpDate) - 1, stReversal.szExpDate));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_REVERSE_INPUTMODE, sizeof(stReversal.szInputMode) - 1, stReversal.szInputMode));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_REVERSE_CARDSN, sizeof(stReversal.szCardSerialNo) - 1, stReversal.szCardSerialNo));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_REVERSE_NII, sizeof(stReversal.szNii) - 1, stReversal.szNii));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_REVERSE_SERVERCODE, sizeof(stReversal.szServerCode) - 1, stReversal.szServerCode));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_REVERSE_TRACK1, sizeof(stReversal.szTrack1) - 1, stReversal.szTrack1));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_REVERSE_TRACK2, sizeof(stReversal.szTrack2) - 1, stReversal.szTrack2));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_REVERSE_RESPONSECODE, sizeof(stReversal.szResponse) - 1, stReversal.szResponse));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_REVERSE_INVOICE, sizeof(stReversal.szInvoice) - 1, stReversal.szInvoice));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_REVERSE_CVV2, sizeof(stReversal.szCVV2) - 1, stReversal.szCVV2));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_REVERSE_ADD1, stReversal.nFieldAdd1Len, stReversal.szFieldAdd1));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_REVERSE_CASHBACKAMT, sizeof(stReversal.szCashbackAmount) - 1, stReversal.szCashbackAmount));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_REVERSE_OLDAUTHCODE, sizeof(stReversal.szOldAuthCode) - 1, stReversal.szOldAuthCode));

	return APP_SUCC;
}

/**
* @brief update commparam
* @param [in] stAppCommParam
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int UpdateAppCommParam(char *pszFileName, STAPPCOMMPARAM stAppCommParam)
{
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_COMM_TYPE, 1, &stAppCommParam.cCommType));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_COMM_TYPE2, 1, &stAppCommParam.cCommType2));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_COMM_ISPREDIAL, 1, &stAppCommParam.cPreDialFlag));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_COMM_REDIALNUM, 1, &stAppCommParam.cReDialNum));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_COMM_RESENDNUM, 1, &stAppCommParam.cReSendNum));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_COMM_TIMEOUT, 1, &stAppCommParam.cTimeOut));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_COMM_TPDU, 5, stAppCommParam.sTpdu));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_COMM_PREDIALNO, strlen(stAppCommParam.szPreDial), stAppCommParam.szPreDial));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_COMM_TELNO1, strlen(stAppCommParam.szTelNum1), stAppCommParam.szTelNum1));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_COMM_TELNO2, strlen(stAppCommParam.szTelNum2), stAppCommParam.szTelNum2));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_COMM_TELNO3, strlen(stAppCommParam.szTelNum3), stAppCommParam.szTelNum3));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_COMM_IP1, strlen(stAppCommParam.szIp1), stAppCommParam.szIp1));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_COMM_PORT1, strlen(stAppCommParam.szPort1), stAppCommParam.szPort1));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_COMM_IP2, strlen(stAppCommParam.szIp2), stAppCommParam.szIp2));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_COMM_PORT2, strlen(stAppCommParam.szPort2), stAppCommParam.szPort2));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_COMM_WIRELESSDIALNUM, strlen(stAppCommParam.szWirelessDialNum), stAppCommParam.szWirelessDialNum));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_COMM_APN1, strlen(stAppCommParam.szAPN1), stAppCommParam.szAPN1));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_COMM_APN2, strlen(stAppCommParam.szAPN2), stAppCommParam.szAPN2));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_COMM_USER, strlen(stAppCommParam.szUser), stAppCommParam.szUser));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_COMM_PWD, strlen(stAppCommParam.szPassWd), stAppCommParam.szPassWd));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_COMM_SIMPWD, strlen(stAppCommParam.szSIMPassWd), stAppCommParam.szSIMPassWd));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_COMM_MODE, 1, &stAppCommParam.cMode));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_COMM_IPADDR, strlen(stAppCommParam.szIpAddr), stAppCommParam.szIpAddr));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_COMM_MASK, strlen(stAppCommParam.szMask), stAppCommParam.szMask));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_COMM_GATE, strlen(stAppCommParam.szGate), stAppCommParam.szGate));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_COMM_DNSIP1, strlen(stAppCommParam.szDNSIp1), stAppCommParam.szDNSIp1));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_COMM_DNSIP2, strlen(stAppCommParam.szDNSIp2), stAppCommParam.szDNSIp2));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_COMM_DNSIP3, strlen(stAppCommParam.szDNSIp3), stAppCommParam.szDNSIp3));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_COMM_DOMAIN1, strlen(stAppCommParam.szDomain), stAppCommParam.szDomain));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_COMM_DOMAIN2, strlen(stAppCommParam.szDomain2), stAppCommParam.szDomain2));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_COMM_AUXISADDTPDU, 1, &stAppCommParam.cAuxIsAddTpdu));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_COMM_ISDNS, 1, &stAppCommParam.cIsDns));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_COMM_OFFRESENDNUM, 1, &stAppCommParam.cOffResendNum));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_COMM_ISDHCP, 1, &stAppCommParam.cIsDHCP));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_COMM_WIFISSID, strlen(stAppCommParam.szWifiSsid), stAppCommParam.szWifiSsid));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_COMM_WIFIKEY, strlen(stAppCommParam.szWifiKey), stAppCommParam.szWifiKey));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_COMM_WIFIMODE, 1, &stAppCommParam.cWifiMode));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_COMM_NII, strlen(stAppCommParam.szNii), stAppCommParam.szNii));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_COMM_ISSSL, 1, &stAppCommParam.cIsSSL));

	SetAppCommParam(stAppCommParam);

	return APP_SUCC;
}

/**
* @brief update posparam
* @param [in] stAppPosParam
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int UpdateAppPosParam(char *pszFileName, STAPPPOSPARAM stAppPosParam)
{
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_MERCHANTID, strlen(stAppPosParam.szMerchantId), stAppPosParam.szMerchantId));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_TERMINALID, strlen(stAppPosParam.szTerminalId), stAppPosParam.szTerminalId));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_MERCHANTNAMEEN, strlen(stAppPosParam.szMerchantNameEn), stAppPosParam.szMerchantNameEn));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_ADMINPWD, strlen(stAppPosParam.szAdminPwd), stAppPosParam.szAdminPwd));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_USRWD, strlen(stAppPosParam.szUsrPwd), stAppPosParam.szUsrPwd));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_FUNCPWD, strlen(stAppPosParam.szFuncPwd), stAppPosParam.szFuncPwd));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_TRANSSWITCH, sizeof(stAppPosParam.sTransSwitch), stAppPosParam.sTransSwitch));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_ACQNAME, strlen(stAppPosParam.szAcqName), stAppPosParam.szAcqName));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_ISSUERNAME, strlen(stAppPosParam.szIssuerName), stAppPosParam.szIssuerName));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_CURRENCYNAME, strlen(stAppPosParam.szCurrencyName), stAppPosParam.szCurrencyName));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_PNTPAGECNT, 1, &stAppPosParam.cPrintPageCount));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_PNTDETAIL, 1, &stAppPosParam.cIsPntDetail));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_TOMSOBTAINCMD, 1, &stAppPosParam.cTomsObtainCmd));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_CVV2, 1, &stAppPosParam.cIsNeedCVV2));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_TIPFLAG, 1, &stAppPosParam.cIsTipFlag));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_TIPRATE, strlen(stAppPosParam.szTipRate), stAppPosParam.szTipRate));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_CARDINPUTMODE, 1, &stAppPosParam.cIsCardInput));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_ISSALEVOIDSTRIP, 1, &stAppPosParam.cIsVoidStrip));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_ISVOIDPIN, 1, &stAppPosParam.cIsVoidPin));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_ISSUPPORTRF, 1, &stAppPosParam.cIsSupportRF));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_ISEXRF, 1, &stAppPosParam.cIsExRF));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_ISPINPAD, 1, &stAppPosParam.cIsPinPad));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_PINPADAUXNO, 1, &stAppPosParam.cPinPadAuxNo));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_PINPADTIMEOUT, strlen(stAppPosParam.szPinPadTimeOut), stAppPosParam.szPinPadTimeOut));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_MAINKEYNO, strlen(stAppPosParam.szMainKeyNo), stAppPosParam.szMainKeyNo));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_ENCRYMODE, 1, &stAppPosParam.cEncyptMode));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_PINENCRYMODE, 1, &stAppPosParam.cPinEncyptMode));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_MAXTRANSCNT, strlen(stAppPosParam.szMaxTransCount), stAppPosParam.szMaxTransCount));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_ISPRINTERRREPORT, 1, &stAppPosParam.cIsPrintErrReport));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_SUPPORTCONTACT, 1, &stAppPosParam.cIsSupportContact));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_DEFAULTTRANS, 1, &stAppPosParam.cDefaultTransType));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_ISSHOWTVRTSI, 1, &stAppPosParam.cIsDispEMV_TVRTSI));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_ISSHIELDPAN, 1, &stAppPosParam.cIsShieldPan));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_PNTTITLEMODE, 1, &stAppPosParam.cPntTitleMode));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_PNTTITLEEN, strlen(stAppPosParam.szPntTitleEn), stAppPosParam.szPntTitleEn));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_SHOWAPPNAME, strlen(stAppPosParam.szAppDispname), stAppPosParam.szAppDispname));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_ISADMINPWD, 1, &stAppPosParam.cIsAdminPwd));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_PNTMINUS, 1, &stAppPosParam.cIsPrintMinus));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_ISPREAUTHSHIELDPAN, 1, &stAppPosParam.cIsPreauthShieldPan));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_ISREPNTSETTLE, 1, &stAppPosParam.cIsReprintSettle));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_SUPPOTRSWIPE, 1, &stAppPosParam.cIsSupportSwipe));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_ISPNTISO, 1, &stAppPosParam.cIsPrintIso));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_KEYSYSTEMTYPE, 1, &stAppPosParam.cKeySystemType));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_LANGUAGE, 1, &stAppPosParam.cLanguage));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_FONTSIZE, 1, &stAppPosParam.cFontSize));
	ASSERT_FAIL(UpdateTagParam(pszFileName, TAG_ELECSIGN, 1, &stAppPosParam.cElecSignFlag));

	SetAppPosParam(stAppPosParam);

	return APP_SUCC;
}

/**
* @brief  save the commparam in a file
* @param [in] stAppCommParam
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int InitCommParamFile(STAPPCOMMPARAM stAppCommParam)
{
	char *pszFileName = FILE_APPCOMMPARAM;

	if (PubFsExist(pszFileName) == NAPI_OK)
	{
		return APP_SUCC;
	}

	ASSERT_FAIL(AddTagParam(pszFileName, TAG_COMM_TYPE, 1, &stAppCommParam.cCommType));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_COMM_TYPE2, 1, &stAppCommParam.cCommType2));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_COMM_ISPREDIAL, 1, &stAppCommParam.cPreDialFlag));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_COMM_REDIALNUM, 1, &stAppCommParam.cReDialNum));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_COMM_RESENDNUM, 1, &stAppCommParam.cReSendNum));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_COMM_TIMEOUT, 1, &stAppCommParam.cTimeOut));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_COMM_TPDU, 5, stAppCommParam.sTpdu));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_COMM_PREDIALNO, strlen(stAppCommParam.szPreDial), stAppCommParam.szPreDial));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_COMM_TELNO1, strlen(stAppCommParam.szTelNum1), stAppCommParam.szTelNum1));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_COMM_TELNO2, strlen(stAppCommParam.szTelNum2), stAppCommParam.szTelNum2));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_COMM_TELNO3, strlen(stAppCommParam.szTelNum3), stAppCommParam.szTelNum3));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_COMM_IP1, strlen(stAppCommParam.szIp1), stAppCommParam.szIp1));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_COMM_PORT1, strlen(stAppCommParam.szPort1), stAppCommParam.szPort1));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_COMM_IP2, strlen(stAppCommParam.szIp2), stAppCommParam.szIp2));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_COMM_PORT2, strlen(stAppCommParam.szPort2), stAppCommParam.szPort2));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_COMM_WIRELESSDIALNUM, strlen(stAppCommParam.szWirelessDialNum), stAppCommParam.szWirelessDialNum));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_COMM_APN1, strlen(stAppCommParam.szAPN1), stAppCommParam.szAPN1));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_COMM_APN2, strlen(stAppCommParam.szAPN2), stAppCommParam.szAPN2));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_COMM_USER, strlen(stAppCommParam.szUser), stAppCommParam.szUser));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_COMM_PWD, strlen(stAppCommParam.szPassWd), stAppCommParam.szPassWd));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_COMM_SIMPWD, strlen(stAppCommParam.szSIMPassWd), stAppCommParam.szSIMPassWd));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_COMM_MODE, 1, &stAppCommParam.cMode));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_COMM_IPADDR, strlen(stAppCommParam.szIpAddr), stAppCommParam.szIpAddr));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_COMM_MASK, strlen(stAppCommParam.szMask), stAppCommParam.szMask));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_COMM_GATE, strlen(stAppCommParam.szGate), stAppCommParam.szGate));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_COMM_DNSIP1, strlen(stAppCommParam.szDNSIp1), stAppCommParam.szDNSIp1));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_COMM_DNSIP2, strlen(stAppCommParam.szDNSIp2), stAppCommParam.szDNSIp2));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_COMM_DNSIP3, strlen(stAppCommParam.szDNSIp3), stAppCommParam.szDNSIp3));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_COMM_DOMAIN1, strlen(stAppCommParam.szDomain), stAppCommParam.szDomain));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_COMM_DOMAIN2, strlen(stAppCommParam.szDomain2), stAppCommParam.szDomain2));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_COMM_AUXISADDTPDU, 1, &stAppCommParam.cAuxIsAddTpdu));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_COMM_ISDNS, 1, &stAppCommParam.cIsDns));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_COMM_OFFRESENDNUM, 1, &stAppCommParam.cOffResendNum));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_COMM_ISDHCP, 1, &stAppCommParam.cIsDHCP));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_COMM_WIFISSID, strlen(stAppCommParam.szWifiSsid), stAppCommParam.szWifiSsid));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_COMM_WIFIKEY, strlen(stAppCommParam.szWifiKey), stAppCommParam.szWifiKey));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_COMM_WIFIMODE, 1, &stAppCommParam.cWifiMode));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_COMM_NII, strlen(stAppCommParam.szNii), stAppCommParam.szNii));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_COMM_ISSSL, 1, &stAppCommParam.cIsSSL));

    ASSERT_FAIL(AddTagParam(pszFileName, TAG_COMM_TOMSAPPDOMAIN, strlen(stAppCommParam.szTOMSAppDomain), stAppCommParam.szTOMSAppDomain));
    ASSERT_FAIL(AddTagParam(pszFileName, TAG_COMM_TOMSPARAMDOMAIN, strlen(stAppCommParam.szTOMSParamDomain), stAppCommParam.szTOMSParamDomain));
    ASSERT_FAIL(AddTagParam(pszFileName, TAG_COMM_TOMSKEYPOSDOMAIN, strlen(stAppCommParam.szTOMSKeyPosDomain), stAppCommParam.szTOMSKeyPosDomain));
    ASSERT_FAIL(AddTagParam(pszFileName, TAG_COMM_TOMSFILESERDOMAIN, strlen(stAppCommParam.szTOMSFileServerDomain), stAppCommParam.szTOMSFileServerDomain));
    ASSERT_FAIL(AddTagParam(pszFileName, TAG_COMM_TOMSTDASDOMAIN, strlen(stAppCommParam.szTOMSTdasDomain), stAppCommParam.szTOMSTdasDomain));

    ASSERT_FAIL(AddTagParam(pszFileName, TAG_COMM_TOMSISUSEROID, 1, &stAppCommParam.cIsUserOid));
    ASSERT_FAIL(AddTagParam(pszFileName, TAG_COMM_TOMSUSEROID, strlen(stAppCommParam.szTomsUserOid), stAppCommParam.szTomsUserOid));

	return APP_SUCC;
}

/**
* @brief  save the posparam in a file
* @param [in] stAppPosParam
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int InitPosParamFile(STAPPPOSPARAM stAppPosParam)
{
	char *pszFileName = FILE_APPPOSPARAM;
	char szChkValue[APPCHKLEN + 1];

	if (PubFsExist(pszFileName) == NAPI_OK)
	{
		return APP_SUCC;
	}

	ASSERT_FAIL(AddTagParam(pszFileName, TAG_MERCHANTID, strlen(stAppPosParam.szMerchantId), stAppPosParam.szMerchantId));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_TERMINALID, strlen(stAppPosParam.szTerminalId), stAppPosParam.szTerminalId));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_MERCHANTNAMEEN, strlen(stAppPosParam.szMerchantNameEn), stAppPosParam.szMerchantNameEn));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_MERCHANTADDR1, strlen(stAppPosParam.szMerchantAddr[0]), stAppPosParam.szMerchantAddr[0]));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_MERCHANTADDR2, strlen(stAppPosParam.szMerchantAddr[1]), stAppPosParam.szMerchantAddr[1]));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_MERCHANTADDR3, strlen(stAppPosParam.szMerchantAddr[2]), stAppPosParam.szMerchantAddr[2]));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_ADMINPWD, strlen(stAppPosParam.szAdminPwd), stAppPosParam.szAdminPwd));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_USRWD, strlen(stAppPosParam.szUsrPwd), stAppPosParam.szUsrPwd));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_FUNCPWD, strlen(stAppPosParam.szFuncPwd), stAppPosParam.szFuncPwd));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_TRANSSWITCH, sizeof(stAppPosParam.sTransSwitch), stAppPosParam.sTransSwitch));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_ACQNAME, strlen(stAppPosParam.szAcqName), stAppPosParam.szAcqName));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_ISSUERNAME, strlen(stAppPosParam.szIssuerName), stAppPosParam.szIssuerName));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_CURRENCYNAME, strlen(stAppPosParam.szCurrencyName), stAppPosParam.szCurrencyName));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_PNTPAGECNT, 1, &stAppPosParam.cPrintPageCount));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_PNTDETAIL, 1, &stAppPosParam.cIsPntDetail));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_TOMSOBTAINCMD, 1, &stAppPosParam.cTomsObtainCmd));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_CVV2, 1, &stAppPosParam.cIsNeedCVV2));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_TIPFLAG, 1, &stAppPosParam.cIsTipFlag));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_TIPRATE, strlen(stAppPosParam.szTipRate), stAppPosParam.szTipRate));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_CARDINPUTMODE, 1, &stAppPosParam.cIsCardInput));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_ISSALEVOIDSTRIP, 1, &stAppPosParam.cIsVoidStrip));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_ISVOIDPIN, 1, &stAppPosParam.cIsVoidPin));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_ISSUPPORTRF, 1, &stAppPosParam.cIsSupportRF));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_ISEXRF, 1, &stAppPosParam.cIsExRF));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_ISPINPAD, 1, &stAppPosParam.cIsPinPad));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_PINPADAUXNO, 1, &stAppPosParam.cPinPadAuxNo));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_PINPADCALLBACKFLAG, 1, &stAppPosParam.cPinPadCallbackFlag));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_PINPADTIMEOUT, strlen(stAppPosParam.szPinPadTimeOut), stAppPosParam.szPinPadTimeOut));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_MAINKEYNO, strlen(stAppPosParam.szMainKeyNo), stAppPosParam.szMainKeyNo));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_ENCRYMODE, 1, &stAppPosParam.cEncyptMode));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_PINENCRYMODE, 1, &stAppPosParam.cPinEncyptMode));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_MAXTRANSCNT, strlen(stAppPosParam.szMaxTransCount), stAppPosParam.szMaxTransCount));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_ISPRINTERRREPORT, 1, &stAppPosParam.cIsPrintErrReport));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_SUPPORTCONTACT, 1, &stAppPosParam.cIsSupportContact));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_DEFAULTTRANS, 1, &stAppPosParam.cDefaultTransType));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_ISSHOWTVRTSI, 1, &stAppPosParam.cIsDispEMV_TVRTSI));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_ISSHIELDPAN, 1, &stAppPosParam.cIsShieldPan));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_PNTTITLEMODE, 1, &stAppPosParam.cPntTitleMode));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_PNTTITLEEN, strlen(stAppPosParam.szPntTitleEn), stAppPosParam.szPntTitleEn));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_SHOWAPPNAME, strlen(stAppPosParam.szAppDispname), stAppPosParam.szAppDispname));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_ISADMINPWD, 1, &stAppPosParam.cIsAdminPwd));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_PNTMINUS, 1, &stAppPosParam.cIsPrintMinus));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_ISPREAUTHSHIELDPAN, 1, &stAppPosParam.cIsPreauthShieldPan));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_ISREPNTSETTLE, 1, &stAppPosParam.cIsReprintSettle));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_SUPPOTRSWIPE, 1, &stAppPosParam.cIsSupportSwipe));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_ISPNTISO, 1, &stAppPosParam.cIsPrintIso));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_KEYSYSTEMTYPE, 1, &stAppPosParam.cKeySystemType));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_LANGUAGE, 1, &stAppPosParam.cLanguage));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_FONTSIZE, 1, &stAppPosParam.cFontSize));
	GenerateAppChkValue(szChkValue);
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_APPCHKVALUE, APPCHKLEN + 1, szChkValue));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_ELECSIGN, 1, &stAppPosParam.cElecSignFlag));

	return APP_SUCC;
}

/**
* @brief  save the stTransParam in a file
* @param [in] stTransParam
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int InitTransParamFile(STTRANSPARAM stTransParam)
{
	char *pszFileName = FILE_APPTRANSPARAM;
	char szBuf[4+1] = {0};

	if (PubFsExist(pszFileName) == NAPI_OK)
	{
		return APP_SUCC;
	}

	ASSERT_FAIL(AddTagParam(pszFileName, TAG_TRANS_ISLOGIN, 1, &stTransParam.cIsLogin));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_TRANS_TRACENO, strlen(stTransParam.szTraceNo), stTransParam.szTraceNo));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_TRANS_BATCH, strlen(stTransParam.szBatchNo), stTransParam.szBatchNo));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_TRANS_INVOICENO, strlen(stTransParam.szInvoiceNo), stTransParam.szInvoiceNo));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_TRANS_LOGINDATETIME, 5, stTransParam.sLastLoginDateTime));

	PubIntToC2((uchar *)szBuf, stTransParam.nOnlineUpNum);
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_TRANS_ONLINEUPNUM, 2, szBuf));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_TRANS_QPSLIMIT, 6, stTransParam.szQPSLimit));
	return APP_SUCC;
}

/**
* @brief  save the stReversalParam in a file
* @param [in] stTransParam
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int InitReversalFile(STREVERSALPARAM stReversalParam)
{
	char *pszFileName = FILE_APPREVERSAL;
	STREVERSAL stReversal;

	if (PubFsExist(pszFileName) == NAPI_OK)
	{
		return APP_SUCC;
	}
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_REVERSE_FLAG, 1, &stReversalParam.cIsReversal));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_REVERSE_HADSENDCNT, 1, (char *)&stReversalParam.nHaveReversalNum));
	memcpy((char *)&stReversal, (char *)&stReversalParam.stReversal, sizeof(STREVERSAL));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_REVERSE_TRANSTYPE, 1, &stReversal.cTransType));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_REVERSE_TRANSATTR, 1, &stReversal.cTransAttr));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_REVERSE_PAN, sizeof(stReversal.szPan)-1, stReversal.szPan));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_REVERSE_PROCESSCODE, sizeof(stReversal.szProcCode)-1, stReversal.szProcCode));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_REVERSE_AMT, sizeof(stReversal.szAmount)-1, stReversal.szAmount));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_REVERSE_TRACE, sizeof(stReversal.szTrace)-1, stReversal.szTrace));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_REVERSE_EXDATE, sizeof(stReversal.szExpDate)-1, stReversal.szExpDate));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_REVERSE_INPUTMODE, sizeof(stReversal.szInputMode)-1, stReversal.szInputMode));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_REVERSE_CARDSN, sizeof(stReversal.szCardSerialNo)-1, stReversal.szCardSerialNo));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_REVERSE_NII, sizeof(stReversal.szNii)-1, stReversal.szNii));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_REVERSE_SERVERCODE, sizeof(stReversal.szServerCode)-1, stReversal.szServerCode));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_REVERSE_TRACK1, sizeof(stReversal.szTrack1)-1, stReversal.szTrack1));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_REVERSE_TRACK2, sizeof(stReversal.szTrack2)-1, stReversal.szTrack2));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_REVERSE_RESPONSECODE, sizeof(stReversal.szResponse)-1, stReversal.szResponse));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_REVERSE_INVOICE, sizeof(stReversal.szInvoice)-1, stReversal.szInvoice));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_REVERSE_CVV2, sizeof(stReversal.szCVV2)-1, stReversal.szCVV2));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_REVERSE_ADD1, stReversal.nFieldAdd1Len, stReversal.szFieldAdd1));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_REVERSE_CASHBACKAMT, sizeof(stReversal.szCashbackAmount)-1, stReversal.szCashbackAmount));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_REVERSE_OLDAUTHCODE, sizeof(stReversal.szOldAuthCode)-1, stReversal.szOldAuthCode));

	return APP_SUCC;
}

/**
* @brief  save the stSettleParam in a file
* @param [in] stSettleParam
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int InitSettleFile(STSETTLEPARAM stSettleParam)
{
	STSETTLE stSettle;
	char szLen[4+1] = {0};
	char *pszFileName = FILE_APPSETTLEMENT;

	if (PubFsExist(pszFileName) == NAPI_OK)
	{
		return APP_SUCC;
	}
	PubIntToC2((uchar *)szLen, stSettleParam.nOffLineUnsendNum);
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_SETTLE_OFFLINEUNSENDNUM, 2, szLen));
	PubIntToC2((uchar *)szLen, stSettleParam.nRecNum);
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_SETTLE_RECNUM, 2, szLen));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_SETTLE_HADRESENDNUM, 1, (char *)&stSettleParam.nHaveReSendNum));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_SETTLE_PNTSETTLEHALT, 1, &stSettleParam.cPrintSettleHalt));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_SETTLE_PNTDETAILHALT, 1, &stSettleParam.cPrintDetialHalt));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_SETTLE_TOTALMATCHFLAG, 1, &stSettleParam.cIsTotalMatch));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_SETTLE_BATCHHALT, 1, &stSettleParam.cBatchHaltFlag));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_SETTLE_CLRSETTLE, 1, &stSettleParam.cClrSettleDataFlag));
	PubIntToC2((uchar *)szLen, stSettleParam.nBatchSum);
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_SETTLE_BATCH_SUM, 2, szLen));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_SETTLE_DATETIME, 5, stSettleParam.sSettleDateTime));

	GetSettleData(&stSettle);
	PubIntToC2((uchar *)szLen, stSettle._SaleNum);
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_SETTLE_SALENUM, 2, szLen));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_SETTLE_SALEAMT, sizeof(SETTLE_AMT), (char *)stSettle._SaleAmount));

	PubIntToC2((uchar *)szLen, stSettle._VoidSaleNum);
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_SETTLE_VOIDSALENUM, 2, szLen));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_SETTLE_VOIDSALEAMT, sizeof(SETTLE_AMT), (char *)stSettle._VoidSaleAmount));

	PubIntToC2((uchar *)szLen, stSettle._AuthSaleNum);
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_SETTLE_AUTHSALENUM, 2, szLen));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_SETTLE_AUTHSALEAMT, sizeof(SETTLE_AMT), (char *)stSettle._AuthSaleAmount));

	PubIntToC2((uchar *)szLen, stSettle._VoidAuthSaleNum);
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_SETTLE_VOIDAUTHSALENUM, 2, szLen));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_SETTLE_VOIDAUTHSALEAMT, sizeof(SETTLE_AMT), (char *)stSettle._VoidAuthSaleAmount));

	PubIntToC2((uchar *)szLen, stSettle._PreAuthNum);
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_SETTLE_PREAUTHNUM, 2, szLen));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_SETTLE_PREAUTHAMT, sizeof(SETTLE_AMT), (char *)stSettle._PreAuthAmount));

	PubIntToC2((uchar *)szLen, stSettle._RefundNum);
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_SETTLE_REFUNDNUM, 2, szLen));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_SETTLE_REFUNDAMT, sizeof(SETTLE_AMT), (char *)stSettle._RefundAmount));

	PubIntToC2((uchar *)szLen, stSettle._AdjustNum);
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_SETTLE_ADJUSTNUM, 2, szLen));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_SETTLE_ADJUSTAMT, sizeof(SETTLE_AMT), (char *)stSettle._AdjustAmount));

	PubIntToC2((uchar *)szLen, stSettle._VoidPreAuthNum);
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_SETTLE_VOIDPREAUTHNUM, 2, szLen));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_SETTLE_VOIDPREAUTHAMT, sizeof(SETTLE_AMT), (char *)stSettle._VOidPreAuthAmount));

	PubIntToC2((uchar *)szLen, stSettle._OfflineNum);
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_SETTLE_OFFLINENUM, 2, szLen));
	ASSERT_FAIL(AddTagParam(pszFileName, TAG_SETTLE_OFFLINEAMT, sizeof(SETTLE_AMT), (char *)stSettle._OfflineAmount));

	return APP_SUCC;
}

/**
* @brief  add the tlv(tag+len+value) to the pszTagBuf
* @param [in] the len of the pszTagBuf
* @param [in] nTagId
* @param [in] nLen -- the len of  the pszTagValue
* @param [in] pszValue -- tag value
* @param [out] pszTagBuf
* @param [out] pnTagBufLen the length of the pszTagBuf after adding the tlv
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
static int AddTlvToRecordBuf(char *pszRecordBuf, int *pnRecordBufLen, const int nTagId, const int nLen, const char *pszValue)
{
	char szBuf[MAX_VALUE_LEN+1] = {0};
	int nOff;

	nOff = TlvTransferToStr(nTagId, nLen, pszValue, szBuf);
	if(*pnRecordBufLen + nOff > PER_TRANSRECORD_LEN)
	{
		PubMsgDlg("WARNING", "the record size is too large", 3, 3);
		return APP_QUIT;
	}
    memcpy(pszRecordBuf + *pnRecordBufLen, szBuf, nOff);
	*pnRecordBufLen += nOff;

	return APP_SUCC;
}

/**
* @brief assigned the tag to the pstTransRecord
* @param [in] the len of the pszTagBuf
* @param [in] nTagId
* @param [in] nValueLen -- the len of  the pszTagValue
* @param [in] pszTagValue -- tag value
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
static void TlvToStRecord(int nTagid, int nValueLen, char *pszTagValue, STTRANSRECORD *pstTransRecord)
{
	char szBuf[4+1] = {0};

	switch (nTagid)
	{
	case TAG_RECORD_TRANSTYPE:
		pstTransRecord->cTransType = pszTagValue[0];
		break;
	case TAG_RECORD_TRANSATTR:
		pstTransRecord->cTransAttr = pszTagValue[0];
		break;
	case TAG_RECORD_EMVSTATUS:
		pstTransRecord->cEMV_Status = pszTagValue[0];
		break;
	case TAG_RECORD_PAN:
		memcpy(pstTransRecord->sPan, pszTagValue, nValueLen);
		break;
	case TAG_RECORD_PANLEN:
		pstTransRecord->nPanLen = pszTagValue[0];
		break;
	case TAG_RECORD_AMT:
		memcpy(pstTransRecord->sAmount, pszTagValue, nValueLen);
		break;
	case TAG_RECORD_TRACE:
		memcpy(pstTransRecord->sTrace, pszTagValue, nValueLen);
		break;
	case TAG_RECORD_TIME:
		memcpy(pstTransRecord->sTime, pszTagValue, nValueLen);
		break;
	case TAG_RECORD_DATE:
		memcpy(pstTransRecord->sDate, pszTagValue, nValueLen);
		break;
	case TAG_RECORD_EXPDATE:
		memcpy(pstTransRecord->sExpDate, pszTagValue, nValueLen);
		break;
	case TAG_RECORD_INPUTMODE:
		memcpy(pstTransRecord->szInputMode, pszTagValue, nValueLen);
		break;
	case TAG_RECORD_CARDSN:
		memcpy(pstTransRecord->szCardSerialNo, pszTagValue, nValueLen);
		break;
	case TAG_RECORD_TRACK2:
		memcpy(pstTransRecord->sTrack2, pszTagValue, nValueLen);
		break;
	case TAG_RECORD_TRACK2LEN:
		pstTransRecord->nTrack2Len = pszTagValue[0];
		break;
	case TAG_RECORD_TRACK3:
		memcpy(pstTransRecord->sTrack3, pszTagValue, nValueLen);
		break;
	case TAG_RECORD_TRACK3_LEN:
		pstTransRecord->nTrack3Len = pszTagValue[0];
		break;
	case TAG_RECORD_REFNUM:
		memcpy(pstTransRecord->szRefnum, pszTagValue, nValueLen);
		break;
	case TAG_RECORD_AUTHCODE:
		memcpy(pstTransRecord->szAuthCode, pszTagValue, nValueLen);
		break;
	case TAG_RECORD_RESPONSE:
		memcpy(pstTransRecord->szResponse, pszTagValue, nValueLen);
		break;
	case TAG_RECORD_BATCH:
		memcpy(pstTransRecord->sBatchNum, pszTagValue, nValueLen);
		break;
	case TAG_RECORD_OLDTRACE:
		memcpy(pstTransRecord->sOldTrace, pszTagValue, nValueLen);
		break;
	case TAG_RECORD_OLDAUTHCODE:
		memcpy(pstTransRecord->szOldAuthCode, pszTagValue, nValueLen);
		break;
	case TAG_RECORD_OLDREFNUM:
		memcpy(pstTransRecord->szOldRefnum, pszTagValue, nValueLen);
		break;
	case TAG_RECORD_STATUS:
		pstTransRecord->cStatus = pszTagValue[0];
		break;
	case TAG_RECORD_TIPAMT:
		memcpy(pstTransRecord->sTipAmount, pszTagValue, nValueLen);
		break;
	case TAG_RECORD_BASEAMT:
		memcpy(pstTransRecord->sBaseAmount, pszTagValue, nValueLen);
		break;
	case TAG_RECORD_SENDFLAG:
		pstTransRecord->cSendFlag = pszTagValue[0];
		break;
	case TAG_RECORD_BATCHUPFLAG:
		pstTransRecord->cBatchUpFlag = pszTagValue[0];
		break;
	case TAG_RECORD_TCFLAG:
		pstTransRecord->cTcFlag = pszTagValue[0];
		break;
	case TAG_RECORD_HOLDNAME:
		memcpy(pstTransRecord->szHolderName, pszTagValue, nValueLen);
		break;
	case TAG_RECORD_SERVERCODE:
		memcpy(pstTransRecord->szServerCode, pszTagValue, nValueLen);
		break;
	case TAG_RECORD_FIELD55:
		memcpy(pstTransRecord->sField55, pszTagValue, nValueLen);
		pstTransRecord->nField55Len = nValueLen;
		break;
	case TAG_RECORD_PRINTFLAG:
		pstTransRecord->cPrintFlag = pszTagValue[0];
		break;
	case TAG_RECORD_CVV:
		memcpy(pstTransRecord->szCVV2, pszTagValue, nValueLen);
		break;
	case TAG_RECORD_ADD:
		memcpy(pstTransRecord->sAddition, pszTagValue, nValueLen);
		break;
	case TAG_RECORD_ADDLEN:
		memcpy(szBuf, pszTagValue, nValueLen);
		PubC2ToInt((uint *)&pstTransRecord->nAdditionLen, (uchar *)szBuf);
		break;
	case TAG_RECORD_SIG_PIN:
		pstTransRecord->cPinAndSigFlag = pszTagValue[0];
		break;
	case TAG_RECORD_CASHBACKAMT:
		memcpy(pstTransRecord->sCashbackAmount, pszTagValue, nValueLen);
		break;
	case TAG_RECORD_OLDDATE:
		memcpy(pstTransRecord->szOldDate, pszTagValue, nValueLen);
		break;
	default:
		break;
	}
}

/**
* @brief STTRANSRECORD merge into the pszTagstr
* @param [in] pstTransRecord
* @param [out] pszTagstr
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int StRecordToRecordBuf(const STTRANSRECORD *pstTransRecord, char *pszRecordBuf)
{
	char szBuf[PER_TRANSRECORD_LEN+1] = {0};
	int nLen = 0;
	char szLen[2+1] = {0};

    ASSERT_FAIL(AddTlvToRecordBuf(szBuf, &nLen, TAG_RECORD_TRANSTYPE, 1, &pstTransRecord->cTransType));
	ASSERT_FAIL(AddTlvToRecordBuf(szBuf, &nLen, TAG_RECORD_TRANSATTR, 1, &pstTransRecord->cTransAttr));
	ASSERT_FAIL(AddTlvToRecordBuf(szBuf, &nLen, TAG_RECORD_EMVSTATUS, 1, &pstTransRecord->cEMV_Status));
	ASSERT_FAIL(AddTlvToRecordBuf(szBuf, &nLen, TAG_RECORD_PAN, sizeof(pstTransRecord->sPan), pstTransRecord->sPan));
	ASSERT_FAIL(AddTlvToRecordBuf(szBuf, &nLen, TAG_RECORD_PANLEN, 1, (char *)&pstTransRecord->nPanLen));
	ASSERT_FAIL(AddTlvToRecordBuf(szBuf, &nLen, TAG_RECORD_AMT, sizeof(pstTransRecord->sAmount), pstTransRecord->sAmount));
	ASSERT_FAIL(AddTlvToRecordBuf(szBuf, &nLen, TAG_RECORD_TRACE, sizeof(pstTransRecord->sTrace), pstTransRecord->sTrace));
	ASSERT_FAIL(AddTlvToRecordBuf(szBuf, &nLen, TAG_RECORD_TIME, sizeof(pstTransRecord->sTime), pstTransRecord->sTime));
	ASSERT_FAIL(AddTlvToRecordBuf(szBuf, &nLen, TAG_RECORD_DATE, sizeof(pstTransRecord->sDate), pstTransRecord->sDate));
	ASSERT_FAIL(AddTlvToRecordBuf(szBuf, &nLen, TAG_RECORD_EXPDATE, sizeof(pstTransRecord->sExpDate), pstTransRecord->sExpDate));
	ASSERT_FAIL(AddTlvToRecordBuf(szBuf, &nLen, TAG_RECORD_INPUTMODE, sizeof(pstTransRecord->szInputMode)-1, pstTransRecord->szInputMode));
	ASSERT_FAIL(AddTlvToRecordBuf(szBuf, &nLen, TAG_RECORD_CARDSN, sizeof(pstTransRecord->szCardSerialNo)-1, pstTransRecord->szCardSerialNo));
	ASSERT_FAIL(AddTlvToRecordBuf(szBuf, &nLen, TAG_RECORD_TRACK2, sizeof(pstTransRecord->sTrack2), pstTransRecord->sTrack2));
	ASSERT_FAIL(AddTlvToRecordBuf(szBuf, &nLen, TAG_RECORD_TRACK2LEN, 1, (char *)&pstTransRecord->nTrack2Len));
	ASSERT_FAIL(AddTlvToRecordBuf(szBuf, &nLen, TAG_RECORD_TRACK3, sizeof(pstTransRecord->sTrack3), pstTransRecord->sTrack3));
	ASSERT_FAIL(AddTlvToRecordBuf(szBuf, &nLen, TAG_RECORD_TRACK3_LEN, 1, (char *)&pstTransRecord->nTrack3Len));
	ASSERT_FAIL(AddTlvToRecordBuf(szBuf, &nLen, TAG_RECORD_REFNUM, sizeof(pstTransRecord->szRefnum)-1, pstTransRecord->szRefnum));
	ASSERT_FAIL(AddTlvToRecordBuf(szBuf, &nLen, TAG_RECORD_AUTHCODE, sizeof(pstTransRecord->szAuthCode)-1, pstTransRecord->szAuthCode));
	ASSERT_FAIL(AddTlvToRecordBuf(szBuf, &nLen, TAG_RECORD_RESPONSE, sizeof(pstTransRecord->szResponse)-1, pstTransRecord->szResponse));
	ASSERT_FAIL(AddTlvToRecordBuf(szBuf, &nLen, TAG_RECORD_BATCH, sizeof(pstTransRecord->sBatchNum), pstTransRecord->sBatchNum));
	ASSERT_FAIL(AddTlvToRecordBuf(szBuf, &nLen, TAG_RECORD_OLDTRACE, sizeof(pstTransRecord->sOldTrace), pstTransRecord->sOldTrace));
	ASSERT_FAIL(AddTlvToRecordBuf(szBuf, &nLen, TAG_RECORD_OLDAUTHCODE, sizeof(pstTransRecord->szOldAuthCode)-1, pstTransRecord->szOldAuthCode));
	ASSERT_FAIL(AddTlvToRecordBuf(szBuf, &nLen, TAG_RECORD_OLDREFNUM, sizeof(pstTransRecord->szOldRefnum)-1, pstTransRecord->szOldRefnum));
	ASSERT_FAIL(AddTlvToRecordBuf(szBuf, &nLen, TAG_RECORD_STATUS, 1, &pstTransRecord->cStatus));
	ASSERT_FAIL(AddTlvToRecordBuf(szBuf, &nLen, TAG_RECORD_TIPAMT, sizeof(pstTransRecord->sTipAmount), pstTransRecord->sTipAmount));
	ASSERT_FAIL(AddTlvToRecordBuf(szBuf, &nLen, TAG_RECORD_BASEAMT, sizeof(pstTransRecord->sBaseAmount), pstTransRecord->sBaseAmount));
	ASSERT_FAIL(AddTlvToRecordBuf(szBuf, &nLen, TAG_RECORD_SENDFLAG, 1, &pstTransRecord->cSendFlag));
	ASSERT_FAIL(AddTlvToRecordBuf(szBuf, &nLen, TAG_RECORD_BATCHUPFLAG, 1, &pstTransRecord->cBatchUpFlag));
	ASSERT_FAIL(AddTlvToRecordBuf(szBuf, &nLen, TAG_RECORD_TCFLAG, 1, &pstTransRecord->cTcFlag));
	ASSERT_FAIL(AddTlvToRecordBuf(szBuf, &nLen, TAG_RECORD_HOLDNAME, sizeof(pstTransRecord->szHolderName)-1, pstTransRecord->szHolderName));
	ASSERT_FAIL(AddTlvToRecordBuf(szBuf, &nLen, TAG_RECORD_SERVERCODE, sizeof(pstTransRecord->szServerCode)-1, pstTransRecord->szServerCode));
	ASSERT_FAIL(AddTlvToRecordBuf(szBuf, &nLen, TAG_RECORD_FIELD55, pstTransRecord->nField55Len, pstTransRecord->sField55));
	ASSERT_FAIL(AddTlvToRecordBuf(szBuf, &nLen, TAG_RECORD_PRINTFLAG, 1, &pstTransRecord->cPrintFlag));
	ASSERT_FAIL(AddTlvToRecordBuf(szBuf, &nLen, TAG_RECORD_CVV, sizeof(pstTransRecord->szCVV2)-1, pstTransRecord->szCVV2));
	ASSERT_FAIL(AddTlvToRecordBuf(szBuf, &nLen, TAG_RECORD_ADD, sizeof(pstTransRecord->sAddition), pstTransRecord->sAddition));
	PubIntToC2((uchar *)szLen, pstTransRecord->nAdditionLen);
	ASSERT_FAIL(AddTlvToRecordBuf(szBuf, &nLen, TAG_RECORD_ADDLEN, 2, szLen));
    ASSERT_FAIL(AddTlvToRecordBuf(szBuf, &nLen, TAG_RECORD_SIG_PIN, 1, &pstTransRecord->cPinAndSigFlag));
	ASSERT_FAIL(AddTlvToRecordBuf(szBuf, &nLen, TAG_RECORD_CASHBACKAMT, sizeof(pstTransRecord->sCashbackAmount), pstTransRecord->sCashbackAmount));
	ASSERT_FAIL(AddTlvToRecordBuf(szBuf, &nLen, TAG_RECORD_OLDDATE, sizeof(pstTransRecord->szOldDate)-1, pstTransRecord->szOldDate));

	memcpy(pszRecordBuf + 2, szBuf, nLen);
	nLen += 2; // len
	PubIntToC2((uchar *)pszRecordBuf, nLen);

	return APP_SUCC;
}

/**
* @brief assign tagstr to the pstTransRecord
* @param [in] pszTagstr
* @param [out] pstTransRecord
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int RecordBufToStRecord(char *pszRecordBuf, STTRANSRECORD *pstTransRecord)
{
	int nOff = 0, nLen;
	char szRecordLen[2+1] = {0};
	char szTagid[TLV_TAGLEN+1] = {0};
	char szValueLen[TLV_LENGTHLEN+1] = {0};

	if (pszRecordBuf == NULL || pstTransRecord == NULL)
	{
		return APP_FAIL;
	}
	memcpy(szRecordLen, pszRecordBuf, 2);
	nOff += 2;
	PubC2ToInt((uint *)&nLen, (uchar *)szRecordLen);
	while(1)
	{
		memcpy(szTagid, pszRecordBuf + nOff, TLV_TAGLEN);
		nOff += TLV_TAGLEN;
		memcpy(szValueLen, pszRecordBuf + nOff, TLV_LENGTHLEN);
		nOff += TLV_LENGTHLEN;
		TlvToStRecord(atoi(szTagid), atoi(szValueLen), pszRecordBuf + nOff, pstTransRecord);
		nOff += atoi(szValueLen);
		if (nOff >= nLen)
		{
			break;
		}
	}

	return APP_SUCC;
}

/**
* @brief get the tag(id is nTagid) from the tagstr
* @param [in] pszTagstr
* @param [in] nTagid
* @param [out] pnLen   the length of the tagvalue
* @param [out] pszValue
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int GetRecordTag(char *pszRecordBuf, int nTagid, int *pnLen, char *pszValue)
{
	int nOff = 0, nLen;
	char szTagid[TLV_TAGLEN+1] = {0};
	char szValueLen[TLV_LENGTHLEN+1] = {0};
	char szRecordLen[2+1] = {0};

	if (pszRecordBuf == NULL)
	{
		return APP_FAIL;
	}
	memcpy(szRecordLen, pszRecordBuf, 2);
	nOff += 2;
	PubC2ToInt((uint *)&nLen, (uchar *)szRecordLen);
	while(1)
	{
		memcpy(szTagid, pszRecordBuf + nOff, TLV_TAGLEN);
		nOff += TLV_TAGLEN;
		memcpy(szValueLen, pszRecordBuf + nOff, TLV_LENGTHLEN);
		nOff += TLV_LENGTHLEN;
		if (atoi(szTagid) == nTagid)
		{
			TRACE("tagid = %d", nTagid);
			*pnLen = atoi(szValueLen);
			memcpy(pszValue, pszRecordBuf + nOff, *pnLen);
			return APP_SUCC;
		}
		nOff += atoi(szValueLen);
		if (nOff >= nLen)
		{
			break;
		}
	}

	return APP_FAIL;
}

int InitExPosParam()
{
	int nNum, i;
	int nLen;
	char szBuf[256+1] = {0};
	ulong ulDistance;

	nNum = sizeof(stExPosParam) / sizeof(STTAGINFO);
	for (i = 0; i < nNum; i++)
	{
		if (stExPosParam[i].nLen == DEFAULT_LEN)
		{
			nLen = strlen(stExPosParam[i].pszValue);
		}
		else
		{
			nLen = stExPosParam[i].nLen;
		}
		if (IsTagExist(stExPosParam[i].pszFileName, stExPosParam[i].nTagid, &ulDistance, szBuf) != APP_SUCC)
		{
			ASSERT_FAIL(AddTagParam(stExPosParam[i].pszFileName, stExPosParam[i].nTagid, nLen, stExPosParam[i].pszValue));
		}
	}

	return APP_SUCC;
}

int ExportFileContent(char *pszFileName)
{
	int nRet, nFd;
	uint nFileSize;
	char szBuf[4096] = {0};

	if (PubGetDebugMode() == DEBUG_NONE)
	{
		return APP_SUCC;
	}
	nFd = PubFsOpen(pszFileName, "r");
	if(nFd < 0)
	{
		TRACE("nFd = %d", nFd);
		return APP_FAIL;
	}

	ASSERT_FILE_FAIL(PubFsFileSize(pszFileName, &nFileSize), nFd);
	TRACE(" file name : %s(%d bytes)", pszFileName, nFileSize);
	ASSERT_FILE_FAIL(PubFsSeek(nFd, 0, SEEK_SET), nFd);
	nRet = PubFsRead(nFd, szBuf, nFileSize);
	if (nRet != nFileSize)
	{
		TRACE("nRet = %d size = %d ", nRet, nFileSize);
		TRACE_HEX(szBuf, nFileSize, "file info:");
		PubFsClose(nFd);
		return APP_FAIL;
	}
	PubFsClose(nFd);
	TRACE_HEX(szBuf, nFileSize, "file info:");

	return APP_SUCC;
}


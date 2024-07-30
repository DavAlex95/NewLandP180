/***************************************************************************
** Copyright (c) 2019 Newland Payment Technology Co., Ltd All right reserved   
** File name:  packet.c
** File indentifier: 
** Brief:  Packing module
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
#include "pinpad.h"

const static STTRANSCFG gstTxnCfg[] = 
{
	/* transtype | Title | type of message | processing code | pos condition code | oper cfg | field */
	{TRANS_LOGIN, "SIGN IN", "0800", "920000", "", CFG_NULL, {3,11,12,13,24,41,42}},
	{TRANS_SENDTC, "SEND TC", "0320", "940000", "", CFG_NULL, {2,3,4,11,14,22,23,24,25,35,41,42,48,49,52,55,57,62,64}},
	{TRANS_UPLOAD, "UPLOAD", "0320", "000000", "", CFG_NULL, {2,3,4,11,14,22,23,24,25,35,41,42,48,49,52,55,57,62,64}},
	{TRANS_REVERSAL, "REVERSAL", "0400", "", "", CFG_NULL, {2,3,4,11,14,22,23,24,25,35,38,41,42,48,49,52,54,55,57,62,64}},

	{TRANS_SALE, "SALE", "0200", "000000", "00", CFG_AMOUNT|CFG_CARD|CFG_REVERSAL|CFG_PRINT|CFG_TIPS
		, {2,3,4,11,14,22,23,24,25,35,41,42,48,49,52,54,55,57,62,64}},
	{TRANS_CASHBACK, "CASHBACK", "0200", "090000", "00", CFG_AMOUNT|CFG_CARD|CFG_REVERSAL|CFG_PRINT
		, {2,3,4,11,14,22,23,24,25,35,41,42,48,49,52,54,55,57,62,64}},

	{TRANS_VOID, "VOID", "0200", "020000", "00", CFG_PASSWORD|CFG_REVERSAL|CFG_PRINT|CFG_SEARCH|CFG_CARD
		, {2,3,4,11,12,13,22,23,24,25,35,37,41,42,48,49,52,54,55,57,62,64}},

	{TRANS_REFUND, "REFUND", "0220", "200000", "00", CFG_PASSWORD|CFG_PRINT|CFG_OLDINFO|CFG_AMOUNT|CFG_CARD
		, {2,3,4,11,12,13,14,22,23,24,25,35,37,41,42,48,49,52,55,57,61,62,64}},

	{TRANS_BALANCE, "BALANCE", "0100", "310000", "00", CFG_CARD
		, {2,3,4,11,14,22,23,24,25,35,41,42,48,49,52,55,57,62,64}},

	{TRANS_PREAUTH, "PREAUTH", "0100", "300000", "06", CFG_AMOUNT|CFG_CARD|CFG_REVERSAL|CFG_PRINT
		, {2,3,4,11,22,23,24,25,35,41,42,48,49,52,55,57,62,64}},

	{TRANS_VOID_PREAUTH, "VOID PREAUTH", "0100", "020000", "06", CFG_PASSWORD|CFG_AMOUNT|CFG_CARD|CFG_OLDINFO|CFG_REVERSAL|CFG_PRINT
		, {2,3,4,11,12,13,14,22,23,24,25,35,37,38,41,42,48,49,52,55,57,62,64}},

	{TRANS_AUTHCOMP, "AUTH COMPLETE", "0220", "000000", "06", CFG_OLDINFO|CFG_AMOUNT|CFG_CARD|CFG_REVERSAL|CFG_PRINT
		, {2,3,4,11,12,13,14,22,23,24,25,35,38,39,41,42,48,49,52,55,57,62,64}},

	{TRANS_VOID_AUTHSALE, "VOID AUTH-COMP", "0220", "020000", "06", CFG_PASSWORD|CFG_SEARCH|CFG_REVERSAL|CFG_PRINT
		, {2,3,4,11,12,13,14,22,23,24,25,35,37,38,41,42,48,49,52,55,57,62,64}},

	{TRANS_ADJUST, "ADJUST", "0220", "210000", "00", CFG_SEARCH|CFG_AMOUNT|CFG_CARD|CFG_PRINT|CFG_TIPS
		, {2,3,4,11,14,22,23,24,25,35,41,42,48,49,52,55,57,62,64}},
};

static int TxnCheckAllow(char cTransType, STTRANSRECORD *pstTransRecord, char *pszContent)
{
	char szOldTransName[32] = {0};
	char szTransName[32] = {0};

	GetTransName(pstTransRecord->cTransType, szOldTransName);
	GetTransName(cTransType, szTransName);
	switch (cTransType)
	{
	case TRANS_ADJUST:
		if (pstTransRecord->cTransType != TRANS_SALE) {
			sprintf(pszContent, "not allow %s for %s", szTransName, szOldTransName);
			return APP_FAIL;
		}
		break;
	case TRANS_VOID_AUTHSALE:
		if (pstTransRecord->cTransType != TRANS_AUTHCOMP) {
			sprintf(pszContent, "not allow %s for %s", szTransName, szOldTransName);
			return APP_FAIL;
		}
		break;
	case TRANS_VOID:
		if (pstTransRecord->cTransType != TRANS_SALE && pstTransRecord->cTransType != TRANS_CASHBACK) {
			sprintf(pszContent, "not allow %s for %s", szTransName, szOldTransName);
			return APP_FAIL;
		}
		break;
	default:
		break;
	}

	switch(pstTransRecord->cStatus) {
	case SALECOMPLETED:
		sprintf(pszContent, "%s has been auth complete", szOldTransName);
		break;
	case CANCELED:
		sprintf(pszContent, "%s has been voided", szOldTransName);
		break;
	default:
		break;
	}

	if (pstTransRecord->cStatus != 0) {
		return APP_FAIL;
	}

	if (cTransType == TRANS_VOID) {
		if (pstTransRecord->cEMV_Status == EMV_STATUS_OFFLINE_SUCC) {
			sprintf(pszContent, "not allow %s for [offline] %s", szTransName, szOldTransName);
			return APP_FAIL;
		} else if ((pstTransRecord->cTransAttr == ATTR_CONTACT || pstTransRecord->cTransAttr == ATTR_CONTACTLESS)
			   && pstTransRecord->cEMV_Status != EMV_STATUS_ONLINE_SUCC) {
			sprintf(pszContent, "Online fail, Not allow");
			return APP_FAIL;
		}
	}

	return APP_SUCC;
}

//search record by trace number and output it
static int TxnSearchRecord(char *pszTitle, char cTransType, STTRANSRECORD *pstTransRecord, char *pszTrace)
{
	int nLen;
	int nRet;
	char szContent[100] = {0};
	char szTrace[6+1] = {0}, sTrace[3] = {0};

	/**
	* Find Transaction Record
	*/
	ASSERT_QUIT(PubInputDlg(pszTitle, tr("TRACE NO:"), szTrace, &nLen, 1, 6, 0, INPUT_MODE_NUMBER));
	PubAddSymbolToStr(szTrace, 6, '0', 0);
	PubAscToHex((uchar *)szTrace, 6, 0, (uchar *)sTrace);
	nRet = FindRecordWithTagid(TAG_RECORD_TRACE, sTrace, pstTransRecord);
	if (nRet == APP_FAIL)
	{
		PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent, "|CINVALID TRACE");
		PubMsgDlg(pszTitle, szContent, 1, 5);
		return APP_FAIL;
	}
	else
	{
		if (TxnCheckAllow(cTransType,  pstTransRecord, szContent) != APP_SUCC) {
			PubMsgDlg(pszTitle, szContent, 1, 5);
			return APP_FAIL;
		}
		ASSERT_FAIL(DispRecordInfo(pszTitle, pstTransRecord));
	}
	strcpy(pszTrace, szTrace);

	return APP_SUCC;
}

static int TxnPreprocess(const char cTransType)
{
	/**
	* Check transaction ON-OFF
	*/
	ASSERT_QUIT(ChkTransOnOffStatus(cTransType));

	/**
	* Check if it has login
	*/
	ASSERT_QUIT(ChkLoginStatus());

	if (CheckIsTmkExist() != APP_SUCC)
	{
		PubMsgDlg(NULL, tr("PLEASE LOADING TMK FIRST"), 3, 1);
		return APP_QUIT;
	}

	/**
	* check limit
	*/
	if(cTransType != TRANS_BALANCE)
	{
		ASSERT_QUIT(DealPosLimit());
	}

	TRACE("preprocessing succ");

	return APP_SUCC;
}

int TxnObtainFromRecord(STTRANSRECORD stTransRecord, STSYSTEM *pstSystem)
{
	if (stTransRecord.nPanLen > 0) {
		PubHexToAsc((uchar *)stTransRecord.sPan, stTransRecord.nPanLen, 0, (uchar *)pstSystem->szPan);
	}

	if (stTransRecord.nTrack2Len > 0) {
		PubHexToAsc((uchar *)stTransRecord.sTrack2, stTransRecord.nTrack2Len, 0, (uchar *)pstSystem->szTrack2);
	}

	if (strlen(stTransRecord.szCVV2) > 0)
	{
		memcpy(pstSystem->szCVV2, stTransRecord.szCVV2, strlen(stTransRecord.szCVV2));
	}

	if (strlen(stTransRecord.szCardSerialNo) > 0)
	{
		memcpy(pstSystem->szCardSerialNo, stTransRecord.szCardSerialNo, 3);
	}
	if (strlen(stTransRecord.sExpDate) > 0)
	{
		PubHexToAsc((uchar *)stTransRecord.sExpDate, 4, 0, (uchar *)pstSystem->szExpDate);
	}
	memcpy(pstSystem->szInputMode, stTransRecord.szInputMode, 3);
	PubHexToAsc((uchar *)stTransRecord.sCashbackAmount, 12, 0, (uchar *)pstSystem->szCashbackAmount);
	PubHexToAsc((uchar *)stTransRecord.sAmount, 12, 0, (uchar *)pstSystem->szAmount);
	PubHexToAsc((uchar *)stTransRecord.sDate, 4, 0, (uchar *)pstSystem->szDate);
	PubHexToAsc((uchar *)stTransRecord.sTime, 6, 0, (uchar *)pstSystem->szTime);
	memcpy(pstSystem->szOldRefnum, stTransRecord.szRefnum, strlen(stTransRecord.szRefnum));

	switch(pstSystem->cTransType)
	{
	case TRANS_VOID:
		//memcpy(pstSystem->szRefnum, stTransRecord.szRefnum, 12);
		PubHexToAsc((uchar *)stTransRecord.sTrace, 6, 0, (uchar *)pstSystem->szOldTrace);
		break;
	case TRANS_VOID_PREAUTH:
		memcpy(pstSystem->szOldAuthCode, stTransRecord.szAuthCode, 6);
		break;
	case TRANS_VOID_AUTHSALE:
		PubHexToAsc((uchar *)stTransRecord.sTrace, 6, 0, (uchar *)pstSystem->szOldTrace);
		memcpy(pstSystem->szOldAuthCode, stTransRecord.szAuthCode, 6);
		break;
	case TRANS_ADJUST:
		PubHexToAsc((uchar *)stTransRecord.sTipAmount, 12, 0, (uchar *)pstSystem->szOldTipAmount);
		PubHexToAsc((uchar *)stTransRecord.sBaseAmount, 12, 0, (uchar *)pstSystem->szBaseAmount);
		PubHexToAsc((uchar *)stTransRecord.sTrace, 6, 0, (uchar *)pstSystem->szOldTrace);
		break;
	default:
		break;
	}

	return APP_SUCC;
}

static void TxnShowBalance(STSYSTEM stSystem)
{
	char szDispAmt[DISPAMTLEN] = {0};

	ProAmtToDispOrPnt(stSystem.szAmount, szDispAmt);
	PubClearAll();
	if (YES == GetVarIsPinpad())
	{
		PubDisplayStrInline(DISPLAY_MODE_CENTER, 3, "please check pinpad");
		PubUpdateWindow();
		PubDispPinPad ("Balance:", NULL, NULL, NULL);
		PubDispPinPad (NULL, szDispAmt, NULL, NULL);
		TxnWaitAnyKey(5);
		PubClrPinPad();
	}
	else
	{
		PubDisplayStrInline(1, 2, "Balance:");
		PubDisplayStrInline(1, 3, "%s", szDispAmt);
		PubUpdateWindow();
		PubWaitConfirm(5);
	}
}


int TxnLoadConfig(char cTransType, STTRANSCFG *pstTransCfg)
{
	int nTotalNum, i ;
	char szTransName[32+1];

	memset(szTransName, 0, sizeof(szTransName));
	nTotalNum = sizeof(gstTxnCfg)/sizeof(STTRANSCFG);
	
	TRACE("nNum = [%d]", nTotalNum);
	for(i= 0 ; i < nTotalNum ; i++)
	{
		if(gstTxnCfg[i].cTransType == cTransType)
		{
			break;
		}
	}

	if(i >= nTotalNum)
	{
		TRACE("cTransType =[%02x] was not defined in STTRANSCFG ", cTransType);
		PubMsgDlg("Warn", "Unknow Trans Type", 3, 3);
		return APP_FAIL;
	}
	memcpy(pstTransCfg, &gstTxnCfg[i], sizeof(STTRANSCFG));
	TRACE("load config succ (transtype = %d :%s  cOperFlag = %d)", cTransType, pstTransCfg->szTitle, pstTransCfg->cOperFlag);

	return APP_SUCC;
}

static YESORNO TxnIsNeedCard(char cTransType)
{
	switch(cTransType)
	{
	case TRANS_VOID:
	case TRANS_VOID_AUTHSALE:
		return GetVarIsVoidStrip();
		break;
	default:
		break;
	}

	return YES;
}

static YESORNO TxnIsNeedPin(STSYSTEM stSystem)
{
	if(memcmp(stSystem.szPin, "\x00\x00\x00\x00\x00\x00\x00\x00", 8) != 0)
	{
		return NO;
	}

	//Balance Transaction mandatory input pin (Cusomter can delete it based on their specific requirements)
	if (stSystem.cTransType == TRANS_BALANCE)
	{
		return YES;
	}

	if (GetVarIsPinpad() == YES && stSystem.cTransAttr == ATTR_MANUAL)
	{
		return YES;
	}

	return NO;
}

/**
* @brief Change System structure to Reveral structure
* @param in  STSYSTEM *pstSystem  System structure
* @param out STREVERSAL *pstReversal Reveral structure
* @return void
*/
void TxnSystemToReveral(const STSYSTEM *pstSystem, STREVERSAL *pstReversal)
{
	pstReversal->cTransType = pstSystem->cTransType;
	pstReversal->cTransAttr = pstSystem->cTransAttr;
	//pstReversal->cEMV_Status = pstSystem->cEMV_Status;

	memcpy(pstReversal->szPan, pstSystem->szPan, 19);
	memcpy(pstReversal->szProcCode, pstSystem->szProcCode, 6);
	memcpy(pstReversal->szAmount, pstSystem->szAmount, 12);
	memcpy(pstReversal->szCashbackAmount, pstSystem->szCashbackAmount, 12);
	memcpy(pstReversal->szTrace, pstSystem->szTrace, 6);
	memcpy(pstReversal->szExpDate, pstSystem->szExpDate, 4);
	memcpy(pstReversal->szInputMode, pstSystem->szInputMode, 3);
	memcpy(pstReversal->szCardSerialNo, pstSystem->szCardSerialNo, 3);	
	memcpy(pstReversal->szNii, pstSystem->szNii, 3);
	memcpy(pstReversal->szServerCode, pstSystem->szServerCode, 2);
	memcpy(pstReversal->szOldAuthCode, pstSystem->szOldAuthCode, 6);
	memcpy(pstReversal->szResponse, "98", 2);		/**<no receive defualt 98*/

	if (pstSystem->nAddFieldLen > 0 && pstSystem->nAddFieldLen <= sizeof(pstReversal->szFieldAdd1))
	{
		memcpy(pstReversal->szFieldAdd1, pstSystem->psAddField, pstSystem->nAddFieldLen);
		pstReversal->nFieldAdd1Len = pstSystem->nAddFieldLen;
	}
	else
	{
		pstReversal->nFieldAdd1Len = 0;
	}
	memcpy(pstReversal->szInvoice, pstSystem->szInvoice, 6);
	memcpy(pstReversal->szTrack1, pstSystem->szTrack1, sizeof(pstSystem->szTrack1));
	memcpy(pstReversal->szTrack2, pstSystem->szTrack2, sizeof(pstSystem->szTrack2));
	memcpy(pstReversal->szCVV2, pstSystem->szCVV2, sizeof(pstSystem->szCVV2));

	return ;
}

/**
* @brief Change Reveral structure to System structure
* @param in STREVERSAL *pstReversal Reveral structure
* @param out STSYSTEM *pstSystem System structure
* @return void
*/

void TxnReveralToSystem(const STREVERSAL *pstReversal, STSYSTEM *pstSystem)
{
	pstSystem->cTransType = pstReversal->cTransType;
	pstSystem->cTransAttr = pstReversal->cTransAttr;
	//pstSystem->cEMV_Status = pstReversal->cEMV_Status ;
 
	memcpy(pstSystem->szPan, pstReversal->szPan, 19);
	memcpy(pstSystem->szProcCode, pstReversal->szProcCode, 6);
	memcpy(pstSystem->szAmount, pstReversal->szAmount, 12);
	memcpy(pstSystem->szCashbackAmount, pstReversal->szCashbackAmount, 12);
	memcpy(pstSystem->szTrace, pstReversal->szTrace, 6);
	memcpy(pstSystem->szExpDate, pstReversal->szExpDate, 4);
	memcpy(pstSystem->szInputMode, pstReversal->szInputMode, 3);
	memcpy(pstSystem->szCardSerialNo, pstReversal->szCardSerialNo, 3);
	memcpy(pstSystem->szNii, pstReversal->szNii, 4);
	memcpy(pstSystem->szServerCode, pstReversal->szServerCode, 2);
	memcpy(pstSystem->szOldAuthCode, pstReversal->szOldAuthCode, 6);
	memcpy(pstSystem->szResponse, pstReversal->szResponse, 2);
	memcpy(pstSystem->szInvoice, pstReversal->szInvoice, 6);
	memcpy(pstSystem->szTrack1, pstReversal->szTrack1, sizeof(pstSystem->szTrack1));
	memcpy(pstSystem->szCVV2, pstReversal->szCVV2, sizeof(pstSystem->szCVV2));
	memcpy(pstSystem->szTrack2, pstReversal->szTrack2, sizeof(pstSystem->szTrack2));

	if (NULL != pstSystem->psAddField)
	{
		memcpy(pstSystem->psAddField, pstReversal->szFieldAdd1, pstReversal->nFieldAdd1Len);
		pstSystem->nAddFieldLen = pstReversal->nFieldAdd1Len ;
	}
	else
	{
		pstSystem->nAddFieldLen = 0;
	}

	return ;
}

/**
* @brief Auto Reversal
* @param void
* @return @li APP_SUCC
*		@li APP_FAIL
*		@li APP_QUIT
*/
int TxnReversal(void)
{
	int nRet = 0;
	int nPackLen = 0;
	int nReSend = 0;
	int nConnectFailNum = 0 ;
	char sPackBuf[MAX_PACK_SIZE];
	char cMaxReSend;
	char sAddField1[256] = {0};
	char szContent[100] = {0};
	char szDispBuf[30] = {0};
	char cTransType = TRANS_REVERSAL;
	STSYSTEM stSystem;
	STREVERSAL stReversal;
	STTRANSCFG stTransCfg;

	if (YES != GetVarIsReversal())
	{
		SetVarHaveReversalNum(0);
		return APP_SUCC;
	}
	memset(&stSystem, 0, sizeof(STSYSTEM));
	memset(&stTransCfg, 0, sizeof(STTRANSCFG));
	memset(&stReversal, 0, sizeof(STREVERSAL));
	
	ASSERT_QUIT(TxnLoadConfig(cTransType, &stTransCfg));
	GetVarCommReSendNum((char *)&cMaxReSend);
	GetVarHaveReversalNum(&nReSend);/**<Get times have reversal*/
	GetReversalData(&stReversal);

	nConnectFailNum = 0;
	while(nReSend <= cMaxReSend)
	{
		GetReversalData(&stReversal);
		memset(&stSystem, 0, sizeof(STSYSTEM));
		DealSystem(&stSystem);
		stSystem.psAddField = sAddField1;
		TxnReveralToSystem(&stReversal, &stSystem);
		PubDisplayTitle(tr(stTransCfg.szTitle));
		nRet = CommConnect();
		if (nRet != APP_SUCC)
		{
			nConnectFailNum++;
			if(nConnectFailNum >= cMaxReSend)
			{
				return APP_FAIL;
			}
			else
			{
				continue;
			}
		}
		nConnectFailNum = 0;
		memcpy(stSystem.szMsgID, stTransCfg.szMsgID, 4);
		PackGeneral(&stSystem, NULL, stTransCfg);
		ASSERT_FAIL(Pack(sPackBuf, &nPackLen));
		if (stSystem.cMustChkMAC == 0x01)
		{
			nPackLen -= 8;
			ASSERT_FAIL(AddMac(sPackBuf, &nPackLen, KEY_TYPE_MAC));
		}

		nReSend++;
		SetVarHaveReversalNum(nReSend);
		nRet = CommSendRecv(sPackBuf, nPackLen, sPackBuf, &nPackLen);
		if (nRet != APP_SUCC)
		{
			continue;
		}
		nRet = Unpack(sPackBuf, nPackLen);
		if (nRet != APP_SUCC)
		{
			continue;
		}
		nRet = ChkRespMsgID(stSystem.szMsgID, sPackBuf);
		if (nRet != APP_SUCC)
		{
			continue;
		}
		nRet = ChkRespon(&stSystem, sPackBuf + 2);
		if (nRet != APP_SUCC)
		{
			continue;
		}

		if (stSystem.cMustChkMAC == 0x01)
		{
			nRet = CheckMac(sPackBuf, nPackLen);
			if (nRet != APP_SUCC)
			{
				PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent, "|CMAC FROM HOST IS ERR");
				PubMsgDlg(tr("REVERSAL"), szContent, 3, 1);
				continue;
			}
		}

		if ((memcmp(stSystem.szResponse, "00", 2) == 0))
		{
			SetVarIsReversal(NO);
			SetVarHaveReversalNum(0);
			PubClearAll();
			PubDisplayGen(3, tr("REVERSAL SUCC"));
			PubUpdateWindow();
			TxnWaitAnyKey(1);
			return APP_SUCC;
		}
		else
		{
			DispResp(stSystem.szResponse);
			continue;
		}
	}

	PubGetStrFormat(0, szDispBuf, tr("|CREVERSAL FAIL"));
	PubMsgDlg(NULL, szDispBuf, 0, 3);
	if (YES == GetVarIsPrintErrReport())
	{
		STTRANSRECORD stTransRecord;

		memset(&stSystem, 0, sizeof(STSYSTEM));
		memset(&stTransRecord, 0, sizeof(STTRANSRECORD));
		DealSystem(&stSystem);
		stSystem.psAddField = sAddField1;
		TxnReveralToSystem(&stReversal, &stSystem);
		SysToRecord(&stSystem, &stTransRecord);
		PrintRecord(&stTransRecord, REVERSAL_PRINT);
	}
	SetVarIsReversal(NO);
	SetVarHaveReversalNum(0);

	return APP_SUCC;
}

/**
* @brief Send Offline
* @parm int const char cSendFlag =0 Use when online
* @parm int const char cSendFlag =1 Use when settle
* @return @li APP_SUCC
*		@li APP_FAIL
*		@li APP_QUIT
*/
int TxnSendOffline(const char cSendFlag)
{
	int nRet = 0;
	int nRecNum = 0;
	int nLoop = 0;
	int nFileHandle;
	int nSendNum = 0;
	int nReSend = 0;
	int nOfflineUnSendNum = 0;
	int nCurrentSendNum = 0;	
	char nMaxReSend;
	char *pszTitle = tr("SEND OFFLINE");
	STSYSTEM stSystem;
	STTRANSRECORD stTransRecord;
	STTRANSCFG stTransCfg = {
		0, "","", "", "", CFG_NULL, {2,3,4,11,14,22,23,24,25,35,41,42,48,52,55,57,62,64}
	};

	memset(&stSystem, 0, sizeof(STSYSTEM));
	memset(&stTransRecord, 0, sizeof(STTRANSRECORD));

	nOfflineUnSendNum = GetVarOfflineUnSendNum();
	TRACE("nOfflineUnSendNum=%d cSendFlag=%d", nOfflineUnSendNum, cSendFlag);
	if(nOfflineUnSendNum <= 0)
	{
		TRACE("unsendnum=%d",nOfflineUnSendNum);
		return APP_SUCC;
	}
	GetRecordNum(&nRecNum);
	TRACE("nRecNum=%d",nRecNum);

	if (nRecNum > 0)
	{
		nRet = PubOpenFile(FILE_RECORD, "w", &nFileHandle);
		if (nRet != APP_SUCC)
		{
			TRACE("open file %s error nRet=%d", FILE_RECORD, nRet);
			return APP_FAIL;
		}
	}
	else
	{
		TRACE("nRecNum=%d",nRecNum);
		return APP_SUCC;
	}
	PubClearAll();

	GetVarCommOffReSendNum(&nMaxReSend);
	SetVarHaveReSendNum(nReSend);
	for (; nReSend <= nMaxReSend; nReSend++, SetVarHaveReSendNum(nReSend))
	{
		nCurrentSendNum = 0;
		for (nLoop = 1; nLoop <= nRecNum; nLoop++)
		{
			memset(&stTransRecord, 0, sizeof(STTRANSRECORD));
			nRet = ReadTransRecord(nFileHandle, nLoop, &stTransRecord);
			if (nRet != APP_SUCC)
			{
				TRACE("nRet = %d", nRet);
				continue;
			}
			switch (stTransRecord.cTransType)
			{
			case TRANS_ADJUST:
			case TRANS_OFFLINE:
				if (stTransRecord.cSendFlag > nMaxReSend)
				{
					continue;
				}
				break;
			case TRANS_SALE:
				if (stTransRecord.cSendFlag <= nMaxReSend && stTransRecord.cEMV_Status == EMV_STATUS_OFFLINE_SUCC &&
				        (stTransRecord.cTransAttr == ATTR_CONTACT || stTransRecord.cTransAttr == ATTR_CONTACTLESS))
				{
					;
				}
				else
				{
					continue;
				}
				break;
			default:
				continue;
			}
			nCurrentSendNum++;
			nSendNum ++;
			
			PubDisplayTitle(pszTitle);
			nRet = CommConnect();
			if (nRet != APP_SUCC)
			{
				PubCloseFile(&nFileHandle);
				return APP_QUIT;
			}
			PubClearAll();
			PubDisplayTitle(pszTitle);
			PubDisplayStr(DISPLAY_MODE_CENTER, 3, 1, tr("Processing[%d]..."), nCurrentSendNum);
			PubDisplayGen(4, tr("Please wait..."));
			PubUpdateWindow();
			PubSysMsDelay(200);
			if(PubKbHit() == KEY_ESC)
			{
				PubCloseFile(&nFileHandle);
				return APP_QUIT;
			}
			DealSystem(&stSystem);

			switch (stTransRecord.cTransType)
			{
			case TRANS_OFFLINE:
				RecordToSys(&stTransRecord, &stSystem);
				memcpy(stSystem.szMsgID, "0220", 4);
				memcpy(stSystem.szProcCode, "000000", 6);
				memcpy(stSystem.szServerCode, "00", 2);
				nRet = PackGeneral(&stSystem, &stTransRecord, stTransCfg);
				if (nRet != APP_SUCC)
				{
					PubMsgDlg(pszTitle, tr("PACKED FAIL"), 3, 10);
					PubCloseFile(&nFileHandle);
					return APP_QUIT;
				}
				break;
			default:
				break;
			}
			SetVarHaveReSendNum(nReSend+1);
			PubDisplayTitle(pszTitle);
			nRet = DealPackAndComm(pszTitle,CFG_NOCHECKRESP,&stSystem,NULL,3);
			if(nRet != APP_SUCC)
			{
				goto SENDFAIL;
			}
			PubClearAll();

			if (memcmp(stSystem.szResponse, "00", 2) != 0 && memcmp(stSystem.szResponse, "94", 2) != 0)
			{
				DispResp(stSystem.szResponse);
				stTransRecord.cSendFlag = 0xFE;
				DelVarOfflineUnSendNum();
				goto SENDFAIL;
			}

			PubDisplayGen(3, tr("Send ok"));
			PubUpdateWindow();
			PubSysMsDelay(200);

			stTransRecord.cSendFlag = 0xFD;
			DelVarOfflineUnSendNum();
			memcpy(stTransRecord.szRefnum, stSystem.szRefnum, 12);
SENDFAIL:
			if (stTransRecord.cSendFlag != 0xFE && stTransRecord.cSendFlag != 0xFD)
			{
				if(stTransRecord.cSendFlag >= nMaxReSend)
				{
					stTransRecord.cSendFlag = 0xFF;
					DelVarOfflineUnSendNum();
				}
				else
				{
					stTransRecord.cSendFlag++;
				}
			}
			if(stTransRecord.cSendFlag != 0)
			{
				nRet = UpdateRecByHandle(nFileHandle, nLoop, &stTransRecord);
				if (nRet != APP_SUCC)
				{
					SetVarHaveReSendNum(0);
					PubCloseFile(&nFileHandle);
					return APP_FAIL;
				}
			}
			
			continue;
		}
		if(nSendNum == 0)		
		{
			SetVarHaveReSendNum(0);
			PubCloseFile(&nFileHandle);
			return APP_FAIL;
		}
	}
	SetVarHaveReSendNum(0);
	PubCloseFile(&nFileHandle);
	return APP_SUCC;
}

static YESORNO TxnIsNeedTip(char cTransType, STTRANSCFG *pstTransCfg)
{
	if (pstTransCfg == NULL)
	{
		return NO;
	}

	if(YES == GetVarIsTipFlag() && pstTransCfg->cOperFlag & CFG_TIPS)
	{
		return YES;
	}

	return NO;
}

//Input Old infomation
int TxnGetOldInfo(char *pszTitle, STSYSTEM *pstSystem)
{
	int nLen = 0;

	switch (pstSystem->cTransType)
	{
	case TRANS_REFUND:
		//input Original refer number
		ASSERT_HANGUP_QUIT(PubInputDlg(pszTitle, "Original reference number:", pstSystem->szOldRefnum, &nLen, 0, 12, INPUT_STRING_TIMEOUT, INPUT_MODE_STRING));
		//input Original trans date
		ASSERT_HANGUP_QUIT(PubInputDate(pszTitle, "Original trans date(MMDD):", pstSystem->szOldDate, INPUT_DATE_MODE_MMDD, INPUT_DATE_TIMEOUT));
		break;
	case TRANS_AUTHCOMP:
	case TRANS_VOID_PREAUTH:
		ASSERT_HANGUP_QUIT(PubInputDlg(pszTitle, "Auth Code:", pstSystem->szOldAuthCode, &nLen, 6, 6, INPUT_STRING_TIMEOUT, INPUT_MODE_STRING));
	default:
		break;
	}

	return APP_SUCC;
}

int CheckTip(STSYSTEM *pstSystem)
{
	char szTipRate[2+1];
	char szTipTemp[12+1];
	char szTipMax[12+1];

	memset(szTipRate, 0, sizeof(szTipRate));			
	GetVarTipRate(szTipRate);
	PubAscMulAsc((uchar *)pstSystem->szBaseAmount, (uchar *)szTipRate, (uchar *)szTipTemp);			
	PubAscDivAsc((uchar *)szTipTemp, (uchar *)"100", (uchar *)szTipMax);		
	PubAddSymbolToStr((char *)szTipMax, 12, '0', 0);

	if(strcmp(pstSystem->szTipAmount, szTipMax) > 0)
	{
		PubMsgDlg(NULL, "Tip Amount is too large", 1, 5);
		return APP_FAIL;
	}
	return APP_SUCC;
}

//Input amount
int TxnGetAmout(char *pszTitle, STSYSTEM *pstSystem, STTRANSCFG *pstTransCfg)
{
	char szContent[256] = {0};
	int nAmtLen = 12;
	char szDispAmt[DISPAMTLEN] = {0};
	char cTransType = pstSystem->cTransType;
	
	while(1)
	{
		if (cTransType == TRANS_ADJUST)
		{
			strcpy(szContent, tr("ENTER TIP:"));
			for(;;)
			{
				memset(pstSystem->szTipAmount, 0, sizeof(pstSystem->szTipAmount));
				ASSERT_HANGUP_QUIT(PubInputAmount(pszTitle, szContent, pstSystem->szTipAmount, &nAmtLen, INPUT_AMOUNT_MODE_NONE, INPUT_AMOUNT_TIMEOUT));
				if(CheckTip(pstSystem)==APP_SUCC)
					break;
			}		
			
			PubAscAddAsc((uchar *)pstSystem->szBaseAmount, (uchar *)pstSystem->szTipAmount, (uchar *)pstSystem->szAmount);			
			PubAddSymbolToStr((char *)pstSystem->szAmount, 12, '0', 0);
			/* Check amt if available */			
			if (CheckTransAmount(pstSystem->szTipAmount, pstSystem->cTransType) != APP_SUCC)
			{				
				continue;			
			}
			return APP_SUCC;
		}
		
		strcpy(szContent, tr("ENTER AMOUNT:"));
		memset(pstSystem->szBaseAmount, 0, sizeof(pstSystem->szBaseAmount));
		ASSERT_HANGUP_QUIT(PubInputAmount(pszTitle, szContent, pstSystem->szBaseAmount, &nAmtLen, INPUT_AMOUNT_MODE_NOT_NONE, INPUT_AMOUNT_TIMEOUT));	
		if (cTransType == TRANS_CASHBACK)
		{
			strcpy(szContent, tr("ENTER CASHBACK:"));
			memset(pstSystem->szCashbackAmount, 0, sizeof(pstSystem->szCashbackAmount));
			ASSERT_HANGUP_QUIT(PubInputAmount(pszTitle, szContent, pstSystem->szCashbackAmount, &nAmtLen, INPUT_AMOUNT_MODE_NONE, INPUT_AMOUNT_TIMEOUT));
			PubAscAddAsc((uchar *)pstSystem->szBaseAmount, (uchar *)pstSystem->szCashbackAmount, (uchar *)pstSystem->szAmount);
			PubAddSymbolToStr((char *)pstSystem->szAmount, 12, '0', 0);
		}
		else
		{
			if (cTransType == TRANS_SALE && YES == TxnIsNeedTip(pstSystem->cTransType, pstTransCfg))
			{
				strcpy(szContent, tr("ENTER TIP:"));					
				for(;;)
				{
					memset(pstSystem->szTipAmount, 0, sizeof(pstSystem->szTipAmount));
					ASSERT_HANGUP_QUIT(PubInputAmount(pszTitle, szContent, pstSystem->szTipAmount, &nAmtLen, INPUT_AMOUNT_MODE_NONE, INPUT_AMOUNT_TIMEOUT));
					if(CheckTip(pstSystem)==APP_SUCC)
						break;
				}			
				
				PubAscAddAsc((uchar *)pstSystem->szBaseAmount, (uchar *)pstSystem->szTipAmount, (uchar *)pstSystem->szAmount);		
				PubAddSymbolToStr((char *)pstSystem->szAmount, 12, '0', 0);
			}
			else
			{
				memcpy(pstSystem->szAmount, pstSystem->szBaseAmount, 12);
			}
		}

		/**
		* Check amt if available
		*/
		if (CheckTransAmount(pstSystem->szAmount, pstSystem->cTransType) != APP_SUCC )
		{
			continue;
		}

		/**<Total amt*/
		PubClear2To4();
		ProAmtToDispOrPnt(pstSystem->szAmount, szDispAmt);
		PubAllTrim(szDispAmt);
		PubDisplayStr(DISPLAY_MODE_TAIL, 3, 1, szDispAmt);
		PubDisplayGen(4, tr("CORRECT ? Y/N"));
		if (APP_SUCC == ProConfirm())
		{
			break;
		}
	}

	return APP_SUCC;
}

int TxnCommonEntry(char cTransType, int *pnInputMode)
{
	int nRet, nInputMode = INPUT_NO, nInputPinNum = 0;
	int nOnlineResult = TRUE;
	char szInvno[6+1] = {0};
	char szTitle[32+1] = {0};
	char szSendFiled55[260+1] = {0};

	EM_OPERATEFLAG cOperFlag = CFG_NULL;
	STSYSTEM stSystem, stSystemBak;
	STREVERSAL stReversal;
	STTRANSCFG stTransCfg;
	STTRANSRECORD stTransRecord, stOldTransRecord;

	memset(&stSystem, 0, sizeof(STSYSTEM));
	memset(&stSystemBak, 0, sizeof(STSYSTEM));
	memset(&stReversal, 0, sizeof(STREVERSAL));
	memset(&stTransRecord, 0, sizeof(STTRANSRECORD));
	memset(&stOldTransRecord, 0, sizeof(STTRANSRECORD));

	stSystem.cTransType = cTransType;
	
	/**
	* pre-processing: check ON-OFF, login status, TMK, tranction count limit, battary for print
	*/
	ASSERT_QUIT(TxnPreprocess(cTransType));

	/**
	* Load transaction configuration
	*/
	ASSERT_QUIT(TxnLoadConfig(cTransType, &stTransCfg));

	/**
	* Get necessary data for ISO package
	*/
	DealSystem(&stSystem);
	memcpy(stSystem.szMsgID, stTransCfg.szMsgID, 4);
	memcpy(stSystem.szProcCode, stTransCfg.szProcessCode, 6);
	memcpy(stSystem.szServerCode, stTransCfg.szServiceCode, 2);
	strcpy(szTitle, tr(stTransCfg.szTitle));

	/**
	* verify password, for void/settlement
	*/
	if (stTransCfg.cOperFlag & CFG_PASSWORD)
	{
		ASSERT_QUIT(ProCheckPwd(szTitle, EM_TRANS_PWD));
	}

	PubClearAll();
	PubDisplayTitle(szTitle);

	/**
	* search record by trace No. and display it, used for void/void sale_comp
	*/
	if (stTransCfg.cOperFlag & CFG_SEARCH) 
	{
		ASSERT_FAIL(TxnSearchRecord(szTitle, cTransType, &stOldTransRecord, szInvno));
		TxnObtainFromRecord(stOldTransRecord, &stSystem); // update system data from original record
	}

	/**
	* input Old infomation
	*/
	if (stTransCfg.cOperFlag & CFG_OLDINFO)
	{
		ASSERT_QUIT(TxnGetOldInfo(szTitle, &stSystem));
	}

	/**
	* input Amount
	*/
	if (stTransCfg.cOperFlag & CFG_AMOUNT)
	{
		ASSERT_QUIT(TxnGetAmout(szTitle, &stSystem, &stTransCfg));
	}

	if (stTransCfg.cOperFlag & CFG_REVERSAL)
	{
		cOperFlag |= CFG_REVERSAL;
	}

	if (stTransCfg.cOperFlag & CFG_PRINT)
	{
		stSystem.cPrintFlag = YES;
	}

	/**
	* pre-dial 
	*/
	CommPreDial();

	/**
	* clear emv online pin
	*/
	EmvClrOnlinePin();
	
	/**
	* Perform transactions on the MSR, contact and contactless card interfaces. 
	*/
	if (stTransCfg.cOperFlag & CFG_CARD && TxnIsNeedCard(cTransType) == YES) 	
	{
		if (pnInputMode != NULL)
		{
			nInputMode = *pnInputMode;		//input card from idle menu
		}
		ASSERT_HANGUP_QUIT(PerformTransaction(szTitle, &stSystem, &nInputMode));
	}
	else
	{
		strcpy(stSystem.szInputMode, "01");
	}

	if (stSystem.cTransAttr == ATTR_CONTACT || stSystem.cTransAttr == ATTR_CONTACTLESS)
	{
		memset(szSendFiled55, 0, sizeof(szSendFiled55));
		stSystem.psAddField = szSendFiled55;
		EmvPackField55(cTransType, stSystem.psAddField, &stSystem.nAddFieldLen);
		SaveData9F26RQ();
	}

	/**
	* Get Pin (EMV/contactless/magnetic/manual process PIN in kernel)
	*/
	EmvGetOnlinePin(stSystem.szPin);

	if (TxnIsNeedPin(stSystem) == YES)
	{
		ASSERT_HANGUP_QUIT(GetPin(stSystem.szPan, stSystem.szAmount, stSystem.szPin));
	}

	stSystemBak = stSystem;
regetpin:
	stSystem = stSystemBak;

	if (nInputPinNum > 0)
	{
		ASSERT_HANGUP_QUIT(GetPin(stSystem.szPan, stSystem.szAmount, stSystem.szPin));
	}

	if (memcmp(stSystem.szPin, "\x00\x00\x00\x00\x00\x00\x00\x00", 8) == 0)
	{
		stSystem.szInputMode[2] = '2';
		stSystem.cPinAndSigFlag &= ~CVM_PIN; //NO INPUT PIN
 	}
	else
	{
		stSystem.szInputMode[2] = '1';
		stSystem.cPinAndSigFlag |= CVM_PIN;
	}

	/**
	* Process pending Reveral
	*/
	ASSERT_HANGUP_QUIT(TxnReversal());
	PubDisplayTitle(szTitle);

	/**
	* Connect to host
	*/
	if(APP_SUCC != CommConnect())
	{
		if (stSystem.cTransAttr == ATTR_CONTACT)
		{
			nOnlineResult = 0;
		}
		else
		{
			CommHangUp();
			return APP_FAIL;
		}
	}

	if(nOnlineResult == TRUE)
	{
		/**
		* Set ISO field according to configuration
		*/
		nRet = PackGeneral(&stSystem, &stTransRecord, stTransCfg);
		if (nRet != APP_SUCC)
		{
			CommHangUp();
			DispOutICC(szTitle, tr("PACKET FAIL"), "");
			return APP_FAIL;
		}
		
		nInputPinNum++;
		nRet = DealPackAndComm(szTitle, cOperFlag, &stSystem, &stReversal, nInputPinNum);
		if (nRet != APP_SUCC)
		{
			if(nRet == APP_REPIN)
			{
				PubMsgDlg(szTitle, tr("PASSWORD ERROR.\nPLEASE TRY AGAIN!"), 3, 30);
				goto regetpin;
			}
			CommHangUp();
			return APP_FAIL;
		}
	}

	/**
	* Complete the transaction. 
	*/
	if (stSystem.cTransAttr == ATTR_CONTACT)
	{
		ASSERT_HANGUP_QUIT(CompleteTransaction(szTitle, nOnlineResult, &stSystem, &stReversal, nInputPinNum));
	}

	/**
	* show available balance --- only for balace inquiry
	*/
	if (stSystem.cTransType == TRANS_BALANCE)
	{
		TxnShowBalance(stSystem);
		CommHangUp();
		return APP_SUCC;
	}

	if (stSystem.cTransAttr == ATTR_CONTACT || stSystem.cTransAttr == ATTR_CONTACTLESS)
	{
		stSystem.cEMV_Status = EMV_STATUS_ONLINE_SUCC;
		TxnL3SetData(_EMV_TAG_8A_TM_ARC, (uchar *)stSystem.szResponse, 2);
		EmvSaveRecord(TRUE, &stSystem);
		EmvAddtionRecord(&stTransRecord);
	}

	SysToRecord(&stSystem, &stTransRecord);
	TradeComplete(szTitle, &stSystem, &stTransRecord, szInvno);

	return APP_SUCC;
}

int TxnL3PerformTransaction(char *pszTlvLise, int nTlvLen, L3_TXN_RES *res, STSYSTEM *pstSystem)
{
	int nErrcode;

	if (GetVarIsPinpadReadCard() == YES)
	{
		char szPinPadResCode[2+1] = {0};
		char sz1F8139Tag[3+1] = {0};
		int nMainKeyNo;

		if(GetVarKeySystemType() == KS_DUKPT) // increase KSN
		{
			GetVarMainKeyNo(&nMainKeyNo);
			PubSetCurrentMainKeyIndex(nMainKeyNo);
			if (PubDukptIncreaseKSN() != APP_SUCC)
			{
				TRACE("increase KSN fail");
				return APP_FAIL;
			}
		}

		// pinpad information is synchronized with upper computer.
		sz1F8139Tag[0] = GetPinpadCallBackFlag();
		sz1F8139Tag[1] = 0x01;
		TRACE_HEX(sz1F8139Tag, 3, "tag1F8139");
		TlvAdd(0x1F8139, 3, sz1F8139Tag, pszTlvLise, &nTlvLen);
		if (GetVarPinPadType() == PINPAD_SP100)
		{
			pszTlvLise[0] = L3_CARD_CONTACTLESS;
		}
		// card confirm setting
		TlvAdd(0x1F8130, 1, "\x0B", pszTlvLise, &nTlvLen);
		nErrcode = PinPad_PerformTransaction(pszTlvLise, nTlvLen, res, pstSystem, szPinPadResCode);
		if (memcmp(szPinPadResCode, "00", 2) != 0)
		{
			return APP_QUIT;
		}
	}
	else
	{
		if (PubGetKbAttr() == KB_VIRTUAL) 
		{
			ResetVirtualkbStatus();
		}
		nErrcode = NAPI_L3PerformTransaction(pszTlvLise, nTlvLen, res);
	}

	return nErrcode;
}

void TxnL3TerminateTransaction()
{
	if (GetVarIsPinpadReadCard() == YES)
	{
		PinPad_L3TerminateTransaction();
		while(1)
		{
			if (PubCheckIcc_PINPAD() != APP_SUCC)
			{
				break;
			}
			PubClearAll();
			PubDisplayGen(5, tr("remove card from Pinpad"));
			PubUpdateWindow();
			PubBeep_PINPAD();
		}
		PubClrPinPad_PINPAD();
	}
	else
	{
		NAPI_L3TerminateTransaction();
	}
}

int TxnL3CompleteTransaction(char *pszTlvList, int nTlvLen, L3_TXN_RES *res)
{
	int nErrcode;
	char szPinPadResCode[2+1] = {0};

	if (GetVarIsPinpadReadCard() == YES)
	{
		nErrcode = PinPad_L3CompleteTransaction(pszTlvList[0], pszTlvList + 1, nTlvLen - 1, res, szPinPadResCode);
		if (memcmp(szPinPadResCode, "00", 2) != 0)
		{
			return APP_QUIT;
		}
	}
	else
	{
		nErrcode = NAPI_L3CompleteTransaction(pszTlvList, nTlvLen, res);
	}
	return nErrcode;
}

int TxnL3GetData(unsigned int type, void *data, int maxLen)
{
	if (GetVarIsPinpadReadCard() == YES)
	{
		return PinPad_L3GetData(type, data, maxLen);
	}
	else
	{
		return NAPI_L3GetData(type, data, maxLen);
	}
}

int TxnL3SetData(unsigned int tag, void *data, unsigned int len)
{
	if (GetVarIsPinpadReadCard() == YES)
	{
		return PinPad_L3SetData(tag, data, len);
	}
	else
	{
		return NAPI_L3SetData(tag, data, len);
	}
}

int TxnL3GetTlvData(unsigned int *tagList, unsigned int tagNum, unsigned char *tlvData, unsigned int maxLen,int ctl)
{
	if (GetVarIsPinpadReadCard() == YES)
	{
		return PinPad_L3GetTlvData(tagList, tagNum, tlvData, maxLen, ctl);
	}
	else
	{
		return NAPI_L3GetTlvData(tagList, tagNum, tlvData, maxLen, ctl);
	}
}

int TxnL3SetDebugMode(int debugLV)
{
	if (GetVarIsPinpadReadCard() == YES)
	{
		return PinPad_L3SetDebugMode(debugLV);
	}
	else
	{
		NAPI_L3SetDebugMode(debugLV);
	}
	return APP_SUCC;
}

int TxnL3ModuleInit(char *filePath, char *config)
{
	if (GetVarIsPinpadReadCard() == YES)
	{
		return PinPad_L3init(config, 8);
	}
	else
	{
		return NAPI_L3Init(filePath, config);
	}
}

int TxnL3LoadAIDConfig(L3_CARD_INTERFACE interface, L3_AID_ENTRY *aidEntry, unsigned char tlv_list[], int *tlv_len, L3_CONFIG_OP mode)
{
	if (GetVarIsPinpadReadCard() == YES)
	{
		return PinPad_L3LoadAIDConfig(interface, aidEntry, tlv_list, tlv_len, mode);
	}
	else
	{
		return NAPI_L3LoadAIDConfig(interface, aidEntry, tlv_list, tlv_len, mode);
	}
}

int TxnL3LoadCAPK(L3_CAPK_ENTRY *capk, L3_CONFIG_OP mode)
{
	if (GetVarIsPinpadReadCard() == YES)
	{
		return PinPad_L3LoadCapk(capk, mode);
	}
	else
	{
		return NAPI_L3LoadCAPK(capk, mode);
	}
}

int TxnL3LoadTerminalConfig(L3_CARD_INTERFACE cardinterface, unsigned char tlv_list[], int *tlv_len, L3_CONFIG_OP mode)
{
	if (GetVarIsPinpadReadCard() == YES)
	{
		return PinPad_L3LoadTerminalConfig(cardinterface, tlv_list, tlv_len, mode);
	}
	else
	{
		return NAPI_L3LoadTerminalConfig(cardinterface, tlv_list, tlv_len, mode);
	}
}

int TxnL3EnumEmvConfig(L3_CARD_INTERFACE cardinterface, L3_AID_ENTRY * aidEntry, int maxCount)
{
	if (GetVarIsPinpadReadCard() == YES)
	{
		return PinPad_L3EnumEmvConfig(cardinterface, aidEntry, maxCount);
	}
	else
	{
		return NAPI_L3EnumEmvConfig(cardinterface, aidEntry, maxCount);
	}
}

int TxnL3EnumCapk(int start, int end, char capk[][6])
{
	if (GetVarIsPinpadReadCard() == YES)
	{
		return PinPad_L3EnumCapk(start, end, capk);
	}
	else
	{
		return NAPI_L3EnumCapk(start, end, capk);
	}
}

/**
** brief: init l3 module and load xml config
** param: 
** return: init success - app_succ
*/
int TxnL3Init()
{
	char szCfg[8+1] = {0};
	int nRet;

	SetupCallbackFunc();
	L3_CFG_UNSET(szCfg, L3_CFG_SUPPORT_EC);
	L3_CFG_UNSET(szCfg, L3_CFG_SUPPORT_SM);

	nRet = TxnL3ModuleInit(CONFIG_PATH, szCfg);
	if (nRet != APP_SUCC)
	{
		TRACE("NAPI_L3Init,nRet=%d", nRet);
		return nRet;
	}

	if(PubFsExist(XML_CONFIG) == NAPI_OK && GetIsLoadXMLConfig())
	{
		if(APP_SUCC != LoadXMLConfig())
		{
			PubMsgDlg(NULL, "BAD PARSE XML", 0, 60);
			return APP_FAIL;
		}
	}

	return APP_SUCC;
}

int TxnWaitAnyKey(int nTimeout)
{
	int nRet;

	if (PubGetKbAttr() == KB_VIRTUAL) {
		nRet = PubWaitGetKbPad(nTimeout);
	} else {
		nRet = PubGetKeyCode(nTimeout);
	}

	return nRet;
}


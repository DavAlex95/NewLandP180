/***************************************************************************
** All right reserved:  Desay Copyright (c) 2016-2026  
** File name:  emv l3 dealing
** File indentifier: 
** Synopsis:  
** Current Verion:  v1.0
** Auther: sunh
** Complete date: 2019-8-9
** Modify record: 
** Modify record: 
** Modify date: 
** Version: 
** Modify content: 
***************************************************************************/
#include "apiinc.h"
#include "libapiinc.h"
#include "appinc.h"


static int AddTransType(STSYSTEM *pstSystem, char* pszTlvlist, int* pnTlvLen)
{
    char szTransType[1+1] = {0};

    switch(pstSystem->cTransType)
    {
	case TRANS_CASHBACK:
    case TRANS_SALE: 
    case TRANS_VOID:
    case TRANS_CASH:
    case TRANS_REFUND:
    case TRANS_PREAUTH:
    case TRANS_AUTHCOMP:
    case TRANS_OFFLINE:
    case TRANS_ADJUST:
    case TRANS_BALANCE:
        PubAscToHex((uchar* )pstSystem->szProcCode, 2, 0, (uchar* )szTransType);
        break;
    default:
        szTransType[0] = L3_TRANSACTION_PURCHASE;
        break;
    }
    TlvAdd(0x9C, 1, szTransType, pszTlvlist, pnTlvLen);
    return APP_SUCC;
}


static void GetEntryMode(STSYSTEM *pstSystem)
{
    uchar usInputMode[2+1] = {0};

    TxnL3GetData(L3_DATA_POS_ENTRY_MODE, usInputMode, sizeof(usInputMode)-1);

    switch(usInputMode[0])
    {
    case ATTR_MANUAL:
    case ATTR_MAGSTRIPE:
    case ATTR_CONTACT:
    case ATTR_CONTACTLESS:
    case ATTR_FALLBACK:
        pstSystem->cTransAttr = usInputMode[0];
        PubHexToAsc(usInputMode, 2, 0, (uchar* )pstSystem->szInputMode);
        break;
    default:
        break;
    }
}

static void ObtainIccData(STSYSTEM *pstSystem)
{
    char szTmp[64+1] = {0};

    GetEntryMode(pstSystem);
	if (pstSystem->cTransAttr == ATTR_MANUAL)
	{
		GetCardEventData(pstSystem->szPan);
		GetManualExpiryDate(pstSystem->szExpDate);
		GetManualCVV(pstSystem->szCVV2);
	}
	else
	{
		TxnL3GetData(L3_DATA_PAN, pstSystem->szPan, 19);
	}
	TRACE("pstSystem->szPan = %s", pstSystem->szPan);
    TxnL3GetData(L3_DATA_TRACK2, pstSystem->szTrack2, 37);
    TxnL3GetData(L3_DATA_TRACK3, pstSystem->szTrack3, 104);

    if(TxnL3GetData(L3_DATA_CARDHOLDER_NAME, pstSystem->szHolderName, 26) > 0)
    {
        PubAllTrim(pstSystem->szHolderName);
    }

    if(TxnL3GetData(_EMV_TAG_5F34_IC_PANSN, szTmp, 1) > 0)
    {
		sprintf(pstSystem->szCardSerialNo, "0%02x", szTmp[0]);
	}
	else
	{
		memcpy(pstSystem->szCardSerialNo, "\x00\x00\x00", 3);
	}

    if(TxnL3GetData(L3_DATA_EXPIRE_DATE, szTmp, sizeof(szTmp)) > 0)
    {
		memcpy(pstSystem->szExpDate, szTmp, 4);
    }

	if (TxnL3GetData(L3_DATA_SIGNATURE, szTmp, 1) > 0)
	{
		if(szTmp[0] == 1)
		{
			pstSystem->cPinAndSigFlag |= CVM_SIG;
		}
	}	
}

static void ShowFailInfo(int nErrorCode)
{
    ShowLightFail();
    switch(nErrorCode)
    {
    case L3_ERR_TIMEOUT:
        DispOutICC(NULL, tr("TIMEOUT"), "");
        break;
    case L3_ERR_CANCEL:
        DispOutICC(NULL, tr("CANCEL"), "");
        break;
    case L3_ERR_COLLISION:
        DispOutICC(NULL, tr("MUTIPLE CARD"), "");
        break;
    case L3_ERR_REMOVE_INTERRUPT:
        DispOutICC(NULL, tr("Card Was Removed"), "");
        break;
    default:
        DispOutICC(NULL, tr("TERMINATE"), EmvErrMsg(nErrorCode));
        break;
    }
}

static void EmvDispTvrTsi(void)
{
	char sTVR[5+1];
	char sTSI[2+1];
	int nRet = 0;

	if (YES != GetVarIsDispEMV_TVRTSI())
	{
		return ;
	}

	nRet = TxnL3GetData(_EMV_TAG_95_TM_TVR, sTVR ,sizeof(sTVR));
	if (nRet != 5)
	{
		return ;
	}
	nRet = TxnL3GetData(_EMV_TAG_9B_TM_TSI, sTSI, sizeof(sTSI));
	if (nRet != 2)
	{
		return ;
	}
	NAPI_ScrPush();
	PubClearAll();
	PubDisplayStrs(DISPLAY_ALIGN_BIGFONT, 0, 3, 1, "TVR:%02X%02X%02X%02X%02X",  sTVR[0], sTVR[1],
	               sTVR[2], sTVR[3], sTVR[4]);
	PubDisplayStrs(DISPLAY_ALIGN_BIGFONT, 0, 4, 1, "TSI:%02X%02X",  sTSI[0], sTSI[1]);
	PubUpdateWindow();
	PubWaitConfirm(0);
	NAPI_ScrPop();
	return ;
}

static YESORNO CheckIsPayWave(const STSYSTEM *pstSystem)
{
	char sBuff[32] = {0};

	if (pstSystem->cTransAttr != ATTR_CONTACTLESS) {
		return NO;
	}
	if(TxnL3GetData(_EMV_TAG_9F06_TM_AID, sBuff, sizeof(sBuff)) <= 0) {
		return NO;
	}
	if (memcmp(sBuff, "\xA0\x00\x00\x00\x03", 5) == 0) {
		return YES;
	}

	return NO;
}

int PerformTransaction(char *pszTitle, STSYSTEM *pstSystem, int *pnInputMode, char pzsTransMode)
{
    int nTlvLen = 0, nErrcode = 0;
    char cInputMode = *pnInputMode;
    char szTlvList[512] = {0};
    char szHexAmt[6+1] = {0};
    char szTmpData[8+1] = {0};
    char szDate[14+1] = {0};
	char szServiceCode[3+1] = {0};
    L3_TXN_RES res; 

    if(cInputMode == INPUT_NO)
    {
        if(YES == GetVarIsSwipe())
        {
            cInputMode |= L3_CARD_MAGSTRIPE;
        }	
        if (YES == GetVarIsCardInput())
        {
            cInputMode |= L3_CARD_MANUAL;
        }
		if (YES == GetVarIsSupportContact())
		{
			cInputMode |= L3_CARD_CONTACT;
		}
		if(YES == GetVarIsSupportRF())
        {
            cInputMode |= L3_CARD_CONTACTLESS;
            ShowLightReady();
        }
    }
    szTlvList[0] = cInputMode;
    nTlvLen++;
    PubIntToC4((uchar*)szTlvList+nTlvLen, STRIPE_TIMEOUT);
    nTlvLen += 4;

    TlvAdd(0x1F8121, 1, (char* )pnInputMode, szTlvList, &nTlvLen);
    if(strlen(pstSystem->szAmount) > 0)
    {
		if (pstSystem->cTransType == TRANS_CASHBACK)
		{
			PubAscToHex((uchar*)pstSystem->szBaseAmount, 12, 0, (uchar*)szHexAmt);
			TlvAdd(_EMV_TAG_9F02_TM_AUTHAMNTN, 6, szHexAmt, szTlvList, &nTlvLen);
			PubAscToHex((uchar*)pstSystem->szCashbackAmount, 12, 0, (uchar*)szHexAmt);
			TlvAdd(_EMV_TAG_9F03_TM_OTHERAMNTN, 6, szHexAmt, szTlvList, &nTlvLen);
		}
		else
		{
			PubAscToHex((uchar*)pstSystem->szAmount, 12, 0, (uchar*)szHexAmt);			
			TlvAdd(_EMV_TAG_9F02_TM_AUTHAMNTN, 6, szHexAmt, szTlvList, &nTlvLen);
		}
        SaveInputAMT(pstSystem->szAmount);
    }
    else
    {
        InitInputAMT();
    }

    //To be done 9F03 (Cash back)
    AddTransType(pstSystem, szTlvList, &nTlvLen);

	SetCurTransType(pstSystem->cTransType);

    PubGetCurrentDatetime(szDate);

    PubAscToHex((uchar* )szDate+2, 6, 0, (uchar*)szTmpData);
    TlvAdd(0x9A, 3, szTmpData, szTlvList, &nTlvLen);

	PubAscToHex((uchar* )szDate+8, 6, 0, (uchar*)szTmpData);
    TlvAdd(0x9F21, 3, szTmpData, szTlvList, &nTlvLen);

    //Force Online Authorization
    if(pstSystem->cTransType == TRANS_PREAUTH || pstSystem->cTransType == TRANS_BALANCE)
    {
        szTmpData[0] = 0x01;
        TlvAdd(0x1F8126, 1, szTmpData, szTlvList, &nTlvLen);
    }

    if(YES == GetVarIsSupportRF())
    {
        char szQPSLimit[6+1] = {0};
        GetVarQPSLimit(szQPSLimit);
        TlvAdd(0x1F8124, 6, szQPSLimit, szTlvList, &nTlvLen);
    }

	// control LED: on Led(\x80\x00\x00) off led((\x00\x00\x00))
	// 1F8129 ,byte1 bit8 - led  bit7 - beep when reading card succ with rf
	TlvAdd(0x1F8129, 3, "\xC0\x00\x00", szTlvList, &nTlvLen);
	ShowLightDeal();

	nErrcode = TxnL3PerformTransaction(szTlvList, nTlvLen, &res, pstSystem);
	TRACE("TxnL3PerformTransaction, nErrcode=%d, res=%d", nErrcode, res);
	if (nErrcode == APP_QUIT)
	{
		return APP_QUIT;
	}

	if (res == L3_TXN_DECLINE || res == L3_TXN_APPROVED || res == L3_TXN_ONLINE)
	{
		ObtainIccData(pstSystem);
	}

	PubClearAll();
	EmvDispTvrTsi();
    switch(res)
    {
    case L3_TXN_DECLINE:
        EmvOfflineDenial(pszTitle, pstSystem);
        return APP_FAIL;
        break;
    case L3_TXN_APPROVED:
		// for paywave should not be treated as a "decline" of the refund
		if (pstSystem->cTransType == TRANS_REFUND && CheckIsPayWave(pstSystem) == YES) {
			break;
		}
        EmvOfflineAccept(pszTitle, pstSystem);
        return APP_QUIT;
        break;
    case L3_TXN_ONLINE:
        break;
	case L3_TXN_TRY_ANOTHER:
		DispOutICC(NULL, tr("PLEASE INSERT/SWIPE"), tr("OR TRY ANOTHER CARD"));
		return APP_FAIL;
		break;
    default:
        ShowFailInfo(nErrcode);
        return APP_FAIL;
    }

	if (GetServiceCodeFromTk2(pstSystem->szTrack2, szServiceCode) == APP_SUCC)
	{
		TRACE("szServiceCode = %s", szServiceCode);
		if (memcmp(szServiceCode, SERVICECODE_ATM_103, 3) == 0)
		{
			PubMsgDlg(pszTitle, "This card is not supported(ATM)", 2, 0);
			return APP_FAIL;
		}
	}

    return APP_SUCC;
}

int CompleteTransaction(char* pszTitle, int nOnlineResult, STSYSTEM* pstSystem, STREVERSAL* pstReversal, int nInputPinNum)
{
    int nErrcode;
    int nTlvLen, nFieldLen;
    char cReSuccFlag = FALSE;
    char szTlvList[512] = {0};
    char szRecvFiled55[256] = {0};
    L3_TXN_RES res; 
	char szPrompt[128] = {0};

    szTlvList[0] = (char)nOnlineResult;
    nTlvLen = 1;
    if (nOnlineResult == 1)
    {
        if((memcmp(pstSystem->szResponse, "00", 2) != 0) && (memcmp(pstSystem->szResponse, "01", 2) != 0))
        {
            TlvAdd(0x8A, 2, "05", szTlvList, &nTlvLen);
        }
        else
        {
            TlvAdd(0x8A, 2, pstSystem->szResponse, szTlvList, &nTlvLen);
        }
        nFieldLen = 255;
        GetField(55, szRecvFiled55+2, &nFieldLen);
        PubIntToC2((uchar *)szRecvFiled55, (uint)nFieldLen);
        memcpy(szTlvList+nTlvLen, szRecvFiled55+2, nFieldLen);
        nTlvLen += nFieldLen;
	}

	nErrcode = TxnL3CompleteTransaction(szTlvList, nTlvLen, &res);
	TRACE("NAPI_L3CompleteTransaction, nErrcode=%d, res=%d", nErrcode, res);
	if (nErrcode == APP_QUIT)
	{
		return APP_FAIL;
	}
	if (nErrcode == L3_ERR_REMOVE_INTERRUPT)
	{
		char szContent[64+1] = {0};
		PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent, "|C%s", tr("Card Was Removed"));
		PubMsgDlg(NULL, szContent, 0, 1);
		return APP_FAIL;
	}

    PubClearAll();
	EmvDispTvrTsi();

    if(res != L3_TXN_APPROVED)     //Decline Or Terminate
    {
        if (TRUE == nOnlineResult &&  memcmp(pstSystem->szResponse, "00", 2) == 0)
		{
			cReSuccFlag = TRUE;
		}
		else if (TRUE == nOnlineResult &&  memcmp(pstSystem->szResponse, "00", 2) != 0)
		{
			if (NULL != pstReversal)
			{
				SetVarIsReversal(NO);
				if (memcmp(pstSystem->szResponse, "55", 2) == 0 && (nInputPinNum >= 3))
				{
					PubConfirmDlg(pszTitle, tr("Input Wrong Pwd for 3 times"), 0, 30);
				}
			}
			DispResp(pstSystem->szResponse);
		}

        if (FALSE == nOnlineResult && res == L3_TXN_DECLINE)
		{
			IncVarTraceNo();
			pstSystem->cEMV_Status = EMV_STATUS_OFFLINE_FAIL;
			EmvSaveRecord(FALSE,pstSystem);
		}

		if (res == L3_TXN_DECLINE)
		{
			if (TRUE == cReSuccFlag)
			{
				sprintf(szPrompt, "%s\n%s", tr("DECLINE"), tr("PLEASE Contact issue bank"));
				PubMsgDlg(pszTitle, szPrompt, 1, 5);
			}
			else 
			{
				if (TRUE == nOnlineResult) 
				{
					PubMsgDlg(pszTitle, tr("DECLINE"), 1, 5);
				}
				else
				{
					sprintf(szPrompt, "%s\n%s", tr("DECLINE"), EmvErrMsg(nErrcode));
					PubMsgDlg(pszTitle, szPrompt, 1, 5);
				}
			}
		}
		else
		{
			if (TRUE == cReSuccFlag)
			{
				sprintf(szPrompt, "%s\n%s", tr("TRANS TERMINATE"), tr("PLEASE Contact issue bank"));
				PubMsgDlg(pszTitle, szPrompt, 1, 5);
			}
			else
			{
				if (TRUE == nOnlineResult) 
				{
					PubMsgDlg(pszTitle, tr("TRANS TERMINATE"), 1, 5);
				}
				else
				{
					sprintf(szPrompt, "%s\n%s", tr("TRANS TERMINATE"), EmvErrMsg(nErrcode));
					PubMsgDlg(pszTitle, szPrompt, 1, 5);
				}
			}
		}
        return APP_FAIL;
    }
    else    //Approved
    {
        if(TRUE != nOnlineResult)
        {
            /**
			* Online failure, but ic card approved. 
            * Because of hight risk temporarily close.
            * Can be opened if needed. 
			*/
			sprintf(szPrompt, "%s\n%s", tr("TRANS TERMINATE"), tr("TERMINAL REJECTION"));
			PubMsgDlg(pszTitle, szPrompt, 1, 5);
		    return APP_FAIL;
        }
    }
    return APP_SUCC;
}



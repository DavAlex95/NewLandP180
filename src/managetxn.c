/***************************************************************************
** Copyright (c) 2019 Newland Payment Technology Co., Ltd All right reserved   
** File name:  managetxn.c
** File indentifier: 
** Brief:  Manage transaction processing module
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
#include "napi_crypto.h"

/**
* define global variable
*/

/**
* internal function statement
*/
static int DealFld57WorkKey(char *psField57, int nFieldLen);

/**
* Interface function implementation
*/
/**
* @brief Get Necessary Field
* @param pstSystem 
* @return void
*/
void DealSystem(STSYSTEM *pstSystem)
{
	char szDateTime[14+1];
	
	GetVarTraceNo(pstSystem->szTrace);	
	GetVarCommNii(pstSystem->szNii);
	GetVarTerminalId(pstSystem->szPosID);
	GetVarMerchantId(pstSystem->szShopID);
	GetVarBatchNo(pstSystem->szBatchNum);	
	GetVarInvoiceNo(pstSystem->szInvoice);
	memset(szDateTime, 0, sizeof(szDateTime));
	PubGetCurrentDatetime(szDateTime);
	memcpy(pstSystem->szDate, szDateTime + 4, 4);
	memcpy(pstSystem->szTime, szDateTime + 8, 6);
	return ;
}

/**
** brief: Check Has login
** param [in]: 
** param [out]: 
** return: 
** li: APP_SUCC
** li: APP_FAIL
** auther: 
** date: 2016-9-21
** modify: 
*/
int ChkLoginStatus(void)
{
	if (GetVarKeySystemType() == KS_MSK && YES != GetVarIsLogin())
	{
		PubMsgDlg(NULL, "PLEASE SIGN IN", 3, 1);
		return APP_FAIL;
	}
	return APP_SUCC;
}

/**
** brief: Check Message ID is Match
** param [in]: psReqMsgID request MSGID ASCII
** param [in]: psRespMsgID respond MSGID BCD
** return: 
** li: APP_SUCC
** li: APP_FAIL
** auther: 
** date: 2016-9-21
** modify: 
*/
int ChkRespMsgID(const char *psReqMsgID, const char *psRespMsgID)
{
	uchar szReqMsgId[2+1];
	char *pszTitle = "UNPACK";

	PubAscToHex((uchar *)psReqMsgID, 4, 0, szReqMsgId);
	szReqMsgId[1] |= 0x10;
	if (memcmp(szReqMsgId, psRespMsgID, 2) != 0)
	{
		PubMsgDlg(pszTitle, "MSG ID MISMATCH", 3, 10);
		TRACE_HEX((char *)szReqMsgId,2,"szReqMsgId:");
		TRACE_HEX((char *)psRespMsgID,2,"psRespMsgID:");
		return APP_FAIL;
	}
	return APP_SUCC;
}

/**
* @brief Check some fields received is match to the fields sent 
* @param in pstSystem 
* @param in psBitmap 
* @return @li APP_SUCC 
*		@li APP_FAIL
*/
int ChkRespon(STSYSTEM *pstSystem, const char *psBitmap)
{
	struct
	{
		char szProcCode[6];			/**<F3 */
		char szAmount[12+1];			/**<F4 */
		char szTrace[6+1];			/**<F11 */
		char szServerCode[2+1];		/**<F25 */
		char szPosID[8+1];			/**<F41  */
		char szShopID[15+1];			/**<F42  */
		char szCurrencyCode[3+1];	/**<F49 */
	}stMustChkResp;
	int nFieldLen = 0, nBitmapLen = 8;
	int i, j, n;
	uchar cBitMask;
	char *pszTitle = "UNPACK";

	if (NULL == psBitmap)
	{
		return APP_FAIL;
	}
	memset(&stMustChkResp, 0, sizeof(stMustChkResp));
	if (psBitmap[0] & 0x80)
	{
		nBitmapLen = 16;
	}
	else
	{
		nBitmapLen = 8;
	}

	n = 0 ;
	pstSystem->cMustChkMAC = 0x00;
	for (i = 0; i < nBitmapLen; i++)
	{
		cBitMask = 0x80 ;
		for(j=1; j<9; j++, cBitMask>>=1)
		{
			if ((psBitmap[i] & cBitMask) == 0) 
				continue ;
			n = (i<<3) + j;
			switch(n)
			{
			case 2:
				nFieldLen = 19;
				ASSERT_FAIL(GetField(n, pstSystem->szPan, &nFieldLen));		
				break;
			case 3:
				nFieldLen = 6;
				ASSERT_FAIL(GetField(n, stMustChkResp.szProcCode, &nFieldLen));		
				if (memcmp(stMustChkResp.szProcCode, pstSystem->szProcCode, 5) != 0)
				{
					PubMsgDlg(pszTitle, "PROCESS CODE MISMATCH", 3, 10);
					return APP_FAIL;
				}
				break;
			case 4:
				nFieldLen = 2;
				ASSERT_FAIL(GetField(39, pstSystem->szResponse, &nFieldLen));
				nFieldLen = 12;
				ASSERT_FAIL(GetField(n, stMustChkResp.szAmount, &nFieldLen));
				if(pstSystem->cTransType == TRANS_BALANCE || memcmp(pstSystem->szResponse, "10", 2) == 0)
				{
					memcpy(pstSystem->szAmount, stMustChkResp.szAmount, 12);
					break;
				}
				if ((memcmp(stMustChkResp.szAmount, pstSystem->szAmount, 12) != 0))
				{
					if (AtoLL(stMustChkResp.szAmount) != AtoLL(pstSystem->szAmount))
					{
						PubMsgDlg(pszTitle, "AMOUNT MISMATCH", 3, 10);
						return APP_FAIL;
					}
				}
				break;
			case 11:
				nFieldLen = 6;
				ASSERT_FAIL(GetField(n, stMustChkResp.szTrace, &nFieldLen));
				if (APP_SUCC != PubIsDigitStr(stMustChkResp.szTrace))
				{
					continue;
				}
				if (memcmp(stMustChkResp.szTrace, pstSystem->szTrace, 6) != 0)
				{
					if (AtoLL(stMustChkResp.szTrace) != AtoLL(pstSystem->szTrace))
					{
						PubMsgDlg(pszTitle, "STAN MISMATCH", 3, 10);
						return APP_FAIL;
					}
				}
				break;
			case 12:
				nFieldLen = 6;
				ASSERT_FAIL(GetField(n, pstSystem->szTime, &nFieldLen));
				break;
			case 13:
				nFieldLen = 4;
				ASSERT_FAIL(GetField(n, pstSystem->szDate, &nFieldLen));
				break;
			case 14:
				nFieldLen = 4;
				ASSERT_FAIL(GetField(n, pstSystem->szExpDate, &nFieldLen));
				break;
			case 15:
				nFieldLen = 4;
				ASSERT_FAIL(GetField(n, pstSystem->szSettleDate, &nFieldLen));
				break;
			case 25:
				nFieldLen = 2;
				ASSERT_FAIL(GetField(n, stMustChkResp.szServerCode, &nFieldLen));
				if (memcmp(stMustChkResp.szServerCode, pstSystem->szServerCode, 2) != 0)
				{
					PubMsgDlg(pszTitle, "CONDITION CODE MISMATCH", 3, 10);
					return APP_FAIL;
				}
				break;
			case 37:
				nFieldLen = 12;
				ASSERT_FAIL(GetField(n, pstSystem->szRefnum, &nFieldLen));
				break;
			case 38:
				nFieldLen = 6;
				ASSERT_FAIL(GetField(n, pstSystem->szAuthCode, &nFieldLen));
				break;
			case 39:
				nFieldLen = 2;
				ASSERT_FAIL(GetField(n, pstSystem->szResponse, &nFieldLen));
				break;
			case 41:
				nFieldLen = 8;
				ASSERT_FAIL(GetField(n, stMustChkResp.szPosID, &nFieldLen));
				if (memcmp(stMustChkResp.szPosID, pstSystem->szPosID, 8) != 0)
				{
					PubMsgDlg(pszTitle, "TID MISMATCH", 3, 10);
					return APP_FAIL;
				}
				break;
			case 42:	
				nFieldLen = 15;
				ASSERT_FAIL(GetField(n, stMustChkResp.szShopID, &nFieldLen));
				if (memcmp(stMustChkResp.szShopID, pstSystem->szShopID, 15) != 0)
				{
					TRACE("recv=%s send=%s", stMustChkResp.szShopID,pstSystem->szShopID);
					PubMsgDlg(pszTitle, "SID MISMATCH", 3, 10);
					return APP_FAIL;
				}
				break;
			case 48:
				nFieldLen = 4;
				ASSERT_FAIL(GetField(n, (char *)&(pstSystem->szCVV2), &nFieldLen));
				break;
			case 64:
				pstSystem->cMustChkMAC = 0x01;
				break;
			default:
				break;
			}
		}
	}
	return APP_SUCC;
}

/**
* @brief Login
* @param void
* @return @li APP_SUCC
*		@li APP_FAIL
*		@li APP_QUIT
*/
int Login(void)
{
	STSYSTEM stSystem;
	int nFieldLen = 0;
	char sPackBuf[MAX_PACK_SIZE]; 
	int nPackLen;
	char cTransType = TRANS_LOGIN;
	char szTitle[32] = {0};
	STTRANSCFG stTransCfg;

	memset(&stSystem, 0, sizeof(STSYSTEM));
	memset(&stTransCfg, 0, sizeof(STTRANSCFG));

	if (CheckIsTmkExist() != APP_SUCC)
	{
		PubMsgDlg(NULL, tr("PLEASE LOADING TMK FIRST"), 3, 1);
		return APP_QUIT;
	}

	ASSERT_QUIT(TxnLoadConfig(cTransType, &stTransCfg));
	strcpy(szTitle, tr(stTransCfg.szTitle));
	PubDisplayTitle(szTitle);

	/**
	* Dialing
	*/
	ASSERT_HANGUP_QUIT(CommConnect());
	
	DealSystem(&stSystem);
	memcpy(stSystem.szMsgID, stTransCfg.szMsgID, 4);
	memcpy(stSystem.szProcCode, stTransCfg.szProcessCode, 6);

	/**
	* Begin to pack
	*/
	ClrPack();
	ASSERT_HANGUP_FAIL(PackGeneral(&stSystem, NULL, stTransCfg));
	ASSERT_HANGUP_FAIL(Pack(sPackBuf, &nPackLen));
	
	IncVarTraceNo();

	ASSERT_HANGUP_FAIL(CommSend(sPackBuf, nPackLen));

	ASSERT_HANGUP_FAIL(CommRecv(sPackBuf, &nPackLen));

	CommHangUp();

	ASSERT_FAIL(Unpack(sPackBuf, nPackLen));
	ASSERT_FAIL(ChkRespMsgID(stSystem.szMsgID, sPackBuf));
	ASSERT_FAIL(ChkRespon(&stSystem, sPackBuf + 2));

	PubClearAll();
	if (memcmp(stSystem.szResponse, "00", 2) != 0)
	{
		DispResp(stSystem.szResponse);
		return APP_QUIT;
	}
	PubDisplayTitle(szTitle);
	
	memset(stSystem.szField57, 0, sizeof(stSystem.szField57));
	nFieldLen = 57;
	ASSERT_HANGUP_FAIL(GetField(57, stSystem.szField57, &nFieldLen));
	ASSERT_FAIL(DealFld57WorkKey(stSystem.szField57, nFieldLen));
	
	ASSERT(SetVarBatchNo(stSystem.szBatchNum));

	ASSERT(SetVarIsLogin(YES));
	SetVarLastLoginDateTime(stSystem.szDate);
	PubSetPosDateTime(stSystem.szDate, tr("MMDD"), stSystem.szTime);
	PubMsgDlg(szTitle, tr("SIGN IN OK"), 1, 1);

	return APP_SUCC;
}

/**
* @brief Check the limits of terminal
* @param void
* @return @li APP_SUCC
*		@li APP_FAIL
*		@li APP_QUIT
*/
int ChkPosLimit(void)
{
	int nNowRecordSum = 0;
	int nPosMaxRecordSum;
	char szMsg[48+1];

	GetRecordNum(&nNowRecordSum);
	GetVarMaxTransCount(&nPosMaxRecordSum);

	if (nNowRecordSum >= nPosMaxRecordSum)
	{
		sprintf(szMsg, tr("TXN Log is Up to %d,Please Settle"), nNowRecordSum);
		PubMsgDlg("Warning", szMsg, 3, 30);
		return APP_FAIL;
	}
	return APP_SUCC;
}

//Check whether to Reboot
int CheckAutoReboot()
{
	int nTime = 0;

	TRACE("NAPI_SecGetTimeToReboot...");
	if (NAPI_OK != NAPI_SecGetTimeToReboot((uint *)&nTime))
	{
		return APP_SUCC;
	}

	if (nTime <= 2*60)
	{
		PubClearAll();
		PubDisplayGen(3, tr("Reboot..."));
		PubUpdateWindow();
		NAPI_SysReboot();
	}
	return APP_SUCC;
}

/**
* @brief Precess the limits of terminal
* @param void
* @return @li APP_SUCC
*		@li APP_FAIL
*		@li APP_QUIT
*/
int DealPosLimit(void)
{
	int nNowRecordSum = 0;
	int nPosMaxRecordSum;
	int nRet;
	ulong nSpace;

	GetRecordNum(&nNowRecordSum);
	GetVarMaxTransCount(&nPosMaxRecordSum);
	PubFsGetDiskSpace(1, &nSpace);

	nRet = DealSettleTask();
	if (APP_SUCC != nRet)
	{
		return nRet;
	}

	if (nNowRecordSum >= nPosMaxRecordSum || nSpace <= (10*1024))
	{
		nRet = PubConfirmDlg(tr("Warning"), tr("Translog is out of limit,please settle"), 3, 30);
		if (nRet == APP_SUCC)
		{
			ASSERT_QUIT(DispTotal());
			if (Settle(0) != APP_SUCC)
			{
				PubMsgDlg(tr("Settle Fail"), tr("Please re settle"), 3, 10);
				return APP_FAIL;
			}

			return APP_QUIT;
		}
		else
		{
			return APP_FAIL;
		}
	}

	ASSERT_FAIL(DealPrintLimit());
	ASSERT_FAIL(DealPrinterPaper());
	return APP_SUCC;
}

/**
* @brief Deal Workkey from Field57
* @param in psField57
* @param in nFieldLen
* @return 
* @li APP_SUCC
* @li APP_FAIL
* @author 
* @date 
*/
static int DealFld57WorkKey(char *psField57, int nFieldLen)
{
	char *pszTitle = tr("SIGN IN");
	char sPinKey[16];
	char sMacKey[16];
	char sTrkKey[16];
	char sPinKeyKcv[3];
	char sMacKeyKcv[3];
	char sTrkKeyKcv[3];
	int nMainKeyNo, nRet, nOff = 0;
	
	if (nFieldLen != 38 && nFieldLen != 57)
	{
		PubMsgDlg(pszTitle, tr("Len Of WorkKey Errors"), 3, 1);
		return APP_FAIL;
	}

	GetVarMainKeyNo(&nMainKeyNo);
	PubSetCurrentMainKeyIndex(nMainKeyNo);

	//PINKEY
	memcpy(sPinKey, psField57 + nOff, 16);
	nOff += 16;
	memcpy(sPinKeyKcv, psField57 + nOff, 3);
	nOff += 3;
	nRet = PubLoadWorkKey(KEY_TYPE_PIN, sPinKey, 16, sPinKeyKcv);
	if (nRet != APP_SUCC)
	{
		PubMsgDlg(pszTitle, tr("Loading PinKey Errs"), 3, 10);
		return APP_FAIL;
	}

	//MACKEY
	memcpy(sMacKey, psField57 + nOff, 16);
	nOff += 16;
	memcpy(sMacKeyKcv, psField57 + nOff, 3);
	nOff += 3;
	nRet = PubLoadWorkKey(KEY_TYPE_MAC, sMacKey, 16, sMacKeyKcv);
	if (nRet != APP_SUCC)
	{
		PubMsgDlg(pszTitle, tr("Loading MacKey Errs"), 3, 10);
		return APP_FAIL;
	}

	//DATAKEY
	memcpy(sTrkKey, psField57 + nOff, 16);
	nOff += 16;
	memcpy(sTrkKeyKcv, psField57 + nOff, 3);
	nOff += 3;
	nRet = PubLoadWorkKey(KEY_TYPE_DATA, sTrkKey, 16, sTrkKeyKcv);
	if (nRet != APP_SUCC)
	{
		PubMsgDlg(pszTitle, tr("Loading TrkKey Errs"), 3, 10);
		return APP_FAIL;
	}
	TRACE("load work key ok ");
	return APP_SUCC;
}

/**
* @brief Check the switch of Transaction is Enable
* @param void
* @return @li APP_SUCC support
*		@li APP_FAIL nonsupport
*/
int ChkTransOnOffStatus(char cTransType)
{
	if (YES != GetTransSwitchOnoff(cTransType))
	{
		PubMsgDlg(NULL, tr("NONSUPPORT"), 3, 3);
		return APP_FAIL;
	}
	if (cTransType == TRANS_ADJUST && GetVarIsTipFlag() == NO)
	{
		PubMsgDlg(NULL, tr("NONSUPPORT TIP, PLEASE turn on TIP"), 3, 3);
		return APP_FAIL;	
	}
	return APP_SUCC;
}


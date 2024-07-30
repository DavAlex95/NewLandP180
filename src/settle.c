/***************************************************************************
** Copyright (c) 2019 Newland Payment Technology Co., Ltd All right reserved   
** File name:  settle.c
** File indentifier: 
** Brief:  
** Current Verion:  v1.0
** Auther: sunh
** Complete date: 2016-9-24
** Modify record: 
** Modify date: 
** Version: 
** Modify content: 
***************************************************************************/
#include "apiinc.h"
#include "libapiinc.h"
#include "appinc.h"

#define MAX_AMOUNT "20000000000"
#define ONE_TRADE_MAX_AMOUNT "9999999999"/**<Max amount per transaction*/


/**
* define global variable
*/
static STSETTLE gstSettle;

static int DispSettleDataItem(char *, SETTLE_NUM, SETTLE_AMT);
static int ChangeSettleItem(char, const char *, SETTLE_NUM *, SETTLE_AMT *);
static int BatchTrans(const int, const int, int *);
static int DoBatchUp(void);
static int CheckSettleReponse(STSYSTEM stSystem);

/**
* @brief Settlement
* @param in cFlag 0:Normal settle  nonzero:settle again after interupting settle 
* @return 
* @li APP_SUCC
* @li APP_FAIL
* @li APP_QUIT
*/
int Settle(char cFlag)
{	
	char *pszTitle = tr("SETTLE");
	STSYSTEM stSystem;
	STAMT_NUM_SETTLE stAmtNumSettle;
	STAMT_NUM_INFO_SETTLE stAmtNumInfo;
	char cTailFlag = 0;
	int nRet = 0;
	int nRecordNum;
	char sPackBuf[MAX_PACK_SIZE]; 
	int nPackLen = 0;
	char szSettleDateTime[5+1];
	char cProcStep = 0;
	char szContent[100]={0};
	char sKSN[10+1] = {0};
	int nMainKeyNo = 0;

	memset(&stSystem, 0, sizeof(STSYSTEM));
	memset(szContent, 0, sizeof(szContent));

	if(GetVarKeySystemType() == KS_DUKPT)	//DUKPT
	{
		GetVarMainKeyNo(&nMainKeyNo);
		PubSetCurrentMainKeyIndex(nMainKeyNo);
		PubGetDukptKSN(sKSN);
		TRACE_HEX(sKSN, 10, "sKSN: ");
	}

	GetRecordNum(&nRecordNum);
	if(nRecordNum == 0)
	{
		PubMsgDlg(NULL, "Empty Batch", 1, 5);
		return APP_FAIL;
	}
	
	/*
	* Deal POS limit
	*/
	ASSERT_QUIT(DealPrintLimit()); 
	if (cFlag)
	{
		/*
		* Get halt step
		*/
		if (YES == GetVarBatchHaltFlag())
		{
			cProcStep = 2;
		}
		else if (YES == GetVarPrintSettleHalt())
		{
			cProcStep = 3;
		}
		else if (YES == GetVarPrintDetialHalt())
		{
			cProcStep = 4;
		}
		else if (YES == GetVarClrSettleDataFlag())
		{
			cProcStep = 5;
		}
	}
	else
	{
		cProcStep = 0;
		SetVarBatchMagOfflineHaltFlag(1);	
		SetVarEmvOnlineUpNum(1);
		SetVarBatchSum(0);
	}

	/**
	* Settle online
	*/
	if (cProcStep <= 1)
	{
		/**
		* Check if it has login
		*/
		ASSERT_QUIT(ChkLoginStatus());

		/**
		* Execute Reveral
		*/
		ASSERT_HANGUP_QUIT(TxnReversal());

		/**
		* Send offline
		*/
		ASSERT_HANGUP_QUIT(TxnSendOffline(1));

SETTLE_TAIL:
		PubDisplayTitle(pszTitle);		
		/**
		* Check Connect again
		*/
		ASSERT_HANGUP_QUIT(CommConnect());
		/**
		* Get necessary data
		*/
		DealSystem(&stSystem);
		DealSettle(&stAmtNumInfo, gstSettle);
		memcpy(stSystem.szMsgID, "0500", 4);
		if (1 == cTailFlag)
		{
			memcpy(stSystem.szProcCode, "960000", 6);
		}
		else
		{
			memcpy(stSystem.szProcCode, "920000", 6);
		}

		sprintf(stAmtNumSettle.sDebitNum, "%03d", stAmtNumInfo.nDebitNum);
		PubHexToAsc(stAmtNumInfo.sDebitAmount, 12, 0,(uchar*)stAmtNumSettle.sDebitAmt);
		sprintf(stAmtNumSettle.sCreditNum, "%03d", stAmtNumInfo.nCreditNum);
		PubHexToAsc(stAmtNumInfo.sCreditAmount, 12, 0, (uchar*)stAmtNumSettle.sCreditAmt);

		/**
		* Begin to pack
		*/
		ClrPack();
		ASSERT_HANGUP_FAIL(SetField(0, stSystem.szMsgID, 4));	
		ASSERT_HANGUP_FAIL(SetField(3, stSystem.szProcCode, 6));
		ASSERT_HANGUP_FAIL(SetField(11, stSystem.szTrace, 6));
		ASSERT_HANGUP_FAIL(SetField(24, stSystem.szNii, 3));
		ASSERT_HANGUP_FAIL(SetField(41, stSystem.szPosID, 8));
		ASSERT_HANGUP_FAIL(SetField(42, stSystem.szShopID, 15));
		ASSERT_HANGUP_FAIL(SetField(49, CURRENCY_CODE, 3));
		if(GetVarKeySystemType() == KS_DUKPT) {
			ASSERT_FAIL(SetField(57, sKSN, 10));
		}
		ASSERT_HANGUP_FAIL(SetField(60, stSystem.szBatchNum, 6));
		ASSERT_HANGUP_FAIL(SetField(62, stSystem.szInvoice, 6));	
		ASSERT_HANGUP_FAIL(SetField(63, (char *)&stAmtNumSettle, 33));
		ASSERT_HANGUP_FAIL(SetField(64, "\x00\x00\x00\x00\x00\x00\x00\x00", 8));

		ASSERT_HANGUP_FAIL(Pack(sPackBuf, &nPackLen));
		nPackLen -= 8;
		ASSERT_HANGUP_FAIL(AddMac(sPackBuf,&nPackLen, KEY_TYPE_MAC));

		IncVarTraceNo();
		ASSERT_HANGUP_FAIL(CommSend(sPackBuf, nPackLen));
		ASSERT_HANGUP_FAIL(CommRecv(sPackBuf, &nPackLen));
		CommHangUpSocket();

		/*Check response code*/
		ASSERT_HANGUP_FAIL(Unpack(sPackBuf, nPackLen));
		ASSERT_HANGUP_FAIL(ChkRespMsgID("0500", sPackBuf));
		ASSERT_HANGUP_FAIL(ChkRespon(&stSystem, sPackBuf + 2));
		if (stSystem.cMustChkMAC == 0x01)
		{
			nRet = CheckMac(sPackBuf, nPackLen);
			ASSERT(nRet);
			if (nRet != APP_SUCC)
			{
				CommHangUp();
				PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent, tr("\n|CMAC FROM HOST IS ERROR"));
				PubMsgDlg(tr("SETTLE"), szContent, 3, 10);
				return APP_FAIL;
			}
		}
		ASSERT_HANGUP_FAIL(CheckSettleReponse(stSystem));
		/**
		* Save settle date time
		*/
		PubAscToHex((uchar *)stSystem.szDate, 4, 0, (uchar *)szSettleDateTime);
		PubAscToHex((uchar *)stSystem.szTime, 6, 0, (uchar *)szSettleDateTime + 2);
		SetVarSettleDateTime(szSettleDateTime);
	}

	if (NO == GetVarIsTotalMatch())//total unmatch need to send all the batch
	{
		cTailFlag = 1;
	}
	else
	{
		cTailFlag = 0;
		SetVarBatchHaltFlag(NO);
	}

	/**
	* Send batch
	*/
	if (cProcStep <= 2 && cTailFlag == 1)
	{
		SetVarBatchHaltFlag(YES);
		ASSERT_HANGUP_FAIL(DoBatchUp());
		SetVarBatchHaltFlag(NO);
		goto SETTLE_TAIL;
	}

	CommHangUp();

	/**
	* Print settle
	*/
	if (PubIsSupportPrint() == YES && cProcStep <= 3)
	{
		SetVarPrintSettleHalt(YES);
		PubClearAll();
		PubDisplayTitle(tr("PRINT SETTLE"));
		PubDisplayGen(3, tr("PLEASE WAIT"));
		PubUpdateWindow();
		nRet = PrintSettle(FIRSTPRINT);
		if (nRet == APP_FAIL)
		{
			return APP_FAIL;
		}
 		SetVarPrintSettleHalt(NO);
 	}
	
	/**
	* print detail
	*/
	if (PubIsSupportPrint() == YES && cProcStep <= 4)
	{
		SetVarPrintDetialHalt(YES);
		if (YES == GetVarIsPntDetail())
		{
			if (PubConfirmDlg(pszTitle, tr("PRINT DETAIL?"), 0, 0) == APP_SUCC) {
				PubClear2To4();
				PubDisplayGen(2, tr("PRINT DETAIL..."));
				PubUpdateWindow();
				nRet = PrintAllRecord();
				if (nRet == APP_FAIL)
				{
					return APP_FAIL;
				}
			}
		}

	}

	/*
	* Clear settle data and record
	*/
	if (cProcStep <= 5)
	{
		SetVarClrSettleDataFlag(YES);
		PubClearAll();
		PubDisplayTitle(pszTitle);
		PubDisplayGen(2, tr("PROCESSING..."));
		PubUpdateWindow();
		nRet = InitBatchFile();
		nRet += EmvClearRecord();
		ASSERT(nRet);
		if (nRet != APP_SUCC)
		{
			InitBatchFile();
			EmvClearRecord();
		
		}
		ClearSettle();

		IncVarBatchNo();/**<Increase batch no*/
		SetVarOfflineUnSendNum(0);
		SetVarClrSettleDataFlag(NO);
	}

	PubMsgDlg(pszTitle, tr("SETTLE SUCC"), 1, 1);

	return APP_SUCC;
}

/**
* @brief Get settle data
* @param in pstSettle
* @return 
* @li APP_SUCC
*/
int GetSettleData(STSETTLE *pstSettle)
{
	memcpy((void *)pstSettle , (void *)&gstSettle, sizeof(STSETTLE));
	return APP_SUCC;
}

/**
* @brief Set settle data
* @param in pstSettle
* @return 
* @li APP_SUCC
*/
int SetSettleData(STSETTLE *pstSettle)
{
	memcpy((void *)&gstSettle, (void *)pstSettle, sizeof(STSETTLE));
	return APP_SUCC;
}


/**
* @brief Show settle data
* @param in pszTransName	
* @param in nTransNum	
* @param in nTransAmt		
* @return 
* @li APP_SUCC
*/
static int DispSettleDataItem(char *pszTransName, SETTLE_NUM nTransNum, SETTLE_AMT stTransAmt)
{
	int nRet;
	char szDispBuf[100] = {0};
	char szAmt[13] = {0};
	char szDispAmt[DISPAMTLEN] = {0};
	
	PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szDispBuf, pszTransName);
	PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szDispBuf+strlen(szDispBuf), tr("\nNum:|R%12d"), nTransNum);	
	PubHexToAsc(stTransAmt, 12, 0, (uchar*)szAmt);
	memset(szDispAmt, 0, sizeof(szDispAmt));
	ProAmtToDispOrPnt(szAmt, szDispAmt);
	PubAllTrim(szDispAmt);
	PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szDispBuf+strlen(szDispBuf), tr("\nAmt:|R%s"), szDispAmt);		
	nRet = PubUpDownMsgDlg(tr("Look Up Summary"), szDispBuf, YES, 60, NULL);
	if (nRet==APP_QUIT)
	{
		return APP_QUIT;
	}
	else
	{
		return APP_SUCC;
	}

}

/**
* @brief Precess summary
* @param in STAMT_NUM_INFO_SETTLE *pstInfoSettle 
* @return 
* @li APP_SUCC
*/
int DealSettle(STAMT_NUM_INFO_SETTLE *pstInfoSettle, STSETTLE stSettle)
{
	uchar szAmt[13] = {0};
	uchar szTotalAmt[13] = {0};

//debit
	// sale
	pstInfoSettle->nDebitNum = stSettle._SaleNum;
	PubHexToAsc((uchar *)stSettle._SaleAmount, 12, 0, szAmt);
	AmtAddAmt(szTotalAmt, szAmt, szTotalAmt);

#if 0
	// TIP/ADJUST
	pstInfoSettle->nDebitNum += stSettle._TipsNum;
	PubHexToAsc(stSettle._TipsAmount, 12, 0, szAmt);
	AmtAddAmt(szTotalAmt, szAmt, szTotalAmt);
#endif
	// auth complete
	pstInfoSettle->nDebitNum += stSettle._AuthSaleNum;
	PubHexToAsc(stSettle._AuthSaleAmount, 12, 0, szAmt);
	AmtAddAmt(szTotalAmt, szAmt, szTotalAmt);

	// offline
	pstInfoSettle->nDebitNum += stSettle._OfflineNum;
	PubHexToAsc(stSettle._OfflineAmount, 12, 0, szAmt);
	AmtAddAmt(szTotalAmt, szAmt, szTotalAmt);

	PubAscToHex(szTotalAmt, 12, 0, pstInfoSettle->sDebitAmount);

//credit
	memset(szTotalAmt, 0, sizeof(szTotalAmt));
	//refund
	pstInfoSettle->nCreditNum = stSettle._RefundNum;
	PubHexToAsc(stSettle._RefundAmount, 12, 0, szAmt);
	AmtAddAmt(szTotalAmt, szAmt, szTotalAmt);

	// void
	pstInfoSettle->nCreditNum += stSettle._VoidSaleNum;
	PubHexToAsc(stSettle._VoidSaleAmount, 12, 0, szAmt);
	AmtAddAmt(szTotalAmt, szAmt, szTotalAmt);

	// void auth sale
	pstInfoSettle->nCreditNum += stSettle._VoidAuthSaleNum;
	PubHexToAsc(stSettle._VoidAuthSaleAmount, 12, 0, szAmt);
	AmtAddAmt(szTotalAmt, szAmt, szTotalAmt);

	// void offline
	pstInfoSettle->nCreditNum += stSettle._VoidOfflineNum;
	PubHexToAsc(stSettle._VoidOfflineAmount, 12, 0, szAmt);
	AmtAddAmt(szTotalAmt, szAmt, szTotalAmt);

	PubAscToHex(szTotalAmt, 12, 0, pstInfoSettle->sCreditAmount);

	return APP_SUCC;
}

/**
* @brief Display summary
* @param in void
* @return 
* @li APP_SUCC
*/
int DispTotal()
{
	STAMT_NUM_INFO_SETTLE stInfoSettle;

	memset(&stInfoSettle, 0, sizeof(STAMT_NUM_INFO_SETTLE));
	DealSettle(&stInfoSettle, gstSettle);

	/* SALE*/
	ASSERT_QUIT(DispSettleDataItem(tr("TOTAL SALES:"), stInfoSettle.nDebitNum,  stInfoSettle.sDebitAmount));
	/* REFUND*/
	ASSERT_QUIT(DispSettleDataItem(tr("TOTAL REFUNDS:"), stInfoSettle.nCreditNum, stInfoSettle.sCreditAmount));

	return APP_SUCC;
}

/**
* @brief Look up or print summary
* @param in void
* @return void
*/
int FindSettle()
{
	int nSelect = 1, nStartItem = 1;
	char *pszItems[] = {
		tr("1.Look up"),
		tr("2.Print")
	};

	while(1)
	{
		ASSERT_QUIT(PubShowMenuItems(tr("Look up summary"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelect, &nStartItem,60));
		switch(nSelect)
		{
		case 1:
			DispTotal();
			break;
		case 2:
			if(APP_SUCC==PubConfirmDlg(tr("Summary"), tr("Print?"), 0, 0 )) 
			{
				PubClearAll();
				DISP_PRINTING_NOW;
				PrintTotal();
			}
			break;				
		default:
			break;
		}
	}
}

/**
* @brief Clean settle data
* @param in void
* @return void
*/
void ClearSettle(void)
{
	PubFsDel(FILE_APPSETTLEMENT);
	InitSettleParam();
	SetVarEmvOnlineUpNum(1);
}

/**
* @brief Update settle data
* @param in pszAmount	
* @param out pnTransNum	
* @param out pszAmount	
* @return 
* @li APP_SUCC
* @li APP_FAIL
*/
static int ChangeSettleItem(char cTransType, const char *pszAmount, SETTLE_NUM *pnTransNum, SETTLE_AMT *pstTransAmt)
{
	uchar szAmt1[12+1] = {0};
	uchar szAmt[12+1] = {0};
	uchar szMaxAmt[12+1] = {0};

	(*pnTransNum)++;
	TRACE_HEX(*pstTransAmt, 8, "old settle amt:");
	PubHexToAsc(*pstTransAmt, 12, 0, szAmt1);
	AmtAddAmt(szAmt1, (uchar *)pszAmount, szAmt);
	PubAscToHex(szAmt, 12, 0, *pstTransAmt);
	PubAddSymbolToStr((char *)szAmt, 12, '0', 0);
	strcpy((char *)szMaxAmt, MAX_AMOUNT);
	PubAddSymbolToStr((char *)szMaxAmt, 12, '0', 0);
	ASSERT_FAIL(SaveSettleDataItem(cTransType, pnTransNum, pstTransAmt));
	if(memcmp(szAmt, szMaxAmt, 12) > 0)
	{
		PubMsgDlg(tr("Amount Out Of Limit!"), tr("Please Settle\nPree Any To Go on"), 3, 10);
	}
	
	return APP_SUCC;
}

/**
* @brief Record and Update settle data after doing a transaction
* @param in pszAmount	 
* @param in cTransType	 
* @param in pszInterOrg	 
* @return 
* @li APP_SUCC
*/
int ChangeSettle(const STSYSTEM *pstSystem)
{
	char cTransType;
	STSETTLE *pstSettle;
		
	pstSettle = &gstSettle;
	cTransType = pstSystem->cTransType;
	switch(cTransType)
	{
	case TRANS_SALE:
		ChangeSettleItem(cTransType, pstSystem->szAmount, &(pstSettle->_SaleNum), &(pstSettle->_SaleAmount));
		if (YES == GetVarIsTipFlag() && strlen(pstSystem->szTipAmount)!=0)
		{		
			ChangeSettleItem(TRANS_ADJUST, pstSystem->szTipAmount, &(pstSettle->_TipsNum), &(pstSettle->_TipsAmount));
		}
		break;
	case TRANS_CASHBACK:
		ChangeSettleItem(TRANS_SALE, pstSystem->szAmount, &(pstSettle->_SaleNum), &(pstSettle->_SaleAmount));
		break;	
	case TRANS_VOID:
		ChangeSettleItem(cTransType, pstSystem->szAmount, &(pstSettle->_VoidSaleNum), &(pstSettle->_VoidSaleAmount));
		break;		
	case TRANS_AUTHCOMP:
		ChangeSettleItem(cTransType, pstSystem->szAmount, &(pstSettle->_AuthSaleNum), &(pstSettle->_AuthSaleAmount));
		break;
	case TRANS_VOID_AUTHSALE:
		ChangeSettleItem(cTransType, pstSystem->szAmount, &(pstSettle->_VoidAuthSaleNum), &(pstSettle->_VoidAuthSaleAmount));
        break;
	case TRANS_REFUND:
		ChangeSettleItem(cTransType, pstSystem->szAmount, &(pstSettle->_RefundNum), &(pstSettle->_RefundAmount));
        break;
	case TRANS_OFFLINE:
		ChangeSettleItem(cTransType, pstSystem->szAmount, &(pstSettle->_OfflineNum), &(pstSettle->_OfflineAmount));
        break;
	case TRANS_ADJUST:
		/* Note:the saleAmt include TipAmt */
		if (AtoLL(pstSystem->szOldTipAmount) > 0)
		{
			char szAmt1[12+1] = {0};
			char szAmt[12+1] = {0};

			//Tip Total Amount sub original Tip  Amount then add new Tip Amount
			pstSettle->_TipsNum--;
			PubHexToAsc((uchar *)pstSettle->_TipsAmount, 12, 0, (uchar *)szAmt1);	
			AmtSubAmt((uchar *)szAmt1, (uchar *)pstSystem->szOldTipAmount, (uchar *)szAmt);
			PubAscToHex((uchar *)szAmt, 12, 0, (uchar *)pstSettle->_TipsAmount);

			//sale Total Amount sub original Tip  Amount then add new Tip Amount
			pstSettle->_SaleNum--;
			PubHexToAsc((uchar *)pstSettle->_SaleAmount, 12, 0, (uchar *)szAmt1);	
			AmtSubAmt((uchar *)szAmt1, (uchar *)pstSystem->szOldTipAmount, (uchar *)szAmt);
			PubAscToHex((uchar *)szAmt, 12, 0, (uchar *)pstSettle->_SaleAmount);
		}
		ChangeSettleItem(cTransType, pstSystem->szTipAmount, &(pstSettle->_TipsNum), &(pstSettle->_TipsAmount));
		ChangeSettleItem(TRANS_SALE, pstSystem->szTipAmount, &(pstSettle->_SaleNum), &(pstSettle->_SaleAmount));
        break;
	case TRANS_PREAUTH:	
		ChangeSettleItem(cTransType, pstSystem->szAmount, &(pstSettle->_PreAuthNum), &(pstSettle->_PreAuthAmount));
		break;
	default:
		break;
	}

	return APP_SUCC;
}

/**
* @brief Processing settle task, Check if settle is completed
* @param void
* @return 
* @li APP_SUCC
* @li APP_FAIL
* @li APP_QUIT
*/
int DealSettleTask(void)
{
	if (YES == GetVarBatchHaltFlag() 
		|| YES == GetVarPrintSettleHalt()
		|| YES == GetVarPrintDetialHalt()
		|| YES == GetVarClrSettleDataFlag()
		)
	{
		if(PubConfirmDlg(tr("SETTLE"), tr("Settlement had not commplete,continue?"), 0, 30) != APP_SUCC)
			return APP_FAIL;
		if (APP_SUCC == Settle(1))
		{
			return APP_SUCC;
		}
		return APP_QUIT;
	}
	return APP_SUCC;
}

/**
* @brief Do batch up
* @param void
* @return 
* @li APP_SUCC
* @li APP_FAIL
* @li APP_QUIT
*/
static int DoBatchUp(void)
{	
	int nRet = 0;
	int nRecNum;
	int nFileHandle;
	int nBatchNum = 0;
	char nMaxReSend, nSendNum = 0;
	int nIsFailBatchNum = TRUE;/**if fail number exist*/

	if (YES == GetVarIsTotalMatch())
	{
		return APP_SUCC;
	}

	PubDisplayTitle(tr("UPLOAD"));
	GetRecordNum(&nRecNum);

	if (nRecNum > 0)
	{
		nRet = PubOpenFile(FILE_RECORD, "w", &nFileHandle);
		ASSERT(nRet);
		if (nRet != APP_SUCC)
		{
			return APP_FAIL;
		}
	}
	else
	{
		return APP_SUCC;
	}

	GetVarCommReSendNum(&nMaxReSend);
	for(nSendNum = 0; nSendNum <= nMaxReSend && nIsFailBatchNum; nSendNum++)
	{
		nRet = BatchTrans(nFileHandle, nRecNum, &nBatchNum);
		if (nRet != APP_SUCC)
		{
			PubCloseFile(&nFileHandle);			
			return APP_FAIL;
		}
	}
	ASSERT_FAIL(PubCloseFile(&nFileHandle));	
	GetVarBatchSum(&nBatchNum);

	SetVarBatchMagOfflineHaltFlag(1);
	SetVarEmvOnlineUpNum(1);

	SetVarBatchSum(0);
	return APP_SUCC;
}

/**
* @brief Send batch
* @param in nFileHandle	
* @param in nRecNum	
* @param in pnBatchNum	
* @return 
* @li APP_SUCC
* @li APP_FAIL
*/
static int BatchTrans(const int nFileHandle, const int nRecNum, int *pnBatchNum)
{
	int nRet, nLoop, nPackLen = 0;
	int nFailLoop = 0, nIsFirstFail = TRUE;
	char nMaxReSend;
	char sPackBuf[MAX_PACK_SIZE]; 
	char cTransType = TRANS_UPLOAD;
	char szTitle[32] = {0};
	STSYSTEM stSystem;
	STTRANSCFG stTransCfg;
	STTRANSRECORD stTransRecord;
	
	memset(&stSystem, 0, sizeof(STSYSTEM));
	memset(&stTransCfg, 0, sizeof(STTRANSCFG));
	memset(&stTransRecord, 0, sizeof(STTRANSRECORD));

	
	GetVarCommReSendNum(&nMaxReSend);
	nLoop = 1;

	GetVarBatchMagOfflineHaltFlag(&nLoop);
	if (0 >= nLoop)
	{
		nLoop = 1;
	}

	ASSERT_FAIL(TxnLoadConfig(cTransType, &stTransCfg));
	strcpy(szTitle, tr(stTransCfg.szTitle));
	for (; nLoop <= nRecNum; nLoop++)
	{
		memset(&stTransRecord, 0, sizeof(STTRANSRECORD));
		nRet = ReadTransRecord(nFileHandle, nLoop, &stTransRecord);
		if (nRet != APP_SUCC)
		{
			continue;
		}
		switch (stTransRecord.cTransType)
		{
		case TRANS_PREAUTH:
		case TRANS_VOID:
			continue;
			break;
		default:
			if(stTransRecord.cBatchUpFlag > nMaxReSend)
			{
				continue;
			}
			break;
		}
		sprintf(szTitle, tr("UPLOAD [%d]"), nLoop);
		PubDisplayTitle(szTitle);
		PubClear2To4();
		PubDisplayGen(2, tr("UPLOADING..."));
		PubDisplayGen(3, tr("PLEASE WAIT..."));
		PubUpdateWindow();

		RecordToSys(&stTransRecord, &stSystem);
		
		DealSystem(&stSystem);
		memcpy(stSystem.szMsgID, stTransCfg.szMsgID, 4);
		memcpy(stSystem.szProcCode, stTransCfg.szProcessCode, 6);

		ASSERT_HANGUP_FAIL(PackGeneral(&stSystem, &stTransRecord, stTransCfg));
		ASSERT_HANGUP_FAIL(Pack(sPackBuf, &nPackLen));
		if (stSystem.cMustChkMAC == 0x01)
		{
			nPackLen -= 8;
			ASSERT_HANGUP_FAIL(AddMac(sPackBuf,&nPackLen, KEY_TYPE_MAC));
		}

			
		ASSERT_FAIL(CommConnect());
		IncVarTraceNo();
		nRet = CommSendRecv(sPackBuf, nPackLen, sPackBuf, &nPackLen);
		ASSERT(nRet);
		if (nRet != APP_SUCC)
		{
			goto BATCHUPFAIL;
		}

		nRet = Unpack(sPackBuf, nPackLen);
		ASSERT(nRet);
		if (nRet != APP_SUCC)
		{	
			goto BATCHUPFAIL;
		}
		nRet = ChkRespMsgID(stSystem.szMsgID, sPackBuf);
		ASSERT(nRet);
		if (nRet != APP_SUCC)
		{
			goto BATCHUPFAIL;
		}
		nRet = ChkRespon(&stSystem, sPackBuf+2);
		ASSERT(nRet);
		if (nRet != APP_SUCC)
		{	
			goto BATCHUPFAIL;
		}
		PubClear2To4();
		if (memcmp(stSystem.szResponse, "00", 2) != 0)
		{
			DispResp(stSystem.szResponse);
			stTransRecord.cBatchUpFlag = 0xFE;
			goto BATCHUPFAIL;
		}
		stTransRecord.cBatchUpFlag = 0xFD;
		PubDisplayGen(3, tr("UPLOAD SUCC"));
		PubUpdateWindow();
	BATCHUPFAIL:
		ASSERT_FAIL(DealBatchUpFail(&stTransRecord, nFileHandle, nLoop));
		if(stTransRecord.cBatchUpFlag <= nMaxReSend)
		{
			if(nIsFirstFail)
			{
				nFailLoop = nLoop;
				nIsFirstFail = FALSE;
				SetVarBatchMagOfflineHaltFlag(nFailLoop);
			}
		}
	}
	if(nIsFirstFail)
		nFailLoop = nRecNum+1;
	SetVarBatchMagOfflineHaltFlag(nFailLoop);
	return APP_SUCC;

}

int CheckTransAmount(const char *pszAmount, const char cTransType)
{	
	SETTLE_AMT sTotalAmount_NK;
	SETTLE_AMT sTotalAmount_WK;
	uchar szAmt1_NK[12+1] = {0};
	uchar szAmt1_WK[12+1] = {0};
	uchar szAmt1[12+1] = {0};
	uchar szAmt[12+1] = {0};
	uchar szMaxAmt[12+1] = {0};
	
	switch(cTransType)
	{
	case TRANS_CASHBACK:
	case TRANS_SALE:
		if(AscBigCmpAsc(pszAmount, ONE_TRADE_MAX_AMOUNT) > 0)
		{
			PubMsgDlg(NULL, tr("Amout Out Of Range Per Trans"), 3, 10);
			return APP_FAIL;
		}
		memcpy(&sTotalAmount_NK, &gstSettle._SaleAmount, sizeof(SETTLE_AMT));
		break;
	case TRANS_VOID:
		memcpy(&sTotalAmount_NK, &gstSettle._VoidSaleAmount, sizeof(SETTLE_AMT));
		break;		
	case TRANS_AUTHCOMP:
		if(AscBigCmpAsc(pszAmount, ONE_TRADE_MAX_AMOUNT) > 0)
		{
			PubMsgDlg(NULL, tr("Amout Out Of Range Per Trans"), 3, 10);
			return APP_FAIL;
		}
		memcpy(&sTotalAmount_NK, &gstSettle._AuthSaleAmount, sizeof(SETTLE_AMT));
		break;		
	case TRANS_VOID_AUTHSALE:
		memcpy(&sTotalAmount_NK, &gstSettle._VoidAuthSaleAmount, sizeof(SETTLE_AMT));
		break;
	case TRANS_REFUND:
		if(AscBigCmpAsc(pszAmount, ONE_TRADE_MAX_AMOUNT) > 0)
		{
			PubMsgDlg(NULL, tr("Amout Out Of Range Per Trans"), 3, 10);
			return APP_FAIL;
		}
		memcpy(&sTotalAmount_NK, &gstSettle._RefundAmount, sizeof(SETTLE_AMT));
        break;        	
	case TRANS_OFFLINE:
		memcpy(&sTotalAmount_NK, &gstSettle._OfflineAmount, sizeof(SETTLE_AMT));
		break;	
	case TRANS_ADJUST: 
		memcpy(&sTotalAmount_NK, &gstSettle._AdjustAmount, sizeof(SETTLE_AMT));
		break;
	case TRANS_PREAUTH: 
		if(AscBigCmpAsc(pszAmount, ONE_TRADE_MAX_AMOUNT) > 0)
		{
			PubMsgDlg(NULL, tr("Amout Out Of Range Per Trans"), 3, 10);
			return APP_FAIL;
		}
		memset(&sTotalAmount_NK, 0, sizeof(SETTLE_AMT));
		memset(&sTotalAmount_WK, 0, sizeof(SETTLE_AMT));
		break;
	default:
		if(AscBigCmpAsc(pszAmount, ONE_TRADE_MAX_AMOUNT) > 0)
		{
			PubMsgDlg(NULL, tr("Amout Out Of Range Per Trans"), 3, 10);
			return APP_FAIL;
		}
		break;
	}

	memcpy(&sTotalAmount_NK, &gstSettle._SaleAmount, sizeof(SETTLE_AMT));
	memset(&sTotalAmount_WK, 0, sizeof(SETTLE_AMT));

	memset(szAmt1_NK, 0, sizeof(szAmt1_NK));
	memset(szAmt1_WK, 0, sizeof(szAmt1_WK));
	memset(szAmt1, 0, sizeof(szAmt1));
	PubHexToAsc(sTotalAmount_NK, 12, 0, szAmt1_NK);
	PubHexToAsc(sTotalAmount_WK, 12, 0, szAmt1_WK);
	AmtAddAmt(szAmt1_NK, szAmt1_WK, szAmt1);
	memset(szAmt, 0, sizeof(szAmt));
	AmtAddAmt(szAmt1, (uchar*)pszAmount, szAmt);
	PubAddSymbolToStr((char *)szAmt, 12, '0', 0);
	strcpy((char *)szMaxAmt, MAX_AMOUNT);
	PubAddSymbolToStr((char *)szMaxAmt, 12, '0', 0);
	
	if (memcmp(szAmt, szMaxAmt, 12) > 0) 
	{
		PubMsgDlg(NULL, tr("Total Amout Out Of Range"), 3, 10);
		return APP_FAIL;
	}
	else
	{
		return APP_SUCC;
	}
}

int DealBatchUpFail(STTRANSRECORD *pstTransRecord, int nHandle, int nRecNo)
{
	int nRet, nBatchNum; 
	char nMaxReSend;

	GetVarCommReSendNum(&nMaxReSend);
	GetVarBatchSum(&nBatchNum);

	if(pstTransRecord->cBatchUpFlag < nMaxReSend)
	{
		pstTransRecord->cBatchUpFlag++;
	}
	else if(pstTransRecord->cBatchUpFlag != 0xFE && pstTransRecord->cBatchUpFlag != 0xFD)
	{
		pstTransRecord->cBatchUpFlag = 0xFF;
		nBatchNum++;
		SetVarBatchSum(nBatchNum);
	}
	else
	{
		nBatchNum++;
		SetVarBatchSum(nBatchNum);
	}
	
	if(pstTransRecord->cBatchUpFlag != 0)
	{	
		nRet = UpdateRecByHandle(nHandle, nRecNo, pstTransRecord);
		ASSERT(nRet);
		if (nRet != APP_SUCC)
		{	
			return APP_FAIL; 
		}
	}
	return APP_SUCC;
}

static int CheckSettleReponse(STSYSTEM stSystem)
{
	if (memcmp(stSystem.szResponse, "95", 2) == 0)
	{
		DispResp(stSystem.szResponse);
		SetVarIsTotalMatch(NO);
	}			
	else if(memcmp(stSystem.szResponse, "77", 2) == 0)
	{
		SetVarIsTotalMatch(YES);
		DispResp(stSystem.szResponse);
	}
	else if(memcmp(stSystem.szResponse, "00", 2) == 0)
	{
		ASSERT(1);
		SetVarIsTotalMatch(YES);
	}
	else 
	{
		DispResp(stSystem.szResponse);
		CommHangUp();
		return APP_FAIL;
	}
	return APP_SUCC;
}


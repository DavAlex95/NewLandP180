/***************************************************************************
** Copyright (c) 2019 Newland Payment Technology Co., Ltd All right reserved   
** File name:  print.c
** File indentifier: 
** Brief:  Print module
** Current Verion:  v1.0
** Auther: sunh
** Complete date: 2016-9-22
** Modify record: 
** Modify date: 
** Version: 
** Modify content: 
***************************************************************************/

#include "apiinc.h"
#include "libapiinc.h"
#include "appinc.h"

static int _printrecord(void *);
static int PntHex(const char *, const uchar *, const int);
static int  _printemvparam();
static int PrintBillHead();
static int PrintAmt(const STTRANSRECORD *pstTransRecord);
static void PntTraditionOnoff(const char *lszTransFlag);
static void CheckAmtMinus(char cTransType, char *pszAmount);
static int GetPrintAmt(const STTRANSRECORD *pstTransRecord, char *pszDispAmt);

enum PRINT_PARAM_TYPE
{
	PRINT_PARAM_TYPE_MERCHANTINFO=0x01,		/*Merchant Msg*/
	PRINT_PARAM_TYPE_TRANSCCTRL=0x02,		/*Trans control*/
	PRINT_PARAM_TYPE_SYSTEMCTRL=0x04,		/*System control*/
	PRINT_PARAM_TYPE_COMM=0x08,				/*Comm Param*/
	PRINT_PARAM_TYPE_VERSION=0x10,			/*Version Msg*/
	PRINT_PARAM_TYPE_EMV=0x20,				/*EMV param*/
	PRINT_PARAM_TYPE_OTHER=0x40				/*Other*/
};

typedef struct tagPrintPara
{
	const STTRANSRECORD *pstTransRecord;
	int nReprintFlag;
	int nCurrentPage;
} STPRINTPATA;


typedef struct
{
	
	unsigned char usAid[16];                /* AID 4F(ICC), 9F06(Terminal), b, 5-16 bytes */
	unsigned char ucAidLen;                 /* length of AID */
	unsigned char usAppVer[2];              /* 9F09(application version), b, 2 bytes */
	unsigned char usTacDefault[5];          /*default Terminal Action Code*/
	unsigned char usTacDenial[5];           /*denial Terminal Action Code*/
	unsigned char usTacOnline[5];           /*online Terminal Action Code*/
	unsigned char usFloorlimit[4];          /* 9F1B(Floor limit), b, 4 bytes */
	unsigned char usThresholdValue[4];      /*thresold value*/
	unsigned char ucMaxTargetPercent;      	/*max target percent*/
	unsigned char ucTargetPercent;          /*target percent */
	unsigned char ucDefaultDdolLen;        /* the length of the default ddol following the merchant name and location */
	unsigned char ucDefaultTdolLen;        /* the length of the default todl following the default ddol */
	unsigned char usDefaultDdol[252];        /*default ddol */
	unsigned char usDefaultTdol[252-9];        /* default tdol     */
	unsigned char usCap[3];                 /* 9F33(Terminal capability), b,  3 */
	unsigned char usAddCap[5];              /* 9F40(Addtional Terminal capability), b,  5 */
	unsigned char ucAppSelIndicator;  
	unsigned char usIcs[7];
	unsigned char ucLimitExist;    			/* limist exist?(To determine the identity of the following limit exists)
												bit 1    =1    EC limint exist
												bit 2    =1    contactless limit exist
												bit 3    =1    contactless offline limit exist
												bit 4    =1    cvm limit  exist*/
	unsigned char usClLimit[6];             /* contactless terminal transaction limit n12  6bytes */
	unsigned char usClOfflineLimit[6];     	/* contactless terminal offline minimum limit n12  6bytes */
	unsigned char usCvmLimit[6];            /* terminal implement CVM Limit n12  6bytes */
}PRT_AIDPARAM;

/**
* @brief Print receipt
* @param in const STTRANSRECORD *pstTransRecord 
* @param in int nReprintFlag  
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int PrintRecord(const STTRANSRECORD *pstTransRecord, int nReprintFlag)
{
	int nCurrentPage,nPage=1;
	STPRINTPATA stPara;

	if (pstTransRecord->cPrintFlag == NO || PubIsSupportPrint() == NO) {
		PubBeep(1);
		TxnWaitAnyKey(1);
		return APP_SUCC;
	}

	if (PubGetPrinter() == _PRINTTYPE_TP)
	{
		GetVarPrintPageCount(&nPage);
	}

	for(nCurrentPage = 1; nCurrentPage <= nPage; nCurrentPage++)
	{
		stPara.pstTransRecord = pstTransRecord;
		stPara.nReprintFlag = nReprintFlag;
		stPara.nCurrentPage = nCurrentPage;

		ASSERT_FAIL(PubPrintCommit(_printrecord, (void *)&stPara, 1));
		if (nCurrentPage < nPage)
		{
			NAPI_ScrPush();
			PubClearAll();
			PubDisplayStrInline(DISPLAY_MODE_CENTER, 4, tr("TEAR THE RECEIPT..."));
			PubUpdateWindow();
			if(PubWaitConfirm(30) == KEY_ESC)
			{
				return APP_SUCC;
			}
			PubClearLine(4, 4);
			NAPI_ScrPop();
		}
	}
	return APP_SUCC;
}

/**
* @brief Get input mode
* @param void
* @return 
* @li APP_SUCC
* @li APP_FAIL
* @author 
* @date
*/
void GetInputMode(const char cAttrType, char *pszName)
{
	switch (cAttrType)
	{
	case ATTR_MAGSTRIPE:
	    strcpy(pszName, tr("Swipe"));
		break;
	case ATTR_MANUAL:
		strcpy(pszName, tr("Manual"));
		break;
	case ATTR_CONTACT:
		strcpy(pszName, tr("Chip"));
		break;
	case ATTR_CONTACTLESS:			
		strcpy(pszName, tr("Wave"));
		break;
	case ATTR_FALLBACK:
		strcpy(pszName, tr("Fall"));
		break;
	default:
		//strcpy(pszName, tr("Chip"));
		break;
	}
	
	return;
}

int PrePrintInMiddle( char *pszStr, int nLineFeed)
{
	char szLineFeed[64] = {0};
	char szTmp[256] = {0};

	if (nLineFeed-- <= 0)
		return APP_FAIL;
	
	memset(szTmp, '\r', sizeof(szTmp));
	memcpy(szLineFeed, szTmp, nLineFeed);
	
	return PubPriPrintStr(PRINT_ALIGN_CENTER, pszStr);
	//PubPrePrinter(szLineFeed);
}

static int PrintSign(const STTRANSRECORD *pstTransRecord)
{
	char szTrace[6+1] = {0};
	char szFileName[32+1] = {0};

	PubHexToAsc((uchar *)pstTransRecord->sTrace, 6, 0, (uchar *)szTrace);
	sprintf(szFileName, "%s/%s.bmp", ELECSIGN_PATH, szTrace);
	PubPrintPicture(0, szFileName);
	return APP_SUCC;
}

static int PrintStatement(const STTRANSRECORD *pstTransRecord, const int nCurrentPage)
{
	char szTmpStr[128] = {0};
	char cPinAndSigFlag = pstTransRecord->cPinAndSigFlag;

	if (nCurrentPage == 1) //merchant copy
	{
		PubPrintTail();
	}
	ASSERT(PubSetPrtFont(20, PRN_MODE_NORMAL, 0));
	if (cPinAndSigFlag & CVM_PIN)
	{
		PrePrintInMiddle("PIN VERIFIED", 1);
	}
	else
	{
		PrePrintInMiddle(tr("NO PIN REQUIRED"), 1);
	}

	if (cPinAndSigFlag & CVM_SIG)
	{
		PrePrintInMiddle("SIGNATURE VERIFY", 1);
		PubPrePrinter("\r\r\r");
	}
	else
	{
		PrePrintInMiddle(tr("NO SIGNATURE REQUIRED"), 1);
	}
	PrintSign(pstTransRecord);

	if (nCurrentPage == 1) //merchant copy
	{
		PubPrintTail();
		if (strlen(pstTransRecord->szHolderName) > 0)
		{
			memset(szTmpStr, 0, sizeof(szTmpStr)); 
			ConvertHolderName((uchar *)pstTransRecord->szHolderName, (uchar *)szTmpStr);
			PubPrePrinter("%s", szTmpStr);
		}
		PubPrePrinter(tr("I AGREE TO PAY THE ABOVE TOTAL AMOUNT ACCORDING TO CARD ISSUER AGREEMENT\r"));
	}

	ASSERT(PubSetPrtFont(14, PRN_MODE_HEIGHT_DOUBLE, 0));
	if (1 == nCurrentPage)
	{
		PrePrintInMiddle( tr("**** MERCHANT COPY ****"), 1);
	}
	else if (2 == nCurrentPage)
	{
		PrePrintInMiddle( tr("**** CUSTOMER COPY ****"), 1);
	}
	else 
	{
		PrePrintInMiddle( tr("**** BANK COPY ****"), 1);
	}
	PubPrePrinter("\r");

	return APP_SUCC;
}


int PrintRecord_TP(const STTRANSRECORD *pstTransRecord, int nReprintFlag, const int nCurrentPage)
{
	char szInputMode[10] = {0};		
	char szTmpStr[80+1] = {0}, szStr[256];
	char szBatch[6+1] = {0}, szTrace[6+1] = {0}, szPan[20+1] = {0};
	char szPosYear[4+1];
	char cPinAndSigFlag = pstTransRecord->cPinAndSigFlag;
    int	nPage;

	PrintBillHead();
	
	//Trans name
	GetTransName(pstTransRecord->cTransType, szStr);
	ASSERT(PubSetPrtFont(20, PRN_MODE_HEIGHT_DOUBLE, 0));
	PrePrintInMiddle(szStr, 1);

	ASSERT(PubSetPrtFont(14, PRN_MODE_HEIGHT_DOUBLE, 0));
	
	//Card no and input mode
	GetInputMode(pstTransRecord->cTransAttr, szInputMode);
	memset(szTmpStr, 0, sizeof(szTmpStr));
	PubHexToAsc((uchar *)pstTransRecord->sPan, pstTransRecord->nPanLen, 0, (uchar *)szPan);

	CtrlCardPan((uchar*)szPan, (uint)pstTransRecord->nPanLen, (uint)pstTransRecord->cTransType, pstTransRecord->cEMV_Status);
	PubPrePrinter(tr("Card No:"));

	ASSERT(PubSetPrtFont(20, PRN_MODE_HEIGHT_DOUBLE, 0));
	sprintf(szStr, "%s %s", szPan, szInputMode);
	PrePrintInMiddle(szStr, 3);

	ASSERT(PubSetPrtFont(14, PRN_MODE_HEIGHT_DOUBLE, 0));

	//Aid and TC
	if ((pstTransRecord->cTransAttr != ATTR_MAGSTRIPE 
		&& pstTransRecord->cTransAttr != ATTR_MANUAL
		&& pstTransRecord->cTransAttr != ATTR_FALLBACK)
		&& (pstTransRecord->cTransType == TRANS_SALE 
		|| pstTransRecord->cTransType == TRANS_CASHBACK
		|| pstTransRecord->cTransType == TRANS_OFFLINE
		|| pstTransRecord->cTransType == TRANS_REFUND
		|| pstTransRecord->cTransType == TRANS_PREAUTH))
	{
		STEMVADDTIONRECORDINFO *pstAddition = (STEMVADDTIONRECORDINFO *)pstTransRecord->sAddition;
		PubPrePrinter(tr("APP:%s"), pstAddition->szAIDLable);
		PubPrePrinter(tr("APP Preferred Name:%s"), pstAddition->szAppPreferedName);
		PntHex(tr("AID:"), (uchar *)pstAddition->sAID, pstAddition->cAIDLen);
		PntHex(tr("TC:"), (uchar *)pstAddition->sTC, 8);
		PntHex(tr("TSI"), (uchar *)pstAddition->sTSI, 2);
		PntHex(tr("TVR"), (uchar *)pstAddition->sTVR, 5);
	}

	//REF number, trace no and so on
	PubHexToAsc((uchar *)pstTransRecord->sBatchNum, 6, 0,  (uchar *)szBatch);
	PubHexToAsc((uchar *)pstTransRecord->sTrace, 6, 0,  (uchar *)szTrace);
	PubPrePrinter(tr("REF. NO:%12s"), pstTransRecord->szRefnum);
	PubPrePrinter(tr("APPR.CODE:%s"), pstTransRecord->szAuthCode);
	PubPrePrinter(tr("BATCH NO:%.6s  TRACE NO:%.6s"), szBatch, szTrace);

	//Date time
	memset(szPosYear, 0, sizeof(szPosYear));
	PubGetCurrentDatetime(szTmpStr);
	memcpy(szPosYear, szTmpStr, 4);
	PubPrePrinter(tr("DATE:%.4s/%02x/%02x TIME:%02x:%02x:%02x\r"), szPosYear, pstTransRecord->sDate[0], pstTransRecord->sDate[1],
		pstTransRecord->sTime[0], pstTransRecord->sTime[1], pstTransRecord->sTime[2]);

	//Print Amt
	ASSERT(PubSetPrtFont(20, PRN_MODE_HEIGHT_DOUBLE, 0));
	PrintAmt(pstTransRecord);
	ASSERT(PubSetPrtFont(12, PRN_MODE_HEIGHT_DOUBLE, 0));

	// original data
	switch(pstTransRecord->cTransType)
	{
	case TRANS_ADJUST:
	case TRANS_VOID:
		PubPrintTail();
		PubPrePrinter(tr("ORIGINAL TRACE:%02x%02x%02x"), pstTransRecord->sOldTrace[0], pstTransRecord->sOldTrace[1], pstTransRecord->sOldTrace[2]);
		break;
	case TRANS_AUTHCOMP:
	case TRANS_VOID_PREAUTH:
		PubPrintTail();
		PubPrePrinter(tr("ORIGINAL AUTHCODE:%s"), pstTransRecord->szOldAuthCode);
		break;
	case TRANS_REFUND:
		PubPrintTail();
		PubPrePrinter(tr("ORIGINAL REF:%s"), pstTransRecord->szOldRefnum);
		PubPrePrinter(tr("ORIGINAL DATE:%.2s/%.2s"), pstTransRecord->szOldDate, pstTransRecord->szOldDate + 2);
		break;
	case TRANS_VOID_AUTHSALE:
		PubPrintTail();
		PubPrePrinter(tr("ORIGINAL TRACE:%02x%02x%02x"), pstTransRecord->sOldTrace[0], pstTransRecord->sOldTrace[1], pstTransRecord->sOldTrace[2]);
		PubPrePrinter(tr("ORIGINAL AUTHCODE:%s"), pstTransRecord->szOldAuthCode);
		break;
	default:
		break;
	}
	/**********************end**************************/
	//Print statement
	PrintStatement(pstTransRecord, nCurrentPage);

	if (nReprintFlag == REPRINT)
	{
		ASSERT(PubSetPrtFont(14, PRN_MODE_HEIGHT_DOUBLE, 0));
		PubPrePrinter(tr("DUPLICATED"));
		ASSERT(PubSetPrtFont(12, PRN_MODE_HEIGHT_DOUBLE, 0));
	}

    GetVarPrintPageCount(&nPage);	
	if (1 == nCurrentPage || nCurrentPage < nPage)
	{
		PubClearAll();
		if (cPinAndSigFlag & CVM_SIG)
		{
			PubDisplayGen(2, tr("SIGNATURE VERIFY"));
			PubDisplayGen(3, tr("SIGNATURE"));
		}
		else
		{	
			PubDisplayGen(2, tr("NO SIGNATURE"));
			PubDisplayGen(3, tr("THANK YOU ! "));
		}
		PubUpdateWindow();
	}
	return APP_SUCC;
}

/**
* @brief Print receipt(Call by PrintRecord)
* @param in void *ptrPara 
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int _printrecord( void *ptrPara )
{
	STPRINTPATA *pstPara;
	
	pstPara = (STPRINTPATA *)ptrPara;

	ASSERT_FAIL(PrintRecord_TP(pstPara->pstTransRecord, pstPara->nReprintFlag, pstPara->nCurrentPage));
	
	return APP_SUCC;
}

static int _printallrecord(void *ptrPara)
{
	static int nHasRecord;
	int nFileHandle;
	STTRANSRECORD stTransRecord;
	int nRecordNum = 0;
	int i, nSpace, nOff = 0;
	uint unSpcWidth, unTypeWidth;
	int nStartRecNo;
	char szTransName[64] = {0}, szAmt[16] = {0}, szCardNo[32] = {0}, szDateTime[14+1] = {0};
	char szNii[3+1] = {0};
	char szAcqName[20+1] = {0};
	char szTmp[128] = {0}, szSpace[64];
	
	int *pnCurPrintRecNo = (int *)ptrPara;
		

	GetRecordNum(&nRecordNum);
	nStartRecNo = *pnCurPrintRecNo;
	if (nStartRecNo==1)
	{
		nHasRecord=0;
	}

	ASSERT_FAIL(PubOpenFile(FILE_RECORD, "r", &nFileHandle));

	for(i=0; (*pnCurPrintRecNo <=nRecordNum); (*pnCurPrintRecNo)++)
	{
		ReadTransRecord(nFileHandle, *pnCurPrintRecNo, &stTransRecord);
		if (stTransRecord.cSendFlag == 2)
		{
			continue;
		}

		// Print head
		if (nStartRecNo==1 && i==0 )
		{
			ASSERT(PubSetPrtFont(18, PRN_MODE_HEIGHT_DOUBLE, 0));
			PrePrintInMiddle( tr("TXN LIST"), 2);
			
			if (NO == GetVarPrintDetialHalt())
			{
				if (PubGetPrinter() == _PRINTTYPE_IP)
				{
					ASSERT(PubSetPrtFont(12, PRN_MODE_HEIGHT_DOUBLE, 0));
				}
				
				PubGetCurrentDatetime(szDateTime);
				GetVarCommNii(szNii);
				GetAcqName(szAcqName);
				
				ASSERT(PubSetPrtFont(14, PRN_MODE_HEIGHT_DOUBLE, 0));
				PubPrePrinter(tr("DATE:%4.4s/%2.2s/%2.2s  TIME:%2.2s:%2.2s:%2.2s\r"), 
					szDateTime, szDateTime+4, szDateTime+6, 
					szDateTime+8, szDateTime+10, szDateTime+12);
				PubPrePrinter(tr("HOST NII:%s  %s\r"), szNii, szAcqName);
			}	
			ASSERT(PubSetPrtFont(12, PRN_MODE_HEIGHT_DOUBLE, 0));
			PubPrePrinter(tr("TYPE                                      CARDNO."));
			PubPrePrinter(tr("DATETIME                   TRACE    AMOUNT"));
			PubPrePrinter(" ===============================================");	
				
		}

		//TYPE
		GetTransName(stTransRecord.cTransType, szTransName);
		//CARD NO
		PubHexToAsc((uchar *)stTransRecord.sPan, stTransRecord.nPanLen, 0, (uchar *)szCardNo);
		CtrlCardPan((uchar*)szCardNo, (uint)stTransRecord.nPanLen, (uint)stTransRecord.cTransType, stTransRecord.cEMV_Status);
		//GetInputMode(stTransRecord.cTransAttr, szInputMode);

		memset(szSpace, ' ', sizeof(szSpace) - 1);
		NAPI_PrnGetTTFStrSize("\x20", 1, (int *)&unSpcWidth, NULL);
		NAPI_PrnGetTTFStrSize(szTransName, strlen(szTransName), (int *)&unTypeWidth, NULL);
		if (unTypeWidth < PRINT_PAPER_WIDTH/2)
		{
			nSpace = (PRINT_PAPER_WIDTH/2 - unTypeWidth) / unSpcWidth;
		}
		else
		{
			nSpace = 1;
		}
		nOff = strlen(szTransName);
		memcpy(szTmp, szTransName, nOff);
		memcpy(szTmp + nOff, szSpace, nSpace);
		nOff += nSpace;
		sprintf(szTmp + nOff, "%s", szCardNo);
		nOff += strlen(szCardNo);
		// print type + cardNo
		PubPrePrinter("%s", szTmp);

		GetPrintAmt(&stTransRecord, szAmt);
		// print date + time + trace + amount
		PubPrePrinter("%02x/%02x %02x:%02x:%02x            %02x%02x%02x       %-13s\r",  stTransRecord.sDate[0], stTransRecord.sDate[1], 
			stTransRecord.sTime[0], stTransRecord.sTime[1], stTransRecord.sTime[2],
			stTransRecord.sTrace[0], stTransRecord.sTrace[1], stTransRecord.sTrace[2], szAmt);

		if((nStartRecNo == 1) && (i >= 11) && (PubGetPrinter() == _PRINTTYPE_IP))
		{
			(*pnCurPrintRecNo)++;
			break;
		}
		i++;
	}
	
	ASSERT_FAIL(PubCloseFile(&nFileHandle));

	if (i>0)
	{
		nHasRecord = 1;
	}
	else
	{
		PubMsgDlg(tr("PRINT ALL"), tr("NO TRANS RECORD TO PRINT"), 0, 5);
		return -5;
	}
		
	//Print tail
	if (*pnCurPrintRecNo>nRecordNum && nHasRecord==1)
	{
		if (PubGetPrinter() == _PRINTTYPE_TP)
		{
			PubPrePrinter( "\r\r- - - - - - - X - - - - - - - X - - - - - - - ");
		}	
	}
	return APP_SUCC;
}

/**
* @brief Print detail
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int PrintAllRecord(void)
{
	int nRecordNum = 0, nCurPrintRecNo=1;
	int nRet = 0;
	
	GetRecordNum(&nRecordNum);
	if( nRecordNum<=0 )
	{
		PubMsgDlg(tr("PRINT ALL"), tr("NO TRANS"), 0, 5);
		return APP_SUCC;
	}
	while(1)
	{
		if(nCurPrintRecNo > nRecordNum)
		{
			break;
		}
		nRet = PubPrintCommit(_printallrecord, (void *)&nCurPrintRecNo, 1);
		if(nRet != APP_SUCC)
		{
			if (APP_FUNCQUIT == nRet)
			{
				PubMsgDlg(tr("PRINT ALL"), tr("NO TRANS TO PRINT"), 1, 3);
			}
			return nRet;
		}
	}
	return APP_SUCC;
}

void PrintSettleInfo(char *pszType, SETTLE_NUM nSettleNum, uchar *psSettleAmt)
{
	char szPrintInfo[128] = {0};
	char szDispAmt[DISPAMTLEN] = {0};
	char szSpace[64];
	int nSpace, nOff = 0;
	uint unSpcWidth, unTypeWidth;

	memset(szSpace, ' ', sizeof(szSpace) - 1);
	NAPI_PrnGetTTFStrSize("\x20", 1, (int *)&unSpcWidth, NULL);
	NAPI_PrnGetTTFStrSize(pszType, strlen(pszType), (int *)&unTypeWidth, NULL);

	strcpy(szPrintInfo, pszType);
	nOff += strlen(pszType);
	if (unTypeWidth >= PRINT_PAPER_WIDTH/2-15)
	{
		nSpace = 1;
	}
	else
	{
		nSpace = (PRINT_PAPER_WIDTH/2 - 15 - unTypeWidth) / unSpcWidth;
	}

	memcpy(szPrintInfo + nOff, szSpace, nSpace);
	nOff += nSpace;
	sprintf(szPrintInfo + nOff, "%03d", nSettleNum);
	nOff += 3;
	memcpy(szPrintInfo + nOff, szSpace, 8);
	nOff += 8;
	BcdAmtToDisp(psSettleAmt, szDispAmt);
	strcat(szPrintInfo, szDispAmt);

	PubPrePrinter(tr(szPrintInfo));
}


int _printsettle(void *ptrPara)
{
	ST_SETTLE_REC stSettleRec;
	STRECFILE stSettleFile;
	STAMT_NUM_INFO_SETTLE stInfoSettle;
	char szDebitAmt[12+1] = {0};
	char szCreditAmt[12+1] = {0};
	char szTotalAmt[12+1] = {0};
	char szTotalAmtBcd[12+1] = {0};
	char szTmpstr[128];
	int nNumAll;

	int *pnReprintFlag = (int *)ptrPara;
	
	memset(&stSettleRec, 0, sizeof(ST_SETTLE_REC));
	memset(&stInfoSettle, 0, sizeof(stInfoSettle));
	
	if (*pnReprintFlag == REPRINT)
	{
		ASSERT_FAIL(PubReadOneRec(LASTSETTLEFILE, 1, (char *)(&stSettleRec)));
	}
	else
	{
		GetVarInvoiceNo(stSettleRec.szInvoice);
		GetVarBatchNo(stSettleRec.BatchNo);
		PubGetCurrentDatetime(stSettleRec.BatchDatetime);
		GetVarSettleDateTime(szTmpstr);
		if (memcmp(szTmpstr, "\x00\x00\x00\x00\x00", 5) == 0)
		{
			;
		}
		else
		{
			PubHexToAsc((uchar *)szTmpstr, 10, 0, (uchar *)(stSettleRec.BatchDatetime + 4));
		}
		stSettleRec.SettleFlag = GetVarIsTotalMatch();
		GetSettleData(&(stSettleRec.stSettle));
		if (*pnReprintFlag == FIRSTPRINT)
		{
			strcpy(stSettleFile.szFileName, LASTSETTLEFILE);
			stSettleFile.cIsIndex = 0x00;
			stSettleFile.unMaxOneRecLen = sizeof(ST_SETTLE_REC);
			stSettleFile.unIndex1Start = 1;
			stSettleFile.unIndex1Len = 1;
			stSettleFile.unIndex2Start = 1;
			stSettleFile.unIndex2Len = 1;
			ASSERT(PubCreatRecFile(&stSettleFile));
			ASSERT(PubAddRec(LASTSETTLEFILE, (char *)(&stSettleRec)));
		}
	}

	if (PubGetPrinter()==_PRINTTYPE_TP)
	{
		ASSERT(PubSetPrtFont(18, PRN_MODE_HEIGHT_DOUBLE, 0));
	}
	else
	{	
		ASSERT(PubSetPrtFont(12, PRN_MODE_HEIGHT_DOUBLE, 0));
	}	
 
	ASSERT(PubSetPrtFont(18, PRN_MODE_HEIGHT_DOUBLE, 0));
	PrintBillHead();	
	ASSERT(PubSetPrtFont(20, PRN_MODE_HEIGHT_DOUBLE, 0));
	if (*pnReprintFlag==TOTAL_PRINT)
	{
		PrePrintInMiddle( tr("TOTAL"), 2);
	}
	else
	{
		PrePrintInMiddle( tr("SETTLEMENT"), 2);
	}

	ASSERT(PubSetPrtFont(14, PRN_MODE_HEIGHT_DOUBLE, 0));

	// SETTLE DATETIME
	PubPrePrinter(tr("DATE:%4.4s/%2.2s/%2.2s  TIME:%2.2s:%2.2s:%2.2s\r"), 
		stSettleRec.BatchDatetime, stSettleRec.BatchDatetime+4, stSettleRec.BatchDatetime+6, 
		stSettleRec.BatchDatetime+8, stSettleRec.BatchDatetime+10, stSettleRec.BatchDatetime+12);

	//BATCH NO, INVOICE NO
	GetVarInvoiceNo(stSettleRec.szInvoice);
	if (*pnReprintFlag==TOTAL_PRINT)
	{
		PubPrePrinter(tr("BATCH NO:%.6s"), stSettleRec.BatchNo);
	}
	else
	{
		PubPrePrinter(tr("BATCH NO:%.6s\rINVOICE NO:%.6s"), stSettleRec.BatchNo, stSettleRec.szInvoice);
	}

	PubPrePrinter(tr("***SUMMARY OF CHARGE***"));

	PubPrePrinter(tr("TYPE                                              NUM      AMOUNT"));
	
	PubPrePrinter(" ==========================================");
	//BASE
	PrintSettleInfo("SALE", stSettleRec.stSettle._SaleNum, stSettleRec.stSettle._SaleAmount);
	//VOID
	PrintSettleInfo("VOID", stSettleRec.stSettle._VoidSaleNum, stSettleRec.stSettle._VoidSaleAmount);
	//REFUND
	PrintSettleInfo("REFUND", stSettleRec.stSettle._RefundNum, stSettleRec.stSettle._RefundAmount);
#if 0
	//ADJUST/TIP
	PrintSettleInfo("TIPS", stSettleRec.stSettle._TipsNum, stSettleRec.stSettle._TipsAmount);
#endif
	//OFFLINE
	PrintSettleInfo("OFFLINE", stSettleRec.stSettle._OfflineNum, stSettleRec.stSettle._OfflineAmount);
	//AUTHCOMP
	PrintSettleInfo("AUTHCOMP", stSettleRec.stSettle._AuthSaleNum, stSettleRec.stSettle._AuthSaleAmount);
	//_VoidAuthSaleNum
	PrintSettleInfo("VOID AUTHCOMP", stSettleRec.stSettle._VoidAuthSaleNum, stSettleRec.stSettle._VoidAuthSaleAmount);

	// TOTAL
	DealSettle(&stInfoSettle, stSettleRec.stSettle);
	PubHexToAsc((uchar *)stInfoSettle.sDebitAmount, 12, 0, (uchar *)szDebitAmt);
	PubHexToAsc((uchar *)stInfoSettle.sCreditAmount, 12, 0, (uchar *)szCreditAmt);
	AmtSubAmt((uchar *)szDebitAmt, (uchar *)szCreditAmt, (uchar *)szTotalAmt);
	PubAscToHex((uchar *)szTotalAmt, 12, 0, (uchar *)szTotalAmtBcd);

	nNumAll = stInfoSettle.nDebitNum + stInfoSettle.nCreditNum;
	PubPrePrinter(" ==========================================");
	PrintSettleInfo("TOTAL", nNumAll, (uchar *)szTotalAmtBcd);

	if (*pnReprintFlag == REPRINT)
	{
		PubPrePrinter(tr(" ******REPRINT*******"));
	}
	
	if (PubGetPrinter() == _PRINTTYPE_TP)
	{
		PubPrePrinter("\r" );
	}

	return APP_SUCC;
}

/**
* @brief Print settle
* @param in int nReprintFlag  
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int PrintSettle(int nReprintFlag)
{	
	int nRet = -1;
	ST_SETTLE_REC stSettleRec;

	memset(&stSettleRec, 0, sizeof(ST_SETTLE_REC));
	if (REPRINT == nReprintFlag)
	{
		nRet = PubReadOneRec(LASTSETTLEFILE, 1, (char *)(&stSettleRec));
		if (APP_SUCC != nRet)
		{
			PubMsgDlg(tr("REPRINT SETTLE"), tr("NO RECEIPT"), 0, 10);
			return APP_QUIT;
		}
	}	
	
	PubClearAll();
	DISP_PRINTING_NOW;
	ASSERT(PubPrintCommit( _printsettle, (void *)&nReprintFlag, 1));
	return APP_SUCC;
}

/**
* @brief Print summary
* @param in int nReprintFlag  
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int PrintTotal()
{
	ST_SETTLE_REC stSettleRec;
	int nPrintFlag = TOTAL_PRINT;

	memset(&stSettleRec, 0, sizeof(ST_SETTLE_REC));
	
	PubClearAll();
	DISP_PRINTING_NOW;
	ASSERT(PubPrintCommit( _printsettle, (void *)&nPrintFlag, 1));

	return APP_SUCC;
}

int _printParam(void *ptrPara)
{
	char szTmpStr[21];
	STAPPPOSPARAM stAppPosParam;
	STAPPCOMMPARAM stAppCommParam;
	
	int *pnPrintType=(int *)ptrPara	;
	char szSoftVer[16+1];
	char szDispAmt[DISPAMTLEN] = {0};
	

	PubClearAll();
	PubDisplayStrInline(DISPLAY_MODE_CENTER, 3, tr("PRINTING..."));
	PubUpdateWindow();

	ASSERT(PubSetPrtFont(12, PRN_MODE_HEIGHT_DOUBLE, 0));

	if((*pnPrintType)&PRINT_PARAM_TYPE_EMV) /*<EMV PARAM*/
	{
		return _printemvparam();
	}
		
	// Get parameter
	GetAppCommParam(&stAppCommParam);
	GetAppPosParam(&stAppPosParam);

	if((*pnPrintType)&PRINT_PARAM_TYPE_MERCHANTINFO) /*<Merchant msg*/
	{
		PubPrePrinter(tr("MERCHANT INFO\r"));
		
		PubPrePrinter(tr("Merchant Name:%s"), stAppPosParam.szMerchantNameEn);

		PubPrePrinter(tr("Merchant addr1:%s"), stAppPosParam.szMerchantAddr[0]);
		PubPrePrinter(tr("Merchant addr2:%s"), stAppPosParam.szMerchantAddr[1]);
		PubPrePrinter(tr("Merchant addr3:%s"), stAppPosParam.szMerchantAddr[2]);

		PubPrePrinter(tr("Merchant ID:%s"), stAppPosParam.szMerchantId);

		PubPrePrinter(tr("Terminal ID:%s"), stAppPosParam.szTerminalId);

		if(strlen(stAppPosParam.szAppDispname)>0)
		{
			PubPrePrinter(tr("App Name:%s"), stAppPosParam.szAppDispname);
		}
		else
		{
			PubPrePrinter(tr("App Name:%s"), APP_NAMEDESC);
		}	
	}

	if((*pnPrintType)&PRINT_PARAM_TYPE_VERSION) /*<Version msg*/
	{
		PubPrePrinter(tr("VERSION INFO\r"));
		
		memset(szSoftVer, 0, sizeof(szSoftVer));
		memcpy(szSoftVer, "V", 1);
		GetVarSoftVer(&szSoftVer[1]);	
		PubPrePrinter(tr("App Version: %s"), szSoftVer);
	}
	if((*pnPrintType)&PRINT_PARAM_TYPE_TRANSCCTRL) /*<Transactiion control*/
	{
		PubPrePrinter(tr("TRANS CONTROL\r"));
		
		PntTraditionOnoff(stAppPosParam.sTransSwitch);
		PubPrePrinter(tr("Enable Manul:%s"), stAppPosParam.cIsCardInput == '1' ? tr("YES") : tr("NO"));
		PubPrePrinter(tr("CVV2:%s"), stAppPosParam.cIsNeedCVV2 == '0' ? tr("NO") : tr("YES"));
		PubPrePrinter(tr("Default Trans:%s"), stAppPosParam.cDefaultTransType == '1' ? tr("SALE") : tr("AUTH"));
		memset(szDispAmt, 0, sizeof(szDispAmt));
		PubPrePrinter(tr("Print Record Detail:%s"), stAppPosParam.cIsPntDetail == '1' ? tr("YES") : tr("NO"));
		PubPrePrinter(tr("Offline Send Times:%d"), stAppCommParam.cOffResendNum >= 0x30 ? stAppCommParam.cOffResendNum - 0x30 : stAppCommParam.cOffResendNum);
		PubPrePrinter(tr("Enter Admin Pwd:%s"), stAppPosParam.cIsAdminPwd == '1' ? tr("YES") : tr("NO"));
	}
	
	if((*pnPrintType)&PRINT_PARAM_TYPE_SYSTEMCTRL) /*<System param*/
	{
		PubPrePrinter(tr("SYSTEM CONTROL\r"));
		
		memset(szTmpStr, 0, sizeof(szTmpStr));
		GetVarTraceNo(szTmpStr);
		PubPrePrinter(tr("Trace No.:%s"), szTmpStr);
		memset(szTmpStr, 0, sizeof(szTmpStr));
		GetVarBatchNo(szTmpStr);
		PubPrePrinter(tr("Batch No.:%s"), szTmpStr);
		PubPrePrinter(tr("Print Page:%d"), stAppPosParam.cPrintPageCount - '0');
		PubPrePrinter(tr("Reversal Times:%d"), stAppCommParam.cReSendNum >= 0x30 ? stAppCommParam.cReSendNum - 0x30 : stAppCommParam.cReSendNum);
		PubPrePrinter(tr("Max Trans Count:%d"), atoi(stAppPosParam.szMaxTransCount));
		PubPrePrinter(tr("Tip Rate:%d"), atoi(stAppPosParam.szTipRate));
		PubPrePrinter(tr("Print Minus:%s"), stAppPosParam.cIsPrintMinus == '1' ? tr("YES") : tr("NO"));
	}

	if((*pnPrintType)&PRINT_PARAM_TYPE_OTHER) /*<Other*/
	{
		PubPrePrinter(tr("OTHER\r"));
		
		PubPrePrinter(tr("Enable Swipe In Void:%s"), stAppPosParam.cIsVoidStrip == '1' ? tr("YES") : tr("NO"));
		PubPrePrinter(tr("Enable Pin In Void:%s"), stAppPosParam.cIsVoidPin == '1' ? tr("YES") : tr("NO"));
		PubPrePrinter(tr("Key Index:%s"), stAppPosParam.szMainKeyNo);
		PubPrePrinter(tr("Encypt Mode:%s"), stAppPosParam.cEncyptMode == '0' ? tr("DES") : tr("3DES"));
	}

	if((*pnPrintType)&PRINT_PARAM_TYPE_COMM)
	{
		PubPrePrinter("COMM\r");
		
		switch(stAppCommParam.cCommType)
		{
		case COMM_RS232:
			PubPrePrinter(tr("Comm Type: %s"), tr("RS232"));
			break;
		case COMM_DIAL:
			PubPrePrinter(tr("Comm Type: %s"), tr("MODEM"));
			PubPrePrinter(tr("Tel.1: %s"), stAppCommParam.szTelNum1);
			PubPrePrinter(tr("Tel.2: %s"), stAppCommParam.szTelNum1);
			PubPrePrinter(tr("Tel.3: %s"), stAppCommParam.szTelNum1);
			break;
		case COMM_GPRS:
		case COMM_CDMA:
		case COMM_ETH:
		case COMM_WIFI:
			if (stAppCommParam.cCommType == COMM_GPRS)
			{
				PubPrePrinter(tr("Comm Type: %s"), "GPRS");
				PubPrePrinter("APN.1: %s", stAppCommParam.szAPN1);
				PubPrePrinter("APN.2: %s", stAppCommParam.szAPN2);
			}
			else if (stAppCommParam.cCommType == COMM_ETH)
			{
				PubPrePrinter(tr("Comm Type: %s"), tr("ETH"));
				PubPrePrinter(tr("Ip Addr.1: %s"), stAppCommParam.szIpAddr);
				PubPrePrinter(tr("Mask: %s"), stAppCommParam.szMask);
				PubPrePrinter(tr("Gate: %s"), stAppCommParam.szGate);
			}
			else if (stAppCommParam.cCommType == COMM_CDMA)
			{
				PubPrePrinter(tr("Comm Type: %s"), "CDMA");			
			}
			else if(stAppCommParam.cCommType == COMM_WIFI)
			{
				PubPrePrinter(tr("Comm Type: %s"), "Wifi");			
				PubPrePrinter("Ssid: %s", stAppCommParam.szWifiSsid);			
				switch(stAppCommParam.cWifiMode)
				{	
				case WIFI_AUTH_OPEN:
					PubPrePrinter(tr("Wifi Net Mode: OPEN"));			
					break;
				case WIFI_AUTH_WEP_PSK:
					PubPrePrinter(tr("Wifi Net Mode: SHARED"));			
					break;
				case WIFI_AUTH_WPA_PSK:
					PubPrePrinter(tr("Wifi Net Mode: WAP"));			
					break;
				case WIFI_AUTH_WPA2_PSK:
					PubPrePrinter(tr("Wifi Net Mode: WAP2"));			
					break;
				default:
					PubPrePrinter(tr("Wifi Net Mode: NONE"));			
					break;
				}
			}

			if(stAppCommParam.cIsDns)
			{
				PubPrePrinter("Dns Ip: %s", stAppCommParam.szDNSIp1);			
				PubPrePrinter("Domain.1: %s", stAppCommParam.szDomain);			
				PubPrePrinter("Domain.2: %s", stAppCommParam.szDomain2);			
				PubPrePrinter("Dns Port.1: %s", stAppCommParam.szPort1);			
				PubPrePrinter("Dns Port.2: %s", stAppCommParam.szPort2);			
			}
			else
			{
				PubPrePrinter("Ip.1: %s", stAppCommParam.szIp1);			
				PubPrePrinter("Ip.2: %s", stAppCommParam.szIp2);			
				PubPrePrinter("Port.1: %s", stAppCommParam.szPort1);			
				PubPrePrinter("Port.2: %s", stAppCommParam.szPort2);			
			}

			if ((stAppCommParam.cCommType == COMM_GPRS)||(stAppCommParam.cCommType == COMM_CDMA))
			{
				PubPrePrinter(tr("User Name: %3.3s****"), stAppCommParam.szUser);			
				
				PubPrePrinter(tr("User Pwd: %3.3s****"), stAppCommParam.szPassWd);			

				PubPrePrinter(tr("Wireless Dialnum: %s"), stAppCommParam.szWirelessDialNum);

				PubPrePrinter(tr("Wireless Mode: %d"), stAppCommParam.cMode);
			}
			break;
		default:
			break;
		}
		
		PubPrePrinter("NII: %s", stAppCommParam.szNii);			
		
		PubPrePrinter(tr("Redial Times: %d"), stAppCommParam.cReDialNum >= 0x30 ? stAppCommParam.cReDialNum - 0x30 : stAppCommParam.cReDialNum);			

		PubPrePrinter(tr("Timeout: %d"), stAppCommParam.cTimeOut);			

		PubPrePrinter(tr("Is Predial: %s"), stAppCommParam.cPreDialFlag=='1' || stAppCommParam.cPreDialFlag==1 ? tr("YES") : tr("NO"));			
	}
	
	return APP_SUCC;
}

int PrintParam(void)
{
	int nRet;
	int nType=0;
	char *pszItems[] = {
		tr("1.MERCHANT"), 
		tr("2.TRANS CONTROL"), 
		tr("3.SYSTEM CONTROL"), 
		tr("4.COMM"),
		tr("5.VERSION"), 
		tr("6.EMV PARA"), 
		tr("7.OTHER")
	};
	int nSelcItem = 1, nStartItem = 1;

	while(1)
	{
		nRet = PubShowMenuItems(tr("PRINT PARA"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 0);
	
		if (nRet==APP_QUIT || nRet==APP_TIMEOUT)
		{
			return nRet;
		}

		switch(nSelcItem)
		{
		case 1:
			nType=PRINT_PARAM_TYPE_MERCHANTINFO;
			break;
		case 2:
			nType=PRINT_PARAM_TYPE_TRANSCCTRL;	
			break;
		case 3:
			nType=PRINT_PARAM_TYPE_SYSTEMCTRL;
			break;
		case 4:
			nType=PRINT_PARAM_TYPE_COMM;
			break;
		case 5:
			nType=PRINT_PARAM_TYPE_VERSION;
			break;
		case 6:
			nType=PRINT_PARAM_TYPE_EMV;
			break;
		case 7:
			nType=PRINT_PARAM_TYPE_OTHER;
			break;
		default :                         
			continue;
		}

		ASSERT_FAIL(PubPrintCommit(_printParam, (void *)&nType, 1));
	}
	return APP_SUCC;
}


/**
* @brief Deal print limit. (if power is low, the printer can not work)
* @param void
* @return
* @li APP_SUCC 
* @li APP_FAIL
* @li APP_QUIT
* @author 
* @date
*/
int DealPrintLimit(void)
{	
	if(PubLowPowerCheck() == APP_SUCC)
	{
		PubMsgDlg(tr("Warning"),tr("LOW BATTERY\nPLEASE CHARGE"),3,30);
		return APP_FAIL;
	}
	else
	{
		return APP_SUCC;
	}
}


/**
* @brief Deal paper of printer. (if out of paper, the transaction unable process)
* @param void
* @return
* @li APP_SUCC
* @li APP_FAIL
* @author
* @date
*/
int DealPrinterPaper()
{
	int nStatus;
	char szContent[100] = {0};
	char szErrorInfo[32] = {0};

	if (PubIsSupportPrint() == NO) {
		return APP_SUCC;
	}
	while(1)
	{
		nStatus = PubGetPrintStatus();
		TRACE("nStatus = %d", nStatus);
		switch (nStatus)
		{
		case NAPI_PRN_STATUS_OK:
			break;
		default:
			memset(szContent, 0, sizeof(szContent));
			if (nStatus == NAPI_PRN_STATUS_NOPAPER)
			{
				strcpy(szErrorInfo, "|COut of paper");
			}
			else
			{
				sprintf(szErrorInfo, "|CError of printer (%d)", nStatus);
			}
			PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent, szErrorInfo);
			if (PubConfirmDlg("Printer Status", szContent, 3, 50) == APP_SUCC)
			{
				continue;
			}
			return APP_FAIL;
			break;
		}
		break;
	}
	return APP_SUCC;
}

/**
*	Print transaction switch
* @param TransFlag 
* @return void
*/
static void PntTraditionOnoff(const char *szTransFlag)
{
	int i;
	int nTransNum=TRANSSWITCHNUM;		
	char *lszName[TRANSSWITCHNUM]= {tr("SALE"),tr("VOID"),NULL,tr("PREAUTH"), tr("AUTHCOMP"),
	                               NULL,tr("ADJUST"),tr("BALANCE"), NULL, tr("VOID AUTHSALE"), tr("VOID PREAUTH")};
			
	for(i=0;i<nTransNum;i++)
	{	
		if (lszName[i]==0)
			continue;
		if (YES == GetTransSwitchOnoff(i))
		{
			PubPrePrinter(tr("%s: YES"), lszName[i]);
		}
		else
		{
			PubPrePrinter(tr("%s: NO"), lszName[i]);
		}

	}
}


static int PntHex(const char *pszTitle, const uchar *psContent, const int nConternLen)
{
	char szAsc[1024] = {0};

	if(pszTitle)
		strcpy(szAsc, pszTitle);
	PubHexToAsc(psContent, nConternLen*2, 0, (uchar *)szAsc + strlen(pszTitle));
	ASSERT_FAIL(PubPrePrinter(szAsc));
	return APP_SUCC;		
}


static int AidParse(unsigned char tlv_list[], int tlv_len, PRT_AIDPARAM * pstAidParam)
{
	int i = 0;
	int nRet = 0;
	tlv_t stTlvObj[30]; 

	memset(stTlvObj, 0, sizeof(stTlvObj));
	nRet = TlvParse(tlv_list, tlv_len, stTlvObj, 30);
	if (nRet != 0)
	{
		PubDebug("[%s][%d] nRet = %d\n", __func__, __LINE__, nRet);
		return APP_FAIL;
	}
	for(i=1; i<=stTlvObj[0].nChildNum; i++)
	{
		switch(stTlvObj[i].unTagName)
		{
		case _EMVPARAM_9F06_AID:
			memcpy(pstAidParam->usAid, stTlvObj[i].pusValue, stTlvObj[i].unValueLen);
			pstAidParam->ucAidLen = stTlvObj[i].unValueLen;
			break;
		case _EMVPARAM_9F09_APPVER:
			memcpy(pstAidParam->usAppVer, stTlvObj[i].pusValue, stTlvObj[i].unValueLen);
			break;
		case _EMVPARAM_DF11_TACDEFAULT:
			memcpy(pstAidParam->usTacDefault, stTlvObj[i].pusValue, stTlvObj[i].unValueLen);
			break;
		case _EMVPARAM_DF13_TACDENIAL:
			memcpy(pstAidParam->usTacDenial, stTlvObj[i].pusValue, stTlvObj[i].unValueLen);
			break;
		case _EMVPARAM_DF12_TACONLINE:
			memcpy(pstAidParam->usTacOnline, stTlvObj[i].pusValue, stTlvObj[i].unValueLen);
			break;
		case _EMVPARAM_9F1B_FLOORLIMIT:
			memcpy(pstAidParam->usFloorlimit, stTlvObj[i].pusValue, stTlvObj[i].unValueLen);
			break;
		case _EMVPARAM_DF15_THRESHOLDVA:
			memcpy(pstAidParam->usThresholdValue, stTlvObj[i].pusValue, stTlvObj[i].unValueLen);
			break;
		case _EMVPARAM_DF16_MAXTARPER:
			memcpy(&pstAidParam->ucMaxTargetPercent, stTlvObj[i].pusValue, 1);
			break;
		case _EMVPARAM_DF17_TARGETPER:
			memcpy(&pstAidParam->ucTargetPercent, stTlvObj[i].pusValue, 1);
			break;
		case _EMVPARAM_DF44_DEDDOL:
			memcpy(pstAidParam->usDefaultDdol, stTlvObj[i].pusValue, stTlvObj[i].unValueLen);
			pstAidParam->ucDefaultDdolLen = stTlvObj[i].unValueLen;
			break;
		case _EMVPARAM_DF45_DETDOL:
			memcpy(pstAidParam->usDefaultTdol, stTlvObj[i].pusValue, stTlvObj[i].unValueLen);
			pstAidParam->ucDefaultTdolLen = stTlvObj[i].unValueLen;
			break;
		case _EMVPARAM_9F33_CAP:
			memcpy(pstAidParam->usCap, stTlvObj[i].pusValue, stTlvObj[i].unValueLen);
			break;
		case _EMVPARAM_9F40_ADDCAP:
			memcpy(pstAidParam->usAddCap, stTlvObj[i].pusValue, stTlvObj[i].unValueLen);
			break;
		case _EMVPARAM_DF01_APPSELIND:
			memcpy(&pstAidParam->ucAppSelIndicator, stTlvObj[i].pusValue, 1);
			break;
		case _EMVPARAM_DF24_ICS:
			memcpy(pstAidParam->usIcs, stTlvObj[i].pusValue, stTlvObj[i].unValueLen);
			break;
		case _EMVPARAM_DF27_LIMITEXIST:
			memcpy(&pstAidParam->ucLimitExist, stTlvObj[i].pusValue, 1);
			break;
		case _EMVPARAM_DF20_CLLIMMIT:
			memcpy(pstAidParam->usClLimit, stTlvObj[i].pusValue, stTlvObj[i].unValueLen);
			break;
		case _EMVPARAM_DF19_CLOFFLIMIT:
			memcpy(pstAidParam->usClOfflineLimit, stTlvObj[i].pusValue, stTlvObj[i].unValueLen);
			break;
		case _EMVPARAM_DF21_CVMLIMT:
			memcpy(pstAidParam->usCvmLimit, stTlvObj[i].pusValue, stTlvObj[i].unValueLen);
			break;
		default:
			break;
		}
	}
	return APP_SUCC;
}

static int PrintEmvParam(PRT_AIDPARAM* pstAidParam, int nFlag)
{
	unsigned int nNum = 0;
	//Terminal config
	if(nFlag == 1)
	{
		ASSERT_FAIL(PubPrintTail());
		PubPrePrinter("Terminal Config:");
		PntHex("AID:", pstAidParam->usAid, pstAidParam->ucAidLen);
		PntHex("VER:", pstAidParam->usAppVer, 2);
		PntHex("TAC_Default:", pstAidParam->usTacDefault, 5);
		PntHex("TAC_Decline:", pstAidParam->usTacDenial, 5);
		PntHex("TAC_Online :", pstAidParam->usTacOnline, 5);
		PubC4ToInt(&nNum, pstAidParam->usFloorlimit);
		ASSERT_FAIL(PubPrePrinter("FloorLimit:%d", nNum));
		PubC4ToInt(&nNum, pstAidParam->usThresholdValue);
		ASSERT_FAIL(PubPrePrinter("ThresholdValue:%d", nNum));
		PntHex("MaxTargetPercent:", &pstAidParam->ucMaxTargetPercent, 1);
		PntHex("TargetPercent:", &pstAidParam->ucTargetPercent, 1);
		PntHex("Default DDOL:", pstAidParam->usDefaultDdol, pstAidParam->ucDefaultDdolLen);
		PntHex("Default TDOL:", pstAidParam->usDefaultTdol, pstAidParam->ucDefaultTdolLen);
		PntHex("CAP:", pstAidParam->usCap, 3);
		PntHex("ADDCAP:", pstAidParam->usAddCap, 5);
		PntHex("ASI(PartialAID):", &pstAidParam->ucAppSelIndicator, 1);
		PntHex("ICS:", pstAidParam->usIcs, 7);
		PntHex("_limit_exist:", &pstAidParam->ucLimitExist, 1);
		PntHex("_cl_limit:", pstAidParam->usClLimit, 6);
		PntHex("_cl_offline_limit:", pstAidParam->usClOfflineLimit, 6);
		PntHex("_cvm_limit:", pstAidParam->usCvmLimit, 6);
		ASSERT_FAIL(PubPrintTail());
	}
	else
	{
		ASSERT_FAIL(PubPrintTail());
		PntHex("AID:", pstAidParam->usAid, pstAidParam->ucAidLen);
		ASSERT_FAIL(PubPrintTail());
	}
	return APP_SUCC;
}

static int HandleEmvParamPrt(L3_CARD_INTERFACE interface)
{
	unsigned char tlv_list[1500] = {0};
	PRT_AIDPARAM stAidParam;
	L3_AID_ENTRY lstAidEntry[50];
	int nCount, nTlvLen, i, nRet, nFlag = 0;

	memset(lstAidEntry, 0, sizeof(lstAidEntry));
	nCount = TxnL3EnumEmvConfig(interface, lstAidEntry, 50);
	if(interface == L3_CONTACT)
	{
		ASSERT_FAIL(PubPrePrinter("CONTACT"));
	}
	else
	{
		ASSERT_FAIL(PubPrePrinter("CONTACTLESS"));
	}
	
	if (nCount > 0)
	{
		for (i = 0; i < nCount; i++)
		{	
			memset(tlv_list, 0, sizeof(tlv_list));
			nTlvLen = 0;
			if(memcmp(lstAidEntry[i].aid, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 16) != 0)
			{
				nRet = TxnL3LoadAIDConfig(interface, &lstAidEntry[i], tlv_list, &nTlvLen, CONFIG_GET);
				nFlag = 1;
			}
			else
			{
				nRet = TxnL3LoadTerminalConfig(interface, tlv_list, &nTlvLen, CONFIG_GET);
				nFlag = 0;
			}
			if(nRet == APP_SUCC)
			{
				memset(&stAidParam, 0, sizeof(PRT_AIDPARAM));
				AidParse(tlv_list, nTlvLen, &stAidParam);
				PrintEmvParam(&stAidParam, nFlag);
			}
		}
	}
	else
	{
		ASSERT_FAIL(PubPrePrinter("AID_Num:%d", nCount));
	}
	return APP_SUCC;
}

int  _printemvparam()
{
	int nRet, nCount = 0;
	int i;
	char lstrCapk[50][6];
	L3_CAPK_ENTRY stCapk;

	PubClearAll();
	PubDisplayStrInline(DISPLAY_MODE_CENTER, 3, tr("PRINTING"));
	PubUpdateWindow();

	//Print Emv terminal config and aid data
	HandleEmvParamPrt(L3_CONTACT);
	HandleEmvParamPrt(L3_CONTACTLESS);

	//Print capk 
	memset(lstrCapk, 0, sizeof(lstrCapk));
	nCount = TxnL3EnumCapk(0, 50, lstrCapk);
	ASSERT_FAIL(PubPrePrinter("EnumCAPK=%d", nCount));
	if(nCount > 0)
	{
		for(i=0; i<nCount; i++)
		{
			memset(&stCapk, 0, sizeof(L3_CAPK_ENTRY));
			memcpy(stCapk.rid, lstrCapk[i], 5);
			stCapk.index = lstrCapk[i][5];
			nRet = TxnL3LoadCAPK(&stCapk, CONFIG_GET);
			if(nRet == APP_SUCC)
			{
				ASSERT_FAIL(PubPrePrinter(tr("Index:%02x "), stCapk.index));
				PntHex(tr("RID:"), stCapk.rid, 5);
				PntHex(tr("   ExpDate:"), stCapk.expiredDate, 4);
			}
		}
	}

	if (PubGetPrinter()==_PRINTTYPE_TP)
	{
		ASSERT_FAIL(PubPrePrinter("\r\r" ));
		ASSERT_FAIL(PubPrePrinter("- - - - - - - X - - - - - - - X - - - - - - - "));
	}	
	return APP_SUCC;
}

static int PrintBillHead()
{
	char szShopname[40+1] = {0};
	char szShopAddr[3][80+1] = {{0}};
	char szShopid[15+1] = {0};
	char szTerminalid[8+1] = {0};
	char szTmpStr[80+1] = {0};
	int i = 0;
	
	if (NO == GetIsPntTitleMode())//'0' logo
	{
		//print logo
		PubPrintPicture(0, "PrintLogo.bmp");
		ASSERT(PubSetPrtFont(14, PRN_MODE_HEIGHT_DOUBLE, 0));
	}
	else
	{
		//ASSERT(PubSetPrintFont(PRN_ZM_FONT_12x24A, PRN_HZ_FONT_24x24A, PRN_MODE_HEIGHT_DOUBLE, 0));
		ASSERT(PubSetPrtFont(20, PRN_MODE_HEIGHT_DOUBLE, 0));
		GetFunctionPntTitle(szTmpStr);
		if (0 == strlen(szTmpStr))
		{
			strcpy(szTmpStr, tr("POS RECEIPT"));
		}
		PrePrintInMiddle( szTmpStr, 3);
		//ASSERT(PubSetPrintFont(PRN_ZM_FONT_12x24A, PRN_HZ_FONT_24x24A, PRN_MODE_NORMAL, 0));
		ASSERT(PubSetPrtFont(14, PRN_MODE_HEIGHT_DOUBLE, 0));
	}

	GetVarMerchantNameEn(szShopname);
	ProTrimStr(szShopname);
	PrePrintInMiddle( szShopname, 1);
	for (i = 0; i < 3; i++)
	{
		GetVarMerchantAddr(szShopAddr[i], i);
		if (strlen(szShopAddr[i]))
		{
			ProTrimStr(szShopAddr[i]);
			PrePrintInMiddle( szShopAddr[i], 2+i);
		}
	}

	GetVarTerminalId(szTerminalid);
	GetVarMerchantId(szShopid);
	PubPrePrinter(tr("TID: %s"), szTerminalid);
	PubPrePrinter(tr("MID: %s\r"), szShopid);

	return APP_SUCC;

}

static int PrintAmt(const STTRANSRECORD *pstTransRecord)
{
	char szCurrencyName[3+1] = {0};
	char szStr[128] = {0}, szTmpStr[128] = {0}, szTmp[256] = {0};
	char szDispAmt[DISPAMTLEN] = {0}, szDispTip[DISPAMTLEN] = {0}, szDispTotal[DISPAMTLEN] = {0};
	char szTotalAmt[14] = {0};
	char cTransType = pstTransRecord->cTransType;
	
	GetVarCurrencyName(szCurrencyName);

	memset(szStr, 0, sizeof(szStr));
	memset(szTmpStr, 0, sizeof(szTmpStr));

	if (cTransType == TRANS_CASHBACK)
	{
		PubHexToAsc((uchar *)pstTransRecord->sCashbackAmount, 12, 0,  (uchar *)szTmpStr);
	}
	else
	{
		PubHexToAsc((uchar *)pstTransRecord->sTipAmount, 12, 0,  (uchar *)szTmpStr);
	}
	if ((cTransType == TRANS_SALE && YES == GetVarIsTipFlag() && memcmp(szTmpStr, "000000000000", 12) != 0)
		|| cTransType == TRANS_CASHBACK || cTransType == TRANS_ADJUST)
	{
		PubHexToAsc((uchar *)pstTransRecord->sBaseAmount, 12, 0,  (uchar *)szStr);
		ProAmtToDispOrPnt((char*)szStr, szDispAmt);
		PubAllTrim(szDispAmt);
		sprintf(szTmp, "%s %s", szCurrencyName, szDispAmt);
		PubPrtDoubleStrs(tr("BASE:"), szTmp);
		ProAmtToDispOrPnt((char*)szTmpStr, szDispTip);
		PubAllTrim(szDispTip);
		sprintf(szTmp, "%s %s", szCurrencyName, szDispTip);
		if (cTransType == TRANS_CASHBACK)
		{
			PubPrtDoubleStrs(tr("CASH BACK:"), szTmp);
		}
		else
		{
			PubPrtDoubleStrs(tr("TIP:"), szTmp);
		}

		PubAscAddAsc((uchar *)szStr, (uchar *)szTmpStr, (uchar *)szTotalAmt);
		ProAmtToDispOrPnt((char*)szTotalAmt, szDispTotal);
		PubAllTrim(szDispTotal);
		sprintf(szTmp, "%s %s", szCurrencyName, szDispTotal);
		PubPrtDoubleStrs(tr("TOTAL:"), szTmp);
	}
	else
	{
		PubHexToAsc((uchar *)pstTransRecord->sAmount, 12, 0,  (uchar *)szStr);
		ProAmtToDispOrPnt((char*)szStr, szDispAmt);
		PubAllTrim(szDispAmt);
		CheckAmtMinus(pstTransRecord->cTransType, szDispAmt);
		sprintf(szTmp, "%s %s", szCurrencyName, szDispAmt);
		PubPrtDoubleStrs(tr("TOTAL:"), szTmp);
	}
	
	return APP_SUCC;
}


static int GetPrintAmt(const STTRANSRECORD *pstTransRecord, char *pszDispAmt)
{
	char szStr[128] = {0};
	char szDispAmt[DISPAMTLEN] = {0};
	
	memset(szStr, 0, sizeof(szStr));
	PubHexToAsc((uchar *)pstTransRecord->sAmount, 12, 0,  (uchar *)szStr);

	ProAmtToDispOrPnt((char*)szStr, szDispAmt);
	PubAllTrim(szDispAmt);
	CheckAmtMinus(pstTransRecord->cTransType, szDispAmt);
	strcpy(pszDispAmt, szDispAmt);

	return APP_SUCC;
}

static void CheckAmtMinus(char cTransType, char *pszAmount)
{		
	char szAmt[20] = {0};
	
	if(GetVarIsPrintPrintMinus()==YES)
	{
		switch(cTransType)
		{
		case TRANS_VOID:				
		case TRANS_VOID_AUTHSALE: 			
		case TRANS_VOID_PREAUTH:			
		case TRANS_REFUND:  			
			sprintf(szAmt, "-%s", pszAmount);
			strcpy(pszAmount, szAmt);
			break;
		default:
			break;
		}
	}
}

/**
* @brief Callback func for ISO print
* @param in ptrPara  
* @return APP_SUCC
* @author
* @date 
*/
int _printIsoData(void *pvPara)
{
	int nFieldLen = 0;
	int nBitmapLen = 8;
	int n;
	uchar cBitMask;
	char szTmp[16];
	char szField[999];
	char *psBitmap = (char *)pvPara+2;
	int i = 0, j = 0;
	int t = 0;
	char sMsgId[2] = {0};
	
	memcpy(sMsgId, pvPara, 2);
	ASSERT(PubSetPrtFont(20, PRN_MODE_NORMAL, 0));
	PubPrintTail();
	PubPrePrinter("MSGID: %02x%02x", sMsgId[0], sMsgId[1]);
	if (psBitmap[0] & 0x80)
	{
		nBitmapLen = 16;
	}
	else
	{
		nBitmapLen = 8;
	}
	PntHex(tr("BITMAP:"), (uchar *)psBitmap, nBitmapLen);
	n = 0 ;
	for (i = 0; i < nBitmapLen; i++)
	{
		cBitMask = 0x80 ;
		for(j=1; j<9; j++, cBitMask>>=1)
		{
			if (0 == (psBitmap[i] & cBitMask)) 
			{
				continue;
			}
			n = (i<<3) + j;
			nFieldLen = 999;
			memset(szField, 0, sizeof(szField));
			ASSERT_FAIL(GetField(n, szField, &nFieldLen));
			
			if (n == 55)
			{
				tlv_t stTlvObj[30]; 
				int nRet = 0;

				memset(stTlvObj, 0, sizeof(stTlvObj));
				nRet = TlvParse((uchar *)szField, nFieldLen, stTlvObj, 30);
				if (nRet != 0)
				{
					PubDebug("[%s][%d] nRet = %d\n", __func__, __LINE__, nRet);
					return APP_FAIL;
				}
				PubPrePrinter("[%d]  FieldLen = %d", n, nFieldLen);
				for (t = 1; t <= stTlvObj[0].nChildNum; t++)
				{
					memset(szTmp, 0, sizeof(szTmp));
					sprintf(szTmp, "     %-4x [%02d] ", stTlvObj[t].unTagName, stTlvObj[t].unValueLen);
					PntHex(szTmp, (uchar *)stTlvObj[t].pusValue, stTlvObj[t].unValueLen);
				}
			}
			else
			{
				PubPrePrinter("[%02d]%s", n, szField);
			}
		}
	}
	PubPrePrinter("\r\r");

	return APP_SUCC;
}

/**
* @brief Print ISO
* @param in pszData iso pointer
* @return 
* @li APP_SUCC 
* @li APP_FAIL
* @author 
* @date 
*/
int PrintIsoData(const char *pszData)
{
 	ASSERT_QUIT(PubPrintCommit(_printIsoData, (void *)pszData, 1));		
 	return APP_SUCC; 
}

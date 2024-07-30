/***************************************************************************
** Copyright (c) 2019 Newland Payment Technology Co., Ltd All right reserved   
** File name:  batch.c
** File indentifier: 
** Synopsis:  Processing translog
** Current Verion:  v1.0
** Auther: sunh
** Complete date: 2016-9-5
** Modify record: 
** Modify date: 
** Version: 
** Modify content: 
***************************************************************************/
#include "apiinc.h"
#include "libapiinc.h"
#include "appinc.h"

static int gnCurrentRecNo=0;	

/**
* @brief Create or clean translog file
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int InitBatchFile(void)
{
	STRECFILE stTransRecordFile;
	char szCmd[32] = {0};

	strcpy( stTransRecordFile.szFileName, FILE_RECORD);
	stTransRecordFile.cIsIndex = FILE_NOCREATEINDEX;				
	stTransRecordFile.unMaxOneRecLen = PER_TRANSRECORD_LEN;
	stTransRecordFile.unIndex1Start = 1;
	stTransRecordFile.unIndex1Len =  1;
	stTransRecordFile.unIndex2Start =  1;
	stTransRecordFile.unIndex2Len = 1;
	gnCurrentRecNo = 0;
	ASSERT_FAIL(PubCreatRecFile(&stTransRecordFile));

	sprintf(szCmd, "rm %s/*.bmp", ELECSIGN_PATH);
	system(szCmd);

	return APP_SUCC;
}

/**
* @brief Append translog
* @param in const STTRANSRECORD *pstTransRecord 
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int AppendTransRecord(const STTRANSRECORD *pstTransRecord)
{	
	char szBuf[PER_TRANSRECORD_LEN] = {0};

	PubGetRecSum(FILE_RECORD, &gnCurrentRecNo);
	ASSERT_FAIL(StRecordToRecordBuf(pstTransRecord, szBuf));
	ASSERT_FAIL(PubAddRec(FILE_RECORD, szBuf));
	gnCurrentRecNo++;
	return APP_SUCC;
}

/**
* @brief Update transRec (recNo = gnCurrentRecNo)
* @param in const STTRANSRECORD *pstTransRecord
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int UpdateTransRecord(const STTRANSRECORD *pstTransRecord)
{
	char szBuf[PER_TRANSRECORD_LEN] = {0};

	ASSERT_FAIL(StRecordToRecordBuf(pstTransRecord, szBuf));
	return PubUpdateRec (FILE_RECORD, gnCurrentRecNo, szBuf);
}

/**
* @brief Update transRec by handle
* @param in const STTRANSRECORD *pstTransRecord
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int UpdateRecByHandle (int nFileHandle, const int nRecNo, STTRANSRECORD *pstTransRecord)
{
	char szBuf[PER_TRANSRECORD_LEN] = {0};

	ASSERT_FAIL(StRecordToRecordBuf(pstTransRecord, szBuf));
	return PubUpdateRecByHandle(nFileHandle, nRecNo, szBuf);
}

int ReadTransRecord(const int nFileHandle, const int nRecNo, STTRANSRECORD *pstTransRecord)
{
	char szBuf[PER_TRANSRECORD_LEN] = {0};

	ASSERT_FAIL(PubReadRec(nFileHandle, nRecNo, szBuf));
	RecordBufToStRecord(szBuf, pstTransRecord);

	return APP_SUCC;
}
 /**
 * @brief Find translog with ref number
 * @param in const char *szRefno no
 * @param in pstTransRecord
 * @return
 * @li APP_SUCC
 * @li APP_FAIL
 */
int FindRecordWithTagid(int nTagid, char *pszValue, STTRANSRECORD *pstTransRecord)
{
	char szBuf[PER_TRANSRECORD_LEN] = {0};
	char szOldValue[254+1] = {0};
	int nRecNum, nFileHandle, nLoop;
	int nRet, nLen;

	GetRecordNum(&nRecNum);
	if (nRecNum > 0)
	{
		nRet = PubOpenFile(FILE_RECORD, "r", &nFileHandle);
		ASSERT(nRet);
		if (nRet != APP_SUCC)
		{
			TRACE("nRet = %d", nRet);
			return APP_FAIL;
		}
	}
	else
	{
		TRACE("nRecNum = %d", nRecNum);
		return APP_FAIL;
	}

	for (nLoop = 1; nLoop <= nRecNum; nLoop++)
	{
		nRet = PubReadRec(nFileHandle, nLoop, szBuf);
		if (nRet != APP_SUCC)
		{
			TRACE("nRet = %d", nRet);
			PubCloseFile(&nFileHandle);
			return APP_FAIL;
		}
		if (GetRecordTag(szBuf, nTagid, &nLen, szOldValue) != APP_SUCC)
		{
			PubCloseFile(&nFileHandle);
			return APP_FAIL;		
		}
		if (memcmp(szOldValue, pszValue, nLen) == 0)
		{
			RecordBufToStRecord(szBuf, pstTransRecord);
			PubCloseFile(&nFileHandle);
			gnCurrentRecNo = nLoop;
			return APP_SUCC;
		}
	}
	PubCloseFile(&nFileHandle);

	return APP_FAIL;
}

 /**
* @brief Find translog with trace number and display in the screen
* @param in int PrintFlag  if equal to 1, print the translog
* @return
* @li APP_SUCC		
* @li APP_FAIL
*/
int FindByInvoice(int PrintFlag)
{
	int nRet, nLen;
	STTRANSRECORD stTransRecord;
	char szInvno[6+1];
	char sTrace[3] = {0};
	char szDispTitle[32] = {0};

	if (PrintFlag)
	{
		strcpy(szDispTitle, tr("PRINT ANY"));
	}
	else
	{
		strcpy(szDispTitle, tr("VIEW APPOINTED"));
	}

	nLen = 6;
	memset(szInvno, 0, sizeof(szInvno));
	ASSERT_QUIT(PubInputDlg(szDispTitle, tr("TRACE NO:"), szInvno, &nLen, 1, 6, 0, INPUT_MODE_NUMBER));	
	PubAddSymbolToStr(szInvno, 6, '0', 0);
	PubAscToHex((uchar *)szInvno, 6, 0, (uchar *)sTrace);	
	nRet = FindRecordWithTagid(TAG_RECORD_TRACE, sTrace, &stTransRecord);
	if (nRet==APP_FAIL)
	{
		char szDispBuf[100];
		
		PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szDispBuf, "|CNO TRANS");
		PubMsgDlg(szDispTitle, szDispBuf, 0, 5);
		return APP_FAIL;		
	}
	else
	{
		nRet = DispTransRecord(&stTransRecord, VIEWRECORD_ATTR_DETAIL);
		if(nRet != KEY_ENTER)
		{
			return APP_QUIT;
		}
		
		if (PrintFlag)
		{	
			nRet = PubConfirmDlg(szDispTitle, tr("PRINT RECIEPT?"), 0, 50);
			if (nRet == APP_SUCC)
			{
				PubClearAll();
				DISP_PRINTING_NOW;
				PrintRecord(&stTransRecord, REPRINT);
			}
		}
	}
	return APP_SUCC;
}

 /**
* @brief Find sign pic with trace number and display in the screen
* @param
* @return
* @li APP_SUCC		
* @li APP_FAIL
*/
int FindSignpicByTrace()
{
	int nLen;
	char szInvno[6+1];
	char sTrace[3] = {0};
	char szDispTitle[32] = {0};
	char szFileName[64] = {0};

	strcpy(szDispTitle, tr("VIEW Sign"));

	nLen = 6;
	memset(szInvno, 0, sizeof(szInvno));
	ASSERT_QUIT(PubInputDlg(szDispTitle, tr("TRACE NO:"), szInvno, &nLen, 1, 6, 0, INPUT_MODE_NUMBER));	
	PubAddSymbolToStr(szInvno, 6, '0', 0);
	PubAscToHex((uchar *)szInvno, 6, 0, (uchar *)sTrace);	
	PubClearAll();
	sprintf(szFileName, "%s/%s.bmp", ELECSIGN_PATH, szInvno);
	if (PubFsExist(szFileName) == NAPI_ERR) {
		memset(szDispTitle, 0, sizeof(szDispTitle));
		sprintf(szDispTitle, "%s is not exist", szFileName);
		PubMsgDlg("warning", szDispTitle, 1, 30);
		return APP_QUIT;
	}
	PubDisplayLogo(szFileName, 0, 30);
	PubUpdateWindow();
	PubWaitConfirm(30);

	return APP_SUCC;
}


/**
* @brief Get current translog
* @param out STTRANSRECORD *pstTransRecord
* @return
* @li APP_SUCC		
* @li APP_FAIL
*/
int FetchCurrentRecord(STTRANSRECORD *pstTransRecord)
{
	char szBuf[PER_TRANSRECORD_LEN] = {0};

	ASSERT_FAIL(PubReadOneRec(FILE_RECORD,gnCurrentRecNo, (char *)szBuf));
	RecordBufToStRecord(szBuf, pstTransRecord);

	return APP_SUCC;
}

/**
* @brief Get first translog
* @param out STTRANSRECORD *pstTransRecord
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int FetchFirstRecord(STTRANSRECORD *pstTransRecord)
{
	gnCurrentRecNo = 1;
	return FetchCurrentRecord(pstTransRecord);
}

/**
* @brief Get next translog
* @param out STTRANSRECORD *pstTransRecord
* @return
* @li APP_SUCC		
* @li APP_FAIL
*/
int FetchNextRecord(STTRANSRECORD *pstTransRecord)
{
	int nRecNum;

	PubGetRecSum(FILE_RECORD, &nRecNum );
	gnCurrentRecNo++;
	if( gnCurrentRecNo > nRecNum )
	{
		gnCurrentRecNo = nRecNum;
	}
	return FetchCurrentRecord(pstTransRecord);
}

/**
* @brief Get previous translog
* @param out STTRANSRECORD *pstTransRecord
* @return
* @li APP_SUCC		
* @li APP_FAIL
*/
int FetchPreviousRecord(STTRANSRECORD *pstTransRecord)
{
	gnCurrentRecNo--;
	if( gnCurrentRecNo ==  0 )
	{
		gnCurrentRecNo = 1;
	}
	return FetchCurrentRecord(pstTransRecord);
}

/**
* @brief Get last translog
* @param out STTRANSRECORD *pstTransRecord
* @return
* @li APP_SUCC		
* @li APP_FAIL
*/
int FetchLastRecord(STTRANSRECORD *pstTransRecord)
{
	PubGetRecSum(FILE_RECORD, &gnCurrentRecNo );
	return FetchCurrentRecord(pstTransRecord);
}

int DispTransRecord(const STTRANSRECORD *pstTransRecord, EM_VIEWRECORD_ATTR emAttr)
{
	char szDispBuf[256], szContent[512], szTemp[256];
	char szZero[12+1];
	int nKey, nRet = 0;
	int i;

	/**
	* Show Trans type
	*/
	switch(pstTransRecord->cTransType)
	{
	case TRANS_SALE:			
		sprintf(szContent, tr("SALE:"));
		break;
	case TRANS_CASHBACK:			
		sprintf(szContent, tr(" CASHBACK:"));
		break;
	case TRANS_AUTHCOMP:			
		sprintf(szContent, tr("AUTH COMP:"));
		break;
	case TRANS_PREAUTH:		
		sprintf(szContent, tr("AUTH:"));
		break;
	case TRANS_REFUND:				
		sprintf(szContent, tr("REFUND:"));
		break;
	case TRANS_VOID:		
		sprintf(szContent, tr("VOID:"));
		break;
	case TRANS_VOID_AUTHSALE:		
		sprintf(szContent, tr("VOID AUTH COMP:"));
		break;
	case TRANS_VOID_PREAUTH:	
		sprintf(szContent, tr("VOID AUTH:"));
		break;
	case TRANS_OFFLINE:			
		sprintf(szContent, tr("OFFLINE:"));
		break;
	case TRANS_ADJUST:			
		sprintf(szContent, tr("ADJUST:"));
		break;
	default:
		sprintf(szContent, "UNKNOW:%d",pstTransRecord->cTransType);
		break;
	}

	switch(pstTransRecord->cStatus)
	{
	case 3:		
		break;
	case 1:	
		PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent, "%s|R(VOID)", szContent);
		break;
	case 2:
		break;
	default:	
		switch(pstTransRecord->cTransType)
		{
		case TRANS_VOID:
		case TRANS_VOID_AUTHSALE:
		case TRANS_ADJUST:	
			if(pstTransRecord->cSendFlag < 0xFD)
			{	
				PubHexToAsc((uchar *)pstTransRecord->sOldTrace, 6, 0, (uchar *)szDispBuf);
				PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent, "%s|R%6.6s", szContent,szDispBuf);
	    	}
			break;
	    	default:	
			break;
		}
		break;
	}
	if(pstTransRecord->cTransType == TRANS_ADJUST && pstTransRecord->cSendFlag >= 0xFD)
	{
		PubHexToAsc((uchar *)pstTransRecord->sOldTrace, 6, 0, (uchar *)szDispBuf);
		PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent, "%s|R%6.6s(SENDED)", szContent,szDispBuf);
	}
	else if(pstTransRecord->cSendFlag >= 0xFD && (pstTransRecord->cStatus == 2 || pstTransRecord->cStatus == 4))
	{
		PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent, "%s|R(ADJUSTED SENDED)", szContent);
	}
	else if(pstTransRecord->cSendFlag >= 0xFD)
	{
		PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent, "%s|R(SENDED)", szContent);
	}
	else if(pstTransRecord->cStatus == 2 || pstTransRecord->cStatus == 4)
	{
		PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent, "%s|R(ADJUSTED)", szContent);
	}

	/*------Card no--------*/
	memset(szDispBuf, 0, sizeof(szDispBuf));
	PubHexToAsc((uchar *)pstTransRecord->sPan, pstTransRecord->nPanLen, 0, (uchar *)szDispBuf);
	CtrlCardPan((uchar*)szDispBuf, (uint)pstTransRecord->nPanLen, (uint)pstTransRecord->cTransType, pstTransRecord->cEMV_Status);
	sprintf(szContent+strlen(szContent), "\n%s", szDispBuf);	
	/*------date time--------*/
	sprintf(szContent+strlen(szContent), "\n%02x/%02x %02x:%02x", pstTransRecord->sDate[0], pstTransRecord->sDate[1], pstTransRecord->sTime[0], pstTransRecord->sTime[1]);
	/*------trace no-------------*/
	PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent+strlen(szContent), "\nTRACE:%02x%02x%02x", pstTransRecord->sTrace[0], pstTransRecord->sTrace[1], pstTransRecord->sTrace[2]);	

	/*Amt*/
	memset(szDispBuf,0,sizeof(szDispBuf));
	memset(szTemp,0,sizeof(szTemp));
	PubHexToAsc((uchar *)pstTransRecord->sAmount, 12, 0, (uchar *)szDispBuf);
	sprintf(szTemp,"%10.10s.%2.2s",szDispBuf,szDispBuf+10);
	for (i=0; i<9; i++)
	{
		if (szTemp[i] == '0')
		{
			szTemp[i] = ' ';
		}
		else
		{
			break;
		}
	}
	PubLeftTrim(szTemp);
	sprintf(szContent+strlen(szContent), "\nAMT:%s",szTemp);

	/*Tip*/
	memset(szDispBuf, 0, sizeof(szDispBuf));
	memset(szTemp, 0, sizeof(szTemp));
	memset(szZero, 0x30, sizeof(szZero)-1);
	szZero[12]='\0';
	PubHexToAsc((uchar *)pstTransRecord->sTipAmount, 12, 0, (uchar *)szDispBuf);	
	if(strncmp(szDispBuf, szZero, 12) > 0)
	{
		sprintf(szTemp,"%10.10s.%2.2s",szDispBuf,szDispBuf+10);
		for (i=0; i<9; i++)
		{
			if (szTemp[i] == '0')
			{
				szTemp[i] = ' ';
			}
			else
			{
				break;
			}
		}
		PubLeftTrim(szTemp);
		sprintf(szContent+strlen(szContent), "\nTIP:%s",szTemp);
	}

	/*Ref number*/
	sprintf(szContent+strlen(szContent), "\nREF:%12.12s",pstTransRecord->szRefnum);
	/*------auth code-----------------*/	
	PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent+strlen(szContent), "\nAUTH:%6.6s", pstTransRecord->szAuthCode);
	if (PubGetKbAttr() == KB_VIRTUAL)
	{
		if (emAttr == VIEWRECORD_ATTR_DETAIL)
		{
			return PubUpDownMsgDlg(NULL, szContent, NO, 60, NULL);
		} 
		else if (emAttr == VIEWRECORD_ATTR_ONEBYONE) 
		{
			nRet = PubUpDownMsgDlg(NULL, szContent, NO, 60, &nKey);
			if(nRet == APP_QUIT) {
				return KEY_ESC;
			}
			return nKey;
		}
		return PubUpDownMsgDlg(NULL, szContent, NO, 60, NULL);
	}
	
	nRet = PubUpDownMsgDlg(NULL, szContent, NO, 60, &nKey);	
	if(nRet == APP_QUIT)
		return KEY_ESC;
	if(nRet == APP_FUNCQUIT && nKey == KEY_ENTER)
	{
		nRet = PubUpDownMsgDlg(NULL, szContent, YES, 60, &nKey);
		nKey = KEY_ENTER;
	}
	return nKey;
}

/**
* @brief Display simple transaction info
* @param in STTRANSRECORD *pstTransRecord
* @return
* @li APP_SUCC		
* @li APP_FAIL
*/
int DispRecordInfo(const char * pszTitle, const STTRANSRECORD *pstTransRecord)
{
	char szDispBuf[30], szAmount[13];
	char szContent[500];
	char szDispAmt[DISPAMTLEN] = {0};
	int nKey =0;
	
	//Card
	memset(szDispBuf, 0, sizeof(szDispBuf));
	PubHexToAsc((uchar *)pstTransRecord->sPan, pstTransRecord->nPanLen, 0, (uchar *)szDispBuf);
	CtrlCardPan((uchar*)szDispBuf, (uint)pstTransRecord->nPanLen, (uint)pstTransRecord->cTransType, pstTransRecord->cEMV_Status);
	PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent, "%s", szDispBuf);
	//Trace no
	PubHexToAsc((uchar *)pstTransRecord->sTrace, 6, 0, (uchar *)szDispBuf);
	PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent+strlen(szContent), "\nTRACE NO:|R%s", szDispBuf);

	//Amt
	if (pstTransRecord->cTransType == TRANS_CASHBACK)
	{
		memset(szAmount, 0, sizeof(szAmount));
		PubHexToAsc((uchar *)pstTransRecord->sBaseAmount, 12, 0, (uchar *)szAmount);	
		ProAmtToDispOrPnt(szAmount, szDispAmt);
		PubAllTrim(szDispAmt);
		PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent+strlen(szContent), "\nAMOUNT:|R%s", szDispAmt);

		memset(szAmount, 0, sizeof(szAmount));
		PubHexToAsc((uchar *)pstTransRecord->sCashbackAmount, 12, 0, (uchar *)szAmount);	
		ProAmtToDispOrPnt(szAmount, szDispAmt);
		PubAllTrim(szDispAmt);
		PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent+strlen(szContent), "\nCASHBACK:|R%s", szDispAmt);
	}
	else if (pstTransRecord->cTransType == TRANS_SALE && memcmp(pstTransRecord->sTipAmount, "\x00\x00\x00\x00\x00\x00", 6) != 0)
	{
		memset(szAmount, 0, sizeof(szAmount));
		PubHexToAsc((uchar *)pstTransRecord->sBaseAmount, 12, 0, (uchar *)szAmount);	
		ProAmtToDispOrPnt(szAmount, szDispAmt);
		PubAllTrim(szDispAmt);
		PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent+strlen(szContent), "\nAMOUNT:|R%s", szDispAmt);

		memset(szAmount, 0, sizeof(szAmount));
		PubHexToAsc((uchar *)pstTransRecord->sTipAmount, 12, 0, (uchar *)szAmount);	
		ProAmtToDispOrPnt(szAmount, szDispAmt);
		PubAllTrim(szDispAmt);
		PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent+strlen(szContent), "\nTIP:|R%s", szDispAmt);
	}
	else
	{
		memset(szAmount, 0, sizeof(szAmount));
		PubHexToAsc((uchar *)pstTransRecord->sAmount, 12, 0, (uchar *)szAmount);	
		ProAmtToDispOrPnt(szAmount, szDispAmt);
		PubAllTrim(szDispAmt);
		PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent+strlen(szContent), "\nAMOUNT:|R%s", szDispAmt);
	}

	//Ref number
	PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent+strlen(szContent),"\nREF:|R%s", pstTransRecord->szRefnum);
	//auth code
	if (strlen(pstTransRecord->szAuthCode) > 0)
	{
		PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent+strlen(szContent),"\nAUTH CODE:|R%s", pstTransRecord->szAuthCode);
	}

	nKey = PubUpDownMsgDlg(pszTitle, szContent, YES, 30, NULL);
	if(nKey == APP_SUCC)
	{
		return APP_SUCC;
	}
	else
	{
	  	return APP_FAIL;
	}
}

/**
* @brief Look up translog in the screen
* @return
* @li APP_SUCC		
* @li APP_FAIL
*/
int RecordOneByOne(void)
{
	int nRet, nRecNum;
	STTRANSRECORD stTransRecord;

	GetRecordNum(&nRecNum);
	if (nRecNum==0) 
	{
		char szDispBuf[100];
		PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szDispBuf, "|CNO TRANS");
		PubMsgDlg("LOOK UP", szDispBuf, 0, 5);
		return APP_SUCC;
	}
	
	ASSERT(FetchLastRecord(&stTransRecord));
	while(1)
	{
		nRet = DispTransRecord(&stTransRecord, VIEWRECORD_ATTR_ONEBYONE);
		switch(nRet)
		{
		case KEY_STAR:
		case KEY_LEFT:
		case KEY_ENTER:
			ASSERT(FetchPreviousRecord(&stTransRecord));
			break;
		case KEY_RIGHT:
		case KEY_SHARP:
			ASSERT(FetchNextRecord(&stTransRecord));
			break;
		case 0:
		case KEY_ESC:
			return APP_QUIT;
		default:
			break;
		}
	}
	return APP_SUCC;
}

/**
* @brief Record structure to system structure
* @param in STTRANSRECORD *pstTransRecord
* @param out  STSYSTEM *pstSystem 
* @return
* @li APP_SUCC		
* @li APP_FAIL
*/
int RecordToSys(const STTRANSRECORD *pstTransRecord, STSYSTEM *pstSystem)
{
	pstSystem->cTransType = pstTransRecord->cTransType;
	pstSystem->cTransAttr = pstTransRecord->cTransAttr;
	pstSystem->cEMV_Status = pstTransRecord->cEMV_Status;
	PubHexToAsc((uchar *)pstTransRecord->sPan, pstTransRecord->nPanLen, 0, (uchar *)pstSystem->szPan);
	PubHexToAsc((uchar *)pstTransRecord->sAmount, 12, 0, (uchar *)pstSystem->szAmount);
	PubHexToAsc((uchar *)pstTransRecord->sTrace, 6, 0, (uchar *)pstSystem->szTrace);
	PubHexToAsc((uchar *)pstTransRecord->sTime, 6, 0, (uchar *)pstSystem->szTime);
	PubHexToAsc((uchar *)pstTransRecord->sDate, 4, 0, (uchar *)pstSystem->szDate);
	if (memcmp(pstTransRecord->sExpDate, "\x00\x00", 2) == 0)
	{
		;
	}
	else
	{
		PubHexToAsc((uchar *)pstTransRecord->sExpDate, 4, 0, (uchar *)pstSystem->szExpDate);
	}
	strcpy(pstSystem->szInputMode, pstTransRecord->szInputMode);
	strcpy(pstSystem->szCardSerialNo, pstTransRecord->szCardSerialNo);
	strcpy(pstSystem->szRefnum, pstTransRecord->szRefnum);
	PubHexToAsc((uchar *)pstTransRecord->sTrack2, pstTransRecord->nTrack2Len, 0, (uchar *)pstSystem->szTrack2);
	PubHexToAsc((uchar *)pstTransRecord->sTrack3, pstTransRecord->nTrack3Len, 0, (uchar *)pstSystem->szTrack3);
	strcpy(pstSystem->szAuthCode, pstTransRecord->szAuthCode);
	strcpy(pstSystem->szResponse, pstTransRecord->szResponse);
	PubHexToAsc((uchar *)pstTransRecord->sBatchNum, 6, 0, (uchar *)pstSystem->szBatchNum);
	strcpy(pstSystem->szOldAuthCode, pstTransRecord->szOldAuthCode);
	strcpy(pstSystem->szOldRefnum, pstTransRecord->szRefnum);
	PubHexToAsc((uchar *)pstTransRecord->sTipAmount, 12, 0, (uchar *)pstSystem->szTipAmount); 
	PubHexToAsc((uchar *)pstTransRecord->sCashbackAmount, 12, 0, (uchar *)pstSystem->szCashbackAmount); 
	memcpy(pstSystem->szServerCode, pstTransRecord->szServerCode, 2);

	pstSystem->nAddFieldLen = pstTransRecord->nField55Len;
	pstSystem->psAddField = (char *)pstTransRecord->sField55;
	
	return APP_SUCC;
}

/**
* @brief system structure to record structure 
* @param in  STSYSTEM *pstSystem 
* @param out STTRANSRECORD *pstTransRecord
* @return
* @li APP_SUCC		
* @li APP_FAIL
*/
int SysToRecord(const STSYSTEM *pstSystem, STTRANSRECORD *pstTransRecord )
{
	pstTransRecord->cTransType = pstSystem->cTransType;
	pstTransRecord->cTransAttr = pstSystem->cTransAttr;
	pstTransRecord->cEMV_Status = pstSystem->cEMV_Status;
	TRACE("pstTransRecord->cEMV_Status=%d",pstTransRecord->cEMV_Status);
	pstTransRecord->nPanLen = strlen(pstSystem->szPan);
	PubAscToHex((uchar *)pstSystem->szPan, pstTransRecord->nPanLen, 0, (uchar *)pstTransRecord->sPan);
	PubAscToHex((uchar *)pstSystem->szAmount, 12, 0, (uchar *)pstTransRecord->sAmount);
	PubAscToHex((uchar *)pstSystem->szTipAmount, 12, 0, (uchar *)pstTransRecord->sTipAmount);
	PubAscToHex((uchar *)pstSystem->szCashbackAmount, 12, 0, (uchar *)pstTransRecord->sCashbackAmount);
	PubAscToHex((uchar *)pstSystem->szBaseAmount, 12, 0, (uchar *)pstTransRecord->sBaseAmount);
	PubAscToHex((uchar *)pstSystem->szTrace, 6, 0, (uchar *)pstTransRecord->sTrace);
	PubAscToHex((uchar *)pstSystem->szTime, 6, 0, (uchar *)pstTransRecord->sTime);
	PubAscToHex((uchar *)pstSystem->szDate, 4, 0, (uchar *)pstTransRecord->sDate);
	PubAscToHex((uchar *)pstSystem->szExpDate, 4, 0, (uchar *)pstTransRecord->sExpDate);
	strcpy(pstTransRecord->szInputMode, pstSystem->szInputMode );
	strcpy(pstTransRecord->szCardSerialNo, pstSystem->szCardSerialNo);
	memcpy(pstTransRecord->szServerCode, pstSystem->szServerCode, 2);
	pstTransRecord->nTrack2Len = strlen(pstSystem->szTrack2);
	PubAscToHex((uchar *)pstSystem->szTrack2, pstTransRecord->nTrack2Len, 0, (uchar *)pstTransRecord->sTrack2);
	pstTransRecord->nTrack3Len = strlen(pstSystem->szTrack3);
	PubAscToHex((uchar *)pstSystem->szTrack3, pstTransRecord->nTrack3Len, 0, (uchar *)pstTransRecord->sTrack3);
	strcpy(pstTransRecord->szRefnum, pstSystem->szRefnum);
	strcpy(pstTransRecord->szAuthCode, pstSystem->szAuthCode);
	strcpy(pstTransRecord->szResponse, pstSystem->szResponse);
	PubAscToHex((uchar *)(pstSystem->szBatchNum), 6, 0, (uchar *)pstTransRecord->sBatchNum);
	strcpy(pstTransRecord->szOldAuthCode, pstSystem->szOldAuthCode);
	strcpy(pstTransRecord->szOldRefnum, pstSystem->szOldRefnum);
	memcpy(pstTransRecord->szOldDate, pstSystem->szOldDate, 4);
	PubAscToHex((uchar *)pstSystem->szOldTrace, 6, 0, (uchar *)pstTransRecord->sOldTrace);
	pstTransRecord->cStatus = 0;
	pstTransRecord->cSendFlag = 0;
	pstTransRecord->cBatchUpFlag= 0;

	if (strlen(pstSystem->szHolderName) > 0)
	{
		memcpy(pstTransRecord->szHolderName, pstSystem->szHolderName, strlen(pstSystem->szHolderName));
	}
	pstTransRecord->nField55Len = pstSystem->nAddFieldLen;
	memcpy(pstTransRecord->sField55, pstSystem->psAddField, pstSystem->nAddFieldLen);
	pstTransRecord->cPrintFlag = pstSystem->cPrintFlag;
	pstTransRecord->cPinAndSigFlag = pstSystem->cPinAndSigFlag;

	return APP_SUCC;
}


/**
* @brief Get translog number
* @param out int *pnRecordNum
* @return void
*/
void GetRecordNum( int *pnRecordNum)
{
	PubGetRecSum(FILE_RECORD, pnRecordNum);
}

int GetFailSendNum(char cFlag)
{
	STTRANSRECORD stTransRecord;
	int nRecNum;
	int i;
	int nFileHandle;
	int nFailSendSum = 0;
	GetRecordNum(&nRecNum);
	ASSERT_FAIL(PubOpenFile(FILE_RECORD, "r", &nFileHandle));
	for(i=1; i<= nRecNum;i++)
	{
		ReadTransRecord(nFileHandle, i, &stTransRecord);
		switch(stTransRecord.cTransType)
		{
		case TRANS_ADJUST:
		case TRANS_OFFLINE:
			if(stTransRecord.cSendFlag== cFlag)
				break;
			else
				continue;
		case TRANS_SALE:
			if((stTransRecord.cEMV_Status == EMV_STATUS_OFFLINE_SUCC) && (stTransRecord.cSendFlag==cFlag))
				break;
			else
				continue;
		default:
			continue;
		}
		nFailSendSum++;
	}
	PubCloseFile(&nFileHandle);
	return nFailSendSum;
}

int GetTcNum(int *pnTcNum)
{
	STTRANSRECORD stTransRecord;
	int nRecNum;
	int i;
	int nFileHandle;
	int nTcNum = 0;
	
	GetRecordNum(&nRecNum);
	ASSERT_FAIL(PubOpenFile(FILE_RECORD, "r", &nFileHandle));
	
	for(i=1; i<= nRecNum;i++)
	{
		ReadTransRecord(nFileHandle, i, &stTransRecord);
		if (memcmp(stTransRecord.szInputMode, INPUTMODE_IC_RFID, 2) == 0
			||memcmp(stTransRecord.szInputMode, INPUTMODE_IC_INSERT, 2) == 0)
		{
			nTcNum++;
		}
	}
	PubCloseFile(&nFileHandle);
	*pnTcNum = nTcNum;
	return APP_SUCC;
}


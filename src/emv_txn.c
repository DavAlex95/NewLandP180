/***************************************************************************
** Copyright (c) 2019 Newland Payment Technology Co., Ltd All right reserved   
** File name:  emv_txn.c
** File indentifier: 
** Brief:  EMV Transaction processing module
** Current Verion:  v1.0
** Auther: sunh
** Complete date: 2016-9-20
** Modify record: 
** Modify date: 
** Version: 
** Modify content: 
***************************************************************************/
#include "apiinc.h"
#include "libapiinc.h"
#include "appinc.h"
#include "napi_smartcard.h"

#define IC1_EXIST		0x01	/**<bit0:If the IC card has been inserted,turn 1*/
#define IC1_POWERON		0x02	/**<bit1:if IC card is on power,turn 1*/
#define IC2_POWERON		0x04	/**<bit2:maintain,return*/
#define IC3_POWERON		0x08	/**<bit3:maintain,return*/
#define SAM1_POWERON	0x10	/**<bit4:if SAM card 1 is on power,turn 1*/
#define SAM2_POWERON	0x20	/**<bit5:if SAM card 2 is on power,turn 1*/
#define SAM3_POWERON	0x40	/**<bit6:if SAM card 3 is on power,turn 1*/
#define SAM4_POWERON	0x80	/**<bit7:if SAM card 4 is on power,turn 1*/

/**
* define global variable
*/
static char gs9F26_RQ[8+1];


static int ProGetICStatus(char *pcStatus)
{
	int nRet, nStatus;

	nRet = NAPI_SCGetStatus(&nStatus);
	if (nRet != NAPI_OK)
	{
		return APP_FAIL;
	}

	PubDebugSelectly(1, "[CARD LIB][%s][%s][%d]>>>NAPI_SCGetStatus nIcState=%d,nRet=%d", __FILE__, __FUNCTION__, __LINE__, nStatus, nRet);
	nRet = nStatus & 0x0B;
	switch(nRet)
	{
	case IC1_EXIST:
	case IC1_EXIST | IC1_POWERON:
	case SAM1_POWERON:
	case SAM2_POWERON:
	case SAM3_POWERON:
	case SAM4_POWERON:
		*pcStatus = nRet;
		return APP_SUCC;
	default:
		return APP_FAIL;
	}
	return APP_FAIL;
}

int ProGetCardStatus()
{
	int nRet;
	char cStatus;
	// only use for reminding Inserted card
	nRet = ProGetICStatus(&cStatus);
	if( nRet == APP_SUCC && (cStatus & IC1_EXIST))
	{
		return APP_SUCC;
	}

	return APP_FAIL;
}
/**
* @brief Wait for Cardholder Get their Card
* @param in pszTitle
* @param in pszContent2
* @param in pszContent3
* @return 
* @li APP_SUCC
* @li APP_QUIT
*/
int DispOutICC(char *pszTitle, char *pszContent2, char *pszContent3)
{

	if(ProGetCardStatus() == APP_SUCC)
	{
		PubClear2To4();
		if (NULL != pszTitle)
		{	
			PubDisplayTitle(pszTitle);
		}
		if (NULL != pszContent2)
		{
			PubDisplayStrInline(DISPLAY_MODE_CENTER, 2, "%s", pszContent2);
		}
		if (NULL != pszContent3)
		{
			PubDisplayStrInline(DISPLAY_MODE_CENTER, 3, "%s", pszContent3);
		}
		PubDisplayGen(5, tr("PLEASE REMOVE CARD"));
	    PubUpdateWindow();
		PubKbHit();
		while (1)
		{
			if (ProGetCardStatus() == APP_FAIL)
			{
				break;
			}

			PubBeep(1);
			PubSysDelay(1);
		}	
	}
	else if ((NULL != pszContent2 && strcmp(pszContent2, "")) || (NULL != pszContent3 && strcmp(pszContent3, "")))
	{
		char szContent[64+1];
		
		memset(szContent, 0, sizeof(szContent));
		if(NULL != pszContent2)
		{
			PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent+strlen(szContent), "|C%s", pszContent2);
		}
		
		if(NULL != pszContent3)
		{
			PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent+strlen(szContent), "\n|C%s", pszContent3);
		}
		PubMsgDlg(pszTitle, szContent, 0, 1);
	}
	return APP_SUCC;
}

/**
* @brief Get Error Message form Config File with Error Code
* @return Message Content
*/
char *EmvErrMsg(int nErrCode)
{
	static char szMsg[100] = {0};

	GetEmvErrorCodeIni(nErrCode, szMsg);

	return szMsg;
}

/**
* @brief Search International organization Code
* @param in pRid RID
* @param out pOrg 
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int EmvOrgSheet(const char *pRid, char *pOrg)
{
	if (memcmp(pRid,"\xa0\x00\x00\x00\x03",5) == 0)
	{
		memcpy(pOrg,"VIS",3);
		return APP_SUCC;
	}
	else if (memcmp(pRid,"\xa0\x00\x00\x00\x04",5) == 0)
	{
		memcpy(pOrg,"MAE",3);
		return APP_SUCC;
	}
	else if (memcmp(pRid,"\xa0\x00\x00\x00\x65",5) == 0)
	{
		memcpy(pOrg,"JCB",3);
		return APP_SUCC;
	}
	else if (memcmp(pRid,"\xa0\x00\x00\x00\x25",5) == 0)
	{
		memcpy(pOrg,"AMX",3);
		return APP_SUCC;
	}
	else
	{
		if (memcmp(pRid,"\xD1\x56\x00\x00\x01",5) == 0)
		{
			memcpy(pOrg,"CUP",3);
			return APP_SUCC;
		}
		else if (memcmp(pRid,"\xa0\x00\x00\x03\x33",5) == 0)
		{
			memcpy(pOrg,"CUP",3);
			return APP_SUCC;
		}
		else if (memcmp(pRid,"\xB0\x00\x00\x03\x33",5) == 0)
		{
			memcpy(pOrg,"CUP",3);
			return APP_SUCC;
		}
		else
		{
			memcpy(pOrg, "   ", 3);/**<unknow*/
		}
	}
	return APP_FAIL;
}

/**
* @brief Save 9F26(Cryptograph First genarate In Transaction, use to send)
* @return Return:		
*/
int SaveData9F26RQ()
{
	memset(gs9F26_RQ, 0, sizeof(gs9F26_RQ));
	TxnL3GetData(_EMV_TAG_9F26_IC_AC, (uchar *)gs9F26_RQ, sizeof(gs9F26_RQ));
	return APP_SUCC;
}

int GetData9F26RQ(char *pszInOutStr)
{
	if (NULL != pszInOutStr )
	{
		memcpy(pszInOutStr, gs9F26_RQ, sizeof(gs9F26_RQ));	
	}
	return APP_SUCC;
}


/*
*	@brief     get emv tag from Appdata, code in tlv string to 55 fiedld
*	@param          const char	iTransType	 , transaction type
*	@param          char 		*oField55		out put field55, (first 2 byte is length[BCD])
*	@return			APP_SUCC/APP_FAIL
*/
int EmvPackField55(const char cTransType, char *pszOutField55, int *pnOutField55Len)
{
	int nLen = 0;
	uint nTagList[MAX_TAG_LIST_NUM];

	switch (cTransType)
	{
	case TRANS_BALANCE:
	case TRANS_SALE:
	case TRANS_CASHBACK:
	case TRANS_PREAUTH:
	case TRANS_REVERSAL:
	case TRANS_REFUND:
		nTagList[nLen++] = _EMV_TAG_72_ISSUE_SCRIPT;
		nTagList[nLen++] = _EMV_TAG_82_IC_AIP;
		nTagList[nLen++] = _EMV_TAG_84_IC_DFNAME;
		nTagList[nLen++] = _EMV_TAG_91_TM_ISSAUTHDT;
		nTagList[nLen++] = _EMV_TAG_95_TM_TVR;
		nTagList[nLen++] = _EMV_TAG_9B_TM_TSI;
		nTagList[nLen++] = _EMV_TAG_9A_TM_TRANSDATE;
		nTagList[nLen++] = _EMV_TAG_9C_TM_TRANSTYPE;
		nTagList[nLen++] = _EMV_TAG_5F2A_TM_CURCODE;
		nTagList[nLen++] = _EMV_TAG_5F30_IC_SERVICECODE;
		nTagList[nLen++] = _EMV_TAG_5F34_IC_PANSN;
		nTagList[nLen++] = _EMV_TAG_9F02_TM_AUTHAMNTN;
		nTagList[nLen++] = _EMV_TAG_9F03_TM_OTHERAMNTN;
		nTagList[nLen++] = _EMV_TAG_9F06_TM_AID;
		nTagList[nLen++] = _EMV_TAG_9F09_TM_APPVERNO;
		nTagList[nLen++] = _EMV_TAG_9F10_IC_ISSAPPDATA;
		nTagList[nLen++] = _EMV_TAG_9F1A_TM_CNTRYCODE;
		nTagList[nLen++] = _EMV_TAG_9F1E_TM_IFDSN;
		nTagList[nLen++] = _EMV_TAG_9F26_IC_AC;
		nTagList[nLen++] = _EMV_TAG_9F27_IC_CID;
		nTagList[nLen++] = _EMV_TAG_9F28_ARQC;
		nTagList[nLen++] = _EMV_TAG_9F29_TC;
		nTagList[nLen++] = _EMV_TAG_9F33_TM_CAP;
		nTagList[nLen++] = _EMV_TAG_9F34_TM_CVMRESULT;
		nTagList[nLen++] = _EMV_TAG_9F35_TM_TERMTYPE;
		nTagList[nLen++] = _EMV_TAG_9F36_IC_ATC;
		nTagList[nLen++] = _EMV_TAG_9F37_TM_UNPNUM;
		nTagList[nLen++] = _EMV_TAG_9F41_TM_TRSEQCNTR;
		nTagList[nLen++] = _EMV_TAG_9F53_MCC;			
		break;
	default:
		break;
	}

	nLen = TxnL3GetTlvData(nTagList, (uint)nLen, (uchar *)pszOutField55, 255, 0);
	TRACE("nLen = %d", nLen);
	TRACE_HEX(pszOutField55, nLen, "pszOutField55");
	if (nLen <= 0)
	{
		return APP_FAIL;
	}
	*pnOutField55Len = nLen;
	return APP_SUCC;
}

/*
 *	@brief     get emv tag from Appdata, code in tlv string
 *	@param          const char	iTransType	 , transaction type
 *	@param          char 		*oField55		out put field55,
 *(first 2 byte is length[BCD])
 *	@return			APP_SUCC/APP_FAIL
 */
int EmvPackTLVData(uint nTagList[],
                   uint numTagList,
                   char *pchTagList,
                   int *inTagList) {
  uint nLen = 0;

  nLen =
      NAPI_L3GetTlvData(nTagList, numTagList, (uchar *)pchTagList, 255, 0x01);
  TRACE("nLen = %d", nLen);
  pdump(pchTagList, nLen, "EmvPackTLVData");
  if (nLen <= 0) {
    return APP_FAIL;
  }
  *inTagList = nLen;
  return APP_SUCC;
}

/**
*	@brief  clear emv transaction log
*	@param	void
*	@return	 EMV_SUCC/EMV_FAIL
*/
int EmvClearRecord(void)
{
	int i;
	char szLogFile[100];
	STRECFILE stTransRecordFile;
	STEMVRECORD stTransRecordRec;
	
	for (i = 0; i < 2; i++)
	{
		memset(&stTransRecordFile, 0, sizeof(STRECFILE));
		
		stTransRecordFile.cIsIndex = FILE_CREATEINDEX;				
		stTransRecordFile.unMaxOneRecLen = sizeof(STEMVRECORD);
		stTransRecordFile.unIndex1Start =(uint)((char *)(stTransRecordRec.sRecordNo) - (char *)&stTransRecordRec)+1;
		stTransRecordFile.unIndex1Len =  sizeof(stTransRecordRec.sRecordNo);
		stTransRecordFile.unIndex2Start =  (uint)((char *)(stTransRecordRec.sBatchNo) - (char *)&stTransRecordRec)+1;
		stTransRecordFile.unIndex2Len = sizeof(stTransRecordRec.sBatchNo);
		if (i == 0)
		{
			strcpy(stTransRecordFile.szFileName, FILE_EMV_RECORD_FAIL);
			ASSERT_FAIL(PubCreatRecFile(&stTransRecordFile));
		}
		else
		{
			strcpy( stTransRecordFile.szFileName, FILE_EMV_RECORD_SUCC);
			ASSERT_FAIL(PubCreatRecFile(&stTransRecordFile));
		}	
	}
	sprintf(szLogFile, "%s", EMVFILE_TERMLOG);
	PubDelFile(szLogFile);/**<delete emv kernel log*/
	return APP_SUCC;
}

/**
*	@brief          save emv translog
*	@param			in recordflag	TRUE  success	FALSE fail
*	@param			in pstSystem	
*	@return			
*/
int EmvSaveRecord(int nRecordFlag, const STSYSTEM *pstSystem)
{
	int nRet;
	int nLen;
	STEMVRECORD stEmvRecord;
	char szBuff[512];

	memset(&stEmvRecord, 0, sizeof(STEMVRECORD));
	PubAscToHex((uchar *)pstSystem->szPan, strlen(pstSystem->szPan), 0, (uchar *)stEmvRecord.szPan);
	stEmvRecord.nPanLen = strlen(pstSystem->szPan);
	stEmvRecord.nPanSeq = atoi(pstSystem->szCardSerialNo);
	PubAscToHex((uchar *)pstSystem->szDate, 4, 0, (uchar *)stEmvRecord.sDate);
	PubAscToHex((uchar *)pstSystem->szTime, 6, 0, (uchar *)stEmvRecord.sTime);
	PubAscToHex((uchar *)pstSystem->szAmount, 12, 0, (uchar *)stEmvRecord.sAmount);
	PubAscToHex((uchar *)pstSystem->szTrace, 6, 0, (uchar *)stEmvRecord.sRecordNo);
	PubAscToHex((uchar *)pstSystem->szBatchNum, 6, 0, (uchar *)stEmvRecord.sBatchNo);
	stEmvRecord.cTransType = pstSystem->cTransType;
	
	TxnL3GetData(_EMV_TAG_8A_TM_ARC, (uchar *)szBuff, sizeof(szBuff));
	memcpy(stEmvRecord.sAuthResp, szBuff, 2);

	TxnL3GetData(_EMV_TAG_95_TM_TVR, (uchar *)szBuff, sizeof(szBuff));
	memcpy(stEmvRecord.sTVR, szBuff, 5);

	nLen = TxnL3GetData(_EMV_TAG_9B_TM_TSI, (uchar *)szBuff, sizeof(szBuff));
	memcpy(stEmvRecord.sTSI, szBuff, 2);

	EmvPackField55(TRANS_SALE, stEmvRecord.szField55+2, &nLen);

	PubIntToC2((uchar *)szBuff, (uint)nLen);
	memcpy(stEmvRecord.szField55, szBuff, 2);

	if(memcmp(stEmvRecord.sAuthResp, "Y1", 2)==0 || memcmp(stEmvRecord.sAuthResp, "Y3", 2)==0 || 
		memcmp(stEmvRecord.sAuthResp, "Z1", 2)==0 || memcmp(stEmvRecord.sAuthResp, "Z3", 2)==0 )
	{
		stEmvRecord.szField55[nLen+2] = 0x8a;
		stEmvRecord.szField55[nLen+3] = 0x02;
		stEmvRecord.szField55[nLen+4] = stEmvRecord.sAuthResp[0];
		stEmvRecord.szField55[nLen+5] = stEmvRecord.sAuthResp[1];
		stEmvRecord.szField55[0] = (nLen+4)/256;
		stEmvRecord.szField55[1] = (nLen+4)%256;
	}
	
	//deal script
	if((nLen = TxnL3GetData(_EMV_TAG_DF31_IC_IISSCRIRES, (uchar *)szBuff, sizeof(szBuff))) > 0)
	{
		stEmvRecord.nIssuerScriptResultLen = nLen;
		memcpy(stEmvRecord.szIssuerScriptResult, szBuff, nLen);
	}

	if(memcmp(pstSystem->szInputMode, "05", 2) == 0)
	{
		stEmvRecord.cInputMode = '5';
	}
	else if(memcmp(pstSystem->szInputMode, "07", 2) == 0)
	{
		stEmvRecord.cInputMode = '7';
	}
	stEmvRecord.cBatchUpFlag = 0;

	if (nRecordFlag)  
	{
		nRet = PubAddRec(FILE_EMV_RECORD_SUCC, (char *)&stEmvRecord);
	}
	else
	{
		nRet = PubAddRec(FILE_EMV_RECORD_FAIL, (char *)&stEmvRecord);
	}
	if (nRet != APP_SUCC)
	{
		return APP_FAIL;
	}
	return APP_SUCC;
}

/**
*	@brief          get emv translog

*	@param          char		*iBatchNo		
*	@param          char		*iRecordNo		
*	@param			TEMV_RECORD	*oRecord			
*	@param			int falg    TRUE-success log FALSE-fail log
*	@return			EMV_SUCC/EMV_FAIL
*/
int EmvLoadRecordByTrace(const char * pszInBatchNo, const char * pszInRecordNo, STEMVRECORD * pstOutRecord, int nFlag)
{
	char sBatchno[3+1];
	char sTraceno[3+1];
	int nRet;
	int nRecNO;

	memset(sBatchno, 0, sizeof(sBatchno));
	memset(sTraceno, 0, sizeof(sTraceno));
	PubAscToHex((uchar *)pszInBatchNo, 6, 0, (uchar *)sBatchno);
	PubAscToHex((uchar *)pszInRecordNo, 6, 0, (uchar *)sTraceno);

	if(nFlag)  
	{
		nRet = PubFindRec(FILE_EMV_RECORD_SUCC, sTraceno, sBatchno, 1, (char *)pstOutRecord, &nRecNO);
	}
	else
	{
		nRet = PubFindRec(FILE_EMV_RECORD_FAIL, sTraceno, sBatchno, 1, (char *)pstOutRecord, &nRecNO);
	}
	
	if (nRet != APP_SUCC)
	{
		return APP_FAIL;
	}
	return APP_SUCC;
}

/**
* @brief Additional debit/credit card flow information based on EMV
* @param in out pstTransRecord STTRANSRECORD flow structure pointer
* @return @li APP_SUCC
*		@li APP_FAIL
*		@li APP_QUIT
*/
int EmvAddtionRecord(STTRANSRECORD *pstTransRecord)
{
	int nLen;
	uchar sBuff[20]={0};
	uchar szBuff[32+1]={0};
	STEMVADDTIONRECORDINFO *pstEMVAddtionRecordInfo = (STEMVADDTIONRECORDINFO *)pstTransRecord->sAddition;

	memset(sBuff, 0, sizeof(sBuff));
	nLen = TxnL3GetData(_EMV_TAG_9F26_IC_AC, sBuff, sizeof(sBuff));
	memcpy(pstEMVAddtionRecordInfo->sTC, sBuff, 8);

	memset(sBuff, 0, sizeof(sBuff));
	nLen = TxnL3GetData(_EMV_TAG_95_TM_TVR, sBuff, sizeof(sBuff));
	memcpy(pstEMVAddtionRecordInfo->sTVR, sBuff, 5);

	memset(sBuff, 0, sizeof(sBuff));
	nLen = TxnL3GetData(_EMV_TAG_9B_TM_TSI, sBuff, sizeof(sBuff));
	memcpy(pstEMVAddtionRecordInfo->sTSI, sBuff, 2);

	memset(sBuff, 0, sizeof(sBuff));
	nLen = TxnL3GetData(_EMV_TAG_84_IC_DFNAME, sBuff, sizeof(sBuff));
	if (nLen > 0)
	{
		memcpy(pstEMVAddtionRecordInfo->sAID, sBuff, nLen);
		pstEMVAddtionRecordInfo->cAIDLen =(char)nLen;
	}
	else
	{
		if((nLen = TxnL3GetData(_EMV_TAG_9F06_TM_AID, sBuff, sizeof(sBuff))) > 0 )
		{
			memcpy(pstEMVAddtionRecordInfo->sAID, sBuff, nLen);
			pstEMVAddtionRecordInfo->cAIDLen =(char)nLen;
		}
	}

	memset(sBuff, 0, sizeof(sBuff));
	nLen = TxnL3GetData(_EMV_TAG_9F36_IC_ATC, sBuff, sizeof(sBuff));
	memcpy(pstEMVAddtionRecordInfo->sATC, sBuff, 2);

	memset(sBuff, 0, sizeof(sBuff));
	if((nLen = TxnL3GetData(_EMV_TAG_50_IC_APPLABEL, sBuff, sizeof(sBuff))) > 0)
	{
		memcpy(pstEMVAddtionRecordInfo->szAIDLable, sBuff, nLen);
		pstEMVAddtionRecordInfo->szAIDLable[nLen] = 0;
	}

	memset(sBuff, 0, sizeof(sBuff));
	nLen = TxnL3GetData(_EMV_TAG_9F12_IC_APNAME, sBuff, sizeof(sBuff));
	if(nLen >= 16)
	{
		nLen = 16;
	}
	if(nLen > 0)
	{
		memcpy(pstEMVAddtionRecordInfo->szAppPreferedName, sBuff, nLen);
		pstEMVAddtionRecordInfo->szAppPreferedName[nLen] = 0;
	}

	memset(sBuff, 0, sizeof(sBuff));
	nLen = TxnL3GetData(0xdf42, sBuff, sizeof(sBuff));
	pstEMVAddtionRecordInfo->cSignatureFlag = sBuff[0];

	memset(sBuff, 0, sizeof(sBuff));
	nLen = TxnL3GetData(0xdf41, sBuff, sizeof(sBuff));
	pstEMVAddtionRecordInfo->cForceAcceptFlag = sBuff[0];

	memset(sBuff, 0, sizeof(sBuff));
	nLen = TxnL3GetData(_EMV_TAG_9F37_TM_UNPNUM, sBuff, sizeof(sBuff));
	memcpy(pstEMVAddtionRecordInfo->sUnpNum, sBuff, 4);

	memset(sBuff, 0, sizeof(sBuff));
	nLen = TxnL3GetData(_EMV_TAG_82_IC_AIP, sBuff, sizeof(sBuff));
	memcpy(pstEMVAddtionRecordInfo->sAIP, sBuff, 2);

	memset(sBuff, 0, sizeof(sBuff));
	nLen = TxnL3GetData(_EMV_TAG_9F34_TM_CVMRESULT, sBuff, sizeof(sBuff));
	memcpy(pstEMVAddtionRecordInfo->sCVM, sBuff, 3);

	memset(sBuff, 0, sizeof(sBuff));
	nLen = TxnL3GetData(_EMV_TAG_9F10_IC_ISSAPPDATA, sBuff, sizeof(sBuff));
	if(nLen <= 32 && nLen > 0)
	{
		memcpy(pstEMVAddtionRecordInfo->sIAD, sBuff, nLen);
	}
	else
	{
		nLen = 0;
	}
	pstEMVAddtionRecordInfo->cIADlen = (char)nLen;

	memset(sBuff, 0, sizeof(sBuff));
	nLen = TxnL3GetData(_EMV_TAG_9F33_TM_CAP, sBuff, sizeof(sBuff));
	memcpy(pstEMVAddtionRecordInfo->sTermCap, sBuff, 3);

	memset(sBuff, 0, sizeof(sBuff));
	GetData9F26RQ((char *)sBuff);
	memcpy(pstEMVAddtionRecordInfo->sARQC, sBuff, 8);

	//Result of Issuing bank script
	memset(szBuff,0,sizeof(szBuff));
	if((nLen = TxnL3GetData(_EMV_TAG_DF31_IC_IISSCRIRES, (uchar *)szBuff, sizeof(szBuff))) > 0 )
	{
		pstEMVAddtionRecordInfo->nIssuerScriptResultLen = nLen ;
		memcpy(pstEMVAddtionRecordInfo->szIssuerScriptResult, szBuff, nLen) ;
		TRACE_HEX((char *)szBuff, nLen,"_EMV_TAG_DF31_IC_IISSCRIRES[%d]:", nLen);
	}
	
	memset(szBuff,0,sizeof(szBuff));
	nLen = TxnL3GetData(_EMV_TAG_9F27_IC_CID, (uchar *)szBuff, sizeof(szBuff));
	pstEMVAddtionRecordInfo->cCrmInfoData = szBuff[0] ;
	TRACE_HEX((char *)szBuff, nLen,"_EMV_TAG_9F27_IC_CID[%d]:", nLen);

	
	pstTransRecord->nAdditionLen = sizeof(STEMVADDTIONRECORDINFO);
	return APP_SUCC;
}

void EmvOfflineAccept(char *pszTitle, STSYSTEM *pstSystem)
{
	char szAid[20+1] = {0};
	char szField55[320+1] = {0};
	STTRANSRECORD stTransRecord;

	memset(&stTransRecord, 0, sizeof(STTRANSRECORD));
	if (pstSystem->cTransAttr == ATTR_CONTACT)
	{
		PubBeep(1);
	}
	pstSystem->cTransType = TRANS_OFFLINE;
	pstSystem->szInputMode[2] = '2';

	pstSystem->psAddField = szField55;

	EmvPackField55(TRANS_SALE, pstSystem->psAddField, &pstSystem->nAddFieldLen);
	SaveData9F26RQ();

	TxnL3GetData(_EMV_TAG_9F06_TM_AID, (uchar *)szAid, sizeof(szAid));
	EmvOrgSheet(szAid, pstSystem->szInterOrg);
	pstSystem->cEMV_Status = EMV_STATUS_OFFLINE_SUCC;
	IncVarTraceNo();
	EmvSaveRecord(TRUE, pstSystem);

	IncVarOfflineUnSendNum();
	EmvAddtionRecord(&stTransRecord);
	SysToRecord(pstSystem, &stTransRecord);
	TradeComplete(pszTitle, pstSystem, &stTransRecord, NULL);
	ShowLightSucc();
    DispOutICC(NULL, tr("APPROVED"), "");
}

void EmvOfflineDenial(char *pszTitle, STSYSTEM* pstSystem)
{
	char szAid[20+1];
	char szField55[320+1];

	memset(szField55, 0, sizeof(szField55));
	memset(szAid, 0, sizeof(szAid));

	IncVarTraceNo();

	TxnL3GetData(_EMV_TAG_9F06_TM_AID, (uchar *)szAid, sizeof(szAid));
	EmvOrgSheet(szAid, pstSystem->szInterOrg);
	pstSystem->cEMV_Status = EMV_STATUS_OFFLINE_FAIL;
	pstSystem->psAddField = szField55;
	EmvPackField55(pstSystem->cTransType, pstSystem->psAddField, &pstSystem->nAddFieldLen);

	/*Save Fail Transaction log*/
	EmvSaveRecord(FALSE, pstSystem);
	ShowLightFail();
	DispOutICC(pszTitle, tr("DECLINE"), "");
	TRACE("transType = %d field55_len = %d\r", pstSystem->cTransType, pstSystem->nAddFieldLen);
}

/**
* @brief Send Online Detail
* @param in pstSystem 
* @param in pstEMV_Record
* @return
* @li APP_SUCC		
* @li APP_FAIL
*/
static int EmvDetailSendOnline(STSYSTEM *pstSystem, STTRANSCFG stTransCfg)
{
	int nRet;
	char sPack[MAX_PACK_SIZE];
	int nPackLen;

	ASSERT_HANGUP_FAIL(PackGeneral(pstSystem, NULL, stTransCfg));	
	ASSERT_HANGUP_FAIL(Pack(sPack, &nPackLen));

	if (pstSystem->cMustChkMAC == 0x01)
	{
		nPackLen -= 8;
		ASSERT_HANGUP_FAIL(AddMac(sPack,&nPackLen, KEY_TYPE_MAC));
	}
	PubDisplayTitle(tr(stTransCfg.szTitle));
	ASSERT_HANGUP_FAIL(CommConnect());
	nRet = CommSendRecv(sPack, nPackLen, sPack, &nPackLen);
	if (nRet != APP_SUCC)
	{
		return APP_FAIL;
	}

	nRet = Unpack(sPack, nPackLen);
	if (nRet != APP_SUCC)
	{
		return APP_FAIL;
	}
	nRet = ChkRespMsgID(stTransCfg.szMsgID, sPack);
	if (nRet != APP_SUCC)
	{
		return APP_FAIL;
	}
	nRet = ChkRespon(pstSystem, sPack + 2);
	if (nRet != APP_SUCC)
	{
		return APP_FAIL;
	}
	
	PubClearAll();
	if (memcmp(pstSystem->szResponse, "00", 2) != 0)
	{
		DispResp(pstSystem->szResponse);
		return -5;
	}

	return APP_SUCC;
}

/**
* @brief Send all the successful online detail
* @param in nFileHandle
* @param in nRecNum
* @param in pnBatchNum
* @return
* @li APP_SUCC		
* @li APP_FAIL
*/
int EmvBatchUpOnlineSucc(const int nFileHandle, const int nRecNum, int *pnBatchNum)
{
	STSYSTEM stSystem;
	STTRANSRECORD stTransRecord;
	STEMVRECORD stEmvRecord;
	STTRANSCFG stTransCfg;
	int nRet;
	int nLoop, nFailLoop = 0, nIsFirstFail = TRUE;
	char nMaxReSend;
	char cTransType = TRANS_SENDTC;

	memset(&stSystem, 0, sizeof(STSYSTEM));
	memset(&stTransRecord, 0, sizeof(STTRANSRECORD));
	memset(&stEmvRecord, 0, sizeof(STEMVRECORD));
	memset(&stTransCfg, 0, sizeof(STTRANSCFG));

	GetVarCommReSendNum(&nMaxReSend);
	nLoop = 1;
	
	GetVarEmvOnlineUpNum(&nLoop);
	if (0 >= nLoop)
	{
		nLoop = 1;
	}

	ASSERT_FAIL(TxnLoadConfig(cTransType, &stTransCfg));
	for (; nLoop <= nRecNum; nLoop++)
	{
		memset(&stTransRecord, 0, sizeof(STTRANSRECORD));
		nRet = ReadTransRecord(nFileHandle, nLoop, &stTransRecord);
		if (nRet != APP_SUCC)
		{
			continue;
		}
		
		if ((ATTR_CONTACT == stTransRecord.cTransAttr && EMV_STATUS_ONLINE_SUCC == stTransRecord.cEMV_Status))
		{
			;
		}
		else
		{
			continue;
		}

		if(stTransRecord.cTcFlag > nMaxReSend)
			continue;

		DealSystem(&stSystem);
		RecordToSys(&stTransRecord, &stSystem);
		if (APP_SUCC != EmvLoadRecordByTrace(stSystem.szBatchNum, stSystem.szTrace, &stEmvRecord, TRUE))
		{
			continue;
		}

		if (stEmvRecord.sTVR[4]&0x40)
		{
			continue;
		}

		memcpy(stSystem.szMsgID, stTransCfg.szMsgID, 4);
		memcpy(stSystem.szProcCode, stTransCfg.szProcessCode, 6);
		nRet = EmvDetailSendOnline(&stSystem, stTransCfg);
		if (nRet != APP_SUCC)
		{
			if(nRet == -5)
			{
				stTransRecord.cTcFlag = 0xFE;
			}
			goto BATCHUPFAIL;
		}
		stTransRecord.cTcFlag = 0xFD;
	
		PubDisplayGen(3, "SEND TC OK");
		PubUpdateWindow();
	BATCHUPFAIL:
		ASSERT_FAIL(DealBatchUpFail(&stTransRecord, nFileHandle, nLoop));
		if(stTransRecord.cTcFlag <= nMaxReSend)
		{
			if(nIsFirstFail)
			{
				nFailLoop = nLoop;
				nIsFirstFail = FALSE;
				SetVarEmvOnlineUpNum(nFailLoop);
			}
		}	
	}
	if(nIsFirstFail)
		nFailLoop = nRecNum+1;
	SetVarEmvOnlineUpNum(nFailLoop);
	return APP_SUCC;
}

/**
* @brief Send TC
* @param void
* @return 
* @li APP_SUCC
* @li APP_FAIL
* @li APP_QUIT
* @author 
* @date 
*/
int EmvSendTC(void)
{	
	int nRet = 0;
	int nRecNum;
	int nFileHandle;
	int nBatchNum = 0;

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
		return APP_FAIL;
	}

	nRet = EmvBatchUpOnlineSucc(nFileHandle, nRecNum, &nBatchNum);
	if (nRet != APP_SUCC)
 	{   
		PubCloseFile(&nFileHandle);
		return APP_FAIL;
 	}
	
	ASSERT_FAIL(PubCloseFile(&nFileHandle));
	
	SetVarEmvOnlineUpNum(1);

	return APP_SUCC;
}


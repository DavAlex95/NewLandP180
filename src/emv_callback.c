/**************************************************************************
* Copyright (C) 2019 Newland Payment Technology Co., Ltd All Rights Reserved
*
* @file callback.c
* @brief 
* @version 1.0
* @author 
* @date 2019-9-2
**************************************************************************/


#include "apiinc.h"
#include "libapiinc.h"
#include "appinc.h"
#include "napi_crypto.h"
#include "pinpadtool.h"
#include "pinpad.h"

static char gszLanguageType[2+1] = {0};
static char gszAmount[12+1] = {0};
static char gsPINBlock[16+1] = {0};
static char gszCardEventData[19+1] = {0};
static char gszExpriryDate[4+1] = {0};
static char gszCvv[4+1] = {0};

static char gcIsCreateVirtualKb = NO;
static char gcTransType = 0xFF;

extern int ActivePIN(L3_PIN_TYPE type, char *pszPan, publicKey *pinPK);
extern int PubGetKeySystemType();

#define ASSERT_L3_RET(e) \
		{\
			int nRet = e;\
			if (nRet != APP_SUCC)\
			{\
				if(nRet == APP_QUIT)\
				{\
					return L3_ERR_CANCEL;\
				}\
				else if (nRet == APP_TIMEOUT)\
				{\
				    return L3_ERR_TIMEOUT;\
				}\
				return L3_ERR_FAIL;\
			}\
		}\

typedef enum{
	KEY_SYSTEM_MKSK   = 1,
    KEY_SYSTEM_DUKPT  = 2,
	KEY_SYSTEM_FIXED  = 3,
}ENM_KEY_SYSTEM;


typedef struct
{
    L3_UI_CARD nUIId;
    char* pszPrompt;
}STUIPROMPT;

const static STUIPROMPT gszUICard[] = {
	
    {UI_KEYIN, "MANUAL"},
    {UI_STRIPE, "SWIPE"},
    {UI_INSERT, "INSERT"},
    {UI_TAP, "TAP"},
    {UI_INSERTC_TAP, "INSERT/TAP"},
    {UI_STRIPE_INSERT, "SWIPE/INSERT"},
    {UI_STRIPE_TAP, "SWIPE/TAP"},
    {UI_STRIPE_INSERT_TAP, "SWIPE/INSERT/TAP"},
    {UI_PRESENTCARD_AGAIN, "TAP CARD AGAIN"},
    {UI_USE_CHIP, "PLEASE USE CHIP"},
    {UI_FALLBACK_CT, "CHIP ERROR, PLEASE USE SWIPE"},
    {UI_FALLBACK_CLSS, "INSERT, SWIPE OR TRY ANOTHER CARD"},
	{UI_STRIPE_INSERT_TAP_MANUAL, "SWIPE/INSERT\n              TAP/MANUAL"},
    {UI_STRIPE_INSERT_MANUAL, "SWIPE/INSERT/MANUAL"},
    {UI_STRIPE_TAP_MANUAL, "SWIPE/TAP/MANUAL"},
    {UI_INSERT_TAP_MANUAL, "INSERT/TAP/MANUAL"},
	{UI_STRIPE_MANUAL, "SWIPE/MANUAL"},
	{UI_INSERT_MANUAL, "INSERT/MANUAL"},
	{UI_TAP_MANUAL, "TAP/MANUAL"},
};

static void UI_PresentCard(int nUIType)
{
	int i;
	int nLine = 3;
	char szDispAmt[DISPAMTLEN] = {0};
	
	char *pContent = "Present Card";

	TRACE("nUIType = %d", nUIType);
	for (i = 0; i < sizeof(gszUICard)/sizeof(gszUICard[0]); i++)
	{
	    if (nUIType == gszUICard[i].nUIId)
	    {
	        pContent = gszUICard[i].pszPrompt;
			break;
	    }
	}

	if (CheckIsNullOrEmpty(gszAmount, 12) == NO)
	{
		ProAmtToDispOrPnt(gszAmount, szDispAmt);
		PubAllTrim(szDispAmt);
		PubDisplayStrInline(DISPLAY_MODE_CENTER, nLine, szDispAmt);
		nLine++;
	}
	
    PubDisplayStr(DISPLAY_MODE_CENTER, nLine, 1, pContent);
	PubUpdateWindow();
	
}

//typedef int (*FUNC_UI_EVENT)(unsigned int uiEventID, unsigned char*uiEventData);
int Func_UI_Event(unsigned int uiEventID, unsigned char*uiEventData)
{
 	int nLine = 3;
	
    PubClear2To4();
	switch(uiEventID)
	{
	case UI_PRESENT_CARD:
		if (uiEventData == NULL)
		{
			TRACE("uiEventData is NULL");
			ASSERT_L3_RET(APP_FAIL);
		}
		UI_PresentCard(uiEventData[0]);
		break;
	case UI_PROCESSING:
		PubDisplayStr(DISPLAY_MODE_CENTER, nLine, 1, "PROCESSING...");
		PubUpdateWindow();
		break;
	case UI_CAPK_LOAD_FAIL:
		PubMsgDlg(NULL, "Special CAPK is Wrong", 0, 1);
		break;
	case UI_SEE_PHONE:
		PubMsgDlg(NULL, "See Phone for Instructions", 0, 3);
		break;
	case UI_CARDNUM_CONFIRM:
		{
			char cEntryMode = 0;
			char szPan[19+1] = {0};
			int nLen = 0;

			TxnL3GetData(L3_DATA_POS_ENTRY_MODE, &cEntryMode, 1);
			if(cEntryMode == ATTR_CONTACT || cEntryMode == ATTR_CONTACTLESS)
			{
				return APP_SUCC;
			}
			else if(cEntryMode == ATTR_MANUAL)
			{
				GetCardEventData(szPan);
				nLen = strlen(szPan);
 			}
			else
			{
				nLen = TxnL3GetData(L3_DATA_PAN, szPan, 19);
			}
			if(nLen <= 0)
			{
				ASSERT_L3_RET(APP_FAIL);
			}
			
			ASSERT_L3_RET(DispPan(szPan));
		}
		break;
	case UI_CHIP_ERR_RETRY:
		PubMsgDlg(NULL, "Chip Error, Retry", 0, 1);
		if (PubGetKbAttr() == KB_VIRTUAL) {
			ResetVirtualkbStatus();
		}
		break;
	}

    return APP_SUCC;
}

static void ProShowStar(int nCount, int nLine)
{
    int i;
	char szStar[32]={0};
	int nMaxLen = 12;

    for(i = 0 ; i < nCount ; i++)
    {
    	szStar[i] = '*' ;
    }
    if(nCount < nMaxLen)
    {
    	szStar[i] = '_' ;
    	for(i++ ; i < nMaxLen ; i++)
    	{
    		szStar[i] = ' ' ;
    	}
    }

	PubDisplayStrInline(DISPLAY_MODE_CLEARLINE, nLine, szStar);
	PubUpdateWindow();
}

// typedef int (*FUNC_GET_PIN)(L3_PIN_TYPE type, unsigned int cnt, PublicKey *pinPK, unsigned char *sw1sw2);
int Func_GET_PIN(L3_PIN_TYPE type, unsigned int cnt, publicKey *pinPK, unsigned char *sw1sw2)
{
	char szDispAmt[DISPAMTLEN] = {0};
	int nLine = 1;
	int nRet = 0;
	unsigned char sPin[20] = {0};
	unsigned char szKsn[20+1] = {0};
	int nEvent, nOutPinLen, nKsnLen;
	int nMainKeyNo;
	int nCount = 0;
	char szPan[19+1] = {0};
	char cEntryMode;
	static char cPinResultStatus = TRUE;

	memset(gsPINBlock, 0, sizeof(gsPINBlock));
	PubClear2To4();
	if (CheckIsNullOrEmpty(gszAmount, 12) == NO)
	{
	    ProAmtToDispOrPnt(gszAmount, szDispAmt);
		PubAllTrim(szDispAmt);
		PubDisplayStrInline(DISPLAY_MODE_CLEARLINE, nLine, szDispAmt);
		nLine++;
	}

	TxnL3GetData(L3_DATA_POS_ENTRY_MODE, &cEntryMode, 1);

	if (cEntryMode == ATTR_MANUAL)
	{
		GetCardEventData(szPan);
	}
	else
	{
		nRet = TxnL3GetData(L3_DATA_PAN, szPan, 19);
		if (nRet <= 0)
		{
			cPinResultStatus = TRUE;
			return -1;
		}
	}

	TRACE("PAN = %s", szPan);
	if (YES == GetVarIsPinpad())
	{
		ASSERT_QUIT(GetPin(szPan, NULL, gsPINBlock));
		return APP_SUCC;
	}

	if (type == L3_PIN_ONLINE)
	{
	    PubDisplayStrInline(DISPLAY_MODE_CLEARLINE, nLine, "Enter PIN:");
		nLine++;
	}
	else
	{
		if (cPinResultStatus == FALSE)
		{
			PubDisplayStrInline(DISPLAY_MODE_CLEARLINE, nLine, "INCORRECT PIN:");
			PubUpdateWindow();
			PubBeep(1);
			PubSysMsDelay(300);
		}

		if (cnt == 1)
		{
		    PubDisplayStrInline(DISPLAY_MODE_CLEARLINE, nLine, "Last PIN:");
		}
		else
		{
		    PubDisplayStrInline(DISPLAY_MODE_CLEARLINE, nLine, "Enter PIN:");
		}
		nLine++;
	}
	PubUpdateWindow();
	GetVarMainKeyNo(&nMainKeyNo);
	PubSetCurrentMainKeyIndex(nMainKeyNo);
	nRet = ActivePIN(type, szPan, pinPK);
	if (nRet != 0)
	{
		TRACE("nRet = %d", nRet);
		cPinResultStatus = TRUE;
		_FINISH_PIN_INPUT_THREAD(L3_ERR_FAIL);
	}

	while(1)
	{
		if(cEntryMode == ATTR_CONTACT && ProGetCardStatus() != APP_SUCC)
		{
			break;
		}
		nRet = NAPI_SecVPPGetEvent(&nEvent, sPin, &nOutPinLen, szKsn, &nKsnLen);
		
		if (nRet == NAPI_ERR_SECVP_PIN_TOO_SHORT)
		{
			TRACE("NAPI_ERR_SECVP_PIN_TOO_SHORT");
			continue;
		}
		if (nRet != NAPI_OK)
		{
			ProSetSecurityErrCode(ERR_GETPINRESULT,nRet);
            PINPAD_TRACE_SECU("NAPI_SecVPPGetEvent error,nRet=%d", nRet);
			if (nRet == NAPI_ERR_TIMEOUT
				|| nRet == NAPI_ERR_SECVP_TIMED_OUT)
			{
				cPinResultStatus = TRUE;
				_FINISH_PIN_INPUT_THREAD(L3_ERR_CANCEL);
				//_FINISH_PIN_INPUT_THREAD(L3_ERR_TIMEOUT);
			}
			cPinResultStatus = TRUE;
			_FINISH_PIN_INPUT_THREAD(L3_ERR_FAIL);
		}
		TRACE("nEvent = %d", nEvent);
		switch(nEvent)
		{
		case NAPI_SEC_VPP_KEY_PIN:
		case NAPI_SEC_VPP_KEY_BACKSPACE:
		case NAPI_SEC_VPP_KEY_CLEAR:
            nCount = sPin[0] & 0x0F;
            ProShowStar(nCount, nLine);
			break;
		case NAPI_SEC_VPP_KEY_ENTER:
			if (L3_PIN_ONLINE == type)
			{
				memcpy(gsPINBlock, sPin, 16);
				
				if (0 == memcmp(gsPINBlock, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 16))			    
				{
					_FINISH_PIN_INPUT_THREAD(L3_ERR_BYPASS);
				}
				else
				{
					_FINISH_PIN_INPUT_THREAD(L3_ERR_SUCC);
				}
			}
			else
			{
			    memcpy(sw1sw2, sPin, 2);
				if (0 == memcmp(sw1sw2, "\x00\x00", 2))
				{
					cPinResultStatus = TRUE;
					_FINISH_PIN_INPUT_THREAD(L3_ERR_BYPASS);
				}
				else if (0 != memcmp(sw1sw2, "\x90\x00", 2))
                {
                    cPinResultStatus = FALSE;
                }
				else
				{
					cPinResultStatus = TRUE;
				}
				_FINISH_PIN_INPUT_THREAD(L3_ERR_SUCC);
			}
			break;
		case NAPI_SEC_VPP_KEY_ESC:
			cPinResultStatus = TRUE;
			_FINISH_PIN_INPUT_THREAD(L3_ERR_CANCEL);	
		default:
			break;
		}
	}
	cPinResultStatus = TRUE;
	_FINISH_PIN_INPUT_THREAD(L3_ERR_FAIL);
}

// typedef int (*FUNC_GET_AMOUNT)(L3_AMOUNT_TYPE type, unsigned char *amt);
int Func_GET_AMOUNT(L3_AMOUNT_TYPE type, unsigned char *amt)
{
    int nAmtLen = 12;
    char szContent[256] = {0};
    char szAmount[12+1] = {0};

    if(type == L3_AMOUNT)
    {
        strcpy(szContent, "ENTER AMOUNT:");
    }
    else
    {
        strcpy(szContent, "ENTER CASHBACK:");
    }
    ASSERT_FAIL(PubInputAmount(NULL, szContent, szAmount, &nAmtLen, INPUT_AMOUNT_MODE_NOT_NONE, 60))
    memcpy(amt, szAmount, 12);
    return APP_SUCC;
}

/**
* @brief  is it support the issuing bank code table
* @param in ucIssCodeIdx             --- code table index
* @param in usTerminalCode           --- supported code table
* @return
* @li  >0               support
* @li  <=0              unsupport
*/
static int IsSuppIssCode(uchar ucIssCodeIdx, uchar usTerminalCode[])
{
	int i;

	i = 1;

	if (ucIssCodeIdx > 8)
	{
		i -= 1;
		ucIssCodeIdx -= 8;
	}
	return (usTerminalCode[i] & (1UL << (ucIssCodeIdx - 1)));
}

// typedef int (*FUNC_SELECT_CANDIDATE_LIST)(L3_CANDIDATE_LIST candidateList[], int listNum,  int *pSelect);
int Func_SELECT_CANDIDATE_LIST(L3_CANDIDATE_LIST candidateList[], int listNum,  int *pSelect)
{
    int i;
//	int nRet = 0;
	int nCountLine = 0;
	char lszBuffer[9][40];
	char szTitle[20] = {0};
	char *pszItems[9] = {0}; 
	int nSelectItem = 1, nStartItem = 1;
	
    memset(lszBuffer, 0, sizeof(lszBuffer));

	/*At most 9 applications*/
	for (i = 0; i < listNum; i++) 
	{
        if (9 == nCountLine)
        {
            break;
        }

		if (candidateList[i].customDataSize > 0)
		{
		    TRACE_HEX((char*)candidateList[i].customTagData, candidateList[i].customDataSize, "customTagData:");
		}
			
		
        sprintf(lszBuffer[nCountLine], "%d.", nCountLine+1); 

		if (candidateList[i].issuerCodeTableIndex != 0xFF
			&& IsSuppIssCode(candidateList[i].issuerCodeTableIndex, candidateList[i].terminalCodeTable)
			&& candidateList[i].perferNameLen > 0)
		{
		    memcpy(&(lszBuffer[nCountLine][2]), candidateList[i].perferName, candidateList[i].perferNameLen);
		}
		else 
		{
		    if (candidateList[i].lableLen > 0)
		    {
		        memcpy(&(lszBuffer[nCountLine][2]), candidateList[i].lable, candidateList[i].lableLen);
		    }
			else
			{
			    PubHexToAsc(candidateList[i].aid, candidateList[i].aidLen * 2, 0, (uchar *)&(lszBuffer[nCountLine][2]));
			}
		}
        nCountLine ++;
	}

	NAPI_ScrPush();
	PubClearAll();
	strcpy(szTitle, tr("CHOOSE THE APP"));
	for (i=0; i< nCountLine; i++)
	{
		pszItems[i] = lszBuffer[i];
	}
	while (1)
	{
		ASSERT_L3_RET(PubShowMenuItems(szTitle, pszItems, nCountLine, &nSelectItem, &nStartItem,0));
		if (nSelectItem > 0 && nSelectItem <= 9)
		{
			break;
		}
	}
	NAPI_ScrPop();
    *pSelect = nSelectItem - 1;
    return L3_ERR_SUCC;
}

// typedef int (*FUNC_SELECT_ACCOUNT_TYPE)(unsigned char *acct_type);
int Func_SELECT_ACCOUNT_TYPE(unsigned char *acct_type)
{
    	char *pszItems[] = {
		tr("1.Default"),
		tr("2.Savings"),
		tr("3.Cheque/debit"),
		tr("4.Credit")
	}; 
	int nSelectItem = 1;
	int nStartItem = 1;
//	int nRet = 0;

	NAPI_ScrPush();
	ASSERT_L3_RET(PubShowMenuItems("ACCOUNT TYPE", pszItems, sizeof(pszItems)/sizeof(char *), &nSelectItem, &nStartItem,0));
	NAPI_ScrPop();
    *acct_type = nSelectItem - 1;
    return L3_ERR_SUCC;
}

// typedef int (*FUNC_SELECT_LANGUAGE)();
int Func_SELECT_LANGUAGE()
{
    int i;
	int nRet = 0;
	char languagePreference[8+1] = {0};

	nRet = TxnL3GetData(0x5F2D, languagePreference, 8);
	if (nRet <= 0)
	{
		memcpy(gszLanguageType, "en", 2);
	    return L3_ERR_SUCC;
	}
	
    for(i=0; i < 8; i+=2)
    {
        if ((memcmp(languagePreference + i, "zh", 2) == 0) 
			|| (memcmp(languagePreference + i, "ZH", 2) == 0)
			|| (memcmp(languagePreference + i, "Zh", 2) == 0))
		{
			memcpy(gszLanguageType, "zh", 2);
			return L3_ERR_SUCC;
		}
		else if ((memcmp(languagePreference + i, "en", 2) == 0) 
			|| (memcmp(languagePreference + i, "EN", 2) == 0)
			|| (memcmp(languagePreference + i, "En", 2) == 0))
		{
			memcpy(gszLanguageType, "en", 2);
			return L3_ERR_SUCC;
		}
    }

    if(i >= 8)
    {
        int nSelcItem = 1, nStartItem = 1;
		char *pszItems[] = {
			//"1.Chinese",
			"2.English"
		}; 
		ASSERT_L3_RET(PubShowMenuItems("Language Choose", pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 0));

		switch(nSelcItem)
		{
		case 1:
			memcpy(gszLanguageType, "zh", 2);
			return APP_SUCC;
		case 2:
			memcpy(gszLanguageType, "en", 2);
			return APP_SUCC;
		//case 3:
		//	memcpy(pusLangueData, "fr", 2);
		//	memcpy(pusLanguageType, "fr", 2);
		//	return 0;
		default:
			return L3_ERR_CANCEL;
		}
    }
    return APP_SUCC;
}
	

// typedef int (*FUNC_CHECK_CREDENTIALS)(void);
int Func_CHECK_CREDENTIALS(void)
{
	int nRet = 0;
    int nLen = 1;
	uchar ucType = 0;
    char szNumber[50] = {0};
    char szBuf[100] = {0};
    char *szTypestr[] = 
	{
		tr("ID CARD"),
		tr("OFFICER"), 
		tr("PASSPORT"),
		tr("ENTRY CARD"),
		tr("TEMP IDCARD"), 
		tr("OTHER")
	};

	nRet = TxnL3GetData(0x9F62, &ucType, 1);
	if (nRet <= 0 || ucType > 0x05)
	{
	    return L3_ERR_FAIL;
	}

	nLen = TxnL3GetData(0x9F61, szNumber, sizeof(szNumber));
	if (nLen <= 0)
	{
	    return L3_ERR_FAIL;
	}


    NAPI_ScrPush();
    sprintf(szBuf, "TYPE:%s\nCARD NO:%*.*s", szTypestr[ucType], nLen, nLen, szNumber);
    ASSERT_L3_RET(PubConfirmDlg("CHECK CREDENTIALS", szBuf, 1, 60));	
    NAPI_ScrPop();

	return L3_ERR_SUCC;
	
}


//typedef int (*FUNC_VOICE_REFERRALS)(void);
int Func_VOICE_REFERRALS(int *result)
{
	char szPan[19+1] = {0};
	int nRet = 0;
	char *pszItems[] = {
		tr("1.Approve"),
		tr("2.Decline"),
	}; 
	int nSelectItem = 1, nStartItem = 1;

	nRet = TxnL3GetData(L3_DATA_PAN, szPan, 19);
	if (nRet <= 0)
	{
	    return L3_ERR_FAIL;
	}

	PubClearAll();
	PubDisplayStrInline(0,2, "Please Call your bank");
	PubDisplayStrInline(0,3, "Card Number:");
	PubDisplayStrInline(0,4, szPan);
	PubUpdateWindow();
	PubBeep(1);
	PubWaitConfirm(20);
	PubClearAll();



	NAPI_ScrPush();
	ASSERT_L3_RET(PubShowMenuItems("Bank Result", pszItems, sizeof(pszItems)/sizeof(char *), &nSelectItem, &nStartItem, 0));
	NAPI_ScrPop();
	if (nSelectItem == 1)
	{
	   *result = 0;
	}
	else
	{
		*result = 1;
	}
	return L3_ERR_SUCC;
}

//typedef int(*FUNC_DekDet)(unsigned char type, unsigned char *buf, int *len);
int Func_DekDet(unsigned char type, unsigned char *buf, int *len)
{
    if (type == 0x01) //DEK, Send buf to Host
    {
    }
	else //DET, Receive data from Host
	{
	}
	return L3_ERR_SUCC;
}

// typedef void(*FUNC_EmvDebug)(const char *log, int len);
void Func_EmvDebug(const char *log, int len)
{
	PubBufToAux(log, len);
}

//typedef int(*FUNC_AFTER_FINAL_SELECT)(L3_CARD_INTERFACE interface, unsigned char *aid, int aidLen);
int Func_AFTER_FINAL_SELECT(L3_CARD_INTERFACE interface, unsigned char *aid, int aidLen)
{
	/**
	* You can update AID/Terminal configuration in current transaction in this function.
	* These codes are just for Demo 
	*/
	if (L3_CONTACT == interface)
	{
		if (0 == memcmp(aid, "\xA0\x00\x00\x00\x04\x30\x60", 7))
		{
		    TxnL3SetData(_EMVPARAM_9F09_APPVER, "\x00\x02", 2);
		}
	}
	else //CONTACTLESS
	{
		if (memcmp(aid, "\xA0\x00\x00\x03\x33", 5) == 0)
		{
		    TxnL3SetData(_EMVPARAM_5F2A_TRANSCCODE, "\x01\x56", 2);
		}
		else if (memcmp(aid, "\xA0\x00\x00\x00\x04", 5) == 0 && gcTransType == TRANS_REFUND)
		{
			// according to the M/Chip Requirements a refund should lead to an AAC(test case:MCD01.Test.02.Scenario.02)
			TxnL3SetData(_EMVPARAM_DF13_TACDENIAL, "\xFF\xFF\xFF\xFF\xFF", 5);
		}
	}
	
	return 0;
}

static int ProInputCardNo(int nMin, int nMax, char *szCardNo, int nTimeOut)
{
	int nCode, nLen;
	uint unX, unY;
	char szBuf[128] = {0};
	int nLineHeight,nLineno;
	
	NAPI_ScrGetxy(&unX, &unY);
	PubGetDispForm(NULL, NULL ,&nLineHeight);
	//PubGetDispLineInfo(&nLineHeight);

	if (PubGetKbAttr() == KB_PHYSICAL) {
		nLineno = unY/nLineHeight + 2;
	} else {
		unX = 48;
		unX = 192;
		nLineno = 6;
	}

	strcpy(szBuf, szCardNo);
	nLen = strlen(szBuf);
	while(1)
	{		
		PubDisplayStrInline(DISPLAY_MODE_CLEARLINE, nLineno, szBuf);
		NAPI_ScrRefresh();
		if (PubGetKbAttr() == KB_VIRTUAL)
		{
			nCode = Virtual_KbGetCode(nTimeOut);
			if (nCode == APP_TIMEOUT) {
				nCode = 0;
			}
		} 
		else 
		{
			NAPI_KbGetCode(nTimeOut, &nCode);
		}

		switch(nCode)
		{
		case 0:
			NAPI_ScrGotoxy(unX, unY);
			return APP_TIMEOUT;
		case KEY_ESC:
			NAPI_ScrGotoxy(unX, unY);
			return APP_QUIT;
		case KEY_BACK:
			nLen--;
			szBuf[nLen] = 0x20;
			if(nLen == 0)
			{
				PubDisplayStrInline(DISPLAY_MODE_CLEARLINE,  nLineno, szBuf);
				NAPI_ScrRefresh();
				NAPI_ScrGotoxy(unX, unY);
				return APP_FAIL;
			}
			break;
		case KEY_ENTER:
			if(nLen >= nMin && nLen <= nMax)
			{
				memcpy(szCardNo, szBuf, nLen);
				return APP_SUCC;
			}
			PubBeep(1);
			break;
		case KEY_0:
		case KEY_1:
		case KEY_2:
		case KEY_3:
		case KEY_4:
		case KEY_5:
		case KEY_6:
		case KEY_7:
		case KEY_8:
		case KEY_9:
			if(nLen < nMax)
			{
				szBuf[nLen] = nCode;
				nLen++;
			}
			else
			{
				PubBeep(1);
			}
			break;
		}
	}
	NAPI_ScrGotoxy(unX, unY);
	return APP_FAIL;
}

//typedef int(*FUNC_CARD_DETECT_EVENT)(int input, int *res);
int Func_CARD_DETECT_EVENT(int input, int *res)
{
	int nKeyCode = 0;
	char szInput[200] = {0};
	ST_PADDATA stPaddata;
	uint unX, unY, unScrWidth, unScrHeight;

	memset(gszCardEventData, 0, sizeof(gszCardEventData));
	if (PubGetKbAttr() == KB_VIRTUAL)
	{
		memset(&stPaddata, 0, sizeof(ST_PADDATA));
		NAPI_ScrGetViewPort(&unX, &unY, &unScrWidth, &unScrHeight);

		if (input & L3_CARD_OTHER_EVENT)
		{
			nKeyCode = PubShowGetKbPad_Ms(300, BUTTON_HOMEMENU, "F1", "Enter", "F2", NULL);
			if(nKeyCode == APP_TIMEOUT) {
				nKeyCode = 0;
			}
		}
		else
		{
			if (gcIsCreateVirtualKb == NO)
			{
				Virtual_KbCreate(NULL, INPUT_MODE_NUMBER);
			}
			nKeyCode = Virtual_KbHit();
			if (nKeyCode == 0) {
				gcIsCreateVirtualKb = YES;
				return L3_ERR_TIMEOUT;
			}
		}
	} 
	else
	{
		NAPI_KbHit(&nKeyCode);
	}

	switch(nKeyCode)
	{
	case 0:
		return L3_ERR_TIMEOUT;
	case KEY_ESC:
		return L3_ERR_CANCEL;
	case KEY_0:
	case KEY_1:
	case KEY_2:
	case KEY_3:
	case KEY_4:
	case KEY_5:
	case KEY_6:
	case KEY_7:
	case KEY_8:
	case KEY_9:
		if (input & L3_CARD_MANUAL)
		{
			szInput[0] = nKeyCode;
			ASSERT_L3_RET(ProInputCardNo(13, 19, szInput, 60));
			strcpy(gszCardEventData, szInput);
			*res = L3_CARD_MANUAL;
			return L3_ERR_SUCC;
		}
	default:
		if (input & L3_CARD_OTHER_EVENT)
		{
			gszCardEventData[0] = nKeyCode;
			*res = L3_CARD_OTHER_EVENT;
			return L3_ERR_SUCC;
		}
		break;
	}

	return L3_ERR_FAIL;
}


//typedef int(*FUNC_GET_MANUAL_DATA)(void);
int Func_GET_MANUAL_DATA(void)
{
	//TODO Get Expiry date
	//TODO Get Cvv2

//	int nRet;
	int nLen = 0;

	memset(gszExpriryDate, 0, sizeof(gszExpriryDate));
	memset(gszCvv, 0, sizeof(gszCvv));

	ASSERT_L3_RET(PubInputDate(NULL, "Enter Expriry Date(YYMM):", gszExpriryDate, INPUT_DATE_MODE_YYMM_NULL, 60));
	if (GetVarIsNeedCVV2() == YES)
	{
		ASSERT_L3_RET(PubInputDlg(NULL, "Enter CVV2:", gszCvv, &nLen, 0, 4, 60, INPUT_MODE_NUMBER));
	}

	return L3_ERR_SUCC;
}


/**
* @brief  get emv online pin
* @return 
* @li 
* @li 
*/
void EmvGetOnlinePin(char *psOnlinePin)
{
	if (NULL != psOnlinePin)
	{
		memcpy(psOnlinePin, gsPINBlock, 8);
	}
	return ;
}

/**
* @brief  set emv online pin
* @return void
* @li 
* @li 
*/
void EmvSetOnlinePin(char *psOnlinePin)
{
	if (NULL != psOnlinePin)
	{
		memcpy(gsPINBlock, psOnlinePin, 8);
	}
}

/**
* @brief  clear emv online pin
* @return void
* @li 
* @li 
*/
void EmvClrOnlinePin()
{
	memset(gsPINBlock, 0, sizeof(gsPINBlock));
}

//Assignment callback 
void SetupCallbackFunc(void)
{
	NAPI_L3SetCallback(L3_CALLBACK_DEBUG, Func_EmvDebug); //You'd better setup 'DEBUG' in the first place
	NAPI_L3SetCallback(L3_CALLBACK_UI_EVENT, Func_UI_Event);
	NAPI_L3SetCallback(L3_CALLBACK_GET_PIN, Func_GET_PIN);
	NAPI_L3SetCallback(L3_CALLBACK_GET_AMOUNT, Func_GET_AMOUNT);
	NAPI_L3SetCallback(L3_CALLBACK_SELECT_CANDIDATE_LIST, Func_SELECT_CANDIDATE_LIST);
	NAPI_L3SetCallback(L3_CALLBACK_SELECT_ACCOUNT_TYPE, Func_SELECT_ACCOUNT_TYPE);
	NAPI_L3SetCallback(L3_CALLBACK_SELECT_LANGUAGE, Func_SELECT_LANGUAGE);
	NAPI_L3SetCallback(L3_CALLBACK_CHECK_CREDENTIALS, Func_CHECK_CREDENTIALS);
	NAPI_L3SetCallback(L3_CALLBACK_VOICE_REFERRALS, Func_VOICE_REFERRALS);
	NAPI_L3SetCallback(L3_CALLBACK_DEK_DET, Func_DekDet);
	NAPI_L3SetCallback(L3_CALLBACK_AFTER_FINAL_SELECT, Func_AFTER_FINAL_SELECT);
	NAPI_L3SetCallback(L3_CALLBACK_CARD_DETECT_EVENT, Func_CARD_DETECT_EVENT);
	NAPI_L3SetCallback(L3_CALLBACK_GET_MANUAL_DATA, Func_GET_MANUAL_DATA);
}

void SaveInputAMT(char* pszAmt)
{
	memcpy(gszAmount, pszAmt, 12);
}

void InitInputAMT(void)
{
	memset(gszAmount, 0, sizeof(gszAmount));
}

void GetCardEventData(char *pszOut)
{
    strcpy(pszOut, gszCardEventData);
}

void GetManualExpiryDate(char *pszOut)
{
    memcpy(pszOut, gszExpriryDate, 4);
}

void GetManualCVV(char *pszOut)
{
    memcpy(pszOut, gszCvv, 4);
}

/**
** brief:  Shield section of card number,replace with *
** param [in]: uchar *pCardPan, 	  original card number
** param [in]: const uint nPanLen,    len of card number
** param [in]: const uchar cTransType  
** param [in]: const uint  nMode,   EMV status
** param [out]: uchar *pCardPan,    The card number after dealing
** return: 
** li: APP_SUCC
** li: APP_FAIL
** auther: 
** date: 2016-7-3
** modify: 
*/
int CtrlCardPan(uchar *pCardPan, const uint nPanLen, const uchar cTransType, const uint nMode)
{
	int i = 0;
	
	if (NULL == pCardPan)
	{
		return APP_FAIL;
	}

	if (19 < nPanLen || 12 > nPanLen) 
	{
		return APP_FAIL;
	}

	switch (cTransType) 
	{
	case TRANS_SALE:
	case TRANS_ADJUST:
	case TRANS_CASHBACK:
#if 0 // jose hope
		if (nMode == EMV_STATUS_OFFLINE_SUCC || nMode == EMV_STATUS_OFFLINE_FAIL)
		{
			/**<off line trans not shield*/
			return APP_QUIT;
		}
#endif
		break;
	case TRANS_OFFLINE:
	case TRANS_VOID:
	case TRANS_REFUND:
	case TRANS_VOID_PREAUTH:
	case TRANS_AUTHCOMP:
	case TRANS_AUTHSALEOFF:
	case TRANS_VOID_AUTHSALE:
		break;
	case TRANS_PREAUTH:
		if(YES != GetVarIsPreauthShieldPan())
		{
			return APP_QUIT;
		}
		break;
	default:
		return APP_QUIT;
	}

	if (YES != GetVarIsShieldPan())
	{
		return APP_QUIT;
	}

	//memset(pCardPan + 6, '*', nPanLen - 6 - 4);
	for(i = 0; i<(nPanLen-4);i++)//shou last 4 letters
	{
		pCardPan[i] = '*';
	}
	
	return APP_SUCC;
}

/**
** brief: Show card number
** param [out]: 
** return: 
** li: APP_SUCC
** li: APP_FAIL
** auther: 
** date: 2016-7-3
** modify: 
*/
int DispPan(char *pszPan)
{
	int nTimeOut = 60;
	char szMsg[64+1];
	
	PubClear2To4();
	sprintf(szMsg, "PAN: %s\n", pszPan);
	ASSERT_FAIL(PubConfirmDlg(NULL, szMsg, 0, nTimeOut));
	
	return APP_SUCC;
}

/**
* @brief get ServiceCode from magnetic track 2
* @param in  const char *pszTk2
* @param out   char *pszServiceCode 
* @return 
* @li APP_SUCC 
* @li APP_FAIL
*/
int GetServiceCodeFromTk2(const char *pszTk2, char *pszServiceCode)
{
	int i;

	for(i=0; i<strlen(pszTk2); i++)
	{
		if (pszTk2[i] == '=')
		{
			if (NULL == pszServiceCode)
			{
				return APP_FAIL;
			}
			memcpy(pszServiceCode, &pszTk2[i+5], 3);
			return APP_SUCC;
		}
	}
	return APP_FAIL;
}

static int GetCredemtialData(unsigned char tlv_list[], int tlv_len, uchar *pcType, char *pszCerNumber)
{
	int i = 0;
	int nRet = 0;
	tlv_t stTlvObj[5]; 

	memset(stTlvObj, 0, sizeof(stTlvObj));
	nRet = TlvParse(tlv_list, tlv_len, stTlvObj, 5);
	if (nRet != 0)
	{
		TRACE("nRet = %d", nRet);
		return APP_FAIL;
	}

	for(i=1; i <= stTlvObj[0].nChildNum; i++)
	{
		TRACE("stTlvObj[i].unTagName = %x", stTlvObj[i].unTagName);
		switch(stTlvObj[i].unTagName)
		{
		case 0x9F62:
			*pcType = stTlvObj[i].pusValue[0];
			break;
		case 0x9F61:
			memcpy(pszCerNumber, stTlvObj[i].pusValue, stTlvObj[i].unValueLen);
			break;
		default:
			break;
		}
	}
	return APP_SUCC;
}

static int Pinpad_UI_Event(char *pszData)
{
	int nOff = 0;
	int nUiEventDataLen;
	uint unEventId = pszData[nOff];

	nOff += 1;
	PubC2ToInt((uint *)&nUiEventDataLen, (uchar *)pszData + nOff);
	nOff += 2;// uiEventData len
	if (unEventId == UI_PRESENT_CARD)
	{
		PubClear2To4();
		//UI_PresentCard(pszData[nOff]);
		PubDisplayStr(DISPLAY_MODE_CENTER, 3, 1, "Please Use Card On Pinpad");
		PubUpdateWindow();
		return APP_SUCC;
	}
	else if (unEventId == 0x06) // UI pin status
	{
		int nRet;
		PubC4ToInt((uint *)&nRet, (uchar *)pszData + nOff);
		TRACE("nRet = %d", nRet);
		return APP_SUCC;
	}

	return  Func_UI_Event(unEventId, (uchar *)pszData + nOff);
}

static int CandidateParse(unsigned char tlv_list[], int tlv_len, L3_CANDIDATE_LIST *pstCandidate)
{
	int i = 0;
	int nRet = 0;
	tlv_t stTlvObj[30]; 

	memset(stTlvObj, 0, sizeof(stTlvObj));
	nRet = TlvParse(tlv_list, tlv_len, stTlvObj, 30);
	if (nRet != 0)
	{
		TRACE("nRet = %d", nRet);
		return APP_FAIL;
	}

	for(i=1; i <= stTlvObj[0].nChildNum; i++)
	{
		TRACE("stTlvObj[i].unTagName = %x %d", stTlvObj[i].unTagName, stTlvObj[i].unValueLen);
	//	TRACE_HEX(stTlvObj[i].pusValue, stTlvObj[i].unValueLen, "stTlvObj[i].unValue");
		switch(stTlvObj[i].unTagName)
		{
		case 0x9F40:
			memcpy(pstCandidate->terminalCodeTable, stTlvObj[i].pusValue, stTlvObj[i].unValueLen);
			break;
		case 0x9F12:
			memcpy(pstCandidate->perferName , stTlvObj[i].pusValue, stTlvObj[i].unValueLen);
			pstCandidate->perferNameLen = stTlvObj[i].unValueLen;
			break;
		case 0x50:
			memcpy(pstCandidate->lable , stTlvObj[i].pusValue, stTlvObj[i].unValueLen);
			pstCandidate->lableLen = stTlvObj[i].unValueLen;
			break;
		case 0x4F:
			memcpy(pstCandidate->aid , stTlvObj[i].pusValue, stTlvObj[i].unValueLen);
			pstCandidate->aidLen = stTlvObj[i].unValueLen;
			break;
		case 0x9F11:
			memcpy(&pstCandidate->issuerCodeTableIndex , stTlvObj[i].pusValue, 1);
			break;
#if 0
		case 0x5F2D:
			memcpy(&pstCandidate->languagePreference , stTlvObj[i].pusValue, stTlvObj[i].unValueLen);
			break;
		case 0xDF37:
			memcpy(&pstCandidate->kernelId , stTlvObj[i].pusValue, stTlvObj[i].unValueLen);
			break;
		case 0xDF65:
			memcpy(&pstCandidate->terminalPriority , stTlvObj[i].pusValue, 1);
			break;
		case 0x87:
			memcpy(&pstCandidate->priority , stTlvObj[i].pusValue, 1);
			break;
#endif
		case 0x1F811F:
			if (stTlvObj[i].unValueLen > 256)
			{
				TRACE("stTlvObj[i].unValueLen = %d", stTlvObj[i].unValueLen);
				stTlvObj[i].unValueLen = 256;
			}
			memcpy(pstCandidate->customTagData , stTlvObj[i].pusValue, stTlvObj[i].unValueLen);
			pstCandidate->customDataSize = stTlvObj[i].unValueLen;
			break;
		default:
			break;
		}
	}
	return APP_SUCC;
}

static int Pinpad_SELECT_CANDIDATE_LIST(char *pszData)
{
	L3_CANDIDATE_LIST candidateList[10] = {{0}};
	STPINPADL3_IN stL3Param;
	char szCandidate[1024] = {0};
	char szInputData[1024]= {0};
	uchar szAid[10][16+1] = {{0}};
	uchar szLable[10][20+1] = {{0}};
	uchar szPerferName[10][20+1] = {{0}};
	uchar customTagData[10][256+1] = {{0}};
	char szTlvList[32+1] = {0};
	int nResult = 0; // RFU
	int nListNum;
	int nTlvLen;
	int nOff = 0, nRet, nSelect, i;
	int nCandidateLen = 0;
	char cSelectInPinpad = YES;

	memset((char *)&stL3Param, 0, sizeof(STPINPADL3_IN));
	nListNum = pszData[nOff];
	nOff += 1;

	for (i = 0; i < nListNum; i++)
	{
		PubC2ToInt((uint *)&nTlvLen, (uchar *)pszData + nOff);
		nOff += 2;
		candidateList[i].aid = szAid[i];
		candidateList[i].lable = szLable[i];
		candidateList[i].perferName = szPerferName[i];
		candidateList[i].customTagData = customTagData[i];
		nRet = CandidateParse((uchar *)pszData + nOff, nTlvLen, &candidateList[i]);
		if (nRet != APP_SUCC)
		{
			TRACE("nRet = %d", nRet);
			return APP_FAIL;
		}
		if (cSelectInPinpad == YES)
		{
			//if (memcmp(candidateList[i].aid, "", 16) == 0){} select what you want via aid
			PubIntToC2((uchar *)szCandidate+nCandidateLen, (uint)nTlvLen);
			nCandidateLen += 2;
			memcpy(szCandidate+nCandidateLen, (uchar *)pszData + nOff, nTlvLen);
			nCandidateLen += nTlvLen;
		}
		nOff += nTlvLen;
	}

	if (cSelectInPinpad == NO)
	{
		nRet = Func_SELECT_CANDIDATE_LIST(candidateList, nListNum, &nSelect);
		if (nRet != L3_ERR_SUCC)
		{
			TRACE("nRet = %d", nRet);
			return APP_FAIL;
		}
	}

	nOff = 0;
	szInputData[nOff] = 0xFF;
	nOff += 1;
	PubIntToC4((uchar *)szInputData + nOff, (uint)nResult);
	nOff += 4;
	nOff += 4;// data Length
	if (cSelectInPinpad == NO)
	{
		szInputData[nOff] = nSelect; // index
		nOff += 1;
		nTlvLen = 0;
		TlvAdd(0x9F06, candidateList[nSelect].aidLen, (char *)candidateList[nSelect].aid, szTlvList, &nTlvLen);
		memcpy(szInputData + nOff, szTlvList, nTlvLen);
		nOff += nTlvLen;
	}
	else
	{
		szInputData[nOff] = 0xFF; // index
		nOff += 1;
		memcpy(szInputData + nOff, szCandidate, nCandidateLen);
		nOff += nCandidateLen;
	}

	PubIntToC4((uchar *)szInputData + 5, (uint)nOff - 9);// 9 = L3_callback_id(0xFF) + result length + data length
	stL3Param.cMsgType = PINPAD_L3CALLBACK;
	stL3Param.pszInputData = szInputData;
	stL3Param.nInputDataLen = nOff;
	return PinPad_L3PerformSend((char *)&stL3Param);
}

static int Pinpad_AFTER_FINAL_SELECT(char *pszData)
{
	L3_CARD_INTERFACE cardInterface;
	STPINPADL3_IN stL3Param;
	int nOff = 0;
	int nAidLen;
	int nTlvLen = 0;
	int nResult = 0; // RFU
	char szInputData[64] = {0};
	char szTlvList[32+1] = {0};
	char szAid[16] = {0};

	memset((char *)&stL3Param, 0, sizeof(STPINPADL3_IN));
	cardInterface = pszData[nOff];
	nOff += 1;
	PubC2ToInt((uint *)&nAidLen, (uchar *)pszData + nOff);
	nOff += 2;
	memcpy(szAid, pszData + nOff, nAidLen);

	nOff = 0;
	szInputData[nOff] = 0xFF;
	nOff += 1;
	PubIntToC4((uchar *)szInputData + nOff, (uint)nResult);
	nOff += 4;
	nOff += 4; // data length
	if (L3_CONTACT == cardInterface)
	{
		if (0 == memcmp(szAid, "\xA0\x00\x00\x00\x04\x30\x60", 7))
		{
		    TlvAdd(_EMVPARAM_9F09_APPVER, 2, "\x00\x02", szTlvList, &nTlvLen);
		}
	}
	else //CONTACTLESS
	{
		if (memcmp(szAid, "\xA0\x00\x00\x03\x33", 5) == 0)
		{
		    TlvAdd(_EMVPARAM_5F2A_TRANSCCODE, 2, "\x01\x56", szTlvList, &nTlvLen);
		}
		else if (memcmp(szAid, "\xA0\x00\x00\x00\x04", 5) == 0 && gcTransType == TRANS_REFUND)
		{
			// according to the M/Chip Requirements a refund should lead to an AAC(test case:MCD01.Test.02.Scenario.02)
			TlvAdd(_EMVPARAM_DF13_TACDENIAL, 5, "\xFF\xFF\xFF\xFF\xFF", szTlvList, &nTlvLen);
		}
	}
	memcpy(szInputData + nOff, szTlvList, nTlvLen);
	nOff += nTlvLen;

	PubIntToC4((uchar *)szInputData + 5, (uint)nOff - 9); // 9 = L3_callback_id(0xFF) + result length + data length
	stL3Param.cMsgType = PINPAD_L3CALLBACK;
	stL3Param.pszInputData = szInputData;
	stL3Param.nInputDataLen = nOff;
	return PinPad_L3PerformSend((char *)&stL3Param);
}

static int Pinpad_GET_PIN(char *pszData)
{
	int nOff;
	int nIndex;
	int nTlvLen = 0;
	int nResult = 0; // skpi pin (0xFFFFFFFD = -610)
	char szInputData[128] = {0};
	char szTlvList[64+1] = {0};
	char szTmpData[8] = {0};
	L3_PIN_TYPE type;
	STPINPADL3_IN stL3Param;

	if (nResult != PERFORM_SKIPPIN)
	{
		PubClear2To4();
		PubDisplayStr(DISPLAY_MODE_CENTER, 3, 1, "Please Enter PIN On Pinpad");
		PubUpdateWindow();
	}

	memset(&stL3Param, 0, sizeof(STPINPADL3_IN));
	nOff = 0;
	type = pszData[nOff];
	nOff += 1;

	// package send data
	nOff = 0;
	szInputData[nOff] = 0xFF;
	nOff += 1;
	PubIntToC4((uchar *)szInputData + nOff, (uint)nResult);
	nOff += 4;
	nOff += 4; // data length
	if (type == L3_PIN_ONLINE)
	{
		// KeyType
		szTmpData[0] = PubGetKeySystemType();
		TlvAdd(0x1F8136, 1, szTmpData, szTlvList, &nTlvLen);

		// key index
		GetVarMainKeyNo(&nIndex);
		szTmpData[0] = nIndex;
		TlvAdd(0x1F8137, 1, szTmpData, szTlvList, &nTlvLen);
	}
	TlvAdd(0x1F8135, 10, "\x00\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c", szTlvList, &nTlvLen);
	// timeout
	szTmpData[0] = 60;
	TlvAdd(0x1F8138, 1, szTmpData, szTlvList, &nTlvLen);

	memcpy(szInputData + nOff, szTlvList, nTlvLen);
	nOff += nTlvLen;
	
	PubIntToC4((uchar *)szInputData + 5, (uint)nOff - 9);
	
	stL3Param.cMsgType = PINPAD_L3CALLBACK;
	stL3Param.pszInputData = szInputData;
	stL3Param.nInputDataLen = nOff;
	return PinPad_L3PerformSend((char *)&stL3Param);
}

static int Pinpad_CHECK_CREDENTIALS(char *pszData)
{
	uchar ucType = 0;
    char szNumber[50] = {0};
    char szBuf[100] = {0};
	int nRet = 0;
    int nLen = 1;
	int nOff;
	int nTlvLen;
	int nResult = 0;
	STPINPADL3_IN stL3Param;

	memset(&stL3Param, 0, sizeof(STPINPADL3_IN));
    char *szTypestr[] = 
	{
		tr("ID CARD"),
		tr("OFFICER"), 
		tr("PASSPORT"),
		tr("ENTRY CARD"),
		tr("TEMP IDCARD"), 
		tr("OTHER")
	};

	nOff = 0;
	PubC2ToInt((uint *)&nTlvLen, (uchar *)pszData + nOff);
	nOff += 2;
	ASSERT_FAIL(GetCredemtialData((uchar *)pszData + nOff, nTlvLen, &ucType, szNumber));
	if (ucType <= 0x05 || strlen(szNumber) <= 0)
	{
	    nRet = L3_ERR_FAIL;
	}

	if (nRet != L3_ERR_FAIL)
	{
		NAPI_ScrPush();
	    sprintf(szBuf, "TYPE:%s\nCARD NO:%*.*s", szTypestr[ucType], nLen, nLen, szNumber);
	    nRet = PubConfirmDlg("CHECK CREDENTIALS", szBuf, 1, 60);
		if (nRet != APP_SUCC)
		{
			if(nRet == APP_QUIT)
			{
				nRet = L3_ERR_CANCEL;
			}
			else if (nRet == APP_TIMEOUT)
			{
			    nRet = L3_ERR_TIMEOUT;
			}
			nRet = L3_ERR_FAIL;
		}
	}
	NAPI_ScrPop();

	nOff = 0;
	pszData[nOff] = 0xFF;
	nOff += 1;
	nResult = nRet;
	PubIntToC4((uchar *)pszData + nOff, nResult);
	nOff += 4;
	nOff += 4; // data length
	PubIntToC4((uchar *)pszData + 5, (uint)nOff - 9);
	stL3Param.cMsgType = PINPAD_L3CALLBACK;
	stL3Param.pszInputData = pszData;
	stL3Param.nInputDataLen = nOff;
	return PinPad_L3PerformSend((char *)&stL3Param);
}

static int Pinpad_CardConfirm(char *pszData)
{
	char cEntryMode;
	char szInputData[32]= {0};
	STPINPADL3_IN stL3Param;
	int nLen, nRet, nOff = 0, nResult;
	char szPan[19+1] = {0};

	memset((char *)&stL3Param, 0, sizeof(STPINPADL3_IN));
	TxnL3GetData(L3_DATA_POS_ENTRY_MODE, &cEntryMode, 1);
	if(cEntryMode == ATTR_CONTACT || cEntryMode == ATTR_CONTACTLESS)
	{
		//return APP_SUCC;
	}

	nLen = TxnL3GetData(L3_DATA_PAN, szPan, 19);
	if (nLen <= 0)
	{
		TRACE("nLen = %d", nLen);
		return APP_FAIL;
	}
	nRet = DispPan(szPan);
	if (nRet != APP_SUCC)
	{
		nRet = L3_ERR_FAIL;
	}
	nOff = 0;
	szInputData[nOff] = 0xFF;
	nOff += 1;
	nResult = nRet;
	PubIntToC4((uchar *)szInputData + nOff, nResult);
	nOff += 4;
	nOff += 4; // data length
	PubIntToC4((uchar *)szInputData + 5, (uint)nOff - 9);
	stL3Param.cMsgType = PINPAD_L3CALLBACK;
	stL3Param.pszInputData = szInputData;
	stL3Param.nInputDataLen = nOff;
	return PinPad_L3PerformSend((char *)&stL3Param);
}

int PerformCallBack(char cCallbackId, char *pszInput)
{
	switch (cCallbackId)
	{
	case L3_CALLBACK_UI_EVENT:
		ASSERT_FAIL(Pinpad_UI_Event(pszInput));
		return APP_SUCC;
	case L3_CALLBACK_SELECT_CANDIDATE_LIST:
		return Pinpad_SELECT_CANDIDATE_LIST(pszInput);
	case L3_CALLBACK_AFTER_FINAL_SELECT:
		return Pinpad_AFTER_FINAL_SELECT(pszInput);
	case L3_CALLBACK_GET_PIN:
		return Pinpad_GET_PIN(pszInput);
	case L3_CALLBACK_CHECK_CREDENTIALS:
		return Pinpad_CHECK_CREDENTIALS(pszInput);
	case 0x0D: // card confirm
		return Pinpad_CardConfirm(pszInput);
	default:
		TRACE("callbackid = %x", cCallbackId);
		return APP_FAIL;
	}
	
	return APP_SUCC;
}

static int PerformTransResTlvParse(unsigned char tlv_list[], int tlv_len, STSYSTEM *pstSystem)
{
	int i = 0;
	int nRet = 0;
	char szExpire[6+1] = {0};
	tlv_t stTlvObj[20]; 

	memset(stTlvObj, 0, sizeof(stTlvObj));
	nRet = TlvParse(tlv_list, tlv_len, stTlvObj, 20);
	if (nRet != 0)
	{
		TRACE("nRet = %d", nRet);
		return APP_FAIL;
	}

	for(i = 1; i <= stTlvObj[0].nChildNum; i++)
	{
		//TRACE("stTlvObj[i].unTagName = %x i == %d nChildNum = %d", stTlvObj[i].unTagName, i, stTlvObj[0].nChildNum);
		//TRACE_HEX(stTlvObj[i].pusValue, stTlvObj[i].unValueLen, "stTlvObj[i].unValue");
		switch(stTlvObj[i].unTagName)
		{
		case 0x9F39:
			pstSystem->cTransAttr = stTlvObj[i].pusValue[0];
			PubHexToAsc((uchar *)&pstSystem->cTransAttr, 2, 0, (uchar* )pstSystem->szInputMode);
			break;
		case 0x5F20:
			memcpy(pstSystem->szHolderName, stTlvObj[i].pusValue, stTlvObj[i].unValueLen);
			break;
		case 0x5F24:
			memcpy(szExpire, stTlvObj[i].pusValue, stTlvObj[i].unValueLen);
			memcpy(pstSystem->szExpDate, szExpire, 4);
			break;
		case 0x1F8154:
			if(stTlvObj[i].pusValue[0] == 1)
			{
				pstSystem->cPinAndSigFlag |= CVM_SIG;
			}
			break;
		case 0x1F8155:
			EmvSetOnlinePin((char *)stTlvObj[i].pusValue);
			break;
		default:
			break;
		}
	}
	return APP_SUCC;
}

static int Pinpad_inputExpiryDate(char *pszExpiry)
{
	char szData[64] = {0};
	int nDatalen = 0;
	int nDlgNum = 0;
	STINPUTDLG stInputDlg[8] = {{0}};
	STOUTDLG stOutDlg;

	PubClear2To4();
	PubDisplayStrInline(0, 3, tr("Please Input Expiry Date"));
	PubUpdateWindow();

	memset(&stOutDlg, 0, sizeof(STOUTDLG));
	stInputDlg[nDlgNum].cInputFieldID = DLG_ID_EXIPRYDATE;
	stInputDlg[nDlgNum].cInputMode = 0x00;
	stInputDlg[nDlgNum].cInputFormat = EXPIRYDATE_MMYY;
	stInputDlg[nDlgNum].cMinLen = 4;
	stInputDlg[nDlgNum].cMaxLen = 4;
	stInputDlg[nDlgNum].unTimeout = 60;
	nDlgNum++;

	if (PubInputDlg_PINPAD(szData, nDatalen, nDlgNum, stInputDlg, &stOutDlg) != APP_SUCC)
	{
		return APP_QUIT;
	}
	memcpy(pszExpiry, stOutDlg.szExpireDate, 4);

	return APP_SUCC;
}

static int Pinpad_inputCVV(char *pszCVV)
{
	char szData[64] = {0};
	int nDatalen = 0;
	int nDlgNum = 0;
	STINPUTDLG stInputDlg[8] = {{0}};
	STOUTDLG stOutDlg;

	PubClear2To4();
	PubDisplayStrInline(0, 3, tr("Please Input CVV"));
	PubUpdateWindow();

	memset(&stOutDlg, 0, sizeof(STOUTDLG));
	stInputDlg[nDlgNum].cInputFieldID = DLG_ID_CVV;
	stInputDlg[nDlgNum].cInputMode = 0x00;
	stInputDlg[nDlgNum].cInputFormat = 0xFF;
	stInputDlg[nDlgNum].cMinLen = 3;
	stInputDlg[nDlgNum].cMaxLen = 4;
	stInputDlg[nDlgNum].unTimeout = 60;
	nDlgNum++;

	if (PubInputDlg_PINPAD(szData, nDatalen, nDlgNum, stInputDlg, &stOutDlg) != APP_SUCC)
	{
		return APP_QUIT;
	}
	memcpy(pszCVV, stOutDlg.szCvv, 4);
	return APP_SUCC;
}

int PinPad_PerformTransaction(char *pszInput, int nInPutLen, L3_TXN_RES *res, STSYSTEM *pstSystem, char *pszResPonseCode)
{
	char sRecvBuf[2048+1] = {0};
	int nRet, nErrCode;
	int nOff = 0;
	int nTlvLen;

	nRet = PinPad_L3PerformTransaction(pszInput, nInPutLen, sRecvBuf, NULL);
	if (nRet != APP_SUCC)
	{
		TRACE("nRet = %d", nRet);
		return APP_FAIL;
	}

	memcpy(pszResPonseCode, "00", 2);
	*res = sRecvBuf[nOff];
	nOff += 1;
	pstSystem->cCvmStatus = sRecvBuf[nOff];
	nOff += 1;
	PubC4ToInt((uint *)&nErrCode, (uchar *)sRecvBuf + nOff);
	nOff += 4;
	// shecm id
	nOff += 1;
	PubC4ToInt((uint *)&nTlvLen, (uchar *)sRecvBuf + nOff);
	nOff += 4;
	PerformTransResTlvParse((uchar *)sRecvBuf + nOff, nTlvLen, pstSystem);
	if (nErrCode == PERFORM_MANUNAL)
	{
		memset(gszCardEventData, 0, sizeof(gszCardEventData));
		memset(gszExpriryDate, 0, sizeof(gszExpriryDate));
		memset(gszCvv, 0, sizeof(gszCvv));

		TxnL3GetData(L3_DATA_PAN, gszCardEventData, 19);
		nRet = Pinpad_inputExpiryDate(gszExpriryDate);
		if (nRet != APP_SUCC)
		{
			return APP_QUIT;
		}

		nRet = Pinpad_inputCVV(gszCvv);
		if (nRet != APP_SUCC)
		{
			return APP_QUIT;
		}
		*res = L3_TXN_ONLINE;
		return APP_SUCC;
	}
	return nErrCode;
}

void ResetVirtualkbStatus()
{
	gcIsCreateVirtualKb = NO;
}

void SetCurTransType(char cTransType)
{
	gcTransType = cTransType;
}


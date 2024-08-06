/***************************************************************************
** Copyright (c) 2019 Newland Payment Technology Co., Ltd All right reserved
** File name:  main.c
** File indentifier:
** Synopsis:
** Current Verion:  v1.0
** Auther: sunh
** Complete date: 2016-7-1
** Modify record:
** Modify date:
** Version:
** Modify content:
***************************************************************************/

#include "apiinc.h"
#include "libapiinc.h"
#include "appinc.h"
#include <locale.h>
#include <langinfo.h>


STSYSTEM stSystemSafe;
int lockGuard = 0;


/**
* internal function statement
*/
static int AppTrans(void);
static int ReprintMenu(void);
static int MenuFuncSel(void);
static int MenuDefault(void);
static int FirstRunChk(void);
static int AppInit(void);
static int ChkPdAndRF(void);

int main(void)
{
	STUIFUN_STYLE stUiFunStyle;

	InitException();
	PubInitUiParam();
	memset(&stUiFunStyle, 0, sizeof(STUIFUN_STYLE));
	stUiFunStyle.emMenu = 0;
	strcpy(stUiFunStyle.szPageDown, "->");
	strcpy(stUiFunStyle.szPageUp, "<-");
	PubSetUiStyle(stUiFunStyle);
	PubClearAll();
	PubKbHit();					/*Clear key cache*/

	/**
	* First launch check. If it's lauched for the fisrt time, many files are created in 'FirstRunChk'.
	*/
	FirstRunChk();
	/**
	* module initialize , important !!!
	*/
	AppInit();

#ifdef USE_NTOMS
	if (NTOMS_proInit() != APP_SUCC)
	{
		PubMsgDlg("Error", "Init Toms Fail", 0, 1);
	}
#endif

	/*
	* Run task
	*/
	while(1)
	{
		/**
		* Communication Initialize
		*/
		CommInit();

#ifdef USE_NTOMS
		NTOMS_CheckUpdate();
#endif
		/**
		* Enter default screen
		*/
		if (APP_QUIT == AppTrans())
		{
			CommDump();
			SetStatusBar(STATUSBAR_OTHER_CLOSE);
			DestroyCustomizedFont();
			return APP_QUIT;
		}
	}
}

/**
** brief: Run application firstly and initialize
** param [in]:
** param [out]:
** return:
** auther: sunh
** date: 2016-7-1
** modify:
*/
static int FirstRunChk(void)
{
	char szVersion[32+1] = {0};
	char szLanguage[1+1] = {0};
	int nLen;
	char szCmd[32] = {0};

	if (APP_SUCC != IsFirstRun())
	{
		SetCustomizedFont(LANG_EN_US);
		/**
		* Import default parameter
		*/
		PubClearAll();
		PubDisplayGen(2, tr("INITIALIZE"));
		PubDisplayGen(3, tr("PLEASE WAIT...."));
		PubUpdateWindow();
		InitCommParam();
		InitBatchFile();
		PubDeReclFile(LASTSETTLEFILE);
		EmvClearRecord();
		ASSERT_FAIL(InitPosDefaultParam());
		sprintf(szCmd, "mkdir %s", ELECSIGN_PATH);
		system(szCmd);
	}

	if (GetTag(FILE_APPPOSPARAM, TAG_LANGUAGE, &nLen, szLanguage) == APP_SUCC)
	{
		SetCustomizedFont(szLanguage[0]);
		TRACE("szLanguage[0] = %d", szLanguage[0]);
	}
	else
	{
		PubMsgDlg("WARNING", "GET TAG_LANGUAGE FAIL", 0, 1);
	}

	GetTag(FILE_APPPOSPARAM, TAG_VERSION, &nLen, szVersion);
	if (memcmp(szVersion, APP_VERSION, strlen(APP_VERSION)) != 0)
	{
		UpdateTagParam(FILE_APPPOSPARAM, TAG_VERSION, strlen(APP_VERSION), APP_VERSION);
		InitExPosParam();
	}

	if (PubFsExist(ELECSIGN_PATH) == NAPI_ERR) {
		sprintf(szCmd, "mkdir %s", ELECSIGN_PATH);
		system(szCmd);
	}

	return APP_SUCC;
}

/**
** brief: Close default screen
** param: void
** return: void
** auther:
** date: 2016-7-3
** modify:
*/
static void DisableDispDefault(void)
{
	PubClearAll();
	ShowLightOff();
	SetStatusBar(STATUSBAR_STANDBY_CLOSE);
	return ;
}

void DispScreenSaver_U1000(void)
{
	int i, nPicNum = 3;
	ST_PADDATA Paddata;

	SetStatusBar(STATUSBAR_OTHER_CLOSE);
	while(1)
	{
		for (i = 0 ; i < nPicNum; i++)
		{
			PubClearAll();
			PubDisplayLogo("screensaver1.png", 0, 160*i);
			PubUpdateWindow();
			while (1) {
				if (NAPI_KbGetPad(&Paddata, 3*1000) == NAPI_ERR_TIMEOUT) {
					break;
				}
				if (Paddata.emPadState == PADSTATE_UP) {
					SetStatusBar(STATUSBAR_OTHER_OPEN);
					return;
				} else if (Paddata.emPadState == PADSTATE_KEY) {
					SetStatusBar(STATUSBAR_OTHER_OPEN);
					return;
				}
			}
		}
	}
}

void DispScreenSaver(void)
{
	if (strcmp(PubGetPosTypeStr(), "U1000") == 0) {
		DispScreenSaver_U1000();
		return;
	}

	SetStatusBar(STATUSBAR_OTHER_CLOSE);
	while(1)
	{
		PubClearAll();
		PubDisplayLogo("screensaver2.png", 0, 0);
		PubUpdateWindow();
		if (TxnWaitAnyKey(3) != 0) {
			SetStatusBar(STATUSBAR_OTHER_OPEN);
			return;
		}

		PubClearAll();
		PubDisplayLogo("screensaver1.png", 0, 40);
		PubUpdateWindow();
		if (TxnWaitAnyKey(3) != 0) {
			SetStatusBar(STATUSBAR_OTHER_OPEN);
			return;
		}
	}
}

/**
** brief: Show default screen
** param: void
** return: void
** auther:
** date: 2016-7-4
** modify:
*/
static void EnableDispDefault(void)
{
	char szSoftVer[16+1] = {0};
	char szShowInfo[16+1]={0};
	//uint unWspace, unHspace;
	int nMaxLine;
	//open status bar
	SetStatusBar(STATUSBAR_STANDBY_OPEN);
	PubGetDispView(&nMaxLine ,NULL);
	szSoftVer[0] = 'V';
	GetVarSoftVer(&szSoftVer[1]);

	if(GetVarIsSwipe() == YES)
	{
		strcpy(szShowInfo, tr("SWIPE"));
	}
	if (GetVarIsSupportContact() == YES)
	{
		if (GetVarIsSwipe() == YES)
		{
			strcpy(szShowInfo, tr("SWIPE/INSERT"));
		}
		else
		{
			strcpy(szShowInfo, tr("INSERT"));
		}
	}
	if (GetVarIsPinpadReadCard() == YES)
	{
		strcpy(szShowInfo, "");
	}
	PubClearAll();
	PubDisplayLogo("DispLogo.png", 4, 26);

	if (PubGetKbAttr() == KB_VIRTUAL) {
		PubDisplayGen(nMaxLine/2 + 1, szShowInfo);
		PubDisplayStrInlines(DISPLAY_ALIGN_SMALLFONT, 0, nMaxLine*2 - 3, PubGetPosTypeStr());
		PubDisplayStrInlines(DISPLAY_ALIGN_SMALLFONT, DISPLAY_MODE_TAIL, nMaxLine*2-3, szSoftVer);
	} else {
		PubDisplayGen(nMaxLine - 1, szShowInfo);
		PubDisplayStrInlines(DISPLAY_ALIGN_SMALLFONT, 0, nMaxLine*2, PubGetPosTypeStr());
		PubDisplayStrInlines(DISPLAY_ALIGN_SMALLFONT, DISPLAY_MODE_TAIL, nMaxLine*2, szSoftVer);
	}
	PubUpdateWindow();

	ShowLightIdle();
	return;
}

/**
** brief: Application initialize
** param [in]:
** param [out]:
** return:
** auther: sunh
** date: 2016-7-1
** modify:
*/
static int AppInit(void)
{
	int nRet = 0;

	PubClearAll();
	PubDisplayStrInline(0, 2, tr("Loading..."));
	PubDisplayStrInline(0, 4, tr("Please wait..."));
	PubUpdateWindow();
	nRet = InitISO8583();
	if (APP_SUCC != nRet)
	{
		PubMsgDlg("Warning", "Exporting Iso definition fail", 3, 10);
	}

	nRet = ExportPosParam();
	nRet += ExportCommParam();
	if (APP_SUCC != nRet)
	{
		PubMsgDlg("Warning", "POS param loses", 3, 10);
	}

	SetParamFromIni();

	nRet = ChkPdAndRF();
	if (nRet != APP_SUCC)
	{
		TRACE("nRet = %d", nRet);
	}
	TxnL3Init();
	//nRet = NAPI_LedSetColor(NAPI_LED_GREEN);
	nRet=NAPI_OK;
	if (nRet != NAPI_OK)
	{
		PubMsgDlg("Warning", "set LED fail", 1, 2);
	}

	// set keyboard attr
	if (strcmp(PubGetPosTypeStr(), "U1000") == 0)
	{
		PubSetKbAttr(KB_VIRTUAL);
	}
	else
	{
		PubSetKbAttr(KB_PHYSICAL);
	}

	return APP_SUCC;
}

/**
** brief: Enter into interface of transaction
** param [in]:
** param [out]:
** return:
** auther: sunh
** date: 2016-7-1
** modify:
*/
static int AppTrans(void)
{
	int nRet = 0;
	char cMenuSel = 0xff;			/*Enter default screen*/

	DealSettleTask();//deal settle
	while(1)
	{
		if (cMenuSel == 0)
		{
			/**
			* Enter main menu
			*/
			nRet = MenuFuncSel();
			if (nRet == KEY_FUNC_APP_SELECT)
			{
				return APP_QUIT;
			}
			if (nRet == APP_SUCC)
			{
				continue;
			}
		}
		else
		{
			/**
			* Enter default menu
			*/
		    MenuDefault();
		}
		cMenuSel = ~cMenuSel;
	}
	return APP_SUCC;
}

/**
* NewLand apps support two kinds of debug mode: port and file.
*
* 1)Port debug is adding debug statements in the program code.
* When a terminal is connected to PC through serial or USB cable, PC launch a serial port monitoring tool.
* Then serial port tool will receive debug log as debug statements execute. It is for programmer to locate
* issues. Port debug is usually used when the port of terminal is available.
* This mode is convenient to see debug log immediately.
*
* 2)File debug is similar to port debug, but before programmer calls a specific function (PubExportDebugFile)
* to export debug log, nobody will see the debug log. File debug mode is usually used when both serial and USB
* port of terminal are not available temporarily but available when a task ends.
*
*
* The following function provides a debug menu to turn on/off and choose port/file debug, see following.
* Once debug function is enabled, programmer can add debug statements to anywhere to debug app.
* for example,
* PubDebug("%d%s", i, str); 				//out formatted debuging text
* PubDebugData("title", buff, len);			//out text in hex mode
*
* or call the following macros to get more debug details such as line no and file name
* TRACE(("%d%s", i, str)); 					//out formatted debuging text
* TRACE_HEX((buff, len, "%d%s" i, str));	//out text in hex mode
*
* Debug level is not applicable for PubDebug/PubDebugData/TRACE/TRACE_HEX.
* It's only used to control debug level in Public APIs for locating issues of Public APIs.
*
*/
static int DebugMenu(void)
{
	STAPPCOMMPARAM stAppCommParam;
	char *pszItems1[] = {
		tr("1.Close"),
		tr("2.COM Mode"),
		tr("3.FILE Mode"),
		tr("4.CONSOLE Mode"),
		tr("5.Export Log"),
		tr("6.EMV Debug"),
	};
	char *pszItems2[] = {
		tr("1.RS232"), 
		tr("2.PINPAD"), 
		tr("3.USB"),
	};
	char *pszItems3[] = {
		tr("1.Normal"), 
		tr("2.Warning"), 
		tr("3.All")
	};
	
	int nSelect = 1, nStartItem = 1;

	nSelect = PubGetDebugMode() + 1;

	ASSERT_QUIT(PubShowMenuItems(tr("Debug"), pszItems1, sizeof(pszItems1)/sizeof(char *), &nSelect, &nStartItem,60));

	switch(nSelect)
	{
	case 1:
		PubSetDebugMode(DEBUG_NONE);
		TxnL3SetDebugMode(LV_CLOSE);
		GetAppCommParam(&stAppCommParam);
		if (stAppCommParam.cCommType == COMM_RS232)
		{
			SetControlCommInit();
			CommInit();
		}
		break;
	case 2:
		nSelect = PubGetDebugPortNum() + 1;
		ASSERT_QUIT(PubShowMenuItems(tr("Port Num"), pszItems2, sizeof(pszItems2)/sizeof(char *), &nSelect, &nStartItem,60));
		switch(nSelect)
		{
		case 1:
			PubSetDebugPort(RS232);
			break;
		case 2:
			PubSetDebugPort(PINPAD);
			break;
		case 3:
			PubSetDebugPort(USB_SERIAL);
			break;
		default:
			break;
		}
		PubSetDebugMode(DEBUG_PORT);
		nSelect = PubGetDebugLevel() + 1;

		ASSERT_QUIT(PubShowMenuItems(tr("Debug Level"), pszItems3, sizeof(pszItems3)/sizeof(char *), &nSelect, &nStartItem,60));
		switch(nSelect)
		{
		case 1:
		    PubSetDebugLevel(DEBUG_LEVEL_NORMAL);
			break;
		case 2:
		    PubSetDebugLevel(DEBUG_LEVEL_WARNING);
		   break;
		case 3:
		    PubSetDebugLevel(DEBUG_LEVEL_ALL);
			break;
		}
		break;
	case 3:
		PubSetDebugMode(DEBUG_FILE);
		nSelect = PubGetDebugLevel() + 1;
		ASSERT_QUIT(PubShowMenuItems(tr("Debug Level"), pszItems3, sizeof(pszItems3)/sizeof(char *), &nSelect, &nStartItem,60));
		switch(nSelect)
		{
		case 1:
		    PubSetDebugLevel(DEBUG_LEVEL_NORMAL);
			break;
		case 2:
		    PubSetDebugLevel(DEBUG_LEVEL_WARNING);
		   break;
		case 3:
		    PubSetDebugLevel(DEBUG_LEVEL_ALL);
			break;
		}
		break;
	case 4:
		PubSetDebugMode(DEBUG_CONSOLE);
		nSelect = PubGetDebugLevel() + 1;
		ASSERT_QUIT(PubShowMenuItems(tr("Debug Level"), pszItems3, sizeof(pszItems3)/sizeof(char *), &nSelect, &nStartItem,60));
		switch(nSelect)
		{
		case 1:
		    PubSetDebugLevel(DEBUG_LEVEL_NORMAL);
			break;
		case 2:
		    PubSetDebugLevel(DEBUG_LEVEL_WARNING);
		   break;
		case 3:
		    PubSetDebugLevel(DEBUG_LEVEL_ALL);
			break;
		}
		break;
	case 5:	
		ASSERT_QUIT(PubShowMenuItems(tr("Port Num"), pszItems2, sizeof(pszItems2)/sizeof(char *), &nSelect, &nStartItem,60));
		switch(nSelect)
		{
		case 1:
			PubSetDebugPort(RS232);
			break;
		case 2:
			PubSetDebugPort(PINPAD);
			break;
		case 3:
			PubSetDebugPort(USB_SERIAL);
			break;
		default:
			break;
		}
		PubExportDebugFile();//Even if 'PubExportDebugFile' is not invoked, the log will disappear after rebooting.
		break;
	case 6:
		MenuEmvSetDebug();
		break;
	default:
		break;
	}
	return APP_SUCC;
}

 static int ReportMenu()
 {
	int nRet, nSelcItem = 1, nStartItem = 1;
	int nRecordNum;
		char *pszItems[] = {
		tr("1.PRINT TOTAL"),
		tr("2.PRINT DETAIL"),
		tr("3.PRINT TOTAL & DETAIL"),
		tr("4.LOOK UP")
	};

	while (1)
	{
		nRet = PubShowMenuItems(tr("BANK DEMO"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 60);
		if (nRet == APP_QUIT || nRet == APP_TIMEOUT)
		{
			return nRet;
		}
		if (nSelcItem != 4 && PubIsSupportPrint() == NO) {
			PubMsgDlg(NULL, "NOSUPPORT PRINT", 3, 10);
			return APP_SUCC;
		}
		switch(nSelcItem)
		{
		case 1:
			PrnTotal();
			break;
		case 2:
			PrintAllRecord();
			break;
		case 3:
			GetRecordNum(&nRecordNum);
			if (nRecordNum <=0 )
			{
				char szDispBuf[100];
				PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szDispBuf, "|CNO TRANS");
				PubMsgDlg(tr("PRINT"), szDispBuf, 0, 5);
				return APP_QUIT;
			}
			ASSERT_QUIT(PrintTotal());
			PrintAllRecord();
			break;
		case 4:
			DoLookUp();
			break;
		default:
			break;
		}
	}
	
	return APP_SUCC;
 }

static int ReprintMenu()
{
	int nRecordNum = 0;
	STTRANSRECORD stTransRecord;
	int nRet, nSelcItem = 1, nStartItem = 1;
	char *pszItems[] = {
		tr("1.LASTTRX"),
		tr("2.TRACETRX"),
		tr("3.LAST SETTLEMENT"),
	};

	if (PubIsSupportPrint() == NO) {
		PubMsgDlg(NULL, "NOSUPPORT PRINT", 3, 10);
		return APP_SUCC;
	}

	nRet = PubShowMenuItems(tr("BANK DEMO"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 60);
	if (nRet == APP_QUIT || nRet == APP_TIMEOUT)
	{
		return nRet;
	}
	switch(nSelcItem)
	{
	case 1:
		memset(&stTransRecord, 0, sizeof(STTRANSRECORD));
		GetRecordNum(&nRecordNum);
		if (nRecordNum <=0 )
		{
			char szDispBuf[100];
			PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szDispBuf, "|CNO TRANS");
			PubMsgDlg(tr("PRINT LAST"), szDispBuf, 0, 5);
			break;
		}
		FetchLastRecord(&stTransRecord);
		PubClearAll();
		DISP_PRINTING_NOW;
		PrintRecord(&stTransRecord, REPRINT);
		break;
	case 2:
		GetRecordNum(&nRecordNum);
		if (nRecordNum <=0 )
		{
			char szDispBuf[100];
			PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szDispBuf, "|CNO TRANS");
			PubMsgDlg("PRINT ANY", szDispBuf, 0, 5);
			break;
		}
		FindByInvoice(1);
		break;
	case 3:
		PrintSettle(REPRINT);
		break;
	default:
		break;
	}

	return APP_SUCC;
}

static int PreauthMenu()
{
 	int nInputMode = INPUT_NO;
	int nRet, nSelcItem = 1, nStartItem = 1;
	char *pszItems[] = {
		tr("1.PRE-AUTH"),
		tr("2.VOID PRE-AUTH"),
		tr("3.AUTH COMPLETE"),
		tr("4.VOID AUTH-COMP"),
	};

	nRet = PubShowMenuItems(tr("BANK DEMO"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 60);
	if (nRet == APP_QUIT || nRet == APP_TIMEOUT)
	{
		return nRet;
	}

	switch(nSelcItem)
	{
	case 1:
		TxnCommonEntry(TRANS_PREAUTH, &nInputMode);
		break;
	case 2:
		TxnCommonEntry(TRANS_VOID_PREAUTH, &nInputMode);
		break;
	case 3:
		TxnCommonEntry(TRANS_AUTHCOMP, &nInputMode);
		break;
	case 4:
		TxnCommonEntry(TRANS_VOID_AUTHSALE, &nInputMode);
		break;
	default:
		break;
	}
	TxnL3TerminateTransaction();

	return APP_SUCC;

}

/**
** brief: Main menu of transaction
** param [in]:
** param [out]:
** return:
** auther: sunh
** date: 2016-7-1
** modify:
*/
static int MenuFuncSel(void)
{
 	int nInputMode = INPUT_NO;
    int nRet = 0;
	int nSelcItem = 1, nStartItem = 1;
	char *pszItems[] = {
		tr("1.SALE"),
		tr("2.VOID"),
		tr("3.SETTLEMENT"),
		tr("4.REPRINT"),
		tr("5.REPORT"),
		tr("6.PREAUTH"),
		tr("7.DEBUG"),
		tr("8.SIGNIN"),
		tr("9.BALANCE"),
		tr("10.ADJUST"),
		tr("11.CASHBACK"),
		tr("12.REFUND"),
		tr("13.QUIT")
	};

	while(1)
	{
		if (PubGetKbAttr() == KB_PHYSICAL) {
			nRet = PubShowMenuItems(tr("BANK DEMO"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 60);
		} else {
			nRet = PubShowMenuItems_Ext(tr("BANK DEMO"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 60);
		}
		if (nRet==APP_QUIT || nRet==APP_TIMEOUT)
		{
			return nRet;
		}
		switch(nSelcItem)
		{
		case 1:
			TxnCommonEntry(TRANS_SALE, &nInputMode);
			TxnL3TerminateTransaction();
			DISP_OUT_ICC;
			break;
		case 2:
			TxnCommonEntry(TRANS_VOID, &nInputMode);
			TxnL3TerminateTransaction();
			DISP_OUT_ICC;
			break;
		case 3:
			Settle(0);
			break;		
		case 4:
			ReprintMenu();
			break;
		case 5:
			ReportMenu();
			break;
		case 6:
			PreauthMenu();
			DISP_OUT_ICC;
			break;
		case 7:
			DebugMenu();
			break;
		case 8:
			Login();
			break;
		case 9:
			TxnCommonEntry(TRANS_BALANCE, &nInputMode);
			TxnL3TerminateTransaction();
			DISP_OUT_ICC;
			break;
		case 10:
			TxnCommonEntry(TRANS_ADJUST, &nInputMode);
			TxnL3TerminateTransaction();
			DISP_OUT_ICC;
			break;
		case 11:
			TxnCommonEntry(TRANS_CASHBACK, &nInputMode);
			TxnL3TerminateTransaction();
			DISP_OUT_ICC;
			break;
		case 12:
			TxnCommonEntry(TRANS_REFUND, &nInputMode);
			TxnL3TerminateTransaction();
			DISP_OUT_ICC;
			break;
		case 13:
			return KEY_FUNC_APP_SELECT;
			break;
		default:
			break;
		}
	}
	return APP_SUCC;
}

/**
** brief: Default interface
** param [in]:
** param [out]:
** return:
** auther: sunh
** date: 2016-7-1
** modify:
*/
static int MenuDefault(void)
{
	int nRet = 0;
	int nFirstsInput = L3_CARD_OTHER_EVENT;
	int nRecordNum = 0;
 	int nInputMode;
	char cDefaultTransType;
	char szFuncEvent[19+1] = {0};
	STTRANSRECORD stTransRecord;

	GetVarDefaultTransType(&cDefaultTransType);
	if (YES == GetVarIsSwipe())
	{
		nFirstsInput |= L3_CARD_MAGSTRIPE;
	}
	if (YES == GetVarIsSupportContact())
	{
		nFirstsInput |= L3_CARD_CONTACT;
	}

	while(1)
	{
		nRecordNum = 0;
		CheckAutoReboot();


		EnableDispDefault();
		if (GetVarIsPinpadReadCard() == YES)
		{
			nFirstsInput = L3_CARD_OTHER_EVENT;
		}
		nRet = NAPI_L3DetectCard(nFirstsInput, 10, &nInputMode);
		if(nRet == L3_ERR_SWIPE_CHIP)
		{
			PubMsgDlg(NULL, "Chip Card, PLEASE Insert Card", 0, 1);
			continue;
		}
		if(nRet == APP_SUCC && nInputMode == L3_CARD_OTHER_EVENT)
		{
			GetCardEventData(szFuncEvent);
			nRet = szFuncEvent[0];
		}
		if((nRet >= KEY_1 && nRet <= KEY_8)
			|| (nRet >= KEY_F1 && nRet <= KEY_F2)
			|| nRet == KEY_ENTER
		    || nRet == APP_SUCC
		    || nRet == L3_ERR_CANCEL)
		{
			DisableDispDefault();
			// Todo Light up the screen
			//NAPI_ScrBackLight
		}
		switch(nRet)
		{
		case KEY_1:
			nInputMode = INPUT_NO;
			TxnCommonEntry(TRANS_VOID, &nInputMode);
			TxnL3TerminateTransaction();
			DISP_OUT_ICC;
			break;
		case KEY_2:
			Settle(0);
			DISP_OUT_ICC;
			break;
		case KEY_3:
			nRecordNum = 0;
			GetRecordNum(&nRecordNum);
			if (nRecordNum <=0 )
			{
				char szDispBuf[100];
				PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szDispBuf, "|CNO TRANSLOG");
				PubMsgDlg("PRINT LAST", szDispBuf, 0, 5);
				break;
			}
			FetchLastRecord(&stTransRecord);
			PubClearAll();
			DISP_PRINTING_NOW;
			PrintRecord(&stTransRecord, REPRINT);
			break;
		case KEY_4:
			nRecordNum = 0;
			GetRecordNum(&nRecordNum);
			if (nRecordNum <=0 )
			{
				char szDispBuf[100];
				PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szDispBuf, "|CNO TRANSLOG");
				PubMsgDlg("PRINT ANYONE", szDispBuf, 0, 5);
				break;
			}
			FindByInvoice(1);
			break;
		case KEY_5:
			//OfflineSale();
			DISP_OUT_ICC;
			break;
		case KEY_6:
			nInputMode = INPUT_NO;
			TxnCommonEntry(TRANS_PREAUTH, &nInputMode);
			TxnL3TerminateTransaction();
			DISP_OUT_ICC;
			break;
		case KEY_7:
			nInputMode = INPUT_NO;
			TxnCommonEntry(TRANS_SALE, &nInputMode);
			TxnL3TerminateTransaction();
			DISP_OUT_ICC;
			break;
		case KEY_8:
			Login();
			break;
		case KEY_F1:
		case KEY_F2:
		case K_ZMK:
		case K_DOT:
			SystemManage();
			return APP_SUCC;
			break;
		case L3_ERR_CANCEL:
			return APP_SUCC;//Come back to main screen
			break;
		case KEY_ENTER:
			nInputMode = INPUT_NO;
			TxnCommonEntry(TRANS_SALE, &nInputMode);
			TxnL3TerminateTransaction();
			DISP_OUT_ICC;
			break;
		case KEY_MENU:
			break;
		case L3_ERR_TIMEOUT:
			DispScreenSaver();
			break;
		default:
			break;
		}
		if (nRet == APP_SUCC)
		{
			if ('1' == cDefaultTransType)/**<'1'-SALE, 0'-PREAUTH*/
			{
				TxnCommonEntry(TRANS_SALE, &nInputMode);
			}
			else
			{
				TxnCommonEntry(TRANS_PREAUTH, &nInputMode);
			}
			TxnL3TerminateTransaction();
			DISP_OUT_ICC;
		}
	}
}

/**
** brief: Detect pinpad
** param [in]:
** param [out]:
** return:
** auther: sunh
** date: 2016-7-1
** modify:
*/
static int ChkPdAndRF(void)
{
	ChkRF();
	/**
	* Check pinpad: initialize the secure module
	*/
	return ChkPinpad();
}


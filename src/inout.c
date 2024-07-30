/**
* @file inout.c
* @brief Input/Output module
* @version  1.0
* @author Zhang J
* @date 2007-01-22
*/
#include "apiinc.h"
#include "libapiinc.h"
#include "appinc.h"

/**
* @brief Get Pin
* @param in pszPan Card number for encryption
* @param in pszAmount Amount for display
* @param in pszPin fixed 8 bytes of cipher text
* @return 
*/
int GetPin(const char *pszPan, const char *pszAmount, char *psPin)
{
	int nRet, nMainKeyNo = 0, nPinLen=0, nMode = PINTYPE_WITHOUTPAN;
	char szDispAmt[64+1] = {0};

	GetVarMainKeyNo(&nMainKeyNo);
	PubSetCurrentMainKeyIndex(nMainKeyNo);

	if (NULL != pszAmount)
	{
		/*<Amount for display*/
		ProAmtToDispOrPnt(pszAmount, szDispAmt);
	}

	if(PINTYPE_WITHPAN == GetVarPinEncyptMode())
	{
		nMode = PINTYPE_WITHPAN;
	}

	if (YES == GetVarIsPinpad())
	{
		PubClear2To4();
		if (NULL != pszAmount)
		{
			/*<Amount for display*/
			PubAllTrim(szDispAmt);
			PubDisplayStrInline(DISPLAY_MODE_NORMAL, 1, "%s", szDispAmt);
		}
		PubDisplayStrInline(0, 3, tr("Please Enter PIN On Pinpad"));
		PubUpdateWindow();
	}

	nRet = PubGetPin(psPin, &nPinLen, nMode, pszPan, pszAmount, PIN_MAX_LEN, PIN_MIN_LEN);
	if (nRet != APP_SUCC)
	{
		if (nRet == APP_QUIT)
		{
			return APP_QUIT;
		}
		else if (nRet == L3_ERR_REMOVE_INTERRUPT)
		{
			char szContent[64+1] = {0};
			PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent, "|C%s", tr("Card Was Removed"));
			PubMsgDlg(NULL, szContent, 0, 1);
			return APP_QUIT;
		}
		//Get PIN error message
		PubDispErr(tr("Get PIN"));
		return APP_FAIL;
	}
	return APP_SUCC;
}



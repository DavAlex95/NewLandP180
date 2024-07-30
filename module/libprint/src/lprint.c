#include <stdarg.h>
#include "ltool.h"
#include "lprint.h"
#include "lui.h"
#include "libapiinc.h"

#define PRINTVERSION "ALPRNT0117052301"

#define PRINT_FEED_BACK  (-65) /**Feed back before print*/ 

static char gszPrnFontTTFName[128] = {0};
static int gnPrtBorder = 0;

/**
* @brief Get type of printer
* @return 
* @li Printer type refer to '_PRINTTYPE_DEF'
*/
int PubGetPrinter(void)
{
	char szBuf[10] = {0};
	
	if (APP_SUCC != PubGetHardwareSuppot(HARDWARE_SUPPORT_PRINTER, szBuf))
	{
		PubDebug("Fail to get printer type");
		return APP_FAIL;
	}
	
	if (0 == strcmp(szBuf, "STYLUS"))
		return _PRINTTYPE_IP;
	else if (0 == strcmp(szBuf, "THERMAL"))
		return _PRINTTYPE_TP;
	else
		return APP_FAIL;
}

/**
* @brief Get print version
* @param pszOutVer The version of print library
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int PubGetPrintLibVer(char *pszOutVer)
{
	if (NULL != pszOutVer)
	{
		strcpy(pszOutVer, PRINTVERSION);
		return APP_SUCC;
	}
	return APP_FAIL;
	
}

static int ProLowPowerCheck(uchar ucPercent )
{
	int nVolPercent;
	int nStatus = -1;

	nStatus = PubGetPrintStatus();
	if(NAPI_PRN_STATUS_VOLERR == nStatus)
	{
		return APP_SUCC;
	}
	 if (NAPI_SysGetVolPercent(&nVolPercent) == NAPI_OK)
	{
        if (nVolPercent < ucPercent && nVolPercent)
		{
			return APP_SUCC;
		}
	}
	return APP_FAIL;
}

/**
* @brief Low-voltage detect (Printer cannot work when voltage is low)
* @return 
* @li APP_SUCC Low-voltage
* @li APP_FAIL 
*/
int PubLowPowerCheck(void)
{/*<Check before a traction. threshold value 10%*/
	return ProLowPowerCheck(10);
}

static int DispPrintError(PRN_STATUS emStatus)
{
	switch (emStatus)
	{
	case NAPI_PRN_STATUS_OK:
		return APP_SUCC;
		break;
	case NAPI_PRN_STATUS_BUSY:
		PubMsgDlg(NULL,"Printing",3, 3);
		break;
	case NAPI_PRN_STATUS_NOPAPER:
		PubMsgDlg(NULL,"Out of paper..",3,3);
		break;
	case NAPI_PRN_STATUS_OVERHEAT:
		PubMsgDlg(NULL,"Printer overheating",3,3);
		break;
	case NAPI_PRN_STATUS_VOLERR:
		PubMsgDlg(NULL,"Voltage abnormal",3,3);
		break;
	default:
		PubMsgDlg(NULL,"Printer Err",3, 3);
		break;
	}

	return APP_SUCC;
}

/**
* @brief Check current status of printer
* @return 
* @li The value of printer status
* @author Liug
* @date 2012-9-19
*/
int PubGetPrintStatus(void)
{
	PRN_STATUS emPrnStatus;

    if (PubIsSupportPrint() == NO) {
        return NAPI_PRN_STATUS_BAD;
    }
	NAPI_PrnGetStatus(&emPrnStatus);

	return emPrnStatus;
}


/**
* @brief Set print font
* @param [in] nFontSize       font size
* @param [in] emMode          Print Mode, refer to 'EM_PRN_MODE' in lprint.h
* @param [in] unSigOrDou      Single or double (for stylus printer)
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author Liug
* @date 2018-05-15
*/

int PubSetPrtFont(int nFontSize, EM_PRN_MODE  emMode,  uint  unBold  )	
{
	PRN_TTF_PARAM stParam;
	int nWidth, nHeight;
	if(nFontSize <= 0)
		return APP_FAIL;

	nWidth = nFontSize;
	nHeight = nFontSize;

	if (emMode == PRN_MODE_ALL_DOUBLE)
	{
		nWidth = nFontSize*2;
		nHeight =  nFontSize*2;
	}
	else if(emMode == PRN_MODE_WIDTH_DOUBLE)
	{
		nWidth = nFontSize*2;
		nHeight =  nFontSize;
	}
	else if(emMode == PRN_MODE_HEIGHT_DOUBLE)
	{
		nWidth = nFontSize;
		nHeight =  nFontSize*2;
	}
	else if(emMode == PRN_MODE_NORMAL)
	{
		nWidth = nFontSize;
		nHeight =  nFontSize;
	}
	else
	{
		return APP_FAIL;
	}
	memset(&stParam, 0, sizeof(stParam));
	stParam.encoding = PRN_TTF_ENCODING_ASCII;

	if (1 == unBold)
	{
        stParam.NewMode = (PRN_TTF_MODE)(1<<4|1);
	}
	else
	{
        stParam.NewMode = (PRN_TTF_MODE)1;
	}
	stParam.FontWidth = nWidth;
	stParam.FontHeight = nHeight;

	return NAPI_PrnSetTTFParam(&stParam) == NAPI_OK ? APP_SUCC : APP_FAIL;

}

/**
* @brief Formats output string to print buffer
* @param [in] 	nMode	   Align Mode, see EM_PRINTF_ALIGN
				pszContent Pointer of string that needs to be printed
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author jiangxc
* @date 2018-5-15
*/

int PubPriPrintStr(int nMode, const char *pszContent, ...)
{
	char    szStr[500];
	va_list VarList;
	int x;
	int unSrcWidth ,unFontWidth,unFontHeight;


	if(!pszContent)
		return APP_FAIL;

	memset(szStr, 0, sizeof(szStr));
	va_start(VarList, pszContent);
	vsprintf(szStr, pszContent, VarList);
	va_end(VarList);

	unSrcWidth = PRINT_PAPER_WIDTH - gnPrtBorder;

	NAPI_PrnGetTTFStrSize(szStr, strlen(szStr), &unFontWidth, &unFontHeight);

	if(nMode & PRINT_ALIGN_CENTER)
	{
		if(unFontWidth < unSrcWidth)
			x = (unSrcWidth - unFontWidth)/2;
		else
			x = 0;
	}
	else if(nMode & PRINT_ALIGN_TAIL)
	{
		if(unFontWidth < unSrcWidth)
			x = (unSrcWidth - unFontWidth)/8*8;//As NAPI print api will ajust the x coodinate to Multiple of 8
		else
			x = 0;
	}
	else
	{
		x = 0;
	}
	NAPI_PrnPrintData(PRN_DATA_TYPE_STR_TTF, NULL, szStr, strlen(szStr), 0, 0, x);
	PubPrnStr("\r");

	return APP_SUCC;
}

int PubPrnStr(const char *pszStr)
{
	NAPI_ERR_CODE emRet;

	if (pszStr == NULL)
		return APP_FAIL;

	emRet = NAPI_PrnPrintData(PRN_DATA_TYPE_STR_TTF, NULL, pszStr, strlen(pszStr), 0, 0, 0);

	return emRet != NAPI_OK ? APP_FAIL : APP_SUCC;
}


/**
* @brief Formats output string to print buffer
* @param [in] pszContent Pointer of string that needs to be printed
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author Liug
* @date 2012-6-5
*/
#if 0
int PubPrePrinter(const char *pszContent, ...)
{
	int nRet = -1;
	va_list vlArgs;
//	char szBuffer[2048];

//	memset(szBuffer, 0, sizeof(szBuffer));
	va_start(vlArgs, pszContent);
//	vsprintf(szBuffer, pszContent, vlArgs);
	

	nRet =  PubPriPrintStr(0, pszContent, vlArgs);
	va_end(vlArgs);
	return nRet;

}
#endif

/**
* @brief Line feed
* @param [in] nLine  Number of line 
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author Liug
* @date 2012-6-5
*/
int PubPaperPeed(int nLine)
{
	int i;

	for(i=0; i<nLine; i++)
	{
		if(PubPrnStr("\r") != APP_SUCC)
		{
			return APP_FAIL;
		}
	}
	return APP_SUCC;
}

/**
* @brief Commit datas to printer
* @param [in] (*SendPrintData)() ---Function pointer
* @param [in] pvParam            ---The parameter of SendPrintData
* @param [in] unPrintType        ---Print type 0:print single time, 1:Print by send
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author Liug
* @date 2012-9-20
*/
int PubPrintCommit(int (*SendPrintData)(void *), void *pvParam,uint unPrintType)
{
	int nStatus = -1;
	int nRet;
	PRN_STATUS nPrnStatus = NAPI_PRN_STATUS_OK;
	char szContent[100] = {0};
	char szFontName[128] = {0};

	if (PubIsSupportPrint() == NO) {
		return APP_SUCC;
	}

	if(SendPrintData == NULL)
	{
		return APP_FAIL;
	}

	NAPI_ScrPush();
	PubGetPrnTTFFontFile(szFontName);
	while(1)
	{
		nRet = NAPI_PrnOpen((int)unPrintType, szFontName, 0);
		if(nRet != NAPI_OK)
		{
			PubDebug("NAPI_PrnInit fail, nRet=%d, unPrintType=%d\n", nRet, unPrintType);
			return APP_FAIL;
		}
		nStatus = PubGetPrintStatus();
		switch (nStatus)
		{
		case NAPI_PRN_STATUS_OK:
			break;
		case NAPI_PRN_STATUS_NOPAPER:		
			memset(szContent, 0, sizeof(szContent));
			PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent, "|COut of paper");
			if (PubConfirmDlg("Printer Status",szContent,3,50) == APP_SUCC)
			{
				NAPI_ScrPop();
				PubUpdateWindow();
				continue;
			}	
			return APP_QUIT;
			break;
		default:
			DispPrintError(nStatus);
			return APP_FAIL;
			break;
		}

		if (_PRINTTYPE_IP == PubGetPrinter())
		{
		}

		PubDebugSelectly(1, "PubPrintCommit->SendPrintData->begin\n");
		nRet = SendPrintData(pvParam);
		PubDebugSelectly(1, "PubPrintCommit->SendPrintData->end\n");
		
		if(nRet != APP_SUCC)
		{
			return (nRet);
		}
		
		if (_PRINTTYPE_IP == PubGetPrinter())
		{
			PubPrePrinter("\f");   //form feed
		}
		PubDebugSelectly(1, "NAPI_PrnStart.................\n");	
		nStatus = NAPI_PrnStart();	
		PubDebugSelectly(1, "NAPI_PrnStart->ret->nStatus=%d\n", nStatus);			
		switch (nStatus)
		{
		case NAPI_OK:
			if (_PRINTTYPE_TP == PubGetPrinter())
			{
				NAPI_PrnFeedPaper();
				while( NAPI_OK == NAPI_PrnGetStatus(&nPrnStatus))
				{
					if(nPrnStatus != NAPI_PRN_STATUS_BUSY)
						break;
				}
			}
			return APP_SUCC;
			break;
		case NAPI_PRN_STATUS_NOPAPER:
			memset(szContent, 0, sizeof(szContent));
			PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent, "|COut of paper\n|CPlease Load paper");
			nRet = PubConfirmDlg("Printer Status", szContent, 3, 0);
			if(nRet == APP_SUCC)
			{
				continue;
			}		
			return APP_QUIT;
			break;
		default:
			DispPrintError(nStatus);
			return APP_FAIL;
			break;
		}
	}
}


/**
* @brief Print bitmap
* @param [in] unXSize The width of picture
* @param [in] unYSize The height of picture
* @param [in] unXPos The position of top left corner of picture
* @param [in] psPrintBuf The buffer of picture
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author Liug
* @date 2012-9-19
*/
int PubPrintImage(uint unXSize, uint unYSize, uint unXPos,const char *psPrintBuf)
{
	if (NAPI_PrnPrintData(PRN_DATA_TYPE_IMAGE_BUFFER, NULL, psPrintBuf, 0, unXSize, unYSize, unXPos) != NAPI_OK)
	{
		return APP_FAIL;
	}
	return APP_SUCC;
}


/**
* @brief Print picture
* @param [in] unXPos The position of top left corner of picture
* @param [in] pszPicPath: Path of picture
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author Liug
* @date 2012-9-19
*/
int PubPrintPicture(uint unXPos,const char *pszPicPath)
{
	int nRet;

	nRet = NAPI_PrnPrintData(PRN_DATA_TYPE_IMAGE_FILE, pszPicPath, NULL, 0, 0, 0, unXPos);
	if(nRet != NAPI_OK)
	{
		return APP_FAIL;
	}
	return APP_SUCC;
}



/**
* @brief Print a tail line
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author jiangxc
* @date 2018-5-15
*/
int PubPrintTail()
{
	char pszTails[256] ={0};
	int unFontWidth,unFontHeight, nNum;

	NAPI_PrnGetTTFStrSize("-", 1, &unFontWidth, &unFontHeight);
	nNum = PRINT_PAPER_WIDTH/unFontWidth;


	memset(pszTails, '-', nNum);

	PubPriPrintStr(0, pszTails);
	return APP_SUCC;
}

/**
* @brief Print double str on the both ends of one line
* @param [in] pszContent1
			  pszContent2
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author jiangxc
* @date 2018-5-15
*/
int PubPrtDoubleStrs(const char *pszContent1, const char *pszContent2)
{
	int x, nWidth1, nWidth2, height;

	NAPI_PrnGetTTFStrSize(pszContent1, strlen(pszContent1), &nWidth1, &height);
	NAPI_PrnGetTTFStrSize(pszContent2, strlen(pszContent2), &nWidth2, &height);
	//string is too long, normal print
	if(nWidth1 + nWidth2 + gnPrtBorder > PRINT_PAPER_WIDTH)
	{
		NAPI_PrnPrintData(PRN_DATA_TYPE_STR_TTF, NULL, pszContent1, strlen(pszContent1), 0, 0, 0);
		PubPrnStr("\r");
		NAPI_PrnPrintData(PRN_DATA_TYPE_STR_TTF, NULL, pszContent2, strlen(pszContent2), 0, 0, 0);
		PubPrnStr("\r");
		return APP_SUCC;

	}
	NAPI_PrnPrintData(PRN_DATA_TYPE_STR_TTF, NULL, pszContent1, strlen(pszContent1), 0, 0, 0);
	x = (PRINT_PAPER_WIDTH - gnPrtBorder - nWidth2)/8*8;////As NAPI print api will padding the x coodinate to Multiple of 8
	NAPI_PrnPrintData(PRN_DATA_TYPE_STR_TTF, NULL, pszContent2, strlen(pszContent2), 0, 0, x);
	PubPrnStr("\r");

	return APP_SUCC;
}

/**
* @brief Whether the device supports printing
* @return 
* @li YES
* @li NO
* @author lingdz
*/
YESORNO PubIsSupportPrint()
{
	char szBuf[10] = {0};

    if (APP_SUCC != PubGetHardwareSuppot(HARDWARE_SUPPORT_PRINTER, szBuf))
    {
        PubDebug("Fail to get printer type");
        return NO;
    }

	return YES;
}

/**
* @brief Set print font file
* @return 
* @li FilePathName TTF font file name with absolute path
* @author lingdz
*/
int PubSetPrnTTFFontFile(const char *pszFilePathName)
{
	if (strlen(pszFilePathName) > sizeof(gszPrnFontTTFName) - 1) {
		PubDebug("set print ttf name fail");
		return APP_FAIL;
	}

	strcpy(gszPrnFontTTFName, pszFilePathName);
	return APP_SUCC;
}

/**
* @brief Get print font file
* @return 
* @li FilePathName TTF font file name with absolute path
* @author lingdz
*/
int PubGetPrnTTFFontFile(char *pszFilePathName)
{
	if (strlen(gszPrnFontTTFName) == 0) {
		strcpy(gszPrnFontTTFName, "roboto.ttf");
		PubDebug("default ttf name = %s", gszPrnFontTTFName);
		return APP_SUCC;
	}

	strcpy(pszFilePathName, gszPrnFontTTFName);
	return APP_SUCC;
}

/* end of lprint.c */


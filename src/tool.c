/***************************************************************************
** Copyright (c) 2019 Newland Payment Technology Co., Ltd All right reserved   
** File name:  tool.c
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
//#include "process.h"

/**
* @brief Asicc Amount to unsigned long long
* @param in pszAmt 
* @return 
* @li the result amount
*/
unsigned long long AtoLL(const char *pszAmt)
{
	char szTmpAmt[12+1] = {0};
	int i;
	unsigned long long ullAmount = 0;
	
	memcpy(szTmpAmt, pszAmt, 12);
	PubLeftTrim(szTmpAmt);
	
	for (i = 0; i< strlen(szTmpAmt); i++)
	{
		if (PubIsDigitChar(szTmpAmt[i]) == APP_SUCC)
		{
			ullAmount = ullAmount * 10 + (szTmpAmt[i] - '0');
		}
		else
		{
			break;
		}
	}
	return ullAmount;
}

//Delete all the '0' in left, If all the string letter is '0', Leave one '0'
void LeftTrimZero(char *pszSrc)
{
	PubLeftTrimChar((uchar *)pszSrc, '0');
	if(pszSrc[0] == 0)
	{
		pszSrc[0] = '0';
		pszSrc[1] = 0;
	}
}

int AscBigCmpAsc(const char *pszNum1,const char *pszNum2)
{
	int nLen1=0,nLen2=0;
	char szNum1[100+1], szNum2[100+1];
	strcpy(szNum1, pszNum1);
	strcpy(szNum2, pszNum2);
	LeftTrimZero(szNum1);
	LeftTrimZero(szNum2);
	
	nLen1=strlen(szNum1);
	nLen2=strlen(szNum2);
	if (nLen1 == nLen2)
		return strcmp(szNum1,szNum2);
	else if (nLen1 > nLen2)
		return 1;
	else
		return -1;
}

/**
* @brief Amout format change to display,like 000000000010 to 0,10
* @param in pszInAmt 
* @param out pszOutAmt 
* @return 
* @li APP_SUCC Success
* @li APP_FAIL Fail
*/
int ProAmtToDispOrPnt(const char *pszInAmt, char *pszOutAmt)
{
	int i;
	char szAmt[12+1] = {0}, szTempAmt[12+1] = {0};
	
	if (NULL == pszInAmt ||NULL == pszOutAmt)
	{
		return APP_FAIL;
	}
	if (memcmp(pszInAmt, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 12) == 0)
	{
		return APP_FAIL;
	}
	memcpy(szAmt, pszInAmt, 12);
	PubAllTrim(szAmt);
	if (0 == strlen(szAmt))
	{
		return APP_FAIL;
	}
	
	PubAddSymbolToStr(szAmt,12,'0',ADDCH_MODE_BEFORE_STRING);
	strcpy(szTempAmt, szAmt);
	for (i = 0; i < strlen(szAmt); i++)
	{
		if (PubIsDigitChar(szAmt[i]) == APP_FAIL)
		{
			return APP_FAIL;
		}
	}
	memset(szAmt, 0, sizeof(szAmt));
	memcpy(szAmt, szTempAmt, 10);
	PubAllTrim(szAmt);
	if ((strlen(szAmt) > 9) && (szAmt[0] != '0'))  
	{
		sprintf(pszOutAmt, "%s.%02d", szAmt, atoi(szTempAmt + 10));
	}
	else
	{
		sprintf(pszOutAmt, "%ld.%02d", atol(szAmt), atoi(szTempAmt + 10));
		PubAddSymbolToStr(pszOutAmt, 13, ' ', 0);
	}
	return APP_SUCC;
}

/*

int ProSelectYesOrNo(char *pszMainMenu, char *pszTitle, char *pszStr, char *pcSelect)
{
	int nSelect;

	if (*pcSelect == 0 || *pcSelect =='0')
	{
		nSelect = 0;
	}
	else if ( *pcSelect ==1 || *pcSelect =='1')
	{
		nSelect = 1;
	}
	else
	{
		nSelect = 0XFF;
	}
	ASSERT_RETURNCODE(PubSelectListItem(pszStr, pszMainMenu, pszTitle, nSelect, &nSelect));
	*pcSelect = nSelect + '0';
	return APP_SUCC;
}


int ProSelectYesOrNoExt(char *pszMainMenu, char *pszTitle, char *pszStr, char *pcSelect)
{
	int nRet, nSelect;
	char szFunKey[2+1]={0};
	szFunKey[0] = KEY_F1; 
	szFunKey[1] = KEY_F2; 
	
	if (*pcSelect == 0 || *pcSelect =='0')
	{
		nSelect = 0;
	}
	else if ( *pcSelect ==1 || *pcSelect =='1')
	{
		nSelect = 1;
	}
	else
	{
		nSelect = 0XFF;
	}
	nRet = PubSelectListItemExtend(pszStr, pszMainMenu, pszTitle, nSelect, &nSelect, 60, szFunKey);
	if (nRet==APP_QUIT || nRet == APP_TIMEOUT)
	{
		return APP_QUIT;
	}
	if (nSelect - 0xFF == KEY_UP)
    {
		return KEY_UP;
    }
	else if(nSelect - 0xFF == KEY_DOWN)
	{
		return KEY_DOWN;
	}
	*pcSelect = nSelect + '0';
	return APP_SUCC;
}
*/
int ProChkDigitStr(const char *psStr, const int nLen)
{
	int i;
	
	for (i = 0; i < nLen; i++)
	{
		if (PubIsDigitChar(psStr[i]) != APP_SUCC)
		{
			return APP_FAIL;
		}
	}
	return APP_SUCC;
}

int ProConfirm(void)
{
	return PubConfirmDlg(NULL, NULL, 0, 0);
}

unsigned long long Bcd2Unit64( uchar *bcd,  int bcd_len )
{
	unsigned long long  result = 0;
	uchar *ptr = bcd;

	if( bcd == NULL || bcd_len <= 0 )
		return 0;

	while( ptr < bcd + bcd_len ){
		result = result * 10  + (*ptr >> 4);
		result = result  * 10 + (*ptr++ & 0x0f) ;
	}

	return result;
}

const char * ImgManage(EM_LOGO_TYPE emLogoType)
{
	char cIsColor = NO;

	if (1 == PubScrGetColorDepth())  //For black-white machine 
	{
		cIsColor = NO; 
	}
	else if (16 == PubScrGetColorDepth())  
	{
		cIsColor = YES;
	}
	
	switch(emLogoType)
	{
	case LOGO_MAIN_MENU:
		if (YES == cIsColor)
		{
			return "CUP320x64.jpg";
		}
		break;
	case LOGO_RF:
		if (YES == cIsColor)
		{
			PubDisplayLogo("RF150x93.jpg", 152, 80);
		}
		break;	
	default:
		break;
	}
	
	return NULL;
}

void AmtAddAmt(uchar *pszAsc1, uchar *pszAsc2, uchar *pszSum)
{
	PubAscAddAsc(pszAsc1, pszAsc2, pszSum);			
	PubAddSymbolToStr((char *)pszSum, 12, '0', 0);
}

void AmtSubAmt(uchar *pszAsc1, uchar *pszAsc2, uchar *pszResult)
{
	PubAscSubAsc(pszAsc1, pszAsc2, pszResult);
	PubAddSymbolToStr((char *)pszResult, 12, '0', 0);
}


void ShowLightIdle()
{
	return; /* no support */
#if 0
    if(YES != GetVarIsSupportRF() || YES != GetVarIsSupportContact())
	{
		return ;
	}
	if (APP_SUCC == PubGetHardwareSuppot(HARDWARE_SUPPORT_LED_LIGHT, NULL))
	{
		NAPI_LedStatus((EM_NAPI_LED)(NAPI_LED_RFID_BLUE_ON | NAPI_LED_RFID_YELLOW_OFF | NAPI_LED_RFID_GREEN_OFF | NAPI_LED_RFID_RED_OFF));
		PubSysMsDelay(100);
		NAPI_LedStatus((EM_NAPI_LED)(NAPI_LED_RFID_BLUE_OFF | NAPI_LED_RFID_YELLOW_OFF | NAPI_LED_RFID_GREEN_OFF | NAPI_LED_RFID_RED_OFF));
	}
	else if (16 == PubScrGetColorDepth())
	{
		//Cup_ShowScrLed(LED_UNCHANGED, LED_UNCHANGED, LED_UNCHANGED, LED_FLICK);
	}
#endif
}

void ShowLightReady()
{
	return; /* no support */
#if 0
    if(YES != GetVarIsSupportRF() || GetVarIsSupportContact() != YES)
   	{
   		return ;
   	}
	if (APP_SUCC == PubGetHardwareSuppot(HARDWARE_SUPPORT_LED_LIGHT, NULL))
	{
		NAPI_LedStatus((EM_NAPI_LED)(NAPI_LED_RFID_BLUE_ON | NAPI_LED_RFID_YELLOW_OFF | NAPI_LED_RFID_GREEN_OFF | NAPI_LED_RFID_RED_OFF));
	}
#endif
}


void ShowLightDeal()
{
	return; /* no support */
#if 0
    if(YES != GetVarIsSupportRF() || YES != GetVarIsSupportContact())
   	{
   		return ;
   	}
	if (APP_SUCC == PubGetHardwareSuppot(HARDWARE_SUPPORT_LED_LIGHT, NULL))
	{
		NAPI_LedStatus((EM_NAPI_LED)(NAPI_LED_RFID_BLUE_ON | NAPI_LED_RFID_YELLOW_ON | NAPI_LED_RFID_GREEN_OFF | NAPI_LED_RFID_RED_OFF));
	}
#endif 
}

void ShowLightSucc()
{
	return; /* no support */
#if 0
    if(YES != GetVarIsSupportRF() || YES != GetVarIsSupportContact())
   	{
   		return ;
   	}
	if (APP_SUCC == PubGetHardwareSuppot(HARDWARE_SUPPORT_LED_LIGHT, NULL))
	{
		NAPI_LedStatus((EM_NAPI_LED)(NAPI_LED_RFID_BLUE_ON | NAPI_LED_RFID_YELLOW_ON | NAPI_LED_RFID_GREEN_ON | NAPI_LED_RFID_RED_OFF));
	}
#endif
}

void ShowLightFail()
{
	return; /* no support */
#if 0
    if(YES != GetVarIsSupportRF() || YES != GetVarIsSupportContact())
   	{
   		return ;
   	}
	if (APP_SUCC == PubGetHardwareSuppot(HARDWARE_SUPPORT_LED_LIGHT, NULL))
	{
		NAPI_LedStatus((EM_NAPI_LED)(NAPI_LED_RFID_BLUE_OFF | NAPI_LED_RFID_YELLOW_OFF | NAPI_LED_RFID_GREEN_OFF | NAPI_LED_RFID_RED_ON));
	}
#endif
}



void ShowLightOff()
{
	return; /* no support */
#if 0
    if(YES != GetVarIsSupportRF() || YES != GetVarIsSupportContact())
   	{
   		return ;
   	}
	if (APP_SUCC == PubGetHardwareSuppot(HARDWARE_SUPPORT_LED_LIGHT, NULL))
	{
		NAPI_LedStatus((EM_NAPI_LED)(NAPI_LED_RFID_BLUE_OFF | NAPI_LED_RFID_YELLOW_OFF | NAPI_LED_RFID_GREEN_OFF | NAPI_LED_RFID_RED_OFF));
	}
#endif
}

void ShowLightOnline()
{
	return; /* no support */
#if 0
    if(YES != GetVarIsSupportRF() || YES != GetVarIsSupportContact())
   	{
   		return ;
   	}
	if (APP_SUCC == PubGetHardwareSuppot(HARDWARE_SUPPORT_LED_LIGHT, NULL))
	{
		NAPI_LedStatus((EM_NAPI_LED)(NAPI_LED_RFID_BLUE_ON|NAPI_LED_RFID_YELLOW_ON|NAPI_LED_RFID_GREEN_FLICK|NAPI_LED_RFID_RED_OFF));
	}
	else if (16 == PubScrGetColorDepth())
	{
		//Cup_ShowScrLed(LED_UNCHANGED, LED_FLICK, LED_UNCHANGED, LED_UNCHANGED);
	}
#endif
}


/**ISO 4217*/
int CurrencyAmtToDisp(const char *pszCurrency, const char *pszAmt, char *pszOutstr)
{
	char *pszCurrcyName[] = 
	{
		"156RMB",	
		"344HKD",	
		"446MOP",	
		"458MYR",	
		"702SGD",	
		"764THB",	
		"360IDR",	
		"840USD",	
		"392JPY",
		"978EUR",	
		"826GBP",	
		"124CAD",	
		NULL,		
	};	

	char **p = NULL;	
	char szDispAmt[DISPAMTLEN] = {0};

	ProAmtToDispOrPnt(pszAmt, szDispAmt);
	PubAllTrim(szDispAmt);
	
	for (p = pszCurrcyName; *p != NULL; p++)
	{
		if (0 == memcmp(pszCurrency, *p, 3))
		{
			sprintf(pszOutstr, "%3.3s %s", *p + 3, szDispAmt);
			return APP_SUCC;
		}
	}

	sprintf(pszOutstr, "OTHER %s", szDispAmt);
	return APP_FAIL;
}


int SetStatusBar(EMSTATUSBARTYPE emType)
{
	EM_STATUSBAR emCommType = STATUSBAR_DISP_TIME|STATUSBAR_DISP_DATE|STATUSBAR_DISP_BATTERY;
	STAPPCOMMPARAM stAppCommParam;

	if (STATUSBAR_STANDBY_OPEN  == emType || STATUSBAR_STANDBY_CLOSE == emType) 
	{	
		if (16 == PubScrGetColorDepth())
		{
			return APP_SUCC;
		}
	}
	else 
	{
		if (1 == PubScrGetColorDepth())  
		{
			return APP_SUCC;
		}
	}

	NAPI_ScrStatusbar(STATUSBAR_DISP_CLOSE);
	
	if (STATUSBAR_STANDBY_OPEN == emType
		||STATUSBAR_OTHER_OPEN == emType)
	{
		if (16 == PubScrGetColorDepth()) 
		{
			emCommType |= STATUSBAR_POSITION_TOP;
		}
		else
		{
			emCommType |= STATUSBAR_POSITION_BOTTOM;
		}

		memset(&stAppCommParam, 0, sizeof(STAPPCOMMPARAM));
		GetAppCommParam(&stAppCommParam);
		
		switch(stAppCommParam.cCommType)
		{
		case COMM_RS232:
			break;
		case COMM_DIAL:
			emCommType |= STATUSBAR_DISP_MODEM;		
			break;
		case COMM_GPRS:
		case COMM_CDMA:
			emCommType |= STATUSBAR_DISP_WLSIGNAL;
			break;
		case COMM_ETH:
			emCommType |= STATUSBAR_DISP_ETH;
			break;
		case COMM_WIFI:
			emCommType |= STATUSBAR_DISP_WIFI;
			break;
		default:
			break;
		}

		NAPI_ScrStatusbar(emCommType);
	}	
	return APP_SUCC;
}

/**
* @brief Check if string is digit
* @param [in] psStr 	Strings pointer
* @param [in] nLen 	Strings length
* @return 
* @li APP_SUCC 
* @li APP_FAIL
* @author 
* @date
*/
int ChkDigitStr(const char *psStr, const int nLen)
{
	int i;
	
	for (i = 0; i < nLen; i++)
	{
		ASSERT_FAIL(PubIsDigitChar(psStr[i]));
	}
	return APP_SUCC;
}

//psStr1
void EdcProXor(char *psStr1, char *psStr2, int nLen)
{
	int i = 0;
	
	while (1)
	{
		if (nLen == i)
			break;
		*(psStr1+i) ^= *(psStr2+i);
		i++;
	}
}


/**
* @brief  bcd amount calculate, the result is display format like X.XX
* @param [in] cType	0Plus, 1Minus
* @param [in] psBcdAmt1
* @param [in] psBcdAmt2	
* @return 
* @li APP_SUCC
* @li APP_FAIL
* @author 
* @date
*/
int MathBcdAmtAndToDisp(const uchar *psBcdAmt1, const uchar *psBcdAmt2, EM_MATH_TYPE emType, char *pszDispAmt)
{
	uchar szAmt[12+1] = {0}, szAmt1[12+1] = {0}, szAmt2[12+1] = {0};
	
	if (NULL == pszDispAmt)
		return APP_FAIL;

	PubHexToAsc(psBcdAmt1, 12, 0, szAmt1);
	PubHexToAsc(psBcdAmt2, 12, 0, szAmt2);
	memset(szAmt, 0, sizeof(szAmt));
	switch (emType)
	{
	case EM_SUB:
		PubAscSubAsc(szAmt1, szAmt2, szAmt);
		break;
	case EM_PLUS:
	default:
		PubAscAddAsc(szAmt1, szAmt2, szAmt);
		break;
	}
	PubAddSymbolToStr((char *)szAmt, 12, '0', 0);
	ProAmtToDispOrPnt((char*)szAmt, pszDispAmt);
	PubAllTrim(pszDispAmt);
	return APP_SUCC;
}

/**
* @brief Bcd amount to Display format
* @param void
* @return 
* @li APP_SUCC
* @li APP_FAIL
* @author 
* @date
*/
int BcdAmtToDisp(const uchar *psBcdAmt, char *pszDispAmt)
{
	char szAmt[12+1] = {0};
	
	if (NULL == pszDispAmt)
		return APP_FAIL;
	PubHexToAsc(psBcdAmt, 12, 0,  (uchar*)szAmt);
	ProAmtToDispOrPnt((char*)szAmt, pszDispAmt);
	PubAllTrim(pszDispAmt);
	return APP_SUCC;
}

//Del space enter tab
void ProTrimStr(char *pszString)
{
#define ISSPACE(ch) ( ((ch) == ' ')  || ((ch) == '\t') || \
                      ((ch) == '\n') || ((ch) == '\r') )

    char *p, *q;

    if( !pszString || !*pszString )
    {
        return;
    }

    p = &pszString[strlen((char*)pszString) - 1];
    while(( p > pszString) && ISSPACE(*p) )
    {
        *p-- = 0;
    }
    if( (p == pszString) && ISSPACE(*p) )  *p = 0;

    for(p = pszString; *p && ISSPACE(*p); p++);
    if( p != pszString )
    {
        q = pszString;
        while( *p )   *q++ = *p++;
        *q = 0;
    }
#undef ISSPACE
}

// "A/B.Mr" --> "Mr A B"
void ConvertHolderName(uchar *pszOrgName, uchar *pszNormalName)
{
	char	*pszTitle, *pszMidName, *pszTemp, szBuff[50];

	sprintf((char *)pszNormalName, "%s", pszOrgName);
	if( *pszOrgName==0 )
	{
		return;
	}
	pszTemp = (char *)pszNormalName;

	pszMidName = strchr((char *)pszOrgName, '/');
	if( pszMidName==NULL )
	{
		return;
	}

	pszTitle = strrchr((char *)pszOrgName, '.');
	if( pszTitle!=NULL )
	{
		sprintf(szBuff, "%s ", pszTitle+1);
		ProTrimStr(szBuff);
		pszTemp += sprintf(pszTemp, "%s ", szBuff);

		sprintf(szBuff, "%.*s", (int)(pszMidName-(char *)pszOrgName), pszOrgName);
		ProTrimStr(szBuff);
		pszTemp += sprintf(pszTemp, "%s ", szBuff);
		sprintf(szBuff, "%.*s ", (int)(pszTitle-pszMidName-1), pszMidName+1);
		ProTrimStr(szBuff);
		pszTemp += sprintf(pszTemp, "%s ", szBuff);
		return ;
	}
	else
	{
		sprintf(szBuff, "%s", pszMidName+1);
		ProTrimStr(szBuff);
		pszTemp += sprintf(pszTemp, "%s ", szBuff);
	}
	sprintf(pszTemp, "%.*s", (int)(pszMidName-(char *)pszOrgName), pszOrgName);
}

/**
* @brief Add some same letter between String A and String B
* @param [in] pszStrA 
* @param [in] pszStrB       
* @param [in] ch      
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int ProAddSymbolBtnStrs(char *pszStrA, char *pszStrB, char *pszOut, int nLen, char ch)
{
	int nSpaceLen = 0;
	int nLenA = 0, nLenB = 0;
	
	if (NULL == pszOut)
		return APP_FAIL;

	if (NULL != pszStrA)
	{
		nLenA = strlen(pszStrA);
	}
	if (NULL != pszStrB)
	{
		nLenB = strlen(pszStrB);
	}
	
	if (NULL != pszStrA)
		strcpy(pszOut, pszStrA);

	if (NULL != pszStrB)
	{
		if (0 == ch)
		{
			memcpy(pszOut+nLenA, pszStrB, nLenB);
			return APP_SUCC;
		}
		nSpaceLen = nLen-nLenB;
		if (nSpaceLen < 0)
		{
			nSpaceLen = 0;
		}
		if ((nSpaceLen - nLenA) > 0)
		{	
			PubAddSymbolToStr(pszOut, nSpaceLen, ch, 1);
		}
		memcpy(pszOut+nSpaceLen, pszStrB, nLenB);
	}
	else
	{
		PubAddSymbolToStr(pszOut, nLen, ch, 0);	
	}
	
	return APP_SUCC;
}

/*
 YYYYMMDD
*/
int CaculateWeekDay(char *pszData)
{
    char szYear[4+1];
    char szMonth[2+1];
    char szDay[2+1];
    int y;
    int m;
    int d;
    int iWeek;

    if (pszData == NULL)
    {
        return APP_FAIL;
    }

    //YEAR
    memset(szYear, 0, sizeof(szYear));
    memcpy(szYear, pszData, sizeof(szYear)-1);
    //MONTH
    memset(szMonth, 0, sizeof(szMonth));
    memcpy(szMonth, pszData+4, sizeof(szMonth)-1);
    //DAY
    memset(szDay, 0, sizeof(szDay));
    memcpy(szDay, pszData+4+2, sizeof(szDay)-1);

    y = atoi(szYear);
    m = atoi(szMonth);
    d = atoi(szDay);
    
    if(m==1||m==2) {
        m+=12;
        y--;
    }
    iWeek=(d+2*m+3*(m+1)/5+y+y/4-y/100+y/400)%7;
    iWeek++;
    
    return iWeek;
} 


int GetRandValue(int nRange)
{
	srand( (unsigned)time( NULL ) );  
    return rand()%nRange+1;
}

/**
* @brief
* @param  Check the string whether is NULL or empty
* @return
* @li YES
* @li NO
* @author linld
* @date 2015-7-6
*/
YESORNO CheckIsNullOrEmpty(const char *pszStr, const int nLen)
{
	char szZero[1024] = {0};

	if (pszStr == NULL)
	{
		return YES;
	}
	else if (memcmp(pszStr, szZero, nLen) == 0)
	{
		return YES;
	}

	return NO;
} 

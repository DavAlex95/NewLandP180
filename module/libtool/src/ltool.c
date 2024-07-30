#include <string.h>
#include <sys/time.h>
#include "ltool.h"
#include "GbkToUtf_8.h"
#include "libapiinc.h"

#undef MAX_FIELD_LEN
#undef MAX_MSG_LEN

#define MAX_FIELD_LEN 2048		/**<Field length*/
#define MAX_MSG_LEN 8192		/**<Max message length*/

#define GETSAVEVARFILE		"GetSaveVar.dat"	/**<File for power down protection area*/
#define PUBTOOLSVER		"ALTOOL0117052301"	/**<tool library version*/




int PubSysDelay(uint unDelayTime)
{
	usleep(unDelayTime * 100000);
	return APP_SUCC;
}


int PubSysMsDelay(uint unDelayTime)
{
#if 0
	struct timeval delay;
	delay.tv_sec = 0;
	delay.tv_usec = unDelayTime * 1000;
	int nErr;
    do {
		nErr = select(0, NULL, NULL, NULL, &delay);
    } while(nErr < 0 && errno == EINTR);
#else
	usleep(unDelayTime * 1000);
#endif

	return APP_SUCC;
}
/**
* @brief Add two unsigned int number(no more than 12 bytes), the result can not be more than 12 bytes.
* @param [in] pszDigStr1 Pointer to number1
* @param [in] pszDigStr2 Pointer to number2
* @param [out] pszResult Pointer to result
* @param [out] pnResultLen  Pointer to the length of result
* @return 
* @li APP_FAIL Fail
* @li APP_APP_SUCC Success
* @date 2012-06-19
*/
int PubAddDigitStr (const char* pszDigStr1, const char* pszDigStr2, char* pszResult, int* pnResultLen )
{
	int i, nFlag, nLen1, nLen2;
	char sBuf[12+1];

	if ((pszDigStr1 == NULL) || (pszDigStr2 == NULL) || (pszResult == NULL) || (pnResultLen == NULL))
	{
		return APP_FAIL;
	}
	/*judge whether the digit string is legal or not*/
	if ((PubIsDigitStr(pszDigStr1) != APP_SUCC) || (PubIsDigitStr(pszDigStr2) != APP_SUCC))
	{
		return APP_FAIL;
	}

	nLen1 = strlen(pszDigStr1);
	nLen2 = strlen(pszDigStr2);
	if ((12 < nLen1) || (12 < nLen2))
	{
		return APP_FAIL;
	}
	memset(sBuf, '0', 12);
	sBuf[12] = 0;
	memcpy(sBuf + 12 - nLen1, pszDigStr1, nLen1);

	nFlag = 0;
	sBuf[11] += pszDigStr2[nLen2 - 1] - '0';
	for (i=nLen2-1; i >= 0; i--)
	{
		if (sBuf[i+(11-(nLen2-1))] - '0' >= 10)
		{
			nFlag = 1;
			if (i + (11 - (nLen2 - 1)) - 1 < 0)
			{
				return APP_FAIL;
			}
			sBuf[i + (11 - (nLen2 - 1))] -= 10;
			if ( i == 0 )
			{
				sBuf[i + (11 - (nLen2 - 1)) - 1] += nFlag;
				if ( nLen1 > nLen2 )
				{
					if( sBuf[i + (11 - (nLen2 - 1)) - 1] - '0' >= 10 )
					{
						sBuf[i + (11-(nLen2-1)) -1] -= 10;
						sBuf[i + (11-(nLen2-1)) -1 - 1] += nFlag;
					}
					else
					{
						nFlag = 0;
					}
				}
			}
			else
			{
				sBuf[i + (11 - (nLen2 - 1)) - 1] += (pszDigStr2[i - 1] - '0') + nFlag;
			}
		}
		else
		{
			nFlag = 0;
			sBuf[i + (11 - (nLen2 - 1)) - 1] += (pszDigStr2[i - 1] - '0');
		}
	}
	*pnResultLen = (nLen1>nLen2)?nLen1:nLen2 + nFlag;
	memcpy(pszResult, sBuf + 12 - *pnResultLen, *pnResultLen);
	return APP_SUCC;
}

/**
* @brief A 6 bytes string plus 1(when pcStrNum == "999999", result is 000001)
* @param [in] pcStrNum  string (6bytes)
* @param [out] pcStrNum result after plus 1
* @return 
* @li APP_FAIL Fail
* @li APP_APP_SUCC Success
* @date 2012-06-19
*/
int PubIncNum (char* pcStrNum )
{
	long lnNum;

	if ( (pcStrNum == NULL) || ( strlen(pcStrNum) > 6 ) )
	{
		return APP_FAIL;
	}
	if(PubIsDigitStr(pcStrNum) != APP_SUCC)
	{
		return APP_FAIL;
	}

	lnNum = atol(pcStrNum) + 1l;
	if (lnNum >= 1000000l)
	{
		lnNum = 1l;
	}
	sprintf(pcStrNum, "%06ld", lnNum);
	return APP_SUCC;
}

static void ProDelFirstChar(char *pszAsc)
{
	char szTmp[1024+1];

	strcpy(szTmp, pszAsc+1);
	strcpy(pszAsc, szTmp);
	return;
}

/*<Delete '0' before pszNum*/
static void ProPreZero(char *pszNum)
{
	int i = 0;
    char *p = pszNum;
	char szBuf[1024];

    while(*p == '0')
    {
		p++;
    }
    while((szBuf[i++] = *p++))
	{
		;
    }

    if( szBuf[0] == 0 )
	{
        strcpy(szBuf,"0");
    }
	memset(pszNum, 0, strlen(pszNum));
	strcpy(pszNum, szBuf);
}

/*<comparte 2 strings, return :0 equal>0 bigger<0 smaller*/
static int ProBigCmp(char *pszNum1, char *pszNum2)
{
    int nLen1, nLen2;

	nLen1 = strlen(pszNum1);
    nLen2 = strlen(pszNum2);

    if (nLen1 == nLen2)
    {
		return strcmp(pszNum1, pszNum2);
    }
    else if (nLen1 > nLen2)
    {
    	return 1;
    }
    else
    {
    	return -1;
    }
}

static int ProCheckOper1(char *pszAsc)
{
	if(PubIsDigitChar(pszAsc[0]) != APP_SUCC)
	{
		if(pszAsc[0] != '-' && pszAsc[0] != '+')
		{
			return APP_FAIL;
		}
	}
	if(PubIsDigitStr(pszAsc+1) != APP_SUCC)
	{
		return APP_FAIL;
	}
	if(pszAsc[0] == '+')
	{
		ProDelFirstChar(pszAsc);
	}
	ProPreZero(pszAsc);
	return APP_SUCC;
}

static int ProCheckOper2(char *pszAsc1, char *pszAsc2)
{
	if(ProCheckOper1(pszAsc1) != APP_SUCC || ProCheckOper1(pszAsc2) != APP_SUCC)
	{
		return APP_FAIL;
	}
	return APP_SUCC;
}

/**
* @brief Ascii string1 plus Ascii string2
* @param [in] pszAsc1	Poniter to Ascii string1
* @param [in] pszAsc2	Poniter to Ascii string2
* @param [out] pszSum 	Pointer to the sum
* @return 
* @li void
*/
void PubAscAddAsc(uchar *pszAsc1, uchar *pszAsc2, uchar *pszSum)
{
	int i, nLen1, nLen2, nFlag=0, nSign=0;
	char sAdd1[1024]={0};
	char sAdd2[1024]={0};
	char sRes[1024+1]={0};

	if(pszAsc1==NULL || pszAsc2==NULL || pszSum==NULL)
	{
		return;
	}

	nLen1 = strlen((char*)pszAsc1);
	nLen2 = strlen((char*)pszAsc2);
	if( nLen1>=1024 || nLen2>=1024)
	{
		return;
	}
	if(nLen1 == 0 || nLen2 == 0)
	{
		if(nLen1 > 0)
		{
			strcpy(sAdd1, (char*)pszAsc1);
			ProPreZero(sAdd1);
			strcpy((char *)pszSum, sAdd1);
		}
		if(nLen2 > 0)
		{
			strcpy(sAdd2, (char*)pszAsc2);
			ProPreZero(sAdd2);
			strcpy((char *)pszSum, sAdd2);
		}
		return ;
	}
	strcpy(sAdd1, (char*)pszAsc1);
	strcpy(sAdd2, (char*)pszAsc2);
	if(ProCheckOper2(sAdd1, sAdd2) != APP_SUCC)
	{
		return;
	}
	if(sAdd1[0] == '-' && sAdd2[0] == '-')
	{
		nSign = -1;
		ProDelFirstChar(sAdd1);
		ProDelFirstChar(sAdd2);
	}
	else if(sAdd1[0] == '-')
	{
		PubAscSubAsc((uchar*)sAdd2, (uchar*)sAdd1+1, pszSum);
		return;
	}
	else if(sAdd2[0] == '-')
	{
		PubAscSubAsc((uchar*)sAdd1, (uchar*)sAdd2+1, pszSum);
		return;
	}

	nLen1 = strlen(sAdd1);
	nLen2 = strlen(sAdd2);
	if(ProBigCmp(sAdd1,sAdd2) < 0)
	{
		strcpy(sRes, sAdd1);
		strcpy(sAdd1, sAdd2);
		strcpy(sAdd2, sRes);
		i = nLen1;
		nLen1 = nLen2;
		nLen2 = i;
    }

	for(i=1; i<=nLen1; i++)
	{
		if(i <= nLen2)
		{
			sRes[nLen1-i] = sAdd1[nLen1-i] + sAdd2[nLen2-i] - '0' + nFlag;
		}
		else
		{/*<i>nLen2*/
			sRes[nLen1-i] = sAdd1[nLen1-i] + nFlag;
		}
		if( sRes[nLen1-i] > '9' )
		{
            nFlag = 1;
            sRes[nLen1-i] -= 10;
        }
		else
		{
			nFlag = 0;
		}
    }
	if(nFlag == 1 && nSign == -1)
	{
		sprintf((char*)pszSum, "-1%s", sRes);
	}
	else if(nSign == -1)
	{
		sprintf((char*)pszSum, "-%s", sRes);
	}
	else if(nFlag == 1)
	{
		sprintf((char*)pszSum, "1%s", sRes);
	}
	else
	{
		sprintf((char*)pszSum, "%s", sRes);
	}
	return;
}

/**
* @brief Ascii string1 substract Ascii string2
* @param [in] pszAsc1	Poniter to Ascii string 1
* @param [in] pszAsc2	Poniter to Ascii string 2
* @param [out] pszResult If the result is minus, show "-XXXX"
* @return 
* @li void
*/
void PubAscSubAsc(uchar *pszAsc1, uchar *pszAsc2, uchar *pszResult)
{
	int i, nLen1, nLen2, nSign, nFlag=0;
	char sSub1[1024]={0};
	char sSub2[1024]={0};
	char sRes[1024+1]={0};

	if(pszAsc1==NULL || pszAsc2==NULL || pszResult==NULL)
	{
		return;
	}

	nLen1 = strlen((char*)pszAsc1);
	nLen2 = strlen((char*)pszAsc2);
	if(nLen1==0 || nLen1>=1024 || nLen2==0 || nLen2>=1024)
	{
		return;
	}
	strcpy(sSub1, (char*)pszAsc1);
	strcpy(sSub2, (char*)pszAsc2);
	if(ProCheckOper2(sSub1, sSub2) != APP_SUCC)
	{
		return;
	}
	if(sSub1[0] == '-' && sSub2[0] == '-')
	{
		strcpy(sRes, sSub1);
		strcpy(sSub1, sSub2+1);
		strcpy(sSub2, sRes+1);
	}
	else if(sSub1[0] == '-')
	{
		pszResult[0] = '-';
		PubAscAddAsc((uchar*)sSub1+1, (uchar*)sSub2, pszResult+1);
		return;
	}
	else if(pszAsc2[0] == '-')
	{
		PubAscAddAsc((uchar*)sSub1, (uchar*)sSub2+1, pszResult);
		return;
	}

	nLen1 = strlen(sSub1);
	nLen2 = strlen(sSub2);
	nSign=ProBigCmp(sSub1, sSub2);
	if (nSign == 0)
	{
        strcpy((char*)pszResult, "0");
        return ;
    }
    else if(nSign < 0)
	{
		strcpy(sRes, sSub1);
		strcpy(sSub1, sSub2);
		strcpy(sSub2, sRes);
		i = nLen1;
		nLen1 = nLen2;
		nLen2 = i;
    }

	for(i=1; i<=nLen1; i++)
	{
		if(i <= nLen2)
		{
			sRes[nLen1-i] = sSub1[nLen1-i] - sSub2[nLen2-i] + '0' - nFlag;
		}
        else
        {/*<i>nLen2*/
        	sRes[nLen1-i] = sSub1[nLen1-i]-nFlag;
        }
		if( sRes[nLen1-i] < '0' )
		{
            nFlag = 1;
            sRes[nLen1-i] += 10;
        }
		else
		{
			nFlag = 0;
		}
    }
	ProPreZero((char*)sRes);
	if(nSign < 0)
	{
		sprintf((char*)pszResult, "-%s", sRes);
	}
	else
	{
		strcpy((char*)pszResult, sRes);
	}
	return;
}

/**
* @brief Ascii string1 multiply Ascii string2
* @param [in] pszAsc1	Poniter to Ascii string 1
* @param [in] pszAsc2	Poniter to Ascii string 2
* @param [out] pszProduct Poniter to the product
* @return 
* @li void
*/
void PubAscMulAsc(uchar *pszAsc1, uchar *pszAsc2, uchar *pszProduct )
{
	int i,nLen1,nLen2,nLoop;
	char *pszResult;
	char sMul1[1024]={0};
	char sMul2[1024]={0};
	char sTmp[1024+1]={0};
	
	if(pszAsc1==NULL || pszAsc2==NULL || pszProduct==NULL)
	{
		return;
	}

	nLen1 = strlen((char*)pszAsc1);
	nLen2 = strlen((char*)pszAsc2);
	if(nLen1==0 || nLen1>=1024 || nLen2==0 || nLen2>=1024)
	{
		return;
	}
	strcpy(sMul1, (char*)pszAsc1);
	strcpy(sMul2, (char*)pszAsc2);
	if(ProCheckOper2(sMul1, sMul2) != APP_SUCC)
	{
		return;
	}
	pszResult = (char*)pszProduct;
	if(sMul1[0] == '-' && sMul2[0] == '-')
	{
		ProDelFirstChar(sMul1);
		ProDelFirstChar(sMul2);
	}
	else if(sMul1[0] == '-')
	{
		pszProduct[0] = '-';
		pszResult = (char*)pszProduct+1;
		ProDelFirstChar(sMul1);
	}
	else if(pszAsc2[0] == '-')
	{
		pszProduct[0] = '-';
		pszResult = (char*)pszProduct+1;
		ProDelFirstChar(sMul2);
	}

	nLen1 = strlen(sMul1);
	nLen2 = strlen(sMul2);
	if(strcmp(sMul1, "0")==0 || strcmp(sMul2, "0")==0)
	{
		strcpy((char*)pszProduct, "0");
		return;
	}

	if(ProBigCmp(sMul1, sMul2) < 0)
	{
		strcpy(sTmp, sMul1);
		strcpy(sMul1, sMul2);
		strcpy(sMul2, sTmp);
		i = nLen1;
		nLen1 = nLen2;
		nLen2 = i;
    }

	strcpy(pszResult, "0");
	for (i=nLen2-1; i>=0; i--)
	{
		if((nLoop=(sMul2[i]-0x30)) > 0)
		{
			memset(sTmp, 0, sizeof(sTmp));
			strcpy(sTmp, sMul1);
			for (; nLoop>1; nLoop--)
			{
				PubAscAddAsc((uchar *)sMul1, (uchar *)sTmp, (uchar *)sTmp);
			}
			for (nLoop=1; nLoop<(nLen2-i); nLoop++)
			{
				strcat(sTmp, "0");
			}
			PubAscAddAsc((uchar *)pszResult, (uchar *)sTmp, (uchar *)pszResult);
		}
	}
	return;
}

/**
* @brief Ascii string1 devide Ascii string2
* @param [in] pszAsc1	 Poniter to Ascii string 1
* @param [in] pszAsc2	 Poniter to Ascii string 2
* @param [out] pszResult Pointer to the result
* @return 
* @li void
*/
void PubAscDivAsc(uchar *pszAsc1, uchar *pszAsc2, uchar *pszResult )
{
	int i,j,nLoop,nLen1,nLen2,nSign=0;
	char sDiv1[1024]={0};
	char sDiv2[1024]={0};
	char sRemainder[1024]={0};
	char sTmp[1024]={0};
	char sSingle[1024]={0};

	if(pszAsc1==NULL || pszAsc2==NULL || pszResult==NULL)
	{
		return;
	}

	nLen1 = strlen((char*)pszAsc1);
	nLen2 = strlen((char*)pszAsc2);
	if(nLen1==0 || nLen1>=1024 || nLen2==0 ||  nLen2>=1024)
	{
		return;
	}
	strcpy(sDiv1, (char*)pszAsc1);
	strcpy(sDiv2, (char*)pszAsc2);
	if(ProCheckOper2(sDiv1, sDiv2) != APP_SUCC)
	{
		return;
	}
	if(sDiv1[0] == '-' && sDiv2[0] == '-')
	{
		ProDelFirstChar(sDiv1);
		ProDelFirstChar(sDiv2);
	}
	else if(sDiv1[0] == '-')
	{
		nSign = -1;
		ProDelFirstChar(sDiv1);
	}
	else if(sDiv2[0] == '-')
	{
		nSign = -1;
		ProDelFirstChar(sDiv2);
	}

	nLen1 = strlen(sDiv1);
	nLen2 = strlen(sDiv2);
	if(strcmp(sDiv2, "0") == 0)
	{
		return;
	}
	else if(strcmp(sDiv1, "0") == 0)
	{
		strcpy((char*)pszResult, "0");
		return;
	}

	i = ProBigCmp(sDiv1, sDiv2);
	if (i == 0)
	{
        strcpy((char*)pszResult, "1");
		return;
    }
	else if(i < 0)
	{
		strcpy((char*)pszResult, "0");
		return;
	}

	strcpy((char*)pszResult, "0");
	nLoop = nLen1 - nLen2;
	memcpy(sTmp, sDiv1, nLen2);
	for(i=0; i<=nLoop; i++)
	{
		if(strlen(sRemainder) > 0)
		{
			PubAscAddAsc((uchar*)sTmp, (uchar*)sRemainder, (uchar*)sTmp);
		}
		memset(sRemainder, 0, sizeof(sRemainder));
		memset(sSingle, 0, sizeof(sSingle));
		strcpy(sSingle, "0");
		while(1)
		{
			PubAscSubAsc((uchar*)sTmp, (uchar*)sDiv2, (uchar*)sTmp);
			if(sTmp[0] == '-')
			{
				PubAscSubAsc((uchar*)sDiv2, (uchar*)sTmp+1, (uchar*)sTmp);
				sprintf(sRemainder, "%s0", sTmp);
				break;
			}
			PubAscAddAsc((uchar*)sSingle, (uchar*)"1", (uchar*)sSingle);
		}
		if(strcmp(sSingle, "0") != 0)
		{
			for (j=0; j<nLoop-i; j++)
			{
				strcat(sSingle, "0");
			}
			PubAscAddAsc(pszResult, (uchar*)sSingle, pszResult);
		}
		memset(sTmp, 0, sizeof(sTmp));
		memcpy(sTmp, sDiv1+nLen2+i, 1);
	}
	if(nSign == -1)
	{
		sTmp[0] = '-';
		strcpy(sTmp+1, (char*)pszResult);
		strcpy((char*)pszResult, sTmp);
	}
	return;
}

/**
* @brief Remove '.' character from amount str, e.g. '12.34' to '1234'
* @param [in] pszSource    Source string.
* @param [out] pszTarget   Result
* @param [out] pnTargetLen Length of result.
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubFmtAmtStr (const char* pszSource, char* pszTarget, int* pnTargetLen )
{
	int i = 0, j = 0, unLen = 0, nFlag = 0;

	if (pszSource == NULL || pszTarget == NULL || pnTargetLen == NULL)
	{
		return APP_FAIL;
	}

	unLen = strlen(pszSource);
	if (*pnTargetLen < unLen-1)
	{
		return APP_FAIL;
	}

	for (i=0,j=0; i<unLen; i++,j++)
	{
		if ( ((pszSource[i] < '0') || (pszSource[i] > '9')) && (pszSource[i] != '.'))
		{
			return APP_FAIL;
		}
		if (pszSource[i] == '.')
		{
			j --;
			nFlag++;
			if (nFlag > 1 || i == 0 || i == unLen-1)
			{/*1*/
				return APP_FAIL;
			}
		}
		else
		{
			if(nFlag == 0 && *pnTargetLen <= unLen-1 && i == unLen-1)
			{/*,*/
				return APP_FAIL;
			}
			pszTarget[j] = pszSource[i];
		}
	}
	*pnTargetLen = strlen(pszTarget);
	return APP_SUCC;
}

/**
* @brief Convert Ascii string to Hex data, if the length of string isn't even number, it needs left padding 0, or right padding 0.
* @param [in] pszAsciiBuf Pointer to the buffer
* @param [in] nLen        Length of buffer.
* @param [in] cType       0-left padding  1-right padding
* @param [out] pszBcdBuf  Pointer to the buffer outputed
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubAscToHex (const unsigned char* pszAsciiBuf, int nLen, char cType, unsigned char* pszBcdBuf)
{
	int i;
	char cTmp, cTmp1;

	if (pszAsciiBuf == NULL || pszBcdBuf == NULL || nLen <= 0 || (cType!=0&&cType!=1) )
	{
		return APP_FAIL;
	}

	if (nLen & 0x01 && cType)
	{/**/
		cTmp1 = 0 ;
	}
	else
	{
		cTmp1 = 0x55 ;
	}

	for (i = 0; i < nLen; pszAsciiBuf ++, i ++)
	{
		if ( *pszAsciiBuf >= 'a' && *pszAsciiBuf <= 'f')
		{
			cTmp = *pszAsciiBuf - 'a' + 10 ;
		}
		else if ( *pszAsciiBuf >= 'A' && *pszAsciiBuf <= 'F')
		{
			cTmp = *pszAsciiBuf - 'A' + 10 ;
		}
		else if ( *pszAsciiBuf >= '0' && *pszAsciiBuf <= '9')
		{
			cTmp = *pszAsciiBuf - '0' ;
		}
		else
		{
			cTmp = *pszAsciiBuf;
			cTmp &= 0x0f;
		}

		if ( cTmp1 == 0x55 )
		{
			cTmp1 = cTmp;
		}
		else
		{
			*pszBcdBuf ++ = cTmp1 << 4 | cTmp;
			cTmp1 = 0x55;
		}
	}
	if (cTmp1 != 0x55)
	{
		*pszBcdBuf = cTmp1 << 4;
	}
    return APP_SUCC;
}

/**
* @brief Convert Hex to Ascii
* @param [in] pszBcdBuf    Pointer to the buffer
* @param [in] nLen         Length of buffer.
* @param [in] cType        Alignment:0-left 1-right
* @param [out] pszAsciiBuf 
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubHexToAsc (const unsigned char* pszBcdBuf, int nLen, char cType, unsigned char* pszAsciiBuf)
{
	int i = 0;

	if (pszBcdBuf == NULL || pszAsciiBuf == NULL || nLen <= 0 || (cType != 0 && cType != 1) )
	{
		return APP_FAIL;
	}

	if (nLen & 0x01 && cType) /**/
	{                           /*01*/
		i = 1;
		nLen ++;
	}
	else
	{
		i = 0;
	}
	for (; i < nLen; i ++, pszAsciiBuf ++)
	{
		if (i & 0x01)
		{
			*pszAsciiBuf = *pszBcdBuf ++ & 0x0f;
		}
		else
		{
			*pszAsciiBuf = *pszBcdBuf >> 4;
		}
		if (*pszAsciiBuf > 9)
		{
			*pszAsciiBuf += 'A' - 10;
		}
		else
		{
			*pszAsciiBuf += '0';
		}

	}
	*pszAsciiBuf = 0;
	return APP_SUCC;
}

/**
* @brief Convert an integer to 4 bytes HEX
* @param [in] nNum   input interger number
* @param [out] pszBuf  output buffer
* @return 
* @li void
* @date 2012-06-19
*/
void PubIntToC4 (unsigned char* pszBuf, const unsigned int nNum )
{
	if(pszBuf == NULL)
	{
		return ;
	}
	*( pszBuf ) = nNum >> 24;
	*( pszBuf + 1 ) = (nNum >> 16) ;
	*( pszBuf + 2 ) = (nNum >> 8) ;
	*( pszBuf + 3 ) = nNum %256;
	return ;
}

/**
* @brief Convert an integer to 2 bytes HEX
* @param [in] nNum    input interger number
* @param [out] pszBuf  output buffer
* @return 
* @li void
* @date 2012-06-19
*/
void PubIntToC2 (unsigned char* pszBuf, const unsigned int nNum )
{
	if (pszBuf == NULL )
	{
		return ;
	}
	if( nNum <= 65535)
	{
		*(pszBuf + 1) = nNum % 256;
		*pszBuf = nNum >>8;
	}
	return ;
}

/**
* @fn PubCnToInt
* @brief Convert n bytes HEX to an integer
* @param [in]		pszBuf	[Input string buffer.]
* @param [out]		nNum    	[Output number]
* @param [in]		nLen		[String length]
* @return 
* @li void
* @date 2020-03-24
*/
void PubCnToInt (unsigned int* nNum, unsigned char* pszBuf, int nLen)
{
	int i = 0;
	if (nNum == NULL || pszBuf == NULL)
	{
		return;
	}

	while(nLen > 0)
	{
		*nNum += (*(pszBuf + i) << (8*(nLen-1)));
		i++;
		nLen -= 1;
	}
}

/**
* @brief Convert 4 bytes HEX to an integer
* @param [in] pszBuf  input string buffer.
* @param [out] nNum   output number
* @return 
* @li void
* @date 2012-06-19
*/
void PubC4ToInt (unsigned int* nNum, unsigned char* pszBuf )
{
	if (nNum == NULL || pszBuf == NULL)
	{
		return;
	}
	*nNum = ((*pszBuf) << 24) + (*(pszBuf+1) << 16) + (*(pszBuf+2) << 8) + (*(pszBuf + 3));
}

/**
* @brief Convert 2 bytes HEX to an integer
* @param [in] pszBuf  input string buffer.
* @param [out] nNum   output number
* @return 
* @li void
* @date 2012-06-19
*/
void PubC2ToInt(unsigned int *nNum, unsigned char *pszBuf)
{
	if (nNum == NULL || pszBuf == NULL)
	{
		return;
	}
	*nNum = ((*pszBuf) << 8) + (*(pszBuf + 1));
}

/**
* @brief Convert an integer(0-99) to BCD string(1byte)
* @param [in] nNum  the integer(0-99)
* @param [out] ch   output character
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubByteToBcd(int nNum, char *ch)
{
	if (ch == NULL)
	{
		return APP_FAIL;
	}
	if ((nNum < 0) || (nNum > 99))
	{
		return APP_FAIL;
	}
	*ch = ((nNum / 10) << 4) | (nNum % 10);
	return APP_SUCC;
}

/**
* @brief Convert BCD string(1byte) to an integer(0-99)
* @param [in] ch    input character
* @param [out] pnNum  output interger
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubBcdToByte(char ch, int *pnNum)
{
	if (pnNum == NULL)
	{
		return APP_FAIL;
	}
	if (((ch & 0x0F) > 9) || ((ch >> 4) > 9))
	{
		return APP_FAIL;
	}

	*pnNum = (ch >> 4) * 10 + (ch & 0x0f);
	return APP_SUCC;
}

/**
* @brief Convert an integer(0-9999) to BCD string(2bytes)
* @param [in] nNum      input interger(0-9999)
* @param [out] psBcd    output bcd string
* @param [out] pnBcdLen  output the length of bcd string
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubIntToBcd(char *psBcd, int *pnBcdLen, int nNum)
{
	if (psBcd == NULL || pnBcdLen == NULL)
	{
		return APP_FAIL;
	}
	if ((nNum < 0) || (nNum > 9999) || (*pnBcdLen < 2))
	{
		return APP_FAIL;
	}

	PubByteToBcd(nNum / 100, &psBcd[0]);
	PubByteToBcd(nNum % 100, &psBcd[1]);
	*pnBcdLen = 2;
	return APP_SUCC;
}

/**
* @brief  Convert BCD string(2bytes) to an integer(0-9999)
* @param [in] psBcd  input bcd string
* @param [out] nNum  output interger(0-9999)
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubBcdToInt(const char * psBcd, int *nNum)
{
	int nNum1, nNum2;

	if (psBcd == NULL || nNum == NULL || psBcd == NULL)
	{
		return APP_FAIL;
	}

	if (PubBcdToByte(psBcd[0], &nNum1) == APP_FAIL)
	{
		return APP_FAIL;
	}
	if (PubBcdToByte(psBcd[1], &nNum2) == APP_FAIL)
	{
		return APP_FAIL;
	}
	*nNum = nNum1 * 100 + nNum2;
	return APP_SUCC;
}

/**
* @brief Calculate LRC
* @param [in] psBuf  The buffer used to calculate
* @param [in] nLen   length of buffer
* @param [out] chLRC LRC
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubCalcLRC(const char *psBuf, int nLen, char *chLRC)
{
	int	i;
	if (psBuf == NULL || chLRC == NULL || nLen <= 0)
	{
		return APP_FAIL;
	}

	*chLRC = 0x00;
	for (i = 0; i < nLen; i++)
	{
		*chLRC ^= psBuf[i];
	}
	return APP_SUCC;
}

/**
* @brief Delete left spaces in the string
* @param [in] pszSrc  Source string
* @param [out] pszSrc String after deleting
* @return 
* @li void
* @date 2012-06-19
*/
void PubLeftTrim(char *pszSrc)
{
	int j, k, nLen;

	if(pszSrc == NULL)
	{
		return;
	}

	j = 0;
	nLen = strlen(pszSrc);
	while ((j < nLen) && (pszSrc[j] == ' '))
	{
		j++;
	}
	for (k = j; k <= nLen; k++)
	{
		pszSrc[k-j] = pszSrc[k];
	}
}

/**
* @brief Delete right spaces in the string
* @param [in] pszSrc  Source string
* @param [out] pszSrc String after deleting
* @return 
* @li void
* @date 2012-06-19
*/
void PubRightTrim(char *pszSrc)
{
	int i;

	if (pszSrc == NULL)
	{
		return;
	}
	i = strlen(pszSrc);
	while ((i > 0) && (pszSrc[i-1] == ' '))
	{
		i--;
		pszSrc[i] = '\0';
	}
}

/**
* @brief Delete both side spaces in the string
* @param [in] pszSrc  Source string
* @param [out] pszSrc String after deleting
* @return 
* @li void
* @date 2012-06-19
*/
void PubAllTrim(char *pszSrc)
{
	int i, j, k;

	if (pszSrc == NULL)
	{
		return;
	}
	i = strlen(pszSrc);
	while ((i > 0) && (pszSrc[i-1] == ' '))
	{
		i--;
		pszSrc[i] = '\0';
	}

	j = 0;
	while ((j < i) && (pszSrc[j] == ' '))
	{
		j++;
	}
	for (k = j; k <= i; k++)
	{
		pszSrc[k-j] = pszSrc[k];
	}
}

/**
* @brief Delete a speficied charactor from a string
* @param [in] pszString		Source string
* @param [in] ucSymbol	  the speficied character
* @param [out] pszString	Output string
* @return
* li 
*/
void PubDelSymbolFromStr(uchar *pszString, uchar ucSymbol)
{
	int i,j;

	if(pszString == NULL)
	{
		return;
	}
	for (i=0,j=0; i<strlen((char*)pszString); i++)
	{
		if (pszString[i] != ucSymbol)
		{
			pszString[j] = pszString[i];
			j++;
		}
	}
	pszString[j] = 0;
}

/**
* @brief Add a charactor to a string, make it to a specified length
* @param [in] pszString  Source string
* @param [in] nLen       The length after adding
* @param [in] ch         The specified charactor
* @param [in] nOption    Operation type--EM_ADDCH_MODE
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubAddSymbolToStr(char *pszString, int nLen, char ch, int nOption)
{
	int i, j, p;

	if (pszString == NULL)
	{
		return APP_FAIL;
	}
	if ((i = strlen(pszString)) >= nLen)
	{
		return APP_FAIL;
	}

	switch (nOption)
	{
		case ADDCH_MODE_BEFORE_STRING:
			p = nLen-i;
			for (j = i; j >= 0; j--)
			{
				pszString[j+p] = pszString[j];
			}
			for (i = 0; i < p; i++)
			{
				pszString[i] = ch;
			}
			break;
		case ADDCH_MODE_AFTER_STRING:
			for (; i < nLen; i++)
			{
				pszString[i] = ch;
			}
			pszString[i] = '\0';
			break;
		case ADDCH_MODE_BEFORE_AFTER_STRING:
			p = (nLen - i) / 2;
			for (j = i; j >= 0; j--)
			{
				pszString[j+p] = pszString[j];
			}

			for (i = 0; i < p; i++)
			{
				pszString[i] = ch;
			}

			i = strlen(pszString);
			for (; i < nLen; i++)
			{
				pszString[i] = ch;
			}
			pszString[i] = '\0';
			break;
		default:
			return APP_FAIL;
	}
	return APP_SUCC;
}

/**
* @brief Delete a specified charater at left of a string
* @param [in] pszSrc 	Source string
* @param [in] ucRemoveChar	a character need to delete
* @param [out] pszSrc		result
* @return
* li void
*/
void PubLeftTrimChar(uchar *pszSrc, uchar ucRemoveChar)
{
	int j, k, nLen;

	if(pszSrc == NULL)
	{
		return;
	}

	j = 0;
	nLen = strlen((char*)pszSrc);
	while ((j < nLen) && (pszSrc[j] == ucRemoveChar))
	{
		j++;
	}
	for (k = j; k <= nLen; k++)
	{
		pszSrc[k-j] = pszSrc[k];
	}
	pszSrc[nLen-j] = '\0';
}

/**
* @brief Delete a specified charater at right of a string
* @param [in] pszSrc	Source string
* @param [in] ucRemoveChar	a character need to delete
* @param [out] pszSrc	result
* @return
* li void
*/
void PubRightTrimChar(uchar *pszSrc, uchar ucRemoveChar)
{
	int i;
	if(pszSrc == NULL)
	{
		return;
	}

	i = strlen((char*)pszSrc);
	while ((i > 0) && (pszSrc[i-1] == ucRemoveChar))
	{
		i--;
		pszSrc[i] = '\0';
	}
}

/**
* @brief Set Radix point
* @param [in] pszNum	Source string
* @param [in] btyes after point
* @param [out] pszNum	result
* @return
* li void
*/
static void SetRadixpoint(char * pszNum,int nPoint)
{
	int nLen,i;

	if (nPoint == 0)
	{
		return;
	}
	
	nLen = strlen(pszNum);
	if (nLen < 3)
	{
		PubAddSymbolToStr(pszNum, 3, '0', 0);
		nLen = 3;
	}
	if (nLen < nPoint + 1)
	{
	    PubAddSymbolToStr(pszNum, nPoint + 1, '0', 0);
		nLen = nPoint + 1;
	}
	for (i=nLen; i>nLen-nPoint; i--)
	{
		pszNum[i] = pszNum[i-1];
	}
	pszNum[nLen-nPoint] = '.';
	pszNum[nLen+1] = 0;
}

/**
* @brief	Format the amount string, such as HKD$****.**, RMB****.**
* @param [in] pszPrefix 	Prefix Such as "HKD"in "HKD 200.00"
* @param [in] pszIn			The string of integer part.
* @param [in] ucRadPt		The string of decimal part, must be less than 4
* @param [out] pszOut		Output string
* @return
* li coverted number.
*/
void PubConvAmount(uchar *pszPrefix, uchar *pszIn, uchar ucRadPt, uchar *pszOut)
{
	char szNum[32+1]={0};

	if(pszIn == NULL || pszOut == NULL)
	{
		return;
	}
	if(PubIsDigitStr((char*)pszIn) != APP_SUCC)
	{
		return;
	}

	strcpy(szNum, (char*)pszIn);
	PubLeftTrimChar((uchar*)szNum, '0');//'0'
	if (ucRadPt >= 6)
	{
		return;
	}
	SetRadixpoint(szNum, ucRadPt);
	if (pszPrefix == NULL)
	{
		memcpy(pszOut, szNum, strlen(szNum)+1);
	}
	else
	{
		sprintf((char*)pszOut, "%s%s", pszPrefix, szNum);
		pszOut[strlen((char*)pszPrefix)+strlen(szNum)] = 0x00;
	}
}

/**
* @brief Check if the string is digit
* @param [in] pszString    input string
* @return 
* @li APP_FAIL NO
* @li APP_SUCC YES
* @date 2012-06-19
*/
int PubIsDigitStr(const char *pszString)
{
	char c;
	int i;

	if(pszString == NULL)
	{
		return APP_FAIL;
	}
	for(i=0; (c = *(pszString+i)) != '\0'; i++)
	{
		if (PubIsDigitChar(c) == APP_FAIL)
		{
			return APP_FAIL;
		}
	}
	return APP_SUCC;
}

/**
* @brief Check if the charactor is digit
* @param [in] ch  input charactor
* @return 
* @li APP_FAIL NO
* @li APP_SUCC YES
* @date 2012-06-19
*/
int PubIsDigitChar(char ch)
{
	if ((ch >= '0') && (ch <= '9'))
	{
		return APP_SUCC;
	}
	else
	{
		return APP_FAIL;
	}
}

/**
* @brief Intercept a string and switch to a number
* @param [in] ch input charactor
* @return 
* @li APP_FAIL NO
* @li APP_SUCC YES
* @date 2012-06-19
*/
static int ProSubNum(const char *psString, int nStart, int nNumber)
{
	int nNum;
	char sBuf[6];

	if (psString == NULL || nNumber > 5)
	{
		return APP_FAIL;
	}

	memcpy(sBuf, psString + nStart, nNumber);
	*(sBuf + nNumber) = '\0';
	sscanf(sBuf, "%d", &nNum);
	return (nNum);
}

/**
* @brief Check if the format of date is valid 
* @param [in] pDate Date format  YYYYMMDD
* @return 
* @li APP_FAIL Illegality
* @li APP_SUCC Valid
* @date 2012-06-19
*/
int  PubIsValidDate(const char *pDate)
{
	int nYear, nMonth, nDay, nMonthDays;

	if (pDate == NULL || strlen(pDate) != 8)
	{
		return APP_FAIL;
	}
	if (PubIsDigitStr(pDate) == APP_FAIL )
	{
		return APP_FAIL;
	}

	nYear  = ProSubNum(pDate, 0, 4);
	nMonth = ProSubNum(pDate, 4, 2);
	nDay   = ProSubNum(pDate, 6, 2);

	if (nYear < 1970)
	{
		return APP_FAIL;
	}
	if (PubMonthDays(nYear, nMonth, &nMonthDays) == APP_FAIL)
	{
		return APP_FAIL;
	}
	if (( nDay < 1 ) || (nDay > nMonthDays))
	{
		return APP_FAIL;
	}
	return APP_SUCC;
}

/**
* @brief Get current date time
* @param [out] pDatetime Format is YYYYMMDDHHMMSS, length >14
* @return 
* @li void
* @date 2012-06-19
*/
void PubGetCurrentDatetime(char *pDatetime)
{
	struct tm stPosTime;

	if (pDatetime == NULL)
	{
		return;
	}
	memset(&stPosTime, 0, sizeof(stPosTime));
	NAPI_SysGetDateTime(&stPosTime);
	sprintf(pDatetime, "%04d%02d%02d%02d%02d%02d", stPosTime.tm_year+1900, stPosTime.tm_mon+1,\
		stPosTime.tm_mday, stPosTime.tm_hour, stPosTime.tm_min, stPosTime.tm_sec);
}

/**
* @brief Set date time in POS
* @param [in] pszDate date
* @param [in] pszDateFormat Date format like "MMDD" or "YYYYMMDD"
* @param [in] pszTime time
* @return 
* @li APP_SUCC Success
* @li APP_FAIL Fail
*/
int PubSetPosDateTime(const char *pszDate, const char *pszDateFormat, const char *pszTime)
{
	char sztmp[5];
	struct tm stPosTime;

	if (NULL == pszDate || NULL == pszDateFormat || NULL == pszTime || strlen(pszTime) != 6)
	{
		return APP_FAIL;
	}
	if(PubIsDigitStr(pszDate) != APP_SUCC || PubIsDigitStr(pszTime) != APP_SUCC)
	{
		return APP_FAIL;
	}

	if (NAPI_SysGetDateTime(&stPosTime) != NAPI_OK)
	{
		return APP_FAIL;
	}

	if (strcmp(pszDateFormat, "YYYYMMDD") == 0)
	{
		memset(sztmp, 0, sizeof(sztmp));
		memcpy(sztmp, pszDate, 4);
		stPosTime.tm_year = atoi(sztmp) - 1900;

		memset(sztmp, 0, sizeof(sztmp));
		memcpy(sztmp, pszDate+4, 2);
		stPosTime.tm_mon = atoi(sztmp) - 1;

		memset(sztmp, 0, sizeof(sztmp));
		memcpy(sztmp, pszDate+6, 2);
		stPosTime.tm_mday = atoi(sztmp);
	}
	else if (strcmp(pszDateFormat, "YYMMDD") == 0)
	{
		memset(sztmp, 0, sizeof(sztmp));
		memcpy(sztmp, pszDate, 2);
		stPosTime.tm_year = (stPosTime.tm_year/100)*100 + atoi(sztmp);

		memset(sztmp, 0, sizeof(sztmp));
		memcpy(sztmp, pszDate+2, 2);
		stPosTime.tm_mon = atoi(sztmp) - 1;

		memset(sztmp, 0, sizeof(sztmp));
		memcpy(sztmp, pszDate+4, 2);
		stPosTime.tm_mday = atoi(sztmp);
	}
	else if (strcmp(pszDateFormat, "MMDD") == 0)
	{
		memset(sztmp, 0, sizeof(sztmp));
		memcpy(sztmp, pszDate, 2);
		stPosTime.tm_mon = atoi(sztmp) - 1;

		memset(sztmp, 0, sizeof(sztmp));
		memcpy(sztmp, pszDate+2, 2);
		stPosTime.tm_mday = atoi(sztmp);
	}
	else
	{
		return APP_FAIL;
	}

	memset(sztmp, 0, sizeof(sztmp));
	memcpy(sztmp, pszTime, 2);
	stPosTime.tm_hour = atoi(sztmp);

	memset(sztmp, 0, sizeof(sztmp));
	memcpy(sztmp, pszTime+2, 2);
	stPosTime.tm_min = atoi(sztmp);

	memset(sztmp, 0, sizeof(sztmp));
	memcpy(sztmp, pszTime+4, 2);
	stPosTime.tm_sec = atoi(sztmp);

	if (NAPI_SysSetDateTime(&stPosTime) != NAPI_OK)
	{
		return APP_FAIL;
	}
	return APP_SUCC;
}

/**
* @brief The total number of day in a certain month.
* @param [in] nYear   
* @param [in] nMon    
* @param [out] pnDays  Total days in a certain month.
* @return 
* @li APP_FAIL 
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubMonthDays(int nYear, int nMon, int *pnDays)
{
	int monthdays[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	if (pnDays == NULL )
	{
		return  APP_FAIL;
	}
	if ((nMon < 1) || (nMon > 12) || (nYear <= 0) )
	{
		*pnDays = 0;
		return APP_FAIL;
	}

	if (nMon == 2)
	{
		if (PubIsLeapYear(nYear) == APP_SUCC)
		{
			*pnDays = monthdays[nMon-1] + 1;
		}
		else
		{
			*pnDays = monthdays[nMon-1];
		}
	}
	else
	{
		*pnDays = monthdays[nMon-1];
	}
	return APP_SUCC;
}

/**
* @brief Check if the year is leap year
* @param [in] nYear input year
* @return 
* @li APP_FAIL Not leap year
* @li APP_SUCC Leap year
* @date 2012-06-19
*/
int PubIsLeapYear(int nYear)
{
	if ((nYear % 400) == 0)
	{
 		return APP_SUCC;
 	}
	if ((nYear % 4 == 0) && (nYear % 100 != 0))
	{
		return APP_SUCC;
	}
	else
	{
		return APP_FAIL;
	}
}

static uint ProGetTimer(uint unMsTime)
{
	struct timeval stTime;

	gettimeofday(&stTime, NULL); 
	return (uint)(unMsTime + ((long)stTime.tv_sec)*1000+(long)stTime.tv_usec/1000);
}

/**
* @brief get timer count of timeout
* @param [in]  unMsTime  Timeout (Unit:ms)
* @return 
* @li  Time count
* @author Liug
* @date 2012-10-24
*/
uint PubGetOverTimer(uint unMsTime)
{
	return ProGetTimer(unMsTime);
}

/**
* @brief Check if timer is timeout.
* @param [in]  unOverTime  Timer count (unit:ms, usually get from PubGetOverTimer returning)
* @return 
* @li APP_APP_SUCC Timeout
* @li APP_APP_FAIL No timeout
* @author Liug
* @date 2012-10-24
*/
int PubTimerIsOver(uint unOverTime)
{
	uint unNowTime;
	
	unNowTime = ProGetTimer(0);
	if (unOverTime < unNowTime)
	{
		return APP_SUCC;
	}
	else
	{
		return APP_FAIL;
	}
}

/**
* @brief Get tool lib version
* @param [out] pszVer Greater than 12bytes
* @return 
* @li void
* @date 2012-06-19
*/
void PubGetToolsVer(char *pszVer)
{
	if (pszVer != NULL)
	{
		strcpy (pszVer, PUBTOOLSVER);
	}
	return;
}


/*****************************************************
**File operation section
******************************************************/

#define _U 0x01 /* upper letter[A-Z]*/
#define _L 0x02 /* lower letter[a-z]*/
#define _D 0x04 /* digit character[0-9]*/
#define _C 0x08 /* cntrl character*/
#define _P 0x10 /* punct character*/
#define _S 0x20 /* white space (space/lf/tab) such space,\t,\n and so on*/
#define _X 0x40 /* hex digit */
#define _SP 0x80 /* hard space (0x20) */
static unsigned char gucType[] =
{
	_C,_C,_C,_C,_C,_C,_C,_C,   /* 0-7 */
	_C,_C|_S,_C|_S,_C|_S,_C|_S,_C|_S,_C,_C,  /* 8-15 */
	_C,_C,_C,_C,_C,_C,_C,_C,   /* 16-23 */
	_C,_C,_C,_C,_C,_C,_C,_C,   /* 24-31 */
	_S|_SP,_P,_P,_P,_P,_P,_P,_P,   /* 32-39 */
	_P,_P,_P,_P,_P,_P,_P,_P,   /* 40-47 */
	_D,_D,_D,_D,_D,_D,_D,_D,   /* 48-55 */
	_D,_D,_P,_P,_P,_P,_P,_P,   /* 56-63 */
	_P,_U|_X,_U|_X,_U|_X,_U|_X,_U|_X,_U|_X,_U, /* 64-71 */
	_U,_U,_U,_U,_U,_U,_U,_U,   /* 72-79 */
	_U,_U,_U,_U,_U,_U,_U,_U,   /* 80-87 */
	_U,_U,_U,_P,_P,_P,_P,_P,   /* 88-95 */
	_P,_L|_X,_L|_X,_L|_X,_L|_X,_L|_X,_L|_X,_L, /* 96-103 */
	_L,_L,_L,_L,_L,_L,_L,_L,   /* 104-111 */
	_L,_L,_L,_L,_L,_L,_L,_L,   /* 112-119 */
	_L,_L,_L,_P,_P,_P,_P,_C,   /* 120-127 */
//	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* 128-143 */
//	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* 144-159 */
	_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,   /* 128-143 */
	_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,       /* 144-159 */	
	_S|_SP,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,   /* 160-175 */
	_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,       /* 176-191 */
	_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,       /* 192-207 */
	_U,_U,_U,_U,_U,_U,_U,_P,_U,_U,_U,_U,_U,_U,_U,_L,       /* 208-223 */
	_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,       /* 224-239 */
	_L,_L,_L,_L,_L,_L,_L,_P,_L,_L,_L,_L,_L,_L,_L,_L		/* 240-255 */
};

#define __ismask(x)	(gucType[(int)(unsigned char)(x)])
#define Proisalnum(c) ((__ismask(c)&(_U|_L|_D)) != 0)
#define Proisalpha(c) ((__ismask(c)&(_U|_L)) != 0)
#define Proiscntrl(c) ((__ismask(c)&(_C)) != 0)
#define Proisdigit(c) ((__ismask(c)&(_D)) != 0)
#define Proisgraph(c) ((__ismask(c)&(_P|_U|_L|_D)) != 0)
#define Proislower(c) ((__ismask(c)&(_L)) != 0)
#define Proisprint(c) ((__ismask(c)&(_P|_U|_L|_D|_SP)) != 0)
#define Proispunct(c) ((__ismask(c)&(_P)) != 0)
#define Proisspace(c) ((__ismask(c)&(_S)) != 0)
#define Proisupper(c) ((__ismask(c)&(_U)) != 0)
#define Proisxdigit(c) ((__ismask(c)&(_D|_X)) != 0)
#define Proisascii(c) (((unsigned char)(c)) <= 0x7f)
#define Protoascii(c) (((unsigned char)(c))&0x7f)

/*base parameter info*/
#define RECMAXLEN		1024*8		/*Max length of record*/
#define FILEHEADER_LEN	139	   		/*Length of file header*/
#define LINE_LEN		128			// Set   line   length   in   configuration   files


static long ProFilelength (int nFd)
{
	ulong ulFileLen = 0;
	ulong ulOldOffset = 0;
	if(PubFsTell(nFd, &ulOldOffset) != NAPI_OK)
		return APP_FAIL;
	if(PubFsSeek(nFd, 0L, SEEK_END) != NAPI_OK)
		return APP_FAIL;
	if(PubFsTell(nFd, &ulFileLen) != NAPI_OK)
		return APP_FAIL;
	if(PubFsSeek(nFd, ulOldOffset, SEEK_SET) != NAPI_OK)
		return APP_FAIL;
	return (long)ulFileLen;
}

/**
* @brief Write header of record file
* @param [in] nFd    file indentifier
* @param [in] pstRecFile structure of record file
* @return 
* @li void
* @date 2012-06-19
*/
static int ProWriteRecFileHeader (int nFd, const STRECFILE *pstRecFile)
{
	int nOff = 0;
	unsigned char usHeader[FILEHEADER_LEN];

	if (pstRecFile == NULL)
	{
		return APP_FAIL;
	}
	memset (usHeader, 0, sizeof (usHeader));
	//record length
	PubIntToC2( usHeader,  pstRecFile->unMaxOneRecLen );
	nOff += 2 ;
	//start position of index1
	PubIntToC2( usHeader + nOff,  pstRecFile->unIndex1Start );
	nOff += 2 ;
	//length of index1
	PubIntToC2( usHeader + nOff, pstRecFile->unIndex1Len );
	nOff += 2 ;
	//start position of index2
	PubIntToC2( usHeader + nOff, pstRecFile->unIndex2Start );
	nOff += 2 ;
	//length of index2
	PubIntToC2( usHeader + nOff, pstRecFile->unIndex2Len );
	nOff += 2 ;
	//index file name
	memcpy (usHeader + nOff, pstRecFile->szFileName, 128);
	nOff += 128 ;

	usHeader[nOff] = pstRecFile->cIsIndex;

	if(PubFsWrite (nFd, (char *)usHeader, FILEHEADER_LEN) != FILEHEADER_LEN)
	{
		return APP_FAIL;
	}
	return APP_SUCC ;
}

/**
* @brief read header of record file
* @param [in] nFd    file indentifier
* @param [out] pstRecFile structure of record file
* @return 
* @li void
* @date 2012-06-19
*/
static int ProReadRecFileHeader (int nFd, STRECFILE * pstRecFile)
{
	unsigned long ulOldOffset;
	unsigned char usHeader[FILEHEADER_LEN];
	if (pstRecFile == NULL)
	{
		return APP_FAIL;
	}

	PubFsTell(nFd, &ulOldOffset);
	if(PubFsSeek (nFd, 0L, SEEK_SET) != NAPI_OK)
	{
		return APP_FAIL;
	}

	memset(usHeader, 0, sizeof (usHeader));
	if (PubFsRead(nFd, (char *)usHeader, FILEHEADER_LEN) != FILEHEADER_LEN)
	{
		return APP_FAIL;
	}
	if(PubFsSeek (nFd, ulOldOffset, SEEK_SET) != NAPI_OK)
	{
		return APP_FAIL;
	}

	//
	PubC2ToInt( &(pstRecFile->unMaxOneRecLen), usHeader );
	//1
	PubC2ToInt( &(pstRecFile->unIndex1Start), usHeader + 2 );
	//1
	PubC2ToInt( &(pstRecFile->unIndex1Len), usHeader + 4 );
	//2
	PubC2ToInt( &(pstRecFile->unIndex2Start), usHeader + 6 );
	//2
	PubC2ToInt( &(pstRecFile->unIndex2Len), usHeader + 8 );
	//
	memcpy(pstRecFile->szFileName, usHeader+10, 128);
	//
	pstRecFile->cIsIndex = usHeader[138];
	return APP_SUCC;
}

/**
* @brief Update appointed record
* @param [in] stHeader
* @param [in] nRecNo
* @param [in] pszBuf
* @return 
* @li void
* @date 2012-06-19
*/
static int ProUpdateIndex (STRECFILE stHeader, int nRecNo, const char *pszBuf)
{
	int nRet, nFd, nIndexLen;
	char szIndexName[128] = {0};
	char sBuf[RECMAXLEN];

	if (pszBuf == NULL)
	{
		return APP_FAIL;
	}
	/**/
	if (stHeader.cIsIndex == FILE_NOCREATEINDEX)
	{
		return APP_SUCC;
	}

	//
	if (strlen (stHeader.szFileName) == 0)
	{
		return APP_FAIL;
	}


	sprintf(szIndexName, "%s.idx", stHeader.szFileName);	
	if((nFd=PubFsOpen (szIndexName, "w")) < 0)             
	{/**/
		return APP_FAIL;
	}

	nIndexLen = stHeader.unIndex1Len + stHeader.unIndex2Len;
	if(ProFilelength (nFd) != 0)   //
	{
		nRet = PubFsSeek (nFd, ((long)nRecNo - 1) * ((long)nIndexLen + 2), SEEK_SET);
		if(nRet != NAPI_OK)
		{
			PubFsClose(nFd);
			return APP_FAIL;
		}
	}

	/*   (2) + */
	memset(sBuf, 0, sizeof (sBuf));
	PubIntToC2( (unsigned char *) sBuf, (unsigned int)nRecNo );
	memcpy(sBuf + 2,  pszBuf + stHeader.unIndex1Start-1, stHeader.unIndex1Len);
	memcpy( sBuf + 2 + stHeader.unIndex1Len, pszBuf+stHeader.unIndex2Start-1, \
			stHeader.unIndex2Len);
	nRet = PubFsWrite(nFd, sBuf, nIndexLen + 2);
	PubFsClose (nFd);
	if(nRet != nIndexLen + 2)
	{
		return APP_FAIL;
	}
	return APP_SUCC;
}

/**
* @brief Read appointed record
* @param [in] nFd
* @param [in] nRecNo
* @param [in] stHeader
* @param [out] pszBuf  index data
* @return 
* @li void
* @date 2012-06-19
*/
static int ProReadRecFromFile (int nFd, int nRecNo, STRECFILE stHeader, char *psBuf)
{
	long lOffset;

	if (psBuf == NULL || nRecNo <= 0)
	{
		return APP_FAIL;
	}

	lOffset = (long)FILEHEADER_LEN + ((long)nRecNo - 1) * stHeader.unMaxOneRecLen;
	if (PubFsSeek (nFd, lOffset, SEEK_SET) != NAPI_OK)
	{	
		return APP_FAIL;
	}

	if(PubFsRead(nFd, psBuf, stHeader.unMaxOneRecLen) != stHeader.unMaxOneRecLen)
	{
		return APP_FAIL;
	}
	return APP_SUCC;
}

/**
* @brief Delete appointed record
* @param [in] nBeginRecNo
* @param [out] pszBuf  index data
* @return 
* @li void
* @date 2012-06-19
*/
static int ProDelIndex (STRECFILE stHeader, const int nBeginRecNo)
{
	int nI, nFd, nIndexLen;
	long lIndexFileLen;
	char szIndexName[128] = {0};
	char sBuf[RECMAXLEN];

	if(stHeader.cIsIndex != FILE_CREATEINDEX || nBeginRecNo < 1 || strlen (stHeader.szFileName) == 0)
	{/*,,*/
		return APP_FAIL;
	}

	sprintf(szIndexName, "%s.idx", stHeader.szFileName);
	if ((nFd = PubFsOpen (szIndexName, "w")) < 0)
	{
		return APP_FAIL;
	}

	lIndexFileLen = ProFilelength(nFd);
	nIndexLen = stHeader.unIndex1Len+stHeader.unIndex2Len+2;

	if(nBeginRecNo >= lIndexFileLen/nIndexLen)	
	{/**/
		PubFsClose(nFd);
		PubFsTruncate(szIndexName,lIndexFileLen-nIndexLen);
		return APP_SUCC;

	}
 	PubFsSeek (nFd, ((long)nBeginRecNo *(long) nIndexLen), SEEK_SET);	/**/

	for(nI=nBeginRecNo; nI<(lIndexFileLen/nIndexLen); nI++)
	{
		memset(sBuf, 0, sizeof(sBuf));
		if (PubFsRead(nFd, sBuf, nIndexLen) != nIndexLen)
		{
			PubFsClose(nFd);
			return APP_FAIL;
		}
		PubFsSeek (nFd, -(2 * (long)nIndexLen), SEEK_CUR);
		PubIntToC2((uchar *) sBuf, (uint)nI);	/**/
		if (PubFsWrite (nFd, sBuf, nIndexLen) != nIndexLen)
		{
			PubFsClose(nFd);
			return APP_FAIL;
		}
		PubFsSeek (nFd, nIndexLen, SEEK_CUR);	/**/
	}
	PubFsClose (nFd);
	PubFsTruncate(szIndexName, lIndexFileLen - nIndexLen);
	return APP_SUCC;
}

/**
* @brief Seatch the record file by index and return the record number
* @param [in] nBeginRecNo
* @param [out] pszBuf  index data
* @return 
* @li void
* @date 2012-06-19
*/
static int ProReadIndex (STRECFILE stHeader, const char *psIndexStr1,
						 const char *psIndexStr2, const int nBeginRecNo)
{
	int nFd, nIndexLen, nRecNo;
	char szIndexName[128] = {0};
	char sBuf[RECMAXLEN];

	/**/
	if(stHeader.cIsIndex != FILE_CREATEINDEX)
	{
		return APP_FAIL;
	}

	/**/
	if(((psIndexStr1 == NULL)&&(psIndexStr2 == NULL)) || (nBeginRecNo < 1))
	{
		return APP_FAIL;
	}

	/**/
	if(strlen (stHeader.szFileName) == 0)
	{
		return APP_FAIL;
	}

	/**/
	sprintf(szIndexName, "%s.idx", stHeader.szFileName);
	if ((nFd = PubFsOpen (szIndexName, "r")) < 0)
	{
		return APP_FAIL;
	}

	nIndexLen = stHeader.unIndex1Len+stHeader.unIndex2Len;
	if((long)(nIndexLen+2)*((long)nBeginRecNo-1) >= ProFilelength(nFd))
	{
		PubFsClose(nFd);
		return APP_FAIL;
	}
	if(PubFsSeek (nFd, (long)(nIndexLen+2) * ((long)nBeginRecNo -1), SEEK_SET) != NAPI_OK)
	{
		PubFsClose(nFd);
		return APP_FAIL;
	}
	/**/
	while (1)
	{
		memset (sBuf, 0, sizeof(sBuf));
		if(PubFsRead(nFd, sBuf, nIndexLen + 2) != nIndexLen + 2)
		{
			PubFsClose (nFd);
			return APP_FAIL;
		}

		/**/
		if((psIndexStr1 != NULL)&&(psIndexStr2 != NULL))
		{
			if((memcmp(sBuf + 2, psIndexStr1, stHeader.unIndex1Len) == 0)&&\
				(memcmp(sBuf+2+stHeader.unIndex1Len, psIndexStr2, stHeader.unIndex2Len) == 0))
			{
				PubC2ToInt( (unsigned int *) &nRecNo, (unsigned char *)sBuf ) ;
				break;
			}
		}
		else if(psIndexStr1 != NULL)
		{
			if(memcmp(sBuf+2, psIndexStr1, stHeader.unIndex1Len) == 0)
			{
				PubC2ToInt( (unsigned int *) &nRecNo, (unsigned char *)sBuf ) ;
				break;
			}
		}
		else if(psIndexStr2 != NULL)
		{
			if(memcmp(sBuf+2+stHeader.unIndex1Len, psIndexStr2, stHeader.unIndex2Len) == 0)
			{
				PubC2ToInt( (unsigned int *) &nRecNo, (unsigned char *)sBuf ) ;
				break;
			}
		}
	}
	PubFsClose (nFd);
	return nRecNo;
}

/**
* @brief Create record file and index files if necessary
* @param [in] pstRecFile The structure of file.
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubCreatRecFile(const STRECFILE *pstRecFile)
{
	int nRet, nFd;
	char szRecFileName[128] = {0};

	if (pstRecFile == NULL || strlen(pstRecFile->szFileName) == 0 || pstRecFile->unMaxOneRecLen > RECMAXLEN)
	{
		return APP_FAIL;
	}

	if (FILE_CREATEINDEX == pstRecFile->cIsIndex)
	{/**/
		if((pstRecFile->unIndex1Start < 1)||(pstRecFile->unIndex2Start < 1)\
			||(pstRecFile->unIndex1Len < 1)||(pstRecFile->unIndex2Len < 1)\
			||((pstRecFile->unIndex1Start + pstRecFile->unIndex1Len) > pstRecFile->unMaxOneRecLen)\
			||((pstRecFile->unIndex2Start + pstRecFile->unIndex2Len)>pstRecFile->unMaxOneRecLen)\
			||(pstRecFile->unMaxOneRecLen < 1 ))
		{
			return APP_FAIL;
		}
	}

	if(FILE_CREATEINDEX == pstRecFile->cIsIndex)
	{/**/
		strcpy (szRecFileName, pstRecFile->szFileName);
		strcat (szRecFileName, ".idx");
		if(PubFsExist(szRecFileName) == NAPI_OK)
		{/**/
			if(PubFsDel(szRecFileName)!=NAPI_OK)
			{
				return APP_FAIL;
			}
		}
		if ((nFd = PubFsOpen (szRecFileName, "w")) < 0)
		{/**/
			return APP_FAIL;
		}
		PubFsClose(nFd);
	}

	if(PubFsExist(pstRecFile->szFileName) == NAPI_OK)
	{/**/
		if(PubFsDel(pstRecFile->szFileName) != NAPI_OK)
		{
			return APP_FAIL;
		}
	}
	if ((nFd = PubFsOpen (pstRecFile->szFileName, "w")) < 0)
	{/**/
		return APP_FAIL;
	}

	nRet = ProWriteRecFileHeader (nFd, pstRecFile);
	PubFsClose (nFd);
	return nRet;
}

/**
* @brief Open file
* @param [in] pszFileName   File name
* @param [in] pszMode       Open mode only "w"(read & write), "r"(read)
* @param [out] pnFileHandle File handle
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubOpenFile (const char *pszFileName, const char *pszMode,int *pnFileHandle)
{
	int nFd;

	if((pszFileName == NULL) || (pszMode == NULL) || (pnFileHandle == NULL))
	{
		return APP_FAIL;
	}

	if ( (nFd = PubFsOpen( pszFileName, pszMode )) < 0 )
	{
		return APP_FAIL;
	}
	*pnFileHandle = nFd;
	return APP_SUCC;
}

/**
* @brief Read data from an opened file
* @param [in] nHandle   File handle
* @param [in] unLength  Length
* @param [out] psBuffer Data in file
* @return 
* @li APP_FAIL Fail
* @li The real length of reading
*/
int PubReadFile(int nHandle, char * psBuffer, uint unLength)
{
	int nLen;

	if(psBuffer == NULL)
	{
		return APP_FAIL;
	}
	if((nLen=PubFsRead(nHandle, psBuffer, unLength)) < 0)
	{
		return APP_FAIL;
	}
	return nLen;
}

/**
* @brief Write data to an opened file
* @param [in] nHandle   File handle
* @param [in] psBuffer  Data need to write
* @param [in] unLength	Data length
* @return 
* @li APP_FAIL Fail
* @li The real length of writing
*/
int PubWriteFile(int nHandle, const char * psBuffer, uint unLength)
{
	int nLen=0;

	if(psBuffer == NULL)
	{
		return APP_FAIL;
	}
	if((nLen=PubFsWrite(nHandle, psBuffer, unLength)) < 0)
	{
		return APP_FAIL;
	}
	return nLen;
}

/**
* @brief Close an opened file
* @param [in] pnFileHandle File handle
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubCloseFile (int *pnFileHandle)
{
	if (pnFileHandle == NULL)
	{
		return APP_FAIL;
	}

	if(PubFsClose(*pnFileHandle) != NAPI_OK)
	{
		return APP_FAIL;
	}
	*pnFileHandle = -1;
	return APP_SUCC;
}

/**
* @brief Delete record file created by 'PubCreatRecFile' including other extra index files.
* @param [in] pszFileName File name
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubDeReclFile (const char *pszFileName)
{
	int			nFp;
	char		szRecFileName[128] = {0};
	STRECFILE	stRecFileHead;

	if (pszFileName == NULL)
	{
		return APP_FAIL;
	}

	memset(szRecFileName, 0, sizeof(szRecFileName));
	memset( &stRecFileHead, 0, sizeof(stRecFileHead) ) ;
	if(PubFsExist(pszFileName) == NAPI_OK)
	{
		if ((nFp = PubFsOpen(pszFileName, "r")) == NAPI_ERR)
		{
			return APP_FAIL ;
		}
		if (ProReadRecFileHeader( nFp, &stRecFileHead) != APP_SUCC)
		{
			PubFsClose( nFp );
			return APP_FAIL;
		}
		PubFsClose( nFp );
		if(PubFsDel(pszFileName)!=NAPI_OK)
		{
			return APP_FAIL;
		}
		if( FILE_CREATEINDEX == stRecFileHead.cIsIndex)
		{
			strcpy (szRecFileName, pszFileName);
			strcat (szRecFileName, ".idx");
			if(PubFsDel(szRecFileName) != NAPI_OK)
			{
				return APP_FAIL;
			}
		}
		return APP_SUCC;
	}
	return APP_FAIL;
}

/**
* @brief Get record number of record file created by 'PubCreatRecFile'.
* @param [in] pszFileName File name
* @param [out] pnRecSum  Number of record 
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubGetRecSum (const char *pszFileName, int *pnRecSum)
{
	int nFd;
	long lSum;
	STRECFILE stRecFile;

	if(pszFileName == NULL || pnRecSum == NULL)
	{
		return APP_FAIL;
	}

	if((nFd = PubFsOpen(pszFileName, "r")) == NAPI_ERR)
	{
		return APP_FAIL;
	}
	if((lSum = ProFilelength(nFd)) < 0)
	{
		PubFsClose(nFd);
		return APP_FAIL;
	}

	if(ProReadRecFileHeader(nFd, &stRecFile) != APP_SUCC)
	{
		PubFsClose(nFd);
		return APP_FAIL;
	}
	PubFsClose(nFd);

	*pnRecSum = (int)(lSum - FILEHEADER_LEN)/stRecFile.unMaxOneRecLen;
	return APP_SUCC;
}

/**
* @brief Add a record to a file created by 'PubCreatRecFile'
* @param [in] pszFileName File name
* @param [in] pszRec Record buffer
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubAddRec (const char *pszFileName, const char *pszRec)
{
	int nFd, nRet, nRecNo;
	long lFilelength;
	STRECFILE stHeader;     /**/
	char szBuf[RECMAXLEN];

	if(pszFileName == NULL || pszRec == NULL)
	{
		return APP_FAIL;
	}

	memset ((char *)&stHeader, 0, sizeof (STRECFILE));
	if(PubOpenFile(pszFileName, "w", &nFd) != APP_SUCC)
	{
		return APP_FAIL;
	}
	if (ProReadRecFileHeader(nFd, &stHeader) != APP_SUCC)
	{
		PubFsClose(nFd);
		return APP_FAIL;
	}

	//
	lFilelength = ProFilelength(nFd);
	nRecNo = (int)(lFilelength - FILEHEADER_LEN) / stHeader.unMaxOneRecLen;
	if(ProUpdateIndex(stHeader, nRecNo+1, pszRec) != APP_SUCC)
	{
		PubFsClose(nFd);
		return APP_FAIL;
	}
	if (PubFsSeek (nFd, 0L, SEEK_END) != NAPI_OK)
	{
		PubFsClose(nFd);
		return APP_FAIL;
	}
	memset (szBuf, 0, sizeof (szBuf));
	memcpy (szBuf, pszRec, stHeader.unMaxOneRecLen);
	nRet=PubFsWrite (nFd, szBuf, stHeader.unMaxOneRecLen);
	PubFsClose(nFd);
	if (nRet != stHeader.unMaxOneRecLen)
	{
		return APP_FAIL;
	}	
	return APP_SUCC;
}

/**
* @brief Read a specified record from an opened file created by 'PubCreatRecFile'
* @param [in] nFileHandle File handle
* @param [in] nRecNo      Index
* @param [out] psRec      Record buffer
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubReadRec (const int nFileHandle, const int nRecNo, char *psRec)
{
	STRECFILE stHeader;     /**/
	char sBuf[RECMAXLEN];

	if(psRec == NULL || nRecNo <= 0)
	{
		return APP_FAIL;
	}
	/**/
	memset (&stHeader, 0, sizeof (STRECFILE));
	if (ProReadRecFileHeader (nFileHandle, &stHeader) != APP_SUCC)
	{
		return APP_FAIL;
	}

	/**/
	memset (sBuf, 0, sizeof (sBuf));
	if(ProReadRecFromFile(nFileHandle, nRecNo, stHeader, sBuf) != APP_SUCC)
	{
		return APP_FAIL;
	}
	memcpy (psRec, sBuf, stHeader.unMaxOneRecLen);
	return APP_SUCC;
}

/**
* @brief Update a specified record to a file created by 'PubCreatRecFile'
* @param [in] pszFileName File name
* @param [in] nRecNo      Index
* @param [in] psRec       Record buffer
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubUpdateRec (const char *pszFileName, const int nRecNo, const char *psRec)
{
	int nFd, nRet;
	STRECFILE stHeader;     /**/
	long lOffset;
	char sBuf[RECMAXLEN];
	char sOldbuf[RECMAXLEN];

	if(pszFileName == NULL || psRec == NULL || nRecNo <= 0)
	{
		return APP_FAIL;
	}

	if(PubOpenFile(pszFileName, "w", &nFd) != APP_SUCC)
	{
		return APP_FAIL;
	}
	/**/
	memset (&stHeader, 0, sizeof (STRECFILE));
	nRet = ProReadRecFileHeader (nFd, &stHeader);
	if (nRet != APP_SUCC)
	{
		PubFsClose(nFd);
		return APP_FAIL;
	}

	/**/
	memset (sBuf, 0, sizeof (sBuf));
	if(ProReadRecFromFile (nFd, nRecNo, stHeader, sBuf) != APP_SUCC)
	{
		PubFsClose(nFd);
		return APP_FAIL;
	}

	/**/
	memcpy (sOldbuf, sBuf, sizeof (sOldbuf));

	/**/
	if(ProUpdateIndex (stHeader, nRecNo, psRec) != APP_SUCC)
	{
		PubFsClose(nFd);
		return APP_FAIL;
	}

	lOffset = (long)FILEHEADER_LEN + ((long)nRecNo - 1) * stHeader.unMaxOneRecLen;
	nRet = PubFsSeek (nFd, lOffset, SEEK_SET);
	if (nRet != NAPI_OK)
	{/**/
		ProUpdateIndex (stHeader, nRecNo, sOldbuf);
		PubFsClose(nFd);
		return APP_FAIL;
	}

	/**/
	memset (sBuf, 0, sizeof(sBuf));
	memcpy (sBuf, psRec, stHeader.unMaxOneRecLen);
	nRet = PubFsWrite (nFd, sBuf, stHeader.unMaxOneRecLen);
	PubFsClose(nFd);
	if(nRet != stHeader.unMaxOneRecLen)
	{/**/
		ProUpdateIndex (stHeader, nRecNo, sOldbuf);
		return APP_FAIL;
	}
	return APP_SUCC;
}

/**
* @brief Update a specified record to an opened file created by 'PubCreatRecFile'
* @param [in] nFileHandle File handle
* @param [in] nRecNo      Index
* @param [in] psRec       Record buffer
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubUpdateRecByHandle (int nFileHandle, const int nRecNo, const char *psRec)
{
	long lOffset;
	STRECFILE stHeader;
	char sBuf[RECMAXLEN];
	char sOldbuf[RECMAXLEN];

	if (psRec == NULL || nRecNo <= 0)
	{
		return APP_FAIL;
	}
	/**/
	memset (&stHeader, 0, sizeof (STRECFILE));
	if (ProReadRecFileHeader (nFileHandle, &stHeader) != APP_SUCC)
	{
		return APP_FAIL;
	}

	/**/
	memset (sBuf, 0, sizeof (sBuf));
	if(ProReadRecFromFile (nFileHandle, nRecNo, stHeader, sBuf) != APP_SUCC)
	{
		return APP_FAIL;
	}

	/**/
	memcpy (sOldbuf, sBuf, sizeof (sOldbuf));

	/**/
	if(ProUpdateIndex (stHeader, nRecNo, psRec) != APP_SUCC)
	{
		return APP_FAIL;
	}

	/**/
	lOffset = (long)FILEHEADER_LEN + ((long)nRecNo - 1) * stHeader.unMaxOneRecLen;
	if (PubFsSeek (nFileHandle, lOffset, SEEK_SET) != NAPI_OK)
	{
		/**/
		ProUpdateIndex (stHeader, nRecNo, sOldbuf);
		return APP_FAIL;
	}

	/**/
	memset (sBuf, 0, sizeof(sBuf));
	memcpy (sBuf, psRec, stHeader.unMaxOneRecLen);
	if(PubFsWrite (nFileHandle, sBuf, stHeader.unMaxOneRecLen) != stHeader.unMaxOneRecLen)
	{
		/**/
		ProUpdateIndex (stHeader, nRecNo, sOldbuf);
		return APP_FAIL;
	}
	return APP_SUCC;
}

/**
* @brief Read a specified record from a file created by 'PubCreatRecFile'
* @param [in] pszFileName File name
* @param [in] nRecNo      Record number
* @param [out] psRec      Record buffer
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubReadOneRec(const char *pszFileName, const int nRecNo, char *psRec)
{
	int nFd, nRet;
	STRECFILE stHeader;
	char sBuf[RECMAXLEN];

	if (pszFileName == NULL || psRec == NULL || nRecNo <= 0)
	{
		return APP_FAIL;
	}
	if(PubOpenFile(pszFileName, "r", &nFd) != APP_SUCC)
	{
		return APP_FAIL;
	}
	/**/
	memset (&stHeader, 0, sizeof (STRECFILE));
	if (ProReadRecFileHeader (nFd, &stHeader) != APP_SUCC)
	{
		PubFsClose(nFd);
		return APP_FAIL;
	}

	/**/
	memset (sBuf, 0, sizeof (sBuf));
	nRet = ProReadRecFromFile (nFd, nRecNo, stHeader, sBuf);
	PubFsClose(nFd);
	if (nRet != APP_SUCC)
	{
		return APP_FAIL;
	}
	memcpy(psRec, sBuf, stHeader.unMaxOneRecLen);
	return APP_SUCC;
}

/**
* @brief Find a record from a file created by 'PubCreatRecFile' with 'FILE_CREATEINDEX' mode.
* @details If 'psIndexStr1' and 'psIndexStr2' are both assigned, needs to meet these two condition, otherwise one of them is enough.
* @param [in] pszFileName File name
* @param [in] psIndexStr1 Index string1
* @param [in] psIndexStr2 Index string2
* @param [in] nBeginRecNo From which record to start finding.
* @param [out] psRec Record buffer
* @param [out] pnRecNo Record number
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubFindRec(const char *pszFileName, const char *psIndexStr1,
			   const char *psIndexStr2,const int nBeginRecNo, char *psRec, int *pnRecNo)
{
	int nFd, nRet, nRecNo;
	STRECFILE stHeader;
	char sBuf[RECMAXLEN];

	if (pszFileName == NULL || psRec == NULL || pnRecNo == NULL)
	{
		return APP_FAIL;
	}
	/**/
	if(PubOpenFile(pszFileName, "r", &nFd) != APP_SUCC)
	{
		return APP_FAIL;
	}

    /**/
	memset (&stHeader, 0, sizeof (STRECFILE));
	if(ProReadRecFileHeader(nFd, &stHeader) != APP_SUCC)
	{
		PubFsClose(nFd);
		return APP_FAIL;
	}

	/**/
	nRecNo = ProReadIndex (stHeader, psIndexStr1, psIndexStr2,nBeginRecNo);
	if(nRecNo <= 0)
	{
		PubFsClose(nFd);
		return APP_FAIL;
	}

	/**/
	nRet = ProReadRecFromFile (nFd, nRecNo, stHeader, sBuf);
	PubFsClose(nFd);
	memcpy (psRec, sBuf, stHeader.unMaxOneRecLen);
	if (nRet == APP_SUCC)
	{
		*pnRecNo = nRecNo;
		return APP_SUCC;
	}
	return APP_FAIL;
}

/**
* @brief Delete a record from a file created by 'PubCreatRecFile' with 'FILE_CREATEINDEX' mode.
* @details If 'psIndexStr1' and 'psIndexStr2' are both assigned, needs to meet these two condition, otherwise one of them is enough.
* @param [in] pszFileName File name
* @param [in] psIndexStr1 Index string1
* @param [in] psIndexStr2 Index string2
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubDelRec(const char *pszFileName, const char *psIndexStr1,
			   const char *psIndexStr2)
{
	int i, nFd, nRecNo, nSumRecNum;
	long lRecFILELen;
	STRECFILE stHeader;
	char sBuf[RECMAXLEN];

	if (pszFileName == NULL || (psIndexStr1 == NULL && psIndexStr2 == NULL))
	{
		return APP_FAIL;
	}
	if(PubFindRec(pszFileName, psIndexStr1, psIndexStr2, 1,  sBuf, &nRecNo) != APP_SUCC)
	{
		return APP_FAIL;
	}

	/**/
	if(PubOpenFile(pszFileName, "r", &nFd) != APP_SUCC)
	{
		return APP_FAIL;
	}

		/**/
	memset (&stHeader, 0, sizeof (STRECFILE));
	if(ProReadRecFileHeader(nFd, &stHeader) != APP_SUCC)
	{
		PubFsClose(nFd);
		return APP_FAIL;
	}
	PubFsClose(nFd);

	if(ProDelIndex(stHeader, nRecNo) != APP_SUCC)
	{
		return APP_FAIL;
	}

	/**/
	if(PubOpenFile(pszFileName, "w", &nFd) != APP_SUCC)
	{
		return APP_FAIL;
	}

	lRecFILELen = ProFilelength(nFd);
	nSumRecNum = (int)(lRecFILELen - FILEHEADER_LEN)/stHeader.unMaxOneRecLen;
	if(nRecNo >= nSumRecNum)	/**/
	{
		PubFsClose(nFd);
		PubFsTruncate(pszFileName,lRecFILELen-stHeader.unMaxOneRecLen);
		return APP_SUCC;
	}
	PubFsSeek (nFd, ((long)nRecNo * stHeader.unMaxOneRecLen+(long)FILEHEADER_LEN), SEEK_SET);	/**/

	for(i=nRecNo; i<(nSumRecNum); i++)
	{
		if (PubFsRead(nFd, sBuf, stHeader.unMaxOneRecLen) != stHeader.unMaxOneRecLen)
		{
			PubFsClose(nFd);
			return APP_FAIL;
		}
		PubFsSeek (nFd, 0 -(2 * (long)stHeader.unMaxOneRecLen), SEEK_CUR);
		if (PubFsWrite (nFd, sBuf, stHeader.unMaxOneRecLen) != stHeader.unMaxOneRecLen)
		{
			PubFsClose(nFd);
			return APP_FAIL;
		}
		PubFsSeek (nFd, stHeader.unMaxOneRecLen, SEEK_CUR);	/**/
	}
	PubFsClose (nFd);
	PubFsTruncate(pszFileName, lRecFILELen-stHeader.unMaxOneRecLen);
	return APP_SUCC;
}

/**
* @brief Write error info to log file
* @param [in] nErrorCode    		error code
* @param [in] pszLogFileName      	log file name
* @param [in] pszMoudleName      	module name
* @param [in] nLine      			__LINE__ of calling
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubErrLog (const int nErrorCode, const char *pszLogFileName,
			   char *pszMoudleName, int nLine )
{
	int nFd;
	char szBuf[RECMAXLEN];

	if(pszLogFileName ==  NULL || pszMoudleName == NULL)
	{
		return APP_FAIL;
	}

	if((nFd = PubFsOpen(pszLogFileName, "w")) < 0)
	{
		return APP_FAIL;
	}

	if(PubFsSeek(nFd, 0L, SEEK_END) != NAPI_OK)
	{
		PubFsClose(nFd);
		return APP_FAIL;

	}

	memset(szBuf, 0, sizeof(szBuf));
	sprintf(szBuf, "%d %s %d \x0d\x0a", nErrorCode, pszMoudleName, nLine);
	if(PubFsWrite (nFd, szBuf, strlen(szBuf)) != strlen(szBuf))
	{
		PubFsClose(nFd);
		return APP_FAIL;
	}
	PubFsClose(nFd);
	return APP_SUCC;
}

/**
* @brief Get value from ini file with an appointed ID
* @param [in] pnHandle    	
* @param [in] nPosition     	
* @param [in] psID      	
* @param [out] psParamValue    
* @param [out] pnParamLen      			
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubFindParam (const int pnHandle, const int nPosition,
				  const char *psID, char *psParamValue, int *pnParamLen)
{
	int i;
	long lSeek;
	unsigned int unParamSum=0;		/**/
	unsigned int unLableOff;		/**/
	unsigned int unLableLen;		/**/
	unsigned int unValueOff;		/**/
	unsigned int unValueLen;		/**/
	unsigned char   uszParamNum[3];
	unsigned char uszTempBuffer[3];
	char szBuf[1024];

	if(psID ==  NULL || psParamValue == NULL || pnParamLen == NULL)
	{
		return APP_FAIL;
	}
	if(PubFsSeek (pnHandle, (long)nPosition, SEEK_SET) != NAPI_OK)	/**/
	{
        return APP_FAIL;
	}

	/**/
	memset(uszTempBuffer, 0, sizeof(uszTempBuffer));
	if(PubFsRead(pnHandle, (char *)uszTempBuffer, 2) != 2)
	{
		return APP_FAIL;
	}

	memset(uszParamNum, 0, sizeof(uszParamNum));
	uszParamNum[0] = uszTempBuffer[1];
	uszParamNum[1] = uszTempBuffer[0];
	PubC2ToInt( &unParamSum, uszParamNum );
	for(i = 0; i < unParamSum; i++)
	{
		lSeek = ((long)i << 3) + nPosition + 2;
		if(PubFsSeek(pnHandle, lSeek, SEEK_SET) != NAPI_OK)
		{
			return APP_FAIL;
		}

		/**/
		memset (uszTempBuffer, 0, sizeof (uszTempBuffer));
		if(PubFsRead(pnHandle, (char *)uszTempBuffer, 2) != 2)
		{
			return APP_FAIL;
		}

		memset(uszParamNum, 0, sizeof(uszParamNum));
		uszParamNum[0] = uszTempBuffer[1];
		uszParamNum[1] = uszTempBuffer[0];
		PubC2ToInt( &unLableOff, uszParamNum );

		/**/
		memset (uszTempBuffer, 0, sizeof (uszTempBuffer));
		if(PubFsRead(pnHandle, (char *)uszTempBuffer, 2) != 2)
		{
			return APP_FAIL;
		}

		memset(uszParamNum, 0, sizeof(uszParamNum));
		uszParamNum[0] = uszTempBuffer[1];
		uszParamNum[1] = uszTempBuffer[0];
		PubC2ToInt( &unLableLen, uszParamNum );
		/**/
		memset (uszTempBuffer, 0, sizeof (uszTempBuffer));
		if(PubFsRead(pnHandle, (char *)uszTempBuffer, 2) != 2)
		{
			return APP_FAIL;
		}

		memset(uszParamNum, 0, sizeof(uszParamNum));
		uszParamNum[0] = uszTempBuffer[1];
		uszParamNum[1] = uszTempBuffer[0];
		PubC2ToInt( &unValueOff, uszParamNum ) ;
		/**/
		memset (uszTempBuffer, 0, sizeof (uszTempBuffer));
		if(PubFsRead(pnHandle, (char *)uszTempBuffer, 2) != 2)
		{
			return APP_FAIL;
		}

		memset(uszParamNum, 0, sizeof(uszParamNum));
		uszParamNum[0] = uszTempBuffer[1];
		uszParamNum[1] = uszTempBuffer[0];
		PubC2ToInt( &unValueLen, uszParamNum ) ;
		/**/
		if(PubFsSeek(pnHandle, unLableOff, SEEK_SET) != NAPI_OK)
		{
			return APP_FAIL;
		}

		memset(szBuf, 0, sizeof(szBuf));
		if(PubFsRead(pnHandle, szBuf, unLableLen) != unLableLen)
		{
			return APP_FAIL;
		}
		if(strcmp(szBuf, psID) == 0)	/**/
		{
			/**/
			if(PubFsSeek(pnHandle, unValueOff, SEEK_SET) != NAPI_OK)
			{
				return APP_FAIL;
			}
			memset(szBuf, 0, sizeof(szBuf));
			if(PubFsRead(pnHandle, szBuf, unValueLen) < 0)
			{
				return APP_FAIL;
			}

			memcpy(psParamValue, szBuf, unValueLen);
			*pnParamLen = unValueLen;
			return APP_SUCC;
		}
	}
	return APP_FAIL;
}

/**
* @brief Delete file
* @param [in] pszFileName File name
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubDelFile (const char *pszFileName)
{
	if(pszFileName ==  NULL)
	{
		return APP_FAIL;
	}
	if(PubFsDel(pszFileName) != NAPI_OK)
	{
		return APP_FAIL;
	}
	return APP_SUCC;
}

/**
* @brief Get one character from a file
* @param [in] nFp 		file identifier
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
static int ProFGetC(int nFp)
{
	char ch;

	if(PubFsRead(nFp, (char *)&ch, 1) != 1)
	{
		return (-1);
	}
	else
	{
		return ch;
	}
}

/**
* @brief Get appointed length of string from a file
* @param [in] nFp 		file identifier
* @param [in] nMaxLen 	length that want to read
* @param [out] pszBuf 	data read from file
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
static char * ProFGets(char *pszBuf, int nMaxLen, int nFp)
{
	char *p;
	int nC;

	if(pszBuf ==  NULL || nMaxLen <= 0)
	{
		return NULL;
	}

	p = pszBuf;
	while(--nMaxLen)
	{
		if((nC = ProFGetC(nFp)) == (-1))
		{
			break;
		}
		if ((*p++ = nC) == '\n')
		{
			break;
		}
	}

	if (p > pszBuf)
	{
		*p = 0;
		return pszBuf;
	}
	return NULL;
}

/**
* @brief Read one line from a file
* @param [in] nFp 		file indentifier
* @param [in] nLen 		length that want to read
* @param [out] pszLine 	data read from file
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
 static char * ProReadline(char* pszLine, int nLen, int nFp)
 {
	static char* psRs;

	if(pszLine ==  NULL)
	{
		return NULL;
	}
  	psRs = ProFGets(pszLine, nLen, nFp);
	if(strlen(pszLine) >= 1 )
	{
		pszLine[strlen(pszLine) - 1] = pszLine[strlen(pszLine)]; 	/*0x0a*/
		if(pszLine[strlen(pszLine)-1] == 0x0d)					/*0x0d*/
		{
			pszLine[strlen(pszLine) - 1] = pszLine[strlen(pszLine)];
		}
	}
  	return psRs;
 }

/**
* @brief Check whether there is a newline character
* @param [in] pszLine 	source string
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
static int ProIsjoint(const char* pszLine)
{
	unsigned int nI;
	unsigned int nSign = 0;

	if(pszLine ==  NULL)
	{
		return APP_FAIL;
	}
	for(nI = 0; nI < strlen(pszLine); nI++)
	{
		if(pszLine[nI] == ' ' || pszLine[nI] == '\t' || pszLine[nI] == '\\')
		{
			if(pszLine[nI] == ' ' || pszLine[nI] == '\t' )
			{
				if(nSign < 1 )
				{
					nSign++;
				}
			}
			else
			{
				if(nSign > 0 )
				{
					nSign++;
				}
				else
				{
					return 0;
				}
			}
		}
		else
		{
			return 0;
		}
	}

	if(nSign == 2)
	{
		return 1;
	}

	return 0;
}

/**
* @brief Check whether it is a remark
* @param [in] pszLine 	source string
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
static int ProIsremark(const char* pszLine)
{
	int i;

	if(pszLine ==  NULL)
	{
		return APP_FAIL;
	}
	for(i = 0; i < strlen(pszLine); i++)
	{
		if(Proisgraph(pszLine[i]))
		{
			if(pszLine[i] == '#')
			{
				return 1;
			}
			else
			{
				return 0;
			}
		}
	}
	return 1;
}

/**
* @brief Get string between "[" and "]"
* @param 
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
static int ProGetSection(const char* pszLine, char* pszSection, unsigned int nSeclen)
{
	unsigned int unStart;
	unsigned int unMid;
	unsigned int unEnd;

	if(pszLine ==  NULL || pszSection == NULL)
	{
		return APP_FAIL;
	}
	for(unStart = 0; unStart < strlen(pszLine); unStart++)
	{
		if(Proisgraph(pszLine[unStart]))
		{
			if(pszLine[unStart] != '[')
			{
				return 0;
			}
			else
			{
				break;
			}
		}
	}

	if(unStart >= strlen(pszLine)) // 
	{
		return 0;
	}

	for(unEnd = strlen(pszLine); unEnd > 1; unEnd --)
	{
		if(Proisgraph(pszLine[unEnd]))
		{
			if(pszLine[unEnd] != ']')
			{
				return 0;
			}
			else
			{
				break;
			}
		}
	}

	if(unEnd <= 1) // 
	{
		return 0;
	}

	if(unEnd - unStart < 2) // 
	{
		return 0;
	}

	for(unMid = unStart + 1; unMid < unEnd; unMid++)
	{
		if(!Proisalnum(pszLine[unMid])) // 
		{
			return 0;
		}
	}

	if(nSeclen > unEnd - unStart - 1)
	{
		memcpy(pszSection, &pszLine[unStart + 1], unEnd - unStart - 1);
		pszSection[unEnd - unStart - 1] = '\0';
		return(unEnd - unStart - 1);
	}
	else
	{
		memcpy(pszSection, &pszLine[unStart + 1], nSeclen - 1);
		pszSection[nSeclen - 1] = '\0';
		return(nSeclen -1);
	}
}

/**
* @brief Get string after "="
* @param 
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
static int ProGetKeyName(const char* pszLine, char* pszKeyName, unsigned int unKeyLen)
{
	unsigned int unStart;
	unsigned int unMid;
	unsigned int unEnd;

	if(pszLine ==  NULL || pszKeyName == NULL)
	{
		return APP_FAIL;
	}
	for(unStart = 0; unStart < strlen(pszLine); unStart++) // 
	{
		if (Proisgraph(pszLine[unStart]))
		{
			if(!Proisalnum(pszLine[unStart]))
			{
				return 0;
			}
			else
			{
				break;
			}
		}
	}

	if(unStart >= strlen(pszLine)) // 
	{
		return 0;
	}

	for(unEnd = unStart; pszLine[unEnd] != '=' && unEnd < strlen(pszLine); unEnd++) // 
	{
		if (!Proisalnum(pszLine[unEnd]) && pszLine[unEnd] != ' ' && pszLine[unEnd] != '\t')
		{
			return 0;
		}
	}

	if(unEnd >= strlen(pszLine)) // 
	{
		return 0;
	}

	for(unEnd = unEnd- 1; !Proisalnum(pszLine[unEnd]); unEnd--) // 
	{
		;
	}
	for(unMid = unStart; unMid <= unEnd; unMid++)
	{
		if (pszLine[unMid] == ' ' || pszLine[unMid] == '\t' ) // 
		{
			return 0;
		}
	}

	if(unKeyLen > unEnd - unStart + 1)
	{
		memcpy(pszKeyName, &pszLine[unStart], unEnd - unStart + 1);
		pszKeyName[unEnd - unStart + 1] = '\0';
		return (unEnd - unStart + 1);
	}
	else
	{
		memcpy(pszKeyName, &pszLine[unStart], unKeyLen - 1);
		pszKeyName[unKeyLen - 1] = '\0';
		return (unKeyLen -1);
	}
}

/**
* @brief Get key value
* @param 
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
static int ProGetKeyValue(const char* pszLine, char* pszKeyValue, unsigned int unValen)
{
	unsigned int unStart;
	unsigned int unEnd;
	int nLen;

	if(pszLine ==  NULL || pszKeyValue == NULL)
	{
		return APP_FAIL;
	}
	for(unStart = 0; pszLine[unStart] != '=' && unStart < strlen(pszLine); unStart++) // =
	{
		;
	}

	for(unStart = unStart + 1;
		!Proisgraph(pszLine[unStart]) && unStart < strlen(pszLine);
		unStart++) // =
	{
		;
	}

	if( unStart >= strlen(pszLine)) // 
	{
		return 0;
	}
//VAL
	for(unEnd = unStart;
		pszLine[unEnd] != '\t' && unEnd < strlen(pszLine);
		unEnd++) // 
	{
		;
	}

	if(unValen > unEnd - unStart)
	{
		memcpy(pszKeyValue, &pszLine[unStart], unEnd - unStart);
		pszKeyValue[unEnd - unStart] = '\0';
		nLen = unEnd - unStart;
	}
	else
	{
		memcpy(pszKeyValue, &pszLine[unStart], unValen - 1);
		pszKeyValue[unValen - 1] = '\0';
		nLen = unValen -1;
	}

	if(unEnd >= strlen(pszLine))
	{
		return nLen;
	}
	else
	{
		if(ProIsjoint(&pszLine[unEnd]))
		{
			return -nLen;
		}
		return nLen;
	}
}

/**
* @brief Get next key value
* @param 
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
static int ProGetKeyAdd(const char* pszLine, char* pszKeyValue, unsigned int unValen)
{
	unsigned int unStart;
	unsigned int unEnd;
	int nLen;

	if(pszLine ==  NULL || pszKeyValue == NULL)
	{
		return APP_FAIL;
	}
	for(unStart = 0; !Proisgraph(pszLine[unStart]) && unStart < strlen(pszLine); unStart++)
	{
		;
	}
	if(unStart >= strlen(pszLine))
	{
		return 0;
	}
	for(unEnd = unStart;
		pszLine[unEnd] != ' ' && pszLine[unEnd] != '\t' && unEnd < strlen(pszLine);
		unEnd++)
	{
		;
	}

	if(unValen > unEnd - unStart)
	{
		memcpy(pszKeyValue, &pszLine[unStart], unEnd - unStart);
		pszKeyValue[unEnd - unStart] = '\0';
		nLen = unEnd - unStart;
	}
	else
	{
		memcpy(pszKeyValue, &pszLine[unStart], unValen - 1);
		pszKeyValue[unValen - 1] = '\0';
		nLen = unValen -1;
	}

	if(unEnd >= strlen(pszLine))
	{
		return nLen;
	}
	else
	{
		if(ProIsjoint(&pszLine[unEnd]))
		{
			return -nLen;
		}
		return nLen;
	}
}

/**
* @brief Judge whether it has reach to the end of file
* @param [in] nFd		file indentifier
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
static int ProFoef(int nFd)
{
	long lFileLen = 0;
	ulong lCurOff = 0;

	if((lFileLen = ProFilelength(nFd)) < 0)
	{
		return (-1);
	}
	PubFsTell(nFd, &lCurOff);
	if(lCurOff  < 0)
	{
		return (-1);
	}

	if(lCurOff >= lFileLen)	/**/
	{
		return (-1);
	}

	return 0;
}

/**
* @brief Get appointed string from ini file
* @param [in] nFd		file indentifier
* @param [in] psSeg		Section name
* @param [in] psItem	tag name
* @param [in] nMaxStrLen	Max length
* @param [out] psValueStr	Output buffer
* @param [out] pnValueLen	Output buffer length
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubGetINIItemStr (const int nHandle, const char *psSeg, const char *psItem,
					  const int nMaxStrLen, char *psValueStr, int *pnValueLen)
{
	int nStep = 0;
	int nRs = 0;
	int nRa = 0;
	char szLine[LINE_LEN];
	char szSec[LINE_LEN];
	char szKen[LINE_LEN];
	char szKev[LINE_LEN];

	if(psSeg ==  NULL || psItem == NULL || psValueStr == NULL || pnValueLen == NULL || nMaxStrLen <= 0 )
	{
		return APP_FAIL;
	}
	memset(szLine, 0, sizeof(szLine));
	memset(szSec, 0, sizeof(szSec));
	memset(szKen, 0, sizeof(szKen));
	memset(szKev, 0, sizeof(szKev));

	PubFsSeek(nHandle, 0L, SEEK_SET);		/**/

	while(!ProFoef(nHandle))
	{
		if(ProReadline(szLine, 100, nHandle) == NULL)
		{
			return APP_FAIL;	/*   error   occur   in   reading   configuration   file   */
		}

		if(!ProIsremark(szLine))
		{
			if(nStep == 0)
			{
				if(ProGetSection(szLine, szSec, LINE_LEN))
				{
					if(strcmp(szSec, psSeg) == 0)
					{
						nStep = 1;
					}
				}
			}
			else if(nStep == 1)
			{
				if(ProGetKeyName(szLine, szKen, LINE_LEN))
				{
					if(strcmp(szKen, psItem) == 0)
					{
						nRs = ProGetKeyValue(szLine, szKev, LINE_LEN);
						if(nRs >= 0)
						{
							memset(psValueStr, 0, nMaxStrLen);
							strncpy(psValueStr, szKev, nMaxStrLen);
							*pnValueLen = strlen(psValueStr);
							return APP_SUCC;  /*  read configuration file successfully   */
						}
						else if(nRs < 0)
						{
							nStep = 2;
							nRs = abs(nRs);
						}
						else
						{
							return APP_FAIL;/* invalid  format  in  configuration  file  */
						}

					} // end if(strcmp(szKen, psItem) == 0)
				} // end if(ProGetKeyName(szLine, szKen, LINE_LEN))
				else
				{/*<Section*/
					//return APP_FAIL;
				}
			} // else if(nStep == 1)
			else
			{
				nRa = ProGetKeyAdd(szLine, szKev + nRs, LINE_LEN - nRs);
				if(nRa > 0)
				{
					strncpy(psValueStr, szKev, nMaxStrLen);
					*pnValueLen = strlen(psValueStr);
					return APP_SUCC;
				}
				else if( nRa < 0 )
				{
					nRs += abs(nRa);
				}
				else
				{
					return APP_FAIL;
				}
			} // end else
		}  // end if(!ProIsremark(szLine))
	} // end while
	return APP_FAIL;
}

/**
* @brief Get appointed integer value from ini file
* @param [in] nHandle		file indentifier
* @param [in] psSeg			section name
* @param [in] psItem		tag name
* @param [out] pnValue		Output integer value
* @return 
* @li	
*/
int PubGetINIItemInt (const int nHandle,const char *psSeg, const char *psItem, int *pnValue)
{
	int nValueLen;
	char szBuf[LINE_LEN];

	if(psSeg ==  NULL || psItem == NULL || pnValue == NULL)
	{
		return APP_FAIL;
	}
	memset(szBuf, 0, sizeof(szBuf));
	if(PubGetINIItemStr(nHandle, psSeg, psItem, LINE_LEN, szBuf, &nValueLen) != APP_SUCC)
	{
		return APP_FAIL;
	}

	if(nValueLen == 0)
	{
		//return APP_FAIL;
		return APP_SUCC;
	}
	*pnValue = atoi(szBuf);
	return APP_SUCC;
}

/**
* @brief Get the position of a charactor in a string
* @param [in] pszData     ---string
* @param [in] cSubChar    ---charactor
* @return 
* @li APP_FAIL  ---inexistence
* @li >=0   ---Position
* @author Liug
* @date 2012-7-23
*/
int PubPosChar(const char *pszData,char cSubChar)
{
	int i;
	
	if (pszData == NULL)
	{
		return APP_FAIL;
	}

	for (i=0; i<strlen(pszData); i++)
	{
		if (pszData[i] == cSubChar)
		{
			return i;
		}
	}
	return APP_FAIL;
}

/**
* @brief Get the position of a string in another string
* @param [in] pszData     ---Source string
* @param [in] pszSub      ---String need to search
* @return 
* @li APP_FAIL  ---inexistence
* @li >=0   ---Position
* @author Liug
* @date 2012-7-23
*/
int PubPosStr(const char *pszData,const char *pszSub)
{
	int i,nDataLen,nSubLen;
	
	if ((pszData == NULL)||(pszSub == NULL))
	{
		return APP_FAIL;
	}
	
	nDataLen = strlen(pszData);
	nSubLen = strlen(pszSub);
	for (i=0; i<nDataLen; i++)
	{
		if (strncmp(pszData+i, pszSub, nSubLen) == 0 )
		{
			return i;
		}
	}
	return APP_FAIL;
}

/*
* NEWLAND PAYMENT
* 8583 function module
* Author    Xusl
* Date    2006-09-10
*/
#include "isodef.h"

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef isprint
#define isprint(c) ((char)(c) >= 0x20 && (char)(c) < 0x7f)
#endif

#ifndef isdigit
#define isdigit(c) ((char)(c) >= 0x40 && (char)(c) <= 0x49)
#endif

/**
* @brief Padding string for a Field
* @param [in] pthis     		---ISO structure format
* @param [in] pfieldbuf      	---field buf need to pad
* @param [in] nFieldlen      	---field length after padding
* @param [in] cHead      		---whether there is a header character
* @param [out] poutdata      		---out put data
* @return 
* @li length after padding
* @author Xusl
* @date 2006-9-29
*/
static int ProAddPadChar(STISO_FIELD_DEF * pthis, const unsigned char *pfieldbuf,
						 int nFieldlen, unsigned char *poutdata, unsigned char cHead)
{
    int nLen,nCount = 0;

	if(pthis ==  NULL || pfieldbuf == NULL || poutdata == NULL)
	{
		return APP_FAIL;
	}
    if (pthis->lentype == LENGTH_TYPE_FIX)
	{
        nLen = pthis->nLen;
	}
    else
	{
        nLen = min(pthis->nLen, nFieldlen);
	}

    nLen += cHead;
    nFieldlen = min(nLen, nFieldlen);

    /* put C/D char */
	if (cHead == 1)
	{
		poutdata[nCount++] = pfieldbuf[0];
		--nLen;
		--nFieldlen;
	}

        /* add left cPadchar */
	if ((pthis->lentype == LENGTH_TYPE_FIX) && (pthis->align == ALIGN_TYPE_RIGHT))
	{
		memset(poutdata + nCount, pthis->cPadchar, nLen - nFieldlen);
		nCount += nLen - nFieldlen;
	}

    memcpy(poutdata + nCount, (char *) pfieldbuf + cHead,nFieldlen);
    nCount += nFieldlen;

        /* add right cPadchar */
    if ((pthis->lentype == LENGTH_TYPE_FIX) && (pthis->align == ALIGN_TYPE_LEFT))
    {
        memset(poutdata + nCount, pthis->cPadchar, nLen - nFieldlen);
        nCount += nLen - nFieldlen;
    }
    return nCount;
}

/**
* @brief Calculate the BCD length of ISO 8583 info
* @param [in] pthis     		---ISO structure format
* @param [in] nLen      		---field length
* @param [out] poutdata      	---length buffer after calculation
* @param [out] pcount      		---length of filed length
* @return 
* @li
* @author Xusl
* @date 2006-9-29
*/
void PubFLBLengthSet(STISO_FIELD_DEF *pthis, int nLen, unsigned char *poutdata, int *pcount)
{
    unsigned char *pWt = poutdata;

	if(pthis ==  NULL || poutdata == NULL || pcount == NULL)
	{
		return;
	}
    if (pthis->lentype == LENGTH_TYPE_LLVAR)
	{
        (*pWt++) = (unsigned char)(nLen % 10) + (nLen / 10) * 16;
	}
    else
	{
		if (pthis->lentype == LENGTH_TYPE_LLLVAR)
		{
			(*pWt++) = (unsigned char)(nLen / 100);
			(*pWt++) = (unsigned char)((nLen % 100) / 10) * 16 + (nLen % 100) % 10;
		}
	}

    *pcount = (pWt - poutdata);
    return;
}

/**
* @brief Get the length buffer of one field
* @param [in] pthis     		---ISO structure format
* @param [in] pindata      		---field length
* @param [out] plen      		---length buffer 
* @param [out] pcount      		---length of filed length
* @return 
* @li
* @author Xusl
* @date 2006-9-29
*/
void PubFLBLengthGet(STISO_FIELD_DEF * pthis, unsigned char *pindata, int *plen, int *pcount)
{
    unsigned char *pRt = pindata;

	if(pthis ==  NULL || plen == NULL || pcount == NULL)
	{
		return;
	}
    if (pthis->lentype == LENGTH_TYPE_FIX)
	{
        *plen = pthis->nLen;
	}
    else
    {
        *plen = (*pRt) - ((*pRt) >> 4) * 6;
        pRt++;
        if (pthis->lentype == LENGTH_TYPE_LLLVAR)
        {
            *plen = (*plen) * 100 + ((*pRt) - ((*pRt) >> 4) * 6);
            pRt++;
        }
        if (*plen > pthis->nLen)
		{
            *plen = pthis->nLen;
		}
    }

    *pcount = (pRt - pindata);
    return;
}

/**
* @brief Calculate the length of message(Ascii) which format is ISO 8583
* @param [in] pthis			
* @param [in] len		
* @param [out] poutdata		
* @param [out] pcount		
* @return 
* @li void
*/
void PubFLALengthSet(STISO_FIELD_DEF *pthis, int nLen, unsigned char *poutdata, int *pcount)
{
    unsigned char *pWt = poutdata;
	if(pthis ==  NULL || poutdata == NULL || pcount == NULL)
	{
		return;
	}
    if (pthis->lentype == LENGTH_TYPE_LLVAR)
    {
        (*pWt++) = (unsigned char)(nLen / 10) + 0x30;
        (*pWt++) = (unsigned char)(nLen % 10) + 0x30;
    }
    else if (pthis->lentype == LENGTH_TYPE_LLLVAR)
    {
        (*pWt++) = (unsigned char)(nLen / 100) + 0x30;
        (*pWt++) = (unsigned char)((nLen % 100) / 10)  + 0x30;
        (*pWt++) = (unsigned char)(nLen % 10)  + 0x30;
    }

    *pcount = (pWt - poutdata);
    return;
}

/**
* @brief Calculate the length of field in message(Ascii) which format is ISO 8583.
* @param [in] pthis			
* @param [in] pindata	
* @param [out] plen		
* @param [out] pcount		
* @return 
* @li void
*/
void PubFLALengthGet(STISO_FIELD_DEF * pthis, unsigned char *pindata, int *plen, int *pcount)
{
    unsigned char *pRt = pindata;

	if(pthis ==  NULL || plen == NULL || pcount == NULL)
	{
		return;
	}
    if (pthis->lentype == LENGTH_TYPE_FIX)
	{
        *plen = pthis->nLen;
	}
    else
    {
        if (pthis->lentype == LENGTH_TYPE_LLLVAR)
		{
            *plen = ((*pRt++) - 0x30) * 100;
		}
		else
		{
			*plen = 0;
		}
        *plen += ((*pRt++) - 0x30) * 10;
        *plen += (*pRt++) - 0x30;
        if (*plen > pthis->nLen)
		{
            *plen = pthis->nLen;
		}
    }

    *pcount = (pRt - pindata);
    return;
}

/**
* @brief Calculate the length of message(Hex) which format is ISO 8583
* @param [in] pthis			
* @param [in] len			
* @param [out] poutdata		
* @param [out] pcount		
* @return 
* @li void
*/
void PubFLHLengthSet(STISO_FIELD_DEF *pthis, int nLen, unsigned char *poutdata, int *pcount)
{
    unsigned char *pWt = poutdata;

	if(pthis ==  NULL || poutdata == NULL || pcount == NULL)
	{
		return;
	}
    if (pthis->lentype == LENGTH_TYPE_LLVAR)
	{
        (*pWt++) = (unsigned char) nLen;
	}
    else
	{
		if (pthis->lentype == LENGTH_TYPE_LLLVAR)
		{
			(*pWt++) = (unsigned char) (nLen / 256);
			(*pWt++) = (unsigned char) (nLen % 256);
		}
	}

    *pcount = (pWt - poutdata);
    return;
}

/**
* @brief Calculate the length of field in message(Hex) which format is ISO 8583.
* @param [in] pthis		
* @param [in] pindata		
* @param [out] plen			
* @param [out] pcount	
* @return 
* @li void
*/
void PubFLHLengthGet(STISO_FIELD_DEF * pthis, unsigned char *pindata, int *plen, int *pcount)
{
    unsigned char *pRt = pindata;

	if(pthis ==  NULL || pcount == NULL)
	{
		return;
	}
    if (pthis->lentype == LENGTH_TYPE_FIX)
	{
        *plen = pthis->nLen;
	}
    else
    {
        if (pthis->lentype == LENGTH_TYPE_LLLVAR)
		{
            *plen = (*pRt++) * 256;
		}
		if (pthis->lentype == LENGTH_TYPE_LLVAR)
		{
            *plen = 0; /*init plen*/
        }
        *plen += (*pRt++); /* */
        if (*plen > pthis->nLen)
		{
            *plen = pthis->nLen;
		}
    }

    *pcount = (pRt - pindata);
    return;
}

/**
* @brief Pack data in ISO8583(Msg ID is Ascii)
* @param [in] pthis			
* @param [in] pfieldbuf		
* @param [in] fieldlen		
* @param [out] poutdata		
* @return 
* @li
*/
int PubFPAMsgIDPack(STISO_FIELD_DEF * pthis, const unsigned char *pfieldbuf, int nFieldlen,
					unsigned char *poutdata)
{
    int nLen = pthis->nLen;

	if(pthis ==  NULL || pfieldbuf == NULL || poutdata == NULL)
	{
		return APP_FAIL;
	}
    memcpy(poutdata, pfieldbuf, nLen);
    return nLen;
}

/**
* @brief Unpack data in ISO8583(Msg ID is Ascii)
* @param [in] pthis			
* @param [in] pindata	
* @param [out] pfieldbuf	
* @param [out] pfieldlen	
* @return 
* @li 
*/
int PubFPAMsgIDUnpack(STISO_FIELD_DEF * pthis, unsigned char *pfieldbuf, int *pfieldlen,
					  unsigned char *pindata)
{
    int nLen;

	if(pthis ==  NULL || pfieldbuf == NULL || pfieldlen == NULL || pindata == NULL)
	{
		return APP_FAIL;
	}
	nLen = pthis->nLen;
    memcpy(pfieldbuf, pindata, pthis->nLen);
    *pfieldlen = nLen;
    return nLen;
}

/**
* @brief Pack data in ISO8583(Msg ID is BCD)
* @param [in] pthis		
* @param [in] pfieldbuf		
* @param [in] fieldlen		
* @param [out] poutdata		
* @return 
* @li
*/
int PubFPBMsgIDpack(STISO_FIELD_DEF * pthis, const unsigned char *pfieldbuf, int nFieldlen,
					unsigned char *poutdata)
{
    int nLen;

	if(pthis ==  NULL || pfieldbuf == NULL || poutdata == NULL)
	{
		return APP_FAIL;
	}
	nLen = pthis->nLen >> 1;
	if (PubAscToHex( pfieldbuf, pthis->nLen, 0, poutdata ) != APP_SUCC)
	{
		return APP_FAIL;
	}
    return nLen;
}

/**
* @brief Unpack data in ISO8583(Msg ID is BCD)
* @param [in] pthis		
* @param [in] pindata		
* @param [out] pfieldbuf		
* @param [out] pfieldlen	
* @return 
* @li 
*/
int PubFPBMsgIDUnpack(STISO_FIELD_DEF * pthis, unsigned char *pfieldbuf, int *pfieldlen,
					  unsigned char *pindata)
{
    int nLen;

	if(pthis ==  NULL || pfieldbuf == NULL || pfieldlen == NULL || pindata == NULL)
	{
		return APP_FAIL;
	}
    nLen = pthis->nLen >> 1;
	if (PubHexToAsc(pindata, pthis->nLen, 0, pfieldbuf) != APP_SUCC)
	{
		return APP_FAIL ;
	}
    *pfieldlen = pthis->nLen;
    return nLen;
}

/**
* @brief Pack data in ISO8583(Bitmap in BCD)
* @param [in] pthis			
* @param [in] pfieldbuf		
* @param [in] fieldlen		
* @param [out] poutdata		
* @return 
* @li
*/
int PubFPBBitmapPack(STISO_FIELD_DEF * pthis, const unsigned char *pfieldbuf, int nFieldlen,
					 unsigned char *poutdata)
{
    int nBitmaplen;
    int nI, nJ;
    unsigned char sBitmap[16], ucBitmask;

	if(pthis ==  NULL || pfieldbuf == NULL || poutdata == NULL)
	{
		return APP_FAIL;
	}
    nBitmaplen = nFieldlen >> 3;
    memset(sBitmap, 0, sizeof(sBitmap));
    for (nI = 0; nI < nBitmaplen; nI++)
    {
        ucBitmask = 0x80;
        for (nJ = 0; nJ < 8; nJ++, ucBitmask >>= 1)
        {
            if (pfieldbuf[nI * 8 + nJ])
			{
                sBitmap[nI] |= ucBitmask;
			}
        }
    }

    memcpy(poutdata, sBitmap, nBitmaplen);
    return nBitmaplen;
}

/**
* @brief Unpack data in ISO8583(Bitmap in BCD)
* @param [in] pthis			
* @param [in] pindata		
* @param [out] pfieldbuf	
* @param [out] pfieldlen	
* @return 
* @li 
*/
int PubFPBBitmapUnPack(STISO_FIELD_DEF * pthis, unsigned char *pfieldbuf, int *pfieldlen,
					   unsigned char *pindata)
{
    int nI,nJ,nBitmaplen;
    unsigned char ucBitmask;

	if(pthis ==  NULL || pfieldbuf == NULL || pfieldlen == NULL || pindata == NULL)
	{
		return APP_FAIL;
	}
    if (pindata[0] & 0x80)
	{
        nBitmaplen = 16;
	}
    else
	{
        nBitmaplen = 8;
	}

	/*memset(pfieldbuf, 0, sizeof(pfieldbuf));*/
    for (nI = 0; nI < nBitmaplen; nI++)
    {
        ucBitmask = pindata[nI];
        for (nJ = 0; nJ < 8; nJ++, ucBitmask <<= 1)
		{
			pfieldbuf[(nI << 3) + nJ] = ((ucBitmask & 0x80) ? 1 : 0);
		}
    }

    *pfieldlen = (nBitmaplen << 3);
    return nBitmaplen;
}

/**
* @brief Pack data in ISO8583(Bitmap in Ascii)
* @param [in] pthis			
* @param [in] pfieldbuf		
* @param [in] fieldlen		
* @param [out] poutdata		
* @return 
* @li
*/
int PubFPABitmapPack(STISO_FIELD_DEF * pthis, const unsigned char *pfieldbuf, int nFieldlen,
					 unsigned char *poutdata)
{
    int nBitmaplen;
    unsigned char sBitmap[16];

	if(pthis ==  NULL || pfieldbuf == NULL || poutdata == NULL)
	{
		return APP_FAIL;
	}
    memset(sBitmap, 0, sizeof(sBitmap));
    nBitmaplen = PubFPBBitmapPack(pthis, pfieldbuf, nFieldlen, sBitmap);
    /*ProBcdToAsc(poutdata, sBitmap, nBitmaplen * 2, 0);*/
	//PubHexToAsc(poutdata, nBitmaplen*2, 0, sBitmap);
	PubHexToAsc(sBitmap, nBitmaplen*2, 0, poutdata);  ///,wwt 2011-4-11
    return nBitmaplen * 2;
}

/**
* @brief Unpack data in ISO8583(Bitmap in Ascii)
* @param [in] pthis			
* @param [in] pindata	
* @param [out] pfieldbuf
* @param [out] pfieldlen	
* @return 
* @li
*/
int PubFPABitmapUnPack(STISO_FIELD_DEF * pthis, unsigned char *pfieldbuf, int *pfieldlen,
					   unsigned char *pindata)
{
    int nBitmaplen;
    unsigned char sBitmap[16];

	if(pthis ==  NULL || pfieldbuf == NULL || pfieldlen == NULL || pindata == NULL)
	{
		return APP_FAIL;
	}
    /*ProAscToBcd(sBitmap, pindata, 16, 0);*/
	PubAscToHex(pindata, 16, 0, sBitmap);
    if (sBitmap[0] & 0x80)
	{
        nBitmaplen = 16;
	}
    else
	{
        nBitmaplen = 8;
	}
    /*ProAscToBcd(sBitmap, pindata, nBitmaplen * 2, 0);*/
	PubAscToHex(pindata, nBitmaplen*2, 0, sBitmap);
    PubFPBBitmapUnPack(pthis, pfieldbuf, pfieldlen, sBitmap);
    return nBitmaplen * 2;
	//return nBitmaplen;
}

/**
* @brief Pack data in ISO8583(string in ascii)
* @param [in] pthis		
* @param [in] pfieldbuf		
* @param [in] fieldlen	
* @param [out] poutdata	
* @return 
* @li 
*/
int PubFPACharPack(STISO_FIELD_DEF * pthis, const unsigned char *pfieldbuf,
				   int nFieldlen, unsigned char *poutdata)
{
    int nI, nLen, nCount;
    unsigned char sztmp[MAX_FIELD_LEN];

	if(pthis ==  NULL || pfieldbuf == NULL || poutdata == NULL)
	{
		return APP_FAIL;
	}
    for (nI = 0; nI < nFieldlen; nI++)
    {
        if (!isprint(pfieldbuf[nI]))
		{
            return APP_FAIL;
		}
    }

    nLen = ProAddPadChar(pthis, pfieldbuf, nFieldlen, sztmp, 0);
    stiso8583_fldcalc.pSet(pthis, nLen, poutdata, &nCount);

    memcpy(poutdata + nCount, (char *) sztmp, nLen);
    return nCount + nLen;
}

/**
* @brief Unpack data in ISO8583(string in ascii)
* @param [in] pthis		
* @param [in] pindata		
* @param [out] pfieldbuf	
* @param [out] pfieldlen	
* @return 
* @li 
*/
int PubFPACharUnPack(STISO_FIELD_DEF * pthis, unsigned char *pfieldbuf,
					 int *pfieldlen, unsigned char *pindata)
{
    int nI, nLen, nCount;

	if(pthis ==  NULL || pfieldbuf == NULL || pfieldlen == NULL || pindata == NULL)
	{
		return APP_FAIL;
	}
    stiso8583_fldcalc.pGet(pthis, pindata, &nLen, &nCount);

    memcpy(pfieldbuf, pindata + nCount, nLen);
    *pfieldlen = nLen;

    for (nI = 0; nI < *pfieldlen; nI++)
    {
        if (!isprint(pfieldbuf[nI]))
        {
            *pfieldlen = 0;
            return APP_FAIL;
        }
    }

    return nCount + nLen;
}

/**
* @brief Pack data in ISO8583(string in BCD)
* @param [in] pthis			
* @param [in] pfieldbuf		
* @param [in] fieldlen	
* @param [out] poutdata		
* @return 
* @li 
*/
int PubFPBCharPack(STISO_FIELD_DEF * pthis, const unsigned char *pfieldbuf,
				   int nFieldlen, unsigned char *poutdata)
{
    int nI, nLen, nCount;
    unsigned char sztmp[MAX_FIELD_LEN];

	if(pthis ==  NULL || pfieldbuf == NULL || poutdata == NULL)
	{
		return APP_FAIL;
	}
    for (nI = 0; nI < nFieldlen; nI++)
    {
        if (!isprint(pfieldbuf[nI]))
		{
            return APP_FAIL;
		}
    }

    nLen = ProAddPadChar(pthis, pfieldbuf, nFieldlen, sztmp, 0);
    stiso8583_fldcalc.pSet(pthis, nLen, poutdata, &nCount);

    /*  */
    if ((nLen & 1) && (pthis->align == ALIGN_TYPE_LEFT))
	{
        sztmp[nLen++] = pthis->cPadchar;
	}

    /*ProAscToBcd(poutdata + nCount, sztmp, nLen, (unsigned char) ((pthis->align == ALIGN_TYPE_LEFT) ? 0 : 1));*/
	PubAscToHex( sztmp, nLen, (unsigned char) ((pthis->align == ALIGN_TYPE_LEFT) ? 0 : 1),
				poutdata + nCount );
	return nCount + (++nLen >> 1);
}

/**
* @brief Unpack data in ISO8583(string in BCD)
* @param [in] pthis		
* @param [in] pindata		
* @param [out] pfieldbuf	
* @param [out] pfieldlen
* @return 
* @li 
*/
int PubFPBCharUnPack(STISO_FIELD_DEF * pthis, unsigned char *pfieldbuf,
					 int *pfieldlen, unsigned char *pindata)
{
    int nI, nLen, nCount;

	if(pthis ==  NULL || pfieldbuf == NULL || pfieldlen == NULL || pindata == NULL)
	{
		return APP_FAIL;
	}
    stiso8583_fldcalc.pGet(pthis, pindata, &nLen, &nCount);

    //ProBcdToAsc(pfieldbuf, pindata + nCount, nLen, (unsigned char) ((pthis->align == ALIGN_TYPE_LEFT) ? 0 : 1));
	PubHexToAsc(pindata + nCount, nLen,
				(unsigned char) ((pthis->align == ALIGN_TYPE_LEFT) ? 0 : 1),
				pfieldbuf);
	*pfieldlen = nLen;

    for (nI = 0; nI < *pfieldlen; nI++)
    {
        if (!isprint(pfieldbuf[nI]))
        {
            *pfieldlen = 0;
            return APP_FAIL;
        }
    }

    return nCount + (++nLen >> 1);
}

/**
* @brief Pack data in ISO8583(digit string in Ascii)
* @param [in] pthis		
* @param [in] pfieldbuf	
* @param [in] fieldlen		
* @param [out] poutdata		
* @return 
* @li 
*/
int PubFPADigitPack(STISO_FIELD_DEF * pthis, const unsigned char *pfieldbuf,
					int nFieldlen, unsigned char *poutdata)
{
    int nI, nLen, nCount;
    unsigned char sztmp[MAX_FIELD_LEN];

	if(pthis ==  NULL || pfieldbuf == NULL || poutdata == NULL)
	{
		return APP_FAIL;
	}
    for (nI = 0; nI < nFieldlen; nI++)
    {
        if (!isdigit(pfieldbuf[nI]))
		{
			return APP_FAIL;
		}
    }

    nLen = ProAddPadChar(pthis, pfieldbuf, nFieldlen, sztmp, 0);
    stiso8583_fldcalc.pSet(pthis, nLen, poutdata, &nCount);

    memcpy(poutdata + nCount, (char *) sztmp, nLen);
    return nCount + nLen;
}

/**
* @brief Unpack data in ISO8583(digit string in Ascii)
* @param [in] pthis			
* @param [in] pindata	
* @param [out] pfieldbuf	
* @param [out] pfieldlen	
* @return 
* @li
*/
int PubFPADigitUnPack(STISO_FIELD_DEF * pthis, unsigned char *pfieldbuf,
					  int *pfieldlen, unsigned char *pindata)
{
    int nI, nLen, nCount;

	if(pthis ==  NULL || pfieldbuf == NULL || pfieldlen == NULL || pindata == NULL)
	{
		return APP_FAIL;
	}
    stiso8583_fldcalc.pGet(pthis, pindata, &nLen, &nCount);

    memcpy(pfieldbuf, pindata + nCount, nLen);
    *pfieldlen = nLen;

    for (nI = 0; nI < *pfieldlen; nI++)
    {
        if (!isdigit(pfieldbuf[nI]))
        {
            *pfieldlen = 0;
            return APP_FAIL;
        }
    }

    return nCount + nLen;
}

/**
* @brief Pack data in ISO8583(digit string in BCD)
* @param [in] pthis			
* @param [in] pfieldbuf		
* @param [in] fieldlen		
* @param [out] poutdata	
* @return 
* @li the length of data packed.
*/
int PubFPBDigitPack(STISO_FIELD_DEF * pthis, const unsigned char *pfieldbuf,
					int nFieldlen, unsigned char *poutdata)
{
    int nI, nLen, nCount;
    unsigned char sztmp[MAX_FIELD_LEN];

	if(pthis ==  NULL || pfieldbuf == NULL || poutdata == NULL)
	{
		return APP_FAIL;
	}
    for (nI = 0; nI < nFieldlen; nI++)
    {
        if (!isdigit(pfieldbuf[nI]))
		{
            return APP_FAIL;
		}
    }

    nLen = ProAddPadChar(pthis, pfieldbuf, nFieldlen, sztmp, 0);
    stiso8583_fldcalc.pSet(pthis, nLen, poutdata, &nCount);

    /*  */
    if ((nLen & 1) && (pthis->align == ALIGN_TYPE_LEFT))
	{
        sztmp[nLen++] = pthis->cPadchar;
    }
    //ProAscToBcd(poutdata + nCount, sztmp, nLen, (unsigned char) ((pthis->align == ALIGN_TYPE_LEFT) ? 0 : 1));
    PubAscToHex(sztmp, nLen, (unsigned char) ((pthis->align == ALIGN_TYPE_LEFT) ? 0 : 1),
				poutdata+nCount);
	return nCount + (++nLen >> 1);
}

/**
* @brief Unpack data in ISO8583(digit string in BCD)
* @param [in] pthis			
* @param [in] pindata	
* @param [out] pfieldbuf	
* @param [out] pfieldlen	
* @return 
* @li
*/
int PubFPBDigitUnPack(STISO_FIELD_DEF * pthis, unsigned char *pfieldbuf,
					  int *pfieldlen, unsigned char *pindata)
{
    int nI, nLen, nCount;

	if(pthis ==  NULL || pfieldbuf == NULL || pfieldlen == NULL || pindata == NULL)
	{
		return APP_FAIL;
	}
    stiso8583_fldcalc.pGet(pthis, pindata, &nLen, &nCount);

    //ProBcdToAsc(pfieldbuf, pindata + nCount, nLen, (unsigned char) ((pthis->align == ALIGN_TYPE_LEFT) ? 0 : 1));
	PubHexToAsc(pindata+nCount, nLen, (unsigned char) ((pthis->align == ALIGN_TYPE_LEFT) ? 0 : 1),
				pfieldbuf);
	*pfieldlen = nLen;

    for (nI = 0; nI < *pfieldlen; nI++)
    {
        if (!isdigit(pfieldbuf[nI]))
        {
            *pfieldlen = 0;
            return APP_FAIL;
        }
    }

    return nCount + (++nLen >> 1);
}

/**
* @brief Pack data in ISO8583(Amount string in BCD)
* @param [in] pthis			
* @param [in] pfieldbuf		
* @param [in] fieldlen		
* @param [out] poutdata		
* @return 
* @li the length of data packed.
*/
int PubFPBAmountPack(STISO_FIELD_DEF * pthis, const unsigned char *pfieldbuf,
					 int nFieldlen, unsigned char *poutdata)
{
    int nLen, nCount;
    unsigned char sztmp[MAX_FIELD_LEN];

	if(pthis ==  NULL || pfieldbuf == NULL || poutdata == NULL)
	{
		return APP_FAIL;
	}
    nLen = ProAddPadChar(pthis, pfieldbuf, nFieldlen, sztmp, 1); // nLen contain the char 'C/D'
    stiso8583_fldcalc.pSet(pthis, nLen, poutdata, &nCount);

    poutdata[nCount++] = sztmp[0];
    --nLen;
    //ProAscToBcd(poutdata + nCount, sztmp + 1, nLen, (unsigned char) ((pthis->align == ALIGN_TYPE_LEFT) ? 0 : 1));
    PubAscToHex(sztmp + 1, nLen,
				(unsigned char) ((pthis->align == ALIGN_TYPE_LEFT) ? 0 : 1),
				poutdata + nCount);
	return nCount + (++nLen >> 1);
}

/**
* @brief Unpack data in ISO8583(Amount string in BCD)
* @param [in] pthis			
* @param [in] pindata	
* @param [out] pfieldbuf	
* @param [out] pfieldlen	
* @return 
* @li 
*/
int PubFPBAmountUnPack(STISO_FIELD_DEF * pthis, unsigned char *pfieldbuf,
					   int *pfieldlen, unsigned char *pindata)
{
    int nLen, nCount;

	if(pthis ==  NULL || pfieldbuf == NULL || pfieldlen == NULL || pindata == NULL)
	{
		return APP_FAIL;
	}
    stiso8583_fldcalc.pGet(pthis, pindata, &nLen, &nCount); // nLen NOT contain the char 'C/D'

    pfieldbuf[0] = pindata[nCount++];

    //ProBcdToAsc(pfieldbuf + 1, pindata + nCount, nLen,
	//			(unsigned char) ((pthis->align == ALIGN_TYPE_LEFT) ? 0 : 1));
	PubHexToAsc(pindata + nCount, nLen, (unsigned char) ((pthis->align == ALIGN_TYPE_LEFT) ? 0 : 1),
				pfieldbuf + 1);
    *pfieldlen = nCount + nLen;
    return nCount + (++nLen >> 1);
}

/**
* @brief Pack data in ISO8583(Binary string in BCD)
* @param [in] pthis			
* @param [in] pfieldbuf		
* @param [in] fieldlen		
* @param [out] poutdata		
* @return 
* @li the length of data packed.
*/
int PubFPBBinaryPack(STISO_FIELD_DEF * pthis, const unsigned char *pfieldbuf,
					 int nFieldlen, unsigned char *poutdata)
{
    int nLen, nCount;
    unsigned char sztmp[MAX_FIELD_LEN];

	if(pthis ==  NULL || pfieldbuf == NULL || poutdata == NULL)
	{
		return APP_FAIL;
	}
    nLen = ProAddPadChar(pthis, pfieldbuf, nFieldlen, sztmp, 0);
    stiso8583_fldcalc.pSet(pthis, nLen, poutdata, &nCount);

    memcpy(poutdata + nCount, (char *) sztmp, nLen);
    return nCount + nLen;
}

/**
* @brief Unpack data in ISO8583(Binary string in BCD)
* @param [in] pthis			
* @param [in] pindata		
* @param [out] pfieldbuf
* @param [out] pfieldlen	
* @return 
* @li the used length of pindata when unpacking.
*/
int PubFPBBinaryUnPack(STISO_FIELD_DEF * pthis, unsigned char *pfieldbuf,
					   int *pfieldlen, unsigned char *pindata)
{
    int nLen, nCount;

	if(pthis ==  NULL || pfieldbuf == NULL || pfieldlen == NULL || pindata == NULL)
	{
		return APP_FAIL;
	}
    stiso8583_fldcalc.pGet(pthis, pindata, &nLen, &nCount);

    memcpy(pfieldbuf, pindata + nCount, nLen);
    *pfieldlen = nLen;
    return nCount + nLen;
}

/**
* @brief Clear all the member of structure STISODataStru.
* @param [in] pIsoStru		
* @param [out] pIsoStru		
* @return 
* @li APP_FAIL		Fail
* @li APP_APP_SUCC	Success
*/
int PubDelAllBit(STISODataStru *pIsoStru)
{
    int nI;

    if(NULL == pIsoStru)
    {
        return APP_FAIL;
    }

    /*Initialize the iso fields*/
    for (nI = 0; nI < 256; nI++)
    {
        pIsoStru->sBitmap[nI] = 0;
        pIsoStru->fld[nI].nOff = 0;
        pIsoStru->fld[nI].nLen = 0;
    }
    memset(pIsoStru->szDatabuf,0,MAX_MSG_LEN + MAX_FIELD_LEN);
    pIsoStru->nUsed = 0;
    return APP_SUCC;
}

/**
* @brief Clear a specified field in structure STISODataStru.
* @param [in] pIsoStru		
* @param [in] nPosition		
* @param [out] pIsoStru		DelOneBit
* @return 
* @li APP_FAIL		Fail
* @li APP_APP_SUCC	Success
*/
int PubDelOneBit(STISODataStru *pIsoStru, int nPosition)
{
    int nI;
	//int nBitmap = 0;

    if(NULL == pIsoStru)
    {
        return APP_FAIL;
    }

    if (nPosition < 0 || nPosition >= 256)
    {
        return APP_FAIL;
    }
    if (nPosition != 1) //128
	{
		pIsoStru->sBitmap[nPosition-1] = 0;
	}
    if (pIsoStru->fld[nPosition].nOff != 0 && pIsoStru->fld[nPosition].nLen != 0)
    {
        /* move  forward all the field data in iso->szDatabuf*/
        for (nI = 0; nI < 256; nI++)
        {
            if ((pIsoStru->fld[nI].nLen > 0)
				&& (pIsoStru->fld[nI].nOff > pIsoStru->fld[nPosition].nOff))
			{
                pIsoStru->fld[nI].nOff -= pIsoStru->fld[nPosition].nLen;
			}
        }
        pIsoStru->nUsed -= pIsoStru->fld[nPosition].nLen;
        for (nI = pIsoStru->fld[nPosition].nOff; nI < pIsoStru->nUsed; nI++)
		{
            pIsoStru->szDatabuf[nI] = pIsoStru->szDatabuf[nI + pIsoStru->fld[nPosition].nLen];
		}
    }
    pIsoStru->fld[nPosition].nOff = 0;
    pIsoStru->fld[nPosition].nLen = 0;

    return APP_SUCC;
}

/**
* @brief Convert string to Iso structure.
* @param [in] pcSourceStr		
* @param [in] nStrLen		
* @param [out] pTargetIsoStru	
* @param [out] pnPosition		
* @return 
* @li APP_FAIL		
* @li APP_APP_SUCC
*/
int PubStrToIso(char *pcSourceStr, int nStrLen, STISODataStru *pTargetIsoStru, int *pnPosition)
{
    int nI = 0;        /*                        */
    int unpacked = 0; /* 8583   */
    int nUsed = 0;     /* 8583 */

    if (NULL == pcSourceStr || NULL == pTargetIsoStru || NULL == pnPosition )
    {
        return APP_FAIL;
    }

    /* Initialize the pTargetIsoStru fields */
    PubDelAllBit(pTargetIsoStru);

    /* Get 128 + 64 elements */
    //for (nI = 0; nI <= ((pTargetIsoStru->sBitmap[65 - 1]) ? 192 : 128); nI++)
    for (nI = 0; nI <= 128; nI++)
    {
        /* field 0 (MSG_ID) and field 1 (BITMAP) need not check */
        if (nI >= 2 && 0 == pTargetIsoStru->sBitmap[nI - 1])
		{
			continue;
		}

        pTargetIsoStru->fld[nI].nOff = pTargetIsoStru->nUsed;
        nUsed = stiso8583_def[nI].pUnpack(&stiso8583_def[nI],
										  pTargetIsoStru->szDatabuf + pTargetIsoStru->fld[nI].nOff,
										  &pTargetIsoStru->fld[nI].nLen,
										  (unsigned char*) pcSourceStr + unpacked);
        if ((nUsed <= 0)
			|| ((unpacked + nUsed) > nStrLen)
			|| ((pTargetIsoStru->nUsed + pTargetIsoStru->fld[nI].nLen) > MAX_MSG_LEN))
        {
            if (NULL != pnPosition)
			{
                *pnPosition = nI;
			}
            pTargetIsoStru->fld[nI].nOff = 0;

            return APP_QUIT;
        }
        unpacked += nUsed;
        pTargetIsoStru->nUsed += pTargetIsoStru->fld[nI].nLen;

        if (nI == 1) /* Bitmap for 2 - 128 */
        {
            memset( pTargetIsoStru->sBitmap, 0,
					sizeof(pTargetIsoStru->sBitmap) / sizeof(pTargetIsoStru->sBitmap[0])) ;
            memcpy( pTargetIsoStru->sBitmap,
					pTargetIsoStru->szDatabuf + pTargetIsoStru->fld[nI].nOff,
					pTargetIsoStru->fld[nI].nLen);
        }
#if 0
        if (nI == 65) /* Bitmap for 129-192 */
		{
            memcpy( pTargetIsoStru->sBitmap + 128,
					pTargetIsoStru->szDatabuf + pTargetIsoStru->fld[nI].nOff,
					pTargetIsoStru->fld[nI].nLen);
		}
#endif
	}
	/**
    if (unpacked != nStrLen)
    {
        return APP_FAIL;
    }
    */ //
    return APP_SUCC;
}

/**
* @brief Convert Iso structure to string
* @param [in] SourceIsoStru	
* @param [in] pnLen				
* @param [out] pcTargetStr	BCD
* @param [out] pnLen			
* @return 
* @li APP_FAIL		Fail
* @li APP_APP_SUCC	Success
*/
int PubIsoToStr(char *pcTargetStr, STISODataStru *pSourceIsoStru, int *pnLen)
{
    int nI = 0;        /*                        */
    int packed = 0;   /* 8583   */
    int nUsed = 0;     /* 8583 */

    if (NULL == pcTargetStr || NULL == pSourceIsoStru || NULL == pnLen)
    {
        return APP_FAIL;
    }

    PubSetBit(pSourceIsoStru, 1, pSourceIsoStru->sBitmap, (pSourceIsoStru->sBitmap[0] ? 128 : 64));
#if 0
    if (pSourceIsoStru->sBitmap[65-1])
	{
        PubSetBit(pSourceIsoStru, 65, pSourceIsoStru->sBitmap + 128, 64);
	}
    for (nI = 0; nI <= 128 + pSourceIsoStru->fld[65].nLen; nI++)
#endif
	for (nI = 0; nI <= 128; nI++)
    {
        if (nI == 0 || nI == 1 || pSourceIsoStru->sBitmap[nI - 1])
        {
            nUsed = stiso8583_def[nI].pPack(&stiso8583_def[nI],
											pSourceIsoStru->szDatabuf + pSourceIsoStru->fld[nI].nOff,
											pSourceIsoStru->fld[nI].nLen,
											(unsigned char*) pcTargetStr + packed);
			if (nUsed < 0)
			{
				if (NULL != pnLen)
				{
					*pnLen = -1;
				}
				return APP_FAIL;
			}
            packed += nUsed;
        }
    }

    if (NULL != pnLen)
	{
        *pnLen = packed;
	}
	return APP_SUCC;
}

/**
* @brief Get appointed member in ISO structure, then put it in szTargetStr.
* @param [in] pSourceIsoStru	
* @param [in] nPosition		
* @param [in] pnLen			
* @param [out] pcTargetStr		
* @param [out] pnLen		
* @return 
* @li APP_FAIL		Fail
* @li APP_APP_SUCC	Success
*/
int PubGetBit(STISODataStru *pSourceIsoStru, int nPosition, char *pcTargetStr, int *pnLen)
{
    if (NULL == pSourceIsoStru || NULL == pcTargetStr || NULL == pnLen)
    {
        return APP_FAIL;
    }

    if (nPosition > 0 && 0 == pSourceIsoStru->sBitmap[nPosition - 1])
    {
        pcTargetStr[0] = '\0';
        *pnLen = 0;
        return APP_SUCC;
    }

    if (*pnLen >= pSourceIsoStru->fld[nPosition].nLen)
    {
        memcpy( pcTargetStr,
				pSourceIsoStru->szDatabuf + pSourceIsoStru->fld[nPosition].nOff,
				pSourceIsoStru->fld[nPosition].nLen);

        if (*pnLen > pSourceIsoStru->fld[nPosition].nLen)
		{
            pcTargetStr[pSourceIsoStru->fld[nPosition].nLen] = '\0';
		}
        *pnLen = pSourceIsoStru->fld[nPosition].nLen;
        return APP_SUCC;
    }
    else
    {
        *pnLen = -1;
        return APP_FAIL;
    }
	return APP_SUCC ;
}

/**
* @brief Put szTargetStr in appointed member of ISO structure.
* @param [in] nPosition			
* @param [in] pcSourceStr		
* @param [in] nStrLen			
* @param [out] pTargetIsoStru
* @return 
* @li APP_FAIL		Fail, TargetIsostru or pcSourceStr is NULL, or appointed member is invalid, or call AscToHex error, or the length of ISO structure exceeds MAXISOLEN
* @li APP_APP_SUCC	Success.
*/
int PubSetBit(STISODataStru *pTargetIsoStru, int nPosition, char *pcSourceStr, int nStrLen)
{
    unsigned char szBuf[MAX_FIELD_LEN];

	if (NULL == pTargetIsoStru || NULL == pcSourceStr)
    {
        return APP_FAIL;
    }
    PubDelOneBit(pTargetIsoStru, nPosition);

    if (nPosition > 1 && nPosition <= 196)
	{
        pTargetIsoStru->sBitmap[nPosition - 1] = 1;
	}

    if (nPosition >= 65 && nPosition <= 128)
	{
        pTargetIsoStru->sBitmap[0] = 1;
	}
    else
	{
		if (nPosition > 128)
		{
			//pTargetIsoStru->sBitmap[64] = 1;
			return APP_FAIL;
		}
	}

    if (nStrLen > MAX_FIELD_LEN)
	{
        nStrLen = MAX_FIELD_LEN;
	}

       /* because the input maybe too long, we call stiso8583_def.pack
          to valid the input data. and pTargetIsoStru->fld[nPosition].nLen
          will be updated in function unpack();*/
    pTargetIsoStru->fld[nPosition].nOff = pTargetIsoStru->nUsed;
    memcpy(pTargetIsoStru->szDatabuf + pTargetIsoStru->fld[nPosition].nOff, pcSourceStr, nStrLen);
    pTargetIsoStru->fld[nPosition].nLen = nStrLen;

    if ( (0 >= stiso8583_def[nPosition].pPack( &stiso8583_def[nPosition],
												pTargetIsoStru->szDatabuf + pTargetIsoStru->fld[nPosition].nOff,
												pTargetIsoStru->fld[nPosition].nLen, szBuf))
		|| (0 >= stiso8583_def[nPosition].pUnpack( &stiso8583_def[nPosition],
													pTargetIsoStru->szDatabuf + pTargetIsoStru->fld[nPosition].nOff,
													&pTargetIsoStru->fld[nPosition].nLen, szBuf)) )
	{
			pTargetIsoStru->fld[nPosition].nLen = 0;
			pTargetIsoStru->fld[nPosition].nOff = 0;
			return APP_FAIL;
	}
    pTargetIsoStru->nUsed += pTargetIsoStru->fld[nPosition].nLen;
    return APP_SUCC;
}

/**
* @brief Install initial parameter in structure ISO8583.
* @param [in] nPosition		Field number
* @param [in] nMaxLen		Max length
* @param [in] LenType		Length type
* @param [in] AlignType		Align type
* @param [in] chPadChar		Align charactor
* @param [in] PackFunc		Pack function
* @param [in] UnpackFunc	Unpack function
* @return 
* @li APP_FAIL		Fail
* @li APP_APP_SUCC	Success
*/
int PubSetDefine8583(int nPosition, int nMaxLen, EM_LENGTH_TYPE  LenType,
					 EM_ALIGN_TYPE AlignType, char chPadChar, FIELD_PACKAGER_PACK PackFunc,
					 FIELD_PACKAGER_UNPACK UnpackFunc)
{
	if (nPosition < 1 || nPosition > 256 || nMaxLen > 2048)
    {
        return APP_FAIL;
    }
    stiso8583_def[nPosition - 1].nLen = nMaxLen;
    stiso8583_def[nPosition - 1].lentype = LenType;
    stiso8583_def[nPosition - 1].align = AlignType;
    stiso8583_def[nPosition - 1].cPadchar = chPadChar;
    stiso8583_def[nPosition - 1].pPack = PackFunc;
    stiso8583_def[nPosition - 1].pUnpack = UnpackFunc;

    return APP_SUCC;
}

/**
* @brief Import structure ISO8583
* @param [in] pNewDef		
* @return 
* @li APP_FAIL		Fail
* @li APP_APP_SUCC	Success
*/
int PubImport8583(const struct STISO_FIELD_DEF *pNewDef)
{
    int nBitNum;
    int nRet;

    if (pNewDef == NULL)
	{
        return APP_FAIL;
	}
    for (nBitNum = 0; nBitNum <= 128; nBitNum++, pNewDef++)
    {
        nRet = PubSetDefine8583(nBitNum + 1, pNewDef->nLen, pNewDef->lentype,
								pNewDef->align, pNewDef->cPadchar, pNewDef->pPack,
								pNewDef->pUnpack);
        if (nRet == APP_FAIL)
		{
            return APP_FAIL;
		}
    }
    stiso8583_fldcalc.pSet = PubFLBLengthSet; //BCD wwt 2011-4-11
    stiso8583_fldcalc.pGet = PubFLBLengthGet; //BCD wwt 2011-4-11
    return APP_SUCC;
}

/**
* @brief Input calback function, and calculate length of field.
* @param [in] pNewCalc		
* @return 
* @li APP_FAIL		Fail
* @li APP_APP_SUCC	Success
*/
int PubSetFieldLengthCalc(const struct STISO_FIELD_LENGTH_CALC *pNewCalc)
{
	if (pNewCalc == NULL)
	{
        return APP_FAIL;
	}
	stiso8583_fldcalc.pSet = pNewCalc->pSet;
    stiso8583_fldcalc.pGet = pNewCalc->pGet;
    return APP_SUCC;
}

/**
* @brief Copy a iso data to another iso data.
* @param [in] pSourceIsoStru		
* @param [out] pTargetIsoStru		
* @return 
* @li APP_FAIL		Fail, pSourceIsoStru or pTargetIsoStru is NULL
* @li APP_APP_SUCC	Success
*/
int PubCopyIso(STISODataStru *pSourceIsoStru, STISODataStru *pTargetIsoStru)
{
    if (pSourceIsoStru == NULL || pTargetIsoStru == NULL)
	{
        return APP_FAIL;
	}
    memcpy(pTargetIsoStru, pSourceIsoStru, sizeof(STISODataStru));
    return APP_SUCC;
}

/* End of liso8583.c */
/**
* @brief Get data from FRAM
* @param [out] psBuf   --- Data
* @param [in]  nStart      --- Start location
* @param [in]  nLen        --- Data len
* @return 
* @li APP_SUCC Success
* @li APP_FAIL Fail
* @author Liug
* @date 2012-10-24
*/
int PubGetVar (char *psBuf, const int nStart, const int nLen)
{
	int nFd;

	if (NULL == psBuf || nStart < 0 || nLen <= 0)
    {
        return APP_FAIL;
    }
	if((nFd = PubFsOpen(GETSAVEVARFILE, "r")) < 0 )
	{
		return APP_FAIL;
	}
	if (PubFsSeek(nFd,nStart,SEEK_SET) != NAPI_OK)
	{
		PubFsClose(nFd);
		return APP_FAIL;
	}
	if(PubFsRead(nFd, psBuf, nLen) != nLen)
	{
		PubFsClose(nFd);
		return APP_FAIL;
	}
	PubFsClose(nFd);
	return APP_SUCC;
}

/**
* @brief Save data to FRAM
* @param [in]  psSaveBuf   --- Data
* @param [in]  nStart      --- Start location
* @param [in]  nLen        --- Data len
* @return 
* @li APP_SUCC Success
* @li APP_FAIL Fail
* @author Liug
* @date 2012-10-24
*/
int PubSaveVar (const char *psSaveBuf, const int nStart, const int nLen)
{
	int nFd;

	if (NULL == psSaveBuf || nStart < 0 || nLen <= 0)
    {
        return APP_FAIL;
    }

	if((nFd = PubFsOpen(GETSAVEVARFILE, "w")) < 0 )
	{
		return APP_FAIL;
	}
	if (PubFsSeek(nFd, nStart, SEEK_SET) != NAPI_OK)
	{
		PubFsClose(nFd);
		return APP_FAIL;
	}
	if (PubFsWrite (nFd, psSaveBuf, nLen) != nLen)
	{
		PubFsClose(nFd);
		return APP_FAIL;
	}
	PubFsClose(nFd);
	return APP_SUCC;
}

/**
* @brief Beep
* @param [in] nTimes	Times
* @return 
* @li void
*/
void PubBeep(int nTimes)
{
	int i;
	BeepMode stBeep;

	if (nTimes == 1)
	{
		stBeep.freq = 2500;
		stBeep.duration = 200;
		NAPI_SysBeep(nTimes, &stBeep);
	}
	else if (nTimes == 3)
	{
		for (i = 0; i < nTimes; i ++)
		{
			stBeep.freq = 2500;
			stBeep.duration = 40;
			NAPI_SysBeep(1, &stBeep);
			PubSysMsDelay(200);
		}
	}
	else
	{
		for(i=0; i<nTimes; i++)
		{
			NAPI_SysBeep(1, NULL);
			PubSysDelay(1);
		}
	}
}

/**
* @brief Shut down
* @return
* @li APP_SUCC	Success 
* @li APP_FAIL   Fail
*/
int PubShutDown()
{
	NAPI_SysPowerOff();

	return APP_SUCC;
}



/**
* @brief Get hardware information in the pos machine
* @param [in] emFlag	EM_HARDWARE_SUPPORT 
* @param [out] pszBuf	Hardware message
* @return 
* @li APP_FAIL		Nonsupport
* @li APP_APP_SUCC	Support
* @date 2012-11-13
*/
int PubGetHardwareSuppot(int emFlag, char* pszBuf)
{
	static char cIsGetInfo=0;
	static char szBuf[64]={0};
	int nBufLen = sizeof(szBuf);

	if (emFlag == HARDWARE_SUPPORT_PINPAD)
	{
		return APP_SUCC;
	}

	if (cIsGetInfo != 1)
	{
		if(NAPI_SysGetInfo (HW, szBuf, &nBufLen) == NAPI_OK)
		{
			cIsGetInfo = 1;
		}
		else
		{
			return APP_FAIL;
		}
	}
	if((uchar)szBuf[emFlag] == 0xFF)
	{
		return APP_FAIL;
	}
	switch(emFlag)
	{
	case HARDWARE_SUPPORT_WIRELESS:
		if (NULL == pszBuf)
		{
			break;
		}
		if (szBuf[emFlag]&0x80)
		{
			strcpy(pszBuf,"CDMA+");/**<0x80CDMA*/
		}
		else
		{
			strcpy(pszBuf,"GPRS+");
		}
		if(szBuf[emFlag] == 0x02)
		{
			strcat(pszBuf,"MC39");
		}
		else if(szBuf[emFlag] == 0x03)
		{
			strcat(pszBuf,"sim300");
		}
		else if(szBuf[emFlag] == 0x06)
		{
			strcat(pszBuf,"M72");
		}
		else if((uchar)szBuf[emFlag] == 0x81)
		{
			strcat(pszBuf,"dtgs800");
		}
		else if((uchar)szBuf[emFlag] == 0x82)
		{
			strcat(pszBuf,"dtm228c");
		}
		break;
	case HARDWARE_SUPPORT_RADIO_FREQUENCY:
		if (NULL == pszBuf)
		{
			break;
		}
		if(szBuf[emFlag] == 0x01)
		{
			strcpy(pszBuf,"RC531");
		}
		else if(szBuf[emFlag] == 0x02)
		{
			strcpy(pszBuf,"PN512");
		}
		break;
	case HARDWARE_SUPPORT_MAGCARD:
		if (NULL == pszBuf)
		{
			break;
		}
		if(szBuf[emFlag] == 0x01)
		{
			strcpy(pszBuf,"mesh");
		}
		else if(szBuf[emFlag] == 0x02)
		{
			strcpy(pszBuf,"giga");
		}
		break;
	case HARDWARE_SUPPORT_SCANNER:
		if (NULL == pszBuf)
		{
			break;
		}
		if(szBuf[emFlag] == 0x01)
		{
			strcpy(pszBuf,"EM1300");
		}
		else if(szBuf[emFlag] == 0x02)
		{
			strcpy(pszBuf,"EM3000");
		}
		else if(szBuf[emFlag] == 0x03)
		{
			strcpy(pszBuf,"SE955");
		}
		break;
	case HARDWARE_SUPPORT_PINPAD:
		break;
	case HARDWARE_SUPPORT_COMM_NUM:
		if (NULL == pszBuf)
		{
			break;
		}
		pszBuf[0] = szBuf[emFlag]+'0';
		pszBuf[1] = 0x00;
		break;
	case HARDWARE_SUPPORT_USB:
		break;
	case HARDWARE_SUPPORT_MODEM:
		break;
	case HARDWARE_SUPPORT_WIFI:
		if (NULL == pszBuf)
		{
			break;
		}
		if(szBuf[emFlag] == 0x01)
		{
			strcpy(pszBuf,"USI WM-G-MR-09");
		}
		break;
	case HARDWARE_SUPPORT_ETH:
		if (NULL == pszBuf)
		{
			break;
		}
		if(szBuf[emFlag] == 0x01)
		{
			strcpy(pszBuf,"DM9000");
		}
		else if(szBuf[emFlag] == 0x02)
		{
			strcpy(pszBuf,"589X_MAC");
		}
		break;
	case HARDWARE_SUPPORT_PRINTER:
		if (NULL == pszBuf)
		{
			break;
		}
		if(!(szBuf[emFlag] & 0x80))
		{// 0x01~0x7F :thermal
			strcpy(pszBuf,"THERMAL");
		}
		else
		{// 0x82~0x7E :stylus
			strcpy(pszBuf,"STYLUS");
		}
		break;
	case HARDWARE_SUPPORT_TOUCH_SCREEN:
		break;
	case HARDWARE_SUPPORT_LED_LIGHT:
		break;
	default:
		return APP_FAIL;
		break;
	}
	return APP_SUCC;
}



/**
* @brief Get pos information
* @param [in] emFlag	EM_HARDWARE_GET 
* @param [out] pszBuf	hardware info
* @return 
* @li APP_FAIL		Fail
* @li APP_APP_SUCC	Success
*/
int PubGetPosInfo(int emFlag, char* pszBuf, int nBuffLen)
{
	if (pszBuf != NULL && emFlag>=MODEL && emFlag<=HW)
	{
		if(NAPI_SysGetInfo((SYS_INFO_ID)(emFlag), pszBuf, &nBuffLen) == NAPI_OK)
		{
			return APP_SUCC;
		}
	}
	return APP_FAIL;
}

/**
* @brief Get Security configure
* @param [in] emFlag	EM_SEC_CFG 
* @param 
* @return 
* @li YES
* @li NO
*/
YESORNO PubGetSecCfg(int emSecCfg)
{
	uint unCfg=0;
	int nRet;

	//nRet = NAPI_SecGetCfg(&unCfg);
	nRet=SEC_CFG_UNIQUE;
	if (nRet != NAPI_OK) {
		PubDebug("NAPI_SecGetCfg = %d", nRet);
		return APP_FAIL;
	}

	PubDebug("unCfg = %x", unCfg);
	switch (emSecCfg)
	{
	case SEC_CFG_UNIQUE:
		if (unCfg & 0x00000002) {
			return YES;
		}
		break;
	case SEC_CFG_MISUSE:
		if (unCfg & 0x00000004) {
			return YES;
		}
		break;
	case SEC_CFG_KEYLEN_LIMIT:
		if (unCfg & 0x00000020) {
			return NO;
		}
		return YES;
	case SEC_CFG_CLEARKEY_LIMIT:
		if (unCfg & 0x00000100) {
			return NO;
		}
		return YES;
	default:
		break;
	}

	return NO;
}



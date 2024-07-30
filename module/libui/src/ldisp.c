#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <pthread.h>
#include "uitool.h"
#include "lui.h"
#include "ltool.h"
#include "lcomm.h"
#include "libapiinc.h"


#define THREADRUN 1
#define THREADSTOP 0

static uint gunSrcWidth = 0;
extern void ProSwitchFontSize(EM_DISPLAY_ALIGN emFont);

void GetLcdWidth(uint *punWidth)
{
	if(gunSrcWidth != 0)
	{
		*punWidth = gunSrcWidth;
	}
	else
	{
		uint unScrHeight;
		NAPI_ScrGetLcdSize  (&gunSrcWidth, &unScrHeight);
		*punWidth = gunSrcWidth;
	}
}

int GetChaWidth(char c)
{
	uint unStrWidth = 0;
	char szStr[2] = {0};
	szStr[0] = c;
	NAPI_ScrGetTrueTypeFontSize(szStr, 1, (int *)&unStrWidth, NULL);	
	return unStrWidth;
}


int GetStrWidth(char* pszStr)
{
	uint unStrWidth = 0, unStrlen = 0;
	int	 nRet;
	unStrlen = strlen(pszStr);

	if(unStrlen==0)
		return 0;
	
	nRet = NAPI_ScrGetTrueTypeFontSize(pszStr, unStrlen, (int *)&unStrWidth, NULL);	

	if(nRet!=NAPI_OK)
		return APP_FAIL;
	else
		return unStrWidth;

}

void TrimStrToOneLine(int nStartX, char *pszStr, int nStrlen, int *pnTrimedLen, int *nEndX)
{
	int i, nRet, nTmp;
	uint unScrWidth, unStrWidth, unTmpWidth;

	if(nStrlen == 0)
	{
		*pnTrimedLen =0;
		return;
	}
		
	
	GetLcdWidth  (&unScrWidth);
	unScrWidth -= nStartX;
	
	if(unScrWidth < 0)
	{
		*pnTrimedLen = 0;
		return;
	}
	
	nRet = NAPI_ScrGetTrueTypeFontSize(pszStr, nStrlen, (int *)&unStrWidth, NULL);
	if((nRet == NAPI_OK)&& (unStrWidth<= unScrWidth))
	{
		*pnTrimedLen = nStrlen;
		if(nEndX)
			*nEndX = unStrWidth + nStartX;		
		return;
	}

	nTmp = 0;
	unTmpWidth = 0;

	
	for(i=1;i<=nStrlen; i++)
	{
		if(pszStr[i-1] == '\n')
		{
			*pnTrimedLen = i;
			return;
		}
		nRet = NAPI_ScrGetTrueTypeFontSize(pszStr, i, (int *)&unStrWidth, NULL);
		//if the last char is not a full font tha API will return fail or  unStrWidth will not be longer than before
		if(nRet == NAPI_OK)
		{
			if(unStrWidth > unScrWidth)
			{
				*pnTrimedLen = nTmp;
				if(nEndX)
					*nEndX = unTmpWidth + nStartX;
				return;
			}
			else if(unStrWidth>unTmpWidth)
			{
				unTmpWidth = unStrWidth;
				nTmp = i;
			}

		}
	}

	*pnTrimedLen = nStrlen;
}


// Format
static void ProGetStrFormatOneLine(int nFont, char *pOutStr, char *pInStr, int nLine)
{
	int i=0;
	int nLen=0;
	int nPos=0;
	int nCur=0 , nTmp = 0;
	int nMark=1;
	// int nMaxCol=0;
	int nItemNum=0;
	char *p=pInStr;
	char szFormat[128]={0};
	char szItem[32][256];
	uint unScrWidth, unScrHeight,unStrWidth, unSpcWidth, unCurWidth;
	int nRet;
	
	if(pInStr == NULL || pOutStr == NULL)
	{
		return;
	}
	memset(szItem, 0, sizeof(szItem));
	
	if(nFont == SMALLFONT)
	{
		ProSwitchFontSize(SMALLFONT);
	}
	
	NAPI_ScrGetTrueTypeFontSize("\x20", 1, (int *)&unSpcWidth, NULL);
	
	NAPI_ScrGetLcdSize(&unScrWidth, &unScrHeight);
	
	

	// nMaxCol = unScrWidth/unSpcWidth;
	while(*p != '\0' && nItemNum < 32)
	{
		nPos = PubPosChar(p,'|');
		if (nPos < 0 )
		{
			nLen = strlen(p);
			if (nLen > 0 )
			{
				memcpy(szItem[nItemNum], p, nLen);
				nItemNum++;
			}
			break;
		}
		if(*(p+nPos+1) == '|')
		{
			nMark++;
			continue;
		}
		if(*(p+nPos+1) == 'L' || *(p+nPos+1) == 'C' || *(p+nPos+1) == 'R' )
		{
			if(nMark%2 == 0)
			{
				nMark = 1;
				continue;
			}
		}
		if(nPos == 0)
		{
			p += (nPos+1);
			continue;
		}
		nLen = nPos - nMark/2;
		memcpy(szItem[nItemNum], p, nLen);
		p += (nPos+1);
		nItemNum++;
		nMark = 1;
	}
	nMark = 0;
	nCur = 0;
	unCurWidth = 0;
	nTmp = 0;
	memset(szFormat, 0x20, sizeof(szFormat));
	for(i=0; i<nItemNum; i++)
	{
		//printf("szItem[%d]:%s\r\n", i, szItem[i]);
		nLen = strlen(szItem[i])-1;
		nRet = NAPI_ScrGetTrueTypeFontSize(szItem[i]+1, nLen, (int *)&unStrWidth, NULL);
		/**<Subtract the length format-char */
		if((nRet==APP_SUCC) && (unStrWidth > 0))
		{
			if(unCurWidth+unStrWidth > unScrWidth)
			{
				if(nCur > 0)
				{
					strcat(pOutStr, szItem[i-1]+1);
					nCur = 0;
				}
				strcat(pOutStr, szItem[i]+1);
				if(!(nItemNum == 1 && nLine == 1))
				{
					strcat(pOutStr, "\n");
				}
				unCurWidth = unStrWidth;
				memset(szFormat, 0, sizeof(szFormat));
				memset(szFormat, 0x20, sizeof(szFormat));
			}
			else
			{
				switch(szItem[i][0])
				{
				case 'L':
					memcpy(szFormat, szItem[i]+1, nLen);
					unCurWidth = unStrWidth;
					nCur = nLen;
					break;
				case 'C':
					nTmp = ((unScrWidth-unStrWidth)/2 - unCurWidth) /unSpcWidth;
					memcpy(szFormat+nCur + nTmp, szItem[i]+1, nLen);
					unCurWidth = (unScrWidth-unStrWidth)/2 + unStrWidth;
					nCur += (nTmp + nLen);
					break;
				case 'R':
					nTmp = (unScrWidth-unStrWidth - unCurWidth)/unSpcWidth;
					memcpy(szFormat + nCur +nTmp, szItem[i]+1, nLen);
					nCur += (nTmp + nLen);
					NAPI_ScrGetTrueTypeFontSize(szItem[i]+1, 1, (int *)&unStrWidth, NULL);
					unCurWidth = unScrWidth;
					break;
				default:
					strcpy(szFormat, szItem[i]);
					nCur = nLen+1;
					unCurWidth = unStrWidth;
					break;
				}
			}
		}
	}

	if(nCur > 0)
	{
		szFormat[nCur] = '\0';//modify the issue of "|R" increase one line
		strcat(pOutStr, szFormat);
	}

	if(nFont == SMALLFONT)//restore the normal font
	{
		ProSwitchFontSize(BIGFONT);
	}	
}



void GetStrFormat(int nFont, char *pOutStr, char *pInStr)
{
	int nPos = 0;
	char *p = pInStr;
	char szMaxInBuf[1024];
	char szMaxOutBuf[1024];

	if(pInStr == NULL || pOutStr == NULL)
	{
		return;
	}
	pOutStr[0]='\0';
	while(*p != '\0')
	{
		nPos = PubPosChar(p,'\n');
		memset(szMaxInBuf, 0, sizeof(szMaxInBuf));
		memset(szMaxOutBuf, 0, sizeof(szMaxOutBuf));
		if (nPos < 0 )
		{
			if (strlen(p) > 0 )
			{
				if(p[0] != '|')
				{
					strcpy(szMaxInBuf, "|L");
				}
				strcat(szMaxInBuf, p);
				ProGetStrFormatOneLine(nFont, szMaxOutBuf, szMaxInBuf, 0);
				strcat(pOutStr, szMaxOutBuf);
			}
			break;
		}
		else if(nPos > 0)
		{
			if(p[0] != '|')
			{
				strcpy(szMaxInBuf, "|L");
				memcpy(szMaxInBuf+2,p, nPos);
			}
			else
			{
				memcpy(szMaxInBuf, p, nPos);
			}
			ProGetStrFormatOneLine(nFont, szMaxOutBuf, szMaxInBuf, 1);
			strcat(pOutStr, szMaxOutBuf);
		}
		strcat(pOutStr, "\n");	
		p += (nPos+1);
	}
}


void DisplayLogo(char *pszPath, int nX, int nY)
{
	uint unX, unY, unScrWidth, unScrHeight;

	NAPI_ScrGetViewPort(&unX, &unY, &unScrWidth, &unScrHeight);
	NAPI_ScrDispPic(nX, nY, unScrWidth-nX, unScrHeight-nY, pszPath);
}


void DispBoldFontText(uint unX, uint unY, const char *str)
{
	//Display 3 times to realize  bold effect
	NAPI_ScrDispTrueTypeFontText(unX, unY, str, strlen(str));
	NAPI_ScrDispTrueTypeFontText(unX, unY, str, strlen(str));
	NAPI_ScrDispTrueTypeFontText(unX, unY, str, strlen(str));
}

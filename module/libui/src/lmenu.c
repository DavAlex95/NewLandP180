#include <string.h>
#include <stdlib.h>
#include "uitool.h"
#include "lui.h"
#include "lmenu.h"
#include "linput.h"
#include "ltool.h"
#include "ldisp.h"

#include "napi_display.h"


void DisplayTitleWithStyle(char *pszTitle, int nUpDown)
{
	uint unScrWidth;
	uint unLineHeight;
	STUIFUN_STYLE stUiFunStyle;
	EM_TEXT_ATTRIBUTE emOldAttr;

	PubClearLine(1, 1);
	GetLcdWidth(&unScrWidth);
	PubGetDispForm(NULL, NULL, (int*)&unLineHeight);
	NAPI_ScrRectangle(0, 0, unScrWidth,unLineHeight, RECT_PATTERNS_SOLID_FILL, TITLE_BG_COLOUR);//Draw bg rectangle 
	NAPI_ScrSetAttr(TEXT_ATTRIBUTE_REVERSE , &emOldAttr);
	PubDisplayGen(1, pszTitle);
	

	PubGetUiStyle(&stUiFunStyle);
	if(nUpDown & PAGEDOWNTIP)
	{
		PubDisplayStrInline(DISPLAY_MODE_TAIL, 1, stUiFunStyle.szPageDown);
	}
	if(nUpDown & PAGEUPTIP)
	{
		PubDisplayStrInline(0 ,1 , stUiFunStyle.szPageUp);
	}
	
	NAPI_ScrSetAttr(emOldAttr , NULL);
}

void DisplayTailWithStyle(char *pszTail, int nUpDown)
{
	uint unScrWidth;
	uint unLineHeight;
	STUIFUN_STYLE stUiFunStyle;
	int 	nMaxLine;
	

	PubGetDispView(&nMaxLine,NULL);
	PubClearLine(nMaxLine, nMaxLine);
	GetLcdWidth(&unScrWidth);
	PubGetDispForm(NULL, NULL, (int*)&unLineHeight);
	
	NAPI_ScrRectangle(0, (nMaxLine-1)*unLineHeight , unScrWidth,unLineHeight, RECT_PATTERNS_SOLID_FILL, DISP_INVBG_COLOUR);//Draw bg rectangle 
	PubDisplayGen(nMaxLine, pszTail);
	

	PubGetUiStyle(&stUiFunStyle);
	if(nUpDown & PAGEDOWNTIP)
	{
		PubDisplayStrInline(DISPLAY_MODE_TAIL, nMaxLine, stUiFunStyle.szPageDown);
	}
	if(nUpDown & PAGEUPTIP)
	{
		PubDisplayStrInline(0 ,nMaxLine , stUiFunStyle.szPageUp);
	}
	
}


static void DispListItem( uint unX, uint unY, const char* pszStr, int nTimeOut, int* pnItemNum)
{
	int	i,nPos,nKey;	
	int nSelect = 0;
	int nItemNum = 0;
	int nLineNum = 0;
	uint unFormX;
	uint unFormY;
	uint unFormWidth;
	uint unFormHeight;
	uint unFontWidth;
	uint unFontHeight;
	int nCurrentNo=0; 
	char *p = (char *)pszStr;
	char szBuf[64];
	uchar usKey[50];
	char szItem[50][64];
	int nLineHeight;
	
	if ( pszStr == NULL || strlen(pszStr) == 0)
	{
		return;
	}

	memset(usKey,0,sizeof(usKey));
	memset(szItem,0,sizeof(szItem));
	
	while(*p != '\0')
	{
		nPos = PubPosChar(p,'|');
		if (nPos < 0 )
		{
			if (strlen(p) > 0 )
			{
				usKey[nItemNum] = *p;
				strcpy(szItem[nItemNum],p); 
				nItemNum++;
			}
			break;
		}
		usKey[nItemNum] = *p;
		memcpy(szItem[nItemNum],p,nPos); 
		p += (nPos+1);
		nItemNum++;
	}
	NAPI_ScrGetFontSize(&unFontWidth,&unFontHeight);
	NAPI_ScrGetViewPort(&unFormX,&unFormY,&unFormWidth,&unFormHeight);
	PubGetDispForm(NULL, NULL, &nLineHeight);
	
	nLineNum = (unFormHeight - unY+1) / nLineHeight;
	while(1)
	{
		NAPI_ScrClrLine(unY, unFormY+unFormHeight);
		unFormY = unY;
		for (i=nCurrentNo; (i<nCurrentNo+nLineNum*2) && (i<nItemNum);)
		{
			/**<Displays the left half of the line*/
			if(strlen(szItem[i]) > 0)
			{
				PubStrCopy(szBuf, szItem[i], unFormWidth/(unFontWidth/2));
				if (usKey[i]-'0' == nSelect)
				{
					NAPI_ScrSetAttr (TEXT_ATTRIBUTE_REVERSE,NULL);
					NAPI_ScrDispTrueTypeFontText(0,unFormY, szBuf, strlen(szBuf));
					NAPI_ScrSetAttr (TEXT_ATTRIBUTE_NORMAL ,NULL);
				} 
				else
				{
					NAPI_ScrDispTrueTypeFontText(0,unFormY, szBuf, strlen(szBuf));
				}
			}
			i++;
			/**<Displays the right half of the line*/
			if(strlen(szItem[i]) > 0)
			{
				PubStrCopy(szBuf, szItem[i], (unFormWidth/2)/(unFontWidth/2));
				if (usKey[i]-'0' == nSelect)
				{
					NAPI_ScrSetAttr (TEXT_ATTRIBUTE_REVERSE,NULL);
					NAPI_ScrDispTrueTypeFontText(unFormWidth / 2,unFormY, szBuf, strlen(szBuf));
					NAPI_ScrSetAttr (TEXT_ATTRIBUTE_NORMAL ,NULL);
				}
				else
				{
					NAPI_ScrDispTrueTypeFontText(unFormWidth / 2,unFormY, szBuf, strlen(szBuf));
				}
			}
			i++;
			unFormY += nLineHeight;
		}

		if (PubGetKbAttr() == KB_VIRTUAL) {
			nKey = PubShowGetKbPad(nTimeOut, BUTTON_NORMAL, "UP", "CANCEL", "ENTER", "DOWN");
		} else {
			nKey = PubGetKeyCode(nTimeOut);
		}

		switch (nKey)
		{
		case KEY_F1:
		case KEY_STAR:
			if((nCurrentNo-nLineNum*2) >= 0)
				nCurrentNo -= nLineNum*2;
			break;
		case KEY_F2:
		case KEY_SHARP:
			if((nCurrentNo+nLineNum*2) < nItemNum)
				nCurrentNo += nLineNum*2;
			break;
		default:
			*pnItemNum = nKey;
			return;
			break;
		}
	}
	return;
}

void DrawBitmapList(const uint unWidth, const uint unHeight, const char *pszBitmap, const char *pszContent, int nTimeout, int* pnItemNum)
{
	uint unDispStrHeight = 0;
	uint unX,unY;
	uint unScrWidth,unScrHeight;
	uint unFontWidth,unFontHeight;

	NAPI_ScrGetViewPort(&unX, &unY, &unScrWidth,&unScrHeight);
	NAPI_ScrGetFontSize(&unFontWidth, &unFontHeight);

	PubClearAll();
	if(unWidth > 0 && unHeight > 0 && unScrHeight>64)
	{
		NAPI_ScrDrawBitmap(0, 0, unWidth, unHeight, pszBitmap );
		if(unHeight%unFontWidth == 0)
		{
			unDispStrHeight = unHeight;
		}
		else
		{
			unDispStrHeight = unHeight + unFontWidth - unHeight%unFontWidth;
		}
	}
	DispListItem(0, unDispStrHeight, pszContent, nTimeout, pnItemNum);
}

void DispPicList(const uint unWidth, const uint unHeight, const char *pszPic, const char *pszContent, int nTimeout, int* pnItemNum)
{
	uint unDispStrHeight = 0;
	uint unX,unY;
	uint unScrWidth,unScrHeight;
	uint unFontWidth,unFontHeight;

	NAPI_ScrGetViewPort(&unX, &unY, &unScrWidth,&unScrHeight);
	NAPI_ScrGetFontSize(&unFontWidth, &unFontHeight);
	
	PubClearAll();
	if(unWidth>0 && unHeight>0 && pszPic!=NULL && strlen(pszPic)>0 && unScrHeight>64)
	{
		NAPI_ScrDispPic(0, 0, unWidth, unHeight, pszPic);
		if(unHeight%unFontWidth == 0)
		{
			unDispStrHeight = unHeight;
		}
		else
		{
			unDispStrHeight = unHeight + unFontWidth - unHeight%unFontWidth;
		}
	}
	DispListItem(0, unDispStrHeight, pszContent, nTimeout, pnItemNum);
}


int UpDownMsgDlg(const char* pszTitle, const char* pszContent,int nIsResKey, int nTimeOut, int* nResKey)
{
	char cUpFlag;
	char cDownFlag;
	char cShowTitleFlag;
	char cShowTailFlag;
	int i;
	int nKeyCode;
	int nTotalNum;
	int nCurrentNo;
	int nContentLen,nLen;
	int nTitleStyle;
	int nMaxHzLines;
	uint unX,unY,unScrWidth,unScrHeight,unSWidth, unTmpWidth;
	uint unFontWidth,unFontHeight;
	STUIFUN_STYLE stUiFunStyle;
	char szTail[64];
	char szDisp[100][64], szTmp[64];
	int	 nCount;
	int nRet;

	if (pszContent == NULL || (nContentLen = strlen(pszContent)) < 0)
	{
		return APP_FAIL;
	}

	PubGetUiStyle(&stUiFunStyle);
	NAPI_ScrGetFontSize(&unFontWidth, &unFontHeight);
	NAPI_ScrGetViewPort(&unX,&unY,&unScrWidth,&unScrHeight);

	PubGetDispView(&nMaxHzLines, NULL);
	cShowTitleFlag = 0;
	if (pszTitle!=NULL && strlen(pszTitle)>0)
	{
		cShowTitleFlag = 1;
	}
	cShowTailFlag=0;
	if(cShowTitleFlag == 0)
	{
		cShowTailFlag = 1;
	}
	else if(nIsResKey == NO)
	{
		cShowTailFlag = 1;
	}

	nLen = 0;
	nTotalNum = 0;
	nCount	= 1;
	memset(szDisp,0,sizeof(szDisp));
	memset(szTmp,0,sizeof(szTmp));
	unTmpWidth = 0;
	for(i=0; i<nContentLen; i++)
	{
		if (pszContent[i] == 0x0A)
		{
			nTotalNum++;
			memset(szTmp,0,sizeof(szTmp));
			nLen = 0;
			continue;
		}
		
		memcpy(szTmp + nLen,pszContent + i,1);
		nRet = NAPI_ScrGetTrueTypeFontSize(szTmp, nLen+ 1, (int *)&unSWidth, NULL);
		if((nRet!=APP_SUCC) || (unTmpWidth==unSWidth))
		{
			nCount++;
			nLen++;
			continue;
		}
		unTmpWidth = unSWidth;
		
		if(unSWidth > unScrWidth)
		{
			nTotalNum ++;
			memcpy(szDisp[nTotalNum] ,pszContent + i - (nCount-1), nCount);
			memset(szTmp,0,sizeof(szTmp));
			memcpy(szTmp ,pszContent + i-(nCount-1),nCount);
			nLen = nCount;
		
		}
		else
		{
			memcpy(szDisp[nTotalNum] + nLen - (nCount-1),pszContent + i-(nCount-1),nCount);
			nLen ++;
		}
		nCount = 1;		
	}
	
	if(pszContent[nContentLen-1] != '\n')
	{
		nTotalNum++;
	}
	nCurrentNo = 0;
	while (1)
	{
		cUpFlag = FALSE;
		cDownFlag = FALSE;
		if(nTotalNum == nMaxHzLines-cShowTitleFlag)
		{
			cShowTailFlag=0;
		}
		if (nTotalNum > nMaxHzLines-cShowTitleFlag-cShowTailFlag)
		{
			//Page down
			if (nCurrentNo+nMaxHzLines-cShowTitleFlag-cShowTailFlag < nTotalNum)
			{
				if(nTotalNum == nMaxHzLines-cShowTitleFlag)
				{
					cShowTailFlag = 0;// Not display tail when only one page
				}
				else
				{
					cDownFlag = TRUE;
				}
			}
			else
			{
			}
			//Page up
			if (nCurrentNo > nMaxHzLines-cShowTitleFlag-cShowTailFlag -1)
			{
				cUpFlag = TRUE;
			}
			else
			{
			}
		}
		PubClearAll();
		if(cShowTitleFlag)
		{
			nTitleStyle=0;
			if(nIsResKey != NO)
			{
				if(cDownFlag == TRUE)
				{
					nTitleStyle |= PAGEDOWNTIP;
				}
				if(cUpFlag == TRUE)
				{
					nTitleStyle |= PAGEUPTIP;
				}
			}
			DisplayTitleWithStyle((char*)pszTitle, nTitleStyle);
		}
		if(cShowTailFlag)
		{
			if(nIsResKey == NO)
			{
				if(cDownFlag == TRUE || cUpFlag == TRUE)
				{
					PubDisplayTail(0, 1, YES, "[ENTER] for details");
				}
				else
				{
					cShowTailFlag = 0;
				}
			}
			else
			{
				memset(szTail,0,sizeof(szTail));
				strcpy(szTail, "[CANCEL] to ESC");
				if(cDownFlag == TRUE && cUpFlag == TRUE)
				{
					DisplayTailWithStyle(szTail, PAGEDOWNTIP | PAGEUPTIP);
				}
				else if(cDownFlag == TRUE)
				{
					DisplayTailWithStyle(szTail, PAGEDOWNTIP);
				}
				else if(cUpFlag == TRUE)
				{
					DisplayTailWithStyle(szTail, PAGEUPTIP);
				}
				else
				{
					cShowTailFlag = 0;
				}
			}
		}
		else
		{
			if(nIsResKey == NO)
			{
				if(cDownFlag == TRUE || cUpFlag == TRUE)
				{
					PubDisplayTail(0, 1, YES, "[ENTER] for details");
				}
			}
		}
		for (i=0; i<nMaxHzLines-cShowTitleFlag-cShowTailFlag; i++)
		{
			if (nCurrentNo+i < nTotalNum)
			{
				PubDisplayStr(0,i+1+cShowTitleFlag,1,"%s",szDisp[nCurrentNo+i]);
			}
		}
		while(1)
		{
			if (PubGetKbAttr() == KB_VIRTUAL) {
				if(nResKey == NULL) {
					nKeyCode = PubShowGetKbPad(nTimeOut, BUTTON_CONFIRM, "CANCEL", "ENTER", NULL, NULL);
				} else if (nIsResKey == YES) {
					nKeyCode = PubShowGetKbPad(nTimeOut, BUTTON_CONFIRM, "UP", "CANCEL", "ENTER", "DOWN");
				} else {
					nKeyCode = PubShowGetKbPad(nTimeOut, BUTTON_CONFIRM, "UP", "CANCLE", "DOWN", NULL);
				}
			} else {
				nKeyCode = PubGetKeyCode(nTimeOut);
			}

			if((cUpFlag != TRUE && cDownFlag != TRUE) || nIsResKey == NO)
			{
				if(nResKey != NULL)
				{
					*nResKey = nKeyCode;
				}
				if( (nKeyCode == KEY_ENTER) && (nIsResKey == NO)
					 && (cUpFlag == TRUE || cDownFlag == TRUE) )
				{
					return APP_FUNCQUIT;
				}
				if(nKeyCode == KEY_ESC)
				{
					return APP_QUIT;
				}
				return APP_SUCC;
			}
			switch (nKeyCode)
			{
			case KEY_UP:
				if (cUpFlag != TRUE)
				{
					continue;
				}
				nCurrentNo -= nMaxHzLines-cShowTitleFlag-cShowTailFlag;
				break;
			case KEY_DOWN:
				if (cDownFlag != TRUE)
				{
					continue;
				}
				nCurrentNo += nMaxHzLines-cShowTitleFlag-cShowTailFlag;
				break;
			case 0: 
				if(nResKey != NULL)
				{
					*nResKey = nKeyCode;
				}
				return APP_TIMEOUT;
			case KEY_ESC:
				if(nResKey != NULL)
				{
					*nResKey = nKeyCode;
				}
				return APP_QUIT;
			case KEY_ENTER:
				if(nResKey != NULL)
				{
					*nResKey = nKeyCode;
				}
				return APP_SUCC;
			default:
				continue;
				break;
			}
			break;
		}
	}
	return APP_SUCC;
}



int UpDownConfirmDlg(const char* pszTitle, const char* pszContent,int nBeep, int nTimeOut)
{
	char cUpFlag;
	char cDownFlag;
	char cShowTitleFlag;
	int i;
	int nKeyCode;
	int nTotalNum;
	int nCurrentNo;
	int nContentLen,nLen;
	int nTitleStyle;
	uint unX,unY;
	uint unFontWidth,unFontHeight;
	uint unScrWidth,unScrHeight, unSWidth, unTmpWidth;
	char szDisp[100][64] ,szTmp[64];
	int nMaxLine, nCount, nRet;
	
	if (pszContent == NULL)
	{
		return APP_FAIL;
	}
	nContentLen = strlen(pszContent);
	if (nContentLen <= 0 )
	{
		return APP_FAIL;
	}
	cShowTitleFlag = 0;
	if (pszTitle != NULL)
	{
		if (strlen(pszTitle)>0)
		{
			cShowTitleFlag = 1;
		}
	}


	nLen = 0;
	nTotalNum = 0;
	nCount	= 1;
	memset(szDisp,0,sizeof(szDisp));
	memset(szTmp,0,sizeof(szTmp));
	unTmpWidth = 0;	
	NAPI_ScrGetViewPort(&unX, &unY, &unScrWidth,&unScrHeight);
	NAPI_ScrGetFontSize(&unFontWidth, &unFontHeight);
	PubGetDispView(&nMaxLine, NULL);
	


	for(i=0; i<nContentLen; i++)
	{
		if (pszContent[i] == 0x0A)
		{
			nTotalNum++;
			memset(szTmp,0,sizeof(szTmp));
			nLen = 0;
			continue;
		}
		
		memcpy(szTmp + nLen,pszContent + i,1);
		nRet = NAPI_ScrGetTrueTypeFontSize(szTmp, nLen+ 1, (int *)&unSWidth, NULL);
		if((nRet!=APP_SUCC) || (unTmpWidth==unSWidth))
		{
			nCount++;
			nLen++;
			continue;
		}
		unTmpWidth = unSWidth;
		
		if(unSWidth > unScrWidth)
		{
			nTotalNum ++;
			memcpy(szDisp[nTotalNum] ,pszContent + i - (nCount-1), nCount);
			memset(szTmp,0,sizeof(szTmp));
			memcpy(szTmp ,pszContent + i-(nCount-1),nCount);
			nLen = nCount;
		
		}
		else
		{
			memcpy(szDisp[nTotalNum] + nLen - (nCount-1),pszContent + i-(nCount-1),nCount);
			nLen ++;
		}
		nCount = 1;		
	}

	if(pszContent[nContentLen-1] != '\n')
	{
		nTotalNum++;
	}

	nCurrentNo = 0;
	while (1)
	{
		cUpFlag = FALSE;
		cDownFlag = FALSE;
		if (nTotalNum > nMaxLine-cShowTitleFlag-1)
		{
			//Page down
			if (nCurrentNo+nMaxLine-cShowTitleFlag-1 < nTotalNum)
			{
				cDownFlag = TRUE;
			}
			else
			{
			}
			//Page up
			if (nCurrentNo > nMaxLine-cShowTitleFlag-1 -1)
			{
				cUpFlag = TRUE;
			}
			else
			{
			}
		}
		PubClearAll();
		if(cShowTitleFlag)
		{
			nTitleStyle=0;
			if(cDownFlag == TRUE)
			{
				nTitleStyle |= PAGEDOWNTIP;
			}
			if(cUpFlag == TRUE)
			{
				nTitleStyle |= PAGEUPTIP;
			}
			DisplayTitleWithStyle((char*)pszTitle, nTitleStyle);
		}

		PubDisplayStrInline(0, nMaxLine, (char*)("[CANCEL]"));
		PubDisplayStrInline(DISPLAY_MODE_TAIL, nMaxLine, (char*)("[ENTER]"));		
		for (i=0; i<nMaxLine-cShowTitleFlag-1; i++)
		{
			if (nCurrentNo+i < nTotalNum)
			{
				PubDisplayStr(0,i+1+cShowTitleFlag,1,"%s",szDisp[nCurrentNo+i]);
			}
		}
		PubUpdateWindow();
		PubBeep(nBeep);
		while(1)
		{
			if (PubGetKbAttr() == KB_VIRTUAL) {
				nKeyCode = PubShowGetKbPad(nTimeOut, BUTTON_NORMAL, "UP", "CANCEL", "ENTER", "DOWN");
			} else {
				nKeyCode = PubGetKeyCode(nTimeOut);
			}
			switch (nKeyCode)
			{
			case KEY_UP:
				if (cUpFlag != TRUE)
				{
					continue;
				}
				nCurrentNo -= nMaxLine-cShowTitleFlag-1;
				break;
			case KEY_DOWN:
				if (cDownFlag != TRUE)
				{
					continue;
				}
				nCurrentNo += nMaxLine-cShowTitleFlag-1;
				break;
			case 0:
			case APP_TIMEOUT:
				return APP_TIMEOUT;
			case KEY_ENTER:
				return APP_SUCC;
			case KEY_ESC:
				return APP_QUIT;
			default:
				continue;
				break;
			}
			break;
		}
	}
	return APP_SUCC;
}


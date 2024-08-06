#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/time.h> 
#include "uitool.h"
#include "lui.h"
#include "linput.h"
#include "ldisp.h"
#include "Ltool.h"
#include "lmenu.h"
#include "libapiinc.h"
#include "virtualpad.h"

#define INPUT_MAX_LINE_NUM		8
#define INPUT_MAX_COLUMN_NUM	40	
#define INPUT_MAX_CHA_NUM		INPUT_MAX_LINE_NUM*INPUT_MAX_COLUMN_NUM

//#define AMT_TYPE_POINT			

#ifdef AMT_TYPE_POINT
#define AMT_SYMBOL				'.'   	//Amount format 1234.56
#else
#define AMT_SYMBOL				','		//Amount format 123,456
#endif

#define CURRENCY_NAME			""

typedef struct
{
	int  nMode;			
	int  nGetNum;		
	char szGetBuf[INPUT_MAX_CHA_NUM];
	char szTmp[INPUT_MAX_CHA_NUM];
	char szDispBuf[INPUT_MAX_LINE_NUM][INPUT_MAX_COLUMN_NUM];
	int  snLOffset[INPUT_MAX_LINE_NUM];
	int  snLEndX[INPUT_MAX_LINE_NUM];
	char *pcShow;
	int  nLcdWidth;
	int  nLcdHeight;
	int  nDispLine;	
	int	 nOrgLine;
	int  nStartLine;
	int  nStartX;
	int	 nStartY;
	int	 nEndY;
	int	 nEditFlag;
	int  nCursorX;
	int	 nCursorY;	
	int  nCurOffset;
	int	 nCursorLine;
	int  nOrgCursorLine;
	int	 nIfStrUndate;
}ST_INPUTDISP;

ST_INPUTDISP gstIptInfo;

const char	ChTbl[]=
{
	'0', '=', '<', '>', '(', ')', '&', '{', '}', '[', ']','0',
	'1', 'Q', 'Z', 'q', 'z', ' ', '#', '*', '1',
	'2', 'A', 'B', 'C', 'a', 'b', 'c', '2',
	'3', 'D', 'E', 'F', 'd', 'e', 'f', '3',
	'4', 'G', 'H', 'I', 'g', 'h', 'i', '4',
	'5', 'J', 'K', 'L', 'j', 'k', 'l', '5',
	'6', 'M', 'N', 'O', 'm', 'n', 'o', '6',
	'7', 'P', 'R', 'S', 'p', 'r', 's', '7',
	'8', 'T', 'U', 'V', 't', 'u', 'v', '8',
	'9', 'W', 'X', 'Y', 'w', 'x', 'y', '9',
	'.', '-', '_', ',', '@', '\\', '/', '+', '|', '~', '%', '^', ';', '$', '!', ':', '?', '\'', '\"', '`','.',
	'*', '^', ';', '$', '!', ':', '?','*',
};

const char	ChTbl_Virtual[]=
{
	'.', '-', '_', ',', '@', '\\', '/', '+', '|', '~', '%', '^', ';', '$', '*', '#', '!', ':', '?', '\'', '\"', '`','.',
};


extern int gnDispFontSize;
extern int gnDispLineSpacing ;
extern int gnDispLineHeight ;


static char SwitchChar( char ch);
static int InsChaToStr(char c, uint unOffset, char* pszStr);
static int InsChaToAmt(char c, uint unOffset, char* pszAmt, int *pnAddLen);
static int DelChaFromStr(char* pszStr, uint unOffset);
static int DelChaFromAmt(char* pszAmt, uint unOffset,  int *pnSubLen);
static int AmttoStr(char *psAmt,char *psStr);
static int StrToAmt(char *psStr, char *psAmt);

static void InitIptInfo(int nLineno, int nClumno, char *pszDispStr, int nMode);
static void SetIptCursor(int x, int y);
static void InsIptStr(char c);
static void ShapIptStr(void);
static void DelIptStr(void);
static void UpDateIptDispBuf(void);
static void GetIptStr(char *pszGetStr, int *pmGetLen);
static void ShowIptDispLBuf(int nLine);
static void ShowIptDispBuf(void);

static void DrawCursor(int x, int y);

static char SwitchChar( char ch)
{
	int i;
	int num;

	if (PubGetKbAttr() == KB_VIRTUAL) {
		num = sizeof(ChTbl_Virtual);
		for(i=0;i<num;i++)
		{
			if(ChTbl_Virtual[i] == ch)
				return ChTbl_Virtual[i+1];
		}
	} else {
		num = sizeof(ChTbl);
		for(i=0;i<num;i++)
		{
			if(ChTbl[i] == ch)
				return ChTbl[i+1];
		}
	}

	return ch;
}


/**
* @brief insert a specific character to a string
* @param [in] char value
* @param [in] offset
* @param [out] string
* @return
*/	
static int InsChaToStr(char c, uint unOffset, char* pszStr)
{
	char szTmp[256] = {0};
	int nStrlen;
	
	nStrlen = strlen(pszStr);
	strcpy(szTmp,pszStr);


	if(unOffset > nStrlen)
		return APP_FAIL;

	else if(unOffset  ==  nStrlen)
	{
		memcpy(pszStr,szTmp,unOffset);
		pszStr[unOffset] = c;
		pszStr[unOffset+1] = 0;
		return APP_SUCC;
	}
	else
	{
		memcpy(pszStr,szTmp,unOffset);
		pszStr[unOffset] = c;
		strcpy( pszStr + unOffset +1,szTmp + unOffset);
		return APP_SUCC;		
	}
	
}

//Insert a char in the end of AMT
static int InsChaToAmt(char c, uint unOffset, char* pszAmt, int *pnAddLen)
{
	char szTmp[256] = {0},szStr[256]={0};
	int nStrlen;
	
	nStrlen = strlen(pszAmt);

	if(unOffset > nStrlen)
		return APP_FAIL;
	
	AmttoStr(pszAmt,szStr);
	szStr[strlen(szStr)] = c;
	
	StrToAmt(szStr, szTmp);

	strcpy(pszAmt , szTmp);
	pszAmt[strlen(szTmp)] = 0;
	*pnAddLen = strlen(pszAmt) - nStrlen;
	return APP_SUCC;		
}

static int DelChaFromStr(char* pszStr, uint unOffset)
{
	char szTmp[256] = {0};
	int nStrlen;
	nStrlen = strlen(pszStr);

	if((unOffset >= nStrlen) || (nStrlen ==0))
		return APP_FAIL;

	strcpy(szTmp,pszStr + unOffset + 1);
	strcpy(pszStr + unOffset , szTmp);
	pszStr[nStrlen-1] = 0;
	return APP_SUCC;	
}


//"12,345,678" to "12345678"
//or "123456.78" to "12345678" 
static int AmttoStr(char *psAmt,char *psStr)
{
	int len ,i;
	int nOffset = 0;
	int flag = 0;
	char szTmp[128]={0};

	//remove currency name first
	strcpy(szTmp,psAmt+strlen(CURRENCY_NAME));

	len = strlen(szTmp);
	for(i=0; i<len; i++)
	{
		if((szTmp[i] =='0') && (flag ==0))
			continue;
		else if(szTmp[i]==AMT_SYMBOL)
			continue;
		else{
			psStr[nOffset++] = szTmp[i];
			flag = 1;
		}
	}

	psStr[nOffset] = 0;
	return APP_SUCC;	
}



//"12345678" to "12,345,678"
//or "12345678" to "123456.78"
static int StrToAmt(char *psStr, char *psAmt)
{
#ifdef AMT_TYPE_POINT
	int num;
	char szTmp[128] = {0};
	num = strlen(psStr);

	if(num <3 ){
		strcpy(szTmp, "000");
		strcpy(szTmp+ 3-num ,psStr);
		num = 3;
	}
	else
		strcpy(szTmp, psStr);

	InsChaToStr(AMT_SYMBOL, num-2, szTmp);

#else
	int num ,nBigNum , nDotNum,i;
	char szTmp[128] = {0};
	
	num = strlen(psStr);
	if(num == 0){
		strcpy(psAmt,CURRENCY_NAME);
		return APP_SUCC;
	}
	
	nBigNum = (num-1)%3+1;
	nDotNum =(num-1)/3;

	strcpy(szTmp,psStr);
	for(i=nDotNum;i>=1;i--)
	{
		InsChaToStr(AMT_SYMBOL, nBigNum + (i-1)*3, szTmp);
	}
	
#endif	
	//merge currency name with amt
	strcpy(psAmt,CURRENCY_NAME);
	strcat(psAmt,szTmp);
	return APP_SUCC;
}

//delete the last char of AMT
static int DelChaFromAmt(char* pszAmt, uint unOffset, int *pnSubLen)
{
	char szTmp[256] = {0},szStr[256]={0};
	int nStrlen;
	nStrlen = strlen(pszAmt);
	if((unOffset > nStrlen) || (nStrlen ==0))
		return APP_FAIL;

	AmttoStr(pszAmt,szStr);
	if(strlen(szStr)==0)
		return APP_SUCC;

	szStr[strlen(szStr)-1] = 0;
	StrToAmt(szStr, szTmp);

	strcpy(pszAmt , szTmp);
	pszAmt[strlen(szTmp)] = 0;
	*pnSubLen = nStrlen - strlen(pszAmt);
	return APP_SUCC;
}



static int AmtOffsetToX(char *pszAmt, int nOffset)
{
	int  nStartX;
	uint unSWidth, unLWidth;

	GetLcdWidth(&unLWidth);
	if(strlen(pszAmt) == 0)
		unSWidth = 0;
	else
 		NAPI_ScrGetTrueTypeFontSize(pszAmt, strlen(pszAmt), (int *)&unSWidth, NULL);
	
	nStartX = (unLWidth - unSWidth)/2;	

	if(nOffset==0)
		return nStartX;
	
	NAPI_ScrGetTrueTypeFontSize(pszAmt, nOffset, (int *)&unSWidth, NULL);

	//printf("AMT:%s unSWidth:%d nStartX:%d\r\n ",pszAmt, unSWidth, nStartX);
	return (unSWidth + nStartX);
}

static void InitIptInfo(int nLineno, int nClumno, char *pszDispStr, int nMode)
{
	uint unX, unY, unLHeight, unLWidth;
	if(nLineno < 1)
	{
		nLineno = 1;
	}
	if(nClumno < 1)
	{
		nClumno = 1;
	}	

	NAPI_ScrGetViewPort(&unX, &unY, &unLWidth, &unLHeight);	
	memset(&gstIptInfo,0,sizeof(ST_INPUTDISP));

	gstIptInfo.nMode = nMode;
	gstIptInfo.nLcdHeight = unLHeight;
	gstIptInfo.nLcdWidth = unLWidth;
	gstIptInfo.nStartLine = nLineno;
	gstIptInfo.nStartY = (nLineno-1)*gnDispLineHeight+ gnDispLineSpacing/2;
	strcpy(gstIptInfo.szGetBuf, pszDispStr);
	gstIptInfo.nGetNum = strlen(pszDispStr);
	
	//if(nMode == INPUT_MODE_PASSWD)
	if(nMode == INPUT_MODE_PASSWD || gstIptInfo.nMode == INPUT_MODE_SECURITYCODE || gstIptInfo.nMode == INPUT_MODE_SECURITYCODE2)
	{
		memset(gstIptInfo.szTmp, '*', gstIptInfo.nGetNum);
		gstIptInfo.nStartX = (nClumno-1)*gnDispFontSize;//Cursor X of first line
		gstIptInfo.pcShow = gstIptInfo.szTmp;

	}
	else if(nMode == INPUT_MODE_AMOUNT)//Show at middle of line, only sypport single line displaying 
	{
		StrToAmt(pszDispStr, gstIptInfo.szTmp);
		gstIptInfo.nStartX = AmtOffsetToX(gstIptInfo.szTmp, 0);
		gstIptInfo.pcShow = gstIptInfo.szTmp;
	}
	else
	{
		gstIptInfo.nStartX = (nClumno-1)*gnDispFontSize;//Cursor X of first line
		gstIptInfo.pcShow = gstIptInfo.szGetBuf;
	}
	
	UpDateIptDispBuf();


	gstIptInfo.nCurOffset = strlen(gstIptInfo.pcShow);
	gstIptInfo.nCursorLine = gstIptInfo.nOrgCursorLine = gstIptInfo.nDispLine - 1;
	gstIptInfo.nCursorX = gstIptInfo.snLEndX[gstIptInfo.nCursorLine];
	gstIptInfo.nCursorY = gstIptInfo.nEndY - gnDispLineHeight;	
	gstIptInfo.nIfStrUndate = 1;

	//printf("nDispLine:%d nCursorLine:%d",gstIptInfo.nDispLine ,gstIptInfo.nCursorLine);

}

__attribute__((unused)) static void SetIptCursor(int x, int y)
{
	int nLine;
	int i, nStrLen;
	uint unSWidth;
	int  nStartX;
	if( (y<gstIptInfo.nStartY) || (y > gstIptInfo.nEndY))
		return;

	nLine = (y-gstIptInfo.nStartY)/gnDispLineHeight;

	if(nLine == 0)
		nStartX = gstIptInfo.nStartX;
	else
		nStartX = 0;
	
	nStrLen = strlen(gstIptInfo.szDispBuf[nLine]);
	if(nStrLen<=0)
		return;

	gstIptInfo.nOrgCursorLine = gstIptInfo.nCursorLine;
	gstIptInfo.nCursorLine = nLine;
	gstIptInfo.nCursorY = gstIptInfo.nStartY + nLine*gnDispLineHeight;
	for(i = nStrLen; i> 0; i--)
	{
		NAPI_ScrGetTrueTypeFontSize(gstIptInfo.szDispBuf[nLine], i, (int *)&unSWidth, NULL);
		if((unSWidth + nStartX) < x)
		{
			gstIptInfo.nCurOffset = gstIptInfo.snLOffset[nLine] + i;
			gstIptInfo.nCursorX = nStartX + unSWidth;
			break;
		}
	}	

	if(i==0)
	{
		gstIptInfo.nCurOffset = gstIptInfo.snLOffset[nLine];
		gstIptInfo.nCursorX = nStartX;
	}

	//printf(">nTouchOffset:%d CursorX:%d Y:%d\r\n",nTouchOffset ,nCursorX, nCursorY);
	return;	
}


static void InsIptStr(char c)
{
	uint unChaWidth = 0;
	int  nAddLen = 0;
	if(gstIptInfo.nMode == INPUT_MODE_AMOUNT)
	{
		InsChaToAmt(c, gstIptInfo.nCurOffset , gstIptInfo.szTmp, &nAddLen);
		AmttoStr(gstIptInfo.szTmp, gstIptInfo.szGetBuf);
		gstIptInfo.nCurOffset += nAddLen;
	}
	//else if(gstIptInfo.nMode == INPUT_MODE_PASSWD)
	else if(gstIptInfo.nMode == INPUT_MODE_PASSWD || gstIptInfo.nMode == INPUT_MODE_SECURITYCODE || gstIptInfo.nMode == INPUT_MODE_SECURITYCODE2)
	{
		InsChaToStr(c, gstIptInfo.nCurOffset , gstIptInfo.szGetBuf);
		gstIptInfo.szTmp[gstIptInfo.nGetNum] = '*';
		unChaWidth = GetChaWidth('*');
		gstIptInfo.nCurOffset++;
	}	
	else
	{
		InsChaToStr(c, gstIptInfo.nCurOffset , gstIptInfo.szGetBuf);
		unChaWidth = GetChaWidth(c);
		gstIptInfo.nCurOffset++;
	}
	
	
	gstIptInfo.nGetNum++;
	
	UpDateIptDispBuf();
	
	if(gstIptInfo.nMode == INPUT_MODE_AMOUNT)
	{
		gstIptInfo.nCursorX = AmtOffsetToX(gstIptInfo.szTmp, gstIptInfo.nCurOffset);
	}
	else
	{
		if(unChaWidth + gstIptInfo.nCursorX > gstIptInfo.nLcdWidth)//line feed
		{
			gstIptInfo.nCursorX = unChaWidth;
			gstIptInfo.nCursorY += gnDispLineHeight;
			gstIptInfo.nOrgCursorLine = gstIptInfo.nCursorLine;
			gstIptInfo.nCursorLine++;
		}
		else
		{
			gstIptInfo.nCursorX += unChaWidth; 
		}
	}

}

static void ShapIptStr(void)
{
	uint unOrgChaWidth, unNewChaWidth;
	char cOrgCha, cNewCha; 
	int  nDifWidth;

	if((gstIptInfo.nCurOffset == 0) || (gstIptInfo.nGetNum==0))
		return;
	
	cOrgCha = gstIptInfo.szGetBuf[gstIptInfo.nCurOffset-1];
	cNewCha = SwitchChar(cOrgCha);

	if(cOrgCha == cNewCha)
		return;

	gstIptInfo.szGetBuf[gstIptInfo.nCurOffset-1] = cNewCha;
	UpDateIptDispBuf();
	
	unOrgChaWidth = GetChaWidth(cOrgCha);
	unNewChaWidth = GetChaWidth(cNewCha);

	if(unOrgChaWidth == unNewChaWidth)
		return;

	nDifWidth =unNewChaWidth -  unOrgChaWidth;
	if(nDifWidth <=0)
	{
		gstIptInfo.nCursorX += nDifWidth;
	}
	else
	{
		if(nDifWidth + gstIptInfo.nCursorX > gstIptInfo.nLcdWidth)
		{
			gstIptInfo.nCursorX = unNewChaWidth;
			gstIptInfo.nCursorY += gnDispLineHeight;
			gstIptInfo.nOrgCursorLine = gstIptInfo.nCursorLine;
			gstIptInfo.nCursorLine++;
		}
		else
		{
			gstIptInfo.nCursorX += nDifWidth; 
		}

	}
}
static void DelIptStr(void)
{
	uint unChaWidth = 0;
	char c;
	int nSubLen = 0;
	if((gstIptInfo.nCurOffset == 0) || (gstIptInfo.nGetNum==0))
		return;	
	
	c = gstIptInfo.szGetBuf[gstIptInfo.nCurOffset-1];
	

	if(gstIptInfo.nMode == INPUT_MODE_AMOUNT)
	{
		DelChaFromAmt(gstIptInfo.szTmp, gstIptInfo.nCurOffset-1, &nSubLen);
		AmttoStr(gstIptInfo.szTmp, gstIptInfo.szGetBuf);
		gstIptInfo.nCurOffset -= nSubLen;
	}
	//else if(gstIptInfo.nMode == INPUT_MODE_PASSWD)
	else if(gstIptInfo.nMode == INPUT_MODE_PASSWD || gstIptInfo.nMode == INPUT_MODE_SECURITYCODE || gstIptInfo.nMode == INPUT_MODE_SECURITYCODE2)
	{
		DelChaFromStr(gstIptInfo.szGetBuf, gstIptInfo.nCurOffset-1);
		gstIptInfo.szTmp[gstIptInfo.nGetNum-1] = 0;
		unChaWidth = GetChaWidth('*');
		gstIptInfo.nCurOffset--;
	}	
	else
	{
		DelChaFromStr(gstIptInfo.szGetBuf, gstIptInfo.nCurOffset-1);
		unChaWidth = GetChaWidth(c);
		gstIptInfo.nCurOffset--;
	}
	
	
	gstIptInfo.nGetNum--;


	if(gstIptInfo.nMode == INPUT_MODE_AMOUNT)
	{
		gstIptInfo.nCursorX = AmtOffsetToX(gstIptInfo.szTmp, gstIptInfo.nCurOffset);
		//gstIptInfo.nCursorX -= unChaWidth; 
	}
	else
	{
		if(gstIptInfo.nCursorX >unChaWidth )// such as:123|
		{
			gstIptInfo.nCursorX -= unChaWidth;
		}
		else if(gstIptInfo.nCursorX == unChaWidth)//such as:1|
		{
			if(gstIptInfo.nCursorLine == 0)
			{
				gstIptInfo.nCursorX = 0;
			}
			else
			{
				gstIptInfo.nCursorX = gstIptInfo.snLEndX[gstIptInfo.nCursorLine - 1];
				gstIptInfo.nCursorY -= gnDispLineHeight;
				gstIptInfo.nCursorLine--;		
			}
		}
		else//such as:|
		{
			if(gstIptInfo.nCursorLine == 0)
			{
			}			
			else
			{
				gstIptInfo.nCursorX = gstIptInfo.snLEndX[gstIptInfo.nCursorLine - 1] - unChaWidth;
				gstIptInfo.nCursorY -= gnDispLineHeight;
				gstIptInfo.nCursorLine--;
			}	
		}
	}
	UpDateIptDispBuf();
		
}

static void UpDateIptDispBuf(void)
{
	int		nLeftLen, nTrimedLen ,nTmp;
	int 	x,y;
	int 	i;
	uint 	unSWidth;

	if(gstIptInfo.nMode != INPUT_MODE_AMOUNT)
	{
		memset(&gstIptInfo.szDispBuf,0,sizeof(gstIptInfo.szDispBuf));
		gstIptInfo.nOrgLine = gstIptInfo.nDispLine;
		gstIptInfo.nDispLine = 0;

		x = gstIptInfo.nStartX;//first line start cursor
		y = gstIptInfo.nStartY;
		nLeftLen = gstIptInfo.nGetNum;
		nTrimedLen = 0;
		
		for(i=0;;i++)
		{	
			if(i)//Line cursor start from 0 except first line
				x =0;
			
			TrimStrToOneLine(x, gstIptInfo.pcShow + nTrimedLen, nLeftLen, &nTmp, &gstIptInfo.snLEndX[i]);
			memcpy(gstIptInfo.szDispBuf[i], gstIptInfo.pcShow + nTrimedLen,nTmp);
			gstIptInfo.snLOffset[i] = nTrimedLen;
			y += gnDispLineHeight;
			nTrimedLen += nTmp;
			nLeftLen -= nTmp;
			gstIptInfo.nDispLine++;

			if(nLeftLen <= 0)
			{
				break;
			}

			if( y>= gstIptInfo.nLcdHeight)
			{
				break;
			}			
		}

		gstIptInfo.nEndY = y;
		
	}
	else
	{
		gstIptInfo.nOrgLine = gstIptInfo.nDispLine = 1;
		memset(gstIptInfo.szDispBuf[0],0,sizeof(gstIptInfo.szDispBuf[0]));
		strcpy(gstIptInfo.szDispBuf[0], gstIptInfo.szTmp);
		unSWidth = GetStrWidth(gstIptInfo.szDispBuf[0]);
		gstIptInfo.nStartX = (gstIptInfo.nLcdWidth - unSWidth)/2;	
		if(gstIptInfo.nStartX < 0)
		{
			gstIptInfo.nStartX =0;
			gstIptInfo.snLEndX[0] =  gstIptInfo.nLcdWidth;
		}
		else
		{
			gstIptInfo.snLEndX[0] = gstIptInfo.nStartX  + unSWidth;
		}
		
		gstIptInfo.nEndY   = gstIptInfo.nStartY + gnDispLineHeight;
	}
	gstIptInfo.nIfStrUndate = 1;
}


static void GetIptStr(char *pszGetStr, int *pmGetLen)
{
	memcpy(pszGetStr, gstIptInfo.szGetBuf, gstIptInfo.nGetNum);
	*pmGetLen = gstIptInfo.nGetNum;
	pszGetStr[gstIptInfo.nGetNum] = 0;
}

static void DrawCursor(int x, int y)
{
//	if(x == 0)
//		x = 1;
	NAPI_ScrLine(x, y ,x, y+gnDispFontSize, 0);
}


static void ShowIptDispLBuf(int nLine)
{
	int x ,y;
	if(nLine == 0)
		x = gstIptInfo.nStartX;
	else
		x = 0;

	y = gstIptInfo.nStartY + nLine*gnDispLineHeight;

	NAPI_ScrDispTrueTypeFontText(x ,y , gstIptInfo.szDispBuf[nLine], strlen(gstIptInfo.szDispBuf[nLine]));
}

static void ShowIptDispBuf(void)
{
	int 	i;
	int 	nLineno;


	if(gstIptInfo.nIfStrUndate)//Redisplay all line
	{
		nLineno = (gstIptInfo.nOrgLine > gstIptInfo.nDispLine) ? gstIptInfo.nOrgLine:gstIptInfo.nDispLine;
		PubClearLine( gstIptInfo.nStartLine, gstIptInfo.nStartLine + nLineno -1);
		for(i=0; i<gstIptInfo.nDispLine;i++ )
		{	
			ShowIptDispLBuf(i);
		}
	}
	else//only redisplay lines where new cursor and old cursor stay
	{
		PubClearLine( gstIptInfo.nStartLine + gstIptInfo.nCursorLine, gstIptInfo.nStartLine + gstIptInfo.nCursorLine);
		ShowIptDispLBuf( gstIptInfo.nCursorLine);
		if(gstIptInfo.nOrgCursorLine != gstIptInfo.nCursorLine)
		{
			PubClearLine( gstIptInfo.nStartLine + gstIptInfo.nOrgCursorLine, gstIptInfo.nStartLine + gstIptInfo.nOrgCursorLine);
			ShowIptDispLBuf( gstIptInfo.nOrgCursorLine);
		}
	}

	if(gstIptInfo.nMode != INPUT_MODE_AMOUNT){//AMT do not need cursor
		if(gstIptInfo.nEditFlag)//cursor twinkle
		{
			DrawCursor(gstIptInfo.nCursorX, gstIptInfo.nCursorY);
			gstIptInfo.nEditFlag = 0;
		}
		else
		{
			gstIptInfo.nEditFlag = 1;
		}
	}
	gstIptInfo.nIfStrUndate = 0;
}

int Inputxy(int nClumno, int nLineno, char* pszOut, int* pnOutLen, int nMinLen, int nMaxLen,
	int nTimeOut, int nEditMask)
{
	int		nKey;
	int     nNum;
	int		nShowChange = 1;
	ST_PADDATA stPaddata;
	int nTimeCount = 0;

	nNum = strlen(pszOut);
	if (nNum > nMaxLen)
	{
		return APP_FAIL;
	}

	if (PubGetKbAttr() == KB_VIRTUAL)
	{
		Virtual_KbCreate(NULL, nEditMask);
	}
	else
	{
		if (nEditMask == INPUT_MODE_AMOUNT)
		{
			PubDisplayTail(0, DISPLAY_MODE_CLEARLINE , 0, "Clear Key To Remove");
		}
	}

	InitIptInfo(nLineno, nClumno, pszOut, nEditMask);
	NAPI_KbHit(&nKey);
	for (;;)
	{
		if (nShowChange == 1)
		{
			ShowIptDispBuf();
		}
		nShowChange = 1;
		NAPI_ScrRefresh();
		if (PubGetKbAttr() == KB_VIRTUAL)
		{
			nKey = Virtual_KbGetCode(nTimeOut);
			if (nKey == APP_TIMEOUT) {
				nKey = 0;
			}
		}
		else
		{
			if(NAPI_KbGetPad(&stPaddata, 500) == NAPI_ERR_TIMEOUT)
			{
				if(nTimeOut > 0)
				{
					nTimeCount += 500;
					if(nTimeCount/1000 > nTimeOut)
						return APP_TIMEOUT;
				}
				continue;
			}
			nTimeCount = 0;

			if(stPaddata.emPadState == PADSTATE_KEY)
			{
				nKey = stPaddata.unKeycode;			
				
			}
			else if(stPaddata.emPadState == PADSTATE_DOWN)
			{
				if(gstIptInfo.nMode != INPUT_MODE_AMOUNT)//AMT do not support touch screen
					SetIptCursor(stPaddata.unPadX, stPaddata.unPadY);
				continue;
			}
		}

		switch (nKey)
		{
		case 0:
			return APP_TIMEOUT;
		case KEY_SHARP:
			/*If letter key is pressed in INPUT_MODE_STRING mode*/
			if (nEditMask == INPUT_MODE_STRING)
			{
				ShapIptStr();
			}
			break;
		case KEY_STAR:
			/*'.' is not allowed in non-INPUT_MODE_STRING mode*/
			if (nEditMask != INPUT_MODE_STRING && nEditMask != INPUT_MODE_NUMBERPOINT)
			{
				break;
			}
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
			if((nKey == KEY_0) && (nEditMask == INPUT_MODE_AMOUNT) && (gstIptInfo.nCurOffset == 0))/*Amount can not begin with '0'*/
			{
				break;
			}
			if (gstIptInfo.nGetNum >= nMaxLen)
			{
				break;
			}
			InsIptStr(nKey);
			break;
		case KEY_BACK:
			DelIptStr();
			break;
		case KEY_ENTER:
			if ((gstIptInfo.nGetNum >= nMinLen) && (gstIptInfo.nGetNum <= nMaxLen))
			{
					if (nEditMask == INPUT_MODE_SECURITYCODE || nEditMask == INPUT_MODE_SECURITYCODE2)
					{
						if(nEditMask==INPUT_MODE_SECURITYCODE){
								if(gstIptInfo.nGetNum>nMinLen && gstIptInfo.nGetNum<nMaxLen){
								break;
							}
						}else{
								if(gstIptInfo.nGetNum>nMinLen && gstIptInfo.nGetNum<nMaxLen-1){
								break;
							}
						}
						
					}
				GetIptStr(pszOut, pnOutLen);
				return APP_SUCC;
			}
			break;
		case KEY_ESC:
			return APP_QUIT;
		case KEY_UP:
		case KEY_DOWN:
			if (UI_ChkIsUpDownMenu())
			{
			      return nKey;
			}
			nShowChange = 0;
			break;	
		default:
			if (PubGetKbAttr() == KB_PHYSICAL) {
				nShowChange = 0;
				break;
			}
			// for virtual keyboard
			if (nEditMask == INPUT_MODE_STRING)
			{
				if ((nKey >= KEY_A && nKey <= KEY_Z) ||
					(nKey >= KEY_a && nKey <= KEY_z) ||
					nKey == KEY_RIGHTPRNTHS || nKey == KEY_LEFTPRNTHS || nKey == KEY_SEMICOLON ||
					nKey == KEY_COMMA || nKey == KEY_DOT || nKey == KEY_DIAGONAL)
				{
					if (gstIptInfo.nGetNum >= nMaxLen)
					{
						break;
					}
					InsIptStr(nKey);
				}
			}
			break;
		}
		
	}
	return APP_SUCC;
}

int ProInputDlg(const char *pszTitle, const char *pszContent, const char* psInfo, 
                        int nClumno, int nLineno, char *pszOut, int *pnOutLen,int nMinLen, 
                        int nMaxLen, int nTimeOut, int nEditMask)
{
	uint unWidth, unHeight;

	if (pszOut == NULL || pnOutLen == NULL)
	{
		return APP_FAIL;
	}
	NAPI_ScrGetFontSize(&unWidth, &unHeight);
	PubClearAll();
	if (pszTitle != NULL)
	{
		PubDisplayTitle((char*)pszTitle);
	}
	if (pszContent != NULL && strlen(pszContent) != 0)
	{
		PubDisplayStr(DISPLAY_MODE_CLEARLINE, 2, 1, (char*)pszContent);
	}
    if (psInfo != NULL)
    {
    	PubDisplayStr(DISPLAY_MODE_CLEARLINE, nLineno+2,1,  (char*)psInfo);
    }
	return Inputxy (1, nLineno, pszOut, pnOutLen, nMinLen, nMaxLen, nTimeOut, nEditMask);
}

int ProInputDlgWithAmount(const char *amount,const char *pszTitle, const char *pszContent, const char* psInfo, 
                        int nClumno, int nLineno, char *pszOut, int *pnOutLen,int nMinLen, 
                        int nMaxLen, int nTimeOut, int nEditMask)
{
	uint unWidth, unHeight;

	if (pszOut == NULL || pnOutLen == NULL)
	{
		return APP_FAIL;
	}
	NAPI_ScrGetFontSize(&unWidth, &unHeight);
	PubClearAll();
	if (amount != NULL)
	{
		PubDisplayTitle((char*)amount);
	}
	if (pszTitle != NULL)
	{
		PubDisplayStr(DISPLAY_MODE_CLEARLINE, 2, 1, (char*)pszTitle);
	}
	if (pszContent != NULL && strlen(pszContent) != 0)
	{
		PubDisplayStr(DISPLAY_MODE_CLEARLINE, 3, 1, (char*)pszContent);
	}
    if (psInfo != NULL)
    {
    	PubDisplayStr(DISPLAY_MODE_CLEARLINE, nLineno+2,1,  (char*)psInfo);
    }
	return Inputxy (nClumno, nLineno, pszOut, pnOutLen, nMinLen, nMaxLen, nTimeOut, nEditMask);
}


int InputDlg(const char *pszTitle, const char *pszContent, char *pszOut, int *pnOutLen,
	int nMinLen, int nMaxLen, int nTimeOut, int nEditMask)
{
		return ProInputDlg(pszTitle, pszContent, NULL, \
                    1, 3, pszOut, pnOutLen, nMinLen,nMaxLen,nTimeOut,nEditMask);
}





int ProInputDate(const char *pszTitle, const char *pszContent, const char * pStr, \
                int nClumno, int nLineno, char *pszDate, int nFormat, int nTimeOut)
{
	int			nMinLen = 0;
	int			nMaxLen = 0;
	int			nRet = 0;
	int			nOutLen = 0;
	char		szTempDate[15];
	char		cFlag = 0;
	char tmpDateMMYY[4];

	if (pszDate == NULL)
	{
		return APP_FAIL;
	}
	switch (nFormat)
	{
	case INPUT_DATE_MODE_MMDD:
	case INPUT_DATE_MODE_YYMM:
	case INPUT_DATE_MODE_MMYY:
		nMinLen = 1;
		nMaxLen = 4;
		break;
	case INPUT_DATE_MODE_YYMMDD:
		nMinLen = 1;
		nMaxLen = 6;
		break;
	case INPUT_DATE_MODE_YYYYMMDD:
		nMinLen = 1;
		nMaxLen = 8;
		break;
	case INPUT_DATE_MODE_MMDD_NULL:
	case INPUT_DATE_MODE_YYMM_NULL:
	case INPUT_DATE_MODE_MMYY_NULL:
		nMinLen = 0;
		nMaxLen = 4;
		break;
	case INPUT_DATE_MODE_YYMMDD_NULL:
		nMinLen = 0;
		nMaxLen = 6;
		break;
	case INPUT_DATE_MODE_YYYYMMDD_NULL:
		nMinLen = 0;
		nMaxLen = 8;
		break;
	default:
		PubMsgDlg("Date input", "\nInvalid date input", 3, 3);
		return APP_FAIL;
	}
	for (; ;)
	{
        nRet = ProInputDlg(pszTitle, pszContent, pStr, nClumno, nLineno, pszDate, &nOutLen, nMinLen, nMaxLen, nTimeOut, INPUT_MODE_NUMBER);
		if (nRet != APP_SUCC)
		{
			return nRet;
		}

		if ( 0 == nOutLen )
		{
			return APP_SUCC;
		}

		memset ( szTempDate, 0, sizeof(szTempDate) );
		PubGetCurrentDatetime(szTempDate);
		switch ( nFormat )
		{
		case INPUT_DATE_MODE_MMDD:
		case INPUT_DATE_MODE_MMDD_NULL:
			if ( nOutLen != 4 )
			{
				cFlag = 1;
				break;
			}
			memcpy( szTempDate + 4, pszDate, strlen(pszDate) );
			szTempDate[8] = '\0';
			break;
		case INPUT_DATE_MODE_MMYY:
      	case INPUT_DATE_MODE_MMYY_NULL:
			if (nOutLen != 4) {
			cFlag = 1;
			break;
			}
			memcpy(tmpDateMMYY, pszDate + 2, 2);
			memcpy(tmpDateMMYY + 2, pszDate, 2);
			strcpy(szTempDate + 6, "01");
			memcpy(szTempDate + 2, tmpDateMMYY, strlen(tmpDateMMYY) - 1);
			szTempDate[8] = '\0';

			break;
		case INPUT_DATE_MODE_YYMM:
		case INPUT_DATE_MODE_YYMM_NULL:
			if ( nOutLen != 4 )
			{
				cFlag = 1;
				break;
			}

			strcpy(szTempDate + 6, "01");
			memcpy( szTempDate +2, pszDate , strlen(pszDate) );
			szTempDate[8] = '\0';
			break;
		case INPUT_DATE_MODE_YYMMDD:
		case INPUT_DATE_MODE_YYMMDD_NULL:
			if ( nOutLen != 6 )
			{
				cFlag = 1;
				break;
			}
			memcpy( szTempDate +2, pszDate, strlen(pszDate));
			szTempDate[8] = '\0';
			break;
		case INPUT_DATE_MODE_YYYYMMDD:
		case INPUT_DATE_MODE_YYYYMMDD_NULL:
			if ( nOutLen != 8 )
			{
				cFlag = 1;
				break;
			}
			memset( szTempDate, 0, sizeof(szTempDate) );
			memcpy( szTempDate, pszDate, strlen(pszDate) );
			szTempDate[8] = '\0';
			break;
		}
		if ( 1 == cFlag )
		{
			PubMsgDlg("FECHA INVALIDA", " ", 3, 3);
			memset(pszDate,0x00,strlen(pszDate));
			cFlag = 0;
			continue;
		}

		if (PubIsValidDate(szTempDate) != APP_SUCC )
		{
			PubMsgDlg("FECHA INVALIDA"," ", 3, 3);
			memset(pszDate,0x00,strlen(pszDate));
			continue;
		}
		return APP_SUCC;
	}
	return APP_SUCC;
}


int ProInputDateWithAmount(const char *amount, const char *pszTitle, const char *pszContent, const char * pStr, \
                int nClumno, int nLineno, char *pszDate, int nFormat, int nTimeOut)
{
	int			nMinLen = 0;
	int			nMaxLen = 0;
	int			nRet = 0;
	int			nOutLen = 0;
	char		szTempDate[15];
	char		cFlag = 0;
	char tmpDateMMYY[4];

	if (pszDate == NULL)
	{
		return APP_FAIL;
	}
	switch (nFormat)
	{
	case INPUT_DATE_MODE_MMDD:
	case INPUT_DATE_MODE_YYMM:
	case INPUT_DATE_MODE_MMYY:
		nMinLen = 1;
		nMaxLen = 4;
		break;
	case INPUT_DATE_MODE_YYMMDD:
		nMinLen = 1;
		nMaxLen = 6;
		break;
	case INPUT_DATE_MODE_YYYYMMDD:
		nMinLen = 1;
		nMaxLen = 8;
		break;
	case INPUT_DATE_MODE_MMDD_NULL:
	case INPUT_DATE_MODE_YYMM_NULL:
	case INPUT_DATE_MODE_MMYY_NULL:
		nMinLen = 0;
		nMaxLen = 4;
		break;
	case INPUT_DATE_MODE_YYMMDD_NULL:
		nMinLen = 0;
		nMaxLen = 6;
		break;
	case INPUT_DATE_MODE_YYYYMMDD_NULL:
		nMinLen = 0;
		nMaxLen = 8;
		break;
	default:
		PubMsgDlg("Date input", "\nInvalid date input", 3, 3);
		return APP_FAIL;
	}
	for (; ;)
	{
        nRet = ProInputDlgWithAmount(amount ,pszTitle, pszContent, pStr, nClumno, nLineno, pszDate, &nOutLen, nMinLen, nMaxLen, nTimeOut, INPUT_MODE_NUMBER);
		if (nRet != APP_SUCC)
		{
			return nRet;
		}

		/*if ( 0 == nOutLen )
		{
			return APP_SUCC;
		}*/

		memset ( szTempDate, 0, sizeof(szTempDate) );
		PubGetCurrentDatetime(szTempDate);
		switch ( nFormat )
		{
		case INPUT_DATE_MODE_MMDD:
		case INPUT_DATE_MODE_MMDD_NULL:
			if ( nOutLen != 4 )
			{
				cFlag = 1;
				break;
			}
			memcpy( szTempDate + 4, pszDate, strlen(pszDate) );
			szTempDate[8] = '\0';
			break;
		case INPUT_DATE_MODE_MMYY:
      	case INPUT_DATE_MODE_MMYY_NULL:
			if (nOutLen != 4) {
			cFlag = 1;
			break;
			}
			memcpy(tmpDateMMYY, pszDate + 2, 2);
			memcpy(tmpDateMMYY + 2, pszDate, 2);
			strcpy(szTempDate + 6, "01");
			memcpy(szTempDate + 2, tmpDateMMYY, strlen(tmpDateMMYY) - 1);
			szTempDate[8] = '\0';

			break;
		case INPUT_DATE_MODE_YYMM:
		case INPUT_DATE_MODE_YYMM_NULL:
			if ( nOutLen != 4 )
			{
				cFlag = 1;
				break;
			}

			strcpy(szTempDate + 6, "01");
			memcpy( szTempDate +2, pszDate , strlen(pszDate) );
			szTempDate[8] = '\0';
			break;
		case INPUT_DATE_MODE_YYMMDD:
		case INPUT_DATE_MODE_YYMMDD_NULL:
			if ( nOutLen != 6 )
			{
				cFlag = 1;
				break;
			}
			memcpy( szTempDate +2, pszDate, strlen(pszDate));
			szTempDate[8] = '\0';
			break;
		case INPUT_DATE_MODE_YYYYMMDD:
		case INPUT_DATE_MODE_YYYYMMDD_NULL:
			if ( nOutLen != 8 )
			{
				cFlag = 1;
				break;
			}
			memset( szTempDate, 0, sizeof(szTempDate) );
			memcpy( szTempDate, pszDate, strlen(pszDate) );
			szTempDate[8] = '\0';
			break;
		}
		if ( 1 == cFlag )
		{
			PubMsgDlg("FECHA INVALIDA", " ", 3, 3);
			memset(pszDate,0x00,strlen(pszDate));
			cFlag = 0;
			continue;
		}

		if (PubIsValidDate(szTempDate) != APP_SUCC )
		{
			PubMsgDlg("FECHA INVALIDA"," ", 3, 3);
			memset(pszDate,0x00,strlen(pszDate));
			continue;
		}
		return APP_SUCC;
	}
	return APP_SUCC;
}

int InputCNMode(const char *pszTitle, const char *pszContent,char *pszStr, int *pnInputLen, int nMinLen, int nMaxLen, int nImeMode)
{
	return APP_FAIL;
}

int InputIp(const char * pszTitle, const char * pszContent, char * pszOut, int * pnOutLen)
{
	char szTmp[17];
	char szBuf[17];
	int nRet;
	int k;
	int i;
	int j;
	char cFlag=0;	
	int nLen;

	memset(szTmp, 0, sizeof(szTmp));		
	memcpy(szTmp, pszOut, *pnOutLen > 16 ? 16 : *pnOutLen);

	while(1)
	{
		nRet = InputDlg(pszTitle, pszContent, szTmp, &nLen, 7, 15, 60, INPUT_MODE_NUMBERPOINT);
		if (nRet != APP_SUCC)
		{
			return nRet;
		}
		/**<Check if there are 3 dots*/
		for (k=0,i=0;i<nLen;i++)
		{
			if (szTmp[i] == '.') 
			{
				k++;
			}
		}
		if (k != 3) 
		{
			continue;
		}
		
		for (k=0, i=j=0; i<nLen; i++)
		{
			if (szTmp[i] == '.')
			{
				if (((i-j)<1)||((i-j)>3))
				{
					break;
				}
				memset(szBuf, 0, sizeof(szBuf));
				memcpy(szBuf, szTmp + j, i-j);
				if ((atoi(szBuf)<0) || (atoi(szBuf)>255))
				{
					break;
				}
				j = i+1;
				k++;
			}
		}
		if (k == 3)
		{
			if (((nLen - j) < 1) || ((nLen - j) > 3))
			{
				continue;
			}
			memset(szBuf, 0, sizeof(szBuf));
			memcpy(szBuf, szTmp+j, nLen-j);
			if ((atoi(szBuf)<0) || (atoi(szBuf)>255))
			{
				continue;
			}
			cFlag=1;
		}

		if (cFlag)
		{
			break;
		}
	}
	memset(pszOut, 0, *pnOutLen);
	memcpy(pszOut, szTmp, nLen);
	*pnOutLen = nLen;
	return APP_SUCC;
}

int InputPin(const char *pszTitle,const char * pszAmount,const char * pszContent,int nMinLen, int nMaxLen,char *pszOut)
{
	char szPin[20+1];
	int nRet,nPinLen=0;

	PubClearAll();
	
	
	if(pszTitle)
		PubDisplayTitle((char*)pszTitle);
	
	if(pszAmount)
		PubDisplayStrInline(0, 2, (char *)pszAmount);
	if (pszContent == NULL)
	{
		PubDisplayStrInline(0, 3,"Please enter PIN:");
	}
	else
	{
		PubDisplayStrInline(0, 3, (char *)pszContent);
	}
	PubDisplayTail(0, 0, 0 , "[ENTER] to skip");
	
	while(1)
	{
		memset(szPin, 0, sizeof(szPin));
		PubClearLine(4,6);
		
		nRet = PubInputxy(1, 5, szPin, &nPinLen, 0, nMaxLen, 120, INPUT_MODE_PASSWD);
		if (nRet != APP_SUCC)
		{
			return nRet;
		}
		if (nPinLen == 0)
		{
			memset(pszOut,0, 8);
			return APP_SUCC;
		}
		if (nPinLen < nMinLen)
		{
			continue;
		}
		strcpy(pszOut,szPin);
		return APP_SUCC;
	}
}

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/wait.h>
#include "uitool.h"
#include "lui.h"
#include "ltool.h"
#include "ldisp.h"
#include "linput.h"
#include "lmenu.h"
#include "lcomm.h"
#include "libapiinc.h"
#include "virtualpad.h"

#define PUBUIVER			"ALUILB0117053101"	

#define TRACE_UI(fmt,args...) PubDebug("[%s][%s][%d]>>>"""fmt,__FILE__, __FUNCTION__, __LINE__,##args)


typedef struct{
	char szPosType[12+1];
	int nPosType;
}STPOSTYPE;


static STUIFUN_STYLE gstUiFunStyle;

int gnDispFontSize = 24;
int gnDispLineSpacing = 8;
int gnDispLineHeight = 32;

static int gnTmpFontSize = 24;
static int gnTmpLineSpacing = 8;
static int gnTmpLineHeight = 32;

static EM_KEYBOARD_ATTR gemKeyboardAttr = KB_PHYSICAL;

void ProDisplayInv(int nMode, int nLineno, char *pStr);
void ProDisplayInvs(int nAlign, int nMode, int nLineno, char *pstr);
void ProSwitchFontSize(EM_DISPLAY_ALIGN emFont);


/**
* @brief Get UI library version
* @param [out] pszVer Version string(16 bytes)
* @return 
* @li void
* @author Linw
* @date 2012-06-26
*/
void PubGetUIVer(char *pszVer)
{
	if(pszVer != NULL)
	{
		strcpy (pszVer, PUBUIVER);
	}	return;
}

/**
* @brief Get Pos module type(string)
* @return 
* @li NULL Fail
* @li POS module name (string) --Success
* @author Linw
* @date 2013-05-27
*/
char * PubGetPosTypeStr()
{
	char szTmp[64] = {0};
	static char szPosType[64] = {0};
	int nTmpLen = sizeof(szTmp);

	if(strlen(szPosType) > 0)
	{
		return szPosType;
	}
	if (NAPI_SysGetInfo(MODEL, szTmp, &nTmpLen) != NAPI_OK)
	{
		return NULL;
	}
	if(strstr(szTmp, "_dev") != NULL || strstr(szTmp, "_pro") != NULL)
	{
		memcpy(szPosType, szTmp, strlen(szTmp) - 4);
	}
	else
	{
		strcpy(szPosType, szTmp);
	}
	return szPosType;
}

/**
* @brief Initialize UI parameter. It must be called first before using UI library.
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author Linw
* @date 2012-07-12
*/
int PubInitUiParam()
{
	if(NAPI_ScrInitGui() != NAPI_OK)
   	{
	   return APP_FAIL;
   	}
	NAPI_ScrAutoUpdate(0, NULL);
	NAPI_ScrSetAttr(TEXT_ATTRIBUTE_NOBACKCOLOR, NULL);
	NAPI_ScrSetFontColor(0,FONTCOLOR_NORMAL);
	return APP_SUCC;
}


/**
* @brief Set UI menu style
* @return 
* @li void
* @author Linw
* @date 2012-11-28
*/
void PubSetUiStyle(STUIFUN_STYLE stUiFunStyle)
{
	memcpy(&gstUiFunStyle, &stUiFunStyle, sizeof(STUIFUN_STYLE));
}

/**
* @brief Get UI menu style
* @param [out] pstUiFunStyle refer to STUIFUN_STYLE
* @return 
* @li void
* @date 2015-01-09
*/
void PubGetUiStyle(STUIFUN_STYLE *pstUiFunStyle)
{
	if(pstUiFunStyle != NULL)
	{
		memcpy(pstUiFunStyle, &gstUiFunStyle, sizeof(STUIFUN_STYLE));
	}
}

/**
* @brief Clear screen
* @return 
* @li void
*/
void PubClearAll(void)
{
	NAPI_ScrClrs();
	NAPI_ScrSetAttr(TEXT_ATTRIBUTE_NOBACKCOLOR, NULL);
}

/**
* @brief Clear from the second line to the last line
* @return 
* @li void
*/
void PubClear2To4(void)
{
	uint unStartY=0, unX, unY, unLHeight, unLWidth;

	NAPI_ScrGetViewPort(&unX, &unY, &unLWidth, &unLHeight);	
	unStartY = gnDispLineHeight ;
	NAPI_ScrClrLine(unStartY, unLHeight);
	NAPI_ScrGotoxy(0, unStartY);	
}

/**
* @brief Clear appointed lines(large font)
* @param [in] ucStartLine Begin line, from 1
* @param [in] ucEndLine	  End line, last line can use (0xff)
* @return 
* @li void
*/
void PubClearLine(uchar ucStartLine, uchar ucEndLine)
{
	uint unStartY=0, unEndY=0;

	unStartY = gnDispLineHeight*(ucStartLine-1) ;
	unEndY = gnDispLineHeight*ucEndLine;
	NAPI_ScrClrLine(unStartY, unEndY);
	NAPI_ScrGotoxy(0, unStartY);

}

/**
* @brief  Clear appointed lines(small font)
* @param [in] ucStartLine Begin line, from 1
* @param [in] ucEndLine   End line, last line can use (0xff)
* @return 
* @li void
*/
void PubClearLines(uchar ucStartLine, uchar ucEndLine)
{
	uint unStartY=0, unEndY=0;

	unStartY = gnDispLineHeight/2*(ucStartLine-1) ;
	unEndY = gnDispLineHeight/2*ucEndLine;
	NAPI_ScrClrLine(unStartY, unEndY);
	NAPI_ScrGotoxy(0, unStartY);

}

/**
* @brief Get the first key value from buffer and return immediately
* @return 
* @li the key value
*/
int PubKbHit()
{
	int nCode;
	if(NAPI_KbHit(&nCode) != NAPI_OK)
		return APP_FAIL;
	return nCode;
}
 
/**
* @brief Get key value before time is out. Timeout(>0) can be set, set 0 for blocking unless any key is pressed.
* @param [in] nTimeout	
* @return 
* @li key value or 0 when time is out
*/
int PubGetKeyCode(int nTimeout)
{
	int nCode;

	if (PubGetKbAttr() == KB_VIRTUAL) {
		Virtual_KbCreate(NULL, INPUT_MODE_STRING);
		nCode = Virtual_KbGetCode(nTimeout);
	} else {
		NAPI_ScrRefresh();
		NAPI_KbGetCode(nTimeout, &nCode);
	}

	return nCode;
}

/**
* @brief Get touch value before time is out. Timeout(>0) can be set, set 0 for blocking unless any key is pressed.
* @param [in] nTimeout	
* @return 
* @li 0 - any touch or APP_TIMEOUT when time is out
*/
int PubWaitGetKbPad(int nTimeout)
{
	ST_PADDATA stPaddata;

	NAPI_ScrRefresh();
	if (PubGetKbAttr() == KB_VIRTUAL)
	{
		NAPI_KbVppTpFlush();
	}
	while (1) {
		if(NAPI_KbGetPad(&stPaddata, nTimeout*1000) == NAPI_ERR_TIMEOUT)
		{
			return APP_TIMEOUT;
		}

		if (stPaddata.emPadState == PADSTATE_UP) {
			break;
		}
	}

	return APP_SUCC;
}

/** 
* @brief Get touch value before time is out. Timeout(>0) can be set, set 0 for blocking unless any key is pressed.
* @param [in] nTimeout	
* @return KEY_ENTER or KEY_ESC
* @li 
*/
int PubWaitConfirm(int nTimeout)
{
	if (PubGetKbAttr() == KB_VIRTUAL) {
		return PubShowGetKbPad(nTimeout, BUTTON_CONFIRM, "CANCEL", "ENTER", NULL, NULL);
	} else {
		return PubGetKeyCode(nTimeout);
	}
}

/**
* @brief Display string in appointed location(large font,Support Hanzi)
* @details  When nMode is DISPLAY_MODE_CENTER or DISPLAY_MODE_RIGHT, nLineno & nClumno will be ignored
* @param [in] nMode   Show mode---EM_DISPLAY_MODE 
* @param [in] nLineno line, from 1
* @param [in] nClumno Clumn, from 1
* @param [in] pFormat Display format
* @return 
* @li void
*/

void PubDisplayStr(int nMode, int nLineno, int nClumno, char * pFormat,...)
{
	char    szDispStr[500] = {0};
	va_list VarList;
	int x,y;
	uint unX, unY, unLHeight, unLWidth ,unSWidth, unSHeight;

	if(nLineno < 1 )
		nLineno = 1;
	if(nClumno < 1)
		nClumno = 1;
	
	if(!pFormat)
		return;
	
	va_start(VarList, pFormat);
	vsprintf(szDispStr, pFormat, VarList);
	va_end(VarList);

	NAPI_ScrGetViewPort(&unX, &unY, &unLWidth, &unLHeight);	
	
	if(strlen(szDispStr))
		NAPI_ScrGetTrueTypeFontSize(szDispStr, strlen(szDispStr), (int *)&unSWidth, (int *)&unSHeight);
	else
		unSWidth = 0;
	

	if (nMode & DISPLAY_MODE_CLEARLINE)
	{
		int nAddLine =0 ;
		if(unSWidth)//Clear multlines
			nAddLine = (unSWidth-1)/unLWidth;
		
		PubClearLine(nLineno, nLineno + nAddLine);
	}
	
	if(nMode & DISPLAY_MODE_CENTER)
	{
		if(unSWidth < unLWidth)
			x = (unLWidth - unSWidth)/2;
		else
			x = 0;		
		//printf("unSWidth:%d unLWidth:%d x:%d\r\n",unSWidth,unLWidth, x);
	}
	else if(nMode & DISPLAY_MODE_TAIL)
	{
		if(unSWidth < unLWidth)
			x = unLWidth - unSWidth;
		else
			x = 0;
	}
	else
	{
		x = (nClumno-1)*gnDispFontSize;
	}
	if (unSHeight > gnDispLineHeight)
	{
		unSHeight = gnDispLineHeight;
	}
	y = (nLineno-1)*gnDispLineHeight + (gnDispLineHeight - unSHeight)/2;
	NAPI_ScrDispTrueTypeFontText(x ,y , szDispStr, strlen(szDispStr));
	
	if((nMode & DISPLAY_MODE_CENTER))//padding
	{
		NAPI_ScrGotoxy(0, y +  gnDispLineHeight);
	}
	
	return;
}

/**
* @brief Display string in appointed location(small font, English only)
* @details  When nMode is DISPLAY_MODE_CENTER or DISPLAY_MODE_RIGHT, nLineno & nClumno will be ignored
* @param [in] nAlign  Align mode---EM_DISPLAY_ALIGN(colorful machine ignores nAlign)
* @param [in] nMode   Show mode---EM_DISPLAY_MODE
* @param [in] nLineno Line, from 1
* @param [in] nClumno Clumn, from 1
* @param [in] pFormat Display format
* @return 
* @li void
*/
void PubDisplayStrs(int nAlign, int nMode, int nLineno, int nClumno, char * pFormat,...)
{
	va_list VarList;
	char    szDispStr[500] = {0};

	
	va_start(VarList, pFormat);
	vsprintf(szDispStr, pFormat, VarList);
	va_end(VarList);


	if(nAlign == DISPLAY_ALIGN_SMALLFONT)
	{
		ProSwitchFontSize(DISPLAY_ALIGN_SMALLFONT);
	}

	
	PubDisplayStr(nMode, nLineno, nClumno, szDispStr);

	if(nAlign == DISPLAY_ALIGN_SMALLFONT)//restore
	{
		ProSwitchFontSize(DISPLAY_ALIGN_BIGFONT);
	}	

	
}


/**
* @brief Center display string at appointed line(Large font, clear line before displaying)
* @param [in] pStr    Enter string
* @param [in] nLineno Appointed line, from 1
* @return 
* @li void
*/

void PubDisplayGen(int nLineno, char *pStr)
{
	int x,y;
	uint unX, unY, unLHeight, unLWidth ,unSWidth,unSHeight;

	if(pStr == NULL || nLineno < 1)
	{
		return;
	}
	NAPI_ScrGetViewPort(&unX, &unY, &unLWidth, &unLHeight);	
	NAPI_ScrGetTrueTypeFontSize(pStr, strlen(pStr), (int *)&unSWidth, (int *)&unSHeight);

	if(unSWidth < unLWidth)
		x = (unLWidth - unSWidth)/2;
	else
		x = 0;		

	if (unSHeight > gnDispLineHeight)
	{
		unSHeight = gnDispLineHeight;
	}

	y = (nLineno-1)*gnDispLineHeight + (gnDispLineHeight - unSHeight)/2;
	NAPI_ScrDispTrueTypeFontText(x ,y , pStr, strlen(pStr)); 
}	


/**
* @brief Center display string at appointed line(Small font, clear line before displaying)
* @param [in] nAlign  Align mode---EM_DISPLAY_ALIGN(colorful machine ignores nAlign)
* @param [in] pStr    Enter string
* @param [in] nLineno Appointed line, from 1
* @return 
* @li void
*/
void PubDisplayGens(int nAlign,int nLineno, char *pStr)
{
	if(pStr == NULL || nLineno < 1)
	{
		return;
	}

	if(nAlign == DISPLAY_ALIGN_SMALLFONT)
	{
		ProSwitchFontSize(DISPLAY_ALIGN_SMALLFONT);
	}



	PubDisplayGen(nLineno, pStr);

	if(nAlign == DISPLAY_ALIGN_SMALLFONT)//restore
	{
		ProSwitchFontSize(DISPLAY_ALIGN_BIGFONT);
	}	

}

/**
* @brief Inverse diplay string (large font)
* @param [in] pstr    Eenter string
* @param [in] nLineno Appointed line
* @return 
* @li void
*/
void PubDisplayInv(int nLineno, char *pStr)
{
	int y;
	uint unX, unY, unLHeight, unLWidth;
	uint unSWidth, unSHeight;
	
	if(pStr == NULL || nLineno < 1)
	{
		return;
	}
	if(strlen(pStr)) {
		NAPI_ScrGetTrueTypeFontSize(pStr, strlen(pStr), (int *)&unSWidth, (int *)&unSHeight);
	}

	PubClearLine(nLineno, nLineno);
	NAPI_ScrGetViewPort(&unX, &unY, &unLWidth, &unLHeight);	

	y = (nLineno-1)*gnDispLineHeight;
	NAPI_ScrRectangle(0, y, unLWidth,gnDispLineHeight, RECT_PATTERNS_SOLID_FILL, DISP_INVBG_COLOUR);//Draw black rectangle
	if (unSHeight > gnDispLineHeight)
	{
		unSHeight = gnDispLineHeight;
	}
	y += (gnDispLineHeight - unSHeight)/2;
	DispBoldFontText(1 ,y , pStr);		
	
}

/**
* @brief Inverse diplay string(small font)
* @param [in] nAlign   Align mode---EM_DISPLAY_ALIGN(colorful machine ignores nAlign)
* @param [in] nLineno  Appointed line
* @param [in] pstr     Enter string
* @return 
* @li void
*/
void PubDisplayInvs(int nAlign, int nLineno, char *pstr)
{
	if(pstr == NULL || nLineno < 1)
	{
		return;
	}

	if(nAlign == DISPLAY_ALIGN_SMALLFONT)
	{
		ProSwitchFontSize(DISPLAY_ALIGN_SMALLFONT);
	}

	
	PubDisplayInv(nLineno, pstr);

	if(nAlign == DISPLAY_ALIGN_SMALLFONT)//restore
	{
		ProSwitchFontSize(DISPLAY_ALIGN_BIGFONT);
	}	
}

/**
* @brief Display title
* @param [in] pszTitle
* @return 
* @author Liug
* @date 2012-7-23
*/

void PubDisplayTitle(char *pszTitle)
{
	uint unX, unY, unLHeight, unLWidth;
	EM_TEXT_ATTRIBUTE emOldAttr;
		
	if(pszTitle == NULL)	
	{
		return;
	}

	PubClearLine(1, 1);
	NAPI_ScrGetViewPort(&unX, &unY, &unLWidth, &unLHeight);	
	NAPI_ScrRectangle(0, 0, unLWidth,gnDispLineHeight, RECT_PATTERNS_SOLID_FILL, TITLE_BG_COLOUR);//Draw bg rectangle 
	NAPI_ScrSetAttr(TEXT_ATTRIBUTE_REVERSE , &emOldAttr);
	PubDisplayGen(1, pszTitle);
	NAPI_ScrSetAttr(emOldAttr , NULL);
}

/**
* @brief Display string at the end of lines
* @param [in] nFont	Font
* @param [in] nMode	Show mode---EM_DISPLAY_MODE 
* @param [in] nIsInv NO-normal  YES-inverse
* @param [in] pFormat Display content
* @return 
*/
void PubDisplayTail(int nFont, int nMode, int nIsInv, char * pFormat,...)
{
	char	szDispStr[500] = {0};
	va_list VarList;
	int 	nMaxLine;
	
	va_start(VarList, pFormat);
	vsprintf(szDispStr, pFormat, VarList);
	va_end(VarList);

	PubGetDispView(&nMaxLine,NULL);
	if(nFont == SMALLFONT)
	{
		nMaxLine *=2;
		if(nIsInv)
			ProDisplayInvs(DISPLAY_ALIGN_SMALLFONT, nMode, nMaxLine, szDispStr);
		else
			PubDisplayStrs(DISPLAY_ALIGN_SMALLFONT, nMode, nMaxLine, 1,szDispStr );
	}
	else
	{
		if(nIsInv)
			ProDisplayInv( nMode, nMaxLine, szDispStr);
		else
			PubDisplayStr( nMode, nMaxLine, 1,szDispStr );

	}
}

/**
* @brief Display string in appointed location. Support multiple lines
* @param [in] nFont		0 Big font, 1 small font
* @param [in] nLineno	Line from 1-8
* @param [in] nClumno	Clumno form 1-16
* @param [in] pFormat	Content
* @return 
* @li void
*/
void PubDispMultLines(int nFont, int nLineno, int nClumno, char * pFormat,...)
{
	va_list VarList;
	char	szDispStr[1024] = {0};
	int		nLeftLen, nTrimedLen ,nTmp;
	int 	x,y;
	uint unX, unY, unLHeight, unLWidth;
	int unSHeight;
	
	if(nLineno < 1)
	{
		nLineno = 1;
	}
	if(nClumno < 1)
	{
		nClumno = 1;
	}	

	va_start(VarList, pFormat);
	vsprintf(szDispStr, pFormat, VarList);
	va_end(VarList);
	NAPI_ScrGetViewPort(&unX, &unY, &unLWidth, &unLHeight);	


	if(nFont == DISPLAY_ALIGN_SMALLFONT)
	{
		ProSwitchFontSize(DISPLAY_ALIGN_SMALLFONT);
	}
	
	x = (nClumno-1)*gnDispFontSize;
	NAPI_ScrGetTrueTypeFontSize(szDispStr, strlen(szDispStr), NULL, (int *)&unSHeight);
	if (unSHeight > gnDispLineHeight)
	{
		unSHeight = gnDispLineHeight;
	}
	y = (nLineno-1)*gnDispLineHeight + (gnDispLineHeight - unSHeight)/2;
	nLeftLen = strlen(szDispStr);
	nTrimedLen = 0;

	while(1)
	{	
		TrimStrToOneLine(x, szDispStr + nTrimedLen, nLeftLen, &nTmp, NULL);
		NAPI_ScrDispTrueTypeFontText(x ,y , szDispStr + nTrimedLen, nTmp);
		y += gnDispLineHeight;
		x = 0;
		nTrimedLen += nTmp;
		nLeftLen -= nTmp;
		
		if(nLeftLen <= 0)
		{
			break;
		}

		if( y>= unLHeight)
		{
			break;
		}			
	}

	if(nFont == DISPLAY_ALIGN_SMALLFONT)//restore
	{
		ProSwitchFontSize(DISPLAY_ALIGN_BIGFONT);
	}
	//PubDisplayStrs(nFont, 0, nLineno, nClumno, szDispStr);
}

/**
* @brief Get format string, support multiple lines
* @param [in] nFont	0 big font,  1 small font
* @param [in] pFormat |L close to left, |C close to centre, |R close to right, such as("L%s|C%s|R%s",szLeft,szCenter,szRight)
* @param [out] pOutStr   Aligned string
* @return 
* @li void
*/
void PubGetStrFormat(int nFont, char * pOutStr, char * pFormat,...)
{
	char	szDispStr[1024] = {0};
	va_list VarList;
	
	va_start(VarList, pFormat);
	vsprintf(szDispStr, pFormat, VarList);
	va_end(VarList);

	GetStrFormat(nFont, pOutStr, szDispStr);
}

/**
* @brief Display logo
* @param [in] pszPath Logo path
* @param [in] nX      Abscissa of picture (top left corner)
* @param [in] nY	  Ordinate of picture (top left corner)
* @return 
* @li void
*/
void PubDisplayLogo(char *pszPath, int nX, int nY)
{
	DisplayLogo(pszPath, nX, nY);
}

/**
* @brief Display string in appointed line(big font)
* @param [in] nMode	Show mode---EM_DISPLAY_MODE
* @param [in] nLineno Line from 1
* @param [in] pFormat Content
* @return 
* @li void
*/
void PubDisplayStrInline(int nMode, int nLineno, char *pFormat,...)
{
	va_list VarList;
	char szDispStr[500];
	if(nLineno < 1)
		return;
	va_start(VarList, pFormat);	
	vsprintf(szDispStr, pFormat, VarList);
	PubDisplayStr(nMode, nLineno, 1, szDispStr);
	va_end(VarList);
}


/**
* @brief Display string in appointed line(small font)
* @param [in] nAlign Align mode---EM_DISPLAY_ALIGN(colorful machine ignores nAlign)
* @param [in] nMode	show mode---EM_DISPLAY_MODE
* @param [in] nLineno Line from 1
* @param [in] pFormat Content
* @return 
* @li void
*/
void PubDisplayStrInlines(int nAlign, int nMode, int nLineno, char *pFormat,...)
{
	va_list VarList;
	char    szDispStr[500] = {0};

	if(nLineno < 1)
		return;
	va_start(VarList, pFormat); 
	vsprintf(szDispStr, pFormat, VarList);
	PubDisplayStrs(nAlign, nMode, nLineno, 1, szDispStr);
	va_end(VarList);
}

/**
* @brief Input message in appointed location 
* @param [in] nClumno   Column(from 1)
* @param [in] nLineno   Line(from 1)
* @param [in] nMinLen   Minimum length
* @param [in] nMaxLen   Maximum length
* @param [in] nTimeOut  Timeout
* @param [in] nEditMask Input mode---EM_INPUT_MODE
* @param [out] pszOut   Output
* @param [out] pnOutLen Output length
* @return 
* @li APP_TIMEOUT 	Timeout
* @li APP_FAIL    	Abnormal
* @li APP_QUIT    	User cancel
* @li APP_SUCC    	Success
* @li KEY_F1/KEY_F2 Only when 'PubInputxy' is called in a callback function which is parameter of 'PubUpDownMenus'.
* <pre>	
*	e.g. 
*	int func(void)
*	{
*		//In this circumstance, PubInputxy will return KEY_F1 or KEY_F2, 
*		//if KEY_F1 or KEY_F2 is pressed, in order to achieve function of scrolling up and down.
*		PubInputxy	
*	}
*	int (*lSetFuns[])(void) = 
*	{
*		func,
*		...,
*		NULL
*	};
*	PubUpDownMenus(lSetFuns);
* </pre>
* 
*/
int PubInputxy(int nClumno, int nLineno, char* pszOut, int* pnOutLen, int nMinLen, int nMaxLen, int nTimeOut, int nEditMask)
{
	if(nClumno < 0 || nLineno < 0 || nMinLen < 0 || nMaxLen < 0 || nMinLen > nMaxLen || nTimeOut < 0 
		|| nEditMask < 0 || pszOut == NULL || pnOutLen == NULL || strlen(pszOut) > 60 )
	{
		return APP_FAIL;
	}
	return Inputxy(nClumno, nLineno, pszOut, pnOutLen, nMinLen, nMaxLen, nTimeOut, nEditMask);
}

/**
* @brief Inout dialog box
* @param [in] pszTitle   Title
* @param [in] pszContent Prompt content
* @param [in] nMinLen    Minimum length
* @param [in] nMaxLen    Maximum length
* @param [in] nTimeOut   Timeout
* @param [in] nEditMask  Input mode---EM_INPUT_MODE
* @param [out] pszOut    Output string
* @param [out] pnOutLen  Output length
* @return 
* @li APP_TIMEOUT Timeout
* @li APP_FAIL    	Abnormal
* @li APP_QUIT    	user cancel
* @li APP_SUCC    	Success
* @li KEY_F1/KEY_F2 Refer to 'PubInputxy'
*/
int PubInputDlg(const char *pszTitle, const char *pszContent, char *pszOut, int *pnOutLen, int nMinLen, int nMaxLen, int nTimeOut, int nEditMask)
{

	if(nMinLen < 0 || nMaxLen < 0 || nMinLen > nMaxLen || nTimeOut < 0 || nEditMask < 0)
	{
		return APP_FAIL;
	}


	if (pszOut == NULL || pnOutLen == NULL)
	{
		return APP_FAIL;
	}
	
	PubClearAll();
	if (pszTitle != NULL)
	{
		PubDisplayTitle((char*)pszTitle);
	}
	if (pszContent != NULL && strlen(pszContent) != 0)
	{
		PubDisplayStr(0, 2, 1, (char*)pszContent);
	}

	return Inputxy (1, 3, pszOut, pnOutLen, nMinLen, nMaxLen, nTimeOut, nEditMask);


}

/**
* @brief Extension input dialog box
* @param [in] pszTitle   Title
* @param [in] pszContent Prompt content
* @param [in] psInfo      If no need, set NULL
* @param [in] nClumno   Column(from 1)
* @param [in] nLineno   Line(from 1)
* @param [in] nMinLen    Minimum length
* @param [in] nMaxLen    Maximum length
* @param [in] nTimeOut   Timeout
* @param [in] nEditMask  Input mode---EM_INPUT_MODE
* @param [out] pszOut    Output
* @param [out] pnOutLen  Output length
* @return 
* @li APP_TIMEOUT Timeout
* @li APP_FAIL    Abnormal
* @li APP_QUIT    User cancel
* @li APP_SUCC    Success
* @li KEY_F1/KEY_F2 Refer to 'PubInputxy'
*/
int PubExtInputDlg(const char *pszTitle, const char *pszContent, char* psInfo, 
                int nClumno, int nLineno, char *pszOut, int *pnOutLen,int nMinLen, 
                int nMaxLen, int nTimeOut, int nEditMask)
{
	if(nClumno < 0 || nLineno < 0 || nMinLen < 0 || nMaxLen < 0 || nMaxLen < nMinLen || nTimeOut < 0 || nEditMask < 0)
		return APP_FAIL;

	return ProInputDlg(pszTitle, pszContent, psInfo, nClumno, nLineno, pszOut, pnOutLen, nMinLen, nMaxLen, nTimeOut, nEditMask);

}

/**
* @brief Prompt dialog box
* @param [in] pszTitle   Title
* @param [in] pszContent Prompt content
* @param [in] nBeep      Beep times
* @param [in] nTimeOut   Timeout
* @return 
* @li APP_TIMEOUT Timeout
* @li APP_SUCC    Success
*/
int PubMsgDlg(const char *pszTitle, const char *pszContent, int nBeep, int nTimeOut)
{
	int nKey;

	if (PubGetKbAttr() == KB_VIRTUAL)
	{
		if (nTimeOut <= 2) {
			PubClearAll();
			PubDisplayTitle((char*)pszTitle);
			PubDisplayStr(DISPLAY_MODE_CLEARLINE, 4, 1, (char*)pszContent);
			PubUpdateWindow();
			PubBeep(nBeep);
			PubWaitGetKbPad(nTimeOut);
			return APP_SUCC;
		}
		return PubConfirmDlg(pszTitle, pszContent, nBeep, nTimeOut);
	}

	PubClearAll();
	PubDisplayTitle((char*)pszTitle);
	PubDisplayStr(DISPLAY_MODE_CLEARLINE,2,1, (char*)pszContent);
	PubUpdateWindow();
	PubBeep(nBeep);
	NAPI_KbHit(&nKey);
	if( nTimeOut < 0 )
	{
		return APP_TIMEOUT;
	}

	while(1)
	{
		nKey = PubGetKeyCode(nTimeOut);

		if (nKey == 0)
		{
			return APP_TIMEOUT;
		}
		else
		{
			return APP_SUCC;
		}
	}

	return APP_QUIT;
}

/**
* @brief show buttons
* @param [in] nButton_attr   button attr (normal/confirm...)
* @param [in] pszButton1/2/3/4 first/second/third.. button name
* @return 
* @li APP_SUCC    Success
*/
int PubShowButton(EM_BUTTON_ATTR nButton_attr, char *pszButton1, char *pszButton2, char *pszButton3, char *pszButton4)
{
	int nMaxLine;
	uint unX, unY, unScrWidth, unScrHeight, unColor;
	int nGap = 2, nButWidth, nOff = 0;
	int nTxtWidth, nTxtHeight, x, y, nSelectLine, nColumnNum = 0, nSpace;
	
	PubGetDispView(&nMaxLine, NULL);
	NAPI_ScrGetViewPort(&unX, &unY, &unScrWidth,&unScrHeight);
	if (pszButton1 == NULL) {
		return APP_QUIT;
	}

	nColumnNum = 1;
	if (pszButton2 != NULL) {
		nColumnNum++;
	}

	if (pszButton3 != NULL) {
		nColumnNum++;
	}

	if (pszButton4 != NULL) {
		nColumnNum++;
	}

	if (nButton_attr == BUTTON_CONFIRM || nButton_attr == BUTTON_NORMAL) {
		nGap = 0;
		nSelectLine = 2;
	} else if (nButton_attr == BUTTON_HOMEMENU) {
		nGap = 2;
		nSelectLine = 1;
	} else {
		nGap = 2;
		nSelectLine = 2;
	}
	nButWidth = (unScrWidth - (nColumnNum+1)*nGap) / nColumnNum;
	nSpace = (unScrWidth - (nColumnNum+1)*nGap) % nColumnNum;
// button1
	if (pszButton1 != NULL)
	{
		nOff = nGap;
		if (nButton_attr == BUTTON_CONFIRM) {
			if (nColumnNum == 1) {
				unColor = COLOR_GREEN;
			} else if (nColumnNum == 2) {
				unColor = COLOR_RED;
			} else if (nColumnNum == 3) {
				unColor = COLOR_GREEN;
			} else {
				unColor = COLOR_RED;
			}
		} else if (nButton_attr == BUTTON_HOMEMENU) {
			unColor = COLOR_LGRAY;
		} else {
			unColor = COLOR_LGRAY;
		}
		NAPI_ScrRectangle(nOff, gnDispLineHeight*(nMaxLine-nSelectLine), nButWidth, gnDispLineHeight*nSelectLine, RECT_PATTERNS_SOLID_FILL, unColor);
		nOff += nButWidth + nGap;
		x = 0;
		NAPI_ScrGetTrueTypeFontSize(pszButton1, strlen(pszButton1), &nTxtWidth, &nTxtHeight);
		y = gnDispLineHeight*(nMaxLine-nSelectLine) + (gnDispLineHeight*nSelectLine-nTxtHeight)/2;
		DispBoldFontText((nButWidth-nTxtWidth)/2, y, pszButton1);
	}

// button2
	if (pszButton2 != NULL)
	{
		if (nButton_attr == BUTTON_CONFIRM) {
			if (nColumnNum == 2) {
				unColor = COLOR_GREEN;
			} else if (nColumnNum == 3) {
				unColor = COLOR_RED;
			} else {
				unColor = COLOR_LGRAY;
			}
		} else if (nButton_attr == BUTTON_HOMEMENU) {
			unColor = COLOR_LGRAY;
		} else {
			unColor = COLOR_LGRAY;
		}
		if (nColumnNum == 2) {
			nButWidth += nSpace;
		}
		NAPI_ScrRectangle(nOff, gnDispLineHeight*(nMaxLine-nSelectLine), nButWidth, gnDispLineHeight*nSelectLine, RECT_PATTERNS_SOLID_FILL, unColor);
		x = nButWidth + nGap;
		NAPI_ScrGetTrueTypeFontSize(pszButton2, strlen(pszButton2), &nTxtWidth, &nTxtHeight);
		y = gnDispLineHeight*(nMaxLine-nSelectLine) + (gnDispLineHeight*nSelectLine-nTxtHeight)/2;
		DispBoldFontText((nButWidth-nTxtWidth)/2+x, y, pszButton2);
		nOff += nButWidth + nGap;
	}

// button3
	if (pszButton3 != NULL)
	{
		if (nButton_attr == BUTTON_CONFIRM) {
			if (nColumnNum == 3) {
				unColor = COLOR_GREEN;
			} else {
				unColor = COLOR_RED;
			}
		} else {
			unColor = COLOR_LGRAY;
		}
		if (nButton_attr == BUTTON_CONFIRM) {
			if (nColumnNum == 3) {
				unColor = COLOR_GREEN;
			} else {
				unColor = COLOR_LGRAY;
			}
		} else if (nButton_attr == BUTTON_HOMEMENU) {
			unColor = COLOR_LGRAY;
		} else {
			unColor = COLOR_LGRAY;
		}
		if (nColumnNum == 3) {
			nButWidth += nSpace;
		}
		NAPI_ScrRectangle(nOff, gnDispLineHeight*(nMaxLine-nSelectLine), nButWidth, gnDispLineHeight*nSelectLine, RECT_PATTERNS_SOLID_FILL, unColor);
		x += nButWidth + nGap;
		NAPI_ScrGetTrueTypeFontSize(pszButton3, strlen(pszButton3), &nTxtWidth, &nTxtHeight);
		y = gnDispLineHeight*(nMaxLine-nSelectLine) + (gnDispLineHeight*nSelectLine-nTxtHeight)/2;
		DispBoldFontText((nButWidth-nTxtWidth)/2+x, y, pszButton3);
		nOff += nButWidth + nGap;
	}

	// button4
	if (pszButton4 != NULL)
	{
		if (nButton_attr == BUTTON_CONFIRM) {
			if (nColumnNum == 4) {
				unColor = COLOR_RED;
			} else {
				unColor = COLOR_LGRAY;
			}
		} else if (nButton_attr == BUTTON_HOMEMENU) {
			unColor = COLOR_LGRAY;
		} else {
			unColor = COLOR_LGRAY;
		}
		if (nColumnNum == 4) {
			nButWidth += nSpace;
		}
		NAPI_ScrRectangle(nOff, gnDispLineHeight*(nMaxLine-nSelectLine), nButWidth, gnDispLineHeight*nSelectLine, RECT_PATTERNS_SOLID_FILL, unColor);
		x += nButWidth + nGap;
		NAPI_ScrGetTrueTypeFontSize(pszButton4, strlen(pszButton4), &nTxtWidth, &nTxtHeight);
		y = gnDispLineHeight*(nMaxLine-nSelectLine) + (gnDispLineHeight*nSelectLine-nTxtHeight)/2;
		DispBoldFontText((nButWidth-nTxtWidth)/2+x, y, pszButton4);
		nOff += nButWidth + nGap;
	}

	PubUpdateWindow();
	return APP_SUCC;
}

/**
* @brief Prompt dialog box
* @param [in] nButton_attr button attr
* @param [in] pszButton1/pszButton2/pszButton3/pszButton3  button key name
* @param [in] nTimeOut   Timeout : second
* @return 
* @ the button num = 1, return key_enter
* @ the button num = 2, return key_esc key_enter
* @ the button num = 3, return key_F1 key_esc key_F2
* @ the button num = 4, return key_up/key_esc/key_enter/key_down
* @li APP_TIMEOUT 0: Block; Non-zero: Timeout in milliseconds
* @li APP_SUCC    Success
*/
int PubShowGetKbPad(int nTimeout, EM_BUTTON_ATTR nButton_attr, char *pszButton1, char *pszButton2, char *pszButton3, char *pszButton4)
{
	ST_PADDATA stPaddata;
	uint unX, unY, unScrWidth, unScrHeight;
	int nColumnNum, nButWidth, nSelectLine, nGap = 2;

	if (pszButton1 == NULL) {
		return APP_QUIT;
	}

	nColumnNum = 1;
	if (pszButton2 != NULL) {
		nColumnNum++;
	}

	if (pszButton3 != NULL) {
		nColumnNum++;
	}

	if (pszButton4 != NULL) {
		nColumnNum++;
	}
	NAPI_ScrGetViewPort(&unX, &unY, &unScrWidth, &unScrHeight);

	if (nButton_attr == BUTTON_CONFIRM || nButton_attr == BUTTON_NORMAL) {
		nGap = 0;
		nSelectLine = 2;
	} else if (nButton_attr == BUTTON_HOMEMENU) {
		nGap = 2;
		nSelectLine = 1;
	} else {
		nGap = 2;
		nSelectLine = 2;
	}
	nButWidth = (unScrWidth - (nColumnNum+1)*nGap) / nColumnNum;

	PubShowButton(nButton_attr, pszButton1, pszButton2, pszButton3, pszButton4);
	NAPI_KbVppTpFlush();
	while (1) {
		if(NAPI_KbGetPad(&stPaddata, nTimeout*1000) == NAPI_ERR_TIMEOUT) {
			return APP_TIMEOUT;
		}

		if(stPaddata.emPadState != PADSTATE_UP) {
			continue;
		}

		if (nButton_attr == BUTTON_HOMEMENU || nButton_attr == BUTTON_NORMAL) {
			break;
		}

		if (stPaddata.unPadY >= unScrHeight-nSelectLine*gnDispLineHeight) {
			break;
		}
	}

	if (nButton_attr == BUTTON_HOMEMENU) {
		if (stPaddata.unPadY < unScrHeight-nSelectLine*gnDispLineHeight) {
			return KEY_ESC;
		}
	}

	if (nColumnNum == 3) { // F1 esc F2
		if(stPaddata.unPadX < nButWidth) {
			return KEY_F1;
		}
		if(stPaddata.unPadX < 2*nButWidth) {
			if (nButton_attr == BUTTON_HOMEMENU) {
				return KEY_ENTER;
			} else {
				return KEY_ESC;
			}
		}
		return KEY_F2;
	} else if (nColumnNum == 2) { // esc enter
		if(stPaddata.unPadX < nButWidth) {
			return KEY_ESC;
		}
		if(stPaddata.unPadX < 2*nButWidth) {
			return KEY_ENTER;
		}
		return KEY_ENTER;
	} else if (nColumnNum == 4) { //up esc enter down
		if(stPaddata.unPadX < nButWidth) {
			return KEY_UP;
		} else if(stPaddata.unPadX < 2*nButWidth) {
			return KEY_ESC;
		} else if(stPaddata.unPadX < 3*nButWidth) {
			return KEY_ENTER;
		}
		return KEY_DOWN;
	} else { // enter
		return KEY_ENTER;
	}

	return APP_SUCC;
}

/**
* @brief Prompt dialog box
* @param [in] nButton_attr button attr
* @param [in] pszButton1/pszButton2/pszButton3/pszButton3  button key name
* @param [in] nTimeOut   Timeout : milliseconds
* @return 
* @ the button num = 1, return key_enter
* @ the button num = 2, return key_esc key_enter
* @ the button num = 3, return key_F1 key_esc key_F2
* @ the button num = 4, return key_up/key_esc/key_enter/key_down
* @li APP_TIMEOUT 0: Block; Non-zero: Timeout in milliseconds
* @li APP_SUCC    Success
*/
int PubShowGetKbPad_Ms(int nMilliseconds, EM_BUTTON_ATTR nButton_attr, char *pszButton1, char *pszButton2, char *pszButton3, char *pszButton4)
{
	ST_PADDATA stPaddata;
	uint unX, unY, unScrWidth, unScrHeight;
	int nColumnNum, nButWidth, nSelectLine, nGap = 2;

	if (pszButton1 == NULL) {
		return APP_QUIT;
	}

	nColumnNum = 1;
	if (pszButton2 != NULL) {
		nColumnNum++;
	}

	if (pszButton3 != NULL) {
		nColumnNum++;
	}

	if (pszButton4 != NULL) {
		nColumnNum++;
	}
	NAPI_ScrGetViewPort(&unX, &unY, &unScrWidth, &unScrHeight);

	if (nButton_attr == BUTTON_CONFIRM || nButton_attr == BUTTON_NORMAL) {
		nGap = 0;
		nSelectLine = 2;
	} else if (nButton_attr == BUTTON_HOMEMENU) {
		nGap = 2;
		nSelectLine = 1;
	} else {
		nGap = 2;
		nSelectLine = 2;
	}
	nButWidth = (unScrWidth - (nColumnNum+1)*nGap) / nColumnNum;

	PubShowButton(nButton_attr, pszButton1, pszButton2, pszButton3, pszButton4);
	if (nButton_attr != BUTTON_HOMEMENU)
	{
		NAPI_KbVppTpFlush();
	}
	while (1) {
		if(NAPI_KbGetPad(&stPaddata, nMilliseconds) == NAPI_ERR_TIMEOUT) {
			return APP_TIMEOUT;
		}

		if(stPaddata.emPadState != PADSTATE_UP) {
			continue;
		}

		if (nButton_attr == BUTTON_HOMEMENU || nButton_attr == BUTTON_NORMAL) {
			break;
		}

		if (stPaddata.unPadY >= unScrHeight-nSelectLine*gnDispLineHeight) {
			break;
		}
	}

	if (nButton_attr == BUTTON_HOMEMENU) {
		if (stPaddata.unPadY < unScrHeight-nSelectLine*gnDispLineHeight) {
			return KEY_ESC;
		}
	}

	if (nColumnNum == 3) { // F1 esc F2
		if(stPaddata.unPadX < nButWidth) {
			return KEY_F1;
		}
		if(stPaddata.unPadX < 2*nButWidth) {
			if (nButton_attr == BUTTON_HOMEMENU) {
				return KEY_ENTER;
			} else {
				return KEY_ESC;
			}
		}
		return KEY_F2;
	} else if (nColumnNum == 2) { // esc enter
		if(stPaddata.unPadX < nButWidth) {
			return KEY_ESC;
		}
		if(stPaddata.unPadX < 2*nButWidth) {
			return KEY_ENTER;
		}
		return KEY_ENTER;
	} else if (nColumnNum == 4) { //up esc enter down
		if(stPaddata.unPadX < nButWidth) {
			return KEY_UP;
		} else if(stPaddata.unPadX < 2*nButWidth) {
			return KEY_ESC;
		} else if(stPaddata.unPadX < 3*nButWidth) {
			return KEY_ENTER;
		}
		return KEY_DOWN;
	} else { // enter
		return KEY_ENTER;
	}

	return APP_SUCC;
}


/**
* @brief Confirm dialog box(need confirmation)
* @param [in] pszTitle   Title
* @param [in] pszContent Content
* @param [in] nBeep      Beep times
* @param [in] nTimeOut   Timeout
* @return 
* @li APP_TIMEOUT Timeout
* @li APP_QUIT    Cancel
* @li APP_SUCC    Press ENTER
*/
int PubConfirmDlg(const char *pszTitle, const char *pszContent, int nBeep, int nTimeOut)
{
	int	nKey;
	uint unX,unY;
	uint unSWidth1 , unSWidth2,unSHeight1, unSHeight2, unScrWidth ,unScrHeight;
	char * str1 = (char*)("CANCEL");
	char * str2 = (char*)("ENTER");
	int 	nMaxLine, x1, x2, y1, y2, nTouchline;
	ST_PADDATA stPaddata;
	int 	nTimeCount = 0;
	
	if(nBeep < 0 || nTimeOut < 0)
	{		
		return APP_FAIL;
	}

	NAPI_ScrGetViewPort(&unX, &unY, &unScrWidth,&unScrHeight);
	PubGetDispView(&nMaxLine, NULL);
	
	NAPI_ScrGetTrueTypeFontSize(str1, strlen(str1), (int*)&unSWidth1, (int*)&unSHeight1);
	NAPI_ScrGetTrueTypeFontSize(str2, strlen(str2), (int*)&unSWidth2, (int*)&unSHeight2);

	if (pszTitle!=NULL && strlen(pszTitle)>0)
	{
		PubClearAll();
		PubDisplayTitle((char*)pszTitle);
	}
	if (pszContent!=NULL && strlen(pszContent)>0)
	{
		PubClear2To4();
		PubDisplayStr(0,2,1,(char*)pszContent);
	}

	if (PubGetKbAttr() == KB_VIRTUAL) {
		NAPI_ScrRectangle(0, gnDispLineHeight*(nMaxLine-2), unScrWidth/2,gnDispLineHeight*2, RECT_PATTERNS_SOLID_FILL, 0xF800);//Draw black rectangle
		NAPI_ScrRectangle(unScrWidth/2, gnDispLineHeight*(nMaxLine-2), unScrWidth/2,gnDispLineHeight*2, RECT_PATTERNS_SOLID_FILL, 0x07E0);//Draw black rectangle
	} else {
		NAPI_ScrRectangle(0, gnDispLineHeight*(nMaxLine-1), unScrWidth/2,gnDispLineHeight, RECT_PATTERNS_SOLID_FILL, 0xF800);//Draw black rectangle
		NAPI_ScrRectangle(unScrWidth/2, gnDispLineHeight*(nMaxLine-1), unScrWidth/2,gnDispLineHeight, RECT_PATTERNS_SOLID_FILL, 0x07E0);//Draw black rectangle
	}

	if(unSWidth1 < unScrWidth/2)
		x1 = (unScrWidth/2 - unSWidth1)/2;
	else
		x1 = 0;		

	if(unSWidth1 < unScrWidth/2)
		x2 = unScrWidth/2 + (unScrWidth/2 - unSWidth2)/2;
	else
		x2 = unScrWidth/2;	

	if (unSHeight1 > gnDispLineHeight)
	{
		unSHeight1 = gnDispLineHeight;
	}

	if (unSHeight2 > gnDispLineHeight)
	{
		unSHeight2 = gnDispLineHeight;
	}

	if (PubGetKbAttr() == KB_VIRTUAL) {
		y1 = gnDispLineHeight*(nMaxLine-2) + (2*gnDispLineHeight - unSHeight1)/2;
		y2 = gnDispLineHeight*(nMaxLine-2) + (2*gnDispLineHeight - unSHeight2)/2;
	} else {
		y1 = gnDispLineHeight*(nMaxLine-1) + (gnDispLineHeight - unSHeight1)/2;
		y2 = gnDispLineHeight*(nMaxLine-1) + (gnDispLineHeight - unSHeight2)/2;
	}

	DispBoldFontText(x1, y1, str1);
	DispBoldFontText(x2 ,y2, str2);

	NAPI_ScrRefresh();
	PubBeep(nBeep);
	NAPI_KbHit(&nKey);
	if( nTimeOut < 0 )
	{
		return APP_TIMEOUT;
	}

	NAPI_ScrRefresh();
	if (PubGetKbAttr() == KB_VIRTUAL) {
		NAPI_KbVppTpFlush();
	}

	while(1)
	{

		if(NAPI_KbGetPad(&stPaddata, 1000) == NAPI_ERR_TIMEOUT)
		{
			if((nTimeOut>0) && (++nTimeCount == nTimeOut )){
				return APP_TIMEOUT;
			}			
		}	
		
		if(stPaddata.emPadState == PADSTATE_KEY)
		{	
			nKey = stPaddata.unKeycode;			 
			switch (nKey)
			{
			case 0:
				return APP_TIMEOUT;
			case KEY_ENTER:
				return APP_SUCC;
			case KEY_ESC:
				return APP_QUIT;
			default:
				break;
			}
		}	
		else if(stPaddata.emPadState == PADSTATE_UP)
		{
			if (PubGetKbAttr() == KB_VIRTUAL) {
				nTouchline = stPaddata.unPadY/gnDispLineHeight;
				if (nTouchline < nMaxLine - 2) {
					continue;
				}
			} else {
				nTouchline = stPaddata.unPadY/gnDispLineHeight + 1;
				if (nTouchline < nMaxLine) {
					continue;
				}
			}
			if(stPaddata.unPadX < unScrWidth/2 )
				return APP_QUIT;
			else
				return APP_SUCC;
		}		
	}

	return APP_QUIT;
}

/**
* @brief Input date dialog box
* @param [in] pszTitle   Title
* @param [in] pszContent Content
* @param [in] nFormat    Date format---EM_INPUT_DATE_MODE
* @param [in] nTimeOut   Timeout
* @param [out] pszDate   Date
* @return 
* @li APP_FAIL    	Abnormal
* @li APP_TIMEOUT 	Timeout
* @li APP_QUIT    	Cancel
* @li APP_SUCC    	Success
* @li KEY_F1/KEY_F2 Refer to 'PubInputxy'
*/
int PubInputDate(const char *pszTitle, const char *pszContent, char *pszDate, int nFormat, int nTimeOut)
{
	if(nFormat < 0 || nTimeOut < 0)
	{
		return APP_FAIL;
	}

	return ProInputDate(pszTitle, pszContent, NULL, 1, 3, pszDate, nFormat, nTimeOut);
}

/**
* @brief Extension input date dialog box
* @param [in] pszTitle   Title
* @param [in] pszContent Content
* @param [in] pInfo      If no need, set NULL
* @param [in] nClumno    Column(from 1)
* @param [in] nLineno    Line(from 1)
* @param [in] nFormat    Date format---EM_INPUT_DATE_MODE
* @param [in] nTimeOut   Timeout
* @param [out] pszDate   Date
* @return 
* @li APP_FAIL    	Abnormat
* @li APP_TIMEOUT T	imeout
* @li APP_QUIT    	Cancel
* @li APP_SUCC    	Success
* @li KEY_F1/KEY_F2 Refer to 'PubInputxy'
*/
int PubExtInputDate(const char *pszTitle, const char *pszContent, const char *pInfo, 
											int nClumno, int nLineno, char *pszDate,int nFormat, int nTimeOut)
{
	if(nClumno < 0 || nLineno < 0 || nFormat < 0 || nTimeOut < 0)
	{
		return APP_FAIL;
	}

	return ProInputDate(pszTitle, pszContent, pInfo, nClumno, nLineno, pszDate, nFormat, nTimeOut);
}

/**
* @brief Input Amount dialog box
* @param [in] pszTitle   Title
* @param [in] pszContent Content
* @param [in] nNoneFlag  EM_INPUT_AMOUNT_MODE
* @param [in] nTimeOut   Timeout
* @param [out] pszAmount Amount
* @param [out] pnInputLen Output length
* @return 
* @li APP_TIMEOUT Timeout
* @li APP_FAIL    	Abnormal
* @li APP_QUIT    	Cancel
* @li KEY_F1/KEY_F2 Refer to 'PubInputxy'
*/
int PubInputAmount(const char *pszTitle, const char *pszContent, char *pszAmount, 
													int *pnInputLen, int nNoneFlag, int nTimeOut)
{
	int		nMinLen = 0;
	int		nRet;		
	char	szBuf[13] = {0};
	int		i;
	
	if(nNoneFlag < 0 || nTimeOut < 0)
	{
		return APP_FAIL;
	}	
	if (pszAmount == NULL )
	{
		return APP_FAIL;
	}
	if (nNoneFlag == INPUT_AMOUNT_MODE_NONE)
	{
		nMinLen = 0;
	}
	else
	{
		nMinLen = 1;
	}

	strcpy (szBuf, pszAmount);
	nRet = PubInputDlg(pszTitle, pszContent, szBuf, pnInputLen, nMinLen,
		GUI_AMOUNT_MAXLEN, nTimeOut, INPUT_MODE_AMOUNT);

	if ( nRet == APP_SUCC )
	{
		sprintf(pszAmount, "%12s", szBuf);
		for (i = 0; i < 12; i ++)
		{
			if (pszAmount[i] == 0x20)
			{
			    pszAmount[i] = 0x30;
			}
        }

		return APP_SUCC;
	}
	else
	{
		return nRet;
	}	
}

/**
* @brief Display contents & picture
* @param [in] unWidth		Width
* @param [in] unHeight		Height
* @param [in] pszBitmap  	Bitmap picture data
* @param [in] pszStr     	If select item list, seperate by '|', else directly show
* @param [in] nTimeout      Timeout
* @param [out] pnItemNum    Return item
* @return 
* @li void
*/
void PubDrawBitmapList(const uint unWidth, const uint unHeight, const char *pszBitmap, const char *pszStr, int nTimeout, int* pnItemNum)
{
	if(pnItemNum == NULL)
	{
		return;
	}
	DrawBitmapList(unWidth, unHeight, pszBitmap, pszStr, nTimeout, pnItemNum);
}

/**
* @brief Display contents & picture
* @param [in] unWidth		Width
* @param [in] unHeight		Height
* @param [in] pszPic   		File path
* @param [in] pszStr     	If select item list, seperate by '|', else directly show
* @param [in] nTimeout		Timeout
* @param [out] pnItemNum     Return item
* @return 
* @li void
*/
void PubDispPicList(const uint unWidth, const uint unHeight, const char *pszPic, const char *pszStr, int nTimeout, int* pnItemNum)
{
	if(pnItemNum == NULL)
	{
		return;
	}
	
	DispPicList(unWidth, unHeight, pszPic, pszStr, nTimeout, pnItemNum);
}

/**
* @brief Input IP dialog box
* @param [in] pszTitle   Title
* @param [in] pszContent Menu hint
* @param [in] pnOutLen   Input length
* @param [out] pszOut    Entered Ip
* @param [out] pnOutLen  Output the length of pszOut
* @return
* @li APP_QUIT    	Cancel
* @li APP_SUCC    	Success
* @li KEY_F1/KEY_F2 Refer to 'PubInputxy'
*/
int PubInputIp(const char * pszTitle, const char * pszContent, char * pszOut, int * pnOutLen)
{
	if(pszOut == NULL || pnOutLen == NULL)
	{
		return APP_FAIL;
	}
	return InputIp(pszTitle, pszContent, pszOut, pnOutLen);
}

/**
* @brief Enter PIN (plaintext)
* @param [in] pszTitle    ---Title (no use)
* @param [in] pszAmount   ---Entered Amount or NULL
* @param [in] pszContent  ---Contents or NULL
* @param [in] nMinLen     ---Minimum length
* @param [in] nMaxLen     ---Maximum length
* @param [out] pszOut     ---PIN (plaintext)
* @return 
* @li APP_SUCC		Success
* @li APP_FAIL		Fail 
* @li KEY_F1/KEY_F2 Refer to 'PubInputxy'
* @author Liug
* @date 2012-7-23
*/
int PubInputPin(const char *pszTitle,const char * pszAmount,const char * pszContent,int nMinLen, int nMaxLen,char *pszOut)
{
	if(pszOut == NULL || nMinLen < 0 || nMaxLen < 0 || nMinLen > nMaxLen)
	{
		return APP_FAIL;
	}
	return InputPin(pszTitle, pszAmount, pszContent, nMinLen, nMaxLen, pszOut);
}

/**
* @brief Scroll pages & display dialox box
* @param [in] pszTitle     ---Title
* @param [in] pszContent   ---Content
* @param [in] nIsResKey    ---Flag of special usage, 'YES' or 'No'
* <pre>	
*	If nFlag is set 'NO' and full screen is not enough to display 'pszContent, 
*		'APP_FUNCQUIT' will be returned as KEY_ENTER is pressed, then apps will call
*		'PubUpDownMsgDlg' again to see the details.
*	If nFlag is set 'YES', it will always return 'APP_SUCC' when KEY_ENTER is pressed.
*	e.g.
*	//Below full screen is not enough to display 'szContent'
*	nRet = PubUpDownMsgDlg("Title", szContent, NO, 60, &nKey);
*	if(nRet == APP_FUNCQUIT && nKey == KEY_ENTER)
*	{
*		nRet = PubUpDownMsgDlg("Title", szContent, YES, 60, &nKey);
*	}
* </pre>
* @param [in] nTimeOut     ---Timeout
* @param [out] nResKey	   ---Key value
* @return 
* @li APP_TIMEOUT  	Timeout
* @li APP_SUCC		Success
* @li APP_QUIT  	Quit
* @li APP_FUNCQUIT 	See above
* @li APP_FAIL  	Fail 
* @author Liug
* @date 2012-8-2
*/
int PubUpDownMsgDlg(const char* pszTitle,const char* pszContent,int nIsResKey,int nTimeOut,int* nResKey)
{
	return UpDownMsgDlg(pszTitle, pszContent, nIsResKey, nTimeOut, nResKey);
}

/**
* @brief Show confirmation dialog box(F1,F2 to turn pages)
* @param [in] pszTitle	 Title
* @param [in] pszContent Content
* @param [in] nBeep 	 Beep times
* @param [in] nTimeOut	 Timeout
* @return 
* @li TIMEOUT 	  Timeout
* @li APP_QUIT	  User cancel
* @li APP_SUCC	  Confrim
*/
int PubUpDownConfirmDlg(const char* pszTitle,const char* pszContent,int nBeep,int nTimeOut)
{
	return UpDownConfirmDlg(pszTitle, pszContent, nBeep, nTimeOut);
}

/**
* @brief Update & display screen
* @return void
* @author Liug
* @date 2012-7-20
*/
void PubUpdateWindow(void)
{
	NAPI_ScrRefresh();
	return;
}

/**
* @brief Show pub api error info(no err, no display)
* @param [in] pszTitle Title
* @return 
* @li APP_SUCC  
* @li APP_FAIL  
*/
int PubDispErr(const char *pszTitle)
{
	int nErrorCode = 0;
	int nNapiErrCode = 0;
	char szErrMsg[64+1] = {0};
	char szConTent[128+1] = {0};
	
	PubGetErrorCode(&nErrorCode, szErrMsg, &nNapiErrCode);
	if (nErrorCode == 0 && nNapiErrCode == 0)
	{
		return APP_SUCC;
	}
	if(strlen(szErrMsg)>0)
	{
		sprintf(szConTent, "%s", szErrMsg);
	}
	PubMsgDlg(pszTitle, szConTent, 3, 10);
	return APP_FAIL;
}

/**
* @brief Get color depth in order to identify the type of screen
* @return 
* @li APP_FAIL    Fail
* @li other       1----black-white, 16----color
*/
int PubScrGetColorDepth(void)
{
	int nColorDepth;

	if(NAPI_ScrGetColorDepth((uint*)&nColorDepth) == NAPI_OK)
	{
		return nColorDepth;
	}
	return APP_FAIL;
}

/**
* @fn PubStrCopy
* @brief  Smart copy funtion. Used to avoid half char of non-English string.
* @param in  scr: 	Source string
* @param in  AscLen:Length of source string
* @param out Dst:	Buffer to store result
* @return
* @li  Length of result
*/
int PubStrCopy(char *psDst, const char *psSrc, int nLen)
{
	int i;

	if(psDst == NULL || psSrc == NULL || nLen <= 0)
	{
		return APP_FAIL;
	}
	for(i=0; i<nLen; i++)
	{
		if((unsigned char)(psSrc[i]) > 0x80)  
		{/*<One Chinese char occupy 2 bytes in GBK */
			i++;
		}
	}
	if(i > nLen)
	{/*<Discard half Chinese char*/
		nLen--;
	}
	memcpy(psDst, psSrc, nLen);
	psDst[nLen] = 0;
	return nLen;
}

/**
* @brief  Get the maximum line and column 
* @param [out] pnMaxLine maximum lines to display
* @param [out] pnLineCol maximum Cn Char to display(pnLineCol * 2 is the max columns)
* @return
* @li void
*/
void PubGetDispView(int* pnMaxLine, int* pnLineCol)
{
	uint unX, unY, unScrWidth, unScrHeight;

	NAPI_ScrGetViewPort(&unX, &unY, &unScrWidth, &unScrHeight);
	if(pnMaxLine != NULL)
	{
		*pnMaxLine = (unScrHeight+1)/gnDispLineHeight;
	}
	if(pnLineCol != NULL)
	{
		*pnLineCol = unScrWidth/gnDispFontSize;
	}	

}



/**
 *@brief    Display data in HEX format
* @param [in] pcData		Display data 
* @param [in] nLen			Length
 *@return
 *@li       APP_SUCC    Success
 *@li       APP_FAIL 	Fail
*/
int PubDispData (const char* pcData, int nLen )
{
	int i,j,nKey,nCurPos=0;
	int nMaxLine,nLineCol;
	char sBuf[512];

	if(pcData == NULL)
	{
		return APP_FAIL;
	}
	PubGetDispView(&nMaxLine, &nLineCol);
	while(1)
	{
		memset(sBuf, 0, sizeof(sBuf));
		for(i=0; i<nMaxLine; i++)
		{
			sprintf(sBuf+strlen(sBuf), "%03d:", (nCurPos+i*8)%1000);
			for(j=0; j<8; j++)
			{
				if(nCurPos+i*8+j < nLen)
				{
					sprintf(sBuf+strlen(sBuf), "%02X", pcData[nCurPos+i*8+j]);
				}
				else
				{
					goto DISPDATA;
				}
			}
			strcat(sBuf,"\n");
		}
DISPDATA:
		PubClearAll();
		PubDispMultLines(0, 2, 1, sBuf);
		NAPI_ScrRefresh();
GETKEYCODE:
		NAPI_KbGetCode(0, &nKey);
		switch(nKey)
		{
		case KEY_F1:
		case KEY_STAR:
			if(nCurPos-nMaxLine*8 >= 0)
			{
				nCurPos -= nMaxLine*8;
			}
			else
			{
				goto GETKEYCODE;
			}
			break;
		case KEY_F2:
		case KEY_SHARP:
		case KEY_ENTER:
			if(nCurPos + nMaxLine*8 < nLen)
			{
				nCurPos += nMaxLine*8;
			}
			else
			{
				goto GETKEYCODE;
			}
			break;
		case KEY_ESC:
			return APP_SUCC;
		default:
			continue;
		}
	}	
	return APP_SUCC;
}

/**
 *@brief   set font size for display and line space
 * @param [in]  nFontSize 		
 				nLineSpacing	
 *@return
 *@li       APP_SUCC    		Success
 *@li       APP_FAIL 			Failure
  @author  Ashin
  @data    2018-5-14 
*/
int PubSetDispForm(int nFontSize, int nLineSpacing)
{
	if((nFontSize <= 0) || (nLineSpacing < 0))
		return APP_FAIL;

	
	if(nLineSpacing == 0xff)//auto set a optinmal linespacing
	{
		nLineSpacing = nFontSize/4;

	}
	
	if(NAPI_ScrSetTrueTypeFontSizeInPixel(nFontSize, nFontSize)!=APP_SUCC)
		return APP_FAIL;

	NAPI_ScrSetSpace(0,nLineSpacing);
	gnDispFontSize = nFontSize;
	gnDispLineSpacing = nLineSpacing;
	gnDispLineHeight = nFontSize + nLineSpacing;

	gnTmpFontSize = nFontSize;
	gnTmpLineSpacing = nLineSpacing;
	gnTmpLineHeight = nFontSize + nLineSpacing;

	return APP_SUCC;
}


/**
 *@brief   get font size for display line space and line height
 * @param [out] pnFontSize 		
 				pnLineSpacing	
 				pnLineHeight
 *@return
 *@li       APP_SUCC    		Success
 *@li       APP_FAIL 			Failure
  @author  Ashin
  @data    2018-5-14
*/

int PubGetDispForm(int *pnFontSize, int *pnLineSpacing, int *pnLineHeight)
{
	if(pnFontSize)
		*pnFontSize = gnDispFontSize;
	if(pnLineSpacing)
		*pnLineSpacing = gnDispLineSpacing;
	if(pnLineHeight)
		*pnLineHeight = gnDispLineHeight;

	return APP_SUCC;
}

/**
 *@brief  Show a menu with items, the items format is XXX.abcd -->just shows abcd; or XXXabcd -->shows XXXabcd
 * @param [in]  pszTitle 		
 				ppMenuItems	
 				nMenuItemNum
 				pnSelectItem	
 				pnStartItem	
 				nTimeout
 * @param [out]	pnSelectItem
 				pnStartItem
 *@return
 *@li       APP_SUCC    		Success
 *@li       APP_FAIL 			Failure
  @author  wison
  @data    20201215 
*/
int PubShowVirtualMenuItems(char *pszTitle, char **ppMenuItems, int nMenuItemNum, int *pnSelectItem, int *pnStartItem, int nTimeout)
{
	uint unX = 0, unY = 0, unScrWidth = 0, unScrHeight = 0;
	int i, j = 0, k;
	uint unTitleLine = 0;
	ST_PADDATA stPaddata;
	int nCurrentPage = 0, nX, nY, nGap = 2, nItemX, nItemY;
	int nWidth, nHeight, nTxtX, nTxtY, nFunKeyWidth, nFunKeyHeight;
	int nTxtWidth, nTxtHeight;
	char szBuf[32] = {0};
	int nLine = 7, nColumn = 1, nFuncColumn = 3;
	int nPageItem = 6;
	int nPage, nSelectItem, nSelectLine = 0, nShowItem, nTmp = 0;
	
	if(nMenuItemNum <= 0 || ppMenuItems == NULL)
	{
		return APP_FAIL;
	}

	if (nMenuItemNum % nPageItem == 0) {
		nPage = nMenuItemNum / nPageItem - 1;
	} else {
		nPage = nMenuItemNum / nPageItem;
	}
	NAPI_ScrGetViewPort(&unX, &unY, &unScrWidth, &unScrHeight);
	if(pszTitle != NULL && strlen(pszTitle) > 0)
	{
		PubDisplayTitle(pszTitle);
		unTitleLine = 1;
	}

	if (nColumn == 1) {
		nWidth = unScrWidth; // 2 column
		nGap = 1;
	} else {
		nWidth = (unScrWidth - (nColumn+1) * nGap) / nColumn; // 2 column
	}
	nHeight = (unScrHeight - unTitleLine * gnDispLineHeight - (nLine+1) * nGap) / nLine; // 5 line
	if (nFuncColumn == 1) {
		nFunKeyWidth = unScrWidth;
	} else {
		nFunKeyWidth = (unScrWidth - (nFuncColumn+1) * nGap) / nFuncColumn;
	}
	nFunKeyHeight = (unScrHeight - (nLine-1) * nHeight - unTitleLine * gnDispLineHeight - nLine * nGap) - 3*nGap;
//	TRACE_UI("nMenuItemNum = %d nWidth = %d nHeight = %d nFunKeyWidth =%d nFunKeyHeight = %d", nMenuItemNum, nWidth, nHeight, nFunKeyWidth, nFunKeyHeight);
	while(1)
	{
		PubClear2To4();
		if (nPage == 0 || nPage == nCurrentPage) {
			if (nMenuItemNum % nPageItem == 0) {
				nShowItem = nPageItem;
			} else {
				nShowItem = nMenuItemNum % nPageItem;
			}
		} else {
			nShowItem = nPageItem;
		}
//		TRACE_UI("nPage = %d nCurrentPage = %d nShowItem = %d", nPage, nCurrentPage, nShowItem);
		//Display menu from 2nd line to the last line
		for(j = 0; j < nShowItem; j++)
		{
			i = nCurrentPage*nPageItem + j;
			if (nColumn == 1) {
				nItemX = 0;
			} else {
				nItemX = nGap + (i%nColumn)*(nGap+nWidth);
			}
			nItemY = nGap + unTitleLine * gnDispLineHeight + (j/nColumn) * (nHeight+nGap);
			if (*pnSelectItem == i+1) {
				NAPI_ScrRectangle(nItemX, nItemY, nWidth, nHeight, RECT_PATTERNS_SOLID_FILL, COLOR_DGREY);
			} else {
				NAPI_ScrRectangle(nItemX, nItemY, nWidth, nHeight, RECT_PATTERNS_SOLID_FILL, COLOR_LGRAY);
			}
			//check the first three bytes, such as menu is 1.item1 -->it just shows item1
			for (k = 0, nTmp = 0; k < 3; k++)
			{
				if (ppMenuItems[i][k] == '.') {
					nTmp++;
					break;
				}
				nTmp++;
			}
			if (k >= 3) {
				nTmp = 0;
			}
			strcpy(szBuf, ppMenuItems[i]+nTmp);
			NAPI_ScrGetTrueTypeFontSize(szBuf, strlen(szBuf), &nTxtWidth, &nTxtHeight);
			if(nTxtWidth >= nWidth) {
				nTxtWidth = nWidth;
			}
			nTxtX = (nWidth - nTxtWidth)/2 + nItemX;
			nTxtY = (nHeight - nTxtHeight)/2 + nItemY;
//			TRACE_UI("ppMenuItems[%d] = %s j = %d nItemX = %d nItemY = %d nTxtX = %d %d", i, ppMenuItems[i], j, nItemX, nItemY, nTxtX, nTxtY);
			DispBoldFontText(nTxtX, nTxtY, ppMenuItems[i]+nTmp);
		}

		// select Key
		{
//			TRACE_UI("show funckey");
			nX = nGap;
			nY = (nLine-1) * nHeight + (nLine+1) * nGap + unTitleLine * gnDispLineHeight;

			if (nCurrentPage == 0) {
				NAPI_ScrRectangle(nX, nY, nFunKeyWidth, nFunKeyHeight, RECT_PATTERNS_SOLID_FILL, 0xFFFF);
			} else {
				NAPI_ScrRectangle(nX, nY, nFunKeyWidth, nFunKeyHeight, RECT_PATTERNS_SOLID_FILL, 0x07E0);
				NAPI_ScrGetTrueTypeFontSize("UP", strlen("UP"), &nTxtWidth, &nTxtHeight);
				nTxtX = (nFunKeyWidth - nTxtWidth)/2 + nX;
				nTxtY = (nFunKeyHeight - nTxtHeight)/2 + nY;
				DispBoldFontText(nTxtX, nTxtY , "UP");
			}

			nX += (nGap + nFunKeyWidth);
			NAPI_ScrRectangle(nX, nY, nFunKeyWidth, nFunKeyHeight, RECT_PATTERNS_SOLID_FILL, 0xF800);
			NAPI_ScrGetTrueTypeFontSize("CANCEL", strlen("CANCEL"), &nTxtWidth, &nTxtHeight);
			nTxtX = (nFunKeyWidth - nTxtWidth)/2 + nX;
			nTxtY = (nFunKeyHeight - nTxtHeight)/2 + nY;
			DispBoldFontText(nTxtX, nTxtY , "CANCEL");

			nX += (nGap + nFunKeyWidth);
			if (nCurrentPage == nPage) {
				NAPI_ScrRectangle(nX, nY, nFunKeyWidth, nFunKeyHeight, RECT_PATTERNS_SOLID_FILL, 0xFFFF);
			} else {
				NAPI_ScrRectangle(nX, nY, nFunKeyWidth, nFunKeyHeight, RECT_PATTERNS_SOLID_FILL, 0x07E0);
				NAPI_ScrGetTrueTypeFontSize("DOWN", strlen("DOWN"), &nTxtWidth, &nTxtHeight);
				nTxtX = (nFunKeyWidth - nTxtWidth)/2 + nX;
				nTxtY = (nFunKeyHeight - nTxtHeight)/2 + nY;
				DispBoldFontText(nTxtX, nTxtY , "DOWN");
			}
		}
		NAPI_ScrRefresh();
GETKEYPAD:
		TRACE_UI("start NAPI_KbGetPad");
		NAPI_KbVppTpFlush();
		while (1) {
			if(NAPI_KbGetPad(&stPaddata, nTimeout*1000) == NAPI_ERR_TIMEOUT)
			{
				return APP_TIMEOUT;
			}
			if(stPaddata.emPadState == PADSTATE_UP) {
				break;
			}
		}
		TRACE_UI("stPaddata.emPadState = %d unPadX = %d unPadY = %d", stPaddata.emPadState, stPaddata.unPadX, stPaddata.unPadY);
		if(stPaddata.unPadY <=0 )//touch bar
			continue;
		
		if (stPaddata.unPadY < nY) { // >= nY is funcKey area nY = 4 * nHeight + 6 * nGap + unTitleLine * gnDispLineHeight
			nSelectLine = 0;
			for (i = 0; i < nLine; i++) {
				if (stPaddata.unPadY > i * (nGap + nHeight) + nGap + unTitleLine * gnDispLineHeight) {
					nSelectLine++;
				}
			}
			if (nSelectLine == 0)
			{
				goto GETKEYPAD;
			}
			if (nColumn == 1) {
				nSelectItem = nSelectLine;
			} else {
				if (stPaddata.unPadX <= nGap + nWidth) {
					nSelectItem = (nSelectLine - 1)*nColumn+1;
				} else {
					nSelectItem = nSelectLine * nColumn;
				}
			}
			TRACE_UI("ncurrent page = %d nSelectItem = %d nSelectLine = %d", nCurrentPage, nSelectItem, nSelectLine);
			*pnSelectItem = nSelectItem + nPageItem*nCurrentPage;
			if (*pnSelectItem > nMenuItemNum) {
				goto GETKEYPAD;
			}
			
			return APP_SUCC;
		} else { // func key area
			if (stPaddata.unPadX <= nGap + nFunKeyWidth) { // up-->previous page
				if (nCurrentPage > 0) {
					nCurrentPage--;
				}
			} else if (stPaddata.unPadX <= 2*(nGap + nFunKeyWidth)) {
				*pnSelectItem = KEY_ESC;
				return APP_QUIT;
			} else { // next page
				if (nCurrentPage < nPage) {
					nCurrentPage++;
				}
			}
			TRACE_UI("ncurrent page = %d", nCurrentPage);
		}
	}
	return APP_SUCC;
}

/**
 *@brief  Show a menu with items
 * @param [in]  pszTitle 		
 				ppMenuItems	
 				nMenuItemNum
 				pnSelectItem	
 				pnStartItem	
 				nTimeout
 * @param [out]	pnSelectItem
 				pnStartItem
 *@return
 *@li       APP_SUCC    		Success
 *@li       APP_FAIL 			Failure
  @author  Ashin
  @data    2018-5-14 
*/
int PubShowMenuItems(char *pszTitle, char **ppMenuItems, int nMenuItemNum, int *pnSelectItem, int *pnStartItem, int nTimeout)
{
	uint unX = 0, unY = 0, unScrWidth = 0, unScrHeight = 0;
	int nMaxLineNum = 0, nMaxMenuItemLineNum = 0, nRemainLine;
	int i, j = 0, nCurrentItem = 0;
	int nKey;
	uint unTitleLine = 0;
	uint unLineNum = 1;
	ST_PADDATA stPaddata;
	int nTouchline, nTouchItem, nRealTouchLine;
	BOOL bRefleshAndReturn = 0;

	if (PubGetKbAttr() == KB_VIRTUAL)
	{
		return PubShowVirtualMenuItems(pszTitle, ppMenuItems, nMenuItemNum, pnSelectItem, pnStartItem, nTimeout);
	}

	if(nMenuItemNum <= 0 || ppMenuItems == NULL)
	{
		return APP_FAIL;
	}
	
	NAPI_ScrGetViewPort(&unX, &unY, &unScrWidth, &unScrHeight);
	
	nMaxLineNum = unScrHeight / gnDispLineHeight;
	//TRACE("\n nMaxLineNum:%d, %d\n",nMaxLineNum, unScrHeight % gnDispLineHeight);
	//TRACE("unScrHeight:%d,gnDispLineHeight:%d,gnDispFontSize:%d, gnDispLineSpacing:%d\n",unScrHeight,gnDispLineHeight,gnDispFontSize,gnDispLineSpacing);

	
	nMaxMenuItemLineNum = nMaxLineNum;
	if(pszTitle != NULL && strlen(pszTitle) > 0)
	{
		nMaxMenuItemLineNum = nMaxLineNum - 1;
	}
	//TRACE(("unMaxHeight:%d, nMaxLineNum:%d", gnDispLineHeight, nMaxLineNum));
	
	if(pnSelectItem != NULL && *pnSelectItem != 0xFF && *pnSelectItem > 0 && *pnSelectItem <= nMenuItemNum)
	{
		nCurrentItem = *pnSelectItem - 1;
	}
	if(pnStartItem != NULL && *pnStartItem != 0xFF && *pnStartItem > 0 && *pnStartItem <= nMenuItemNum)
	{
		if(nCurrentItem >= (*pnStartItem + nMaxMenuItemLineNum - 1))
		{
			j = nCurrentItem - nMaxMenuItemLineNum + 1;
		}
		else if(nCurrentItem < (*pnStartItem - 1))
		{
			j = nCurrentItem;
		}
		else
		{
			j = *pnStartItem - 1;
		}
	}
	//TRACE(("nCurrentItem:%d, j:%d", nCurrentItem, j));


	if(pszTitle != NULL && strlen(pszTitle) > 0)
	{
		PubDisplayTitle(pszTitle);
		// unTitleHight = gnDispLineHeight;
		unTitleLine = 1;
	}

	
	while(1)
	{
		nRemainLine = nMaxLineNum - unTitleLine;
		unLineNum = 1 + unTitleLine;
		
		PubClear2To4();
		
		//Display menu from 2nd line to the last line
		for(i = j; (nRemainLine > 0) && (i < nMenuItemNum); i++, nRemainLine--)
		{
			//printf("i:%d, nRemainLine:%d,unLineNum:%d \n\r", i,  nRemainLine, unLineNum);
			//printf("ppMenuItems[i]:%s\n\r",ppMenuItems[i]);
		
			if(i == nCurrentItem)
			{
				PubDisplayInv(unLineNum, ppMenuItems[i]);
			}
			else
			{
				PubDisplayStrInline(0, unLineNum, ppMenuItems[i]);
				
			}
			unLineNum++;
		}
		NAPI_ScrRefresh();
		if(bRefleshAndReturn)
			return APP_SUCC;
		if(NAPI_KbGetPad(&stPaddata, nTimeout*1000) == NAPI_ERR_TIMEOUT)
		{
			return APP_TIMEOUT;
		}
		
		if(stPaddata.emPadState == PADSTATE_KEY)
		{
			nKey = stPaddata.unKeycode;
			if(nKey == KEY_F1 || nKey ==K_DOT)
			{
				nCurrentItem--;
				if(nCurrentItem < j)
				{
					j--;
				}
				if(nCurrentItem < 0)
				{
					nCurrentItem = nMenuItemNum - 1;
					if(nMenuItemNum > nMaxMenuItemLineNum)
						j = nMenuItemNum - nMaxMenuItemLineNum;
					else
						j = 0;
				}
			}
			else if(nKey == KEY_F2 ||nKey ==K_ZMK)
			{
				if(nCurrentItem >= i - 1)
				{
					j++;
				}
				nCurrentItem++;
				if(nCurrentItem >= nMenuItemNum)
				{
					nCurrentItem = 0;
					j = 0;
				}
			}
			else if(nKey == 0)
			{
				return APP_TIMEOUT;
			}
			else if(nKey == KEY_ESC)
			{
				return APP_QUIT;
			}
			else if(nKey >= KEY_1 && nKey <= KEY_9)
			{
				if((nKey - KEY_0) <= nMenuItemNum)
				{
					if(pnSelectItem != NULL)
					{
						*pnSelectItem = nKey - KEY_0;
					}
					if(pnStartItem != NULL)
					{
						*pnStartItem = j + 1;
					}
					return APP_SUCC;
				}
			}
			else if(nKey == KEY_ENTER)
			{
				if(pnSelectItem != NULL)
				{
					*pnSelectItem = nCurrentItem + 1;
				}
				if(pnStartItem != NULL)
				{
					*pnStartItem = j + 1;
				}
				return APP_SUCC;
			}
		}
		else if(stPaddata.emPadState == PADSTATE_UP)
		{
			if(stPaddata.unPadY <=0 )//touch bar
				continue;
			
			nTouchline = stPaddata.unPadY/gnDispLineHeight + 1 ;
			nRealTouchLine = nTouchline - unTitleLine;
			//printf("\nY:%d j:%d nRealTouchLine:%d\n",stPaddata.unPadY,j,nRealTouchLine);
			if(nRealTouchLine>0 && nRealTouchLine<= nMenuItemNum && nTouchline <= nMaxLineNum )
			{	
				nTouchItem = j + nRealTouchLine ;
				nCurrentItem = nTouchItem-1;

				//printf("nTouchItem :%d nCurrentItem:%d\n",nTouchItem,nCurrentItem);
				if(pnSelectItem != NULL)
				{
					*pnSelectItem = nTouchItem;
				}
				if(pnStartItem != NULL)
				{
					*pnStartItem = j + 1;
				}
				bRefleshAndReturn = 1;
			}	
		}
	}
}

/**
 *@brief  Show a menu with items, the items format is XXX.abcd -->just shows abcd; or XXXabcd -->shows XXXabcd
 * @param [in]  pszTitle 		
 				ppMenuItems	
 				nMenuItemNum
 				pnSelectItem	
 				pnStartItem	
 				nTimeout
 * @param [out]	pnSelectItem
 				pnStartItem
 *@return
 *@li       APP_SUCC    		Success
 *@li       APP_FAIL 			Failure
  @author  wison
  @data    2020-12-15 
*/
int PubShowMenuItems_Ext(char *pszTitle, char **ppMenuItems, int nMenuItemNum, int *pnSelectItem, int *pnStartItem, int nTimeout)
{
	uint unX = 0, unY = 0, unScrWidth = 0, unScrHeight = 0;
	int i, j = 0;
	uint unTitleLine = 0;
	ST_PADDATA stPaddata;
	int nCurrentPage = 0, nX, nY, nGap = 2, nItemX, nItemY;
	int nWidth, nHeight, nTxtX, nTxtY, nFunKeyWidth, nFunKeyHeight;
	int nTxtWidth, nTxtHeight;
	char szBuf[32] = {0};

	int nPage, nSelectItem, nSelectLine = 0, nShowItem, nTmp = 0, k;
	
	if(nMenuItemNum <= 0 || ppMenuItems == NULL)
	{
		return APP_FAIL;
	}

	nPage = nMenuItemNum / 8;

	NAPI_ScrGetViewPort(&unX, &unY, &unScrWidth, &unScrHeight);
	if(pszTitle != NULL && strlen(pszTitle) > 0)
	{
		PubDisplayTitle(pszTitle);
		unTitleLine = 1;
	}

	nWidth = (unScrWidth - 3 * nGap) / 2; // 2 column
	nHeight = (unScrHeight - unTitleLine * gnDispLineHeight - 6 * nGap) / 5; // 5 line
	nFunKeyWidth = (unScrWidth - 4 * nGap) / 3;
	nFunKeyHeight = (unScrHeight - 4 * nHeight - unTitleLine * gnDispLineHeight - 5 * nGap) - 3*nGap;
	TRACE_UI("nMenuItemNum = %d nWidth = %d nHeight = %d nFunKeyWidth =%d nFunKeyHeight = %d", nMenuItemNum, nWidth, nHeight, nFunKeyWidth, nFunKeyHeight);
	while(1)
	{
		PubClear2To4();
		
		if (nPage == 0 || nPage == nCurrentPage) {
			nShowItem = nMenuItemNum % 8;
		} else {
			nShowItem = 8;
		}
		TRACE_UI("nPage = %d nCurrentPage = %d nShowItem = %d", nPage, nCurrentPage, nShowItem);
		//Display menu from 2nd line to the last line
		for(j = 0; j < nShowItem; j++)
		{
			i = nCurrentPage*8 + j;
			nItemX = nGap + (i%2)*(nGap+nWidth);
			nItemY = nGap + unTitleLine * gnDispLineHeight + (j/2) * (nHeight+nGap);

			NAPI_ScrRectangle(nItemX, nItemY, nWidth, nHeight, RECT_PATTERNS_SOLID_FILL, 0xC618);
			//check the first three bytes, such as menu is 1.item1 -->it just shows item1
			for (k = 0, nTmp = 0; k < 3; k++)
			{
				if (ppMenuItems[i][k] == '.') {
					nTmp++;
					break;
				}
				nTmp++;
			}
			if (k >= 3) {
				nTmp = 0;
			}
			strcpy(szBuf, ppMenuItems[i]+nTmp);

			NAPI_ScrGetTrueTypeFontSize(szBuf, strlen(szBuf), &nTxtWidth, &nTxtHeight);
			if(nTxtWidth >= nWidth) {
				nTxtWidth = nWidth;
			}
			nTxtX = (nWidth - nTxtWidth)/2 + nItemX;
			nTxtY = (nHeight - nTxtHeight)/2 + nItemY;
			//TRACE_UI("ppMenuItems[%d] = %s j = %d nItemX = %d nItemY = %d nTxtX = %d %d", i, ppMenuItems[i], j,nItemX, nItemY, nTxtX, nTxtY);
			DispBoldFontText(nTxtX, nTxtY, ppMenuItems[i]+nTmp);
		}

		// select Key
		{
			TRACE_UI("show funckey");
			nX = nGap;
			nY = 4 * nHeight + 6 * nGap + unTitleLine * gnDispLineHeight;

			NAPI_ScrRectangle(nX, nY, nFunKeyWidth, nFunKeyHeight, RECT_PATTERNS_SOLID_FILL, COLOR_GREEN);
			NAPI_ScrGetTrueTypeFontSize("UP", strlen("UP"), &nTxtWidth, &nTxtHeight);
			nTxtX = (nFunKeyWidth - nTxtWidth)/2 + nX;
			nTxtY = (nFunKeyHeight - nTxtHeight)/2 + nY;
			DispBoldFontText(nTxtX, nTxtY , "UP");

			nX += (nGap + nFunKeyWidth);
			NAPI_ScrRectangle(nX, nY, nFunKeyWidth, nFunKeyHeight, RECT_PATTERNS_SOLID_FILL, COLOR_RED);
			NAPI_ScrGetTrueTypeFontSize("HOME", strlen("HOME"), &nTxtWidth, &nTxtHeight);
			nTxtX = (nFunKeyWidth - nTxtWidth)/2 + nX;
			nTxtY = (nFunKeyHeight - nTxtHeight)/2 + nY;
			DispBoldFontText(nTxtX, nTxtY , "HOME");

			nX += (nGap + nFunKeyWidth);
			NAPI_ScrRectangle(nX, nY, nFunKeyWidth, nFunKeyHeight, RECT_PATTERNS_SOLID_FILL, COLOR_GREEN);
			NAPI_ScrGetTrueTypeFontSize("DOWN", strlen("DOWN"), &nTxtWidth, &nTxtHeight);
			nTxtX = (nFunKeyWidth - nTxtWidth)/2 + nX;
			nTxtY = (nFunKeyHeight - nTxtHeight)/2 + nY;
			DispBoldFontText(nTxtX, nTxtY , "DOWN");
		}
		NAPI_ScrRefresh();
GETKEYPAD:
		NAPI_KbVppTpFlush();
		TRACE_UI("start NAPI_KbGetPad");
		while (1) {
			if(NAPI_KbGetPad(&stPaddata, nTimeout*1000) == NAPI_ERR_TIMEOUT)
			{
				return APP_TIMEOUT;
			}
			if(stPaddata.emPadState == PADSTATE_UP) {
				break;
			}
		}

		TRACE_UI("stPaddata.emPadState = %d unPadX = %d unPadY = %d", stPaddata.emPadState, stPaddata.unPadX, stPaddata.unPadY);
		if(stPaddata.unPadY <=0 )//touch bar
			continue;
		
		if (stPaddata.unPadY < nY) { // >= nY is funcKey area nY = 4 * nHeight + 6 * nGap + unTitleLine * gnDispLineHeight
			nSelectLine = 0;
			for (i = 0; i < 4; i++) {
				if (stPaddata.unPadY > i * (nGap + nHeight) + nGap + unTitleLine * gnDispLineHeight) {
					nSelectLine++;
				}
			}
			if (nSelectLine == 0)
			{
				goto GETKEYPAD;
			}
			if (stPaddata.unPadX <= nGap + nWidth) {
				nSelectItem = (nSelectLine - 1)*2+1;
			} else {
				nSelectItem = nSelectLine * 2;
			}
			TRACE_UI("ncurrent page = %d nSelectItem = %d nSelectLine = %d", nCurrentPage, nSelectItem, nSelectLine);
			*pnSelectItem = nSelectItem + 8*nCurrentPage;
			if (*pnSelectItem > nMenuItemNum) {
				goto GETKEYPAD;
			}
			return APP_SUCC;
		} else { // func key area
			if (stPaddata.unPadX <= nGap + nFunKeyWidth) { // up-->previous page
				if (nCurrentPage > 0) {
					nCurrentPage--;
				}
			} else if (stPaddata.unPadX <= 2*(nGap + nFunKeyWidth)) {
				*pnSelectItem = KEY_ESC;
				return APP_QUIT;
			} else { // next page
				if (nCurrentPage < nPage) {
					nCurrentPage++;
				}
			}
			TRACE_UI("ncurrent page = %d", nCurrentPage);
		}
	}
}

/**
 *@brief  Prompt a menu with 2 options
 * @param [in]  pszTitle 		
 				pszStr	
 				SelMenu		if it is NULL, use the default options:Yes and No
 				pcSelect
 * @param [out]	pcSelect
 				
 *@return
 *@li       APP_SUCC    		Success
 *@li       APP_FAIL 			Failure
  @author  Ashin
  @data    2018-5-14 
*/

int PubSelectYesOrNo(char *pszTitle, char *pszStr, char* SelMenu[], char *pcSelect)
{
	int nSelect;
	int nKey;
	char *pStr0;
	char *pStr1;
	int nMaxLine;
	ST_PADDATA stPaddata;
	int nTouchline;
	BOOL bRefleshAndReturn = 0;

	uint unSWidth1 , unSWidth2,unSHeight, unScrWidth ,unScrHeight, unX, unY;
	int x1, y;


	if(SelMenu && SelMenu[0] &&  SelMenu[1])
	{
		pStr0 = SelMenu[0];
		pStr1 = SelMenu[1];		

	}
	else
	{
		pStr0 = (char*)("0.NO");
		pStr1 = (char*)("1.YES");
	}
	
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


	PubGetDispView(&nMaxLine, NULL);


	PubClear2To4();
	if(pszTitle)
		PubDisplayTitle(pszTitle);

	if(pszStr)
		PubDisplayStrInline(0, 2, pszStr);

	if (PubGetKbAttr() == KB_VIRTUAL) {
		NAPI_KbVppTpFlush();
	}

	while(1)
	{
		if (PubGetKbAttr() == KB_VIRTUAL) {
			NAPI_ScrGetViewPort(&unX, &unY, &unScrWidth,&unScrHeight);
			if(nSelect == 0) {
				NAPI_ScrRectangle(0, 240-gnDispLineHeight*2, unScrWidth,gnDispLineHeight*2, RECT_PATTERNS_SOLID_FILL, COLOR_LGRAY);
				NAPI_ScrRectangle(0, 240, unScrWidth,gnDispLineHeight*2, RECT_PATTERNS_SOLID_FILL, COLOR_WHITE);
				NAPI_ScrRectangle(0, 240+gnDispLineHeight*2, unScrWidth,0, RECT_PATTERNS_SOLID_FILL, COLOR_LGRAY);
			} else {
				NAPI_ScrRectangle(0, 240-gnDispLineHeight*2, unScrWidth,0, RECT_PATTERNS_SOLID_FILL, COLOR_LGRAY);
				NAPI_ScrRectangle(0, 240-gnDispLineHeight*2, unScrWidth,gnDispLineHeight*2, RECT_PATTERNS_SOLID_FILL, COLOR_WHITE);
				NAPI_ScrRectangle(0, 240, unScrWidth,gnDispLineHeight*2, RECT_PATTERNS_SOLID_FILL, COLOR_LGRAY);
			}

			NAPI_ScrGetTrueTypeFontSize(pStr0+2, strlen(pStr0+2), (int*)&unSWidth1, (int*)&unSHeight);
			NAPI_ScrGetTrueTypeFontSize(pStr1+2, strlen(pStr1+2), (int*)&unSWidth2, (int*)&unSHeight);
			x1 = (unScrWidth - unSWidth1)/2;
			y = 240 + (gnDispLineHeight*2-unSHeight)/2 - gnDispLineHeight*2;
			DispBoldFontText(x1, y, pStr0+2);
			y = 240 + (gnDispLineHeight*2-unSHeight)/2;
			DispBoldFontText(x1 ,y, pStr1+2);
			PubShowButton(BUTTON_CONFIRM, "CANCEL", "ENTER", NULL, NULL);
		} else {
			if(nSelect == 0)
				PubDisplayInv(nMaxLine-1 , pStr0);
			else
				PubDisplayStrInline(DISPLAY_MODE_CLEARLINE , nMaxLine-1, pStr0);
	
			if(nSelect == 1)
				PubDisplayInv(nMaxLine, pStr1);
			else
				PubDisplayStrInline(DISPLAY_MODE_CLEARLINE , nMaxLine, pStr1);
		}

		NAPI_ScrRefresh();
		
		if(bRefleshAndReturn)
			return APP_SUCC;
		if(NAPI_KbGetPad(&stPaddata, 0) == NAPI_ERR_TIMEOUT)
		{
			return APP_TIMEOUT;
		}	

		if(stPaddata.emPadState == PADSTATE_KEY)
		{
			nKey = stPaddata.unKeycode;
			if(nKey == KEY_1)
			{
				*pcSelect = '1';
				return APP_SUCC;
			}
			else if(nKey == KEY_0)
			{
				*pcSelect = '0';
				return APP_SUCC;
			}		
			else if(nKey == KEY_ESC)
			{
				return APP_QUIT;
			}
			else if(nKey == KEY_F1 || nKey == KEY_STAR)
			{
				if(nSelect == 0XFF)
					nSelect = 1;
				else
					nSelect = (nSelect + 1)%2;
			}
			else if(nKey == KEY_F2 || nKey == KEY_SHARP)
			{
				if(nSelect == 0XFF)
					nSelect = 0;
				else
					nSelect = (nSelect + 1)%2;
			}			
			else if(nKey == KEY_ENTER)
			{
				if(nSelect != 0XFF)
				{
					*pcSelect = nSelect + '0';
					return APP_SUCC;
				}
			}
		}	
		else if(stPaddata.emPadState == PADSTATE_UP)
		{
			if(stPaddata.unPadY <=0 )//touch bar
				continue;
			if (PubGetKbAttr() == KB_VIRTUAL)
			{
				if (stPaddata.unPadY >= 240-2*gnDispLineHeight && stPaddata.unPadY < 240) {
					*pcSelect = '0';
					return APP_SUCC;
				} else if (stPaddata.unPadY >= 240 && stPaddata.unPadY < 240+2*gnDispLineHeight) {
					*pcSelect = '1';
					return APP_SUCC;
				}else {
					nTouchline = stPaddata.unPadY/gnDispLineHeight ;
					if(nTouchline >= nMaxLine-2)
					{
						if(stPaddata.unPadX < unScrWidth/2 ) {
							return APP_QUIT;
						} else {
							*pcSelect = nSelect + '0';
							return APP_SUCC;
						}
					}
				}
			}
			else
			{
				nTouchline = stPaddata.unPadY/gnDispLineHeight + 1 ;
				if((nTouchline == nMaxLine - 1) || (nTouchline == nMaxLine))
				{
					nSelect = (nMaxLine - nTouchline+1)%2;
					*pcSelect = nSelect + '0';
					bRefleshAndReturn = 1;
				}
			}
		}

			
	}

	return APP_SUCC;
}


void ProDisplayInvs(int nAlign, int nMode, int nLineno, char *pstr)
{
	if(pstr == NULL || nLineno < 1)
	{
		return;
	}

	if(nAlign == DISPLAY_ALIGN_BIGFONT)
	{
		return ProDisplayInv(nMode, nLineno, pstr);
	}	

	if(nAlign == DISPLAY_ALIGN_SMALLFONT)
	{
		ProSwitchFontSize(DISPLAY_ALIGN_SMALLFONT);
	}

	
	ProDisplayInv(nMode, nLineno, pstr);

	if(nAlign == DISPLAY_ALIGN_SMALLFONT)//restore
	{
		ProSwitchFontSize(DISPLAY_ALIGN_BIGFONT);
	}	
}


/**
* @brief Inverse diplay string (large font)
* @param [in] pstr    Eenter string
* @param [in] nLineno Appointed line
* @return 
* @li void
*/
void ProDisplayInv(int nMode, int nLineno, char *pStr)
{
	int x, y;
	uint unLHeight, unLWidth, unSWidth, unSHeight;
	
	if(pStr == NULL || nLineno < 1)
	{
		return;
	}

	NAPI_ScrGetLcdSize  (&unLWidth, &unLHeight);
	if(strlen(pStr))
		NAPI_ScrGetTrueTypeFontSize(pStr, strlen(pStr), (int *)&unSWidth, (int *)&unSHeight);
	else
		unSWidth = 0;


	if (nMode & DISPLAY_MODE_CLEARLINE)
	{
		PubClearLine(nLineno, nLineno);
	}
	
	if(nMode & DISPLAY_MODE_CENTER)
	{
		if(unSWidth < unLWidth)
			x = (unLWidth - unSWidth)/2;
		else
			x = 0;		
	}
	else if(nMode & DISPLAY_MODE_TAIL)
	{
		if(unSWidth < unLWidth)
			x = unLWidth - unSWidth;
		else
			x = 0;
	}
	else
	{
		x = 0;
	}


	y = (nLineno-1)*gnDispLineHeight;
	NAPI_ScrRectangle(0, y, unLWidth,gnDispLineHeight, RECT_PATTERNS_SOLID_FILL, DISP_INVBG_COLOUR);//Draw black rectangle
	y += (gnDispLineHeight - unSHeight)/2;
	DispBoldFontText(x ,y , pStr);		
	
}

void ProSwitchFontSize(EM_DISPLAY_ALIGN emFont)
{
	if(emFont == DISPLAY_ALIGN_SMALLFONT)
	{
		//switch to small font
		gnDispFontSize /= 2;
		gnDispLineSpacing /= 2;
		gnDispLineHeight = gnDispFontSize + gnDispLineSpacing;		
	}
	else
	{
		//restore to big font
		gnDispFontSize = gnTmpFontSize;
		gnDispLineSpacing = gnTmpLineSpacing;
		gnDispLineHeight =gnTmpLineHeight;	
		
	}
	NAPI_ScrSetTrueTypeFontSizeInPixel(gnDispFontSize, gnDispFontSize);
}

void PubSetKbAttr(EM_KEYBOARD_ATTR emKbAttr)
{
	gemKeyboardAttr = emKbAttr;
}

EM_KEYBOARD_ATTR PubGetKbAttr()
{
	return gemKeyboardAttr;
}



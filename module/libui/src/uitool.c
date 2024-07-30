#include <string.h>
#include <stdlib.h>
#include "ltool.h"
#include "uitool.h"
#include "lui.h"
#include "lmenu.h"
#include "libapiinc.h"

#define GREENFONT		0x3384
#define REDFONT			0xFACB
#define BLACKFONT		0x0000
#define WHITEFONT		0xFFFF


static uchar gucIsUpDownMenu = 0; 	/**<Support page-up/down or not*/


int UI_ChkIsUpDownMenu()
{
    return gucIsUpDownMenu;
}
	
/**
* @brief Multi menu-pages management.
* @details Supporting moving to previous and next menu page in order among the menu page functions which are listed in the 'plSetFuns'.
* <pre> 	
*	e.g.
*	int func(void)
*	{
*		//When KEY_F1 or KEY_F2 is pressed, 'func' should return KEY_F1 or KEY_F2, 
*		//in order to achieve function of scrolling up and down.
*	}
*	int func2(void){//Similar to func}
*	int func3(void){//Similar to func}
*
*	int (*lSetFuns[])(void) = 
*	{
*		func,
*		func2,
*		func3,
*		NULL
*	};
*	PubUpDownMenus(lSetFuns);
* </pre>
* @param [in] plSetFuns  Function list (usually used in page settings)
* @return
* @li		APP_SUCC  Success
* @li		APP_QUIT  Cancel
* @li		APP_FAIL  Fail
*/
int PubUpDownMenus(int (*plSetFuns[])(void))
{
	int i=0, nRet, nOldRet = 0;

	if (plSetFuns == NULL)
	{
	   return APP_FAIL;
	}
	gucIsUpDownMenu = 1;

	while (1)
	{
		nRet = plSetFuns[i]();
		switch(nRet)
		{
		case APP_SUCC:
		case KEY_DOWN:
			if (plSetFuns[i+1]  != NULL)
			{
				i++;
			}
			else
			{
				gucIsUpDownMenu = 0;
				return APP_SUCC;
			}
			nOldRet = nRet;
			break;
		case KEY_UP:
			i--;
			if (i < 0 )
			{
			    i = 0;
			}
			nOldRet = nRet;
			break;
		case APP_QUIT:
			gucIsUpDownMenu = 0;
			return APP_QUIT;
			break;
		case APP_FUNCQUIT:
			if(nOldRet == KEY_DOWN || nOldRet == APP_SUCC)
			{
			    if (plSetFuns[i+1]  != NULL)
				{
					i++;
				}
				else
				{
					gucIsUpDownMenu = 0;
					return APP_SUCC;
				}
			}
			else if(nOldRet == KEY_UP)
			{
				i--;
				if (i < 0 )
				{
				    i = 0;
					nOldRet = KEY_DOWN;
				}
			}
			break;
		default:
		case APP_FAIL:
			gucIsUpDownMenu = 0;
			return APP_FAIL;
			break;
		}
	}
	gucIsUpDownMenu = 0;
	return APP_SUCC;
}

void DispBigASC(int x, int y, char c)
{
	int	nX = 0, nY = 0;
	uint unBigFontCol;
	uint unX,unY;
	uint unScrWidth,unScrHeight;
	uint unFontWidth,unFontHeight;
	char szBuf[8] = {0};
	int nLineHeight;
	
	NAPI_ScrGetViewPort(&unX, &unY, &unScrWidth,&unScrHeight);
	NAPI_ScrGetFontSize(&unFontWidth, &unFontHeight);
	PubGetDispForm(NULL, NULL ,&nLineHeight);
	
	unBigFontCol = unScrWidth/(unFontWidth/2);

	nX = x  % unBigFontCol;
	nY = y + (x / unBigFontCol);
	sprintf(szBuf, "%c", c);
	NAPI_ScrDispTrueTypeFontText(nX*(unFontWidth/2), nY*nLineHeight , szBuf, strlen(szBuf));
	return;
}

int DispBigAscStr(int x, int y, const char *pszStr, int nNum)
{
	int	i = 0;

	if(pszStr == NULL)
	{
		return APP_FAIL;
	}
	for (i = 0; i < nNum; ++i)
	{
		if (pszStr[i] != 0)
		{
			DispBigASC((x + i), y, pszStr[i]);
		}
		else
		{
			return i;
		}
	}
	return i;
}

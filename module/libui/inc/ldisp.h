#ifndef _LDISP_H_
#define _LDISP_H_

#define	SELECT_ITEM_COLOUR	0xC618
#define TITLE_BG_COLOUR		0x0000
#define	DISP_INVBG_COLOUR	0xC618

void DispBoldFontText(uint unX, uint unY, const char *str);
void GetStrFormat(int nFont, char *pOutStr, char *pInStr);
void TrimStrToOneLine(int nStartX, char *pszStr, int nStrlen, int *pnTrimedLen, int *nEndX);
int GetChaWidth(char c);
void GetLcdWidth(uint *punWidth);
int GetStrWidth(char* pszStr);
void DisplayLogo(char *pszPath, int nX, int nY);

#endif


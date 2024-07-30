#ifndef _LMENU_H_
#define _LMENU_H_

#define PAGEUPTIP   	1
#define PAGEDOWNTIP 	2

void DisplayTitleWithStyle(char *pszTitle, int nUpDown);
void DrawBitmapList(const uint ,const uint ,const char *,const char *,int ,int* );
void DispPicList(const uint ,const uint ,const char *,const char *,int ,int* );
int UpDownMsgDlg(const char* pszTitle,const char* pszContent,int nIsResKey,int nTimeOut, int* nResKey);
int UpDownConfirmDlg(const char* pszTitle,const char* pszContent,int nBeep, int nTimeOut);

#endif


#ifndef _LINPUT_H_
#define _LINPUT_H_


int InputDlg(const char *pszTitle, const char *pszContent, char *pszOut, int *pnOutLen, 
	int nMinLen, int nMaxLen,int nTimeOut,int nEditMask);

int Inputxy(int nX, int nY, char* pszOut, int* pnOutLen, int nMinLen, int nMaxLen, int nTimeOut, int nEditMask);

int InputCNMode(const char *pszTitle, const char *pszContent,char *pszStr, int *pnInputLen, int nMinLen, int nMaxLen, int nImeMode);

int InputIp(const char * pszTitle, const char * pszContent, char * pszOut, int * pnOutLen);

int InputPin(const char *pszTitle,const char * pszAmount,const char * pszContent,int nMinLen, int nMaxLen,char *pszOut);

int ProInputDate(const char *pszTitle, const char *pszContent, const char * pStr, \
                int x, int y, char *pszDate, int nFormat, int nTimeOut);

int ProInputDlg(const char *pszTitle, const char *pszContent, const char* psInfo, \
										int Line, int Clum, char *pszOut, int *pnOutLen,int nMinLen, \
										int nMaxLen, int nTimeOut, int nEditMask);

#endif


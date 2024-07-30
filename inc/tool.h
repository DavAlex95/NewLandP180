/***************************************************************************
** Copyright (c) 2019 Newland Payment Technology Co., Ltd All right reserved
** File name:  tool.h
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
#ifndef _TOOL_H_
#define _TOOL_H_


#define KEY_FUNC_APP_SELECT KEY_MENU

#define DISP_OUT_ICC DispOutICC(NULL, "", "")

#define DISP_TRANS_SUCC \
{\
	PubDisplayGen(2,"TRANS SUCC");\
	PubUpdateWindow();\
}

#define DISP_PRINTING_NOW \
{\
	PubDisplayGen(3,"PRINTING...");\
	PubUpdateWindow();\
}

typedef enum
{
	LOGO_RF = 1,
	LOGO_MAIN_MENU,
	LOGO_STANDBY,

}EM_LOGO_TYPE;

#define DEFAULT_PIN_INPUT_TIME 60




typedef enum EM_STATUSBARTYPE{
	STATUSBAR_STANDBY_OPEN = 0,
	STATUSBAR_STANDBY_CLOSE,
	STATUSBAR_OTHER_OPEN,
	STATUSBAR_OTHER_CLOSE
}EMSTATUSBARTYPE;

typedef enum{
	EM_PLUS = 0,
	EM_SUB = 1,
}EM_MATH_TYPE;

#define TAGLEN 4

#define MAX_ITEM 100
#define MAX_PAGE 100
#define MAX_LINELETTER_MIDDLE 26

extern int ProAmtToDispOrPnt(const char *, char *);
extern int ProChkDigitStr(const char *, const int);
extern int ProConfirm(void);
extern unsigned long long Bcd2Unit64(uchar *,  int);
extern unsigned long long AtoLL(const char *);
extern const char * ImgManage(EM_LOGO_TYPE);
extern int AscBigCmpAsc(const char *, const char *);
extern void AmtSubAmt(uchar *, uchar *, uchar *);
extern void AmtAddAmt(uchar *, uchar *, uchar *);
extern void ShowLightIdle();
extern void ShowLightReady();
extern void ShowLightDeal();
extern void ShowLightFail();
extern void ShowLightSucc();
extern void ShowLightOnline();
extern void ShowLightOff();
extern void LeftTrimZero(char *);
extern int CurrencyAmtToDisp(const char *, const char *, char *);
extern int SetStatusBar(EMSTATUSBARTYPE);
extern int ChkDigitStr(const char *, const int);
extern void EdcProXor(char *psStr1, char *psStr2, int nLen);
extern int MathBcdAmtAndToDisp(const uchar *psBcdAmt1, const uchar *psBcdAmt2, EM_MATH_TYPE emType, char *pszDispAmt);
extern int BcdAmtToDisp(const uchar *psBcdAmt, char *pszDispAmt);
extern void ConvertHolderName(uchar *pszOrgName, uchar *pszNormalName);
extern int GetRandValue(int nRange);
extern int ProAddSymbolBtnStrs(char *pszStrA, char *pszStrB, char *pszOut, int nLen, char ch);
extern int CaculateWeekDay(char *pszData);
extern void ProTrimStr(char *pszString);
extern int CommSetSuspend(uint unFlag);
extern int CommHungUp(int nFlag);
extern YESORNO CheckIsNullOrEmpty(const char *pszStr, const int nLen);

#endif


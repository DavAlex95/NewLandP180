/***************************************************************************
** Copyright (c) 2019 Newland Payment Technology Co., Ltd All right reserved   
** File name:  print.h
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
#ifndef _PRINT_H_
#define _PRINT_H_

enum EPRINTFLAG
{
	FIRSTPRINT,
	REPRINT,
	REVERSAL_PRINT,
	OFFLINE_PRINT,
	TOTAL_PRINT
};

#define LASTSETTLEFILE "OLD_SETT"

extern int PrintRecord(const STTRANSRECORD *, int);
extern int PrintAllRecord(void);
extern int PrintSettle(int);
extern int PrintTotal(void);
extern int PrintParam(void);
extern int DealPrintLimit(void);
extern int DealPrinterPaper();
extern int PrintIsoData(const char *pszData);

extern int digitStrToArabic(char cDigitStr, char *pszArabic);
extern int TransferAmtToArabic(char *pszAmt, char *pszArabicAmt);
#endif


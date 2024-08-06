/***************************************************************************
** Copyright (c) 2019 Newland Payment Technology Co., Ltd All right reserved
** File name:  param.h
** File indentifier:
** Brief:
** Current Verion:  v1.0
** Auther: sunh
** Complete date: 2016-9-22
** Modify record:
** Modify date:
** Version:
** Modify content:
***************************************************************************/
#ifndef _PARAM_H_
#define _PARAM_H_

#include "settle.h"

#define TRANSSWITCHNUM		12
#define APPCHECKVALUE		APP_NAME"001"	/**<If check value changes, need to initialize*/
#define APPCHKLEN			7

#define PINPADMODE_CARD          0x01
#define PINPADMODE_PIN           0x02

#define L3INIT_NONE              0X00
#define L3INIT_INSIDE            0X01
#define L3INIT_PINPAD            0X02

#define DISPAMTLEN               32

/**
* @struct STREVERSALPARAM
*/
typedef struct
{
	char cIsReversal;		/**<Reversal flag, '0'-Has reversal; '1'-Not yet*/
	int nHaveReversalNum;	/**<The reversal time had sent*/
	STREVERSAL stReversal;
} STREVERSALPARAM;

typedef struct
{
	char cTransType;
	char *pszTransName;
} STTRANSACTION;

typedef struct
{
	char cType;
	char *pszName;
} STITEM;


typedef enum
{
	EM_NONE_PWD = 0,
	EM_SYSTEM_PWD = 1,
	EM_TRANS_PWD,
	EM_FUNC_PWD,
}EM_PWD_TYPE;

typedef enum {
	LANG_EN_US 		= 0,
	LANG_ES_MX 		= 1,
	LANG_CUSTOMIZED	= 2
}EM_LANG;

enum CardAttribute
{
	DEBITCARD = 10,
	CREDITCARD,
	QUACREDEITCARD
};

typedef enum {
	PINPAD_ME51,
	PINPAD_SP100,
}EM_PINPADTYPE;

/**
* @struct pos
*/
typedef struct
{
	char szTerminalId[8+1];			/**<TID*/
	char szMerchantId[15+1];			/**<MID*/
	char szMerchantNameEn[40+1];
	char szMerchantAddr[3][80+1];
	char szAdminPwd[16+1];
	char szUsrPwd[16+1];
	char szFuncPwd[16+1];
	char sTransSwitch[4];
	char szAcqName[20+1];
	char szIssuerName[20+1];
	char szCurrencyName[3+1];
	char cPrintPageCount;				/**<'0'-'9'*/
	char cIsPntDetail;
	char cIsTipFlag;
	char szTipRate[2+1];
	char cIsCardInput;
	char cIsVoidStrip;
	char cIsVoidPin;
	char cIsSupportRF;					/**<'1' support*/
	char cIsExRF;						/**<'1' support*/
	char cIsPinPad;						/**<'1'-use pinpad*/
	char cPinPadAuxNo;
	char szPinPadTimeOut[3+1];
	char szMainKeyNo[3+1];				/**<INDEX   '00'-'99'*/
	char cEncyptMode;					/**<'0'-de   '1'-3des*/
	char cPinEncyptMode;				/**<'0'-With pan '1'-without pan*/
	char szMaxTransCount[5+1];
	char cIsPrintErrReport;
	char cDefaultTransType;				/**<'1'-sale,'0'-preauth*/
	char cIsDispEMV_TVRTSI;				/**<'1'show,'0'Not show*/
	char cIsShieldPan;
	char cPntTitleMode;
	char szPntTitleEn[40+1];
	char szAppDispname[20+1];
	char cIsAdminPwd;
	char cIsPrintMinus;					/*1 print*/
	char cIsNeedCVV2;
	char cIsPreauthShieldPan;
	char cIsReprintSettle;
	char cIsSupportSwipe;					//support swipe 0: not 1:yes
	char cIsSupportContact;
	char cIsPrintIso;
	char cKeySystemType;				//'0'-MK/SK '1'--DUKPT
	char cLanguage;
	char cTomsObtainCmd;
	char cFontSize;
	char szVerChkValue[8+1];
	char szVersion[8+1];
	char cPinPadUsage; // read card/Enter pin on Pinpad
	char cPinPadCallbackFlag; // pinpad information is synchronized with upper computer.
	char cPinpadType;
	char cL3initStatus;
    char cLockTerminal;
    char szLockPromptInfo[32+1];
	char cElecSignFlag;
}STAPPPOSPARAM;

/**
* @struct STSETTLEPARAM
*/
typedef struct
{
	int nOffLineUnsendNum;
	int	nRecNum;						/**<Record num*/
	int nHaveReSendNum;					/**<Count has sent*/
	char cPrintSettleHalt;				/**<Print settle flag*/
	char cPrintDetialHalt;				/**<Print detail flag*/
	char cPrintRecordHaltFlag;			/**<Print record flag*/
	char cIsTotalMatch;					/**<Is total summary */
	char cBatchHaltFlag;				/**<Send batch flag*/
	char cClrSettleDataFlag;			/**<Clear settle flag*/
	int	nBatchMagOfflinHaltFlag;		/**<Send magnetic offline flag*/
	int	nBatchSum;						/**<Batch num*/
	char sSettleDateTime[5];			/**<Settle time*/
	STSETTLE stSettle;
} STSETTLEPARAM;


/**
* @struct STTRANSPARAM
*/
typedef struct
{
	char cIsLogin;						/**<Is login*/
	char szTraceNo[6+1];		        /**<Stan*/
	char szBatchNo[6+1];		        /**<Batch no*/
	char szInvoiceNo[6+1];	            /**<Invoice No*/
	char sLastLoginDateTime[4+1];		/**<Last login time*/
	uint nOnlineUpNum;					/**<Count of sended EMV online transactions*/
	char szQPSLimit[6+1];				/**<Quics Limit*/
} STTRANSPARAM;


extern int IsFirstRun(void);
extern int InitPosDefaultParam(void);
extern int ExportPosParam(void);
extern int SetParamFromIni(void);
extern YESORNO GetVarIsLogin(void);
extern int SetVarIsLogin(const YESORNO);
extern YESORNO GetVarIsLock(void);
extern int GetVarTraceNo(char *);
extern int SetVarTraceNo(const char *);
extern int IncVarTraceNo(void);
extern int GetVarBatchNo(char *);
extern int SetVarBatchNo(const char *);
extern int IncVarBatchNo(void);
extern int IncVarDialSum(void);
extern int GetVarTimeCounter(long *);
extern int GetVarLastLoginDateTime(char *);
extern int SetVarLastLoginDateTime(const char *);
extern YESORNO GetVarIsReversal(void);
extern int SetVarIsReversal(const YESORNO);
extern int GetVarHaveReversalNum(int *);
extern int SetVarHaveReversalNum(const int);
extern int IncVarHaveReversalNum(void);
extern int GetReversalData(STREVERSAL *);
extern int SetReversalData(STREVERSAL *);
extern void GetVarTerminalId(char *);
extern void GetVarMerchantId(char *);
extern void GetVarMerchantNameEn(char *);
extern void GetVarAppType(char *);
extern int SetVarAppType(const char *);
extern void GetVarSoftVer(char *);
extern void GetVarSoftVerAlt(char *);
extern void GetVarAppVer(char *);
extern YESORNO GetVarIsPinpad(void);
extern int GetVarPinpadAuxNo(int *);
extern int GetVarPinpadTimeOut(int *);
extern void GetVarPwd(char *, uchar);
extern void GetVarMainKeyNo(int *);
extern  int SetVarMainKeyNo(const int);
extern int  GetVarEncyptMode(void);
extern int  GetVarPinEncyptMode(void);
extern int SetVarIsAutoLogOut(const YESORNO);
extern YESORNO GetVarIsPntDetail(void);
extern YESORNO GetVarIsNeedCVV2(void);
extern YESORNO GetVarIsPrintErrReport(void);
extern void GetVarPrintPageCount(int *);
extern void GetVarMaxTransCount(int *);
extern int SetVarTipRate(const char *);
extern void GetVarTipRate(char *);
extern YESORNO GetVarIsTipFlag(void);
extern int SetVarIsTipFlag(const YESORNO);
extern YESORNO GetVarIsSupportContact(void);
extern YESORNO GetVarIsCardInput(void);
extern int SetVarIsCardInput(const YESORNO);
extern YESORNO GetVarIsVoidStrip(void);
extern YESORNO GetVarIsAuthSaleVoidStrip(void);
extern YESORNO GetVarIsVoidPin(void);
extern YESORNO GetVarIsPreauthVoidPin(void);
extern YESORNO GetVarIsAuthSalePin(void);
extern void GetVarDefaultTransType(char *);
extern void GetVarAuthSaleMode(char *);
extern YESORNO GetVarIsDispEMV_TVRTSI(void);
extern int GetVarEmvOnlineUpNum(int *);
extern int SetVarEmvOnlineUpNum(const int);
extern int IncVarEmvOnlineUpNum(void);
extern int GetVarEMVOnlineARPCErrHaltFlag(int *);
extern YESORNO GetVarEmvIsDownCAPKFlag(void);
extern int SetVarEmvIsDownCAPKFlag(const YESORNO);
extern YESORNO GetVarEmvIsDownAIDFlag(void);
extern int SetVarEmvIsDownAIDFlag(const YESORNO);
extern int SetFunctionMainKeyNo(void);
extern int SetFunctionMaxRefundAmount(void);
extern int SetFunctionPrintPageCount(void);
extern int SetFunctionMaxTransCount(void);
extern int SetFunctionMerchantId(void);
extern int SetFunctionTerminalId(void);
extern int SetFunctionMerchantName(void);
extern int SetFunctionTraceNo(void);
extern int SetFunctionBatchNo(void);
extern int SetFunctionIsPinPad(void);
extern int SetFunctionIsContact(void);
extern int SetFunctionIsManul(void);
extern int SetFunctionIsPrintErrReport(void);
extern int SetFunctionPosState(void);
extern int SetFunctionDefaultTransType(void);
extern int SetFunctionIsVoidPin(void);
extern int SetFunctionIsVoidStrip(void);
extern int SetFunctionDesMode(void);
extern int SetFunctionPinEncyptMode(void);
extern int SetFunctionIsPntDetail(void);
extern int SetFunctionIsNeedCVV2(void);
extern int SetFunctionIsTipFlag(void);
extern int ChkSecurityPwd(char);
extern int SetVarPwd(const char *, uchar);
extern int ChangePwd(uchar);
extern int SetFunctionIsIcConfirmFlag(void);
extern int SetFunctionIsDispEMV_TVRTSI(void);
extern int SetVarRecordSum(const int);
extern int GetVarRecordSum(int *);
extern int SetVarHaveReSendNum(const int);
extern int GetVarHaveReSendNum(int *);
extern int SetVarPrintSettleHalt(const YESORNO);
extern YESORNO GetVarPrintSettleHalt(void);
extern int SetVarIsTotalMatch(const char);
extern int SetVarBatchHaltFlag(const YESORNO);
extern YESORNO GetVarBatchHaltFlag(void);
extern int GetVarFinanceHaltFlag(int *);
extern int SetVarBatchMagOfflineHaltFlag(const int);
extern int GetVarBatchMagOfflineHaltFlag(int *);
extern int SetVarBatchSum(const int);
extern int GetVarBatchSum(int *);
extern int GetVarSettleDateTime(char *);
extern int SetVarSettleDateTime(const char *);
extern int SetVarSettleData(const STSETTLE);
extern int GetVarSettleData(STSETTLE *, STSETTLE *);
extern int SaveSettleDataItem(char , SETTLE_NUM *, SETTLE_AMT *);
extern int SetVarPrintDetialHalt(const YESORNO);
extern YESORNO GetVarPrintDetialHalt(void);
extern int SetVarPrintRecordHaltFlag(const YESORNO);
extern YESORNO GetVarPrintRecordHaltFlag(void);
extern int SetVarClrSettleDataFlag(const YESORNO);
extern YESORNO GetVarClrSettleDataFlag(void);
extern void GetAppPosParam(STAPPPOSPARAM *);
extern void SetAppPosParam(STAPPPOSPARAM);
extern int SetFunctionTipRate(void);
extern void SetControlChkPinpad(const YESORNO);
extern YESORNO GetControlChkPinpad(void);
extern void SetControlChkPinpad(const YESORNO);
extern void SetControlChkInside(const YESORNO);
extern void SetControlChkRF(const YESORNO);
extern YESORNO GetControlChkPinpad(void);
extern YESORNO GetControlChkInside(void);
extern YESORNO GetControlChkRF(void);
extern YESORNO GetVarIsIcConfirmFlag(void);
extern int SetVarMCEnterTimeAndRandom(int, int);
extern int GetVarUID(char *);
extern YESORNO GetVarIsExRF(void);
extern int SetFunctionIsExRF(void);
extern YESORNO GetVarIsSupportRF(void);
extern int SetFunctionIsSupportRF(void);
extern YESORNO GetVarIsUsePP60RF(void);
extern int SetFunctionIsUsePP60RF(void);
extern int SetVarIsPhoneSale(const YESORNO);
extern int SetVarIsEncryptTrack(const YESORNO);
extern YESORNO GetVarIsSmallGeneAuth(void);
extern int SetVarIsSmallGeneAuth(const YESORNO);
extern YESORNO GetVarIsShieldPan(void);
extern int SetFunctionIsShieldPan(void);
extern void GetVarSoftVer(char *);
extern int IncVarOfflineUnSendNum(void);
extern int DelVarOfflineUnSendNum(void);
extern int GetVarOfflineUnSendNum(void);
extern int SetVarOfflineUnSendNum(int);
extern YESORNO GetVarIsPrintPrintMinus(void);
extern int SetFunctionIsPrintPrintMinus(void);
extern int SetFunctionAppName(void);
extern int SetFunctionMerchantName(void);
extern int SetFunctionIsTransPinInput(void);
extern int SetFunctionIsOfflineSendNow(void);
extern int SetFunctionOffResendNum(void);
extern int SetFunctionIsPrintFailRecordRec(void);
extern int SetFunctionPrintTitleMode(void);
extern int SetFunctionPntTitle(void);
extern int SetFunctionHotLineNo(void);
extern int SetVarPinKey(char *, int);
extern int SetVarTrKey(char *, int);
extern int SetFunctionMaxOffSendNum(void);
extern void GetVarMaxOffSendNum(int *);
extern int SetFunctionPinpadTimeOut(void);
extern int SetFunctionIsSwipe(void);
extern YESORNO GetVarIsAdminPin();
extern int GetVarAdInfo(char *);
extern YESORNO GetIsPntChAquirer(void);
extern YESORNO GetIsPntChCardScheme(void);
extern YESORNO GetIsPntTitleMode(void);
extern int GetFunctionPntTitle(char *);
extern int GetFunctionHotLineNo(char *);
extern int SetFunctionIsPreauthShieldPan(void);
extern YESORNO GetVarIsPreauthShieldPan();
extern int SetFunctionReprintSettle();
extern YESORNO GetVarReprintSettle(void);
extern void GetAcqName(char *);
extern int  GetShowName(char *);
extern YESORNO GetVarShowRFLogo();
extern int SaveEmvTransSerial(void);
extern int Version(void);
extern int GetEmvErrorCodeIni(int, char *);
extern int GetPrnLogo(char *, uint *);
extern int SetFunctionPosDateTime(void);
extern int SetVarEncyptMode(const char );
extern int SetVarEmvIsTransResults(const YESORNO );
extern int SetTransSwitchOnoff(int , char );
extern int SetFunction2DSwitch(void);
extern int SetVarIsDownParmFlag(char );
extern YESORNO GetVarIsDownParmFlag(void);
extern YESORNO GetVarIsPrintSettle();
extern void GetVarAdjustRate(char *psValue);

extern void GetVarTmkRefNum(char *psValue);
extern int SetFunctionTransSwitch(void);
extern YESORNO GetTransSwitchOnoff(char );
extern int ChangePassword();
extern int ViewTranList();
extern int ViewSpecList();
extern int ViewTotal();
extern int ViewTVR_TSI();
extern int RePrnSettle();
extern int PrnLastTrans();
extern int RePrnSpecTrans();
extern int PrnTotal();
extern int SetSystemParam(void);
extern int ProCheckPwd(char *pszTitle, uchar cType);
extern void GetVarIssueBank(char *pszValue);
extern int GetVarInvoiceNo(char *pszInvoiceNo);
extern char GetVarIsTotalMatch(void);
extern int SetScan();
extern int IncVarInvoiceNo();
extern int SetFunctionMerchantAddr(void);
extern void GetVarMerchantAddr(char *pszValue, int nIndex);
extern void GetVarCurrencyName(char *pszValue);
extern int SetFunctionIssuerName(void);
extern int SetFunctionAcqName(void);
extern int SendOffSale();
extern int SetVarTmkRefNum(char *psAscRefNum);
extern YESORNO GetVarIsSendAutoDown();
extern int SetVatIsSendAutoDown(char);
extern int SetFunctionIsSendAutoDown(void);
extern int SetFunctionDownMode(void);
extern int SetVarInvoiceNo(const char *pszInvoiceNo);
extern int SetFunctionDlWeek(void);
extern int SetFunctionDlHour(void);
extern char GetVarDlHour();
extern char GetVarDlMinute();
extern char GetVarDlMode();
extern int SetVarDlDate(const char *pszValue);
extern char GetVarDlWeek();
extern void GetVarDlDate(char *pszValue);
extern YESORNO GetVarIsSwipe();
extern int SetFunctionInvoiceNo(void);
extern int SetFunctionKeySystemType(void);
extern int GetVarKeySystemType(void);
extern int SetVarSystemLanguage(const EM_LANG cLanguage);
extern EM_LANG GetVarSystemLanguage(void);

extern int GetVarSystemFontSize(void);

extern int GetVarIsPrintIso(void);
extern int SetVarSystemFontSize(const char cFontSize);
extern int SetFunctionIsPrintIso(void);
extern int SetCustomizedFont(EM_LANG emLang);

extern int GenerateAppChkValue(char *pszChkValue);
extern void InitSettleParam();
extern void GetVarQPSLimit(char *pszValue);
extern int SetFunctionQPSLimit(void);

extern void GetTransName(char , char *);
extern void GetTransSwitchValue(char *);


extern int SetFunctionPinpadUsage(void);
extern int GetVarPinPadUsage(void);
extern int SetFunctionPinpadPort(void);
extern YESORNO GetFuncPinpadCallBackFlag();
extern int GetVarPinPadType(void);
extern int SetFunctionPinpadType(void);
extern YESORNO GetVarIsPinpadReadCard();
extern void SetL3initStatus(char cStatus);
extern char GetL3initStatus();
extern int GetIsLoadXMLConfig();

extern void SetLockTerminal(char cStatus);
extern char GetLockTerminal();

extern int SetLockPromptInfo(char *pszPromptInfo);
extern char *GetLockPromptInfo();

extern char GetElecSignFlag();
extern void SetElecSignFlag(char cFlag);
extern int SetFunctionIsELecSign(void);
extern char GetPinpadCallBackFlag();
extern int SetFuncPinpadCallBack(void);
#endif


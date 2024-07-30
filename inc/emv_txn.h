#ifndef _EMVTXN_H_
#define _EMVTXN_H_

#define APP_FALLBACK (APP_TIMEOUT -1)

#define EMVFILE_TERMLOG		"EMVLOG"APP_NAME
#define FILE_EMV_RECORD_FAIL	APP_NAME"WEFA"	
#define FILE_EMV_RECORD_SUCC	APP_NAME"WESU"	
#define MAX_TAG_LIST_NUM		32

/*          Transaction type                    */
#define 		EMV_IC_CAPK 					40
#define 		EMV_IC_AIDPARAM				41
#define 		EMV_OFFLINEBATCHUP			43
#define 		EMV_ONLINEBATCHUP			44
#define 		EMV_STATUSSEND_IC_CAPK 					45
#define 		EMV_STATUSSEND_IC_AIDPARAM				46

/**EMV_InitApp return value definition*/
#define          	EMV_INITAPP_FAIL                            -100                           				                               
#define           EMV_INITAPP_QUIT				  -101
#define           EMV_INITAPP_UPCARD			  -102

//Emv transaction statuc
enum
{
	EMV_STATUS_OFFLINE_FAIL = 1,	
	EMV_STATUS_OFFLINE_SUCC,		
	EMV_STATUS_ONLINE_FAIL,		
	EMV_STATUS_ONLINE_SUCC		
};

enum EM_CVM {
	CVM_NO_PIN_SIG = 0,
	CVM_PIN = (1 << 0),
	CVM_SIG = (1 << 1),
};

#define		FIELD55_MAXLEN	(384)		//Maximum length of field 55

//EMV record
typedef struct
{
	char szPan[20];						//PAN
	int nPanLen;					//Pan length
	int nPanSeq;					//Serial No
	char sDate[4];						//Date(BCD)
	char sTime[3];						//TIME(BCD, HHMMSS)
	char sAmount[6];					//AmountBCD)
	char sOtherAmount[6];				//Other amount(BCD)
	char sAuthResp[2];					//Respond code
	char cTransType;					//Transaction type
	char sRecordNo[3];					//Trace no
	char sBatchNo[3];					//batch no
	char sTVR[5];
	char sTSI[2];
	char szField55[FIELD55_MAXLEN];		///55 Field (Len[2]+Data)(Len format is HEX)
	char cBatchUpFlag;
	char cInputMode;					///'5' is contact,'7' is contactless
	char szIssuerScriptResult[20];		//Issuer Script Result
	int nIssuerScriptResultLen;		//Issuer Script Result length
	char szInterOrg[3+1];
	char szAdd[512];
}STEMVRECORD;


/**
* @struct STSCRIPT structure
*/
typedef struct
{
	char cTransType;				/**<Trans Type*/
	char cTransAttr;				/**<Trans Attr*/
	char cEMV_Status;			/**<EMV Trans status*/
	char szPan[10];			/**<2 PAN*/
	char cPanLen;
	char szProcCode[3];			/**<3 Process code*/
	char szAmount[6];			/**<4 Amount*/
	char szInputMode[3];		/**<22 Input mode*/
	char szCardSerialNo[3];		/**<23 Serial number*/
	char szServerCode[2];		/**<25 Server code*/
	char szRefnum[12];			/**<37 Reference number*/
	char szAuthCode[6];		/**<38 Auth code*/
	char sFuncCode[2];			/**<60.1 */
	char cTermCapacity;			/**<60.4 */
	char cIcConditionCode;		/**<60.5 */

	char sOldBatch[6];			/**<61.1 */
	char sOldTrace[6];			/**<61.2 N6*/
	char sOldDate[4];				/**<61.3 N4*/
	
}STSCRIPT;

typedef struct
{
	char cSignatureFlag;					/**<IS need sign (1-YES, 0£­NO)*/
	char sTC[8];							/**<IC TC*/
	char sTVR[5];							/**<TVR*/
	char sTSI[2];							/**<TSI*/
	char sAID[16];						/**<AID*/
	char cAIDLen;						/**<AID length*/
	char sATC[2];							/**<ATC*/
	char szAIDLable[16+1];				/**<Aid lable*/
	char szAppPreferedName[16+1];		/**<App Prefered Name*/
	char cForceAcceptFlag;				/**<Force Accept Flag*/
	char sUnpNum[4];					/**<Unknow number*/
	char sTermCap[3];					/**<Terminal property*/
	char sAIP[2];							/**<AIP*/
	char sCVM[3];
	char sIAD[32];
	char cIADlen;
	char sARQC[8];						/**<ARQC*/
	
	char cCrmInfoData;					/**<Cryptogram information data (9F27)*/
	char szIssuerScriptResult[32+1];		//Issuer Script Result
	int nIssuerScriptResultLen;		//Issuer Script Result length
}STEMVADDTIONRECORDINFO;

extern int DispOutICC(char *, char *, char *);
extern int ProGetCardStatus();
extern int EmvOrgSheet(const char *, char *);
extern char *EmvErrMsg(int);
extern int SaveData9F26RQ();
extern int GetData9F26RQ(char *);
extern int EmvLoadRecordByTrace(const char *, const char *, STEMVRECORD *, int);
extern int EmvLoadRecordByRecNo(int, STEMVRECORD *, int);
extern int EmvGetRecordRecNum(int);
extern int EmvSaveRecord(int , const STSYSTEM *);
extern int EmvClearRecord(void);
extern int EmvPackField55(const char, char *, int *);
extern int EmvAddtionRecord(STTRANSRECORD *);
extern void EmvOfflineDenial(char *pszTitle, STSYSTEM* pstSystem);
extern void EmvOfflineAccept(char *pszTitle, STSYSTEM *pstSystem);
extern int EmvBatchUpOnlineSucc(const int, const int, int *);
extern int EmvSendTC(void);
#endif


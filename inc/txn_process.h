/***************************************************************************
** Copyright (c) 2019 Newland Payment Technology Co., Ltd All right reserved   
** File name:  packet.h
** File indentifier: 
** Brief:  
** Current Verion:  v1.0
** Auther: sunh
** Complete date: 2016-9-21
** Modify record: 
** Modify date: 
** Version: 
** Modify content: 
***************************************************************************/

#ifndef _TRANS_PROCESS_H_
#define _TRANS_PROCESS_H_

#define INPUT_NO 0

#define MAX_PAN 19
#define MAX_TK1 100
#define MAX_TK2 37
#define MAX_TK3 104

#define MIN_PAN 13
#define MIN_TK1 MIN_PAN
#define MIN_TK2 MIN_PAN
#define MIN_TK3 MIN_PAN

#define STRIPE_TIMEOUT 60		
#define DISPPAN_TIMEOUT 30		
#define INPUT_AMOUNT_TIMEOUT 60	
#define INPUT_DATE_TIMEOUT 60	
#define INPUT_STRING_TIMEOUT 60	

#define MAX_PACK_SIZE 1000		

#define CURRENCY_CODE "840"	

#define INPUTMODE_KEYIN 				"011"
#define INPUTMODE_IC_INSERT			"051"
#define INPUTMODE_STRIPE 		"021"
#define INPUTMODE_IC_RFID   			"071"
#define INPUTMODE_FALLBACK   			"801"
 
enum TRANSTYPE
{
	TRANS_SALE,				
	TRANS_VOID,		
	TRANS_REFUND,				
	TRANS_PREAUTH,			
	TRANS_AUTHCOMP,			
	TRANS_ADJUST,			
	TRANS_BALANCE,			
	TRANS_AUTHSALEOFF,		
	TRANS_VOID_AUTHSALE,		
	TRANS_VOID_PREAUTH,		
	TRANS_CASH,
	TRANS_CASHBACK,
	TRANS_OFFLINE,
	TRANS_MAX,
};

enum TRANSTYPE_MANAGE
{
	TRANS_LOGIN = TRANS_MAX + 1,
	TRANS_SENDTC,
	TRANS_REVERSAL,
	TRANS_UPLOAD,
};

enum TXNATTR
{
	ATTR_MANUAL = 0x01,
	ATTR_MAGSTRIPE, 	
	ATTR_CONTACT = 0x05,
	ATTR_CONTACTLESS = 0x07,			
	ATTR_FALLBACK = 0x80,	
};

enum KEYSYSTEM
{
	KS_MSK = '0',  /* MK/SK */
	KS_DUKPT = '1',   /* DUKPT */
};

enum TRANS_STATUS
{
	CANCELED = 0x01,       /* transaction has been canceled */
	ADJUSTED = 0x02,       /* transaction has been adjusted */
	SALECOMPLETED = 0x03,  /* Pre-authorization completed */
	SENT_ADJUST = 0x04,  /* It has been sent before adjustment */
};

typedef enum
{
	CFG_NULL 		= (0x00),
	CFG_PASSWORD 	= (0x01 << 0),	//verify pwd
	CFG_OLDINFO 	= (0x01 << 1),	//input old infomation
	CFG_AMOUNT 		= (0x01 << 2),	//input amount
	CFG_CARD 		= (0x01 << 3),	//input card
	CFG_SEARCH 		= (0x01 << 4),	//find transaction log by trace NO
	CFG_REVERSAL 	= (0x01 << 5),	//need reversal
	CFG_PRINT	 	= (0x01 << 6),	//need print
	CFG_ADVICE		= (0x01 << 7),  //need to send TC
	CFG_TIPS		= (0x01 << 8),  //support tip
	CFG_ONLINE		= (0x01 << 9),  //online transaction
	CFG_NOCHECKRESP	= (0x01 << 10),  //response code check 
	CFG_MAC 		= (0x01 << 11), //mac 
} EM_OPERATEFLAG;

/**
* @struct STTRANS transaction Configuration  structure
*/
typedef struct
{
	char cTransType;
	char szTitle[32];
	char szMsgID[4+1];                /**< Field 0 message ID */
	char szProcessCode[6+1];
	char szServiceCode[2+1];
	EM_OPERATEFLAG cOperFlag;
	char szField[128+1]; 
} STTRANSCFG;

/**
* @struct STSYSTEM structure
*/
typedef struct
{
	char cTransType;			
	char cTransAttr;
	char cEMV_Status;		
	char szMsgID[4+1];		
	char szPan[19+1];			/**<Field 2 */
	char szProcCode[6+1];			/**<Field 3*/ 
	char szAmount[12+1];			/**<Field 4 Amt(BCD)*/
	char szCashbackAmount[12+1];	/**<cash back amt		*/
	char szTrace[6+1];			/**<Field 11 STAN*/
	char szTime[6+1];			/**<Field 12 TIME(hhmmss) */
	char szDate[8+1];				/**<Field 13 DATE(yyyymmdd/yymmdd/mmdd ) */
	char szExpDate[4+1];			/**<Field 14 Valid Date*/
	char szSettleDate[4+1];		/**<Field 15 SETTLE DATE(Retrun used)  */
	char szInputMode[3+1];		/**<Field 22 */
	char szCardSerialNo[3+1];		/**<Field 23 */ 
	char szNii[3+1];				/**<Field 24 */
	char szServerCode[2+1];		/**<Field 25 */
	char szTrack2[37+1];			/**<Field 35 */
	char szTrack3[104+1];		/**<F36 */

	char szRefnum[12+1];			/**<F37  */
	char szAuthCode[6+1];		/**<F38 */
	char szResponse[2+1];		/**<F39 */
	char szPosID[8+1];			/**<F41 TID */
	char szShopID[15+1];			/**<F42 MID */
	char szTrack1[75+1];			/**<F45 */
	char szCVV2[4+1];			/**<F48 CVV2*/
	char szField57[999+1];			/**<F57 */
	char szPin[8+1];				/**<F52 */
	char szInvoice[6+1];		/**<F62 */
	char cMustChkMAC;			/**<F64 */
	char szOldAuthCode[6+1];	
	char szOldRefnum[12+1];  
	char szOldAmount[12+1];		
	char szTipAmount[12+1];
	char szBaseAmount[12+1];
	char szOldTipAmount[12+1];
	char szOldDate[4+1];
	char szOldTrace[6+1];
	char szInterOrg[3+1];
	char szBatchNum[6+1];
	char *psAddField;			/**<Addtional Field pointed to F55*/
	int nAddFieldLen;				/**<Addtional Field len*/
	
	char cPrintFlag;
	char szHolderName[26+1];
	char cPinAndSigFlag;		/**<PIN and SIGN flag */
	char cCvmStatus;
	char cGetPerformDataFlag;
}STSYSTEM;

/**
* @struct STREVERSAL reversal structure
*/
typedef struct
{
	char cTransType;			
	char cTransAttr;
	char szPan[19+1];				/**<F2 */
	char szProcCode[6+1];			/**<3 Process(Same to original)*/
	char szAmount[12+1];			/**<F4 Amount(Same to original)*/
	char szCashbackAmount[12+1];    /**<cash back amt		*/
	char szTrace[6+1];				/**<F11 STAN (Same to original)*/
	char szExpDate[4+1];			/**<F14 Valid DATE(Same to original)*/
	char szInputMode[3+1];			/**<F22 */
	char szCardSerialNo[3+1];		/**<F23 */
	char szNii[3+1];				/**<F24 */	
	char szServerCode[2+1];			/**<F25 */
	char szOldAuthCode[6+1];
	char szTrack1[75+1];
	char szTrack2[37+1];			/**<F35 */
	char szResponse[2+1];			/**<F39 */
	char szInvoice[6+1];
	char szCVV2[4+1];				/**<F48 CVV2*/
	char szFieldAdd1[260+1];			
	int nFieldAdd1Len;
}STREVERSAL;

/**
* Interface functions
*/
extern int TxnAdjust(void);
extern int TxnSendOffline(const char);
extern void TxnSystemToReveral(const STSYSTEM *, STREVERSAL *);
extern int TxnGetAmout(char *, STSYSTEM *, STTRANSCFG *);
extern void TxnReveralToSystem(const STREVERSAL *, STSYSTEM *);
extern int TxnReversal(void);
extern int TxnCommonEntry(char cTransType, int *pnInputMode);
extern int TxnLoadConfig(char cTransType, STTRANSCFG *pstTransConfig);
extern int CheckTip(STSYSTEM *pstSystem);
extern int TxnL3PerformTransaction(char *pszTlvLise, int nTlvLen, L3_TXN_RES *res, STSYSTEM *pstSystem);
extern int TxnL3CompleteTransaction(char *pszTlvList, int nTlvLen, L3_TXN_RES *res);
extern void TxnL3TerminateTransaction();
extern int TxnL3GetData(unsigned int type, void *data, int maxLen);
extern int TxnL3GetTlvData(unsigned int *tagList, unsigned int tagNum, unsigned char *tlvData, unsigned int maxLen,int ctl);
extern int TxnL3SetData(unsigned int tag, void *data, unsigned int len);
extern int TxnL3SetDebugMode(int debugLV);
extern int TxnL3ModuleInit(char *filePath, char *config);
extern int TxnL3LoadAIDConfig(L3_CARD_INTERFACE interface, L3_AID_ENTRY *aidEntry, unsigned char tlv_list[], int *tlv_len, L3_CONFIG_OP mode);
extern int TxnL3LoadCAPK(L3_CAPK_ENTRY *capk, L3_CONFIG_OP mode);
extern int TxnL3LoadTerminalConfig(L3_CARD_INTERFACE cardinterface, unsigned char tlv_list[], int *tlv_len, L3_CONFIG_OP mode);
extern int TxnL3EnumEmvConfig(L3_CARD_INTERFACE cardinterface, L3_AID_ENTRY * aidEntry, int maxCount);
extern int TxnL3EnumCapk(int start, int end, char capk[][6]);
extern int TxnL3Init();

extern int TxnWaitAnyKey(int nTimeout);
#endif


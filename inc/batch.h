/***************************************************************************
** Copyright (c) 2019 Newland Payment Technology Co., Ltd All right reserved   
** File name:  batch.h
** File indentifier: 
** Brief:  
** Current Verion:  v1.0
** Auther: sunh
** Complete date: 2016-9-25
** Modify record: 
** Modify date: 
** Version: 
** Modify content: 
***************************************************************************/
#ifndef _BATCH_H_
#define _BATCH_H_

#define FILE_RECORD APP_NAME"RECORD"

typedef enum {
	VIEWRECORD_ATTR_DETAIL,
	VIEWRECORD_ATTR_ONEBYONE,
} EM_VIEWRECORD_ATTR;

/**
* @struct STTRANSRECORD flow structure
*/
typedef struct
{
	char cTransType;		
	char cTransAttr;			
	char cEMV_Status;		
	char sPan[10];				/**<F2 Card no(BCD)*/
	int nPanLen;				/**<2 Pan length*/
	char sAmount[6];			/**<4 Amt(BCD)*/
	char sTrace[3];				/**<11 Trace no or stan(BCD)   */
	char sTime[3];				/**<12 hhmmss(BCD)  */
	char sDate[4];				/**<13 mmdd  (BCD)  */
	char sExpDate[2];			/**<14 valid dat (BCD)e*/
	char szInputMode[3+1];		/**<22 */
	char szCardSerialNo[3+1];	/**<23 */
	char sTrack2[19];			/**<35 (BCD)*/
	int nTrack2Len;				/**<35 */
	char sTrack3[53];			/**<36 (BCD)*/
	int nTrack3Len;				/**<36 */
	char szRefnum[12+1];		/**<37 */
	char szAuthCode[6+1];		/**<38 */
	char szResponse[2+1];		/**<39 */
	char sBatchNum[3];			/**<batch no(BCD)*/
	char sOldTrace[3];			/**<(BCD)    */
	char szOldDate[4+1];
	char szOldAuthCode[6+1];	/**<*/
	char szOldRefnum[12+1];   	/**<*/
	char cStatus;				/**<Cancel1*/
	char sTipAmount[6];			/**<48 */
	char sBaseAmount[6];
	char cSendFlag;				/**<0 unsend 0xFD sent, 0xFE-Refuse, 0xFF-Send Fail, ohter Send times*/
	char cBatchUpFlag;			/**<batch up send flag; 0 unsend 0xFD sent, 0xFE-Refuse, 0xFF-Send Fail, ohter Send times*/
	char cTcFlag;				/**<Tc send flag; 0 unsend 0xFD sent, 0xFE-Refuse, 0xFF-Send Fail, ohter Send times*/
	char szHolderName[26+1]; 	
	char szServerCode[2+1];		/**<25 */
	char sField55[354];			
	int nField55Len;
	char cPrintFlag;
	char szCVV2[4+1];
	char cPinAndSigFlag;
	char sCashbackAmount[6];	/**<cashback amt*/
	char sAddition[256];
	int nAdditionLen;
}STTRANSRECORD;


extern int InitBatchFile(void);
extern int AppendTransRecord(const STTRANSRECORD *);
extern int UpdateTransRecord(const STTRANSRECORD *);
extern int UpdateRecByHandle (int , const int , STTRANSRECORD *);
extern int FindRecordWithInvno( const char *, STTRANSRECORD *);
extern int FetchFirstRecord(STTRANSRECORD *);
extern int FetchNextRecord(STTRANSRECORD *);
extern int FetchPreviousRecord(STTRANSRECORD *);
extern int FetchLastRecord(STTRANSRECORD *);
extern int DispTransRecord(const STTRANSRECORD *, EM_VIEWRECORD_ATTR);
extern int RecordToSys(const STTRANSRECORD *, STSYSTEM *);
extern int SysToRecord(const STSYSTEM *, STTRANSRECORD *);
extern void GetRecordNum(int *);
extern int FetchCurrentRecord(STTRANSRECORD *);
extern int RecordOneByOne(void);
extern int FindByInvoice(int);
extern int FindSignpicByTrace();
extern int GetFailSendNum(char);
extern int DispRecordInfo(const char * , const STTRANSRECORD *);
extern int GetTcNum(int *pnTcNum);
extern int ReadTransRecord(const int , const int , STTRANSRECORD *);
extern int FindRecordWithTagid(int nTagid, char *pszValue, STTRANSRECORD *pstTransRecord);

#endif


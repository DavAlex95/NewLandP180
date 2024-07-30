/***************************************************************************
** Copyright (c) 2019 Newland Payment Technology Co., Ltd All right reserved   
** File name:  settle.h
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

#ifndef _SETTLE_H_
#define _SETTLE_H_

#include "batch.h"

typedef uint SETTLE_NUM;
typedef uchar SETTLE_AMT[8];

/**
* @struct STSETTLE 
*/
typedef struct tagSettle
{
	SETTLE_NUM _SaleNum;				
	SETTLE_AMT _SaleAmount ;		

	SETTLE_NUM _VoidSaleNum;		
	SETTLE_AMT _VoidSaleAmount;	

	SETTLE_NUM _AuthSaleNum;			
	SETTLE_AMT _AuthSaleAmount;	

	SETTLE_NUM _VoidAuthSaleNum;		
	SETTLE_AMT _VoidAuthSaleAmount;

	SETTLE_NUM _PreAuthNum;				
	SETTLE_AMT _PreAuthAmount;

	SETTLE_NUM _VoidPreAuthNum;				
	SETTLE_AMT _VOidPreAuthAmount;	

	SETTLE_NUM _AdjustNum;				
	SETTLE_AMT _AdjustAmount;		

	SETTLE_NUM _TipsNum;			
	SETTLE_AMT _TipsAmount;
	
	SETTLE_NUM _RefundNum;				
	SETTLE_AMT _RefundAmount; 		

	SETTLE_NUM _OfflineNum;				
	SETTLE_AMT _OfflineAmount;		

	SETTLE_NUM _VoidOfflineNum;		
	SETTLE_AMT _VoidOfflineAmount;	
}STSETTLE;

typedef struct
{
	char sDebitNum[3];			
	char sDebitAmt[12];		
	char sCreditNum[3];			
	char sCreditAmt[12];		
	char sEmvTcCount[3];		
}STAMT_NUM_SETTLE;

typedef struct
{
	SETTLE_NUM nDebitNum;		
	SETTLE_AMT sDebitAmount;
	SETTLE_NUM nCreditNum;		
	SETTLE_AMT sCreditAmount;	
}STAMT_NUM_INFO_SETTLE;

typedef struct 
{
	char BatchNo[6+1];			
	char szInvoice[6+1];			
	char BatchDatetime[14+1];	
	char SettleFlag;		
	STSETTLE stSettle;			
}ST_SETTLE_REC;

extern int GetSettleData(STSETTLE *);
extern int DispTotal(void);
extern void ClearSettle(void);
extern int ChangeSettle(const STSYSTEM *);
extern int DealSettle(STAMT_NUM_INFO_SETTLE *, STSETTLE);
extern int DealSettleTask(void);
extern int FindSettle(void);
extern int Settle(char);
extern int DoSettle(void);
extern void InitSettleData(void);
extern int CheckTransAmount(const char *, const char);
extern int DealBatchUpFail(STTRANSRECORD *, int, int);
extern int SetSettleData(STSETTLE *pstSettle);
#endif


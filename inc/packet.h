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

#ifndef _PACKET_H_
#define _PACKET_H_


#define APP_REPIN	(-5)		/**/

typedef enum
{
	TDC_DEFAULT     = 0x00,     //default
	TDC_UNSETREVERSAL = 0x01,	//Set reversal flag
	TDC_OFFLINE		= 0x02
}ENM_TRADECOMPLETE;

typedef enum
{
	TDI_DEFAULT     = 0x00,     //default
	TDI_CHECKOPER 	= 0x01,		//Check is normal oper
}ENM_TRADEINIT;


extern int InitISO8583(void);
extern int CheckMac(const char *,  int);
extern int AddMac(char *, int *, char);
extern int ClrPack(void);
extern int SetField(const int , const char *, const int);
extern int GetField(const int, char *, int *);
extern int Pack(char *,  int *);
extern int Unpack(const char *, const int);
extern void DispResp(const char *);
extern int ChkRespIsSucc(YESORNO, const char *);
extern int DealPackAndComm(char *, EM_OPERATEFLAG, STSYSTEM *, STREVERSAL *,int);
extern int TradeComplete(char* , const STSYSTEM *, STTRANSRECORD *, const char *);
extern int PackGeneral(STSYSTEM *, STTRANSRECORD *, STTRANSCFG );
#endif


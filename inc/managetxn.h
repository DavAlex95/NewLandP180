/***************************************************************************
** Copyright (c) 2019 Newland Payment Technology Co., Ltd All right reserved   
** File name:  managetxn.h
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
#ifndef _MANAGETXN_H_
#define _MANAGETXN_H_

#define FILE_BLACKLIST APP_NAME"BLCKLIST"

extern void DealSystem(STSYSTEM *);
extern int ChkLoginStatus(void);
extern int ChkRespon(STSYSTEM *, const char *);
extern int ChkPosLimit(void);
extern int ChkRespMsgID(const char *, const char *);
extern int Login(void);
extern void DealSend(void);
extern int ChkTransOnOffStatus(char cTransType);
extern int CheckAutoReboot();
extern int DealPosLimit(void);

#endif


/**************************************************************************
* Copyright (C) 2019 Newland Payment Technology Co., Ltd All Rights Reserved
*
* @file callback.h
* @brief  
* @version 1.0
* @author 
* @date 2019-9-2
**************************************************************************/


#ifndef _EMV_CALLBACK_H_
#define _EMV_CALLBACK_H_

// tag 1F8139 for pinpad
enum{
	PINPAD_L3_CALLBACK_NOTIFICATION 			= (1 << 7),
	PINPAD_L3_CALLBACK_SELECT_CANDIDATE_LIST 	= (1 << 6),
	PINPAD_L3_CALLBACK_CHECK_CREDENTIALS		= (1 << 5),
	PINPAD_L3_CALLBACK_AFTER_FINAL_SELECT		= (1 << 4),
	PINPAD_L3_CALLBACK_GET_PIN					= (1 << 3),
	PINPAD_L3_CALLBACK_CARDNUMBER_CONFIRM		= (1 << 2),
};

#define PERFORM_SKIPPIN 	(-610)
#define PERFORM_MANUNAL		(-523)

extern void SaveInputAMT(char* );
extern void InitInputAMT(void);
extern void SetupCallbackFunc(void);
extern void GetCardEventData(char *pszOut);
extern void GetManualExpiryDate(char *pszOut);
extern void GetManualCVV(char *pszOut);
extern void EmvGetOnlinePin(char *);
extern void EmvSetOnlinePin(char *psOnlinePin);
extern void SetEmvDebugPort(int );
extern int CtrlCardPan(uchar *, const uint, const uchar, const uint);
extern int DispPan(char *);
extern void EmvClrOnlinePin();
extern int PinPad_PerformTransaction(char *pszInput, int nInPutLen, L3_TXN_RES *res, STSYSTEM *pstSystem, char *pszResPonseCode);

extern void ResetVirtualkbStatus();
extern void SetCurTransType(char cTransType);

#endif


/***************************************************************************
** Copyright (c) 2023 Newland Payment Technology Co., Ltd All right reserved
** File name: ntoms_callback.h
** File identifier:
** Synopsis:
** Current Version:  v1.0
** Author: Jax Ji
** Complete date: 2023-09-18
** Modify record:
** Modify date:
** Version:
** Modify content:
***************************************************************************/

#ifndef _NTOMS_CALLBACK_H_
#define _NTOMS_CALLBACK_H_

#ifdef USE_NTOMS

#include "libapiinc.h"

extern int NTOMS_CB_ShowMenu(char *title, char **menu, int cnt);

extern NTOMS_ERRCODE NTOMS_CB_UpdateAppParam(const char *szFilePath, const char *pszStoragePath, const char* szAppName);

extern void NTOMS_CB_OutputLog(const char *log, int len);

extern NTOMS_ERRCODE NTOMS_CB_ShowUiMsg(EM_NTOMS_UI_ID_T emUIID,char* title, char* line1, char* line2);

extern int NTOMS_CB_SwitchDialog(char *title, char *info, int def);

extern int NTOMS_CB_InputDialog(char *title, char *tip, char *str, int maxLen);

extern void NTOMS_CB_DemoMode(int demoMode, char *oid);

extern void NTOMS_CB_AppQuit(char *cause);

#endif // end of USE_NTOMS

#endif // end of _NTOMS_CALLBACK_H_

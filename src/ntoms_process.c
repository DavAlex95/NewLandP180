/***************************************************************************
** Copyright (c) 2023 Newland Payment Technology Co., Ltd All right reserved
** File name: ntoms_process.c
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

#ifdef USE_NTOMS

#include "ntoms_process.h"
#include "ntoms_callback.h"

#include "appinc.h"


int NTOMS_proInit()
{
	// NTOMS need callback functions below to be defined.
	ASSERT_FAIL(NTOMS_SetOption(NTOMS_OPT_CONF_CAPABILITY, NTOMS_SUBC_ALL_APPS));

	ASSERT_FAIL(NTOMS_SetOption(NTOMS_OPT_EXEC_SHOW_MENU, NTOMS_CB_ShowMenu));

	ASSERT_FAIL(NTOMS_SetOption(NTOMS_OPT_EXEC_UPT_PARAM, NTOMS_CB_UpdateAppParam));

	ASSERT_FAIL(NTOMS_SetOption(NTOMS_OPT_EXEC_OUTPUT_LOG, NTOMS_CB_OutputLog));

	ASSERT_FAIL(NTOMS_SetOption(NTOMS_OPT_EXEC_SHOW_UI_MESSAGE, NTOMS_CB_ShowUiMsg));

	ASSERT_FAIL(NTOMS_SetOption(NTOMS_OPT_EXEC_SWITCH_DLG, NTOMS_CB_SwitchDialog));

	ASSERT_FAIL(NTOMS_SetOption(NTOMS_OPT_EXEC_INPUT_DLG, NTOMS_CB_InputDialog));

	ASSERT_FAIL(NTOMS_SetOption(NTOMS_OPT_EXEC_DEMO_MODE, NTOMS_CB_DemoMode));

	ASSERT_FAIL(NTOMS_SetOption(NTOMS_OPT_CMD_QUIT_APP, NTOMS_CB_AppQuit));

	ASSERT_FAIL(NTOMS_Initialize());

	return APP_SUCC;
}

void NTOMS_ProDealLockTerminal()
{
	char szPasswd[20] = {0};
	int nOutLen;

	if (!GetLockTerminal())
	{
		return;
	}

	while(PubInputDlg(tr("Lock"), GetLockPromptInfo(), szPasswd, &nOutLen, 1, 20, 60, INPUT_MODE_PASSWD) != APP_SUCC
		  || strcmp(szPasswd, "1234") != 0);
}


#endif // end of USE_NTOMS

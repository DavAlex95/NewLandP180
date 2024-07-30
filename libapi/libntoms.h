/***************************************************************************
** Copyright (c) 2019 Newland Payment Technology Co., Ltd All right reserved
** File name:  libnTOMS
** File indentifier:
** Brief:
** Current Verion:  v1.0.4
** Auther: Tom chen
** Modify record:
** Modify date:
** Version:
** Modify content:
***************************************************************************/
#ifndef _LIBNTOMS_H_
#define _LIBNTOMS_H_

typedef enum {
    NTOMS_OK,
    NTOMS_ERR_FAIL = -1,
    NTOMS_ERR_PARAM = -2,
    NTOMS_ERR_QUIT = -3,
} NTOMS_ERRCODE;

typedef enum {
    EM_NTS_NONE,
    EM_NTS_RUN_FAIL,
    EM_NTS_CONNECT_FAIL,
    EM_NTS_CONNECT_ING,
    EM_NTS_CONNECT_DONE,
} EM_NTOMS_STATUS_T;


/**
* @brief  subscrible capability,
*  NTOMS_SUBC_SELF_PARAM:   TOMS service will notify your APP Parameter
*  NTOMS_SUBC_SELF_APP:     TOMS service will notify your own APP
*  NTOMS_SUBC_ALL_APPS:     TOMS service will notify your own APP, other APP and patch
*/
typedef enum {
    NTOMS_SUBC_SELF_PARAM = (1 << 0),
    NTOMS_SUBC_SELF_APP   = (1 << 1),
    NTOMS_SUBC_ALL_APPS   = (1 << 2),
} NTOMS_SUB_CAP_T;

typedef enum {
    NTOMS_OPT_CONF_APP_NAME         = 0,	 // For TOMS service notifying specific app, you can ignore it because TOMS service can directly read the app name from app package
    NTOMS_OPT_CONF_CAPABILITY,               // NTOMS_SUB_CAP_T
    NTOMS_OPT_CONF_CUSTOMIZE_URL,
    NTOMS_OPT_CONF_MIDDLEWARE_URL,

    NTOMS_OPT_EXEC_SHOW_MENU        = 2000,  // nroms_exec_show_menu_cb
    NTOMS_OPT_EXEC_UPT_PARAM,                // ntoms_exec_update_param_cb
    NTOMS_OPT_EXEC_SHOW_UI_MESSAGE,			 // ntoms_exec_show_ui_msg_cb
    NTOMS_OPT_EXEC_OUTPUT_LOG,				 // ntoms_exec_output_log_cb
    NTOMS_OPT_EXEC_SWITCH_DLG,				 // ntoms_exec_switch_dlg_cb
    NTOMS_OPT_EXEC_INPUT_DLG,				 // ntoms_exec_input_dlg_cb
    NTOMS_OPT_EXEC_DEMO_MODE,				 // ntoms_exec_demo_mode_cb

    NTOMS_OPT_CMD_QUIT_APP          = 3000,  // ntoms_cmd_quit_app_cb
    NTOMS_OPT_CMD_REFRESH_SCREEN,            // ntoms_cmd_refresh_screen_cb
} EM_NTOMS_OPT;

typedef enum {
    NTOMS_UI_PROCESSING,
    NTOMS_UI_ERROR,
    NTOMS_UI_SUCC
} EM_NTOMS_UI_ID_T;

/**
* @brief  User API which should output menulist by menu parameter.
* for example:
	menu = {
			"1. hello",
			"2. OTA"
			}
	cnt = 2;
	You should show these items in your screen, then when user selects one of them, the sequence number will be returned
* @param [in] title
* @param [in] menu array
* @param [in] the count of menu
* @param [in] def selected
* @return
* @li int : user select item number. >= 1
*/
typedef int (*ntoms_exec_show_menu_cb)(char *title, char **menu, int cnt, int def);
/**
* @brief  User API which shoud parse APP Parameter file
* @param [in] cfgPath : file path in our terminal (like /app/share/xxxx/a),  you need to parse this file. it can be json or xml ...
* @param [in] filesStorage: you can configure file in parameter like picture, and lib will return the directory then you can get the file by 'strcat' the file name and path
* @return
* @li NTOMS_ERRCODE
*/
typedef NTOMS_ERRCODE (*ntoms_exec_update_param_cb)(const char *cfgPath, const char *filesStorage);

/**
* @brief Display the information during interaction between your app and TOMS service
*       if the text is too long , it will be seperated to three lines, currently we only focus on line1. line 2 and line3 are for extended
* since v1.0.2 modified
* @param [in] emUIID
* @param [in] line1
* @param [in] line2
* @param [in] title
* @return
* @li NTOMS_ERRCODE
*/
typedef NTOMS_ERRCODE (*ntoms_exec_show_ui_msg_cb)(EM_NTOMS_UI_ID_T emUIID, char* title, char* line1, char* line2);

/**
* @brief TOMS lib`s debug log in your app
* @param [in] log: log text
* @param [in] nlen: the length of log
* @return
* @li void
*/
typedef void  (*ntoms_exec_output_log_cb)(const char* log, int nLen);

/**
* @brief Switch dialog, in your lib, we will use switch dialog to do some check, like whether to open demo oid
* @param [in] title
* @param [in] info
* @param [in] def: default value
* @return
* @li int  -1: fail  0: close , 1: open
*/
typedef int  (*ntoms_exec_switch_dlg_cb)(char *title, const char* info, int def);  //return -1. 0, 1

/**
* @brief input dialog, our lib will use this to wait for user input
* @param [in] title
* @param [in] tip
* @param [out] str: user input string value
* @param [in] max : max length of str
* @return
* @li int -1 : fail  0: success
*/
typedef int  (*ntoms_exec_input_dlg_cb)(char *title, char *tip, char* str, int maxlen);

/**
* @brief TOMS service will notify your app whether it's demo mode, and demo oid value, you can set it in your app
* 		oid means operator id, different TOMS account has different oid
* @param [in] demoMode
* @param [in] oid
* @return
* @li void
*/
typedef void  (*ntoms_exec_demo_mode_cb)(int demoMode, char *oid);


/**
* @brief libntoms will call this callback after recieving quit cmd, user's app should free owner memory and quit
* @param [in] cause : toms_service will notify the quit app reason
* @return
* @li void
*/
typedef void  (*ntoms_cmd_quit_app_cb)(char *cause);

/**
* @brief libntoms will call this callback after recieving refresh screen
* @param [in]
* @return
* @li void
*/
typedef void  (*ntoms_cmd_refresh_screen_cb)(void);

/**
* @brief  Set callback functions, it is basical user APP api, in our TOMS lib, it will call callback automatically
* @param [in] emOpt, refer to EM_NTOMS_OPT
* @param [...] Variable parameters
* @return
* @li NTOMS_ERRCODE
*/
__attribute__((visibility("default")))
NTOMS_ERRCODE NTOMS_SetOption(EM_NTOMS_OPT emOpt, ...);

/**
* @brief  After setting option callback, you need to call initialize api, it will asynchronously connect toms service
* @return
* @li NTOMS_ERRCODE
*/
__attribute__((visibility("default")))
NTOMS_ERRCODE NTOMS_Initialize(void);

/**
* @brief Check the status between your App and TOMS service
* @param [out] status
* @return
* @li NTOMS_ERRCODE
*/
__attribute__((visibility("default")))
NTOMS_ERRCODE NTOMS_GetStatus(EM_NTOMS_STATUS_T *status);

/**
* @brief Show notify menu, when you call this function, it will show current updated items
* @param [in] title
* @return
* @li NTOMS_ERRCODE
*/
__attribute__((visibility("default")))
NTOMS_ERRCODE NTOMS_ShowNotifMenu(char *title);

/**
* @brief Trigger check update right now
* @return
* @li NTOMS_ERRCODE
*/
__attribute__((visibility("default")))
NTOMS_ERRCODE NTOMS_CheckUpdate(void);

/**
* @brief Open TOMS service debug menu.
* @return
* @li NTOMS_ERRCODE
*/
__attribute__((visibility("default")))
NTOMS_ERRCODE NTOMS_ShowDebugMenu(void);

/**
* @brief User can use this api to set demo oid for test, but it will be rejected by TOMS service in Production device
* @return
* @li NTOMS_ERRCODE
*/
__attribute__((visibility("default")))
NTOMS_ERRCODE NTOMS_SetOID(int demoMode, char *oid);

/**
* @brief The TOMS Service URL setting
* @param [in] title
* @return
* @li NTOMS_ERRCODE
* @note:
*    if you set NTOMS_OPT_CONF_CUSTOMIZE_URL callback.
*    when you restart terminal. all the urls will be customized url
*/
__attribute__((visibility("default")))
NTOMS_ERRCODE NTOMS_SetUrls(char *title);

/**
* @brief in local deployment, if a middleware server is used, you need to set this ip
* @param [in] title
* @return
* @li NTOMS_ERRCODE
* @note:
*    if you set NTOMS_OPT_CONF_MIDDLEWARE_URL callback.
*    when you restart terminal. this url will be customized url
*/
__attribute__((visibility("default")))
NTOMS_ERRCODE NTOMS_SetMiddleWareUrl(char *title);


/**
* @brief set toms service switch
* since v1.0.2
* @param
* @return
* @li NTOMS_ERRCODE
* @note:
*/
__attribute__((visibility("default")))
NTOMS_ERRCODE NTOMS_SetServiceSwitch(char *title);


/**
* @brief Automatically Update parameters
* since v1.0.2
* @param
* @return
* @li NTOMS_ERRCODE
* @note:
*/
__attribute__((visibility("default")))
NTOMS_ERRCODE NTOMS_AutoUpdateParameter(char *title);

/**
* @brief Get lib version
* since v1.0.2
* @param
* @return
* @li vesion
* @note:
*/
__attribute__((visibility("default")))
char *NTOMS_GetLibVersion(void);

__attribute__((visibility("default")))
NTOMS_ERRCODE NTOMS_GetTomsServiceVersion(void);

__attribute__((visibility("default")))
NTOMS_ERRCODE NTOMS_SetDevOrManVer(void);

__attribute__((visibility("default")))
NTOMS_ERRCODE NTOMS_SetBranch_Edition(void);

__attribute__((visibility("default")))
NTOMS_ERRCODE NTOMS_SetEnvironment(char *title);

__attribute__((visibility("default")))
NTOMS_ERRCODE NTOMS_SetStressTest_Counts(int nCount);

__attribute__((visibility("default")))
NTOMS_ERRCODE NTOMS_Fetch_Token(char * pszInPkgName, char * pszOutToken);

__attribute__((visibility("default")))
NTOMS_ERRCODE NTOMS_GetMssUrl(char * pszOutUrl);

__attribute__((visibility("default")))
NTOMS_ERRCODE NTOMS_Fetch_Oid(char * pszOutOid);

#endif /*_LIBNTOMS_H_*/


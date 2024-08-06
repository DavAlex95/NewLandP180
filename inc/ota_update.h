#ifndef _OTA_UPDATE_
#define _OTA_UPDATE_

#define MAX_OTA_FILE_BUFFER_LENGTH 2048
#define INSTRUCTION_OK "0"
#define OTA_APP_FILE "app.in"
#define OTA_APP_INSTALL_FILE "app.inst"
#define OTA_MASTER_FILE "master"
#define OTA_MASTER_INSTALL_FILE "imaster"
#define OTA_DEVCFG_FILE "dmaster"
#define OTA_DEVCFG_INSTALL_FILE "dimaster"

#define MPOS_VARIABLE_OFFSET 0
#define MPOS_EXT_OFFSET 0
#define MPOS_VARIABLE_MINLEN 1024
#define Response_Code_ERR_PARM "-1"
#define Response_Code_Good "00"
#define Response_Code_Insufficient_Storage "-11"
#define INSTRUCTION_ERR_OTHER "99"
#define Response_Code_ERR_SEQUENCE "-22"
#define Response_Code_ERR_SIGN "-33"
#define Response_Code_General_error "-99"

#define OTA_STATUS_ACTIVE 0x00
#define OTA_STATUS_IDLE 0x01

#define EM_NEED_TO_UPDATE_APP 0x00
#define EM_NOTHING_TO_UPDATE 0x01
#define EM_NEED_TO_UPDATE_MASTER 0x02

#define UPDATE_APP 0x11
#define UPDATE_XML 0x22

/**
 *@brief	Upgrade application and write  application file.
 *@return
 *@li
 */
int INSTRUCTION_OTA(char *pMessage, int nMessageLength);

/**
 *@brief	write files £¨firmware£©
 *@return
 *@li
 */
int INSTRUCTION_WriteFile(char *pMessage, int nMessageLength);

/**
 *@brief	Upgrade firmware
 *@return
 *@li
 */
int OTA_UpdateMasterFromFileSystem();

extern void SYS_SetOTAStatus(char);
extern char SYS_GetOTAStatus();

#endif
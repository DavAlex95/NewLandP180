#ifndef _CONFIG_FILE_H
#define _CONFIG_FILE_H

#define ME30S_CONFIG "config.txt"

typedef enum {
  FILE_TXT = 0,              // key for a text
  FILE_BINESECR,             // key for the bines table
  FILE_BINESPINPAD,          // key for the bines table
  FILE_VERSIONBINES,         // key for the version of bines table
  FILE_REALCIPHERCOUNTER,    // key for the real counter
  FILE_FAILEDCIPHERCOUNTER,  // key for the counter fails
  FILE_BINTABLE,             // key for the bin table
} DESC_KEY;

typedef struct {
  DESC_KEY key;
  char value[350];
} CONF_FILEPROP;

/**
 *@brief 		Carga el archivo config.txt
 *@return
  On success, it returns \ref APP_SUCC "APP_SUCC"; on error, it returns \ref
 APP_FAIL "APP_FAIL".
*/
int loadConfig();
/**
 *@brief 		Cambia el valor de la propiedad en el archivo
 *@param[in]    key key: DESC_KEY
 *@param[in]   value valor a cambiar
 *@return
  On success, it returns \ref NAPI_OK "NAPI_OK"; on error, it returns \ref
 EM_NAPI_ERR "EM_NAPI_ERR".
*/
int setProperty(DESC_KEY key, char *value);
/**
 *@brief 		Obtiene el valor de la propiedad en el archivo
 *@param[in]    key key: DESC_KEY
 *@param[out]   output valor de la popiedad en el archivo
 *@return
  On success, it returns \ref APP_SUCC "APP_SUCC"; on error, it returns \ref
 APP_FAIL "APP_FAIL".
*/
int getProperty(DESC_KEY key, char *output);
/**
 *@brief 		Checa si es la primera vez que se corre la aplicacion
 *@return
  On success, it returns \ref APP_SUCC "APP_SUCC"; on error, it returns \ref
 APP_FAIL "APP_FAIL".
*/
int IsFirstRunChk();

#endif
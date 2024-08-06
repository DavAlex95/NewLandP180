#include "config_file.h"

#include "log.h"

#include "libapiinc.h"

#include "apiinc.h"
#include "appinc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Variable que guarda la configuracion de archivo config.txt
CONF_FILEPROP saveProp[] = {
    {FILE_TXT, ""},
    {FILE_BINESECR, ""},
    {FILE_BINESPINPAD, ""},
    {FILE_VERSIONBINES, ""},
    {FILE_REALCIPHERCOUNTER, ""},
    {FILE_FAILEDCIPHERCOUNTER, ""},
    {FILE_BINTABLE, ""},
};

static int SaveItemsFromFile() {
  char auxvalue[350] = {0};
  char c;
  int i = 0, counterDesc = 0, nFd;

  // ABRIR ARCHIVO
  if (PubOpenFile(ME30S_CONFIG, "w", &nFd) != APP_SUCC) {
    return APP_FAIL;
  }

  PubFsSeek(nFd, 0L, SEEK_SET);

  while (read(nFd, &c, sizeof(c) != 0)) {
    if (c != '\n') {
      memcpy(auxvalue + i, &c, 1);
      i++;
    } else {
      strcat(saveProp[counterDesc].value, auxvalue);
      counterDesc++;
      i = 0;
      memset(auxvalue, 0x00, 350);
    }
  }

  // CERRAR ARCHIVO
  PubFsClose(nFd);

  for (i = 0; i < sizeof(saveProp) / sizeof(saveProp[0]); i++) {
    TRACE("KEY: %d  ,  VALUE: %s ", saveProp[i].key, saveProp[i].value);
  }

  return APP_SUCC;
}

int loadConfig() {
  int nRet = 0, nFd;

  // EN CASO DE QUERER REESTABLECER VALORES DE EL ARCHIVO ELIMINELO PRIMERO
  // PubFsDel(ME30S_CONFIG);

  if (PubFsExist(ME30S_CONFIG) != NAPI_OK) {
    // En caso de que no exista el archivo de configuracion lo creamos y lo
    // inicializamos en el orden que dicta la variable FILEPROP saveProp[]

    char text[] = {"CreandoOportunidades\n00000000\n00000000\n00\n0\n0\n\n"};
    int len = strlen(text);
    nRet = PubFsCreateDirectory(ME30S_CONFIG);

    log_debug("PubFsCreateDirectory: %d", nRet);
    if (PubOpenFile(ME30S_CONFIG, "w", &nFd) != APP_SUCC) {
      return APP_FAIL;
    }
    PubFsSeek(nFd, 0L, SEEK_SET);
    if (PubFsWrite(nFd, text, len) < 0) {
      log_debug("ERROR WRITE FILE");
      return APP_FAIL;
    }

    PubFsClose(nFd);

    // Salvamos valores en el mpos
    return SaveItemsFromFile();

  } else {
    log_debug("Ya existe el archivo de configuracion de la ME30S");
    // Salvamos valores en el mpos
    return SaveItemsFromFile();
  }
}

int setProperty(DESC_KEY key, char value[]) {
  int nFd, i = 0;
  char vector[350 + 1] = {0};

  PubFsDel(ME30S_CONFIG);

  // ABRIR ARCHIVO
  if (PubOpenFile(ME30S_CONFIG, "w", &nFd) != APP_SUCC) {
    return APP_FAIL;
  }

  for (i = 0; i < sizeof(saveProp) / sizeof(saveProp[0]); i++) {
    if (key == saveProp[i].key) {
      strcpy(saveProp[i].value, value);
    }
    sprintf(vector, "%s\n", saveProp[i].value);
    write(nFd, vector, strlen(vector));
  }

  // CERRAR ARCHIVO
  PubFsClose(nFd);
  return APP_SUCC;
}

int getProperty(DESC_KEY key, char *output) {
  int i;
  for (i = 0; i < sizeof(saveProp) / sizeof(saveProp[0]); i++) {
    if (saveProp[i].key == key) {
      strcat(output, saveProp[i].value);
      return APP_SUCC;
    }
  }
  return APP_FAIL;
}

int IsFirstRunChk() {
  int  nFd;
  char auxvalue[30] = {0};

  // ABRIR ARCHIVO
  if (PubOpenFile(ME30S_CONFIG, "r", &nFd) != APP_SUCC) {
    return APP_FAIL;
  }

  PubFsSeek(nFd, 0L, SEEK_SET);
  read(nFd, auxvalue, 22);
  TRACE("FIRST RUN: %s", auxvalue);

  PubFsClose(nFd);
  if (memcmp(auxvalue, "FIRSTRUN", 8) == 0) {
    TRACE("SI ES LA PRIMERA VEZ");
    return APP_SUCC;
  } else {
    return APP_FAIL;
  }

  return APP_SUCC;
}

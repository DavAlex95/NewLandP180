/**
 * @file et_token.c
 * @author David Alexis Fuentes S.
 * @brief
 * @version 0.1
 * @date 2021-07-26
 *
 * @copyright Copyright (c) 2021
 *
 */
#include "bines_table.h"

#include "config_file.h"
#include "debug.h"
#include "tool.h"

#include <log.h>

BIN_STRUCTURE Bin_Table[20];
int contIdTable;

int Load_BinesTable() {
  char binTable[350];
  getProperty(FILE_BINTABLE, binTable);

  if (!(strlen(binTable) > 0)) {
    log_debug("TABLA DE BINES NO EXISTE");
    return -1;
  }

  Parse_BinesTable(binTable);
  return 0;
}

static int Separate_Ranges(char *block,
                           unsigned long long *r1,
                           unsigned long long *r2,
                           int *len,
                           int *len2) {
  char auxrange[10] = {0};
  int i;

  for (i = 0; i < strlen(block); i++) {
    if (block[i] == 'B') {
      memcpy(auxrange, block, i);
      *len = strlen(auxrange);
      if (strlen(block) < 1 && strlen(block) > 12) {
        return -1;
      }
      if (PubIsDigitStr(auxrange) != APP_SUCC) {
        return -1;
      }
      *r1 = AtoLL(auxrange);

      memcpy(auxrange, block + (i + 1), strlen(block));
      *len2 = strlen(auxrange);
      if (strlen(block) < 1 && strlen(block) > 12) {
        return -1;
      }
      if (PubIsDigitStr(auxrange) != APP_SUCC) {
        return -1;
      }
      *r2 = AtoLL(auxrange);
      return 0;
    }
  }

  return 0;
}

static int Compare_Ranges(char *block) {
  char *pch;
  unsigned long long range = 0;
  unsigned long long range2 = 0;
  int lenRange = 0;
  int lenRange2 = 0;
  // Si encuentra que rangos los separa

  pch = strchr(block, 'B');
  if (pch != NULL) {
    log_debug("Bloque de Rangos");
    if (Separate_Ranges(block, &range, &range2, &lenRange, &lenRange2) == 0) {
      log_debug("De %d a %d", range, range2);

      Bin_Table[contIdTable].len_BIN_Minimum = lenRange;
      Bin_Table[contIdTable].minimum_BIN_Value = range;
      Bin_Table[contIdTable].len_BIN_Maximum = lenRange2;
      Bin_Table[contIdTable].maximum_BIN_value = range2;
    } else {
      return -1;
    }

  } else {
    log_debug("Un solo Bloque");
    // Validamos la longitud de los bloques
    if (strlen(block) < 1 && strlen(block) > 12) {
      return -1;
    }
    // Validamos que sean digitos
    if (PubIsDigitStr(block) != APP_SUCC) {
      return -1;
    }
    log_debug("De %llu a %llu", AtoLL(block), AtoLL(block));

    lenRange = strlen(block);
    lenRange2 = strlen(block);
    Bin_Table[contIdTable].len_BIN_Minimum = lenRange;
    Bin_Table[contIdTable].minimum_BIN_Value = AtoLL(block);
    Bin_Table[contIdTable].len_BIN_Maximum = lenRange2;
    Bin_Table[contIdTable].maximum_BIN_value = AtoLL(block);
  }

  return 0;
}

static int Validate_RangeOfBines(char *binesTable) {
  char delimitador[] = "AC";
  char *block_range = strtok(binesTable, delimitador);
  int nRet = 0;

  contIdTable = 0;
  if (block_range != NULL) {
    while (block_range != NULL && block_range[0] != 'F') {
      log_debug("<-------------------------------------------------------->");
      log_debug("Bloque: %s", block_range);

      Bin_Table[contIdTable].id_Range = (contIdTable + 1);
      nRet = Compare_Ranges(block_range);
      if (nRet == 1) {
        return 1;
      } else if (nRet == -1) {
        return -1;
      }

      block_range = strtok(NULL, delimitador);
      contIdTable++;
    }
  }
  return 0;
}

int Parse_BinesTable(char *binesTable) {
  int nRet = 0;
  char auxBinesTable[350] = {0};
  memcpy(auxBinesTable, binesTable, strlen(binesTable));

  memset(Bin_Table, 0x00, sizeof(Bin_Table));
  log_debug("INICIA PARSEO DE TABLA DE BINES");

  nRet = Validate_RangeOfBines(auxBinesTable);

  // En caso de error de parseo vaciamos la Tabla de bines
  if (nRet == -1) {
    memset(Bin_Table, 0x00, sizeof(Bin_Table));
  }

  return nRet;
}

int CompareBin(char *pan) {
  int i = 0;
  unsigned long long TMP_MINBIN = 0, TMP_MAXBIN = 0;
  char auxBin[12] = {0};

  if (contIdTable == 0) {
    log_error("TABLA DE BINES NO EXISTE");
    return -1;
  }

  log_debug("PAN a comparar: %s", pan);
  log_debug("Contador ID de Tabla de Bines: %d", contIdTable);
  for (i = 0; i < contIdTable; i++) {
    //	Comparar si el Bin se encuentra dentro de un rango de la Tabla de Bines.
    memcpy(auxBin, pan, Bin_Table[i].len_BIN_Minimum);
    TMP_MINBIN = AtoLL(auxBin);
    memset(auxBin, 0x00, 12);
    memcpy(auxBin, pan, Bin_Table[i].len_BIN_Maximum);
    TMP_MAXBIN = AtoLL(auxBin);
    memset(auxBin, 0x00, 12);

    log_debug(
        "TMP_MINBIN: %llu  >=  MIN_VALUE: %llu && TMP_MAXBIN: %llu  >=  "
        "MAX_VALUE: "
        "%llu",
        TMP_MINBIN, Bin_Table[i].minimum_BIN_Value, TMP_MAXBIN,
        Bin_Table[i].maximum_BIN_value);

    if (TMP_MINBIN >= Bin_Table[i].minimum_BIN_Value &&
        TMP_MAXBIN <= Bin_Table[i].maximum_BIN_value) {
      log_debug("SE ENCONTRO EN TABLA DE BINES ,NO SE CIFRAN DATOS.");
      return 1;
    }
  }

  return 0;
}
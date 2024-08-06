/**
 * @file bines_table.h
 * @author David Alexis Fuentes S.
 * @brief
 * @version 0.1
 * @date 2021-07-26
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef _BINES_TABLE_H_
#define _BINES_TABLE_H_

typedef struct {
  int id_Range;
  int len_BIN_Minimum;
  unsigned long long minimum_BIN_Value;
  int len_BIN_Maximum;
  unsigned long long maximum_BIN_value;
} BIN_STRUCTURE;

/**
 * @brief Valida y descompone rangos de la tabla de bines
 *
 * @return int 0
 */
int Parse_BinesTable(char *binesTable);

int Load_BinesTable();

#endif
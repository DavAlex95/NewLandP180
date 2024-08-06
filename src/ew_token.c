/**
 * @file ew_token.c
 * @author Guillermo Garcia Maynez R. (ggarcia@necsweb.com)
 * @brief
 * @version 0.1
 * @date 2021-05-14
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <log.h>
#include <tk.h>
#include <utils.h>

#include <string.h>

void GenerateEwToken(struct EncryptedTransportKey *tk, char output[549]) {
  char hexEncryptedTk[512 + 1], hexCheckValue[3 + 1];
  char hexTkCRC32[8 + 1] = {0};
  unsigned int tkCRC32 = 0;

  log_trace("Generando token EW...");

  strcpy(output, "! EW00538 ");

  //BytesToHexString(tk->encryptedKey, RSA_KEY_SIZE_BYTES, &hexEncryptedTk);
  BytesToHexString(tk->encryptedKey, RSA_KEY_SIZE_BYTES, hexEncryptedTk);
  strcat(output, hexEncryptedTk);
  log_debug("TK cifrada: %s", hexEncryptedTk);

  //BytesToHexString(tk->kcv, 3, &hexCheckValue);
  BytesToHexString(tk->kcv, 3, hexCheckValue);
  strcat(output, hexCheckValue);
  log_debug("KCV: %s", hexCheckValue);

  // RSA version
  strcat(output, "BCMER001  ");

  // Padding
  strcat(output, "01");
  log_debug("Padding: 01");

  // CRC32(hexEncryptedTk, 512, &tkCRC32);
  tkCRC32 = Crc32b(hexEncryptedTk);
  snprintf(hexTkCRC32, 8 + 1, "%08X", tkCRC32);
  strcat(output, hexTkCRC32);
  log_debug("CRC32: %s", hexTkCRC32);

  log_trace("Finalizo la generacion del token EW");
}

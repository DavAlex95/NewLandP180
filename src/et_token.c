/**
 * @file et_token.c
 * @author Guillermo Garcia Maynez R. (ggarcia@necsweb.com)
 * @brief
 * @version 0.1
 * @date 2021-05-23
 *
 * @copyright Copyright (c) 2021
 *
 */

#include "debug.h"

#include <et_token.h>
#include <log.h>

int parse_et(const char *et, struct EtToken *out) {
  
  int inEt = 0;

  inEt += 10;

  log_debug("Iniciando Parseo de Token ET");
  log_debug("Len et: %d", strlen(et));

    memcpy(out->binTableId, et + inEt, 8);
    log_debug("out->binTableId: %s", out->binTableId);
    inEt += 8;

    memcpy(out->version, et + inEt, 2);
    log_debug("out->version: %s", out->version);
    inEt += 2;

    memcpy(out->encryptionKsn, et + inEt, 20);
    log_debug("out->encryptionKsn: %s", out->encryptionKsn);
    inEt += 20;

    memcpy(out->buffLenEncryption, et + inEt, 4);
    log_debug("out->buffLenEncryption: %s", out->buffLenEncryption);
    inEt += 4;

    memcpy(out->actualLength, et + inEt, 4);
    log_debug("out->actualLength: %s", out->actualLength);
    inEt += 4;

    memcpy(out->bufferEncryption, et + inEt, 320);
    log_debug("out->bufferEncryption: %s", out->bufferEncryption);
    inEt += 320;

    memcpy(out->crc32, et + inEt, 8);
    log_debug("out->crc32: %s", out->crc32);
    inEt += 8;

  log_debug("Finalizo exitosamente Parseo de Token ET");
  return 0;
}
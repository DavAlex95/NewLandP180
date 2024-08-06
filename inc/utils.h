#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdint.h>
#include <stdlib.h>

#define PINPAD_BYTE_BUF_LEN 2048

int BytesToHexString(const unsigned char *inputBytes,
                     unsigned int inputLen,
                     char *output);

int HexStringToBytes(const char *hexStr,
                     unsigned int *outputLen,
                     unsigned char *output);

void CRC32(const void *data, size_t n_bytes, uint32_t *crc);

#endif

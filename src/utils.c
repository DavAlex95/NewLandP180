/**
 * @file utils.c
 * @author Guillermo Garcia Maynez R. (ggarcia@necsweb.com)
 * @brief
 * @date 2021-04-29
 *
 * @copyright Copyright (c) 2021 New England Computer Solutions
 *
 */

#include "apiinc.h"
#include "libapiinc.h"
#include "appinc.h"
#include "utils.h"

/**
 * @brief Convierte un arreglo de bytes a una cadena hexadecimal.
 *
 * @warning El tamaño del arreglo de salida debe ser del doble que la entrada
 * original mas el espacio para el caracter nulo.
 *
 * @param inputBytes
 * @param inputLen
 * @param output
 * @return int
 */
int BytesToHexString(const unsigned char *inputBytes,
                     unsigned int inputLen,
                     char *output) {
  int byteNo;
  if (inputLen > PINPAD_BYTE_BUF_LEN) {
    return -1;
  }

  for (byteNo = 0; byteNo < inputLen; byteNo++) {
    output[2 * byteNo] = (inputBytes[byteNo] >> 4) + 48;
    output[2 * byteNo + 1] = (inputBytes[byteNo] & 15) + 48;
    if (output[2 * byteNo] > 57) {
      output[2 * byteNo] += 7;
    }
    if (output[2 * byteNo + 1] > 57) {
      output[2 * byteNo + 1] += 7;
    }
  }

  output[inputLen * 2] = '\0';

  return 0;
}

/**
 * @brief Convierte una cadena hexadecimal a bytes
 *
 * @warning El filtrado de caracteres hexadecimales debe hacerse previamente.
 * Esta función no validará los caracteres, por ejemplo una P.
 *
 * @param hexStr
 * @param output
 * @param outputLen
 * @return int
 */
int HexStringToBytes(const char *hexStr,
                     unsigned int *outputLen,
                     unsigned char *output) {
  unsigned int inIdx = 0;                      
  unsigned int outIdx = 0;
  unsigned int len = strlen(hexStr);
  if (len % 2 != 0) {
    return -1;
  }
  unsigned int finalLen = len / 2;
  *outputLen = finalLen;
  for (inIdx = 0, outIdx = 0; outIdx < finalLen;
       inIdx += 2, outIdx++) {
    if (((hexStr[inIdx] - 48) <= 9 || (hexStr[inIdx] - 65) <= 5) &&
        ((hexStr[inIdx + 1] - 48) <= 9 || (hexStr[inIdx + 1] - 65) <= 5)) {
      output[outIdx] = (hexStr[inIdx] % 32 + 9) % 25 * 16 +
                       (hexStr[inIdx + 1] % 32 + 9) % 25;
    } else {
      *outputLen = 0;
      return -1;
    }
  }

  return 0;
}

static uint32_t crc32_for_byte(uint32_t r) {
  int j = 0;
  for (j = 0; j < 8; ++j) r = (r & 1 ? 0 : (uint32_t)0xEDB88320L) ^ r >> 1;
  return r ^ (uint32_t)0xFF000000L;
}

void CRC32(const void *data, size_t n_bytes, uint32_t *crc) {
  size_t i = 0;
  static uint32_t table[0x100];
  if (!*table)
    for (i = 0; i < 0x100; ++i) table[i] = crc32_for_byte(i);
  for (i = 0; i < n_bytes; ++i)
    *crc = table[(uint8_t)*crc ^ ((uint8_t *)data)[i]] ^ *crc >> 8;
}

// ----------------------------- crc32b --------------------------------

/* This is the basic CRC-32 calculation with some optimization but no
table lookup. The the byte reversal is avoided by shifting the crc reg
right instead of left and by using a reversed 32-bit word to represent
the polynomial.
   When compiled to Cyclops with GCC, this function executes in 8 + 72n
instructions, where n is the number of bytes in the input message. It
should be doable in 4 + 61n instructions.
   If the inner loop is strung out (approx. 5*8 = 40 instructions),
it would take about 6 + 46n instructions. */

unsigned int Crc32b(unsigned char *message) {
  int i, j;
  unsigned int byte, crc, mask;

  i = 0;
  crc = 0xFFFFFFFF;
  while (message[i] != 0) {
    byte = message[i];  // Get next byte.
    crc = crc ^ byte;
    for (j = 7; j >= 0; j--) {  // Do eight times.
      mask = -(crc & 1);
      crc = (crc >> 1) ^ (0xEDB88320 & mask);
    }
    i = i + 1;
  }
  return ~crc;
}

/**
 * @file et_token.h
 * @author Guillermo Garcia Maynez R. (ggarcia@necsweb.com)
 * @brief
 * @version 0.1
 * @date 2021-05-23
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef _ET_TOKEN_H_
#define _ET_TOKEN_H_

struct EtToken {
  char binTableId[8 + 1]; /*!< ID de la tabla de BINES. */
  char version[2 + 1];    /*!< Versión de la tabla de BINES Locales. */
  char encryptionKsn[20 +
                     1]; /*!< KSN con el cual se ha diversificado la llave BDK
                            en el HostBancario para cifrar la Tabla de BINES. */
  char buffLenEncryption[4 + 1]; /*!< Cantidad real de posiciones del buffer. */
  char actualLength[4 + 1]; /*!< Cantidad real deposiciones que ocupa la tabla
                               de BINES Locales una vez descifrado el BUFFER*/
  char bufferEncryption[320 + 1]; /*!< BLOQUES de la tabla de BINES CIFRADOS
                                     usando 3DES inverso */
  char crc32[8 + 1]; /*!< CRC32 calculado sobre la porción usada del BUFFER
                        CIFRADO */
};

/**
 * @brief Valida y descompone un token ET en sus campos
 *
 * @param ex Token ET como cadena de caracteres
 * @param out Estructura procesada
 * @return int 0
 */
int parse_et(const char *et, struct EtToken *out);

#endif
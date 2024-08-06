/**
 * @file ex_token.h
 * @author Guillermo Garcia Maynez R. (ggarcia@necsweb.com)
 * @brief
 * @version 0.1
 * @date 2021-05-14
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef _EX_TOKEN_H_
#define _EX_TOKEN_H_

struct ExToken {
  char encryptedIpek[32 + 1]; /*!< IPEK cifrada con la llave de transporte. */
  char initialKsn[20 + 1];    /*!< KSN inicial a cargar en la parte segura. */
  char kcv[6 + 1];            /*!< Valor de verificación de la IPEK. */
  char requestStatus[2 + 1];  /*!< Estado informado por el servidor sobre la
                           solicitud  de generación de llave IPEK */
  char crc32[8 + 1];          /*!< CRC32 sobre la llave cifrada */
};

/**
 * @brief Valida y descompone un token EX en sus campos
 *
 * @param ex Token EX como cadena de caracteres
 * @param out Estructura procesada
 * @return int 0
 */
int parse_ex(const char *ex, struct ExToken *out);

#endif
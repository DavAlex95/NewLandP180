/**
 * @file tk.h
 * @author Guillermo Garcia Maynez R. (ggarcia@necsweb.com)
 * @brief Operaciones con llave de transporte
 * @date 2021-04-29
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef _TK_H_
#define _TK_H_

#define RSA_KEY_SIZE_BYTES 256

struct EncryptedTransportKey {
  unsigned char
      encryptedKey[RSA_KEY_SIZE_BYTES]; /*!< Llave de transporte cifrada con la
                                           llave pública RSA configurada. */
  unsigned char kcv[3]; /*!< Valor de verificación de la llave de transporte */
};

void MbedTLSInit();

/**
 * @brief Genera una llave aleatoria de transporte cifrada con RSA
 *
 * @param key Estructura que tendrá la TK cifrada
 * @return int 0
 */
int GenerateTransportKey(struct EncryptedTransportKey *key);

int tk_decrypt(const unsigned char *input,
               unsigned int inputLen,
               unsigned char *output);

#endif

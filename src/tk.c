/**
 * @file tk.c
 * @author Guillermo Garcia Maynez (ggarcia@necsweb.com)
 * @brief Implementación usando mbedTLS de la generación de una aleatoria llave
 * de transporte, cifrado con RSA 2048 de dicha llave y cálculo de KCV.
 * @date 2021-04-26
 *
 * @copyright Copyright (c) 2021 New England Computer Solutions.
 *
 */

/*
#include <mbedtls/des.h>
#include <mbedtls/entropy.h>
#include <mbedtls/hmac_drbg.h>
#include <mbedtls/memory_buffer_alloc.h>
#include <mbedtls/platform.h>
#include <mbedtls/rsa.h>
#include <mbedtls/sha256.h>
*/

#include <log.h>
#include <napi_power.h>
#include <tk.h>

#include "string.h"
//#include <stdint.h>
//#include <stdlib.h>

#define TK_SIZE 16

#define EXIT_SUCCESS 0
#define EXIT_FAILURE -1

const char *bankModulus =
    "CF41C5307C06CEDEB52983CFA1DE0E2F397BC87970C14AB2EF0822739E6D64B89BF57170"
    "55F600420FC442A9E2E0C96AC62C27ED6C864BEDCAC119B7A469D3541532B660B4CB0ABA"
    "8C0D3CEAFD44D0F73C8C95D02E69DF6FD438C9D4092ACEDE830C3E8CABC7C3D2C7544963"
    "073738BA340C7C20240772BDCE9315DA070FA774C3C6B5050E21B02A594BA4E5EBCB3908"
    "E864AED79F9231B18D02152F255955D3D10CA006C18E9CD768E6137469A45BB82DFF1D9C"
    "8E095D467EC9150AC38AB84743D21AC1D82D310BC421D26A4B8901CE879E0B6F5DA981BE"
    "BCEDD0C17F39DB57DCEF68B1B574BDF8C817D209660D8145735C1899AFEE41C795E1CE46"
    "901ABADB";
const char *bankPubExponent = "010001";
const char *bankRsaHash =
    "925ceb72f740162c78513c0cd760b173a2dc60a369d32cd3a3f8d3cd38757520";

/*
const mbedtls_md_info_t *md_info;

static unsigned char mbedMemoryBuf[8192];
static unsigned char transportKey[TK_SIZE];

static mbedtls_entropy_context rsa_entropy;
static mbedtls_hmac_drbg_context rsa_hmac_drbg;

static mbedtls_entropy_context tk_entropy;
static mbedtls_hmac_drbg_context tk_hmac_drbg;

static mbedtls_rsa_context bankRsa;
static mbedtls_mpi N, E;
unsigned char encryptedTk[256] = {0};
*/


// mbedTLS necesita una funcion exit
//void exit(int status) {
//  log_warn("La funcion exit fue llamada con status: %d", status);
//  NAPI_SysReboot();
//}

// Entropía de hardware para la ME30S
/*
int mbedtls_hardware_poll(void *data,
                          unsigned char *output,
                          size_t len,
                          size_t *olen) {
  int ret;
  ret = NAPI_SecGetRandom(len, output);
  if (ret == NAPI_OK) {
    *olen = len;
  } else {
    log_error("Error al obtener bytes aleatorios. Código de error: %d", ret);
    *olen = 0;
  }
  return ret;
}
*/

/**
 * @brief Verifica que la llave RSA publica coincida con el hash SHA256
 * precargado.
 *
 * @return int 0
 */
static int verify_rsa_key() {
  char key[520];
  unsigned char sha256[32];
  strcpy(key, bankModulus);
  strcat(key, bankPubExponent);

  //mbedtls_sha256_ret(key, strlen(key), sha256, 0);

  // TODO: Añadir verificación
}

/**
 * @brief Inicializa mbedTLS para las llamadas subsecuentes de Capitulo X.
 *
 */
void MbedTLSInit() {
  /*
  int ret;
  mbedtls_memory_buffer_alloc_init(mbedMemoryBuf, sizeof(mbedMemoryBuf));

  if ((md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256)) == NULL) {
    log_error("No se pudo obtener informacion de hash");
    mbedtls_exit(EXIT_FAILURE);
  }

  mbedtls_hmac_drbg_init(&tk_hmac_drbg);
  mbedtls_entropy_init(&tk_entropy);
  mbedtls_hmac_drbg_set_prediction_resistance(&tk_hmac_drbg,
                                              MBEDTLS_HMAC_DRBG_PR_ON);
  ret = mbedtls_hmac_drbg_seed(&tk_hmac_drbg, md_info, mbedtls_entropy_func,
                               &tk_entropy, "CapXTK", 6);
  if (ret != 0) {
    log_error("Error al alimentar el DRBG de TK. Codigo de error: %d", ret);
    mbedtls_exit(EXIT_FAILURE);
  }

  mbedtls_hmac_drbg_init(&rsa_hmac_drbg);
  mbedtls_entropy_init(&rsa_entropy);

  ret = mbedtls_hmac_drbg_seed(&rsa_hmac_drbg, md_info, mbedtls_entropy_func,
                               &rsa_entropy, "CapXRSA", 7);
  if (ret != 0) {
    log_error("Error al alimentar el DRBG de RSA. Codigo de error: %d", ret);
    mbedtls_exit(EXIT_FAILURE);
  }

  mbedtls_mpi_init(&N);
  mbedtls_mpi_init(&E);
  mbedtls_rsa_init(&bankRsa, MBEDTLS_RSA_PKCS_V15, 0);

  if ((ret = mbedtls_mpi_read_string(&N, 16, bankModulus)) != 0 ||
      (ret = mbedtls_mpi_read_string(&E, 16, bankPubExponent)) != 0) {
    log_error("No pudo leerse la llave RSA. Ret: %d", ret);
    mbedtls_exit(EXIT_FAILURE);
    // TODO: Verificar SHA256 de la llave dada
  }

  if ((ret = mbedtls_rsa_import(&bankRsa, &N, NULL, NULL, NULL, &E)) != 0) {
    log_error("No pudo cargarse la llave RSA. Ret: %d", ret);
    mbedtls_exit(EXIT_FAILURE);
  }
  */
}

/**
 * @brief Libera los recursos utilizados por mbedTLS.
 *
 */
void MbedTLSEnd() {
  /*
  mbedtls_mpi_free(&N);
  mbedtls_mpi_free(&E);
  mbedtls_rsa_free(&bankRsa);

  mbedtls_hmac_drbg_free(&tk_hmac_drbg);
  mbedtls_entropy_free(&tk_entropy);
  mbedtls_hmac_drbg_free(&rsa_hmac_drbg);
  mbedtls_entropy_free(&rsa_entropy);

  mbedtls_memory_buffer_alloc_free();
  */
}

/**
 * @brief Descifra con la llave de transporte actual.
 *
 * @param input
 * @param inputLen
 * @param output
 * @return int 0
 */
int tk_decrypt(const unsigned char *input,
               unsigned int inputLen,
               unsigned char *output) {
                return 0;
}
/*
int tk_decrypt(const unsigned char *input,
               unsigned int inputLen,
               unsigned char *output) {
  int i = 0;
  int ret = 1;
  mbedtls_des3_context tk_context;

  if (inputLen % 8 != 0) {
    log_error("Longitud incorrecta de datos a descifrar");
    return -1;
  }

  mbedtls_des3_init(&tk_context);
  if ((ret = mbedtls_des3_set2key_dec(&tk_context, transportKey)) != 0) {
    log_error("Error al fijar llave TDES. Codigo de error: %d", ret);
    return ret;
  }

  for (i = 0; i < inputLen; i += 8) {
    if ((ret = mbedtls_des3_crypt_ecb(&tk_context, input + i, output + i)) !=
        0) {
      log_error("Error al descifrar con llave TDES. Codigo de error: %d", ret);
      break;
    }
  }

  mbedtls_des3_free(&tk_context);
  return ret;
}
*/

/**
 * @brief Calcula el valor de verificación de la llave de transporte actual.
 *
 * Utiliza el metodo de cifrar un bloque en 0's y tomar los primeros 3 bytes del
 * criptograma resultante.
 *
 * @param output 3 bytes de KCV
 * @return int 0
 */
static int calculate_tk_kcv(unsigned char output[3]) {
  return 0;
}
/*
static int calculate_tk_kcv(unsigned char output[3]) {
  int i = 0;
  int ret = 1;
  mbedtls_des3_context tk_context;
  unsigned char kcvOutput[8];
  const unsigned char kcvCalcInput[8] = {0};

  log_debug("Calculando KCV");

  mbedtls_des3_init(&tk_context);
  if ((ret = mbedtls_des3_set2key_enc(&tk_context, transportKey)) != 0) {
    log_error("Error al fijar llave TDES. Codigo de error: %d", ret);
    return ret;
  }

  if ((ret = mbedtls_des3_crypt_ecb(&tk_context, kcvCalcInput, kcvOutput)) !=
      0) {
    log_error("Error al cifrar con llave TDES. Codigo de error: %d", ret);
    return ret;
  }

  for (i = 0; i < 3; i++) {
    output[i] = kcvOutput[i];
  }

  mbedtls_des3_free(&tk_context);
  return ret;
}
*/

static int generate_random_tk() {
  return 0;
}
/*
static int generate_random_tk() {
  return mbedtls_hmac_drbg_random(&tk_hmac_drbg, transportKey, 16);
}
*/

/**
 * @brief Cifra la llave de transporte con RSA
 *
 */
static int rsa_tk_encrypt_pkcs15() {
  return 0;
}
/*
static int rsa_tk_encrypt_pkcs15() {
  int ret = 1;
  log_debug("Cifrando con RSA");

  ret = mbedtls_rsa_pkcs1_encrypt(
      &bankRsa, mbedtls_hmac_drbg_random, &rsa_hmac_drbg, MBEDTLS_RSA_PUBLIC,
      sizeof(transportKey), transportKey, encryptedTk);
  if (ret != 0) {
    log_error("Cifrado RSA fallido. Ret: %d", ret);
  }

  return ret;
}
*/
int GenerateTransportKey(struct EncryptedTransportKey *key) {
   return 0;
}
/*
int GenerateTransportKey(struct EncryptedTransportKey *key) {
  int i = 0;
  int ret;

  log_debug("Generando llave de transporte");
  if ((ret = generate_random_tk()) == 0) {
    calculate_tk_kcv(key->kcv);
    if ((ret = rsa_tk_encrypt_pkcs15()) == 0) {
      for (i = 0; i < RSA_KEY_SIZE_BYTES; i++) {
        key->encryptedKey[i] = encryptedTk[i];
      }
    }
  }
  return ret;
}
*/

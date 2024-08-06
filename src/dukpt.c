/**
 * @file dukpt.c
 * @author Guillermo Garcia Maynez R. (ggarcia@necsweb.com)
 * @brief
 * @version 0.1
 * @date 2021-05-14
 *
 * @copyright Copyright (c) 2021
 *
 */

#include "apiinc.h"
#include "libapiinc.h"
#include "appinc.h"

#include "config_file.h"
#include "pinpad_commands.h"

#include <log.h>
#include <napi_crypto.h>

int load_ipek(const unsigned char ipek[16], const unsigned char ksn[10]) {
  int ret;
  int i=0;
  ST_SEC_KEYIN_DATA ipekData;
  ST_SEC_KCV_DATA kcvData;

  unsigned char *pAux;

  memset(&ipekData, 0x00, sizeof(ST_SEC_KEYIN_DATA));
  memset(&kcvData, 0x00, sizeof(ST_SEC_KCV_DATA));

  ipekData.ucKeyIdx = 1;
  ipekData.KeyType = KEY_TYPE_DES;
  ipekData.KeyUsage = KEY_USE_DUKPT;
  ipekData.CipherMode = SEC_CIPHER_MODE_ECB;
  ipekData.nKeyLen = 16;
  //ipekData.pKeyData = ipek;
  memset(pAux,0x00,sizeof(pAux));
  for(i=0; i<sizeof(ipek); i++) {
    *(pAux + i) = ipek[i];
  }
  ipekData.pKeyData = pAux;
  ipekData.nKeyDataLen = 16;
  ipekData.nKsnLen = 10;
  //ipekData.psKsn = ksn;
  memset(pAux,0x00,sizeof(pAux));
  for(i=0; i<sizeof(ksn); i++) {
    *(pAux + i) = ksn[i];
  }
  ipekData.psKsn = pAux;

  kcvData.nCheckMode = NAPI_SEC_KCV_NONE;
  kcvData.nLen = 0;

  ret = NAPI_SecGenerateKey(SEC_KIM_CLEAR, &ipekData, &kcvData);
  if (ret != NAPI_OK) {
    log_error("Error al cargar llave DUKPT. Codigo de error: %d", ret);
  }

  return ret;
}

int get_ksn(char *psKSN) {
  int nRet, nKSNLen;
  EM_SEC_CRYPTO_KEY_TYPE KeyType;
  EM_SEC_KEY_USAGE KeyUsage;

  if (psKSN == NULL) {
    return -1;
  }

  KeyType = KEY_TYPE_DES;
  KeyUsage = KEY_USE_DUKPT;
  nRet = NAPI_SecGetKeyInfo(SEC_KEY_INFO_KSN, 1, KeyType, KeyUsage, NULL, 0,
                            (uchar *)psKSN, &nKSNLen);
  if (nRet != NAPI_OK) {
    log_error("NAPI_SecGetDukptKsn error = %d", nRet);
    return -1;
  }

  return 0;
}

int increase_ksn() {
  int ret;
  ST_SEC_KEYIN_DATA dukptData;
  ST_SEC_KCV_DATA kcvData;

  memset(&dukptData, 0x00, sizeof(ST_SEC_KEYIN_DATA));
  memset(&kcvData, 0x00, sizeof(ST_SEC_KCV_DATA));

  dukptData.ucKEKIdx = 1;
  dukptData.ucKeyIdx = 1;
  dukptData.KEKType = KEY_TYPE_DES;
  dukptData.KEKUsage = KEY_USE_DUKPT;
  dukptData.KeyType = KEY_TYPE_DES;
  dukptData.KeyUsage = KEY_USE_DUKPT;
  dukptData.CipherMode = SEC_CIPHER_MODE_ECB;
  kcvData.nCheckMode = NAPI_SEC_KCV_NONE;
  kcvData.nLen = 0;
  ret = NAPI_SecGenerateKey(SEC_KIM_DUKPT_DERIVE, &dukptData, &kcvData);
  if (ret != NAPI_OK) {
    log_error("Error incrementando KSN. Codigo de error: %d", ret);
    return -1;
  }
  return ret;
}

int ipek_exists() {
  log_info("Comprobando existencia de llave DUKPT");
  unsigned char ksn[10] = {0};
  int ksnLen = 0;
  int ret;
  ret = NAPI_SecGetKeyInfo(SEC_KEY_INFO_KSN, 1, KEY_TYPE_DES, KEY_USE_DUKPT,
                           NULL, 0, ksn, &ksnLen);
  if (ret != NAPI_OK) {
    log_warn("No se pudo obtener KSN: %d", ret);
    return 0;
  }
  log_info("La llave existe");
  return 1;
}

int ipek_delete() {
  log_info("Comprobando existencia de llave DUKPT");
  unsigned char ksn[10] = {0};
  int ksnLen = 0;
  int ret;
  ret = NAPI_SecDeleteKey(1, KEY_TYPE_DES, KEY_USE_DUKPT);
  if (ret != NAPI_OK) {
    log_warn("No se pudo borrar la llave DUKPT: %d", ret);
    return 0;
  }
  log_info("La llave ah sido borrada exitosamente");
  return 1;
}

int dukpt_encrypt(const unsigned char *dataIn,
                  unsigned int dataLen,
                  unsigned char *dataOut,
                  unsigned char *ksnOut) {
  int ret = NAPI_ERR;
  int encDataOutLen = 0;
  int encKsnDataLen = 0;
  unsigned char *pAux;
  int i=0;
  
  ST_SEC_ENCRYPTION_DATA encData = {0};

  char auxChipher[7 + 1];

  if (dataLen % 8 != 0) {
    log_error("La longitud a cifrar con DUKPT no es multiplo de 8 bytes");
    return -1;
  }

  encData.ucKeyID = 1;
  encData.CipherType = SEC_CIPHER_DUKPT_ECB_BOTH;
  encData.KeyUsage = KEY_USE_DUKPT;
  encData.PaddingMode = SEC_PADDING_NONE;
  encData.unDataInLen = dataLen;
  //encData.psDataIn = dataIn;
  memset(pAux,0x00,sizeof(pAux));
  for(i=0; i<sizeof(dataIn); i++) {
    *(pAux + i) = dataIn[i];
  }

  ret = NAPI_SecEncryption(&encData, dataOut, &encDataOutLen, ksnOut,
                           &encKsnDataLen);
  if (ret != NAPI_OK) {
    char chFailChipherCounter[7 + 1] = {0};
    int inFailChipherCounter = 0;
    log_error("Error al cifrar con DUKPT. Codigo de error: %d", ret);
    getProperty(FILE_FAILEDCIPHERCOUNTER, chFailChipherCounter);
    inFailChipherCounter = atoi(chFailChipherCounter);
    inFailChipherCounter++;
    sprintf(auxChipher, "%d", inFailChipherCounter);
    setProperty(FILE_REALCIPHERCOUNTER, auxChipher);
    log_error("FILE_REALCIPHERCOUNTER: %s", inFailChipherCounter);

    return -1;
  } else {
    char chRealCounterChipher[7 + 1] = {0};
    int inRealCounterChipher = 0;
    getProperty(FILE_REALCIPHERCOUNTER, chRealCounterChipher);
    inRealCounterChipher = atoi(chRealCounterChipher);
    inRealCounterChipher++;

    if (inRealCounterChipher < 1000000) {
      sprintf(auxChipher, "%d", inRealCounterChipher);
      setProperty(FILE_REALCIPHERCOUNTER, auxChipher);
      log_debug("FILE_REALCIPHERCOUNTER: %d", inRealCounterChipher);

    } else {
      // PRENDEMOS LA BANDERA DE NUEVA LLAVE REQUERIDA
      SetPetitionFlagOfNewKey(KEY_REQUEST_REQUIRED);
      log_error("NECESITA OTRA CARGA DE LLAVES");
    }
  }

  if (encDataOutLen != dataLen) {
    log_error(
        "La longitud de datos cifrados con DUKPT no corresponde con la "
        "longitud de datos de entrada");
    return -2;
  }
  log_trace("Longitud de Datos cifrados: %d", encDataOutLen);
  log_trace("Longitud de KSN: %d", encKsnDataLen);
  increase_ksn();

  return 0;
}

int dukpt_decrypt(const unsigned char *dataIn,
                  unsigned int dataLen,
                  unsigned char *dataOut,
                  unsigned char *ksnOut) {
  int ret = NAPI_ERR;
  int encDataOutLen = 0;
  int encKsnDataLen = 0;
  ST_SEC_ENCRYPTION_DATA encData = {0};

  unsigned char *pAux;
  int i=0;

  if (dataLen % 8 != 0) {
    log_error("La longitud a decifrar con DUKPT no es multiplo de 8 bytes");
    return -1;
  }

  encData.ucKeyID = 1;
  encData.CipherType = SEC_CIPHER_DUKPT_ECB_BOTH;
  encData.KeyUsage = KEY_USE_DUKPT;
  encData.PaddingMode = SEC_PADDING_NONE;
  encData.unDataInLen = dataLen;
  //encData.psDataIn = dataIn;
  memset(pAux,0x00,sizeof(pAux));
  for(i=0; i<sizeof(dataIn); i++) {
    *(pAux + i) = dataIn[i];
  }


  ret = NAPI_SecDecryption(&encData, dataOut, &encDataOutLen, ksnOut,
                           &encKsnDataLen);
  if (ret != NAPI_OK) {
    log_error("Error al descifrar con DUKPT. Codigo de error: %d", ret);
    return -1;
  }

  if (encDataOutLen != dataLen) {
    log_error(
        "La longitud de datos cifrados con DUKPT no corresponde con la "
        "longitud de datos de entrada");
    return -2;
  }
  log_trace("Longitud de buffer: %d", encKsnDataLen);
  log_trace("Longitud de KSN: %d", encKsnDataLen);
  increase_ksn();

  return 0;
}

int assemble_sensitiveDataBlock(char chTk1[],
                                char chTk2[],
                                char chCv2[],
                                char chPan[],
                                char chExpDate[],
                                unsigned char currentKsn[],
                                unsigned char chB1[],
                                unsigned char chB2[]) {
  unsigned char encryptedCpBlockInBytes[104 + 1] = {0};

  char track2Padeado[48 + 1] = {0};
  char cvv2Padeado[10 + 1] = {0};
  int x;
  int lenCPB = 0;
  char track1InASCII[160 + 1] = {0};
  char track1Padeado[160 + 1] = {0};
  char completeBlock[208 + 1] = {0};
  char completeBlockBytes[104 + 1] = {0};

  memset(track2Padeado, 0x00, 48 + 1);

  // BLOQUE DE TRACK 2 + CVV
  if (strlen(chTk2) > 0) {
    char *pch;
    // Quita el caracter "=" del Track 2 si es que tiene y lo remplaza con
    // una D
    pch = strchr(chTk2, '=');
    if (pch != NULL) {
      *pch = 'D';
    }

    log_trace("Track 2 remplazado '=' con 'D': %s", chTk2);
  } else {
    // En caso de no tener Track 2 toma el PAN (digitadas)
    char track2Digitada[24] = {0};

    /*memcpy(stSystemSafe.szTrack2, stSystemSafe.szPan,
     strlen(stSystemSafe.szPan));*/
    memcpy(track2Digitada, chPan, strlen(chPan));
    if (strlen(chExpDate) > 0) {
      track2Digitada[strlen(chPan)] = 'D';
      memcpy(track2Digitada + (strlen(chPan) + 1), chExpDate,
             strlen(chExpDate));
    } else {
      track2Digitada[strlen(chPan)] = 'D';
      memcpy(track2Digitada + (strlen(chPan) + 1), "0000", 4);
    }
    memcpy(chTk2, track2Digitada, strlen(track2Digitada));
    log_trace("Track 2 para digitada: %s", chTk2);
  }

  if (strlen(chCv2) > 0) {
    log_trace("CON CVV");

    for (x = 0; x < 38; x++) {
      track2Padeado[x] = 'F';
    }
    memcpy(track2Padeado, chTk2, strlen(chTk2));
    log_trace("Bloque track2 Padeado: %s", track2Padeado);

    for (x = 0; x < 10; x++) {
      cvv2Padeado[x] = 'F';
    }
    memcpy(cvv2Padeado, chCv2, strlen(chCv2));
    log_trace("Bloque cvv2 Padeado: %s", cvv2Padeado);
    strcat(track2Padeado, cvv2Padeado);
  } else {
    log_trace("SIN CVV");

    for (x = 0; x < 48; x++) {
      track2Padeado[x] = 'F';
    }
    memcpy(track2Padeado, chTk2, strlen(chTk2));
    log_trace("Bloque track2 Padeado: %s", track2Padeado);
  }

  // BLOQUE DE TRACK 1
  memset(track1Padeado, 0x00, 160 + 1 + 1);
  if (strlen(chTk1) > 0) {
    for (x = 0; x < 160; x++) {
      track1Padeado[x] = 'F';
    }
    PubHexToAsc(chTk1, strlen(chTk1) * 2, 0, track1InASCII);
    memcpy(track1Padeado, track1InASCII, strlen(track1InASCII));
  } else {
    for (x = 0; x < 160; x++) {
      track1Padeado[x] = 'F';
    }
  }

  log_trace("Bloque track1 Padeado: %s, len : %d", track1Padeado,
            strlen(track1Padeado));

  strcat(completeBlock, track2Padeado);
  strcat(completeBlock, track1Padeado);

  log_trace("Bloque completo: %s", completeBlock);

  lenCPB = inAscToHex(completeBlock, completeBlockBytes, strlen(completeBlock));

  log_trace("Longitud de Bloque de Datos sensitivos: %d ", lenCPB);
  log_trace("Cifrando Bloque Comleto de Datos Sensitivos...");

  dukpt_encrypt(completeBlockBytes, lenCPB, encryptedCpBlockInBytes,
                currentKsn);

  memcpy(chB1, encryptedCpBlockInBytes, 24);
  memcpy(chB2, encryptedCpBlockInBytes + 24, 80);
}

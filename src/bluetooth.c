/***************************************************************************
* @copyright (c) 2019 Newland Payment Technology Co., Ltd All right reserved
* @file 	main.c
* @brief 	Simple message exchanging on ME51 via bluetooth(SSP mode)
                        Pls call 'BluetoothOpen' on your APP.
* @version  1.0
* @author	Hobart
* @date 	2020-10-22
* @revision history
***************************************************************************/
#include "log.h"

#include "comm_channel.h"
#include "napi.h"
#include "napi_bt.h"
#include "napi_display.h"
#include "napi_kb.h"
#include "napi_sysinfo.h"

#include <debug.h>
#include <lui.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BT_NAME_PREFIX "BBVA"

#define MIN_BT_PIN_LENGTH 4
#define MAX_BT_PIN_LENGTH 16
#define BT_PIN_TIMEOUT 20
#define BT_RECV_TIMEOUT 1000

int BluetoothDataExchange();
int BluetoothOpen();


/*
*/
int NAPI_BTSend_fake(int l, const unsigned char *cont)
{
  return 0;
}
int NAPI_BTRecive_fake(unsigned int buffToRead, const unsigned char *outBuff, unsigned int timeout, unsigned int *outLen)
{
  return 0;
}
int NAPI_BTReadLen_fake(unsigned int *lll)
{
  return 0;
}
static void EnableDispDefault_fake(void)
{

}




static int ComputeBluetoothName(char *btName) {
  int retVal;
  char serialNumber[32] = {0};
  int buffLength;

  retVal = NAPI_SysGetInfo(SN, serialNumber, &buffLength);
  if (retVal == NAPI_OK) {
    char snLastFour[4 + 1] = {0};
    strncpy(snLastFour, serialNumber + buffLength - 4, 4);
    sprintf(btName, "Terminal%s%s", BT_NAME_PREFIX, snLastFour);
    return 0;
  }
  log_error("Error al obtener numero de serie del dispositivo");
  return retVal;
}

static int BTWrite(const unsigned char *content, unsigned int len) {
  return NAPI_BTSend_fake((size_t)len, content);
}


// 2024073
//static int BTRead(const unsigned char *outBuff,
//                  unsigned int *outLen,
//                  unsigned int timeout) {
//  unsigned int buffToRead = 0;
//  NAPI_BTReadLen_fake(&buffToRead);
//  return NAPI_BTRecive_fake(buffToRead, outBuff, timeout, outLen);
//}

static int BTBuffCheck(unsigned int *outLen) { return NAPI_BTReadLen_fake(outLen); }

static int BTFlush() { return NAPI_BTFlush(); }

int BluetoothSetup() {
  int ret;
  char bluetoothName[32] = {0};

  NAPI_BTReset();

  log_info("Activando Bluetooth...");

  SetBuffCheckFor(kBluetooth, &BTBuffCheck);
  SetBuffFlushFor(kBluetooth, &BTFlush);
  SetWriteFor(kBluetooth, &BTWrite);
//  SetReadFor(kBluetooth, &BTRead); // 20240737

  // Open BT
  ret = NAPI_BTPortOpen(true);
  if (ret != NAPI_OK) {
    log_error("Error al abrir puerto Bluetooth. Codigo de error: %d", ret);
    return -1;
  } else {
    log_trace("Puerto Bluetooth abierto exitosamente");
  }

  ret = ComputeBluetoothName(bluetoothName);
  if (ret != NAPI_OK) {
    log_error("Error al calcular nombre de dispositivo. Codigo de error: %d",
              ret);
    // TODO: Administrar error
    return -1;
  }

  ret = NAPI_BTSetLocalName(bluetoothName);
  if (ret != NAPI_OK) {
    log_error("Error al fijar nombre de dispositivo. Codigo de error: %d", ret);
    // TODO: Administrar error
    return -1;
  }

  ret = NAPI_BTGetLocalName(bluetoothName);
  if (ret != NAPI_OK) {
    log_error(
        "Error al obtener nombre propio de Bluetooth. Codigo de error: %d",
        ret);
    // TODO: Administrar error
    return -1;
  } else {
    log_info("Nombre de este dispositivo: %s", bluetoothName);
  }

  /* El modo de vinculación PASSKEY es el más seguro, pues forza al usuario a
   * verificar que el código de emparejamiento coincida con el del teléfono. No
   * debería usarse ningún otro modo en PIN pads Bluetooth, pues un error humano
   * podría vulnerar las comunicaciones. */
  ret = NAPI_BTSetPairingMode(PAIRING_MODE_PASSKEY);
  if (ret != NAPI_OK) {
    log_error("Error al fijar modo de emparejamiento. Codigo de error: %d",
              ret);
    // TODO: Administrar error
  } else {
    log_debug("Modo de emparejamiento fijado a passkey");
    log_info("Esperando emparejamiento BT...");
  }

  return NAPI_OK;
}

int BluetoothCheckPairingStatus() {
  int pairingStatus = -1, ret;
  char pairingCode[16] = {0};
  int sucflag = 0;
  //unsigned short pCodeLen = 0;
  int pCodeLen = 0;
  int status;

  ret = NAPI_BTGetPairingStatus(pairingCode, &pairingStatus);
  if (ret != NAPI_OK) {
    log_error("Error al obtener estado de emparejamiento. Codigo de error: %d",
              ret);
    status = NAPI_ERR;
  }

  switch (pairingStatus) {
    case 1:
      log_debug("Recibida solicitud de emparejamiento.");
      NAPI_ScrClrs();
      PubInputDlg("Vinculacion segura", "Codigo:", pairingCode, &pCodeLen,
                  MIN_BT_PIN_LENGTH, MAX_BT_PIN_LENGTH, BT_PIN_TIMEOUT,
                  INPUT_MODE_NUMBER);
      ret = NAPI_BTConfirmPairing(pairingCode, 1);
      memset(pairingCode, 0x00, pCodeLen);
      if (ret == NAPI_OK) {
        log_warn("Confirmacion BT exitosa: %d", ret);
        PubSysDelay(15);
        ret = NAPI_BTGetPairingStatus(pairingCode, &pairingStatus);
        EnableDispDefault_fake();
        sucflag = 1;
        status = NAPI_OK;
      } else {
        log_warn("Error al confirmar emparejamiento. Codigo de error: %d", ret);
        status = NAPI_ERR;
      }
      break;
    case 2:
      sucflag = 1;
      break;
    case 3:
      log_error("Error al confirmar emparejamiento. Codigo de error: %d", ret);
      SetOpenForChannel(kBluetooth, false);
      status = NAPI_ERR;
      break;
    default:
      status = NAPI_ERR_BT_NOT_CONNECTED;
      break;
  }
  if (sucflag == 1) {
    log_info("Emparejado exitosamente");
    SetOpenForChannel(kBluetooth, true);
    status = NAPI_OK;
  }
  return status;
}

// Esta funcion va a desaparecer. Esta siendo refactorizada en funciones mas
// pequeñas.
int BluetoothOpen() {
  int ret, nRet;
  int key, status = -1;

  log_trace("Esperando conexion");
  while (1) {
    NAPI_KbHit(&key);
    if (key == K_ESC) goto err;

    ret = NAPI_BTStatus(&status);
    if (ret != NAPI_OK) {
      log_error("Error al obtener estado de conexion. Codigo de error: %d",
                ret);
      NAPI_ScrClrs();
      NAPI_ScrPrintf("Line:: [%d][ret=%d][status=%d]\n", __LINE__, ret, status);
      NAPI_ScrPrintf("Get Bluetooth Status Fail\n");
      NAPI_ScrRefresh();
      NAPI_KbGetCode(10, &nRet);
      goto err;
    }
    if (status == 0) {
      log_debug("Conectado");
      break;
    } else if (status == 1) {
      // fprintf(stderr, "===unconnect\n");
    }
    usleep(1000 * 1000);
  }

#if 0
    NAPI_ScrClrs();
    NAPI_ScrPrintf("Line:: [%d]\n", __LINE__);
    NAPI_ScrRefresh();
    NAPI_KbGetCode(10, &nRet);
#endif
  // BluetoothDataExchange();
err:
  log_trace("Cerrando Bluetooth por error descrito anteriormente");
  NAPI_BTPortOpen(0);
  return 0;
}

// Data exchange Test
int BluetoothDataExchange() {
  int nRet;
  int rlen, key, len, status = -1;
  char readBuf[256] = {0};
  char writeBuf[256] = {0};

  while (1) {
    int ret;
    NAPI_ScrClrs();
    NAPI_ScrPrintf("Receiving Data...");
    NAPI_ScrRefresh();
    NAPI_KbHit(&key);
    if (key == K_ESC) return 0;

    ret = NAPI_BTStatus(&status);
    if (ret != NAPI_OK) {
      NAPI_ScrClrs();
      NAPI_ScrPrintf("Get Bluetooth Status Fail");
      NAPI_ScrRefresh();
      NAPI_KbGetCode(10, &nRet);
      return -1;
    }

    memset(readBuf, 0x00, sizeof(readBuf));
    NAPI_BTReadLen(&len);
    if (len > sizeof(readBuf) - 1) {
      len = sizeof(readBuf) - 1;
    }
    if (len <= 0) {
      usleep(100 * 1000);
      continue;
    }

    ret = NAPI_BTRecive(len, readBuf, BT_RECV_TIMEOUT, &rlen);
    if (ret == NAPI_OK) {
      NAPI_ScrClrs();
      NAPI_ScrPrintf("\nRecived Data:%s", readBuf);
      NAPI_ScrRefresh();
      NAPI_KbGetCode(1, &nRet);

      NAPI_BTFlush();

      memset(writeBuf, 0, sizeof(writeBuf));
      sprintf(writeBuf, "POS send same data:[%s]", readBuf);
      ret = NAPI_BTSend(sizeof(writeBuf), writeBuf);
      if (ret != NAPI_OK) {
        NAPI_ScrClrs();
        NAPI_ScrPrintf("Write Data Fail\n");
        NAPI_ScrRefresh();
        NAPI_KbGetCode(10, &nRet);
        return -1;
      }
    }
  }

  return 0;
}

/*
 * @Author: Diego Escalera
 * @Date:   28/01/2021
 */

/*ParadoX Technologies
 */

#include "pinpad_commands.h"

#include "log.h"

#include "libapiinc.h"

#include "apiinc.h"
#include "appinc.h"
#include "bines_table.h"
#include "comm.h"
#include "comm_channel.h"
#include "config_file.h"
#include "debug.h"
#include "lcomm.h"
#include "lfileSystem.h"
#include "ota_update.h"
#include "port.h"
#include "softalg.h"
#include "tool.h"

#include <tk.h>
#include <dukpt.h>
#include <ew_token.h>
#include <ex_token.h>
#include <ltool.h>
#include <utils.h>

#include "string.h"
#include <stdint.h>
#include <stdlib.h>

void EnableDispDefault(void);
static enum CommChannel pinpadChannel = kBluetooth;
static char flagNewKey = KEY_REQUEST_REQUIRED;

extern int lockGuard;
int isC50Present = 0;
int lenTelecarga = 0;
char panC50[20] = {0};
char entryModeC50;
char amountC50[12 + 1] = {0};
int gbo_ipk_crd;
int logitudListaTags;
#define MAX_DATA_CMD 2048
unsigned char script71[1024] = {0};
unsigned char script72[1024] = {0};
int lenScripts71 = 0;
//extern int lenScripts72 = 0;
int lenScripts72 = 0;
int firstScriptOfTwo = 0;
// Data for Transactional Commands
TxnCommonData txnCommonData;

void SetPinPadChannel(enum CommChannel channel) { pinpadChannel = channel; }
void SetPetitionFlagOfNewKey(char flag) { flagNewKey = flag; }

int GetPinPadChannel() { return pinpadChannel; }
char GetPetitionFlagOfNewKey() { return flagNewKey; }

int CheckLowBattery() {
  int nPersent = 0;
  // SI TENEMOS MENOS DEL 5% REGRESAMOS ERROR
  NAPI_SysGetVolPercent(&nPersent);
  if (nPersent <= 13 && nPersent > 0) {
    PubBeep(1);
    DispOutICC(" ", "   BATERIA MUY BAJA  ", "  CARGAR DISPOSITIVO ");
    return RET_CMD_FAIL;
  }
  return RET_CMD_OK;
}

static int CheckNeedOfNewKeyForTransaction() {
  // NO SE PUEDE TRANSACCIONAR SI SE NECESITA UNA CARGA DE LLAVES NUEVA
  if (GetPetitionFlagOfNewKey() == KEY_REQUEST_REQUIRED) {
    log_debug("NECESITA UNA CARGA DE LLAVES");
    char chRealCounterChipher[7 + 1] = {0};

    getProperty(FILE_REALCIPHERCOUNTER, chRealCounterChipher);
    PubBeep(1);
    PubClearAll();
    if (atoi(chRealCounterChipher) >= 1000000) {
      PubDisplayStr(DISPLAY_MODE_CENTER, 2, 1, "LIM CIF ECXCEDIDO");
      PubDisplayStr(DISPLAY_MODE_CENTER, 3, 1, "INICIALICE LLAVE");
    } else {
      PubDisplayStr(DISPLAY_MODE_CENTER, 2, 1, "NO SE HA REALIZADO");
      PubDisplayStr(DISPLAY_MODE_CENTER, 3, 1, "INIC DE LLAVES");
    }

    PubUpdateWindow();
    PubSysDelay(20);
    NAPI_L3TerminateTransaction();
    DispOutICC("", "", "");

    return APP_FAIL;
  }
  log_debug("NO NECESITA UNA CARGA DE LLAVES");

  return APP_SUCC;
}

static void ParseFlagTransType(int firstNibble, TxnCommonData *txnCommonData) {
  switch (firstNibble) {
    case 0:
      txnCommonData->inCvvReq = 1;
      txnCommonData->inExpDateReq = 1;
      break;
    case 1:
      txnCommonData->inCvvReq = 0;
      txnCommonData->inExpDateReq = 1;
      break;
    case 2:
      txnCommonData->inCvvReq = 1;
      txnCommonData->inExpDateReq = 0;
      break;
    case 3:
      txnCommonData->inCvvReq = 0;
      txnCommonData->inExpDateReq = 0;
      break;
    default:
      txnCommonData->inCvvReq = 1;
      txnCommonData->inExpDateReq = 1;
  }
}

/*---------------------------------------------------*/
// 1 PROCESS COMAND ASYNC
/*---------------------------------------------------*/

int inProcessComand(void) {
  static char chCmdRx[MAX_DATA_CMD] = {0};
  static char chCmdTx[MAX_DATA_CMD] = {0};
  static int inCmdRx = 0;
  static int inCmdTx = 0;
  static int inRet;
  static int bad_lrc = 0;
  static int snd_retry = 0;

  unsigned int channelBufLen = 0;

  // SetControlCommInit();
  // PubCommConnect();

  memset(chCmdRx, 0, sizeof chCmdRx);

  // inRet = PubCommRead(chCmdRx, sizeof chCmdRx, &inCmdRx);
  CheckBuffFromChannel(pinpadChannel, &channelBufLen);

  if (channelBufLen > 0) {
    int i=0;
    log_trace("Buffer con %u nuevos bytes a ser leidos", channelBufLen);

    usleep(1000);
    inCmdRx = MAX_DATA_CMD;
    inRet = ReadFromChannel(pinpadChannel, chCmdRx, &inCmdRx, 100);

    // FIXME: Algo extraño sucede con USB serial, consultar con Newland
    if (pinpadChannel == kUSB) {
      if (inRet == -10) {
        inRet = 0;
      }
    }

    log_debug("Lectura de %d bytes con codigo %d", inCmdRx, inRet);
    for (i = 0; i < inCmdRx; i++) {
      log_trace("Byte %d: %02X", i, chCmdRx[i]);
    }
  } else {
    return RET_CMD_OK;
  }

  if (inRet == RET_CMD_OK) {
    log_info("Iniciando procesamiento del comando");

    // PubCommClear();
    FlushChannel(pinpadChannel);
    // NAPI_ScrClrs();

    pdump(chCmdRx, inCmdRx, "chCmdRx: ");
    if (inCmdRx == 1) {
      if (chCmdRx[0] == ACK) {
      } else if (chCmdRx[0] == ENQ) {
        inSendByte(ACK);
      } else if (chCmdRx[0] == NAK) {
        if (snd_retry > 0 && snd_retry < 3) {
          // if (PubCommWrite(chCmdTx, inCmdTx) == RET_CMD_FAIL)
          if (WriteToChannel(pinpadChannel, chCmdTx, inCmdTx) == RET_CMD_FAIL) {
            PubMsgDlg("ERROR COMM", tr("SEND FAIL"), 3, 3);
            log_debug(
                "ERROR DE LECTURA 3 "
                "VECEZ------------------------------------------------>>>>>>>"
                ">>"
                ">");
            // PubCommHangUp();
            // TODO: Añadir cierre de puerto
          }
          snd_retry++;
        } else {
          inSendByte(EOT);
        }
      }
      return 22;
    } else {
      snd_retry = 0;
      if (inProcessCommandAsyn(chCmdRx, &inCmdRx) == RET_CMD_FAIL) {
        bad_lrc++;
        if (bad_lrc == 4) {
          bad_lrc = 0;
          inSendByte(EOT);
        } else {
          inSendByte(NAK);
        }
      } else {
        inSendByte(ACK);

        memset(chCmdTx, 0, sizeof chCmdTx);
        inRet = inParseProccessComand(chCmdRx, inCmdRx, chCmdTx, &inCmdTx);

        if (inRet == RET_CMD_OK) {
          if (inPrepareCommandAsyn(chCmdTx, &inCmdTx) == RET_CMD_OK) {
            // if (PubCommWrite(chCmdTx, inCmdTx) == RET_CMD_FAIL)
            log_debug("LONGITUD DE EL COMANDO: %d", inCmdTx);

            inRet = WriteToChannel(pinpadChannel, chCmdTx, inCmdTx);

            if (inRet == NAPI_OK) {
              log_trace("ENVIO DE RESPUESTA EXITOSO");
            } else {
              log_trace("ENVIO DE RESPUESTA FALLIDO");
              PubClearAll();
              PubDisplayStr(DISPLAY_MODE_CENTER, 2, 1, "ERROR COMM [%d]",
                            inRet);
              PubDisplayStr(DISPLAY_MODE_CENTER, 3, 1, "PUERTO [%d]",
                            pinpadChannel);
              PubSysDelay(15);
              lockGuard = 0;
              isC50Present = 0;
              EnableDispDefault();
              NAPI_L3TerminateTransaction();
            }
            snd_retry++;
          }
          return 22;
        }
      }
    }
  }

  return RET_CMD_OK;
}

// Daee 28/01/2021  /*copy modified of  PinPad_DeleteAsyn --> pinpad.c*/
int inProcessCommandAsyn(char *psBuf, int *punLen) {
  int nLen = 0;
  char cLRC = 0;
  char sTemp[MAX_DATA_CMD * 2 + 5] = {0};

  log_trace("inProcessCommandAsyn");

  pdump(psBuf, *punLen, "psBuf: ");

  if (NULL == psBuf || NULL == punLen) {
    return RET_CMD_FAIL;
  }

  if (psBuf[0] != STX) {
    return RET_CMD_FAIL;
  }

  if (psBuf[*punLen - 2] != ETX) {
    return RET_CMD_FAIL;
  }

  if (PubCalcLRC(psBuf + 1, *punLen - 2, &cLRC) != RET_CMD_OK) {
    return RET_CMD_FAIL;
  }

  log_trace("inProcessCommandAsyn cLRC=%x", cLRC);

  if (*(psBuf + *punLen - 1) != cLRC) {
    return RET_CMD_FAIL;
  }

  nLen = *punLen - 3;

  memcpy(sTemp, psBuf + 1, nLen);
  memset(psBuf, 0x00, *punLen);
  memcpy(psBuf, sTemp, nLen);
  *punLen = nLen;
  return RET_CMD_OK;
}

// Daee 28/01/2021  /*copy modified of  PinPad_InsertAsyn --> pinpad.c*/
int inPrepareCommandAsyn(char *psBuf, int *punLen) {
  char sTemp[MAX_DATA_CMD * 2 + 10] = {0};

  log_trace("inPrepareCommandAsyn");

  pdump(psBuf, *punLen, "psBuf: ");

  if (NULL == psBuf || NULL == punLen) {
    return RET_CMD_FAIL;
  }

  memcpy(sTemp, psBuf, *punLen);
  memset(psBuf, 0x00, *punLen + 3);
  psBuf[0] = STX;
  memcpy(psBuf + 1, sTemp, *punLen);
  psBuf[*punLen + 1] = ETX;
  PubCalcLRC(psBuf + 1, *punLen + 1, psBuf + *punLen + 2);
  *punLen += 3;
  return RET_CMD_OK;
}

int inSendByte(char byte) {
  char chCmdRx[10] = {0};
  chCmdRx[0] = byte;
  log_trace("inSendByte byte[%x]", chCmdRx[0]);
  // if (PubCommWrite(chCmdRx, 1) == RET_CMD_FAIL)
  if (WriteToChannel(pinpadChannel, chCmdRx, 1) == RET_CMD_FAIL) {
    log_trace(
        "ERROR AL ENVIAR BYTE: %02x "
        "COMANDO------------------------------------------------>>>>>"
        ">>>>>",
        byte);
    PubMsgDlg("ERROR COMM", tr("SEND FAIL"), 3, 3);
    // TODO: Cierre de puerto
    // PubCommHangUp();
  };
}

/*---------------------------------------------------*/
// 1 PROCESS COMAND APP
/*---------------------------------------------------*/

int inParseProccessComand(char *chCmdRx,
                          int inCmdRx,
                          char *chCmdTx,
                          int *inCmdTx) {
  int inRet = RET_CMD_FAIL;

  log_trace("inParseProccessComand");

  pdump(chCmdRx, inCmdRx, "chCmdRx: ");

  switch (chCmdRx[0]) {
    case 'C':

      //	C25 SAVE SCRIPT
      if (!memcmp(chCmdRx, "C25", 3)) {
        *inCmdTx = inC25(chCmdTx, MAX_DATA_CMD, chCmdRx, inCmdRx);

        if (*inCmdTx > RET_CMD_OK) {
          inRet = RET_CMD_OK;
        }
      }

      //	C50 BEGIN TRANSACTION
      if (!memcmp(chCmdRx, "C50", 3)) {
        NAPI_ScrClrs();
        *inCmdTx = inC50(chCmdTx, MAX_DATA_CMD, chCmdRx, inCmdRx);

        if (*inCmdTx > RET_CMD_OK) {
          inRet = RET_CMD_OK;
        }
      }

      //	C51 TRANSACTION
      if (!memcmp(chCmdRx, "C51", 3)) {
        NAPI_ScrClrs();
        *inCmdTx = inC51(chCmdTx, MAX_DATA_CMD, chCmdRx, inCmdRx);

        if (*inCmdTx > RET_CMD_OK) {
          inRet = RET_CMD_OK;
        }
      }

      //	C54 COMPLETE TRANSACTION
      if (!memcmp(chCmdRx, "C54", 3)) {
        NAPI_ScrClrs();
        *inCmdTx = inC54(chCmdTx, MAX_DATA_CMD, chCmdRx, inCmdRx);

        if (*inCmdTx > RET_CMD_OK) {
          inRet = RET_CMD_OK;
        }
      }

      //	C14 TABLA DE BINES
      if (!memcmp(chCmdRx, "C14", 3)) {
        NAPI_ScrClrs();
        *inCmdTx = inC14(chCmdTx, MAX_DATA_CMD, chCmdRx, inCmdRx);

        if (*inCmdTx > RET_CMD_OK) {
          inRet = RET_CMD_OK;
        }
      }

      //	C12 EJECUTAR SCRIPT
      if (!memcmp(chCmdRx, "C12", 3)) {
        *inCmdTx = inC12(chCmdTx, MAX_DATA_CMD, chCmdRx, inCmdRx);

        if (*inCmdTx > RET_CMD_OK) {
          inRet = RET_CMD_OK;
        }
      }

      break;
    case 'I':
      //	I02 REMOVE CARD
      if (!memcmp(chCmdRx, "I02", 3)) {
        *inCmdTx = inI02(chCmdTx, MAX_DATA_CMD, chCmdRx, inCmdRx);

        if (*inCmdTx > RET_CMD_OK) {
          inRet = RET_CMD_OK;
        }
      }
      break;

    case 'Q':
      if (!memcmp(chCmdRx, "Q8", 2)) {
        *inCmdTx = inQ8(chCmdTx, MAX_DATA_CMD, (char *)chCmdRx, inCmdRx);

        if (*inCmdTx > RET_CMD_OK) {
          inRet = RET_CMD_OK;
        }
      }

// Q7 pendiente
/*
      if (!memcmp(chCmdRx, "Q7", 2)) {
        *inCmdTx = inQ7(chCmdTx, MAX_DATA_CMD, (char *)chCmdRx, inCmdRx);

        if (*inCmdTx > RET_CMD_OK) {
          inRet = RET_CMD_OK;
        }
      }
*/

      break;

    case 'Z':
      // Z2 DSP MESSAGE
      if (!memcmp(chCmdRx, "Z2", 2)) {
        *inCmdTx =
            inZ2(chCmdTx, MAX_DATA_CMD, (char *)chCmdRx + 2, inCmdRx - 2);

        if (*inCmdTx > RET_CMD_OK) {
          inRet = RET_CMD_OK;
        }
      }

      if (!memcmp(chCmdRx, "Z3", 2)) {
        *inCmdTx =
            inZ3(chCmdTx, MAX_DATA_CMD, (char *)chCmdRx + 2, inCmdRx - 2);

        if (*inCmdTx > RET_CMD_OK) {
          inRet = RET_CMD_OK;
        }
      }

      if (!memcmp(chCmdRx, "Z10", 3)) {
        NAPI_ScrClrs();
        *inCmdTx = inZ10(chCmdTx, MAX_DATA_CMD, (char *)chCmdRx, inCmdRx);

        if (*inCmdTx > RET_CMD_OK) {
          inRet = RET_CMD_OK;
        }
      }

      if (!memcmp(chCmdRx, "Z11", 3)) {
        NAPI_ScrClrs();
        *inCmdTx = inZ11(chCmdTx, MAX_DATA_CMD, (char *)chCmdRx, inCmdRx);

        if (*inCmdTx > RET_CMD_OK) {
          inRet = RET_CMD_OK;
        }
      }

      break;

    default:
      break;
  }

  pdump(chCmdTx, *inCmdTx, "chCmdTx: ");

  return inRet;
}

/*---------------------------------------------------*/
// 1 COMANDs APP
/*---------------------------------------------------*/

int inC50(char pch_snd[], int in_max, char pch_rcv[], int in_rcv) {
  int in_val = RET_CMD_OK;

  pdump(pch_rcv, in_rcv, "rcv");

  memset(&txnCommonData, 0x00, sizeof txnCommonData);

  in_val = inC50Disassemble(pch_rcv, in_rcv, &txnCommonData);

  /*if (CheckNeedOfNewKeyForTransaction() == APP_FAIL) {
    isC50Present = 0;
    txnCommonData.status = kKeyInitNotPerformed;
    in_val = kKeyInitNotPerformed;
    lockGuard = 0;
    EnableDispDefault();
  }*/

  if (in_val == RET_CMD_OK) {
    char auxTransType = 0x00;
    switch (txnCommonData.inTranType) {
      case GenericTransExclAmex:
        auxTransType = TRANS_SALE;
        break;
      case RefundExclAmex:
        auxTransType = TRANS_REFUND;
        break;
      case GenericTransInclAmex:
        auxTransType = TRANS_SALE;
        break;
      case RefundInclAmex:
        auxTransType = TRANS_REFUND;
        break;
      case QpsSaleExclAmex:
        auxTransType = TRANS_QPS;
        break;
      case GenericTransCtlss: {
        if (txnCommonData.inFlagTransType == 8) {
          auxTransType = TRANS_QPS;
        } else {
          auxTransType = TRANS_SALE;
        }
      } break;
      default:
        auxTransType = TRANS_SALE;
        break;
    }

    lockGuard = 1;
    in_val = TxnCommonEntryPinpad(auxTransType + '0', &txnCommonData);

    if (txnCommonData.status == RET_CMD_OK ||
        txnCommonData.status == L3_TXN_APPROVED ||
        txnCommonData.status == L3_TXN_ONLINE) {
      PubClearAll();
      PubDisplayTitle(tr(" "));
      PubDisplayStr(DISPLAY_MODE_CENTER, 3, 1, tr("PROCESSING..."));
      PubUpdateWindow();
    } else if (txnCommonData.status == L3_TXN_DECLINE) {
      DispOutICC("", "DECLINADA  EMV", "");
      lockGuard = 0;
      EnableDispDefault();
    } else {
      lockGuard = 0;
      EnableDispDefault();
    }

  } else {
    txnCommonData.status = in_val;
    lockGuard = 0;
    EnableDispDefault();
  }

  in_val = inC50Assemble(pch_snd, in_max, &txnCommonData);

  return in_val;
}

int inC51(char pch_snd[], int in_max, char pch_rcv[], int in_rcv) {
  int in_val = RET_CMD_OK;
  int c51Fail = 0;

  log_trace("inC51");

  pdump(pch_rcv, in_rcv, "rcv");

  memset(&txnCommonData, 0x00, sizeof txnCommonData);

  in_val = inC51Disassemble(pch_rcv, in_rcv, &txnCommonData);

  // NO SE PUEDE TRANSACCIONAR SI SE NECESITA UNA CARGA DE LLAVES NUEVA
  /*if (CheckNeedOfNewKeyForTransaction() == APP_FAIL) {
    isC50Present = 0;
    txnCommonData.status = kKeyInitNotPerformed;
    in_val = kKeyInitNotPerformed;
    c51Fail = 1;
    lockGuard = 0;
    EnableDispDefault();
  }*/

  if (in_val == RET_CMD_OK) {
    char auxTransType = 0x00;
    switch (txnCommonData.inTranType) {
      case GenericTransExclAmex:
        auxTransType = TRANS_SALE;
        break;
      case RefundExclAmex:
        auxTransType = TRANS_REFUND;
        break;
      case GenericTransInclAmex:
        auxTransType = TRANS_SALE;
        break;
      case RefundInclAmex:
        auxTransType = TRANS_REFUND;
        break;
      case QpsSaleExclAmex:
        auxTransType = TRANS_QPS;
        break;
      case GenericTransCtlss: {
        if (txnCommonData.inFlagTransType == 8) {
          auxTransType = TRANS_QPS;
        } else {
          auxTransType = TRANS_SALE;
        }
      } break;
      default:
        auxTransType = TRANS_SALE;
        break;
    }

    lockGuard = 1;
    in_val = TxnCommonEntryPinpad(auxTransType + '0', &txnCommonData);

    if (txnCommonData.status == RET_CMD_OK ||
        txnCommonData.status == L3_TXN_DECLINE ||
        txnCommonData.status == L3_TXN_APPROVED ||
        txnCommonData.status == L3_TXN_ONLINE ||
        txnCommonData.status == kCtlss ||
        txnCommonData.status == kUseMagStripe ||
        txnCommonData.status == kManualEntry) {
      PubClearAll();
      PubDisplayTitle(tr(" "));
      PubDisplayStr(DISPLAY_MODE_CENTER, 3, 1, tr("PROCESSING..."));
      PubUpdateWindow();

    } else {
      c51Fail = 1;
      lockGuard = 0;
      EnableDispDefault();
    }

  } else {
    txnCommonData.status = in_val;
    lockGuard = 0;
    EnableDispDefault();
  }

  if (c51Fail == 1) {
    in_val = inC54Assemble(pch_snd, in_max, &txnCommonData);
  } else {
    in_val = inC53Assemble(pch_snd, in_max, &txnCommonData);
  }

  return in_val;
}

int inC54(char pch_snd[], int in_max, char pch_rcv[], int in_rcv) {
  int in_val = RET_CMD_OK;

  log_trace("inC54");

  pdump(pch_rcv, in_rcv, "rcv");

  in_val = inC54Disassemble(pch_rcv, in_rcv, &txnCommonData);

  if (in_val == RET_CMD_OK) {
    in_val =
        TxnCommonEntryPinpad(txnCommonData.inTranType + '0', &txnCommonData);

  } else {
    txnCommonData.status = in_val;
  }

  lockGuard = 0;
  EnableDispDefault();
  in_val = inC54Assemble(pch_snd, in_max, &txnCommonData);

  return in_val;
}

int inC25(char pch_snd[], int in_max, char pch_rcv[], int in_rcv) {
  int in_val = 0;

  log_trace("inC25");

  pdump(pch_rcv, in_rcv, "rcv");

  in_val = inC25Disassemble(pch_rcv, in_rcv, &txnCommonData);

  txnCommonData.status = in_val;
  in_val = inC25Assemble(pch_snd, in_max, &txnCommonData);

  return in_val;
}

int inC14(char pch_snd[], int in_max, char pch_rcv[], int in_rcv) {
  int in_val = 0;
  struct EtToken et;
  int status = 0;
  int auxLength = 0;
  int i;
  char decripterbuff[320 + 1] = {0};
  char ksnOut[20 + 1];
  char bufferInHex[160 + 1] = {0};
  char binesECR[8] = {0};

  memset(&et, 0, sizeof(struct EtToken));

  log_trace("inC14");

  pdump(pch_rcv, in_rcv, "rcv");

  in_val = inC14Disassemble(pch_rcv, in_rcv, &et);

  status = in_val;

  if (status == APP_SUCC) {
    unsigned int etCrc32 = 0;
    char auxcrc32[8 + 1];
    etCrc32 = Crc32b(et.bufferEncryption);
    snprintf(auxcrc32, 8 + 1, "%08X", etCrc32);

    // Validamos el CRC32
    if (strcmp(auxcrc32, et.crc32) != 0) {
      PubBeep(1);
      PubDisplayStr(DISPLAY_MODE_CENTER, 2, 1, tr("TABLA DE BINES"));
      PubDisplayStr(DISPLAY_MODE_CENTER, 3, 1, tr("CRC32 NO COINCIDE"));
      status = kCrcMismatch;
    } else {
      status = RET_CMD_OK;
    }

    if (status == RET_CMD_OK) {
      PubBeep(1);
      PubDisplayStr(DISPLAY_MODE_CENTER, 2, 1, tr("TABLA DE BINES"));
      PubDisplayStr(DISPLAY_MODE_CENTER, 3, 1, tr("RECIBIDA"));

      auxLength = atoi(et.buffLenEncryption);
      auxLength = inAscToHex(et.bufferEncryption, bufferInHex, auxLength);
      dukpt_decrypt(bufferInHex, auxLength, decripterbuff, ksnOut);

      char bufferString[320 + 1] = {0};
      char ksnString[20 + 1] = {0};

      auxLength = atoi(et.buffLenEncryption) / 2;
      inHexToAsc(decripterbuff, bufferString, auxLength);

      for (i = 0; i < 10; i++) {
        sprintf(ksnString, "%s%02X", ksnString, ksnOut[i]);
      }

      log_trace("Buffer Longitud: %d", auxLength);
      log_trace("Buffer descencriptado: %s", bufferString);
      log_trace("KSN Desecriptado: %s", ksnString);

      TRACE_HEX(decripterbuff, auxLength, "Buffer: ");
      TRACE_HEX(ksnOut, 10, "KSN: ");

      /*char cadena[] =
          "37000000B37999999CA40081800B40081899CA50622100B50622199CA"
          "50785800B50785899CA58877202B58877284CA60368101B60368103CA528843CA627"
          "31811CA62753500B62753506CA62763604B62763640CA63916100B63916199CA6393"
          "88CA63950900B63950999CA90000000B90000199CA92000000B92000099CA9300000"
          "0B93000099CA37666409B37666410CFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
          "FFFFFFFFFFFFFFFFFFFF";
      setProperty(FILE_BINTABLE, cadena);*/

      // Validamos ID de tabla de Bines con Z3
      getProperty(FILE_BINESECR, binesECR);

      if (strcmp(binesECR, et.binTableId) != 0) {
        PubBeep(1);
        PubDisplayStr(DISPLAY_MODE_CENTER, 2, 1, tr("ERROR EN DESCARGA"));
        PubDisplayStr(DISPLAY_MODE_CENTER, 3, 1, tr("TABLA NO COINCIDE"));
        PubSysDelay(10);
      } else {
        // VALIDAMOS QUE SEA UNA TABLA DE BINES VALIDA
        if (Parse_BinesTable(bufferString) != 0) {
          PubBeep(1);
          PubDisplayStr(DISPLAY_MODE_CENTER, 3, 1, tr("ERROR DE FORMATO"));
          setProperty(FILE_BINTABLE, " ");
        } else {
          // SALVAMOS TABLA DE BINES
          setProperty(FILE_BINTABLE, bufferString);

          // Salvamos ID de Tabla de Bines para Token ES
          setProperty(FILE_BINESPINPAD, et.binTableId);

          // Salvamos Version de Tabla de Bines para Token ES
          setProperty(FILE_VERSIONBINES, et.version);
        }
      }
    }
  }

  in_val = inC14Assemble(pch_snd, in_max, status);

  // Regresamos al IDLE
  PubSysDelay(10);
  EnableDispDefault();

  return in_val;
}

int inC12(char pch_snd[], int in_max, char pch_rcv[], int in_rcv) {
  int in_val = RET_CMD_OK;
  int scriptType = 0;

  TRACE("inC12");

  pdump(pch_rcv, in_rcv, "rcv");

  in_val = inC12Disassemble(pch_rcv, in_rcv, &scriptType);

  in_val = inC12Assemble(pch_snd, in_max, in_val, scriptType);

  return in_val;
}

int inI02(char pch_snd[], int in_max, char pch_rcv[], int in_rcv) {
  int inI02 = 0;

  log_trace("inI02");

  pdump(pch_rcv, in_rcv, "rcv");

  DISP_OUT_ICC;

  inI02 = 0;

  memcpy(pch_snd + inI02, "I020", 4);
  inI02 += 4;

  return inI02;
}

int inZ2(char pch_snd[], int in_max, char pch_rcv[], int in_rcv) {
  char ch_rw1[21 + 1] = {0};
  char ch_rw2[21 + 1] = {0};
  char *pch_rw1;
  char *pch_rw2;
  char *pch_end;
  int in_siz;
  int inZ2 = 0;

  log_trace("inZ2");

  pdump(pch_rcv, in_rcv, "rcv");

  log_trace("inZ2");

  pch_rw1 = pch_rcv;
  if (*pch_rw1 == 0x1A) pch_rw1++;

  if (strlen(pch_rcv) - 1 > 21) {
    memcpy(ch_rw1, pch_rcv + 1, 21);
  } else {
    memcpy(ch_rw1, pch_rcv + 1, strlen(pch_rcv) - 1);
  }

  setProperty(FILE_TXT, ch_rw1);

  EnableDispDefault();
  PubBeep(1);

  /*pch_end = pch_rcv + in_rcv;
  pch_rw1 = pch_rcv;
  if (*pch_rw1 == 0x1A) pch_rw1++;
  in_siz = pch_end - pch_rw1;
  pch_rw2 = NULL;
  if (in_siz > 16) pch_rw2 = pch_rw1 + 16;
  if (pch_rw2)
    in_siz = 16;
  else
    in_siz = pch_end - pch_rw1;
  memcpy(ch_rw1, pch_rw1, in_siz);
  in_siz = 0;
  if (pch_rw2) in_siz = pch_end - pch_rw2;
  memcpy(ch_rw2, pch_rw2, in_siz);

  pdump(ch_rw1, strlen(ch_rw1), "ch_rw1");

  pdump(ch_rw2, strlen(ch_rw2), "ch_rw2");

  pch_rw1 = ch_rw1;
  pch_rw2 = ch_rw2;

  if (pch_rw1 != NULL || pch_rw2 != NULL) {
    PubClearAll();

    if (pch_rw1 != NULL) {
      PubDisplayStrInline(DISPLAY_MODE_CENTER, 2, "%s", pch_rw1);
    }
    if (pch_rw2 != NULL) {
      PubDisplayStrInline(DISPLAY_MODE_CENTER, 3, "%s", pch_rw2);
    }

    PubUpdateWindow();
    PubKbHit();

    PubBeep(1);
    PubSysDelay(8);
}*/

  return inZ2;
}

int inZ3(char pch_snd[], int in_max, char pch_rcv[], int in_rcv) {
  int inZ3 = 0;
  char chBinesECR[8] = {0};

  log_trace("inZ3");

  log_trace("Text in Z3: %s ", pch_rcv);

  getProperty(FILE_BINESECR, chBinesECR);

  // Validamos que solo se cambie una sola vez la variable del Z3
  if (strcmp(chBinesECR, "00000000") == 0) {
    if (strlen(pch_rcv) == 8) {
      setProperty(FILE_BINESECR, pch_rcv);
    }
  }

  return inZ3;
}

int inZ10(char pch_snd[], int in_max, char pch_rcv[], int in_rcv) {
  int in_val = RET_CMD_OK;

  // FIXME: A esta funcion solo llega un Z10 pelon (3)
  /* 	// El unico Z10 de solicitud correcto
          const unsigned char z10[] = {0x02, 0x5A, 0x31,  0x30, 0x03, 0x58};
          if(memcmp(pch_rcv, z10, in_rcv) != 0)
          {
                  log_error("Z10 recibido no tiene el formato valido");
                  return RET_CMD_FAIL;
          }
          log_info("Z10 recibido correctamente"); */

  PubDisplayStr(DISPLAY_MODE_CENTER, 2, 1, tr("INICIALIZANDO"));
  PubDisplayStr(DISPLAY_MODE_CENTER, 3, 1, tr("LLAVES"));

  in_val = inZ10Assemble(pch_snd, in_max);

  return in_val;
}

int inZ11(char pch_snd[], int in_max, char pch_rcv[], int in_rcv) {
  int in_val = RET_CMD_OK;
  struct ExToken ex;
  int status = 0;

  // Must be zeroed after loading
  unsigned char encryptedIpek[16] = {0};
  unsigned char ipek[16] = {0};
  unsigned char ksn[10] = {0};

  unsigned int ipekLen = 0;
  unsigned int ksnLen = 0;

  in_val = inZ11Dissasemble(pch_rcv, in_rcv, &ex);
  if (in_val == 0) {
    if (strcmp(ex.requestStatus, "00") == 0) {

    
      HexStringToBytes(ex.encryptedIpek, &ipekLen, encryptedIpek);
      HexStringToBytes(ex.initialKsn, &ksnLen, ksn);

      log_debug("IPEK cifrada: %s", ex.encryptedIpek);

      tk_decrypt(encryptedIpek, sizeof(encryptedIpek), ipek);

      in_val = load_ipek(ipek, ksn);

      // TODO: Zero IPEK, properly (memset is optimized out)
      memset(ipek, 0, 16);

      if (in_val == NAPI_OK) {
        PubDisplayStr(DISPLAY_MODE_CENTER, 2, 1, "INICIALIZACION");
        PubDisplayStr(DISPLAY_MODE_CENTER, 3, 1, "EXITOSA");

        // REINICIAMOS EL CONTADOR REAL DE CIFRADOS
        setProperty(FILE_REALCIPHERCOUNTER, "0");
        log_debug("Llave cargada correctamente");
        status = RET_CMD_OK;

        // APAGAMOS LA BANDERA DE NUEVA LLAVE REQUERIDA
        SetPetitionFlagOfNewKey(NO_KEY_REQUEST_REQUIRED);

      } else {
        PubDisplayStr(DISPLAY_MODE_CENTER, 2, 1, "ERROR AL CARGAR LLAVE");
        PubDisplayStr(DISPLAY_MODE_CENTER, 3, 1, "IPEK A PINPAD");
        status = kOtherFailure;
      }
    } else {
      PubDisplayStr(DISPLAY_MODE_CENTER, 2, 1, "INI LLAVES FALLO");
      PubDisplayStr(DISPLAY_MODE_CENTER, 3, 1, "LLAME A SOPORTE %s",
                    ex.requestStatus);
      if (strcmp(ex.requestStatus, "01") == 0) {
        status = kBadCheckValue;
      } else if (strcmp(ex.requestStatus, "02") == 0) {
        status = kNonExistentKey;
      } else if (strcmp(ex.requestStatus, "03") == 0) {
        status = kCrcMismatch;
      } else if (strcmp(ex.requestStatus, "04") == 0) {
        status = kHsmCryptoError;
      } else {
        status = kOtherFailure;
      }
    }
  } else {
    PubDisplayStr(DISPLAY_MODE_CENTER, 2, 1, "ERROR");
    PubDisplayStr(DISPLAY_MODE_CENTER, 3, 1, "TOKEN EX INVALIDO");
    status = kOtherFailure;
  }
  in_val = inZ11Assemble(pch_snd, in_max, status);

  // Regresamos al IDLE
  PubSysDelay(10);
  EnableDispDefault();

  return in_val;
}

int inQ8(char pch_snd[], int in_max, char pch_rcv[], int in_rcv) {
  int inSnd;

  log_trace("inQ8");

  pdump(pch_rcv, in_rcv, "rcv");
  inSnd = 0;

  if (in_rcv == 2) {
    int inQs;

    memcpy(pch_snd + inSnd, "Q8", 2);
    pdump(pch_snd + inSnd, 2, "cmd");
    inSnd += 2;

    inQs = inESAssemble(pch_snd + inSnd, TRUE, GetPetitionFlagOfNewKey());
    pdump(pch_snd + inSnd, inQs, "qs");
    inSnd += inQs;
  }

  pdump(pch_snd, inSnd, "snd");

  log_trace("inSnd=%d", inSnd);

  return inSnd;
}

// Q7 pendiente
int inQ7(char pch_snd[], int in_max, char pch_rcv[], int in_rcv) {
  return 0;
}
/*
int inQ7(char pch_snd[], int in_max, char pch_rcv[], int in_rcv) {
  int inQ7 = 0, auxlen = 0, status = RET_CMD_OK, nRet = 0;
  char chPackage[1000 + 1] = {0};
  int nFd = 0;
  int ulRemainStorage = 0;

  log_trace("inQ7");

  pdump(pch_rcv, in_rcv, "rcv");

  auxlen = pch_rcv[2];
  auxlen <<= 8;
  auxlen |= pch_rcv[3];
  log_trace("in_siz=%d , realsize= %d", auxlen, (in_rcv - 4));

  if ((in_rcv - 4) != auxlen) {
    log_error("LONGITUD DE COMANDO NO COINCIDE CON LONGITUD REAL");
    lenTelecarga = 0;
    status = kRemoteLoadLengthError;
  }

  NAPI_FsGetDiskSpace(1, &ulRemainStorage);
  log_trace("EDisp: %d , lenPkt: %d", ulRemainStorage, auxlen);
  if (ulRemainStorage <= auxlen) {
    log_error("MPOS NO CUENTA CON SUFICIENTE MEMORIA");
    status = RET_CMD_INVALID_PARAM;
  }

  if (status == RET_CMD_OK) {
    // PRIMER BLOQUE BORRAMOS EL ARCHIVO DE CARGA
    if (lenTelecarga == 0) {
      log_trace("PRIMER BLOQUE DE TELECARGA");
      PubDelFile(OTA_APP_FILE);
    }

    // ULTIMO BLOQUE DE TELECARGA
    if (auxlen == 0) {
      log_trace("ULTIMO BLOQUE DE TELECARGA");
      lenTelecarga = 0;
      nRet = NAPI_AppInstall(OTA_APP_FILE);
      TRACE("NAPI_AppInstall: %d", nRet);
      status = nRet;
    } else {
      // OMITIMOS EL Q7,ETX Y LRC
      memcpy(chPackage, pch_rcv + 2, in_rcv - 2);

      PubOpenFile(OTA_APP_FILE, "w", &nFd);
      PubFsSeek(nFd, 0, SEEK_END);
      PubFsWrite(nFd, chPackage, auxlen);
      PubFsClose(nFd);
      lenTelecarga = lenTelecarga + auxlen;
    }

  } else {
    lenTelecarga = 0;
    PubDelFile(OTA_APP_FILE);
  }

  memcpy(pch_snd + inQ7, "Q7", 2);
  pdump(pch_snd + inQ7, 2, "cmd");
  inQ7 += 2;
  inStatusAssemble(pch_snd + inQ7, status);
  inQ7 += 2;

  pdump(pch_snd, inQ7, "snd");

  log_trace("inSnd=%d", inQ7);

  return inQ7;
}
*/

/*---------------------------------------------------*/
// 1 DISASSEMBLER /   ASSEMBLER
/*---------------------------------------------------*/

int inC50Disassemble(char pch_rcv[], int in_rcv, TxnCommonData *txnCommonData) {
  unsigned short usTag = 0;
  unsigned long ulAmount;
  unsigned char *pucAmount;
  int iLen = 0;
  char *pucAux, *pucInp;
  char szTmp0[50] = {0};
  int idx = 0;
  int idx2 = 0;

  pdump(pch_rcv, in_rcv, "rcv");

  if (txnCommonData == NULL) return RET_CMD_INVALID_PARAM;

  if (!memcmp(pch_rcv, "C50", 3)) {
    txnCommonData->inCommandtype = CMD_C50;
  } else {
    return RET_CMD_INVALID_PARAM;
  }

  txnCommonData->inLongitud = pch_rcv[3];
  txnCommonData->inLongitud <<= 8;
  txnCommonData->inLongitud |= pch_rcv[4];

  if ((in_rcv - 5) != txnCommonData->inLongitud) {
    return RET_CMD_INVALID_PARAM;
  }

  pucInp = pch_rcv + 5;

  // timeout

  iLen = iGetFieldFromInp(&usTag, &pucAux, &pucInp);

  if (usTag != TAG_C1_INPUT_PARAM || iLen != 1) {
    return RET_CMD_INVALID_PARAM;
  }

  inHexToAsc((char *)pucAux, szTmp0, 1);

  szTmp0[2] = 0;

  txnCommonData->inTimeOut = atoi(szTmp0);
  // Ajuste de timeout a 80% del valor que recibe
  txnCommonData->inTimeOut = (txnCommonData->inTimeOut * .8);

  log_trace("iTimeout=%d", txnCommonData->inTimeOut);

  // date

  iLen = iGetFieldFromInp(&usTag, &pucAux, &pucInp);

  if (usTag != TAG_C1_INPUT_PARAM || iLen != 3) {
    return RET_CMD_INVALID_PARAM;
  }

  memset(szTmp0, 0x00, sizeof szTmp0);

  memcpy(szTmp0, pucAux, iLen);

  pdump(szTmp0, 3, "H Date:");
  inHexToAsc(szTmp0, txnCommonData->chDate, 3);
  pdump(txnCommonData->chDate, strlen(txnCommonData->chDate), "S Date:");

  log_trace("chDate=%s", txnCommonData->chDate);

  // time

  iLen = iGetFieldFromInp(&usTag, &pucAux, &pucInp);

  if (usTag != TAG_C1_INPUT_PARAM || iLen != 3) {
    return RET_CMD_INVALID_PARAM;
  }

  memset(szTmp0, 0x00, sizeof szTmp0);

  memcpy(szTmp0, pucAux, iLen);

  pdump(szTmp0, 3, "H Time:");

  inHexToAsc(szTmp0, txnCommonData->chTime, 3);
  pdump(txnCommonData->chTime, strlen(txnCommonData->chTime), "S Time:");

  log_trace("chTime=%s", txnCommonData->chTime);

  // type

  iLen = iGetFieldFromInp(&usTag, &pucAux, &pucInp);

  if (usTag != TAG_C1_INPUT_PARAM || iLen != 1) {
    return RET_CMD_INVALID_PARAM;
  }

  txnCommonData->inFlagTransType = *pucAux;

  txnCommonData->inFlagTransType >>= 4;

  txnCommonData->inFlagTransType &= 0x0F;

  ParseFlagTransType(txnCommonData->inFlagTransType, txnCommonData);

  log_trace("CVVREQ: %d   ||   EXPDATEREQ: %d", txnCommonData->inCvvReq,
            txnCommonData->inExpDateReq);

  txnCommonData->inTranType = (int)*pucAux;

  txnCommonData->inTranType &= 0x0F;

  log_trace("inTranType=%d", txnCommonData->inTranType);

  // amount

  iLen = iGetFieldFromInp(&usTag, &pucAux, &pucInp);

  if (usTag != TAG_C1_INPUT_PARAM || iLen != 4) {
    return RET_CMD_INVALID_PARAM;
  }

  log_trace("AMOUNT C50=%s", pucAux);

  pucAmount = pucAux;

  vdUTL_BinToLong(&ulAmount, pucAmount);

  log_trace("ulAmount=%ld", ulAmount);

  sprintf(txnCommonData->chAmount, "%012ld", ulAmount);

  log_trace("chAmount [%s]", txnCommonData->chAmount);

  // Bandera de PAN o BIN

  iLen = iGetFieldFromInp(&usTag, &pucAux, &pucInp);

  if (usTag != TAG_C1_INPUT_PARAM || iLen != 1) {
    txnCommonData->inFlagPanOrBin = 0x01;
  } else {
    txnCommonData->inFlagPanOrBin = (int)(*pucAux);
  }

  log_trace("inFlagPanOrBin=%d", txnCommonData->inFlagPanOrBin);

  return RET_CMD_OK;
}

int inC50Assemble(char pch_snd[], int in_max, TxnCommonData *txnCommonData) {
  int inC50;
  int inC50Len;
  char *pchFld;
  char chTemp[512] = {0};
  int inTemp = 0;

  // VALIDATE PEM
  switch (txnCommonData->shEntryMode) {
    case CMD_PEM_EMV:
      strcpy(txnCommonData->chMod, "05");
      break;
    case CMD_PEM_CTLS:
      strcpy(txnCommonData->chMod, "07");
      break;
    case CMD_PEM_MSR:
      strcpy(txnCommonData->chMod, "90");
      break;
    case CMD_PEM_MSR_FBK:
      strcpy(txnCommonData->chMod, "80");
      break;
    case CMD_PEM_KBD:
      strcpy(txnCommonData->chMod, "01");
      break;
    case CMD_PEM_UNDEF:
      strcpy(txnCommonData->chMod, "00");
      break;
  }

  txnCommonData->inMod = 2;

  inC50 = 0;

  memcpy(pch_snd + inC50, "C50", 3);
  inC50 += 3;

  // status
  inStatusAssemble(pch_snd + inC50, txnCommonData->status);
  inC50 += 2;

  // Length
  inC50 += 2;

  pchFld = pch_snd + inC50;

  if (txnCommonData->status == RET_CMD_OK ||
      txnCommonData->status == L3_TXN_APPROVED ||
      txnCommonData->status == L3_TXN_ONLINE) {
    char auxPan[20] = {0};
    memset(auxPan, 0x00, sizeof(auxPan));

    /*BANDERA PARA INFORMAR PAN O BIN
      00 PAN Completo
      01 BIN tarjeta a 6 posiciones
      02 BIN tarjeta a 8 posiciones*/

    if (txnCommonData->inFlagPanOrBin == 0x00) {
      // PADEAMOS CON F's EN CASO DE SER NECESARIO
      if (txnCommonData->inPan % 2 != 0) {
        txnCommonData->inPan++;
        strcat(auxPan, txnCommonData->chPan);
        strcat(auxPan, "F");
      } else {
        strcat(auxPan, txnCommonData->chPan);
      }
    } else if (txnCommonData->inFlagPanOrBin == 0x01) {
      memcpy(auxPan, txnCommonData->chPan, 6);
    } else if (txnCommonData->inFlagPanOrBin == 0x02) {
      memcpy(auxPan, txnCommonData->chPan, 8);
    }

    inTemp = inAscToHex(auxPan, chTemp, strlen(auxPan));
    vdAddFieldToOut(TAG_C1_INPUT_PARAM, (uchar *)chTemp, inTemp,
                    (uchar **)&pchFld, &inC50);

  } else {
    vdAddFieldToOut(TAG_C1_INPUT_PARAM, (uchar *)txnCommonData->chPan,
                    txnCommonData->inPan, (uchar **)&pchFld, &inC50);
    NAPI_L3TerminateTransaction();
  }

  inC50Len = inC50 - 7;  // Daee 05/02/2021

  pch_snd[5] = (inC50Len >> 8) & 0xFF;
  pch_snd[6] = (inC50Len)&0xFF;

  pdump(pch_snd, inC50, "c50");

  return inC50;
}

int inC51Disassemble(char pch_rcv[], int in_rcv, TxnCommonData *txnCommonData) {
  unsigned short usTag = 0;
  unsigned long ulAmount;
  unsigned char *pucAmount;
  int iLen = 0;
  char *pucAux, *pucInp;
  char szTmp0[50] = {0};
  int idxP1 = 0;
  int idxP2 = 0;
  int idx2 = 0;

  pdump(pch_rcv, in_rcv, "rcv");

  if (txnCommonData == NULL) return RET_CMD_INVALID_PARAM;

  if (!memcmp(pch_rcv, "C51", 3)) {
    txnCommonData->inCommandtype = CMD_C51;
  } else {
    return RET_CMD_INVALID_PARAM;
  }

  txnCommonData->inLongitud = pch_rcv[3];
  txnCommonData->inLongitud <<= 8;
  txnCommonData->inLongitud |= pch_rcv[4];
  log_trace("in_siz=%d", txnCommonData->inLongitud);

  if ((in_rcv - 5) != txnCommonData->inLongitud) {
    return RET_CMD_INVALID_PARAM;
  }

  pucInp = pch_rcv + 5;

  // timeout

  iLen = iGetFieldFromInp(&usTag, &pucAux, &pucInp);

  if (usTag != TAG_C1_INPUT_PARAM || iLen != 1) {
    return RET_CMD_INVALID_PARAM;
  }

  inHexToAsc((char *)pucAux, szTmp0, 1);

  szTmp0[2] = 0;

  txnCommonData->inTimeOut = atoi(szTmp0);
  // Ajuste de timeout a 80% del valor que recibe
  txnCommonData->inTimeOut = (txnCommonData->inTimeOut * .8);

  log_trace("iTimeout=%d", txnCommonData->inTimeOut);

  // date

  iLen = iGetFieldFromInp(&usTag, &pucAux, &pucInp);

  if (usTag != TAG_C1_INPUT_PARAM || iLen != 3) {
    return RET_CMD_INVALID_PARAM;
  }

  memset(szTmp0, 0x00, sizeof szTmp0);

  memcpy(szTmp0, pucAux, iLen);

  pdump(szTmp0, 3, "H Date:");
  inHexToAsc(szTmp0, txnCommonData->chDate, 3);
  pdump(txnCommonData->chDate, strlen(txnCommonData->chDate), "S Date:");

  // time

  iLen = iGetFieldFromInp(&usTag, &pucAux, &pucInp);

  if (usTag != TAG_C1_INPUT_PARAM || iLen != 3) {
    return RET_CMD_INVALID_PARAM;
  }

  memset(szTmp0, 0x00, sizeof szTmp0);

  memcpy(szTmp0, pucAux, iLen);

  pdump(szTmp0, 3, "H Time:");

  inHexToAsc(szTmp0, txnCommonData->chTime, 3);
  pdump(txnCommonData->chTime, strlen(txnCommonData->chTime), "S Time:");

  // type

  iLen = iGetFieldFromInp(&usTag, &pucAux, &pucInp);

  if (usTag != TAG_C1_INPUT_PARAM || iLen != 1) {
    return RET_CMD_INVALID_PARAM;
  }

  txnCommonData->inFlagTransType = *pucAux;

  txnCommonData->inFlagTransType >>= 4;

  txnCommonData->inFlagTransType &= 0x0F;

  ParseFlagTransType(txnCommonData->inFlagTransType, txnCommonData);

  log_trace("CVVREQ: %d   ||   EXPDATEREQ: %d", txnCommonData->inCvvReq,
            txnCommonData->inExpDateReq);

  log_trace("inFlagTransType=%d", txnCommonData->inFlagTransType);

  txnCommonData->inTranType = (int)*pucAux;

  txnCommonData->inTranType &= 0x0F;

  log_trace("iTransType=%d", txnCommonData->inTranType);

  // amount

  iLen = iGetFieldFromInp(&usTag, &pucAux, &pucInp);

  if (usTag != TAG_C1_INPUT_PARAM || iLen != 4) {
    return RET_CMD_INVALID_PARAM;
  }

  pucAmount = pucAux;

  vdUTL_BinToLong(&ulAmount, pucAmount);

  log_trace("ulAmount=%ld", ulAmount);

  sprintf(txnCommonData->chAmount, "%012ld", ulAmount);

  log_trace("chAmount [%s]", txnCommonData->chAmount);

  // cash back

  iLen = iGetFieldFromInp(&usTag, &pucAux, &pucInp);

  if (usTag != TAG_C1_INPUT_PARAM || iLen != 4) {
    return RET_CMD_INVALID_PARAM;
  }

  pucAmount = pucAux;

  vdUTL_BinToLong(&ulAmount, pucAmount);

  log_trace("ulAmount cash back=%ld", ulAmount);

  sprintf(txnCommonData->chOtherAmount, "%012ld", ulAmount);

  log_trace("chOtherAmount [%s]", txnCommonData->chOtherAmount);

  // curr code

  iLen = iGetFieldFromInp(&usTag, &pucAux, &pucInp);

  if (usTag != TAG_C1_INPUT_PARAM || iLen != 2) {
    return RET_CMD_INVALID_PARAM;
  }

  memcpy(txnCommonData->chCurrCode, pucAux, 2);

  pdump(txnCommonData->chCurrCode, 2, "chCurCod:");

  // mrch dcsn

  iLen = iGetFieldFromInp(&usTag, &pucAux, &pucInp);

  if (usTag != TAG_C1_INPUT_PARAM || iLen != 1) {
    return RET_CMD_INVALID_PARAM;
  }

  txnCommonData->inMerchantDesicion = *pucAux;

  log_trace("iTermDecision=%d", txnCommonData->inMerchantDesicion);

  // e1

  iLen = iGetFieldFromInp(&usTag, &pucAux, &pucInp);

  if (usTag != TAG_E1_INPUT_PARAM) {
    return RET_CMD_INVALID_PARAM;
  }

  if (iLen > 0) {
    pdump(pucAux, iLen, "e1:");
    int idx = 0;
    idxP1 = 0;
    idxP2 = 0;
    idx2 = 0;
    // SET E1 for Response
    txnCommonData->uinE1P1[idxP1++] = _EMV_TAG_4F_IC_AID;
    txnCommonData->uinE1P1[idxP1++] = _EMV_TAG_9F12_IC_APNAME;
    txnCommonData->uinE1P1[idxP1++] = _EMV_TAG_50_IC_APPLABEL;
    txnCommonData->uinE1P1[idxP1++] = _EMV_TAG_5F30_IC_SERVICECODE;
    txnCommonData->uinE1P1[idxP1++] = _EMV_TAG_5F34_IC_PANSN;
    txnCommonData->uinE1P1[idxP1++] = _EMV_TAG_9F34_TM_CVMRESULT;
    txnCommonData->uinE1P2[idxP2++] = _EMV_TAG_95_TM_TVR;
    txnCommonData->uinE1P2[idxP2++] = _EMV_TAG_9F27_IC_CID;
    txnCommonData->uinE1P2[idxP2++] = _EMV_TAG_9F26_IC_AC;
    txnCommonData->uinE1P2[idxP2++] = _EMV_TAG_9B_TM_TSI;
    txnCommonData->uinE1P2[idxP2++] = _EMV_TAG_9F39_TM_POSENTMODE;
    txnCommonData->uinE1P2[idxP2++] = _EMV_TAG_8A_TM_ARC;
    txnCommonData->uinE1P2[idxP2++] = _EMV_TAG_99_TM_PINDATA;
    txnCommonData->uinE1P2[idxP2++] = 0x9F6E;

    txnCommonData->inuE1P1 = idxP1;
    txnCommonData->inuE1P2 = idxP2;

    // SET E2 for Response
    idx = 0;
    while (idx2 < iLen) {
      if ((pucAux[idx2] & 0x1F) == 0x1F) {
        // 2-byte tag
        usTag = pucAux[idx2++];
        usTag <<= 8;
        usTag = (short)(usTag + pucAux[idx2++]);
      } else {
        usTag = pucAux[idx2++];
        //	            usTag <<= 8;
      }

      txnCommonData->uinE2[idx++] = (unsigned int)usTag;
      log_trace("tag %d : %04x", idx, txnCommonData->uinE2[idx - 1]);
    }
    txnCommonData->inuE2 = idx;

    log_trace("inuE1P1=%d", txnCommonData->inuE1P1);
    log_trace("inuE1P2=%d", txnCommonData->inuE1P2);
    log_trace("inuE2=%d", txnCommonData->inuE2);
  }

  // PAN Masking

  iLen = iGetFieldFromInp(&usTag, &pucAux, &pucInp);

  if (usTag != TAG_C1_INPUT_PARAM || iLen != 1) {
    txnCommonData->inPanMasking = 0;
  } else {
    txnCommonData->inPanMasking = (int)(*pucAux);
  }

  log_trace("inpanMasking=%d", txnCommonData->inPanMasking);

  return RET_CMD_OK;
}

int inC53Assemble(char pch_snd[], int in_max, TxnCommonData *txnCommonData) {
  int inC53;
  int inC53Len;
  char *pchFld;
  char chTemp[512] = {0};
  int inTemp = 0;
  int inE1 = 0;
  int inE2 = 0;
  int auxEMVstatus = 0;

  log_trace("inC53Assemble");

  auxEMVstatus = txnCommonData->status;

  if (txnCommonData->status == RET_CMD_OK ||
      txnCommonData->status == L3_TXN_APPROVED ||
      txnCommonData->status == L3_TXN_DECLINE ||
      txnCommonData->status == L3_TXN_ONLINE) {
    // VALIDATE PEM

    switch (txnCommonData->shEntryMode) {
      case CMD_PEM_EMV:
        strcpy(txnCommonData->chMod, "05");
        txnCommonData->status = RET_CMD_OK;
        break;
      case CMD_PEM_CTLS:
        strcpy(txnCommonData->chMod, "07");
        txnCommonData->status = kCtlss;
        break;
      case CMD_PEM_MSR:
        strcpy(txnCommonData->chMod, "90");
        txnCommonData->status = kUseMagStripe;
        break;
      case CMD_PEM_MSR_FBK:
        strcpy(txnCommonData->chMod, "80");
        txnCommonData->status = kUseMagStripe;
        break;
      case CMD_PEM_KBD:
        strcpy(txnCommonData->chMod, "01");
        txnCommonData->status = kManualEntry;
        break;
      case CMD_PEM_UNDEF:
        strcpy(txnCommonData->chMod, "00");
        txnCommonData->status = RET_CMD_INVALID_PARAM;
        break;
    }

    txnCommonData->inMod = 2;
  }

  inC53 = 0;
  // EN CASO DE QUE SEA DECLINADA EMV SE CAMBIA RESPUESTA A C54
  if (auxEMVstatus == L3_TXN_DECLINE) {
    memcpy(pch_snd + inC53, "C54", 3);
  } else {
    memcpy(pch_snd + inC53, "C53", 3);
  }
  inC53 += 3;
  // status
  inStatusAssemble(pch_snd + inC53, txnCommonData->status);
  inC53 += 2;

  // Length
  inC53 += 2;

  pchFld = pch_snd + inC53;

  if (txnCommonData->status == RET_CMD_OK ||
      txnCommonData->status == L3_TXN_APPROVED ||
      txnCommonData->status == L3_TXN_DECLINE ||
      txnCommonData->status == L3_TXN_ONLINE ||
      txnCommonData->status == RET_CMD_USE_MAG_CARD ||
      txnCommonData->status == kCtlss ||
      txnCommonData->status == kUseMagStripe ||
      txnCommonData->status == kManualEntry) {
    unsigned char currentKsn[10];
    unsigned char block1[24 + 1];
    unsigned char block2[80 + 1];

    memset(currentKsn, 0x00, sizeof(currentKsn));
    memset(block1, 0x00, sizeof(block1));
    memset(block2, 0x00, sizeof(block2));

    // PADEAMOS A 26 ESPACIOS AL NOMBRE DE TARJETAHABIENTE
    if (txnCommonData->inHld > 0) {
      char cardHolderNamePadded[26 + 1] = {0};

      memset(cardHolderNamePadded, 0x20, 26);
      cardHolderNamePadded[26 + 1] = 0x00;
      memcpy(cardHolderNamePadded, txnCommonData->chHld, txnCommonData->inHld);
      memset(txnCommonData->chHld, 0x00, 50);
      strcat(txnCommonData->chHld, cardHolderNamePadded);
      txnCommonData->inHld = 26;
    }

    // PADEO DE PAN EN CASO DE LONGITUD NON
    char auxPan[30] = {0};
    memset(auxPan, 0x00, sizeof(auxPan));
    if (txnCommonData->inPan % 2 != 0) {
      txnCommonData->inPan++;
      strcat(auxPan, txnCommonData->chPan);
      strcat(auxPan, "F");
    } else {
      strcat(auxPan, txnCommonData->chPan);
    }

    if (txnCommonData->inPanMasking == 1) {
      memcpy(auxPan + 6, "2A2A2A", 6);
    }

    inTemp = inAscToHex(auxPan, chTemp, txnCommonData->inPan);
    vdAddFieldToOut(TAG_C1_INPUT_PARAM, (uchar *)chTemp, inTemp,
                    (uchar **)&pchFld, &inC53);
    vdAddFieldToOut(TAG_C1_INPUT_PARAM, (uchar *)txnCommonData->chHld,
                    txnCommonData->inHld, (uchar **)&pchFld, &inC53);
    //	vdAddFieldToOut (TAG_C1_INPUT_PARAM, (uchar *) txnCommonData->chTk2,
    // txnCommonData->inTk2, (uchar **) &pchFld, &inC53);
    vdAddFieldToOut(TAG_C1_INPUT_PARAM, (uchar *)txnCommonData->chTk2,
                    txnCommonData->inTk2, (uchar **)&pchFld, &inC53);
    //	vdAddFieldToOut (TAG_C1_INPUT_PARAM, (uchar *) txnCommonData->chTk1,
    // txnCommonData->inTk1, (uchar **) &pchFld, &inC53);
    vdAddFieldToOut(TAG_C1_INPUT_PARAM, (uchar *)txnCommonData->chTk1,
                    txnCommonData->inTk1, (uchar **)&pchFld, &inC53);
    //		vdAddFieldToOut (TAG_C1_INPUT_PARAM, (uchar *)
    // txnCommonData->chCv2, txnCommonData->inCv2, (uchar **) &pchFld, &inC53);
    vdAddFieldToOut(TAG_C1_INPUT_PARAM, (uchar *)txnCommonData->chCv2,
                    txnCommonData->inCv2, (uchar **)&pchFld, &inC53);
    vdAddFieldToOut(TAG_C1_INPUT_PARAM, (uchar *)txnCommonData->chMod,
                    txnCommonData->inMod, (uchar **)&pchFld, &inC53);

    if (!memcmp(txnCommonData->chMod, "05", 2) ||
        !memcmp(txnCommonData->chMod, "07", 2)) {
      inE1 = txnCommonData->inE1;
      inE2 = txnCommonData->inE2;
    }

    pdump(txnCommonData->chE1, txnCommonData->inE1, "chE1");
    pdump(txnCommonData->chE2, txnCommonData->inE2, "chE2");

    vdAddFieldToOut(TAG_E1_INPUT_PARAM, (uchar *)txnCommonData->chE1, inE1,
                    (uchar **)&pchFld, &inC53);
    vdAddFieldToOut(TAG_E2_OUTPUT_PARAM, (uchar *)txnCommonData->chE2, inE2,
                    (uchar **)&pchFld, &inC53);

    // Ensamblamos los datos sensibles en un solo bloque
    if (txnCommonData->chReqEncryption == ENCRYPT_DATA) {
      assemble_sensitiveDataBlock(txnCommonData->chTk1, txnCommonData->chTk2,
                                  txnCommonData->chCv2, txnCommonData->chPan,
                                  txnCommonData->chExp, currentKsn, block1,
                                  block2);
    }

    inC53 += inESAssemble(pch_snd + inC53, TRUE, GetPetitionFlagOfNewKey());
    inC53 += inR1Assemble(pch_snd + inC53);
    inC53 += inEZAssemble(pch_snd + inC53, txnCommonData->chCv2,
                          txnCommonData->chTk1, txnCommonData->chTk2,
                          txnCommonData->chPan, txnCommonData->chLast4Pan,
                          txnCommonData->chExp, txnCommonData->chMod,
                          txnCommonData->inCvvReq, txnCommonData->shEntryMode,
                          currentKsn, block1, txnCommonData->chReqEncryption);
    inC53 += inEYAssemble(pch_snd + inC53, txnCommonData->chTk1, currentKsn,
                          block2, txnCommonData->chReqEncryption);
    inC53 += inCZAssemble(pch_snd + inC53);
  }

  inC53Len = inC53 - 7;  // Daee 05/02/2021

  pch_snd[5] = (inC53Len >> 8) & 0xFF;
  pch_snd[6] = (inC53Len)&0xFF;

  pdump(pch_snd, inC53, "c53");

  if (auxEMVstatus == L3_TXN_DECLINE) {
    // MENSAJE DE DECLINADA EMV
    NAPI_L3TerminateTransaction();
    PubClearAll();
    PubDisplayStr(DISPLAY_MODE_CENTER, 2, 1, "DECLINADA  EMV");
    PubBeep(1);
    PubSysDelay(10);

    // REGRESAMOS A PROCESANDO
    PubClearAll();
    PubDisplayTitle(tr(" "));
    PubDisplayStr(DISPLAY_MODE_CENTER, 3, 1, tr("PROCESSING..."));
    PubUpdateWindow();
  }

  return inC53;
}

int inC54Disassemble(char pch_rcv[], int in_rcv, TxnCommonData *txnCommonData) {
  unsigned short usTag = 0;
  int iLen = 0;
  char *pucAux, *pucInp;
  char szTmp0[50] = {0};
  int idx = 0;
  int idx2 = 0;

  pdump(pch_rcv, in_rcv, "rcv");

  if (txnCommonData == NULL) return RET_CMD_INVALID_PARAM;

  if (!memcmp(pch_rcv, "C54", 3)) {
    txnCommonData->inCommandtype = CMD_C54;
  } else {
    return RET_CMD_INVALID_PARAM;
  }

  txnCommonData->inLongitud = pch_rcv[3];
  txnCommonData->inLongitud <<= 8;
  txnCommonData->inLongitud |= pch_rcv[4];
  log_trace("in_siz=%d", txnCommonData->inLongitud);

  if ((in_rcv - 5) != txnCommonData->inLongitud) {
    return RET_CMD_INVALID_PARAM;
  }

  pucInp = pch_rcv + 5;

  // host dscn

  iLen = iGetFieldFromInp(&usTag, &pucAux, &pucInp);

  if (usTag != TAG_C1_INPUT_PARAM || iLen != 1) {
    return RET_CMD_INVALID_PARAM;
  }

  txnCommonData->inHostDec = (int)(*pucAux + 1);

  log_trace("inHostDec=%d", txnCommonData->inHostDec);

  // auth code

  iLen = iGetFieldFromInp(&usTag, &pucAux, &pucInp);

  if (usTag != TAG_C1_INPUT_PARAM) {
    return RET_CMD_INVALID_PARAM;
  }

  memcpy(txnCommonData->chAuthCode, pucAux, iLen);

  pdump(txnCommonData->chAuthCode, iLen, "chAuthCode:");

  // resp code

  iLen = iGetFieldFromInp(&usTag, &pucAux, &pucInp);

  if (usTag != TAG_C1_INPUT_PARAM) {
    return RET_CMD_INVALID_PARAM;
  }

  memcpy(txnCommonData->chRspCode, pucAux, iLen);

  pdump(txnCommonData->chRspCode, iLen, "chRspCod:");

  // issr auth data

  iLen = iGetFieldFromInp(&usTag, &pucAux, &pucInp);

  if (usTag != TAG_91_ISS_AUTH_DATA) {
    return RET_CMD_INVALID_PARAM;
  }

  memcpy(txnCommonData->chIssAuthData, pucAux, iLen);
  txnCommonData->inIssAuthData = iLen;

  pdump(txnCommonData->chIssAuthData, txnCommonData->inIssAuthData,
        "chIssAuthData:");

  // date

  iLen = iGetFieldFromInp(&usTag, &pucAux, &pucInp);

  if (usTag != TAG_C1_INPUT_PARAM || iLen != 3) {
    // return RET_CMD_INVALID_PARAM;
    memset(txnCommonData->chDate, 0x00, 6);

  } else {
    memset(szTmp0, 0x00, sizeof szTmp0);
    memcpy(szTmp0, pucAux, iLen);
    pdump(szTmp0, 3, "H Date:");
    inHexToAsc(szTmp0, txnCommonData->chDate, 3);
    pdump(txnCommonData->chDate, strlen(txnCommonData->chDate), "S Date:");
  }

  // time

  iLen = iGetFieldFromInp(&usTag, &pucAux, &pucInp);

  if (usTag != TAG_C1_INPUT_PARAM || iLen != 3) {
    // return RET_CMD_INVALID_PARAM;
    memset(txnCommonData->chTime, 0x00, 6);

  } else {
    memset(szTmp0, 0x00, sizeof szTmp0);

    memcpy(szTmp0, pucAux, iLen);

    pdump(szTmp0, 3, "H Time:");

    inHexToAsc(szTmp0, txnCommonData->chTime, 3);
    pdump(txnCommonData->chTime, strlen(txnCommonData->chTime), "S Time:");
  }

  // e2

  iLen = iGetFieldFromInp(&usTag, &pucAux, &pucInp);

  if (usTag != TAG_E2_OUTPUT_PARAM) {
    return RET_CMD_INVALID_PARAM;
  }

  // Validamos si envian ARPC y no pide tags E2
  if (iLen == 0 && txnCommonData->inIssAuthData != 0) {
    return RET_CMD_INVALID_PARAM;
  }

  if (iLen > 0) {
    pdump(pucAux, iLen, "e2:");

    if ((txnCommonData->inHostDec > 3) ||
        (memcmp(txnCommonData->chMod, "05", 2) != 0)) {
      txnCommonData->inuE2 = 0;
    } else {
      idx = 0;
      idx2 = 0;
      // SET E2 for Response
      while (idx2 < iLen) {
        if ((pucAux[idx2] & 0x1F) == 0x1F) {
          // 2-byte tag
          usTag = pucAux[idx2++];
          usTag <<= 8;
          usTag = (short)(usTag + pucAux[idx2++]);
        } else {
          usTag = pucAux[idx2++];
          //		            usTag <<= 8;
        }

        txnCommonData->uinE2[idx++] = (unsigned int)usTag;
        log_trace("tag %d : %04x", idx, txnCommonData->uinE2[idx - 1]);
      }

      txnCommonData->inuE2 = idx;
    }
    log_trace("inuE2=%d", txnCommonData->inuE2);
  } else {
    memset(txnCommonData->uinE2, 0x00, sizeof txnCommonData->uinE2);
    txnCommonData->inuE2 = 0;
  }

  memset(txnCommonData->chE1, 0x00, sizeof txnCommonData->chE1);
  memset(txnCommonData->chE2, 0x00, sizeof txnCommonData->chE2);
  txnCommonData->inE1 = 0;
  txnCommonData->inE2 = 0;

  // Generate Tlv for kernel Emv
  if (!memcmp(txnCommonData->chMod, "05", 2)) {
    log_trace("Generate EMV Tlv for complete txn");
    idx = 0;

    memset(txnCommonData->chTlvCompleteTxn, 0x00,
           sizeof txnCommonData->chTlvCompleteTxn);
    /*if (txnCommonData->inIssAuthData > 0) {
      TlvAdd(0x91, txnCommonData->inIssAuthData, txnCommonData->chIssAuthData,
             txnCommonData->chTlvCompleteTxn, &txnCommonData->inTlvCompleteTxn);
      txnCommonData->chTlvCompleteTxn[idx++] = 0x91;
      txnCommonData->chTlvCompleteTxn[idx++] =
          (char)txnCommonData->inIssAuthData;
      memcpy(txnCommonData->chTlvCompleteTxn + 2, txnCommonData->chIssAuthData,
             txnCommonData->inIssAuthData);
      idx += txnCommonData->inIssAuthData;

      if (txnCommonData->inScriptEMV > 0) {
      memcpy(txnCommonData->chTlvCompleteTxn + idx, txnCommonData->chScriptEMV,
             txnCommonData->inScriptEMV);
      idx += txnCommonData->inScriptEMV;

    // txnCommonData->inTlvCompleteTxn = idx;
    }*/

    // Scripts for transmission to the ICC before the second GE
    if (lenScripts71 > 0) {
      TlvAdd(0x71, lenScripts71, script71, txnCommonData->chTlvCompleteTxn,
             &txnCommonData->inTlvCompleteTxn);
    }
    if (lenScripts72 > 0) {
      TlvAdd(0x72, lenScripts72, script72, txnCommonData->chTlvCompleteTxn,
             &txnCommonData->inTlvCompleteTxn);
    }

    // Vaciamos los scripts
    memset(script71, 0x00, 1024);
    memset(script72, 0x00, 1024);
    lenScripts71 = 0;
    lenScripts72 = 0;

    pdump(txnCommonData->chTlvCompleteTxn, txnCommonData->inTlvCompleteTxn,
          "chTlvCompleteTxn:");
  }

  return RET_CMD_OK;
}

int inC54Assemble(char pch_snd[], int in_max, TxnCommonData *txnCommonData) {
  int inC54;
  int inC54Len;
  char *pchFld;

  log_trace("inC54Assemble");

  inC54 = 0;

  memcpy(pch_snd + inC54, "C54", 3);
  inC54 += 3;

  // status
  inStatusAssemble(pch_snd + inC54, txnCommonData->status);
  inC54 += 2;

  // Length
  inC54 += 2;

  pchFld = pch_snd + inC54;

  if (txnCommonData->status == L3_TXN_OK ||
      txnCommonData->status == L3_TXN_APPROVED ||
      txnCommonData->status == L3_TXN_DECLINE ||
      txnCommonData->status == RET_CMD_OK) {
    pdump(txnCommonData->chE2, txnCommonData->inE2, "pchE2:");

    vdAddFieldToOut(TAG_E2_OUTPUT_PARAM, (uchar *)txnCommonData->chE2,
                    txnCommonData->inE2, (uchar **)&pchFld, &inC54);
  }

  inC54Len = inC54 - 7;
  pch_snd[5] = (inC54Len >> 8) & 0xFF;
  pch_snd[6] = (inC54Len)&0xFF;

  pdump(pch_snd, inC54, "c54");

  return inC54;
}

int inC12Disassemble(char pch_rcv[], int in_rcv, int *type) {
  char chTmp[2 + 1] = {0};
  int idx = 0;

  TRACE("inC12Disassemble");

  pdump(pch_rcv, in_rcv, "rcv");

  // if (txnCommonData == NULL) return RET_CMD_INVALID_PARAM;

  if (!memcmp(pch_rcv, "C12", 3)) {
    // txnCommonData->inCommandtype = CMD_C12;
  } else {
    return RET_CMD_INVALID_PARAM;
  }

  idx += 3;

  if ((in_rcv - idx) != 2) {
    return RET_CMD_INVALID_PARAM;
  }

  memset(chTmp, 0x00, sizeof chTmp);
  memcpy(chTmp, pch_rcv + idx, 2);
  idx += 2;
  *type = atoi(chTmp);
  TRACE("inScriptType [%d]", *type);

  if (*type == 71) {
    if (lenScripts71 == 0) {
      return kOtherFailure;
    }

  } else if (*type == 72) {
    if (lenScripts72 == 0) {
      return kOtherFailure;
    }
  } else {
    return kOtherFailure;
  }

  return RET_CMD_OK;
}

int inC12Assemble(char pch_snd[], int in_max, int status, int scriptType) {
  int inC12;
  int number = 0;
  int scriptLength = 0;
  int scriptsCount = 0;
  //int i, res;
  int i;
  L3_TXN_RES res;
  int nRet = 0;
  char szTlvList[250 + 1] = {0};
  int nTlvLen = 0;
  char respScriptString[10 + 1] = {0};

  log_trace("inC12Assemble");

  inC12 = 0;

  memcpy(pch_snd + inC12, "C12", 3);
  inC12 += 3;

  /*if (txnCommonData->shEntryMode == CMD_PEM_EMV) {
    if (ProGetCardStatus() != APP_SUCC) {
      PubClear2To4();
      PubDisplayStr(DISPLAY_MODE_CENTER, 3, 1, "TARJETA RETIRADA");
      PubUpdateWindow();
      PubBeep(1);
      PubSysDelay(5);
      status = RET_CMD_CARD_REMOVE;
      NAPI_L3TerminateTransaction();
      isC50Present = 0;
    }

  } else {
    status = kOtherFailure;
  }*/

  // status
  inStatusAssemble(pch_snd + inC12, status);
  inC12 += 2;

  // Length Script
  inC12 += 2;

  TRACE("inScriptType [%d]", scriptType);

  if (status == RET_CMD_OK) {
    if (scriptType == 71 && lenScripts71 > 0) {
      TRACE("C12 Script 71, comenzando ");
      unsigned char auxScript71[250] = {0};
      for (i = 0; i < lenScripts71; i++) {
        if (script71[i] == '|') {
          memcpy(auxScript71, script71 + number, i - number);

          scriptLength = (i - number);
          TRACE("Len de script = %d ", scriptLength);
          number = i + 1;
          scriptsCount += 1;
          TRACE("Script N°: %d ", scriptsCount);
          TRACE_HEX(auxScript71, scriptLength, "Script:");

          // Script Result
          szTlvList[0] = 0x01;
          nTlvLen = 1;
          TlvAdd(0x8A, 2, "00", szTlvList, &nTlvLen);
          TlvAdd(0x71, scriptLength, auxScript71, szTlvList, &nTlvLen);
          TRACE_HEX(szTlvList, nTlvLen, "TlvList:");
          nRet = NAPI_L3CompleteTransaction(szTlvList, nTlvLen, &res);
          TRACE("NAPI_L3CompleteTransaction, nErrcode=%d, res=%d", nRet, res);

          nRet = NAPI_L3GetData(L3_DATA_ISSUER_SCRIPT_RESULT, auxScript71, 90);
          TRACE("Script NAPI_L3GetData: %d", nRet);
          if (nRet < 0) {
            return kOtherFailure;
          } else {
            TRACE_HEX(auxScript71, nRet, "ScriptResult:");
            memcpy(pch_snd + inC12, auxScript71, 5);
            inC12 += 5;
          }

          memset(auxScript71, 0, 250);
          memset(szTlvList, 0, 250);
        }
      }

      TRACE_HEX(script71, lenScripts71, "Script:");

      memcpy(pch_snd + inC12, "\x00\x00\x00\x00\x00", 5);
      inC12 += 5;

      // TODO: Calcular la longitud de forma correcta
      pch_snd[5] = 0;
      pch_snd[6] = 1;

    } else if (scriptType == 72 && lenScripts72 > 0) {
      TRACE("C12 Script 72, comenzando ");
      unsigned char auxScript72[250] = {0};

          memcpy(auxScript72, script72 + number, i - number);

          scriptLength = (i - number);
          TRACE("Len de script = %d ", scriptLength);
          number = i + 1;
          scriptsCount += 1;
          TRACE("Script N°: %d ", scriptsCount);
          TRACE_HEX(script72, lenScripts72, "Script:");

          // Script Result
          szTlvList[0] = 0x01;
          nTlvLen = 1;
          TlvAdd(0x8A, 2, "00", szTlvList, &nTlvLen);
          TlvAdd(0x71, lenScripts72, script72, szTlvList, &nTlvLen);
          TRACE_HEX(szTlvList, nTlvLen, "TlvList:");
          nRet = NAPI_L3CompleteTransaction(szTlvList, nTlvLen, &res);
          TRACE("NAPI_L3CompleteTransaction, nErrcode=%d, res=%d", nRet, res);

          nRet = NAPI_L3GetData(L3_DATA_ISSUER_SCRIPT_RESULT, auxScript72, 90);
          TRACE("Script NAPI_L3GetData: %d", nRet);
          if (nRet < 0) {
            return kOtherFailure;
          } else {
            TRACE_HEX(script72, nRet, "ScriptResult:");
            memcpy(pch_snd + inC12, auxScript72, 5);
            inC12 += 5;
          }
          memset(auxScript72, 0, 250);
          memset(szTlvList, 0, 250);

      TRACE_HEX(script72, lenScripts72, "Script:");
      memcpy(pch_snd + inC12, "\x00\x00\x00\x00\x00", 5);
      inC12 += 5;
      // TODO: Calcular la longitud de forma correcta
      pch_snd[5] = 0;
      pch_snd[6] = 1;
    }
  }

  pdump(pch_snd, inC12, "c12");

  return inC12;
}

int inC25Disassemble(char pch_rcv[], int in_rcv, TxnCommonData *txnCommonData) {
  char chTmp[512] = {0};
  int idx = 0;
  int inScriptType = 0;
  short flagFile = 0;
  int inScriptNum = 0;
  int lengthBeforeScript = 0;

  TRACE("inC25Disassemble");

  pdump(pch_rcv, in_rcv, "rcv");

  if (txnCommonData == NULL) return RET_CMD_INVALID_PARAM;

  if (!memcmp(pch_rcv, "C25", 3)) {
    txnCommonData->inCommandtype = CMD_C51;
  } else {
    return RET_CMD_INVALID_PARAM;
  }

  idx += 3;

  if ((in_rcv - idx) < 8) {
    return RET_CMD_INVALID_PARAM;
  }

  memset(chTmp, 0x00, sizeof chTmp);
  memcpy(chTmp, pch_rcv + idx, 2);
  idx += 2;
  inScriptType = atoi(chTmp);
  TRACE("inScriptType [%d]", inScriptType);

  memset(chTmp, 0x00, sizeof chTmp);
  memcpy(chTmp, pch_rcv + idx, 1);
  idx += 1;
  flagFile = atoi(chTmp);
  TRACE("flagFile [%d]", flagFile);

  memset(chTmp, 0x00, sizeof chTmp);
  memcpy(chTmp, pch_rcv + idx, 2);
  idx += 2;
  inScriptNum = atoi(chTmp);
  TRACE("inScriptNum [%d]", inScriptNum);

  memset(chTmp, 0x00, sizeof chTmp);
  memcpy(chTmp, pch_rcv + idx, 3);
  idx += 3;
  txnCommonData->inScriptEMV = atoi(chTmp);
  TRACE("inScript [%d]", txnCommonData->inScriptEMV);

  if ((in_rcv - idx) != (txnCommonData->inScriptEMV * 2)) {
    return RET_CMD_INVALID_PARAM;
  }

  pdump(pch_rcv + idx, (txnCommonData->inScriptEMV * 2), "chScriptEMV");

  inAscToHex(pch_rcv + idx, txnCommonData->chScriptEMV,
             (txnCommonData->inScriptEMV * 2));

  pdump(txnCommonData->chScriptEMV, txnCommonData->inScriptEMV, "chScriptEMV");

  if (inScriptType == 71) {
    if (flagFile == 1) {
      TRACE("Bandera de Archivo = 1");
      if (inScriptNum == 1 && firstScriptOfTwo != 71 &&
          firstScriptOfTwo != 72) {
        TRACE("Only one message Script 71");
        // Limpia el archivo antes de agregar el script
        lenScripts71 = 0;
        memset(script71, 0x00, 1024);
        memcpy(script71, txnCommonData->chScriptEMV,
               txnCommonData->inScriptEMV);
        // script71[txnCommonData->inScriptEMV] = '|';
        lenScripts71 = txnCommonData->inScriptEMV;
      } else if (inScriptNum == 81 && firstScriptOfTwo != 71 &&
                 firstScriptOfTwo != 72) {
        TRACE("First c25 of two");
        // Limpia el archivo antes de agregar el script
        lenScripts71 = 0;
        memset(script71, 0x00, 1024);
        memcpy(script71, txnCommonData->chScriptEMV,
               txnCommonData->inScriptEMV);
        firstScriptOfTwo = 71;
        lenScripts71 = txnCommonData->inScriptEMV;
      } else if (inScriptNum == 91) {
        TRACE("Second c25 of two");
        if (firstScriptOfTwo != 71) {
          return kInvalidMsg;
        } else {
          lengthBeforeScript = lenScripts71;
          unsigned char concatScript[txnCommonData->inScriptEMV];
          memcpy(concatScript, txnCommonData->chScriptEMV,
                 txnCommonData->inScriptEMV);
          memcpy(script71 + lengthBeforeScript, concatScript,
                 txnCommonData->inScriptEMV);
          // script71[(txnCommonData->inScriptEMV) + (lengthBeforeScript)] =
          // '|';
          lenScripts71 = txnCommonData->inScriptEMV + (lengthBeforeScript);
          firstScriptOfTwo = 0;
        }
      } else {
        TRACE("ERROR falta segunda parte de script %d ", firstScriptOfTwo);
        return kInvalidMsg;
      }
    } else if (flagFile == 0 && lenScripts71 > 0) {
      TRACE("Bandera de Archivo = 0");
      lengthBeforeScript = lenScripts71;
      // Agrega un script a un archivo existente
      char concatScript[txnCommonData->inScriptEMV];
      if (inScriptNum == 1 && firstScriptOfTwo != 71 &&
          firstScriptOfTwo != 72) {
        memcpy(concatScript, txnCommonData->chScriptEMV,
               txnCommonData->inScriptEMV);
        memcpy(script71 + lengthBeforeScript, concatScript,
               txnCommonData->inScriptEMV);
        // script71[(txnCommonData->inScriptEMV) + (lengthBeforeScript)] = '|';
        lenScripts71 = txnCommonData->inScriptEMV + (lengthBeforeScript);
      } else if (inScriptNum == 81 && firstScriptOfTwo != 71 &&
                 firstScriptOfTwo != 72) {
        TRACE("First c25 of two");
        memcpy(concatScript, txnCommonData->chScriptEMV,
               txnCommonData->inScriptEMV);
        memcpy(script71 + lengthBeforeScript, concatScript,
               txnCommonData->inScriptEMV);
        firstScriptOfTwo = 71;
        lenScripts71 = txnCommonData->inScriptEMV + (lengthBeforeScript);
      } else if (inScriptNum == 91) {
        TRACE("Second c25 of two");
        if (firstScriptOfTwo != 71) {
          return kInvalidMsg;
        } else {
          memcpy(concatScript, txnCommonData->chScriptEMV,
                 txnCommonData->inScriptEMV);
          memcpy(script71 + lengthBeforeScript, concatScript,
                 txnCommonData->inScriptEMV);
          // script71[(txnCommonData->inScriptEMV) + (lengthBeforeScript)] =
          // '|';
          firstScriptOfTwo = 0;
          lenScripts71 = txnCommonData->inScriptEMV + (lengthBeforeScript);
        }
      } else {
        TRACE("ERROR falta segunda parte de script %d ", firstScriptOfTwo);
        return kInvalidMsg;
      }
    } else {
      TRACE("ERROR No existe archivo");
      return kOtherFailure;
    }
    TRACE("LONGITUD Script 71: %d", lenScripts71);
    TRACE_HEX(script71, lenScripts71, "script 71 hex:");
  } else if (inScriptType == 72) {
    if (flagFile == 1) {
      TRACE("Bandera de Archivo = 1");
      if (inScriptNum == 1 && firstScriptOfTwo != 72 &&
          firstScriptOfTwo != 71) {
        TRACE("Only one message Script 72");
        // Limpia el archivo antes de agregar el script
        lenScripts72 = 0;
        memset(script72, 0x00, 1024);
        memcpy(script72, txnCommonData->chScriptEMV,
               txnCommonData->inScriptEMV);
        // script72[txnCommonData->inScriptEMV] = '|';
        lenScripts72 = txnCommonData->inScriptEMV;
      } else if (inScriptNum == 81 && firstScriptOfTwo != 72 &&
                 firstScriptOfTwo != 71) {
        TRACE("First c25 of two");
        // Limpia el archivo antes de agregar el script
        lenScripts72 = 0;
        memset(script72, 0x00, 1024);
        memcpy(script72, txnCommonData->chScriptEMV,
               txnCommonData->inScriptEMV);
        firstScriptOfTwo = 72;
        lenScripts72 = txnCommonData->inScriptEMV;
      } else if (inScriptNum == 91) {
        TRACE("Second c25 of two");
        if (firstScriptOfTwo = !72) {
          return kInvalidMsg;
        } else {
          lengthBeforeScript = lenScripts72;
          unsigned char concatScript[txnCommonData->inScriptEMV];
          memcpy(concatScript, txnCommonData->chScriptEMV,
                 txnCommonData->inScriptEMV);
          memcpy(script72 + lengthBeforeScript, concatScript,
                 txnCommonData->inScriptEMV);
          // script72[(txnCommonData->inScriptEMV) + (lengthBeforeScript)] =
          // '|';
          firstScriptOfTwo = 0;
          lenScripts72 = txnCommonData->inScriptEMV + (lengthBeforeScript);
        }
      } else {
        TRACE("ERROR falta segunda parte de script %d ", firstScriptOfTwo);
        return kInvalidMsg;
      }
    } else if (flagFile == 0 && lenScripts72 > 0) {
      TRACE("Bandera de Archivo = 0");
      lengthBeforeScript = lenScripts72;
      // Agrega un script a un archivo existente
      char concatScript[txnCommonData->inScriptEMV];
      if (inScriptNum == 1 && firstScriptOfTwo != 71 &&
          firstScriptOfTwo != 72) {
        memcpy(concatScript, txnCommonData->chScriptEMV,
               txnCommonData->inScriptEMV);
        memcpy(script72 + lengthBeforeScript, concatScript,
               txnCommonData->inScriptEMV);
        // script72[(txnCommonData->inScriptEMV) + (lengthBeforeScript)] = '|';
        lenScripts72 = txnCommonData->inScriptEMV + (lengthBeforeScript);
      } else if (inScriptNum == 81 && firstScriptOfTwo != 72 &&
                 firstScriptOfTwo != 71) {
        TRACE("First c25 of two");
        memcpy(concatScript, txnCommonData->chScriptEMV,
               txnCommonData->inScriptEMV);
        memcpy(script72 + lengthBeforeScript, concatScript,
               txnCommonData->inScriptEMV);
        firstScriptOfTwo = 72;
        lenScripts72 = txnCommonData->inScriptEMV + (lengthBeforeScript);
      } else if (inScriptNum == 91) {
        TRACE("Second c25 of two");
        if (firstScriptOfTwo != 72) {
          return kInvalidMsg;
        } else {
          memcpy(concatScript, txnCommonData->chScriptEMV,
                 txnCommonData->inScriptEMV);
          memcpy(script72 + lengthBeforeScript, concatScript,
                 txnCommonData->inScriptEMV);
          // script72[(txnCommonData->inScriptEMV) + (lengthBeforeScript)] =
          // '|';
          firstScriptOfTwo = 0;
          lenScripts72 = txnCommonData->inScriptEMV + (lengthBeforeScript);
        }
      } else {
        TRACE("ERROR falta segunda parte de script %d ", firstScriptOfTwo);
        return kInvalidMsg;
      }
    } else {
      TRACE("ERROR No existe archivo");
      return kOtherFailure;
    }
    TRACE("LONGITUD Script 72: %d", lenScripts72);
    TRACE_HEX(script72, lenScripts72, "script 72 hex:");
  }

  return RET_CMD_OK;
}

int inC25Assemble(char pch_snd[], int in_max, TxnCommonData *txnCommonData) {
  int inC25;

  log_trace("inC25Assemble");

  inC25 = 0;

  memcpy(pch_snd + inC25, "C25", 3);
  inC25 += 3;

  // status
  inStatusAssemble(pch_snd + inC25, txnCommonData->status);
  inC25 += 2;

  pdump(pch_snd, inC25, "c25");

  return inC25;
}

int inC14Disassemble(char pch_rcv[], int in_rcv, struct EtToken *token) {
  char etToken[376 + 1] = {0};

  TRACE("inC14Disassemble");

  pdump(pch_rcv, in_rcv, "rcv");

  // if (txnCommonData == NULL) return RET_CMD_INVALID_PARAM;

  // Verificamos tamaño de token ET
  if ((in_rcv - 3) < 376) {
    return RET_CMD_INVALID_PARAM;
  }

  memcpy(etToken, pch_rcv + 3, 376);

  TRACE("Token ET: %s", etToken);

  return parse_et(etToken, token);
}

int inC14Assemble(char pch_snd[], int in_max, int in_status) {
  int inC14;

  TRACE("inC14Assemble");

  inC14 = 0;

  memcpy(pch_snd + inC14, "C14", 3);
  inC14 += 3;

  // status
  inStatusAssemble(pch_snd + inC14, in_status);
  inC14 += 2;

  pdump(pch_snd, inC14, "c14");

  return inC14;
}

int inZ10Assemble(char pch_snd[], int in_max) {
  int inZ10 = 0, inZ10Len = 0;
  struct EncryptedTransportKey encTk;
  char ewToken[549] = {0};

  memcpy(pch_snd, "Z10", 3);
  inZ10 += 3;

  inStatusAssemble(pch_snd + inZ10, 0);
  inZ10 += 2;

  // Longitud
  inZ10 += 2;

  GenerateTransportKey(&encTk);
  GenerateEwToken(&encTk, ewToken);

  memcpy(pch_snd + inZ10, ewToken, 548);
  inZ10 += 548;

  inZ10 += inESAssemble(pch_snd + inZ10, TRUE, TRUE);

  inZ10Len = inZ10 - 7;
  pch_snd[5] = (inZ10Len >> 8) & 0xFF;
  pch_snd[6] = (inZ10Len)&0xFF;

  return inZ10;
}

int inZ11Dissasemble(char pch_rcv[], int in_rcv, struct ExToken *token) {
  char exToken[78 + 1] = {0};

  log_trace("Desensamblando token EX");
  pdump(pch_rcv, in_rcv, "rcv");

  if (token == NULL) return RET_CMD_INVALID_PARAM;

  // TODO: Verificar tamaño antes de copiar
  memcpy(exToken, pch_rcv + 3, 78);

  if (strlen(exToken) != 78) {
    return -1;
  }

  log_debug("Token EX: %s", exToken);

  return parse_ex(exToken, token);
}

int inZ11Assemble(char pch_snd[], int in_max, int status) {
  int inZ11 = 0;
  memcpy(pch_snd, "Z11", 3);
  inZ11 += 3;

  inStatusAssemble(pch_snd + inZ11, status);
  inZ11 += 2;

  return inZ11;
}

int inESAssemble(char pchQs[], int cipher, int fDukAsk) {
  char szTmp0[50];
  char szTmp1[50] = {0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                     0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                     0x20, 0x20, 0x20, 0x20, 0x20, 0x20};
  int inQs;
  int len = 0;
  int ret = 0;

  log_trace("inAsmQs");

  memset(pchQs, 0, 80);
  inQs = 0;

  memcpy(pchQs + inQs, "!", 1);
  pdump(pchQs + inQs, 1, "eye cth:");
  inQs += 1;

  memcpy(pchQs + inQs, " ", 1);
  pdump(pchQs + inQs, 1, "usr fld 1:");
  inQs += 1;

  memcpy(pchQs + inQs, "ES", 2);
  pdump(pchQs + inQs, 2, "tkn id:");
  inQs += 2;

  memcpy(pchQs + inQs, "00060", 5);
  pdump(pchQs + inQs, 5, "dta len:");
  inQs += 5;

  memcpy(pchQs + inQs, " ", 1);
  pdump(pchQs + inQs, 1, "usr fld 2:");
  inQs += 1;

  memset(szTmp0, 0, sizeof(szTmp0));

  memcpy(szTmp0, "NWME30", 6);
  GetVarSoftVerAlt(szTmp0 + 6);
  log_trace("SoftVer is [%s]", szTmp0);

  memcpy(szTmp0 + strlen(szTmp0), szTmp1, 20 - strlen(szTmp0));
  memcpy(pchQs + inQs, szTmp0, 20);
  pdump(pchQs + inQs, 20, "app ver:");
  inQs += 20;

  memset(szTmp0, 0, sizeof(szTmp0));

  len = 16;
  ret = NAPI_SysGetInfo(SN, szTmp0, &len);
  log_trace("SN is [%s], ret: [%d]\n", szTmp0, ret);
  memcpy(szTmp0 + strlen(szTmp0), szTmp1, 20 - strlen(szTmp0));
  memcpy(pchQs + inQs, szTmp0, 20);
  pdump(pchQs + inQs, 20, "trm srl:");
  inQs += 20;

  memset(szTmp0, 0, sizeof(szTmp0));
  *szTmp0 = '0';
  if (cipher) *szTmp0 = '5';
  memcpy(pchQs + inQs, szTmp0, 1);
  pdump(pchQs + inQs, 1, "crp cfg:");
  inQs += 1;

  memset(szTmp0, 0x00, sizeof szTmp0);
  getProperty(FILE_BINESECR, szTmp0);
  memcpy(pchQs + inQs, szTmp0, CRD_EXC_NAM);
  pdump(pchQs + inQs, CRD_EXC_NAM, "bin ecr:");
  inQs += CRD_EXC_NAM;

  memset(szTmp0, 0x00, sizeof szTmp0);
  getProperty(FILE_BINESPINPAD, szTmp0);
  memcpy(pchQs + inQs, szTmp0, CRD_EXC_NAM);
  pdump(pchQs + inQs, CRD_EXC_NAM, "bin ecr:");
  inQs += CRD_EXC_NAM;

  memset(szTmp0, 0x00, sizeof szTmp0);
  getProperty(FILE_VERSIONBINES, szTmp0);
  memcpy(pchQs + inQs, szTmp0, CRD_EXC_VER);
  pdump(pchQs + inQs, CRD_EXC_VER, "bin ecr:");
  inQs += CRD_EXC_VER;

  memset(szTmp0, 0, sizeof(szTmp0));
  *szTmp0 = '0';
  if (fDukAsk) *szTmp0 = '1';
  memcpy(pchQs + inQs, szTmp0, 1);
  pdump(pchQs + inQs, 1, "new key:");
  inQs += 1;

  pdump(pchQs, inQs, "Qs:");

  return inQs;
}

int inR1Assemble(char chTokenR1[]) {
  char szTmp0[50];
  char szTmp1[50] = {0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                     0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                     0x20, 0x20, 0x20, 0x20, 0x20, 0x20};
  int inR1;
  int len = 0;
  int ret = 0;

  log_trace("inAsmQs");

  memset(chTokenR1, 0, 10);
  inR1 = 0;

  memcpy(chTokenR1 + inR1, "!", 1);
  pdump(chTokenR1 + inR1, 1, "eye cth:");
  inR1 += 1;

  memcpy(chTokenR1 + inR1, " ", 1);
  pdump(chTokenR1 + inR1, 1, "usr fld 1:");
  inR1 += 1;

  memcpy(chTokenR1 + inR1, "R1", 2);
  pdump(chTokenR1 + inR1, 2, "tkn id:");
  inR1 += 2;

  memcpy(chTokenR1 + inR1, "00000", 5);
  pdump(chTokenR1 + inR1, 5, "dta len:");
  inR1 += 5;

  memcpy(chTokenR1 + inR1, " ", 1);
  pdump(chTokenR1 + inR1, 1, "usr fld 2:");
  inR1 += 1;

  pdump(chTokenR1, inR1, "R1:");

  return inR1;
}

int inEZAssemble(char chTokenEZ[],
                 char chCv2[],
                 char chTk1[],
                 char chTk2[],
                 char chPan[],
                 char last4Pan[],
                 char chExpDate[],
                 char chMod[],
                 int reqCvv,
                 int PEM,
                 unsigned char chcurrentKsn[],
                 unsigned char chB1[],
                 char chReqEncryption) {
  int inEz;
  // TOKEN EZ
  log_trace("Start Token EZ");

  char generatedEzToken[108 + 1] = {0};
  memset(generatedEzToken, 0, 108);
  memset(chTokenEZ, 0, 108);
  unsigned int ezCrc32 = 0;
  char chRealCounterChipher[7 + 1] = {0};
  int inRealCounterChipher = 0;
  char chFailChipherCounter[7 + 1] = {0};
  int inFailChipherCounter = 0;

  struct ChapX_EzToken ez;

  memset(&ez, 0, sizeof(struct ChapX_EzToken));

  if (strlen(chTk2) > 0) {
    ez.track2Flag = 1;
    ez.track2Length = strlen(chTk2);
  } else {
    ez.track2Flag = 0;
    ez.track2Length = 0;
  }
  if (strlen(chTk1) > 0) {
    ez.track1Flag = 1;
  } else {
    ez.track1Flag = 0;
  }

  memcpy(ez.posEntryMode, chMod, 2);
  ez.posEntryMode[strlen(ez.posEntryMode)] = 0x00;
  strcat(ez.panLast4Digits, last4Pan);

  TRACE("PAN las 4: %s", ez.panLast4Digits);
  // ez.panLast4Digits = last4Pan;

  char track2Padeado[48 + 1] = {0};
  char cvv2Padeado[10 + 1] = {0};
  char completeBlock[48 + 1] = {0};
  int x;

  if (strlen(chCv2) > 0) {
    TRACE("CON CVV");
    ez.cvvFlag = '1';
    ez.cvvLength = strlen(chCv2);

  } else {
    TRACE("SIN CVV");

    if (PEM == CMD_PEM_EMV || PEM == CMD_PEM_CTLS) {
      ez.cvvFlag = 'A';
    } else {
      if (reqCvv == 1) {
        ez.cvvFlag = '0';
      } else if (reqCvv == 0) {
        ez.cvvFlag = 'A';
      }
    }
    ez.cvvLength = 0;
  }

  // Leemos el contador real de cifrados del mpos
  getProperty(FILE_REALCIPHERCOUNTER, chRealCounterChipher);
  inRealCounterChipher = atoi(chRealCounterChipher);
  getProperty(FILE_FAILEDCIPHERCOUNTER, chFailChipherCounter);
  inFailChipherCounter = atoi(chFailChipherCounter);

  ez.cipherCount = inRealCounterChipher;
  ez.failedCipherCount = inFailChipherCounter;

  // DukptEncrypt(paddedTrack, 32, encryptedTrack2, currentKsn);

  int i;
  char encryptedTrackString[48 + 1] = {0};
  char paddedTrackString[48 + 1] = {0};
  char ksnString[20 + 1] = {0};
  for (i = 0; i < 24; i++) {
    sprintf(encryptedTrackString, "%s%02X", encryptedTrackString, chB1[i]);
  }
  for (i = 0; i < 10; i++) {
    sprintf(ksnString, "%s%02X", ksnString, chcurrentKsn[i]);
  }
  TRACE("Bloque 1 cifrado: %s", encryptedTrackString);
  TRACE("KSN: %s", ksnString);

  strcat(ez.encryptedData, encryptedTrackString);
  strcat(ez.ksn, ksnString);

  ezCrc32 = Crc32b(encryptedTrackString);
  TRACE("CRC32 int : %d", ezCrc32);
  // strcat(ez.crc32, "00000000");
  snprintf(ez.crc32, 8 + 1, "%08X", ezCrc32);
  TRACE("CRC32 char: %s", ez.crc32);
  TRACE("Tamaño CRC32 %d", strlen(ez.crc32));

  char crx32Exc[8 + 1] = {0};

  memset(crx32Exc, '0', 8 + 1);

  if (strlen(ez.crc32) < 8) {
    int espacios = 8 - strlen(ez.crc32);
    memcpy(crx32Exc + espacios, ez.crc32, strlen(ez.crc32));
  } else {
    memcpy(crx32Exc, ez.crc32, strlen(ez.crc32));
  }
  crx32Exc[8] = 0x00;
  memcpy(ez.crc32, crx32Exc, strlen(crx32Exc));

  inEz = ChapX_GenerateEzToken(&ez, generatedEzToken, chReqEncryption);

  strcat(chTokenEZ, generatedEzToken);

  pdump(chTokenEZ, inEz, "EZ:");

  log_trace("Finish Token EZ");

  return inEz;
}

int inEYAssemble(char chTokenEY[],
                 char chTk1[],
                 unsigned char chcurrentKsn[],
                 unsigned char chB2[],
                 char chReqEncryption) {
  log_trace("EMPEZANDO TOKEN EY");

  char generatedEyToken[182 + 1] = {0};
  memset(generatedEyToken, 0, 182);
  memset(chTokenEY, 0, 182);
  char encryptedTrack1String[160 + 1] = {0};
  char crc32[8 + 1] = {0};
  int lentrack1 = strlen(chTk1);
  int inEy;

  if (lentrack1 > 0 && chReqEncryption == ENCRYPT_DATA) {
    unsigned int eyCrc32 = 0;

    int i;
    char ksnString[20 + 1] = {0};
    for (i = 0; i < 80; i++) {
      sprintf(encryptedTrack1String, "%s%02X", encryptedTrack1String, chB2[i]);
    }
    for (i = 0; i < 10; i++) {
      sprintf(ksnString, "%s%02X", ksnString, chcurrentKsn[i]);
    }
    TRACE("Track 1 cifrado: %s", encryptedTrack1String);
    TRACE("KSN: %s", ksnString);

    eyCrc32 = Crc32b(encryptedTrack1String);
    TRACE("CRC32 int : %d", eyCrc32);
    // strcat(ez.crc32, "00000000");
    snprintf(crc32, 8 + 1, "%08X", eyCrc32);
    TRACE("CRC32 char: %s", crc32);

    char crx32Exc[8 + 1];

    memset(crx32Exc, '0', 8 + 1);

    if (strlen(crc32) < 8) {
      int espacios = 8 - strlen(crc32);
      memcpy(crx32Exc + espacios, crc32, strlen(crc32));
    } else {
      memcpy(crx32Exc, crc32, strlen(crc32));
    }
    crx32Exc[8] = 0x00;
    memcpy(crc32, crx32Exc, strlen(crx32Exc));
  } else {
    log_trace("NO EXISTE TK1 O BIN SE ENCUENTRA EN TABLA DE BINES");
    lentrack1 = 0;
  }

  inEy = GenerateEyToken(lentrack1, encryptedTrack1String, crc32,
                         generatedEyToken);

  strcat(chTokenEY, generatedEyToken);

  pdump(chTokenEY, inEy, "EY:");

  log_trace("TERMINO TOKEN EY");

  return inEy;
}

int inCZAssemble(char chTokenCZ[]) {
  log_trace("GENERANDO TOKEN CZ");
  unsigned char tag9F36[2 + 1] = {0};
  unsigned char tag9F36inASCII[4 + 1] = {0};
  unsigned char tag9F6E[20 + 1] = {0};
  unsigned char tag9F6EinASCII[50] = {0};
  unsigned char tag9F06[20 + 1] = {0};
  unsigned char tag9F6EBytes[16 + 1] = {0};
  unsigned char tag9F06inASCII[20 + 1] = {0};
  unsigned char generatedCzToken[50 + 1];
  int retTag9F36;
  int retTag9F6E;
  int retTag9F06;

  int inCz;

  memset(generatedCzToken, 0x00, 51);

  retTag9F36 = NAPI_L3GetData(0x9F36, tag9F36, 5);
  if (retTag9F36 > 0) {
    TRACE("Si existe tag 9F36(ATM)");
    PubHexToAsc(tag9F36, (retTag9F36 * 2), 0, tag9F36inASCII);
    TRACE("Tag9F36: %s  ,len 9F36(ATM) [%d]", tag9F36inASCII, retTag9F36 * 2);
  } else {
    TRACE("No existe tag 9F36(ATM)");
  }

  retTag9F06 = NAPI_L3GetData(_EMV_TAG_9F06_TM_AID, tag9F06, 100);
  if (retTag9F06 > 0) {
    TRACE("Si existe tag 9F06(AID)");
    TRACE("len 0x9F36(AID) [%d]", retTag9F06);
    TRACE_HEX(tag9F06, retTag9F06, "tag 9F06(AID) ");
    PubHexToAsc(tag9F06, (retTag9F06 * 2), 0, tag9F06inASCII);
    TRACE("tag9F06inASCII [%s]", tag9F06inASCII);

  } else {
    TRACE("No existe tag 9F06(AID)");
  }

  retTag9F6E = NAPI_L3GetData(0x9F6E, tag9F6E, 100);
  if (retTag9F6E > 0) {
    TRACE("Si existe tag 0x9F6E");
    TRACE("len 0x9F6E [%d]", retTag9F6E);

    if (strcmp(tag9F06inASCII, "A0000000041010") == 0) {
      TRACE("TARJETA CTL MASTERCARD");
      if (retTag9F6E >= 6) {
        PubHexToAsc(tag9F6E, (retTag9F6E * 2), 0, tag9F6EinASCII);
        TRACE("IN ASCII: %s", tag9F6EinASCII);
        memcpy(tag9F6EBytes, tag9F6EinASCII + 8, 4);
        strcat(tag9F6EBytes, "0000");
        TRACE("TAG 0x9F6E: %s , len: %d", tag9F6EBytes, strlen(tag9F6EBytes));
        // memcpy(tag9F6EBytes + 4, "0000", 4);
      } else {
        TRACE("TARJETA CTL MASTERCARD NO TIENE LONGITUD VALIDA EN TAG 0x9F6E");
      }
    } else if (strcmp(tag9F06inASCII, "A0000000031010") == 0) {
      TRACE("TARJETA CTL VISA");
      if (retTag9F6E >= 4) {
        PubHexToAsc(tag9F6E, (retTag9F6E * 2), 0, tag9F6EinASCII);
        memcpy(tag9F6EBytes, tag9F6EinASCII, 8);
        TRACE("TAG 0x9F6E: %s , len: %d", tag9F6EBytes, strlen(tag9F6EBytes));
      } else {
        TRACE("TARJETA CTL VISA NO TIENE LONGITUD VALIDA EN TAG 0x9F6E");
      }
    } else {
      log_trace("NO SOPORTA TOKEN CZ");
    }

  } else {
    log_trace("No existe tag 0x9F6E");
  }

  inCz = GenerateCzToken(tag9F36inASCII, tag9F6EBytes, generatedCzToken);

  strcat(chTokenCZ, generatedCzToken);

  pdump(chTokenCZ, inCz, "CZ:");

  log_trace("TERMINO TOKEN CZ");

  return inCz;
}

int GenerateCzToken(unsigned char tag9F36[],
                    unsigned char tag9F6E[],
                    char *output) {
  char czToken[50 + 1] = {0};
  int len = 0;

  memset(czToken, 0x00, 51);
  memset(output, 0x00, sizeof output);

  TRACE("tag9F36: %d , tag9F36E: %d", strlen(tag9F36), strlen(tag9F6E));

  if (strlen(tag9F36) != 0 && strlen(tag9F6E) != 0) {
    len = sprintf(czToken, "! CZ00040 %s%s                            ",
                  tag9F36, tag9F6E);
  } else if (strlen(tag9F36) != 0 && strlen(tag9F6E) == 0) {
    len = sprintf(czToken, "! CZ00040 %s                                    ",
                  tag9F36);
  } else {
    len =
        sprintf(czToken, "! CZ00040                                         ");
  }
  memset(output, 0x00, sizeof output);
  log_trace("len: %d , len CZ: %d", len, strlen(czToken));
  log_trace("Token CZ: %s ", czToken);
  strcat(output, czToken);
  log_trace("TokenCZ in ASCII: %s", output);

  log_trace("TokenCZ generado");

  return len;
}

int GenerateEyToken(int lentrack1,
                    char track1Cifrado[],
                    char CRC32[],
                    char *output) {
  char eyToken[182 + 1] = {0};
  int len = 0;

  log_trace("Track 1 Length: 00%d", lentrack1);
  TRACE("CRC32: %s", CRC32);

  if (lentrack1 == 0) {
    len = sprintf(eyToken, "! EY00000 ");

  } else {
    len =
        sprintf(eyToken, "! EY00172 00%d%s%s", lentrack1, track1Cifrado, CRC32);
  }

  TRACE("len EY: %d", len);
  strcat(output, eyToken);
  TRACE("TkenEY in ASCII: %s", eyToken);

  log_trace("Token EZ generado");

  return len;
}

int ChapX_GenerateEzToken(struct ChapX_EzToken *ez,
                          char *output,
                          char reqEncryption) {
  char ezToken[108 + 1] = {0};
  unsigned int len = 0;

  if (ez->ksn == NULL) {
    TRACE("KSN nulo");
  }

  if (ez->encryptedData == NULL) {
    TRACE("Datos cifrados nulos");
  }

  log_trace("Generando token EZ");
  /*log_trace("Convirtiendo bytes de KSN a cadena HEX");
  ChapX_ConvertHexToString(ez->ksn, 10, &hexKsn);
  log_trace("Convirtiendo bytes de CRC32 a cadena HEX");
  ChapX_ConvertHexToString(ez->crc32, 4, &hexCrc32);*/
  log_trace("Req Encription : %d", reqEncryption);
  log_trace("KSN: %s", ez->ksn);
  log_trace("Cipher count: %07u", ez->cipherCount);
  log_trace("Failed cipher: %02u", ez->failedCipherCount);
  log_trace("Track 2 Flag: %01X", ez->track2Flag);
  log_trace("POS Entry Mode: %s", ez->posEntryMode);
  log_trace("Track 2 Length: %02u", ez->track2Length);
  log_trace("CVV Flag: %c", ez->cvvFlag);
  log_trace("CVV Length: %02u", ez->cvvLength);
  log_trace("Track 1 Flag: %01X", ez->track1Flag);
  log_trace("Encrypted data: %s", ez->encryptedData);
  log_trace("PAN last 4: %s", ez->panLast4Digits);
  log_trace("CRC32: %s", ez->crc32);

  len =
      sprintf(ezToken, "! EZ00098 %s%07u%02u%01X%s%02u%c%02u%01X%s%s%s",
              ez->ksn, ez->cipherCount, ez->failedCipherCount, ez->track2Flag,
              ez->posEntryMode, ez->track2Length, ez->cvvFlag, ez->cvvLength,
              ez->track1Flag, ez->encryptedData, ez->panLast4Digits, ez->crc32);

  TRACE("Len token EZ: %d ", len);

  if (len == 108 && reqEncryption == ENCRYPT_DATA) {
    strcat(output, ezToken);
    log_trace("Token EZ generado");
    return len;
  } else {
    memset(output, 0, 108);
    len = sprintf(ezToken, "! EZ00000 ");
    TRACE("Len token EZ: %d ", len);
    strcat(output, ezToken);
    log_trace("Token EZ generado");
    return len;
  }
}

int inStatusAssemble(char pchStatus[], int status) {
  log_trace("inStatusAssemble : status[%d]", status);

  /*
00 = Command successful
01 = Invalid command code
02 = Invalid data format
03 = Response has more packs
04 = Previous step missing
05 = Invalid Configuration
06 = Timed out
07 = Timer error
08 = Operation Cancelled
09 = Communication Error
10 = Chip Reader Failure
11 = Unrecognized Return Code

20 = Use Chip Reader
21 = Use Mag Stripe
22 = Chip Error
23 = Card Removed
24 = Card Blocked
25 = Card not supported

97 = Unrecognized Return Code
98 = Unexpected - Should not ocurr
99 = Command failed
*/

  switch (status) {
    case RET_CMD_OK:
      memcpy(pchStatus, "00", 2);
      break;
    case RET_CMD_INVALID_PARAM:
      memcpy(pchStatus, "02", 2);
      break;

    case RET_CMD_EMV_TIMEOUT:
      memcpy(pchStatus, "06", 2);
      break;
    case RET_CMD_CARD_REMOVE:
      memcpy(pchStatus, "23", 2);
      break;
    case RET_CMD_EMV_FAILURE:
      memcpy(pchStatus, "10", 2);
      break;
    case RET_CMD_USE_MAG_CARD:
      memcpy(pchStatus, "21", 2);
      break;
    // 2 L3 EMV STATUS
    case L3_TXN_DECLINE:
      memcpy(pchStatus, "99", 2);
      break;
    case L3_TXN_APPROVED:  // Offline
      memcpy(pchStatus, "00", 2);
      break;
    case L3_TXN_ONLINE:
      memcpy(pchStatus, "00", 2);
      break;
    case L3_TXN_TRY_ANOTHER:
      memcpy(pchStatus, "99", 2);
      break;
    case L3_ERR_TIMEOUT:
      memcpy(pchStatus, "06", 2);
      break;
    case L3_ERR_CANCEL:
      //			memcpy(pchStatus , "08" , 2);
      memcpy(pchStatus, "99", 2);
      break;
    case L3_ERR_COLLISION:
      memcpy(pchStatus, "99", 2);
      break;
    case L3_ERR_REMOVE_INTERRUPT:
      memcpy(pchStatus, "23", 2);
      break;
    case (-10) /*EMVL2_ERR_TERMINATE*/:
      memcpy(pchStatus, "25", 2);
      break;

    case L3_ERR_ACTIVATE:
    case L3_ERR_KERNEL_ERR:
    case L3_ERR_FAIL:
    case L3_ERR_TRY_AGAIN:
      memcpy(pchStatus, "10", 2);
      break;

      // AnnexBStatus

      /*case kSuccess:
      memcpy(pchStatus, "00", 2);
      break;*/

    case kInvalidMsg:
      memcpy(pchStatus, "01", 2);
      break;

      /*case kInvalidDataFormat:
      memcpy(pchStatus, "02", 2);
      break;*/

    case kTimeOut:
      memcpy(pchStatus, "06", 2);
      break;

    case kIccReadError:
      memcpy(pchStatus, "10", 2);
      break;
    case kManualEntry:
      memcpy(pchStatus, "20", 2);
      break;

    case kUseMagStripe:
      memcpy(pchStatus, "21", 2);
      break;

    case kCtlss:
      memcpy(pchStatus, "22", 2);
      break;

    case kCardRemoved:
      memcpy(pchStatus, "23", 2);
      break;

    case kCardUnsupported:
      memcpy(pchStatus, "25", 2);
      break;

    case kInvalidApp:
      memcpy(pchStatus, "26", 2);
      break;

    case kInvalidOperatorCard:
      memcpy(pchStatus, "27", 2);
      break;

    case kCardExpired:
      memcpy(pchStatus, "29", 2);
      break;

    case kInvalidDate:
      memcpy(pchStatus, "30", 2);
      break;

    case kCrcMismatch:
      memcpy(pchStatus, "50", 2);
      break;

    case kBadCheckValue:
      memcpy(pchStatus, "51", 2);
      break;

    case kNonExistentKey:
      memcpy(pchStatus, "52", 2);
      break;

    case kHsmCryptoError:
      memcpy(pchStatus, "53", 2);
      break;

    case kCipherLimitReached:
      memcpy(pchStatus, "54", 2);
      break;

    case kInvalidSignature:
      memcpy(pchStatus, "55", 2);
      break;
    case kRemoteLoadLengthError:
      memcpy(pchStatus, "56", 2);
      break;

    case kCommandNotAllowed:
      memcpy(pchStatus, "60", 2);
      break;

    case kKeysInitialized:
      memcpy(pchStatus, "61", 2);
      break;

    case kKeyInitNotPerformed:
      memcpy(pchStatus, "62", 2);
      break;

    case kReadError:
      memcpy(pchStatus, "63", 2);
      break;

    case kCardMismatch:
      memcpy(pchStatus, "64", 2);
      break;

    case kBadDataToCipher:
      memcpy(pchStatus, "65", 2);
      break;

    case kOtherFailure:
      memcpy(pchStatus, "99", 2);
      break;

    // 3 ----------------------------
    case RET_CMD_FAIL:
    default:
      memcpy(pchStatus, "99", 2);
      break;
  }

  return RET_CMD_OK;
}

/*---------------------------------------------------*/
// 1 CMD Tools
/*---------------------------------------------------*/

int iGetFieldFromInp(unsigned short *pusTag,
                     char **pucField,
                     char **pucMessage) {
  int iLen, iSizeBytes, i;
  unsigned short usTag;

  log_trace("iGetFieldFromInp");

  usTag = (unsigned short)**pucMessage;

  (*pucMessage)++;

  usTag <<= 8;

  if ((usTag & 0x1F00) == 0x1F00) {
    usTag += **pucMessage;

    (*pucMessage)++;
  }

  log_trace("usTag=0x%04x", usTag);

  if (pusTag != NULL) *pusTag = usTag;

  iLen = (int)**pucMessage;

  (*pucMessage)++;

  if (((uchar)iLen & 0x80) == 0x80) {
    iSizeBytes = iLen % 0x80;

    iLen = 0;
    for (i = 0; i < iSizeBytes; i++) {
      iLen *= 0x100;
      iLen += **pucMessage;
      (*pucMessage)++;
    }
  }

  log_trace("iLen=%d", iLen);

  if (iLen > 0) {
    if (pucField != NULL) {
      *pucField = *pucMessage;

      pdump(*pucField, iLen, "*pucField:");
    }
    (*pucMessage) += iLen;
  }

  log_trace("iLen=%d", iLen);

  return iLen;
}

void vdAddFieldToOut(unsigned short usTag,
                     unsigned char *pucField,
                     unsigned char ucSize,
                     unsigned char **pucOut,
                     int *piDataSize) {
  log_trace("vdAddFieldToOut");

  log_trace("tag=0x%04x", (int)usTag);

  pdump(pucField, ucSize, "val:");

  **pucOut = (uchar)(usTag >> 8);
  (*pucOut)++;
  (*piDataSize)++;

  if (usTag % 0x100) {
    **pucOut = (uchar)(usTag % 0x100);
    (*pucOut)++;
    (*piDataSize)++;
  }

  /*if (ucSize > 0x7F)
  {
      **pucOut = 0x81;
      (*pucOut)++;
      (*piDataSize)++;
  } */   //Daee 05/02/2021  //* comment length format BR *//

  **pucOut = ucSize;
  (*pucOut)++;
  (*piDataSize)++;

  if (ucSize > 0 && pucField != NULL) {
    memcpy(*pucOut, pucField, ucSize);
    pdump(*pucOut, ucSize, "*pucOut:");
    (*pucOut) += ucSize;
    (*piDataSize) += ucSize;
  }
}

/*---------------------------------------------------*/
// 1 File  Tools
/*---------------------------------------------------*/

void SVC_DSP_2_HEX(char *ascii_ptr, char *hex_ptr, int len) {
  int i;

  for (i = 0; i < (len); i++) {
    *(hex_ptr + i) = (*(ascii_ptr + (2 * i)) <= '9')
                         ? ((*(ascii_ptr + (2 * i)) - '0') * 16)
                         : (((*(ascii_ptr + (2 * i)) - 'A') + 10) << 4);
    *(hex_ptr + i) |= (*(ascii_ptr + (2 * i) + 1) <= '9')
                          ? (*(ascii_ptr + (2 * i) + 1) - '0')
                          : (*(ascii_ptr + (2 * i) + 1) - 'A' + 10);
  }
}

void SVC_HEX_2_DSP(char *hex, char *dsp, int n) {
  int bajo;
  int alto, i, j = 0;

  for (i = 0; i < n; i++) {
    bajo = ((hex[i] >> 4) & 0x0F);
    if (bajo < 10) {
      dsp[j] = bajo + '0';
    } else {
      dsp[j] = bajo + '7';
    }
    alto = ((hex[i]) & 0x0F);
    if (alto < 10) {
      dsp[j + 1] = alto + '0';
    } else {
      dsp[j + 1] = alto + '7';
    }
    j += 2;
  }
}

int inAscToHex(char pchAsc[], char pchHex[], int inAsc) {
  int inHex;

  log_trace("inAscToHex");

  pdump(pchAsc, inAsc, "asc:");

  inHex = inAsc >> 1;

  memset(pchHex, 0, inHex);

  SVC_DSP_2_HEX(pchAsc, pchHex, inHex);

  pdump(pchHex, inHex, "hex:");

  return inHex;
}

int inHexToAsc(char pchHex[], char pchAsc[], int inHex) {
  int inAsc;

  log_trace("inHexToAsc");

  pdump(pchHex, inHex, "hex:");

  inAsc = inHex << 1;

  memset(pchAsc, 0, inAsc);

  SVC_HEX_2_DSP(pchHex, pchAsc, inHex);

  pdump(pchAsc, inAsc, "asc:");

  return inAsc;
}

int ConvertStringToAscii(const char *string, unsigned char *output) {
  int loop;
  int i;

  i = 0;
  loop = 0;

  while (string[loop] != '\0') {
    sprintf((char *)(output + i), "%02X", string[loop]);
    loop += 1;
    i += 2;
  }
  // insert NULL at the end of the output string
  output[i++] = '\0';
  return 0;
}

void vdUTL_BinToLong(unsigned long *pulLong, unsigned char *pucBin) {
  log_trace("vdUTL_BinToLong");

  *pulLong = pucBin[0] * 0x1000000L + pucBin[1] * 0x10000L +
             pucBin[2] * 0x100L + pucBin[3];
}

void EnableDispDefault(void)
{
  //
}
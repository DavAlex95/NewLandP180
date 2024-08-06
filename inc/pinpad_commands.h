
#include <et_token.h>

#ifndef _COMANDOS_H_
#define _COMANDOS_H_

/*
 * @Author: Diego Escalera
 * @Date:   28/01/2021
 */

/*ParadoX Technologies
 */

// Command Type
#define CMD_C51 1
#define CMD_C54 2
#define CMD_C25 3
#define CMD_Z62 4
#define CMD_Z2 5
#define CMD_M10 6
#define CMD_M11 7
#define CMD_W11 8
#define CMD_C50 9
#define CMD_C14 10
#define CMD_C12 11

// Entry Mode
#define CMD_PEM_EMV 1
#define CMD_PEM_CTLS 2
#define CMD_PEM_MSR 3
#define CMD_PEM_MSR_FBK 4
#define CMD_PEM_KBD 5
#define CMD_PEM_UNDEF 6

#define TAG_C1_INPUT_PARAM 0xC100
#define TAG_C2_OUTPUT_PARAM 0xC200
#define TAG_E1_INPUT_PARAM 0xE100
#define TAG_E2_OUTPUT_PARAM 0xE200
#define TAG_91_ISS_AUTH_DATA 0x9100

// Errror Command
#define RET_CMD_OK 0
#define RET_CMD_FAIL -1
#define RET_CMD_INVALID_PARAM -200
#define RET_CMD_EMV_FAILURE -201
#define RET_CMD_EMV_TIMEOUT -202
#define RET_CMD_USE_MAG_CARD -203
#define RET_CMD_CARD_REMOVE -204

// AnnexB_Status
#define kSuccess 0
#define kInvalidMsg 1
#define kInvalidDataFormat 2
#define kTimeOut 6
#define kIccReadError 10
#define kManualEntry 20
#define kUseMagStripe 21
#define kCtlss 22
#define kCardRemoved 23
#define kCardUnsupported 25
#define kInvalidApp 26
#define kInvalidOperatorCard 27
#define kCardExpired 29
#define kInvalidDate 30
#define kCrcMismatch 50
#define kBadCheckValue 51
#define kNonExistentKey 52
#define kHsmCryptoError 53
#define kCipherLimitReached 54
#define kInvalidSignature 55
#define kRemoteLoadLengthError 56
#define kCommandNotAllowed 60
#define kKeysInitialized 61
#define kKeyInitNotPerformed 62
#define kReadError 63
#define kCardMismatch 64
#define kBadDataToCipher 65
#define kOtherFailure 99

// Host or referral decisions
#define HOST_AUTHORISED 1
#define HOST_DECLINED 2
#define FAILED_TO_CONNECT 3
#define REFERRAL_AUTHORISED 4
#define REFERRAL_DECLINED 5

#define CRD_EXC_NAM 8
#define CRD_EXC_VER 2

// Requires Encryption
#define ENCRYPT_DATA 0x01
#define NO_ENCRYPT_DATA 0x00

// Requires New Key
#define KEY_REQUEST_REQUIRED 0x01
#define NO_KEY_REQUEST_REQUIRED 0x00

typedef struct TxnCommonData {
  int inCommandtype;

  // Request
  int inLongitud;
  int inTimeOut;       // In seconds
  char chDate[5 + 1];  // hex YYMMDD
  char chTime[5 + 1];  // hex HHMMSS
  int inTranType;
  int inFlagTransType;
  int inCvvReq;
  int inExpDateReq;
  char chAmount[12 + 1];       // BCD Str
  char chOtherAmount[12 + 1];  // BCD Str
  char chCurrCode[5 + 1];      // hex xxxx
  int inMerchantDesicion;
  unsigned int uinE1P1[256];
  unsigned int uinE1P2[256];
  unsigned int uinE2[256];
  int inPanMasking;
  int inFlagPanOrBin;
  int inHostDec;
  char chAuthCode[10];    // str
  char chRspCode[5 + 1];  // str
  char chIssAuthData[16 + 1];
  int inIssAuthData;

  char chScriptEMV[512];
  int inScriptEMV;

  int inuE1P1;
  int inuE1P2;
  int inuE2;

  // Response
  int status;
  char chPan[30];          // BCD Str
  char chLast4Pan[4 + 1];  // BCD Str
  char chExp[10];          // BCD Str
  char chHld[50];          // str
  char chTk2[50];          // str
  char chTk1[80];          // str
  char chCv2[10];
  char chMod[5];

  int inPan;
  int inExp;
  int inHld;
  int inTk2;
  int inTk1;
  int inCv2;
  int inMod;

  char chE1[512];
  char chE2[512];
  int inE1;
  int inE2;

  char cPinBlock;
  char chReqEncryption;

  short shEntryMode;

  // Kernel Emv
  char chTlvCompleteTxn[512];
  int inTlvCompleteTxn;
} TxnCommonData;

enum TransactionType {
  GenericTransExclAmex = 1,
  RefundExclAmex = 2,
  GenericTransInclAmex = 3,
  RefundInclAmex = 4,
  QpsSaleExclAmex = 5,
  GenericTransCtlss = 7
};

struct ChapX_EzToken {
  unsigned char ksn[20 + 1]; /*!< Key Serial Number de DUKPT */
  unsigned int cipherCount;  /*!< Contador de cifrados exitosos de PIN pad. */
  unsigned int
      failedCipherCount;     /*!< Contador de cifrados fallidos de PIN pad. */
  unsigned short track2Flag; /*!< Indica si track 2 está presente. */
  unsigned char posEntryMode[2 + 1]; /*!< Modo de ingreso de la tarjeta. */
  unsigned short track2Length;       /*!< Longitud del track 2 en claro. */
  unsigned char cvvFlag;             /*!< Indica si el CVV está presente. */
  unsigned short cvvLength;          /*!< Longitud de CVV en claro. */
  unsigned short track1Flag;  /*!< Indica si el track 1 está presente. */
  char encryptedData[48 + 1]; /*!< Criptograma con datos sensibles. */
  unsigned char
      panLast4Digits[4 + 1]; /*!< Ultimos cuatro digitos de la tarjeta. */
  char crc32[8 + 1];         /*!< CRC32 sobre los datos sensibles cifrados. */
};

extern void SetPetitionFlagOfNewKey(char);
extern char GetPetitionFlagOfNewKey();

int CheckLowBattery();
int inProcessComand(void);  // Daee 28/01/2021
int inProcessCommandAsyn(char *psBuf, int *punLen);
int inPrepareCommandAsyn(char *psBuf, int *punLen);
int inSendByte(char byte);

int inParseProccessComand(char *chCmdRx,
                          int inCmdRx,
                          char *chCmdTx,
                          int *inCmdTx);

int inC51(char pch_snd[], int in_max, char pch_rcv[], int in_rcv);
int inC54(char pch_snd[], int in_max, char pch_rcv[], int in_rcv);
int inC25(char pch_snd[], int in_max, char pch_rcv[], int in_rcv);
int inI02(char pch_snd[], int in_max, char pch_rcv[], int in_rcv);
int inZ2(char pch_snd[], int in_max, char pch_rcv[], int in_rcv);
int inQ8(char pch_snd[], int in_max, char pch_rcv[], int in_rcv);
int inQ7(char pch_snd[], int in_max, char pch_rcv[], int in_rcv);

int inC51Disassemble(char pch_rcv[], int in_rcv, TxnCommonData *txnCommonData);
int inC53Assemble(char pch_snd[], int in_max, TxnCommonData *txnCommonData);
int inC54Disassemble(char pch_rcv[], int in_rcv, TxnCommonData *txnCommonData);
int inC54Assemble(char pch_snd[], int in_max, TxnCommonData *txnCommonData);
int inC25Disassemble(char pch_rcv[], int in_rcv, TxnCommonData *txnCommonData);
int inC25Assemble(char pch_snd[], int in_max, TxnCommonData *txnCommonData);
// int inC14Disassemble(char pch_rcv[], int in_rcv, struct EtToken *token);
int inC14Assemble(char pch_snd[], int in_max, int in_status);
int inC12Disassemble(char pch_rcv[], int in_rcv, int *type);
int inC12Assemble(char pch_snd[], int in_max, int status, int scriptType);

int inESAssemble(char pchQs[], int cipher, int fDukAsk);
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
                 char chReqEncryption);
int inEYAssemble(char chTokenEY[],
                 char chTk1[],
                 unsigned char chcurrentKsn[],
                 unsigned char chB2[],
                 char chReqEncryption);
int inCZAssemble(char chTokenCZ[]);
int GenerateCzToken(unsigned char tag9F36[],
                    unsigned char tag9F6E[],
                    char *output);
int GenerateEyToken(int lentrack1,
                    char track1Cifrado[],
                    char CRC32[],
                    char *output);
int ChapX_GenerateEzToken(struct ChapX_EzToken *ez,
                          char *output,
                          char reqEncryption);
int inStatusAssemble(char pchStatus[], int status);

int iGetFieldFromInp(unsigned short *pusTag,
                     char **pucField,
                     char **pucMessage);
void vdAddFieldToOut(unsigned short usTag,
                     unsigned char *pucField,
                     unsigned char ucSize,
                     unsigned char **pucOut,
                     int *piDataSize);

void SVC_DSP_2_HEX(char *ascii_ptr, char *hex_ptr, int len);
void SVC_HEX_2_DSP(char *hex, char *dsp, int n);
int inAscToHex(char pchAsc[], char pchHex[], int inAsc);
int inHexToAsc(char pchHex[], char pchAsc[], int inHex);
int ConvertStringToAscii(const char *string, unsigned char *output);
void vdUTL_BinToLong(unsigned long *pulLong, unsigned char *pucBin);

#endif

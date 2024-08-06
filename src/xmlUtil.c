/**************************************************************************
 * Copyright (C) 2019 Newland Payment Technology Co., Ltd All Rights Reserved
 *
 * @file  xml.c
 * @brief consider the limit of memory(MPOS), we load XML configuraion one line
 *by on line
 * @version 1.0
 * @author linld
 * @date 2019-9-25
 **************************************************************************/

#include "libapiinc.h"

#include "apiinc.h"
#include "appinc.h"



// ------------------------------------------------------------------
#define XML_EMV_CONFIG     "newland.xml"
#define XML_EMV_CONFIG_BK  "EmvCfg.BK"
#define XML_APP_CONFIG "AppCfg.xml"
#define XML_STRING  "String.xml"

// #define XML_EMV_CONFIGNFIG  "Newland_L3_configuration.xml"

extern int Xml_LoadEMVConfig(void);
extern int Xml_LoadAPPConfig(void);
extern int Xml_LoadString(void);
// ------------------------------------------------------------------



#define YES 1
#define NO 0

typedef struct {
  unsigned char Transtype;
  int nTLVListLen;
  unsigned char szTLVList[200];
} PaypassTranTypeAID;

#define NODE_CONFIG "config"
#define NODE_ENTRY "entry"
#define NODE_DRL "DRL"
#define NODE_CUSTOM_TAG "CUSTOM"
#define DRL_TYPE_PAYWAVE 1
#define DRL_TYPE_AMEX 2
static int gnPaypassAidControl = 0;


int GetConfigID(uchar *szKey);
int HashInsertConfig(int nKey, char *szVal, int lenszVal);


static int ParseAID(char *szLineBuf,
                    char *szAidTlv,
                    int *nTlvLen,
                    PaypassTranTypeAID *Tlvlist,
                    int *pnPaypassAidAddNum);
static int ParseCapk(char *szLineBuf, L3_CAPK_ENTRY *stCapk);

/**
 * @brief read one line from file
 * 		 [fgets] only get one line which is end with "0A"/"0D 0A" ,
 * 		 But, sometimes one line is end with "0D", so we make this api.
 * @params
 * @return 0
 * @author linld
 * @date 2019-9-26
 */

static int ReadLine(char *str, int n, int nFd) {
  int num = 0;
  int index = 0;

  while (1) {
    if (index >= n) {
      break;
    }
    num = PubFsRead(nFd, str + index, 1);
    if (num < 1) {
      break;
    }
    if (*(str + index) == 0x0D || *(str + index) == 0x0A) {
      if (index == 0)  // empty line
      {
        continue;
      } else {
        *(str + index) = 0x00;
      }
      return NO;
    }
    index++;
  }

  return YES;
}

static int Invaild(char *lineStr) {
  if (strstr(lineStr, "<!--") != NULL)  // Comments
  {
    return YES;
  }

  if (strstr(lineStr, "<?") != NULL)  // head
  {
    return YES;
  }

  return NO;
}

static int IsNode(char *node, char *str, char *lineStr) {
  char *p1 = NULL;
  char *p2 = NULL;

  char szContent[100] = {0};

  if (NULL == strstr(lineStr, node)) {
    return NO;
  }

  p1 = strchr(lineStr, '\"');
  if (p1 == NULL) {
    return NO;
  }

  p2 = strchr(p1 + 1, '\"');
  if (p2 == NULL) {
    return NO;
  }

  memcpy(szContent, p1 + 1, p2 - p1 - 1);

  PubAllTrim(szContent);

  if (strlen(szContent) != strlen(str)) {
    return NO;
  }

  if (0 == memcmp(szContent, str, strlen(str))) {
    return YES;
  }

  return NO;
}

static int IsNodeEnd(char *node, char *lineStr) {
  char szTmp[50] = {0};

  sprintf(szTmp, "</%s", node);

  if (strstr(lineStr, szTmp) != NULL) {
    return YES;
  }
  return NO;
}

static int IsElement(char *lineStr) {
  if (strstr(lineStr, "<item") != NULL) {
    return YES;
  }
  return NO;
}

static int IsCustomTag(char *lineStr) {
  if (strstr(lineStr, "CUSTOM TAG") != NULL) {
    return YES;
  }

  return NO;
}

static int IsDrlElement(char *lineStr, unsigned char *drlType) {
  if (strstr(lineStr, "PAYWAVE DRL") != NULL) {
    *drlType = DRL_TYPE_PAYWAVE;
    return YES;
  } else if (strstr(lineStr, "AMEX DRL") != NULL) {
    *drlType = DRL_TYPE_AMEX;
    return YES;
  }
  return NO;
}

static int GetAttribute(char *attr, char *out, char *lineStr) {
  char *p = NULL;
  char *p1 = NULL;
  char *p2 = NULL;

  p = strstr(lineStr, attr);
  if (p == NULL) {
    return -1;
  }

  p1 = strchr(p, '\"');
  if (p1 == NULL) {
    return -2;
  }

  p2 = strchr(p1 + 1, '\"');
  if (p2 == NULL) {
    return -3;
  }

  memcpy(out, p1 + 1, p2 - p1 - 1);
  PubAllTrim(out);
  return 0;
}

static int SetCustomTag(char *szLineBuf, EMVTagAttr *strEmvCusTag) {
  int nRet;
  char szKey[50] = {0};
  char szValue[100] = {0};
  char szHexVal[4] = {0};
  int nValLen = 0;

  memset(szKey, 0, sizeof(szKey));
  nRet = GetAttribute("key", szKey, szLineBuf);
  if (nRet != 0) {
    TRACE("GetAttribute fail, nRet=%d", nRet);
    return nRet;
  }

  memset(szValue, 0, sizeof(szValue));
  nRet = GetAttribute("value", szValue, szLineBuf);
  if (nRet != 0) {
    TRACE("GetAttribute fail, nRet=%d", nRet);
    return nRet;
  }

  TRACE("[%s]:[%s]", szKey, szValue);
  nValLen = strlen(szValue);
  memset(szHexVal, 0, sizeof(szHexVal));
  PubAscToHex((unsigned char *)szValue, nValLen, 1, (unsigned char *)szHexVal);
  nValLen = (nValLen + 1) >> 1;

  if (0 == strcmp(szKey, "Tag name")) {
    PubCnToInt((uint *)&(strEmvCusTag->tag), (uchar *)szHexVal, nValLen);
  } else if (0 == strcmp(szKey, "Template1")) {
    PubCnToInt((uint *)&(strEmvCusTag->template1), (uchar *)szHexVal, nValLen);
  } else if (0 == strcmp(szKey, "Template2")) {
    PubCnToInt((uint *)&(strEmvCusTag->template2), (uchar *)szHexVal, nValLen);
  } else if (0 == strcmp(szKey, "Source")) {
    strEmvCusTag->source = szHexVal[0] & 0xFF;
  } else if (0 == strcmp(szKey, "Format")) {
    strEmvCusTag->format = szHexVal[0] & 0xFF;
  } else if (0 == strcmp(szKey, "Min lenth")) {
    strEmvCusTag->minLen = szHexVal[0] & 0xFF;
  } else if (0 == strcmp(szKey, "Max lenth")) {
    strEmvCusTag->maxLen = szHexVal[0] & 0xFF;
  }

  return 0;
}

static int SetPayWaveDrl(char *szLineBuf, STDRLPARAM *strPayWaveDrl) {
  char szKey[30] = {0};
  int nRet = 0;
  char szValue[80] = {0};
  char szHexVal[40] = {0};
  int nValLen = 0;

  memset(szKey, 0, sizeof(szKey));
  nRet = GetAttribute("key", szKey, szLineBuf);
  if (nRet != 0) {
    TRACE("GetAttribute fail, nRet=%d", nRet);
    return nRet;
  }

  memset(szValue, 0, sizeof(szValue));
  nRet = GetAttribute("value", szValue, szLineBuf);
  if (nRet != 0) {
    TRACE("GetAttribute fail, nRet=%d", nRet);
    return nRet;
  }

  TRACE("[%s]:[%s]", szKey, szValue);
  nValLen = strlen(szValue);
  memset(szHexVal, 0, sizeof(szHexVal));
  PubAscToHex((unsigned char *)szValue, nValLen, 1, (unsigned char *)szHexVal);
  nValLen = (nValLen + 1) >> 1;

  if (0 == strcmp(szKey, "APP ID")) {
    strPayWaveDrl->ucDrlAppIDLen = nValLen;
    memcpy(strPayWaveDrl->usDrlAppId, szHexVal, strPayWaveDrl->ucDrlAppIDLen);
  } else if (0 == strcmp(szKey, "ClLimitExist")) {
    strPayWaveDrl->ucClLimitExist = (szHexVal[0] & 0xFF);
  } else if (0 == strcmp(szKey, "Clss Transaction Limit")) {
    memcpy(strPayWaveDrl->usClTransLimit, szHexVal, 6);
  } else if (0 == strcmp(szKey, "Clss Offline Limit")) {
    memcpy(strPayWaveDrl->usClOfflineLimit, szHexVal, 6);
  } else if (0 == strcmp(szKey, "Cvm Limit")) {
    memcpy(strPayWaveDrl->usClCvmLimit, szHexVal, 6);
  }

  return 0;
}

static int SetAmexDrl(char *szLineBuf, STEXDRLPARAM *strAmexDrl) {
  char szKey[30] = {0};
  int nRet = 0;
  char szValue[60] = {0};
  char szHexVal[30] = {0};
  int nValLen = 0;

  memset(szKey, 0, sizeof(szKey));
  nRet = GetAttribute("key", szKey, szLineBuf);
  if (nRet != 0) {
    TRACE("GetAttribute fail, nRet=%d", nRet);
    return nRet;
  }

  memset(szValue, 0, sizeof(szValue));
  nRet = GetAttribute("value", szValue, szLineBuf);
  if (nRet != 0) {
    TRACE("GetAttribute fail, nRet=%d", nRet);
    return nRet;
  }

  TRACE("[%s]:[%s]", szKey, szValue);
  nValLen = strlen(szValue);
  memset(szHexVal, 0, sizeof(szHexVal));
  PubAscToHex((unsigned char *)szValue, nValLen, 1, (unsigned char *)szHexVal);

  if (0 == strcmp(szKey, "DRL Exist")) {
    strAmexDrl->ucDrlExist = atoi(szValue);
  } else if (0 == strcmp(szKey, "DRL ID")) {
    strAmexDrl->ucDrlId = (szHexVal[0] & 0xFF);
  } else if (0 == strcmp(szKey, "Limist Exist")) {
    strAmexDrl->ucClLimitExist = (szHexVal[0] & 0xFF);
  } else if (0 == strcmp(szKey, "Clss Transaction Limit")) {
    memcpy(strAmexDrl->usClTransLimit, szHexVal, 6);
  } else if (0 == strcmp(szKey, "Clss Offline Limit")) {
    memcpy(strAmexDrl->usClOfflineLimit, szHexVal, 6);
  } else if (0 == strcmp(szKey, "Cvm Limit")) {
    memcpy(strAmexDrl->usClCvmLimit, szHexVal, 6);
  }

  return 0;
}

static int AddTagOne(unsigned char *uCustagTlv,
                     EMVTagAttr *strCusTag,
                     int *nCusTagOffset) {
  int nEmvTagAttrLen = 0;

  TRACE("[nCusTagOffset] = %d", *nCusTagOffset);
  TRACE_HEX(strCusTag, sizeof(EMVTagAttr), "[Custom tag add]: ");

  nEmvTagAttrLen = sizeof(EMVTagAttr);
  memcpy(uCustagTlv + *nCusTagOffset, strCusTag, nEmvTagAttrLen);
  *nCusTagOffset += nEmvTagAttrLen;
  memset(strCusTag, 0, sizeof(EMVTagAttr));
  return 0;
}

static int AddDrlOne(unsigned char *ucDrlTlv,
                     STDRLPARAM *strPayWaveDrl,
                     STEXDRLPARAM *strAmexDrl,
                     unsigned char ucType,
                     int *nDrlOffset) {
  int nDrlstrLen = 0;
  TRACE("[nDrlOffset] = %d", *nDrlOffset);

  if (ucType == DRL_TYPE_PAYWAVE) {
    TRACE_HEX(strPayWaveDrl, sizeof(STDRLPARAM), "[Paywave drl add]: ");

    nDrlstrLen = sizeof(STDRLPARAM);
    memcpy(ucDrlTlv + *nDrlOffset, strPayWaveDrl, nDrlstrLen);
    *nDrlOffset += nDrlstrLen;
    memset(strPayWaveDrl, 0, sizeof(STDRLPARAM));
  } else if (ucType == DRL_TYPE_AMEX) {
    TRACE_HEX(strAmexDrl, sizeof(STEXDRLPARAM), "[Amex drl add]: ");

    nDrlstrLen = sizeof(STEXDRLPARAM);
    memcpy(ucDrlTlv + *nDrlOffset, strAmexDrl, nDrlstrLen);
    *nDrlOffset += nDrlstrLen;
    memset(strAmexDrl, 0, sizeof(STEXDRLPARAM));
  }

  return 0;
}

static int AddDrlAll(unsigned char *ucDrlTlv,
                     char *szAidTlv,
                     int *nTlvLen,
                     int nDrlOffset,
                     unsigned char ucDrlType) {
  TRACE("[nDrlOffset] = %d:", nDrlOffset);

  if (ucDrlType == DRL_TYPE_PAYWAVE) {
    TRACE_HEX(ucDrlTlv, nDrlOffset, "Paywave Drl [DF3F]: ");
    TlvAddEx((uchar *)"DF3F", nDrlOffset, (char *)ucDrlTlv, szAidTlv, nTlvLen);
  } else if (ucDrlType == DRL_TYPE_AMEX) {
    TRACE_HEX(ucDrlTlv, nDrlOffset, "Amex Drl [DF53]: ");
    TlvAddEx((uchar *)"DF53", nDrlOffset, (char *)ucDrlTlv, szAidTlv, nTlvLen);
  }

  return 0;
}

static int AddPaypassAidTlv(char *szHexTranstypevalue,
                            PaypassTranTypeAID *Tlvlist,
                            char *szTag,
                            int nValLen,
                            char *szHexVal,
                            int *pnPaypassAidAddNum) {
  int i = 0;
  TRACE("pnPaypassAidAddNum=%d", *pnPaypassAidAddNum);
  for (i = 0; i <= *pnPaypassAidAddNum; i++) {
    if (((Tlvlist[i].Transtype == szHexTranstypevalue[0]) &&
         (Tlvlist[i].nTLVListLen != 0))) {
      TlvAddEx((unsigned char *)szTag, nValLen, szHexVal,
               (char *)Tlvlist[i].szTLVList, &Tlvlist[i].nTLVListLen);
      TRACE("Tlvlist[i].nTLVListLen=%d", Tlvlist[i].nTLVListLen);
      TRACE_HEX(Tlvlist[i].szTLVList, Tlvlist[i].nTLVListLen,
                "Tlvlist[0].szTLVList: ");
      break;
    } else if (Tlvlist[i].nTLVListLen == 0) {
      Tlvlist[i].Transtype = szHexTranstypevalue[0];
      TlvAddEx((unsigned char *)szTag, nValLen, szHexVal,
               (char *)Tlvlist[i].szTLVList, &Tlvlist[i].nTLVListLen);
      TRACE_HEX(Tlvlist[i].szTLVList, Tlvlist[i].nTLVListLen,
                "Tlvlist[0].szTLVList: ");
      *pnPaypassAidAddNum += 1;
      break;
    }
  }

  return 0;
}

static int LoadPaypassTranTypeAID(char *szAidTlv,
                                  int nTlvLen,
                                  PaypassTranTypeAID *Tlvlist,
                                  int nPaypassAidAddNum) {
  int i = 0;
  char szNewAidTlv[1500] = {0};
  int nNEWTlvLen = 0;
  int nRet = 0;

  for (i = 0; i < nPaypassAidAddNum; i++) {
    memset(szNewAidTlv, 0, sizeof(szNewAidTlv));
    nNEWTlvLen = nTlvLen;
    memcpy(szNewAidTlv, szAidTlv, nNEWTlvLen);

    TlvAddEx((uchar *)"1F8101", 1, "\x01", szNewAidTlv, &nNEWTlvLen);
    TlvAddEx((uchar *)"DF7D", 1, (char *)&Tlvlist[i].Transtype, szNewAidTlv,
             &nNEWTlvLen);
    memcpy(szNewAidTlv + nNEWTlvLen, Tlvlist[i].szTLVList,
           Tlvlist[i].nTLVListLen);
    nNEWTlvLen += Tlvlist[i].nTLVListLen;
    TRACE_HEX(szNewAidTlv, nNEWTlvLen, "szNewAidTlv: ");

    nRet = NAPI_L3LoadAIDConfig(0x02, NULL, (unsigned char *)szNewAidTlv,
                                &nNEWTlvLen, CONFIG_UPT);
    TRACE("NAPI_L3LoadAIDConfig->CONFIG_UPT, nRet=%d", nRet);
  }
  gnPaypassAidControl = 0;
  return 0;
}

static int ParseAID(char *szLineBuf,
                    char *szAidTlv,
                    int *nTlvLen,
                    PaypassTranTypeAID *Tlvlist,
                    int *pnPaypassAidAddNum) {
  int nRet;
  char szTag[10];
  char szValue[500] = {0};
  char szHexVal[250] = {0};
  int nValLen = 0;
  char szTranstypevalue[2 + 1] = {0};
  char szHexTranstypevalue[2 + 1] = {0};
  int nTagvalueLen = 0;

  memset(szTag, 0, sizeof(szTag));
  nRet = GetAttribute("tag", szTag, szLineBuf);
  TRACE("ParseAID, szLineBuf=%s", szLineBuf);
  if (nRet != 0) {
    TRACE("GetAttribute fail, nRet=%d", nRet);
    return nRet;
  }

  memset(szValue, 0, sizeof(szValue));
  nRet = GetAttribute("value", szValue, szLineBuf);
  if (nRet != 0) {
    TRACE("GetAttribute fail, nRet=%d", nRet);
    return nRet;
  }
  nValLen = strlen(szValue);
  memset(szHexVal, 0, sizeof(szHexVal));
  PubAscToHex((uchar *)szValue, nValLen, 1, (uchar *)szHexVal);
  nValLen = (nValLen + 1) >> 1;

  nRet = GetAttribute("TransType", szTranstypevalue, szLineBuf);
  if (nRet == 0) {
    nTagvalueLen = strlen(szTranstypevalue);
    PubAscToHex((unsigned char *)szTranstypevalue, nTagvalueLen, 1,
                (unsigned char *)szHexTranstypevalue);
    TRACE("szHexTagvalue =0x%02x", szHexTranstypevalue[0]);
    AddPaypassAidTlv(szHexTranstypevalue, Tlvlist, szTag, nValLen, szHexVal,
                     pnPaypassAidAddNum);
    gnPaypassAidControl = 1;
    return 0;
  }

  TlvAddEx((uchar *)szTag, nValLen, szHexVal, szAidTlv, nTlvLen);
  return 0;
}

static int ParseCapk(char *szLineBuf, L3_CAPK_ENTRY *stCapk) {
  int nRet;
  char szKey[50] = {0};
  char szValue[500] = {0};
  char szHexVal[250] = {0};
  int nValLen = 0;

  memset(szKey, 0, sizeof(szKey));
  nRet = GetAttribute("key", szKey, szLineBuf);
  if (nRet != 0) {
    TRACE("GetAttribute fail, nRet=%d", nRet);
    return nRet;
  }

  memset(szValue, 0, sizeof(szValue));
  nRet = GetAttribute("value", szValue, szLineBuf);
  if (nRet != 0) {
    TRACE("GetAttribute fail, nRet=%d", nRet);
    return nRet;
  }

  nValLen = strlen(szValue);
  memset(szHexVal, 0, sizeof(szHexVal));
  PubAscToHex((uchar *)szValue, nValLen, 1, (uchar *)szHexVal);
  nValLen = (nValLen + 1) >> 1;

  if (0 == strcmp(szKey, "RID")) {
    memcpy(stCapk->rid, szHexVal, 5);
  } else if (0 == strcmp(szKey, "Index")) {
    stCapk->index = (szHexVal[0] & 0xFF);
  } else if (0 == strcmp(szKey, "Hash")) {
    memcpy(stCapk->hashValue, szHexVal, 20);
  } else if (0 == strcmp(szKey, "Exponent")) {
    if (0 == memcmp(szValue, "03", 2)) {
      stCapk->pkExponent[0] = 0x00;
      stCapk->pkExponent[1] = 0x00;
      stCapk->pkExponent[2] = 0x03;
    } else {
      memcpy(stCapk->pkExponent, szHexVal, 3);
    }
  } else if (0 == strcmp(szKey, "Modulus")) {
    stCapk->pkModulusLen = nValLen;
    memcpy(stCapk->pkModulus, szHexVal, stCapk->pkModulusLen);
  } else if (0 == strcmp(szKey, "Hash Algorithm")) {
    stCapk->hashAlgorithmIndicator = atoi(szValue);
  } else if (0 == strcmp(szKey, "Sign Algorithm")) {
    stCapk->pkAlgorithmIndicator = atoi(szValue);
  }

  return 0;
}

/**
 * @brief Load EMV configuration from XML file
 * @param NONE
 * @return
 * @li 0  success
 * @li < 0 fail
 * @author linld
 * @date 2019-9-26
 */
int Xml_LoadEMVConfig(void) {
  // FILE *fp = NULL;
  char szLineBuf[1000] = {0};
  int nAidInterface = 0;
  int nIsTerminalConfig = NO;
  int nConfigType = 0;
  int nRet = 0;
  char szAidTlv[1500] = {0};
  int nTlvLen = 0;
  L3_CAPK_ENTRY stCapk;
  int nFd = 0;
  int nEnd = 0;
  EMVTagAttr strEmvTag;
  STDRLPARAM strPayWaveDrl;
  STEXDRLPARAM strAmexDrl;
  // unsigned char ucAllPayWaveDrlEnd;
  // unsigned char ucAllAmexDrlEnd;
  unsigned char ucCusTagSetting = 0;
  unsigned char ucDrlIsSetting = 0;
  unsigned char ucDrlType = 0;
  int nTagOffset = 0;
  int nDrlOffset = 0;
  unsigned char uCustagTlv[280] = {0};
  unsigned char ucDrlTlv[500] = {0};
  PaypassTranTypeAID pTrasTypeAidTlv[10];
  int nPaypassAidAddNum = 0;
  int i = 0;
  if (PubFsExist(XML_EMV_CONFIG) != NAPI_OK) {
    return 0;
  }
  nFd = PubFsOpen(XML_EMV_CONFIG, "r");
  if (nFd < 0) {
    TRACE("open file[%s] failed[%d].", XML_EMV_CONFIG, nFd);
    return -1;
  }

  PubClearAll();
  PubDisplayStrInline(0, 2, tr("LOAD CONFIG"));
  PubDisplayStrInline(0, 3, tr("PLEASE WAIT..."));
  PubUpdateWindow();

  NAPI_L3LoadTerminalConfig(L3_CONTACT, NULL, NULL, CONFIG_FLUSH);
  NAPI_L3LoadTerminalConfig(L3_CONTACTLESS, NULL, NULL, CONFIG_FLUSH);
  NAPI_L3LoadAIDConfig(L3_CONTACT, NULL, NULL, NULL, CONFIG_FLUSH);
  NAPI_L3LoadAIDConfig(L3_CONTACTLESS, NULL, NULL, NULL, CONFIG_FLUSH);
  NAPI_L3LoadCAPK(NULL, CONFIG_FLUSH);

  nTlvLen = 0;

  memset(&strPayWaveDrl, 0, sizeof(STDRLPARAM));
  memset(&strAmexDrl, 0, sizeof(STEXDRLPARAM));
  memset(&strEmvTag, 0, sizeof(EMVTagAttr));
  memset(uCustagTlv, 0, sizeof(uCustagTlv));
  memset(ucDrlTlv, 0, sizeof(ucDrlTlv));

  memset(szAidTlv, 0, sizeof(szAidTlv));
  memset(&stCapk, 0, sizeof(stCapk));

  for (i = 0; i < 10; i++) {
    memset(&pTrasTypeAidTlv[i], 0, sizeof(PaypassTranTypeAID));
  }

  while (1) {
    if (nEnd == 1) {
      break;
    }
    memset(szLineBuf, 0, sizeof(szLineBuf));
    if (YES == ReadLine(szLineBuf, sizeof(szLineBuf), nFd)) {
      nEnd = 1;
    }
    if (YES == Invaild(szLineBuf)) {
      continue;
    }
    if (YES == IsNode(NODE_CONFIG, "CONTACT", szLineBuf)) {
      nConfigType = 1;
      nAidInterface = L3_CONTACT;
      continue;
    }
    if (YES == IsNode(NODE_CONFIG, "CONTACTLESS", szLineBuf)) {
      nConfigType = 1;
      nAidInterface = L3_CONTACTLESS;
      continue;
    }
    if (YES == IsNode(NODE_CONFIG, "PublicKeys", szLineBuf)) {
      nConfigType = 2;
      continue;
    }
    if (YES == IsNode(NODE_ENTRY, "Terminal Configuration", szLineBuf)) {
      nIsTerminalConfig = YES;
      continue;
    }
    if (YES == IsCustomTag(szLineBuf)) {
      ucCusTagSetting = 1;
      continue;
    }
    if (YES == IsDrlElement(szLineBuf, &ucDrlType)) {
      ucDrlIsSetting = 1;
      continue;
    }
    if (YES == IsElement(szLineBuf)) {
      if (ucCusTagSetting) {
        SetCustomTag(szLineBuf, &strEmvTag);
        continue;
      }
      if (ucDrlIsSetting) {
        if (ucDrlType == DRL_TYPE_PAYWAVE) {
          SetPayWaveDrl(szLineBuf, &strPayWaveDrl);
        } else if (ucDrlType == DRL_TYPE_AMEX) {
          SetAmexDrl(szLineBuf, &strAmexDrl);
        }
        continue;
      }

      if (nConfigType == 1)  // AID
      {
        nRet = ParseAID(szLineBuf, szAidTlv, &nTlvLen, pTrasTypeAidTlv,
                        &nPaypassAidAddNum);
      } else  // CAPK
      {
        nRet = ParseCapk(szLineBuf, &stCapk);
      }

      if (nRet != 0) {
        PubFsClose(nFd);
        return nRet;
      }
      continue;
    }

    if (YES == IsNodeEnd(NODE_CUSTOM_TAG, szLineBuf)) {
      AddTagOne(uCustagTlv, &strEmvTag, &nTagOffset);
      ucCusTagSetting = 0;
      continue;
    }

    if (YES == IsNodeEnd(NODE_DRL, szLineBuf)) {
      AddDrlOne(ucDrlTlv, &strPayWaveDrl, &strAmexDrl, ucDrlType, &nDrlOffset);
      ucDrlIsSetting = 0;
      continue;
    }

    if (YES == IsNodeEnd(NODE_ENTRY, szLineBuf)) {
      if (nTagOffset > 0) {
        TRACE("nTagOffset = %d", nTagOffset);
        TRACE_HEX(uCustagTlv, nTagOffset, "Custom Tag [1F811F]: ");
        TlvAddEx((uchar *)"1F811F", nTagOffset, (char *)uCustagTlv, szAidTlv,
                 &nTlvLen);
        nTagOffset = 0;
        ucCusTagSetting = 0;
      }

      if (nDrlOffset > 0) {
        AddDrlAll(ucDrlTlv, szAidTlv, &nTlvLen, nDrlOffset, ucDrlType);
        nDrlOffset = 0;
        ucDrlIsSetting = 0;
      }
      TRACE("nConfigType=%d", nConfigType);
      if (nConfigType == 1)  // AID
      {
        TRACE("nAidInterface=%d", nAidInterface);
        TRACE("nIsTerminalConfig=%d", nIsTerminalConfig);

        TRACE_HEX(szAidTlv, nTlvLen, "TLV CONFIG:");

        if (YES == nIsTerminalConfig) {
          nRet = NAPI_L3LoadTerminalConfig(nAidInterface, (uchar *)szAidTlv,
                                           &nTlvLen, CONFIG_UPT);
          TRACE("NAPI_L3LoadTerminalConfig->CONFIG_UPT,nRet=%d", nRet);

          nIsTerminalConfig = NO;
        } else {
          if (gnPaypassAidControl != 0) {
            LoadPaypassTranTypeAID(szAidTlv, nTlvLen, pTrasTypeAidTlv,
                                   nPaypassAidAddNum);
            nPaypassAidAddNum = 0;
          }
          nRet = NAPI_L3LoadAIDConfig(nAidInterface, NULL, (uchar *)szAidTlv,
                                      &nTlvLen, CONFIG_UPT);
          TRACE("NAPI_L3LoadAIDConfig->CONFIG_UPT, nRet=%d", nRet);
        }
        memset(szAidTlv, 0, sizeof(szAidTlv));
        memset(pTrasTypeAidTlv, 0, sizeof(pTrasTypeAidTlv));
        memset(uCustagTlv, 0, sizeof(uCustagTlv));
        memset(ucDrlTlv, 0, sizeof(ucDrlTlv));
        nTlvLen = 0;
      } else  // CAPK
      {
        nRet = NAPI_L3LoadCAPK(&stCapk, CONFIG_UPT);
        TRACE("NAPI_L3LoadCAPK->CONFIG_UPT,nRet=%d", nRet);

        memset(&stCapk, 0, sizeof(stCapk));
      }

      continue;
    }
  }

  PubFsClose(nFd);
  // PubFsRename(XML_EMV_CONFIG, XML_EMV_CONFIG_BK);
  return 0;
}

int Xml_LoadAPPConfig(void) {
  char szLineBuf[1000] = {0};
  uchar szKey[100] = {0};
  char szVal[200] = {0};
  int nKey = 0;
  int nLen = 0;
  int nRet = 0;
  int nFd = 0;

  if (PubFsExist(XML_APP_CONFIG) != NAPI_OK) {
    return APP_FAIL;
  }

  nFd = PubFsOpen(XML_APP_CONFIG, "r");
  if (nFd < 0) {
    TRACE("open file[%s] failed[%d].", XML_APP_CONFIG, nFd);
    return APP_FAIL;
  }
  while (1) {
    memset(szLineBuf, 0, sizeof(szLineBuf));
    if (YES == ReadLine(szLineBuf, sizeof(szLineBuf), nFd)) break;
    if (IsElement(szLineBuf) == YES) {
      memset(szKey, 0, sizeof(szKey));
      if (GetAttribute("key", szKey, szLineBuf) != APP_SUCC) {
        TRACE("GET CONFIG KEY ERR");
        continue;
      }
      memset(szVal, 0, sizeof(szVal));
      if (GetAttribute("value", szVal, szLineBuf) != APP_SUCC) {
        TRACE("GET CONFIG VALUE ERR");
        continue;
      }
      // TODO Get Config Key ID
      nKey = GetConfigID(szKey);
      if (nKey < 0) {
        TRACE("---Error config [%s]----", szKey);
        continue;
      }
      // TODO
      nRet = HashInsertConfig(nKey, szVal, strlen(szVal));
      if (nRet < 0) {
        TRACE("HASH TABLE IS FULL");
        return nRet;
      }
    } else
      continue;
  }
  PubFsClose(nFd);
  return APP_SUCC;
}

int Xml_LoadString(void) {
  char szLineBuf[1000] = {0};
  uchar szKey[100] = {0};
  char szVal[200] = {0};
  int nKey = 0;
  int nLen = 0;
  int nRet = 0;
  int nFd = 0;

  if (PubFsExist(XML_STRING) != NAPI_OK) return APP_FAIL;

  nFd = PubFsOpen(XML_STRING, "r");
  if (nFd < 0) {
    TRACE("open file[%s] failed[%d].", XML_STRING, nFd);
    return APP_FAIL;
  }

  while (1) {
    memset(szLineBuf, 0, sizeof(szLineBuf));
    if (YES == ReadLine(szLineBuf, sizeof(szLineBuf), nFd)) break;
    if (IsElement(szLineBuf) == YES) {
      memset(szKey, 0, sizeof(szKey));
      if (GetAttribute("key", szKey, szLineBuf) != APP_SUCC) {
        TRACE("GET STRING KEY ERR");
        continue;
      }
      memset(szVal, 0, sizeof(szVal));
      if (GetAttribute("value", szVal, szLineBuf) != APP_SUCC) {
        TRACE("GET STRING VALUE ERR");
        continue;
      }
      // TODO Get Config Key ID
      nKey = GetStringID(szKey);
      if (nKey < 0) {
        TRACE("---Error STRING [%s]----", szKey);
        continue;
      }
      // TODO
      nRet = HashUpdateString(nKey, szVal, strlen(szVal));
      if (nRet < 0) {
        TRACE("HASH TABLE IS FULL");
        return nRet;
      }
      // TRACE("---Insert [%s]----", tr2(szKey));
    } else
      continue;
  }
  PubFsClose(nFd);
  return APP_SUCC;
}

int GetConfigID(uchar *szKey)
{

}

int HashInsertConfig(int nKey, char *szVal, int lenszVal)
{

}

int GetStringID(uchar *szKey)
{

}

int HashUpdateString(int nKey, char *szVal, int lenszVal)
{

}




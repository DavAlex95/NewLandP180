#include "pinpad.h"
#include "lsecurity.h"
#include "ltool.h"
#include "lui.h"
#include "pinpadtool.h"
#include "unistd.h"
#include "emvconfig.h"
#include "softalg.h"
#include "napi.h"
#include "napi_app.h"

#include "napi_crypto.h"
#include "napi_sysinfo.h"
#include "napi_display.h"
#include "napi_smartcard.h"
#include "virtualpad.h"

#define SECURITYLIBVER			"ALSECU0117052301"
#define PINMAXLEN 				12

#define GREENFONT				0x3384
#define REDFONT					0xFACB
#define BLACKFONT				0x0000
#define WHITEFONT				0xFFFF
#define MONEYBACKGROUNDPNG		"money_background.png"	/*<background of money display:269*64 */
#define MONEYBACKGROUNDW		269						/*<height:269 */
#define MONEYBACKGROUNDH		50						/*<width:64-->50 */
#define BORDERW					8						/*<margin:8 */

#define DUKPT_KSN_LEN (10)
#define KEY_MAX_LEN   (48)
#define MAX_TIMEOUT   (200)
#define MIN_TIMEOUT   (5)

static int gnSecurityMode = SECRITY_MODE_INSIDE;   		/**< built-in or external pinpad*/
static int gnMainKeyIndex = 0;  						/**< current key index*/

static int gnTimeOut = MAX_TIMEOUT;						/**< timeout of built-in pinpad (5-200s)*/
static int gnYesOrNoInit = 0;							/**< panpad init flag 0: uninit, 1:init*/
static char gcKeySystem = SECRITY_KEYSYSTEM_MSKEY;		/**< default key system type*/

typedef struct {
	int (*pLoadKey)(int , int , const char *, int , char*);
	int (*pGetPinBlock)(char *, int *, int , int , int, const char *, int , int);
	int (*pCalMac)(char*, int, int, int, const char*, int);
	int (*pClrPinpad)(void);
	int (*pDispPinpad)(const char *, const char *, const char *, const char *);
	int (*pDespinpad)(const char *, int , char *, int , int );
	int (*pReadString)(char*, int*, int, int);
	int (*pClrKey)(void);
	int (*pPinpadBeep)(int, int);
	int (*pPinpadQR)(int, char*);
	int (*pPinpadDoScan)(char *);
	int (*pEsignature)(char *, char *, int );
	int (*pSwipeCard)(char *, char *);
	int (*pL3OrderSet)(char *, char *, int *);
	int (*pSetFontSize)(char );
	int (*pGetDukptKSN)(int , char *);
	int (*PIncDukptKSN)(int);
	int (*PGetKeyInfo)(EM_SEC_KEY_INFO_ID , uchar , EM_SEC_CRYPTO_KEY_TYPE , EM_SEC_KEY_USAGE ,
					  uchar *, uint , uchar *, int *);
	int (*PL3PerformSend)(char *);
	int (*PSendToPinpad)(char *, int );
	int (*PRecvFromPinpad)(char *, int , int *);	
}STPINPAD;

static STPINPAD gstPinpad;					/**< structure for callback funtions*/


static int ProInitPinpadParam(const STPINPADPARAM stPinpadParam)
{
	if(PubResetPinpad_PINPAD(stPinpadParam.cAux, stPinpadParam.cTimeout) != APP_SUCC)
	{
		return APP_FAIL;
	}
	memset(&gstPinpad, 0, sizeof(STPINPAD));
	gstPinpad.pLoadKey = PubLoadKey_PINPAD;
	gstPinpad.pGetPinBlock = PubGetPinBlock_PINPAD;
	gstPinpad.pCalMac = PubCalcMac_PINPAD;
	gstPinpad.pClrPinpad = PubClrPinPad_PINPAD;
	gstPinpad.pDispPinpad = PubDispPinPad_PINPAD;
	gstPinpad.pDespinpad = PubDesPinpad_PINPAD;
	gstPinpad.pClrKey = PubClearKey_PINPAD;
	gstPinpad.pPinpadBeep = PubBeep_PINPAD;
	gstPinpad.pPinpadQR = PubGenAndShowQr_PINPAD;
	gstPinpad.pPinpadDoScan = PubDoScan_PINPAD;
	gstPinpad.pEsignature = PubEsignature_PINPAD;
	gstPinpad.pSwipeCard = PubSwipeCard_PINPAD;
	gstPinpad.pL3OrderSet = PubL3OrderSet_PINPAD;
	gstPinpad.pSetFontSize = PubSetFontSize_PINPAD;
	gstPinpad.pGetDukptKSN = PubGetDukptKSN_PINPAD;
	gstPinpad.PIncDukptKSN = PubIncDukptKSN_PINPAD;
	gstPinpad.PGetKeyInfo = PubGetKeyInfo_PINPAD;
	gstPinpad.PL3PerformSend = PubL3PerformSend_PINPAD;
	gstPinpad.PSendToPinpad = PubSend_PINPAD;
	gstPinpad.PRecvFromPinpad = PubRecv_PINPAD;
	return APP_SUCC;
}

/**
* @brief Init security module
* @details Will check if the module is existed
* @param [in] nMode           ---Refer to EM_SECRITY_MODE
* @param [in] pstPinpadParam  ---Pinpad parameters
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author Liug
* @date 2012-5-23
*/
int PubInitSecrity(int nMode,const STPINPADPARAM* pstPinpadParam)
{
	int nRet;
	STPINPADPARAM stPinPadParam;

	gnYesOrNoInit = 0;
	memset(&stPinPadParam,0,sizeof(STPINPADPARAM));
	if (pstPinpadParam != NULL)
	{
		memcpy(&stPinPadParam,pstPinpadParam,sizeof(STPINPADPARAM));
	}

	switch (nMode)
	{
	case SECRITY_MODE_INSIDE:
		if(pstPinpadParam != NULL)
		{
			gnTimeOut = stPinPadParam.cTimeout;
		}

		if (gnTimeOut < MIN_TIMEOUT)
		{
			gnTimeOut = MIN_TIMEOUT;
		}
		else if (gnTimeOut > MAX_TIMEOUT)
		{
			gnTimeOut = MAX_TIMEOUT;
		}
		gnSecurityMode = SECRITY_MODE_INSIDE;
		gnYesOrNoInit = 1;
		break;
	case SECRITY_MODE_PINPAD:
		if (pstPinpadParam == NULL)
		{
			ProSetSecurityErrCode(ERR_PINPAD_PARAM,0);
			PINPAD_TRACE_SECU("PubInitSecrity nMode=SECRITY_MODE_PINPAD,but pstPinpadParam is NULL");
			return APP_FAIL;
		}
		nRet = ProInitPinpadParam(stPinPadParam);
		if (nRet != APP_SUCC)
		{
			ProSetSecurityErrCode(ERR_PINPAD_PININITERROR,0);
			PINPAD_TRACE_SECU("ProInitPinpadParam error");
			return APP_FAIL;
		}
		gnSecurityMode = SECRITY_MODE_PINPAD;
		gnYesOrNoInit = 1;
		PubClrPinPad();
		break;
	default:
		PINPAD_TRACE_SECU("ProInitPinpadParam nMode error");
		ProSetSecurityErrCode(ERR_PINPAD_NOSUPPORTMODE,0);
		return APP_FAIL;
	}
	PINPAD_TRACE_SECU("PubInitSecrity succ");

	return APP_SUCC;
}

static int ProCheckInit(void)
{
	if(gnYesOrNoInit == 0)
	{
		PubMsgDlg("Not yet Init","Please Init Pinpad", 2, 5);
		return APP_FAIL;
	}
	return APP_SUCC;
}

/**
* @brief Clear KEY
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author Liug
* @date 2012-5-23
*/
int PubClearKey(void)
{
	int nRet = -1;

	if(ProCheckInit() != APP_SUCC)
		return APP_FAIL;

	if(gnSecurityMode == SECRITY_MODE_INSIDE)
	{
		if ((nRet=NAPI_SecKeyErase()) != NAPI_OK)
		{
			ProSetSecurityErrCode(ERR_KEYERASE,nRet);
            PINPAD_TRACE_SECU("NAPI_SecKeyErase error nRet=%d",nRet);
			return APP_FAIL;
		}
		PINPAD_TRACE_SECU("PubClearKey SUCC");
		return APP_SUCC;
	}
	else
	{
		return gstPinpad.pClrKey();
	}
}

/**
* @brief Set the index of current main key.
* @param [in] nIndex Index of key
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author Liug
* @date 2012-5-23
*/
int PubSetCurrentMainKeyIndex(int nIndex)
{
	if(ProCheckInit() != APP_SUCC)
		return APP_FAIL;

	if(nIndex < 1 || nIndex > 250)
	{
		ProSetSecurityErrCode(ERR_PINPAD_PARAM,0);
		PINPAD_TRACE_SECU("nIndex error");
		return APP_FAIL;
	}
	gnMainKeyIndex = nIndex;
	PINPAD_TRACE_SECU("PubSetCurrentMainKeyIndex(%d) SUCC",nIndex);
	return APP_SUCC;
}

/**
* @brief Set the key system type.
* @param [in] nType Type of key system--EM_KEYSYSTEM
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author chenwu
* @date 2018-5-25
*/
int PubSetKeySystemType(int nType)
{
	if(ProCheckInit() != APP_SUCC)
		return APP_FAIL;
	
	if(nType != SECRITY_KEYSYSTEM_MSKEY && nType != SECRITY_KEYSYSTEM_DUKPT)
	{
		ProSetSecurityErrCode(ERR_PINPAD_PARAM,0);
		PINPAD_TRACE_SECU("nType error");
		return APP_FAIL;
	}
	gcKeySystem = nType;
	PINPAD_TRACE_SECU("PubSetCurrentKeySystemType(%d) SUCC", nType);
	return APP_SUCC;
}

int PubGetKeySystemType()
{
	return gcKeySystem;
}

/**
* @brief Install word key(calling 'PubSetCurrentMainKeyIndex' to set index before calling this api)
* @param [in] nKeyType      Key type---EM_KEY_TYPE
* @param [in] psKey         Buffer of the working key
* @param [in] nKeyLen       key length 16
* @param [in] psCheckValue  The buffer of key check value (3 bytes) or NULL
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author chenwu
* @date 2018-3-22
*/
int PubLoadWorkKey(int nKeyType,const char* psKey, int nKeyLen,const char* psCheckValue)
{
	int nRet;
	ST_SEC_KEYIN_DATA stKGData;
	ST_SEC_KCV_DATA stKcvData;

	if (ProCheckInit() != APP_SUCC)
	{
		return APP_FAIL;
	}

	if (SECRITY_KEYSYSTEM_DUKPT == gcKeySystem)
	{
        PINPAD_TRACE_SECU("SECRITY_KEYSYSTEM_MSKEY error : not support workkey!");
		ProSetSecurityErrCode(ERR_PINPAD_NOSUPPORT_WK,0);
		return APP_FAIL;
	}
	if(psKey == NULL)
	{
		PINPAD_TRACE_SECU("psKey error");
		ProSetSecurityErrCode(ERR_PINPAD_PARAM,0);
		return APP_FAIL;
	}

	if(nKeyLen != 16 && nKeyLen != 24) /* no support 8 bytes*/
	{
		PINPAD_TRACE_SECU("nKeyLen error");
		ProSetSecurityErrCode(ERR_PINPAD_PARAM,0);
		return APP_FAIL;
	}
	if(nKeyType != KEY_TYPE_MAC && nKeyType != KEY_TYPE_PIN && nKeyType != KEY_TYPE_DATA && nKeyType != KEY_TYPE_TMK)
	{
		PINPAD_TRACE_SECU("nKeyType error");
		ProSetSecurityErrCode(ERR_PINPAD_PARAM,0);
		return APP_FAIL;
	}
	if (gnSecurityMode == SECRITY_MODE_INSIDE)
	{
		memset(&stKGData, 0, sizeof(stKGData));
		memset(&stKcvData, 0, sizeof(stKcvData));
		stKGData.ucKEKIdx = gnMainKeyIndex;
		stKGData.KEKType = KEY_TYPE_DES;
		stKGData.KEKUsage = KEY_USE_KEK;
		stKGData.CipherMode = SEC_CIPHER_MODE_ECB;
		stKGData.ucKeyIdx  = gnMainKeyIndex;
		stKGData.KeyType = KEY_TYPE_DES;

		switch (nKeyType)
		{
		case KEY_TYPE_PIN:
			stKGData.KeyUsage = KEY_USE_PIN;
			break;
		case KEY_TYPE_MAC:
			stKGData.KeyUsage = KEY_USE_MAC;
			break;
		case KEY_TYPE_DATA:
			stKGData.KeyUsage = KEY_USE_DATA;
			break;
		case KEY_TYPE_TMK:
			stKGData.KeyUsage = KEY_USE_KEK;
			break;
		default:
			ProSetSecurityErrCode(ERR_PINPAD_PARAM,0);
            PINPAD_TRACE_SECU("nKeyType[%d] is invalid ", nKeyType);
			return APP_FAIL;
			break;
		}
		stKGData.pKeyData = (uchar *)psKey;
		stKGData.nKeyDataLen = nKeyLen;
		stKGData.nKeyLen = nKeyLen;
		stKcvData.nCheckMode = NAPI_SEC_KCV_NONE;
		stKcvData.nLen = 0;
		if (psCheckValue != NULL)
		{
			stKcvData.nCheckMode = NAPI_SEC_KCV_ZERO;
			stKcvData.nLen = 3;
			memcpy(stKcvData.sCheckBuf, psCheckValue, stKcvData.nLen);
		}
		nRet = NAPI_SecGenerateKey(SEC_KIM_CIPHER, &stKGData, &stKcvData);
		if (nRet != NAPI_OK)
		{
			ProSetSecurityErrCode(ERR_LOADKEY,nRet);
            PINPAD_TRACE_SECU("NAPI_SecGenerateKey error [nRet = %d]",nRet);
			return APP_FAIL;
		}
		PINPAD_TRACE_SECU("(%d,%d) INSIDE SUCC",gnMainKeyIndex,nKeyType);
		return APP_SUCC;
	}
	else
	{
		int nInDex;
		if (nKeyType == KEY_TYPE_DATA)
		{
			nInDex = 128 + gnMainKeyIndex;
		}
		else
		{
			nInDex = gnMainKeyIndex;
		}
		return gstPinpad.pLoadKey(nKeyType, nInDex, psKey, nKeyLen,(char *)psCheckValue);
	}
}

static void ProShowStar(int nPressKey, int nMaxLen)
{
    int i;
	char szStar[32]={0};
    
    for(i = 0 ; i < nPressKey ; i++)
    {
    	szStar[i] = '*' ;
    }

	PubDisplayStrInline(DISPLAY_MODE_CLEARLINE, 3, szStar);
	NAPI_ScrRefresh();
}

uchar num_btn[80]={
		0x02,0x00,0xF2,0x00,0x50,0x00,0x2C,0x01,
		0x52,0x00,0xF2,0x00,0xA0,0x00,0x2C,0x01,
		0xA2,0x00,0xF2,0x00,0xF0,0x00,0x2C,0x01,

		0x02,0x00,0x2E,0x01,0x50,0x00,0x68,0x01,
		0x52,0x00,0x2E,0x01,0xA0,0x00,0x68,0x01,
		0xA2,0x00,0x2E,0x01,0xF0,0x00,0x68,0x01,
		
		0x02,0x00,0x6A,0x01,0x50,0x00,0xA4,0x01,
		0x52,0x00,0x6A,0x01,0xA0,0x00,0xA4,0x01,
		0xA2,0x00,0x6A,0x01,0xF0,0x00,0xA4,0x01,

		0x52,0x00,0xA6,0x01,0xA0,0x00,0xE0,0x01,
	};

uchar func_key[36]={
	KEY_BACK, 0x00,0x00,0x00, 0xF2,0x00,0x2E,0x01,0x40,0x01,0x68,0x01,
	KEY_ENTER,0x00,0x00,0x00, 0xF2,0x00,0x6A,0x01,0x40,0x01,0xA4,0x01,
	KEY_ESC,  0x00,0x00,0x00, 0xF2,0x00,0xF2,0x00,0x40,0x01,0x2C,0x01
};

int ActivePIN(L3_PIN_TYPE type, char *pszPan, publicKey *pinPK)
{
	int nRet = 0;
	EM_SEC_CRYPTO_KEY_TYPE KeyType = KEY_TYPE_DES;
	ST_NAPI_RSA_KEY stRsaKey, *pstRsaKey = NULL;
	EM_SEC_VPP_SESSION_TYPE SessionType;
	NAPI_EM_SEC_PIN emPinMode = NAPI_SEC_PIN_ISO9564_0;

	if (PubGetKbAttr() == KB_VIRTUAL) {
		uchar out_seq[11] = {0};  /* Sequence of Keys */
			
		nRet = NAPI_SecVppTpInit(num_btn, func_key, out_seq);
		if (nRet != NAPI_OK)
		{
			ProSetSecurityErrCode(ERR_GETPIN,nRet);
			PINPAD_TRACE_SECU("NAPI_SecVppTpInit error,nRet=%d",nRet);
			return nRet;
		}
		PINPAD_TRACE_SECU("out_seq = %s", out_seq);
		Virtual_KbCreate((char *)out_seq, INPUT_MODE_PASSWD);
	}

	if (type == L3_PIN_ONLINE)
	{
		if (gcKeySystem == SECRITY_KEYSYSTEM_MSKEY)
		{
			SessionType = SEC_VPP_MASTER_SESSION;
		}
		else
		{
			SessionType = SEC_VPP_DUKPT;
		}
	}
	else
	{
		emPinMode = NAPI_SEC_PIN_ISO9564_2;
		pszPan = NULL;
		if (type == L3_PIN_OFFLINE)
		{
			SessionType = SEC_VPP_EMV_OFFLINE_CLEARPIN;
		}
		else
		{
		    if (pinPK == NULL)
		    {
		        return -1;
		    }

			SessionType = SEC_VPP_EMV_OFFLINE_ENCPIN;
			PINPAD_TRACE_SECU("pinPK->pk_mod_len=%#x", pinPK->modulusLen);
			//TRACE_HEX(pinPK->modulus, pinPK->modulusLen, "pstPinPK->pk_modulus:");
			//TRACE_HEX(pinPK->exponent, 3, "pstPinPK->pk_exponent:");
			stRsaKey.usBits = pinPK->modulusLen*8;
			memcpy(stRsaKey.sModulus, pinPK->modulus, pinPK->modulusLen);
			memcpy(stRsaKey.sExponent, pinPK->exponent, 3);
            pstRsaKey = &stRsaKey;
		}
	}

	nRet = NAPI_SecVPPInit(SessionType, KeyType, gnMainKeyIndex, pszPan, emPinMode, gnTimeOut, pstRsaKey, NULL, 0);
	if(SessionType == SEC_VPP_DUKPT && nRet == NAPI_ERR_SECKM_KEY_ALREADY_USED)
	{
		PubDukptIncreaseKSN();
		nRet = NAPI_SecVPPInit(SessionType, KeyType, gnMainKeyIndex, pszPan, emPinMode, gnTimeOut, pstRsaKey, NULL, 0);
	}
	PINPAD_TRACE_SECU("NAPI_SecVPPInit %d gnMainKeyIndex %d, nRet %d\n", nRet, gnMainKeyIndex, nRet);
	if (nRet != NAPI_OK)
	{
		ProSetSecurityErrCode(ERR_GETPIN,nRet);
		PINPAD_TRACE_SECU("NAPI_SecVPPInit error,nRet=%d",nRet);
		return nRet;
	}

	return NAPI_OK;
}

static int DetectIcstatus()
{
	int nRet, nStatus;

	nRet = NAPI_SCGetStatus(&nStatus);
	if (nRet != NAPI_OK)
	{
		return APP_FAIL;
	}
	PubDebug("nRet = %d nStatus = %d", nRet, nStatus);
	nRet = nStatus & 0x0B;
	switch(nRet)
	{
	case 0x01: // IC1_EXIST
	case 0x01 | 0x02:
		return APP_SUCC;
	default:
		return APP_FAIL;
	}
	return APP_FAIL;
}

static int ProGetPin(char *psPin, int *pnPinLen, int nMode, const char *pszCardno,const char *pszAmount,
              int nMaxLen, int nMinLen)
{
	int nRet,i;
	char szExpPinLenIn[100+1];
	char szDispAmt[64+1];
	int nPressKey = 0;
	int nFlag=0;
	char szConvert[30]= {0};
	char sPin[20];
	uchar szKsn[20] = {0};
	int nKsnLen = 0;
	char szCardNo[19+1] = {0};
	int nEvent, nOutPinLen;
	char cEntryMode = 0x00;
	
	if (gnSecurityMode == SECRITY_MODE_INSIDE)
	{
		if(nMode == PINTYPE_PLAIN || nMode == PINTYPE_PLAIN_STAR)
		{
			ProSetSecurityErrCode(ERR_PINPAD_NOSUPPORTMODE,0);
			PINPAD_TRACE_SECU("nMode error");
			return APP_FAIL;
		}

		memset(szDispAmt, 0, sizeof(szDispAmt));
		if(pszAmount != NULL && 0 != strlen(pszAmount))
		{
			if (memcmp(pszAmount, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 12) == 0)
			{
				;
			}
			else
			{
				PubConvAmount(	NULL,(uchar *)pszAmount,2,(uchar *)szDispAmt);
			}

			PubClearAll();
			sprintf(szConvert,"%s",szDispAmt);
			PubDisplayStrInline(0|DISPLAY_MODE_TAIL, 1, szConvert);
			PubDisplayStrInline(0, 2, "Please enter PIN:");
			PubDisplayStrInline(0, 4, "press [ENTER] to skip");
			PubUpdateWindow();
		}
		else
		{
			PubClear2To4();
			PubDisplayStrInline(0, 2, "Please enter PIN:");
			PubDisplayStrInline(0, 4, "press [ENTER] to skip");		
			PubUpdateWindow();
		}
 
		/**< Password length (Can be separated as 0,4,6) */
		strcpy(szExpPinLenIn,"0");
		if (nMinLen <= 0)
			nMinLen=1;//avoid multiple 0
		for (i=nMinLen; i<=nMaxLen; i++ )
		{
			sprintf(szExpPinLenIn+strlen(szExpPinLenIn),",%d",i);
		}

		if(nMode == PINTYPE_WITHPAN)
		{
			strcpy(szCardNo, pszCardno);
		}
		else
		{
			strcpy(szCardNo, "0000000000000000");
		}
		PubSysMsDelay(200);
		nRet = ActivePIN(L3_PIN_ONLINE, szCardNo, NULL);
		if (nRet != NAPI_OK)
		{
			return nRet;
		}

		nPressKey = 0;
		NAPI_L3GetData(L3_DATA_POS_ENTRY_MODE, &cEntryMode, 1);
		while(1)
		{
			if(cEntryMode == 0x05 && DetectIcstatus() != APP_SUCC) //ATTR_CONTACT = 0x05
			{
				PubDebug("remove card and quit");
				_FINISH_PIN_INPUT_THREAD(L3_ERR_REMOVE_INTERRUPT);
			}
			nRet = NAPI_SecVPPGetEvent(&nEvent, (uchar *)sPin, &nOutPinLen, szKsn, &nKsnLen);
			if (nRet == NAPI_ERR_SECVP_PIN_TOO_SHORT)
			{
				continue;
			}
			if (nRet != NAPI_OK)
			{
				ProSetSecurityErrCode(ERR_GETPINRESULT,nRet);
		        PINPAD_TRACE_SECU("NAPI_SecVPPGetEvent error,nRet=%d", nRet);
				if (nRet == NAPI_ERR_TIMEOUT
					|| nRet == NAPI_ERR_SECVP_TIMED_OUT)
				{
					_FINISH_PIN_INPUT_THREAD(L3_ERR_CANCEL);
				}
				_FINISH_PIN_INPUT_THREAD(L3_ERR_FAIL);
			}
			switch(nEvent)
			{
			case NAPI_SEC_VPP_KEY_PIN:
			case NAPI_SEC_VPP_KEY_BACKSPACE:
			case NAPI_SEC_VPP_KEY_CLEAR:
		        nPressKey = sPin[0] & 0x0F;
		        ProShowStar(nPressKey, PINMAXLEN);
				break;
			case NAPI_SEC_VPP_KEY_ENTER:
				PINPAD_TRACE_SECU("nStatus=SEC_VPP_KEY_ENTER,nRet = %d",nRet);
				nFlag = 1;
				break;
			case NAPI_SEC_VPP_KEY_ESC:
				PINPAD_TRACE_SECU("ESC:user quit");
				*pnPinLen = 0;
				return APP_QUIT;
				break;
			default:
				break;
			}
			if(nFlag == 1)
			{
				break;
			}
		}
        
		memcpy(psPin, sPin, 8);
		if(memcmp(psPin, "\x00\x00\x00\x00\x00\x00\x00\x00", 8) == 0)
			*pnPinLen = 0;
		else
			*pnPinLen = 8;
		PINPAD_TRACE_SECU("PubGetPin SUCC");
		return APP_SUCC;
	}
	else
	{
		nRet = gstPinpad.pGetPinBlock(psPin, pnPinLen, nMode, gcKeySystem, gnMainKeyIndex, pszCardno, strlen(pszCardno), nMaxLen);
		PubClrPinPad();
		return nRet;
	}
}



/**
* @brief Get PIN-Block(calling 'PubSetCurrentMainKeyIndex' to set index before calling this api)
* @param [out] psPin         Pointer to PIN-block buffer
* @param [out] pnPinLen      Pointer to PIN-block-length buffer
* @param [in]  nMode         EM_PIN_TYPE
* @param [in]  pszCardno     Card Number/(NULL)
* @param [in]  pszAmount     Amount(used for display, e.g. buff:000000001234-->display:12.34)
* @param [in]  nMaxLen       Maximum PIN length (4-12)
* @param [in]  nMinLen       Minimun PIN length
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author chenwu
* @date 2018-3-22
*/
int PubGetPin(char *psPin, int *pnPinLen, int nMode, const char *pszCardno,const char *pszAmount,
              int nMaxLen, int nMinLen)
{	
	if(ProCheckInit() != APP_SUCC)
		return APP_FAIL;
	if(psPin == NULL || pnPinLen == NULL || nMinLen < 0 || nMaxLen <= 0 || nMinLen > nMaxLen)
	{
		ProSetSecurityErrCode(ERR_PINPAD_PARAM,0);
		PINPAD_TRACE_SECU("incorrect params");
		return APP_FAIL;
	}
	if(nMode < PINTYPE_WITHPAN || nMode > PINTYPE_PLAIN)
	{
		ProSetSecurityErrCode(ERR_PINPAD_PARAM,0);
		PINPAD_TRACE_SECU("incorrect params");
		return APP_FAIL;
	}
	if(nMode == PINTYPE_WITHPAN && pszCardno == NULL)
	{
		ProSetSecurityErrCode(ERR_PINPAD_PARAM,0);
		PINPAD_TRACE_SECU("incorrect params");
		return APP_FAIL;
	}	

	return ProGetPin(psPin, pnPinLen, nMode, pszCardno, pszAmount, nMaxLen, nMinLen);
}

/**
* @brief Calculate MAC(calling 'PubSetCurrentMainKeyIndex' to set index before calling this api)
* @param [in]  nMode         Method---EM_MAC_TYPE
* @param [in]  psData        Data that needs to be calculated
* @param [in]  nDataLen      Date length
* @param [out] psMac         MAC
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author chenwu
* @date 2018-3-22
*/
int PubCalcMac(int nMode,const char *psData, int nDataLen, char * psMac)
{
    int nMacType;
	int nMacLen;
	int nRet = 0;
	uchar sKsnOut[20];
	int nOutKsnLen;

	if (ProCheckInit() != APP_SUCC)
	{
		return APP_FAIL;
	}

	if (psData == NULL || psMac == NULL || nDataLen < 0)
	{
		ProSetSecurityErrCode(ERR_PINPAD_PARAM,0);
		PINPAD_TRACE_SECU("PubCalcMac incorrect params");
		return APP_FAIL;
	}

	PINPAD_TRACE_SECU("gnMainKeyIndex = %d, nMode = %d,nDataLen = %d",gnMainKeyIndex,nMode,nDataLen);
	
	if (gnSecurityMode == SECRITY_MODE_INSIDE)
	{
		if(SECRITY_KEYSYSTEM_MSKEY == gcKeySystem) //SEC_MAC_DUKPT_LAST
		{
			switch (nMode)
			{
			case MAC_TYPE_ECB:
				nMacType = SEC_MAC_TDES_UNIONPAY_ECB;
				break;
			case MAC_TYPE_X99:
				nMacType = SEC_MAC_TDES_X99;
				break;
			case MAC_TYPE_X919:
				nMacType = SEC_MAC_TDES_X919;
				break;
			case MAC_TYPE_9606:
				nMacType = SEC_MAC_TDES_LAST;
				break;
			case MAC_TYPE_AES_X99:
				nMacType = SEC_MAC_AES_X99;
				break;
			case MAC_TYPE_AES_X9606:
				nMacType = SEC_MAC_AES_X99;
				break;
			default:
                PINPAD_TRACE_SECU("nMode error ");
				ProSetSecurityErrCode(ERR_PINPAD_PARAM,0);
				return APP_FAIL;
				break;
			}
		}
		else
		{
			switch (nMode)
			{
			case MAC_TYPE_ECB:
				nMacType = SEC_MAC_DUKPT_UNIONPAY_ECB;
				break;
			case MAC_TYPE_X99:
				nMacType = SEC_MAC_DUKPT_X99;
				break;
			case MAC_TYPE_X919:
				nMacType = SEC_MAC_DUKPT_X919;
				break;
			case MAC_TYPE_9606:
				nMacType = SEC_MAC_DUKPT_LAST;
				break;
			case MAC_TYPE_AES_X99:
				nMacType = SEC_MAC_AES_DUKPT_X99;
				break;
			case MAC_TYPE_AES_X9606:
				nMacType = SEC_MAC_AES_DUKPT_LAST;
				break;
			case MAC_TYPE_ECB_RESP:
				nMacType = SEC_MAC_DUKPT_RESP_UNIONPAY_ECB;
				break;
			case MAC_TYPE_X99_RESP:
				nMacType = SEC_MAC_DUKPT_RESP_X99;
				break;
			case MAC_TYPE_X919_RESP:
				nMacType = SEC_MAC_DUKPT_RESP_X919;
				break;
			case MAC_TYPE_9606_RESP:
				nMacType = SEC_MAC_DUKPT_RESP_LAST;
				break;
			case MAC_TYPE_AES_X99_RESP:
				nMacType = SEC_MAC_AES_DUKPT_RESP_X99;
				break;
			case MAC_TYPE_AES_X9606_RESP:
				nMacType = SEC_MAC_AES_DUKPT_RESP_LAST;
				break;
			default:
                PINPAD_TRACE_SECU("nMode error ");
				ProSetSecurityErrCode(ERR_PINPAD_PARAM,0);
				return APP_FAIL;
				break;
			}
		}
		nMacLen = 0;
        nRet = NAPI_SecGenerateMAC((EM_SEC_MAC_TYPE)nMacType, gnMainKeyIndex, NULL, 0, (uchar *)psData, nDataLen, NULL, 0,
					(uchar *)psMac, &nMacLen, sKsnOut, &nOutKsnLen);
		if (nRet != NAPI_OK)
		{
			ProSetSecurityErrCode(ERR_GETMAC,nRet);
            PINPAD_TRACE_SECU("NAPI_SecGenerateMAC error =%d",nRet);
			return APP_FAIL;
		}

        PINPAD_TRACE_SECU("PubCalcMac SUCC");
		return APP_SUCC;
	}
	else
	{
		switch (nMode)
		{
		case MAC_TYPE_X99:
			nMacType = PINPAD_X99;
			break;
		case MAC_TYPE_X919:/**<X919 :X99+3DES(last 8 bytes)*/
			nMacType = PINPAD_X919;
			break;
		case MAC_TYPE_ECB:
			nMacType = PINPAD_ECB;
			break;
		case MAC_TYPE_9606:
			nMacType = PINPAD_9606;
			break;	
		default:
			PINPAD_TRACE_SECU("nMode error ");
			ProSetSecurityErrCode(ERR_PINPAD_PARAM,0);
			return APP_FAIL;
			break;
		}
		return gstPinpad.pCalMac(psMac, gcKeySystem, nMacType, gnMainKeyIndex, psData, nDataLen);
	}
}

/**
* @brief Calculate KCV
* @param[in] KeyType		 Key Type
* @param[in] KeyUsage		 Key Usage
* @param[in] ucKeyID 		 Key index, 1~250
* @param[out] psKcv			 Pointer to the output buffer
* @param[out] pnKcvLen		 Pointer to the output length
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author wison
* @date 2022-5-9
*/
int PubGetKcv(uchar ucKeyID, EM_SEC_CRYPTO_KEY_TYPE KeyType, EM_SEC_KEY_USAGE KeyUsage, uchar *psKcv, int *pnKcvLen)
{
	int nRet = 0;

	if (ProCheckInit() != APP_SUCC)
	{
		return APP_FAIL;
	}

	PINPAD_TRACE_SECU("KeyType = %d, KeyUsage = %d", KeyType, KeyUsage);
	
	if (gnSecurityMode == SECRITY_MODE_INSIDE)
	{
		nRet = NAPI_SecGetKeyInfo(SEC_KEY_INFO_KCV, ucKeyID, KeyType, KeyUsage, NULL, 0, psKcv, pnKcvLen);
		if (nRet != NAPI_OK)
		{
			 PINPAD_TRACE_SECU("NAPI_SecGetKeyInfo fail %d", nRet);
			return APP_FAIL;
		}

        PINPAD_TRACE_SECU("NAPI_SecGetKeyInfo SUCC");
		return APP_SUCC;
	}
	else
	{
		return gstPinpad.PGetKeyInfo(SEC_KEY_INFO_KCV, ucKeyID, KeyType, KeyUsage, NULL, 0, psKcv, pnKcvLen);
	}
}


/**
* @brief Increase KSN
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author chenwu
* @date 2018-5-25
*/
int PubDukptIncreaseKSN(void)
{
	int nRet;
    ST_SEC_KEYIN_DATA stKGData;
    ST_SEC_KCV_DATA stKcvData;

    memset(&stKGData, 0, sizeof(stKGData));
    memset(&stKcvData, 0, sizeof(stKcvData));

	if(gnSecurityMode == SECRITY_MODE_INSIDE)
	{
		stKGData.ucKEKIdx = gnMainKeyIndex;
		stKGData.ucKeyIdx  = gnMainKeyIndex;
		stKGData.KEKType = KEY_TYPE_DES;
		stKGData.KEKUsage = KEY_USE_DUKPT;
		stKGData.KeyType = KEY_TYPE_DES;
		stKGData.KeyUsage = KEY_USE_DUKPT;
		stKGData.CipherMode = SEC_CIPHER_MODE_ECB;
		stKcvData.nCheckMode = NAPI_SEC_KCV_NONE;
		stKcvData.nLen = 0;
		nRet = NAPI_SecGenerateKey(SEC_KIM_DUKPT_DERIVE, &stKGData, &stKcvData);
		if(nRet != NAPI_OK)
		{
            PINPAD_TRACE_SECU("NAPI_SecIncreaseDukptKsn error = %d",nRet);
			return APP_FAIL;
		}
	}
	else
	{
		return gstPinpad.PIncDukptKSN(gnMainKeyIndex);
	}
    PINPAD_TRACE_SECU("PubDukptIncreaseKSN SUCC");
	return APP_SUCC;
}

/**
* @brief Get KSN
* @param [out] psKSN    KSN data---10byte
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author chenwu
* @date 2018-5-25
*/
int PubGetDukptKSN(char *psKSN)
{
	int nRet, nKSNLen;
	EM_SEC_CRYPTO_KEY_TYPE KeyType;
	EM_SEC_KEY_USAGE KeyUsage;

	if(psKSN == NULL)
	{
		ProSetSecurityErrCode(ERR_PINPAD_PARAM,0);
        PINPAD_TRACE_SECU("psKSN == NULL");
		return APP_FAIL;
	}
	if(gnSecurityMode == SECRITY_MODE_INSIDE)
	{
		KeyType = KEY_TYPE_DES;
		KeyUsage = KEY_USE_DUKPT;
		nRet = NAPI_SecGetKeyInfo(SEC_KEY_INFO_KSN, (uchar)gnMainKeyIndex, KeyType, KeyUsage, NULL, 0, (uchar *)psKSN, &nKSNLen);
		if(nRet != NAPI_OK)
		{
            PINPAD_TRACE_SECU("NAPI_SecGetDukptKsn error = %d",nRet);
			return APP_FAIL;
		}
	}
	else
	{
		nRet = gstPinpad.pGetDukptKSN(gnMainKeyIndex, psKSN);
	}
	PINPAD_TRACE_SECU("PubGetDukptKSN SUCC");
	return APP_SUCC;
}

/**
* @brief Display string on pinpad
* @param [in]  pszLine1  String displayed in line 1
* @param [in]  pszLine2  String displayed in line 2
* @param [in]  pszLine3  String displayed in line 3
* @param [in]  pszLine4  String displayed in line 4
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author chenwu
* @date 2018-3-22
*/
int PubDispPinPad(const char *pszLine1, const char *pszLine2, const char *pszLine3, const char *pszLine4)
{
	if(ProCheckInit() != APP_SUCC)
		return APP_FAIL;

	if(pszLine1 == NULL && pszLine2 == NULL && pszLine3 == NULL && pszLine4 == NULL)
	{
		PINPAD_TRACE_SECU("pszData error ");
		ProSetSecurityErrCode(ERR_PINPAD_PARAM,0);
		return APP_FAIL;
	}

	if(gnSecurityMode == SECRITY_MODE_INSIDE)
	{
		return APP_SUCC;
	}
	else
	{
		return gstPinpad.pDispPinpad(pszLine1, pszLine2, pszLine3, pszLine4);
	}
}

/**
* @brief Clear the message and display default info on pinpad(for PinPad)
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author Liug
* @date 2012-5-23
*/
int PubClrPinPad(void)
{
	if(ProCheckInit() != APP_SUCC)
		return APP_FAIL;

	if(gnSecurityMode == SECRITY_MODE_INSIDE)
	{
		return APP_SUCC;
	}
	else
	{
		return gstPinpad.pClrPinpad();
	}
}

/**
* @brief Do TDES encryption with specified 'nKeyType' key(calling 'PubSetCurrentMainKeyIndex' to set index before calling this api)
* @param [in]  nLen          Length of data
* @param [in]  psSrc         Source data
* @param [out] psDest        Target data
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author chenwu
* @date 2018-3-22
*/
int PubDes3(const char* psSrc, int nLen, char *psDest)
{
	int nRet;
	ST_SEC_ENCRYPTION_DATA encData;
	int encDataLen = 0;
	uchar encKsnData[20] = {0}; /**< Max Len 20 */
	int encKsnDataLen = 0;

	if(ProCheckInit() != APP_SUCC)
	{
		return APP_FAIL;
	}
	if(psSrc == NULL || psDest == NULL || nLen <= 0)
	{
        PINPAD_TRACE_SECU("psSrc /psDest error ");
		ProSetSecurityErrCode(ERR_PINPAD_PARAM,0);
		return APP_FAIL;
	}

    PINPAD_TRACE_SECU("gnMainKeyIndex=%d", gnMainKeyIndex);
	if (gnSecurityMode == SECRITY_MODE_INSIDE)
	{
		memset(&encData, 0, sizeof(encData));
		encData.ucKeyID = gnMainKeyIndex;
		encData.PaddingMode = SEC_PADDING_NONE;
		encData.psDataIn = (uchar *)psSrc;
		encData.unDataInLen = nLen;

		if (gcKeySystem == SECRITY_KEYSYSTEM_MSKEY)
		{
			encData.CipherType = SEC_CIPHER_DES_ECB; /* only support 3des*/
			encData.KeyUsage = KEY_USE_DATA;
		}
		else
		{
			encData.CipherType = SEC_CIPHER_DUKPT_ECB_BOTH;
			encData.KeyUsage = KEY_USE_DUKPT;
		}

		nRet = NAPI_SecEncryption(&encData, (uchar *)psDest, &encDataLen, encKsnData, &encKsnDataLen);
		if (nRet != NAPI_OK)
		{
            PINPAD_TRACE_SECU("NAPI_SecEncryption error nRet = %d",nRet);
			ProSetSecurityErrCode(ERR_CALCDES, nRet);
			return APP_FAIL;
		}

        PINPAD_TRACE_SECU("PubDes3 SUCC");
		//TRACE_HEX((char*)encKsnData, encKsnDataLen,"MSKEY [%s] DES3 len [%d] ksnLen [%d] ksn :",
		//gcKeySystem == SECRITY_KEYSYSTEM_MSKEY ? "YES" : "NO", encDataLen, encKsnDataLen);

		return APP_SUCC;
	}
	else
	{
		return gstPinpad.pDespinpad(psSrc, nLen, psDest, gnMainKeyIndex, DESMODE_3DES);
	}
	return APP_FAIL;
}

/**
* @brief Do TDES decryption with specified 'nKeyType' key(calling 'PubSetCurrentMainKeyIndex' to set index before calling this api)
* @param [in]  psSrc         Source data
* @param [in]  nLen          Length of data
* @param [out] psDest        Target data
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author chenwu
* @date 2018-3-22
*/
int PubUnDes3(const char *psSrc, int nLen, char *psDest)
{
	int nRet;
	ST_SEC_ENCRYPTION_DATA decData;
	int decDataLen = 0;
	uchar decKsnData[20] = {0}; /**< max len 20 */
	int decKsnDataLen = 0;

	if(ProCheckInit() != APP_SUCC)
	{
		return APP_FAIL;
	}
	if(psSrc == NULL || psDest == NULL)
	{
        PINPAD_TRACE_SECU("psSrc /psDest error ");
		ProSetSecurityErrCode(ERR_PINPAD_PARAM,0);
		return APP_FAIL;
	}

    PINPAD_TRACE_SECU("gnMainKeyIndex=%d", gnMainKeyIndex);
	if (gnSecurityMode == SECRITY_MODE_INSIDE)
	{
		memset(&decData, 0 ,sizeof(decData));
		decData.ucKeyID = gnMainKeyIndex;
		decData.PaddingMode = SEC_PADDING_NONE;
		decData.psDataIn = (uchar *)psSrc;
		decData.unDataInLen = nLen;
		if (gcKeySystem == SECRITY_KEYSYSTEM_MSKEY)
		{
			decData.CipherType = SEC_CIPHER_DES_ECB; /* only support 3des*/
			decData.KeyUsage = KEY_USE_DATA;
		}
		else
		{
			decData.CipherType = SEC_CIPHER_DUKPT_ECB_RESP;
			decData.KeyUsage = KEY_USE_DUKPT;
		}
		nRet = NAPI_SecDecryption(&decData, (uchar *)psDest, &decDataLen, decKsnData, &decKsnDataLen);
		if (nRet != NAPI_OK)
		{
            PINPAD_TRACE_SECU("NAPI_SecDecryption error nRet = %d",nRet);
			ProSetSecurityErrCode(ERR_CALCDES,nRet);
			return APP_FAIL;
		}
        PINPAD_TRACE_SECU("PubUnDes3 SUCC");
		//TRACE_HEX((char*)decKsnData, decKsnDataLen,"MSKEY [%s] undes declen [%d] decKsnLen [%d] ksn :",
		//	gcKeySystem == SECRITY_KEYSYSTEM_MSKEY ? "YES" : "NO", decDataLen, decKsnDataLen);
		return APP_SUCC;
	}
	else
	{
		return gstPinpad.pDespinpad(psSrc, nLen, psDest, gnMainKeyIndex, DESMODE_3UNDES);
	}
}

int PubSoftDes(const char* psKey,const char* psSrc,char *psDest)
{
	int nRet;

	if(psKey == NULL || psSrc == NULL || psDest == NULL)
	{
		PINPAD_TRACE_SECU("psKey/psSrc/psDest error");
		ProSetSecurityErrCode(ERR_PINPAD_PARAM,0);
		return APP_FAIL;
	}
	nRet = Soft_AlgTDes((uchar*)psSrc,(uchar*)psDest,(uchar*)psKey,8,SALG_TDS_MODE_ENC);
	if(nRet != SALG_OK)
	{
		ProSetSecurityErrCode(ERR_ALGDES,nRet);
        PINPAD_TRACE_SECU("NAPI_AlgTDes error = %d",nRet);
		return APP_FAIL;
	}
	PINPAD_TRACE_SECU("PubSoftDes SUCC");
	return APP_SUCC;
}
/**
* @brief Do soft-TDES encryption with 'psKey'.
* @param [in]  psKey         Key string
* @param [in]  psSrc         Source data---8byte
* @param [out] psDest        Target data---8byte
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author Liug
* @date 2012-5-23
*/
int PubSoftDes3(const char* psKey, const char* psSrc, char *psDest)
{
	int nRet;

	if(psKey == NULL || psSrc == NULL || psDest == NULL)
	{
		ProSetSecurityErrCode(ERR_PINPAD_PARAM,0);
		PINPAD_TRACE_SECU("psKey/psSrc/psDest error");
		return APP_FAIL;
	}
	nRet = Soft_AlgTDes((uchar*)psSrc,(uchar*)psDest,(uchar*)psKey,16,SALG_TDS_MODE_ENC);
	if (nRet != SALG_OK)
	{
		ProSetSecurityErrCode(ERR_ALGDES,nRet);
        PINPAD_TRACE_SECU("NAPI_AlgTDes error = %d",nRet);
		return APP_FAIL;
	}
	PINPAD_TRACE_SECU("PubSoftDes3 SUCC");
	return APP_SUCC;
}

int PubSoftUnDes(const char* psKey,const char* psSrc,char *psDest)
{
	int nRet;

	if(psKey == NULL || psSrc == NULL || psDest == NULL)
	{
		PINPAD_TRACE_SECU("psKey/psSrc/psDest error");
		ProSetSecurityErrCode(ERR_PINPAD_PARAM,0);
		return APP_FAIL;
	}
	nRet = Soft_AlgTDes((uchar*)psSrc,(uchar*)psDest,(uchar*)psKey,8,SALG_TDS_MODE_DEC);
	if(nRet != SALG_OK)
	{
        PINPAD_TRACE_SECU("NAPI_AlgTDes error = %d",nRet);
		ProSetSecurityErrCode(ERR_ALGDES,nRet);
		return APP_FAIL;
	}
	PINPAD_TRACE_SECU("PubSoftUnDes SUCC");
	return APP_SUCC;
}
/**
* @brief Do soft-TDES decryption with 'psKey'.
* @param [in]  psKey         Key string
* @param [in]  psSrc         Source data---8byte
* @param [out] psDest        Target data---8byte
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author Liug
* @date 2012-5-23
*/
int PubSoftUnDes3(const char* psKey,const char* psSrc,char *psDest)
{
	int nRet;

	if(psKey == NULL || psSrc == NULL || psDest == NULL)
	{
		PINPAD_TRACE_SECU("psKey/psSrc/psDest error");
		ProSetSecurityErrCode(ERR_PINPAD_PARAM,0);
		return APP_FAIL;
	}

	nRet = Soft_AlgTDes((uchar*)psSrc,(uchar*)psDest,(uchar*)psKey,16,SALG_TDS_MODE_DEC);
	if(nRet != SALG_OK)
	{
		ProSetSecurityErrCode(ERR_ALGDES,nRet);
        PINPAD_TRACE_SECU("NAPI_AlgTDes error = %d",nRet);
		return APP_FAIL;
	}
	PINPAD_TRACE_SECU("PubSoftUnDes3 SUCC");
	return APP_SUCC;
}

/**
* @brief Get security library version
* @param [out] pszVer  Version information, 12bytes
* @return 
* @li void
*/
void PubGetSecrityVerion(char *pszVer)
{
	if (pszVer != NULL)
	{
		strcpy(pszVer, SECURITYLIBVER);
	}
	else
	{
		PINPAD_TRACE_SECU("pszVer is NULL");
		ProSetSecurityErrCode(ERR_PINPAD_PARAM,0);
	}
}

/**
* @brief pinpad beep
* @param [in]  nDuration     time of duration
* @param [in]  Buzzer tone   0 – Default   1 – Alert (750Hz)  2 – Success (1500Hz) 
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
*/
int PinPad_Beep(int nDuration, int nType)
{
	if(ProCheckInit() != APP_SUCC)
		return APP_FAIL;
	if (gnSecurityMode == SECRITY_MODE_INSIDE)
	{
		PINPAD_TRACE_SECU("NOT support");
		return APP_QUIT;
	}
	else
	{
		return gstPinpad.pPinpadBeep(nDuration, nType);
	}
}

/**
* @brief show QR Code
* @param [in]  nVersion	Version(defult 0, from 0-40)
* @param [in]  pszBuffer content of display
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
*/
int PinPad_GenAndShowQr(int nVersion, char *pszBuffer)
{
	if(ProCheckInit() != APP_SUCC)
		return APP_FAIL;
	if (gnSecurityMode == SECRITY_MODE_INSIDE)
	{
		PINPAD_TRACE_SECU("NOT support");
		return APP_QUIT;
	}
	else
	{
		return gstPinpad.pPinpadQR(nVersion, pszBuffer);
	}
}

/**
* @brief Scan the code through the Pinpad
* @param [out]  results of scanning.
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
*/
int PinPad_DoScan(char *pszBuffer)
{
	if(ProCheckInit() != APP_SUCC)
		return APP_FAIL;
	if (gnSecurityMode == SECRITY_MODE_INSIDE)
	{
		PINPAD_TRACE_SECU("NOT support");
		return APP_QUIT;
	}
	else
	{
		return gstPinpad.pPinpadDoScan(pszBuffer);
	}
}

/**
* @brief signature
* @param [in]  pszCharaterCode	feature code
* @param [in]  name of signature
* @param [in]  nTimeOut
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
*/
int PinPad_DoSignature(char *pszCharaterCode, char *pszSignName, int nTimeOut)
{
	if(ProCheckInit() != APP_SUCC)
		return APP_FAIL;
	if (gnSecurityMode == SECRITY_MODE_INSIDE)
	{
		PINPAD_TRACE_SECU("NOT support");
		return APP_QUIT;
	}
	else
	{
		return gstPinpad.pEsignature(pszCharaterCode, pszSignName, nTimeOut);
	}
}

/**
* @brief swipe card on Pinpad
* @param [in] STREADCARD_IN
* @param [out] STREADCARD_OUT
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
*/
int PinPad_SwipeCard(char *pIn ,char *pOut)
{
	if(ProCheckInit() != APP_SUCC)
		return APP_FAIL;
	if (gnSecurityMode == SECRITY_MODE_INSIDE)
	{
		PINPAD_TRACE_SECU("NOT support");
		return APP_QUIT;
	}
	else
	{
		return gstPinpad.pSwipeCard(pIn, pOut);
	}
}

/**
* @brief  Initialize newland Level3 module		
*         Initialization and configuration need only be performed once at module startup 
*         and whilst configuration remains the same during processing.
* @param  [in] pszConfig:	According to configuration bitmap definitions.
* @param [in] nConfigLen 
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
*/
int PinPad_L3init(char *pszConfig, int nConfigLen)
{
	STPINPADL3_IN stL3Param;

	memset(&stL3Param, 0, sizeof(STPINPADL3_IN));
	if(ProCheckInit() != APP_SUCC)
		return APP_FAIL;
	if (gnSecurityMode == SECRITY_MODE_INSIDE)
	{
		PINPAD_TRACE_SECU("NOT support");
		return APP_QUIT;
	}
	else
	{
		stL3Param.cMsgType = PINPAD_INITL3;
		stL3Param.pszInputData = pszConfig;
		stL3Param.nInputDataLen = nConfigLen;
		return gstPinpad.pL3OrderSet((char *)&stL3Param, NULL, NULL);
	}
}

/**
* @brief CAPK management(Update/Get/Remove/Flush)
* @param [IN] capk: ca publick key
* @param [IN] mode:  Update/Get/Remove/Flush
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
*/
int PinPad_L3LoadCapk(L3_CAPK_ENTRY *capk, L3_CONFIG_OP mode)
{
	char cMsgType;
	char szData[16] = {0};
	STPINPADL3_IN stL3Param;
	char *pszInput = NULL;
	int nInPutLen = 0;

	memset(&stL3Param, 0, sizeof(STPINPADL3_IN));
	if(ProCheckInit() != APP_SUCC)
		return APP_FAIL;
	if (gnSecurityMode == SECRITY_MODE_INSIDE)
	{
		PINPAD_TRACE_SECU("NOT support");
		return APP_QUIT;
	}
	else
	{
		switch (mode)
		{
		case CONFIG_UPT:
			cMsgType = PINPAD_UPDATECAPK;
			pszInput = (char *)capk;
			nInPutLen = sizeof(L3_CAPK_ENTRY) - 4;
			break;
		case CONFIG_GET:
			cMsgType = PINPAD_GETCAPK;
			memcpy(szData, capk->rid, 5);
			szData[5] = capk->index;
			pszInput = szData;
			nInPutLen = 6;
			break;
		case CONFIG_RMV:
			cMsgType = PINPAD_RMSPECCAPK;
			memcpy(szData, capk->rid, 5);
			szData[5] = capk->index;
			pszInput = szData;
			nInPutLen = 6;
			break;
		case CONFIG_FLUSH:
			cMsgType = PINPAD_RMALLCAPK;
			break;
		default:
			return APP_FAIL;
		}
		stL3Param.cMsgType = cMsgType;
		stL3Param.pszInputData = pszInput;
		stL3Param.nInputDataLen = nInPutLen;
		return gstPinpad.pL3OrderSet((char *)&stL3Param, (char *)capk, NULL);
	}
}

/**
* @brief AID configuration management(Update/Get/Remove/Flush)
* @param [in] interface: Contact / Contactless
* @param [in] aidEntry:  Aid indicator(Get/Remove) or NULL(Update/Flush)
* @param [IN/OUT] tlv_list: Tlv AID configuration string
* @param [IN/OUT] tlv_len:  the length of tlv string
* @param [IN] mode: Update/Get/Remove/Flush, if Flush, will delete all of the configuration(Terminal and AID)
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
*/
int PinPad_L3LoadAIDConfig(L3_CARD_INTERFACE cardInterface, L3_AID_ENTRY *aidEntry, unsigned char tlv_list[], int *tlv_len, L3_CONFIG_OP mode)
{
	char cMsgType;
	STPINPADL3_IN stL3Param;
	char *pszInput = NULL;
	char szOutTlv[1500] = {0};
	int nInPutLen = 0;
	int nRet, nOff;

	memset(&stL3Param, 0, sizeof(STPINPADL3_IN));
	if(ProCheckInit() != APP_SUCC)
		return APP_FAIL;
	if (gnSecurityMode == SECRITY_MODE_INSIDE)
	{
		PINPAD_TRACE_SECU("NOT support");
		return APP_QUIT;
	}
	else
	{
		switch (mode)
		{
		case CONFIG_UPT:
			cMsgType = PINPAD_UPDATEAID;
			pszInput = (char *)tlv_list;
			nInPutLen = *tlv_len;
			break;
		case CONFIG_GET:
			cMsgType = PINPAD_GETAIDCFG;
			pszInput = (char *)aidEntry;
			nInPutLen = 27; // except externString and len
			break;
		case CONFIG_RMV:
			cMsgType = PINPAD_RMSPECAID;
			pszInput = (char *)aidEntry;
			nInPutLen = 27; // except externString and len
			break;
		case CONFIG_FLUSH:
			cMsgType = PINPAD_RMALLAID;
			break;
		default:
			return APP_FAIL;
		}
		stL3Param.cMsgType = cMsgType;
		stL3Param.cCardInteface = cardInterface;
		stL3Param.pszInputData = pszInput;
		stL3Param.nInputDataLen = nInPutLen;
		nRet = gstPinpad.pL3OrderSet((char *)&stL3Param, szOutTlv, NULL);
		if (nRet != APP_SUCC)
		{
			return nRet;
		}

		if (mode == CONFIG_GET)
		{
			nOff = 0;
			PubC2ToInt((uint *)tlv_len, (uchar *)szOutTlv + nOff);
			nOff += 2;
			memcpy(tlv_list, szOutTlv + nOff, *tlv_len);
		}
		return APP_SUCC; 
	}
}

/**
* @brief Terminal configuration management(Update/Get)
* @param [IN] interface: Contact / Contactless
* @param [IN/OUT] tlv_list: Tlv terminal configuration string
* @param [IN/OUT] tlv_len:  the length of tlv string
* @param [IN] mode: Update/Get/Remove/Flush, if Flush, will delete all of the configuration(Terminal and AID)
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
*/
int PinPad_L3LoadTerminalConfig(L3_CARD_INTERFACE cardInterface, unsigned char tlv_list[], int *tlv_len, L3_CONFIG_OP mode)
{
	char cMsgType = PINPAD_SETTERMINALCFG;
	STPINPADL3_IN stL3Param;
	char *pszInput = (char *)tlv_list;
	char szOutTlv[1500] = {0};
	int nOutTlvLen;
	int nInPutLen = 0, nRet, nOff;

	if (tlv_len)
	{
		nInPutLen = *tlv_len;
	}
	memset(&stL3Param, 0, sizeof(STPINPADL3_IN));
	if(ProCheckInit() != APP_SUCC)
		return APP_FAIL;
	if (gnSecurityMode == SECRITY_MODE_INSIDE)
	{
		PINPAD_TRACE_SECU("NOT support");
		return APP_QUIT;
	}
	else
	{
		switch (mode)
		{
		case CONFIG_UPT:
			cMsgType = PINPAD_SETTERMINALCFG;
			break;
		case CONFIG_GET:
			cMsgType = PINPAD_GETTERMINALCFG;
			break;
		default:
			PINPAD_TRACE_SECU("mode = %d", mode);
			return APP_FAIL;
		}
		stL3Param.cMsgType = cMsgType;
		stL3Param.cCardInteface = cardInterface;
		stL3Param.pszInputData = pszInput;
		stL3Param.nInputDataLen = nInPutLen;
		nRet = gstPinpad.pL3OrderSet((char *)&stL3Param, szOutTlv, &nOutTlvLen);
		if (mode == CONFIG_UPT || nRet != APP_SUCC)
		{
			return nRet;
		}
		nOff = 0;
		PubC2ToInt((uint *)tlv_len, (uchar *)szOutTlv + nOff);
		nOff += 2;
		memcpy(tlv_list, szOutTlv + nOff, *tlv_len);
		return APP_SUCC;
	}
}

/**
*@brief Perform transactions on the MSR, contact and contactless card interfaces. 
*			A transaction may have a simple flow that can be completed via this API, 
*			in which case the result returned will be Offline Approved, Offline Declined, Failed or some other error status. 
*			A transaction may also have a more complex flow that may require going online for authorization. 
*			For such flows the transaction will have to be completed by using multiple API. 
*			In this case this API will return a status of Online Authorization Required.
*			When this happens, the terminal must send the transaction online for authorization.
* @param [IN] pszInput: transaction data
* @param [IN] nInPutLen: the length of data
* @param [OUT] pszOut: the cmd result
* @param [OUT] pnOutLen: the cmd result data length
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
*/
int PinPad_L3PerformTransaction(char *pszInput, int nInPutLen, char *pszOut, int *pnOutLen)
{
	char cMsgType = PINPAD_PERFORMTRANS;
	int nRet;
	STPINPADL3_IN stL3Param;

	memset(&stL3Param, 0, sizeof(STPINPADL3_IN));
	if(ProCheckInit() != APP_SUCC)
		return APP_FAIL;
	if (gnSecurityMode == SECRITY_MODE_INSIDE)
	{
		PINPAD_TRACE_SECU("NOT support");
		return APP_QUIT;
	}
	else
	{
		stL3Param.cMsgType = cMsgType;
		stL3Param.pszInputData = pszInput + 1;
		stL3Param.cCardInteface = pszInput[0];
		stL3Param.nInputDataLen = nInPutLen - 1;
		nRet = gstPinpad.pL3OrderSet((char *)&stL3Param, pszOut, pnOutLen);
		if (nRet != APP_SUCC)
		{
			return nRet;
		}
		return APP_SUCC;
	}
}

/**
* @brief Complete the transaction. 
*		 When PinPad_L3PerformTransaction return a status of Online Authorization Required.
*		 the terminal must send the transaction online for authorization.
*		 The back-end host may respond back with a result or there may be a timeout or a network error.
*		 In any case, the terminal should use this API conveying the result of the Online Authorization Request to complete the transaction.
* @param [IN] nResult online result
* @param [IN] pszInput: transaction data
* @param [IN] nInPutLen: the length of data
* @param [OUT] res: the transaction result
* @param [OUT] pszResPonseCode: the cmd response
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
*/
int PinPad_L3CompleteTransaction(int nResult, char *pszInput, int nInPutLen, L3_TXN_RES *res, char *pszResPonseCode)
{
	char cMsgType = PINPAD_COMPLETETRANS;
	int nRet;
	int nOff;
	int nErrCode;
	char szInputData[32] = {0};
	STPINPADL3_IN stL3Param;
	char szOutPut[256+1] = {0};

	memset(&stL3Param, 0, sizeof(STPINPADL3_IN));
	if(ProCheckInit() != APP_SUCC)
		return APP_FAIL;
	if (gnSecurityMode == SECRITY_MODE_INSIDE)
	{
		PINPAD_TRACE_SECU("NOT support");
		return APP_QUIT;
	}
	else
	{
		nOff = 0;
		szInputData[nOff] = nResult;
		nOff += 1;
		PubIntToC2((uchar *)szInputData + nOff, nInPutLen);
		nOff += 2;
		memcpy(szInputData + nOff, pszInput, nInPutLen);
		nOff += nInPutLen;
		stL3Param.cMsgType = cMsgType;
		stL3Param.pszInputData = szInputData;
		stL3Param.nInputDataLen = nOff;
		nRet = gstPinpad.pL3OrderSet((char *)&stL3Param, szOutPut, NULL);
		if (nRet != APP_SUCC)
		{
			return nRet;
		}
		memcpy(pszResPonseCode, "00", 2);
		nOff = 0;
		*res = szOutPut[nOff];
		nOff += 1;
		PubC4ToInt((uint *)&nErrCode, (uchar *)szOutPut + nOff);
		return nErrCode;
	}
}

/**
* @brief Terminate transaction and release resource.
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
*/
int PinPad_L3TerminateTransaction()
{
	char cMsgType = PINPAD_TERMINATE;
	STPINPADL3_IN stL3Param;

	memset(&stL3Param, 0, sizeof(STPINPADL3_IN));
	if(ProCheckInit() != APP_SUCC)
		return APP_FAIL;
	if (gnSecurityMode == SECRITY_MODE_INSIDE)
	{
		PINPAD_TRACE_SECU("NOT support");
		return APP_QUIT;
	}
	else
	{
		stL3Param.cMsgType = cMsgType;
		stL3Param.pszInputData = NULL;
		stL3Param.nInputDataLen = 0;
		return gstPinpad.pL3OrderSet((char *)&stL3Param, NULL, NULL);
	}
}

/**
* @brief Get the EMVL3 data 
* @param [IN]  type: ,L3_DATA or L2 TAG.
* @param [OUT] data: The data buffer.
* @param [IN]  maxLen: The buffer length of data.
* @return description
* @retval 0    tag value does not exist
* @retval >0   The length of the Value
* @retval -1   Data length exceeds length limit
*/
int PinPad_L3GetData(int nTag, char *pszOut, int nMaxLen)
{
	char cMsgType = PINPAD_GETL3DATA;
	char szInPut[16] = {0};
	char szOut[256] = {0};
	int nOff, nRet;
	int nLen;
	STPINPADL3_IN stL3Param;

	memset(&stL3Param, 0, sizeof(STPINPADL3_IN));
	if(ProCheckInit() != APP_SUCC)
		return APP_FAIL;
	if (gnSecurityMode == SECRITY_MODE_INSIDE)
	{
		PINPAD_TRACE_SECU("NOT support");
		return APP_QUIT;
	}
	else
	{
		nOff = 0;
		szInPut[0] = 0; // index
		nOff += 1;
		PubIntToC4((uchar *)szInPut + nOff, (uint)nTag);
		nOff += 4;
		PubIntToC2((uchar *)szInPut + nOff, (uint)nMaxLen);
		nOff += 2;
		stL3Param.cMsgType = cMsgType;
		stL3Param.pszInputData = szInPut;
		stL3Param.nInputDataLen = nOff;
		nRet = gstPinpad.pL3OrderSet((char *)&stL3Param, szOut, &nLen);
		if (nRet != APP_SUCC)
		{
			return nRet;
		}
		nOff = 0;
		if (szOut[nOff] != 0) // tlv data status
		{
			return APP_QUIT;
		}
		nOff += 1;
		if (szInPut[0] != 0)
		{
			nOff += 2; //actual data len
		}
		PubC2ToInt((uint *)&nLen, (uchar *)szOut + nOff);
		nOff += 2;
		memcpy(pszOut, szOut + nOff, nLen);

		return nLen;
	}
}

/**
* @brief Setup kernel data.
* @param [IN]  tag: Tag.
* @param [IN]  data: The setting data buffer.
* @param [IN]  len: The length of data.
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
*/
int PinPad_L3SetData(unsigned int tag, void *data, unsigned int len)
{
	char cMsgType = PINPAD_SETTLVDATA;
	char szInPut[64] = {0};
	int nOff;
	STPINPADL3_IN stL3Param;

	memset(&stL3Param, 0, sizeof(STPINPADL3_IN));
	if(ProCheckInit() != APP_SUCC)
		return APP_FAIL;
	if (gnSecurityMode == SECRITY_MODE_INSIDE)
	{
		PINPAD_TRACE_SECU("NOT support");
		return APP_QUIT;
	}
	else
	{
		nOff = 0;
		PubIntToC4((uchar *)szInPut + nOff, (uint)tag);
		nOff += 4;
		PubIntToC2((uchar *)szInPut + nOff, (uint)len);
		nOff += 2;
		memcpy(szInPut + nOff, data, len);
		nOff += len;
		stL3Param.cMsgType = cMsgType;
		stL3Param.pszInputData = szInPut;
		stL3Param.nInputDataLen = nOff;
		return gstPinpad.pL3OrderSet((char *)&stL3Param, NULL, NULL);
	}
}

/**
* @brief Get the TLV format data from current kernel.
* @param [IN] tagList: The tag list which will be packed to tlvData.
* @param [IN] tagNum: The number of tag list.
* @param [OUT] tlvData: Out tlv data
* @param [IN] maxLen: The size of tlvData.
* @param [IN] ctl: Control code.
             bit0: 1: ignore the zero length tag.
* @return description
* @retval < 0 Fail
*         > 0 the length of out data(tlvData)
*/
int PinPad_L3GetTlvData(unsigned int *tagList, unsigned int tagNum, unsigned char *tlvData, unsigned int maxLen,int ctl)
{
	char cMsgType = PINPAD_GETTLVLIST;
	char szInPut[256] = {0};
	char szOut[256] = {0};
	int nOff, nRet;
	int nLen;
	STPINPADL3_IN stL3Param;
	char szTagList[256] = {0};
	char szTagTmp[16] = {0};
	int nTagListLength = 0, i;

	memset(&stL3Param, 0, sizeof(STPINPADL3_IN));
	if(ProCheckInit() != APP_SUCC)
		return APP_FAIL;
	if (gnSecurityMode == SECRITY_MODE_INSIDE)
	{
		PINPAD_TRACE_SECU("NOT support");
		return APP_QUIT;
	}
	else
	{
		nOff = 0;
		szInPut[nOff] = 0;
		nOff += 1;
		// tag num
		szInPut[nOff] = tagNum;
		nOff += 1;
		//max length
		PubIntToC2((uchar *)szInPut + nOff, (uint)maxLen);
		nOff += 2;
		// control code
		szInPut[nOff] = ctl;
		nOff += 1;
		// tag list length
		for (i = 0; i < tagNum; i++)
		{
			memset(szTagTmp, 0, sizeof(szTagTmp));
			sprintf(szTagTmp, "%x", tagList[i]);
			PubAscToHex((uchar *)szTagTmp, strlen(szTagTmp), 0, (uchar *)szTagList + nTagListLength);
			nTagListLength += (strlen(szTagTmp) / 2);
		}
		PubIntToC2((uchar *)szInPut + nOff, (uint)nTagListLength);
		nOff += 2;
		memcpy(szInPut + nOff, szTagList, nTagListLength);
		nOff += nTagListLength;

		stL3Param.cMsgType = cMsgType;
		stL3Param.pszInputData = szInPut;
		stL3Param.nInputDataLen = nOff;

		PINPAD_TRACEHEX_SECU(szInPut, nOff, "szInput");
		nRet = gstPinpad.pL3OrderSet((char *)&stL3Param, szOut, &nLen);
		if (nRet != APP_SUCC)
		{
			return nRet;
		}
		nOff = 0;
		PINPAD_TRACE_SECU("nLen = %d", nLen);
		PINPAD_TRACEHEX_SECU(szOut, nLen, "szOut");
		if (szOut[nOff] != 0) // tlv data status
		{
			return APP_QUIT;
		}
		nOff += 1;
		if (szInPut[0] != 0)
		{
			nOff += 2; //actual data len
		}
		PubC2ToInt((uint *)&nLen, (uchar *)szOut + nOff);
		nOff += 2;
		memcpy(tlvData, szOut + nOff, nLen);

		return nLen;
	}
}

/**
* @brief Setup Debug Mode
* @param [IN]debugLV   LV_CLOSE / LV_DEBUG / LV_ALL
* @li APP_FAIL Fail
* @li APP_SUCC Success
*/
int PinPad_L3SetDebugMode(int nDebugLv)
{
	STPINPADL3_IN stL3Param;
	char szOutPut[64] = {0};
	char szInput[2+1] = {0};

	memset(&stL3Param, 0, sizeof(STPINPADL3_IN));
	if(ProCheckInit() != APP_SUCC)
		return APP_FAIL;
	if (gnSecurityMode == SECRITY_MODE_INSIDE)
	{
		PINPAD_TRACE_SECU("NOT support");
		return APP_QUIT;
	}
	else
	{
		szInput[0] = nDebugLv;
		stL3Param.cMsgType = PINPAD_SETDEBUGMODE;
		stL3Param.pszInputData = szInput;
		stL3Param.nInputDataLen = 1;
		return gstPinpad.pL3OrderSet((char *)&stL3Param, szOutPut, NULL);
	}
}

/**
* @brief  Enumerate all AID, include terminal config 
* @param [IN] interface: Contact/Contactless
* @param [OUT] aidEntry: AID Lists
* @param [IN]maxCount:   Max size of aidEntry
* @return
* @li >0  Number of AID(<= maxCount)
* @li <= 0 FAIL
*/
int PinPad_L3EnumEmvConfig(L3_CARD_INTERFACE interface, L3_AID_ENTRY * aidEntry, int maxCount)
{
	STPINPADL3_IN stL3Param;
	char szOutPut[1024+1] = {0};
	char szInput[2+1] = {0};
	int nRet, nNum, nOff, nAidLen, i;

	memset(&stL3Param, 0, sizeof(STPINPADL3_IN));
	if(ProCheckInit() != APP_SUCC)
		return APP_FAIL;
	if (gnSecurityMode == SECRITY_MODE_INSIDE)
	{
		PINPAD_TRACE_SECU("NOT support");
		return APP_QUIT;
	}
	else
	{
		szInput[0] = interface;
		stL3Param.cMsgType = PINPAD_GETAIDNUM;
		stL3Param.cCardInteface = interface;
		stL3Param.pszInputData = szInput;
		stL3Param.nInputDataLen = 0;
		nRet = gstPinpad.pL3OrderSet((char *)&stL3Param, szOutPut, NULL);
		if (nRet != APP_SUCC)
		{
			return nRet;
		}
		nOff = 0;
		PubC2ToInt((uint*)&nNum, (uchar *)szOutPut + nOff);
		PINPAD_TRACE_SECU("Num = %d", nNum);
		if (aidEntry == NULL)
		{
			return nNum;
		}
		nOff += 2; // aid Num
		for (i = 0; i < nNum; i++)
		{
			PubC2ToInt((uint *)&nAidLen, (uchar *)szOutPut + nOff);
			nOff += 2; // aid len
			nOff += 3; // 9F06 + len
			nAidLen -= 3;
			aidEntry[i].aidLen = nAidLen;
			memcpy((char *)aidEntry[i].aid, szOutPut + nOff, nAidLen);
			nOff += nAidLen;
			//PINPAD_TRACEHEX_SECU(aidEntry[i].aid, nAidLen, "aid");
		}
		return nNum;
	}

}

/**
* @brief Enumerate CAPK
* @param in  start        ---Start index
* @param in  end          ---End index
* @param out capk        ---CAPK Lists
* @return
* @li        >0            Number of public key
* @li        <= 0          FAIL
*/
int PinPad_L3EnumCapk(int start, int end, char capk[][6])
{
	STPINPADL3_IN stL3Param;
	char szOutPut[1024] = {0};
	int nRet, nNum, nOff, nOutPutLen, i;

	memset(&stL3Param, 0, sizeof(STPINPADL3_IN));
	if(ProCheckInit() != APP_SUCC)
		return APP_FAIL;
	if (gnSecurityMode == SECRITY_MODE_INSIDE)
	{
		PINPAD_TRACE_SECU("NOT support");
		return APP_QUIT;
	}
	else
	{
		stL3Param.cMsgType = PINPAD_GETCAPKNUM;
		stL3Param.pszInputData = NULL;
		stL3Param.nInputDataLen = 0;
		nRet = gstPinpad.pL3OrderSet((char *)&stL3Param, szOutPut, &nOutPutLen);
		if (nRet != APP_SUCC)
		{
			return nRet;
		}
		nOff = 0;
		PubC2ToInt((uint*)&nNum, (uchar *)szOutPut + nOff);
		nOff += 2;
		PINPAD_TRACE_SECU("Num = %d", nNum);
		if (capk == NULL)
		{
			return nNum;
		}
		for (i = 0; i < nNum; i++)
		{
			memcpy((char *)capk[i], szOutPut + nOff, 6);
			nOff += 6;
		//	PINPAD_TRACEHEX_SECU(capk[i], 6, "RID");
		}
		
		return nNum;
	}
}

/**
* @brief send L3 perform data to Pinpad
* @param in  pL3Cfg        --- request data
* @return
* @li        0           succ
*/
int PinPad_L3PerformSend(char *pL3Cfg)
{
	if(ProCheckInit() != APP_SUCC)
		return APP_FAIL;
	if (gnSecurityMode == SECRITY_MODE_INSIDE)
	{
		PINPAD_TRACE_SECU("NOT support");
		return APP_QUIT;
	}
	else
	{
		return gstPinpad.PL3PerformSend(pL3Cfg);
	}
}

/**
* @brief send  data to Pinpad
* @param in  psSend        --- request data
* @param in  nSendLen      --- request data length
* @return
* @li        0           succ
*/
int PinPad_Send(char *psSend, int nSendLen)
{
	if(ProCheckInit() != APP_SUCC)
		return APP_FAIL;
	if (gnSecurityMode == SECRITY_MODE_INSIDE)
	{
		PINPAD_TRACE_SECU("NOT support");
		return APP_QUIT;
	}
	else
	{
		return gstPinpad.PSendToPinpad(psSend, nSendLen);
	}
}

/**
* @brief send receive data from Pinpad
* @param[out] psRecv 	    Buffer to save data
* @param[in]  pnRecvLen     The length buffer want to read
* @param[out] pnRecvLen 	Actual length  of read
* @return
* @li        0           succ
*/
int PinPad_Recv(char *psRecv, int nMaxReadLen, int *pnRecvLen)
{
	if(ProCheckInit() != APP_SUCC)
		return APP_FAIL;
	if (gnSecurityMode == SECRITY_MODE_INSIDE)
	{
		PINPAD_TRACE_SECU("NOT support");
		return APP_QUIT;
	}
	else
	{
		return gstPinpad.PRecvFromPinpad(psRecv, nMaxReadLen, pnRecvLen);
	}
}

/**
* @brief set font size
* @param [in] nSize 1 - normal 2- small 3- large
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
*/
int PinPad_SetFontSize(char cSize)
{
	if(ProCheckInit() != APP_SUCC)
		return APP_FAIL;
	if (gnSecurityMode == SECRITY_MODE_INSIDE)
	{
		PINPAD_TRACE_SECU("NOT support");
		return APP_QUIT;
	}
	else
	{
		return gstPinpad.pSetFontSize(cSize);
	}
}


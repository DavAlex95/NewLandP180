#include "pinpad.h"

#include "ltool.h"
#include "lui.h"
#include "pinpadtool.h"
#include "libapiinc.h"

#define PINPAD_MAX_DATA_LEN 5240

static int nPinpadBps = BPS115200;
static int nPinpadPort;	
static int nPinpadTimeOut = 5;	
static uchar sSendBuf[PINPAD_MAX_DATA_LEN*2+5];
static uchar sRecvBuf[PINPAD_MAX_DATA_LEN*2+5];
static char gcPortStatus = NO;

extern int PerformCallBack(char cCallbackId, char *pszInput);

STPINPADMSGTYPE gstMsgType[] = {
	{PINPAD_SETTERMINALCFG,		    "L0", "L1" , FUNCID_SETTERMINALCFG, "SETTERMINALCFG"},
	{PINPAD_GETTERMINALCFG,			"L0", "L1" , FUNCID_GETTERMINALCFG, "GETTERMINALCFG"},
	{PINPAD_UPDATEAID,  		    "L0", "L1" , FUNCID_UPDATEAID, "UPDATEAID"},
	{PINPAD_GETAIDCFG,				"L0", "L1" , FUNCID_GETAIDCFG, "GETAIDCFG"},
	{PINPAD_RMSPECAID,				"L0", "L1" , FUNCID_RMSPECAID, "RMSPECAID"},
	{PINPAD_RMALLAID,				"L0", "L1" , FUNCID_RMALLAID, "RMALLAID"},
	{PINPAD_UPDATECAPK,				"L0", "L1" , FUNCID_UPDATECAPK, "UPDATECAPK"},
	{PINPAD_GETCAPK,				"L0", "L1" , FUNCID_GETCAPK, "GETCAPK"},
	{PINPAD_RMSPECCAPK,				"L0", "L1" , FUNCID_RMSPECCAPK, "RMSPECCAPK"},
	{PINPAD_RMALLCAPK,		        "L0", "L1" , FUNCID_RMALLCAPK, "RMALLCAPK"},
	{PINPAD_GETAIDNUM,		        "L0", "L1" , FUNCID_GETAIDNUM, "GETAIDNUM"},
	{PINPAD_GETCAPKNUM,		        "L0", "L1" , FUNCID_GETCAPKNUM, "GETCAPKNUM"},
	{PINPAD_UPDATECERTLIST,			"L0", "L1" , FUNCID_UPDATECERTLIST, "UPDATECERTLIST"},
	{PINPAD_GETCERTLIST,		    "L0", "L1" , FUNCID_GETCERTLIST, "GETCERTLIST"},
	{PINPAD_RMSPECCERTLIST,		    "L0", "L1" , FUNCID_RMSPECCERTLIST, "RMSPECCERTLIST"},
	{PINPAD_RMALLCERTLIST,		    "L0", "L1" , FUNCID_RMALLCERTLIST, "RMALLCERTLIST"},
	{PINPAD_UPDATEEXCEPTIONLIST,    "L0", "L1" , FUNCID_UPDATEEXCEPTIONLIST, "UPDATEEXCEPTIONLIST"},
	{PINPAD_GETEXCEPTIONLIST,		"L0", "L1" , FUNCID_GETEXCEPTIONLIST, "GETEXCEPTIONLIST"},
	{PINPAD_RMSPECEXCEPTIONLIST,	"L0", "L1" , FUNCID_RMSPECEXCEPTIONLIST, "RMSPECEXCEPTIONLIST"},
	{PINPAD_RMALLEXCEPTIONLIST,		"L0", "L1" , FUNCID_RMALLEXCEPTIONLIST, "RMALLEXCEPTIONLIST"},
	{PINPAD_INITL3,		            "L0", "L1" , FUNCID_INITL3, "INITL3"},
	{PINPAD_SETTLVDATA,				"L0", "L1" , FUNCID_SETTLVDATA, "SETTLVDATA"},
	{PINPAD_GETL3DATA,				"L0", "L1" , FUNCID_GETL3DATA, "GETL3DATA"},
	{PINPAD_SETTLVLIST,				"L0", "L1" , FUNCID_SETTLVLIST, "SETTLVLIST"},
	{PINPAD_GETTLVLIST,				"L0", "L1" , FUNCID_GETTLVLIST, "GETTLVLIST"},
	{PINPAD_SETDEBUGMODE,			"L0", "L1" , FUNCID_SETDEBUGMODE, "SETDEBUGMODE"},
	{PINPAD_GETKERNELVER,			"L0", "L1" , FUNCID_GETKERNELVER, "GETKERNELVER"},
	{PINPAD_CANCELREADCARD,			"", "" , 0, "CANCELREADCARD"},
	{PINPAD_PERFORMTRANS,			"L0", "L1" , FUNCID_PERFORMTRANS, "PERFORMTRANS"},
	{PINPAD_COMPLETETRANS,			"L0", "L1" , FUNCID_COMPLETETRANS, "COMPLETETRANS"},
	{PINPAD_TERMINATE,				"L0", "L1" , FUNCID_TERMINATE, "TERMINATE"},
	{PINPAD_L3CALLBACK,				"L0", "L1" , FUNCID_L3CALLBACK, "L3CALLBACK"},
};

#define ASSERT_FAIL(e) \
		{\
			if ((e)!=APP_SUCC)\
			{\
				return APP_FAIL;\
			}\
		}
	
#define ASSERT_QUIT(e) \
	{\
		if ((e)!=APP_SUCC)\
		{\
			return APP_QUIT;\
		}\
	}


#define	 SEPERATOR_CMD					(0x2F)

#define MEMSETSENDBUF ( memset(sSendBuf, 0, sizeof(sSendBuf)) )
#define MEMSETRECVBUF ( memset(sRecvBuf, 0, sizeof(sRecvBuf)) )

static int ProInitPinpadAux(const int nPort, const int nBps);
static int ProGetDeviceAttribute(const int nPort, const int nBps, char *pszDeviceAttribute, int nWaitTime);

static int LoadMsgType(char cMsgType, STPINPADMSGTYPE *pstMsgType)
{
	int nTotalNum, i ;
	char szTransName[32+1];

	memset(szTransName, 0, sizeof(szTransName));
	nTotalNum = sizeof(gstMsgType)/sizeof(STPINPADMSGTYPE);
	
	PINPAD_TRACE_SECU("nNum = [%d]", nTotalNum);
	for(i= 0 ; i < nTotalNum ; i++)
	{
		if(gstMsgType[i].cMsgType == cMsgType)
		{
			break;
		}
	}

	if(i >= nTotalNum)
	{
		PINPAD_TRACE_SECU("cMsgType =[%02x] was not defined in STPINPADMSGTYPE ", cMsgType);
		PubMsgDlg("Warn", "Unknow Trans Type", 3, 3);
		return APP_FAIL;
	}
	memcpy(pstMsgType, &gstMsgType[i], sizeof(STPINPADMSGTYPE));
	PINPAD_TRACE_SECU("type = %d req = %s rsp = %s funid = %02x name = %s", gstMsgType[i].cMsgType, gstMsgType[i].szReq, gstMsgType[i].szRsp, gstMsgType[i].cFuncId, gstMsgType[i].szMsgName);

	return APP_SUCC;
}


static int PinPad_InsertAsyn(unsigned char *psBuf, unsigned int *punLen)
{
	char sTemp[PINPAD_MAX_DATA_LEN*2+10] = {0};
	int nLen = 2;
	
	if (NULL == psBuf || NULL == punLen)
	{
		return APP_FAIL;
	}

	memcpy(sTemp, psBuf, *punLen);
	psBuf[0] = STX;
	PubIntToBcd((char *)psBuf + 1, &nLen, *punLen);
	memcpy(psBuf + 3, sTemp, *punLen);
	psBuf[*punLen + 3] = ETX;
	PubCalcLRC((char *)psBuf + 1, *punLen + 3, (char *)psBuf + *punLen + 4);
	*punLen += 5;
	return APP_SUCC;
}

static int PinPad_DeleteAsyn(unsigned char *psBuf, unsigned int *punLen)
{
	int nLen = 0;
	char cLRC = 0;
	char sTemp[PINPAD_MAX_DATA_LEN*2+5] = {0};
	
	if (NULL == psBuf || NULL == punLen)
	{
		return APP_FAIL;
	}
	if (psBuf[0] != STX)
	{
		return APP_FAIL;
	}
	if (*punLen < 5)
	{
		return APP_FAIL;
	}
	PubBcdToInt((char *)psBuf + 1, &nLen);
	if (*punLen != nLen + 5)
	{
		return APP_FAIL;
	}
	if (PubCalcLRC((char *)psBuf + 1, nLen + 3, &cLRC) != APP_SUCC)
	{
		return APP_FAIL;
	}	
	if (*(psBuf + *punLen - 1) != cLRC)
	{
		return APP_FAIL;
	}
	memcpy(sTemp, psBuf + 3, nLen);
	memcpy(psBuf, sTemp, nLen);
	*punLen = nLen;
	return APP_SUCC;
}

static int SendAck(int nPort)
{
	char szAck[1+1] = {0};
	int nRet;

	PINPAD_TRACE_SECU("SendAck ");
	szAck[0] = ACK;
	nRet = NAPI_PortWrite(nPort, (uchar *)szAck, 1);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("SendAck nRet = %d",nRet);
		return APP_FAIL;
	}
	return APP_SUCC;
}

static int GetKbHit()
{
	int nCode = 0xFF;
	int nRet;

	nRet = NAPI_KbHit(&nCode);
	if (nRet != NAPI_OK)
	{
		PINPAD_TRACE_SECU("nRet = %d",nRet);
	}

	return nCode;
}

static int CheckIsStartText(int nRecvResponFlag, int nPort, uint nTimeOut)
{
	char szBuf[32+1] = {0};
	int nRet;
	int nReadLen;
	uint unTimeOut;

	unTimeOut = PubGetOverTimer(nTimeOut * 1000);
	while(1)
	{
		nReadLen = 1;
		if(GetKbHit() == KEY_ESC)
		{
			PINPAD_TRACE_SECU("press cancel");
			return APP_QUIT;
		}
		nRet = NAPI_PortRead(nPort, (uchar *)szBuf, &nReadLen, 1000);
		if(nRet == NAPI_ERR_TIMEOUT)
		{
			continue;
		}

		if (nRet != NAPI_OK)
		{
			PINPAD_TRACE_SECU("nRet = %d", nRet);
			return APP_FAIL;
		}

		if (PubTimerIsOver(unTimeOut) == APP_SUCC && nReadLen != 1)
		{
			PINPAD_TRACE_SECU("ERR_PINPAD_NOTRET");
			ProSetSecurityErrCode(ERR_PINPAD_NOTRET, 0);
			return APP_FAIL;
		}
		if (szBuf[0] == NAK)
		{
			continue;
		}
		else if (szBuf[0] == STX)
		{
			return APP_SUCC;
		}
		else if (szBuf[0] == ACK)
		{
			if (nRecvResponFlag == 0)
			{
				return APP_SUCC;
			}
			continue;
		}
		else
		{
			PINPAD_TRACE_SECU("szbuf[0] = %x", szBuf[0]);
			break;
		}
	}

	nReadLen = 16;
	nRet = NAPI_PortRead(nPort, (uchar *)szBuf, &nReadLen, 1000);
	PINPAD_TRACEHEX_SECU(szBuf, nReadLen, "next byte");
	return APP_FAIL;
}

static int ReadDataLength(int nPort, int nTimeOut, char *pszRecv)
{
	char szBuf[16+1] = {0};
	int nRet;
	int nReadLen;
	uint unTimeOut;

	if (pszRecv == NULL)
	{
		PINPAD_TRACE_SECU("pszRecv is NULL");
		return APP_FAIL;
	}
	unTimeOut = PubGetOverTimer(nTimeOut * 1000);
	while(1)
	{
		if(GetKbHit() == KEY_ESC)
		{
			PINPAD_TRACE_SECU("press cancel");
			SendAck(nPort);
			return APP_FAIL;
		}
		nReadLen = 2;
		nRet = NAPI_PortRead(nPort, (uchar *)szBuf, (int *)&nReadLen, 1000);
		if (nRet != NAPI_OK && nRet != NAPI_ERR_TIMEOUT)
		{
			PINPAD_TRACE_SECU("nRet = %d", nRet);
			return APP_FAIL;
		}
		if(PubTimerIsOver(unTimeOut) == APP_SUCC && nReadLen != 2)
		{
			PINPAD_TRACE_SECU("ERR_PINPAD_NOTRET");
			ProSetSecurityErrCode(ERR_PINPAD_NOTRET, 0);
			SendAck(nPort);
			return APP_FAIL;
		}
		if(nReadLen == 2)
		{
			break;
		}
	}

	memcpy(pszRecv, szBuf, nReadLen);
	return APP_SUCC;
}

static int ReadData(int nPort, int nTimeOut, char *pszRecv, int *pnRecvLen)
{
	int nRet;
	int nReadLen, nDataLen;
	uint unTimeOut;

	if (pszRecv == NULL)
	{
		PINPAD_TRACE_SECU("pszRecv is NULL");
		return APP_FAIL;
	}
	unTimeOut = PubGetOverTimer(nTimeOut * 1000);
	PubBcdToInt((char *)pszRecv + 1, &nDataLen);
	nDataLen += 2;// ETX + LRC
	while(1)
	{
		nReadLen = nDataLen;
		nRet = NAPI_PortRead(nPort, (uchar *)pszRecv + 3, (int *)&nReadLen, 1000); // + 3 (STX + Length(2 bytes))
		if (nRet != NAPI_OK && nRet != NAPI_ERR_TIMEOUT)
		{
			PINPAD_TRACE_SECU("nRet = %d", nRet);
			return APP_FAIL;
		}
		if(PubTimerIsOver(unTimeOut) == APP_SUCC && nReadLen != nDataLen)
		{
			PINPAD_TRACE_SECU("ERR_PINPAD_NOTRET");
			ProSetSecurityErrCode(ERR_PINPAD_NOTRET, 0);
			SendAck(nPort);
			return APP_FAIL;
		}
		if(nReadLen == nDataLen)
		{
			break;
		}
		if(GetKbHit() == KEY_ESC)
		{
			PINPAD_TRACE_SECU("press cancel");
			SendAck(nPort);
			return APP_FAIL;
		}
	}
	*pnRecvLen = nReadLen;

	return APP_SUCC;
}

static YESORNO CheckIsSendAck(char *pszRecv)
{
	char szMsgType[2+1] = {0};
	int nFunid;

	memcpy(szMsgType, pszRecv, 2);
	if (memcmp(szMsgType, "L1", 2) == 0)
	{
		nFunid = pszRecv[3];
		if (nFunid == 0x21 || nFunid == 0x36) // callback L3 log and notification
		{
			return NO;
		}
	}
	return YES;
}

static int PinPad_SendRecv(char cFlag, uchar *psSend, uint nSendLen, uchar *psRecv, uint *pnRecvLen, int nPort, int nTimeOut)
{
	int nRet;
	uint nLen;
	char sSendBuf[PINPAD_MAX_DATA_LEN*2+5] = {0};
	char sRecvBuf[PINPAD_MAX_DATA_LEN*2+5] = {0};
	char szDataLen[2+1] = {0};
	int nReadSumLen; // STX + Data len + data + ETX + LRC
	
	memcpy(sSendBuf, psSend, nSendLen);
	nLen = nSendLen;

	ASSERT_FAIL(PinPad_InsertAsyn((uchar *)sSendBuf, &nLen));

	NAPI_KbFlush();
	PINPAD_TRACEHEX_SECU(sSendBuf, nLen, "send");
	nRet = NAPI_PortWrite(nPort, (uchar *)sSendBuf, nLen);
	if (nRet != NAPI_OK)
	{
		gcPortStatus = NO;
		PINPAD_TRACE_SECU("NAPI_PortWrite nRet = %d", nRet);
		return APP_FAIL;
	}

	// checking start of text
	nRet = CheckIsStartText(cFlag, nPort, nTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("nRet = %d", nRet);
		if (nRet == APP_QUIT) // press cancel button when checking STX flag
		{
			if (memcmp(psSend, "32", 2) == 0)
			{
				PubCancelPIN_PINPAD();
			}
			if (memcmp(psSend, "\x4C\x30\x2F\x36", 4) == 0)
			{
				PubL3CancalReadCard();
			}
			return SendAck(nPort);
		}
		return nRet;
	}

	//This command does not receive a response message; the PIN pad response consists of just the single [ACK] character.
	if(cFlag == 0)
	{
		PINPAD_TRACE_SECU("cFlag = 0");
		return APP_SUCC;
	}

	NAPI_KbFlush();
	nReadSumLen = 0;
	sRecvBuf[0] = STX;
	nReadSumLen += 1;

	// read data length
	ASSERT_FAIL(ReadDataLength(nPort, nTimeOut, szDataLen));
	memcpy(sRecvBuf + nReadSumLen, szDataLen, 2);
	nReadSumLen += 2;

	NAPI_KbFlush();	
	//read data : data + ETX + LCR
	ASSERT_FAIL(ReadData(nPort, nTimeOut, sRecvBuf, (int *)&nLen));
	nReadSumLen += nLen;

	PINPAD_TRACEHEX_SECU(sRecvBuf, nReadSumLen, "recv");
	ASSERT_FAIL(PinPad_DeleteAsyn((uchar *)sRecvBuf, (uint *)&nReadSumLen));

	memcpy(psRecv, sRecvBuf, nReadSumLen);
	*pnRecvLen = nReadSumLen;

	if (CheckIsSendAck(sRecvBuf) != YES)
	{
		return APP_SUCC;
	}
	SendAck(nPort);

	return APP_SUCC;
}

static int ProInitPinpadAux(const int nPort, const int nBps)
{
    PORT_SETTINGS stPortSetting;
	PORT_TYPE PortType = (PORT_TYPE)nPort;
	int nRet;
	static int nPortTmp = 0;
	static int nBpsTmp = 0;

	if (nPortTmp != nPort || nBpsTmp != nBps)
	{
		nPortTmp = nPort;
		nBpsTmp = nBps;
		gcPortStatus = NO;
	}
    memset(&stPortSetting, 0, sizeof(stPortSetting));
	switch(nBps)
	{
	case BPS9600:
		stPortSetting.BaudRate = BAUD9600;
		break;
	case BPS19200:
		stPortSetting.BaudRate = BAUD19200;
		break;
	case BPS38400:
		stPortSetting.BaudRate = BAUD38400;
		break;
	case BPS57600:
		stPortSetting.BaudRate = BAUD57600;
		break;
	case BPS115200:
		stPortSetting.BaudRate = BAUD115200;
		break;
	default:
		return APP_FAIL;
	}
	stPortSetting.DataBits = DATA_8;
	stPortSetting.Parity = PAR_NONE;
	stPortSetting.StopBits = STOP_1;
	if (nPort == USB_HOST && gcPortStatus == YES)
	{
		if(NAPI_PortFlush(PortType) == NAPI_OK)
		{
			return APP_SUCC;
		}
	}

	nRet = NAPI_PortOpen(PortType, stPortSetting) ;
	if(nRet!= NAPI_OK)
	{
		NAPI_PortClose(PortType);
		nRet = NAPI_PortOpen(PortType, stPortSetting);
		if(nRet!= NAPI_OK)
		{
			PINPAD_TRACE_SECU("nPort = %d bps = %d nRet = %d", nPort, nBps, nRet);
			return APP_FAIL;
		}
	}

	gcPortStatus = YES;
	if(NAPI_PortFlush(PortType) != NAPI_OK)
	{
		return APP_FAIL;
	}

	return APP_SUCC;
}

/**
* @brief Reset pinpad,set baudrate and port number.
* @param [in] nPort	Port number
* @param [in] nTimeOut	Timeout
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author chenwu
* @date 2018-3-22
*/
int PubResetPinpad_PINPAD(const int nPort, const int nTimeOut)
{
	int nRet = 0;
	char szDeviceAttribute[100] = {0};
	int nDefaultBps = BPS115200;

	if (nTimeOut <= 0)
	{
		PINPAD_TRACE_SECU("PubResetPinpad_PINPAD:nTimeOut<=0");
		ProSetSecurityErrCode(ERR_PINPAD_PARAM, 0);
		return APP_FAIL;
	}
	
	nPinpadTimeOut = 3;

	nRet = ProGetDeviceAttribute(nPort, nDefaultBps, szDeviceAttribute, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		ProSetSecurityErrCode(ERR_PINPAD_FAIL, 0);
		return APP_FAIL;
	}
	nPinpadPort = nPort;
	nPinpadTimeOut = nTimeOut;
	nPinpadBps = nDefaultBps;
	return APP_SUCC;
}

/**
* @brief Install main and work key
* @param [in] nKeyType		EM_KEY_TYPE
* @param [in] nKeyIndex		Key index
* @param [in] psKeyValue	Buffer of the working key
* @param [in] nKeyLen		key length 8 or 16 or 24
* @param [in] psCheckValue	check value (3 bytes) used to check if work key is correct
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author chenwu
* @date 2018-3-21
*/
int PubLoadKey_PINPAD(int nKeyType, int nKeyIndex, const char *psKeyValue, int nKeyLen, char *psCheckValue)
{
	int nRet = 0;
	int nLen = 0;
	int nOff = 0;

	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("ProLoadKey_PINPAD:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;
	
	memcpy(sSendBuf, "50", 2);
	nOff += 2;
	sSendBuf[nOff] = SEPERATOR_CMD;
	nOff += 1;
	if (nKeyType == KEY_TYPE_TMK)
	{
		sSendBuf[nOff] = nKeyIndex;	// KEK index
	}
	else if (nKeyType == KEY_TYPE_DATA)
	{
		sSendBuf[nOff] = nKeyIndex - 128;
	}
	else 
	{
		sSendBuf[nOff] = nKeyIndex;
	}

	PINPAD_TRACE_SECU("load key = %d", nKeyType, sSendBuf[nOff]);
	nOff += 1;
	if (nKeyType == KEY_TYPE_DATA)
	{
		sSendBuf[nOff] = 0x02; //0 - TR31 block, 1 - AESK,2 - DES block
	}
	else
	{
		sSendBuf[nOff] = 0x02; //0 - TR31 block, 1 - AESK,2 - DES block
	}
	nOff += 1;
	//sSendBuf[nOff] = 24; // for AESK : 16/24/32
	//nOff += 1;
	switch(nKeyType)
	{
	case KEY_TYPE_PIN:
		sSendBuf[nOff] = 0x00;
		break;
	case KEY_TYPE_MAC:
		sSendBuf[nOff] = 0x01;
		break;
	case KEY_TYPE_DATA:
		sSendBuf[nOff] = 0x02;
		break;
	case KEY_TYPE_TMK:
		sSendBuf[nOff] = 0x03;
		break;
	default:
		return APP_FAIL;
	}
	nOff += 1;
	sSendBuf[nOff] = nKeyIndex;
	nOff += 1;
	PubIntToC2(sSendBuf + nOff, nKeyLen);
	nOff += 2;
	memcpy(sSendBuf + nOff, psKeyValue, nKeyLen);
	nLen = nOff + nKeyLen;
	if(psCheckValue != NULL)
	{
		memcpy(sSendBuf + nLen, psCheckValue, 3);
		nLen = nLen + 3;
	}
	nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("ProLoadKey_PINPAD:PinPad_SendRecv APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}

	if(memcmp(sRecvBuf, "51", 2) != 0)
	{
		PINPAD_TRACE_SECU("ProLoadKey_PINPAD:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}
	if(memcmp(sRecvBuf + 3, "00", 2) != 0)
	{
		ProSetSecurityErrCode(ERR_LOADKEY,nRet);
		PINPAD_TRACE_SECU("Response Code [%2.2s]", sRecvBuf + 3);
		return APP_FAIL;
	}
	return APP_SUCC;
}

int PubInjectKey_PINPAD(int nKeyType, int nSrcIndex, int nDstIndex,const char *psKeyValue, int nKeyLen, char *psCheckValue)
{
	int nRet = 0;
	int nLen = 0;

	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("ProLoadKey_PINPAD:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;
	
	memcpy(sSendBuf, "50", 2);
	sSendBuf[2] = SEPERATOR_CMD;
	sSendBuf[3] = nSrcIndex;
	sSendBuf[4] = 0x02; //0 - TR31 block, 1 - AESK,2 - DES block
	switch(nKeyType)
	{
	case KEY_TYPE_PIN:
		sSendBuf[5] = 0x00;
		break;
	case KEY_TYPE_MAC:
		sSendBuf[5] = 0x01;
		break;
	case KEY_TYPE_DATA:
		sSendBuf[5] = 0x02;
		break;
	case KEY_TYPE_TMK:
		sSendBuf[5] = 0x03;
		break;	
	default:
		return APP_FAIL;
	}
	sSendBuf[6] = nDstIndex;
	PubIntToC2(sSendBuf + 7, nKeyLen);
	memcpy(sSendBuf + 9, psKeyValue, nKeyLen);
	nLen = 9 + nKeyLen;
	if(psCheckValue != NULL)
	{
		memcpy(sSendBuf + nLen, psCheckValue, 3);
		nLen = nLen + 3;
	}
	nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("ProLoadKey_PINPAD:PinPad_SendRecv APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}

	if(memcmp(sRecvBuf, "51", 2) != 0)
	{
		PINPAD_TRACE_SECU("ProLoadKey_PINPAD:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}
	if(memcmp(sRecvBuf + 3, "00", 2) != 0)
	{
		ProSetSecurityErrCode(ERR_LOADKEY,nRet);
		PINPAD_TRACE_SECU("Response Code [%2.2s]", sRecvBuf + 3);
		return APP_FAIL;
	}
	return APP_SUCC;
}

/**
* @brief Get PIN inlcuding plain and encrypted
* @param [out] pszPin	 Pointer to PIN-block buffer
* @param [out] pnPinLen  Pointer to PIN-block-length buffer
* @param [in]  nMode	 0-Encrypt with PAN, 1-Encrypt without PAN 2-Plain
* @param [in]  nKeyType - 0 - Master/Session, 1 - DUKPT, 2 - AES
* @param [in]  nKeyIndex
* @param [in]  pszCardno Card Number/(NULL)
* @param [in]  nPanLen	 Length of card NO.
* @param [in]  nMaxLen	 Maximum PIN length (4-12)
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
int PubGetPinBlock_PINPAD(char *psPin, int *pnPinLen, int nMode, int nKeyType, int nKeyIndex, const char *pszCardno, int nPanLen, int nMaxLen)
{
	int nRet = 0;
	int nLen = 0;
	char *pszLine1 = "Enter online PIN:";
	char *pszLine2 = NULL;
	char *pszLine3 = NULL;
	char *pszLine4 = NULL;
	
	if (psPin == NULL || pszCardno == NULL)
	{
		ProSetSecurityErrCode(ERR_PINPAD_PARAM, 0);
		return APP_FAIL;
	}
	
	if(nMode != PINTYPE_WITHPAN)
	{
		ProSetSecurityErrCode(ERR_PINPAD_PARAM, 0);
		return APP_FAIL;
	}
	
	if (nMaxLen < 0)
	{
		ProSetSecurityErrCode(ERR_PINPAD_PARAM, 0);
		return APP_FAIL;
	}
	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubGetPinBlock_PINPAD:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;
	memcpy(sSendBuf, "32", 2);
	sSendBuf[2] = SEPERATOR_CMD;
	sSendBuf[3] = nKeyIndex;
	sSendBuf[4] = nKeyType;	//0 - Master/Session, 1 - DUKPT, 2 - AES
	sSendBuf[5] = 0x00;

	nLen = strlen(pszCardno);
	if(nLen <= 0)
	{
		ProSetSecurityErrCode(ERR_PINPAD_PARAM, 0);
		return APP_FAIL;
	}

	memcpy(sSendBuf + 6, pszCardno, nLen);
	nLen = 6 + nLen;
	sSendBuf[nLen++] = 0x1C;
	sSendBuf[nLen++] = 0x00;
	sSendBuf[nLen++] = 0x00;
	sSendBuf[nLen++] = nMaxLen;
	sSendBuf[nLen++] = 0x00;
	sSendBuf[nLen++] = nPinpadTimeOut;
	sSendBuf[nLen++] = 0x00;

	if(pszLine1 != NULL)
	{
		memcpy(sSendBuf + nLen, pszLine1, strlen(pszLine1));
		nLen = nLen + strlen(pszLine1);
	}
	memcpy(sSendBuf + nLen, "\x1C", 1);
	nLen++;
	if(pszLine2 != NULL)
	{
		memcpy(sSendBuf + nLen, pszLine2, strlen(pszLine2));
		nLen = nLen + strlen(pszLine2);
	}
	memcpy(sSendBuf + nLen, "\x1C", 1);
	nLen++;
	if(pszLine3 != NULL)
	{
		memcpy(sSendBuf + nLen, pszLine3, strlen(pszLine3));
		nLen = nLen + strlen(pszLine3);
	}
	memcpy(sSendBuf + nLen, "\x1C", 1);
	nLen++;
	if(pszLine4 != NULL)
	{
		memcpy(sSendBuf + nLen, pszLine4, strlen(pszLine4));
		nLen = nLen + strlen(pszLine4);
	}
	memcpy(sSendBuf + nLen, "\x1C", 1);
	nLen++;

	nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		if (nRet == APP_QUIT)
		{
			ProSetSecurityErrCode(ERR_PINPAD_QUIT, 0);
			return APP_FAIL;
		}
		PINPAD_TRACE_SECU("PubGetPinBlock_PINPAD:PinPad_SendRecv APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}

	if(memcmp(sRecvBuf, "33", 2) != 0)
	{
		PINPAD_TRACE_SECU("PubGetPinBlock_PINPAD:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}
	if(memcmp(sRecvBuf + 3, "04", 2) == 0)
	{
		ProSetSecurityErrCode(ERR_PINPAD_QUIT,nRet);
		PINPAD_TRACE_SECU("Response Code [%2.2s]", sRecvBuf + 3);
		return APP_FAIL;
	}
	else if(memcmp(sRecvBuf + 3, "00", 2) != 0)
	{
		ProSetSecurityErrCode(ERR_PINPAD_FAIL,nRet);
		PINPAD_TRACE_SECU("Response Code [%2.2s]", sRecvBuf + 3);
		return APP_FAIL;
	}
	if(sRecvBuf[6] == 0)
	{
		memset(psPin, 0, 8);
	}
	else
	{
		memcpy(psPin, sRecvBuf + 7, 8);
		*pnPinLen = 8;
	}
	return APP_SUCC;
}

/**
* @brief Calculate MAC
* @param [out] psMac		 MAC
* @param [in]  nMode		 MAC9606, X99, UCB
* @param [in]  psData		 Data to be calculated
* @param [in]  nDataLen 	 Data length
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author chenwu
* @date 2018-3-22
*/
int PubCalcMac_PINPAD(char *psMac, int nKeyType, int nMode, int nKeyIndex, const char *psData, int nDataLen)
{
	int nRet = 0;
	int nLen = 0;
	char sStr[1024] = {0};
	int nStrLen, nOff = 0;

	nStrLen = nDataLen;
	memcpy(sStr, psData, nStrLen);
	if (nStrLen%8 != 0)
	{
		nStrLen += 8 - nStrLen%8;
	}
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("ProLoadKey_PINPAD:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;
	nOff = 0;
	memcpy(sSendBuf, "8C", 2);
	nOff += 2;
	sSendBuf[nOff] = SEPERATOR_CMD;
	nOff += 1;
	sSendBuf[nOff] = nKeyIndex;
	nOff += 1;
	sSendBuf[nOff] = nKeyType;	//0 - Master/Session, 1 - DUKPT, 2 - AES
	nOff += 1;
	switch(nMode)
	{
	case PINPAD_X99:
		sSendBuf[nOff] = 0x00;
		break;
	case PINPAD_X919:
		sSendBuf[nOff] = 0x01;
		break;
	case PINPAD_ECB:
		sSendBuf[nOff] = 0x02;
		break;
	case PINPAD_9606:
		sSendBuf[nOff] = 0x03;
		break;
	case PINPAD_AES:
		sSendBuf[nOff] = 0x05;
		break;
	default:
		ProSetSecurityErrCode(ERR_PINPAD_PARAM, 0);
		return APP_FAIL;
	}
	nOff += 1;
	sSendBuf[nOff] = 0x03; // only block for less than 4096
	nOff += 1;

	PubIntToC2(sSendBuf + nOff, nStrLen);
	nOff += 2;
	memcpy(sSendBuf + nOff, sStr, nStrLen);
	nOff += nStrLen;
	sSendBuf[nOff] = 0x00;
	nOff += 1;
	sSendBuf[nOff] = 0x00;
	nOff += 1;
	
	nRet = PinPad_SendRecv(1, sSendBuf, nOff, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubCalcMac_PINPAD:PinPad_SendRecv APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}

	nOff = 0;
	if(memcmp(sRecvBuf, "8D", 2) != 0)
	{
		PINPAD_TRACE_SECU("PubCalcMac_PINPAD:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}
	nOff += 4;
	if(memcmp(sRecvBuf + nOff, "00", 2) != 0)
	{
		ProSetSecurityErrCode(ERR_GETMAC,nRet);
		PINPAD_TRACE_SECU("Response Code [%2.2s]", sRecvBuf + 4);
		return APP_FAIL;
	}
	nOff += 2;
	memcpy(psMac, sRecvBuf + nOff, 8); // dukpt/MK/SK

	return APP_SUCC;
}


/**
* @brief Clear pinpad
* @param void
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author chenwu
* @date 2018-3-22
*/
int PubClrPinPad_PINPAD(void)
{
	PubDispPinPad("           Welcome", "    Newland Payment", NULL, NULL);
	return APP_FAIL;
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
int PubDispPinPad_PINPAD(const char *pszLine1, const char *pszLine2, const char *pszLine3, const char *pszLine4)
{
	int nRet = 0;
	int nLen = 0;

	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("ProLoadKey_PINPAD:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;
	
	memcpy(sSendBuf, "3C", 2);
	sSendBuf[2] = SEPERATOR_CMD;
	nLen = 3;
	if(pszLine1 != NULL)
	{
		memcpy(sSendBuf + nLen, pszLine1, strlen(pszLine1));
		nLen = nLen + strlen(pszLine1);
	}
	memcpy(sSendBuf + nLen, "\x1C", 1);
	nLen++;
	if(pszLine2 != NULL)
	{
		memcpy(sSendBuf + nLen, pszLine2, strlen(pszLine2));
		nLen = nLen + strlen(pszLine2);
	}
	memcpy(sSendBuf + nLen, "\x1C", 1);
	nLen++;
	if(pszLine3 != NULL)
	{
		memcpy(sSendBuf + nLen, pszLine3, strlen(pszLine3));
		nLen = nLen + strlen(pszLine3);
	}
	memcpy(sSendBuf + nLen, "\x1C", 1);
	nLen++;
	if(pszLine4 != NULL)
	{
		memcpy(sSendBuf + nLen, pszLine4, strlen(pszLine4));
		nLen = nLen + strlen(pszLine4);
	}
	memcpy(sSendBuf + nLen, "\x1C", 1);
	nLen++;
	
	nRet = PinPad_SendRecv(0, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubDispPinPad_PINPAD:PinPad_SendRecv APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}
	return APP_SUCC;
}


/**
* @brief Do DES encryption with specified 'nDesMode' key
* @param [out] psDest		 Target data
* @param [in]  psSrc		 Source data
* @param [in] nKeyIndex		 Key index
* @param [in]  nDesMode 	 Des mode
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author chenwu
* @date 2018-3-22
*/
int PubDesPinpad_PINPAD(const char *psSrc, int nSrcLen, char *psDest, int nKeyIndex, int nDesMode)
{
	int nRet = 0;
	int nLen = 0;
	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("ProLoadKey_PINPAD:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;
	
	memcpy(sSendBuf, "80", 2);
	sSendBuf[2] = SEPERATOR_CMD;
	sSendBuf[3] = nKeyIndex;

	/*
	1 = CBC Encryption
	2 = ECB Encryption 
	3 = CBC Decryption
	4 = ECB Decryption 
	*/
	switch(nDesMode)
	{
	case DESMODE_DES:
	case DESMODE_3DES:
		sSendBuf[4] = 0x02;
		break;
	case DESMODE_UNDES:
	case DESMODE_3UNDES:
		sSendBuf[4] = 0x04;
		break;
	default:
		ProSetSecurityErrCode(ERR_PINPAD_PARAM, 0);
		return APP_FAIL;
	}
	
	PubIntToC2(sSendBuf + 5, nSrcLen);
	memcpy(sSendBuf + 7, psSrc, nSrcLen);
	nLen = 7 + nSrcLen;
	sSendBuf[nLen] = 0x00;
	nLen++;
	sSendBuf[nLen + 1] = 0x00;
	nLen++;
	
	nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubDesPinpad_PINPAD:PinPad_SendRecv APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}

	if(memcmp(sRecvBuf, "81", 2) != 0)
	{
		PINPAD_TRACE_SECU("PubDesPinpad_PINPAD:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}
	if(memcmp(sRecvBuf + 4, "00", 2) != 0)
	{
		ProSetSecurityErrCode(ERR_CALCDES,nRet);
		PINPAD_TRACE_SECU("Response Code [%2.2s]", sRecvBuf + 3);
		return APP_FAIL;
	}
	memcpy(psDest, sRecvBuf + 8, nSrcLen);
	return APP_SUCC;
}

int PubGetErrCode_PINPAD(void)
{
	int nErrCode=0;
	char buf[80]={0};
	
	PubGetErrorCode(&nErrCode,buf, NULL);
	return nErrCode;
}

static int ProGetDeviceAttribute(const int nPort, const int nBps, char *pszDeviceAttribute, int nWaitTime)
{
	int nRet = 0;
	int nLen = 0;

	PINPAD_TRACE_SECU("ProGetDeviceAttribute:nPort = %d nBps = %d", nPort, nBps);

	nRet = ProInitPinpadAux(nPort, nBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("ProGetDeviceAttribute:ProInitPinpadAux APP_FAIL = %d", nPort);
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;
	
	memcpy(sSendBuf, "90", 2);
	sSendBuf[2] = SEPERATOR_CMD;
	nLen = 3;

	nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPort, nWaitTime);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("ProGetDeviceAttribute:PinPad_SendRecv APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}

	if(memcmp(sRecvBuf, "91", 2) != 0)
	{
		PINPAD_TRACE_SECU("ProGetDeviceAttribute:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}
	memcpy(pszDeviceAttribute, sRecvBuf + 3, nLen);
	return APP_SUCC;
}

/**
* @brief display image
* @param [out] 		
* @param [in]  
* @param [in]  
* @param [in]  
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author chenwu
* @date 2018-3-22
*/
int PubDisplayImage_PINPAD(int nX, int nY, int nWidth, int nHeight, const char *psData)
{
	int nRet = 0;
	int nLen = 0;
	int nSurplus = 0;
	int nTotalPage = 0;
	int nImageDataPos = 0;
	int nCurrentPage = 0;
	int nDataLen = 0;

	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("ProLoadKey_PINPAD:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}
	
	nDataLen = nWidth*((nHeight+7)>>3);
	nTotalPage = nDataLen/256;
	if (nTotalPage == 0)
	{
		nTotalPage++;
	}
	if (nDataLen%256 != 0)
	{
		nTotalPage++;
	}
	nTotalPage --;
	nSurplus = nDataLen;
	while (1)
	{
		MEMSETSENDBUF;
		MEMSETRECVBUF;
		nLen = 0;
		memcpy(sSendBuf, "D0", 2);
		nLen += 2;
		sSendBuf[nLen] = SEPERATOR_CMD;
		nLen ++;

		PubIntToC2(sSendBuf+nLen, nX);
		nLen += 2;
		
		PubIntToC2(sSendBuf+nLen, nY);
		nLen += 2;
		
		PubIntToC2(sSendBuf+nLen, nWidth);
		nLen += 2;
		
		PubIntToC2(sSendBuf+nLen, nHeight);
		nLen += 2;
		
		PubIntToC2(sSendBuf+nLen, nCurrentPage);
		nLen += 2;
		
		PubIntToC2(sSendBuf+nLen, nTotalPage);
		nLen += 2;

		if (nTotalPage == 0)
		{
			memcpy(sSendBuf+nLen, psData+nImageDataPos, nSurplus);
			nLen += nSurplus;
		}
		else
		{
			memcpy(sSendBuf+nLen, psData+nImageDataPos, 256);
			nLen += 256;
		}
		
		nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
		if (nRet != APP_SUCC)
		{
			PINPAD_TRACE_SECU("PubCalcMac_PINPAD:PinPad_SendRecv APP_FAIL");
			ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
			return APP_FAIL;
		}

		if(memcmp(sRecvBuf, "D1", 2) != 0)
		{
			PINPAD_TRACE_SECU("PubDisplayImage_PINPAD:CMD erro");
			ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
			return APP_FAIL;
		}
		
		if(memcmp(sRecvBuf + 3, "\x00\x00", 2) != 0)
		{
			PINPAD_TRACE_SECU("Response Code [%2.2s]", sRecvBuf + 3);
			return APP_FAIL;
		}

		if (nTotalPage == 0)
		{
			break;
		}
		
		nCurrentPage ++;
		nTotalPage --;
		nSurplus -= 256;
		nImageDataPos += 256;
			
	}

	return APP_SUCC;
}

int PubClearKey_PINPAD(void)
{
	int nRet = 0;
	int nLen = 0;
	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubClearKey_PINPAD:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;
	
	memcpy(sSendBuf, "60", 2);
	sSendBuf[2] = SEPERATOR_CMD;
	nLen = 3;
	
	nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubClearKey:PinPad_SendRecv APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}

	if(memcmp(sRecvBuf, "61", 2) != 0)
	{
		PINPAD_TRACE_SECU("PubClearKey:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}
	if(memcmp(sRecvBuf + 3, "00", 2) != 0)
	{
		ProSetSecurityErrCode(ERR_GETMAC,nRet);
		PINPAD_TRACE_SECU("Response Code [%2.2s]", sRecvBuf + 3);
		return APP_FAIL;
	}
	return APP_SUCC;
}

int PubCancelPIN_PINPAD()
{
	int nRet = 0;
	int nLen = 0;

	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubClearKey_PINPAD:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;
	
	memcpy(sSendBuf, "38", 2);
	sSendBuf[2] = SEPERATOR_CMD;
	nLen = 3;
	
	nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubPinpadBeep_PINPAD:PinPad_SendRecv APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}

	return APP_SUCC;
}

int PubGenAndShowQr_PINPAD(int nVersion, char *pszBuffer)
{
	int nRet = 0;
	int nLen = 0;
 	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubClearKey_PINPAD:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;
	
	memcpy(sSendBuf, "D2", 2);
	nLen += 2;
	sSendBuf[nLen++]= SEPERATOR_CMD;
	sSendBuf[nLen++]= 1;//adjust level
	sSendBuf[nLen++]= 0;//mask no
	sSendBuf[nLen++]= nVersion;//version
	sSendBuf[nLen++]= 1;//Auto Centering
	//sSendBuf[nLen++]= 0;//x
	//sSendBuf[nLen++]= 0;//y
	sSendBuf[nLen++]= 1;//title
	sSendBuf[nLen++]= 0;//title length
	PubIntToC2(sSendBuf+nLen, strlen(pszBuffer));
	nLen += 2;
	memcpy(sSendBuf+nLen, pszBuffer, strlen(pszBuffer));
	nLen += strlen(pszBuffer);
	nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubPinpadBeep_PINPAD:PinPad_SendRecv APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}
	if(memcmp(sRecvBuf, "D3", 2) != 0)
	{
		PINPAD_TRACE_SECU("PubLoadClearKey_PINPAD:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}
	//the spec shows the BCD 00.
	if(memcmp(sRecvBuf + 3, "\x00\x00", 2) != 0)
	{
		ProSetSecurityErrCode(ERR_GETMAC,nRet);
		PINPAD_TRACE_SECU("Response Code [%2.2s]", sRecvBuf + 3);
		return APP_FAIL;
	}

	return APP_SUCC;
}

static int PubScanStart()
{
	int nRet = 0;
	uint nLen = 0;
 	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubClearKey_PINPAD:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;
	
	memcpy(sSendBuf, "G0", 2);
	nLen += 2;
	sSendBuf[nLen++]= SEPERATOR_CMD;
	sSendBuf[nLen++]= 0x01;
	nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("scan start");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}
	if(memcmp(sRecvBuf, "G1", 2) != 0)
	{
		PINPAD_TRACE_SECU("star:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}
	if(memcmp(sRecvBuf + 4, "00", 2) != 0)
	{
		ProSetSecurityErrCode(ERR_GETMAC,nRet);
		PINPAD_TRACE_SECU("start failed [%2.2s]", sRecvBuf + 3);
		return APP_FAIL;
	}
	PubDebug("#### scan start ok");
	return APP_SUCC;
}

static int PubScanGet(char *pszBuf)
{
	uint nLen = 0;
	int nRet = 0;
	int nDataLen = 0;
	int nKey;
 	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubClearKey_PINPAD:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}
GETTT:
	nLen = 0;
	MEMSETSENDBUF;
	MEMSETRECVBUF;
	NAPI_KbHit(&nKey);
	if(nKey == KEY_ESC)
	{
		return APP_FAIL;
	}
	
	memcpy(sSendBuf, "G0", 2);
	nLen += 2;
	sSendBuf[nLen++]= SEPERATOR_CMD;
	sSendBuf[nLen++]= 0x02;
	nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("scan get");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}
	if(memcmp(sRecvBuf, "G1", 2) != 0)
	{
		PINPAD_TRACE_SECU("get:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}
	if(memcmp(sRecvBuf + 4, "00", 2) != 0)
	{
		if (memcmp(sRecvBuf + 4, "01", 2) == 0)
		{
			PubDebug("#### get again");
			PubSysDelay(1);
			goto GETTT;
		}
		ProSetSecurityErrCode(ERR_GETMAC,nRet);
		PINPAD_TRACE_SECU("get failed [%2.2s]", sRecvBuf + 3);
		return APP_FAIL;
	}
	nLen = 6;
	PubC2ToInt((uint *)&nDataLen, sRecvBuf + nLen);
	nLen += 2;
	memcpy(pszBuf, sRecvBuf + nLen, nDataLen);
	PubDebug("#### scan get ok [%s]", pszBuf);

	return APP_SUCC;
}

static int PubScanStop()
{
	int nRet = 0;
	uint nLen = 0;
 	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubScanStop_PINPAD:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;

	memcpy(sSendBuf, "G0", 2);
	nLen += 2;
	sSendBuf[nLen++]= SEPERATOR_CMD;
	sSendBuf[nLen++]= 0x03;
	nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("scan stop");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}
	if(memcmp(sRecvBuf, "G1", 2) != 0)
	{
		PINPAD_TRACE_SECU("stop:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}
	if(memcmp(sRecvBuf + 4, "00", 2) != 0)
	{
		ProSetSecurityErrCode(ERR_GETMAC,nRet);
		PINPAD_TRACE_SECU("stop failed [%2.2s]", sRecvBuf + 3);
		return APP_FAIL;
	}

	PubDebug("#### scan stop ok ");

	return APP_SUCC;
}

int PubDoScan_PINPAD(char *pszBuf)
{
	int nRet = 0;

	if (PubScanStart() == APP_SUCC)
	{
		nRet = PubScanGet(pszBuf);
		nRet += PubScanStop();
	}
	else
	{
		return APP_FAIL;
	}
	
	return nRet;
}

// sign
static int ProEsignStart()
{
	int nRet = 0;
	uint nLen = 0;
	int nPos = 0;
 	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubClearKey_PINPAD:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;
	
	memcpy(sSendBuf, "S0", 2);
	nLen += 2;
	sSendBuf[nLen++]= SEPERATOR_CMD;
	sSendBuf[nLen++]= 0xA0;
	nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("scan start");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}
	if(memcmp(sRecvBuf, "S1", 2) != 0)
	{
		PINPAD_TRACE_SECU("star:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}
	if(memcmp(sRecvBuf + 4, "00", 2) != 0)
	{
		ProSetSecurityErrCode(ERR_GETMAC,nRet);
		PINPAD_TRACE_SECU("start failed [%2.2s]", sRecvBuf + 3);
		return APP_FAIL;
	}
	nPos += 5;
	//state
	PubDebugData("state: ", sRecvBuf+nPos, 1);
	nPos ++;
	//version
	PubDebugData("version: ", sRecvBuf+nPos, 6);
	nPos += 6;
	//sn
	PubDebugData("sn: ", sRecvBuf+nPos, 16);
	//flag

	return APP_SUCC;
}

static int ProEsignCheck(char *psNum)
{
	int nRet = 0;
	uint nLen = 0;
	int nPos = 0;
 	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubClearKey_PINPAD:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;
	
	memcpy(sSendBuf, "S0", 2);
	nLen += 2;
	sSendBuf[nLen++]= SEPERATOR_CMD;
	sSendBuf[nLen++]= 0xA1;
	memcpy(sSendBuf+nLen, psNum, 3);//BCD
	nLen += 3;
	sSendBuf[nLen++] = 0x01; //state
	
	nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("EsignCheck");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}
	if(memcmp(sRecvBuf, "S1", 2) != 0)
	{
		PINPAD_TRACE_SECU("star:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}
	if(memcmp(sRecvBuf + 4, "00", 2) != 0)
	{
		ProSetSecurityErrCode(ERR_GETMAC,nRet);
		PINPAD_TRACE_SECU("start failed [%2.2s]", sRecvBuf + 3);
		return APP_FAIL;
	}
	nPos += 5;
	//state
	PubDebugData("state: ", sRecvBuf+nPos, 1);

	return APP_SUCC;
}

static int SaveToFile(const char *psInBuf,int nOff, int nLen, const char *pszFileName)
{
	int fp;

	if((fp=PubFsOpen(pszFileName,"w"))<0)
	{
		PubFsClose(fp);
		return APP_FAIL;
	}
	
	if(PubFsSeek (fp,(long)nOff,SEEK_SET)<0)
	{
		PubFsClose(fp);
		return APP_FAIL;
	}
	
	if(PubFsWrite(fp, psInBuf,nLen)<0)
	{
		PubFsClose(fp);
		return APP_FAIL;
	}
	PubFsClose(fp);	
	return APP_SUCC;
}

static int ProEsignTransfer(char *pszCharaterCode, char *pszSignName, int nTimeOut)
{
	int nRet = 0;
	int nLen = 0;
	int nPos = 0;
	char szTimeOut[2+1] = {0};
 	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubClearKey_PINPAD:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;
	
	memcpy(sSendBuf, "S0", 2);
	nLen += 2;
	sSendBuf[nLen++]= SEPERATOR_CMD;
	sSendBuf[nLen++]= 0xA2;
	sSendBuf[nLen++]= 0x3C;
	PubIntToC2((uchar *)szTimeOut, (uint)nTimeOut);
	memcpy(sSendBuf+ nLen, szTimeOut, 2);
	nLen += 2;
	if (pszCharaterCode != NULL)
	{
		memcpy(sSendBuf + nLen, pszCharaterCode, 8);
	}
	nLen += 8;

	nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("nRet = %d", nRet);
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}

	if(memcmp(sRecvBuf, "S1", 2) != 0)
	{
		PINPAD_TRACE_SECU("star:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}
	if(memcmp(sRecvBuf + 4, "00", 2) != 0)
	{
		ProSetSecurityErrCode(ERR_GETMAC,nRet);
		PINPAD_TRACE_SECU("start failed [%2.2s]", sRecvBuf + 3);
		return APP_FAIL;
	}
	nPos += 6;
	//num
	PubDebugData("num: ", sRecvBuf+nPos, 3);
	nPos+=3;
	if (pszSignName != NULL)
	{
		SaveToFile((char *)sRecvBuf+nPos, 0, nLen-nPos, pszSignName);
	}
	else
	{
		SaveToFile((char *)sRecvBuf+nPos, 0, nLen-nPos, SIGNFILENAME);
	}
	
	PubDebug("save ok: %d", nLen-nPos);

	return APP_SUCC;
}


static int ProEsignFinish()
{
	int nRet = 0;
	uint nLen = 0;
 	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubClearKey_PINPAD:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;
	
	memcpy(sSendBuf, "S0", 2);
	nLen += 2;
	sSendBuf[nLen++]= SEPERATOR_CMD;
	sSendBuf[nLen++]= 0xA3;
	
	nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("scan start");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}
	if(memcmp(sRecvBuf, "S1", 2) != 0)
	{
		PINPAD_TRACE_SECU("star:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}
	if(memcmp(sRecvBuf + 4, "00", 2) != 0)
	{
		ProSetSecurityErrCode(ERR_GETMAC,nRet);
		PINPAD_TRACE_SECU("start failed [%2.2s]", sRecvBuf + 3);
		return APP_FAIL;
	}

	return APP_SUCC;
}

int PubEsignature_PINPAD(char *pszCharaterCode, char *pszSignName, int nTimeOut)
{
	int nRet = 0;
	char sNum[6] = "\x00\x00\x01";
	
	nRet = ProEsignStart();
	nRet = ProEsignTransfer(pszCharaterCode, pszSignName, nTimeOut);
	nRet += ProEsignCheck(sNum);
	nRet += ProEsignFinish();

	return nRet;
}

int PubGetKcv_PINPAD( int nKeyIndex,int nKeyType, char *psKcv)
{
	int nRet = 0;
	int nLen = 0, nOff;
	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("ProLoadKey_PINPAD:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;
	nOff = 0;
	memcpy(sSendBuf, "52", 2);
	nOff += 2;
	sSendBuf[nOff] = SEPERATOR_CMD;
	nOff += 1;
	sSendBuf[nOff] = nKeyIndex;
	nOff += 1;
	switch(nKeyType)
	{
	case KEY_TYPE_TMK:
		sSendBuf[nOff] = 0;
		break;		
	case KEY_TYPE_PIN:
		sSendBuf[nOff] = 1;
		break;
	case KEY_TYPE_MAC:
		sSendBuf[nOff] = 2;
		break;
	case KEY_TYPE_DATA:
		sSendBuf[nOff] = 3;
		break;
	default:
		return APP_FAIL;
	}

	nOff += 1;
	
	nRet = PinPad_SendRecv(1, sSendBuf, nOff, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubGetKcv_PINPAD:PinPad_SendRecv APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}

	if(memcmp(sRecvBuf, "53", 2) != 0)
	{
		PINPAD_TRACE_SECU("PubGetKcv_PINPAD:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}
	memcpy(psKcv, sRecvBuf + 4, 3);
	return APP_SUCC;
}

int PubGetKeyInfo_PINPAD(EM_SEC_KEY_INFO_ID InfoID, uchar ucKeyID, EM_SEC_CRYPTO_KEY_TYPE KeyType, EM_SEC_KEY_USAGE KeyUsage,
					  uchar *pAD, uint unADSize, uchar *psOutInfo, int *pnOutInfoLen)
{
	int nRet = 0;
	int nLen = 0, nOff;

	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("ProLoadKey_PINPAD:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;
	nOff = 0;
	memcpy(sSendBuf, "C2", 2);
	nOff += 2;
	sSendBuf[nOff++] = SEPERATOR_CMD;
	sSendBuf[nOff++] = InfoID;
	sSendBuf[nOff++] = ucKeyID;
	sSendBuf[nOff++] = KeyType;
	sSendBuf[nOff++] = KeyUsage;
	PubIntToC2((uchar *)sSendBuf + nOff, unADSize);
	nOff += 2;
	if (pAD)
	{
		memcpy(sSendBuf + nOff, pAD, unADSize);
		nOff += unADSize;
	}

	nRet = PinPad_SendRecv(1, sSendBuf, nOff, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubGetKeyInfo_PINPAD:PinPad_SendRecv APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}

	nOff = 0;
	if(memcmp(sRecvBuf, "C3", 2) != 0)
	{
		PINPAD_TRACE_SECU("PubGetKeyInfo_PINPAD:CMD erro %2.2s", sRecvBuf);
		return APP_FAIL;
	}

	nOff += 3;
	if(memcmp(sRecvBuf+nOff, "00", 2) != 0)
	{
		PINPAD_TRACE_SECU("PubGetKeyInfo_PINPAD:CMD erro %2.2s", sRecvBuf+nOff);
		return APP_QUIT;
	}

	if (psOutInfo && pnOutInfoLen)
	{
		nOff += 2;
		PubC2ToInt((uint *)pnOutInfoLen, (uchar *)sRecvBuf + nOff);
		nOff += 2;
		memcpy(psOutInfo, sRecvBuf + nOff, *pnOutInfoLen);
	}

	return APP_SUCC;
}


int PubSetFontColor_PINPAD( ushort usColor, char cObject)
{
	int nRet = 0;
	int nLen = 0;
	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("SetFontColor:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;
	
	memcpy(sSendBuf, "42", 2);
	sSendBuf[2] = SEPERATOR_CMD;
	PubIntToC2(&sSendBuf[3], usColor);
	//memcpy(&sSendBuf[3], &usColor, 2);
	sSendBuf[5] = cObject;


	nLen = 6;
	
	nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubSetFontColor_PINPAD:PinPad_SendRecv APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}

	return APP_SUCC;
}

int PubLoadImg_PINPAD(uchar ucImgID, int nImgLen, char *psImgData)
{
	int nRet = 0;
	int nLen = 0;
	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubLoadImg:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;
	
	memcpy(sSendBuf, "3E", 2);
	sSendBuf[2] = SEPERATOR_CMD;
	sSendBuf[3] = ucImgID;
	PubIntToC2(sSendBuf+4, nImgLen);
	memcpy(sSendBuf+6,psImgData, nImgLen);


	nLen = 6 + nImgLen;
	
	nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubLoadImg_PINPAD:PinPad_SendRecv APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}

	if(memcmp(sRecvBuf, "3F", 2) != 0)
	{
		PINPAD_TRACE_SECU("PubLoadImg_PINPAD:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}
	if(memcmp(sRecvBuf + 3, "00", 2) != 0)
	{
		ProSetSecurityErrCode(ERR_GETMAC,nRet);
		PINPAD_TRACE_SECU("Response Code [%2.2s]", sRecvBuf + 3);
		return APP_FAIL;
	}	
	return APP_SUCC;

}


int PubDispImg_PINPAD(uchar ucImgID, int nX, int nY)
{
	int nRet = 0;
	int nLen = 0;
	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubDispImg_PINPAD:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;
	
	memcpy(sSendBuf, "3D", 2);
	sSendBuf[2] = SEPERATOR_CMD;
	sSendBuf[3] = ucImgID;
	PubIntToC2(sSendBuf+4, nX);
	PubIntToC2(sSendBuf+6, nY);


	nLen = 8;
	
	nRet = PinPad_SendRecv(0, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubDispImg_PINPAD:PinPad_SendRecv APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}


	return APP_SUCC;

}




int PubScrClrs_PINPAD(void)
{
	int nRet = 0;
	int nLen = 0;
	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubScrClrs:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;
	
	memcpy(sSendBuf, "4e",2);
	sSendBuf[2] = SEPERATOR_CMD;
	nLen = 3;
	
	nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubScrClrs_PINPAD:PinPad_SendRecv APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}


	if(memcmp(sRecvBuf, "4f", 2) != 0)
	{
		PINPAD_TRACE_SECU("PubScrClrs_PINPAD:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}

	if(memcmp(sRecvBuf+3, "00", 2) != 0)
	{
		PINPAD_TRACE_SECU("PubScrClrs_PINPAD:Response Code Erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}	
	return APP_SUCC;

}

int PubSetAlignMode_PINPAD(char cMode)
{
	int nRet = 0;
	int nLen = 0;
	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubSetAlignMode:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;
	
	memcpy(sSendBuf, "5c",2);
	sSendBuf[2] = SEPERATOR_CMD;
	sSendBuf[3] = cMode;
	nLen = 4;
	
	nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubSetAlignMode_PINPAD:PinPad_SendRecv APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}


	if(memcmp(sRecvBuf, "5d", 2) != 0)
	{
		PINPAD_TRACE_SECU("PubSetAlignMode_PINPAD:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}

	if(memcmp(sRecvBuf+3, "00", 2) != 0)
	{
		PINPAD_TRACE_SECU("PubScrClrs_PINPAD:Response Code Erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}	
	return APP_SUCC;

}


int PubSetClrsMode_PINPAD(char cMode)
{
	int nRet = 0;
	int nLen = 0;
	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("SetClrsMode:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;
	
	memcpy(sSendBuf, "d6",2);
	sSendBuf[2] = SEPERATOR_CMD;
	sSendBuf[3] = cMode;
	nLen = 4;
	
	nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubScrClrs_PINPAD:PinPad_SendRecv APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}


	if(memcmp(sRecvBuf, "d7", 2) != 0)
	{
		PINPAD_TRACE_SECU("PubScrClrs_PINPAD:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}

	if(memcmp(sRecvBuf+3, "00", 2) != 0)
	{
		PINPAD_TRACE_SECU("PubScrClrs_PINPAD:Response Code Erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}	
	return APP_SUCC;

}


int PubSetFontSize_PINPAD(char cSize)
{
	int nRet = 0;
	int nLen = 0;
	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubDispImg_PINPAD:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;
	
	memcpy(sSendBuf, "5a",2);
	sSendBuf[2] = SEPERATOR_CMD;
	sSendBuf[3] = cSize;
	nLen = 4;
	
	nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("SetFontSize:PinPad_SendRecv APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}


	if(memcmp(sRecvBuf, "5b", 2) != 0)
	{
		PINPAD_TRACE_SECU("SetFontSize:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}

	if(memcmp(sRecvBuf+3, "00", 2) != 0)
	{
		PINPAD_TRACE_SECU("SetFontSize:Response Code Erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}	
	return APP_SUCC;

}



int PubCreateFile_PINPAD(char *pszFileName)
{
	int nRet = 0;
	int nLen = 0;
	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubDispImg_PINPAD:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;


	memcpy(sSendBuf, "46",2);
	sSendBuf[2] = SEPERATOR_CMD;
	sSendBuf[3] = strlen(pszFileName);
	strcpy((char *)sSendBuf+4, pszFileName);
	nLen = 4 + strlen(pszFileName);
	
	nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("Create File:PinPad_SendRecv APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}


	if(memcmp(sRecvBuf, "47", 2) != 0)
	{
		PINPAD_TRACE_SECU("Create File:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}

	if(memcmp(sRecvBuf+3, "00", 2) != 0)
	{
		PINPAD_TRACE_SECU("Create File:Response Code Erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}	


	return APP_SUCC;	
}


int PubLoadFile_PINPAD(char *pszPath)
{
	int nRet = 0;
	int nLen = 0;
	int nGotLen = 0;
	int nBlockLen = 1024;
	
	FILE *fp = NULL;

	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubDispImg_PINPAD:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;


	memcpy(sSendBuf, "48",2);
	sSendBuf[2] = SEPERATOR_CMD;

	fp = fopen(pszPath,"r");
	if(fp == NULL)
	{
		return APP_FAIL;
	}
	
	//fseek(fp, 0, SEEK_END);
	//nFileLen = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	

	while(1)
	{

		nGotLen = fread(sSendBuf+5,1,nBlockLen,fp);
		if(nGotLen < 0)
		{
			fclose(fp);
			PINPAD_TRACE_SECU("Fread Error");
			ProSetSecurityErrCode(ERR_PINPAD_FAIL, 0);
			return APP_FAIL;			
		}
		else if(nGotLen == 0)//fread finished
		{
			break;
		}
		else 
		{
			PubIntToC2(sSendBuf+3, nGotLen);
			nLen = 5 + nGotLen;
			nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
			if (nRet != APP_SUCC)
			{
				fclose(fp);
				PINPAD_TRACE_SECU("Load File:PinPad_SendRecv APP_FAIL");
				ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
				return APP_FAIL;
			}
			
			
			if(memcmp(sRecvBuf, "49", 2) != 0)
			{
				fclose(fp);
				PINPAD_TRACE_SECU("Load File:CMD erro");
				ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
				return APP_FAIL;
			}
			
			if(memcmp(sRecvBuf+3, "00", 2) != 0)
			{
				fclose(fp);
				PINPAD_TRACE_SECU("Load File:Response Code Erro");
				ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
				return APP_FAIL;
			}

		
			if(nGotLen != nBlockLen) //fread finished
				break;
		}
		
		
	}

	fclose(fp);
	return APP_SUCC;	
}

int PubUpdateFile_PINPAD(char *pszFileName, char cRebootFlag)
{
	int nRet = 0;
	int nLen = 0;
	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("UpdateFile:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;


	memcpy(sSendBuf, "44",2);
	sSendBuf[2] = SEPERATOR_CMD;
	sSendBuf[3] = strlen(pszFileName);
	strcpy((char *)sSendBuf+4, pszFileName);
	sSendBuf[4+strlen(pszFileName)] = cRebootFlag;
	nLen = 4 + strlen(pszFileName) + 1;
	
	
	nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("Update File:PinPad_SendRecv APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}


	if(memcmp(sRecvBuf, "45", 2) != 0)
	{
		PINPAD_TRACE_SECU("Update File:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}

	if(memcmp(sRecvBuf+3, "00", 2) != 0)
	{
		PINPAD_TRACE_SECU("Update File:Response Code Erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}	

	return APP_SUCC;	
}

int PubSwipeCard_PINPAD(char *pIn, char *pOut)
{
	int nRet;
	int nLen, nOff = 0;
	char szResp[2+1] = {0};
	int nPanLen, nCipherLen = 0;
	int nTrack1Status, nTrack2Status, nTrack3Status;
	int nTrack1Len, nTrack2Len, nTrack3Len;
	STREADCARD_IN *pstCardIn = (STREADCARD_IN *)pIn;
	STREADCARD_OUT *pstCardOut = (STREADCARD_OUT *)pOut;

	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubDispImg_PINPAD:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;

	memcpy(sSendBuf, "J0",2);
	nOff += 2;
	sSendBuf[nOff] = SEPERATOR_CMD;
	nOff += 1;
	sSendBuf[nOff] = pstCardIn->nIndex; // index
	nOff += 1;
	sSendBuf[nOff] = pstCardIn->nKeyType; // key Type 0 ¡§C DES 1 ¡§C AES
	nOff += 1;
   	sSendBuf[nOff] = pstCardIn->nEncryMode; // Encrypting  key mode  0-DES_CBC 1-DES_ECB 2-AES
   	nOff += 1;
	PubIntToC2(sSendBuf +nOff, 1770);
	nOff += 2;
	sSendBuf[nOff] = 0x01; // track1
	nOff += 1;
	sSendBuf[nOff] = 0x01; // track2
	nOff += 1;
	sSendBuf[nOff] = 0x01; // track3
	nOff += 1;
	if (pstCardIn->nEncryMode == 0x00)
	{
		memcpy((char *)sSendBuf + nOff, "\x00\x00\x00\x00\x00\x00\x00\x00", 8); // CBC IV
		nOff += 8;
	}
	strcpy((char *)sSendBuf + nOff, "pls swipe card"); // disp data
	nOff += 16;
	sSendBuf[nOff] = 0x1C;
	nOff += 1;
	strcpy((char *)sSendBuf + nOff, "pls swipe card"); // disp data
	nOff += 16;
	sSendBuf[nOff] = 0x1C;
	nOff += 1;
	strcpy((char *)(char *)sSendBuf + nOff, "pls swipe card"); // disp data
	nOff += 16;
	sSendBuf[nOff] = 0x1C;
	nOff += 1;
	strcpy((char *)sSendBuf + nOff, "pls swipe card"); // disp data
	nOff += 16;
	sSendBuf[nOff] = 0x1C;
	nOff += 1;

	PubDebugData("send buff:", sSendBuf, nOff);
	nLen = nOff;
	nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("Create File:PinPad_SendRecv APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}
	PubDebugData("recv buff:", sRecvBuf, nLen);

	nOff = 0;
	if(memcmp(sRecvBuf, "J1", 2) != 0)
	{
		PINPAD_TRACE_SECU("Create File:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}

	nOff += 3;
	memcpy(szResp, (char *)sRecvBuf+nOff, 2); // response code
	if(memcmp(szResp, "00", 2) != 0)
	{
		PINPAD_TRACE_SECU("Create File:Response Code Erro %s", szResp);
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}

	nOff += 2;
	nPanLen = sRecvBuf[nOff];
	pstCardOut->nPanLen = nPanLen;
	if (pstCardIn->nIndex > 0) // encry
	{
		nCipherLen = 32;
	}
	else
	{
		nCipherLen = nPanLen;
	}
	PINPAD_TRACE_SECU("panlen = %d", nPanLen);
	nOff += 1;
	memcpy(pstCardOut->szCipherPan , sRecvBuf + nOff, nCipherLen);
	PINPAD_TRACEHEX_SECU(pstCardOut->szCipherPan, nCipherLen, "szCipherPan");
	nOff += nCipherLen;
	memcpy(pstCardOut->szMaskPan, sRecvBuf + nOff, nPanLen);
	PINPAD_TRACE_SECU("szMaskPan = %s", pstCardOut->szMaskPan);
	nOff += nPanLen;
	nTrack1Status = sRecvBuf[nOff];
	nOff += 1;
	PINPAD_TRACE_SECU("nTrack1Status = %d", nTrack1Status);
	nTrack2Status = sRecvBuf[nOff];
	nOff += 1;
	PINPAD_TRACE_SECU("nTrack2Status = %d", nTrack2Status);
	nTrack3Status = sRecvBuf[nOff];
	nOff += 1;
	PINPAD_TRACE_SECU("nTrack3Status = %d", nTrack3Status);
	PubC2ToInt((uint*)&nLen, (uchar *)sRecvBuf + nOff);
	(pstCardIn->nIndex == 0) ? (nTrack1Len = nLen) : (nTrack1Len = nLen + (nLen % 16 == 0 ? 0 : (16 - nLen % 16)));
	nOff += 2;
	if (nTrack1Len > 0)
	{
		PINPAD_TRACEHEX_SECU(sRecvBuf + nOff, nTrack1Len, "track1");
	}
	nOff += nTrack1Len;

	PubC2ToInt((uint*)&nLen, (uchar *)sRecvBuf + nOff);
	(pstCardIn->nIndex == 0) ? (nTrack2Len = nLen) : (nTrack2Len = nLen + (nLen % 16 == 0 ? 0 : (16 - nLen % 16)));
	nOff += 2;
	if (nTrack2Len > 0)
	{
		memcpy(pstCardOut->szTrack2, sRecvBuf + nOff, nTrack2Len);
		PINPAD_TRACEHEX_SECU(pstCardOut->szTrack2, nTrack2Len, "track2");
	}
	nOff += nTrack2Len;

	pstCardOut->nTrack2Len = nTrack2Len;

	PubC2ToInt((uint*)&nLen, (uchar *)sRecvBuf + nOff);
	(pstCardIn->nIndex == 0) ? (nTrack3Len = nLen) : (nTrack3Len = nLen + (nLen % 16 == 0 ? 0 : (16 - nLen % 16)));
	nOff += 2;
	if (nTrack3Len > 0)
	{
		memcpy(pstCardOut->szTrack3, sRecvBuf + nOff, nTrack3Len);
		PINPAD_TRACEHEX_SECU(pstCardOut->szTrack3, nTrack3Len, "track3");
	}
	nOff += nTrack3Len;
	pstCardOut->nTrack3Len = nTrack3Len;

	return APP_SUCC;	
}

static int PackL3SendData(STPINPADL3_IN *pstL3Cfg, STPINPADMSGTYPE stMsgType, char *pszOutData, int *pnOutLen)
{
	int nOff;
	char *pszInputData;
	int nInputLen;

	pszInputData = pstL3Cfg->pszInputData;
	nInputLen = pstL3Cfg->nInputDataLen;

	nOff = 0;
	memcpy(pszOutData + nOff, stMsgType.szReq, 2);
	nOff += 2;
	pszOutData[nOff] = SEPERATOR_CMD;
	nOff += 1;
	pszOutData[nOff] = stMsgType.cFuncId; // func id
	nOff += 1;
	if (pstL3Cfg->cCardInteface > 0)
	{
		pszOutData[nOff] = pstL3Cfg->cCardInteface;
		nOff += 1;
	}

	if (pstL3Cfg->cMsgType == PINPAD_PERFORMTRANS)
	{
		memcpy(pszOutData + nOff, pstL3Cfg->pszInputData, 4); // time out
		nOff += 4;
		pszInputData = pstL3Cfg->pszInputData + 4;
		nInputLen = pstL3Cfg->nInputDataLen - 4;
	}

	if (pstL3Cfg->cMsgType == PINPAD_SETTERMINALCFG || pstL3Cfg->cMsgType == PINPAD_UPDATEAID || pstL3Cfg->cMsgType == PINPAD_PERFORMTRANS)
	{
		PubIntToC2((uchar *)pszOutData + nOff, (uint)nInputLen);
		nOff += 2;
	}

	if (pszInputData && nInputLen > 0)
	{
		memcpy(pszOutData + nOff, pszInputData, nInputLen);
		nOff += nInputLen;
	}

	*pnOutLen = nOff;
	return APP_SUCC;
}

int PubL3OrderSet_PINPAD(char *pL3Cfg, char *pszOutPut, int *pnOutPutLen)
{
	char sDebugBuff[4096 + 1] = {0};
	char sRecvBuf[8192+1] = {0};
	int nReadLen = 0;
	int nRet;
	int nOff = 0;
	int nTotalLen = 0;
	int nCmdDataLen = 0;
	L3_CALLBACK emCallbackID = 0xFF;
	L3_UIID emEventID = 0xFF;
	char cAck = ACK;

	nRet = PubL3PerformSend_PINPAD(pL3Cfg);
	if (nRet != APP_SUCC)
	{
		return APP_FAIL;
	}

	while (1)
	{
		nOff = 0;
		nReadLen = 0;
		nRet = PubRecv_PINPAD(sRecvBuf + nTotalLen, 8192 - nTotalLen, &nReadLen); // 8192 = 8K
		if (nRet != APP_SUCC)
		{
			PINPAD_TRACE_SECU("nRet = %d nReadLen = %d", nRet, nReadLen);
			return APP_FAIL;
		}
		nTotalLen += nReadLen;
		if (PubKbHit() == KEY_ESC)
		{
			PINPAD_TRACE_SECU("emEventID = %d emCallbackID = %d", emEventID, emCallbackID);
			if (emEventID == UI_PRESENT_CARD)
			{
				PubL3CancalReadCard();
			}
			if (emCallbackID == L3_CALLBACK_GET_PIN || emCallbackID == L3_CALLBACK_SELECT_CANDIDATE_LIST)
			{
//				continue; // press cancel button in POS(not pinpad) to quit enter PIN.
			}
			return APP_QUIT;
		}

		if (nTotalLen == 0)
		{
			continue;
		}

		// cmd format:02 + len(2) + msgtype(2) + seprator(1) + funcID + ...
		if (sRecvBuf[nOff] != 0x02)
		{
			nOff += 1;
			memmove(sRecvBuf, sRecvBuf + nOff, nTotalLen - nOff);
			nTotalLen -= 1;
			nOff -= 1;
			PINPAD_TRACE_SECU("sRecvBuf[nOff] = %x", sRecvBuf[nOff]);
			continue;
		}
		nOff += 1; // 02
		PubBcdToInt(sRecvBuf + nOff, &nCmdDataLen);
		nOff += 2; // len(2)
		if (nTotalLen < nCmdDataLen + nOff)
		{
			continue;
		}

		if (memcmp(sRecvBuf + nOff, "L1", 2) != 0)
		{
			PINPAD_TRACEHEX_SECU(sRecvBuf, nCmdDataLen + nOff, "data format is not match");
			return APP_FAIL;
		}
		nOff += 3; // msgtype(2) + seprator(1)
		if (sRecvBuf[nOff] == FUNCID_OUTPUTDEBUG)
		{
			int nDebugDataLen;
			int nAllDbgLen = 0;

			while (1)
			{
				nOff += 1;
				nOff += 2; // response code
				PubC2ToInt((uint *)&nDebugDataLen, (uchar *)sRecvBuf + nOff);
				nOff += 2;
				memcpy(sDebugBuff + nAllDbgLen, sRecvBuf + nOff, nDebugDataLen);
				nAllDbgLen += nDebugDataLen;
				nOff += nDebugDataLen;
				nOff += 2;
				if (sRecvBuf[nOff] != 0x02)
				{
					break;
				}
				nOff += 1; // 02
				PubBcdToInt(sRecvBuf + nOff, &nCmdDataLen);
				nOff += 2; // len(2)
				if (nTotalLen < nCmdDataLen + nOff)
				{
					nOff -= 3;
					break;
				}
				nOff += 3; // msgtype(2) + seprator(1)
				if (sRecvBuf[nOff] != FUNCID_OUTPUTDEBUG)
				{
					nOff -= 6;
					break;
				}
			}
			PubBufToAux(sDebugBuff, nAllDbgLen);
			memmove(sRecvBuf, sRecvBuf + nOff, nTotalLen - nOff);
			nTotalLen -= nOff;
			continue;
		}
		else if (sRecvBuf[nOff] == FUNCID_L3CALLBACK)
		{
			PINPAD_TRACEHEX_SECU(sRecvBuf, nCmdDataLen + 5, "recv ");
			nOff += 1;
			emCallbackID = sRecvBuf[nOff];
			if (emCallbackID == L3_CALLBACK_UI_EVENT)
			{
				emEventID = sRecvBuf[nOff+1];
			}
			nOff += 1;
			ASSERT_FAIL(PerformCallBack(emCallbackID, sRecvBuf + nOff));
			memmove(sRecvBuf, sRecvBuf + nCmdDataLen + 5, nTotalLen - nCmdDataLen - 5); // 5 = 02 + length(2) + 03 + LRC
			nTotalLen -= (nCmdDataLen + 5);
			continue;
		}
		else
		{
			PINPAD_TRACEHEX_SECU(sRecvBuf, nCmdDataLen + 5, "recv ");
			PubSend_PINPAD((char *)&cAck, 1);
			nOff += 1;
			if (memcmp(sRecvBuf + nOff, "00", 2) != 0)
			{
				PINPAD_TRACE_SECU("fail response code %2s", sRecvBuf + nOff);
				return APP_FAIL;
			}
			nOff += 2;
			if (pszOutPut)
			{
				memcpy(pszOutPut, sRecvBuf + nOff, nCmdDataLen - 6);
			}
			if (pnOutPutLen)
			{
				*pnOutPutLen = nCmdDataLen - 6; // 6 msgtype(2) + seprator(1) + funcId + response code
			}
			break;
		}
	}

	return APP_SUCC;
}

void PubL3CancalReadCard()
{
	int nRet;
	
	PINPAD_TRACE_SECU("send data :+++CANCEL");
	nRet = NAPI_PortWrite(nPinpadPort, (uchar *)"+++CANCEL", 9);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("cancel read card fail = %d",nRet);
		return;
	}
}

/**
* @brief check ICC
* @return description
* @retval app_succ IC card detected
*/
int PubCheckIcc_PINPAD()
{
	int nRet, nLen;
	int nOff = 0;
	char cCardType;
	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubScrClrs:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;
	
	memcpy(sSendBuf, "BM", 2);
	nOff += 2;
	sSendBuf[nOff] = SEPERATOR_CMD;
	nOff += 1;
	memcpy(sSendBuf + nOff, "remove card", 11);
	nOff += 11;
	sSendBuf[nOff] = 0x1C;

	nRet = PinPad_SendRecv(1, sSendBuf, nOff, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubScrClrs_PINPAD:PinPad_SendRecv APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}

	nOff = 0;
	if(memcmp(sRecvBuf, "BN", 2) != 0)
	{
		PINPAD_TRACE_SECU("PubScrClrs_PINPAD:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}
	nOff += 3;
	if(memcmp(sRecvBuf+nOff, "00", 2) != 0)
	{
		return APP_QUIT;
	}
	nOff += 2;
	cCardType = sRecvBuf[nOff];
	if (cCardType == 0x01) { // 01 -IC 02 - RF
		return APP_SUCC;
	}
	
	return APP_QUIT;
}

int PubBeep_PINPAD()
{
	int nRet, nLen;
	int nOff = 0;
	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubScrClrs:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;
	
	memcpy(sSendBuf, "39", 2);
	nOff += 2;
	sSendBuf[nOff] = SEPERATOR_CMD;
	nOff += 1;
	sSendBuf[nOff] = 0x00;
	nOff += 1;
	PubIntToC2(sSendBuf + nOff, 100);
	nOff += 2;

	nRet = PinPad_SendRecv(0, sSendBuf, nOff, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubScrClrs_PINPAD:PinPad_SendRecv APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}

	return APP_SUCC;
}

int PubReboot_PINPAD()
{
	int nRet, nLen;
	int nOff = 0;
	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubScrClrs:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;

	memcpy(sSendBuf, "A2", 2);
	nOff += 2;
	sSendBuf[nOff] = SEPERATOR_CMD;
	nOff += 1;

	nRet = PinPad_SendRecv(0, sSendBuf, nOff, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubScrClrs_PINPAD:PinPad_SendRecv APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}

	nOff = 0;
	if(memcmp(sRecvBuf, "A3", 2) != 0)
	{
		PINPAD_TRACE_SECU("PubScrClrs_PINPAD:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}

	return APP_SUCC;
}

// Dukpt
int PubIncDukptKSN_PINPAD(int nKeyIndex)
{
	int nRet, nLen;
	int nOff = 0;
	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubScrClrs:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;

	memcpy(sSendBuf, "7g", 2);
	nOff += 2;
	sSendBuf[nOff] = SEPERATOR_CMD;
	nOff += 1;
	if (nKeyIndex < 1 || nKeyIndex > 250)
	{
		PINPAD_TRACE_SECU("invalid key value (%d)", nKeyIndex);
		return APP_FAIL;
	}
	sSendBuf[nOff] = nKeyIndex;
	nOff += 1;

	nRet = PinPad_SendRecv(1, sSendBuf, nOff, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubScrClrs_PINPAD:PinPad_SendRecv APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}

	nOff = 0;
	if(memcmp(sRecvBuf, "7h", 2) != 0)
	{
		PINPAD_TRACE_SECU("PubScrClrs_PINPAD:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}

	nOff += 3;
	if(memcmp(sRecvBuf+nOff, "00", 2) != 0)
	{
		return APP_QUIT;
	}

	return APP_SUCC;
}

int PubGetDukptKSN_PINPAD(int nKeyIndex, char *pszKsn)
{
	int nRet, nLen;
	int nOff = 0;
	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubScrClrs:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;

	memcpy(sSendBuf, "34", 2);
	nOff += 2;
	sSendBuf[nOff] = SEPERATOR_CMD;
	nOff += 1;
	if (nKeyIndex < 1 || nKeyIndex > 250)
	{
		PINPAD_TRACE_SECU("invalid key value (%d)", nKeyIndex);
		return APP_FAIL;
	}
	sSendBuf[nOff] = nKeyIndex;
	nOff += 1;

	nRet = PinPad_SendRecv(1, sSendBuf, nOff, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubScrClrs_PINPAD:PinPad_SendRecv APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}

	nOff = 0;
	if(memcmp(sRecvBuf, "35", 2) != 0)
	{
		PINPAD_TRACE_SECU("PubScrClrs_PINPAD:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}
	nOff += 3;
	if(memcmp(sRecvBuf+nOff, "00", 2) != 0)
	{
		return APP_QUIT;
	}
	nOff += 2;
	memcpy(pszKsn, sRecvBuf + nOff, 10);

	return APP_SUCC;
}

int PubL3PerformSend_PINPAD(char *pL3Cfg)
{
	int nRet;
	int nSendLen;
	STPINPADMSGTYPE stMsgType;

	STPINPADL3_IN *pstL3Cfg = (STPINPADL3_IN *)pL3Cfg;
	if (pL3Cfg == NULL)
	{
		PINPAD_TRACE_SECU("pL3Cfg is null");
		return APP_FAIL;
	}

	ASSERT_FAIL(LoadMsgType(pstL3Cfg->cMsgType , &stMsgType));

	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("UpdateFile:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;

	ASSERT_FAIL(PackL3SendData(pstL3Cfg, stMsgType, (char *)sSendBuf, &nSendLen));
	ASSERT_FAIL(PinPad_InsertAsyn((uchar *)sSendBuf, (uint *)&nSendLen));
	NAPI_KbFlush();
	PINPAD_TRACEHEX_SECU(sSendBuf, nSendLen, "send");
	nRet = NAPI_PortWrite(nPinpadPort, (uchar *)sSendBuf, nSendLen);
	if (nRet != NAPI_OK)
	{
		gcPortStatus = NO;
		PINPAD_TRACE_SECU("NAPI_PortWrite nRet = %d", nRet);
		return APP_FAIL;
	}
	return APP_SUCC;
}

int PubRecv_PINPAD(char *psRecv, int nMaxReadLen, int *pnRecvLen)
{
	int nRet;
	int nReadLen;

	if (pnRecvLen)
	{
		*pnRecvLen = 0;
	}
	nRet = NAPI_PortReadLen(nPinpadPort, &nReadLen);
	if (nRet != NAPI_OK)
	{
		PINPAD_TRACE_SECU("Read Port Data Length Error,nRet=%d", nRet);
		return APP_FAIL;
	}

	if (nReadLen >= PORTBUFSIZE)
	{
		PINPAD_TRACE_SECU("out of buff size (%d)", nReadLen);
		return APP_FAIL;
	}

	if (nReadLen > nMaxReadLen)
	{
		nReadLen = nMaxReadLen;
	}
	if (nReadLen > 0)
	{
		nRet = NAPI_PortRead(nPinpadPort, (uchar *)psRecv, &nReadLen, 1000);
		if (nRet != NAPI_OK && nRet != NAPI_ERR_TIMEOUT)
		{
			PINPAD_TRACE_SECU("nRet = %d", nRet);
			return APP_FAIL;
		}
		*pnRecvLen = nReadLen;

		return APP_SUCC;
	}

	return APP_SUCC;
}

int PubSend_PINPAD(char *psSend, int nSendLen)
{
	int nRet;

	PINPAD_TRACEHEX_SECU(psSend, nSendLen, "send data:");
	nRet = NAPI_PortWrite(nPinpadPort, (uchar *)psSend, nSendLen);
	if (nRet != NAPI_OK)
	{
		PINPAD_TRACE_SECU("nRet = %d", nRet);
		return APP_FAIL;
	}

	return APP_SUCC;
}

static int ParseOutDlg(uchar *pszData, int nLen, STOUTDLG *pstOutDlg)
{
	int nOff = 0;
	int nTagLen = 0;
	char szTag[2+1] = {0};

	while(1)
	{
		if (nOff >= nLen)
		{
			break;
		}
		memcpy(szTag, pszData + nOff, 2);
		if (memcmp(szTag, "\xDF\x01", 2) == 0) {
			nOff += 4;
			nTagLen = pszData[nOff];
			nOff += 1;
			memcpy(pstOutDlg->szPhone, pszData + nOff, nTagLen);
			nOff += nTagLen;
			continue;
		} else if (memcmp(szTag, "\xDF\x02", 2) == 0) {
			nOff += 4;
			nTagLen = pszData[nOff];
			nOff += 1;
			memcpy(pstOutDlg->szZip, pszData + nOff, nTagLen);
			nOff += nTagLen;
			continue;
		} else if (memcmp(szTag, "\xDF\x03", 2) == 0) {
			nOff += 4;
			nTagLen = pszData[nOff];
			nOff += 1;
			memcpy(pstOutDlg->szZipAddr, pszData + nOff, nTagLen);
			nOff += nTagLen;
			continue;
		} else if (memcmp(szTag, "\xDF\x04", 2) == 0) {
			nOff += 4;
			nTagLen = pszData[nOff];
			nOff += 1;
			memcpy(pstOutDlg->szPan, pszData + nOff, nTagLen);
			nOff += nTagLen;
			continue;
		} else if (memcmp(szTag, "\xDF\x05", 2) == 0) {
			nOff += 4;
			nTagLen = pszData[nOff];
			nOff += 1;
			memcpy(pstOutDlg->szExpireDate, pszData + nOff, nTagLen);
			nOff += nTagLen;
			continue;
		} else if (memcmp(szTag, "\xDF\x06", 2) == 0) {
			nOff += 4;
			nTagLen = pszData[nOff];
			nOff += 1;
			memcpy(pstOutDlg->szCvv, pszData + nOff, nTagLen);
			nOff += nTagLen;
			continue;
		} else if (memcmp(szTag, "\xDF\x07", 2) == 0) {
			nOff += 4;
			nTagLen = pszData[nOff];
			nOff += 1;
			memcpy(pstOutDlg->szGiftCardPIN, pszData + nOff, nTagLen);
			nOff += nTagLen;
			continue;
		} else {
			break;
		}
	}
	return APP_SUCC;
}

int PubInputDlg_PINPAD(char *pszData, int nDataLen, char cInputNum, STINPUTDLG *pstInputDlg, STOUTDLG *pstOutDlg)
{
	int nRet, nLen;
	int nOff = 0;
	int i;

	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubScrClrs:ProInitPinpadAux APP_FAIL");
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;

	memcpy(sSendBuf, "6k", 2);
	nOff += 2;
	sSendBuf[nOff++] = SEPERATOR_CMD;
	PubIntToC2((uchar *)sSendBuf + nOff, (uint)nDataLen);
	nOff += 2;
	if (pszData == NULL) {
		memcpy(sSendBuf + nOff, "\x00\x00", 2);
	} else {
		memcpy(sSendBuf + nOff, pszData, nDataLen);
	}
	nOff += nDataLen;
	sSendBuf[nOff++] = cInputNum;
	for (i = 0; i < cInputNum; i++) {
		sSendBuf[nOff++] = pstInputDlg[i].cInputFieldID;
		sSendBuf[nOff++] = pstInputDlg[i].cInputMode;
		sSendBuf[nOff++] = pstInputDlg[i].cInputFormat;
		sSendBuf[nOff++] = pstInputDlg[i].cMinLen;
		sSendBuf[nOff++] = pstInputDlg[i].cMaxLen;
		PubIntToC2((uchar *)sSendBuf + nOff, pstInputDlg[i].unTimeout);
		nOff += 2;
	}

	nRet = PinPad_SendRecv(1, sSendBuf, nOff, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubInputDlg_PINPAD:PinPad_SendRecv APP_FAIL");
		return APP_FAIL;
	}

	nOff = 0;
	if(memcmp(sRecvBuf, "6l", 2) != 0)
	{
		PINPAD_TRACE_SECU("PubInputDlg_PINPAD:CMD erro");
		return APP_FAIL;
	}
	nOff += 3;
	if(memcmp(sRecvBuf+nOff, "00", 2) != 0)
	{
		PINPAD_TRACE_SECU("PubInputDlg_PINPAD:%2.2s", sRecvBuf+nOff);
		return APP_QUIT;
	}
	nOff += 2;
	PubC2ToInt((uint *)&nLen, (uchar *)sRecvBuf + nOff);
	nOff += 2;

	ParseOutDlg(sRecvBuf + nOff, nLen, pstOutDlg);
	return APP_SUCC;
}



/***************************************************************************
** Copyright (c) 2019 Newland Payment Technology Co., Ltd All right reserved
** File name:  comm.c
** File indentifier:
** Synopsis:
** Current Verion:  v1.0
** Auther: sunh
** Complete date: 2016-7-4
** Modify record:
** Modify date:
** Version:
** Modify content:
***************************************************************************/

#include "apiinc.h"
#include "libapiinc.h"
#include "appinc.h"
#include "wifi.h"
#include "napi_wlm.h"

/**
* define global variable
*/
static STAPPCOMMPARAM gstAppCommParam; /**<communication parameter of application*/

/**
* internal function
*/
static int CommParamAppToPub(const STAPPCOMMPARAM *, STCOMMPARAM *);
static int SetFuncCommMode(void);
static int SetFuncCommIsPreDial(void);
static int SetFuncCommTimeOut(void);
static int SetFuncCommTelNo(void);
static int SetFuncCommWLMDialNum(void);
static int SetFuncSimPassWd(void);
static int SetFuncCommAPN(void);
static int SetFuncCommPort(void);
static int SetFuncCommBackPort(void);
static int SetFuncCommIp(void);
static int SetFuncCommBackIp(void);
static int SetFuncCommIpAddr(void);
static int SetFuncCommMask(void);
static int SetFuncCommGate(void);
static int SetFuncCommReDialNum(void);
static int SetFuncAuxIsAddTpdu(void);
static int SetFuncWifiMode(void);
static int SetFuncWifiPwd(void);
static int SetFuncWifiSsid(void);
static int SetFuncIsUseDns(void);
static int SetFuncIsDhcp(void);
static YESORNO GetControlCommInit(void);
static int SetFuncDomain(void);
static int SetFuncDNSIp1(void);
static int SetFuncDNSIp2(void);
static int SetFuncIsSSL(void);
static int GetVarIsSSL(void);
#ifdef USE_NTOMS
static int SetFuncTOMSAppDomain(void);
static int SetFuncTOMSParamDomain(void);
static int SetFuncTOMSKeyPosDomain(void);
static int SetFuncTOMSFileServerDomain(void);
static int SetFuncTOMSTdasDomain(void);
#endif

/**
* Interface function implementation
*/
static char gcIsCommInit = YES;/**<to record if need to initialize, YES-need*/

/**
** brief: ConTrol need to initialize commucation
** param: void
** return: void
** auther:
** date: 2016-7-4
** modify:
*/
void SetControlCommInit(void)
{
	gcIsCommInit = YES;
}

/**
** brief: Get the flag if need to initialize
** param: void
** return: YESORNO
** auther:
** date: 2016-7-4
** modify:
*/
static YESORNO GetControlCommInit(void)
{
	if (gcIsCommInit==YES)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}

/**
** brief: Set default communication parameters, it should run one time at first run.
** param: void
** return:
** li APP_SUCC
** li APP_FAIL
** auther:
** date: 2016-7-4
** modify:
*/
int InitCommParam(void)
{
	char szBuf[50] = {0};

	memset(&gstAppCommParam, 0, sizeof(STAPPCOMMPARAM));
	if (APP_SUCC != PubGetHardwareSuppot(HARDWARE_SUPPORT_WIRELESS,szBuf))
	{
		gstAppCommParam.cCommType = COMM_DIAL;

	}
	else if (0 == memcmp(szBuf, "GPRS", 4))
	{
		gstAppCommParam.cCommType = COMM_GPRS;
	}
	else if (0 == memcmp(szBuf, "CDMA", 4))
	{
		gstAppCommParam.cCommType = COMM_CDMA;
	}
	else
	{
		gstAppCommParam.cCommType = COMM_DIAL;
	}
	gstAppCommParam.cCommType2 = COMM_NONE;
	gstAppCommParam.cPreDialFlag = 1;
	strcpy(gstAppCommParam.szPreDial,"");
	strcpy(gstAppCommParam.szTelNum1, "66522028");
	strcpy(gstAppCommParam.szTelNum2, "66522028");
	strcpy(gstAppCommParam.szTelNum3, "66522028");
	gstAppCommParam.cTimeOut = 60;
	memcpy(gstAppCommParam.sTpdu, "\x60\x00\x12\x00\x00", 5);
	gstAppCommParam.cReDialNum = 3;
	gstAppCommParam.cReSendNum = 3;
	gstAppCommParam.cOffResendNum=3;
	strcpy(gstAppCommParam.szAPN1, "CMNET");
	strcpy(gstAppCommParam.szAPN2, "CMNET");
	strcpy(gstAppCommParam.szDNSIp1, "8.8.8.8");
	strcpy(gstAppCommParam.szDNSIp2, "8.8.8.8");
	strcpy(gstAppCommParam.szDNSIp3, "8.8.8.8");
	strcpy(gstAppCommParam.szIp1, "218.66.48.231");
	strcpy(gstAppCommParam.szIp2, "218.66.48.231");
	strcpy(gstAppCommParam.szPort1, "3456");
	strcpy(gstAppCommParam.szPort2, "3456");
	strcpy(gstAppCommParam.szIpAddr, "192.168.5.114");
	strcpy(gstAppCommParam.szMask, "255.255.255.0");
	strcpy(gstAppCommParam.szGate, "192.168.5.254");
	gstAppCommParam.cMode = 1;
	if (gstAppCommParam.cCommType == COMM_GPRS)
	{
		strcpy(gstAppCommParam.szWirelessDialNum, "D*99***1#");/**<"D*99***1#"*/
	}
	else if (gstAppCommParam.cCommType == COMM_CDMA)
	{
		strcpy(gstAppCommParam.szWirelessDialNum, "D#777");/**<"D#777"*/
		gstAppCommParam.cMode = 0;
	}
	else
	{
		strcpy(gstAppCommParam.szWirelessDialNum, "D*99***1#");/**<"D*99***1#"*/
	}

	strcpy(gstAppCommParam.szPdpType, "IP");
	strcpy(gstAppCommParam.szUser, "card");
	strcpy(gstAppCommParam.szPassWd, "card");
	strcpy(gstAppCommParam.szSIMPassWd, "1234");
	strcpy(gstAppCommParam.szDomain, "www.163.com");
	gstAppCommParam.cAuxIsAddTpdu= 1;
	gstAppCommParam.cIsDns = 0;
	gstAppCommParam.cIsDHCP = 1;
	strcpy(gstAppCommParam.szWifiSsid,"ssid");
	strcpy(gstAppCommParam.szWifiKey,"password");
	gstAppCommParam.cWifiMode = WIFI_AUTH_OPEN;
	memcpy(gstAppCommParam.szDomain2,gstAppCommParam.szDomain,50);
	memcpy(gstAppCommParam.szNii,"280",3);
	gstAppCommParam.cIsSSL = 0;

#ifdef DEV
	/* Tetsting URL */
	strcpy(gstAppCommParam.szTOMSAppDomain, "http://218.66.48.231:6033/");
	strcpy(gstAppCommParam.szTOMSParamDomain, "http://218.66.48.231:6082/");
	strcpy(gstAppCommParam.szTOMSKeyPosDomain, "http://218.66.48.231:6039/");
    strcpy(gstAppCommParam.szTOMSFileServerDomain, "http://218.66.48.231:6030/");
    strcpy(gstAppCommParam.szTOMSTdasDomain, "http://218.66.48.231:6036/");
	/* Production URL */
#else
	strcpy(gstAppCommParam.szTOMSAppDomain, "https://toms-t.newlandpayment.com:6043/");
	strcpy(gstAppCommParam.szTOMSParamDomain, "https://toms.newlandpayment.com/");
	strcpy(gstAppCommParam.szTOMSKeyPosDomain, "https://toms-t.newlandpayment.com:6049/");
	strcpy(gstAppCommParam.szTOMSFileServerDomain, "http://toms-t.newlandpayment.com:6040/");
	strcpy(gstAppCommParam.szTOMSTdasDomain, "https://toms-t.newlandpayment.com:6046/");
#endif

	InitCommParamFile(gstAppCommParam);

	return APP_SUCC;
}

/**
** brief: Initialize communication, like init modem,WIFI...
** param: void
** return: void
** li APP_SUCC
** li APP_FAIL
** auther:
** date: 2016-7-4
** modify:
*/
int CommInit(void)
{
	int nRet = 0;
	STCOMMPARAM stPubCommParam;
	static STCOMMPARAM stOldPubCommParam = {0};
	static char cInitFlag = 0;

	memset(&stPubCommParam, 0, sizeof(STCOMMPARAM));
	CommParamAppToPub(&gstAppCommParam, &stPubCommParam);

	if (memcmp(&stOldPubCommParam, &stPubCommParam, sizeof(STCOMMPARAM)) != 0 || 0 == cInitFlag || YES == GetControlCommInit())
	{
		SetStatusBar(STATUSBAR_OTHER_OPEN);
		PubClearAll();
		PubDisplayGen(2, tr("Init communication"));
		PubDisplayGen(3, tr("Please wait..."));
		PubUpdateWindow();
		nRet = PubCommInit(&stPubCommParam);
		if (nRet != APP_SUCC)
		{
			if(gstAppCommParam.cCommType2 != COMM_NONE)
			{
				nRet = SwitchCommType();
				if (nRet != APP_SUCC)
				{
					PubDispErr(tr("Comm Fail"));
					cInitFlag = 0;
					return APP_FAIL;
				}
			}
			else
			{
				PubDispErr(tr("Comm Fail"));
				cInitFlag = 0;
				return APP_FAIL;
			}
		}

		memcpy(&stOldPubCommParam, &stPubCommParam, sizeof(STCOMMPARAM));/**<save the old parameters*/
		cInitFlag = 1;
		gcIsCommInit = NO;
	}

	return APP_SUCC;
}


int PreCommInit(void)
{
	int nRet = 0;
	STCOMMPARAM stPubCommParam;
	static STCOMMPARAM stOldPubCommParam = {0};
	static char cInitFlag = 0;

	memset(&stPubCommParam, 0, sizeof(STCOMMPARAM));
	CommParamAppToPub(&gstAppCommParam, &stPubCommParam);

	if (memcmp(&stOldPubCommParam, &stPubCommParam, sizeof(STCOMMPARAM)) != 0 || 0 == cInitFlag || YES == GetControlCommInit())
	{
		nRet = PubCommInit(&stPubCommParam);
		if (nRet != APP_SUCC)
		{
			cInitFlag = 0;
			return APP_FAIL;
		}
		memcpy(&stOldPubCommParam, &stPubCommParam, sizeof(STCOMMPARAM));/**<save the old parameters*/
		cInitFlag = 1;
		gcIsCommInit = NO;
	}
	return APP_SUCC;
}

/**
* @brief Dial in advance
* @param void
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int CommPreDial(void)
{
	int nRet = 0;

	nRet = PubCommPreConnect();
	if (nRet != APP_SUCC)
	{
		if(gstAppCommParam.cCommType2 != COMM_NONE)
		{
			ASSERT_FAIL(SwitchCommType());
			nRet = PubCommPreConnect();
			if (nRet != APP_SUCC)
			{
				return APP_FAIL;
			}
		}
		else
		{
			return APP_FAIL;
		}
	}
	return APP_SUCC;
}

/**
* @brief Process SSL setting
* @param
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SSLConfig(void)
{
	STSSLMODE stSslMode;

	if(GetVarIsSSL() == APP_SUCC)
	{
		memset(&stSslMode, 0, sizeof(STSSLMODE));
		stSslMode.nAuthOpt = SSL_AUTH_CLIENT;
		stSslMode.nType = HANDSHAKE_TLSv1_2;
		stSslMode.nFileFormat = SSL_FILE_PEM;
		strcpy(stSslMode.szServerCert, "cert.pem");//for testing. require to add in filepath.txt when necessary
		return PubSslSetMode(&stSslMode);
	}
	else
	{
		return APP_SUCC;
	}
}

/**
* @brief Connecting, If CommType1 is failed, use CommType2
* @return
* @li APP_FAIL
* @li APP_SUCC
*/
int SwitchCommType()
{
	char cTmp;

	cTmp = gstAppCommParam.cCommType;
	gstAppCommParam.cCommType = gstAppCommParam.cCommType2;
	gstAppCommParam.cCommType2 = cTmp;

	PubDebug("\n\n gstAppCommParam.cCommType = %d\n\n", gstAppCommParam.cCommType);
	PubDebug("\n\n gstAppCommParam.cCommType2 = %d\n\n", gstAppCommParam.cCommType2);

	ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_TYPE, 1, &gstAppCommParam.cCommType));
	ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_TYPE2, 1, &gstAppCommParam.cCommType2));
	/**
	* do initialize
	*/
	if (PreCommInit() != APP_SUCC)
	{
		return APP_FAIL;
	}
	return APP_SUCC;
}


/**
* @brief communication connect
* @param  void
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int CommConnect(void)
{
	int nRet = 0;
	STSHOWINFOXY stShowInfoxy;

	SSLConfig();
	PubClear2To4();
	if(COMM_DIAL == gstAppCommParam.cCommType)
	{
		PubDisplayStrInline(DISPLAY_MODE_CENTER, 3,(char*) tr("Dialing..."));
	}
	else
	{
		PubDisplayStrInline(DISPLAY_MODE_CENTER, 3, (char*)tr("Connecting..."));
	}
//	PubDisplayStrInline(0, 6, (char*)tr("Press ESC To Quit"));
	PubUpdateWindow();

	stShowInfoxy.nType = 0;
	stShowInfoxy.nColumn = 7;
	stShowInfoxy.nRow = 4;
	PubSetShowXY(stShowInfoxy);

	nRet = PubCommConnect();
	if (nRet == APP_QUIT)
	{
		return APP_QUIT;
	}
	if (nRet != APP_SUCC)
	{

		if(gstAppCommParam.cCommType2 != COMM_NONE)
		{
			ASSERT_FAIL(SwitchCommType());
			nRet = PubCommConnect();
			if (nRet != APP_SUCC)
			{
				PubDispErr(tr("Connect Fail"));
				return APP_FAIL;
			}
		}
		else
		{
			PubDispErr(tr("Connect Fail"));
			return APP_FAIL;
		}
	}

	return APP_SUCC;
}


/**
* @brief add tpdu
* @param [in] [out] psBuf,input original data output data after adding tpdu
* @param [in] [out] punLen,input original data output datalen after adding tpdu
* @return
* @li APP_SUCC
* @li APP_FAIL
* @author
* @date
*/
static int AddTpdu(char *psBuf, uint *punLen)
{
	char sTemp[MAX_SEND_SIZE]={0};

#if 0
	char szTpdu[10+1] = {0};

	sprintf(szTpdu, "600%3.3s0000", gstAppCommParam.szNii);
	PubAscToHex((uchar *)szTpdu, 10, 0, (uchar *)gstAppCommParam.sTpdu);
#endif
	if (NULL == psBuf ||NULL == punLen)
	{
		return APP_FAIL;
	}
	memcpy(sTemp, psBuf, *punLen);
	memcpy(psBuf, gstAppCommParam.sTpdu, 5);
	memcpy(psBuf + 5, sTemp, *punLen);
	*punLen += 5;
	return APP_SUCC;
}

/**
* @brief delete tpdu
* @param in out psBuf,input original data output data after deleting tpdu
* @return in out punLen,input original data output datalen after deleting tpdu
* @li APP_SUCC
* @li APP_FAIL
*/
static int DelTpdu(char *psBuf, uint *punLen)
{
	char sTemp[MAX_SEND_SIZE];

	if (NULL == psBuf ||NULL == punLen)
	{
		return APP_FAIL;
	}
	if (*punLen < 5)
	{
		return APP_FAIL;
	}
	*punLen -= 5;
	memcpy(sTemp, psBuf + 5, *punLen);
	memcpy(psBuf, sTemp, *punLen);
	return APP_SUCC;
}

/**
* @brief Send data
* @param in psSendBuf
* @param in nSendLen
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int CommSend(const char *psSendBuf, int nSendLen)
{
	int nRet = 0;
	int nLen = 0;
	char sSendBuf[MAX_SEND_SIZE];

	memcpy(sSendBuf, psSendBuf, nSendLen);
	nLen = nSendLen;
	AddTpdu(sSendBuf, (uint *)&nLen);

	PubCommClear();

	PubClear2To4();
	PubDisplayStrInline(DISPLAY_MODE_CENTER, 3, tr("Sending..."));
	PubUpdateWindow();
	if (YES == GetVarIsPrintIso() && APP_SUCC == PubConfirmDlg(NULL, tr("Print ISO?"), 0, 10))
	{
		PrintIsoData(psSendBuf);
	}
	TRACE_HEX(sSendBuf, nLen, "<COMM SEND>");

	nRet = PubCommSend(sSendBuf, nLen);

	if (nRet != APP_SUCC)
	{
		PubDispErr(tr("CommSend FAIL"));
		return APP_FAIL;
	}
	return APP_SUCC;
}

/**
* @brief Revieve data
* @param out psRecvBuf
* @param out nRecvLen
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int CommRecv(char *psRecvBuf, int *nRecvLen)
{
	int nRet = 0;
	int nLen = 0;
	char sRecvBuf[MAX_RECV_SIZE+1] = {0};
	char *psRecv = sRecvBuf;
	STSHOWINFOXY stShowInfoxy;

	PubClear2To4();
	PubDisplayStrInline(DISPLAY_MODE_CENTER, 3, tr("Receiving..."));
	PubUpdateWindow();
	stShowInfoxy.nType = 0;
	stShowInfoxy.nColumn = 7;
	stShowInfoxy.nRow = 4;
	PubSetShowXY(stShowInfoxy);

	nRet = PubCommRecv(sRecvBuf, &nLen);
	if(nRet != APP_SUCC)
	{
		PubDispErr(tr("CommRecv FAIL"));
		return APP_FAIL;
	}

	TRACE_HEX(psRecv, nLen, "<COMM RECV>");

	nRet = DelTpdu(psRecv, (uint *)&nLen);
	if (APP_SUCC == nRet)
	{
		memcpy(psRecvBuf, psRecv, nLen);
		*nRecvLen = nLen;
		if (YES == GetVarIsPrintIso() && APP_SUCC == PubConfirmDlg(NULL, tr("Print ISO?"), 0, 10))
		{
			Unpack(psRecvBuf, nLen);
			PrintIsoData(psRecvBuf);
		}
		return APP_SUCC;
	}

	return APP_FAIL;
}

/**
* @brief send and recieve data
* @param in psSendBuf
* @param in nSendLen
* @param out psRecvBuf
* @param out nRecvLen
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int CommSendRecv(const char *psSendBuf, int nSendLen, char *psRecvBuf, int *nRecvLen)
{
	int nRet = 0;

	nRet = CommSend(psSendBuf, nSendLen);
	if (nRet != APP_SUCC)
	{
		return APP_FAIL;
	}

	nRet = CommRecv(psRecvBuf, nRecvLen);

	CommHangUpSocket();

	if (nRet != APP_SUCC)
	{
		return APP_FAIL;
	}

	return APP_SUCC;

}

/**
* @brief Hangup communication
* @param void
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int CommHangUp(void)
{
	if (COMM_GPRS == gstAppCommParam.cCommType || COMM_CDMA == gstAppCommParam.cCommType)
	{
		if (gstAppCommParam.cMode == 1 || gstAppCommParam.cMode == '1' )
		{
			;
		}
		else/**<not alive hangup PPP*/
		{
			return PubCommClose();
		}
	}
	return PubCommHangUp();
}

/**
* @brief
* @param void
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int CommHangUpSocket(void)
{
	if (COMM_DIAL != gstAppCommParam.cCommType)
	{
		PubCommHangUp();
	}

	return APP_SUCC;
}

/**
* @brief hangup communication
* @param void
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int CommDump(void)
{
	if (COMM_DIAL == gstAppCommParam.cCommType)
	{
		return PubCommHangUp();
	}
	else
	{
		return PubCommClose();
	}
}

/**
* @brief set predial number and initialize
* @param void
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int DoSetPreDialNum(void)
{
	SetFuncCommPreDialNum();
	return CommInit();
}

/**
* @brief set predial number
* @param void
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetFuncCommPreDialNum(void)
{
	char szTemp[14+1] = {0};
	int nLen = 10;

	memcpy(szTemp, gstAppCommParam.szPreDial, sizeof(gstAppCommParam.szPreDial)-1);
	ASSERT_FAIL(PubInputDlg(tr("SET COMM"), tr("Exterior line:"), szTemp, &nLen, 0, 10, 60, INPUT_MODE_STRING));
	memcpy(gstAppCommParam.szPreDial, szTemp, sizeof(gstAppCommParam.szPreDial)-1);

	ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_PREDIALNO, strlen(gstAppCommParam.szPreDial),\
		gstAppCommParam.szPreDial));
	return APP_SUCC;
}

/**
* @brief  communicaiton setting menu
* @param  void
* @return    void
*/
void CommMenu(void)
{
    int nRet;
	const char *pszItems[] = {
		tr("1.COMMTYPE1"),
		tr("2.COMMTYPE2"),
		tr("3.NII/TPDU"),
		tr("4.PSTN"),
		tr("5.GPRS"),
		tr("6.CDMA"),
		tr("7.ETH"),
		tr("8.RS232"),
		tr("9.WIFI"),
		tr("10.OTHER")
	};
	int nSelcItem = 1, nStartItem = 1;
	while(1)
	{
		nRet = PubShowMenuItems(tr("SET COMM"), (char**)pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem,60);
		if (nRet==APP_QUIT || nRet==APP_TIMEOUT)
		{
			return;
		}
		switch(nSelcItem)
		{
		case 1:
			SetFuncCommType();
			break;
		case 2:
			SetFuncCommType2();
			break;
		case 3:
			SetFuncCommNii();
			SetFuncCommTpdu();
			break;
		case 4:
			{
			int (*lSetFuns[])(void) = {
				SetFuncCommTelNo,
				DoSetPreDialNum,
				SetFuncCommIsPreDial,
				NULL};
				PubUpDownMenus(lSetFuns);
			}
			break;
		case 5:
			{
			int (*lSetFuns[])(void) = {
				SetFuncCommWLMDialNum,
				SetFuncCommAPN,
				SetFuncCommIp,
				SetFuncCommPort,
				SetFuncCommBackIp,
				SetFuncCommBackPort,
				SetFuncCommMode,
				SetFuncSimPassWd,
				NULL};
			    PubUpDownMenus(lSetFuns);
			}
			break;
		case 6:
			{
			int (*lSetFuns[])(void) = {
				SetFuncCommWLMDialNum,
				SetFuncCommIp,
				SetFuncCommPort,
				SetFuncCommBackIp,
				SetFuncCommBackPort,
				SetFuncCommMode,
				SetFuncSimPassWd,
				NULL};
			    PubUpDownMenus(lSetFuns);
			}
			break;
		case 7:
			{
			int (*lSetFuns[])(void) = {
				SetFuncIsDhcp,
				SetFuncCommIpAddr,
			    SetFuncCommGate,
			    SetFuncCommMask,
				SetFuncCommIp,
				SetFuncCommPort,
			    SetFuncCommBackIp,
				SetFuncCommBackPort,
				NULL};
			   	PubUpDownMenus(lSetFuns);
			}
			break;
		case 8:
			SetFuncAuxIsAddTpdu();
			break;
		case 9:
			{
			int (*lSetFuns[])(void) = {
			    SetFuncWifiSsid,
			    SetFuncWifiPwd,
			    SetFuncIsDhcp,
				SetFuncCommIpAddr,
				SetFuncCommMask,
				SetFuncCommGate,
			    SetFuncCommIp,
				SetFuncCommPort,
				SetFuncCommBackIp,
				SetFuncCommBackPort,
				NULL};
			    PubUpDownMenus(lSetFuns);
			}
			break;
		case 10:
			{
			int (*lSetFuns[])(void) = {
				SetFuncIsUseDns,
				SetFuncDomain,
				SetFuncDNSIp1,
			   	SetFuncDNSIp2,
				SetFuncCommTimeOut,
				SetFuncCommReDialNum,
				SetFuncIsSSL,
#ifdef USE_NTOMS
				SetFuncTOMSAppDomain,
				SetFuncTOMSParamDomain,
                SetFuncTOMSKeyPosDomain,
                SetFuncTOMSFileServerDomain,
                SetFuncTOMSTdasDomain,
#endif
				NULL};
			    PubUpDownMenus(lSetFuns);
			}
			break;
		default:
			break;
		}
	}
}


/**
* @brief Set standby type of communication
* @param void
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetFuncCommType2(void)
{
	char szInfo[64]={0};
	const char *pszItems[] = {
		tr("1.NONE"),
		tr("2.MODEM"),
		tr("3.GPRS"),
		tr("4.CDMA"),
		tr("5.RS232"),
		tr("6.ETH"),
		tr("7.WIFI"),
	};
	int nSelcItem, nStartItem = 1;

	nSelcItem = gstAppCommParam.cCommType2 + 1;
	ASSERT_FAIL(PubShowMenuItems(tr("SET COMM"), (char **)pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem,0));

	if((nSelcItem-1) == gstAppCommParam.cCommType)
	{
		PubMsgDlg("SET COMM", "CommType2 should be different from CommType1", 3, 5);
		return APP_FAIL;
	}

	switch(nSelcItem)
	{
	case 1:
		gstAppCommParam.cCommType2 = COMM_NONE;
		break;
	case 2:
		if(APP_SUCC != PubGetHardwareSuppot(HARDWARE_SUPPORT_MODEM, NULL))
		{
			PubMsgDlg(NULL, "NONSUPPORT", 0, 3);
			return APP_FAIL;
		}
		gstAppCommParam.cCommType2 = COMM_DIAL;
		break;
	case 3:
		if(APP_SUCC == PubGetHardwareSuppot(HARDWARE_SUPPORT_WIRELESS, szInfo))
		{
			if(memcmp(szInfo, "GPRS", 4) == 0)
			{
				gstAppCommParam.cCommType2 = COMM_GPRS;
				break;
			}
		}
		PubMsgDlg(NULL, "NONSUPPORT", 0, 3);
		return APP_FAIL;
	case 4:
		if(APP_SUCC == PubGetHardwareSuppot(HARDWARE_SUPPORT_WIRELESS, szInfo))
		{
			if(memcmp(szInfo, "CDMA", 4) == 0)
			{
				gstAppCommParam.cCommType2 = COMM_CDMA;
				break;
			}
		}
		PubMsgDlg(NULL, "NONSUPPORT", 0, 3);
		return APP_FAIL;
	case 5:
		gstAppCommParam.cCommType2 = COMM_RS232;
		break;
	case 6:
		if(APP_SUCC != PubGetHardwareSuppot(HARDWARE_SUPPORT_ETH, NULL))
		{
			PubMsgDlg(NULL, "NONSUPPORT", 0, 3);
			return APP_FAIL;
		}
		gstAppCommParam.cCommType2 = COMM_ETH;
		break;
	case 7:
		if(APP_SUCC != PubGetHardwareSuppot(HARDWARE_SUPPORT_WIFI, NULL))
		{
			PubMsgDlg(NULL, "NONSUPPORT", 0, 3);
			return APP_FAIL;
		}
		gstAppCommParam.cCommType2 = COMM_WIFI;
		break;
	default:
		return APP_QUIT;
	}

	ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_TYPE2, 1, &gstAppCommParam.cCommType2));

	return APP_SUCC;
}



/**
* @brief Set type of communication
* @param void
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetFuncCommType(void)
{
	char szInfo[64]={0};
	char cOldCommType = gstAppCommParam.cCommType;
	const char *pszItems[] = {
		tr("1.MODEM"),
		tr("2.GPRS"),
		tr("3.CDMA"),
		tr("4.RS232"),
		tr("5.ETH"),
		tr("6.WIFI")
	};
	int nSelcItem = 1, nStartItem = 1;

	nSelcItem = gstAppCommParam.cCommType;
	ASSERT_FAIL(PubShowMenuItems(tr("SET COMM"), (char **)pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem,0));

	if(nSelcItem == gstAppCommParam.cCommType2)
	{
		PubMsgDlg("SET COMM", "CommType1 should be different from CommType2", 3, 5);
		return APP_FAIL;
	}

	switch(nSelcItem)
	{
	case 1:
		if(APP_SUCC != PubGetHardwareSuppot(HARDWARE_SUPPORT_MODEM, NULL))
		{
			PubMsgDlg(NULL, "NONSUPPORT", 0, 3);
			return APP_FAIL;
		}
		gstAppCommParam.cCommType = COMM_DIAL;
		break;
	case 2:
		if(APP_SUCC == PubGetHardwareSuppot(HARDWARE_SUPPORT_WIRELESS, szInfo))
		{
			if(memcmp(szInfo, "GPRS", 4) == 0)
			{
				gstAppCommParam.cCommType = COMM_GPRS;
				break;
			}
		}
		PubMsgDlg(NULL, "NONSUPPORT", 0, 3);
		return APP_FAIL;
	case 3:
		if(APP_SUCC == PubGetHardwareSuppot(HARDWARE_SUPPORT_WIRELESS, szInfo))
		{
			if(memcmp(szInfo, "CDMA", 4) == 0)
			{
				gstAppCommParam.cCommType = COMM_CDMA;
				break;
			}
		}
		PubMsgDlg(NULL, "NONSUPPORT", 0, 3);
		return APP_FAIL;
	case 4:
		gstAppCommParam.cCommType = COMM_RS232;
		break;
	case 5:
		if(APP_SUCC != PubGetHardwareSuppot(HARDWARE_SUPPORT_ETH, NULL))
		{
			PubMsgDlg(NULL, "NONSUPPORT", 0, 3);
			return APP_FAIL;
		}
		gstAppCommParam.cCommType = COMM_ETH;
		break;
	case 6:
		if(APP_SUCC != PubGetHardwareSuppot(HARDWARE_SUPPORT_WIFI, NULL))
		{
			PubMsgDlg(NULL, "NONSUPPORT", 0, 3);
			return APP_FAIL;
		}
		gstAppCommParam.cCommType = COMM_WIFI;
		break;
	default:
		return APP_QUIT;
	}
	ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_TYPE, 1, &gstAppCommParam.cCommType));

	if (cOldCommType != gstAppCommParam.cCommType)
	{
		CommHangUp();
	}
	/**
	* do initialize
	*/
	if (CommInit() != APP_SUCC)
	{
		return APP_FAIL;
	}
	return APP_SUCC;
}

/**
* @brief set resend time
* @param void
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetFuncCommReSendNum(void)
{
	int nLen = 0,nRet;
	char szTemp[3+1];

	for (;;)
	{
		memset(szTemp, 0, sizeof(szTemp));
		sprintf(szTemp, "%d", gstAppCommParam.cReSendNum >= 0x30 ? (gstAppCommParam.cReSendNum - 0x30)%9 : gstAppCommParam.cReSendNum%9);
		ASSERT_RETURNCODE( PubInputDlg("REVERSAL RESEND", "NUM(<=3):", szTemp, \
			&nLen, 1, 1, 60, INPUT_MODE_NUMBER));
		nRet = atoi(szTemp);
		if ((nRet >= 1 )&& (nRet <= 3))
		{
			break;
		}
		PubMsgDlg(tr("REVERSAL RESEND"),tr("INVALID INPUT.TRY AGAIN!"),0,1);
	}
	gstAppCommParam.cReSendNum = nRet;
	ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_RESENDNUM, 1, &gstAppCommParam.cReSendNum));

	return APP_SUCC;
}

/**
* @brief change app parameter structure to  pub_lib parameter strcuture
* @param in pstApp ,app parameter structure
* @param in pstCommParam  ,pub_lib parameter strcuture
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
static int CommParamAppToPub(const STAPPCOMMPARAM *pstApp, STCOMMPARAM *pstCommParam)
{
	pstCommParam->cPreDialFlag = pstApp->cPreDialFlag == '1' ||pstApp->cPreDialFlag == 1 ? 1 : 0;
	pstCommParam->nTimeOut = pstApp->cTimeOut;
	pstCommParam->cReDialNum = pstApp->cReDialNum > 0x30 ? pstApp->cReDialNum - 0x30 : pstApp->cReDialNum;
	pstCommParam->cMode = pstApp->cMode == '1' || pstApp->cMode == 1 ? 0x01 : 0x02;
	pstCommParam->cIsSupportQuitRecv = SERVERMODE_RECVQUIT|SERVERMODE_CLOSENOWAIT|SERVERMODE_INITNOWAIT;
	if(pstApp->cIsSSL == 0)
	{
		pstCommParam->cSslFlag = 0;
	}
	else
	{
		pstCommParam->cSslFlag = 1;//one-way authentication
	}
	switch(pstApp->cCommType)
	{
	case COMM_RS232:
		pstCommParam->cCommType= COMMTYPE_PORT_ASYN1;
		pstCommParam->ConnInfo.stPortParam.nAux = RS232;
		pstCommParam->ConnInfo.stPortParam.nBaudRate = BAUD115200;
		pstCommParam->ConnInfo.stPortParam.nDataBits = DATA_8;
		pstCommParam->ConnInfo.stPortParam.nParity = PAR_NONE;
		pstCommParam->ConnInfo.stPortParam.nStopBits = STOP_1;
		memcpy(pstCommParam->ConnInfo.stPortParam.sTPDU, pstApp->sTpdu, 5);
		break;
	case COMM_DIAL:
		pstCommParam->cCommType = COMMTYPE_SYNDIAL;
		memcpy(pstCommParam->ConnInfo.stDialParam.szPredialNo, pstApp->szPreDial, 10);
		memcpy(pstCommParam->ConnInfo.stDialParam.lszTelNo[0], pstApp->szTelNum1, 14);
		memcpy(pstCommParam->ConnInfo.stDialParam.lszTelNo[1], pstApp->szTelNum2, 14);
		memcpy(pstCommParam->ConnInfo.stDialParam.lszTelNo[2], pstApp->szTelNum3, 14);
		memcpy(pstCommParam->ConnInfo.stDialParam.lsTPDU[0], pstApp->sTpdu, 5);
		memcpy(pstCommParam->ConnInfo.stDialParam.lsTPDU[1], pstApp->sTpdu, 5);
		memcpy(pstCommParam->ConnInfo.stDialParam.lsTPDU[2], pstApp->sTpdu, 5);
		pstCommParam->ConnInfo.stDialParam.nCycTimes = 3;
		pstCommParam->ConnInfo.stDialParam.nCountryid = 5;
		break;
	case COMM_GPRS:
		pstCommParam->cCommType = COMMTYPE_GPRS_HEADLEN;
		memcpy(pstCommParam->ConnInfo.stGprsParam.szNetUsername, pstApp->szUser, 40);
		memcpy(pstCommParam->ConnInfo.stGprsParam.szNetPassword, pstApp->szPassWd, 20);
		memcpy(pstCommParam->ConnInfo.stGprsParam.szPinPassWord, pstApp->szSIMPassWd, 10);
		if(pstApp->szWirelessDialNum[0] == 'D' || pstApp->szWirelessDialNum[0] == 'd')
		{
			memcpy(pstCommParam->ConnInfo.stGprsParam.szModemDialNo, pstApp->szWirelessDialNum+1, 20);
		}
		else
		{
			memcpy(pstCommParam->ConnInfo.stGprsParam.szModemDialNo, pstApp->szWirelessDialNum, 20);
		}
		sprintf(pstCommParam->ConnInfo.stGprsParam.szGprsApn, "%s", pstApp->szAPN1);
		memcpy(pstCommParam->ConnInfo.stGprsParam.sTPDU, pstApp->sTpdu, 5);
		memcpy(pstCommParam->ConnInfo.stGprsParam.szPdpType, pstApp->szPdpType, 20);
		break;
	case COMM_CDMA:
		pstCommParam->cCommType = COMMTYPE_CDMA_HEADLEN;
		memcpy(pstCommParam->ConnInfo.stCdmaParam.szNetUsername, pstApp->szUser, 40);
		memcpy(pstCommParam->ConnInfo.stCdmaParam.szNetPassword, pstApp->szPassWd, 20);
		memcpy(pstCommParam->ConnInfo.stCdmaParam.szPinPassWord, pstApp->szSIMPassWd, 10);
		if(pstApp->szWirelessDialNum[0] == 'D' || pstApp->szWirelessDialNum[0] == 'd')
		{
			memcpy(pstCommParam->ConnInfo.stCdmaParam.szModemDialNo, pstApp->szWirelessDialNum+1, 20);
		}
		else
		{
			memcpy(pstCommParam->ConnInfo.stCdmaParam.szModemDialNo, pstApp->szWirelessDialNum, 20);
		}
		memcpy(pstCommParam->ConnInfo.stCdmaParam.sTPDU, pstApp->sTpdu, 5);
		memcpy(pstCommParam->ConnInfo.stGprsParam.szPdpType, pstApp->szPdpType, 20);
		break;
	case COMM_ETH:
		pstCommParam->cCommType = COMMTYPE_ETH_HEADLEN;//COMMTYPE_ETH_TPDU_HEADLEN;
		if(pstApp->cIsDHCP != 0)
		{
			pstCommParam->ConnInfo.stEthParam.nDHCP = 1;
		}
		else
		{
			pstCommParam->ConnInfo.stEthParam.nDHCP = 0;
			memcpy(pstCommParam->ConnInfo.stEthParam.szIP, pstApp->szIpAddr, 16);
			memcpy(pstCommParam->ConnInfo.stEthParam.szGateway, pstApp->szGate, 16);
			memcpy(pstCommParam->ConnInfo.stEthParam.szMask, pstApp->szMask, 16);
			memcpy(pstCommParam->ConnInfo.stEthParam.szDNS, pstApp->szDNSIp1, 16);
		}
		memcpy(pstCommParam->ConnInfo.stEthParam.sTPDU, pstApp->sTpdu, 5);
		break;
	case COMM_WIFI:
		pstCommParam->cCommType = COMMTYPE_WIFI_HEADLEN;
		if(pstApp->cIsDHCP == '1' || pstApp->cIsDHCP == 1)
		{
			pstCommParam->ConnInfo.stWifiParam.ucIfDHCP = 1;
		}
		else
		{
			pstCommParam->ConnInfo.stWifiParam.ucIfDHCP = 0;
			memcpy(pstCommParam->ConnInfo.stWifiParam.szIP,pstApp->szIpAddr, 16);
			memcpy(pstCommParam->ConnInfo.stWifiParam.szGateway,pstApp->szGate, 16);
			memcpy(pstCommParam->ConnInfo.stWifiParam.szMask,pstApp->szMask, 16);
			strcpy(pstCommParam->ConnInfo.stWifiParam.szDnsPrimary,pstApp->szDNSIp1);
			strcpy(pstCommParam->ConnInfo.stWifiParam.szDnsSecondary,pstApp->szDNSIp2);
		}

		pstCommParam->ConnInfo.stWifiParam.nNetId = -1;
		pstCommParam->ConnInfo.stWifiParam.cWifiMode = pstApp->cWifiMode;
		memcpy(pstCommParam->ConnInfo.stWifiParam.szSsid,pstApp->szWifiSsid,32);
		memcpy(pstCommParam->ConnInfo.stWifiParam.szKey,pstApp->szWifiKey,32);
		memcpy(pstCommParam->ConnInfo.stWifiParam.sTPDU,pstApp->sTpdu, 5);
		break;
	default:
		return APP_FAIL;
	}

	if(pstApp->cIsDns == 0)
	{
		memcpy(pstCommParam->stServerAddress.lszIp[0], pstApp->szIp1, 16);
		memcpy(pstCommParam->stServerAddress.lszIp[1], pstApp->szIp2, 16);
	}
	else
	{
		memcpy(pstCommParam->stServerAddress.szDN, pstApp->szDomain, 50);
		memcpy(pstCommParam->stServerAddress.szDNSIp, pstApp->szDNSIp1, 16);
	}
	pstCommParam->stServerAddress.lnPort[0] = atoi(pstApp->szPort1);
	pstCommParam->stServerAddress.lnPort[1] = atoi(pstApp->szPort2);

	pstCommParam->ShowFunc = NULL;
	return APP_SUCC;
}

/**
* @brief set ppp mode
* @param void
* @li APP_SUCC
* @li APP_FAIL
*/
static int SetFuncCommMode(void)
{
	char cSelect = 0;
	char* SelMenu[] = {"0.BREAK","1.ALIVE"};

	cSelect = gstAppCommParam.cMode == '1' || gstAppCommParam.cMode == 1 ? 1 : 0;
	ASSERT_RETURNCODE(PubSelectYesOrNo((char*)tr("SET COMM"), (char*)tr("PPP MODE"), SelMenu, &cSelect));//'NO' means long link
	switch(cSelect)
	{
	case '0':
		gstAppCommParam.cMode = 0;
		break;
	case '1':
		gstAppCommParam.cMode = 1;
		break;
	default:
		return APP_QUIT;
	}
	ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_MODE, 1, &gstAppCommParam.cMode));

	return APP_SUCC;
}


/**
* @brief Set is need to predial
* @param void
* @li APP_SUCC
* @li APP_FAIL
*/
static int SetFuncCommIsPreDial(void)
{
	char cSelect = 0;

	cSelect = gstAppCommParam.cPreDialFlag == '1' || gstAppCommParam.cPreDialFlag == 1 ? 1 : 0;
	ASSERT_RETURNCODE( PubSelectYesOrNo(tr("SET COMM"), tr("Predictive Dialing"), NULL, &cSelect));
	switch(cSelect)
	{
	case '0':
		gstAppCommParam.cPreDialFlag = 0;
		break;
	case '1':
		gstAppCommParam.cPreDialFlag = 1;
		break;
	default:
		return APP_QUIT;
	}
	ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_ISPREDIAL, 1, &gstAppCommParam.cPreDialFlag));

	return APP_SUCC;
}

/**
* @brief set timeout
* @param void
* @li APP_SUCC
* @li APP_FAIL
*/
static int SetFuncCommTimeOut(void)
{
	int nRet = 0;
	char szTemp[3+1];
	int nLen = 0;

	for (;;)
	{
		memset(szTemp, 0, sizeof(szTemp));
		sprintf(szTemp, "%d", (uint)gstAppCommParam.cTimeOut);
		ASSERT_RETURNCODE(PubInputDlg(tr("SET COMM"), tr("Communication timeout:"), szTemp, \
			&nLen, 2, 2, 60, INPUT_MODE_NUMBER));
		nRet = atoi(szTemp);
		if (nRet < 10 || nRet >= 100)
		{
			continue;
		}
		gstAppCommParam.cTimeOut = nRet;
		break;
	}
	ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_TIMEOUT, 1, &gstAppCommParam.cTimeOut));

	return APP_SUCC;
}

/**
* @brief set redial time
* @param void
* @li APP_SUCC
* @li APP_FAIL
*/
static int SetFuncCommReDialNum(void)
{
	int nLen;
	char szNumber[2+1] = {0};

	sprintf(szNumber, "%d", gstAppCommParam.cReDialNum > 0X30 ? gstAppCommParam.cReDialNum - 0X30 : gstAppCommParam.cReDialNum );
	ASSERT_RETURNCODE(PubInputDlg(tr("SET COMM"), tr("Redial times:"), szNumber, &nLen, 1, 1, 60, INPUT_MODE_NUMBER));
	gstAppCommParam.cReDialNum = atoi(szNumber);
	ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_REDIALNUM, 1, &gstAppCommParam.cReDialNum));
	return APP_SUCC;
}
/**
* @brief set offline resend time
* @param void
* @li APP_SUCC
* @li APP_FAIL
*/
int SetFunctionOffResendNum(void)
{
	char szNum[2] = {0};
	int nNum, nLen;

	while(1)
	{
		szNum[0] = gstAppCommParam.cOffResendNum+'0';
		ASSERT_RETURNCODE( PubInputDlg(tr("OFFLINE RESEND"), tr("NUM(<=9):"), szNum, &nLen, 1, 1, 0, INPUT_MODE_NUMBER));
		nNum=atoi(szNum);
		if( (nNum >= 1) && (nNum <= 9) )
		{
			break;
		}
		PubMsgDlg(tr("OFFLINE RESEND"),tr("INVALID INPUT,TRY AGAIN!"),0,1);
	}
	gstAppCommParam.cOffResendNum = nNum;
	ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_OFFRESENDNUM, 1, &gstAppCommParam.cOffResendNum));
	return APP_SUCC;
}

/**
* @brief Set telephone number
* @param void
* @li APP_SUCC
* @li APP_FAIL
*/
static int SetFuncCommTelNo(void)
{
	char szTemp[19+1] = {0};
	int nLen=0, i, nRet;
	char *p = NULL;
	char szContent[16+1] = {0};

	p = gstAppCommParam.szTelNum1;
	for (i = 1; i <= 3; i++)
	{
		memset(szTemp, 0, sizeof(szTemp));
		memcpy(szTemp, p, sizeof(gstAppCommParam.szTelNum1) -1);
		sprintf(szContent, "TEL NUM(%d):", i);
		nRet = PubInputDlg(tr("SET COMM"), szContent, szTemp, &nLen, 0, 14, 60, INPUT_MODE_STRING);
		if(nRet == KEY_UP)
		{
			if(i <= 1)
			{
				return KEY_UP;
			}
			else
			{
				i -= 2;
				p -= sizeof(gstAppCommParam.szTelNum1);
				continue;
			}
		}
		else if(nRet == KEY_DOWN)
		{
			p += sizeof(gstAppCommParam.szTelNum1);
			continue;
		}
		ASSERT_RETURNCODE(nRet);
		memcpy(p, szTemp, sizeof(gstAppCommParam.szTelNum1) -1);
		p += sizeof(gstAppCommParam.szTelNum1);
		if (i == 1)
		{
			ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_TELNO1, strlen(gstAppCommParam.szTelNum1), gstAppCommParam.szTelNum1));
		}
		else if (i == 2)
		{
			ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_TELNO2, strlen(gstAppCommParam.szTelNum2), gstAppCommParam.szTelNum2));
		}
		else if (i == 3)
		{
			ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_TELNO3, strlen(gstAppCommParam.szTelNum3), gstAppCommParam.szTelNum3));
		}
	}
	return APP_SUCC;
}

/**
* @brief set dialing access number of wireless modem
* @param void
* @li APP_SUCC
* @li APP_FAIL
*/
static int SetFuncCommWLMDialNum(void)
{
	char szTemp[20+1] = {0};
	int nLen;

	memcpy(szTemp, gstAppCommParam.szWirelessDialNum, 20);
	ASSERT_RETURNCODE(PubInputDlg(tr("SET COMM"), tr("Wireless dial No.:"), szTemp, \
			&nLen, 0, 20, 60, INPUT_MODE_STRING));
	memcpy(gstAppCommParam.szWirelessDialNum, szTemp, sizeof(gstAppCommParam.szWirelessDialNum) -1);
	ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_WIRELESSDIALNUM, strlen(gstAppCommParam.szWirelessDialNum),
		gstAppCommParam.szWirelessDialNum));

	return APP_SUCC;
}

/**
* @brief set apn
* @param void
* @li APP_SUCC
* @li APP_FAIL
*/
static int SetFuncCommAPN(void)
{
	char szTemp[40+1] = {0};
	int nLen;

	memcpy(szTemp, gstAppCommParam.szAPN1, 40);
	ASSERT_RETURNCODE( PubInputDlg(tr("SET COMM"), "APN:", szTemp, \
			&nLen, 0, 40, 60, INPUT_MODE_STRING));
	memcpy(gstAppCommParam.szAPN1, szTemp, sizeof(gstAppCommParam.szAPN1) - 1);
	ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_APN1, strlen(gstAppCommParam.szAPN1), gstAppCommParam.szAPN1));
	return APP_SUCC;
}

/**
* @brief set SIM password
* @param void
* @li APP_SUCC
* @li APP_FAIL
*/
static int SetFuncSimPassWd(void)
{
	char szTemp[20+1];
	int nLen, nRet;
	EM_SIM_Status simStatus;

	nRet = NAPI_WlmGetSIMStatus(&simStatus);
	if (nRet != NAPI_OK || simStatus != SIM_PIN)
	{
		TRACE("nRet = %d simStatus = %d", nRet, simStatus);
		return APP_SUCC;
	}

	memset(szTemp, 0, sizeof(szTemp));
	memcpy(szTemp, gstAppCommParam.szSIMPassWd, 20);
	ASSERT_RETURNCODE( PubInputDlg(tr("SET SIM PIN"), tr("SIM PIN CODE"), szTemp, \
			&nLen, 0, 20, 60, INPUT_MODE_STRING));
	memcpy(gstAppCommParam.szSIMPassWd, szTemp, sizeof(gstAppCommParam.szSIMPassWd) -1);
	ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_SIMPWD, strlen(gstAppCommParam.szSIMPassWd), gstAppCommParam.szSIMPassWd));
	return APP_SUCC;
}


/**
* @brief Domain name
* @param int
* @li APP_SUCC
* @li APP_FAIL
*/
static int SetFuncDomain(void)
{
	char szTemp[100+1];
	int nLen;

	if(gstAppCommParam.cIsDns == 0)
	{
		return APP_FUNCQUIT;
	}

	memset(szTemp, 0, sizeof(szTemp));
	memcpy(szTemp, gstAppCommParam.szDomain, sizeof(gstAppCommParam.szDomain)-1);
	nLen = strlen(szTemp);
	ASSERT_RETURNCODE(PubInputDlg(tr("SET COMM"), tr("Server Domain Name"), szTemp, &nLen,0,50,60,INPUT_MODE_STRING));
	memcpy(gstAppCommParam.szDomain, szTemp, sizeof(gstAppCommParam.szDomain) -1);
	ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_DOMAIN1, strlen(gstAppCommParam.szDomain), gstAppCommParam.szDomain));
	return APP_SUCC;
}

#ifdef USE_NTOMS
/**
* @brief TOMSDomain name
* @param int
* @li APP_SUCC
* @li APP_FAIL
*/
static int SetFuncTOMSAppDomain(void)
{
	char szTemp[100+1];
	int nLen;

	memset(szTemp, 0, sizeof(szTemp));
	memcpy(szTemp, gstAppCommParam.szTOMSAppDomain, sizeof(gstAppCommParam.szTOMSAppDomain)-1);
	nLen = strlen(szTemp);
	ASSERT_RETURNCODE(PubInputDlg(tr("SET COMM"), tr("TOMS App Domain Name"), szTemp, &nLen,0,100,60,INPUT_MODE_STRING));
	memcpy(gstAppCommParam.szTOMSAppDomain, szTemp, sizeof(gstAppCommParam.szTOMSAppDomain) -1);
	ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_TOMSAPPDOMAIN, strlen(gstAppCommParam.szTOMSAppDomain), gstAppCommParam.szTOMSAppDomain));

	return APP_SUCC;
}

/**
* @brief TOMS Param Domain
* @param int
* @li APP_SUCC
* @li APP_FAIL
*/
static int SetFuncTOMSParamDomain(void)
{
	char szTemp[100+1];
	int nLen;

	memset(szTemp, 0, sizeof(szTemp));
	memcpy(szTemp, gstAppCommParam.szTOMSParamDomain, sizeof(gstAppCommParam.szTOMSParamDomain)-1);
	nLen = strlen(szTemp);
	ASSERT_RETURNCODE(PubInputDlg(tr("SET COMM"), tr("Param Domain Name"), szTemp, &nLen,0,100,60,INPUT_MODE_STRING));
	memcpy(gstAppCommParam.szTOMSParamDomain, szTemp, sizeof(gstAppCommParam.szTOMSParamDomain) -1);
	ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_TOMSPARAMDOMAIN, strlen(gstAppCommParam.szTOMSParamDomain), gstAppCommParam.szTOMSParamDomain));

	return APP_SUCC;
}

/**
* @brief TOMS Key Pos Domain
* @param int
* @li APP_SUCC
* @li APP_FAIL
*/
static int SetFuncTOMSKeyPosDomain(void)
{
	char szTemp[100+1];
	int nLen;

	memset(szTemp, 0, sizeof(szTemp));
	memcpy(szTemp, gstAppCommParam.szTOMSKeyPosDomain, sizeof(gstAppCommParam.szTOMSKeyPosDomain)-1);
	nLen = strlen(szTemp);
	ASSERT_RETURNCODE(PubInputDlg(tr("SET COMM"), tr("KeyPos Domain Name"), szTemp, &nLen,0,100,60,INPUT_MODE_STRING));
	memcpy(gstAppCommParam.szTOMSKeyPosDomain, szTemp, sizeof(gstAppCommParam.szTOMSKeyPosDomain) -1);
	ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_TOMSKEYPOSDOMAIN, strlen(gstAppCommParam.szTOMSKeyPosDomain), gstAppCommParam.szTOMSKeyPosDomain));

	return APP_SUCC;
}

/**
* @brief TOMS file server name
* @param int
* @li APP_SUCC
* @li APP_FAIL
*/
static int SetFuncTOMSFileServerDomain(void)
{
	char szTemp[100+1];
	int nLen;

	memset(szTemp, 0, sizeof(szTemp));
	memcpy(szTemp, gstAppCommParam.szTOMSFileServerDomain, sizeof(gstAppCommParam.szTOMSFileServerDomain)-1);
	nLen = strlen(szTemp);
	ASSERT_RETURNCODE(PubInputDlg(tr("SET COMM"), tr("File Server Domain Name"), szTemp, &nLen,0,100,60,INPUT_MODE_STRING));
	memcpy(gstAppCommParam.szTOMSFileServerDomain, szTemp, sizeof(gstAppCommParam.szTOMSFileServerDomain) -1);
	ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_TOMSFILESERDOMAIN, strlen(gstAppCommParam.szTOMSFileServerDomain), gstAppCommParam.szTOMSFileServerDomain));

	return APP_SUCC;
}

/**
* @brief TOMS Tdas Domain
* @param int
* @li APP_SUCC
* @li APP_FAIL
*/
static int SetFuncTOMSTdasDomain(void)
{
	char szTemp[100+1];
	int nLen;

	memset(szTemp, 0, sizeof(szTemp));
	memcpy(szTemp, gstAppCommParam.szTOMSTdasDomain, sizeof(gstAppCommParam.szTOMSTdasDomain)-1);
	nLen = strlen(szTemp);
	ASSERT_RETURNCODE(PubInputDlg(tr("SET COMM"), tr("TDAS Domain Name"), szTemp, &nLen,0,100,60,INPUT_MODE_STRING));
	memcpy(gstAppCommParam.szTOMSTdasDomain, szTemp, sizeof(gstAppCommParam.szTOMSTdasDomain) -1);
	ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_TOMSTDASDOMAIN, strlen(gstAppCommParam.szTOMSTdasDomain), gstAppCommParam.szTOMSTdasDomain));

	return APP_SUCC;
}


int SetFuncTOMSUserOID(void)
{
	char szTemp[100+1] = {""};
	int nLen, nOffset;

    char cSelect = gstAppCommParam.cIsUserOid;

    ASSERT_QUIT(PubSelectYesOrNo((char*)tr("DEMO ID"), (char*)"OPEN?", NULL, &cSelect));
    switch(cSelect)
    {
    case '0':
        gstAppCommParam.cIsUserOid = 0;
        break;
    case '1':
        {
            gstAppCommParam.cIsUserOid = 1;
            strcpy(szTemp, "DEMO");
            nOffset = strlen(szTemp);
            memcpy(szTemp + nOffset, gstAppCommParam.szTomsUserOid + nOffset, sizeof(gstAppCommParam.szTomsUserOid)-1- nOffset);
            ASSERT_FAIL(PubInputDlg(tr("OID"), tr("OID"), szTemp + nOffset, &nLen,0,60,60,INPUT_MODE_STRING));
        }
        break;
    default:
        return APP_QUIT;
    }

    memcpy(gstAppCommParam.szTomsUserOid, szTemp, sizeof(gstAppCommParam.szTomsUserOid) -1);

    ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_TOMSISUSEROID, 1, &gstAppCommParam.cIsUserOid));
    ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_TOMSUSEROID, strlen(gstAppCommParam.szTomsUserOid), gstAppCommParam.szTomsUserOid));

    return APP_SUCC;

}


#endif


char GetUserOidSwitch()
{
	return gstAppCommParam.cIsUserOid;
}

void SetUserOidSwitch(int nMode)
{
	if(nMode)
	{
		gstAppCommParam.cIsUserOid = '1';
	}
	else
	{
		gstAppCommParam.cIsUserOid = '0';
	}
}

/**
* @brief 1st DNS
* @param int
* @li APP_SUCC
* @li APP_FAIL
*/
static int SetFuncDNSIp1(void)
{
	char szTemp[16+1];
	int nLen;

	if(gstAppCommParam.cIsDns == 0)
	{
		return APP_FUNCQUIT;
	}

	memset(szTemp, 0, sizeof(szTemp));
	memcpy(szTemp, gstAppCommParam.szDNSIp1, sizeof(gstAppCommParam.szDNSIp1)-1);
	nLen = strlen(szTemp);
	ASSERT_RETURNCODE(PubInputIp(tr("SET COMM"), tr("Primary DNS Address"), szTemp, &nLen));
	memcpy(gstAppCommParam.szDNSIp1, szTemp, sizeof(gstAppCommParam.szDNSIp1) -1);
	ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_DNSIP1, strlen(gstAppCommParam.szDNSIp1), gstAppCommParam.szDNSIp1));
	return APP_SUCC;
}

/**
* @brief 2nd DNS
* @param int
* @li APP_SUCC
* @li APP_FAIL
*/
static int SetFuncDNSIp2(void)
{
	char szTemp[16+1];
	int nLen;

	if(gstAppCommParam.cIsDns == 0)
	{
		return APP_FUNCQUIT;
	}

	memset(szTemp, 0, sizeof(szTemp));
	memcpy(szTemp, gstAppCommParam.szDNSIp2, sizeof(gstAppCommParam.szDNSIp2)-1);
	nLen = strlen(szTemp);
	ASSERT_RETURNCODE(PubInputIp(tr("SET COMM"), tr("Secondary DNS Address"), szTemp, &nLen));
	memcpy(gstAppCommParam.szDNSIp2, szTemp, sizeof(gstAppCommParam.szDNSIp2) -1);
	ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_DNSIP2, strlen(gstAppCommParam.szDNSIp2), gstAppCommParam.szDNSIp2));
	return APP_SUCC;
}

/**
* @brief set ip
* @param void
* @li APP_SUCC
* @li APP_FAIL
*/
static int SetFuncCommIp(void)
{
	char szTemp[16+1]= {0};
	int nLen;

	if(gstAppCommParam.cIsDns != 0)
	{
		return APP_FUNCQUIT;
	}
	memcpy(szTemp, gstAppCommParam.szIp1, sizeof(gstAppCommParam.szIp1)-1);
	nLen = strlen(szTemp);
	ASSERT_RETURNCODE(PubInputIp(tr("SET COMM"), tr("HOST IP1"), szTemp, &nLen));
	memcpy(gstAppCommParam.szIp1, szTemp, sizeof(gstAppCommParam.szIp1) -1);
	ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_IP1, strlen(gstAppCommParam.szIp1), gstAppCommParam.szIp1));
	return APP_SUCC;
}

/**
* @brief set second ip
* @param void
* @li APP_SUCC
* @li APP_FAIL
*/
static int SetFuncCommBackIp(void)
{
	char szTemp[16+1] = {0};
	int nLen;

	if(gstAppCommParam.cIsDns != 0)
	{
		return APP_FUNCQUIT;
	}
	memcpy(szTemp, gstAppCommParam.szIp2, sizeof(gstAppCommParam.szIp2)-1);
	nLen = strlen(szTemp);
	ASSERT_RETURNCODE(PubInputIp(tr("SET COMM"), tr("HOST IP2"), szTemp, &nLen));
	memcpy(gstAppCommParam.szIp2, szTemp, sizeof(gstAppCommParam.szIp2) -1);
	ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_IP2, strlen(gstAppCommParam.szIp2), gstAppCommParam.szIp2));
	return APP_SUCC;
}

/**
* @brief set port
* @param void
* @li APP_SUCC
* @li APP_FAIL
*/
static int SetFuncCommPort(void)
{
	char szTemp[20+1];
	int nRet;
	int nLen;

	while(1)
	{
		memset(szTemp, 0, sizeof(szTemp));
		memcpy(szTemp, gstAppCommParam.szPort1, sizeof(gstAppCommParam.szPort1)-1);
		ASSERT_RETURNCODE(PubInputDlg(tr("SET COMM"), tr("HOST PORT1"), szTemp, \
				&nLen, 0, 6, 60, INPUT_MODE_NUMBER));
		nRet = atoi(szTemp);
		if (nRet < 0 || nRet > 65535)
		{
			continue;
		}
		break;
	}
	memcpy(gstAppCommParam.szPort1, szTemp, sizeof(gstAppCommParam.szPort1) -1);
	ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_PORT1, strlen(gstAppCommParam.szPort1), gstAppCommParam.szPort1));
	return APP_SUCC;
}

/**
* @brief set second port
* @param void
* @li APP_SUCC
* @li APP_FAIL
*/
static int SetFuncCommBackPort(void)
{
	char szTemp[20+1];
	int nRet;
	int nLen;

	while(1)
	{
		memset(szTemp, 0, sizeof(szTemp));
		memcpy(szTemp, gstAppCommParam.szPort2, sizeof(gstAppCommParam.szPort2)-1);
		ASSERT_RETURNCODE( PubInputDlg(tr("SET COMM"), tr("HOST PORT2"), szTemp, \
				&nLen, 0, 6, 60, INPUT_MODE_NUMBER));
		nRet = atoi(szTemp);
		if (nRet < 0 || nRet > 65535)
		{
			continue;
		}
		break;
	}
	memcpy(gstAppCommParam.szPort2, szTemp, sizeof(gstAppCommParam.szPort2) -1);
	ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_PORT2, strlen(gstAppCommParam.szPort2), gstAppCommParam.szPort2));
	return APP_SUCC;
}

/**
* @brief set local ip
* @param void
* @li APP_SUCC
* @li APP_FAIL
*/
static int SetFuncCommIpAddr(void)
{
	char szTemp[16+1] = {0};
	int nLen;

	if(gstAppCommParam.cIsDHCP != 0)
	{
		return APP_FUNCQUIT;
	}

	memcpy(szTemp, gstAppCommParam.szIpAddr, sizeof(gstAppCommParam.szIpAddr)-1);
	nLen = strlen(szTemp);
	ASSERT_RETURNCODE(PubInputIp(tr("SET COMM"), tr("LOCAL IP"), szTemp, &nLen));
	memcpy(gstAppCommParam.szIpAddr, szTemp, sizeof(gstAppCommParam.szIpAddr) -1);
	ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_IPADDR, strlen(gstAppCommParam.szIpAddr), gstAppCommParam.szIpAddr));

	return APP_SUCC;
}

/**
* @brief set mask
* @param void
* @li APP_SUCC
* @li APP_FAIL
*/
static int SetFuncCommMask(void)
{
	char szTemp[16+1] = {0};
	int nLen;

	if(gstAppCommParam.cIsDHCP != 0)
	{
		return APP_FUNCQUIT;
	}

	memcpy(szTemp, gstAppCommParam.szMask, sizeof(gstAppCommParam.szMask)-1);
	nLen = strlen(szTemp);
	ASSERT_RETURNCODE(PubInputIp(tr("SET COMM"), tr("MASK"), szTemp, &nLen));
	memcpy(gstAppCommParam.szMask, szTemp, sizeof(gstAppCommParam.szMask) -1);
	ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_MASK, strlen(gstAppCommParam.szMask), gstAppCommParam.szMask));

	return APP_SUCC;
}

/**
* @brief set gate
* @param void
* @li APP_SUCC
* @li APP_FAIL
*/
static int SetFuncCommGate(void)
{
	char szTemp[16+1] = {0};
	int nLen;

	if(gstAppCommParam.cIsDHCP != 0)
	{
		return APP_FUNCQUIT;
	}

	memcpy(szTemp, gstAppCommParam.szGate, sizeof(gstAppCommParam.szGate)-1);
	nLen = strlen(szTemp);
	ASSERT_RETURNCODE(PubInputIp(tr("SET COMM"), tr("GATE"), szTemp, &nLen));
	memcpy(gstAppCommParam.szGate, szTemp, sizeof(gstAppCommParam.szGate) -1);
	ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_GATE, strlen(gstAppCommParam.szGate), gstAppCommParam.szGate));
	return APP_SUCC;
}

/**
* @brief set if aux need tpdu
* @param void
* @li APP_SUCC
* @li APP_FAIL
*/
static int SetFuncAuxIsAddTpdu(void)
{
	char cSelect = gstAppCommParam.cAuxIsAddTpdu;

	ASSERT_RETURNCODE(PubSelectYesOrNo(tr("SET COMM"), tr("ADD TPDU"), NULL, &cSelect));
	switch(cSelect)
	{
	case '0':
		gstAppCommParam.cAuxIsAddTpdu = 0;
		break;
	case '1':
		gstAppCommParam.cAuxIsAddTpdu = 1;
		break;
	default:
		return APP_QUIT;
	}
	ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_AUXISADDTPDU, 1, &gstAppCommParam.cAuxIsAddTpdu));
	return APP_SUCC;
}

/**
* @brief get timeout from app communication parameter
* @param out pszTimeOut 2 ascii bytes
* @li APP_SUCC
*/
int GetVarCommTimeOut(char *pszTimeOut)
{
	char szTmp[2+1] = {0};

	sprintf(szTmp, "%02d", gstAppCommParam.cTimeOut);
	memcpy(pszTimeOut, szTmp, 2);

	return APP_SUCC;
}

/**
* @brief set timeout to app communication parameter
* @param in pszTimeOut 2 ascii bytes
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarCommTimeOut(const char *pszTimeOut)
{
	char szTemp[2+1] = {0};

	memcpy(szTemp, pszTimeOut, 2);
	gstAppCommParam.cTimeOut = (char)atoi(szTemp);
	ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_TIMEOUT, 1, &gstAppCommParam.cTimeOut));
	return APP_SUCC;
}

/**
* @brief get resend time
* @param out pcReSendNum 1byte
* @li APP_SUCC
*/
int GetVarCommReSendNum(char *pcReSendNum)
{
	*pcReSendNum = gstAppCommParam.cReSendNum;
	return APP_SUCC;
}

/**
* @brief set resend time
* @param in pcReSendNum 1byte
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarCommReSendNum(const char *pcReSendNum)
{
	char cReSendUum = *pcReSendNum;

	gstAppCommParam.cReSendNum  = cReSendUum >= 0x30 ? (cReSendUum - 0x30)%9 : cReSendUum%9;
	ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_RESENDNUM, 1, &gstAppCommParam.cReSendNum));
	return APP_SUCC;
}

/**
* @brief get offline resend time
* @param out pcReSendNum 1byte
* @li APP_SUCC
*/
int GetVarCommOffReSendNum(char *pcReSendNum)
{
	*pcReSendNum = gstAppCommParam.cOffResendNum;
	return APP_SUCC;
}

/**
* @brief set offline resend time
* @param in pcReSendNum 1byte
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarCommOffReSendNum(const char *pcReSendNum)
{
	char cReSendUum = *pcReSendNum;

	gstAppCommParam.cOffResendNum  = cReSendUum >= 0x30 ? (cReSendUum - 0x30)%9 : cReSendUum%9;
	ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_OFFRESENDNUM, 1, &gstAppCommParam.cOffResendNum));
	return APP_SUCC;
}

/**
* @brief get redial time
* @param out pcReSendNum 1byte
* @li APP_SUCC
*/
int GetVarCommReDialNum(char *pcReDialNum)
{
	*pcReDialNum = gstAppCommParam.cReDialNum;
	return APP_SUCC;
}

/**
* @brief set redial time
* @param in pcReSendNum 1byte
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarCommReDialNum(const char *pcReDialNum)
{
	char cReDialUum = *pcReDialNum;

	gstAppCommParam.cReDialNum  = cReDialUum >= 0x30 ? (cReDialUum - 0x30)%9 : cReDialUum%9;
	ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_REDIALNUM, 1, &gstAppCommParam.cReDialNum));
	return APP_SUCC;
}

/**
* @brief Set wifi ssid
* @param in  void
* @li APP_SUCC
* @li APP_FAIL
*/
int SetFuncWifiSsid(void)
{
	char szTemp[100+1] = {0};
	int nLen, nRet, nWifiMode;
	ST_WIFI_INFO_T stInfo;

	if (WifiGetCurrentInfo(&stInfo) == APP_SUCC)
	{
		gstAppCommParam.cWifiMode = stInfo.stApInfo.emAuthType;
		strcpy(gstAppCommParam.szWifiSsid, stInfo.stApInfo.szSsid);
		ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_WIFIMODE, 1, &gstAppCommParam.cWifiMode));
		ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_WIFISSID, strlen(gstAppCommParam.szWifiSsid), gstAppCommParam.szWifiSsid));
	}

	PubClearAll();
	PubDisplayTitle("WIFI PARAM");
	PubDisplayStrInline(1, 2,"SSID:");
	PubDisplayStrInline(1, 3, "%s",gstAppCommParam.szWifiSsid);

	if (PubGetKbAttr() == KB_VIRTUAL)
	{
		nRet = PubShowGetKbPad(60, BUTTON_CONFIRM, tr("SCAN"), tr("CANCEL"), tr("ENTER"), NULL);
	}
	else
	{
		PubDisplayStrInline(1, 4, "0.SCAN    1.ENTER");
		PubUpdateWindow();
		while (1) {
			nRet = PubGetKeyCode(60);
			if(nRet == KEY_0 || nRet == KEY_1 || nRet == KEY_ENTER || nRet == KEY_ESC) {
				break;
			}
		}
	}

	if(nRet == KEY_0 || nRet == KEY_F1)
	{
		PubClear2To4();
		PubDisplayStrInline(0, 2, (char*)tr("SEARCH SSID"));
		PubDisplayStrInline(0, 3, (char*)tr("PLEASE WAIT..."));
		PubUpdateWindow();
		ASSERT_RETURNCODE(PubCommScanWifi(tr("SET COMM"), szTemp, &nWifiMode, 60));
		gstAppCommParam.cWifiMode = nWifiMode;
		memset(gstAppCommParam.szWifiSsid, 0, sizeof(gstAppCommParam.szWifiSsid));
		memcpy(gstAppCommParam.szWifiSsid, szTemp, sizeof(gstAppCommParam.szWifiSsid));
		ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_WIFIMODE, 1, &gstAppCommParam.cWifiMode));
		ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_WIFISSID, strlen(gstAppCommParam.szWifiSsid), gstAppCommParam.szWifiSsid));
	}
	else if(nRet == KEY_1 || nRet == KEY_F2 || nRet == KEY_ENTER)
	{
		memcpy(szTemp, gstAppCommParam.szWifiSsid, 32);
		ASSERT_RETURNCODE( PubInputDlg(tr("WIFI PARAM"), "SSID", szTemp, \
				&nLen, 1, 32, 60, INPUT_MODE_STRING));
		memcpy(gstAppCommParam.szWifiSsid, szTemp, sizeof(gstAppCommParam.szWifiSsid) -1);
		ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_WIFISSID, strlen(gstAppCommParam.szWifiSsid), gstAppCommParam.szWifiSsid));
		ASSERT_FAIL(SetFuncWifiMode());
	} else if (nRet == KEY_ESC) {
		return APP_QUIT;
	}
	return APP_SUCC;
}

/**
* @brief set wifi password
* @param in  void
* @li APP_SUCC
* @li APP_FAIL
*/
int SetFuncWifiPwd(void)
{
	char szTemp[64+1] = {0};
	int nRet;
	ST_WIFI_CONFIG_LIST stList;
	ST_WIFI_CONFIG_T stConfig[WIFI_AP_NUM_MAX];
	int nLen, i;

	stList.pstList = stConfig;
	nRet = NAPI_WifiGetConfiguredNetworks(&stList);
	TRACE("num=%d", stList.num);
	if (nRet == NAPI_OK)
	{
		for (i = 0; i < stList.num; i ++)
		{
			TRACE("stList.pstList[%d].szSsid=%s", i, stList.pstList[i].szSsid);
			if (strcmp(stList.pstList[i].szSsid, gstAppCommParam.szWifiSsid) == 0)
			{
				memcpy(gstAppCommParam.szWifiKey, stList.pstList[i].szPasswd, 64);
				break;
			}
		}
	}

	memcpy(szTemp, gstAppCommParam.szWifiKey, 32);
	ASSERT_RETURNCODE( PubInputDlg(tr("WIFI PARAM"), tr("Password"), szTemp, \
				&nLen, 0, 32, 60, INPUT_MODE_STRING));
	memcpy(gstAppCommParam.szWifiKey, szTemp, sizeof(gstAppCommParam.szWifiKey) -1);

	ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_WIFIKEY, strlen(gstAppCommParam.szWifiKey), gstAppCommParam.szWifiKey));
	return APP_SUCC;
}

/**
* @brief set wifi modo
* @param in  void
* @li APP_SUCC
* @li APP_FAIL
*/
int SetFuncWifiMode(void)
{
	const char *pszItems[] = {
		tr("1.OPEN"),
		tr("2.WEP"),
		tr("3.WPA-PSK"),
		tr("4.WPA2-PSK"),
		tr("5.WPA-CCKM")
	};
	int nSelcItem = 1, nStartItem = 1;

	nSelcItem = gstAppCommParam.cWifiMode;
	ASSERT_RETURNCODE(PubShowMenuItems(tr("WIFI MODE"), (char**)pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem,0));

	switch(nSelcItem)
	{
	case 1:
		gstAppCommParam.cWifiMode = WIFI_AUTH_OPEN;
		break;
	case 2:
		gstAppCommParam.cWifiMode = WIFI_AUTH_WEP_PSK;
		break;
	case 3:
		gstAppCommParam.cWifiMode = WIFI_AUTH_WPA_PSK;
		break;
	case 4:
		gstAppCommParam.cWifiMode = WIFI_AUTH_WPA2_PSK;
		break;
	case 5:
		gstAppCommParam.cWifiMode = WIFI_AUTH_WPA_WPA2_MIXED_PSK;
		break;
	default:
		return APP_QUIT;
	}
	ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_WIFIMODE, 1, &gstAppCommParam.cWifiMode));
	return APP_SUCC;
}


/**
* @brief set if need to use DNS
* @param in  int
* @li APP_SUCC
* @li APP_FAIL
*/
int SetFuncIsUseDns(void)
{
	char cSelect = gstAppCommParam.cIsDns;

	ASSERT_RETURNCODE(PubSelectYesOrNo((char*)tr("SET COMM"), "Use DNS?", NULL, &cSelect));
	switch(cSelect)
	{
	case '0':
		gstAppCommParam.cIsDns = 0;
		break;
	case '1':
		gstAppCommParam.cIsDns = 1;
		break;
	default:
		return APP_QUIT;
	}

	ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_ISDNS, 1, &gstAppCommParam.cIsDns));
	return APP_SUCC;
}


/**
* @brief set if need to dhcp
* @param in  void
* @li APP_SUCC
* @li APP_FAIL
*/

static int SetFuncIsDhcp(void)
{
	char cSelect = gstAppCommParam.cIsDHCP;

	ASSERT_RETURNCODE(PubSelectYesOrNo((char*)tr("SET COMM"), (char*)"OPEN DHCP", NULL, &cSelect));
	switch(cSelect)
	{
	case '0':
		gstAppCommParam.cIsDHCP = 0;
		break;
	case '1':
		gstAppCommParam.cIsDHCP = 1;
		break;
	default:
		return APP_QUIT;
	}

	ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_ISDHCP, 1, &gstAppCommParam.cIsDHCP));
	return APP_SUCC;
}

/**
* @brief get app communication parameter
* @param out :pstAppCommParam
* @return void
*/
void GetAppCommParam(STAPPCOMMPARAM *pstAppCommParam)
{
	memcpy((char *)pstAppCommParam, (char *)(&gstAppCommParam), sizeof(STAPPCOMMPARAM));
}

/**
* @brief get app communication parameter
* @param out :pstAppCommParam
* @return void
*/
void SetAppCommParam(STAPPCOMMPARAM stAppCommParam)
{
	memcpy((char *)&gstAppCommParam, (char *)(&stAppCommParam), sizeof(STAPPCOMMPARAM));
}

/**
* @brief Get NII
* @param out pszNii
* @li APP_SUCC
* @li APP_FAIL
*/
int GetVarCommNii(char * pszNii)
{
	memcpy(pszNii, gstAppCommParam.szNii, 3);
	return APP_SUCC;
}

/**
** brief: Exprot communication parammeter
** param: void
** return: APP_SUCC or APP_FAIL
** auther:
** date: 2016-7-4
** modify:
*/
int ExportCommParam(void)
{
	memset(&gstAppCommParam, 0, sizeof(STAPPCOMMPARAM));
	ASSERT_FAIL(LoadTagParam(FILE_APPCOMMPARAM, (void *)&gstAppCommParam));

	return APP_SUCC;
}

/**
** brief: Set Nii
** param: void
** return: APP_SUCC or APP_FAIL
** auther:
** date: 2016-7-4
** modify:
*/
int SetFuncCommNii(void)
{
	char szTemp[3+1];
	int nLen;

	memset(szTemp, 0, sizeof(szTemp));
	memcpy(szTemp, gstAppCommParam.szNii, sizeof(gstAppCommParam.szNii)-1);
	nLen = strlen(szTemp);
	ASSERT_RETURNCODE(PubInputDlg(tr("SET COMM"), "Nii", szTemp, &nLen,3,3,60,INPUT_MODE_STRING));
	memcpy(gstAppCommParam.szNii, szTemp, sizeof(gstAppCommParam.szNii) -1);

	ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_NII, sizeof(gstAppCommParam.szNii) - 1, gstAppCommParam.szNii));
	return APP_SUCC;
}

/**
** brief: Set TPDU
** param: void
** return: APP_SUCC or APP_FAIL
** auther:
** date: 2016-7-4
** modify:
*/
int SetFuncCommTpdu(void)
{
	char szTemp[3+1];
	int nLen;
	char szTpdu[10+1] = {0};

	memset(szTemp, 0, sizeof(szTemp));
	PubHexToAsc((uchar *)gstAppCommParam.sTpdu, 10, 0, (uchar *)szTpdu);
	nLen = strlen(szTemp);
	ASSERT_RETURNCODE(PubInputDlg(tr("SET COMM"), "TPDU", szTpdu, &nLen,10,10,60,INPUT_MODE_NUMBER));

	PubAscToHex((uchar *)szTpdu, 10, 0, (uchar *)gstAppCommParam.sTpdu);
	ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_TPDU, 5, gstAppCommParam.sTpdu));
	return APP_SUCC;
}

int SetFuncUserOid(char *pOidBuf, int nOidBufLen)
{
	if (!pOidBuf || nOidBufLen <= 0)
	{
		TRACE("Parameter error.(pOidBuf - %p, nOidBufLen - %d)", (void*)pOidBuf, nOidBufLen);
		return APP_FAIL;
	}

	memcpy(gstAppCommParam.szTomsUserOid, pOidBuf, nOidBufLen);
	return APP_SUCC;
}

int GetFuncUserOid(char *pOidBuf, int nOidBufLen)
{
	if (!pOidBuf || nOidBufLen <= 0)
	{
		TRACE("Parameter error.(pOidBuf - %p, nOidBufLen - %d)", (void*)pOidBuf, nOidBufLen);
		return APP_FAIL;
	}

	memset(pOidBuf, 0, nOidBufLen);
	snprintf(pOidBuf, nOidBufLen, "%s", gstAppCommParam.szTomsUserOid);
	return APP_SUCC;
}

/**
** brief: set communication parameter
** param: void
** return: APP_SUCC
** auther:
** date: 2016-7-4
** modify:
*/
int SetComm()
{
	CommMenu();
	CommInit();
	return APP_SUCC;
}

static int SetFuncIsSSL(void)
{
	char cSelect = NO;

	if (gstAppCommParam.cIsSSL == 1)
	{
		cSelect = YES;
	}

	ASSERT_RETURNCODE(PubSelectYesOrNo(tr("SET COMM"),tr("IS USE SSL"), NULL,&cSelect));
	switch(cSelect)
	{
	case '0':
		gstAppCommParam.cIsSSL = 0;
		break;
	case '1':
		if(gstAppCommParam.cCommType == COMM_DIAL)
		{
			PubMsgDlg("Warn", "Nonsupport", 3, 3);
			return APP_QUIT;
		}
		gstAppCommParam.cIsSSL = 1;
		break;
	default:
		return APP_QUIT;
	}
	ASSERT_FAIL(UpdateTagParam(FILE_APPCOMMPARAM, TAG_COMM_ISSSL, 1, &gstAppCommParam.cIsSSL));
	CommInit();

	return APP_SUCC;
}

static int GetVarIsSSL(void)
{
	if(gstAppCommParam.cCommType != COMM_DIAL)
	{
		if(gstAppCommParam.cIsSSL == 1)
		{
			return APP_SUCC;
		}
	}
	return APP_FAIL;
}


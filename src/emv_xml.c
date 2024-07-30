/**************************************************************************
* Copyright (C) 2019 Newland Payment Technology Co., Ltd All Rights Reserved
*
* @file  xml.c
* @brief consider the limit of memory(MPOS), we load XML configuraion one line by on line
* @version 1.0
* @author linld
* @date 2019-9-25
**************************************************************************/


#include "apiinc.h"
#include "libapiinc.h"
#include "appinc.h"




typedef struct
{
	char Transtype; 					
	int nTLVListLen;
	uchar szTLVList[200];
	
}PaypassTranTypeAID;



#define YES 1
#define NO 0


#define NODE_CONFIG "config"
#define NODE_ENTRY  "entry"
#define NODE_DRL  "DRL"
#define NODE_CUSTOM_TAG "CUSTOM"
#define DRL_TYPE_PAYWAVE 1 
#define DRL_TYPE_AMEX    2
static int gnPaypassAidControl = 0;


static int ParseAID(char *szLineBuf, char *szAidTlv, int *nTlvLen ,PaypassTranTypeAID *Tlvlist ,int *pnPaypassAidAddNum);
static int ParseCapk(char *szLineBuf, L3_CAPK_ENTRY *stCapk);

/**
* @fn ReadLine
* @brief read one line from file
* @details read one line from file
* 		   [fgets] only get one line which is end with "0A"/"0D 0A" ,
* 		   But, sometimes one line is end with "0D", so we make this api.
* @param [out] pStr [One line data read from file]
* @param [in]  nFd  [The data file handle]
* @return 
* @li 0 
* @li 1
* @author linld
* @date 2019-9-26
*/
static int ReadLine(char *str, int n, int nFd)
{
	int num = 0;
	int index = 0;
	
	while(1)
	{
		num = PubFsRead(nFd, str+index, 1);
		if (num < 1)
		{
			break;
		}
		
		if (*(str+index) == 0x0D 
			|| *(str+index) == 0x0A)
		{
			if (index == 0) //empty line
			{
			    continue;
			}
			else
			{
			    *(str+index) = 0x00;
			}
			
		    return 0;
		}
	

		index++;
	}

	return 1;
    
}

/**
* @fn Invaild
* @brief Judge if the data is invaild
* @details Judge if the data is invaild
* 		   The invaild data in xml file will begin with "<!--" or "<?".
* @param [in] lineStr [One line data to judge]
* @return 
* @li YES The data is invaild
* @li NO
* @author
* @date
*/
static int Invaild(char *lineStr)
{
//	if (strlen(lineStr) == 1 || (lineStr[0] == 0x0d && lineStr[1] == 0x0a))//Empty line
//	{
//	    return YES;
//	}

	if (strstr(lineStr, "<!--") != NULL)//Comments
	{
	    return YES;
	}

	if (strstr(lineStr, "<?") != NULL) //head
	{
	    return YES;
	}

	return NO;
    
}

/**
* @fn IsNode
* @brief Judge if the data have the node and the tag
* @param [in] node    [The node data shuold have]
* @param [in] str     [The tag data shuold have]
* @param [in] lineStr [The data to judge]
* @return 
* @li YES Have the node and tag
* @li NO
* @author
* @date
*/
static int IsNode(char *node, char *str, char *lineStr)
{
	char *p1 = NULL;
	char *p2 = NULL;
	
	char szContent[100] = {0};
	
    if (NULL == strstr(lineStr, node))
    {
        return NO;
    }

	p1 = strchr(lineStr, '\"');
	if (p1 == NULL)
	{
	    return NO;
	}

	p2 = strchr(p1+1, '\"');
	if (p2 == NULL)
	{
	    return NO;
	}

	memcpy(szContent, p1+1, p2-p1-1);
	
	PubAllTrim(szContent);
	if (strlen(szContent) != strlen(str))
	{
	    return NO;
	}

	if (0 == memcmp(szContent, str, strlen(str)))
	{
	    return YES;
	}

	return NO;
}

/**
* @fn IsNodeEnd
* @brief Judge whether the data have the end of node 
* @details Judge whether the data have the end of node 
*          The Tag will combine with '</' and pszNode
*          Use for xml data
* @param [in] pszNode [the node for judge]
* @param [in] pszLineStr [the original data]
* @li YES 
* @li NO
* @author qiuzx
* @date 2020-12-24
*/
int IsNodeEnd(char *pszNode, char *pszLineStr)
{
	char szTmp[50] = {0};
	
	sprintf(szTmp, "</%s", pszNode);
	
    if (strstr(pszLineStr, szTmp) != NULL)
	{
	    return YES;
	}
	return NO;
}

/**
* @fn IsElement
* @brief Judge if the data is element
* @details Judge if the data is element
* @        The Element will begin with "<item"
* @param [in] lineStr [The data to judge]
* @return 
* @li YES Is the element
* @li NO
* @author
* @date
*/
int IsElement(char *lineStr)
{
    if (strstr(lineStr, "<item") != NULL)
	{
	    return YES;
	}
	return NO;
}

/**
* @fn IsCustomTag
* @brief Judge if the data is custom element
* @details Judge if the data is custom element
* @        The Element will begin with "<CUSTOM TAG"
* @param [in] lineStr [The data to judge]
* @return 
* @li YES Is the element
* @li NO
* @author
* @date
*/
int IsCustomTag(char * lineStr)
{
    if (strstr(lineStr, "CUSTOM TAG") != NULL)
	{
  		return YES;
	}
	
	return NO;
}

/**
* @fn IsDrlElement
* @brief Judge if the data is drl element
* @details Judge if the data is drl element
* @        The Element will begin with "<CUSTOM TAG"
* @param [in]  lineStr [The data to judge]
* @param [out] drlType [Drl type DRL_TYPE_PAYWAVE or DRL_TYPE_AMEX]
* @return 
* @li YES Is the element
* @li NO
* @author
* @date
*/
int IsDrlElement(char * lineStr, uchar * drlType)
{
    if (strstr(lineStr, "PAYWAVE DRL") != NULL)
	{
		*drlType = DRL_TYPE_PAYWAVE;
  		return YES;
	}
	else if(strstr(lineStr, "AMEX DRL") != NULL)
	{
		*drlType = DRL_TYPE_AMEX;
		return YES;
	}
	return NO;
}

/**
* @fn GetAttribute
* @brief Get the Attribute from the data 
* @details Get the Attribute from the data 
*          Example data -> "<value name="PRINT_ALL">"
*          "name" is the Attribute Name, "PRINT_ALL" is the target
*          Use for xml data
* @param [in]  pszAttr 	  [attribute Name]
* @param [out] pszOut	  [target attribute]
* @param [in]  pszLineStr [the original data]
* @return 
* @li  0  - SUCCESS 
* @li -1 - No Found Attribute Name
* @li -2 - No Found '"' for the beginning
* @li -3 - No Found '"' for the end
* @author qiuzx
* @date 2020-12-24
*/
int GetAttribute(char *pszAttr, char *pszOut, char *pszLineStr)
{
	char *p = NULL;
	char *p1 = NULL;
	char *p2 = NULL;

	p = strstr(pszLineStr, pszAttr);
    if (p == NULL)
    {
        return -1;
    }

	p1 = strchr(p, '\"');
	if (p1 == NULL)
	{
	    return -2;
	}

	p2 = strchr(p1+1, '\"');
	if (p2 == NULL)
	{
		return -3;    
	}

	memcpy(pszOut, p1+1, p2-p1-1);
	PubAllTrim(pszOut);
	return 0;
	
}

static void DispLoadCfgInfo(char cCardInterface, char cType)
{
	char szContent[32] = {0};

	PubClearAll();
	if (cType == 2) // capk
	{
		sprintf(szContent, "%s", tr("LOAD CAPK"));
	}
	else
	{
		if (cCardInterface == L3_CONTACT)
		{
			sprintf(szContent, "%s", tr("LOAD CT AID"));
		}
		else
		{
			sprintf(szContent, "%s", tr("LOAD CTL AID"));
		}
	}

	PubDisplayStrInline(0, 2, szContent);
	PubDisplayStrInline(0, 4, tr("PLEASE WAIT..."));
	PubUpdateWindow();
}

/**
* @fn SetCustomTag
* @brief Set custom tag 
* @param [in]  szLineBuf 	  [Line buffer]
* @param [out] strEmvCusTag	  [Custom data]
* @return 
* @li  0  - SUCCESS 
* @li -1 - No Found Attribute Name
* @li -2 - No Found '"' for the beginning
* @li -3 - No Found '"' for the end
* @author 
* @date
*/
int SetCustomTag(char * szLineBuf, EMVTagAttr * strEmvCusTag)
{
	int nRet;
	char szKey[50] = {0};
	char szValue[100] = {0};
	char szHexVal[4] = {0};
	int nValLen = 0;
	//int temp[2] = {0};

	memset(szKey, 0, sizeof(szKey));
    nRet = GetAttribute("key", szKey, szLineBuf);
	if (nRet != 0)
	{
	    TRACE("GetAttribute fail, nRet=%d", nRet);
		return nRet;
	}
	
	memset(szValue, 0, sizeof(szValue));
	nRet = GetAttribute("value", szValue, szLineBuf);
	if (nRet != 0)
	{
	    TRACE("GetAttribute fail, nRet=%d", nRet);
		return nRet;
	}

	TRACE("[%s]:[%s]", szKey, szValue);
	nValLen = strlen(szValue);
	memset(szHexVal, 0, sizeof(szHexVal));
	PubAscToHex((uchar *)szValue, nValLen, 1, (uchar * )szHexVal);
	nValLen = (nValLen+1)>>1;
	
	if (0 == strcmp(szKey, "Tag name"))
	{	
		PubCnToInt(&(strEmvCusTag->tag), (uchar *)szHexVal, nValLen);	
	
	} 
	else if (0 == strcmp(szKey, "Template1"))
	{
		PubCnToInt(&(strEmvCusTag->template1), (uchar *)szHexVal, nValLen);
	}
	else if (0 == strcmp(szKey, "Template2"))
	{
		PubCnToInt(&(strEmvCusTag->template2), (uchar *)szHexVal, nValLen);
	}
	else if (0 == strcmp(szKey, "Source"))
	{
		strEmvCusTag->source = szHexVal[0] & 0xFF;
	}
	else if (0 == strcmp(szKey, "Format"))
	{
		strEmvCusTag->format = szHexVal[0] & 0xFF;
	}
	else if (0 == strcmp(szKey, "Min lenth"))
	{
		strEmvCusTag->minLen = szHexVal[0] & 0xFF;
		TRACE("strEmvCusTag->minLen = %d", strEmvCusTag->minLen);
	}
	else if (0 == strcmp(szKey, "Max lenth"))
	{
		strEmvCusTag->maxLen = szHexVal[0] & 0xFF;
	}

	return 0;
}

/**
* @fn SetPayWaveDrl
* @brief Set paywave drl 
* @param [in]  szLineBuf 	  [Line buffer]
* @param [out] strPayWaveDrl	  [Paywave drl]
* @return 
* @li  0  - SUCCESS 
* @li -1 - No Found Attribute Name
* @li -2 - No Found '"' for the beginning
* @li -3 - No Found '"' for the end
* @author 
* @date
*/
int SetPayWaveDrl(char * szLineBuf, STDRLPARAM * strPayWaveDrl)
{
	char szKey[30] = {0};
	int nRet = 0;
	char szValue[80] = {0};
	char szHexVal[40] = {0};
	int nValLen = 0;
		
	memset(szKey, 0, sizeof(szKey));
	nRet = GetAttribute("key", szKey, szLineBuf);
	if (nRet != 0)
	{
		TRACE("GetAttribute fail, nRet=%d", nRet);
		return nRet;
	}

	memset(szValue, 0, sizeof(szValue));
	nRet = GetAttribute("value", szValue, szLineBuf);
	if (nRet != 0)
	{
		TRACE("GetAttribute fail, nRet=%d", nRet);
		return nRet;
	}

	TRACE("[%s]:[%s]", szKey, szValue);
	nValLen = strlen(szValue);
	memset(szHexVal, 0, sizeof(szHexVal));
	PubAscToHex((uchar *)szValue, nValLen, 1, (uchar * )szHexVal);
	nValLen = (nValLen+1)>>1;

	if (0 == strcmp(szKey, "APP ID"))
	{
		strPayWaveDrl->ucDrlAppIDLen = nValLen;
		memcpy(strPayWaveDrl->usDrlAppId, szHexVal, strPayWaveDrl->ucDrlAppIDLen);		
	} 
	else if (0 == strcmp(szKey, "ClLimitExist"))
	{
		strPayWaveDrl->ucClLimitExist = (szHexVal[0] & 0xFF);
	}
	else if (0 == strcmp(szKey, "Clss Transaction Limit"))
	{
		memcpy(strPayWaveDrl->usClTransLimit , szHexVal, 6);
	}
	else if (0 == strcmp(szKey, "Clss Offline Limit"))
	{
		memcpy(strPayWaveDrl->usClOfflineLimit , szHexVal, 6);
	}
	else if (0 == strcmp(szKey, "Cvm Limit"))
	{
		memcpy(strPayWaveDrl->usClCvmLimit , szHexVal, 6);	
	}

	return 0;
}


/**
* @fn SetAmexDrl
* @brief Set Amex drl 
* @param [in]  szLineBuf 	  [Line buffer]
* @param [out] strAmexDrl	  [Amex drl]
* @return 
* @li  0  - SUCCESS 
* @li -1 - No Found Attribute Name
* @li -2 - No Found '"' for the beginning
* @li -3 - No Found '"' for the end
* @author 
* @date
*/
int SetAmexDrl(char * szLineBuf, STEXDRLPARAM * strAmexDrl)
{
	char szKey[30] = {0};
	int nRet = 0;
	char szValue[60] = {0};
	char szHexVal[30] = {0};
	int nValLen = 0;	
	
	memset(szKey, 0, sizeof(szKey));
	nRet = GetAttribute("key", szKey, szLineBuf);
	if (nRet != 0)
	{
		TRACE("GetAttribute fail, nRet=%d", nRet);
		return nRet;
	}

	memset(szValue, 0, sizeof(szValue));
	nRet = GetAttribute("value", szValue, szLineBuf);
	if (nRet != 0)
	{
		TRACE("GetAttribute fail, nRet=%d", nRet);
		return nRet;
	}

	TRACE("[%s]:[%s]", szKey, szValue);
	nValLen = strlen(szValue);
	memset(szHexVal, 0, sizeof(szHexVal));
	PubAscToHex((uchar *)szValue, nValLen, 1, (uchar * )szHexVal);
	
	if (0 == strcmp(szKey, "DRL Exist"))
	{
		strAmexDrl->ucDrlExist = atoi(szValue);
	} 
	else if (0 == strcmp(szKey, "DRL ID"))
	{
	   strAmexDrl->ucDrlId = (szHexVal[0] & 0xFF);
	}
	else if (0 == strcmp(szKey, "Limist Exist"))
	{
		strAmexDrl->ucClLimitExist = (szHexVal[0] & 0xFF);
	}
	else if (0 == strcmp(szKey, "Clss Transaction Limit"))
	{
		memcpy(strAmexDrl->usClTransLimit , szHexVal, 6);
	}
	else if (0 == strcmp(szKey, "Clss Offline Limit"))
	{
		memcpy(strAmexDrl->usClOfflineLimit , szHexVal, 6);
	}
	else if (0 == strcmp(szKey, "Cvm Limit"))
	{
		memcpy(strAmexDrl->usClCvmLimit , szHexVal, 6);	
		
	}

	return 0;
}

/**
* @fn AddTagOne
* @brief Add custom tag
* @param [out] uCustagTlv 	  [Output data]
* @param [in]  strCusTag	  [Custom tag]
* @param [out] nCusTagOffset	  [Data offset]	
* @param
* @return 
* @li  0  - SUCCESS 
* @author 
* @date
*/
int AddTagOne(uchar * uCustagTlv, EMVTagAttr * strCusTag, int * nCusTagOffset)
{
	
	int nEmvTagAttrLen = 0;
	
	TRACE("[nCusTagOffset] = %d", *nCusTagOffset);
	TRACE_HEX((char *)strCusTag, sizeof(EMVTagAttr), "[Custom tag add]: ");
				
	nEmvTagAttrLen = sizeof(EMVTagAttr);
	memcpy(uCustagTlv + *nCusTagOffset, strCusTag, nEmvTagAttrLen);
	*nCusTagOffset += nEmvTagAttrLen;
	memset(strCusTag, 0, sizeof(EMVTagAttr));

	return 0;

}

/**
* @fn AddDrlOne
* @brief Add drl tag(Paywave or Amex)
* @param [out] ucDrlTlv 	  [Output data]
* @param [in]  strPayWaveDrl	  [Paywave drl]
* @param [in]  strAmexDrl	  [Amex drl]
* @parma [in]  ucType		  [Drl type]
* @param [out] nDrlOffset	  [Data offset]	
* @param
* @return 
* @li  0  - SUCCESS 
* @author 
* @date
*/
int AddDrlOne(uchar * ucDrlTlv, STDRLPARAM * strPayWaveDrl, STEXDRLPARAM * strAmexDrl, uchar ucType, int * nDrlOffset)
{
	int nDrlstrLen = 0;
	TRACE("[nDrlOffset] = %d", *nDrlOffset);
	
	if(ucType == DRL_TYPE_PAYWAVE)
	{		
		TRACE_HEX((char *)strPayWaveDrl, sizeof(STDRLPARAM), "[Paywave drl add]: ");
		
		nDrlstrLen = sizeof(STDRLPARAM);
		memcpy(ucDrlTlv + *nDrlOffset, strPayWaveDrl, nDrlstrLen);
		*nDrlOffset += nDrlstrLen;
		memset(strPayWaveDrl, 0, sizeof(STDRLPARAM));
	}
	else if(ucType == DRL_TYPE_AMEX)
	{
		TRACE_HEX((char *)strAmexDrl, sizeof(STEXDRLPARAM), "[Amex drl add]: ");
		
		nDrlstrLen = sizeof(STEXDRLPARAM);
		memcpy(ucDrlTlv + *nDrlOffset, strAmexDrl, nDrlstrLen);		
		*nDrlOffset += nDrlstrLen;
		memset(strAmexDrl, 0, sizeof(STEXDRLPARAM));
	}

	return 0;
}


/**
* @fn AddDrlAll
* @brief Add all drl tag(Paywave or Amex)
* @param [out] ucDrlTlv 	[Output data]
* @param [in]  szAidTlv	  	[Aid list]
* @param [out] nTlvLen	  	[Aid length]
* @parma [in]  ucDrlType	[Drl type]
* @param [in]  nDrlOffset	[Data offset]	
* @param
* @return 
* @li  0  - SUCCESS 
* @author 
* @date
*/
int AddDrlAll(uchar * ucDrlTlv, char *szAidTlv, int *nTlvLen, int nDrlOffset, uchar ucDrlType)
{
	TRACE("[nDrlOffset] = %d:", nDrlOffset);
	
	if (ucDrlType == DRL_TYPE_PAYWAVE)
	{
		TRACE_HEX((char *)ucDrlTlv, nDrlOffset, "Paywave Drl [DF3F]: ");
		TlvAddEx((const uchar *)"DF3F", nDrlOffset, (char *)ucDrlTlv, szAidTlv, nTlvLen);
	}
	else if (ucDrlType == DRL_TYPE_AMEX)
	{
		TRACE_HEX((char *)ucDrlTlv, nDrlOffset, "Amex Drl [DF53]: ");
		TlvAddEx((const uchar *)"DF53", nDrlOffset, (char *)ucDrlTlv, szAidTlv, nTlvLen);
	}
	
	return 0;
}

/**
* @fn AddPaypassAidTlv
* @brief Add paypass aid tlv data
* @param [in]  szHexTranstypevalue 	[Transation type]
* @param [in]  Tlvlist	  			[Tlv list]
* @param [in]  szTag	  			[Tag]
* @parma [in]  nValLen				[Length of tag value]
* @param [out] szHexVal				[Output value]	
* @param [out] pnPaypassAidAddNum		[Add number]
* @param
* @return 
* @li  0  - SUCCESS 
* @author 
* @date
*/
int AddPaypassAidTlv(char* szHexTranstypevalue,PaypassTranTypeAID *Tlvlist,char * szTag, int nValLen ,char *szHexVal,int *pnPaypassAidAddNum)
{
	int i = 0;
	TRACE("pnPaypassAidAddNum=%d", *pnPaypassAidAddNum);
	for(i=0;i <= *pnPaypassAidAddNum;i++)
	{
	
		if( ((Tlvlist[i].Transtype == szHexTranstypevalue[0]) &&(Tlvlist[i].nTLVListLen != 0)) )
		{
			TlvAddEx((uchar *)szTag, nValLen, szHexVal, (char *)Tlvlist[i].szTLVList, &Tlvlist[i].nTLVListLen);
			TRACE("Tlvlist[i].nTLVListLen=%d", Tlvlist[i].nTLVListLen);
			TRACE_HEX((char *) Tlvlist[i].szTLVList, Tlvlist[i].nTLVListLen, "Tlvlist[0].szTLVList: ");
			break;
		}
		else if(Tlvlist[i].nTLVListLen == 0)
		{		
			Tlvlist[i].Transtype = szHexTranstypevalue[0];
			TlvAddEx((uchar *)szTag, nValLen, szHexVal, (char *)Tlvlist[i].szTLVList, &Tlvlist[i].nTLVListLen);
			TRACE_HEX( (char *)Tlvlist[i].szTLVList, Tlvlist[i].nTLVListLen, "Tlvlist[0].szTLVList: ");
			*pnPaypassAidAddNum += 1;
			break;
		}
	
	}

	return 0;

}

/**
* @fn LoadPaypassTranTypeAID
* @brief Load aid tlv data and add to tlv list
* @param [in]  szAidTlv 			[Aid]
* @param [in]  nTlvLen	  			[Tlv length]
* @param [in]  Tlvlist	  			[Output tlv list]
* @param [in]  nPaypassAidAddNum		[Add number]
* @param
* @return 
* @li  0  - SUCCESS 
* @author 
* @date
*/
int LoadPaypassTranTypeAID(char *szAidTlv ,int nTlvLen ,PaypassTranTypeAID *Tlvlist ,int nPaypassAidAddNum)
{
	int i = 0;
	char szNewAidTlv[1500] = {0};
	int nNEWTlvLen = 0;
	int nRet = 0;

	  for(i=0;i<nPaypassAidAddNum;i++)
	  {
	 	memset(szNewAidTlv, 0, sizeof(szNewAidTlv));
	 	nNEWTlvLen = nTlvLen;
	  	memcpy(szNewAidTlv, szAidTlv, nNEWTlvLen);

		
		TlvAddEx((const uchar *)"1F8101", 1, "\x01", szNewAidTlv, &nNEWTlvLen);
		TlvAddEx((const uchar *)"DF7D", 1, &Tlvlist[i].Transtype, szNewAidTlv, &nNEWTlvLen);
		memcpy(szNewAidTlv+nNEWTlvLen, Tlvlist[i].szTLVList, Tlvlist[i].nTLVListLen);
		nNEWTlvLen += Tlvlist[i].nTLVListLen;
		TRACE_HEX( szNewAidTlv, nNEWTlvLen, "szNewAidTlv: ");
		
		nRet = NAPI_L3LoadAIDConfig(0x02, NULL, (uchar *)szNewAidTlv, &nNEWTlvLen, CONFIG_UPT);
		TRACE("NAPI_L3LoadAIDConfig->CONFIG_UPT, nRet=%d", nRet);

	  }
 	gnPaypassAidControl = 0;
	return 0;
}


/**
* @fn LoadXMLConfig
* @brief Load configuration from XML file
* @param NONE
* @return
* @li 0  success
* @li < 0 fail
* @author linld
* @date 2019-9-26
*/
int LoadXMLConfig(void)
{
	char szLineBuf[1000] = {0};
	L3_CARD_INTERFACE cardInterface = 0;
	int nIsTerminalConfig = NO;
	int nConfigType = 0;
	int nRet = 0, nFd = 0, nEnd = 0;
	char szAidTlv[1500] = {0};
	int nTlvLen = 0;
	L3_CAPK_ENTRY stCapk;
	EMVTagAttr strEmvTag;
	STDRLPARAM strPayWaveDrl;
	STEXDRLPARAM strAmexDrl;	
	uchar ucCusTagSetting = 0;
	uchar ucDrlIsSetting = 0;
	uchar ucDrlType = 0;
	int nTagOffset = 0;
	int nDrlOffset = 0;
	uchar uCustagTlv[280] = {0};
	uchar ucDrlTlv[500] = {0};
	PaypassTranTypeAID pTrasTypeAidTlv[10] = {{0, 0, {0}}};
	int nPaypassAidAddNum = 0;
		
	nFd = PubFsOpen(XML_CONFIG, "r");
	if (nFd < 0)	
	{
	    TRACE("open file[%s] failed[%d].", XML_CONFIG, nFd);
		return -1;
	}

	PubClearAll();
	PubDisplayStrInline(0, 2, tr("LOAD CONFIG"));
	PubDisplayStrInline(0, 4, tr("PLEASE WAIT..."));
	PubUpdateWindow();

	TxnL3LoadAIDConfig(L3_CONTACT, NULL, NULL, NULL, CONFIG_FLUSH);
	TxnL3LoadAIDConfig(L3_CONTACTLESS, NULL, NULL, NULL, CONFIG_FLUSH);
	TxnL3LoadCAPK(NULL, CONFIG_FLUSH);

	memset(&strPayWaveDrl, 0, sizeof(STDRLPARAM));
	memset(&strAmexDrl, 0, sizeof(STEXDRLPARAM));
	memset(&strEmvTag, 0, sizeof(EMVTagAttr));
	memset(uCustagTlv, 0, sizeof(uCustagTlv));
	memset(ucDrlTlv, 0, sizeof(ucDrlTlv));
	memset(szAidTlv, 0, sizeof(szAidTlv));
	memset(&stCapk, 0, sizeof(stCapk));
	
	while (1)
	{
		if (nEnd == 1)
		{
		    break;
		}
		memset(szLineBuf, 0, sizeof(szLineBuf));

	    nRet = ReadLine(szLineBuf, sizeof(szLineBuf), nFd);
		if (nRet == 1)
		{
		    nEnd = 1;
		}
		
		
		if (YES == Invaild(szLineBuf))
		{
		    continue;
		}

		if (YES == IsNode(NODE_CONFIG, "CONTACT", szLineBuf))
		{
			nConfigType = 1;
			cardInterface = L3_CONTACT;
			DispLoadCfgInfo(cardInterface, nConfigType);
			continue;
		}

		if (YES == IsNode(NODE_CONFIG, "CONTACTLESS", szLineBuf))
		{
			nConfigType = 1;
			cardInterface = L3_CONTACTLESS;
			DispLoadCfgInfo(cardInterface, nConfigType);
			continue;
		}

		if (YES == IsNode(NODE_CONFIG, "PublicKeys", szLineBuf))
		{
			nConfigType = 2;
			DispLoadCfgInfo(cardInterface, nConfigType);
			continue;
		}

		if (YES == IsNode(NODE_ENTRY, "Terminal Configuration", szLineBuf))
		{
		    nIsTerminalConfig = YES;
			continue;
		}

		if(YES == IsCustomTag(szLineBuf))
		{
			ucCusTagSetting = 1;
			continue;
		}
		
		if(YES == IsDrlElement(szLineBuf, &ucDrlType))
		{
			ucDrlIsSetting = 1;
			continue;
		}
				
		if (YES == IsElement(szLineBuf))
		{
			if (ucCusTagSetting)
			{
				SetCustomTag(szLineBuf, &strEmvTag);
				continue;
			}

			if (ucDrlIsSetting)
			{
				if (ucDrlType == DRL_TYPE_PAYWAVE)
				{		
					SetPayWaveDrl(szLineBuf, &strPayWaveDrl);
				}
				else if(ucDrlType == DRL_TYPE_AMEX)
				{
					SetAmexDrl(szLineBuf, &strAmexDrl);
				}
				continue;
				
			}

			if (nConfigType == 1) //AID
			{
				nRet = ParseAID(szLineBuf, szAidTlv, &nTlvLen, pTrasTypeAidTlv, &nPaypassAidAddNum);
			}
			else //CAPK
			{
				nRet = ParseCapk(szLineBuf, &stCapk);
			}
			
			if (nRet != 0)
			{
				PubFsClose(nFd);
				return nRet;
			}

			continue;
			
		}
		
		if (YES == IsNodeEnd(NODE_CUSTOM_TAG, szLineBuf))
		{
			AddTagOne(uCustagTlv, &strEmvTag, &nTagOffset);
			ucCusTagSetting = 0;
			continue;
		}

		if (YES == IsNodeEnd(NODE_DRL, szLineBuf))
		{
			AddDrlOne(ucDrlTlv, &strPayWaveDrl, &strAmexDrl, ucDrlType, &nDrlOffset);
			ucDrlIsSetting = 0;
			continue;
		}

		if (YES == IsNodeEnd(NODE_ENTRY, szLineBuf))
		{
			if(nTagOffset > 0)
			{
				TRACE("nTagOffset = %d", nTagOffset);
				TRACE_HEX((char *)uCustagTlv, nTagOffset, "Custom Tag [1F811F]: ");
				TlvAddEx((const uchar *)"1F811F", nTagOffset, (char *)uCustagTlv, szAidTlv, &nTlvLen);
				nTagOffset = 0;
				ucCusTagSetting = 0;
			}
		
			if (nDrlOffset > 0)
			{
				AddDrlAll(ucDrlTlv, szAidTlv, &nTlvLen, nDrlOffset, ucDrlType);
				nDrlOffset = 0;
				ucDrlIsSetting = 0;
			}
			
			if (nConfigType == 1) 
			{
				TRACE("nAidInterface=%d", cardInterface);
				TRACE("nIsTerminalConfig=%d", nIsTerminalConfig);

				TRACE_HEX(szAidTlv, nTlvLen, "TLV CONFIG:");
				
				if (YES == nIsTerminalConfig)
				{
					nRet = TxnL3LoadTerminalConfig(cardInterface, (uchar *)szAidTlv, &nTlvLen, CONFIG_UPT);
					nIsTerminalConfig = NO;
				}
				else
				{
					if(gnPaypassAidControl != 0)
					{
					   LoadPaypassTranTypeAID(szAidTlv,nTlvLen ,pTrasTypeAidTlv,nPaypassAidAddNum);
					   nPaypassAidAddNum = 0;
					}

					nRet = TxnL3LoadAIDConfig(cardInterface, NULL, (uchar *)szAidTlv, &nTlvLen, CONFIG_UPT);
				}
				if (nRet != 0) {
					PubMsgDlg("load aid", "load aid fail", 1, 1);
					TRACE("nRet = %d", nRet);
				}
				memset(szAidTlv, 0, sizeof(szAidTlv));
				memset(pTrasTypeAidTlv,0,sizeof(pTrasTypeAidTlv));
				memset(uCustagTlv, 0, sizeof(uCustagTlv));
				memset(ucDrlTlv, 0, sizeof(ucDrlTlv));
				nTlvLen = 0;
				
			}
			else //CAPK
			{
				nRet = TxnL3LoadCAPK(&stCapk, CONFIG_UPT);
				if (nRet != 0) {
					PubMsgDlg("load capk", "load capk fail", 1, 1);
					TRACE("nRet = %d", nRet);
				}
		 		memset(&stCapk, 0, sizeof(stCapk));
			}
			continue;
		}

	}

	PubFsClose(nFd);
	return 0;
}

/**
* @fn ParseAID
* @brief Parse AID and add Tag-Len-Value to szAidTlv
* @param [in]  szLineBuf		 [The data to parse]
* @param [out] szAidTlv 		 [Output the TLV of the AID]
* @param [out] nTlvLen   		 [The length of AidTlv]
* @param [out] Tlvlist			 [Output the TLV list]
* @param [out] pnPaypassAidAddNum	 [The number of Tlv]		
* @return 
* @li = 0 success
* @li < 0 fail
* @author
* @date
*/
int ParseAID(char *szLineBuf, char *szAidTlv, int *nTlvLen ,PaypassTranTypeAID *Tlvlist, int *pnPaypassAidAddNum)
{
	int nRet;
	char szTag[10];
	char szValue[500] = {0};
	char szHexVal[250] = {0};
	int nValLen = 0;
	char szTranstypevalue[2+1] = {0};
	char szHexTranstypevalue[2+1] = {0};
	int nTagvalueLen = 0;

	memset(szTag, 0, sizeof(szTag));
	nRet = GetAttribute("tag", szTag, szLineBuf);
	if (nRet != 0)
	{
		TRACE("GetAttribute fail, nRet=%d", nRet);
		return nRet;
	}
	
	memset(szValue, 0, sizeof(szValue));
	nRet = GetAttribute("value", szValue, szLineBuf);
	if (nRet != 0)
	{
		TRACE("GetAttribute fail, nRet=%d", nRet);
		return nRet;
	}

//	TRACE("[%s]:[%s]", szTag, szValue);
	nValLen = strlen(szValue);
	memset(szHexVal, 0, sizeof(szHexVal));
	PubAscToHex((uchar *)szValue, nValLen, 1, (uchar * )szHexVal);
	nValLen = (nValLen+1)>>1;
	
	nRet = GetAttribute("TransType", szTranstypevalue, szLineBuf);
	if (nRet == 0)
	{
		nTagvalueLen = strlen(szTranstypevalue);
		PubAscToHex((uchar *)szTranstypevalue, nTagvalueLen, 1, (uchar * )szHexTranstypevalue);
		TRACE("szHexTagvalue =0x%02x",szHexTranstypevalue[0]);

		AddPaypassAidTlv(szHexTranstypevalue,Tlvlist,szTag,nValLen,szHexVal,pnPaypassAidAddNum);

		gnPaypassAidControl = 1;
		return 0;

	}

	TlvAddEx((uchar *)szTag, nValLen, szHexVal, szAidTlv, nTlvLen);

	return 0;
	
}

/**
* @fn ParseCapk
* @brief Parse Capk
* @param [in]  szLineBuf [The data to parse]
* @param [out] stCapk    [Output the CAPK]
* @return 
* @li = 0 success
* @li < 0 fail
* @author
* @date
*/
int ParseCapk(char *szLineBuf, L3_CAPK_ENTRY *stCapk)
{
	int nRet;
	char szKey[50] = {0};
	char szValue[500] = {0};
	char szHexVal[250] = {0};
	int nValLen = 0;

	memset(szKey, 0, sizeof(szKey));
    nRet = GetAttribute("key", szKey, szLineBuf);
	if (nRet != 0)
	{
	    TRACE("GetAttribute fail, nRet=%d", nRet);
		return nRet;
	}
	
	memset(szValue, 0, sizeof(szValue));
	nRet = GetAttribute("value", szValue, szLineBuf);
	if (nRet != 0)
	{
	    TRACE("GetAttribute fail, nRet=%d", nRet);
		return nRet;
	}

//	TRACE("[%s]:[%s]", szKey, szValue);
	nValLen = strlen(szValue);
	memset(szHexVal, 0, sizeof(szHexVal));
	PubAscToHex((uchar *)szValue, nValLen, 1, (uchar * )szHexVal);
	nValLen = (nValLen+1)>>1;
	
	if (0 == strcmp(szKey, "RID"))
	{
	    memcpy(stCapk->rid, szHexVal, 5);
	} 
	else if (0 == strcmp(szKey, "Index"))
	{
	    stCapk->index = (szHexVal[0] & 0xFF);
//		TRACE("stCapk.index=0x%X", stCapk.index);
	}
	else if (0 == strcmp(szKey, "Hash"))
	{
	    memcpy(stCapk->hashValue, szHexVal, 20);
	}
	else if (0 == strcmp(szKey, "Exponent"))
	{
	    if (0 == memcmp(szValue, "03", 2))
	    {
	        stCapk->pkExponent[0] = 0x00;
			stCapk->pkExponent[1] = 0x00;
			stCapk->pkExponent[2] = 0x03;
	    }
		else
		{
		    memcpy(stCapk->pkExponent, szHexVal, 3);
		}
	}
	else if (0 == strcmp(szKey, "Modulus"))
	{
		stCapk->pkModulusLen = nValLen;
	    memcpy(stCapk->pkModulus, szHexVal, stCapk->pkModulusLen);
	}
	else if (0 == strcmp(szKey, "Hash Algorithm"))
	{
		stCapk->hashAlgorithmIndicator = atoi(szValue);
//		TRACE("stCapk.hashAlgorithmIndicator=%d", stCapk.hashAlgorithmIndicator);
	}
	else if (0 == strcmp(szKey, "Sign Algorithm"))
	{
		stCapk->pkAlgorithmIndicator = atoi(szValue);
	}

	return 0;
}


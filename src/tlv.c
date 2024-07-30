/**************************************************************************
* Copyright (C) 2019 Newland Payment Technology Co., Ltd All Rights Reserved
*
* @file  tlv.c
* @brief 
* @version 1.0
* @author linld
* @date 2019-9-2
**************************************************************************/


#include "apiinc.h"
#include "libapiinc.h"
#include "appinc.h"

#define is_constructed(byte)        ((byte) & 0x20)
#define is_primitive(byte)          !is_constructed(byte)
#define has_subsequent(byte)        (((byte) & 0x1F) == 0x1F)
#define another_byte_follow(byte)   ((byte) & 0x80)
#define lenlen_exceed_one(byte)     ((byte) & 0x80)
#define is_cpos_private(byte)        (((byte) & 0xFF) == 0xFF)


static int decode_tlv_recur(int nParent, tlv_t * pobj, int objspace, int deflag);
static int decode_lv_recur(int nParent, tlv_t * pobj, int objspace, int deflag);
static int parse_lv(unsigned char * ptlvstr, int tlvlen, tlv_t * pobj, int objspace, int deflag);


/*	Func:	Parse TLV string
  *	Para:	nParent		Ordinal number of parent node
  *			*pobj		Store parsed TLV structures
  *			objspace		Maximum capacity of TLV structure for storage analysis
  *			deflag		Analytic tag
  *						SINGLE_TLVOBJ		a constructed object
  *						STRING_TLVOBJ		a tlv object string not in a constructed 
  *						DECODE_LEVEL1		Only child node TLV strings are resolved
  											(Do not continue to analyze successor nodes of child nodes)
  *	Return:				Success		0
  *						Fail		<0
  *						*/
static int decode_tlv_recur(int nParent, tlv_t * pobj, int objspace, int deflag)
{

	int itrack = 0, i, tmplen;
	int tlvpos = nParent + 1;
	unsigned int tmptag;
	uchar * ptlvstr = pobj[nParent].pusValue;
	int tlvlen = pobj[nParent].unValueLen;

	while (itrack < tlvlen)
	{/* omit the 00H and FFH between objects */
		if ((*(ptlvstr + itrack) == 0x00) /*|| (*(ptlvstr + itrack) == 0xFF)*/)
		{
			itrack ++;
			continue;
		}
		if (tlvpos == objspace)
		{
			return -1;
		}
		if (is_primitive(*(ptlvstr + itrack)))
		{
			pobj[tlvpos].nChildNum = -1; /* without any children */
		} else
		{
			pobj[tlvpos].nChildNum = 0;/* may have, but now zero */
		}
       
		/* tag name coding */
		tmptag = 0;
		pobj[tlvpos].pusTagPtr = ptlvstr + itrack;
		/* xuehj modify s3pos */	
		if (has_subsequent(ptlvstr[itrack]))
		{
			i = 0;
			do {
				tmptag = (tmptag | ptlvstr[itrack ++]) << 8 ;
				if ( ++ i == 4 || itrack == tlvlen)
				{
					return -1;
				}	  
			} while (another_byte_follow(ptlvstr[itrack]));
		}
		pobj[tlvpos].unTagName = tmptag | ptlvstr[itrack ++];
		if (itrack == tlvlen)
		{
			return -1;
		}

		/* length field */
		pobj[tlvpos].pusLenPtr = ptlvstr + itrack;
		tmplen = ptlvstr[itrack] & 0x7F;
		if (lenlen_exceed_one(ptlvstr[itrack ++]))
		{
			if (tmplen > 4 || tmplen > tlvlen - itrack)
			{
				return -1;
			}
			i = tmplen;
			tmplen = 0;
			while (i > 0) {
				tmplen |= ptlvstr[itrack ++] << ((--i)  << 3);
			}
		}
		if (tmplen > tlvlen - itrack)
		{
			return -1;
		}

		pobj[tlvpos].unValueLen = tmplen;
		pobj[tlvpos].nParent = nParent;
		pobj[tlvpos].pusValue = ptlvstr + itrack;
		itrack += tmplen;
		pobj[tlvpos].unOffLen = ptlvstr + itrack - pobj[tlvpos].pusTagPtr;

		if (pobj[tlvpos].nChildNum == 0 && !(deflag & DECODE_LEVEL1)) 
		{
			if ((tmplen = decode_tlv_recur(tlvpos, pobj, objspace, deflag)) < 0)
			{
				return tmplen;
			}
			tlvpos += pobj[tlvpos].nChildNum;
		}
		tlvpos ++;
	}

	pobj[nParent].nChildNum = tlvpos - nParent - 1;
	return 0;
}

/*	Func:	Parsing TLV string to tlv_t *pobj structure.
			(pobj[0]The relevant information of the recorded TLV does not store the parsed TLV object,
			Actual storage starts with pobj[1])
			
				pobj[0].pusValue 		Save TLV string pointer
				pobj[0].unValueLen 		Save TLV string length
				pobj[0].nChildNum       How many TLV nodes have been resolved this time
						
  *	Para:	*ptlvstr		The first pointer to the incoming TLV string
  *			 tlvlen		TLV string length
  *			*pobj		Stores the parsed TLV data structure array pointer
  *			objspace		Maximum space limit for pobj of stored TLV
  *			deflag		SINGLE_TLVOBJ		TLV string is a single structured data object
  *						STRING_TLVOBJ		TLV object string sets(tag+len+value+tag+len+value...)
  						DECODE_LEVEL1		Only child node TLV strings are resolved
  											(If the child node is a structured data object, it will not continue to parse its child node.)
  *	Return:			parsing success		0
  *					parsing fail		<0
  */
int parse_tlv(uchar * ptlvstr, const int tlvlen, tlv_t * pobj, int objspace, int deflag)
{
	int ret;

	if (ptlvstr == NULL || pobj == NULL || objspace == 0)
	{
		return -1;
	}
	if (*ptlvstr == 0x00 /*|| *ptlvstr == 0xFF*/)
	{
		/* wrong starting */
		return -1;
	}

//	memset(pobj, 0x00, sizeof (tlv_t) * objspace);
	pobj[0].pusValue = (uchar *)ptlvstr;		/*Save TLV string pointer*/
	pobj[0].unValueLen = tlvlen;		/*Save TLV string length*/

	/*Call the actual analytic function*/
	if ((ret = decode_tlv_recur(0, pobj,  objspace, deflag)) < 0)
	{
		return ret;
	}

	if (deflag & SINGLE_TLVOBJ)
	{
		if (pobj[1].unOffLen  != tlvlen)
		{
			return -1;
		}
	}
	return 0;
}

/*	Func:	Obtaining TLV data of tagname in tlv_t pobj structure.
  *	Para:	nParent		Start the search from this parent node
  *			unTagName		The tagname to find
  *			*pobj		Tlv_t structure to search for
  *			level			Level of search
  						SEARCH_ONLY_SON		Find only in their own child nodes
  						SINGLE_TLVOBJ		Find at all subsequent nodes
  *	Return:				find success	Returns the child node serial number
  						find fail		0
  */
int fetch_tlv(int nParent,  unsigned int unTagName, tlv_t * pobj, int level)
{
	int i = (nParent < 0 ? 0 : nParent) + 1;
	int end ;

	/* if no objects or nParent referred to a primitive element */
	if (pobj == NULL || pobj[i - 1].nChildNum < 0)
	{
		return 0;
	}

	end = pobj[i - 1].nChildNum + i;

	while (i < end)
	{
		if (pobj[i].unTagName == unTagName)
		{
			return i;
		}
		if (level == STRING_TLVOBJ)
		{
			/*Skip child nodes of child nodes*/
			if (pobj[i].nChildNum > 0)
			{
				i += pobj[i].nChildNum;
			}
		}
		i ++;
	}
	return 0;
}

/**
* @fn Tlv_Parse
* @brief parse tlv string
* @param in ptlvstr TLV string buffer pointer
* @param in tlvlen TLV string buffer size
* @param out pobj,   Stores the tlv structure pointer (pobj[0]) that has been parsed, records the relevant information of the TLV,
						and does not store the parsed TLV object,Actual storage starts with pobj[1])
			
				pobj[0].pusValue 		Save TLV string pointer
				pobj[0].unValueLen 		Save TLV string length
				pobj[0].nChildNum           How many TLV nodes have been resolved this time
* @param in objspace  Size of tlv Structure Data Stored and Resolved
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int TlvParse(uchar *pszTlvBuf, const int nTlvBufLen, tlv_t *pstTlvObj, int nTlvArrayNum)
{
	return parse_tlv((uchar *)pszTlvBuf,  nTlvBufLen,   pstTlvObj,  nTlvArrayNum,  DECODE_LEVEL1);
}

/**
* @fn Tlv_Parse
* @brief Tlv_GetValue
* @param in unTagName Tagname
* @param in pobj Stores the parsed tlv structure pointer, which is the tlv_t structure pointer obtained through Tlv_Parse
* @param out psVal The value of the resulting tlv
* @param in out pnSize  In *pnSize is the size of the psVal buffer and out is the actual size of the obtained psVal.
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int TlvGet(const uint unTag, void *pszOutValue, int *pnOutLen, tlv_t *pstTlvObj)
{
	int nRet;
	
	nRet = fetch_tlv( 0, unTag, pstTlvObj, SINGLE_TLVOBJ);
	if (nRet == 0)
	{
		return -1;
	}
//	if (*pnOutLen < pstTlvObj[nRet].unValueLen)
//	{
//		return -2;
//	}
	if (NULL != pszOutValue)
	{
		memcpy(pszOutValue, pstTlvObj[nRet].pusValue, pstTlvObj[nRet].unValueLen);
		*pnOutLen = pstTlvObj[nRet].unValueLen;
	}
	return 0;
}

#if 0
/**
* @fn Tlv_Tag2List
* @brief Analyzing a Tag string, and analyzing the tag string into a tag list
* @param in psTag Tag string pointer
* @param in nLen Tag string length
* @param out nTagList The resulting tag list array
* @param in nSize  The size of the tag list array
* @return
* @li The resulting nTaglist size
* @li APP_FAIL
*/
int Tlv_Tag2List(unsigned char *psTag, const unsigned int nLen, unsigned int nTagList[], int nSize)
{
	unsigned char *p = psTag;
	int nCount = nLen;
	int i = 0;
	int nTagLen = 0;

	if (NULL == nTagList || nSize <= 0)
	{
		return -1;
	}
	
	memset(nTagList, 0, sizeof(int)*nSize);
	
	while(*p != 0x00 && nCount > 0 && i < nSize)
	{
		GetTagByBuf((const char*)p, (int *)&nTagList[i], &nTagLen);
		p += nTagLen;
		nCount -= nTagLen;
		i++;
	}
	return i;
}
#endif /* if 0. 2015-12-17 10:56:42 linld */
/**
* @fn FetchTagByBuf
* @brief parsing a Tag string, and analyzing the tag string into a tag list
* @param in psTag Tag string pointer
* @param in nLen Tag string length
* @param in nTag TAG value to match
* @param in nSize  The size of the tag list array
* @return
* @li TAG VALUE got
* @li APP_FAIL
*/
int FetchTagByBuf(unsigned char *psTag, const unsigned int nLen, char* cFTag, char* psOut,int* pnOutLen)
{
	int nOff = 0;
	uchar *p = psTag;
	uchar *start = psTag;
	uchar *pRet;

	if( memcmp(p,cFTag,3) == 0)
	{
		pRet = p;
		p = p + 3;
		nOff = *p;
		memcpy(psOut,pRet,nOff+1+3);
		* pnOutLen = nOff+1+3;
		return APP_SUCC;
	}
	while((p-start)<nLen)
	{
		p = p + 3;
		nOff = *p;
		p++;
		p = p + nOff;
		if( memcmp(p,cFTag,3) == 0)
		{
			pRet = p;
			p = p + 3;
			nOff = *p;
			memcpy(psOut,pRet,nOff+1+3);
			* pnOutLen = nOff+1+3;
			return APP_SUCC;
		}		
	}
	* pnOutLen = 0;
	return APP_FAIL;
}

/*	Func:	Parse LV string
  *	Para:	nParent		Ordinal number of parent node
  *			*pobj		Store parsed TLV structures
  *			objspace		Maximum capacity of TLV structure for storage analysis
  *			deflag		parse tag
  *						SINGLE_TLVOBJ		a constructed object
  *						STRING_TLVOBJ		a tlv object string not in a constructed 
  *						DECODE_LEVEL1		Only child node TLV strings are resolved
  											(Do not continue to analyze successor nodes of child nodes)
  *	Return:				success		0
  *						fail		<0
  *						*/
static int decode_lv_recur(int nParent, tlv_t * pobj, int objspace, int deflag)
{
	int itrack = 0, i, tmplen;
	int tlvpos = nParent + 1;
	uchar * ptlvstr = pobj[nParent].pusValue;
	int tlvlen = pobj[nParent].unValueLen;

	while (itrack < tlvlen)
	{
		if ((*(ptlvstr + itrack) == 0x00) /*|| (*(ptlvstr + itrack) == 0xFF)*/) {
			itrack ++;
			continue;
		}
		if (tlvpos == objspace)
		{
			return -2;
		}
		
		/* length field */
		pobj[tlvpos].pusLenPtr = ptlvstr + itrack;
		tmplen = ptlvstr[itrack] & 0x7F;
		if (lenlen_exceed_one(ptlvstr[itrack ++]))
		{
			if (tmplen > 4 || tmplen > tlvlen - itrack)
			{
				return -3;
			}
			i = tmplen;
			tmplen = 0;
			while (i > 0)
			{
				tmplen |= ptlvstr[itrack ++] << ((--i)  << 3);
			}
		}
		if (tmplen > tlvlen - itrack)
		{
			return -4;
		}

		pobj[tlvpos].unValueLen = tmplen;
		pobj[tlvpos].nParent = nParent;
		pobj[tlvpos].pusValue = ptlvstr + itrack;
		itrack += tmplen;
		pobj[tlvpos].unOffLen = ptlvstr + itrack - pobj[tlvpos].pusTagPtr;

		tlvpos ++;
	}

	pobj[nParent].nChildNum = tlvpos - nParent - 1;
	return 0;

}

/*	Func:	Parsing LV string to tlv_t *pobj structure.
			(Pobj[0] records TLV related information without storing the parsed TLV objects,
			Actual storage starts with pobj[1])
			
				pobj[0].pusValue 		Save TLV string pointer
				pobj[0].unValueLen 		Save TLV string length
				pobj[0].nChildNum           How many TLV nodes have been resolved this time
						
  *	Para:	*ptlvstr		The first pointer to the incoming TLV string
  *			 tlvlen		TLV string length
  *			*pobj		Stores the parsed TLV data structure array pointer
  *			objspace		Maximum space limit for pobj of stored TLV
  *			deflag		SINGLE_TLVOBJ		TLV string is a single structured data object
  *						STRING_TLVOBJ		TLV object string set(tag+len+value+tag+len+value...)
  						DECODE_LEVEL1		Only child node TLV strings are resolved
  											(If the child node is a structured data object, it will not continue to parse its child node.)
  *	Return:			parsing success		0
  *					parsing fail		<0
  */
static int parse_lv(unsigned char * ptlvstr, int tlvlen, tlv_t * pobj, int objspace, int deflag)
{
	int ret;

	if (ptlvstr == NULL || pobj == NULL || objspace == 0)
	{
		return -1;
	}

//	memset(pobj, 0x00, sizeof (tlv_t) * objspace);
	pobj[0].pusValue = ptlvstr;		/*Save TLV string pointer*/
	pobj[0].unValueLen = tlvlen;		/*Save TLV string length*/

	/*Call the actual lv analytic function*/
	if ((ret = decode_lv_recur(0, pobj,  objspace, deflag)) < 0) {
		return ret;
	}

	return 0;
}

/**
* @fn Lv_Parse
* @brief parse tlv strings
* @param in ptlvstr LV string buffer pointe
* @param in tlvlen LV string buffer size
* @param out pobj The resolved Tlv object is not stored in the LV structure pointer (pobj[0]) that stores LV related information,
			Actual storage starts with pobj[1])
			
				pobj[0].pusValue 		Save TLV string pointer
				pobj[0].unValueLen 		Save TLV string length
				pobj[0].nChildNum           How many TLV nodes have been resolved this time
* @param in objspace  Size of tlv Structure Data Stored and Resolved
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int LvParse(unsigned char * pLvstr, int nLvlen, tlv_t *pstTlvObj, int nTlvArrayNum)
{
	return parse_lv( pLvstr,  nLvlen,   pstTlvObj,  nTlvArrayNum, 0);
}



/*
*	@brief          add a Tag-Len-Value to oTlvBuf
*	@param          const uint		iTag
*	@param          const int		iLen
*	@param          const char		*iValue
*	@return			TLVERR_xxx
*/
int TlvAdd(const uint unTag, const int nInLen, const char *pszInValue, char *pszOutTlvBuf, int *pnOutTlvBufLen)
{
	uchar usTag[20] = {0};
	
	sprintf((char*)usTag, "%x", unTag);

	return TlvAddEx(usTag, nInLen, pszInValue, pszOutTlvBuf, pnOutTlvBufLen);

}

int TlvAddEx(const uchar *usTag, const int nInLen, const char *pszInValue, char *pszOutTlvBuf, int *pnOutTlvBufLen)
{
	int i, nTagLen,nLen, nLenLen;
	char sLenBuf[8];
	
	nTagLen = strlen((char*)usTag);
	PubAscToHex((uchar *)usTag, nTagLen, 0, (uchar *)(pszOutTlvBuf+*pnOutTlvBufLen));
	nTagLen >>= 1;
	*pnOutTlvBufLen += nTagLen;

	if (nInLen > 127)
	{
		nLen = nInLen;
		nLenLen = nInLen/256+1;	//Follow n bytes
		sLenBuf[0] = 128 | nLenLen;//zj & to |
		for (i=0; i<nLenLen; i++)
		{
			sLenBuf[nLenLen-i] = nLen%256;
			nLen /= 256;
		}
	}
	else
	{
		sLenBuf[0] = nInLen;
		nLenLen = 0;
	}
	memcpy(pszOutTlvBuf+*pnOutTlvBufLen, sLenBuf, nLenLen+1);
	*pnOutTlvBufLen += nLenLen+1;
	
	if(pszInValue != NULL)
	{
		memcpy(pszOutTlvBuf+*pnOutTlvBufLen, pszInValue, nInLen);
		*pnOutTlvBufLen += nInLen;
	}
	
	return APP_SUCC;
}

/*
*	@brief          update a Tag-Len-Value too TlvBuf
*	@param          const uint		iTag
*	@param          const int		iLen
*	@param          const char		*iValue
*	@return			TLVERR_xxx
*/
int TlvUpdate(const uint unTag, const int nInLen, const char *pszInValue, char *pszOutTlvBuf, int *pnOutTlvBufLen)
{
	int nRet = 0;
	tlv_t stTlvObj[100];
	char *pTmp = NULL;
	int nIndex = 0;
	int nLen1 = 0;/**The data is divided into three segments with unTag as the node*/
	int nLen2 = 0;
	int nLen3 = 0;

	memset(stTlvObj, 0, sizeof(tlv_t) * 100);
	
	//TRACE_HEX("TLV before update:", pszOutTlvBuf, *pnOutTlvBufLen);
	
	nRet = TlvParse((uchar *)pszOutTlvBuf, *pnOutTlvBufLen, stTlvObj, 100);
	if (nRet < 0)
	{
	    return APP_FAIL;
	}

	nIndex = fetch_tlv( 0, unTag, stTlvObj, SINGLE_TLVOBJ);
	if (nIndex ==0)
	{
		//TRACE("Tag[%X]no found", unTag);
	    //This tag was not found and was appended directly
		return TlvAdd(unTag, nInLen, pszInValue, pszOutTlvBuf, pnOutTlvBufLen);
	}

	nLen1 = stTlvObj[nIndex].pusTagPtr - stTlvObj[0].pusValue;
	nLen2 = stTlvObj[nIndex].unOffLen;
	nLen3 = *pnOutTlvBufLen - nLen1 - nLen2;
	
	//TRACE("Total length of original TLV string[%d]", *pnOutTlvBufLen);
	//TRACE("Length len1 of TLV string before this Tag[%d]", nLen1);
	//TRACE("Data length len2 of original Tlv[%d]", nLen2);
	//TRACE("Backup data length len3[%d]", nLen3);
	
	//Copy the data after this TLV
	if (nLen3 > 0)
	{
		pTmp = (char *)malloc(nLen3+1);
		memset(pTmp, 0, nLen3+1);

	    memcpy(pTmp, pszOutTlvBuf + nLen1+ nLen2, nLen3);
	}
	

	memset(pszOutTlvBuf + nLen1, 0, *pnOutTlvBufLen - nLen1);

	TlvAdd(unTag, nInLen, pszInValue, pszOutTlvBuf, &nLen1);
	
	//TRACE("The new length len1 after adding this TLV[%d]", nLen1);
	
	//Add back-up data before
	if (nLen3 > 0)
	{
	    memcpy(pszOutTlvBuf+nLen1, pTmp, nLen3);
		free(pTmp);
	}
	
	*pnOutTlvBufLen = nLen1 + nLen3;

	//TRACE_HEX("Updated TLV:", pszOutTlvBuf, *pnOutTlvBufLen);
	return APP_SUCC;
}


/*
*	@brief          delete a Tag-Len-Value from oTlvBuf
*	@param          const uint		iTag
*	@param          const int		iLen
*	@param          const char		*iValue
*	@return			TLVERR_xxx
*/
int TlvDelete(const uint unTag, char *pszOutTlvBuf, int *pnOutTlvBufLen)
{
	int nRet = 0;
	tlv_t stTlvObj[100];
	char *pTmp = NULL;
	int nIndex = 0;
	int nLen1 = 0; /**The data is divided into three segments with unTag as the node*/
	int nLen2 = 0;
	int nLen3 = 0;


	memset(stTlvObj, 0, sizeof(tlv_t) * 100);
	
	//TRACE_HEX("TLV before deletion:", pszOutTlvBuf, *pnOutTlvBufLen);
	
	nRet = TlvParse((uchar *)pszOutTlvBuf, *pnOutTlvBufLen, stTlvObj, 100);
	if (nRet < 0)
	{
	    return APP_FAIL;
	}

	nIndex = fetch_tlv( 0, unTag, stTlvObj, SINGLE_TLVOBJ);
	if (nIndex ==0)
	{
		//TRACE("Tag[%X]no found", unTag);
	    //This tag was not found and returned successfully.
		return APP_SUCC;
	}

	nLen1 = stTlvObj[nIndex].pusTagPtr - stTlvObj[0].pusValue;
	nLen2 = stTlvObj[nIndex].unOffLen;
	nLen3 = *pnOutTlvBufLen - nLen1 - nLen2;
	
	//TRACE("Total length of original TLV string[%d]", *pnOutTlvBufLen);
	//TRACE("Length len1 of TLV string before this Tag[%d]", nLen1);
	//TRACE("Data length len2 of original Tlv[%d]", nLen2);
	//TRACE("Backup data length len3[%d]", nLen3);
	
	//Copy the data after this TLV
	if (nLen3 > 0)
	{
		pTmp = (char *)malloc(nLen3+1);
		memset(pTmp, 0, nLen3+1);
		memcpy(pTmp, pszOutTlvBuf + nLen1+ nLen2, nLen3);
	}	

	memset(pszOutTlvBuf + nLen1, 0, *pnOutTlvBufLen - nLen1);
	
	//Add back-up data before
	if (nLen3 > 0)
	{
	    memcpy(pszOutTlvBuf+nLen1, pTmp, nLen3);
		free(pTmp);
	}
	
	*pnOutTlvBufLen = nLen1 + nLen3;

	//TRACE_HEX("TLV after deletion:", pszOutTlvBuf, *pnOutTlvBufLen);
	return APP_SUCC;
}


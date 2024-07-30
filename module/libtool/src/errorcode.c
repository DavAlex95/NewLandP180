/**
* @file errorcode.c
* @brief Error Module
* @version  1.0
* @author Liug
* @date 2012-10-24
*/
#include "ltool.h"

static int gnErrorCode = 0 ;
static int gnNAPIErrorCode = 0 ;
static char gszErrTip[MAX_ERR_BUF_LEN+1]={0};

/**
* @brief Get Error Code
* @param [out]  pnErrorCode			Module error code
* @param [out]  pszErr				Module error info
* @param [out]  pnNapiErrorCode		NAPI error code
* @return 
* @li APP_FAIL 
* @li APP_SUCC 
* @author Liug
* @date 2012-10-22
*/
void PubGetErrorCode(int *pnErrorCode, char *pszErr, int *pnNapiErrorCode)
{
	if (pnErrorCode != NULL)
	{
		*pnErrorCode = gnErrorCode; 
	}
	if (pnNapiErrorCode != NULL)
	{
		*pnNapiErrorCode = gnNAPIErrorCode;
	}
	if (pszErr != NULL)
	{
		strcpy(pszErr,gszErrTip);
	}
}

/**
* @brief Set Error Code
* @param [in]  nErrorCode            Module error code
* @param [in]  nNapiErrCode         NAPI error code
* @return 
* @li APP_FAIL 
* @li APP_SUCC 
* @author Liug
* @date 2012-10-22
*/
void PubSetErrorCode(int nErrCode,const char *pszErr,int nNapiErrCode)
{
	int nLen;

	if(nErrCode != 0)
	{
		gnErrorCode = nErrCode ;
	}
	//if(nNapiErrCode != 0)
	{
		gnNAPIErrorCode = nNapiErrCode ;
	}
	if (pszErr != NULL)
	{
		nLen=strlen(pszErr);
		if (nLen > MAX_ERR_BUF_LEN)
		{
			nLen = MAX_ERR_BUF_LEN;
		}
		memset(gszErrTip,0,sizeof(gszErrTip));
		memcpy(gszErrTip,pszErr,nLen);
	}
}

/**
* @brief Set Error Info
* @param [in]  nErrorCode            Module error code
* @param [in]  nNapiErrCode         NAPI error code
* @param [in]  ppszErr				 
* @return 
* @li 
*/
void PubSetErrorInfo(int nErrCode,const char *ppszErr[MAX_ERR_BUF_LEN],int nNapiErrCode)
{
	int i;
	char szErrCodeBuf[10]={0};
	char szErrTipBuf[MAX_ERR_BUF_LEN+1]={0};
	
	sprintf(szErrCodeBuf,"%d",nErrCode);
	for(i=0; i<MAX_ERR_NUM; i++)
	{
		if((memcpy(szErrCodeBuf,ppszErr[i],strlen(szErrCodeBuf))==0))
		{
			strcpy(szErrTipBuf,ppszErr[i]+strlen(szErrCodeBuf));
			PubSetErrorCode(nErrCode,szErrTipBuf,0);
			break;
		}
	}
}



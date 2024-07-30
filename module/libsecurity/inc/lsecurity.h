/***************************************************************************
** All right reserved:  Newland Copyright (c) 2016-2026  
** File name:  lsecurity.h
** File indentifier: 
** Brief:  
** Current Verion:  v1.0
** Auther: sunh
** Complete date: 2016-9-26
** Modify record: 
** Modify record: 
** Modify date: 
** Version: 
** Modify content: 
***************************************************************************/
#ifndef _LSECRITY_H_
#define _LSECRITY_H_

#include "emvl3.h"
#include "napi_crypto.h"

/** @addtogroup Security
* @{
*/

#define DESMODE_DES  '0'		/**< DES MODE*/
#define DESMODE_3DES '1'		/**< 3DES MODE*/

#define DESMODE_UNDES  '2'		/**< UNDES MODE*/
#define DESMODE_3UNDES '3'		/**< 3UNDES MODE*/

#define FORCE_EXIT_PINMODE do{ \
		NAPI_SecVPPSetEvent(NAPI_SEC_VPP_KEY_ESC); \
		NAPI_SecVPPGetEvent(&nEvent, (uchar *)sPin, &nOutPinLen, szKsn, &nKsnLen); \
		NAPI_SecVPPSetEvent(NAPI_SEC_VPP_KEY_ESC); \
		NAPI_SecVPPGetEvent(&nEvent, (uchar *)sPin, &nOutPinLen, szKsn, &nKsnLen); \
    }while(0);

#define _FINISH_PIN_INPUT_THREAD(nRet) do{PubDebug("_FINISH_PIN_INPUT_THREAD : nRet = %d", nRet); FORCE_EXIT_PINMODE; return nRet;}while(0)

typedef enum 
{
	SECRITY_KEYSYSTEM_MSKEY=0x00,  			/**<MSKEY*/
	SECRITY_KEYSYSTEM_DUKPT,				/**<DUKPT*/
}EM_KEYSYSTEM;

typedef enum 
{
	SECRITY_MODE_INSIDE=0x00,  				/**<Built-in Security module inside*/
	SECRITY_MODE_PINPAD,					/**<Use pinpad*/
}EM_SECRITY_MODE;

enum EM_KEY_TYPE
{
	KEY_TYPE_PIN=0x00,  			    /**<Pin key*/
	KEY_TYPE_MAC,					    /**<Mac key*/	
	KEY_TYPE_DATA,				        /**<Data key*/
	KEY_TYPE_TMK,						/**<Master key*/
};

enum EM_MAC_TYPE
{
	MAC_TYPE_ECB=0,						/**<ECB (TMK & dukpt)*/
	MAC_TYPE_X99,						/**<X99 (TMK & dukpt)*/
	MAC_TYPE_X919,						/**<X919 (TMK & dukpt)*/
	MAC_TYPE_9606,						/**<Mac 9606 (TMK & dukpt)*/
	MAC_TYPE_AES_X99,					/**<AES x99 (TMK & dukpt)*/
	MAC_TYPE_AES_X9606,					/**<AES x99 (TMK & dukpt)*/
	MAC_TYPE_ECB_RESP,					/**<ECB RES only for dukpt*/
	MAC_TYPE_X99_RESP,					/**<X99 RESP only for dukpt*/
	MAC_TYPE_X919_RESP,					/**<X919 RESP only for dukpt*/
	MAC_TYPE_9606_RESP,					/**<Mac 9606 only for dukpt*/
	MAC_TYPE_AES_X99_RESP, 				/**<AES X99 RESP only for dukpt*/
	MAC_TYPE_AES_X9606_RESP,			/**<AES X9606 RESP only for dukpt*/
};

enum EM_PIN_TYPE
{
   PINTYPE_WITHPAN = 0,       	    /**<With PAN encrypt*/
   PINTYPE_WITHOUTPAN = 1,     		/**<Without PAN encrypt*/
   PINTYPE_PLAIN_STAR = 2,       	/**<No encrypt,Show PIN replaced by '*' */
   PINTYPE_PLAIN = 3,               /**<No encrypt,Show clear PIN*/
};

typedef struct{
	char cAux;             /**<The serial port connected to PINPAD(built-in ignored)*/
	char cTimeout;	       /**<Timeout, used when getting pin block and detecting pinpad*/
	char sReserve[63];     /**<Reserve-*/
}STPINPADPARAM;

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
int PubInitSecrity(int nMode, const STPINPADPARAM* pstPinpadParam);

/**
* @brief Clear KEY
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author Liug
* @date 2012-5-23
*/
int PubClearKey(void);

/**
* @brief Set the index of current main key.
* @param [in] nIndex Index of key
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author Liug
* @date 2012-5-23
*/
int PubSetCurrentMainKeyIndex(int nIndex);

/**
* @brief Set the key system type.
* @param [in] nType Type of key system--EM_KEYSYSTEM
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author chenwu
* @date 2018-5-25
*/
int PubSetKeySystemType(int nType);

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
int PubLoadWorkKey(int nKeyType, const char *psKey, int nKeyLen, const char *psCheckValue);

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
int PubGetPin(char *psPin, int *pnPinLen, int nMode, const char *pszCardno, const char *pszAmount,
			  int nMaxLen, int nMinLen);

/**
* @brief Calculate MAC(calling 'PubSetCurrentMainKeyIndex' to set index before calling this api)
* @param [in]  nMode         Method---EM_MAC_TYPE
* @param [in]  psData        Data that needs to be calculated
* @param [in]  nDataLen      Data length
* @param [out] psMac         MAC
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author chenwu
* @date 2018-3-22
*/
int PubCalcMac(int nMode, const char *psData, int nDataLen, char *psMac);

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
int PubGetKcv(uchar ucKeyID, EM_SEC_CRYPTO_KEY_TYPE KeyType, EM_SEC_KEY_USAGE KeyUsage, uchar *psKcv, int *pnKcvLen);

/**
* @brief Increase KSN
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author chenwu
* @date 2018-5-25
*/
int PubDukptIncreaseKSN(void);

/**
* @brief Get KSN
* @param [out] psKSN    KSN data---10byte
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author chenwu
* @date 2018-5-25
*/
int PubGetDukptKSN(char *psKSN);

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
int PubDes3(const char* psSrc, int nLen, char *psDest);

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
int PubUnDes3(const char *psSrc, int nLen, char *psDest);

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
int PubSoftDes3(const char *psKey, const char *psSrc, char *psDest);

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
int PubSoftUnDes3(const char *psKey, const char *psSrc, char *psDest);

/**
 *@brief 	Calculate 3DES
 *@param[in] nMode		Encryption mode
 *@param[in] nKeyLen     Key length (8, 16 or 24 only)
 *@param[in] psKey		Key buffer
 *@param[in] nSrcLen	length of psSrc
 *@param[in] psSrc	Buffer for data to encrypt
 *@param[out]	psDest	Output data
 *@return
 *@li \ref NAPI_OK "NAPI_OK"  				Success
 *@li \ref EM_NAPI_ERR "EM_NAPI_ERR"		Fail
*/
int PubTDes(int nMode, int nKeyLen, const char* psKey, int nSrcLen, const char* psSrc, char *psDest);

/**
 *@brief 	Calculate KCV
 *@param[in] nKeyLen     Key length (8, 16 or 24 only)
 *@param[in] pszKey		Key buffer
 *@param[out]	pszKcv	Output data
 *@return
 *@li \ref NAPI_OK "NAPI_OK"  				Success
 *@li \ref EM_NAPI_ERR "EM_NAPI_ERR"		Fail
*/
int PubCalcKCV(int nKeyLen, char *pszKey, char *pszKcv);

/**
* @brief Get security library version
* @param [out] pszVer  Version information, 12bytes
* @return 
* @li void
*/
void PubGetSecrityVerion(char *pszVer);

/**
* @brief Display string on pinpad
* @param [in]  pszLine1  String displayed in lin 1
* @param [in]  pszLine2  String displayed in lin 2
* @param [in]  pszLine3  String displayed in lin 3
* @param [in]  pszLine4  String displayed in lin 4
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author chenwu
* @date 2018-3-22
*/
int PubDispPinPad(const char *pszLine1, const char *pszLine2, const char *pszLine3, const char *pszLine4);

/**
* @brief Clear the message and display default info on pinpad(for PinPad)
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author Liug
* @date 2012-5-23
*/
int PubClrPinPad(void);

/**
* @brief pinpad beep
* @param [in]  nDuration     time of duration
* @param [in]  Buzzer tone   0 – Default   1 – Alert (750Hz)  2 – Success (1500Hz) 
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
*/
int PinPad_Beep(int nDuration, int nType);

/**
* @brief show QR Code
* @param [in]  nVersion	Version(defult 0, from 0-40)
* @param [in]  pszBuffer content of display
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
*/
int PinPad_GenAndShowQr(int nVersion, char *pszBuffer);

/**
* @brief signature
* @param [in]  pszCharaterCode	feature code
* @param [in]  name of signature
* @param [in]  nTimeOut
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
*/
int PinPad_DoSignature(char *pszCharaterCode, char *pszSignName, int nTimeOut);

/**
* @brief Scan the code through the Pinpad
* @param [out]  results of scanning.
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
*/
int PinPad_DoScan(char *pszBuffer);

/**
* @brief swipe card on Pinpad
* @param [in] STREADCARD_IN
* @param [out] STREADCARD_OUT
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
*/
int PinPad_SwipeCard(char *pIn ,char *pOut);

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
int PinPad_L3init(char *pszConfig, int nConfigLen);

/**
* @brief CAPK management(Update/Get/Remove/Flush)
* @param [IN] capk: ca publick key
* @param [IN] mode:  Update/Get/Remove/Flush
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
*/
int PinPad_L3LoadCapk(L3_CAPK_ENTRY *capk, L3_CONFIG_OP mode);

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
int PinPad_L3LoadAIDConfig(L3_CARD_INTERFACE cardInterface, L3_AID_ENTRY *aidEntry, unsigned char tlv_list[], int *tlv_len, L3_CONFIG_OP mode);

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
int PinPad_L3LoadTerminalConfig(L3_CARD_INTERFACE cardInterface, unsigned char tlv_list[], int *tlv_len, L3_CONFIG_OP mode);

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
int PinPad_L3CompleteTransaction(int nResult, char *pszInput, int nInPutLen, L3_TXN_RES *res, char *pszResPonseCode);

/**
*@brief Terminate transaction and release resource.
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
*/
int PinPad_L3TerminateTransaction();

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
int PinPad_L3GetData(int nTag, char *pszOut, int nMaxLen);

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
int PinPad_L3PerformTransaction(char *pszInput, int nInPutLen, char *pszOut, int *pnOutLen);

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
int PinPad_L3GetTlvData(unsigned int *tagList, unsigned int tagNum, unsigned char *tlvData, unsigned int maxLen,int ctl);

/**
* @brief Setup kernel data.
* @param [IN]  tag: Tag.
* @param [IN]  data: The setting data buffer.
* @param [IN]  len: The length of data.
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
*/
int PinPad_L3SetData(unsigned int tag, void *data, unsigned int len);

/**
* @brief Setup Debug Mode
* @param [IN]debugLV   LV_CLOSE / LV_DEBUG / LV_ALL
* @li APP_FAIL Fail
* @li APP_SUCC Success
*/
int PinPad_L3SetDebugMode(int nDebugLv);

/**
* @brief Enumerate CAPK
* @param in  start        ---Start index
* @param in  end          ---End index
* @param out capk        ---CAPK Lists
* @return
* @li        >0            Number of public key
* @li        <= 0          FAIL
*/
int PinPad_L3EnumCapk(int start, int end, char capk[][6]);

/**
* @brief  Enumerate all AID, include terminal config 
* @param [IN] interface: Contact/Contactless
* @param [OUT] aidEntry: AID Lists
* @param [IN]maxCount:   Max size of aidEntry
* @return
* @li >0  Number of AID(<= maxCount)
* @li <= 0 FAIL
*/
int PinPad_L3EnumEmvConfig(L3_CARD_INTERFACE interface, L3_AID_ENTRY * aidEntry, int maxCount);

/*
* @brief set font size
* @param [in] nSize 1 - normal 2- small 3- large
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
*/
int PinPad_SetFontSize(char cSize);

/**
* @brief send L3 perform data to Pinpad
* @param in  pL3Cfg        --- request data
* @return
* @li        0           succ
*/
int PinPad_L3PerformSend(char *pL3Cfg);

/**
* @brief send  data to Pinpad
* @param in  psSend        --- request data
* @param in  nSendLen      --- request data length
* @return
* @li        0           succ
*/
int PinPad_Send(char *psSend, int nSendLen);

/**
* @brief send receive data from Pinpad
* @param[out] psRecv 	    Buffer to save data
* @param[in]  pnRecvLen     The length buffer want to read
* @param[out] pnRecvLen 	Actual length  of read
* @return
* @li        0           succ
*/
int PinPad_Recv(char *psRecv, int nMaxReadLen, int *pnRecvLen);

/** @}*/ // End of Security
#endif


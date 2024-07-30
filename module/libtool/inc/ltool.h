/***************************************************************************
** All right reserved:  Newland Copyright (c) 2016-2026
** File name:  ltool.h
** File indentifier:
** Brief:
** Current Verion:  v1.0
** Auther: liug
** Complete date: 2016-9-26
** Modify record:
** Modify record:
** Modify date:
** Version:
** Modify content:
***************************************************************************/
#ifndef _LTOOLS_H_
#define _LTOOLS_H_

#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include "lfilesystem.h"
#include "napi.h"

/** @addtogroup CommonDefinitions
* @{
*/
typedef enum
{
	NO   = '0',
	YES  = '1'
}YESORNO;

#define SOH     1
#define STX     2
#define ETX     3
#define EOT     4
#define ENQ     5
#define ACK     6
#define DLE     0x10
#define NAK     0x15
#define ETB     0x17

#define FALSE 0
#define TRUE (!FALSE)

#define APP_SUCC		(0)   	/**<Success      */
#define APP_FAIL		(-1)    /**<Fail      */
#define APP_QUIT		(-2)    /**<Quit*/
#define APP_TIMEOUT 	(-3)    /**<Timeout    */
#define APP_FUNCQUIT	(-4)    /**<Function key*/

/** @}*/ // End of common definitions


#define FILE_NOCREATEINDEX 	0x00	/**<Not create index*/
#define FILE_CREATEINDEX   	0x01	/**<Create index  */

#define MAX_ERR_BUF_LEN		(128)	/**<Maximum error buffer length */
#define MAX_ERR_NUM			(1000)	/**<Maximum error amount */

#define MAX_INPUTDATASIZE  	512
#define MAX_BMPBUFFSIZE		8192

#define DEBUG_NONE  	0
#define DEBUG_PORT  	1
#define DEBUG_FILE		2
#define DEBUG_CONSOLE	3

#define DEBUG_LEVEL_NORMAL  	0
#define DEBUG_LEVEL_WARNING  	1
#define DEBUG_LEVEL_ALL			2


enum EM_ADDCH_MODE
{
	ADDCH_MODE_BEFORE_STRING=0,			/**<Add characters before a string*/
	ADDCH_MODE_AFTER_STRING,			/**<Add characters after a string*/
	ADDCH_MODE_BEFORE_AFTER_STRING		/**<Add characters before and after a string*/
};

typedef enum
{
	SEC_CFG_UNIQUE,			/* check if installing key is unique : 1 - yes */
	SEC_CFG_MISUSE,			/* check if key is misused according to its type : 1 - yes */
	SEC_CFG_TRIES_LIMIT,	/* check if current function is overrun: 1 - yes */
	SEC_CFG_STRENGTH,		/* keys should be protected by the same or higher strength keys: 1 - yes */
	SEC_CFG_KEYLEN_LIMIT,	/* key length should be stronger than 8 bytes : 0- support */
	SEC_CFG_KSN_UPDATE,		/* KSN updated after the KSN has been used : 1 - yes */
	SEC_CFG_CLEARKEY_LIMIT, /* check if the clearkey is allowed to be installed: 0 - support */
} EM_SEC_CFG;

enum EM_HARDWARE_SUPPORT
{
	HARDWARE_SUPPORT_WIRELESS=0,		/**<Wireless modem*/
	HARDWARE_SUPPORT_RADIO_FREQUENCY,	/**<Rf module*/
	HARDWARE_SUPPORT_MAGCARD,			/**<Magnetic module*/
	HARDWARE_SUPPORT_SCANNER,			/**<Scan module*/
	HARDWARE_SUPPORT_PINPAD,			/**<Pinpad*/
	HARDWARE_SUPPORT_COMM_NUM,			/**<Serial port numbers*/
	HARDWARE_SUPPORT_USB,				/**<USB*/
	HARDWARE_SUPPORT_MODEM,				/**<MODEM*/
	HARDWARE_SUPPORT_WIFI,				/**<Wifi module*/
	HARDWARE_SUPPORT_ETH,				/**<Ethernet module*/
	HARDWARE_SUPPORT_PRINTER,			/**<Printer module*/
	HARDWARE_SUPPORT_TOUCH_SCREEN,		/**<Support touch*/
	HARDWARE_SUPPORT_LED_LIGHT			/**<Support Rf led*/
};

enum EM_HARDWARE_GET
{
	HARDWARE_GET_BIOS_VER=2,		/**<Get bios version */
	HARDWARE_GET_POS_USN,			/**<Get serial number */
	HARDWARE_GET_POS_PSN,			/**<Get merchant sn*/
	HARDWARE_GET_BOARD_VER,			/**<Mainboard version*/
	HARDWARE_GET_CREDITCARD_COUNT, 	/**<Get card using times*/
	HARDWARE_GET_PRN_LEN,			/**<Printer length*/
	HARDWARE_GET_POS_RUNTIME,		/**<Runing time*/
	HARDWARE_GET_KEY_COUNT			/**<Get key pressing times*/
};

typedef struct tagRecFile
{
	unsigned int unMaxOneRecLen;	/**<Maximum length of one record*/
	unsigned int unIndex1Start;		/**<Start position of index1*/
	unsigned int unIndex1Len;		/**<Length of index1*/
	unsigned int unIndex2Start; 	/**<Start position of index1*/
	unsigned int unIndex2Len;		/**<Length of index2*/
	char cIsIndex;					/**<Need to create index, FILE_NOCREATEINDEX or FILE_CREATEINDEX*/
	char szFileName[128];			/**<Record name,Max to 128*/
	char s[2];
}STRECFILE;

typedef struct
{
	int nLen;								/**<Length*/
	int nLevel;								/**<Correction level*/
	int nMask;								/**<Mask number*/
	int nVersion;							/**<Version*/
	char szInputData[MAX_INPUTDATASIZE];	/**<Data need to code*/
} DataInfo;

typedef struct
{
	int xsize;
	int ysize;
	char bmpbuff[MAX_BMPBUFFSIZE];
}BmpInfo;


/**
 *@brief 		Delay (Unit: 1ms (depend on unix kernel, most unix may more than 10ms)
 *@param[in]   unDelayTime Delay time
 *@return
 *@li \ref APP_SUCC "APP_SUCC"  				Success
 *@li	 \ref APP_FAIL "APP_FAIL"
 *@author tom chen
 */
int PubSysMsDelay(uint unDelayTime);

/**
 *@brief 		Delay (Unit: 1ms (depend on unix kernel, most unix may more than 0.1s)
 *@param[in]   unDelayTime Delay time
 *@return
 *@li \ref APP_SUCC "APP_SUCC"  				Success
 *@li	 \ref APP_FAIL "APP_FAIL"
 *@author tom chen
 */
int PubSysDelay(uint unDelayTime);


/** @addtogroup Utilities
* @{
*/

/**
* @brief Add two unsigned int number(no more than 12 bytes), the result can not be more than 12 bytes.
* @param [in] pszDigStr1 Pointer to number1
* @param [in] pszDigStr2 Pointer to number2
* @param [out] pszResult Pointer to result
* @param [out] pnResultLen  Pointer to the length of result
* @return
* @li APP_FAIL Fail
* @li APP_APP_SUCC Success
* @date 2012-06-19
*/
int PubAddDigitStr(const char *pszDigStr1, const char *pszDigStr2, char *pszResult, int *pnResultLen);

/**
* @brief A 6 bytes string plus 1(when pcStrNum == "999999", result is 000001)
* @param [in] pcStrNum  string (6bytes)
* @param [out] pcStrNum result after plus 1
* @return
* @li APP_FAIL Fail
* @li APP_APP_SUCC Success
* @date 2012-06-19
*/
int PubIncNum(char *pcStrNum);

/**
* @brief Ascii string1 plus Ascii string2
* @param [in] pszAsc1	Poniter to Ascii string1
* @param [in] pszAsc2	Poniter to Ascii string2
* @param [out] pszSum 	Pointer to the sum
* @return
* @li void
*/
void PubAscAddAsc(uchar *pszAsc1, uchar *pszAsc2, uchar *pszSum);

/**
* @brief Ascii string1 substract Ascii string2
* @param [in] pszAsc1	Poniter to Ascii string 1
* @param [in] pszAsc2	Poniter to Ascii string 2
* @param [out] pszResult If the result is minus, show "-XXXX"
* @return
* @li void
*/
void PubAscSubAsc(uchar *pszAsc1, uchar *pszAsc2, uchar *pszResult);

/**
* @brief Ascii string1 multiply Ascii string2
* @param [in] pszAsc1	Poniter to Ascii string 1
* @param [in] pszAsc2	Poniter to Ascii string 2
* @param [out] pszProduct Poniter to the product
* @return
* @li void
*/
void PubAscMulAsc(uchar *pszAsc1, uchar *pszAsc2, uchar *pszProduct);

/**
* @brief Ascii string1 devide Ascii string2
* @param [in] pszAsc1	 Poniter to Ascii string 1
* @param [in] pszAsc2	 Poniter to Ascii string 2
* @param [out] pszResult Pointer to the result
* @return
* @li void
*/
void PubAscDivAsc(uchar *pszAsc1, uchar *pszAsc2, uchar *pszResult);

/**
* @brief Remove '.' character from amount str, e.g. '12.34' to '1234'
* @param [in] pszSource    Source string.
* @param [out] pszTarget   Result
* @param [out] pnTargetLen Length of result.
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubFmtAmtStr(const char *pszSource, char *pszTarget, int *pnTargetLen);

/**
* @brief Convert Ascii string to Hex data, if the length of string isn't even number, it needs left padding 0, or right padding 0.
* @param [in] pszAsciiBuf Pointer to the buffer
* @param [in] nLen        Length of buffer.
* @param [in] cType       0-left padding  1-right padding
* @param [out] pszBcdBuf  Pointer to the buffer outputed
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubAscToHex(const unsigned char *pszAsciiBuf, int nLen, char cType, unsigned char *pszBcdBuf);

/**
* @brief Convert Hex to Ascii
* @param [in] pszBcdBuf    Pointer to the buffer
* @param [in] nLen         Length of buffer.
* @param [in] cType        Alignment:0-left 1-right
* @param [out] pszAsciiBuf
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubHexToAsc(const unsigned char *pszBcdBuf, int nLen, char cType, unsigned char *pszAsciiBuf);

/**
* @brief Convert an integer to 4 bytes HEX
* @param [in] nNum   input interger number
* @param [out] pszBuf  output buffer
* @return
* @li void
* @date 2012-06-19
*/
void PubIntToC4(unsigned char *pszBuf, unsigned int nNum);

/**
* @brief Convert 4 bytes HEX to an integer
* @param [in] pszBuf  input string buffer.
* @param [out] nNum   output number
* @return
* @li void
* @date 2012-06-19
*/
void PubC4ToInt(unsigned int *nNum, unsigned char *pszBuf);

/**
* @brief Convert an integer to 2 bytes HEX
* @param [in] nNum    input interger number
* @param [out] pszBuf  output buffer
* @return
* @li void
* @date 2012-06-19
*/
void PubIntToC2(unsigned char *pszBuf, unsigned int nNum);

/**
* @fn PubCnToInt
* @brief Convert n bytes HEX to an integer
* @param [in]		pszBuf	[Input string buffer.]
* @param [out]		nNum    	[Output number]
* @param [in]		nLen		[String length]
* @return 
* @li void
* @date 2020-03-24
*/
void PubCnToInt (unsigned int* nNum, unsigned char* pszBuf, int nLen);

/**
* @brief Convert 2 bytes HEX to an integer
* @param [in] pszBuf  input string buffer.
* @param [out] nNum   output number
* @return
* @li void
* @date 2012-06-19
*/
void PubC2ToInt(unsigned int *nNum, unsigned char *pszBuf);

/**
* @brief Convert an integer(0-99) to BCD string(1byte)
* @param [in] nNum  the integer(0-99)
* @param [out] ch   output character
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubByteToBcd(int nNum, char *ch);

/**
* @brief Convert BCD string(1byte) to an integer(0-99)
* @param [in] ch    input character
* @param [out] pnNum  output interger
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubBcdToByte(char ch, int *pnNum);

/**
* @brief Convert an integer(0-9999) to BCD string(2bytes)
* @param [in] nNum      input interger(0-9999)
* @param [out] psBcd    output bcd string
* @param [out] pnBcdLen  output the length of bcd string
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubIntToBcd(char *psBcd, int *pnBcdLen, int nNum);

/**
* @brief  Convert BCD string(2bytes) to an integer(0-9999)
* @param [in] psBcd  input bcd string
* @param [out] nNum  output interger(0-9999)
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubBcdToInt(const char *psBcd, int *nNum);

/**
* @brief Calculate LRC
* @param [in] psBuf  The buffer used to calculate
* @param [in] nLen   length of buffer
* @param [out] chLRC LRC
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubCalcLRC(const char *psBuf, int nLen, char *chLRC);

/**
* @brief Delete left spaces in the string
* @param [in] pszSrc  Source string
* @param [out] pszSrc String after deleting
* @return
* @li void
* @date 2012-06-19
*/
void PubLeftTrim(char *pszSrc);

/**
* @brief Delete right spaces in the string
* @param [in] pszSrc  Source string
* @param [out] pszSrc String after deleting
* @return
* @li void
* @date 2012-06-19
*/
void PubRightTrim(char *pszSrc);

/**
* @brief Delete both side spaces in the string
* @param [in] pszSrc  Source string
* @param [out] pszSrc String after deleting
* @return
* @li void
* @date 2012-06-19
*/
void PubAllTrim(char *pszSrc);

/**
* @brief Delete a speficied charactor from a string
* @param [in] pszString		Source string
* @param [in] ucSymbol	  the speficied character
* @param [out] pszString	Output string
* @return
* li
*/
void PubDelSymbolFromStr(uchar *pszString, uchar ucSymbol);

/**
* @brief Add a charactor to a string, make it to a specified length
* @param [in] pszString  Source string
* @param [in] nLen       The length after adding
* @param [in] ch         The specified charactor
* @param [in] nOption    Operation type--EM_ADDCH_MODE
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubAddSymbolToStr(char *pszString, int nLen, char ch, int nOption);

/**
* @brief Delete a specified charater at left of a string
* @param [in] pszSrc 	Source string
* @param [in] ucRemoveChar	a character need to delete
* @param [out] pszSrc		result
* @return
* li void
*/
void PubLeftTrimChar(uchar *pszSrc, uchar ucRemoveChar);

/**
* @brief Delete a specified charater at right of a string
* @param [in] pszSrc	Source string
* @param [in] ucRemoveChar	a character need to delete
* @param [out] pszSrc	result
* @return
* li void
*/
void PubRightTrimChar(uchar *pszSrc, uchar ucRemoveChar);

/**
* @brief Check if the string is digit
* @param [in] pszString    input string
* @return
* @li APP_FAIL NO
* @li APP_SUCC YES
* @date 2012-06-19
*/
int PubIsDigitStr(const char *pszString);

/**
* @brief Check if the charactor is digit
* @param [in] ch  input charactor
* @return
* @li APP_FAIL NO
* @li APP_SUCC YES
* @date 2012-06-19
*/
int PubIsDigitChar(char ch);

/**
* @brief Check if the format of date is valid
* @param [in] pDate Date format  YYYYMMDD
* @return
* @li APP_FAIL Illegality
* @li APP_SUCC Valid
* @date 2012-06-19
*/
int PubIsValidDate(const char *pDate);

/**
* @brief	Format the amount string, such as HKD$****.**, RMB****.**
* @param [in] pszPrefix 	Prefix Such as "HKD"in "HKD 200.00"
* @param [in] pszIn			The string of integer part.
* @param [in] ucRadPt		The string of decimal part, must be less than 4
* @param [out] pszOut		Output string
* @return
* li coverted number.
*/
void PubConvAmount(uchar *pszPrefix, uchar *pszIn, uchar ucRadPt, uchar *pszOut);

/**
* @brief Get current date time
* @param [out] pDatetime Format is YYYYMMDDHHMMSS, length >14
* @return
* @li void
* @date 2012-06-19
*/
void PubGetCurrentDatetime(char *pDatetime);

/**
* @brief Set date time in POS
* @param [in] pszDate date
* @param [in] pszDateFormat Date format like "MMDD" or "YYYYMMDD"
* @param [in] pszTime time
* @return
* @li APP_SUCC Success
* @li APP_FAIL Fail
*/
int PubSetPosDateTime(const char *pszDate, const char *pszDateFormat, const char *pszTime);

/**
* @brief The total number of day in a certain month.
* @param [in] nYear
* @param [in] nMon
* @param [out] pnDays  Total days in a certain month.
* @return
* @li APP_FAIL
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubMonthDays(int nYear, int nMon, int *pnDays);

/**
* @brief Check if the year is leap year
* @param [in] nYear input year
* @return
* @li APP_FAIL Not leap year
* @li APP_SUCC Leap year
* @date 2012-06-19
*/
int PubIsLeapYear(int nYear);

/**
* @brief get timer count of timeout
* @param [in]  unMsTime  Timeout (Unit:ms)
* @return
* @li  Time count
* @author Liug
* @date 2012-10-24
*/
uint PubGetOverTimer(uint unMsTime);

/**
* @brief Check if timer is timeout.
* @param [in]  unOverTime  Timer count (unit:ms, usually get from PubGetOverTimer returning)
* @return
* @li APP_APP_SUCC Timeout
* @li APP_APP_FAIL No timeout
* @author Liug
* @date 2012-10-24
*/
int PubTimerIsOver(uint unOverTime);

/**
* @brief Get tool lib version
* @param [out] pszVer Greater than 12bytes
* @return
* @li void
* @date 2012-06-19
*/
void PubGetToolsVer(char *pszVer);

/**
* @brief Get the position of a charactor in a string
* @param [in] pszData     ---string
* @param [in] cSubChar    ---charactor
* @return
* @li APP_FAIL  ---inexistence
* @li >=0   ---Position
* @author Liug
* @date 2012-7-23
*/
int PubPosChar(const char *pszData,char cSubChar);

/**
* @brief Get the position of a string in another string
* @param [in] pszData     ---Source string
* @param [in] pszSub      ---String need to search
* @return
* @li APP_FAIL  ---inexistence
* @li >=0   ---Position
* @author Liug
* @date 2012-7-23
*/
int PubPosStr(const char *pszData,const char *pszSub);



/**
* @brief Get data from FRAM
* @param [out] psBuf   --- Data
* @param [in]  nStart      --- Start location
* @param [in]  nLen        --- Data len
* @return
* @li APP_SUCC Success
* @li APP_FAIL Fail
* @author Liug
* @date 2012-10-24
*/
int PubGetVar(char *psBuf, const int nStart, const int nLen);

/**
* @brief Save data to FRAM
* @param [in]  psSaveBuf   --- Data
* @param [in]  nStart      --- Start location
* @param [in]  nLen        --- Data len
* @return
* @li APP_SUCC Success
* @li APP_FAIL Fail
* @author Liug
* @date 2012-10-24
*/
int PubSaveVar(const char *psSaveBuf, const int nStart, const int nLen);

/**
* @brief Beep
* @param [in] nTimes	Times
* @return
* @li void
*/
void PubBeep(int nTimes);

/**
* @brief Shut down
* @return
* @li APP_SUCC	Success
* @li APP_FAIL   Fail
*/
int PubShutDown(void);


/**
* @brief Get hardware information in the pos machine
* @param [in] emFlag	EM_HARDWARE_SUPPORT
* @param [out] pszBuf	Hardware message
* @return
* @li APP_FAIL		Nonsupport
* @li APP_APP_SUCC	Support
* @date 2012-11-13
*/
int PubGetHardwareSuppot(int emFlag, char *pszBuf);

/**
* @brief Get pos information
* @param [in] emFlag	EM_HARDWARE_GET
* @param [out] pszBuf	hardware info
* @param [in] nBuffLen pszBuf maxLen
* @return
* @li APP_FAIL		Fail
* @li APP_APP_SUCC	Success
*/
int PubGetPosInfo(int emFlag, char *pszBuf, int nBuffLen);

/**
* @brief Generate bitmap within QR code encryption.
* @param [in] stDataInfo
				nLen		String length
				nLevel		level(defult 1, from 1-3)
				nMask		Mask no(defult 1, from 0-7)
				nVersion	Version(defult 0, from 0-40)
				szInputData	String
* @param [out] BmpBuff
				xsize		Picture width(pixel)
				ysize		Picture herght(pixel)
				bmpbuff		Binary buffer
* @return
* @li APP_FAIL		Fail
* @li APP_APP_SUCC	Success
*/
int PubEncodeDataAndGenerateBmp(DataInfo* stDataInfo, BmpInfo* BmpBuff);

/**
* @brief Convert ASCII to UTF8
* @param [in] stDataInfo
* @param [out] stDataInfo
* @return
* @li void
*/
void PubAsciiToUtf8(DataInfo* stDataInfo);


/** @}*/ // End of Utilities

/** @addtogroup File
* @{
*/

/**
* @brief Open file
* @param [in] pszFileName   File name
* @param [in] pszMode       Open mode only "w"(read & write), "r"(read)
* @param [out] pnFileHandle File handle
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubOpenFile(const char *pszFileName, const char *pszMode, int *pnFileHandle);

/**
* @brief Close an opened file
* @param [in] pnFileHandle File handle
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubCloseFile(int *pnFileHandle);

/**
* @brief Delete file
* @param [in] pszFileName File name
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubDelFile(const char *pszFileName);

/**
* @brief Read data from an opened file
* @param [in] nHandle   File handle
* @param [in] unLength  Length
* @param [out] psBuffer Data in file
* @return
* @li APP_FAIL Fail
* @li The real length of reading
*/
int PubReadFile(int nHandle, char *psBuffer, uint unLength);

/**
* @brief Write data to an opened file
* @param [in] nHandle   File handle
* @param [in] psBuffer  Data need to write
* @param [in] unLength	Data length
* @return
* @li APP_FAIL Fail
* @li The real length of writing
*/
int PubWriteFile(int nHandle, const char *psBuffer, uint unLength);

/**
* @brief Create record file and index files if necessary
* @param [in] pstRecFile The structure of file.
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubCreatRecFile(const STRECFILE *pstRecFile);

/**
* @brief Delete record file created by 'PubCreatRecFile' including other extra index files.
* @param [in] pszFileName File name
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubDeReclFile(const char *pszFileName);

/**
* @brief Get record number of record file created by 'PubCreatRecFile'.
* @param [in] pszFileName File name
* @param [out] pnRecSum  Number of record
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubGetRecSum(const char *pszFileName, int *pnRecSum);

/**
* @brief Add a record to a file created by 'PubCreatRecFile'
* @param [in] pszFileName File name
* @param [in] pszRec Record buffer
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubAddRec(const char *pszFileName, const char *pszRec);

/**
* @brief Read a specified record from a file created by 'PubCreatRecFile'
* @param [in] pszFileName File name
* @param [in] nRecNo      Record number
* @param [out] psRec      Record buffer
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubReadOneRec(const char *pszFileName, const int nRecNo, char *psRec);

/**
* @brief Update a specified record to a file created by 'PubCreatRecFile'
* @param [in] pszFileName File name
* @param [in] nRecNo      Index
* @param [in] psRec       Record buffer
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubUpdateRec(const char *pszFileName, const int nRecNo, const char *psRec);

/**
* @brief Find a record from a file created by 'PubCreatRecFile' with 'FILE_CREATEINDEX' mode.
* @details If 'psIndexStr1' and 'psIndexStr2' are both assigned, needs to meet these two condition, otherwise one of them is enough.
* @param [in] pszFileName File name
* @param [in] psIndexStr1 Index string1
* @param [in] psIndexStr2 Index string2
* @param [in] nBeginRecNo From which record to start finding.
* @param [out] psRec Record buffer
* @param [out] pnRecNo Record number
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubFindRec(const char *pszFileName, const char *psIndexStr1, const char *psIndexStr2, const int nBeginRecNo, char *psRec, int *pnRecNo);

/**
* @brief Delete a record from a file created by 'PubCreatRecFile' with 'FILE_CREATEINDEX' mode.
* @details If 'psIndexStr1' and 'psIndexStr2' are both assigned, needs to meet these two condition, otherwise one of them is enough.
* @param [in] pszFileName File name
* @param [in] psIndexStr1 Index string1
* @param [in] psIndexStr2 Index string2
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubDelRec(const char *pszFileName, const char *psIndexStr1, const char *psIndexStr2);

/**
* @brief Read a specified record from an opened file created by 'PubCreatRecFile'
* @param [in] nFileHandle File handle
* @param [in] nRecNo      Index
* @param [out] psRec      Record buffer
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubReadRec(const int nFileHandle, const int nRecNo, char *psRec);

/**
* @brief Update a specified record to an opened file created by 'PubCreatRecFile'
* @param [in] nFileHandle File handle
* @param [in] nRecNo      Index
* @param [in] psRec       Record buffer
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubUpdateRecByHandle(int nFileHandle, const int nRecNo, const char *psRec);

/**
* @brief Get a specified string value from INI file which must be ended by a new line
* @param [in] nHandle     File handle
* @param [in] psSeg       Pointer to segment.
* @param [in] psItem      Pointer to item.
* @param [in] nMaxStrLen  Max length of value
* @param [out] psValueStr Value
* @param [out] pnValueLen Length of value
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubGetINIItemStr(const int nHandle, const char *psSeg, const char *psItem, const int nMaxStrLen, char *psValueStr, int *pnValueLen);

/**
* @brief Get a specified integer value from INI file which must be ended by a new line
* @param [in] nHandle  File handle
* @param [in] psSeg    Pointer to segment.
* @param [in] psItem   Pointer to item.
* @param [out] pnValue Integer vaule
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubGetINIItemInt(const int nHandle,const char *psSeg, const char *psItem, int *pnValue);

/** @}*/ // End of File

/** @addtogroup ISO8583
* @{
*/

#ifndef BOOL
typedef unsigned char BOOL;
#endif

#define MAX_FIELD_LEN 2048		/**<Max length of every field*/
#define MAX_MSG_LEN 8192		/**<Max length of a ISO message */

typedef enum
{	/**<Align of field */
    ALIGN_TYPE_LEFT   = 0,
    ALIGN_TYPE_RIGHT  = 1
}EM_ALIGN_TYPE;

typedef enum
{	/**<Type of length */
    LENGTH_TYPE_FIX    = 0,
    LENGTH_TYPE_LLVAR  = 1,
    LENGTH_TYPE_LLLVAR = 2
} EM_LENGTH_TYPE;

typedef struct STISO_FIELD STISO_FIELD;
typedef struct STISO_FIELD_DEF STISO_FIELD_DEF;

/**
* @brief Pack the ISO8583, return the data and length
* @param [in] pthis			ISO format structure
* @param [in] pfieldbuf		Data pointer need to pack
* @param [in] fieldlen		Data length
* @param [out] poutdata		Output data
* @return
* @li	Pack
*/
typedef int (*FIELD_PACKAGER_PACK)(STISO_FIELD_DEF *pthis, const unsigned char *pfieldbuf, int fieldlen, unsigned char *poutdata);

/**
* @brief Unpack the data to ISO format
* @param [in] pthis			ISO format structure
* @param [in] pindata		Source data
* @param [out] pfieldbuf	Output data after unpacking
* @param [out] pfieldlen	length of output data
* @return
* @li
*/
typedef int (*FIELD_PACKAGER_UNPACK)(STISO_FIELD_DEF *pthis, unsigned char *pfieldbuf, int *pfieldlen, unsigned char *pindata);

/**
* @brief Get length value of a 8583 field
* @param [in] pthis			ISO format structure
* @param [in] len			Field length
* @param [out] poutdata		Output data
* @param [out] pcount		Length value (Fix is 0, LLVAR is 1, LLLVAR is 2)
* @return
* @li	void
*/
typedef void (*FIELD_LENGTH_SET)(STISO_FIELD_DEF *pthis, int len, unsigned char *poutdata, int *pcount);

/**
* @brief Get length of a 8583 field
* @param [in] pthis			ISO format structure
* @param [in] pindata		Field data
* @param [out] plen			Length
* @param [out] pcount		Length value (Fix is 0, LLVAR is 1, LLLVAR is 2)
* @return
* @li	void
*/
typedef void (*FIELD_LENGTH_GET)(STISO_FIELD_DEF *pthis, unsigned char *pindata, int *plen, int *pcount);

struct STISO_FIELD_LENGTH_CALC
{	/**< ISO 8583 field length precess*/
    FIELD_LENGTH_SET	pSet;		/**< Set */
    FIELD_LENGTH_GET	pGet;		/**< Get */
};

struct STISO_FIELD
{	/**< ISO 8583 Field */
    int    nLen;
    int    nOff;					/**< Pointer to iso->databuf, not contain length */
};

struct STISO_FIELD_DEF
{	/**< ISO 8583 Data field define */
    int                     nLen;			/**< Max field length */
    EM_LENGTH_TYPE		    lentype;		/**< Length type: 1--Type LLVAR 2--Type LLLVAR */
    EM_ALIGN_TYPE           align;			/**< Align type */
    char                    cPadchar;		/**< Align charactor */
    FIELD_PACKAGER_PACK		pPack;			/**< Pack function */
    FIELD_PACKAGER_UNPACK	pUnpack;		/**< Unpack function */
};


typedef struct STISODataStru
{	/**< ISO 8583 Msg define */
    STISO_FIELD		fld[256];
    char			sBitmap[256];
    unsigned char 	szDatabuf[MAX_MSG_LEN + MAX_FIELD_LEN];
    int        		nUsed;
} STISODataStru;

/**
* @brief Calculate the length of message(Ascii) which format is ISO 8583
* @param [in] pthis
* @param [in] len
* @param [out] poutdata
* @param [out] pcount
* @return
* @li void
*/
void PubFLALengthSet(STISO_FIELD_DEF *pthis, int len, unsigned char *poutdata, int *pcount);

/**
* @brief Calculate the length of field in message(Ascii) which format is ISO 8583.
* @param [in] pthis
* @param [in] pindata
* @param [out] plen
* @param [out] pcount
* @return
* @li void
*/
void PubFLALengthGet(STISO_FIELD_DEF *pthis, unsigned char *pindata, int *plen, int *pcount);

/**
* @brief Calculate the length of message(BCD) which format is ISO 8583
* @param [in] pthis
* @param [in] len
* @param [out] poutdata
* @param [out] pcount
* @return
* @li void
*/
void PubFLBLengthSet(STISO_FIELD_DEF *pthis, int len, unsigned char *poutdata, int *pcount);

/**
* @brief Calculate the length of field in message(BCD) which format is ISO 8583.
* @param [in] pthis
* @param [in] pindata
* @param [out] plen
* @param [out] pcount
* @return
* @li void
*/
void PubFLBLengthGet(STISO_FIELD_DEF *pthis, unsigned char *pindata, int *plen, int *pcount);

/**
* @brief Calculate the length of message(Hex) which format is ISO 8583
* @param [in] pthis
* @param [in] len
* @param [out] poutdata
* @param [out] pcount
* @return
* @li void
*/
void PubFLHLengthSet(STISO_FIELD_DEF *pthis, int len, unsigned char *poutdata, int *pcount);

/**
* @brief Calculate the length of field in message(Hex) which format is ISO 8583.
* @param [in] pthis
* @param [in] pindata
* @param [out] plen
* @param [out] pcount
* @return
* @li void
*/
void PubFLHLengthGet(STISO_FIELD_DEF *pthis, unsigned char *pindata, int *plen, int *pcount);

/**
* @brief Pack data in ISO8583(Msg ID is Ascii)
* @param [in] pthis
* @param [in] pfieldbuf
* @param [in] fieldlen
* @param [out] poutdata
* @return
* @li
*/
int PubFPAMsgIDPack(STISO_FIELD_DEF *pthis, const unsigned char *pfieldbuf, int fieldlen, unsigned char *poutdata);

/**
* @brief Unpack data in ISO8583(Msg ID is Ascii)
* @param [in] pthis
* @param [in] pindata
* @param [out] pfieldbuf
* @param [out] pfieldlen
* @return
* @li
*/
int PubFPAMsgIDUnpack(STISO_FIELD_DEF *pthis, unsigned char *pfieldbuf, int  *pfieldlen, unsigned char *pindata);

/**
* @brief Pack data in ISO8583(Msg ID is BCD)
* @param [in] pthis
* @param [in] pfieldbuf
* @param [in] fieldlen
* @param [out] poutdata
* @return
* @li
*/
int PubFPBMsgIDpack(STISO_FIELD_DEF *pthis, const unsigned char *pfieldbuf, int fieldlen, unsigned char *poutdata);

/**
* @brief Unpack data in ISO8583(Msg ID is BCD)
* @param [in] pthis
* @param [in] pindata
* @param [out] pfieldbuf
* @param [out] pfieldlen
* @return
* @li
*/
int PubFPBMsgIDUnpack(STISO_FIELD_DEF *pthis, unsigned char *pfieldbuf, int *pfieldlen, unsigned char *pindata);

/**
* @brief Pack data in ISO8583(Bitmap in Ascii)
* @param [in] pthis
* @param [in] pfieldbuf
* @param [in] fieldlen
* @param [out] poutdata
* @return
* @li
*/
int PubFPABitmapPack(STISO_FIELD_DEF *pthis, const unsigned char *pfieldbuf, int fieldlen, unsigned char *poutdata);

/**
* @brief Unpack data in ISO8583(Bitmap in Ascii)
* @param [in] pthis
* @param [in] pindata
* @param [out] pfieldbuf
* @param [out] pfieldlen
* @return
* @li
*/
int PubFPABitmapUnPack(STISO_FIELD_DEF *pthis, unsigned char *pfieldbuf, int *pfieldlen, unsigned char *pindata);

/**
* @brief Pack data in ISO8583(Bitmap in BCD)
* @param [in] pthis
* @param [in] pfieldbuf
* @param [in] fieldlen
* @param [out] poutdata
* @return
* @li
*/
int PubFPBBitmapPack(STISO_FIELD_DEF *pthis, const unsigned char *pfieldbuf, int fieldlen, unsigned char *poutdata);

/**
* @brief Unpack data in ISO8583(Bitmap in BCD)
* @param [in] pthis
* @param [in] pindata
* @param [out] pfieldbuf
* @param [out] pfieldlen
* @return
* @li
*/
int PubFPBBitmapUnPack(STISO_FIELD_DEF *pthis, unsigned char *pfieldbuf, int *pfieldlen, unsigned char *pindata);

/**
* @brief Pack data in ISO8583(string in ascii)
* @param [in] pthis
* @param [in] pfieldbuf
* @param [in] fieldlen
* @param [out] poutdata
* @return
* @li
*/
int PubFPACharPack(STISO_FIELD_DEF *pthis, const unsigned char *pfieldbuf, int fieldlen, unsigned char *poutdata);

/**
* @brief Unpack data in ISO8583(string in ascii)
* @param [in] pthis
* @param [in] pindata
* @param [out] pfieldbuf
* @param [out] pfieldlen
* @return
* @li
*/
int PubFPACharUnPack(STISO_FIELD_DEF *pthis, unsigned char *pfieldbuf, int *pfieldlen, unsigned char *pindata);

/**
* @brief Pack data in ISO8583(string in BCD)
* @param [in] pthis
* @param [in] pfieldbuf
* @param [in] fieldlen
* @param [out] poutdata
* @return
* @li
*/
int PubFPBCharPack(STISO_FIELD_DEF *pthis, const unsigned char *pfieldbuf, int fieldlen, unsigned char *poutdata);

/**
* @brief Unpack data in ISO8583(string in BCD)
* @param [in] pthis
* @param [in] pindata
* @param [out] pfieldbuf
* @param [out] pfieldlen
* @return
* @li
*/
int PubFPBCharUnPack(STISO_FIELD_DEF *pthis, unsigned char *pfieldbuf, int *pfieldlen, unsigned char *pindata);

/**
* @brief Pack data in ISO8583(digit string in Ascii)
* @param [in] pthis
* @param [in] pfieldbuf
* @param [in] fieldlen
* @param [out] poutdata
* @return
* @li
*/
int PubFPADigitPack(STISO_FIELD_DEF *pthis, const unsigned char *pfieldbuf, int fieldlen, unsigned char *poutdata);

/**
* @brief Unpack data in ISO8583(digit string in Ascii)
* @param [in] pthis
* @param [in] pindata
* @param [out] pfieldbuf
* @param [out] pfieldlen
* @return
* @li
*/
int PubFPADigitUnPack(STISO_FIELD_DEF *pthis, unsigned char *pfieldbuf, int *pfieldlen, unsigned char *pindata);

/**
* @brief Pack data in ISO8583(digit string in BCD)
* @param [in] pthis
* @param [in] pfieldbuf
* @param [in] fieldlen
* @param [out] poutdata
* @return
* @li the length of data packed.
*/
int PubFPBDigitPack(STISO_FIELD_DEF *pthis, const unsigned char *pfieldbuf, int fieldlen, unsigned char *poutdata);

/**
* @brief Unpack data in ISO8583(digit string in BCD)
* @param [in] pthis
* @param [in] pindata
* @param [out] pfieldbuf
* @param [out] pfieldlen
* @return
* @li
*/
int PubFPBDigitUnPack(STISO_FIELD_DEF *pthis, unsigned char *pfieldbuf, int *pfieldlen, unsigned char *pindata);

/**
* @brief Pack data in ISO8583(Amount string in BCD)
* @param [in] pthis
* @param [in] pfieldbuf
* @param [in] fieldlen
* @param [out] poutdata
* @return
* @li the length of data packed.
*/
int PubFPBAmountPack(STISO_FIELD_DEF *pthis, const unsigned char *pfieldbuf, int fieldlen, unsigned char *poutdata);

/**
* @brief Unpack data in ISO8583(Amount string in BCD)
* @param [in] pthis
* @param [in] pindata
* @param [out] pfieldbuf
* @param [out] pfieldlen
* @return
* @li
*/
int PubFPBAmountUnPack(STISO_FIELD_DEF *pthis, unsigned char *pfieldbuf, int *pfieldlen, unsigned char *pindata);

/**
* @brief Pack data in ISO8583(Binary string in BCD)
* @param [in] pthis
* @param [in] pfieldbuf
* @param [in] fieldlen
* @param [out] poutdata
* @return
* @li the length of data packed.
*/
int PubFPBBinaryPack(STISO_FIELD_DEF *pthis, const unsigned char *pfieldbuf, int fieldlen, unsigned char *poutdata);

/**
* @brief Unpack data in ISO8583(Binary string in BCD)
* @param [in] pthis
* @param [in] pindata
* @param [out] pfieldbuf
* @param [out] pfieldlen
* @return
* @li the used length of pindata when unpacking.
*/
int PubFPBBinaryUnPack(STISO_FIELD_DEF *pthis, unsigned char *pfieldbuf, int *pfieldlen, unsigned char *pindata);

/**
* @brief Convert string to Iso structure.
* @param [in] pcSourceStr
* @param [in] nStrLen
* @param [out] pTargetIsoStru
* @param [out] pnPosition
* @return
* @li APP_FAIL
* @li APP_APP_SUCC
*/
int PubStrToIso(char *pcSourceStr, int nStrLen, STISODataStru *pTargetIsoStru, int *pnPosition);

/**
* @brief Convert Iso structure to string
* @param [in] SourceIsoStru
* @param [in] pnLen
* @param [out] pcTargetStr	BCD
* @param [out] pnLen
* @return
* @li APP_FAIL		Fail
* @li APP_APP_SUCC	Success
*/
int PubIsoToStr(char *pcTargetStr, STISODataStru *SourceIsoStru, int *pnLen);

/**
* @brief Get appointed member in ISO structure, then put it in szTargetStr.
* @param [in] pSourceIsoStru
* @param [in] nPosition
* @param [in] pnLen
* @param [out] pcTargetStr
* @param [out] pnLen
* @return
* @li APP_FAIL		Fail
* @li APP_APP_SUCC	Success
*/
int PubGetBit(STISODataStru *pSourceIsoStru, int nPosition, char *pcTargetStr, int *pnLen);

/**
* @brief Put szTargetStr in appointed member of ISO structure.
* @param [in] nPosition
* @param [in] pcSourceStr
* @param [in] nStrLen
* @param [out] pTargetIsoStru
* @return
* @li APP_FAIL		Fail, TargetIsostru or pcSourceStr is NULL, or appointed member is invalid, or call AscToHex error, or the length of ISO structure exceeds MAXISOLEN
* @li APP_APP_SUCC	Success.
*/
int PubSetBit(STISODataStru *pTargetIsoStru, int nPosition, char *pcSourceStr, int nStrLen);

/**
* @brief Clear all the member of structure STISODataStru.
* @param [in] pIsoStru
* @param [out] pIsoStru
* @return
* @li APP_FAIL		Fail
* @li APP_APP_SUCC	Success
*/
int PubDelAllBit(STISODataStru *pIsoStru);

/**
* @brief Clear a specified field in structure STISODataStru.
* @param [in] pIsoStru
* @param [in] nPosition
* @param [out] pIsoStru		DelOneBit
* @return
* @li APP_FAIL		Fail
* @li APP_APP_SUCC	Success
*/
int PubDelOneBit(STISODataStru *pIsoStru, int nPosition);

/**
* @brief Install initial parameter in structure ISO8583.
* @param [in] nPosition		Field number
* @param [in] nMaxLen		Max length
* @param [in] LenType		Length type
* @param [in] AlignType		Align type
* @param [in] chPadChar		Align charactor
* @param [in] PackFunc		Pack function
* @param [in] UnpackFunc	Unpack function
* @return
* @li APP_FAIL		Fail
* @li APP_APP_SUCC	Success
*/
int PubSetDefine8583(int nPosition, int nMaxLen, EM_LENGTH_TYPE	LenType, EM_ALIGN_TYPE AlignType, char chPadChar, FIELD_PACKAGER_PACK PackFunc, FIELD_PACKAGER_UNPACK UnpackFunc);

/**
* @brief Import structure ISO8583
* @param [in] pNewDef
* @return
* @li APP_FAIL		Fail
* @li APP_APP_SUCC	Success
*/
int PubImport8583(const struct STISO_FIELD_DEF *pNewDef);

/**
* @brief Display ISO8583 data according to field.
* @param [in] pIsoStru
* @return
* @li void
*/
void PubFormatDisp8583(STISODataStru *pIsoStru);

/**
* @brief Input calback function, and calculate length of field.
* @param [in] pNewCalc
* @return
* @li APP_FAIL		Fail
* @li APP_APP_SUCC	Success
*/
int PubSetFieldLengthCalc(const struct STISO_FIELD_LENGTH_CALC *pNewCalc);

/**
* @brief Copy a iso data to another iso data.
* @param [in] pSourceIsoStru
* @param [out] pTargetIsoStru
* @return
* @li APP_FAIL		Fail, pSourceIsoStru or pTargetIsoStru is NULL
* @li APP_APP_SUCC	Success
*/
int PubCopyIso(STISODataStru *pSourceIsoStru, STISODataStru *pTargetIsoStru);

/** @}*/ // End of ISO8583

/** @addtogroup Debug
* @{
*/

/**
* @brief Export debug file to RS232 Port, when debug mode is file mode,
* @details Once is has been invoked, log will be exported and then deleted. Even if you don't 'PubExportDebugFile' is not invoked, the log will disappear after rebooting.
* @return
* @li void
* @author Liug
* @date 2012-10-24
*/
void PubExportDebugFile(void);

/**
* @brief Output debug data(hex format)
* @param [in] pszTitle  --- Title
* @param [in] pData     --- Data
* @param [in] nLen      --- Length
* @return
* @li void
* @author Liug
* @date 2012-10-24
*/
void PubDebugData(const char *pszTitle,const void* pData,int nLen,...);

/**
* @brief Output debug data(ascii format)
* @param [in]  lpszFormat   --- Data
* @return
* @li void
* @author Liug
* @date 2012-10-24
*/
void PubDebug(char *lpszFormat, ...);

/**
* @brief Output debug data
* @param [in]  pszBuf   --- Data
* @param [in]  nBufLen  --- length
* @return
* @li void
* @author lingdz
* @date 2019-11-13
*/
void PubBufToAux(const char *pszBuf, const int nBufLen);

/**
* @brief Get debug mode (port or file)
* @return
* @li DEBUG_NONE: Debug closed
* @li DEBUG_PORT: Port mode (including serial and USB)
* @li DEBUG_FILE: Debug mode: Debug file
*/
int PubGetDebugMode(void);

/**
* @brief Set debug mode
* @param [in] nMode  --Debug mode
*					DEBUG_NONE: Close
*					DEBUG_PORT: PORT
*					DEBUG_FILE: File
* @return
* @li APP_FAIL
* @li APP_SUCC
*/
int PubSetDebugMode(int nMode);

/**
* @brief Get debug level
* @return
* @li DEBUG_LEVEL_NORMAL:  	Basic debug log
* @li DEBUG_LEVEL_WARNING:  Basic + Warning log
* @li DEBUG_LEVEL_ALL:  	All log
* @li APP_FAIL:  Not set
*/
int PubGetDebugLevel(void);

/**
* @brief Set debug level
* @param [in] nLevel --Debug level
*					DEBUG_LEVEL_NORMAL:	Basic debug log
*                   DEBUG_LEVEL_WARNING:Basic + Warning log
*                   DEBUG_LEVEL_ALL:  	All log
* @return
* @li APP_FAIL
* @li APP_SUCC
*/
int PubSetDebugLevel(int nLevel);

/**
* @brief Get debug port number
* @return
* @li PORT_NUM_COM1: RS232
* @li PORT_NUM_PINPAD: PINPAD
* @li PORT_NUM_USB:  USB
* @li APP_FAIL:  Not set
*/
int PubGetDebugPortNum(void);

/**
* @brief Set debug Port
* @param [in] nPortType Debug port number
*					RS232
*                   PINPAD
*                   USB
* @return
* @li APP_FAIL
* @li APP_SUCC
*/
void PubSetDebugPort(int nPortNum);

/**
* @brief Get Error Code
* @param [out]  pnErrorCode			Module error code
* @param [out]  pszErr				Module error info
* @param [out]  pnNAPIErrorCode		NAPI error code
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author Liug
* @date 2012-10-22
*/
void PubGetErrorCode(int *pnErrorCode, char *pszErr, int *pnNapiErrorCode);

/**
* @brief Get value from ini file with an appointed ID
* @param [in] pnHandle
* @param [in] nPosition
* @param [in] psID
* @param [out] psParamValue
* @param [out] pnParamLen
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int PubFindParam (const int pnHandle, const int nPosition,
				  const char *psID, char *psParamValue, int *pnParamLen);

/**
* @brief Set Error Code
* @param [in]  nErrorCode 		   Module error code
* @param [in]  nNapiErrCode		   Napi error code
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author Liug
* @date 2012-10-22
*/
void PubSetErrorCode(int nErrCode,const char *pszErr,int nNapiErrCode);

/**
* @brief Output debug according to debug level
* @param [in] cLevel 1-3 , 3 is lowest
* @return
* @li APP_FAIL
* @li APP_SUCC
* @author sunh
* @date
*/
int PubDebugSelectly(char cLevel, char* lpszFormat, ...);

void PubBufToAux(const char *pszBuf, const int nBufLen);

int PubGetDebugPortLevel(void);

void ProGetLogFileAttr(char *pszFilePath, char *pszLogFileName, char *pszOldLogFileName);

/**
* @brief Get Security configure
* @param [in] emFlag	EM_SEC_CFG 
* @param 
* @return 
* @li YES
* @li NO
*/
YESORNO PubGetSecCfg(int emSecCfg);


/** @}*/ // End of Debug



#endif

